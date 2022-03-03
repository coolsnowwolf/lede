// SPDX-License-Identifier: GPL-2.0-only
/*
 * Setup for the Realtek RTL838X SoC:
 *	Memory, Timer and Serial
 *
 * Copyright (C) 2020 B. Koblitz
 * based on the original BSP by
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 *
 */

#include <linux/console.h>
#include <linux/init.h>
#include <linux/clkdev.h>
#include <linux/clk-provider.h>
#include <linux/delay.h>
#include <linux/of_fdt.h>
#include <linux/irqchip.h>

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/bootinfo.h>
#include <asm/time.h>
#include <asm/prom.h>
#include <asm/smp-ops.h>

#include "mach-rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;

static void __init rtl838x_setup(void)
{
	/* Setup System LED. Bit 15 then allows to toggle it */
	sw_w32_mask(0, 3 << 16, RTL838X_LED_GLB_CTRL);
}

static void __init rtl839x_setup(void)
{
	/* Setup System LED. Bit 14 of RTL839X_LED_GLB_CTRL then allows to toggle it */
	sw_w32_mask(0, 3 << 15, RTL839X_LED_GLB_CTRL);
}

static void __init rtl930x_setup(void)
{
	if (soc_info.id == 0x9302)
		sw_w32_mask(0, 3 << 13, RTL9302_LED_GLB_CTRL);
	else
		sw_w32_mask(0, 3 << 13, RTL930X_LED_GLB_CTRL);
}

static void __init rtl931x_setup(void)
{
	sw_w32_mask(0, 3 << 12, RTL931X_LED_GLB_CTRL);
}

void __init plat_mem_setup(void)
{
	void *dtb;

	set_io_port_base(KSEG1);

	if (fw_passed_dtb) /* UHI interface */
		dtb = (void *)fw_passed_dtb;
	else if (__dtb_start != __dtb_end)
		dtb = (void *)__dtb_start;
	else
		panic("no dtb found");

	/*
	 * Load the devicetree. This causes the chosen node to be
	 * parsed resulting in our memory appearing
	 */
	__dt_setup_arch(dtb);

	switch (soc_info.family) {
	case RTL8380_FAMILY_ID:
		rtl838x_setup();
		break;
	case RTL8390_FAMILY_ID:
		rtl839x_setup();
		break;
	case RTL9300_FAMILY_ID:
		rtl930x_setup();
		break;
	case RTL9310_FAMILY_ID:
		rtl931x_setup();
		break;
	}
}

void __init plat_time_init(void)
{
	struct device_node *np;
	u32 freq = 500000000;

	of_clk_init(NULL);
	timer_probe();

	np = of_find_node_by_name(NULL, "cpus");
	if (!np) {
		pr_err("Missing 'cpus' DT node, using default frequency.");
	} else {
		if (of_property_read_u32(np, "frequency", &freq) < 0)
			pr_err("No 'frequency' property in DT, using default.");
		else
			pr_info("CPU frequency from device tree: %dMHz", freq / 1000000);
		of_node_put(np);
	}

	mips_hpt_frequency = freq / 2;
}

void __init arch_init_irq(void)
{
	irqchip_init();
}
