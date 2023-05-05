// SPDX-License-Identifier: GPL-2.0-only
/*
 *
 *   Copyright (C) 2009-2016 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2016 Felix Fietkau <nbd@openwrt.org>
 *   Copyright (C) 2013-2016 Michael Lee <igvtee@gmail.com>
 */

#include <linux/of_device.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/mfd/syscon.h>
#include <linux/regmap.h>
#include <linux/clk.h>
#include <linux/pm_runtime.h>
#include <linux/if_vlan.h>
#include <linux/reset.h>
#include <linux/tcp.h>
#include <linux/interrupt.h>
#include <linux/pinctrl/devinfo.h>
#include <linux/phylink.h>
#include <net/dsa.h>

#include "mtk_eth_soc.h"
#include "mtk_eth_dbg.h"
#include "mtk_eth_reset.h"

#if defined(CONFIG_NET_MEDIATEK_HNAT) || defined(CONFIG_NET_MEDIATEK_HNAT_MODULE)
#include "mtk_hnat/nf_hnat_mtk.h"
#endif

static int mtk_msg_level = -1;
atomic_t reset_lock = ATOMIC_INIT(0);
atomic_t force = ATOMIC_INIT(0);

module_param_named(msg_level, mtk_msg_level, int, 0);
MODULE_PARM_DESC(msg_level, "Message level (-1=defaults,0=none,...,16=all)");
DECLARE_COMPLETION(wait_ser_done);

#define MTK_ETHTOOL_STAT(x) { #x, \
			      offsetof(struct mtk_hw_stats, x) / sizeof(u64) }

/* strings used by ethtool */
static const struct mtk_ethtool_stats {
	char str[ETH_GSTRING_LEN];
	u32 offset;
} mtk_ethtool_stats[] = {
	MTK_ETHTOOL_STAT(tx_bytes),
	MTK_ETHTOOL_STAT(tx_packets),
	MTK_ETHTOOL_STAT(tx_skip),
	MTK_ETHTOOL_STAT(tx_collisions),
	MTK_ETHTOOL_STAT(rx_bytes),
	MTK_ETHTOOL_STAT(rx_packets),
	MTK_ETHTOOL_STAT(rx_overflow),
	MTK_ETHTOOL_STAT(rx_fcs_errors),
	MTK_ETHTOOL_STAT(rx_short_errors),
	MTK_ETHTOOL_STAT(rx_long_errors),
	MTK_ETHTOOL_STAT(rx_checksum_errors),
	MTK_ETHTOOL_STAT(rx_flow_control_packets),
};

static const char * const mtk_clks_source_name[] = {
	"ethif", "sgmiitop", "esw", "gp0", "gp1", "gp2", "fe", "trgpll",
	"sgmii_tx250m", "sgmii_rx250m", "sgmii_cdr_ref", "sgmii_cdr_fb",
	"sgmii2_tx250m", "sgmii2_rx250m", "sgmii2_cdr_ref", "sgmii2_cdr_fb",
	"sgmii_ck", "eth2pll", "wocpu0","wocpu1",
};

void mtk_w32(struct mtk_eth *eth, u32 val, unsigned reg)
{
	__raw_writel(val, eth->base + reg);
}

u32 mtk_r32(struct mtk_eth *eth, unsigned reg)
{
	return __raw_readl(eth->base + reg);
}

u32 mtk_m32(struct mtk_eth *eth, u32 mask, u32 set, unsigned reg)
{
	u32 val;

	val = mtk_r32(eth, reg);
	val &= ~mask;
	val |= set;
	mtk_w32(eth, val, reg);
	return reg;
}

static int mtk_mdio_busy_wait(struct mtk_eth *eth)
{
	unsigned long t_start = jiffies;

	while (1) {
		if (!(mtk_r32(eth, MTK_PHY_IAC) & PHY_IAC_ACCESS))
			return 0;
		if (time_after(jiffies, t_start + PHY_IAC_TIMEOUT))
			break;
		cond_resched();
	}

	dev_err(eth->dev, "mdio: MDIO timeout\n");
	return -1;
}

u32 _mtk_mdio_write(struct mtk_eth *eth, u16 phy_addr,
			   u16 phy_register, u16 write_data)
{
	if (mtk_mdio_busy_wait(eth))
		return -1;

	write_data &= 0xffff;

	mtk_w32(eth, PHY_IAC_ACCESS | PHY_IAC_START | PHY_IAC_WRITE |
		((phy_register & 0x1f) << PHY_IAC_REG_SHIFT) |
		((phy_addr & 0x1f) << PHY_IAC_ADDR_SHIFT) | write_data,
		MTK_PHY_IAC);

	if (mtk_mdio_busy_wait(eth))
		return -1;

	return 0;
}

u32 _mtk_mdio_read(struct mtk_eth *eth, u16 phy_addr, u16 phy_reg)
{
	u32 d;

	if (mtk_mdio_busy_wait(eth))
		return 0xffff;

	mtk_w32(eth, PHY_IAC_ACCESS | PHY_IAC_START | PHY_IAC_READ |
		((phy_reg & 0x1f) << PHY_IAC_REG_SHIFT) |
		((phy_addr & 0x1f) << PHY_IAC_ADDR_SHIFT),
		MTK_PHY_IAC);

	if (mtk_mdio_busy_wait(eth))
		return 0xffff;

	d = mtk_r32(eth, MTK_PHY_IAC) & 0xffff;

	return d;
}

static int mtk_mdio_write(struct mii_bus *bus, int phy_addr,
			  int phy_reg, u16 val)
{
	struct mtk_eth *eth = bus->priv;

	return _mtk_mdio_write(eth, phy_addr, phy_reg, val);
}

static int mtk_mdio_read(struct mii_bus *bus, int phy_addr, int phy_reg)
{
	struct mtk_eth *eth = bus->priv;

	return _mtk_mdio_read(eth, phy_addr, phy_reg);
}

u32 mtk_cl45_ind_read(struct mtk_eth *eth, u16 port, u16 devad, u16 reg, u16 *data)
{
        mutex_lock(&eth->mii_bus->mdio_lock);

        _mtk_mdio_write(eth, port, MII_MMD_ACC_CTL_REG, devad);
        _mtk_mdio_write(eth, port, MII_MMD_ADDR_DATA_REG, reg);
        _mtk_mdio_write(eth, port, MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
        *data = _mtk_mdio_read(eth, port, MII_MMD_ADDR_DATA_REG);

        mutex_unlock(&eth->mii_bus->mdio_lock);

        return 0;
}

u32 mtk_cl45_ind_write(struct mtk_eth *eth, u16 port, u16 devad, u16 reg, u16 data)
{
        mutex_lock(&eth->mii_bus->mdio_lock);

        _mtk_mdio_write(eth, port, MII_MMD_ACC_CTL_REG, devad);
        _mtk_mdio_write(eth, port, MII_MMD_ADDR_DATA_REG, reg);
        _mtk_mdio_write(eth, port, MII_MMD_ACC_CTL_REG, MMD_OP_MODE_DATA | devad);
        _mtk_mdio_write(eth, port, MII_MMD_ADDR_DATA_REG, data);

        mutex_unlock(&eth->mii_bus->mdio_lock);

        return 0;
}

static int mt7621_gmac0_rgmii_adjust(struct mtk_eth *eth,
				     phy_interface_t interface)
{
	u32 val;

	/* Check DDR memory type.
	 * Currently TRGMII mode with DDR2 memory is not supported.
	 */
	regmap_read(eth->ethsys, ETHSYS_SYSCFG, &val);
	if (interface == PHY_INTERFACE_MODE_TRGMII &&
	    val & SYSCFG_DRAM_TYPE_DDR2) {
		dev_err(eth->dev,
			"TRGMII mode with DDR2 memory is not supported!\n");
		return -EOPNOTSUPP;
	}

	val = (interface == PHY_INTERFACE_MODE_TRGMII) ?
		ETHSYS_TRGMII_MT7621_DDR_PLL : 0;

	regmap_update_bits(eth->ethsys, ETHSYS_CLKCFG0,
			   ETHSYS_TRGMII_MT7621_MASK, val);

	return 0;
}

static void mtk_gmac0_rgmii_adjust(struct mtk_eth *eth,
				   phy_interface_t interface, int speed)
{
	u32 val;
	int ret;

	if (interface == PHY_INTERFACE_MODE_TRGMII) {
		mtk_w32(eth, TRGMII_MODE, INTF_MODE);
		val = 500000000;
		ret = clk_set_rate(eth->clks[MTK_CLK_TRGPLL], val);
		if (ret)
			dev_err(eth->dev, "Failed to set trgmii pll: %d\n", ret);
		return;
	}

	val = (speed == SPEED_1000) ?
		INTF_MODE_RGMII_1000 : INTF_MODE_RGMII_10_100;
	mtk_w32(eth, val, INTF_MODE);

	regmap_update_bits(eth->ethsys, ETHSYS_CLKCFG0,
			   ETHSYS_TRGMII_CLK_SEL362_5,
			   ETHSYS_TRGMII_CLK_SEL362_5);

	val = (speed == SPEED_1000) ? 250000000 : 500000000;
	ret = clk_set_rate(eth->clks[MTK_CLK_TRGPLL], val);
	if (ret)
		dev_err(eth->dev, "Failed to set trgmii pll: %d\n", ret);

	val = (speed == SPEED_1000) ?
		RCK_CTRL_RGMII_1000 : RCK_CTRL_RGMII_10_100;
	mtk_w32(eth, val, TRGMII_RCK_CTRL);

	val = (speed == SPEED_1000) ?
		TCK_CTRL_RGMII_1000 : TCK_CTRL_RGMII_10_100;
	mtk_w32(eth, val, TRGMII_TCK_CTRL);
}

static void mtk_mac_config(struct phylink_config *config, unsigned int mode,
			   const struct phylink_link_state *state)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);
	struct mtk_eth *eth = mac->hw;
	u32 mcr_cur, mcr_new, sid, i;
	int val, ge_mode, err=0;

	/* MT76x8 has no hardware settings between for the MAC */
	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628) &&
	    mac->interface != state->interface) {
		/* Setup soc pin functions */
		switch (state->interface) {
		case PHY_INTERFACE_MODE_TRGMII:
			if (mac->id)
				goto err_phy;
			if (!MTK_HAS_CAPS(mac->hw->soc->caps,
					  MTK_GMAC1_TRGMII))
				goto err_phy;
			/* fall through */
		case PHY_INTERFACE_MODE_RGMII_TXID:
		case PHY_INTERFACE_MODE_RGMII_RXID:
		case PHY_INTERFACE_MODE_RGMII_ID:
		case PHY_INTERFACE_MODE_RGMII:
		case PHY_INTERFACE_MODE_MII:
		case PHY_INTERFACE_MODE_REVMII:
		case PHY_INTERFACE_MODE_RMII:
			if (MTK_HAS_CAPS(eth->soc->caps, MTK_RGMII)) {
				err = mtk_gmac_rgmii_path_setup(eth, mac->id);
				if (err)
					goto init_err;
			}
			break;
		case PHY_INTERFACE_MODE_1000BASEX:
		case PHY_INTERFACE_MODE_2500BASEX:
		case PHY_INTERFACE_MODE_SGMII:
			if (MTK_HAS_CAPS(eth->soc->caps, MTK_SGMII)) {
				err = mtk_gmac_sgmii_path_setup(eth, mac->id);
				if (err)
					goto init_err;
			}
			break;
		case PHY_INTERFACE_MODE_GMII:
			if (MTK_HAS_CAPS(eth->soc->caps, MTK_GEPHY)) {
				err = mtk_gmac_gephy_path_setup(eth, mac->id);
				if (err)
					goto init_err;
			}
			break;
		default:
			goto err_phy;
		}

		/* Setup clock for 1st gmac */
		if (!mac->id && state->interface != PHY_INTERFACE_MODE_SGMII &&
		    !phy_interface_mode_is_8023z(state->interface) &&
		    MTK_HAS_CAPS(mac->hw->soc->caps, MTK_GMAC1_TRGMII)) {
			if (MTK_HAS_CAPS(mac->hw->soc->caps,
					 MTK_TRGMII_MT7621_CLK)) {
				if (mt7621_gmac0_rgmii_adjust(mac->hw,
							      state->interface))
					goto err_phy;
			} else {
				mtk_gmac0_rgmii_adjust(mac->hw,
						       state->interface,
						       state->speed);

				/* mt7623_pad_clk_setup */
				for (i = 0 ; i < NUM_TRGMII_CTRL; i++)
					mtk_w32(mac->hw,
						TD_DM_DRVP(8) | TD_DM_DRVN(8),
						TRGMII_TD_ODT(i));

				/* Assert/release MT7623 RXC reset */
				mtk_m32(mac->hw, 0, RXC_RST | RXC_DQSISEL,
					TRGMII_RCK_CTRL);
				mtk_m32(mac->hw, RXC_RST, 0, TRGMII_RCK_CTRL);
			}
		}

		ge_mode = 0;
		switch (state->interface) {
		case PHY_INTERFACE_MODE_MII:
		case PHY_INTERFACE_MODE_GMII:
			ge_mode = 1;
			break;
		case PHY_INTERFACE_MODE_REVMII:
			ge_mode = 2;
			break;
		case PHY_INTERFACE_MODE_RMII:
			if (mac->id)
				goto err_phy;
			ge_mode = 3;
			break;
		default:
			break;
		}

		/* put the gmac into the right mode */
		spin_lock(&eth->syscfg0_lock);
		regmap_read(eth->ethsys, ETHSYS_SYSCFG0, &val);
		val &= ~SYSCFG0_GE_MODE(SYSCFG0_GE_MASK, mac->id);
		val |= SYSCFG0_GE_MODE(ge_mode, mac->id);
		regmap_write(eth->ethsys, ETHSYS_SYSCFG0, val);
		spin_unlock(&eth->syscfg0_lock);

		mac->interface = state->interface;
	}

	/* SGMII */
	if (state->interface == PHY_INTERFACE_MODE_SGMII ||
	    phy_interface_mode_is_8023z(state->interface)) {
		/* The path GMAC to SGMII will be enabled once the SGMIISYS is
		 * being setup done.
		 */
		spin_lock(&eth->syscfg0_lock);
		regmap_read(eth->ethsys, ETHSYS_SYSCFG0, &val);

		regmap_update_bits(eth->ethsys, ETHSYS_SYSCFG0,
				   SYSCFG0_SGMII_MASK,
				   ~(u32)SYSCFG0_SGMII_MASK);

		/* Decide how GMAC and SGMIISYS be mapped */
		sid = (MTK_HAS_CAPS(eth->soc->caps, MTK_SHARED_SGMII)) ?
		       0 : mac->id;

		/* Setup SGMIISYS with the determined property */
		if (state->interface != PHY_INTERFACE_MODE_SGMII)
			err = mtk_sgmii_setup_mode_force(eth->sgmii, sid,
							 state);
		else if (phylink_autoneg_inband(mode))
			err = mtk_sgmii_setup_mode_an(eth->sgmii, sid);

		if (err) {
			spin_unlock(&eth->syscfg0_lock);
			goto init_err;
		}

		regmap_update_bits(eth->ethsys, ETHSYS_SYSCFG0,
				   SYSCFG0_SGMII_MASK, val);
		spin_unlock(&eth->syscfg0_lock);
	} else if (phylink_autoneg_inband(mode)) {
		dev_err(eth->dev,
			"In-band mode not supported in non SGMII mode!\n");
		return;
	}

	/* Setup gmac */
	mcr_cur = mtk_r32(mac->hw, MTK_MAC_MCR(mac->id));
	mcr_new = mcr_cur;
	mcr_new &= ~(MAC_MCR_SPEED_100 | MAC_MCR_SPEED_1000 |
		     MAC_MCR_FORCE_DPX | MAC_MCR_FORCE_TX_FC |
		     MAC_MCR_FORCE_RX_FC);
	mcr_new |= MAC_MCR_MAX_RX_1536 | MAC_MCR_IPG_CFG | MAC_MCR_FORCE_MODE |
		   MAC_MCR_BACKOFF_EN | MAC_MCR_BACKPR_EN | MAC_MCR_FORCE_LINK;

	switch (state->speed) {
	case SPEED_2500:
	case SPEED_1000:
		mcr_new |= MAC_MCR_SPEED_1000;
		break;
	case SPEED_100:
		mcr_new |= MAC_MCR_SPEED_100;
		break;
	}
	if (state->duplex == DUPLEX_FULL) {
		mcr_new |= MAC_MCR_FORCE_DPX;
		if (state->pause & MLO_PAUSE_TX)
			mcr_new |= MAC_MCR_FORCE_TX_FC;
		if (state->pause & MLO_PAUSE_RX)
			mcr_new |= MAC_MCR_FORCE_RX_FC;
	}

	/* Only update control register when needed! */
	if (mcr_new != mcr_cur)
		mtk_w32(mac->hw, mcr_new, MTK_MAC_MCR(mac->id));

	return;

err_phy:
	dev_err(eth->dev, "%s: GMAC%d mode %s not supported!\n", __func__,
		mac->id, phy_modes(state->interface));
	return;

init_err:
	dev_err(eth->dev, "%s: GMAC%d mode %s err: %d!\n", __func__,
		mac->id, phy_modes(state->interface), err);
}

static int mtk_mac_link_state(struct phylink_config *config,
			      struct phylink_link_state *state)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);
	u32 pmsr = mtk_r32(mac->hw, MTK_MAC_MSR(mac->id));

	state->link = (pmsr & MAC_MSR_LINK);
	state->duplex = (pmsr & MAC_MSR_DPX) >> 1;

	switch (pmsr & (MAC_MSR_SPEED_1000 | MAC_MSR_SPEED_100)) {
	case 0:
		state->speed = SPEED_10;
		break;
	case MAC_MSR_SPEED_100:
		state->speed = SPEED_100;
		break;
	case MAC_MSR_SPEED_1000:
		state->speed = SPEED_1000;
		break;
	default:
		state->speed = SPEED_UNKNOWN;
		break;
	}

	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (pmsr & MAC_MSR_RX_FC)
		state->pause |= MLO_PAUSE_RX;
	if (pmsr & MAC_MSR_TX_FC)
		state->pause |= MLO_PAUSE_TX;

	return 1;
}

static void mtk_mac_an_restart(struct phylink_config *config)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);

	mtk_sgmii_restart_an(mac->hw, mac->id);
}

static void mtk_mac_link_down(struct phylink_config *config, unsigned int mode,
			      phy_interface_t interface)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);
	u32 mcr = mtk_r32(mac->hw, MTK_MAC_MCR(mac->id));

	mcr &= ~(MAC_MCR_TX_EN | MAC_MCR_RX_EN);
	mtk_w32(mac->hw, mcr, MTK_MAC_MCR(mac->id));
}

static void mtk_mac_link_up(struct phylink_config *config, unsigned int mode,
			    phy_interface_t interface,
			    struct phy_device *phy)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);
	u32 mcr = mtk_r32(mac->hw, MTK_MAC_MCR(mac->id));

	mcr |= MAC_MCR_TX_EN | MAC_MCR_RX_EN;
	mtk_w32(mac->hw, mcr, MTK_MAC_MCR(mac->id));
}

static void mtk_validate(struct phylink_config *config,
			 unsigned long *supported,
			 struct phylink_link_state *state)
{
	struct mtk_mac *mac = container_of(config, struct mtk_mac,
					   phylink_config);
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	if (state->interface != PHY_INTERFACE_MODE_NA &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    !(MTK_HAS_CAPS(mac->hw->soc->caps, MTK_RGMII) &&
	      phy_interface_mode_is_rgmii(state->interface)) &&
	    !(MTK_HAS_CAPS(mac->hw->soc->caps, MTK_TRGMII) &&
	      !mac->id && state->interface == PHY_INTERFACE_MODE_TRGMII) &&
	    !(MTK_HAS_CAPS(mac->hw->soc->caps, MTK_SGMII) &&
	      (state->interface == PHY_INTERFACE_MODE_SGMII ||
	       phy_interface_mode_is_8023z(state->interface)))) {
		linkmode_zero(supported);
		return;
	}

	phylink_set_port_modes(mask);
	phylink_set(mask, Autoneg);

	switch (state->interface) {
	case PHY_INTERFACE_MODE_TRGMII:
		phylink_set(mask, 1000baseT_Full);
		break;
	case PHY_INTERFACE_MODE_1000BASEX:
	case PHY_INTERFACE_MODE_2500BASEX:
		phylink_set(mask, 1000baseX_Full);
		phylink_set(mask, 2500baseX_Full);
		break;
	case PHY_INTERFACE_MODE_GMII:
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		phylink_set(mask, 1000baseT_Half);
		/* fall through */
	case PHY_INTERFACE_MODE_SGMII:
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseX_Full);
		/* fall through */
	case PHY_INTERFACE_MODE_MII:
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_REVMII:
	case PHY_INTERFACE_MODE_NA:
	default:
		phylink_set(mask, 10baseT_Half);
		phylink_set(mask, 10baseT_Full);
		phylink_set(mask, 100baseT_Half);
		phylink_set(mask, 100baseT_Full);
		break;
	}

	if (state->interface == PHY_INTERFACE_MODE_NA) {
		if (MTK_HAS_CAPS(mac->hw->soc->caps, MTK_SGMII)) {
			phylink_set(mask, 1000baseT_Full);
			phylink_set(mask, 1000baseX_Full);
			phylink_set(mask, 2500baseX_Full);
		}
		if (MTK_HAS_CAPS(mac->hw->soc->caps, MTK_RGMII)) {
			phylink_set(mask, 1000baseT_Full);
			phylink_set(mask, 1000baseT_Half);
			phylink_set(mask, 1000baseX_Full);
		}
		if (MTK_HAS_CAPS(mac->hw->soc->caps, MTK_GEPHY)) {
			phylink_set(mask, 1000baseT_Full);
			phylink_set(mask, 1000baseT_Half);
		}
	}

	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	linkmode_and(supported, supported, mask);
	linkmode_and(state->advertising, state->advertising, mask);

	/* We can only operate at 2500BaseX or 1000BaseX. If requested
	 * to advertise both, only report advertising at 2500BaseX.
	 */
	phylink_helper_basex_speed(state);
}

static const struct phylink_mac_ops mtk_phylink_ops = {
	.validate = mtk_validate,
	.mac_link_state = mtk_mac_link_state,
	.mac_an_restart = mtk_mac_an_restart,
	.mac_config = mtk_mac_config,
	.mac_link_down = mtk_mac_link_down,
	.mac_link_up = mtk_mac_link_up,
};

static int mtk_mdio_init(struct mtk_eth *eth)
{
	struct device_node *mii_np;
	int ret;

	mii_np = of_get_child_by_name(eth->dev->of_node, "mdio-bus");
	if (!mii_np) {
		dev_err(eth->dev, "no %s child node found", "mdio-bus");
		return -ENODEV;
	}

	if (!of_device_is_available(mii_np)) {
		ret = -ENODEV;
		goto err_put_node;
	}

	eth->mii_bus = devm_mdiobus_alloc(eth->dev);
	if (!eth->mii_bus) {
		ret = -ENOMEM;
		goto err_put_node;
	}

	eth->mii_bus->name = "mdio";
	eth->mii_bus->read = mtk_mdio_read;
	eth->mii_bus->write = mtk_mdio_write;
	eth->mii_bus->priv = eth;
	eth->mii_bus->parent = eth->dev;

	if(snprintf(eth->mii_bus->id, MII_BUS_ID_SIZE, "%pOFn", mii_np) < 0) {
		ret = -ENOMEM;
		goto err_put_node;
	}
	ret = of_mdiobus_register(eth->mii_bus, mii_np);

err_put_node:
	of_node_put(mii_np);
	return ret;
}

static void mtk_mdio_cleanup(struct mtk_eth *eth)
{
	if (!eth->mii_bus)
		return;

	mdiobus_unregister(eth->mii_bus);
}

static inline void mtk_tx_irq_disable(struct mtk_eth *eth, u32 mask)
{
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&eth->tx_irq_lock, flags);
	val = mtk_r32(eth, eth->tx_int_mask_reg);
	mtk_w32(eth, val & ~mask, eth->tx_int_mask_reg);
	spin_unlock_irqrestore(&eth->tx_irq_lock, flags);
}

static inline void mtk_tx_irq_enable(struct mtk_eth *eth, u32 mask)
{
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&eth->tx_irq_lock, flags);
	val = mtk_r32(eth, eth->tx_int_mask_reg);
	mtk_w32(eth, val | mask, eth->tx_int_mask_reg);
	spin_unlock_irqrestore(&eth->tx_irq_lock, flags);
}

static inline void mtk_rx_irq_disable(struct mtk_eth *eth, u32 mask)
{
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&eth->rx_irq_lock, flags);
	val = mtk_r32(eth, MTK_PDMA_INT_MASK);
	mtk_w32(eth, val & ~mask, MTK_PDMA_INT_MASK);
	spin_unlock_irqrestore(&eth->rx_irq_lock, flags);
}

static inline void mtk_rx_irq_enable(struct mtk_eth *eth, u32 mask)
{
	unsigned long flags;
	u32 val;

	spin_lock_irqsave(&eth->rx_irq_lock, flags);
	val = mtk_r32(eth, MTK_PDMA_INT_MASK);
	mtk_w32(eth, val | mask, MTK_PDMA_INT_MASK);
	spin_unlock_irqrestore(&eth->rx_irq_lock, flags);
}

static int mtk_set_mac_address(struct net_device *dev, void *p)
{
	int ret = eth_mac_addr(dev, p);
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	const char *macaddr = dev->dev_addr;

	if (ret)
		return ret;

	if (unlikely(test_bit(MTK_RESETTING, &mac->hw->state)))
		return -EBUSY;

	spin_lock_bh(&mac->hw->page_lock);
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
		mtk_w32(mac->hw, (macaddr[0] << 8) | macaddr[1],
			MT7628_SDM_MAC_ADRH);
		mtk_w32(mac->hw, (macaddr[2] << 24) | (macaddr[3] << 16) |
			(macaddr[4] << 8) | macaddr[5],
			MT7628_SDM_MAC_ADRL);
	} else {
		mtk_w32(mac->hw, (macaddr[0] << 8) | macaddr[1],
			MTK_GDMA_MAC_ADRH(mac->id));
		mtk_w32(mac->hw, (macaddr[2] << 24) | (macaddr[3] << 16) |
			(macaddr[4] << 8) | macaddr[5],
			MTK_GDMA_MAC_ADRL(mac->id));
	}
	spin_unlock_bh(&mac->hw->page_lock);

	return 0;
}

void mtk_stats_update_mac(struct mtk_mac *mac)
{
	struct mtk_hw_stats *hw_stats = mac->hw_stats;
	unsigned int base = MTK_GDM1_TX_GBCNT;
	u64 stats;

	base += hw_stats->reg_offset;

	u64_stats_update_begin(&hw_stats->syncp);

	hw_stats->rx_bytes += mtk_r32(mac->hw, base);
	stats =  mtk_r32(mac->hw, base + 0x04);
	if (stats)
		hw_stats->rx_bytes += (stats << 32);
	hw_stats->rx_packets += mtk_r32(mac->hw, base + 0x08);
	hw_stats->rx_overflow += mtk_r32(mac->hw, base + 0x10);
	hw_stats->rx_fcs_errors += mtk_r32(mac->hw, base + 0x14);
	hw_stats->rx_short_errors += mtk_r32(mac->hw, base + 0x18);
	hw_stats->rx_long_errors += mtk_r32(mac->hw, base + 0x1c);
	hw_stats->rx_checksum_errors += mtk_r32(mac->hw, base + 0x20);
	hw_stats->rx_flow_control_packets +=
					mtk_r32(mac->hw, base + 0x24);
	hw_stats->tx_skip += mtk_r32(mac->hw, base + 0x28);
	hw_stats->tx_collisions += mtk_r32(mac->hw, base + 0x2c);
	hw_stats->tx_bytes += mtk_r32(mac->hw, base + 0x30);
	stats =  mtk_r32(mac->hw, base + 0x34);
	if (stats)
		hw_stats->tx_bytes += (stats << 32);
	hw_stats->tx_packets += mtk_r32(mac->hw, base + 0x38);
	u64_stats_update_end(&hw_stats->syncp);
}

static void mtk_stats_update(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->mac[i] || !eth->mac[i]->hw_stats)
			continue;
		if (spin_trylock(&eth->mac[i]->hw_stats->stats_lock)) {
			mtk_stats_update_mac(eth->mac[i]);
			spin_unlock(&eth->mac[i]->hw_stats->stats_lock);
		}
	}
}

static void mtk_get_stats64(struct net_device *dev,
			    struct rtnl_link_stats64 *storage)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_hw_stats *hw_stats = mac->hw_stats;
	unsigned int start;

	if (netif_running(dev) && netif_device_present(dev)) {
		if (spin_trylock_bh(&hw_stats->stats_lock)) {
			mtk_stats_update_mac(mac);
			spin_unlock_bh(&hw_stats->stats_lock);
		}
	}

	do {
		start = u64_stats_fetch_begin_irq(&hw_stats->syncp);
		storage->rx_packets = hw_stats->rx_packets;
		storage->tx_packets = hw_stats->tx_packets;
		storage->rx_bytes = hw_stats->rx_bytes;
		storage->tx_bytes = hw_stats->tx_bytes;
		storage->collisions = hw_stats->tx_collisions;
		storage->rx_length_errors = hw_stats->rx_short_errors +
			hw_stats->rx_long_errors;
		storage->rx_over_errors = hw_stats->rx_overflow;
		storage->rx_crc_errors = hw_stats->rx_fcs_errors;
		storage->rx_errors = hw_stats->rx_checksum_errors;
		storage->tx_aborted_errors = hw_stats->tx_skip;
	} while (u64_stats_fetch_retry_irq(&hw_stats->syncp, start));

	storage->tx_errors = dev->stats.tx_errors;
	storage->rx_dropped = dev->stats.rx_dropped;
	storage->tx_dropped = dev->stats.tx_dropped;
}

static inline int mtk_max_frag_size(int mtu)
{
	/* make sure buf_size will be at least MTK_MAX_RX_LENGTH */
	if (mtu + MTK_RX_ETH_HLEN < MTK_MAX_RX_LENGTH)
		mtu = MTK_MAX_RX_LENGTH - MTK_RX_ETH_HLEN;

	return SKB_DATA_ALIGN(MTK_RX_HLEN + mtu) +
		SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
}

static inline int mtk_max_buf_size(int frag_size)
{
	int buf_size = frag_size - NET_SKB_PAD - NET_IP_ALIGN -
		       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));

	WARN_ON(buf_size < MTK_MAX_RX_LENGTH);

	return buf_size;
}

static inline bool mtk_rx_get_desc(struct mtk_rx_dma *rxd,
				   struct mtk_rx_dma *dma_rxd)
{
	rxd->rxd2 = READ_ONCE(dma_rxd->rxd2);
	if (!(rxd->rxd2 & RX_DMA_DONE))
		return false;

	rxd->rxd1 = READ_ONCE(dma_rxd->rxd1);
	rxd->rxd3 = READ_ONCE(dma_rxd->rxd3);
	rxd->rxd4 = READ_ONCE(dma_rxd->rxd4);
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	rxd->rxd5 = READ_ONCE(dma_rxd->rxd5);
	rxd->rxd6 = READ_ONCE(dma_rxd->rxd6);
#endif
	return true;
}

/* the qdma core needs scratch memory to be setup */
static int mtk_init_fq_dma(struct mtk_eth *eth)
{
	dma_addr_t phy_ring_tail;
	int cnt = MTK_DMA_SIZE;
	dma_addr_t dma_addr;
	int i;

	if (!eth->soc->has_sram) {
		eth->scratch_ring = dma_alloc_coherent(eth->dev,
					       cnt * sizeof(struct mtk_tx_dma),
					       &eth->phy_scratch_ring,
					       GFP_ATOMIC);
	} else {
		eth->scratch_ring = eth->base + MTK_ETH_SRAM_OFFSET;
	}

	if (unlikely(!eth->scratch_ring))
                        return -ENOMEM;

	eth->scratch_head = kcalloc(cnt, MTK_QDMA_PAGE_SIZE,
				    GFP_KERNEL);
	if (unlikely(!eth->scratch_head))
		return -ENOMEM;

	dma_addr = dma_map_single(eth->dev,
				  eth->scratch_head, cnt * MTK_QDMA_PAGE_SIZE,
				  DMA_FROM_DEVICE);
	if (unlikely(dma_mapping_error(eth->dev, dma_addr)))
		return -ENOMEM;

	phy_ring_tail = eth->phy_scratch_ring +
			(sizeof(struct mtk_tx_dma) * (cnt - 1));

	for (i = 0; i < cnt; i++) {
		eth->scratch_ring[i].txd1 =
				(dma_addr + (i * MTK_QDMA_PAGE_SIZE));
		if (i < cnt - 1)
			eth->scratch_ring[i].txd2 = (eth->phy_scratch_ring +
				((i + 1) * sizeof(struct mtk_tx_dma)));
		eth->scratch_ring[i].txd3 = TX_DMA_SDL(MTK_QDMA_PAGE_SIZE);

		eth->scratch_ring[i].txd4 = 0;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
		if (eth->soc->has_sram && ((sizeof(struct mtk_tx_dma)) > 16)) {
			eth->scratch_ring[i].txd5 = 0;
			eth->scratch_ring[i].txd6 = 0;
			eth->scratch_ring[i].txd7 = 0;
			eth->scratch_ring[i].txd8 = 0;
		}
#endif
	}

	mtk_w32(eth, eth->phy_scratch_ring, MTK_QDMA_FQ_HEAD);
	mtk_w32(eth, phy_ring_tail, MTK_QDMA_FQ_TAIL);
	mtk_w32(eth, (cnt << 16) | cnt, MTK_QDMA_FQ_CNT);
	mtk_w32(eth, MTK_QDMA_PAGE_SIZE << 16, MTK_QDMA_FQ_BLEN);

	return 0;
}

static inline void *mtk_qdma_phys_to_virt(struct mtk_tx_ring *ring, u32 desc)
{
	void *ret = ring->dma;

	return ret + (desc - ring->phys);
}

static inline struct mtk_tx_buf *mtk_desc_to_tx_buf(struct mtk_tx_ring *ring,
						    struct mtk_tx_dma *txd)
{
	int idx = txd - ring->dma;

	return &ring->buf[idx];
}

static struct mtk_tx_dma *qdma_to_pdma(struct mtk_tx_ring *ring,
				       struct mtk_tx_dma *dma)
{
	return ring->dma_pdma - ring->dma + dma;
}

static int txd_to_idx(struct mtk_tx_ring *ring, struct mtk_tx_dma *dma)
{
	return ((void *)dma - (void *)ring->dma) / sizeof(*dma);
}

static void mtk_tx_unmap(struct mtk_eth *eth, struct mtk_tx_buf *tx_buf,
			 bool napi)
{
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		if (tx_buf->flags & MTK_TX_FLAGS_SINGLE0) {
			dma_unmap_single(eth->dev,
					 dma_unmap_addr(tx_buf, dma_addr0),
					 dma_unmap_len(tx_buf, dma_len0),
					 DMA_TO_DEVICE);
		} else if (tx_buf->flags & MTK_TX_FLAGS_PAGE0) {
			dma_unmap_page(eth->dev,
				       dma_unmap_addr(tx_buf, dma_addr0),
				       dma_unmap_len(tx_buf, dma_len0),
				       DMA_TO_DEVICE);
		}
	} else {
		if (dma_unmap_len(tx_buf, dma_len0)) {
			dma_unmap_page(eth->dev,
				       dma_unmap_addr(tx_buf, dma_addr0),
				       dma_unmap_len(tx_buf, dma_len0),
				       DMA_TO_DEVICE);
		}

		if (dma_unmap_len(tx_buf, dma_len1)) {
			dma_unmap_page(eth->dev,
				       dma_unmap_addr(tx_buf, dma_addr1),
				       dma_unmap_len(tx_buf, dma_len1),
				       DMA_TO_DEVICE);
		}
	}

	tx_buf->flags = 0;
	if (tx_buf->skb &&
	    (tx_buf->skb != (struct sk_buff *)MTK_DMA_DUMMY_DESC)) {
		if (napi)
			napi_consume_skb(tx_buf->skb, napi);
		else
			dev_kfree_skb_any(tx_buf->skb);
	}
	tx_buf->skb = NULL;
}

static void setup_tx_buf(struct mtk_eth *eth, struct mtk_tx_buf *tx_buf,
			 struct mtk_tx_dma *txd, dma_addr_t mapped_addr,
			 size_t size, int idx)
{
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		dma_unmap_addr_set(tx_buf, dma_addr0, mapped_addr);
		dma_unmap_len_set(tx_buf, dma_len0, size);
	} else {
		if (idx & 1) {
			txd->txd3 = mapped_addr;
			txd->txd2 |= TX_DMA_PLEN1(size);
			dma_unmap_addr_set(tx_buf, dma_addr1, mapped_addr);
			dma_unmap_len_set(tx_buf, dma_len1, size);
		} else {
			tx_buf->skb = (struct sk_buff *)MTK_DMA_DUMMY_DESC;
			txd->txd1 = mapped_addr;
			txd->txd2 = TX_DMA_PLEN0(size);
			dma_unmap_addr_set(tx_buf, dma_addr0, mapped_addr);
			dma_unmap_len_set(tx_buf, dma_len0, size);
		}
	}
}

static int mtk_tx_map(struct sk_buff *skb, struct net_device *dev,
		      int tx_num, struct mtk_tx_ring *ring, bool gso)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	struct mtk_tx_dma *itxd, *txd;
	struct mtk_tx_dma *itxd_pdma, *txd_pdma;
	struct mtk_tx_buf *itx_buf, *tx_buf;
	dma_addr_t mapped_addr;
	unsigned int nr_frags;
	int i, n_desc = 1;
	u32 txd4 = 0, txd5 = 0, txd6 = 0;
	u32 fport;
	u32 qid = 0;
	int k = 0;

	itxd = ring->next_free;
	itxd_pdma = qdma_to_pdma(ring, itxd);
	if (itxd == ring->last_free)
		return -ENOMEM;

	itx_buf = mtk_desc_to_tx_buf(ring, itxd);
	memset(itx_buf, 0, sizeof(*itx_buf));

	mapped_addr = dma_map_single(eth->dev, skb->data,
				     skb_headlen(skb), DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(eth->dev, mapped_addr)))
		return -ENOMEM;

	WRITE_ONCE(itxd->txd1, mapped_addr);
	itx_buf->flags |= MTK_TX_FLAGS_SINGLE0;
	itx_buf->flags |= (!mac->id) ? MTK_TX_FLAGS_FPORT0 :
			  MTK_TX_FLAGS_FPORT1;
	setup_tx_buf(eth, itx_buf, itxd_pdma, mapped_addr, skb_headlen(skb),
		     k++);

	nr_frags = skb_shinfo(skb)->nr_frags;

        qid = mac->id;

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	if(!qid && mac->id)
		qid = MTK_QDMA_GMAC2_QID;
#endif

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		/* set the forward port */
		fport = (mac->id + 1) << TX_DMA_FPORT_SHIFT_V2;
		txd4 |= fport;

		if (gso)
			txd5 |= TX_DMA_TSO_V2;

		/* TX Checksum offload */
		if (skb->ip_summed == CHECKSUM_PARTIAL)
			txd5 |= TX_DMA_CHKSUM_V2;

		/* VLAN header offload */
		if (skb_vlan_tag_present(skb))
			txd6 |= TX_DMA_INS_VLAN_V2 | skb_vlan_tag_get(skb);

		txd4 = txd4 | TX_DMA_SWC_V2;
	} else {
		/* set the forward port */
		fport = (mac->id + 1) << TX_DMA_FPORT_SHIFT;
		txd4 |= fport;

                if (gso)
                        txd4 |= TX_DMA_TSO;

                /* TX Checksum offload */
                if (skb->ip_summed == CHECKSUM_PARTIAL)
                        txd4 |= TX_DMA_CHKSUM;

		/* VLAN header offload */
		if (skb_vlan_tag_present(skb))
			txd4 |= TX_DMA_INS_VLAN | skb_vlan_tag_get(skb);
	}
	/* TX SG offload */
	txd = itxd;
	txd_pdma = qdma_to_pdma(ring, txd);

#if defined(CONFIG_NET_MEDIATEK_HNAT) || defined(CONFIG_NET_MEDIATEK_HNAT_MODULE)
	if (HNAT_SKB_CB2(skb)->magic == 0x78681415) {
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
			txd4 &= ~(0xf << TX_DMA_FPORT_SHIFT_V2);
			txd4 |= 0x4 << TX_DMA_FPORT_SHIFT_V2;
		} else {
			txd4 &= ~(0x7 << TX_DMA_FPORT_SHIFT);
			txd4 |= 0x4 << TX_DMA_FPORT_SHIFT;
		}
	}

	trace_printk("[%s] nr_frags=%x HNAT_SKB_CB2(skb)->magic=%x txd4=%x<-----\n",
		     __func__, nr_frags, HNAT_SKB_CB2(skb)->magic, txd4);
#endif

	for (i = 0; i < nr_frags; i++) {
		skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		unsigned int offset = 0;
		int frag_size = skb_frag_size(frag);

		while (frag_size) {
			bool last_frag = false;
			unsigned int frag_map_size;
			bool new_desc = true;

			if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA) ||
			    (i & 0x1)) {
				txd = mtk_qdma_phys_to_virt(ring, txd->txd2);
				txd_pdma = qdma_to_pdma(ring, txd);
				if (txd == ring->last_free)
					goto err_dma;

				n_desc++;
			} else {
				new_desc = false;
			}


			frag_map_size = min(frag_size, MTK_TX_DMA_BUF_LEN);
			mapped_addr = skb_frag_dma_map(eth->dev, frag, offset,
						       frag_map_size,
						       DMA_TO_DEVICE);
			if (unlikely(dma_mapping_error(eth->dev, mapped_addr)))
				goto err_dma;

			if (i == nr_frags - 1 &&
			    (frag_size - frag_map_size) == 0)
				last_frag = true;

			WRITE_ONCE(txd->txd1, mapped_addr);

			if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
				WRITE_ONCE(txd->txd3, (TX_DMA_PLEN0(frag_map_size) |
					   last_frag * TX_DMA_LS0));
				WRITE_ONCE(txd->txd4, fport | TX_DMA_SWC_V2 |
						      QID_BITS_V2(qid));
			} else {
				WRITE_ONCE(txd->txd3,
					   (TX_DMA_SWC | QID_LOW_BITS(qid) |
					    TX_DMA_PLEN0(frag_map_size) |
					    last_frag * TX_DMA_LS0));
				WRITE_ONCE(txd->txd4,
					   fport | QID_HIGH_BITS(qid));
			}

			tx_buf = mtk_desc_to_tx_buf(ring, txd);
			if (new_desc)
				memset(tx_buf, 0, sizeof(*tx_buf));
			tx_buf->skb = (struct sk_buff *)MTK_DMA_DUMMY_DESC;
			tx_buf->flags |= MTK_TX_FLAGS_PAGE0;
			tx_buf->flags |= (!mac->id) ? MTK_TX_FLAGS_FPORT0 :
					 MTK_TX_FLAGS_FPORT1;

			setup_tx_buf(eth, tx_buf, txd_pdma, mapped_addr,
				     frag_map_size, k++);

			frag_size -= frag_map_size;
			offset += frag_map_size;
		}
	}

	/* store skb to cleanup */
	itx_buf->skb = skb;

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	WRITE_ONCE(itxd->txd5, txd5);
	WRITE_ONCE(itxd->txd6, txd6);
	WRITE_ONCE(itxd->txd7, 0);
	WRITE_ONCE(itxd->txd8, 0);
#endif

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		WRITE_ONCE(itxd->txd4, txd4 | QID_BITS_V2(qid));
		WRITE_ONCE(itxd->txd3, (TX_DMA_PLEN0(skb_headlen(skb)) |
				(!nr_frags * TX_DMA_LS0)));
	} else {
		WRITE_ONCE(itxd->txd4, txd4 | QID_HIGH_BITS(qid));
		WRITE_ONCE(itxd->txd3,
			   TX_DMA_SWC | TX_DMA_PLEN0(skb_headlen(skb)) |
			   (!nr_frags * TX_DMA_LS0) | QID_LOW_BITS(qid));
	}

	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		if (k & 0x1)
			txd_pdma->txd2 |= TX_DMA_LS0;
		else
			txd_pdma->txd2 |= TX_DMA_LS1;
	}

	ring->next_free = mtk_qdma_phys_to_virt(ring, txd->txd2);
	atomic_sub(n_desc, &ring->free_count);

	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		if (netif_xmit_stopped(netdev_get_tx_queue(dev, 0)) ||
		    !netdev_xmit_more())
			mtk_w32(eth, txd->txd2, MTK_QTX_CTX_PTR);
	} else {
		int next_idx = NEXT_DESP_IDX(txd_to_idx(ring, txd),
					     ring->dma_size);
		mtk_w32(eth, next_idx, MT7628_TX_CTX_IDX0);
	}

	return 0;

err_dma:
	do {
		tx_buf = mtk_desc_to_tx_buf(ring, itxd);

		/* unmap dma */
		mtk_tx_unmap(eth, tx_buf, false);

		itxd->txd3 = TX_DMA_LS0 | TX_DMA_OWNER_CPU;
		if (!MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA))
			itxd_pdma->txd2 = TX_DMA_DESP2_DEF;

		itxd = mtk_qdma_phys_to_virt(ring, itxd->txd2);
		itxd_pdma = qdma_to_pdma(ring, itxd);
	} while (itxd != txd);

	return -ENOMEM;
}

static inline int mtk_cal_txd_req(struct sk_buff *skb)
{
	int i, nfrags;
	skb_frag_t *frag;

	nfrags = 1;
	if (skb_is_gso(skb)) {
		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			frag = &skb_shinfo(skb)->frags[i];
			nfrags += DIV_ROUND_UP(skb_frag_size(frag),
						MTK_TX_DMA_BUF_LEN);
		}
	} else {
		nfrags += skb_shinfo(skb)->nr_frags;
	}

	return nfrags;
}

static int mtk_queue_stopped(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		if (netif_queue_stopped(eth->netdev[i]))
			return 1;
	}

	return 0;
}

static void mtk_wake_queue(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		netif_wake_queue(eth->netdev[i]);
	}
}

static int mtk_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	struct mtk_tx_ring *ring = &eth->tx_ring;
	struct net_device_stats *stats = &dev->stats;
	bool gso = false;
	int tx_num;

	/* normally we can rely on the stack not calling this more than once,
	 * however we have 2 queues running on the same ring so we need to lock
	 * the ring access
	 */
	spin_lock(&eth->page_lock);

	if (unlikely(test_bit(MTK_RESETTING, &eth->state)))
		goto drop;

	tx_num = mtk_cal_txd_req(skb);
	if (unlikely(atomic_read(&ring->free_count) <= tx_num)) {
		netif_stop_queue(dev);
		netif_err(eth, tx_queued, dev,
			  "Tx Ring full when queue awake!\n");
		spin_unlock(&eth->page_lock);
		return NETDEV_TX_BUSY;
	}

	/* TSO: fill MSS info in tcp checksum field */
	if (skb_is_gso(skb)) {
		if (skb_cow_head(skb, 0)) {
			netif_warn(eth, tx_err, dev,
				   "GSO expand head fail.\n");
			goto drop;
		}

		if (skb_shinfo(skb)->gso_type &
				(SKB_GSO_TCPV4 | SKB_GSO_TCPV6)) {
			gso = true;
			tcp_hdr(skb)->check = htons(skb_shinfo(skb)->gso_size);
		}
	}

	if (mtk_tx_map(skb, dev, tx_num, ring, gso) < 0)
		goto drop;

	if (unlikely(atomic_read(&ring->free_count) <= ring->thresh))
		netif_stop_queue(dev);

	spin_unlock(&eth->page_lock);

	return NETDEV_TX_OK;

drop:
	spin_unlock(&eth->page_lock);
	stats->tx_dropped++;
	dev_kfree_skb_any(skb);
	return NETDEV_TX_OK;
}

static struct mtk_rx_ring *mtk_get_rx_ring(struct mtk_eth *eth)
{
	int i;
	struct mtk_rx_ring *ring;
	int idx;

	for (i = 0; i < MTK_MAX_RX_RING_NUM; i++) {
		if (!IS_NORMAL_RING(i) && !IS_HW_LRO_RING(i))
			continue;

		ring = &eth->rx_ring[i];
		idx = NEXT_DESP_IDX(ring->calc_idx, ring->dma_size);
		if (ring->dma[idx].rxd2 & RX_DMA_DONE) {
			ring->calc_idx_update = true;
			return ring;
		}
	}

	return NULL;
}

static void mtk_update_rx_cpu_idx(struct mtk_eth *eth, struct mtk_rx_ring *ring)
{
	int i;

	if (!eth->hwlro)
		mtk_w32(eth, ring->calc_idx, ring->crx_idx_reg);
	else {
		for (i = 0; i < MTK_MAX_RX_RING_NUM; i++) {
			ring = &eth->rx_ring[i];
			if (ring->calc_idx_update) {
				ring->calc_idx_update = false;
				mtk_w32(eth, ring->calc_idx, ring->crx_idx_reg);
			}
		}
	}
}

static int mtk_poll_rx(struct napi_struct *napi, int budget,
		       struct mtk_eth *eth)
{
	struct mtk_napi *rx_napi = container_of(napi, struct mtk_napi, napi);
	struct mtk_rx_ring *ring = rx_napi->rx_ring;
	int idx;
	struct sk_buff *skb;
	u8 *data, *new_data;
	struct mtk_rx_dma *rxd, trxd;
	int done = 0;

	if (unlikely(!ring))
		goto rx_done;

	while (done < budget) {
		struct net_device *netdev;
		unsigned int pktlen;
		dma_addr_t dma_addr;
		int mac;

		if (eth->hwlro)
			ring = mtk_get_rx_ring(eth);

		if (unlikely(!ring))
			goto rx_done;

		idx = NEXT_DESP_IDX(ring->calc_idx, ring->dma_size);
		rxd = &ring->dma[idx];
		data = ring->data[idx];

		if (!mtk_rx_get_desc(&trxd, rxd))
			break;

		/* find out which mac the packet come from. values start at 1 */
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
			mac = 0;
		} else {
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
			if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
				mac = RX_DMA_GET_SPORT(trxd.rxd5) - 1;
			else
#endif
				mac = (trxd.rxd4 & RX_DMA_SPECIAL_TAG) ?
				      0 : RX_DMA_GET_SPORT(trxd.rxd4) - 1;
		}

		if (mac == 4)
			mac = 0;

		if (unlikely(mac < 0 || mac >= MTK_MAC_COUNT ||
			     !eth->netdev[mac]))
			goto release_desc;

		netdev = eth->netdev[mac];

		if (unlikely(test_bit(MTK_RESETTING, &eth->state)))
			goto release_desc;

		/* alloc new buffer */
		new_data = napi_alloc_frag(ring->frag_size);
		if (unlikely(!new_data)) {
			netdev->stats.rx_dropped++;
			goto release_desc;
		}
		dma_addr = dma_map_single(eth->dev,
					  new_data + NET_SKB_PAD +
					  eth->ip_align,
					  ring->buf_size,
					  DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(eth->dev, dma_addr))) {
			skb_free_frag(new_data);
			netdev->stats.rx_dropped++;
			goto release_desc;
		}

		dma_unmap_single(eth->dev, trxd.rxd1,
				 ring->buf_size, DMA_FROM_DEVICE);

		/* receive data */
		skb = build_skb(data, ring->frag_size);
		if (unlikely(!skb)) {
			skb_free_frag(data);
			netdev->stats.rx_dropped++;
			goto skip_rx;
		}
		skb_reserve(skb, NET_SKB_PAD + NET_IP_ALIGN);

		pktlen = RX_DMA_GET_PLEN0(trxd.rxd2);
		skb->dev = netdev;
		skb_put(skb, pktlen);

		if ((!MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2) &&
				  (trxd.rxd4 & eth->rx_dma_l4_valid)) ||
		    (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2) &&
				  (trxd.rxd3 & eth->rx_dma_l4_valid)))
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		else
			skb_checksum_none_assert(skb);
		skb->protocol = eth_type_trans(skb, netdev);

		if (netdev->features & NETIF_F_HW_VLAN_CTAG_RX) {
			if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
				if (trxd.rxd3 & RX_DMA_VTAG_V2)
					__vlan_hwaccel_put_tag(skb,
					htons(RX_DMA_VPID_V2(trxd.rxd4)),
					RX_DMA_VID_V2(trxd.rxd4));
			} else {
				if (trxd.rxd2 & RX_DMA_VTAG)
					__vlan_hwaccel_put_tag(skb,
					htons(RX_DMA_VPID(trxd.rxd3)),
					RX_DMA_VID(trxd.rxd3));
			}

			/* If netdev is attached to dsa switch, the special
			 * tag inserted in VLAN field by switch hardware can
			 * be offload by RX HW VLAN offload. Clears the VLAN
			 * information from @skb to avoid unexpected 8021d
			 * handler before packet enter dsa framework.
			 */
			if (netdev_uses_dsa(netdev))
				__vlan_hwaccel_clear_tag(skb);
		}

#if defined(CONFIG_NET_MEDIATEK_HNAT) || defined(CONFIG_NET_MEDIATEK_HNAT_MODULE)
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
			*(u32 *)(skb->head) = trxd.rxd5;
		else
#endif
			*(u32 *)(skb->head) = trxd.rxd4;

		skb_hnat_alg(skb) = 0;
		skb_hnat_filled(skb) = 0;
		skb_hnat_magic_tag(skb) = HNAT_MAGIC_TAG;

		if (skb_hnat_reason(skb) == HIT_BIND_FORCE_TO_CPU) {
			trace_printk("[%s] reason=0x%x(force to CPU) from WAN to Ext\n",
				     __func__, skb_hnat_reason(skb));
			skb->pkt_type = PACKET_HOST;
		}

		trace_printk("[%s] rxd:(entry=%x,sport=%x,reason=%x,alg=%x\n",
			     __func__, skb_hnat_entry(skb), skb_hnat_sport(skb),
			     skb_hnat_reason(skb), skb_hnat_alg(skb));
#endif
		if (mtk_hwlro_stats_ebl &&
		    IS_HW_LRO_RING(ring->ring_no) && eth->hwlro) {
			hw_lro_stats_update(ring->ring_no, &trxd);
			hw_lro_flush_stats_update(ring->ring_no, &trxd);
		}

		skb_record_rx_queue(skb, 0);
		napi_gro_receive(napi, skb);

skip_rx:
		ring->data[idx] = new_data;
		rxd->rxd1 = (unsigned int)dma_addr;

release_desc:
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628))
			rxd->rxd2 = RX_DMA_LSO;
		else
			rxd->rxd2 = RX_DMA_PLEN0(ring->buf_size);

		ring->calc_idx = idx;

		done++;
	}

rx_done:
	if (done) {
		/* make sure that all changes to the dma ring are flushed before
		 * we continue
		 */
		wmb();
		mtk_update_rx_cpu_idx(eth, ring);
	}

	return done;
}

static void mtk_poll_tx_qdma(struct mtk_eth *eth, int budget,
			    unsigned int *done, unsigned int *bytes)
{
	struct mtk_tx_ring *ring = &eth->tx_ring;
	struct mtk_tx_dma *desc;
	struct sk_buff *skb;
	struct mtk_tx_buf *tx_buf;
	u32 cpu, dma;

	cpu = ring->last_free_ptr;
	dma = mtk_r32(eth, MTK_QTX_DRX_PTR);

	desc = mtk_qdma_phys_to_virt(ring, cpu);

	while ((cpu != dma) && budget) {
		u32 next_cpu = desc->txd2;
		int mac = 0;

		if ((desc->txd3 & TX_DMA_OWNER_CPU) == 0)
			break;

		desc = mtk_qdma_phys_to_virt(ring, desc->txd2);

		tx_buf = mtk_desc_to_tx_buf(ring, desc);
		if (tx_buf->flags & MTK_TX_FLAGS_FPORT1)
			mac = 1;

		skb = tx_buf->skb;
		if (!skb)
			break;

		if (skb != (struct sk_buff *)MTK_DMA_DUMMY_DESC) {
			bytes[mac] += skb->len;
			done[mac]++;
			budget--;
		}
		mtk_tx_unmap(eth, tx_buf, true);

		ring->last_free = desc;
		atomic_inc(&ring->free_count);

		cpu = next_cpu;
	}

	ring->last_free_ptr = cpu;
	mtk_w32(eth, cpu, MTK_QTX_CRX_PTR);
}

static void mtk_poll_tx_pdma(struct mtk_eth *eth, int budget,
			    unsigned int *done, unsigned int *bytes)
{
	struct mtk_tx_ring *ring = &eth->tx_ring;
	struct mtk_tx_dma *desc;
	struct sk_buff *skb;
	struct mtk_tx_buf *tx_buf;
	u32 cpu, dma;

	cpu = ring->cpu_idx;
	dma = mtk_r32(eth, MT7628_TX_DTX_IDX0);

	while ((cpu != dma) && budget) {
		tx_buf = &ring->buf[cpu];
		skb = tx_buf->skb;
		if (!skb)
			break;

		if (skb != (struct sk_buff *)MTK_DMA_DUMMY_DESC) {
			bytes[0] += skb->len;
			done[0]++;
			budget--;
		}

		mtk_tx_unmap(eth, tx_buf, true);

		desc = &ring->dma[cpu];
		ring->last_free = desc;
		atomic_inc(&ring->free_count);

		cpu = NEXT_DESP_IDX(cpu, ring->dma_size);
	}

	ring->cpu_idx = cpu;
}

static int mtk_poll_tx(struct mtk_eth *eth, int budget)
{
	struct mtk_tx_ring *ring = &eth->tx_ring;
	unsigned int done[MTK_MAX_DEVS];
	unsigned int bytes[MTK_MAX_DEVS];
	int total = 0, i;

	memset(done, 0, sizeof(done));
	memset(bytes, 0, sizeof(bytes));

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA))
		mtk_poll_tx_qdma(eth, budget, done, bytes);
	else
		mtk_poll_tx_pdma(eth, budget, done, bytes);

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i] || !done[i])
			continue;
		total += done[i];
	}

	if (mtk_queue_stopped(eth) &&
	    (atomic_read(&ring->free_count) > ring->thresh))
		mtk_wake_queue(eth);

	return total;
}

static void mtk_handle_status_irq(struct mtk_eth *eth)
{
	u32 status2 = mtk_r32(eth, MTK_FE_INT_STATUS);

	if (unlikely(status2 & (MTK_GDM1_AF | MTK_GDM2_AF))) {
		mtk_stats_update(eth);
		mtk_w32(eth, (MTK_GDM1_AF | MTK_GDM2_AF),
			MTK_FE_INT_STATUS);
	}
}

static int mtk_napi_tx(struct napi_struct *napi, int budget)
{
	struct mtk_eth *eth = container_of(napi, struct mtk_eth, tx_napi);
	u32 status, mask;
	int tx_done = 0;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA))
		mtk_handle_status_irq(eth);
	mtk_w32(eth, MTK_TX_DONE_INT, eth->tx_int_status_reg);
	tx_done = mtk_poll_tx(eth, budget);

	if (unlikely(netif_msg_intr(eth))) {
		status = mtk_r32(eth, eth->tx_int_status_reg);
		mask = mtk_r32(eth, eth->tx_int_mask_reg);
		dev_info(eth->dev,
			 "done tx %d, intr 0x%08x/0x%x\n",
			 tx_done, status, mask);
	}

	if (tx_done == budget)
		return budget;

	status = mtk_r32(eth, eth->tx_int_status_reg);
	if (status & MTK_TX_DONE_INT)
		return budget;

	if (napi_complete(napi))
		mtk_tx_irq_enable(eth, MTK_TX_DONE_INT);

	return tx_done;
}

static int mtk_napi_rx(struct napi_struct *napi, int budget)
{
	struct mtk_napi *rx_napi = container_of(napi, struct mtk_napi, napi);
	struct mtk_eth *eth = rx_napi->eth;
	struct mtk_rx_ring *ring = rx_napi->rx_ring;
	u32 status, mask;
	int rx_done = 0;
	int remain_budget = budget;

	mtk_handle_status_irq(eth);

poll_again:
	mtk_w32(eth, MTK_RX_DONE_INT(ring->ring_no), MTK_PDMA_INT_STATUS);
	rx_done = mtk_poll_rx(napi, remain_budget, eth);

	if (unlikely(netif_msg_intr(eth))) {
		status = mtk_r32(eth, MTK_PDMA_INT_STATUS);
		mask = mtk_r32(eth, MTK_PDMA_INT_MASK);
		dev_info(eth->dev,
			 "done rx %d, intr 0x%08x/0x%x\n",
			 rx_done, status, mask);
	}
	if (rx_done == remain_budget)
		return budget;

	status = mtk_r32(eth, MTK_PDMA_INT_STATUS);
	if (status & MTK_RX_DONE_INT(ring->ring_no)) {
		remain_budget -= rx_done;
		goto poll_again;
	}

	if (napi_complete(napi))
		mtk_rx_irq_enable(eth, MTK_RX_DONE_INT(ring->ring_no));

	return rx_done + budget - remain_budget;
}

static int mtk_tx_alloc(struct mtk_eth *eth)
{
	struct mtk_tx_ring *ring = &eth->tx_ring;
	int i, sz = sizeof(*ring->dma);

	ring->buf = kcalloc(MTK_DMA_SIZE, sizeof(*ring->buf),
			       GFP_KERNEL);
	if (!ring->buf)
		goto no_tx_mem;

	if (!eth->soc->has_sram)
		ring->dma = dma_alloc_coherent(eth->dev, MTK_DMA_SIZE * sz,
					       &ring->phys, GFP_ATOMIC);
	else {
		ring->dma =  eth->scratch_ring + MTK_DMA_SIZE;
		ring->phys = eth->phy_scratch_ring + MTK_DMA_SIZE * sz;
	}

	if (!ring->dma)
		goto no_tx_mem;

	for (i = 0; i < MTK_DMA_SIZE; i++) {
		int next = (i + 1) % MTK_DMA_SIZE;
		u32 next_ptr = ring->phys + next * sz;

		ring->dma[i].txd2 = next_ptr;
		ring->dma[i].txd3 = TX_DMA_LS0 | TX_DMA_OWNER_CPU;
		ring->dma[i].txd4 = 0;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
                if (eth->soc->has_sram && ( sz > 16)) {
                        ring->dma[i].txd5 = 0;
                        ring->dma[i].txd6 = 0;
                        ring->dma[i].txd7 = 0;
                        ring->dma[i].txd8 = 0;
                }
#endif
	}

	/* On MT7688 (PDMA only) this driver uses the ring->dma structs
	 * only as the framework. The real HW descriptors are the PDMA
	 * descriptors in ring->dma_pdma.
	 */
	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		ring->dma_pdma = dma_alloc_coherent(eth->dev, MTK_DMA_SIZE * sz,
						    &ring->phys_pdma,
						    GFP_ATOMIC);
		if (!ring->dma_pdma)
			goto no_tx_mem;

		for (i = 0; i < MTK_DMA_SIZE; i++) {
			ring->dma_pdma[i].txd2 = TX_DMA_DESP2_DEF;
			ring->dma_pdma[i].txd4 = 0;
		}
	}

	ring->dma_size = MTK_DMA_SIZE;
	atomic_set(&ring->free_count, MTK_DMA_SIZE - 2);
	ring->next_free = &ring->dma[0];
	ring->last_free = &ring->dma[MTK_DMA_SIZE - 1];
	ring->last_free_ptr = (u32)(ring->phys + ((MTK_DMA_SIZE - 1) * sz));
	ring->thresh = MAX_SKB_FRAGS;

	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		mtk_w32(eth, ring->phys, MTK_QTX_CTX_PTR);
		mtk_w32(eth, ring->phys, MTK_QTX_DTX_PTR);
		mtk_w32(eth,
			ring->phys + ((MTK_DMA_SIZE - 1) * sz),
			MTK_QTX_CRX_PTR);
		mtk_w32(eth, ring->last_free_ptr, MTK_QTX_DRX_PTR);
		mtk_w32(eth, (QDMA_RES_THRES << 8) | QDMA_RES_THRES,
			MTK_QTX_CFG(0));
		mtk_w32(eth, BIT(31), MTK_QTX_SCH(0));
	} else {
		mtk_w32(eth, ring->phys_pdma, MT7628_TX_BASE_PTR0);
		mtk_w32(eth, MTK_DMA_SIZE, MT7628_TX_MAX_CNT0);
		mtk_w32(eth, 0, MT7628_TX_CTX_IDX0);
		mtk_w32(eth, MT7628_PST_DTX_IDX0, MTK_PDMA_RST_IDX);
	}

	return 0;

no_tx_mem:
	return -ENOMEM;
}

static void mtk_tx_clean(struct mtk_eth *eth)
{
	struct mtk_tx_ring *ring = &eth->tx_ring;
	int i;

	if (ring->buf) {
		for (i = 0; i < MTK_DMA_SIZE; i++)
			mtk_tx_unmap(eth, &ring->buf[i], false);
		kfree(ring->buf);
		ring->buf = NULL;
	}

	if (!eth->soc->has_sram && ring->dma) {
		dma_free_coherent(eth->dev,
				  MTK_DMA_SIZE * sizeof(*ring->dma),
				  ring->dma,
				  ring->phys);
		ring->dma = NULL;
	}

	if (ring->dma_pdma) {
		dma_free_coherent(eth->dev,
				  MTK_DMA_SIZE * sizeof(*ring->dma_pdma),
				  ring->dma_pdma,
				  ring->phys_pdma);
		ring->dma_pdma = NULL;
	}
}

static int mtk_rx_alloc(struct mtk_eth *eth, int ring_no, int rx_flag)
{
	struct mtk_rx_ring *ring;
	int rx_data_len, rx_dma_size;
	int i;

	if (rx_flag == MTK_RX_FLAGS_QDMA) {
		if (ring_no)
			return -EINVAL;
		ring = &eth->rx_ring_qdma;
	} else {
		ring = &eth->rx_ring[ring_no];
	}

	if (rx_flag == MTK_RX_FLAGS_HWLRO) {
		rx_data_len = MTK_MAX_LRO_RX_LENGTH;
		rx_dma_size = MTK_HW_LRO_DMA_SIZE;
	} else {
		rx_data_len = ETH_DATA_LEN;
		rx_dma_size = MTK_DMA_SIZE;
	}

	ring->frag_size = mtk_max_frag_size(rx_data_len);
	ring->buf_size = mtk_max_buf_size(ring->frag_size);
	ring->data = kcalloc(rx_dma_size, sizeof(*ring->data),
			     GFP_KERNEL);
	if (!ring->data)
		return -ENOMEM;

	for (i = 0; i < rx_dma_size; i++) {
		ring->data[i] = netdev_alloc_frag(ring->frag_size);
		if (!ring->data[i])
			return -ENOMEM;
	}

	if ((!eth->soc->has_sram) || (eth->soc->has_sram
				&& (rx_flag != MTK_RX_FLAGS_NORMAL)))
		ring->dma = dma_alloc_coherent(eth->dev,
					       rx_dma_size * sizeof(*ring->dma),
					       &ring->phys, GFP_ATOMIC);
	else {
		struct mtk_tx_ring *tx_ring = &eth->tx_ring;
		ring->dma = (struct mtk_rx_dma *)(tx_ring->dma +
			     MTK_DMA_SIZE * (ring_no + 1));
		ring->phys = tx_ring->phys + MTK_DMA_SIZE *
			     sizeof(*tx_ring->dma) * (ring_no + 1);
	}

	if (!ring->dma)
		return -ENOMEM;

	for (i = 0; i < rx_dma_size; i++) {
		dma_addr_t dma_addr = dma_map_single(eth->dev,
				ring->data[i] + NET_SKB_PAD + eth->ip_align,
				ring->buf_size,
				DMA_FROM_DEVICE);
		if (unlikely(dma_mapping_error(eth->dev, dma_addr)))
			return -ENOMEM;
		ring->dma[i].rxd1 = (unsigned int)dma_addr;

		if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628))
			ring->dma[i].rxd2 = RX_DMA_LSO;
		else
			ring->dma[i].rxd2 = RX_DMA_PLEN0(ring->buf_size);

		ring->dma[i].rxd3 = 0;
		ring->dma[i].rxd4 = 0;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
		if (eth->soc->has_sram && ((sizeof(struct mtk_rx_dma)) > 16)) {
			ring->dma[i].rxd5 = 0;
			ring->dma[i].rxd6 = 0;
			ring->dma[i].rxd7 = 0;
			ring->dma[i].rxd8 = 0;
		}
#endif
	}
	ring->dma_size = rx_dma_size;
	ring->calc_idx_update = false;
	ring->calc_idx = rx_dma_size - 1;
	ring->crx_idx_reg = (rx_flag == MTK_RX_FLAGS_QDMA) ?
			     MTK_QRX_CRX_IDX_CFG(ring_no) :
			     MTK_PRX_CRX_IDX_CFG(ring_no);
	ring->ring_no = ring_no;
	/* make sure that all changes to the dma ring are flushed before we
	 * continue
	 */
	wmb();

	if (rx_flag == MTK_RX_FLAGS_QDMA) {
		mtk_w32(eth, ring->phys, MTK_QRX_BASE_PTR_CFG(ring_no));
		mtk_w32(eth, rx_dma_size, MTK_QRX_MAX_CNT_CFG(ring_no));
		mtk_w32(eth, ring->calc_idx, ring->crx_idx_reg);
		mtk_w32(eth, MTK_PST_DRX_IDX_CFG(ring_no), MTK_QDMA_RST_IDX);
	} else {
		mtk_w32(eth, ring->phys, MTK_PRX_BASE_PTR_CFG(ring_no));
		mtk_w32(eth, rx_dma_size, MTK_PRX_MAX_CNT_CFG(ring_no));
		mtk_w32(eth, ring->calc_idx, ring->crx_idx_reg);
		mtk_w32(eth, MTK_PST_DRX_IDX_CFG(ring_no), MTK_PDMA_RST_IDX);
	}

	return 0;
}

static void mtk_rx_clean(struct mtk_eth *eth, struct mtk_rx_ring *ring, int in_sram)
{
	int i;

	if (ring->data && ring->dma) {
		for (i = 0; i < ring->dma_size; i++) {
			if (!ring->data[i])
				continue;
			if (!ring->dma[i].rxd1)
				continue;
			dma_unmap_single(eth->dev,
					 ring->dma[i].rxd1,
					 ring->buf_size,
					 DMA_FROM_DEVICE);
			skb_free_frag(ring->data[i]);
		}
		kfree(ring->data);
		ring->data = NULL;
	}

	if(in_sram)
		return;

	if (ring->dma) {
		dma_free_coherent(eth->dev,
				  ring->dma_size * sizeof(*ring->dma),
				  ring->dma,
				  ring->phys);
		ring->dma = NULL;
	}
}

static int mtk_hwlro_rx_init(struct mtk_eth *eth)
{
	int i;
	u32 val;
	u32 ring_ctrl_dw1 = 0, ring_ctrl_dw2 = 0, ring_ctrl_dw3 = 0;
	u32 lro_ctrl_dw0 = 0, lro_ctrl_dw3 = 0;

	/* set LRO rings to auto-learn modes */
	ring_ctrl_dw2 |= MTK_RING_AUTO_LERAN_MODE;

	/* validate LRO ring */
	ring_ctrl_dw2 |= MTK_RING_VLD;

	/* set AGE timer (unit: 20us) */
	ring_ctrl_dw2 |= MTK_RING_AGE_TIME_H;
	ring_ctrl_dw1 |= MTK_RING_AGE_TIME_L;

	/* set max AGG timer (unit: 20us) */
	ring_ctrl_dw2 |= MTK_RING_MAX_AGG_TIME;

	/* set max LRO AGG count */
	ring_ctrl_dw2 |= MTK_RING_MAX_AGG_CNT_L;
	ring_ctrl_dw3 |= MTK_RING_MAX_AGG_CNT_H;

	for (i = 1; i <= MTK_HW_LRO_RING_NUM; i++) {
		mtk_w32(eth, ring_ctrl_dw1, MTK_LRO_CTRL_DW1_CFG(i));
		mtk_w32(eth, ring_ctrl_dw2, MTK_LRO_CTRL_DW2_CFG(i));
		mtk_w32(eth, ring_ctrl_dw3, MTK_LRO_CTRL_DW3_CFG(i));
	}

	/* IPv4 checksum update enable */
	lro_ctrl_dw0 |= MTK_L3_CKS_UPD_EN;

	/* switch priority comparison to packet count mode */
	lro_ctrl_dw0 |= MTK_LRO_ALT_PKT_CNT_MODE;

	/* bandwidth threshold setting */
	mtk_w32(eth, MTK_HW_LRO_BW_THRE, MTK_PDMA_LRO_CTRL_DW2);

	/* auto-learn score delta setting */
	mtk_w32(eth, MTK_HW_LRO_REPLACE_DELTA, MTK_LRO_ALT_SCORE_DELTA);

	/* set refresh timer for altering flows to 1 sec. (unit: 20us) */
	mtk_w32(eth, (MTK_HW_LRO_TIMER_UNIT << 16) | MTK_HW_LRO_REFRESH_TIME,
		MTK_PDMA_LRO_ALT_REFRESH_TIMER);

	/* the minimal remaining room of SDL0 in RXD for lro aggregation */
	lro_ctrl_dw3 |= MTK_LRO_MIN_RXD_SDL;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		val = mtk_r32(eth, MTK_PDMA_RX_CFG);
		mtk_w32(eth, val | (MTK_PDMA_LRO_SDL << MTK_RX_CFG_SDL_OFFSET),
			MTK_PDMA_RX_CFG);

		lro_ctrl_dw0 |= MTK_PDMA_LRO_SDL << MTK_CTRL_DW0_SDL_OFFSET;
	} else {
		/* set HW LRO mode & the max aggregation count for rx packets */
		lro_ctrl_dw3 |= MTK_ADMA_MODE | (MTK_HW_LRO_MAX_AGG_CNT & 0xff);
	}

	/* enable HW LRO */
	lro_ctrl_dw0 |= MTK_LRO_EN;

	/* enable cpu reason black list */
	lro_ctrl_dw0 |= MTK_LRO_CRSN_BNW;

	mtk_w32(eth, lro_ctrl_dw3, MTK_PDMA_LRO_CTRL_DW3);
	mtk_w32(eth, lro_ctrl_dw0, MTK_PDMA_LRO_CTRL_DW0);

	/* no use PPE cpu reason */
	mtk_w32(eth, 0xffffffff, MTK_PDMA_LRO_CTRL_DW1);

	return 0;
}

static void mtk_hwlro_rx_uninit(struct mtk_eth *eth)
{
	int i;
	u32 val;

	/* relinquish lro rings, flush aggregated packets */
	mtk_w32(eth, MTK_LRO_RING_RELINGUISH_REQ, MTK_PDMA_LRO_CTRL_DW0);

	/* wait for relinquishments done */
	for (i = 0; i < 10; i++) {
		val = mtk_r32(eth, MTK_PDMA_LRO_CTRL_DW0);
		if (val & MTK_LRO_RING_RELINGUISH_DONE) {
			mdelay(20);
			continue;
		}
		break;
	}

	/* invalidate lro rings */
	for (i = 1; i <= MTK_HW_LRO_RING_NUM; i++)
		mtk_w32(eth, 0, MTK_LRO_CTRL_DW2_CFG(i));

	/* disable HW LRO */
	mtk_w32(eth, 0, MTK_PDMA_LRO_CTRL_DW0);
}

static void mtk_hwlro_val_ipaddr(struct mtk_eth *eth, int idx, __be32 ip)
{
	u32 reg_val;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
		idx += 1;

	reg_val = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(idx));

	/* invalidate the IP setting */
	mtk_w32(eth, (reg_val & ~MTK_RING_MYIP_VLD), MTK_LRO_CTRL_DW2_CFG(idx));

	mtk_w32(eth, ip, MTK_LRO_DIP_DW0_CFG(idx));

	/* validate the IP setting */
	mtk_w32(eth, (reg_val | MTK_RING_MYIP_VLD), MTK_LRO_CTRL_DW2_CFG(idx));
}

static void mtk_hwlro_inval_ipaddr(struct mtk_eth *eth, int idx)
{
	u32 reg_val;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
		idx += 1;

	reg_val = mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(idx));

	/* invalidate the IP setting */
	mtk_w32(eth, (reg_val & ~MTK_RING_MYIP_VLD), MTK_LRO_CTRL_DW2_CFG(idx));

	mtk_w32(eth, 0, MTK_LRO_DIP_DW0_CFG(idx));
}

static int mtk_hwlro_get_ip_cnt(struct mtk_mac *mac)
{
	int cnt = 0;
	int i;

	for (i = 0; i < MTK_MAX_LRO_IP_CNT; i++) {
		if (mac->hwlro_ip[i])
			cnt++;
	}

	return cnt;
}

static int mtk_hwlro_add_ipaddr(struct net_device *dev,
				struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int hwlro_idx;

	if ((fsp->flow_type != TCP_V4_FLOW) ||
	    (!fsp->h_u.tcp_ip4_spec.ip4dst) ||
	    (fsp->location > 1))
		return -EINVAL;

	mac->hwlro_ip[fsp->location] = htonl(fsp->h_u.tcp_ip4_spec.ip4dst);
	hwlro_idx = (mac->id * MTK_MAX_LRO_IP_CNT) + fsp->location;

	mac->hwlro_ip_cnt = mtk_hwlro_get_ip_cnt(mac);

	mtk_hwlro_val_ipaddr(eth, hwlro_idx, mac->hwlro_ip[fsp->location]);

	return 0;
}

static int mtk_hwlro_del_ipaddr(struct net_device *dev,
				struct ethtool_rxnfc *cmd)
{
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int hwlro_idx;

	if (fsp->location > 1)
		return -EINVAL;

	mac->hwlro_ip[fsp->location] = 0;
	hwlro_idx = (mac->id * MTK_MAX_LRO_IP_CNT) + fsp->location;

	mac->hwlro_ip_cnt = mtk_hwlro_get_ip_cnt(mac);

	mtk_hwlro_inval_ipaddr(eth, hwlro_idx);

	return 0;
}

static void mtk_hwlro_netdev_disable(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int i, hwlro_idx;

	for (i = 0; i < MTK_MAX_LRO_IP_CNT; i++) {
		mac->hwlro_ip[i] = 0;
		hwlro_idx = (mac->id * MTK_MAX_LRO_IP_CNT) + i;

		mtk_hwlro_inval_ipaddr(eth, hwlro_idx);
	}

	mac->hwlro_ip_cnt = 0;
}

static int mtk_hwlro_get_fdir_entry(struct net_device *dev,
				    struct ethtool_rxnfc *cmd)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct ethtool_rx_flow_spec *fsp =
		(struct ethtool_rx_flow_spec *)&cmd->fs;

	/* only tcp dst ipv4 is meaningful, others are meaningless */
	fsp->flow_type = TCP_V4_FLOW;
	fsp->h_u.tcp_ip4_spec.ip4dst = ntohl(mac->hwlro_ip[fsp->location]);
	fsp->m_u.tcp_ip4_spec.ip4dst = 0;

	fsp->h_u.tcp_ip4_spec.ip4src = 0;
	fsp->m_u.tcp_ip4_spec.ip4src = 0xffffffff;
	fsp->h_u.tcp_ip4_spec.psrc = 0;
	fsp->m_u.tcp_ip4_spec.psrc = 0xffff;
	fsp->h_u.tcp_ip4_spec.pdst = 0;
	fsp->m_u.tcp_ip4_spec.pdst = 0xffff;
	fsp->h_u.tcp_ip4_spec.tos = 0;
	fsp->m_u.tcp_ip4_spec.tos = 0xff;

	return 0;
}

static int mtk_hwlro_get_fdir_all(struct net_device *dev,
				  struct ethtool_rxnfc *cmd,
				  u32 *rule_locs)
{
	struct mtk_mac *mac = netdev_priv(dev);
	int cnt = 0;
	int i;

	for (i = 0; i < MTK_MAX_LRO_IP_CNT; i++) {
		if (mac->hwlro_ip[i]) {
			rule_locs[cnt] = i;
			cnt++;
		}
	}

	cmd->rule_cnt = cnt;

	return 0;
}

static int mtk_rss_init(struct mtk_eth *eth)
{
	u32 val;

	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		/* Set RSS rings to PSE modes */
		val =  mtk_r32(eth, MTK_LRO_CTRL_DW2_CFG(1));
		val |= MTK_RING_PSE_MODE;
		mtk_w32(eth, val, MTK_LRO_CTRL_DW2_CFG(1));

		/* Enable non-lro multiple rx */
		val = mtk_r32(eth, MTK_PDMA_LRO_CTRL_DW0);
		val |= MTK_NON_LRO_MULTI_EN;
		mtk_w32(eth, val, MTK_PDMA_LRO_CTRL_DW0);

		/* Enable RSS dly int supoort */
		val |= MTK_LRO_DLY_INT_EN;
		mtk_w32(eth, val, MTK_PDMA_LRO_CTRL_DW0);

		/* Set RSS delay config int ring1 */
		mtk_w32(eth, MTK_MAX_DELAY_INT, MTK_LRO_RX1_DLY_INT);
	}

	/* Hash Type */
	val = mtk_r32(eth, MTK_PDMA_RSS_GLO_CFG);
	val |= MTK_RSS_IPV4_STATIC_HASH;
	val |= MTK_RSS_IPV6_STATIC_HASH;
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Select the size of indirection table */
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW0);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW1);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW2);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW3);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW4);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW5);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW6);
	mtk_w32(eth, MTK_RSS_INDR_TABLE_SIZE4, MTK_RSS_INDR_TABLE_DW7);

	/* Pause */
	val |= MTK_RSS_CFG_REQ;
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Enable RSS*/
	val |= MTK_RSS_EN;
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Release pause */
	val &= ~(MTK_RSS_CFG_REQ);
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Set perRSS GRP INT */
	mtk_w32(eth, MTK_RX_DONE_INT(MTK_RSS_RING1), MTK_PDMA_INT_GRP3);

	/* Set GRP INT */
	mtk_w32(eth, 0x21021030, MTK_FE_INT_GRP);

	return 0;
}

static void mtk_rss_uninit(struct mtk_eth *eth)
{
	u32 val;

	/* Pause */
	val = mtk_r32(eth, MTK_PDMA_RSS_GLO_CFG);
	val |= MTK_RSS_CFG_REQ;
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Disable RSS*/
	val &= ~(MTK_RSS_EN);
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);

	/* Release pause */
	val &= ~(MTK_RSS_CFG_REQ);
	mtk_w32(eth, val, MTK_PDMA_RSS_GLO_CFG);
}

static netdev_features_t mtk_fix_features(struct net_device *dev,
					  netdev_features_t features)
{
	if (!(features & NETIF_F_LRO)) {
		struct mtk_mac *mac = netdev_priv(dev);
		int ip_cnt = mtk_hwlro_get_ip_cnt(mac);

		if (ip_cnt) {
			netdev_info(dev, "RX flow is programmed, LRO should keep on\n");

			features |= NETIF_F_LRO;
		}
	}

	if ((features & NETIF_F_HW_VLAN_CTAG_TX) && netdev_uses_dsa(dev)) {
		netdev_info(dev, "TX vlan offload cannot be enabled when dsa is attached.\n");

		features &= ~NETIF_F_HW_VLAN_CTAG_TX;
	}

	return features;
}

static int mtk_set_features(struct net_device *dev, netdev_features_t features)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int err = 0;

	if (!((dev->features ^ features) & MTK_SET_FEATURES))
		return 0;

	if (!(features & NETIF_F_LRO))
		mtk_hwlro_netdev_disable(dev);

	if (!(features & NETIF_F_HW_VLAN_CTAG_RX))
		mtk_w32(eth, 0, MTK_CDMP_EG_CTRL);
	else
		mtk_w32(eth, 1, MTK_CDMP_EG_CTRL);

	return err;
}

/* wait for DMA to finish whatever it is doing before we start using it again */
static int mtk_dma_busy_wait(struct mtk_eth *eth)
{
	unsigned long t_start = jiffies;

	while (1) {
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
			if (!(mtk_r32(eth, MTK_QDMA_GLO_CFG) &
			      (MTK_RX_DMA_BUSY | MTK_TX_DMA_BUSY)))
				return 0;
		} else {
			if (!(mtk_r32(eth, MTK_PDMA_GLO_CFG) &
			      (MTK_RX_DMA_BUSY | MTK_TX_DMA_BUSY)))
				return 0;
		}

		if (time_after(jiffies, t_start + MTK_DMA_BUSY_TIMEOUT))
			break;
	}

	dev_err(eth->dev, "DMA init timeout\n");
	return -1;
}

static int mtk_dma_init(struct mtk_eth *eth)
{
	int err;
	u32 i;

	if (mtk_dma_busy_wait(eth))
		return -EBUSY;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		/* QDMA needs scratch memory for internal reordering of the
		 * descriptors
		 */
		err = mtk_init_fq_dma(eth);
		if (err)
			return err;
	}

	err = mtk_tx_alloc(eth);
	if (err)
		return err;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		err = mtk_rx_alloc(eth, 0, MTK_RX_FLAGS_QDMA);
		if (err)
			return err;
	}

	err = mtk_rx_alloc(eth, 0, MTK_RX_FLAGS_NORMAL);
	if (err)
		return err;

	if (eth->hwlro) {
		i = (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) ? 4 : 1;
		for (; i < MTK_MAX_RX_RING_NUM; i++) {
			err = mtk_rx_alloc(eth, i, MTK_RX_FLAGS_HWLRO);
			if (err)
				return err;
		}
		err = mtk_hwlro_rx_init(eth);
		if (err)
			return err;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		for (i = 1; i < MTK_RX_NAPI_NUM; i++) {
			err = mtk_rx_alloc(eth, i, MTK_RX_FLAGS_NORMAL);
			if (err)
				return err;
		}
		err = mtk_rss_init(eth);
		if (err)
                        return err;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		/* Enable random early drop and set drop threshold
		 * automatically
		 */
		mtk_w32(eth, FC_THRES_DROP_MODE | FC_THRES_DROP_EN |
			FC_THRES_MIN, MTK_QDMA_FC_THRES);
		mtk_w32(eth, 0x0, MTK_QDMA_HRED2);
	}

	return 0;
}

static void mtk_dma_free(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++)
		if (eth->netdev[i])
			netdev_reset_queue(eth->netdev[i]);
	if ( !eth->soc->has_sram && eth->scratch_ring) {
		dma_free_coherent(eth->dev,
				  MTK_DMA_SIZE * sizeof(struct mtk_tx_dma),
				  eth->scratch_ring,
				  eth->phy_scratch_ring);
		eth->scratch_ring = NULL;
		eth->phy_scratch_ring = 0;
	}
	mtk_tx_clean(eth);
	mtk_rx_clean(eth, &eth->rx_ring[0],1);
	mtk_rx_clean(eth, &eth->rx_ring_qdma,0);

	if (eth->hwlro) {
		mtk_hwlro_rx_uninit(eth);

		i = (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) ? 4 : 1;
		for (; i < MTK_MAX_RX_RING_NUM; i++)
			mtk_rx_clean(eth, &eth->rx_ring[i], 0);
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		mtk_rss_uninit(eth);

		for (i = 1; i < MTK_RX_NAPI_NUM; i++)
			mtk_rx_clean(eth, &eth->rx_ring[i], 1);
	}

	if (eth->scratch_head) {
		kfree(eth->scratch_head);
		eth->scratch_head = NULL;
	}
}

static void mtk_tx_timeout(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;

	eth->netdev[mac->id]->stats.tx_errors++;
	netif_err(eth, tx_err, dev,
		  "transmit timed out\n");

	if (atomic_read(&reset_lock) == 0)
		schedule_work(&eth->pending_work);
}

static irqreturn_t mtk_handle_irq_rx(int irq, void *priv)
{
	struct mtk_napi *rx_napi = priv;
	struct mtk_eth *eth = rx_napi->eth;
	struct mtk_rx_ring *ring = rx_napi->rx_ring;

	if (likely(napi_schedule_prep(&rx_napi->napi))) {
		mtk_rx_irq_disable(eth, MTK_RX_DONE_INT(ring->ring_no));
		__napi_schedule(&rx_napi->napi);
	}

	return IRQ_HANDLED;
}

static irqreturn_t mtk_handle_irq_tx(int irq, void *_eth)
{
	struct mtk_eth *eth = _eth;

	if (likely(napi_schedule_prep(&eth->tx_napi))) {
		mtk_tx_irq_disable(eth, MTK_TX_DONE_INT);
		__napi_schedule(&eth->tx_napi);
	}

	return IRQ_HANDLED;
}

static irqreturn_t mtk_handle_irq(int irq, void *_eth)
{
	struct mtk_eth *eth = _eth;

	if (mtk_r32(eth, MTK_PDMA_INT_MASK) & MTK_RX_DONE_INT(0)) {
		if (mtk_r32(eth, MTK_PDMA_INT_STATUS) & MTK_RX_DONE_INT(0))
			mtk_handle_irq_rx(irq, &eth->rx_napi[0]);
	}
	if (mtk_r32(eth, eth->tx_int_mask_reg) & MTK_TX_DONE_INT) {
		if (mtk_r32(eth, eth->tx_int_status_reg) & MTK_TX_DONE_INT)
			mtk_handle_irq_tx(irq, _eth);
	}

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
static void mtk_poll_controller(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;

	mtk_tx_irq_disable(eth, MTK_TX_DONE_INT);
	mtk_rx_irq_disable(eth, MTK_RX_DONE_INT(0));
	mtk_handle_irq_rx(eth->irq[2], &eth->rx_napi[0]);
	mtk_tx_irq_enable(eth, MTK_TX_DONE_INT);
	mtk_rx_irq_enable(eth, MTK_RX_DONE_INT(0));
}
#endif

static int mtk_start_dma(struct mtk_eth *eth)
{
	u32 rx_2b_offset = (NET_IP_ALIGN == 2) ? MTK_RX_2B_OFFSET : 0;
	int val, err;

	err = mtk_dma_init(eth);
	if (err) {
		mtk_dma_free(eth);
		return err;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		val = mtk_r32(eth, MTK_QDMA_GLO_CFG);
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
			val &= ~MTK_RESV_BUF_MASK;
			mtk_w32(eth,
				val | MTK_TX_DMA_EN | MTK_RX_DMA_EN |
				MTK_DMA_SIZE_32DWORDS | MTK_TX_WB_DDONE |
				MTK_NDP_CO_PRO | MTK_MUTLI_CNT |
				MTK_RESV_BUF | MTK_WCOMP_EN |
				MTK_DMAD_WR_WDONE | MTK_CHK_DDONE_EN |
				MTK_RX_2B_OFFSET, MTK_QDMA_GLO_CFG);
		}
		else
			mtk_w32(eth,
				val | MTK_TX_DMA_EN |
				MTK_DMA_SIZE_32DWORDS | MTK_NDP_CO_PRO |
				MTK_RX_DMA_EN | MTK_RX_2B_OFFSET |
				MTK_RX_BT_32DWORDS,
				MTK_QDMA_GLO_CFG);

		val = mtk_r32(eth, MTK_PDMA_GLO_CFG);
		mtk_w32(eth,
			val | MTK_RX_DMA_EN | rx_2b_offset |
			MTK_RX_BT_32DWORDS | MTK_MULTI_EN,
			MTK_PDMA_GLO_CFG);
	} else {
		mtk_w32(eth, MTK_TX_WB_DDONE | MTK_TX_DMA_EN | MTK_RX_DMA_EN |
			MTK_MULTI_EN | MTK_PDMA_SIZE_8DWORDS,
			MTK_PDMA_GLO_CFG);
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2) && eth->hwlro) {
		val = mtk_r32(eth, MTK_PDMA_GLO_CFG);
		mtk_w32(eth, val | MTK_RX_DMA_LRO_EN, MTK_PDMA_GLO_CFG);
	}

	return 0;
}

void mtk_gdm_config(struct mtk_eth *eth, u32 config)
{
	int i;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628))
		return;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		u32 val = mtk_r32(eth, MTK_GDMA_FWD_CFG(i));

		/* default setup the forward port to send frame to PDMA */
		val &= ~0xffff;

		/* Enable RX checksum */
		val |= MTK_GDMA_ICS_EN | MTK_GDMA_TCS_EN | MTK_GDMA_UCS_EN;

		val |= config;

		if (eth->netdev[i] && netdev_uses_dsa(eth->netdev[i]))
			val |= MTK_GDMA_SPECIAL_TAG;

		mtk_w32(eth, val, MTK_GDMA_FWD_CFG(i));
	}
}

static int mtk_open(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int err, i;
	struct device_node *phy_node;

	err = phylink_of_phy_connect(mac->phylink, mac->of_node, 0);
	if (err) {
		netdev_err(dev, "%s: could not attach PHY: %d\n", __func__,
			   err);
		return err;
	}

	/* we run 2 netdevs on the same dma ring so we only bring it up once */
	if (!refcount_read(&eth->dma_refcnt)) {
		int err = mtk_start_dma(eth);

		if (err)
			return err;

		mtk_gdm_config(eth, MTK_GDMA_TO_PDMA);

		/* Indicates CDM to parse the MTK special tag from CPU */
		if (netdev_uses_dsa(dev)) {
			u32 val;
			val = mtk_r32(eth, MTK_CDMQ_IG_CTRL);
			mtk_w32(eth, val | MTK_CDMQ_STAG_EN, MTK_CDMQ_IG_CTRL);
			val = mtk_r32(eth, MTK_CDMP_IG_CTRL);
			mtk_w32(eth, val | MTK_CDMP_STAG_EN, MTK_CDMP_IG_CTRL);
		}

		napi_enable(&eth->tx_napi);
		napi_enable(&eth->rx_napi[0].napi);
		mtk_tx_irq_enable(eth, MTK_TX_DONE_INT);
		mtk_rx_irq_enable(eth, MTK_RX_DONE_INT(0));

		if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
			for (i = 1; i < MTK_RX_NAPI_NUM; i++) {
				napi_enable(&eth->rx_napi[i].napi);
				mtk_rx_irq_enable(eth, MTK_RX_DONE_INT(i));
			}
		}

		refcount_set(&eth->dma_refcnt, 1);
	}
	else
		refcount_inc(&eth->dma_refcnt);

	phylink_start(mac->phylink);
	netif_start_queue(dev);

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	phy_node = of_parse_phandle(mac->of_node, "phy-handle", 0);
	if (!phy_node) {
		regmap_write(eth->sgmii->regmap[mac->id], SGMSYS_QPHY_PWR_STATE_CTRL, 0);
	}
#endif

	return 0;
}

static void mtk_stop_dma(struct mtk_eth *eth, u32 glo_cfg)
{
	u32 val;
	int i;

	/* stop the dma engine */
	spin_lock_bh(&eth->page_lock);
	val = mtk_r32(eth, glo_cfg);
	mtk_w32(eth, val & ~(MTK_TX_WB_DDONE | MTK_RX_DMA_EN | MTK_TX_DMA_EN),
		glo_cfg);
	spin_unlock_bh(&eth->page_lock);

	/* wait for dma stop */
	for (i = 0; i < 10; i++) {
		val = mtk_r32(eth, glo_cfg);
		if (val & (MTK_TX_DMA_BUSY | MTK_RX_DMA_BUSY)) {
			mdelay(20);
			continue;
		}
		break;
	}
}

static int mtk_stop(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;
	int i;
	u32 val = 0;
	struct device_node *phy_node;

	netif_tx_disable(dev);

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	phy_node = of_parse_phandle(mac->of_node, "phy-handle", 0);
	if (phy_node) {
		val = _mtk_mdio_read(eth, 0, 0);
		val |= BMCR_PDOWN;
		_mtk_mdio_write(eth, 0, 0, val);
	}else {
		regmap_read(eth->sgmii->regmap[mac->id], SGMSYS_QPHY_PWR_STATE_CTRL, &val);
		val |= SGMII_PHYA_PWD;
		regmap_write(eth->sgmii->regmap[mac->id], SGMSYS_QPHY_PWR_STATE_CTRL, val);
	}
#endif

	//GMAC RX disable
	val = mtk_r32(eth, MTK_MAC_MCR(mac->id));
	mtk_w32(eth, val & ~(MAC_MCR_RX_EN), MTK_MAC_MCR(mac->id));

	phylink_stop(mac->phylink);

	phylink_disconnect_phy(mac->phylink);

	/* only shutdown DMA if this is the last user */
	if (!refcount_dec_and_test(&eth->dma_refcnt))
		return 0;

	mtk_gdm_config(eth, MTK_GDMA_DROP_ALL);

	mtk_tx_irq_disable(eth, MTK_TX_DONE_INT);
	mtk_rx_irq_disable(eth, MTK_RX_DONE_INT(0));
	napi_disable(&eth->tx_napi);
	napi_disable(&eth->rx_napi[0].napi);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		for (i = 1; i < MTK_RX_NAPI_NUM; i++) {
			mtk_rx_irq_disable(eth, MTK_RX_DONE_INT(i));
			napi_disable(&eth->rx_napi[i].napi);
		}
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA))
		mtk_stop_dma(eth, MTK_QDMA_GLO_CFG);
	mtk_stop_dma(eth, MTK_PDMA_GLO_CFG);

	mtk_dma_free(eth);

	return 0;
}

void ethsys_reset(struct mtk_eth *eth, u32 reset_bits)
{
	u32 val = 0, i = 0;

	regmap_update_bits(eth->ethsys, ETHSYS_RSTCTRL,
			   reset_bits, reset_bits);

	while (i++ < 5000) {
		mdelay(1);
		regmap_read(eth->ethsys, ETHSYS_RSTCTRL, &val);

		if ((val & reset_bits) == reset_bits) {
			mtk_reset_event_update(eth, MTK_EVENT_COLD_CNT);
			regmap_update_bits(eth->ethsys, ETHSYS_RSTCTRL,
					   reset_bits, ~reset_bits);
			break;
		}
	}

	mdelay(10);
}

static void mtk_clk_disable(struct mtk_eth *eth)
{
	int clk;

	for (clk = MTK_CLK_MAX - 1; clk >= 0; clk--)
		clk_disable_unprepare(eth->clks[clk]);
}

static int mtk_clk_enable(struct mtk_eth *eth)
{
	int clk, ret;

	for (clk = 0; clk < MTK_CLK_MAX ; clk++) {
		ret = clk_prepare_enable(eth->clks[clk]);
		if (ret)
			goto err_disable_clks;
	}

	return 0;

err_disable_clks:
	while (--clk >= 0)
		clk_disable_unprepare(eth->clks[clk]);

	return ret;
}

static int mtk_napi_init(struct mtk_eth *eth)
{
	struct mtk_napi *rx_napi = &eth->rx_napi[0];
	int i;

	rx_napi->eth = eth;
	rx_napi->rx_ring = &eth->rx_ring[0];
	rx_napi->irq_grp_no = 2;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		for (i = 1; i < MTK_RX_NAPI_NUM; i++) {
			rx_napi = &eth->rx_napi[i];
			rx_napi->eth = eth;
			rx_napi->rx_ring = &eth->rx_ring[i];
			rx_napi->irq_grp_no = 2 + i;
		}
	}

	return 0;
}

static int mtk_hw_init(struct mtk_eth *eth, u32 type)
{
	int i, ret = 0;

	pr_info("[%s] reset_lock:%d, force:%d\n", __func__,
		atomic_read(&reset_lock), atomic_read(&force));

	if (atomic_read(&reset_lock) == 0) {
		if (test_and_set_bit(MTK_HW_INIT, &eth->state))
			return 0;

		pm_runtime_enable(eth->dev);
		pm_runtime_get_sync(eth->dev);

		ret = mtk_clk_enable(eth);
		if (ret)
			goto err_disable_pm;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
		ret = device_reset(eth->dev);
		if (ret) {
			dev_err(eth->dev, "MAC reset failed!\n");
			goto err_disable_pm;
		}

		/* enable interrupt delay for RX */
		mtk_w32(eth, MTK_PDMA_DELAY_RX_DELAY, MTK_PDMA_DELAY_INT);

		/* disable delay and normal interrupt */
		mtk_tx_irq_disable(eth, ~0);
		mtk_rx_irq_disable(eth, ~0);

		return 0;
	}

	pr_info("[%s] execute fe %s reset\n", __func__,
		(type == MTK_TYPE_WARM_RESET) ? "warm" : "cold");

	if (type == MTK_TYPE_WARM_RESET)
		mtk_eth_warm_reset(eth);
	else
		mtk_eth_cold_reset(eth);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		/* Set FE to PDMAv2 if necessary */
		mtk_w32(eth, mtk_r32(eth, MTK_FE_GLO_MISC) | MTK_PDMA_V2, MTK_FE_GLO_MISC);
	}

	if (eth->pctl) {
		/* Set GE2 driving and slew rate */
		regmap_write(eth->pctl, GPIO_DRV_SEL10, 0xa00);

		/* set GE2 TDSEL */
		regmap_write(eth->pctl, GPIO_OD33_CTRL8, 0x5);

		/* set GE2 TUNE */
		regmap_write(eth->pctl, GPIO_BIAS_CTRL, 0x0);
	}

	/* Set linkdown as the default for each GMAC. Its own MCR would be set
	 * up with the more appropriate value when mtk_mac_config call is being
	 * invoked.
	 */
	for (i = 0; i < MTK_MAC_COUNT; i++)
		mtk_w32(eth, MAC_MCR_FORCE_LINK_DOWN, MTK_MAC_MCR(i));

	/* Enable RX VLan Offloading */
	if (eth->soc->hw_features & NETIF_F_HW_VLAN_CTAG_RX)
		mtk_w32(eth, 1, MTK_CDMP_EG_CTRL);
	else
		mtk_w32(eth, 0, MTK_CDMP_EG_CTRL);

	/* enable interrupt delay for RX/TX */
	mtk_w32(eth, 0x8f0f8f0f, MTK_PDMA_DELAY_INT);
	mtk_w32(eth, 0x8f0f8f0f, MTK_QDMA_DELAY_INT);

	mtk_tx_irq_disable(eth, ~0);
	mtk_rx_irq_disable(eth, ~0);

	/* FE int grouping */
	mtk_w32(eth, MTK_TX_DONE_INT, MTK_PDMA_INT_GRP1);
	mtk_w32(eth, MTK_RX_DONE_INT(0), MTK_PDMA_INT_GRP2);
	mtk_w32(eth, MTK_TX_DONE_INT, MTK_QDMA_INT_GRP1);
	mtk_w32(eth, MTK_RX_DONE_INT(0), MTK_QDMA_INT_GRP2);
	mtk_w32(eth, 0x21021003, MTK_FE_INT_GRP);
	mtk_w32(eth, MTK_FE_INT_FQ_EMPTY | MTK_FE_INT_TSO_FAIL |
		MTK_FE_INT_TSO_ILLEGAL | MTK_FE_INT_TSO_ALIGN |
		MTK_FE_INT_RFIFO_OV | MTK_FE_INT_RFIFO_UF, MTK_FE_INT_ENABLE);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2)) {
		/* PSE Free Queue Flow Control  */
		mtk_w32(eth, 0x01fa01f4, PSE_FQFC_CFG2);

		/* PSE should not drop port8 and port9 packets */
		mtk_w32(eth, 0x00000300, PSE_DROP_CFG);

		/* PSE config input queue threshold */
		mtk_w32(eth, 0x001a000e, PSE_IQ_REV(1));
		mtk_w32(eth, 0x01ff001a, PSE_IQ_REV(2));
		mtk_w32(eth, 0x000e01ff, PSE_IQ_REV(3));
		mtk_w32(eth, 0x000e000e, PSE_IQ_REV(4));
		mtk_w32(eth, 0x000e000e, PSE_IQ_REV(5));
		mtk_w32(eth, 0x000e000e, PSE_IQ_REV(6));
		mtk_w32(eth, 0x000e000e, PSE_IQ_REV(7));
		mtk_w32(eth, 0x002a000e, PSE_IQ_REV(8));

		/* PSE config output queue threshold */
		mtk_w32(eth, 0x000f000a, PSE_OQ_TH(1));
		mtk_w32(eth, 0x001a000f, PSE_OQ_TH(2));
		mtk_w32(eth, 0x000f001a, PSE_OQ_TH(3));
		mtk_w32(eth, 0x01ff000f, PSE_OQ_TH(4));
		mtk_w32(eth, 0x000f000f, PSE_OQ_TH(5));
		mtk_w32(eth, 0x0006000f, PSE_OQ_TH(6));
		mtk_w32(eth, 0x00060006, PSE_OQ_TH(7));
		mtk_w32(eth, 0x00060006, PSE_OQ_TH(8));

		/* GDM and CDM Threshold */
		mtk_w32(eth, 0x00000004, MTK_GDM2_THRES);
                mtk_w32(eth, 0x00000004, MTK_CDMW0_THRES);
                mtk_w32(eth, 0x00000004, MTK_CDMW1_THRES);
                mtk_w32(eth, 0x00000004, MTK_CDME0_THRES);
                mtk_w32(eth, 0x00000004, MTK_CDME1_THRES);
                mtk_w32(eth, 0x00000004, MTK_CDMM_THRES);
	}

	return 0;

err_disable_pm:
	pm_runtime_put_sync(eth->dev);
	pm_runtime_disable(eth->dev);

	return ret;
}

static int mtk_hw_deinit(struct mtk_eth *eth)
{
	if (!test_and_clear_bit(MTK_HW_INIT, &eth->state))
		return 0;

	mtk_clk_disable(eth);

	pm_runtime_put_sync(eth->dev);
	pm_runtime_disable(eth->dev);

	return 0;
}

static int __init mtk_init(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;

	of_get_mac_address(mac->of_node, dev->dev_addr);

	/* If the mac address is invalid, use random mac address  */
	if (!is_valid_ether_addr(dev->dev_addr)) {
		eth_hw_addr_random(dev);
		dev_err(eth->dev, "generated random MAC address %pM\n",
			dev->dev_addr);
	}

	return 0;
}

static void mtk_uninit(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_eth *eth = mac->hw;

	phylink_disconnect_phy(mac->phylink);
	mtk_tx_irq_disable(eth, ~0);
	mtk_rx_irq_disable(eth, ~0);
}

static int mtk_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct mtk_mac *mac = netdev_priv(dev);

	switch (cmd) {
	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		return phylink_mii_ioctl(mac->phylink, ifr, cmd);
	default:
		/* default invoke the mtk_eth_dbg handler */
		return mtk_do_priv_ioctl(dev, ifr, cmd);
		break;
	}

	return -EOPNOTSUPP;
}

static void mtk_pending_work(struct work_struct *work)
{
	struct mtk_eth *eth = container_of(work, struct mtk_eth, pending_work);
	struct device_node *phy_node = NULL;
	struct mtk_mac *mac = NULL;
	int err, i = 0;
	unsigned long restart = 0;
	u32 val = 0;

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	atomic_inc(&reset_lock);
	val = mtk_r32(eth, MTK_FE_INT_STATUS);
	if (!mtk_check_reset_event(eth, val)) {
		atomic_dec(&reset_lock);
		pr_info("[%s] No need to do FE reset !\n", __func__);
		return;
	}

	rtnl_lock();

	/* Disabe FE P3 and P4 */
	val = mtk_r32(eth, MTK_FE_GLO_CFG);
	val |= MTK_FE_LINK_DOWN_P3;
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		val |= MTK_FE_LINK_DOWN_P4;
	mtk_w32(eth, val, MTK_FE_GLO_CFG);

	/* Adjust PPE configurations to prepare for reset */
	mtk_prepare_reset_ppe(eth, 0);
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		mtk_prepare_reset_ppe(eth, 1);

	/* Adjust FE configurations to prepare for reset */
	mtk_prepare_reset_fe(eth);

	/* Trigger Wifi SER reset */
	call_netdevice_notifiers(MTK_FE_START_RESET, eth->netdev[0]);
	rtnl_unlock();
	wait_for_completion_timeout(&wait_ser_done, 5000);
#endif
	rtnl_lock();

	while (test_and_set_bit_lock(MTK_RESETTING, &eth->state))
		cpu_relax();

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	del_timer_sync(&eth->mtk_dma_monitor_timer);
#endif
	pr_info("[%s] mtk_stop starts !\n", __func__);
	/* stop all devices to make sure that dma is properly shut down */
	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		mtk_stop(eth->netdev[i]);
		__set_bit(i, &restart);
	}
	pr_info("[%s] mtk_stop ends !\n", __func__);
	mdelay(15);

	if (eth->dev->pins)
		pinctrl_select_state(eth->dev->pins->p,
				     eth->dev->pins->default_state);

	pr_info("[%s] mtk_hw_init starts !\n", __func__);
	mtk_hw_init(eth, MTK_TYPE_WARM_RESET);
	pr_info("[%s] mtk_hw_init ends !\n", __func__);

	/* restart DMA and enable IRQs */
	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!test_bit(i, &restart))
			continue;
		err = mtk_open(eth->netdev[i]);
		if (err) {
			netif_alert(eth, ifup, eth->netdev[i],
			      "Driver up/down cycle failed, closing device.\n");
			dev_close(eth->netdev[i]);
		}
	}

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	/* Set KA tick select */
	mtk_m32(eth, MTK_PPE_TICK_SEL_MASK, 0, MTK_PPE_TB_CFG(0));
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		mtk_m32(eth, MTK_PPE_TICK_SEL_MASK, 0, MTK_PPE_TB_CFG(1));

	/* Enabe FE P3 and P4*/
	val = mtk_r32(eth, MTK_FE_GLO_CFG);
	val &= ~MTK_FE_LINK_DOWN_P3;
	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSTCTRL_PPE1))
		val &= ~MTK_FE_LINK_DOWN_P4;
	mtk_w32(eth, val, MTK_FE_GLO_CFG);

	/* Power up sgmii */
	for (i = 0; i < MTK_MAC_COUNT; i++) {
		mac = netdev_priv(eth->netdev[i]);
		phy_node = of_parse_phandle(mac->of_node, "phy-handle", 0);
		if (!phy_node) {
			mtk_gmac_sgmii_path_setup(eth, i);
			regmap_write(eth->sgmii->regmap[i], SGMSYS_QPHY_PWR_STATE_CTRL, 0);
		}
	}

	call_netdevice_notifiers(MTK_FE_RESET_NAT_DONE, eth->netdev[0]);
	pr_info("[%s] HNAT reset done !\n", __func__);

	call_netdevice_notifiers(MTK_FE_RESET_DONE, eth->netdev[0]);
	pr_info("[%s] WiFi SER reset done !\n", __func__);

	atomic_dec(&reset_lock);
	if (atomic_read(&force) > 0)
		atomic_dec(&force);

	timer_setup(&eth->mtk_dma_monitor_timer, mtk_dma_monitor, 0);
	eth->mtk_dma_monitor_timer.expires = jiffies;
	add_timer(&eth->mtk_dma_monitor_timer);
#endif
	clear_bit_unlock(MTK_RESETTING, &eth->state);

	rtnl_unlock();
}

static int mtk_free_dev(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		free_netdev(eth->netdev[i]);
	}

	return 0;
}

static int mtk_unreg_dev(struct mtk_eth *eth)
{
	int i;

	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		unregister_netdev(eth->netdev[i]);
	}

	return 0;
}

static int mtk_cleanup(struct mtk_eth *eth)
{
	mtk_unreg_dev(eth);
	mtk_free_dev(eth);
	cancel_work_sync(&eth->pending_work);

	return 0;
}

static int mtk_get_link_ksettings(struct net_device *ndev,
				  struct ethtool_link_ksettings *cmd)
{
	struct mtk_mac *mac = netdev_priv(ndev);

	if (unlikely(test_bit(MTK_RESETTING, &mac->hw->state)))
		return -EBUSY;

	return phylink_ethtool_ksettings_get(mac->phylink, cmd);
}

static int mtk_set_link_ksettings(struct net_device *ndev,
				  const struct ethtool_link_ksettings *cmd)
{
	struct mtk_mac *mac = netdev_priv(ndev);

	if (unlikely(test_bit(MTK_RESETTING, &mac->hw->state)))
		return -EBUSY;

	return phylink_ethtool_ksettings_set(mac->phylink, cmd);
}

static void mtk_get_drvinfo(struct net_device *dev,
			    struct ethtool_drvinfo *info)
{
	struct mtk_mac *mac = netdev_priv(dev);

	strlcpy(info->driver, mac->hw->dev->driver->name, sizeof(info->driver));
	strlcpy(info->bus_info, dev_name(mac->hw->dev), sizeof(info->bus_info));
	info->n_stats = ARRAY_SIZE(mtk_ethtool_stats);
}

static u32 mtk_get_msglevel(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);

	return mac->hw->msg_enable;
}

static void mtk_set_msglevel(struct net_device *dev, u32 value)
{
	struct mtk_mac *mac = netdev_priv(dev);

	mac->hw->msg_enable = value;
}

static int mtk_nway_reset(struct net_device *dev)
{
	struct mtk_mac *mac = netdev_priv(dev);

	if (unlikely(test_bit(MTK_RESETTING, &mac->hw->state)))
		return -EBUSY;

	if (!mac->phylink)
		return -ENOTSUPP;

	return phylink_ethtool_nway_reset(mac->phylink);
}

static void mtk_get_strings(struct net_device *dev, u32 stringset, u8 *data)
{
	int i;

	switch (stringset) {
	case ETH_SS_STATS:
		for (i = 0; i < ARRAY_SIZE(mtk_ethtool_stats); i++) {
			memcpy(data, mtk_ethtool_stats[i].str, ETH_GSTRING_LEN);
			data += ETH_GSTRING_LEN;
		}
		break;
	}
}

static int mtk_get_sset_count(struct net_device *dev, int sset)
{
	switch (sset) {
	case ETH_SS_STATS:
		return ARRAY_SIZE(mtk_ethtool_stats);
	default:
		return -EOPNOTSUPP;
	}
}

static void mtk_get_ethtool_stats(struct net_device *dev,
				  struct ethtool_stats *stats, u64 *data)
{
	struct mtk_mac *mac = netdev_priv(dev);
	struct mtk_hw_stats *hwstats = mac->hw_stats;
	u64 *data_src, *data_dst;
	unsigned int start;
	int i;

	if (unlikely(test_bit(MTK_RESETTING, &mac->hw->state)))
		return;

	if (netif_running(dev) && netif_device_present(dev)) {
		if (spin_trylock_bh(&hwstats->stats_lock)) {
			mtk_stats_update_mac(mac);
			spin_unlock_bh(&hwstats->stats_lock);
		}
	}

	data_src = (u64 *)hwstats;

	do {
		data_dst = data;
		start = u64_stats_fetch_begin_irq(&hwstats->syncp);

		for (i = 0; i < ARRAY_SIZE(mtk_ethtool_stats); i++)
			*data_dst++ = *(data_src + mtk_ethtool_stats[i].offset);
	} while (u64_stats_fetch_retry_irq(&hwstats->syncp, start));
}

static int mtk_get_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd,
			 u32 *rule_locs)
{
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_GRXRINGS:
		if (dev->hw_features & NETIF_F_LRO) {
			cmd->data = MTK_MAX_RX_RING_NUM;
			ret = 0;
		}
		break;
	case ETHTOOL_GRXCLSRLCNT:
		if (dev->hw_features & NETIF_F_LRO) {
			struct mtk_mac *mac = netdev_priv(dev);

			cmd->rule_cnt = mac->hwlro_ip_cnt;
			ret = 0;
		}
		break;
	case ETHTOOL_GRXCLSRULE:
		if (dev->hw_features & NETIF_F_LRO)
			ret = mtk_hwlro_get_fdir_entry(dev, cmd);
		break;
	case ETHTOOL_GRXCLSRLALL:
		if (dev->hw_features & NETIF_F_LRO)
			ret = mtk_hwlro_get_fdir_all(dev, cmd,
						     rule_locs);
		break;
	default:
		break;
	}

	return ret;
}

static int mtk_set_rxnfc(struct net_device *dev, struct ethtool_rxnfc *cmd)
{
	int ret = -EOPNOTSUPP;

	switch (cmd->cmd) {
	case ETHTOOL_SRXCLSRLINS:
		if (dev->hw_features & NETIF_F_LRO)
			ret = mtk_hwlro_add_ipaddr(dev, cmd);
		break;
	case ETHTOOL_SRXCLSRLDEL:
		if (dev->hw_features & NETIF_F_LRO)
			ret = mtk_hwlro_del_ipaddr(dev, cmd);
		break;
	default:
		break;
	}

	return ret;
}

static const struct ethtool_ops mtk_ethtool_ops = {
	.get_link_ksettings	= mtk_get_link_ksettings,
	.set_link_ksettings	= mtk_set_link_ksettings,
	.get_drvinfo		= mtk_get_drvinfo,
	.get_msglevel		= mtk_get_msglevel,
	.set_msglevel		= mtk_set_msglevel,
	.nway_reset		= mtk_nway_reset,
	.get_link		= ethtool_op_get_link,
	.get_strings		= mtk_get_strings,
	.get_sset_count		= mtk_get_sset_count,
	.get_ethtool_stats	= mtk_get_ethtool_stats,
	.get_rxnfc		= mtk_get_rxnfc,
	.set_rxnfc              = mtk_set_rxnfc,
};

static const struct net_device_ops mtk_netdev_ops = {
	.ndo_init		= mtk_init,
	.ndo_uninit		= mtk_uninit,
	.ndo_open		= mtk_open,
	.ndo_stop		= mtk_stop,
	.ndo_start_xmit		= mtk_start_xmit,
	.ndo_set_mac_address	= mtk_set_mac_address,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_do_ioctl		= mtk_do_ioctl,
	.ndo_tx_timeout		= mtk_tx_timeout,
	.ndo_get_stats64        = mtk_get_stats64,
	.ndo_fix_features	= mtk_fix_features,
	.ndo_set_features	= mtk_set_features,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= mtk_poll_controller,
#endif
};

static int mtk_add_mac(struct mtk_eth *eth, struct device_node *np)
{
	const __be32 *_id = of_get_property(np, "reg", NULL);
	struct phylink *phylink;
	int phy_mode, id, err;
	struct mtk_mac *mac;

	if (!_id) {
		dev_err(eth->dev, "missing mac id\n");
		return -EINVAL;
	}

	id = be32_to_cpup(_id);
	if (id < 0 || id >= MTK_MAC_COUNT) {
		dev_err(eth->dev, "%d is not a valid mac id\n", id);
		return -EINVAL;
	}

	if (eth->netdev[id]) {
		dev_err(eth->dev, "duplicate mac id found: %d\n", id);
		return -EINVAL;
	}

	eth->netdev[id] = alloc_etherdev(sizeof(*mac));
	if (!eth->netdev[id]) {
		dev_err(eth->dev, "alloc_etherdev failed\n");
		return -ENOMEM;
	}
	mac = netdev_priv(eth->netdev[id]);
	eth->mac[id] = mac;
	mac->id = id;
	mac->hw = eth;
	mac->of_node = np;

	memset(mac->hwlro_ip, 0, sizeof(mac->hwlro_ip));
	mac->hwlro_ip_cnt = 0;

	mac->hw_stats = devm_kzalloc(eth->dev,
				     sizeof(*mac->hw_stats),
				     GFP_KERNEL);
	if (!mac->hw_stats) {
		dev_err(eth->dev, "failed to allocate counter memory\n");
		err = -ENOMEM;
		goto free_netdev;
	}
	spin_lock_init(&mac->hw_stats->stats_lock);
	u64_stats_init(&mac->hw_stats->syncp);
	mac->hw_stats->reg_offset = id * MTK_STAT_OFFSET;

	/* phylink create */
	phy_mode = of_get_phy_mode(np);
	if (phy_mode < 0) {
		dev_err(eth->dev, "incorrect phy-mode\n");
		err = -EINVAL;
		goto free_netdev;
	}

	/* mac config is not set */
	mac->interface = PHY_INTERFACE_MODE_NA;
	mac->mode = MLO_AN_PHY;
	mac->speed = SPEED_UNKNOWN;

	mac->phylink_config.dev = &eth->netdev[id]->dev;
	mac->phylink_config.type = PHYLINK_NETDEV;

	phylink = phylink_create(&mac->phylink_config,
				 of_fwnode_handle(mac->of_node),
				 phy_mode, &mtk_phylink_ops);
	if (IS_ERR(phylink)) {
		err = PTR_ERR(phylink);
		goto free_netdev;
	}

	mac->phylink = phylink;

	SET_NETDEV_DEV(eth->netdev[id], eth->dev);
	eth->netdev[id]->watchdog_timeo = 5 * HZ;
	eth->netdev[id]->netdev_ops = &mtk_netdev_ops;
	eth->netdev[id]->base_addr = (unsigned long)eth->base;

	eth->netdev[id]->hw_features = eth->soc->hw_features;
	if (eth->hwlro)
		eth->netdev[id]->hw_features |= NETIF_F_LRO;

	eth->netdev[id]->vlan_features = eth->soc->hw_features &
		~(NETIF_F_HW_VLAN_CTAG_TX | NETIF_F_HW_VLAN_CTAG_RX);
	eth->netdev[id]->features |= eth->soc->hw_features;
	eth->netdev[id]->ethtool_ops = &mtk_ethtool_ops;

	eth->netdev[id]->irq = eth->irq[0];
	eth->netdev[id]->dev.of_node = np;

	return 0;

free_netdev:
	free_netdev(eth->netdev[id]);
	return err;
}

static int mtk_probe(struct platform_device *pdev)
{
	struct device_node *mac_np;
	struct mtk_eth *eth;
	int err, i;

	eth = devm_kzalloc(&pdev->dev, sizeof(*eth), GFP_KERNEL);
	if (!eth)
		return -ENOMEM;

	eth->soc = of_device_get_match_data(&pdev->dev);

	eth->dev = &pdev->dev;
	eth->base = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(eth->base))
		return PTR_ERR(eth->base);

	if(eth->soc->has_sram) {
		struct resource *res;
		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (unlikely(!res))
			return -EINVAL;
		eth->phy_scratch_ring = res->start + MTK_ETH_SRAM_OFFSET;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_QDMA)) {
		eth->tx_int_mask_reg = MTK_QDMA_INT_MASK;
		eth->tx_int_status_reg = MTK_QDMA_INT_STATUS;
	} else {
		eth->tx_int_mask_reg = MTK_PDMA_INT_MASK;
		eth->tx_int_status_reg = MTK_PDMA_INT_STATUS;
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
		eth->rx_dma_l4_valid = RX_DMA_L4_VALID_PDMA;
		eth->ip_align = NET_IP_ALIGN;
	} else {
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_NETSYS_V2))
			eth->rx_dma_l4_valid = RX_DMA_L4_VALID_V2;
		else
			eth->rx_dma_l4_valid = RX_DMA_L4_VALID;
	}

	spin_lock_init(&eth->page_lock);
	spin_lock_init(&eth->tx_irq_lock);
	spin_lock_init(&eth->rx_irq_lock);
	spin_lock_init(&eth->syscfg0_lock);

	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
		eth->ethsys = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
							      "mediatek,ethsys");
		if (IS_ERR(eth->ethsys)) {
			dev_err(&pdev->dev, "no ethsys regmap found\n");
			return PTR_ERR(eth->ethsys);
		}
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_INFRA)) {
		eth->infra = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
							     "mediatek,infracfg");
		if (IS_ERR(eth->infra)) {
			dev_err(&pdev->dev, "no infracfg regmap found\n");
			return PTR_ERR(eth->infra);
		}
	}

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SGMII)) {
		eth->sgmii = devm_kzalloc(eth->dev, sizeof(*eth->sgmii),
					  GFP_KERNEL);
		if (!eth->sgmii)
			return -ENOMEM;

		err = mtk_sgmii_init(eth->sgmii, pdev->dev.of_node,
				     eth->soc->ana_rgc3);

		if (err)
			return err;
	}

	if (eth->soc->required_pctl) {
		eth->pctl = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
							    "mediatek,pctl");
		if (IS_ERR(eth->pctl)) {
			dev_err(&pdev->dev, "no pctl regmap found\n");
			return PTR_ERR(eth->pctl);
		}
	}

	for (i = 0; i < MTK_MAX_IRQ_NUM; i++) {
		if (MTK_HAS_CAPS(eth->soc->caps, MTK_SHARED_INT) && i > 0)
			eth->irq[i] = eth->irq[0];
		else
			eth->irq[i] = platform_get_irq(pdev, i);
		if (eth->irq[i] < 0) {
			dev_err(&pdev->dev, "no IRQ%d resource found\n", i);
			return -ENXIO;
		}
	}

	for (i = 0; i < ARRAY_SIZE(eth->clks); i++) {
		eth->clks[i] = devm_clk_get(eth->dev,
					    mtk_clks_source_name[i]);
		if (IS_ERR(eth->clks[i])) {
			if (PTR_ERR(eth->clks[i]) == -EPROBE_DEFER)
				return -EPROBE_DEFER;
			if (eth->soc->required_clks & BIT(i)) {
				dev_err(&pdev->dev, "clock %s not found\n",
					mtk_clks_source_name[i]);
				return -EINVAL;
			}
			eth->clks[i] = NULL;
		}
	}

	eth->msg_enable = netif_msg_init(mtk_msg_level, MTK_DEFAULT_MSG_ENABLE);
	INIT_WORK(&eth->pending_work, mtk_pending_work);

	err = mtk_hw_init(eth, MTK_TYPE_COLD_RESET);
	if (err)
		return err;

	eth->hwlro = MTK_HAS_CAPS(eth->soc->caps, MTK_HWLRO);

	for_each_child_of_node(pdev->dev.of_node, mac_np) {
		if (!of_device_is_compatible(mac_np,
					     "mediatek,eth-mac"))
			continue;

		if (!of_device_is_available(mac_np))
			continue;

		err = mtk_add_mac(eth, mac_np);
		if (err) {
			of_node_put(mac_np);
			goto err_deinit_hw;
		}
	}

	err = mtk_napi_init(eth);
	if (err)
		goto err_free_dev;

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_SHARED_INT)) {
		err = devm_request_irq(eth->dev, eth->irq[0],
				       mtk_handle_irq, 0,
				       dev_name(eth->dev), eth);
	} else {
		err = devm_request_irq(eth->dev, eth->irq[1],
				       mtk_handle_irq_tx, 0,
				       dev_name(eth->dev), eth);
		if (err)
			goto err_free_dev;

		err = devm_request_irq(eth->dev, eth->irq[2],
				       mtk_handle_irq_rx, 0,
				       dev_name(eth->dev), &eth->rx_napi[0]);
		if (err)
			goto err_free_dev;

		if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
			for (i = 1; i < MTK_RX_NAPI_NUM; i++) {
				err = devm_request_irq(eth->dev,
						eth->irq[2 + i],
						mtk_handle_irq_rx, 0,
						dev_name(eth->dev),
						&eth->rx_napi[i]);
				if (err)
					goto err_free_dev;
			}
		} else {
			err = devm_request_irq(eth->dev, eth->irq[3],
					       mtk_handle_fe_irq, 0,
					       dev_name(eth->dev), eth);
			if (err)
				goto err_free_dev;
		}
	}

	if (err)
		goto err_free_dev;

	/* No MT7628/88 support yet */
	if (!MTK_HAS_CAPS(eth->soc->caps, MTK_SOC_MT7628)) {
		err = mtk_mdio_init(eth);
		if (err)
			goto err_free_dev;
	}

	for (i = 0; i < MTK_MAX_DEVS; i++) {
		if (!eth->netdev[i])
			continue;

		err = register_netdev(eth->netdev[i]);
		if (err) {
			dev_err(eth->dev, "error bringing up device\n");
			goto err_deinit_mdio;
		} else
			netif_info(eth, probe, eth->netdev[i],
				   "mediatek frame engine at 0x%08lx, irq %d\n",
				   eth->netdev[i]->base_addr, eth->irq[0]);
	}

	/* we run 2 devices on the same DMA ring so we need a dummy device
	 * for NAPI to work
	 */
	init_dummy_netdev(&eth->dummy_dev);
	netif_napi_add(&eth->dummy_dev, &eth->tx_napi, mtk_napi_tx,
		       MTK_NAPI_WEIGHT);
	netif_napi_add(&eth->dummy_dev, &eth->rx_napi[0].napi, mtk_napi_rx,
		       MTK_NAPI_WEIGHT);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		for (i = 1; i < MTK_RX_NAPI_NUM; i++)
			netif_napi_add(&eth->dummy_dev, &eth->rx_napi[i].napi,
				       mtk_napi_rx, MTK_NAPI_WEIGHT);
	}

	mtketh_debugfs_init(eth);
	debug_proc_init(eth);

	platform_set_drvdata(pdev, eth);

	register_netdevice_notifier(&mtk_eth_netdevice_nb);
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	timer_setup(&eth->mtk_dma_monitor_timer, mtk_dma_monitor, 0);
	eth->mtk_dma_monitor_timer.expires = jiffies;
	add_timer(&eth->mtk_dma_monitor_timer);
#endif

	return 0;

err_deinit_mdio:
	mtk_mdio_cleanup(eth);
err_free_dev:
	mtk_free_dev(eth);
err_deinit_hw:
	mtk_hw_deinit(eth);

	return err;
}

static int mtk_remove(struct platform_device *pdev)
{
	struct mtk_eth *eth = platform_get_drvdata(pdev);
	struct mtk_mac *mac;
	int i;

	/* stop all devices to make sure that dma is properly shut down */
	for (i = 0; i < MTK_MAC_COUNT; i++) {
		if (!eth->netdev[i])
			continue;
		mtk_stop(eth->netdev[i]);
		mac = netdev_priv(eth->netdev[i]);
		phylink_disconnect_phy(mac->phylink);
	}

	mtk_hw_deinit(eth);

	netif_napi_del(&eth->tx_napi);
	netif_napi_del(&eth->rx_napi[0].napi);

	if (MTK_HAS_CAPS(eth->soc->caps, MTK_RSS)) {
		for (i = 1; i < MTK_RX_NAPI_NUM; i++)
			netif_napi_del(&eth->rx_napi[i].napi);
	}

	mtk_cleanup(eth);
	mtk_mdio_cleanup(eth);
	unregister_netdevice_notifier(&mtk_eth_netdevice_nb);
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	del_timer_sync(&eth->mtk_dma_monitor_timer);
#endif

	return 0;
}

static const struct mtk_soc_data mt2701_data = {
	.caps = MT7623_CAPS | MTK_HWLRO,
	.hw_features = MTK_HW_FEATURES,
	.required_clks = MT7623_CLKS_BITMAP,
	.required_pctl = true,
	.has_sram = false,
};

static const struct mtk_soc_data mt7621_data = {
	.caps = MT7621_CAPS,
	.hw_features = MTK_HW_FEATURES,
	.required_clks = MT7621_CLKS_BITMAP,
	.required_pctl = false,
	.has_sram = false,
};

static const struct mtk_soc_data mt7622_data = {
	.ana_rgc3 = 0x2028,
	.caps = MT7622_CAPS | MTK_HWLRO,
	.hw_features = MTK_HW_FEATURES,
	.required_clks = MT7622_CLKS_BITMAP,
	.required_pctl = false,
	.has_sram = false,
};

static const struct mtk_soc_data mt7623_data = {
	.caps = MT7623_CAPS | MTK_HWLRO,
	.hw_features = MTK_HW_FEATURES,
	.required_clks = MT7623_CLKS_BITMAP,
	.required_pctl = true,
	.has_sram = false,
};

static const struct mtk_soc_data mt7629_data = {
	.ana_rgc3 = 0x128,
	.caps = MT7629_CAPS | MTK_HWLRO,
	.hw_features = MTK_HW_FEATURES,
	.required_clks = MT7629_CLKS_BITMAP,
	.required_pctl = false,
	.has_sram = false,
};

static const struct mtk_soc_data rt5350_data = {
	.caps = MT7628_CAPS,
	.hw_features = MTK_HW_FEATURES_MT7628,
	.required_clks = MT7628_CLKS_BITMAP,
	.required_pctl = false,
	.has_sram = false,
};

const struct of_device_id of_mtk_match[] = {
	{ .compatible = "mediatek,mt2701-eth", .data = &mt2701_data},
	{ .compatible = "mediatek,mt7621-eth", .data = &mt7621_data},
	{ .compatible = "mediatek,mt7622-eth", .data = &mt7622_data},
	{ .compatible = "mediatek,mt7623-eth", .data = &mt7623_data},
	{ .compatible = "mediatek,mt7629-eth", .data = &mt7629_data},
	{ .compatible = "ralink,rt5350-eth", .data = &rt5350_data},
	{},
};
MODULE_DEVICE_TABLE(of, of_mtk_match);

static struct platform_driver mtk_driver = {
	.probe = mtk_probe,
	.remove = mtk_remove,
	.driver = {
		.name = "mtk_soc_eth",
		.of_match_table = of_mtk_match,
	},
};

module_platform_driver(mtk_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("John Crispin <blogic@openwrt.org>");
MODULE_DESCRIPTION("Ethernet driver for MediaTek SoC");
