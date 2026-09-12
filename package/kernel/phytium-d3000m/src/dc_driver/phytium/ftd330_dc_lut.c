// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include "ftd330_dc_lut.h"

#include <linux/printk.h>
#include <linux/types.h>

bool ftd330_dc_lut_check_xstep(const u32 *data, u32 size, u32 bit_width)
{
	u32 i;
	/* check the effectiveness of x-step */
	for (i = 0; i < size; i++) {
		if (data[i] >> bit_width) {
			pr_err("%s: The x-step %u exceeds the valid bits %u.\n", __func__, data[i],
			       bit_width);
			return false;
		}

		if (data[i] & (data[i] - 1)) {
			pr_err("%s: The x-step %u should be the power of 2.\n", __func__, data[i]);
			return false;
		}
	}
	return true;
}

bool ftd330_dc_lut_check_data(const u32 *data, u32 size, u32 bit_width)
{
	u32 i;
	/* check the effectiveness of LUT data */
	for (i = 0; i < size; i++) {
		if (data[i] >> bit_width) {
			pr_err("%s: The LUT data %u exceeds the valid bits %u.\n", __func__,
			       data[i], bit_width);
			return false;
		}

		/* EOTF/OETF no restricted requirement incremental curve in HW */
		// if ((i > 0) && (data[i] < data[i - 1])) {
		//	pr_err("%s: The curve is not monotonically increasing.\n", __func__);
		//	return false;
		// }

	}
	return true;
}
