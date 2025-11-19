// SPDX-License-Identifier: GPL-2.0-only
/*
 * Airoha AN8855 DSA Switch driver
 * Copyright (C) 2023 Min Yao <min.yao@airoha.com>
 * Copyright (C) 2024 Christian Marangi <ansuelsmth@gmail.com>
 */
#include <linux/bitfield.h>
#include <linux/ethtool.h>
#include <linux/etherdevice.h>
#include <linux/gpio/consumer.h>
#include <linux/if_bridge.h>
#include <linux/iopoll.h>
#include <linux/netdevice.h>
#include <linux/of_net.h>
#include <linux/of_platform.h>
#include <linux/phylink.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <net/dsa.h>

#include "an8855.h"

static const struct an8855_mib_desc an8855_mib[] = {
	MIB_DESC(1, AN8855_PORT_MIB_TX_DROP, "TxDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_TX_CRC_ERR, "TxCrcErr"),
	MIB_DESC(1, AN8855_PORT_MIB_TX_COLLISION, "TxCollision"),
	MIB_DESC(1, AN8855_PORT_MIB_TX_OVERSIZE_DROP, "TxOversizeDrop"),
	MIB_DESC(2, AN8855_PORT_MIB_TX_BAD_PKT_BYTES, "TxBadPktBytes"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_DROP, "RxDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_FILTERING, "RxFiltering"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_CRC_ERR, "RxCrcErr"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_CTRL_DROP, "RxCtrlDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_INGRESS_DROP, "RxIngressDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_RX_ARL_DROP, "RxArlDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_FLOW_CONTROL_DROP, "FlowControlDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_WRED_DROP, "WredDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_MIRROR_DROP, "MirrorDrop"),
	MIB_DESC(2, AN8855_PORT_MIB_RX_BAD_PKT_BYTES, "RxBadPktBytes"),
	MIB_DESC(1, AN8855_PORT_MIB_RXS_FLOW_SAMPLING_PKT_DROP, "RxsFlowSamplingPktDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_RXS_FLOW_TOTAL_PKT_DROP, "RxsFlowTotalPktDrop"),
	MIB_DESC(1, AN8855_PORT_MIB_PORT_CONTROL_DROP, "PortControlDrop"),
};

static int
an8855_mib_init(struct an8855_priv *priv)
{
	int ret;

	ret = regmap_write(priv->regmap, AN8855_MIB_CCR,
			   AN8855_CCR_MIB_ENABLE);
	if (ret)
		return ret;

	return regmap_write(priv->regmap, AN8855_MIB_CCR,
			    AN8855_CCR_MIB_ACTIVATE);
}

static void an8855_fdb_write(struct an8855_priv *priv, u16 vid,
			     u8 port_mask, const u8 *mac,
			     bool add) __must_hold(&priv->reg_mutex)
{
	u32 mac_reg[2] = { };
	u32 reg;

	mac_reg[0] |= FIELD_PREP(AN8855_ATA1_MAC0, mac[0]);
	mac_reg[0] |= FIELD_PREP(AN8855_ATA1_MAC1, mac[1]);
	mac_reg[0] |= FIELD_PREP(AN8855_ATA1_MAC2, mac[2]);
	mac_reg[0] |= FIELD_PREP(AN8855_ATA1_MAC3, mac[3]);
	mac_reg[1] |= FIELD_PREP(AN8855_ATA2_MAC4, mac[4]);
	mac_reg[1] |= FIELD_PREP(AN8855_ATA2_MAC5, mac[5]);

	regmap_bulk_write(priv->regmap, AN8855_ATA1, mac_reg,
			  ARRAY_SIZE(mac_reg));

	reg = AN8855_ATWD_IVL;
	if (add)
		reg |= AN8855_ATWD_VLD;
	reg |= FIELD_PREP(AN8855_ATWD_VID, vid);
	reg |= FIELD_PREP(AN8855_ATWD_FID, AN8855_FID_BRIDGED);
	regmap_write(priv->regmap, AN8855_ATWD, reg);
	regmap_write(priv->regmap, AN8855_ATWD2,
		     FIELD_PREP(AN8855_ATWD2_PORT, port_mask));
}

static void an8855_fdb_read(struct an8855_priv *priv, struct an8855_fdb *fdb)
{
	u32 reg[4];

	regmap_bulk_read(priv->regmap, AN8855_ATRD0, reg,
			 ARRAY_SIZE(reg));

	fdb->live = FIELD_GET(AN8855_ATRD0_LIVE, reg[0]);
	fdb->type = FIELD_GET(AN8855_ATRD0_TYPE, reg[0]);
	fdb->ivl = FIELD_GET(AN8855_ATRD0_IVL, reg[0]);
	fdb->vid = FIELD_GET(AN8855_ATRD0_VID, reg[0]);
	fdb->fid = FIELD_GET(AN8855_ATRD0_FID, reg[0]);
	fdb->aging = FIELD_GET(AN8855_ATRD1_AGING, reg[1]);
	fdb->port_mask = FIELD_GET(AN8855_ATRD3_PORTMASK, reg[3]);
	fdb->mac[0] = FIELD_GET(AN8855_ATRD2_MAC0, reg[2]);
	fdb->mac[1] = FIELD_GET(AN8855_ATRD2_MAC1, reg[2]);
	fdb->mac[2] = FIELD_GET(AN8855_ATRD2_MAC2, reg[2]);
	fdb->mac[3] = FIELD_GET(AN8855_ATRD2_MAC3, reg[2]);
	fdb->mac[4] = FIELD_GET(AN8855_ATRD1_MAC4, reg[1]);
	fdb->mac[5] = FIELD_GET(AN8855_ATRD1_MAC5, reg[1]);
	fdb->noarp = !!FIELD_GET(AN8855_ATRD0_ARP, reg[0]);
}

static int an8855_fdb_cmd(struct an8855_priv *priv, u32 cmd,
			  u32 *rsp) __must_hold(&priv->reg_mutex)
{
	u32 val;
	int ret;

	/* Set the command operating upon the MAC address entries */
	val = AN8855_ATC_BUSY | cmd;
	ret = regmap_write(priv->regmap, AN8855_ATC, val);
	if (ret)
		return ret;

	ret = regmap_read_poll_timeout(priv->regmap, AN8855_ATC, val,
				       !(val & AN8855_ATC_BUSY), 20, 200000);
	if (ret)
		return ret;

	if (rsp)
		*rsp = val;

	return 0;
}

static void
an8855_port_stp_state_set(struct dsa_switch *ds, int port, u8 state)
{
	struct dsa_port *dp = dsa_to_port(ds, port);
	struct an8855_priv *priv = ds->priv;
	bool learning = false;
	u32 stp_state;

	switch (state) {
	case BR_STATE_DISABLED:
		stp_state = AN8855_STP_DISABLED;
		break;
	case BR_STATE_BLOCKING:
		stp_state = AN8855_STP_BLOCKING;
		break;
	case BR_STATE_LISTENING:
		stp_state = AN8855_STP_LISTENING;
		break;
	case BR_STATE_LEARNING:
		stp_state = AN8855_STP_LEARNING;
		learning = dp->learning;
		break;
	case BR_STATE_FORWARDING:
		learning = dp->learning;
		fallthrough;
	default:
		stp_state = AN8855_STP_FORWARDING;
		break;
	}

	regmap_update_bits(priv->regmap, AN8855_SSP_P(port),
			   AN8855_FID_PST_MASK(AN8855_FID_BRIDGED),
			   AN8855_FID_PST_VAL(AN8855_FID_BRIDGED, stp_state));

	regmap_update_bits(priv->regmap, AN8855_PSC_P(port), AN8855_SA_DIS,
			   learning ? 0 : AN8855_SA_DIS);
}

static void an8855_port_fast_age(struct dsa_switch *ds, int port)
{
	struct an8855_priv *priv = ds->priv;
	int ret;

	/* Set to clean Dynamic entry */
	ret = regmap_write(priv->regmap, AN8855_ATA2, AN8855_ATA2_TYPE);
	if (ret)
		return;

	/* Set Port */
	ret = regmap_write(priv->regmap, AN8855_ATWD2,
			   FIELD_PREP(AN8855_ATWD2_PORT, BIT(port)));
	if (ret)
		return;

	/* Flush Dynamic entry at port */
	an8855_fdb_cmd(priv, AN8855_ATC_MAT(AND8855_FDB_MAT_MAC_TYPE_PORT) |
		       AN8855_FDB_FLUSH, NULL);
}

static int an8855_update_port_member(struct dsa_switch *ds, int port,
				     const struct net_device *bridge_dev,
				     bool join)
{
	struct an8855_priv *priv = ds->priv;
	bool isolated, other_isolated;
	struct dsa_port *dp;
	u32 port_mask = 0;
	int ret;

	isolated = !!(priv->port_isolated_map & BIT(port));

	dsa_switch_for_each_user_port(dp, ds) {
		if (dp->index == port)
			continue;

		if (!dsa_port_offloads_bridge_dev(dp, bridge_dev))
			continue;

		other_isolated = !!(priv->port_isolated_map & BIT(dp->index));
		port_mask |= BIT(dp->index);
		/* Add/remove this port to the portvlan mask of the other
		 * ports in the bridge
		 */
		if (join && !(isolated && other_isolated))
			ret = regmap_set_bits(priv->regmap,
					      AN8855_PORTMATRIX_P(dp->index),
					      FIELD_PREP(AN8855_USER_PORTMATRIX,
							 BIT(port)));
		else
			ret = regmap_clear_bits(priv->regmap,
						AN8855_PORTMATRIX_P(dp->index),
						FIELD_PREP(AN8855_USER_PORTMATRIX,
							   BIT(port)));
		if (ret)
			return ret;
	}

	/* Add/remove all other ports to this port's portvlan mask */
	return regmap_update_bits(priv->regmap, AN8855_PORTMATRIX_P(port),
				  AN8855_USER_PORTMATRIX,
				  join ? port_mask : ~port_mask);
}

static int an8855_port_pre_bridge_flags(struct dsa_switch *ds, int port,
					struct switchdev_brport_flags flags,
					struct netlink_ext_ack *extack)
{
	if (flags.mask & ~(BR_LEARNING | BR_FLOOD | BR_MCAST_FLOOD |
			   BR_BCAST_FLOOD | BR_ISOLATED))
		return -EINVAL;

	return 0;
}

static int an8855_port_bridge_flags(struct dsa_switch *ds, int port,
				    struct switchdev_brport_flags flags,
				    struct netlink_ext_ack *extack)
{
	struct an8855_priv *priv = ds->priv;
	int ret;

	if (flags.mask & BR_LEARNING) {
		ret = regmap_update_bits(priv->regmap, AN8855_PSC_P(port), AN8855_SA_DIS,
					 flags.val & BR_LEARNING ? 0 : AN8855_SA_DIS);
		if (ret)
			return ret;
	}

	if (flags.mask & BR_FLOOD) {
		ret = regmap_update_bits(priv->regmap, AN8855_UNUF, BIT(port),
					 flags.val & BR_FLOOD ? BIT(port) : 0);
		if (ret)
			return ret;
	}

	if (flags.mask & BR_MCAST_FLOOD) {
		ret = regmap_update_bits(priv->regmap, AN8855_UNMF, BIT(port),
					 flags.val & BR_MCAST_FLOOD ? BIT(port) : 0);
		if (ret)
			return ret;

		ret = regmap_update_bits(priv->regmap, AN8855_UNIPMF, BIT(port),
					 flags.val & BR_MCAST_FLOOD ? BIT(port) : 0);
		if (ret)
			return ret;
	}

	if (flags.mask & BR_BCAST_FLOOD) {
		ret = regmap_update_bits(priv->regmap, AN8855_BCF, BIT(port),
					 flags.val & BR_BCAST_FLOOD ? BIT(port) : 0);
		if (ret)
			return ret;
	}

	if (flags.mask & BR_ISOLATED) {
		struct dsa_port *dp = dsa_to_port(ds, port);
		struct net_device *bridge_dev = dsa_port_bridge_dev_get(dp);

		if (flags.val & BR_ISOLATED)
			priv->port_isolated_map |= BIT(port);
		else
			priv->port_isolated_map &= ~BIT(port);

		ret = an8855_update_port_member(ds, port, bridge_dev, true);
		if (ret)
			return ret;
	}

	return 0;
}

static int an8855_set_ageing_time(struct dsa_switch *ds, unsigned int msecs)
{
	struct an8855_priv *priv = ds->priv;
	u32 age_count, age_unit, val;

	/* Convert msec in AN8855_L2_AGING_MS_CONSTANT counter */
	val = msecs / AN8855_L2_AGING_MS_CONSTANT;
	/* Derive the count unit */
	age_unit = val / FIELD_MAX(AN8855_AGE_UNIT);
	/* Get the count in unit, age_unit is always incremented by 1 internally */
	age_count = val / (age_unit + 1);

	return regmap_update_bits(priv->regmap, AN8855_AAC,
				  AN8855_AGE_CNT | AN8855_AGE_UNIT,
				  FIELD_PREP(AN8855_AGE_CNT, age_count) |
				  FIELD_PREP(AN8855_AGE_UNIT, age_unit));
}

static int an8855_port_bridge_join(struct dsa_switch *ds, int port,
				   struct dsa_bridge bridge,
				   bool *tx_fwd_offload,
				   struct netlink_ext_ack *extack)
{
	struct an8855_priv *priv = ds->priv;
	int ret;

	ret = an8855_update_port_member(ds, port, bridge.dev, true);
	if (ret)
		return ret;

	/* Set to fallback mode for independent VLAN learning if in a bridge */
	return regmap_update_bits(priv->regmap, AN8855_PCR_P(port),
				  AN8855_PORT_VLAN,
				  FIELD_PREP(AN8855_PORT_VLAN,
					     AN8855_PORT_FALLBACK_MODE));
}

static void an8855_port_bridge_leave(struct dsa_switch *ds, int port,
				     struct dsa_bridge bridge)
{
	struct an8855_priv *priv = ds->priv;

	an8855_update_port_member(ds, port, bridge.dev, false);

	/* When a port is removed from the bridge, the port would be set up
	 * back to the default as is at initial boot which is a VLAN-unaware
	 * port.
	 */
	regmap_update_bits(priv->regmap, AN8855_PCR_P(port),
			   AN8855_PORT_VLAN,
			   FIELD_PREP(AN8855_PORT_VLAN,
				      AN8855_PORT_MATRIX_MODE));
}

static int an8855_port_fdb_add(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       struct dsa_db db)
{
	struct an8855_priv *priv = ds->priv;
	u8 port_mask = BIT(port);
	int ret;

	/* Set the vid to the port vlan id if no vid is set */
	if (!vid)
		vid = AN8855_PORT_VID_DEFAULT;

	mutex_lock(&priv->reg_mutex);
	an8855_fdb_write(priv, vid, port_mask, addr, true);
	ret = an8855_fdb_cmd(priv, AN8855_FDB_WRITE, NULL);
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int an8855_port_fdb_del(struct dsa_switch *ds, int port,
			       const unsigned char *addr, u16 vid,
			       struct dsa_db db)
{
	struct an8855_priv *priv = ds->priv;
	u8 port_mask = BIT(port);
	int ret;

	/* Set the vid to the port vlan id if no vid is set */
	if (!vid)
		vid = AN8855_PORT_VID_DEFAULT;

	mutex_lock(&priv->reg_mutex);
	an8855_fdb_write(priv, vid, port_mask, addr, false);
	ret = an8855_fdb_cmd(priv, AN8855_FDB_WRITE, NULL);
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int an8855_port_fdb_dump(struct dsa_switch *ds, int port,
				dsa_fdb_dump_cb_t *cb, void *data)
{
	struct an8855_priv *priv = ds->priv;
	int banks, count = 0;
	u32 rsp;
	int ret;
	int i;

	mutex_lock(&priv->reg_mutex);

	/* Load search port */
	ret = regmap_write(priv->regmap, AN8855_ATWD2,
			   FIELD_PREP(AN8855_ATWD2_PORT, BIT(port)));
	if (ret)
		goto exit;
	ret = an8855_fdb_cmd(priv, AN8855_ATC_MAT(AND8855_FDB_MAT_MAC_PORT) |
			     AN8855_FDB_START, &rsp);
	if (ret < 0)
		goto exit;

	do {
		/* From response get the number of banks to read, exit if 0 */
		banks = FIELD_GET(AN8855_ATC_HIT, rsp);
		if (!banks)
			break;

		/* Each banks have 4 entry */
		for (i = 0; i < 4; i++) {
			struct an8855_fdb _fdb = {  };

			count++;

			/* Check if bank is present */
			if (!(banks & BIT(i)))
				continue;

			/* Select bank entry index */
			ret = regmap_write(priv->regmap, AN8855_ATRDS,
					   FIELD_PREP(AN8855_ATRD_SEL, i));
			if (ret)
				break;
			/* wait 1ms for the bank entry to be filled */
			usleep_range(1000, 1500);
			an8855_fdb_read(priv, &_fdb);

			if (!_fdb.live)
				continue;
			ret = cb(_fdb.mac, _fdb.vid, _fdb.noarp, data);
			if (ret < 0)
				break;
		}

		/* Stop if reached max FDB number */
		if (count >= AN8855_NUM_FDB_RECORDS)
			break;

		/* Read next bank */
		ret = an8855_fdb_cmd(priv, AN8855_ATC_MAT(AND8855_FDB_MAT_MAC_PORT) |
				     AN8855_FDB_NEXT, &rsp);
		if (ret < 0)
			break;
	} while (true);

exit:
	mutex_unlock(&priv->reg_mutex);
	return ret;
}

static int an8855_vlan_cmd(struct an8855_priv *priv, enum an8855_vlan_cmd cmd,
			   u16 vid) __must_hold(&priv->reg_mutex)
{
	u32 val;
	int ret;

	val = AN8855_VTCR_BUSY | FIELD_PREP(AN8855_VTCR_FUNC, cmd) |
	      FIELD_PREP(AN8855_VTCR_VID, vid);
	ret = regmap_write(priv->regmap, AN8855_VTCR, val);
	if (ret)
		return ret;

	return regmap_read_poll_timeout(priv->regmap, AN8855_VTCR, val,
					!(val & AN8855_VTCR_BUSY), 20, 200000);
}

static int an8855_vlan_add(struct an8855_priv *priv, u8 port, u16 vid,
			   bool untagged) __must_hold(&priv->reg_mutex)
{
	u32 port_mask;
	u32 val;
	int ret;

	/* Fetch entry */
	ret = an8855_vlan_cmd(priv, AN8855_VTCR_RD_VID, vid);
	if (ret)
		return ret;

	ret = regmap_read(priv->regmap, AN8855_VARD0, &val);
	if (ret)
		return ret;
	port_mask = FIELD_GET(AN8855_VA0_PORT, val) | BIT(port);

	/* Validate the entry with independent learning, create egress tag per
	 * VLAN and joining the port as one of the port members.
	 */
	val = (val & AN8855_VA0_ETAG) | AN8855_VA0_IVL_MAC |
	      AN8855_VA0_VTAG_EN | AN8855_VA0_VLAN_VALID |
	      FIELD_PREP(AN8855_VA0_PORT, port_mask) |
	      FIELD_PREP(AN8855_VA0_FID, AN8855_FID_BRIDGED);
	ret = regmap_write(priv->regmap, AN8855_VAWD0, val);
	if (ret)
		return ret;
	ret = regmap_write(priv->regmap, AN8855_VAWD1, 0);
	if (ret)
		return ret;

	/* CPU port is always taken as a tagged port for serving more than one
	 * VLANs across and also being applied with egress type stack mode for
	 * that VLAN tags would be appended after hardware special tag used as
	 * DSA tag.
	 */
	if (port == AN8855_CPU_PORT)
		val = AN8855_VLAN_EGRESS_STACK;
	/* Decide whether adding tag or not for those outgoing packets from the
	 * port inside the VLAN.
	 */
	else
		val = untagged ? AN8855_VLAN_EGRESS_UNTAG : AN8855_VLAN_EGRESS_TAG;
	ret = regmap_update_bits(priv->regmap, AN8855_VAWD0,
				 AN8855_VA0_ETAG_PORT_MASK(port),
				 AN8855_VA0_ETAG_PORT_VAL(port, val));
	if (ret)
		return ret;

	/* Flush result to hardware */
	return an8855_vlan_cmd(priv, AN8855_VTCR_WR_VID, vid);
}

static int an8855_vlan_del(struct an8855_priv *priv, u8 port,
			   u16 vid) __must_hold(&priv->reg_mutex)
{
	u32 port_mask;
	u32 val;
	int ret;

	/* Fetch entry */
	ret = an8855_vlan_cmd(priv, AN8855_VTCR_RD_VID, vid);
	if (ret)
		return ret;

	ret = regmap_read(priv->regmap, AN8855_VARD0, &val);
	if (ret)
		return ret;
	port_mask = FIELD_GET(AN8855_VA0_PORT, val) & ~BIT(port);

	if (!(val & AN8855_VA0_VLAN_VALID)) {
		dev_err(priv->dev, "Cannot be deleted due to invalid entry\n");
		return -EINVAL;
	}

	if (port_mask) {
		val = (val & AN8855_VA0_ETAG) | AN8855_VA0_IVL_MAC |
		       AN8855_VA0_VTAG_EN | AN8855_VA0_VLAN_VALID |
		       FIELD_PREP(AN8855_VA0_PORT, port_mask);
		ret = regmap_write(priv->regmap, AN8855_VAWD0, val);
		if (ret)
			return ret;
	} else {
		ret = regmap_write(priv->regmap, AN8855_VAWD0, 0);
		if (ret)
			return ret;
	}
	ret = regmap_write(priv->regmap, AN8855_VAWD1, 0);
	if (ret)
		return ret;

	/* Flush result to hardware */
	return an8855_vlan_cmd(priv, AN8855_VTCR_WR_VID, vid);
}

static int an8855_port_set_vlan_mode(struct an8855_priv *priv, int port,
				     enum an8855_port_mode port_mode,
				     enum an8855_vlan_port_eg_tag eg_tag,
				     enum an8855_vlan_port_attr vlan_attr,
				     enum an8855_vlan_port_acc_frm acc_frm)
{
	int ret;

	ret = regmap_update_bits(priv->regmap, AN8855_PCR_P(port),
				 AN8855_PORT_VLAN,
				 FIELD_PREP(AN8855_PORT_VLAN, port_mode));
	if (ret)
		return ret;

	return regmap_update_bits(priv->regmap, AN8855_PVC_P(port),
				  AN8855_PVC_EG_TAG | AN8855_VLAN_ATTR | AN8855_ACC_FRM,
				  FIELD_PREP(AN8855_PVC_EG_TAG, eg_tag) |
				  FIELD_PREP(AN8855_VLAN_ATTR, vlan_attr) |
				  FIELD_PREP(AN8855_ACC_FRM, acc_frm));
}

static int an8855_port_set_pid(struct an8855_priv *priv, int port,
			       u16 pid)
{
	int ret;

	ret = regmap_update_bits(priv->regmap, AN8855_PPBV1_P(port),
				 AN8855_PPBV_G0_PORT_VID,
				 FIELD_PREP(AN8855_PPBV_G0_PORT_VID, pid));
	if (ret)
		return ret;

	return regmap_update_bits(priv->regmap, AN8855_PVID_P(port),
				  AN8855_G0_PORT_VID,
				  FIELD_PREP(AN8855_G0_PORT_VID, pid));
}

static int an8855_port_vlan_filtering(struct dsa_switch *ds, int port,
				      bool vlan_filtering,
				      struct netlink_ext_ack *extack)
{
	struct an8855_priv *priv = ds->priv;
	u32 val;
	int ret;

	/* The port is being kept as VLAN-unaware port when bridge is
	 * set up with vlan_filtering not being set, Otherwise, the
	 * port and the corresponding CPU port is required the setup
	 * for becoming a VLAN-aware port.
	 */
	if (vlan_filtering) {
		u32 acc_frm;
		/* CPU port is set to fallback mode to let untagged
		 * frames pass through.
		 */
		ret = an8855_port_set_vlan_mode(priv, AN8855_CPU_PORT,
						AN8855_PORT_FALLBACK_MODE,
						AN8855_VLAN_EG_CONSISTENT,
						AN8855_VLAN_USER,
						AN8855_VLAN_ACC_ALL);
		if (ret)
			return ret;

		ret = regmap_read(priv->regmap, AN8855_PVID_P(port), &val);
		if (ret)
			return ret;

		/* Only accept tagged frames if PVID is not set */
		if (FIELD_GET(AN8855_G0_PORT_VID, val) != AN8855_PORT_VID_DEFAULT)
			acc_frm = AN8855_VLAN_ACC_TAGGED;
		else
			acc_frm = AN8855_VLAN_ACC_ALL;

		/* Trapped into security mode allows packet forwarding through VLAN
		 * table lookup.
		 * Set the port as a user port which is to be able to recognize VID
		 * from incoming packets before fetching entry within the VLAN table.
		 */
		ret = an8855_port_set_vlan_mode(priv, port,
						AN8855_PORT_SECURITY_MODE,
						AN8855_VLAN_EG_DISABLED,
						AN8855_VLAN_USER,
						acc_frm);
		if (ret)
			return ret;
	} else {
		bool disable_cpu_vlan = true;
		struct dsa_port *dp;
		u32 port_mode;

		/* This is called after .port_bridge_leave when leaving a VLAN-aware
		 * bridge. Don't set standalone ports to fallback mode.
		 */
		if (dsa_port_bridge_dev_get(dsa_to_port(ds, port)))
			port_mode = AN8855_PORT_FALLBACK_MODE;
		else
			port_mode = AN8855_PORT_MATRIX_MODE;

		/* When a port is removed from the bridge, the port would be set up
		 * back to the default as is at initial boot which is a VLAN-unaware
		 * port.
		 */
		ret = an8855_port_set_vlan_mode(priv, port, port_mode,
						AN8855_VLAN_EG_CONSISTENT,
						AN8855_VLAN_TRANSPARENT,
						AN8855_VLAN_ACC_ALL);
		if (ret)
			return ret;

		/* Restore default PVID */
		ret = an8855_port_set_pid(priv, port, AN8855_PORT_VID_DEFAULT);
		if (ret)
			return ret;

		dsa_switch_for_each_user_port(dp, ds) {
			if (dsa_port_is_vlan_filtering(dp)) {
				disable_cpu_vlan = false;
				break;
			}
		}

		if (disable_cpu_vlan) {
			ret = an8855_port_set_vlan_mode(priv, AN8855_CPU_PORT,
							AN8855_PORT_MATRIX_MODE,
							AN8855_VLAN_EG_CONSISTENT,
							AN8855_VLAN_USER,
							AN8855_VLAN_ACC_ALL);
			if (ret)
				return ret;
		}
	}

	return 0;
}

static int an8855_port_vlan_add(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_vlan *vlan,
				struct netlink_ext_ack *extack)
{
	bool untagged = vlan->flags & BRIDGE_VLAN_INFO_UNTAGGED;
	bool pvid = vlan->flags & BRIDGE_VLAN_INFO_PVID;
	struct an8855_priv *priv = ds->priv;
	u32 val;
	int ret;

	mutex_lock(&priv->reg_mutex);
	ret = an8855_vlan_add(priv, port, vlan->vid, untagged);
	mutex_unlock(&priv->reg_mutex);
	if (ret)
		return ret;

	if (pvid) {
		/* Accept all frames if PVID is set */
		regmap_update_bits(priv->regmap, AN8855_PVC_P(port), AN8855_ACC_FRM,
				   FIELD_PREP(AN8855_ACC_FRM, AN8855_VLAN_ACC_ALL));

		/* Only configure PVID if VLAN filtering is enabled */
		if (dsa_port_is_vlan_filtering(dsa_to_port(ds, port))) {
			ret = an8855_port_set_pid(priv, port, vlan->vid);
			if (ret)
				return ret;
		}
	} else if (vlan->vid) {
		ret = regmap_read(priv->regmap, AN8855_PVID_P(port), &val);
		if (ret)
			return ret;

		if (FIELD_GET(AN8855_G0_PORT_VID, val) != vlan->vid)
			return 0;

		/* This VLAN is overwritten without PVID, so unset it */
		if (dsa_port_is_vlan_filtering(dsa_to_port(ds, port))) {
			ret = regmap_update_bits(priv->regmap, AN8855_PVC_P(port),
						 AN8855_ACC_FRM,
						 FIELD_PREP(AN8855_ACC_FRM,
							    AN8855_VLAN_ACC_TAGGED));
			if (ret)
				return ret;
		}

		ret = an8855_port_set_pid(priv, port, AN8855_PORT_VID_DEFAULT);
		if (ret)
			return ret;
	}

	return 0;
}

static int an8855_port_vlan_del(struct dsa_switch *ds, int port,
				const struct switchdev_obj_port_vlan *vlan)
{
	struct an8855_priv *priv = ds->priv;
	u32 val;
	int ret;

	mutex_lock(&priv->reg_mutex);
	ret = an8855_vlan_del(priv, port, vlan->vid);
	mutex_unlock(&priv->reg_mutex);
	if (ret)
		return ret;

	ret = regmap_read(priv->regmap, AN8855_PVID_P(port), &val);
	if (ret)
		return ret;

	/* PVID is being restored to the default whenever the PVID port
	 * is being removed from the VLAN.
	 */
	if (FIELD_GET(AN8855_G0_PORT_VID, val) == vlan->vid) {
		/* Only accept tagged frames if the port is VLAN-aware */
		if (dsa_port_is_vlan_filtering(dsa_to_port(ds, port))) {
			ret = regmap_update_bits(priv->regmap, AN8855_PVC_P(port),
						 AN8855_ACC_FRM,
						 FIELD_PREP(AN8855_ACC_FRM,
							    AN8855_VLAN_ACC_TAGGED));
			if (ret)
				return ret;
		}

		ret = an8855_port_set_pid(priv, port, AN8855_PORT_VID_DEFAULT);
		if (ret)
			return ret;
	}

	return 0;
}

static int
an8855_port_mdb_add(struct dsa_switch *ds, int port,
		    const struct switchdev_obj_port_mdb *mdb,
		    struct dsa_db db)
{
	struct an8855_priv *priv = ds->priv;
	const u8 *addr = mdb->addr;
	u16 vid = mdb->vid;
	u8 port_mask = 0;
	u32 val;
	int ret;

	/* Set the vid to the port vlan id if no vid is set */
	if (!vid)
		vid = AN8855_PORT_VID_DEFAULT;

	mutex_lock(&priv->reg_mutex);

	an8855_fdb_write(priv, vid, 0, addr, false);
	if (!an8855_fdb_cmd(priv, AN8855_FDB_READ, NULL)) {
		ret = regmap_read(priv->regmap, AN8855_ATRD3, &val);
		if (ret)
			goto exit;

		port_mask = FIELD_GET(AN8855_ATRD3_PORTMASK, val);
	}

	port_mask |= BIT(port);
	an8855_fdb_write(priv, vid, port_mask, addr, true);
	ret = an8855_fdb_cmd(priv, AN8855_FDB_WRITE, NULL);

exit:
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int
an8855_port_mdb_del(struct dsa_switch *ds, int port,
		    const struct switchdev_obj_port_mdb *mdb,
		    struct dsa_db db)
{
	struct an8855_priv *priv = ds->priv;
	const u8 *addr = mdb->addr;
	u16 vid = mdb->vid;
	u8 port_mask = 0;
	u32 val;
	int ret;

	/* Set the vid to the port vlan id if no vid is set */
	if (!vid)
		vid = AN8855_PORT_VID_DEFAULT;

	mutex_lock(&priv->reg_mutex);

	an8855_fdb_write(priv, vid, 0, addr, 0);
	if (!an8855_fdb_cmd(priv, AN8855_FDB_READ, NULL)) {
		ret = regmap_read(priv->regmap, AN8855_ATRD3, &val);
		if (ret)
			goto exit;

		port_mask = FIELD_GET(AN8855_ATRD3_PORTMASK, val);
	}

	port_mask &= ~BIT(port);
	an8855_fdb_write(priv, vid, port_mask, addr, port_mask ? true : false);
	ret = an8855_fdb_cmd(priv, AN8855_FDB_WRITE, NULL);

exit:
	mutex_unlock(&priv->reg_mutex);

	return ret;
}

static int
an8855_port_change_mtu(struct dsa_switch *ds, int port, int new_mtu)
{
	struct an8855_priv *priv = ds->priv;
	int length;
	u32 val;

	/* When a new MTU is set, DSA always set the CPU port's MTU to the
	 * largest MTU of the slave ports. Because the switch only has a global
	 * RX length register, only allowing CPU port here is enough.
	 */
	if (!dsa_is_cpu_port(ds, port))
		return 0;

	/* RX length also includes Ethernet header, MTK tag, and FCS length */
	length = new_mtu + ETH_HLEN + MTK_TAG_LEN + ETH_FCS_LEN;
	if (length <= 1522)
		val = AN8855_MAX_RX_PKT_1518_1522;
	else if (length <= 1536)
		val = AN8855_MAX_RX_PKT_1536;
	else if (length <= 1552)
		val = AN8855_MAX_RX_PKT_1552;
	else if (length <= 3072)
		val = AN8855_MAX_RX_JUMBO_3K;
	else if (length <= 4096)
		val = AN8855_MAX_RX_JUMBO_4K;
	else if (length <= 5120)
		val = AN8855_MAX_RX_JUMBO_5K;
	else if (length <= 6144)
		val = AN8855_MAX_RX_JUMBO_6K;
	else if (length <= 7168)
		val = AN8855_MAX_RX_JUMBO_7K;
	else if (length <= 8192)
		val = AN8855_MAX_RX_JUMBO_8K;
	else if (length <= 9216)
		val = AN8855_MAX_RX_JUMBO_9K;
	else if (length <= 12288)
		val = AN8855_MAX_RX_JUMBO_12K;
	else if (length <= 15360)
		val = AN8855_MAX_RX_JUMBO_15K;
	else
		val = AN8855_MAX_RX_JUMBO_16K;

	/* Enable JUMBO packet */
	if (length > 1552)
		val |= AN8855_MAX_RX_PKT_JUMBO;

	return regmap_update_bits(priv->regmap, AN8855_GMACCR,
				  AN8855_MAX_RX_JUMBO | AN8855_MAX_RX_PKT_LEN,
				  val);
}

static int
an8855_port_max_mtu(struct dsa_switch *ds, int port)
{
	return AN8855_MAX_MTU;
}

static void
an8855_get_strings(struct dsa_switch *ds, int port, u32 stringset,
		   uint8_t *data)
{
	int i;

	if (stringset != ETH_SS_STATS)
		return;

	for (i = 0; i < ARRAY_SIZE(an8855_mib); i++)
		ethtool_puts(&data, an8855_mib[i].name);
}

static void
an8855_read_port_stats(struct an8855_priv *priv, int port, u32 offset, u8 size,
		       uint64_t *data)
{
	u32 val, reg = AN8855_PORT_MIB_COUNTER(port) + offset;

	regmap_read(priv->regmap, reg, &val);
	*data = val;

	if (size == 2) {
		regmap_read(priv->regmap, reg + 4, &val);
		*data |= (u64)val << 32;
	}
}

static void
an8855_get_ethtool_stats(struct dsa_switch *ds, int port, uint64_t *data)
{
	struct an8855_priv *priv = ds->priv;
	const struct an8855_mib_desc *mib;
	int i;

	for (i = 0; i < ARRAY_SIZE(an8855_mib); i++) {
		mib = &an8855_mib[i];

		an8855_read_port_stats(priv, port, mib->offset, mib->size,
				       data + i);
	}
}

static int
an8855_get_sset_count(struct dsa_switch *ds, int port, int sset)
{
	if (sset != ETH_SS_STATS)
		return 0;

	return ARRAY_SIZE(an8855_mib);
}

static void
an8855_get_eth_mac_stats(struct dsa_switch *ds, int port,
			 struct ethtool_eth_mac_stats *mac_stats)
{
	struct an8855_priv *priv = ds->priv;

	/* MIB counter doesn't provide a FramesTransmittedOK but instead
	 * provide stats for Unicast, Broadcast and Multicast frames separately.
	 * To simulate a global frame counter, read Unicast and addition Multicast
	 * and Broadcast later
	 */
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_UNICAST, 1,
			       &mac_stats->FramesTransmittedOK);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_SINGLE_COLLISION, 1,
			       &mac_stats->SingleCollisionFrames);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_MULTIPLE_COLLISION, 1,
			       &mac_stats->MultipleCollisionFrames);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_UNICAST, 1,
			       &mac_stats->FramesReceivedOK);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_BYTES, 2,
			       &mac_stats->OctetsTransmittedOK);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_ALIGN_ERR, 1,
			       &mac_stats->AlignmentErrors);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_DEFERRED, 1,
			       &mac_stats->FramesWithDeferredXmissions);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_LATE_COLLISION, 1,
			       &mac_stats->LateCollisions);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_EXCESSIVE_COLLISION, 1,
			       &mac_stats->FramesAbortedDueToXSColls);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_BYTES, 2,
			       &mac_stats->OctetsReceivedOK);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_MULTICAST, 1,
			       &mac_stats->MulticastFramesXmittedOK);
	mac_stats->FramesTransmittedOK += mac_stats->MulticastFramesXmittedOK;
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_BROADCAST, 1,
			       &mac_stats->BroadcastFramesXmittedOK);
	mac_stats->FramesTransmittedOK += mac_stats->BroadcastFramesXmittedOK;

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_MULTICAST, 1,
			       &mac_stats->MulticastFramesReceivedOK);
	mac_stats->FramesReceivedOK += mac_stats->MulticastFramesReceivedOK;
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_BROADCAST, 1,
			       &mac_stats->BroadcastFramesReceivedOK);
	mac_stats->FramesReceivedOK += mac_stats->BroadcastFramesReceivedOK;
}

static const struct ethtool_rmon_hist_range an8855_rmon_ranges[] = {
	{ 0, 64 },
	{ 65, 127 },
	{ 128, 255 },
	{ 256, 511 },
	{ 512, 1023 },
	{ 1024, 1518 },
	{ 1519, AN8855_MAX_MTU },
	{}
};

static void an8855_get_rmon_stats(struct dsa_switch *ds, int port,
				  struct ethtool_rmon_stats *rmon_stats,
				  const struct ethtool_rmon_hist_range **ranges)
{
	struct an8855_priv *priv = ds->priv;

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_UNDER_SIZE_ERR, 1,
			       &rmon_stats->undersize_pkts);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_OVER_SZ_ERR, 1,
			       &rmon_stats->oversize_pkts);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_FRAG_ERR, 1,
			       &rmon_stats->fragments);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_JABBER_ERR, 1,
			       &rmon_stats->jabbers);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_64, 1,
			       &rmon_stats->hist[0]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_65_TO_127, 1,
			       &rmon_stats->hist[1]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_128_TO_255, 1,
			       &rmon_stats->hist[2]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_256_TO_511, 1,
			       &rmon_stats->hist[3]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_512_TO_1023, 1,
			       &rmon_stats->hist[4]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_1024_TO_1518, 1,
			       &rmon_stats->hist[5]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PKT_SZ_1519_TO_MAX, 1,
			       &rmon_stats->hist[6]);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_64, 1,
			       &rmon_stats->hist_tx[0]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_65_TO_127, 1,
			       &rmon_stats->hist_tx[1]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_128_TO_255, 1,
			       &rmon_stats->hist_tx[2]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_256_TO_511, 1,
			       &rmon_stats->hist_tx[3]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_512_TO_1023, 1,
			       &rmon_stats->hist_tx[4]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_1024_TO_1518, 1,
			       &rmon_stats->hist_tx[5]);
	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PKT_SZ_1519_TO_MAX, 1,
			       &rmon_stats->hist_tx[6]);

	*ranges = an8855_rmon_ranges;
}

static void an8855_get_eth_ctrl_stats(struct dsa_switch *ds, int port,
				      struct ethtool_eth_ctrl_stats *ctrl_stats)
{
	struct an8855_priv *priv = ds->priv;

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_TX_PAUSE, 1,
			       &ctrl_stats->MACControlFramesTransmitted);

	an8855_read_port_stats(priv, port, AN8855_PORT_MIB_RX_PAUSE, 1,
			       &ctrl_stats->MACControlFramesReceived);
}

static int an8855_port_mirror_add(struct dsa_switch *ds, int port,
				  struct dsa_mall_mirror_tc_entry *mirror,
				  bool ingress,
				  struct netlink_ext_ack *extack)
{
	struct an8855_priv *priv = ds->priv;
	int monitor_port;
	u32 val;
	int ret;

	/* Check for existent entry */
	if ((ingress ? priv->mirror_rx : priv->mirror_tx) & BIT(port))
		return -EEXIST;

	ret = regmap_read(priv->regmap, AN8855_MIR, &val);
	if (ret)
		return ret;

	/* AN8855 supports 4 monitor port, but only use first group */
	monitor_port = FIELD_GET(AN8855_MIRROR_PORT, val);
	if (val & AN8855_MIRROR_EN && monitor_port != mirror->to_local_port)
		return -EEXIST;

	val = AN8855_MIRROR_EN;
	val |= FIELD_PREP(AN8855_MIRROR_PORT, mirror->to_local_port);
	ret = regmap_update_bits(priv->regmap, AN8855_MIR,
				 AN8855_MIRROR_EN | AN8855_MIRROR_PORT,
				 val);
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->regmap, AN8855_PCR_P(port),
			      ingress ? AN8855_PORT_RX_MIR : AN8855_PORT_TX_MIR);
	if (ret)
		return ret;

	if (ingress)
		priv->mirror_rx |= BIT(port);
	else
		priv->mirror_tx |= BIT(port);

	return 0;
}

static void an8855_port_mirror_del(struct dsa_switch *ds, int port,
				   struct dsa_mall_mirror_tc_entry *mirror)
{
	struct an8855_priv *priv = ds->priv;

	if (mirror->ingress)
		priv->mirror_rx &= ~BIT(port);
	else
		priv->mirror_tx &= ~BIT(port);

	regmap_clear_bits(priv->regmap, AN8855_PCR_P(port),
			  mirror->ingress ? AN8855_PORT_RX_MIR :
					    AN8855_PORT_TX_MIR);

	if (!priv->mirror_rx && !priv->mirror_tx)
		regmap_clear_bits(priv->regmap, AN8855_MIR, AN8855_MIRROR_EN);
}

static int an8855_port_set_status(struct an8855_priv *priv, int port,
				  bool enable)
{
	if (enable)
		return regmap_set_bits(priv->regmap, AN8855_PMCR_P(port),
				       AN8855_PMCR_TX_EN | AN8855_PMCR_RX_EN);
	else
		return regmap_clear_bits(priv->regmap, AN8855_PMCR_P(port),
					 AN8855_PMCR_TX_EN | AN8855_PMCR_RX_EN);
}

static int an8855_port_enable(struct dsa_switch *ds, int port,
			      struct phy_device *phy)
{
	return an8855_port_set_status(ds->priv, port, true);
}

static void an8855_port_disable(struct dsa_switch *ds, int port)
{
	an8855_port_set_status(ds->priv, port, false);
}

static u32 en8855_get_phy_flags(struct dsa_switch *ds, int port)
{
	struct an8855_priv *priv = ds->priv;

	/* PHY doesn't need calibration */
	if (!priv->phy_require_calib)
		return 0;

	/* Use AN8855_PHY_FLAGS_EN_CALIBRATION to signal
	 * calibration needed.
	 */
	return AN8855_PHY_FLAGS_EN_CALIBRATION;
}

static enum dsa_tag_protocol
an8855_get_tag_protocol(struct dsa_switch *ds, int port,
			enum dsa_tag_protocol mp)
{
	return DSA_TAG_PROTO_MTK;
}

/* Similar to MT7530 also trap link local frame and special frame to CPU */
static int an8855_trap_special_frames(struct an8855_priv *priv)
{
	int ret;

	/* Trap BPDUs to the CPU port(s) and egress them
	 * VLAN-untagged.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_BPC,
				 AN8855_BPDU_BPDU_FR | AN8855_BPDU_EG_TAG |
				 AN8855_BPDU_PORT_FW,
				 AN8855_BPDU_BPDU_FR |
				 FIELD_PREP(AN8855_BPDU_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				 FIELD_PREP(AN8855_BPDU_PORT_FW, AN8855_BPDU_CPU_ONLY));
	if (ret)
		return ret;

	/* Trap 802.1X PAE frames to the CPU port(s) and egress them
	 * VLAN-untagged.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_PAC,
				 AN8855_PAE_BPDU_FR | AN8855_PAE_EG_TAG |
				 AN8855_PAE_PORT_FW,
				 AN8855_PAE_BPDU_FR |
				 FIELD_PREP(AN8855_PAE_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				 FIELD_PREP(AN8855_PAE_PORT_FW, AN8855_BPDU_CPU_ONLY));
	if (ret)
		return ret;

	/* Trap frames with :01 MAC DAs to the CPU port(s) and egress
	 * them VLAN-untagged.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_RGAC1,
				 AN8855_R01_BPDU_FR | AN8855_R01_EG_TAG |
				 AN8855_R01_PORT_FW,
				 AN8855_R01_BPDU_FR |
				 FIELD_PREP(AN8855_R01_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				 FIELD_PREP(AN8855_R01_PORT_FW, AN8855_BPDU_CPU_ONLY));
	if (ret)
		return ret;

	/* Trap frames with :02 MAC DAs to the CPU port(s) and egress
	 * them VLAN-untagged.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_RGAC1,
				 AN8855_R02_BPDU_FR | AN8855_R02_EG_TAG |
				 AN8855_R02_PORT_FW,
				 AN8855_R02_BPDU_FR |
				 FIELD_PREP(AN8855_R02_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				 FIELD_PREP(AN8855_R02_PORT_FW, AN8855_BPDU_CPU_ONLY));
	if (ret)
		return ret;

	/* Trap frames with :03 MAC DAs to the CPU port(s) and egress
	 * them VLAN-untagged.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_RGAC1,
				 AN8855_R03_BPDU_FR | AN8855_R03_EG_TAG |
				 AN8855_R03_PORT_FW,
				 AN8855_R03_BPDU_FR |
				 FIELD_PREP(AN8855_R03_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				 FIELD_PREP(AN8855_R03_PORT_FW, AN8855_BPDU_CPU_ONLY));
	if (ret)
		return ret;

	/* Trap frames with :0E MAC DAs to the CPU port(s) and egress
	 * them VLAN-untagged.
	 */
	return regmap_update_bits(priv->regmap, AN8855_RGAC1,
				  AN8855_R0E_BPDU_FR | AN8855_R0E_EG_TAG |
				  AN8855_R0E_PORT_FW,
				  AN8855_R0E_BPDU_FR |
				  FIELD_PREP(AN8855_R0E_EG_TAG, AN8855_VLAN_EG_UNTAGGED) |
				  FIELD_PREP(AN8855_R0E_PORT_FW, AN8855_BPDU_CPU_ONLY));
}

static int
an8855_setup_pvid_vlan(struct an8855_priv *priv)
{
	u32 val;
	int ret;

	/* Validate the entry with independent learning, keep the original
	 * ingress tag attribute.
	 */
	val = AN8855_VA0_IVL_MAC | AN8855_VA0_EG_CON |
	      FIELD_PREP(AN8855_VA0_FID, AN8855_FID_BRIDGED) |
	      AN8855_VA0_PORT | AN8855_VA0_VLAN_VALID;
	ret = regmap_write(priv->regmap, AN8855_VAWD0, val);
	if (ret)
		return ret;

	return an8855_vlan_cmd(priv, AN8855_VTCR_WR_VID,
			       AN8855_PORT_VID_DEFAULT);
}

static int an8855_setup(struct dsa_switch *ds)
{
	struct an8855_priv *priv = ds->priv;
	struct dsa_port *dp;
	int ret;

	/* Enable and reset MIB counters */
	ret = an8855_mib_init(priv);
	if (ret)
		return ret;

	dsa_switch_for_each_user_port(dp, ds) {
		/* Disable MAC by default on all user ports */
		ret = an8855_port_set_status(priv, dp->index, false);
		if (ret)
			return ret;

		/* Individual user ports get connected to CPU port only */
		ret = regmap_write(priv->regmap, AN8855_PORTMATRIX_P(dp->index),
				   FIELD_PREP(AN8855_PORTMATRIX, BIT(AN8855_CPU_PORT)));
		if (ret)
			return ret;

		/* Disable Broadcast Forward on user ports */
		ret = regmap_clear_bits(priv->regmap, AN8855_BCF, BIT(dp->index));
		if (ret)
			return ret;

		/* Disable Unknown Unicast Forward on user ports */
		ret = regmap_clear_bits(priv->regmap, AN8855_UNUF, BIT(dp->index));
		if (ret)
			return ret;

		/* Disable Unknown Multicast Forward on user ports */
		ret = regmap_clear_bits(priv->regmap, AN8855_UNMF, BIT(dp->index));
		if (ret)
			return ret;

		ret = regmap_clear_bits(priv->regmap, AN8855_UNIPMF, BIT(dp->index));
		if (ret)
			return ret;

		/* Set default PVID to on all user ports */
		ret = an8855_port_set_pid(priv, dp->index, AN8855_PORT_VID_DEFAULT);
		if (ret)
			return ret;
	}

	/* Enable Airoha header mode on the cpu port */
	ret = regmap_write(priv->regmap, AN8855_PVC_P(AN8855_CPU_PORT),
			   AN8855_PORT_SPEC_REPLACE_MODE | AN8855_PORT_SPEC_TAG);
	if (ret)
		return ret;

	/* Unknown multicast frame forwarding to the cpu port */
	ret = regmap_write(priv->regmap, AN8855_UNMF, BIT(AN8855_CPU_PORT));
	if (ret)
		return ret;

	/* Set CPU port number */
	ret = regmap_update_bits(priv->regmap, AN8855_MFC,
				 AN8855_CPU_EN | AN8855_CPU_PORT_IDX,
				 AN8855_CPU_EN |
				 FIELD_PREP(AN8855_CPU_PORT_IDX, AN8855_CPU_PORT));
	if (ret)
		return ret;

	/* CPU port gets connected to all user ports of
	 * the switch.
	 */
	ret = regmap_write(priv->regmap, AN8855_PORTMATRIX_P(AN8855_CPU_PORT),
			   FIELD_PREP(AN8855_PORTMATRIX, dsa_user_ports(ds)));
	if (ret)
		return ret;

	/* CPU port is set to fallback mode to let untagged
	 * frames pass through.
	 */
	ret = regmap_update_bits(priv->regmap, AN8855_PCR_P(AN8855_CPU_PORT),
				 AN8855_PORT_VLAN,
				 FIELD_PREP(AN8855_PORT_VLAN, AN8855_PORT_FALLBACK_MODE));
	if (ret)
		return ret;

	/* Enable Broadcast Forward on CPU port */
	ret = regmap_set_bits(priv->regmap, AN8855_BCF, BIT(AN8855_CPU_PORT));
	if (ret)
		return ret;

	/* Enable Unknown Unicast Forward on CPU port */
	ret = regmap_set_bits(priv->regmap, AN8855_UNUF, BIT(AN8855_CPU_PORT));
	if (ret)
		return ret;

	/* Enable Unknown Multicast Forward on CPU port */
	ret = regmap_set_bits(priv->regmap, AN8855_UNMF, BIT(AN8855_CPU_PORT));
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->regmap, AN8855_UNIPMF, BIT(AN8855_CPU_PORT));
	if (ret)
		return ret;

	/* Setup Trap special frame to CPU rules */
	ret = an8855_trap_special_frames(priv);
	if (ret)
		return ret;

	dsa_switch_for_each_port(dp, ds) {
		/* Disable Learning on all ports.
		 * Learning on CPU is disabled for fdb isolation and handled by
		 * assisted_learning_on_cpu_port.
		 */
		ret = regmap_set_bits(priv->regmap, AN8855_PSC_P(dp->index),
				      AN8855_SA_DIS);
		if (ret)
			return ret;

		/* Enable consistent egress tag (for VLAN unware VLAN-passtrough) */
		ret = regmap_update_bits(priv->regmap, AN8855_PVC_P(dp->index),
					 AN8855_PVC_EG_TAG,
					 FIELD_PREP(AN8855_PVC_EG_TAG, AN8855_VLAN_EG_CONSISTENT));
		if (ret)
			return ret;
	}

	/* Setup VLAN for Default PVID */
	ret = an8855_setup_pvid_vlan(priv);
	if (ret)
		return ret;

	ret = regmap_clear_bits(priv->regmap, AN8855_CKGCR,
				AN8855_CKG_LNKDN_GLB_STOP | AN8855_CKG_LNKDN_PORT_STOP);
	if (ret)
		return ret;

	/* Release global PHY power down */
	ret = regmap_write(priv->regmap, AN8855_RG_GPHY_AFE_PWD, 0x0);
	if (ret)
		return ret;

	ds->configure_vlan_while_not_filtering = true;

	/* Flush the FDB table */
	ret = an8855_fdb_cmd(priv, AN8855_FDB_FLUSH, NULL);
	if (ret < 0)
		return ret;

	/* Set min a max ageing value supported */
	ds->ageing_time_min = AN8855_L2_AGING_MS_CONSTANT;
	ds->ageing_time_max = FIELD_MAX(AN8855_AGE_CNT) *
			      FIELD_MAX(AN8855_AGE_UNIT) *
			      AN8855_L2_AGING_MS_CONSTANT;

	/* Enable assisted learning for fdb isolation */
	ds->assisted_learning_on_cpu_port = true;

	return 0;
}

static struct phylink_pcs *an8855_phylink_mac_select_pcs(struct phylink_config *config,
							 phy_interface_t interface)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct an8855_priv *priv = dp->ds->priv;

	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
	case PHY_INTERFACE_MODE_2500BASEX:
		return &priv->pcs;
	default:
		return NULL;
	}
}

static void an8855_phylink_mac_config(struct phylink_config *config,
				      unsigned int mode,
				      const struct phylink_link_state *state)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct dsa_switch *ds = dp->ds;
	struct an8855_priv *priv;
	int port = dp->index;

	priv = ds->priv;

	/* Nothing to configure for internal ports */
	if (port != 5)
		return;

	regmap_update_bits(priv->regmap, AN8855_PMCR_P(port),
			   AN8855_PMCR_IFG_XMIT | AN8855_PMCR_MAC_MODE |
			   AN8855_PMCR_BACKOFF_EN | AN8855_PMCR_BACKPR_EN,
			   FIELD_PREP(AN8855_PMCR_IFG_XMIT, 0x1) |
			   AN8855_PMCR_MAC_MODE | AN8855_PMCR_BACKOFF_EN |
			   AN8855_PMCR_BACKPR_EN);
}

static void an8855_phylink_get_caps(struct dsa_switch *ds, int port,
				    struct phylink_config *config)
{
	switch (port) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
		__set_bit(PHY_INTERFACE_MODE_GMII,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_INTERNAL,
			  config->supported_interfaces);
		break;
	case 5:
		phy_interface_set_rgmii(config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_SGMII,
			  config->supported_interfaces);
		__set_bit(PHY_INTERFACE_MODE_2500BASEX,
			  config->supported_interfaces);
		break;
	}

	config->mac_capabilities = MAC_ASYM_PAUSE | MAC_SYM_PAUSE |
				   MAC_10 | MAC_100 | MAC_1000FD | MAC_2500FD;
}

static void an8855_phylink_mac_link_down(struct phylink_config *config,
					 unsigned int mode,
					 phy_interface_t interface)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct an8855_priv *priv = dp->ds->priv;

	/* With autoneg just disable TX/RX else also force link down */
	if (phylink_autoneg_inband(mode)) {
		regmap_clear_bits(priv->regmap, AN8855_PMCR_P(dp->index),
				  AN8855_PMCR_TX_EN | AN8855_PMCR_RX_EN);
	} else {
		regmap_update_bits(priv->regmap, AN8855_PMCR_P(dp->index),
				   AN8855_PMCR_TX_EN | AN8855_PMCR_RX_EN |
				   AN8855_PMCR_FORCE_MODE | AN8855_PMCR_FORCE_LNK,
				   AN8855_PMCR_FORCE_MODE);
	}
}

static void an8855_phylink_mac_link_up(struct phylink_config *config,
				       struct phy_device *phydev, unsigned int mode,
				       phy_interface_t interface, int speed,
				       int duplex, bool tx_pause, bool rx_pause)
{
	struct dsa_port *dp = dsa_phylink_to_port(config);
	struct an8855_priv *priv = dp->ds->priv;
	int port = dp->index;
	u32 reg;

	reg = regmap_read(priv->regmap, AN8855_PMCR_P(port), &reg);
	if (phylink_autoneg_inband(mode)) {
		reg &= ~AN8855_PMCR_FORCE_MODE;
	} else {
		reg |= AN8855_PMCR_FORCE_MODE | AN8855_PMCR_FORCE_LNK;

		reg &= ~AN8855_PMCR_FORCE_SPEED;
		switch (speed) {
		case SPEED_10:
			reg |= AN8855_PMCR_FORCE_SPEED_10;
			break;
		case SPEED_100:
			reg |= AN8855_PMCR_FORCE_SPEED_100;
			break;
		case SPEED_1000:
			reg |= AN8855_PMCR_FORCE_SPEED_1000;
			break;
		case SPEED_2500:
			reg |= AN8855_PMCR_FORCE_SPEED_2500;
			break;
		case SPEED_5000:
			dev_err(priv->dev, "Missing support for 5G speed. Aborting...\n");
			return;
		}

		reg &= ~AN8855_PMCR_FORCE_FDX;
		if (duplex == DUPLEX_FULL)
			reg |= AN8855_PMCR_FORCE_FDX;

		reg &= ~AN8855_PMCR_RX_FC_EN;
		if (rx_pause || dsa_port_is_cpu(dp))
			reg |= AN8855_PMCR_RX_FC_EN;

		reg &= ~AN8855_PMCR_TX_FC_EN;
		if (rx_pause || dsa_port_is_cpu(dp))
			reg |= AN8855_PMCR_TX_FC_EN;

		/* Disable any EEE options */
		reg &= ~(AN8855_PMCR_FORCE_EEE5G | AN8855_PMCR_FORCE_EEE2P5G |
			 AN8855_PMCR_FORCE_EEE1G | AN8855_PMCR_FORCE_EEE100);
	}

	reg |= AN8855_PMCR_TX_EN | AN8855_PMCR_RX_EN;

	regmap_write(priv->regmap, AN8855_PMCR_P(port), reg);
}

static unsigned int an8855_pcs_inband_caps(struct phylink_pcs *pcs,
					   phy_interface_t interface)
{
	/* SGMII can be configured to use inband with AN result */
	if (interface == PHY_INTERFACE_MODE_SGMII)
		return LINK_INBAND_DISABLE | LINK_INBAND_ENABLE;

	/* inband is not supported in 2500-baseX and must be disabled */
	return  LINK_INBAND_DISABLE;
}

static void an8855_pcs_get_state(struct phylink_pcs *pcs,
				 struct phylink_link_state *state)
{
	struct an8855_priv *priv = container_of(pcs, struct an8855_priv, pcs);
	u32 val;
	int ret;

	ret = regmap_read(priv->regmap, AN8855_PMSR_P(AN8855_CPU_PORT), &val);
	if (ret < 0) {
		state->link = false;
		return;
	}

	state->link = !!(val & AN8855_PMSR_LNK);
	state->an_complete = state->link;
	state->duplex = (val & AN8855_PMSR_DPX) ? DUPLEX_FULL :
						  DUPLEX_HALF;

	switch (val & AN8855_PMSR_SPEED) {
	case AN8855_PMSR_SPEED_10:
		state->speed = SPEED_10;
		break;
	case AN8855_PMSR_SPEED_100:
		state->speed = SPEED_100;
		break;
	case AN8855_PMSR_SPEED_1000:
		state->speed = SPEED_1000;
		break;
	case AN8855_PMSR_SPEED_2500:
		state->speed = SPEED_2500;
		break;
	case AN8855_PMSR_SPEED_5000:
		dev_err(priv->dev, "Missing support for 5G speed. Setting Unknown.\n");
		fallthrough;
	default:
		state->speed = SPEED_UNKNOWN;
		break;
	}

	if (val & AN8855_PMSR_RX_FC)
		state->pause |= MLO_PAUSE_RX;
	if (val & AN8855_PMSR_TX_FC)
		state->pause |= MLO_PAUSE_TX;
}

static int an8855_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			     phy_interface_t interface,
			     const unsigned long *advertising,
			     bool permit_pause_to_mac)
{
	struct an8855_priv *priv = container_of(pcs, struct an8855_priv, pcs);
	u32 val;
	int ret;

	/*                   !!! WELCOME TO HELL !!!                   */

	/* TX FIR - improve TX EYE */
	ret = regmap_update_bits(priv->regmap, AN8855_INTF_CTRL_10,
				 AN8855_RG_DA_QP_TX_FIR_C2_SEL |
				 AN8855_RG_DA_QP_TX_FIR_C2_FORCE |
				 AN8855_RG_DA_QP_TX_FIR_C1_SEL |
				 AN8855_RG_DA_QP_TX_FIR_C1_FORCE,
				 AN8855_RG_DA_QP_TX_FIR_C2_SEL |
				 FIELD_PREP(AN8855_RG_DA_QP_TX_FIR_C2_FORCE, 0x4) |
				 AN8855_RG_DA_QP_TX_FIR_C1_SEL |
				 FIELD_PREP(AN8855_RG_DA_QP_TX_FIR_C1_FORCE, 0x0));
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x0;
	else
		val = 0xd;
	ret = regmap_update_bits(priv->regmap, AN8855_INTF_CTRL_11,
				 AN8855_RG_DA_QP_TX_FIR_C0B_SEL |
				 AN8855_RG_DA_QP_TX_FIR_C0B_FORCE,
				 AN8855_RG_DA_QP_TX_FIR_C0B_SEL |
				 FIELD_PREP(AN8855_RG_DA_QP_TX_FIR_C0B_FORCE, val));
	if (ret)
		return ret;

	/* RX CDR - improve RX Jitter Tolerance */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x5;
	else
		val = 0x6;
	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_LPF_BOT_LIM,
				 AN8855_RG_QP_CDR_LPF_KP_GAIN |
				 AN8855_RG_QP_CDR_LPF_KI_GAIN,
				 FIELD_PREP(AN8855_RG_QP_CDR_LPF_KP_GAIN, val) |
				 FIELD_PREP(AN8855_RG_QP_CDR_LPF_KI_GAIN, val));
	if (ret)
		return ret;

	/* PLL */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x1;
	else
		val = 0x0;
	ret = regmap_update_bits(priv->regmap, AN8855_QP_DIG_MODE_CTRL_1,
				 AN8855_RG_TPHY_SPEED,
				 FIELD_PREP(AN8855_RG_TPHY_SPEED, val));
	if (ret)
		return ret;

	/* PLL - LPF */
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_2,
				 AN8855_RG_DA_QP_PLL_RICO_SEL_INTF |
				 AN8855_RG_DA_QP_PLL_FBKSEL_INTF |
				 AN8855_RG_DA_QP_PLL_BR_INTF |
				 AN8855_RG_DA_QP_PLL_BPD_INTF |
				 AN8855_RG_DA_QP_PLL_BPA_INTF |
				 AN8855_RG_DA_QP_PLL_BC_INTF,
				 AN8855_RG_DA_QP_PLL_RICO_SEL_INTF |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_FBKSEL_INTF, 0x0) |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_BR_INTF, 0x3) |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_BPD_INTF, 0x0) |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_BPA_INTF, 0x5) |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_BC_INTF, 0x1));
	if (ret)
		return ret;

	/* PLL - ICO */
	ret = regmap_set_bits(priv->regmap, AN8855_PLL_CTRL_4,
			      AN8855_RG_DA_QP_PLL_ICOLP_EN_INTF);
	if (ret)
		return ret;
	ret = regmap_clear_bits(priv->regmap, AN8855_PLL_CTRL_2,
				AN8855_RG_DA_QP_PLL_ICOIQ_EN_INTF);
	if (ret)
		return ret;

	/* PLL - CHP */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x6;
	else
		val = 0x4;
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_2,
				 AN8855_RG_DA_QP_PLL_IR_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_IR_INTF, val));
	if (ret)
		return ret;

	/* PLL - PFD */
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_2,
				 AN8855_RG_DA_QP_PLL_PFD_OFFSET_EN_INTRF |
				 AN8855_RG_DA_QP_PLL_PFD_OFFSET_INTF |
				 AN8855_RG_DA_QP_PLL_KBAND_PREDIV_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_PFD_OFFSET_INTF, 0x1) |
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_KBAND_PREDIV_INTF, 0x1));
	if (ret)
		return ret;

	/* PLL - POSTDIV */
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_2,
				 AN8855_RG_DA_QP_PLL_POSTDIV_EN_INTF |
				 AN8855_RG_DA_QP_PLL_PHY_CK_EN_INTF |
				 AN8855_RG_DA_QP_PLL_PCK_SEL_INTF,
				 AN8855_RG_DA_QP_PLL_PCK_SEL_INTF);
	if (ret)
		return ret;

	/* PLL - SDM */
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_2,
				 AN8855_RG_DA_QP_PLL_SDM_HREN_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_SDM_HREN_INTF, 0x0));
	if (ret)
		return ret;
	ret = regmap_clear_bits(priv->regmap, AN8855_PLL_CTRL_2,
				AN8855_RG_DA_QP_PLL_SDM_IFM_INTF);
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_SS_LCPLL_PWCTL_SETTING_2,
				 AN8855_RG_NCPO_ANA_MSB,
				 FIELD_PREP(AN8855_RG_NCPO_ANA_MSB, 0x1));
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x7a000000;
	else
		val = 0x48000000;
	ret = regmap_write(priv->regmap, AN8855_SS_LCPLL_TDC_FLT_2,
			   FIELD_PREP(AN8855_RG_LCPLL_NCPO_VALUE, val));
	if (ret)
		return ret;
	ret = regmap_write(priv->regmap, AN8855_SS_LCPLL_TDC_PCW_1,
			   FIELD_PREP(AN8855_RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON, val));
	if (ret)
		return ret;

	ret = regmap_clear_bits(priv->regmap, AN8855_SS_LCPLL_TDC_FLT_5,
				AN8855_RG_LCPLL_NCPO_CHG);
	if (ret)
		return ret;
	ret = regmap_clear_bits(priv->regmap, AN8855_PLL_CK_CTRL_0,
				AN8855_RG_DA_QP_PLL_SDM_DI_EN_INTF);
	if (ret)
		return ret;

	/* PLL - SS */
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_3,
				 AN8855_RG_DA_QP_PLL_SSC_DELTA_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_SSC_DELTA_INTF, 0x0));
	if (ret)
		return ret;
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_4,
				 AN8855_RG_DA_QP_PLL_SSC_DIR_DLY_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_SSC_DIR_DLY_INTF, 0x0));
	if (ret)
		return ret;
	ret = regmap_update_bits(priv->regmap, AN8855_PLL_CTRL_3,
				 AN8855_RG_DA_QP_PLL_SSC_PERIOD_INTF,
				 FIELD_PREP(AN8855_RG_DA_QP_PLL_SSC_PERIOD_INTF, 0x0));
	if (ret)
		return ret;

	/* PLL - TDC */
	ret = regmap_clear_bits(priv->regmap, AN8855_PLL_CK_CTRL_0,
				AN8855_RG_DA_QP_PLL_TDC_TXCK_SEL_INTF);
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->regmap, AN8855_RG_QP_PLL_SDM_ORD,
			      AN8855_RG_QP_PLL_SSC_TRI_EN);
	if (ret)
		return ret;
	ret = regmap_set_bits(priv->regmap, AN8855_RG_QP_PLL_SDM_ORD,
			      AN8855_RG_QP_PLL_SSC_PHASE_INI);
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_RX_DAC_EN,
				 AN8855_RG_QP_SIGDET_HF,
				 FIELD_PREP(AN8855_RG_QP_SIGDET_HF, 0x2));
	if (ret)
		return ret;

	/* TCL Disable (only for Co-SIM) */
	ret = regmap_clear_bits(priv->regmap, AN8855_PON_RXFEDIG_CTRL_0,
				AN8855_RG_QP_EQ_RX500M_CK_SEL);
	if (ret)
		return ret;

	/* TX Init */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x4;
	else
		val = 0x0;
	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_TX_MODE,
				 AN8855_RG_QP_TX_RESERVE |
				 AN8855_RG_QP_TX_MODE_16B_EN,
				 FIELD_PREP(AN8855_RG_QP_TX_RESERVE, val));
	if (ret)
		return ret;

	/* RX Control/Init */
	ret = regmap_set_bits(priv->regmap, AN8855_RG_QP_RXAFE_RESERVE,
			      AN8855_RG_QP_CDR_PD_10B_EN);
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x1;
	else
		val = 0x2;
	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_LPF_MJV_LIM,
				 AN8855_RG_QP_CDR_LPF_RATIO,
				 FIELD_PREP(AN8855_RG_QP_CDR_LPF_RATIO, val));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_LPF_SETVALUE,
				 AN8855_RG_QP_CDR_PR_BUF_IN_SR |
				 AN8855_RG_QP_CDR_PR_BETA_SEL,
				 FIELD_PREP(AN8855_RG_QP_CDR_PR_BUF_IN_SR, 0x6) |
				 FIELD_PREP(AN8855_RG_QP_CDR_PR_BETA_SEL, 0x1));
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0xf;
	else
		val = 0xc;
	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_PR_CKREF_DIV1,
				 AN8855_RG_QP_CDR_PR_DAC_BAND,
				 FIELD_PREP(AN8855_RG_QP_CDR_PR_DAC_BAND, val));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_PR_KBAND_DIV_PCIE,
				 AN8855_RG_QP_CDR_PR_KBAND_PCIE_MODE |
				 AN8855_RG_QP_CDR_PR_KBAND_DIV_PCIE_MASK,
				 FIELD_PREP(AN8855_RG_QP_CDR_PR_KBAND_DIV_PCIE_MASK, 0x19));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_FORCE_IBANDLPF_R_OFF,
				 AN8855_RG_QP_CDR_PHYCK_SEL |
				 AN8855_RG_QP_CDR_PHYCK_RSTB |
				 AN8855_RG_QP_CDR_PHYCK_DIV,
				 FIELD_PREP(AN8855_RG_QP_CDR_PHYCK_SEL, 0x2) |
				 FIELD_PREP(AN8855_RG_QP_CDR_PHYCK_DIV, 0x21));
	if (ret)
		return ret;

	ret = regmap_clear_bits(priv->regmap, AN8855_RG_QP_CDR_PR_KBAND_DIV_PCIE,
				AN8855_RG_QP_CDR_PR_XFICK_EN);
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RG_QP_CDR_PR_CKREF_DIV1,
				 AN8855_RG_QP_CDR_PR_KBAND_DIV,
				 FIELD_PREP(AN8855_RG_QP_CDR_PR_KBAND_DIV, 0x4));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_26,
				 AN8855_RG_QP_EQ_RETRAIN_ONLY_EN |
				 AN8855_RG_LINK_NE_EN |
				 AN8855_RG_LINK_ERRO_EN,
				 AN8855_RG_QP_EQ_RETRAIN_ONLY_EN |
				 AN8855_RG_LINK_ERRO_EN);
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_DLY_0,
				 AN8855_RG_QP_RX_SAOSC_EN_H_DLY |
				 AN8855_RG_QP_RX_PI_CAL_EN_H_DLY,
				 FIELD_PREP(AN8855_RG_QP_RX_SAOSC_EN_H_DLY, 0x3f) |
				 FIELD_PREP(AN8855_RG_QP_RX_PI_CAL_EN_H_DLY, 0x6f));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_42,
				 AN8855_RG_QP_EQ_EN_DLY,
				 FIELD_PREP(AN8855_RG_QP_EQ_EN_DLY, 0x150));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_2,
				 AN8855_RG_QP_RX_EQ_EN_H_DLY,
				 FIELD_PREP(AN8855_RG_QP_RX_EQ_EN_H_DLY, 0x150));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_PON_RXFEDIG_CTRL_9,
				 AN8855_RG_QP_EQ_LEQOSC_DLYCNT,
				 FIELD_PREP(AN8855_RG_QP_EQ_LEQOSC_DLYCNT, 0x1));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_8,
				 AN8855_RG_DA_QP_SAOSC_DONE_TIME |
				 AN8855_RG_DA_QP_LEQOS_EN_TIME,
				 FIELD_PREP(AN8855_RG_DA_QP_SAOSC_DONE_TIME, 0x200) |
				 FIELD_PREP(AN8855_RG_DA_QP_LEQOS_EN_TIME, 0xfff));
	if (ret)
		return ret;

	/* Frequency meter */
	if (interface == PHY_INTERFACE_MODE_2500BASEX)
		val = 0x10;
	else
		val = 0x28;
	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_5,
				 AN8855_RG_FREDET_CHK_CYCLE,
				 FIELD_PREP(AN8855_RG_FREDET_CHK_CYCLE, val));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_6,
				 AN8855_RG_FREDET_GOLDEN_CYCLE,
				 FIELD_PREP(AN8855_RG_FREDET_GOLDEN_CYCLE, 0x64));
	if (ret)
		return ret;

	ret = regmap_update_bits(priv->regmap, AN8855_RX_CTRL_7,
				 AN8855_RG_FREDET_TOLERATE_CYCLE,
				 FIELD_PREP(AN8855_RG_FREDET_TOLERATE_CYCLE, 0x2710));
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->regmap, AN8855_PLL_CTRL_0,
			      AN8855_RG_PHYA_AUTO_INIT);
	if (ret)
		return ret;

	/* PCS Init */
	if (interface == PHY_INTERFACE_MODE_SGMII &&
	    neg_mode == PHYLINK_PCS_NEG_INBAND_DISABLED) {
		ret = regmap_clear_bits(priv->regmap, AN8855_QP_DIG_MODE_CTRL_0,
					AN8855_RG_SGMII_MODE | AN8855_RG_SGMII_AN_EN);
		if (ret)
			return ret;
	}

	ret = regmap_clear_bits(priv->regmap, AN8855_RG_HSGMII_PCS_CTROL_1,
				AN8855_RG_TBI_10B_MODE);
	if (ret)
		return ret;

	if (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED) {
		/* Set AN Ability - Interrupt */
		ret = regmap_set_bits(priv->regmap, AN8855_SGMII_REG_AN_FORCE_CL37,
				      AN8855_RG_FORCE_AN_DONE);
		if (ret)
			return ret;

		ret = regmap_update_bits(priv->regmap, AN8855_SGMII_REG_AN_13,
					 AN8855_SGMII_REMOTE_FAULT_DIS |
					 AN8855_SGMII_IF_MODE,
					 AN8855_SGMII_REMOTE_FAULT_DIS |
					 FIELD_PREP(AN8855_SGMII_IF_MODE, 0xb));
		if (ret)
			return ret;
	}

	/* Rate Adaption - GMII path config. */
	if (interface == PHY_INTERFACE_MODE_2500BASEX) {
		ret = regmap_clear_bits(priv->regmap, AN8855_RATE_ADP_P0_CTRL_0,
					AN8855_RG_P0_DIS_MII_MODE);
		if (ret)
			return ret;
	} else {
		if (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED) {
			ret = regmap_set_bits(priv->regmap, AN8855_MII_RA_AN_ENABLE,
					      AN8855_RG_P0_RA_AN_EN);
			if (ret)
				return ret;
		} else {
			ret = regmap_update_bits(priv->regmap, AN8855_RG_AN_SGMII_MODE_FORCE,
						 AN8855_RG_FORCE_CUR_SGMII_MODE |
						 AN8855_RG_FORCE_CUR_SGMII_SEL,
						 AN8855_RG_FORCE_CUR_SGMII_SEL);
			if (ret)
				return ret;

			ret = regmap_clear_bits(priv->regmap, AN8855_RATE_ADP_P0_CTRL_0,
						AN8855_RG_P0_MII_RA_RX_EN |
						AN8855_RG_P0_MII_RA_TX_EN |
						AN8855_RG_P0_MII_RA_RX_MODE |
						AN8855_RG_P0_MII_RA_TX_MODE);
			if (ret)
				return ret;
		}

		ret = regmap_set_bits(priv->regmap, AN8855_RATE_ADP_P0_CTRL_0,
				      AN8855_RG_P0_MII_MODE);
		if (ret)
			return ret;
	}

	ret = regmap_set_bits(priv->regmap, AN8855_RG_RATE_ADAPT_CTRL_0,
			      AN8855_RG_RATE_ADAPT_RX_BYPASS |
			      AN8855_RG_RATE_ADAPT_TX_BYPASS |
			      AN8855_RG_RATE_ADAPT_RX_EN |
			      AN8855_RG_RATE_ADAPT_TX_EN);
	if (ret)
		return ret;

	/* Disable AN if not in autoneg */
	ret = regmap_update_bits(priv->regmap, AN8855_SGMII_REG_AN0, BMCR_ANENABLE,
				 neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED ? BMCR_ANENABLE :
									      0);
	if (ret)
		return ret;

	if (interface == PHY_INTERFACE_MODE_SGMII) {
		/* Follow SDK init flow with restarting AN after AN enable */
		if (neg_mode == PHYLINK_PCS_NEG_INBAND_ENABLED) {
			ret = regmap_set_bits(priv->regmap, AN8855_SGMII_REG_AN0,
					      BMCR_ANRESTART);
			if (ret)
				return ret;
		} else {
			ret = regmap_set_bits(priv->regmap, AN8855_PHY_RX_FORCE_CTRL_0,
					      AN8855_RG_FORCE_TXC_SEL);
			if (ret)
				return ret;
		}
	}

	/* Force Speed with fixed-link or 2500base-x as doesn't support aneg */
	if (interface == PHY_INTERFACE_MODE_2500BASEX ||
	    neg_mode != PHYLINK_PCS_NEG_INBAND_ENABLED) {
		if (interface == PHY_INTERFACE_MODE_2500BASEX)
			val = AN8855_RG_LINK_MODE_P0_SPEED_2500;
		else
			val = AN8855_RG_LINK_MODE_P0_SPEED_1000;
		ret = regmap_update_bits(priv->regmap, AN8855_SGMII_STS_CTRL_0,
					 AN8855_RG_LINK_MODE_P0 |
					 AN8855_RG_FORCE_SPD_MODE_P0,
					 val | AN8855_RG_FORCE_SPD_MODE_P0);
		if (ret)
			return ret;
	}

	/* bypass flow control to MAC */
	ret = regmap_write(priv->regmap, AN8855_MSG_RX_LIK_STS_0,
			   AN8855_RG_DPX_STS_P3 | AN8855_RG_DPX_STS_P2 |
			   AN8855_RG_DPX_STS_P1 | AN8855_RG_TXFC_STS_P0 |
			   AN8855_RG_RXFC_STS_P0 | AN8855_RG_DPX_STS_P0);
	if (ret)
		return ret;
	ret = regmap_write(priv->regmap, AN8855_MSG_RX_LIK_STS_2,
			   AN8855_RG_RXFC_AN_BYPASS_P3 |
			   AN8855_RG_RXFC_AN_BYPASS_P2 |
			   AN8855_RG_RXFC_AN_BYPASS_P1 |
			   AN8855_RG_TXFC_AN_BYPASS_P3 |
			   AN8855_RG_TXFC_AN_BYPASS_P2 |
			   AN8855_RG_TXFC_AN_BYPASS_P1 |
			   AN8855_RG_DPX_AN_BYPASS_P3 |
			   AN8855_RG_DPX_AN_BYPASS_P2 |
			   AN8855_RG_DPX_AN_BYPASS_P1 |
			   AN8855_RG_DPX_AN_BYPASS_P0);
	if (ret)
		return ret;

	return 0;
}

static void an8855_pcs_an_restart(struct phylink_pcs *pcs)
{
	return;
}

static const struct phylink_pcs_ops an8855_pcs_ops = {
	.pcs_inband_caps = an8855_pcs_inband_caps,
	.pcs_get_state = an8855_pcs_get_state,
	.pcs_config = an8855_pcs_config,
	.pcs_an_restart = an8855_pcs_an_restart,
};

static const struct phylink_mac_ops an8855_phylink_mac_ops = {
	.mac_select_pcs	= an8855_phylink_mac_select_pcs,
	.mac_config	= an8855_phylink_mac_config,
	.mac_link_down	= an8855_phylink_mac_link_down,
	.mac_link_up	= an8855_phylink_mac_link_up,
};

static const struct dsa_switch_ops an8855_switch_ops = {
	.get_tag_protocol = an8855_get_tag_protocol,
	.setup = an8855_setup,
	.get_phy_flags = en8855_get_phy_flags,
	.phylink_get_caps = an8855_phylink_get_caps,
	.get_strings = an8855_get_strings,
	.get_ethtool_stats = an8855_get_ethtool_stats,
	.get_sset_count = an8855_get_sset_count,
	.get_eth_mac_stats = an8855_get_eth_mac_stats,
	.get_eth_ctrl_stats = an8855_get_eth_ctrl_stats,
	.get_rmon_stats = an8855_get_rmon_stats,
	.port_enable = an8855_port_enable,
	.port_disable = an8855_port_disable,
	.set_ageing_time = an8855_set_ageing_time,
	.port_bridge_join = an8855_port_bridge_join,
	.port_bridge_leave = an8855_port_bridge_leave,
	.port_fast_age = an8855_port_fast_age,
	.port_stp_state_set = an8855_port_stp_state_set,
	.port_pre_bridge_flags = an8855_port_pre_bridge_flags,
	.port_bridge_flags = an8855_port_bridge_flags,
	.port_vlan_filtering = an8855_port_vlan_filtering,
	.port_vlan_add = an8855_port_vlan_add,
	.port_vlan_del = an8855_port_vlan_del,
	.port_fdb_add = an8855_port_fdb_add,
	.port_fdb_del = an8855_port_fdb_del,
	.port_fdb_dump = an8855_port_fdb_dump,
	.port_mdb_add = an8855_port_mdb_add,
	.port_mdb_del = an8855_port_mdb_del,
	.port_change_mtu = an8855_port_change_mtu,
	.port_max_mtu = an8855_port_max_mtu,
	.port_mirror_add = an8855_port_mirror_add,
	.port_mirror_del = an8855_port_mirror_del,
};

static int an8855_read_switch_id(struct an8855_priv *priv)
{
	u32 id;
	int ret;

	ret = regmap_read(priv->regmap, AN8855_CREV, &id);
	if (ret)
		return ret;

	if (id != AN8855_ID) {
		dev_err(priv->dev,
			"Switch id detected %x but expected %x\n",
			id, AN8855_ID);
		return -ENODEV;
	}

	return 0;
}

static int an8855_switch_probe(struct platform_device *pdev)
{
	struct an8855_priv *priv;
	u32 val;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;
	priv->phy_require_calib = of_property_read_bool(priv->dev->of_node,
							"airoha,ext-surge");

	priv->reset_gpio = devm_gpiod_get_optional(priv->dev, "reset",
						   GPIOD_OUT_LOW);
	if (IS_ERR(priv->reset_gpio))
		return PTR_ERR(priv->reset_gpio);

	/* Get regmap from MFD */
	priv->regmap = dev_get_regmap(priv->dev->parent, NULL);

	if (priv->reset_gpio) {
		usleep_range(100000, 150000);
		gpiod_set_value_cansleep(priv->reset_gpio, 0);
		usleep_range(100000, 150000);
		gpiod_set_value_cansleep(priv->reset_gpio, 1);

		/* Poll HWTRAP reg to wait for Switch to fully Init */
		ret = regmap_read_poll_timeout(priv->regmap, AN8855_HWTRAP, val,
					       val, 20, 200000);
		if (ret)
			return ret;
	}

	ret = an8855_read_switch_id(priv);
	if (ret)
		return ret;

	priv->ds = devm_kzalloc(priv->dev, sizeof(*priv->ds), GFP_KERNEL);
	if (!priv->ds)
		return -ENOMEM;

	priv->ds->dev = priv->dev;
	priv->ds->num_ports = AN8855_NUM_PORTS;
	priv->ds->priv = priv;
	priv->ds->ops = &an8855_switch_ops;
	devm_mutex_init(priv->dev, &priv->reg_mutex);
	priv->ds->phylink_mac_ops = &an8855_phylink_mac_ops;

	priv->pcs.ops = &an8855_pcs_ops;
	priv->pcs.neg_mode = true;
	priv->pcs.poll = true;

	dev_set_drvdata(priv->dev, priv);

	return dsa_register_switch(priv->ds);
}

static void an8855_switch_remove(struct platform_device *pdev)
{
	struct an8855_priv *priv = dev_get_drvdata(&pdev->dev);

	if (priv)
		dsa_unregister_switch(priv->ds);
}

static const struct of_device_id an8855_switch_of_match[] = {
	{ .compatible = "airoha,an8855-switch" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, an8855_switch_of_match);

static struct platform_driver an8855_switch_driver = {
	.probe = an8855_switch_probe,
	.remove_new = an8855_switch_remove,
	.driver = {
		.name = "an8855-switch",
		.of_match_table = an8855_switch_of_match,
	},
};
module_platform_driver(an8855_switch_driver);

MODULE_AUTHOR("Min Yao <min.yao@airoha.com>");
MODULE_AUTHOR("Christian Marangi <ansuelsmth@gmail.com>");
MODULE_DESCRIPTION("Driver for Airoha AN8855 Switch");
MODULE_LICENSE("GPL");
