/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_WRITEBACK_H_
#define __FTD330_WRITEBACK_H_

#include <drm/drm_writeback.h>
#include "drm/ftd330_drm.h"
#include "ftd330_type.h"
#include "ftd330_dc_property.h"

#define MAX_WB_NUM_PLANES 3 /* colour format plane */

struct ftd330_writeback_connector;

struct ftd330_writeback_funcs {
	void (*config)(struct ftd330_writeback_connector *wb_connector, struct drm_framebuffer *fb);
	void (*disable)(struct ftd330_writeback_connector *wb_connector, struct drm_crtc *crtc);
	int (*check)(struct ftd330_writeback_connector *wb_connector, struct drm_framebuffer *fb,
		     struct drm_display_mode *mode, struct drm_connector_state *state);
#ifdef CONFIG_DEBUG_FS
	void (*set_qos)(struct device *dev, struct ftd330_writeback_connector *ftd330_wb,
			const char __user *ubuf, size_t len);
	int (*show_qos)(struct seq_file *s);
#endif
};

struct ftd330_writeback_connector_state {
	struct drm_connector_state base;

	u32 wb_point;
	struct dc_hw_wb_qos qos;

	struct ftd330_drm_property_state drm_states[FTD330_DC_MAX_PROPERTY_NUM];
};

struct ftd330_writeback_connector {
	struct drm_writeback_connector base;
	u8 id;
	struct device *dev;
	dma_addr_t dma_addr[MAX_WB_NUM_PLANES];
	unsigned int pitch[MAX_WB_NUM_PLANES];

	struct drm_property *point_prop;

	struct ftd330_drm_property_group properties;

	const struct ftd330_writeback_funcs *funcs;
	u8 armed;
};

struct ftd330_writeback_connector *ftd330_writeback_create(const struct dc_hw_wb *hw_wb,
						   struct drm_device *drm_dev,
						   const struct ftd330_wb_info *info,
						   unsigned int possible_crtcs);

void ftd330_writeback_handle_vblank(struct ftd330_writeback_connector *ftd330_wb_connector);

struct drm_writeback_connector *find_wb_connector(struct drm_crtc *crtc);

static inline struct ftd330_writeback_connector *
to_ftd330_writeback_connector(struct drm_writeback_connector *wb_connector)
{
	return container_of(wb_connector, struct ftd330_writeback_connector, base);
}

static inline struct ftd330_writeback_connector_state *
to_ftd330_writeback_connector_state(struct drm_connector_state *state)
{
	return container_of(state, struct ftd330_writeback_connector_state, base);
}

#endif
