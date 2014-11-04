// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
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

#include "image.h"
#include "sys/stl/string.h"

#include <map>
#include <iostream>

namespace embree
{
	//--------------------------------------------------------
	// Datei von Disk laden
	//--------------------------------------------------------
	Ref<Image> loadImageFromDisk(const FileName& fileName)
	{

		std::string ext = std::strlwr(fileName.ext());

		try
		{

			//if (ext == "vrh")  return loadFreeImage(fileName);

			Ref<Image> Bild = loadFreeImage( fileName ); 

			if (Bild)
				return Bild;
			else
				throw std::runtime_error("image format " + ext + " not supported");


		}
		catch (const std::exception& e)
		{
			std::cout << "cannot read file " << fileName << ": " << e.what() << std::endl;
			return null;
		}
	}

  //-----------------------------------------------------------------
  /*! loads an image from a file with auto-detection of format */
  //-----------------------------------------------------------------
  Ref<Image> loadImage(const FileName& fileName, bool cache)
  {
    static std::map<std::string,Ref<Image> > image_cache;

    if (!cache)
      return loadImageFromDisk(fileName);

    if (image_cache.find(fileName) == image_cache.end())
      image_cache[fileName] = loadImageFromDisk(fileName);

    return image_cache[fileName];
  }

  //---------------------------------------------------------------------
  /*! stores an image to file with auto-detection of format */
  //---------------------------------------------------------------------
  void storeImage(const Ref<Image>& img, const FileName& fileName)
  {

	  std::string ext = strlwr(fileName.ext());

	  try
	  {

		  std::string ext = strlwr(fileName.ext());
		  if (ext == "tga")  { storeFreeImage(img, fileName); return; }
		  if (ext == "gif")  { storeFreeImage(img, fileName); return; }
		  if (ext == "jpg")  { storeFreeImage(img, fileName); return; }
		  if (ext == "png")  { storeFreeImage(img, fileName); return; }
		  if (ext == "bmp")  { storeFreeImage(img, fileName); return; }
		  if (ext == "ppm")  { storeFreeImage(img, fileName); return; }
		  if (ext == "tif")  { storeFreeImage(img, fileName); return; }
		  if (ext == "tiff") { storeFreeImage(img, fileName); return; }
		  if (ext == "pfm")  { storeFreeImage(img, fileName); return; }

	  }
	  catch (const std::exception& e)
	  {
		  std::cout << "cannot write file " << fileName << ": " << e.what() << std::endl;
	  }
  }

  /*! template instantiations */
  template class ImageT<Col3c>;
  template class ImageT<Col3f>;

}
