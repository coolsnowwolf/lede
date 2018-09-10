/*
 * ADM8668 minimal clock support
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under the terms of the GPLv2
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <adm8668.h>

struct clk {
	unsigned long rate;
};

static struct clk uart_clk = {
	.rate	= 62500000,
};

static struct clk sys_clk;

struct clk *clk_get(struct device *dev, const char *id)
{
	const char *lookup = id;

	if (dev)
		lookup = dev_name(dev);

	if (!strcmp(lookup, "apb:uart0"))
		return &uart_clk;
	if (!strcmp(lookup, "sys"))
		return &sys_clk;

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

void __init adm8668_init_clocks(void)
{
	u32 adj;

	/* adjustable clock selection
	 * CR3 bit 14~11, 0000 -> 175MHz, 0001 -> 180MHz, etc...
	 */
	adj = (ADM8668_CONFIG_REG(ADM8668_CR3) >> 11) & 0xf;
	sys_clk.rate = 175000000 + (adj * 5000000);

	pr_info("ADM8668 CPU clock: %lu MHz\n", sys_clk.rate / 1000000);
}
