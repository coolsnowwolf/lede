/*
 * SPI driver for the Linksys WRT55AG v2 board.
 *
 * Copyright (C) 2008 Gabor Juhos <juhosg at openwrt.org>
 *
 * This file was based on the mmc_over_gpio driver:
 *	Copyright 2008 Michael Buesch <mb@bu3sch.de>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/platform_device.h>
#include <linux/spi/spi_gpio_old.h>
#include <linux/module.h>

#define DRV_NAME	"wrt55agv2-spidevs"
#define DRV_DESC	"SPI driver for the WRT55AG v2 board"
#define DRV_VERSION	"0.1.0"
#define PFX		DRV_NAME ": "

#define GPIO_PIN_MISO	1
#define GPIO_PIN_CS	2
#define GPIO_PIN_CLK	3
#define GPIO_PIN_MOSI	4

static struct platform_device *spi_gpio_dev;

static int __init boardinfo_setup(struct spi_board_info *bi,
		struct spi_master *master, void *data)
{

	strlcpy(bi->modalias, "spi-ks8995", sizeof(bi->modalias));

	bi->max_speed_hz = 5000000 /* Hz */;
	bi->bus_num = master->bus_num;
	bi->mode = SPI_MODE_0;

	return 0;
}

static int __init wrt55agv2_spidevs_init(void)
{
	struct spi_gpio_platform_data pdata;
	int err;

	spi_gpio_dev = platform_device_alloc("spi-gpio", 0);
	if (!spi_gpio_dev) {
		printk(KERN_ERR PFX "no memory for spi-gpio device\n");
		return -ENOMEM;
	}

	memset(&pdata, 0, sizeof(pdata));
	pdata.pin_miso = GPIO_PIN_MISO;
	pdata.pin_cs = GPIO_PIN_CS;
	pdata.pin_clk = GPIO_PIN_CLK;
	pdata.pin_mosi = GPIO_PIN_MOSI;
	pdata.cs_activelow = 1;
	pdata.no_spi_delay = 1;
	pdata.boardinfo_setup = boardinfo_setup;
	pdata.boardinfo_setup_data = NULL;

	err = platform_device_add_data(spi_gpio_dev, &pdata, sizeof(pdata));
	if (err)
		goto err_free_dev;

	err = platform_device_register(spi_gpio_dev);
	if (err) {
		printk(KERN_ERR PFX "unable to register device\n");
		goto err_free_pdata;
	}

	return 0;

err_free_pdata:
	kfree(spi_gpio_dev->dev.platform_data);
	spi_gpio_dev->dev.platform_data = NULL;

err_free_dev:
	platform_device_put(spi_gpio_dev);
	return err;
}

static void __exit wrt55agv2_spidevs_cleanup(void)
{
	if (!spi_gpio_dev)
		return;

	platform_device_unregister(spi_gpio_dev);

	kfree(spi_gpio_dev->dev.platform_data);
	spi_gpio_dev->dev.platform_data = NULL;
	platform_device_put(spi_gpio_dev);
}

static int __init wrt55agv2_spidevs_modinit(void)
{
	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");
	return wrt55agv2_spidevs_init();
}
module_init(wrt55agv2_spidevs_modinit);

static void __exit wrt55agv2_spidevs_modexit(void)
{
	wrt55agv2_spidevs_cleanup();
}
module_exit(wrt55agv2_spidevs_modexit);

MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg at openwrt.org>");
MODULE_LICENSE("GPL v2");

