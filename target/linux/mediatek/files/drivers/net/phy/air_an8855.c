// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2024 Christian Marangi <ansuelsmth@gmail.com>
 */

#include <linux/bitfield.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/phy.h>

#define AN8855_PHY_SELECT_PAGE			0x1f
#define   AN8855_PHY_PAGE			GENMASK(2, 0)
#define   AN8855_PHY_PAGE_STANDARD		FIELD_PREP_CONST(AN8855_PHY_PAGE, 0x0)
#define   AN8855_PHY_PAGE_EXTENDED_1		FIELD_PREP_CONST(AN8855_PHY_PAGE, 0x1)

/* MII Registers Page 1 */
#define AN8855_PHY_EXT_REG_14			0x14
#define   AN8855_PHY_EN_DOWN_SHIFT		BIT(4)

/* R50 Calibration regs in MDIO_MMD_VEND1 */
#define AN8855_PHY_R500HM_RSEL_TX_AB		0x174
#define AN8855_PHY_R50OHM_RSEL_TX_A_EN		BIT(15)
#define AN8855_PHY_R50OHM_RSEL_TX_A		GENMASK(14, 8)
#define AN8855_PHY_R50OHM_RSEL_TX_B_EN		BIT(7)
#define AN8855_PHY_R50OHM_RSEL_TX_B		GENMASK(6, 0)
#define AN8855_PHY_R500HM_RSEL_TX_CD		0x175
#define AN8855_PHY_R50OHM_RSEL_TX_C_EN		BIT(15)
#define AN8855_PHY_R50OHM_RSEL_TX_C		GENMASK(14, 8)
#define AN8855_PHY_R50OHM_RSEL_TX_D_EN		BIT(7)
#define AN8855_PHY_R50OHM_RSEL_TX_D		GENMASK(6, 0)

#define AN8855_SWITCH_EFUSE_R50O		GENMASK(30, 24)

/* PHY TX PAIR DELAY SELECT Register */
#define AN8855_PHY_TX_PAIR_DLY_SEL_GBE		0x013
#define   AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_A_GBE GENMASK(14, 12)
#define   AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_B_GBE GENMASK(10, 8)
#define   AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_C_GBE GENMASK(6, 4)
#define   AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_D_GBE GENMASK(2, 0)
/* PHY ADC Register */
#define AN8855_PHY_RXADC_CTRL			0x0d8
#define   AN8855_PHY_RG_AD_SAMNPLE_PHSEL_A	BIT(12)
#define   AN8855_PHY_RG_AD_SAMNPLE_PHSEL_B	BIT(8)
#define   AN8855_PHY_RG_AD_SAMNPLE_PHSEL_C	BIT(4)
#define   AN8855_PHY_RG_AD_SAMNPLE_PHSEL_D	BIT(0)
#define AN8855_PHY_RXADC_REV_0			0x0d9
#define   AN8855_PHY_RG_AD_RESERVE0_A		GENMASK(15, 8)
#define   AN8855_PHY_RG_AD_RESERVE0_B		GENMASK(7, 0)
#define AN8855_PHY_RXADC_REV_1			0x0da
#define   AN8855_PHY_RG_AD_RESERVE0_C		GENMASK(15, 8)
#define   AN8855_PHY_RG_AD_RESERVE0_D		GENMASK(7, 0)

#define AN8855_PHY_ID				0xc0ff0410

#define AN8855_PHY_FLAGS_EN_CALIBRATION		BIT(0)

struct air_an8855_priv {
	u8 calibration_data[4];
};

static const u8 dsa_r50ohm_table[] = {
	127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
	127, 127, 127, 127, 127, 127, 127, 126, 122, 117,
	112, 109, 104, 101,  97,  94,  90,  88,  84,  80,
	78,  74,  72,  68,  66,  64,  61,  58,  56,  53,
	51,  48,  47,  44,  42,  40,  38,  36,  34,  32,
	31,  28,  27,  24,  24,  22,  20,  18,  16,  16,
	14,  12,  11,   9
};

static int en8855_get_r50ohm_val(struct device *dev, const char *calib_name,
				 u8 *dest)
{
	u32 shift_sel, val;
	int ret;
	int i;

	ret = nvmem_cell_read_u32(dev, calib_name, &val);
	if (ret)
		return ret;

	shift_sel = FIELD_GET(AN8855_SWITCH_EFUSE_R50O, val);
	for (i = 0; i < ARRAY_SIZE(dsa_r50ohm_table); i++)
		if (dsa_r50ohm_table[i] == shift_sel)
			break;

	if (i < 8 || i >= ARRAY_SIZE(dsa_r50ohm_table))
		*dest = dsa_r50ohm_table[25];
	else
		*dest = dsa_r50ohm_table[i - 8];

	return 0;
}

static int an8855_probe(struct phy_device *phydev)
{
	struct device *dev = &phydev->mdio.dev;
	struct device_node *node = dev->of_node;
	struct air_an8855_priv *priv;

	/* If we don't have a node, skip calib */
	if (!node)
		return 0;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	phydev->priv = priv;

	return 0;
}

static int an8855_get_downshift(struct phy_device *phydev, u8 *data)
{
	int val;

	val = phy_read_paged(phydev, AN8855_PHY_PAGE_EXTENDED_1, AN8855_PHY_EXT_REG_14);
	if (val < 0)
		return val;

	*data = val & AN8855_PHY_EN_DOWN_SHIFT ? DOWNSHIFT_DEV_DEFAULT_COUNT :
						 DOWNSHIFT_DEV_DISABLE;

	return 0;
}

static int an8855_set_downshift(struct phy_device *phydev, u8 cnt)
{
	u16 ds = cnt != DOWNSHIFT_DEV_DISABLE ? AN8855_PHY_EN_DOWN_SHIFT : 0;

	return phy_modify_paged(phydev, AN8855_PHY_PAGE_EXTENDED_1,
				AN8855_PHY_EXT_REG_14, AN8855_PHY_EN_DOWN_SHIFT,
				ds);
}

static int an8855_config_init(struct phy_device *phydev)
{
	struct air_an8855_priv *priv = phydev->priv;
	struct device *dev = &phydev->mdio.dev;
	int ret;

	/* Enable HW auto downshift */
	ret = an8855_set_downshift(phydev, DOWNSHIFT_DEV_DEFAULT_COUNT);
	if (ret)
		return ret;

	/* Apply calibration values, if needed.
	 * AN8855_PHY_FLAGS_EN_CALIBRATION signal this.
	 */
	if (priv && phydev->dev_flags & AN8855_PHY_FLAGS_EN_CALIBRATION) {
		u8 *calibration_data = priv->calibration_data;

		ret = en8855_get_r50ohm_val(dev, "tx_a", &calibration_data[0]);
		if (ret)
			return ret;

		ret = en8855_get_r50ohm_val(dev, "tx_b", &calibration_data[1]);
		if (ret)
			return ret;

		ret = en8855_get_r50ohm_val(dev, "tx_c", &calibration_data[2]);
		if (ret)
			return ret;

		ret = en8855_get_r50ohm_val(dev, "tx_d", &calibration_data[3]);
		if (ret)
			return ret;

		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_R500HM_RSEL_TX_AB,
				     AN8855_PHY_R50OHM_RSEL_TX_A | AN8855_PHY_R50OHM_RSEL_TX_B,
				     FIELD_PREP(AN8855_PHY_R50OHM_RSEL_TX_A, calibration_data[0]) |
				     FIELD_PREP(AN8855_PHY_R50OHM_RSEL_TX_B, calibration_data[1]));
		if (ret)
			return ret;
		ret = phy_modify_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_R500HM_RSEL_TX_CD,
				     AN8855_PHY_R50OHM_RSEL_TX_C | AN8855_PHY_R50OHM_RSEL_TX_D,
				     FIELD_PREP(AN8855_PHY_R50OHM_RSEL_TX_C, calibration_data[2]) |
				     FIELD_PREP(AN8855_PHY_R50OHM_RSEL_TX_D, calibration_data[3]));
		if (ret)
			return ret;
	}

	/* Apply values to reduce signal noise */
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_TX_PAIR_DLY_SEL_GBE,
			    FIELD_PREP(AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_A_GBE, 0x4) |
			    FIELD_PREP(AN8855_PHY_CR_DA_TX_PAIR_DELKAY_SEL_C_GBE, 0x4));
	if (ret)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_RXADC_CTRL,
			    AN8855_PHY_RG_AD_SAMNPLE_PHSEL_A |
			    AN8855_PHY_RG_AD_SAMNPLE_PHSEL_C);
	if (ret)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_RXADC_REV_0,
			    FIELD_PREP(AN8855_PHY_RG_AD_RESERVE0_A, 0x1));
	if (ret)
		return ret;
	ret = phy_write_mmd(phydev, MDIO_MMD_VEND1, AN8855_PHY_RXADC_REV_1,
			    FIELD_PREP(AN8855_PHY_RG_AD_RESERVE0_C, 0x1));
	if (ret)
		return ret;

	return 0;
}

static int an8855_get_tunable(struct phy_device *phydev,
			      struct ethtool_tunable *tuna, void *data)
{
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		return an8855_get_downshift(phydev, data);
	default:
		return -EOPNOTSUPP;
	}
}

static int an8855_set_tunable(struct phy_device *phydev,
			      struct ethtool_tunable *tuna, const void *data)
{
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		return an8855_set_downshift(phydev, *(const u8 *)data);
	default:
		return -EOPNOTSUPP;
	}
}

static int an8855_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, AN8855_PHY_SELECT_PAGE);
}

static int an8855_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, AN8855_PHY_SELECT_PAGE, page);
}

static struct phy_driver an8855_driver[] = {
{
	PHY_ID_MATCH_EXACT(AN8855_PHY_ID),
	.name			= "Airoha AN8855 internal PHY",
	/* PHY_GBIT_FEATURES */
	.flags			= PHY_IS_INTERNAL,
	.probe			= an8855_probe,
	.config_init		= an8855_config_init,
	.soft_reset		= genphy_soft_reset,
	.get_tunable		= an8855_get_tunable,
	.set_tunable		= an8855_set_tunable,
	.suspend		= genphy_suspend,
	.resume			= genphy_resume,
	.read_page		= an8855_read_page,
	.write_page		= an8855_write_page,
}, };

module_phy_driver(an8855_driver);

static struct mdio_device_id __maybe_unused an8855_tbl[] = {
	{ PHY_ID_MATCH_EXACT(AN8855_PHY_ID) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, an8855_tbl);

MODULE_DESCRIPTION("Airoha AN8855 PHY driver");
MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_LICENSE("GPL");
