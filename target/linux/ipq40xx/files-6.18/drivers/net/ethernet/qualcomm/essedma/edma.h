/*
 * Copyright (c) 2014 - 2016, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _EDMA_H_
#define _EDMA_H_

#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <linux/pagemap.h>
#include <linux/smp.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/clk.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/sysctl.h>
#include <linux/phy.h>
#include <linux/of_net.h>
#include <net/checksum.h>
#include <net/ip6_checksum.h>
#include <asm-generic/bug.h>
#include "ess_edma.h"

#define EDMA_CPU_CORES_SUPPORTED 4
#define EDMA_MAX_PORTID_SUPPORTED 5
#define EDMA_MAX_VLAN_SUPPORTED  EDMA_MAX_PORTID_SUPPORTED
#define EDMA_MAX_PORTID_BITMAP_INDEX (EDMA_MAX_PORTID_SUPPORTED + 1)
#define EDMA_MAX_PORTID_BITMAP_SUPPORTED 0x1f	/* 0001_1111 = 0x1f */
#define EDMA_MAX_NETDEV_PER_QUEUE 4 /* 3 Netdev per queue, 1 space for indexing */

#define EDMA_MAX_RECEIVE_QUEUE 8
#define EDMA_MAX_TRANSMIT_QUEUE 16

/* WAN/LAN adapter number */
#define EDMA_WAN 0
#define EDMA_LAN 1

/* VLAN tag */
#define EDMA_LAN_DEFAULT_VLAN 1
#define EDMA_WAN_DEFAULT_VLAN 2

#define EDMA_DEFAULT_GROUP1_VLAN 1
#define EDMA_DEFAULT_GROUP2_VLAN 2
#define EDMA_DEFAULT_GROUP3_VLAN 3
#define EDMA_DEFAULT_GROUP4_VLAN 4
#define EDMA_DEFAULT_GROUP5_VLAN 5

/* Queues exposed to linux kernel */
#define EDMA_NETDEV_TX_QUEUE 4
#define EDMA_NETDEV_RX_QUEUE 4

/* Number of queues per core */
#define EDMA_NUM_TXQ_PER_CORE 4
#define EDMA_NUM_RXQ_PER_CORE 2

#define EDMA_TPD_EOP_SHIFT 31

#define EDMA_PORT_ID_SHIFT 12
#define EDMA_PORT_ID_MASK 0x7

/* tpd word 3 bit 18-28 */
#define EDMA_TPD_PORT_BITMAP_SHIFT 18

#define EDMA_TPD_FROM_CPU_SHIFT 25

#define EDMA_FROM_CPU_MASK 0x80
#define EDMA_SKB_PRIORITY_MASK 0x38

/* TX/RX descriptor ring count */
/* should be a power of 2 */
#define EDMA_RX_RING_SIZE 128
#define EDMA_TX_RING_SIZE 128

/* Flags used in paged/non paged mode */
#define EDMA_RX_HEAD_BUFF_SIZE_JUMBO 256
#define EDMA_RX_HEAD_BUFF_SIZE 1540

/* MAX frame size supported by switch */
#define EDMA_MAX_JUMBO_FRAME_SIZE 9216

/* Configurations */
#define EDMA_INTR_CLEAR_TYPE 0
#define EDMA_INTR_SW_IDX_W_TYPE 0
#define EDMA_FIFO_THRESH_TYPE 0
#define EDMA_RSS_TYPE 0
#define EDMA_RX_IMT 0x0020
#define EDMA_TX_IMT 0x0050
#define EDMA_TPD_BURST 5
#define EDMA_TXF_BURST 0x100
#define EDMA_RFD_BURST 8
#define EDMA_RFD_THR 16
#define EDMA_RFD_LTHR 0

/* RX/TX per CPU based mask/shift */
#define EDMA_TX_PER_CPU_MASK 0xF
#define EDMA_RX_PER_CPU_MASK 0x3
#define EDMA_TX_PER_CPU_MASK_SHIFT 0x2
#define EDMA_RX_PER_CPU_MASK_SHIFT 0x1
#define EDMA_TX_CPU_START_SHIFT 0x2
#define EDMA_RX_CPU_START_SHIFT 0x1

/* FLags used in transmit direction */
#define EDMA_HW_CHECKSUM 0x00000001
#define EDMA_VLAN_TX_TAG_INSERT_FLAG 0x00000002
#define EDMA_VLAN_TX_TAG_INSERT_DEFAULT_FLAG 0x00000004

#define EDMA_SW_DESC_FLAG_LAST 0x1
#define EDMA_SW_DESC_FLAG_SKB_HEAD 0x2
#define EDMA_SW_DESC_FLAG_SKB_FRAG 0x4
#define EDMA_SW_DESC_FLAG_SKB_FRAGLIST 0x8
#define EDMA_SW_DESC_FLAG_SKB_NONE 0x10
#define EDMA_SW_DESC_FLAG_SKB_REUSE 0x20


#define EDMA_MAX_SKB_FRAGS (MAX_SKB_FRAGS + 1)

/* Ethtool specific list of EDMA supported features */
#define EDMA_SUPPORTED_FEATURES (SUPPORTED_10baseT_Half \
					| SUPPORTED_10baseT_Full \
					| SUPPORTED_100baseT_Half \
					| SUPPORTED_100baseT_Full \
					| SUPPORTED_1000baseT_Full)

/* Recevie side atheros Header */
#define EDMA_RX_ATH_HDR_VERSION 0x2
#define EDMA_RX_ATH_HDR_VERSION_SHIFT 14
#define EDMA_RX_ATH_HDR_PRIORITY_SHIFT 11
#define EDMA_RX_ATH_PORT_TYPE_SHIFT 6
#define EDMA_RX_ATH_HDR_RSTP_PORT_TYPE 0x4

/* Transmit side atheros Header */
#define EDMA_TX_ATH_HDR_PORT_BITMAP_MASK 0x7F
#define EDMA_TX_ATH_HDR_FROM_CPU_MASK 0x80
#define EDMA_TX_ATH_HDR_FROM_CPU_SHIFT 7

#define EDMA_TXQ_START_CORE0 8
#define EDMA_TXQ_START_CORE1 12
#define EDMA_TXQ_START_CORE2 0
#define EDMA_TXQ_START_CORE3 4

#define EDMA_TXQ_IRQ_MASK_CORE0 0x0F00
#define EDMA_TXQ_IRQ_MASK_CORE1 0xF000
#define EDMA_TXQ_IRQ_MASK_CORE2 0x000F
#define EDMA_TXQ_IRQ_MASK_CORE3 0x00F0

#define EDMA_ETH_HDR_LEN 12
#define EDMA_ETH_TYPE_MASK 0xFFFF

#define EDMA_RX_BUFFER_WRITE 16
#define EDMA_RFD_AVAIL_THR 80

#define EDMA_GMAC_NO_MDIO_PHY	PHY_MAX_ADDR

extern int ssdk_rfs_ipct_rule_set(__be32 ip_src, __be32 ip_dst,
				  __be16 sport, __be16 dport,
				  uint8_t proto, u16 loadbalance, bool action);
struct edma_ethtool_statistics {
	u32 tx_q0_pkt;
	u32 tx_q1_pkt;
	u32 tx_q2_pkt;
	u32 tx_q3_pkt;
	u32 tx_q4_pkt;
	u32 tx_q5_pkt;
	u32 tx_q6_pkt;
	u32 tx_q7_pkt;
	u32 tx_q8_pkt;
	u32 tx_q9_pkt;
	u32 tx_q10_pkt;
	u32 tx_q11_pkt;
	u32 tx_q12_pkt;
	u32 tx_q13_pkt;
	u32 tx_q14_pkt;
	u32 tx_q15_pkt;
	u32 tx_q0_byte;
	u32 tx_q1_byte;
	u32 tx_q2_byte;
	u32 tx_q3_byte;
	u32 tx_q4_byte;
	u32 tx_q5_byte;
	u32 tx_q6_byte;
	u32 tx_q7_byte;
	u32 tx_q8_byte;
	u32 tx_q9_byte;
	u32 tx_q10_byte;
	u32 tx_q11_byte;
	u32 tx_q12_byte;
	u32 tx_q13_byte;
	u32 tx_q14_byte;
	u32 tx_q15_byte;
	u32 rx_q0_pkt;
	u32 rx_q1_pkt;
	u32 rx_q2_pkt;
	u32 rx_q3_pkt;
	u32 rx_q4_pkt;
	u32 rx_q5_pkt;
	u32 rx_q6_pkt;
	u32 rx_q7_pkt;
	u32 rx_q0_byte;
	u32 rx_q1_byte;
	u32 rx_q2_byte;
	u32 rx_q3_byte;
	u32 rx_q4_byte;
	u32 rx_q5_byte;
	u32 rx_q6_byte;
	u32 rx_q7_byte;
	u32 tx_desc_error;
	u32 rx_alloc_fail_ctr;
};

struct edma_mdio_data {
	struct mii_bus	*mii_bus;
	void __iomem	*membase;
	int phy_irq[PHY_MAX_ADDR];
};

/* EDMA LINK state */
enum edma_link_state {
	__EDMA_LINKUP, /* Indicate link is UP */
	__EDMA_LINKDOWN /* Indicate link is down */
};

/* EDMA GMAC state */
enum edma_gmac_state {
	__EDMA_UP /* use to indicate GMAC is up */
};

/* edma transmit descriptor */
struct edma_tx_desc {
	__le16  len; /* full packet including CRC */
	__le16  svlan_tag; /* vlan tag */
	__le32  word1; /* byte 4-7 */
	__le32  addr; /* address of buffer */
	__le32  word3; /* byte 12 */
};

/* edma receive return descriptor */
struct edma_rx_return_desc {
	u16 rrd0;
	u16 rrd1;
	u16 rrd2;
	u16 rrd3;
	u16 rrd4;
	u16 rrd5;
	u16 rrd6;
	u16 rrd7;
};

/* RFD descriptor */
struct edma_rx_free_desc {
	__le32  buffer_addr; /* buffer address */
};

/* edma hw specific data */
struct edma_hw {
	u32  __iomem *hw_addr; /* inner register address */
	struct edma_adapter *adapter; /* netdevice adapter */
	u32 rx_intr_mask; /*rx interrupt mask */
	u32 tx_intr_mask; /* tx interrupt nask */
	u32 misc_intr_mask; /* misc interrupt mask */
	u32 wol_intr_mask; /* wake on lan interrupt mask */
	bool intr_clear_type; /* interrupt clear */
	bool intr_sw_idx_w; /* interrupt software index */
	u32 rx_head_buff_size; /* Rx buffer size */
	u8 rss_type; /* rss protocol type */
};

/* edma_sw_desc stores software descriptor
 * SW descriptor has 1:1 map with HW descriptor
 */
struct edma_sw_desc {
	struct sk_buff *skb;
	dma_addr_t dma; /* dma address */
	u16 length; /* Tx/Rx buffer length */
	u32 flags;
};

/* per core related information */
struct edma_per_cpu_queues_info {
	struct napi_struct napi; /* napi associated with the core */
	u32 tx_mask; /* tx interrupt mask */
	u32 rx_mask; /* rx interrupt mask */
	u32 tx_status; /* tx interrupt status */
	u32 rx_status; /* rx interrupt status */
	u32 tx_start; /* tx queue start */
	u32 rx_start; /* rx queue start */
	struct edma_common_info *edma_cinfo; /* edma common info */
};

/* edma specific common info */
struct edma_common_info {
	struct edma_tx_desc_ring *tpd_ring[16]; /* 16 Tx queues */
	struct edma_rfd_desc_ring *rfd_ring[8]; /* 8 Rx queues */
	struct platform_device *pdev; /* device structure */
	struct net_device *netdev[EDMA_MAX_PORTID_SUPPORTED];
	struct net_device *portid_netdev_lookup_tbl[EDMA_MAX_PORTID_BITMAP_INDEX];
	struct ctl_table_header *edma_ctl_table_hdr;
	int num_gmac;
	struct edma_ethtool_statistics edma_ethstats; /* ethtool stats */
	int num_rx_queues; /* number of rx queue */
	u32 num_tx_queues; /* number of tx queue */
	u32 tx_irq[16]; /* number of tx irq */
	u32 rx_irq[8]; /* number of rx irq */
	u32 from_cpu; /* from CPU TPD field */
	u32 num_rxq_per_core; /* Rx queues per core */
	u32 num_txq_per_core; /* Tx queues per core */
	u16 tx_ring_count; /* Tx ring count */
	u16 rx_ring_count; /* Rx ring*/
	u16 rx_head_buffer_len; /* rx buffer length */
	u16 rx_page_buffer_len; /* rx buffer length */
	u32 page_mode; /* Jumbo frame supported flag */
	u32 fraglist_mode; /* fraglist supported flag */
	struct edma_hw hw; /* edma hw specific structure */
	struct edma_per_cpu_queues_info edma_percpu_info[CONFIG_NR_CPUS]; /* per cpu information */
	spinlock_t stats_lock; /* protect edma stats area for updation */
	struct timer_list edma_stats_timer;
	bool is_single_phy;
	void __iomem *ess_hw_addr;
	struct clk *ess_clk;
};

/* transimit packet descriptor (tpd) ring */
struct edma_tx_desc_ring {
	struct netdev_queue *nq[EDMA_MAX_NETDEV_PER_QUEUE]; /* Linux queue index */
	struct net_device *netdev[EDMA_MAX_NETDEV_PER_QUEUE];
			/* Array of netdevs associated with the tpd ring */
	void *hw_desc; /* descriptor ring virtual address */
	struct edma_sw_desc *sw_desc; /* buffer associated with ring */
	int netdev_bmp; /* Bitmap for per-ring netdevs */
	u32 size; /* descriptor ring length in bytes */
	u16 count; /* number of descriptors in the ring */
	dma_addr_t dma; /* descriptor ring physical address */
	u16 sw_next_to_fill; /* next Tx descriptor to fill */
	u16 sw_next_to_clean; /* next Tx descriptor to clean */
};

/* receive free descriptor (rfd) ring */
struct edma_rfd_desc_ring {
	void *hw_desc; /* descriptor ring virtual address */
	struct edma_sw_desc *sw_desc; /* buffer associated with ring */
	u16 size; /* bytes allocated to sw_desc */
	u16 count; /* number of descriptors in the ring */
	dma_addr_t dma; /* descriptor ring physical address */
	u16 sw_next_to_fill; /* next descriptor to fill */
	u16 sw_next_to_clean; /* next descriptor to clean */
	u16 pending_fill; /* fill pending from previous iteration */
};

/* edma_rfs_flter_node - rfs filter node in hash table */
struct edma_rfs_filter_node {
	struct flow_keys keys;
	u32 flow_id; /* flow_id of filter provided by kernel */
	u16 filter_id; /* filter id of filter returned by adaptor */
	u16 rq_id; /* desired rq index */
	struct hlist_node node; /* edma rfs list node */
};

/* edma_rfs_flow_tbl - rfs flow table */
struct edma_rfs_flow_table {
	u16 max_num_filter; /* Maximum number of filters edma supports */
	u16 hashtoclean; /* hash table index to clean next */
	int filter_available; /* Number of free filters available */
	struct hlist_head hlist_head[EDMA_RFS_FLOW_ENTRIES];
	spinlock_t rfs_ftab_lock;
	struct timer_list expire_rfs; /* timer function for edma_rps_may_expire_flow */
};

/* EDMA net device structure */
struct edma_adapter {
	struct net_device *netdev; /* netdevice */
	struct platform_device *pdev; /* platform device */
	struct edma_common_info *edma_cinfo; /* edma common info */
	struct phy_device *phydev; /* Phy device */
	struct edma_rfs_flow_table rfs; /* edma rfs flow table */
	struct net_device_stats stats; /* netdev statistics */
	set_rfs_filter_callback_t set_rfs_rule;
	u32 flags;/* status flags */
	unsigned long state_flags; /* GMAC up/down flags */
	u32 forced_speed; /* link force speed */
	u32 forced_duplex; /* link force duplex */
	u32 link_state; /* phy link state */
	u32 phy_mdio_addr; /* PHY device address on MII interface */
	u32 poll_required; /* check if link polling is required */
	u32 tx_start_offset[CONFIG_NR_CPUS]; /* tx queue start */
	u32 default_vlan_tag; /* vlan tag */
	u32 dp_bitmap;
	uint8_t phy_id[MII_BUS_ID_SIZE + 3];
};

int edma_alloc_queues_tx(struct edma_common_info *edma_cinfo);
int edma_alloc_queues_rx(struct edma_common_info *edma_cinfo);
int edma_open(struct net_device *netdev);
int edma_close(struct net_device *netdev);
void edma_free_tx_resources(struct edma_common_info *edma_c_info);
void edma_free_rx_resources(struct edma_common_info *edma_c_info);
int edma_alloc_tx_rings(struct edma_common_info *edma_cinfo);
int edma_alloc_rx_rings(struct edma_common_info *edma_cinfo);
void edma_free_tx_rings(struct edma_common_info *edma_cinfo);
void edma_free_rx_rings(struct edma_common_info *edma_cinfo);
void edma_free_queues(struct edma_common_info *edma_cinfo);
void edma_irq_disable(struct edma_common_info *edma_cinfo);
int edma_reset(struct edma_common_info *edma_cinfo);
int edma_poll(struct napi_struct *napi, int budget);
netdev_tx_t edma_xmit(struct sk_buff *skb,
		struct net_device *netdev);
int edma_configure(struct edma_common_info *edma_cinfo);
void edma_irq_enable(struct edma_common_info *edma_cinfo);
void edma_enable_tx_ctrl(struct edma_hw *hw);
void edma_enable_rx_ctrl(struct edma_hw *hw);
void edma_stop_rx_tx(struct edma_hw *hw);
void edma_free_irqs(struct edma_adapter *adapter);
irqreturn_t edma_interrupt(int irq, void *dev);
void edma_write_reg(u16 reg_addr, u32 reg_value);
void edma_read_reg(u16 reg_addr, volatile u32 *reg_value);
struct net_device_stats *edma_get_stats(struct net_device *netdev);
int edma_set_mac_addr(struct net_device *netdev, void *p);
int edma_rx_flow_steer(struct net_device *dev, const struct sk_buff *skb,
		u16 rxq, u32 flow_id);
int edma_register_rfs_filter(struct net_device *netdev,
		set_rfs_filter_callback_t set_filter);
void edma_flow_may_expire(struct timer_list *t);
void edma_set_ethtool_ops(struct net_device *netdev);
void edma_set_stp_rstp(bool tag);
void edma_assign_ath_hdr_type(int tag);
int edma_get_default_vlan_tag(struct net_device *netdev);
void edma_adjust_link(struct net_device *netdev);
int edma_fill_netdev(struct edma_common_info *edma_cinfo, int qid, int num, int txq_id);
void edma_read_append_stats(struct edma_common_info *edma_cinfo);
void edma_change_tx_coalesce(int usecs);
void edma_change_rx_coalesce(int usecs);
void edma_get_tx_rx_coalesce(u32 *reg_val);
void edma_clear_irq_status(void);
void ess_set_port_status_speed(struct edma_common_info *edma_cinfo,
                               struct phy_device *phydev, uint8_t port_id);
#endif /* _EDMA_H_ */
