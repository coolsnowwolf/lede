/*
 * lib/handlers.c	default netlink message handlers
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
 * @defgroup cb Callbacks/Customization
 *
 * @details
 * @par 1) Setting up a callback set
 * @code
 * // Allocate a callback set and initialize it to the verbose default set
 * struct nl_cb *cb = nl_cb_alloc(NL_CB_VERBOSE);
 *
 * // Modify the set to call my_func() for all valid messages
 * nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, my_func, NULL);
 *
 * // Set the error message handler to the verbose default implementation
 * // and direct it to print all errors to the given file descriptor.
 * FILE *file = fopen(...);
 * nl_cb_err(cb, NL_CB_VERBOSE, NULL, file);
 * @endcode
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/msg.h>
#include <netlink/handlers.h>

/**
 * @name Callback Handle Management
 * @{
 */

/**
 * Allocate a new callback handle
 * @arg kind		callback kind to be used for initialization
 * @return Newly allocated callback handle or NULL
 */
struct nl_cb *nl_cb_alloc(enum nl_cb_kind kind)
{
	int i;
	struct nl_cb *cb;

	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return NULL;

	cb = calloc(1, sizeof(*cb));
	if (!cb)
		return NULL;

	cb->cb_refcnt = 1;

	for (i = 0; i <= NL_CB_TYPE_MAX; i++)
		nl_cb_set(cb, i, kind, NULL, NULL);

	nl_cb_err(cb, kind, NULL, NULL);

	return cb;
}

/**
 * Clone an existing callback handle
 * @arg orig		original callback handle
 * @return Newly allocated callback handle being a duplicate of
 *         orig or NULL
 */
struct nl_cb *nl_cb_clone(struct nl_cb *orig)
{
	struct nl_cb *cb;
	
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb)
		return NULL;

	memcpy(cb, orig, sizeof(*orig));
	cb->cb_refcnt = 1;

	return cb;
}

void nl_cb_put(struct nl_cb *cb)
{
	if (!cb)
		return;

	cb->cb_refcnt--;

	if (cb->cb_refcnt < 0)
		BUG();

	if (cb->cb_refcnt <= 0)
		free(cb);
}

/** @} */

/**
 * @name Callback Setup
 * @{
 */

/**
 * Set up a callback 
 * @arg cb		callback set
 * @arg type		callback to modify
 * @arg kind		kind of implementation
 * @arg func		callback function (NL_CB_CUSTOM)
 * @arg arg		argument passed to callback
 *
 * @return 0 on success or a negative error code
 */
int nl_cb_set(struct nl_cb *cb, enum nl_cb_type type, enum nl_cb_kind kind,
	      nl_recvmsg_msg_cb_t func, void *arg)
{
	if (type < 0 || type > NL_CB_TYPE_MAX)
		return -NLE_RANGE;

	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return -NLE_RANGE;

	if (kind == NL_CB_CUSTOM) {
		cb->cb_set[type] = func;
		cb->cb_args[type] = arg;
	}

	return 0;
}

/**
 * Set up an error callback
 * @arg cb		callback set
 * @arg kind		kind of callback
 * @arg func		callback function
 * @arg arg		argument to be passed to callback function
 */
int nl_cb_err(struct nl_cb *cb, enum nl_cb_kind kind,
	      nl_recvmsg_err_cb_t func, void *arg)
{
	if (kind < 0 || kind > NL_CB_KIND_MAX)
		return -NLE_RANGE;

	if (kind == NL_CB_CUSTOM) {
		cb->cb_err = func;
		cb->cb_err_arg = arg;
	}

	return 0;
}

/** @} */

/** @} */
