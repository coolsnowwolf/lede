/*
 *  MikroTik SPI-NOR RouterBOARDs support
 *
 *  - MikroTik RouterBOARD mAP 2nD
 *  - MikroTik RouterBOARD mAP L-2nD
 *  - MikroTik RouterBOARD 911-2Hn (911 Lite2)
 *  - MikroTik RouterBOARD 911-5Hn (911 Lite5)
 *  - MikroTik RouterBOARD 941L-2nD
 *  - MikroTik RouterBOARD 951Ui-2nD
 *  - MikroTik RouterBOARD 952Ui-5ac2nD
 *  - MikroTik RouterBOARD 962UiGS-5HacT2HnT
 *  - MikroTik RouterBOARD 750UP r2
 *  - MikroTik RouterBOARD 750P-PBr2
 *  - MikroTik RouterBOARD 750 r2
 *  - MikroTik RouterBOARD LHG 5nD
 *  - MikroTik RouterBOARD wAP2nD
 *  - MikroTik RouterBOARD wAP G-5HacT2HnDwAP (wAP AC)
 *
 *  Preliminary support for the following hardware
 *  - MikroTik RouterBOARD cAP2nD
 *  Furthermore, the cAP lite (cAPL2nD) appears to feature the exact same
 *  hardware as the mAP L-2nD. It is unknown if they share the same board
 *  identifier.
 *
 *  Copyright (C) 2017 Thibaut VARENE <varenet@parisc-linux.org>
 *  Copyright (C) 2016 David Hutchison <dhutchison@bluemesh.net>
 *  Copyright (C) 2017 Ryan Mounce <ryan@mounce.com.au>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/routerboot.h>
#include <linux/gpio.h>

#include <linux/spi/spi.h>
#include <linux/spi/74x164.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>

#include <linux/ar8216_platform.h>
#include <linux/platform_data/phy-at803x.h>
#include <linux/platform_data/mdio-gpio.h>

#include <asm/prom.h>
#include <asm/mach-ath79/ar71xx_regs.h>
#include <asm/mach-ath79/ath79.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-spi.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"
#include "pci.h"
#include "routerboot.h"

#define RBSPI_KEYS_POLL_INTERVAL 20 /* msecs */
#define RBSPI_KEYS_DEBOUNCE_INTERVAL (3 * RBSPI_KEYS_POLL_INTERVAL)

#define RBSPI_HAS_USB		BIT(0)
#define RBSPI_HAS_WLAN0		BIT(1)
#define RBSPI_HAS_WLAN1		BIT(2)
#define RBSPI_HAS_WAN4		BIT(3)	/* has WAN port on PHY4 */
#define RBSPI_HAS_SSR		BIT(4)	/* has an SSR on SPI bus 0 */
#define RBSPI_HAS_POE		BIT(5)
#define RBSPI_HAS_MDIO1		BIT(6)
#define RBSPI_HAS_PCI		BIT(7)

#define RB_ROUTERBOOT_OFFSET    0x0000
#define RB_BIOS_SIZE            0x1000
#define RB_SOFT_CFG_SIZE        0x1000

/* Flash partitions indexes */
enum {
	RBSPI_PART_RBOOT,
	RBSPI_PART_HCONF,
	RBSPI_PART_BIOS,
	RBSPI_PART_RBOOT2,
	RBSPI_PART_SCONF,
	RBSPI_PART_FIRMW,
	RBSPI_PARTS
};

static struct mtd_partition rbspi_spi_partitions[RBSPI_PARTS];

/*
 * Setup the SPI flash partition table based on initial parsing.
 * The kernel can be at any aligned position and have any size.
 */
static void __init rbspi_init_partitions(const struct rb_info *info)
{
	struct mtd_partition *parts = rbspi_spi_partitions;
	memset(parts, 0x0, sizeof(*parts));

	parts[RBSPI_PART_RBOOT].name = "routerboot";
	parts[RBSPI_PART_RBOOT].offset = RB_ROUTERBOOT_OFFSET;
	parts[RBSPI_PART_RBOOT].size = info->hard_cfg_offs;
	parts[RBSPI_PART_RBOOT].mask_flags = MTD_WRITEABLE;

	parts[RBSPI_PART_HCONF].name = "hard_config";
	parts[RBSPI_PART_HCONF].offset = info->hard_cfg_offs;
	parts[RBSPI_PART_HCONF].size = info->hard_cfg_size;
	parts[RBSPI_PART_HCONF].mask_flags = MTD_WRITEABLE;

	parts[RBSPI_PART_BIOS].name = "bios";
	parts[RBSPI_PART_BIOS].offset = info->hard_cfg_offs
					+ info->hard_cfg_size;
	parts[RBSPI_PART_BIOS].size = RB_BIOS_SIZE;
	parts[RBSPI_PART_BIOS].mask_flags = MTD_WRITEABLE;

	parts[RBSPI_PART_RBOOT2].name = "routerboot2";
	parts[RBSPI_PART_RBOOT2].offset = parts[RBSPI_PART_BIOS].offset
					+ RB_BIOS_SIZE;
	parts[RBSPI_PART_RBOOT2].size = info->soft_cfg_offs
					- parts[RBSPI_PART_RBOOT2].offset;
	parts[RBSPI_PART_RBOOT2].mask_flags = MTD_WRITEABLE;

	parts[RBSPI_PART_SCONF].name = "soft_config";
	parts[RBSPI_PART_SCONF].offset = info->soft_cfg_offs;
	parts[RBSPI_PART_SCONF].size = RB_SOFT_CFG_SIZE;

	parts[RBSPI_PART_FIRMW].name = "firmware";
	parts[RBSPI_PART_FIRMW].offset = parts[RBSPI_PART_SCONF].offset
					+ parts[RBSPI_PART_SCONF].size;
	parts[RBSPI_PART_FIRMW].size = MTDPART_SIZ_FULL;
}

static struct flash_platform_data rbspi_spi_flash_data = {
	.parts = rbspi_spi_partitions,
	.nr_parts = ARRAY_SIZE(rbspi_spi_partitions),
};

/*
 * Several boards only have a single reset button, use a common
 * structure for that.
 */
static struct gpio_keys_button rbspi_gpio_keys_reset[] __initdata = {
	{
		.desc = "Reset button",
		.type = EV_KEY,
		.code = KEY_RESTART,
		.debounce_interval = RBSPI_KEYS_DEBOUNCE_INTERVAL,
		.gpio = -ENOENT, /* filled dynamically */
		.active_low = 1,
	},
};

/* RB mAP L-2nD gpios */
#define RBMAPL_GPIO_LED_POWER	17
#define RBMAPL_GPIO_LED_USER	14
#define RBMAPL_GPIO_LED_ETH	4
#define RBMAPL_GPIO_LED_WLAN	11
#define RBMAPL_GPIO_BTN_RESET	16

static struct gpio_led rbmapl_leds[] __initdata = {
	{
		.name = "rb:green:power",
		.gpio = RBMAPL_GPIO_LED_POWER,
		.active_low = 0,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name = "rb:green:user",
		.gpio = RBMAPL_GPIO_LED_USER,
		.active_low = 0,
	}, {
		.name = "rb:green:eth",
		.gpio = RBMAPL_GPIO_LED_ETH,
		.active_low = 0,
	}, {
		.name = "rb:green:wlan",
		.gpio = RBMAPL_GPIO_LED_WLAN,
		.active_low = 0,
	},
};

/* RB 941L-2nD gpios */
#define RBHAPL_GPIO_LED_USER   14
#define RBHAPL_GPIO_BTN_RESET	16

static struct gpio_led rbhapl_leds[] __initdata = {
	{
		.name = "rb:green:user",
		.gpio = RBHAPL_GPIO_LED_USER,
		.active_low = 1,
	},
};

/* common RB SSRs */
#define RBSPI_SSR_GPIO_BASE	40
#define RBSPI_SSR_GPIO(bit)	(RBSPI_SSR_GPIO_BASE + (bit))

/* RB 951Ui-2nD gpios */
#define RB952_SSR_BIT_LED_LAN1	0
#define RB952_SSR_BIT_LED_LAN2	1
#define RB952_SSR_BIT_LED_LAN3	2
#define RB952_SSR_BIT_LED_LAN4	3
#define RB952_SSR_BIT_LED_LAN5	4
#define RB952_SSR_BIT_USB_POWER	5
#define RB952_SSR_BIT_LED_WLAN	6
#define RB952_GPIO_SSR_CS	11
#define RB952_GPIO_LED_USER	4
#define RB952_GPIO_POE_POWER	14
#define RB952_GPIO_POE_STATUS	12
#define RB952_GPIO_BTN_RESET	16
#define RB952_GPIO_USB_POWER	RBSPI_SSR_GPIO(RB952_SSR_BIT_USB_POWER)
#define RB952_GPIO_LED_LAN1	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_LAN1)
#define RB952_GPIO_LED_LAN2	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_LAN2)
#define RB952_GPIO_LED_LAN3	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_LAN3)
#define RB952_GPIO_LED_LAN4	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_LAN4)
#define RB952_GPIO_LED_LAN5	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_LAN5)
#define RB952_GPIO_LED_WLAN	RBSPI_SSR_GPIO(RB952_SSR_BIT_LED_WLAN)

static struct gpio_led rb952_leds[] __initdata = {
	{
		.name = "rb:green:user",
		.gpio = RB952_GPIO_LED_USER,
		.active_low = 0,
	}, {
		.name = "rb:blue:wlan",
		.gpio = RB952_GPIO_LED_WLAN,
		.active_low = 1,
	}, {
		.name = "rb:green:port1",
		.gpio = RB952_GPIO_LED_LAN1,
		.active_low = 1,
	}, {
		.name = "rb:green:port2",
		.gpio = RB952_GPIO_LED_LAN2,
		.active_low = 1,
	}, {
		.name = "rb:green:port3",
		.gpio = RB952_GPIO_LED_LAN3,
		.active_low = 1,
	}, {
		.name = "rb:green:port4",
		.gpio = RB952_GPIO_LED_LAN4,
		.active_low = 1,
	}, {
		.name = "rb:green:port5",
		.gpio = RB952_GPIO_LED_LAN5,
		.active_low = 1,
	},
};


/* RB 962UiGS-5HacT2HnT gpios */
#define RB962_GPIO_POE_STATUS	2
#define RB962_GPIO_POE_POWER	3
#define RB962_GPIO_LED_USER	12
#define RB962_GPIO_USB_POWER	13
#define RB962_GPIO_BTN_RESET	20

static struct gpio_led rb962_leds_gpio[] __initdata = {
	{
		.name		= "rb:green:user",
		.gpio		= RB962_GPIO_LED_USER,
		.active_low	= 1,
	},
};

static const struct ar8327_led_info rb962_leds_ar8327[] = {
		AR8327_LED_INFO(PHY0_0, HW, "rb:green:port1"),
		AR8327_LED_INFO(PHY1_0, HW, "rb:green:port2"),
		AR8327_LED_INFO(PHY2_0, HW, "rb:green:port3"),
		AR8327_LED_INFO(PHY3_0, HW, "rb:green:port4"),
		AR8327_LED_INFO(PHY4_0, HW, "rb:green:port5"),
};

static struct ar8327_pad_cfg rb962_ar8327_pad0_cfg = {
		.mode = AR8327_PAD_MAC_RGMII,
		.txclk_delay_en = true,
		.rxclk_delay_en = true,
		.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
		.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
		.mac06_exchange_dis = true,
};

static struct ar8327_pad_cfg rb962_ar8327_pad6_cfg = {
		/* Use SGMII interface for GMAC6 of the AR8337 switch */
		.mode = AR8327_PAD_MAC_SGMII,
		.rxclk_delay_en = true,
		.rxclk_delay_sel = AR8327_CLK_DELAY_SEL0,
};

static struct ar8327_led_cfg rb962_ar8327_led_cfg = {
		.led_ctrl0 = 0xc737c737,
		.led_ctrl1 = 0x00000000,
		.led_ctrl2 = 0x00000000,
		.led_ctrl3 = 0x0030c300,
		.open_drain = false,
};

static struct ar8327_platform_data rb962_ar8327_data = {
		.pad0_cfg = &rb962_ar8327_pad0_cfg,
		.pad6_cfg = &rb962_ar8327_pad6_cfg,
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
		.led_cfg = &rb962_ar8327_led_cfg,
		.num_leds = ARRAY_SIZE(rb962_leds_ar8327),
		.leds = rb962_leds_ar8327,
};

static struct mdio_board_info rb962_mdio0_info[] = {
		{
				.bus_id = "ag71xx-mdio.0",
				.phy_addr = 0,
				.platform_data = &rb962_ar8327_data,
		},
};

/* RB wAP-2nD gpios */
#define RBWAP_GPIO_LED_USER	14
#define RBWAP_GPIO_LED_WLAN	11
#define RBWAP_GPIO_BTN_RESET	16

static struct gpio_led rbwap_leds[] __initdata = {
	{
		.name = "rb:green:user",
		.gpio = RBWAP_GPIO_LED_USER,
		.active_low = 1,
	}, {
		.name = "rb:green:wlan",
		.gpio = RBWAP_GPIO_LED_WLAN,
		.active_low = 1,
	},
};

/* RB cAP-2nD gpios */
#define RBCAP_GPIO_LED_1	14
#define RBCAP_GPIO_LED_2	12
#define RBCAP_GPIO_LED_3	11
#define RBCAP_GPIO_LED_4	4
#define RBCAP_GPIO_LED_ALL	13

static struct gpio_led rbcap_leds[] __initdata = {
	{
		.name = "rb:green:rssi1",
		.gpio = RBCAP_GPIO_LED_1,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi2",
		.gpio = RBCAP_GPIO_LED_2,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi3",
		.gpio = RBCAP_GPIO_LED_3,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi4",
		.gpio = RBCAP_GPIO_LED_4,
		.active_low = 1,
	},
};

/* RB mAP-2nD gpios */
#define RBMAP_SSR_BIT_LED_LAN1	0
#define RBMAP_SSR_BIT_LED_LAN2	1
#define RBMAP_SSR_BIT_LED_POEO	2
#define RBMAP_SSR_BIT_LED_USER	3
#define RBMAP_SSR_BIT_LED_WLAN	4
#define RBMAP_SSR_BIT_USB_POWER	5
#define RBMAP_SSR_BIT_LED_APCAP	6
#define RBMAP_GPIO_BTN_RESET	16
#define RBMAP_GPIO_SSR_CS	11
#define RBMAP_GPIO_LED_POWER	4
#define RBMAP_GPIO_POE_POWER	14
#define RBMAP_GPIO_POE_STATUS	12
#define RBMAP_GPIO_USB_POWER	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_USB_POWER)
#define RBMAP_GPIO_LED_LAN1	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_LAN1)
#define RBMAP_GPIO_LED_LAN2	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_LAN2)
#define RBMAP_GPIO_LED_POEO	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_POEO)
#define RBMAP_GPIO_LED_USER	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_USER)
#define RBMAP_GPIO_LED_WLAN	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_WLAN)
#define RBMAP_GPIO_LED_APCAP	RBSPI_SSR_GPIO(RBMAP_SSR_BIT_LED_APCAP)

static struct gpio_led rbmap_leds[] __initdata = {
	{
		.name = "rb:green:power",
		.gpio = RBMAP_GPIO_LED_POWER,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	}, {
		.name = "rb:green:eth1",
		.gpio = RBMAP_GPIO_LED_LAN1,
		.active_low = 1,
	}, {
		.name = "rb:green:eth2",
		.gpio = RBMAP_GPIO_LED_LAN2,
		.active_low = 1,
	}, {
		.name = "rb:red:poe_out",
		.gpio = RBMAP_GPIO_LED_POEO,
		.active_low = 1,
	}, {
		.name = "rb:green:user",
		.gpio = RBMAP_GPIO_LED_USER,
		.active_low = 1,
	}, {
		.name = "rb:green:wlan",
		.gpio = RBMAP_GPIO_LED_WLAN,
		.active_low = 1,
	}, {
		.name = "rb:green:ap_cap",
		.gpio = RBMAP_GPIO_LED_APCAP,
		.active_low = 1,
	},
};

/* RB LHG 5nD gpios */
#define RBLHG_GPIO_LED_0	13
#define RBLHG_GPIO_LED_1	12
#define RBLHG_GPIO_LED_2	4
#define RBLHG_GPIO_LED_3	21
#define RBLHG_GPIO_LED_4	18
#define RBLHG_GPIO_LED_ETH	14
#define RBLHG_GPIO_LED_POWER	11
#define RBLHG_GPIO_LED_USER	20
#define RBLHG_GPIO_BTN_RESET	15

static struct gpio_led rblhg_leds[] __initdata = {
	{
		.name = "rb:green:rssi0",
		.gpio = RBLHG_GPIO_LED_0,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi1",
		.gpio = RBLHG_GPIO_LED_1,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi2",
		.gpio = RBLHG_GPIO_LED_2,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi3",
		.gpio = RBLHG_GPIO_LED_3,
		.active_low = 1,
	}, {
		.name = "rb:green:rssi4",
		.gpio = RBLHG_GPIO_LED_4,
		.active_low = 1,
	}, {
		.name = "rb:green:eth",
		.gpio = RBLHG_GPIO_LED_ETH,
		.active_low = 1,
	}, {
		.name = "rb:green:user",
		.gpio = RBLHG_GPIO_LED_USER,
		.active_low = 1,
	}, {
		.name = "rb:blue:power",
		.gpio = RBLHG_GPIO_LED_POWER,
		.active_low = 0,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
};

/* RB w APG-5HacT2HnD (wAP AC) gpios*/
#define RBWAPGSC_WIFI_LED_1		1
#define RBWAPGSC_WIFI_LED_2		8
#define RBWAPGSC_WIFI_LED_3		9
#define RBWAPGSC_GPIO_LED_POWER		16
#define RBWAPGSC_GPIO_BTN_RESET		1
#define RBWAPGSC_GPIO_MDIO_MDC		12
#define RBWAPGSC_GPIO_MDIO_DATA		11
#define RBWAPGSC_MDIO_PHYADDR		0

static struct gpio_led rbwapgsc_leds[] __initdata = {
	{
		.name = "rb:green:power",
		.gpio = RBWAPGSC_GPIO_LED_POWER,
		.active_low = 1,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
	},
};

static struct mdio_gpio_platform_data rbwapgsc_mdio_data = {
	.mdc = RBWAPGSC_GPIO_MDIO_MDC,
	.mdio = RBWAPGSC_GPIO_MDIO_DATA,
	.phy_mask = ~BIT(RBWAPGSC_MDIO_PHYADDR),
};

static struct platform_device rbwapgsc_phy_device = {
	.name = "mdio-gpio",
	.id = 1,
	.dev = {
		.platform_data = &rbwapgsc_mdio_data
	},
};

/* RB911L GPIOs */
#define RB911L_GPIO_BTN_RESET	15
#define RB911L_GPIO_LED_1	13
#define RB911L_GPIO_LED_2	12
#define RB911L_GPIO_LED_3	4
#define RB911L_GPIO_LED_4	21
#define RB911L_GPIO_LED_5	18
#define RB911L_GPIO_LED_ETH	20
#define RB911L_GPIO_LED_POWER	11
#define RB911L_GPIO_LED_USER	3
#define RB911L_GPIO_PIN_HOLE	14 /* for reference */

static struct gpio_led rb911l_leds[] __initdata = {
	{
		.name = "rb:green:eth",
		.gpio = RB911L_GPIO_LED_ETH,
		.active_low = 1,
	}, {
		.name = "rb:green:led1",
		.gpio = RB911L_GPIO_LED_1,
		.active_low = 1,
	}, {
		.name = "rb:green:led2",
		.gpio = RB911L_GPIO_LED_2,
		.active_low = 1,
	}, {
		.name = "rb:green:led3",
		.gpio = RB911L_GPIO_LED_3,
		.active_low = 1,
	}, {
		.name = "rb:green:led4",
		.gpio = RB911L_GPIO_LED_4,
		.active_low = 1,
	}, {
		.name = "rb:green:led5",
		.gpio = RB911L_GPIO_LED_5,
		.active_low = 1,
	}, {
		.name = "rb:green:power",
		.gpio = RB911L_GPIO_LED_POWER,
		.default_state = LEDS_GPIO_DEFSTATE_ON,
		.open_drain = 1,
	}, {
		.name = "rb:green:user",
		.gpio = RB911L_GPIO_LED_USER,
		.active_low = 1,
		.open_drain = 1,
	},
};

static struct gen_74x164_chip_platform_data rbspi_ssr_data = {
	.base = RBSPI_SSR_GPIO_BASE,
	.num_registers = 1,
};

/* the spi-ath79 driver can only natively handle CS0. Other CS are bit-banged */
static int rbspi_spi_cs_gpios[] = {
	-ENOENT,	/* CS0 is always -ENOENT: natively handled */
	-ENOENT,	/* CS1 can be updated by the code as necessary */
};

static struct ath79_spi_platform_data rbspi_ath79_spi_data = {
	.bus_num = 0,
	.cs_gpios = rbspi_spi_cs_gpios,
};

/*
 * Global spi_board_info: devices that don't have an SSR only have the SPI NOR
 * flash on bus0 CS0, while devices that have an SSR add it on the same bus CS1
 */
static struct spi_board_info rbspi_spi_info[] = {
	{
		.bus_num	= 0,
		.chip_select	= 0,
		.max_speed_hz	= 25000000,
		.modalias	= "m25p80",
		.platform_data	= &rbspi_spi_flash_data,
	}, {
		.bus_num	= 0,
		.chip_select	= 1,
		.max_speed_hz	= 25000000,
		.modalias	= "74x164",
		.platform_data	= &rbspi_ssr_data,
	}
};

void __init rbspi_wlan_init(u16 id, int wmac_offset)
{
	char *art_buf;
	u8 wlan_mac[ETH_ALEN];

	art_buf = rb_get_ext_wlan_data(id);
	if (!art_buf)
		return;

	ath79_init_mac(wlan_mac, ath79_mac_base, wmac_offset);
	ath79_register_wmac(art_buf + 0x1000, wlan_mac);

	kfree(art_buf);
}

#define RBSPI_MACH_BUFLEN	64
/*
 * Common platform init routine for all SPI NOR devices.
 */
static __init const struct rb_info *rbspi_platform_setup(void)
{
	const struct rb_info *info;
	char buf[RBSPI_MACH_BUFLEN] = "MikroTik ";
	char *str;
	int len = RBSPI_MACH_BUFLEN - strlen(buf) - 1;

	info = rb_init_info((void *)(KSEG1ADDR(AR71XX_SPI_BASE)), 0x20000);
	if (!info)
		return NULL;

	if (info->board_name) {
		str = "RouterBOARD ";
		if (strncmp(info->board_name, str, strlen(str))) {
			strncat(buf, str, len);
			len -= strlen(str);
		}
		strncat(buf, info->board_name, len);
	}
	else
		strncat(buf, "UNKNOWN", len);

	mips_set_machine_name(buf);

	/* fix partitions based on flash parsing */
	rbspi_init_partitions(info);

	return info;
}

/*
 * Common peripherals init routine for all SPI NOR devices.
 * Sets SPI and USB.
 */
static void __init rbspi_peripherals_setup(u32 flags)
{
	unsigned spi_n;

	if (flags & RBSPI_HAS_SSR)
		spi_n = ARRAY_SIZE(rbspi_spi_info);
	else
		spi_n = 1;     /* only one device on bus0 */

	rbspi_ath79_spi_data.num_chipselect = spi_n;
	rbspi_ath79_spi_data.cs_gpios = rbspi_spi_cs_gpios;
	ath79_register_spi(&rbspi_ath79_spi_data, rbspi_spi_info, spi_n);

	if (flags & RBSPI_HAS_USB)
		ath79_register_usb();

	if (flags & RBSPI_HAS_PCI)
		ath79_register_pci();
}

/*
 * Common network init routine for all SPI NOR devices.
 * Sets LAN/WAN/WLAN.
 */
static void __init rbspi_network_setup(u32 flags, int gmac1_offset,
					int wmac0_offset, int wmac1_offset)
{
	/* for QCA953x that will init mdio1_device/data */
	ath79_register_mdio(0, 0x0);
	if (flags & RBSPI_HAS_MDIO1)
		ath79_register_mdio(1, 0x0);

	if (flags & RBSPI_HAS_WAN4) {
		ath79_setup_ar934x_eth_cfg(0);

		/* set switch to oper mode 1, PHY4 connected to CPU */
		ath79_switch_data.phy4_mii_en = 1;
		ath79_switch_data.phy_poll_mask |= BIT(4);

		/* init GMAC0 connected to PHY4 at 100M */
		ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
		ath79_eth0_data.phy_mask = BIT(4);
		ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
		ath79_register_eth(0);
	} else {
		/* set the SoC to SW_ONLY_MODE, which connects all PHYs
		 * to the internal switch.
		 * We hijack ath79_setup_ar934x_eth_cfg() to set the switch in
		 * the QCA953x, this works because this configuration bit is
		 * the same as the AR934x. There's no equivalent function for
		 * QCA953x for now. */
		ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);
	}

	/* init GMAC1 */
	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, gmac1_offset);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

	if (flags & RBSPI_HAS_WLAN0)
		rbspi_wlan_init(0, wmac0_offset);

	if (flags & RBSPI_HAS_WLAN1)
		rbspi_wlan_init(1, wmac1_offset);
}

static __init void rbspi_register_reset_button(int gpio)
{
	rbspi_gpio_keys_reset[0].gpio = gpio;
	ath79_register_gpio_keys_polled(-1, RBSPI_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(rbspi_gpio_keys_reset),
					rbspi_gpio_keys_reset);
}

/*
 * Init the mAP lite hardware (QCA953x).
 * The mAP L-2nD (mAP lite) has a single ethernet port, connected to PHY0.
 * Trying to use GMAC0 in direct mode was unsucessful, so we're
 * using SW_ONLY_MODE, which connects PHY0 to MAC1 on the internal
 * switch, which is connected to GMAC1 on the SoC. GMAC0 is unused.
 */
static void __init rbmapl_setup(void)
{
	u32 flags = RBSPI_HAS_WLAN0;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN0 MAC is HW MAC + 1 */
	rbspi_network_setup(flags, 0, 1, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbmapl_leds), rbmapl_leds);

	/* mAP lite has a single reset button as gpio 16 */
	rbspi_register_reset_button(RBMAPL_GPIO_BTN_RESET);

	/* clear internal multiplexing */
	ath79_gpio_output_select(RBMAPL_GPIO_LED_ETH, AR934X_GPIO_OUT_GPIO);
	ath79_gpio_output_select(RBMAPL_GPIO_LED_POWER, AR934X_GPIO_OUT_GPIO);
}

/*
 * Init the hAP lite hardware (QCA953x).
 * The 941-2nD (hAP lite) has 4 ethernet ports, with port 2-4
 * being assigned to LAN on the casing, and port 1 being assigned
 * to "internet" (WAN) on the casing. Port 1 is connected to PHY3.
 * Since WAN is neither PHY0 nor PHY4, we cannot use GMAC0 with this device.
 */
static void __init rbhapl_setup(void)
{
	u32 flags = RBSPI_HAS_WLAN0;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN0 MAC is HW MAC + 4 */
	rbspi_network_setup(flags, 0, 4, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbhapl_leds), rbhapl_leds);

	/* hAP lite has a single reset button as gpio 16 */
	rbspi_register_reset_button(RBHAPL_GPIO_BTN_RESET);
}

/*
 * The hAP, hAP ac lite, hEX lite and hEX PoE lite share the same platform
 */
static void __init rbspi_952_750r2_setup(u32 flags)
{
	if (flags & RBSPI_HAS_SSR)
		rbspi_spi_cs_gpios[1] = RB952_GPIO_SSR_CS;

	rbspi_peripherals_setup(flags);

	/*
	 * GMAC1 is HW MAC + 1, WLAN0 MAC IS HW MAC + 5 (hAP),
	 * WLAN1 MAC IS HW MAC + 6 (hAP ac lite)
	 */
	rbspi_network_setup(flags, 1, 5, 6);

	if (flags & RBSPI_HAS_USB)
		gpio_request_one(RB952_GPIO_USB_POWER,
				GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
				"USB power");

	if (flags & RBSPI_HAS_POE)
		gpio_request_one(RB952_GPIO_POE_POWER,
				GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
				"POE power");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb952_leds), rb952_leds);

	/* These devices have a single reset button as gpio 16 */
	rbspi_register_reset_button(RB952_GPIO_BTN_RESET);
}

/*
 * Init the hAP (ac lite) hardware (QCA953x).
 * The 951Ui-2nD (hAP) has 5 ethernet ports, with ports 2-5 being assigned
 * to LAN on the casing, and port 1 being assigned to "internet" (WAN).
 * Port 1 is connected to PHY4 (the ports are labelled in reverse physical
 * number), so the SoC can be set to connect GMAC0 to PHY4 and GMAC1 to the
 * internal switch for the LAN ports.
 * The device also has USB, PoE output and an SSR used for LED multiplexing.
 * The 952Ui-5ac2nD (hAP ac lite) is nearly identical to the hAP, it adds a
 * QCA9887 5GHz radio via PCI and moves 2.4GHz from WLAN0 to WLAN1.
 */
static void __init rb952_setup(void)
{
	u32 flags = RBSPI_HAS_WAN4 | RBSPI_HAS_USB |
			RBSPI_HAS_SSR | RBSPI_HAS_POE;

	if (!rbspi_platform_setup())
		return;

	/* differentiate the hAP from the hAP ac lite */
	if (strstr(mips_get_machine_name(), "952Ui-5ac2nD"))
		flags |= RBSPI_HAS_WLAN1 | RBSPI_HAS_PCI;
	else
		flags |= RBSPI_HAS_WLAN0;

	rbspi_952_750r2_setup(flags);
}

/*
 * Init the hEX (PoE) lite hardware (QCA953x).
 * The 750UP r2 (hEX PoE lite) is nearly identical to the hAP, only without
 * WLAN. The 750 r2 (hEX lite) is nearly identical to the 750UP r2, only
 * without USB and POE. The 750P Pbr2 (Powerbox) is nearly identical to hEX PoE
 * lite, only without USB. It shares the same bootloader board identifier.
 */
static void __init rb750upr2_setup(void)
{
	u32 flags = RBSPI_HAS_WAN4 | RBSPI_HAS_SSR;

	if (!rbspi_platform_setup())
		return;

	/* differentiate the hEX lite from the hEX PoE lite */
	if (strstr(mips_get_machine_name(), "750UP r2"))
		flags |= RBSPI_HAS_USB | RBSPI_HAS_POE;

	/* differentiate the Powerbox from the hEX lite */
	else if (strstr(mips_get_machine_name(), "750P r2"))
		flags |= RBSPI_HAS_POE;

	rbspi_952_750r2_setup(flags);
}

/*
 * Init the hAP ac / 962UiGS-5HacT2HnT hardware (QCA9558).
 * The hAP ac has 5 ethernet ports provided by an AR8337 switch. Port 1 is
 * assigned to WAN, ports 2-5 are assigned to LAN. Port 0 is connected to the
 * SoC, ports 1-5 of the switch are connected to physical ports 1-5 in order.
 * The SFP cage is not assigned by default on RouterOS. Extra work is required
 * to support this interface as it is directly connected to the SoC (eth1).
 * Wireless is provided by a 2.4GHz radio on the SoC (WLAN1) and a 5GHz radio
 * attached via PCI (QCA9880). Red and green WLAN LEDs are populated however
 * they are not attached to GPIOs, extra work is required to support these.
 * PoE and USB output power control is supported.
 */
static void __init rb962_setup(void)
{
	u32 flags = RBSPI_HAS_USB | RBSPI_HAS_POE | RBSPI_HAS_PCI;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* Do not call rbspi_network_setup as we have a discrete switch chip */
	ath79_eth0_pll_data.pll_1000 = 0xae000000;
	ath79_eth0_pll_data.pll_100 = 0xa0000101;
	ath79_eth0_pll_data.pll_10 = 0xa0001313;

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(rb962_mdio0_info,
					ARRAY_SIZE(rb962_mdio0_info));

	ath79_setup_qca955x_eth_cfg(QCA955X_ETH_CFG_RGMII_EN);

	ath79_init_mac(ath79_eth0_data.mac_addr, ath79_mac_base, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_register_eth(0);

	/* WLAN1 MAC is HW MAC + 7 */
	rbspi_wlan_init(1, 7);

	if (flags & RBSPI_HAS_USB)
		gpio_request_one(RB962_GPIO_USB_POWER,
				GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
				"USB power");

	/* PoE output GPIO is inverted, set GPIOF_ACTIVE_LOW for consistency */
	if (flags & RBSPI_HAS_POE)
		gpio_request_one(RB962_GPIO_POE_POWER,
				GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW |
					GPIOF_EXPORT_DIR_FIXED,
				"POE power");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb962_leds_gpio),
				rb962_leds_gpio);

	/* This device has a single reset button as gpio 20 */
	rbspi_register_reset_button(RB962_GPIO_BTN_RESET);
}

/*
 * Init the LHG hardware (AR9344).
 * The LHG 5nD has a single ethernet port connected to PHY0.
 * Wireless is provided via 5GHz WLAN1.
 */
static void __init rblhg_setup(void)
{
	u32 flags = RBSPI_HAS_WLAN1 | RBSPI_HAS_MDIO1;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN1 MAC is HW MAC + 1 */
	rbspi_network_setup(flags, 0, 0, 1);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rblhg_leds), rblhg_leds);

	rbspi_register_reset_button(RBLHG_GPIO_BTN_RESET);
}

/*
 * Init the wAP hardware.
 * The wAP 2nD has a single ethernet port.
 */
static void __init rbwap_setup(void)
{
	u32 flags = RBSPI_HAS_WLAN0;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN0 MAC is HW MAC + 1 */
	rbspi_network_setup(flags, 0, 1, 0);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbwap_leds), rbwap_leds);

	/* wAP has a single reset button as GPIO 16 */
	rbspi_register_reset_button(RBWAP_GPIO_BTN_RESET);
}

/*
 * Init the cAP hardware (EXPERIMENTAL).
 * The cAP 2nD has a single ethernet port, and a global LED switch.
 */
static void __init rbcap_setup(void)
{
	u32 flags = RBSPI_HAS_WLAN0;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN0 MAC is HW MAC + 1 */
	rbspi_network_setup(flags, 0, 1, 0);

	gpio_request_one(RBCAP_GPIO_LED_ALL,
			 GPIOF_OUT_INIT_HIGH | GPIOF_EXPORT_DIR_FIXED,
			 "LEDs enable");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbcap_leds), rbcap_leds);
}

/*
 * Init the mAP hardware.
 * The mAP 2nD has two ethernet ports, PoE output, SSR for LED
 * multiplexing and USB port.
 */
static void __init rbmap_setup(void)
{
	u32 flags = RBSPI_HAS_USB | RBSPI_HAS_WLAN0 |
			RBSPI_HAS_SSR | RBSPI_HAS_POE;

	if (!rbspi_platform_setup())
		return;

	rbspi_spi_cs_gpios[1] = RBMAP_GPIO_SSR_CS;
	rbspi_peripherals_setup(flags);

	/* GMAC1 is HW MAC, WLAN0 MAC is HW MAC + 2 */
	rbspi_network_setup(flags, 0, 2, 0);

	if (flags & RBSPI_HAS_POE)
		gpio_request_one(RBMAP_GPIO_POE_POWER,
				GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED,
				"POE power");

	/* USB power GPIO is inverted, set GPIOF_ACTIVE_LOW for consistency */
	if (flags & RBSPI_HAS_USB)
		gpio_request_one(RBMAP_GPIO_USB_POWER,
				GPIOF_OUT_INIT_HIGH | GPIOF_ACTIVE_LOW |
					GPIOF_EXPORT_DIR_FIXED,
				"USB power");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbmap_leds), rbmap_leds);

	/* mAP 2nD has a single reset button as gpio 16 */
	rbspi_register_reset_button(RBMAP_GPIO_BTN_RESET);
}

/*
 * Init the wAPGSC (RB wAPG-5HacT2HnD // wAP AC) hardware.
 * The wAPGSC has one Ethernet port via AR8033 with PoE input, dual radio (SoC
 * 2.4 GHz and external QCA9880) and a ZT2046Q temperature and voltage sensor
 * (currently not supported).
 */
static void __init rbwapgsc_setup(void)
{
	u32 flags = RBSPI_HAS_PCI;

	if (!rbspi_platform_setup())
		return;

	rbspi_peripherals_setup(flags);

	platform_device_register(&rbwapgsc_phy_device);

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);
	ath79_eth1_data.mii_bus_dev = &rbwapgsc_phy_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_SGMII;
	ath79_eth1_data.phy_mask = BIT(RBWAPGSC_MDIO_PHYADDR);
	ath79_eth1_pll_data.pll_1000 = 0x03000101;
	ath79_eth1_pll_data.pll_100 = 0x80000101;
	ath79_eth1_pll_data.pll_10 = 0x80001313;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;
	ath79_register_eth(1);

	rbspi_wlan_init(1, 2);

	rbspi_register_reset_button(RBWAPGSC_GPIO_BTN_RESET);

	ath79_gpio_function_enable(QCA955X_GPIO_FUNC_JTAG_DISABLE|
				QCA955X_GPIO_REG_OUT_FUNC4|
				QCA955X_GPIO_REG_OUT_FUNC3);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rbwapgsc_leds),
			rbwapgsc_leds);
}

/*
 * Setup the 911L hardware (AR9344).
 */
static void __init rb911l_setup(void)
{
	const struct rb_info *info;

	info = rbspi_platform_setup();
	if (!info)
		return;

	if (!rb_has_hw_option(info, RB_HW_OPT_NO_NAND)) {
		/*
		 * Old hardware revisions might be equipped with a NAND flash
		 * chip instead of the 16MiB SPI NOR device. Those boards are
		 * not supported at the moment, so throw a warning and skip
		 * the peripheral setup to avoid messing up the data in the
		 * flash chip.
		 */
		WARN(1, "The NAND flash on this board is not supported.\n");
	} else {
		rbspi_peripherals_setup(0);
	}

	ath79_register_mdio(1, 0x0);

	ath79_init_mac(ath79_eth1_data.mac_addr, ath79_mac_base, 0);

	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.speed = SPEED_1000;
	ath79_eth1_data.duplex = DUPLEX_FULL;

	ath79_register_eth(1);

	rbspi_wlan_init(0, 1);

	rbspi_register_reset_button(RB911L_GPIO_BTN_RESET);

	/* Make the eth LED controllable by software. */
	ath79_gpio_output_select(RB911L_GPIO_LED_ETH, AR934X_GPIO_OUT_GPIO);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(rb911l_leds), rb911l_leds);
}

MIPS_MACHINE_NONAME(ATH79_MACH_RB_MAPL, "map-hb", rbmapl_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_941, "H951L", rbhapl_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_911L, "911L", rb911l_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_952, "952-hb", rb952_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_962, "962", rb962_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_750UPR2, "750-hb", rb750upr2_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_LHG5, "lhg", rblhg_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_WAP, "wap-hb", rbwap_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_CAP, "cap-hb", rbcap_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_MAP, "map2-hb", rbmap_setup);
MIPS_MACHINE_NONAME(ATH79_MACH_RB_WAPAC, "wapg-sc", rbwapgsc_setup);
