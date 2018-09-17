/*
 *  Atheros AR71xx SoC device definitions
 *
 *  Copyright (C) 2008-2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_DEV_ETH_H
#define _ATH79_DEV_ETH_H

#include <asm/mach-ath79/ag71xx_platform.h>

struct platform_device;

extern unsigned char ath79_mac_base[] __initdata;
void ath79_parse_ascii_mac(char *mac_str, u8 *mac);
void ath79_extract_mac_reverse(u8 *ptr, u8 *out);
void ath79_init_mac(unsigned char *dst, const unsigned char *src,
		    int offset);
void ath79_init_local_mac(unsigned char *dst, const unsigned char *src);

struct ath79_eth_pll_data {
	u32	pll_10;
	u32	pll_100;
	u32	pll_1000;
};

extern struct ath79_eth_pll_data ath79_eth0_pll_data;
extern struct ath79_eth_pll_data ath79_eth1_pll_data;

extern struct ag71xx_platform_data ath79_eth0_data;
extern struct ag71xx_platform_data ath79_eth1_data;
extern struct platform_device ath79_eth0_device;
extern struct platform_device ath79_eth1_device;
void ath79_register_eth(unsigned int id);

extern struct ag71xx_switch_platform_data ath79_switch_data;

extern struct ag71xx_mdio_platform_data ath79_mdio0_data;
extern struct ag71xx_mdio_platform_data ath79_mdio1_data;
extern struct platform_device ath79_mdio0_device;
extern struct platform_device ath79_mdio1_device;
void ath79_register_mdio(unsigned int id, u32 phy_mask);

void ath79_setup_ar933x_phy4_switch(bool mac, bool mdio);
void ath79_setup_ar934x_eth_cfg(u32 mask);
void ath79_setup_ar934x_eth_rx_delay(unsigned int rxd, unsigned int rxdv);
void ath79_setup_qca955x_eth_cfg(u32 mask);
void ath79_setup_qca956x_eth_cfg(u32 mask);

#endif /* _ATH79_DEV_ETH_H */
