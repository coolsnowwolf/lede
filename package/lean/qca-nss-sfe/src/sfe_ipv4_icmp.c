/*
 * sfe_ipv4_icmp.c
 *	Shortcut forwarding engine - IPv4 ICMP implementation
 *
 * Copyright (c) 2013-2016, 2019-2020, The Linux Foundation. All rights reserved.
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
#include <linux/lockdep.h>

#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv4.h"

/*
 * sfe_ipv4_recv_icmp()
 *	Handle ICMP packet receives.
 *
 * ICMP packets aren't handled as a "fast path" and always have us process them
 * through the default Linux stack.  What we do need to do is look for any errors
 * about connections we are handling in the fast path.  If we find any such
 * connections then we want to flush their state so that the ICMP error path
 * within Linux has all of the correct state should it need it.
 */
int sfe_ipv4_recv_icmp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
			      unsigned int len, struct iphdr *iph, unsigned int ihl)
{
	struct icmphdr *icmph;
	struct iphdr *icmp_iph;
	unsigned int icmp_ihl_words;
	unsigned int icmp_ihl;
	u32 *icmp_trans_h;
	struct udphdr *icmp_udph;
	struct tcphdr *icmp_tcph;
	__be32 src_ip;
	__be32 dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv4_connection_match *cm;
	struct sfe_ipv4_connection *c;
	u32 pull_len = sizeof(struct icmphdr) + ihl;
	bool ret;

	/*
	 * Is our packet too short to contain a valid ICMP header?
	 */
	len -= ihl;
	if (!pskb_may_pull(skb, pull_len)) {
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE);

		DEBUG_TRACE("packet too short for ICMP header\n");
		return 0;
	}

	/*
	 * We only handle "destination unreachable" and "time exceeded" messages.
	 */
	icmph = (struct icmphdr *)(skb->data + ihl);
	if ((icmph->type != ICMP_DEST_UNREACH)
	    && (icmph->type != ICMP_TIME_EXCEEDED)) {

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE);
		DEBUG_TRACE("unhandled ICMP type: 0x%x\n", icmph->type);
		return 0;
	}

	/*
	 * Do we have the full embedded IP header?
	 */
	len -= sizeof(struct icmphdr);
	pull_len += sizeof(struct iphdr);
	if (!pskb_may_pull(skb, pull_len)) {

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE);
		DEBUG_TRACE("Embedded IP header not complete\n");
		return 0;
	}

	/*
	 * Is our embedded IP version wrong?
	 */
	icmp_iph = (struct iphdr *)(icmph + 1);
	if (unlikely(icmp_iph->version != 4)) {

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4);
		DEBUG_TRACE("IP version: %u\n", icmp_iph->version);
		return 0;
	}

	/*
	 * Do we have the full embedded IP header, including any options?
	 */
	icmp_ihl_words = icmp_iph->ihl;
	icmp_ihl = icmp_ihl_words << 2;
	pull_len += icmp_ihl - sizeof(struct iphdr);
	if (!pskb_may_pull(skb, pull_len)) {

		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE);
		DEBUG_TRACE("Embedded header not large enough for IP options\n");
		return 0;
	}

	len -= icmp_ihl;
	icmp_trans_h = ((u32 *)icmp_iph) + icmp_ihl_words;

	/*
	 * Handle the embedded transport layer header.
	 */
	switch (icmp_iph->protocol) {
	case IPPROTO_UDP:
		/*
		 * We should have 8 bytes of UDP header - that's enough to identify
		 * the connection.
		 */
		pull_len += 8;
		if (!pskb_may_pull(skb, pull_len)) {
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE);
			DEBUG_TRACE("Incomplete embedded UDP header\n");
			return 0;
		}

		icmp_udph = (struct udphdr *)icmp_trans_h;
		src_port = icmp_udph->source;
		dest_port = icmp_udph->dest;
		break;

	case IPPROTO_TCP:
		/*
		 * We should have 8 bytes of TCP header - that's enough to identify
		 * the connection.
		 */
		pull_len += 8;
		if (!pskb_may_pull(skb, pull_len)) {
			sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE);
			DEBUG_TRACE("Incomplete embedded TCP header\n");
			return 0;
		}

		icmp_tcph = (struct tcphdr *)icmp_trans_h;
		src_port = icmp_tcph->source;
		dest_port = icmp_tcph->dest;
		break;

	default:
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL);
		DEBUG_TRACE("Unhandled embedded IP protocol: %u\n", icmp_iph->protocol);
		return 0;
	}

	src_ip = icmp_iph->saddr;
	dest_ip = icmp_iph->daddr;

	rcu_read_lock();

	/*
	 * Look for a connection match.  Note that we reverse the source and destination
	 * here because our embedded message contains a packet that was sent in the
	 * opposite direction to the one in which we just received it.  It will have
	 * been sent on the interface from which we received it though so that's still
	 * ok to use.
	 */
	cm = sfe_ipv4_find_connection_match_rcu(si, dev, icmp_iph->protocol, dest_ip, dest_port, src_ip, src_port);
	if (unlikely(!cm)) {

		rcu_read_unlock();
		sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION);
		DEBUG_TRACE("no connection found\n");
		return 0;
	}

	/*
	 * We found a connection so now remove it from the connection list and flush
	 * its state.
	 */
	c = cm->connection;
	spin_lock_bh(&si->lock);
	ret = sfe_ipv4_remove_connection(si, c);
	spin_unlock_bh(&si->lock);

	if (ret) {
		sfe_ipv4_flush_connection(si, c, SFE_SYNC_REASON_FLUSH);
	}
	rcu_read_unlock();
	sfe_ipv4_exception_stats_inc(si, SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION);
	return 0;
}
