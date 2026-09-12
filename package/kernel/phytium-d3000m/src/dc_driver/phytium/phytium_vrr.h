/* SPDX-License-Identifier: GPL-2.0 */
/* Phytium display drm driver
 *
 * Copyright (C) 2021-2025, Phytium Technology Co., Ltd.
 */

#ifndef __PHYTIUM_vrr_H__
#define __PHYTIUM_vrr_H__
#include "phytium_dp.h"

#define _DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 ||			\
	 (((__x) > 0) == ((__d) > 0))) ?		\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

void phytium_change_fps(struct phytium_dp_device *phytium_dp, bool enable, int freq);
bool
phytium_sink_supports_vrr(struct phytium_dp_device *phytium_dp);

#endif /* __PHYTIUM_vrr_H__ */