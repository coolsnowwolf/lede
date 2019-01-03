/*
 * LED Kernel Netdev Trigger
 *
 * Toggles the LED to reflect the link and traffic state of a named net device
 *
 * Copyright 2007 Oliver Jowett <oliver@opencloud.com>
 *
 * Derived from ledtrig-timer.c which is:
 *  Copyright 2005-2006 Openedhand Ltd.
 *  Author: Richard Purdie <rpurdie@openedhand.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/module.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/netdevice.h>
#include <linux/timer.h>
#include <linux/ctype.h>
#include <linux/leds.h>

#include "leds.h"

/*
 * Configurable sysfs attributes:
 *
 * device_name - network device name to monitor
 *
 * interval - duration of LED blink, in milliseconds
 *
 * mode - either "none" (LED is off) or a space separated list of one or more of:
 *   link: LED's normal state reflects whether the link is up (has carrier) or not
 *   tx:   LED blinks on transmitted data
 *   rx:   LED blinks on receive data
 *
 * Some suggestions:
 *
 *  Simple link status LED:
 *  $ echo netdev >someled/trigger
 *  $ echo eth0 >someled/device_name
 *  $ echo link >someled/mode
 *
 *  Ethernet-style link/activity LED:
 *  $ echo netdev >someled/trigger
 *  $ echo eth0 >someled/device_name
 *  $ echo "link tx rx" >someled/mode
 *
 *  Modem-style tx/rx LEDs:
 *  $ echo netdev >led1/trigger
 *  $ echo ppp0 >led1/device_name
 *  $ echo tx >led1/mode
 *  $ echo netdev >led2/trigger
 *  $ echo ppp0 >led2/device_name
 *  $ echo rx >led2/mode
 *
 */

#define MODE_LINK 1
#define MODE_TX   2
#define MODE_RX   4

struct led_netdev_data {
	spinlock_t lock;

	struct delayed_work work;
	struct notifier_block notifier;

	struct led_classdev *led_cdev;
	struct net_device *net_dev;

	char device_name[IFNAMSIZ];
	unsigned interval;
	unsigned mode;
	unsigned link_up;
	unsigned last_activity;
};

static void set_baseline_state(struct led_netdev_data *trigger_data)
{
	if ((trigger_data->mode & MODE_LINK) != 0 && trigger_data->link_up)
		led_set_brightness(trigger_data->led_cdev, LED_FULL);
	else
		led_set_brightness(trigger_data->led_cdev, LED_OFF);

	if ((trigger_data->mode & (MODE_TX | MODE_RX)) != 0 && trigger_data->link_up)
		schedule_delayed_work(&trigger_data->work, trigger_data->interval);
}

static ssize_t led_device_name_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;

	spin_lock_bh(&trigger_data->lock);
	sprintf(buf, "%s\n", trigger_data->device_name);
	spin_unlock_bh(&trigger_data->lock);

	return strlen(buf) + 1;
}

static ssize_t led_device_name_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;

	if (size >= IFNAMSIZ)
		return -EINVAL;

	cancel_delayed_work_sync(&trigger_data->work);

	spin_lock_bh(&trigger_data->lock);

	strcpy(trigger_data->device_name, buf);
	if (size > 0 && trigger_data->device_name[size-1] == '\n')
		trigger_data->device_name[size-1] = 0;
	trigger_data->link_up = 0;
	trigger_data->last_activity = 0;

	if (trigger_data->device_name[0] != 0) {
		/* check for existing device to update from */
		trigger_data->net_dev = dev_get_by_name(&init_net, trigger_data->device_name);
		if (trigger_data->net_dev != NULL)
			trigger_data->link_up = (dev_get_flags(trigger_data->net_dev) & IFF_LOWER_UP) != 0;
	}

	set_baseline_state(trigger_data);
	spin_unlock_bh(&trigger_data->lock);

	return size;
}

static DEVICE_ATTR(device_name, 0644, led_device_name_show, led_device_name_store);

static ssize_t led_mode_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;

	spin_lock_bh(&trigger_data->lock);

	if (trigger_data->mode == 0) {
		strcpy(buf, "none\n");
	} else {
		if (trigger_data->mode & MODE_LINK)
			strcat(buf, "link ");
		if (trigger_data->mode & MODE_TX)
			strcat(buf, "tx ");
		if (trigger_data->mode & MODE_RX)
			strcat(buf, "rx ");
		strcat(buf, "\n");
	}

	spin_unlock_bh(&trigger_data->lock);

	return strlen(buf)+1;
}

static ssize_t led_mode_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;
	char copybuf[128];
	int new_mode = -1;
	char *p, *token;

	/* take a copy since we don't want to trash the inbound buffer when using strsep */
	strncpy(copybuf, buf, sizeof(copybuf));
	copybuf[sizeof(copybuf) - 1] = 0;
	p = copybuf;

	while ((token = strsep(&p, " \t\n")) != NULL) {
		if (!*token)
			continue;

		if (new_mode == -1)
			new_mode = 0;

		if (!strcmp(token, "none"))
			new_mode = 0;
		else if (!strcmp(token, "tx"))
			new_mode |= MODE_TX;
		else if (!strcmp(token, "rx"))
			new_mode |= MODE_RX;
		else if (!strcmp(token, "link"))
			new_mode |= MODE_LINK;
		else
			return -EINVAL;
	}

	if (new_mode == -1)
		return -EINVAL;

	cancel_delayed_work_sync(&trigger_data->work);

	spin_lock_bh(&trigger_data->lock);
	trigger_data->mode = new_mode;
	set_baseline_state(trigger_data);
	spin_unlock_bh(&trigger_data->lock);

	return size;
}

static DEVICE_ATTR(mode, 0644, led_mode_show, led_mode_store);

static ssize_t led_interval_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;

	spin_lock_bh(&trigger_data->lock);
	sprintf(buf, "%u\n", jiffies_to_msecs(trigger_data->interval));
	spin_unlock_bh(&trigger_data->lock);

	return strlen(buf) + 1;
}

static ssize_t led_interval_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;
	int ret = -EINVAL;
	char *after;
	unsigned long value = simple_strtoul(buf, &after, 10);
	size_t count = after - buf;

	if (isspace(*after))
		count++;

	/* impose some basic bounds on the timer interval */
	if (count == size && value >= 5 && value <= 10000) {
		cancel_delayed_work_sync(&trigger_data->work);

		spin_lock_bh(&trigger_data->lock);
		trigger_data->interval = msecs_to_jiffies(value);
		set_baseline_state(trigger_data); /* resets timer */
		spin_unlock_bh(&trigger_data->lock);

		ret = count;
	}

	return ret;
}

static DEVICE_ATTR(interval, 0644, led_interval_show, led_interval_store);

static int netdev_trig_notify(struct notifier_block *nb,
			      unsigned long evt,
			      void *dv)
{
	struct net_device *dev = netdev_notifier_info_to_dev((struct netdev_notifier_info *) dv);
	struct led_netdev_data *trigger_data = container_of(nb, struct led_netdev_data, notifier);

	if (evt != NETDEV_UP && evt != NETDEV_DOWN && evt != NETDEV_CHANGE && evt != NETDEV_REGISTER && evt != NETDEV_UNREGISTER && evt != NETDEV_CHANGENAME)
		return NOTIFY_DONE;

	if (strcmp(dev->name, trigger_data->device_name))
		return NOTIFY_DONE;

	cancel_delayed_work_sync(&trigger_data->work);

	spin_lock_bh(&trigger_data->lock);

	if (evt == NETDEV_REGISTER || evt == NETDEV_CHANGENAME) {
		if (trigger_data->net_dev != NULL)
			dev_put(trigger_data->net_dev);

		dev_hold(dev);
		trigger_data->net_dev = dev;
		trigger_data->link_up = 0;
		goto done;
	}

	if (evt == NETDEV_UNREGISTER && trigger_data->net_dev != NULL) {
		dev_put(trigger_data->net_dev);
		trigger_data->net_dev = NULL;
		goto done;
	}

	/* UP / DOWN / CHANGE */

	trigger_data->link_up = (evt != NETDEV_DOWN && netif_carrier_ok(dev));
	set_baseline_state(trigger_data);

done:
	spin_unlock_bh(&trigger_data->lock);
	return NOTIFY_DONE;
}

/* here's the real work! */
static void netdev_trig_work(struct work_struct *work)
{
	struct led_netdev_data *trigger_data = container_of(work, struct led_netdev_data, work.work);
	struct rtnl_link_stats64 *dev_stats;
	unsigned new_activity;
	struct rtnl_link_stats64 temp;

	if (!trigger_data->link_up || !trigger_data->net_dev || (trigger_data->mode & (MODE_TX | MODE_RX)) == 0) {
		/* we don't need to do timer work, just reflect link state. */
		led_set_brightness(trigger_data->led_cdev, ((trigger_data->mode & MODE_LINK) != 0 && trigger_data->link_up) ? LED_FULL : LED_OFF);
		return;
	}

	dev_stats = dev_get_stats(trigger_data->net_dev, &temp);
	new_activity =
		((trigger_data->mode & MODE_TX) ? dev_stats->tx_packets : 0) +
		((trigger_data->mode & MODE_RX) ? dev_stats->rx_packets : 0);

	if (trigger_data->mode & MODE_LINK) {
		/* base state is ON (link present) */
		/* if there's no link, we don't get this far and the LED is off */

		/* OFF -> ON always */
		/* ON -> OFF on activity */
		if (trigger_data->led_cdev->brightness == LED_OFF) {
			led_set_brightness(trigger_data->led_cdev, LED_FULL);
		} else if (trigger_data->last_activity != new_activity) {
			led_set_brightness(trigger_data->led_cdev, LED_OFF);
		}
	} else {
		/* base state is OFF */
		/* ON -> OFF always */
		/* OFF -> ON on activity */
		if (trigger_data->led_cdev->brightness == LED_FULL) {
			led_set_brightness(trigger_data->led_cdev, LED_OFF);
		} else if (trigger_data->last_activity != new_activity) {
			led_set_brightness(trigger_data->led_cdev, LED_FULL);
		}
	}

	trigger_data->last_activity = new_activity;
	schedule_delayed_work(&trigger_data->work, trigger_data->interval);
}

static void netdev_trig_activate(struct led_classdev *led_cdev)
{
	struct led_netdev_data *trigger_data;
	int rc;

	trigger_data = kzalloc(sizeof(struct led_netdev_data), GFP_KERNEL);
	if (!trigger_data)
		return;

	spin_lock_init(&trigger_data->lock);

	trigger_data->notifier.notifier_call = netdev_trig_notify;
	trigger_data->notifier.priority = 10;

	INIT_DELAYED_WORK(&trigger_data->work, netdev_trig_work);

	trigger_data->led_cdev = led_cdev;
	trigger_data->net_dev = NULL;
	trigger_data->device_name[0] = 0;

	trigger_data->mode = 0;
	trigger_data->interval = msecs_to_jiffies(50);
	trigger_data->link_up = 0;
	trigger_data->last_activity = 0;

	led_cdev->trigger_data = trigger_data;

	rc = device_create_file(led_cdev->dev, &dev_attr_device_name);
	if (rc)
		goto err_out;
	rc = device_create_file(led_cdev->dev, &dev_attr_mode);
	if (rc)
		goto err_out_device_name;
	rc = device_create_file(led_cdev->dev, &dev_attr_interval);
	if (rc)
		goto err_out_mode;

	register_netdevice_notifier(&trigger_data->notifier);
	return;

err_out_mode:
	device_remove_file(led_cdev->dev, &dev_attr_mode);
err_out_device_name:
	device_remove_file(led_cdev->dev, &dev_attr_device_name);
err_out:
	led_cdev->trigger_data = NULL;
	kfree(trigger_data);
}

static void netdev_trig_deactivate(struct led_classdev *led_cdev)
{
	struct led_netdev_data *trigger_data = led_cdev->trigger_data;

	if (trigger_data) {
		unregister_netdevice_notifier(&trigger_data->notifier);

		device_remove_file(led_cdev->dev, &dev_attr_device_name);
		device_remove_file(led_cdev->dev, &dev_attr_mode);
		device_remove_file(led_cdev->dev, &dev_attr_interval);

		cancel_delayed_work_sync(&trigger_data->work);

		spin_lock_bh(&trigger_data->lock);

		if (trigger_data->net_dev) {
			dev_put(trigger_data->net_dev);
			trigger_data->net_dev = NULL;
		}

		spin_unlock_bh(&trigger_data->lock);

		kfree(trigger_data);
	}
}

static struct led_trigger netdev_led_trigger = {
	.name     = "netdev",
	.activate = netdev_trig_activate,
	.deactivate = netdev_trig_deactivate,
};

static int __init netdev_trig_init(void)
{
	return led_trigger_register(&netdev_led_trigger);
}

static void __exit netdev_trig_exit(void)
{
	led_trigger_unregister(&netdev_led_trigger);
}

module_init(netdev_trig_init);
module_exit(netdev_trig_exit);

MODULE_AUTHOR("Oliver Jowett <oliver@opencloud.com>");
MODULE_DESCRIPTION("Netdev LED trigger");
MODULE_LICENSE("GPL");
