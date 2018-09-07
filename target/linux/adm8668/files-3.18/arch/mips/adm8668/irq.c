/*
 * Copyright (C) 2010 Scott Nicholas <neutronscott@scottn.us>
 * Copyright (C) 2012 Florian Fainelli <florian@openwrt.org>
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */

#include <linux/init.h>
#include <linux/kernel_stat.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/random.h>
#include <linux/pm.h>
#include <linux/irq.h>
#include <asm/mipsregs.h>
#include <asm/irq_cpu.h>
#include <asm/irq.h>
#include <adm8668.h>

/* interrupt controller */
#define IRQ_STATUS_REG		0x00	/* Read */
#define IRQ_ENABLE_REG		0x08	/* Read/Write */
#define IRQ_DISABLE_REG		0x0C	/* Write */

#define IRQ_MASK		0xffff

static inline void intc_write_reg(u32 val, unsigned int reg)
{
	void __iomem *base = (void __iomem *)KSEG1ADDR(ADM8668_INTC_BASE);

	__raw_writel(val, base + reg);
}

static inline u32 intc_read_reg(unsigned int reg)
{
	void __iomem *base = (void __iomem *)KSEG1ADDR(ADM8668_INTC_BASE);

	return __raw_readl(base + reg);
}

static void adm8668_irq_cascade(void)
{
	int irq;
	u32 intsrc;

	intsrc = intc_read_reg(IRQ_STATUS_REG) & IRQ_MASK;
	if (intsrc) {
		irq = fls(intsrc) - 1;
		do_IRQ(irq);
	} else
		spurious_interrupt();
}

/*
 * System irq dispatch
 */
void plat_irq_dispatch(void)
{
	unsigned int pending;

	pending = read_c0_cause() & read_c0_status() & ST0_IM;

	/* timer interrupt, that we renumbered */
	if (pending & STATUSF_IP7)
		do_IRQ(MIPS_CPU_IRQ_BASE + 7);
	else if (pending & STATUSF_IP2)
		adm8668_irq_cascade();
	else
		spurious_interrupt();
}

/*
 * enable 8668 irq
 */
static void enable_adm8668_irq(struct irq_data *d)
{
	intc_write_reg((1 << d->irq), IRQ_ENABLE_REG);
}


static void ack_adm8668_irq(struct irq_data *d)
{
	intc_write_reg((1 << d->irq), IRQ_DISABLE_REG);
}

/*
 * system irq type
 */

static struct irq_chip adm8668_irq_type = {
	.name = "adm8668",
	.irq_ack = ack_adm8668_irq,
	.irq_mask = ack_adm8668_irq,
	.irq_unmask = enable_adm8668_irq
};

/*
 * irq init
 */
static void __init init_adm8668_irqs(void)
{
	int i;

	/* disable all interrupts for the moment */
	intc_write_reg(IRQ_MASK, IRQ_DISABLE_REG);

	for (i = 0; i <= ADM8668_IRQ_MAX; i++)
		irq_set_chip_and_handler(i, &adm8668_irq_type,
			handle_level_irq);

	/* hw0 is where our interrupts are uh.. interrupted at. */
	set_c0_status(IE_IRQ0);
}

/*
 * system init
 */
void __init arch_init_irq(void)
{
	mips_cpu_irq_init();
	init_adm8668_irqs();
}
