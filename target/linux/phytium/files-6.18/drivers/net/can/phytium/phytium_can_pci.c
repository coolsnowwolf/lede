// SPDX-License-Identifier: GPL-2.0
/* Platform CAN bus driver for Phytium CAN controller
 *
 * Copyright (c) 2021-2024 Phytium Technology Co., Ltd.
 */

#include <linux/pci.h>

#include "phytium_can.h"

struct phytium_can_pci_config {
	const struct phytium_can_devtype *devtype;
	unsigned int clock_freq;
	unsigned int tx_fifo_depth;
};

#define cdev2priv(dev) container_of(dev, struct phytium_can_pci, cdev)

struct phytium_can_pci {
	struct phytium_can_dev cdev;

	void __iomem *base;
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

static const struct phytium_can_devtype phytium_can_pci = {
	.cantype = PHYTIUM_CAN,
	.bittiming_const = &phytium_bittiming_const_8192,
};

static const struct phytium_can_pci_config phytium_can_pci_data = {
	.devtype = &phytium_can_pci,
	.clock_freq = 480000000,
	.tx_fifo_depth = 64,
};

static int phytium_can_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	const struct phytium_can_pci_config *cfg;
	struct phytium_can_dev *cdev;
	struct phytium_can_pci *priv;
	int ret;

	cfg = (const struct phytium_can_pci_config *)id->driver_data;

	ret = pcim_enable_device(pdev);
	if (ret)
		goto err;

	ret = pcim_iomap_regions(pdev, 0x1, pci_name(pdev));
	if (ret)
		goto err;

	cdev = phytium_can_allocate_dev(&pdev->dev, sizeof(struct phytium_can_pci),
					cfg->tx_fifo_depth);
	if (!cdev)
		return -ENOMEM;

	priv = cdev2priv(cdev);
	priv->base = pcim_iomap_table(pdev)[0];

	cdev->dev = &pdev->dev;
	cdev->fdmode = cfg->devtype->cantype;
	cdev->bit_timing = cfg->devtype->bittiming_const;
	cdev->can.clock.freq = cfg->clock_freq;
	cdev->tx_fifo_depth = cfg->tx_fifo_depth;

	cdev->base = priv->base;
	cdev->net->irq = pdev->irq;

	pci_set_drvdata(pdev, cdev->net);

	if (!pm_runtime_enabled(cdev->dev))
		pm_runtime_enable(cdev->dev);
	ret = pm_runtime_get_sync(cdev->dev);
	if (ret < 0) {
		netdev_err(cdev->net, "%s: pm_runtime_get failed(%d)\n",
			   __func__, ret);
		goto err_pmdisable;
	}
	ret = phytium_can_register(cdev);
	if (ret)
		goto err;

	return 0;

err_pmdisable:
	pm_runtime_disable(&pdev->dev);
err:
	return ret;
}

static void phytium_can_pci_remove(struct pci_dev *pdev)
{
	struct net_device *dev = pci_get_drvdata(pdev);
	struct phytium_can_dev *cdev = netdev_priv(dev);

	pm_runtime_disable(cdev->dev);

	phytium_can_unregister(cdev);
	phytium_can_free_dev(cdev->net);
}

static __maybe_unused int phytium_can_pci_suspend(struct device *dev)
{
	return phytium_can_suspend(dev);
}

static __maybe_unused int phytium_can_pci_resume(struct device *dev)
{
	return phytium_can_resume(dev);
}

static SIMPLE_DEV_PM_OPS(phytium_can_pci_pm_ops,
			 phytium_can_pci_suspend, phytium_can_pci_resume);

static const struct pci_device_id phytium_can_pci_id_table[] = {
	{ PCI_VDEVICE(PHYTIUM, 0xdc2d), (kernel_ulong_t)&phytium_can_pci_data, },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(pci, phytium_can_pci_id_table);

static struct pci_driver phytium_can_pci_driver = {
	.name = KBUILD_MODNAME,
	.probe = phytium_can_pci_probe,
	.remove = phytium_can_pci_remove,
	.id_table = phytium_can_pci_id_table,
	.driver = {
		.pm = &phytium_can_pci_pm_ops,
	},
};

module_pci_driver(phytium_can_pci_driver);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium CAN driver for PCI-based controllers");
