/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_HW_H__
#define __FTD330_DC_HW_H__

#include <linux/version.h>
#include "ftd330_dc_property.h"
#include "drm/ftd330_drm.h"
#include "ftd330_dc_info.h"

#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#endif

#ifdef CONFIG_PHYTIUM_MMU
#include "ftd330_dc_mmu.h"
#endif

#define __phyFIELDSTART(reg_field) \
			(0 ? reg_field)

#define __phyFIELDEND(reg_field) \
			(1 ? reg_field)

#define __phyFIELDSIZE(reg_field) (__phyFIELDEND(reg_field) - __phyFIELDSTART(reg_field) + 1)
#define __phyFIELDALIGN(data, reg_field) (((u32)(data)) << __phyFIELDSTART(reg_field))
#define __phyFIELDMASK(reg_field) \
	((u32)((__phyFIELDSIZE(reg_field) == 32) ? ~0 : (~(~0 << __phyFIELDSIZE(reg_field)))))
#define FTD330_SET_FIELD(data, reg, field, value)                                             \
	((((u32)(data)) & ~__phyFIELDALIGN(__phyFIELDMASK(reg##_##field), reg##_##field)) | \
	 __phyFIELDALIGN((u32)(value)&__phyFIELDMASK(reg##_##field), reg##_##field))
#define AQ_INTR_ACKNOWLEDGE 0x0010
#define AQ_INTR_ENBL 0x0014
#define DC_HW_REVISION 0x0024
#define DC_HW_CHIP_CID 0x0030

#define DC_REG_BASE 0x0800
#define DC_REG_RANGE 0x10000
#define DC_SEC_REG_OFFSET 0x100000
#define DC_SEC_REG_BASE 0x10000
#define DC_SEC_REG_RANGE 0x1000
#define DC_REG_OFFSET 0x8000

#define DC_FRAMEBUFFER_CONFIG 0x1518
#define DC_FRAMEBUFFER_CONFIG_EX 0x1CC0
#define DC_FRAMEBUFFER_SCALE_CONFIG 0x1520
#define DC_FRAMEBUFFER_TOP_LEFT 0x24D8
#define DC_FRAMEBUFFER_BOTTOM_RIGHT 0x24E0
#define DC_FRAMEBUFFER_ADDRESS 0x1400
#define DC_FRAMEBUFFER_U_ADDRESS 0x1530
#define DC_FRAMEBUFFER_V_ADDRESS 0x1538
#define DC_FRAMEBUFFER_STRIDE 0x1408
#define DC_FRAMEBUFFER_U_STRIDE 0x1800
#define DC_FRAMEBUFFER_V_STRIDE 0x1808
#define DC_FRAMEBUFFER_SIZE 0x1810
#define DC_FRAMEBUFFER_SCALE_FACTOR_X 0x1828
#define DC_FRAMEBUFFER_SCALE_FACTOR_Y 0x1830
#define DC_FRAMEBUFFER_H_FILTER_COEF_INDEX 0x1838
#define DC_FRAMEBUFFER_H_FILTER_COEF_DATA 0x1A00
#define DC_FRAMEBUFFER_V_FILTER_COEF_INDEX 0x1A08
#define DC_FRAMEBUFFER_V_FILTER_COEF_DATA 0x1A10
#define DC_FRAMEBUFFER_INIT_OFFSET 0x1A20
#define DC_FRAMEBUFFER_COLOR_KEY 0x1508
#define DC_FRAMEBUFFER_COLOR_KEY_HIGH 0x1510
#define DC_FRAMEBUFFER_CLEAR_VALUE 0x1A18
#define DC_FRAMEBUFFER_COLOR_TABLE_INDEX 0x1818
#define DC_FRAMEBUFFER_COLOR_TABLE_DATA 0x1820
#define DC_FRAMEBUFFER_BG_COLOR 0x1528
#define DC_FRAMEBUFFER_ROI_ORIGIN 0x1CB0
#define DC_FRAMEBUFFER_ROI_SIZE 0x1CB8
#define DC_FRAMEBUFFER_WATER_MARK 0x1CE8
#define DC_FRAMEBUFFER_DEGAMMA_INDEX 0x1D88
#define DC_FRAMEBUFFER_DEGAMMA_DATA 0x1D90
#define DC_FRAMEBUFFER_DEGAMMA_EX_DATA 0x1D98
#define DC_FRAMEBUFFER_YUVTORGB_COEF0 0x1DA0
#define DC_FRAMEBUFFER_YUVTORGB_COEF1 0x1DA8
#define DC_FRAMEBUFFER_YUVTORGB_COEF2 0x1DB0
#define DC_FRAMEBUFFER_YUVTORGB_COEF3 0x1DB8
#define DC_FRAMEBUFFER_YUVTORGB_COEF4 0x1E00
#define DC_FRAMEBUFFER_YUVTORGB_COEFD0 0x1E08
#define DC_FRAMEBUFFER_YUVTORGB_COEFD1 0x1E10
#define DC_FRAMEBUFFER_YUVTORGB_COEFD2 0x1E18
#define DC_FRAMEBUFFER_Y_CLAMP_BOUND 0x1E88
#define DC_FRAMEBUFFER_UV_CLAMP_BOUND 0x1E90
#define DC_FRAMEBUFFER_RGBTORGB_COEF0 0x1E20
#define DC_FRAMEBUFFER_RGBTORGB_COEF1 0x1E28
#define DC_FRAMEBUFFER_RGBTORGB_COEF2 0x1E30
#define DC_FRAMEBUFFER_RGBTORGB_COEF3 0x1E38
#define DC_FRAMEBUFFER_RGBTORGB_COEF4 0x1E40
#define DC_FRAMEBUFFER_BLEND_CONFIG 0x2510
#define DC_FRAMEBUFFER_SRC_GLOBAL_COLOR 0x2500
#define DC_FRAMEBUFFER_DST_GLOBAL_COLOR 0x2508

#define DC_OVERLAY_CONFIG 0x1540
#define DC_OVERLAY_CONFIG_EX 0x2540
#define DC_OVERLAY_SCALE_CONFIG 0x1C00
#define DC_OVERLAY_BLEND_CONFIG 0x1580
#define DC_OVERLAY_TOP_LEFT 0x1640
#define DC_OVERLAY_BOTTOM_RIGHT 0x1680
#define DC_OVERLAY_ADDRESS 0x15C0
#define DC_OVERLAY_U_ADDRESS 0x1840
#define DC_OVERLAY_V_ADDRESS 0x1880
#define DC_OVERLAY_STRIDE 0x1600
#define DC_OVERLAY_U_STRIDE 0x18C0
#define DC_OVERLAY_V_STRIDE 0x1900
#define DC_OVERLAY_SIZE 0x17C0
#define DC_OVERLAY_SCALE_FACTOR_X 0x1A40
#define DC_OVERLAY_SCALE_FACTOR_Y 0x1A80
#define DC_OVERLAY_H_FILTER_COEF_INDEX 0x1AC0
#define DC_OVERLAY_H_FILTER_COEF_DATA 0x1B00
#define DC_OVERLAY_V_FILTER_COEF_INDEX 0x1B40
#define DC_OVERLAY_V_FILTER_COEF_DATA 0x1B80
#define DC_OVERLAY_INIT_OFFSET 0x1BC0
#define DC_OVERLAY_COLOR_KEY 0x1740
#define DC_OVERLAY_COLOR_KEY_HIGH 0x1780
#define DC_OVERLAY_CLEAR_VALUE 0x1940
#define DC_OVERLAY_COLOR_TABLE_INDEX 0x1980
#define DC_OVERLAY_COLOR_TABLE_DATA 0x19C0
#define DC_OVERLAY_SRC_GLOBAL_COLOR 0x16C0
#define DC_OVERLAY_DST_GLOBAL_COLOR 0x1700
#define DC_OVERLAY_ROI_ORIGIN 0x1D00
#define DC_OVERLAY_ROI_SIZE 0x1D40
#define DC_OVERLAY_WATER_MARK 0x1DC0
#define DC_OVERLAY_DEGAMMA_INDEX 0x2200
#define DC_OVERLAY_DEGAMMA_DATA 0x2240
#define DC_OVERLAY_DEGAMMA_EX_DATA 0x2280
#define DC_OVERLAY_YUVTORGB_COEF0 0x1EC0
#define DC_OVERLAY_YUVTORGB_COEF1 0x1F00
#define DC_OVERLAY_YUVTORGB_COEF2 0x1F40
#define DC_OVERLAY_YUVTORGB_COEF3 0x1F80
#define DC_OVERLAY_YUVTORGB_COEF4 0x1FC0
#define DC_OVERLAY_YUVTORGB_COEFD0 0x2000
#define DC_OVERLAY_YUVTORGB_COEFD1 0x2040
#define DC_OVERLAY_YUVTORGB_COEFD2 0x2080
#define DC_OVERLAY_Y_CLAMP_BOUND 0x22C0
#define DC_OVERLAY_UV_CLAMP_BOUND 0x2300
#define DC_OVERLAY_RGBTORGB_COEF0 0x20C0
#define DC_OVERLAY_RGBTORGB_COEF1 0x2100
#define DC_OVERLAY_RGBTORGB_COEF2 0x2140
#define DC_OVERLAY_RGBTORGB_COEF3 0x2180
#define DC_OVERLAY_RGBTORGB_COEF4 0x21C0

#define DC_CURSOR_CONFIG 0x1468
#define DC_CURSOR_ADDRESS 0x146C
#define DC_CURSOR_LOCATION 0x1470
#define DC_CURSOR_BACKGROUND 0x1474
#define DC_CURSOR_FOREGROUND 0x1478
#define DC_CURSOR_CLK_GATING 0x1484
#define DC_CURSOR_CONFIG_EX 0x24E8
#define DC_CURSOR_OFFSET 0x1080

#define DC_DISPLAY_DITHER_CONFIG 0x1410
#define DC_DISPLAY_PANEL_CONFIG 0x1418
#define DC_DISPLAY_PANEL_CONFIG_EX 0x2518
#define DC_DISPLAY_DITHER_TABLE_LOW 0x1420
#define DC_DISPLAY_DITHER_TABLE_HIGH 0x1428
#define DC_DISPLAY_WRITE_BACK_DITHER_TABLE_LOW 0x2650
#define DC_DISPLAY_WRITE_BACK_DITHER_TABLE_HIGH 0x2658
#define DC_DISPLAY_H 0x1430
#define DC_DISPLAY_H_SYNC 0x1438
#define DC_DISPLAY_V 0x1440
#define DC_DISPLAY_V_SYNC 0x1448
#define DC_DISPLAY_CURRENT_LOCATION 0x1450
#define DC_DISPLAY_GAMUT_MATRIX_CONFIG 0x2C38
#define DC_DISPLAY_GAMUT_MATRIX_COEF 0x2C80
#define DC_DISPLAY_GAMUT_MATRIX_OFFSET 0x2CC8
#define DC_DISPLAY_GAMMA_INDEX 0x1458
#define DC_DISPLAY_GAMMA_DATA 0x1460
#define DC_DISPLAY_INT 0x147C
#define DC_DISPLAY_INT_ENABLE 0x1480
#define DC_DISPLAY_DBI_CONFIG 0x1488
#define DC_DISPLAY_GENERAL_CONFIG 0x14B0
#define DC_DISPLAY_DPI_CONFIG 0x14B8
#define DC_DISPLAY_PANEL_START 0x1CCC
#define DC_DISPLAY_DEBUG_COUNTER_SELECT 0x14D0
#define DC_DISPLAY_DEBUG_COUNTER_VALUE 0x14D8
#define DC_DISPLAY_DP_CONFIG 0x1CD0
#define DC_DISPLAY_GAMMA_EX_INDEX 0x1CF0
#define DC_DISPLAY_GAMMA_EX_DATA 0x1CF8
#define DC_DISPLAY_GAMMA_EX_ONE_DATA 0x1D80
#define DC_DISPLAY_RGBTOYUV_COEF0 0x1E48
#define DC_DISPLAY_RGBTOYUV_COEF1 0x1E50
#define DC_DISPLAY_RGBTOYUV_COEF2 0x1E58
#define DC_DISPLAY_RGBTOYUV_COEF3 0x1E60
#define DC_DISPLAY_RGBTOYUV_COEF4 0x1E68
#define DC_DISPLAY_RGBTOYUV_COEFD0 0x1E70
#define DC_DISPLAY_RGBTOYUV_COEFD1 0x1E78
#define DC_DISPLAY_RGBTOYUV_COEFD2 0x1E80
#define DC_RGB_TO_YUV_WB_COEF0 0x2BF8
#define DC_RGB_TO_YUV_WB_COEF1 0x2C00
#define DC_RGB_TO_YUV_WB_COEF2 0x2C08
#define DC_RGB_TO_YUV_WB_COEF3 0x2C10
#define DC_RGB_TO_YUV_WB_COEF4 0x2C18
#define DC_RGB_TO_YUV_WB_COEF_D0 0x2C20
#define DC_RGB_TO_YUV_WB_COEF_D1 0x2C28
#define DC_RGB_TO_YUV_WB_COEF_D2 0x2C30
#define DC_DISPLAY_THREED_LUT_INDEX 0x1CD8
#define DC_DISPLAY_THREED_LUT_DATA 0x1CE0

#ifdef CONFIG_DEBUG_FS
#define DCREG_PANEL_CRC_CONTROL_Address 0x02620
#define DCREG_PANEL_CRC_CONTROL_INITIAL_VALUE (1 : 1)
#define DCREG_FUSA_CTRL_CRC_WRITE_Address 0x06410
#define DCREG_FUSA_CTRL_CRC_WRITE_ENABLE_WB_CRC (0 : 0)
#define DCREG_FUSA_CTRL_CRC_WRITE_INITIAL_VALUE_MODE (1 : 1)
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE0_Address 0x06418
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE0_VALUE (31 : 0)
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE0_Address 0x06430
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE0_VALUE (31 : 0)
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE1_Address 0x06420
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE1_VALUE (31 : 0)
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE1_Address 0x06438
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE1_VALUE (31 : 0)
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE2_Address 0x06428
#define DCREG_FUSA_CRC_WRITE_INITIAL_VALUE_PLANE2_VALUE (31 : 0)
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE2_Address 0x06440
#define DCREG_FUSA_CRC_WRITE_XOR_VALUE_PLANE2_VALUE (31 : 0)
#define DCREG_FUSA_CTRL_CRC_OUTPUT_Address 0x06460
#define DCREG_FUSA_CTRL_CRC_OUTPUT_OUTPUT_FRAME_CRC_EN (0 : 0)
#define DCREG_FUSA_CTRL_CRC_OUTPUT_INITIAL_VALUE_MODE (1 : 1)
#define DCREG_FUSA_OUTPUT_INITIAL_VALUE_FRAME_Address 0x06468
#define DCREG_FUSA_OUTPUT_INITIAL_VALUE_FRAME_VALUE (31 : 0)
#define DCREG_FUSA_OUTPUT_XOR_VALUE_FRAME_Address 0x06470
#define DCREG_FUSA_OUTPUT_XOR_VALUE_FRAME_VALUE (31 : 0)
#define DCREG_PANEL_REGION_CRC_TL_Address 0x02700
#define DCREG_PANEL_REGION_CRC_TL_X (14 : 0)
#define DCREG_PANEL_REGION_CRC_TL_Y (30 : 16)
#define DCREG_PANEL_REGION_CRC_BR_Address 0x02780
#define DCREG_PANEL_REGION_CRC_BR_X (14 : 0)
#define DCREG_PANEL_REGION_CRC_BR_Y (30 : 16)
#define DCREG_PANEL_CRC_CONTROL_Address 0x02620
#define DCREG_PANEL_CRC_CONTROL_INITIAL_VALUE (1 : 1)
#define DCREG_PANEL_CRC_VALUE_Address 0x02630
#define DCREG_FUSA_CRC_WRITE_VALUE_PLANE0_Address 0x06448
#define DCREG_FUSA_CRC_WRITE_VALUE_PLANE1_Address 0x06450
#define DCREG_FUSA_CRC_WRITE_VALUE_PLANE2_Address 0x06458
#define DCREG_PANEL_REGION_CRC_VALUE_Address 0x02680
#endif
#define DC_DEST_SRTIDE 0x1500
#define DC_DEST_CONFIG_Address 0x14F8
#define DC_MEM_DEST_ADDRESS_Address 0x14E8
#define DC_MEM_DEST_UV_PLANE_ADDRESS 0x2640
#define DC_MEM_DEST_VPLANE_ADDRESS_Address 0x2B40
#define SE_DC_DEST_CONFIG 0x101E0
#define SE_DC_DEST_ADDRESS 0x101E8

#define AQ_AXI_CONFIG 0x00008
#define AQ_HI_CLOCK_CONTROL 0x00000
#define AQ_INTR_ENBL_Address 0x00014

#define DC_CLK_GATTING 0x1A28
#define DC_QOS_CONFIG 0x1A38

#define DC_TRANSPARENCY_OPAQUE 0x00
#define DC_TRANSPARENCY_KEY 0x02
#define DC_DISPLAY_DITHERTABLE_LOW 0x7B48F3C0
#define DC_DISPLAY_DITHERTABLE_HIGH 0x596AD1E2
#define DC_DISPLAY_WRITE_BACK_DITHERTABLE_LOW 0x7B48F3C0
#define DC_DISPLAY_WRITE_BACK_DITHERTABLE_HIGH 0x596AD1E2
#ifdef CONFIG_DEBUG_FS
#define DCREG_FUSA_CTRL_CRC_READ_Address 0x06C08
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_VIDEO0_PLANE0_Address 0x06C6C
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_VIDEO0_PLANE1_Address 0x06C70
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_VIDEO0_PLANE2_Address 0x06C74
#define DCREG_FUSA_CRC_READ_XOR_VALUE_VIDEO0_PLANE0_Address 0x06CCC
#define DCREG_FUSA_CRC_READ_XOR_VALUE_VIDEO0_PLANE1_Address 0x06CD0
#define DCREG_FUSA_CRC_READ_XOR_VALUE_VIDEO0_PLANE2_Address 0x06CD4
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_OVERLAY0_PLANE0_Address 0x06C84
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_OVERLAY0_PLANE1_Address 0x06C88
#define DCREG_FUSA_CRC_READ_INITIAL_VALUE_OVERLAY0_PLANE2_Address 0x06C8C
#define DCREG_FUSA_CRC_READ_XOR_VALUE_OVERLAY0_PLANE0_Address 0x06CE4
#define DCREG_FUSA_CRC_READ_XOR_VALUE_OVERLAY0_PLANE1_Address 0x06CE8
#define DCREG_FUSA_CRC_READ_XOR_VALUE_OVERLAY0_PLANE2_Address 0x06CEC
#define DCREG_FUSA_CRC_READ_GOLDEN_VIDEO0_PLANE0_Address 0x06C0C
#define DCREG_FUSA_CRC_READ_GOLDEN_VIDEO0_PLANE1_Address 0x06C10
#define DCREG_FUSA_CRC_READ_GOLDEN_VIDEO0_PLANE2_Address 0x06C14
#define DCREG_FUSA_CRC_READ_GOLDEN_OVERLAY0_PLANE0_Address 0x06C24
#define DCREG_FUSA_CRC_READ_GOLDEN_OVERLAY0_PLANE1_Address 0x06C28
#define DCREG_FUSA_CRC_READ_GOLDEN_OVERLAY0_PLANE2_Address 0x06C2C
#define DCREG_FUSA_CRC_READ_STATUS_Address 0x06D3C
#endif /* CONFIG_DEBUG_FS */

#define MAX_CRC_CORE_NUM 2 /* For OFIFO_OUT CRC */

#define RGB_TO_RGB_TABLE_SIZE 9
#define YUV_TO_RGB_TABLE_SIZE 16
#define RGB_TO_YUV_TABLE_SIZE 12

#ifdef CONFIG_PHYTIUM_DEC
/* DEC400 register */
#define DEC_CONTROL 0x0800
#define DEC_CONTROL_EX 0x0804
#define DEC_CONTROL_EX2 0x0808
#define DEC_FAST_CLEAR_VALUE_Address 0x1300
#define DEC_FAST_CLEAR_VALUE_HIGH_Address 0x1380

#define DEC_READ_CONFIG 0x0880
#define DEC_READ_EX_CONFIG 0x0900
#define DEC_READ_BUFFER_BASE 0x0A80
#define DEC_READ_BUFFER_END 0x0B80
#define DEC_READ_CACHE_BASE 0x1080

#define DEC_CONTROL_RESET 0x0301018A
#define DEC_CONTROL_EX_RESET 0x00080000
#define DEC_CONTROL_EX2_RESET 0x103FC810
#define DEC_READ_CONFIG_RESET 0x00020000
#endif

#ifdef CONFIG_PHYTIUM_MMU
#define DC_MMU_PREFETCH 0x1E98

#define MMU_REG_BASE 0x0180
#define MMU_REG_RANGE 0x700

#define MMU_REG_CONFIG 0x0184
#define MMU_REG_CONTROL 0x0388
#define MMU_REG_TABLE_ARRAY_SIZE 0x0394
#define MMU_REG_SAFE_NON_SECURE 0x0398
#define MMU_REG_SAFE_SECURE 0x039C
#define MMU_REG_SAFE_EXT_ADDRESS 0x03A0
#define MMU_REG_CONTEXT_PD 0x03B4

#define DEC_REG_CONTROL 0x0800
#define DEC_REG_CONTROL_VALUE 0x02010188

#define SE_MMU_REG_CONFIG 0x10010
#define SE_MMU_REG_CONTROL 0x10024
#define SE_MMU_REG_TABLE_ARRAY_SIZE 0x10030
#define SE_MMU_REG_SAFE_NON_SECUR 0x10034
#define SE_MMU_REG_SAFE_SECURE 0x10038
#define SE_MMU_REG_SAFE_EXT_ADDRESS 0x1003C
#define SE_MMU_REG_CONTEXT_PD 0x10040
#define SE_MMU_REG_INTR_ENBL 0x10044
#define SE_MMU_INTR_ACKNOWLEDGE 0x10048
#endif
#ifdef CONFIG_DEBUG_FS
#define DCREG_PANEL_COLOR_BAR_CONTROL_Address 0x02638
#define DCREG_PANEL_COLOR_BAR_TL_Address 0x02800
#define DCREG_PANEL_COLOR_BAR_BR_Address 0x02880
#define DCREG_PANEL_COLOR_BAR_COLOR_Address 0x02900
#define DCREG_PANEL_COLOR_BAR_TL_X (14 : 0)
#define DCREG_PANEL_COLOR_BAR_TL_Y (30 : 16)
#define DCREG_PANEL_COLOR_BAR_BR_X (14 : 0)
#define DCREG_PANEL_COLOR_BAR_BR_Y (30 : 16)
#define DCREG_PANEL_COLOR_BAR_COLOR_VALUE (31 : 0)
#endif

enum dc_hw_color_format {
	FORMAT_X4R4G4B4,
	FORMAT_A4R4G4B4,
	FORMAT_X1R5G5B5,
	FORMAT_A1R5G5B5,
	FORMAT_R5G6B5,
	FORMAT_X8R8G8B8,
	FORMAT_A8R8G8B8,
	FORMAT_YUY2,
	FORMAT_UYVY,
	FORMAT_INDEX8,
	FORMAT_MONOCHROME,
	FORMAT_YV12 = 0xf,
	FORMAT_A8,
	FORMAT_NV12,
	FORMAT_NV16,
	FORMAT_RG16,
	FORMAT_R8,
	FORMAT_NV12_10BIT,
	FORMAT_A2R10G10B10,
	FORMAT_NV16_10BIT,
	FORMAT_INDEX1,
	FORMAT_INDEX2,
	FORMAT_INDEX4,
	FORMAT_P010,
	FORMAT_YUV444,
	FORMAT_YUV444_10BIT,
	FORMAT_RGB888P,
};

enum dc_hw_csc_gamut {
	CSC_GAMUT_601 = 0,
	CSC_GAMUT_709 = 1,
	CSC_GAMUT_2020 = 3,
};

enum dc_hw_rotation {
	ROT_0 = 0,
	ROT_90 = 4,
	ROT_180 = 5,
	ROT_270 = 6,
	FLIP_X = 1,
	FLIP_Y = 2,
	FLIP_XY = 3,
};

enum dc_hw_swizzle {
	SWIZZLE_ARGB = 0,
	SWIZZLE_RGBA,
	SWIZZLE_ABGR,
	SWIZZLE_BGRA,
};

enum dc_hw_out {
	OUT_DPI,
	OUT_DP,
};

enum dc_hw_cursor_size {
	CURSOR_SIZE_32X32 = 0,
	CURSOR_SIZE_64X64,
	CURSOR_SIZE_128X128,
};

enum dc_hw_blend_mode {
	/* out.rgb = plane_alpha * fg.rgb +
	 *      (1 - (plane_alpha * fg.alpha)) * bg.rgb
	 */
	BLEND_PREMULTI,
	/* out.rgb = plane_alpha * fg.alpha * fg.rgb +
	 *      (1 - (plane_alpha * fg.alpha)) * bg.rgb
	 */
	BLEND_COVERAGE,
	/* out.rgb = plane_alpha * fg.rgb +
	 *      (1 - plane_alpha) * bg.rgb
	 */
	BLEND_PIXEL_NONE,
};

enum dc_hw_color_blend_mode {
	BLENDING_ZERO,
	BLENDING_ONE,
	BLENDING_NORMAL,
	BLENDING_INVERSED,
	BLENDING_COLOR,
	BLENDING_COLOR_INVERSED,
	BLENDING_SATURATED_ALPHA,
	BLENDING_SATURATED_DEST_ALPHA,
};

enum dc_hw_csc_mode {
	CSC_MODE_L2L = 0,
	CSC_MODE_F2F = 1,
};
struct dc_hw_plane_reg {
	u32 y_address;
	u32 u_address;
	u32 v_address;
	u32 y_stride;
	u32 u_stride;
	u32 v_stride;
	u32 size;
	u32 top_left;
	u32 bottom_right;
	u32 scale_factor_x;
	u32 scale_factor_y;
	u32 h_filter_coef_index;
	u32 h_filter_coef_data;
	u32 v_filter_coef_index;
	u32 v_filter_coef_data;
	u32 init_offset;
	u32 color_key;
	u32 color_key_high;
	u32 clear_value;
	u32 color_table_index;
	u32 color_table_data;
	u32 scale_config;
	u32 water_mark;
	u32 degamma_index;
	u32 degamma_data;
	u32 degamma_ex_data;
	u32 src_global_color;
	u32 dst_global_color;
	u32 blend_config;
	u32 roi_origin;
	u32 roi_size;
	u32 YUVToRGBCoef0;
	u32 YUVToRGBCoef1;
	u32 YUVToRGBCoef2;
	u32 YUVToRGBCoef3;
	u32 YUVToRGBCoef4;
	u32 YUVToRGBCoefD0;
	u32 YUVToRGBCoefD1;
	u32 YUVToRGBCoefD2;
	u32 YClampBound;
	u32 UVClampBound;
	u32 RGBToRGBCoef0;
	u32 RGBToRGBCoef1;
	u32 RGBToRGBCoef2;
	u32 RGBToRGBCoef3;
	u32 RGBToRGBCoef4;
	u32 LayerConfigEx;
};

#ifdef CONFIG_PHYTIUM_MMU
struct dc_hw_mmu_reg {
	u32 mmu_config;
	u32 mmu_control;
	u32 table_array_size;
	u32 safe_non_secure;
	u32 safe_secure;
	u32 safe_ex;
	u32 context_pd_entry;
};
#endif

struct dc_hw_fb {
	u32 y_address;
	u32 u_address;
	u32 v_address;
	u32 water_mark;
	u32 y_stride;
	u32 u_stride;
	u32 v_stride;
	u16 width;
	u16 height;
	u8 format;
	u8 tile_mode;
	u8 rotation;
	u8 yuv_gamut;
	u8 swizzle;
	u8 uv_swizzle;
	u8 zpos;
	u8 display_id;
	bool dec_enable;
	bool enable;
	bool dirty;
};

struct dc_hw_scale {
	u32 scale_factor_x;
	u32 scale_factor_y;
	bool enable;
	bool dirty;
};

struct dc_hw_position {
	u16 start_x;
	u16 start_y;
	u16 end_x;
	u16 end_y;
	bool dirty;
};

struct dc_hw_blend {
	u8 alpha;
	u8 blend_mode;
	bool enable;
	bool dirty;
};

struct dc_hw_colorkey {
	u32 colorkey;
	u32 colorkey_high;
	u8 transparency;
	bool dirty;
};

struct dc_hw_roi {
	u16 x;
	u16 y;
	u16 width;
	u16 height;
	bool enable;
	bool dirty;
};

struct dc_hw_cursor {
	u32 address;
	long x;
	long y;
	long hot_x;
	long hot_y;
	u8 size;
	u8 display_id;
	bool enable;
	bool dirty;
};

struct dc_hw_display_mode {
	enum dc_hw_out out;
	u32 bus_format;
	u32 clock;
	u16 h_active;
	u16 h_total;
	u16 h_sync_start;
	u16 h_sync_end;
	u16 v_active;
	u16 v_total;
	u16 v_sync_start;
	u16 v_sync_end;
	bool h_sync_polarity;
	bool v_sync_polarity;
	bool enable;
};

struct dc_hw_ctm {
	u64 *matrix;
	bool enable;
	bool dirty;
};
struct dc_hw_gamma {
	u16 gamma[GAMMA_EX_SIZE][3];
	bool enable;
	bool dirty;
};

struct dc_hw_plane_qos {
/* TODO:TBD */
};
struct dc_hw_wb_qos {
};
struct dc_hw_disp_qos {
};

struct dc_hw_disp_crc {
	bool enable;
	u8 pos;
	bool index[MAX_CRC_NUM];
	u8 init_mode;
	u32 xor_value;
	u32 init_value;
	u32 full_crc_value[3];
	u32 roi_crc_value[MAX_CRC_NUM];
	struct drm_ftd330_rect crc_roi_rect[MAX_CRC_NUM];
};
struct dc_hw_crc {
	bool enable;
	u8 pos;
	struct drm_ftd330_color seed;
	struct drm_ftd330_color result;
	u8 init_mode;
	__u32 init_value;
	__u32 xor_value;
	__u32 golden[3];
};

struct dc_hw_pattern_entry {
	bool enable;
	u8 index;
	u64 color;
	struct drm_ftd330_rect rect;
};
struct dc_hw_pattern {
	struct dc_hw_pattern_entry pattern_entry[FTD330_MAX_COLOR_BAR_NUM];
};
struct dc_hw_y2r {
	u8 gamut;
	u8 mode;
	bool dirty;
	bool enable;
};
struct dc_hw_plane {
	const struct ftd330_plane_info *info;
	struct dc_hw_fb fb;
	struct dc_hw_roi roi;
	struct dc_hw_position pos;
	struct dc_hw_blend blend;
	struct dc_hw_crc crc;
	struct ftd330_dc_property_state_group states;
	struct dc_hw_y2r y2r;
	struct ftd330_plane *ftd330_plane;
};

struct dc_hw_display {
	const struct ftd330_display_info *info;
	struct dc_hw_display_mode mode;
	struct dc_hw_ctm ctm;
	struct dc_hw_gamma gamma;
	u16 sync_mode;
	bool sync_enable;
	u32 vblank_count;
	bool vblank_enable;
	u32 underflow_count;
	struct ftd330_dc_property_state_group states;
};

struct dc_hw_qos {
	u8 low_value;
	u8 high_value;
	bool dirty;
};

struct dc_hw_wb {
	const struct ftd330_wb_info *info;
	struct dc_hw_fb fb;
	struct ftd330_dc_property_state_group states;
	bool wb_frm_done;
	bool config_status;
	/* TBD */
};

struct dc_hw_read {
	u32 reg;
	u32 value;
};

struct dc_hw;
struct dc_hw_funcs {
	void (*plane)(struct dc_hw *hw, u8 display_id);
	void (*display)(struct dc_hw *hw, u8 display_id);
	void (*set_mode)(struct dc_hw *hw, u8 hw_id, struct dc_hw_display_mode *mode);
	void (*set_wb)(struct dc_hw *hw, u8 hw_id, struct dc_hw_wb *wb);
};

struct dc_hw_sub_funcs {
	void (*wb_fb)(struct dc_hw *hw, u8 hw_id, struct dc_hw_fb *fb);
	/* TBD */
};

struct dc_hw_interrupt_status {
	u8 display_frm_done;
	u8 display_underflow;
	u8 wb_frm_done;
	u8 wb_datalost;
};

struct dc_hw {
	enum dc_chip_rev rev;
	void *hi_base;
	void *reg_base;
#if defined(CONFIG_PHYTIUM_MMU) || defined(CONFIG_PHYTIUM_WRITEBACK)
	void *sec_base;
#endif

#ifdef CONFIG_PHYTIUM_DEBUG
	struct file *dc_capture_fp;
#endif
	struct dc_hw_display display[DC_DISPLAY_NUM];
	struct dc_hw_plane plane[DC_LAYER_NUM];
	struct dc_hw_cursor cursor[DC_CURSOR_NUM];
	struct dc_hw_wb wb[DC_WB_NUM];
	struct dc_hw_qos qos;
	struct dc_hw_funcs *func;
	struct dc_hw_sub_funcs *sub_func;
	struct ftd330_dc_info *info;
	const struct ftd330_output_info *output_info;
	int total_pipes;
	int pipe_mask;
	struct drm_device *drm_dev;
	bool phytium_log_enable;
	unsigned char overlay_enable;
};

void ftd330_dc_hw_get_plane_config(struct dc_hw *hw, u32 id, u32 *offset, const struct dc_hw_plane_reg **reg,
			       bool *primary);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 18))
inline void dc_write(struct dc_hw *hw, u32 reg, u32 value);
inline void dc_set_clear(struct dc_hw *hw, u32 reg, u32 set, u32 clear);
#else
void dc_write(struct dc_hw *hw, u32 reg, u32 value);
void dc_set_clear(struct dc_hw *hw, u32 reg, u32 set, u32 clear);
#endif
int dc_hw_init(struct dc_hw *hw);
void dc_hw_deinit(struct dc_hw *hw);
u32 hi_read(struct dc_hw *hw, u32 reg);
bool is_dc_powered(struct dc_hw *hw, int dc_id);
const struct dc_hw_plane *ftd330_dc_hw_get_plane(const struct dc_hw *hw, u32 hw_id);
const struct dc_hw_display *ftd330_dc_hw_get_display(const struct dc_hw *hw, u32 hw_id);
void dc_hw_update_plane(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb, struct dc_hw_position *pos,
			struct dc_hw_blend *blend, struct dc_hw_roi *roi);
void dc_hw_update_qos(struct dc_hw *hw, struct dc_hw_qos *qos);
void dc_hw_update_cursor(struct dc_hw *hw, u8 id, struct dc_hw_cursor *cursor);
void dc_hw_update_ctm(struct dc_hw *hw, u8 id, u64 *matrix);
void dc_hw_update_plane_y2r(struct dc_hw *hw, u8 id, struct dc_hw_y2r *y2r_conf);
void dc_hw_enable_ctm(struct dc_hw *hw, u8 id, bool enable);
void dc_hw_update_gamma(struct dc_hw *hw, u8 id, u16 index, u16 r, u16 g, u16 b);
void dc_hw_enable_gamma(struct dc_hw *hw, u8 id, bool enable);
void dc_hw_setup_display_mode(struct dc_hw *hw, u8 id, struct dc_hw_display_mode *mode);
u32 dc_hw_get_vblank_count(struct dc_hw *hw, u8 id);
void dc_hw_enable_interrupt(struct dc_hw *hw, bool enable, u32 display_index);
int dc_hw_get_interrupt(struct dc_hw *hw, struct dc_hw_interrupt_status *status, u32 dc_id);
bool dc_hw_check_underflow(struct dc_hw *hw, u32 display_id);
void dc_hw_enable_shadow_register(struct dc_hw *hw, bool enable, u8 display_id);
void dc_hw_do_reset(struct dc_hw *hw, u32 dc_id);
void dc_hw_start_trigger(struct dc_hw *hw, u8 display_id);
void dc_hw_stop_trigger(struct dc_hw *hw, u8 display_id);
void dc_hw_commit(struct dc_hw *hw, u8 display_id);
void load_rgb_to_yuv(struct dc_hw *hw, u32 offset, s32 *table);
void load_wb_rgb_to_yuv(struct dc_hw *hw, u32 offset, s32 *table);
void dc_hw_set_plane_roi(struct dc_hw *hw, u8 hw_id, struct dc_hw_roi *roi);
int phytium_display_virtual_to_physical(int pipe_mask, u32 id);
int phytium_display_physical_to_virtual(int pipe_mask, u32 id);
int phytium_plane_to_display(struct dc_hw *hw, u32 plane_id);
u32 dc_get_offset_from_cursor(struct dc_hw *hw, u32 display_id);
u32 plane_get_addr_offset(struct dc_hw *hw, u32 id);
u32 display_get_addr_offset(struct dc_hw *hw, u32 id);
u32 get_dc_from_display(struct dc_hw *hw, u32 id);
u32 dc_get_offset_from_plane(struct dc_hw *hw, u32 id);
u32 dc_get_offset_from_display(struct dc_hw *hw, u32 id);
u32 phytium_get_efifb_address(struct dc_hw *hw, u32 reg);
#ifdef CONFIG_PHYTIUM_DEC
u32 get_offset_from_stream_id(struct dc_hw *hw, u8 stream_id);
#endif


#ifdef CONFIG_PHYTIUM_WRITEBACK
void dc_hw_setup_wb(struct dc_hw *hw, u8 id);
void dc_hw_update_wb_fb(struct dc_hw *hw, u8 id, struct dc_hw_fb *fb);
#endif

#ifdef CONFIG_PHYTIUM_DEC
void dc_hw_dec_init(struct dc_hw *hw, u8 dc_id);
void dc_hw_dec_stream_set(struct dc_hw *hw, u32 main_base_addr, u32 main_end_addr, u32 ts_base_addr,
			   u64 clear_color, u8 tile_mode, u8 align_mode, u8 format, u8 depth, u8 stream_id);
void dc_hw_dec_stream_disable(struct dc_hw *hw, u8 stream_id);
#endif

#ifdef CONFIG_PHYTIUM_MMU
int dc_hw_mmu_init(struct dc_hw *hw, dc_mmu_pt mmu, u32 dc_id);
void dc_hw_enable_mmu_prefetch(struct dc_hw *hw, bool enable);
#endif

void dc_hw_mmu_flush(struct dc_hw *hw);


#endif /* __FTD330_DC_HW_H__ */
