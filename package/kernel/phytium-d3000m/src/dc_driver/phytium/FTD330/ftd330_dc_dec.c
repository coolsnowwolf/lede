// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/errno.h>
#include <linux/types.h>
#include <linux/io.h>

#include <drm/ftd330_drm_fourcc.h>
#include <drm/drm_framebuffer.h>

#include "ftd330_dc_dec.h"
#include "ftd330_gem.h"
#include "ftd330_plane.h"
#include "ftd330_fb.h"

#define UP_ALIGN(x, align) ((x + align - 1) & ~(align - 1))
#define fourcc_mod_ftd330_get_tile_mode(val) ((u8)((val) & DRM_FORMAT_MOD_FTD330_DEC_TILE_MODE_MASK))

static inline bool _is_stream_changed(struct dc_dec400l *dec400l, u8 stream_id)
{
	return dec400l->stream[stream_id].dirty;
}

static inline bool _is_stream_valid(struct dc_dec400l *dec400l, u8 stream_id)
{
	return !!(dec400l->stream[stream_id].main_base_addr);
}

void phytium_dec_init(struct dc_hw *hw)
{
	if (hw->pipe_mask & BIT(DISPLAY_0)) {
		dc_hw_dec_init(hw, DC_0);
	}

	if (hw->pipe_mask & BIT(DISPLAY_1) ||
			hw->pipe_mask & BIT(DISPLAY_2)) {
		dc_hw_dec_init(hw, DC_1);
	}
}


static void _enable_stream(struct dc_dec400l *dec400l, struct dc_hw *hw, u8 stream_id)
{
	if (!(dec400l->stream_status & (1 << stream_id))) {
		dec400l->stream_status |= 1 << stream_id;
	}
}


static void _disable_stream(struct dc_dec400l *dec400l, struct dc_hw *hw, u8 stream_id)
{
	if ((dec400l->stream_status & (1 << stream_id)))
		dec400l->stream_status &= ~(1 << stream_id);
}

static u16 get_dec_tile_size(u8 tile_mode, u8 cpp)
{
	u16 multi = 0;

	switch (tile_mode) {
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_16X1:
		multi = 16;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_4X8:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_32X1:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4_S:
		multi = 32;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_XMAJOR:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_YMAJOR:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_16X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_32X2:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4_S:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4_LSB:
		multi = 64;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X16:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_32X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X2:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X4_S:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X4_LSB:
		multi = 128;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_64X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X2:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X16:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X8:
		multi = 256;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X2:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X4:
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_512X1:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_128X4:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X16:
		multi = 512;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_256X4:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_64X16:
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_128X8:
		multi = 1024;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_512X4:
		multi = 2048;
		break;
	default:
		break;
	}

	return multi * cpp;
}

static void update_fb_format(struct dc_dec_fb *dec_fb)
{
	struct drm_framebuffer *drm_fb = dec_fb->fb;
	u8 tile_mod = fourcc_mod_ftd330_get_tile_mode(drm_fb->modifier);
	u8 norm_mod = DRM_FORMAT_MOD_FTD330_LINEAR;

	if (drm_fb->modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC)
		tile_mod = DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4;
	switch (tile_mod) {
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_32X1:
		norm_mod = DRM_FORMAT_MOD_FTD330_TILE_32X1;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1:
		norm_mod = DRM_FORMAT_MOD_FTD330_TILE_64X1;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1:
		norm_mod = DRM_FORMAT_MOD_FTD330_TILE_128X1;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1:
		norm_mod = DRM_FORMAT_MOD_FTD330_TILE_256X1;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4:
		norm_mod = DRM_FORMAT_MOD_FTD330_SUPER_TILED_XMAJOR_8X4;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_4X8:
		norm_mod = DRM_FORMAT_MOD_FTD330_SUPER_TILED_YMAJOR_4X8;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_XMAJOR:
		if (drm_fb->format->format == DRM_FORMAT_YUYV ||
		    drm_fb->format->format == DRM_FORMAT_UYVY
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
		    || drm_fb->format->format == DRM_FORMAT_P010
#endif
		)
			norm_mod = DRM_FORMAT_MOD_FTD330_TILE_8X8;
		else
			norm_mod = DRM_FORMAT_MOD_FTD330_SUPER_TILED_XMAJOR;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8:
		if (drm_fb->format->format == DRM_FORMAT_NV12 ||
			drm_fb->format->format == DRM_FORMAT_NV21
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
			|| drm_fb->format->format == DRM_FORMAT_P010
#endif
			)
			norm_mod = DRM_FORMAT_MOD_FTD330_TILE_8X8;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X8:
		if (drm_fb->format->format == DRM_FORMAT_NV12 ||
			drm_fb->format->format == DRM_FORMAT_NV21)
			norm_mod = DRM_FORMAT_MOD_FTD330_TILE_8X8;
		break;
	default:
		break;
	}

	drm_fb->modifier = fourcc_mod_ftd330_norm_code(norm_mod);
}


static void drm_phy_get_align_size(uint32_t *width, uint32_t *height, uint32_t format, uint64_t mod)
{
	uint8_t tile_mode;
	uint32_t ori_width = *width;
	uint32_t ori_height = *height;

	if (mod == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC)
		tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4;
	else
		tile_mode = fourcc_mod_ftd330_get_tile_mode(mod);

	/*alignment requirements for dec400 sub-IP*/
	switch (tile_mode) {
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_XMAJOR:
		if (format == DRM_FORMAT_YUYV || format == DRM_FORMAT_UYVY ||
		    format == DRM_FORMAT_P010) {
			*width = UP_ALIGN(ori_width, 16);
			*height = UP_ALIGN(ori_height, 8);
		} else {
			*width = UP_ALIGN(ori_width, 64);
			*height = UP_ALIGN(ori_height, 64);
		}
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4:
		*width = UP_ALIGN(ori_width, 64);
		*height = UP_ALIGN(ori_height, 64);
		break;
	default:
		break;
	}
}



static void _stream_config(struct dc_dec_fb *dec_fb, struct dc_dec_stream *stream, u8 index)
{
	struct drm_framebuffer *drm_fb = dec_fb->fb;
	const struct drm_format_info *info = drm_fb->format;
	u32 plane_width;
	u16 tile_size;
	struct ftd330_ts_metadata *metadata = NULL;
	struct ftd330_gem_object *ftd330_obj = NULL;
	u32 plane_height;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	plane_height = drm_format_info_plane_height(info, drm_fb->height, index);
    plane_width = drm_format_info_plane_width(info, drm_fb->width, index);
#else
	if (index == 0) {
		plane_height = drm_fb->height;
        plane_width = drm_fb->width;
    } else {
		plane_height = drm_fb->height / info->vsub;
        plane_width = drm_fb->width / info->hsub;
    }
#endif
	stream->main_base_addr = dec_fb->addr[index];
	if (drm_fb->modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC)
		stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4;
	else
		stream->tile_mode = fourcc_mod_ftd330_get_tile_mode(drm_fb->modifier);
	if ((fourcc_mod_ftd330_get_align_mode(drm_fb->modifier) == DRM_FORMAT_MOD_FTD330_DEC_ALIGN_32) ||
		drm_fb->modifier == DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC) {
		stream->align_mode = DEC_ALIGN_32;
	} else {
		stream->align_mode = DEC_ALIGN_64;
	}

	switch (drm_fb->format->format) {
	case DRM_FORMAT_ARGB8888:
		stream->format = DEC_FORMAT_ARGB8;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_XRGB8888:
		stream->format = DEC_FORMAT_XRGB8;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_RGB565:
		stream->format = DEC_FORMAT_R5G6B5;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_ARGB1555:
		stream->format = DEC_FORMAT_A1RGB5;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_XRGB1555:
		stream->format = DEC_FORMAT_X1RGB5;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_ARGB4444:
		stream->format = DEC_FORMAT_ARGB4;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_XRGB4444:
		stream->format = DEC_FORMAT_XRGB4;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_ARGB2101010:
		stream->format = DEC_FORMAT_A2R10G10B10;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_YUYV:
		stream->format = DEC_FORMAT_YUY2;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_UYVY:
		stream->format = DEC_FORMAT_UYVY;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_YVU420:
	case DRM_FORMAT_YUV420:
		stream->format = DEC_FORMAT_YUV_ONLY;
		stream->depth = DEC_DEPTH_8;
		break;
	case DRM_FORMAT_NV12:
	case DRM_FORMAT_NV21:
		WARN_ON(stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_TILE_32X8 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8);
		if (index) {
			stream->format = DEC_FORMAT_UV_MIX;
			switch (stream->tile_mode) {
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X8:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_32X4;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4;
				break;
			default:
				break;
			}
		} else {
			stream->format = DEC_FORMAT_YUV_ONLY;
		}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
		stream->depth = DEC_DEPTH_8;
#endif
		break;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18)
	case DRM_FORMAT_P010:
		WARN_ON(stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_XMAJOR);
		if (index) {
			stream->format = DEC_FORMAT_UV_MIX;
			switch (stream->tile_mode) {
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_32X1;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X8:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8_XMAJOR:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_TILE_8X4;
				break;
			default:
				break;
			}
		} else {
			stream->format = DEC_FORMAT_YUV_ONLY;
		}

		stream->depth = DEC_DEPTH_10;
		break;
#endif
	case DRM_FORMAT_NV16:
		WARN_ON(stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1 &&
			stream->tile_mode != DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1);
		if (index) {
			stream->format = DEC_FORMAT_UV_MIX;
			switch (stream->tile_mode) {
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_256X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1;
				break;
			case DRM_FORMAT_MOD_FTD330_DEC_RASTER_128X1:
				stream->tile_mode = DRM_FORMAT_MOD_FTD330_DEC_RASTER_64X1;
				break;
			default:
				break;
			}
		} else {
			stream->format = DEC_FORMAT_YUV_ONLY;
		}

		stream->depth = DEC_DEPTH_8;
		break;
	}

	tile_size = get_dec_tile_size(stream->tile_mode, info->cpp[index]);
	stream->aligned_stride = ALIGN(dec_fb->stride[index], tile_size);

	drm_phy_get_align_size(&plane_width, &plane_height, drm_fb->format->format,drm_fb->modifier);
	if (dec_fb->ts_dma_buf[index]) {
		ftd330_obj = ftd330_fb_get_gem_obj(drm_fb, index);
		metadata = ftd330_obj->base.dma_buf->priv;
		stream->ts_base_addr = dec_fb->ts_addr[index] + 128;
		stream->clear_color = ((u64)metadata->plane[index].fc_value_upper << 32) |
					(u64)metadata->plane[index].fc_value_lower;
		stream->format = metadata->plane[index].compression_format;
	} else {
		stream->ts_base_addr =
			stream->main_base_addr + stream->aligned_stride * plane_height;
	}
	stream->main_end_addr =
		stream->main_base_addr + stream->aligned_stride * plane_height - 1;
}

static int _dec400l_config(struct dc_dec400l *dec400l, struct dc_dec_fb *dec_fb, u8 stream_base)
{
	struct dc_dec_stream stream;
	u8 i, stream_id, num_planes = 0;

	if (dec_fb) {
		const struct drm_format_info *info = dec_fb->fb->format;

		num_planes = info->num_planes;
	}

	for (i = 0; i < STREAM_COUNT; i++) {
		stream_id = stream_base + i;

		memset(&dec400l->stream[stream_id], 0, sizeof(struct dc_dec_stream));

		if (i < num_planes) {
			memset(&stream, 0, sizeof(struct dc_dec_stream));
			_stream_config(dec_fb, &stream, i);
			memcpy(&dec400l->stream[stream_id], &stream, sizeof(struct dc_dec_stream));
		}
		dec400l->stream[stream_id].dirty = true;
	}

	if (dec_fb)
		update_fb_format(dec_fb);

	return 0;
}

int dc_dec_config(struct dc_dec400l *dec400l, struct dc_dec_fb *dec_fb, u8 stream_base)
{
	if (dec_fb && !dec_fb->fb)
		return -EINVAL;

	if (dec_fb &&
	    (fourcc_mod_ftd330_get_type(dec_fb->fb->modifier) != DRM_FORMAT_MOD_FTD330_TYPE_COMPRESSED) &&
	    (dec_fb->fb->modifier != DRM_FORMAT_MOD_PHYTIUM_SUPER_TILED_FC))
		_dec400l_config(dec400l, NULL, stream_base);
	else
		_dec400l_config(dec400l, dec_fb, stream_base);

	return 0;
}

int dc_dec_commit(struct dc_dec400l *dec400l, struct dc_hw *hw)
{
	u8 i;
	u32 stream_total = 0;

	if (hw->total_pipes == 1)
		stream_total = 9;
	else if (hw->total_pipes == 2)
		stream_total = 18;
	else
		stream_total = 27;

	for (i = 0; i < stream_total; i++) {
		if (!_is_stream_changed(dec400l, i))
			continue;

		if (_is_stream_valid(dec400l, i)) {
			_enable_stream(dec400l, hw, i);
			dc_hw_dec_stream_set(hw, dec400l->stream[i].main_base_addr,
					     dec400l->stream[i].main_end_addr,
					     dec400l->stream[i].ts_base_addr,
					     dec400l->stream[i].clear_color,
					     dec400l->stream[i].tile_mode,
					     dec400l->stream[i].align_mode,
					     dec400l->stream[i].format, dec400l->stream[i].depth,
					     i);
		} else {
			dc_hw_dec_stream_disable(hw, i);
			_disable_stream(dec400l, hw, i);
		}

		dec400l->stream[i].dirty = false;
	}

	return 0;
}
