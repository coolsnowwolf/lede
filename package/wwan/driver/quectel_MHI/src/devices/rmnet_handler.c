#if 0

#define RMNET_MAX_PACKET_SIZE      16384
#define RMNET_DFLT_PACKET_SIZE     1500
#define RMNET_NEEDED_HEADROOM      16
#define RMNET_TX_QUEUE_LEN         1000

#define RMNET_MAX_LOGICAL_EP 255
#define RMNET_MAP_DESC_HEADROOM   128
#define RMNET_FRAG_DESCRIPTOR_POOL_SIZE 64

/* Pass the frame up the stack with no modifications to skb->dev */
#define RMNET_EPMODE_NONE (0)
/* Replace skb->dev to a virtual rmnet device and pass up the stack */
#define RMNET_EPMODE_VND (1)
/* Pass the frame directly to another device with dev_queue_xmit() */
#define RMNET_EPMODE_BRIDGE (2)

/* rmnet section */

#define RMNET_FLAGS_INGRESS_DEAGGREGATION         (1U << 0)
#define RMNET_FLAGS_INGRESS_MAP_COMMANDS          (1U << 1)
#define RMNET_FLAGS_INGRESS_MAP_CKSUMV4           (1U << 2)
#define RMNET_FLAGS_EGRESS_MAP_CKSUMV4            (1U << 3)
#define RMNET_FLAGS_INGRESS_COALESCE              (1U << 4)
#define RMNET_FLAGS_INGRESS_MAP_CKSUMV5           (1U << 5)
#define RMNET_FLAGS_EGRESS_MAP_CKSUMV5            (1U << 6)

enum rmnet_map_v5_header_type {
	RMNET_MAP_HEADER_TYPE_UNKNOWN,
	RMNET_MAP_HEADER_TYPE_COALESCING = 0x1,
	RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD = 0x2,
	RMNET_MAP_HEADER_TYPE_ENUM_LENGTH
};

/* Main QMAP header */
struct rmnet_map_header {
	u8  pad_len:6;
	u8  next_hdr:1;
	u8  cd_bit:1;
	u8  mux_id;
	__be16 pkt_len;
}  __aligned(1);

/* QMAP v5 headers */
struct rmnet_map_v5_csum_header {
	u8  next_hdr:1;
	u8  header_type:7;
	u8  hw_reserved:7;
	u8  csum_valid_required:1;
	__be16 reserved;
} __aligned(1);

struct rmnet_map_v5_nl_pair {
	__be16 pkt_len;
	u8  csum_error_bitmap;
	u8  num_packets;
} __aligned(1);

/* NLO: Number-length object */
#define RMNET_MAP_V5_MAX_NLOS         (6)
#define RMNET_MAP_V5_MAX_PACKETS      (48)

struct rmnet_map_v5_coal_header {
	u8  next_hdr:1;
	u8  header_type:7;
	u8  reserved1:4;
	u8  num_nlos:3;
	u8  csum_valid:1;
	u8  close_type:4;
	u8  close_value:4;
	u8  reserved2:4;
	u8  virtual_channel_id:4;

	struct rmnet_map_v5_nl_pair nl_pairs[RMNET_MAP_V5_MAX_NLOS];
} __aligned(1);

/* QMAP v4 headers */
struct rmnet_map_dl_csum_trailer {
	u8  reserved1;
	u8  valid:1;
	u8  reserved2:7;
	u16 csum_start_offset;
	u16 csum_length;
	__be16 csum_value;
} __aligned(1);

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

struct rmnet_endpoint {
	u8 rmnet_mode;
	u8 mux_id;
	struct net_device *rmnet_dev;
};

/* One instance of this structure is instantiated for each real_dev associated
 * with rmnet.
 */
struct rmnet_port {
	struct net_device *dev;
	u8 rmnet_mode;
	u32 data_format;
	u32 nr_rmnet_devs;
	struct rmnet_endpoint muxed_ep[16];

	/* Descriptor pool */
	spinlock_t desc_pool_lock;
	struct rmnet_frag_descriptor_pool *frag_desc_pool;
	struct sk_buff *chain_head;
	struct sk_buff *chain_tail;
};

static struct sk_buff * add_qhdr_v5(struct sk_buff *skb, u8 mux_id)
{
	struct rmnet_map_header *map_header;
	struct rmnet_map_v5_csum_header *ul_header;
	u32 padding, map_datalen;

	map_datalen = skb->len;
	padding = map_datalen%4;
	if (padding) {
		padding = 4 - padding;
		if (skb_tailroom(skb) < padding) {
			printk("skb_tailroom small!\n");
			padding = 0;
		}
		if (padding)
			__skb_put(skb, padding);
	}
					
	map_header = (struct rmnet_map_header *)skb_push(skb, (sizeof(struct rmnet_map_header) + sizeof(struct rmnet_map_v5_csum_header)));

	BUILD_BUG_ON((sizeof(struct rmnet_map_header) + sizeof(struct rmnet_map_v5_csum_header)) != 8);

	map_header->cd_bit = 0;
	map_header->next_hdr = 1;
	map_header->pad_len = padding;
	map_header->mux_id = mux_id;
	map_header->pkt_len = htons(map_datalen + padding);

	ul_header = (struct rmnet_map_v5_csum_header *)(map_header + 1);
	memset(ul_header, 0, sizeof(*ul_header));
	ul_header->header_type = RMNET_MAP_HEADER_TYPE_CSUM_OFFLOAD;

	return skb;
}

struct rmnet_endpoint *rmnet_get_endpoint(struct rmnet_port *port, u8 mux_id)
{
	return &port->muxed_ep[0];
}

static void
rmnet_deliver_skb(struct sk_buff *skb, struct rmnet_port *port)
{
	struct rmnet_nss_cb *nss_cb;

	//rmnet_vnd_rx_fixup(skb->dev, skb->len);

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

static inline unsigned char *rmnet_map_data_ptr(struct sk_buff *skb)
{
	/* Nonlinear packets we receive are entirely within frag 0 */
	if (skb_is_nonlinear(skb) && skb->len == skb->data_len)
		return skb_frag_address(skb_shinfo(skb)->frags);

	return skb->data;
}

static inline void *rmnet_frag_data_ptr(struct rmnet_frag_descriptor *frag_desc)
{
	return skb_frag_address(&frag_desc->frag);
}

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

static inline void rmnet_frag_fill(struct rmnet_frag_descriptor *frag_desc,
				   struct page *p, u32 page_offset, u32 len)
{
	get_page(p);
	__skb_frag_set_page(&frag_desc->frag, p);
	skb_frag_size_set(&frag_desc->frag, len);
	frag_desc->frag.page_offset = page_offset;
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
			qmap_hex_dump(__func__, data, 64);
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
		WARN_ON(1);
		break;
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
		qmap_hex_dump(__func__, rmnet_frag_data_ptr(frag_desc), 64);
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
		goto recycle;
	}

	if (mux_id >= RMNET_MAX_LOGICAL_EP)
		goto recycle;

	ep = rmnet_get_endpoint(port, mux_id);
	if (!ep)
		goto recycle;

	frag_desc->dev = ep->rmnet_dev;

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

static void
rmnet_map_ingress_handler(struct sk_buff *skb,
			  struct rmnet_port *port)
{
	if (port->data_format & (RMNET_FLAGS_INGRESS_COALESCE |
				 RMNET_FLAGS_INGRESS_MAP_CKSUMV5)) {
		if (skb_is_nonlinear(skb)) {
			rmnet_frag_ingress_handler(skb, port);
			return;
		}
	}

	WARN_ON(1);
}

static rx_handler_result_t rmnet_rx_handler(struct sk_buff **pskb);
static int rmnet_is_real_dev_registered(const struct net_device *real_dev)
{
	return rcu_access_pointer(real_dev->rx_handler) == rmnet_rx_handler;
}


/* Needs either rcu_read_lock() or rtnl lock */
struct rmnet_port *rmnet_get_port(struct net_device *real_dev)
{
	if (rmnet_is_real_dev_registered(real_dev))
		return rcu_dereference_rtnl(real_dev->rx_handler_data);
	else
		return NULL;
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

	if (nss_debug) printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	if (skb->pkt_type == PACKET_LOOPBACK)
		return RX_HANDLER_PASS;

	if (skb->protocol != htons(ETH_P_MAP)) {
		WARN_ON(1);
		return RX_HANDLER_PASS;
	}

	dev = skb->dev;
	port = rmnet_get_port(dev);

	if (port == NULL)
		return RX_HANDLER_PASS;

	port->chain_head = NULL;
	port->chain_tail = NULL;

	switch (port->rmnet_mode) {
	case RMNET_EPMODE_VND:
		rmnet_map_ingress_handler(skb, port);
		break;
	case RMNET_EPMODE_BRIDGE:
		//rmnet_bridge_handler(skb, port->bridge_ep);
		break;
	}

done:
	return RX_HANDLER_CONSUMED;
}

static void rmnet_descriptor_deinit(struct rmnet_port *port)
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

static int rmnet_descriptor_init(struct rmnet_port *port)
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

struct rmnet_priv {
	//struct rmnet_endpoint local_ep;
	struct net_device *real_dev;
	u8 mux_id;
};

static netdev_tx_t rmnet_vnd_start_xmit(struct sk_buff *skb,
					struct net_device *dev)
{
	struct rmnet_priv *priv;

	if (nss_debug) printk("%s skb=%p, len=%d, protocol=%x, hdr_len=%d\n", __func__, skb, skb->len, skb->protocol, skb->hdr_len);

	priv = netdev_priv(dev);
	if (priv->real_dev) {
		add_qhdr_v5(skb, priv->mux_id);
		skb->protocol = htons(ETH_P_MAP);
		skb->dev = priv->real_dev;
		dev_queue_xmit(skb);
		dev->stats.tx_packets++;
		//rmnet_egress_handler(skb);
	} else {
		//this_cpu_inc(priv->pcpu_stats->stats.tx_drops);
		kfree_skb(skb);
	}
	return NETDEV_TX_OK;
}

static int rmnet_vnd_change_mtu(struct net_device *rmnet_dev, int new_mtu)
{
	if (new_mtu < 0 || new_mtu > RMNET_MAX_PACKET_SIZE)
		return -EINVAL;

	rmnet_dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops rmnet_vnd_ops = {
	.ndo_start_xmit = rmnet_vnd_start_xmit,
	.ndo_change_mtu = rmnet_vnd_change_mtu,
	//.ndo_get_iflink = rmnet_vnd_get_iflink,
	//.ndo_add_slave  = rmnet_add_bridge,
	//.ndo_del_slave  = rmnet_del_bridge,
	//.ndo_init       = rmnet_vnd_init,
	//.ndo_uninit     = rmnet_vnd_uninit,
	//.ndo_get_stats64 = rmnet_get_stats64,
};

static void rmnet_vnd_setup(struct net_device *rmnet_dev)
{
	rmnet_dev->netdev_ops = &rmnet_vnd_ops;
	rmnet_dev->mtu = RMNET_DFLT_PACKET_SIZE;
	rmnet_dev->needed_headroom = RMNET_NEEDED_HEADROOM;
	random_ether_addr(rmnet_dev->dev_addr);
	rmnet_dev->tx_queue_len = RMNET_TX_QUEUE_LEN;

	/* Raw IP mode */
	rmnet_dev->header_ops = NULL;  /* No header */
	rmnet_dev->type = ARPHRD_RAWIP;
	rmnet_dev->hard_header_len = 0;
	rmnet_dev->flags &= ~(IFF_BROADCAST | IFF_MULTICAST);

	//rmnet_dev->needs_free_netdev = true;

	rmnet_dev->hw_features = NETIF_F_RXCSUM;
	rmnet_dev->hw_features |= NETIF_F_IP_CSUM | NETIF_F_IPV6_CSUM;
	//rmnet_dev->hw_features |= NETIF_F_SG;
	//rmnet_dev->hw_features |= NETIF_F_GRO_HW;	
}
#else
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/etherdevice.h>

static uint nss_debug = 0;
module_param( nss_debug, uint, S_IRUGO | S_IWUSR);

/* rmnet section */

#define RMNET_FLAGS_INGRESS_DEAGGREGATION         (1U << 0)
#define RMNET_FLAGS_INGRESS_MAP_COMMANDS          (1U << 1)
#define RMNET_FLAGS_INGRESS_MAP_CKSUMV4           (1U << 2)
#define RMNET_FLAGS_EGRESS_MAP_CKSUMV4            (1U << 3)
#define RMNET_FLAGS_INGRESS_COALESCE              (1U << 4)
#define RMNET_FLAGS_INGRESS_MAP_CKSUMV5           (1U << 5)
#define RMNET_FLAGS_EGRESS_MAP_CKSUMV5            (1U << 6)

#ifdef CONFIG_ARCH_IPQ807x
#define CONFIG_QCA_NSS_DRV
#endif
#ifdef CONFIG_QCA_NSS_DRV
#include "rmnet/rmnet_nss.c"
#else
#include "rmnet_nss.h"
#endif

#include "rmnet/rmnet_vnd.c"
#include "rmnet/rmnet_map_command.c"
#include "rmnet/rmnet_map_data.c"
#include "rmnet/rmnet_descriptor.c"
#include "rmnet/rmnet_config.c"
#include "rmnet/rmnet_handlers.c"

struct rmnet_nss_cb *rmnet_nss_callbacks __rcu __read_mostly;

void rmnet_init(struct net_device *real_dev, u32 nr_rmnet_devs)
{
	struct rmnet_port *port;
	struct rmnet_endpoint *ep;
	struct net_device *rmnet_dev = NULL;
	u32 nr = 0;
	struct rmnet_nss_cb *nss_cb = rcu_dereference(rmnet_nss_callbacks);

	if (!nss_cb) {
#ifdef CONFIG_QCA_NSS_DRV
		rmnet_nss_init();
#endif
	}
	
	rmnet_register_real_device(real_dev);

	port = rmnet_get_port_rtnl(real_dev);

	port->data_format = RMNET_FLAGS_INGRESS_DEAGGREGATION
		| RMNET_FLAGS_INGRESS_MAP_CKSUMV5 | RMNET_FLAGS_EGRESS_MAP_CKSUMV5;
	port->rmnet_mode = RMNET_EPMODE_VND;

	for (nr = 0;  nr < nr_rmnet_devs; nr++) {
		u8 mux_id = 0x81+nr;

		rmnet_dev = alloc_netdev(sizeof(struct rmnet_priv),
					"rmnet_data%d", NET_NAME_PREDICTABLE,
					rmnet_vnd_setup);

		ep = kzalloc(sizeof(*ep), GFP_ATOMIC);

		rmnet_vnd_newlink(mux_id, rmnet_dev, port, real_dev, ep);
		netdev_rx_handler_register(rmnet_dev, rmnet_rx_priv_handler, NULL);
		hlist_add_head_rcu(&ep->hlnode, &port->muxed_ep[mux_id]);
	}

	port->nr_rmnet_devs = nr_rmnet_devs;
}

void rmnet_deinit(struct net_device *real_dev, u32 nr_rmnet_devs)
{
	struct rmnet_port *port;
	u32 nr = 0;
	struct rmnet_nss_cb *nss_cb = rcu_dereference(rmnet_nss_callbacks);
	
	port = rmnet_get_port_rtnl(real_dev);

	if (!real_dev || !rmnet_is_real_dev_registered(real_dev))
		return;

	port = rmnet_get_port_rtnl(real_dev);

	for (nr = 0;  nr < nr_rmnet_devs; nr++) {
		struct rmnet_endpoint *ep;
		u8 mux_id = 0x81+nr;

		ep = rmnet_get_endpoint(port, mux_id);
		if (ep) {
			hlist_del_init_rcu(&ep->hlnode);
			rmnet_vnd_dellink(mux_id, port, ep);
			synchronize_rcu();
			kfree(ep);
		}
	}

	rmnet_unregister_real_device(real_dev, port);

	if (nss_cb) {
#ifdef CONFIG_QCA_NSS_DRV
		rmnet_nss_exit();
#endif
	}
}
#endif
