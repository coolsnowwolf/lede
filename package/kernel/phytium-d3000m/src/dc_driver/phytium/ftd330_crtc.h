/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_CRTC_H__
#define __FTD330_CRTC_H__

#include <drm/drm_crtc.h>
#include <drm/drm_crtc_helper.h>
#include <drm/ftd330_drm.h>

#include "ftd330_type.h"
#include "ftd330_dc_hw.h"
#include "ftd330_dc_property.h"
#include "ftd330_dc_drm_property.h"

struct ftd330_crtc;

struct ftd330_crtc_funcs {
	void (*enable)(struct device *dev, struct drm_crtc *crtc);
	void (*disable)(struct device *dev, struct drm_crtc *crtc);
	bool (*mode_fixup)(struct device *dev, const struct drm_display_mode *mode,
			   struct drm_display_mode *adjusted_mode);
#ifdef CONFIG_DEBUG_FS
	int (*show_pattern_config)(struct seq_file *s);
	void (*set_pattern)(struct drm_crtc *crtc, const char __user *ubuf, size_t len);
	void (*set_crc)(struct device *dev, struct drm_crtc *crtc, const char __user *ubuf,
			size_t len);
	int (*show_crc)(struct seq_file *s);
	void (*set_qos)(struct device *dev, struct drm_crtc *crtc, const char __user *ubuf,
			size_t len);
	int (*show_qos)(struct seq_file *s);
#endif /* CONFIG_DEBUG_FS */
	void (*config)(struct device *dev, struct drm_crtc *crtc);
	void (*enable_vblank)(struct ftd330_crtc *crtc, bool enable);
	u32 (*get_vblank_count)(struct ftd330_crtc *crtc);
	void (*commit)(struct device *dev, struct drm_crtc *crtc);
	int (*check)(struct device *dev, struct drm_crtc *crtc, struct drm_crtc_state *state);
	int (*get_crtc_scanout_position)(struct device *dev, struct drm_crtc *crtc, u32 *position);
};

struct ftd330_crtc_pattern {
	bool enable;
	u8 pos;
	u8 mode;
	u64 color;
	struct drm_ftd330_rect rect;
};

struct ftd330_crtc_crc {
	bool enable;
	u8 pos;
	struct drm_ftd330_color seed;
	struct drm_ftd330_color result;
};

struct ftd330_crtc_state {
	struct drm_crtc_state base;
#ifdef CONFIG_DEBUG_FS
	struct dc_hw_pattern pattern; /* for pattern debugfs */
	struct dc_hw_disp_qos qos; /* for CRTC qos debugfs */
#endif
	struct drm_framebuffer *blur_mask;
	struct drm_framebuffer *brightness_mask;
	struct drm_framebuffer *rcd_mask;
	struct dc_hw_disp_crc crc; /* for crc debugfs */

	u32 sync_mode;
	u32 output_fmt;
	u8 output_id;
	u32 output_mode;
	u8 encoder_type;
	u8 mmu_prefetch;
	u8 bpp;

	struct drm_property_blob *prior_gamma;
	struct drm_property_blob *roi0_gamma;
	struct drm_property_blob *roi1_gamma;
	struct drm_property_blob *ltm_luma_get;
	struct drm_property_blob *ltm_cd_get;
	struct drm_property_blob *ltm_hist_get;
	struct drm_property_blob *hist_get;
	struct drm_property_blob *rgb_hist_get;

	enum drm_ftd330_data_extend_mode data_ext_mode;

	bool sync_enable;
	bool underflow;
	bool out_dp;
	bool prior_gamma_changed;
	bool roi0_gamma_changed;
	bool roi1_gamma_changed;
	bool ltm_luma_get_changed;
	bool ltm_cd_get_changed;
	bool ltm_hist_get_changed;
	bool hist_get_changed;
	bool rgb_hist_get_changed;

	struct ftd330_drm_property_state drm_states[FTD330_DC_MAX_PROPERTY_NUM];
};

struct ftd330_pageflip {
        u32 fps;
        u32 count;
        ktime_t time;
};

struct ftd330_crtc {
	struct drm_crtc base;
	u8 id;
	struct device *dev;
	spinlock_t slock;
	unsigned int max_bpc;
	unsigned int color_formats; /* supported color format */
	/* last state event */
	struct drm_pending_vblank_event *event;
	bool commit_hw_done;
	/* frame completion */
	struct completion frame_completion;

	
	unsigned int scale_enable;
	struct drm_ftd330_scale_config scale;

	ktime_t t_vblank;
	ktime_t t_cut_vblank;
	struct drm_property *sync_mode;
	struct drm_property *mmu_prefetch;
	struct drm_property *panel_sync;
	struct drm_property *data_ext_prop;
	struct drm_property *prior_gamma_prop;
	struct drm_property *roi0_gamma_prop;
	struct drm_property *roi1_gamma_prop;
	struct drm_property *ltm_luma_get_prop;
	struct drm_property *ltm_cd_get_prop;
	struct drm_property *ltm_hist_get_prop;
	struct drm_property *hist_get_prop;
	struct drm_property *rgb_hist_get_prop;
	struct drm_property *rcd_mask_fb;
	struct drm_property *blur_mask_fb;
	struct drm_property *brightness_mask_fb;

	struct ftd330_drm_property_group properties;

	const struct ftd330_crtc_funcs *funcs;

	struct ftd330_pageflip pageflip;
};
bool ftd330_display_get_crtc_scanoutpos(struct drm_device *dev, unsigned int crtc_id,
				    bool in_vblank_irq, int *vpos, int *hpos, ktime_t *stime,
				    ktime_t *etime, const struct drm_display_mode *mode);

void ftd330_crtc_destroy(struct drm_crtc *crtc);

struct ftd330_crtc *ftd330_crtc_create(const struct dc_hw_display *display, struct drm_device *drm_dev,
			       const struct ftd330_dc_info *info, u8 index);

void ftd330_crtc_handle_vblank(struct drm_crtc *crtc);

void ftd330_crtc_handle_frame_done(struct drm_crtc *crtc);

void ftd330_crtc_handle_flip_done(struct drm_crtc *crtc);

void ftd330_crtc_handle_flip_done_while_hw_done(struct drm_crtc *crtc);

int ftd330_drm_atomic_helper_page_flip(struct drm_crtc *crtc,
                                struct drm_framebuffer *fb,
                                struct drm_pending_vblank_event *event,
                                uint32_t flags,
                                struct drm_modeset_acquire_ctx *ctx);

static inline struct ftd330_crtc *to_ftd330_crtc(struct drm_crtc *crtc)
{
	return container_of(crtc, struct ftd330_crtc, base);
}

static inline struct ftd330_crtc *to_ftd330_crtc_const(const struct drm_crtc *crtc)
{
	return container_of(crtc, struct ftd330_crtc, base);
}

static inline struct ftd330_crtc_state *to_ftd330_crtc_state(struct drm_crtc_state *state)
{
	return container_of(state, struct ftd330_crtc_state, base);
}

#endif /* __FTD330_CRTC_H__ */
