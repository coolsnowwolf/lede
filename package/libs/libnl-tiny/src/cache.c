/*
 * lib/cache.c		Caching Module
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cache_mngt
 * @defgroup cache Cache
 *
 * @code
 *   Cache Management             |    | Type Specific Cache Operations
 *                                      
 *                                |    | +----------------+ +------------+
 *                                       | request update | | msg_parser |
 *                                |    | +----------------+ +------------+
 *                                     +- - - - -^- - - - - - - -^- -|- - - -
 *    nl_cache_update:            |              |               |   |
 *          1) --------- co_request_update ------+               |   |
 *                                |                              |   |
 *          2) destroy old cache     +----------- pp_cb ---------|---+
 *                                |  |                           |
 *          3) ---------- nl_recvmsgs ----------+   +- cb_valid -+
 *             +--------------+   |  |          |   |
 *             | nl_cache_add |<-----+   + - - -v- -|- - - - - - - - - - -
 *             +--------------+   |      | +-------------+
 *                                         | nl_recvmsgs |
 *                                |      | +-----|-^-----+
 *                                           +---v-|---+
 *                                |      |   | nl_recv |
 *                                           +---------+
 *                                |      |                 Core Netlink
 * @endcode
 * 
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/object.h>
#include <netlink/utils.h>

/**
 * @name Cache Creation/Deletion
 * @{
 */

/**
 * Allocate an empty cache
 * @arg ops		cache operations to base the cache on
 * 
 * @return A newly allocated and initialized cache.
 */
struct nl_cache *nl_cache_alloc(struct nl_cache_ops *ops)
{
	struct nl_cache *cache;

	cache = calloc(1, sizeof(*cache));
	if (!cache)
		return NULL;

	nl_init_list_head(&cache->c_items);
	cache->c_ops = ops;

	NL_DBG(2, "Allocated cache %p <%s>.\n", cache, nl_cache_name(cache));

	return cache;
}

int nl_cache_alloc_and_fill(struct nl_cache_ops *ops, struct nl_sock *sock,
			    struct nl_cache **result)
{
	struct nl_cache *cache;
	int err;
	
	if (!(cache = nl_cache_alloc(ops)))
		return -NLE_NOMEM;

	if (sock && (err = nl_cache_refill(sock, cache)) < 0) {
		nl_cache_free(cache);
		return err;
	}

	*result = cache;
	return 0;
}

/**
 * Clear a cache.
 * @arg cache		cache to clear
 *
 * Removes all elements of a cache.
 */
void nl_cache_clear(struct nl_cache *cache)
{
	struct nl_object *obj, *tmp;

	NL_DBG(1, "Clearing cache %p <%s>...\n", cache, nl_cache_name(cache));

	nl_list_for_each_entry_safe(obj, tmp, &cache->c_items, ce_list)
		nl_cache_remove(obj);
}

/**
 * Free a cache.
 * @arg cache		Cache to free.
 *
 * Removes all elements of a cache and frees all memory.
 *
 * @note Use this function if you are working with allocated caches.
 */
void nl_cache_free(struct nl_cache *cache)
{
	if (!cache)
		return;

	nl_cache_clear(cache);
	NL_DBG(1, "Freeing cache %p <%s>...\n", cache, nl_cache_name(cache));
	free(cache);
}

/** @} */

/**
 * @name Cache Modifications
 * @{
 */

static int __cache_add(struct nl_cache *cache, struct nl_object *obj)
{
	obj->ce_cache = cache;

	nl_list_add_tail(&obj->ce_list, &cache->c_items);
	cache->c_nitems++;

	NL_DBG(1, "Added %p to cache %p <%s>.\n",
	       obj, cache, nl_cache_name(cache));

	return 0;
}

/**
 * Add object to a cache.
 * @arg cache		Cache to add object to
 * @arg obj		Object to be added to the cache
 *
 * Adds the given object to the specified cache. The object is cloned
 * if it has been added to another cache already.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_add(struct nl_cache *cache, struct nl_object *obj)
{
	struct nl_object *new;

	if (cache->c_ops->co_obj_ops != obj->ce_ops)
		return -NLE_OBJ_MISMATCH;

	if (!nl_list_empty(&obj->ce_list)) {
		new = nl_object_clone(obj);
		if (!new)
			return -NLE_NOMEM;
	} else {
		nl_object_get(obj);
		new = obj;
	}

	return __cache_add(cache, new);
}

/**
 * Removes an object from a cache.
 * @arg obj		Object to remove from its cache
 *
 * Removes the object \c obj from the cache it is assigned to, since
 * an object can only be assigned to one cache at a time, the cache
 * must ne be passed along with it.
 */
void nl_cache_remove(struct nl_object *obj)
{
	struct nl_cache *cache = obj->ce_cache;

	if (cache == NULL)
		return;

	nl_list_del(&obj->ce_list);
	obj->ce_cache = NULL;
	nl_object_put(obj);
	cache->c_nitems--;

	NL_DBG(1, "Deleted %p from cache %p <%s>.\n",
	       obj, cache, nl_cache_name(cache));
}

/** @} */

/**
 * @name Synchronization
 * @{
 */

/**
 * Request a full dump from the kernel to fill a cache
 * @arg sk		Netlink socket.
 * @arg cache		Cache subjected to be filled.
 *
 * Send a dumping request to the kernel causing it to dump all objects
 * related to the specified cache to the netlink socket.
 *
 * Use nl_cache_pickup() to read the objects from the socket and fill them
 * into a cache.
 */
int nl_cache_request_full_dump(struct nl_sock *sk, struct nl_cache *cache)
{
	NL_DBG(2, "Requesting dump from kernel for cache %p <%s>...\n",
	          cache, nl_cache_name(cache));

	if (cache->c_ops->co_request_update == NULL)
		return -NLE_OPNOTSUPP;

	return cache->c_ops->co_request_update(cache, sk);
}

/** @cond SKIP */
struct update_xdata {
	struct nl_cache_ops *ops;
	struct nl_parser_param *params;
};

static int update_msg_parser(struct nl_msg *msg, void *arg)
{
	struct update_xdata *x = arg;
	
	return nl_cache_parse(x->ops, &msg->nm_src, msg->nm_nlh, x->params);
}
/** @endcond */

int __cache_pickup(struct nl_sock *sk, struct nl_cache *cache,
		   struct nl_parser_param *param)
{
	int err;
	struct nl_cb *cb;
	struct update_xdata x = {
		.ops = cache->c_ops,
		.params = param,
	};

	NL_DBG(1, "Picking up answer for cache %p <%s>...\n",
		  cache, nl_cache_name(cache));

	cb = nl_cb_clone(sk->s_cb);
	if (cb == NULL)
		return -NLE_NOMEM;

	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, update_msg_parser, &x);

	err = nl_recvmsgs(sk, cb);
	if (err < 0)
		NL_DBG(2, "While picking up for %p <%s>, recvmsgs() returned " \
		       "%d: %s", cache, nl_cache_name(cache),
		       err, nl_geterror(err));

	nl_cb_put(cb);

	return err;
}

static int pickup_cb(struct nl_object *c, struct nl_parser_param *p)
{
	return nl_cache_add((struct nl_cache *) p->pp_arg, c);
}

/**
 * Pickup a netlink dump response and put it into a cache.
 * @arg sk		Netlink socket.
 * @arg cache		Cache to put items into.
 *
 * Waits for netlink messages to arrive, parses them and puts them into
 * the specified cache.
 *
 * @return 0 on success or a negative error code.
 */
int nl_cache_pickup(struct nl_sock *sk, struct nl_cache *cache)
{
	struct nl_parser_param p = {
		.pp_cb = pickup_cb,
		.pp_arg = cache,
	};

	return __cache_pickup(sk, cache, &p);
}


/** @} */

/**
 * @name Parsing
 * @{
 */

/** @cond SKIP */
int nl_cache_parse(struct nl_cache_ops *ops, struct sockaddr_nl *who,
		   struct nlmsghdr *nlh, struct nl_parser_param *params)
{
	int i, err;

	if (!nlmsg_valid_hdr(nlh, ops->co_hdrsize))
		return -NLE_MSG_TOOSHORT;

	for (i = 0; ops->co_msgtypes[i].mt_id >= 0; i++) {
		if (ops->co_msgtypes[i].mt_id == nlh->nlmsg_type) {
			err = ops->co_msg_parser(ops, who, nlh, params);
			if (err != -NLE_OPNOTSUPP)
				goto errout;
		}
	}


	err = -NLE_MSGTYPE_NOSUPPORT;
errout:
	return err;
}
/** @endcond */

/**
 * Parse a netlink message and add it to the cache.
 * @arg cache		cache to add element to
 * @arg msg		netlink message
 *
 * Parses a netlink message by calling the cache specific message parser
 * and adds the new element to the cache.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_parse_and_add(struct nl_cache *cache, struct nl_msg *msg)
{
	struct nl_parser_param p = {
		.pp_cb = pickup_cb,
		.pp_arg = cache,
	};

	return nl_cache_parse(cache->c_ops, NULL, nlmsg_hdr(msg), &p);
}

/**
 * (Re)fill a cache with the contents in the kernel.
 * @arg sk		Netlink socket.
 * @arg cache		cache to update
 *
 * Clears the specified cache and fills it with the current state in
 * the kernel.
 *
 * @return 0 or a negative error code.
 */
int nl_cache_refill(struct nl_sock *sk, struct nl_cache *cache)
{
	int err;

	err = nl_cache_request_full_dump(sk, cache);
	if (err < 0)
		return err;

	NL_DBG(2, "Upading cache %p <%s>, request sent, waiting for dump...\n",
	       cache, nl_cache_name(cache));
	nl_cache_clear(cache);

	return nl_cache_pickup(sk, cache);
}

/** @} */
