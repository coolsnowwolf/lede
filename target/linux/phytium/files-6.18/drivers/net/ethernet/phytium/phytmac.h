/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#ifndef _PHYTMAC_H
#define _PHYTMAC_H

#include <linux/phy.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/net_tstamp.h>
#include <linux/interrupt.h>
#include <linux/phy/phy.h>
#include <linux/phylink.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <net/xdp.h>

#define PHYTMAC_PCI_DRV_NAME		"phytmac_pci"
#define PHYTMAC_PLAT_DRV_NAME		"phytmac_platform"
#define PHYTMAC_DRV_DESC		"PHYTIUM Ethernet Driver"
#define PHYTMAC_DRIVER_VERSION		"1.0.56"
#define PHYTMAC_DEFAULT_MSG_ENABLE	  \
		(NETIF_MSG_DRV		| \
		NETIF_MSG_PROBE	| \
		NETIF_MSG_LINK	| \
		NETIF_MSG_INTR	| \
		NETIF_MSG_HW	|\
		NETIF_MSG_PKTDATA)

#define IRQ_TYPE_INT			0
#define IRQ_TYPE_MSI			1
#define IRQ_TYPE_INTX			2

#define PHY_INIT_SMC_ID			0xc2000015
#define PHY_SLOT(i)			((i) * 4)
#define PHY_INIT(i)			(0x1U << (i))
#define	PHY_MULTIPLEX_GMAC(i)		(0x2U << PHY_SLOT(i))
#define	PHY_MODE_SGMII(i)		(0x3U << PHY_SLOT(i))
#define	PHY_SPEED_1000(i)		(0x3U << PHY_SLOT(i))
#define	PHY_SPEED_2500(i)		(0x4U << PHY_SLOT(i))

#define PHYTMAC_MAX_QUEUES		8
#define DEFAULT_DMA_BURST_LENGTH	16
#define DEFAULT_JUMBO_MAX_LENGTH	10240
#define PHYTMAC_MAX_TX_LEN		16320
#define PHYTMAC_MIN_TX_LEN		64
#define DEFAULT_TX_RING_SIZE		512
#define DEFAULT_RX_RING_SIZE		512
#define MAX_TX_RING_SIZE		1024
#define MAX_RX_RING_SIZE		4096
#define MIN_TX_RING_SIZE		64
#define MIN_RX_RING_SIZE		64
#define DEFAULT_TX_DESC_MIN_FREE	64

#define MEMORY_SIZE			4096
#define MHU_SIZE			0x20

#define PHYTMAC_POWEROFF	1
#define PHYTMAC_POWERON		2
#define PHYTMAC_PWCTL_GMAC_ID		6
#define PHYTMAC_PWCTL_DEFAULT_VAL	0

#define PHYTMAC_WOL_MAGIC_PACKET	1

#define DEFAULT_MSG_RING_SIZE	16

#define PHYTMAC_MDIO_TIMEOUT	1000000 /* in usecs */

#define PHYTMAC_CAPS_JUMBO			0x00000001
#define PHYTMAC_CAPS_PTP			0x00000002
#define PHYTMAC_CAPS_BD_RD_PREFETCH		0x00000004
#define PHYTMAC_CAPS_PCS			0x00000008
#define PHYTMAC_CAPS_LSO			0x00000010
#define PHYTMAC_CAPS_SG_DISABLED		0x00000020
#define PHYTMAC_CAPS_TAILPTR			0x00000040
#define PHYTMAC_CAPS_START			0x00000080
#define PHYTMAC_CAPS_NO_WOL			0x0000100
#define PHYTMAC_CAPS_PWCTRL			0x0000200
#define PHYTMAC_CAPS_MSG			0x0000800
#define PHYTMAC_CAPS_RXPTR			0x0001000

#define VERSION_V0			0
#define VERSION_V3			0x3

#define PHYTMAC_TX			0x1
#define PHYTMAC_RX			0x2

#define PHYTMAC_ETHTOOLD_REGS_LEN	64
#define PHYTMAC_STATIS_REG_NUM		45

#define PHYTMAC_MTU_MIN_SIZE	ETH_MIN_MTU

#define EQUAL(a, b) ((a) == (b))

#define TXD_USE_COUNT(_pdata, s)	DIV_ROUND_UP((s), (_pdata)->max_tx_length)

/* Bit manipulation macros */
#define PHYTMAC_BIT(_field)					\
	(1 << PHYTMAC_##_field##_INDEX)

#define PHYTMAC_BITS(_field, value)				\
		(((value) & ((1 << PHYTMAC_##_field##_WIDTH) - 1))	\
		 << PHYTMAC_##_field##_INDEX)

#define PHYTMAC_GET_BITS(_var, _field)					\
	(((_var) >> (PHYTMAC_##_field##_INDEX))	\
	& ((0x1 << (PHYTMAC_##_field##_WIDTH)) - 1))

#define PHYTMAC_SET_BITS(_var, _field, _val)			\
	(((_var) & ~(((1 << PHYTMAC_##_field##_WIDTH) - 1)	\
		    << PHYTMAC_##_field##_INDEX))		\
	 | (((_val) & ((1 << PHYTMAC_##_field##_WIDTH) - 1))	\
	 << PHYTMAC_##_field##_INDEX))

#define PHYTMAC_READ(_pdata, _reg)					\
	__raw_readl((_pdata)->mac_regs + (_reg))

#define PHYTMAC_READ_BITS(_pdata, _reg, _field)				\
	PHYTMAC_GET_BITS(PHYTMAC_READ((_pdata), _reg), _field)

#define PHYTMAC_WRITE(_pdata, _reg, _val)				\
	__raw_writel((_val), (_pdata)->mac_regs + (_reg))

#define PHYTMAC_MSG_READ(_pdata, _reg)					\
	__raw_readl((_pdata)->mac_regs + (_reg))

#define LSO_UFO		1
#define LSO_TSO		2

#define PHYTMAC_INT_TX_COMPLETE		0x1
#define PHYTMAC_INT_TX_ERR		0x2
#define PHYTMAC_INT_RX_COMPLETE		0x4
#define PHYTMAC_INT_RX_OVERRUN		0x8
#define PHYTMAC_INT_RX_DESC_FULL	0x10
#define PHYTMAC_RX_INT_FLAGS	(PHYTMAC_INT_RX_COMPLETE)
#define PHYTMAC_TX_INT_FLAGS	(PHYTMAC_INT_TX_COMPLETE	\
				| PHYTMAC_INT_TX_ERR)

#define PHYTMAC_WAKE_MAGIC		0x00000001
#define PHYTMAC_WAKE_ARP		0x00000002
#define PHYTMAC_WAKE_UCAST		0x00000004
#define PHYTMAC_WAKE_MCAST		0x00000008

/* XDP */
#define PHYTMAC_XDP_PASS		0
#define PHYTMAC_XDP_CONSUMED		BIT(0)
#define PHYTMAC_XDP_TX			BIT(1)
#define PHYTMAC_XDP_REDIR		BIT(2)

#define PHYTMAC_DESC_NEEDED (MAX_SKB_FRAGS + 4)

enum phytmac_interface {
	PHYTMAC_PHY_INTERFACE_MODE_NA,
	PHYTMAC_PHY_INTERFACE_MODE_INTERNAL,
	PHYTMAC_PHY_INTERFACE_MODE_MII,
	PHYTMAC_PHY_INTERFACE_MODE_GMII,
	PHYTMAC_PHY_INTERFACE_MODE_SGMII,
	PHYTMAC_PHY_INTERFACE_MODE_TBI,
	PHYTMAC_PHY_INTERFACE_MODE_REVMII,
	PHYTMAC_PHY_INTERFACE_MODE_RMII,
	PHYTMAC_PHY_INTERFACE_MODE_RGMII,
	PHYTMAC_PHY_INTERFACE_MODE_RGMII_ID,
	PHYTMAC_PHY_INTERFACE_MODE_RGMII_RXID,
	PHYTMAC_PHY_INTERFACE_MODE_RGMII_TXID,
	PHYTMAC_PHY_INTERFACE_MODE_RTBI,
	PHYTMAC_PHY_INTERFACE_MODE_SMII,
	PHYTMAC_PHY_INTERFACE_MODE_XGMII,
	PHYTMAC_PHY_INTERFACE_MODE_MOCA,
	PHYTMAC_PHY_INTERFACE_MODE_QSGMII,
	PHYTMAC_PHY_INTERFACE_MODE_TRGMII,
	PHYTMAC_PHY_INTERFACE_MODE_100BASEX,
	PHYTMAC_PHY_INTERFACE_MODE_1000BASEX,
	PHYTMAC_PHY_INTERFACE_MODE_2500BASEX,
	PHYTMAC_PHY_INTERFACE_MODE_5GBASER,
	PHYTMAC_PHY_INTERFACE_MODE_RXAUI,
	PHYTMAC_PHY_INTERFACE_MODE_XAUI,
	/* 10GBASE-R, XFI, SFI - single lane 10G Serdes */
	PHYTMAC_PHY_INTERFACE_MODE_10GBASER,
	PHYTMAC_PHY_INTERFACE_MODE_USXGMII,
	/* 10GBASE-KR - with Clause 73 AN */
	PHYTMAC_PHY_INTERFACE_MODE_10GKR,
	PHYTMAC_PHY_INTERFACE_MODE_MAX,
};

struct packet_info {
	int lso;
	int desc_cnt;
	int hdrlen;
	int nocrc;
	u32 mss;
	u32 seq;
};

struct phyinit_cfg {
	u64 phy_domain;
	u64 phy_multiplex;
	u64 phy_mode;
	u64 phy_speed;
};

#define DEV_TYPE_PLATFORM	0
#define DEV_TYPE_PCI		1

struct phytmac_statistics {
	char stat_string[ETH_GSTRING_LEN];
};

#define STAT_TITLE(title) {	\
	.stat_string = title,	\
}

static const struct phytmac_statistics phytmac_statistics[] = {
	STAT_TITLE("tx_octets"),
	STAT_TITLE("tx_packets"),
	STAT_TITLE("tx_bcast_packets"),
	STAT_TITLE("tx_mcase_packets"),
	STAT_TITLE("tx_pause_packets"),
	STAT_TITLE("tx_64_byte_packets"),
	STAT_TITLE("tx_65_127_byte_packets"),
	STAT_TITLE("tx_128_255_byte_packets"),
	STAT_TITLE("tx_256_511_byte_packets"),
	STAT_TITLE("tx_512_1023_byte_packets"),
	STAT_TITLE("tx_1024_1518_byte_packets"),
	STAT_TITLE("tx_more_than_1518_byte_packets"),
	STAT_TITLE("tx_underrun"),
	STAT_TITLE("tx_single_collisions"),
	STAT_TITLE("tx_multiple_collisions"),
	STAT_TITLE("tx_excessive_collisions"),
	STAT_TITLE("tx_late_collisions"),
	STAT_TITLE("tx_deferred"),
	STAT_TITLE("tx_carrier_sense_errors"),
	STAT_TITLE("rx_octets"),
	STAT_TITLE("rx_packets"),
	STAT_TITLE("rx_bcast_packets"),
	STAT_TITLE("rx_mcast_packets"),
	STAT_TITLE("rx_pause_packets"),
	STAT_TITLE("rx_64_byte_packets"),
	STAT_TITLE("rx_65_127_byte_packets"),
	STAT_TITLE("rx_128_255_byte_packets"),
	STAT_TITLE("rx_256_511_byte_packets"),
	STAT_TITLE("rx_512_1023_byte_packets"),
	STAT_TITLE("rx_1024_1518_byte_packets"),
	STAT_TITLE("rx_more_than_1518_byte_packets"),
	STAT_TITLE("rx_undersized_packets"),
	STAT_TITLE("rx_oversize_packets"),
	STAT_TITLE("rx_jabbers"),
	STAT_TITLE("rx_fcs_errors"),
	STAT_TITLE("rx_length_errors"),
	STAT_TITLE("rx_symbol_errors"),
	STAT_TITLE("rx_alignment_errors"),
	STAT_TITLE("rx_resource_over"),
	STAT_TITLE("rx_overruns"),
	STAT_TITLE("rx_iphdr_csum_errors"),
	STAT_TITLE("rx_tcp_csum_errors"),
	STAT_TITLE("rx_udp_csum_errors"),
};

#define PHYTMAC_STATS_LEN	ARRAY_SIZE(phytmac_statistics)

/* per queue statistics, each should be unsigned long type */
struct phytmac_queue_stats {
	unsigned long rx_packets;
	unsigned long rx_bytes;
	unsigned long rx_dropped;
	unsigned long tx_packets;
	unsigned long tx_bytes;
	unsigned long tx_dropped;
};

static const struct phytmac_statistics queue_statistics[] = {
		STAT_TITLE("rx_packets"),
		STAT_TITLE("rx_bytes"),
		STAT_TITLE("rx_dropped"),
		STAT_TITLE("tx_packets"),
		STAT_TITLE("tx_bytes"),
		STAT_TITLE("tx_dropped"),
};

#define QUEUE_STATS_LEN ARRAY_SIZE(queue_statistics)

struct phytmac_config {
	struct	phytmac_hw_if *hw_if;
	u32	caps;
	u16	queue_num;
};

struct phytmac_stats {
	u64	tx_octets;
	u64	tx_packets;
	u64	tx_bcast_packets;
	u64	tx_mcase_packets;
	u64	tx_pause_packets;
	u64	tx_64_byte_packets;
	u64	tx_65_127_byte_packets;
	u64	tx_128_255_byte_packets;
	u64	tx_256_511_byte_packets;
	u64	tx_512_1023_byte_packets;
	u64	tx_1024_1518_byte_packets;
	u64	tx_more_than_1518_byte_packets;
	u64	tx_underrun;
	u64	tx_single_collisions;
	u64	tx_multiple_collisions;
	u64	tx_excessive_collisions;
	u64	tx_late_collisions;
	u64	tx_deferred;
	u64	tx_carrier_sense_errors;
	u64	rx_octets;
	u64	rx_packets;
	u64	rx_bcast_packets;
	u64	rx_mcast_packets;
	u64	rx_pause_packets;
	u64	rx_64_byte_packets;
	u64	rx_65_127_byte_packets;
	u64	rx_128_255_byte_packets;
	u64	rx_256_511_byte_packets;
	u64	rx_512_1023_byte_packets;
	u64	rx_1024_1518_byte_packets;
	u64	rx_more_than_1518_byte_packets;
	u64	rx_undersized_packets;
	u64	rx_oversize_packets;
	u64	rx_jabbers;
	u64	rx_fcs_errors;
	u64	rx_length_errors;
	u64	rx_symbol_errors;
	u64	rx_alignment_errors;
	u64	rx_resource_over;
	u64	rx_overruns;
	u64	rx_iphdr_csum_errors;
	u64	rx_tcp_csum_errors;
	u64	rx_udp_csum_errors;
};

struct ts_incr {
	u32 sub_ns;
	u32 ns;
};

enum phytmac_bd_control {
	TS_DISABLED,
	TS_FRAME_PTP_EVENT_ONLY,
	TS_ALL_PTP_FRAMES,
	TS_ALL_FRAMES,
};

#ifdef CONFIG_PHYTMAC_ENABLE_PTP
struct phytmac_dma_desc {
	u32	desc0;
	u32	desc1;
	u32	desc2;
	u32	desc3;
	u32	desc4;
	u32	desc5;
};
#else
struct phytmac_dma_desc {
	u32	desc0;
	u32	desc1;
	u32	desc2;
	u32	desc3;
};
#endif

/* TX resources are shared between XDP and netstack
 * and we need to tag the buffer type to distinguish them
 */
enum phytmac_tx_buf_type {
	PHYTMAC_TYPE_SKB = 0,
	PHYTMAC_TYPE_XDP,
};

struct phytmac_tx_skb {
	union {
		struct sk_buff		*skb;
		struct xdp_frame		*xdpf;
	};
	enum phytmac_tx_buf_type type;
	dma_addr_t		addr;
	size_t			length;
	bool			mapped_as_page;
};

struct phytmac_tx_ts {
	struct sk_buff *skb;
	u32	ts_1;
	u32	ts_2;
};

struct phytmac_rx_buffer {
	dma_addr_t addr;
	struct page *page;
	__u16 page_offset;
	__u16 pagecnt_bias;
};

struct phytmac_queue {
	struct phytmac				*pdata;
	int					irq;
	int					index;
	struct bpf_prog				*xdp_prog;

	/* tx queue info */
	unsigned int				tx_head;
	unsigned int				tx_tail;
	unsigned int				tx_xmit_more;
	dma_addr_t				tx_ring_addr;
	struct work_struct			tx_error_task;
	struct napi_struct			tx_napi;
	struct phytmac_dma_desc			*tx_ring;
	struct phytmac_tx_skb			*tx_skb;
	/* Lock to protect tx */
	spinlock_t				tx_lock;

	/* rx queue info */
	dma_addr_t				rx_ring_addr;
	unsigned int				rx_head;
	unsigned int				rx_tail;
	unsigned int				rx_next_to_alloc;
	struct phytmac_dma_desc			*rx_ring;
	struct phytmac_rx_buffer		*rx_buffer_info;
	struct napi_struct			rx_napi;
	struct phytmac_queue_stats		stats;
	struct xdp_rxq_info			xdp_rxq;

#ifdef CONFIG_PHYTMAC_ENABLE_PTP
	struct work_struct			tx_ts_task;
	unsigned int				tx_ts_head;
	unsigned int				tx_ts_tail;
	struct phytmac_tx_ts			tx_timestamps[128];
#endif
};

struct ethtool_rx_fs_item {
	struct ethtool_rx_flow_spec fs;
	struct list_head list;
};

struct ethtool_rx_fs_list {
	struct list_head list;
	unsigned int count;
};

struct phytmac_msg {
	struct completion	tx_msg_comp;
	u32			tx_msg_ring_size;
	u32			rx_msg_ring_size;
	u32			tx_msg_head;
	u32			tx_msg_wr_tail;
	u32			tx_msg_rd_tail;
	u32			rx_msg_head;
	u32			rx_msg_tail;
	/*use msg_lock to protect msg */
	spinlock_t			msg_lock;
};

struct ts_ctrl {
	int			tx_control;
	int			rx_control;
	int			one_step;
};

struct phytmac {
	void __iomem			*mac_regs;
	void __iomem			*msg_regs;
	void __iomem			*mhu_regs;
	struct pci_dev			*pcidev;
	struct platform_device		*platdev;
	struct net_device		*ndev;
	struct device			*dev;
	struct bpf_prog			*xdp_prog;
	struct ncsi_dev			*ncsidev;
	struct fwnode_handle		*fwnode;
	struct phytmac_hw_if		*hw_if;
	struct phytmac_msg		msg_ring;
	int				dev_type;
	int				sfp_irq;
	int				irq_type;
	int				queue_irq[PHYTMAC_MAX_QUEUES];
	u32				rx_irq_mask;
	u32				tx_irq_mask;
	u32				msg_enable;
	u32				capacities;
	u32				max_tx_length;
	u32				min_tx_length;
	u32				jumbo_len;
	u32				wol;
	u32				power_state;
	struct work_struct		restart_task;
	/* Lock to protect mac config */
	spinlock_t			lock;
	u32				rx_ring_size;
	u32				tx_ring_size;
	u32				dma_data_width;
	u32				dma_addr_width;
	u32				dma_burst_length;
	int				rx_bd_prefetch;
	int				tx_bd_prefetch;
	int				rx_buffer_len;
	u16				queues_max_num;
	u16				queues_num;
	struct phytmac_queue		queues[PHYTMAC_MAX_QUEUES];
	struct phytmac_stats		stats;
	u64				ethtool_stats[PHYTMAC_STATS_LEN +
						QUEUE_STATS_LEN * PHYTMAC_MAX_QUEUES];
	int				use_ncsi;
	int				use_mii;
	struct mii_bus			*mii_bus;
	struct phylink			*phylink;
	struct phylink_config		phylink_config;
	struct phylink_pcs		phylink_pcs;
	int				pause;
	phy_interface_t			phy_interface;
	enum phytmac_interface	phytmac_v2_interface;
	int				ori_speed;
	int				speed;
	int				duplex;
	int				autoneg;
	/* 1588 */
	spinlock_t			ts_clk_lock;	 /* clock locking */
	unsigned int			ts_rate;
	struct ptp_clock		*ptp_clock;
	struct ptp_clock_info		ptp_clock_info;
	struct ts_incr			ts_incr;
	struct hwtstamp_config		ts_config;
	struct ts_ctrl			ts_ctrl;
	/* RX queue filer rule set */
	struct ethtool_rx_fs_list	rx_fs_list;
	/* Lock to protect fs */
	spinlock_t			rx_fs_lock;
	unsigned int			max_rx_fs;
	u32						version;
	char				fw_version[32];
};

/* phytmac_desc_unused - calculate if we have unused descriptors */
static inline int phytmac_txdesc_unused(struct phytmac_queue *queue)
{
	struct phytmac *pdata = queue->pdata;

	if (queue->tx_head > queue->tx_tail)
		return queue->tx_head - queue->tx_tail - 1;

	return pdata->tx_ring_size + queue->tx_head - queue->tx_tail - 1;
}

static inline struct netdev_queue *phytmac_get_txq(const struct phytmac *pdata,
						   struct phytmac_queue *queue)
{
	return netdev_get_tx_queue(pdata->ndev, queue->index);
}

struct phytmac_hw_if {
	int (*init_msg_ring)(struct phytmac *pdata);
	int (*init_hw)(struct phytmac *pdata);
	void (*reset_hw)(struct phytmac *pdata);
	int (*init_ring_hw)(struct phytmac *pdata);
	int (*poweron)(struct phytmac *pdata, int on);
	int (*set_wol)(struct phytmac *pdata, int wake);
	int (*get_feature)(struct phytmac *pdata);
	int (*set_mac_address)(struct phytmac *pdata, const u8 *addr);
	int (*get_mac_address)(struct phytmac *pdata, u8 *addr);
	int (*enable_promise)(struct phytmac *pdata, int enable);
	int (*enable_multicast)(struct phytmac *pdata, int enable);
	int (*set_hash_table)(struct phytmac *pdata, unsigned long *mc_filter);
	int (*enable_rx_csum)(struct phytmac *pdata, int enable);
	int (*enable_tx_csum)(struct phytmac *pdata, int enable);
	int (*enable_pause)(struct phytmac *pdata, int enable);
	int (*enable_autoneg)(struct phytmac *pdata, int enable);
	int (*enable_network)(struct phytmac *pdata, int enable, int rx_tx);
	void (*get_stats)(struct phytmac *pdata);
	void (*get_regs)(struct phytmac *pdata, u32 *reg_buff);
	int (*set_speed)(struct phytmac *pdata);

	void (*mac_config)(struct phytmac *pdata, u32 mode,
			   const struct phylink_link_state *state);
	int (*mac_linkup)(struct phytmac *pdata, phy_interface_t interface,
			  int speed, int duplex);
	int (*mac_linkdown)(struct phytmac *pdata);
	int (*pcs_linkup)(struct phytmac *pdata, phy_interface_t interface,
			  int speed, int duplex);
	int (*pcs_linkdown)(struct phytmac *pdata);
	unsigned int  (*get_link)(struct phytmac *pdata, phy_interface_t interface);

	/* For RX coalescing */
	int (*config_rx_coalesce)(struct phytmac *pdata);
	int (*config_tx_coalesce)(struct phytmac *pdata);

	/* ethtool nfc */
	int (*add_fdir_entry)(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow);
	int (*del_fdir_entry)(struct phytmac *pdata, struct ethtool_rx_flow_spec *rx_flow);

	/* mido ops */
	void (*phy_speed_switch)(struct phytmac *pdata, int speed);
	int (*enable_mdio_control)(struct phytmac *pdata, int enable);
	int (*mdio_read)(struct phytmac *pdata, int mii_id, int regnum);
	int (*mdio_write)(struct phytmac *pdata, int mii_id,
			  int regnum, u16 data);
	int (*mdio_idle)(struct phytmac *pdata);
	int (*mdio_read_c45)(struct phytmac *pdata, int mii_id, int devad, int regnum);
	int (*mdio_write_c45)(struct phytmac *pdata, int mii_id, int devad,
			      int regnum, u16 data);
	void (*enable_irq)(struct phytmac *pdata, int queue_index, u32 mask);
	void (*disable_irq)(struct phytmac *pdata, int queue_index, u32 mask);
	void (*clear_irq)(struct phytmac *pdata, int queue_index, u32 mask);
	void (*mask_irq)(struct phytmac *pdata, int queue_index, u32 mask);
	unsigned int (*get_irq)(struct phytmac *pdata, int queue_index);
	unsigned int (*get_intx_mask)(struct phytmac *pdata);
	unsigned int (*tx_map)(struct phytmac_queue *pdata, u32 tx_tail,
			       struct packet_info *packet);
	void (*init_rx_map)(struct phytmac_queue *queue, u32 index);
	unsigned int (*rx_map)(struct phytmac_queue *queue, u32 index, dma_addr_t addr);
	void (*transmit)(struct phytmac_queue *queue);
	void (*update_rx_tail)(struct phytmac_queue *queue);
	int (*tx_complete)(const struct phytmac_dma_desc *desc);
	bool (*rx_complete)(const struct phytmac_dma_desc *desc);
	int (*get_rx_pkt_len)(struct phytmac *pdata, const struct phytmac_dma_desc *desc);
	bool (*rx_checksum)(const struct phytmac_dma_desc *desc);
	void (*set_desc_rxused)(struct phytmac_dma_desc *desc);
	bool (*rx_single_buffer)(const struct phytmac_dma_desc *desc);
	bool (*rx_pkt_start)(const struct phytmac_dma_desc *desc);
	bool (*rx_pkt_end)(const struct phytmac_dma_desc *desc);
	unsigned int (*zero_rx_desc_addr)(struct phytmac_dma_desc *desc);
	unsigned int (*zero_tx_desc)(struct phytmac_dma_desc *desc);
	void (*clear_rx_desc)(struct phytmac_queue *queue, int begin, int end);
	void (*clear_tx_desc)(struct phytmac_queue *queue);
	/* ptp */
	void (*init_ts_hw)(struct phytmac *pdata);
	void (*get_time)(struct phytmac *pdata, struct timespec64 *ts);
	void (*set_time)(struct phytmac *pdata, time64_t sec, long nsec);
	int (*set_ts_config)(struct phytmac *pdata, struct ts_ctrl *ctrl);
	void (*clear_time)(struct phytmac *pdata);
	int (*set_incr)(struct phytmac *pdata, struct ts_incr *incr);
	int (*adjust_fine)(struct phytmac *pdata, long ppm, bool negative);
	int (*adjust_time)(struct phytmac *pdata, s64 delta, int neg);
	int (*ts_valid)(struct phytmac *pdata, struct phytmac_dma_desc *desc,
			int direction);
	void (*get_timestamp)(struct phytmac *pdata, u32 ts_1, u32 ts_2,
			      struct timespec64 *ts);
	unsigned int (*get_ts_rate)(struct phytmac *pdata);
};

/* mhu */
#define PHYTMAC_MHU_AP_CPP_STAT	0x00
#define PHYTMAC_MHU_AP_CPP_SET	0x04
#define PHYTMAC_MHU_CPP_DATA0	0x18
#define PHYTMAC_MHU_CPP_DATA1	0x1c

#define PHYTMAC_MHU_STAT_BUSY_INDEX	0
#define PHYTMAC_MHU_STAT_BUSY_WIDTH	1

#define PHYTMAC_MHU_SET_INDEX		0
#define PHYTMAC_MHU_SET_WIDTH		1

#define PHYTMAC_DATA0_FREE_INDEX		0
#define PHYTMAC_DATA0_FREE_WIDTH		1
#define PHYTMAC_DATA0_DOMAIN_INDEX		1
#define PHYTMAC_DATA0_DOMAIN_WIDTH		7
#define PHYTMAC_DATA0_MSG_INDEX		8
#define PHYTMAC_DATA0_MSG_WIDTH		8
#define PHYTMAC_MSG_PM			0x04
#define PHYTMAC_DATA0_PRO_INDEX		16
#define PHYTMAC_DATA0_PRO_WIDTH		8
#define PHYTMAC_PRO_ID			0x11
#define PHYTMAC_DATA0_PAYLOAD_INDEX	24
#define PHYTMAC_DATA0_PAYLOAD_WIDTH	8

#define PHYTMAC_DATA1_STAT_INDEX		0
#define PHYTMAC_DATA1_STAT_WIDTH		28
#define PHYTMAC_STATON			8
#define PHYTMAC_STATOFF			0
#define PHYTMAC_DATA1_MUST0_INDEX		28
#define PHYTMAC_DATA1_MUST0_WIDTH		2
#define PHYTMAC_DATA1_STATTYPE_INDEX	30
#define PHYTMAC_DATA1_STATTYPE_WIDTH	1
#define PHYTMAC_STATTYPE			0x1
#define PHYTMAC_DATA1_MUST1_INDEX		31
#define PHYTMAC_DATA1_MUST1_WIDTH		1

#define PHYTMAC_MHU_READ(_pdata, _reg)					\
	__raw_readl((_pdata)->mhu_regs + (_reg))
#define PHYTMAC_MHU_WRITE(_pdata, _reg, _val)				\
		__raw_writel((_val), (_pdata)->mhu_regs + (_reg))
#define PHYTMAC_READ_STAT(pdata)	PHYTMAC_MHU_READ(pdata, PHYTMAC_MHU_AP_CPP_STAT)
#define PHYTMAC_READ_DATA0(pdata)	PHYTMAC_MHU_READ(pdata, PHYTMAC_MHU_CPP_DATA0)
#define PHYTMAC_TIMEOUT	1000000000 /* in usecs */

#define PHYTMAC_GFP_FLAGS \
	(GFP_ATOMIC | __GFP_NOWARN | __GFP_DMA32)
#define PHYTMAC_RX_DMA_ATTR \
	(DMA_ATTR_SKIP_CPU_SYNC | DMA_ATTR_WEAK_ORDERING)
#define PHYTMAC_SKB_PAD		(NET_SKB_PAD)
#define PHYTMAC_ETH_PKT_HDR_PAD		(ETH_HLEN + ETH_FCS_LEN + (VLAN_HLEN * 2))

#define PHYTMAC_RXBUFFER_2048	2048
#define PHYTMAC_MAX_FRAME_BUILD_SKB \
	(SKB_WITH_OVERHEAD(PHYTMAC_RXBUFFER_2048) - PHYTMAC_SKB_PAD)

#define PHYTMAC_RX_PAGE_ORDER	0
#define PHYTMAC_RX_PAGE_SIZE	(PAGE_SIZE << PHYTMAC_RX_PAGE_ORDER)

void __iomem *
phytmac_devm_ioremap_resource_np(struct device *dev, const struct resource *res);
struct phytmac_tx_skb *phytmac_get_tx_skb(struct phytmac_queue *queue,
					  unsigned int index);
inline struct phytmac_dma_desc *phytmac_get_tx_desc(struct phytmac_queue *queue,
					     unsigned int index);
inline struct phytmac_dma_desc *phytmac_get_rx_desc(struct phytmac_queue *queue,
					     unsigned int index);
void phytmac_set_ethtool_ops(struct net_device *netdev);
int phytmac_drv_probe(struct phytmac *pdata);
int phytmac_drv_remove(struct phytmac *pdata);
int phytmac_drv_suspend(struct phytmac *pdata);
int phytmac_drv_resume(struct phytmac *pdata);
void phytmac_drv_shutdown(struct phytmac *pdata);
struct phytmac *phytmac_alloc_pdata(struct device *dev);
void phytmac_free_pdata(struct phytmac *pdata);
int phytmac_reset_ringsize(struct phytmac *pdata, u32 rx_size, u32 tx_size);
void phytmac_set_bios_wol_enable(struct phytmac *pdata, u32 wol);
#endif
