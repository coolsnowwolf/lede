/*
 * SPI driver for the CPLD chip on the Mikrotik RB4xx boards
 *
 * Copyright (C) 2010 Gabor Juhos <juhosg@openwrt.org>
 *
 * This file was based on the patches for Linux 2.6.27.39 published by
 * MikroTik for their RouterBoard 4xx series devices.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/bitops.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/slab.h>

#include <asm/mach-ath79/rb4xx_cpld.h>

#define DRV_NAME	"spi-rb4xx-cpld"
#define DRV_DESC	"RB4xx CPLD driver"
#define DRV_VERSION	"0.1.0"

#define CPLD_CMD_WRITE_NAND	0x08 /* send cmd, n x send data, send indle */
#define CPLD_CMD_WRITE_CFG	0x09 /* send cmd, n x send cfg */
#define CPLD_CMD_READ_NAND	0x0a /* send cmd, send idle, n x read data */
#define CPLD_CMD_READ_FAST	0x0b /* send cmd, 4 x idle, n x read data */
#define CPLD_CMD_LED5_ON	0x0c /* send cmd */
#define CPLD_CMD_LED5_OFF	0x0d /* send cmd */

struct rb4xx_cpld {
	struct spi_device	*spi;
	struct mutex		lock;
	struct gpio_chip	chip;
	unsigned int		config;
};

static struct rb4xx_cpld *rb4xx_cpld;

static inline struct rb4xx_cpld *gpio_to_cpld(struct gpio_chip *chip)
{
	return container_of(chip, struct rb4xx_cpld, chip);
}

static int rb4xx_cpld_write_cmd(struct rb4xx_cpld *cpld, unsigned char cmd)
{
	struct spi_transfer t[1];
	struct spi_message m;
	unsigned char tx_buf[1];
	int err;

	spi_message_init(&m);
	memset(&t, 0, sizeof(t));

	t[0].tx_buf = tx_buf;
	t[0].len = sizeof(tx_buf);
	spi_message_add_tail(&t[0], &m);

	tx_buf[0] = cmd;

	err = spi_sync(cpld->spi, &m);
	return err;
}

static int rb4xx_cpld_write_cfg(struct rb4xx_cpld *cpld, unsigned char config)
{
	struct spi_transfer t[1];
	struct spi_message m;
	unsigned char cmd[2];
	int err;

	spi_message_init(&m);
	memset(&t, 0, sizeof(t));

	t[0].tx_buf = cmd;
	t[0].len = sizeof(cmd);
	spi_message_add_tail(&t[0], &m);

	cmd[0] = CPLD_CMD_WRITE_CFG;
	cmd[1] = config;

	err = spi_sync(cpld->spi, &m);
	return err;
}

static int __rb4xx_cpld_change_cfg(struct rb4xx_cpld *cpld, unsigned mask,
				   unsigned value)
{
	unsigned int config;
	int err;

	config = cpld->config & ~mask;
	config |= value;

	if ((cpld->config ^ config) & 0xff) {
		err = rb4xx_cpld_write_cfg(cpld, config);
		if (err)
			return err;
	}

	if ((cpld->config ^ config) & CPLD_CFG_nLED5) {
		err = rb4xx_cpld_write_cmd(cpld, (value) ? CPLD_CMD_LED5_ON :
							   CPLD_CMD_LED5_OFF);
		if (err)
			return err;
	}

	cpld->config = config;
	return 0;
}

int rb4xx_cpld_change_cfg(unsigned mask, unsigned value)
{
	int ret;

	if (rb4xx_cpld == NULL)
		return -ENODEV;

	mutex_lock(&rb4xx_cpld->lock);
	ret = __rb4xx_cpld_change_cfg(rb4xx_cpld, mask, value);
	mutex_unlock(&rb4xx_cpld->lock);

	return ret;
}
EXPORT_SYMBOL_GPL(rb4xx_cpld_change_cfg);

int rb4xx_cpld_read(unsigned char *rx_buf, unsigned count)
{
	static const unsigned char cmd[2] = { CPLD_CMD_READ_NAND, 0 };
	struct spi_transfer t[2] = {
		{
			.tx_buf = &cmd,
			.len = 2,
		}, {
			.rx_buf = rx_buf,
			.len = count,
		},
	};
	struct spi_message m;

	if (rb4xx_cpld == NULL)
		return -ENODEV;

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	return spi_sync(rb4xx_cpld->spi, &m);
}
EXPORT_SYMBOL_GPL(rb4xx_cpld_read);

int rb4xx_cpld_write(const unsigned char *buf, unsigned count)
{
	static const unsigned char cmd = CPLD_CMD_WRITE_NAND;
	struct spi_transfer t[3] = {
		{
			.tx_buf = &cmd,
			.len = 1,
		}, {
			.tx_buf = buf,
			.len = count,
			.tx_nbits = SPI_NBITS_DUAL,
		}, {
			.len = 1,
			.tx_nbits = SPI_NBITS_DUAL,
		},
	};
	struct spi_message m;

	if (rb4xx_cpld == NULL)
		return -ENODEV;

	spi_message_init(&m);
	spi_message_add_tail(&t[0], &m);
	spi_message_add_tail(&t[1], &m);
	spi_message_add_tail(&t[2], &m);
	return spi_sync(rb4xx_cpld->spi, &m);
}
EXPORT_SYMBOL_GPL(rb4xx_cpld_write);

static int rb4xx_cpld_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	struct rb4xx_cpld *cpld = gpio_to_cpld(chip);
	int ret;

	mutex_lock(&cpld->lock);
	ret = (cpld->config >> offset) & 1;
	mutex_unlock(&cpld->lock);

	return ret;
}

static void rb4xx_cpld_gpio_set(struct gpio_chip *chip, unsigned offset,
				int value)
{
	struct rb4xx_cpld *cpld = gpio_to_cpld(chip);

	mutex_lock(&cpld->lock);
	__rb4xx_cpld_change_cfg(cpld, (1 << offset), !!value << offset);
	mutex_unlock(&cpld->lock);
}

static int rb4xx_cpld_gpio_direction_input(struct gpio_chip *chip,
					   unsigned offset)
{
	return -EOPNOTSUPP;
}

static int rb4xx_cpld_gpio_direction_output(struct gpio_chip *chip,
					    unsigned offset,
					    int value)
{
	struct rb4xx_cpld *cpld = gpio_to_cpld(chip);
	int ret;

	mutex_lock(&cpld->lock);
	ret = __rb4xx_cpld_change_cfg(cpld, (1 << offset), !!value << offset);
	mutex_unlock(&cpld->lock);

	return ret;
}

static int rb4xx_cpld_gpio_init(struct rb4xx_cpld *cpld, unsigned int base)
{
	int err;

	/* init config */
	cpld->config = CPLD_CFG_nLED1 | CPLD_CFG_nLED2 | CPLD_CFG_nLED3 |
		       CPLD_CFG_nLED4 | CPLD_CFG_nCE;
	rb4xx_cpld_write_cfg(cpld, cpld->config);

	/* setup GPIO chip */
	cpld->chip.label = DRV_NAME;

	cpld->chip.get = rb4xx_cpld_gpio_get;
	cpld->chip.set = rb4xx_cpld_gpio_set;
	cpld->chip.direction_input = rb4xx_cpld_gpio_direction_input;
	cpld->chip.direction_output = rb4xx_cpld_gpio_direction_output;

	cpld->chip.base = base;
	cpld->chip.ngpio = CPLD_NUM_GPIOS;
	cpld->chip.can_sleep = 1;
	cpld->chip.dev = &cpld->spi->dev;
	cpld->chip.owner = THIS_MODULE;

	err = gpiochip_add(&cpld->chip);
	if (err)
		dev_err(&cpld->spi->dev, "adding GPIO chip failed, err=%d\n",
			err);

	return err;
}

static int rb4xx_cpld_probe(struct spi_device *spi)
{
	struct rb4xx_cpld *cpld;
	struct rb4xx_cpld_platform_data *pdata;
	int err;

	pdata = spi->dev.platform_data;
	if (!pdata) {
		dev_dbg(&spi->dev, "no platform data\n");
		return -EINVAL;
	}

	cpld = kzalloc(sizeof(*cpld), GFP_KERNEL);
	if (!cpld) {
		dev_err(&spi->dev, "no memory for private data\n");
		return -ENOMEM;
	}

	mutex_init(&cpld->lock);
	cpld->spi = spi_dev_get(spi);
	dev_set_drvdata(&spi->dev, cpld);

	spi->mode = SPI_MODE_0 | SPI_TX_DUAL;
	spi->bits_per_word = 8;
	err = spi_setup(spi);
	if (err) {
		dev_err(&spi->dev, "spi_setup failed, err=%d\n", err);
		goto err_drvdata;
	}

	err = rb4xx_cpld_gpio_init(cpld, pdata->gpio_base);
	if (err)
		goto err_drvdata;

	rb4xx_cpld = cpld;

	return 0;

err_drvdata:
	dev_set_drvdata(&spi->dev, NULL);
	kfree(cpld);

	return err;
}

static int rb4xx_cpld_remove(struct spi_device *spi)
{
	struct rb4xx_cpld *cpld;

	rb4xx_cpld = NULL;
	cpld = dev_get_drvdata(&spi->dev);
	dev_set_drvdata(&spi->dev, NULL);
	kfree(cpld);

	return 0;
}

static struct spi_driver rb4xx_cpld_driver = {
	.driver = {
		.name		= DRV_NAME,
		.bus		= &spi_bus_type,
		.owner		= THIS_MODULE,
	},
	.probe		= rb4xx_cpld_probe,
	.remove		= rb4xx_cpld_remove,
};

static int __init rb4xx_cpld_init(void)
{
	return spi_register_driver(&rb4xx_cpld_driver);
}
module_init(rb4xx_cpld_init);

static void __exit rb4xx_cpld_exit(void)
{
	spi_unregister_driver(&rb4xx_cpld_driver);
}
module_exit(rb4xx_cpld_exit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_LICENSE("GPL v2");
