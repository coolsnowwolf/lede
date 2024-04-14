/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_FB_H__
#define __PHYTIUM_FB_H__

#include <drm/drm_framebuffer.h>

struct phytium_framebuffer {
	struct drm_framebuffer base;
	struct phytium_gem_object *phytium_gem_obj[PHYTIUM_FORMAT_MAX_PLANE];
};

#define	to_phytium_framebuffer(fb)	container_of(fb, struct phytium_framebuffer, base)

struct phytium_framebuffer *phytium_fb_alloc(struct drm_device *dev,
						   const struct drm_mode_fb_cmd2 *mode_cmd,
						   struct phytium_gem_object **phytium_gem_obj,
						   unsigned int num_planes);

struct drm_framebuffer *phytium_fb_create(struct drm_device *dev, struct drm_file *file_priv,
						 const struct drm_mode_fb_cmd2 *mode_cmd);
#endif /* __PHYTIUM_FB_H__ */
