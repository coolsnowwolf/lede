// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * BCM6318 PCIe Controller Driver
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <linux/clk.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/reset.h>
#include <linux/types.h>
#include <linux/vmalloc.h>

#include "../pci.h"

#define PCIE_BUS_BRIDGE			0
#define PCIE_BUS_DEVICE			1

#define PCIE_SPECIFIC_REG		0x188
#define SPECIFIC_ENDIAN_MODE_BAR1_SHIFT	0
#define SPECIFIC_ENDIAN_MODE_BAR1_MASK	(0x3 << SPECIFIC_ENDIAN_MODE_BAR1_SHIFT)
#define SPECIFIC_ENDIAN_MODE_BAR2_SHIFT	2
#define SPECIFIC_ENDIAN_MODE_BAR2_MASK	(0x3 << SPECIFIC_ENDIAN_MODE_BAR1_SHIFT)
#define SPECIFIC_ENDIAN_MODE_BAR3_SHIFT	4
#define SPECIFIC_ENDIAN_MODE_BAR3_MASK	(0x3 << SPECIFIC_ENDIAN_MODE_BAR1_SHIFT)
#define SPECIFIC_ENDIAN_MODE_WORD_ALIGN	0
#define SPECIFIC_ENDIAN_MODE_HALFWORD_ALIGN 1
#define SPECIFIC_ENDIAN_MODE_BYTE_ALIGN	2

#define PCIE_CONFIG2_REG		0x408
#define CONFIG2_BAR1_SIZE_EN		1
#define CONFIG2_BAR1_SIZE_MASK		0xf

#define PCIE_IDVAL3_REG			0x43c
#define IDVAL3_CLASS_CODE_MASK		0xffffff
#define IDVAL3_SUBCLASS_SHIFT		8
#define IDVAL3_CLASS_SHIFT		16

#define PCIE_DLSTATUS_REG		0x1048
#define DLSTATUS_PHYLINKUP		(1 << 13)

#define PCIE_CPU_2_PCIE_MEM_WIN0_LO_REG	0x400c
#define C2P_MEM_WIN_ENDIAN_MODE_MASK	0x3
#define C2P_MEM_WIN_ENDIAN_NO_SWAP	0
#define C2P_MEM_WIN_ENDIAN_HALF_WORD_SWAP 1
#define C2P_MEM_WIN_ENDIAN_HALF_BYTE_SWAP 2
#define C2P_MEM_WIN_BASE_ADDR_SHIFT	20
#define C2P_MEM_WIN_BASE_ADDR_MASK	(0xfff << C2P_MEM_WIN_BASE_ADDR_SHIFT)

#define PCIE_RC_BAR1_CONFIG_LO_REG	0x402c
#define RC_BAR_CFG_LO_SIZE_256MB	0xd
#define RC_BAR_CFG_LO_MATCH_ADDR_SHIFT	20
#define RC_BAR_CFG_LO_MATCH_ADDR_MASK	(0xfff << RC_BAR_CFG_LO_MATCH_ADDR_SHIFT)

#define PCIE_CPU_2_PCIE_MEM_WIN0_BASELIMIT_REG 0x4070
#define C2P_BASELIMIT_LIMIT_SHIFT	20
#define C2P_BASELIMIT_LIMIT_MASK	(0xfff << C2P_BASELIMIT_LIMIT_SHIFT)
#define C2P_BASELIMIT_BASE_SHIFT	4
#define C2P_BASELIMIT_BASE_MASK		(0xfff << C2P_BASELIMIT_BASE_SHIFT)

#define PCIE_UBUS_BAR1_CFG_REMAP_REG	0x4088
#define BAR1_CFG_REMAP_OFFSET_SHIFT	20
#define BAR1_CFG_REMAP_OFFSET_MASK	(0xfff << BAR1_CFG_REMAP_OFFSET_SHIFT)
#define BAR1_CFG_REMAP_ACCESS_EN	1

#define PCIE_HARD_DEBUG_REG		0x4204
#define HARD_DEBUG_SERDES_IDDQ		(1 << 23)

#define PCIE_CPU_INT1_MASK_CLEAR_REG	0x830c
#define CPU_INT_PCIE_ERR_ATTN_CPU	(1 << 0)
#define CPU_INT_PCIE_INTA		(1 << 1)
#define CPU_INT_PCIE_INTB		(1 << 2)
#define CPU_INT_PCIE_INTC		(1 << 3)
#define CPU_INT_PCIE_INTD		(1 << 4)
#define CPU_INT_PCIE_INTR		(1 << 5)
#define CPU_INT_PCIE_NMI		(1 << 6)
#define CPU_INT_PCIE_UBUS		(1 << 7)
#define CPU_INT_IPI			(1 << 8)

#define PCIE_EXT_CFG_INDEX_REG		0x8400
#define EXT_CFG_FUNC_NUM_SHIFT		12
#define EXT_CFG_FUNC_NUM_MASK		(0x7 << EXT_CFG_FUNC_NUM_SHIFT)
#define EXT_CFG_DEV_NUM_SHIFT		15
#define EXT_CFG_DEV_NUM_MASK		(0xf << EXT_CFG_DEV_NUM_SHIFT)
#define EXT_CFG_BUS_NUM_SHIFT		20
#define EXT_CFG_BUS_NUM_MASK		(0xff << EXT_CFG_BUS_NUM_SHIFT)

#define PCIE_DEVICE_OFFSET		0x9000

struct bcm6318_pcie {
	void __iomem *base;
	int irq;
	struct clk *clk;
	struct clk *clk25;
	struct clk *clk_ubus;
	struct reset_control *reset;
	struct reset_control *reset_ext;
	struct reset_control *reset_core;
	struct reset_control *reset_hard;
};

static struct bcm6318_pcie bcm6318_pcie;

extern int bmips_pci_irq;

/*
 * swizzle 32bits data to return only the needed part
 */
static int postprocess_read(u32 data, int where, unsigned int size)
{
	u32 ret = 0;

	switch (size) {
	case 1:
		ret = (data >> ((where & 3) << 3)) & 0xff;
		break;
	case 2:
		ret = (data >> ((where & 3) << 3)) & 0xffff;
		break;
	case 4:
		ret = data;
		break;
	}

	return ret;
}

static int preprocess_write(u32 orig_data, u32 val, int where,
			    unsigned int size)
{
	u32 ret = 0;

	switch (size) {
	case 1:
		ret = (orig_data & ~(0xff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 2:
		ret = (orig_data & ~(0xffff << ((where & 3) << 3))) |
			(val << ((where & 3) << 3));
		break;
	case 4:
		ret = val;
		break;
	}

	return ret;
}

static int bcm6318_pcie_can_access(struct pci_bus *bus, int devfn)
{
	struct bcm6318_pcie *priv = &bcm6318_pcie;

	switch (bus->number) {
	case PCIE_BUS_BRIDGE:
		return PCI_SLOT(devfn) == 0;
	case PCIE_BUS_DEVICE:
		if (PCI_SLOT(devfn) == 0)
			return __raw_readl(priv->base + PCIE_DLSTATUS_REG)
					& DLSTATUS_PHYLINKUP;
		/* else, fall through */
	default:
		return false;
	}
}

static int bcm6318_pcie_read(struct pci_bus *bus, unsigned int devfn,
			     int where, int size, u32 *val)
{
	struct bcm6318_pcie *priv = &bcm6318_pcie;
	u32 data;
	u32 reg = where & ~3;

	if (!bcm6318_pcie_can_access(bus, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == PCIE_BUS_DEVICE)
		reg += PCIE_DEVICE_OFFSET;

	data = __raw_readl(priv->base + reg);
	*val = postprocess_read(data, where, size);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm6318_pcie_write(struct pci_bus *bus, unsigned int devfn,
			      int where, int size, u32 val)
{
	struct bcm6318_pcie *priv = &bcm6318_pcie;
	u32 data;
	u32 reg = where & ~3;

	if (!bcm6318_pcie_can_access(bus, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == PCIE_BUS_DEVICE)
		reg += PCIE_DEVICE_OFFSET;

	data = __raw_readl(priv->base + reg);
	data = preprocess_write(data, val, where, size);
	__raw_writel(data, priv->base + reg);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops bcm6318_pcie_ops = {
	.read = bcm6318_pcie_read,
	.write = bcm6318_pcie_write,
};

static struct resource bcm6318_pcie_io_resource;
static struct resource bcm6318_pcie_mem_resource;
static struct resource bcm6318_pcie_busn_resource;

static struct pci_controller bcm6318_pcie_controller = {
	.pci_ops = &bcm6318_pcie_ops,
	.io_resource = &bcm6318_pcie_io_resource,
	.mem_resource = &bcm6318_pcie_mem_resource,
	.busn_resource = &bcm6318_pcie_busn_resource,
};

static void bcm6318_pcie_reset(struct bcm6318_pcie *priv)
{
	u32 val;

	reset_control_deassert(priv->reset_hard);

	reset_control_assert(priv->reset);
	reset_control_assert(priv->reset_core);
	reset_control_assert(priv->reset_ext);
	mdelay(10);

	reset_control_deassert(priv->reset_ext);
	mdelay(10);

	reset_control_deassert(priv->reset);
	mdelay(10);

	val = __raw_readl(priv->base + PCIE_HARD_DEBUG_REG);
	val &= ~HARD_DEBUG_SERDES_IDDQ;
	__raw_writel(val, priv->base + PCIE_HARD_DEBUG_REG);
	mdelay(10);

	reset_control_deassert(priv->reset_core);
	mdelay(200);
}

static void bcm6318_pcie_setup(struct bcm6318_pcie *priv)
{
	u32 val;

	__raw_writel(CPU_INT_PCIE_INTA | CPU_INT_PCIE_INTB |
		     CPU_INT_PCIE_INTC | CPU_INT_PCIE_INTD,
		     priv->base + PCIE_CPU_INT1_MASK_CLEAR_REG);

	val = bcm6318_pcie_mem_resource.end & C2P_BASELIMIT_LIMIT_MASK;
	val |= (bcm6318_pcie_mem_resource.start >> C2P_BASELIMIT_LIMIT_SHIFT)
	       << C2P_BASELIMIT_BASE_SHIFT;
	__raw_writel(val, priv->base + PCIE_CPU_2_PCIE_MEM_WIN0_BASELIMIT_REG);

	/* setup class code as bridge */
	val = __raw_readl(priv->base + PCIE_IDVAL3_REG);
	val &= ~IDVAL3_CLASS_CODE_MASK;
	val |= (PCI_CLASS_BRIDGE_PCI << IDVAL3_SUBCLASS_SHIFT);
	__raw_writel(val, priv->base + PCIE_IDVAL3_REG);

	/* disable bar1 size */
	val = __raw_readl(priv->base + PCIE_CONFIG2_REG);
	val &= ~CONFIG2_BAR1_SIZE_MASK;
	__raw_writel(val, priv->base + PCIE_CONFIG2_REG);

	/* set bar0 to little endian */
	val = __raw_readl(priv->base + PCIE_CPU_2_PCIE_MEM_WIN0_LO_REG);
	val |= bcm6318_pcie_mem_resource.start & C2P_MEM_WIN_BASE_ADDR_MASK;
	val |= C2P_MEM_WIN_ENDIAN_HALF_BYTE_SWAP;
	__raw_writel(val, priv->base + PCIE_CPU_2_PCIE_MEM_WIN0_LO_REG);

	__raw_writel(SPECIFIC_ENDIAN_MODE_BYTE_ALIGN,
		     priv->base + PCIE_SPECIFIC_REG);

	__raw_writel(RC_BAR_CFG_LO_SIZE_256MB,
		     priv->base + PCIE_RC_BAR1_CONFIG_LO_REG);

	__raw_writel(BAR1_CFG_REMAP_ACCESS_EN,
		     priv->base + PCIE_UBUS_BAR1_CFG_REMAP_REG);

	__raw_writel(PCIE_BUS_DEVICE << EXT_CFG_BUS_NUM_SHIFT,
		     priv->base + PCIE_EXT_CFG_INDEX_REG);
}

static int bcm6318_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm6318_pcie *priv = &bcm6318_pcie;
	struct resource *res;
	int ret;

	of_pci_check_probe_only();

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->irq = platform_get_irq(pdev, 0);
	if (!priv->irq)
		return -ENODEV;

	bmips_pci_irq = priv->irq;

	priv->reset = devm_reset_control_get(dev, "pcie");
	if (IS_ERR(priv->reset))
		return PTR_ERR(priv->reset);

	priv->reset_ext = devm_reset_control_get(dev, "pcie-ext");
	if (IS_ERR(priv->reset_ext))
		return PTR_ERR(priv->reset_ext);

	priv->reset_core = devm_reset_control_get(dev, "pcie-core");
	if (IS_ERR(priv->reset_core))
		return PTR_ERR(priv->reset_core);

	priv->reset_hard = devm_reset_control_get(dev, "pcie-hard");
	if (IS_ERR(priv->reset_hard))
		return PTR_ERR(priv->reset_hard);

	priv->clk = devm_clk_get(dev, "pcie");
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	priv->clk25 = devm_clk_get(dev, "pcie25");
	if (IS_ERR(priv->clk25))
		return PTR_ERR(priv->clk25);

	priv->clk_ubus = devm_clk_get(dev, "pcie-ubus");
	if (IS_ERR(priv->clk_ubus))
		return PTR_ERR(priv->clk_ubus);

	ret = clk_prepare_enable(priv->clk);
	if (ret) {
		dev_err(dev, "could not enable clock\n");
		return ret;
	}

	ret = clk_prepare_enable(priv->clk25);
	if (ret) {
		dev_err(dev, "could not enable clock\n");
		return ret;
	}

	ret = clk_prepare_enable(priv->clk_ubus);
	if (ret) {
		dev_err(dev, "could not enable clock\n");
		return ret;
	}

	pci_load_of_ranges(&bcm6318_pcie_controller, np);
	if (!bcm6318_pcie_mem_resource.start)
		return -EINVAL;

	of_pci_parse_bus_range(np, &bcm6318_pcie_busn_resource);

	bcm6318_pcie_reset(priv);
	bcm6318_pcie_setup(priv);

	register_pci_controller(&bcm6318_pcie_controller);

	return 0;
}

static const struct of_device_id bcm6318_pcie_of_match[] = {
	{ .compatible = "brcm,bcm6318-pcie", },
	{ /* sentinel */ }
};

static struct platform_driver bcm6318_pcie_driver = {
	.probe = bcm6318_pcie_probe,
	.driver	= {
		.name = "bcm6318-pcie",
		.of_match_table = bcm6318_pcie_of_match,
	},
};

int __init bcm6318_pcie_init(void)
{
	int ret = platform_driver_register(&bcm6318_pcie_driver);
	if (ret)
		pr_err("pci-bcm6318: Error registering platform driver!\n");
	return ret;
}
late_initcall_sync(bcm6318_pcie_init);
