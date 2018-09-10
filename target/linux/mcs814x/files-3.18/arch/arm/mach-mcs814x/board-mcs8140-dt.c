/*
 * Setup code for Moschip MCS8140-based board using Device Tree
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under GPLv2.
 */
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/irqdomain.h>
#include <linux/of_platform.h>

#include "common.h"

#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

static void __init mcs814x_dt_device_init(void)
{
	of_platform_populate(NULL, of_default_bus_match_table, NULL, NULL);
	mcs814x_init_machine();
}

static const char *mcs8140_dt_board_compat[] __initdata = {
	"moschip,mcs8140",
	NULL,	/* sentinel */
};

DT_MACHINE_START(mcs8140_dt, "Moschip MCS8140 board")
	/* Maintainer: Florian Fainelli <florian@openwrt.org> */
	.map_io		= mcs814x_map_io,
	.init_early	= mcs814x_clk_init,
	.init_irq	= mcs814x_of_irq_init,
	.init_time	= mcs814x_timer_init,
	.init_machine	= mcs814x_dt_device_init,
	.restart	= mcs814x_restart,
	.dt_compat	= mcs8140_dt_board_compat,
	.handle_irq	= mcs814x_handle_irq,
MACHINE_END

