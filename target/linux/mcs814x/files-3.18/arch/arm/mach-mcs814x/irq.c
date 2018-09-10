/*
 * Moschip MCS814x generic interrupt controller routines
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under the GPLv2
 */
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/irqdomain.h>

#include <asm/exception.h>
#include <asm/mach/irq.h>
#include <mach/mcs814x.h>

static void __iomem *mcs814x_intc_base;
static struct irq_domain *domain;

static void __init mcs814x_alloc_gc(void __iomem *base, unsigned int irq_start,
					unsigned int num)
{
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;

	if (irq_alloc_domain_generic_chips(domain, num, 1, "mcs814x-intc", handle_level_irq,
                IRQ_GC_INIT_MASK_CACHE, IRQ_NOREQUEST, 0))
		panic("unable to allocate domain generic irq chip");

	gc = irq_get_domain_generic_chip(domain, irq_start);
	if (!gc)
		panic("unable to get generic irq chip");

	gc->reg_base = base;
	ct = gc->chip_types;
	ct->chip.irq_ack = irq_gc_unmask_enable_reg;
	ct->chip.irq_mask = irq_gc_mask_clr_bit;
	ct->chip.irq_unmask = irq_gc_mask_set_bit;
	ct->regs.mask = MCS814X_IRQ_MASK;
	ct->regs.enable = MCS814X_IRQ_ICR;

	/* Clear all interrupts */
	writel_relaxed(0xffffffff, base + MCS814X_IRQ_ICR);
}

asmlinkage void __exception_irq_entry mcs814x_handle_irq(struct pt_regs *regs)
{
	u32 status, irq;

	do {
		/* read the status register */
		status = __raw_readl(mcs814x_intc_base + MCS814X_IRQ_STS0);
		if (!status)
			break;

		irq = ffs(status) - 1;
		status |= (1 << irq);
		/* clear the interrupt */
		__raw_writel(status, mcs814x_intc_base + MCS814X_IRQ_STS0);
		/* call the generic handler */
		handle_domain_irq(domain, irq, regs);

	} while (1);
}

static const struct of_device_id mcs814x_intc_ids[] = {
	{ .compatible = "moschip,mcs814x-intc" },
	{ /* sentinel */ },
};

void __init mcs814x_of_irq_init(void)
{
	struct device_node *np;

	np = of_find_matching_node(NULL, mcs814x_intc_ids);
	if (!np)
		panic("unable to find compatible intc node in dtb\n");

	mcs814x_intc_base = of_iomap(np, 0);
	if (!mcs814x_intc_base)
		panic("unable to map intc cpu registers\n");

	domain = irq_domain_add_linear(np, 32, &irq_generic_chip_ops, NULL);
	if (!domain)
		panic("unable to add irq domain\n");
	irq_set_default_host(domain);

	of_node_put(np);

	mcs814x_alloc_gc(mcs814x_intc_base, 0, 32);
}

