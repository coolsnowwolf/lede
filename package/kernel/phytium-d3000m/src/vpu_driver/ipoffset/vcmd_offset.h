// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver main entrance.
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
/*

1. xxx_offset.h config strategy:
	Without VCMD:
		1. Config all supported xxx_offset.h
		2. Except vcmd_offset.h

	With VCDM:
		Config vcmd_offset.h only

2. Array config example:
	struct vcmd_config vcmd_core_array[VCMD_SLICE_NUM][CORE_NUM]
	When enable decoder and encoder together, you can change like following:
	   1. Change cmd_core_array[1][1] to vcmd_core_array[1][2]
	   3. Put all decoder core info in vcmd_core_array[0]
   	   4. Put all encoder core info in vcmd_core_array[0]
	   5. Example:
			struct vcmd_config vcmd_core_array[1][2] = {
				{
					{ 0x600000, 30*4, -1, 2, 0x800, 0xFFFF, 0xFFFF, 0x1400, 0xFFFF, { 0xFFFF, 0xFFFF } }, //config decoder core0 IP info
					{ 0x400000, 30*4, -1, 0, 0x800, 0xFFFF, 0xFFFF, 0x1400, 0xFFFF, { 0xFFFF, 0xFFFF } }, //config encoder core0 IP info
				},
			};

	When enable decoder only with multi core, you can change like following:
	   1. Using cmd_core_array[1][x]
	   2. put decoder core info into cmd_core_array[0][x], x can be "1,2,3"
	   3. Example:
			struct vcmd_config vcmd_core_array[1][3] = {
				{
					{ 0x600000, 30*4, -1, 2, 0x800, 0xFFFF, 0xFFFF, 0x1400, 0xFFFF, { 0xFFFF, 0xFFFF } },
					{ 0x500000, 30*4, -1, 2, 0x800, 0xFFFF, 0xFFFF, 0x1400, 0xFFFF, { 0xFFFF, 0xFFFF } }
				},
			};

*/

#define VCMD_SLICE_NUM 1 //always using VCMD_SLICE_NUM equal to 1 if you don't specify different slice in UMD source code

struct vcmd_config vcmd_core_array[VCMD_SLICE_NUM][2] = {
    {
    	{ 0x0, 0x400, -1, 0, 0x1000, 0x2000, 0xFFFF, 0x4000, 0xFFFF, { 0xffff, 0xFFFF } }, // encode ok
	{ 0x8000, 0x400, -1, 2, 0x800, 0x2000, 0xFFFF, 0x1400, 0xFFFF, { 0xffff, 0xFFFF } }, // decode ok
    },
};




