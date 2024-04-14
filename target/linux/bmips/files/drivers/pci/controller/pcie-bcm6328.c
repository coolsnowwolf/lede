// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * BCM6328 PCIe Controller Driver
 *
 * Copyright (C) 2020 Álvaro Fernández Rojas <noltari@gmail.com>
 * Copyright (C) 2015 Jonas Gorski <jonas.gorski@gmail.com>
 * Copyright (C) 2008 Maxime Bizon <mbizon@freebox.fr>
 */

#include <linux/clk.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/mm.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/pci.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>
#include <linux/reset.h>
#include <linux/regmap.h>
#include <linux/types.h>
#include <linux/version.h>
#include <linux/vmalloc.h>

#include "../pci.h"

#define SERDES_PCIE_EXD_EN		BIT(15)
#define SERDES_PCIE_EN			BIT(0)

#define PCIE_BUS_BRIDGE			0
#define PCIE_BUS_DEVICE			1

#define PCIE_CONFIG2_REG		0x408
#define CONFIG2_BAR1_SIZE_EN		1
#define CONFIG2_BAR1_SIZE_MASK		0xf

#define PCIE_IDVAL3_REG			0x43c
#define IDVAL3_CLASS_CODE_MASK		0xffffff
#define IDVAL3_SUBCLASS_SHIFT		8
#define IDVAL3_CLASS_SHIFT		16

#define PCIE_DLSTATUS_REG		0x1048
#define DLSTATUS_PHYLINKUP		(1 << 13)

#define PCIE_BRIDGE_OPT1_REG		0x2820
#define OPT1_RD_BE_OPT_EN		(1 << 7)
#define OPT1_RD_REPLY_BE_FIX_EN		(1 << 9)
#define OPT1_PCIE_BRIDGE_HOLE_DET_EN	(1 << 11)
#define OPT1_L1_INT_STATUS_MASK_POL	(1 << 12)

#define PCIE_BRIDGE_OPT2_REG		0x2824
#define OPT2_UBUS_UR_DECODE_DIS		(1 << 2)
#define OPT2_TX_CREDIT_CHK_EN		(1 << 4)
#define OPT2_CFG_TYPE1_BD_SEL		(1 << 7)
#define OPT2_CFG_TYPE1_BUS_NO_SHIFT	16
#define OPT2_CFG_TYPE1_BUS_NO_MASK	(0xff << OPT2_CFG_TYPE1_BUS_NO_SHIFT)

#define PCIE_BRIDGE_BAR0_BASEMASK_REG	0x2828
#define BASEMASK_REMAP_EN		(1 << 0)
#define BASEMASK_SWAP_EN		(1 << 1)
#define BASEMASK_MASK_SHIFT		4
#define BASEMASK_MASK_MASK		(0xfff << BASEMASK_MASK_SHIFT)
#define BASEMASK_BASE_SHIFT		20
#define BASEMASK_BASE_MASK		(0xfff << BASEMASK_BASE_SHIFT)

#define PCIE_BRIDGE_BAR0_REBASE_ADDR_REG 0x282c
#define REBASE_ADDR_BASE_SHIFT		20
#define REBASE_ADDR_BASE_MASK		(0xfff << REBASE_ADDR_BASE_SHIFT)

#define PCIE_BRIDGE_RC_INT_MASK_REG	0x2854
#define PCIE_RC_INT_A			(1 << 0)
#define PCIE_RC_INT_B			(1 << 1)
#define PCIE_RC_INT_C			(1 << 2)
#define PCIE_RC_INT_D			(1 << 3)

#define PCIE_DEVICE_OFFSET		0x8000

struct bcm6328_pcie {
	void __iomem *base;
	int irq;
	struct regmap *serdes;
	struct device **pm;
	struct device_link **link_pm;
	unsigned int num_pms;
	struct clk *clk;
	struct reset_control *reset;
	struct reset_control *reset_ext;
	struct reset_control *reset_core;
	struct reset_control *reset_hard;
};

static struct bcm6328_pcie bcm6328_pcie;

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

static int bcm6328_pcie_can_access(struct pci_bus *bus, int devfn)
{
	struct bcm6328_pcie *priv = &bcm6328_pcie;

	switch (bus->number) {
	case PCIE_BUS_BRIDGE:
		return PCI_SLOT(devfn) == 0;
	case PCIE_BUS_DEVICE:
		if (PCI_SLOT(devfn) == 0)
			return __raw_readl(priv->base + PCIE_DLSTATUS_REG)
			       & DLSTATUS_PHYLINKUP;
		fallthrough;
	default:
		return false;
	}
}

static int bcm6328_pcie_read(struct pci_bus *bus, unsigned int devfn,
			     int where, int size, u32 *val)
{
	struct bcm6328_pcie *priv = &bcm6328_pcie;
	u32 data;
	u32 reg = where & ~3;

	if (!bcm6328_pcie_can_access(bus, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == PCIE_BUS_DEVICE)
		reg += PCIE_DEVICE_OFFSET;

	data = __raw_readl(priv->base + reg);
	*val = postprocess_read(data, where, size);

	return PCIBIOS_SUCCESSFUL;
}

static int bcm6328_pcie_write(struct pci_bus *bus, unsigned int devfn,
			      int where, int size, u32 val)
{
	struct bcm6328_pcie *priv = &bcm6328_pcie;
	u32 data;
	u32 reg = where & ~3;

	if (!bcm6328_pcie_can_access(bus, devfn))
		return PCIBIOS_DEVICE_NOT_FOUND;

	if (bus->number == PCIE_BUS_DEVICE)
		reg += PCIE_DEVICE_OFFSET;

	data = __raw_readl(priv->base + reg);
	data = preprocess_write(data, val, where, size);
	__raw_writel(data, priv->base + reg);

	return PCIBIOS_SUCCESSFUL;
}

static struct pci_ops bcm6328_pcie_ops = {
	.read = bcm6328_pcie_read,
	.write = bcm6328_pcie_write,
};

static struct resource bcm6328_pcie_io_resource;
static struct resource bcm6328_pcie_mem_resource;
static struct resource bcm6328_pcie_busn_resource;

static struct pci_controller bcm6328_pcie_controller = {
	.pci_ops = &bcm6328_pcie_ops,
	.io_resource = &bcm6328_pcie_io_resource,
	.mem_resource = &bcm6328_pcie_mem_resource,
};

static void bcm6328_pcie_reset(struct bcm6328_pcie *priv)
{
	regmap_write_bits(priv->serdes, 0,
			  SERDES_PCIE_EXD_EN | SERDES_PCIE_EN,
			  SERDES_PCIE_EXD_EN | SERDES_PCIE_EN);

	reset_control_assert(priv->reset);
	reset_control_assert(priv->reset_core);
	reset_control_assert(priv->reset_ext);
	if (priv->reset_hard) {
		reset_control_assert(priv->reset_hard);
		mdelay(10);
		reset_control_deassert(priv->reset_hard);
	}
	mdelay(10);

	reset_control_deassert(priv->reset_core);
	reset_control_deassert(priv->reset);
	mdelay(10);

	reset_control_deassert(priv->reset_ext);
	mdelay(200);
}

static void bcm6328_pcie_setup(struct bcm6328_pcie *priv)
{
	u32 val;

	val = __raw_readl(priv->base + PCIE_BRIDGE_OPT1_REG);
	val |= OPT1_RD_BE_OPT_EN;
	val |= OPT1_RD_REPLY_BE_FIX_EN;
	val |= OPT1_PCIE_BRIDGE_HOLE_DET_EN;
	val |= OPT1_L1_INT_STATUS_MASK_POL;
	__raw_writel(val, priv->base + PCIE_BRIDGE_OPT1_REG);

	val = __raw_readl(priv->base + PCIE_BRIDGE_RC_INT_MASK_REG);
	val |= PCIE_RC_INT_A;
	val |= PCIE_RC_INT_B;
	val |= PCIE_RC_INT_C;
	val |= PCIE_RC_INT_D;
	__raw_writel(val, priv->base + PCIE_BRIDGE_RC_INT_MASK_REG);

	val = __raw_readl(priv->base + PCIE_BRIDGE_OPT2_REG);
	/* enable credit checking and error checking */
	val |= OPT2_TX_CREDIT_CHK_EN;
	val |= OPT2_UBUS_UR_DECODE_DIS;
	/* set device bus/func for the pcie device */
	val |= (PCIE_BUS_DEVICE << OPT2_CFG_TYPE1_BUS_NO_SHIFT);
	val |= OPT2_CFG_TYPE1_BD_SEL;
	__raw_writel(val, priv->base + PCIE_BRIDGE_OPT2_REG);

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
	val = (bcm6328_pcie_mem_resource.start >> 20)
	      << BASEMASK_BASE_SHIFT;
	val |= (bcm6328_pcie_mem_resource.end >> 20) << BASEMASK_MASK_SHIFT;
	val |= BASEMASK_REMAP_EN;
	__raw_writel(val, priv->base + PCIE_BRIDGE_BAR0_BASEMASK_REG);

	val = (bcm6328_pcie_mem_resource.start >> 20)
	      << REBASE_ADDR_BASE_SHIFT;
	__raw_writel(val, priv->base + PCIE_BRIDGE_BAR0_REBASE_ADDR_REG);
}

static int bcm6328_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;
	struct bcm6328_pcie *priv = &bcm6328_pcie;
	struct resource *res;
	unsigned int i;
	int ret;
	LIST_HEAD(resources);

	pm_runtime_enable(dev);
	pm_runtime_no_callbacks(dev);

	priv->num_pms = of_count_phandle_with_args(np, "power-domains",
						   "#power-domain-cells");
	if (priv->num_pms > 1) {
		priv->pm = devm_kcalloc(dev, priv->num_pms,
					sizeof(struct device *), GFP_KERNEL);
		if (!priv->pm)
			return -ENOMEM;

		priv->link_pm = devm_kcalloc(dev, priv->num_pms,
					     sizeof(struct device_link *),
					     GFP_KERNEL);
		if (!priv->link_pm)
			return -ENOMEM;

		for (i = 0; i < priv->num_pms; i++) {
			priv->pm[i] = genpd_dev_pm_attach_by_id(dev, i);
			if (IS_ERR(priv->pm[i])) {
				dev_err(dev, "error getting pm %d\n", i);
				return -EINVAL;
			}

			priv->link_pm[i] = device_link_add(dev, priv->pm[i],
				DL_FLAG_STATELESS | DL_FLAG_PM_RUNTIME |
				DL_FLAG_RPM_ACTIVE);
		}
	}

	ret = pm_runtime_get_sync(dev);
	if (ret < 0) {
		pm_runtime_disable(dev);
		dev_info(dev, "PM prober defer: ret=%d\n", ret);
		return -EPROBE_DEFER;
	}

	of_pci_check_probe_only();

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	priv->base = devm_ioremap_resource(dev, res);
	if (IS_ERR(priv->base))
		return PTR_ERR(priv->base);

	priv->irq = platform_get_irq(pdev, 0);
	if (!priv->irq)
		return -ENODEV;

	bmips_pci_irq = priv->irq;

	priv->serdes = syscon_regmap_lookup_by_phandle(np, "brcm,serdes");
	if (IS_ERR(priv->serdes))
		return PTR_ERR(priv->serdes);

	priv->reset = devm_reset_control_get(dev, "pcie");
	if (IS_ERR(priv->reset))
		return PTR_ERR(priv->reset);

	priv->reset_ext = devm_reset_control_get(dev, "pcie-ext");
	if (IS_ERR(priv->reset_ext))
		return PTR_ERR(priv->reset_ext);

	priv->reset_core = devm_reset_control_get(dev, "pcie-core");
	if (IS_ERR(priv->reset_core))
		return PTR_ERR(priv->reset_core);

	priv->reset_hard = devm_reset_control_get_optional(dev, "pcie-hard");
	if (IS_ERR(priv->reset_hard))
		return PTR_ERR(priv->reset_hard);

	priv->clk = devm_clk_get(dev, "pcie");
	if (IS_ERR(priv->clk))
		return PTR_ERR(priv->clk);

	ret = clk_prepare_enable(priv->clk);
	if (ret) {
		dev_err(dev, "could not enable clock\n");
		return ret;
	}

	pci_load_of_ranges(&bcm6328_pcie_controller, np);
	if (!bcm6328_pcie_mem_resource.start)
		return -EINVAL;

	of_pci_parse_bus_range(np, &bcm6328_pcie_busn_resource);
	pci_add_resource(&resources, &bcm6328_pcie_busn_resource);

	bcm6328_pcie_reset(priv);
	bcm6328_pcie_setup(priv);

	register_pci_controller(&bcm6328_pcie_controller);

	return 0;
}

static const struct of_device_id bcm6328_pcie_of_match[] = {
	{ .compatible = "brcm,bcm6328-pcie", },
	{ /* sentinel */ }
};

static struct platform_driver bcm6328_pcie_driver = {
	.probe = bcm6328_pcie_probe,
	.driver	= {
		.name = "bcm6328-pcie",
		.of_match_table = bcm6328_pcie_of_match,
	},
};

int __init bcm6328_pcie_init(void)
{
	int ret = platform_driver_register(&bcm6328_pcie_driver);
	if (ret)
		pr_err("pci-bcm6328: Error registering platform driver!\n");
	return ret;
}
late_initcall_sync(bcm6328_pcie_init);
