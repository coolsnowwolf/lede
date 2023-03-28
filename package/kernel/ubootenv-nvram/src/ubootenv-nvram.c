// SPDX-License-Identifier: GPL-2.0-only
/*
 *   Copyright (C) 2022  Bjørn Mork <bjorn@mork.no>
 */

#include <linux/fs.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/miscdevice.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/of.h>
#include <linux/of_reserved_mem.h>
#include <linux/platform_device.h>

#define NAME "ubootenv"

struct ubootenv_drvdata {
	void *env;
	struct reserved_mem *rmem;
	struct miscdevice misc;
};

static inline struct ubootenv_drvdata *to_ubootenv_drvdata(struct file *file)
{
	return container_of(file->private_data, struct ubootenv_drvdata, misc);
}

static ssize_t ubootenv_write(struct file *file, const char __user *buffer, size_t count,
			      loff_t *ppos)
{
	struct ubootenv_drvdata *data = to_ubootenv_drvdata(file);

	if (!data->env)
		return -EIO;
	return simple_write_to_buffer(data->env, data->rmem->size, ppos, buffer, count);
}

static ssize_t ubootenv_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	struct ubootenv_drvdata *data = to_ubootenv_drvdata(file);

	if (!data->env)
		return 0;
	return simple_read_from_buffer(buffer, count, ppos, data->env, data->rmem->size);
}

static loff_t ubootenv_llseek(struct file *file, loff_t off, int whence)
{
	struct ubootenv_drvdata *data = to_ubootenv_drvdata(file);

	return fixed_size_llseek(file, off, whence, data->rmem->size);
}

/* Faking the minimal mtd ioctl subset required by the fw_env.c */
static long ubootenv_ioctl(struct file *file, u_int cmd, u_long arg)
{
	struct ubootenv_drvdata *data = to_ubootenv_drvdata(file);
	void __user *argp = (void __user *)arg;
	struct mtd_info_user info = {
		.type = MTD_NORFLASH,
		.size = data->rmem->size,
	};

	switch (cmd) {
	case MEMISLOCKED:
	case MEMERASE:
		break;
	case MEMGETINFO:
		if (copy_to_user(argp, &info, sizeof(struct mtd_info_user)))
			return -EFAULT;
		break;
	default:
		return -ENOTTY;
	}
	return 0;
}

static const struct file_operations ubootenv_fops = {
	.owner          = THIS_MODULE,
	.read           = ubootenv_read,
	.write          = ubootenv_write,
	.llseek         = ubootenv_llseek,
	.unlocked_ioctl = ubootenv_ioctl,
};

/* We can only map a single reserved-memory range */
static struct ubootenv_drvdata drvdata = {
	.misc = {
		.fops  = &ubootenv_fops,
		.minor = MISC_DYNAMIC_MINOR,
		.name  = NAME,
	},
};

const struct of_device_id of_ubootenv_match[] = {
	{ .compatible = "ubootenv" },
	{},
};
MODULE_DEVICE_TABLE(of, of_ubootenv_match);

static int ubootenv_probe(struct platform_device *pdev)
{
	struct ubootenv_drvdata *data = &drvdata;
	struct device *dev = &pdev->dev;
	struct device_node *np;

	/* enforce single instance */
	if (data->env)
		return -EINVAL;

	np = of_parse_phandle(dev->of_node, "memory-region", 0);
	if (!np)
		return -ENODEV;

	data->rmem = of_reserved_mem_lookup(np);
	of_node_put(np);
	if (!data->rmem)
		return -ENODEV;

	if (!data->rmem->size || (data->rmem->size > ULONG_MAX))
		return -EINVAL;

	if (!PAGE_ALIGNED(data->rmem->base) || !PAGE_ALIGNED(data->rmem->size))
		return -EINVAL;

	data->env = devm_memremap(&pdev->dev, data->rmem->base, data->rmem->size, MEMREMAP_WB);
	platform_set_drvdata(pdev, data);

	data->misc.parent = &pdev->dev;
	return misc_register(&data->misc);
}

static int ubootenv_remove(struct platform_device *pdev)
{
	struct ubootenv_drvdata *data = platform_get_drvdata(pdev);

	data->env = NULL;
	misc_deregister(&data->misc);
	return 0;
}

static struct platform_driver ubootenv_driver = {
	.probe = ubootenv_probe,
	.remove = ubootenv_remove,
	.driver = {
		.name           = NAME,
		.owner          = THIS_MODULE,
		.of_match_table = of_ubootenv_match,
	},
};

module_platform_driver(ubootenv_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bjørn Mork <bjorn@mork.no>");
MODULE_DESCRIPTION("Access u-boot environment in RAM");
