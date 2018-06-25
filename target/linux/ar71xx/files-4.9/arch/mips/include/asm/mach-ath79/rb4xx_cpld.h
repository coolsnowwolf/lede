/*
 * SPI driver definitions for the CPLD chip on the Mikrotik RB4xx boards
 *
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was based on the patches for Linux 2.6.27.39 published by
 * MikroTik for their RouterBoard 4xx series devices.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#define CPLD_GPIO_nLED1		0
#define CPLD_GPIO_nLED2		1
#define CPLD_GPIO_nLED3		2
#define CPLD_GPIO_nLED4		3
#define CPLD_GPIO_FAN		4
#define CPLD_GPIO_ALE		5
#define CPLD_GPIO_CLE		6
#define CPLD_GPIO_nCE		7
#define CPLD_GPIO_nLED5		8

#define CPLD_NUM_GPIOS		9

#define CPLD_CFG_nLED1		BIT(CPLD_GPIO_nLED1)
#define CPLD_CFG_nLED2		BIT(CPLD_GPIO_nLED2)
#define CPLD_CFG_nLED3		BIT(CPLD_GPIO_nLED3)
#define CPLD_CFG_nLED4		BIT(CPLD_GPIO_nLED4)
#define CPLD_CFG_FAN		BIT(CPLD_GPIO_FAN)
#define CPLD_CFG_ALE		BIT(CPLD_GPIO_ALE)
#define CPLD_CFG_CLE		BIT(CPLD_GPIO_CLE)
#define CPLD_CFG_nCE		BIT(CPLD_GPIO_nCE)
#define CPLD_CFG_nLED5		BIT(CPLD_GPIO_nLED5)

struct rb4xx_cpld_platform_data {
	unsigned	gpio_base;
};

extern int rb4xx_cpld_change_cfg(unsigned mask, unsigned value);
extern int rb4xx_cpld_read(unsigned char *rx_buf,
			   unsigned cnt);
extern int rb4xx_cpld_write(const unsigned char *buf, unsigned count);
