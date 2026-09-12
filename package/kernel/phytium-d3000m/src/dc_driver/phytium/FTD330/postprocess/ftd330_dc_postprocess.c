// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#include "ftd330_dc_postprocess.h"

#include <drm/drm_property.h>
#include "drm/ftd330_drm.h"
#include "drm/ftd330_drm_fourcc.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_hw.h"
#include "ftd330_drv.h"
#include "phytium_dp.h"
#include "phytium_dp_reg.h"

#define _DIV_ROUND_CLOSEST(x, divisor)                               \
	({                                                           \
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
		(((typeof(x))-1) > 0 || ((typeof(divisor))-1) > 0 || \
	 (((__x) > 0) == ((__d) > 0))) ?		\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
	})

/*
 * Full RGB to Limited YUV BT601 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV601[RGB_TO_YUV_TABLE_SIZE] = { 262, 515,  100, -151, -297, 448,
					       448,  -375, -73, 64,  512,  512 };

/*
 * Full RGB to Limited YUV BT709 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV709[RGB_TO_YUV_TABLE_SIZE] = { 186, 627,  64,  -103, -345, 448,
					       448,  -407,  -41,  64,  512,  512 };

/*
 * Full RGB to Limited YUV BT2020 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV2020[RGB_TO_YUV_TABLE_SIZE] = { 231, 594,  52,  -125, -323, 448,
					      448,  -412,  -36,  64,  512,  512 };

static bool vrr_mode_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const u32 vrr_refresh = *(u32 *)data;
	struct dc_hw_display_mode old_mode = hw->display[hw_id].mode, vrr_mode;
	struct drm_device *dev = hw->drm_dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	u32 new_vtotal = 0, new_vfp = 0, vsync_width = 0, vbp = 0;
	u32 offset = display_get_addr_offset(hw, hw_id);
	u32 dp_offset = priv->dp_reg_base[hw_id];

	/* calculate vrr display mode */
	vsync_width = old_mode.v_sync_end - old_mode.v_sync_start;
	vbp = old_mode.v_total - old_mode.v_sync_end;
	/* clock = v_total * h_total * refresh , not the real hardware situation */
	new_vtotal = _DIV_ROUND_CLOSEST(old_mode.clock * 1000, old_mode.h_total * vrr_refresh);
	new_vfp = new_vtotal - old_mode.v_active - vsync_width - vbp;

	vrr_mode.v_active = old_mode.v_active;
	vrr_mode.v_sync_start = vrr_mode.v_active + new_vfp;
	vrr_mode.v_sync_end = vrr_mode.v_sync_start + vsync_width;
	vrr_mode.v_total = new_vtotal;

	if (enable) {
		dc_set_clear(hw, DC_DISPLAY_V + offset,
			     vrr_mode.v_active | (vrr_mode.v_total << 16),
			     GENMASK(14, 0) | GENMASK(30, 16));
		dc_set_clear(hw, DC_DISPLAY_V_SYNC + offset,
			     vrr_mode.v_sync_start | (vrr_mode.v_sync_end << 15), GENMASK(29, 0));

		phytium_writel_reg(priv, vrr_mode.v_total,
			dp_offset, PHYTIUM_DP_MAIN_LINK_VTOTAL);
		phytium_writel_reg(priv, vrr_mode.v_active,
				dp_offset, PHYTIUM_DP_MAIN_LINK_VRES);
		phytium_writel_reg(priv,
				vrr_mode.v_sync_end - vrr_mode.v_sync_start,
				dp_offset, PHYTIUM_DP_MAIN_LINK_VSWIDTH);
		phytium_writel_reg(priv, vrr_mode.v_total - vrr_mode.v_sync_start,
				dp_offset, PHYTIUM_DP_MAIN_LINK_VSTART);
		DRM_DEBUG_KMS("VRR set vtotal %d to dp offset reg 0x%x\n", vrr_mode.v_total, dp_offset);
	}
	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(vrr_mode_proto, "VRR_REFRESH", u32, NULL, NULL, vrr_mode_config_hw);

static bool _3d_lut_check(const struct drm_ftd330_color *lut, u32 size, u32 lut_bit)
{
	u32 i;

	for (i = 0; i < size; i++) {
		if ((lut[i].r >> lut_bit) || (lut[i].g >> lut_bit) || (lut[i].b >> lut_bit)) {
			pr_err("%s: The entry of 3D LUT %u(%u, %u, %u) over valid bit(%u).\n",
			       __func__, i, lut[i].r, lut[i].g, lut[i].b, lut_bit);
			return false;
		}
	}
	return true;
}

static bool prior_3d_lut_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			       const void *obj_state)
{
	const struct ftd330_dc_info *info = hw->info;

	return _3d_lut_check(data, FTD330_MAX_PRIOR_3DLUT_SIZE, info->cgm_lut_bits);
}

static bool prior_3d_lut_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const struct drm_ftd330_color *lut  = data;
	u32 i;
	u32 offset = display_get_addr_offset(hw, hw_id);

	dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, enable << 14, BIT(14));
	if (enable) {
		/* set the index start from 0 */
		dc_write(hw, DC_DISPLAY_THREED_LUT_INDEX + offset, 0);

		/* coef data of RED/GREEN/BLUE channel*/
		for (i = 0; i < FTD330_MAX_PRIOR_3DLUT_SIZE; i++) {
			dc_write(hw, DC_DISPLAY_THREED_LUT_DATA + offset,
				 ((lut[i].r & 0x3FF) << 20) | ((lut[i].g & 0x3FF) << 10) |
					 (lut[i].b & 0x3FF));
		}
	}
	return true;
}

FTD330_DC_ARRAY_PROPERTY_PROTO(prior_3d_lut_proto, "PRIOR_3DLUT", struct drm_ftd330_color,
			   FTD330_MAX_PRIOR_3DLUT_SIZE, false, prior_3d_lut_check, NULL,
			   prior_3d_lut_config_hw);

static bool display_r2y_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const struct drm_ftd330_r2y_config *r2y = data;
	u32 offset = display_get_addr_offset(hw, hw_id);

	if (enable) {
		/* For FTD330, conversion tables of the limited range
		 * are provided by default, at this stage.
		 */
		switch (r2y->gamut) {
		case FTD330_CSC_CG_601:
			load_rgb_to_yuv(hw, offset, RGB_2YUV601);
			break;
		case FTD330_CSC_CG_709:
			load_rgb_to_yuv(hw, offset, RGB_2YUV709);
			break;
		case FTD330_CSC_CG_2020:
			load_rgb_to_yuv(hw, offset, RGB_2YUV2020);
			break;
		default:
			break;
		}
	}
	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(r2y_proto, "R2Y", struct drm_ftd330_r2y_config, NULL, NULL,
			  display_r2y_config_hw);

static bool bg_color_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			   const void *obj_state)
{
	const struct dc_hw_display *hw_display = ftd330_dc_hw_get_display(hw, hw_id);
	const struct ftd330_display_info *display_info = hw_display->info;

	if (!display_info->background) {
		pr_err("%s The display is not support set background color.\n", __func__);
		return false;
	}
	return true;
}

static bool bg_color_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset = display_get_addr_offset(hw, hw_id);

	const struct drm_ftd330_color *bg_color = data;

	if (enable)
		dc_write(hw, DC_FRAMEBUFFER_BG_COLOR + offset,
			 (bg_color->a & 0xFF) << 24 | (bg_color->r & 0xFF) << 16 |
				 (bg_color->g & 0xFF) << 8 | (bg_color->b & 0xFF));
	else
		dc_write(hw, DC_FRAMEBUFFER_BG_COLOR + offset, 0x0);
	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(bg_color_proto, "BG_COLOR", struct drm_ftd330_color, bg_color_check, NULL,
			  bg_color_config_hw);
static bool dither_enabled_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	u32 offset = display_get_addr_offset(hw, hw_id);


	if (enable) {
		dc_write(hw, DC_DISPLAY_DITHER_TABLE_LOW + offset, DC_DISPLAY_DITHERTABLE_LOW);
		dc_write(hw, DC_DISPLAY_DITHER_TABLE_HIGH + offset, DC_DISPLAY_DITHERTABLE_HIGH);
		dc_write(hw, DC_DISPLAY_DITHER_CONFIG + offset, BIT(31));
	} else
		dc_write(hw, DC_DISPLAY_DITHER_CONFIG + offset, 0);
	return true;
}
FTD330_DC_BOOL_PROPERTY_PROTO(dither_enabled_proto, "DITHER_ENABLED", NULL, NULL,
			  dither_enabled_config_hw);

bool ftd330_dc_register_postprocess_states(struct ftd330_dc_property_state_group *states,
				       const struct ftd330_display_info *display_info)
{

	if (display_info->background)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &bg_color_proto), on_error);

	if (display_info->color_formats &
		(DRM_COLOR_FORMAT_YCBCR444 | DRM_COLOR_FORMAT_YCBCR422 | DRM_COLOR_FORMAT_YCBCR420))
		__ERR_CHECK(ftd330_dc_property_register_state(states, &r2y_proto), on_error);

	if (display_info->cgm_lut)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &prior_3d_lut_proto), on_error);

	if (display_info->vrr)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &vrr_mode_proto), on_error);

	if (display_info->dither)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &dither_enabled_proto), on_error);

	return true;


on_error:
	return false;

}
