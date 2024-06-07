// SPDX-License-Identifier: GPL-2.0-only
/*
 * WorldSemi WS2812B individually-addressable LED driver using SPI
 *
 * Copyright 2022 Chuanhong Guo <gch981213@gmail.com>
 *
 * This driver simulates WS2812B protocol using SPI MOSI pin. A one pulse
 * is transferred as 3'b110 and a zero pulse is 3'b100. For this driver to
 * work properly, the SPI frequency should be 2.105MHz~2.85MHz and it needs
 * to transfer all the bytes continuously.
 */

#include <linux/led-class-multicolor.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/property.h>
#include <linux/spi/spi.h>
#include <linux/mutex.h>
#include <linux/version.h>

#define WS2812B_BYTES_PER_COLOR 3
#define WS2812B_NUM_COLORS 3
/* A continuous 0 for 50us+ as the 'reset' signal */
#define WS2812B_RESET_LEN 18

struct ws2812b_led {
	struct led_classdev_mc mc_cdev;
	struct mc_subled subled[WS2812B_NUM_COLORS];
	int cascade;
};

struct ws2812b_priv {
	struct led_classdev ldev;
	struct spi_device *spi;
	struct mutex mutex;
	int num_leds;
	size_t data_len;
	u8 *data_buf;
	struct ws2812b_led leds[];
};

/**
 * ws2812b_set_byte - convert a byte of data to 3-byte SPI data for pulses
 * @priv: pointer to the private data structure
 * @offset: offset of the target byte in the data stream
 * @val: 1-byte data to be set
 *
 * WS2812B receives a stream of bytes from DI, takes the first 3 byte as LED
 * brightness and pases the rest to the next LED through the DO pin.
 * This function assembles a single byte of data to the LED:
 * A bit is represented with a pulse of specific length. A long pulse is a 1
 * and a short pulse is a 0.
 * SPI transfers data continuously, MSB first. We can send 3'b100 to create a
 * 0 pulse and 3'b110 for a 1 pulse. In this way, a byte of data takes up 3
 * bytes in a SPI transfer:
 *  1x0 1x0 1x0 1x0 1x0 1x0 1x0 1x0
 * Let's rearrange it in 8 bits:
 *  1x01x01x 01x01x01 x01x01x0
 * The higher 3 bits, middle 2 bits and lower 3 bits are represented with the
 * 1st, 2nd and 3rd byte in the SPI transfer respectively.
 * There are only 8 combinations for 3 bits and 4 for 2 bits, so we can create
 * a lookup table for the 3 bytes.
 * e.g. For 0x6b -> 2'b01101011:
 *  Bit 7-5: 3'b011 -> 10011011 -> 0x9b
 *  Bit 4-3: 2'b01  -> 01001101 -> 0x4d
 *  Bit 2-0: 3'b011 -> 00110110 -> 0x36
 */
static void ws2812b_set_byte(struct ws2812b_priv *priv, size_t offset, u8 val)
{
	/* The lookup table for Bit 7-5 4-3 2-0 */
	const u8 h3b[] = { 0x92, 0x93, 0x9a, 0x9b, 0xd2, 0xd3, 0xda, 0xdb };
	const u8 m2b[] = { 0x49, 0x4d, 0x69, 0x6d };
	const u8 l3b[] = { 0x24, 0x26, 0x34, 0x36, 0xa4, 0xa6, 0xb4, 0xb6 };
	u8 *p = priv->data_buf + WS2812B_RESET_LEN + (offset * WS2812B_BYTES_PER_COLOR);

	p[0] = h3b[val >> 5]; /* Bit 7-5 */
	p[1] = m2b[(val >> 3) & 0x3]; /* Bit 4-3 */
	p[2] = l3b[val & 0x7]; /* Bit 2-0 */
}

static int ws2812b_set(struct led_classdev *cdev,
		       enum led_brightness brightness)
{
	struct led_classdev_mc *mc_cdev = lcdev_to_mccdev(cdev);
	struct ws2812b_led *led =
		container_of(mc_cdev, struct ws2812b_led, mc_cdev);
	struct ws2812b_priv *priv = dev_get_drvdata(cdev->dev->parent);
	int ret;
	int i;

	led_mc_calc_color_components(mc_cdev, brightness);

	mutex_lock(&priv->mutex);
	for (i = 0; i < WS2812B_NUM_COLORS; i++)
		ws2812b_set_byte(priv, led->cascade * WS2812B_NUM_COLORS + i,
				 led->subled[i].brightness);
	ret = spi_write(priv->spi, priv->data_buf, priv->data_len);
	mutex_unlock(&priv->mutex);

	return ret;
}

static int ws2812b_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	int cur_led = 0;
	struct ws2812b_priv *priv;
	struct fwnode_handle *led_node;
	int num_leds, i, cnt, ret;

	num_leds = device_get_child_node_count(dev);

	priv = devm_kzalloc(dev, struct_size(priv, leds, num_leds), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;
	priv->data_len =
		num_leds * WS2812B_BYTES_PER_COLOR * WS2812B_NUM_COLORS +
		WS2812B_RESET_LEN;
	priv->data_buf = kzalloc(priv->data_len, GFP_KERNEL);
	if (!priv->data_buf)
		return -ENOMEM;

	for (i = 0; i < num_leds * WS2812B_NUM_COLORS; i++)
		ws2812b_set_byte(priv, i, 0);

	mutex_init(&priv->mutex);
	priv->num_leds = num_leds;
	priv->spi = spi;

	device_for_each_child_node(dev, led_node) {
		struct led_init_data init_data = {
			.fwnode = led_node,
		};
		/* WS2812B LEDs usually come with GRB color */
		u32 color_idx[WS2812B_NUM_COLORS] = {
			LED_COLOR_ID_GREEN,
			LED_COLOR_ID_RED,
			LED_COLOR_ID_BLUE,
		};
		u32 cascade;

		ret = fwnode_property_read_u32(led_node, "reg", &cascade);
		if (ret) {
			dev_err(dev, "failed to obtain numerical LED index for %s",
				fwnode_get_name(led_node));
			goto ERR_UNREG_LEDS;
		}
		if (cascade >= num_leds) {
			dev_err(dev, "LED index of %s is larger than the number of LEDs.",
				fwnode_get_name(led_node));
			ret = -EINVAL;
			goto ERR_UNREG_LEDS;
		}

		cnt = fwnode_property_count_u32(led_node, "color-index");
		if (cnt > 0 && cnt <= WS2812B_NUM_COLORS)
			fwnode_property_read_u32_array(led_node, "color-index",
						       color_idx, (size_t)cnt);

		priv->leds[cur_led].mc_cdev.subled_info =
			priv->leds[cur_led].subled;
		priv->leds[cur_led].mc_cdev.num_colors = WS2812B_NUM_COLORS;
		priv->leds[cur_led].mc_cdev.led_cdev.max_brightness = 255;
		priv->leds[cur_led].mc_cdev.led_cdev.brightness_set_blocking = ws2812b_set;

		for (i = 0; i < WS2812B_NUM_COLORS; i++) {
			priv->leds[cur_led].subled[i].color_index = color_idx[i];
			priv->leds[cur_led].subled[i].intensity = 255;
		}

		priv->leds[cur_led].cascade = cascade;

		ret = led_classdev_multicolor_register_ext(
			dev, &priv->leds[cur_led].mc_cdev, &init_data);
		if (ret) {
			dev_err(dev, "registration of %s failed.",
				fwnode_get_name(led_node));
			goto ERR_UNREG_LEDS;
		}
		cur_led++;
	}

	spi_set_drvdata(spi, priv);

	return 0;
ERR_UNREG_LEDS:
	for (; cur_led >= 0; cur_led--)
		led_classdev_multicolor_unregister(&priv->leds[cur_led].mc_cdev);
	mutex_destroy(&priv->mutex);
	kfree(priv->data_buf);
	return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,18,0)
static void ws2812b_remove(struct spi_device *spi)
#else
static int ws2812b_remove(struct spi_device *spi)
#endif
{
	struct ws2812b_priv *priv = spi_get_drvdata(spi);
	int cur_led;

	for (cur_led = priv->num_leds - 1; cur_led >= 0; cur_led--)
		led_classdev_multicolor_unregister(&priv->leds[cur_led].mc_cdev);
	kfree(priv->data_buf);
	mutex_destroy(&priv->mutex);

#if LINUX_VERSION_CODE < KERNEL_VERSION(5,18,0)
	return 0;
#endif
}

static const struct spi_device_id ws2812b_spi_ids[] = {
	{ "ws2812b" },
	{},
};
MODULE_DEVICE_TABLE(spi, ws2812b_spi_ids);

static const struct of_device_id ws2812b_dt_ids[] = {
	{ .compatible = "worldsemi,ws2812b" },
	{},
};
MODULE_DEVICE_TABLE(of, ws2812b_dt_ids);

static struct spi_driver ws2812b_driver = {
	.probe		= ws2812b_probe,
	.remove		= ws2812b_remove,
	.id_table	= ws2812b_spi_ids,
	.driver = {
		.name		= KBUILD_MODNAME,
		.of_match_table	= ws2812b_dt_ids,
	},
};

module_spi_driver(ws2812b_driver);

MODULE_AUTHOR("Chuanhong Guo <gch981213@gmail.com>");
MODULE_DESCRIPTION("WS2812B LED driver using SPI");
MODULE_LICENSE("GPL");
