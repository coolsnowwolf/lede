/*
 * Copyright (C) 2018 John Crispin <john@phrozen.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#include <linux/err.h>
#include <linux/reset.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mod_devicetable.h>
#include <linux/of.h>
#include <linux/platform_device.h>

struct reset_led_data {
	struct reset_control *rst;
	struct led_classdev cdev;
};

static inline struct reset_led_data *
			cdev_to_reset_led_data(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct reset_led_data, cdev);
}

static void reset_led_set(struct led_classdev *led_cdev,
	enum led_brightness value)
{
	struct reset_led_data *led_dat = cdev_to_reset_led_data(led_cdev);

	if (value == LED_OFF)
		reset_control_assert(led_dat->rst);
	else
		reset_control_deassert(led_dat->rst);
}

struct reset_leds_priv {
	int num_leds;
	struct reset_led_data leds[];
};

static int reset_led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct reset_leds_priv *priv;
	int count;

	count = device_get_child_node_count(dev);
	if (!count)
		return -ENODEV;

	priv = devm_kzalloc(dev, struct_size(priv, leds, count), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	device_for_each_child_node_scoped(dev, child) {
		struct reset_led_data *led = &priv->leds[priv->num_leds];
		struct led_init_data init_data = {
			.fwnode = child,
		};
		int ret;

		ret = fwnode_property_read_string(child, "label", &led->cdev.name);
		if (ret)
			return ret;

		led->rst = of_reset_control_get_exclusive(to_of_node(child), NULL);
		if (IS_ERR(led->rst))
			return PTR_ERR(led->rst);

		fwnode_property_read_string(child, "linux,default-trigger",
						&led->cdev.default_trigger);

		led->cdev.brightness_set = reset_led_set;
		ret = devm_led_classdev_register_ext(dev, &led->cdev, &init_data);
		if (ret < 0)
			return ret;

		priv->num_leds++;
	}

	platform_set_drvdata(pdev, priv);

	return 0;
}

static void reset_led_shutdown(struct platform_device *pdev)
{
	struct reset_leds_priv *priv = platform_get_drvdata(pdev);
	int i;

	for (i = 0; i < priv->num_leds; i++) {
		struct reset_led_data *led = &priv->leds[i];

		if (!(led->cdev.flags & LED_RETAIN_AT_SHUTDOWN))
			reset_led_set(&led->cdev, LED_OFF);
	}
}

static const struct of_device_id of_reset_leds_match[] = {
	{ .compatible = "reset-leds", },
	{},
};

MODULE_DEVICE_TABLE(of, of_reset_leds_match);

static struct platform_driver reset_led_driver = {
	.probe		= reset_led_probe,
	.shutdown	= reset_led_shutdown,
	.driver		= {
		.name	= "leds-reset",
		.of_match_table = of_reset_leds_match,
	},
};

module_platform_driver(reset_led_driver);

MODULE_AUTHOR("John Crispin <john@phrozen.org>");
MODULE_DESCRIPTION("reset controller LED driver");
MODULE_LICENSE("GPL");
