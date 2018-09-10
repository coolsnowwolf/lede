/*
 *  ADM5120 minimal CLK API implementation
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This file was based on the CLK API implementation in:
 *	arch/mips/tx4938/toshiba_rbtx4938/setup.c
 *	Copyright (C) 2000-2001 Toshiba Corporation
 *	2003-2005 (c) MontaVista Software, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/device.h>

#include <asm/mach-adm5120/adm5120_defs.h>

struct clk {
	unsigned long rate;
};

static struct clk uart_clk = {
	.rate = ADM5120_UART_CLOCK
};

struct clk *clk_get(struct device *dev, const char *id)
{
	const char *name = dev_name(dev);

	if (!strcmp(name, "apb:uart0") || !strcmp(name, "apb:uart1"))
		return &uart_clk;

	return ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(clk_get);

int clk_enable(struct clk *clk)
{
	return 0;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	return clk->rate;
}
EXPORT_SYMBOL(clk_get_rate);

void clk_put(struct clk *clk)
{
}
EXPORT_SYMBOL(clk_put);
