//
// x64\Debug\depthoffieldcamera_ispc.h
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#ifndef ISPC_X64_DEBUG_DEPTHOFFIELDCAMERA_ISPC_H
#define ISPC_X64_DEBUG_DEPTHOFFIELDCAMERA_ISPC_H

#include <stdint.h>



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus
#ifndef __ISPC_STRUCT_vec3f__
#define __ISPC_STRUCT_vec3f__
struct vec3f {
    float x;
    float y;
    float z;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
extern "C" {
#endif // __cplusplus
    extern void *  DepthOfFieldCamera__new(const struct vec3f &vx, const struct vec3f &vy, const struct vec3f &vz, const struct vec3f &p, const float &fov, const float &aspectRatio, const float &lensRadius, const float &focalDistance);
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus

#endif // ISPC_X64_DEBUG_DEPTHOFFIELDCAMERA_ISPC_H
