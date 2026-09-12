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
  Once using MMU, put all mmu core info into this array
  Different MMU core order must keep same as vcmd_offset.h or vcd_offset.h
*/
struct mmu_core_cfg mmu_core_array[] = {
#ifdef HAS_VCD
	{ 0x1400, 238*4, 0, 2 }, // decode ok
#else
	{ 0x4000, 238*4, 0, 4 }, // encode ok
#endif

    //{ 0x501400, 238*4, 0, 2 },
};
