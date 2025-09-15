// SPDX-License-Identifier: GPL-2.0-only
/*
 * CPLD driver for the MikroTik RouterBoard 4xx series
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2015 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 Christopher Hill <ch6574@gmail.com>
 *
 * This file was based on the driver for Linux 2.6.22 published by
 * MikroTik for their RouterBoard 4xx series devices.
 */
#include <linux/spi/spi.h>

struct rb4xx_cpld {
	struct spi_device *spi;

	int (*write_nand)(struct rb4xx_cpld *self, const void *tx_buf,
			  unsigned int len);
	int (*read_nand)(struct rb4xx_cpld *self, void *rx_buf,
			 unsigned int len);

	int (*gpio_set_0_7)(struct rb4xx_cpld *self, u8 values);
	int (*gpio_set_8)(struct rb4xx_cpld *self, u8 value);
};
