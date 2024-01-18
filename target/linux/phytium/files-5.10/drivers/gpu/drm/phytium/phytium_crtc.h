/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_CRTC_H__
#define __PHYTIUM_CRTC_H__

struct phytium_crtc {
	struct drm_crtc base;
	int phys_pipe;
	unsigned int bpc;

	/* scale */
	uint32_t src_width;
	uint32_t src_height;
	uint32_t dst_width;
	uint32_t dst_height;
	uint32_t dst_x;
	uint32_t dst_y;
	bool scale_enable;
	bool reserve[3];

	void (*dc_hw_config_pix_clock)(struct drm_crtc *crtc, int clock);
	void (*dc_hw_disable)(struct drm_crtc *crtc);
	void (*dc_hw_reset)(struct drm_crtc *crtc);
};

struct phytium_crtc_state {
	struct drm_crtc_state base;
};

#define to_phytium_crtc(x) container_of(x, struct phytium_crtc, base)
#define to_phytium_crtc_state(x) container_of(x, struct phytium_crtc_state, base)

void phytium_crtc_resume(struct drm_device *drm_dev);
int phytium_crtc_init(struct drm_device *dev, int pipe);
#endif /* __PHYTIUM_CRTC_H__ */
