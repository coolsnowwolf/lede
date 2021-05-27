// SPDX-License-Identifier: GPL-2.0-only

#include <linux/clockchips.h>
#include <linux/init.h>
#include <asm/time.h>
#include <linux/interrupt.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/sched_clock.h>
#include "timer-of.h"

#include <mach-rtl83xx.h>

/* 
 * Timer registers
 * the RTL9300/9310 SoCs have 6 timers, each register block 0x10 apart
 */
#define RTL9300_TC_DATA		0x0
#define RTL9300_TC_CNT		0x4
#define RTL9300_TC_CTRL		0x8
#define RTL9300_TC_CTRL_MODE	BIT(24)
#define RTL9300_TC_CTRL_EN	BIT(28)
#define RTL9300_TC_INT		0xc
#define RTL9300_TC_INT_IP	BIT(16)
#define RTL9300_TC_INT_IE	BIT(20)

// Clocksource is using timer 0, clock event uses timer 1
#define TIMER_CLK_SRC		0
#define TIMER_CLK_EVT		1
#define TIMER_BLK_EVT		(TIMER_CLK_EVT << 4)

// Timer modes
#define TIMER_MODE_REPEAT	1
#define TIMER_MODE_ONCE		0

// Minimum divider is 2
#define DIVISOR_RTL9300		2

#define N_BITS			28

static void __iomem *rtl9300_sched_reg __read_mostly;

static u64 notrace rtl9300_sched_clock_read(void)
{
/*	pr_info("In %s: %x\n", __func__, readl_relaxed(rtl9300_sched_reg));
	dump_stack();*/
	return readl_relaxed(rtl9300_sched_reg);
}

static irqreturn_t rtl9300_timer_interrupt(int irq, void *dev_id)
{
	struct clock_event_device *clk = dev_id;
	struct timer_of *to = to_timer_of(clk);
	u32 v = readl(timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_INT);

	// Acknowledge the IRQ
	v |= RTL9300_TC_INT_IP;
	writel(v, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_INT);

	clk->event_handler(clk);
	return IRQ_HANDLED;
}

static void rtl9300_timer_stop(struct timer_of *to)
{
	u32 v;

	writel(0, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_CTRL);

	// Acknowledge possibly pending IRQ
	v = readl(timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_INT);
	if (v & RTL9300_TC_INT_IP)
		writel(v, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_INT);
}

static void rtl9300_timer_start(struct timer_of *to, int timer, bool periodic)
{
	u32 v = (periodic ? RTL9300_TC_CTRL_MODE : 0) | RTL9300_TC_CTRL_EN | DIVISOR_RTL9300;
	writel(v, timer_of_base(to) + timer * 0x10 + RTL9300_TC_CTRL);
}

static int rtl9300_set_next_event(unsigned long delta, struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	rtl9300_timer_stop(to);
	writel(delta, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_DATA);
	rtl9300_timer_start(to, TIMER_CLK_EVT, TIMER_MODE_ONCE);
	return 0;
}

static int rtl9300_set_state_periodic(struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	rtl9300_timer_stop(to);
	writel(to->of_clk.period, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_DATA);
	rtl9300_timer_start(to, TIMER_CLK_EVT, TIMER_MODE_REPEAT);
	return 0;
}

static int rtl9300_set_state_oneshot(struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	rtl9300_timer_stop(to);
	writel(to->of_clk.period, timer_of_base(to) + TIMER_BLK_EVT + RTL9300_TC_DATA);
	rtl9300_timer_start(to, TIMER_CLK_EVT, TIMER_MODE_ONCE);
	return 0;
}

static int rtl9300_set_state_shutdown(struct clock_event_device *clk)
{
	struct timer_of *to = to_timer_of(clk);

	rtl9300_timer_stop(to);
	return 0;
}

static struct timer_of t_of = {
	.flags = TIMER_OF_BASE | TIMER_OF_IRQ | TIMER_OF_CLOCK,

	.clkevt = {
		.name = "rtl9300_timer",
		.rating = 350,
		.features = CLOCK_EVT_FEAT_PERIODIC | CLOCK_EVT_FEAT_ONESHOT,
		.set_next_event	= rtl9300_set_next_event,
		.set_state_oneshot = rtl9300_set_state_oneshot,
		.set_state_periodic = rtl9300_set_state_periodic,
		.set_state_shutdown = rtl9300_set_state_shutdown,
	},

	.of_irq = {
		.name = "ostimer",
		.handler = rtl9300_timer_interrupt,
		.flags = IRQF_TIMER,
	},
};

static void __init rtl9300_timer_setup(u8 timer)
{
	u32 v;

	// Disable timer
	writel(0, timer_of_base(&t_of) + 0x10 * timer + RTL9300_TC_CTRL);

	// Acknowledge possibly pending IRQ
	v = readl(timer_of_base(&t_of) + 0x10 * timer + RTL9300_TC_INT);
	if (v & RTL9300_TC_INT_IP)
		writel(v, timer_of_base(&t_of) + 0x10 * timer + RTL9300_TC_INT);

	// Setup maximum period (for use as clock-source)
	writel(0x0fffffff, timer_of_base(&t_of) + 0x10 * timer + RTL9300_TC_DATA);
}


static int __init rtl9300_timer_init(struct device_node *node)
{
	int err = 0;
	unsigned long rate;

	pr_info("%s: setting up timer\n", __func__);

	err = timer_of_init(node, &t_of);
	if (err)
		return err;

	rate = timer_of_rate(&t_of) / DIVISOR_RTL9300;
	pr_info("Frequency in dts: %ld, my rate is %ld, period %ld\n",
		timer_of_rate(&t_of), rate, timer_of_period(&t_of));
	pr_info("With base %08x IRQ: %d\n", (u32)timer_of_base(&t_of), timer_of_irq(&t_of));

	// Configure clock source and register it for scheduling
	rtl9300_timer_setup(TIMER_CLK_SRC);
	rtl9300_timer_start(&t_of, TIMER_CLK_SRC, TIMER_MODE_REPEAT);

	rtl9300_sched_reg = timer_of_base(&t_of) + TIMER_CLK_SRC * 0x10 + RTL9300_TC_CNT;

	err = clocksource_mmio_init(rtl9300_sched_reg, node->name, rate , 100, N_BITS,
				    clocksource_mmio_readl_up);
	if (err)
		return err;

	sched_clock_register(rtl9300_sched_clock_read, N_BITS, rate);

	// Configure clock event source
	rtl9300_timer_setup(TIMER_CLK_EVT);
	clockevents_config_and_register(&t_of.clkevt, rate, 100, 0x0fffffff);

	// Enable interrupt
	writel(RTL9300_TC_INT_IE, timer_of_base(&t_of) + TIMER_BLK_EVT + RTL9300_TC_INT);

	return err;
}

TIMER_OF_DECLARE(rtl9300_timer, "realtek,rtl9300-timer", rtl9300_timer_init);
