// SPDX-License-Identifier: GPL-2.0-only
/* IEI WT61P803 PUZZLE MCU LED Driver
 *
 * Copyright (C) 2020 Sartura Ltd.
 * Author: Luka Kovacic <luka.kovacic@sartura.hr>
 */

#include <linux/leds.h>
#include <linux/mfd/iei-wt61p803-puzzle.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/property.h>
#include <linux/slab.h>
#include <linux/workqueue.h>

#define IEI_LEDS_MAX		4

enum iei_wt61p803_puzzle_led_state {
	IEI_LED_OFF = 0x30,
	IEI_LED_ON = 0x31,
	IEI_LED_BLINK_5HZ = 0x32,
	IEI_LED_BLINK_1HZ = 0x33,
};

/**
 * struct iei_wt61p803_puzzle_led - MCU LED Driver
 * @cdev:		LED classdev
 * @mcu:		MCU struct pointer
 * @response_buffer	Global MCU response buffer
 * @lock:		General mutex lock to protect simultaneous R/W access to led_power_state
 * @led_power_state:	State of the front panel power LED
 */
struct iei_wt61p803_puzzle_led {
	struct led_classdev cdev;
	struct iei_wt61p803_puzzle *mcu;
	unsigned char response_buffer[IEI_WT61P803_PUZZLE_BUF_SIZE];
	struct mutex lock; /* mutex to protect led_power_state */
	struct work_struct work;
	int led_power_state;
	int id;
	u8 blinking;
	bool active_low;
};

static inline struct iei_wt61p803_puzzle_led *cdev_to_iei_wt61p803_puzzle_led
	(struct led_classdev *led_cdev)
{
	return container_of(led_cdev, struct iei_wt61p803_puzzle_led, cdev);
}

static int iei_wt61p803_puzzle_led_brightness_set_blocking(struct led_classdev *cdev,
							   enum led_brightness brightness)
{
	struct iei_wt61p803_puzzle_led *priv = cdev_to_iei_wt61p803_puzzle_led(cdev);
	unsigned char *resp_buf = priv->response_buffer;
	unsigned char led_power_cmd[5] = {};
	size_t reply_size;
	int ret;

	if (priv->blinking) {
		if (brightness == LED_OFF)
			priv->blinking = 0;
		else
			return 0;
	}

	led_power_cmd[0] = IEI_WT61P803_PUZZLE_CMD_HEADER_START;
	led_power_cmd[1] = IEI_WT61P803_PUZZLE_CMD_LED;
	led_power_cmd[2] = IEI_WT61P803_PUZZLE_CMD_LED_SET(priv->id);
	led_power_cmd[3] = ((brightness == LED_OFF) ^ priv->active_low) ?
				IEI_LED_OFF : priv->blinking?priv->blinking:IEI_LED_ON;

	ret = iei_wt61p803_puzzle_write_command(priv->mcu, led_power_cmd,
						sizeof(led_power_cmd),
						resp_buf,
						&reply_size);
	if (ret)
		return ret;

	if (reply_size != 3)
		return -EIO;

	if (!(resp_buf[0] == IEI_WT61P803_PUZZLE_CMD_HEADER_START &&
	      resp_buf[1] == IEI_WT61P803_PUZZLE_CMD_RESPONSE_OK &&
	      resp_buf[2] == IEI_WT61P803_PUZZLE_CHECKSUM_RESPONSE_OK))
		return -EIO;

	mutex_lock(&priv->lock);
	priv->led_power_state = brightness;
	mutex_unlock(&priv->lock);

	return 0;
}

static enum led_brightness iei_wt61p803_puzzle_led_brightness_get(struct led_classdev *cdev)
{
	struct iei_wt61p803_puzzle_led *priv = cdev_to_iei_wt61p803_puzzle_led(cdev);
	int led_state;

	mutex_lock(&priv->lock);
	led_state = priv->led_power_state;
	mutex_unlock(&priv->lock);

	return led_state;
}

static void iei_wt61p803_puzzle_led_apply_blink(struct work_struct *work)
{
	struct iei_wt61p803_puzzle_led *priv = container_of(work, struct iei_wt61p803_puzzle_led, work);
	unsigned char led_blink_cmd[5] = {};
	unsigned char resp_buf[IEI_WT61P803_PUZZLE_BUF_SIZE];
	size_t reply_size;

	led_blink_cmd[0] = IEI_WT61P803_PUZZLE_CMD_HEADER_START;
	led_blink_cmd[1] = IEI_WT61P803_PUZZLE_CMD_LED;
	led_blink_cmd[2] = IEI_WT61P803_PUZZLE_CMD_LED_SET(priv->id);
	led_blink_cmd[3] = priv->blinking;

	iei_wt61p803_puzzle_write_command(priv->mcu, led_blink_cmd,
					  sizeof(led_blink_cmd),
					  resp_buf,
					  &reply_size);

	return;
}

static int iei_wt61p803_puzzle_led_set_blink(struct led_classdev *cdev,
					     unsigned long *delay_on,
					     unsigned long *delay_off)
{
	struct iei_wt61p803_puzzle_led *priv = cdev_to_iei_wt61p803_puzzle_led(cdev);
	u8 blink_mode = 0;
	int ret = 0;

	/* set defaults */
	if (!*delay_on && !*delay_off) {
		*delay_on = 500;
		*delay_off = 500;
	}

	/* minimum delay for soft-driven blinking is 100ms to keep load low */
	if (*delay_on < 100)
		*delay_on = 100;

	if (*delay_off < 100)
		*delay_off = 100;

	/* offload blinking to hardware, if possible */
	if (*delay_on != *delay_off) {
		ret = -EINVAL;
	} else if (*delay_on == 100) {
		blink_mode = IEI_LED_BLINK_5HZ;
		*delay_on = 100;
		*delay_off = 100;
	} else if (*delay_on <= 500) {
		blink_mode = IEI_LED_BLINK_1HZ;
		*delay_on = 500;
		*delay_off = 500;
	} else {
		ret = -EINVAL;
	}

	mutex_lock(&priv->lock);
	priv->blinking = blink_mode;
	mutex_unlock(&priv->lock);

	if (blink_mode)
		schedule_work(&priv->work);

	return ret;
}


static int iei_wt61p803_puzzle_led_set_dt_default(struct led_classdev *cdev,
				     struct fwnode_handle *child)
{
	const char *state;
	int ret = 0;

	if (!fwnode_property_read_string(child, "default-state", &state)) {
		if (!strcmp(state, "on")) {
			ret = iei_wt61p803_puzzle_led_brightness_set_blocking(
				cdev, cdev->max_brightness);
		} else  {
			ret = iei_wt61p803_puzzle_led_brightness_set_blocking(
				cdev, LED_OFF);
		}
	}

	return ret;
}

static int iei_wt61p803_puzzle_led_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct iei_wt61p803_puzzle *mcu = dev_get_drvdata(dev->parent);
	struct iei_wt61p803_puzzle_led *priv;
	int ret;
	u32 reg;

	if (device_get_child_node_count(dev) > IEI_LEDS_MAX)
		return -EINVAL;

	device_for_each_child_node_scoped(dev, child) {
		struct led_init_data init_data = {};

		ret = fwnode_property_read_u32(child, "reg", &reg);
		if (ret)
			return dev_err_probe(dev, ret, "Failed to read led 'reg' property\n");

		if (reg > IEI_LEDS_MAX) {
			dev_err(dev, "Invalid led reg %u\n", reg);
			return -EINVAL;
		}

		priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
		if (!priv)
			return -ENOMEM;

		ret = devm_mutex_init(dev, &priv->lock);
		if (ret)
			return ret;

		dev_set_drvdata(dev, priv);

		priv->active_low = fwnode_property_present(child, "active-low");
		priv->mcu = mcu;
		priv->id = reg;
		priv->led_power_state = 1;
		priv->blinking = 0;
		init_data.fwnode = fwnode_handle_get(child);

		priv->cdev.brightness_set_blocking = iei_wt61p803_puzzle_led_brightness_set_blocking;
		priv->cdev.brightness_get = iei_wt61p803_puzzle_led_brightness_get;
		priv->cdev.blink_set = iei_wt61p803_puzzle_led_set_blink;

		priv->cdev.max_brightness = 1;

		INIT_WORK(&priv->work, iei_wt61p803_puzzle_led_apply_blink);

		ret = iei_wt61p803_puzzle_led_set_dt_default(&priv->cdev, child);
		if (ret)
			return dev_err_probe(dev, ret, "Could apply default from DT\n");

		ret = devm_led_classdev_register_ext(dev, &priv->cdev, &init_data);
		if (ret)
			return dev_err_probe(dev, ret, "Could not register LED\n");
	}

	return ret;
}

static const struct of_device_id iei_wt61p803_puzzle_led_of_match[] = {
	{ .compatible = "iei,wt61p803-puzzle-leds" },
	{ }
};
MODULE_DEVICE_TABLE(of, iei_wt61p803_puzzle_led_of_match);

static struct platform_driver iei_wt61p803_puzzle_led_driver = {
	.driver = {
		.name = "iei-wt61p803-puzzle-led",
		.of_match_table = iei_wt61p803_puzzle_led_of_match,
	},
	.probe = iei_wt61p803_puzzle_led_probe,
};
module_platform_driver(iei_wt61p803_puzzle_led_driver);

MODULE_DESCRIPTION("IEI WT61P803 PUZZLE front panel LED driver");
MODULE_AUTHOR("Luka Kovacic <luka.kovacic@sartura.hr>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:leds-iei-wt61p803-puzzle");
