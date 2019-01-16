/*
 * Arris sbr-ac1750 support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2019 Mleaf <mleaf90@gmail.com>
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
#include <linux/mtd/mtd.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0)
#include <linux/mtd/nand.h>
#else
#include <linux/mtd/rawnand.h>
#endif
#include <linux/platform/ar934x_nfc.h>

#include <asm/mach-ath79/ar71xx_regs.h>
#include <linux/gpio.h>
#include <linux/delay.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-gpio-buttons.h"
#include "dev-eth.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-nfc.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define SBR_AC1750_GPIO_LED_WLAN_2G		2
#define SBR_AC1750_GPIO_LED_WLAN_5G		21
#define SBR_AC1750_GPIO_LED_WPS		    23
#define SBR_AC1750_GPIO_LED_USB		    22

#define SBR_AC1750_GPIO_BTN_RESET		17
#define SBR_AC1750_GPIO_BTN_WPS		    19

#define SBR_AC1750_KEYS_POLL_INTERVAL	20	/* msecs */
#define SBR_AC1750_KEYS_DEBOUNCE_INTERVAL	(3 * SBR_AC1750_KEYS_POLL_INTERVAL)

#define SBR_AC1750_MAC0_OFFSET		0
#define SBR_AC1750_MAC1_OFFSET		6
#define SBR_AC1750_WMAC_OFFSET		12
#define SBR_AC1750_WMAC_CALDATA_OFFSET	0x1000
#define SBR_AC1750_PCIE_CALDATA_OFFSET	0x5000

#define SBR_AC1750_EXT_WDT_TIMEOUT_MS	200
#define SBR_AC1750_GPIO_EXT_WDT		18

static struct timer_list gpio_wdt_timer;

static struct gpio_led sbr_ac1750_leds_gpio[] __initdata = {
	{
		.name		= "sbr-ac1750:blue:wlan-2g",
		.gpio		= SBR_AC1750_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "sbr-ac1750:blue:wlan-5g",
		.gpio		= SBR_AC1750_GPIO_LED_WLAN_5G,
		.active_low	= 1,
	},
	{
		.name		= "sbr-ac1750:blue:wps",
		.gpio		= SBR_AC1750_GPIO_LED_WPS,
		.active_low	= 1,
	},
	{
		.name		= "sbr-ac1750:blue:usb",
		.gpio		= SBR_AC1750_GPIO_LED_USB,
		.active_low	= 1,
	}
};

static struct gpio_keys_button sbr_ac1750_gpio_keys[] __initdata = {
	{
		.desc		= "WPS button",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = SBR_AC1750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= SBR_AC1750_GPIO_BTN_WPS,
		.active_low	= 1,
	},
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = SBR_AC1750_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= SBR_AC1750_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

/* GMAC6 of the QCA8337 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg sbr_ac1750_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

/* GMAC0 of the QCA8337 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg sbr_ac1750_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg sbr_ac1750_ar8327_led_cfg = {
	.led_ctrl0 = 0x0000cc35,
	.led_ctrl1 = 0x0000ca35,
	.led_ctrl2 = 0x0000c935,
	.led_ctrl3 = 0x03ffff00,
	.open_drain = true,
};

static struct ar8327_platform_data sbr_ac1750_qca8337_data = {
	.pad0_cfg = &sbr_ac1750_qca8337_pad0_cfg,
	.pad6_cfg = &sbr_ac1750_qca8337_pad6_cfg,
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
	.led_cfg = &sbr_ac1750_ar8327_led_cfg,
};

static struct mdio_board_info sbr_ac1750_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &sbr_ac1750_qca8337_data,
	},
};

/*
 * SBR_AC1750 devices include external hardware watchdog chip,
 * watchdog chip connected to a selected GPIO
 * and WiSoC RESET_L input. Watchdog time-out is ~1.6 s.
 */
static void gpio_wdt_toggle(unsigned long gpio)
{
	static int state;

	state = !state;
	gpio_set_value(gpio, state);

	mod_timer(&gpio_wdt_timer,
		  jiffies + msecs_to_jiffies(SBR_AC1750_EXT_WDT_TIMEOUT_MS));
}

static void init_sbr_ac1750_wdt(int gpio_wdt){
	
	if (gpio_wdt >= 0) {
		gpio_request_one(gpio_wdt, GPIOF_OUT_INIT_HIGH, "watchdog");
		gpio_set_value(gpio_wdt, 0);
		ndelay(1000);
		gpio_set_value(gpio_wdt, 1);
		setup_timer(&gpio_wdt_timer, gpio_wdt_toggle, gpio_wdt);
		gpio_wdt_toggle(gpio_wdt);
	}
}

static void __init sbr_ac1750_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	init_sbr_ac1750_wdt(SBR_AC1750_GPIO_EXT_WDT);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(sbr_ac1750_leds_gpio),
				 sbr_ac1750_leds_gpio);
	ath79_register_gpio_keys_polled(-1, SBR_AC1750_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(sbr_ac1750_gpio_keys),
					sbr_ac1750_gpio_keys);

	ath79_register_usb();
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();

	ath79_register_pci();
	ath79_register_wmac_simple();

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + SBR_AC1750_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + SBR_AC1750_MAC1_OFFSET, 0);

	mdiobus_register_board_info(sbr_ac1750_mdio0_info,
				    ARRAY_SIZE(sbr_ac1750_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
        ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_register_eth(0);

	/* GMAC1 is connected tot eh SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
        ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_ARRIS_SBR_AC1750, "SBR-AC1750",
	     "ARRIS SBR-AC1750",
	     sbr_ac1750_setup);
