/*
 * ADM6996 GPIO platform data
 *
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation
 */

#ifndef __PLATFORM_ADM6996_GPIO_H
#define __PLATFORM_ADM6996_GPIO_H

#include <linux/kernel.h>

enum adm6996_model {
	ADM6996FC = 1,
	ADM6996M = 2,
	ADM6996L = 3,
};

struct adm6996_gpio_platform_data {
	u8 eecs;
	u8 eesk;
	u8 eedi;
	enum adm6996_model model;
};

#endif
