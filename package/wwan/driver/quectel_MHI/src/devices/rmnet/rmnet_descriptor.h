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

#ifndef _RMNET_DESCRIPTOR_H_
#define _RMNET_DESCRIPTOR_H_

#include <linux/netdevice.h>
#include <linux/list.h>
#include <linux/skbuff.h>
#include "rmnet_config.h"
#include "rmnet_map.h"

struct rmnet_frag_descriptor_pool {
	struct list_head free_list;
	u32 pool_size;
};

struct rmnet_frag_descriptor {
	struct list_head list;
	struct list_head sub_frags;
	skb_frag_t frag;
	u8 *hdr_ptr;
	struct net_device *dev;
	u32 hash;
	__be32 tcp_seq;
	__be16 ip_id;
	u16 data_offset;
	u16 gso_size;
	u16 gso_segs;
	u16 ip_len;
	u16 trans_len;
	u8 ip_proto;
	u8 trans_proto;
	u8 pkt_id;
	u8 csum_valid:1,
	   hdrs_valid:1,
	   ip_id_set:1,
	   tcp_seq_set:1,
	   flush_shs:1,
	   reserved:3;
};

/* Descriptor management */
static struct rmnet_frag_descriptor *
rmnet_get_frag_descriptor(struct rmnet_port *port);
static void rmnet_recycle_frag_descriptor(struct rmnet_frag_descriptor *frag_desc,
				   struct rmnet_port *port);
static void rmnet_descriptor_add_frag(struct rmnet_port *port, struct list_head *list,
			       struct page *p, u32 page_offset, u32 len);

/* QMAP command packets */

/* Ingress data handlers */
static void rmnet_frag_deaggregate(skb_frag_t *frag, struct rmnet_port *port,
			    struct list_head *list);
static void rmnet_frag_deliver(struct rmnet_frag_descriptor *frag_desc,
			struct rmnet_port *port);
static int rmnet_frag_process_next_hdr_packet(struct rmnet_frag_descriptor *frag_desc,
				       struct rmnet_port *port,
				       struct list_head *list,
				       u16 len);
static void rmnet_frag_ingress_handler(struct sk_buff *skb,
				struct rmnet_port *port);

static int rmnet_descriptor_init(struct rmnet_port *port);
static void rmnet_descriptor_deinit(struct rmnet_port *port);

static inline void *rmnet_frag_data_ptr(struct rmnet_frag_descriptor *frag_desc)
{
	return skb_frag_address(&frag_desc->frag);
}

static inline void *rmnet_frag_pull(struct rmnet_frag_descriptor *frag_desc,
				    struct rmnet_port *port,
				    unsigned int size)
{
	if (size >= skb_frag_size(&frag_desc->frag)) {
		pr_info("%s(): Pulling %u bytes from %u byte pkt. Dropping\n",
			__func__, size, skb_frag_size(&frag_desc->frag));
		rmnet_recycle_frag_descriptor(frag_desc, port);
		return NULL;
	}

	frag_desc->frag.page_offset += size;
	skb_frag_size_sub(&frag_desc->frag, size);

	return rmnet_frag_data_ptr(frag_desc);
}

static inline void *rmnet_frag_trim(struct rmnet_frag_descriptor *frag_desc,
				    struct rmnet_port *port,
				    unsigned int size)
{
	if (!size) {
		pr_info("%s(): Trimming %u byte pkt to 0. Dropping\n",
			__func__, skb_frag_size(&frag_desc->frag));
		rmnet_recycle_frag_descriptor(frag_desc, port);
		return NULL;
	}

	if (size < skb_frag_size(&frag_desc->frag))
		skb_frag_size_set(&frag_desc->frag, size);

	return rmnet_frag_data_ptr(frag_desc);
}

static inline void rmnet_frag_fill(struct rmnet_frag_descriptor *frag_desc,
				   struct page *p, u32 page_offset, u32 len)
{
	get_page(p);
	__skb_frag_set_page(&frag_desc->frag, p);
	skb_frag_size_set(&frag_desc->frag, len);
	frag_desc->frag.page_offset = page_offset;
}

static inline u8
rmnet_frag_get_next_hdr_type(struct rmnet_frag_descriptor *frag_desc)
{
	unsigned char *data = rmnet_frag_data_ptr(frag_desc);

	data += sizeof(struct rmnet_map_header);
	return ((struct rmnet_map_v5_coal_header *)data)->header_type;
}

static inline bool
rmnet_frag_get_csum_valid(struct rmnet_frag_descriptor *frag_desc)
{
	unsigned char *data = rmnet_frag_data_ptr(frag_desc);

	data += sizeof(struct rmnet_map_header);
	return ((struct rmnet_map_v5_csum_header *)data)->csum_valid_required;
}

#endif /* _RMNET_DESCRIPTOR_H_ */
