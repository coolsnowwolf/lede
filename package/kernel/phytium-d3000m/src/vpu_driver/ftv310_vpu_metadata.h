/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL PRECISION INSIGHT AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef _FTV310_VPU_METADATA_H_
#define _FTV310_VPU_METADATA_H_

#define FTV310_VPU_MAGIC(ch0, ch1, ch2, ch3)                                       \
	((unsigned long)(unsigned char)(ch0) |                                 \
	 ((unsigned long)(unsigned char)(ch1) << 8) |                          \
	 ((unsigned long)(unsigned char)(ch2) << 16) |                         \
	 ((unsigned long)(unsigned char)(ch3) << 24))

#define FTV310_VPU_IMAGE_VIV_META_DATA_MAGIC FTV310_VPU_MAGIC('V', 'I', 'V', 'M')

/**
 * name of image format for exchange.
 */
enum viv_image_format {
	IMAGE_BYTE = 0, /**< pure data, not image */
	IMAGE_YUV420, /**< 3 plane YUV420 8 bits */
	IMAGE_NV12, /**< 2 plane NV12 8 bits */
	IMAGE_YV12, /**< 3 plane YV12 8 bits */
	IMAGE_UYVY, /**< packed YUV422 8 bits */
	IMAGE_YUY2, /**< packed YUV422 8 bits */
	IMAGE_Y8, /**< single plane 8 bits */
	IMAGE_UV8, /**< single UV interleave plan 8 bits */
	IMAGE_MS_P010, /**< 2 plane Simiplanar 10 bits, MSB valid */
	IMAGE_P010, /**< 3 plane YUV420 10 bits, MSB valid */
	IMAGE_Y210, /**< packed YUV422 10 bits, LSB valid */
	IMAGE_Y10, /**< single plane 10 bits */
	IMAGE_UV10, /**< single UV interleave plan 10 bits */
	IMAGE_XRGB8888, /**< packed XRGB 8 bits */
	IMAGE_ARGB8888,
	IMAGE_A2R10G10B10,
	IMAGE_X2R10G10B10,
	IMAGE_BAYER10, /**< packed */
	IMAGE_BAYER12,
	IMAGE_BAYER14,
	IMAGE_BAYER16,
	IMAGE_FORMAT_MAX
};

enum viv_dec_compression_format {
	CMVDEC_COMPRESSION_FMT_ARGB8 = 0x0,
	CMVDEC_COMPRESSION_FMT_XRGB8 = 0x1,
	CMVDEC_COMPRESSION_FMT_AYUV = 0x2,
	CMVDEC_COMPRESSION_FMT_UYVY = 0x3,
	CMVDEC_COMPRESSION_FMT_YUY2 = 0x4,
	CMVDEC_COMPRESSION_FMT_YUV_ONLY = 0x5,
	CMVDEC_COMPRESSION_FMT_UV_MIX = 0x6,
	CMVDEC_COMPRESSION_FMT_ARGB4 = 0x7,
	CMVDEC_COMPRESSION_FMT_XRGB4 = 0x8,
	CMVDEC_COMPRESSION_FMT_A1RGB5 = 0x9,
	CMVDEC_COMPRESSION_FMT_X1RGB5 = 0x0A,
	CMVDEC_COMPRESSION_FMT_R5G6B5 = 0x0B,
	CMVDEC_COMPRESSION_FMT_A2R10G10B10 = 0x0F,
	CMVDEC_COMPRESSION_FMT_BAYER = 0x10,
	CMVDEC_COMPRESSION_FMT_ARGB16 = 0x13,
	CMVDEC_COMPRESSION_FMT_X2RGB10 = 0x15,
	CMVDEC_COMPRESSION_FMT_MAX
};

/**
 * dec400 tile mode, defines the tile size for each compression block
 */
enum viv_dec_tile_mode {
	CMVDEC_TILE_MODE_8X8_XMAJOR = 0x0,
	CMVDEC_TILE_MODE_8X8_YMAJOR = 0x1,
	CMVDEC_TILE_MODE_16X4 = 0x2,
	CMVDEC_TILE_MODE_8X4 = 0x3,
	CMVDEC_TILE_MODE_4X8 = 0x4,
	CMVDEC_TILE_MODE_4X4 = 0x5,
	CMVDEC_TILE_MODE_RASTER16X4 = 0x6,
	CMVDEC_TILE_MODE_64X4 = 0x7,
	CMVDEC_TILE_MODE_32X4 = 0x8,
	CMVDEC_TILE_MODE_RASTER256X1 = 0x9,
	CMVDEC_TILE_MODE_RASTER128X1 = 0x0A,
	CMVDEC_TILE_MODE_RASTER64X4 = 0x0B,
	CMVDEC_TILE_MODE_RASTER256X2 = 0x0C,
	CMVDEC_TILE_MODE_RASTER128X2 = 0x0D,
	CMVDEC_TILE_MODE_RASTER128X4 = 0x0E,
	CMVDEC_TILE_MODE_RASTER64X1 = 0x0F,
	CMVDEC_TILE_MODE_16X8 = 0x10,
	CMVDEC_TILE_MODE_8X16 = 0x11,
	CMVDEC_TILE_MODE_RASTER512X1 = 0x12,
	CMVDEC_TILE_MODE_RASTER32X4 = 0x13,
	CMVDEC_TILE_MODE_RASTER64X2 = 0x14,
	CMVDEC_TILE_MODE_RASTER32X2 = 0x15,
	CMVDEC_TILE_MODE_RASTER32X1 = 0x16,
	CMVDEC_TILE_MODE_RASTER16X1 = 0x17,
	CMVDEC_TILE_MODE_128X4 = 0x18,
	CMVDEC_TILE_MODE_256X4 = 0x19,
	CMVDEC_TILE_MODE_512X4 = 0x1A,
	CMVDEC_TILE_MODE_16X16 = 0x1B,
	CMVDEC_TILE_MODE_32X16 = 0x1C,
	CMVDEC_TILE_MODE_64X16 = 0x1D,
	CMVDEC_TILE_MODE_128X8 = 0x1E,
	CMVDEC_TILE_MODE_8X4_S = 0x1F,
	CMVDEC_TILE_MODE_16X4_S = 0x20,
	CMVDEC_TILE_MODE_32X4_S = 0x21,
	CMVDEC_TILE_MODE_16X4_LSB = 0x22,
	CMVDEC_TILE_MODE_32X4_LSB = 0x23,
	CMVDEC_TILE_MODE_32X8 = 0x24,
	CMVDEC_TILE_MODE_MAX
};

/**
 * The surface meta data saved in meta data buffer
 */
struct viv_vidmem_metadata {
	u32 magic; /**< __FOURCC('v', 'i', 'v', 'm') */
	u32 dmabuf_size; /**< DMABUF buffer size in byte (Maximum 4GB) */
	u32 time_stamp; /**< time stamp for the DMABUF buffer */

	u32 compressed; /**< if DMABUF buffer is compressed by DEC400 */
	u32 image_format; /**< ImageFormat, determined plane number. */
	struct {
		u32 offset; /**< plane buffer address offset */
		u32 stride; /**< pitch in bytes */
		u32 width; /**< width in pixels */
		u32 height; /**< height in pixels */
		u32 compression_format; /**< specified  */
		u32 tile_mode; /**< tile mode for DEC400 */
		s32 ts_fd; /** fd of separated tile status buffer of the plane buffer */
		void *ts_dma_buf; /** deliver this param for multi-progress */
		u32 ts_offset; /** tile status buffer offset within this plane */
		u32 fc_enabled; /** gpu fastclear enabled for the plane buffer */
		u32 fc_value_lower; /** gpu fastclear color value (lower 32 bits) for the buffer */
		u32 fc_value_upper;/** gpu fastclear color value (upper 32 bits) for the buffer */
		u32 header_size; /** specify dec400 header data size in bytes  */
	} plane[3];
	u32 reserved[8];
};

#endif /* _FTV310_VPU_METADATA_H_ */
