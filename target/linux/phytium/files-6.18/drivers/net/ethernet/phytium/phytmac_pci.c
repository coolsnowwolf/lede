// SPDX-License-Identifier: GPL-2.0-only
/*
 * Phytium GMAC PCI wrapper.
 *
 * Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd.
 *
 * Author: Wenting Song <songwenting@phytium.com>
 */

#include <linux/pci.h>
#include <linux/pci_ids.h>
#include "phytmac.h"
#include "phytmac_v1.h"
#include "phytmac_v2.h"

#define PCI_DEVICE_ID_GMAC					0xDC3B
#define PCI_SUBDEVICE_ID_SGMII				0x1000
#define PCI_SUBDEVICE_ID_1000BASEX			0x1001
#define PCI_SUBDEVICE_ID_2500BASEX			0x1002
#define PCI_SUBDEVICE_ID_5GBASER			0x1003
#define PCI_SUBDEVICE_ID_USXGMII			0x1004
#define PCI_SUBDEVICE_ID_10GBASER			0x1005

struct phytmac_data {
	struct phytmac_hw_if	*hw_if;
	u32			caps;
	u16			queue_num;
	int			speed;
	bool			duplex;
	bool			use_mii;
	bool			use_ncsi;
	phy_interface_t		interface;
	const struct property_entry *properties;
};

static const u32 fixedlink[][5] = {
	{0, 1, 1000, 1, 0},
	{0, 1, 2500, 1, 0},
	{0, 1, 5000, 1, 0},
	{0, 1, 10000, 1, 0},
};

static const struct property_entry fl_properties[][2] = {
	{PROPERTY_ENTRY_U32_ARRAY("fixed-link", fixedlink[0]), {} },
	{PROPERTY_ENTRY_U32_ARRAY("fixed-link", fixedlink[1]), {} },
	{PROPERTY_ENTRY_U32_ARRAY("fixed-link", fixedlink[2]), {} },
	{PROPERTY_ENTRY_U32_ARRAY("fixed-link", fixedlink[3]), {} },
};

static int phytmac_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct phytmac_data *data = (struct phytmac_data *)id->driver_data;
	struct phytmac *pdata;
	struct device *dev = &pdev->dev;
	void __iomem * const *iomap_table;
	struct fwnode_handle *fw_node = NULL;
	int bar_mask;
	int ret, i;

	pdata = phytmac_alloc_pdata(dev);
	if (IS_ERR(pdata)) {
		ret = PTR_ERR(pdata);
		goto err_alloc;
	}

	pdata->pcidev = pdev;
	pci_set_drvdata(pdev, pdata);

	ret = pcim_enable_device(pdev);
	if (ret) {
		dev_err(dev, "pcim_enable_device failed\n");
		goto err_pci_enable;
	}

	/* Obtain the mmio areas for the device */
	bar_mask = pci_select_bars(pdev, IORESOURCE_MEM);
	ret = pcim_iomap_regions(pdev, bar_mask, PHYTMAC_PCI_DRV_NAME);
	if (ret) {
		dev_err(dev, "pcim_iomap_regions failed\n");
		goto err_pci_enable;
	}

	iomap_table = pcim_iomap_table(pdev);
	if (!iomap_table) {
		dev_err(dev, "pcim_iomap_table failed\n");
		ret = -ENOMEM;
		goto err_pci_enable;
	}

	pdata->mac_regs = iomap_table[0];
	if (!pdata->mac_regs) {
		dev_err(dev, "xgmac ioremap failed\n");
		ret = -ENOMEM;
		goto err_pci_enable;
	}

	pdata->msg_regs = iomap_table[1];
	if (!pdata->msg_regs) {
		dev_err(dev, "xpcs ioremap failed\n");
		ret = -ENOMEM;
		goto err_pci_enable;
	}

	pci_set_master(pdev);

	/* para */
	pdata->dma_burst_length = DEFAULT_DMA_BURST_LENGTH;
	pdata->jumbo_len = DEFAULT_DMA_BURST_LENGTH;
	pdata->wol |= PHYTMAC_WAKE_MAGIC;
	pdata->use_ncsi = data->use_ncsi;
	pdata->use_mii = data->use_mii;
	pdata->phy_interface = data->interface;
	pdata->queues_num = data->queue_num;
	pdata->capacities = data->caps;
	pdata->hw_if = data->hw_if;

	if (!pdata->use_mii) {
		fw_node = fwnode_create_software_node(data->properties, NULL);
		if (IS_ERR(fw_node)) {
			dev_err(dev, "Failed to create software node\n");
			goto err_pci_enable;
		}
		pdata->dev->fwnode = fw_node;
	}

	/* irq */
	ret = pci_alloc_irq_vectors(pdev, pdata->queues_num, pdata->queues_num, PCI_IRQ_MSI);
	if (ret < 0) {
		pdata->irq_type = IRQ_TYPE_INTX;
		pdata->queue_irq[0] = pdev->irq;
	} else {
		pdata->irq_type = IRQ_TYPE_MSI;
		for (i = 0; i < pdata->queues_num; i++)
			pdata->queue_irq[i] = pci_irq_vector(pdev, i);
	}

	/* Configure the netdev resource */
	ret = phytmac_drv_probe(pdata);
	if (ret)
		goto err_irq_vectors;

	netdev_notice(pdata->ndev, "net device enabled\n");

	return 0;

err_irq_vectors:
	if (fw_node)
		fwnode_remove_software_node(fw_node);
	pci_free_irq_vectors(pdata->pcidev);

err_pci_enable:
	phytmac_free_pdata(pdata);

err_alloc:
	dev_notice(dev, "net device not enabled\n");

	return ret;
}

static void phytmac_pci_remove(struct pci_dev *pdev)
{
	struct phytmac *pdata = pci_get_drvdata(pdev);
	struct fwnode_handle *fw_node = dev_fwnode(pdata->dev);
	int i = 0;
	int bar_mask;

	if (fw_node) {
		fwnode_remove_software_node(fw_node);
		pdata->dev->fwnode = NULL;
	}

	phytmac_drv_remove(pdata);

	for (i = 0; i < pdata->queues_num; i++)
		free_irq(pci_irq_vector(pdev, i), &pdata->queues[i]);
	pci_free_irq_vectors(pdev);

	phytmac_free_pdata(pdata);
	bar_mask = pci_select_bars(pdev, IORESOURCE_MEM);
	for (i = 0; i < PCI_STD_NUM_BARS; i++)
		if (bar_mask & BIT(i))
			pcim_iounmap_region(pdev, i);

	pci_disable_device(pdev);
}

static int __maybe_unused phytmac_pci_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytmac *pdata = pci_get_drvdata(pdev);
	int ret;

	ret = phytmac_drv_suspend(pdata);

	return ret;
}

static int __maybe_unused phytmac_pci_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytmac *pdata = pci_get_drvdata(pdev);
	int ret;

	ret = phytmac_drv_resume(pdata);

	return ret;
}

struct phytmac_data phytmac_sgmii = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
	.use_ncsi = false,
	.use_mii = true,
	.interface = PHY_INTERFACE_MODE_SGMII,
};

struct phytmac_data phytmac_1000basex = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
	.use_ncsi = false,
	.use_mii = false,
	.speed = 1000,
	.duplex = true,
	.interface = PHY_INTERFACE_MODE_1000BASEX,
	.properties = fl_properties[0],
};

struct phytmac_data phytmac_2500basex = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
	.use_ncsi = false,
	.use_mii = false,
	.speed = 2500,
	.duplex = true,
	.interface = PHY_INTERFACE_MODE_2500BASEX,
	.properties = fl_properties[1],
};

struct phytmac_data phytmac_5000baser = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
	.use_ncsi = false,
	.use_mii = false,
	.speed = 5000,
	.duplex = true,
	.interface = PHY_INTERFACE_MODE_5GBASER,
	.properties = fl_properties[2],
};

struct phytmac_data phytmac_usxgmii = {
	.hw_if = &phytmac_1p0_hw,
	.caps = PHYTMAC_CAPS_TAILPTR
			| PHYTMAC_CAPS_START
			| PHYTMAC_CAPS_JUMBO
			| PHYTMAC_CAPS_LSO,
	.queue_num = 4,
	.use_ncsi = false,
	.use_mii = false,
	.speed = 10000,
	.duplex = true,
	.interface = PHY_INTERFACE_MODE_USXGMII,
	.properties = fl_properties[3],
};

static const struct pci_device_id phytmac_pci_table[] = {
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_SGMII),
		.driver_data = (kernel_ulong_t)&phytmac_sgmii},
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_1000BASEX),
		.driver_data = (kernel_ulong_t)&phytmac_1000basex},
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_2500BASEX),
		.driver_data = (kernel_ulong_t)&phytmac_2500basex},
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_5GBASER),
		.driver_data = (kernel_ulong_t)&phytmac_5000baser},
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_USXGMII),
		.driver_data = (kernel_ulong_t)&phytmac_usxgmii},
	{ PCI_DEVICE_SUB(PCI_VENDOR_ID_PHYTIUM, PCI_DEVICE_ID_GMAC,
				PCI_VENDOR_ID_PHYTIUM, PCI_SUBDEVICE_ID_10GBASER),
		.driver_data = (kernel_ulong_t)&phytmac_usxgmii},
	/* Last entry must be zero */
	{ 0, }
};
MODULE_DEVICE_TABLE(pci, phytmac_pci_table);

static const struct dev_pm_ops phytmac_pci_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phytmac_pci_suspend, phytmac_pci_resume)
};

static struct pci_driver phytmac_driver = {
	.name = PHYTMAC_PCI_DRV_NAME,
	.id_table = phytmac_pci_table,
	.probe = phytmac_pci_probe,
	.remove = phytmac_pci_remove,
	.driver = {
		.pm = &phytmac_pci_pm_ops,
	}
};

module_pci_driver(phytmac_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Phytium NIC PCI wrapper");
MODULE_VERSION(PHYTMAC_DRIVER_VERSION);
