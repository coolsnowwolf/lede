// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * PCI driver for Phytium I2C adapter.
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/errno.h>
#include <linux/i2c.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/slab.h>

#include "i2c-phytium-core.h"

#define DRV_NAME "i2c-phytium-pci"

enum phytium_pci_ctl_id_t {
	octopus_i2c,
};

struct scl_sda_cfg {
	u32 ss_hcnt;
	u32 fs_hcnt;
	u32 ss_lcnt;
	u32 fs_lcnt;
	u32 sda_hold;
};

struct phytium_pci_i2c {
	u32 bus_num;
	u32 bus_cfg;
	u32 tx_fifo_depth;
	u32 rx_fifo_depth;
	u32 clk_khz;
	u32 functionality;
	u32 flags;
	struct scl_sda_cfg *scl_sda_cfg;
	int (*setup)(struct pci_dev *pdev, struct phytium_pci_i2c *c);
};

/* Octopus HCNT/LCNT/SDA hold time */
static struct scl_sda_cfg octopus_config = {
	.ss_hcnt = 0x190,
	.ss_lcnt = 0x1d6,
	.fs_hcnt = 0x3c,
	.fs_lcnt = 0x82,
	.sda_hold = 0x0, // XXX
};

static int octopus_setup(struct pci_dev *pdev, struct phytium_pci_i2c *c)
{
	struct phytium_i2c_dev *i2c = pci_get_drvdata(pdev);
	struct i2c_client *ara;

	if (pdev->device == 0xdc32) {
		/*
		 * Since we have already register the adapter, the dev->irq
		 * must be valid.
		 */
		i2c->alert_data.irq = i2c->irq;

		ara = i2c_new_smbus_alert_device(&i2c->adapter, &i2c->alert_data);
		if (IS_ERR(ara))
			return PTR_ERR(ara);

		i2c->ara = ara;
	}

	return 0;
}

static struct phytium_pci_i2c pci_ctrl_info[] = {
	[octopus_i2c] = {
		.bus_num	= -1,
		.bus_cfg	= IC_CON_MASTER | IC_CON_SLAVE_DISABLE |
				  IC_CON_RESTART_EN | IC_CON_SPEED_FAST,
		.tx_fifo_depth	= 7,
		.rx_fifo_depth	= 7,
		.functionality	= I2C_FUNC_10BIT_ADDR,
		.clk_khz	= 48000000,
		.scl_sda_cfg	= &octopus_config,
		.setup		= octopus_setup,
	},
};

#ifdef CONFIG_PM
static int i2c_phytium_pci_suspend(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_i2c_dev *i_dev = pci_get_drvdata(pdev);

	i_dev->disable(i_dev);

	return 0;
}

static int i2c_phytium_pci_resume(struct device *dev)
{
	struct pci_dev *pdev = to_pci_dev(dev);
	struct phytium_i2c_dev *i_dev = pci_get_drvdata(pdev);

	return i_dev->init(i_dev);
}
#endif

static UNIVERSAL_DEV_PM_OPS(i2c_phytium_pm_ops, i2c_phytium_pci_suspend,
			    i2c_phytium_pci_resume, NULL);

static u32 i2c_phytium_get_clk_rate_khz(struct phytium_i2c_dev *dev)
{
	return dev->controller->clk_khz;
}

static int i2c_phytium_pci_probe(struct pci_dev *pdev,
			    const struct pci_device_id *id)
{
	struct phytium_i2c_dev *dev;
	struct i2c_adapter *adapter;
	struct phytium_pci_i2c *controller;
	struct scl_sda_cfg *cfg;
	int ret;

	if (id->driver_data >= ARRAY_SIZE(pci_ctrl_info)) {
		dev_err(&pdev->dev, "%s: invalid driver data %ld\n", __func__,
			id->driver_data);
		ret = -EINVAL;
		goto out;
	}

	controller = &pci_ctrl_info[id->driver_data];

	ret = pcim_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Failed to enable I2C PCI device (%d)\n", ret);
		goto out;
	}

	ret = pcim_iomap_regions(pdev, 0x1, pci_name(pdev));
	if (ret) {
		dev_err(&pdev->dev, "I/O memory remapping failed\n");
		goto out;
	}

	dev = devm_kzalloc(&pdev->dev, sizeof(struct phytium_i2c_dev), GFP_KERNEL);
	if (!dev) {
		ret = -ENOMEM;
		goto out;
	}

	dev->controller = controller;
	dev->get_clk_rate_khz = i2c_phytium_get_clk_rate_khz;
	dev->base = pcim_iomap_table(pdev)[0];
	dev->dev = &pdev->dev;
	dev->irq = pdev->irq;
	dev->flags |= controller->flags;

	dev->functionality = controller->functionality | IC_DEFAULT_FUNCTIONALITY;
	dev->master_cfg = controller->bus_cfg;
	if (controller->scl_sda_cfg) {
		cfg = controller->scl_sda_cfg;
		dev->ss_hcnt = cfg->ss_hcnt;
		dev->fs_hcnt = cfg->fs_hcnt;
		dev->ss_lcnt = cfg->ss_lcnt;
		dev->fs_lcnt = cfg->fs_lcnt;
		dev->sda_hold_time = cfg->sda_hold;
	}

	pci_set_drvdata(pdev, dev);

	dev->tx_fifo_depth = controller->tx_fifo_depth;
	dev->rx_fifo_depth = controller->rx_fifo_depth;

	adapter = &dev->adapter;
	adapter->owner = THIS_MODULE;
	adapter->class = 0;
	ACPI_COMPANION_SET(&adapter->dev, ACPI_COMPANION(&pdev->dev));
	adapter->nr = controller->bus_num;

	ret = i2c_phytium_probe(dev);
	if (ret)
		goto out;

	if (controller->setup) {
		ret = controller->setup(pdev, controller);
		if (ret)
			goto out;
	}

	pm_runtime_set_autosuspend_delay(&pdev->dev, 1000);
	pm_runtime_use_autosuspend(&pdev->dev);
	pm_runtime_put_autosuspend(&pdev->dev);
	pm_runtime_allow(&pdev->dev);

out:
	return ret;
}

static void i2c_phytium_pci_remove(struct pci_dev *pdev)
{
	struct phytium_i2c_dev *dev = pci_get_drvdata(pdev);

	dev->disable(dev);
	pm_runtime_forbid(&pdev->dev);
	pm_runtime_get_noresume(&pdev->dev);

	i2c_del_adapter(&dev->adapter);
}

static const struct pci_device_id i2_phytium_pci_ids[] = {
	{ PCI_DEVICE(0x1db7, 0xdc32), 0, 0, octopus_i2c },
	{ PCI_DEVICE(0x1db7, 0xdc30), 0, 0, octopus_i2c },
	{ }
};
MODULE_DEVICE_TABLE(pci, i2_phytium_pci_ids);

static struct pci_driver phytium_i2c_driver = {
	.name		= DRV_NAME,
	.id_table	= i2_phytium_pci_ids,
	.probe		= i2c_phytium_pci_probe,
	.remove		= i2c_phytium_pci_remove,
	.driver         = {
		.pm     = &i2c_phytium_pm_ops,
	},
};

module_pci_driver(phytium_i2c_driver);

MODULE_ALIAS("i2c-phytium-pci");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium PCI I2C bus adapter");
MODULE_LICENSE("GPL");
