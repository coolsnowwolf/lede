/*
 *  Custom GPIO-based SPI driver
 *
 *  Copyright (C) 2013 Marco Burato <zmaster.adsl@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  Based on i2c-gpio-custom by:
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 * ---------------------------------------------------------------------------
 *
 *  The behaviour of this driver can be altered by setting some parameters
 *  from the insmod command line.
 *
 *  The following parameters are adjustable:
 *
 *	bus0	These four arguments can be arrays of
 *	bus1	1-8 unsigned integers as follows:
 *	bus2
 *	bus3	<id>,<sck>,<mosi>,<miso>,<mode1>,<maxfreq1>,<cs1>,...
 *
 *  where:
 *
 *  <id>	ID to used as device_id for the corresponding bus (required)
 *  <sck>	GPIO pin ID to be used for bus SCK (required)
 *  <mosi>	GPIO pin ID to be used for bus MOSI (required*)
 *  <miso>	GPIO pin ID to be used for bus MISO (required*)
 *  <modeX>	Mode configuration for slave X in the bus (required)
 *		(see /include/linux/spi/spi.h)
 *  <maxfreqX>	Maximum clock frequency in Hz for slave X in the bus (required)
 *  <csX>	GPIO pin ID to be used for slave X CS (required**)
 *
 *	Notes:
 *	*	If a signal is not used (for example there is no MISO) you need
 *		to set the GPIO pin ID for that signal to an invalid value.
 *	**	If you only have 1 slave in the bus with no CS, you can omit the
 *		<cs1> param or set it to an invalid GPIO id to disable it. When
 *		you have 2 or more slaves, they must all have a valid CS.
 *
 *  If this driver is built into the kernel, you can use the following kernel
 *  command line parameters, with the same values as the corresponding module
 *  parameters listed above:
 *
 *	spi-gpio-custom.bus0
 *	spi-gpio-custom.bus1
 *	spi-gpio-custom.bus2
 *	spi-gpio-custom.bus3
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>

#define DRV_NAME	"spi-gpio-custom"
#define DRV_DESC	"Custom GPIO-based SPI driver"
#define DRV_VERSION	"0.1"

#define PFX		DRV_NAME ": "

#define BUS_PARAM_ID		0
#define BUS_PARAM_SCK		1
#define BUS_PARAM_MOSI		2
#define BUS_PARAM_MISO		3
#define BUS_PARAM_MODE1		4
#define BUS_PARAM_MAXFREQ1	5
#define BUS_PARAM_CS1		6

#define BUS_SLAVE_COUNT_MAX	8
#define BUS_PARAM_REQUIRED	6
#define BUS_PARAM_PER_SLAVE	3
#define BUS_PARAM_COUNT		(4+BUS_PARAM_PER_SLAVE*BUS_SLAVE_COUNT_MAX)
#define BUS_COUNT_MAX		4

static unsigned int bus0[BUS_PARAM_COUNT] __initdata;
static unsigned int bus1[BUS_PARAM_COUNT] __initdata;
static unsigned int bus2[BUS_PARAM_COUNT] __initdata;
static unsigned int bus3[BUS_PARAM_COUNT] __initdata;

static unsigned int bus_nump[BUS_COUNT_MAX] __initdata;

#define BUS_PARM_DESC \
	" config -> id,sck,mosi,miso,mode1,maxfreq1[,cs1,mode2,maxfreq2,cs2,...]"

module_param_array(bus0, uint, &bus_nump[0], 0);
MODULE_PARM_DESC(bus0, "bus0" BUS_PARM_DESC);
module_param_array(bus1, uint, &bus_nump[1], 0);
MODULE_PARM_DESC(bus1, "bus1" BUS_PARM_DESC);
module_param_array(bus2, uint, &bus_nump[2], 0);
MODULE_PARM_DESC(bus2, "bus2" BUS_PARM_DESC);
module_param_array(bus3, uint, &bus_nump[3], 0);
MODULE_PARM_DESC(bus3, "bus3" BUS_PARM_DESC);

static struct platform_device *devices[BUS_COUNT_MAX];
static unsigned int nr_devices;

static void spi_gpio_custom_cleanup(void)
{
	int i;

	for (i = 0; i < nr_devices; i++)
		if (devices[i])
			platform_device_unregister(devices[i]);
}

static int __init spi_gpio_custom_get_slave_mode(unsigned int id,
					  unsigned int *params,
					  int slave_index)
{
	int param_index;

	param_index = BUS_PARAM_MODE1+slave_index*BUS_PARAM_PER_SLAVE;
	if (param_index >= bus_nump[id])
		return -1;

	return params[param_index];
}
static int __init spi_gpio_custom_get_slave_maxfreq(unsigned int id,
					     unsigned int *params,
					     int slave_index)
{
	int param_index;

	param_index = BUS_PARAM_MAXFREQ1+slave_index*BUS_PARAM_PER_SLAVE;
	if (param_index >= bus_nump[id])
		return -1;

	return params[param_index];
}
static int __init spi_gpio_custom_get_slave_cs(unsigned int id,
					unsigned int *params,
					int slave_index)
{
	int param_index;

	param_index = BUS_PARAM_CS1+slave_index*BUS_PARAM_PER_SLAVE;
	if (param_index >= bus_nump[id])
		return -1;
	if (!gpio_is_valid(params[param_index]))
		return -1;

	return params[param_index];
}

static int __init spi_gpio_custom_check_params(unsigned int id, unsigned int *params)
{
	int i;
	struct spi_master *master;

	if (bus_nump[id] < BUS_PARAM_REQUIRED) {
		printk(KERN_ERR PFX "not enough values for parameter bus%d\n",
		       id);
		return -EINVAL;
	}

	if (bus_nump[id] > (1+BUS_PARAM_CS1)) {
		/* more than 1 device: check CS GPIOs */
		for (i = 0; i < BUS_SLAVE_COUNT_MAX; i++) {
			/* no more slaves? */
			if (spi_gpio_custom_get_slave_mode(id, params, i) < 0)
				break;

			if (spi_gpio_custom_get_slave_cs(id, params, i) < 0) {
				printk(KERN_ERR PFX "invalid/missing CS gpio for slave %d on bus %d\n",
				       i, params[BUS_PARAM_ID]);
				return -EINVAL;
			}
		}
	}

	if (!gpio_is_valid(params[BUS_PARAM_SCK])) {
		printk(KERN_ERR PFX "invalid SCK gpio for bus %d\n",
		       params[BUS_PARAM_ID]);
		return -EINVAL;
	}

	master = spi_busnum_to_master(params[BUS_PARAM_ID]);
	if (master) {
		spi_master_put(master);
		printk(KERN_ERR PFX "bus %d already exists\n",
		       params[BUS_PARAM_ID]);
		return -EEXIST;
	}

	return 0;
}

static int __init spi_gpio_custom_add_one(unsigned int id, unsigned int *params)
{
	struct platform_device *pdev;
	struct spi_gpio_platform_data pdata;
	int i;
	int num_cs;
	int err;
	struct spi_master *master;
	struct spi_device *slave;
	struct spi_board_info slave_info;
	int mode, maxfreq, cs;


	if (!bus_nump[id])
		return 0;

	err = spi_gpio_custom_check_params(id, params);
	if (err)
		goto err;

	/* Create BUS device node */

	pdev = platform_device_alloc("spi_gpio", params[BUS_PARAM_ID]);
	if (!pdev) {
		err = -ENOMEM;
		goto err;
	}

	num_cs = 0;
	for (i = 0; i < BUS_SLAVE_COUNT_MAX; i++) {
		/* no more slaves? */
		if (spi_gpio_custom_get_slave_mode(id, params, i) < 0)
			break;

		if (spi_gpio_custom_get_slave_cs(id, params, i) >= 0)
			num_cs++;
	}
	if (num_cs == 0) {
		/*
		 * Even if no CS is used, spi modules expect
		 * at least 1 (unused)
		 */
		num_cs = 1;
	}

	pdata.sck = params[BUS_PARAM_SCK];
	pdata.mosi = gpio_is_valid(params[BUS_PARAM_MOSI])
		? params[BUS_PARAM_MOSI]
		: SPI_GPIO_NO_MOSI;
	pdata.miso = gpio_is_valid(params[BUS_PARAM_MISO])
		? params[BUS_PARAM_MISO]
		: SPI_GPIO_NO_MISO;
	pdata.num_chipselect = num_cs;

	err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
	if (err) {
		platform_device_put(pdev);
		goto err;
	}

	err = platform_device_add(pdev);
	if (err) {
		printk(KERN_ERR PFX "platform_device_add failed with return code %d\n",
		       err);
		platform_device_put(pdev);
		goto err;
	}

	/* Register SLAVE devices */

	for (i = 0; i < BUS_SLAVE_COUNT_MAX; i++) {
		mode = spi_gpio_custom_get_slave_mode(id, params, i);
		maxfreq = spi_gpio_custom_get_slave_maxfreq(id, params, i);
		cs = spi_gpio_custom_get_slave_cs(id, params, i);

		/* no more slaves? */
		if (mode < 0)
			break;

		memset(&slave_info, 0, sizeof(slave_info));
		strcpy(slave_info.modalias, "spidev");
		slave_info.controller_data = (void *)((cs >= 0)
			? cs
			: SPI_GPIO_NO_CHIPSELECT);
		slave_info.max_speed_hz = maxfreq;
		slave_info.bus_num = params[BUS_PARAM_ID];
		slave_info.chip_select = i;
		slave_info.mode = mode;

		master = spi_busnum_to_master(params[BUS_PARAM_ID]);
		if (!master) {
			printk(KERN_ERR PFX "unable to get master for bus %d\n",
			       params[BUS_PARAM_ID]);
			err = -EINVAL;
			goto err_unregister;
		}
		slave = spi_new_device(master, &slave_info);
		spi_master_put(master);
		if (!slave) {
			printk(KERN_ERR PFX "unable to create slave %d for bus %d\n",
			       i, params[BUS_PARAM_ID]);
			/* Will most likely fail due to unsupported mode bits */
			err = -EINVAL;
			goto err_unregister;
		}
	}

	devices[nr_devices++] = pdev;

	return 0;

err_unregister:
	platform_device_unregister(pdev);
err:
	return err;
}

static int __init spi_gpio_custom_probe(void)
{
	int err;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");

	err = spi_gpio_custom_add_one(0, bus0);
	if (err)
		goto err;

	err = spi_gpio_custom_add_one(1, bus1);
	if (err)
		goto err;

	err = spi_gpio_custom_add_one(2, bus2);
	if (err)
		goto err;

	err = spi_gpio_custom_add_one(3, bus3);
	if (err)
		goto err;

	if (!nr_devices) {
		printk(KERN_ERR PFX "no bus parameter(s) specified\n");
		err = -ENODEV;
		goto err;
	}

	return 0;

err:
	spi_gpio_custom_cleanup();
	return err;
}

#ifdef MODULE
static int __init spi_gpio_custom_init(void)
{
	return spi_gpio_custom_probe();
}
module_init(spi_gpio_custom_init);

static void __exit spi_gpio_custom_exit(void)
{
	spi_gpio_custom_cleanup();
}
module_exit(spi_gpio_custom_exit);
#else
subsys_initcall(spi_gpio_custom_probe);
#endif /* MODULE*/

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Marco Burato <zmaster.adsl@gmail.com>");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
