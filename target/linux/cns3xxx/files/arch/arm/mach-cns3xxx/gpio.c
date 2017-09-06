/*
 * Copyright 2012 Gateworks Corporation
 *     Chris Lang <clang@gateworks.com>
 *     Tim Harvey <tharvey@gateworks.com>
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, Version 2, as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irqchip/chained_irq.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/irq.h>
#include <linux/irqdomain.h>

#include <asm/mach/irq.h>

/*
 * Registers
 */
#define GPIO_INPUT                          0x04
#define GPIO_DIR                            0x08
#define GPIO_SET                            0x10
#define GPIO_CLEAR                          0x14
#define GPIO_INTERRUPT_ENABLE               0x20
#define GPIO_INTERRUPT_RAW_STATUS           0x24
#define GPIO_INTERRUPT_MASKED_STATUS        0x28
#define GPIO_INTERRUPT_MASK                 0x2C
#define GPIO_INTERRUPT_CLEAR                0x30
#define GPIO_INTERRUPT_TRIGGER_METHOD       0x34
#define GPIO_INTERRUPT_TRIGGER_BOTH_EDGES   0x38
#define GPIO_INTERRUPT_TRIGGER_TYPE         0x3C

#define GPIO_INTERRUPT_TRIGGER_METHOD_EDGE  0
#define GPIO_INTERRUPT_TRIGGER_METHOD_LEVEL 1
#define GPIO_INTERRUPT_TRIGGER_EDGE_SINGLE  0
#define GPIO_INTERRUPT_TRIGGER_EDGE_BOTH    1
#define GPIO_INTERRUPT_TRIGGER_TYPE_RISING  0
#define GPIO_INTERRUPT_TRIGGER_TYPE_FALLING 1
#define GPIO_INTERRUPT_TRIGGER_TYPE_HIGH    0
#define GPIO_INTERRUPT_TRIGGER_TYPE_LOW     1

struct cns3xxx_gpio_chip {
	struct gpio_chip    chip;
	struct irq_domain   *domain;
	spinlock_t          lock;
	void __iomem        *base;
};

static struct cns3xxx_gpio_chip cns3xxx_gpio_chips[2];
static int cns3xxx_gpio_chip_count;

static inline void
__set_direction(struct cns3xxx_gpio_chip *cchip, unsigned pin, int input)
{
	u32 reg;

	reg = __raw_readl(cchip->base + GPIO_DIR);
	if (input)
		reg &= ~(1 << pin);
	else
		reg |= (1 << pin);
	__raw_writel(reg, cchip->base + GPIO_DIR);
}

/*
 * GENERIC_GPIO primatives
 */
static int cns3xxx_gpio_direction_input(struct gpio_chip *chip, unsigned pin)
{
	struct cns3xxx_gpio_chip *cchip =
		container_of(chip, struct cns3xxx_gpio_chip, chip);
	unsigned long flags;

	spin_lock_irqsave(&cchip->lock, flags);
	__set_direction(cchip, pin, 1);
	spin_unlock_irqrestore(&cchip->lock, flags);

	return 0;
}

static int cns3xxx_gpio_get(struct gpio_chip *chip, unsigned pin)
{
	struct cns3xxx_gpio_chip *cchip =
		container_of(chip, struct cns3xxx_gpio_chip, chip);
	int val;

	val = ((__raw_readl(cchip->base + GPIO_INPUT) >> pin) & 0x1);

	return val;
}

static int cns3xxx_gpio_direction_output(struct gpio_chip *chip, unsigned pin, int level)
{
	struct cns3xxx_gpio_chip *cchip =
		container_of(chip, struct cns3xxx_gpio_chip, chip);
	unsigned long flags;

	spin_lock_irqsave(&cchip->lock, flags);
	if (level)
		__raw_writel(1 << pin, cchip->base + GPIO_SET);
	else
		__raw_writel(1 << pin, cchip->base + GPIO_CLEAR);
	__set_direction(cchip, pin, 0);
	spin_unlock_irqrestore(&cchip->lock, flags);

	return 0;
}

static void cns3xxx_gpio_set(struct gpio_chip *chip, unsigned pin,
	int level)
{
	struct cns3xxx_gpio_chip *cchip =
		container_of(chip, struct cns3xxx_gpio_chip, chip);

	if (level)
		__raw_writel(1 << pin, cchip->base + GPIO_SET);
	else
		__raw_writel(1 << pin, cchip->base + GPIO_CLEAR);
}

static int cns3xxx_gpio_to_irq(struct gpio_chip *chip, unsigned pin)
{
	struct cns3xxx_gpio_chip *cchip =
		container_of(chip, struct cns3xxx_gpio_chip, chip);

	return irq_find_mapping(cchip->domain, pin);
}


/*
 * IRQ support
 */

/* one interrupt per GPIO controller (GPIOA/GPIOB)
 * this is called in task context, with IRQs enabled
 */
static void cns3xxx_gpio_irq_handler(struct irq_desc *desc)
{
	struct cns3xxx_gpio_chip *cchip = irq_desc_get_handler_data(desc);
	struct irq_chip *chip = irq_desc_get_chip(desc);
	u16 i;
	u32 reg;

	chained_irq_enter(chip, desc); /* mask and ack the base interrupt */

	/* see which pin(s) triggered the interrupt */
	reg = __raw_readl(cchip->base + GPIO_INTERRUPT_RAW_STATUS);
	for (i = 0; i < 32; i++) {
		if (reg & (1 << i)) {
			/* let the generic IRQ layer handle an interrupt */
			generic_handle_irq(irq_find_mapping(cchip->domain, i));
		}
	}

	chained_irq_exit(chip, desc); /* unmask the base interrupt */
}

static int cns3xxx_gpio_irq_set_type(struct irq_data *d, u32 irqtype)
{
	struct irq_chip_generic *gc = irq_data_get_irq_chip_data(d);
	struct cns3xxx_gpio_chip *cchip = gc->private;
	u32 gpio = d->hwirq;
	unsigned long flags;
	u32 method, edges, type;

	spin_lock_irqsave(&cchip->lock, flags);
	method = __raw_readl(cchip->base + GPIO_INTERRUPT_TRIGGER_METHOD);
	edges  = __raw_readl(cchip->base + GPIO_INTERRUPT_TRIGGER_BOTH_EDGES);
	type   = __raw_readl(cchip->base + GPIO_INTERRUPT_TRIGGER_TYPE);
	method &= ~(1 << gpio);
	edges  &= ~(1 << gpio);
	type   &= ~(1 << gpio);

	switch(irqtype) {
	case IRQ_TYPE_EDGE_RISING:
		method |= (GPIO_INTERRUPT_TRIGGER_METHOD_EDGE << gpio);
		edges  |= (GPIO_INTERRUPT_TRIGGER_EDGE_SINGLE << gpio);
		type   |= (GPIO_INTERRUPT_TRIGGER_TYPE_RISING << gpio);
		break;
	case IRQ_TYPE_EDGE_FALLING:
		method |= (GPIO_INTERRUPT_TRIGGER_METHOD_EDGE << gpio);
		edges  |= (GPIO_INTERRUPT_TRIGGER_EDGE_SINGLE << gpio);
		type   |= (GPIO_INTERRUPT_TRIGGER_TYPE_FALLING << gpio);
		break;
	case IRQ_TYPE_EDGE_BOTH:
		method |= (GPIO_INTERRUPT_TRIGGER_METHOD_EDGE << gpio);
		edges  |= (GPIO_INTERRUPT_TRIGGER_EDGE_BOTH << gpio);
		break;
	case IRQ_TYPE_LEVEL_LOW:
		method |= (GPIO_INTERRUPT_TRIGGER_METHOD_LEVEL << gpio);
		type   |= (GPIO_INTERRUPT_TRIGGER_TYPE_LOW << gpio);
		break;
	case IRQ_TYPE_LEVEL_HIGH:
		method |= (GPIO_INTERRUPT_TRIGGER_METHOD_LEVEL << gpio);
		type   |= (GPIO_INTERRUPT_TRIGGER_TYPE_HIGH << gpio);
		break;
	default:
		printk(KERN_WARNING "No irq type\n");
		spin_unlock_irqrestore(&cchip->lock, flags);
		return -EINVAL;
	}

	__raw_writel(method, cchip->base + GPIO_INTERRUPT_TRIGGER_METHOD);
	__raw_writel(edges,  cchip->base + GPIO_INTERRUPT_TRIGGER_BOTH_EDGES);
	__raw_writel(type,   cchip->base + GPIO_INTERRUPT_TRIGGER_TYPE);
	spin_unlock_irqrestore(&cchip->lock, flags);

	if (type & (IRQ_TYPE_LEVEL_LOW | IRQ_TYPE_LEVEL_HIGH))
		irq_set_handler_locked(d, handle_level_irq);
	else if (type & (IRQ_TYPE_EDGE_FALLING | IRQ_TYPE_EDGE_RISING))
		irq_set_handler_locked(d, handle_edge_irq);

	return 0;
}

void __init cns3xxx_gpio_init(int gpio_base, int ngpio,
	u32 base, int irq, int secondary_irq_base)
{
	struct cns3xxx_gpio_chip *cchip;
	struct irq_chip_generic *gc;
	struct irq_chip_type *ct;
	char gc_label[16];
	int irq_base;

	if (cns3xxx_gpio_chip_count == ARRAY_SIZE(cns3xxx_gpio_chips))
		return;

	snprintf(gc_label, sizeof(gc_label), "cns3xxx_gpio%d",
		cns3xxx_gpio_chip_count);

	cchip = cns3xxx_gpio_chips + cns3xxx_gpio_chip_count;
	cchip->chip.label = kstrdup(gc_label, GFP_KERNEL);
	cchip->chip.direction_input = cns3xxx_gpio_direction_input;
	cchip->chip.get = cns3xxx_gpio_get;
	cchip->chip.direction_output = cns3xxx_gpio_direction_output;
	cchip->chip.set = cns3xxx_gpio_set;
	cchip->chip.to_irq = cns3xxx_gpio_to_irq;
	cchip->chip.base = gpio_base;
	cchip->chip.ngpio = ngpio;
	cchip->chip.can_sleep = 0;
	spin_lock_init(&cchip->lock);
	cchip->base = (void __iomem *)base;

	BUG_ON(gpiochip_add(&cchip->chip) < 0);
	cns3xxx_gpio_chip_count++;

	/* clear GPIO interrupts */
	__raw_writel(0xffff, cchip->base + GPIO_INTERRUPT_CLEAR);

	irq_base = irq_alloc_descs(-1, secondary_irq_base, ngpio,
		numa_node_id());
	if (irq_base < 0)
		goto out_irqdesc_free;

	cchip->domain = irq_domain_add_legacy(NULL, ngpio, irq_base, 0,
		&irq_domain_simple_ops, NULL);
	if (!cchip->domain)
		goto out_irqdesc_free;

	/*
	 * IRQ chip init
	 */
	gc = irq_alloc_generic_chip("cns3xxx_gpio_irq", 1, irq_base,
		cchip->base, handle_edge_irq);

	gc->private = cchip;

	ct = gc->chip_types;
	ct->type = IRQ_TYPE_EDGE_FALLING;
	ct->regs.ack = GPIO_INTERRUPT_CLEAR;
	ct->regs.enable = GPIO_INTERRUPT_ENABLE;
	ct->chip.irq_ack = irq_gc_ack_set_bit;
	ct->chip.irq_enable = irq_gc_unmask_enable_reg;
	ct->chip.irq_disable = irq_gc_mask_disable_reg;
	ct->chip.irq_set_type = cns3xxx_gpio_irq_set_type;
	ct->handler = handle_edge_irq;

	irq_setup_generic_chip(gc, IRQ_MSK(ngpio), IRQ_GC_INIT_MASK_CACHE,
		IRQ_NOREQUEST, 0);
	irq_set_chained_handler(irq, cns3xxx_gpio_irq_handler);
	irq_set_handler_data(irq, cchip);

	return;

out_irqdesc_free:
	irq_free_descs(irq_base, ngpio);
}
