/*
 * LED ADM5120 Switch Port State Trigger
 *
 * Copyright (C) 2007 Bernhard Held <bernhard at bernhardheld.de>
 * Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was based on: drivers/leds/ledtrig-timer.c
 *	Copyright 2005-2006 Openedhand Ltd.
 *	Author: Richard Purdie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>

#include <linux/gpio.h>

#include "leds.h"

#define DRV_NAME "port_state"
#define DRV_DESC "LED ADM5120 Switch Port State Trigger"

struct port_state {
	char *name;
	unsigned int value;
};

#define PORT_STATE(n, v) {.name = (n), .value = (v)}

static struct port_state port_states[] = {
	PORT_STATE("off",		LED_OFF),
	PORT_STATE("on",		LED_FULL),
	PORT_STATE("flash",		ADM5120_GPIO_FLASH),
	PORT_STATE("link",		ADM5120_GPIO_LINK),
	PORT_STATE("speed",		ADM5120_GPIO_SPEED),
	PORT_STATE("duplex",		ADM5120_GPIO_DUPLEX),
	PORT_STATE("act",		ADM5120_GPIO_ACT),
	PORT_STATE("coll",		ADM5120_GPIO_COLL),
	PORT_STATE("link_act",		ADM5120_GPIO_LINK_ACT),
	PORT_STATE("duplex_coll",	ADM5120_GPIO_DUPLEX_COLL),
	PORT_STATE("10M_act",		ADM5120_GPIO_10M_ACT),
	PORT_STATE("100M_act",		ADM5120_GPIO_100M_ACT),
};

static ssize_t led_port_state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct port_state *state = led_cdev->trigger_data;
	int len = 0;
	int i;

	*buf = '\0';
	for (i = 0; i < ARRAY_SIZE(port_states); i++) {
		if (&port_states[i] == state)
			len += sprintf(buf+len, "[%s] ", port_states[i].name);
		else
			len += sprintf(buf+len, "%s ", port_states[i].name);
	}
	len += sprintf(buf+len, "\n");

	return len;
}

static ssize_t led_port_state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	size_t len;
	int i;

	for (i = 0; i < ARRAY_SIZE(port_states); i++) {
		len = strlen(port_states[i].name);
		if (strncmp(port_states[i].name, buf, len) != 0)
			continue;

		if (buf[len] != '\0' && buf[len] != '\n')
			continue;

		led_cdev->trigger_data = &port_states[i];
		led_set_brightness(led_cdev, port_states[i].value);
		return size;
	}

	return -EINVAL;
}

static DEVICE_ATTR(port_state, 0644, led_port_state_show,
			 led_port_state_store);

static void adm5120_switch_trig_activate(struct led_classdev *led_cdev)
{
	struct port_state *state = port_states;
	int rc;

	led_cdev->trigger_data = state;

	rc = device_create_file(led_cdev->dev, &dev_attr_port_state);
	if (rc)
		goto err;

	led_set_brightness(led_cdev, state->value);

	return;
err:
	led_cdev->trigger_data = NULL;
}

static void adm5120_switch_trig_deactivate(struct led_classdev *led_cdev)
{
	struct port_state *state = led_cdev->trigger_data;

	if (!state)
		return;

	device_remove_file(led_cdev->dev, &dev_attr_port_state);

}

static struct led_trigger adm5120_switch_led_trigger = {
	.name		= DRV_NAME,
	.activate	= adm5120_switch_trig_activate,
	.deactivate	= adm5120_switch_trig_deactivate,
};

static int __init adm5120_switch_trig_init(void)
{
	led_trigger_register(&adm5120_switch_led_trigger);
	return 0;
}

static void __exit adm5120_switch_trig_exit(void)
{
	led_trigger_unregister(&adm5120_switch_led_trigger);
}

module_init(adm5120_switch_trig_init);
module_exit(adm5120_switch_trig_exit);

MODULE_AUTHOR("Bernhard Held <bernhard at bernhardheld.de>, "
		"Gabor Juhos <juhosg@openwrt.org>");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_LICENSE("GPL v2");
