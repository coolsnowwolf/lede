// SPDX-License-Identifier: GPL-2.0
/* Platform CAN bus driver for Phytium CAN controller
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/platform_device.h>
#include <linux/phy/phy.h>

#include "phytium_can.h"

#define cdev2priv(dev) container_of(dev, struct phytium_can_plat, cdev)

struct phytium_can_plat {
	struct phytium_can_dev cdev;
	struct phytium_can_devtype *devtype;

	int irq;
	void __iomem *reg_base;
};

static const struct can_bittiming_const phytium_bittiming_const_512 = {
	.name = "phytium_can",
	.tseg1_min = 1,		/* Time segment 1 = prop_seg + phase_seg1 */
	.tseg1_max = 16,
	.tseg2_min = 1,		/* Time segment 2 = phase_seg2 */
	.tseg2_max = 8,
	.sjw_max = 4,		/* Synchronisation jump width */
	.brp_min = 1,		/* Bit-rate prescaler */
	.brp_max = 512,
	.brp_inc = 2,
};

static const struct can_bittiming_const phytium_bittiming_const_8192 = {
	.name = "phytium_can",
	.tseg1_min = 1,		/* Time segment 1 = prop_seg + phase_seg1 */
	.tseg1_max = 16,
	.tseg2_min = 1,		/* Time segment 2 = phase_seg2 */
	.tseg2_max = 8,
	.sjw_max = 4,		/* Synchronisation jump width */
	.brp_min = 1,		/* Bit-rate prescaler */
	.brp_max = 8192,
	.brp_inc = 2,
};

static const struct phytium_can_devtype phytium_can_data = {
	.cantype = PHYTIUM_CAN,
	.bittiming_const = &phytium_bittiming_const_512,
};

static const struct phytium_can_devtype phytium_canfd_data = {
	.cantype = PHYTIUM_CANFD,
	.bittiming_const = &phytium_bittiming_const_8192,
};

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_can_acpi_ids[] = {
	{ "PHYT000A", 0 },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_can_acpi_ids);
#endif

#ifdef CONFIG_OF
static const struct of_device_id phytium_can_of_ids[] = {
	{ .compatible = "phytium,can", .data = &phytium_can_data },
	{ .compatible = "phytium,canfd", .data = &phytium_canfd_data },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, phytium_can_of_ids);
#endif


static int phytium_can_plat_probe(struct platform_device *pdev)
{
	struct phytium_can_dev *cdev;
	struct phytium_can_plat *priv;
	struct resource *res;
	const struct of_device_id *of_id;
	const struct phytium_can_devtype *devtype = &phytium_can_data;
	u32 tx_fifo_depth;
	int ret;
	const char *mode;

	ret = fwnode_property_read_u32(dev_fwnode(&pdev->dev), "tx-fifo-depth", &tx_fifo_depth);
	if (ret)
		tx_fifo_depth = 64;

	cdev = phytium_can_allocate_dev(&pdev->dev, sizeof(struct phytium_can_plat),
					tx_fifo_depth);
	if (!cdev)
		return -ENOMEM;

	priv = cdev2priv(cdev);

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->reg_base = devm_ioremap_resource(&pdev->dev, res);
	priv->irq = platform_get_irq(pdev, 0);
	if (IS_ERR(priv->reg_base) || cdev->net->irq < 0) {
		ret = -EINVAL;
		goto fail;
	}

	if (pdev->dev.of_node) {
		cdev->clk = devm_clk_get(&pdev->dev, "can_clk");
		if (IS_ERR(cdev->clk)) {
			dev_err(&pdev->dev, "no clock found\n");
			ret = -ENODEV;
			goto fail;
		}
		cdev->can.clock.freq = clk_get_rate(cdev->clk);

		of_id = of_match_device(phytium_can_of_ids, &pdev->dev);
		if (of_id && of_id->data)
			devtype = of_id->data;
	} else if (has_acpi_companion(&pdev->dev)) {
		ret = fwnode_property_read_u32(dev_fwnode(&pdev->dev),
					       "clock-frequency",
					       &cdev->can.clock.freq);
		if (ret < 0) {
			dev_err(&pdev->dev, "failed to get clock frequency.\n");
			goto fail;
		}
		ret = fwnode_property_read_string(dev_fwnode(&pdev->dev), "mode-select", &mode);
		if (ret < 0) {
			dev_info(&pdev->dev, "get mode-select ret: %d\n", ret);
		}
		else {
			if (strncmp("canfd", mode, strlen("canfd")) == 0) {
				dev_info(&pdev->dev, "use mode-select: canfd\n");
				devtype = &phytium_canfd_data;
			}
		}
	}

	cdev->tx_fifo_depth = tx_fifo_depth;

	if (devtype->cantype == PHYTIUM_CANFD)
		cdev->fdmode = 1;
	else
		cdev->fdmode = 0;

	if (fwnode_property_present(dev_fwnode(&pdev->dev), "extend_brp"))
		cdev->bit_timing = &phytium_bittiming_const_8192;
	else
		cdev->bit_timing = devtype->bittiming_const;
	cdev->can.bittiming_const = devtype->bittiming_const;
	cdev->base = priv->reg_base;
	cdev->net->irq = priv->irq;

	platform_set_drvdata(pdev, cdev->net);

	pm_runtime_enable(cdev->dev);
	ret = phytium_can_register(cdev);
	if (ret)
		goto out_runtime_disable;

	return ret;

out_runtime_disable:
	pm_runtime_disable(cdev->dev);
fail:
	phytium_can_free_dev(cdev->net);
	return ret;
}

static __maybe_unused int phytium_can_plat_suspend(struct device *dev)
{
	return phytium_can_suspend(dev);
}

static __maybe_unused int phytium_can_plat_resume(struct device *dev)
{
	return phytium_can_resume(dev);
}

static int phytium_can_plat_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct phytium_can_dev *cdev = netdev_priv(dev);

	phytium_can_unregister(cdev);

	phytium_can_free_dev(cdev->net);

	return 0;
}

static int __maybe_unused phytium_can_runtime_suspend(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct phytium_can_dev *cdev = netdev_priv(ndev);

	clk_disable_unprepare(cdev->clk);

	return 0;
}

static int __maybe_unused phytium_can_runtime_resume(struct device *dev)
{
	struct net_device *ndev = dev_get_drvdata(dev);
	struct phytium_can_dev *cdev = netdev_priv(ndev);

	return clk_prepare_enable(cdev->clk);
}

static const struct dev_pm_ops phytium_can_plat_pm_ops = {
	SET_RUNTIME_PM_OPS(phytium_can_runtime_suspend,
			   phytium_can_runtime_resume, NULL)
	SET_SYSTEM_SLEEP_PM_OPS(phytium_can_suspend, phytium_can_resume)
};

static struct platform_driver phytium_can_plat_driver = {
	.driver = {
		.name = KBUILD_MODNAME,
		.of_match_table = of_match_ptr(phytium_can_of_ids),
		.acpi_match_table = ACPI_PTR(phytium_can_acpi_ids),
		.pm = &phytium_can_plat_pm_ops,
	},
	.probe = phytium_can_plat_probe,
	.remove = phytium_can_plat_remove,
};

module_platform_driver(phytium_can_plat_driver);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium CAN driver for IO Mapped controllers");
