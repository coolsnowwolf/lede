/* Copyright (c) 2013-2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * RMNET Data MAP protocol
 *
 */

#include <linux/netdevice.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ip6_checksum.h>
#include "rmnet_config.h"
#include "rmnet_map.h"
#include "rmnet_private.h"
#include "rmnet_handlers.h"

#define RMNET_MAP_PKT_COPY_THRESHOLD 64
#define RMNET_MAP_DEAGGR_SPACING  64
#define RMNET_MAP_DEAGGR_HEADROOM (RMNET_MAP_DEAGGR_SPACING / 2)

struct rmnet_map_coal_metadata {
	void *ip_header;
	void *trans_header;
	u16 ip_len;
	u16 trans_len;
	u16 data_offset;
	u16 data_len;
	u8 ip_proto;
	u8 trans_proto;
	u8 pkt_id;
	u8 pkt_count;
};

static __sum16 *rmnet_map_get_csum_field(unsigned char protocol,
					 const void *txporthdr)
{
	__sum16 *check = NULL;

	switch (protocol) {
	case IPPROTO_TCP:
		check = &(((struct tcphdr *)txporthdr)->check);
		break;

	case IPPROTO_UDP:
		check = &(((struct udphdr *)txporthdr)->check);
		break;

	default:
		check = NULL;
		break;
	}

	return check;
}

static void rmnet_map_complement_ipv4_txporthdr_csum_field(void *iphdr)
{
	struct iphdr *ip4h = (struct iphdr *)iphdr;
	void *txphdr;
	u16 *csum;

	txphdr = iphdr + ip4h->ihl * 4;

	if (ip4h->protocol == IPPROTO_TCP || ip4h->protocol == IPPROTO_UDP) {
		csum = (u16 *)rmnet_map_get_csum_field(ip4h->protocol, txphdr);
		*csum = ~(*csum);
	}
}

static void
rmnet_map_ipv4_ul_csum_header(void *iphdr,
			      struct rmnet_map_ul_csum_header *ul_header,
			      struct sk_buff *skb)
{
	struct iphdr *ip4h = (struct iphdr *)iphdr;
	__be16 *hdr = (__be16 *)ul_header, offset;

	offset = htons((__force u16)(skb_transport_header(skb) -
				     (unsigned char *)iphdr));
	ul_header->csum_start_offset = offset;
	ul_header->csum_insert_offset = skb->csum_offset;
	ul_header->csum_enabled = 1;
	if (ip4h->protocol == IPPROTO_UDP)
		ul_header->udp_ind = 1;
	else
		ul_header->udp_ind = 0;

	/* Changing remaining fields to network order */
	hdr++;
	*hdr = htons((__force u16)*hdr);

	skb->ip_summed = CHECKSUM_NONE;

	rmnet_map_complement_ipv4_txporthdr_csum_field(iphdr);
}

#if IS_ENABLED(CONFIG_IPV6)
static void rmnet_map_complement_ipv6_txporthdr_csum_field(void *ip6hdr)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)ip6hdr;
	void *txphdr;
	u16 *csum;

	txphdr = ip6hdr + sizeof(struct ipv6hdr);

	if (ip6h->nexthdr == IPPROTO_TCP || ip6h->nexthdr == IPPROTO_UDP) {
		csum = (u16 *)rmnet_map_get_csum_field(ip6h->nexthdr, txphdr);
		*csum = ~(*csum);
	}
}

static void
rmnet_map_ipv6_ul_csum_header(void *ip6hdr,
			      struct rmnet_map_ul_csum_header *ul_header,
			      struct sk_buff *skb)
{
	struct ipv6hdr *ip6h = (struct ipv6hdr *)ip6hdr;
	__be16 *hdr = (__be16 *)ul_header, offset;

	offset = htons((__force u16)(skb_transport_header(skb) -
				     (unsigned char *)ip6hdr));
	ul_header->csum_start_offset = offset;
	ul_header->csum_insert_offset = skb->csum_offset;
	ul_header->csum_enabled = 1;

	if (ip6h->nexthdr == IPPROTO_UDP)
		ul_header->udp_ind = 1;
	else
		ul_header->udp_ind = 0;

	/* Changing remaining fields to network order */
	hdr++;
	*hdr = htons((__force u16)*hdr);

	skb->ip_summed = CHECKSUM_NONE;

	rmnet_map_complement_ipv6_txporthdr_csum_field(ip6hdr);
}
#endif

/* Adds MAP header to front of skb->data
 * Padding is calculated and set appropriately in MAP header. Mux ID is
 * initialized to 0.
 */
static struct rmnet_map_header *rmnet_map_add_map_header(struct sk_buff *skb,
						  int hdrlen, int pad,
						  struct rmnet_port *port)
{
	struct rmnet_map_header *map_header;
	u32 padding, map_datalen;
	u8 *padbytes;

	map_datalen = skb->len - hdrlen;
	map_header = (struct rmnet_map_header *)
			skb_push(skb, sizeof(struct rmnet_map_header));
	memset(map_header, 0, sizeof(struct rmnet_map_header));

	/* Set next_hdr bit for csum offload packets */
	if (port->data_format & RMNET_FLAGS_EGRESS_MAP_CKSUMV5)
		map_header->next_hdr = 1;

	if (pad == RMNET_MAP_NO_PAD_BYTES) {
		map_header->pkt_len = htons(map_datalen);
		return map_header;
	}

	padding = ALIGN(map_datalen, 4) - map_datalen;

	if (padding == 0)
		goto done;

	if (skb_tailroom(skb) < padding)
		return NULL;

	padbytes = (u8 *)skb_put(skb, padding);
	memset(padbytes, 0, padding);

done:
	map_header->pkt_len = htons(map_datalen + padding);
	map_header->pad_len = padding & 0x3F;

	return map_header;
}

/* Deaggregates a single packet
 * A whole new buffer is allocated for each portion of an aggregated frame.
 * Caller should keep calling deaggregate() on the source skb until 0 is
 * returned, indicating that there are no more packets to deaggregate. Caller
 * is responsible for freeing the original skb.
 */
static struct sk_buff *rmnet_map_deaggregate(struct sk_buff *skb,
				      struct rmnet_port *port)
{
	struct rmnet_map_header *maph;
	struct sk_buff *skbn;
	unsigned char *data = rmnet_map_data_ptr(skb), *next_hdr = NULL;
	u32 packet_len;

	if (skb->len == 0)
		return NULL;

	maph = (struct rmnet_map_header *)data;
	packet_len = ntohs(maph->pkt_len) + sizeof(struct rmnet_map_header);

	if (port->data_format & RMNET_FLAGS_INGRESS_MAP_CKSUMV4)
		packet_len += sizeof(struct rmnet_map_dl_csum_trailer);
	else if (port->data_format & RMNET_FLAGS_INGRESS_MAP_CKSUMV5) {
		if (!maph->cd_bit) {
			packet_len += sizeof(struct rmnet_map_v5_csum_header);

			/* Coalescing headers require MAPv5 */
			next_hdr = data + sizeof(*maph);
		}
	}

	if (((int)skb->len - (int)packet_len) < 0)
		return NULL;

	/* Some hardware can send us empty frames. Catch them */
	if (ntohs(maph->pkt_len) == 0)
		return NULL;

	if (next_hdr &&
	    ((struct rmnet_map_v5_coal_header *)next_hdr)->header_type ==
	     RMNET_MAP_HEADER_TYPE_COALESCING)
		return skb;

	if (skb_is_nonlinear(skb)) {
		skb_frag_t *frag0 = skb_shinfo(skb)->frags;
		struct page *page = skb_frag_page(frag0);

		skbn = alloc_skb(RMNET_MAP_DEAGGR_HEADROOM, GFP_ATOMIC);
		if (!skbn)
			return NULL;

		skb_append_pagefrags(skbn, page, frag0->page_offset,
				     packet_len);
		skbn->data_len += packet_len;
		skbn->len += packet_len;
	} else {
		skbn = alloc_skb(packet_len + RMNET_MAP_DEAGGR_SPACING,
				 GFP_ATOMIC);
		if (!skbn)
			return NULL;

		skb_reserve(skbn, RMNET_MAP_DEAGGR_HEADROOM);
		skb_put(skbn, packet_len);
		memcpy(skbn->data, data, packet_len);
	}

	pskb_pull(skb, packet_len);

	return skbn;
}

static void rmnet_map_v4_checksum_uplink_packet(struct sk_buff *skb,
					 struct net_device *orig_dev)
{
	struct rmnet_priv *priv = netdev_priv(orig_dev);
	struct rmnet_map_ul_csum_header *ul_header;
	void *iphdr;

	ul_header = (struct rmnet_map_ul_csum_header *)
		    skb_push(skb, sizeof(struct rmnet_map_ul_csum_header));

	if (unlikely(!(orig_dev->features &
		     (NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM))))
		goto sw_csum;

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		iphdr = (char *)ul_header +
			sizeof(struct rmnet_map_ul_csum_header);

		if (skb->protocol == htons(ETH_P_IP)) {
			rmnet_map_ipv4_ul_csum_header(iphdr, ul_header, skb);
			priv->stats.csum_hw++;
			return;
		} else if (skb->protocol == htons(ETH_P_IPV6)) {
#if IS_ENABLED(CONFIG_IPV6)
			rmnet_map_ipv6_ul_csum_header(iphdr, ul_header, skb);
			priv->stats.csum_hw++;
			return;
#else
			priv->stats.csum_err_invalid_ip_version++;
			goto sw_csum;
#endif
		} else {
			priv->stats.csum_err_invalid_ip_version++;
		}
	}

sw_csum:
	ul_header->csum_start_offset = 0;
	ul_header->csum_insert_offset = 0;
	ul_header->csum_enabled = 0;
	ul_header->udp_ind = 0;

	priv->stats.csum_sw++;
}

static void rmnet_map_v5_checksum_uplink_packet(struct sk_buff *skb,
					 struct net_device *orig_dev)
{
	struct rmnet_priv *priv = netdev_priv(orig_dev);
	struct rmnet_map_v5_csum_header *ul_header;

	ul_header = (struct rmnet_map_v5_csum_header *)
		    skb_push(skb, sizeof(*ul_header));
	memset(ul_header, 0, sizeof(*ul_header));
	ul_header->header_type = RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD;

	if (skb->ip_summed == CHECKSUM_PARTIAL) {
		void *iph = (char *)ul_header + sizeof(*ul_header);
		void *trans;
		__sum16 *check;
		u8 proto;

		if (skb->protocol == htons(ETH_P_IP)) {
			u16 ip_len = ((struct iphdr *)iph)->ihl * 4;

			proto = ((struct iphdr *)iph)->protocol;
			trans = iph + ip_len;
		} else if (skb->protocol == htons(ETH_P_IPV6)) {
			u16 ip_len = sizeof(struct ipv6hdr);

			proto = ((struct ipv6hdr *)iph)->nexthdr;
			trans = iph + ip_len;
		} else {
			priv->stats.csum_err_invalid_ip_version++;
			goto sw_csum;
		}

		check = rmnet_map_get_csum_field(proto, trans);
		if (check) {
			*check = 0;
			skb->ip_summed = CHECKSUM_NONE;
			/* Ask for checksum offloading */
			ul_header->csum_valid_required = 1;
			priv->stats.csum_hw++;
			return;
		}
	}

sw_csum:
	priv->stats.csum_sw++;
}


/* Generates UL checksum meta info header for IPv4 and IPv6 over TCP and UDP
 * packets that are supported for UL checksum offload.
 */
void rmnet_map_checksum_uplink_packet(struct sk_buff *skb,
				      struct net_device *orig_dev,
				      int csum_type)
{
	switch (csum_type) {
	case RMNET_FLAGS_EGRESS_MAP_CKSUMV4:
		rmnet_map_v4_checksum_uplink_packet(skb, orig_dev);
		break;
	case RMNET_FLAGS_EGRESS_MAP_CKSUMV5:
		rmnet_map_v5_checksum_uplink_packet(skb, orig_dev);
		break;
	default:
		break;
	}
}

static void rmnet_map_move_headers(struct sk_buff *skb)
{
	struct iphdr *iph;
	u16 ip_len;
	u16 trans_len = 0;
	u8 proto;

	/* This only applies to non-linear SKBs */
	if (!skb_is_nonlinear(skb))
		return;

	iph = (struct iphdr *)rmnet_map_data_ptr(skb);
	if (iph->version == 4) {
		ip_len = iph->ihl * 4;
		proto = iph->protocol;
		if (iph->frag_off & htons(IP_OFFSET))
			/* No transport header information */
			goto pull;
	} else if (iph->version == 6) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)iph;
		__be16 frag_off;
		u8 nexthdr = ip6h->nexthdr;

		ip_len = ipv6_skip_exthdr(skb, sizeof(*ip6h), &nexthdr,
					  &frag_off);
		if (ip_len < 0)
			return;

		proto = nexthdr;
	} else {
		return;
	}

	if (proto == IPPROTO_TCP) {
		struct tcphdr *tp = (struct tcphdr *)((u8 *)iph + ip_len);

		trans_len = tp->doff * 4;
	} else if (proto == IPPROTO_UDP) {
		trans_len = sizeof(struct udphdr);
	} else if (proto == NEXTHDR_FRAGMENT) {
		/* Non-first fragments don't have the fragment length added by
		 * ipv6_skip_exthdr() and sho up as proto NEXTHDR_FRAGMENT, so
		 * we account for the length here.
		 */
		ip_len += sizeof(struct frag_hdr);
	}

pull:
	__pskb_pull_tail(skb, ip_len + trans_len);
	skb_reset_network_header(skb);
	if (trans_len)
		skb_set_transport_header(skb, ip_len);
}


/* Process a QMAPv5 packet header */
static int rmnet_map_process_next_hdr_packet(struct sk_buff *skb,
				      struct sk_buff_head *list,
				      u16 len)
{
	struct rmnet_priv *priv = netdev_priv(skb->dev);
	int rc = 0;

	switch (rmnet_map_get_next_hdr_type(skb)) {
	case RMNET_MAP_HEADER_TYPE_COALESCING:
		priv->stats.coal.coal_rx++;
		break;
	case RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD:
		if (rmnet_map_get_csum_valid(skb)) {
			priv->stats.csum_ok++;
			skb->ip_summed = CHECKSUM_UNNECESSARY;
		} else {
			priv->stats.csum_valid_unset++;
		}

		/* Pull unnecessary headers and move the rest to the linear
		 * section of the skb.
		 */
		pskb_pull(skb,
			  (sizeof(struct rmnet_map_header) +
			   sizeof(struct rmnet_map_v5_csum_header)));
		rmnet_map_move_headers(skb);

		/* Remove padding only for csum offload packets.
		 * Coalesced packets should never have padding.
		 */
		pskb_trim(skb, len);
		__skb_queue_tail(list, skb);
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

long rmnet_agg_time_limit __read_mostly = 1000000L;
long rmnet_agg_bypass_time __read_mostly = 10000000L;

static int rmnet_map_tx_agg_skip(struct sk_buff *skb, int offset)
{
	u8 *packet_start = skb->data + offset;
	int is_icmp = 0;

	if (skb->protocol == htons(ETH_P_IP)) {
		struct iphdr *ip4h = (struct iphdr *)(packet_start);

		if (ip4h->protocol == IPPROTO_ICMP)
			is_icmp = 1;
	} else if (skb->protocol == htons(ETH_P_IPV6)) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)(packet_start);

		if (ip6h->nexthdr == IPPROTO_ICMPV6) {
			is_icmp = 1;
		} else if (ip6h->nexthdr == NEXTHDR_FRAGMENT) {
			struct frag_hdr *frag;

			frag = (struct frag_hdr *)(packet_start
						   + sizeof(struct ipv6hdr));
			if (frag->nexthdr == IPPROTO_ICMPV6)
				is_icmp = 1;
		}
	}

	return is_icmp;
}

static void rmnet_map_flush_tx_packet_work(struct work_struct *work)
{
	struct sk_buff *skb = NULL;
	struct rmnet_port *port;
	unsigned long flags;

	port = container_of(work, struct rmnet_port, agg_wq);

	spin_lock_irqsave(&port->agg_lock, flags);
	if (likely(port->agg_state == -EINPROGRESS)) {
		/* Buffer may have already been shipped out */
		if (likely(port->agg_skb)) {
			skb = port->agg_skb;
			port->agg_skb = NULL;
			port->agg_count = 0;
			memset(&port->agg_time, 0, sizeof(struct timespec));
		}
		port->agg_state = 0;
	}

	spin_unlock_irqrestore(&port->agg_lock, flags);
	if (skb)
		dev_queue_xmit(skb);
}

static enum hrtimer_restart rmnet_map_flush_tx_packet_queue(struct hrtimer *t)
{
	struct rmnet_port *port;

	port = container_of(t, struct rmnet_port, hrtimer);

	schedule_work(&port->agg_wq);
	return HRTIMER_NORESTART;
}

static void rmnet_map_linearize_copy(struct sk_buff *dst, struct sk_buff *src)
{
	unsigned int linear = src->len - src->data_len, target = src->len;
	unsigned char *src_buf;
	struct sk_buff *skb;

	src_buf = src->data;
	skb_put_data(dst, src_buf, linear);
	target -= linear;

	skb = src;

	while (target) {
		unsigned int i = 0, non_linear = 0;

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			non_linear = skb_frag_size(&skb_shinfo(skb)->frags[i]);
			src_buf = skb_frag_address(&skb_shinfo(skb)->frags[i]);

			skb_put_data(dst, src_buf, non_linear);
			target -= non_linear;
		}

		if (skb_shinfo(skb)->frag_list) {
			skb = skb_shinfo(skb)->frag_list;
			continue;
		}

		if (skb->next)
			skb = skb->next;
	}
}

static void rmnet_map_tx_aggregate(struct sk_buff *skb, struct rmnet_port *port)
{
	struct timespec diff, last;
	int size, agg_count = 0;
	struct sk_buff *agg_skb;
	unsigned long flags;

new_packet:
	spin_lock_irqsave(&port->agg_lock, flags);
	memcpy(&last, &port->agg_last, sizeof(struct timespec));
	getnstimeofday(&port->agg_last);

	if (!port->agg_skb) {
		/* Check to see if we should agg first. If the traffic is very
		 * sparse, don't aggregate. We will need to tune this later
		 */
		diff = timespec_sub(port->agg_last, last);
		size = port->egress_agg_params.agg_size - skb->len;

		if (diff.tv_sec > 0 || diff.tv_nsec > rmnet_agg_bypass_time ||
		    size <= 0) {
			spin_unlock_irqrestore(&port->agg_lock, flags);
			skb->protocol = htons(ETH_P_MAP);
			dev_queue_xmit(skb);
			return;
		}

		port->agg_skb = alloc_skb(port->egress_agg_params.agg_size,
					  GFP_ATOMIC);
		if (!port->agg_skb) {
			port->agg_skb = 0;
			port->agg_count = 0;
			memset(&port->agg_time, 0, sizeof(struct timespec));
			spin_unlock_irqrestore(&port->agg_lock, flags);
			skb->protocol = htons(ETH_P_MAP);
			dev_queue_xmit(skb);
			return;
		}
		rmnet_map_linearize_copy(port->agg_skb, skb);
		port->agg_skb->dev = skb->dev;
		port->agg_skb->protocol = htons(ETH_P_MAP);
		port->agg_count = 1;
		getnstimeofday(&port->agg_time);
		dev_kfree_skb_any(skb);
		goto schedule;
	}
	diff = timespec_sub(port->agg_last, port->agg_time);
	size = port->egress_agg_params.agg_size - port->agg_skb->len;

	if (skb->len > size ||
	    port->agg_count >= port->egress_agg_params.agg_count ||
	    diff.tv_sec > 0 || diff.tv_nsec > rmnet_agg_time_limit) {
		agg_skb = port->agg_skb;
		agg_count = port->agg_count;
		port->agg_skb = 0;
		port->agg_count = 0;
		memset(&port->agg_time, 0, sizeof(struct timespec));
		port->agg_state = 0;
		spin_unlock_irqrestore(&port->agg_lock, flags);
		hrtimer_cancel(&port->hrtimer);
		dev_queue_xmit(agg_skb);
		goto new_packet;
	}

	rmnet_map_linearize_copy(port->agg_skb, skb);
	port->agg_count++;
	dev_kfree_skb_any(skb);

schedule:
	if (port->agg_state != -EINPROGRESS) {
		port->agg_state = -EINPROGRESS;
		hrtimer_start(&port->hrtimer,
			      ns_to_ktime(port->egress_agg_params.agg_time),
			      HRTIMER_MODE_REL);
	}
	spin_unlock_irqrestore(&port->agg_lock, flags);
}

static void rmnet_map_tx_aggregate_init(struct rmnet_port *port)
{
	hrtimer_init(&port->hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	port->hrtimer.function = rmnet_map_flush_tx_packet_queue;
	port->egress_agg_params.agg_size = 8192;
	port->egress_agg_params.agg_count = 20;
	port->egress_agg_params.agg_time = 3000000;
	spin_lock_init(&port->agg_lock);

	INIT_WORK(&port->agg_wq, rmnet_map_flush_tx_packet_work);
}

static void rmnet_map_tx_aggregate_exit(struct rmnet_port *port)
{
	unsigned long flags;

	hrtimer_cancel(&port->hrtimer);
	cancel_work_sync(&port->agg_wq);

	spin_lock_irqsave(&port->agg_lock, flags);
	if (port->agg_state == -EINPROGRESS) {
		if (port->agg_skb) {
			kfree_skb(port->agg_skb);
			port->agg_skb = NULL;
			port->agg_count = 0;
			memset(&port->agg_time, 0, sizeof(struct timespec));
		}

		port->agg_state = 0;
	}

	spin_unlock_irqrestore(&port->agg_lock, flags);
}
