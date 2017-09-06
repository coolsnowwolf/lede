/*
 *  Senao EAP7660D board support
 *
 *  Copyright (C) 2010 Daniel Golle <daniel.golle@gmail.com>
 *  Copyright (C) 2008 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/ath5k_platform.h>
#include <linux/delay.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "machtypes.h"
#include "pci.h"

#define EAP7660D_KEYS_POLL_INTERVAL	20	/* msecs */
#define EAP7660D_KEYS_DEBOUNCE_INTERVAL	(3 * EAP7660D_KEYS_POLL_INTERVAL)

#define EAP7660D_GPIO_DS4		7
#define EAP7660D_GPIO_DS5		2
#define EAP7660D_GPIO_DS7		0
#define EAP7660D_GPIO_DS8		4
#define EAP7660D_GPIO_SW1		3
#define EAP7660D_GPIO_SW3		8
#define EAP7660D_PHYMASK		BIT(20)
#define EAP7660D_BOARDCONFIG		0x1F7F0000
#define EAP7660D_GBIC_MAC_OFFSET	0x1000
#define EAP7660D_WMAC0_MAC_OFFSET	0x1010
#define EAP7660D_WMAC1_MAC_OFFSET	0x1016
#define EAP7660D_WMAC0_CALDATA_OFFSET	0x2000
#define EAP7660D_WMAC1_CALDATA_OFFSET	0x3000

#ifdef CONFIG_PCI
static struct ath5k_platform_data eap7660d_wmac0_data;
static struct ath5k_platform_data eap7660d_wmac1_data;
static char eap7660d_wmac0_mac[6];
static char eap7660d_wmac1_mac[6];
static u16 eap7660d_wmac0_eeprom[ATH5K_PLAT_EEP_MAX_WORDS];
static u16 eap7660d_wmac1_eeprom[ATH5K_PLAT_EEP_MAX_WORDS];

static int eap7660d_pci_plat_dev_init(struct pci_dev *dev)
{
	switch (PCI_SLOT(dev->devfn)) {
	case 17:
		dev->dev.platform_data = &eap7660d_wmac0_data;
		break;

	case 18:
		dev->dev.platform_data = &eap7660d_wmac1_data;
		break;
	}

	return 0;
}

void __init eap7660d_pci_init(u8 *cal_data0, u8 *mac_addr0,
			      u8 *cal_data1, u8 *mac_addr1)
{
	if (cal_data0 && *cal_data0 == 0xa55a) {
		memcpy(eap7660d_wmac0_eeprom, cal_data0,
			ATH5K_PLAT_EEP_MAX_WORDS);
		eap7660d_wmac0_data.eeprom_data = eap7660d_wmac0_eeprom;
	}

	if (cal_data1 && *cal_data1 == 0xa55a) {
		memcpy(eap7660d_wmac1_eeprom, cal_data1,
			ATH5K_PLAT_EEP_MAX_WORDS);
		eap7660d_wmac1_data.eeprom_data = eap7660d_wmac1_eeprom;
	}

	if (mac_addr0) {
		memcpy(eap7660d_wmac0_mac, mac_addr0,
			sizeof(eap7660d_wmac0_mac));
		eap7660d_wmac0_data.macaddr = eap7660d_wmac0_mac;
	}

	if (mac_addr1) {
		memcpy(eap7660d_wmac1_mac, mac_addr1,
			sizeof(eap7660d_wmac1_mac));
		eap7660d_wmac1_data.macaddr = eap7660d_wmac1_mac;
	}

	ath79_pci_set_plat_dev_init(eap7660d_pci_plat_dev_init);
	ath79_register_pci();
}
#else
static inline void eap7660d_pci_init(u8 *cal_data0, u8 *mac_addr0,
				     u8 *cal_data1, u8 *mac_addr1)
{
}
#endif /* CONFIG_PCI */

static struct gpio_led eap7660d_leds_gpio[] __initdata = {
	{
		.name		= "eap7660d:green:ds8",
		.gpio		= EAP7660D_GPIO_DS8,
		.active_low	= 0,
	},
	{
		.name		= "eap7660d:green:ds5",
		.gpio		= EAP7660D_GPIO_DS5,
		.active_low	= 0,
	},
	{
		.name		= "eap7660d:green:ds7",
		.gpio		= EAP7660D_GPIO_DS7,
		.active_low	= 0,
	},
	{
		.name		= "eap7660d:green:ds4",
		.gpio		= EAP7660D_GPIO_DS4,
		.active_low	= 0,
	}
};

static struct gpio_keys_button eap7660d_gpio_keys[] __initdata = {
	{
		.desc		= "reset",
		.type		= EV_KEY,
		.code		= KEY_RESTART,
		.debounce_interval = EAP7660D_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EAP7660D_GPIO_SW1,
		.active_low	= 1,
	},
	{
		.desc		= "wps",
		.type		= EV_KEY,
		.code		= KEY_WPS_BUTTON,
		.debounce_interval = EAP7660D_KEYS_DEBOUNCE_INTERVAL,
		.gpio		= EAP7660D_GPIO_SW3,
		.active_low	= 1,
	}
};

static const char *eap7660d_part_probes[] = {
	"RedBoot",
	NULL,
};

static struct flash_platform_data eap7660d_flash_data = {
	.part_probes	= eap7660d_part_probes,
};

static void __init eap7660d_setup(void)
{
	u8 *boardconfig = (u8 *) KSEG1ADDR(EAP7660D_BOARDCONFIG);

	ath79_register_mdio(0, ~EAP7660D_PHYMASK);

	ath79_init_mac(ath79_eth0_data.mac_addr,
			boardconfig + EAP7660D_GBIC_MAC_OFFSET, 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_RGMII;
	ath79_eth0_data.phy_mask = EAP7660D_PHYMASK;
	ath79_register_eth(0);
	ath79_register_m25p80(&eap7660d_flash_data);
	ath79_register_leds_gpio(-1, ARRAY_SIZE(eap7660d_leds_gpio),
					eap7660d_leds_gpio);
	ath79_register_gpio_keys_polled(-1, EAP7660D_KEYS_POLL_INTERVAL,
					 ARRAY_SIZE(eap7660d_gpio_keys),
					 eap7660d_gpio_keys);
	eap7660d_pci_init(boardconfig + EAP7660D_WMAC0_CALDATA_OFFSET,
			  boardconfig + EAP7660D_WMAC0_MAC_OFFSET,
			  boardconfig + EAP7660D_WMAC1_CALDATA_OFFSET,
			  boardconfig + EAP7660D_WMAC1_MAC_OFFSET);
};

MIPS_MACHINE(ATH79_MACH_EAP7660D, "EAP7660D", "Senao EAP7660D",
	     eap7660d_setup);

MIPS_MACHINE(ATH79_MACH_ALL0305, "ALL0305", "Allnet ALL0305",
	     eap7660d_setup);
