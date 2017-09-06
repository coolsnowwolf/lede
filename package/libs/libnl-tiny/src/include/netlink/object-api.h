/*
 * netlink/object-api.c		Object API
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2007 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_OBJECT_API_H_
#define NETLINK_OBJECT_API_H_

#include <netlink/netlink.h>
#include <netlink/utils.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @ingroup object
 * @defgroup object_api Object API
 * @brief
 *
 * @par 1) Object Definition
 * @code
 * // Define your object starting with the common object header
 * struct my_obj {
 * 	NLHDR_COMMON
 * 	int		my_data;
 * };
 *
 * // Fill out the object operations structure
 * struct nl_object_ops my_ops = {
 * 	.oo_name	= "my_obj",
 * 	.oo_size	= sizeof(struct my_obj),
 * };
 *
 * // At this point the object can be allocated, you may want to provide a
 * // separate _alloc() function to ease allocting objects of this kind.
 * struct nl_object *obj = nl_object_alloc(&my_ops);
 *
 * // And release it again...
 * nl_object_put(obj);
 * @endcode
 *
 * @par 2) Allocating additional data
 * @code
 * // You may require to allocate additional data and store it inside
 * // object, f.e. assuming there is a field `ptr'.
 * struct my_obj {
 * 	NLHDR_COMMON
 * 	void *		ptr;
 * };
 *
 * // And at some point you may assign allocated data to this field:
 * my_obj->ptr = calloc(1, ...);
 *
 * // In order to not introduce any memory leaks you have to release
 * // this data again when the last reference is given back.
 * static void my_obj_free_data(struct nl_object *obj)
 * {
 * 	struct my_obj *my_obj = nl_object_priv(obj);
 *
 * 	free(my_obj->ptr);
 * }
 *
 * // Also when the object is cloned, you must ensure for your pointer
 * // stay valid even if one of the clones is freed by either making
 * // a clone as well or increase the reference count.
 * static int my_obj_clone(struct nl_object *src, struct nl_object *dst)
 * {
 * 	struct my_obj *my_src = nl_object_priv(src);
 * 	struct my_obj *my_dst = nl_object_priv(dst);
 *
 * 	if (src->ptr) {
 * 		dst->ptr = calloc(1, ...);
 * 		memcpy(dst->ptr, src->ptr, ...);
 * 	}
 * }
 *
 * struct nl_object_ops my_ops = {
 * 	...
 * 	.oo_free_data	= my_obj_free_data,
 * 	.oo_clone	= my_obj_clone,
 * };
 * @endcode
 *
 * @par 3) Object Dumping
 * @code
 * static int my_obj_dump_detailed(struct nl_object *obj,
 * 				   struct nl_dump_params *params)
 * {
 * 	struct my_obj *my_obj = nl_object_priv(obj);
 *
 * 	// It is absolutely essential to use nl_dump() when printing
 *	// any text to make sure the dumping parameters are respected.
 * 	nl_dump(params, "Obj Integer: %d\n", my_obj->my_int);
 *
 * 	// Before we can dump the next line, make sure to prefix
 *	// this line correctly.
 * 	nl_new_line(params);
 *
 * 	// You may also split a line into multiple nl_dump() calls.
 * 	nl_dump(params, "String: %s ", my_obj->my_string);
 * 	nl_dump(params, "String-2: %s\n", my_obj->another_string);
 * }
 *
 * struct nl_object_ops my_ops = {
 * 	...
 * 	.oo_dump[NL_DUMP_FULL]	= my_obj_dump_detailed,
 * };
 * @endcode
 *
 * @par 4) Object Attributes
 * @code
 * // The concept of object attributes is optional but can ease the typical
 * // case of objects that have optional attributes, e.g. a route may have a
 * // nexthop assigned but it is not required to.
 *
 * // The first step to define your object specific bitmask listing all
 * // attributes
 * #define MY_ATTR_FOO		(1<<0)
 * #define MY_ATTR_BAR		(1<<1)
 *
 * // When assigning an optional attribute to the object, make sure
 * // to mark its availability.
 * my_obj->foo = 123123;
 * my_obj->ce_mask |= MY_ATTR_FOO;
 *
 * // At any time you may use this mask to check for the availability
 * // of the attribute, e.g. while dumping
 * if (my_obj->ce_mask & MY_ATTR_FOO)
 * 	nl_dump(params, "foo %d ", my_obj->foo);
 *
 * // One of the big advantages of this concept is that it allows for
 * // standardized comparisons which make it trivial for caches to
 * // identify unique objects by use of unified comparison functions.
 * // In order for it to work, your object implementation must provide
 * // a comparison function and define a list of attributes which
 * // combined together make an object unique.
 *
 * static int my_obj_compare(struct nl_object *_a, struct nl_object *_b,
 * 			     uint32_t attrs, int flags)
 * {
 * 	struct my_obj *a = nl_object_priv(_a):
 * 	struct my_obj *b = nl_object_priv(_b):
 * 	int diff = 0;
 *
 * 	// We help ourselves in defining our own DIFF macro which will
 *	// call ATTR_DIFF() on both objects which will make sure to only
 *	// compare the attributes if required.
 * 	#define MY_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, MY_ATTR_##ATTR, a, b, EXPR)
 *
 * 	// Call our own diff macro for each attribute to build a bitmask
 *	// representing the attributes which mismatch.
 * 	diff |= MY_DIFF(FOO, a->foo != b->foo)
 * 	diff |= MY_DIFF(BAR, strcmp(a->bar, b->bar))
 *
 * 	return diff;
 * }
 *
 * // In order to identify identical objects with differing attributes
 * // you must specify the attributes required to uniquely identify
 * // your object. Make sure to not include too many attributes, this
 * // list is used when caches look for an old version of an object.
 * struct nl_object_ops my_ops = {
 * 	...
 * 	.oo_id_attrs		= MY_ATTR_FOO,
 * 	.oo_compare		= my_obj_compare,
 * };
 * @endcode
 * @{
 */

/**
 * Common Object Header
 *
 * This macro must be included as first member in every object
 * definition to allow objects to be cached.
 */
#define NLHDR_COMMON				\
	int			ce_refcnt;	\
	struct nl_object_ops *	ce_ops;		\
	struct nl_cache *	ce_cache;	\
	struct nl_list_head	ce_list;	\
	int			ce_msgtype;	\
	int			ce_flags;	\
	uint32_t		ce_mask;

/**
 * Return true if attribute is available in both objects
 * @arg A		an object
 * @arg B		another object
 * @arg ATTR		attribute bit
 *
 * @return True if the attribute is available, otherwise false is returned.
 */
#define AVAILABLE(A, B, ATTR)	(((A)->ce_mask & (B)->ce_mask) & (ATTR))

/**
 * Return true if attributes mismatch
 * @arg A		an object
 * @arg B		another object
 * @arg ATTR		attribute bit
 * @arg EXPR		Comparison expression
 *
 * This function will check if the attribute in question is available
 * in both objects, if not this will count as a mismatch.
 *
 * If available the function will execute the expression which must
 * return true if the attributes mismatch.
 *
 * @return True if the attribute mismatch, or false if they match.
 */
#define ATTR_MISMATCH(A, B, ATTR, EXPR)	(!AVAILABLE(A, B, ATTR) || (EXPR))

/**
 * Return attribute bit if attribute does not match
 * @arg LIST		list of attributes to be compared
 * @arg ATTR		attribute bit
 * @arg A		an object
 * @arg B		another object
 * @arg EXPR		Comparison expression
 *
 * This function will check if the attribute in question is available
 * in both objects, if not this will count as a mismatch.
 *
 * If available the function will execute the expression which must
 * return true if the attributes mismatch.
 *
 * In case the attributes mismatch, the attribute is returned, otherwise
 * 0 is returned.
 *
 * @code
 * diff |= ATTR_DIFF(attrs, MY_ATTR_FOO, a, b, a->foo != b->foo);
 * @endcode
 */
#define ATTR_DIFF(LIST, ATTR, A, B, EXPR) \
({	int diff = 0; \
	if (((LIST) & (ATTR)) && ATTR_MISMATCH(A, B, ATTR, EXPR)) \
		diff = ATTR; \
	diff; })

/**
 * Object Operations
 */
struct nl_object;
struct nl_object_ops
{
	/**
	 * Unique name of object type
	 *
	 * Must be in the form family/name, e.g. "route/addr"
	 */
	char *		oo_name;

	/** Size of object including its header */
	size_t		oo_size;

	/* List of attributes needed to uniquely identify the object */
	uint32_t	oo_id_attrs;

	/**
	 * Constructor function
	 *
	 * Will be called when a new object of this type is allocated.
	 * Can be used to initialize members such as lists etc.
	 */
	void  (*oo_constructor)(struct nl_object *);

	/**
	 * Destructor function
	 *
	 * Will be called when an object is freed. Must free all
	 * resources which may have been allocated as part of this
	 * object.
	 */
	void  (*oo_free_data)(struct nl_object *);

	/**
	 * Cloning function
	 *
	 * Will be called when an object needs to be cloned. Please
	 * note that the generic object code will make an exact
	 * copy of the object first, therefore you only need to take
	 * care of members which require reference counting etc.
	 *
	 * May return a negative error code to abort cloning.
	 */
	int  (*oo_clone)(struct nl_object *, struct nl_object *);

	/**
	 * Dumping functions
	 *
	 * Will be called when an object is dumped. The implementations
	 * have to use nl_dump(), nl_dump_line(), and nl_new_line() to
	 * dump objects.
	 *
	 * The functions must return the number of lines printed.
	 */
	void (*oo_dump[NL_DUMP_MAX+1])(struct nl_object *,
				       struct nl_dump_params *);

	/**
	 * Comparison function
	 *
	 * Will be called when two objects of the same type are
	 * compared. It takes the two objects in question, an object
	 * specific bitmask defining which attributes should be
	 * compared and flags to control the behaviour.
	 *
	 * The function must return a bitmask with the relevant bit
	 * set for each attribute that mismatches.
	 */
	int   (*oo_compare)(struct nl_object *, struct nl_object *,
			    uint32_t, int);


	char *(*oo_attrs2str)(int, char *, size_t);
};

/** @} */

#ifdef __cplusplus
}
#endif

#endif
