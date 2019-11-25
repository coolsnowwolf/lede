/*
 * netlink/object.c	Generic Cacheable Object
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2008 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_OBJECT_H_
#define NETLINK_OBJECT_H_

#include <netlink/netlink.h>
#include <netlink/utils.h>
#include <netlink/object-api.h>

#ifdef __cplusplus
extern "C" {
#endif

#define NL_OBJ_MARK		1

struct nl_cache;
struct nl_object;
struct nl_object_ops;

struct nl_object
{
	NLHDR_COMMON
};

#define OBJ_CAST(ptr)		((struct nl_object *) (ptr))

/* General */
extern struct nl_object *	nl_object_alloc(struct nl_object_ops *);
extern void			nl_object_free(struct nl_object *);
extern struct nl_object *	nl_object_clone(struct nl_object *obj);

#ifdef disabled

extern int			nl_object_alloc_name(const char *,
						     struct nl_object **);
extern void			nl_object_dump(struct nl_object *,
					       struct nl_dump_params *);

extern uint32_t			nl_object_diff(struct nl_object *,
					       struct nl_object *);
extern int			nl_object_match_filter(struct nl_object *,
						       struct nl_object *);
extern int			nl_object_identical(struct nl_object *,
						    struct nl_object *);
extern char *			nl_object_attrs2str(struct nl_object *,
						    uint32_t attrs, char *buf,
						    size_t);
#endif
/**
 * Check whether this object is used by multiple users
 * @arg obj		object to check
 * @return true or false
 */
static inline int nl_object_shared(struct nl_object *obj)
{
	return obj->ce_refcnt > 1;
}


static inline void nl_object_get(struct nl_object *obj)
{
	obj->ce_refcnt++;
}

static inline void nl_object_put(struct nl_object *obj)
{
	if (!obj)
		return;

	obj->ce_refcnt--;

	if (obj->ce_refcnt <= 0)
		nl_object_free(obj);
}


/**
 * @name Marks
 * @{
 */

/**
 * Add mark to object
 * @arg obj		Object to mark
 */
static inline void nl_object_mark(struct nl_object *obj)
{
	obj->ce_flags |= NL_OBJ_MARK;
}

/**
 * Remove mark from object
 * @arg obj		Object to unmark
 */
static inline void nl_object_unmark(struct nl_object *obj)
{
	obj->ce_flags &= ~NL_OBJ_MARK;
}

/**
 * Return true if object is marked
 * @arg obj		Object to check
 * @return true if object is marked, otherwise false
 */
static inline int nl_object_is_marked(struct nl_object *obj)
{
	return (obj->ce_flags & NL_OBJ_MARK);
}

/** @} */

#ifdef disabled
/**
 * Return list of attributes present in an object
 * @arg obj		an object
 * @arg buf		destination buffer
 * @arg len		length of destination buffer
 *
 * @return destination buffer.
 */
static inline char *nl_object_attr_list(struct nl_object *obj, char *buf, size_t len)
{
	return nl_object_attrs2str(obj, obj->ce_mask, buf, len);
}
#endif

/**
 * @name Attributes
 * @{
 */

static inline int nl_object_get_refcnt(struct nl_object *obj)
{
	return obj->ce_refcnt;
}

static inline struct nl_cache *nl_object_get_cache(struct nl_object *obj)
{
	return obj->ce_cache;
}

static inline void *		nl_object_priv(struct nl_object *obj)
{
	return obj;
}


/** @} */


#ifdef __cplusplus
}
#endif

#endif
