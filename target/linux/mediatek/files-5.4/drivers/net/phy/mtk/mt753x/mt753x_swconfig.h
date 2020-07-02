/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#ifndef _MT753X_SWCONFIG_H_
#define _MT753X_SWCONFIG_H_

#ifdef CONFIG_SWCONFIG
#include <linux/switch.h>
#include "mt753x.h"

int mt753x_swconfig_init(struct gsw_mt753x *gsw);
void mt753x_swconfig_destroy(struct gsw_mt753x *gsw);
#else
static inline int mt753x_swconfig_init(struct gsw_mt753x *gsw)
{
	mt753x_apply_vlan_config(gsw);

	return 0;
}

static inline void mt753x_swconfig_destroy(struct gsw_mt753x *gsw)
{
}
#endif

#endif /* _MT753X_SWCONFIG_H_ */
