/*
 *  NXP 74HC153 - Dual 4-input multiplexer defines
 *
 *  Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#ifndef _NXP_74HC153_H
#define _NXP_74HC153_H

#define NXP_74HC153_DRIVER_NAME "nxp-74hc153"

struct nxp_74hc153_platform_data {
	unsigned	gpio_base;
	unsigned	gpio_pin_s0;
	unsigned	gpio_pin_s1;
	unsigned	gpio_pin_1y;
	unsigned	gpio_pin_2y;
};

#endif /* _NXP_74HC153_H */
