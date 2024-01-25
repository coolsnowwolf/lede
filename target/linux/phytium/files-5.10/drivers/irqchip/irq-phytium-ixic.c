// SPDX-License-Identifier: GPL-2.0
/*
 * Driver for Phytium PCIe legacy INTx interrupt controller
 *
 * Copyright (c) 2020-2023, Phytium Technology Co., Ltd.
 */

#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/irqchip.h>
#include <linux/irqdomain.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>

#include <dt-bindings/interrupt-controller/arm-gic.h>

#define NUM_IRQS		4

#define CTR_BANK_NUM		6
#define CTR_BANK_SIZE		0x10000
#define CTR_BANK_ISTATUS_LOCAL	0x184

#define HPB_INTX_STATUS_0	0x0
#define HPB_INTX_STATUS_1	0x1000

struct ixic_irq_data {
	void __iomem *ctr;
	void __iomem *hpb;
	u32 spi_base;
};

static void phytium_ixic_irq_eoi(struct irq_data *d)
{
	struct ixic_irq_data *data = irq_data_get_irq_chip_data(d);
	unsigned int intx = irqd_to_hwirq(d);
	u32 gstatus = readl(data->hpb) | (readl(data->hpb + HPB_INTX_STATUS_1) << 12);
	u32 imask, istatus;
	int i;

	WARN_ON(intx >= NUM_IRQS);
	imask = 1 << (3 - intx);
	istatus = (1 << intx) << 24;
	for (i = 0; i < CTR_BANK_NUM; i++, gstatus >>= 4) {
		if (gstatus & imask)
			writel(istatus, data->ctr + CTR_BANK_SIZE*i + CTR_BANK_ISTATUS_LOCAL);
	}

	irq_chip_eoi_parent(d);
}

static struct irq_chip phytium_ixic_irq_chip = {
	.name = "IXIU",
	.irq_eoi = phytium_ixic_irq_eoi,
	.irq_mask = irq_chip_mask_parent,
	.irq_unmask = irq_chip_unmask_parent,
	.irq_set_type = irq_chip_set_type_parent,
	.irq_set_affinity = irq_chip_set_affinity_parent,
	.flags = IRQCHIP_MASK_ON_SUSPEND,
};

static int phytium_ixic_translate(struct irq_domain *domain,
				  struct irq_fwspec *fwspec,
				  unsigned long *hwirq,
				  unsigned int *type)
{
	struct ixic_irq_data *info = domain->host_data;

	if (is_of_node(fwspec->fwnode)) {
		if (fwspec->param_count != 3)
			return -EINVAL;

		if (fwspec->param[0] != GIC_SPI)
			return -EINVAL; /* No PPI should point to this domain */

		*hwirq = fwspec->param[1] - info->spi_base;
		*type = fwspec->param[2] & IRQ_TYPE_SENSE_MASK;
	} else {
		if (fwspec->param_count != 2)
			return -EINVAL;
		*hwirq = fwspec->param[0] - info->spi_base;
		*type = fwspec->param[1] & IRQ_TYPE_SENSE_MASK;
	}

	return 0;
}

static int phytium_ixic_alloc(struct irq_domain *dom, unsigned int virq,
			      unsigned int nr_irqs, void *data)
{
	struct irq_fwspec *fwspec = data;
	struct irq_fwspec parent_fwspec;
	struct ixic_irq_data *info = dom->host_data;
	irq_hw_number_t hwirq;

	/* We assume the device use the parent's format directly */
	parent_fwspec = *fwspec;
	if (is_of_node(dom->parent->fwnode)) {
		if (fwspec->param_count != 3)
			return -EINVAL; /* Not GIC compliant */
		if (fwspec->param[0] != GIC_SPI)
			return -EINVAL; /* No PPI should point to this domain */

		/* Get the local hwirq of IXIC */
		hwirq = fwspec->param[1] - info->spi_base;
	} else {
		hwirq = fwspec->param[0] - info->spi_base;
	}
	WARN_ON(nr_irqs != 1);
	irq_domain_set_hwirq_and_chip(dom, virq, hwirq, &phytium_ixic_irq_chip, info);

	parent_fwspec.fwnode = dom->parent->fwnode;
	return irq_domain_alloc_irqs_parent(dom, virq, nr_irqs, &parent_fwspec);
}

static const struct irq_domain_ops ixic_domain_ops = {
	.translate = phytium_ixic_translate,
	.alloc	= phytium_ixic_alloc,
	.free	= irq_domain_free_irqs_common,
};

static struct ixic_irq_data *phytium_ixic_init(const struct fwnode_handle *fwnode,
					      struct resource *ctr, struct resource *hpb)
{
	struct ixic_irq_data *data;
	int err;

	data = kzalloc(sizeof(*data), GFP_KERNEL);
	if (!data)
		return ERR_PTR(-ENOMEM);

	if (fwnode_property_read_u32_array(fwnode, "intx-spi-base",
					   &data->spi_base, 1)) {
		err = -ENODEV;
		goto out_free;
	}

	data->ctr = ioremap(ctr->start, resource_size(ctr));
	if (!data->ctr) {
		err = -ENODEV;
		goto out_free;
	}

	data->hpb = ioremap(hpb->start, resource_size(hpb));
	if (!data->hpb) {
		err = -ENODEV;
		goto out_free;
	}

	return data;

out_free:
	kfree(data);
	return ERR_PTR(err);
}

static int __init phytium_ixic_dt_init(struct device_node *node,
				   struct device_node *parent)
{
	struct irq_domain *pd, *d;
	struct ixic_irq_data *data;
	struct resource ctr, hpb;

	if (!parent) {
		pr_err("%pOF: no parent, giving up\n", node);
		return -ENODEV;
	}

	pd = irq_find_host(parent);
	if (!pd) {
		pr_err("%pOF: unable to obtain parent domain\n", node);
		return -ENXIO;
	}

	if (of_address_to_resource(node, 0, &ctr)) {
		pr_err("%pOF: failed to parse 'ctr' memory resource\n", node);
		return -ENXIO;
	}

	if (of_address_to_resource(node, 1, &hpb)) {
		pr_err("%pOF: failed to parse 'hpb' memory resource\n", node);
		return -ENXIO;
	}

	data = phytium_ixic_init(of_node_to_fwnode(node), &ctr, &hpb);
	if (IS_ERR(data))
		return PTR_ERR(data);

	d = irq_domain_add_hierarchy(pd, 0, NUM_IRQS, node, &ixic_domain_ops, data);
	if (!d) {
		pr_err("%pOF: failed to allocate domain\n", node);
		goto out_unmap;
	}

	pr_info("%pOF: %d interrupts forwarded to %pOF\n", node, NUM_IRQS, parent);

	return 0;

out_unmap:
	iounmap(data->ctr);
	iounmap(data->hpb);
	kfree(data);
	return -ENOMEM;
}
IRQCHIP_DECLARE(ixic, "phytium,ixic", phytium_ixic_dt_init);

#ifdef CONFIG_ACPI
static int phytium_ixic_acpi_probe(struct platform_device *pdev)
{
	struct irq_domain *domain;
	struct ixic_irq_data *data;
	struct resource *ctr, *hpb;

	ctr = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!ctr) {
		dev_err(&pdev->dev, "failed to parse 'ctr' memory resource\n");
		return -ENXIO;
	}

	hpb = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (!hpb) {
		dev_err(&pdev->dev, "failed to parse 'hpb' memory resource\n");
		return -ENXIO;
	}

	data = phytium_ixic_init(dev_fwnode(&pdev->dev), ctr, hpb);
	if (IS_ERR(data))
		return PTR_ERR(data);

	domain = acpi_irq_create_hierarchy(0, NUM_IRQS, dev_fwnode(&pdev->dev),
					   &ixic_domain_ops, data);
	if (!domain) {
		dev_err(&pdev->dev, "failed to create IRQ domain\n");
		goto out_unmap;
	}

	dev_info(&pdev->dev, "%d interrupts forwarded\n", NUM_IRQS);

	return 0;

out_unmap:
	iounmap(data->ctr);
	iounmap(data->hpb);
	kfree(data);
	return -ENOMEM;
}

static const struct acpi_device_id phytium_ixic_acpi_ids[] = {
	{ "PHYT0013" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(acpi, phytium_ixic_acpi_ids);

static struct platform_driver phytium_ixic_driver = {
	.driver = {
		.name = "phytium-ixic",
		.acpi_match_table = phytium_ixic_acpi_ids,
	},
	.probe = phytium_ixic_acpi_probe,
};
builtin_platform_driver(phytium_ixic_driver);
#endif
