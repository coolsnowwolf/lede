// SPDX-License-Identifier: GPL-2.0
// Copyright (c) 2022 Furong Xu <xfr@outlook.com>

#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/spi/spi.h>
#include <linux/mutex.h>
#include <uapi/linux/uleds.h>

#define LED_COLOR_MAX 5

static unsigned char *led_color_table[LED_COLOR_MAX + 1] = {
	"\x92\x49\x24\x92\x49\x24\x92\x49\x24", /* LED OFF */
	"\xdb\x6d\x36\x92\x49\x24\x92\x49\x24", /* LED RED */
	"\x92\x49\x24\x92\x49\x24\xdb\x6d\x36", /* LED BLUE */
	"\xdb\x6d\x36\x9a\x49\x34\x92\x49\x24", /* LED YELLOW */
	"\xda\x4d\x36\xdb\x6d\x36\xda\x49\x24", /* LED WHITE */
	"\x92\x49\x24\xdb\x6d\x36\x92\x49\x24", /* LED GREEN */
};

struct spi_single_wire_chipdef {
	unsigned char chip_num;
	unsigned char led_ctrl_cmd_len;
};

struct spi_single_wire_led {
	struct led_classdev ldev;
	struct spi_device *spi;
	char name[LED_MAX_NAME_SIZE];
	struct mutex mutex;
	const struct spi_single_wire_chipdef *cdef;
	unsigned char *led_ctrl_cmd;
	unsigned char chip_id;
	unsigned char color_id;
};

static struct spi_single_wire_chipdef miwifi_hm0807a_led = {
	.chip_num = 2, /* Redmi AX6000 has 2 LED chips */
	.led_ctrl_cmd_len = 9, /* 9 bytes per LED chip */
};

static const struct spi_device_id spi_ids[] = {
	{ .name = "HM0807A" },
	{},
};

static const struct of_device_id spi_single_wire_dt_ids[] = {
	{ .compatible = "xiaomi,HM0807A", .data = &miwifi_hm0807a_led },
	{},
};

MODULE_DEVICE_TABLE(of, spi_single_wire_dt_ids);

static int spi_brightness_set_blocking(struct led_classdev *dev,
					    enum led_brightness brightness)
{
	struct spi_single_wire_led *led = container_of(dev, struct spi_single_wire_led, ldev);
	unsigned char *ctrl_cmd_head;
	unsigned char *ctrl_cmd_payload;
	unsigned char ctrl_cmd_len = led->cdef->chip_num * led->cdef->led_ctrl_cmd_len;
	int ret = 0;

	ctrl_cmd_head = kzalloc(ctrl_cmd_len + 2, GFP_KERNEL); /* 2 bytes pad is required */
	if (!ctrl_cmd_head)
		return -ENOMEM;

	mutex_lock(&led->mutex);
	ctrl_cmd_payload = ctrl_cmd_head + 1; /* skip the SOF byte */

	if (brightness == LED_OFF) {
		memcpy(led->led_ctrl_cmd, led_color_table[0], led->cdef->led_ctrl_cmd_len);
	} else {
		memcpy(led->led_ctrl_cmd, led_color_table[led->color_id + 1], led->cdef->led_ctrl_cmd_len);
	}

	memcpy(ctrl_cmd_payload, led->led_ctrl_cmd - led->cdef->led_ctrl_cmd_len * led->chip_id,
		led->cdef->led_ctrl_cmd_len * led->cdef->chip_num);

	ret = spi_write(led->spi, ctrl_cmd_head, ctrl_cmd_len + 2); /* 2 bytes pad */
	mutex_unlock(&led->mutex);

	kfree(ctrl_cmd_head);

	return ret;
}

static int spi_single_wire_probe(struct spi_device *spi)
{
	struct spi_single_wire_led *led;
	const struct spi_single_wire_chipdef *cdef;
	struct device *dev = &spi->dev;
	struct device_node *np = dev_of_node(dev);
	struct device_node *chip_node;
	struct device_node *color_node;
	const char *color_name = "";
	unsigned char *chip_cmd_buffer;
	const char *state;
	unsigned char cur_chip_id = 0;
	unsigned char cur_color_id = 0;
	int ret;
	int chip_count;

	cdef = device_get_match_data(dev);
	if (!cdef)
		return -ENODEV;

	chip_count = of_get_available_child_count(np);
	dev_info(dev, "Device has %d LED chip(s)\n", chip_count);
	if (chip_count == 0 || chip_count != cdef->chip_num)
		return -ENODEV;

	chip_cmd_buffer = devm_kzalloc(dev, cdef->led_ctrl_cmd_len * cdef->chip_num, GFP_KERNEL);
	if (!chip_cmd_buffer)
		return -ENOMEM;

	for_each_available_child_of_node(np, chip_node) {
		for_each_available_child_of_node(chip_node, color_node) {
			led = devm_kzalloc(dev, sizeof(*led), GFP_KERNEL);
			if (!led)
				return -ENOMEM;

			/* all LEDs of one chip share one cmd_buffer */
			led->led_ctrl_cmd = chip_cmd_buffer + cdef->led_ctrl_cmd_len * cur_chip_id;

			of_property_read_string(color_node, "label", &color_name);
			snprintf(led->name, sizeof(led->name), "%s", color_name);
			led->spi = spi;
			mutex_init(&led->mutex);
			led->cdef = cdef;
			led->chip_id = cur_chip_id;
			led->color_id = cur_color_id;
			led->ldev.name = led->name;
			led->ldev.brightness = LED_OFF;
			led->ldev.max_brightness = LED_FULL;
			led->ldev.brightness_set_blocking = spi_brightness_set_blocking;

			state = of_get_property(color_node, "default-state", NULL);
			if (state) {
				if (!strcmp(state, "on")) {
					led->ldev.brightness = led->ldev.max_brightness;
				}
				/* default to LED_OFF already */
			}
			spi_brightness_set_blocking(&led->ldev, led->ldev.brightness);

			ret = devm_led_classdev_register(&spi->dev, &led->ldev);
			if (ret) {
				mutex_destroy(&led->mutex);
				return ret;
			}
			spi_set_drvdata(spi, led);
			cur_color_id++;
		}
		cur_chip_id++;
		cur_color_id = 0;
	}

	return 0;
}

static int spi_single_wire_remove(struct spi_device *spi)
{
	/* module should never be removed from kernel */
	return 0;
}

static struct spi_driver spi_single_wire_led_driver = {
	.probe		= spi_single_wire_probe,
	.remove		= spi_single_wire_remove,
	.driver = {
		.name		= KBUILD_MODNAME,
		.of_match_table	= spi_single_wire_dt_ids,
	},
	.id_table = spi_ids,
};

module_spi_driver(spi_single_wire_led_driver);

MODULE_AUTHOR("Furong Xu <xfr@outlook.com>");
MODULE_DESCRIPTION("single wire SPI LED driver for Xiaomi Redmi AX6000");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("spi:leds-spi-single-wire");
