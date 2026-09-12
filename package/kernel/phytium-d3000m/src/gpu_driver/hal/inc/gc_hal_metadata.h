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


#ifndef __gc_hal_kernel_metadata_h_
#define __gc_hal_kernel_metadata_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Macro to combine four characters into a Character Code. */
#define __FOURCC(a, b, c, d) \
    ((uint32_t)(a) | ((uint32_t)(b) << 8) | ((uint32_t)(c) << 16) | ((uint32_t)(d) << 24))

#define VIV_VIDMEM_METADATA_MAGIC __FOURCC('v', 's', 'i', 'm')

#define PLANE_COUNT 3

/* Metadata for cross-device fd share with additional (ts) info. */
typedef struct _VIV_VIDMEM_METADATA {
    uint32_t magic;
    uint32_t dmabuf_size;
    uint32_t time_stamp;
    uint32_t compressed;
    uint32_t image_format;
    struct {
        uint32_t offset;
        uint32_t stride;
        uint32_t width;
        uint32_t height;
        uint32_t compression_format;
        uint32_t tile_mode;
        int32_t  ts_fd;
        void     *ts_dma_buf;
        uint32_t ts_offset;
        uint32_t fc_enabled;
        uint32_t fc_value_lower;
        uint32_t fc_value_upper;
        uint32_t header_size;
    } plane[3];
    uint32_t reserved[8];
} _VIV_VIDMEM_METADATA;

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_metadata_h_ */
