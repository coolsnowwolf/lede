/*
 * lib/cache_mngt.c	Cache Management
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup core
 * @defgroup cache_mngt Caching
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/utils.h>

static struct nl_cache_ops *cache_ops;

/**
 * @name Cache Operations Sets
 * @{
 */

/**
 * Lookup the set cache operations of a certain cache type
 * @arg name		name of the cache type
 *
 * @return The cache operations or NULL if no operations
 *         have been registered under the specified name.
 */
struct nl_cache_ops *nl_cache_ops_lookup(const char *name)
{
	struct nl_cache_ops *ops;

	for (ops = cache_ops; ops; ops = ops->co_next)
		if (!strcmp(ops->co_name, name))
			return ops;

	return NULL;
}

/**
 * Associate a message type to a set of cache operations
 * @arg protocol		netlink protocol
 * @arg msgtype			netlink message type
 *
 * Associates the specified netlink message type with
 * a registered set of cache operations.
 *
 * @return The cache operations or NULL if no association
 *         could be made.
 */
struct nl_cache_ops *nl_cache_ops_associate(int protocol, int msgtype)
{
	int i;
	struct nl_cache_ops *ops;

	for (ops = cache_ops; ops; ops = ops->co_next) {
		if (ops->co_protocol != protocol)
			continue;

		for (i = 0; ops->co_msgtypes[i].mt_id >= 0; i++)
			if (ops->co_msgtypes[i].mt_id == msgtype)
				return ops;
	}

	return NULL;
}

/**
 * Register a set of cache operations
 * @arg ops		cache operations
 *
 * Called by users of caches to announce the avaibility of
 * a certain cache type.
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_mngt_register(struct nl_cache_ops *ops)
{
	if (!ops->co_name || !ops->co_obj_ops)
		return -NLE_INVAL;

	if (nl_cache_ops_lookup(ops->co_name))
		return -NLE_EXIST;

	ops->co_next = cache_ops;
	cache_ops = ops;

	NL_DBG(1, "Registered cache operations %s\n", ops->co_name);

	return 0;
}

/**
 * Unregister a set of cache operations
 * @arg ops		cache operations
 *
 * Called by users of caches to announce a set of
 * cache operations is no longer available. The
 * specified cache operations must have been registered
 * previously using nl_cache_mngt_register()
 *
 * @return 0 on success or a negative error code
 */
int nl_cache_mngt_unregister(struct nl_cache_ops *ops)
{
	struct nl_cache_ops *t, **tp;

	for (tp = &cache_ops; (t=*tp) != NULL; tp = &t->co_next)
		if (t == ops)
			break;

	if (!t)
		return -NLE_NOCACHE;

	NL_DBG(1, "Unregistered cache operations %s\n", ops->co_name);

	*tp = t->co_next;
	return 0;
}

/** @} */

/** @} */
