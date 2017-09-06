/*
 * netlink/genl/family.h	Generic Netlink Family
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

#ifndef NETLINK_GENL_FAMILY_H_
#define NETLINK_GENL_FAMILY_H_

#include <netlink/netlink.h>
#include <netlink/cache.h>

#ifdef __cplusplus
extern "C" {
#endif

/** @cond SKIP */
#define FAMILY_ATTR_ID		0x01
#define FAMILY_ATTR_NAME	0x02
#define FAMILY_ATTR_VERSION	0x04
#define FAMILY_ATTR_HDRSIZE	0x08
#define FAMILY_ATTR_MAXATTR	0x10
#define FAMILY_ATTR_OPS		0x20


struct genl_family
{
	NLHDR_COMMON

	uint16_t		gf_id;
	char 			gf_name[GENL_NAMSIZ];
	uint32_t		gf_version;
	uint32_t		gf_hdrsize;
	uint32_t		gf_maxattr;

	struct nl_list_head	gf_ops;
	struct nl_list_head	gf_mc_grps;
};


extern struct genl_family *	genl_family_alloc(void);
extern void			genl_family_put(struct genl_family *);

extern int			genl_family_add_op(struct genl_family *,
						   int, int);
extern int 			genl_family_add_grp(struct genl_family *,
					uint32_t , const char *);


/**
 * @name Attributes
 * @{
 */

static inline unsigned int genl_family_get_id(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_ID)
		return family->gf_id;
	else
		return 0;
}

static inline void genl_family_set_id(struct genl_family *family, unsigned int id)
{
	family->gf_id = id;
	family->ce_mask |= FAMILY_ATTR_ID;
}

static inline char *genl_family_get_name(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_NAME)
		return family->gf_name;
	else
		return NULL;
}

static inline void genl_family_set_name(struct genl_family *family, const char *name)
{
	strncpy(family->gf_name, name, GENL_NAMSIZ-1);
	family->ce_mask |= FAMILY_ATTR_NAME;
}

static inline uint8_t genl_family_get_version(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_VERSION)
		return family->gf_version;
	else
		return 0;
}

static inline void genl_family_set_version(struct genl_family *family, uint8_t version)
{
	family->gf_version = version;
	family->ce_mask |= FAMILY_ATTR_VERSION;
}

static inline uint32_t genl_family_get_hdrsize(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_HDRSIZE)
		return family->gf_hdrsize;
	else
		return 0;
}

static inline void genl_family_set_hdrsize(struct genl_family *family, uint32_t hdrsize)
{
	family->gf_hdrsize = hdrsize;
	family->ce_mask |= FAMILY_ATTR_HDRSIZE;
}

static inline uint32_t genl_family_get_maxattr(struct genl_family *family)
{
	if (family->ce_mask & FAMILY_ATTR_MAXATTR)
		return family->gf_maxattr;
	else
		return family->gf_maxattr;
}

static inline void genl_family_set_maxattr(struct genl_family *family, uint32_t maxattr)
{
	family->gf_maxattr = maxattr;
	family->ce_mask |= FAMILY_ATTR_MAXATTR;
}

#ifdef __cplusplus
}
#endif

#endif
