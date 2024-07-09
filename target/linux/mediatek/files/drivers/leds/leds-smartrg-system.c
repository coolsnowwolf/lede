// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/version.h>

/**
 * Driver for SmartRG RGBW LED microcontroller.
 * RGBW LED is connected to a Holtek HT45F0062 that is on the I2C bus.
 *
 */

struct srg_led_ctrl;
struct srg_led {
	u8 index;
	struct led_classdev led;
	struct srg_led_ctrl *ctrl;
};

struct srg_led_ctrl {
	struct mutex lock;
	struct i2c_client *client;
	struct srg_led channel[4];
	u8 control[5];
};

static int
srg_led_i2c_write(struct srg_led_ctrl *sysled_ctrl, u8 reg, u8 value)
{
	return i2c_smbus_write_byte_data(sysled_ctrl->client, reg, value);
}

/*
 * MC LED Command: 0 = OFF, 1 = ON, 2 = Flash, 3 = Pulse, 4 = Blink
 * */
static int
srg_led_control_sync(struct srg_led_ctrl *sysled_ctrl)
{
	int i, ret;

	for (i = 1; i < 5; i++) {
		ret = srg_led_i2c_write(sysled_ctrl, i, sysled_ctrl->control[i]);
		if (ret)
			break;
	}
	return ret;
}

/*
 * This function overrides the led driver timer trigger to offload
 * flashing to the micro-controller.  The negative effect of this
 * is the inability to configure the delay_on and delay_off periods.
 *
 * */
static int
srg_led_set_pulse(struct led_classdev *led_cdev,
		  unsigned long *delay_on,
		  unsigned long *delay_off)
{
	struct srg_led *sysled = container_of(led_cdev, struct srg_led, led);
	struct srg_led_ctrl *sysled_ctrl = sysled->ctrl;
	bool blinking = false, pulsing = false;
	u8 cbyte;
	int ret;

	if (delay_on && delay_off && (*delay_on > 100) && (*delay_on <= 500)) {
		pulsing = true;
		*delay_on = 500;
		*delay_off = 500;
	} else if (delay_on && delay_off && (*delay_on >= 50) && (*delay_on <= 100)) {
		blinking = true;
		*delay_on = 50;
		*delay_off = 50;
	}

	cbyte = pulsing ? 3 : blinking ? 2 : 0;
	mutex_lock(&sysled_ctrl->lock);
	ret = srg_led_i2c_write(sysled_ctrl, sysled->index + 4,
				(blinking || pulsing) ? 255 : 0);
	if (!ret) {
		sysled_ctrl->control[sysled->index] = cbyte;
		ret = srg_led_control_sync(sysled_ctrl);
	}
	mutex_unlock(&sysled_ctrl->lock);

	return !cbyte;
}

static int
srg_led_set_brightness(struct led_classdev *led_cdev,
			enum led_brightness value)
{
	struct srg_led *sysled = container_of(led_cdev, struct srg_led, led);
	struct srg_led_ctrl *sysled_ctrl = sysled->ctrl;
	int ret;

	mutex_lock(&sysled_ctrl->lock);
	ret = srg_led_i2c_write(sysled_ctrl, sysled->index + 4, value);
	if (!ret) {
		sysled_ctrl->control[sysled->index] = !!value;
		ret = srg_led_control_sync(sysled_ctrl);
	}
	mutex_unlock(&sysled_ctrl->lock);
	return ret;
}

static int
srg_led_init_led(struct srg_led_ctrl *sysled_ctrl, struct device_node *np)
{
	struct led_init_data init_data = {};
	struct led_classdev *led_cdev;
	struct srg_led *sysled;
	int index, ret;

	if (!np)
		return -ENOENT;

	ret = of_property_read_u32(np, "reg", &index);
	if (ret) {
		dev_err(&sysled_ctrl->client->dev,
			"srg_led_init_led: no reg defined in np!\n");
		return ret;
	}

	if (index < 1 || index > 4)
		return -EINVAL;

	sysled = &sysled_ctrl->channel[index - 1];
	led_cdev = &sysled->led;

	sysled->index = index;
	sysled->ctrl = sysled_ctrl;

	init_data.fwnode = of_fwnode_handle(np);

	led_cdev->name = of_get_property(np, "label", NULL) ? : np->name;
	led_cdev->brightness = LED_OFF;
	led_cdev->max_brightness = LED_FULL;
	led_cdev->brightness_set_blocking = srg_led_set_brightness;
	led_cdev->blink_set = srg_led_set_pulse;

	srg_led_i2c_write(sysled_ctrl, index + 4, 0);

	ret = devm_led_classdev_register_ext(&sysled_ctrl->client->dev,
						led_cdev, &init_data);
	if (ret) {
		dev_err(&sysled_ctrl->client->dev,
			"srg_led_init_led: led register %s error ret %d!n",
			led_cdev->name, ret);
		return ret;
	}

	return 0;
}

static int

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,6,0)
srg_led_probe(struct i2c_client *client, const struct i2c_device_id *id)
#else
srg_led_probe(struct i2c_client *client)
#endif
{
	struct device_node *np = client->dev.of_node, *child;
	struct srg_led_ctrl *sysled_ctrl;

	sysled_ctrl = devm_kzalloc(&client->dev, sizeof(*sysled_ctrl), GFP_KERNEL);
	if (!sysled_ctrl)
		return -ENOMEM;

	sysled_ctrl->client = client;

	mutex_init(&sysled_ctrl->lock);

	i2c_set_clientdata(client, sysled_ctrl);

	for_each_child_of_node(np, child) {
		if (srg_led_init_led(sysled_ctrl, child))
			continue;

		msleep(5);
	}

	return srg_led_control_sync(sysled_ctrl);;
}

static void srg_led_disable(struct i2c_client *client)
{
	struct srg_led_ctrl *sysled_ctrl = i2c_get_clientdata(client);
	int i;

	for (i = 1; i < 10; i++)
		srg_led_i2c_write(sysled_ctrl, i, 0);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,16,0)
static void
#else
static int
#endif
srg_led_remove(struct i2c_client *client)
{
	struct srg_led_ctrl *sysled_ctrl = i2c_get_clientdata(client);

	srg_led_disable(client);
	mutex_destroy(&sysled_ctrl->lock);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,16,0)
	return 0;
#endif
}

static const struct i2c_device_id srg_led_id[] = {
	{ "srg-sysled", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, srg_led_id);

static const struct of_device_id of_srg_led_match[] = {
	{ .compatible = "srg,sysled", },
	{},
};
MODULE_DEVICE_TABLE(of, of_srg_led_match);

static struct i2c_driver srg_sysled_driver = {
	.driver = {
		.name	= "srg-sysled",
		.of_match_table = of_srg_led_match,
	},
	.probe		= srg_led_probe,
	.remove		= srg_led_remove,
	.id_table	= srg_led_id,
};
module_i2c_driver(srg_sysled_driver);

MODULE_DESCRIPTION("SmartRG system LED driver");
MODULE_AUTHOR("Shen Loh <shen.loh@adtran.com>");
MODULE_AUTHOR("Daniel Golle <daniel@makrotopia.org>");
MODULE_LICENSE("GPL v2");
