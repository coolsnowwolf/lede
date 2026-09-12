// SPDX-License-Identifier: GPL-2.0
/*
 * ftv310 register for power control.
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

#ifndef __PHYTIUM_VPU_POWER_REG_H_
#define __PHYTIUM_VPU_POWER_REG_H_

#include <linux/acpi.h>
#include "ftv310_vpu_vcmd.h"

#define DECODE_REG_OFFSET		0x0
#define ENCODE_REG_OFFSET		0x20

#define AP_CPPC1_STAT		0x0
#define AP_CPPC1_SET  		0x4
	#define AP_TRIGER 		0x1

#define SE_PAYLOAD_0		0x18
	#define PAYLOAD_STATE_SHIFT	24
	#define PROTOCAL_ID 		(0x11 << 16)
	#define MESSAGE_ID		(0x04 << 8)
	#define CHANNEL_STATUS_FREE	1
#define	SE_PAYLOAD_1		0x1c
	#define STATE_OFF		0x00
	#define STATE_ON		0x08
	#define STATE_TYPE		(1 << 30)

#endif
