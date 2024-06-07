/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium Pe220x display controller DRM driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PE220X_DC_H__
#define __PE220X_DC_H__

#define PE220X_DC_PIX_CLOCK_MAX				(594000)
#define PE220X_DC_HDISPLAY_MAX				3840
#define PE220X_DC_VDISPLAY_MAX				2160
#define PE220X_DC_ADDRESS_MASK				0x7f

extern void pe220x_dc_hw_vram_init(struct phytium_display_private *priv,
				  resource_size_t vram_addr,
				  resource_size_t vram_size);
extern void pe220x_dc_hw_config_pix_clock(struct drm_crtc *crtc, int clock);
extern void pe220x_dc_hw_disable(struct drm_crtc *crtc);
extern int pe220x_dc_hw_fb_format_check(const struct drm_mode_fb_cmd2 *mode_cmd, int count);
extern void pe220x_dc_hw_plane_get_primary_format(const uint64_t **format_modifiers,
						 const uint32_t **formats,
						 uint32_t *format_count);
extern void pe220x_dc_hw_plane_get_cursor_format(const uint64_t **format_modifiers,
						const uint32_t **formats,
						uint32_t *format_count);
extern void pe220x_dc_hw_update_primary_hi_addr(struct drm_plane *plane);
extern void pe220x_dc_hw_update_cursor_hi_addr(struct drm_plane *plane, uint64_t iova);
void pe220x_dc_hw_reset(struct drm_crtc *crtc);
#endif /* __PE220X_DC_H__ */
