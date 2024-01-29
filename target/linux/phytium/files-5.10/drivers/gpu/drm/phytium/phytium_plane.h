/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_PLANE_H__
#define __PHYTIUM_PLANE_H__

struct phytium_plane {
	struct drm_plane base;
	int phys_pipe;
	unsigned long iova[PHYTIUM_FORMAT_MAX_PLANE];
	unsigned long size[PHYTIUM_FORMAT_MAX_PLANE];
	unsigned int format;
	unsigned int tiling[PHYTIUM_FORMAT_MAX_PLANE];
	unsigned int swizzle;
	unsigned int uv_swizzle;
	unsigned int rot_angle;

	/* only for cursor */
	bool enable;
	bool reserve[3];
	int cursor_x;
	int cursor_y;
	int cursor_hot_x;
	int cursor_hot_y;

	void (*dc_hw_plane_get_format)(const uint64_t **format_modifiers,
				       const uint32_t **formats,
				       uint32_t *format_count);
	void (*dc_hw_update_dcreq)(struct drm_plane *plane);
	void (*dc_hw_update_primary_hi_addr)(struct drm_plane *plane);
	void (*dc_hw_update_cursor_hi_addr)(struct drm_plane *plane, uint64_t iova);
};

struct phytium_plane_state {
	struct drm_plane_state base;
};

#define	to_phytium_plane(x)		container_of(x, struct phytium_plane, base)
#define	to_phytium_plane_state(x)	container_of(x, struct phytium_plane_state, base)

struct phytium_plane *phytium_primary_plane_create(struct drm_device *dev, int pipe);
struct phytium_plane *phytium_cursor_plane_create(struct drm_device *dev, int pipe);
#endif /* __PHYTIUM_PLANE_H__ */
