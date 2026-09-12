/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Phytium display port DRM driver
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */

#ifndef __PE220X_DP_H__
#define __PE220X_DP_H__

#define PE220X_DP_BACKLIGHT_MAX				99
#define PE220X_DP_BACKLIGHT_MIN				2

void pe220x_dp_func_register(struct phytium_dp_device *phytium_dp);
#endif /* __PE220X_DP_H__ */
