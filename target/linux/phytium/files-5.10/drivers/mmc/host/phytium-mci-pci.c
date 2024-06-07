// SPDX-License-Identifier: GPL-2.0+
/*
 * Phytium Multimedia Card Interface PCI driver
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 *
 */

#include <linux/module.h>
#include <linux/irq.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include "phytium-mci.h"

static u32 sd_caps = MMC_CAP_SD_HIGHSPEED | MMC_CAP_WAIT_WHILE_BUSY | MMC_CAP_CMD23 |
		     MMC_CAP_4_BIT_DATA;
static u32 sd_caps2 = MMC_CAP2_NO_MMC;

static u32 emmc_caps = MMC_CAP_4_BIT_DATA | MMC_CAP_8_BIT_DATA | MMC_CAP_WAIT_WHILE_BUSY |
		       MMC_CAP_CMD23 | MMC_CAP_HW_RESET | MMC_CAP_MMC_HIGHSPEED |
		       MMC_CAP_NONREMOVABLE;
static u32 emmc_caps2 = MMC_CAP2_NO_SDIO | MMC_CAP2_NO_SD;

#define PCI_BAR_NO 0

#if defined CONFIG_PM && defined CONFIG_PM_SLEEP
static const struct dev_pm_ops phytium_mci_dev_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytium_mci_suspend,
				phytium_mci_resume)
	SET_RUNTIME_PM_OPS(phytium_mci_runtime_suspend,
			   phytium_mci_runtime_resume, NULL)
};
#else
#define phytium_mci_dev_pm_ops NULL
#endif

static int
phytium_mci_pci_probe(struct pci_dev *pdev, const struct pci_device_id *pid)
{
	struct phytium_mci_host *host;
	struct mmc_host *mmc;
	int ret;

	ret = pcim_enable_device(pdev);

	if (ret)
		return ret;
	pci_set_master(pdev);

	mmc = mmc_alloc_host(sizeof(struct phytium_mci_host), &pdev->dev);

	if (!mmc)
		return -ENOMEM;

	host = mmc_priv(mmc);

	pci_enable_msi(pdev);

	host->irq = pdev->irq;
	host->irq_flags = IRQF_SHARED;
	host->dev = &pdev->dev;
	ret = pcim_iomap_regions(pdev, 1 << PCI_BAR_NO, pci_name(pdev));

	if (ret) {
		dev_err(&pdev->dev, "I/O memory remapping failed\n");
		goto host_free;
	}

	host->base = pcim_iomap_table(pdev)[PCI_BAR_NO];
	host->is_use_dma = 1;
	host->is_device_x100 = 1;

	if (pdev->devfn == 2) {
		host->caps = emmc_caps;
		host->caps2 = emmc_caps2;
	} else {
		host->caps = sd_caps;
		host->caps2 = sd_caps2;
		mmc->f_max = 25000000; /* stable frequency */
	}

	host->mmc = mmc;
	host->clk_rate = MCI_CLK;

	dev_info(&pdev->dev, "%s %d: [bar %d] addr: 0x%llx size: 0x%llx km: 0x%llx devfn:%d\n",
		 __func__, __LINE__, PCI_BAR_NO, pci_resource_start(pdev, 0),
		 pci_resource_len(pdev, 0), (uint64_t)host->base, pdev->devfn);

	dev_dbg(&pdev->dev, "%s %d:irq:0x%x\n", __func__, __LINE__, host->irq);

	ret = phytium_mci_common_probe(host);

	if (ret == MCI_REALEASE_MEM) {
		ret = -ENOMEM;
		goto release_mem;
	} else if (ret) {
		goto release;
	}
	pci_set_drvdata(pdev, mmc);
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
	pci_disable_device(pdev);
	return ret;
}

static void phytium_mci_pci_remove(struct pci_dev *pdev)
{
	struct phytium_mci_host *host;
	struct mmc_host *mmc;

	mmc = pci_get_drvdata(pdev);
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

	del_timer(&host->hotplug_timer);

	mmc_remove_host(host->mmc);

	if (host->dma.adma_table) {
		dma_free_coherent(&pdev->dev,
				  MAX_BD_NUM * sizeof(struct phytium_adma2_64_desc),
				  host->dma.adma_table, host->dma.adma_addr);
	}
	phytium_mci_deinit_hw(host);
	mmc_free_host(mmc);
	pci_set_drvdata(pdev, NULL);
}

static const struct pci_device_id phytium_mci_pci_tbl[] = {
	{
		PCI_DEVICE(PCI_VENDOR_ID_PHYTIUM, 0xdc28),
		.class		= 0x5,
		.class_mask	= 0,
	},
	{}
};
MODULE_DEVICE_TABLE(pci, phytium_mci_pci_tbl);

static struct pci_driver phytium_mci_pci_driver = {
	.name		= "phytium-mci-pci",
	.id_table	= phytium_mci_pci_tbl,
	.probe		= phytium_mci_pci_probe,
	.remove		= phytium_mci_pci_remove,
	.driver	= {
		.pm	= &phytium_mci_dev_pm_ops,
	}
};
module_pci_driver(phytium_mci_pci_driver);

MODULE_DESCRIPTION("Phytium Multimedia Card Interface PCI driver");
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
