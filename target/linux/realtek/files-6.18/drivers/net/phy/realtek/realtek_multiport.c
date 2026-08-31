// SPDX-License-Identifier: GPL-2.0-only
/* Realtek RTL838X Ethernet MDIO interface driver
 *
 * Copyright (C) 2020 B. Koblitz
 */

#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/firmware.h>
#include <linux/crc32.h>
#include <linux/sfp.h>
#include <linux/mii.h>
#include <linux/mdio.h>
#include "../phylib.h"

/*
 * Realtek PHYs have three special page registers. Register 31 (page select) switches the
 * register pages and gives access to special registers that are mapped into register
 * range 16-24. Register 30 (extended page select) does basically the same. It especially
 * grants access to special internal data like fibre, copper or serdes setup. Register
 * 29 is a write only companion of register 30. As it flips back to 0 and register 30
 * shows the original write content it should be avoided at all cost.
 */
#define RTL821x_PAGE_SELECT		0x1f
#define RTL821x_EXT_PAGE_SELECT		0x1e

#define RTL8XXX_PAGE_MAIN		0x0000
#define RTL821X_PAGE_PORT		0x0266
#define RTL821X_PAGE_POWER		0x0a40
#define RTL821X_PAGE_GPHY		0x0a42
#define RTL821X_PAGE_MAC		0x0a43
#define RTL821X_PAGE_STATE		0x0b80
#define RTL821X_PAGE_PATCH		0x0b82
#define RTL821X_MAC_SDS_PAGE(sds, page)	(0x404 + (sds) * 0x20 + (page))

#define RTL821X_PHYCR2			0x19
#define RTL821X_PHYCR2_PHY_EEE_ENABLE	BIT(5)

#define RTL821X_MEDIA_PAGE_AUTO		0
#define RTL821X_MEDIA_PAGE_COPPER	1
#define RTL821X_MEDIA_PAGE_FIBRE	3
#define RTL821X_MEDIA_PAGE_INTERNAL	8

#define RTL821X_JOIN_FIRST		0
#define RTL821X_JOIN_LAST		1
#define RTL821X_JOIN_OTHER		2

#define RTL8214FC_MEDIA_COPPER		BIT(11)

#define PHY_ID_RTL8214C			0x001cc942
#define PHY_ID_RTL8218B_E		0x001cc980
#define PHY_ID_RTL8214_OR_8218		0x001cc981
#define PHY_ID_RTL8218D			0x001cc983
#define PHY_ID_RTL8218E			0x001cc984
#define PHY_ID_RTL8218B_I		0x001cca40

/* These PHYs share the same id (0x001cc981) */
#define PHY_IS_NOT_RTL821X		0
#define PHY_IS_RTL8214FC		1
#define PHY_IS_RTL8214FB		2
#define PHY_IS_RTL8218B_E		3

struct rtl821x_shared_priv {
	int base_addr;
	int ports;
};

static int rtl821x_package_join(struct phy_device *phydev, int ports)
{
	struct rtl821x_shared_priv *shared_priv;
	int base_addr = phydev->mdio.addr & ~(ports - 1);

	devm_phy_package_join(&phydev->mdio.dev, phydev, base_addr,
			      sizeof(struct rtl821x_shared_priv));

	shared_priv = phy_package_get_priv(phydev);
	shared_priv->base_addr = base_addr;
	shared_priv->ports++;

	if (shared_priv->ports == 1)
		return RTL821X_JOIN_FIRST;

	if (shared_priv->ports == ports)
		return RTL821X_JOIN_LAST;

	return RTL821X_JOIN_OTHER;
}

static inline struct phy_device *get_package_phy(struct phy_device *phydev, int port)
{
	struct rtl821x_shared_priv *shared_priv = phy_package_get_priv(phydev);

	return mdiobus_get_phy(phydev->mdio.bus, shared_priv->base_addr + port);
}

static inline struct phy_device *get_base_phy(struct phy_device *phydev)
{
	return get_package_phy(phydev, 0);
}

static int rtl821x_match_phy_device(struct phy_device *phydev)
{
	int oldpage, oldxpage, chip_mode, chip_cfg_mode;
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr & ~3;

	if (phydev->phy_id == PHY_ID_RTL8218B_E)
		return PHY_IS_RTL8218B_E;

	if (phydev->phy_id != PHY_ID_RTL8214_OR_8218)
		return PHY_IS_NOT_RTL821X;

	/*
	 * RTL8214FC and RTL8218B are the same PHYs with different configurations. That info is
	 * stored in the first PHY of the package. In all known configurations packages start at
	 * bus addresses that are multiples of four. Avoid paged access as this is not available
	 * during detection.
	 */

	oldpage = mdiobus_read(bus, addr, RTL821x_PAGE_SELECT);
	oldxpage = mdiobus_read(bus, addr, RTL821x_EXT_PAGE_SELECT);

	mdiobus_write(bus, addr, RTL821x_EXT_PAGE_SELECT, 0x8);
	mdiobus_write(bus, addr, RTL821x_PAGE_SELECT, 0x278);
	mdiobus_write(bus, addr, 0x12, 0x455);
	mdiobus_write(bus, addr, RTL821x_PAGE_SELECT, 0x260);
	chip_mode = mdiobus_read(bus, addr, 0x12);
	dev_dbg(&phydev->mdio.dev, "got RTL8218B/RTL8214Fx chip mode %04x\n", chip_mode);

	mdiobus_write(bus, addr, RTL821x_EXT_PAGE_SELECT, oldxpage);
	mdiobus_write(bus, addr, RTL821x_PAGE_SELECT, oldpage);

	/* no values while reading the 5th port during 5-8th port detection of RTL8218B */
	if (!chip_mode)
		return PHY_IS_RTL8218B_E;

	chip_cfg_mode = (chip_mode >> 4) & 0xf;
	chip_mode &= 0xf;

	if (chip_mode == 0xd || chip_mode == 0xf)
		return PHY_IS_RTL8218B_E;

	if (chip_mode == 0x4 || chip_mode == 0x6)
		return PHY_IS_RTL8214FC;

	if (chip_mode != 0xc && chip_mode != 0xe)
		return PHY_IS_NOT_RTL821X;

	if (chip_cfg_mode == 0x4 || chip_cfg_mode == 0x6)
		return PHY_IS_RTL8214FC;

	return PHY_IS_RTL8214FB;
}

static int rtl8218b_ext_match_phy_device(struct phy_device *phydev,
					 const struct phy_driver *phydrv)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8218B_E;
}

static int rtl8214fc_match_phy_device(struct phy_device *phydev,
				      const struct phy_driver *phydrv)
{
	return rtl821x_match_phy_device(phydev) == PHY_IS_RTL8214FC;
}

static int rtl821x_read_page(struct phy_device *phydev)
{
	return __phy_read(phydev, RTL821x_PAGE_SELECT);
}

static int rtl821x_write_page(struct phy_device *phydev, int page)
{
	return __phy_write(phydev, RTL821x_PAGE_SELECT, page);
}

static bool __rtl8214fc_media_is_fibre(struct phy_device *phydev)
{
	struct phy_device *basephy = get_base_phy(phydev);
	static int regs[] = {16, 19, 20, 21};
	int reg = regs[phydev->mdio.addr & 3];
	int oldpage, oldxpage, val;

	oldpage = __phy_read(basephy, RTL821x_PAGE_SELECT);
	oldxpage = __phy_read(basephy, RTL821x_EXT_PAGE_SELECT);

	__phy_write(basephy, RTL821x_EXT_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	__phy_write(basephy, RTL821x_PAGE_SELECT, RTL821X_PAGE_PORT);
	val = __phy_read(basephy, reg);

	__phy_write(basephy, RTL821x_EXT_PAGE_SELECT, oldxpage);
	__phy_write(basephy, RTL821x_PAGE_SELECT, oldpage);

	return !(val & RTL8214FC_MEDIA_COPPER);
}

static bool rtl8214fc_media_is_fibre(struct phy_device *phydev)
{
	int ret;

	phy_lock_mdio_bus(phydev);
	ret = __rtl8214fc_media_is_fibre(phydev);
	phy_unlock_mdio_bus(phydev);

	return ret;
}

static void rtl8214fc_power_set(struct phy_device *phydev, int port, bool on)
{
	int page = port == PORT_FIBRE ? RTL821X_MEDIA_PAGE_FIBRE : RTL821X_MEDIA_PAGE_COPPER;
	int oldxpage = __phy_read(phydev, RTL821x_EXT_PAGE_SELECT);
	int pdown = on ? 0 : BMCR_PDOWN;

	phydev_info(phydev, "power %s %s\n", on ? "on" : "off",
		    port == PORT_FIBRE ? "fibre" : "copper");

	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, page);
	phy_modify_paged(phydev, RTL821X_PAGE_POWER, 0x10, BMCR_PDOWN, pdown);
	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, oldxpage);
}

static int rtl8214fc_suspend(struct phy_device *phydev)
{
	rtl8214fc_power_set(phydev, PORT_MII, false);
	rtl8214fc_power_set(phydev, PORT_FIBRE, false);

	return 0;
}

static int rtl8214fc_resume(struct phy_device *phydev)
{
	bool set_fibre = rtl8214fc_media_is_fibre(phydev);

	rtl8214fc_power_set(phydev, PORT_MII, !set_fibre);
	rtl8214fc_power_set(phydev, PORT_FIBRE, set_fibre);

	return 0;
}

static void rtl8214fc_media_set(struct phy_device *phydev, bool set_fibre)
{
	struct phy_device *basephy = get_base_phy(phydev);
	int oldxpage = phy_read(basephy, RTL821x_EXT_PAGE_SELECT);
	int copper = set_fibre ? 0 : RTL8214FC_MEDIA_COPPER;
	static int regs[] = {16, 19, 20, 21};
	int reg = regs[phydev->mdio.addr & 3];

	phydev_info(phydev, "switch to %s\n", set_fibre ? "fibre" : "copper");
	phy_write(basephy, RTL821x_EXT_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
	phy_modify_paged(basephy, RTL821X_PAGE_PORT, reg, RTL8214FC_MEDIA_COPPER, copper);
	phy_write(basephy, RTL821x_EXT_PAGE_SELECT, oldxpage);

	if (!phydev->suspended) {
		rtl8214fc_power_set(phydev, PORT_MII, !set_fibre);
		rtl8214fc_power_set(phydev, PORT_FIBRE, set_fibre);
	}
}

static int rtl8214fc_set_tunable(struct phy_device *phydev,
				 struct ethtool_tunable *tuna, const void *data)
{
	/*
	 * The RTL8214FC driver usually detects insertion of SFP modules and automatically toggles
	 * between copper and fiber. There may be cases where the user wants to switch the port on
	 * demand. Usually ethtool offers to change the port of a multiport network card with
	 * "ethtool -s lan25 port fibre/tp" if the driver supports it. This does not work for
	 * attached phys. For more details see phy_ethtool_ksettings_set(). To avoid patching the
	 * kernel misuse the phy downshift tunable to offer that feature. For this use
	 * "ethtool --set-phy-tunable lan25 downshift on/off".
	 */
	switch (tuna->id) {
	case ETHTOOL_PHY_DOWNSHIFT:
		rtl8214fc_media_set(phydev, !rtl8214fc_media_is_fibre(phydev));
		return 0;
	default:
		return -EOPNOTSUPP;
	}
}

static int rtl8214fc_get_tunable(struct phy_device *phydev,
				 struct ethtool_tunable *tuna, void *data)
{
	/* Needed to make rtl8214fc_set_tunable() work */
	return 0;
}

static int rtl8214fc_get_features(struct phy_device *phydev)
{
	int ret = 0;

	ret = genphy_read_abilities(phydev);
	if (ret)
		return ret;
	/*
	 * The RTL8214FC only advertises TP capabilities in the standard registers. This is
	 * independent from what fibre/copper combination is currently activated. For now just
	 * announce the superset of all possible features.
	 */
	linkmode_set_bit(ETHTOOL_LINK_MODE_1000baseX_Full_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_100baseFX_Full_BIT, phydev->supported);
	linkmode_set_bit(ETHTOOL_LINK_MODE_FIBRE_BIT, phydev->supported);

	return 0;
}

static int rtl8214fc_read_status(struct phy_device *phydev)
{
	bool changed;
	int ret;

	if (rtl8214fc_media_is_fibre(phydev)) {
		phydev->port = PORT_FIBRE;
		ret = genphy_c37_read_status(phydev, &changed);
	} else {
		phydev->port = PORT_MII; /* for now aligend with rest of code */
		ret = genphy_read_status(phydev);
	}

	return ret;
}

static int rtl8214fc_config_aneg(struct phy_device *phydev)
{
	int ret;

	if (rtl8214fc_media_is_fibre(phydev)) {
		if (linkmode_test_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, phydev->advertising)) {
			linkmode_clear_bit(ETHTOOL_LINK_MODE_100baseT_Full_BIT, phydev->advertising);
			linkmode_set_bit(ETHTOOL_LINK_MODE_100baseFX_Full_BIT, phydev->advertising);
		}
		ret = genphy_c37_config_aneg(phydev);
	} else {
		ret = genphy_config_aneg(phydev);
	}

	return ret;
}

static int rtl821x_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
	int ret;

	/*
	 * The RTL821x PHYs are usually only C22 capable and are defined accordingly in DTS.
	 * Nevertheless GPL source drops clearly indicate that EEE features can be accessed
	 * directly via C45. Testing shows that C45 over C22 (as used in kernel EEE framework)
	 * works as well but only as long as PHY polling is disabled in the SOC. To avoid ugly
	 * hacks pass through C45 accesses for important EEE registers. Maybe some day the mdio
	 * bus can intercept these patterns and switch off/on polling on demand. That way this
	 * phy device driver can avoid handling special cases on its own.
	 */

	if ((devnum == MDIO_MMD_PCS && regnum == MDIO_PCS_EEE_ABLE) ||
	    (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV) ||
	    (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_LPABLE))
		ret = __mdiobus_c45_read(bus, addr, devnum, regnum);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtl821x_write_mmd(struct phy_device *phydev, int devnum, u16 regnum, u16 val)
{
	struct mii_bus *bus = phydev->mdio.bus;
	int addr = phydev->mdio.addr;
	int ret;

	/* see rtl821x_read_mmd() */
	if (devnum == MDIO_MMD_AN && regnum == MDIO_AN_EEE_ADV)
		ret = __mdiobus_c45_write(bus, addr, devnum, regnum, val);
	else
		ret = -EOPNOTSUPP;

	return ret;
}

static int rtl8214fc_read_mmd(struct phy_device *phydev, int devnum, u16 regnum)
{
	if (__rtl8214fc_media_is_fibre(phydev))
		return -EOPNOTSUPP;

	return rtl821x_read_mmd(phydev, devnum, regnum);
}

static int rtl8214fc_write_mmd(struct phy_device *phydev, int devnum, u16 regnum, u16 val)
{
	if (__rtl8214fc_media_is_fibre(phydev))
		return -EOPNOTSUPP;

	return rtl821x_write_mmd(phydev, devnum, regnum, val);
}

static int rtl8214fc_sfp_insert(void *upstream, const struct sfp_eeprom_id *id)
{
	struct phy_device *phydev = upstream;
	const struct sfp_module_caps *caps;
	phy_interface_t iface;

	caps = sfp_get_module_caps(phydev->sfp_bus);
	iface = sfp_select_interface(phydev->sfp_bus, caps->link_modes);

	phydev_info(phydev, "%s SFP module inserted\n", phy_modes(iface));

	rtl8214fc_media_set(phydev, true);

	return 0;
}

static void rtl8214fc_sfp_remove(void *upstream)
{
	struct phy_device *phydev = upstream;

	rtl8214fc_media_set(phydev, false);
}

static const struct sfp_upstream_ops rtl8214fc_sfp_ops = {
	.attach = phy_sfp_attach,
	.detach = phy_sfp_detach,
	.module_insert = rtl8214fc_sfp_insert,
	.module_remove = rtl8214fc_sfp_remove,
};

static int rtl8214c_phy_probe(struct phy_device *phydev)
{
	rtl821x_package_join(phydev, 4);

	return 0;
}

static int rtl8218x_phy_probe(struct phy_device *phydev)
{
	rtl821x_package_join(phydev, 8);

	return 0;
}

static int rtl821x_config_init(struct phy_device *phydev)
{
	/* Disable PHY-mode EEE so LPI is passed to the MAC */
	phy_modify_paged(phydev, RTL821X_PAGE_MAC, RTL821X_PHYCR2,
			 RTL821X_PHYCR2_PHY_EEE_ENABLE, 0);

	return 0;
}

static int rtl8218d_config_init(struct phy_device *phydev)
{
	int oldpage, oldxpage;
	bool is_qsgmii;
	int chip_info;

	rtl821x_config_init(phydev);

	if (phydev->mdio.addr % 8)
		return 0;
	/*
	 * The RTl8218D has two MAC (aka SoC side) operation modes. Either dual QSGMII
	 * or single XSGMII (Realtek proprietary) link. The mode is usually configured via
	 * strapping pins CHIP_MODE1/2. For the moment offer a configuration that at least
	 * works for RTL93xx devices. This sequence even "revives" a PHY that has been hard
	 * reset with
	 *
	 *	phy_write(phydev, 0x1e, 0x8);
	 *	phy_write_paged(phydev, 0x262, 0x10, 0x1);
	 */
	oldpage = phy_read(phydev, RTL821x_PAGE_SELECT);
	oldxpage = phy_read(phydev, RTL821x_EXT_PAGE_SELECT);

	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, 0x8);
	chip_info = phy_read_paged(phydev, 0x327, 0x15);
	is_qsgmii = (phy_read_paged(phydev, 0x260, 0x12) & 0xf0) == 0xd0;

	pr_info("RTL8218D (chip_id=%d, rev_id=%d) on port %d running in %s mode\n",
		(chip_info >> 7) & 0x7, chip_info & 0x7f, phydev->mdio.addr,
		is_qsgmii ? "QSGMII" : "XSGMII");

	if (is_qsgmii) {
		for (int sds = 0; sds < 2; sds++) {
			/* unknown amplification value */
			phy_modify_paged(phydev, 0x4a8 + sds * 0x100, 0x12, BIT(3), 0);
			/* main aplification */
			phy_modify_paged(phydev, 0x4ab + sds * 0x100, 0x16, 0x3e0, 0x1e0);
			/* unknown LCCMU value */
			phy_write_paged(phydev, 0x4ac + sds * 0x100, 0x15, 0x4380);
		}
	} else {
		/* serdes 0/1 disable auto negotiation */
		phy_modify_paged(phydev, 0x400, 0x12, 0, BIT(8));
		phy_modify_paged(phydev, 0x500, 0x12, 0, BIT(8));
		/* unknown eye configuration */
		phy_modify_paged(phydev, 0x4b8, 0x12, BIT(3), 0);
	}

	/* reset serdes 0 */
	phy_write_paged(phydev, 0x400, 0x10, 0x1700);
	phy_write_paged(phydev, 0x400, 0x10, 0x1703);

	/* reset serdes 1 */
	phy_write_paged(phydev, 0x500, 0x10, 0x1400);
	phy_write_paged(phydev, 0x500, 0x10, 0x1403);

	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, oldxpage);
	phy_write(phydev, RTL821x_PAGE_SELECT, oldpage);

	return 0;
}

static void rtl8218b_cmu_reset(struct phy_device *phydev, int reset_id)
{
	int bitpos = reset_id * 2;

	/* CMU seems to have 8 pairs of reset bits that always work the same way */
	phy_modify_paged(phydev, 0x467, 0x14, 0, BIT(bitpos));
	phy_modify_paged(phydev, 0x467, 0x14, 0, BIT(bitpos + 1));
	phy_write_paged(phydev, 0x467, 0x14, 0x0);
}

static int rtl8218b_config_init(struct phy_device *phydev)
{
	int oldpage, oldxpage;

	rtl821x_config_init(phydev);

	if (phydev->mdio.addr % 8)
		return 0;
	/*
	 * Realtek provides two ways of initializing the PHY package. Either by U-Boot or via
	 * vendor software and SDK. In case U-Boot setup is missing, run basic configuration
	 * so that ports at least get link up and pass traffic.
	 */

	oldpage = phy_read(phydev, RTL821x_PAGE_SELECT);
	oldxpage = phy_read(phydev, RTL821x_EXT_PAGE_SELECT);
	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, 0x8);

	/* activate 32/40 bit redundancy algorithm for first MAC serdes */
	phy_modify_paged(phydev, RTL821X_MAC_SDS_PAGE(0, 1), 0x14, 0, BIT(3));
	/* magic CMU setting for stable connectivity of first MAC serdes */
	phy_write_paged(phydev, 0x462, 0x15, 0x6e58);
	/* magic setting for rate select 10G full */
	phy_write_paged(phydev, 0x464, 0x15, 0x202a);
	/* magic setting for variable gain amplifier */
	phy_modify_paged(phydev, 0x464, 0x12, 0, 0x1f80);
	/* magic setting for equalizer of second MAC serdes */
	phy_write_paged(phydev, RTL821X_MAC_SDS_PAGE(1, 8), 0x12, 0x2020);
	/* unknown magic for second MAC serdes */
	phy_write_paged(phydev, RTL821X_MAC_SDS_PAGE(1, 9), 0x11, 0xc014);
	rtl8218b_cmu_reset(phydev, 0);

	for (int sds = 0; sds < 2; sds++) {
		/* disable ring PLL for serdes 2+3 */
		phy_modify_paged(phydev, RTL821X_MAC_SDS_PAGE(sds + 1, 8), 0x11, BIT(15), 0);
		/* force negative clock edge */
		phy_modify_paged(phydev, RTL821X_MAC_SDS_PAGE(sds, 0), 0x17, 0, BIT(14));
		rtl8218b_cmu_reset(phydev, 5 + sds);
		/* soft reset */
		phy_modify_paged(phydev, RTL821X_MAC_SDS_PAGE(sds, 0), 0x13, 0, BIT(6));
		phy_modify_paged(phydev, RTL821X_MAC_SDS_PAGE(sds, 0), 0x13, BIT(6), 0);
	}

	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, oldxpage);
	phy_write(phydev, RTL821x_PAGE_SELECT, oldpage);

	return 0;
}

static int rtl8214fc_phy_probe(struct phy_device *phydev)
{
	static int regs[] = {16, 19, 20, 21};
	int ret;

	rtl821x_package_join(phydev, 4);

	/*
	 * Normally phy_probe() only initializes PHY structures and setup is run in
	 * config_init(). The RTL8214FC needs configuration before SFP probing while
	 * the preferred media is still copper. This way all SFP events (even before
	 * the first config_init()) will find a consistent port state.
	 */

	/* Step 1 - package setup: Due to similar design reuse RTL8218B coding */
	ret = rtl8218b_config_init(phydev);
	if (ret)
		return ret;

	if (phydev->mdio.addr % 8 == 0) {
		/* Force all ports to copper */
		phy_write(phydev, RTL821x_EXT_PAGE_SELECT, RTL821X_MEDIA_PAGE_INTERNAL);
		for (int port = 0; port < 4; port++)
			phy_modify_paged(phydev, 0x266, regs[port], 0, GENMASK(11, 10));
	}

	/* Step 2 - port setup */
	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, RTL821X_MEDIA_PAGE_FIBRE);
	/* set fiber SerDes RX to negative edge */
	phy_modify_paged(phydev, 0x8, 0x17, 0, BIT(14));
	/* auto negotiation disable link on */
	phy_modify_paged(phydev, 0x8, 0x14, 0, BIT(2));
	/* disable fiber 100MBit */
	phy_modify_paged(phydev, 0x8, 0x11, BIT(5), 0);
	phy_write(phydev, RTL821x_EXT_PAGE_SELECT, RTL821X_MEDIA_PAGE_AUTO);

	/* Disable EEE. 0xa5d/0x10 is the same as MDIO_MMD_AN / MDIO_AN_EEE_ADV */
	phy_write_paged(phydev, 0xa5d, 0x10, 0x0000);

	return phy_sfp_probe(phydev, &rtl8214fc_sfp_ops);
}

static struct phy_driver rtl83xx_phy_driver[] = {
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8214C),
		.name		= "Realtek RTL8214C",
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8214c_phy_probe,
		.read_page	= rtl821x_read_page,
		.write_page	= rtl821x_write_page,
		.suspend	= genphy_suspend,
		.resume		= genphy_resume,
	},
	{
		.match_phy_device = rtl8214fc_match_phy_device,
		.name		= "Realtek RTL8214FC",
		.config_aneg	= rtl8214fc_config_aneg,
		.get_features	= rtl8214fc_get_features,
		.get_tunable    = rtl8214fc_get_tunable,
		.probe		= rtl8214fc_phy_probe,
		.read_mmd	= rtl8214fc_read_mmd,
		.read_page	= rtl821x_read_page,
		.read_status    = rtl8214fc_read_status,
		.resume		= rtl8214fc_resume,
		.set_tunable	= rtl8214fc_set_tunable,
		.suspend	= rtl8214fc_suspend,
		.write_mmd	= rtl8214fc_write_mmd,
		.write_page	= rtl821x_write_page,
	},
	{
		.match_phy_device = rtl8218b_ext_match_phy_device,
		.name		= "Realtek RTL8218B (external)",
		.config_init	= rtl8218b_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218x_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
	},
	{
		PHY_ID_MATCH_MODEL(PHY_ID_RTL8218B_I),
		.name		= "Realtek RTL8218B (internal)",
		.config_init	= rtl821x_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218x_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
	},
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8218D),
		.name		= "REALTEK RTL8218D",
		.config_init	= rtl8218d_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218x_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
	},
	{
		PHY_ID_MATCH_EXACT(PHY_ID_RTL8218E),
		.name		= "REALTEK RTL8218E",
		.config_init	= rtl821x_config_init,
		.features	= PHY_GBIT_FEATURES,
		.probe		= rtl8218x_phy_probe,
		.read_mmd	= rtl821x_read_mmd,
		.read_page	= rtl821x_read_page,
		.resume		= genphy_resume,
		.suspend	= genphy_suspend,
		.write_mmd	= rtl821x_write_mmd,
		.write_page	= rtl821x_write_page,
	},
};

module_phy_driver(rtl83xx_phy_driver);

static const struct mdio_device_id __maybe_unused rtl83xx_tbl[] = {
	{ PHY_ID_MATCH_MODEL(PHY_ID_RTL8214_OR_8218) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, rtl83xx_tbl);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL83xx PHY driver");
MODULE_LICENSE("GPL");
