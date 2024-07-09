
/* Copyright (c) 2019, The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/hashtable.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <qca-nss-drv/nss_api_if.h>

#include <linux/rmnet_nss.h>

#define RMNET_NSS_HASH_BITS 8
#define hash_add_ptr(table, node, key) \
	hlist_add_head(node, &table[hash_ptr(key, HASH_BITS(table))])

static DEFINE_HASHTABLE(rmnet_nss_ctx_hashtable, RMNET_NSS_HASH_BITS);

struct rmnet_nss_ctx {
	struct hlist_node hnode;
	struct net_device *rmnet_dev;
	struct nss_rmnet_rx_handle *nss_ctx;
};

enum __rmnet_nss_stat {
	RMNET_NSS_RX_ETH,
	RMNET_NSS_RX_FAIL,
	RMNET_NSS_RX_NON_ETH,
	RMNET_NSS_RX_BUSY,
	RMNET_NSS_TX_NO_CTX,
	RMNET_NSS_TX_SUCCESS,
	RMNET_NSS_TX_FAIL,
	RMNET_NSS_TX_NONLINEAR,
	RMNET_NSS_TX_BAD_IP,
	RMNET_NSS_EXCEPTIONS,
	RMNET_NSS_EX_BAD_HDR,
	RMNET_NSS_EX_BAD_IP,
	RMNET_NSS_EX_SUCCESS,
	RMNET_NSS_TX_BAD_FRAGS,
	RMNET_NSS_TX_LINEARIZE_FAILS,
	RMNET_NSS_TX_NON_ZERO_HEADLEN_FRAGS,
	RMNET_NSS_TX_BUSY_LOOP,
	RMNET_NSS_NUM_STATS,
};

static unsigned long rmnet_nss_stats[RMNET_NSS_NUM_STATS];

#define RMNET_NSS_STAT(name, counter, desc) \
	module_param_named(name, rmnet_nss_stats[counter], ulong, 0444); \
	MODULE_PARM_DESC(name, desc)

RMNET_NSS_STAT(rmnet_nss_rx_ethernet, RMNET_NSS_RX_ETH,
	       "Number of Ethernet headers successfully removed");
RMNET_NSS_STAT(rmnet_nss_rx_fail, RMNET_NSS_RX_FAIL,
	       "Number of Ethernet headers that could not be removed");
RMNET_NSS_STAT(rmnet_nss_rx_non_ethernet, RMNET_NSS_RX_NON_ETH,
	       "Number of non-Ethernet packets received");
RMNET_NSS_STAT(rmnet_nss_rx_busy, RMNET_NSS_RX_BUSY,
	       "Number of packets dropped decause rmnet_data device was busy");
RMNET_NSS_STAT(rmnet_nss_tx_slow, RMNET_NSS_TX_NO_CTX,
	       "Number of packets sent over non-NSS-accelerated rmnet device");
RMNET_NSS_STAT(rmnet_nss_tx_fast, RMNET_NSS_TX_SUCCESS,
	       "Number of packets sent over NSS-accelerated rmnet device");
RMNET_NSS_STAT(rmnet_nss_tx_fail, RMNET_NSS_TX_FAIL,
	       "Number of packets that NSS could not transmit");
RMNET_NSS_STAT(rmnet_nss_tx_nonlinear, RMNET_NSS_TX_NONLINEAR,
	       "Number of non linear sent over NSS-accelerated rmnet device");
RMNET_NSS_STAT(rmnet_nss_tx_invalid_ip, RMNET_NSS_TX_BAD_IP,
	       "Number of ingress packets with invalid IP headers");
RMNET_NSS_STAT(rmnet_nss_tx_invalid_frags, RMNET_NSS_TX_BAD_FRAGS,
	       "Number of ingress packets with invalid frag format");
RMNET_NSS_STAT(rmnet_nss_tx_linearize_fail, RMNET_NSS_TX_LINEARIZE_FAILS,
	       "Number of ingress packets where linearize in tx fails");
RMNET_NSS_STAT(rmnet_nss_tx_exceptions, RMNET_NSS_EXCEPTIONS,
	       "Number of times our DL exception handler was invoked");
RMNET_NSS_STAT(rmnet_nss_exception_non_ethernet, RMNET_NSS_EX_BAD_HDR,
	       "Number of non-Ethernet exception packets");
RMNET_NSS_STAT(rmnet_nss_exception_invalid_ip, RMNET_NSS_EX_BAD_IP,
	       "Number of exception packets with invalid IP headers");
RMNET_NSS_STAT(rmnet_nss_exception_success, RMNET_NSS_EX_SUCCESS,
	       "Number of exception packets handled successfully");
RMNET_NSS_STAT(rmnet_nss_tx_non_zero_headlen_frags, RMNET_NSS_TX_NON_ZERO_HEADLEN_FRAGS,
	       "Number of packets with non zero headlen");
RMNET_NSS_STAT(rmnet_nss_tx_busy_loop, RMNET_NSS_TX_BUSY_LOOP,
	       "Number of times tx packets busy looped");

static void rmnet_nss_inc_stat(enum __rmnet_nss_stat stat)
{
	if (stat >= 0 && stat < RMNET_NSS_NUM_STATS)
		rmnet_nss_stats[stat]++;
}

static struct rmnet_nss_ctx *rmnet_nss_find_ctx(struct net_device *dev)
{
	struct rmnet_nss_ctx *ctx;
	struct hlist_head *bucket;
	u32 hash;

	hash = hash_ptr(dev, HASH_BITS(rmnet_nss_ctx_hashtable));
	bucket = &rmnet_nss_ctx_hashtable[hash];
	hlist_for_each_entry(ctx, bucket, hnode) {
		if (ctx->rmnet_dev == dev)
			return ctx;
	}

	return NULL;
}

static void rmnet_nss_free_ctx(struct rmnet_nss_ctx *ctx)
{
	if (ctx) {
		hash_del(&ctx->hnode);
		nss_rmnet_rx_xmit_callback_unregister(ctx->nss_ctx);
		nss_rmnet_rx_destroy_sync(ctx->nss_ctx);
		kfree(ctx);
	}
}

/* Pull off an ethernet header, if possible */
static int rmnet_nss_ethhdr_pull(struct sk_buff *skb)
{
	if (!skb->protocol || skb->protocol == htons(ETH_P_802_3)) {
		void *ret = skb_pull(skb, sizeof(struct ethhdr));

		rmnet_nss_inc_stat((ret) ? RMNET_NSS_RX_ETH :
					   RMNET_NSS_RX_FAIL);
		return !ret;
	}

	rmnet_nss_inc_stat(RMNET_NSS_RX_NON_ETH);
	return -1;
}

/* Copy headers to linear section for non linear packets */
static int rmnet_nss_adjust_header(struct sk_buff *skb)
{
	struct iphdr *iph;
	skb_frag_t *frag;
	int bytes = 0;
	u8 transport;

	if (skb_shinfo(skb)->nr_frags != 1) {
		rmnet_nss_inc_stat(RMNET_NSS_TX_BAD_FRAGS);
		return -EINVAL;
	}

	if (skb_headlen(skb)) {
		rmnet_nss_inc_stat(RMNET_NSS_TX_NON_ZERO_HEADLEN_FRAGS);
		return 0;
	}

	frag = &skb_shinfo(skb)->frags[0];

	iph = (struct iphdr *)(skb_frag_address(frag));

	if (iph->version == 4) {
		bytes = iph->ihl*4;
		transport = iph->protocol;
	} else if (iph->version == 6) {
		struct ipv6hdr *ip6h = (struct ipv6hdr *)iph;

		bytes = sizeof(struct ipv6hdr);
		/* Dont have to account for extension headers yet */
		transport = ip6h->nexthdr;
	} else {
		rmnet_nss_inc_stat(RMNET_NSS_TX_BAD_IP);
		return -EINVAL;
	}

	if (transport == IPPROTO_TCP) {
		struct tcphdr *th;

		th = (struct tcphdr *)((u8 *)iph + bytes);
		bytes += th->doff * 4;
	} else if (transport == IPPROTO_UDP) {
		bytes += sizeof(struct udphdr);
	} else {
		/* cant do anything else here unfortunately so linearize */
		if (skb_linearize(skb)) {
			rmnet_nss_inc_stat(RMNET_NSS_TX_LINEARIZE_FAILS);
			return -EINVAL;
		} else {
			return 0;
		}
	}

	if (bytes > skb_frag_size(frag)) {
		rmnet_nss_inc_stat(RMNET_NSS_TX_BAD_FRAGS);
		return -EINVAL;
	}

	skb_push(skb, bytes);
	memcpy(skb->data, iph, bytes);

	/* subtract to account for skb_push */
	skb->len -= bytes;

	frag->page_offset += bytes;
	skb_frag_size_sub(frag, bytes);

	/* subtract to account for skb_frag_size_sub */
	skb->data_len -= bytes;

	return 0;
}

/* Main downlink handler
 * Looks up NSS contex associated with the device. If the context is found,
 * we add a dummy ethernet header with the approriate protocol field set,
 * the pass the packet off to NSS for hardware acceleration.
 */
int rmnet_nss_tx(struct sk_buff *skb)
{
	struct ethhdr *eth;
	struct rmnet_nss_ctx *ctx;
	struct net_device *dev = skb->dev;
	nss_tx_status_t rc;
	unsigned int len;
	u8 version;

	if (skb_is_nonlinear(skb)) {
		if (rmnet_nss_adjust_header(skb))
			goto fail;
		else
			rmnet_nss_inc_stat(RMNET_NSS_TX_NONLINEAR);
	}

	version = ((struct iphdr *)skb->data)->version;

	ctx = rmnet_nss_find_ctx(dev);
	if (!ctx) {
		rmnet_nss_inc_stat(RMNET_NSS_TX_NO_CTX);
		return -EINVAL;
	}

	eth = (struct ethhdr *)skb_push(skb, sizeof(*eth));
	memset(&eth->h_dest, 0, ETH_ALEN * 2);
	if (version == 4) {
		eth->h_proto = htons(ETH_P_IP);
	} else if (version == 6) {
		eth->h_proto = htons(ETH_P_IPV6);
	} else {
		rmnet_nss_inc_stat(RMNET_NSS_TX_BAD_IP);
		goto fail;
	}

	skb->protocol = htons(ETH_P_802_3);
	/* Get length including ethhdr */
	len = skb->len;

transmit:
	rc = nss_rmnet_rx_tx_buf(ctx->nss_ctx, skb);
	if (rc == NSS_TX_SUCCESS) {
		/* Increment rmnet_data device stats.
		 * Don't call rmnet_data_vnd_rx_fixup() to do this, as
		 * there's no guarantee the skb pointer is still valid.
		 */
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += len;
		rmnet_nss_inc_stat(RMNET_NSS_TX_SUCCESS);
		return 0;
	} else if (rc == NSS_TX_FAILURE_QUEUE) {
		rmnet_nss_inc_stat(RMNET_NSS_TX_BUSY_LOOP);
		goto transmit;
	}

fail:
	rmnet_nss_inc_stat(RMNET_NSS_TX_FAIL);
	kfree_skb(skb);
	return 1;
}

/* Called by NSS in the DL exception case.
 * Since the packet cannot be sent over the accelerated path, we need to
 * handle it. Remove the ethernet header and pass it onward to the stack
 * if possible.
 */
void rmnet_nss_receive(struct net_device *dev, struct sk_buff *skb,
		       struct napi_struct *napi)
{
	rmnet_nss_inc_stat(RMNET_NSS_EXCEPTIONS);

	if (!skb)
		return;

	if (rmnet_nss_ethhdr_pull(skb)) {
		rmnet_nss_inc_stat(RMNET_NSS_EX_BAD_HDR);
		goto drop;
	}

	/* reset header pointers */
	skb_reset_transport_header(skb);
	skb_reset_network_header(skb);
	skb_reset_mac_header(skb);

	/* reset packet type */
	skb->pkt_type = PACKET_HOST;

	skb->dev = dev;

	/* reset protocol type */
	switch (skb->data[0] & 0xF0) {
	case 0x40:
		skb->protocol = htons(ETH_P_IP);
		break;
	case 0x60:
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		rmnet_nss_inc_stat(RMNET_NSS_EX_BAD_IP);
		goto drop;
	}

	rmnet_nss_inc_stat(RMNET_NSS_EX_SUCCESS);

	/* Set this so that we dont loop around netif_receive_skb */

	skb->cb[0] = 1;

	netif_receive_skb(skb);
	return;

drop:
	kfree_skb(skb);
}

/* Called by NSS in the UL acceleration case.
 * We are guaranteed to have an ethernet packet here from the NSS hardware,
 * We need to pull the header off and invoke our ndo_start_xmit function
 * to handle transmitting the packet to the network stack.
 */
void rmnet_nss_xmit(struct net_device *dev, struct sk_buff *skb)
{
	netdev_tx_t ret;

	skb_pull(skb, sizeof(struct ethhdr));
	rmnet_nss_inc_stat(RMNET_NSS_RX_ETH);

	/* NSS takes care of shaping, so bypassing Qdiscs like this is OK */
	ret = dev->netdev_ops->ndo_start_xmit(skb, dev);
	if (unlikely(ret == NETDEV_TX_BUSY)) {
		dev_kfree_skb_any(skb);
		rmnet_nss_inc_stat(RMNET_NSS_RX_BUSY);
	}
}

/* Create and register an NSS context for an rmnet_data device */
int rmnet_nss_create_vnd(struct net_device *dev)
{
	struct rmnet_nss_ctx *ctx;

	ctx = kzalloc(sizeof(*ctx), GFP_ATOMIC);
	if (!ctx)
		return -ENOMEM;

	ctx->rmnet_dev = dev;
	ctx->nss_ctx = nss_rmnet_rx_create_sync_nexthop(dev, NSS_N2H_INTERFACE,
						       NSS_C2C_TX_INTERFACE);
	if (!ctx->nss_ctx) {
		kfree(ctx);
		return -1;
	}

	nss_rmnet_rx_register(ctx->nss_ctx, rmnet_nss_receive, dev);
	nss_rmnet_rx_xmit_callback_register(ctx->nss_ctx, rmnet_nss_xmit);
	hash_add_ptr(rmnet_nss_ctx_hashtable, &ctx->hnode, dev);
	return 0;
}

/* Unregister and destroy the NSS context for an rmnet_data device */
int rmnet_nss_free_vnd(struct net_device *dev)
{
	struct rmnet_nss_ctx *ctx;

	ctx = rmnet_nss_find_ctx(dev);
	rmnet_nss_free_ctx(ctx);

	return 0;
}

static const struct rmnet_nss_cb rmnet_nss = {
	.nss_create = rmnet_nss_create_vnd,
	.nss_free = rmnet_nss_free_vnd,
	.nss_tx = rmnet_nss_tx,
};

int __init rmnet_nss_init(void)
{
	pr_err("%s(): initializing rmnet_nss\n", __func__);
	RCU_INIT_POINTER(rmnet_nss_callbacks, &rmnet_nss);
	return 0;
}

void __exit rmnet_nss_exit(void)
{
	struct hlist_node *tmp;
	struct rmnet_nss_ctx *ctx;
	int bkt;

	pr_err("%s(): exiting rmnet_nss\n", __func__);
	RCU_INIT_POINTER(rmnet_nss_callbacks, NULL);

	/* Tear down all NSS contexts */
	hash_for_each_safe(rmnet_nss_ctx_hashtable, bkt, tmp, ctx, hnode)
		rmnet_nss_free_ctx(ctx);
}

#if 0
MODULE_LICENSE("GPL v2");
module_init(rmnet_nss_init);
module_exit(rmnet_nss_exit);
#endif
