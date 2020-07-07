/*
 * TP-LINK Archer C5/C7/TL-WDR4900 v2 board support
 *
 * Copyright (c) 2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2014 施康成 <tenninjas@tenninjas.ca>
 * Copyright (c) 2014 Imre Kaloz <kaloz@openwrt.org>
 *
 * Based on the Qualcomm Atheros AP135/AP136 reference board support code
 *   Copyright (c) 2012 Qualcomm Atheros
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
#include "pci.h"

#define ARCHER_C7_GPIO_LED_WLAN2G	12
#define ARCHER_C7_GPIO_LED_SYSTEM	14
#define ARCHER_C7_GPIO_LED_QSS		15
#define ARCHER_C7_GPIO_LED_WLAN5G	17
#define ARCHER_C7_GPIO_LED_USB1		18
#define ARCHER_C7_GPIO_LED_USB2		19

#define ARCHER_C7_GPIO_BTN_RFKILL	23
#define ARCHER_C7_V2_GPIO_BTN_RFKILL	23
#define ARCHER_C7_GPIO_BTN_RESET	16

#define ARCHER_C7_GPIO_USB1_POWER	22
#define ARCHER_C7_GPIO_USB2_POWER	21

#define ARCHER_C7_KEYS_POLL_INTERVAL	20	/* msecs */
#define ARCHER_C7_KEYS_DEBOUNCE_INTERVAL (3 * ARCHER_C7_KEYS_POLL_INTERVAL)

#define ARCHER_C7_WMAC_CALDATA_OFFSET	0x1000
#define ARCHER_C7_PCIE_CALDATA_OFFSET	0x5000

static const char *archer_c7_part_probes[] = {
	"tp-link",
	NULL,
};

static struct flash_platform_data archer_c7_flash_data = {
	.part_probes	= archer_c7_part_probes,
};

static struct gpio_led archer_c7_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:green:qss",
		.gpio		= ARCHER_C7_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:system",
		.gpio		= ARCHER_C7_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan2g",
		.gpio		= ARCHER_C7_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:wlan5g",
		.gpio		= ARCHER_C7_GPIO_LED_WLAN5G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb1",
		.gpio		= ARCHER_C7_GPIO_LED_USB1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb2",
		.gpio		= ARCHER_C7_GPIO_LED_USB2,
		.active_low	= 1,
	},
};

static struct gpio_led wdr4900_leds_gpio[] __initdata = {
	{
		.name		= "tp-link:blue:qss",
		.gpio		= ARCHER_C7_GPIO_LED_QSS,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:blue:system",
		.gpio		= ARCHER_C7_GPIO_LED_SYSTEM,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:blue:wlan2g",
		.gpio		= ARCHER_C7_GPIO_LED_WLAN2G,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb1",
		.gpio		= ARCHER_C7_GPIO_LED_USB1,
		.active_low	= 1,
	},
	{
		.name		= "tp-link:green:usb2",
		.gpio		= ARCHER_C7_GPIO_LED_USB2,
		.active_low	= 1,
	},
};

static struct gpio_keys_button archer_c7_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARCHER_C7_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARCHER_C7_GPIO_BTN_RFKILL,
	},
};

static struct gpio_keys_button archer_c7_v2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARCHER_C7_GPIO_BTN_RESET,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL switch",
		.type		= EV_SW,
		.code		= KEY_RFKILL,
		.debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARCHER_C7_V2_GPIO_BTN_RFKILL,
	},
};

static struct gpio_keys_button wdr4900_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= ARCHER_C7_GPIO_BTN_RESET,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info archer_c7_leds_ar8327[] = {
	AR8327_LED_INFO(PHY0_0, HW, "tp-link:green:wan"),
	AR8327_LED_INFO(PHY1_0, HW, "tp-link:green:lan1"),
	AR8327_LED_INFO(PHY2_0, HW, "tp-link:green:lan2"),
	AR8327_LED_INFO(PHY3_0, HW, "tp-link:green:lan3"),
	AR8327_LED_INFO(PHY4_0, HW, "tp-link:green:lan4"),
};

/* GMAC0 of the AR8327 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg archer_c7_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC6 of the AR8327 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg archer_c7_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg archer_c7_ar8327_led_cfg = {
	.led_ctrl0 = 0xc737c737,
	.led_ctrl1 = 0x00000000,
	.led_ctrl2 = 0x00000000,
	.led_ctrl3 = 0x0030c300,
	.open_drain = false,
};

static struct ar8327_platform_data archer_c7_ar8327_data = {
	.pad0_cfg = &archer_c7_ar8327_pad0_cfg,
	.pad6_cfg = &archer_c7_ar8327_pad6_cfg,
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
	.led_cfg = &archer_c7_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(archer_c7_leds_ar8327),
	.leds = archer_c7_leds_ar8327,
};

static struct mdio_board_info archer_c7_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &archer_c7_ar8327_data,
	},
};

static void __init common_setup(bool pcie_slot)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1f01fc00);
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 tmpmac[ETH_ALEN];
	u8 tmpmac2[ETH_ALEN];

	ath79_register_m25p80(&archer_c7_flash_data);

	if (pcie_slot) {
		ath79_register_wmac(art + ARCHER_C7_WMAC_CALDATA_OFFSET, mac);
		ath79_register_pci();
	} else {
		ath79_init_mac(tmpmac, mac, -1);
		ath79_register_wmac(art + ARCHER_C7_WMAC_CALDATA_OFFSET, tmpmac);

		ath79_init_mac(tmpmac2, mac, -2);
		ap9x_pci_setup_wmac_led_pin(0, 0);
		ap91_pci_init(art + ARCHER_C7_PCIE_CALDATA_OFFSET, tmpmac2);
	}

	mdiobus_register_board_info(archer_c7_mdio0_info,
				    ARRAY_SIZE(archer_c7_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 1);
	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, mac, 0);
	ath79_register_eth(1);

	gpio_request_one(ARCHER_C7_GPIO_USB1_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB1 power");
	gpio_request_one(ARCHER_C7_GPIO_USB2_POWER,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "USB2 power");
	ath79_register_usb();
}

static void __init archer_c5_setup(void)
{
	ath79_register_gpio_keys_polled(-1, ARCHER_C7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c7_gpio_keys),
					archer_c7_gpio_keys);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c7_leds_gpio),
				 archer_c7_leds_gpio);
	common_setup(true);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C5, "ARCHER-C5", "TP-LINK Archer C5",
	     archer_c5_setup);

static void __init archer_c7_setup(void)
{
	ath79_register_gpio_keys_polled(-1, ARCHER_C7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c7_gpio_keys),
					archer_c7_gpio_keys);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c7_leds_gpio),
				 archer_c7_leds_gpio);
	common_setup(true);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C7, "ARCHER-C7", "TP-LINK Archer C7",
	     archer_c7_setup);

static void __init archer_c7_v2_setup(void)
{
	ath79_register_gpio_keys_polled(-1, ARCHER_C7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c7_v2_gpio_keys),
					archer_c7_v2_gpio_keys);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c7_leds_gpio),
				 archer_c7_leds_gpio);
	common_setup(true);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C7_V2, "ARCHER-C7-V2", "TP-LINK Archer C7",
	     archer_c7_v2_setup);

static void __init tl_wdr4900_v2_setup(void)
{
	ath79_register_gpio_keys_polled(-1, ARCHER_C7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wdr4900_gpio_keys),
					wdr4900_gpio_keys);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(wdr4900_leds_gpio),
				 wdr4900_leds_gpio);
	common_setup(false);
}

MIPS_MACHINE(ATH79_MACH_TL_WDR4900_V2, "TL-WDR4900-v2", "TP-LINK TL-WDR4900 v2",
	     tl_wdr4900_v2_setup)

