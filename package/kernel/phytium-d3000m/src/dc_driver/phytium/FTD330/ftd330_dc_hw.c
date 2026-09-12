// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 4, 18)
#include <linux/acpi.h>
#endif
#include <linux/bits.h>
#include <linux/io.h>
#include <linux/media-bus-format.h>
#include <linux/delay.h>
#include <linux/pci.h>

#include <drm/ftd330_drm.h>
#include <drm/ftd330_drm_fourcc.h>
#include "ftd330_dc_dec.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc.h"
#include "ftd330_type.h"
#include "preprocess/ftd330_dc_plane_blender.h"
#include "preprocess/ftd330_dc_preprocess.h"
#include "postprocess/ftd330_dc_postprocess.h"
#include "../phytium_se_communicate.h"
#include "writeback/ftd330_dc_writeback.h"

#ifdef CONFIG_PHYTIUM_DEBUG
#include "ftd330_debug.h"
#endif

extern void phytium_dc_scale_register_config(struct dc_hw *hw, uint32_t display_id, uint32_t hw_id);

/* The default horizontal scale coefficient data
 * with the filter tap of 3.
 */
static const u32 horKernel[] = {
	 0x00000000, 0x20000000, 0x00002000, 0x00000000, 0x00000000, 0x00000000, 0x23fd1c03,
	 0x00000000, 0x00000000, 0x00000000, 0x181f0000, 0x000027e1, 0x00000000, 0x00000000,
	 0x00000000, 0x2b981468, 0x00000000, 0x00000000, 0x00000000, 0x10f00000, 0x00002f10,
	 0x00000000, 0x00000000, 0x00000000, 0x32390dc7, 0x00000000, 0x00000000, 0x00000000,
	 0x0af50000, 0x0000350b, 0x00000000, 0x00000000, 0x00000000, 0x3781087f, 0x00000000,
	 0x00000000, 0x00000000, 0x06660000, 0x0000399a, 0x00000000, 0x00000000, 0x00000000,
	 0x3b5904a7, 0x00000000, 0x00000000, 0x00000000, 0x033c0000, 0x00003cc4, 0x00000000,
	 0x00000000, 0x00000000, 0x3de1021f, 0x00000000, 0x00000000, 0x00000000, 0x01470000,
	 0x00003eb9, 0x00000000, 0x00000000, 0x00000000, 0x3f5300ad, 0x00000000, 0x00000000,
	 0x00000000, 0x00480000, 0x00003fb8, 0x00000000, 0x00000000, 0x00000000, 0x3fef0011,
	 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000,
	 0x00000000, 0x20002000, 0x00000000, 0x00000000, 0x00000000, 0x1c030000, 0x000023fd,
	 0x00000000, 0x00000000, 0x00000000, 0x27e1181f, 0x00000000, 0x00000000, 0x00000000,
	 0x14680000, 0x00002b98, 0x00000000, 0x00000000, 0x00000000, 0x2f1010f0, 0x00000000,
	 0x00000000, 0x00000000, 0x0dc70000, 0x00003239, 0x00000000, 0x00000000, 0x00000000,
	 0x350b0af5, 0x00000000, 0x00000000, 0x00000000, 0x087f0000, 0x00003781, 0x00000000,
	 0x00000000, 0x00000000, 0x399a0666, 0x00000000, 0x00000000, 0x00000000, 0x04a70000,
	 0x00003b59, 0x00000000, 0x00000000, 0x00000000, 0x3cc4033c, 0x00000000, 0x00000000,
	 0x00000000, 0x021f0000,
};

#define H_COEF_SIZE (sizeof(horKernel) / sizeof(u32))

/* The default vertical scale coefficient data
 * with the filter tap of 3.
 */
static const u32 verKernel[] = {
	0x00000000, 0x20000000, 0x00002000, 0x00000000, 0x00000000, 0x00000000, 0x23fd1c03,
	0x00000000, 0x00000000, 0x00000000, 0x181f0000, 0x000027e1, 0x00000000, 0x00000000,
	0x00000000, 0x2b981468, 0x00000000, 0x00000000, 0x00000000, 0x10f00000, 0x00002f10,
	0x00000000, 0x00000000, 0x00000000, 0x32390dc7, 0x00000000, 0x00000000, 0x00000000,
	0x0af50000, 0x0000350b, 0x00000000, 0x00000000, 0x00000000, 0x3781087f, 0x00000000,
	0x00000000, 0x00000000, 0x06660000, 0x0000399a, 0x00000000, 0x00000000, 0x00000000,
	0x3b5904a7, 0x00000000, 0x00000000, 0x00000000, 0x033c0000, 0x00003cc4, 0x00000000,
	0x00000000, 0x00000000, 0x3de1021f, 0x00000000, 0x00000000, 0x00000000, 0x01470000,
	0x00003eb9, 0x00000000, 0x00000000, 0x00000000, 0x3f5300ad, 0x00000000, 0x00000000,
	0x00000000, 0x00480000, 0x00003fb8, 0x00000000, 0x00000000, 0x00000000, 0x3fef0011,
	0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004000, 0x00000000, 0xcdcd0000,
	0xfdfdfdfd, 0xabababab, 0xabababab, 0x00000000, 0x00000000, 0x5ff5f456, 0x000f5f58,
	0x02cc6c78, 0x02cc0c28, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee, 0xfeeefeee,
	0xfeeefeee, 0xfeeefeee,
};
#define V_COEF_SIZE (sizeof(verKernel) / sizeof(u32))

/*
 * RGB 709->2020 conversion parameters
 */
static u16 RGB2RGB[RGB_TO_RGB_TABLE_SIZE] = {
	10279, 5395, 709, 1132, 15065, 187, 269, 1442, 14674 };

/*
 * YUV601 to RGB conversion parameters
 * YUV2RGB[0]  - [8] : C0 - C8;
 * YUV2RGB[9]  - [11]: D0 - D2;
 * YUV2RGB[12] - [13]: Y clamp min & max calue;
 * YUV2RGB[14] - [15]: UV clamp min & max calue;
 */
static s32 YUV601_2RGB_LIMITED[YUV_TO_RGB_TABLE_SIZE] = {
	1196, 0, 1640, 1196, -404, -836, 1196, 2076, 0, -916224, 558336, -1202944, 64, 940, 64, 960
};

/*
 * YUV709 to RGB conversion parameters
 * YUV2RGB[0]  - [8] : C0 - C8;
 * YUV2RGB[9]  - [11]: D0 - D2;
 * YUV2RGB[12] - [13]: Y clamp min & max calue;
 * YUV2RGB[14] - [15]: UV clamp min & max calue;
 */
static s32 YUV709_2RGB_LIMITED[YUV_TO_RGB_TABLE_SIZE] = {
	1196, 0, 1844, 1196, -220, -548, 1196, 2172, 0, -1020672, 316672, -1188608, 64, 940, 64, 960
};

/*
 * YUV2020 to RGB conversion parameters
 * YUV2RGB[0]  - [8] : C0 - C8;
 * YUV2RGB[9]  - [11]: D0 - D2;
 * YUV2RGB[12] - [13]: Y clamp min & max calue;
 * YUV2RGB[14] - [15]: UV clamp min & max calue;
 */
static s32 YUV2020_2RGB_LIMITED[YUV_TO_RGB_TABLE_SIZE] = {
	1196, 0, 1724, 1196, -192, -668, 1196, 2200, 0, -959232, 363776, -1202944, 64, 940, 64, 960

/*
 * Degamma table for 709, Gamma = 2.4f color space data.
 */
};


static s32 YUV601_2RGB_FULL[YUV_TO_RGB_TABLE_SIZE] = { 1023, 0,	   1434, 1023,	  -352,	  -730,
						       1023, 1813, 0,	 -734208, 553984, -928256 };

static s32 YUV709_2RGB_FULL[YUV_TO_RGB_TABLE_SIZE] = { 1023, 0,	   1611, 1023,	  -192,	  -479,
						       1023, 1898, 0,	 -824832, 343552, -971776 };

/* YUV2020 to RGB with full range
 * YUV2RGB[0]  - [8] : C0 - C8;
 * YUV2RGB[9]  - [11]: D0 - D2;
 */
static s32 YUV2020_2RGB_FULL[YUV_TO_RGB_TABLE_SIZE] = {
	1023, 0, 1509, 1023, -168, -585, 1023, 1925, 0, -772608, 385536, -985600
};


/* one is for primary plane and the other is for all overlay planes */
static const struct dc_hw_plane_reg dc_plane_reg[] = {
	{
		.y_address = DC_FRAMEBUFFER_ADDRESS,
		.u_address = DC_FRAMEBUFFER_U_ADDRESS,
		.v_address = DC_FRAMEBUFFER_V_ADDRESS,
		.y_stride = DC_FRAMEBUFFER_STRIDE,
		.u_stride = DC_FRAMEBUFFER_U_STRIDE,
		.v_stride = DC_FRAMEBUFFER_V_STRIDE,
		.size = DC_FRAMEBUFFER_SIZE,
		.top_left = DC_FRAMEBUFFER_TOP_LEFT,
		.bottom_right = DC_FRAMEBUFFER_BOTTOM_RIGHT,
		.scale_factor_x = DC_FRAMEBUFFER_SCALE_FACTOR_X,
		.scale_factor_y = DC_FRAMEBUFFER_SCALE_FACTOR_Y,
		.h_filter_coef_index = DC_FRAMEBUFFER_H_FILTER_COEF_INDEX,
		.h_filter_coef_data = DC_FRAMEBUFFER_H_FILTER_COEF_DATA,
		.v_filter_coef_index = DC_FRAMEBUFFER_V_FILTER_COEF_INDEX,
		.v_filter_coef_data = DC_FRAMEBUFFER_V_FILTER_COEF_DATA,
		.init_offset = DC_FRAMEBUFFER_INIT_OFFSET,
		.color_key = DC_FRAMEBUFFER_COLOR_KEY,
		.color_key_high = DC_FRAMEBUFFER_COLOR_KEY_HIGH,
		.clear_value = DC_FRAMEBUFFER_CLEAR_VALUE,
		.color_table_index = DC_FRAMEBUFFER_COLOR_TABLE_INDEX,
		.color_table_data = DC_FRAMEBUFFER_COLOR_TABLE_DATA,
		.scale_config = DC_FRAMEBUFFER_SCALE_CONFIG,
		.water_mark = DC_FRAMEBUFFER_WATER_MARK,
		.degamma_index = DC_FRAMEBUFFER_DEGAMMA_INDEX,
		.degamma_data = DC_FRAMEBUFFER_DEGAMMA_DATA,
		.degamma_ex_data = DC_FRAMEBUFFER_DEGAMMA_EX_DATA,
		.src_global_color = DC_FRAMEBUFFER_SRC_GLOBAL_COLOR,
		.dst_global_color = DC_FRAMEBUFFER_DST_GLOBAL_COLOR,
		.blend_config = DC_FRAMEBUFFER_BLEND_CONFIG,
		.roi_origin = DC_FRAMEBUFFER_ROI_ORIGIN,
		.roi_size = DC_FRAMEBUFFER_ROI_SIZE,
		.YUVToRGBCoef0 = DC_FRAMEBUFFER_YUVTORGB_COEF0,
		.YUVToRGBCoef1 = DC_FRAMEBUFFER_YUVTORGB_COEF1,
		.YUVToRGBCoef2 = DC_FRAMEBUFFER_YUVTORGB_COEF2,
		.YUVToRGBCoef3 = DC_FRAMEBUFFER_YUVTORGB_COEF3,
		.YUVToRGBCoef4 = DC_FRAMEBUFFER_YUVTORGB_COEF4,
		.YUVToRGBCoefD0 = DC_FRAMEBUFFER_YUVTORGB_COEFD0,
		.YUVToRGBCoefD1 = DC_FRAMEBUFFER_YUVTORGB_COEFD1,
		.YUVToRGBCoefD2 = DC_FRAMEBUFFER_YUVTORGB_COEFD2,
		.YClampBound = DC_FRAMEBUFFER_Y_CLAMP_BOUND,
		.UVClampBound = DC_FRAMEBUFFER_UV_CLAMP_BOUND,
		.RGBToRGBCoef0 = DC_FRAMEBUFFER_RGBTORGB_COEF0,
		.RGBToRGBCoef1 = DC_FRAMEBUFFER_RGBTORGB_COEF1,
		.RGBToRGBCoef2 = DC_FRAMEBUFFER_RGBTORGB_COEF2,
		.RGBToRGBCoef3 = DC_FRAMEBUFFER_RGBTORGB_COEF3,
		.RGBToRGBCoef4 = DC_FRAMEBUFFER_RGBTORGB_COEF4,
		.LayerConfigEx = DC_FRAMEBUFFER_CONFIG_EX,
	},
	{
		.y_address = DC_OVERLAY_ADDRESS,
		.u_address = DC_OVERLAY_U_ADDRESS,
		.v_address = DC_OVERLAY_V_ADDRESS,
		.y_stride = DC_OVERLAY_STRIDE,
		.u_stride = DC_OVERLAY_U_STRIDE,
		.v_stride = DC_OVERLAY_V_STRIDE,
		.size = DC_OVERLAY_SIZE,
		.top_left = DC_OVERLAY_TOP_LEFT,
		.bottom_right = DC_OVERLAY_BOTTOM_RIGHT,
		.scale_factor_x = DC_OVERLAY_SCALE_FACTOR_X,
		.scale_factor_y = DC_OVERLAY_SCALE_FACTOR_Y,
		.h_filter_coef_index = DC_OVERLAY_H_FILTER_COEF_INDEX,
		.h_filter_coef_data = DC_OVERLAY_H_FILTER_COEF_DATA,
		.v_filter_coef_index = DC_OVERLAY_V_FILTER_COEF_INDEX,
		.v_filter_coef_data = DC_OVERLAY_V_FILTER_COEF_DATA,
		.init_offset = DC_OVERLAY_INIT_OFFSET,
		.color_key = DC_OVERLAY_COLOR_KEY,
		.color_key_high = DC_OVERLAY_COLOR_KEY_HIGH,
		.clear_value = DC_OVERLAY_CLEAR_VALUE,
		.color_table_index = DC_OVERLAY_COLOR_TABLE_INDEX,
		.color_table_data = DC_OVERLAY_COLOR_TABLE_DATA,
		.scale_config = DC_OVERLAY_SCALE_CONFIG,
		.water_mark = DC_OVERLAY_WATER_MARK,
		.degamma_index = DC_OVERLAY_DEGAMMA_INDEX,
		.degamma_data = DC_OVERLAY_DEGAMMA_DATA,
		.degamma_ex_data = DC_OVERLAY_DEGAMMA_EX_DATA,
		.src_global_color = DC_OVERLAY_SRC_GLOBAL_COLOR,
		.dst_global_color = DC_OVERLAY_DST_GLOBAL_COLOR,
		.blend_config = DC_OVERLAY_BLEND_CONFIG,
		.roi_origin = DC_OVERLAY_ROI_ORIGIN,
		.roi_size = DC_OVERLAY_ROI_SIZE,
		.YUVToRGBCoef0 = DC_OVERLAY_YUVTORGB_COEF0,
		.YUVToRGBCoef1 = DC_OVERLAY_YUVTORGB_COEF1,
		.YUVToRGBCoef2 = DC_OVERLAY_YUVTORGB_COEF2,
		.YUVToRGBCoef3 = DC_OVERLAY_YUVTORGB_COEF3,
		.YUVToRGBCoef4 = DC_OVERLAY_YUVTORGB_COEF4,
		.YUVToRGBCoefD0 = DC_OVERLAY_YUVTORGB_COEFD0,
		.YUVToRGBCoefD1 = DC_OVERLAY_YUVTORGB_COEFD1,
		.YUVToRGBCoefD2 = DC_OVERLAY_YUVTORGB_COEFD2,
		.YClampBound = DC_OVERLAY_Y_CLAMP_BOUND,
		.UVClampBound = DC_OVERLAY_UV_CLAMP_BOUND,
		.RGBToRGBCoef0 = DC_OVERLAY_RGBTORGB_COEF0,
		.RGBToRGBCoef1 = DC_OVERLAY_RGBTORGB_COEF1,
		.RGBToRGBCoef2 = DC_OVERLAY_RGBTORGB_COEF2,
		.RGBToRGBCoef3 = DC_OVERLAY_RGBTORGB_COEF3,
		.RGBToRGBCoef4 = DC_OVERLAY_RGBTORGB_COEF4,
		.LayerConfigEx = DC_OVERLAY_CONFIG,
	},
};

#ifdef CONFIG_PHYTIUM_MMU
static const struct dc_hw_mmu_reg dc_mmu_reg = {
	.mmu_config = SE_MMU_REG_CONFIG,
	.mmu_control = SE_MMU_REG_CONTROL,
	.table_array_size = SE_MMU_REG_TABLE_ARRAY_SIZE,
	.safe_non_secure = SE_MMU_REG_SAFE_NON_SECUR,
	.safe_secure = SE_MMU_REG_SAFE_SECURE,
	.safe_ex = SE_MMU_REG_SAFE_EXT_ADDRESS,
	.context_pd_entry = SE_MMU_REG_CONTEXT_PD,
};
#endif

static const struct dc_hw_funcs hw_func;
static const struct dc_hw_sub_funcs hw_sub_func[];

u32 hi_read(struct dc_hw *hw, u32 reg)
{
	u32 value = readl(hw->hi_base + reg);

	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	return value;
}

static inline void hi_write(struct dc_hw *hw, u32 reg, u32 value)
{

	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	writel(value, hw->hi_base + reg);
#ifdef CONFIG_PHYTIUM_DEBUG
	ftd330_debug_dump_capture(hw->dc_capture_fp, reg, value);
#endif
}

inline void dc_write(struct dc_hw *hw, u32 reg, u32 value)
{

	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	writel(value, hw->reg_base + reg);
#ifdef CONFIG_PHYTIUM_DEBUG
	ftd330_debug_dump_capture(hw->dc_capture_fp, reg, value);
#endif
}

static inline u32 dc_read(struct dc_hw *hw, u32 reg)
{
	u32 value = readl(hw->reg_base + reg);

	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	return value;
}

inline void dc_set_clear(struct dc_hw *hw, u32 reg, u32 set, u32 clear)
{
	u32 value = dc_read(hw, reg);

	value &= ~clear;
	value |= set;
	dc_write(hw, reg, value);
}

#if defined(CONFIG_PHYTIUM_MMU) || defined(CONFIG_PHYTIUM_WRITEBACK)
static void sec_write(struct dc_hw *hw, u32 reg, u32 value)
{
	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	writel(value, hw->sec_base + reg - DC_SEC_REG_BASE);
#ifdef CONFIG_PHYTIUM_DEBUG
	ftd330_debug_dump_capture(hw->dc_capture_fp, reg, value);
#endif
}
#endif

#ifdef CONFIG_PHYTIUM_MMU
static u32 sec_read(struct dc_hw *hw, u32 reg)
{
	u32 value = readl(hw->sec_base + reg - DC_SEC_REG_BASE);

	if (hw->phytium_log_enable)
		pr_info("%s: 0x%08x = 0x%08x\n", __func__, reg, value);

	return value;
}

static void sec_set_clear(struct dc_hw *hw, u32 reg, u32 set, u32 clear)
{
	u32 value = sec_read(hw, reg);

	value &= ~clear;
	value |= set;
	sec_write(hw, reg, value);
}
#endif


bool is_dc_powered(struct dc_hw *hw, int dc_id) {
	u32 reg = DC_HW_CHIP_CID;
	u32 cid = 0;
	
	if (dc_id == DC_1)
		reg += DC_REG_OFFSET;

	cid = hi_read(hw, reg);
	if (!cid) 
		return false;
	else
		return true;
}


static void load_default_filter(struct dc_hw *hw, const struct dc_hw_plane_reg *reg, u32 offset)
{
	u8 i;

	dc_write(hw, reg->scale_config + offset, 0x33);
	dc_write(hw, reg->init_offset + offset, 0x80008000);
	dc_write(hw, reg->h_filter_coef_index + offset, 0x00);
	for (i = 0; i < H_COEF_SIZE; i++)
		dc_write(hw, reg->h_filter_coef_data + offset, horKernel[i]);

	dc_write(hw, reg->v_filter_coef_index + offset, 0x00);
	for (i = 0; i < V_COEF_SIZE; i++)
		dc_write(hw, reg->v_filter_coef_data + offset, verKernel[i]);
}

static void load_rgb_to_rgb(struct dc_hw *hw, const struct dc_hw_plane_reg *reg, u32 offset,
			    u16 *table)
{
	dc_write(hw, reg->RGBToRGBCoef0 + offset, table[0] | (table[1] << 16));
	dc_write(hw, reg->RGBToRGBCoef1 + offset, table[2] | (table[3] << 16));
	dc_write(hw, reg->RGBToRGBCoef2 + offset, table[4] | (table[5] << 16));
	dc_write(hw, reg->RGBToRGBCoef3 + offset, table[6] | (table[7] << 16));
	dc_write(hw, reg->RGBToRGBCoef4 + offset, table[8]);
}

static void load_yuv_to_rgb(struct dc_hw *hw, const struct dc_hw_plane_reg *reg, u32 offset,
			    s32 *table, u8 mode, bool primary)
{
	dc_write(hw, reg->YUVToRGBCoef0 + offset, (0xFFFF & table[0]) | (table[1] << 16));
	dc_write(hw, reg->YUVToRGBCoef1 + offset, (0xFFFF & table[2]) | (table[3] << 16));
	dc_write(hw, reg->YUVToRGBCoef2 + offset, (0xFFFF & table[4]) | (table[5] << 16));
	dc_write(hw, reg->YUVToRGBCoef3 + offset, (0xFFFF & table[6]) | (table[7] << 16));
	dc_write(hw, reg->YUVToRGBCoef4 + offset, table[8]);
	dc_write(hw, reg->YUVToRGBCoefD0 + offset, table[9]);
	dc_write(hw, reg->YUVToRGBCoefD1 + offset, table[10]);
	dc_write(hw, reg->YUVToRGBCoefD2 + offset, table[11]);
	if (mode == CSC_MODE_L2L) {
		if (primary)
			dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, BIT(8), BIT(6));
		else
			dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, BIT(30), BIT(29));
		dc_write(hw, reg->YClampBound + offset, table[12] | (table[13] << 16));
		dc_write(hw, reg->UVClampBound + offset, table[14] | (table[15] << 16));
	}
}

void load_rgb_to_yuv(struct dc_hw *hw, u32 offset, s32 *table)
{
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEF0 + offset, table[0] | (table[1] << 16));
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEF1 + offset, table[2] | (table[3] << 16));
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEF2 + offset, table[4] | (table[5] << 16));
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEF3 + offset, table[6] | (table[7] << 16));
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEF4 + offset, table[8]);
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEFD0 + offset, table[9]);
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEFD1 + offset, table[10]);
	dc_write(hw, DC_DISPLAY_RGBTOYUV_COEFD2 + offset, table[11]);
}
void load_wb_rgb_to_yuv(struct dc_hw *hw, u32 offset, s32 *table)
{
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF0 + offset, (0xFFFF & table[0]) | (table[1] << 16));
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF1 + offset, (0xFFFF & table[2]) | (table[3] << 16));
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF2 + offset, (0xFFFF & table[4]) | (table[5] << 16));
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF3 + offset, (0xFFFF & table[6]) | (table[7] << 16));
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF4 + offset, (0xFFFF & table[8]));
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF_D0 + offset, table[9]);
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF_D1 + offset, table[10]);
	dc_write(hw, DC_RGB_TO_YUV_WB_COEF_D2 + offset, table[11]);
}

static bool is_rgb(enum dc_hw_color_format format)
{
	switch (format) {
	case FORMAT_X4R4G4B4:
	case FORMAT_A4R4G4B4:
	case FORMAT_X1R5G5B5:
	case FORMAT_A1R5G5B5:
	case FORMAT_R5G6B5:
	case FORMAT_X8R8G8B8:
	case FORMAT_A8R8G8B8:
	case FORMAT_A2R10G10B10:
		return true;
	default:
		return false;
	}
}

/* calculate the actual dc to be set.
*  sometime,driver only register one dc,which can be DC0,or DC1 or DC2.
*  so we need to know if we actural dc to be set.
*/
int phytium_display_virtual_to_physical(int pipe_mask, u32 id)
{
	int i = 0;
	int count = 0;
	bool found = false;

	for (i = DISPLAY_0;i < DISPLAY_NUM;i++) {
		if (pipe_mask & BIT(i)) {
			if (count == id) {
				found = true;
				break;
			} else {
				count++;
			}
		}
	}

	if (found) {
		return i;
	} else {
		pr_err("%s: no right dc found\n", __func__);
		return -1;
	}
}


int phytium_display_physical_to_virtual(int pipe_mask, u32 id)
{
	int i = 0;
	int count = 0;
	bool found = false;

	for (i = DISPLAY_0;i < DISPLAY_NUM;i++) {
		if (pipe_mask & BIT(i)) {
			if ( i == id) {
				found = true;
				break;
			} else {
				count++;
			}
		}
	}

	if (found) {
		return count;
	} else {
		pr_err("%s: no right dc found\n", __func__);
		return -1;
	}
}



int phytium_plane_to_display(struct dc_hw *hw, u32 plane_id)
{
	if (plane_id == PRIMARY_PLANE_0 || 
			plane_id == OVERLAY_PLANE_0 ||
			plane_id == OVERLAY_PLANE_1  ||
			plane_id == CURSOR_PLANE_0) {
		return DISPLAY_0;
	} else if (plane_id == PRIMARY_PLANE_1 || 
					plane_id == OVERLAY_PLANE_2 ||
					plane_id == OVERLAY_PLANE_3 ||
					plane_id == CURSOR_PLANE_1) {
		return DISPLAY_1;
	} else {
		return DISPLAY_2;
	}
}


/*this driver has to meet three kinds of project requirements,hw->total_pipes = 1, hw->total_pipes = 2 or hw->total_pipes = 3;
 * 1、hw->total_pipes = 1 means only one dc is used.
 * Under such circustance, we register only one display,which is DISPLAY_0.
 *	and the physical device that actually used is DC0 whose chip id is 0x331.
 * 2、hw->total_pipes = 2 means only two dc are used.
 * Under such circustance, we register two display,which are DISPLAY_0 and DISPLAY_1.
 * and the physical device that actually used is DC1 whose chip id is 0x335 and have 2 dcs built in.
 * 3、hw->total_pipes = 3 means three dc are all used.
 * Under such circustance, we register three display,which are DISPLAY_0 , DISPLAY_1 and DISPLAY_2.
 * and the physical devices that actually used are DC0 and DC1.
 * DISPLAY_0 is designed to have PRIMARY_PLANE_0,OVERLAY_PLANE_0,OVERLAY_PLANE_1,CURSOR_0
 * DISPLAY_1 is designed to have PRIMARY_PLANE_1,OVERLAY_PLANE_2,OVERLAY_PLANE_3,CURSOR_1
 * DISPLAY_2 is designed to have PRIMARY_PLANE_2,OVERLAY_PLANE_4,OVERLAY_PLANE_5,CURSOR_2
 * The easiest way to meet above requirments is to
 * treat DISPLAY_0 as DISPLAY_1 and treat DISPLAY_1 as DISPLAY_2 when hw->total_pipes = 2.
 * Because when hw->total_pipes = 1 or hw->total_pipes = 3,we are using DISPLAY_x in order:
 *			DC_0 for DISPLAY_0
 *			DC_1 for DISPLAY_1 and DISPLAY_2.
 * Planes are executed the same.when hw->total_pipes = 2,
 * we treat *_PLANE_0 as *_PLANE_1 and treat *_PLANE_1 as *_PLANE_2;
 */
 
u32  plane_get_addr_offset(struct dc_hw *hw, u32 id)
{
	u32 offset = 0;
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, phytium_plane_to_display(hw, id));

	if (actual_display_id == DISPLAY_0) {
		switch (id) {
		case PRIMARY_PLANE_0:
		case OVERLAY_PLANE_0:
			offset = 0x00;
			break;
		case OVERLAY_PLANE_1:
			offset = 0x04;
			break;
		default:
			break;
		}
	} else if (actual_display_id == DISPLAY_1) {
		switch (id) {
		case PRIMARY_PLANE_0:
		case PRIMARY_PLANE_1:
		case OVERLAY_PLANE_0:
		case OVERLAY_PLANE_2:
			offset = 0x00 + DC_REG_OFFSET;
			break;
		case OVERLAY_PLANE_1:
		case OVERLAY_PLANE_3:
			offset = 0x04 + DC_REG_OFFSET;
			break;
		default:
			break;
		}
	} else {
		switch (id) {
		case PRIMARY_PLANE_0:
		case PRIMARY_PLANE_1:
		case PRIMARY_PLANE_2:
			offset = 0x04 + DC_REG_OFFSET;
			break;
		case OVERLAY_PLANE_0:
		case OVERLAY_PLANE_2:
		case OVERLAY_PLANE_4:
			offset = 0x08 + DC_REG_OFFSET;
			break;
		case OVERLAY_PLANE_1:
		case OVERLAY_PLANE_3:
		case OVERLAY_PLANE_5:
			offset = 0x0C + DC_REG_OFFSET;
			break;
		default:
			break;
		}
	}

	return offset;
}

u32 display_get_addr_offset(struct dc_hw *hw, u32 id)
{
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, id);

	if (actual_display_id == DISPLAY_0)
		return 0x00;
	else if (actual_display_id == DISPLAY_1)
		return DC_REG_OFFSET;
	else
		return DC_REG_OFFSET + 0x04;
}

u32 get_dc_from_display(struct dc_hw *hw, u32 id)
{
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, id);

	if (actual_display_id == DISPLAY_0)
		return DC_0;
	else
		return DC_1;
}

u32 dc_get_offset_from_plane(struct dc_hw *hw, u32 id)
{
	int virtual_display_id = phytium_plane_to_display(hw, id);
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, virtual_display_id);

	if (actual_display_id == DISPLAY_0)
		return 0x00;
	else
		return DC_REG_OFFSET;
}


u32 dc_get_offset_from_cursor(struct dc_hw *hw, u32 display_id)
{
	u32 offset = 0;
	switch (phytium_display_virtual_to_physical(hw->pipe_mask, display_id)) {
			case DISPLAY_0:
				offset = 0;
				break;
			case DISPLAY_1:
				offset = DC_REG_OFFSET;
				break;
			case DISPLAY_2:
				offset = DC_REG_OFFSET + DC_CURSOR_OFFSET;
				break;
			default:
				offset = 0;
				break;
	}
	return offset;
}


u32 dc_get_offset_from_display(struct dc_hw *hw, u32 id)
{
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, id);

	if (actual_display_id == DISPLAY_0)
		return 0x00;
	else
		return DC_REG_OFFSET;
}


#ifdef CONFIG_PHYTIUM_DEC
u32 get_offset_from_stream_id(struct dc_hw *hw, u8 stream_id)
{
	u32 offset = 0;
	int virtual_display_id = stream_id/STREAM_PER_DC;
	int actual_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, virtual_display_id);

	if (actual_display_id == DISPLAY_0) {
		switch (stream_id) {
		case 0:
			offset = 0x00;
			break;
		case 1:
			offset = 0x04;
			break;
		case 2:
			offset = 0x08;
			break;
		case 3:
			offset = 0x0c;
			break;
		case 4:
			offset = 0x10;
			break;
		case 5:
			offset = 0x14;
			break;
		case 6:
			offset = 0x18;
			break;
		case 7:
			offset = 0x1c;
			break;
		case 8:
			offset = 0x20;
			break;
		default:
		break;
		}
	} else if (actual_display_id == DISPLAY_1) {
		switch (stream_id) {
		case 0:
			offset = 0x00 + DC_REG_OFFSET;
			break;
		case 1:
			offset = 0x04 + DC_REG_OFFSET;
			break;
		case 2:
			offset = 0x08 + DC_REG_OFFSET;
			break;
		case 3:
			offset = 0x0c + DC_REG_OFFSET;
			break;
		case 4:
			offset = 0x10 + DC_REG_OFFSET;
			break;
		case 5:
			offset = 0x14 + DC_REG_OFFSET;
			break;
		case 6:
			offset = 0x18 + DC_REG_OFFSET;
			break;
		case 7:
			offset = 0x1c + DC_REG_OFFSET;
			break;
		case 8:
			offset = 0x20 + DC_REG_OFFSET;
			break;
		case 9:
			offset = 0x00 + DC_REG_OFFSET;
			break;
		case 10:
			offset = 0x04 + DC_REG_OFFSET;
			break;
		case 11:
			offset = 0x08 + DC_REG_OFFSET;
			break;
		case 12:
			offset = 0x0c + DC_REG_OFFSET;
			break;
		case 13:
			offset = 0x10 + DC_REG_OFFSET;
			break;
		case 14:
			offset = 0x14 + DC_REG_OFFSET;
			break;
		case 15:
			offset = 0x18 + DC_REG_OFFSET;
			break;
		case 16:
			offset = 0x1c + DC_REG_OFFSET;
			break;
		case 17:
			offset = 0x20 + DC_REG_OFFSET;
			break;
		default:
			break;

		}
	} else {
		switch (stream_id) {
		case 0:
			offset = 0x40 + DC_REG_OFFSET;
			break;
		case 1:
			offset = 0x44 + DC_REG_OFFSET;
			break;
		case 2:
			offset = 0x48 + DC_REG_OFFSET;
			break;
		case 3:
			offset = 0x4c + DC_REG_OFFSET;
			break;
		case 4:
			offset = 0x50 + DC_REG_OFFSET;
			break;
		case 5:
			offset = 0x54 + DC_REG_OFFSET;
			break;
		case 6:
			offset = 0x58 + DC_REG_OFFSET;
			break;
		case 7:
			offset = 0x5c + DC_REG_OFFSET;
			break;
		case 8:
			offset = 0x60 + DC_REG_OFFSET;
			break;
		case 9:
			offset = 0x40 + DC_REG_OFFSET;
			break;
		case 10:
			offset = 0x44 + DC_REG_OFFSET;
			break;
		case 11:
			offset = 0x48 + DC_REG_OFFSET;
			break;
		case 12:
			offset = 0x4c + DC_REG_OFFSET;
			break;
		case 13:
			offset = 0x50 + DC_REG_OFFSET;
			break;
		case 14:
			offset = 0x54 + DC_REG_OFFSET;
			break;
		case 15:
			offset = 0x58 + DC_REG_OFFSET;
			break;
		case 16:
			offset = 0x5c + DC_REG_OFFSET;
			break;
		case 17:
			offset = 0x60 + DC_REG_OFFSET;
			break;
		case 18:
			offset = 0x40 + DC_REG_OFFSET;
			break;
		case 19:
			offset = 0x44 + DC_REG_OFFSET;
			break;
		case 20:
			offset = 0x48 + DC_REG_OFFSET;
			break;
		case 21:
			offset = 0x4c + DC_REG_OFFSET;
			break;
		case 22:
			offset = 0x50 + DC_REG_OFFSET;
			break;
		case 23:
			offset = 0x54 + DC_REG_OFFSET;
			break;
		case 24:
			offset = 0x58 + DC_REG_OFFSET;
			break;
		case 25:
			offset = 0x5c + DC_REG_OFFSET;
			break;
		case 26:
			offset = 0x60 + DC_REG_OFFSET;
			break;
		default:
			break;
		}
	}
	return offset;
}
#endif



static const struct dc_hw_plane_reg *get_plane_reg(u32 id)
{
	return (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2) ? &dc_plane_reg[0] : &dc_plane_reg[1];
}

static bool get_plane_primary(u32 id)
{
	return (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2) ? true : false;
}


void ftd330_dc_hw_get_plane_config(struct dc_hw *hw, u32 id, u32 *offset, const struct dc_hw_plane_reg **reg,
			       bool *primary)
{
	*offset = plane_get_addr_offset(hw, id);
	*reg = get_plane_reg(id);
	*primary = get_plane_primary(id);
}



int phytium_dc_registers_init(struct ftd330_drm_private *priv, u32 dc_id)
{
	struct device *dev = &priv->pdev->dev;
	struct ftd330_dc *dc = dev_get_drvdata(dev);
	struct dc_hw *hw = &dc->hw;
	u8 i, id, display_num, layer_num;
	u32 offset;
	u32 plane_offset;
	u32 display_offset = 0;
	const struct dc_hw_plane_reg *reg;
	const struct ftd330_plane_info *plane_info;
	const struct ftd330_display_info *display_info;
	int ret = 0;

	i = 0;
	layer_num = 0;
	if (dc_id == DC_0) {
		dc_write(&dc->hw, DC_DISPLAY_PANEL_START, 0);
		dc_hw_do_reset(&dc->hw, DC_0);
		if (hw->overlay_enable) {
		i = 0;
		layer_num = 3;
		} else {
			i = 0;
			layer_num = 1;
		}
	} else {
		dc_write(&dc->hw, DC_DISPLAY_PANEL_START + DC_REG_OFFSET, 0);
		dc_hw_do_reset(&dc->hw, DC_1);
		if (hw->pipe_mask & BIT(DISPLAY_0)) {
			if (hw->overlay_enable) {
			i = 3;
				layer_num = 3;
			} else {
				i = 1;
				layer_num = 1;
			}
		} else {
			i = 0;
		}

		if (hw->pipe_mask & BIT(DISPLAY_1)) {
			if (hw->overlay_enable) {
			layer_num += 3; 
			} else {
				layer_num += 1;
			}
		}

		if (hw->pipe_mask & BIT(DISPLAY_2)) {
		        if (hw->overlay_enable) {
			layer_num += 3; 
                        } else {
                                layer_num += 1;
                        }
		}
 	}

	for (; i < layer_num; i++) {
		id = hw->info->planes[i].id;
		plane_offset = plane_get_addr_offset(hw, id);
		plane_info = &hw->info->planes[i];
		hw->plane[i].info = plane_info;
		FTD330_LOG("%s: plane name is %s,id= %d,max_scale = %d,helper_no_scaling = %d\n",
			__func__, plane_info->name, plane_info->id,
			plane_info->max_scale, DRM_PLANE_HELPER_NO_SCALING);
		if (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2)
			reg = &dc_plane_reg[0];
		else
			reg = &dc_plane_reg[1];
		if (plane_info->min_scale != DRM_PLANE_HELPER_NO_SCALING ||
			plane_info->max_scale != DRM_PLANE_HELPER_NO_SCALING) {
			load_default_filter(hw, reg, plane_offset);
		}
		load_rgb_to_rgb(hw, reg, plane_offset, RGB2RGB);
	}


	i = 0;
	display_num = 0;
	if (dc_id == DC_0) {
		i = 0;
		display_num = 1;
	} else {
		if (hw->pipe_mask & BIT(DISPLAY_0)) {
			i = 1;
		} else {
			i = 0;
		}

		if (hw->pipe_mask & BIT(DISPLAY_1)) {
			display_num += 1; 
		}

		if (hw->pipe_mask & BIT(DISPLAY_2)) {
			display_num += 1;  
		}
 	}

	for (; i < display_num; i++) {
		display_info = &hw->info->displays[i];
		hw->display[i].info = display_info;
		display_offset = display_get_addr_offset(hw, hw->display[i].info->id);
		dc_write(hw, DC_DISPLAY_PANEL_CONFIG + display_offset, 0x111);

/* for DC1 has 2 cursors.But the register offset between 2 dc is not 4(different from
 * display registers).its offset is 0x1080.So we need to handle the register offset
 * for cursors specially.
 */
 		offset = dc_get_offset_from_cursor(hw, i);
		dc_write(hw, DC_CURSOR_BACKGROUND + offset, 0x00FFFFFF);
		dc_write(hw, DC_CURSOR_FOREGROUND + offset, 0x00AAAAAA);
	}
#ifdef CONFIG_PHYTIUM_MMU
	ret = dc_mmu_construct(priv->dma_dev, &priv->mmu);
	if (ret)
		dev_err(dev, "failed to construct DC MMU\n");

	ret = dc_hw_mmu_init(&dc->hw, priv->mmu, dc_id);
	if (ret) {
		dev_err(dev, "failed to init DC MMU\n");
		return ret;
	}
#endif
	if (dc_id == 0) {
		dc_write(&dc->hw, DC_FRAMEBUFFER_WATER_MARK, priv->info.water_mark[0]);
		dc_write(&dc->hw, DC_QOS_CONFIG, priv->info.qos[0]);
	} else {
		dc_write(&dc->hw, DC_FRAMEBUFFER_WATER_MARK + DC_REG_OFFSET,
					priv->info.water_mark[1]);
		dc_write(&dc->hw, DC_QOS_CONFIG + DC_REG_OFFSET, priv->info.qos[1]);
		dc_write(&dc->hw, DC_FRAMEBUFFER_WATER_MARK + DC_REG_OFFSET + 4,
					priv->info.water_mark[2]);
	}
#ifdef CONFIG_PHYTIUM_DEC
	phytium_dec_init(&dc->hw);
#endif
	return ret;
}
int dc_hw_init(struct dc_hw *hw)
{
	u8 i, display_num, layer_num;
	struct ftd330_plane_info *plane_info;
	const struct ftd330_display_info *display_info;
	int ret = 0;
#ifdef CONFIG_PHYTIUM_WRITEBACK
	u8 wb_num;
	const struct ftd330_wb_info *wb_info;
#endif
	int plane_index = 0;

	hw->info = ftd330_dc_get_chip_info(hw->total_pipes);
	hw->output_info = ftd330_dc_get_output_info(hw->total_pipes);
	hw->func = (struct dc_hw_funcs *)&hw_func;
	if (!hw->overlay_enable) {
		for (i = 0; i < hw->info->plane_num; i++) {
			if (hw->info->planes[i].id == OVERLAY_PLANE_0 ||
					hw->info->planes[i].id == OVERLAY_PLANE_1 ||
					hw->info->planes[i].id == OVERLAY_PLANE_2 ||
					hw->info->planes[i].id == OVERLAY_PLANE_3 ||
					hw->info->planes[i].id == OVERLAY_PLANE_4 ||
					hw->info->planes[i].id == OVERLAY_PLANE_5) {
				continue;
			} else {
				hw->info->planes[plane_index] = hw->info->planes[i];
				plane_index++;
			}
		}
		 hw->info->plane_num = plane_index;
		 hw->info->layer_num = hw->info->plane_num - hw->total_pipes;
	}

	/* For non-security writeback use hw_sub_func[0], security writeback use hw_sub_func[1] */
	if (hw->info->write_back[0].program_point)
		hw->sub_func = (struct dc_hw_sub_funcs *)&hw_sub_func[0];
	else
		hw->sub_func = (struct dc_hw_sub_funcs *)&hw_sub_func[1];

	layer_num = hw->info->layer_num;

	for (i = 0; i < layer_num; i++) {
		plane_info = &hw->info->planes[i];

		/* Initialize property states */
		if (!ftd330_dc_register_plane_blender_states(&hw->plane[i].states, plane_info)) {
			pr_err("%s: Failed to register plane blender states.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}
		if (!ftd330_dc_register_preprocess_states(&hw->plane[i].states, plane_info)) {
			pr_err("%s: Failed to register preprocess.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}

		if (!ftd330_dc_initialize_property_states(&hw->plane[i].states)) {
			pr_err("%s: Failed to initialize plane property states.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}
		pr_err("%s: Alloc states mem %lu for plane %u\n",
			__func__, hw->plane[i].states.mem.total_size, i);
	}

	display_num = hw->info->display_num;
	for (i = 0; i < display_num; i++) {
		display_info = &hw->info->displays[i];
		hw->display[i].info = display_info;

		/* Initialize property states */
		if (!ftd330_dc_register_postprocess_states(&hw->display[i].states, display_info)) {
			pr_err("%s: Failed to register postprocess.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}

		if (!ftd330_dc_initialize_property_states(&hw->display[i].states)) {
			pr_err("%s: Failed to initialize display property states.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}
		pr_debug("%s: Alloc states mem %lu for display %u\n",
			__func__, hw->display[i].states.mem.total_size, i);
	}

	for (i = 0; i < display_num; i++)
		hw->display[i].vblank_enable = false;

#ifdef CONFIG_PHYTIUM_WRITEBACK
	wb_num = hw->info->wb_num;
	for (i = 0; i < wb_num; i++) {
		wb_info = &hw->info->write_back[i];
		hw->wb[i].info = wb_info;

		/* Initialize property states */
		if (!ftd330_dc_register_writeback_states(&hw->wb[i].states, wb_info)) {
			pr_err("%s: Failed to register writeback states.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}

		if (!ftd330_dc_initialize_property_states(&hw->wb[i].states)) {
			pr_err("%s: Failed to initialize writeback property states.\n", __func__);
			ret = -ENOMEM;
			goto err_cleanup;
		}
		pr_debug("%s: Alloc states mem %lu for writeback %u\n", __func__,
			 hw->wb[i].states.mem.total_size, i);
	}
#endif

	return 0;
err_cleanup:
	return ret;
}

void dc_hw_deinit(struct dc_hw *hw)
{
	int i;

	for (i = 0; i < hw->info->layer_num; i++)
		ftd330_dc_deinitialize_property_states(&hw->plane[i].states);

	for (i = 0; i < hw->info->display_num; i++)
		ftd330_dc_deinitialize_property_states(&hw->display[i].states);

#ifdef CONFIG_PHYTIUM_WRITEBACK
	for (i = 0; i < hw->info->wb_num; i++)
		ftd330_dc_deinitialize_property_states(&hw->wb[i].states);
#endif
}

const struct dc_hw_plane *ftd330_dc_hw_get_plane(const struct dc_hw *hw, u32 hw_id)
{
	u32 i;
	const struct ftd330_dc_info *dc_info = ftd330_dc_get_chip_info(hw->total_pipes);

	for (i = 0; i < dc_info->layer_num; i++)
		if (hw->plane[i].info->id == hw_id)
			return &hw->plane[i];
	return NULL;
}

const struct dc_hw_display *ftd330_dc_hw_get_display(const struct dc_hw *hw, u32 hw_id)
{
	u32 i;
	const struct ftd330_dc_info *dc_info = ftd330_dc_get_chip_info(hw->total_pipes);

	for (i = 0; i < dc_info->display_num; i++)
		if (hw->display[i].info->id == hw_id)
			return &hw->display[i];
	return NULL;
}

void dc_hw_update_plane(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb, struct dc_hw_position *pos,
			struct dc_hw_blend *blend, struct dc_hw_roi *roi)
{
	struct dc_hw_plane *plane = &hw->plane[id];

	if (plane) {
		if (fb) {
			if (!fb->enable)
				plane->fb.enable = false;
			else
				memcpy(&plane->fb, fb, sizeof(*fb) - sizeof(fb->dirty));
			plane->fb.dirty = true;
		}
		if (pos) {
			memcpy(&plane->pos, pos, sizeof(*pos) - sizeof(pos->dirty));
			plane->pos.dirty = true;
		}
		if (blend) {
			if (blend->enable) {
				memcpy(&plane->blend, blend, sizeof(*blend) - sizeof(blend->dirty));
				plane->blend.dirty = true;
			}
		}

		if (roi)
			memcpy(&plane->roi, roi, sizeof(*roi));

	}
}

void dc_hw_update_qos(struct dc_hw *hw, struct dc_hw_qos *qos)
{
	memcpy(&hw->qos, qos, sizeof(*qos) - sizeof(qos->dirty));
	hw->qos.dirty = true;
}

void dc_hw_update_cursor(struct dc_hw *hw, u8 id, struct dc_hw_cursor *cursor)
{
	memcpy(&hw->cursor[id], cursor, sizeof(*cursor) - sizeof(cursor->dirty));
	hw->cursor[id].dirty = true;
}

void dc_hw_update_ctm(struct dc_hw *hw, u8 id, u64 *matrix)
{
	hw->display[id].ctm.matrix = matrix;
}
void dc_hw_update_gamma(struct dc_hw *hw, u8 id, u16 index, u16 r, u16 g, u16 b)
{
	hw->display[id].gamma.gamma[index][0] = r;
	hw->display[id].gamma.gamma[index][1] = g;
	hw->display[id].gamma.gamma[index][2] = b;
	hw->display[id].gamma.dirty = true;
}
void dc_hw_update_plane_y2r(struct dc_hw *hw, u8 id, struct dc_hw_y2r *y2r_conf)
{
	struct dc_hw_plane *plane = &hw->plane[id];

	if (plane && y2r_conf) {
		memcpy(&plane->y2r, y2r_conf, sizeof(*y2r_conf) - sizeof(y2r_conf->dirty));
		plane->y2r.dirty = true;
		plane->y2r.enable = true;
	}
}
void dc_hw_enable_ctm(struct dc_hw *hw, u8 id, bool enable)
{
	hw->display[id].ctm.enable = enable;
	hw->display[id].ctm.dirty = true;
}

void dc_hw_enable_gamma(struct dc_hw *hw, u8 id, bool enable)
{
	hw->display[id].gamma.enable = enable;
	hw->display[id].gamma.dirty = true;
}

void dc_hw_setup_display_mode(struct dc_hw *hw, u8 id, struct dc_hw_display_mode *mode)
{
	struct dc_hw_display *display = &hw->display[id];
	u8 hw_id = 0;

	if (display && mode) {
		if (!mode->enable)
			display->mode.enable = false;
		else
			memcpy(&display->mode, mode, sizeof(*mode));
	}

	hw_id = hw->info->displays[id].id;
	hw->func->set_mode(hw, hw_id, &display->mode);
}

u32 dc_hw_get_vblank_count(struct dc_hw *hw, u8 id)
{
	return hw->display[id].vblank_count;
}

void dc_hw_enable_interrupt(struct dc_hw *hw, bool enable, u32 display_index)
{
	u32 dc_id = get_dc_from_display(hw, display_index);
	u32 dc_offset = (dc_id == 0) ? 0 : DC_REG_OFFSET;
	u32 dc1_interrupt_data = 0xFFFFFFFC;
	int physical_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, display_index);

	hw->display[physical_display_id].vblank_enable = enable;

	if (physical_display_id == DISPLAY_0) {
		if (enable)
			hi_write(hw, AQ_INTR_ENBL, 0xFFFFFFFF);
		else
			hi_write(hw, AQ_INTR_ENBL, 0);
	} else {
		if (hw->display[DISPLAY_1].vblank_enable)
			dc1_interrupt_data |= BIT(0);

		if (hw->display[DISPLAY_2].vblank_enable)
			dc1_interrupt_data |= BIT(1);

		if (!((hw->display[DISPLAY_1].vblank_enable) |
					(hw->display[DISPLAY_2].vblank_enable)))
			dc1_interrupt_data = 0;
		hi_write(hw, AQ_INTR_ENBL + dc_offset, dc1_interrupt_data);
	}
}

int dc_hw_get_interrupt(struct dc_hw *hw, struct dc_hw_interrupt_status *status, u32 dc_id)
{

	u8 i = 0, wb_bit_mask = 0;
	u32 intr_status = hi_read(hw, AQ_INTR_ACKNOWLEDGE + dc_id*DC_REG_OFFSET);
	u32 display_id = dc_id;
#ifdef CONFIG_PHYTIUM_MMU
	u32 mmu_intr_status = sec_read(hw, SE_MMU_INTR_ACKNOWLEDGE + dc_id*DC_REG_OFFSET);
#endif

	if (!intr_status)
		return -1;
	if (intr_status & BIT(0)) {
		if (dc_id == DC_0) {
			display_id = DISPLAY_0;
		} else {
			if (hw->pipe_mask & BIT(DISPLAY_1)) {
				display_id = phytium_display_physical_to_virtual(hw->pipe_mask, DISPLAY_1);
			} else {
				pr_err("dc1_hw_get_interrupt wrong frame_done interrput\n");
			}
		}
	        status->display_frm_done |= BIT(display_id);
	        hw->display[display_id].vblank_count++;

	}

	if (intr_status & BIT(1)) {
		if (hw->pipe_mask & BIT(DISPLAY_2)) {
			display_id = phytium_display_physical_to_virtual(hw->pipe_mask, DISPLAY_2);
		} else {
			pr_err("dc_hw2_get_interrupt wrong frame_done interrupt\n");
		}
	        status->display_frm_done |= BIT(display_id);
	        hw->display[display_id].vblank_count++;

	}

	if (intr_status & BIT(11)) {
		if (dc_id == DC_0) {/*DC0 panel 0 data underflow intr */
			display_id = DISPLAY_0;
		} else {/*DC1 panel 0 data underflow intr */
			if (hw->pipe_mask & BIT(DISPLAY_1)) {
				display_id = phytium_display_physical_to_virtual(hw->pipe_mask, DISPLAY_1);
			} else {
				pr_err("dc1_hw_get_interrupt wrong underflow intrrupt");
			}
		}
		status->display_underflow |= BIT(display_id);
	        hw->display[display_id].underflow_count++;
	}
	if (intr_status & BIT(12)) {
		if (hw->pipe_mask & BIT(DISPLAY_2)) {
			display_id = phytium_display_physical_to_virtual(hw->pipe_mask, DISPLAY_2);
		} else {
			pr_err("dc2_hw_get_interrupt wrong underflow interrupt\n");
		}
		status->display_underflow |= BIT(display_id);
	        hw->display[display_id].underflow_count++;
	}

	if (intr_status & BIT(13))
		status->wb_datalost |= BIT(WB_0); /* panel 0 writeback buffer overflow */
	if (intr_status & BIT(14))
		status->wb_datalost |= BIT(WB_1); /* panel 1 writeback buffer overflow */

	for (i = 0; i < hw->info->wb_num; i++) {
		if (hw->info->write_back[i].program_point)
			wb_bit_mask = 27 + i;
		else
			wb_bit_mask = i;

		if (intr_status & BIT(wb_bit_mask)) {
			status->wb_frm_done |= BIT(i);
			hw->wb[i].wb_frm_done = true; /* panel 0 writeback done intr */
		}
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (mmu_intr_status & BIT(0))
		pr_err("MMU0 slave not presend.\n");
	if (mmu_intr_status & BIT(1))
		pr_err("MMU0 client out of bound.\n");
	if (mmu_intr_status & BIT(2))
		pr_err("MMU0 size of bound.\n");
	if (mmu_intr_status & BIT(3))
		pr_err("MMU0 read security exception.\n");
	if (mmu_intr_status & BIT(4))
		pr_err("MMU0 write security exception.\n");
	if (mmu_intr_status & BIT(5))
		pr_err("MMU0 page not present.\n");
	if (mmu_intr_status & BIT(6))
		pr_err("MMU0 page write to unwritable page.\n");
	if (mmu_intr_status & BIT(7))
		pr_err("MMU0 read security mem violation.\n");
	if (mmu_intr_status & BIT(8))
		pr_err("MMU1 slave not presend.\n");
	if (mmu_intr_status & BIT(9))
		pr_err("MMU1 client out of bound.\n");
	if (mmu_intr_status & BIT(10))
		pr_err("MMU1 size of bound.\n");
	if (mmu_intr_status & BIT(11))
		pr_err("MMU1 read security exception.\n");
	if (mmu_intr_status & BIT(12))
		pr_err("MMU1 write security exception.\n");
	if (mmu_intr_status & BIT(13))
		pr_err("MMU1 page not present.\n");
	if (mmu_intr_status & BIT(14))
		pr_err("MMU1 page write to unwritable page.\n");
	if (mmu_intr_status & BIT(15))
		pr_err("MMU1 read security mem violation.\n");
#endif

	return 0;
}

bool dc_hw_check_underflow(struct dc_hw *hw, u32 display_id)
{
	return dc_read(hw, DC_FRAMEBUFFER_CONFIG + display_get_addr_offset(hw, display_id)) & BIT(5);
}

void dc_hw_enable_shadow_register(struct dc_hw *hw, bool enable, u8 display_id)
{
	u32 i, offset;
	struct dc_hw_plane *plane;
	u8 id, layer_num = hw->info->layer_num;


	for (i = 0; i < layer_num; i++) {
		plane = &hw->plane[i];
		id = hw->info->planes[i].id;

		if (plane->fb.display_id != display_id)
			continue;

                if (!((hw->pipe_mask & BIT(DISPLAY_1)) && (hw->pipe_mask & BIT(DISPLAY_2)) && !(hw->pipe_mask & BIT(DISPLAY_0)))) {
                        if (display_id == DISPLAY_0 &&
                                (id != PRIMARY_PLANE_0 && id != OVERLAY_PLANE_0 &&
                                        id != OVERLAY_PLANE_1 && id != CURSOR_PLANE_0))
                                continue;
                }

		offset = plane_get_addr_offset(hw, id);
		if (enable) {
			if (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2)
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, BIT(12), 0);
			else
				dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, BIT(31), 0);
		} else {
			if (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2)
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset, 0, BIT(12));
			else
				dc_set_clear(hw, DC_OVERLAY_CONFIG + offset, 0, BIT(31));
		}
	}

	offset = display_get_addr_offset(hw, display_id);
	if (enable)
		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG_EX + offset, 0, BIT(0));
	else
		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG_EX + offset, BIT(0), 0);
}

void dc_hw_do_reset(struct dc_hw *hw, u32 dc_id)
{
	u32 ret = 0, ret1 = 0;
	u32 dc_offset = dc_id*DC_REG_OFFSET;

	do {
		mdelay(1);
		ret = hi_read(hw, AQ_AXI_CONFIG + dc_offset);
	} while ((ret >> 28) & 0x6);

	ret1 = hi_read(hw, AQ_HI_CLOCK_CONTROL + dc_offset);
	hi_write(hw, AQ_HI_CLOCK_CONTROL + dc_offset, ret1 | (1 << 12));
	mdelay(5);
#ifdef CONFIG_PHYTIUM_DEC
	phytium_dec_init(hw);
#endif
}

void dc_hw_start_trigger(struct dc_hw *hw, u8 display_id)
{
	u32 dc_id = get_dc_from_display(hw, display_id);
	u32 dc_offset = (dc_id == 0) ? 0 : DC_REG_OFFSET;
	u32 physical_display_id = phytium_display_virtual_to_physical(hw->pipe_mask, display_id);

	if (hw->display[display_id].sync_enable)
		dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, BIT(2) | BIT(3), 0);
	else if (physical_display_id == 0 || physical_display_id == 1)
		dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, BIT(0), BIT(3));
	else
		dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, BIT(1), BIT(3));
}


void dc_hw_stop_trigger(struct dc_hw *hw, u8 physical_display_id)
{
	u32 dc_id = get_dc_from_display(hw, physical_display_id);
	u32 dc_offset = (dc_id == 0) ? 0 : DC_REG_OFFSET;
	u32 value;

	if (physical_display_id == 0 || physical_display_id == 1) {
		value = dc_read(hw, DC_DISPLAY_PANEL_START + dc_offset);
		value &= 0xfffffffe;
		dc_write(hw, DC_DISPLAY_PANEL_START + dc_offset, value);
	} else {
		value = dc_read(hw, DC_DISPLAY_PANEL_START + dc_offset);
		value &= 0xfffffffd;
		dc_write(hw, DC_DISPLAY_PANEL_START + dc_offset, value);
		dc_write(hw, DC_DISPLAY_PANEL_START + dc_offset,value);
	}
}


void dc_hw_set_plane_roi(struct dc_hw *hw, u8 hw_id, struct dc_hw_roi *roi)
{
	u32 offset;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;

	ftd330_dc_hw_get_plane_config(hw, hw_id, &offset, &reg, &primary);

	if (roi->enable) {

		dc_write(hw, reg->roi_origin + offset,
			 roi->x | (roi->y << 16));
		dc_write(hw, reg->roi_size + offset,
			 roi->width | (roi->height << 16));

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

	roi->dirty = false;
}

static void plane_commit(struct dc_hw *hw, u8 display_id)
{
	struct dc_hw_plane *plane;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	u8 id, layer_num = hw->info->layer_num;
	u32 i, j, offset;
	u32 real_display_id = DISPLAY_0;
	u32 real_zpos = 0;

	for (i = 0; i < layer_num; i++) {
		plane = &hw->plane[i];
		id = hw->info->planes[i].id;

		if (plane->fb.display_id != display_id)
			continue;

		if (!((hw->pipe_mask & BIT(DISPLAY_1)) && (hw->pipe_mask & BIT(DISPLAY_2)) && !(hw->pipe_mask & BIT(DISPLAY_0)))) {
			if (display_id == DISPLAY_0 &&
				(id != PRIMARY_PLANE_0 && id != OVERLAY_PLANE_0 &&
					id != OVERLAY_PLANE_1 && id != CURSOR_PLANE_0))
				continue;
		}

		id = hw->info->planes[i].id;
		offset = plane_get_addr_offset(hw, id);
		real_zpos = (hw->info->planes[i].zpos == 255) ? 0 : hw->info->planes[i].zpos;

		/*for display0,no changed need to be made.for display1,we have to realize that it is
		 *display0 for DC1.so we have to change the display id and zpos.
		 */

		if (phytium_display_virtual_to_physical(hw->pipe_mask, display_id) == DISPLAY_2) {
			real_display_id = DISPLAY_1;
		} else {
			real_display_id = DISPLAY_0;
		}

		if (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2) {
			reg = &dc_plane_reg[0];
			primary = true;
		} else {
			reg = &dc_plane_reg[1];
			primary = false;
		}

		if (plane->fb.dirty) {
			if (plane->fb.enable) {
				dc_write(hw, reg->y_address + offset, plane->fb.y_address);
				dc_write(hw, reg->u_address + offset, plane->fb.u_address);
				dc_write(hw, reg->v_address + offset, plane->fb.v_address);
				dc_write(hw, reg->y_stride + offset, plane->fb.y_stride);
				dc_write(hw, reg->u_stride + offset, plane->fb.u_stride);
				dc_write(hw, reg->v_stride + offset, plane->fb.v_stride);

				dc_write(hw, reg->size + offset, plane->fb.width |(plane->fb.height << 15));
			}

			if (primary) {
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG + offset,
					     (plane->fb.format << 26) |
						     (plane->fb.uv_swizzle << 25) |
						     (plane->fb.swizzle << 23) |
						     (plane->fb.tile_mode << 17) |
						     (plane->fb.yuv_gamut << 14) |
						     (plane->fb.rotation << 11),
					     (0x1F << 26) | BIT(25) | (0x03 << 23) | (0x1F << 17) |
						     (0x07 << 14) | (0x07 << 11));
				dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + offset,
					     (plane->fb.dec_enable << 1) |
						     (plane->fb.enable << 13) |
						     (real_zpos << 16) |
						     (real_display_id << 19),
					     BIT(1) | BIT(13) | (0x07 << 16) | BIT(19));
			} else {
				dc_set_clear(hw, DC_OVERLAY_CONFIG + offset,
					(plane->fb.dec_enable << 27) | (plane->fb.enable << 24) |
						     (plane->fb.format << 16) |
						     (plane->fb.uv_swizzle << 15) |
						     (plane->fb.swizzle << 13) |
						     (plane->fb.tile_mode << 8) |
						     (plane->fb.yuv_gamut << 5) |
						     (plane->fb.rotation << 2),
					BIT(27) | BIT(24) | (0x1F << 16) | BIT(15) | (0x03 << 13) |
						(0x1F << 8) | (0x07 << 5) | (0x07 << 2));
				dc_set_clear(hw, DC_OVERLAY_CONFIG_EX + offset,
					     real_zpos | (real_display_id << 3),
					     0x07 | BIT(3));
			}
			plane->fb.dirty = false;
		}

		if (plane->roi.dirty)
			dc_hw_set_plane_roi(hw, id, &plane->roi);
		if (plane->pos.dirty) {
			dc_write(hw, reg->top_left + offset,
				 plane->pos.start_x | (plane->pos.start_y << 15));
			dc_write(hw, reg->bottom_right + offset,
				 plane->pos.end_x | (plane->pos.end_y << 15));
			plane->pos.dirty = false;
		}

		if (plane->blend.dirty) {
			if (plane->blend.blend_mode == DRM_MODE_BLEND_PIXEL_NONE &&
					 plane->blend.alpha == 0xFF) {
				dc_write(hw, reg->blend_config + offset, BIT(1));
			} else {
				dc_write(hw, reg->src_global_color + offset,
					 plane->blend.alpha << 24);
				dc_write(hw, reg->dst_global_color + offset,
					 plane->blend.alpha << 24);
				switch (plane->blend.blend_mode) {
				case BLEND_PREMULTI:
					dc_write(hw, reg->blend_config + offset, 0x3450);
					break;
				case BLEND_COVERAGE:
					dc_write(hw, reg->blend_config + offset, 0x3950);
					break;
				case BLEND_PIXEL_NONE:
					dc_write(hw, reg->blend_config + offset, 0x3548);
					break;
				default:
					break;
				}
			}
			plane->blend.dirty = false;
		}

		for (j = 0; j < plane->states.num; j++) {
			struct ftd330_dc_property_state *state = &plane->states.items[j];

			if (!state->dirty)
				continue;
			if (!state->proto->config_hw) {
				pr_err("%s: %s not provide config_hw func\n", __func__,
					   state->proto->name);
				continue;
				}
			ftd330_dc_property_config_hw(hw, id, state);
		}

		if (dc_fake_mode_enable)
			phytium_dc_scale_register_config(hw, display_id, id);

	}
}

static void plane_ex_commit(struct dc_hw *hw, u8 display_id)
{
	struct dc_hw_plane *plane;
	const struct dc_hw_plane_reg *reg;
	bool primary = false;
	u8 id, layer_num = hw->info->layer_num;
	u32 i, offset;

	for (i = 0; i < layer_num; i++) {
		plane = &hw->plane[i];
		

		id = hw->info->planes[i].id;

		if (plane->fb.display_id != display_id)
			continue;

		if (!((hw->pipe_mask & BIT(DISPLAY_1)) && (hw->pipe_mask & BIT(DISPLAY_2)) && !(hw->pipe_mask & BIT(DISPLAY_0)))) {
			if (display_id == DISPLAY_0 &&
				(id != PRIMARY_PLANE_0 && id != OVERLAY_PLANE_0 &&
					id != OVERLAY_PLANE_1 && id != CURSOR_PLANE_0))
				continue;
		}

		offset = plane_get_addr_offset(hw, id);
		if (id == PRIMARY_PLANE_0 || id == PRIMARY_PLANE_1 || id == PRIMARY_PLANE_2) {
			reg = &dc_plane_reg[0];
			primary = true;
		} else {
			reg = &dc_plane_reg[1];
			primary = false;
		}

		if (plane->fb.dirty) {
			if (!is_rgb(plane->fb.format)) {
				switch (plane->y2r.gamut) {
				/* For FTD330, conversion tables of the limited range
				 * are provided by default, at this stage.
				 */
				case CSC_GAMUT_601:
					if (plane->y2r.mode == CSC_MODE_L2L)
						load_yuv_to_rgb(hw, reg, offset,
								YUV601_2RGB_LIMITED,
								plane->y2r.mode, primary);
					else if (plane->y2r.mode == CSC_MODE_F2F)
						load_yuv_to_rgb(hw, reg, offset, YUV601_2RGB_FULL,
								plane->y2r.mode, primary);
					break;
				case CSC_GAMUT_709:
					if (plane->y2r.mode == CSC_MODE_L2L)
						load_yuv_to_rgb(hw, reg, offset,
								YUV709_2RGB_LIMITED,
								plane->y2r.mode, primary);
					else if (plane->y2r.mode == CSC_MODE_F2F)
						load_yuv_to_rgb(hw, reg, offset, YUV709_2RGB_FULL,
								plane->y2r.mode, primary);
					break;
				case CSC_GAMUT_2020:
					if (plane->y2r.mode == CSC_MODE_L2L)
						load_yuv_to_rgb(hw, reg, offset,
								YUV2020_2RGB_LIMITED,
								plane->y2r.mode, primary);
					else if (plane->y2r.mode == CSC_MODE_F2F)
						load_yuv_to_rgb(hw, reg, offset, YUV2020_2RGB_FULL,
								plane->y2r.mode, primary);
					break;
				default:
					break;
				}
			}
		}
	}
	plane_commit(hw, display_id);
}

#ifdef CONFIG_PHYTIUM_CHANGE_PIXCLK
#ifndef CONFIG_PHYTIUM_PCIE
static void phytium_change_pxlclk_acpi(struct ftd330_drm_private *priv, int display_id, u32 pixelClk)
{
	struct device *dev = &priv->pdev->dev;
	acpi_handle handle = ACPI_HANDLE(dev);
	union acpi_object args_write[4];
	struct acpi_object_list arg_list_write = {
		.pointer = args_write,
		.count = ARRAY_SIZE(args_write),
	};

	acpi_status status;
	long long ret;
	int physical_display_id = phytium_display_virtual_to_physical(priv->info.pipe_mask, display_id);

	args_write[0].type = ACPI_TYPE_INTEGER;
	args_write[0].integer.value = 2;
	args_write[1].type = ACPI_TYPE_INTEGER;
	args_write[1].integer.value = pixelClk;
	args_write[2].type = ACPI_TYPE_INTEGER;
	args_write[2].integer.value = physical_display_id;
	args_write[3].type = ACPI_TYPE_INTEGER;
	args_write[3].integer.value = 0;
	if (!has_acpi_companion(dev))
		pr_err("get acpi device failed\n");

	status = acpi_evaluate_integer(handle, "PSCF", &arg_list_write, &ret);
	if (ACPI_FAILURE(status)) {
		pr_err("No PSCF Method\n");
		return;
	}
	if (ret < 0) {
		pr_err("Failed to set pixel clock");
		return;
	}
}
#ifdef CONFIG_PHYTIUM_LOW_FPS
static void
phytium_change_edp_lowfps_pxlclk_acpi(struct ftd330_drm_private *priv, int physical_display_id, u32 pixelClk)
{
	struct device *dev = &priv->pdev->dev;
	acpi_handle handle = ACPI_HANDLE(dev);
	union acpi_object args_write[4];
	struct acpi_object_list arg_list_write = {
		.pointer = args_write,
		.count = ARRAY_SIZE(args_write),
	};

	acpi_status status;
	long long ret;

	args_write[0].type = ACPI_TYPE_INTEGER;
	args_write[0].integer.value = 2;
	args_write[1].type = ACPI_TYPE_INTEGER;
	args_write[1].integer.value = pixelClk;
	args_write[2].type = ACPI_TYPE_INTEGER;
	args_write[2].integer.value = physical_display_id;
	args_write[3].type = ACPI_TYPE_INTEGER;
	args_write[3].integer.value = 0;
	if (!has_acpi_companion(dev))
		pr_err("get acpi device failed\n");

	status = acpi_evaluate_integer(handle, "PSCF", &arg_list_write, &ret);
	if (ACPI_FAILURE(status)) {
		pr_err("No PSCF Method\n");
		return;
	}
	if (ret < 0) {
		pr_err("Failed to set pixel clock");
		return;
	}
}
#endif
#endif

static void phytium_change_pxlclk(struct dc_hw *hw, u32 id, u32 pixelClk)
{

	struct drm_device *drm_dev = hw->drm_dev;
	struct ftd330_drm_private *priv = drm_dev->dev_private;
#ifndef CONFIG_PHYTIUM_PCIE
	struct platform_device *pdev = priv->pdev;
#endif
#ifdef CONFIG_PHYTIUM_LOW_FPS
	int physical_display_id;
#endif
	if (!drm_dev)
		pr_info("fatal_error,dc->hw not init complete,can not change pixel clock right now\n");
#ifdef CONFIG_PHYTIUM_LOW_FPS
	physical_display_id= phytium_display_virtual_to_physical(priv->info.pipe_mask, id);
#endif

	FTD330_LOG("display-%d change pxlclk to %d\n",id, pixelClk);

#ifdef CONFIG_PHYTIUM_PCIE
	phytium_change_pxlclk_se(priv, id, pixelClk);
#ifdef CONFIG_PHYTIUM_LOW_FPS
	if (priv->info.edp_mask & BIT(physical_display_id)) {
		if (priv->info.edp_mask & BIT(0))
			phytium_change_edp_lowfps_pxlclk_se(priv, 3, pixelClk);
		if (priv->info.edp_mask & BIT(2) || priv->info.edp_mask & BIT(1))
			phytium_change_edp_lowfps_pxlclk_se(priv, 4, pixelClk);
	}
#endif
#else
	if (pdev->dev.of_node) {
		phytium_change_pxlclk_se(priv, id, pixelClk);
#ifdef CONFIG_PHYTIUM_LOW_FPS
		if (priv->info.edp_mask & BIT(physical_display_id)) {
			if (priv->info.edp_mask & BIT(0))
				phytium_change_edp_lowfps_pxlclk_se(priv, 3, pixelClk);
			if (priv->info.edp_mask & BIT(2) || priv->info.edp_mask & BIT(1))
				phytium_change_edp_lowfps_pxlclk_se(priv, 4, pixelClk);
		}
#endif
	} else if (has_acpi_companion(&pdev->dev)) {
		phytium_change_pxlclk_acpi(priv, id, pixelClk);
#ifdef CONFIG_PHYTIUM_LOW_FPS
		if (priv->info.edp_mask & BIT(physical_display_id)) {
			if (priv->info.edp_mask & BIT(0))
				phytium_change_edp_lowfps_pxlclk_acpi(priv, 3, pixelClk);
			if (priv->info.edp_mask & BIT(2) || priv->info.edp_mask & BIT(1))
				phytium_change_edp_lowfps_pxlclk_acpi(priv, 4, pixelClk);
		}
#endif
	}
#endif
}
#endif


static void display_setup_mode(struct dc_hw *hw, u8 id, struct dc_hw_display_mode *mode)
{
	u32 dpi_cfg;
	u32 offset = display_get_addr_offset(hw, id);
	u32 dc_offset = dc_get_offset_from_display(hw, id);

	if (mode->enable) {
		phytium_display_power_request_on(hw->drm_dev, phytium_display_virtual_to_physical(hw->pipe_mask, id), false);
	}
	if (mode->enable) {
		switch (mode->bus_format) {
		case MEDIA_BUS_FMT_RGB565_1X16:
			dpi_cfg = 0;
			break;
		case MEDIA_BUS_FMT_RGB666_1X18:
			dpi_cfg = 3;
			break;
		case MEDIA_BUS_FMT_RGB666_1X24_CPADHI:
			dpi_cfg = 4;
			break;
		case MEDIA_BUS_FMT_RGB888_1X24:
			dpi_cfg = 5;
			break;
		case MEDIA_BUS_FMT_RGB101010_1X30:
			dpi_cfg = 6;
			break;
		default:
			dpi_cfg = 5;
			break;
		}
		dc_write(hw, DC_DISPLAY_DPI_CONFIG + offset, dpi_cfg);

#ifdef CONFIG_PHYTIUM_CHANGE_PIXCLK
		phytium_change_pxlclk(hw, id, mode->clock);
#endif

		if (phytium_display_virtual_to_physical(hw->pipe_mask, id) == DISPLAY_2) {
			dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, 0, BIT(1) | BIT(2));
		} else {
			dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, 0, BIT(0) | BIT(2));
		}

		dc_write(hw, DC_DISPLAY_H + offset, mode->h_active | (mode->h_total << 16));
		dc_write(hw, DC_DISPLAY_H_SYNC + offset,
			 mode->h_sync_start | (mode->h_sync_end << 15) |
				 (mode->h_sync_polarity ? 0 : BIT(31)) | BIT(30));
		dc_write(hw, DC_DISPLAY_V + offset, mode->v_active | (mode->v_total << 16));
		dc_write(hw, DC_DISPLAY_V_SYNC + offset,
			 mode->v_sync_start | (mode->v_sync_end << 15) |
				 (mode->v_sync_polarity ? 0 : BIT(31)) | BIT(30));

		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset,
					BIT(12) | BIT(0) | BIT(4) | BIT(8), 0);
	} else {
		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, 0, BIT(12));

		if (phytium_display_virtual_to_physical(hw->pipe_mask, id) == DISPLAY_2) {
			dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, 0, BIT(1) | BIT(2));
		} else {
			dc_set_clear(hw, DC_DISPLAY_PANEL_START + dc_offset, 0, BIT(0) | BIT(2));
		}
#ifdef CONFIG_PHYTIUM_CHANGE_PIXCLK
                phytium_change_pxlclk(hw, id, 0);
#endif

	}
	if (!mode->enable) {
		phytium_display_power_request_off(hw->drm_dev, phytium_display_virtual_to_physical(hw->pipe_mask, id));
	}
	FTD330_LOG("display-%d %sable mode:%dx%d\n",id,
			mode->enable ? "en":"dis", mode->h_active, mode->v_active);
}

static void display_ex_setup_mode(struct dc_hw *hw, u8 id, struct dc_hw_display_mode *mode)
{
	u32 dp_cfg;
	u32 offset = display_get_addr_offset(hw, id);
	bool is_yuv = false;

	if (mode->enable) {
		switch (mode->bus_format) {
		case MEDIA_BUS_FMT_RGB565_1X16:
			dp_cfg = 0;
			break;
		case MEDIA_BUS_FMT_RGB666_1X18:
			dp_cfg = 1;
			break;
		case MEDIA_BUS_FMT_RGB888_1X24:
			dp_cfg = 2;
			break;
		case MEDIA_BUS_FMT_RGB101010_1X30:
			dp_cfg = 3;
			break;
		case MEDIA_BUS_FMT_UYVY8_1X16:
			dp_cfg = 2 << 4;
			is_yuv = true;
			break;
		case MEDIA_BUS_FMT_YUV8_1X24:
			dp_cfg = 4 << 4;
			is_yuv = true;
			break;
		case MEDIA_BUS_FMT_UYVY10_1X20:
			dp_cfg = 8 << 4;
			is_yuv = true;
			break;
		case MEDIA_BUS_FMT_YUV10_1X30:
			dp_cfg = 10 << 4;
			is_yuv = true;
			break;
		case MEDIA_BUS_FMT_UYYVYY8_0_5X24:
			dp_cfg = 12 << 4;
			is_yuv = true;
			break;
		case MEDIA_BUS_FMT_UYYVYY10_0_5X30:
			dp_cfg = 13 << 4;
			is_yuv = true;
			break;
		default:
			dp_cfg = 2;
			break;
		}
		if (is_yuv)
			dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, BIT(16), 0);
		else
			dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, 0, BIT(16));

		if (mode->out == OUT_DP)
			dc_write(hw, DC_DISPLAY_DP_CONFIG + offset, dp_cfg | BIT(3));
		else
			dc_write(hw, DC_DISPLAY_DP_CONFIG + offset, dp_cfg);

		/*When connecting to a 6bpc monitor,
		  there is a noticeable contour line in gray scenes,
		  turn on the dither function to alleviate this phenomenon.
		*/
		if (hw->info->displays[id].dither) {
			DRM_DEBUG_KMS("Enable dither on DC-%d\n", id);
			dc_write(hw, DC_DISPLAY_DITHER_TABLE_LOW + offset, DC_DISPLAY_DITHERTABLE_LOW);
			dc_write(hw, DC_DISPLAY_DITHER_TABLE_HIGH + offset, DC_DISPLAY_DITHERTABLE_HIGH);
			dc_write(hw, DC_DISPLAY_DITHER_CONFIG + offset, BIT(31));
		}
	}

	display_setup_mode(hw, id, mode);
}

static void display_set_ctm(struct dc_hw *hw, u8 hw_id, struct dc_hw_ctm *ctm)
{
	u32 i;
	u32 offset = display_get_addr_offset(hw, hw_id);

	if (ctm->enable) {
		dc_write(hw, DC_DISPLAY_GAMUT_MATRIX_CONFIG + offset, 1);
		for (i = 0; i < 9; i++)
			dc_write(hw, DC_DISPLAY_GAMUT_MATRIX_COEF + 8 * i + offset, ctm->matrix[i]);
	}
	ctm->dirty = false;
}
static void display_set_gamma(struct dc_hw *hw, u8 hw_id, struct dc_hw_gamma *gamma)
{

	u32 i, value;
	u32 offset = display_get_addr_offset(hw, hw_id);

	if (gamma->enable) {
		dc_write(hw, DC_DISPLAY_GAMMA_EX_INDEX + offset, 0x00);
		for (i = 0; i < GAMMA_EX_SIZE; i++) {
			value = gamma->gamma[i][2] | (gamma->gamma[i][1] << 16);
			dc_write(hw, DC_DISPLAY_GAMMA_EX_DATA + offset, value);
			dc_write(hw, DC_DISPLAY_GAMMA_EX_ONE_DATA + offset, gamma->gamma[i][0]);
		}
		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, BIT(13), 0);
	} else
		dc_set_clear(hw, DC_DISPLAY_PANEL_CONFIG + offset, 0, BIT(13));

	gamma->dirty = false;
}



static void display_set_pipe_sync(struct dc_hw *hw, u8 hw_id, u16 sync_mode)
{
	u32 display_offset = display_get_addr_offset(hw, hw_id);

	switch (sync_mode) {
	case FTD330_SINGLE_DC:
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + display_offset, 0, BIT(3) | BIT(4));
		break;
	case FTD330_MULTI_DC_PRIMARY:
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + display_offset, BIT(3) | BIT(4), 0);
		break;
	case FTD330_MULTI_DC_SECONDARY:
		dc_set_clear(hw, DC_FRAMEBUFFER_CONFIG_EX + display_offset, BIT(3), BIT(4));
		break;
	default:
		break;
	}
}

static void display_commit(struct dc_hw *hw, u8 display_id)
{
	struct dc_hw_display *display;
	u8 hw_id, display_num = hw->info->display_num;
	u32 i, j;

	for (j = 0; j < display_num; j++) {
		display = &hw->display[j];
		hw_id = hw->info->displays[j].id;
		if (hw_id != display_id)
			continue;

		if (hw->info->pipe_sync)
			display_set_pipe_sync(hw, hw_id, display->sync_mode);

		if (display->ctm.dirty)
			display_set_ctm(hw, hw_id, &display->ctm);

		if (display->gamma.dirty)
			display_set_gamma(hw, hw_id, &display->gamma);

		/* commit dc property */
		for (i = 0; i < display->states.num; i++) {
			struct ftd330_dc_property_state *state = &display->states.items[i];

			if (!state->dirty)
				continue;
			if (!state->proto->config_hw) {
				pr_err("%s: %s not provide config_hw func\n", __func__,
					   state->proto->name);
				continue;
			}
			ftd330_dc_property_config_hw(hw, hw_id, state);
		}
	}
}

#ifdef CONFIG_PHYTIUM_WRITEBACK
static void wb_ex_enable_dump(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb)
{
	u32 offset = display_get_addr_offset(hw, id);

	if (!fb->enable) {
		dc_write(hw, DC_DEST_CONFIG_Address + offset, 0x0);
		fb->dirty = false;
		return;
	}

	/*need update wb_point setting, BIT(17)*/
	dc_write(hw, DC_DEST_CONFIG_Address + offset, BIT(17) | BIT(16) | fb->format);
	dc_write(hw, DC_MEM_DEST_ADDRESS_Address + offset, fb->y_address);
	dc_write(hw, DC_MEM_DEST_UV_PLANE_ADDRESS + offset, fb->u_address);
	dc_write(hw, DC_MEM_DEST_VPLANE_ADDRESS_Address + offset, fb->v_address);
	dc_write(hw, DC_DEST_SRTIDE + offset, fb->y_stride);

	fb->dirty = false;
}

static void wb_enable_dump(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb)
{
	u32 offset = display_get_addr_offset(hw, id);

	if (!fb->enable) {
		sec_write(hw, SE_DC_DEST_CONFIG + offset, 0x0);
		fb->dirty = false;
		return;
	}

	dc_write(hw, DC_DEST_SRTIDE + offset, fb->y_stride);
	sec_write(hw, SE_DC_DEST_ADDRESS + offset, fb->y_address);
	sec_write(hw, SE_DC_DEST_CONFIG + offset, 0x03);

	fb->dirty = false;
}

static void wb_commit(struct dc_hw *hw, u8 hw_id, struct dc_hw_wb *wb)
{
	u32 j = 0;

	if (wb->fb.dirty)
		hw->sub_func->wb_fb(hw, hw_id, &wb->fb);
	for (j = 0; j < wb->states.num; j++) {
		struct ftd330_dc_property_state *state = &wb->states.items[j];

		if (!state->dirty)
			continue;
		if (!state->proto->config_hw) {
			pr_err("%s: %s not provide config_hw func\n", __func__, state->proto->name);
			continue;
		}
		ftd330_dc_property_config_hw(hw, hw_id, state);
	}
}

void dc_hw_setup_wb(struct dc_hw *hw, u8 id)
{
	struct dc_hw_wb *wb = &hw->wb[id];
	u8 hw_id = 0;

	if (wb) {
		hw_id = hw->info->write_back[id].id;
		hw->func->set_wb(hw, hw_id, wb);
	}
}

void dc_hw_update_wb_fb(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb)
{
	struct dc_hw_wb *wb = &hw->wb[id];

	if (wb && fb) {
		if (!fb->enable)
			wb->fb.enable = false;
		else
			memcpy(&wb->fb, fb, sizeof(*fb) - sizeof(fb->dirty));
		wb->fb.dirty = true;
	}
}

#endif

static const struct dc_hw_funcs hw_func = {
	.plane = plane_ex_commit,
	.display = display_commit,
	.set_mode = display_ex_setup_mode,
#ifdef CONFIG_PHYTIUM_WRITEBACK
	.set_wb = wb_commit,
#endif
};

static const struct dc_hw_sub_funcs hw_sub_func[] = { {
#ifdef CONFIG_PHYTIUM_WRITEBACK
		.wb_fb = wb_ex_enable_dump,
#endif
		/* TBD */
	},
	{
#ifdef CONFIG_PHYTIUM_WRITEBACK
		.wb_fb = wb_enable_dump,
#endif
		/* TBD */
						      } };

void dc_hw_commit(struct dc_hw *hw, u8 display_id)
{
	u32 offset = dc_get_offset_from_cursor(hw, display_id);

	hw->func->plane(hw, display_id);
	hw->func->display(hw, display_id);

	if (hw->cursor[display_id].dirty) {
		if (hw->cursor[display_id].enable) {
			dc_write(hw, DC_CURSOR_ADDRESS + offset, hw->cursor[display_id].address);
			dc_write(hw, DC_CURSOR_LOCATION + offset,
					 hw->cursor[display_id].x | (hw->cursor[display_id].y << 16));
			dc_set_clear(
				hw, DC_CURSOR_CONFIG + offset,
				(hw->cursor[display_id].hot_x << 16) | (hw->cursor[display_id].hot_y << 8) |
					(hw->cursor[display_id].size << 5) | BIT(3) | BIT(2) | 0x02,
				(0xFF << 16) | (0xFF << 8) | (0x07 << 5) | 0x1F);
		} else {
			dc_set_clear(hw, DC_CURSOR_CONFIG + offset, BIT(3), 0x03);
		}

		hw->cursor[display_id].dirty = false;
	}


	if (hw->qos.dirty) {
		dc_set_clear(hw, DC_QOS_CONFIG, (hw->qos.high_value << 4) | hw->qos.low_value,
			     0xFF);
		hw->qos.dirty = false;
	}
}

#ifdef CONFIG_PHYTIUM_DEC
void dc_hw_dec_init(struct dc_hw *hw, u8 dc_id)
{
	u32 config = 0;
	u32 dc_offset = dc_id*DC_REG_OFFSET;

	config = DEC_CONTROL_RESET & (~COMPRESSION_DISABLE);
	dc_write(hw, DEC_CONTROL + dc_offset, config | FLUSH_ENABLE);

	config = DEC_CONTROL_EX2_RESET & (~TILE_STATUS_READ_ID_MASK) &
		 (~TILE_STATUS_READ_ID_H_MASK) & (~DISABLE_HW_DEC_FLUSH);
	dc_write(hw, DEC_CONTROL_EX2 + dc_offset, config | (TILE_STATUS_READ_ID_H << 22) | TILE_STATUS_READ_ID);

	config = DEC_CONTROL_EX_RESET & (~WRITE_MISS_POLICY_MASK) & (~READ_MISS_POLICY_MASK);
	dc_write(hw, DEC_CONTROL_EX + dc_offset, config | (WRITE_MISS_POLICY1 << 19));
}

void dc_hw_dec_stream_set(struct dc_hw *hw, u32 main_base_addr, u32 main_end_addr, u32 ts_base_addr,
			  u64 clear_color, u8 tile_mode, u8 align_mode, u8 format, u8 depth, u8 stream_id)
{
	u32 offset = get_offset_from_stream_id(hw, stream_id);
	u32 clear_color_low = clear_color & 0xFFFFFFFF;
	u32 clear_color_high = (clear_color & 0xFFFFFFFF00000000) >> 32;

	dc_set_clear(hw, DEC_READ_CONFIG + offset,
		     (tile_mode << 25) | (align_mode << 16) | (format << 3) | COMPRESSION_EN,
		     TILE_MODE_MASK | COMPRESSION_ALIGN_MODE_MASK | COMPRESSION_FORMAT_MASK);

	dc_set_clear(hw, DEC_READ_EX_CONFIG + offset, (depth << 16), BIT_DEPTH_MASK);

	dc_write(hw, DEC_FAST_CLEAR_VALUE_Address + offset, clear_color_low);
	dc_write(hw, DEC_FAST_CLEAR_VALUE_HIGH_Address + offset, clear_color_high);
	dc_write(hw, DEC_READ_BUFFER_BASE + offset, main_base_addr);
	dc_write(hw, DEC_READ_BUFFER_END + offset, main_end_addr);
	dc_write(hw, DEC_READ_CACHE_BASE + offset, ts_base_addr);
}

void dc_hw_dec_stream_disable(struct dc_hw *hw, u8 stream_id)
{
	u32 offset = get_offset_from_stream_id(hw, stream_id);

	dc_write(hw, DEC_READ_CONFIG + offset, DEC_READ_CONFIG_RESET);
	dc_write(hw, DEC_READ_BUFFER_BASE + offset, 0xFFFFFFFF);
	dc_write(hw, DEC_READ_BUFFER_END + offset, 0xFFFFFFFF);
}
#endif

#ifdef CONFIG_PHYTIUM_MMU
int dc_hw_mmu_init(struct dc_hw *hw, dc_mmu_pt mmu, u32 dc_id)
{
	const struct dc_hw_mmu_reg *reg;
	u32 mtlb = 0, ext_mtlb = 0;
	u32 safe_addr = 0, ext_safe_addr = 0;
	u32 config = 0;
	u32 dc_offset = dc_id*DC_REG_OFFSET;

	reg = &dc_mmu_reg;

	mtlb = (u32)(mmu->mtlb_physical & 0xFFFFFFFF);
	ext_mtlb = (u32)(mmu->mtlb_physical >> 32);

	/* more than 40bit physical address */
	if (ext_mtlb & 0xFFFFFF00) {
		pr_err("Mtlb address out of range.\n");
		return -EFAULT;
	}

	config = (ext_mtlb << 20) | (mtlb >> 12);

	if (mmu->mode == MMU_MODE_1K)
		sec_set_clear(hw, reg->context_pd_entry + dc_offset, (config << 4) | BIT(0),
			      (0xFFFFFFF << 4) | (0x07));
	else
		sec_set_clear(hw, reg->context_pd_entry + dc_offset, (config << 4), (0xFFFFFFF << 4) | (0x07));

	safe_addr = (u32)(mmu->safe_page_physical & 0xFFFFFFFF);
	ext_safe_addr = (u32)(mmu->safe_page_physical >> 32);

	if ((safe_addr & 0x3F) || (ext_safe_addr & 0xFFFFFF00)) {
		pr_err("Invalid safe_address.\n");
		return -EFAULT;
	}

	sec_write(hw, reg->table_array_size + dc_offset, 1);
	sec_write(hw, reg->safe_secure + dc_offset, safe_addr);
	sec_write(hw, reg->safe_non_secure + dc_offset, safe_addr);

	sec_set_clear(hw, reg->safe_ex + dc_offset, (ext_safe_addr << 16) | ext_safe_addr,
		      BIT(31) | (0xFF << 16) | BIT(15) | 0xFF);

		/* mmu configuration for ree driver */
	sec_write(hw, reg->mmu_control + dc_offset, BIT(5) | BIT(0));

	sec_write(hw, SE_MMU_REG_INTR_ENBL + dc_offset, 0xFFFFFFFF);

	return 0;
}

void dc_hw_enable_mmu_prefetch(struct dc_hw *hw, bool enable)
{
	int i = 0;
	u32 dc_num = (hw->total_pipes == 1) ? 1 : 2;

	if (!hw->info->mmu_prefetch)
		return;

	for (i = 0; i < dc_num; i++) {
		if (enable)
			dc_write(hw, DC_MMU_PREFETCH, BIT(0));
		else
			dc_write(hw, DC_MMU_PREFETCH, 0);
	}
}
#endif

void dc_hw_mmu_flush(struct dc_hw *hw)
{
	const struct dc_hw_mmu_reg *reg = &dc_mmu_reg;
	u32 value = sec_read(hw, reg->mmu_config);

	sec_write(hw, reg->mmu_config, value | BIT(4));
	sec_write(hw, reg->mmu_config + DC_REG_OFFSET, value | BIT(4));
	sec_write(hw, reg->mmu_config, value);
	sec_write(hw, reg->mmu_config + DC_REG_OFFSET, value);
}

u32 phytium_get_efifb_address(struct dc_hw *hw, u32 reg) {
	int i = DISPLAY_0;
	u32 dc_offset = 0;
	u32 address = 0;

	for (;i < DISPLAY_NUM;i++) {
		if (i == DISPLAY_1) {
			dc_offset = DC_REG_OFFSET;
		} else if (i == DISPLAY_2) {
			dc_offset = DC_REG_OFFSET + 0x04;
		} else {
			dc_offset = 0;
		}

		address = dc_read(hw, reg + dc_offset);
		if (address != 0)
			return address;
	}

	return 0;	
}
