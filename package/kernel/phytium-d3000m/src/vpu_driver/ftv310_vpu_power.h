/* SPDX-License-Identifier: GPL-2.0 */
/*
 * header file for ftv310 power control.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You may obtain a copy of the GNU General Public License
 * Version 2 at the following locations:
 * https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef __PHYTIUM_VPU_POWER_H_
#define __PHYTIUM_VPU_POWER_H_

#include "ftv310_vpu_vcmd.h"

extern void ftv310_vpu_power_init(struct platform_device *pdev, vcmd_slice_str *slice);
extern int ftv310_vpu_power_request(struct device *dev, vcmd_slice_str *slice, bool is_decode, bool on);

#endif
