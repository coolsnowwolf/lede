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
#define UBNT_LEDBAR_TRANSACTION_SUCCESS	(char) 0xaa

#define UBNT_LEDBAR_TRANSACTION_BLUE_IDX	2
#define UBNT_LEDBAR_TRANSACTION_GREEN_IDX	3
#define UBNT_LEDBAR_TRANSACTION_RED_IDX		4
#define UBNT_LEDBAR_TRANSACTION_LED_COUNT_IDX	6

struct ubnt_ledbar {
	struct mutex lock;
	u32 led_count;
	struct i2c_client *client;
	struct led_classdev led_red;
	struct led_classdev led_green;
	struct led_classdev led_blue;
	struct gpio_desc *enable_gpio;
	struct gpio_desc *reset_gpio;
};

static void ubnt_ledbar_perform_transaction(struct ubnt_ledbar *ledbar,
					   const char *transaction, int len,
					   char *result, int result_len)
{
	int i;

	for (i = 0; i < len; i++)
		i2c_smbus_write_byte(ledbar->client, transaction[i]);

	for (i = 0; i < result_len; i++)
		result[i] = i2c_smbus_read_byte(ledbar->client);
}

static int ubnt_ledbar_apply_state(struct ubnt_ledbar *ledbar)
{
	char setup_msg[UBNT_LEDBAR_TRANSACTION_LENGTH] = {0x40, 0x10, 0x00, 0x00,
							  0x00, 0x00, 0x00, 0x11};
	char led_msg[UBNT_LEDBAR_TRANSACTION_LENGTH] = {0x40, 0x00, 0x00, 0x00,
							0x00, 0x00, 0x00, 0x00};
	char i2c_response;
	int ret = 0;

	mutex_lock(&ledbar->lock);

	led_msg[UBNT_LEDBAR_TRANSACTION_BLUE_IDX] = ledbar->led_blue.brightness;
	led_msg[UBNT_LEDBAR_TRANSACTION_GREEN_IDX] = ledbar->led_green.brightness;
	led_msg[UBNT_LEDBAR_TRANSACTION_RED_IDX] = ledbar->led_red.brightness;
	led_msg[UBNT_LEDBAR_TRANSACTION_LED_COUNT_IDX] = ledbar->led_count;

	gpiod_set_value(ledbar->enable_gpio, 1);

	msleep(10);

	ubnt_ledbar_perform_transaction(ledbar, setup_msg, sizeof(setup_msg), &i2c_response, sizeof(i2c_response));
	if (i2c_response != UBNT_LEDBAR_TRANSACTION_SUCCESS) {
		dev_err(&ledbar->client->dev, "Error initializing LED transaction: %02hhx\n", i2c_response);
		ret = -EINVAL;
		goto out_gpio;
	}

	ubnt_ledbar_perform_transaction(ledbar, led_msg, sizeof(led_msg), &i2c_response, sizeof(i2c_response));
	if (i2c_response != UBNT_LEDBAR_TRANSACTION_SUCCESS) {
		dev_err(&ledbar->client->dev, "Failed LED transaction: %02hhx\n", i2c_response);
		ret = -EINVAL;
		goto out_gpio;
	}

	msleep(10);
out_gpio:
	gpiod_set_value(ledbar->enable_gpio, 0);

	mutex_unlock(&ledbar->lock);

	return ret;
}

static void ubnt_ledbar_reset(struct ubnt_ledbar *ledbar)
{
	static const char init_msg[16] = {0x02, 0x81, 0xfd, 0x7e,
					  0x00, 0x00, 0x00, 0x00,
					  0x00, 0x00, 0x00, 0x00,
					  0x00, 0x00, 0x00, 0x00};
	char init_response[4];

	if (!ledbar->reset_gpio)
		return;

	mutex_lock(&ledbar->lock);

	gpiod_set_value(ledbar->reset_gpio, 1);
	msleep(10);
	gpiod_set_value(ledbar->reset_gpio, 0);

	msleep(10);

	gpiod_set_value(ledbar->enable_gpio, 1);
	msleep(10);
	ubnt_ledbar_perform_transaction(ledbar, init_msg, sizeof(init_msg), init_response, sizeof(init_response));
	msleep(10);
	gpiod_set_value(ledbar->enable_gpio, 0);

	mutex_unlock(&ledbar->lock);
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

	if (!np)
		return 0;

	init_data.fwnode = of_fwnode_handle(np);

	led_cdev->max_brightness = UBNT_LEDBAR_MAX_BRIGHTNESS;

	return devm_led_classdev_register_ext(&ledbar->client->dev, led_cdev, &init_data);
}

static int ubnt_ledbar_probe(struct i2c_client *client)
{
	struct device_node *np = client->dev.of_node;
	struct ubnt_ledbar *ledbar;
	int err;

	ledbar = devm_kzalloc(&client->dev, sizeof(*ledbar), GFP_KERNEL);
	if (!ledbar)
		return -ENOMEM;

	ledbar->enable_gpio = devm_gpiod_get(&client->dev, "enable", GPIOD_OUT_LOW);

	if (IS_ERR(ledbar->enable_gpio))
		return dev_err_probe(&client->dev, PTR_ERR(ledbar->enable_gpio), "Failed to get enable gpio");

	ledbar->reset_gpio = devm_gpiod_get_optional(&client->dev, "reset", GPIOD_OUT_LOW);

	if (IS_ERR(ledbar->reset_gpio))
		return dev_err_probe(&client->dev, PTR_ERR(ledbar->reset_gpio), "Failed to get reset gpio");

	ledbar->led_count = 1;
	of_property_read_u32(np, "led-count", &ledbar->led_count);

	ledbar->client = client;

	err = devm_mutex_init(&client->dev, &ledbar->lock);
	if (err)
		return err;

	i2c_set_clientdata(client, ledbar);

	// Reset and initialize the MCU
	ubnt_ledbar_reset(ledbar);

	ledbar->led_red.brightness_set_blocking = ubnt_ledbar_set_red_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "red"), ledbar, &ledbar->led_red);

	ledbar->led_green.brightness_set_blocking = ubnt_ledbar_set_green_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "green"), ledbar, &ledbar->led_green);

	ledbar->led_blue.brightness_set_blocking = ubnt_ledbar_set_blue_brightness;
	ubnt_ledbar_init_led(of_get_child_by_name(np, "blue"), ledbar, &ledbar->led_blue);

	return ubnt_ledbar_apply_state(ledbar);
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
	.id_table	= ubnt_ledbar_id,
};
module_i2c_driver(ubnt_ledbar_driver);

MODULE_DESCRIPTION("Ubiquiti LEDBAR driver");
MODULE_AUTHOR("David Bauer <mail@david-bauer.net>");
MODULE_LICENSE("GPL v2");
