// SPDX-License-Identifier: GPL-2.0

#include <linux/mfd/syscon.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/phy/phy.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/regmap.h>

#define SF_PCIE_PHY_NLANES	2

#define TOPCRM_LVDS0_CFG	0xe8
#define TOPCRM_LVDS1_CFG	0x120
#define  LVDS_BIAS_EN		BIT(20)
#define  LVDS_PULLDN		BIT(19)
#define  LVDS_SCHMITT_EN	BIT(18)
#define  LVDS_TX_CM		BIT(17)
#define  LVDS_RXCM_EN		BIT(16)
#define  LVDS_RTERM_VAL		GENMASK(15, 13)
#define  LVDS_TXDRV		GENMASK(12, 9)
#define  LVDS_IEN_N		BIT(8)
#define  LVDS_IEN_P		BIT(7)
#define  LVDS_OEN_N		BIT(6)
#define  LVDS_OEN_P		BIT(5)
#define  LVDS_RTERM_EN		BIT(4)
#define  LVDS_RXEN		BIT(3)
#define  LVDS_TXEN		BIT(2)
#define  LVDS_VBIAS_SEL		BIT(0)

#define PCIE_SYSM_SET		0x0
#define  PCIE_LANE_MUX		GENMASK(9, 8)
#define  PHY0_L0_PHY1_L1	(0 << 8)
#define  PHY0_L0L1		(1 << 8)
#define  PHY0_L1_PHY1_L0	(2 << 8)
#define  PHY1_L0L1		(3 << 8)

#define PCIE_SYSM_INIT		0x4
#define  PCIE_L1_REPEAT_CLK_EN	BIT(10)
#define  PCIE_L0_REPEAT_CLK_EN	BIT(9)
#define  PCIE_L1_RSTN		BIT(2)
#define  PCIE_L0_RSTN		BIT(1)
#define  PCIE_PHY_RSTN		BIT(0)


struct sf21_pcie_phy_inst {
	struct phy *phy;
	u8 idx;
	u8 num_lanes;
	u8 lvds_idx;
};

struct sf21_pcie_phy {
	struct device *dev;
	struct clk *refclk;
	struct clk *csrclk;
	struct regmap *pcie_regmap;
	struct regmap *topcrm_regmap;
	struct mutex lock;
	int nlanes_enabled;
	struct sf21_pcie_phy_inst insts[2];
};

static struct sf21_pcie_phy_inst *phy_to_instance(struct phy *phy)
{
	return phy_get_drvdata(phy);
}

static struct sf21_pcie_phy *
instance_to_priv(struct sf21_pcie_phy_inst *inst)
{
	return container_of(inst, struct sf21_pcie_phy, insts[inst->idx]);
}

static int sf_pcie_phy_lvds_on(struct sf21_pcie_phy *priv, int idx)
{
	return regmap_set_bits(priv->topcrm_regmap,
			       idx ? TOPCRM_LVDS1_CFG : TOPCRM_LVDS0_CFG,
			       LVDS_TXEN | LVDS_BIAS_EN);
}

static int sf_pcie_phy_lvds_off(struct sf21_pcie_phy *priv, int idx)
{
	return regmap_clear_bits(priv->topcrm_regmap,
				 idx ? TOPCRM_LVDS1_CFG : TOPCRM_LVDS0_CFG,
				 LVDS_TXEN | LVDS_BIAS_EN);
}

static int sf_pcie_lane_on(struct sf21_pcie_phy *priv, int idx)
{
	return regmap_set_bits(priv->pcie_regmap, PCIE_SYSM_INIT,
				 idx ? PCIE_L1_RSTN : PCIE_L0_RSTN);
}

static int sf_pcie_lane_off(struct sf21_pcie_phy *priv, int idx)
{
	return regmap_clear_bits(priv->pcie_regmap, PCIE_SYSM_INIT,
			       idx ? PCIE_L1_RSTN : PCIE_L0_RSTN);
}

static int sf21_pcie_phy_power_on(struct phy *phy)
{
	struct sf21_pcie_phy_inst *inst = phy_to_instance(phy);
	struct sf21_pcie_phy *priv = instance_to_priv(inst);
	int ret;
	mutex_lock(&priv->lock);
	if (SF_PCIE_PHY_NLANES - priv->nlanes_enabled < inst->num_lanes) {
		dev_err(priv->dev, "too many lanes requested for PHY %u\n",
			inst->idx);
		ret = -EBUSY;
		goto out;
	}

	if (inst->num_lanes == 2) {
		regmap_update_bits(priv->pcie_regmap, PCIE_SYSM_SET,
				   PCIE_LANE_MUX,
				   inst->idx ? PHY1_L0L1 : PHY0_L0L1);
	} else {
		regmap_update_bits(priv->pcie_regmap, PCIE_SYSM_SET,
				   PCIE_LANE_MUX, PHY0_L0_PHY1_L1);
	}

	/*
	 * The PCIE clock goes like:
	 *  internal refclk -- serdes0 -- serdes1 -- LVDS0
	 *                                        \- LVDS1
	 * Both clock repeaters must be enabled at PHY power-on,
	 * otherwise there's no PCIE reference clock output.
	 */

	if (!priv->nlanes_enabled) {
		ret = clk_prepare_enable(priv->refclk);
		if (ret)
			goto out;

		ret = regmap_set_bits(priv->pcie_regmap, PCIE_SYSM_INIT, PCIE_PHY_RSTN);
		if (ret)
			goto out;

		ret = regmap_set_bits(priv->pcie_regmap, PCIE_SYSM_INIT,
				      PCIE_L0_REPEAT_CLK_EN |
					      PCIE_L1_REPEAT_CLK_EN);
		if (ret)
			goto out;
	}

	priv->nlanes_enabled += inst->num_lanes;

	ret = sf_pcie_phy_lvds_on(priv, inst->lvds_idx);
	if (ret)
		goto out;

	ret = sf_pcie_lane_on(priv, inst->idx);
	if (ret)
		goto out;
	if (inst->num_lanes == 2)
		ret = sf_pcie_lane_on(priv, !inst->idx);
out:
	mutex_unlock(&priv->lock);
	return ret;
}

static int sf21_pcie_phy_power_off(struct phy *phy)
{
	struct sf21_pcie_phy_inst *inst = phy_to_instance(phy);
	struct sf21_pcie_phy *priv = instance_to_priv(inst);
	int ret;
	mutex_lock(&priv->lock);

	if (inst->num_lanes == 2) {
		ret = sf_pcie_lane_off(priv, !inst->idx);
		if (ret)
			goto out;
	}

	ret = sf_pcie_lane_off(priv, inst->idx);
	if (ret)
		goto out;

	ret = sf_pcie_phy_lvds_off(priv, inst->lvds_idx);
	if (ret)
		goto out;
	priv->nlanes_enabled -= inst->num_lanes;

	if (!priv->nlanes_enabled) {
		ret = regmap_clear_bits(priv->pcie_regmap, PCIE_SYSM_INIT, PCIE_PHY_RSTN);
		if (ret)
			goto out;

		ret = regmap_clear_bits(priv->pcie_regmap, PCIE_SYSM_INIT,
					PCIE_L0_REPEAT_CLK_EN |
						PCIE_L1_REPEAT_CLK_EN);
		if (ret)
			goto out;
		clk_disable_unprepare(priv->refclk);
	}
out:
	mutex_unlock(&priv->lock);
	return ret;
}

static const struct phy_ops sf21_pcie_phy_ops = {
	.power_on = sf21_pcie_phy_power_on,
	.power_off = sf21_pcie_phy_power_off,
	.owner = THIS_MODULE,
};

static int sf21_pcie_phy_probe(struct platform_device *pdev)
{
	struct sf21_pcie_phy *p_phy;
	struct phy_provider *provider;
	struct phy *phy;
	struct device_node *child;
	int num_insts = 0;
	u32 reg_idx, num_lanes, lvds_idx;
	int ret;

	p_phy = devm_kzalloc(&pdev->dev, sizeof(*p_phy), GFP_KERNEL);
	if (!p_phy)
		return -ENOMEM;

	p_phy->dev = &pdev->dev;
	platform_set_drvdata(pdev, p_phy);

	p_phy->refclk = devm_clk_get(p_phy->dev, "ref");
	if (IS_ERR(p_phy->refclk))
		return dev_err_probe(p_phy->dev, PTR_ERR(p_phy->refclk),
				     "Failed to get phy reference clock.\n");

	p_phy->csrclk = devm_clk_get_enabled(p_phy->dev, "csr");
	if (IS_ERR(p_phy->csrclk))
		return dev_err_probe(p_phy->dev, PTR_ERR(p_phy->csrclk),
				     "Failed to get enabled phy csr clock.\n");

	p_phy->pcie_regmap = syscon_node_to_regmap(pdev->dev.of_node);
	if (IS_ERR(p_phy->pcie_regmap))
		return dev_err_probe(p_phy->dev, PTR_ERR(p_phy->pcie_regmap),
				     "Failed to get regmap.\n");

	p_phy->topcrm_regmap = syscon_regmap_lookup_by_phandle(
		pdev->dev.of_node, "siflower,topcrm");
	if (IS_ERR(p_phy->topcrm_regmap))
		return dev_err_probe(p_phy->dev, PTR_ERR(p_phy->topcrm_regmap),
				     "Failed to get regmap for topcrm.\n");

	p_phy->nlanes_enabled = 0;
	mutex_init(&p_phy->lock);

	regmap_clear_bits(p_phy->pcie_regmap, PCIE_SYSM_INIT,
			PCIE_L1_RSTN | PCIE_L0_RSTN | PCIE_PHY_RSTN);

	for_each_available_child_of_node(pdev->dev.of_node, child) {
		ret = of_property_read_u32(child, "reg", &reg_idx);
		if (ret)
			return dev_err_probe(
				p_phy->dev, ret,
				"failed to read reg of child node %d.\n",
				num_insts);

		if (reg_idx > 1) {
			dev_err(p_phy->dev, "PHY reg should be 0 or 1.\n");
			return -EINVAL;
		}

		p_phy->insts[reg_idx].idx = reg_idx;

		ret = of_property_read_u32(child, "siflower,num-lanes",
					   &num_lanes);
		if (ret)
			return dev_err_probe(
				p_phy->dev, ret,
				"failed to read num-lanes of phy@%u.\n",
				reg_idx);

		if (num_lanes != 1 && num_lanes != 2) {
			dev_err(p_phy->dev,
				"One PHY can only request 1 or 2 serdes lanes.\n");
			return -EINVAL;
		}

		p_phy->insts[reg_idx].num_lanes = num_lanes;

		/* LVDS provides PCIE reference clock and is a separated block. */
		ret = of_property_read_u32(child, "siflower,lvds-idx",
					   &lvds_idx);
		if (ret)
			p_phy->insts[reg_idx].lvds_idx = reg_idx;
		else
			p_phy->insts[reg_idx].lvds_idx = lvds_idx;

		phy = devm_phy_create(p_phy->dev, child,
				      &sf21_pcie_phy_ops);
		if (IS_ERR(phy))
			return dev_err_probe(p_phy->dev, PTR_ERR(phy),
					     "failed to register phy@%d.\n",
					     reg_idx);

		phy_set_drvdata(phy, &p_phy->insts[reg_idx]);
		p_phy->insts[reg_idx].phy = phy;

		num_insts++;
		if (num_insts >= 2)
			break;
	}

	provider = devm_of_phy_provider_register(p_phy->dev, of_phy_simple_xlate);
	if (IS_ERR(provider))
		return dev_err_probe(p_phy->dev, PTR_ERR(provider),
				     "Failed to register PHY provider.\n");

	return 0;
}

static const struct of_device_id sf21_pcie_phy_of_match[] = {
	{ .compatible = "siflower,sf21-pcie-phy" },
	{ },
};
MODULE_DEVICE_TABLE(of, sf21_pcie_phy_of_match);

static struct platform_driver sf21_pcie_phy_driver = {
	.probe		= sf21_pcie_phy_probe,
	.driver		= {
		.name	= "sf21-pcie-phy",
		.of_match_table = sf21_pcie_phy_of_match,
	},
};
module_platform_driver(sf21_pcie_phy_driver);

MODULE_AUTHOR("Chuanhong Guo <gch981213@gmail.com>");
MODULE_DESCRIPTION("Siflower SF21A6826/SF21H8898 PCIE PHY driver");
MODULE_LICENSE("GPL");
