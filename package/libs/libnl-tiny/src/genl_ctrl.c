/*
 * lib/genl/ctrl.c		Generic Netlink Controller
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup genl_mngt
 * @defgroup ctrl Controller
 * @brief
 *
 * @{
 */

#include <netlink-generic.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/ctrl.h>
#include <netlink/utils.h>

/** @cond SKIP */
#define CTRL_VERSION		0x0001

static struct nl_cache_ops genl_ctrl_ops;
/** @endcond */

static int ctrl_request_update(struct nl_cache *c, struct nl_sock *h)
{
	return genl_send_simple(h, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
				CTRL_VERSION, NLM_F_DUMP);
}

static struct nla_policy ctrl_policy[CTRL_ATTR_MAX+1] = {
	[CTRL_ATTR_FAMILY_ID]	= { .type = NLA_U16 },
	[CTRL_ATTR_FAMILY_NAME]	= { .type = NLA_STRING,
				    .maxlen = GENL_NAMSIZ },
	[CTRL_ATTR_VERSION]	= { .type = NLA_U32 },
	[CTRL_ATTR_HDRSIZE]	= { .type = NLA_U32 },
	[CTRL_ATTR_MAXATTR]	= { .type = NLA_U32 },
	[CTRL_ATTR_OPS]		= { .type = NLA_NESTED },
	[CTRL_ATTR_MCAST_GROUPS] = { .type = NLA_NESTED },
};

static struct nla_policy family_op_policy[CTRL_ATTR_OP_MAX+1] = {
	[CTRL_ATTR_OP_ID]	= { .type = NLA_U32 },
	[CTRL_ATTR_OP_FLAGS]	= { .type = NLA_U32 },
};

static struct nla_policy family_grp_policy[CTRL_ATTR_MCAST_GRP_MAX+1] = {
	[CTRL_ATTR_MCAST_GRP_NAME] = { .type = NLA_STRING },
	[CTRL_ATTR_MCAST_GRP_ID]   = { .type = NLA_U32 },
};

static int ctrl_msg_parser(struct nl_cache_ops *ops, struct genl_cmd *cmd,
			   struct genl_info *info, void *arg)
{
	struct genl_family *family;
	struct nl_parser_param *pp = arg;
	int err;

	family = genl_family_alloc();
	if (family == NULL) {
		err = -NLE_NOMEM;
		goto errout;
	}

	if (info->attrs[CTRL_ATTR_FAMILY_NAME] == NULL) {
		err = -NLE_MISSING_ATTR;
		goto errout;
	}

	if (info->attrs[CTRL_ATTR_FAMILY_ID] == NULL) {
		err = -NLE_MISSING_ATTR;
		goto errout;
	}

	family->ce_msgtype = info->nlh->nlmsg_type;
	genl_family_set_id(family,
			   nla_get_u16(info->attrs[CTRL_ATTR_FAMILY_ID]));
	genl_family_set_name(family,
		     nla_get_string(info->attrs[CTRL_ATTR_FAMILY_NAME]));

	if (info->attrs[CTRL_ATTR_VERSION]) {
		uint32_t version = nla_get_u32(info->attrs[CTRL_ATTR_VERSION]);
		genl_family_set_version(family, version);
	}

	if (info->attrs[CTRL_ATTR_HDRSIZE]) {
		uint32_t hdrsize = nla_get_u32(info->attrs[CTRL_ATTR_HDRSIZE]);
		genl_family_set_hdrsize(family, hdrsize);
	}

	if (info->attrs[CTRL_ATTR_MAXATTR]) {
		uint32_t maxattr = nla_get_u32(info->attrs[CTRL_ATTR_MAXATTR]);
		genl_family_set_maxattr(family, maxattr);
	}

	if (info->attrs[CTRL_ATTR_OPS]) {
		struct nlattr *nla, *nla_ops;
		int remaining;

		nla_ops = info->attrs[CTRL_ATTR_OPS];
		nla_for_each_nested(nla, nla_ops, remaining) {
			struct nlattr *tb[CTRL_ATTR_OP_MAX+1];
			int flags = 0, id;

			err = nla_parse_nested(tb, CTRL_ATTR_OP_MAX, nla,
					       family_op_policy);
			if (err < 0)
				goto errout;

			if (tb[CTRL_ATTR_OP_ID] == NULL) {
				err = -NLE_MISSING_ATTR;
				goto errout;
			}
			
			id = nla_get_u32(tb[CTRL_ATTR_OP_ID]);

			if (tb[CTRL_ATTR_OP_FLAGS])
				flags = nla_get_u32(tb[CTRL_ATTR_OP_FLAGS]);

			err = genl_family_add_op(family, id, flags);
			if (err < 0)
				goto errout;

		}
	}

	if (info->attrs[CTRL_ATTR_MCAST_GROUPS]) {
		struct nlattr *nla, *nla_grps;
		int remaining;

		nla_grps = info->attrs[CTRL_ATTR_MCAST_GROUPS];
		nla_for_each_nested(nla, nla_grps, remaining) {
			struct nlattr *tb[CTRL_ATTR_MCAST_GRP_MAX+1];
			int id;
			const char * name;

			err = nla_parse_nested(tb, CTRL_ATTR_MCAST_GRP_MAX, nla,
					       family_grp_policy);
			if (err < 0)
				goto errout;

			if (tb[CTRL_ATTR_MCAST_GRP_ID] == NULL) {
				err = -NLE_MISSING_ATTR;
				goto errout;
			}
			id = nla_get_u32(tb[CTRL_ATTR_MCAST_GRP_ID]);

			if (tb[CTRL_ATTR_MCAST_GRP_NAME] == NULL) {
				err = -NLE_MISSING_ATTR;
				goto errout;
			}
			name = nla_get_string(tb[CTRL_ATTR_MCAST_GRP_NAME]);

			err = genl_family_add_grp(family, id, name);
			if (err < 0)
				goto errout;
		}

	}

	err = pp->pp_cb((struct nl_object *) family, pp);
errout:
	genl_family_put(family);
	return err;
}

/**
 * @name Cache Management
 * @{
 */

int genl_ctrl_alloc_cache(struct nl_sock *sock, struct nl_cache **result)
{
	return nl_cache_alloc_and_fill(&genl_ctrl_ops, sock, result);
}

/**
 * Look up generic netlink family by id in the provided cache.
 * @arg cache		Generic netlink family cache.
 * @arg id		Family identifier.
 *
 * Searches through the cache looking for a registered family
 * matching the specified identifier. The caller will own a
 * reference on the returned object which needs to be given
 * back after usage using genl_family_put().
 *
 * @return Generic netlink family object or NULL if no match was found.
 */
struct genl_family *genl_ctrl_search(struct nl_cache *cache, int id)
{
	struct genl_family *fam;

	if (cache->c_ops != &genl_ctrl_ops)
		BUG();

	nl_list_for_each_entry(fam, &cache->c_items, ce_list) {
		if (fam->gf_id == id) {
			nl_object_get((struct nl_object *) fam);
			return fam;
		}
	}

	return NULL;
}

/**
 * @name Resolver
 * @{
 */

/**
 * Look up generic netlink family by family name in the provided cache.
 * @arg cache		Generic netlink family cache.
 * @arg name		Family name.
 *
 * Searches through the cache looking for a registered family
 * matching the specified name. The caller will own a reference
 * on the returned object which needs to be given back after
 * usage using genl_family_put().
 *
 * @return Generic netlink family object or NULL if no match was found.
 */
struct genl_family *genl_ctrl_search_by_name(struct nl_cache *cache,
					    const char *name)
{
	struct genl_family *fam;

	if (cache->c_ops != &genl_ctrl_ops)
		BUG();

	nl_list_for_each_entry(fam, &cache->c_items, ce_list) {
		if (!strcmp(name, fam->gf_name)) {
			nl_object_get((struct nl_object *) fam);
			return fam;
		}
	}

	return NULL;
}

/** @} */

/**
 * Resolve generic netlink family name to its identifier
 * @arg sk		Netlink socket.
 * @arg name		Name of generic netlink family
 *
 * Resolves the generic netlink family name to its identifer and returns
 * it.
 *
 * @return A positive identifier or a negative error code.
 */
int genl_ctrl_resolve(struct nl_sock *sk, const char *name)
{
	struct nl_cache *cache;
	struct genl_family *family;
	int err;

	if ((err = genl_ctrl_alloc_cache(sk, &cache)) < 0)
		return err;

	family = genl_ctrl_search_by_name(cache, name);
	if (family == NULL) {
		err = -NLE_OBJ_NOTFOUND;
		goto errout;
	}

	err = genl_family_get_id(family);
	genl_family_put(family);
errout:
	nl_cache_free(cache);

	return err;
}

static int genl_ctrl_grp_by_name(const struct genl_family *family,
				const char *grp_name)
{
	struct genl_family_grp *grp;

	nl_list_for_each_entry(grp, &family->gf_mc_grps, list) {
		if (!strcmp(grp->name, grp_name)) {
			return grp->id;
		}
	}

	return -NLE_OBJ_NOTFOUND;
}

int genl_ctrl_resolve_grp(struct nl_sock *sk, const char *family_name,
	const char *grp_name)
{
	struct nl_cache *cache;
	struct genl_family *family;
	int err;

	if ((err = genl_ctrl_alloc_cache(sk, &cache)) < 0)
		return err;

	family = genl_ctrl_search_by_name(cache, family_name);
	if (family == NULL) {
		err = -NLE_OBJ_NOTFOUND;
		goto errout;
	}

	err = genl_ctrl_grp_by_name(family, grp_name);
	genl_family_put(family);
errout:
	nl_cache_free(cache);

	return err;
}

/** @} */

static struct genl_cmd genl_cmds[] = {
	{
		.c_id		= CTRL_CMD_NEWFAMILY,
		.c_name		= "NEWFAMILY" ,
		.c_maxattr	= CTRL_ATTR_MAX,
		.c_attr_policy	= ctrl_policy,
		.c_msg_parser	= ctrl_msg_parser,
	},
	{
		.c_id		= CTRL_CMD_DELFAMILY,
		.c_name		= "DELFAMILY" ,
	},
	{
		.c_id		= CTRL_CMD_GETFAMILY,
		.c_name		= "GETFAMILY" ,
	},
	{
		.c_id		= CTRL_CMD_NEWOPS,
		.c_name		= "NEWOPS" ,
	},
	{
		.c_id		= CTRL_CMD_DELOPS,
		.c_name		= "DELOPS" ,
	},
};

static struct genl_ops genl_ops = {
	.o_cmds			= genl_cmds,
	.o_ncmds		= ARRAY_SIZE(genl_cmds),
};

/** @cond SKIP */
extern struct nl_object_ops genl_family_ops;
/** @endcond */

static struct nl_cache_ops genl_ctrl_ops = {
	.co_name		= "genl/family",
	.co_hdrsize		= GENL_HDRSIZE(0),
	.co_msgtypes		= GENL_FAMILY(GENL_ID_CTRL, "nlctrl"),
	.co_genl		= &genl_ops,
	.co_protocol		= NETLINK_GENERIC,
	.co_request_update      = ctrl_request_update,
	.co_obj_ops		= &genl_family_ops,
};

static void __init ctrl_init(void)
{
	genl_register(&genl_ctrl_ops);
}

static void __exit ctrl_exit(void)
{
	genl_unregister(&genl_ctrl_ops);
}

/** @} */
