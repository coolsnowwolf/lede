/*
 *  Atheros AP9X reference board PCI initialization
 *
 *  Copyright (C) 2009-2012 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef _ATH79_DEV_AP9X_PCI_H
#define _ATH79_DEV_AP9X_PCI_H

struct gpio_led;
struct gpio_keys_button;
struct ath9k_platform_data;

#if defined(CONFIG_ATH79_DEV_AP9X_PCI)
void ap9x_pci_setup_wmac_led_pin(unsigned wmac, int pin);
void ap9x_pci_setup_wmac_gpio(unsigned wmac, u32 mask, u32 val);
void ap9x_pci_setup_wmac_leds(unsigned wmac, struct gpio_led *leds,
			      int num_leds);
void ap9x_pci_setup_wmac_btns(unsigned wmac, struct gpio_keys_button *btns,
			      unsigned num_btns, unsigned poll_interval);
struct ath9k_platform_data *ap9x_pci_get_wmac_data(unsigned wmac);

void ap91_pci_init(u8 *cal_data, u8 *mac_addr);
void ap91_pci_init_simple(void);
void ap94_pci_init(u8 *cal_data0, u8 *mac_addr0,
		   u8 *cal_data1, u8 *mac_addr1);

#else
static inline void ap9x_pci_setup_wmac_led_pin(unsigned wmac, int pin) {}
static inline void ap9x_pci_setup_wmac_gpio(unsigned wmac,
					    u32 mask, u32 val) {}
static inline void ap9x_pci_setup_wmac_leds(unsigned wmac,
					    struct gpio_led *leds,
					    int num_leds) {}
static inline void ap9x_pci_setup_wmac_btns(unsigned wmac,
					    struct gpio_keys_button *btns,
					    unsigned num_btns,
					    unsigned poll_interval) {}
static inline struct ath9k_platform_data *ap9x_pci_get_wmac_data(unsigned wmac)
{
	return NULL;
}

static inline void ap91_pci_init(u8 *cal_data, u8 *mac_addr) {}
static inline void ap91_pci_init_simple(void) {}
static inline void ap94_pci_init(u8 *cal_data0, u8 *mac_addr0,
				 u8 *cal_data1, u8 *mac_addr1) {}
#endif

#endif /* _ATH79_DEV_AP9X_PCI_H */

