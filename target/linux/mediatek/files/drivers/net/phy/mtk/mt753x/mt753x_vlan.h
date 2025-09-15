/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2018 MediaTek Inc.
 */

#ifndef _MT753X_VLAN_H_
#define _MT753X_VLAN_H_

#define MT753X_NUM_PORTS	7
#define MT753X_NUM_VLANS	4095
#define MT753X_MAX_VID		4095
#define MT753X_MIN_VID		0

struct gsw_mt753x;

struct mt753x_port_entry {
	u16	pvid;
};

struct mt753x_vlan_entry {
	u16	vid;
	u8	member;
	u8	etags;
};

struct mt753x_mapping {
	char	*name;
	u16	pvids[MT753X_NUM_PORTS];
	u8	members[MT753X_NUM_VLANS];
	u8	etags[MT753X_NUM_VLANS];
	u16	vids[MT753X_NUM_VLANS];
};

extern struct mt753x_mapping mt753x_defaults[];

void mt753x_vlan_ctrl(struct gsw_mt753x *gsw, u32 cmd, u32 val);
void mt753x_apply_vlan_config(struct gsw_mt753x *gsw);
struct mt753x_mapping *mt753x_find_mapping(struct device_node *np);
void mt753x_apply_mapping(struct gsw_mt753x *gsw, struct mt753x_mapping *map);
#endif /* _MT753X_VLAN_H_ */
