/*
 * uci.c: UCI binding for the switch configuration utility
 *
 * Copyright (C) 2009 Felix Fietkau <nbd@nbd.name>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundatio.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <stdint.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <uci.h>

#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/ctrl.h>
#include <linux/switch.h>
#include "swlib.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

struct swlib_setting {
	struct switch_attr *attr;
	const char *name;
	int port_vlan;
	const char *val;
	struct swlib_setting *next;
};

struct swlib_setting early_settings[] = {
	{ .name = "reset", .val = "1" },
	{ .name = "enable_vlan", .val = "1" },
};

static struct swlib_setting *settings;
static struct swlib_setting **head;

static bool swlib_match_name(struct switch_dev *dev, const char *name)
{
	return (strcmp(name, dev->dev_name) == 0 ||
		strcmp(name, dev->alias) == 0);
}

static void
swlib_map_settings(struct switch_dev *dev, int type, int port_vlan, struct uci_section *s)
{
	struct swlib_setting *setting;
	struct switch_attr *attr;
	struct uci_element *e;
	struct uci_option *o;

	uci_foreach_element(&s->options, e) {
		o = uci_to_option(e);

		if (o->type != UCI_TYPE_STRING)
			continue;

		if (!strcmp(e->name, "device"))
			continue;

		/* map early settings */
		if (type == SWLIB_ATTR_GROUP_GLOBAL) {
			int i;

			for (i = 0; i < ARRAY_SIZE(early_settings); i++) {
				if (strcmp(e->name, early_settings[i].name) != 0)
					continue;

				early_settings[i].val = o->v.string;
				goto skip;
			}
		}

		attr = swlib_lookup_attr(dev, type, e->name);
		if (!attr)
			continue;

		setting = malloc(sizeof(struct swlib_setting));
		memset(setting, 0, sizeof(struct swlib_setting));
		setting->attr = attr;
		setting->port_vlan = port_vlan;
		setting->val = o->v.string;
		*head = setting;
		head = &setting->next;
skip:
		continue;
	}
}

int swlib_apply_from_uci(struct switch_dev *dev, struct uci_package *p)
{
	struct switch_attr *attr;
	struct uci_element *e;
	struct uci_section *s;
	struct uci_option *o;
	struct uci_ptr ptr;
	struct switch_val val;
	int i;

	settings = NULL;
	head = &settings;

	uci_foreach_element(&p->sections, e) {
		struct uci_element *n;

		s = uci_to_section(e);

		if (strcmp(s->type, "switch") != 0)
			continue;

		uci_foreach_element(&s->options, n) {
			struct uci_option *o = uci_to_option(n);

			if (strcmp(n->name, "name") != 0)
				continue;

			if (o->type != UCI_TYPE_STRING)
				continue;

			if (swlib_match_name(dev, o->v.string))
				goto found;

			break;
		}

		if (!swlib_match_name(dev, e->name))
			continue;

		goto found;
	}

	/* not found */
	return -1;

found:
	/* look up available early options, which need to be taken care
	 * of in the correct order */
	for (i = 0; i < ARRAY_SIZE(early_settings); i++) {
		early_settings[i].attr = swlib_lookup_attr(dev,
			SWLIB_ATTR_GROUP_GLOBAL, early_settings[i].name);
	}
	swlib_map_settings(dev, SWLIB_ATTR_GROUP_GLOBAL, 0, s);

	/* look for port or vlan sections */
	uci_foreach_element(&p->sections, e) {
		struct uci_element *os;
		s = uci_to_section(e);

		if (!strcmp(s->type, "switch_port")) {
			char *devn = NULL, *port = NULL, *port_err = NULL;
			int port_n;

			uci_foreach_element(&s->options, os) {
				o = uci_to_option(os);
				if (o->type != UCI_TYPE_STRING)
					continue;

				if (!strcmp(os->name, "device")) {
					devn = o->v.string;
					if (!swlib_match_name(dev, devn))
						devn = NULL;
				} else if (!strcmp(os->name, "port")) {
					port = o->v.string;
				}
			}
			if (!devn || !port || !port[0])
				continue;

			port_n = strtoul(port, &port_err, 0);
			if (port_err && port_err[0])
				continue;

			swlib_map_settings(dev, SWLIB_ATTR_GROUP_PORT, port_n, s);
		} else if (!strcmp(s->type, "switch_vlan")) {
			char *devn = NULL, *vlan = NULL, *vlan_err = NULL;
			int vlan_n;

			uci_foreach_element(&s->options, os) {
				o = uci_to_option(os);
				if (o->type != UCI_TYPE_STRING)
					continue;

				if (!strcmp(os->name, "device")) {
					devn = o->v.string;
					if (!swlib_match_name(dev, devn))
						devn = NULL;
				} else if (!strcmp(os->name, "vlan")) {
					vlan = o->v.string;
				}
			}
			if (!devn || !vlan || !vlan[0])
				continue;

			vlan_n = strtoul(vlan, &vlan_err, 0);
			if (vlan_err && vlan_err[0])
				continue;

			swlib_map_settings(dev, SWLIB_ATTR_GROUP_VLAN, vlan_n, s);
		}
	}

	for (i = 0; i < ARRAY_SIZE(early_settings); i++) {
		struct swlib_setting *st = &early_settings[i];
		if (!st->attr || !st->val)
			continue;
		swlib_set_attr_string(dev, st->attr, st->port_vlan, st->val);

	}

	while (settings) {
		struct swlib_setting *st = settings;

		swlib_set_attr_string(dev, st->attr, st->port_vlan, st->val);
		st = st->next;
		free(settings);
		settings = st;
	}

	/* Apply the config */
	attr = swlib_lookup_attr(dev, SWLIB_ATTR_GROUP_GLOBAL, "apply");
	if (!attr)
		return 0;

	memset(&val, 0, sizeof(val));
	swlib_set_attr(dev, attr, &val);

	return 0;
}
