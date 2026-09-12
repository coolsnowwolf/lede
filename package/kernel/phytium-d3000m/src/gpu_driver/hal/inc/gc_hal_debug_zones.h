/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_debug_zones_h_
#define __gc_hal_debug_zones_h_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
 ************************ Debug Zone Pattern Summary ***************************
 * A debug zone is an unsigned integer of 32 bit (Bit 31- Bit 0).              *
 * Bit 31 to 28 defines API, which is 0 for HAL API and has value of 1 - 14    *
 * for Khronos API. Value 15 (0xF) is reserved for gcdZONE_NONE.               *
 * Bit 27 to 0 defines subzones of each API. Value 0xFFFFFFF is resevered for  *
 * gcdZONE_ALL.                                                                *
 *                                                                             *
\******************************************************************************/

/* Retrieve API bits 31 to 28 */
#define gcmZONE_GET_API(zone) ((zone) >> 28)

/* Retrieve Subzone bits 27 to 0 */
#define gcmZONE_GET_SUBZONES(zone) ((zone) << 4)

/******************************************************************************
 ******************************** HAL Zone ************************************
 ******************************************************************************/

#define gcdZONE_API_HAL ((gctUINT32)0 << 28)

/******************************************************************************
 ******************************** HAL Subzones ********************************
 ******************************************************************************/

/* Subzones Kernel and User have in common */
#define gcvZONE_OS       (1 << 0)
#define gcvZONE_HARDWARE (1 << 1)
#define gcvZONE_HEAP     (1 << 2)
#define gcvZONE_SIGNAL   (1 << 3)

/* Subzones of HAL Kernel */
#define gcvZONE_KERNEL        (1 << 4)
#define gcvZONE_VIDMEM        (1 << 5)
#define gcvZONE_COMMAND       (1 << 6)
#define gcvZONE_DRIVER        (1 << 7)
#define gcvZONE_CMODEL        (1 << 8)
#define gcvZONE_MMU           (1 << 9)
#define gcvZONE_EVENT         (1 << 10)
#define gcvZONE_DEVICE        (1 << 11)
#define gcvZONE_DATABASE      (1 << 12)
#define gcvZONE_INTERRUPT     (1 << 13)
#define gcvZONE_POWER         (1 << 14)
#define gcvZONE_ASYNC_COMMAND (1 << 15)
#define gcvZONE_ALLOCATOR     (1 << 16)

/* Subzones of HAL User */
#define gcdZONE_HAL_API     (1 << 4)
#define gcdZONE_BUFFER      (1 << 5)
#define gcdZONE_VGBUFFER    (1 << 6)
#define gcdZONE_SURFACE     (1 << 7)
#define gcdZONE_INDEX       (1 << 8)
#define gcdZONE_STREAM      (1 << 9)
#define gcdZONE_TEXTURE     (1 << 10)
#define gcdZONE_2D          (1 << 11)
#define gcdZONE_3D          (1 << 12)
#define gcdZONE_COMPILER    (1 << 13)
#define gcdZONE_MEM         (1 << 14)
#define gcdZONE_VERTEXARRAY (1 << 15)
#define gcdZONE_CL          (1 << 16)
#define gcdZONE_VG          (1 << 17)
#define gcdZONE_VX          (1 << 18)
#define gcdZONE_UTILITY     (1 << 19)
#define gcdZONE_RECT        (1 << 20)
#define gcdZONE_BUFOBJ      (1 << 21)
#define gcdZONE_PROFILER    (1 << 22)
#define gcdZONE_SHADER      (1 << 23)

/******************************************************************************
 ******************************** Khronos API Zones ***************************
 ******************************************************************************/

#define gcdZONE_API_EGL  ((gctUINT32)1 << 28)
#define gcdZONE_API_ES11 ((gctUINT32)2 << 28)
#define gcdZONE_API_ES30 ((gctUINT32)3 << 28)
#define gcdZONE_API_GL40 ((gctUINT32)4 << 28)
#define gcdZONE_API_VG3D ((gctUINT32)5 << 28)
#define gcdZONE_API_CL   ((gctUINT32)6 << 28)
#define gcdZONE_API_VX   ((gctUINT32)7 << 28)
#define gcdZONE_API_VG   ((gctUINT32)8 << 28)

/******************************************************************************
 ************************* Subzones of Khronos API Zones **********************
 ******************************************************************************/

/* Subzones of EGL API */
#define gcdZONE_EGL_API           (gcdZONE_API_EGL | (1 << 0))
#define gcdZONE_EGL_SURFACE       (gcdZONE_API_EGL | (1 << 1))
#define gcdZONE_EGL_CONTEXT       (gcdZONE_API_EGL | (1 << 2))
#define gcdZONE_EGL_CONFIG        (gcdZONE_API_EGL | (1 << 3))
#define gcdZONE_EGL_OS            (gcdZONE_API_EGL | (1 << 4)) /* unused */
#define gcdZONE_EGL_IMAGE         (gcdZONE_API_EGL | (1 << 5))
#define gcdZONE_EGL_SWAP          (gcdZONE_API_EGL | (1 << 6))
#define gcdZONE_EGL_INIT          (gcdZONE_API_EGL | (1 << 7))
#define gcdZONE_EGL_SYNC          (gcdZONE_API_EGL | (1 << 8))
#define gcdZONE_EGL_COMPOSE       (gcdZONE_API_EGL | (1 << 9))  /* unused */
#define gcdZONE_EGL_RENDER_THREAD (gcdZONE_API_EGL | (1 << 10)) /* unused */

/* Subzones of ES11 API */
#define gcdZONE_ES11_BUFFER    (gcdZONE_API_ES11 | (1 << 0))
#define gcdZONE_ES11_CLEAR     (gcdZONE_API_ES11 | (1 << 1))
#define gcdZONE_ES11_CLIP      (gcdZONE_API_ES11 | (1 << 2))
#define gcdZONE_ES11_CONTEXT   (gcdZONE_API_ES11 | (1 << 3))
#define gcdZONE_ES11_DRAW      (gcdZONE_API_ES11 | (1 << 4))
#define gcdZONE_ES11_ENABLE    (gcdZONE_API_ES11 | (1 << 5))
#define gcdZONE_ES11_EXTENTION (gcdZONE_API_ES11 | (1 << 6))
#define gcdZONE_ES11_FOG       (gcdZONE_API_ES11 | (1 << 7))
#define gcdZONE_ES11_FRAGMENT  (gcdZONE_API_ES11 | (1 << 8))
#define gcdZONE_ES11_LIGHT     (gcdZONE_API_ES11 | (1 << 9))
#define gcdZONE_ES11_MATRIX    (gcdZONE_API_ES11 | (1 << 10))
#define gcdZONE_ES11_PIXEL     (gcdZONE_API_ES11 | (1 << 11))
#define gcdZONE_ES11_POLIGON   (gcdZONE_API_ES11 | (1 << 12))
#define gcdZONE_ES11_LINE      (gcdZONE_API_ES11 | (1 << 13)) /* unused */
#define gcdZONE_ES11_QUERY     (gcdZONE_API_ES11 | (1 << 14))
#define gcdZONE_ES11_TEXTURE   (gcdZONE_API_ES11 | (1 << 15))
#define gcdZONE_ES11_STATES    (gcdZONE_API_ES11 | (1 << 16))
#define gcdZONE_ES11_STREAM    (gcdZONE_API_ES11 | (1 << 17))
#define gcdZONE_ES11_VIEWPORT  (gcdZONE_API_ES11 | (1 << 18))
#define gcdZONE_ES11_SHADER    (gcdZONE_API_ES11 | (1 << 19))
#define gcdZONE_ES11_HASH      (gcdZONE_API_ES11 | (1 << 20))
#define gcdZONE_ES11_TRACE     (gcdZONE_API_ES11 | (1 << 21))

/* Subzones of ES30 API */
#define gcdZONE_ES30_TRACE    (gcdZONE_API_ES30 | (1 << 0))
#define gcdZONE_ES30_BUFFER   (gcdZONE_API_ES30 | (1 << 1))
#define gcdZONE_ES30_CLEAR    (gcdZONE_API_ES30 | (1 << 2))
#define gcdZONE_ES30_CODEC    (gcdZONE_API_ES30 | (1 << 3))
#define gcdZONE_ES30_CONTEXT  (gcdZONE_API_ES30 | (1 << 4))
#define gcdZONE_ES30_DEPTH    (gcdZONE_API_ES30 | (1 << 5))
#define gcdZONE_ES30_DEVICE   (gcdZONE_API_ES30 | (1 << 6))
#define gcdZONE_ES30_DRAW     (gcdZONE_API_ES30 | (1 << 7))
#define gcdZONE_ES30_FBO      (gcdZONE_API_ES30 | (1 << 8))
#define gcdZONE_ES30_PIXEL    (gcdZONE_API_ES30 | (1 << 9))
#define gcdZONE_ES30_SHADER   (gcdZONE_API_ES30 | (1 << 10))
#define gcdZONE_ES30_STATE    (gcdZONE_API_ES30 | (1 << 11))
#define gcdZONE_ES30_TEXTURE  (gcdZONE_API_ES30 | (1 << 12))
#define gcdZONE_ES30_UTILS    (gcdZONE_API_ES30 | (1 << 13))
#define gcdZONE_ES30_PROFILER (gcdZONE_API_ES30 | (1 << 14))
#define gcdZONE_ES30_CORE     (gcdZONE_API_ES30 | (1 << 15))

/* Subzones of GL40 API */
#define gcdZONE_GL40_TRACE     (gcdZONE_API_GL40 | (1 << 0))
#define gcdZONE_GL40_BUFFER    (gcdZONE_API_GL40 | (1 << 1))
#define gcdZONE_GL40_CLEAR     (gcdZONE_API_GL40 | (1 << 2)) /* unused */
#define gcdZONE_GL40_CODEC     (gcdZONE_API_GL40 | (1 << 3))
#define gcdZONE_GL40_CONTEXT   (gcdZONE_API_GL40 | (1 << 4))
#define gcdZONE_GL40_DEPTH     (gcdZONE_API_GL40 | (1 << 5))
#define gcdZONE_GL40_DEVICE    (gcdZONE_API_GL40 | (1 << 6))
#define gcdZONE_GL40_DRAW      (gcdZONE_API_GL40 | (1 << 7))
#define gcdZONE_GL40_FBO       (gcdZONE_API_GL40 | (1 << 8))
#define gcdZONE_GL40_PIXEL     (gcdZONE_API_GL40 | (1 << 9))
#define gcdZONE_GL40_SHADER    (gcdZONE_API_GL40 | (1 << 10))
#define gcdZONE_GL40_STATE     (gcdZONE_API_GL40 | (1 << 11))
#define gcdZONE_GL40_TEXTURE   (gcdZONE_API_GL40 | (1 << 12))
#define gcdZONE_GL40_UTILS     (gcdZONE_API_GL40 | (1 << 13))
#define gcdZONE_GL40_PROFILER  (gcdZONE_API_GL40 | (1 << 14))
#define gcdZONE_GL40_CORE      (gcdZONE_API_GL40 | (1 << 15))
#define gcdZONE_GL40_FIXVERTEX (gcdZONE_API_GL40 | (1 << 16))
#define gcdZONE_GL40_FIXFRAG   (gcdZONE_API_GL40 | (1 << 17))
#define gcdZONE_GL40_HASH      (gcdZONE_API_GL40 | (1 << 18))

/* Subzones of VG3D API  */
#define gcdZONE_VG3D_CONTEXT     (gcdZONE_API_VG3D | (1 << 0))
#define gcdZONE_VG3D_DUMP        (gcdZONE_API_VG3D | (1 << 1))
#define gcdZONE_VG3D_EGL         (gcdZONE_API_VG3D | (1 << 2))
#define gcdZONE_VG3D_FONT        (gcdZONE_API_VG3D | (1 << 3))
#define gcdZONE_VG3D_HARDWARE    (gcdZONE_API_VG3D | (1 << 4))
#define gcdZONE_VG3D_IMAGE       (gcdZONE_API_VG3D | (1 << 5))
#define gcdZONE_VG3D_MASK        (gcdZONE_API_VG3D | (1 << 6))
#define gcdZONE_VG3D_MATRIX      (gcdZONE_API_VG3D | (1 << 7))
#define gcdZONE_VG3D_OBJECT      (gcdZONE_API_VG3D | (1 << 8))
#define gcdZONE_VG3D_PAINT       (gcdZONE_API_VG3D | (1 << 9))
#define gcdZONE_VG3D_PATH        (gcdZONE_API_VG3D | (1 << 10))
#define gcdZONE_VG3D_PROFILER    (gcdZONE_API_VG3D | (1 << 11))
#define gcdZONE_VG3D_SCANLINE    (gcdZONE_API_VG3D | (1 << 12))
#define gcdZONE_VG3D_SHADER      (gcdZONE_API_VG3D | (1 << 13))
#define gcdZONE_VG3D_TESSELLATOR (gcdZONE_API_VG3D | (1 << 14))
#define gcdZONE_VG3D_VGU         (gcdZONE_API_VG3D | (1 << 15))

/* Subzones of VG11 API  */
#define gcdZONE_VG_ARC       (gcdZONE_API_VG | (1 << 0))
#define gcdZONE_VG_CONTEXT   (gcdZONE_API_VG | (1 << 1))
#define gcdZONE_VG_DEBUG     (gcdZONE_API_VG | (1 << 2))
#define gcdZONE_VG_FILTER    (gcdZONE_API_VG | (1 << 3))
#define gcdZONE_VG_FORMAT    (gcdZONE_API_VG | (1 << 4))
#define gcdZONE_VG_IMAGE     (gcdZONE_API_VG | (1 << 5))
#define gcdZONE_VG_MAIN      (gcdZONE_API_VG | (1 << 6))
#define gcdZONE_VG_MASK      (gcdZONE_API_VG | (1 << 7))
#define gcdZONE_VG_MATRIX    (gcdZONE_API_VG | (1 << 8))
#define gcdZONE_VG_MEMORYMGR (gcdZONE_API_VG | (1 << 9))
#define gcdZONE_VG_OBJECT    (gcdZONE_API_VG | (1 << 10))
#define gcdZONE_VG_PAINT     (gcdZONE_API_VG | (1 << 11))
#define gcdZONE_VG_PATH      (gcdZONE_API_VG | (1 << 12))
#define gcdZONE_VG_STATE     (gcdZONE_API_VG | (1 << 13))
#define gcdZONE_VG_STROKE    (gcdZONE_API_VG | (1 << 14))
#define gcdZONE_VG_TEXT      (gcdZONE_API_VG | (1 << 15))
#define gcdZONE_VG_VGU       (gcdZONE_API_VG | (1 << 16))

/* Subzones of CL API  */
#define gcdZONE_CL_COMMAND        (gcdZONE_API_CL | (1 << 0))
#define gcdZONE_CL_CONTEXT        (gcdZONE_API_CL | (1 << 1))
#define gcdZONE_CL_DEVICE         (gcdZONE_API_CL | (1 << 2))
#define gcdZONE_CL_ENQUEUE        (gcdZONE_API_CL | (1 << 3))
#define gcdZONE_CL_EVENT          (gcdZONE_API_CL | (1 << 4))
#define gcdZONE_CL_EXT            (gcdZONE_API_CL | (1 << 5))
#define gcdZONE_CL_GL             (gcdZONE_API_CL | (1 << 6))
#define gcdZONE_CL_KERNEL         (gcdZONE_API_CL | (1 << 7))
#define gcdZONE_CL_MEM            (gcdZONE_API_CL | (1 << 8))
#define gcdZONE_CL_PLATFORM       (gcdZONE_API_CL | (1 << 9))
#define gcdZONE_CL_PROFILER       (gcdZONE_API_CL | (1 << 10))
#define gcdZONE_CL_PROGRAM        (gcdZONE_API_CL | (1 << 11))
#define gcdZONE_CL_SAMPLER        (gcdZONE_API_CL | (1 << 12))
#define gcdZONE_CL_COMMAND_BUFFER (gcdZONE_API_CL | (1 << 13))

/* Subzones of VX API  */
#define gcdZONE_VX_ARRAY     (gcdZONE_API_VX | (1 << 0))
#define gcdZONE_VX_BINARY    (gcdZONE_API_VX | (1 << 1))
#define gcdZONE_VX_CONTEXT   (gcdZONE_API_VX | (1 << 2))
#define gcdZONE_VX_CONV      (gcdZONE_API_VX | (1 << 3))
#define gcdZONE_VX_DELAY     (gcdZONE_API_VX | (1 << 4))
#define gcdZONE_VX_DIST      (gcdZONE_API_VX | (1 << 5))
#define gcdZONE_VX_GPULAYER  (gcdZONE_API_VX | (1 << 6))
#define gcdZONE_VX_GRAPH     (gcdZONE_API_VX | (1 << 7))
#define gcdZONE_VX_IMAGE     (gcdZONE_API_VX | (1 << 8))
#define gcdZONE_VX_INTERFACE (gcdZONE_API_VX | (1 << 9))
#define gcdZONE_VX_KERNEL    (gcdZONE_API_VX | (1 << 10))
#define gcdZONE_VX_LAYER     (gcdZONE_API_VX | (1 << 11))
#define gcdZONE_VX_LUT       (gcdZONE_API_VX | (1 << 12))
#define gcdZONE_VX_MATRIX    (gcdZONE_API_VX | (1 << 13))
#define gcdZONE_VX_MEMORY    (gcdZONE_API_VX | (1 << 14))
#define gcdZONE_VX_METAFMT   (gcdZONE_API_VX | (1 << 15))
#define gcdZONE_VX_NODE      (gcdZONE_API_VX | (1 << 16))
#define gcdZONE_VX_OBJARRAY  (gcdZONE_API_VX | (1 << 17))
#define gcdZONE_VX_PARAM     (gcdZONE_API_VX | (1 << 18))
#define gcdZONE_VX_PROGRAM   (gcdZONE_API_VX | (1 << 19))
#define gcdZONE_VX_PYRAMID   (gcdZONE_API_VX | (1 << 20))
#define gcdZONE_VX_REF       (gcdZONE_API_VX | (1 << 21))
#define gcdZONE_VX_REMAP     (gcdZONE_API_VX | (1 << 22))
#define gcdZONE_VX_SCALAR    (gcdZONE_API_VX | (1 << 23))
#define gcdZONE_VX_TARGET    (gcdZONE_API_VX | (1 << 24))
#define gcdZONE_VX_TENSOR    (gcdZONE_API_VX | (1 << 25))
#define gcdZONE_VX_THRESHOLD (gcdZONE_API_VX | (1 << 26))
#define gcdZONE_VX_SPINST    (gcdZONE_API_VX | (1 << 27))
#define gcdZONE_VX_SP        (gcdZONE_API_VX | (1 << 28))
#define gcdZONE_VX_DEVICE    (gcdZONE_API_VX | (1 << 29))
#define gcdZONE_VX_ENV       (gcdZONE_API_VX | (1 << 30))
#define gcdZONE_VX_OTHERS    (gcdZONE_API_VX | (1 << 31))

/******************************************************************************
 ******************************** Utility Zones *******************************
 ******************************************************************************/

/* Value for Disabling All Subzones */
#define gcdZONE_NONE 0xF0000000

/* Value for Enabling All Subzones */
#define gcdZONE_ALL 0x0FFFFFFF

/******************************************************************************
 ********************************** END ***************************************
 ******************************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_debug_zones_h_ */
