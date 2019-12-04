/*
 * lib/object.c		Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup cache
 * @defgroup object Object
 * @{
 */

#include <netlink-local.h>
#include <netlink/netlink.h>
#include <netlink/cache.h>
#include <netlink/object.h>
#include <netlink/utils.h>

static inline struct nl_object_ops *obj_ops(struct nl_object *obj)
{
	if (!obj->ce_ops)
		BUG();

	return obj->ce_ops;
}

/**
 * @name Object Creation/Deletion
 * @{
 */

/**
 * Allocate a new object of kind specified by the operations handle
 * @arg ops		cache operations handle
 * @return The new object or NULL
 */
struct nl_object *nl_object_alloc(struct nl_object_ops *ops)
{
	struct nl_object *new;

	if (ops->oo_size < sizeof(*new))
		BUG();

	new = calloc(1, ops->oo_size);
	if (!new)
		return NULL;

	new->ce_refcnt = 1;
	nl_init_list_head(&new->ce_list);

	new->ce_ops = ops;
	if (ops->oo_constructor)
		ops->oo_constructor(new);

	NL_DBG(4, "Allocated new object %p\n", new);

	return new;
}

struct nl_derived_object {
	NLHDR_COMMON
	char data;
};

/**
 * Allocate a new object and copy all data from an existing object
 * @arg obj		object to inherite data from
 * @return The new object or NULL.
 */
struct nl_object *nl_object_clone(struct nl_object *obj)
{
	struct nl_object *new;
	struct nl_object_ops *ops = obj_ops(obj);
	int doff = offsetof(struct nl_derived_object, data);
	int size;

	new = nl_object_alloc(ops);
	if (!new)
		return NULL;

	size = ops->oo_size - doff;
	if (size < 0)
		BUG();

	new->ce_ops = obj->ce_ops;
	new->ce_msgtype = obj->ce_msgtype;

	if (size)
		memcpy((void *)new + doff, (void *)obj + doff, size);

	if (ops->oo_clone) {
		if (ops->oo_clone(new, obj) < 0) {
			nl_object_free(new);
			return NULL;
		}
	} else if (size && ops->oo_free_data)
		BUG();

	return new;
}

/**
 * Free a cacheable object
 * @arg obj		object to free
 *
 * @return 0 or a negative error code.
 */
void nl_object_free(struct nl_object *obj)
{
	struct nl_object_ops *ops = obj_ops(obj);

	if (obj->ce_refcnt > 0)
		NL_DBG(1, "Warning: Freeing object in use...\n");

	if (obj->ce_cache)
		nl_cache_remove(obj);

	if (ops->oo_free_data)
		ops->oo_free_data(obj);

	free(obj);

	NL_DBG(4, "Freed object %p\n", obj);
}

/** @} */

/**
 * @name Reference Management
 * @{
 */

/** @} */

/**
 * @name Utillities
 * @{
 */

/** @} */

/** @} */
