/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */

#ifndef	_PHYTIUM_FBDEV_H
#define _PHYTIUM_FBDEV_H

struct drm_fb_helper;
struct drm_fb_helper_surface_size;
int phytium_drm_fbdev_create(struct drm_fb_helper *helper,
			    struct drm_fb_helper_surface_size *sizes);

int phytium_drm_fbdev_init(struct drm_device *dev);
void phytium_drm_fbdev_fini(struct drm_device *dev);

#endif /* _PHYTIUM_FBDEV_H */
