/*
 * LED driver for the RouterBOARD 750
 *
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/slab.h>

#include <asm/mach-ath79/mach-rb750.h>

#define DRV_NAME	"leds-rb750"

struct rb750_led_dev {
	struct led_classdev	cdev;
	u32			mask;
	int			active_low;
	void			(*latch_change)(u32 clear, u32 set);
};

struct rb750_led_drvdata {
	struct rb750_led_dev	*led_devs;
	int			num_leds;
};

static inline struct rb750_led_dev *to_rbled(struct led_classdev *led_cdev)
{
	return (struct rb750_led_dev *)container_of(led_cdev,
		struct rb750_led_dev, cdev);
}

static void rb750_led_brightness_set(struct led_classdev *led_cdev,
				     enum led_brightness value)
{
	struct rb750_led_dev *rbled = to_rbled(led_cdev);
	int level;

	level = (value == LED_OFF) ? 0 : 1;
	level ^= rbled->active_low;

	if (level)
		rbled->latch_change(0, rbled->mask);
	else
		rbled->latch_change(rbled->mask, 0);
}

static int rb750_led_probe(struct platform_device *pdev)
{
	struct rb750_led_platform_data *pdata;
	struct rb750_led_drvdata *drvdata;
	int ret = 0;
	int i;

	pdata = pdev->dev.platform_data;
	if (!pdata)
		return -EINVAL;

	drvdata = kzalloc(sizeof(struct rb750_led_drvdata) +
			  sizeof(struct rb750_led_dev) * pdata->num_leds,
			  GFP_KERNEL);
	if (!drvdata)
		return -ENOMEM;

	drvdata->num_leds = pdata->num_leds;
	drvdata->led_devs = (struct rb750_led_dev *) &drvdata[1];

	for (i = 0; i < drvdata->num_leds; i++) {
		struct rb750_led_dev *rbled = &drvdata->led_devs[i];
		struct rb750_led_data *led_data = &pdata->leds[i];

		rbled->cdev.name = led_data->name;
		rbled->cdev.default_trigger = led_data->default_trigger;
		rbled->cdev.brightness_set = rb750_led_brightness_set;
		rbled->cdev.brightness = LED_OFF;

		rbled->mask = led_data->mask;
		rbled->active_low = !!led_data->active_low;
		rbled->latch_change = pdata->latch_change;

		ret = led_classdev_register(&pdev->dev, &rbled->cdev);
		if (ret)
			goto err;
	}

	platform_set_drvdata(pdev, drvdata);
	return 0;

err:
	for (i = i - 1; i >= 0; i--)
		led_classdev_unregister(&drvdata->led_devs[i].cdev);

	kfree(drvdata);
	return ret;
}

static int rb750_led_remove(struct platform_device *pdev)
{
	struct rb750_led_drvdata *drvdata;
	int i;

	drvdata = platform_get_drvdata(pdev);
	for (i = 0; i < drvdata->num_leds; i++)
		led_classdev_unregister(&drvdata->led_devs[i].cdev);

	kfree(drvdata);
	return 0;
}

static struct platform_driver rb750_led_driver = {
	.probe		= rb750_led_probe,
	.remove		= rb750_led_remove,
	.driver		= {
		.name	= DRV_NAME,
		.owner	= THIS_MODULE,
	},
};

MODULE_ALIAS("platform:leds-rb750");

static int __init rb750_led_init(void)
{
	return platform_driver_register(&rb750_led_driver);
}

static void __exit rb750_led_exit(void)
{
	platform_driver_unregister(&rb750_led_driver);
}

module_init(rb750_led_init);
module_exit(rb750_led_exit);

MODULE_DESCRIPTION(DRV_NAME);
MODULE_DESCRIPTION("LED driver for the RouterBOARD 750");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
