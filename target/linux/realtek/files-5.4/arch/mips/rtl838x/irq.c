// SPDX-License-Identifier: GPL-2.0-only
/*
 * Realtek RTL83XX architecture specific IRQ handling
 *
 * based on the original BSP
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 * Copyright (C) 2020 B. Koblitz
 * Copyright (C) 2020 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 John Crispin <john@phrozen.org>
 */

#include <linux/irqchip.h>
#include <linux/spinlock.h>
#include <linux/of_address.h>
#include <asm/irq_cpu.h>
#include <linux/of_irq.h>
#include <asm/cevt-r4k.h>

#include <mach-rtl83xx.h>
#include "irq.h"

#define REALTEK_CPU_IRQ_SHARED0		(MIPS_CPU_IRQ_BASE + 2)
#define REALTEK_CPU_IRQ_UART		(MIPS_CPU_IRQ_BASE + 3)
#define REALTEK_CPU_IRQ_SWITCH		(MIPS_CPU_IRQ_BASE + 4)
#define REALTEK_CPU_IRQ_SHARED1		(MIPS_CPU_IRQ_BASE + 5)
#define REALTEK_CPU_IRQ_EXTERNAL	(MIPS_CPU_IRQ_BASE + 6)
#define REALTEK_CPU_IRQ_COUNTER		(MIPS_CPU_IRQ_BASE + 7)

#define REG(x)				(rtl83xx_ictl_base + x)

extern struct rtl83xx_soc_info soc_info;

static DEFINE_RAW_SPINLOCK(irq_lock);
static void __iomem *rtl83xx_ictl_base;

static void rtl83xx_ictl_enable_irq(struct irq_data *i)
{
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&irq_lock, flags);

	value = rtl83xx_r32(REG(RTL83XX_ICTL_GIMR));
	value |= BIT(i->hwirq);
	rtl83xx_w32(value, REG(RTL83XX_ICTL_GIMR));

	raw_spin_unlock_irqrestore(&irq_lock, flags);
}

static void rtl83xx_ictl_disable_irq(struct irq_data *i)
{
	unsigned long flags;
	u32 value;

	raw_spin_lock_irqsave(&irq_lock, flags);

	value = rtl83xx_r32(REG(RTL83XX_ICTL_GIMR));
	value &= ~BIT(i->hwirq);
	rtl83xx_w32(value, REG(RTL83XX_ICTL_GIMR));

	raw_spin_unlock_irqrestore(&irq_lock, flags);
}

static struct irq_chip rtl83xx_ictl_irq = {
	.name = "RTL83xx",
	.irq_enable = rtl83xx_ictl_enable_irq,
	.irq_disable = rtl83xx_ictl_disable_irq,
	.irq_ack = rtl83xx_ictl_disable_irq,
	.irq_mask = rtl83xx_ictl_disable_irq,
	.irq_unmask = rtl83xx_ictl_enable_irq,
	.irq_eoi = rtl83xx_ictl_enable_irq,
};

static int intc_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hw)
{
	irq_set_chip_and_handler(hw, &rtl83xx_ictl_irq, handle_level_irq);

	return 0;
}

static const struct irq_domain_ops irq_domain_ops = {
	.map = intc_map,
	.xlate = irq_domain_xlate_onecell,
};

static void rtl838x_irq_dispatch(struct irq_desc *desc)
{
	unsigned int pending = rtl83xx_r32(REG(RTL83XX_ICTL_GIMR)) &
				rtl83xx_r32(REG(RTL83XX_ICTL_GISR));

	if (pending) {
		struct irq_domain *domain = irq_desc_get_handler_data(desc);
		generic_handle_irq(irq_find_mapping(domain, __ffs(pending)));
	} else {
		spurious_interrupt();
	}
}

asmlinkage void plat_rtl83xx_irq_dispatch(void)
{
	unsigned int pending;

	pending =  read_c0_cause() & read_c0_status() & ST0_IM;

	if (pending & CAUSEF_IP7)
		do_IRQ(REALTEK_CPU_IRQ_COUNTER);

	else if (pending & CAUSEF_IP6)
		do_IRQ(REALTEK_CPU_IRQ_EXTERNAL);

	else if (pending & CAUSEF_IP5)
		do_IRQ(REALTEK_CPU_IRQ_SHARED1);

	else if (pending & CAUSEF_IP4)
		do_IRQ(REALTEK_CPU_IRQ_SWITCH);

	else if (pending & CAUSEF_IP3)
		do_IRQ(REALTEK_CPU_IRQ_UART);

	else if (pending & CAUSEF_IP2)
		do_IRQ(REALTEK_CPU_IRQ_SHARED0);

	else
		spurious_interrupt();
}

static int icu_setup_domain(struct device_node *node)
{
	struct irq_domain *domain;

	domain = irq_domain_add_simple(node, 32, 0,
				       &irq_domain_ops, NULL);
	irq_set_chained_handler_and_data(2, rtl838x_irq_dispatch, domain);
	irq_set_chained_handler_and_data(3, rtl838x_irq_dispatch, domain);
	irq_set_chained_handler_and_data(4, rtl838x_irq_dispatch, domain);
	irq_set_chained_handler_and_data(5, rtl838x_irq_dispatch, domain);

	rtl83xx_ictl_base = of_iomap(node, 0);
	if (!rtl83xx_ictl_base)
		return -EINVAL;

	return 0;
}

static void __init rtl8380_icu_of_init(struct device_node *node, struct device_node *parent)
{
	if (icu_setup_domain(node))
		return;

	/* Disable all cascaded interrupts */
	rtl83xx_w32(0, REG(RTL83XX_ICTL_GIMR));

	/* Set up interrupt routing */
	rtl83xx_w32(RTL83XX_IRR0_SETTING, REG(RTL83XX_IRR0));
	rtl83xx_w32(RTL83XX_IRR1_SETTING, REG(RTL83XX_IRR1));
	rtl83xx_w32(RTL83XX_IRR2_SETTING, REG(RTL83XX_IRR2));
	rtl83xx_w32(RTL83XX_IRR3_SETTING, REG(RTL83XX_IRR3));

	/* Clear timer interrupt */
	write_c0_compare(0);

	/* Enable all CPU interrupts */
	write_c0_status(read_c0_status() | ST0_IM);

	/* Enable timer0 and uart0 interrupts */
	rtl83xx_w32(BIT(RTL83XX_IRQ_TC0) | BIT(RTL83XX_IRQ_UART0), REG(RTL83XX_ICTL_GIMR));
}

static void __init rtl8390_icu_of_init(struct device_node *node, struct device_node *parent)
{
	if (icu_setup_domain(node))
		return;

	/* Disable all cascaded interrupts */
	rtl83xx_w32(0, REG(RTL83XX_ICTL_GIMR));

	/* Set up interrupt routing */
	rtl83xx_w32(RTL83XX_IRR0_SETTING, REG(RTL83XX_IRR0));
	rtl83xx_w32(RTL8390_IRR1_SETTING, REG(RTL83XX_IRR1));
	rtl83xx_w32(RTL83XX_IRR2_SETTING, REG(RTL83XX_IRR2));
	rtl83xx_w32(RTL83XX_IRR3_SETTING, REG(RTL83XX_IRR3));

	/* Clear timer interrupt */
	write_c0_compare(0);

	/* Enable all CPU interrupts */
	write_c0_status(read_c0_status() | ST0_IM);

	/* Enable timer0 and uart0 interrupts */
	rtl83xx_w32(BIT(RTL83XX_IRQ_TC0) | BIT(RTL83XX_IRQ_UART0), REG(RTL83XX_ICTL_GIMR));
}

static void __init rtl9300_icu_of_init(struct device_node *node, struct device_node *parent)
{
	pr_info("RTL9300: Setting up IRQs\n");
	if (icu_setup_domain(node))
		return;

	/* Disable all cascaded interrupts */
	rtl83xx_w32(0, REG(RTL83XX_ICTL_GIMR));

	/* Set up interrupt routing */
	rtl83xx_w32(RTL9300_IRR0_SETTING, REG(RTL83XX_IRR0));
	rtl83xx_w32(RTL9300_IRR1_SETTING, REG(RTL83XX_IRR1));
	rtl83xx_w32(RTL9300_IRR2_SETTING, REG(RTL83XX_IRR2));
	rtl83xx_w32(RTL9300_IRR3_SETTING, REG(RTL83XX_IRR3));

	/* Clear timer interrupt */
	write_c0_compare(0);

	/* Enable all CPU interrupts */
	write_c0_status(read_c0_status() | ST0_IM);
}

static struct of_device_id __initdata of_irq_ids[] = {
	{ .compatible = "mti,cpu-interrupt-controller", .data = mips_cpu_irq_of_init },
	{ .compatible = "realtek,rt8380-intc", .data = rtl8380_icu_of_init },
	{ .compatible = "realtek,rt8390-intc", .data = rtl8390_icu_of_init },
	{ .compatible = "realtek,rt9300-intc", .data = rtl9300_icu_of_init },
	{},
};

void __init arch_init_irq(void)
{
	of_irq_init(of_irq_ids);
}
