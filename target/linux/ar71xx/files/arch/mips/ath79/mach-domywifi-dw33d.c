/*
 * DomyWifi DW33D support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
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

#define DW33D_GPIO_LED_MMC		4
#define DW33D_GPIO_LED_WLAN_2G		13
#define DW33D_GPIO_LED_STATUS		14
#define DW33D_GPIO_LED_USB		15
#define DW33D_GPIO_LED_INTERNET		22

#define DW33D_GPIO_BTN_RESET		17

#define DW33D_KEYS_POLL_INTERVAL	20	/* msecs */
#define DW33D_KEYS_DEBOUNCE_INTERVAL	(3 * DW33D_KEYS_POLL_INTERVAL)

#define DW33D_MAC0_OFFSET		0
#define DW33D_MAC1_OFFSET		6
#define DW33D_WMAC_OFFSET		12
#define DW33D_WMAC_CALDATA_OFFSET	0x1000
#define DW33D_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led dw33d_leds_gpio[] __initdata = {
	{
		.name		= "dw33d:blue:status",
		.gpio		= DW33D_GPIO_LED_STATUS,
		.active_low	= 1,
	},
	{
		.name		= "dw33d:blue:mmc",
		.gpio		= DW33D_GPIO_LED_MMC,
		.active_low	= 1,
	},
	{
		.name		= "dw33d:blue:usb",
		.gpio		= DW33D_GPIO_LED_USB,
		.active_low	= 1,
	},
	{
		.name		= "dw33d:blue:wlan-2g",
		.gpio		= DW33D_GPIO_LED_WLAN_2G,
		.active_low	= 1,
	},
	{
		.name		= "dw33d:blue:internet",
		.gpio		= DW33D_GPIO_LED_INTERNET,
		.active_low	= 1,
	}
};

static struct gpio_keys_button dw33d_gpio_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = DW33D_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= DW33D_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

/* GMAC6 of the QCA8337 switch is connected to the QCA9558 SoC via SGMII */
static struct ar8327_pad_cfg dw33d_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

/* GMAC0 of the QCA8337 switch is connected to the QCA9558 SoC via RGMII */
static struct ar8327_pad_cfg dw33d_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data dw33d_qca8337_data = {
	.pad0_cfg = &dw33d_qca8337_pad0_cfg,
	.pad6_cfg = &dw33d_qca8337_pad6_cfg,
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
};

static struct mdio_board_info dw33d_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &dw33d_qca8337_data,
	},
};

static void __init dw33d_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(dw33d_leds_gpio),
				 dw33d_leds_gpio);
	ath79_register_gpio_keys_polled(-1, DW33D_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(dw33d_gpio_keys),
					dw33d_gpio_keys);

	ath79_register_usb();
	ath79_nfc_set_ecc_mode(AR934X_NFC_ECC_HW);
	ath79_register_nfc();
	ath79_register_pci();

	ath79_register_wmac(art + DW33D_WMAC_CALDATA_OFFSET, art + DW33D_WMAC_OFFSET);

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + DW33D_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + DW33D_MAC1_OFFSET, 0);

	mdiobus_register_board_info(dw33d_mdio0_info,
				    ARRAY_SIZE(dw33d_mdio0_info));

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

MIPS_MACHINE(ATH79_MACH_DOMYWIFI_DW33D, "DW33D",
	     "DomyWifi DW33D",
	     dw33d_setup);
