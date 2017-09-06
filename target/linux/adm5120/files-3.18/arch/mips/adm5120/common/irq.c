/*
 *  ADM5120 specific interrupt handlers
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/bitops.h>

#include <asm/irq_cpu.h>
#include <asm/mipsregs.h>

#include <asm/mach-adm5120/adm5120_defs.h>

static void adm5120_intc_irq_unmask(struct irq_data *d);
static void adm5120_intc_irq_mask(struct irq_data *d);
static int  adm5120_intc_irq_set_type(struct irq_data *d, unsigned int flow_type);

static inline void intc_write_reg(unsigned int reg, u32 val)
{
	void __iomem *base = (void __iomem *)KSEG1ADDR(ADM5120_INTC_BASE);

	__raw_writel(val, base + reg);
}

static inline u32 intc_read_reg(unsigned int reg)
{
	void __iomem *base = (void __iomem *)KSEG1ADDR(ADM5120_INTC_BASE);

	return __raw_readl(base + reg);
}

static struct irq_chip adm5120_intc_irq_chip = {
	.name		= "INTC",
	.irq_unmask	= adm5120_intc_irq_unmask,
	.irq_mask	= adm5120_intc_irq_mask,
	.irq_mask_ack	= adm5120_intc_irq_mask,
	.irq_set_type	= adm5120_intc_irq_set_type
};

static struct irqaction adm5120_intc_irq_action = {
	.handler	= no_action,
	.name		= "cascade [INTC]"
};

static void adm5120_intc_irq_unmask(struct irq_data *d)
{
	intc_write_reg(INTC_REG_IRQ_ENABLE, 1 << (d->irq - ADM5120_INTC_IRQ_BASE));
}

static void adm5120_intc_irq_mask(struct irq_data *d)
{
	intc_write_reg(INTC_REG_IRQ_DISABLE, 1 << (d->irq - ADM5120_INTC_IRQ_BASE));
}

static int adm5120_intc_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	unsigned int irq = d->irq;
	unsigned int sense;
	unsigned long mode;
	int err = 0;

	sense = flow_type & (IRQ_TYPE_SENSE_MASK);
	switch (sense) {
	case IRQ_TYPE_NONE:
	case IRQ_TYPE_LEVEL_HIGH:
		break;
	case IRQ_TYPE_LEVEL_LOW:
		switch (irq) {
		case ADM5120_IRQ_GPIO2:
		case ADM5120_IRQ_GPIO4:
			break;
		default:
			err = -EINVAL;
			break;
		}
		break;
	default:
		err = -EINVAL;
		break;
	}

	if (err)
		return err;

	switch (irq) {
	case ADM5120_IRQ_GPIO2:
	case ADM5120_IRQ_GPIO4:
		mode = intc_read_reg(INTC_REG_INT_MODE);
		if (sense == IRQ_TYPE_LEVEL_LOW)
			mode |= (1 << (irq - ADM5120_INTC_IRQ_BASE));
		else
			mode &= ~(1 << (irq - ADM5120_INTC_IRQ_BASE));

		intc_write_reg(INTC_REG_INT_MODE, mode);
		break;
	}

	return 0;
}

static void adm5120_intc_irq_dispatch(void)
{
	unsigned long status;
	int irq;

	status = intc_read_reg(INTC_REG_IRQ_STATUS) & INTC_INT_ALL;
	if (status) {
		irq = ADM5120_INTC_IRQ_BASE + fls(status) - 1;
		do_IRQ(irq);
	} else
		spurious_interrupt();
}

asmlinkage void plat_irq_dispatch(void)
{
	unsigned long pending;

	pending = read_c0_status() & read_c0_cause() & ST0_IM;

	if (pending & STATUSF_IP7)
		do_IRQ(ADM5120_IRQ_COUNTER);
	else if (pending & STATUSF_IP2)
		adm5120_intc_irq_dispatch();
	else
		spurious_interrupt();
}

#define INTC_IRQ_STATUS (IRQ_LEVEL | IRQ_TYPE_LEVEL_HIGH | IRQ_DISABLED)
static void __init adm5120_intc_irq_init(void)
{
	int i;

	/* disable all interrupts */
	intc_write_reg(INTC_REG_IRQ_DISABLE, INTC_INT_ALL);

	/* setup all interrupts to generate IRQ instead of FIQ */
	intc_write_reg(INTC_REG_INT_MODE, 0);

	/* set active level for all external interrupts to HIGH */
	intc_write_reg(INTC_REG_INT_LEVEL, 0);

	/* disable usage of the TEST_SOURCE register */
	intc_write_reg(INTC_REG_IRQ_SOURCE_SELECT, 0);

	for (i = ADM5120_INTC_IRQ_BASE;
		i <= ADM5120_INTC_IRQ_BASE + INTC_IRQ_LAST;
		i++) {
		irq_set_chip_and_handler(i, &adm5120_intc_irq_chip,
			handle_level_irq);
	}

	setup_irq(ADM5120_IRQ_INTC, &adm5120_intc_irq_action);
}

void __init arch_init_irq(void)
{
	mips_cpu_irq_init();
	adm5120_intc_irq_init();
}
