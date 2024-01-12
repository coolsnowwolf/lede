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
 * RMNET Packet Descriptor Framework
 *
 */

#include <linux/ip.h>
#include <linux/ipv6.h>
#include <net/ipv6.h>
#include <net/ip6_checksum.h>
#include "rmnet_config.h"
#include "rmnet_descriptor.h"
#include "rmnet_handlers.h"
#include "rmnet_private.h"
#include "rmnet_vnd.h"

#define RMNET_FRAG_DESCRIPTOR_POOL_SIZE 64
#define RMNET_DL_IND_HDR_SIZE (sizeof(struct rmnet_map_dl_ind_hdr) + \
			       sizeof(struct rmnet_map_header) + \
			       sizeof(struct rmnet_map_control_command_header))
#define RMNET_DL_IND_TRL_SIZE (sizeof(struct rmnet_map_dl_ind_trl) + \
			       sizeof(struct rmnet_map_header) + \
			       sizeof(struct rmnet_map_control_command_header))

typedef void (*rmnet_perf_desc_hook_t)(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port);
typedef void (*rmnet_perf_chain_hook_t)(void);

static struct rmnet_frag_descriptor *
rmnet_get_frag_descriptor(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool = port->frag_desc_pool;
	struct rmnet_frag_descriptor *frag_desc;

	spin_lock(&port->desc_pool_lock);
	if (!list_empty(&pool->free_list)) {
		frag_desc = list_first_entry(&pool->free_list,
					     struct rmnet_frag_descriptor,
					     list);
		list_del_init(&frag_desc->list);
	} else {
		frag_desc = kzalloc(sizeof(*frag_desc), GFP_ATOMIC);
		if (!frag_desc)
			goto out;

		INIT_LIST_HEAD(&frag_desc->list);
		INIT_LIST_HEAD(&frag_desc->sub_frags);
		pool->pool_size++;
	}

out:
	spin_unlock(&port->desc_pool_lock);
	return frag_desc;
}

static void rmnet_recycle_frag_descriptor(struct rmnet_frag_descriptor *frag_desc,
				   struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool = port->frag_desc_pool;
	struct page *page = skb_frag_page(&frag_desc->frag);

	list_del(&frag_desc->list);
	if (page)
		put_page(page);

	memset(frag_desc, 0, sizeof(*frag_desc));
	INIT_LIST_HEAD(&frag_desc->list);
	INIT_LIST_HEAD(&frag_desc->sub_frags);
	spin_lock(&port->desc_pool_lock);
	list_add_tail(&frag_desc->list, &pool->free_list);
	spin_unlock(&port->desc_pool_lock);
}

static void rmnet_descriptor_add_frag(struct rmnet_port *port, struct list_head *list,
			       struct page *p, u32 page_offset, u32 len)
{
	struct rmnet_frag_descriptor *frag_desc;

	frag_desc = rmnet_get_frag_descriptor(port);
	if (!frag_desc)
		return;

	rmnet_frag_fill(frag_desc, p, page_offset, len);
	list_add_tail(&frag_desc->list, list);
}

static u8 rmnet_frag_do_flow_control(struct rmnet_map_header *qmap,
				     struct rmnet_port *port,
				     int enable)
{
	struct rmnet_map_control_command *cmd;
	struct rmnet_endpoint *ep;
	struct net_device *vnd;
	u16 ip_family;
	u16 fc_seq;
	u32 qos_id;
	u8 mux_id;
	int r;

	mux_id = qmap->mux_id;
	cmd = (struct rmnet_map_control_command *)
	      ((char *)qmap + sizeof(*qmap));

	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		return RX_HANDLER_CONSUMED;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		return RX_HANDLER_CONSUMED;

	vnd = ep->egress_dev;

	ip_family = cmd->flow_control.ip_family;
	fc_seq = ntohs(cmd->flow_control.flow_control_seq_num);
	qos_id = ntohl(cmd->flow_control.qos_id);

	/* Ignore the ip family and pass the sequence number for both v4 and v6
	 * sequence. User space does not support creating dedicated flows for
	 * the 2 protocols
	 */
	r = rmnet_vnd_do_flow_control(vnd, enable);
	if (r)
		return RMNET_MAP_COMMAND_UNSUPPORTED;
	else
		return RMNET_MAP_COMMAND_ACK;
}

static void rmnet_frag_send_ack(struct rmnet_map_header *qmap,
				unsigned char type,
				struct rmnet_port *port)
{
	struct rmnet_map_control_command *cmd;
	struct net_device *dev = port->dev;
	struct sk_buff *skb;
	u16 alloc_len = ntohs(qmap->pkt_len) + sizeof(*qmap);

	skb = alloc_skb(alloc_len, GFP_ATOMIC);
	if (!skb)
		return;

	skb->protocol = htons(ETH_P_MAP);
	skb->dev = dev;

	cmd = rmnet_map_get_cmd_start(skb);
	cmd->cmd_type = type & 0x03;

	netif_tx_lock(dev);
	dev->netdev_ops->ndo_start_xmit(skb, dev);
	netif_tx_unlock(dev);
}


/* Process MAP command frame and send N/ACK message as appropriate. Message cmd
 * name is decoded here and appropriate handler is called.
 */
static void rmnet_frag_command(struct rmnet_map_header *qmap, struct rmnet_port *port)
{
	struct rmnet_map_control_command *cmd;
	unsigned char command_name;
	unsigned char rc = 0;

	cmd = (struct rmnet_map_control_command *)
	      ((char *)qmap + sizeof(*qmap));
	command_name = cmd->command_name;

	switch (command_name) {
	case RMNET_MAP_COMMAND_FLOW_ENABLE:
		rc = rmnet_frag_do_flow_control(qmap, port, 1);
		break;

	case RMNET_MAP_COMMAND_FLOW_DISABLE:
		rc = rmnet_frag_do_flow_control(qmap, port, 0);
		break;

	default:
		rc = RMNET_MAP_COMMAND_UNSUPPORTED;
		break;
	}
	if (rc == RMNET_MAP_COMMAND_ACK)
		rmnet_frag_send_ack(qmap, rc, port);
}

static void rmnet_frag_deaggregate(skb_frag_t *frag, struct rmnet_port *port,
			    struct list_head *list)
{
	struct rmnet_map_header *maph;
	u8 *data = skb_frag_address(frag);
	u32 offset = 0;
	u32 packet_len;

	while (offset < skb_frag_size(frag)) {
		maph = (struct rmnet_map_header *)data;
		packet_len = ntohs(maph->pkt_len);

		/* Some hardware can send us empty frames. Catch them */
		if (packet_len == 0)
			return;

		packet_len += sizeof(*maph);

		if (port->data_format & RMNET_FLAGS_INGRESS_MAP_CKSUMV4) {
			packet_len += sizeof(struct rmnet_map_dl_csum_trailer);
			WARN_ON(1);
		} else if (port->data_format &
			   (RMNET_FLAGS_INGRESS_MAP_CKSUMV5 |
			    RMNET_FLAGS_INGRESS_COALESCE) && !maph->cd_bit) {
			u32 hsize = 0;
			u8 type;

			type = ((struct rmnet_map_v5_coal_header *)
				(data + sizeof(*maph)))->header_type;
			switch (type) {
			case RMNET_MAP_HEADER_TYPE_COALESCING:
				hsize = sizeof(struct rmnet_map_v5_coal_header);
				break;
			case RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD:
				hsize = sizeof(struct rmnet_map_v5_csum_header);
				break;
			}

			packet_len += hsize;
		}
		else {
			//qmap_hex_dump(__func__, data, 64);
			WARN_ON(1);
		}

		if ((int)skb_frag_size(frag) - (int)packet_len < 0)
			return;

		rmnet_descriptor_add_frag(port, list, skb_frag_page(frag),
					  frag->page_offset + offset,
					  packet_len);

		offset += packet_len;
		data += packet_len;
	}
}

/* Allocate and populate an skb to contain the packet represented by the
 * frag descriptor.
 */
static struct sk_buff *rmnet_alloc_skb(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port)
{
	struct sk_buff *head_skb, *current_skb, *skb;
	struct skb_shared_info *shinfo;
	struct rmnet_frag_descriptor *sub_frag, *tmp;

	/* Use the exact sizes if we know them (i.e. RSB/RSC, rmnet_perf) */
	if (frag_desc->hdrs_valid) {
		u16 hdr_len = frag_desc->ip_len + frag_desc->trans_len;

		head_skb = alloc_skb(hdr_len + RMNET_MAP_DESC_HEADROOM,
				     GFP_ATOMIC);
		if (!head_skb)
			return NULL;

		skb_reserve(head_skb, RMNET_MAP_DESC_HEADROOM);
		skb_put_data(head_skb, frag_desc->hdr_ptr, hdr_len);
		skb_reset_network_header(head_skb);

		if (frag_desc->trans_len)
			skb_set_transport_header(head_skb, frag_desc->ip_len);

		/* Packets that have no data portion don't need any frags */
		if (hdr_len == skb_frag_size(&frag_desc->frag))
			goto skip_frags;

		/* If the headers we added are the start of the page,
		 * we don't want to add them twice
		 */
		if (frag_desc->hdr_ptr == rmnet_frag_data_ptr(frag_desc)) {
			if (!rmnet_frag_pull(frag_desc, port, hdr_len)) {
				kfree_skb(head_skb);
				return NULL;
			}
		}
	} else {
		/* Allocate enough space to avoid penalties in the stack
		 * from __pskb_pull_tail()
		 */
		head_skb = alloc_skb(256 + RMNET_MAP_DESC_HEADROOM,
				     GFP_ATOMIC);
		if (!head_skb)
			return NULL;

		skb_reserve(head_skb, RMNET_MAP_DESC_HEADROOM);
	}

	/* Add main fragment */
	get_page(skb_frag_page(&frag_desc->frag));
	skb_add_rx_frag(head_skb, 0, skb_frag_page(&frag_desc->frag),
			frag_desc->frag.page_offset,
			skb_frag_size(&frag_desc->frag),
			skb_frag_size(&frag_desc->frag));

	shinfo = skb_shinfo(head_skb);
	current_skb = head_skb;

	/* Add in any frags from rmnet_perf */
	list_for_each_entry_safe(sub_frag, tmp, &frag_desc->sub_frags, list) {
		skb_frag_t *frag;
		u32 frag_size;

		frag = &sub_frag->frag;
		frag_size = skb_frag_size(frag);

add_frag:
		if (shinfo->nr_frags < MAX_SKB_FRAGS) {
			get_page(skb_frag_page(frag));
			skb_add_rx_frag(current_skb, shinfo->nr_frags,
					skb_frag_page(frag), frag->page_offset,
					frag_size, frag_size);
			if (current_skb != head_skb) {
				head_skb->len += frag_size;
				head_skb->data_len += frag_size;
			}
		} else {
			/* Alloc a new skb and try again */
			skb = alloc_skb(0, GFP_ATOMIC);
			if (!skb)
				break;

			if (current_skb == head_skb)
				shinfo->frag_list = skb;
			else
				current_skb->next = skb;

			current_skb = skb;
			shinfo = skb_shinfo(current_skb);
			goto add_frag;
		}

		rmnet_recycle_frag_descriptor(sub_frag, port);
	}

skip_frags:
	head_skb->dev = frag_desc->dev;
	rmnet_set_skb_proto(head_skb);

	/* Handle any header metadata that needs to be updated after RSB/RSC
	 * segmentation
	 */
	if (frag_desc->ip_id_set) {
		struct iphdr *iph;

		iph = (struct iphdr *)rmnet_map_data_ptr(head_skb);
		csum_replace2(&iph->check, iph->id, frag_desc->ip_id);
		iph->id = frag_desc->ip_id;
	}

	if (frag_desc->tcp_seq_set) {
		struct tcphdr *th;

		th = (struct tcphdr *)
		     (rmnet_map_data_ptr(head_skb) + frag_desc->ip_len);
		th->seq = frag_desc->tcp_seq;
	}

	/* Handle csum offloading */
	if (frag_desc->csum_valid && frag_desc->hdrs_valid) {
		/* Set the partial checksum information */
		//rmnet_frag_partial_csum(head_skb, frag_desc);
		WARN_ON(1);
	} else if (frag_desc->csum_valid) {
		/* Non-RSB/RSC/perf packet. The current checksum is fine */
		head_skb->ip_summed = CHECKSUM_UNNECESSARY;
	} else if (frag_desc->hdrs_valid &&
		   (frag_desc->trans_proto == IPPROTO_TCP ||
		    frag_desc->trans_proto == IPPROTO_UDP)) {
		/* Unfortunately, we have to fake a bad checksum here, since
		 * the original bad value is lost by the hardware. The only
		 * reliable way to do it is to calculate the actual checksum
		 * and corrupt it.
		 */
		__sum16 *check;
		__wsum csum;
		unsigned int offset = skb_transport_offset(head_skb);
		__sum16 pseudo;

		WARN_ON(1);
		/* Calculate pseudo header and update header fields */
		if (frag_desc->ip_proto == 4) {
			struct iphdr *iph = ip_hdr(head_skb);
			__be16 tot_len = htons(head_skb->len);

			csum_replace2(&iph->check, iph->tot_len, tot_len);
			iph->tot_len = tot_len;
			pseudo = ~csum_tcpudp_magic(iph->saddr, iph->daddr,
						    head_skb->len -
						    frag_desc->ip_len,
						    frag_desc->trans_proto, 0);
		} else {
			struct ipv6hdr *ip6h = ipv6_hdr(head_skb);

			ip6h->payload_len = htons(head_skb->len -
						  sizeof(*ip6h));
			pseudo = ~csum_ipv6_magic(&ip6h->saddr, &ip6h->daddr,
						  head_skb->len -
						  frag_desc->ip_len,
						  frag_desc->trans_proto, 0);
		}

		if (frag_desc->trans_proto == IPPROTO_TCP) {
			check = &tcp_hdr(head_skb)->check;
		} else {
			udp_hdr(head_skb)->len = htons(head_skb->len -
						       frag_desc->ip_len);
			check = &udp_hdr(head_skb)->check;
		}

		*check = pseudo;
		csum = skb_checksum(head_skb, offset, head_skb->len - offset,
				    0);
		/* Add 1 to corrupt. This cannot produce a final value of 0
		 * since csum_fold() can't return a value of 0xFFFF
		 */
		*check = csum16_add(csum_fold(csum), htons(1));
		head_skb->ip_summed = CHECKSUM_NONE;
	}

	/* Handle any rmnet_perf metadata */
	if (frag_desc->hash) {
		head_skb->hash = frag_desc->hash;
		head_skb->sw_hash = 1;
	}

	if (frag_desc->flush_shs)
		head_skb->cb[0] = 1;

	/* Handle coalesced packets */
	//if (frag_desc->gso_segs > 1)
	//	rmnet_frag_gso_stamp(head_skb, frag_desc);

	return head_skb;
}

/* Deliver the packets contained within a frag descriptor */
static void rmnet_frag_deliver(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_port *port)
{
	struct sk_buff *skb;

	skb = rmnet_alloc_skb(frag_desc, port);
	if (skb)
		rmnet_deliver_skb(skb, port);
	rmnet_recycle_frag_descriptor(frag_desc, port);
}

/* Process a QMAPv5 packet header */
static int rmnet_frag_process_next_hdr_packet(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port,
				       struct list_head *list,
				       u16 len)
{
	int rc = 0;

	switch (rmnet_frag_get_next_hdr_type(frag_desc)) {
	case RMNET_MAP_HEADER_TYPE_COALESCING:
		rc = -1;
		WARN_ON(1);
		break;
	case RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD:
		if (rmnet_frag_get_csum_valid(frag_desc)) {
			frag_desc->csum_valid = true;
		} else {
		}

		if (!rmnet_frag_pull(frag_desc, port,
				     sizeof(struct rmnet_map_header) +
				     sizeof(struct rmnet_map_v5_csum_header))) {
			rc = -EINVAL;
			break;
		}

		frag_desc->hdr_ptr = rmnet_frag_data_ptr(frag_desc);

		/* Remove padding only for csum offload packets.
		 * Coalesced packets should never have padding.
		 */
		if (!rmnet_frag_trim(frag_desc, port, len)) {
			rc = -EINVAL;
			break;
		}

		list_del_init(&frag_desc->list);
		list_add_tail(&frag_desc->list, list);
		break;
	default:
		//qmap_hex_dump(__func__, rmnet_frag_data_ptr(frag_desc), 64);
		rc = -EINVAL;
		break;
	}

	return rc;
}

static void
__rmnet_frag_ingress_handler(struct rmnet_frag_descriptor *frag_desc,
			     struct rmnet_port *port)
{
	struct rmnet_map_header *qmap;
	struct rmnet_endpoint *ep;
	struct rmnet_frag_descriptor *frag, *tmp;
	LIST_HEAD(segs);
	u16 len, pad;
	u8 mux_id;

	qmap = (struct rmnet_map_header *)skb_frag_address(&frag_desc->frag);
	mux_id = qmap->mux_id;
	pad = qmap->pad_len;
	len = ntohs(qmap->pkt_len) - pad;

	if (qmap->cd_bit) {
		if (port->data_format & RMNET_INGRESS_FORMAT_DL_MARKER) {
			//rmnet_frag_flow_command(qmap, port, len);
			goto recycle;
		}

		if (port->data_format & RMNET_FLAGS_INGRESS_MAP_COMMANDS)
			rmnet_frag_command(qmap, port);

		goto recycle;
	}

	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		goto recycle;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		goto recycle;

	frag_desc->dev = ep->egress_dev;

	/* Handle QMAPv5 packet */
	if (qmap->next_hdr &&
	    (port->data_format & (RMNET_FLAGS_INGRESS_COALESCE |
				  RMNET_FLAGS_INGRESS_MAP_CKSUMV5))) {
		if (rmnet_frag_process_next_hdr_packet(frag_desc, port, &segs,
						       len))
			goto recycle;
	} else {
		/* We only have the main QMAP header to worry about */
		if (!rmnet_frag_pull(frag_desc, port, sizeof(*qmap)))
			return;

		frag_desc->hdr_ptr = rmnet_frag_data_ptr(frag_desc);

		if (!rmnet_frag_trim(frag_desc, port, len))
			return;

		list_add_tail(&frag_desc->list, &segs);
	}

	list_for_each_entry_safe(frag, tmp, &segs, list) {
		list_del_init(&frag->list);
		rmnet_frag_deliver(frag, port);
	}
	return;

recycle:
	rmnet_recycle_frag_descriptor(frag_desc, port);
}

static void rmnet_frag_ingress_handler(struct sk_buff *skb,
				struct rmnet_port *port)
{
	LIST_HEAD(desc_list);
	int i = 0;
	struct rmnet_nss_cb *nss_cb;

	/* Deaggregation and freeing of HW originating
	 * buffers is done within here
	 */
	while (skb) {
		struct sk_buff *skb_frag;

		port->chain_head = NULL;
		port->chain_tail = NULL;

		for (i = 0; i < skb_shinfo(skb)->nr_frags; i++) {
			rmnet_frag_deaggregate(&skb_shinfo(skb)->frags[i], port,
					       &desc_list);
			if (!list_empty(&desc_list)) {
				struct rmnet_frag_descriptor *frag_desc, *tmp;

				list_for_each_entry_safe(frag_desc, tmp,
							 &desc_list, list) {
					list_del_init(&frag_desc->list);
					__rmnet_frag_ingress_handler(frag_desc,
								     port);
				}
			}
		}

		nss_cb = rcu_dereference(rmnet_nss_callbacks);
		if (nss_cb && port->chain_head) {
			port->chain_head->cb[0] = 0;
			netif_receive_skb(port->chain_head);
		}

		skb_frag = skb_shinfo(skb)->frag_list;
		skb_shinfo(skb)->frag_list = NULL;
		consume_skb(skb);
		skb = skb_frag;
	}
}

void rmnet_descriptor_deinit(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool;
	struct rmnet_frag_descriptor *frag_desc, *tmp;

	pool = port->frag_desc_pool;

	list_for_each_entry_safe(frag_desc, tmp, &pool->free_list, list) {
		kfree(frag_desc);
		pool->pool_size--;
	}

	kfree(pool);
}

int rmnet_descriptor_init(struct rmnet_port *port)
{
	struct rmnet_frag_descriptor_pool *pool;
	int i;

	spin_lock_init(&port->desc_pool_lock);
	pool = kzalloc(sizeof(*pool), GFP_ATOMIC);
	if (!pool)
		return -ENOMEM;

	INIT_LIST_HEAD(&pool->free_list);
	port->frag_desc_pool = pool;

	for (i = 0; i < RMNET_FRAG_DESCRIPTOR_POOL_SIZE; i++) {
		struct rmnet_frag_descriptor *frag_desc;

		frag_desc = kzalloc(sizeof(*frag_desc), GFP_ATOMIC);
		if (!frag_desc)
			return -ENOMEM;

		INIT_LIST_HEAD(&frag_desc->list);
		INIT_LIST_HEAD(&frag_desc->sub_frags);
		list_add_tail(&frag_desc->list, &pool->free_list);
		pool->pool_size++;
	}

	return 0;
}
