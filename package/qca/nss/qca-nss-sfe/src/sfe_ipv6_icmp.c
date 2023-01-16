/*
 * sfe_ipv6_icmp.c
 *	Shortcut forwarding engine file for IPv6 ICMP
 *
 * Copyright (c) 2015-2016, 2019-2020, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <linux/ip.h>
#include <net/udp.h>
#include <net/tcp.h>
#include <net/icmp.h>
#include <linux/etherdevice.h>
#include <linux/version.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv6.h"

/*
 * sfe_ipv6_recv_icmp()
 *	Handle ICMP packet receives.
 *
 * ICMP packets aren't handled as a "fast path" and always have us process them
 * through the default Linux stack.  What we do need to do is look for any errors
 * about connections we are handling in the fast path.  If we find any such
 * connections then we want to flush their state so that the ICMP error path
 * within Linux has all of the correct state should it need it.
 */
int sfe_ipv6_recv_icmp(struct sfe_ipv6 *si, struct sk_buff *skb, struct net_device *dev,
			      unsigned int len, struct ipv6hdr *iph, unsigned int ihl)
{
	struct icmp6hdr *icmph;
	struct ipv6hdr *icmp_iph;
	struct udphdr *icmp_udph;
	struct tcphdr *icmp_tcph;
	struct sfe_ipv6_addr *src_ip;
	struct sfe_ipv6_addr *dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv6_connection_match *cm;
	struct sfe_ipv6_connection *c;
	u8 next_hdr;
	bool ret;

	/*
	 * Is our packet too short to contain a valid ICMP header?
	 */
	len -= ihl;
	if (!pskb_may_pull(skb, ihl + sizeof(struct icmp6hdr))) {
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE);

		DEBUG_TRACE("packet too short for ICMP header\n");
		return 0;
	}

	/*
	 * We only handle "destination unreachable" and "time exceeded" messages.
	 */
	icmph = (struct icmp6hdr *)(skb->data + ihl);
	if ((icmph->icmp6_type != ICMPV6_DEST_UNREACH)
	    && (icmph->icmp6_type != ICMPV6_TIME_EXCEED)) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE);
		DEBUG_TRACE("unhandled ICMP type: 0x%x\n", icmph->icmp6_type);
		return 0;
	}

	/*
	 * Do we have the full embedded IP header?
	 * We should have 8 bytes of next L4 header - that's enough to identify
	 * the connection.
	 */
	len -= sizeof(struct icmp6hdr);
	ihl += sizeof(struct icmp6hdr);
	if (!pskb_may_pull(skb, ihl + sizeof(struct ipv6hdr) + sizeof(struct sfe_ipv6_ext_hdr))) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_HEADER_INCOMPLETE);
		DEBUG_TRACE("Embedded IP header not complete\n");
		return 0;
	}

	/*
	 * Is our embedded IP version wrong?
	 */
	icmp_iph = (struct ipv6hdr *)(icmph + 1);
	if (unlikely(icmp_iph->version != 6)) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_NON_V6);
		DEBUG_TRACE("IP version: %u\n", icmp_iph->version);
		return 0;
	}

	len -= sizeof(struct ipv6hdr);
	ihl += sizeof(struct ipv6hdr);
	next_hdr = icmp_iph->nexthdr;
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
		struct sfe_ipv6_ext_hdr *ext_hdr;
		unsigned int ext_hdr_len;

		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);
		if (next_hdr == NEXTHDR_FRAGMENT) {
			struct frag_hdr *frag_hdr = (struct frag_hdr *)ext_hdr;
			unsigned int frag_off = ntohs(frag_hdr->frag_off);

			if (frag_off & SFE_IPV6_FRAG_OFFSET) {

				DEBUG_TRACE("non-initial fragment\n");
				sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT);
				return 0;
			}
		}

		ext_hdr_len = ext_hdr->hdr_len;
		ext_hdr_len <<= 3;
		ext_hdr_len += sizeof(struct sfe_ipv6_ext_hdr);
		len -= ext_hdr_len;
		ihl += ext_hdr_len;
		/*
		 * We should have 8 bytes of next header - that's enough to identify
		 * the connection.
		 */
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {

			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE);
			DEBUG_TRACE("extension header %d not completed\n", next_hdr);
			return 0;
		}

		next_hdr = ext_hdr->next_hdr;
	}

	/*
	 * Handle the embedded transport layer header.
	 */
	switch (next_hdr) {
	case IPPROTO_UDP:
		icmp_udph = (struct udphdr *)(skb->data + ihl);
		src_port = icmp_udph->source;
		dest_port = icmp_udph->dest;
		break;

	case IPPROTO_TCP:
		icmp_tcph = (struct tcphdr *)(skb->data + ihl);
		src_port = icmp_tcph->source;
		dest_port = icmp_tcph->dest;
		break;

	default:

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_IPV6_UNHANDLED_PROTOCOL);
		DEBUG_TRACE("Unhandled embedded IP protocol: %u\n", next_hdr);
		return 0;
	}

	src_ip = (struct sfe_ipv6_addr *)icmp_iph->saddr.s6_addr32;
	dest_ip = (struct sfe_ipv6_addr *)icmp_iph->daddr.s6_addr32;

	rcu_read_lock();
	/*
	 * Look for a connection match.  Note that we reverse the source and destination
	 * here because our embedded message contains a packet that was sent in the
	 * opposite direction to the one in which we just received it.  It will have
	 * been sent on the interface from which we received it though so that's still
	 * ok to use.
	 */
	cm = sfe_ipv6_find_connection_match_rcu(si, dev, icmp_iph->nexthdr, dest_ip, dest_port, src_ip, src_port);
	if (unlikely(!cm)) {
		rcu_read_unlock();
		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_NO_CONNECTION);
		DEBUG_TRACE("no connection found\n");
		return 0;
	}

	/*
	 * We found a connection so now remove it from the connection list and flush
	 * its state.
	 */
	c = cm->connection;
	spin_lock_bh(&si->lock);
	ret = sfe_ipv6_remove_connection(si, c);
	spin_unlock_bh(&si->lock);

	if (ret) {
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
	}

	rcu_read_unlock();

	sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION);
	return 0;
}
