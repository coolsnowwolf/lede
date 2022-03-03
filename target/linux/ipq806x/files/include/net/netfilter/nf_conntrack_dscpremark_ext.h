/*
 **************************************************************************
 * Copyright (c) 2014-2015, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/* DSCP remark conntrack extension APIs. */

#ifndef _NF_CONNTRACK_DSCPREMARK_H
#define _NF_CONNTRACK_DSCPREMARK_H
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>

/* Rule flags */
#define NF_CT_DSCPREMARK_EXT_DSCP_RULE_VALID 0x1

/* Rule validity */
#define NF_CT_DSCPREMARK_EXT_RULE_VALID 0x1
#define NF_CT_DSCPREMARK_EXT_RULE_NOT_VALID 0x0

/*
 * DSCP remark conntrack extension structure.
 */
struct nf_ct_dscpremark_ext {
	__u32 flow_priority;	/* Original direction packet priority */
	__u32 reply_priority;	/* Reply direction packet priority */
	__u16 igs_flow_qos_tag;	/* Original direction ingress packet priority */
	__u16 igs_reply_qos_tag;	/* Reply direction ingress packet priority */
	__u8 flow_dscp;		/* IP DSCP value for original direction */
	__u8 reply_dscp;	/* IP DSCP value for reply direction */
	__u16 rule_flags;	/* Rule Validity flags */
};

/*
 * nf_ct_dscpremark_ext_find()
 *	Finds the extension data of the conntrack entry if it exists.
 */
static inline struct nf_ct_dscpremark_ext *
nf_ct_dscpremark_ext_find(const struct nf_conn *ct)
{
#ifdef CONFIG_NF_CONNTRACK_DSCPREMARK_EXT
	return nf_ct_ext_find(ct, NF_CT_EXT_DSCPREMARK);
#else
	return NULL;
#endif
}

/*
 * nf_ct_dscpremark_ext_add()
 *	Adds the extension data to the conntrack entry.
 */
static inline
struct nf_ct_dscpremark_ext *nf_ct_dscpremark_ext_add(struct nf_conn *ct,
						      gfp_t gfp)
{
#ifdef CONFIG_NF_CONNTRACK_DSCPREMARK_EXT
	struct nf_ct_dscpremark_ext *ncde;

	ncde = nf_ct_ext_add(ct, NF_CT_EXT_DSCPREMARK, gfp);
	if (!ncde)
		return NULL;

	return ncde;
#else
	return NULL;
#endif
};

#ifdef CONFIG_NF_CONNTRACK_DSCPREMARK_EXT
extern int nf_conntrack_dscpremark_ext_init(void);
extern void nf_conntrack_dscpremark_ext_fini(void);
extern int nf_conntrack_dscpremark_ext_set_dscp_rule_valid(struct nf_conn *ct);
extern int
nf_conntrack_dscpremark_ext_get_dscp_rule_validity(struct nf_conn *ct);
#else
/*
 * nf_conntrack_dscpremark_ext_init()
 */
static inline int nf_conntrack_dscpremark_ext_init(void)
{
	return 0;
}

/*
 * nf_conntrack_dscpremark_ext_fini()
 */
static inline void nf_conntrack_dscpremark_ext_fini(void)
{
}
#endif /* CONFIG_NF_CONNTRACK_DSCPREMARK_EXT */
#endif /* _NF_CONNTRACK_DSCPREMARK_H */
