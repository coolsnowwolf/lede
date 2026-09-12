/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_PLANE_H__
#define __FTD330_PLANE_H__

#include <drm/drm_fourcc.h>
#include <drm/drm_plane_helper.h>
#include <drm/drm_framebuffer.h>
#include <drm/ftd330_drm.h>

#include "ftd330_fb.h"
#include "ftd330_type.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_drm_property.h"

#define MAX_NUM_PLANES 3 /* colour format plane */
#define PHYTIUM_TILE_STATUS_BUFFER_ALIGNMENT 64

struct ftd330_plane;

struct ftd330_plane_funcs {
	void (*update)(struct device *dev, struct ftd330_plane *plane);
#ifdef CONFIG_DEBUG_FS
	void (*set_pattern)(struct device *dev, struct ftd330_plane *plane);
	void (*set_crc)(struct device *dev, struct ftd330_plane *plane);
	void (*set_qos)(struct device *dev, struct ftd330_plane *plane, const char __user *ubuf,
			size_t len);
	int (*show_qos)(struct seq_file *s);
#endif /* CONFIG_DEBUG_FS */
	void (*disable)(struct device *dev, struct ftd330_plane *plane,
			struct drm_plane_state *old_state);
	int (*check)(struct device *dev, struct ftd330_plane *plane, struct drm_plane_state *state);
	bool (*format_mod_support)(struct ftd330_plane *plane, u32 format, u64 modifier);
};

struct ftd330_plane_status {
	u32 tile_mode;
	struct drm_rect src;
	struct drm_rect dest;
#if KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE
	struct drm_format_name_buf format_name;
#endif
};

struct ftd330_plane_pattern {
	bool enable;
	u8 mode;
	u64 color;
	struct drm_ftd330_rect rect;
};

struct ftd330_plane_crc {
	bool enable;
	u8 pos;
	struct drm_ftd330_color seed;
	struct drm_ftd330_color result;
	u8 init_mode;
	__u32 init_value;
	__u32 xor_value;
	__u32 golden[3];
};

struct ftd330_plane_sram_pool {
	u32 sp_handle;
	u32 sp_size;
	u8 sp_unit_size;
	u32 scl_sp_handle;
	u32 scl_sp_size;
};


struct ftd330_plane_state {
	struct drm_plane_state base;
	struct ftd330_plane_status status; /* for debugfs */
	struct ftd330_plane_pattern pattern; /* for pattern debugfs */
	struct ftd330_plane_crc crc; /* for crc debugfs */
#ifdef CONFIG_DEBUG_FS
	struct dc_hw_plane_qos qos;
#endif
	struct drm_property_blob *watermark;
	struct drm_property_blob *y2r_coef;
	struct drm_property_blob *lut_3d;
	struct drm_property_blob *pvric_clear;
	struct drm_property_blob *pvric_const;
	struct drm_framebuffer *fb_ext;

	bool lut_3d_changed;
	bool pvric_color_changed;
	struct ftd330_drm_property_state drm_states[FTD330_DC_MAX_PROPERTY_NUM];
};

struct ftd330_plane {
	struct drm_plane base;
	u8 id;
	dma_addr_t dma_addr[MAX_NUM_PLANES];
	dma_addr_t ts_addr[MAX_NUM_PLANES];
	void *ts_dma_buf[MAX_NUM_PLANES];

#ifdef CONFIG_DEBUG_FS
	/**
	 * @debugfs_entry:
	 *
	 * Debugfs directory for this plane.
	 */
	struct dentry *debugfs_entry;
#endif

	struct drm_property *watermark_prop;
	struct drm_property *y2r_prop;
	struct drm_property *lut_3d_prop;
	struct drm_property *pvric_clear_prop;
	struct drm_property *pvric_const_prop;
	struct drm_property *ext_layer_fb;

	struct ftd330_drm_property_group properties;

	struct ftd330_plane_sram_pool sram;

	const struct ftd330_plane_funcs *funcs;
};

struct ftd330_ts_metadata {
	u32 magic;
	u32 dmabuf_size;
	u32 time_stamp;
	u32 compressed;
	u32 image_format;
	struct {
		u32 offset;
		u32 stride;
		u32 width;
		u32 height;
		u32 compression_format;
		u32 tile_mode;
		int ts_fd;
		void *ts_dma_buf;
		u32 ts_offset;
		u32 fc_enabled;
		u32 fc_value_lower;
		u32 fc_value_upper;
		u32 header_size;
	} plane[3];
	u32 reserved[8];
};
void ftd330_plane_destory(struct drm_plane *plane);

void ftd330_plane_get_dec_tile_status(struct drm_device *dev, struct ftd330_gem_object *ftd330_gem,
				  u8 plane_id, dma_addr_t *ts_addr, void **ts_dma_buf);
struct ftd330_plane *ftd330_plane_create(const struct dc_hw_plane *hw_plane, struct drm_device *drm_dev,
				 const struct ftd330_dc_info *info, u8 index,
				 unsigned int possible_crtcs,
				 const struct ftd330_plane_funcs *dc_plane_funcs);

static inline struct ftd330_plane *to_ftd330_plane(struct drm_plane *plane)
{
	return container_of(plane, struct ftd330_plane, base);
}

static inline struct ftd330_plane_state *to_ftd330_plane_state(struct drm_plane_state *state)
{
	return container_of(state, struct ftd330_plane_state, base);
}
#endif /* __FTD330_PLANE_H__ */
