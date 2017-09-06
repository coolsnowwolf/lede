/*
 * Moschip MCS814x clock routines
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under GPLv2
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/export.h>
#include <linux/spinlock.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/clkdev.h>
#include <linux/clk.h>

#include <mach/mcs814x.h>

#include "common.h"

#define KHZ	1000
#define MHZ	(KHZ * KHZ)

struct clk_ops {
	unsigned long (*get_rate)(struct clk *clk);
	int (*set_rate)(struct clk *clk, unsigned long rate);
	struct clk *(*get_parent)(struct clk *clk);
	int (*enable)(struct clk *clk, int enable);
};

struct clk {
	struct clk *parent;     	/* parent clk */
	unsigned long rate;     	/* clock rate in Hz */
	unsigned long divider;		/* clock divider */
	u32 usecount;			/* reference count */
	struct clk_ops *ops;		/* clock operation */
	u32 enable_reg;			/* clock enable register */
	u32 enable_mask;		/* clock enable mask */
};

static unsigned long clk_divide_parent(struct clk *clk)
{
	if (clk->parent && clk->divider)
		return clk_get_rate(clk->parent) / clk->divider;
	else
		return 0;
}

static int clk_local_onoff_enable(struct clk *clk, int enable)
{
	u32 tmp;

	/* no enable_reg means the clock is always enabled */
	if (!clk->enable_reg)
		return 0;

	tmp = readl_relaxed(mcs814x_sysdbg_base + clk->enable_reg);
	if (!enable)
		tmp &= ~clk->enable_mask;
	else
		tmp |= clk->enable_mask;

	writel_relaxed(tmp, mcs814x_sysdbg_base + clk->enable_reg);

	return 0;
}

static struct clk_ops default_clk_ops = {
	.get_rate	= clk_divide_parent,
	.enable		= clk_local_onoff_enable,
};

static DEFINE_SPINLOCK(clocks_lock);

static const unsigned long cpu_freq_table[] = {
	175000,
	300000,
	125000,
	137500,
	212500,
	250000,
	162500,
	187500,
	162500,
	150000,
	225000,
	237500,
	200000,
	262500,
	275000,
	287500
};

static struct clk clk_cpu;

/* System clock is fixed at 50Mhz */
static struct clk clk_sys = {
	.rate	= 50 * MHZ,
};

static struct clk clk_sdram;

static struct clk clk_timer0 = {
	.parent	= &clk_sdram,
	.divider = 2,
	.ops	= &default_clk_ops,
};

static struct clk clk_timer1_2 = {
	.parent	= &clk_sys,
};

/* Watchdog clock is system clock / 128 */
static struct clk clk_wdt = {
	.parent	= &clk_sys,
	.divider = 128,
	.ops	= &default_clk_ops,
};

static struct clk clk_emac = {
	.ops		= &default_clk_ops,
	.enable_reg	= SYSDBG_SYSCTL,
	.enable_mask	= SYSCTL_EMAC,
};

static struct clk clk_ephy = {
	.ops		= &default_clk_ops,
	.enable_reg	= SYSDBG_PLL_CTL,
	.enable_mask	= ~SYSCTL_EPHY,	/* active low */
};

static struct clk clk_cipher = {
	.ops		= &default_clk_ops,
	.enable_reg	= SYSDBG_SYSCTL,
	.enable_mask	= SYSCTL_CIPHER,
};

#define CLK(_dev, _con, _clk)	\
{ .dev_id = (_dev), .con_id = (_con), .clk = (_clk) },

static struct clk_lookup mcs814x_chip_clks[] = {
	CLK("cpu", NULL, &clk_cpu)
	CLK("sys", NULL, &clk_sys)
	CLK("sdram", NULL, &clk_sdram)
	/* 32-bits timer0 */
	CLK("timer0", NULL, &clk_timer0)
	/* 16-bits timer1 */
	CLK("timer1", NULL, &clk_timer1_2)
	/* 64-bits timer2, same as timer 1 */
	CLK("timer2", NULL, &clk_timer1_2)
	CLK(NULL, "wdt", &clk_wdt)
	CLK(NULL, "emac", &clk_emac)
	CLK(NULL, "ephy", &clk_ephy)
	CLK(NULL, "cipher", &clk_cipher)
};

static void local_clk_disable(struct clk *clk)
{
	WARN_ON(!clk->usecount);

	if (clk->usecount > 0) {
		clk->usecount--;

		if ((clk->usecount == 0) && (clk->ops->enable))
			clk->ops->enable(clk, 0);

		if (clk->parent)
			local_clk_disable(clk->parent);
	}
}

static int local_clk_enable(struct clk *clk)
{
	int ret = 0;

	if (clk->parent)
		ret = local_clk_enable(clk->parent);

	if (ret)
		return ret;

	if ((clk->usecount == 0) && (clk->ops->enable))
		ret = clk->ops->enable(clk, 1);

	if (!ret)
		clk->usecount++;
	else if (clk->parent && clk->parent->ops->enable)
		local_clk_disable(clk->parent);

	return ret;
}

int clk_enable(struct clk *clk)
{
	int ret;
	unsigned long flags;

	spin_lock_irqsave(&clocks_lock, flags);
	ret = local_clk_enable(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);

	return ret;
}
EXPORT_SYMBOL(clk_enable);

void clk_disable(struct clk *clk)
{
	unsigned long flags;

	spin_lock_irqsave(&clocks_lock, flags);
	local_clk_disable(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);
}
EXPORT_SYMBOL(clk_disable);

unsigned long clk_get_rate(struct clk *clk)
{
	if (unlikely(IS_ERR_OR_NULL(clk)))
		return 0;

	if (clk->rate)
		return clk->rate;

	if (clk->ops && clk->ops->get_rate)
		return clk->ops->get_rate(clk);

	return clk_get_rate(clk->parent);
}
EXPORT_SYMBOL(clk_get_rate);

struct clk *clk_get_parent(struct clk *clk)
{
	unsigned long flags;

	if (unlikely(IS_ERR_OR_NULL(clk)))
		return NULL;

	if (!clk->ops || !clk->ops->get_parent)
		return clk->parent;

	spin_lock_irqsave(&clocks_lock, flags);
	clk->parent = clk->ops->get_parent(clk);
	spin_unlock_irqrestore(&clocks_lock, flags);

	return clk->parent;
}
EXPORT_SYMBOL(clk_get_parent);

void __init mcs814x_clk_init(void)
{
	u32 bs1;
	u8 cpu_freq;

	clkdev_add_table(mcs814x_chip_clks, ARRAY_SIZE(mcs814x_chip_clks));

	/* read the bootstrap registers to know the exact clocking scheme */
	bs1 = readl_relaxed(mcs814x_sysdbg_base + SYSDBG_BS1);
	cpu_freq = (bs1 >> CPU_FREQ_SHIFT) & CPU_FREQ_MASK;

	pr_info("CPU frequency: %lu (kHz)\n", cpu_freq_table[cpu_freq]);
	clk_cpu.rate = cpu_freq * KHZ;

	/* read SDRAM frequency */
	if (bs1 & SDRAM_FREQ_BIT)
		clk_sdram.rate = 100 * MHZ;
	else
		clk_sdram.rate = 133 * MHZ;

	pr_info("SDRAM frequency: %lu (MHz)\n", clk_sdram.rate / MHZ);
}

