// SPDX-License-Identifier: GPL-2.0+
/*
 * Phytium Multimedia Card Interface PCI driver
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/acpi.h>
#include <linux/dma-mapping.h>
#include "phytium-mci.h"

static u32 mci_caps = MMC_CAP_CMD23;

#if defined CONFIG_PM && defined CONFIG_PM_SLEEP

static const struct dev_pm_ops phytium_mci_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_mci_suspend,
				phytium_mci_resume)
	SET_RUNTIME_PM_OPS(phytium_mci_runtime_suspend,
			   phytium_mci_runtime_resume, NULL)
};
#else
static const struct dev_pm_ops phytium_mci_dev_pm_ops;
#endif

static int phytium_mci_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct phytium_mci_host *host;
	struct resource *res;
	const struct acpi_device_id *match;
	struct device *dev = &pdev->dev;
	int ret;

	mmc = mmc_alloc_host(sizeof(struct phytium_mci_host), &pdev->dev);
	if (!mmc)
		return -ENOMEM;
	host = mmc_priv(mmc);
	ret = mmc_of_parse(mmc);
	if (ret)
		goto host_free;

	if (device_property_read_bool(dev, "use-hold"))
		host->use_hold = 1;

	if (device_property_read_bool(dev, "clk-set"))
		host->clk_set = 1;

	if (host->clk_set) {
		host->clk_smpl_drv_25m = -1;
		host->clk_smpl_drv_50m = -1;
		host->clk_smpl_drv_66m = -1;
		host->clk_smpl_drv_100m = -1;
		device_property_read_u32(dev, "clk-smpl-drv-25m", &host->clk_smpl_drv_25m);
		device_property_read_u32(dev, "clk-smpl-drv-50m", &host->clk_smpl_drv_50m);
		device_property_read_u32(dev, "clk-smpl-drv-66m", &host->clk_smpl_drv_66m);
		device_property_read_u32(dev, "clk-smpl-drv-100m", &host->clk_smpl_drv_100m);
	}
	dev_info(dev, "mci clk set %d %d 0x%x 0x%x 0x%x 0x%x\n",
			host->use_hold, host->clk_set, host->clk_smpl_drv_25m,
			host->clk_smpl_drv_50m, host->clk_smpl_drv_66m, host->clk_smpl_drv_100m);

	if (dev->of_node) {
		host->src_clk = devm_clk_get(&pdev->dev, "phytium_mci_clk");
		if (IS_ERR(host->src_clk)) {
			ret = PTR_ERR(host->src_clk);
			goto host_free;
		}

		host->clk_rate = clk_get_rate(host->src_clk);
	} else if (has_acpi_companion(dev)) {
		match = acpi_match_device(dev->driver->acpi_match_table, dev);
		if (!match) {
			dev_err(dev, "Error ACPI match data is missing\n");
			return -ENODEV;
		}
		host->clk_rate = 1200000000;
	}

	host->is_use_dma = 1;
	host->is_device_x100 = 0;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	host->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(host->base)) {
		ret = PTR_ERR(host->base);
		goto host_free;
	}

	host->irq = platform_get_irq(pdev, 0);

	if (host->irq < 0) {
		ret = -EINVAL;
		goto host_free;
	}
	host->irq_flags = IRQF_SHARED;
	dev_dbg(&pdev->dev, "%s %d:irq:%d\n", __func__, __LINE__, host->irq);
	host->dev = &pdev->dev;
	host->caps = mci_caps;
	host->mmc = mmc;
	ret = phytium_mci_common_probe(host);
	if (ret == MCI_REALEASE_MEM) {
		ret = -ENOMEM;
		goto release_mem;
	} else if (ret) {
		goto release;
	}
	platform_set_drvdata(pdev, mmc);
	dev_info(&pdev->dev, "%s %d: probe phytium mci successful.\n", __func__, __LINE__);
	return 0;

release:
	phytium_mci_deinit_hw(host);
release_mem:
	if (host->dma.adma_table) {
		dma_free_coherent(&pdev->dev,
				  MAX_BD_NUM * sizeof(struct phytium_adma2_64_desc),
				  host->dma.adma_table, host->dma.adma_addr);
	}
host_free:
	mmc_free_host(mmc);
	return ret;
}

static void phytium_mci_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc;
	struct phytium_mci_host *host;

	mmc = platform_get_drvdata(pdev);
	if (!mmc) {
		dev_info(&pdev->dev, "%s %d: mmc is null.\n", __func__, __LINE__);
		return;
	}
	host = mmc_priv(mmc);
	if (!host) {
		dev_info(&pdev->dev, "%s %d: host is null.\n", __func__, __LINE__);
		mmc_remove_host(mmc);
		mmc_free_host(mmc);
		return;
	}
	timer_delete(&host->hotplug_timer);
	timer_delete_sync(&host->timeout_timer);
	mmc_remove_host(host->mmc);

	if (host->dma.adma_table) {
		dma_free_coherent(&pdev->dev,
				  MAX_BD_NUM * sizeof(struct phytium_adma2_64_desc),
				  host->dma.adma_table, host->dma.adma_addr);
	}
	phytium_mci_deinit_hw(host);
	mmc_free_host(mmc);
	platform_set_drvdata(pdev, NULL);
	return;
}

static const struct of_device_id phytium_mci_of_ids[] = {
	{ .compatible =  "phytium,mci", },
	{}
};

MODULE_DEVICE_TABLE(of, phytium_mci_of_ids);

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_mci_acpi_ids[] = {
	{ .id = "PHYT0017" },
	{ }
};

MODULE_DEVICE_TABLE(acpi, phytium_mci_acpi_ids);
#else
#define phytium_mci_acpi_ids NULL
#endif

static struct platform_driver phytium_mci_driver = {
	.probe = phytium_mci_probe,
	.remove = phytium_mci_remove,
	.driver = {
		.name = "phytium-mci-platform",
		.of_match_table = phytium_mci_of_ids,
		.acpi_match_table = phytium_mci_acpi_ids,
		.pm = pm_sleep_ptr(&phytium_mci_dev_pm_ops),
	},
};

module_platform_driver(phytium_mci_driver);

MODULE_DESCRIPTION("Phytium Multimedia Card Interface driver");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_VERSION(PHYTIUM_MMC_DRIVER_VERSION);
