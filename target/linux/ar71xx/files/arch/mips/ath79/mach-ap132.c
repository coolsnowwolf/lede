/*
 * Atheros AP132 reference board support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2013 Embedded Wireless GmbH <info@embeddedwireless.de>
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

#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define AP132_GPIO_LED_USB		4
#define AP132_GPIO_LED_WLAN_5G		12
#define AP132_GPIO_LED_WLAN_2G		13
#define AP132_GPIO_LED_STATUS_RED	14
#define AP132_GPIO_LED_WPS_RED		15

#define AP132_GPIO_BTN_WPS		16

#define AP132_KEYS_POLL_INTERVAL	20	/* msecs */
#define AP132_KEYS_DEBOUNCE_INTERVAL	(3 * AP132_KEYS_POLL_INTERVAL)

#define AP132_MAC0_OFFSET		0
#define AP132_WMAC_CALDATA_OFFSET	0x1000

static struct gpio_led ap132_leds_gpio[] __initdata = {
	{
		.name		= "ap132:red:status",
		.gpio		= AP132_GPIO_LED_STATUS_RED,
		.active_low	= 1,
	},
	{
		.name		= "ap132:red:wps",
		.gpio		= AP132_GPIO_LED_WPS_RED,
		.active_low	= 1,
	},
	{
		.name		= "ap132:red:wlan-2g",
		.gpio		= AP132_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "ap132:red:usb",
		.gpio		= AP132_GPIO_LED_USB,
		.active_low	= 1,
	}
};

static struct gpio_keys_button ap132_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = AP132_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= AP132_GPIO_BTN_WPS,
		.active_low	= 1,
	},
};

static struct ar8327_pad_cfg ap132_ar8327_pad0_cfg;

static struct ar8327_platform_data ap132_ar8327_data = {
	.pad0_cfg = &ap132_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info ap132_mdio1_info[] = {
	{
		.bus_id = "ag71xx-mdio.1",
		.phy_addr = 0,
		.platform_data = &ap132_ar8327_data,
	},
};

static void __init ap132_mdio_setup(void)
{
	void __iomem *base;
	u32 t;

#define GPIO_IN_ENABLE3_ADDRESS                                      0x0050
#define GPIO_IN_ENABLE3_MII_GE1_MDI_MASK                             0x00ff0000
#define GPIO_IN_ENABLE3_MII_GE1_MDI_LSB                              16
#define GPIO_IN_ENABLE3_MII_GE1_MDI_SET(x)                           (((x) << GPIO_IN_ENABLE3_MII_GE1_MDI_LSB) & GPIO_IN_ENABLE3_MII_GE1_MDI_MASK)
#define GPIO_OUT_FUNCTION4_ADDRESS                                   0x003c
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_MASK                       0xff000000
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_LSB                        24
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_SET(x)                     (((x) << GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_LSB) & GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_MASK)
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_MASK                       0x0000ff00
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_LSB                        8
#define GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_SET(x)                     (((x) << GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_LSB) & GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_MASK)

	base = ioremap(AR71XX_GPIO_BASE, AR71XX_GPIO_SIZE);

	t = __raw_readl(base + GPIO_IN_ENABLE3_ADDRESS);
	t &= ~GPIO_IN_ENABLE3_MII_GE1_MDI_MASK;
	t |= GPIO_IN_ENABLE3_MII_GE1_MDI_SET(19);
	__raw_writel(t, base + GPIO_IN_ENABLE3_ADDRESS);


	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << 19), base + AR71XX_GPIO_REG_OE);

	__raw_writel(__raw_readl(base + AR71XX_GPIO_REG_OE) & ~(1 << 17), base + AR71XX_GPIO_REG_OE);


	t = __raw_readl(base + GPIO_OUT_FUNCTION4_ADDRESS);
	t &= ~(GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_MASK | GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_MASK);
	t |= GPIO_OUT_FUNCTION4_ENABLE_GPIO_19_SET(0x20) | GPIO_OUT_FUNCTION4_ENABLE_GPIO_17_SET(0x21);
	__raw_writel(t, base + GPIO_OUT_FUNCTION4_ADDRESS);

	iounmap(base);

}

static void __init ap132_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ap132_leds_gpio),
				 ap132_leds_gpio);
	ath79_register_gpio_keys_polled(-1, AP132_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ap132_gpio_keys),
					ap132_gpio_keys);

	ath79_register_usb();

	ath79_register_wmac(art + AP132_WMAC_CALDATA_OFFSET, NULL);

	/* GMAC0 of the AR8327 switch is connected to GMAC1 via SGMII */
	ap132_ar8327_pad0_cfg.mode = AR8327_PAD_MAC_SGMII;
	ap132_ar8327_pad0_cfg.sgmii_delay_en = true;

	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ap132_mdio_setup();

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, art + AP132_MAC0_OFFSET, 0);

	mdiobus_register_board_info(ap132_mdio1_info,
				    ARRAY_SIZE(ap132_mdio1_info));

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_data.phy_mask = BIT(0);
	ath79_eth1_data.mii_bus_dev = &ath79_mdio1_device.dev;

	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_AP132, "AP132",
	     "Atheros AP132 reference board",
	     ap132_setup);

