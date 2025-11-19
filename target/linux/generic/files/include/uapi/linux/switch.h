/*
 * switch.h: Switch configuration API
 *
 * Copyright (C) 2008 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _UAPI_LINUX_SWITCH_H
#define _UAPI_LINUX_SWITCH_H

#include <linux/types.h>
#include <linux/netdevice.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#ifndef __KERNEL__
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#endif

/* main attributes */
enum {
	SWITCH_ATTR_UNSPEC,
	/* global */
	SWITCH_ATTR_TYPE,
	/* device */
	SWITCH_ATTR_ID,
	SWITCH_ATTR_DEV_NAME,
	SWITCH_ATTR_ALIAS,
	SWITCH_ATTR_NAME,
	SWITCH_ATTR_VLANS,
	SWITCH_ATTR_PORTS,
	SWITCH_ATTR_PORTMAP,
	SWITCH_ATTR_CPU_PORT,
	/* attributes */
	SWITCH_ATTR_OP_ID,
	SWITCH_ATTR_OP_TYPE,
	SWITCH_ATTR_OP_NAME,
	SWITCH_ATTR_OP_PORT,
	SWITCH_ATTR_OP_VLAN,
	SWITCH_ATTR_OP_VALUE_INT,
	SWITCH_ATTR_OP_VALUE_STR,
	SWITCH_ATTR_OP_VALUE_PORTS,
	SWITCH_ATTR_OP_VALUE_LINK,
	SWITCH_ATTR_OP_DESCRIPTION,
	/* port lists */
	SWITCH_ATTR_PORT,
	SWITCH_ATTR_MAX
};

enum {
	/* port map */
	SWITCH_PORTMAP_PORTS,
	SWITCH_PORTMAP_SEGMENT,
	SWITCH_PORTMAP_VIRT,
	SWITCH_PORTMAP_MAX
};

/* commands */
enum {
	SWITCH_CMD_UNSPEC,
	SWITCH_CMD_GET_SWITCH,
	SWITCH_CMD_NEW_ATTR,
	SWITCH_CMD_LIST_GLOBAL,
	SWITCH_CMD_GET_GLOBAL,
	SWITCH_CMD_SET_GLOBAL,
	SWITCH_CMD_LIST_PORT,
	SWITCH_CMD_GET_PORT,
	SWITCH_CMD_SET_PORT,
	SWITCH_CMD_LIST_VLAN,
	SWITCH_CMD_GET_VLAN,
	SWITCH_CMD_SET_VLAN
};

/* data types */
enum switch_val_type {
	SWITCH_TYPE_UNSPEC,
	SWITCH_TYPE_INT,
	SWITCH_TYPE_STRING,
	SWITCH_TYPE_PORTS,
	SWITCH_TYPE_LINK,
	SWITCH_TYPE_NOVAL,
};

/* port nested attributes */
enum {
	SWITCH_PORT_UNSPEC,
	SWITCH_PORT_ID,
	SWITCH_PORT_FLAG_TAGGED,
	SWITCH_PORT_ATTR_MAX
};

/* link nested attributes */
enum {
	SWITCH_LINK_UNSPEC,
	SWITCH_LINK_FLAG_LINK,
	SWITCH_LINK_FLAG_DUPLEX,
	SWITCH_LINK_FLAG_ANEG,
	SWITCH_LINK_FLAG_TX_FLOW,
	SWITCH_LINK_FLAG_RX_FLOW,
	SWITCH_LINK_SPEED,
	SWITCH_LINK_FLAG_EEE_100BASET,
	SWITCH_LINK_FLAG_EEE_1000BASET,
	SWITCH_LINK_FLAG_EEE_2500BASET,
	SWITCH_LINK_FLAG_EEE_5000BASET,
	SWITCH_LINK_FLAG_EEE_10000BASET,
	SWITCH_LINK_ATTR_MAX,
};

#define SWITCH_ATTR_DEFAULTS_OFFSET	0x1000


#endif /* _UAPI_LINUX_SWITCH_H */
