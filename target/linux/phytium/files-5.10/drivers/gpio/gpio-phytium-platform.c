// SPDX-License-Identifier: GPL-2.0
/*
 * Support functions for Phytium GPIO
 *
 * Copyright (c) 2019-2023 Phytium Technology Co., Ltd.
 *
 * Derived from drivers/gpio/gpio-pl061.c
 *   Copyright (C) 2008, 2009 Provigent Ltd.
 */

#include <linux/acpi.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>

#include "gpio-phytium-core.h"

static const struct of_device_id phytium_gpio_of_match[] = {
	{ .compatible = "phytium,gpio", },
	{ }
};
MODULE_DEVICE_TABLE(of, phytium_gpio_of_match);

static const struct acpi_device_id phytium_gpio_acpi_match[] = {
	{ "PHYT0001", 0 },
	{ }
};
MODULE_DEVICE_TABLE(acpi, phytium_gpio_acpi_match);

static int phytium_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct phytium_gpio *gpio;
	struct gpio_irq_chip *girq;
	struct fwnode_handle *fwnode;
	int err, irq_count;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	gpio->regs = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(gpio->regs))
		return PTR_ERR(gpio->regs);

	if (!device_get_child_node_count(dev))
		return -ENODEV;

	device_for_each_child_node(dev, fwnode) {
		int idx;

		if (fwnode_property_read_u32(fwnode, "reg", &idx) ||
		    idx >= MAX_NPORTS) {
			dev_err(dev, "missing/invalid port index\n");
			fwnode_handle_put(fwnode);
			return -EINVAL;
		}

		if (fwnode_property_read_u32(fwnode, "ngpios", &gpio->ngpio[idx]) &&
			fwnode_property_read_u32(fwnode, "nr-gpios", &gpio->ngpio[idx])) {
			dev_info(dev,
				 "failed to get number of gpios for Port%c\n",
				 idx ? 'B' : 'A');
			gpio->ngpio[idx] = NGPIO_DEFAULT;
		}
	}

	/* irq_chip support */
	gpio->irq_chip.name = dev_name(dev);
	gpio->irq_chip.irq_ack = phytium_gpio_irq_ack;
	gpio->irq_chip.irq_mask = phytium_gpio_irq_mask;
	gpio->irq_chip.irq_unmask = phytium_gpio_irq_unmask;
	gpio->irq_chip.irq_set_type = phytium_gpio_irq_set_type;
	gpio->irq_chip.irq_enable = phytium_gpio_irq_enable;
	gpio->irq_chip.irq_disable = phytium_gpio_irq_disable;
#ifdef CONFIG_SMP
	/* TODO: use irq_chip_set_affinity_parent instead? */
	gpio->irq_chip.irq_set_affinity = phytium_gpio_irq_set_affinity;
#endif
	raw_spin_lock_init(&gpio->lock);

	gpio->gc.base = -1;
	gpio->gc.get_direction = phytium_gpio_get_direction;
	gpio->gc.direction_input = phytium_gpio_direction_input;
	gpio->gc.direction_output = phytium_gpio_direction_output;
	gpio->gc.get = phytium_gpio_get;
	gpio->gc.set = phytium_gpio_set;
	gpio->gc.ngpio = gpio->ngpio[0] + gpio->ngpio[1];
	gpio->gc.label = dev_name(dev);
	gpio->gc.parent = dev;
	gpio->gc.owner = THIS_MODULE;

	girq = &gpio->gc.irq;
	girq->handler = handle_bad_irq;
	girq->default_type = IRQ_TYPE_NONE;

	for (irq_count = 0; irq_count < platform_irq_count(pdev); irq_count++) {
		gpio->irq[irq_count] = -ENXIO;
		gpio->irq[irq_count] = platform_get_irq(pdev, irq_count);
		if (gpio->irq[irq_count] < 0) {
			//dev_warn(dev, "no irq is found.\n");
			break;
		}
	};

	girq->num_parents = irq_count;
	girq->parents = gpio->irq;
	girq->parent_handler = phytium_gpio_irq_handler;

	girq->chip = &gpio->irq_chip;

	err = devm_gpiochip_add_data(dev, &gpio->gc, gpio);
	if (err)
		return err;

	platform_set_drvdata(pdev, gpio);
	dev_info(dev, "Phytium GPIO controller @%pa registered\n",
		&res->start);

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phytium_gpio_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct phytium_gpio *gpio = platform_get_drvdata(pdev);
	unsigned long flags;

	raw_spin_lock_irqsave(&gpio->lock, flags);

	gpio->ctx.swporta_dr = readl(gpio->regs + GPIO_SWPORTA_DR);
	gpio->ctx.swporta_ddr = readl(gpio->regs + GPIO_SWPORTA_DDR);
	gpio->ctx.ext_porta = readl(gpio->regs + GPIO_EXT_PORTA);
	gpio->ctx.swportb_dr = readl(gpio->regs + GPIO_SWPORTB_DR);
	gpio->ctx.swportb_ddr = readl(gpio->regs + GPIO_SWPORTB_DDR);
	gpio->ctx.ext_portb = readl(gpio->regs + GPIO_EXT_PORTB);

	gpio->ctx.inten = readl(gpio->regs + GPIO_INTEN);
	gpio->ctx.intmask = readl(gpio->regs + GPIO_INTMASK);
	gpio->ctx.inttype_level = readl(gpio->regs + GPIO_INTTYPE_LEVEL);
	gpio->ctx.int_polarity = readl(gpio->regs + GPIO_INT_POLARITY);
	gpio->ctx.debounce = readl(gpio->regs + GPIO_DEBOUNCE);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}

static int phytium_gpio_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct phytium_gpio *gpio = platform_get_drvdata(pdev);
	unsigned long flags;

	raw_spin_lock_irqsave(&gpio->lock, flags);

	writel(gpio->ctx.swporta_dr, gpio->regs + GPIO_SWPORTA_DR);
	writel(gpio->ctx.swporta_ddr, gpio->regs + GPIO_SWPORTA_DDR);
	writel(gpio->ctx.ext_porta, gpio->regs + GPIO_EXT_PORTA);
	writel(gpio->ctx.swportb_dr, gpio->regs + GPIO_SWPORTB_DR);
	writel(gpio->ctx.swportb_ddr, gpio->regs + GPIO_SWPORTB_DDR);
	writel(gpio->ctx.ext_portb, gpio->regs + GPIO_EXT_PORTB);

	writel(gpio->ctx.inten, gpio->regs + GPIO_INTEN);
	writel(gpio->ctx.intmask, gpio->regs + GPIO_INTMASK);
	writel(gpio->ctx.inttype_level, gpio->regs + GPIO_INTTYPE_LEVEL);
	writel(gpio->ctx.int_polarity, gpio->regs + GPIO_INT_POLARITY);
	writel(gpio->ctx.debounce, gpio->regs + GPIO_DEBOUNCE);

	writel(0xffffffff, gpio->regs + GPIO_PORTA_EOI);

	raw_spin_unlock_irqrestore(&gpio->lock, flags);

	return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(phytium_gpio_pm_ops, phytium_gpio_suspend,
			 phytium_gpio_resume);

static struct platform_driver phytium_gpio_driver = {
	.driver		= {
		.name	= "gpio-phytium-platform",
		.pm	= &phytium_gpio_pm_ops,
		.of_match_table = of_match_ptr(phytium_gpio_of_match),
		.acpi_match_table = ACPI_PTR(phytium_gpio_acpi_match),
	},
	.probe		= phytium_gpio_probe,
};

module_platform_driver(phytium_gpio_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium GPIO driver");
