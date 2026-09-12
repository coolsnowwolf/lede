/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */
#ifndef __FTD330_DC_INFO_H__
#define __FTD330_DC_INFO_H__

#include <linux/kernel.h>
#include <linux/types.h>
#include "ftd330_type.h"
#include "ftd330_simple_enc.h"

#include <drm/drm_blend.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_atomic_helper.h>
#include <drm/drm_connector.h>
#ifndef DRM_PLANE_HELPER_NO_SCALING
#define DRM_PLANE_HELPER_NO_SCALING DRM_PLANE_NO_SCALING
#endif
#ifndef DRM_COLOR_FORMAT_YCRCB444
#define DRM_COLOR_FORMAT_YCRCB444 DRM_COLOR_FORMAT_YCBCR444
#define DRM_COLOR_FORMAT_YCRCB422 DRM_COLOR_FORMAT_YCBCR422
#define DRM_COLOR_FORMAT_YCRCB420 DRM_COLOR_FORMAT_YCBCR420
#endif
#ifndef DRM_MODE_BLEND_PREMULTI
#define DRM_MODE_BLEND_PREMULTI 0
#endif
#ifndef DRM_MODE_BLEND_COVERAGE
#define DRM_MODE_BLEND_COVERAGE 1
#endif
#ifndef DRM_MODE_BLEND_PIXEL_NONE
#define DRM_MODE_BLEND_PIXEL_NONE 2
#endif
enum dc_chip_rev {
 /* For HW_REV_5720;
  * HW_REV_5721_311
  */
	DC_REV_0,
	DC_REV_1, /* For HW_REV_5721_30b */
	DC_REV_2, /* For HW_REV_5721_310 */
};

enum dc_id {
	DC_0,
	DC_1,
	DC_MAX,
};

enum dc_hw_plane_id {
	PRIMARY_PLANE_0,
	OVERLAY_PLANE_0,
	OVERLAY_PLANE_1,
	PRIMARY_PLANE_1,
	OVERLAY_PLANE_2,
	OVERLAY_PLANE_3,
	PRIMARY_PLANE_2,
	OVERLAY_PLANE_4,
	OVERLAY_PLANE_5,
	CURSOR_PLANE_0,
	CURSOR_PLANE_1,
	CURSOR_PLANE_2,
	PLANE_NUM,
};

enum dc_hw_display_id {
	DISPLAY_0,
	DISPLAY_1,
	DISPLAY_2,
	DISPLAY_NUM,
};

enum dc_hw_wb_id {
	WB_0,
	WB_1,
	WB_2,
	WB_NUM,
};

#define GAMMA_SIZE 256
#define GAMMA_EX_SIZE 300
#define DEGAMMA_SIZE 260


/*
 * TODO: Different chip have differnert DC_XXX_NUM.
 *       Need find way to separate.
 *       For now, use the greatest common divisor of all.
 */
#define DC_LAYER_NUM 9
#define DC_CURSOR_NUM 3
#define DC_DISPLAY_NUM 3
#define DC_WB_NUM 3
#define DC_OUTPUT_NUM 6

struct ftd330_dc_info *ftd330_dc_get_chip_info(int total_pipes);
const struct ftd330_output_info *ftd330_dc_get_output_info(int total_pipes);

#endif
