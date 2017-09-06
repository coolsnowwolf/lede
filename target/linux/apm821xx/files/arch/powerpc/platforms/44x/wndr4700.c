/*
 * Netgear Centria N900 WNDR4700/WNDR4720 platform support
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This implemention is based on the simple platform support for the
 * PowerPC 44x chips.
 */

#include <asm/machdep.h>
#include <asm/pci-bridge.h>
#include <asm/ppc4xx.h>
#include <asm/prom.h>
#include <asm/time.h>
#include <asm/udbg.h>
#include <asm/uic.h>

#include <linux/init.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/ath9k_platform.h>
#include <linux/firmware.h>
#include <linux/delay.h>
#include <asm/unaligned.h>

static const struct of_device_id ppc44x_of_bus[] __initconst = {
	{ .compatible = "ibm,plb4", },
	{ .compatible = "ibm,opb", },
	{ .compatible = "ibm,ebc", },
	{ .compatible = "simple-bus", },
	{},
};

static int __init ppc44x_device_probe(void)
{
	of_platform_bus_probe(NULL, ppc44x_of_bus, NULL);

	return 0;
}
machine_device_initcall(wndr4700, ppc44x_device_probe);

static char *board[] __initdata = {
	"netgear,wndr4700",
};

static int __init ppc44x_probe(void)
{
	unsigned long root = of_get_flat_dt_root();
	int i = 0;

	pcie_bus_config = PCIE_BUS_PEER2PEER; /* force 128 Byte MPS */

	for (i = 0; i < ARRAY_SIZE(board); i++) {
		if (of_flat_dt_is_compatible(root, board[i])) {
			pci_set_flags(PCI_REASSIGN_ALL_RSRC); /* PCI_PROBE_ONLY */
			return 1;
		}
	}

	return 0;
}

define_machine(wndr4700) {
	.name = "WNDR4700 Platform",
	.probe = ppc44x_probe,
	.progress = udbg_progress,
	.init_IRQ = uic_init_tree,
	.get_irq = uic_get_irq,
	.restart = ppc4xx_reset_system,
	.calibrate_decr = generic_calibrate_decr,
};

static struct ath9k_platform_data ar9380_wmac0_data = {
	.led_pin = -1,
	.eeprom_name = "pci_wmac1.eeprom",
};
static struct ath9k_platform_data ar9580_wmac1_data = {
	.led_pin = -1,
	.eeprom_name = "pci_wmac0.eeprom",
};

static int __init wndr4700_ath9k_eeprom_load(void)
{
	struct pci_dev *dev;

	dev = pci_get_device(PCI_VENDOR_ID_ATHEROS, 0x0030, NULL);
	if (dev)
		dev->dev.platform_data = &ar9380_wmac0_data;

	dev = pci_get_device(PCI_VENDOR_ID_ATHEROS, 0x0033, NULL);
	if (dev)
		dev->dev.platform_data = &ar9580_wmac1_data;

	return 0;
}
machine_device_initcall(wndr4700, wndr4700_ath9k_eeprom_load);
