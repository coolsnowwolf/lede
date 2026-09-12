// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_plane_blender.h"

#include "drm/ftd330_drm.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_hw.h"

static bool blend_mode_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			     const void *obj_state)
{
	const struct drm_ftd330_blend *bld = data;


	if (bld->color_mode > FTD330_BLD_XOR) {
		pr_err("%s doesn't support this blend mode.\n", __func__);
		return false;
	}
	return true;
}

static bool blend_mode_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_blend *bld = data;
	u8 src_blend_mode = BLENDING_ZERO;
	u8 dst_blend_mode = BLENDING_ZERO;


	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	dc_write(hw, reg->blend_config + offset, !enable << 1);

	if (enable) {
		switch (bld->color_mode) {
		case FTD330_BLD_CLR:
			src_blend_mode = BLENDING_ZERO;
			dst_blend_mode = BLENDING_ZERO;
			break;
		case FTD330_BLD_SRC:
			src_blend_mode = BLENDING_ONE;
			dst_blend_mode = BLENDING_ZERO;
			break;
		case FTD330_BLD_DST:
			src_blend_mode = BLENDING_ZERO;
			dst_blend_mode = BLENDING_ONE;
			break;
		case FTD330_BLD_SRC_OVR:
			src_blend_mode = BLENDING_ONE;
			dst_blend_mode = BLENDING_INVERSED;
			break;
		case FTD330_BLD_DST_OVR:
			src_blend_mode = BLENDING_INVERSED;
			dst_blend_mode = BLENDING_ONE;
			break;
		case FTD330_BLD_SRC_IN:
			src_blend_mode = BLENDING_NORMAL;
			dst_blend_mode = BLENDING_ZERO;
			break;
		case FTD330_BLD_DST_IN:
			src_blend_mode = BLENDING_ZERO;
			dst_blend_mode = BLENDING_NORMAL;
			break;
		case FTD330_BLD_SRC_OUT:
			src_blend_mode = BLENDING_INVERSED;
			dst_blend_mode = BLENDING_ZERO;
			break;
		case FTD330_BLD_DST_OUT:
			src_blend_mode = BLENDING_ZERO;
			dst_blend_mode = BLENDING_INVERSED;
			break;
		case FTD330_BLD_SRC_ATOP:
			src_blend_mode = BLENDING_NORMAL;
			dst_blend_mode = BLENDING_INVERSED;
			break;
		case FTD330_BLD_DST_ATOP:
			src_blend_mode = BLENDING_INVERSED;
			dst_blend_mode = BLENDING_NORMAL;
			break;
		case FTD330_BLD_XOR:
			src_blend_mode = BLENDING_INVERSED;
			dst_blend_mode = BLENDING_INVERSED;
			break;
		default:
			break;
		}

		/* src/dst blending mode configuration */
		dc_set_clear(hw, reg->blend_config + offset,
			     (dst_blend_mode << 12) | (src_blend_mode << 5),
						 (0x07 << 12) | (0x07 << 5));
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(blend_mode_proto, "BLEND_MODE", struct drm_ftd330_blend, blend_mode_check,
			  NULL, blend_mode_config_hw);

static bool blend_alpha_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	const struct drm_ftd330_blend_alpha *alpha = data;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (enable) {
		/* src/dst global alpha value configuration */
		dc_write(hw, reg->src_global_color + offset, alpha->sga << 24);
		dc_write(hw, reg->dst_global_color + offset, alpha->dga << 24);

		/* src/dst alpha mode and global alpha mode configuration */
		dc_set_clear(hw, reg->blend_config + offset,
			     (alpha->dgam << 10) | (alpha->dam << 9) | (alpha->sgam << 3) |
						 (alpha->sam),
						 (0x03 << 10) | (0x01 << 9) | (0x03 << 3) | 0x1);
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(blend_alpha_proto, "BLEND_ALPHA", struct drm_ftd330_blend_alpha, NULL, NULL,
			  blend_alpha_config_hw);

bool ftd330_dc_register_plane_blender_states(struct ftd330_dc_property_state_group *states,
				   const struct ftd330_plane_info *info)
{
	if (info->blend_config) {
		__ERR_CHECK(ftd330_dc_property_register_state(states, &blend_mode_proto), on_error);
		__ERR_CHECK(ftd330_dc_property_register_state(states, &blend_alpha_proto), on_error);
	}

	return true;

on_error:
	return false;
}
