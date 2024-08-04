/*
 * B53 register access through memory mapped registers
 *
 * Copyright (C) 2012-2013 Jonas Gorski <jogo@openwrt.org>
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

static int b53_mmap_read8(struct b53_device *dev, u8 page, u8 reg, u8 *val)
{
	u8 __iomem *regs = dev->priv;

	*val = readb(regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_read16(struct b53_device *dev, u8 page, u8 reg, u16 *val)
{
	u8 __iomem *regs = dev->priv;

	if (WARN_ON(reg % 2))
		return -EINVAL;

	if (dev->pdata && dev->pdata->big_endian)
		*val = readw_be(regs + (page << 8) + reg);
	else
		*val = readw(regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_read32(struct b53_device *dev, u8 page, u8 reg, u32 *val)
{
	u8 __iomem *regs = dev->priv;

	if (WARN_ON(reg % 4))
		return -EINVAL;

	if (dev->pdata && dev->pdata->big_endian)
		*val = readl_be(regs + (page << 8) + reg);
	else
		*val = readl(regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_read48(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	if (WARN_ON(reg % 2))
		return -EINVAL;

	if (reg % 4) {
		u16 lo;
		u32 hi;

		b53_mmap_read16(dev, page, reg, &lo);
		b53_mmap_read32(dev, page, reg + 2, &hi);

		*val = ((u64)hi << 16) | lo;
	} else {
		u32 lo;
		u16 hi;

		b53_mmap_read32(dev, page, reg, &lo);
		b53_mmap_read16(dev, page, reg + 4, &hi);

		*val = ((u64)hi << 32) | lo;
	}

	return 0;
}

static int b53_mmap_read64(struct b53_device *dev, u8 page, u8 reg, u64 *val)
{
	u32 hi, lo;

	if (WARN_ON(reg % 4))
		return -EINVAL;

	b53_mmap_read32(dev, page, reg, &lo);
	b53_mmap_read32(dev, page, reg + 4, &hi);

	*val = ((u64)hi << 32) | lo;

	return 0;
}

static int b53_mmap_write8(struct b53_device *dev, u8 page, u8 reg, u8 value)
{
	u8 __iomem *regs = dev->priv;

	writeb(value, regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_write16(struct b53_device *dev, u8 page, u8 reg,
			     u16 value)
{
	u8 __iomem *regs = dev->priv;

	if (WARN_ON(reg % 2))
		return -EINVAL;

	if (dev->pdata && dev->pdata->big_endian)
		writew_be(value, regs + (page << 8) + reg);
	else
		writew(value, regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_write32(struct b53_device *dev, u8 page, u8 reg,
				    u32 value)
{
	u8 __iomem *regs = dev->priv;

	if (WARN_ON(reg % 4))
		return -EINVAL;

	if (dev->pdata && dev->pdata->big_endian)
		writel_be(value, regs + (page << 8) + reg);
	else
		writel(value, regs + (page << 8) + reg);

	return 0;
}

static int b53_mmap_write48(struct b53_device *dev, u8 page, u8 reg,
				    u64 value)
{
	if (WARN_ON(reg % 2))
		return -EINVAL;

	if (reg % 4) {
		u32 hi = (u32)(value >> 16);
		u16 lo = (u16)value;

		b53_mmap_write16(dev, page, reg, lo);
		b53_mmap_write32(dev, page, reg + 2, hi);
	} else {
		u16 hi = (u16)(value >> 32);
		u32 lo = (u32)value;

		b53_mmap_write32(dev, page, reg, lo);
		b53_mmap_write16(dev, page, reg + 4, hi);
	}

	return 0;
}

static int b53_mmap_write64(struct b53_device *dev, u8 page, u8 reg,
			     u64 value)
{
	u32 hi, lo;

	hi = (u32)(value >> 32);
	lo = (u32)value;

	if (WARN_ON(reg % 4))
		return -EINVAL;

	b53_mmap_write32(dev, page, reg, lo);
	b53_mmap_write32(dev, page, reg + 4, hi);

	return 0;
}

static struct b53_io_ops b53_mmap_ops = {
	.read8 = b53_mmap_read8,
	.read16 = b53_mmap_read16,
	.read32 = b53_mmap_read32,
	.read48 = b53_mmap_read48,
	.read64 = b53_mmap_read64,
	.write8 = b53_mmap_write8,
	.write16 = b53_mmap_write16,
	.write32 = b53_mmap_write32,
	.write48 = b53_mmap_write48,
	.write64 = b53_mmap_write64,
};

static int b53_mmap_probe(struct platform_device *pdev)
{
	struct b53_platform_data *pdata = pdev->dev.platform_data;
	struct b53_device *dev;

	if (!pdata)
		return -EINVAL;

	dev = b53_swconfig_switch_alloc(&pdev->dev, &b53_mmap_ops, pdata->regs);
	if (!dev)
		return -ENOMEM;

	if (pdata)
		dev->pdata = pdata;

	platform_set_drvdata(pdev, dev);

	return b53_swconfig_switch_register(dev);
}

static int b53_mmap_remove(struct platform_device *pdev)
{
	struct b53_device *dev = platform_get_drvdata(pdev);

	if (dev)
		b53_switch_remove(dev);

	return 0;
}

static struct platform_driver b53_mmap_driver = {
	.probe = b53_mmap_probe,
	.remove = b53_mmap_remove,
	.driver = {
		.name = "b53-switch",
	},
};

module_platform_driver(b53_mmap_driver);
MODULE_AUTHOR("Jonas Gorski <jogo@openwrt.org>");
MODULE_DESCRIPTION("B53 MMAP access driver");
MODULE_LICENSE("Dual BSD/GPL");
