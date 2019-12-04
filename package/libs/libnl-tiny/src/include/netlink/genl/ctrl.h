/*
 * netlink/genl/ctrl.h		Generic Netlink Controller
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_CTRL_H_
#define NETLINK_GENL_CTRL_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/addr.h>

#ifdef __cplusplus
extern "C" {
#endif

struct genl_family;

extern int			genl_ctrl_alloc_cache(struct nl_sock *,
						      struct nl_cache **);
extern struct genl_family *	genl_ctrl_search(struct nl_cache *, int);
extern struct genl_family *	genl_ctrl_search_by_name(struct nl_cache *,
							 const char *);
extern int			genl_ctrl_resolve(struct nl_sock *,
						  const char *);
extern int 			genl_ctrl_resolve_grp(struct nl_sock *sk,
						      const char *family,
						      const char *grp);

#ifdef __cplusplus
}
#endif

#endif
