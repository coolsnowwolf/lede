/*
 *  Atheros AP9X reference board PCI initialization
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/pci.h>
#include <linux/ath9k_platform.h>
#include <linux/delay.h>

#include <asm/mach-ath79/ath79.h>

#include "dev-ap9x-pci.h"
#include "pci-ath9k-fixup.h"
#include "pci.h"

static struct ath9k_platform_data ap9x_wmac0_data = {
	.led_pin = -1,
};
static struct ath9k_platform_data ap9x_wmac1_data = {
	.led_pin = -1,
};
static char ap9x_wmac0_mac[6];
static char ap9x_wmac1_mac[6];

__init void ap9x_pci_setup_wmac_led_pin(unsigned wmac, int pin)
{
	switch (wmac) {
	case 0:
		ap9x_wmac0_data.led_pin = pin;
		break;
	case 1:
		ap9x_wmac1_data.led_pin = pin;
		break;
	}
}

__init struct ath9k_platform_data *ap9x_pci_get_wmac_data(unsigned wmac)
{
	switch (wmac) {
	case 0:
		return &ap9x_wmac0_data;

	case 1:
		return &ap9x_wmac1_data;
	}

	return NULL;
}

__init void ap9x_pci_setup_wmac_gpio(unsigned wmac, u32 mask, u32 val)
{
	switch (wmac) {
	case 0:
		ap9x_wmac0_data.gpio_mask = mask;
		ap9x_wmac0_data.gpio_val = val;
		break;
	case 1:
		ap9x_wmac1_data.gpio_mask = mask;
		ap9x_wmac1_data.gpio_val = val;
		break;
	}
}

__init void ap9x_pci_setup_wmac_leds(unsigned wmac, struct gpio_led *leds,
				     int num_leds)
{
	switch (wmac) {
	case 0:
		ap9x_wmac0_data.leds = leds;
		ap9x_wmac0_data.num_leds = num_leds;
		break;
	case 1:
		ap9x_wmac1_data.leds = leds;
		ap9x_wmac1_data.num_leds = num_leds;
		break;
	}
}

__init void ap9x_pci_setup_wmac_btns(unsigned wmac,
				     struct gpio_keys_button *btns,
				     unsigned num_btns, unsigned poll_interval)
{
	struct ath9k_platform_data *ap9x_wmac_data;

	if (!(ap9x_wmac_data = ap9x_pci_get_wmac_data(wmac)))
		return;

	ap9x_wmac_data->btns = btns;
	ap9x_wmac_data->num_btns = num_btns;
	ap9x_wmac_data->btn_poll_interval = poll_interval;
}

static int ap91_pci_plat_dev_init(struct pci_dev *dev)
{
	switch (PCI_SLOT(dev->devfn)) {
	case 0:
		dev->dev.platform_data = &ap9x_wmac0_data;
		break;
	}

	return 0;
}

__init void ap91_pci_init(u8 *cal_data, u8 *mac_addr)
{
	if (cal_data)
		memcpy(ap9x_wmac0_data.eeprom_data, cal_data,
		       sizeof(ap9x_wmac0_data.eeprom_data));

	if (mac_addr) {
		memcpy(ap9x_wmac0_mac, mac_addr, sizeof(ap9x_wmac0_mac));
		ap9x_wmac0_data.macaddr = ap9x_wmac0_mac;
	}

	ath79_pci_set_plat_dev_init(ap91_pci_plat_dev_init);
	ath79_register_pci();

	pci_enable_ath9k_fixup(0, ap9x_wmac0_data.eeprom_data);
}

__init void ap91_pci_init_simple(void)
{
	ap91_pci_init(NULL, NULL);
	ap9x_wmac0_data.eeprom_name = "pci_wmac0.eeprom";
}

static int ap94_pci_plat_dev_init(struct pci_dev *dev)
{
	switch (PCI_SLOT(dev->devfn)) {
	case 17:
		dev->dev.platform_data = &ap9x_wmac0_data;
		break;

	case 18:
		dev->dev.platform_data = &ap9x_wmac1_data;
		break;
	}

	return 0;
}

__init void ap94_pci_init(u8 *cal_data0, u8 *mac_addr0,
			  u8 *cal_data1, u8 *mac_addr1)
{
	if (cal_data0)
		memcpy(ap9x_wmac0_data.eeprom_data, cal_data0,
		       sizeof(ap9x_wmac0_data.eeprom_data));

	if (cal_data1)
		memcpy(ap9x_wmac1_data.eeprom_data, cal_data1,
		       sizeof(ap9x_wmac1_data.eeprom_data));

	if (mac_addr0) {
		memcpy(ap9x_wmac0_mac, mac_addr0, sizeof(ap9x_wmac0_mac));
		ap9x_wmac0_data.macaddr = ap9x_wmac0_mac;
	}

	if (mac_addr1) {
		memcpy(ap9x_wmac1_mac, mac_addr1, sizeof(ap9x_wmac1_mac));
		ap9x_wmac1_data.macaddr = ap9x_wmac1_mac;
	}

	ath79_pci_set_plat_dev_init(ap94_pci_plat_dev_init);
	ath79_register_pci();

	pci_enable_ath9k_fixup(17, ap9x_wmac0_data.eeprom_data);
	pci_enable_ath9k_fixup(18, ap9x_wmac1_data.eeprom_data);
}
