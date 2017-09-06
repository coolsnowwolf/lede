/*
 *  Custom GPIO-based W1 driver
 *
 *  Copyright (C) 2007 Gabor Juhos <juhosg at openwrt.org>
 *  Copyright (C) 2008 Bifferos <bifferos at yahoo.co.uk>
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
 *	bus0	These four arguments must be arrays of
 *	bus1	3 unsigned integers as follows:
 *	bus2
 *	bus3	<id>,<pin>,<od>
 *
 *  where:
 *
 *  <id>	ID to used as device_id for the corresponding bus (required)
 *  <sda>	GPIO pin ID of data pin (required)
 *  <od>	Pin is configured as open drain.
 *
 *  See include/w1-gpio.h for more information about the parameters.
 *
 *  If this driver is built into the kernel, you can use the following kernel
 *  command line parameters, with the same values as the corresponding module
 *  parameters listed above:
 *
 *	w1-gpio-custom.bus0
 *	w1-gpio-custom.bus1
 *	w1-gpio-custom.bus2
 *	w1-gpio-custom.bus3
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>

#include <linux/w1-gpio.h>

#define DRV_NAME	"w1-gpio-custom"
#define DRV_DESC	"Custom GPIO-based W1 driver"
#define DRV_VERSION	"0.1.1"

#define PFX		DRV_NAME ": "

#define BUS_PARAM_ID		0
#define BUS_PARAM_PIN		1
#define BUS_PARAM_OD		2

#define BUS_PARAM_REQUIRED	3
#define BUS_PARAM_COUNT		3
#define BUS_COUNT_MAX		4

static unsigned int bus0[BUS_PARAM_COUNT] __initdata;
static unsigned int bus1[BUS_PARAM_COUNT] __initdata;
static unsigned int bus2[BUS_PARAM_COUNT] __initdata;
static unsigned int bus3[BUS_PARAM_COUNT] __initdata;

static unsigned int bus_nump[BUS_COUNT_MAX] __initdata;

#define BUS_PARM_DESC " config -> id,pin,od"

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

static void w1_gpio_custom_cleanup(void)
{
	int i;

	for (i = 0; i < nr_devices; i++)
		if (devices[i])
			platform_device_put(devices[i]);
}

static int __init w1_gpio_custom_add_one(unsigned int id, unsigned int *params)
{
	struct platform_device *pdev;
	struct w1_gpio_platform_data pdata;
	int err;

	if (!bus_nump[id])
		return 0;

	if (bus_nump[id] < BUS_PARAM_REQUIRED) {
		printk(KERN_ERR PFX "not enough parameters for bus%d\n", id);
		err = -EINVAL;
		goto err;
	}

	pdev = platform_device_alloc("w1-gpio", params[BUS_PARAM_ID]);
	if (!pdev) {
		err = -ENOMEM;
		goto err;
	}

	pdata.pin = params[BUS_PARAM_PIN];
	pdata.is_open_drain = params[BUS_PARAM_OD] ? 1 : 0;
	pdata.enable_external_pullup = NULL;
	pdata.ext_pullup_enable_pin = -EINVAL;

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

static int __init w1_gpio_custom_probe(void)
{
	int err;

	nr_devices = 0;
	printk(KERN_INFO DRV_DESC " version " DRV_VERSION "\n");

	err = w1_gpio_custom_add_one(0, bus0);
	if (err)
		goto err;

	err = w1_gpio_custom_add_one(1, bus1);
	if (err)
		goto err;

	err = w1_gpio_custom_add_one(2, bus2);
	if (err)
		goto err;

	err = w1_gpio_custom_add_one(3, bus3);
	if (err)
		goto err;

	if (!nr_devices) {
		printk(KERN_ERR PFX "no bus parameter(s) specified\n");
		err = -ENODEV;
		goto err;
	}

	return 0;

err:
	w1_gpio_custom_cleanup();
	return err;
}

#ifdef MODULE
static int __init w1_gpio_custom_init(void)
{
	return w1_gpio_custom_probe();
}
module_init(w1_gpio_custom_init);

static void __exit w1_gpio_custom_exit(void)
{
	w1_gpio_custom_cleanup();
}
module_exit(w1_gpio_custom_exit);
#else
subsys_initcall(w1_gpio_custom_probe);
#endif /* MODULE*/

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Bifferos <bifferos at yahoo.co.uk >");
MODULE_DESCRIPTION(DRV_DESC);
MODULE_VERSION(DRV_VERSION);
