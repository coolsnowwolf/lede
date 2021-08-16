// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *  GPIO latch driver
 *
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
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

#define GPIO_LATCH_DRIVER_NAME  "gpio-latch"
#define GPIO_LATCH_LINES 9

struct gpio_latch_chip {
	struct gpio_chip gc;
	struct mutex mutex;
	struct mutex latch_mutex;
	bool latch_enabled;
	int le_gpio;
	bool le_active_low;
	struct gpio_desc *gpios[GPIO_LATCH_LINES];
};

static inline struct gpio_latch_chip *to_gpio_latch_chip(struct gpio_chip *gc)
{
	return container_of(gc, struct gpio_latch_chip, gc);
}

static void gpio_latch_lock(struct gpio_latch_chip *glc, bool enable)
{
	mutex_lock(&glc->mutex);

	if (enable)
		glc->latch_enabled = true;

	if (glc->latch_enabled)
		mutex_lock(&glc->latch_mutex);
}

static void gpio_latch_unlock(struct gpio_latch_chip *glc, bool disable)
{
	if (glc->latch_enabled)
		mutex_unlock(&glc->latch_mutex);

	if (disable)
		glc->latch_enabled = true;

	mutex_unlock(&glc->mutex);
}

static int
gpio_latch_get(struct gpio_chip *gc, unsigned offset)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	int ret;

	gpio_latch_lock(glc, false);
	ret = gpiod_get_value(glc->gpios[offset]);
	gpio_latch_unlock(glc, false);

	return ret;
}

static void
gpio_latch_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	bool enable_latch = false;
	bool disable_latch = false;

	if (offset == glc->le_gpio) {
		enable_latch = value ^ glc->le_active_low;
		disable_latch = !enable_latch;
	}

	gpio_latch_lock(glc, enable_latch);
	gpiod_set_raw_value(glc->gpios[offset], value);
	gpio_latch_unlock(glc, disable_latch);
}

static int
gpio_latch_direction_output(struct gpio_chip *gc, unsigned offset, int value)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	bool enable_latch = false;
	bool disable_latch = false;
	int ret;

	if (offset == glc->le_gpio) {
		enable_latch = value ^ glc->le_active_low;
		disable_latch = !enable_latch;
	}

	gpio_latch_lock(glc, enable_latch);
	ret = gpiod_direction_output_raw(glc->gpios[offset], value);
	gpio_latch_unlock(glc, disable_latch);

	return ret;
}

static int gpio_latch_probe(struct platform_device *pdev)
{
	struct gpio_latch_chip *glc;
	struct gpio_chip *gc;
	struct device *dev = &pdev->dev;
	struct device_node *of_node = dev->of_node;
	int i, n;

	glc = devm_kzalloc(dev, sizeof(*glc), GFP_KERNEL);
	if (!glc)
		return -ENOMEM;

	mutex_init(&glc->mutex);
	mutex_init(&glc->latch_mutex);

	n = gpiod_count(dev, NULL);
	if (n <= 0) {
		dev_err(dev, "failed to get gpios: %d\n", n);
		return n;
	} else if (n != GPIO_LATCH_LINES) {
		dev_err(dev, "expected %d gpios\n", GPIO_LATCH_LINES);
		return -EINVAL;
	}

	for (i = 0; i < n; i++) {
		glc->gpios[i] = devm_gpiod_get_index_optional(dev, NULL, i,
			GPIOD_OUT_LOW);
		if (IS_ERR(glc->gpios[i])) {
			dev_err(dev, "failed to get gpio %d: %d\n", i,
				PTR_ERR(glc->gpios[i]));
			return PTR_ERR(glc->gpios[i]);
		}
	}

	glc->le_gpio = 8;
	glc->le_active_low = gpiod_is_active_low(glc->gpios[glc->le_gpio]);

	if (!glc->gpios[glc->le_gpio]) {
		dev_err(dev, "missing required latch-enable gpio %d\n",
			glc->le_gpio);
		return -EINVAL;
	}

	gc = &glc->gc;
	gc->label = GPIO_LATCH_DRIVER_NAME;
	gc->can_sleep = true;
	gc->base = -1;
	gc->ngpio = GPIO_LATCH_LINES;
	gc->get = gpio_latch_get;
	gc->set = gpio_latch_set;
	gc->direction_output = gpio_latch_direction_output;
	gc->of_node = of_node;

	platform_set_drvdata(pdev, glc);

	i = gpiochip_add(&glc->gc);
	if (i) {
		dev_err(dev, "gpiochip_add() failed: %d\n", i);
		return i;
	}

	return 0;
}

static int gpio_latch_remove(struct platform_device *pdev)
{
	struct gpio_latch_chip *glc = platform_get_drvdata(pdev);

	gpiochip_remove(&glc->gc);
	return 0;
}

static const struct of_device_id gpio_latch_match[] = {
	{ .compatible = GPIO_LATCH_DRIVER_NAME },
	{},
};

MODULE_DEVICE_TABLE(of, gpio_latch_match);

static struct platform_driver gpio_latch_driver = {
	.probe = gpio_latch_probe,
	.remove = gpio_latch_remove,
	.driver = {
		.name = GPIO_LATCH_DRIVER_NAME,
		.owner = THIS_MODULE,
		.of_match_table = gpio_latch_match,
	},
};

module_platform_driver(gpio_latch_driver);

MODULE_DESCRIPTION("GPIO latch driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Denis Kalashnikov <denis281089@gmail.com>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" GPIO_LATCH_DRIVER_NAME);
