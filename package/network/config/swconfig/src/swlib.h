/*
 * swlib.h: Switch configuration API (user space part)
 *
 * Copyright (C) 2008-2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *

Usage of the library functions:

  The main datastructure for a switch is the struct switch_device
  To get started, you first need to use switch_connect() to probe
  for switches and allocate an instance of this struct.

  There are two possible usage modes:
    dev = switch_connect("eth0");
      - this call will look for a switch registered for the linux device
  	  "eth0" and only allocate a switch_device for this particular switch.

    dev = switch_connect(NULL)
      - this will return one switch_device struct for each available
  	  switch. The switch_device structs are chained with by ->next pointer

  Then to query a switch for all available attributes, use:
    swlib_scan(dev);

  All allocated datastructures for the switch_device struct can be freed with
    swlib_free(dev);
  or
    swlib_free_all(dev);

  The latter traverses a whole chain of switch_device structs and frees them all

  Switch attributes (struct switch_attr) are divided into three groups:
    dev->ops:
      - global settings
    dev->port_ops:
      - per-port settings
    dev->vlan_ops:
      - per-vlan settings

  switch_lookup_attr() is a small helper function to locate attributes
  by name.

  switch_set_attr() and switch_get_attr() can alter or request the values
  of attributes.

Usage of the switch_attr struct:

  ->atype: attribute group, one of:
    - SWLIB_ATTR_GROUP_GLOBAL
    - SWLIB_ATTR_GROUP_VLAN
    - SWLIB_ATTR_GROUP_PORT

  ->id: identifier for the attribute

  ->type: data type, one of:
    - SWITCH_TYPE_INT
    - SWITCH_TYPE_STRING
    - SWITCH_TYPE_PORT

  ->name: short name of the attribute
  ->description: longer description
  ->next: pointer to the next attribute of the current group


Usage of the switch_val struct:

  When setting attributes, following members of the struct switch_val need
  to be set up:

    ->len (for attr->type == SWITCH_TYPE_PORT)
    ->port_vlan:
      - port number (for attr->atype == SWLIB_ATTR_GROUP_PORT), or:
      - vlan number (for attr->atype == SWLIB_ATTR_GROUP_VLAN)
    ->value.i (for attr->type == SWITCH_TYPE_INT)
    ->value.s (for attr->type == SWITCH_TYPE_STRING)
      - owned by the caller, not stored in the library internally
    ->value.ports (for attr->type == SWITCH_TYPE_PORT)
      - must point to an array of at lest val->len * sizeof(struct switch_port)

  When getting string attributes, val->value.s must be freed by the caller
  When getting port list attributes, an internal static buffer is used,
  which changes from call to call.

 */

#ifndef __SWLIB_H
#define __SWLIB_H

enum swlib_attr_group {
	SWLIB_ATTR_GROUP_GLOBAL,
	SWLIB_ATTR_GROUP_VLAN,
	SWLIB_ATTR_GROUP_PORT,
};

enum swlib_port_flags {
	SWLIB_PORT_FLAG_TAGGED = (1 << 0),
};

enum swlib_link_flags {
	SWLIB_LINK_FLAG_EEE_100BASET = (1 << 0),
	SWLIB_LINK_FLAG_EEE_1000BASET = (1 << 1),
};

struct switch_dev;
struct switch_attr;
struct switch_port;
struct switch_port_map;
struct switch_port_link;
struct switch_val;
struct uci_package;

struct switch_dev {
	int id;
	char dev_name[IFNAMSIZ];
	char *name;
	char *alias;
	int ports;
	int vlans;
	int cpu_port;
	struct switch_attr *ops;
	struct switch_attr *port_ops;
	struct switch_attr *vlan_ops;
	struct switch_portmap *maps;
	struct switch_dev *next;
	void *priv;
};

struct switch_val {
	struct switch_attr *attr;
	int len;
	int err;
	int port_vlan;
	union {
		char *s;
		int i;
		struct switch_port *ports;
		struct switch_port_link *link;
	} value;
};

struct switch_attr {
	struct switch_dev *dev;
	int atype;
	int id;
	int type;
	char *name;
	char *description;
	struct switch_attr *next;
};

struct switch_port {
	unsigned int id;
	unsigned int flags;
};

struct switch_portmap {
	unsigned int virt;
	char *segment;
};

struct switch_port_link {
	int link:1;
	int duplex:1;
	int aneg:1;
	int tx_flow:1;
	int rx_flow:1;
	int speed;
	/* in ethtool adv_t format */
	uint32_t eee;
};

/**
 * swlib_list: list all switches
 */
void swlib_list(void);

/**
 * swlib_print_portmap: get portmap
 * @dev: switch device struct
 */
void swlib_print_portmap(struct switch_dev *dev, char *segment);

/**
 * swlib_connect: connect to the switch through netlink
 * @name: name of the ethernet interface,
 *
 * if name is NULL, it connect and builds a chain of all switches
 */
struct switch_dev *swlib_connect(const char *name);

/**
 * swlib_free: free all dynamically allocated data for the switch connection
 * @dev: switch device struct
 *
 * all members of a switch device chain (generated by swlib_connect(NULL))
 * must be freed individually
 */
void swlib_free(struct switch_dev *dev);

/**
 * swlib_free_all: run swlib_free on all devices in the chain
 * @dev: switch device struct
 */
void swlib_free_all(struct switch_dev *dev);

/**
 * swlib_scan: probe the switch driver for available commands/attributes
 * @dev: switch device struct
 */
int swlib_scan(struct switch_dev *dev);

/**
 * swlib_lookup_attr: look up a switch attribute
 * @dev: switch device struct
 * @type: global, port or vlan
 * @name: name of the attribute
 */
struct switch_attr *swlib_lookup_attr(struct switch_dev *dev,
		enum swlib_attr_group atype, const char *name);

/**
 * swlib_set_attr: set the value for an attribute
 * @dev: switch device struct
 * @attr: switch attribute struct
 * @val: attribute value pointer
 * returns 0 on success
 */
int swlib_set_attr(struct switch_dev *dev, struct switch_attr *attr,
		struct switch_val *val);

/**
 * swlib_set_attr_string: set the value for an attribute with type conversion
 * @dev: switch device struct
 * @attr: switch attribute struct
 * @port_vlan: port or vlan (if applicable)
 * @str: string value
 * returns 0 on success
 */
int swlib_set_attr_string(struct switch_dev *dev, struct switch_attr *attr,
		int port_vlan, const char *str);

/**
 * swlib_get_attr: get the value for an attribute
 * @dev: switch device struct
 * @attr: switch attribute struct
 * @val: attribute value pointer
 * returns 0 on success
 * for string attributes, the result string must be freed by the caller
 */
int swlib_get_attr(struct switch_dev *dev, struct switch_attr *attr,
		struct switch_val *val);

/**
 * swlib_apply_from_uci: set up the switch from a uci configuration
 * @dev: switch device struct
 * @p: uci package which contains the desired global config
 */
int swlib_apply_from_uci(struct switch_dev *dev, struct uci_package *p);

#endif
