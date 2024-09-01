// SPDX-License-Identifier: GPL-2.0+
#include <linux/bitfield.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/phy.h>
#include <linux/pm_domain.h>
#include <linux/pm_runtime.h>

#define MT7988_2P5GE_PMB "mediatek/mt7988/i2p5ge-phy-pmb.bin"

#define MD32_EN					BIT(0)
#define PMEM_PRIORITY				BIT(8)
#define DMEM_PRIORITY				BIT(16)

#define BASE100T_STATUS_EXTEND			0x10
#define BASE1000T_STATUS_EXTEND			0x11
#define EXTEND_CTRL_AND_STATUS			0x16

#define PHY_AUX_CTRL_STATUS			0x1d
#define   PHY_AUX_DPX_MASK			GENMASK(5, 5)
#define   PHY_AUX_SPEED_MASK			GENMASK(4, 2)

/* Registers on MDIO_MMD_VEND1 */
#define MTK_PHY_LINK_STATUS_MISC		0xa2
#define   MTK_PHY_FDX_ENABLE			BIT(5)

#define MTK_PHY_LPI_PCS_DSP_CTRL		0x121
#define   MTK_PHY_LPI_SIG_EN_LO_THRESH100_MASK	GENMASK(12, 8)

/* Registers on MDIO_MMD_VEND2 */
#define MTK_PHY_LED0_ON_CTRL			0x24
#define   MTK_PHY_LED0_ON_LINK1000		BIT(0)
#define   MTK_PHY_LED0_ON_LINK100		BIT(1)
#define   MTK_PHY_LED0_ON_LINK10		BIT(2)
#define   MTK_PHY_LED0_ON_LINK2500		BIT(7)
#define   MTK_PHY_LED0_POLARITY			BIT(14)

#define MTK_PHY_LED1_ON_CTRL			0x26
#define   MTK_PHY_LED1_ON_FDX			BIT(4)
#define   MTK_PHY_LED1_ON_HDX			BIT(5)
#define   MTK_PHY_LED1_POLARITY			BIT(14)

#define MTK_EXT_PAGE_ACCESS			0x1f
#define MTK_PHY_PAGE_STANDARD			0x0000
#define MTK_PHY_PAGE_EXTENDED_52B5		0x52b5

struct mtk_i2p5ge_phy_priv {
	bool fw_loaded;
};

enum {
	PHY_AUX_SPD_10 = 0,
	PHY_AUX_SPD_100,
	PHY_AUX_SPD_1000,
	PHY_AUX_SPD_2500,
};

static int mtk_2p5ge_phy_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, MTK_EXT_PAGE_ACCESS);
}

static int mtk_2p5ge_phy_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, MTK_EXT_PAGE_ACCESS, page);
}

static int mt7988_2p5ge_phy_probe(struct phy_device *phydev)
{
	struct mtk_i2p5ge_phy_priv *phy_priv;

	phy_priv = devm_kzalloc(&phydev->mdio.dev,
				sizeof(struct mtk_i2p5ge_phy_priv), GFP_KERNEL);
	if (!phy_priv)
		return -ENOMEM;

	phydev->priv = phy_priv;

	return 0;
}

static int mt7988_2p5ge_phy_config_init(struct phy_device *phydev)
{
	int ret, i;
	const struct firmware *fw;
	struct device *dev = &phydev->mdio.dev;
	struct device_node *np;
	void __iomem *pmb_addr;
	void __iomem *md32_en_cfg_base;
	struct mtk_i2p5ge_phy_priv *phy_priv = phydev->priv;
	u16 reg;
	struct pinctrl *pinctrl;

	if (!phy_priv->fw_loaded) {
		np = of_find_compatible_node(NULL, NULL, "mediatek,2p5gphy-fw");
		if (!np)
			return -ENOENT;
		pmb_addr = of_iomap(np, 0);
		if (!pmb_addr)
			return -ENOMEM;
		md32_en_cfg_base = of_iomap(np, 1);
		if (!md32_en_cfg_base)
			return -ENOMEM;

		ret = request_firmware(&fw, MT7988_2P5GE_PMB, dev);
		if (ret) {
			dev_err(dev, "failed to load firmware: %s, ret: %d\n",
				MT7988_2P5GE_PMB, ret);
			return ret;
		}

		reg = readw(md32_en_cfg_base);
		if (reg & MD32_EN) {
			phy_set_bits(phydev, 0, BIT(15));
			usleep_range(10000, 11000);
		}
		phy_set_bits(phydev, 0, BIT(11));

		/* Write magic number to safely stall MCU */
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x800e, 0x1100);
		phy_write_mmd(phydev, MDIO_MMD_VEND1, 0x800f, 0x00df);

		for (i = 0; i < fw->size - 1; i += 4)
			writel(*((uint32_t *)(fw->data + i)), pmb_addr + i);
		release_firmware(fw);

		writew(reg & ~MD32_EN, md32_en_cfg_base);
		writew(reg | MD32_EN, md32_en_cfg_base);
		phy_set_bits(phydev, 0, BIT(15));
		dev_info(dev, "Firmware loading/trigger ok.\n");

		phy_priv->fw_loaded = true;
	}

	/* Setup LED */

	/* Set polarity of led0 to active-high for BPI-R4 */
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
			 MTK_PHY_LED0_POLARITY);

	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
			 MTK_PHY_LED0_ON_LINK10 |
			 MTK_PHY_LED0_ON_LINK100 |
			 MTK_PHY_LED0_ON_LINK1000 |
			 MTK_PHY_LED0_ON_LINK2500);
	phy_set_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED1_ON_CTRL,
			 MTK_PHY_LED1_ON_FDX | MTK_PHY_LED1_ON_HDX);

	pinctrl = devm_pinctrl_get_select(&phydev->mdio.dev, "i2p5gbe-led");
	if (IS_ERR(pinctrl)) {
		dev_err(&phydev->mdio.dev, "Fail to set LED pins!\n");
		return PTR_ERR(pinctrl);
	}

	phy_modify_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LPI_PCS_DSP_CTRL,
		       MTK_PHY_LPI_SIG_EN_LO_THRESH100_MASK, 0);

	/* Enable 16-bit next page exchange bit if 1000-BT isn't advertizing */
	phy_select_page(phydev, MTK_PHY_PAGE_EXTENDED_52B5);
	__phy_write(phydev, 0x11, 0xfbfa);
	__phy_write(phydev, 0x12, 0xc3);
	__phy_write(phydev, 0x10, 0x87f8);
	phy_restore_page(phydev, MTK_PHY_PAGE_STANDARD, 0);

	return 0;
}

static int mt7988_2p5ge_phy_config_aneg(struct phy_device *phydev)
{
	bool changed = false;
	u32 adv;
	int ret;

	if (phydev->autoneg == AUTONEG_DISABLE) {
		/* Configure half duplex with genphy_setup_forced,
		 * because genphy_c45_pma_setup_forced does not support.
		 */
		return phydev->duplex != DUPLEX_FULL
			? genphy_setup_forced(phydev)
			: genphy_c45_pma_setup_forced(phydev);
	}

	ret = genphy_c45_an_config_aneg(phydev);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	adv = linkmode_adv_to_mii_ctrl1000_t(phydev->advertising);
	ret = phy_modify_changed(phydev, MII_CTRL1000,
				 ADVERTISE_1000FULL | ADVERTISE_1000HALF,
				 adv);
	if (ret < 0)
		return ret;
	if (ret > 0)
		changed = true;

	return genphy_c45_check_and_restart_aneg(phydev, changed);
}

static int mt7988_2p5ge_phy_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;

	/* We don't support HDX at MAC layer on mt7988.
	 * So mask phy's HDX capabilities, too.
	 */
	linkmode_set_bit(ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
			 phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_Autoneg_BIT, phydev->supported);

	return 0;
}

static int mt7988_2p5ge_phy_read_status(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (phydev->autoneg == AUTONEG_ENABLE && phydev->autoneg_complete) {
		ret = genphy_c45_read_lpa(phydev);
		if (ret < 0)
			return ret;

		/* Read the link partner's 1G advertisement */
		ret = phy_read(phydev, MII_STAT1000);
		if (ret < 0)
			return ret;
		mii_stat1000_mod_linkmode_lpa_t(phydev->lp_advertising, ret);
	} else if (phydev->autoneg == AUTONEG_DISABLE) {
		linkmode_zero(phydev->lp_advertising);
	}

	ret = phy_read(phydev, PHY_AUX_CTRL_STATUS);
	if (ret < 0)
		return ret;

	switch (FIELD_GET(PHY_AUX_SPEED_MASK, ret)) {
	case PHY_AUX_SPD_10:
		phydev->speed = SPEED_10;
		break;
	case PHY_AUX_SPD_100:
		phydev->speed = SPEED_100;
		break;
	case PHY_AUX_SPD_1000:
		phydev->speed = SPEED_1000;
		break;
	case PHY_AUX_SPD_2500:
		phydev->speed = SPEED_2500;
		break;
	}

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LINK_STATUS_MISC);
	if (ret < 0)
		return ret;

	phydev->duplex = (ret & MTK_PHY_FDX_ENABLE) ? DUPLEX_FULL : DUPLEX_HALF;
	/* FIXME: The current firmware always enables rate adaptation mode. */
	phydev->rate_matching = RATE_MATCH_PAUSE;

	return 0;
}

static int mt7988_2p5ge_phy_get_rate_matching(struct phy_device *phydev,
					      phy_interface_t iface)
{
	return RATE_MATCH_PAUSE;
}

static struct phy_driver mtk_gephy_driver[] = {
	{
		PHY_ID_MATCH_MODEL(0x00339c11),
		.name		= "MediaTek MT798x 2.5GbE PHY",
		.probe		= mt7988_2p5ge_phy_probe,
		.config_init	= mt7988_2p5ge_phy_config_init,
		.config_aneg    = mt7988_2p5ge_phy_config_aneg,
		.get_features	= mt7988_2p5ge_phy_get_features,
		.read_status	= mt7988_2p5ge_phy_read_status,
		.get_rate_matching	= mt7988_2p5ge_phy_get_rate_matching,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
		.read_page	= mtk_2p5ge_phy_read_page,
		.write_page	= mtk_2p5ge_phy_write_page,
	},
};

module_phy_driver(mtk_gephy_driver);

static struct mdio_device_id __maybe_unused mtk_2p5ge_phy_tbl[] = {
	{ PHY_ID_MATCH_VENDOR(0x00339c00) },
	{ }
};

MODULE_DESCRIPTION("MediaTek 2.5Gb Ethernet PHY driver");
MODULE_AUTHOR("SkyLake Huang <SkyLake.Huang@mediatek.com>");
MODULE_LICENSE("GPL");

MODULE_DEVICE_TABLE(mdio, mtk_2p5ge_phy_tbl);
MODULE_FIRMWARE(MT7988_2P5GE_PMB);
