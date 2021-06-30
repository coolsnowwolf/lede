// SPDX-License-Identifier: GPL-2.0-only

#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/gpio/consumer.h>

/**
 * Driver for the Ubiquiti RGB LED controller (LEDBAR).
 * This Controller is based on a Holtek HT32F52241 and connected
 * via I2C.
 *
 *  - The Controller needs an enable signal set to high when
 *    performing a transaction. On the U6-LR, this is located
 *    at Pin 18 (R6902)
 *
 *  - The Pin is also printed when calling the "usetled" function
 *    contained in the ubntapp bootloader application.
 */

#define UBNT_LEDBAR_MAX_BRIGHTNESS	0xff

#define UBNT_LEDBAR_TRANSACTION_LENGTH	8
#define UBNT_LEDBAR_TRANSACTION_SUCCESS	0xaa

#define UBNT_LEDBAR_TRANSACTION_BLUE_IDX	2
#define UBNT_LEDBAR_TRANSACTION_GREEN_IDX	3
#define UBNT_LEDBAR_TRANSACTION_RED_IDX		4

struct ubnt_ledbar {
	struct mutex lock;
	struct i2c_client *client;
	struct led_classdev led_red;
	struct led_classdev led_green;
	struct led_classdev led_blue;
	struct gpio_desc *enable_gpio;
};

static int ubnt_ledbar_perform_transaction(struct ubnt_ledbar *ledbar,
					   char *transaction)
{
	int ret;
	int i;

	for (i = 0; i < UBNT_LEDBAR_TRANSACTION_LENGTH; i++)
		i2c_smbus_write_byte(ledbar->client, transaction[i]);

	return i2c_smbus_read_byte(ledbar->client);
}

static int ubnt_ledbar_apply_state(struct ubnt_ledbar *ledbar)
{
	char setup_msg[UBNT_LEDBAR_TRANSACTION_LENGTH] = {0x40, 0x10, 0x00, 0x00,
							  0x00, 0x00, 0x00, 0x11};
	char led_msg[UBNT_LEDBAR_TRANSACTION_LENGTH] = {0x40, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x01, 0x00};
	char i2c_response;
	int ret = 0;

	mutex_lock(&ledbar->lock);

	led_msg[UBNT_LEDBAR_TRANSACTION_BLUE_IDX] = ledbar->led_blue.brightness;
	led_msg[UBNT_LEDBAR_TRANSACTION_GREEN_IDX] = ledbar->led_green.brightness;
	led_msg[UBNT_LEDBAR_TRANSACTION_RED_IDX] = ledbar->led_red.brightness;

	gpiod_set_raw_value(ledbar->enable_gpio, 1);

	msleep(10);

	i2c_response = ubnt_ledbar_perform_transaction(ledbar, setup_msg);
	if (i2c_response != UBNT_LEDBAR_TRANSACTION_SUCCESS) {
		dev_err(&ledbar->client->dev, "Error initializing LED transaction: %02x\n", ret);
		ret = -EINVAL;
		goto out_gpio;
	}

	i2c_response = ubnt_ledbar_perform_transaction(ledbar, led_msg);
	if (i2c_response != UBNT_LEDBAR_TRANSACTION_SUCCESS) {
		dev_err(&ledbar->client->dev, "Failed LED transaction: %02x\n", ret);
		ret = -EINVAL;
		goto out_gpio;
	}

	msleep(10);
out_gpio:
	gpiod_set_raw_value(ledbar->enable_gpio, 0);

	mutex_unlock(&ledbar->lock);

	return ret;
}

#define UBNT_LEDBAR_CONTROL_RGBS(name)				\
static int ubnt_ledbar_set_##name##_brightness(struct led_classdev *led_cdev,\
					enum led_brightness value)	\
{									\
	struct ubnt_ledbar *ledbar = \
			container_of(led_cdev, struct ubnt_ledbar, led_##name); \
	int ret; \
	led_cdev->brightness = value; \
	ret = ubnt_ledbar_apply_state(ledbar); \
	return ret; \
}

UBNT_LEDBAR_CONTROL_RGBS(red);
UBNT_LEDBAR_CONTROL_RGBS(green);
UBNT_LEDBAR_CONTROL_RGBS(blue);


static int ubnt_ledbar_init_led(struct device_node *np, struct ubnt_ledbar *ledbar,
				struct led_classdev *led_cdev)
{
	struct led_init_data init_data = {};
	int ret;

	if (!np)
		return 0;

	init_data.fwnode = of_fwnode_handle(np);

	led_cdev->max_brightness = UBNT_LEDBAR_MAX_BRIGHTNESS;

	ret = devm_led_classdev_register_ext(&ledbar->client->dev, led_cdev,
					     &init_data);
	if (ret)
		dev_err(&ledbar->client->dev, "led register err: %d\n", ret);

	return ret;
}


static int ubnt_ledbar_probe(struct i2c_client *client,
			     const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct ubnt_ledbar *ledbar;
	int ret;

	ledbar = devm_kzalloc(&client->dev, sizeof(*ledbar), GFP_KERNEL);
	if (!ledbar)
		return -ENOMEM;

	ledbar->enable_gpio = devm_gpiod_get(&client->dev, "enable", GPIOD_OUT_LOW);

	if (IS_ERR(ledbar->enable_gpio)) {
		ret = PTR_ERR(ledbar->enable_gpio);
		dev_err(&client->dev, "Failed to get enable gpio: %d\n", ret);
		return ret;
	}

	gpiod_direction_output(ledbar->enable_gpio, 0);

	ledbar->client = client;

	mutex_init(&ledbar->lock);

	i2c_set_clientdata(client, ledbar);

	ledbar->led_red.brightness_set_blocking = ubnt_ledbar_set_red_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "red"), ledbar, &ledbar->led_red);

	ledbar->led_green.brightness_set_blocking = ubnt_ledbar_set_green_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "green"), ledbar, &ledbar->led_green);

	ledbar->led_blue.brightness_set_blocking = ubnt_ledbar_set_blue_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "blue"), ledbar, &ledbar->led_blue);

	return ubnt_ledbar_apply_state(ledbar);
}

static int ubnt_ledbar_remove(struct i2c_client *client)
{
	struct ubnt_ledbar *ledbar = i2c_get_clientdata(client);

	mutex_destroy(&ledbar->lock);

	return 0;
}

static const struct i2c_device_id ubnt_ledbar_id[] = {
	{ "ubnt-ledbar", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, ubnt_ledbar_id);

static const struct of_device_id of_ubnt_ledbar_match[] = {
	{ .compatible = "ubnt,ledbar", },
	{},
};
MODULE_DEVICE_TABLE(of, of_ubnt_ledbar_match);

static struct i2c_driver ubnt_ledbar_driver = {
	.driver = {
		.name	= "ubnt-ledbar",
		.of_match_table = of_ubnt_ledbar_match,
	},
	.probe		= ubnt_ledbar_probe,
	.remove		= ubnt_ledbar_remove,
	.id_table	= ubnt_ledbar_id,
};
module_i2c_driver(ubnt_ledbar_driver);

MODULE_DESCRIPTION("Ubiquiti LEDBAR driver");
MODULE_AUTHOR("David Bauer <mail@david-bauer.net>");
MODULE_LICENSE("GPL v2");
