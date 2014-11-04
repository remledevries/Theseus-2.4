// ======================================================================== //
// Copyright 2009-2011 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "image/image.h"
#include "sys/stl/string.h"
#include <iostream>
#include <windows.h>
#include "resource.h"

/*! include FreeImage headers */
#include "../Freeimage/FreeImage.h"

/*! include ZLib compression headers */
#define WINAPI  __stdcall
#define WINAPIV __cdecl
#include "../zlib/zlib.h"


namespace embree
{

  FIBITMAP* loadVRH(const FileName& fileName);
  FIBITMAP* loadWatermark();


  Ref<Image> loadFreeImage(const FileName& fileName)
  {
      FIBITMAP* dib = NULL;
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileType(fileName.c_str());

	  //-------------------------------------------------
	  // Sonderfall - unser Schutzformat "vrh" laden
	  // ZLib-compressed HDR-Image!
	  //-------------------------------------------------
      std::string ext = std::strlwr(fileName.ext());
      if( ext == "vrh" ) 
      {
          fif = FIF_HDR;
          dib = loadVRH(fileName);
      }
      else
      {
          if(FreeImage_FIFSupportsReading(fif))
              dib = FreeImage_Load(fif, fileName.c_str());
      }

      if(!dib)
          return NULL;

      FREE_IMAGE_COLOR_TYPE fic = FreeImage_GetColorType(dib);
      
      if(fic != FIC_RGB && fic != FIC_RGBALPHA)
          return NULL;

      BYTE* pixels = FreeImage_GetBits(dib);
      
      unsigned int width  = FreeImage_GetWidth(dib);
      unsigned int height = FreeImage_GetHeight(dib);

      if((pixels == 0) || (width == 0) || (height == 0))
          return NULL;

      FREE_IMAGE_TYPE fit = FreeImage_GetImageType(dib);

      // special image handling for float-images (e.g. *.HDR)
      if(fit == FIT_RGBF)
      {
          Image* out = new Image3f(width, height, fileName);

          for(unsigned int y = 0; y < out->height; y++)
          {
              FIRGBF* bits = (FIRGBF*)FreeImage_GetScanLine(dib, y);

              unsigned int y_pos = y;

              // Mirror HDR-images!
			  if(fif == FIF_HDR)
			      y_pos = (height - 1) - y;

              for(unsigned int x = 0; x < out->width; x++)
              {
                  unsigned int x_pos = x;

				  Color4 c;
                  c.r = bits[x].red;
                  c.g = bits[x].green;
                  c.b = bits[x].blue;

                  out->set(x_pos, y_pos, c);
              }
          }

          return out;
      }
      
	  //---------------------------------
      // normal bitmap images 
	  // --> Texturen
      // --> Backplates
	  //---------------------------------
      if(fit == FIT_BITMAP)
      {
          //Image* out = new Image3c(width, height, fileName);

		  Ref<Image> out   = new Image4c(width, height, fileName);

          unsigned int bpp = FreeImage_GetBPP(dib);
          BOOL bHasAlpha   = FreeImage_IsTransparent(dib);

          float rcpMaxRGB = 1.0f / 255.0f;

		  float rcpGamma = 1.0f; // 1.4f; // TEST - nach HDRI Handbuch ( Texturen um Gamma Wert dunker machen ) 
 
          for(unsigned int y = 0; y < out->height; y++)
          {
              BYTE* bits = (BYTE*)FreeImage_GetScanLine(dib, y);

              for(unsigned int x = 0; x < out->width; x++)
              {
				  Color4 c(0, 0, 0, 1 );

                  if(bpp == 8)
                  {
                      c.r = bits[x];
                      c.g = bits[x];
                      c.b = bits[x];
                  }

                  if(bpp == 24)
                  {
                      RGBTRIPLE& Value = ((RGBTRIPLE*)bits)[x];

                      c.r = ( ((float)Value.rgbtRed  ) * rcpMaxRGB )* rcpGamma ;
                      c.g = ( ((float)Value.rgbtGreen) * rcpMaxRGB )* rcpGamma ;
                      c.b = ( ((float)Value.rgbtBlue ) * rcpMaxRGB )* rcpGamma ;
                  }

                  if(bpp == 32)
                  {
                      RGBQUAD& Value = ((RGBQUAD*)bits)[x];

                      if(bHasAlpha && Value.rgbReserved < 255) // Alpha-channel
                      {
                          // Color = Magenta
                          c.r = (255.0f) * rcpMaxRGB;
                          c.g = (  0.0f) * rcpMaxRGB;
                          c.b = (255.0f) * rcpMaxRGB;
                      }
                      else
                      {
                          c.r = ( ((float)Value.rgbRed  ) * rcpMaxRGB)* rcpGamma ;
                          c.g = ( ((float)Value.rgbGreen) * rcpMaxRGB)* rcpGamma ;
                          c.b = ( ((float)Value.rgbBlue ) * rcpMaxRGB)* rcpGamma ;
                      }
                  }

                  out->set(x, y, c);
              }
          }

          return out;
      }

      return NULL;
  }



  void storeFreeImage(const Ref<Image>& img, const FileName& fileName)
  {
      FIBITMAP* dib = FreeImage_Allocate((int)img->width, (int)img->height, 24);

      for(size_t y = 0; y < img->height; y++)
      {
          for(size_t x = 0; x < img->width; x++)
          {
			  Color4 c = img->get(x, y);
              
              RGBQUAD Value = {0};
              Value.rgbRed   = (BYTE)(clamp(c.r) * 255.0f);
              Value.rgbGreen = (BYTE)(clamp(c.g) * 255.0f);
              Value.rgbBlue  = (BYTE)(clamp(c.b) * 255.0f);

              FreeImage_SetPixelColor(dib, (unsigned int)x, (unsigned int)y, &Value);
          }
      }

      FIBITMAP* fiLogo = loadWatermark();

      unsigned int LogoWidth  = FreeImage_GetWidth (fiLogo);
      unsigned int LogoHeight = FreeImage_GetHeight(fiLogo);

      if(LogoWidth > img->width || LogoHeight > img->height)
      {
          FreeImage_Unload(fiLogo);

          FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(fileName.c_str());
          if(FreeImage_FIFSupportsWriting(fif))
              FreeImage_Save(fif, dib, fileName.c_str());

          FreeImage_Unload(dib);
      }
      else
      {
          int x_pos = (int)img->width  - LogoWidth;
          int y_pos = (int)img->height - LogoHeight;

          FIBITMAP* fiFG = FreeImage_Allocate((int)img->width, (int)img->height, 32);
          BOOL b = FreeImage_Paste(fiFG, fiLogo, x_pos, y_pos, 255);
          FreeImage_Unload(fiLogo);

          FIBITMAP* fiNew = FreeImage_Composite(fiFG, FALSE, NULL, dib);
          FreeImage_Unload(dib);

          FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(fileName.c_str());

          int save_flags = 0;
          if(fif == FIF_JPEG)
              save_flags = JPEG_QUALITYSUPERB | JPEG_BASELINE | JPEG_OPTIMIZE;

          if(FreeImage_FIFSupportsWriting(fif))
              FreeImage_Save(fif, fiNew, fileName.c_str(), save_flags);

          FreeImage_Unload(fiNew);
      }
  }

  FIBITMAP* loadVRH(const FileName& fileName)
  {
      HANDLE hFile = CreateFile(fileName.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
      if(hFile == INVALID_HANDLE_VALUE || hFile == NULL)
      {
          DWORD dwErr = GetLastError();
          //std::cerr << "Input CreateFile failed. ErrorCode: " << dwErr << std::endl;
          return NULL;
      }

      DWORD dwFileSizeHigh = 0;
      DWORD dwFileSize = GetFileSize(hFile, &dwFileSizeHigh);
      if(dwFileSize == 0 && dwFileSizeHigh == 0)
      {
          //std::cerr << "Input file empty." << std::endl;
          CloseHandle(hFile);
          return NULL;
      }

      // Dateiinhalt aus Geschwindigkeitsgründen in den Prozessadressspeicher (process address space) mappen
      HANDLE hFileMap = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
      if(hFileMap == INVALID_HANDLE_VALUE || hFileMap == NULL)
      {
          DWORD dwErr = GetLastError();
          //std::cerr << "Input CreateFileMapping failed. ErrorCode: " << dwErr << std::endl;
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      BYTE* pData = (BYTE*)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
      if(pData == NULL)
      {
          DWORD dwErr = GetLastError();
          //std::cerr << "Input MapViewOfFile failed. ErrorCode: " << dwErr << std::endl;
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      unsigned char sig[4] = {0};
      memcpy_s(sig, _countof(sig), pData, _countof(sig));
      pData += _countof(sig);

      bool IsSignatureValid = (sig[0] == 'V' && sig[1] == 'R' && sig[2] == 'H' && sig[3] == 'Z');
      if(!IsSignatureValid)
      {
          //std::cerr << "Invalid Signature." << std::endl;
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      DWORD origSize = 0;
      memcpy_s(&origSize, sizeof(DWORD), pData, sizeof(DWORD));
      pData += sizeof(DWORD);

      DWORD compSize = 0;
      memcpy_s(&compSize, sizeof(DWORD), pData, sizeof(DWORD));
      pData += sizeof(DWORD);

      DWORD bufsize = origSize;
      BYTE* compdata = (BYTE*)malloc(bufsize * sizeof(BYTE));

      z_stream strm = {0};

      int ret = inflateInit(&strm);
      if(ret != Z_OK)
      {
          //std::cerr << "inflateInit returned " << ret << "." << std::endl;
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      strm.avail_in = compSize;
      strm.next_in = pData;

      strm.avail_out = bufsize;
      strm.next_out = compdata;

      int ret2 = inflate(&strm, Z_FINISH);
      if(ret2 != Z_STREAM_END)
      {
          //std::cerr << "inflate returned " << ret2 << "." << std::endl;
          inflateEnd(&strm);
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      int ret3 = inflateEnd(&strm);
      if(ret3 != Z_OK)
      {
          //std::cerr << "inflateEnd returned " << ret3 << "." << std::endl;
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      DWORD inSize = strm.total_in;
      DWORD outSize = strm.total_out;

      if(inSize != compSize)
      {
          //std::cerr << "Input file size mismatch. (inSize != compSize)" << std::endl;
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      if(outSize != origSize)
      {
          //std::cerr << "Output file size mismatch. (outSize != OrigSize)" << std::endl;
          UnmapViewOfFile(pData);
          CloseHandle(hFileMap);
          CloseHandle(hFile);
          return NULL;
      }

      UnmapViewOfFile(pData);
      pData = NULL;

      CloseHandle(hFileMap);
      CloseHandle(hFile);
      hFileMap = INVALID_HANDLE_VALUE;
      hFile = INVALID_HANDLE_VALUE;

      FIMEMORY* fiMem = FreeImage_OpenMemory(compdata, outSize);
      if(!fiMem)
      {
          return NULL;
      }

      FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem);
      FIBITMAP* fiDib = FreeImage_LoadFromMemory(fif, fiMem);

      FreeImage_CloseMemory(fiMem);

      return fiDib;
  }

  FIBITMAP* loadWatermark()
  {

      HRSRC hRes      = ::FindResource(NULL, MAKEINTRESOURCE(PNG_Visus_Logo), "PNG");
      DWORD dwResSize = SizeofResource(NULL, hRes);
      HGLOBAL hGlobal = ::LoadResource(NULL, hRes);

      BYTE* pData = (BYTE*)LockResource(hGlobal);

      FIMEMORY* fiMem = FreeImage_OpenMemory(pData, dwResSize);
      
      FREE_IMAGE_FORMAT fif = FreeImage_GetFileTypeFromMemory(fiMem);
      FIBITMAP* fiBmp = FreeImage_LoadFromMemory(fif, fiMem);

      FreeImage_CloseMemory(fiMem);

      UnlockResource(hGlobal);   
      DeleteObject(hGlobal);   
      DeleteObject(hRes);    

      return fiBmp;
  }

}

