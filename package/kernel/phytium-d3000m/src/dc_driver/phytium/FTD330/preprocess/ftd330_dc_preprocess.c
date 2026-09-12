// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_preprocess.h"

#include "ftd330_dc_hw.h"
#include "ftd330_type.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc.h"


static u16 DEGAMMA_709[DEGAMMA_SIZE] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0001, 0x0002, 0x0004, 0x0005, 0x0007, 0x000a, 0x000d,
	0x0011, 0x0015, 0x0019, 0x001e, 0x0024, 0x002a, 0x0030, 0x0038, 0x003f, 0x0048, 0x0051,
	0x005a, 0x0064, 0x006f, 0x007b, 0x0087, 0x0094, 0x00a1, 0x00af, 0x00be, 0x00ce, 0x00de,
	0x00ef, 0x0101, 0x0114, 0x0127, 0x013b, 0x0150, 0x0166, 0x017c, 0x0193, 0x01ac, 0x01c4,
	0x01de, 0x01f9, 0x0214, 0x0230, 0x024d, 0x026b, 0x028a, 0x02aa, 0x02ca, 0x02ec, 0x030e,
	0x0331, 0x0355, 0x037a, 0x03a0, 0x03c7, 0x03ef, 0x0418, 0x0441, 0x046c, 0x0498, 0x04c4,
	0x04f2, 0x0520, 0x0550, 0x0581, 0x05b2, 0x05e5, 0x0618, 0x064d, 0x0682, 0x06b9, 0x06f0,
	0x0729, 0x0763, 0x079d, 0x07d9, 0x0816, 0x0854, 0x0893, 0x08d3, 0x0914, 0x0956, 0x0999,
	0x09dd, 0x0a23, 0x0a69, 0x0ab1, 0x0afa, 0x0b44, 0x0b8f, 0x0bdb, 0x0c28, 0x0c76, 0x0cc6,
	0x0d17, 0x0d69, 0x0dbb, 0x0e10, 0x0e65, 0x0ebb, 0x0f13, 0x0f6c, 0x0fc6, 0x1021, 0x107d,
	0x10db, 0x113a, 0x119a, 0x11fb, 0x125d, 0x12c1, 0x1325, 0x138c, 0x13f3, 0x145b, 0x14c5,
	0x1530, 0x159c, 0x160a, 0x1678, 0x16e8, 0x175a, 0x17cc, 0x1840, 0x18b5, 0x192b, 0x19a3,
	0x1a1c, 0x1a96, 0x1b11, 0x1b8e, 0x1c0c, 0x1c8c, 0x1d0c, 0x1d8e, 0x1e12, 0x1e96, 0x1f1c,
	0x1fa3, 0x202c, 0x20b6, 0x2141, 0x21ce, 0x225c, 0x22eb, 0x237c, 0x240e, 0x24a1, 0x2536,
	0x25cc, 0x2664, 0x26fc, 0x2797, 0x2832, 0x28cf, 0x296e, 0x2a0e, 0x2aaf, 0x2b51, 0x2bf5,
	0x2c9b, 0x2d41, 0x2dea, 0x2e93, 0x2f3e, 0x2feb, 0x3099, 0x3148, 0x31f9, 0x32ab, 0x335f,
	0x3414, 0x34ca, 0x3582, 0x363c, 0x36f7, 0x37b3, 0x3871, 0x3930, 0x39f1, 0x3ab3, 0x3b77,
	0x3c3c, 0x3d02, 0x3dcb, 0x3e94, 0x3f5f, 0x402c, 0x40fa, 0x41ca, 0x429b, 0x436d, 0x4442,
	0x4517, 0x45ee, 0x46c7, 0x47a1, 0x487d, 0x495a, 0x4a39, 0x4b19, 0x4bfb, 0x4cde, 0x4dc3,
	0x4eaa, 0x4f92, 0x507c, 0x5167, 0x5253, 0x5342, 0x5431, 0x5523, 0x5616, 0x570a, 0x5800,
	0x58f8, 0x59f1, 0x5aec, 0x5be9, 0x5ce7, 0x5de6, 0x5ee7, 0x5fea, 0x60ef, 0x61f5, 0x62fc,
	0x6406, 0x6510, 0x661d, 0x672b, 0x683b, 0x694c, 0x6a5f, 0x6b73, 0x6c8a, 0x6da2, 0x6ebb,
	0x6fd6, 0x70f3, 0x7211, 0x7331, 0x7453, 0x7576, 0x769b, 0x77c2, 0x78ea, 0x7a14, 0x7b40,
	0x7c6d, 0x7d9c, 0x7ecd, 0x3f65, 0x3f8c, 0x3fb2, 0x3fd8
};
static u16 DEGAMMA_2020[DEGAMMA_SIZE] = {
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001, 0x0001,
	0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0004, 0x0004,
	0x0004, 0x0005, 0x0005, 0x0006, 0x0006, 0x0006, 0x0007, 0x0007, 0x0008, 0x0008, 0x0009,
	0x000a, 0x000a, 0x000b, 0x000c, 0x000c, 0x000d, 0x000e, 0x000f, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0x0016, 0x0017, 0x0018, 0x0019, 0x001b, 0x001c, 0x001e, 0x001f,
	0x0021, 0x0022, 0x0024, 0x0026, 0x0028, 0x002a, 0x002c, 0x002e, 0x0030, 0x0033, 0x0035,
	0x0038, 0x003a, 0x003d, 0x0040, 0x0043, 0x0046, 0x0049, 0x004d, 0x0050, 0x0054, 0x0057,
	0x005b, 0x005f, 0x0064, 0x0068, 0x006d, 0x0071, 0x0076, 0x007c, 0x0081, 0x0086, 0x008c,
	0x0092, 0x0098, 0x009f, 0x00a5, 0x00ac, 0x00b4, 0x00bb, 0x00c3, 0x00cb, 0x00d3, 0x00dc,
	0x00e5, 0x00ee, 0x00f8, 0x0102, 0x010c, 0x0117, 0x0123, 0x012e, 0x013a, 0x0147, 0x0154,
	0x0161, 0x016f, 0x017e, 0x018d, 0x019c, 0x01ac, 0x01bd, 0x01ce, 0x01e0, 0x01f3, 0x0206,
	0x021a, 0x022f, 0x0244, 0x025a, 0x0272, 0x0289, 0x02a2, 0x02bc, 0x02d6, 0x02f2, 0x030f,
	0x032c, 0x034b, 0x036b, 0x038b, 0x03ae, 0x03d1, 0x03f5, 0x041b, 0x0443, 0x046b, 0x0495,
	0x04c1, 0x04ee, 0x051d, 0x054e, 0x0580, 0x05b4, 0x05ea, 0x0622, 0x065c, 0x0698, 0x06d6,
	0x0717, 0x075a, 0x079f, 0x07e7, 0x0831, 0x087e, 0x08cd, 0x0920, 0x0976, 0x09ce, 0x0a2a,
	0x0a89, 0x0aec, 0x0b52, 0x0bbc, 0x0c2a, 0x0c9b, 0x0d11, 0x0d8b, 0x0e0a, 0x0e8d, 0x0f15,
	0x0fa1, 0x1033, 0x10ca, 0x1167, 0x120a, 0x12b2, 0x1360, 0x1415, 0x14d1, 0x1593, 0x165d,
	0x172e, 0x1806, 0x18e7, 0x19d0, 0x1ac1, 0x1bbb, 0x1cbf, 0x1dcc, 0x1ee3, 0x2005, 0x2131,
	0x2268, 0x23ab, 0x24fa, 0x2656, 0x27be, 0x2934, 0x2ab8, 0x2c4a, 0x2dec, 0x2f9d, 0x315f,
	0x3332, 0x3516, 0x370d, 0x3916, 0x3b34, 0x3d66, 0x3fad, 0x420b, 0x4480, 0x470d, 0x49b3,
	0x4c73, 0x4f4e, 0x5246, 0x555a, 0x588e, 0x5be1, 0x5f55, 0x62eb, 0x66a6, 0x6a86, 0x6e8c,
	0x72bb, 0x7714, 0x7b99, 0x3dcb, 0x3e60, 0x3ef5, 0x3f8c
};

static bool scale_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			const void *obj_state)
{
	const struct dc_hw_plane *hw_plane = ftd330_dc_hw_get_plane(hw, hw_id);
	const struct ftd330_plane_info *plane_info = hw_plane->info;
	int factor_x, factor_y;
	const struct drm_ftd330_scale_config *scale = data;

	factor_x = (scale->src_w << 16) / scale->dst_w;
	factor_y = (scale->src_h << 16) / scale->dst_h;

	if (factor_x < plane_info->min_scale || factor_x > plane_info->max_scale ||
		factor_y < plane_info->min_scale || factor_y > plane_info->max_scale) {
		pr_err("%s the scale factor out of range.\n", __func__);
		return false;
	}

	return true;
}

static bool scale_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_scale_config *scale = data;


	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (primary)
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG + offset, !!(enable) << 22, 0);
	else
		dc_set_clear(hw, DC_OVERLAY_SCALE_CONFIG + offset, !!(enable) << 8, 0);

	if (enable) {
		dc_write(hw, reg->scale_factor_x + offset, scale->factor_x);
		dc_write(hw, reg->scale_factor_y + offset, scale->factor_y);
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(scale_proto, "SCALER", struct drm_ftd330_scale_config, scale_check, NULL,
			  scale_config_hw);


static bool colorkey_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			   const void *obj_state)
{
	const struct dc_hw_plane *hw_plane = ftd330_dc_hw_get_plane(hw, hw_id);
	const struct ftd330_plane_info *plane_info = hw_plane->info;

	if (!plane_info->color_mgmt) {
		pr_err("%s The plane is not support colorkey.\n", __func__);
		return false;
	}
	return true;
}

static bool colorkey_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_colorkey *colorkey_data = data;
	u8 transparency;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);
	transparency = (colorkey_data->transparency) ? DC_TRANSPARENCY_KEY : DC_TRANSPARENCY_OPAQUE;

	if (enable) {
		dc_write(hw, reg->color_key + offset, colorkey_data->colorkey);
		dc_write(hw, reg->color_key_high + offset, colorkey_data->colorkey_high);

		if (primary)
			dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG + offset, transparency << 9,
				     0x03 << 9);
		else
			dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, transparency, 0x03);
	}
	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(colorkey_proto, "COLORKEY", struct drm_ftd330_colorkey, colorkey_check, NULL,
			  colorkey_config_hw);

static bool clear_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			const void *obj_state)
{
	const struct dc_hw_plane *hw_plane = ftd330_dc_hw_get_plane(hw, hw_id);
	const struct ftd330_plane_info *plane_info = hw_plane->info;

	if (!plane_info->color_mgmt) {
		pr_err("%s The plane is not support clear.\n", __func__);
		return false;
	}

	return true;
}

static bool clear_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_color *color = data;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (primary)
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG + offset, (!!enable << 8), BIT(8));
	else
		dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, (!!enable << 25), BIT(25));

	if (enable) {
		/* Clear is enabled, format is A8R8G8B8. */
		dc_write(hw, reg->clear_value + offset,
			 (color->a & 0xFF) << 24 | (color->r & 0xFF) << 16 |
				 (color->g & 0xFF) << 8 | (color->b & 0xFF));
	}
	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(clear_proto, "CLEAR", struct drm_ftd330_color, clear_check, NULL,
			  clear_config_hw);

#if 0
static bool dma_config_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			     const void *obj_state)
{
	const struct dc_hw_plane *hw_plane = ftd330_dc_hw_get_plane(hw, hw_id);
	const struct ftd330_plane_info *plane_info = hw_plane->info;
	const struct ftd330_plane_state *ftd330_plane_state = obj_state;
	const u32 one_roi = plane_info->roi;
	const u32 fb_w = ftd330_plane_state->base.fb->width;
	const u32 fb_h = ftd330_plane_state->base.fb->height;

	const struct drm_ftd330_dma *dma = data;

	if ((dma->mode != FTD330_DMA_NORMAL) && (!one_roi)) {
		pr_err("%s not support layer ROI.\n", __func__);
		return false;
	}

	switch (dma->mode) {
	case FTD330_DMA_ONE_ROI:
		if (((dma->in_rect[0].w + dma->in_rect[0].x) > fb_w) ||
			((dma->in_rect[0].h + dma->in_rect[0].y) > fb_h)) {
			pr_err("%s ROI area is out of layer area range.\n", __func__);
			return false;
			}
		break;
	case FTD330_DMA_TWO_ROI:
	case FTD330_DMA_SKIP_ROI:
	case FTD330_DMA_EXT_LAYER:
	case FTD330_DMA_EXT_LAYER_EX:
		pr_err("%s has invalid layer ROI mode.\n", __func__);
		return false;
	default:
		break;
	}

	return true;
}

static bool dma_config_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_dma *dma = data;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (dma->mode == FTD330_DMA_ONE_ROI) {
		if (enable) {
			dc_write(hw, reg->roi_origin + offset,
				 dma->in_rect[0].x | (dma->in_rect[0].y << 16));
			dc_write(hw, reg->roi_size + offset,
				 dma->in_rect[0].w | (dma->in_rect[0].h << 16));
			dc_write(hw, reg->top_left + offset,
				 dma->out_rect[0].x | (dma->out_rect[0].y << 15));
			dc_write(hw, reg->bottom_right + offset,
				 (dma->out_rect[0].x + dma->out_rect[0].w) |
					 ((dma->out_rect[0].y + dma->out_rect[0].h) << 15));

			if (primary)
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, BIT(0), 0);
			else
				dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, BIT(22), 0);
		} else {
			if (primary)
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, 0, BIT(0));
			else
				dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, 0, BIT(22));
		}
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(dma_config_proto, "DMA_CONFIG", struct drm_ftd330_dma, dma_config_check, NULL,
			  dma_config_config_hw);
#endif
static void load_default_degamma_table(struct dc_hw *hw, const struct dc_hw_plane_reg *reg,
				       u32 offset, u16 *table)
{
	u16 i;
	u32 value;

	dc_write(hw, reg->degamma_index + offset, 0);
	for (i = 0; i < DEGAMMA_SIZE; i++) {
		value = table[i] | (table[i] << 16);
		dc_write(hw, reg->degamma_data + offset, value);
		dc_write(hw, reg->degamma_ex_data + offset, table[i]);
	}
}
static bool degamma_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_degamma_config *degamma = data;
	u16 i;
	u32 value;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);
	if (primary)
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, enable << 5, BIT(5));
	else
		dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, enable << 28, BIT(28));
	if (enable) {
		if (degamma->mode == FTD330_DEGAMMA_BT709) {
			load_default_degamma_table(hw, reg, offset, DEGAMMA_709);
		} else if (degamma->mode == FTD330_DEGAMMA_BT2020) {
			load_default_degamma_table(hw, reg, offset, DEGAMMA_2020);
		} else if (degamma->mode == FTD330_DEGAMMA_USR) {
			dc_write(hw, reg->degamma_index + offset, 0);
			for (i = 0; i < DEGAMMA_SIZE; i++) {
				value = degamma->b[i] | (degamma->g[i] << 16);
				dc_write(hw, reg->degamma_data + offset, value);
				dc_write(hw, reg->degamma_ex_data + offset, degamma->r[i]);
			}
		}
	}
	return true;
}
FTD330_DC_BLOB_PROPERTY_PROTO(degamma_config_proto, "DEGAMMA_CONFIG", struct drm_ftd330_degamma_config,
			  NULL, NULL, degamma_config_hw);

static bool gamut_map_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			    const void *obj_state)
{
	const struct drm_ftd330_gamut_map *gamut_map = data;

	if (gamut_map->mode > FTD330_GAMUT_USER_DEF) {
		pr_err("%s: Invalid gamut map mode %#x\n", __func__, gamut_map->mode);
		return false;
	}
	if (gamut_map->mode == FTD330_GAMUT_USER_DEF) {
		if (!gamut_map->coef || !sizeof(gamut_map->coef) ||
		    sizeof(gamut_map->coef) > FTD330_MAX_GAMUT_COEF_NUM) {
			pr_err("%s: Invalid user define gamut map coefficient.\n", __func__);
			return false;
		}
	}
	return true;
}
static bool gamut_map_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const struct drm_ftd330_gamut_map *gamut_map = data;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	u32 offset;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (primary)
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, enable << 6, BIT(6));
	else
		dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, enable << 29, BIT(29));
	if (enable) {
		if (!gamut_map->coef) {
			pr_err("%s: Unkonwn mode %#x\n", __func__, gamut_map->mode);
			return false;
		}
		dc_write(hw, reg->RGBToRGBCoef0 + offset,
			 gamut_map->coef[0] | (gamut_map->coef[1] << 16));
		dc_write(hw, reg->RGBToRGBCoef1 + offset,
			 gamut_map->coef[2] | (gamut_map->coef[3] << 16));
		dc_write(hw, reg->RGBToRGBCoef2 + offset,
			 gamut_map->coef[4] | (gamut_map->coef[5] << 16));
		dc_write(hw, reg->RGBToRGBCoef3 + offset,
			 gamut_map->coef[6] | (gamut_map->coef[7] << 16));
		dc_write(hw, reg->RGBToRGBCoef4 + offset, gamut_map->coef[8]);
	}
	return true;
}
FTD330_DC_BLOB_PROPERTY_PROTO(gamut_map_proto, "GAMUT_MAP", struct drm_ftd330_gamut_map, gamut_map_check,
			  NULL, gamut_map_config_hw);


bool ftd330_dc_register_preprocess_states(struct ftd330_dc_property_state_group *states,
				       const struct ftd330_plane_info *info)
{

	if (info->gamut_map)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &gamut_map_proto), on_error);
#if 0
	if (info->roi)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &dma_config_proto), on_error);
#endif
	if (info->color_mgmt) {
		__ERR_CHECK(ftd330_dc_property_register_state(states, &clear_proto), on_error);
		__ERR_CHECK(ftd330_dc_property_register_state(states, &colorkey_proto), on_error);
	}

	if ((info->min_scale != DRM_PLANE_HELPER_NO_SCALING ||
	     info->max_scale != DRM_PLANE_HELPER_NO_SCALING) &&
		(info->min_scale != 0 || info->max_scale != 0))
		__ERR_CHECK(ftd330_dc_property_register_state(states, &scale_proto), on_error);
	if (info->degamma_size)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &degamma_config_proto), on_error);

	return true;

on_error:
	return false;
}
