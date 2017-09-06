/*
 * lib/genl/mngt.c		Generic Netlink Management
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup genl
 * @defgroup genl_mngt Management
 *
 * @par 1) Registering a generic netlink module
 * @code
 * #include <netlink/genl/mngt.h>
 *
 * // First step is to define all the commands being used in
 * // particular generic netlink family. The ID and name are
 * // mandatory to be filled out. A callback function and
 * // most the attribute policy that comes with it must be
 * // defined for commands expected to be issued towards
 * // userspace.
 * static struct genl_cmd foo_cmds[] = {
 * 	{
 * 		.c_id		= FOO_CMD_NEW,
 * 		.c_name		= "NEWFOO" ,
 * 		.c_maxattr	= FOO_ATTR_MAX,
 * 		.c_attr_policy	= foo_policy,
 * 		.c_msg_parser	= foo_msg_parser,
 * 	},
 * 	{
 * 		.c_id		= FOO_CMD_DEL,
 * 		.c_name		= "DELFOO" ,
 * 	},
 * };
 *
 * // The list of commands must then be integrated into a
 * // struct genl_ops serving as handle for this particular
 * // family.
 * static struct genl_ops my_genl_ops = {
 * 	.o_cmds			= foo_cmds,
 * 	.o_ncmds		= ARRAY_SIZE(foo_cmds),
 * };
 *
 * // Using the above struct genl_ops an arbitary number of
 * // cache handles can be associated to it.
 * //
 * // The macro GENL_HDRSIZE() must be used to specify the
 * // length of the header to automatically take headers on
 * // generic layers into account.
 * //
 * // The macro GENL_FAMILY() is used to represent the generic
 * // netlink family id.
 * static struct nl_cache_ops genl_foo_ops = {
 * 	.co_name		= "genl/foo",
 * 	.co_hdrsize		= GENL_HDRSIZE(sizeof(struct my_hdr)),
 * 	.co_msgtypes		= GENL_FAMILY(GENL_ID_GENERATE, "foo"),
 * 	.co_genl		= &my_genl_ops,
 * 	.co_protocol		= NETLINK_GENERIC,
 * 	.co_request_update      = foo_request_update,
 * 	.co_obj_ops		= &genl_foo_ops,
 * };
 *
 * // Finally each cache handle for a generic netlink family
 * // must be registered using genl_register().
 * static void __init foo_init(void)
 * {
 * 	genl_register(&genl_foo_ops);
 * }
 *
 * // ... respectively unregsted again.
 * static void __exit foo_exit(void)
 * {
 * 	genl_unregister(&genl_foo_ops);
 * }
 * @endcode
 * @{
 */

#include <netlink-generic.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/mngt.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>
#include <netlink/utils.h>

static NL_LIST_HEAD(genl_ops_list);

static int genl_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
			   struct nlmsghdr *nlh, struct nl_parser_param *pp)
{
	int i, err;
	struct genlmsghdr *ghdr;
	struct genl_cmd *cmd;

	ghdr = nlmsg_data(nlh);

	if (ops->co_genl == NULL)
		BUG();

	for (i = 0; i < ops->co_genl->o_ncmds; i++) {
		cmd = &ops->co_genl->o_cmds[i];
		if (cmd->c_id == ghdr->cmd)
			goto found;
	}

	err = -NLE_MSGTYPE_NOSUPPORT;
	goto errout;

found:
	if (cmd->c_msg_parser == NULL)
		err = -NLE_OPNOTSUPP;
	else {
		struct nlattr *tb[cmd->c_maxattr + 1];
		struct genl_info info = {
			.who = who,
			.nlh = nlh,
			.genlhdr = ghdr,
			.userhdr = genlmsg_data(ghdr),
			.attrs = tb,
		};

		err = nlmsg_parse(nlh, ops->co_hdrsize, tb, cmd->c_maxattr,
				  cmd->c_attr_policy);
		if (err < 0)
			goto errout;

		err = cmd->c_msg_parser(ops, cmd, &info, pp);
	}
errout:
	return err;

}

/**
 * @name Register/Unregister
 * @{
 */

/**
 * Register generic netlink operations
 * @arg ops		cache operations
 */
int genl_register(struct nl_cache_ops *ops)
{
	int err;

	if (ops->co_protocol != NETLINK_GENERIC) {
		err = -NLE_PROTO_MISMATCH;
		goto errout;
	}

	if (ops->co_hdrsize < GENL_HDRSIZE(0)) {
		err = -NLE_INVAL;
		goto errout;
	}

	if (ops->co_genl == NULL) {
		err = -NLE_INVAL;
		goto errout;
	}

	ops->co_genl->o_cache_ops = ops;
	ops->co_genl->o_name = ops->co_msgtypes[0].mt_name;
	ops->co_genl->o_family = ops->co_msgtypes[0].mt_id;
	ops->co_msg_parser = genl_msg_parser;

	/* FIXME: check for dup */

	nl_list_add_tail(&ops->co_genl->o_list, &genl_ops_list);

	err = nl_cache_mngt_register(ops);
errout:
	return err;
}

/**
 * Unregister generic netlink operations
 * @arg ops		cache operations
 */
void genl_unregister(struct nl_cache_ops *ops)
{
	nl_cache_mngt_unregister(ops);
	nl_list_del(&ops->co_genl->o_list);
}

/** @} */

/** @} */
