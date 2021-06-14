/*
 *  GPIO Button Hotplug driver
 *
 *  Copyright (C) 2012 Felix Fietkau <nbd@nbd.name>
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *
 *  Based on the diag.c - GPIO interface driver for Broadcom boards
 *    Copyright (C) 2006 Mike Baker <mbm@openwrt.org>,
 *    Copyright (C) 2006-2007 Felix Fietkau <nbd@nbd.name>
 *    Copyright (C) 2008 Andy Boyett <agb@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kmod.h>

#include <linux/workqueue.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/kobject.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_gpio.h>
#include <linux/gpio_keys.h>

#define DRV_NAME	"gpio-keys"

#define BH_SKB_SIZE	2048

#define PFX	DRV_NAME ": "

#undef BH_DEBUG

#ifdef BH_DEBUG
#define BH_DBG(fmt, args...) printk(KERN_DEBUG "%s: " fmt, DRV_NAME, ##args )
#else
#define BH_DBG(fmt, args...) do {} while (0)
#endif

#define BH_ERR(fmt, args...) printk(KERN_ERR "%s: " fmt, DRV_NAME, ##args )

struct bh_priv {
	unsigned long		seen;
};

struct bh_event {
	const char		*name;
	unsigned int		type;
	char			*action;
	unsigned long		seen;

	struct sk_buff		*skb;
	struct work_struct	work;
};

struct bh_map {
	unsigned int	code;
	const char	*name;
};

struct gpio_keys_button_data {
	struct delayed_work work;
	struct bh_priv bh;
	int last_state;
	int count;
	int threshold;
	int can_sleep;
	struct gpio_keys_button *b;
};

extern u64 uevent_next_seqnum(void);

#define BH_MAP(_code, _name)		\
	{				\
		.code = (_code),	\
		.name = (_name),	\
	}

static struct bh_map button_map[] = {
	BH_MAP(BTN_0,			"BTN_0"),
	BH_MAP(BTN_1,			"BTN_1"),
	BH_MAP(BTN_2,			"BTN_2"),
	BH_MAP(BTN_3,			"BTN_3"),
	BH_MAP(BTN_4,			"BTN_4"),
	BH_MAP(BTN_5,			"BTN_5"),
	BH_MAP(BTN_6,			"BTN_6"),
	BH_MAP(BTN_7,			"BTN_7"),
	BH_MAP(BTN_8,			"BTN_8"),
	BH_MAP(BTN_9,			"BTN_9"),
	BH_MAP(KEY_BRIGHTNESS_ZERO,	"brightness_zero"),
	BH_MAP(KEY_CONFIG,		"config"),
	BH_MAP(KEY_COPY,		"copy"),
	BH_MAP(KEY_EJECTCD,		"eject"),
	BH_MAP(KEY_HELP,		"help"),
	BH_MAP(KEY_LIGHTS_TOGGLE,	"lights_toggle"),
	BH_MAP(KEY_PHONE,		"phone"),
	BH_MAP(KEY_POWER,		"power"),
	BH_MAP(KEY_RESTART,		"reset"),
	BH_MAP(KEY_RFKILL,		"rfkill"),
	BH_MAP(KEY_VIDEO,		"video"),
	BH_MAP(KEY_WIMAX,		"wwan"),
	BH_MAP(KEY_WLAN,		"wlan"),
	BH_MAP(KEY_WPS_BUTTON,		"wps"),
};

/* -------------------------------------------------------------------------*/

static __printf(3, 4)
int bh_event_add_var(struct bh_event *event, int argv, const char *format, ...)
{
	static char buf[128];
	char *s;
	va_list args;
	int len;

	if (argv)
		return 0;

	va_start(args, format);
	len = vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);

	if (len >= sizeof(buf)) {
		WARN(1, "buffer size too small");
		return -ENOMEM;
	}

	s = skb_put(event->skb, len + 1);
	strcpy(s, buf);

	BH_DBG("added variable '%s'\n", s);

	return 0;
}

static int button_hotplug_fill_event(struct bh_event *event)
{
	int ret;

	ret = bh_event_add_var(event, 0, "HOME=%s", "/");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "PATH=%s",
					"/sbin:/bin:/usr/sbin:/usr/bin");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SUBSYSTEM=%s", "button");
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "ACTION=%s", event->action);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "BUTTON=%s", event->name);
	if (ret)
		return ret;

	if (event->type == EV_SW) {
		ret = bh_event_add_var(event, 0, "TYPE=%s", "switch");
		if (ret)
			return ret;
	}

	ret = bh_event_add_var(event, 0, "SEEN=%ld", event->seen);
	if (ret)
		return ret;

	ret = bh_event_add_var(event, 0, "SEQNUM=%llu", uevent_next_seqnum());

	return ret;
}

static void button_hotplug_work(struct work_struct *work)
{
	struct bh_event *event = container_of(work, struct bh_event, work);
	int ret = 0;

	event->skb = alloc_skb(BH_SKB_SIZE, GFP_KERNEL);
	if (!event->skb)
		goto out_free_event;

	ret = bh_event_add_var(event, 0, "%s@", event->action);
	if (ret)
		goto out_free_skb;

	ret = button_hotplug_fill_event(event);
	if (ret)
		goto out_free_skb;

	NETLINK_CB(event->skb).dst_group = 1;
	broadcast_uevent(event->skb, 0, 1, GFP_KERNEL);

 out_free_skb:
	if (ret) {
		BH_ERR("work error %d\n", ret);
		kfree_skb(event->skb);
	}
 out_free_event:
	kfree(event);
}

static int button_hotplug_create_event(const char *name, unsigned int type,
		unsigned long seen, int pressed)
{
	struct bh_event *event;

	BH_DBG("create event, name=%s, seen=%lu, pressed=%d\n",
		name, seen, pressed);

	event = kzalloc(sizeof(*event), GFP_KERNEL);
	if (!event)
		return -ENOMEM;

	event->name = name;
	event->type = type;
	event->seen = seen;
	event->action = pressed ? "pressed" : "released";

	INIT_WORK(&event->work, (void *)(void *)button_hotplug_work);
	schedule_work(&event->work);

	return 0;
}

/* -------------------------------------------------------------------------*/

static int button_get_index(unsigned int code)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(button_map); i++)
		if (button_map[i].code == code)
			return i;

	return -1;
}

static void button_hotplug_event(struct gpio_keys_button_data *data,
			   unsigned int type, int value)
{
	struct bh_priv *priv = &data->bh;
	unsigned long seen = jiffies;
	int btn;

	BH_DBG("event type=%u, code=%u, value=%d\n", type, data->b->code, value);

	if ((type != EV_KEY) && (type != EV_SW))
		return;

	btn = button_get_index(data->b->code);
	if (btn < 0)
		return;

	button_hotplug_create_event(button_map[btn].name, type,
			(seen - priv->seen) / HZ, value);
	priv->seen = seen;
}

struct gpio_keys_button_dev {
	int polled;
	struct delayed_work work;

	struct device *dev;
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button_data data[0];
};

static int gpio_button_get_value(struct gpio_keys_button_data *bdata)
{
	int val;

	if (bdata->can_sleep)
		val = !!gpio_get_value_cansleep(bdata->b->gpio);
	else
		val = !!gpio_get_value(bdata->b->gpio);

	return val ^ bdata->b->active_low;
}

static void gpio_keys_polled_check_state(struct gpio_keys_button_data *bdata)
{
	int state = gpio_button_get_value(bdata);

	if (state != bdata->last_state) {
		unsigned int type = bdata->b->type ?: EV_KEY;

		if (bdata->count < bdata->threshold) {
			bdata->count++;
			return;
		}

		if ((bdata->last_state != -1) || (type == EV_SW))
			button_hotplug_event(bdata, type, state);

		bdata->last_state = state;
	}

	bdata->count = 0;
}

static void gpio_keys_polled_queue_work(struct gpio_keys_button_dev *bdev)
{
	struct gpio_keys_platform_data *pdata = bdev->pdata;
	unsigned long delay = msecs_to_jiffies(pdata->poll_interval);

	if (delay >= HZ)
		delay = round_jiffies_relative(delay);
	schedule_delayed_work(&bdev->work, delay);
}

static void gpio_keys_polled_poll(struct work_struct *work)
{
	struct gpio_keys_button_dev *bdev =
		container_of(work, struct gpio_keys_button_dev, work.work);
	int i;

	for (i = 0; i < bdev->pdata->nbuttons; i++) {
		struct gpio_keys_button_data *bdata = &bdev->data[i];
		gpio_keys_polled_check_state(bdata);
	}
	gpio_keys_polled_queue_work(bdev);
}

static void gpio_keys_polled_close(struct gpio_keys_button_dev *bdev)
{
	struct gpio_keys_platform_data *pdata = bdev->pdata;

	cancel_delayed_work_sync(&bdev->work);

	if (pdata->disable)
		pdata->disable(bdev->dev);
}

static irqreturn_t button_handle_irq(int irq, void *_bdata)
{
	struct gpio_keys_button_data *bdata = (struct gpio_keys_button_data *) _bdata;

	button_hotplug_event(bdata, bdata->b->type ?: EV_KEY, gpio_button_get_value(bdata));

	return IRQ_HANDLED;
}

#ifdef CONFIG_OF
static struct gpio_keys_platform_data *
gpio_keys_get_devtree_pdata(struct device *dev)
{
	struct device_node *node, *pp;
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button *button;
	int error;
	int nbuttons;
	int i = 0;

	node = dev->of_node;
	if (!node)
		return NULL;

	nbuttons = of_get_child_count(node);
	if (nbuttons == 0)
		return NULL;

	pdata = devm_kzalloc(dev, sizeof(*pdata) + nbuttons * (sizeof *button),
		GFP_KERNEL);
	if (!pdata) {
		error = -ENOMEM;
		goto err_out;
	}

	pdata->buttons = (struct gpio_keys_button *)(pdata + 1);
	pdata->nbuttons = nbuttons;

	pdata->rep = !!of_get_property(node, "autorepeat", NULL);
	of_property_read_u32(node, "poll-interval", &pdata->poll_interval);

	for_each_child_of_node(node, pp) {
		enum of_gpio_flags flags;

		if (!of_find_property(pp, "gpios", NULL)) {
			pdata->nbuttons--;
			dev_warn(dev, "Found button without gpios\n");
			continue;
		}

		button = &pdata->buttons[i++];

		button->gpio = of_get_gpio_flags(pp, 0, &flags);
		if (button->gpio < 0) {
			error = button->gpio;
			if (error != -ENOENT) {
				if (error != -EPROBE_DEFER)
					dev_err(dev,
						"Failed to get gpio flags, error: %d\n",
						error);
				return ERR_PTR(error);
			}
		} else {
			button->active_low = flags & OF_GPIO_ACTIVE_LOW;
		}

		if (of_property_read_u32(pp, "linux,code", &button->code)) {
			dev_err(dev, "Button without keycode: 0x%x\n",
				button->gpio);
			error = -EINVAL;
			goto err_out;
		}

		button->desc = of_get_property(pp, "label", NULL);

		if (of_property_read_u32(pp, "linux,input-type", &button->type))
			button->type = EV_KEY;

		button->wakeup = !!of_get_property(pp, "gpio-key,wakeup", NULL);

		if (of_property_read_u32(pp, "debounce-interval",
					&button->debounce_interval))
			button->debounce_interval = 5;
	}

	if (pdata->nbuttons == 0) {
		error = -EINVAL;
		goto err_out;
	}

	return pdata;

err_out:
	return ERR_PTR(error);
}

static struct of_device_id gpio_keys_of_match[] = {
	{ .compatible = "gpio-keys", },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_keys_of_match);

static struct of_device_id gpio_keys_polled_of_match[] = {
	{ .compatible = "gpio-keys-polled", },
	{ },
};
MODULE_DEVICE_TABLE(of, gpio_keys_polled_of_match);

#else

static inline struct gpio_keys_platform_data *
gpio_keys_get_devtree_pdata(struct device *dev)
{
	return NULL;
}
#endif

static int gpio_keys_button_probe(struct platform_device *pdev,
		struct gpio_keys_button_dev **_bdev, int polled)
{
	struct gpio_keys_platform_data *pdata = pdev->dev.platform_data;
	struct device *dev = &pdev->dev;
	struct gpio_keys_button_dev *bdev;
	struct gpio_keys_button *buttons;
	int error;
	int i;

	if (!pdata) {
		pdata = gpio_keys_get_devtree_pdata(dev);
		if (IS_ERR(pdata))
			return PTR_ERR(pdata);
		if (!pdata) {
			dev_err(dev, "missing platform data\n");
			return -EINVAL;
		}
	}

	if (polled && !pdata->poll_interval) {
		dev_err(dev, "missing poll_interval value\n");
		return -EINVAL;
	}

	buttons = devm_kzalloc(dev, pdata->nbuttons * sizeof(struct gpio_keys_button),
		       GFP_KERNEL);
	if (!buttons) {
		dev_err(dev, "no memory for button data\n");
		return -ENOMEM;
	}
	memcpy(buttons, pdata->buttons, pdata->nbuttons * sizeof(struct gpio_keys_button));

	bdev = devm_kzalloc(dev, sizeof(struct gpio_keys_button_dev) +
		       pdata->nbuttons * sizeof(struct gpio_keys_button_data),
		       GFP_KERNEL);
	if (!bdev) {
		dev_err(dev, "no memory for private data\n");
		return -ENOMEM;
	}

	bdev->polled = polled;

	for (i = 0; i < pdata->nbuttons; i++) {
		struct gpio_keys_button *button = &buttons[i];
		struct gpio_keys_button_data *bdata = &bdev->data[i];
		unsigned int gpio = button->gpio;

		if (button->wakeup) {
			dev_err(dev, DRV_NAME "does not support wakeup\n");
			return -EINVAL;
		}

		error = devm_gpio_request(dev, gpio,
				     button->desc ? button->desc : DRV_NAME);
		if (error) {
			dev_err(dev, "unable to claim gpio %u, err=%d\n",
				gpio, error);
			return error;
		}

		error = gpio_direction_input(gpio);
		if (error) {
			dev_err(dev,
				"unable to set direction on gpio %u, err=%d\n",
				gpio, error);
			return error;
		}

		bdata->can_sleep = gpio_cansleep(gpio);
		bdata->last_state = -1;

		if (bdev->polled)
			bdata->threshold = DIV_ROUND_UP(button->debounce_interval,
						pdata->poll_interval);
		else
			bdata->threshold = 1;

		bdata->b = &pdata->buttons[i];
	}

	bdev->dev = &pdev->dev;
	bdev->pdata = pdata;
	platform_set_drvdata(pdev, bdev);

	*_bdev = bdev;

	return 0;
}

static int gpio_keys_probe(struct platform_device *pdev)
{
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button_dev *bdev;
	int ret, i;


	ret = gpio_keys_button_probe(pdev, &bdev, 0);

	if (ret)
		return ret;

	pdata = bdev->pdata;
	for (i = 0; i < pdata->nbuttons; i++) {
		struct gpio_keys_button *button = &pdata->buttons[i];
		struct gpio_keys_button_data *bdata = &bdev->data[i];

		if (!button->irq)
			button->irq = gpio_to_irq(button->gpio);
		if (button->irq < 0) {
			dev_err(&pdev->dev, "failed to get irq for gpio:%d\n", button->gpio);
			continue;
		}

		ret = devm_request_threaded_irq(&pdev->dev, button->irq, NULL, button_handle_irq,
						IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
						dev_name(&pdev->dev), bdata);
		if (ret < 0)
			dev_err(&pdev->dev, "failed to request irq:%d for gpio:%d\n", button->irq, button->gpio);
		else
			dev_dbg(&pdev->dev, "gpio:%d has irq:%d\n", button->gpio, button->irq);

		if (bdata->b->type == EV_SW)
			button_hotplug_event(bdata, EV_SW, gpio_button_get_value(bdata));
	}

	return 0;
}

static int gpio_keys_polled_probe(struct platform_device *pdev)
{
	struct gpio_keys_platform_data *pdata;
	struct gpio_keys_button_dev *bdev;
	int ret;
	int i;

	ret = gpio_keys_button_probe(pdev, &bdev, 1);

	if (ret)
		return ret;

	INIT_DELAYED_WORK(&bdev->work, gpio_keys_polled_poll);

	pdata = bdev->pdata;

	if (pdata->enable)
		pdata->enable(bdev->dev);

	for (i = 0; i < pdata->nbuttons; i++)
		gpio_keys_polled_check_state(&bdev->data[i]);

	gpio_keys_polled_queue_work(bdev);

	return ret;
}

static int gpio_keys_remove(struct platform_device *pdev)
{
	struct gpio_keys_button_dev *bdev = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);

	if (bdev->polled)
		gpio_keys_polled_close(bdev);

	return 0;
}

static struct platform_driver gpio_keys_driver = {
	.probe	= gpio_keys_probe,
	.remove	= gpio_keys_remove,
	.driver	= {
		.name	= "gpio-keys",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_keys_of_match),
	},
};

static struct platform_driver gpio_keys_polled_driver = {
	.probe	= gpio_keys_polled_probe,
	.remove	= gpio_keys_remove,
	.driver	= {
		.name	= "gpio-keys-polled",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_keys_polled_of_match),
	},
};

static int __init gpio_button_init(void)
{
	int ret;

	ret = platform_driver_register(&gpio_keys_driver);
	if (ret)
		return ret;

	ret = platform_driver_register(&gpio_keys_polled_driver);
	if (ret)
		platform_driver_unregister(&gpio_keys_driver);

	return ret;
}

static void __exit gpio_button_exit(void)
{
	platform_driver_unregister(&gpio_keys_driver);
	platform_driver_unregister(&gpio_keys_polled_driver);
}

module_init(gpio_button_init);
module_exit(gpio_button_exit);

MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_DESCRIPTION("Polled GPIO Buttons hotplug driver");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" DRV_NAME);
