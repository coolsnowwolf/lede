#include <linux/irqdomain.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/irqchip.h>

struct rps_chip_data {
	void __iomem *base;
	struct irq_chip chip;
	struct irq_domain *domain;
} rps_data;

enum {
	RPS_IRQ_BASE = 64,
	RPS_IRQ_COUNT = 32,
	PRS_HWIRQ_BASE = 0,

	RPS_STATUS = 0,
	RPS_RAW_STATUS = 4,
	RPS_UNMASK = 8,
	RPS_MASK = 0xc,
};

/*
 * Routines to acknowledge, disable and enable interrupts
 */
static void rps_mask_irq(struct irq_data *d)
{
	struct rps_chip_data *chip_data = irq_data_get_irq_chip_data(d);
	u32 mask = BIT(d->hwirq);

	iowrite32(mask, chip_data->base + RPS_MASK);
}

static void rps_unmask_irq(struct irq_data *d)
{
	struct rps_chip_data *chip_data = irq_data_get_irq_chip_data(d);
	u32 mask = BIT(d->hwirq);

	iowrite32(mask, chip_data->base + RPS_UNMASK);
}

static struct irq_chip rps_chip = {
	.name			= "RPS",
	.irq_mask		= rps_mask_irq,
	.irq_unmask		= rps_unmask_irq,
};

static int rps_irq_domain_xlate(struct irq_domain *d,
				struct device_node *controller,
				const u32 *intspec, unsigned int intsize,
				unsigned long *out_hwirq,
				unsigned int *out_type)
{
	if (irq_domain_get_of_node(d) != controller)
		return -EINVAL;
	if (intsize < 1)
		return -EINVAL;

	*out_hwirq = intspec[0];
	/* Honestly I do not know the type */
	*out_type = IRQ_TYPE_LEVEL_HIGH;

	return 0;
}

static int rps_irq_domain_map(struct irq_domain *d, unsigned int irq,
				irq_hw_number_t hw)
{
	irq_set_chip_and_handler(irq, &rps_chip, handle_level_irq);
	irq_set_probe(irq);
	irq_set_chip_data(irq, d->host_data);
	return 0;
}

const struct irq_domain_ops rps_irq_domain_ops = {
	.map = rps_irq_domain_map,
	.xlate = rps_irq_domain_xlate,
};

static void rps_handle_cascade_irq(struct irq_desc *desc)
{
	struct rps_chip_data *chip_data = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	unsigned int cascade_irq, rps_irq;
	u32 status;

	chained_irq_enter(chip, desc);

	status = ioread32(chip_data->base + RPS_STATUS);
	rps_irq = __ffs(status);
	cascade_irq = irq_find_mapping(chip_data->domain, rps_irq);

	if (unlikely(rps_irq >= RPS_IRQ_COUNT))
		handle_bad_irq(desc);
	else
		generic_handle_irq(cascade_irq);

	chained_irq_exit(chip, desc);
}

#ifdef CONFIG_OF
int __init rps_of_init(struct device_node *node, struct device_node *parent)
{
	void __iomem *rps_base;
	int irq_start = RPS_IRQ_BASE;
	int irq_base;
	int irq;

	if (WARN_ON(!node))
		return -ENODEV;

	rps_base = of_iomap(node, 0);
	WARN(!rps_base, "unable to map rps registers\n");
	rps_data.base = rps_base;

	irq_base = irq_alloc_descs(irq_start, 0, RPS_IRQ_COUNT, numa_node_id());
	if (IS_ERR_VALUE(irq_base)) {
		WARN(1, "Cannot allocate irq_descs @ IRQ%d, assuming pre-allocated\n",
		     irq_start);
		irq_base = irq_start;
	}

	rps_data.domain = irq_domain_add_legacy(node, RPS_IRQ_COUNT, irq_base,
			PRS_HWIRQ_BASE, &rps_irq_domain_ops, &rps_data);

	if (WARN_ON(!rps_data.domain))
		return -ENOMEM;

	if (parent) {
		irq = irq_of_parse_and_map(node, 0);
		if (irq_set_handler_data(irq, &rps_data) != 0)
			BUG();
		irq_set_chained_handler(irq, rps_handle_cascade_irq);
	}
	return 0;

}

IRQCHIP_DECLARE(nas782x, "plxtech,nas782x-rps", rps_of_init);
#endif
