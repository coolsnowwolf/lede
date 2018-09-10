/*
 * ZyXEL NBG6716/NBG6616 board support
 *
 * Based on the Qualcomm Atheros AP135/AP136 reference board support code
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2013 Andre Valentin <avalentin@marcant.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include <linux/version.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>
#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-nfc.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "nvram.h"

#define NBG6716_GPIO_LED_INTERNET	18
#define NBG6716_GPIO_LED_POWER		15
#define NBG6716_GPIO_LED_USB1		4
#define NBG6716_GPIO_LED_USB2		13
#define NBG6716_GPIO_LED_WIFI2G		19
#define NBG6716_GPIO_LED_WIFI5G		17
#define NBG6716_GPIO_LED_WPS		21

#define NBG6716_GPIO_BTN_RESET		23
#define NBG6716_GPIO_BTN_RFKILL		1
#define NBG6716_GPIO_BTN_USB1		0
#define NBG6716_GPIO_BTN_USB2		14
#define NBG6716_GPIO_BTN_WPS		22

#define NBG6716_GPIO_USB_POWER		16

#define NBG6716_KEYS_POLL_INTERVAL	20	/* msecs */
#define NBG6716_KEYS_DEBOUNCE_INTERVAL	(3 * NBG6716_KEYS_POLL_INTERVAL)

#define NBG6716_MAC0_OFFSET		0
#define NBG6716_MAC1_OFFSET		6
#define NBG6716_WMAC_CALDATA_OFFSET	0x1000
#define NBG6716_PCIE_CALDATA_OFFSET	0x5000

/* NBG6616 has a different GPIO usage as it does not have USB Buttons */
#define NBG6616_GPIO_LED_USB0		14
#define NBG6616_GPIO_LED_USB1		21
#define NBG6616_GPIO_LED_WPS		0

static struct gpio_led nbg6716_leds_gpio[] __initdata = {
	{
		.name		= "nbg6716:white:internet",
		.gpio		= NBG6716_GPIO_LED_INTERNET,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:power",
		.gpio		= NBG6716_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:usb1",
		.gpio		= NBG6716_GPIO_LED_USB1,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:usb2",
		.gpio		= NBG6716_GPIO_LED_USB2,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:wifi2g",
		.gpio		= NBG6716_GPIO_LED_WIFI2G,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:wifi5g",
		.gpio		= NBG6716_GPIO_LED_WIFI5G,
		.active_low	= 1,
	},
	{
		.name		= "nbg6716:white:wps",
		.gpio		= NBG6716_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button nbg6716_gpio_keys[] __initdata = {
	{
		.desc		= "RESET button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_RFKILL,
		.active_low	= 0,
	},
	{
		.desc		= "USB1 eject button",
		.type		= EV_KEY,
		.code		= BTN_1,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_USB1,
		.active_low	= 1,
	},
	{
		.desc		= "USB2 eject button",
		.type		= EV_KEY,
		.code		= BTN_2,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_USB2,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};



static struct gpio_led nbg6616_leds_gpio[] __initdata = {
	{
		.name		= "nbg6616:green:power",
		.gpio		= NBG6716_GPIO_LED_POWER,
		.active_low	= 1,
	},
	{
		.name		= "nbg6616:green:usb2",
		.gpio		= NBG6616_GPIO_LED_USB0,
		.active_low	= 1,
	},
	{
		.name		= "nbg6616:green:usb1",
		.gpio		= NBG6616_GPIO_LED_USB1,
		.active_low	= 1,
	},
	{
		.name		= "nbg6616:green:wifi2g",
		.gpio		= NBG6716_GPIO_LED_WIFI2G,
		.active_low	= 1,
	},
	{
		.name		= "nbg6616:green:wifi5g",
		.gpio		= NBG6716_GPIO_LED_WIFI5G,
		.active_low	= 1,
	},
	{
		.name		= "nbg6616:green:wps",
		.gpio		= NBG6616_GPIO_LED_WPS,
		.active_low	= 1,
	}
};

static struct gpio_keys_button nbg6616_gpio_keys[] __initdata = {
	{
		.desc		= "RESET button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_RFKILL,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = NBG6716_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= NBG6716_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};


static struct ar8327_pad_cfg nbg6716_ar8327_pad0_cfg;
static struct ar8327_pad_cfg nbg6716_ar8327_pad6_cfg;
static struct ar8327_led_cfg nbg6716_ar8327_led_cfg;

static struct ar8327_platform_data nbg6716_ar8327_data = {
	.pad0_cfg = &nbg6716_ar8327_pad0_cfg,
	.pad6_cfg = &nbg6716_ar8327_pad6_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.port6_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &nbg6716_ar8327_led_cfg
};

static struct mdio_board_info nbg6716_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &nbg6716_ar8327_data,
	},
};

static void nbg6716_get_mac(void* nvram_addr, const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(nvram_addr);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, 0x10000,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

static void __init nbg6716_common_setup(u32 leds_num, struct gpio_led* leds,
					u32 keys_num,
					struct gpio_keys_button* keys,
					void* art_addr, void* nvram)
{
	u8 *art = (u8 *) KSEG1ADDR(art_addr);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, leds_num, leds);
	ath79_register_gpio_keys_polled(-1, NBG6716_KEYS_POLL_INTERVAL,
					keys_num, keys);

	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	gpio_request_one(NBG6716_GPIO_USB_POWER,
		GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
		"USB power");

	ath79_register_usb();

	nbg6716_get_mac(nvram, "ethaddr=", tmpmac);

	ath79_register_pci();

	ath79_register_wmac(art + NBG6716_WMAC_CALDATA_OFFSET, tmpmac);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, tmpmac, 2);
	ath79_init_mac(ath79_eth1_data.mac_addr, tmpmac, 3);

	mdiobus_register_board_info(nbg6716_mdio0_info,
				    ARRAY_SIZE(nbg6716_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);
}

static void __init nbg6716_010_setup(void)
{
	/* GMAC0 of the AR8337 switch is connected to GMAC0 via RGMII */
	nbg6716_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_RGMII;
	nbg6716_ar8327_pad0_cfg.txclk_delay_en = true;
	nbg6716_ar8327_pad0_cfg.rxclk_delay_en = true;
	nbg6716_ar8327_pad0_cfg.txclk_delay_sel = AR8327_CLK_DELAY_SEL1;
	nbg6716_ar8327_pad0_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2;

	/* GMAC6 of the AR8337 switch is connected to GMAC1 via SGMII */
	nbg6716_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_SGMII;
	nbg6716_ar8327_pad6_cfg.rxclk_delay_en = true;
	nbg6716_ar8327_pad6_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0;

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	nbg6716_ar8327_led_cfg.open_drain = 0;
	nbg6716_ar8327_led_cfg.led_ctrl0 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl1 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl2 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl3 = 0x03ffff00;

	nbg6716_common_setup(ARRAY_SIZE(nbg6716_leds_gpio), nbg6716_leds_gpio,
			     ARRAY_SIZE(nbg6716_gpio_keys), nbg6716_gpio_keys,
			     (void*) 0x1f050000, (void*) 0x1f040000);
}

static void __init nbg6616_010_setup(void)
{
	/* GMAC0 of the AR8337 switch is connected to GMAC0 via RGMII */
	nbg6716_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_RGMII;
	nbg6716_ar8327_pad0_cfg.txclk_delay_en = true;
	nbg6716_ar8327_pad0_cfg.rxclk_delay_en = true;
	nbg6716_ar8327_pad0_cfg.txclk_delay_sel = AR8327_CLK_DELAY_SEL1;
	nbg6716_ar8327_pad0_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2;

	/* GMAC6 of the AR8337 switch is connected to GMAC1 via SGMII */
	nbg6716_ar8327_pad6_cfg.mode = AR8327_PAD_MAC_SGMII;
	nbg6716_ar8327_pad6_cfg.rxclk_delay_en = true;
	nbg6716_ar8327_pad6_cfg.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0;

	ath79_eth0_pll_data.pll_1000 = 0xa6000000;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	nbg6716_ar8327_led_cfg.open_drain = 0;
	nbg6716_ar8327_led_cfg.led_ctrl0 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl1 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl2 = 0xffb7ffb7;
	nbg6716_ar8327_led_cfg.led_ctrl3 = 0x03ffff00;


	nbg6716_common_setup(ARRAY_SIZE(nbg6616_leds_gpio), nbg6616_leds_gpio,
			     ARRAY_SIZE(nbg6616_gpio_keys), nbg6616_gpio_keys,
			     (void*) 0x1f040000, (void*) 0x1f030000);
}


MIPS_MACHINE(ATH79_MACH_NBG6716, "NBG6716",
	     "Zyxel NBG6716",
	     nbg6716_010_setup);

MIPS_MACHINE(ATH79_MACH_NBG6616, "NBG6616",
	     "Zyxel NBG6616",
	     nbg6616_010_setup);

