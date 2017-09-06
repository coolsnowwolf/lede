/*
 * Copyright (C) 2007 Nicolas Thill <nico@openwrt.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/device.h>
#include <linux/fs.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <gpio.h>

#define DRVNAME "ar7_gpio"
#define LONGNAME "TI AR7 GPIOs Driver"

MODULE_AUTHOR("Nicolas Thill <nico@openwrt.org>");
MODULE_DESCRIPTION(LONGNAME);
MODULE_LICENSE("GPL");

static int ar7_gpio_major;

static ssize_t ar7_gpio_write(struct file *file, const char __user *buf,
	size_t len, loff_t *ppos)
{
	int pin = iminor(file->f_path.dentry->d_inode);
	size_t i;

	for (i = 0; i < len; ++i) {
		char c;
		if (get_user(c, buf + i))
			return -EFAULT;
		switch (c) {
		case '0':
			gpio_set_value(pin, 0);
			break;
		case '1':
			gpio_set_value(pin, 1);
			break;
		case 'd':
		case 'D':
			ar7_gpio_disable(pin);
			break;
		case 'e':
		case 'E':
			ar7_gpio_enable(pin);
			break;
		case 'i':
		case 'I':
		case '<':
			gpio_direction_input(pin);
			break;
		case 'o':
		case 'O':
		case '>':
			gpio_direction_output(pin, 0);
			break;
		default:
			return -EINVAL;
		}
	}

	return len;
}

static ssize_t ar7_gpio_read(struct file *file, char __user *buf,
	size_t len, loff_t *ppos)
{
	int pin = iminor(file->f_path.dentry->d_inode);
	int value;

	value = gpio_get_value(pin);
	if (put_user(value ? '1' : '0', buf))
		return -EFAULT;

	return 1;
}

static int ar7_gpio_open(struct inode *inode, struct file *file)
{
	int m = iminor(inode);

	if (m >= (ar7_is_titan() ? TITAN_GPIO_MAX : AR7_GPIO_MAX))
		return -EINVAL;

	return nonseekable_open(inode, file);
}

static int ar7_gpio_release(struct inode *inode, struct file *file)
{
	return 0;
}

static const struct file_operations ar7_gpio_fops = {
	.owner   = THIS_MODULE,
	.write   = ar7_gpio_write,
	.read    = ar7_gpio_read,
	.open    = ar7_gpio_open,
	.release = ar7_gpio_release,
	.llseek  = no_llseek,
};

static struct platform_device *ar7_gpio_device;

static int __init ar7_gpio_char_init(void)
{
	int rc;

	ar7_gpio_device = platform_device_alloc(DRVNAME, -1);
	if (!ar7_gpio_device)
		return -ENOMEM;

	rc = platform_device_add(ar7_gpio_device);
	if (rc < 0)
		goto out_put;

	rc = register_chrdev(ar7_gpio_major, DRVNAME, &ar7_gpio_fops);
	if (rc < 0)
		goto out_put;

	ar7_gpio_major = rc;

	rc = 0;

	goto out;

out_put:
	platform_device_put(ar7_gpio_device);
out:
	return rc;
}

static void __exit ar7_gpio_char_exit(void)
{
	unregister_chrdev(ar7_gpio_major, DRVNAME);
	platform_device_unregister(ar7_gpio_device);
}

module_init(ar7_gpio_char_init);
module_exit(ar7_gpio_char_exit);
