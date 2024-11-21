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
#ifndef _LINUX_SWITCH_H
#define _LINUX_SWITCH_H

#include <net/genetlink.h>
#include <uapi/linux/switch.h>

struct switch_dev;
struct switch_op;
struct switch_val;
struct switch_attr;
struct switch_attrlist;
struct switch_led_trigger;

int register_switch(struct switch_dev *dev, struct net_device *netdev);
void unregister_switch(struct switch_dev *dev);

/**
 * struct switch_attrlist - attribute list
 *
 * @n_attr: number of attributes
 * @attr: pointer to the attributes array
 */
struct switch_attrlist {
	int n_attr;
	const struct switch_attr *attr;
};

enum switch_port_speed {
	SWITCH_PORT_SPEED_UNKNOWN = 0,
	SWITCH_PORT_SPEED_10 = 10,
	SWITCH_PORT_SPEED_100 = 100,
	SWITCH_PORT_SPEED_1000 = 1000,
};

struct switch_port_link {
	bool link;
	bool duplex;
	bool aneg;
	bool tx_flow;
	bool rx_flow;
	enum switch_port_speed speed;
	/* in ethtool adv_t format */
	u32 eee;
};

struct switch_port_stats {
	unsigned long long tx_bytes;
	unsigned long long rx_bytes;
};

/**
 * struct switch_dev_ops - switch driver operations
 *
 * @attr_global: global switch attribute list
 * @attr_port: port attribute list
 * @attr_vlan: vlan attribute list
 *
 * Callbacks:
 *
 * @get_vlan_ports: read the port list of a VLAN
 * @set_vlan_ports: set the port list of a VLAN
 *
 * @get_port_pvid: get the primary VLAN ID of a port
 * @set_port_pvid: set the primary VLAN ID of a port
 *
 * @apply_config: apply all changed settings to the switch
 * @reset_switch: resetting the switch
 */
struct switch_dev_ops {
	struct switch_attrlist attr_global, attr_port, attr_vlan;

	int (*get_vlan_ports)(struct switch_dev *dev, struct switch_val *val);
	int (*set_vlan_ports)(struct switch_dev *dev, struct switch_val *val);

	int (*get_port_pvid)(struct switch_dev *dev, int port, int *val);
	int (*set_port_pvid)(struct switch_dev *dev, int port, int val);

	int (*apply_config)(struct switch_dev *dev);
	int (*reset_switch)(struct switch_dev *dev);

	int (*get_port_link)(struct switch_dev *dev, int port,
			     struct switch_port_link *link);
	int (*set_port_link)(struct switch_dev *dev, int port,
			     struct switch_port_link *link);
	int (*get_port_stats)(struct switch_dev *dev, int port,
			      struct switch_port_stats *stats);

	int (*phy_read16)(struct switch_dev *dev, int addr, u8 reg, u16 *value);
	int (*phy_write16)(struct switch_dev *dev, int addr, u8 reg, u16 value);
};

struct switch_dev {
	struct device_node *of_node;
	const struct switch_dev_ops *ops;
	/* will be automatically filled */
	char devname[IFNAMSIZ];

	const char *name;
	/* NB: either alias or netdev must be set */
	const char *alias;
	struct net_device *netdev;

	unsigned int ports;
	unsigned int vlans;
	unsigned int cpu_port;

	/* the following fields are internal for swconfig */
	unsigned int id;
	struct list_head dev_list;
	unsigned long def_global, def_port, def_vlan;

	struct mutex sw_mutex;
	struct switch_port *portbuf;
	struct switch_portmap *portmap;
	struct switch_port_link linkbuf;

	char buf[128];

#ifdef CONFIG_SWCONFIG_LEDS
	struct switch_led_trigger *led_trigger;
#endif
};

struct switch_port {
	u32 id;
	u32 flags;
};

struct switch_portmap {
	u32 virt;
	const char *s;
};

struct switch_val {
	const struct switch_attr *attr;
	unsigned int port_vlan;
	unsigned int len;
	union {
		const char *s;
		u32 i;
		struct switch_port *ports;
		struct switch_port_link *link;
	} value;
};

struct switch_attr {
	int disabled;
	int type;
	const char *name;
	const char *description;

	int (*set)(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val);
	int (*get)(struct switch_dev *dev, const struct switch_attr *attr, struct switch_val *val);

	/* for driver internal use */
	int id;
	int ofs;
	int max;
};

int switch_generic_set_link(struct switch_dev *dev, int port,
			    struct switch_port_link *link);

#endif /* _LINUX_SWITCH_H */
