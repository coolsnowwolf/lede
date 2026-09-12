// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/errno.h>
#include <linux/types.h>

#include "ftd330_drm_fourcc.h"

#include "ftd330_dc_pvric.h"

#define fourcc_mod_ftd330_get_type(val) (((val)&DRM_FORMAT_MOD_FTD330_TYPE_MASK) >> 53)
#define fourcc_mod_ftd330_get_tile_mode(val) ((u8)((val)&DRM_FORMAT_MOD_FTD330_DEC_TILE_MODE_MASK))

static inline void update_pvric_format(struct dc_pvric_reqt *reqt, u32 format)
{
	u8 f = PVRIC_FORMAT_ABGR8888;

	switch (format) {
	case DRM_FORMAT_RGB565:
	case DRM_FORMAT_BGR565:
		f = PVRIC_FORMAT_RGB565;
		break;
	case DRM_FORMAT_ARGB2101010:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_RGBA1010102:
	case DRM_FORMAT_BGRA1010102:
		f = PVRIC_FORMAT_ARGB2101010;
		break;
	case DRM_FORMAT_ARGB16161616F:
	case DRM_FORMAT_ABGR16161616F:
		f = PVRIC_FORMAT_FP16;
		break;
	case DRM_FORMAT_NV12:
		f = PVRIC_FORMAT_YVU420_2PLANE;
		break;
	case DRM_FORMAT_NV21:
		f = PVRIC_FORMAT_YUV420_2PLANE;
		break;
	case DRM_FORMAT_P010:
		f = PVRIC_FORMAT_YUV420_BIT10_PACK16;
		break;
	default:
		break;
	}

	reqt->format = f;
}

static inline void update_pvric_swizzle(struct dc_pvric_reqt *pvric, u32 format)
{
	u8 swizzle = PVRIC_SWIZZLE_ARGB;

	switch (format) {
	case DRM_FORMAT_BGR565:
	case DRM_FORMAT_XBGR8888:
	case DRM_FORMAT_ABGR8888:
	case DRM_FORMAT_ABGR2101010:
	case DRM_FORMAT_ABGR16161616F:
		swizzle = PVRIC_SWIZZLE_ABGR;
		break;
	case DRM_FORMAT_RGBX8888:
	case DRM_FORMAT_RGBA8888:
	case DRM_FORMAT_RGBA1010102:
		swizzle = PVRIC_SWIZZLE_RGBA;
		break;
	case DRM_FORMAT_BGRX8888:
	case DRM_FORMAT_BGRA8888:
	case DRM_FORMAT_BGRA1010102:
		swizzle = PVRIC_SWIZZLE_BGRA;
		break;
	default:
		break;
	}

	pvric->swizzle = swizzle;
}

static inline void update_pvric_tile(struct dc_pvric_reqt *pvric, u8 tile)
{
	u8 tile_mode = PVRIC_TILE_RESERVED;

	switch (tile) {
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8:
		tile_mode = PVRIC_TILE_8X8;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4:
		tile_mode = PVRIC_TILE_16X4;
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X2:
		tile_mode = PVRIC_TILE_32X2;
		break;
	default:
		break;
	}

	pvric->tile_mode = tile_mode;
}

static void update_fb_modifier(struct drm_framebuffer *drm_fb)
{
	u32 format = drm_fb->format->format;
	u8 tile_mod = fourcc_mod_ftd330_get_tile_mode(drm_fb->modifier);
	u8 norm_mode = DRM_FORMAT_MOD_FTD330_LINEAR;

	switch (tile_mod) {
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_8X8:
		switch (format) {
		case DRM_FORMAT_NV12:
		case DRM_FORMAT_NV21:
			norm_mode = DRM_FORMAT_MOD_FTD330_TILE_32X8;
			break;
		case DRM_FORMAT_P010:
			norm_mode = DRM_FORMAT_MOD_FTD330_TILE_16X8;
			break;
		default:
			break;
		}
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_16X4:
		switch (format) {
		case DRM_FORMAT_ARGB8888:
		case DRM_FORMAT_ABGR8888:
		case DRM_FORMAT_RGBA8888:
		case DRM_FORMAT_BGRA8888:
		case DRM_FORMAT_XRGB8888:
		case DRM_FORMAT_XBGR8888:
		case DRM_FORMAT_RGBX8888:
		case DRM_FORMAT_BGRX8888:
		case DRM_FORMAT_ARGB2101010:
		case DRM_FORMAT_ABGR2101010:
		case DRM_FORMAT_RGBA1010102:
		case DRM_FORMAT_BGRA1010102:
			norm_mode = DRM_FORMAT_MOD_FTD330_TILE_16X4;
			break;
		case DRM_FORMAT_RGB565:
		case DRM_FORMAT_BGR565:
			norm_mode = DRM_FORMAT_MOD_FTD330_TILE_32X4;
			break;
		default:
			break;
		}
		break;
	case DRM_FORMAT_MOD_FTD330_DEC_TILE_32X2:
		switch (format) {
		case DRM_FORMAT_ARGB16161616F:
		case DRM_FORMAT_ABGR16161616F:
			norm_mode = DRM_FORMAT_MOD_FTD330_TILE_32X2;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	drm_fb->modifier = fourcc_mod_ftd330_norm_code(norm_mode);
}

static void _pvric_reqt_config(struct dc_pvric_reqt *reqt, struct drm_framebuffer *drm_fb)
{
	const struct drm_format_info *info = drm_fb->format;
	u8 tile_mode;

	reqt->req_num = info->num_planes;
	tile_mode = fourcc_mod_ftd330_get_tile_mode(drm_fb->modifier);

	update_pvric_format(reqt, drm_fb->format->format);
	update_pvric_swizzle(reqt, drm_fb->format->format);
	update_pvric_tile(reqt, tile_mode);

	if (drm_fb->modifier & DRM_FORMAT_MOD_FTD330_DEC_LOSSY)
		reqt->lossy = true;
	else
		reqt->lossy = false;

	reqt->dirty = true;

	update_fb_modifier(drm_fb);
}

static void plane_pvric_commit(struct dc_pvric *pvric, struct dc_hw *hw, u8 id)
{
	if (pvric->reqt.dirty) {
		dc_hw_set_plane_fbc_enable(hw, id, pvric->reqt.enable);

		if (pvric->reqt.enable)
			dc_hw_set_plane_fbc_req(hw, id, pvric->reqt.address, pvric->reqt.req_num,
					    pvric->reqt.format, pvric->reqt.swizzle,
					    pvric->reqt.tile_mode, pvric->reqt.lossy);

		pvric->reqt.dirty = false;
	}

	if (pvric->clear.dirty) {
		dc_hw_set_plane_fbc_clear(hw, id, pvric->clear.color, pvric->reqt.req_num);

		pvric->clear.dirty = false;
	}

	if (pvric->const_cfg.dirty) {
		dc_hw_set_plane_fbc_const(hw, id, pvric->const_cfg.value, pvric->reqt.format);

		pvric->const_cfg.dirty = false;
	}
}

int dc_pvric_reqt_config(struct dc_pvric_reqt *reqt, struct drm_framebuffer *drm_fb)
{
	if (!reqt->enable) {
		reqt->dirty = true;
		return 0;
	}

	if (!drm_fb || fourcc_mod_ftd330_get_type(drm_fb->modifier) != DRM_FORMAT_MOD_FTD330_TYPE_PVRIC)
		return -EINVAL;

	_pvric_reqt_config(reqt, drm_fb);

	return 0;
}

int dc_pvric_clear_config(struct dc_pvric_clear *clear, struct drm_ftd330_pvric_clear *data)
{
	if (!data)
		return -EINVAL;

	memcpy(clear, data, PVRIC_PLANE_MAX * sizeof(u64));

	clear->dirty = true;

	return 0;
}

int dc_pvric_const_config(struct dc_pvric_const *const_cfg, struct drm_ftd330_pvric_const *data)
{
	if (!data)
		return -EINVAL;

	memcpy(const_cfg, data, sizeof(*data));

	const_cfg->dirty = true;

	return 0;
}

int dc_pvric_commit(struct dc_pvric *pvric, struct dc_hw *hw, u8 pvric_type, u8 id)
{
	if (pvric_type == PVRIC_PLANE)
		plane_pvric_commit(pvric, hw, id);

	/* TODO: display brightness, blur, rcd compression data processing.
	 * This part will be supported in parallel with the development of
	 * the corresponding modules.
	 */

	return 0;
}
