/*
 *  Custom GPIO-based I2C driver
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
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
 *	bus3	<id>,<sda>,<scl>,<udelay>,<timeout>,<sda_od>,<scl_od>,<scl_oo>
 *
 *  where:
 *
 *  <id>	ID to used as device_id for the corresponding bus (required)
 *  <sda>	GPIO pin ID to used for SDA (required)
 *  <scl>	GPIO pin ID to used for SCL (required)
 *  <udelay>	signal toggle delay.
 *  <timeout>	clock stretching timeout.
 *  <sda_od>	SDA is configured as open drain.
 *  <scl_od>	SCL is configured as open drain.
 *  <scl_oo>	SCL output drivers cannot be turned off.
 *
 *  See include/i2c-gpio.h for more information about the parameters.
 *
 *  If this driver is built into the kernel, you can use the following kernel
 *  command line parameters, with the same values as the corresponding module
 *  parameters listed above:
 *
 *	i2c-gpio-custom.bus0
 *	i2c-gpio-custom.bus1
 *	i2c-gpio-custom.bus2
 *	i2c-gpio-custom.bus3
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 18, 0)
#include <linux/i2c-gpio.h>
#else
#include <linux/platform_data/i2c-gpio.h>
#endif

#define DRV_NAME	"i2c-gpio-custom"
#define DRV_DESC	"Custom GPIO-based I2C driver"
#define DRV_VERSION	"0.1.1"

#define PFX		DRV_NAME ": "

#define BUS_PARAM_ID		0
#define BUS_PARAM_SDA		1
#define BUS_PARAM_SCL		2
#define BUS_PARAM_UDELAY	3
#define BUS_PARAM_TIMEOUT	4
#define BUS_PARAM_SDA_OD	5
#define BUS_PARAM_SCL_OD	6
#define BUS_PARAM_SCL_OO	7

#define BUS_PARAM_REQUIRED	3
#define BUS_PARAM_COUNT		8
#define BUS_COUNT_MAX		4

static unsigned int bus0[BUS_PARAM_COUNT] __initdata;
static unsigned int bus1[BUS_PARAM_COUNT] __initdata;
static unsigned int bus2[BUS_PARAM_COUNT] __initdata;
static unsigned int bus3[BUS_PARAM_COUNT] __initdata;

static unsigned int bus_nump[BUS_COUNT_MAX] __initdata;

#define BUS_PARM_DESC \
	" config -> id,sda,scl[,udelay,timeout,sda_od,scl_od,scl_oo]"

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

static void i2c_gpio_custom_cleanup(void)
{
	int i;

	for (i = 0; i < nr_devices; i++)
		if (devices[i])
			platform_device_put(devices[i]);
}

static int __init i2c_gpio_custom_add_one(unsigned int id, unsigned int *params)
{
	struct platform_device *pdev;
	struct i2c_gpio_platform_data pdata;
	int err;

	if (!bus_nump[id])
		return 0;

	if (bus_nump[id] < BUS_PARAM_REQUIRED) {
		printk(KERN_ERR PFX "not enough parameters for bus%d\n", id);
		err = -EINVAL;
		goto err;
	}

	pdev = platform_device_alloc("i2c-gpio", params[BUS_PARAM_ID]);
	if (!pdev) {
		err = -ENOMEM;
		goto err;
	}

	pdata.sda_pin = params[BUS_PARAM_SDA];
	pdata.scl_pin = params[BUS_PARAM_SCL];
	pdata.udelay = params[BUS_PARAM_UDELAY];
	pdata.timeout = params[BUS_PARAM_TIMEOUT];
	pdata.sda_is_open_drain = params[BUS_PARAM_SDA_OD] != 0;
	pdata.scl_is_open_drain = params[BUS_PARAM_SCL_OD] != 0;
	pdata.scl_is_output_only = params[BUS_PARAM_SCL_OO] != 0;

	err = platform_device_add_data(pdev, &pdata, sizeof(pdata));
	if (err)
		goto err_put;

	err = platform_device_add(pdev);
	if (err)
		goto err_put;

	devices[nr_devices++] = pdev;
	return 0;

err_put:
	platform_device_put(pdev);
err:
	return err;
}

static int __init i2c_gpio_custom_probe(void)
{
	int err;

	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");

	err = i2c_gpio_custom_add_one(0, bus0);
	if (err)
		goto err;

	err = i2c_gpio_custom_add_one(1, bus1);
	if (err)
		goto err;

	err = i2c_gpio_custom_add_one(2, bus2);
	if (err)
		goto err;

	err = i2c_gpio_custom_add_one(3, bus3);
	if (err)
		goto err;

	if (!nr_devices) {
		printk(KERN_ERR PFX "no bus parameter(s) specified\n");
		err = -ENODEV;
		goto err;
	}

	return 0;

err:
	i2c_gpio_custom_cleanup();
	return err;
}

#ifdef MODULE
static int __init i2c_gpio_custom_init(void)
{
	return i2c_gpio_custom_probe();
}
module_init(i2c_gpio_custom_init);

static void __exit i2c_gpio_custom_exit(void)
{
	i2c_gpio_custom_cleanup();
}
module_exit(i2c_gpio_custom_exit);
#else
subsys_initcall(i2c_gpio_custom_probe);
#endif /* MODULE*/

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org >");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
