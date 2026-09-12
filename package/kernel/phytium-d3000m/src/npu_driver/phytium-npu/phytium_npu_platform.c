// SPDX-License-Identifier: GPL-2.0
/* Platform NPU driver for Phytium NPU controller
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#include <linux/acpi.h>
#include <linux/interrupt.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/io.h>
#include <linux/pm.h>
#include <linux/version.h>
#include <linux/dma-mapping.h>
#include "phytium_npu.h"
#include "phytium_npu_pd2408_reg.h"

static irqreturn_t phytium_npu_thread_irq(int irq, void *dev_id)
{
	struct platform_device *pdev = (struct platform_device *)dev_id;

	return phytium_npu_handle_thread_irq(&pdev->dev);
}

static irqreturn_t phytium_npu_handle_isr(int irq, void *dev_id)
{
	struct platform_device *pdev = (struct platform_device *)dev_id;

	if (!pdev)
		return IRQ_NONE;

	return phytium_npu_handle_irq(&pdev->dev);
}

static int phytium_npu_plat_probe(struct platform_device *pdev)
{
	struct phytium_npu_dev *npu_dev;
	struct resource *res;
	struct device *dev = &pdev->dev;
	struct fwnode_handle *fwnode = dev_fwnode(&pdev->dev);
	void __iomem *reg_addr, *preg_addr = NULL;
	u64 dma_mask;
	int ret, irq;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res) {
		dev_err(&pdev->dev, "no memory resource defined\n");
		return -EINVAL;
	}
	pr_info("%s: registers %#llx-%#llx\n", __func__,
		(unsigned long long)res->start, (unsigned long long)res->end);

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "could not get IRQ\n");
		return -ENXIO;
	}

	reg_addr = devm_ioremap(&pdev->dev, res->start, resource_size(res));

	if (!reg_addr) {
		dev_err(&pdev->dev, "failed to map npu registers\n");
		return -ENXIO;
	}

	if (pdev->dev.of_node) {
		res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
		if (res) {
			pr_info("%s: registers power manage %#llx-%#llx\n", __func__,
				(unsigned long long)res->start, (unsigned long long)res->end);
			preg_addr = devm_ioremap(&pdev->dev, res->start, resource_size(res));

			if (!preg_addr) {
				dev_err(&pdev->dev, "failed to map npu power registers\n");
				return -ENXIO;
			}
		}
	}

	if (!fwnode_property_read_u64(fwnode, "dma-mask", &dma_mask))
		dev_info(dev, "%s get mask from DT : %#llx\n", __func__, dma_mask);
	else
		dma_mask = DMA_BIT_MASK(40);

	ret = dma_set_mask(dev, dma_mask);
	if (ret) {
		dev_err(dev, "%s failed to set dma mask\n", __func__);
		return ret;
	}

	ret = phytium_npu_register_dev(&pdev->dev, preg_addr, reg_addr);
	if (ret) {
		dev_err(&pdev->dev, "failed to initialize driver core!\n");
		goto out_add_dev;
	}

	npu_dev = dev_get_drvdata(dev);
	npu_dev->irq = irq;
	npu_dev->is_platform_dev = TRUE;
	npu_dev->clock_freq = 800000000;

	ret = devm_request_threaded_irq(&pdev->dev, irq, phytium_npu_handle_isr,
					phytium_npu_thread_irq, IRQF_SHARED,
					"phytium-npu", pdev);

	if (ret) {
		dev_err(&pdev->dev, "failed to request irq\n");
		goto out_irq;
	}
	pr_info("%s: registered irq: %d\n", __func__, irq);

	return ret;

out_irq:
	phytium_npu_unregister_dev(&pdev->dev);
out_add_dev:
	devm_iounmap(&pdev->dev, reg_addr);
	return ret;
}

static void phytium_npu_remove(struct platform_device *pdev)
{
	phytium_npu_unregister_dev(&pdev->dev);
}

#ifdef CONFIG_PM
static int phytium_npu_suspend(struct device *dev)
{
	return phytium_npu_common_suspend(dev);
}

static int phytium_npu_platform_resume(struct device *dev)
{
	return phytium_npu_common_resume(dev);
}

static int phytium_npu_runtime_idle(struct device *dev)
{
	/* Eg. turn off external clocks */
	return 0;
}

static int phytium_npu_runtime_suspend(struct device *dev)
{
	return phytium_npu_common_suspend(dev);
}

static int phytium_npu_platform_runtime_resume(struct device *dev)
{
	return phytium_npu_common_resume(dev);
}

#endif

static const struct dev_pm_ops phytium_npu_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_npu_suspend, phytium_npu_platform_resume)
	SET_RUNTIME_PM_OPS(phytium_npu_runtime_suspend,
			   phytium_npu_platform_runtime_resume,
						phytium_npu_runtime_idle)
};

static ssize_t info_show(struct device_driver *drv, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "NPU platform driver version: %s\n", NPU_FTN300_DRIVER_VERSION);
}

static DRIVER_ATTR_RO(info);
static struct attribute *drv_attrs[] = {
	&driver_attr_info.attr,
	NULL
};

ATTRIBUTE_GROUPS(drv);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_npu_acpi_ids[] = {
	{ "PHYT0050", 0 },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(acpi, phytium_npu_acpi_ids);
#endif
#ifdef CONFIG_OF
static const struct of_device_id phytium_npu_of_ids[] = {
	{ .compatible = "phytium,npu"},
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, phytium_npu_of_ids);
#endif

static struct platform_driver phytium_npu_plat_driver = {
	.probe  = phytium_npu_plat_probe,
	.remove = phytium_npu_remove,
	.driver = {
		.name = NPU_FTN300_PLAT_DRV_NAME,
		.groups = drv_groups,
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(phytium_npu_of_ids),
		.acpi_match_table = ACPI_PTR(phytium_npu_acpi_ids),
		.pm = &phytium_npu_pm_ops,
	},
};

module_platform_driver(phytium_npu_plat_driver);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium NPU driver for IO Mapped controllers");
MODULE_LICENSE("GPL");
MODULE_VERSION(NPU_FTN300_DRIVER_VERSION);
