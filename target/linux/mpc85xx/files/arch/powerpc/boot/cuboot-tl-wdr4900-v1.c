/*
 * U-Boot compatibility wrapper for the TP-Link TL-WDR4900 v1 board
 *
 * Copyright (c) 2013 Gabor Juhos <juhosg@openwrt.org>
 *
 * Based on:
 *  cuboot-85xx.c
 *     Author: Scott Wood <scottwood@freescale.com>
 *     Copyright (c) 2007 Freescale Semiconductor, Inc.
 *
 *  simpleboot.c
 *     Authors: Scott Wood <scottwood@freescale.com>
 *              Grant Likely <grant.likely@secretlab.ca>
 *     Copyright (c) 2007 Freescale Semiconductor, Inc.
 *     Copyright (c) 2008 Secret Lab Technologies Ltd.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include "ops.h"
#include "types.h"
#include "io.h"
#include "stdio.h"
#include <libfdt.h>

BSS_STACK(4*1024);

static unsigned long bus_freq;
static unsigned long int_freq;
static u64 mem_size;
static unsigned char enetaddr[6];

static void process_boot_dtb(void *boot_dtb)
{
	const u32 *na, *ns, *reg, *val32;
	const char *path;
	u64 memsize64;
	int node, size, i;

	/* Make sure FDT blob is sane */
	if (fdt_check_header(boot_dtb) != 0)
		fatal("Invalid device tree blob\n");

	/* Find the #address-cells and #size-cells properties */
	node = fdt_path_offset(boot_dtb, "/");
	if (node < 0)
		fatal("Cannot find root node\n");
	na = fdt_getprop(boot_dtb, node, "#address-cells", &size);
	if (!na || (size != 4))
		fatal("Cannot find #address-cells property");

	ns = fdt_getprop(boot_dtb, node, "#size-cells", &size);
	if (!ns || (size != 4))
		fatal("Cannot find #size-cells property");

	/* Find the memory range */
	node = fdt_node_offset_by_prop_value(boot_dtb, -1, "device_type",
					     "memory", sizeof("memory"));
	if (node < 0)
		fatal("Cannot find memory node\n");
	reg = fdt_getprop(boot_dtb, node, "reg", &size);
	if (size < (*na+*ns) * sizeof(u32))
		fatal("cannot get memory range\n");

	/* Only interested in memory based at 0 */
	for (i = 0; i < *na; i++)
		if (*reg++ != 0)
			fatal("Memory range is not based at address 0\n");

	/* get the memsize and trucate it to under 4G on 32 bit machines */
	memsize64 = 0;
	for (i = 0; i < *ns; i++)
		memsize64 = (memsize64 << 32) | *reg++;
	if (sizeof(void *) == 4 && memsize64 >= 0x100000000ULL)
		memsize64 = 0xffffffff;

	mem_size = memsize64;

	/* get clock frequencies */
	node = fdt_node_offset_by_prop_value(boot_dtb, -1, "device_type",
					     "cpu", sizeof("cpu"));
	if (!node)
		fatal("Cannot find cpu node\n");

	val32 = fdt_getprop(boot_dtb, node, "clock-frequency", &size);
	if (!val32 || (size != 4))
		fatal("Cannot get clock frequency");

	int_freq = *val32;

	val32 = fdt_getprop(boot_dtb, node, "bus-frequency", &size);
	if (!val32 || (size != 4))
		fatal("Cannot get bus frequency");

	bus_freq = *val32;

	path = fdt_get_alias(boot_dtb, "ethernet0");
	if (path) {
		const void *p;

		node = fdt_path_offset(boot_dtb, path);
		if (node < 0)
			fatal("Cannot find ethernet0 node");

		p = fdt_getprop(boot_dtb, node, "mac-address", &size);
		if (!p || (size < 6)) {
			printf("no mac-address property, finding local\n\r");
			p = fdt_getprop(boot_dtb, node, "local-mac-address", &size);
		}

		if (!p || (size < 6))
			fatal("cannot get MAC addres");

		memcpy(enetaddr, p, sizeof(enetaddr));
	}
}

static void platform_fixups(void)
{
	void *soc, *mpic;

	dt_fixup_memory(0, mem_size);

	dt_fixup_mac_address_by_alias("ethernet0", enetaddr);
	dt_fixup_cpu_clocks(int_freq, bus_freq / 8, bus_freq);

	/* Unfortunately, the specific model number is encoded in the
	 * soc node name in existing dts files -- once that is fixed,
	 * this can do a simple path lookup.
	 */
	soc = find_node_by_devtype(NULL, "soc");
	if (soc) {
		void *serial = NULL;

		setprop(soc, "bus-frequency", &bus_freq, sizeof(bus_freq));

		while ((serial = find_node_by_devtype(serial, "serial"))) {
			if (get_parent(serial) != soc)
				continue;

			setprop(serial, "clock-frequency", &bus_freq,
				sizeof(bus_freq));
		}
	}

	mpic = find_node_by_compatible(NULL, "fsl,mpic");
	if (mpic)
		setprop(mpic, "clock-frequency", &bus_freq, sizeof(bus_freq));
}

void platform_init(unsigned long r3, unsigned long r4, unsigned long r5,
                   unsigned long r6, unsigned long r7)
{
	mem_size = 64 * 1024 * 1024;

	simple_alloc_init(_end, mem_size - (u32)_end - 1024*1024, 32, 64);

	fdt_init(_dtb_start);
	serial_console_init();

	printf("\n\r-- TL-WDR4900 v1 boot wrapper --\n\r");

	process_boot_dtb((void *) r3);

	platform_ops.fixups = platform_fixups;
}
