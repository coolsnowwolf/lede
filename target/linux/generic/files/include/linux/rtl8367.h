/*
 * Platform data definition for the Realtek RTL8367 ethernet switch driver
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#ifndef _RTL8367_H
#define _RTL8367_H

#define RTL8367_DRIVER_NAME	"rtl8367"
#define RTL8367B_DRIVER_NAME	"rtl8367b"

enum rtl8367_port_speed {
	RTL8367_PORT_SPEED_10 = 0,
	RTL8367_PORT_SPEED_100,
	RTL8367_PORT_SPEED_1000,
};

struct rtl8367_port_ability {
	int force_mode;
	int nway;
	int txpause;
	int rxpause;
	int link;
	int duplex;
	enum rtl8367_port_speed speed;
};

enum rtl8367_extif_mode {
	RTL8367_EXTIF_MODE_DISABLED = 0,
	RTL8367_EXTIF_MODE_RGMII,
	RTL8367_EXTIF_MODE_MII_MAC,
	RTL8367_EXTIF_MODE_MII_PHY,
	RTL8367_EXTIF_MODE_TMII_MAC,
	RTL8367_EXTIF_MODE_TMII_PHY,
	RTL8367_EXTIF_MODE_GMII,
	RTL8367_EXTIF_MODE_RGMII_33V,
};

struct rtl8367_extif_config {
	unsigned int txdelay;
	unsigned int rxdelay;
	enum rtl8367_extif_mode mode;
	struct rtl8367_port_ability ability;
};

struct rtl8367_platform_data {
	unsigned gpio_sda;
	unsigned gpio_sck;
	void (*hw_reset)(bool active);

	struct rtl8367_extif_config *extif0_cfg;
	struct rtl8367_extif_config *extif1_cfg;
};

#endif /*  _RTL8367_H */
