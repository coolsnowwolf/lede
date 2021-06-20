// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2020  Bjørn Mork <bjorn@mork.no>
 */
#include <linux/io.h>
#include <linux/module.h>

#define MODULE_NAME "mt7621-qtn-rgmii"
#define RGMII_REG_BASE	0x1e110008
#define RGMII_REG_SIZE	4
#define RGMII_REG_VALUE	0x9000c

static u32 oldval;

static int __init mt7621_qtn_rgmii_init(void)
{
	void __iomem *base = ioremap(RGMII_REG_BASE, RGMII_REG_SIZE);

	if (!base)
		return -ENOMEM;
	oldval = ioread32(base);
	if (oldval != RGMII_REG_VALUE) {
		iowrite32(RGMII_REG_VALUE, base);
		pr_info(MODULE_NAME ": changed register 0x%08x value from 0x%08x to 0x%08x\n", RGMII_REG_BASE, oldval,  RGMII_REG_VALUE);
	}
	iounmap(base);
	return 0;
}

static void __exit mt7621_qtn_rgmii_exit(void)
{
	void __iomem *base = ioremap(RGMII_REG_BASE, RGMII_REG_SIZE);

	if (!base)
		return;
	if (oldval != RGMII_REG_VALUE) {
		iowrite32(oldval, base);
		pr_info(MODULE_NAME ": reset register 0x%08x back to 0x%08x\n", RGMII_REG_BASE, oldval);
	}
	iounmap(base);
}

module_init(mt7621_qtn_rgmii_init);
module_exit(mt7621_qtn_rgmii_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Bjørn Mork <bjorn@mork.no>");
MODULE_DESCRIPTION("Enable RGMII connected Quantenna module on MT7621");
