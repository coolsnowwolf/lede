/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_H__
#define __FTD330_DC_H__

#include <linux/mm_types.h>
#include <linux/version.h>
#include <linux/of.h>
#include <linux/media-bus-format.h>

#include <drm/drm_modes.h>
#if KERNEL_VERSION(5, 5, 0) > LINUX_VERSION_CODE
#include <drm/drmP.h>
#endif

#include "ftd330_crtc.h"
#include "ftd330_dc_dec.h"
#include "ftd330_dc_hw.h"
#include "ftd330_plane.h"
#include "ftd330_writeback.h"
#ifdef CONFIG_PCI
#include "ftd330_virtual.h"
#endif

#ifdef CONFIG_PHYTIUM_MMU
#include "ftd330_dc_mmu.h"
#endif
#include "ftd330_drv.h"

struct ftd330_dc_plane {
	enum dc_hw_plane_id id;
};

struct ftd330_dc {
	struct ftd330_crtc *crtc[DC_DISPLAY_NUM];
	struct dc_hw hw;
#ifdef CONFIG_PHYTIUM_DEC
	struct dc_dec400l dec400l;
#endif

	// struct clk *core_clk;
	// struct clk *pix_clk;
	// struct clk *axi_clk;
	// unsigned int pix_clk_rate; /* in KHz */


	bool first_frame;

	struct ftd330_dc_plane planes[PLANE_NUM];

	struct simple_encoder *encoder[DC_OUTPUT_NUM];

#ifdef CONFIG_PHYTIUM_WRITEBACK
	struct ftd330_writeback_connector *writeback[DC_WB_NUM];
#endif

#ifdef CONFIG_PHYTIUM_VIRTUAL_DISPLAY
	struct ftd330_virtual_display *vd[DC_OUTPUT_NUM];
#endif
};

extern struct platform_driver dc_platform_driver;
extern struct platform_driver dc_be_platform_driver;
extern struct platform_driver dc_fe0_platform_driver;
extern struct platform_driver dc_fe1_platform_driver;
extern struct platform_driver dc_wb_platform_driver;

int ftd330_get_wb_frm_done_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
int ftd330_sw_reset_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
int ftd330_get_feature_cap_ioctl(struct drm_device *dev, void *data,  struct drm_file *file_priv);
int ftd330_get_hist_info_ioctl(struct drm_device *dev, void *data, struct drm_file *file_priv);
void phytium_drm_device_deinit(struct drm_device *drm_dev);
int phytium_drm_device_init(struct drm_device *drm_dev);

#ifdef CONFIG_PM_SLEEP
int ftd330_dc_suspend(struct device *dev);
int ftd330_dc_resume(struct device *dev);
#endif

extern int phytium_dc_num;
extern unsigned long start_address;
#ifdef CONFIG_PHYTIUM_POWER_OPERATION
extern void phytium_display_power_request(struct ftd330_drm_private *priv, bool enable, int display_id);
#endif
extern void phytium_dc_scaling_config(struct dc_hw *hw, u8 plane_id, struct dc_hw_position *pos);
#endif /* __FTD330_DC_H__ */
