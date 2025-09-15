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

/* DSCP remark handling conntrack extension registration. */

#include <linux/netfilter.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/export.h>

#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_extend.h>
#include <net/netfilter/nf_conntrack_dscpremark_ext.h>

/* nf_conntrack_dscpremark_ext_set_dscp_rule_valid()
 *	Set DSCP rule validity flag in the extension
 */
int nf_conntrack_dscpremark_ext_set_dscp_rule_valid(struct nf_conn *ct)
{
	struct nf_ct_dscpremark_ext *ncde;

	ncde = nf_ct_dscpremark_ext_find(ct);
	if (!ncde)
		return -1;

	ncde->rule_flags = NF_CT_DSCPREMARK_EXT_DSCP_RULE_VALID;
	return 0;
}
EXPORT_SYMBOL(nf_conntrack_dscpremark_ext_set_dscp_rule_valid);

/* nf_conntrack_dscpremark_ext_get_dscp_rule_validity()
 *	Check if the DSCP rule flag is valid from the extension
 */
int nf_conntrack_dscpremark_ext_get_dscp_rule_validity(struct nf_conn *ct)
{
	struct nf_ct_dscpremark_ext *ncde;

	ncde = nf_ct_dscpremark_ext_find(ct);
	if (!ncde)
		return NF_CT_DSCPREMARK_EXT_RULE_NOT_VALID;

	if (ncde->rule_flags & NF_CT_DSCPREMARK_EXT_DSCP_RULE_VALID)
		return NF_CT_DSCPREMARK_EXT_RULE_VALID;

	return NF_CT_DSCPREMARK_EXT_RULE_NOT_VALID;
}
EXPORT_SYMBOL(nf_conntrack_dscpremark_ext_get_dscp_rule_validity);
