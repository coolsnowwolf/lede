/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#ifndef	_PHYTIUM_FBDEV_H
#define _PHYTIUM_FBDEV_H

int phytium_drm_fbdev_init(struct drm_device *dev);
void phytium_drm_fbdev_fini(struct drm_device *dev);

#endif /* _PHYTIUM_FBDEV_H */
