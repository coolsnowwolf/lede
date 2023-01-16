/*
 * sfe_pppoe.c
 *     API for shortcut forwarding engine PPPoE flows
 *
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/skbuff.h>
#include <linux/if_pppox.h>
#include <linux/ppp_defs.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_pppoe.h"

/*
 * sfe_pppoe_add_header()
 *	Add PPPoE header.
 *
 * skb->data will point to PPPoE header after the function
 */
bool sfe_pppoe_add_header(struct sk_buff *skb, u16 pppoe_session_id, u16 ppp_protocol)
{
	u16 *l2_header;
	struct pppoe_hdr *ph;
	struct sfe_ppp_hdr *ppp;
	u16 *l3_header = (u16 *)skb->data;

	/*
	 * Check that we have space for PPPoE header, PPP header (2 bytes) and Ethernet header
	 * TODO: Calculate the l2_hdr_len during rule push so that this is avoided in datapath
	 */
	if (unlikely(skb_headroom(skb) < ((sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)) + ETH_HLEN))) {
		DEBUG_TRACE("%px: Not enough headroom for PPPoE header\n", skb);
		return false;
	}

	/*
	 * PPPoE header (6 bytes) + PPP header (2 bytes)
	 *
	 * Hence move by 8 bytes to accomodate PPPoE header
	 */
	l2_header = l3_header - ((sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)) / 2);

	/*
	 * Headers in skb will look like in below sequence
	 *	| PPPoE hdr(6 bytes) | PPP hdr (2 bytes) | L3 hdr |
	 *
	 *	The length field in the PPPoE header indicates the length of the PPPoE payload which
	 *	consists of a 2-byte PPP header plus a skb->len.
	 */
	ph = (struct pppoe_hdr *)l2_header;
	ph->ver = 1;
	ph->type = 1;
	ph->code = 0;
	ph->sid = htons(pppoe_session_id);
	ph->length = htons(skb->len + sizeof(struct sfe_ppp_hdr));
	skb->protocol = htons(ETH_P_PPP_SES);

	/*
	 * Insert the PPP header protocol
	 */
	ppp = (struct sfe_ppp_hdr *)(l2_header + (sizeof(struct pppoe_hdr) / 2));
	ppp->protocol = htons(ppp_protocol);

	/*
	 * L2 header offset will point to PPPoE header,
	 */
	__skb_push(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));

	return true;
}

/*
 * sfe_pppoe_parse_hdr()
 *	Parse PPPoE header
 *
 * Returns true if the packet is good for further processing.
 */
bool sfe_pppoe_parse_hdr(struct sk_buff *skb, struct sfe_l2_info *l2_info)
{
	unsigned int len;
	int pppoe_len;
	struct sfe_ppp_hdr *ppp;
	struct pppoe_hdr *ph = (struct pppoe_hdr *)skb->data;

	/*
	 * Check that we have space for PPPoE header here.
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr))))) {
		DEBUG_TRACE("%px: packet too short for PPPoE header\n", skb);
		return false;
	}

	len = skb->len;
	pppoe_len = ntohs(ph->length);
	if (unlikely(len < pppoe_len)) {
		DEBUG_TRACE("%px: len: %u is too short to %u\n", skb, len, pppoe_len);
		return false;
	}

	ppp = (struct sfe_ppp_hdr *)((u8*)ph + sizeof(*ph));

	/*
	 * Converting PPP protocol values to ether type protocol values
	 */
	switch(ntohs(ppp->protocol)) {
	case PPP_IP:
		sfe_l2_protocol_set(l2_info, ETH_P_IP);
		break;

	case PPP_IPV6:
		sfe_l2_protocol_set(l2_info, ETH_P_IPV6);
		break;

	case PPP_LCP:
		DEBUG_TRACE("%px: LCP packets are not supported in SFE\n", skb);
		return false;

	default:
		DEBUG_TRACE("%px: Unsupported protocol : %d in PPP header\n", skb, ntohs(ppp->protocol));
		return false;
	}

	sfe_l2_parse_flag_set(l2_info, SFE_L2_PARSE_FLAGS_PPPOE_INGRESS);
	sfe_l2_pppoe_hdr_offset_set(l2_info, (skb->data - skb->head));
	sfe_l2_hdr_size_set(l2_info, (sizeof(struct pppoe_hdr) + sizeof(struct sfe_ppp_hdr)));

	return true;
}
