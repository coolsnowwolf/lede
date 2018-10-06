/*
*  APU2 LED/GPIO Driver
*  Copyright (c) 2016 Christian Lamparter <chunkeey (at) googlemail.com>
*
*  Based on gpio-apu2.c - AMD FCH GPIO support for PC-Engines APU-2 board
*
*  Copyright (c) 2015  Carsten Spiess <fli4l at carsten-spiess.de>
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/miscdevice.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/version.h>
#include <linux/dmi.h>
#include <linux/string.h>

#include <linux/leds.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>

#define DEVNAME                 "leds-apu2"

#define FCH_ACPI_MMIO_BASE      0xFED80000
#define FCH_GPIO_BASE           (FCH_ACPI_MMIO_BASE + 0x1500)
#define FCH_GPIO_SIZE           0x300

#define APU_NUM_GPIO            4

#define GPIO_BIT_DIR            23
#define GPIO_BIT_WRITE          22
#define GPIO_BIT_READ           16

/* internal variables */
static struct pci_dev *gpio_apu2_pci;
static DEFINE_SPINLOCK (gpio_lock);

/* the watchdog platform device */
static struct platform_device *gpio_apu2_platform_device;
static struct platform_device *leddev;
static struct platform_device *keydev;

static const struct pci_device_id gpio_apu2_pci_tbl[] ={
	{PCI_VENDOR_ID_AMD, PCI_DEVICE_ID_AMD_HUDSON2_SMBUS, PCI_ANY_ID, PCI_ANY_ID},
	{ 0, } /* End of list */
};
MODULE_DEVICE_TABLE (pci, gpio_apu2_pci_tbl);

/* EGPIO89=GPIO32, AGPIO68=GPIO57, AGPIO69=GPIO58, AGPIO70=GPIO59 */
static u8 gpio_offset[APU_NUM_GPIO] = {89, 68, 69, 70};

static void __iomem *gpio_addr[APU_NUM_GPIO] = {NULL, NULL, NULL, NULL};

static int gpio_apu2_get_dir (struct gpio_chip *chip, unsigned offset)
{
	u32 val;

	val = ~ioread32 (gpio_addr[offset]);

	return (val >> GPIO_BIT_DIR) & 1;
}

static int gpio_apu2_dir_in (struct gpio_chip *gc, unsigned offset)
{
	u32 val;

	spin_lock_bh (&gpio_lock);

	val = ioread32 (gpio_addr[offset]);
	val &= ~BIT(GPIO_BIT_DIR);
	iowrite32 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);

	return 0;
}

static int gpio_apu2_dir_out (struct gpio_chip *chip, unsigned offset,
		int value)
{
	u32 val;

	spin_lock_bh (&gpio_lock);

	val = ioread32 (gpio_addr[offset]);
	val |= BIT(GPIO_BIT_DIR);
	iowrite32 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);

	return 0;
}

static int gpio_apu2_get_data (struct gpio_chip *chip, unsigned offset)
{
	u32 val;

	val = ioread32 (gpio_addr[offset]);

	return (val >> GPIO_BIT_READ) & 1;
}

static void gpio_apu2_set_data (struct gpio_chip *chip, unsigned offset, int value)
{
	u32 val;

	spin_lock_bh (&gpio_lock);

	val = ioread32 (gpio_addr[offset]);

	if (value)
		val |= BIT(GPIO_BIT_WRITE);
	else
		val &= ~BIT(GPIO_BIT_WRITE);

	iowrite32 (val, gpio_addr[offset]);

	spin_unlock_bh (&gpio_lock);
}

static struct gpio_chip gpio_apu2_chip = {
	.label = DEVNAME,
	.owner = THIS_MODULE,
	.base = -1,
	.ngpio = APU_NUM_GPIO,
	.get_direction = gpio_apu2_get_dir,
	.direction_input = gpio_apu2_dir_in,
	.direction_output = gpio_apu2_dir_out,
	.get = gpio_apu2_get_data,
	.set = gpio_apu2_set_data,
};

/*
 *
 */
static int gpio_apu2_probe (struct platform_device *dev)
{
	int ret = 0;
	int i;
	struct pci_dev *pci_dev = NULL;

	/* Match the PCI device */
	for_each_pci_dev (pci_dev) {
		if (pci_match_id (gpio_apu2_pci_tbl, pci_dev) != NULL) {
			gpio_apu2_pci = pci_dev;
			break;
		}
	}

	if (!gpio_apu2_pci)
		return -ENODEV;

	pr_info ("%s: PCI Revision ID: 0x%x\n", DEVNAME, gpio_apu2_pci->revision);

	/* Determine type of southbridge chipset */
	if (gpio_apu2_pci->revision < 0x40) {
		return -EACCES;
	}

	/* Request memory region for GPIO's */
	if (!devm_request_mem_region (&dev->dev, FCH_GPIO_BASE,
		FCH_GPIO_SIZE, DEVNAME)){
		pr_err ("%s: request GPIO mem region failed\n", DEVNAME);
		return -ENXIO;
	}

	/* Map IO's for GPIO's */
	for (i = 0; i < APU_NUM_GPIO; i++) {
		gpio_addr[i] = devm_ioremap (&dev->dev,
			FCH_GPIO_BASE + (gpio_offset[i] * sizeof (u32)), sizeof (u32));
		if (!gpio_addr[i]) {
			pr_err ("%s: map GPIO%d address failed\n", DEVNAME, gpio_offset[i]);
			return -ENXIO;
		}
	}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,5,0)
	gpio_apu2_chip.dev = &dev->dev;
#else
	gpio_apu2_chip.parent = &dev->dev;
#endif
	ret = gpiochip_add (&gpio_apu2_chip);
	if (ret) {
		pr_err ("%s: adding gpiochip failed\n", DEVNAME);
	}

	return ret;
}

static int gpio_apu2_remove (struct platform_device *dev)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
	int ret;
	ret = gpiochip_remove (&gpio_apu2_chip);
#else /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
	gpiochip_remove (&gpio_apu2_chip);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0) */
	return 0;
}

static struct platform_driver gpio_apu2_driver = {
	.probe = gpio_apu2_probe,
	.remove = gpio_apu2_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = DEVNAME
	}
};

static struct gpio_led apu2_leds_gpio[] = {
	{
		.name           = "apu2:green:power",
		.gpio           = 509,
		.active_low     = 1,
	},
	{
		.name           = "apu2:green:led2",
		.gpio           = 510,
		.active_low     = 1,
	},
	{
		.name           = "apu2:green:led3",
		.gpio           = 511,
		.active_low     = 1,
	},
};

static struct gpio_keys_button apu2_gpio_keys[] = {
	{
		.desc           = "Reset button",
		.type           = EV_KEY,
		.code           = KEY_RESTART,
		.debounce_interval = 60,
		.gpio           = 508,
		.active_low     = 1,
	},
};

static void register_gpio_keys_polled(int id, unsigned poll_interval,
				      unsigned nbuttons,
				      struct gpio_keys_button *buttons)
{
	struct gpio_keys_platform_data pdata = { };
	int err;

	keydev = platform_device_alloc("gpio-keys-polled", id);
	if (!keydev) {
		printk(KERN_ERR "Failed to allocate gpio-keys platform device\n");
		return;
	}

	pdata.poll_interval = poll_interval;
	pdata.nbuttons = nbuttons;
	pdata.buttons = buttons;

	err = platform_device_add_data(keydev, &pdata, sizeof(pdata));
	if (err) {
		dev_err(&keydev->dev, "failed to add platform data to key driver (%d)", err);
		goto err_put_pdev;
	}

	err = platform_device_add(keydev);
	if (err) {
		dev_err(&keydev->dev, "failed to register key platform device (%d)", err);
		goto err_put_pdev;
	}

	return;

err_put_pdev:
	platform_device_put(keydev);
	keydev = NULL;
}

static void register_leds_gpio(int id, unsigned num_leds, struct gpio_led *leds)
{
	struct gpio_led_platform_data pdata = { };
	int err;

	leddev = platform_device_alloc("leds-gpio", id);
	if (!leddev) {
		printk(KERN_ERR "Failed to allocate leds-gpio platform device\n");
		return;
	}

	pdata.num_leds = num_leds;
	pdata.leds = leds;

	err = platform_device_add_data(leddev, &pdata, sizeof(pdata));
	if (err) {
		dev_err(&leddev->dev, "failed to add platform data to key driver (%d)", err);
		goto err_put_pdev;
	}

	err = platform_device_add(leddev);
	if (err) {
		dev_err(&leddev->dev, "failed to register key platform device (%d)", err);
		goto err_put_pdev;
	}

	return;

err_put_pdev:
	platform_device_put(leddev);
	leddev = NULL;
}

static int __init gpio_apu2_init (void)
{
	int err;
	const char *board_vendor = dmi_get_system_info(DMI_BOARD_VENDOR);
	const char *board_name = dmi_get_system_info(DMI_BOARD_NAME);

	pr_info ("%s: load APU2/LED GPIO driver module\n", DEVNAME);

	err = platform_driver_register (&gpio_apu2_driver);
	if (err)
		goto exit;

	gpio_apu2_platform_device = platform_device_register_simple (DEVNAME, -1, NULL, 0);
	if (IS_ERR(gpio_apu2_platform_device)) {
		err = PTR_ERR(gpio_apu2_platform_device);
		goto exit_driver;
	}

	pr_info ("%s: APU2 GPIO/LED driver module loaded\n", DEVNAME);

	register_leds_gpio(-1, ARRAY_SIZE(apu2_leds_gpio), apu2_leds_gpio);
	register_gpio_keys_polled(-1, 20, ARRAY_SIZE(apu2_gpio_keys), apu2_gpio_keys);
	return 0;

exit_driver:
	platform_driver_unregister (&gpio_apu2_driver);
exit:
	return err;
}

static void __exit gpio_apu2_exit (void)
{
	platform_device_unregister (gpio_apu2_platform_device);
	platform_device_unregister (leddev);
	platform_device_unregister (keydev);
	platform_driver_unregister (&gpio_apu2_driver);
	pr_info ("%s: APU2 GPIO/LED driver module unloaded\n", DEVNAME);
}

MODULE_AUTHOR ("Carsten Spiess <fli4l at carsten-spiess.de>");
MODULE_DESCRIPTION("GPIO driver for AMD FCH on PC-Engines APU-2");
MODULE_LICENSE("GPL");

module_init (gpio_apu2_init);
module_exit (gpio_apu2_exit);
