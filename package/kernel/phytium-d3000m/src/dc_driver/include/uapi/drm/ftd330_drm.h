/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DRM_H__
#define __FTD330_DRM_H__

#include <drm/drm.h>

#ifndef __KERNEL__
#include <stdbool.h>
#endif

/* Alignment with a power of two value. */
#define FTD330_ALIGN(n, align) (((n) + ((align)-1)) & ~((align)-1))

#define FTD330_ENABLE_CMD_LIST 0

/* FilterBlt information. */
#define FTD330_MAXKERNELSIZE		9
#define FTD330_SUBPIXELINDEXBITS	5

#define FTD330_SUBPIXELCOUNT (1 << FTD330_SUBPIXELINDEXBITS)

#define FTD330_SUBPIXELLOADCOUNT (FTD330_SUBPIXELCOUNT / 2 + 1)

#define FTD330_MAX_Y2R_COEF_NUM 15
#define FTD330_MAX_R2Y_COEF_NUM 15
#define FTD330_MAX_GAMUT_COEF_NUM 12

#define FTD330_MAX_LUT_SEG_CNT 10
#define FTD330_MAX_LUT_ENTRY_CNT 129
#define FTD330_MAX_PRIOR_3DLUT_SIZE 4913
#define FTD330_MAX_ROI_3DLUT_SIZE 729
#define FTD330_MAX_1D_LUT_ENTRY_CNT 129
#define FTD330_MAX_GAMMA_ENTRY_CNT 257
#define FTD330_MAX_GAMMA_EX_ENTRY_CNT 300
#define FTD330_MAX_COLOR_BAR_NUM 16

#define FTD330_LTM_LUMA_COEF_NUM 5
#define FTD330_LTM_FREQ_COEF_NUM 9
#define FTD330_LTM_FREQ_NORM_FRAC_BIT 18
#define FTD330_LTM_AFFINE_LUT_NUM 1152 /* 12x12x8 */
#define FTD330_LTM_AFFINE_SLICE_NUM 2
#define FTD330_LTM_AFFINE_OUT_SCALE_SIZE 17
#define FTD330_LTM_TONE_ADJ_COEF_NUM 129
#define FTD330_LTM_ALPHA_GAIN_SIZE 121
#define FTD330_LTM_ALPHA_LUMA_SIZE 129
#define FTD330_LTM_SATU_CTRL_SIZE 257
#define FTD330_LTM_XGAMMA_COEF_NUM 65
#define FTD330_LTM_DITHER_COEF_NUM 3
#define FTD330_LTM_CD_COEF_NUM 4
#define FTD330_LTM_CD_THRESH_NUM 4
#define FTD330_LTM_CD_SLOPE_NUM 2
#define FTD330_LTM_CD_RESULT_NUM 256
#define FTD330_LTM_HIST_POS_NUM 2
#define FTD330_LTM_HIST_SCALE_NUM 2
#define FTD330_LTM_HIST_RESULT_NUM 9216 /* 12x12x64 */
#define FTD330_HIST_RESULT_BIN_CNT 256

#define FTD330_SHARPNESS_CSC_COEF_NUM 9
#define FTD330_SHARPNESS_CSC_OFFSET_NUM 3
#define FTD330_SHARPNESS_LUMA_GAIN_LUT_ENTRY_NUM 9
#define FTD330_SHARPNESS_CA_MODE_NUM 3
#define FTD330_SHARPNESS_CA_PARAM_NUM 7
#define FTD330_SHARPNESS_LPF_COEF_NUM 4
#define FTD330_SHARPNESS_LPF_NOISE_LUT_NUM 9
#define FTD330_SHARPNESS_LPF_CURVE_LUT_NUM 13
#define FTD330_SHARPNESS_LPF_NORM_BPP 16


#define FTD330_RANDOM_DITHER_SEED_NUM 8

#define FTD330_SCALE_HORI_COEF_NUM 128
#define FTD330_SCALE_VERT_COEF_NUM 128

#define FTD330_MAX_ROI_CNT 2
#define FTD330_MAX_EXT_LAYER_CNT 3

#define FTD330_BLEND_ALPHA_OPAQUE 0x3ff

/* Max blur ROI width is 1440, min blur width is 100 */
#define FTD330_BLUR_COEF_NUM 4
#define FTD330_BLUR_COEF_SUM_MIN 1
#define FTD330_BLUR_COEF_SUM_MAX 64
#define FTD330_BLUR_NORM_BPP 16
#define FTD330_BLUR_ROI_MAX_WIDTH 1440
#define FTD330_BLUR_ROI_MIN_WIDTH 100

/* brightness: Max target gain is 2^10, max brightness value is 2^14 - 1 */
#define FTD330_MAX_TARGET_GAIN_VALUE 1024
#define FTD330_MAX_BRIGHTNESS_VALUE 16383

#define DEGAMMA_SIZE 260

#define FTD330_NR_COEF_NUM 25
#define FTD330_NR_NULL_COEF_NUM 17
#define FTD330_SSR3_COEF_NUM 268
enum drm_ftd330_degamma_mode {
	FTD330_DEGAMMA_BT709 = 0,
	FTD330_DEGAMMA_BT2020 = 1,
	FTD330_DEGAMMA_USR = 2,
};
struct drm_ftd330_degamma_config {
	enum drm_ftd330_degamma_mode mode;
	__u16 r[DEGAMMA_SIZE];
	__u16 g[DEGAMMA_SIZE];
	__u16 b[DEGAMMA_SIZE];
};

enum drm_ftd330_sync_dc_mode {
	FTD330_SINGLE_DC = 0,
	FTD330_MULTI_DC_PRIMARY = 1,
	FTD330_MULTI_DC_SECONDARY = 2,
};

enum drm_ftd330_mmu_prefetch_mode {
	FTD330_MMU_PREFETCH_DISABLE = 0,
	FTD330_MMU_PREFETCH_ENABLE = 1,
};

enum drm_ftd330_dma_mode {
	/* read full image */
	FTD330_DMA_NORMAL = 0,
	/* read one ROI region in the image */
	FTD330_DMA_ONE_ROI = 1,
	/* read two ROI regions in the image */
	FTD330_DMA_TWO_ROI = 2,
	/* skip the ROI region in the image */
	FTD330_DMA_SKIP_ROI = 3,
	/* for extend layer mode
	 * read full image0 and image1, don't.
	 */
	FTD330_DMA_EXT_LAYER = 4,
	/* for extend layer mode
	 * read ROI region from image0 and
	 * read exrended ROI region from image1.
	 */
	FTD330_DMA_EXT_LAYER_EX = 5,
};

enum drm_ftd330_line_padding_mode {
	/*ratio 1:1*/
	FTD330_DMA_LINE_PADDING_1TO1 = 0,
	/*ratio 2:1*/
	FTD330_DMA_LINE_PADDING_2TO1 = 1,
	/*ratio 3:1*/
	FTD330_DMA_LINE_PADDING_3TO1 = 2,
	/*ratio 3:2*/
	FTD330_DMA_LINE_PADDING_3TO2 = 3,
	/*ratio 4:1*/
	FTD330_DMA_LINE_PADDING_4TO1 = 4,
	/*ratio 4:3*/
	FTD330_DMA_LINE_PADDING_4TO3 = 5,
	/*ratio 5:2*/
	FTD330_DMA_LINE_PADDING_5TO2 = 6,
	/*ratio 5:3*/
	FTD330_DMA_LINE_PADDING_5TO3 = 7,
	/*ratio 8:5*/
	FTD330_DMA_LINE_PADDING_8TO5 = 8,
};

enum drm_ftd330_sbs_mode {
	FTD330_SBS_LEFT = 0,
	FTD330_SBS_RIGHT = 1,
	FTD330_SBS_SPLIT = 2,
	FTD330_SBS_RESERVED = 3,
};

enum drm_ftd330_alpha_mode {
	FTD330_ALPHA_NORMAL = 0,
	FTD330_ALPHA_INVERSE = 1,
};

enum drm_ftd330_galpha_mode {
	FTD330_GALPHA_NORMAL = 0,
	FTD330_GALPHA_GLOBAL = 1,
	FTD330_GALPHA_MULTIPLE = 2,
};

enum drm_ftd330_ltm_luma_mode {
	FTD330_LTM_LUMA_GRAY = 0,
	FTD330_LTM_LUMA_LIGHTNESS = 1,
	FTD330_LTM_LUMA_MIXED = 2,
};

enum drm_ftd330_blend_mode {
	FTD330_BLD_CLR = 0,
	FTD330_BLD_SRC = 1,
	FTD330_BLD_DST = 2,
	FTD330_BLD_SRC_OVR = 3,
	FTD330_BLD_DST_OVR = 4,
	FTD330_BLD_SRC_IN = 5,
	FTD330_BLD_DST_IN = 6,
	FTD330_BLD_SRC_OUT = 7,
	FTD330_BLD_DST_OUT = 8,
	FTD330_BLD_SRC_ATOP = 9,
	FTD330_BLD_DST_ATOP = 10,
	FTD330_BLD_XOR = 11,
	FTD330_BLD_PLUS = 12,
	FTD330_BLD_BLD = 13,
	FTD330_BLD_UDEF = 14,
};

enum drm_ftd330_wb_point {
	FTD330_WB_DISP_OUT = 0,
	FTD330_WB_DISP_IN = 1,
	FTD330_WB_DISP_CC = 2,
	FTD330_WB_OFIFO_IN = 3,
	FTD330_WB_OFIFO_OUT = 4,
	FTD330_WB_POS_CNT = 5,
};
enum drm_ftd330_eotf_mode {
	FTD330_EO_ProgConf = 1,
	FTD330_EO_PQ = 2,
	FTD330_EO_HLG = 3,
	FTD330_EO_170M = 4,
	FTD330_EO_SRGB = 5,
	FTD330_EO_DeGamma2_2 = 6,
};
enum drm_ftd330_oetf_mode {
	FTD330_OE_ProgConf = 1,
	FTD330_OE_PQ = 2,
	FTD330_OE_HLG = 3,
	FTD330_OE_170M = 4,
	FTD330_OE_SRGB = 5,
	FTD330_OE_ReGamma2_2 = 6,
};
enum drm_ftd330_tonemapping_mode {
	FTD330_TM_HDR10 = 1,
	FTD330_TM_HLG = 2,
	FTD330_TM_ProgConf = 3,
	FTD330_TM_Reinhard = 4,
	FTD330_InvTM_Reinhard = 5,
	FTD330_TM_ProgCurve = 6,
};

enum drm_ftd330_gamut_mode {
	FTD330_GAMUT_709_TO_2020 = 0,
	FTD330_GAMUT_2020_TO_709 = 1,
	FTD330_GAMUT_2020_TO_DCIP3 = 2,
	FTD330_GAMUT_DCIP3_TO_2020 = 3,
	FTD330_GAMUT_DCIP3_TO_SRGB = 4,
	FTD330_GAMUT_SRGB_TO_DCIP3 = 5,
	FTD330_GAMUT_USER_DEF = 6,
};

enum drm_ftd330_csc_mode {
	FTD330_CSC_CM_USR,
	FTD330_CSC_CM_L2L,
	FTD330_CSC_CM_L2F,
	FTD330_CSC_CM_F2L,
	FTD330_CSC_CM_F2F,
};

enum drm_ftd330_csc_gamut {
	FTD330_CSC_CG_601,
	FTD330_CSC_CG_709,
	FTD330_CSC_CG_2020,
	FTD330_CSC_CG_P3,
	FTD330_CSC_CG_SRGB,
};

enum drm_ftd330_calcu_mode {
	FTD330_CALC_LNR_COMBINE = 0,
	FTD330_CALC_MAX = 1,
	FTD330_CALC_MIXED = 2,
};

enum drm_ftd330_data_extend_mode {
	FTD330_DATA_EXT_STD = 0,
	FTD330_DATA_EXT_MSB = 1,
	FTD330_DATA_EXT_RANDOM = 2,
};

enum drm_ftd330_data_trunc_mode {
	FTD330_DATA_TRUNCATE = 0,
	FTD330_DATA_ROUNDING = 1,

};

enum drm_ftd330_dth_frm_idx {
	FTD330_DTH_FRM_IDX_NONE = 0,
	FTD330_DTH_FRM_IDX_SW = 1,
	FTD330_DTH_FRM_IDX_HW = 2,
};

enum drm_ftd330_dth_frm_mode {
	FTD330_DTH_FRM_4 = 4,
	FTD330_DTH_FRM_8 = 8,
	FTD330_DTH_FRM_16 = 16,
};

enum drm_ftd330_pattern_mode {
	FTD330_PURE_COLOR = 0,
	FTD330_COLOR_BAR_H = 1,
	FTD330_COLOR_BAR_V = 2,
	FTD330_RMAP_H = 3,
	FTD330_RMAP_V = 4,
	FTD330_BLACK_WHITE_H = 5,
	FTD330_BLACK_WHITE_V = 6,
	FTD330_BLACK_WHITE_SQR = 7,
	FTD330_BORDER_PATRN = 8,
	FTD330_CURSOR_PATRN = 9,
};

enum drm_ftd330_disp_tp_pos {
	FTD330_DISP_TP_BLD = 0,
	FTD330_DISP_TP_POST_PROC = 1,
	FTD330_DISP_TP_OFIFO = 2,
};

enum drm_ftd330_plane_crc_pos {
	FTD330_PLANE_CRC_DFC = 0,
	FTD330_PLANE_CRC_HDR = 1,
};

enum drm_ftd330_disp_crc_pos {
	FTD330_DISP_CRC_BLD = 0,
	FTD330_DISP_POST_PROC = 1,
	FTD330_DISP_CRC_OFIFO_IN = 2,
	FTD330_DISP_CRC_OFIFO_OUT = 3,
	FTD330_DISP_CRC_WB = 4,
};

enum drm_ftd330_reset_mode {
	FTD330_RESET = 0,
	FTD330_FE0_RESET = 1,
	FTD330_FE1_RESET = 2,
	FTD330_BE_RESET = 3,
};

enum drm_ftd330_feature_cap_type {
	FTD330_FEATURE_CAP_FBC = 0,
	FTD330_FEATURE_CAP_MAX_BLEND_LAYER,
	FTD330_FEATURE_CAP_CURSOR_WIDTH,
	FTD330_FEATURE_CAP_CURSOR_HEIGHT,
	FTD330_FEATURE_CAP_LINEAR_YUV_ROTATION,
	FTD330_FEATURE_CAP_ANY_RESOLUTION,
	FTD330_FEATURE_CAP_MAX_WIDTH,
	FTD330_FEATURE_CAP_MAX_HEIGHT,
};

enum drm_ftd330_sharpness_ink_mode {
	FTD330_SHARPNESS_INK_DEFAULT = 0x0,
	FTD330_SHARPNESS_INK_G0 = 0x1,
	FTD330_SHARPNESS_INK_G1 = 0x2,
	FTD330_SHARPNESS_INK_G2 = 0x3,
	FTD330_SHARPNESS_INK_L0 = 0x4,
	FTD330_SHARPNESS_INK_L1 = 0x5,
	FTD330_SHARPNESS_INK_L2 = 0x6,
	FTD330_SHARPNESS_INK_ADAPT = 0x7,
	FTD330_SHARPNESS_INK_V0 = 0x8,
	FTD330_SHARPNESS_INK_V1 = 0x9,
	FTD330_SHARPNESS_INK_V2 = 0xA,
	FTD330_SHARPNESS_INK_LUMA = 0xB,
};

enum drm_ftd330_brightness_cal_mode {
	FTD330_BRIGHTNESS_CAL_MODE_WEIGHT = 0,
	FTD330_BRIGHTNESS_CAL_MODE_MAX = 1,
	FTD330_BRIGHTNESS_CAL_MODE_COUNT = 2,
};

/* Brightness Compensation ROI */
enum drm_ftd330_brightness_roi_type {
	FTD330_BRIGHTNESS_ROI0 = 0,
	FTD330_BRIGHTNESS_ROI1 = 1,
};

enum drm_ftd330_gem_query_type {
	FTD330_GEM_QUERY_HANDLE = 0,
};

enum drm_ftd330_mask_blend_type {
	FTD330_MASK_BLD_NORM,
	FTD330_MASK_BLD_INV,
	FTD330_MASK_BLD_COUNT,
};

enum drm_ftd330_rcd_bg_type {
	FTD330_RCD_BG_PNL,
	FTD330_RCD_BG_ROI,
	FTD330_RCD_BG_COUNT,
};

enum drm_ftd330_rcd_roi_type {
	FTD330_RCD_ROI_TOP,
	FTD330_RCD_ROI_BTM,
	FTD330_RCD_ROI_COUNT,
};

enum drm_ftd330_dec_pos {
	FTD330_DEC_BRT = 0,
	FTD330_DEC_BLUR,
	FTD330_DEC_RCD,
	FTD330_DEC_COUNT,
};

/*
histogram collects the image characteristics for SW to perform various related features.
There are 4 independent programmable histograms: FTD330_HIST_IDX_0 ~ FTD330_HIST_IDX_3
FTD330_HIST_IDX_RGB is for rgbHistogram
*/
enum drm_ftd330_hist_idx {
	FTD330_HIST_IDX_0,
	FTD330_HIST_IDX_1,
	FTD330_HIST_IDX_2,
	FTD330_HIST_IDX_3,
	FTD330_HIST_IDX_RGB,
	FTD330_HIST_IDX_COUNT,
};

enum drm_ftd330_hist_pos {
	FTD330_HIST_POS_SCALE,
	FTD330_HIST_POS_RCD,
	FTD330_HIST_POS_POST,
};

enum drm_ftd330_hist_bin_mode {
	FTD330_HIST_BIN_MAX,
	FTD330_HIST_BIN_WEIGHT,
	FTD330_HIST_BIN_RVALUE,
	FTD330_HIST_BIN_GVALUE,
	FTD330_HIST_BIN_BVALUE,
};

enum drm_ftd330_hist_out_mode {
	FTD330_HIST_BY_APB,
	FTD330_HIST_BY_WDMA,
};
enum dc_hw_sram_unit_size {
	SRAM_UNIT_SIZE_64KB,
	SRAM_UNIT_SIZE_32KB,
};
enum drm_ftd330_nr_mode {
	FTD330_NR_WEAK = 0,
	FTD330_NR_STRONG = 1,
};
enum drm_ftd330_ssr3_mode {
	FTD330_SSR3_STRONG_UPS = 0,
	FTD330_SSR3_NATURE = 1,
	FTD330_SSR3_TV1 = 2,
	FTD330_SSR3_TV2 = 3,
	FTD330_SSR3_TV3 = 4,
	FTD330_SSR3_DESK = 5,
	FTD330_SSR3_GAME = 6,
};
struct drm_ftd330_alpha_data_extend {
	bool enable;
	__u32 alpha_extend_value; /* alpha3[31:24], alpha2[23:16], alpha1[15:8], alpha0[7:0] */
};
struct drm_ftd330_data_extend {
	enum drm_ftd330_data_extend_mode data_extend_mode;
	struct drm_ftd330_alpha_data_extend alpha_data_extend;
};
struct drm_ftd330_splice_config {
	__u8 crtc_id;
	__u8 crtc_id_ex;
	__u8 crtc_ofifo_id;
	__u8 crtc_ofifo_id_ex;
};
struct drm_ftd330_splice_mode {
	bool splice0_enable;
	__u8 splice0_crtc_mask; /* crtc id mask */
	__u8 splice0_output_intf;
	bool splice1_enable;
	__u8 splice1_crtc_mask; /* crtc id mask */
	__u8 splice1_output_intf;
	__u16 src_panel_width0;
	__u16 src_panel_width1;
};
struct drm_ftd330_dp_sync {
	__u8 dp_sync_crtc_mask; /* crtc id mask */
};
enum drm_ftd330_free_sync_type {
	FTD330_FREE_SYNC_CONFIG,
	FTD330_FREE_SYNC_FINISH,
	FTD330_FREE_SYNC_CONFIG_FINISH,
};
struct drm_ftd330_free_sync_mode {
	__u16 free_sync_max_delay; /* max delay lines number */
	bool free_sync_finish;
};
struct drm_ftd330_free_sync {
	enum drm_ftd330_free_sync_type type;
	struct drm_ftd330_free_sync_mode mode;
};

/* Sharpness */
struct drm_ftd330_sharpness {
	bool enable;
	enum drm_ftd330_sharpness_ink_mode ink_mode;
};

struct drm_ftd330_sharpness_csc {
	int y2r_coef[FTD330_SHARPNESS_CSC_COEF_NUM];
	int r2y_coef[FTD330_SHARPNESS_CSC_COEF_NUM];
	int y2r_offset[FTD330_SHARPNESS_CSC_OFFSET_NUM];
	int r2y_offset[FTD330_SHARPNESS_CSC_OFFSET_NUM];
};

struct drm_ftd330_sharpness_luma_gain {
	int lut[FTD330_SHARPNESS_LUMA_GAIN_LUT_ENTRY_NUM];
};

struct drm_ftd330_sharpness_lpf {
	__u32 lpf0_coef[FTD330_SHARPNESS_LPF_COEF_NUM];
	__u32 lpf1_coef[FTD330_SHARPNESS_LPF_COEF_NUM];
	__u32 lpf2_coef[FTD330_SHARPNESS_LPF_COEF_NUM];
	__u32 lpf0_norm;
	__u32 lpf1_norm;
	__u32 lpf2_norm;
};

struct drm_ftd330_sharpness_lpf_noise {
	__u32 lut0[FTD330_SHARPNESS_LPF_NOISE_LUT_NUM];
	__u32 lut1[FTD330_SHARPNESS_LPF_NOISE_LUT_NUM];
	__u32 lut2[FTD330_SHARPNESS_LPF_NOISE_LUT_NUM];
	__u32 luma_strength0;
	__u32 luma_strength1;
	__u32 luma_strength2;
};

struct drm_ftd330_sharpness_lpf_curve {
	__u32 lut0[FTD330_SHARPNESS_LPF_CURVE_LUT_NUM];
	__u32 lut1[FTD330_SHARPNESS_LPF_CURVE_LUT_NUM];
	__u32 lut2[FTD330_SHARPNESS_LPF_CURVE_LUT_NUM];
	__u32 master_gain;
};

// TODO: Directly pass array to drm driver?
struct drm_ftd330_sharpness_color_adaptive_mode {
	bool enable;
	__u32 gain;
	__u32 theta_center;
	__u32 theta_range;
	__u32 theta_slope;
	__u32 radius_center;
	__u32 radius_range;
	__u32 radius_slope;
};

struct drm_ftd330_sharpness_color_adaptive {
	struct drm_ftd330_sharpness_color_adaptive_mode mode[FTD330_SHARPNESS_CA_MODE_NUM];
};

struct drm_ftd330_sharpness_color_boost {
	__u32 pos_gain;
	__u32 neg_gain;
	__u32 y_offset;
};

struct drm_ftd330_sharpness_soft_clip {
	__u32 pos_offset;
	__u32 neg_offset;
	__u32 pos_wet;
	__u32 neg_wet;
};

/* RGB table */
struct drm_ftd330_sharpness_dither {
	__u32 table_low[3];
	__u32 table_high[3];
};

enum drm_ftd330_ds_mode {
	FTD330_DS_DROP = 0,
	FTD330_DS_AVERAGE = 1,
	FTD330_DS_FILTER = 2,
};

struct drm_ftd330_rect {
	__u16 x;
	__u16 y;
	__u16 w;
	__u16 h;
};

struct drm_ftd330_color {
	__u32 a;
	__u32 r;
	__u32 g;
	__u32 b;
};

struct drm_ftd330_spliter {
	__u32 left_x;
	__u32 left_w;
	__u32 right_x;
	__u32 right_w;
	__u32 src_w;
};
struct drm_ftd330_panel_crop {
	struct drm_ftd330_rect crop_rect;
	__u16 panel_src_width;
	__u16 panel_src_height;
};
struct drm_ftd330_colorkey {
	__u32 colorkey;
	__u32 colorkey_high;
	bool transparency;
};

struct drm_ftd330_watermark {
	__u32 watermark;
	__u8 qos_low;
	__u8 qos_high;
};

struct drm_ftd330_dma {
	enum drm_ftd330_dma_mode mode;
	/* in_rect[0] is available under the DMA mode:
	 *	   FTD330_DMA_ONE_ROI: the ROI region rectangle.
	 *	   FTD330_DMA_TWO_ROI: the first ROI region rectangle.
	 *	   FTD330_DMA_SKIP_ROI: the skip ROI region rectangle.
	 *	   FTD330_DMA_EXT_LAYER_EX: the ROI region rectangle of first image.
	 * in_rect[1] is avilable under the DMA mode:
	 *	   FTD330_DMA_TWO_ROI: the seconf ROI region rectangle.
	 *	   FTD330_DMA_EXT_LAYER_EX: the ROI region rectangle of second image.
	 */
	struct drm_ftd330_rect in_rect[FTD330_MAX_ROI_CNT];
	/* out_rect[0] is available under the DMA mode:
	 *	   FTD330_DMA_ONE_ROI: specify the ROI out region.
	 *	   FTD330_DMA_TWO_ROI: specify the first ROI out region.
	 *	   FTD330_DMA_SKIP_ROI: specify skip ROI out region.
	 *	   FTD330_DMA_EXT_LAYER: specify the first image out region.
	 *	   FTD330_DMA_EXT_LAYER_EX: specify the ROI out region of first image.
	 * out_rect[1] is avilable under the DMA mode:
	 *	   FTD330_DMA_TWO_ROI: specify the seconf ROI out region.
	 *	   FTD330_DMA_EXT_LAYER: specify the second image out region.
	 *	   FTD330_DMA_EXT_LAYER_EX: specify the ROI out region of second image.
	 */
	struct drm_ftd330_rect out_rect[FTD330_MAX_ROI_CNT];
};

struct drm_ftd330_ex_layer {
	__u8 num;
	__u32 fb_id[FTD330_MAX_EXT_LAYER_CNT];
	int fd;
	struct drm_ftd330_rect out_rect[FTD330_MAX_EXT_LAYER_CNT];
};
struct drm_ftd330_line_padding {
	enum drm_ftd330_line_padding_mode mode;
	struct drm_ftd330_color color;
};

struct drm_ftd330_sbs {
	enum drm_ftd330_sbs_mode mode;
	/* available under the FTD330_SBS_SPLIT mode, side-by-side left width. */
	__u16 left_w;
	/* available under the FTD330_SBS_SPLIT mode, side-by-side right start X. */
	__u16 right_x;
	/* available under the FTD330_SBS_SPLIT mode, side-by-side right width. */
	__u16 right_w;
};

struct drm_ftd330_y2r_config {
	enum drm_ftd330_csc_mode mode;
	enum drm_ftd330_csc_gamut gamut;
	__s32 coef[FTD330_MAX_Y2R_COEF_NUM];
};

struct drm_ftd330_r2y_config {
	enum drm_ftd330_csc_mode mode;
	enum drm_ftd330_csc_gamut gamut;
	__s32 coef[FTD330_MAX_R2Y_COEF_NUM];
	/* For debug, the output bus format.
	 *     Usually the output bus format info from encoder.
	 *     In our driver, the default output bus format is MEDIA_BUS_FMT_RGB888_1X24
	 *     For the convernience of debugging, adding an output bus format setting here for debugging the
	 *     writeback data.
	 */
	__u32 output_bus_format;
};

struct drm_ftd330_scale_config {
	int src_w;
	int src_h;
	int dst_w;
	int dst_h;
	__u32 factor_x;
	__u32 factor_y;
	__u32 initial_offsetx;
	__u32 initial_offsety;
	__u32 coef_h[FTD330_SCALE_HORI_COEF_NUM];
	__u32 coef_v[FTD330_SCALE_VERT_COEF_NUM];
};

struct drm_ftd330_ds_config {
	enum drm_ftd330_ds_mode h_mode;
	enum drm_ftd330_ds_mode v_mode;
};

struct drm_ftd330_roi_lut_config {
	struct drm_ftd330_rect rect;
	struct drm_ftd330_color data[FTD330_MAX_ROI_3DLUT_SIZE];
};

struct drm_ftd330_gamut_map {
	bool enable;
	enum drm_ftd330_gamut_mode mode;
	__s32 coef[FTD330_MAX_GAMUT_COEF_NUM];
};

/*need to refine*/
struct drm_ftd330_data_block {
	__u32 size; /* total size of data block buffer */
	__u64 logical;
};

struct drm_ftd330_xstep_lut {
	bool enable;
	__u32 seg_cnt;
	__u32 seg_point[FTD330_MAX_LUT_SEG_CNT - 1];
	__u32 seg_step[FTD330_MAX_LUT_SEG_CNT];
	__u32 entry_cnt;
	__u32 data[FTD330_MAX_LUT_ENTRY_CNT];
};

struct drm_ftd330_gamma_lut {
	__u32 seg_cnt;
	__u32 seg_point[FTD330_MAX_LUT_SEG_CNT - 1];
	__u32 seg_step[FTD330_MAX_LUT_SEG_CNT];
	__u32 entry_cnt;
	struct drm_ftd330_color data[FTD330_MAX_GAMMA_ENTRY_CNT];
	struct drm_ftd330_color seg_cnt_sr;
	struct drm_ftd330_color seg_point_sr[FTD330_MAX_LUT_SEG_CNT - 1];
	struct drm_ftd330_color seg_step_sr[FTD330_MAX_LUT_SEG_CNT];
	struct drm_ftd330_color entry_cnt_sr;
};

struct drm_ftd330_lut {
	__u32 entry_cnt;
	__u32 seg_point[FTD330_MAX_LUT_ENTRY_CNT - 1];
	__u32 data[FTD330_MAX_LUT_ENTRY_CNT];
};

struct drm_ftd330_blend_alpha {
	/* src alpha pre process */
	enum drm_ftd330_alpha_mode sam;
	enum drm_ftd330_galpha_mode sgam;
	__u32 sga;
	__u32 saa;

	/* dst alpha pre process */
	enum drm_ftd330_alpha_mode dam;
	enum drm_ftd330_galpha_mode dgam;
	__u32 dga;
	__u32 daa;
};

struct drm_ftd330_1d_lut {
	bool enable;
	__u32 entry_cnt;
	__u32 data[FTD330_MAX_1D_LUT_ENTRY_CNT];
};

struct drm_ftd330_ltm_xgamma {
	bool enable;
	__u32 coef[FTD330_LTM_XGAMMA_COEF_NUM];
};

struct drm_ftd330_ltm_luma {
	bool enable;
	enum drm_ftd330_ltm_luma_mode mode;
	__u16 coef[FTD330_LTM_LUMA_COEF_NUM];
};

struct drm_ftd330_ltm_freq_decomp {
	bool decomp_enable;
	__u16 coef[FTD330_LTM_FREQ_COEF_NUM];
	__u32 norm;
};

struct drm_ftd330_ltm_grid {
	bool enable;
	__u16 width;
	__u16 height;
	__u16 depth;
};

struct drm_ftd330_ltm_af_filter {
	bool enable;
	__u16 weight;
	__u32 slope[FTD330_LTM_AFFINE_LUT_NUM];
	__u32 bias[FTD330_LTM_AFFINE_LUT_NUM];
};

struct drm_ftd330_ltm_af_slice {
	bool enable;
	__u16 start_pos[FTD330_LTM_AFFINE_SLICE_NUM];
	__u16 scale[FTD330_LTM_AFFINE_SLICE_NUM];
	__u16 scale_half[FTD330_LTM_AFFINE_SLICE_NUM];
};

struct drm_ftd330_ltm_af_trans {
	bool enable;
	__u8 slope_bit;
	__u8 bias_bit;
	__u16 scale[FTD330_LTM_AFFINE_OUT_SCALE_SIZE];
};

struct drm_ftd330_ltm_tone_adj {
	bool enable;
	bool luma_from;
	__u32 entry_cnt;
	__u32 data[FTD330_MAX_1D_LUT_ENTRY_CNT];
};

struct drm_ftd330_ltm_color {
	bool enable;
	bool satu_ctrl;
	__u16 luma_thresh;
	__u16 gain[FTD330_LTM_ALPHA_GAIN_SIZE];
	__u16 luma[FTD330_LTM_ALPHA_LUMA_SIZE];
	__u16 satu[FTD330_LTM_SATU_CTRL_SIZE];
};

struct drm_ftd330_ltm_dither {
	bool dither_enable;
	__u32 table_low[FTD330_LTM_DITHER_COEF_NUM];
	__u32 table_high[FTD330_LTM_DITHER_COEF_NUM];
};

struct drm_ftd330_ltm_luma_ave {
	bool enable;
	__u16 margin_x;
	__u16 margin_y;
	__u16 pixel_norm;
	__u16 ave;
};

struct drm_ftd330_ltm_cd_set {
	bool enable;
	bool overlap;
	__u32 min_wgt;
	__u32 filt_norm;
	__u32 coef[FTD330_LTM_CD_COEF_NUM];
	__u32 thresh[FTD330_LTM_CD_THRESH_NUM];
	__u32 slope[FTD330_LTM_CD_SLOPE_NUM];
};

struct drm_ftd330_ltm_cd_get {
	__u32 result[FTD330_LTM_CD_RESULT_NUM];
};

struct drm_ftd330_ltm_hist_set {
	bool enable;
	bool overlap;
	__u32 grid_depth;
	__u32 start_pos[FTD330_LTM_HIST_POS_NUM];
	__u32 grid_scale[FTD330_LTM_HIST_SCALE_NUM];
};

struct drm_ftd330_ltm_hist_get {
	bool enable;
	__u32 fd;
	__u32 hist_bo_handle;
	__u32 result[FTD330_LTM_HIST_RESULT_NUM];
};

struct drm_ftd330_ltm_ds {
	bool enable;
	__u32 h_norm;
	__u32 v_norm;
	__u32 crop_l;
	__u32 crop_r;
	__u32 crop_t;
	__u32 crop_b;
	struct drm_ftd330_rect output;
};
struct drm_ftd330_ltm {
	struct drm_ftd330_ltm_xgamma ltm_degamma;
	struct drm_ftd330_ltm_xgamma ltm_gamma;
	struct drm_ftd330_ltm_luma ltm_luma;
	struct drm_ftd330_ltm_freq_decomp freq_decomp;
	struct drm_ftd330_1d_lut luma_adj;
	struct drm_ftd330_ltm_grid grid_size;
	struct drm_ftd330_ltm_af_filter af_filter;
	struct drm_ftd330_ltm_af_slice af_slice;
	struct drm_ftd330_ltm_af_trans af_trans;
	struct drm_ftd330_ltm_tone_adj tone_adj;
	struct drm_ftd330_ltm_color ltm_color;
	struct drm_ftd330_ltm_dither ltm_dither;
	struct drm_ftd330_ltm_luma_ave ltm_luma_set;
	struct drm_ftd330_ltm_cd_set ltm_cd_set;
	struct drm_ftd330_ltm_hist_set ltm_hist_set;
	struct drm_ftd330_ltm_ds ltm_ds;
	struct drm_ftd330_ltm_hist_get ltm_hist_get;
};
struct drm_ftd330_gtm {
	struct drm_ftd330_ltm_xgamma gtm_degamma;
	struct drm_ftd330_ltm_xgamma gtm_gamma;
	struct drm_ftd330_ltm_luma gtm_luma;
	struct drm_ftd330_ltm_tone_adj tone_adj;
	struct drm_ftd330_ltm_color gtm_color;
	struct drm_ftd330_ltm_dither gtm_dither;
	struct drm_ftd330_ltm_luma_ave gtm_luma_set;
	struct drm_ftd330_ltm_ds gtm_ds;
};

struct drm_ftd330_blend {
	enum drm_ftd330_blend_mode color_mode;
	enum drm_ftd330_blend_mode alpha_mode;
};

struct drm_ftd330_tone_map_y {
	enum drm_ftd330_calcu_mode y_mode;
	__u16 coef0;
	__u16 coef1;
	__u16 coef2;
	__u16 weight;
};

struct drm_ftd330_tone_map {
	bool enable;
	struct drm_ftd330_tone_map_y pseudo_y;
	struct drm_ftd330_lut lut;
};

struct drm_ftd330_hdr_algo_programmable {
	__u8 mode;
	char *programmable_csv;
};
struct drm_ftd330_hdr_algo_tone_map {
	__u8 mode;
	__u8 y2r_gamut;
	char *programmable_csv;
	__u16 max_cll;
	__u16 max_dll;
	double ks;
	double kf;
	double bezier_p[17];
	int len_p;
};
struct drm_ftd330_hdr_algo_config {
	bool hdr_enable;
	bool de_multiply;
	bool eotf_enable;
	bool gamut_enable;
	bool tonemapping_enable;
	bool oetf_enable;
	struct drm_ftd330_hdr_algo_programmable eotf;
	struct drm_ftd330_hdr_algo_programmable gamut;
	struct drm_ftd330_hdr_algo_tone_map tonemap;
	struct drm_ftd330_hdr_algo_programmable oetf;
};
struct drm_ftd330_data_trunc {
	enum drm_ftd330_data_trunc_mode gamma_data_trunc;
	enum drm_ftd330_data_trunc_mode panel_data_trunc;
	enum drm_ftd330_data_trunc_mode blend_data_trunc;
	enum drm_ftd330_data_trunc_mode wb_data_trunc;
};

struct drm_ftd330_lut_config_ex {
	bool enable[3];
	struct drm_ftd330_rect rect[2];
	struct drm_ftd330_data_block data[3];
};

struct drm_ftd330_dither {
	enum drm_ftd330_dth_frm_idx index_type;
	__u8 sw_index;
	__u32 table_low[3];
	__u32 table_high[3];
	enum drm_ftd330_dth_frm_mode frm_mode;
};

struct drm_ftd330_random_dither_seed {
	bool hash_seed_x_enable;
	bool hash_seed_y_enable;
	bool permut_seed1_enable;
	bool permut_seed2_enable;
	__u32 hash_seed_x[FTD330_RANDOM_DITHER_SEED_NUM];
	__u32 hash_seed_y[FTD330_RANDOM_DITHER_SEED_NUM];
	__u32 permut_seed1[FTD330_RANDOM_DITHER_SEED_NUM];
	__u32 permut_seed2[FTD330_RANDOM_DITHER_SEED_NUM];
};

struct drm_ftd330_blender_dither {
	enum drm_ftd330_dth_frm_idx index_type;
	__u8 sw_index;
	__u8 noise;
	__u16 start_x;
	__u16 start_y;
	__u16 mask;
	struct drm_ftd330_random_dither_seed seed;
};

struct drm_ftd330_llv_dither {
	enum drm_ftd330_dth_frm_idx index_type;
	__u8 sw_index;
	__u16 start_x;
	__u16 start_y;
	__u16 mask;
	__u16 threshold;
	__u16 linear_threshold;
	struct drm_ftd330_random_dither_seed seed;
};

struct drm_ftd330_wb_dither {
	__u32 table_low[3];
	__u32 table_high[3];
	enum drm_ftd330_dth_frm_idx index_type;
	__u8 sw_index;
	enum drm_ftd330_dth_frm_mode frm_mode;
};

struct drm_ftd330_wb_spliter {
	struct drm_ftd330_rect split_rect0;
	struct drm_ftd330_rect split_rect1;
};

struct drm_ftd330_ccm {
	__s32 coef[9];
	__s32 offset[3];
};

struct drm_ftd330_blur {
	struct drm_ftd330_rect roi;
	__u8 coef[3][FTD330_BLUR_COEF_NUM];
	__u32 norm[3];
	__u8 coef_num;
};

struct drm_ftd330_rcd_enable {
	bool enable;
	enum drm_ftd330_mask_blend_type type;
};

struct drm_ftd330_rcd_bg {
	__u32 pnl_color;
	bool roi_enable;
	__u32 roi_color;
	struct drm_ftd330_rect roi_rect;
	bool bg_dirty[2];
};

struct drm_ftd330_rcd_roi {
	bool top_enable;
	bool btm_enable;
	struct drm_ftd330_rect top_roi;
	struct drm_ftd330_rect btm_roi;
};

struct drm_ftd330_dec_enable {
	bool lossy;
	enum drm_ftd330_dec_pos pos;
};
struct drm_ftd330_wb_frm_done {
	__u8 wb_id;
	bool wb_frm_done;
};

struct drm_ftd330_reset {
	enum drm_ftd330_reset_mode mode;
};

struct drm_ftd330_hist {
	enum drm_ftd330_hist_idx idx;
	bool			hist_enable;
	enum drm_ftd330_hist_pos pos;
	enum drm_ftd330_hist_bin_mode bin_mode;
	__u32 coef[3];
};

struct drm_ftd330_hist_roi {
	enum drm_ftd330_hist_idx idx;
	struct drm_ftd330_rect rect;
};

struct drm_ftd330_hist_block {
	enum drm_ftd330_hist_idx idx;
	struct drm_ftd330_rect rect;
};

struct drm_ftd330_hist_prot {
	enum drm_ftd330_hist_idx idx;
	bool			prot_enable;
};

struct drm_ftd330_hist_info {
	enum drm_ftd330_hist_idx idx;
	enum drm_ftd330_hist_out_mode out_mode;
	struct drm_ftd330_data_block *buffer;
	__u32 fd;
	__u32 hist_bo_handle;
};

struct drm_ftd330_hist_get {
	enum drm_ftd330_hist_idx idx;
	__u32 result[FTD330_HIST_RESULT_BIN_CNT];
};

struct drm_ftd330_rgb_hist_get {
	__u32 result[FTD330_HIST_RESULT_BIN_CNT * 3];
};

struct drm_ftd330_get_hist_info {
	__u8 crtc_id;
	enum drm_ftd330_hist_idx idx;
	union _hist_get_u {
		struct drm_ftd330_hist_get hist;
		struct drm_ftd330_rgb_hist_get rgb_hist;
	} u;
};

struct drm_ftd330_pvric_offset {
	__u32 format;
	__u32 handles[3];
	__u32 header_size[3];

	__u64 offsets[3];
};

struct drm_ftd330_pvric_clear {
	__u64 color[3];
};

struct drm_ftd330_pvric_const {
	struct drm_ftd330_color color[2];
};

struct drm_ftd330_brightness {
	enum drm_ftd330_brightness_cal_mode mode;
	__u16 target;
	__u16 threshold;
	__u16 luma_coef[3];
};

struct drm_ftd330_brightness_roi {
	bool roi0_enable;
	bool roi1_enable;
	struct drm_ftd330_rect roi0;
	struct drm_ftd330_rect roi1;
};

struct drm_ftd330_decompress {
	bool lossy;
	__u64 physical;
	__u32 format;
	__u32 tile_type;
	__u64 clear_color;
};

struct drm_ftd330_gem_query_info {
	enum drm_ftd330_gem_query_type type;
	__u32 handle;
	__u64 data;
};

struct drm_ftd330_query_feature_cap {
	enum drm_ftd330_feature_cap_type type;
	__u32 cap;
};

struct drm_ftd330_dsc {
	/* Operating mode of the encoder */
	__u8 slices_per_line;
	__u8 ss_num;
	__u16 slice_height;
	__u16 picture_width;
	__u16 picture_height;

	/* For usage model */
	bool split_panel_enable;
	bool multiplex_mode_enable;
	int multiplex_out_sel;
	bool de_raster_enable;
	bool multiplex_eoc_enable;
	bool video_mode;
};

struct dsc_hw_config {
	__u8 dsc_version_major;
	__u8 dsc_version_minor;
	bool native_420_enable;
	bool native_422_enable;
	__u8 nb_hs_enc;
	__u8 nb_ss_enc;
	bool derasterization_buffer_enable;
	int max_container_pixels_per_line;
	int max_container_pixels_hs_line;
	int max_lines;
	int max_bpc;
	int output_data_width;
	int ob_addr_width;
};
struct drm_ftd330_vdc {
	/* Operating mode of the encoder */
	__u8 slices_per_line;
	__u8 ss_num;
	__u16 slice_height;

	/* For usage model */
	bool split_panel_enable;
	bool multiplex_mode_enable;
	int multiplex_out_sel;
	bool multiplex_eoc_enable;
	bool hs_split_input_enable;
	bool video_mode;
};

struct drm_ftd330_hdr {
	bool demultiply;
	struct drm_ftd330_xstep_lut eotf_config;
	struct drm_ftd330_gamut_map gamut_map;
	struct drm_ftd330_tone_map tone_map;
	struct drm_ftd330_xstep_lut oetf_config;
};
struct drm_ftd330_nr_config {
	__u32 coef[FTD330_NR_COEF_NUM];
};
struct drm_ftd330_ssr3_config {
	__u32 DepHWid;
	__u32 DepVWid;
	int HWidRatio;
	int VWidRatio;
	int HRatioSft;
	int VRatioSft;
	__u32 coef[FTD330_SSR3_COEF_NUM];
};
struct drm_ftd330_sr2000 {
	bool r2y_enable;
	bool scale_enable;
	bool nr_enable;
	bool ssr3_enable;
	bool y2r_enable;
	struct drm_ftd330_r2y_config r2y;
	struct drm_ftd330_scale_config scale;
	struct drm_ftd330_nr_config nr;
	struct drm_ftd330_ssr3_config ssr3;
	struct drm_ftd330_y2r_config y2r;
};
struct dc_dec400_fc{
	bool fc_enable;
	__u32 fc_size;
	__u32 fc_rgby_value;
	__u32 fc_uv_value;
};
#define DRM_FTD330_GET_FBC_OFFSET 0x00
#define DRM_FTD330_SW_RESET 0x01
#define DRM_FTD330_GEM_QUERY 0x04
#define DRM_FTD330_GET_FEATURE_CAP 0x05
#define DRM_FTD330_GET_HIST_INFO 0x06
#define DRM_FTD330_GET_WB_FRM_DONE 0x07

#define DRM_IOCTL_FTD330_GET_FBC_OFFSET \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_GET_FBC_OFFSET, struct drm_ftd330_pvric_offset)
#define DRM_IOCTL_FTD330_SW_RESET DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_SW_RESET, struct drm_ftd330_reset)
#define DRM_IOCTL_FTD330_GEM_QUERY \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_GEM_QUERY, struct drm_ftd330_gem_query_info)
#define DRM_IOCTL_FTD330_GET_FEATURE_CAP \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_GET_FEATURE_CAP, struct drm_ftd330_query_feature_cap)
#define DRM_IOCTL_FTD330_GET_HIST_INFO \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_GET_HIST_INFO, struct drm_ftd330_get_hist_info)
#define DRM_IOCTL_FTD330_GET_WB_FRM_DONE \
	DRM_IOWR(DRM_COMMAND_BASE + DRM_FTD330_GET_WB_FRM_DONE, struct drm_ftd330_wb_frm_done)

#endif /* __FTD330_DRM_H__ */
