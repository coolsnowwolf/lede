/*
 * lib/genl/family.c		Generic Netlink Family
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2003-2006 Thomas Graf <tgraf@suug.ch>
 */

/**
 * @ingroup genl
 * @defgroup genl_family Generic Netlink Family
 * @brief
 *
 * @{
 */

#include <netlink-generic.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/utils.h>

struct nl_object_ops genl_family_ops;
/** @endcond */

static void family_constructor(struct nl_object *c)
{
	struct genl_family *family = (struct genl_family *) c;

	nl_init_list_head(&family->gf_ops);
	nl_init_list_head(&family->gf_mc_grps);
}

static void family_free_data(struct nl_object *c)
{
	struct genl_family *family = (struct genl_family *) c;
	struct genl_family_op *ops, *tmp;
	struct genl_family_grp *grp, *t_grp;

	if (family == NULL)
		return;

	nl_list_for_each_entry_safe(ops, tmp, &family->gf_ops, o_list) {
		nl_list_del(&ops->o_list);
		free(ops);
	}

	nl_list_for_each_entry_safe(grp, t_grp, &family->gf_mc_grps, list) {
		nl_list_del(&grp->list);
		free(grp);
	}

}

static int family_clone(struct nl_object *_dst, struct nl_object *_src)
{
	struct genl_family *dst = nl_object_priv(_dst);
	struct genl_family *src = nl_object_priv(_src);
	struct genl_family_op *ops;
	struct genl_family_grp *grp;
	int err;

	nl_list_for_each_entry(ops, &src->gf_ops, o_list) {
		err = genl_family_add_op(dst, ops->o_id, ops->o_flags);
		if (err < 0)
			return err;
	}

	nl_list_for_each_entry(grp, &src->gf_mc_grps, list) {
		err = genl_family_add_grp(dst, grp->id, grp->name);
		if (err < 0)
			return err;
	}

	
	return 0;
}

static int family_compare(struct nl_object *_a, struct nl_object *_b,
			  uint32_t attrs, int flags)
{
	struct genl_family *a = (struct genl_family *) _a;
	struct genl_family *b = (struct genl_family *) _b;
	int diff = 0;

#define FAM_DIFF(ATTR, EXPR) ATTR_DIFF(attrs, FAMILY_ATTR_##ATTR, a, b, EXPR)

	diff |= FAM_DIFF(ID,		a->gf_id != b->gf_id);
	diff |= FAM_DIFF(VERSION,	a->gf_version != b->gf_version);
	diff |= FAM_DIFF(HDRSIZE,	a->gf_hdrsize != b->gf_hdrsize);
	diff |= FAM_DIFF(MAXATTR,	a->gf_maxattr != b->gf_maxattr);
	diff |= FAM_DIFF(NAME,		strcmp(a->gf_name, b->gf_name));

#undef FAM_DIFF

	return diff;
}


/**
 * @name Family Object
 * @{
 */

struct genl_family *genl_family_alloc(void)
{
	return (struct genl_family *) nl_object_alloc(&genl_family_ops);
}

void genl_family_put(struct genl_family *family)
{
	nl_object_put((struct nl_object *) family);
}

/** @} */


int genl_family_add_op(struct genl_family *family, int id, int flags)
{
	struct genl_family_op *op;

	op = calloc(1, sizeof(*op));
	if (op == NULL)
		return -NLE_NOMEM;

	op->o_id = id;
	op->o_flags = flags;

	nl_list_add_tail(&op->o_list, &family->gf_ops);
	family->ce_mask |= FAMILY_ATTR_OPS;

	return 0;
}

int genl_family_add_grp(struct genl_family *family, uint32_t id,
			const char *name)
{
	struct genl_family_grp *grp;

	grp = calloc(1, sizeof(*grp));
	if (grp == NULL)
		return -NLE_NOMEM;

	grp->id = id;
	strncpy(grp->name, name, GENL_NAMSIZ - 1);

	nl_list_add_tail(&grp->list, &family->gf_mc_grps);

	return 0;
}

/** @} */

/** @cond SKIP */
struct nl_object_ops genl_family_ops = {
	.oo_name		= "genl/family",
	.oo_size		= sizeof(struct genl_family),
	.oo_constructor		= family_constructor,
	.oo_free_data		= family_free_data,
	.oo_clone		= family_clone,
	.oo_compare		= family_compare,
	.oo_id_attrs		= FAMILY_ATTR_ID,
};
/** @endcond */

/** @} */
