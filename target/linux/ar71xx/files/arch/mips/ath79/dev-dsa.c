/*
 *  Atheros AR71xx DSA switch device support
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/version.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-dsa.h"

static struct platform_device ar71xx_dsa_switch_device = {
	.name		= "dsa",
	.id		= 0,
};

void __init ath79_register_dsa(struct device *netdev,
			       struct device *miidev,
			       struct dsa_platform_data *d)
{
	int i;

	d->netdev = netdev;
	for (i = 0; i < d->nr_chips; i++)
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
		d->chip[i].mii_bus = miidev;
#else
		d->chip[i].host_dev = miidev;
#endif

	ar71xx_dsa_switch_device.dev.platform_data = d;
	platform_device_register(&ar71xx_dsa_switch_device);
}
