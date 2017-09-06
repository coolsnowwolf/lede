/*
 *  WD My Net N750 board support
 *
 *  Copyright (C) 2013 Felix Kaechele <felix@fetzig.org>
 *  Copyright (C) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/delay.h>
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
#include "nvram.h"


/*
 * Taken from GPL bootloader source:
 *   board/ar7240/db12x/alpha_gpio.c
 */
#define MYNET_N750_GPIO_LED_WIFI	11
#define MYNET_N750_GPIO_LED_INTERNET	12
#define MYNET_N750_GPIO_LED_WPS		13
#define MYNET_N750_GPIO_LED_POWER	14

#define MYNET_N750_GPIO_BTN_RESET	17
#define MYNET_N750_GPIO_BTN_WPS		19

#define MYNET_N750_GPIO_EXTERNAL_LNA0	15
#define MYNET_N750_GPIO_EXTERNAL_LNA1	18

#define MYNET_N750_KEYS_POLL_INTERVAL	20	/* msecs */
#define MYNET_N750_KEYS_DEBOUNCE_INTERVAL (3 * MYNET_N750_KEYS_POLL_INTERVAL)

#define MYNET_N750_WMAC_CALDATA_OFFSET	0x1000
#define MYNET_N750_PCIE_CALDATA_OFFSET	0x5000

#define MYNET_N750_NVRAM_ADDR		0x1f058010
#define MYNET_N750_NVRAM_SIZE		0x7ff0

static struct gpio_led mynet_n750_leds_gpio[] __initdata = {
	{
		.name		= "wd:blue:power",
		.gpio		= MYNET_N750_GPIO_LED_POWER,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:wps",
		.gpio		= MYNET_N750_GPIO_LED_WPS,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:wireless",
		.gpio		= MYNET_N750_GPIO_LED_WIFI,
		.active_low	= 0,
	},
	{
		.name		= "wd:blue:internet",
		.gpio		= MYNET_N750_GPIO_LED_INTERNET,
		.active_low	= 0,
	},
};

static struct gpio_keys_button mynet_n750_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = MYNET_N750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_N750_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = MYNET_N750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= MYNET_N750_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info mynet_n750_leds_ar8327[] __initconst = {
	AR8327_LED_INFO(PHY0_0, HW, "wd:green:lan1"),
	AR8327_LED_INFO(PHY1_0, HW, "wd:green:lan2"),
	AR8327_LED_INFO(PHY2_0, HW, "wd:green:lan3"),
	AR8327_LED_INFO(PHY3_0, HW, "wd:green:lan4"),
	AR8327_LED_INFO(PHY4_0, HW, "wd:green:wan"),
	AR8327_LED_INFO(PHY0_1, HW, "wd:yellow:lan1"),
	AR8327_LED_INFO(PHY1_1, HW, "wd:yellow:lan2"),
	AR8327_LED_INFO(PHY2_1, HW, "wd:yellow:lan3"),
	AR8327_LED_INFO(PHY3_1, HW, "wd:yellow:lan4"),
	AR8327_LED_INFO(PHY4_1, HW, "wd:yellow:wan"),
};

static struct ar8327_pad_cfg mynet_n750_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg mynet_n750_ar8327_led_cfg = {
	.led_ctrl0 = 0xcc35cc35,
	.led_ctrl1 = 0xca35ca35,
	.led_ctrl2 = 0xc935c935,
	.led_ctrl3 = 0x03ffff00,
	.open_drain = false,
};

static struct ar8327_platform_data mynet_n750_ar8327_data = {
	.pad0_cfg = &mynet_n750_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
	.led_cfg = &mynet_n750_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(mynet_n750_leds_ar8327),
	.leds = mynet_n750_leds_ar8327,
};

static struct mdio_board_info mynet_n750_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &mynet_n750_ar8327_data,
	},
};

static void mynet_n750_get_mac(const char *name, char *mac)
{
	u8 *nvram = (u8 *) KSEG1ADDR(MYNET_N750_NVRAM_ADDR);
	int err;

	err = ath79_nvram_parse_mac_addr(nvram, MYNET_N750_NVRAM_SIZE,
					 name, mac);
	if (err)
		pr_err("no MAC address found for %s\n", name);
}

/*
 * The bootloader on this board powers down all PHYs on the switch
 * before booting the kernel. We bring all PHYs back up so that they are
 * discoverable by the mdio bus scan and the switch is detected
 * correctly.
 */
static void mynet_n750_mdio_fixup(struct mii_bus *bus)
{
	int i;

	for (i = 0; i < 5; i++)
		bus->write(bus, i, MII_BMCR,
			   (BMCR_RESET | BMCR_ANENABLE | BMCR_SPEED1000));

	mdelay(1000);
}

static void __init mynet_n750_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(mynet_n750_leds_gpio),
				 mynet_n750_leds_gpio);
	ath79_register_gpio_keys_polled(-1, MYNET_N750_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(mynet_n750_gpio_keys),
					mynet_n750_gpio_keys);
	/*
	 * Control signal for external LNAs 0 and 1
	 * Taken from GPL bootloader source:
	 *   board/ar7240/db12x/alpha_gpio.c
	 */
	ath79_wmac_set_ext_lna_gpio(0, MYNET_N750_GPIO_EXTERNAL_LNA0);
	ath79_wmac_set_ext_lna_gpio(1, MYNET_N750_GPIO_EXTERNAL_LNA1);

	mynet_n750_get_mac("wlan24mac=", tmpmac);
	ath79_register_wmac(art + MYNET_N750_WMAC_CALDATA_OFFSET, tmpmac);

	mynet_n750_get_mac("wlan5mac=", tmpmac);
	ap91_pci_init(art + MYNET_N750_PCIE_CALDATA_OFFSET, tmpmac);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);

	mdiobus_register_board_info(mynet_n750_mdio0_info,
				    ARRAY_SIZE(mynet_n750_mdio0_info));

	ath79_mdio0_data.reset = mynet_n750_mdio_fixup;
	ath79_register_mdio(0, 0x0);

	mynet_n750_get_mac("lanmac=", ath79_eth0_data.mac_addr);

	/* GMAC0 is connected to an AR8327N switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_MYNET_N750, "MYNET-N750", "WD My Net N750",
	     mynet_n750_setup);
