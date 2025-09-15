/*
 * swconfig.c: Switch configuration API
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

#include <linux/types.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/capability.h>
#include <linux/skbuff.h>
#include <linux/switch.h>
#include <linux/of.h>
#include <linux/version.h>
#include <uapi/linux/mii.h>

#define SWCONFIG_DEVNAME	"switch%d"

#include "swconfig_leds.c"

MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_LICENSE("GPL");

static int swdev_id;
static struct list_head swdevs;
static DEFINE_MUTEX(swdevs_lock);
struct swconfig_callback;

struct swconfig_callback {
	struct sk_buff *msg;
	struct genlmsghdr *hdr;
	struct genl_info *info;
	int cmd;

	/* callback for filling in the message data */
	int (*fill)(struct swconfig_callback *cb, void *arg);

	/* callback for closing the message before sending it */
	int (*close)(struct swconfig_callback *cb, void *arg);

	struct nlattr *nest[4];
	int args[4];
};

/* defaults */

static int
swconfig_get_vlan_ports(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	int ret;
	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	if (!dev->ops->get_vlan_ports)
		return -EOPNOTSUPP;

	ret = dev->ops->get_vlan_ports(dev, val);
	return ret;
}

static int
swconfig_set_vlan_ports(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	struct switch_port *ports = val->value.ports;
	const struct switch_dev_ops *ops = dev->ops;
	int i;

	if (val->port_vlan >= dev->vlans)
		return -EINVAL;

	/* validate ports */
	if (val->len > dev->ports)
		return -EINVAL;

	if (!ops->set_vlan_ports)
		return -EOPNOTSUPP;

	for (i = 0; i < val->len; i++) {
		if (ports[i].id >= dev->ports)
			return -EINVAL;

		if (ops->set_port_pvid &&
		    !(ports[i].flags & (1 << SWITCH_PORT_FLAG_TAGGED)))
			ops->set_port_pvid(dev, ports[i].id, val->port_vlan);
	}

	return ops->set_vlan_ports(dev, val);
}

static int
swconfig_set_pvid(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	if (val->port_vlan >= dev->ports)
		return -EINVAL;

	if (!dev->ops->set_port_pvid)
		return -EOPNOTSUPP;

	return dev->ops->set_port_pvid(dev, val->port_vlan, val->value.i);
}

static int
swconfig_get_pvid(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	if (val->port_vlan >= dev->ports)
		return -EINVAL;

	if (!dev->ops->get_port_pvid)
		return -EOPNOTSUPP;

	return dev->ops->get_port_pvid(dev, val->port_vlan, &val->value.i);
}

static int
swconfig_set_link(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	if (!dev->ops->set_port_link)
		return -EOPNOTSUPP;

	return dev->ops->set_port_link(dev, val->port_vlan, val->value.link);
}

static int
swconfig_get_link(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	struct switch_port_link *link = val->value.link;

	if (val->port_vlan >= dev->ports)
		return -EINVAL;

	if (!dev->ops->get_port_link)
		return -EOPNOTSUPP;

	memset(link, 0, sizeof(*link));
	return dev->ops->get_port_link(dev, val->port_vlan, link);
}

static int
swconfig_apply_config(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	/* don't complain if not supported by the switch driver */
	if (!dev->ops->apply_config)
		return 0;

	return dev->ops->apply_config(dev);
}

static int
swconfig_reset_switch(struct switch_dev *dev, const struct switch_attr *attr,
			struct switch_val *val)
{
	/* don't complain if not supported by the switch driver */
	if (!dev->ops->reset_switch)
		return 0;

	return dev->ops->reset_switch(dev);
}

enum global_defaults {
	GLOBAL_APPLY,
	GLOBAL_RESET,
};

enum vlan_defaults {
	VLAN_PORTS,
};

enum port_defaults {
	PORT_PVID,
	PORT_LINK,
};

static struct switch_attr default_global[] = {
	[GLOBAL_APPLY] = {
		.type = SWITCH_TYPE_NOVAL,
		.name = "apply",
		.description = "Activate changes in the hardware",
		.set = swconfig_apply_config,
	},
	[GLOBAL_RESET] = {
		.type = SWITCH_TYPE_NOVAL,
		.name = "reset",
		.description = "Reset the switch",
		.set = swconfig_reset_switch,
	}
};

static struct switch_attr default_port[] = {
	[PORT_PVID] = {
		.type = SWITCH_TYPE_INT,
		.name = "pvid",
		.description = "Primary VLAN ID",
		.set = swconfig_set_pvid,
		.get = swconfig_get_pvid,
	},
	[PORT_LINK] = {
		.type = SWITCH_TYPE_LINK,
		.name = "link",
		.description = "Get port link information",
		.set = swconfig_set_link,
		.get = swconfig_get_link,
	}
};

static struct switch_attr default_vlan[] = {
	[VLAN_PORTS] = {
		.type = SWITCH_TYPE_PORTS,
		.name = "ports",
		.description = "VLAN port mapping",
		.set = swconfig_set_vlan_ports,
		.get = swconfig_get_vlan_ports,
	},
};

static const struct switch_attr *
swconfig_find_attr_by_name(const struct switch_attrlist *alist,
				const char *name)
{
	int i;

	for (i = 0; i < alist->n_attr; i++)
		if (strcmp(name, alist->attr[i].name) == 0)
			return &alist->attr[i];

	return NULL;
}

static void swconfig_defaults_init(struct switch_dev *dev)
{
	const struct switch_dev_ops *ops = dev->ops;

	dev->def_global = 0;
	dev->def_vlan = 0;
	dev->def_port = 0;

	if (ops->get_vlan_ports || ops->set_vlan_ports)
		set_bit(VLAN_PORTS, &dev->def_vlan);

	if (ops->get_port_pvid || ops->set_port_pvid)
		set_bit(PORT_PVID, &dev->def_port);

	if (ops->get_port_link &&
	    !swconfig_find_attr_by_name(&ops->attr_port, "link"))
		set_bit(PORT_LINK, &dev->def_port);

	/* always present, can be no-op */
	set_bit(GLOBAL_APPLY, &dev->def_global);
	set_bit(GLOBAL_RESET, &dev->def_global);
}


static struct genl_family switch_fam;

static const struct nla_policy switch_policy[SWITCH_ATTR_MAX+1] = {
	[SWITCH_ATTR_ID] = { .type = NLA_U32 },
	[SWITCH_ATTR_OP_ID] = { .type = NLA_U32 },
	[SWITCH_ATTR_OP_PORT] = { .type = NLA_U32 },
	[SWITCH_ATTR_OP_VLAN] = { .type = NLA_U32 },
	[SWITCH_ATTR_OP_VALUE_INT] = { .type = NLA_U32 },
	[SWITCH_ATTR_OP_VALUE_STR] = { .type = NLA_NUL_STRING },
	[SWITCH_ATTR_OP_VALUE_PORTS] = { .type = NLA_NESTED },
	[SWITCH_ATTR_TYPE] = { .type = NLA_U32 },
};

static const struct nla_policy port_policy[SWITCH_PORT_ATTR_MAX+1] = {
	[SWITCH_PORT_ID] = { .type = NLA_U32 },
	[SWITCH_PORT_FLAG_TAGGED] = { .type = NLA_FLAG },
};

static struct nla_policy link_policy[SWITCH_LINK_ATTR_MAX] = {
	[SWITCH_LINK_FLAG_DUPLEX] = { .type = NLA_FLAG },
	[SWITCH_LINK_FLAG_ANEG] = { .type = NLA_FLAG },
	[SWITCH_LINK_SPEED] = { .type = NLA_U32 },
};

static inline void
swconfig_lock(void)
{
	mutex_lock(&swdevs_lock);
}

static inline void
swconfig_unlock(void)
{
	mutex_unlock(&swdevs_lock);
}

static struct switch_dev *
swconfig_get_dev(struct genl_info *info)
{
	struct switch_dev *dev = NULL;
	struct switch_dev *p;
	int id;

	if (!info->attrs[SWITCH_ATTR_ID])
		goto done;

	id = nla_get_u32(info->attrs[SWITCH_ATTR_ID]);
	swconfig_lock();
	list_for_each_entry(p, &swdevs, dev_list) {
		if (id != p->id)
			continue;

		dev = p;
		break;
	}
	if (dev)
		mutex_lock(&dev->sw_mutex);
	else
		pr_debug("device %d not found\n", id);
	swconfig_unlock();
done:
	return dev;
}

static inline void
swconfig_put_dev(struct switch_dev *dev)
{
	mutex_unlock(&dev->sw_mutex);
}

static int
swconfig_dump_attr(struct swconfig_callback *cb, void *arg)
{
	struct switch_attr *op = arg;
	struct genl_info *info = cb->info;
	struct sk_buff *msg = cb->msg;
	int id = cb->args[0];
	void *hdr;

	hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &switch_fam,
			NLM_F_MULTI, SWITCH_CMD_NEW_ATTR);
	if (IS_ERR(hdr))
		return -1;

	if (nla_put_u32(msg, SWITCH_ATTR_OP_ID, id))
		goto nla_put_failure;
	if (nla_put_u32(msg, SWITCH_ATTR_OP_TYPE, op->type))
		goto nla_put_failure;
	if (nla_put_string(msg, SWITCH_ATTR_OP_NAME, op->name))
		goto nla_put_failure;
	if (op->description)
		if (nla_put_string(msg, SWITCH_ATTR_OP_DESCRIPTION,
			op->description))
			goto nla_put_failure;

	genlmsg_end(msg, hdr);
	return msg->len;
nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

/* spread multipart messages across multiple message buffers */
static int
swconfig_send_multipart(struct swconfig_callback *cb, void *arg)
{
	struct genl_info *info = cb->info;
	int restart = 0;
	int err;

	do {
		if (!cb->msg) {
			cb->msg = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
			if (cb->msg == NULL)
				goto error;
		}

		if (!(cb->fill(cb, arg) < 0))
			break;

		/* fill failed, check if this was already the second attempt */
		if (restart)
			goto error;

		/* try again in a new message, send the current one */
		restart = 1;
		if (cb->close) {
			if (cb->close(cb, arg) < 0)
				goto error;
		}
		err = genlmsg_reply(cb->msg, info);
		cb->msg = NULL;
		if (err < 0)
			goto error;

	} while (restart);

	return 0;

error:
	if (cb->msg)
		nlmsg_free(cb->msg);
	return -1;
}

static int
swconfig_list_attrs(struct sk_buff *skb, struct genl_info *info)
{
	struct genlmsghdr *hdr = nlmsg_data(info->nlhdr);
	const struct switch_attrlist *alist;
	struct switch_dev *dev;
	struct swconfig_callback cb;
	int err = -EINVAL;
	int i;

	/* defaults */
	struct switch_attr *def_list;
	unsigned long *def_active;
	int n_def;

	dev = swconfig_get_dev(info);
	if (!dev)
		return -EINVAL;

	switch (hdr->cmd) {
	case SWITCH_CMD_LIST_GLOBAL:
		alist = &dev->ops->attr_global;
		def_list = default_global;
		def_active = &dev->def_global;
		n_def = ARRAY_SIZE(default_global);
		break;
	case SWITCH_CMD_LIST_VLAN:
		alist = &dev->ops->attr_vlan;
		def_list = default_vlan;
		def_active = &dev->def_vlan;
		n_def = ARRAY_SIZE(default_vlan);
		break;
	case SWITCH_CMD_LIST_PORT:
		alist = &dev->ops->attr_port;
		def_list = default_port;
		def_active = &dev->def_port;
		n_def = ARRAY_SIZE(default_port);
		break;
	default:
		WARN_ON(1);
		goto out;
	}

	memset(&cb, 0, sizeof(cb));
	cb.info = info;
	cb.fill = swconfig_dump_attr;
	for (i = 0; i < alist->n_attr; i++) {
		if (alist->attr[i].disabled)
			continue;
		cb.args[0] = i;
		err = swconfig_send_multipart(&cb, (void *) &alist->attr[i]);
		if (err < 0)
			goto error;
	}

	/* defaults */
	for (i = 0; i < n_def; i++) {
		if (!test_bit(i, def_active))
			continue;
		cb.args[0] = SWITCH_ATTR_DEFAULTS_OFFSET + i;
		err = swconfig_send_multipart(&cb, (void *) &def_list[i]);
		if (err < 0)
			goto error;
	}
	swconfig_put_dev(dev);

	if (!cb.msg)
		return 0;

	return genlmsg_reply(cb.msg, info);

error:
	if (cb.msg)
		nlmsg_free(cb.msg);
out:
	swconfig_put_dev(dev);
	return err;
}

static const struct switch_attr *
swconfig_lookup_attr(struct switch_dev *dev, struct genl_info *info,
		struct switch_val *val)
{
	struct genlmsghdr *hdr = nlmsg_data(info->nlhdr);
	const struct switch_attrlist *alist;
	const struct switch_attr *attr = NULL;
	unsigned int attr_id;

	/* defaults */
	struct switch_attr *def_list;
	unsigned long *def_active;
	int n_def;

	if (!info->attrs[SWITCH_ATTR_OP_ID])
		goto done;

	switch (hdr->cmd) {
	case SWITCH_CMD_SET_GLOBAL:
	case SWITCH_CMD_GET_GLOBAL:
		alist = &dev->ops->attr_global;
		def_list = default_global;
		def_active = &dev->def_global;
		n_def = ARRAY_SIZE(default_global);
		break;
	case SWITCH_CMD_SET_VLAN:
	case SWITCH_CMD_GET_VLAN:
		alist = &dev->ops->attr_vlan;
		def_list = default_vlan;
		def_active = &dev->def_vlan;
		n_def = ARRAY_SIZE(default_vlan);
		if (!info->attrs[SWITCH_ATTR_OP_VLAN])
			goto done;
		val->port_vlan = nla_get_u32(info->attrs[SWITCH_ATTR_OP_VLAN]);
		if (val->port_vlan >= dev->vlans)
			goto done;
		break;
	case SWITCH_CMD_SET_PORT:
	case SWITCH_CMD_GET_PORT:
		alist = &dev->ops->attr_port;
		def_list = default_port;
		def_active = &dev->def_port;
		n_def = ARRAY_SIZE(default_port);
		if (!info->attrs[SWITCH_ATTR_OP_PORT])
			goto done;
		val->port_vlan = nla_get_u32(info->attrs[SWITCH_ATTR_OP_PORT]);
		if (val->port_vlan >= dev->ports)
			goto done;
		break;
	default:
		WARN_ON(1);
		goto done;
	}

	if (!alist)
		goto done;

	attr_id = nla_get_u32(info->attrs[SWITCH_ATTR_OP_ID]);
	if (attr_id >= SWITCH_ATTR_DEFAULTS_OFFSET) {
		attr_id -= SWITCH_ATTR_DEFAULTS_OFFSET;
		if (attr_id >= n_def)
			goto done;
		if (!test_bit(attr_id, def_active))
			goto done;
		attr = &def_list[attr_id];
	} else {
		if (attr_id >= alist->n_attr)
			goto done;
		attr = &alist->attr[attr_id];
	}

	if (attr->disabled)
		attr = NULL;

done:
	if (!attr)
		pr_debug("attribute lookup failed\n");
	val->attr = attr;
	return attr;
}

static int
swconfig_parse_ports(struct sk_buff *msg, struct nlattr *head,
		struct switch_val *val, int max)
{
	struct nlattr *nla;
	int rem;

	val->len = 0;
	nla_for_each_nested(nla, head, rem) {
		struct nlattr *tb[SWITCH_PORT_ATTR_MAX+1];
		struct switch_port *port;

		if (val->len >= max)
			return -EINVAL;

		port = &val->value.ports[val->len];

		if (nla_parse_nested_deprecated(tb, SWITCH_PORT_ATTR_MAX, nla,
				port_policy, NULL))
			return -EINVAL;

		if (!tb[SWITCH_PORT_ID])
			return -EINVAL;

		port->id = nla_get_u32(tb[SWITCH_PORT_ID]);
		if (tb[SWITCH_PORT_FLAG_TAGGED])
			port->flags |= (1 << SWITCH_PORT_FLAG_TAGGED);
		val->len++;
	}

	return 0;
}

static int
swconfig_parse_link(struct sk_buff *msg, struct nlattr *nla,
		    struct switch_port_link *link)
{
	struct nlattr *tb[SWITCH_LINK_ATTR_MAX + 1];

	if (nla_parse_nested_deprecated(tb, SWITCH_LINK_ATTR_MAX, nla, link_policy, NULL))
		return -EINVAL;

	link->duplex = !!tb[SWITCH_LINK_FLAG_DUPLEX];
	link->aneg = !!tb[SWITCH_LINK_FLAG_ANEG];
	link->speed = nla_get_u32(tb[SWITCH_LINK_SPEED]);

	return 0;
}

static int
swconfig_set_attr(struct sk_buff *skb, struct genl_info *info)
{
	const struct switch_attr *attr;
	struct switch_dev *dev;
	struct switch_val val;
	int err = -EINVAL;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	dev = swconfig_get_dev(info);
	if (!dev)
		return -EINVAL;

	memset(&val, 0, sizeof(val));
	attr = swconfig_lookup_attr(dev, info, &val);
	if (!attr || !attr->set)
		goto error;

	val.attr = attr;
	switch (attr->type) {
	case SWITCH_TYPE_NOVAL:
		break;
	case SWITCH_TYPE_INT:
		if (!info->attrs[SWITCH_ATTR_OP_VALUE_INT])
			goto error;
		val.value.i =
			nla_get_u32(info->attrs[SWITCH_ATTR_OP_VALUE_INT]);
		break;
	case SWITCH_TYPE_STRING:
		if (!info->attrs[SWITCH_ATTR_OP_VALUE_STR])
			goto error;
		val.value.s =
			nla_data(info->attrs[SWITCH_ATTR_OP_VALUE_STR]);
		break;
	case SWITCH_TYPE_PORTS:
		val.value.ports = dev->portbuf;
		memset(dev->portbuf, 0,
			sizeof(struct switch_port) * dev->ports);

		/* TODO: implement multipart? */
		if (info->attrs[SWITCH_ATTR_OP_VALUE_PORTS]) {
			err = swconfig_parse_ports(skb,
				info->attrs[SWITCH_ATTR_OP_VALUE_PORTS],
				&val, dev->ports);
			if (err < 0)
				goto error;
		} else {
			val.len = 0;
			err = 0;
		}
		break;
	case SWITCH_TYPE_LINK:
		val.value.link = &dev->linkbuf;
		memset(&dev->linkbuf, 0, sizeof(struct switch_port_link));

		if (info->attrs[SWITCH_ATTR_OP_VALUE_LINK]) {
			err = swconfig_parse_link(skb,
						  info->attrs[SWITCH_ATTR_OP_VALUE_LINK],
						  val.value.link);
			if (err < 0)
				goto error;
		} else {
			val.len = 0;
			err = 0;
		}
		break;
	default:
		goto error;
	}

	err = attr->set(dev, attr, &val);
error:
	swconfig_put_dev(dev);
	return err;
}

static int
swconfig_close_portlist(struct swconfig_callback *cb, void *arg)
{
	if (cb->nest[0])
		nla_nest_end(cb->msg, cb->nest[0]);
	return 0;
}

static int
swconfig_send_port(struct swconfig_callback *cb, void *arg)
{
	const struct switch_port *port = arg;
	struct nlattr *p = NULL;

	if (!cb->nest[0]) {
		cb->nest[0] = nla_nest_start(cb->msg, cb->cmd);
		if (!cb->nest[0])
			return -1;
	}

	p = nla_nest_start(cb->msg, SWITCH_ATTR_PORT);
	if (!p)
		goto error;

	if (nla_put_u32(cb->msg, SWITCH_PORT_ID, port->id))
		goto nla_put_failure;
	if (port->flags & (1 << SWITCH_PORT_FLAG_TAGGED)) {
		if (nla_put_flag(cb->msg, SWITCH_PORT_FLAG_TAGGED))
			goto nla_put_failure;
	}

	nla_nest_end(cb->msg, p);
	return 0;

nla_put_failure:
		nla_nest_cancel(cb->msg, p);
error:
	nla_nest_cancel(cb->msg, cb->nest[0]);
	return -1;
}

static int
swconfig_send_ports(struct sk_buff **msg, struct genl_info *info, int attr,
		const struct switch_val *val)
{
	struct swconfig_callback cb;
	int err = 0;
	int i;

	if (!val->value.ports)
		return -EINVAL;

	memset(&cb, 0, sizeof(cb));
	cb.cmd = attr;
	cb.msg = *msg;
	cb.info = info;
	cb.fill = swconfig_send_port;
	cb.close = swconfig_close_portlist;

	cb.nest[0] = nla_nest_start(cb.msg, cb.cmd);
	for (i = 0; i < val->len; i++) {
		err = swconfig_send_multipart(&cb, &val->value.ports[i]);
		if (err)
			goto done;
	}
	err = val->len;
	swconfig_close_portlist(&cb, NULL);
	*msg = cb.msg;

done:
	return err;
}

static int
swconfig_send_link(struct sk_buff *msg, struct genl_info *info, int attr,
		   const struct switch_port_link *link)
{
	struct nlattr *p = NULL;
	int err = 0;

	p = nla_nest_start(msg, attr);
	if (link->link) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_LINK))
			goto nla_put_failure;
	}
	if (link->duplex) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_DUPLEX))
			goto nla_put_failure;
	}
	if (link->aneg) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_ANEG))
			goto nla_put_failure;
	}
	if (link->tx_flow) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_TX_FLOW))
			goto nla_put_failure;
	}
	if (link->rx_flow) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_RX_FLOW))
			goto nla_put_failure;
	}
	if (nla_put_u32(msg, SWITCH_LINK_SPEED, link->speed))
		goto nla_put_failure;
	if (link->eee & ADVERTISED_100baseT_Full) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_EEE_100BASET))
			goto nla_put_failure;
	}
	if (link->eee & ADVERTISED_1000baseT_Full) {
		if (nla_put_flag(msg, SWITCH_LINK_FLAG_EEE_1000BASET))
			goto nla_put_failure;
	}
	nla_nest_end(msg, p);

	return err;

nla_put_failure:
	nla_nest_cancel(msg, p);
	return -1;
}

static int
swconfig_get_attr(struct sk_buff *skb, struct genl_info *info)
{
	struct genlmsghdr *hdr = nlmsg_data(info->nlhdr);
	const struct switch_attr *attr;
	struct switch_dev *dev;
	struct sk_buff *msg = NULL;
	struct switch_val val;
	int err = -EINVAL;
	int cmd = hdr->cmd;

	dev = swconfig_get_dev(info);
	if (!dev)
		return -EINVAL;

	memset(&val, 0, sizeof(val));
	attr = swconfig_lookup_attr(dev, info, &val);
	if (!attr || !attr->get)
		goto error;

	if (attr->type == SWITCH_TYPE_PORTS) {
		val.value.ports = dev->portbuf;
		memset(dev->portbuf, 0,
			sizeof(struct switch_port) * dev->ports);
	} else if (attr->type == SWITCH_TYPE_LINK) {
		val.value.link = &dev->linkbuf;
		memset(&dev->linkbuf, 0, sizeof(struct switch_port_link));
	}

	err = attr->get(dev, attr, &val);
	if (err)
		goto error;

	msg = nlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!msg)
		goto error;

	hdr = genlmsg_put(msg, info->snd_portid, info->snd_seq, &switch_fam,
			0, cmd);
	if (IS_ERR(hdr))
		goto nla_put_failure;

	switch (attr->type) {
	case SWITCH_TYPE_INT:
		if (nla_put_u32(msg, SWITCH_ATTR_OP_VALUE_INT, val.value.i))
			goto nla_put_failure;
		break;
	case SWITCH_TYPE_STRING:
		if (nla_put_string(msg, SWITCH_ATTR_OP_VALUE_STR, val.value.s))
			goto nla_put_failure;
		break;
	case SWITCH_TYPE_PORTS:
		err = swconfig_send_ports(&msg, info,
				SWITCH_ATTR_OP_VALUE_PORTS, &val);
		if (err < 0)
			goto nla_put_failure;
		break;
	case SWITCH_TYPE_LINK:
		err = swconfig_send_link(msg, info,
					 SWITCH_ATTR_OP_VALUE_LINK, val.value.link);
		if (err < 0)
			goto nla_put_failure;
		break;
	default:
		pr_debug("invalid type in attribute\n");
		err = -EINVAL;
		goto nla_put_failure;
	}
	genlmsg_end(msg, hdr);
	err = msg->len;
	if (err < 0)
		goto nla_put_failure;

	swconfig_put_dev(dev);
	return genlmsg_reply(msg, info);

nla_put_failure:
	if (msg)
		nlmsg_free(msg);
error:
	swconfig_put_dev(dev);
	if (!err)
		err = -ENOMEM;
	return err;
}

static int
swconfig_send_switch(struct sk_buff *msg, u32 pid, u32 seq, int flags,
		const struct switch_dev *dev)
{
	struct nlattr *p = NULL, *m = NULL;
	void *hdr;
	int i;

	hdr = genlmsg_put(msg, pid, seq, &switch_fam, flags,
			SWITCH_CMD_NEW_ATTR);
	if (IS_ERR(hdr))
		return -1;

	if (nla_put_u32(msg, SWITCH_ATTR_ID, dev->id))
		goto nla_put_failure;
	if (nla_put_string(msg, SWITCH_ATTR_DEV_NAME, dev->devname))
		goto nla_put_failure;
	if (nla_put_string(msg, SWITCH_ATTR_ALIAS, dev->alias))
		goto nla_put_failure;
	if (nla_put_string(msg, SWITCH_ATTR_NAME, dev->name))
		goto nla_put_failure;
	if (nla_put_u32(msg, SWITCH_ATTR_VLANS, dev->vlans))
		goto nla_put_failure;
	if (nla_put_u32(msg, SWITCH_ATTR_PORTS, dev->ports))
		goto nla_put_failure;
	if (nla_put_u32(msg, SWITCH_ATTR_CPU_PORT, dev->cpu_port))
		goto nla_put_failure;

	m = nla_nest_start(msg, SWITCH_ATTR_PORTMAP);
	if (!m)
		goto nla_put_failure;
	for (i = 0; i < dev->ports; i++) {
		p = nla_nest_start(msg, SWITCH_ATTR_PORTS);
		if (!p)
			continue;
		if (dev->portmap[i].s) {
			if (nla_put_string(msg, SWITCH_PORTMAP_SEGMENT,
						dev->portmap[i].s))
				goto nla_put_failure;
			if (nla_put_u32(msg, SWITCH_PORTMAP_VIRT,
						dev->portmap[i].virt))
				goto nla_put_failure;
		}
		nla_nest_end(msg, p);
	}
	nla_nest_end(msg, m);
	genlmsg_end(msg, hdr);
	return msg->len;
nla_put_failure:
	genlmsg_cancel(msg, hdr);
	return -EMSGSIZE;
}

static int swconfig_dump_switches(struct sk_buff *skb,
		struct netlink_callback *cb)
{
	struct switch_dev *dev;
	int start = cb->args[0];
	int idx = 0;

	swconfig_lock();
	list_for_each_entry(dev, &swdevs, dev_list) {
		if (++idx <= start)
			continue;
		if (swconfig_send_switch(skb, NETLINK_CB(cb->skb).portid,
				cb->nlh->nlmsg_seq, NLM_F_MULTI,
				dev) < 0)
			break;
	}
	swconfig_unlock();
	cb->args[0] = idx;

	return skb->len;
}

static int
swconfig_done(struct netlink_callback *cb)
{
	return 0;
}

static struct genl_ops swconfig_ops[] = {
	{
		.cmd = SWITCH_CMD_LIST_GLOBAL,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_list_attrs,
	},
	{
		.cmd = SWITCH_CMD_LIST_VLAN,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_list_attrs,
	},
	{
		.cmd = SWITCH_CMD_LIST_PORT,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_list_attrs,
	},
	{
		.cmd = SWITCH_CMD_GET_GLOBAL,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_get_attr,
	},
	{
		.cmd = SWITCH_CMD_GET_VLAN,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_get_attr,
	},
	{
		.cmd = SWITCH_CMD_GET_PORT,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.doit = swconfig_get_attr,
	},
	{
		.cmd = SWITCH_CMD_SET_GLOBAL,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_ADMIN_PERM,
		.doit = swconfig_set_attr,
	},
	{
		.cmd = SWITCH_CMD_SET_VLAN,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_ADMIN_PERM,
		.doit = swconfig_set_attr,
	},
	{
		.cmd = SWITCH_CMD_SET_PORT,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.flags = GENL_ADMIN_PERM,
		.doit = swconfig_set_attr,
	},
	{
		.cmd = SWITCH_CMD_GET_SWITCH,
		.validate = GENL_DONT_VALIDATE_STRICT | GENL_DONT_VALIDATE_DUMP,
		.dumpit = swconfig_dump_switches,
		.done = swconfig_done,
	}
};

static struct genl_family switch_fam = {
	.name = "switch",
	.hdrsize = 0,
	.version = 1,
	.maxattr = SWITCH_ATTR_MAX,
	.policy = switch_policy,
	.module = THIS_MODULE,
	.ops = swconfig_ops,
	.n_ops = ARRAY_SIZE(swconfig_ops),
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,0,0)
	.resv_start_op = SWITCH_CMD_SET_VLAN + 1,
#endif
};

#ifdef CONFIG_OF
static void
of_switch_load_portmap(struct switch_dev *dev)
{
	struct device_node *port;

	if (!dev->of_node)
		return;

	for_each_child_of_node(dev->of_node, port) {
		const __be32 *prop;
		const char *segment;
		int size, phys;

		if (!of_device_is_compatible(port, "swconfig,port"))
			continue;

		if (of_property_read_string(port, "swconfig,segment", &segment))
			continue;

		prop = of_get_property(port, "swconfig,portmap", &size);
		if (!prop)
			continue;

		if (size != (2 * sizeof(*prop))) {
			pr_err("%s: failed to parse port mapping\n",
					port->name);
			continue;
		}

		phys = be32_to_cpup(prop++);
		if ((phys < 0) | (phys >= dev->ports)) {
			pr_err("%s: physical port index out of range\n",
					port->name);
			continue;
		}

		dev->portmap[phys].s = kstrdup(segment, GFP_KERNEL);
		dev->portmap[phys].virt = be32_to_cpup(prop);
		pr_debug("Found port: %s, physical: %d, virtual: %d\n",
			segment, phys, dev->portmap[phys].virt);
	}
}
#endif

int
register_switch(struct switch_dev *dev, struct net_device *netdev)
{
	struct switch_dev *sdev;
	const int max_switches = 8 * sizeof(unsigned long);
	unsigned long in_use = 0;
	int err;
	int i;

	INIT_LIST_HEAD(&dev->dev_list);
	if (netdev) {
		dev->netdev = netdev;
		if (!dev->alias)
			dev->alias = netdev->name;
	}
	BUG_ON(!dev->alias);

	/* Make sure swdev_id doesn't overflow */
	if (swdev_id == INT_MAX) {
		return -ENOMEM;
	}

	if (dev->ports > 0) {
		dev->portbuf = kzalloc(sizeof(struct switch_port) *
				dev->ports, GFP_KERNEL);
		if (!dev->portbuf)
			return -ENOMEM;
		dev->portmap = kzalloc(sizeof(struct switch_portmap) *
				dev->ports, GFP_KERNEL);
		if (!dev->portmap) {
			kfree(dev->portbuf);
			return -ENOMEM;
		}
	}
	swconfig_defaults_init(dev);
	mutex_init(&dev->sw_mutex);
	swconfig_lock();
	dev->id = ++swdev_id;

	list_for_each_entry(sdev, &swdevs, dev_list) {
		if (!sscanf(sdev->devname, SWCONFIG_DEVNAME, &i))
			continue;
		if (i < 0 || i > max_switches)
			continue;

		set_bit(i, &in_use);
	}
	i = find_first_zero_bit(&in_use, max_switches);

	if (i == max_switches) {
		swconfig_unlock();
		return -ENFILE;
	}

#ifdef CONFIG_OF
	if (dev->ports)
		of_switch_load_portmap(dev);
#endif

	/* fill device name */
	snprintf(dev->devname, IFNAMSIZ, SWCONFIG_DEVNAME, i);

	list_add_tail(&dev->dev_list, &swdevs);
	swconfig_unlock();

	err = swconfig_create_led_trigger(dev);
	if (err)
		return err;

	return 0;
}
EXPORT_SYMBOL_GPL(register_switch);

void
unregister_switch(struct switch_dev *dev)
{
	swconfig_destroy_led_trigger(dev);
	kfree(dev->portbuf);
	mutex_lock(&dev->sw_mutex);
	swconfig_lock();
	list_del(&dev->dev_list);
	swconfig_unlock();
	mutex_unlock(&dev->sw_mutex);
}
EXPORT_SYMBOL_GPL(unregister_switch);

int
switch_generic_set_link(struct switch_dev *dev, int port,
			struct switch_port_link *link)
{
	if (WARN_ON(!dev->ops->phy_write16))
		return -ENOTSUPP;

	/* Generic implementation */
	if (link->aneg) {
		dev->ops->phy_write16(dev, port, MII_BMCR, 0x0000);
		dev->ops->phy_write16(dev, port, MII_BMCR, BMCR_ANENABLE | BMCR_ANRESTART);
	} else {
		u16 bmcr = 0;

		if (link->duplex)
			bmcr |= BMCR_FULLDPLX;

		switch (link->speed) {
		case SWITCH_PORT_SPEED_10:
			break;
		case SWITCH_PORT_SPEED_100:
			bmcr |= BMCR_SPEED100;
			break;
		case SWITCH_PORT_SPEED_1000:
			bmcr |= BMCR_SPEED1000;
			break;
		default:
			return -ENOTSUPP;
		}

		dev->ops->phy_write16(dev, port, MII_BMCR, bmcr);
	}

	return 0;
}
EXPORT_SYMBOL_GPL(switch_generic_set_link);

static int __init
swconfig_init(void)
{
	INIT_LIST_HEAD(&swdevs);

	return genl_register_family(&switch_fam);
}

static void __exit
swconfig_exit(void)
{
	genl_unregister_family(&switch_fam);
}

module_init(swconfig_init);
module_exit(swconfig_exit);
