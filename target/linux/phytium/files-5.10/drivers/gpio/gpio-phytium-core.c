// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/module.h>
#include <linux/bitops.h>

#include "gpio-phytium-core.h"

static int get_pin_location(struct phytium_gpio *gpio, unsigned int offset,
		     struct pin_loc *pl)
{
	int ret;

	if (offset < gpio->ngpio[0]) {
		pl->port = 0;
		pl->offset = offset;
		ret = 0;
	} else if (offset < (gpio->ngpio[0] + gpio->ngpio[1])) {
		pl->port = 1;
		pl->offset = offset - gpio->ngpio[0];
		ret = 0;
	} else {
		ret = -EINVAL;
	}

	return ret;
}

static void phytium_gpio_toggle_trigger(struct phytium_gpio *gpio,
				 unsigned int offset)
{
	struct gpio_chip *gc;
	u32 pol;
	int val;

	/* Only port A can provide interrupt source */
	if (offset >= gpio->ngpio[0])
		return;

	gc = &gpio->gc;

	pol = readl(gpio->regs + GPIO_INT_POLARITY);
	/* Just read the current value right out of the data register */
	val = gc->get(gc, offset);
	if (val)
		pol &= ~BIT(offset);
	else
		pol |= BIT(offset);

	writel(pol, gpio->regs + GPIO_INT_POLARITY);
}

int phytium_gpio_get(struct gpio_chip *gc, unsigned int offset)
{
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct pin_loc loc;
	void __iomem *dat;

	if (get_pin_location(gpio, offset, &loc))
		return -EINVAL;

	dat = gpio->regs + GPIO_EXT_PORTA + (loc.port * GPIO_PORT_STRIDE);

	return !!(readl(dat) & BIT(loc.offset));
}
EXPORT_SYMBOL_GPL(phytium_gpio_get);

void phytium_gpio_set(struct gpio_chip *gc, unsigned int offset, int value)
{
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct pin_loc loc;
	void __iomem *dr;
	unsigned long flags;
	u32 mask;

	if (get_pin_location(gpio, offset, &loc))
		return;
	dr = gpio->regs + GPIO_SWPORTA_DR + (loc.port * GPIO_PORT_STRIDE);

	raw_spin_lock_irqsave(&gpio->lock, flags);

	if (value)
		mask = readl(dr) | BIT(loc.offset);
	else
		mask = readl(dr) & ~BIT(loc.offset);

	writel(mask, dr);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);
}
EXPORT_SYMBOL_GPL(phytium_gpio_set);

int phytium_gpio_direction_input(struct gpio_chip *gc, unsigned int offset)
{
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct pin_loc loc;
	unsigned long flags;
	void __iomem *ddr;

	if (get_pin_location(gpio, offset, &loc))
		return -EINVAL;
	ddr = gpio->regs + GPIO_SWPORTA_DDR + (loc.port * GPIO_PORT_STRIDE);

	raw_spin_lock_irqsave(&gpio->lock, flags);

	writel(readl(ddr) & ~(BIT(loc.offset)), ddr);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_gpio_direction_input);

int phytium_gpio_direction_output(struct gpio_chip *gc, unsigned int offset,
				  int value)
{
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct pin_loc loc;
	unsigned long flags;
	void __iomem *ddr;

	if (get_pin_location(gpio, offset, &loc))
		return -EINVAL;
	ddr = gpio->regs + GPIO_SWPORTA_DDR + (loc.port * GPIO_PORT_STRIDE);

	raw_spin_lock_irqsave(&gpio->lock, flags);

	writel(readl(ddr) | BIT(loc.offset), ddr);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);

	phytium_gpio_set(gc, offset, value);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_gpio_direction_output);

void phytium_gpio_irq_ack(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	u32 val = BIT(irqd_to_hwirq(d));

	raw_spin_lock(&gpio->lock);

	writel(val, gpio->regs + GPIO_PORTA_EOI);

	raw_spin_unlock(&gpio->lock);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_ack);

void phytium_gpio_irq_mask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	u32 val;

	/* Only port A can provide interrupt source */
	if (irqd_to_hwirq(d) >= gpio->ngpio[0])
		return;

	raw_spin_lock(&gpio->lock);

	val = readl(gpio->regs + GPIO_INTMASK);
	val |= BIT(irqd_to_hwirq(d));
	writel(val, gpio->regs + GPIO_INTMASK);

	raw_spin_unlock(&gpio->lock);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_mask);

void phytium_gpio_irq_unmask(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	u32 val;

	/* Only port A can provide interrupt source */
	if (irqd_to_hwirq(d) >= gpio->ngpio[0])
		return;

	raw_spin_lock(&gpio->lock);

	val = readl(gpio->regs + GPIO_INTMASK);
	val &= ~BIT(irqd_to_hwirq(d));
	writel(val, gpio->regs + GPIO_INTMASK);

	raw_spin_unlock(&gpio->lock);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_unmask);

int phytium_gpio_irq_set_type(struct irq_data *d, unsigned int flow_type)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	int hwirq = irqd_to_hwirq(d);
	unsigned long flags, lvl, pol;

	if (hwirq < 0 || hwirq >= gpio->ngpio[0])
		return -EINVAL;

	if ((flow_type & (IRQ_TYPE_LEVEL_HIGH | IRQ_TYPE_LEVEL_LOW)) &&
	    (flow_type & (IRQ_TYPE_EDGE_RISING | IRQ_TYPE_EDGE_FALLING))) {
		dev_err(gc->parent,
			"trying to configure line %d for both level and edge detection, choose one!\n",
			hwirq);
		return -EINVAL;
	}

	raw_spin_lock_irqsave(&gpio->lock, flags);

	lvl = readl(gpio->regs + GPIO_INTTYPE_LEVEL);
	pol = readl(gpio->regs + GPIO_INT_POLARITY);

	switch (flow_type) {
	case IRQ_TYPE_EDGE_BOTH:
		lvl |= BIT(hwirq);
		phytium_gpio_toggle_trigger(gpio, hwirq);
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on both edges\n", hwirq);
		break;
	case IRQ_TYPE_EDGE_RISING:
		lvl |= BIT(hwirq);
		pol |= BIT(hwirq);
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on RISING edge\n", hwirq);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		lvl |= BIT(hwirq);
		pol &= ~BIT(hwirq);
		irq_set_handler_locked(d, handle_edge_irq);
		dev_dbg(gc->parent, "line %d: IRQ on FALLING edge\n", hwirq);
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		lvl &= ~BIT(hwirq);
		pol |= BIT(hwirq);
		irq_set_handler_locked(d, handle_level_irq);
		dev_dbg(gc->parent, "line %d: IRQ on HIGH level\n", hwirq);
		break;
	case IRQ_TYPE_LEVEL_LOW:
		lvl &= ~BIT(hwirq);
		pol &= ~BIT(hwirq);
		irq_set_handler_locked(d, handle_level_irq);
		dev_dbg(gc->parent, "line %d: IRQ on LOW level\n", hwirq);
		break;
	}

	writel(lvl, gpio->regs + GPIO_INTTYPE_LEVEL);
	if (flow_type != IRQ_TYPE_EDGE_BOTH)
		writel(pol, gpio->regs + GPIO_INT_POLARITY);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_set_type);

void phytium_gpio_irq_enable(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	unsigned long flags;
	u32 val;

	/* Only port A can provide interrupt source */
	if (irqd_to_hwirq(d) >= gpio->ngpio[0])
		return;

	raw_spin_lock_irqsave(&gpio->lock, flags);

	val = readl(gpio->regs + GPIO_INTEN);
	val |= BIT(irqd_to_hwirq(d));
	writel(val, gpio->regs + GPIO_INTEN);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_enable);

void phytium_gpio_irq_disable(struct irq_data *d)
{
	struct gpio_chip *gc = irq_data_get_irq_chip_data(d);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	unsigned long flags;
	u32 val;

	/* Only port A can provide interrupt source */
	if (irqd_to_hwirq(d) >= gpio->ngpio[0])
		return;

	raw_spin_lock_irqsave(&gpio->lock, flags);

	val = readl(gpio->regs + GPIO_INTEN);
	val &= ~BIT(irqd_to_hwirq(d));
	writel(val, gpio->regs + GPIO_INTEN);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_disable);

void phytium_gpio_irq_handler(struct irq_desc *desc)
{
	struct gpio_chip *gc = irq_desc_get_handler_data(desc);
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct irq_chip *irqchip = irq_desc_get_chip(desc);
	unsigned long pending;
	int offset;

	chained_irq_enter(irqchip, desc);

	pending = readl(gpio->regs + GPIO_INTSTATUS);
	if (pending) {
		for_each_set_bit(offset, &pending, gpio->ngpio[0]) {
			int gpio_irq = irq_find_mapping(gc->irq.domain,
							offset);
			generic_handle_irq(gpio_irq);

			if ((irq_get_trigger_type(gpio_irq) &
			    IRQ_TYPE_SENSE_MASK) == IRQ_TYPE_EDGE_BOTH)
				phytium_gpio_toggle_trigger(gpio, offset);
		}
	}

	chained_irq_exit(irqchip, desc);
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_handler);

int phytium_gpio_get_direction(struct gpio_chip *gc, unsigned int offset)
{
	struct phytium_gpio *gpio = gpiochip_get_data(gc);
	struct pin_loc loc;
	void __iomem *ddr;

	if (get_pin_location(gpio, offset, &loc))
		return -EINVAL;
	ddr = gpio->regs + GPIO_SWPORTA_DDR + (loc.port * GPIO_PORT_STRIDE);

	return !(readl(ddr) & BIT(loc.offset));
}
EXPORT_SYMBOL_GPL(phytium_gpio_get_direction);

#if CONFIG_SMP
int
phytium_gpio_irq_set_affinity(struct irq_data *d, const struct cpumask *mask_val, bool force)
{
	struct gpio_chip *chip_data = irq_data_get_irq_chip_data(d);
	struct irq_chip *chip = irq_get_chip(*(chip_data->irq.parents));
	struct irq_data *data = irq_get_irq_data(*(chip_data->irq.parents));

	if (chip && chip->irq_set_affinity)
		return chip->irq_set_affinity(data, mask_val, force);

	return -EINVAL;
}
EXPORT_SYMBOL_GPL(phytium_gpio_irq_set_affinity);
#endif

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium GPIO Controller core");
