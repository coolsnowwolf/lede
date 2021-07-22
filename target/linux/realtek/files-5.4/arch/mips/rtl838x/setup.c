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

#include <asm/addrspace.h>
#include <asm/io.h>
#include <asm/bootinfo.h>
#include <asm/reboot.h>
#include <asm/time.h>
#include <asm/prom.h>
#include <asm/smp-ops.h>

#include "mach-rtl83xx.h"

extern struct rtl83xx_soc_info soc_info;

u32 pll_reset_value;

static void rtl838x_restart(char *command)
{
	u32 pll = sw_r32(RTL838X_PLL_CML_CTRL);

	pr_info("System restart.\n");
	pr_info("PLL control register: %x, applying reset value %x\n",
		pll, pll_reset_value);

	sw_w32(3, RTL838X_INT_RW_CTRL);
	sw_w32(pll_reset_value, RTL838X_PLL_CML_CTRL);
	sw_w32(0, RTL838X_INT_RW_CTRL);

	/* Reset Global Control1 Register */
	sw_w32(1, RTL838X_RST_GLB_CTRL_1);
}

static void rtl839x_restart(char *command)
{
	/* SoC reset vector (in flash memory): on RTL839x platform preferred way to reset */
	void (*f)(void) = (void *) 0xbfc00000;

	pr_info("System restart.\n");
	/* Reset SoC */
	sw_w32(0xFFFFFFFF, RTL839X_RST_GLB_CTRL);
	/* and call reset vector */
	f();
	/* If this fails, halt the CPU */
	while
		(1);
}

static void rtl930x_restart(char *command)
{
	pr_info("System restart.\n");
	sw_w32(0x1, RTL930X_RST_GLB_CTRL_0);
	while
		(1);
}

static void rtl931x_restart(char *command)
{
	u32 v;

	pr_info("System restart.\n");
	sw_w32(1, RTL931X_RST_GLB_CTRL);
	v = sw_r32(RTL931X_RST_GLB_CTRL);
	sw_w32(0x101, RTL931X_RST_GLB_CTRL);
	msleep(15);
	sw_w32(v, RTL931X_RST_GLB_CTRL);
	msleep(15);
	sw_w32(0x101, RTL931X_RST_GLB_CTRL);
}

static void rtl838x_halt(void)
{
	pr_info("System halted.\n");
	while
		(1);
}

static void __init rtl838x_setup(void)
{
	pr_info("Registering _machine_restart\n");
	_machine_restart = rtl838x_restart;
	_machine_halt = rtl838x_halt;

	/* This PLL value needs to be restored before a reset and will then be
	 * preserved over a SoC reset. A wrong value prevents the SoC from
	 * connecting to the SPI flash controller at boot and reading the
	 * reset routine */
	pll_reset_value = sw_r32(RTL838X_PLL_CML_CTRL);

	/* Setup System LED. Bit 15 then allows to toggle it */
	sw_w32_mask(0, 3 << 16, RTL838X_LED_GLB_CTRL);
}

static void __init rtl839x_setup(void)
{
	pr_info("Registering _machine_restart\n");
	_machine_restart = rtl839x_restart;
	_machine_halt = rtl838x_halt;

	/* Setup System LED. Bit 14 of RTL839X_LED_GLB_CTRL then allows to toggle it */
	sw_w32_mask(0, 3 << 15, RTL839X_LED_GLB_CTRL);
}

static void __init rtl930x_setup(void)
{
	pr_info("Registering _machine_restart\n");
	_machine_restart = rtl930x_restart;
	_machine_halt = rtl838x_halt;

	if (soc_info.id == 0x9302)
		sw_w32_mask(0, 3 << 13, RTL9302_LED_GLB_CTRL);
	else
		sw_w32_mask(0, 3 << 13, RTL930X_LED_GLB_CTRL);
}

static void __init rtl931x_setup(void)
{
	pr_info("Registering _machine_restart\n");
	_machine_restart = rtl931x_restart;
	_machine_halt = rtl838x_halt;
	sw_w32_mask(0, 3 << 12, RTL931X_LED_GLB_CTRL);
}

void __init plat_mem_setup(void)
{
	void *dtb;

	set_io_port_base(KSEG1);
	_machine_restart = rtl838x_restart;

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
