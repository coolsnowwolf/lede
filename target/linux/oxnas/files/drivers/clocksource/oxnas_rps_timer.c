/*
 * arch/arm/mach-ox820/rps-time.c
 *
 * Copyright (C) 2009 Oxford Semiconductor Ltd
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/clockchips.h>
#include <linux/clk.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#include <linux/sched_clock.h>
#include <mach/hardware.h>

enum {
	TIMER_LOAD = 0,
	TIMER_CURR = 4,
	TIMER_CTRL = 8,
	TIMER_CLRINT = 0xC,

	TIMER_BITS = 24,

	TIMER_MAX_VAL = (1 << TIMER_BITS) - 1,

	TIMER_PERIODIC = (1 << 6),
	TIMER_ENABLE = (1 << 7),

	TIMER_DIV1  = (0 << 2),
	TIMER_DIV16  = (1 << 2),
	TIMER_DIV256  = (2 << 2),

	TIMER1_OFFSET = 0,
	TIMER2_OFFSET = 0x20,

};

static u64 notrace rps_read_sched_clock(void)
{
	return ~readl_relaxed(RPSA_TIMER2_VAL);
}

static void __init rps_clocksource_init(void __iomem *base, ulong ref_rate)
{
	int ret;
	ulong clock_rate;
	/* use prescale 16 */
	clock_rate = ref_rate / 16;

	iowrite32(TIMER_MAX_VAL, base + TIMER_LOAD);
	iowrite32(TIMER_PERIODIC | TIMER_ENABLE | TIMER_DIV16,
			base + TIMER_CTRL);

	ret = clocksource_mmio_init(base + TIMER_CURR, "rps_clocksource_timer",
					clock_rate, 250, TIMER_BITS,
					clocksource_mmio_readl_down);
	if (ret)
		panic("can't register clocksource\n");

	sched_clock_register(rps_read_sched_clock, TIMER_BITS, clock_rate);
}

static void __init rps_timer_init(struct device_node *np)
{
	struct clk *refclk;
	unsigned long ref_rate;
	void __iomem *base;

	refclk = of_clk_get(np, 0);

	if (IS_ERR(refclk) || clk_prepare_enable(refclk))
		panic("rps_timer_init: failed to get refclk\n");
	ref_rate = clk_get_rate(refclk);

	base = of_iomap(np, 0);
	if (!base)
		panic("rps_timer_init: failed to map io\n");

	rps_clocksource_init(base + TIMER2_OFFSET, ref_rate);
}

CLOCKSOURCE_OF_DECLARE(nas782x, "plxtech,nas782x-rps-timer", rps_timer_init);
