/*
 * Driver for Moschip MCS814x internal PHY
 *
 * Copyright (c) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/phy.h>

MODULE_DESCRIPTION("Moschip MCS814x PHY driver");
MODULE_AUTHOR("Florian Fainelli <florian@openwrt.org>");
MODULE_LICENSE("GPL");

/* Nothing special about this PHY but its OUI (O) */
static struct phy_driver mcs8140_driver = {
	.phy_id		= 0,
	.name		= "Moschip MCS8140",
	.phy_id_mask	= 0x02,
	.features	= PHY_BASIC_FEATURES,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.suspend	= genphy_suspend,
	.resume		= genphy_resume,
	.driver		= { .owner = THIS_MODULE,},
};

static int __init mcs814x_phy_init(void)
{
	return phy_driver_register(&mcs8140_driver);
}

static void __exit mcs814x_phy_exit(void)
{
	phy_driver_unregister(&mcs8140_driver);
}

module_init(mcs814x_phy_init);
module_exit(mcs814x_phy_exit);

static struct mdio_device_id __maybe_unused mcs814x_phy_tbl[] = {
	{ 0x0, 0x0ffffff0 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, mcs814x_phy_tbl);
