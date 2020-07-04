// SPDX-License-Identifier: GPL-2.0-only
/*
 * GPIO driver for the MikroTik RouterBoard 4xx series
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 * Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 * Copyright (C) 2015 Bert Vermeulen <bert@biot.com>
 * Copyright (C) 2020 Christopher Hill <ch6574@gmail.com>
 *
 * This file was based on the driver for Linux 2.6.22 published by
 * MikroTik for their RouterBoard 4xx series devices.
 *
 * N.B. driver probe reports "DMA mask not set" warnings which are
 * an artifact of using a platform_driver as an MFD device child.
 * See conversation here https://lkml.org/lkml/2020/4/28/675
 */
#include <linux/platform_device.h>
#include <linux/gpio/driver.h>
#include <linux/module.h>
#include <linux/of_device.h>

#include <mfd/rb4xx-cpld.h>

struct rb4xx_gpio {
	struct rb4xx_cpld *cpld;
	struct device *dev;

	struct gpio_chip chip;
	struct mutex lock;
	u16 values;		/* bitfield of GPIO 0-8 current values */
};

static int rb4xx_gpio_cpld_set(struct rb4xx_gpio *gpio, unsigned int offset,
			       int value)
{
	struct rb4xx_cpld *cpld = gpio->cpld;
	u16 values;
	int ret;

	mutex_lock(&gpio->lock);
	values = gpio->values;

	if (value)
		values |= BIT(offset);
	else
		values &= ~(BIT(offset));

	if (values == gpio->values) {
		ret = 0;
		goto unlock;
	}

	if (offset < 8) {
		ret = cpld->gpio_set_0_7(cpld, values & 0xff);
	} else if (offset == 8) {
		ret = cpld->gpio_set_8(cpld, values >> 8);
	}

	if(likely(!ret))
		gpio->values = values;

unlock:
	mutex_unlock(&gpio->lock);
	return ret;
}

static int rb4xx_gpio_get_direction(struct gpio_chip *chip, unsigned int offset)
{
	return 0; /* All 9 GPIOs are out */
}

static int rb4xx_gpio_direction_input(struct gpio_chip *chip,
				      unsigned int offset)
{
	return -EOPNOTSUPP;
}

static int rb4xx_gpio_direction_output(struct gpio_chip *chip,
				       unsigned int offset, int value)
{
	return rb4xx_gpio_cpld_set(gpiochip_get_data(chip), offset, value);
}

static int rb4xx_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
	struct rb4xx_gpio *gpio = gpiochip_get_data(chip);
	int ret;

	mutex_lock(&gpio->lock);
	ret = (gpio->values >> offset) & 0x1;
	mutex_unlock(&gpio->lock);

	return ret;
}

static void rb4xx_gpio_set(struct gpio_chip *chip, unsigned int offset,
			   int value)
{
	rb4xx_gpio_cpld_set(gpiochip_get_data(chip), offset, value);
}

static int rb4xx_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device *parent = dev->parent;
	struct rb4xx_gpio *gpio;
	u32 val;

	if (!parent)
		return -ENODEV;

	gpio = devm_kzalloc(dev, sizeof(*gpio), GFP_KERNEL);
	if (!gpio)
		return -ENOMEM;

	platform_set_drvdata(pdev, gpio);
	gpio->cpld	= dev_get_drvdata(parent);
	gpio->dev	= dev;
	gpio->values	= 0;
	mutex_init(&gpio->lock);

	gpio->chip.label		= "rb4xx-gpio";
	gpio->chip.parent		= dev;
	gpio->chip.owner		= THIS_MODULE;
	gpio->chip.get_direction	= rb4xx_gpio_get_direction;
	gpio->chip.direction_input	= rb4xx_gpio_direction_input;
	gpio->chip.direction_output	= rb4xx_gpio_direction_output;
	gpio->chip.get			= rb4xx_gpio_get;
	gpio->chip.set			= rb4xx_gpio_set;
	gpio->chip.ngpio		= 9;
	gpio->chip.base			= -1;
	gpio->chip.can_sleep		= 1;

	if (!of_property_read_u32(dev->of_node, "base", &val))
		gpio->chip.base = val;

	return gpiochip_add_data(&gpio->chip, gpio);
}

static int rb4xx_gpio_remove(struct platform_device *pdev)
{
	struct rb4xx_gpio *gpio = platform_get_drvdata(pdev);

	gpiochip_remove(&gpio->chip);
	mutex_destroy(&gpio->lock);

	return 0;
}

static const struct platform_device_id rb4xx_gpio_id_table[] = {
	{ "mikrotik,rb4xx-gpio", },
	{ },
};
MODULE_DEVICE_TABLE(platform, rb4xx_gpio_id_table);

static struct platform_driver rb4xx_gpio_driver = {
	.probe = rb4xx_gpio_probe,
	.remove = rb4xx_gpio_remove,
	.id_table = rb4xx_gpio_id_table,
	.driver = {
		.name = "rb4xx-gpio",
	},
};

module_platform_driver(rb4xx_gpio_driver);

MODULE_DESCRIPTION("Mikrotik RB4xx GPIO driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_AUTHOR("Bert Vermeulen <bert@biot.com>");
MODULE_AUTHOR("Christopher Hill <ch6574@gmail.com");
MODULE_LICENSE("GPL v2");
