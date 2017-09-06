/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/reset-controller.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <mach/hardware.h>

static int ox820_reset_reset(struct reset_controller_dev *rcdev,
			      unsigned long id)
{
	writel(BIT(id), SYS_CTRL_RST_SET_CTRL);
	writel(BIT(id), SYS_CTRL_RST_CLR_CTRL);
	return 0;
}

static int ox820_reset_assert(struct reset_controller_dev *rcdev,
			      unsigned long id)
{
	writel(BIT(id), SYS_CTRL_RST_SET_CTRL);

	return 0;
}

static int ox820_reset_deassert(struct reset_controller_dev *rcdev,
				unsigned long id)
{
	writel(BIT(id), SYS_CTRL_RST_CLR_CTRL);

	return 0;
}

static struct reset_control_ops ox820_reset_ops = {
	.reset		= ox820_reset_reset,
	.assert		= ox820_reset_assert,
	.deassert	= ox820_reset_deassert,
};

static const struct of_device_id ox820_reset_dt_ids[] = {
	 { .compatible = "plxtech,nas782x-reset", },
	 { /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, ox820_reset_dt_ids);

struct reset_controller_dev rcdev;

static int ox820_reset_probe(struct platform_device *pdev)
{
	struct reset_controller_dev *rcdev;

	rcdev = devm_kzalloc(&pdev->dev, sizeof(*rcdev), GFP_KERNEL);
	if (!rcdev)
		return -ENOMEM;

	/* note: reset controller is statically mapped */

	rcdev->owner = THIS_MODULE;
	rcdev->nr_resets = 32;
	rcdev->ops = &ox820_reset_ops;
	rcdev->of_node = pdev->dev.of_node;
	reset_controller_register(rcdev);
	platform_set_drvdata(pdev, rcdev);

	return 0;
}

static int ox820_reset_remove(struct platform_device *pdev)
{
	struct reset_controller_dev *rcdev = platform_get_drvdata(pdev);

	reset_controller_unregister(rcdev);

	return 0;
}

static struct platform_driver ox820_reset_driver = {
	.probe	= ox820_reset_probe,
	.remove	= ox820_reset_remove,
	.driver = {
		.name		= "ox820-reset",
		.owner		= THIS_MODULE,
		.of_match_table	= ox820_reset_dt_ids,
	},
};

static int __init ox820_reset_init(void)
{
	return platform_driver_probe(&ox820_reset_driver,
				     ox820_reset_probe);
}
/*
 * reset controller does not support probe deferral, so it has to be
 * initialized before any user, in particular, PCIE uses subsys_initcall.
 */
arch_initcall(ox820_reset_init);

MODULE_AUTHOR("Ma Haijun");
MODULE_LICENSE("GPL");
