/*
 *  TP-LINK TL-WR941N v7 board support
 *
 *  Copyright (C) 2012 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2014 Weijie Gao <hackpacsal@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define WR941NV7_GPIO_LED_WLAN			12
#define WR941NV7_GPIO_LED_SYSTEM		19
#define WR941NV7_GPIO_LED_QSS			15

#define WR941NV7_GPIO_BTN_RESET		16

#define WR941NV7_KEYS_POLL_INTERVAL		20	/* msecs */
#define WR941NV7_KEYS_DEBOUNCE_INTERVAL	(3 * WR941NV7_KEYS_POLL_INTERVAL)

#define ATH_MII_MGMT_CMD		0x24
#define ATH_MGMT_CMD_READ		0x1

#define ATH_MII_MGMT_ADDRESS		0x28
#define ATH_ADDR_SHIFT			8

#define ATH_MII_MGMT_CTRL		0x2c
#define ATH_MII_MGMT_STATUS		0x30

#define ATH_MII_MGMT_IND		0x34
#define ATH_MGMT_IND_BUSY		(1 << 0)
#define ATH_MGMT_IND_INVALID		(1 << 2)

static const char *wr941nv7_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data wr941nv7_flash_data = {
	.part_probes	= wr941nv7_part_probes,
};

static struct gpio_led wr941nv7_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= WR941NV7_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= WR941NV7_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan",
		.gpio		= WR941NV7_GPIO_LED_WLAN,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wr941nv7_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WR941NV7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= WR941NV7_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static struct mdio_board_info wr941nv7_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = NULL,
	},
};

static unsigned long __init ath_gmac_reg_rd(unsigned long reg)
{
	void __iomem *base;
	unsigned long t;

	base = ioremap(AR71XX_GE0_BASE, AR71XX_GE0_SIZE);

	t = __raw_readl(base + reg);

	iounmap(base);

	return t;
}

static void __init ath_gmac_reg_wr(unsigned long reg, unsigned long value)
{
	void __iomem *base;
	unsigned long t = value;

	base = ioremap(AR71XX_GE0_BASE, AR71XX_GE0_SIZE);

	__raw_writel(t, base + reg);

	iounmap(base);
}

static void __init phy_reg_write(unsigned char phy_addr, unsigned char reg, unsigned short data)
{
	unsigned short addr = (phy_addr << ATH_ADDR_SHIFT) | reg;
	volatile int rddata;
	unsigned short ii = 0xFFFF;

	do
	{
		udelay(5);
		rddata = ath_gmac_reg_rd(ATH_MII_MGMT_IND) & 0x1;
	} while (rddata && --ii);

	ath_gmac_reg_wr(ATH_MII_MGMT_ADDRESS, addr);
	ath_gmac_reg_wr(ATH_MII_MGMT_CTRL, data);

	do
	{
		udelay(5);
		rddata = ath_gmac_reg_rd(ATH_MII_MGMT_IND) & 0x1;
	} while (rddata && --ii);
}

static unsigned short __init phy_reg_read(unsigned char phy_addr, unsigned char reg)
{
	unsigned short addr = (phy_addr << ATH_ADDR_SHIFT) | reg, val;
	volatile int rddata;
	unsigned short ii = 0xffff;

	do
	{
		udelay(5);
		rddata = ath_gmac_reg_rd(ATH_MII_MGMT_IND) & 0x1;
	} while (rddata && --ii);

	ath_gmac_reg_wr(ATH_MII_MGMT_CMD, 0x0);
	ath_gmac_reg_wr(ATH_MII_MGMT_ADDRESS, addr);
	ath_gmac_reg_wr(ATH_MII_MGMT_CMD, ATH_MGMT_CMD_READ);

	do
	{
		udelay(5);
		rddata = ath_gmac_reg_rd(ATH_MII_MGMT_IND) & 0x1;
	} while (rddata && --ii);

	val = ath_gmac_reg_rd(ATH_MII_MGMT_STATUS);
	ath_gmac_reg_wr(ATH_MII_MGMT_CMD, 0x0);

	return val;
}

static void __init athrs27_reg_write(unsigned int s27_addr, unsigned int s27_write_data)
{
	unsigned int addr_temp;
	unsigned int data;
	unsigned char phy_address, reg_address;

	addr_temp = (s27_addr) >> 2;
	data = addr_temp >> 7;

	phy_address = 0x1f;
	reg_address = 0x10;

	phy_reg_write(phy_address, reg_address, data);

	phy_address = (0x17 & ((addr_temp >> 4) | 0x10));

	reg_address = (((addr_temp << 1) & 0x1e) | 0x1);
	data = (s27_write_data >> 16) & 0xffff;
	phy_reg_write(phy_address, reg_address, data);

	reg_address = ((addr_temp << 1) & 0x1e);
	data = s27_write_data  & 0xffff;
	phy_reg_write(phy_address, reg_address, data);
}

static unsigned int __init athrs27_reg_read(unsigned int s27_addr)
{
	unsigned int addr_temp;
	unsigned int s27_rd_csr_low, s27_rd_csr_high, s27_rd_csr;
	unsigned int data;
	unsigned char phy_address, reg_address;

	addr_temp = s27_addr >>2;
	data = addr_temp >> 7;

	phy_address = 0x1f;
	reg_address = 0x10;

	phy_reg_write(phy_address, reg_address, data);

	phy_address = (0x17 & ((addr_temp >> 4) | 0x10));
	reg_address = ((addr_temp << 1) & 0x1e);
	s27_rd_csr_low = (unsigned int) phy_reg_read(phy_address, reg_address);

	reg_address = reg_address | 0x1;
	s27_rd_csr_high = (unsigned int) phy_reg_read(phy_address, reg_address);
	s27_rd_csr = (s27_rd_csr_high << 16) | s27_rd_csr_low ;
	
	return (s27_rd_csr);
}

static void __init ar8236_reset(void)
{
	unsigned short i = 60;

	athrs27_reg_write(0x0, athrs27_reg_read(0x0) | 0x80000000);
	while (i--)
	{
		mdelay(100);
		if (!(athrs27_reg_read(0x0) & 0x80000000))
		break;
	}
}

static void __init wr941nv7_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(&wr941nv7_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wr941nv7_leds_gpio),
				 wr941nv7_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WR941NV7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wr941nv7_gpio_keys),
					wr941nv7_gpio_keys);

	ath79_register_wmac(art, mac);

	ar8236_reset();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_MII_GE0 |
		QCA955X_ETH_CFG_MII_GE0_SLAVE);

	mdiobus_register_board_info(wr941nv7_mdio0_info,
				    ARRAY_SIZE(wr941nv7_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);

	/* GMAC0 is connected to an AR8236 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_TL_WR941N_V7, "TL-WR941N-v7",
	     "TP-LINK TL-WR941N v7",
	     wr941nv7_setup);
