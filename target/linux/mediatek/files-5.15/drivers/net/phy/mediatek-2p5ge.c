// SPDX-License-Identifier: GPL-2.0+
#include <linux/bitfield.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/nvmem-consumer.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/phy.h>

#define MEDAITEK_2P5GE_PHY_DMB_FW "mediatek/mediatek-2p5ge-phy-dmb.bin"
#define MEDIATEK_2P5GE_PHY_PMB_FW "mediatek/mediatek-2p5ge-phy-pmb.bin"

#define MD32_EN_CFG			0x18
#define   MD32_EN			BIT(0)

#define BASE100T_STATUS_EXTEND		0x10
#define BASE1000T_STATUS_EXTEND		0x11
#define EXTEND_CTRL_AND_STATUS		0x16

#define PHY_AUX_CTRL_STATUS		0x1d
#define   PHY_AUX_DPX_MASK		GENMASK(5, 5)
#define   PHY_AUX_SPEED_MASK		GENMASK(4, 2)

/* Registers on MDIO_MMD_VEND1 */
#define MTK_PHY_LINK_STATUS_MISC	0xa2
#define   MTK_PHY_FDX_ENABLE		BIT(5)

/* Registers on MDIO_MMD_VEND2 */
#define MTK_PHY_LED0_ON_CTRL		0x24
#define   MTK_PHY_LED0_ON_LINK1000	BIT(0)
#define   MTK_PHY_LED0_ON_LINK100	BIT(1)
#define   MTK_PHY_LED0_ON_LINK10	BIT(2)
#define   MTK_PHY_LED0_ON_LINK2500	BIT(7)
#define   MTK_PHY_LED0_POLARITY		BIT(14)

#define MTK_PHY_LED1_ON_CTRL		0x26
#define   MTK_PHY_LED1_ON_FDX		BIT(4)
#define   MTK_PHY_LED1_ON_HDX		BIT(5)
#define   MTK_PHY_LED1_POLARITY		BIT(14)

enum {
	PHY_AUX_SPD_10 = 0,
	PHY_AUX_SPD_100,
	PHY_AUX_SPD_1000,
	PHY_AUX_SPD_2500,
};

static int mt798x_2p5ge_phy_config_init(struct phy_device *phydev)
{
	int ret;
	int i;
	const struct firmware *fw;
	struct device *dev = &phydev->mdio.dev;
	struct device_node *np;
	void __iomem *dmb_addr;
	void __iomem *pmb_addr;
	void __iomem *mcucsr_base;
	u16 reg;
	struct pinctrl *pinctrl;

	np = of_find_compatible_node(NULL, NULL, "mediatek,2p5gphy-fw");
	if (!np)
		return -ENOENT;

	dmb_addr = of_iomap(np, 0);
	if (!dmb_addr)
		return -ENOMEM;
	pmb_addr = of_iomap(np, 1);
	if (!pmb_addr)
		return -ENOMEM;
	mcucsr_base = of_iomap(np, 2);
	if (!mcucsr_base)
		return -ENOMEM;

	ret = request_firmware(&fw, MEDAITEK_2P5GE_PHY_DMB_FW, dev);
	if (ret) {
		dev_err(dev, "failed to load firmware: %s, ret: %d\n",
			MEDAITEK_2P5GE_PHY_DMB_FW, ret);
		return ret;
	}
	for (i = 0; i < fw->size - 1; i += 4)
		writel(*((uint32_t *)(fw->data + i)), dmb_addr + i);
	release_firmware(fw);

	ret = request_firmware(&fw, MEDIATEK_2P5GE_PHY_PMB_FW, dev);
	if (ret) {
		dev_err(dev, "failed to load firmware: %s, ret: %d\n",
			MEDIATEK_2P5GE_PHY_PMB_FW, ret);
		return ret;
	}
	for (i = 0; i < fw->size - 1; i += 4)
		writel(*((uint32_t *)(fw->data + i)), pmb_addr + i);
	release_firmware(fw);

	reg = readw(mcucsr_base + MD32_EN_CFG);
	writew(reg | MD32_EN, mcucsr_base + MD32_EN_CFG);
	dev_dbg(dev, "Firmware loading/trigger ok.\n");

	/* Setup LED */
	phy_clear_bits_mmd(phydev, MDIO_MMD_VEND2, MTK_PHY_LED0_ON_CTRL,
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

	return 0;
}

static int mt798x_2p5ge_phy_config_aneg(struct phy_device *phydev)
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

static int mt798x_2p5ge_phy_get_features(struct phy_device *phydev)
{
	int ret;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;

	/* We don't support HDX at MAC layer on mt798x.
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

static int mt798x_2p5ge_phy_read_status(struct phy_device *phydev)
{
	int ret;

	ret = genphy_update_link(phydev);
	if (ret)
		return ret;

	phydev->speed = SPEED_UNKNOWN;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	if (!phydev->link)
		return 0;

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
		phydev->duplex = DUPLEX_FULL; /* 2.5G must be FDX */
		break;
	}

	ret = phy_read_mmd(phydev, MDIO_MMD_VEND1, MTK_PHY_LINK_STATUS_MISC);
	if (ret < 0)
		return ret;

	phydev->duplex = (ret & MTK_PHY_FDX_ENABLE) ? DUPLEX_FULL : DUPLEX_HALF;

	return 0;
}

static struct phy_driver mtk_gephy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(0x00339c11),
		.name		= "MediaTek MT798x 2.5GbE PHY",
		.config_init	= mt798x_2p5ge_phy_config_init,
		.config_aneg    = mt798x_2p5ge_phy_config_aneg,
		.get_features	= mt798x_2p5ge_phy_get_features,
		.read_status	= mt798x_2p5ge_phy_read_status,
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
MODULE_FIRMWARE(MEDAITEK_2P5GE_PHY_DMB_FW);
MODULE_FIRMWARE(MEDIATEK_2P5GE_PHY_PMB_FW);
