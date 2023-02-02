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
 * RMNET Data ingress/egress handler
 *
 */

#include <linux/netdevice.h>
#include <linux/netdev_features.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/sock.h>
#include <linux/tracepoint.h>
#include "rmnet_private.h"
#include "rmnet_config.h"
#include "rmnet_vnd.h"
#include "rmnet_map.h"
#include "rmnet_handlers.h"
#include "rmnet_descriptor.h"

#define RMNET_IP_VERSION_4 0x40
#define RMNET_IP_VERSION_6 0x60

/* Helper Functions */

static void rmnet_set_skb_proto(struct sk_buff *skb)
{
	switch (rmnet_map_data_ptr(skb)[0] & 0xF0) {
	case RMNET_IP_VERSION_4:
		skb->protocol = htons(ETH_P_IP);
		break;
	case RMNET_IP_VERSION_6:
		skb->protocol = htons(ETH_P_IPV6);
		break;
	default:
		skb->protocol = htons(ETH_P_MAP);
		break;
	}
}

/* Generic handler */

static void
rmnet_deliver_skb(struct sk_buff *skb, struct rmnet_port *port)
{
	struct rmnet_nss_cb *nss_cb;

	rmnet_vnd_rx_fixup(skb->dev, skb->len);

	/* Pass off the packet to NSS driver if we can */
	nss_cb = rcu_dereference(rmnet_nss_callbacks);
	if (nss_cb) {
		if (!port->chain_head)
			port->chain_head = skb;
		else
			skb_shinfo(port->chain_tail)->frag_list = skb;

		port->chain_tail = skb;
		return;
	}

	skb_reset_transport_header(skb);
	skb_reset_network_header(skb);

	skb->pkt_type = PACKET_HOST;
	skb_set_mac_header(skb, 0);

	//if (port->data_format & RMNET_INGRESS_FORMAT_DL_MARKER) {
	//} else {
		//if (!rmnet_check_skb_can_gro(skb))
		//	gro_cells_receive(&priv->gro_cells, skb);
		//else
			netif_receive_skb(skb);
	//}
}

/* Deliver a list of skbs after undoing coalescing */
static void rmnet_deliver_skb_list(struct sk_buff_head *head,
				   struct rmnet_port *port)
{
	struct sk_buff *skb;

	while ((skb = __skb_dequeue(head))) {
		rmnet_set_skb_proto(skb);
		rmnet_deliver_skb(skb, port);
	}
}

/* MAP handler */

static void
_rmnet_map_ingress_handler(struct sk_buff *skb,
			    struct rmnet_port *port)
{
	struct rmnet_map_header *qmap;
	struct rmnet_endpoint *ep;
	struct sk_buff_head list;
	u16 len, pad;
	u8 mux_id;

	/* We don't need the spinlock since only we touch this */
	__skb_queue_head_init(&list);

	qmap = (struct rmnet_map_header *)rmnet_map_data_ptr(skb);
	if (qmap->cd_bit) {
		if (port->data_format & RMNET_INGRESS_FORMAT_DL_MARKER) {
			//if (!rmnet_map_flow_command(skb, port, false))
				return;
		}

		if (port->data_format & RMNET_FLAGS_INGRESS_MAP_COMMANDS)
			return rmnet_map_command(skb, port);

		goto free_skb;
	}

	mux_id = qmap->mux_id;
	pad = qmap->pad_len;
	len = ntohs(qmap->pkt_len) - pad;

	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		goto free_skb;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		goto free_skb;

	skb->dev = ep->egress_dev;

	/* Handle QMAPv5 packet */
	if (qmap->next_hdr &&
	    (port->data_format & (RMNET_FLAGS_INGRESS_COALESCE |
				  RMNET_FLAGS_INGRESS_MAP_CKSUMV5))) {
		if (rmnet_map_process_next_hdr_packet(skb, &list, len))
			goto free_skb;
	} else {
		/* We only have the main QMAP header to worry about */
		pskb_pull(skb, sizeof(*qmap));

		rmnet_set_skb_proto(skb);

		if (port->data_format & RMNET_FLAGS_INGRESS_MAP_CKSUMV4) {
			//if (!rmnet_map_checksum_downlink_packet(skb, len + pad))
			//	skb->ip_summed = CHECKSUM_UNNECESSARY;
		}

		pskb_trim(skb, len);

		/* Push the single packet onto the list */
		__skb_queue_tail(&list, skb);
	}

	rmnet_deliver_skb_list(&list, port);
	return;

free_skb:
	kfree_skb(skb);
}

static void
rmnet_map_ingress_handler(struct sk_buff *skb,
			  struct rmnet_port *port)
{
	struct sk_buff *skbn;

	if (port->data_format & (RMNET_FLAGS_INGRESS_COALESCE |
				 RMNET_FLAGS_INGRESS_MAP_CKSUMV5)) {
		if (skb_is_nonlinear(skb)) {
			rmnet_frag_ingress_handler(skb, port);
			return;
		}
	}

	/* Deaggregation and freeing of HW originating
	 * buffers is done within here
	 */
	while (skb) {
		struct sk_buff *skb_frag = skb_shinfo(skb)->frag_list;

		skb_shinfo(skb)->frag_list = NULL;
		while ((skbn = rmnet_map_deaggregate(skb, port)) != NULL) {
			_rmnet_map_ingress_handler(skbn, port);

			if (skbn == skb)
				goto next_skb;
		}

		consume_skb(skb);
next_skb:
		skb = skb_frag;
	}
}

static int rmnet_map_egress_handler(struct sk_buff *skb,
				    struct rmnet_port *port, u8 mux_id,
				    struct net_device *orig_dev)
{
	int required_headroom, additional_header_len, csum_type;
	struct rmnet_map_header *map_header;

	additional_header_len = 0;
	required_headroom = sizeof(struct rmnet_map_header);
	csum_type = 0;

	if (port->data_format & RMNET_FLAGS_EGRESS_MAP_CKSUMV4) {
		additional_header_len = sizeof(struct rmnet_map_ul_csum_header);
		csum_type = RMNET_FLAGS_EGRESS_MAP_CKSUMV4;
	} else if (port->data_format & RMNET_FLAGS_EGRESS_MAP_CKSUMV5) {
		additional_header_len = sizeof(struct rmnet_map_v5_csum_header);
		csum_type = RMNET_FLAGS_EGRESS_MAP_CKSUMV5;
	}

	required_headroom += additional_header_len;

	if (skb_headroom(skb) < required_headroom) {
		if (pskb_expand_head(skb, required_headroom, 0, GFP_ATOMIC))
			return -ENOMEM;
	}

	if (csum_type)
		rmnet_map_checksum_uplink_packet(skb, orig_dev, csum_type);

	map_header = rmnet_map_add_map_header(skb, additional_header_len, 0,
					      port);
	if (!map_header)
		return -ENOMEM;

	map_header->mux_id = mux_id;

	if (port->data_format & RMNET_EGRESS_FORMAT_AGGREGATION) {
		if (rmnet_map_tx_agg_skip(skb, required_headroom))
			goto done;

		rmnet_map_tx_aggregate(skb, port);
		return -EINPROGRESS;
	}

done:
	skb->protocol = htons(ETH_P_MAP);
	return 0;
}

static void
rmnet_bridge_handler(struct sk_buff *skb, struct net_device *bridge_dev)
{
	if (bridge_dev) {
		skb->dev = bridge_dev;
		dev_queue_xmit(skb);
	}
}

/* Ingress / Egress Entry Points */

/* Processes packet as per ingress data format for receiving device. Logical
 * endpoint is determined from packet inspection. Packet is then sent to the
 * egress device listed in the logical endpoint configuration.
 */
static rx_handler_result_t rmnet_rx_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct rmnet_port *port;
	struct net_device *dev;

	if (!skb)
		goto done;

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	dev = skb->dev;
	port = rmnet_get_port(dev);

	port->chain_head = NULL;
	port->chain_tail = NULL;

	switch (port->rmnet_mode) {
	case RMNET_EPMODE_VND:
		rmnet_map_ingress_handler(skb, port);
		break;
	case RMNET_EPMODE_BRIDGE:
		rmnet_bridge_handler(skb, port->bridge_ep);
		break;
	}

done:
	return RX_HANDLER_CONSUMED;
}

static rx_handler_result_t rmnet_rx_priv_handler(struct sk_buff **pskb)
{
	struct sk_buff *skb = *pskb;
	struct rmnet_nss_cb *nss_cb;

	if (!skb)
		return RX_HANDLER_CONSUMED;
	if (nss_debug) printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	/* Check this so that we dont loop around netif_receive_skb */
	if (skb->cb[0] == 1) {
		skb->cb[0] = 0;

		skb->dev->stats.rx_packets++;
		return RX_HANDLER_PASS;
	}

	while (skb) {
		struct sk_buff *skb_frag = skb_shinfo(skb)->frag_list;

		skb_shinfo(skb)->frag_list = NULL;

		nss_cb = rcu_dereference(rmnet_nss_callbacks);
		if (nss_cb)
			nss_cb->nss_tx(skb);

		skb = skb_frag;
	}

	return RX_HANDLER_CONSUMED;
}

/* Modifies packet as per logical endpoint configuration and egress data format
 * for egress device configured in logical endpoint. Packet is then transmitted
 * on the egress device.
 */
static void rmnet_egress_handler(struct sk_buff *skb)
{
	struct net_device *orig_dev;
	struct rmnet_port *port;
	struct rmnet_priv *priv;
	u8 mux_id;
	int err;
	u32 skb_len;

	skb_orphan(skb);

	orig_dev = skb->dev;
	priv = netdev_priv(orig_dev);
	skb->dev = priv->real_dev;
	mux_id = priv->mux_id;

	port = rmnet_get_port(skb->dev);
	if (!port)
		goto drop;

	skb_len = skb->len;
	err = rmnet_map_egress_handler(skb, port, mux_id, orig_dev);
	if (err == -ENOMEM)
		goto drop;
	else if (err == -EINPROGRESS) {
		rmnet_vnd_tx_fixup(orig_dev, skb_len);
		return;
	}

	rmnet_vnd_tx_fixup(orig_dev, skb_len);

	dev_queue_xmit(skb);
	return;

drop:
	this_cpu_inc(priv->pcpu_stats->stats.tx_drops);
	kfree_skb(skb);
}
