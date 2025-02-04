// SPDX-License-Identifier: GPL-2.0
/*
 * Siflower SF21A6826/SF21H8898 PCIE driver
 *
 * Author: Chuanhong Guo <gch981213@gmail.com>
 */

#include <linux/phy/phy.h>
#include <linux/bitfield.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/resource.h>
#include <linux/signal.h>
#include <linux/types.h>
#include <linux/reset.h>

#include "pcie-designware.h"

#define SF_PCIE_MAX_TIMEOUT	10000

#define ELBI_REG0		0x0
#define  APP_LTSSM_ENABLE	BIT(23)

#define to_sf_pcie(x)	dev_get_drvdata((x)->dev)

#define SYSM_PCIE_SET		0x0
#define  PCIE_DEVTYPE_EP	0
#define  PCIE_DEVTYPE_RC	4

#define SYSM_PCIE_INIT		0x4
#define SYSM_PCIE_CLK_EN	0x9c

enum sf_pcie_regfield_ids {
	DEVICE_TYPE,
	PERST_N_OUT,
	PERST_N,
	BUTTON_RSTN,
	POWER_UP_RSTN,
	ACLK_M_EN,
	ACLK_S_EN,
	ACLK_C_EN,
	HCLK_EN,
	SYSM_REGFIELD_MAX,
};

static const struct reg_field pcie0_sysm_regs[SYSM_REGFIELD_MAX] = {
	[DEVICE_TYPE] = REG_FIELD(SYSM_PCIE_SET, 0, 3),
	[PERST_N_OUT] = REG_FIELD(SYSM_PCIE_INIT, 15, 15),
	[PERST_N] = REG_FIELD(SYSM_PCIE_INIT, 5, 5),
	[BUTTON_RSTN] = REG_FIELD(SYSM_PCIE_INIT, 4, 4),
	[POWER_UP_RSTN] = REG_FIELD(SYSM_PCIE_INIT, 3, 3),
	[ACLK_M_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 3, 3),
	[ACLK_S_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 2, 2),
	[ACLK_C_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 1, 1),
	[HCLK_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 0, 0),
};

static const struct reg_field pcie1_sysm_regs[SYSM_REGFIELD_MAX] = {
	[DEVICE_TYPE] = REG_FIELD(SYSM_PCIE_SET, 4, 7),
	[PERST_N_OUT] = REG_FIELD(SYSM_PCIE_INIT, 16, 16),
	[PERST_N] = REG_FIELD(SYSM_PCIE_INIT, 8, 8),
	[BUTTON_RSTN] = REG_FIELD(SYSM_PCIE_INIT, 7, 7),
	[POWER_UP_RSTN] = REG_FIELD(SYSM_PCIE_INIT, 6, 6),
	[ACLK_M_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 8, 8),
	[ACLK_S_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 7, 7),
	[ACLK_C_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 6, 6),
	[HCLK_EN] = REG_FIELD(SYSM_PCIE_CLK_EN, 5, 5),
};

struct sf_pcie {
	struct dw_pcie			pci;
	void __iomem			*elbi;
	struct clk			*csr_clk;
	struct clk			*ref_clk;
	struct phy			*phy;
	struct regmap			*pciesys;
	struct regmap_field		*pciesys_reg[SYSM_REGFIELD_MAX];
	struct gpio_desc		*reset_gpio;
};

static void sf_pcie_enable_part_lanes_rxei_exit(struct sf_pcie *sf_pcie)
{
	u32 val;
	val = readl(sf_pcie->pci.dbi_base + 0x708);
	val = val | 0x1 << 22;
	writel(val, sf_pcie->pci.dbi_base + 0x708);
	val = readl(sf_pcie->pci.dbi_base + 0x708);
	msleep(20);
}

static void sf_pcie_enable_speed_change(struct sf_pcie *sf_pcie)
{
	u32 val;
	val = readl(sf_pcie->pci.dbi_base + 0x80c);
	val = val | 0x1 << 17;
	writel(val, sf_pcie->pci.dbi_base + 0x80c);
	val = readl(sf_pcie->pci.dbi_base + 0x80c);
	msleep(20);
}

static int sf_pcie_clk_enable(struct sf_pcie *sf_pcie)
{
	int ret;
	ret = clk_prepare_enable(sf_pcie->csr_clk);
	if (ret)
		return ret;

	ret = clk_prepare_enable(sf_pcie->ref_clk);
	if (ret)
		return ret;

	regmap_field_write(sf_pcie->pciesys_reg[ACLK_M_EN], 1);
	regmap_field_write(sf_pcie->pciesys_reg[ACLK_S_EN], 1);
	regmap_field_write(sf_pcie->pciesys_reg[ACLK_C_EN], 1);
	regmap_field_write(sf_pcie->pciesys_reg[HCLK_EN], 1);
	return 0;
}

static void sf_pcie_clk_disable(struct sf_pcie *sf_pcie)
{
	regmap_field_write(sf_pcie->pciesys_reg[ACLK_M_EN], 0);
	regmap_field_write(sf_pcie->pciesys_reg[ACLK_S_EN], 0);
	regmap_field_write(sf_pcie->pciesys_reg[ACLK_C_EN], 0);
	regmap_field_write(sf_pcie->pciesys_reg[HCLK_EN], 0);
	clk_disable_unprepare(sf_pcie->csr_clk);
	clk_disable_unprepare(sf_pcie->ref_clk);
}

static int sf_pcie_phy_enable(struct sf_pcie *pcie)
{
	int ret;

	ret = phy_init(pcie->phy);
	if (ret)
		return ret;
	return phy_power_on(pcie->phy);
}

static int sf_pcie_phy_disable(struct sf_pcie *pcie)
{
	int ret;

	ret = phy_power_off(pcie->phy);
	if (ret)
		return ret;
	return phy_exit(pcie->phy);
}

static void sf_pcie_ltssm_set_en(struct sf_pcie *pcie, bool enable)
{
	u32 val;

	val = readl(pcie->elbi + ELBI_REG0);
	if (enable)
		val |= APP_LTSSM_ENABLE;
	else
		val &= ~APP_LTSSM_ENABLE;
	writel(val, pcie->elbi + ELBI_REG0);
}

static void sf_pcie_set_reset(struct sf_pcie *pcie, bool assert) {
	regmap_field_write(pcie->pciesys_reg[PERST_N], !assert);
	regmap_field_write(pcie->pciesys_reg[BUTTON_RSTN], !assert);
	regmap_field_write(pcie->pciesys_reg[POWER_UP_RSTN], !assert);
}

/*
 * The bus interconnect subtracts address offset from the request
 * before sending it to PCIE slave port. Since DT puts config space
 * at the beginning, we can obtain the address offset from there and
 * subtract it.
 */
static u64 sf_pcie_cpu_addr_fixup(struct dw_pcie *pci, u64 cpu_addr)
{
	struct dw_pcie_rp *pp = &pci->pp;

	return cpu_addr - pp->cfg0_base;
}

static int sf_pcie_host_init(struct dw_pcie_rp *pp)
{
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct sf_pcie *sf_pcie = to_sf_pcie(pci);
	int ret;

	ret = sf_pcie_clk_enable(sf_pcie);
	if (ret)
		return dev_err_probe(sf_pcie->pci.dev, ret,
				     "failed to enable pcie clocks.\n");

	sf_pcie_set_reset(sf_pcie, true);

	ret = regmap_field_write(sf_pcie->pciesys_reg[DEVICE_TYPE], PCIE_DEVTYPE_RC);
	if (ret)
		return ret;

	gpiod_set_value_cansleep(sf_pcie->reset_gpio, 1);

	ret = sf_pcie_phy_enable(sf_pcie);
	if (ret)
		return ret;

	/* TODO: release power-down */
	msleep(100);

	sf_pcie_set_reset(sf_pcie, false);

	dw_pcie_dbi_ro_wr_en(pci);
	sf_pcie_enable_part_lanes_rxei_exit(sf_pcie);

	gpiod_set_value_cansleep(sf_pcie->reset_gpio, 0);
	return 0;
}

static void sf_pcie_host_deinit(struct dw_pcie_rp *pp) {
	struct dw_pcie *pci = to_dw_pcie_from_pp(pp);
	struct sf_pcie *sf_pcie = to_sf_pcie(pci);

	sf_pcie_set_reset(sf_pcie, true);

	sf_pcie_phy_disable(sf_pcie);

	gpiod_set_value_cansleep(sf_pcie->reset_gpio, 1);

	sf_pcie_clk_disable(sf_pcie);
}

static const struct dw_pcie_host_ops sf_pcie_host_ops = {
	.init = sf_pcie_host_init,
	.deinit = sf_pcie_host_deinit,
};

static int sf_pcie_start_link(struct dw_pcie *pci)
{
	struct sf_pcie *pcie = to_sf_pcie(pci);
	/*
	 * before link up with GEN1, we should config the field
	 * DIRECTION_SPEED_CHANGE of GEN2_CTRL_OFF register to insure
	 * the LTSSM to initiate a speed change to Gen2 or Gen3 after
	 * the link is initialized at Gen1 speed.
	 */
	sf_pcie_enable_speed_change(pcie);

	sf_pcie_ltssm_set_en(pcie, true);
	return 0;
}

static void sf_pcie_stop_link(struct dw_pcie *pci) {
	struct sf_pcie *pcie = to_sf_pcie(pci);

	sf_pcie_ltssm_set_en(pcie, false);
}

static const struct dw_pcie_ops dw_pcie_ops = {
	.cpu_addr_fixup = sf_pcie_cpu_addr_fixup,
	.start_link = sf_pcie_start_link,
	.stop_link = sf_pcie_stop_link,
};

static int sf_pcie_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	struct sf_pcie *sf_pcie;
	int ret;
	u32 ctlr_id;

	sf_pcie = devm_kzalloc(dev, sizeof(*sf_pcie), GFP_KERNEL);
	if (!sf_pcie)
		return -ENOMEM;

	sf_pcie->pci.dev = dev;
	sf_pcie->pci.ops = &dw_pcie_ops;
	sf_pcie->pci.pp.ops = &sf_pcie_host_ops;

	platform_set_drvdata(pdev, sf_pcie);

	sf_pcie->csr_clk = devm_clk_get(dev, "csr");
	if (IS_ERR(sf_pcie->csr_clk))
		return PTR_ERR(sf_pcie->csr_clk);

	sf_pcie->ref_clk = devm_clk_get(dev, "ref");
	if (IS_ERR(sf_pcie->ref_clk))
		return PTR_ERR(sf_pcie->ref_clk);

	sf_pcie->reset_gpio =
		devm_gpiod_get_optional(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(sf_pcie->reset_gpio)) {
		return dev_err_probe(dev, PTR_ERR(sf_pcie->reset_gpio),
				     "unable to get reset gpio\n");
	}

	sf_pcie->pciesys = syscon_regmap_lookup_by_phandle(
		pdev->dev.of_node, "siflower,pcie-sysm");
	if (IS_ERR(sf_pcie->pciesys))
		return PTR_ERR(sf_pcie->pciesys);

	sf_pcie->phy = devm_phy_get(dev, NULL);
	if (IS_ERR(sf_pcie->phy))
		return PTR_ERR(sf_pcie->phy);

	sf_pcie->elbi = devm_platform_ioremap_resource_byname(pdev, "elbi");
	if (IS_ERR(sf_pcie->elbi)) {
		return PTR_ERR(sf_pcie->elbi);
	}

	ret = of_property_read_u32(node, "siflower,ctlr-idx", &ctlr_id);
	if (ret) {
		ctlr_id = 0;
	}

	ret = devm_regmap_field_bulk_alloc(
		dev, sf_pcie->pciesys, sf_pcie->pciesys_reg,
		ctlr_id ? pcie1_sysm_regs : pcie0_sysm_regs, SYSM_REGFIELD_MAX);
	if (ret)
		return dev_err_probe(dev, ret,
				     "failed to alloc regmap fields.\n");

	ret = dw_pcie_host_init(&sf_pcie->pci.pp);
	if (ret)
		return dev_err_probe(dev, ret, "failed to initialize host\n");

	return 0;
}

static void sf_pcie_remove(struct platform_device *pdev)
{
	struct sf_pcie *pcie = platform_get_drvdata(pdev);

	dw_pcie_host_deinit(&pcie->pci.pp);
}

static const struct of_device_id sf_pcie_of_match[] = {
	{ .compatible = "siflower,sf21-pcie", },
	{},
};

static struct platform_driver sf_pcie_driver = {
	.driver = {
		.name	= "sf21-pcie",
		.of_match_table = sf_pcie_of_match,
		.probe_type = PROBE_PREFER_ASYNCHRONOUS,
	},
	.probe    = sf_pcie_probe,
	.remove_new = sf_pcie_remove,
};

module_platform_driver(sf_pcie_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Chuanhong Guo <gch981213@gmail.com>");
MODULE_DESCRIPTION("PCIe Controller driver for SF21A6826/SF21H8898 SoC");
