/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_DC_LUT_H__
#define __FTD330_DC_LUT_H__

#include <linux/types.h>

bool ftd330_dc_lut_check_xstep(const u32 *data, u32 size, u32 bit_width);

bool ftd330_dc_lut_check_data(const u32 *data, u32 size, u32 bit_width);

#endif