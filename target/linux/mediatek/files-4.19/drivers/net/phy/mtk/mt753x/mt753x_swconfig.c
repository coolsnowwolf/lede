/*
 * OpenWrt swconfig support for MediaTek MT753x Gigabit switch
 *
 * Copyright (C) 2018 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/if.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/bitops.h>
#include <net/genetlink.h>
#include <linux/delay.h>
#include <linux/phy.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/lockdep.h>
#include <linux/workqueue.h>
#include <linux/of_device.h>

#include "mt753x.h"
#include "mt753x_swconfig.h"
#include "mt753x_regs.h"

#define MT753X_PORT_MIB_TXB_ID	18	/* TxByte */
#define MT753X_PORT_MIB_RXB_ID	37	/* RxByte */

#define MIB_DESC(_s, _o, _n)   \
	{                       \
		.size = (_s),   \
		.offset = (_o), \
		.name = (_n),   \
	}

struct mt753x_mib_desc {
	unsigned int size;
	unsigned int offset;
	const char *name;
};

static const struct mt753x_mib_desc mt753x_mibs[] = {
	MIB_DESC(1, STATS_TDPC, "TxDrop"),
	MIB_DESC(1, STATS_TCRC, "TxCRC"),
	MIB_DESC(1, STATS_TUPC, "TxUni"),
	MIB_DESC(1, STATS_TMPC, "TxMulti"),
	MIB_DESC(1, STATS_TBPC, "TxBroad"),
	MIB_DESC(1, STATS_TCEC, "TxCollision"),
	MIB_DESC(1, STATS_TSCEC, "TxSingleCol"),
	MIB_DESC(1, STATS_TMCEC, "TxMultiCol"),
	MIB_DESC(1, STATS_TDEC, "TxDefer"),
	MIB_DESC(1, STATS_TLCEC, "TxLateCol"),
	MIB_DESC(1, STATS_TXCEC, "TxExcCol"),
	MIB_DESC(1, STATS_TPPC, "TxPause"),
	MIB_DESC(1, STATS_TL64PC, "Tx64Byte"),
	MIB_DESC(1, STATS_TL65PC, "Tx65Byte"),
	MIB_DESC(1, STATS_TL128PC, "Tx128Byte"),
	MIB_DESC(1, STATS_TL256PC, "Tx256Byte"),
	MIB_DESC(1, STATS_TL512PC, "Tx512Byte"),
	MIB_DESC(1, STATS_TL1024PC, "Tx1024Byte"),
	MIB_DESC(2, STATS_TOC, "TxByte"),
	MIB_DESC(1, STATS_RDPC, "RxDrop"),
	MIB_DESC(1, STATS_RFPC, "RxFiltered"),
	MIB_DESC(1, STATS_RUPC, "RxUni"),
	MIB_DESC(1, STATS_RMPC, "RxMulti"),
	MIB_DESC(1, STATS_RBPC, "RxBroad"),
	MIB_DESC(1, STATS_RAEPC, "RxAlignErr"),
	MIB_DESC(1, STATS_RCEPC, "RxCRC"),
	MIB_DESC(1, STATS_RUSPC, "RxUnderSize"),
	MIB_DESC(1, STATS_RFEPC, "RxFragment"),
	MIB_DESC(1, STATS_ROSPC, "RxOverSize"),
	MIB_DESC(1, STATS_RJEPC, "RxJabber"),
	MIB_DESC(1, STATS_RPPC, "RxPause"),
	MIB_DESC(1, STATS_RL64PC, "Rx64Byte"),
	MIB_DESC(1, STATS_RL65PC, "Rx65Byte"),
	MIB_DESC(1, STATS_RL128PC, "Rx128Byte"),
	MIB_DESC(1, STATS_RL256PC, "Rx256Byte"),
	MIB_DESC(1, STATS_RL512PC, "Rx512Byte"),
	MIB_DESC(1, STATS_RL1024PC, "Rx1024Byte"),
	MIB_DESC(2, STATS_ROC, "RxByte"),
	MIB_DESC(1, STATS_RDPC_CTRL, "RxCtrlDrop"),
	MIB_DESC(1, STATS_RDPC_ING, "RxIngDrop"),
	MIB_DESC(1, STATS_RDPC_ARL, "RxARLDrop")
};

enum {
	/* Global attributes. */
	MT753X_ATTR_ENABLE_VLAN,
};

struct mt753x_mapping {
	char	*name;
	u16	pvids[MT753X_NUM_PORTS];
	u8	members[MT753X_NUM_VLANS];
	u8	etags[MT753X_NUM_VLANS];
	u16	vids[MT753X_NUM_VLANS];
} mt753x_defaults[] = {
	{
		.name = "llllw",
		.pvids = { 1, 1, 1, 1, 2, 2, 1 },
		.members = { 0, 0x4f, 0x30 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "wllll",
		.pvids = { 2, 1, 1, 1, 1, 2, 1 },
		.members = { 0, 0x5e, 0x21 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	}, {
		.name = "lwlll",
		.pvids = { 1, 2, 1, 1, 1, 2, 1 },
		.members = { 0, 0x5d, 0x22 },
		.etags = { 0, 0, 0 },
		.vids = { 0, 1, 2 },
	},
};

struct mt753x_mapping *mt753x_find_mapping(struct device_node *np)
{
	const char *map;
	int i;

	if (of_property_read_string(np, "mediatek,portmap", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(mt753x_defaults); i++)
		if (!strcmp(map, mt753x_defaults[i].name))
			return &mt753x_defaults[i];

	return NULL;
}

static void mt753x_apply_mapping(struct gsw_mt753x *gsw,
				 struct mt753x_mapping *map)
{
	int i = 0;

	for (i = 0; i < MT753X_NUM_PORTS; i++)
		gsw->port_entries[i].pvid = map->pvids[i];

	for (i = 0; i < MT753X_NUM_VLANS; i++) {
		gsw->vlan_entries[i].member = map->members[i];
		gsw->vlan_entries[i].etags = map->etags[i];
		gsw->vlan_entries[i].vid = map->vids[i];
	}
}

static int mt753x_get_vlan_enable(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	val->value.i = gsw->global_vlan_enable;

	return 0;
}

static int mt753x_set_vlan_enable(struct switch_dev *dev,
				  const struct switch_attr *attr,
				  struct switch_val *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	gsw->global_vlan_enable = val->value.i != 0;

	return 0;
}

static int mt753x_get_port_pvid(struct switch_dev *dev, int port, int *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	if (port >= MT753X_NUM_PORTS)
		return -EINVAL;

	*val = mt753x_reg_read(gsw, PPBV1(port));
	*val &= GRP_PORT_VID_M;

	return 0;
}

static int mt753x_set_port_pvid(struct switch_dev *dev, int port, int pvid)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	if (port >= MT753X_NUM_PORTS)
		return -EINVAL;

	if (pvid < MT753X_MIN_VID || pvid > MT753X_MAX_VID)
		return -EINVAL;

	gsw->port_entries[port].pvid = pvid;

	return 0;
}

static void mt753x_vlan_ctrl(struct gsw_mt753x *gsw, u32 cmd, u32 val)
{
	int i;

	mt753x_reg_write(gsw, VTCR,
			 VTCR_BUSY | ((cmd << VTCR_FUNC_S) & VTCR_FUNC_M) |
			 (val & VTCR_VID_M));

	for (i = 0; i < 300; i++) {
		u32 val = mt753x_reg_read(gsw, VTCR);

		if ((val & VTCR_BUSY) == 0)
			break;

		usleep_range(1000, 1100);
	}

	if (i == 300)
		dev_info(gsw->dev, "vtcr timeout\n");
}

static int mt753x_get_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	u32 member;
	u32 etags;
	int i;

	val->len = 0;

	if (val->port_vlan < 0 || val->port_vlan >= MT753X_NUM_VLANS)
		return -EINVAL;

	mt753x_vlan_ctrl(gsw, VTCR_READ_VLAN_ENTRY, val->port_vlan);

	member = mt753x_reg_read(gsw, VAWD1);
	member &= PORT_MEM_M;
	member >>= PORT_MEM_S;

	etags = mt753x_reg_read(gsw, VAWD2);

	for (i = 0; i < MT753X_NUM_PORTS; i++) {
		struct switch_port *p;
		int etag;

		if (!(member & BIT(i)))
			continue;

		p = &val->value.ports[val->len++];
		p->id = i;

		etag = (etags >> PORT_ETAG_S(i)) & PORT_ETAG_M;

		if (etag == ETAG_CTRL_TAG)
			p->flags |= BIT(SWITCH_PORT_FLAG_TAGGED);
		else if (etag != ETAG_CTRL_UNTAG)
			dev_info(gsw->dev,
				 "vlan egress tag control neither untag nor tag.\n");
	}

	return 0;
}

static int mt753x_set_vlan_ports(struct switch_dev *dev, struct switch_val *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	u8 member = 0;
	u8 etags = 0;
	int i;

	if (val->port_vlan < 0 || val->port_vlan >= MT753X_NUM_VLANS ||
	    val->len > MT753X_NUM_PORTS)
		return -EINVAL;

	for (i = 0; i < val->len; i++) {
		struct switch_port *p = &val->value.ports[i];

		if (p->id >= MT753X_NUM_PORTS)
			return -EINVAL;

		member |= BIT(p->id);

		if (p->flags & BIT(SWITCH_PORT_FLAG_TAGGED))
			etags |= BIT(p->id);
	}

	gsw->vlan_entries[val->port_vlan].member = member;
	gsw->vlan_entries[val->port_vlan].etags = etags;

	return 0;
}

static int mt753x_set_vid(struct switch_dev *dev,
			  const struct switch_attr *attr,
			  struct switch_val *val)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	int vlan;
	u16 vid;

	vlan = val->port_vlan;
	vid = (u16)val->value.i;

	if (vlan < 0 || vlan >= MT753X_NUM_VLANS)
		return -EINVAL;

	if (vid < MT753X_MIN_VID || vid > MT753X_MAX_VID)
		return -EINVAL;

	gsw->vlan_entries[vlan].vid = vid;
	return 0;
}

static int mt753x_get_vid(struct switch_dev *dev,
			  const struct switch_attr *attr,
			  struct switch_val *val)
{
	val->value.i = val->port_vlan;
	return 0;
}

static int mt753x_get_port_link(struct switch_dev *dev, int port,
				struct switch_port_link *link)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	u32 speed, pmsr;

	if (port < 0 || port >= MT753X_NUM_PORTS)
		return -EINVAL;

	pmsr = mt753x_reg_read(gsw, PMSR(port));

	link->link = pmsr & MAC_LNK_STS;
	link->duplex = pmsr & MAC_DPX_STS;
	speed = (pmsr & MAC_SPD_STS_M) >> MAC_SPD_STS_S;

	switch (speed) {
	case MAC_SPD_10:
		link->speed = SWITCH_PORT_SPEED_10;
		break;
	case MAC_SPD_100:
		link->speed = SWITCH_PORT_SPEED_100;
		break;
	case MAC_SPD_1000:
		link->speed = SWITCH_PORT_SPEED_1000;
		break;
	case MAC_SPD_2500:
		/* TODO: swconfig has no support for 2500 now */
		link->speed = SWITCH_PORT_SPEED_UNKNOWN;
		break;
	}

	return 0;
}

static int mt753x_set_port_link(struct switch_dev *dev, int port,
				struct switch_port_link *link)
{
#ifndef MODULE
	if (port >= MT753X_NUM_PHYS)
		return -EINVAL;

	return switch_generic_set_link(dev, port, link);
#else
	return -ENOTSUPP;
#endif
}

static u64 get_mib_counter(struct gsw_mt753x *gsw, int i, int port)
{
	unsigned int offset;
	u64 lo, hi, hi2;

	offset = mt753x_mibs[i].offset;

	if (mt753x_mibs[i].size == 1)
		return mt753x_reg_read(gsw, MIB_COUNTER_REG(port, offset));

	do {
		hi = mt753x_reg_read(gsw, MIB_COUNTER_REG(port, offset + 4));
		lo = mt753x_reg_read(gsw, MIB_COUNTER_REG(port, offset));
		hi2 = mt753x_reg_read(gsw, MIB_COUNTER_REG(port, offset + 4));
	} while (hi2 != hi);

	return (hi << 32) | lo;
}

static int mt753x_get_port_mib(struct switch_dev *dev,
			       const struct switch_attr *attr,
			       struct switch_val *val)
{
	static char buf[4096];
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	int i, len = 0;

	if (val->port_vlan >= MT753X_NUM_PORTS)
		return -EINVAL;

	len += snprintf(buf + len, sizeof(buf) - len,
			"Port %d MIB counters\n", val->port_vlan);

	for (i = 0; i < ARRAY_SIZE(mt753x_mibs); ++i) {
		u64 counter;

		len += snprintf(buf + len, sizeof(buf) - len,
				"%-11s: ", mt753x_mibs[i].name);
		counter = get_mib_counter(gsw, i, val->port_vlan);
		len += snprintf(buf + len, sizeof(buf) - len, "%llu\n",
				counter);
	}

	val->value.s = buf;
	val->len = len;
	return 0;
}

static int mt753x_get_port_stats(struct switch_dev *dev, int port,
				 struct switch_port_stats *stats)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	if (port < 0 || port >= MT753X_NUM_PORTS)
		return -EINVAL;

	stats->tx_bytes = get_mib_counter(gsw, MT753X_PORT_MIB_TXB_ID, port);
	stats->rx_bytes = get_mib_counter(gsw, MT753X_PORT_MIB_RXB_ID, port);

	return 0;
}

static void mt753x_port_isolation(struct gsw_mt753x *gsw)
{
	int i;

	for (i = 0; i < MT753X_NUM_PORTS; i++)
		mt753x_reg_write(gsw, PCR(i),
				 BIT(gsw->cpu_port) << PORT_MATRIX_S);

	mt753x_reg_write(gsw, PCR(gsw->cpu_port), PORT_MATRIX_M);

	for (i = 0; i < MT753X_NUM_PORTS; i++)
		mt753x_reg_write(gsw, PVC(i),
				 (0x8100 << STAG_VPID_S) |
				 (VA_TRANSPARENT_PORT << VLAN_ATTR_S));
}

static void mt753x_write_vlan_entry(struct gsw_mt753x *gsw, int vlan, u16 vid,
				    u8 ports, u8 etags)
{
	int port;
	u32 val;

	/* vlan port membership */
	if (ports)
		mt753x_reg_write(gsw, VAWD1,
				 IVL_MAC | VTAG_EN | VENTRY_VALID |
				 ((ports << PORT_MEM_S) & PORT_MEM_M));
	else
		mt753x_reg_write(gsw, VAWD1, 0);

	/* egress mode */
	val = 0;
	for (port = 0; port < MT753X_NUM_PORTS; port++) {
		if (etags & BIT(port))
			val |= ETAG_CTRL_TAG << PORT_ETAG_S(port);
		else
			val |= ETAG_CTRL_UNTAG << PORT_ETAG_S(port);
	}
	mt753x_reg_write(gsw, VAWD2, val);

	/* write to vlan table */
	mt753x_vlan_ctrl(gsw, VTCR_WRITE_VLAN_ENTRY, vid);
}

static int mt753x_apply_config(struct switch_dev *dev)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	int i, j;
	u8 tag_ports;
	u8 untag_ports;

	if (!gsw->global_vlan_enable) {
		mt753x_port_isolation(gsw);
		return 0;
	}

	/* set all ports as security mode */
	for (i = 0; i < MT753X_NUM_PORTS; i++)
		mt753x_reg_write(gsw, PCR(i),
				 PORT_MATRIX_M | SECURITY_MODE);

	/* check if a port is used in tag/untag vlan egress mode */
	tag_ports = 0;
	untag_ports = 0;

	for (i = 0; i < MT753X_NUM_VLANS; i++) {
		u8 member = gsw->vlan_entries[i].member;
		u8 etags = gsw->vlan_entries[i].etags;

		if (!member)
			continue;

		for (j = 0; j < MT753X_NUM_PORTS; j++) {
			if (!(member & BIT(j)))
				continue;

			if (etags & BIT(j))
				tag_ports |= 1u << j;
			else
				untag_ports |= 1u << j;
		}
	}

	/* set all untag-only ports as transparent and the rest as user port */
	for (i = 0; i < MT753X_NUM_PORTS; i++) {
		u32 pvc_mode = 0x8100 << STAG_VPID_S;

		if (untag_ports & BIT(i) && !(tag_ports & BIT(i)))
			pvc_mode = (0x8100 << STAG_VPID_S) |
				(VA_TRANSPARENT_PORT << VLAN_ATTR_S);

		mt753x_reg_write(gsw, PVC(i), pvc_mode);
	}

	/* first clear the swtich vlan table */
	for (i = 0; i < MT753X_NUM_VLANS; i++)
		mt753x_write_vlan_entry(gsw, i, i, 0, 0);

	/* now program only vlans with members to avoid
	 * clobbering remapped entries in later iterations
	 */
	for (i = 0; i < MT753X_NUM_VLANS; i++) {
		u16 vid = gsw->vlan_entries[i].vid;
		u8 member = gsw->vlan_entries[i].member;
		u8 etags = gsw->vlan_entries[i].etags;

		if (member)
			mt753x_write_vlan_entry(gsw, i, vid, member, etags);
	}

	/* Port Default PVID */
	for (i = 0; i < MT753X_NUM_PORTS; i++) {
		int vlan = gsw->port_entries[i].pvid;
		u16 pvid = 0;
		u32 val;

		if (vlan < MT753X_NUM_VLANS && gsw->vlan_entries[vlan].member)
			pvid = gsw->vlan_entries[vlan].vid;

		val = mt753x_reg_read(gsw, PPBV1(i));
		val &= ~GRP_PORT_VID_M;
		val |= pvid;
		mt753x_reg_write(gsw, PPBV1(i), val);
	}

	return 0;
}

static int mt753x_reset_switch(struct switch_dev *dev)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);
	int i;

	memset(gsw->port_entries, 0, sizeof(gsw->port_entries));
	memset(gsw->vlan_entries, 0, sizeof(gsw->vlan_entries));

	/* set default vid of each vlan to the same number of vlan, so the vid
	 * won't need be set explicitly.
	 */
	for (i = 0; i < MT753X_NUM_VLANS; i++)
		gsw->vlan_entries[i].vid = i;

	return 0;
}

static int mt753x_phy_read16(struct switch_dev *dev, int addr, u8 reg,
			     u16 *value)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	*value = gsw->mii_read(gsw, addr, reg);

	return 0;
}

static int mt753x_phy_write16(struct switch_dev *dev, int addr, u8 reg,
			      u16 value)
{
	struct gsw_mt753x *gsw = container_of(dev, struct gsw_mt753x, swdev);

	gsw->mii_write(gsw, addr, reg, value);

	return 0;
}

static const struct switch_attr mt753x_global[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "enable_vlan",
		.description = "VLAN mode (1:enabled)",
		.max = 1,
		.id = MT753X_ATTR_ENABLE_VLAN,
		.get = mt753x_get_vlan_enable,
		.set = mt753x_set_vlan_enable,
	}
};

static const struct switch_attr mt753x_port[] = {
	{
		.type = SWITCH_TYPE_STRING,
		.name = "mib",
		.description = "Get MIB counters for port",
		.get = mt753x_get_port_mib,
		.set = NULL,
	},
};

static const struct switch_attr mt753x_vlan[] = {
	{
		.type = SWITCH_TYPE_INT,
		.name = "vid",
		.description = "VLAN ID (0-4094)",
		.set = mt753x_set_vid,
		.get = mt753x_get_vid,
		.max = 4094,
	},
};

static const struct switch_dev_ops mt753x_swdev_ops = {
	.attr_global = {
		.attr = mt753x_global,
		.n_attr = ARRAY_SIZE(mt753x_global),
	},
	.attr_port = {
		.attr = mt753x_port,
		.n_attr = ARRAY_SIZE(mt753x_port),
	},
	.attr_vlan = {
		.attr = mt753x_vlan,
		.n_attr = ARRAY_SIZE(mt753x_vlan),
	},
	.get_vlan_ports = mt753x_get_vlan_ports,
	.set_vlan_ports = mt753x_set_vlan_ports,
	.get_port_pvid = mt753x_get_port_pvid,
	.set_port_pvid = mt753x_set_port_pvid,
	.get_port_link = mt753x_get_port_link,
	.set_port_link = mt753x_set_port_link,
	.get_port_stats = mt753x_get_port_stats,
	.apply_config = mt753x_apply_config,
	.reset_switch = mt753x_reset_switch,
	.phy_read16 = mt753x_phy_read16,
	.phy_write16 = mt753x_phy_write16,
};

int mt753x_swconfig_init(struct gsw_mt753x *gsw)
{
	struct device_node *np = gsw->dev->of_node;
	struct switch_dev *swdev;
	struct mt753x_mapping *map;
	int ret;

	if (of_property_read_u32(np, "mediatek,cpuport", &gsw->cpu_port))
		gsw->cpu_port = MT753X_DFL_CPU_PORT;

	swdev = &gsw->swdev;

	swdev->name = gsw->name;
	swdev->alias = gsw->name;
	swdev->cpu_port = gsw->cpu_port;
	swdev->ports = MT753X_NUM_PORTS;
	swdev->vlans = MT753X_NUM_VLANS;
	swdev->ops = &mt753x_swdev_ops;

	ret = register_switch(swdev, NULL);
	if (ret) {
		dev_err(gsw->dev, "Failed to register switch %s\n",
			swdev->name);
		return ret;
	}

	map = mt753x_find_mapping(gsw->dev->of_node);
	if (map)
		mt753x_apply_mapping(gsw, map);
	mt753x_apply_config(swdev);

	return 0;
}

void mt753x_swconfig_destroy(struct gsw_mt753x *gsw)
{
	unregister_switch(&gsw->swdev);
}
