/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 SE header file.
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

#ifndef __PHYTIUM_SE_COMMUNICATE_H_
#define __PHYTIUM_SE_COMMUNICATE_H_

#include "ftv310_vpu_vcmd.h"

#define DECODE_SE_REG_OFFSET		0x0
#define ENCODE_SE_REG_OFFSET      0x20
#define SE_AP_STAT 		0x0
#define SE_AP_SET  		0x4
#define SE_AP_CLEAR		0x8
#define SE_SCP_STAT		0x0c
#define SE_SCP_SET		0x10
#define SE_SCP_CLEAR	0x14
#define SE_PAYLOAD_0		0x18
#define	SE_PAYLOAD_1		0x1c 

#define VPU_POWER_PROTOCAL_ID 	0x11
#define VPU_POWER_MESSAGE_ID	0x04
#define VPU_POWER_PAYLOAD1_RESERVEE	(1 << 30)
#define VPU_POWER_STATE_ENABLE	0x08
#define VPU_POWER_STATE_DISABLE	0x00

#define VPU_CHANGE_PXL_CLK_PROTOCAL_ID	0x14
#define VPU_CHANGE_PXL_CLK_MESSAGE_ID	0x05

void ftv310_vpu_power_request_se(vcmd_slice_str *slice, bool is_decode, bool enable);

#endif
