/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025 Phytium Technology Co., Ltd.
 */
#ifndef __PHYTIUM_FBDEV__H__
#define __PHYTIUM_FBDEV__H__
#include "ftd330_drv.h"

void phytium_drm_fbdev_init(struct ftd330_drm_private *priv);
void phytium_drm_fbdev_fini(struct drm_device *dev);


#endif
