/*
 *  GPIO latch driver
 *
 *  Copyright (C) 2014 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/platform_device.h>

#include <linux/platform_data/gpio-latch.h>

struct gpio_latch_chip {
	struct gpio_chip gc;

	struct mutex mutex;
	struct mutex latch_mutex;
	bool latch_enabled;
	int le_gpio;
	bool le_active_low;
	int *gpios;
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
	ret = gpio_get_value(glc->gpios[offset]);
	gpio_latch_unlock(glc, false);

	return ret;
}

static void
gpio_latch_set(struct gpio_chip *gc, unsigned offset, int value)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	bool enable_latch = false;
	bool disable_latch = false;
	int gpio;

	gpio = glc->gpios[offset];

	if (gpio == glc->le_gpio) {
		enable_latch = value ^ glc->le_active_low;
		disable_latch = !enable_latch;
	}

	gpio_latch_lock(glc, enable_latch);
	gpio_set_value(gpio, value);
	gpio_latch_unlock(glc, disable_latch);
}

static int
gpio_latch_direction_input(struct gpio_chip *gc, unsigned offset)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	int ret;

	gpio_latch_lock(glc, false);
	ret = gpio_direction_input(glc->gpios[offset]);
	gpio_latch_unlock(glc, false);

	return ret;
}

static int
gpio_latch_direction_output(struct gpio_chip *gc, unsigned offset, int value)
{
	struct gpio_latch_chip *glc = to_gpio_latch_chip(gc);
	bool enable_latch = false;
	bool disable_latch = false;
	int gpio;
	int ret;

	gpio = glc->gpios[offset];

	if (gpio == glc->le_gpio) {
		enable_latch = value ^ glc->le_active_low;
		disable_latch = !enable_latch;
	}

	gpio_latch_lock(glc, enable_latch);
	ret = gpio_direction_output(gpio, value);
	gpio_latch_unlock(glc, disable_latch);

	return ret;
}

static int gpio_latch_probe(struct platform_device *pdev)
{
	struct gpio_latch_chip *glc;
	struct gpio_latch_platform_data *pdata;
	struct gpio_chip *gc;
	int size;
	int ret;
	int i;

	pdata = dev_get_platdata(&pdev->dev);
	if (!pdata)
		return -EINVAL;

	if (pdata->le_gpio_index >= pdata->num_gpios ||
	    !pdata->num_gpios ||
	    !pdata->gpios)
		return -EINVAL;

	for (i = 0; i < pdata->num_gpios; i++) {
		int gpio = pdata->gpios[i];

		ret = devm_gpio_request(&pdev->dev, gpio,
					GPIO_LATCH_DRIVER_NAME);
		if (ret)
			return ret;
	}

	glc = devm_kzalloc(&pdev->dev, sizeof(*glc), GFP_KERNEL);
	if (!glc)
		return -ENOMEM;

	mutex_init(&glc->mutex);
	mutex_init(&glc->latch_mutex);

	size = pdata->num_gpios * sizeof(glc->gpios[0]);
	glc->gpios = devm_kzalloc(&pdev->dev, size , GFP_KERNEL);
	if (!glc->gpios)
		return -ENOMEM;

	memcpy(glc->gpios, pdata->gpios, size);

	glc->le_gpio = glc->gpios[pdata->le_gpio_index];
	glc->le_active_low = pdata->le_active_low;

	gc = &glc->gc;

	gc->label = GPIO_LATCH_DRIVER_NAME;
	gc->base = pdata->base;
	gc->can_sleep = true;
	gc->ngpio = pdata->num_gpios;
	gc->get = gpio_latch_get;
	gc->set = gpio_latch_set;
	gc->direction_input = gpio_latch_direction_input,
	gc->direction_output = gpio_latch_direction_output;

	platform_set_drvdata(pdev, glc);

	ret = gpiochip_add(&glc->gc);
	if (ret)
		return ret;

	return 0;
}

static int gpio_latch_remove(struct platform_device *pdev)
{
	struct gpio_latch_chip *glc = platform_get_drvdata(pdev);

	gpiochip_remove(&glc->gc);
	return 0;
}


static struct platform_driver gpio_latch_driver = {
	.probe = gpio_latch_probe,
	.remove = gpio_latch_remove,
	.driver = {
		.name = GPIO_LATCH_DRIVER_NAME,
		.owner = THIS_MODULE,
	},
};

static int __init gpio_latch_init(void)
{
	return platform_driver_register(&gpio_latch_driver);
}

postcore_initcall(gpio_latch_init);

MODULE_DESCRIPTION("GPIO latch driver");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" GPIO_LATCH_DRIVER_NAME);
