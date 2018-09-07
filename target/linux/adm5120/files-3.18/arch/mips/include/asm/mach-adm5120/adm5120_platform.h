/*
 *  ADM5120 specific platform definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _ASM_MIPS_MACH_ADM5120_PLATFORM_H
#define _ASM_MIPS_MACH_ADM5120_PLATFORM_H

#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/leds.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/amba/bus.h>
#include <linux/amba/serial.h>

struct adm5120_flash_platform_data {
	void			(*set_vpp)(struct map_info *, int);
	void			(*switch_bank)(unsigned);
	u32			window_size;
	unsigned int		nr_parts;
	struct mtd_partition	*parts;
};

struct adm5120_switch_platform_data {
	/* TODO: not yet implemented */
};

struct adm5120_pci_irq {
	u8	slot;
	u8	func;
	u8	pin;
	unsigned irq;
};

#define PCIIRQ(s, f, p, i) {.slot = (s), .func = (f), .pin  = (p), .irq  = (i)}

#ifdef CONFIG_PCI
extern void adm5120_pci_set_irq_map(unsigned int nr_irqs,
		struct adm5120_pci_irq *map) __init;
#else
static inline void adm5120_pci_set_irq_map(unsigned int nr_irqs,
		struct adm5120_pci_irq *map)
{
}
#endif

extern void adm5120_setup_eth_macs(u8 *mac_base) __init;

extern struct adm5120_flash_platform_data adm5120_flash0_data;
extern struct adm5120_flash_platform_data adm5120_flash1_data;

extern void adm5120_add_device_flash(unsigned id) __init;
extern void adm5120_add_device_usb(void) __init;
extern void adm5120_add_device_uart(unsigned id) __init;
extern void adm5120_add_device_nand(struct platform_nand_data *pdata) __init;
extern void adm5120_add_device_switch(unsigned num_ports, u8 *vlan_map) __init;
extern void adm5120_register_gpio_buttons(int id,
					  unsigned poll_interval,
					  unsigned nbuttons,
					  struct gpio_keys_button *buttons);

#define GPIO_LED_DEF(g, n, t, a) {	\
	.name = (n),			\
	.default_trigger = (t),		\
	.gpio = (g),			\
	.active_low = (a)		\
}

#define GPIO_LED_STD(g, n, t)	GPIO_LED_DEF((g), (n), (t), 0)
#define GPIO_LED_INV(g, n, t)	GPIO_LED_DEF((g), (n), (t), 1)

extern void adm5120_add_device_gpio_leds(unsigned num_leds,
					struct gpio_led *leds) __init;

#endif /* _ASM_MIPS_MACH_ADM5120_PLATFORM_H */
