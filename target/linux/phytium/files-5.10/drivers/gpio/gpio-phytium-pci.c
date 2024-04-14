// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>

#include "gpio-phytium-core.h"

static int phytium_gpio_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct device *dev = &pdev->dev;
	struct phytium_gpio *gpio;
	struct gpio_irq_chip *girq;
	int err;

	gpio = devm_kzalloc(&pdev->dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	err = pcim_enable_device(pdev);
	if (err) {
		dev_err(dev, "Failed to enable PCI device: err %d\n", err);
		goto out;
	}

	err = pcim_iomap_regions(pdev, 1 << 0, pci_name(pdev));
	if (err) {
		dev_err(dev, "Failed to iomap PCI device: err %d\n", err);
		goto out;
	}

	gpio->regs = pcim_iomap_table(pdev)[0];
	if (!gpio->regs) {
		dev_err(dev, "Cannot map PCI resource\n");
		err = -ENOMEM;
		goto out;
	}

	err = pci_enable_msi(pdev);
	if (err < 0)
		goto out;

	pci_set_master(pdev);

	gpio->irq[0] = pdev->irq;
	if (gpio->irq[0] < 0)
		dev_warn(dev, "no irq is found.\n");

	/* There is only one group of Pins at the moment. */
	gpio->ngpio[0] = NGPIO_MAX;

	/* irq_chip support */
	gpio->irq_chip.name = dev_name(dev);
	gpio->irq_chip.irq_ack = phytium_gpio_irq_ack;
	gpio->irq_chip.irq_mask = phytium_gpio_irq_mask;
	gpio->irq_chip.irq_unmask = phytium_gpio_irq_unmask;
	gpio->irq_chip.irq_set_type = phytium_gpio_irq_set_type;
	gpio->irq_chip.irq_enable = phytium_gpio_irq_enable;
	gpio->irq_chip.irq_disable = phytium_gpio_irq_disable;

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

	girq->num_parents = 1;
	girq->parents = devm_kcalloc(&pdev->dev, girq->num_parents,
				     sizeof(*girq->parents), GFP_KERNEL);
	if (!girq->parents)
		return -ENOMEM;
	girq->parents[0] = gpio->irq[0];
	girq->parent_handler = phytium_gpio_irq_handler;

	girq->chip = &gpio->irq_chip;

	err = devm_gpiochip_add_data(dev, &gpio->gc, gpio);
	if (err)
		goto out;

	dev_info(dev, "Phytium PCI GPIO controller @%pa registered\n",
		&gpio->regs);

	pci_set_drvdata(pdev, gpio);

out:
	return err;
}

static const struct pci_device_id phytium_gpio_pci_ids[] = {
	{ PCI_DEVICE(0x1DB7, 0xDC31) },
	{ 0 }
};
MODULE_DEVICE_TABLE(pci, phytium_gpio_pci_ids);

#ifdef CONFIG_PM_SLEEP
static int phytium_gpio_pci_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_gpio *gpio = pci_get_drvdata(pdev);
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

static int phytium_gpio_pci_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_gpio *gpio = pci_get_drvdata(pdev);
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

static SIMPLE_DEV_PM_OPS(phytium_gpio_pci_pm_ops,
			 phytium_gpio_pci_suspend,
			 phytium_gpio_pci_resume);

static struct pci_driver phytium_gpio_pci_driver = {
	.name		= "gpio-phytium-pci",
	.id_table	= phytium_gpio_pci_ids,
	.probe		= phytium_gpio_pci_probe,
	.driver		= {
		.pm	= &phytium_gpio_pci_pm_ops,
	},
};

module_pci_driver(phytium_gpio_pci_driver);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium GPIO PCI Driver");
