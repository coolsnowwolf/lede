/*
 * OpenWrt swconfig support for MediaTek MT753x Gigabit switch
 *
 * Copyright (C) 2018 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _MT753X_SWCONFIG_H_
#define _MT753X_SWCONFIG_H_

#include <linux/switch.h>

int mt753x_swconfig_init(struct gsw_mt753x *gsw);
void mt753x_swconfig_destroy(struct gsw_mt753x *gsw);

#endif /* _MT753X_SWCONFIG_H_ */
