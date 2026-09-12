/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_psr_H__
#define __PHYTIUM_psr_H__

#define PSR_DEBUG 0

void phytium_psr_init_dpcd(struct phytium_dp_device *phytium_dp);
void phytium_psr_enable(struct phytium_dp_device *phytium_dp);
void phytium_psr_disable(struct phytium_dp_device *phytium_dp);
void phytium_psr_exit_sf_detect(struct phytium_dp_device *phytium_dp);
void phytium_psr_sf_update(struct phytium_dp_device *phytium_dp);
bool phytium_psr_short_pulse(struct phytium_dp_device *phytium_dp);

#endif /* __PHYTIUM_psr_H__ */