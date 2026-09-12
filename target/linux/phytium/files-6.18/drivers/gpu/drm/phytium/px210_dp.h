/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */

#ifndef __PX210_DP_H__
#define __PX210_DP_H__

#define PX210_DP_BACKLIGHT_MAX				100
#define PX210_DP_BACKLIGHT_MIN				0

void px210_dp_func_register(struct phytium_dp_device *phytium_dp);
#endif /* __PX210_DP_H__ */
