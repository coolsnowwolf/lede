// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2018 MediaTek Inc.
 * Author: Sirui Zhao <Sirui.Zhao@mediatek.com>
 */

#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <net/genetlink.h>

#include "mt753x.h"
#include "mt753x_nl.h"

struct mt753x_nl_cmd_item {
	enum mt753x_cmd cmd;
	bool require_dev;
	int (*process)(struct genl_info *info, struct gsw_mt753x *gsw);
	u32 nr_required_attrs;
	const enum mt753x_attr *required_attrs;
};

static int mt753x_nl_response(struct sk_buff *skb, struct genl_info *info);

static const struct nla_policy mt753x_nl_cmd_policy[] = {
	[MT753X_ATTR_TYPE_MESG] = { .type = NLA_STRING },
	[MT753X_ATTR_TYPE_PHY] = { .type = NLA_S32 },
	[MT753X_ATTR_TYPE_REG] = { .type = NLA_S32 },
	[MT753X_ATTR_TYPE_VAL] = { .type = NLA_S32 },
	[MT753X_ATTR_TYPE_DEV_NAME] = { .type = NLA_S32 },
	[MT753X_ATTR_TYPE_DEV_ID] = { .type = NLA_S32 },
	[MT753X_ATTR_TYPE_DEVAD] = { .type = NLA_S32 },
};

static const struct genl_ops mt753x_nl_ops[] = {
	{
		.cmd = MT753X_CMD_REQUEST,
		.doit = mt753x_nl_response,
		.policy = mt753x_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MT753X_CMD_READ,
		.doit = mt753x_nl_response,
		.policy = mt753x_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	}, {
		.cmd = MT753X_CMD_WRITE,
		.doit = mt753x_nl_response,
		.policy = mt753x_nl_cmd_policy,
		.flags = GENL_ADMIN_PERM,
	},
};

static struct genl_family mt753x_nl_family = {
	.name =		MT753X_GENL_NAME,
	.version =	MT753X_GENL_VERSION,
	.maxattr =	MT753X_NR_ATTR_TYPE,
	.ops =		mt753x_nl_ops,
	.n_ops =	ARRAY_SIZE(mt753x_nl_ops),
};

static int mt753x_nl_list_devs(char *buff, int size)
{
	struct gsw_mt753x *gsw;
	int len, total = 0;
	char buf[80];

	memset(buff, 0, size);

	mt753x_lock_gsw();

	list_for_each_entry(gsw, &mt753x_devs, list) {
		len = snprintf(buf, sizeof(buf),
			       "id: %d, model: %s, node: %s\n",
			       gsw->id, gsw->name, gsw->dev->of_node->name);
		strncat(buff, buf, size - total);
		total += len;
	}

	mt753x_put_gsw();

	return total;
}

static int mt753x_nl_prepare_reply(struct genl_info *info, u8 cmd,
				   struct sk_buff **skbp)
{
	struct sk_buff *msg;
	void *reply;

	if (!info)
		return -EINVAL;

	msg = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
	if (!msg)
		return -ENOMEM;

	/* Construct send-back message header */
	reply = genlmsg_put(msg, info->snd_portid, info->snd_seq,
			    &mt753x_nl_family, 0, cmd);
	if (!reply) {
		nlmsg_free(msg);
		return -EINVAL;
	}

	*skbp = msg;
	return 0;
}

static int mt753x_nl_send_reply(struct sk_buff *skb, struct genl_info *info)
{
	struct genlmsghdr *genlhdr = nlmsg_data(nlmsg_hdr(skb));
	void *reply = genlmsg_data(genlhdr);

	/* Finalize a generic netlink message (update message header) */
	genlmsg_end(skb, reply);

	/* reply to a request */
	return genlmsg_reply(skb, info);
}

static s32 mt753x_nl_get_s32(struct genl_info *info, enum mt753x_attr attr,
			     s32 defval)
{
	struct nlattr *na;

	na = info->attrs[attr];
	if (na)
		return nla_get_s32(na);

	return defval;
}

static int mt753x_nl_get_u32(struct genl_info *info, enum mt753x_attr attr,
			     u32 *val)
{
	struct nlattr *na;

	na = info->attrs[attr];
	if (na) {
		*val = nla_get_u32(na);
		return 0;
	}

	return -1;
}

static struct gsw_mt753x *mt753x_nl_parse_find_gsw(struct genl_info *info)
{
	struct gsw_mt753x *gsw;
	struct nlattr *na;
	int gsw_id;

	na = info->attrs[MT753X_ATTR_TYPE_DEV_ID];
	if (na) {
		gsw_id = nla_get_s32(na);
		if (gsw_id >= 0)
			gsw = mt753x_get_gsw(gsw_id);
		else
			gsw = mt753x_get_first_gsw();
	} else {
		gsw = mt753x_get_first_gsw();
	}

	return gsw;
}

static int mt753x_nl_get_swdevs(struct genl_info *info, struct gsw_mt753x *gsw)
{
	struct sk_buff *rep_skb = NULL;
	char dev_info[512];
	int ret;

	ret = mt753x_nl_list_devs(dev_info, sizeof(dev_info));
	if (!ret) {
		pr_info("No switch registered\n");
		return -EINVAL;
	}

	ret = mt753x_nl_prepare_reply(info, MT753X_CMD_REPLY, &rep_skb);
	if (ret < 0)
		goto err;

	ret = nla_put_string(rep_skb, MT753X_ATTR_TYPE_MESG, dev_info);
	if (ret < 0)
		goto err;

	return mt753x_nl_send_reply(rep_skb, info);

err:
	if (rep_skb)
		nlmsg_free(rep_skb);

	return ret;
}

static int mt753x_nl_reply_read(struct genl_info *info, struct gsw_mt753x *gsw)
{
	struct sk_buff *rep_skb = NULL;
	s32 phy, devad, reg;
	int value;
	int ret = 0;

	phy = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_PHY, -1);
	devad = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_DEVAD, -1);
	reg = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_REG, -1);

	if (reg < 0)
		goto err;

	ret = mt753x_nl_prepare_reply(info, MT753X_CMD_READ, &rep_skb);
	if (ret < 0)
		goto err;

	if (phy >= 0) {
		if (devad < 0)
			value = gsw->mii_read(gsw, phy, reg);
		else
			value = gsw->mmd_read(gsw, phy, devad, reg);
	} else {
		value = mt753x_reg_read(gsw, reg);
	}

	ret = nla_put_s32(rep_skb, MT753X_ATTR_TYPE_REG, reg);
	if (ret < 0)
		goto err;

	ret = nla_put_s32(rep_skb, MT753X_ATTR_TYPE_VAL, value);
	if (ret < 0)
		goto err;

	return mt753x_nl_send_reply(rep_skb, info);

err:
	if (rep_skb)
		nlmsg_free(rep_skb);

	return ret;
}

static int mt753x_nl_reply_write(struct genl_info *info, struct gsw_mt753x *gsw)
{
	struct sk_buff *rep_skb = NULL;
	s32 phy, devad, reg;
	u32 value;
	int ret = 0;

	phy = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_PHY, -1);
	devad = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_DEVAD, -1);
	reg = mt753x_nl_get_s32(info, MT753X_ATTR_TYPE_REG, -1);

	if (mt753x_nl_get_u32(info, MT753X_ATTR_TYPE_VAL, &value))
		goto err;

	if (reg < 0)
		goto err;

	ret = mt753x_nl_prepare_reply(info, MT753X_CMD_WRITE, &rep_skb);
	if (ret < 0)
		goto err;

	if (phy >= 0) {
		if (devad < 0)
			gsw->mii_write(gsw, phy, reg, value);
		else
			gsw->mmd_write(gsw, phy, devad, reg, value);
	} else {
		mt753x_reg_write(gsw, reg, value);
	}

	ret = nla_put_s32(rep_skb, MT753X_ATTR_TYPE_REG, reg);
	if (ret < 0)
		goto err;

	ret = nla_put_s32(rep_skb, MT753X_ATTR_TYPE_VAL, value);
	if (ret < 0)
		goto err;

	return mt753x_nl_send_reply(rep_skb, info);

err:
	if (rep_skb)
		nlmsg_free(rep_skb);

	return ret;
}

static const enum mt753x_attr mt753x_nl_cmd_read_attrs[] = {
	MT753X_ATTR_TYPE_REG
};

static const enum mt753x_attr mt753x_nl_cmd_write_attrs[] = {
	MT753X_ATTR_TYPE_REG,
	MT753X_ATTR_TYPE_VAL
};

static const struct mt753x_nl_cmd_item mt753x_nl_cmds[] = {
	{
		.cmd = MT753X_CMD_REQUEST,
		.require_dev = false,
		.process = mt753x_nl_get_swdevs
	}, {
		.cmd = MT753X_CMD_READ,
		.require_dev = true,
		.process = mt753x_nl_reply_read,
		.required_attrs = mt753x_nl_cmd_read_attrs,
		.nr_required_attrs = ARRAY_SIZE(mt753x_nl_cmd_read_attrs),
	}, {
		.cmd = MT753X_CMD_WRITE,
		.require_dev = true,
		.process = mt753x_nl_reply_write,
		.required_attrs = mt753x_nl_cmd_write_attrs,
		.nr_required_attrs = ARRAY_SIZE(mt753x_nl_cmd_write_attrs),
	}
};

static int mt753x_nl_response(struct sk_buff *skb, struct genl_info *info)
{
	struct genlmsghdr *hdr = nlmsg_data(info->nlhdr);
	const struct mt753x_nl_cmd_item *cmditem = NULL;
	struct gsw_mt753x *gsw = NULL;
	u32 sat_req_attrs = 0;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(mt753x_nl_cmds); i++) {
		if (hdr->cmd == mt753x_nl_cmds[i].cmd) {
			cmditem = &mt753x_nl_cmds[i];
			break;
		}
	}

	if (!cmditem) {
		pr_info("mt753x-nl: unknown cmd %u\n", hdr->cmd);
		return -EINVAL;
	}

	for (i = 0; i < cmditem->nr_required_attrs; i++) {
		if (info->attrs[cmditem->required_attrs[i]])
			sat_req_attrs++;
	}

	if (sat_req_attrs != cmditem->nr_required_attrs) {
		pr_info("mt753x-nl: missing required attr(s) for cmd %u\n",
			hdr->cmd);
		return -EINVAL;
	}

	if (cmditem->require_dev) {
		gsw = mt753x_nl_parse_find_gsw(info);
		if (!gsw) {
			pr_info("mt753x-nl: failed to find switch dev\n");
			return -EINVAL;
		}
	}

	ret = cmditem->process(info, gsw);

	mt753x_put_gsw();

	return ret;
}

int __init mt753x_nl_init(void)
{
	int ret;

	ret = genl_register_family(&mt753x_nl_family);
	if (ret) {
		pr_info("mt753x-nl: genl_register_family_with_ops failed\n");
		return ret;
	}

	return 0;
}

void __exit mt753x_nl_exit(void)
{
	genl_unregister_family(&mt753x_nl_family);
}
