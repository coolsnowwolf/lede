// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 */

#include "mt753x.h"
#include "mt753x_regs.h"

struct mt753x_mapping mt753x_def_mapping[] = {
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

void mt753x_vlan_ctrl(struct gsw_mt753x *gsw, u32 cmd, u32 val)
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

void mt753x_apply_vlan_config(struct gsw_mt753x *gsw)
{
	int i, j;
	u8 tag_ports;
	u8 untag_ports;

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

		if ((gsw->port5_cfg.stag_on && i == 5) ||
		    (gsw->port6_cfg.stag_on && i == 6))
			pvc_mode = (0x8100 << STAG_VPID_S) | PVC_PORT_STAG;

		mt753x_reg_write(gsw, PVC(i), pvc_mode);
	}

	/* first clear the switch vlan table */
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
}

struct mt753x_mapping *mt753x_find_mapping(struct device_node *np)
{
	const char *map;
	int i;

	if (of_property_read_string(np, "mediatek,portmap", &map))
		return NULL;

	for (i = 0; i < ARRAY_SIZE(mt753x_def_mapping); i++)
		if (!strcmp(map, mt753x_def_mapping[i].name))
			return &mt753x_def_mapping[i];

	return NULL;
}

void mt753x_apply_mapping(struct gsw_mt753x *gsw, struct mt753x_mapping *map)
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
