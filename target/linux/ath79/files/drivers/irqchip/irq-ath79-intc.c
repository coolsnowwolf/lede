// SPDX-License-Identifier: GPL-2.0-only
/*
 *  Atheros AR71xx/AR724x/AR913x specific interrupt handling
 *
 *  Copyright (C) 2018 John Crispin <john@phrozen.org>
 */

#include <linux/interrupt.h>
#include <linux/irqchip.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/irqdomain.h>

#include <asm/irq_cpu.h>
#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#define ATH79_MAX_INTC_CASCADE	3

struct ath79_intc {
	int irq;
	u32 num_irqs;
	u32 enable_mask;
	u32 int_status;
	u32 irq_mask[ATH79_MAX_INTC_CASCADE];
	u32 irq_wb_chan[ATH79_MAX_INTC_CASCADE];
	struct irq_chip chip;
};

static void ath79_intc_irq_handler(struct irq_desc *desc)
{
	struct irq_domain *domain = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	struct ath79_intc *intc = domain->host_data;
	u32 pending;
	int i;

	chained_irq_enter(chip, desc);

	pending = ath79_reset_rr(intc->int_status);
	pending &= intc->enable_mask;

	if (!pending) {
		spurious_interrupt();
		chained_irq_exit(chip, desc);
		return;
	}

	for (i = 0; i < intc->num_irqs; i++)
		if (pending & intc->irq_mask[i]) {
			if (intc->irq_wb_chan[i] != 0xffffffff)
				ath79_ddr_wb_flush(intc->irq_wb_chan[i]);
			generic_handle_domain_irq(domain, i);
		}

	chained_irq_exit(chip, desc);
}

static void ath79_intc_irq_enable(struct irq_data *d)
{
	struct ath79_intc *intc = d->domain->host_data;
	enable_irq(intc->irq);
}

static void ath79_intc_irq_disable(struct irq_data *d)
{
	struct ath79_intc *intc = d->domain->host_data;
	disable_irq(intc->irq);
}

static int ath79_intc_map(struct irq_domain *d, unsigned int irq, irq_hw_number_t hw)
{
	struct ath79_intc *intc = d->host_data;

	irq_set_chip_and_handler(irq, &intc->chip, handle_level_irq);

	return 0;
}

static const struct irq_domain_ops ath79_irq_domain_ops = {
	.xlate = irq_domain_xlate_onecell,
	.map = ath79_intc_map,
};

static int __init ath79_intc_of_init(
	struct device_node *node, struct device_node *parent)
{
	struct irq_domain *domain;
	struct ath79_intc *intc;
	int cnt, cntwb, i, err;

	cnt = of_property_count_u32_elems(node, "qca,pending-bits");
	if (cnt < 0 || cnt > ATH79_MAX_INTC_CASCADE) {
		pr_err("Invalid number of INTC pending bits (%d)\n", cnt);
		return -EINVAL;
	}

	intc = kzalloc(sizeof(*intc), GFP_KERNEL);
	if (!intc)
		return -ENOMEM;

	intc->chip = dummy_irq_chip;
	intc->chip.name = "INTC";
	intc->chip.irq_disable = ath79_intc_irq_disable;
	intc->chip.irq_enable = ath79_intc_irq_enable;
	intc->num_irqs = cnt;

	if (of_property_read_u32(node, "qca,int-status-addr", &intc->int_status) < 0) {
		pr_err("Missing address of interrupt status register\n");
		err = -EINVAL;
		goto err;
	}

	of_property_read_u32_array(node, "qca,pending-bits", intc->irq_mask, cnt);
	for (i = 0; i < cnt; i++) {
		intc->enable_mask |= intc->irq_mask[i];
		intc->irq_wb_chan[i] = 0xffffffff;
	}

	cntwb = of_count_phandle_with_args(
		node, "qca,ddr-wb-channels", "#qca,ddr-wb-channel-cells");
	cntwb = max(0, cntwb);

	for (i = 0; i < cntwb; i++) {
		struct of_phandle_args args;
		u32 irq = i;

		of_property_read_u32_index(
			node, "qca,ddr-wb-channel-interrupts", i, &irq);
		if (irq >= intc->num_irqs)
			continue;

		err = of_parse_phandle_with_args(
			node, "qca,ddr-wb-channels",
			"#qca,ddr-wb-channel-cells",
			i, &args);
		if (err)
			goto err;

		intc->irq_wb_chan[irq] = args.args[0];
	}

	intc->irq = of_irq_get(node, 0);
	if (intc->irq < 0) {
		pr_err("Failed to get INTC IRQ\n");
		err = intc->irq;
		goto err;
	}

	if (!intc->irq) {
		pr_err("Failed to get INTC IRQ\n");
		err = -ENODEV;
		goto err;
	}

	domain = irq_domain_create_linear(of_fwnode_handle(node), cnt, &ath79_irq_domain_ops, intc);
	if (!domain) {
		err = -EINVAL;
		goto err;
	}

	irq_set_chained_handler_and_data(intc->irq, ath79_intc_irq_handler, domain);

	return 0;

err:
	kfree(intc);
	return err;
}
IRQCHIP_DECLARE(ath79_intc, "qca,ar9340-intc",
		ath79_intc_of_init);
