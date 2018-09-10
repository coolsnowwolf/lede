/*
 *  Cellvision/SparkLAN CAS-771/771W support
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include "cellvision.h"

static struct adm5120_pci_irq cas771_pci_irqs[] __initdata = {
	PCIIRQ(2, 0, 1, ADM5120_IRQ_PCI0),
	PCIIRQ(3, 0, 1, ADM5120_IRQ_PCI1),
	PCIIRQ(3, 2, 3, ADM5120_IRQ_PCI2)
};

static struct gpio_led cas771_gpio_leds[] __initdata = {
	GPIO_LED_STD(ADM5120_GPIO_PIN0, "cam_flash",	NULL),
	/* GPIO PIN3 is the reset */
	GPIO_LED_STD(ADM5120_GPIO_PIN6, "access",	NULL),
	GPIO_LED_STD(ADM5120_GPIO_P0L1, "status",	NULL),
	GPIO_LED_STD(ADM5120_GPIO_P0L2, "diag",		NULL),
};

static void __init cas771_setup(void)
{
	cas7xx_setup();
	adm5120_add_device_gpio_leds(ARRAY_SIZE(cas771_gpio_leds),
					cas771_gpio_leds);
	adm5120_pci_set_irq_map(ARRAY_SIZE(cas771_pci_irqs), cas771_pci_irqs);
}

MIPS_MACHINE(MACH_ADM5120_CAS771, "CAS-771", "Cellvision CAS-771/771W",
	     cas771_setup);
