/*
 * Moschip MCS814x GPIO support
 *
 * Copyright (C) 2012, Florian Fainelli <florian@openwrt.org>
 *
 * Licensed under the GPLv2
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>

struct mcs814x_gpio_chip {
	void __iomem *regs;
	struct gpio_chip chip;
};

#define GPIO_PIN	0x00
#define GPIO_DIR	0x04

#define to_mcs814x_gpio_chip(x)	container_of(x, struct mcs814x_gpio_chip, chip)

static int mcs814x_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct mcs814x_gpio_chip *mcs814x = to_mcs814x_gpio_chip(chip);

	return readl_relaxed(mcs814x->regs + GPIO_PIN) & (1 << offset);
}

static void mcs814x_gpio_set(struct gpio_chip *chip,
				unsigned offset, int value)
{
	struct mcs814x_gpio_chip *mcs814x = to_mcs814x_gpio_chip(chip);
	u32 mask;

	mask = readl_relaxed(mcs814x->regs + GPIO_PIN);
	if (value)
		mask |= (1 << offset);
	else
		mask &= ~(1 << offset);
	writel_relaxed(mask, mcs814x->regs + GPIO_PIN);
}

static int mcs814x_gpio_direction_output(struct gpio_chip *chip,
					unsigned offset, int value)
{
	struct mcs814x_gpio_chip *mcs814x = to_mcs814x_gpio_chip(chip);
	u32 mask;

	mask = readl_relaxed(mcs814x->regs + GPIO_DIR);
	mask &= ~(1 << offset);
	writel_relaxed(mask, mcs814x->regs + GPIO_DIR);

	return 0;
}

static int mcs814x_gpio_direction_input(struct gpio_chip *chip,
					unsigned offset)
{
	struct mcs814x_gpio_chip *mcs814x = to_mcs814x_gpio_chip(chip);
	u32 mask;

	mask = readl_relaxed(mcs814x->regs + GPIO_DIR);
	mask |= (1 << offset);
	writel_relaxed(mask, mcs814x->regs + GPIO_DIR);

	return 0;
}

static int mcs814x_gpio_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct mcs814x_gpio_chip *mcs814x_chip;
	int ret;
	const unsigned int *num_gpios;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -ENODEV;

	num_gpios = of_get_property(pdev->dev.of_node, "num-gpios", NULL);
	if (!num_gpios)
		dev_err(&pdev->dev, "FIXME: no num-gpios property\n");

	mcs814x_chip = kzalloc(sizeof(*mcs814x_chip), GFP_KERNEL);
	if (!mcs814x_chip)
		return -ENOMEM;

	mcs814x_chip->regs = devm_ioremap_resource(&pdev->dev, res);
	if (!mcs814x_chip->regs) {
		ret = -ENOMEM;
		goto out;
	}

	platform_set_drvdata(pdev, mcs814x_chip);

#ifdef CONFIG_OF_GPIO
	mcs814x_chip->chip.of_node = pdev->dev.of_node;
#endif

	mcs814x_chip->chip.label = pdev->name;
	mcs814x_chip->chip.get = mcs814x_gpio_get;
	mcs814x_chip->chip.set = mcs814x_gpio_set;
	mcs814x_chip->chip.direction_input = mcs814x_gpio_direction_input;
	mcs814x_chip->chip.direction_output = mcs814x_gpio_direction_output;
	mcs814x_chip->chip.ngpio = be32_to_cpup(num_gpios);
	/* we want dynamic base allocation */
	mcs814x_chip->chip.base = -1;

	ret = gpiochip_add(&mcs814x_chip->chip);
	if (ret) {
		dev_err(&pdev->dev, "failed to register gpiochip\n");
		goto out;
	}

	return 0;

out:
	platform_set_drvdata(pdev, NULL);
	kfree(mcs814x_chip);
	return ret;
}

static struct of_device_id mcs814x_gpio_ids[] = {
	{ .compatible = "moschip,mcs814x-gpio" },
	{ /* sentinel */ },
};

static struct platform_driver mcs814x_gpio_driver = {
	.driver	= {
		.name	= "mcs814x-gpio",
		.owner	= THIS_MODULE,
		.of_match_table = mcs814x_gpio_ids,
	},
	.probe	= mcs814x_gpio_probe,
};

int __init mcs814x_gpio_init(void)
{
	return platform_driver_register(&mcs814x_gpio_driver);
}
postcore_initcall(mcs814x_gpio_init);
