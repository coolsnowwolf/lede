/*
 * netlink/cache-api.h		Caching API
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_CACHE_API_H_
#define NETLINK_CACHE_API_H_

#include <netlink/netlink.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup cache
 * @defgroup cache_api Cache Implementation
 * @brief
 *
 * @par 1) Cache Definition
 * @code
 * struct nl_cache_ops my_cache_ops = {
 * 	.co_name		= "route/link",
 * 	.co_protocol		= NETLINK_ROUTE,
 * 	.co_hdrsize		= sizeof(struct ifinfomsg),
 * 	.co_obj_ops		= &my_obj_ops,
 * };
 * @endcode
 *
 * @par 2) 
 * @code
 * // The simplest way to fill a cache is by providing a request-update
 * // function which must trigger a complete dump on the kernel-side of
 * // whatever the cache covers.
 * static int my_request_update(struct nl_cache *cache,
 * 				struct nl_sock *socket)
 * {
 * 	// In this example, we request a full dump of the interface table
 * 	return nl_rtgen_request(socket, RTM_GETLINK, AF_UNSPEC, NLM_F_DUMP);
 * }
 *
 * // The resulting netlink messages sent back will be fed into a message
 * // parser one at a time. The message parser has to extract all relevant
 * // information from the message and create an object reflecting the
 * // contents of the message and pass it on to the parser callback function
 * // provide which will add the object to the cache.
 * static int my_msg_parser(struct nl_cache_ops *ops, struct sockaddr_nl *who,
 * 			    struct nlmsghdr *nlh, struct nl_parser_param *pp)
 * {
 * 	struct my_obj *obj;
 *
 * 	obj = my_obj_alloc();
 * 	obj->ce_msgtype = nlh->nlmsg_type;
 *
 * 	// Parse the netlink message and continue creating the object.
 *
 * 	err = pp->pp_cb((struct nl_object *) obj, pp);
 * 	if (err < 0)
 * 		goto errout;
 * }
 *
 * struct nl_cache_ops my_cache_ops = {
 * 	...
 * 	.co_request_update	= my_request_update,
 * 	.co_msg_parser		= my_msg_parser,
 * };
 * @endcode
 *
 * @par 3) Notification based Updates
 * @code
 * // Caches can be kept up-to-date based on notifications if the kernel
 * // sends out notifications whenever an object is added/removed/changed.
 * //
 * // It is trivial to support this, first a list of groups needs to be
 * // defined which are required to join in order to receive all necessary
 * // notifications. The groups are separated by address family to support
 * // the common situation where a separate group is used for each address
 * // family. If there is only one group, simply specify AF_UNSPEC.
 * static struct nl_af_group addr_groups[] = {
 * 	{ AF_INET,	RTNLGRP_IPV4_IFADDR },
 * 	{ AF_INET6,	RTNLGRP_IPV6_IFADDR },
 * 	{ END_OF_GROUP_LIST },
 * };
 *
 * // In order for the caching system to know the meaning of each message
 * // type it requires a table which maps each supported message type to
 * // a cache action, e.g. RTM_NEWADDR means address has been added or
 * // updated, RTM_DELADDR means address has been removed.
 * static struct nl_cache_ops rtnl_addr_ops = {
 * 	...
 * 	.co_msgtypes		= {
 * 					{ RTM_NEWADDR, NL_ACT_NEW, "new" },
 * 					{ RTM_DELADDR, NL_ACT_DEL, "del" },
 * 					{ RTM_GETADDR, NL_ACT_GET, "get" },
 * 					END_OF_MSGTYPES_LIST,
 * 				},
 * 	.co_groups		= addr_groups,
 * };
 *
 * // It is now possible to keep the cache up-to-date using the cache manager.
 * @endcode
 * @{
 */

enum {
	NL_ACT_UNSPEC,
	NL_ACT_NEW,
	NL_ACT_DEL,
	NL_ACT_GET,
	NL_ACT_SET,
	NL_ACT_CHANGE,
	__NL_ACT_MAX,
};

#define NL_ACT_MAX (__NL_ACT_MAX - 1)

#define END_OF_MSGTYPES_LIST	{ -1, -1, NULL }

/**
 * Message type to cache action association
 */
struct nl_msgtype
{
	/** Netlink message type */
	int			mt_id;

	/** Cache action to take */
	int			mt_act;

	/** Name of operation for human-readable printing */
	char *			mt_name;
};

/**
 * Address family to netlink group association
 */
struct nl_af_group
{
	/** Address family */
	int			ag_family;

	/** Netlink group identifier */
	int			ag_group;
};

#define END_OF_GROUP_LIST AF_UNSPEC, 0

struct nl_parser_param
{
	int             (*pp_cb)(struct nl_object *, struct nl_parser_param *);
	void *            pp_arg;
};

/**
 * Cache Operations
 */
struct nl_cache_ops
{
	char  *			co_name;

	int			co_hdrsize;
	int			co_protocol;
	struct nl_af_group *	co_groups;
	
	/**
	 * Called whenever an update of the cache is required. Must send
	 * a request message to the kernel requesting a complete dump.
	 */
	int   (*co_request_update)(struct nl_cache *, struct nl_sock *);

	/**
	 * Called whenever a message was received that needs to be parsed.
	 * Must parse the message and call the paser callback function
	 * (nl_parser_param) provided via the argument.
	 */
	int   (*co_msg_parser)(struct nl_cache_ops *, struct sockaddr_nl *,
			       struct nlmsghdr *, struct nl_parser_param *);

	struct nl_object_ops *	co_obj_ops;

	struct nl_cache_ops *co_next;
	struct nl_cache *co_major_cache;
	struct genl_ops *	co_genl;
	struct nl_msgtype	co_msgtypes[];
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif
