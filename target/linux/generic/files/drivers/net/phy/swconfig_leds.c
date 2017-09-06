/*
 * swconfig_led.c: LED trigger support for the switch configuration API
 *
 * Copyright (C) 2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 */

#ifdef CONFIG_SWCONFIG_LEDS

#include <linux/leds.h>
#include <linux/ctype.h>
#include <linux/device.h>
#include <linux/workqueue.h>

#define SWCONFIG_LED_TIMER_INTERVAL	(HZ / 10)
#define SWCONFIG_LED_NUM_PORTS		32

#define SWCONFIG_LED_PORT_SPEED_NA	0x01	/* unknown speed */
#define SWCONFIG_LED_PORT_SPEED_10	0x02	/* 10 Mbps */
#define SWCONFIG_LED_PORT_SPEED_100	0x04	/* 100 Mbps */
#define SWCONFIG_LED_PORT_SPEED_1000	0x08	/* 1000 Mbps */
#define SWCONFIG_LED_PORT_SPEED_ALL	(SWCONFIG_LED_PORT_SPEED_NA | \
					 SWCONFIG_LED_PORT_SPEED_10 | \
					 SWCONFIG_LED_PORT_SPEED_100 | \
					 SWCONFIG_LED_PORT_SPEED_1000)

struct switch_led_trigger {
	struct led_trigger trig;
	struct switch_dev *swdev;

	struct delayed_work sw_led_work;
	u32 port_mask;
	u32 port_link;
	unsigned long long port_traffic[SWCONFIG_LED_NUM_PORTS];
	u8 link_speed[SWCONFIG_LED_NUM_PORTS];
};

struct swconfig_trig_data {
	struct led_classdev *led_cdev;
	struct switch_dev *swdev;

	rwlock_t lock;
	u32 port_mask;

	bool prev_link;
	unsigned long prev_traffic;
	enum led_brightness prev_brightness;
	u8 speed_mask;
};

static void
swconfig_trig_set_brightness(struct swconfig_trig_data *trig_data,
			     enum led_brightness brightness)
{
	led_set_brightness(trig_data->led_cdev, brightness);
	trig_data->prev_brightness = brightness;
}

static void
swconfig_trig_update_port_mask(struct led_trigger *trigger)
{
	struct list_head *entry;
	struct switch_led_trigger *sw_trig;
	u32 port_mask;

	if (!trigger)
		return;

	sw_trig = (void *) trigger;

	port_mask = 0;
	read_lock(&trigger->leddev_list_lock);
	list_for_each(entry, &trigger->led_cdevs) {
		struct led_classdev *led_cdev;
		struct swconfig_trig_data *trig_data;

		led_cdev = list_entry(entry, struct led_classdev, trig_list);
		trig_data = led_cdev->trigger_data;
		if (trig_data) {
			read_lock(&trig_data->lock);
			port_mask |= trig_data->port_mask;
			read_unlock(&trig_data->lock);
		}
	}
	read_unlock(&trigger->leddev_list_lock);

	sw_trig->port_mask = port_mask;

	if (port_mask)
		schedule_delayed_work(&sw_trig->sw_led_work,
				      SWCONFIG_LED_TIMER_INTERVAL);
	else
		cancel_delayed_work_sync(&sw_trig->sw_led_work);
}

static ssize_t
swconfig_trig_port_mask_store(struct device *dev, struct device_attribute *attr,
			      const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct swconfig_trig_data *trig_data = led_cdev->trigger_data;
	unsigned long port_mask;
	int ret;
	bool changed;

	ret = kstrtoul(buf, 0, &port_mask);
	if (ret)
		return ret;

	write_lock(&trig_data->lock);

	changed = (trig_data->port_mask != port_mask);
	if (changed) {
		trig_data->port_mask = port_mask;
		if (port_mask == 0)
			swconfig_trig_set_brightness(trig_data, LED_OFF);
	}

	write_unlock(&trig_data->lock);

	if (changed)
		swconfig_trig_update_port_mask(led_cdev->trigger);

	return size;
}

static ssize_t
swconfig_trig_port_mask_show(struct device *dev, struct device_attribute *attr,
			     char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct swconfig_trig_data *trig_data = led_cdev->trigger_data;

	read_lock(&trig_data->lock);
	sprintf(buf, "%#x\n", trig_data->port_mask);
	read_unlock(&trig_data->lock);

	return strlen(buf) + 1;
}

static DEVICE_ATTR(port_mask, 0644, swconfig_trig_port_mask_show,
		   swconfig_trig_port_mask_store);

/* speed_mask file handler - display value */
static ssize_t swconfig_trig_speed_mask_show(struct device *dev,
					     struct device_attribute *attr,
					     char *buf)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct swconfig_trig_data *trig_data = led_cdev->trigger_data;

	read_lock(&trig_data->lock);
	sprintf(buf, "%#x\n", trig_data->speed_mask);
	read_unlock(&trig_data->lock);

	return strlen(buf) + 1;
}

/* speed_mask file handler - store value */
static ssize_t swconfig_trig_speed_mask_store(struct device *dev,
					      struct device_attribute *attr,
					      const char *buf, size_t size)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct swconfig_trig_data *trig_data = led_cdev->trigger_data;
	u8 speed_mask;
	int ret;

	ret = kstrtou8(buf, 0, &speed_mask);
	if (ret)
		return ret;

	write_lock(&trig_data->lock);
	trig_data->speed_mask = speed_mask & SWCONFIG_LED_PORT_SPEED_ALL;
	write_unlock(&trig_data->lock);

	return size;
}

/* speed_mask special file */
static DEVICE_ATTR(speed_mask, 0644, swconfig_trig_speed_mask_show,
		   swconfig_trig_speed_mask_store);

static void
swconfig_trig_activate(struct led_classdev *led_cdev)
{
	struct switch_led_trigger *sw_trig;
	struct swconfig_trig_data *trig_data;
	int err;

	if (led_cdev->trigger->activate != swconfig_trig_activate)
		return;

	trig_data = kzalloc(sizeof(struct swconfig_trig_data), GFP_KERNEL);
	if (!trig_data)
		return;

	sw_trig = (void *) led_cdev->trigger;

	rwlock_init(&trig_data->lock);
	trig_data->led_cdev = led_cdev;
	trig_data->swdev = sw_trig->swdev;
	trig_data->speed_mask = SWCONFIG_LED_PORT_SPEED_ALL;
	led_cdev->trigger_data = trig_data;

	err = device_create_file(led_cdev->dev, &dev_attr_port_mask);
	if (err)
		goto err_free;

	err = device_create_file(led_cdev->dev, &dev_attr_speed_mask);
	if (err)
		goto err_dev_free;

	return;

err_dev_free:
	device_remove_file(led_cdev->dev, &dev_attr_port_mask);

err_free:
	led_cdev->trigger_data = NULL;
	kfree(trig_data);
}

static void
swconfig_trig_deactivate(struct led_classdev *led_cdev)
{
	struct swconfig_trig_data *trig_data;

	swconfig_trig_update_port_mask(led_cdev->trigger);

	trig_data = (void *) led_cdev->trigger_data;
	if (trig_data) {
		device_remove_file(led_cdev->dev, &dev_attr_port_mask);
		device_remove_file(led_cdev->dev, &dev_attr_speed_mask);
		kfree(trig_data);
	}
}

static void
swconfig_trig_led_event(struct switch_led_trigger *sw_trig,
			struct led_classdev *led_cdev)
{
	struct swconfig_trig_data *trig_data;
	u32 port_mask;
	bool link;
	u8 speed_mask;

	trig_data = led_cdev->trigger_data;
	if (!trig_data)
		return;

	read_lock(&trig_data->lock);
	port_mask = trig_data->port_mask;
	speed_mask = trig_data->speed_mask;
	read_unlock(&trig_data->lock);

	link = !!(sw_trig->port_link & port_mask);
	if (!link) {
		if (link != trig_data->prev_link)
			swconfig_trig_set_brightness(trig_data, LED_OFF);
	} else {
		unsigned long traffic;
		int speedok;	/* link speed flag */
		int i;

		traffic = 0;
		speedok = 0;
		for (i = 0; i < SWCONFIG_LED_NUM_PORTS; i++) {
			if (port_mask & (1 << i))
				if (sw_trig->link_speed[i] & speed_mask) {
					traffic += sw_trig->port_traffic[i];
					speedok = 1;
				}
		}

		if (speedok) {
			/* At least one port speed matches speed_mask */
			if (trig_data->prev_brightness != LED_FULL)
				swconfig_trig_set_brightness(trig_data,
							     LED_FULL);
			else if (traffic != trig_data->prev_traffic)
				swconfig_trig_set_brightness(trig_data,
							     LED_OFF);
		} else if (trig_data->prev_brightness != LED_OFF)
			swconfig_trig_set_brightness(trig_data, LED_OFF);

		trig_data->prev_traffic = traffic;
	}

	trig_data->prev_link = link;
}

static void
swconfig_trig_update_leds(struct switch_led_trigger *sw_trig)
{
	struct list_head *entry;
	struct led_trigger *trigger;

	trigger = &sw_trig->trig;
	read_lock(&trigger->leddev_list_lock);
	list_for_each(entry, &trigger->led_cdevs) {
		struct led_classdev *led_cdev;

		led_cdev = list_entry(entry, struct led_classdev, trig_list);
		swconfig_trig_led_event(sw_trig, led_cdev);
	}
	read_unlock(&trigger->leddev_list_lock);
}

static void
swconfig_led_work_func(struct work_struct *work)
{
	struct switch_led_trigger *sw_trig;
	struct switch_dev *swdev;
	u32 port_mask;
	u32 link;
	int i;

	sw_trig = container_of(work, struct switch_led_trigger,
			       sw_led_work.work);

	port_mask = sw_trig->port_mask;
	swdev = sw_trig->swdev;

	link = 0;
	for (i = 0; i < SWCONFIG_LED_NUM_PORTS; i++) {
		u32 port_bit;

		sw_trig->link_speed[i] = 0;

		port_bit = BIT(i);
		if ((port_mask & port_bit) == 0)
			continue;

		if (swdev->ops->get_port_link) {
			struct switch_port_link port_link;

			memset(&port_link, '\0', sizeof(port_link));
			swdev->ops->get_port_link(swdev, i, &port_link);

			if (port_link.link) {
				link |= port_bit;
				switch (port_link.speed) {
				case SWITCH_PORT_SPEED_UNKNOWN:
					sw_trig->link_speed[i] =
						SWCONFIG_LED_PORT_SPEED_NA;
					break;
				case SWITCH_PORT_SPEED_10:
					sw_trig->link_speed[i] =
						SWCONFIG_LED_PORT_SPEED_10;
					break;
				case SWITCH_PORT_SPEED_100:
					sw_trig->link_speed[i] =
						SWCONFIG_LED_PORT_SPEED_100;
					break;
				case SWITCH_PORT_SPEED_1000:
					sw_trig->link_speed[i] =
						SWCONFIG_LED_PORT_SPEED_1000;
					break;
				}
			}
		}

		if (swdev->ops->get_port_stats) {
			struct switch_port_stats port_stats;

			memset(&port_stats, '\0', sizeof(port_stats));
			swdev->ops->get_port_stats(swdev, i, &port_stats);
			sw_trig->port_traffic[i] = port_stats.tx_bytes +
						   port_stats.rx_bytes;
		}
	}

	sw_trig->port_link = link;

	swconfig_trig_update_leds(sw_trig);

	schedule_delayed_work(&sw_trig->sw_led_work,
			      SWCONFIG_LED_TIMER_INTERVAL);
}

static int
swconfig_create_led_trigger(struct switch_dev *swdev)
{
	struct switch_led_trigger *sw_trig;
	int err;

	if (!swdev->ops->get_port_link)
		return 0;

	sw_trig = kzalloc(sizeof(struct switch_led_trigger), GFP_KERNEL);
	if (!sw_trig)
		return -ENOMEM;

	sw_trig->swdev = swdev;
	sw_trig->trig.name = swdev->devname;
	sw_trig->trig.activate = swconfig_trig_activate;
	sw_trig->trig.deactivate = swconfig_trig_deactivate;

	INIT_DELAYED_WORK(&sw_trig->sw_led_work, swconfig_led_work_func);

	err = led_trigger_register(&sw_trig->trig);
	if (err)
		goto err_free;

	swdev->led_trigger = sw_trig;

	return 0;

err_free:
	kfree(sw_trig);
	return err;
}

static void
swconfig_destroy_led_trigger(struct switch_dev *swdev)
{
	struct switch_led_trigger *sw_trig;

	sw_trig = swdev->led_trigger;
	if (sw_trig) {
		cancel_delayed_work_sync(&sw_trig->sw_led_work);
		led_trigger_unregister(&sw_trig->trig);
		kfree(sw_trig);
	}
}

#else /* SWCONFIG_LEDS */
static inline int
swconfig_create_led_trigger(struct switch_dev *swdev) { return 0; }

static inline void
swconfig_destroy_led_trigger(struct switch_dev *swdev) { }
#endif /* CONFIG_SWCONFIG_LEDS */
