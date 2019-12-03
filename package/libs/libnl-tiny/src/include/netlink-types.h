/*
 * netlink-types.h	Netlink Types (Private)
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_LOCAL_TYPES_H_
#define NETLINK_LOCAL_TYPES_H_

#include <netlink/list.h>

struct nl_cache_ops;
struct nl_sock;
struct nl_object;

struct nl_cache
{
	struct nl_list_head	c_items;
	int			c_nitems;
	int                     c_iarg1;
	int                     c_iarg2;
	struct nl_cache_ops *   c_ops;
};

struct nl_cache_assoc
{
	struct nl_cache *	ca_cache;
	change_func_t		ca_change;
};

struct nl_cache_mngr
{
	int			cm_protocol;
	int			cm_flags;
	int			cm_nassocs;
	struct nl_sock *	cm_handle;
	struct nl_cache_assoc *	cm_assocs;
};

struct nl_parser_param;

#define LOOSE_COMPARISON	1


struct nl_data
{
	size_t			d_size;
	void *			d_data;
};

struct nl_addr
{
	int			a_family;
	unsigned int		a_maxsize;
	unsigned int		a_len;
	int			a_prefixlen;
	int			a_refcnt;
	char			a_addr[0];
};

#define IFQDISCSIZ	32

#define GENL_OP_HAS_POLICY	1
#define GENL_OP_HAS_DOIT	2
#define GENL_OP_HAS_DUMPIT	4

struct genl_family_grp {
	struct genl_family	*family;	/* private */
	struct nl_list_head	list;		/* private */
	char			name[GENL_NAMSIZ];
	u_int32_t		id;
};

struct genl_family_op
{
	uint32_t		o_id;
	uint32_t		o_flags;

	struct nl_list_head	o_list;
};


#endif
