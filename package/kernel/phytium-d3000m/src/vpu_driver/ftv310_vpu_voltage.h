/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 voltage request header file.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef __PHYTIUM_VPU_VOLTAGE_H_
#define __PHYTIUM_VPU_VOLTAGE_H_

#include <linux/acpi.h>
#include <linux/kernel.h>
#include <linux/device.h>

extern int ftv310_vpu_voltage_request(struct device *dev, bool is_decode, unsigned long *volt);

#endif

