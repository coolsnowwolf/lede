// SPDX-License-Identifier: GPL-2.0
/*
 * Phytium SoC mailbox driver
 *
 * Copyright (c) 2020-2023 Phytium Technology Co., Ltd.
 */

#include <linux/acpi.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mailbox_controller.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#define INTR_STAT	0x0
#define INTR_SET	0x8
#define INTR_CLR	0x10

#define TX_REG		0x100

#define NR_CHANS	1

static spinlock_t lock;

struct phytium_mbox_link {
	unsigned int irq;
	void __iomem *tx_reg;
	void __iomem *rx_reg;
};

struct phytium_mbox {
	void __iomem *base;
	struct phytium_mbox_link mlink;
	struct mbox_chan chan;
	struct mbox_controller mbox;
};

static irqreturn_t phytium_mbox_rx_irq(int irq, void *ch)
{
	struct mbox_chan *chan = ch;
	struct phytium_mbox_link *mlink = chan->con_priv;
	u32 val;
	unsigned long flags = 0;

	spin_lock_irqsave(&lock, flags);
	val = readl_relaxed(mlink->rx_reg + INTR_STAT);
	spin_unlock_irqrestore(&lock, flags);
	if (!val)
		return IRQ_NONE;

	mbox_chan_received_data(chan, (void *)&val);

	spin_lock_irqsave(&lock, flags);
	writel_relaxed(val, mlink->rx_reg + INTR_CLR);
	spin_unlock_irqrestore(&lock, flags);

	return IRQ_HANDLED;
}

static int phytium_mbox_send_data(struct mbox_chan *chan, void *data)
{
	struct phytium_mbox_link *mlink = chan->con_priv;
	u32 *arg = data;
	unsigned long flags = 0;

	spin_lock_irqsave(&lock, flags);
	writel_relaxed(*arg, mlink->tx_reg + INTR_SET);
	spin_unlock_irqrestore(&lock, flags);

	return 0;
}

static int phytium_mbox_startup(struct mbox_chan *chan)
{
	struct phytium_mbox_link *mlink = chan->con_priv;
	u32 val;
	int ret;
	unsigned long flags = 0;

	spin_lock_irqsave(&lock, flags);
	val = readl_relaxed(mlink->tx_reg + INTR_STAT);
	writel_relaxed(val, mlink->tx_reg + INTR_CLR);
	spin_unlock_irqrestore(&lock, flags);

	ret = request_irq(mlink->irq, phytium_mbox_rx_irq,
			  IRQF_SHARED, "phytium_mbox_link", chan);
	if (ret) {
		dev_err(chan->mbox->dev,
			"Unable to acquire IRQ %d\n", mlink->irq);
	}

	return ret;
}

static void phytium_mbox_shutdown(struct mbox_chan *chan)
{
	struct phytium_mbox_link *mlink = chan->con_priv;

	free_irq(mlink->irq, chan);
}

static bool phytium_mbox_last_tx_done(struct mbox_chan *chan)
{
	unsigned long flags;
	struct phytium_mbox_link *mlink = chan->con_priv;
	u32 val;

	spin_lock_irqsave(&lock, flags);
	val = readl_relaxed(mlink->tx_reg + INTR_STAT);
	spin_unlock_irqrestore(&lock, flags);

	return (val == (u32)(1U << 31));
}

static const struct mbox_chan_ops phytium_mbox_ops = {
	.send_data = phytium_mbox_send_data,
	.startup = phytium_mbox_startup,
	.shutdown = phytium_mbox_shutdown,
	.last_tx_done = phytium_mbox_last_tx_done,
};

#ifdef CONFIG_ACPI
static const struct acpi_device_id phytium_mbox_acpi_match[] = {
	{ "PHYT0009", 0 },
	{ },
};
MODULE_DEVICE_TABLE(acpi, phytium_mbox_acpi_match);
#endif

static const struct of_device_id phytium_mbox_of_match[] = {
	{ .compatible = "phytium,mbox", },
	{ },
};
MODULE_DEVICE_TABLE(of, phytium_mbox_of_match);

static int phytium_mbox_probe(struct platform_device *pdev)
{
	struct phytium_mbox *mbox;
	struct resource *res;
	int err, irq;

	spin_lock_init(&lock);

	/* Allocate memory for device */
	mbox = devm_kzalloc(&pdev->dev, sizeof(*mbox), GFP_KERNEL);
	if (!mbox)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mbox->base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(mbox->base)) {
		dev_err(&pdev->dev, "ioremap base failed\n");
		return PTR_ERR(mbox->base);
	}

	irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "cannot obtain irq\n");
		return irq;
	}

	mbox->chan.con_priv = &mbox->mlink;
	mbox->mlink.irq = irq;
	mbox->mlink.rx_reg = mbox->base;
	mbox->mlink.tx_reg = mbox->mlink.rx_reg + TX_REG;

	mbox->mbox.dev = &pdev->dev;
	mbox->mbox.chans = &mbox->chan;
	mbox->mbox.num_chans = NR_CHANS;
	mbox->mbox.ops = &phytium_mbox_ops;
	mbox->mbox.txdone_irq = false;
	mbox->mbox.txdone_poll = true;
	mbox->mbox.txpoll_period = 1;

	platform_set_drvdata(pdev, mbox);

	err = mbox_controller_register(&mbox->mbox);
	if (err) {
		dev_err(&pdev->dev, "Failed to register mailboxes %d\n", err);
		goto fail;
	}

	dev_info(&pdev->dev, "Phytium SoC Mailbox registered\n");
fail:
	return err;
}

static int phytium_mbox_remove(struct platform_device *pdev)
{
	struct phytium_mbox *mbox = platform_get_drvdata(pdev);

	mbox_controller_unregister(&mbox->mbox);

	return 0;
}

static struct platform_driver phytium_mbox_driver = {
	.probe = phytium_mbox_probe,
	.remove = phytium_mbox_remove,
	.driver = {
		.name = "phytium-mbox",
		.of_match_table = of_match_ptr(phytium_mbox_of_match),
		.acpi_match_table = ACPI_PTR(phytium_mbox_acpi_match),
	},
};

module_platform_driver(phytium_mbox_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Phytium SoC Mailbox Driver");
MODULE_AUTHOR("Chen Baozi <chenbaozi@phytium.com.cn>");
