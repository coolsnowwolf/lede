/*
 * B53 register access through Switch Register Access Bridge Registers
 *
 * Copyright (C) 2013 Hauke Mehrtens <hauke@hauke-m.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/platform_data/b53.h>

#include "b53_priv.h"

/* command and status register of the SRAB */
#define B53_SRAB_CMDSTAT		0x2c
#define  B53_SRAB_CMDSTAT_RST		BIT(2)
#define  B53_SRAB_CMDSTAT_WRITE		BIT(1)
#define  B53_SRAB_CMDSTAT_GORDYN	BIT(0)
#define  B53_SRAB_CMDSTAT_PAGE		24
#define  B53_SRAB_CMDSTAT_REG		16

/* high order word of write data to switch registe */
#define B53_SRAB_WD_H			0x30

/* low order word of write data to switch registe */
#define B53_SRAB_WD_L			0x34

/* high order word of read data from switch register */
#define B53_SRAB_RD_H			0x38

/* low order word of read data from switch register */
#define B53_SRAB_RD_L			0x3c

/* command and status register of the SRAB */
#define B53_SRAB_CTRLS			0x40
#define  B53_SRAB_CTRLS_RCAREQ		BIT(3)
#define  B53_SRAB_CTRLS_RCAGNT		BIT(4)
#define  B53_SRAB_CTRLS_SW_INIT_DONE	BIT(6)

/* the register captures interrupt pulses from the switch */
#define B53_SRAB_INTR			0x44

static int b53_srab_request_grant(struct b53_device *dev)
{
	u8 __iomem *regs = dev->priv;
	u32 ctrls;
	int i;

	ctrls = readl(regs + B53_SRAB_CTRLS);
	ctrls |= B53_SRAB_CTRLS_RCAREQ;
	writel(ctrls, regs + B53_SRAB_CTRLS);

	for (i = 0; i < 20; i++) {
		ctrls = readl(regs + B53_SRAB_CTRLS);
		if (ctrls & B53_SRAB_CTRLS_RCAGNT)
			break;
		usleep_range(10, 100);
	}
	if (WARN_ON(i == 5))
		return -EIO;

	return 0;
}

static void b53_srab_release_grant(struct b53_device *dev)
{
	u8 __iomem *regs = dev->priv;
	u32 ctrls;

	ctrls = readl(regs + B53_SRAB_CTRLS);
	ctrls &= ~B53_SRAB_CTRLS_RCAREQ;
	writel(ctrls, regs + B53_SRAB_CTRLS);
}

static int b53_srab_op(struct b53_device *dev, u8 page, u8 reg, u32 op)
{
	int i;
	u32 cmdstat;
	u8 __iomem *regs = dev->priv;

	/* set register address */
	cmdstat = (page << B53_SRAB_CMDSTAT_PAGE) |
		  (reg << B53_SRAB_CMDSTAT_REG) |
		  B53_SRAB_CMDSTAT_GORDYN |
		  op;
	writel(cmdstat, regs + B53_SRAB_CMDSTAT);

	/* check if operation completed */
	for (i = 0; i < 5; ++i) {
		cmdstat = readl(regs + B53_SRAB_CMDSTAT);
		if (!(cmdstat & B53_SRAB_CMDSTAT_GORDYN))
			break;
		usleep_range(10, 100);
	}

	if (WARN_ON(i == 5))
		return -EIO;

	return 0;
}

static int b53_srab_read8(struct b53_device *dev, u8 page, u8 reg, u8 *val)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	ret = b53_srab_op(dev, page, reg, 0);
	if (ret)
		goto err;

	*val = readl(regs + B53_SRAB_RD_L) & 0xff;

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_read16(struct b53_device *dev, u8 page, u8 reg, u16 *val)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	ret = b53_srab_op(dev, page, reg, 0);
	if (ret)
		goto err;

	*val = readl(regs + B53_SRAB_RD_L) & 0xffff;

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_read32(struct b53_device *dev, u8 page, u8 reg, u32 *val)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	ret = b53_srab_op(dev, page, reg, 0);
	if (ret)
		goto err;

	*val = readl(regs + B53_SRAB_RD_L);

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_read48(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	ret = b53_srab_op(dev, page, reg, 0);
	if (ret)
		goto err;

	*val = readl(regs + B53_SRAB_RD_L);
	*val += ((u64)readl(regs + B53_SRAB_RD_H) & 0xffff) << 32;

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_read64(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	ret = b53_srab_op(dev, page, reg, 0);
	if (ret)
		goto err;

	*val = readl(regs + B53_SRAB_RD_L);
	*val += (u64)readl(regs + B53_SRAB_RD_H) << 32;

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_write8(struct b53_device *dev, u8 page, u8 reg, u8 value)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	writel(value, regs + B53_SRAB_WD_L);

	ret = b53_srab_op(dev, page, reg, B53_SRAB_CMDSTAT_WRITE);

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_write16(struct b53_device *dev, u8 page, u8 reg,
			     u16 value)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	writel(value, regs + B53_SRAB_WD_L);

	ret = b53_srab_op(dev, page, reg, B53_SRAB_CMDSTAT_WRITE);

err:
	b53_srab_release_grant(dev);

	return ret;
}

static int b53_srab_write32(struct b53_device *dev, u8 page, u8 reg,
				    u32 value)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	writel(value, regs + B53_SRAB_WD_L);

	ret = b53_srab_op(dev, page, reg, B53_SRAB_CMDSTAT_WRITE);

err:
	b53_srab_release_grant(dev);

	return ret;

}

static int b53_srab_write48(struct b53_device *dev, u8 page, u8 reg,
				    u64 value)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	writel((u32)value, regs + B53_SRAB_WD_L);
	writel((u16)(value >> 32), regs + B53_SRAB_WD_H);

	ret = b53_srab_op(dev, page, reg, B53_SRAB_CMDSTAT_WRITE);

err:
	b53_srab_release_grant(dev);

	return ret;

}

static int b53_srab_write64(struct b53_device *dev, u8 page, u8 reg,
			     u64 value)
{
	u8 __iomem *regs = dev->priv;
	int ret = 0;

	ret = b53_srab_request_grant(dev);
	if (ret)
		goto err;

	writel((u32)value, regs + B53_SRAB_WD_L);
	writel((u32)(value >> 32), regs + B53_SRAB_WD_H);

	ret = b53_srab_op(dev, page, reg, B53_SRAB_CMDSTAT_WRITE);

err:
	b53_srab_release_grant(dev);

	return ret;
}

static struct b53_io_ops b53_srab_ops = {
	.read8 = b53_srab_read8,
	.read16 = b53_srab_read16,
	.read32 = b53_srab_read32,
	.read48 = b53_srab_read48,
	.read64 = b53_srab_read64,
	.write8 = b53_srab_write8,
	.write16 = b53_srab_write16,
	.write32 = b53_srab_write32,
	.write48 = b53_srab_write48,
	.write64 = b53_srab_write64,
};

static int b53_srab_probe(struct platform_device *pdev)
{
	struct b53_platform_data *pdata = pdev->dev.platform_data;
	struct b53_device *dev;

	if (!pdata)
		return -EINVAL;

	dev = b53_swconfig_switch_alloc(&pdev->dev, &b53_srab_ops, pdata->regs);
	if (!dev)
		return -ENOMEM;

	if (pdata)
		dev->pdata = pdata;

	platform_set_drvdata(pdev, dev);

	return b53_swconfig_switch_register(dev);
}

static int b53_srab_remove(struct platform_device *pdev)
{
	struct b53_device *dev = platform_get_drvdata(pdev);

	if (dev)
		b53_switch_remove(dev);

	return 0;
}

static struct platform_driver b53_srab_driver = {
	.probe = b53_srab_probe,
	.remove = b53_srab_remove,
	.driver = {
		.name = "b53-srab-switch",
	},
};

module_platform_driver(b53_srab_driver);
MODULE_AUTHOR("Hauke Mehrtens <hauke@hauke-m.de>");
MODULE_DESCRIPTION("B53 Switch Register Access Bridge Registers (SRAB) access driver");
MODULE_LICENSE("Dual BSD/GPL");
