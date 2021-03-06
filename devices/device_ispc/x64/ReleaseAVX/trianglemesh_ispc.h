//
// x64\ReleaseAVX\trianglemesh_ispc.h
// (Header automatically generated by the ispc compiler.)
// DO NOT EDIT THIS FILE.
//

#ifndef ISPC_X64_RELEASEAVX_TRIANGLEMESH_ISPC_H
#define ISPC_X64_RELEASEAVX_TRIANGLEMESH_ISPC_H

#include <stdint.h>



#ifdef __cplusplus
namespace ispc { /* namespace */
#endif // __cplusplus
#ifndef __ISPC_STRUCT_vec3fa__
#define __ISPC_STRUCT_vec3fa__
struct vec3fa {
    float x;
    float y;
    float z;
    float a;
};
#endif

#ifndef __ISPC_STRUCT_vec2f__
#define __ISPC_STRUCT_vec2f__
struct vec2f {
    float x;
    float y;
};
#endif

#ifndef __ISPC_STRUCT_vec4i__
#define __ISPC_STRUCT_vec4i__
struct vec4i {
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t w;
};
#endif


///////////////////////////////////////////////////////////////////////////
// Functions exported from ispc code
///////////////////////////////////////////////////////////////////////////
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
extern "C" {
#endif // __cplusplus
    extern void *  TriangleMesh__new(struct vec3fa * position, struct vec3fa * motion, struct vec3fa * normal, struct vec2f * texcoord, int32_t numPositions, struct vec4i * triangle, int32_t numTriangles);
#if defined(__cplusplus) && !defined(__ISPC_NO_EXTERN_C)
} /* end extern C */
#endif // __cplusplus


#ifdef __cplusplus
} /* namespace */
#endif // __cplusplus

#endif // ISPC_X64_RELEASEAVX_TRIANGLEMESH_ISPC_H
