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



#ifndef __gc_hal_enum_h_
#define __gc_hal_enum_h_

#include "gc_hal_options.h"
#include "shared/gc_hal_enum_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

/* dummy draw type.*/
typedef enum _gceDUMMY_DRAW_TYPE {
    gcvDUMMY_DRAW_INVALID = 0,
    gcvDUMMY_DRAW_GC400,
    gcvDUMMY_DRAW_V60,
} gceDUMMY_DRAW_TYPE;

/* Option Set*/
typedef enum _gceOPTION {
    /* HW setting. */
    gcvOPTION_PREFER_ZCONVERT_BYPASS      = 0,
    gcvOPTION_PREFER_TILED_DISPLAY_BUFFER = 1,
    gcvOPTION_PREFER_GUARDBAND            = 2,
    gcvOPTION_PREFER_TPG_TRIVIALMODEL     = 3,
    gcvOPTION_PREFER_RA_DEPTH_WRITE       = 4,
    gcvOPTION_PREFER_USC_RECONFIG         = 5,
    gcvOPTION_PREFER_DISALBE_HZ           = 6,
    /* SW options */
    gcvOPTION_HW_NULL           = 50,
    gcvOPTION_PRINT_OPTION      = 51,
    gcvOPTION_KERNEL_FENCE      = 52,
    gcvOPTION_ASYNC_PIPE        = 53,
    gcvOPTION_FBO_PREFER_MEM    = 54,
    gcvOPTION_GPU_TEX_UPLOAD    = 55,
    gcvOPTION_GPU_BUFOBJ_UPLOAD = 56,
    gcvOPTION_NO_Y_INVERT       = 60,

    /* OCL option */
    gcvOPTION_OCL_ASYNC_BLT = 200,
    gcvOPTION_OCL_IN_THREAD,
    gcvOPTION_COMPRESSION_DEC400,
    gcvOPTION_OCL_VIR_SHADER,
    gcvOPTION_OCL_USE_MULTI_DEVICES,

#if gcdUSE_VX

    /* OVX options that HAL could access */
    gcvOPTION_OVX_ENABLE_NN_ZDP3 = 500,
    gcvOPTION_OVX_ENABLE_NN_ZDP6,
    gcvOPTION_OVX_ENABLE_NN_STRIDE,
    gcvOPTION_OVX_USE_MULTI_DEVICES,
    gcvOPTION_OVX_ENABLE_NN_DDR_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_NN_DDR_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_NN_DDR_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_NN_DDR_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_NN_DDR_BURST_SIZE_64B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_DDR_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_DDR_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_DDR_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_DDR_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_DDR_BURST_SIZE_64B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_DDR_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_DDR_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_DDR_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_DDR_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_DDR_BURST_SIZE_64B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_DDR_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_DDR_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_DDR_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_DDR_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_DDR_BURST_SIZE_64B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_64B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_32B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID0_MIN_AXI_BURST_SIZE_16B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_64B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_32B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID1_MIN_AXI_BURST_SIZE_16B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_64B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_32B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID2_MIN_AXI_BURST_SIZE_16B,

    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_1024B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_512B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_256B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_128B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_64B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_32B,
    gcvOPTION_OVX_ENABLE_VIP_AXI_ID3_MIN_AXI_BURST_SIZE_16B,

#endif

    /* Insert option above this comment only */
    gcvOPTION_COUNT /* Not a OPTION*/
} gceOPTION;

typedef enum _gceFRAMEINFO {
    /* Total frame count in one run */
    gcvFRAMEINFO_FRAME_NUM      = 0,
    /* Total draw count in current frame, including draw/compute */
    gcvFRAMEINFO_DRAW_NUM       = 1,
    /* Total compute count in current frame, subset of drawNum */
    gcvFRAMEINFO_COMPUTE_NUM    = 2,
    /* Total dual16 draw/compute count in current frame, subset of drawNum */
    gcvFRAMEINFO_DUAL16_NUM     = 3,
    /* Current programID is being set. only valid for ES20 driver right now */
    gcvFRAMEINFO_PROGRAM_ID     = 4,

    gcvFRAMEINFO_COUNT,
} gceFRAMEINFO;

typedef enum _gceFRAMEINFO_OP {
    gcvFRAMEINFO_OP_INC     = 0,
    gcvFRAMEINFO_OP_DEC     = 1,
    gcvFRAMEINFO_OP_ZERO    = 2,
    gcvFRAMEINFO_OP_GET     = 3,
    gcvFRAMEINFO_OP_SET     = 4,
    gcvFRAMEINFO_OP_COUNT,
} gceFRAMEINFO_OP;

typedef enum _gceSURF_USAGE {
    gcvSURF_USAGE_UNKNOWN,
    gcvSURF_USAGE_RESOLVE_AFTER_CPU,
    gcvSURF_USAGE_RESOLVE_AFTER_3D

} gceSURF_USAGE;

typedef enum _gceSURF_COLOR_SPACE {
    gcvSURF_COLOR_SPACE_UNKNOWN,
    gcvSURF_COLOR_SPACE_LINEAR,
    gcvSURF_COLOR_SPACE_NONLINEAR,
} gceSURF_COLOR_SPACE;

typedef enum _gceSURF_COLOR_TYPE {
    gcvSURF_COLOR_UNKNOWN   = 0,
    gcvSURF_COLOR_LINEAR    = 0x01,
    gcvSURF_COLOR_ALPHA_PRE = 0x02,
} gceSURF_COLOR_TYPE;

/* Rotation. */
typedef enum _gceSURF_ROTATION {
    gcvSURF_0_DEGREE = 0,
    gcvSURF_90_DEGREE,
    gcvSURF_180_DEGREE,
    gcvSURF_270_DEGREE,
    gcvSURF_FLIP_X,
    gcvSURF_FLIP_Y,

    gcvSURF_POST_FLIP_X = 0x40000000,
    gcvSURF_POST_FLIP_Y = 0x80000000,
} gceSURF_ROTATION;

/* Surface flag */
typedef enum _gceSURF_FLAG {
    /* None flag */
    gcvSURF_FLAG_NONE                = 0x0,
    /* content is preserved after swap */
    gcvSURF_FLAG_CONTENT_PRESERVED   = 0x1,
    /* content is updated after swap*/
    gcvSURF_FLAG_CONTENT_UPDATED     = 0x2,
    /* content is y inverted */
    gcvSURF_FLAG_CONTENT_YINVERTED   = 0x4,
    /* surface has multiple nodes */
    gcvSURF_FLAG_MULTI_NODE          = 0x8,
    /* surface no need do dither when resovle*/
    gcvSURF_FLAG_DITHER_DISABLED     = 0x10,
    /* surface used a fake hal format */
    gcvSURF_FLAG_FAKE_FORMAT         = 0x20,
} gceSURF_FLAG;

typedef enum _gceMIPMAP_IMAGE_FORMAT {
    gcvUNKNOWN_MIPMAP_IMAGE_FORMAT = -2

} gceMIPMAP_IMAGE_FORMAT;

typedef enum _gceIMAGE_MEM_TYPE {
    gcvIMAGE_MEM_DEFAULT,
    gcvIMAGE_MEM_HOST_PTR,
    gcvIMAGE_MEM_HOST_PTR_UNCACHED,
    gcvIMAGE_MEM_HOST_PHY_PTR,
    gcvIMAGE_MEM_HOST_PHY_PTR_UNCACHED,
} gceIMAGE_MEM_TYPE;

typedef enum _gceSURF_YUV_COLOR_SPACE {
    gcvSURF_ITU_REC601,
    gcvSURF_ITU_REC709,
    gcvSURF_ITU_REC2020,
} gceSURF_YUV_COLOR_SPACE;

typedef enum _gceSURF_YUV_SAMPLE_RANGE {
    gcvSURF_YUV_FULL_RANGE,
    gcvSURF_YUV_NARROW_RANGE,
} gceSURF_YUV_SAMPLE_RANGE;

typedef enum _gceSURF_YUV_CHROMA_SITING {
    gcvSURF_YUV_CHROMA_SITING_0,
    gcvSURF_YUV_CHROMA_SITING_0_5,
} gceSURF_YUV_CHROMA_SITING;

typedef enum _gceSURF_INFO_TYPE {
    gcvSURF_INFO_UNKNOWN   = 0,
    gcvSURF_INFO_LAYERSIZE = 1,
    gcvSURF_INFO_SLICESIZE = 2,
} gceSURF_INFO_TYPE;

/* Format modifiers. */
typedef enum _gceSURF_FORMAT_MODE {
    gcvSURF_FORMAT_OCL          = 0x80000000,
    gcvSURF_FORMAT_PATCH_BORDER = 0x40000000,
} gceSURF_FORMAT_MODE;

/* Pixel swizzle modes. */
typedef enum _gceSURF_SWIZZLE {
    gcvSURF_NOSWIZZLE = 0,
    gcvSURF_ARGB,
    gcvSURF_ABGR,
    gcvSURF_RGBA,
    gcvSURF_BGRA

} gceSURF_SWIZZLE;

/* Transparency modes. */
typedef enum _gceSURF_TRANSPARENCY {
    /* Valid only for PE 1.0 */
    gcvSURF_OPAQUE = 0,
    gcvSURF_SOURCE_MATCH,
    gcvSURF_SOURCE_MASK,
    gcvSURF_PATTERN_MASK,
} gceSURF_TRANSPARENCY;

/* Surface Alignment. */
typedef enum _gceSURF_ALIGNMENT {
    gcvSURF_FOUR = 0,
    gcvSURF_SIXTEEN,
    gcvSURF_SUPER_TILED,
    gcvSURF_SPLIT_TILED,
    gcvSURF_SPLIT_SUPER_TILED

} gceSURF_ALIGNMENT;

/* Surface Addressing. */
typedef enum _gceSURF_ADDRESSING {
    gcvSURF_NO_STRIDE_TILED = 0,
    gcvSURF_NO_STRIDE_LINEAR,
    gcvSURF_STRIDE_TILED,
    gcvSURF_STRIDE_LINEAR

} gceSURF_ADDRESSING;

/* Transparency modes. */
typedef enum _gce2D_TRANSPARENCY {
    /* Valid only for PE 2.0 */
    gcv2D_OPAQUE = 0,
    gcv2D_KEYED,
    gcv2D_MASKED

} gce2D_TRANSPARENCY;

/* Mono packing modes. */
typedef enum _gceSURF_MONOPACK {
    gcvSURF_PACKED8 = 0,
    gcvSURF_PACKED16,
    gcvSURF_PACKED32,
    gcvSURF_UNPACKED,
} gceSURF_MONOPACK;

/* Blending modes. */
typedef enum _gceSURF_BLEND_MODE {
    /* Porter-Duff blending modes.                    */
    /*                          Fsrc      Fdst        */
    gcvBLEND_CLEAR = 0,      /* 0         0           */
    gcvBLEND_SRC,            /* 1         0           */
    gcvBLEND_DST,            /* 0         1           */
    gcvBLEND_SRC_OVER_DST,   /* 1         1 - Asrc    */
    gcvBLEND_DST_OVER_SRC,   /* 1 - Adst  1           */
    gcvBLEND_SRC_IN_DST,     /* Adst      0           */
    gcvBLEND_DST_IN_SRC,     /* 0         Asrc        */
    gcvBLEND_SRC_OUT_DST,    /* 1 - Adst  0           */
    gcvBLEND_DST_OUT_SRC,    /* 0         1 - Asrc    */
    gcvBLEND_SRC_ATOP_DST,   /* Adst      1 - Asrc    */
    gcvBLEND_DST_ATOP_SRC,   /* 1 - Adst  Asrc        */
    gcvBLEND_SRC_XOR_DST,    /* 1 - Adst  1 - Asrc    */


    /* Special blending modes.                        */
    gcvBLEND_SET,            /* DST = 1               */
    gcvBLEND_SUB             /* DST = DST * (1 - SRC) */
} gceSURF_BLEND_MODE;

/* Per-pixel alpha modes. */
typedef enum _gceSURF_PIXEL_ALPHA_MODE {
    gcvSURF_PIXEL_ALPHA_STRAIGHT = 0,
    gcvSURF_PIXEL_ALPHA_INVERSED

} gceSURF_PIXEL_ALPHA_MODE;

/* Global alpha modes. */
typedef enum _gceSURF_GLOBAL_ALPHA_MODE {
    gcvSURF_GLOBAL_ALPHA_OFF = 0,
    gcvSURF_GLOBAL_ALPHA_ON,
    gcvSURF_GLOBAL_ALPHA_SCALE

} gceSURF_GLOBAL_ALPHA_MODE;

/* Color component modes for alpha blending. */
typedef enum _gceSURF_PIXEL_COLOR_MODE {
    gcvSURF_COLOR_STRAIGHT = 0,
    gcvSURF_COLOR_MULTIPLY

} gceSURF_PIXEL_COLOR_MODE;

/* Color component modes for alpha blending. */
typedef enum _gce2D_PIXEL_COLOR_MULTIPLY_MODE {
    gcv2D_COLOR_MULTIPLY_DISABLE = 0,
    gcv2D_COLOR_MULTIPLY_ENABLE

} gce2D_PIXEL_COLOR_MULTIPLY_MODE;

/* Color component modes for alpha blending. */
typedef enum _gce2D_GLOBAL_COLOR_MULTIPLY_MODE {
    gcv2D_GLOBAL_COLOR_MULTIPLY_DISABLE = 0,
    gcv2D_GLOBAL_COLOR_MULTIPLY_ALPHA,
    gcv2D_GLOBAL_COLOR_MULTIPLY_COLOR

} gce2D_GLOBAL_COLOR_MULTIPLY_MODE;

/* Alpha blending factor modes. */
typedef enum _gceSURF_BLEND_FACTOR_MODE {
    gcvSURF_BLEND_ZERO = 0,
    gcvSURF_BLEND_ONE,
    gcvSURF_BLEND_STRAIGHT,
    gcvSURF_BLEND_INVERSED,
    gcvSURF_BLEND_COLOR,
    gcvSURF_BLEND_COLOR_INVERSED,
    gcvSURF_BLEND_SRC_ALPHA_SATURATED,
    gcvSURF_BLEND_STRAIGHT_NO_CROSS,
    gcvSURF_BLEND_INVERSED_NO_CROSS,
    gcvSURF_BLEND_COLOR_NO_CROSS,
    gcvSURF_BLEND_COLOR_INVERSED_NO_CROSS,
    gcvSURF_BLEND_SRC_ALPHA_SATURATED_CROSS

} gceSURF_BLEND_FACTOR_MODE;

/* Alpha blending porter duff rules. */
typedef enum _gce2D_PORTER_DUFF_RULE {
    gcvPD_CLEAR = 0,
    gcvPD_SRC,
    gcvPD_SRC_OVER,
    gcvPD_DST_OVER,
    gcvPD_SRC_IN,
    gcvPD_DST_IN,
    gcvPD_SRC_OUT,
    gcvPD_DST_OUT,
    gcvPD_SRC_ATOP,
    gcvPD_DST_ATOP,
    gcvPD_ADD,
    gcvPD_XOR,
    gcvPD_DST

} gce2D_PORTER_DUFF_RULE;

/* Alpha blending factor modes. */
typedef enum _gce2D_YUV_COLOR_MODE {
    gcv2D_YUV_601 = 0,
    gcv2D_YUV_709,
    gcv2D_YUV_2020,
    gcv2D_YUV_USER_DEFINED,
    gcv2D_YUV_USER_DEFINED_CLAMP,

    /*  Default setting is for src. gcv2D_YUV_DST

     *  can be ORed to set dst.

     */
    gcv2D_YUV_DST = 0x80000000,
} gce2D_YUV_COLOR_MODE;

/* Nature rotation rules. */
typedef enum _gce2D_NATURE_ROTATION {
    gcvNR_0_DEGREE = 0,
    gcvNR_LEFT_90_DEGREE,
    gcvNR_RIGHT_90_DEGREE,
    gcvNR_180_DEGREE,
    gcvNR_FLIP_X,
    gcvNR_FLIP_Y,
    gcvNR_TOTAL_RULE,
} gce2D_NATURE_ROTATION;

typedef enum _gce2D_COMMAND {
    gcv2D_CLEAR = 0,
    gcv2D_LINE,
    gcv2D_BLT,
    gcv2D_STRETCH,
    gcv2D_HOR_FILTER,
    gcv2D_VER_FILTER,
    gcv2D_MULTI_SOURCE_BLT,
    gcv2D_FILTER_BLT,
} gce2D_COMMAND;

typedef enum _gce2D_TILE_STATUS_CONFIG {
    gcv2D_TSC_DISABLE       = 0,
    gcv2D_TSC_ENABLE        = 0x00000001,
    gcv2D_TSC_COMPRESSED    = 0x00000002,
    gcv2D_TSC_DOWN_SAMPLER  = 0x00000004,
    gcv2D_TSC_2D_COMPRESSED = 0x00000008,

    gcv2D_TSC_DEC_COMPRESSED     = 0x00000020,
    gcv2D_TSC_DEC_TPC            = 0x00000040,
    gcv2D_TSC_DEC_TPC_COMPRESSED = 0x00000080,

    gcv2D_TSC_V4_COMPRESSED      = 0x00000100,
    gcv2D_TSC_V4_COMPRESSED_256B = 0x00000200 | gcv2D_TSC_V4_COMPRESSED,

    gcv2D_TSC_DEC_TPC_TILED            = gcv2D_TSC_DEC_COMPRESSED | gcv2D_TSC_DEC_TPC,
    gcv2D_TSC_DEC_TPC_TILED_COMPRESSED = gcv2D_TSC_DEC_TPC_TILED | gcv2D_TSC_DEC_TPC_COMPRESSED,

    gcv2D_TSC_TPC_COMPRESSED     = 0x00001000,
    gcv2D_TSC_TPC_COMPRESSED_V10 = gcv2D_TSC_TPC_COMPRESSED | 0x00000400,
    gcv2D_TSC_TPC_COMPRESSED_V11 = gcv2D_TSC_TPC_COMPRESSED | 0x00000800,
} gce2D_TILE_STATUS_CONFIG;

typedef enum _gce2D_DEC400_MINOR_VERSION {
    gcv2D_DEC400_MINOR_V1 = 1,
    gcv2D_DEC400_MINOR_V2 = 2,
    gcv2D_DEC400_MINOR_V3 = 3,
    gcv2D_DEC400_MINOR_V4 = 4,
} gce2D_DEC400_MINOR_VERSION;

/*Tiling version when disable compression*/
typedef enum _gce2D_TILING_MINOR_VERSION {
    gcv2D_TILING_MINOR_V1 = 0,
    gcv2D_TILING_MINOR_V2 = 1,
} gce2D_TILING_MINOR_VERSION;

typedef enum _gce2D_QUERY {
    gcv2D_QUERY_RGB_ADDRESS_MIN_ALIGN = 0,
    gcv2D_QUERY_RGB_STRIDE_MIN_ALIGN,
    gcv2D_QUERY_YUV_ADDRESS_MIN_ALIGN,
    gcv2D_QUERY_YUV_STRIDE_MIN_ALIGN,
    gcv2D_QUERY_DEC400_MINOR_VERSION,
    gcv2D_QUERY_TILING_MINOR_VERSION,
} gce2D_QUERY;

typedef enum _gce2D_SUPER_TILE_VERSION {
    gcv2D_SUPER_TILE_VERSION_V1 = 1,
    gcv2D_SUPER_TILE_VERSION_V2 = 2,
    gcv2D_SUPER_TILE_VERSION_V3 = 3,
} gce2D_SUPER_TILE_VERSION;

typedef enum _gce2D_STATE {
    gcv2D_STATE_SPECIAL_FILTER_MIRROR_MODE = 1,
    gcv2D_STATE_SUPER_TILE_VERSION,
    gcv2D_STATE_EN_GAMMA,
    gcv2D_STATE_DE_GAMMA,
    gcv2D_STATE_MULTI_SRC_BLIT_UNIFIED_DST_RECT,
    gcv2D_STATE_MULTI_SRC_BLIT_BILINEAR_FILTER,
    gcv2D_STATE_PROFILE_ENABLE,
    gcv2D_STATE_XRGB_ENABLE,

    gcv2D_STATE_ARRAY_EN_GAMMA      = 0x10001,
    gcv2D_STATE_ARRAY_DE_GAMMA,
    gcv2D_STATE_ARRAY_CSC_YUV_TO_RGB,
    gcv2D_STATE_ARRAY_CSC_RGB_TO_YUV,

    gcv2D_STATE_DEC_TPC_NV12_10BIT  = 0x20001,
    gcv2D_STATE_ARRAY_YUV_SRC_TILE_STATUS_ADDR,
    gcv2D_STATE_ARRAY_YUV_DST_TILE_STATUS_ADDR,
} gce2D_STATE;

typedef enum _gce2D_STATE_PROFILE {
    gcv2D_STATE_PROFILE_NONE    = 0x0,
    gcv2D_STATE_PROFILE_COMMAND = 0x1,
    gcv2D_STATE_PROFILE_SURFACE = 0x2,
    gcv2D_STATE_PROFILE_ALL     = 0xFFFF,
} gce2D_STATE_PROFILE;

/* Texture object types */
typedef enum _gceTEXTURE_TYPE {
    gcvTEXTURE_UNKNOWN = 0,
    gcvTEXTURE_1D,
    gcvTEXTURE_2D,
    gcvTEXTURE_3D,
    gcvTEXTURE_CUBEMAP,
    gcvTEXTURE_1D_ARRAY,
    gcvTEXTURE_2D_ARRAY,
    gcvTEXTURE_2D_MS,
    gcvTEXTURE_2D_MS_ARRAY,
    gcvTEXTURE_CUBEMAP_ARRAY,
    gcvTEXTURE_EXTERNAL

} gceTEXTURE_TYPE;

#if gcdENABLE_3D

/* Texture functions. */
typedef enum _gceTEXTURE_FUNCTION {
    gcvTEXTURE_DUMMY   = 0,
    gcvTEXTURE_REPLACE = 0,
    gcvTEXTURE_MODULATE,
    gcvTEXTURE_ADD,
    gcvTEXTURE_ADD_SIGNED,
    gcvTEXTURE_INTERPOLATE,
    gcvTEXTURE_SUBTRACT,
    gcvTEXTURE_DOT3

} gceTEXTURE_FUNCTION;

/* Texture sources. */
typedef enum _gceTEXTURE_SOURCE {
    gcvCOLOR_FROM_TEXTURE = 0,
    gcvCOLOR_FROM_CONSTANT_COLOR,
    gcvCOLOR_FROM_PRIMARY_COLOR,
    gcvCOLOR_FROM_PREVIOUS_COLOR

} gceTEXTURE_SOURCE;

/* Texture source channels. */
typedef enum _gceTEXTURE_CHANNEL {
    gcvFROM_COLOR = 0,
    gcvFROM_ONE_MINUS_COLOR,
    gcvFROM_ALPHA,
    gcvFROM_ONE_MINUS_ALPHA

} gceTEXTURE_CHANNEL;

#endif /* gcdENABLE_3D */


/* Filter types. */
typedef enum _gceFILTER_TYPE {
    gcvFILTER_SYNC = 0,
    gcvFILTER_BLUR,
    gcvFILTER_USER

} gceFILTER_TYPE;

/* Filter pass types. */
typedef enum _gceFILTER_PASS_TYPE {
    gcvFILTER_HOR_PASS = 0,
    gcvFILTER_VER_PASS

} gceFILTER_PASS_TYPE;

/* Endian hints. */
typedef enum _gceENDIAN_HINT {
    gcvENDIAN_NO_SWAP    = 0,
    gcvENDIAN_SWAP_WORD  = 1,
    gcvENDIAN_SWAP_DWORD = 2,
    gcvENDIAN_SWAP_QWORD = 3,
} gceENDIAN_HINT;

/* Tiling modes. */
typedef enum _gceTILING {
    gcvINVALIDTILED = 0x0,      /* Invalid tiling */
    /* Tiling basic modes enum'ed in power of 2. */
    gcvLINEAR       = 0x1,      /* No    tiling. */
    gcvTILED        = 0x2,      /* 4x4   tiling. */
    gcvSUPERTILED   = 0x4,      /* 64x64 tiling. */
    gcvMINORTILED   = 0x8,      /* 2x2   tiling. */


    /* Tiling special layouts. */
    gcvTILING_SPLIT_BUFFER = 0x10,
    gcvTILING_X_MAJOR      = 0x20,
    gcvTILING_Y_MAJOR      = 0x40,
    gcvTILING_SWAP         = 0x80,

    /* Tiling combination layouts. */
    gcvMULTI_TILED       = gcvTILED      | gcvTILING_SPLIT_BUFFER,

    gcvMULTI_SUPERTILED  = gcvSUPERTILED | gcvTILING_SPLIT_BUFFER,

    gcvYMAJOR_SUPERTILED = gcvSUPERTILED | gcvTILING_Y_MAJOR,

    gcvTILED_8X4  = 0x0100,
    gcvTILED_4X8  = 0x0100 | gcvTILING_SWAP,
    gcvTILED_8X8  = 0x0200,
    gcvTILED_16X4 = 0x0400,
    gcvTILED_32X4 = 0x0800,
    gcvTILED_64X4 = 0x1000,

    gcvTILED_8X8_XMAJOR = gcvTILED_8X8 | gcvTILING_X_MAJOR,
    gcvTILED_8X8_YMAJOR = gcvTILED_8X8 | gcvTILING_Y_MAJOR,

    gcvSUPERTILED_128B = 0x10000 | gcvSUPERTILED,
    gcvSUPERTILED_256B = 0x20000 | gcvSUPERTILED,
} gceTILING;

typedef enum _gceCACHE_MODE {
    gcvCACHE_NONE,
    gcvCACHE_128,
    gcvCACHE_256,
} gceCACHE_MODE;

#define DEFAULT_CACHE_MODE gcvCACHE_256



/* 2D pattern type. */
typedef enum _gce2D_PATTERN {
    gcv2D_PATTERN_SOLID = 0,
    gcv2D_PATTERN_MONO,
    gcv2D_PATTERN_COLOR,
    gcv2D_PATTERN_INVALID

} gce2D_PATTERN;

/* 2D source type. */
typedef enum _gce2D_SOURCE {
    gcv2D_SOURCE_MASKED = 0,
    gcv2D_SOURCE_MONO,
    gcv2D_SOURCE_COLOR,
    gcv2D_SOURCE_INVALID

} gce2D_SOURCE;

typedef enum _gceMMU_MODE {
    gcvMMU_MODE_1K,
    gcvMMU_MODE_4K,
} gceMMU_MODE;

/* gcdDUMP message type. */
typedef enum _gceDEBUG_MESSAGE_TYPE {
    gcvMESSAGE_TEXT,
    gcvMESSAGE_DUMP

} gceDEBUG_MESSAGE_TYPE;

/* Shading format. */
typedef enum _gceSHADING {
    gcvSHADING_SMOOTH,
    gcvSHADING_FLAT_D3D,
    gcvSHADING_FLAT_OPENGL,
} gceSHADING;

/* Culling modes. */
typedef enum _gceCULL {
    gcvCULL_NONE,
    gcvCULL_CCW,
    gcvCULL_CW,
} gceCULL;

/* Fill modes. */
typedef enum _gceFILL {
    gcvFILL_POINT,
    gcvFILL_WIRE_FRAME,
    gcvFILL_SOLID,
} gceFILL;

/* FrontFace modes. */
typedef enum _gceFRONTFACE {
    gcvFRONTFACE_CW,
    gcvFRONTFACE_CCW,
} gceFRONTFACE;

/* PolygonOffset modes. */
typedef enum _gcePOLYGONOFFSETMODE {
    gcvPOLYGONOFFSET_POINT  = 1,
    gcvPOLYGONOFFSET_LINE   = 2,
    gcvPOLYGONOFFSET_FILL   = 4,
} gcePOLYGONOFFSETMODE;

/* Compare modes. */
typedef enum _gceCOMPARE {
    gcvCOMPARE_INVALID = 0,
    gcvCOMPARE_NEVER,
    gcvCOMPARE_NOT_EQUAL,
    gcvCOMPARE_LESS,
    gcvCOMPARE_LESS_OR_EQUAL,
    gcvCOMPARE_EQUAL,
    gcvCOMPARE_GREATER,
    gcvCOMPARE_GREATER_OR_EQUAL,
    gcvCOMPARE_ALWAYS,
} gceCOMPARE;

/* Stencil modes. */
typedef enum _gceSTENCIL_MODE {
    gcvSTENCIL_NONE,
    gcvSTENCIL_SINGLE_SIDED,
    gcvSTENCIL_DOUBLE_SIDED,
} gceSTENCIL_MODE;

/* Stencil operations. */
typedef enum _gceSTENCIL_OPERATION {
    gcvSTENCIL_KEEP,
    gcvSTENCIL_REPLACE,
    gcvSTENCIL_ZERO,
    gcvSTENCIL_INVERT,
    gcvSTENCIL_INCREMENT,
    gcvSTENCIL_DECREMENT,
    gcvSTENCIL_INCREMENT_SATURATE,
    gcvSTENCIL_DECREMENT_SATURATE,
    gcvSTENCIL_OPERATION_INVALID = -1

} gceSTENCIL_OPERATION;

/* Stencil selection. */
typedef enum _gceSTENCIL_WHERE {
    gcvSTENCIL_FRONT,
    gcvSTENCIL_BACK,
} gceSTENCIL_WHERE;

/* Texture addressing selection. */
typedef enum _gceTEXTURE_WHICH {
    gcvTEXTURE_S,
    gcvTEXTURE_T,
    gcvTEXTURE_R,
} gceTEXTURE_WHICH;

/* Texture addressing modes. */
typedef enum _gceTEXTURE_ADDRESSING {
    gcvTEXTURE_INVALID = 0,
    gcvTEXTURE_CLAMP,
    gcvTEXTURE_WRAP,
    gcvTEXTURE_MIRROR,
    gcvTEXTURE_BORDER,
    gcvTEXTURE_MIRROR_ONCE,
} gceTEXTURE_ADDRESSING;

/* Texture filters. */
typedef enum _gceTEXTURE_FILTER {
    gcvTEXTURE_NONE,
    gcvTEXTURE_POINT,
    gcvTEXTURE_LINEAR,
    gcvTEXTURE_ANISOTROPIC,
} gceTEXTURE_FILTER;

typedef enum _gceTEXTURE_COMPONENT {
    gcvTEXTURE_COMPONENT_R,
    gcvTEXTURE_COMPONENT_G,
    gcvTEXTURE_COMPONENT_B,
    gcvTEXTURE_COMPONENT_A,

    gcvTEXTURE_COMPONENT_NUM,
} gceTEXTURE_COMPONENT;

/* Texture swizzle modes. */
typedef enum _gceTEXTURE_SWIZZLE {
    gcvTEXTURE_SWIZZLE_R = 0,
    gcvTEXTURE_SWIZZLE_G,
    gcvTEXTURE_SWIZZLE_B,
    gcvTEXTURE_SWIZZLE_A,
    gcvTEXTURE_SWIZZLE_0,
    gcvTEXTURE_SWIZZLE_1,

    gcvTEXTURE_SWIZZLE_INVALID,
} gceTEXTURE_SWIZZLE;

typedef enum _gceTEXTURE_SRGBDECODE {
    gcvTEXTURE_SRGB_INVALID = 0,
    gcvTEXTURE_DECODE,
    gcvTEXTURE_SKIP_DECODE,
} gceTEXTURE_SRGBDECODE;

typedef enum _gceTEXTURE_COMPARE_MODE {
    gcvTEXTURE_COMPARE_MODE_INVALID = 0,
    gcvTEXTURE_COMPARE_MODE_NONE,
    gcvTEXTURE_COMPARE_MODE_REF,
} gceTEXTURE_COMPARE_MODE;

typedef enum _gceTEXTURE_DS_MODE {
    gcvTEXTURE_DS_MODE_INVALID = 0,
    gcvTEXTURE_DS_MODE_DEPTH   = 1,
    gcvTEXTURE_DS_MODE_STENCIL = 2,
} gceTEXTURE_DS_MODE;

typedef enum _gceTEXTURE_DS_TEX_MODE {
    gcvTEXTURE_DS_TEXTURE_MODE_LUMINANCE = 0,
    gcvTEXTURE_DS_TEXTURE_MODE_INTENSITY,
    gcvTEXTURE_DS_TEXTURE_MODE_ALPHA,
    gcvTEXTURE_DS_TEXTURE_MODE_RED,

    gcvTEXTURE_DS_TEXTURE_MODE_INVALID,
} gceTEXTURE_DS_TEX_MODE;

/* Texture stage */
typedef enum _gceTEXTURE_STAGE {
    gcvTEXTURE_STAGE_INVALID = -1,
    gcvTEXTURE_STAGE_VS      = 0,
    gcvTEXTURE_STAGE_TCS,
    gcvTEXTURE_STAGE_TES,
    gcvTEXTURE_STAGE_GS,
    gcvTEXTURE_STAGE_FS,
    gcvTEXTURE_STAGE_CS,

    gcvTEXTURE_STAGE_LAST

} gceTEXTURE_STAGE;

/* Pixel output swizzle modes. */
typedef enum _gcePIXEL_SWIZZLE {
    gcvPIXEL_SWIZZLE_R = gcvTEXTURE_SWIZZLE_R,
    gcvPIXEL_SWIZZLE_G = gcvTEXTURE_SWIZZLE_G,
    gcvPIXEL_SWIZZLE_B = gcvTEXTURE_SWIZZLE_B,
    gcvPIXEL_SWIZZLE_A = gcvTEXTURE_SWIZZLE_A,

    gcvPIXEL_SWIZZLE_INVALID,
} gcePIXEL_SWIZZLE;

/* Primitive types. */
typedef enum _gcePRIMITIVE {
    gcvPRIMITIVE_POINT_LIST,
    gcvPRIMITIVE_LINE_LIST,
    gcvPRIMITIVE_LINE_STRIP,
    gcvPRIMITIVE_LINE_LOOP,
    gcvPRIMITIVE_TRIANGLE_LIST,
    gcvPRIMITIVE_TRIANGLE_STRIP,
    gcvPRIMITIVE_TRIANGLE_FAN,
    gcvPRIMITIVE_RECTANGLE,
    gcvPRIMITIVE_LINES_ADJACENCY,
    gcvPRIMITIVE_LINE_STRIP_ADJACENCY,
    gcvPRIMITIVE_TRIANGLES_ADJACENCY,
    gcvPRIMITIVE_TRIANGLE_STRIP_ADJACENCY,
    gcvPRIMITIVE_PATCH_LIST,
} gcePRIMITIVE;

/* Index types. */
typedef enum _gceINDEX_TYPE {
    gcvINDEX_8,
    gcvINDEX_16,
    gcvINDEX_32,
} gceINDEX_TYPE;

/* Multi GPU rendering modes. */
typedef enum _gceMULTI_GPU_RENDERING_MODE {
    gcvMULTI_GPU_RENDERING_MODE_OFF,
    gcvMULTI_GPU_RENDERING_MODE_SPLIT_WIDTH,
    gcvMULTI_GPU_RENDERING_MODE_SPLIT_HEIGHT,
    gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_64x64,
    gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x64,
    gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED_128x128,
    gcvMULTI_GPU_RENDERING_MODE_INTERLEAVED,
    gcvMULTI_GPU_RENDERING_MODE_INVALID

} gceMULTI_GPU_RENDERING_MODE;

typedef enum _gceMACHINECODE {
    gcvMACHINECODE_ANTUTU0 = 0x0,

    gcvMACHINECODE_GLB27_RELEASE_0,

    gcvMACHINECODE_GLB25_RELEASE_0,
    gcvMACHINECODE_GLB25_RELEASE_1,

    /* keep it as the last enum */
    gcvMACHINECODE_COUNT

} gceMACHINECODE;

typedef enum _gceUNIFORMCVT {
    gcvUNIFORMCVT_NONE = 0,
    gcvUNIFORMCVT_TO_BOOL,
    gcvUNIFORMCVT_TO_FLOAT,
} gceUNIFORMCVT;

typedef enum _gceHAL_ARG_VERSION {
    gcvHAL_ARG_VERSION_V1 = 0x0,
    gcvHAL_ARG_VERSION_V2,
} gceHAL_ARG_VERSION;




/** endian mode  for each 2Bytes

 * endian mode                        endian

 *    mode0: 0  1  2  3  4  5  6  7  8  9  10  11  12  13  14  15

 *    mode1: 1  0  3  2  5  4  7  6  9  8  11  10  13  12  15  14

 *    mode2: 2  3  0  1  6  7  4  5  10  11  8  9  14  15  12  13

 *    mode3: 3  2  1  0  7  6  5  4  11  10  9  8  15  14  13  12

 *    mode4: 12  13  14  15  8  9  10  11  4  5  6  7  0  1  2  3

 *    mode5: 13  12  15  14  9  8  11  10  5  4  7  6  1  0  3  2

 *    mode6: 14  15  12  13  10  11  8  9  6  7  4  5  2  3  0  1

 *    mode7: 15  14  13  12  11  10  9  8  7  6  5  4  3  2  1  0
 **/
typedef enum _gceENDIAN_MODE {
    gcvENDIAN_MODE0     = 0x0, /* endian mode0 */
    gcvENDIAN_MODE1     = 0x1, /* endian mode1 */
    gcvENDIAN_MODE2     = 0x2, /* endian mode2 */
    gcvENDIAN_MODE3     = 0x3, /* endian mode3 */
    gcvENDIAN_MODE4     = 0x4, /* endian mode4 */
    gcvENDIAN_MODE5     = 0x5, /* endian mode5 */
    gcvENDIAN_MODE6     = 0x6, /* endian mode6 */
    gcvENDIAN_MODE7     = 0x7, /* endian mode7 */
} gceENDIAN_MODE;

typedef enum _gceHW_FE_TYPE {
    gcvHW_FE_WAIT_LINK,
    gcvHW_FE_ASYNC,
    gcvHW_FE_MULTI_CHANNEL,
    gcvHW_FE_END,
} gceHW_FE_TYPE;

typedef enum _gceMCFE_CHANNEL_TYPE {
    gcvMCFE_CHANNEL_NONE = 0,
    gcvMCFE_CHANNEL_SYSTEM,
    gcvMCFE_CHANNEL_SHADER,
    gcvMCFE_CHANNEL_NN,
    gcvMCFE_CHANNEL_TP,

    gcvMCFE_CHANNEL_3DBLIT = 128,
} gceMCFE_CHANNEL_TYPE;

typedef enum _gcePAGE_TYPE {
    gcvPAGE_TYPE_4K,
    gcvPAGE_TYPE_64K,
    gcvPAGE_TYPE_1M,
    gcvPAGE_TYPE_2M,
    gcvPAGE_TYPE_16M,
} gcePAGE_TYPE;

typedef enum _gceAREA_TYPE {
    gcvAREA_TYPE_UNKNOWN = 0,
    gcvAREA_TYPE_FLATMAP,
    gcvAREA_TYPE_64K,
    gcvAREA_TYPE_1M,
    gcvAREA_TYPE_2M,
    gcvAREA_TYPE_4K,
    gcvAREA_TYPE_LOW_4K,
} gceAREA_TYPE;

typedef enum _gce2D_U8ToU10_CONVERSION_MODE

{
    gcvADD_LOWER_BITS,
    gcvNOT_ADD_LOWER_BITS,
    gcvNOT_ADD_HIGHER_BITS

} gce2D_U8ToU10_CONVERSION_MODE;

typedef enum _gce2D_NORMALIZATION_MODE

{
    gcvNORMALIZATION_STD_Z_SCORE,
    gcvNORMALIZATION_Z_SCORE,
    gcvNORMALIZATION_MIN_MAX,
} gce2D_NORMALIZATION_MODE;

typedef enum _gce2D_MULTICORE_MODE

{
    gcvMULTICORE_COMBINED_MODE = 0,
    gcvMULTICORE_INDEPENDENT_MODE

}gce2D_MULTICORE_MODE;

typedef enum _gce2D_MULTICORE_INDEX

{
    gcv2D_CORE_0 = 0,
    gcv2D_CORE_1,
    gcv2D_CORE_2,
    gcv2D_CORE_3,
}gce2D_MULTICORE_INDEX;

/* Available enumeration values that can be used to set 2D state */
typedef enum _gce2D_STATE_KEY

{
    gcvNORMALIZATION_ENABLE = 0,
    gcvNORMALIZATION_MODE,
    gcvNORMALIZATION_MAXMINRECIPROCAL,
    gcvNORMALIZATION_MINVALUE,
    gcvNORMALIZATION_MEANVALUE,
    gcvNORMALIZATION_STDRECIPROCAL,
    gcvQUANTIZATION_STEPRECIPROCAL,
    gcvQUANTIZATION_BYPASSSTEPQUANTIZATION,
    gcvUINT8_2_UINT10_CONVERSION_MODE,
    gcvMulticoreMode,

    gcvSTATE_TAIL,
} gce2D_STATE_KEY;

/*******************************************************************************
 ** Broadcast interface.

 */

typedef enum _gceBROADCAST {
    /* GPU might be idle. */
    gcvBROADCAST_GPU_IDLE,

    /* A commit is going to happen. */
    gcvBROADCAST_GPU_COMMIT,

    /* GPU seems to be stuck. */
    gcvBROADCAST_GPU_STUCK,

    /* First process gets attached. */
    gcvBROADCAST_FIRST_PROCESS,

    /* Last process gets detached. */
    gcvBROADCAST_LAST_PROCESS,

    /* AXI bus error. */
    gcvBROADCAST_AXI_BUS_ERROR,

    /* Out of memory. */
    gcvBROADCAST_OUT_OF_MEMORY,
} gceBROADCAST;

/* Notifications. */
typedef enum _gceNOTIFY {
    gcvNOTIFY_INTERRUPT,
    gcvNOTIFY_COMMAND_QUEUE,
} gceNOTIFY;

/* Flush flags. */
typedef enum _gceKERNEL_FLUSH {
    gcvFLUSH_COLOR              = 0x01,
    gcvFLUSH_DEPTH              = 0x02,
    gcvFLUSH_TEXTURE            = 0x04,
    gcvFLUSH_2D                 = 0x08,
    gcvFLUSH_L2                 = 0x10,
    gcvFLUSH_TILE_STATUS        = 0x20,
    gcvFLUSH_ICACHE             = 0x40,
    gcvFLUSH_TXDESC             = 0x80,
    gcvFLUSH_FENCE              = 0x100,
    gcvFLUSH_VERTEX             = 0x200,
    gcvFLUSH_TFBHEADER          = 0x400,
    gcvFLUSH_ALL                = gcvFLUSH_COLOR

                                | gcvFLUSH_DEPTH

                                | gcvFLUSH_TEXTURE

                                | gcvFLUSH_2D

                                | gcvFLUSH_L2

                                | gcvFLUSH_TILE_STATUS

                                | gcvFLUSH_ICACHE

                                | gcvFLUSH_TXDESC

                                | gcvFLUSH_FENCE

                                | gcvFLUSH_VERTEX

                                | gcvFLUSH_TFBHEADER

} gceKERNEL_FLUSH;

typedef enum _gceCOUNTER {
    gcvCOUNTER_HOST_INTERFACE0,
    gcvCOUNTER_HOST_INTERFACE1,
    gcvCOUNTER_COUNT

} gceCOUNTER;

typedef enum _gceProfilerClient {
    gcvCLIENT_OPENGLES11 = 1,
    gcvCLIENT_OPENGLES,
    gcvCLIENT_OPENGL,
    gcvCLIENT_OPENVG,
    gcvCLIENT_OPENCL,
    gcvCLIENT_OPENVX,
    gcvCLIENT_OPENVK,
} gceProfilerClient;

typedef enum _gceCOUNTER_OPTYPE {
    gcvCOUNTER_OP_DRAW      = 0,
    gcvCOUNTER_OP_BLT       = 1,
    gcvCOUNTER_OP_COMPUTE   = 2,
    gcvCOUNTER_OP_RS        = 3,
    gcvCOUNTER_OP_FINISH    = 4,
    gcvCOUNTER_OP_FRAME     = 5,
    gcvCOUNTER_OP_OPERATION = 6,
    gcvCOUNTER_OP_NONE      = 7

}

gceCOUNTER_OPTYPE;

typedef enum _gceProbeStatus {
    gcvPROBE_Disabled = 0,
    gcvPROBE_Paused   = 1,
    gcvPROBE_Enabled  = 2,
} gceProbeStatus;

typedef enum _gceProbeCmd {
    gcvPROBECMD_BEGIN  = 0,
    gcvPROBECMD_PAUSE  = 1,
    gcvPROBECMD_RESUME = 2,
    gcvPROBECMD_END    = 3,
} gceProbeCmd;

/*******************************************************************************

 * Events. *********************************************************************

 */

typedef enum _halEventType {
    /* Keyboard event. */
    HAL_KEYBOARD,

    /* Mouse move event. */
    HAL_POINTER,

    /* Mouse button event. */
    HAL_BUTTON,

    /* Application close event. */
    HAL_CLOSE,

    /* Application window has been updated. */
    HAL_WINDOW_UPDATE

} halEventType;

/* Scancodes for keyboard. */
typedef enum _halKeys {
    HAL_UNKNOWN = -1,

    HAL_BACKSPACE = 0x08,
    HAL_TAB,
    HAL_ENTER  = 0x0D,
    HAL_ESCAPE = 0x1B,

    HAL_SPACE        = 0x20,
    HAL_SINGLEQUOTE  = 0x27,
    HAL_PAD_ASTERISK = 0x2A,
    HAL_COMMA        = 0x2C,
    HAL_HYPHEN,
    HAL_PERIOD,
    HAL_SLASH,
    HAL_0,
    HAL_1,
    HAL_2,
    HAL_3,
    HAL_4,
    HAL_5,
    HAL_6,
    HAL_7,
    HAL_8,
    HAL_9,
    HAL_SEMICOLON = 0x3B,
    HAL_EQUAL     = 0x3D,
    HAL_A         = 0x41,
    HAL_B,
    HAL_C,
    HAL_D,
    HAL_E,
    HAL_F,
    HAL_G,
    HAL_H,
    HAL_I,
    HAL_J,
    HAL_K,
    HAL_L,
    HAL_M,
    HAL_N,
    HAL_O,
    HAL_P,
    HAL_Q,
    HAL_R,
    HAL_S,
    HAL_T,
    HAL_U,
    HAL_V,
    HAL_W,
    HAL_X,
    HAL_Y,
    HAL_Z,
    HAL_LBRACKET,
    HAL_BACKSLASH,
    HAL_RBRACKET,
    HAL_BACKQUOTE = 0x60,

    HAL_F1 = 0x80,
    HAL_F2,
    HAL_F3,
    HAL_F4,
    HAL_F5,
    HAL_F6,
    HAL_F7,
    HAL_F8,
    HAL_F9,
    HAL_F10,
    HAL_F11,
    HAL_F12,

    HAL_LCTRL,
    HAL_RCTRL,
    HAL_LSHIFT,
    HAL_RSHIFT,
    HAL_LALT,
    HAL_RALT,
    HAL_CAPSLOCK,
    HAL_NUMLOCK,
    HAL_SCROLLLOCK,
    HAL_PAD_0,
    HAL_PAD_1,
    HAL_PAD_2,
    HAL_PAD_3,
    HAL_PAD_4,
    HAL_PAD_5,
    HAL_PAD_6,
    HAL_PAD_7,
    HAL_PAD_8,
    HAL_PAD_9,
    HAL_PAD_HYPHEN,
    HAL_PAD_PLUS,
    HAL_PAD_SLASH,
    HAL_PAD_PERIOD,
    HAL_PAD_ENTER,
    HAL_SYSRQ,
    HAL_PRNTSCRN,
    HAL_BREAK,
    HAL_UP,
    HAL_LEFT,
    HAL_RIGHT,
    HAL_DOWN,
    HAL_HOME,
    HAL_END,
    HAL_PGUP,
    HAL_PGDN,
    HAL_INSERT,
    HAL_DELETE,
    HAL_LWINDOW,
    HAL_RWINDOW,
    HAL_MENU,
    HAL_POWER,
    HAL_SLEEP,
    HAL_WAKE

} halKeys;

/*!

 * @brief Command codes between kernel module and TrustZone

 * @discussion

 * Critical services must be done in TrustZone to avoid sensitive content leak.

 * Most of kernel module is kept in non-Secure os to minimize code in TrustZone.

 */
typedef enum kernel_packet_command {
    KERNEL_START_COMMAND,
    KERNEL_SUBMIT,
    KERNEL_MAP_MEMORY,                  /* */
    KERNEL_UNMAP_MEMORY,
    KERNEL_ALLOCATE_SECRUE_MEMORY,      /*! Security memory management. */
    KERNEL_FREE_SECURE_MEMORY,
    KERNEL_EXECUTE,                     /* Execute a command buffer. */
    KERNEL_DUMP_MMU_EXCEPTION,
    KERNEL_HANDLE_MMU_EXCEPTION,
    KERNEL_READ_MMU_EXCEPTION,
} kernel_packet_command_t;

enum {
    gcvTA_COMMAND_INIT,
    gcvTA_COMMAND_DISPATCH,

    gcvTA_CALLBACK_ALLOC_SECURE_MEM,
    gcvTA_CALLBACK_FREE_SECURE_MEM,
};

typedef enum {
    gcvFENCE_TYPE_READ          = 0x1,
    gcvFENCE_TYPE_WRITE         = 0x2,
    gcvFENCE_TYPE_ALL           = gcvFENCE_TYPE_READ | gcvFENCE_TYPE_WRITE,
    gcvFNECE_TYPE_INVALID       = 0x10000,
} gceFENCE_TYPE;

typedef enum _gceTLS_KEY {
    gcvTLS_KEY_EGL,
    gcvTLS_KEY_OPENGL_ES,
    gcvTLS_KEY_OPENVG,
    gcvTLS_KEY_OPENGL,
    gcvTLS_KEY_OPENCL,
    gcvTLS_KEY_OPENVX,

    gcvTLS_KEY_COUNT

} gceTLS_KEY;

typedef enum _gcePLS_VALUE {
    gcePLS_VALUE_EGL_DISPLAY_INFO,
    gcePLS_VALUE_EGL_CONFIG_FORMAT_INFO,
    gcePLS_VALUE_EGL_DESTRUCTOR_INFO,
    gcePLS_VALUE_OPENCL_DESTRUCTOR_INFO,
} gcePLS_VALUE;

#if gcdENABLE_3D

/* Blending functions. */
typedef enum _gceBLEND_FUNCTION {
    gcvBLEND_ZERO,
    gcvBLEND_ONE,
    gcvBLEND_SOURCE_COLOR,
    gcvBLEND_INV_SOURCE_COLOR,
    gcvBLEND_SOURCE_ALPHA,
    gcvBLEND_INV_SOURCE_ALPHA,
    gcvBLEND_TARGET_COLOR,
    gcvBLEND_INV_TARGET_COLOR,
    gcvBLEND_TARGET_ALPHA,
    gcvBLEND_INV_TARGET_ALPHA,
    gcvBLEND_SOURCE_ALPHA_SATURATE,
    gcvBLEND_CONST_COLOR,
    gcvBLEND_INV_CONST_COLOR,
    gcvBLEND_CONST_ALPHA,
    gcvBLEND_INV_CONST_ALPHA,
    gcvBLEND_SRC1_COLOR,
    gcvBLEND_ONE_MINUS_SRC1_COLOR,
    gcvBLEND_SRC1_ALPHA,
    gcvBLEND_ONE_MINUS_SRC1_ALPHA,
} gceBLEND_FUNCTION;

/* Blending modes. */
typedef enum _gceBLEND_MODE {
    gcvBLEND_ADD = 0,
    gcvBLEND_SUBTRACT,
    gcvBLEND_REVERSE_SUBTRACT,
    gcvBLEND_MIN,
    gcvBLEND_MAX,
    gcvBLEND_MULTIPLY,
    gcvBLEND_SCREEN,
    gcvBLEND_OVERLAY,
    gcvBLEND_DARKEN,
    gcvBLEND_LIGHTEN,
    gcvBLEND_COLORDODGE,
    gcvBLEND_COLORBURN,
    gcvBLEND_HARDLIGHT,
    gcvBLEND_SOFTLIGHT,
    gcvBLEND_DIFFERENCE,
    gcvBLEND_EXCLUSION,
    gcvBLEND_HSL_HUE,
    gcvBLEND_HSL_SATURATION,
    gcvBLEND_HSL_COLOR,
    gcvBLEND_HSL_LUMINOSITY,

    gcvBLEND_TOTAL

} gceBLEND_MODE;

/* Depth modes. */
typedef enum _gceDEPTH_MODE {
    gcvDEPTH_NONE,
    gcvDEPTH_Z,
    gcvDEPTH_W,
} gceDEPTH_MODE;

#endif /* gcdENABLE_3D */


/* API flags. */
typedef enum _gceAPI {
    gcvAPI_D3D = 1,
    gcvAPI_OPENGL_ES11,
    gcvAPI_OPENGL_ES20,
    gcvAPI_OPENGL_ES30,
    gcvAPI_OPENGL_ES31,
    gcvAPI_OPENGL_ES32,
    gcvAPI_OPENGL,
    gcvAPI_OPENVG,
    gcvAPI_OPENCL,
    gcvAPI_OPENVK,
    gcvAPI_EGL,
    gcvAPI_COUNT,
} gceAPI;

typedef enum _gceWHERE {
    gcvWHERE_COMMAND_PREFETCH = 0,
    gcvWHERE_COMMAND,
    gcvWHERE_RASTER,
    gcvWHERE_PIXEL,
    gcvWHERE_BLT,
} gceWHERE;

typedef enum _gceHOW {
    gcvHOW_SEMAPHORE            = 0x1,
    gcvHOW_STALL                = 0x2,
    gcvHOW_SEMAPHORE_STALL      = 0x3,
} gceHOW;

typedef enum _gceSignalHandlerType {
    gcvHANDLE_SIGFPE_WHEN_SIGNAL_CODE_IS_0 = 0x1,
} gceSignalHandlerType;

typedef enum _gceFILE_MODE {
    gcvFILE_CREATE = 0,
    gcvFILE_APPEND,
    gcvFILE_READ,
    gcvFILE_CREATETEXT,
    gcvFILE_APPENDTEXT,
    gcvFILE_READTEXT,
    gcvFILE_READWRITE,
} gceFILE_MODE;

typedef enum _gceFILE_WHENCE {
    gcvFILE_SEEK_SET,
    gcvFILE_SEEK_CUR,
    gcvFILE_SEEK_END

} gceFILE_WHENCE;

/* Color format classes. */
typedef enum _gceFORMAT_CLASS {
    gcvFORMAT_CLASS_RGBA = 4500,
    gcvFORMAT_CLASS_YUV,
    gcvFORMAT_CLASS_INDEX,
    gcvFORMAT_CLASS_LUMINANCE,
    gcvFORMAT_CLASS_BUMP,
    gcvFORMAT_CLASS_DEPTH,
    gcvFORMAT_CLASS_ASTC,
    gcvFORMAT_CLASS_COMPRESSED,
    gcvFORMAT_CLASS_OTHER,
    gcvFORMAT_CLASS_INTENSITY

} gceFORMAT_CLASS;

/* Color format data type */
typedef enum _gceFORMAT_DATATYPE {
    gcvFORMAT_DATATYPE_UNSIGNED_NORMALIZED,
    gcvFORMAT_DATATYPE_SIGNED_NORMALIZED,
    gcvFORMAT_DATATYPE_UNSIGNED_INTEGER,
    gcvFORMAT_DATATYPE_SIGNED_INTEGER,
    gcvFORMAT_DATATYPE_FLOAT16,
    gcvFORMAT_DATATYPE_FLOAT32,
    gcvFORMAT_DATATYPE_FLOAT_E5B9G9R9,
    gcvFORMAT_DATATYPE_FLOAT_B10G11R11F,
    gcvFORMAT_DATATYPE_INDEX,
    gcvFORMAT_DATATYPE_SRGB,
    gcvFORMAT_DATATYPE_FLOAT32_UINT,
} gceFORMAT_DATATYPE;

typedef enum _gceORIENTATION {
    gcvORIENTATION_TOP_BOTTOM,
    gcvORIENTATION_BOTTOM_TOP,
} gceORIENTATION;

/* Special enums for width field in gcsFORMAT_COMPONENT. */
typedef enum _gceCOMPONENT_CONTROL {
    gcvCOMPONENT_NOTPRESENT     = 0x00,
    gcvCOMPONENT_DONTCARE       = 0x80,
    gcvCOMPONENT_WIDTHMASK      = 0x7F,
    gcvCOMPONENT_ODD            = 0x80

} gceCOMPONENT_CONTROL;

/* User option. */
typedef enum _gceDEBUG_MSG {
    gcvDEBUG_MSG_NONE,
    gcvDEBUG_MSG_ERROR,
    gcvDEBUG_MSG_WARNING

} gceDEBUG_MSG;

/* Compressed format now was defined same as dec400d, should be general. */
#if gcdENABLE_DEFALUT_FC_COMPRESSED
typedef enum _VIV_COMPRESS_FMT {
    _VIV_CFMT_ARGB4 = 0,
    _VIV_CFMT_A1R5G5B5,
    _VIV_CFMT_R5G6B5,
    _VIV_CFMT_ARGB8,
    _VIV_CFMT_XRGB8,
    _VIV_CFMT_Z24S8,
    _VIV_CFMT_Z24,
    _VIV_CFMT_VAA16,
    _VIV_CFMT_Z16,
    _VIV_CFMT_S8,
    _VIV_CFMT_YUV_ONLY,
    _VIV_CFMT_UV_MIX,
    _VIV_CFMT_AYUV,
    _VIV_CFMT_UYVY,
    _VIV_CFMT_YUY2,
    _VIV_CFMT_A2R10G10B10,
    _VIV_CFMT_XRGB4,  
    _VIV_CFMT_X1R5G5B5,  
    _VIV_CFMT_BAYER,
    _VIV_CFMT_SIGNED_BAYER,

    _VIV_CFMT_MAX,
} _VIV_COMPRESS_FMT;
#else
typedef enum _VIV_COMPRESS_FMT {
    _VIV_CFMT_ARGB8 = 0,
    _VIV_CFMT_XRGB8,
    _VIV_CFMT_AYUV,
    _VIV_CFMT_UYVY,
    _VIV_CFMT_YUY2,
    _VIV_CFMT_YUV_ONLY,
    _VIV_CFMT_UV_MIX,
    _VIV_CFMT_ARGB4,
    _VIV_CFMT_XRGB4,
    _VIV_CFMT_A1R5G5B5,
    _VIV_CFMT_X1R5G5B5,
    _VIV_CFMT_R5G6B5,
    _VIV_CFMT_Z24S8,
    _VIV_CFMT_Z24,
    _VIV_CFMT_Z16,
    _VIV_CFMT_A2R10G10B10,
    _VIV_CFMT_BAYER,
    _VIV_CFMT_SIGNED_BAYER,
    _VIV_CFMT_VAA16,
    _VIV_CFMT_S8,

    _VIV_CFMT_MAX,
} _VIV_COMPRESS_FMT;
#endif

typedef enum _gcePROGRAM_STAGE {
    gcvPROGRAM_STAGE_VERTEX         = 0x0,
    gcvPROGRAM_STAGE_TCS            = 0x1,
    gcvPROGRAM_STAGE_TES            = 0x2,
    gcvPROGRAM_STAGE_GEOMETRY       = 0x3,
    gcvPROGRAM_STAGE_GPIPE_COUNT    = 0x4,
    gcvPROGRAM_STAGE_FRAGMENT       = 0x4,
    gcvPROGRAM_STAGE_GRAPHICS_COUNT = 0x5,
    gcvPROGRAM_STAGE_COMPUTE        = 0x5,
    gcvPROGRAM_STAGE_OPENCL         = 0x6,
    gcvPROGRAM_STAGE_LAST

} gcePROGRAM_STAGE;

typedef enum _gcePROGRAM_STAGE_BIT {
    gcvPROGRAM_STAGE_VERTEX_BIT   = 1 << gcvPROGRAM_STAGE_VERTEX,
    gcvPROGRAM_STAGE_TCS_BIT      = 1 << gcvPROGRAM_STAGE_TCS,
    gcvPROGRAM_STAGE_TES_BIT      = 1 << gcvPROGRAM_STAGE_TES,
    gcvPROGRAM_STAGE_GEOMETRY_BIT = 1 << gcvPROGRAM_STAGE_GEOMETRY,
    gcvPROGRAM_STAGE_FRAGMENT_BIT = 1 << gcvPROGRAM_STAGE_FRAGMENT,
    gcvPROGRAM_STAGE_COMPUTE_BIT  = 1 << gcvPROGRAM_STAGE_COMPUTE,
    gcvPROGRAM_STAGE_OPENCL_BIT   = 1 << gcvPROGRAM_STAGE_OPENCL,
} gcePROGRAM_STAGE_BIT;

typedef enum _gceBLIT_FLAG {
    gcvBLIT_FLAG_SKIP_DEPTH_WRITE   = 1 << 0,
    gcvBLIT_FLAG_SKIP_STENCIL_WRITE = 1 << 1,
} gceBLIT_FLAG;

/* Clear flags. */
typedef enum _gceCLEAR {
    gcvCLEAR_COLOR              = 0x1,
    gcvCLEAR_DEPTH              = 0x2,
    gcvCLEAR_STENCIL            = 0x4,
    gcvCLEAR_HZ                 = 0x8,
    gcvCLEAR_WITH_GPU_ONLY      = 0x100,
    gcvCLEAR_WITH_CPU_ONLY      = 0x200,
    gcvCLEAR_MULTI_SLICES       = 0x400,
} gceCLEAR;

typedef enum _gceBLIT_TYPE {
    gcvBLIT_DRAW_CLEAR          = 0,
    gcvBLIT_DRAW_BLIT           = 1,
    gcvBLIT_DRAW_BLIT_DEPTH     = 2,
    gcvBLIT_COMPUTE_BLIT        = 3,

    /* last number, not a real type */
    gcvBLIT_NUM_TYPE

} gceBLIT_TYPE;

typedef enum _gceSPLIT_DRAW_TYPE {
    gcvSPLIT_DRAW_UNKNOWN = 0x0,
    gcvSPLIT_DRAW_1,
    gcvSPLIT_DRAW_2,
    gcvSPLIT_DRAW_3,
    gcvSPLIT_DRAW_4,
    gcvSPLIT_DRAW_XFB,
    gcvSPLIT_DRAW_INDEX_FETCH,
    gcvSPLIT_DRAW_TCS,
    gcvSPLIT_DRAW_STIPPLE,
    gcvSPLIT_DRAW_WIDE_LINE,
    gcvSPLIT_DRAW_LINES_HW_ZERO_AREA_LINE_PATCH,
    gcvSPLIT_DRAW_LINES_PRIMITIVE_OVERFLOW_PATCH,
    gcvSPLIT_DRAW_LAST

} gceSPLIT_DRAW_TYPE;

/* Blending targets. */
typedef enum _gceBLEND_UNIT {
    gcvBLEND_SOURCE,
    gcvBLEND_TARGET,
} gceBLEND_UNIT;

typedef enum _gceXfbCmd {
    gcvXFBCMD_BEGIN             = 0,
    gcvXFBCMD_PAUSE             = 1,
    gcvXFBCMD_RESUME            = 2,
    gcvXFBCMD_END               = 3,
    gcvXFBCMD_PAUSE_INCOMMIT    = 4,
    gcvXFBCMD_RESUME_INCOMMIT   = 5,
    gcvXFBCMD_INVALID           = 6,
} gceXfbCmd;

typedef enum _gceXfbStatus {
    gcvXFB_Disabled = 0,
    gcvXFB_Paused,
    gcvXFB_Enabled,
} gceXfbStatus;

typedef enum _gceQueryStatus {
    gcvQUERY_Disabled       = 0,
    gcvQUERY_Paused         = 1,
    gcvQUERY_Enabled        = 2,
} gceQueryStatus;

typedef enum _gceQueryCmd {
    gcvQUERYCMD_BEGIN       = 0,
    gcvQUERYCMD_PAUSE       = 1,
    gcvQUERYCMD_RESUME      = 2,
    gcvQUERYCMD_END         = 3,
    gcvQUERYCMD_INVALID     = 4,
} gceQueryCmd;

/* 0~10 for PIPELINE_STATISTICS_QUERY, DO NOT change. */
typedef enum _gceQueryType {
    gcvQUERY_VERTICES_SUBMITTED                     = 0,
    gcvQUERY_PRIMITIVES_SUBMITTED                   = 1,
    gcvQUERY_VERTEX_SHADER_INVOCATIONIS             = 2,
    gcvQUERY_TESS_CONTROL_SHADER_PATCHES            = 3,
    gcvQUERY_TESS_EVALUATION_SHADER_INVOCATIONS     = 4,
    gcvQUERY_GEOMETRY_SHADER_INVOCATIONS            = 5,
    gcvQUERY_GEOMETRY_SHADER_PRIMITIVES_EMITIED     = 6,
    gcvQUERY_FRAGMENT_SHADER_INVOCATIONS            = 7,
    gcvQUERY_COMPUTE_SHADER_INVOCATIONS             = 8,
    gcvQUERY_CLIPPING_INPUT_PRIMITIVES              = 9,
    gcvQUERY_CLIPPING_OUTPUT_PRIMITIVES             = 10,
    gcvQUERY_OCCLUSION                              = 11,
    gcvQUERY_XFB_WRITTEN                            = 12,
    gcvQUERY_PRIM_GENERATED                         = 13,
    gcvQUERY_TIME_ELAPSED                           = 14,
    gcvQUERY_XFB_OVERFLOW                           = 15,
    gcvQUERY_XFB_STREAM_OVERFLOW                    = 16,
    gcvQUERY_MAX_NUM                                = 17,
} gceQueryType;

/* Cube faces. */
typedef enum _gceTEXTURE_FACE {
    gcvFACE_NONE = 0,
    gcvFACE_POSITIVE_X,
    gcvFACE_NEGATIVE_X,
    gcvFACE_POSITIVE_Y,
    gcvFACE_NEGATIVE_Y,
    gcvFACE_POSITIVE_Z,
    gcvFACE_NEGATIVE_Z,
} gceTEXTURE_FACE;

typedef enum _gceVERTEX_FORMAT {
    gcvVERTEX_BYTE,
    gcvVERTEX_UNSIGNED_BYTE,
    gcvVERTEX_SHORT,
    gcvVERTEX_UNSIGNED_SHORT,
    gcvVERTEX_INT,
    gcvVERTEX_UNSIGNED_INT,
    gcvVERTEX_FIXED,
    gcvVERTEX_HALF,
    gcvVERTEX_FLOAT,
    gcvVERTEX_DOUBLE,
    gcvVERTEX_UNSIGNED_INT_10_10_10_2,
    gcvVERTEX_INT_10_10_10_2,
    gcvVERTEX_UNSIGNED_INT_2_10_10_10_REV,
    gcvVERTEX_INT_2_10_10_10_REV,
    gcvVERTEX_UNSIGNED_INT_10F_11F_11F_REV,
    /* integer format */
    gcvVERTEX_INT8,
    gcvVERTEX_INT16,
    gcvVERTEX_INT32,
} gceVERTEX_FORMAT;

/* What the SW converting scheme to create temp attrib */
typedef enum _gceATTRIB_SCHEME {
    gcvATTRIB_SCHEME_KEEP = 0,
    gcvATTRIB_SCHEME_2_10_10_10_REV_TO_FLOAT,
    gcvATTRIB_SCHEME_BYTE_TO_IVEC4,
    gcvATTRIB_SCHEME_SHORT_TO_IVEC4,
    gcvATTRIB_SCHEME_INT_TO_IVEC4,
    gcvATTRIB_SCHEME_UBYTE_TO_UVEC4,
    gcvATTRIB_SCHEME_USHORT_TO_UVEC4,
    gcvATTRIB_SCHEME_UINT_TO_UVEC4,
    gcvATTRIB_SCHEME_DOUBLE_TO_FLOAT,
    gcvATTRIB_SCHEME_UBYTE_BGRA_TO_UBYTE_RGBA,
    gcvATTRIB_SCHEME_2_10_10_10_REV_BGRA_TO_FLOAT_RGBA,
} gceATTRIB_SCHEME;

typedef enum _gceBUFOBJ_TYPE {
    gcvBUFOBJ_TYPE_ARRAY_BUFFER         = 1,
    gcvBUFOBJ_TYPE_ELEMENT_ARRAY_BUFFER = 2,
    gcvBUFOBJ_TYPE_UNIFORM_BUFFER       = 3,
    gcvBUFOBJ_TYPE_DRAW_INDIRECT_BUFFER = 4,
    gcvBUFOBJ_TYPE_XFB_BUFFER           = 5,
    gcvBUFOBJ_TYPE_GENERIC_BUFFER       = 100



} gceBUFOBJ_TYPE;

typedef enum _gceBUFOBJ_USAGE {
    gcvBUFOBJ_USAGE_NONE                = 0x0,
    gcvBUFOBJ_USAGE_STREAM_DRAW         = 0x1,
    gcvBUFOBJ_USAGE_STREAM_READ         = 0x2,
    gcvBUFOBJ_USAGE_STREAM_COPY         = 0x3,
    gcvBUFOBJ_USAGE_STATIC_DRAW         = 0x4,
    gcvBUFOBJ_USAGE_STATIC_READ         = 0x5,
    gcvBUFOBJ_USAGE_STATIC_COPY         = 0x6,
    gcvBUFOBJ_USAGE_DYNAMIC_DRAW        = 0x7,
    gcvBUFOBJ_USAGE_DYNAMIC_READ        = 0x8,
    gcvBUFOBJ_USAGE_DYNAMIC_COPY        = 0x9,
    gcvBUFOBJ_USAGE_PROBE_COUTNER       = 0x10,

    /* Use 8bits to save the usage. */
    gcvBUFOBJ_USAGE_MASK =              0xFF,

    /* Some special flags. */
    /* special patch for optimaize performance,
     * no fence and duplicate stream to ensure data correct

     */
    gcvBUFOBJ_USAGE_FLAG_DISABLE_FENCE_DYNAMIC_STREAM   = 0x100,

    /* This buffer object is used by driver,
     * so we need to copy the data to the logical memory.

     */
    gcvBUFOBJ_USAGE_FLAG_DATA_USED_BY_DRIVER            = 0x200,
    /* Allocate video memory under 4GB. */
    gcvBUFOBJ_USAGE_FLAG_32BIT_VA                       = 0x400,
} gceBUFOBJ_USAGE;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Tiling mode for painting and imagig.
 **
 **  This enumeration defines the tiling modes supported by the HAL.  This is
 **  in fact a one-to-one mapping of the OpenVG 1.1 tile modes.

 */
typedef enum _gceTILE_MODE {
    gcvTILE_FILL,
    gcvTILE_PAD,
    gcvTILE_REPEAT,
    gcvTILE_REFLECT

} gceTILE_MODE;

/******************************************************************************/
/** @ingroup gcoVG
 **
 **  @brief  The different paint modes.
 **
 **  This enumeration lists the available paint modes.

 */
typedef enum _gcePAINT_TYPE {
    /** Solid color. */
    gcvPAINT_MODE_SOLID,

    /** Linear gradient. */
    gcvPAINT_MODE_LINEAR,

    /** Radial gradient. */
    gcvPAINT_MODE_RADIAL,

    /** Pattern. */
    gcvPAINT_MODE_PATTERN,

    /** Mode count. */
    gcvPAINT_MODE_COUNT

} gcePAINT_TYPE;

/**
 ** @ingroup gcoVG
 **
 **  @brief Types of path data supported by HAL.
 **
 **  This enumeration defines the types of path data supported by the HAL.
 **  This is in fact a one-to-one mapping of the OpenVG 1.1 path types.

 */
typedef enum _gcePATHTYPE {
    gcePATHTYPE_UNKNOWN = -1,
    gcePATHTYPE_INT8,
    gcePATHTYPE_INT16,
    gcePATHTYPE_INT32,
    gcePATHTYPE_FLOAT

} gcePATHTYPE;

/**
 ** @ingroup gcoVG
 **
 **  @brief Supported path segment commands.
 **
 **  This enumeration defines the path segment commands supported by the HAL.

 */
typedef enum _gceVGCMD {
    gcvVGCMD_END,               /*  0: GCCMD_TS_OPCODE_END           */
    gcvVGCMD_CLOSE,             /*  1: GCCMD_TS_OPCODE_CLOSE         */
    gcvVGCMD_MOVE,              /*  2: GCCMD_TS_OPCODE_MOVE          */
    gcvVGCMD_MOVE_REL,          /*  3: GCCMD_TS_OPCODE_MOVE_REL      */
    gcvVGCMD_LINE,              /*  4: GCCMD_TS_OPCODE_LINE          */
    gcvVGCMD_LINE_REL,          /*  5: GCCMD_TS_OPCODE_LINE_REL      */
    gcvVGCMD_QUAD,              /*  6: GCCMD_TS_OPCODE_QUADRATIC     */
    gcvVGCMD_QUAD_REL,          /*  7: GCCMD_TS_OPCODE_QUADRATIC_REL */
    gcvVGCMD_CUBIC,             /*  8: GCCMD_TS_OPCODE_CUBIC         */
    gcvVGCMD_CUBIC_REL,         /*  9: GCCMD_TS_OPCODE_CUBIC_REL     */
    gcvVGCMD_BREAK,             /* 10: GCCMD_TS_OPCODE_BREAK         */
    gcvVGCMD_HLINE,             /* 11: ******* R E S E R V E D *******/
    gcvVGCMD_HLINE_REL,         /* 12: ******* R E S E R V E D *******/
    gcvVGCMD_VLINE,             /* 13: ******* R E S E R V E D *******/
    gcvVGCMD_VLINE_REL,         /* 14: ******* R E S E R V E D *******/
    gcvVGCMD_SQUAD,             /* 15: ******* R E S E R V E D *******/
    gcvVGCMD_SQUAD_REL,         /* 16: ******* R E S E R V E D *******/
    gcvVGCMD_SCUBIC,            /* 17: ******* R E S E R V E D *******/
    gcvVGCMD_SCUBIC_REL,        /* 18: ******* R E S E R V E D *******/
    gcvVGCMD_SCCWARC,           /* 19: ******* R E S E R V E D *******/
    gcvVGCMD_SCCWARC_REL,       /* 20: ******* R E S E R V E D *******/
    gcvVGCMD_SCWARC,            /* 21: ******* R E S E R V E D *******/
    gcvVGCMD_SCWARC_REL,        /* 22: ******* R E S E R V E D *******/
    gcvVGCMD_LCCWARC,           /* 23: ******* R E S E R V E D *******/
    gcvVGCMD_LCCWARC_REL,       /* 24: ******* R E S E R V E D *******/
    gcvVGCMD_LCWARC,            /* 25: ******* R E S E R V E D *******/
    gcvVGCMD_LCWARC_REL,        /* 26: ******* R E S E R V E D *******/


    /* The width of the command recognized by the hardware on bits. */
    gcvVGCMD_WIDTH = 5,

    /* Hardware command mask. */
    gcvVGCMD_MASK = (1 << gcvVGCMD_WIDTH) - 1,

    /* Command modifiers. */
    gcvVGCMD_H_MOD   = 1 << gcvVGCMD_WIDTH, /* =  32 */
    gcvVGCMD_V_MOD   = 2 << gcvVGCMD_WIDTH, /* =  64 */
    gcvVGCMD_S_MOD   = 3 << gcvVGCMD_WIDTH, /* =  96 */
    gcvVGCMD_ARC_MOD = 4 << gcvVGCMD_WIDTH, /* = 128 */


    /* Emulated LINE commands. */
    gcvVGCMD_HLINE_EMUL     = gcvVGCMD_H_MOD | gcvVGCMD_LINE,       /* =  36 */
    gcvVGCMD_HLINE_EMUL_REL = gcvVGCMD_H_MOD | gcvVGCMD_LINE_REL,   /* =  37 */
    gcvVGCMD_VLINE_EMUL     = gcvVGCMD_V_MOD | gcvVGCMD_LINE,       /* =  68 */
    gcvVGCMD_VLINE_EMUL_REL = gcvVGCMD_V_MOD | gcvVGCMD_LINE_REL,   /* =  69 */


    /* Emulated SMOOTH commands. */
    gcvVGCMD_SQUAD_EMUL      = gcvVGCMD_S_MOD | gcvVGCMD_QUAD,      /* = 102 */
    gcvVGCMD_SQUAD_EMUL_REL  = gcvVGCMD_S_MOD | gcvVGCMD_QUAD_REL,  /* = 103 */
    gcvVGCMD_SCUBIC_EMUL     = gcvVGCMD_S_MOD | gcvVGCMD_CUBIC,     /* = 104 */
    gcvVGCMD_SCUBIC_EMUL_REL = gcvVGCMD_S_MOD | gcvVGCMD_CUBIC_REL, /* = 105 */


    /* Emulation ARC commands. */
    gcvVGCMD_ARC_LINE     = gcvVGCMD_ARC_MOD | gcvVGCMD_LINE,       /* = 132 */
    gcvVGCMD_ARC_LINE_REL = gcvVGCMD_ARC_MOD | gcvVGCMD_LINE_REL,   /* = 133 */
    gcvVGCMD_ARC_QUAD     = gcvVGCMD_ARC_MOD | gcvVGCMD_QUAD,       /* = 134 */
    gcvVGCMD_ARC_QUAD_REL = gcvVGCMD_ARC_MOD | gcvVGCMD_QUAD_REL    /* = 135 */
} gceVGCMD;

typedef enum _gceVGCMD *gceVGCMD_PTR;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Blending modes supported by the HAL.
 **
 **  This enumeration defines the blending modes supported by the HAL.  This is
 **  in fact a one-to-one mapping of the OpenVG 1.1 blending modes.

 */
typedef enum _gceVG_BLEND {
    gcvVG_BLEND_SRC,
    gcvVG_BLEND_SRC_OVER,
    gcvVG_BLEND_DST_OVER,
    gcvVG_BLEND_SRC_IN,
    gcvVG_BLEND_DST_IN,
    gcvVG_BLEND_MULTIPLY,
    gcvVG_BLEND_SCREEN,
    gcvVG_BLEND_DARKEN,
    gcvVG_BLEND_LIGHTEN,
    gcvVG_BLEND_ADDITIVE,
    gcvVG_BLEND_SUBTRACT,
    gcvVG_BLEND_FILTER

} gceVG_BLEND;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Image modes supported by the HAL.
 **
 **  This enumeration defines the image modes supported by the HAL.  This is
 **  in fact a one-to-one mapping of the OpenVG 1.1 image modes with the addition
 **  of NO IMAGE.

 */
typedef enum _gceVG_IMAGE {
    gcvVG_IMAGE_NONE,
    gcvVG_IMAGE_NORMAL,
    gcvVG_IMAGE_MULTIPLY,
    gcvVG_IMAGE_STENCIL,
    gcvVG_IMAGE_FILTER

} gceVG_IMAGE;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Filter mode patterns and imaging.
 **
 **  This enumeration defines the filter modes supported by the HAL.

 */
typedef enum _gceIMAGE_FILTER {
    gcvFILTER_POINT,
    gcvFILTER_LINEAR,
    gcvFILTER_BI_LINEAR

} gceIMAGE_FILTER;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Primitive modes supported by the HAL.
 **
 **  This enumeration defines the primitive modes supported by the HAL.

 */
typedef enum _gceVG_PRIMITIVE {
    gcvVG_SCANLINE,
    gcvVG_RECTANGLE,
    gcvVG_TESSELLATED,
    gcvVG_TESSELLATED_TILED

} gceVG_PRIMITIVE;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Rendering quality modes supported by the HAL.
 **
 **  This enumeration defines the rendering quality modes supported by the HAL.

 */
typedef enum _gceRENDER_QUALITY {
    gcvVG_NONANTIALIASED,
    gcvVG_2X2_MSAA,
    gcvVG_2X4_MSAA,
    gcvVG_4X4_MSAA

} gceRENDER_QUALITY;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Fill rules supported by the HAL.
 **
 **  This enumeration defines the fill rules supported by the HAL.

 */
typedef enum _gceFILL_RULE { gcvVG_EVEN_ODD, gcvVG_NON_ZERO } gceFILL_RULE;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Cap styles supported by the HAL.
 **
 **  This enumeration defines the cap styles supported by the HAL.

 */
typedef enum _gceCAP_STYLE { gcvCAP_BUTT, gcvCAP_ROUND, gcvCAP_SQUARE } gceCAP_STYLE;

/**
 **  @ingroup gcoVG
 **
 **  @brief  Join styles supported by the HAL.
 **
 **  This enumeration defines the join styles supported by the HAL.

 */
typedef enum _gceJOIN_STYLE {
    gcvJOIN_MITER,
    gcvJOIN_ROUND,
    gcvJOIN_BEVEL

} gceJOIN_STYLE;

/* Base values for channel mask definitions. */
#define gcvCHANNEL_X (0)

#define gcvCHANNEL_R (1 << 0)

#define gcvCHANNEL_G (1 << 1)

#define gcvCHANNEL_B (1 << 2)

#define gcvCHANNEL_A (1 << 3)



typedef enum _gceCHANNEL {
    gcvCHANNEL_XXXX = (gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_X),
    gcvCHANNEL_XXXA = (gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_A),
    gcvCHANNEL_XXBX = (gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_B | gcvCHANNEL_X),
    gcvCHANNEL_XXBA = (gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_B | gcvCHANNEL_A),

    gcvCHANNEL_XGXX = (gcvCHANNEL_X | gcvCHANNEL_G | gcvCHANNEL_X | gcvCHANNEL_X),
    gcvCHANNEL_XGXA = (gcvCHANNEL_X | gcvCHANNEL_G | gcvCHANNEL_X | gcvCHANNEL_A),
    gcvCHANNEL_XGBX = (gcvCHANNEL_X | gcvCHANNEL_G | gcvCHANNEL_B | gcvCHANNEL_X),
    gcvCHANNEL_XGBA = (gcvCHANNEL_X | gcvCHANNEL_G | gcvCHANNEL_B | gcvCHANNEL_A),

    gcvCHANNEL_RXXX = (gcvCHANNEL_R | gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_X),
    gcvCHANNEL_RXXA = (gcvCHANNEL_R | gcvCHANNEL_X | gcvCHANNEL_X | gcvCHANNEL_A),
    gcvCHANNEL_RXBX = (gcvCHANNEL_R | gcvCHANNEL_X | gcvCHANNEL_B | gcvCHANNEL_X),
    gcvCHANNEL_RXBA = (gcvCHANNEL_R | gcvCHANNEL_X | gcvCHANNEL_B | gcvCHANNEL_A),

    gcvCHANNEL_RGXX = (gcvCHANNEL_R | gcvCHANNEL_G | gcvCHANNEL_X | gcvCHANNEL_X),
    gcvCHANNEL_RGXA = (gcvCHANNEL_R | gcvCHANNEL_G | gcvCHANNEL_X | gcvCHANNEL_A),
    gcvCHANNEL_RGBX = (gcvCHANNEL_R | gcvCHANNEL_G | gcvCHANNEL_B | gcvCHANNEL_X),
    gcvCHANNEL_RGBA = (gcvCHANNEL_R | gcvCHANNEL_G | gcvCHANNEL_B | gcvCHANNEL_A),
} gceCHANNEL;

/* Defines the statistical data keys monitored by the statistics module */
typedef enum _gceSTATISTICS {
    gcvFRAME_FPS    = 1,
} gceSTATISTICS;

/* Value types. */
typedef enum _gceVALUE_TYPE {
    gcvVALUE_UINT = 0x0,
    gcvVALUE_FIXED,
    gcvVALUE_FLOAT,
    gcvVALUE_INT,
    /*
     ** The value need be unsigned denormalized. clamp (0.0-1.0) should be done first.
     */
    gcvVALUE_FLAG_UNSIGNED_DENORM   = 0x00010000,
    /*
     ** The value need be signed denormalized. clamp (-1.0-1.0) should be done first.
     */
    gcvVALUE_FLAG_SIGNED_DENORM     = 0x00020000,
    /*
     ** The value need to gammar
     */
    gcvVALUE_FLAG_GAMMAR            = 0x00040000,
    /*
     ** The value need to convert from float to float16
     */
    gcvVALUE_FLAG_FLOAT_TO_FLOAT16  = 0x0080000,
    /*
     ** Mask for flag field.
     */
    gcvVALUE_FLAG_MASK = 0xFFFF0000,
} gceVALUE_TYPE;

typedef enum _gceTRACEMODE {
    gcvTRACEMODE_NONE       = 0,
    gcvTRACEMODE_FULL       = 1,
    gcvTRACEMODE_LOGGER     = 2,
    gcvTRACEMODE_ALLZONE    = 3,
    gcvTRACEMODE_PRE        = 4,
    gcvTRACEMODE_POST       = 5,
} gceTRACEMODE;

enum {
    /* GPU can't issue more that 32bit physical address */
    gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS = 1 << 0,

    gcvPLATFORM_FLAG_IMX_MM           = 1 << 1,
};

#if gcdUSE_CAPBUF

typedef enum _gceCAPBUF_META_TYPE {
    gcvCAPBUF_META_TYPE_BASE = 0,
    gcvCAPBUF_META_TYPE_DRAW_ID,
    gcvCAPBUF_META_TYPE_SH_UNIFORM,
    gcvCAPBUF_META_TYPE_PPU_PARAMETERS,
    gcvCAPBUF_META_TYPE_VIP_SRAM_REMAP,
    gcvCAPBUF_META_TYPE_AXI_SRAM_REMAP,
    gcvCAPBUF_META_TYPE_IMAGE_PHYSICAL_ADDRESS,
    gcvCAPBUF_META_TYPE_IMAGE_PHYSICAL_ADDRESS_40BIT,
    gcvCAPBUF_META_TYPE_SH_INST_ADDRESS,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_LOCAL_ADDRESS_SPACE,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_CONSTANT_ADDRESS_SPACE,
    gcvCAPBUF_META_TYPE_NN_TP_INST_ADDRESS,
    gcvCAPBUF_META_TYPE_LOW32_OF_40BIT_PHY_ADDR,
    gcvCAPBUF_META_TYPE_GPU_SYNC_CMD,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_PRINTFADDRESS,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_MAXPRINTFADDRESS,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_CIRCULAR_BUFFER_START_ADDR,
    gcvCAPBUF_META_TYPE_SH_UNIFORM_ARGS_CIRCULAR_BUFFER_END_ADDR_PLUS1,
    /* Keep it at the end of the list. */
    gcvCAPBUF_META_TYPE_COUNT

} gceCAPBUF_META_TYPE;

typedef enum _gceCAPBUF_SH_UNIFROM_ARGS {
    gcvCAPBUF_SH_UNIFORM_ARGS_INVALID = 0,
    gcvCAPBUF_SH_UNIFORM_ARGS_IMAGE_PHYSICAL_ADDRESS,
    gcvCAPBUF_SH_UNIFORM_ARGS_IMAGE_PHYSICAL_ADDRESS_40BIT,
    gcvCAPBUF_SH_UNIFORM_ARGS_LOCAL_ADDRESS_SPACE,
    gcvCAPBUF_SH_UNIFORM_ARGS_LOCAL_ADDRESS_SPACE_COMBINE,
    gcvCAPBUF_SH_UNIFORM_ARGS_CONSTANT_ADDRESS_SPACE,
    gcvCAPBUF_SH_UNIFORM_ARGS_LOW32_OF_40BIT_PHY_ADDR,
    gcvCAPBUF_SH_UNIFORM_ARGS_LOW_HIGH_40BIT_PHY_ADDR,
    gcvCAPBUF_SH_UNIFORM_ARGS_PRINTFADDRESS,
    gcvCAPBUF_SH_UNIFORM_ARGS_MAXPRINTFADDRESS,

    /* add new uniform data source type by generate function, fix above type if need */

    /* uniform data generate by function gcfVX_LoadKernelArgValues */
    gcvCAPBUF_SH_UNIFORM_ARGS_CIRCULAR_BUFFER_START_ADDR,
    gcvCAPBUF_SH_UNIFORM_ARGS_CIRCULAR_BUFFER_END_ADDR_PLUS1,

    /* uniform data generate by function gcfVX_FlushVIRKernelResource */
    gcvCAPBUF_SH_UNIFORM_ARGS_40BIT_LOADKERNEL_IMAGE_ADDR,

    /* uniform data generate by function gcfVX_LoadKernelArgValues */
    gcvCAPBUF_SH_UNIFORM_ARGS_40BIT_VIRKERNEL_UNIFORMTABLE,

    /* Keep it at the end of the list. */
    gcvCAPBUF_SH_UNIFORM_ARGS_COUNT

} gceCAPBUF_SH_UNIFORM_ARGS;

typedef enum _gceCAPBUF_SH_UNIFORM_STATE_DATA_TYPE

{
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_INVALID = 0,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_IMAGE_PHYSICAL_ADDRESS,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_IMAGE_PHYSICAL_ADDRESS_40BIT_LOW,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_IMAGE_PHYSICAL_ADDRESS_40BIT_HIGH,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_IMAGE_PHYSICAL_ADDRESS_40BIT_HIGH1,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_LOCAL_ADDRESS_SPACE,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_CONSTANT_ADDRESS_SPACE,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_LOW32_OF_40BIT_PHY_ADDR,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_LOW_HIGH_40BIT_PHY_ADDR_LOW,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_LOW_HIGH_40BIT_PHY_ADDR_HIGH,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_PRINTFADDRESS,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_MAXPRINTFADDRESS,

    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_CIRCULAR_BUFFER_START_ADDR,
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_CIRCULAR_BUFFER_END_ADDR_PLUS1,
    /* Keep it at the end of the list. */
    gcvCAPBUF_SH_UNIFORM_STATE_DATA_TYPE_COUNT

} gceCAPBUF_SH_UNIFORM_STATE_DATA_TYPE;

typedef enum _gceCAPBUF_PPU_PARAMETERS_INDEX {
    gcvCAPBUF_PPU_GLOBAL_OFFSET_X = 0,
    gcvCAPBUF_PPU_GLOBAL_OFFSET_Y,
    gcvCAPBUF_PPU_GLOBAL_OFFSET_Z,
    gcvCAPBUF_PPU_GLOBAL_SCALE_X,
    gcvCAPBUF_PPU_GLOBAL_SCALE_Y,
    gcvCAPBUF_PPU_GLOBAL_SCALE_Z,
    gcvCAPBUF_PPU_GROUP_SIZE_X,
    gcvCAPBUF_PPU_GROUP_SIZE_Y,
    gcvCAPBUF_PPU_GROUP_SIZE_Z,
    gcvCAPBUF_PPU_GROUP_COUNT_X,
    gcvCAPBUF_PPU_GROUP_COUNT_Y,
    gcvCAPBUF_PPU_GROUP_COUNT_Z,
    gcvCAPBUF_PPU_PARAMETERS_COUNT

} gceCAPBUF_PPU_GLOBALE_OFFSET_INDEX;

#endif


/* GL_VIV internal usage */
#ifndef GL_MAP_BUFFER_OBJ_VIV

#define GL_MAP_BUFFER_OBJ_VIV       0x10000

#endif


/* Command buffer usage. */
#define gcvCOMMAND_2D (1 << 0)

#define gcvCOMMAND_3D (1 << 1)


/* Default chip ID means chip ID same as core index. */
#define gcvCHIP_ID_DEFAULT              (~0U)


/* Tile status header size */
#ifndef gcvTS_FC_HEADER_SIZE

#define gcvTS_FC_HEADER_SIZE        128

#endif


/******************************************************************************
 ***************************** Object Declarations ****************************
 ******************************************************************************/
typedef struct _gckCONTEXT            *gckCONTEXT;
typedef struct _gcoCMDBUF             *gcoCMDBUF;

typedef struct _gcsSTATE_DELTA        *gcsSTATE_DELTA_PTR;
typedef struct _gcsQUEUE              *gcsQUEUE_PTR;
typedef struct _gcoQUEUE              *gcoQUEUE;
typedef struct _gcsHAL_INTERFACE      *gcsHAL_INTERFACE_PTR;

#if PHYTIUM_PROFILER
typedef struct _gcsHAL_PROFILER_INTERFACE   *gcsHAL_PROFILER_INTERFACE_PTR;
#endif

typedef struct _gcs2D_PROFILE         *gcs2D_PROFILE_PTR;

#ifdef __cplusplus

}

#endif


#endif /* __gc_hal_enum_h_ */
