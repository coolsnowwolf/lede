/*
 *  Ubiquiti Networks XM (rev 1.0) board support
 *
 *  Copyright (C) 2011 René Bolldorf <xsecute@googlemail.com>
 *
 *  Derived from: mach-pb44.c
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/pci.h>
#include <linux/gpio.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/etherdevice.h>
#include <linux/ar8216_platform.h>
#include <linux/platform_data/phy-at803x.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/irq.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include <linux/platform_data/phy-at803x.h>

#include "common.h"
#include "dev-ap9x-pci.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define UBNT_XM_GPIO_LED_L1		0
#define UBNT_XM_GPIO_LED_L2		1
#define UBNT_XM_GPIO_LED_L3		11
#define UBNT_XM_GPIO_LED_L4		7

#define UBNT_XM_GPIO_BTN_RESET		12

#define UBNT_XM_KEYS_POLL_INTERVAL	20
#define UBNT_XM_KEYS_DEBOUNCE_INTERVAL	(3 * UBNT_XM_KEYS_POLL_INTERVAL)

#define UBNT_XM_EEPROM_ADDR		0x1fff1000

static struct gpio_led ubnt_xm_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:red:link1",
		.gpio		= UBNT_XM_GPIO_LED_L1,
		.active_low	= 0,
	}, {
		.name		= "ubnt:orange:link2",
		.gpio		= UBNT_XM_GPIO_LED_L2,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:link3",
		.gpio		= UBNT_XM_GPIO_LED_L3,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:link4",
		.gpio		= UBNT_XM_GPIO_LED_L4,
		.active_low	= 0,
	},
};

static struct gpio_keys_button ubnt_xm_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= UBNT_XM_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= UBNT_XM_GPIO_BTN_RESET,
		.active_low		= 1,
	}
};

#define UBNT_M_WAN_PHYMASK	BIT(4)

static void __init ubnt_xm_init(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(UBNT_XM_EEPROM_ADDR);
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac2 = (u8 *) KSEG1ADDR(0x1fff0000 + ETH_ALEN);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_xm_leds_gpio),
				 ubnt_xm_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ubnt_xm_gpio_keys),
					ubnt_xm_gpio_keys);

	ath79_register_m25p80(NULL);
	ap91_pci_init(eeprom, NULL);

	ath79_register_mdio(0, ~UBNT_M_WAN_PHYMASK);
	ath79_eth0_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_eth1_data.speed = SPEED_100;
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_UBNT_XM,
	     "UBNT-XM",
	     "Ubiquiti Networks XM (rev 1.0) board",
	     ubnt_xm_init);

MIPS_MACHINE(ATH79_MACH_UBNT_BULLET_M, "UBNT-BM", "Ubiquiti Bullet M",
	     ubnt_xm_init);

static void __init ubnt_rocket_m_setup(void)
{
	ubnt_xm_init();
	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_UBNT_ROCKET_M, "UBNT-RM", "Ubiquiti Rocket M",
	     ubnt_rocket_m_setup);

static void __init ubnt_nano_m_setup(void)
{
	ubnt_xm_init();
	ath79_register_eth(1);
}

MIPS_MACHINE(ATH79_MACH_UBNT_NANO_M, "UBNT-NM", "Ubiquiti Nanostation M",
	     ubnt_nano_m_setup);

static struct gpio_led ubnt_airrouter_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:globe",
		.gpio		= 0,
		.active_low	= 1,
	}, {
	        .name		= "ubnt:green:power",
		.gpio		= 11,
		.active_low	= 1,
		.default_state  = LEDS_GPIO_DEFSTATE_ON,
	}
};

static void __init ubnt_airrouter_setup(void)
{
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(NULL);
	ath79_register_mdio(0, ~UBNT_M_WAN_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_local_mac(ath79_eth1_data.mac_addr, mac1);

	ath79_register_eth(1);
	ath79_register_eth(0);
	ath79_register_usb();

	ap91_pci_init(ee, NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_airrouter_leds_gpio),
				 ubnt_airrouter_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_UBNT_AIRROUTER, "UBNT-AR", "Ubiquiti AirRouter",
	     ubnt_airrouter_setup);

static struct gpio_led ubnt_unifi_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:orange:dome",
		.gpio		= 1,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:dome",
		.gpio		= 0,
		.active_low	= 0,
	}
};

static struct gpio_led ubnt_unifi_outdoor_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:orange:front",
		.gpio		= 1,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:front",
		.gpio		= 0,
		.active_low	= 0,
	}
};

static struct gpio_led ubnt_unifi_outdoor_plus_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:white:front",
		.gpio		= 1,
		.active_low	= 0,
	}, {
		.name		= "ubnt:blue:front",
		.gpio		= 0,
		.active_low	= 0,
	}
};


static void __init ubnt_unifi_setup(void)
{
	u8 *mac = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, ~UBNT_M_WAN_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr, mac, 0);
	ath79_register_eth(0);

	ap91_pci_init(ee, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_unifi_leds_gpio),
				 ubnt_unifi_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_UBNT_UNIFI, "UBNT-UF", "Ubiquiti UniFi",
	     ubnt_unifi_setup);


#define UBNT_UNIFIOD_PRI_PHYMASK	BIT(4)
#define UBNT_UNIFIOD_2ND_PHYMASK	(BIT(0) | BIT(1) | BIT(2) | BIT(3))

static void __init ubnt_unifi_outdoor_setup(void)
{
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac2 = (u8 *) KSEG1ADDR(0x1fff0000 + ETH_ALEN);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, ~(UBNT_UNIFIOD_PRI_PHYMASK |
				 UBNT_UNIFIOD_2ND_PHYMASK));

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ap91_pci_init(ee, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_unifi_outdoor_leds_gpio),
				 ubnt_unifi_outdoor_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_UBNT_UNIFI_OUTDOOR, "UBNT-U20",
	     "Ubiquiti UniFiAP Outdoor",
	     ubnt_unifi_outdoor_setup);


static void __init ubnt_unifi_outdoor_plus_setup(void)
{
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac2 = (u8 *) KSEG1ADDR(0x1fff0000 + ETH_ALEN);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);

	ath79_register_m25p80(NULL);

	ath79_register_mdio(0, ~(UBNT_UNIFIOD_PRI_PHYMASK |
				 UBNT_UNIFIOD_2ND_PHYMASK));

	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr, mac2, 0);
	ath79_register_eth(0);
	ath79_register_eth(1);

	ap9x_pci_get_wmac_data(0)->ubnt_hsr = true;
	ap91_pci_init(ee, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_unifi_outdoor_plus_leds_gpio),
				 ubnt_unifi_outdoor_plus_leds_gpio);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);
}

MIPS_MACHINE(ATH79_MACH_UBNT_UNIFI_OUTDOOR_PLUS, "UBNT-UOP",
	     "Ubiquiti UniFiAP Outdoor+",
	     ubnt_unifi_outdoor_plus_setup);


static struct gpio_led ubnt_uap_pro_gpio_leds[] __initdata = {
	{
		.name		= "ubnt:white:dome",
		.gpio		= 12,
	}, {
		.name		= "ubnt:blue:dome",
		.gpio		= 13,
	}
};

static struct gpio_keys_button uap_pro_gpio_keys[] __initdata = {
	{
		.desc			= "reset",
		.type			= EV_KEY,
		.code			= KEY_RESTART,
		.debounce_interval	= UBNT_XM_KEYS_DEBOUNCE_INTERVAL,
		.gpio			= 17,
		.active_low		= 1,
	}
};

static struct ar8327_pad_cfg uap_pro_ar8327_pad0_cfg = {
	.mode = AR8327_PAD_MAC_RGMII,
	.txclk_delay_en = true,
	.rxclk_delay_en = true,
	.txclk_delay_sel = AR8327_CLK_DELAY_SEL1,
	.rxclk_delay_sel = AR8327_CLK_DELAY_SEL2,
};

static struct ar8327_platform_data uap_pro_ar8327_data = {
	.pad0_cfg = &uap_pro_ar8327_pad0_cfg,
	.port0_cfg = {
		.force_link = 1,
		.speed = AR8327_PORT_SPEED_1000,
		.duplex = 1,
		.txpause = 1,
		.rxpause = 1,
	},
};

static struct mdio_board_info uap_pro_mdio0_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 0,
		.platform_data = &uap_pro_ar8327_data,
	},
};

#define UAP_PRO_MAC0_OFFSET		0x0000
#define UAP_PRO_MAC1_OFFSET		0x0006
#define UAP_PRO_WMAC_CALDATA_OFFSET	0x1000
#define UAP_PRO_PCI_CALDATA_OFFSET	0x5000

static void __init ubnt_uap_pro_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_uap_pro_gpio_leds),
				 ubnt_uap_pro_gpio_leds);
	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(uap_pro_gpio_keys),
                                        uap_pro_gpio_keys);

	ath79_register_wmac(eeprom + UAP_PRO_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(eeprom + UAP_PRO_PCI_CALDATA_OFFSET, NULL);

	ath79_register_mdio(0, 0x0);
	mdiobus_register_board_info(uap_pro_mdio0_info,
				    ARRAY_SIZE(uap_pro_mdio0_info));

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       eeprom + UAP_PRO_MAC0_OFFSET, 0);

	/* GMAC0 is connected to an AR8327 switch */
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

MIPS_MACHINE(ATH79_MACH_UBNT_UAP_PRO, "UAP-PRO", "Ubiquiti UniFi AP Pro",
	     ubnt_uap_pro_setup);

#define UBNT_XW_GPIO_LED_L1		11
#define UBNT_XW_GPIO_LED_L2		16
#define UBNT_XW_GPIO_LED_L3		13
#define UBNT_XW_GPIO_LED_L4		14

static struct gpio_led ubnt_xw_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:red:link1",
		.gpio		= UBNT_XW_GPIO_LED_L1,
		.active_low	= 1,
	}, {
		.name		= "ubnt:orange:link2",
		.gpio		= UBNT_XW_GPIO_LED_L2,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:link3",
		.gpio		= UBNT_XW_GPIO_LED_L3,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:link4",
		.gpio		= UBNT_XW_GPIO_LED_L4,
		.active_low	= 1,
	},
};

#define UBNT_ROCKET_TI_GPIO_LED_L1	16
#define UBNT_ROCKET_TI_GPIO_LED_L2	17
#define UBNT_ROCKET_TI_GPIO_LED_L3	18
#define UBNT_ROCKET_TI_GPIO_LED_L4	19
#define UBNT_ROCKET_TI_GPIO_LED_L5	20
#define UBNT_ROCKET_TI_GPIO_LED_L6	21
static struct gpio_led ubnt_rocket_ti_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:link1",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L1,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:link2",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L2,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:link3",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L3,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:link4",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L4,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:link5",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L5,
		.active_low	= 0,
	}, {
		.name		= "ubnt:green:link6",
		.gpio		= UBNT_ROCKET_TI_GPIO_LED_L6,
		.active_low	= 0,
	},
};

static void __init ubnt_xw_init(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_xw_leds_gpio),
				 ubnt_xw_leds_gpio);
	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);

	ath79_register_wmac(eeprom + UAP_PRO_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(eeprom + UAP_PRO_PCI_CALDATA_OFFSET, NULL);


	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_MII_GMAC0 | AR934X_ETH_CFG_MII_GMAC0_SLAVE);
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       eeprom + UAP_PRO_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
}

static void __init ubnt_nano_m_xw_setup(void)
{
	ubnt_xw_init();

	/* GMAC0 is connected to an AR8326 switch */
	ath79_register_mdio(0, ~(BIT(0) | BIT(1) | BIT(5)));
	ath79_eth0_data.phy_mask = (BIT(0) | BIT(1) | BIT(5));
	ath79_eth0_data.speed = SPEED_100;
	ath79_eth0_data.duplex = DUPLEX_FULL;
	ath79_register_eth(0);
}

static struct at803x_platform_data ubnt_loco_m_xw_at803x_data = {
	.has_reset_gpio = 1,
	.reset_gpio = 0,
};

static struct mdio_board_info ubnt_loco_m_xw_mdio_info[] = {
	{
		.bus_id = "ag71xx-mdio.0",
		.mdio_addr = 1,
		.platform_data = &ubnt_loco_m_xw_at803x_data,
	},
};

static void __init ubnt_loco_m_xw_setup(void)
{
	ubnt_xw_init();

	mdiobus_register_board_info(ubnt_loco_m_xw_mdio_info,
				    ARRAY_SIZE(ubnt_loco_m_xw_mdio_info));

	ath79_register_mdio(0, ~BIT(1));
	ath79_eth0_data.phy_mask = BIT(1);
	ath79_register_eth(0);
}

#define UBNT_LBE_M5_GPIO_LED_LAN		13
#define UBNT_LBE_M5_GPIO_LED_WLAN		14
#define UBNT_LBE_M5_GPIO_LED_SYS		16

static struct gpio_led ubnt_lbe_m5_leds_gpio[] __initdata = {
	{
		.name		= "ubnt:green:lan",
		.gpio		= UBNT_LBE_M5_GPIO_LED_LAN,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:wlan",
		.gpio		= UBNT_LBE_M5_GPIO_LED_WLAN,
		.active_low	= 1,
	}, {
		.name		= "ubnt:green:sys",
		.gpio		= UBNT_LBE_M5_GPIO_LED_SYS,
		.active_low	= 1,
	},
};

static void __init ubnt_lbe_m5_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_wmac(eeprom + UAP_PRO_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(eeprom + UAP_PRO_PCI_CALDATA_OFFSET, NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_lbe_m5_leds_gpio),
				 ubnt_lbe_m5_leds_gpio);
	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(ubnt_xm_gpio_keys),
					ubnt_xm_gpio_keys);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_MII_GMAC0 |
				   AR934X_ETH_CFG_MII_GMAC0_SLAVE);
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       eeprom + UAP_PRO_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	gpio_request_one(0, GPIOF_OUT_INIT_LOW | GPIOF_ACTIVE_LOW |
			 GPIOF_EXPORT_DIR_FIXED, "SPI nWP");

	mdiobus_register_board_info(ubnt_loco_m_xw_mdio_info,
				    ARRAY_SIZE(ubnt_loco_m_xw_mdio_info));

	ath79_register_mdio(0, ~BIT(1));
	ath79_eth0_data.phy_mask = BIT(1);
	ath79_register_eth(0);
}

static void __init ubnt_rocket_m_xw_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_xw_leds_gpio),
				 ubnt_xw_leds_gpio);
	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);

	ath79_register_wmac(eeprom + UAP_PRO_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(eeprom + UAP_PRO_PCI_CALDATA_OFFSET, NULL);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       eeprom + UAP_PRO_MAC0_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;

	ath79_register_mdio(0, ~BIT(4));
	ath79_eth0_data.phy_mask = BIT(4);
	ath79_eth0_pll_data.pll_1000 = 0x06000000;
	ath79_register_eth(0);
}

static struct at803x_platform_data ubnt_rocket_m_ti_at803_data = {
	.disable_smarteee = 1,
	.enable_rgmii_rx_delay = 1,
	.enable_rgmii_tx_delay = 1,
};
static struct mdio_board_info ubnt_rocket_m_ti_mdio_info[] = {
        {
                .bus_id = "ag71xx-mdio.0",
                .mdio_addr = 4,
                .platform_data = &ubnt_rocket_m_ti_at803_data,
        },
};

static void __init ubnt_rocket_m_ti_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);

	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_rocket_ti_leds_gpio),
				 ubnt_rocket_ti_leds_gpio);
	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
                                        ARRAY_SIZE(ubnt_xm_gpio_keys),
                                        ubnt_xm_gpio_keys);

	ap91_pci_init(eeprom + 0x1000, NULL);

	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_RGMII_GMAC0);
	ath79_setup_ar934x_eth_rx_delay(3, 3);
	ath79_init_mac(ath79_eth0_data.mac_addr,
		       eeprom + UAP_PRO_MAC0_OFFSET, 0);
	ath79_init_mac(ath79_eth1_data.mac_addr,
		       eeprom + UAP_PRO_MAC1_OFFSET, 0);

	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.mii_bus_dev = &ath79_mdio0_device.dev;
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_eth1_data.mii_bus_dev = &ath79_mdio1_device.dev;

	mdiobus_register_board_info(ubnt_rocket_m_ti_mdio_info,
			ARRAY_SIZE(ubnt_rocket_m_ti_mdio_info));
	ath79_register_mdio(0, 0x0);


	ath79_eth0_data.phy_mask = BIT(4);
	/* read out from vendor */
	ath79_eth0_pll_data.pll_1000 = 0x2000000;
	ath79_eth0_pll_data.pll_10 = 0x1313;
	ath79_register_eth(0);

	ath79_register_mdio(1, 0x0);
	ath79_eth1_data.phy_mask = BIT(3);
	ath79_register_eth(1);
}


MIPS_MACHINE(ATH79_MACH_UBNT_NANO_M_XW, "UBNT-NM-XW", "Ubiquiti Nanostation M XW",
	     ubnt_nano_m_xw_setup);

MIPS_MACHINE(ATH79_MACH_UBNT_LBE_M5, "UBNT-LBE-M5", "Ubiquiti Litebeam M5",
		ubnt_lbe_m5_setup);

MIPS_MACHINE(ATH79_MACH_UBNT_LOCO_M_XW, "UBNT-LOCO-XW", "Ubiquiti Loco M XW",
	     ubnt_loco_m_xw_setup);

MIPS_MACHINE(ATH79_MACH_UBNT_ROCKET_M_XW, "UBNT-RM-XW", "Ubiquiti Rocket M XW",
	     ubnt_rocket_m_xw_setup);

MIPS_MACHINE(ATH79_MACH_UBNT_BULLET_M_XW, "UBNT-BM-XW", "Ubiquiti Bullet M XW",
	     ubnt_rocket_m_xw_setup);

MIPS_MACHINE(ATH79_MACH_UBNT_ROCKET_M_TI, "UBNT-RM-TI", "Ubiquiti Rocket M TI",
	     ubnt_rocket_m_ti_setup);

static struct gpio_led ubnt_airgateway_gpio_leds[] __initdata = {
	{
		.name	   = "ubnt:blue:wlan",
		.gpio	   = 0,
	}, {
		.name	   = "ubnt:white:status",
		.gpio	   = 1,
	},
};

static struct gpio_keys_button airgateway_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval	= UBNT_XM_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 12,
		.active_low	= 1,
	}
};

static void __init ubnt_airgateway_setup(void)
{
	u32 t;
	u8 *mac0 = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac1 = (u8 *) KSEG1ADDR(0x1fff0000 + ETH_ALEN);
	u8 *ee = (u8 *) KSEG1ADDR(0x1fff1000);


	ath79_gpio_function_disable(AR933X_GPIO_FUNC_ETH_SWITCH_LED0_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED1_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED2_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED3_EN |
				     AR933X_GPIO_FUNC_ETH_SWITCH_LED4_EN);

	t = ath79_reset_rr(AR933X_RESET_REG_BOOTSTRAP);
	t |= AR933X_BOOTSTRAP_MDIO_GPIO_EN;
	ath79_reset_wr(AR933X_RESET_REG_BOOTSTRAP, t);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_airgateway_gpio_leds),
				 ubnt_airgateway_gpio_leds);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(airgateway_gpio_keys),
					airgateway_gpio_keys);

	ath79_init_mac(ath79_eth1_data.mac_addr, mac0, 0);
	ath79_init_mac(ath79_eth0_data.mac_addr, mac1, 0);

	ath79_register_mdio(0, 0x0);

	ath79_register_eth(1);
	ath79_register_eth(0);

	ath79_register_wmac(ee, NULL);
}

MIPS_MACHINE(ATH79_MACH_UBNT_AIRGW, "UBNT-AGW", "Ubiquiti AirGateway",
	     ubnt_airgateway_setup);

static struct gpio_led ubnt_airgateway_pro_gpio_leds[] __initdata = {
	{
		.name	   = "ubnt:blue:wlan",
		.gpio	   = 13,
	}, {
		.name	   = "ubnt:white:status",
		.gpio	   = 17,
	},
};


static struct gpio_keys_button airgateway_pro_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval	= UBNT_XM_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= 12,
		.active_low	= 1,
	}
};

static void __init ubnt_airgateway_pro_setup(void)
{
	u8 *eeprom = (u8 *) KSEG1ADDR(0x1fff0000);
	u8 *mac0 = (u8 *) KSEG1ADDR(0x1fff0000);

	ath79_register_m25p80(NULL);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(ubnt_airgateway_pro_gpio_leds),
				 ubnt_airgateway_pro_gpio_leds);

	ath79_register_gpio_keys_polled(-1, UBNT_XM_KEYS_POLL_INTERVAL,
					ARRAY_SIZE(airgateway_pro_gpio_keys),
					airgateway_pro_gpio_keys);

	ath79_register_wmac(eeprom + UAP_PRO_WMAC_CALDATA_OFFSET, NULL);
	ap91_pci_init(eeprom + UAP_PRO_PCI_CALDATA_OFFSET, NULL);


	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_ONLY_MODE);

	ath79_register_mdio(1, 0x0);

	/* GMAC0 is left unused in this configuration */

	/* GMAC1 is connected to MAC0 on the internal switch */
	/* The PoE/WAN port connects to port 5 on the internal switch */
	/* The LAN port connects to port 4 on the internal switch */
	ath79_init_mac(ath79_eth1_data.mac_addr, mac0, 0);
	ath79_eth1_data.phy_if_mode = PHY_INTERFACE_MODE_GMII;
	ath79_register_eth(1);

}

MIPS_MACHINE(ATH79_MACH_UBNT_AIRGWP, "UBNT-AGWP", "Ubiquiti AirGateway Pro",
	     ubnt_airgateway_pro_setup);
