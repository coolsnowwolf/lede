// SPDX-License-Identifier: GPL-2.0+
/*
 * Phytium PS/2 keyboard controller driver.
 *
 * Copyright (C) 2021-2023, Phytium Technology Co., Ltd.
 */

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/input.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/serio.h>
#include <linux/delay.h>

#define DRV_NAME	"phytium_ps2_pci"

#define REG_STAT		0x0
#define  REG_STAT_TX_TIMEOUT	0x1
#define  REG_STAT_RX_TIMEOUT	0x2
#define  REG_STAT_TX_FULL	0x4
#define REG_CTRL		0x4
#define  REG_CTRL_RESET	0x1
#define  REG_CTRL_TX_TIMEOUT	0x2
#define  REG_CTRL_RX_TIMEOUT	0x4
#define  REG_CTRL_RX_INTR	0x8
#define REG_INTR		0x8
#define  REG_INTR_TIMEOUT	0x1
#define  REG_INTR_RX		0x2
#define REG_TX			0xc
#define REG_RX			0x10
#define REG_TIMER_VAL		0x14

#define REG_CTRL_ENABLE		(REG_CTRL_TX_TIMEOUT|REG_CTRL_RX_TIMEOUT|REG_CTRL_RX_INTR)
#define REG_DATA_PARITY		0x100

#define STAT_RX_COUNTER(stat)	((stat >> 8) & 0x1f)

struct phytium_ps2_data {
	void __iomem *base;
	struct serio *io;
	struct pci_dev *dev;
};

static irqreturn_t phytium_ps2_irq(int irq, void *devid)
{
	struct phytium_ps2_data *ps2if = devid;
	u32 status, scancode, val = 0;
	unsigned int flag;
	int i, rxcount;

	status = readl(ps2if->base + REG_STAT);
	if (!status)
		return IRQ_NONE;

	/* Check if there is timeout interrupt */
	if (status & (REG_STAT_RX_TIMEOUT|REG_STAT_TX_TIMEOUT))
		val |= REG_INTR_TIMEOUT;

	rxcount = STAT_RX_COUNTER(status);
	for (i = 0; i < rxcount; i++) {
		scancode = readl(ps2if->base + REG_RX) & 0x1ff;

		if  (rxcount <= 16 && scancode != 0x1ff) {
			flag = ((scancode & REG_DATA_PARITY) ? SERIO_PARITY : 0);
			serio_interrupt(ps2if->io, scancode & 0xff, flag);
		}
	}

	val |= REG_INTR_RX;
	writel(val, ps2if->base + REG_INTR);

	return IRQ_HANDLED;
}

int phytium_ps2_write(struct serio *serio, unsigned char val)
{
	struct phytium_ps2_data *ps2if = serio->port_data;
	unsigned int stat;

	do {
		stat = readl(ps2if->base + REG_STAT);
		cpu_relax();
	} while (stat & REG_STAT_TX_FULL);

	writel(val, ps2if->base + REG_TX);

	return 0;
}

int phytium_ps2_open(struct serio *io)
{
	struct phytium_ps2_data *ps2if = io->port_data;

	writel(REG_CTRL_RESET, ps2if->base + REG_CTRL);
	/* Wait 4ms for the controller to be reset */
	usleep_range(4000, 6000);
	writel(REG_CTRL_ENABLE, ps2if->base + REG_CTRL);

	return 0;
}

void phytium_ps2_close(struct serio *io)
{
	struct phytium_ps2_data *ps2if = io->port_data;

	writel(0, ps2if->base + REG_CTRL);
}

static int phytium_pci_ps2_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	struct phytium_ps2_data *ps2if;
	struct serio *serio;
	int ret;

	ret = pcim_enable_device(pdev);
	if (ret)
		goto out;

	ret = pcim_iomap_regions(pdev, 0x1, DRV_NAME);
	if (ret)
		goto out;

	ps2if = devm_kzalloc(&pdev->dev, sizeof(struct phytium_ps2_data), GFP_KERNEL);
	serio = kzalloc(sizeof(struct serio), GFP_KERNEL);
	if (!ps2if || !serio) {
		ret = -ENOMEM;
		goto free;
	}

	serio->id.type = SERIO_8042;
	serio->write = phytium_ps2_write;
	serio->open = phytium_ps2_open;
	serio->close = phytium_ps2_close;
	strscpy(serio->name, pci_name(pdev), sizeof(serio->name));
	strscpy(serio->phys, dev_name(&pdev->dev), sizeof(serio->phys));
	serio->port_data = ps2if;
	serio->dev.parent = &pdev->dev;
	ps2if->io = serio;
	ps2if->dev = pdev;
	ps2if->base = pcim_iomap_table(pdev)[0];

	ret = devm_request_irq(&pdev->dev, pdev->irq, phytium_ps2_irq,
			       IRQF_SHARED, DRV_NAME, ps2if);
	if (ret) {
		dev_err(&pdev->dev, "could not request IRQ %d\n", pdev->irq);
		goto free;
	}

	pci_set_drvdata(pdev, ps2if);
	serio_register_port(ps2if->io);

	return 0;

free:
	kfree(serio);
out:
	return ret;
}

static void phytium_pci_ps2_remove(struct pci_dev *pdev)
{
	struct phytium_ps2_data *ps2if = pci_get_drvdata(pdev);

	serio_unregister_port(ps2if->io);
	pcim_iounmap_regions(pdev, 0x1);
}

static const struct pci_device_id phytium_pci_ps2_ids[] = {
	{ PCI_VDEVICE(PHYTIUM, 0xdc34) },
	{},
};
MODULE_DEVICE_TABLE(pci, phytium_pci_ps2_ids);

static struct pci_driver phytium_pci_ps2_driver = {
	.name			= DRV_NAME,
	.id_table		= phytium_pci_ps2_ids,
	.probe			= phytium_pci_ps2_probe,
	.remove			= phytium_pci_ps2_remove,
};
module_pci_driver(phytium_pci_ps2_driver);

MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium PCI PS/2 controller driver");
MODULE_LICENSE("GPL");
