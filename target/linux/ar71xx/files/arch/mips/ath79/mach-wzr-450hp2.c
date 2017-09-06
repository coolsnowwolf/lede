/*
 * Buffalo WZR-450HP2 board support
 *
 * Copyright (c) 2013 Gabor Juhos <juhosg@openwrt.org>
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

#include <linux/phy.h>
#include <linux/gpio.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/platform_device.h>
#include <linux/ar8216_platform.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define WZR_450HP2_KEYS_POLL_INTERVAL	20	/* msecs */
#define WZR_450HP2_KEYS_DEBOUNCE_INTERVAL (3 * WZR_450HP2_KEYS_POLL_INTERVAL)

#define WZR_450HP2_WMAC_CALDATA_OFFSET	0x1000

static struct mtd_partition wzrhpg450h_partitions[] = {
	{
		.name		= "u-boot",
		.offset		= 0,
		.size		= 0x0040000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "u-boot-env",
		.offset		= 0x0040000,
		.size		= 0x0010000,
	}, {
		.name		= "ART",
		.offset		= 0x0ff0000,
		.size		= 0x0010000,
		.mask_flags	= MTD_WRITEABLE,
	}, {
		.name		= "firmware",
		.offset		= 0x0050000,
		.size		= 0x0f90000,
	}, {
		.name		= "user_property",
		.offset		= 0x0fe0000,
		.size		= 0x0010000,
	}
};

static struct flash_platform_data wzr_450hp2_flash_data = {
	.parts		= wzrhpg450h_partitions,
	.nr_parts	= ARRAY_SIZE(wzrhpg450h_partitions),
};

static struct gpio_led wzr_450hp2_leds_gpio[] __initdata = {
	{
		.name		= "buffalo:green:wps",
		.gpio		= 3,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:system",
		.gpio		= 20,
		.active_low	= 1,
	},
	{
		.name		= "buffalo:green:wlan",
		.gpio		= 18,
		.active_low	= 1,
	},
};

static struct gpio_keys_button wzr_450hp2_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = WZR_450HP2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 17,
		.active_low	= 1,
	},
	{
		.desc		= "RFKILL button",
		.type		= EV_KEY,
		.code		= KEY_RFKILL,
		.debounce_interval = WZR_450HP2_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 21,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info wzr_450hp2_leds_ar8327[] = {
	AR8327_LED_INFO(PHY0_0, HW, "buffalo:green:lan1"),
	AR8327_LED_INFO(PHY1_0, HW, "buffalo:green:lan2"),
	AR8327_LED_INFO(PHY2_0, HW, "buffalo:green:lan3"),
	AR8327_LED_INFO(PHY3_0, HW, "buffalo:green:lan4"),
	AR8327_LED_INFO(PHY4_0, HW, "buffalo:green:wan"),
};

/* GMAC0 of the AR8327 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg wzr_450hp2_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC6 of the AR8327 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg wzr_450hp2_ar8327_pad6_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_led_cfg wzr_450hp2_ar8327_led_cfg = {
	.led_ctrl0 = 0xcc35cc35,
	.led_ctrl1 = 0xca35ca35,
	.led_ctrl2 = 0xc935c935,
	.led_ctrl3 = 0x03ffff00,
	.open_drain = true,
};

static struct ar8327_platform_data wzr_450hp2_ar8327_data = {
	.pad0_cfg = &wzr_450hp2_ar8327_pad0_cfg,
	.pad6_cfg = &wzr_450hp2_ar8327_pad6_cfg,
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
	.led_cfg = &wzr_450hp2_ar8327_led_cfg,
	.num_leds = ARRAY_SIZE(wzr_450hp2_leds_ar8327),
	.leds = wzr_450hp2_leds_ar8327,
};

static struct mdio_board_info wzr_450hp2_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &wzr_450hp2_ar8327_data,
	},
};

static void __init wzr_450hp2_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac_wan = art;
	u8 *mac_lan = mac_wan + ETH_ALEN;

	ath79_register_m25p80(&wzr_450hp2_flash_data);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(wzr_450hp2_leds_gpio),
				 wzr_450hp2_leds_gpio);
	ath79_register_gpio_keys_polled(-1, WZR_450HP2_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(wzr_450hp2_gpio_keys),
					wzr_450hp2_gpio_keys);

	ath79_register_wmac(art + WZR_450HP2_WMAC_CALDATA_OFFSET, mac_lan);

	mdiobus_register_board_info(wzr_450hp2_mdio0_info,
				    ARRAY_SIZE(wzr_450hp2_mdio0_info));
	ath79_register_mdio(0, 0x0);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_init_mac(ath79_eth0_data.mac_addr, mac_wan, 0);
	ath79_register_eth(0);

	/* GMAC1 is connected to the SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_init_mac(ath79_eth1_data.mac_addr, mac_lan, 0);
	ath79_register_eth(1);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_WZR_450HP2, "WZR-450HP2",
	     "Buffalo WZR-450HP2", wzr_450hp2_setup);

