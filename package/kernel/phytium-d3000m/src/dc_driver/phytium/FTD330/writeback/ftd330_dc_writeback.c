// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#include <drm/drm_property.h>
#include "drm/ftd330_drm.h"
#include "drm/ftd330_drm_fourcc.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc_writeback.h"


/*
 * Full RGB to Limited YUV BT601 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV601[RGB_TO_YUV_TABLE_SIZE] = { 262, 515,  100, -151, -297, 448,
						  448, -375, -73, 64,	512,  512 };

/*
 * Full RGB to Limited YUV BT709 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV709[RGB_TO_YUV_TABLE_SIZE] = { 186, 627,  64,  -103, -345, 448,
						  448, -407, -41, 64,	512,  512 };

/*
 * Full RGB to Limited YUV BT2020 conversion parameters
 * RGB2YUV[0] - [8] : C0 - C8;
 * RGB2YUV[9] - [11]: D0 - D2;
 */
static s32 RGB_2YUV2020[RGB_TO_YUV_TABLE_SIZE] = { 231, 594,  52,  -125, -323, 448,
						   448, -412, -36, 64,	 512,  512 };

/* writeback r2y */
static bool wb_r2y_check(const struct dc_hw *hw, u8 hw_id, const void *data, u32 size,
			 const void *obj_state)
{
	const struct drm_ftd330_r2y_config *r2y = data;

	if (r2y->mode < FTD330_CSC_CM_USR || r2y->mode > FTD330_CSC_CM_F2F) {
		pr_err("%s: Unkown r2y mode %#x\n", __func__, r2y->mode);
		return false;
	}
	if (r2y->gamut < FTD330_CSC_CG_601 || r2y->gamut > FTD330_CSC_CG_SRGB) {
		pr_err("%s: Unkown r2y gamut %#x\n", __func__, r2y->gamut);
		return false;
	}
	return true;
}

static bool wb_r2y_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const struct drm_ftd330_r2y_config *r2y = data;
	u32 offset = hw_id << 2;

	if (enable) {
		/* For FTD330, conversion tables of the limited range
		 * are provided by default, at this stage.
		 */
		switch (r2y->gamut) {
		case FTD330_CSC_CG_601:
			load_wb_rgb_to_yuv(hw, offset, RGB_2YUV601);
			break;
		case FTD330_CSC_CG_709:
			load_wb_rgb_to_yuv(hw, offset, RGB_2YUV709);
			break;
		case FTD330_CSC_CG_2020:
			load_wb_rgb_to_yuv(hw, offset, RGB_2YUV2020);
			break;
		default:
			break;
		}
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(wb_r2y_proto, "R2Y", struct drm_ftd330_r2y_config, wb_r2y_check, NULL,
			  wb_r2y_config_hw);

/* writeback dither */
static bool wb_dither_config_hw(struct dc_hw *hw, u8 hw_id, bool enable, const void *data)
{
	const u32 offset = hw_id << 2;

	if (enable) {
		/* table */
		dc_write(hw, DC_DISPLAY_WRITE_BACK_DITHER_TABLE_LOW + offset,
			 DC_DISPLAY_WRITE_BACK_DITHERTABLE_LOW);

		dc_write(hw, DC_DISPLAY_WRITE_BACK_DITHER_TABLE_HIGH + offset,
			 DC_DISPLAY_WRITE_BACK_DITHERTABLE_HIGH);

		dc_write(hw, DC_DEST_CONFIG_Address + offset, BIT(18));
	}

	return true;
}

FTD330_DC_BLOB_PROPERTY_PROTO(wb_dither_proto, "WB_DITHER", struct drm_ftd330_wb_dither, NULL, NULL,
			  wb_dither_config_hw);

bool ftd330_dc_register_writeback_states(struct ftd330_dc_property_state_group *states,
				     const struct ftd330_wb_info *wb_info)
{
	if (wb_info->dither)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &wb_dither_proto), on_error);

	if (wb_info->csc)
		__ERR_CHECK(ftd330_dc_property_register_state(states, &wb_r2y_proto), on_error);

	return true;

on_error:
	return false;
}
