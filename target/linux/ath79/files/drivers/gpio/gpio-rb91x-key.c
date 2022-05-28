// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Driver for reset key gpio line on MikroTik RB91x board series.
 * This line is shared between NAND ALE (goes through a latch),
 * NAND IO7 and reset key. We make 3 virtual gpio lines from the
 * single physical one:
 *   1) Capable output one for NAND,
 *   2) Capable input one for reset key,
 *   3) And capable output one, aka "key-poll-disable",
 *      for NAND -- to syncronise NAND operation and key polling.
 *
 * Copyright (C) 2021 Denis Kalashnikov <denis281089@gmail.com>
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/gpio/consumer.h>
#include <linux/gpio/driver.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#define GPIO_RB91X_KEY_DRIVER_NAME  "gpio-rb91x-key"

enum gpio_rb91x_key_gpios {
	GPIO_RB91X_KEY_NAND,
	GPIO_RB91X_KEY_POLL,
	GPIO_RB91X_KEY_PDIS,

	GPIO_RB91X_KEY_NGPIOS,
};

struct gpio_rb91x_key {
	struct gpio_chip gc;
	struct mutex mutex;
	struct mutex poll_mutex;
	int polling_disabled;
	struct gpio_desc *gpio;
};

static inline struct gpio_rb91x_key *to_gpio_rb91x_key(struct gpio_chip *gc)
{
	return container_of(gc, struct gpio_rb91x_key, gc);
}

static int gpio_rb91x_key_get(struct gpio_chip *gc, unsigned offset)
{
	struct gpio_rb91x_key *drvdata = to_gpio_rb91x_key(gc);
	struct gpio_desc *gpio = drvdata->gpio;
	int val, bak_val;

	switch (offset) {
	case GPIO_RB91X_KEY_NAND:
		mutex_lock(&drvdata->mutex);
		val = gpiod_get_value_cansleep(gpio);
		mutex_unlock(&drvdata->mutex);
		break;
	case GPIO_RB91X_KEY_PDIS:
		mutex_lock(&drvdata->mutex);
		val = drvdata->polling_disabled;
		mutex_unlock(&drvdata->mutex);
		break;
	case GPIO_RB91X_KEY_POLL:
		mutex_lock(&drvdata->poll_mutex);
		mutex_lock(&drvdata->mutex);
		bak_val = gpiod_get_raw_value_cansleep(gpio);
		gpiod_direction_input(gpio);
		/*
		 * Without this delay nothing works. Get it
		 * from mikrotik RouterOS linux kernel patches.
		 */
		udelay(200);
		val = gpiod_get_raw_value_cansleep(gpio);
		gpiod_direction_output_raw(gpio, bak_val);
		mutex_unlock(&drvdata->mutex);
		mutex_unlock(&drvdata->poll_mutex);
		break;
	default:
		return -EINVAL;
	}

	return val;
}

static int gpio_rb91x_key_direction_input(struct gpio_chip *gc, unsigned offset)
{
	switch (offset) {
	case GPIO_RB91X_KEY_POLL:
		return 0;
	default:
		return -EINVAL;
	}
}

static void gpio_rb91x_key_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct gpio_rb91x_key *drvdata = to_gpio_rb91x_key(gc);
	struct gpio_desc *gpio = drvdata->gpio;

	mutex_lock(&drvdata->mutex);

	switch (offset) {
	case GPIO_RB91X_KEY_NAND:
		gpiod_set_raw_value_cansleep(gpio, value);
		break;
	case GPIO_RB91X_KEY_PDIS:
		if (value) {
			if (!drvdata->polling_disabled) {
				mutex_lock(&drvdata->poll_mutex);
				drvdata->polling_disabled = 1;
			}
		} else {
			if (drvdata->polling_disabled) {
				mutex_unlock(&drvdata->poll_mutex);
				drvdata->polling_disabled = 0;
			}
		}
		break;
	default:
		break;
	}

	mutex_unlock(&drvdata->mutex);
}

static int gpio_rb91x_key_direction_output(struct gpio_chip *gc, unsigned offset,
					   int value)
{
	switch (offset) {
	case GPIO_RB91X_KEY_NAND:
	case GPIO_RB91X_KEY_PDIS:
		gpio_rb91x_key_set(gc, offset, value);
		return 0;
	default:
		return -EINVAL;
	}
}

static int gpio_rb91x_key_probe(struct platform_device *pdev)
{
	struct gpio_rb91x_key *drvdata;
	struct gpio_chip *gc;
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	int r;

	drvdata = devm_kzalloc(dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	mutex_init(&drvdata->mutex);
	mutex_init(&drvdata->poll_mutex);

	drvdata->gpio = devm_gpiod_get(dev, NULL, GPIOD_OUT_LOW);
	if (IS_ERR(drvdata->gpio)) {
		if (PTR_ERR(drvdata->gpio) != -EPROBE_DEFER) {
			dev_err(dev, "failed to get gpio: %ld\n",
				PTR_ERR(drvdata->gpio));
		}
		return PTR_ERR(drvdata->gpio);
	}

	gc = &drvdata->gc;
	gc->label = GPIO_RB91X_KEY_DRIVER_NAME;
	gc->can_sleep = 1;
	gc->base = -1;
	gc->ngpio = GPIO_RB91X_KEY_NGPIOS;
	gc->get = gpio_rb91x_key_get;
	gc->set = gpio_rb91x_key_set;
	gc->direction_output = gpio_rb91x_key_direction_output;
	gc->direction_input = gpio_rb91x_key_direction_input;
	gc->of_node = of_node;

	platform_set_drvdata(pdev, drvdata);

	r = gpiochip_add(&drvdata->gc);
	if (r) {
		dev_err(dev, "gpiochip_add() failed: %d\n", r);
		return r;
	}

	return 0;
}

static int gpio_rb91x_key_remove(struct platform_device *pdev)
{
	struct gpio_rb91x_key *drvdata = platform_get_drvdata(pdev);

	gpiochip_remove(&drvdata->gc);
	return 0;
}

static const struct of_device_id gpio_rb91x_key_match[] = {
	{ .compatible = "mikrotik,"GPIO_RB91X_KEY_DRIVER_NAME },
	{},
};

MODULE_DEVICE_TABLE(of, gpio_rb91x_key_match);

static struct platform_driver gpio_rb91x_key_driver = {
	.probe = gpio_rb91x_key_probe,
	.remove = gpio_rb91x_key_remove,
	.driver = {
		.name = GPIO_RB91X_KEY_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = gpio_rb91x_key_match,
	},
};

module_platform_driver(gpio_rb91x_key_driver);

MODULE_DESCRIPTION("Driver for reset key gpio line shared with NAND for MikroTik RB91x board series.");
MODULE_AUTHOR("Denis Kalashnikov <denis281089@gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" GPIO_RB91X_KEY_DRIVER_NAME);
