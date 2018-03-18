
/*
 * Atheros ARCHER_C7 reference board support
 *
 * Copyright (c) 2017 Felix Fietkau <nbd@nbd.name>
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
 * Copyright (c) 2012 Gabor Juhos <juhosg@openwrt.org>
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
#include <linux/ath9k_platform.h>
#include <linux/ar8216_platform.h>
#include <linux/proc_fs.h>
#include <linux/gpio.h>
#include <linux/spi/spi_gpio.h>
#include <linux/spi/74x164.h>

#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-spi.h"
#include "dev-usb.h"
#include "dev-wmac.h"


#define ARCHER_C7_GPIO_SHIFT_OE		1
#define ARCHER_C7_GPIO_SHIFT_SER	14
#define ARCHER_C7_GPIO_SHIFT_SRCLK	15
#define ARCHER_C7_GPIO_SHIFT_RCLK	16
#define ARCHER_C7_GPIO_SHIFT_SRCLR	21

#define ARCHER_C7_GPIO_BTN_RESET	5
#define ARCHER_C7_GPIO_BTN_WPS_WIFI	2

#define ARCHER_C7_GPIO_LED_WLAN5	9
#define ARCHER_C7_GPIO_LED_POWER	6
#define ARCHER_C7_GPIO_LED_USB1		7
#define ARCHER_C7_GPIO_LED_USB2		8

#define ARCHER_C7_74HC_GPIO_BASE	32
#define ARCHER_C7_GPIO_LED_WPS		(ARCHER_C7_74HC_GPIO_BASE + 0)
#define ARCHER_C7_GPIO_LED_LAN1		(ARCHER_C7_74HC_GPIO_BASE + 1)
#define ARCHER_C7_GPIO_LED_LAN2		(ARCHER_C7_74HC_GPIO_BASE + 2)
#define ARCHER_C7_GPIO_LED_LAN3		(ARCHER_C7_74HC_GPIO_BASE + 3)
#define ARCHER_C7_GPIO_LED_LAN4		(ARCHER_C7_74HC_GPIO_BASE + 4)
#define ARCHER_C7_GPIO_LED_WAN_GREEN	(ARCHER_C7_74HC_GPIO_BASE + 5)
#define ARCHER_C7_GPIO_LED_WAN_AMBER	(ARCHER_C7_74HC_GPIO_BASE + 6)
#define ARCHER_C7_GPIO_LED_WLAN2	(ARCHER_C7_74HC_GPIO_BASE + 7)

#define ARCHER_C7_KEYS_POLL_INTERVAL        20     /* msecs */
#define ARCHER_C7_KEYS_DEBOUNCE_INTERVAL    (3 * ARCHER_C7_KEYS_POLL_INTERVAL)

#define ARCHER_C7_MAC0_OFFSET               0
#define ARCHER_C7_MAC1_OFFSET               6
#define ARCHER_C7_WMAC_CALDATA_OFFSET       0x1000

#define ARCHER_C7_GPIO_MDC			3
#define ARCHER_C7_GPIO_MDIO			4

static struct spi_gpio_platform_data archer_c7_v4_spi_data = {
	.sck		= ARCHER_C7_GPIO_SHIFT_SRCLK,
	.miso		= SPI_GPIO_NO_MISO,
	.mosi		= ARCHER_C7_GPIO_SHIFT_SER,
	.num_chipselect	= 1,
};

static u8 archer_c7_v4_ssr_initdata = 0xff;

static struct gen_74x164_chip_platform_data archer_c7_v4_ssr_data = {
	.base = ARCHER_C7_74HC_GPIO_BASE,
	.num_registers = 1,
	.init_data = &archer_c7_v4_ssr_initdata,
};

static struct platform_device archer_c7_v4_spi_device = {
	.name		= "spi_gpio",
	.id		= 1,
	.dev = {
		.platform_data = &archer_c7_v4_spi_data,
	},
};

static struct spi_board_info archer_c7_v4_spi_info[] = {
	{
		.bus_num		= 1,
		.chip_select		= 0,
		.max_speed_hz		= 10000000,
		.modalias		= "74x164",
		.platform_data		= &archer_c7_v4_ssr_data,
		.controller_data	= (void *) ARCHER_C7_GPIO_SHIFT_RCLK,
	},
};

static struct gpio_led archer_c7_v4_leds_gpio[] __initdata = {
	{
		.name		= "archer-c7-v4:green:power",
		.gpio		= ARCHER_C7_GPIO_LED_POWER,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:wps",
		.gpio		= ARCHER_C7_GPIO_LED_WPS,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:wlan2g",
		.gpio		= ARCHER_C7_GPIO_LED_WLAN2,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:wlan5g",
		.gpio		= ARCHER_C7_GPIO_LED_WLAN5,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:lan1",
		.gpio		= ARCHER_C7_GPIO_LED_LAN1,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:lan2",
		.gpio		= ARCHER_C7_GPIO_LED_LAN2,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:lan3",
		.gpio		= ARCHER_C7_GPIO_LED_LAN3,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:lan4",
		.gpio		= ARCHER_C7_GPIO_LED_LAN4,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:wan",
		.gpio		=  ARCHER_C7_GPIO_LED_WAN_GREEN,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:amber:wan",
		.gpio		=  ARCHER_C7_GPIO_LED_WAN_AMBER,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:usb1",
		.gpio		=  ARCHER_C7_GPIO_LED_USB1,
		.active_low	= 1,
	}, {
		.name		= "archer-c7-v4:green:usb2",
		.gpio		=  ARCHER_C7_GPIO_LED_USB2,
		.active_low	= 1,
	},
};

static struct gpio_keys_button archer_c7_v4_gpio_keys[] __initdata = {
        {
                .desc           = "WPS and WIFI button",
                .type           = EV_KEY,
                .code           = KEY_WPS_BUTTON,
                .debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
                .gpio           = ARCHER_C7_GPIO_BTN_WPS_WIFI,
                .active_low     = 1,
        },
        {
                .desc           = "Reset button",
                .type           = EV_KEY,
                .code           = KEY_RESTART,
                .debounce_interval = ARCHER_C7_KEYS_DEBOUNCE_INTERVAL,
                .gpio           = ARCHER_C7_GPIO_BTN_RESET,
                .active_low     = 1,
        },
};

static struct ar8327_pad_cfg archer_c7_v4_ar8337_pad0_cfg = {
	.mode = AR8327_PAD_MAC_SGMII,
	.sgmii_delay_en = true,
};

static struct ar8327_platform_data archer_c7_v4_ar8337_data = {
	.pad0_cfg = &archer_c7_v4_ar8337_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info archer_c7_v4_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.phy_addr = 0,
		.platform_data = &archer_c7_v4_ar8337_data,
	},
};


static void __init archer_c7_v4_setup(void)
{
	u8 *art = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac = (u8 *) KSEG1ADDR(0x1ff00008);

	ath79_register_m25p80(NULL);

	spi_register_board_info(archer_c7_v4_spi_info,
				ARRAY_SIZE(archer_c7_v4_spi_info));

	platform_device_register(&archer_c7_v4_spi_device);

	gpio_request_one(ARCHER_C7_GPIO_SHIFT_OE,
			 GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
			 "LED control");

	gpio_request_one(ARCHER_C7_GPIO_SHIFT_SRCLR,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LED reset");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(archer_c7_v4_leds_gpio),
				archer_c7_v4_leds_gpio);

	ath79_register_gpio_keys_polled(-1, ARCHER_C7_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(archer_c7_v4_gpio_keys),
					archer_c7_v4_gpio_keys);

	ath79_register_usb();

	ath79_gpio_output_select(ARCHER_C7_GPIO_MDC, QCA956X_GPIO_OUT_MUX_GE0_MDC);
	ath79_gpio_output_select(ARCHER_C7_GPIO_MDIO, QCA956X_GPIO_OUT_MUX_GE0_MDO);

	ath79_register_mdio(0, 0x0);

	mdiobus_register_board_info(archer_c7_v4_mdio0_info,
				    ARRAY_SIZE(archer_c7_v4_mdio0_info));

	ath79_register_wmac(art + ARCHER_C7_WMAC_CALDATA_OFFSET, mac);
	ath79_register_pci();

	/* GMAC0 is connected to an AR8337 switch */
	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth0_data.speed = SPEED_1000;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_ARCHER_C7_V4, "ARCHER-C7-V4", "TP-LINK Archer C7 v4",
	     archer_c7_v4_setup);
