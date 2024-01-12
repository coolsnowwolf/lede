/* Copyright (c) 2013-2017, 2019 The Linux Foundation. All rights reserved.
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
 * RMNET Data configuration engine
 *
 */

#include <linux/skbuff.h>
#include <net/gro_cells.h>

#ifndef _RMNET_CONFIG_H_
#define _RMNET_CONFIG_H_

#define RMNET_MAX_LOGICAL_EP 255
#define RMNET_MAX_VEID 4

struct rmnet_endpoint {
	u8 mux_id;
	struct net_device *egress_dev;
	struct hlist_node hlnode;
};

struct rmnet_port_priv_stats {
	u64 dl_hdr_last_qmap_vers;
	u64 dl_hdr_last_ep_id;
	u64 dl_hdr_last_trans_id;
	u64 dl_hdr_last_seq;
	u64 dl_hdr_last_bytes;
	u64 dl_hdr_last_pkts;
	u64 dl_hdr_last_flows;
	u64 dl_hdr_count;
	u64 dl_hdr_total_bytes;
	u64 dl_hdr_total_pkts;
	u64 dl_trl_last_seq;
	u64 dl_trl_count;
};

struct rmnet_egress_agg_params {
	u16 agg_size;
	u16 agg_count;
	u32 agg_time;
};

/* One instance of this structure is instantiated for each real_dev associated
 * with rmnet.
 */
struct rmnet_port {
	struct net_device *dev;
	u32 data_format;
	u8 nr_rmnet_devs;
	u8 rmnet_mode;
	struct hlist_head muxed_ep[RMNET_MAX_LOGICAL_EP];
	struct net_device *bridge_ep;
	void *rmnet_perf;

	struct rmnet_egress_agg_params egress_agg_params;

	/* Protect aggregation related elements */
	spinlock_t agg_lock;

	struct sk_buff *agg_skb;
	int agg_state;
	u8 agg_count;
	struct timespec agg_time;
	struct timespec agg_last;
	struct hrtimer hrtimer;
	struct work_struct agg_wq;

	/* dl marker elements */
	struct list_head dl_list;
	struct rmnet_port_priv_stats stats;
	int dl_marker_flush;

	/* Descriptor pool */
	spinlock_t desc_pool_lock;
	struct rmnet_frag_descriptor_pool *frag_desc_pool;
	struct sk_buff *chain_head;
	struct sk_buff *chain_tail;
};

extern struct rtnl_link_ops rmnet_link_ops;

struct rmnet_vnd_stats {
	u64 rx_pkts;
	u64 rx_bytes;
	u64 tx_pkts;
	u64 tx_bytes;
	u32 tx_drops;
};

struct rmnet_pcpu_stats {
	struct rmnet_vnd_stats stats;
	struct u64_stats_sync syncp;
};

struct rmnet_coal_close_stats {
	u64 non_coal;
	u64 ip_miss;
	u64 trans_miss;
	u64 hw_nl;
	u64 hw_pkt;
	u64 hw_byte;
	u64 hw_time;
	u64 hw_evict;
	u64 coal;
};

struct rmnet_coal_stats {
	u64 coal_rx;
	u64 coal_pkts;
	u64 coal_hdr_nlo_err;
	u64 coal_hdr_pkt_err;
	u64 coal_csum_err;
	u64 coal_reconstruct;
	u64 coal_ip_invalid;
	u64 coal_trans_invalid;
	struct rmnet_coal_close_stats close;
	u64 coal_veid[RMNET_MAX_VEID];
};

struct rmnet_priv_stats {
	u64 csum_ok;
	u64 csum_valid_unset;
	u64 csum_validation_failed;
	u64 csum_err_bad_buffer;
	u64 csum_err_invalid_ip_version;
	u64 csum_err_invalid_transport;
	u64 csum_fragmented_pkt;
	u64 csum_skipped;
	u64 csum_sw;
	u64 csum_hw;
	struct rmnet_coal_stats coal;
};

struct rmnet_priv {
	u8 mux_id;
	struct net_device *real_dev;
	struct rmnet_pcpu_stats __percpu *pcpu_stats;
	struct gro_cells gro_cells;
	struct rmnet_priv_stats stats;
};

enum rmnet_dl_marker_prio {
	RMNET_PERF,
	RMNET_SHS,
};

enum rmnet_trace_func {
	RMNET_MODULE,
	NW_STACK_MODULE,
};

enum rmnet_trace_evt {
	RMNET_DLVR_SKB,
	RMNET_RCV_FROM_PND,
	RMNET_TX_UL_PKT,
	NW_STACK_DEV_Q_XMIT,
	NW_STACK_NAPI_GRO_FLUSH,
	NW_STACK_RX,
	NW_STACK_TX,
};

static int rmnet_is_real_dev_registered(const struct net_device *real_dev);
static struct rmnet_port *rmnet_get_port(struct net_device *real_dev);
static struct rmnet_endpoint *rmnet_get_endpoint(struct rmnet_port *port, u8 mux_id);
#endif /* _RMNET_CONFIG_H_ */
