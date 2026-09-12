// SPDX-License-Identifier: GPL-2.0-only
/*
 * Phytium GMAC Platform wrapper.
 *
 * Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd.
 *
 * Author: Wenting Song <songwenting@phytium.com>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_net.h>
#include <linux/acpi.h>
#include <linux/platform_device.h>
#include "phytmac.h"
#include "phytmac_v1.h"
#include "phytmac_v2.h"

static const struct phytmac_config phytium_1p0_config = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
};

static const struct phytmac_config phytium_2p0_config = {
	.hw_if = &phytmac_2p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_RXPTR
			| PHYTMAC_CAPS_LSO
			| PHYTMAC_CAPS_MSG
			| PHYTMAC_CAPS_JUMBO,
	.queue_num = 2,
};

#if defined(CONFIG_OF)
static const struct of_device_id phytmac_dt_ids[] = {
	{ .compatible = "phytium,gmac-1.0", .data = &phytium_1p0_config },
	{ .compatible = "phytium,gmac-2.0", .data = &phytium_2p0_config },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, phytmac_dt_ids);
#endif /* CONFIG_OF */

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytmac_acpi_ids[] = {
	{ .id = "PHYT0046", .driver_data = (kernel_ulong_t)&phytium_1p0_config },
	{ .id = "PHYT0056", .driver_data = (kernel_ulong_t)&phytium_2p0_config },
	{}
};

MODULE_DEVICE_TABLE(acpi, phytmac_acpi_ids);
#else
#define phytmac_acpi_ids NULL
#endif

static const char *phytmac_phy_modes(enum phytmac_interface interface)
{
	switch (interface) {
	case PHYTMAC_PHY_INTERFACE_MODE_NA:
		return "";
	case PHYTMAC_PHY_INTERFACE_MODE_INTERNAL:
		return "internal";
	case PHYTMAC_PHY_INTERFACE_MODE_MII:
		return "mii";
	case PHYTMAC_PHY_INTERFACE_MODE_GMII:
		return "gmii";
	case PHYTMAC_PHY_INTERFACE_MODE_SGMII:
		return "sgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_TBI:
		return "tbi";
	case PHYTMAC_PHY_INTERFACE_MODE_REVMII:
		return "rev-mii";
	case PHYTMAC_PHY_INTERFACE_MODE_RMII:
		return "rmii";
	case PHYTMAC_PHY_INTERFACE_MODE_RGMII:
		return "rgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_RGMII_ID:
		return "rgmii-id";
	case PHYTMAC_PHY_INTERFACE_MODE_RGMII_RXID:
		return "rgmii-rxid";
	case PHYTMAC_PHY_INTERFACE_MODE_RGMII_TXID:
		return "rgmii-txid";
	case PHYTMAC_PHY_INTERFACE_MODE_RTBI:
		return "rtbi";
	case PHYTMAC_PHY_INTERFACE_MODE_SMII:
		return "smii";
	case PHYTMAC_PHY_INTERFACE_MODE_XGMII:
		return "xgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_MOCA:
		return "moca";
	case PHYTMAC_PHY_INTERFACE_MODE_QSGMII:
		return "qsgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_TRGMII:
		return "trgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_100BASEX:
		return "100base-x";
	case PHYTMAC_PHY_INTERFACE_MODE_1000BASEX:
		return "1000base-x";
	case PHYTMAC_PHY_INTERFACE_MODE_2500BASEX:
		return "2500base-x";
	case PHYTMAC_PHY_INTERFACE_MODE_5GBASER:
		return "5gbase-r";
	case PHYTMAC_PHY_INTERFACE_MODE_RXAUI:
		return "rxaui";
	case PHYTMAC_PHY_INTERFACE_MODE_XAUI:
		return "xaui";
	case PHYTMAC_PHY_INTERFACE_MODE_10GBASER:
		return "10gbase-r";
	case PHYTMAC_PHY_INTERFACE_MODE_USXGMII:
		return "usxgmii";
	case PHYTMAC_PHY_INTERFACE_MODE_10GKR:
		return "10gbase-kr";
	default:
		return "unknown";
	}
}

static int phytmac_v2_get_phy_mode(struct platform_device *pdev)
{
	const char *pm;
	int err, i;
	int phy_interface;

	err = device_property_read_string(&pdev->dev, "phy-mode", &pm);
	if (err < 0)
		return err;

	phy_interface = PHYTMAC_PHY_INTERFACE_MODE_MAX + 1;
	for (i = 0; i < PHYTMAC_PHY_INTERFACE_MODE_MAX; i++) {
		if (!strcasecmp(pm, phytmac_phy_modes(i))) {
			phy_interface = i;
			dev_notice(&pdev->dev, "Phy mode is %s.\n", pm);
			break;
		}
	}

	if (phy_interface > PHYTMAC_PHY_INTERFACE_MODE_MAX) {
		dev_err(&pdev->dev, "Invalid phy mode value: %s!\n", pm);
		return -EINVAL;
	}

	return phy_interface;
}

static int phytmac_get_phy_mode(struct platform_device *pdev)
{
	const char *pm;
	int err, i;

	err = device_property_read_string(&pdev->dev, "phy-mode", &pm);
	if (err < 0)
		return err;

	for (i = 0; i < PHY_INTERFACE_MODE_MAX; i++) {
		if (!strcasecmp(pm, phy_modes(i)))
			return i;
	}

	return -ENODEV;
}

static int phytmac_plat_probe(struct platform_device *pdev)
{
	const struct phytmac_config *phytmac_config = &phytium_1p0_config;
	struct device_node *np = pdev->dev.of_node;
	struct resource *regs;
	struct phytmac *pdata;
	int ret, i;
	u32 queue_num;
	const struct of_device_id *match = NULL;
	const struct acpi_device_id *match_acpi = NULL;

	pdata = phytmac_alloc_pdata(&pdev->dev);
	if (IS_ERR(pdata)) {
		ret = PTR_ERR(pdata);
		goto err_alloc;
	}

	platform_set_drvdata(pdev, pdata);

	pdata->platdev = pdev;

	if (pdev->dev.of_node) {
		match = of_match_node(phytmac_dt_ids, np);
		if (match && match->data) {
			phytmac_config = match->data;
			pdata->hw_if = phytmac_config->hw_if;
			pdata->capacities = phytmac_config->caps;
			pdata->queues_max_num = phytmac_config->queue_num;
		}
	} else if (has_acpi_companion(&pdev->dev)) {
		match_acpi = acpi_match_device(phytmac_acpi_ids, &pdev->dev);
		if (match_acpi && match_acpi->driver_data) {
			phytmac_config = (void *)match_acpi->driver_data;
			pdata->hw_if = phytmac_config->hw_if;
			pdata->capacities = phytmac_config->caps;
			pdata->queues_max_num = phytmac_config->queue_num;
		}
	}

	i = 0;
	regs = platform_get_resource(pdev, IORESOURCE_MEM, i);
	pdata->mac_regs = phytmac_devm_ioremap_resource_np(&pdev->dev, regs);
	if (IS_ERR(pdata->mac_regs)) {
		dev_err(&pdev->dev, "mac_regs ioremap failed\n");
		ret = PTR_ERR(pdata->mac_regs);
		goto err_mem;
	}
	pdata->ndev->base_addr = regs->start;

	if (pdev->dev.of_node && match) {
		if (!strcmp(match->compatible, "phytium,gmac-1.0"))
			pdata->version = PHYTMAC_READ(pdata, PHYTMAC_VERSION) & 0xff;
		else
			pdata->version = VERSION_V3;
	} else if (has_acpi_companion(&pdev->dev) && match_acpi) {
		if (!strcmp(match_acpi->id, "PHYT0046"))
			pdata->version = PHYTMAC_READ(pdata, PHYTMAC_VERSION) & 0xff;
		else
			pdata->version = VERSION_V3;
	}

	if (pdata->capacities & PHYTMAC_CAPS_MSG) {
		++i;
		regs = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (regs) {
			pdata->msg_regs = ioremap_wt(regs->start, MEMORY_SIZE);
			if (!pdata->msg_regs) {
				dev_err(&pdev->dev, "msg_regs ioremap failed, i=%d\n", i);
				goto err_mem;
			}
		}
	}

	if (device_property_read_bool(&pdev->dev, "powerctrl"))
		pdata->capacities |= PHYTMAC_CAPS_PWCTRL;

	if (pdata->version == VERSION_V3 && pdev->dev.of_node) {
		++i;
		regs = platform_get_resource(pdev, IORESOURCE_MEM, i);
		if (regs) {
			pdata->mhu_regs = ioremap(regs->start, MHU_SIZE);
			if (!pdata->mhu_regs)
				dev_err(&pdev->dev, "mhu_regs ioremap failed, i=%d\n", i);
		}
	}

	if (device_property_read_u32(&pdev->dev, "dma-burst-length", &pdata->dma_burst_length))
		pdata->dma_burst_length = DEFAULT_DMA_BURST_LENGTH;

	if (device_property_read_u32(&pdev->dev, "jumbo-max-length", &pdata->jumbo_len))
		pdata->jumbo_len = DEFAULT_JUMBO_MAX_LENGTH;

	if (device_property_read_u32(&pdev->dev, "queue-number", &queue_num))
		pdata->queues_num = pdata->queues_max_num;
	else
		pdata->queues_num = queue_num;

	pdata->wol = 0;
	if (device_property_read_bool(&pdev->dev, "magic-packet"))
		pdata->wol |= PHYTMAC_WAKE_MAGIC;

	pdata->use_ncsi = device_property_read_bool(&pdev->dev, "use-ncsi");
	pdata->use_mii = device_property_read_bool(&pdev->dev, "use-mii");

	pdata->power_state = PHYTMAC_POWEROFF;

	device_set_wakeup_capable(&pdev->dev, pdata->wol & PHYTMAC_WOL_MAGIC_PACKET);

	for (i = 0; i < pdata->queues_num; i++) {
		pdata->irq_type = IRQ_TYPE_INT;
		pdata->queue_irq[i] = platform_get_irq(pdev, i);
	}

	ret = phytmac_get_phy_mode(pdev);
	if (ret < 0)
		pdata->phy_interface = PHY_INTERFACE_MODE_MII;
	else
		pdata->phy_interface = ret;

	if (pdata->version == VERSION_V3) {
		ret = phytmac_v2_get_phy_mode(pdev);
		if (ret < 0)
			pdata->phytmac_v2_interface = PHYTMAC_PHY_INTERFACE_MODE_USXGMII;
		else
			pdata->phytmac_v2_interface = ret;
	}

	ret = phytmac_drv_probe(pdata);
	if (ret)
		goto err_mem;

	if (netif_msg_probe(pdata)) {
		dev_notice(&pdev->dev, "phytium net device enabled\n");
		dev_dbg(pdata->dev, "use_ncsi:%d, use_mii:%d, wol:%d, queues_num:%d\n",
			pdata->use_ncsi, pdata->use_mii, pdata->wol, pdata->queues_num);
	}

	return 0;

err_mem:
	phytmac_free_pdata(pdata);

err_alloc:
	dev_err(&pdev->dev, "phytium net device not enabled\n");

	return ret;
}

static void phytmac_plat_remove(struct platform_device *pdev)
{
	struct phytmac *pdata = platform_get_drvdata(pdev);

	phytmac_drv_remove(pdata);
	phytmac_free_pdata(pdata);
}

static void phytmac_plat_shutdown(struct platform_device *pdev)
{
	struct phytmac *pdata = platform_get_drvdata(pdev);

	phytmac_drv_shutdown(pdata);
}

static int __maybe_unused phytmac_plat_suspend(struct device *dev)
{
	struct phytmac *pdata = dev_get_drvdata(dev);
	int ret;

	ret = phytmac_drv_suspend(pdata);

	return ret;
}

static int __maybe_unused phytmac_plat_resume(struct device *dev)
{
	struct phytmac *pdata = dev_get_drvdata(dev);
	int ret;

	ret = phytmac_drv_resume(pdata);

	return ret;
}

static const struct dev_pm_ops phytmac_plat_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytmac_plat_suspend, phytmac_plat_resume)
};

static struct platform_driver phytmac_driver = {
	.probe = phytmac_plat_probe,
	.remove = phytmac_plat_remove,
	.driver = {
		.name = PHYTMAC_PLAT_DRV_NAME,
		.of_match_table = of_match_ptr(phytmac_dt_ids),
		.acpi_match_table = phytmac_acpi_ids,
		.pm = &phytmac_plat_pm_ops,
	},
	.shutdown = phytmac_plat_shutdown,
};

module_platform_driver(phytmac_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Phytium Ethernet driver");
MODULE_AUTHOR("Wenting Song");
MODULE_ALIAS("platform:phytmac");
MODULE_VERSION(PHYTMAC_DRIVER_VERSION);
