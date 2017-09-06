/*
 * LED driver for NU801
 *
 * Kevin Paul Herbert
 * Copyright (c) 2012, Meraki, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/leds-nu801.h>

#include <linux/gpio.h>
#include <linux/of_gpio.h>

#define MAX_NAME_LENGTH 24
#define NUM_COLORS 3

static const char * const led_nu801_colors[] = { "blue", "green", "red" };

struct led_nu801_led_data {
	struct led_classdev cdev;
	struct led_nu801_data *controller;
	enum led_brightness level;
	char name[MAX_NAME_LENGTH];
};

struct led_nu801_data {
	unsigned cki;
	unsigned sdi;
	int lei;
	struct delayed_work work;
	struct led_nu801_led_data *led_chain;
	int num_leds;
	const char *device_name;
	const char *name;
	u32 ndelay;
	atomic_t pending;
};

static void led_nu801_work(struct work_struct *work)
{
	struct led_nu801_data	*controller =
		container_of(work, struct led_nu801_data, work.work);
	struct led_nu801_led_data *led;
	u16 bit;
	u16 brightness;
	int index;

	for (index = 0; index < controller->num_leds; index++) {
		led = &controller->led_chain[index];
		brightness = led->level << 8; /* To do: gamma correction */
		for (bit = 0x8000; bit; bit = bit >> 1) {
			gpio_set_value(controller->sdi,
				       (brightness & bit) != 0);
			gpio_set_value(controller->cki, 1);
			if (unlikely(((index == (controller->num_leds - 1)) &&
				      (bit == 1) &&
				      (controller->lei < 0)))) {
				udelay(600);
			} else {
				ndelay(controller->ndelay);
			}
			gpio_set_value(controller->cki, 0);
			ndelay(controller->ndelay);
		}
	}
	if (controller->lei >= 0) {
		gpio_set_value(controller->lei, 1);
		ndelay(controller->ndelay);
		gpio_set_value(controller->lei, 0);
	}
	atomic_set(&controller->pending, 1);
}

static void led_nu801_set(struct led_classdev *led_cdev,
			  enum led_brightness value)
{
	struct led_nu801_led_data *led_dat =
		container_of(led_cdev, struct led_nu801_led_data, cdev);
	struct led_nu801_data *controller = led_dat->controller;

	if (led_dat->level != value) {
		led_dat->level = value;
		if (atomic_dec_and_test(&controller->pending))
			schedule_delayed_work(&led_dat->controller->work,
					      (HZ/1000) + 1);
	}
}

static int __init led_nu801_create(struct led_nu801_data *controller,
				    struct device *parent,
				    int index,
				    enum led_brightness brightness,
#ifdef CONFIG_LEDS_TRIGGERS
				    const char *default_trigger,
#endif
				    const char *color)
{
	struct led_nu801_led_data *led = &controller->led_chain[index];
	int ret;

	scnprintf(led->name, sizeof(led->name), "%s:%s:%s%d",
		  controller->device_name, color, controller->name,
		  (controller->num_leds - (index + 1)) / NUM_COLORS);
	led->cdev.name = led->name;
	led->cdev.brightness_set = led_nu801_set;
#ifdef CONFIG_LEDS_TRIGGERS
	led->cdev.default_trigger = default_trigger;
#endif
	led->level = brightness;
	led->controller = controller;
	ret = led_classdev_register(parent, &led->cdev);
	if (ret < 0)
		goto err;

	return 0;

err:
	kfree(led);
	return ret;
}

static int __init
led_nu801_create_chain(const struct led_nu801_template *template,
			struct led_nu801_data *controller,
			struct device *parent)
{
	int ret;
	int index;

	controller->cki = template->cki;
	controller->sdi = template->sdi;
	controller->lei = template->lei;
	controller->num_leds = template->num_leds * 3;
	controller->device_name = template->device_name;
	controller->name = template->name;
	controller->ndelay = template->ndelay;
	atomic_set(&controller->pending, 1);

	controller->led_chain = kzalloc(sizeof(struct led_nu801_led_data) *
					controller->num_leds, GFP_KERNEL);

	if (!controller->led_chain)
		return -ENOMEM;

	ret = gpio_request(controller->cki, template->name);
	if (ret < 0)
		goto err_free_chain;

	ret = gpio_request(controller->sdi, template->name);
	if (ret < 0)
		goto err_ret_cki;

	if (controller->lei >= 0) {
		ret = gpio_request(controller->lei, template->name);
		if (ret < 0)
			goto err_ret_sdi;
		ret = gpio_direction_output(controller->lei, 0);
		if (ret < 0)
			goto err_ret_lei;
	}

	ret = gpio_direction_output(controller->cki, 0);
	if (ret < 0)
		goto err_ret_lei;

	ret = gpio_direction_output(controller->sdi, 0);
	if (ret < 0)
		goto err_ret_lei;

	for (index = 0; index < controller->num_leds; index++) {
		ret = led_nu801_create(controller, parent, index,
			template->init_brightness
			[index % NUM_COLORS],
#ifdef CONFIG_LEDS_TRIGGERS
			template->default_trigger,
#endif
			template->led_colors[index % NUM_COLORS] ?
			template->led_colors[index % NUM_COLORS] :
			led_nu801_colors[index % NUM_COLORS]);
		if (ret < 0)
			goto err_ret_sdi;
	}

	INIT_DELAYED_WORK(&controller->work, led_nu801_work);
	schedule_delayed_work(&controller->work, 0);

	return 0;

err_ret_lei:
	if (controller->lei >= 0)
		gpio_free(controller->lei);
err_ret_sdi:
	gpio_free(controller->sdi);
err_ret_cki:
	gpio_free(controller->cki);
err_free_chain:
	kfree(controller->led_chain);

	return ret;
}

static void led_nu801_delete_chain(struct led_nu801_data *controller)
{
	struct led_nu801_led_data *led_chain;
	struct led_nu801_led_data *led;
	int index;
	int num_leds;

	led_chain = controller->led_chain;
	controller->led_chain = 0;
	num_leds = controller->num_leds;
	controller->num_leds = 0;
	cancel_delayed_work_sync(&controller->work);

	for (index = 0; index < num_leds; index++) {
		led = &led_chain[index];
		led_classdev_unregister(&led->cdev);
	}

	gpio_free(controller->cki);
	gpio_free(controller->sdi);
	if (controller->lei >= 0)
		gpio_free(controller->lei);

	kfree(led_chain);
}

static struct led_nu801_data * __init
leds_nu801_create_of(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node, *child;
	struct led_nu801_data *controllers;
	int count = 0, ret;
	int i = 0;

	for_each_child_of_node(np, child)
		count++;
	if (!count)
		return NULL;

	controllers = kzalloc(sizeof(struct led_nu801_data) * count,
			      GFP_KERNEL);
	if (!controllers)
		return NULL;

	for_each_child_of_node(np, child) {
		const char *state;
		struct led_nu801_template template = {};
		struct device_node *colors;
		int jj;

		template.cki = of_get_named_gpio_flags(child, "cki", 0, NULL);
		template.sdi = of_get_named_gpio_flags(child, "sdi", 0, NULL);
		if (of_find_property(child, "lei", NULL)) {
			template.lei = of_get_named_gpio_flags(child, "lei",
							       0, NULL);
		} else {
			template.lei = -1;
		}
		of_property_read_u32(child, "ndelay", &template.ndelay);
		of_property_read_u32(child, "num_leds", &template.num_leds);
		template.name = of_get_property(child, "label", NULL) ? :
			child->name;
		template.default_trigger = of_get_property(child,
			"default-trigger", NULL);

		jj = 0;
		for_each_child_of_node(child, colors) {
			template.led_colors[jj] = of_get_property(colors,
				 "label", NULL);
			state = of_get_property(colors, "state", NULL);
			if (!strncmp(state, "off", 3))
				template.init_brightness[jj] = LED_OFF;
			else if (!strncmp(state, "half", 4))
				template.init_brightness[jj] = LED_HALF;
			else if (!strncmp(state, "full", 4))
				template.init_brightness[jj] = LED_FULL;
			jj++;
		}

		ret = led_nu801_create_chain(&template,
					     &controllers[i],
					     &pdev->dev);
		if (ret < 0)
			goto err;
		i++;
	}

	return controllers;

err:
	for (i = i - 1; i >= 0; i--)
		led_nu801_delete_chain(&controllers[i]);
	kfree(controllers);
	return NULL;
}

static int __init led_nu801_probe(struct platform_device *pdev)
{
	struct led_nu801_platform_data *pdata = pdev->dev.platform_data;
	struct led_nu801_data *controllers;
	int i, ret = 0;

	if (!(pdata && pdata->num_controllers)) {
		controllers = leds_nu801_create_of(pdev);
		if (!controllers)
			return -ENODEV;
	}

	controllers = kzalloc(sizeof(struct led_nu801_data) *
			      pdata->num_controllers, GFP_KERNEL);
	if (!controllers)
		return -ENOMEM;

	for (i = 0; i < pdata->num_controllers; i++) {
		ret = led_nu801_create_chain(&pdata->template[i],
					      &controllers[i],
					      &pdev->dev);
		if (ret < 0)
			goto err;
	}

	platform_set_drvdata(pdev, controllers);

	return 0;

err:
	for (i = i - 1; i >= 0; i--)
		led_nu801_delete_chain(&controllers[i]);

	kfree(controllers);

	return ret;
}

static int led_nu801_remove(struct platform_device *pdev)
{
	int i;
	struct led_nu801_platform_data *pdata = pdev->dev.platform_data;
	struct led_nu801_data *controllers;

	controllers = platform_get_drvdata(pdev);

	for (i = 0; i < pdata->num_controllers; i++)
		led_nu801_delete_chain(&controllers[i]);

	kfree(controllers);

	return 0;
}

static const struct of_device_id of_numen_leds_match[] = {
	{ .compatible = "numen,leds-nu801", },
	{},
};
MODULE_DEVICE_TABLE(of, of_pwm_leds_match);

static struct platform_driver led_nu801_driver = {
	.probe		= led_nu801_probe,
	.remove		= led_nu801_remove,
	.driver		= {
		.name	= "leds-nu801",
		.owner	= THIS_MODULE,
		.of_match_table = of_numen_leds_match,
	},
};

static int __init led_nu801_init(void)
{
	return platform_driver_register(&led_nu801_driver);
}

static void __exit led_nu801_exit(void)
{
	platform_driver_unregister(&led_nu801_driver);
}

module_init(led_nu801_init);
module_exit(led_nu801_exit);

MODULE_AUTHOR("Kevin Paul Herbert <kph@meraki.net>");
MODULE_DESCRIPTION("NU801 LED driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:leds-nu801");
