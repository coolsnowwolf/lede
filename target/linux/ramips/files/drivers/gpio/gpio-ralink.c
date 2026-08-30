/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * Copyright (C) 2009-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2013 John Crispin <blogic@openwrt.org>
 */

#include <linux/err.h>
#include <linux/gpio/driver.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>

#define MAX_NGPIOS 95

enum ralink_gpio_reg {
	GPIO_REG_INT = 0,
	GPIO_REG_EDGE,
	GPIO_REG_RENA,
	GPIO_REG_FENA,
	GPIO_REG_DATA,
	GPIO_REG_DIR,
	GPIO_REG_POL,
	GPIO_REG_SET,
	GPIO_REG_RESET,
	GPIO_REG_TOGGLE,
	GPIO_REG_MAX
};

struct ralink_gpio_chip {
	struct gpio_chip chip;
	u8 regs[GPIO_REG_MAX];

	spinlock_t lock;
	void __iomem *membase;
	int gpio_irq;

	u32 rising;
	u32 falling;
};

static inline void rt_gpio_w32(struct ralink_gpio_chip *rg, u8 reg, u32 val)
{
	iowrite32(val, rg->membase + rg->regs[reg]);
}

static inline u32 rt_gpio_r32(struct ralink_gpio_chip *rg, u8 reg)
{
	return ioread32(rg->membase + rg->regs[reg]);
}

static irqreturn_t ralink_gpio_irq_handler(int irq, void *data)
{
	struct gpio_chip *gc = data;
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	irqreturn_t ret = IRQ_NONE;
	unsigned long pending;
	int bit;

	pending = rt_gpio_r32(rg, GPIO_REG_INT);
	for_each_set_bit(bit, &pending, rg->chip.ngpio) {
		generic_handle_domain_irq(gc->irq.domain, bit);
		rt_gpio_w32(rg, GPIO_REG_INT, BIT(bit));
		ret |= IRQ_HANDLED;
	}

	return ret;
}

static void ralink_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	unsigned long flags;
	u32 rise, fall;

	rise = rt_gpio_r32(rg, GPIO_REG_RENA);
	fall = rt_gpio_r32(rg, GPIO_REG_FENA);

	spin_lock_irqsave(&rg->lock, flags);
	rt_gpio_w32(rg, GPIO_REG_RENA, rise | (BIT(d->hwirq) & rg->rising));
	rt_gpio_w32(rg, GPIO_REG_FENA, fall | (BIT(d->hwirq) & rg->falling));
	spin_unlock_irqrestore(&rg->lock, flags);
}

static void ralink_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	unsigned long flags;
	u32 rise, fall;

	rise = rt_gpio_r32(rg, GPIO_REG_RENA);
	fall = rt_gpio_r32(rg, GPIO_REG_FENA);

	spin_lock_irqsave(&rg->lock, flags);
	rt_gpio_w32(rg, GPIO_REG_FENA, fall & ~BIT(d->hwirq));
	rt_gpio_w32(rg, GPIO_REG_RENA, rise & ~BIT(d->hwirq));
	spin_unlock_irqrestore(&rg->lock, flags);
}

static int ralink_gpio_irq_type(struct irq_data *d, unsigned int type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	u32 mask = BIT(d->hwirq);

	if (type == IRQ_TYPE_PROBE) {
		if ((rg->rising | rg->falling) & mask)
			return 0;

		type = IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING;
	}

	if (type & IRQ_TYPE_EDGE_RISING)
		rg->rising |= mask;
	else
		rg->rising &= ~mask;

	if (type & IRQ_TYPE_EDGE_FALLING)
		rg->falling |= mask;
	else
		rg->falling &= ~mask;

	return 0;
}

static struct irq_chip ralink_gpio_irq_chip = {
	.name		= "gpio-ralink",
	.irq_unmask	= ralink_gpio_irq_unmask,
	.irq_mask	= ralink_gpio_irq_mask,
	.irq_mask_ack	= ralink_gpio_irq_mask,
	.irq_set_type	= ralink_gpio_irq_type,
	.flags		= IRQCHIP_IMMUTABLE,
	GPIOCHIP_IRQ_RESOURCE_HELPERS,
};

static int ralink_get_direction(struct gpio_chip *gc, unsigned offset)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);

	return !(rt_gpio_r32(rg, GPIO_REG_DIR) & BIT(offset));
}

static int ralink_direction_input(struct gpio_chip *gc, unsigned offset)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	u32 val = rt_gpio_r32(rg, GPIO_REG_DIR);
	rt_gpio_w32(rg, GPIO_REG_DIR, val & ~BIT(offset));

	return 0;
}

static int ralink_direction_output(struct gpio_chip *gc, unsigned offset, int val)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	rt_gpio_w32(rg, val ? GPIO_REG_SET : GPIO_REG_RESET, BIT(offset));
	rt_gpio_w32(rg, GPIO_REG_DIR, rt_gpio_r32(rg, GPIO_REG_DIR) | BIT(offset));

	return 0;
}

static int ralink_get(struct gpio_chip *gc, unsigned offset)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);

	return !!(rt_gpio_r32(rg, GPIO_REG_DATA) & BIT(offset));
}

static int ralink_set(struct gpio_chip *gc, unsigned offset, int val)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	rt_gpio_w32(rg, val ? GPIO_REG_SET : GPIO_REG_RESET, BIT(offset));

	return 0;
}

static int ralink_set_multiple(struct gpio_chip *gc, unsigned long *mask,
				     unsigned long *bits)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	unsigned long set = *mask & *bits;
	unsigned long reset = *mask & ~*bits;

	if (set)
		rt_gpio_w32(rg, GPIO_REG_SET, set);
	if (reset)
		rt_gpio_w32(rg, GPIO_REG_RESET, reset);

	return 0;
}

static int ralink_get_multiple(struct gpio_chip *gc, unsigned long *mask,
				     unsigned long *bits)
{
	struct ralink_gpio_chip *rg = gpiochip_get_data(gc);
	*bits = rt_gpio_r32(rg, GPIO_REG_DATA) & *mask;

	return 0;
}

static int ralink_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct fwnode_handle *fw = dev_fwnode(dev);
	struct ralink_gpio_chip *rg;
	struct fwnode_reference_args args;
	u32 ngpios = 0;
	int ret;

	rg = devm_kzalloc(dev, sizeof(struct ralink_gpio_chip), GFP_KERNEL);
	if (!rg)
		return -ENOMEM;

	rg->membase = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(rg->membase))
		return PTR_ERR(rg->membase);

	if (device_property_read_u8_array(dev, "ralink,register-map",
			rg->regs, GPIO_REG_MAX)) {
		dev_err(dev, "failed to read register definition\n");
		return -EINVAL;
	}

	if (device_property_read_u32(dev, "ngpios", &ngpios) < 0)
		ngpios = 0;

	if (ngpios == 0) {
		if (!fwnode_property_get_reference_args(fw, "gpio-ranges", "#gpio-cells", 0, 0, &args)) {
			if (args.nargs > 0)
				ngpios = args.args[args.nargs - 1];
			fwnode_handle_put(args.fwnode);
		}
	}

	if (ngpios == 0 || ngpios > MAX_NGPIOS)
		ngpios = MAX_NGPIOS;

	spin_lock_init(&rg->lock);

	rg->chip.label           = dev_name(dev);
	rg->chip.parent          = dev;
	rg->chip.owner           = THIS_MODULE;
	rg->chip.base            = -1;
	rg->chip.ngpio           = ngpios;
	rg->chip.get_direction   = ralink_get_direction;
	rg->chip.direction_input = ralink_direction_input;
	rg->chip.direction_output= ralink_direction_output;
	rg->chip.get             = ralink_get;
	rg->chip.set             = ralink_set;
	rg->chip.get_multiple    = ralink_get_multiple;
	rg->chip.set_multiple    = ralink_set_multiple;

	/* set polarity to low for all lines */
	rt_gpio_w32(rg, GPIO_REG_POL, 0);

	rg->gpio_irq = platform_get_irq(pdev, 0);
	if (rg->gpio_irq < 0)
		return rg->gpio_irq;

	if (rg->gpio_irq) {
		struct gpio_irq_chip *girq;

		/*
		 * Directly request the irq here instead of passing
		 * a flow-handler because the irq is shared.
		 */
		ret = devm_request_irq(dev, rg->gpio_irq,
				       ralink_gpio_irq_handler, IRQF_SHARED,
				       NULL, &rg->chip);
		if (ret) {
			dev_err(dev, "Error requesting IRQ %d: %d\n",
				rg->gpio_irq, ret);
			return ret;
		}

		girq = &rg->chip.irq;
		gpio_irq_chip_set_chip(girq, &ralink_gpio_irq_chip);
		/* This will let us handle the parent IRQ in the driver */
		girq->parent_handler = NULL;
		girq->num_parents = 0;
		girq->parents = NULL;
		girq->default_type = IRQ_TYPE_NONE;
		girq->handler = handle_simple_irq;

		rt_gpio_w32(rg, GPIO_REG_RENA, 0);
		rt_gpio_w32(rg, GPIO_REG_FENA, 0);
	}

	return devm_gpiochip_add_data(dev, &rg->chip, rg);
}

static const struct of_device_id ralink_gpio_match[] = {
	{ .compatible = "ralink,rt2880-gpio" },
	{},
};
MODULE_DEVICE_TABLE(of, ralink_gpio_match);

static struct platform_driver ralink_gpio_driver = {
	.probe = ralink_gpio_probe,
	.driver = {
		.name = "ralink_gpio",
		.of_match_table = ralink_gpio_match,
	},
};

module_platform_driver(ralink_gpio_driver);
