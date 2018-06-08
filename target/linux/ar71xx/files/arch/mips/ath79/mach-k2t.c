/*
 * Phicomm k2t support
 *
 * Copyright (c) 2012 Qualcomm Atheros
 * Copyright (c) 2012-2013 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (c) 2018 wheretoteardown <yhk23665@gmail.com>
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

#include <linux/init.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ar71xx_regs.h>
 
#include <linux/ar8216_platform.h>

#include "common.h"
#include "pci.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-wmac.h"
#include "dev-usb.h"
#include "machtypes.h"
#include "nvram.h"



#define K2T_GPIO_LED_BLUE		6
#define K2T_GPIO_LED_RED		7
#define K2T_GPIO_LED_YELLOW		8

#define K2T_GPIO_BTN_RESET		2

#define K2T_KEYS_POLL_INTERVAL	20	/* msecs */
#define K2T_KEYS_DEBOUNCE_INTERVAL	(3 * K2T_KEYS_POLL_INTERVAL)

#define K2T_MAC0_OFFSET		0
#define K2T_MAC1_OFFSET		6
#define K2T_WMAC_OFFSET		12
#define K2T_WMAC_CALDATA_OFFSET	0x1000
#define K2T_PCIE_CALDATA_OFFSET	0x5000

static struct gpio_led k2t_leds_gpio[] __initdata = {
	{
		.name		= "k2t:blue",
		.gpio		= K2T_GPIO_LED_BLUE,
		.active_low	= 1,
	},
	{
		.name		= "k2t:red",
		.gpio		= K2T_GPIO_LED_RED,
		.active_low	= 1,
	},
	{
		.name		= "k2t:yellow",
		.gpio		= K2T_GPIO_LED_YELLOW,
		.active_low	= 1,
	},	
};

static struct gpio_keys_button K2T_GPIO_keys[] __initdata = {
	{
		.desc		= "Reset button",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = K2T_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= K2T_GPIO_BTN_RESET,
		.active_low	= 1,
	}
};

static struct ar8327_pad_cfg k2t_qca8337_pad6_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_pad_cfg k2t_qca8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data k2t_qca8337_data = {
	.pad0_cfg = &k2t_qca8337_pad0_cfg,
	.pad6_cfg = &k2t_qca8337_pad6_cfg,
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

static struct mdio_board_info k2t_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &k2t_qca8337_data,
	},
};

static void __init k2t_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(k2t_leds_gpio),
				 k2t_leds_gpio);
	ath79_register_gpio_keys_polled(-1, K2T_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(K2T_GPIO_keys),
					K2T_GPIO_keys);

	ath79_register_usb();
	ath79_register_pci();

	ath79_register_wmac(art + K2T_WMAC_CALDATA_OFFSET, art + K2T_WMAC_OFFSET);

	ath79_setup_qca956x_eth_cfg(QCA956X_ETH_CFG_RGMII_EN);

	ath79_register_mdio(0, 0x0);

	ath79_init_mac(ath79_eth0_data.mac_addr, art + K2T_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, art + K2T_MAC1_OFFSET, 0);

	mdiobus_register_board_info(k2t_mdio0_info,
				    ARRAY_SIZE(k2t_mdio0_info));

	/* GMAC0 is connected to the RMGII interface */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
        ath79_eth0_pll_data.pll_1000 = 0x56000000;

	ath79_register_eth(0);

	/* GMAC1 is connected tot eh SGMII interface */
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
        ath79_eth1_pll_data.pll_1000 = 0x03000101;

	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_K2T, "K2T", "Phicomm K2T", k2t_setup);
