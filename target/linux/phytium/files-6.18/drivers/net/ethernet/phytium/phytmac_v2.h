/* SPDX-License-Identifier: GPL-2.0-only */
/* Copyright(c) 2022 - 2025 Phytium Technology Co., Ltd. */

#ifndef _PHYTMAC_V2_H
#define _PHYTMAC_V2_H

extern struct phytmac_hw_if phytmac_2p0_hw;

#define PHYTMAC_CMD_PRC_SUCCESS	0x1
#define PHYTMAC_MSG_SRAM_SIZE	4096
#define PHYTMAC_MSG_HDR_LEN				8
#define PHYTMAC_MSG_PARA_LEN			56
#define PHYTMAC_READ_REG_NUM_MAX		(PHYTMAC_MSG_PARA_LEN / sizeof(u32))

#define PHYTMAC_TX_MSG_HEAD				0x000
#define PHYTMAC_TX_MSG_TAIL				0x004
#define PHYTMAC_RX_MSG_HEAD				0x008
#define PHYTMAC_RX_MSG_TAIL				0x00c
#define PHYTMAC_MSG_IMR					0x020
#define PHYTMAC_MSG_ISR					0x02c
#define PHYTMAC_TAILPTR_ENABLE				0x038

#define PHYTMAC_SIZE					0x0048
#define PHYTMAC_NETWORK_STATUS				0x0240
#define PHYTMAC_PCS_AN_LP				0x0244
#define PHYTMAC_USX_LINK_STATUS				0x0248
#define PHYTMAC_MDIO					0x0264
#define PHYTMAC_TIMER_INCR_SUB_NSEC			0x024c
#define PHYTMAC_TIMER_INCR				0x0250
#define PHYTMAC_TIMER_MSB_SEC				0x0254
#define PHYTMAC_TIMER_SEC				0x0258
#define PHYTMAC_TIMER_NSEC				0x025c
#define PHYTMAC_TIMER_ADJUST				0x0260
#define PHYTMAC_MSG(i)					((i) * sizeof(struct phytmac_msg_info))
#define PHYTMAC_OCT_TX					0x400

#define PHYTMAC_MODULE_ID_GMAC				0x60
#define PHYTMAC_FLAGS_MSG_COMP				0x1
#define PHYTMAC_FLAGS_MSG_NOINT				0x2

/* Bitfields in PHYTMAC_TX_MSG_TAIL */
#define PHYTMAC_TX_MSG_INT_INDEX			16
#define PHYTMAC_TX_MSG_INT_WIDTH			1

/* Bitfields in PHYTMAC_MSG_ISR */
#define PHYTMAC_MSG_COMPLETE_INDEX			0
#define PHYTMAC_MSG_COMPLETE_WIDTH			1

/* Bitfields in PHYTMAC_TAILPTR_ENABLE */
#define PHYTMAC_TXTAIL_EN_INDEX		0	/* Enable tx tail */
#define PHYTMAC_TXTAIL_EN_WIDTH		1
#define PHYTMAC_RXTAIL_EN_INDEX		16	/* Enable rx tail */
#define PHYTMAC_RXTAIL_EN_WIDTH		1

/* Bitfields in PHYTMAC_SIZE */
#define PHYTMAC_MEM_SIZE_INDEX				0
#define PHYTMAC_MEM_SIZE_WIDTH				4
#define PHYTMAC_TXRING_SIZE_INDEX			8
#define PHYTMAC_TXRING_SIZE_WIDTH			6

/* Bitfields in PHYTMAC_TIMER_INCR_SUB_NSEC */
#define PHYTMAC_INCR_SNSECH_INDEX			0
#define PHYTMAC_INCR_SNSECH_WIDTH			16
#define PHYTMAC_INCR_SNSECL_INDEX			24
#define PHYTMAC_INCR_SNSECL_WIDTH			8
#define PHYTMAC_INCR_SNSEC_WIDTH			24

/* Bitfields in PHYTMAC_TIMER_INCR_SUB_NSEC */
#define PHYTMAC_INCR_SNSEC_INDEX			0
#define PHYTMAC_INCR_SNSEC_WIDTH			24

/* Bitfields in PHYTMAC_TIMER_INCR */
#define PHYTMAC_INCR_NSEC_INDEX				0
#define PHYTMAC_INCR_NSEC_WIDTH				8

/* Bitfields in PHYTMAC_TIMER_MSB_SEC */
#define PHYTMAC_TIMER_SECH_INDEX			0
#define PHYTMAC_TIMER_SECH_WIDTH			16

/* Bitfields in PHYTMAC_TIMER_SEC */
#define PHYTMAC_TIMER_SECL_INDEX			0
#define PHYTMAC_TIMER_SECL_WIDTH			32

/* Bitfields in PHYTMAC_TIMER_NSEC */
#define PHYTMAC_TIMER_NSEC_INDEX			0
#define PHYTMAC_TIMER_NSEC_WIDTH			30

/* Bitfields in PHYTMAC_TIMER_ADJUST */
#define PHYTMAC_ASEC_INDEX				0
#define PHYTMAC_ASEC_WIDTH				30
#define PHYTMAC_AADD_INDEX				31
#define PHYTMAC_AADD_WIDTH				1
#define PHYTMAC_ASEC_MAX		((1 << PHYTMAC_ASEC_WIDTH) - 1)

#define PHYTMAC_TIMER_SEC_WIDTH (PHYTMAC_TIMER_SECH_WIDTH + PHYTMAC_TIMER_SECL_WIDTH)
#define TIMER_SEC_MAX_VAL (((u64)1 << PHYTMAC_TIMER_SEC_WIDTH) - 1)
#define TIMER_NSEC_MAX_VAL ((1 << PHYTMAC_TIMER_NSEC_WIDTH) - 1)

#define PHYTMAC_TX_PTR(i)				(0x0100 + ((i) * 4))
#define PHYTMAC_RX_PTR(i)				(0x0030 + ((i) * 4))
#define PHYTMAC_INT_ER(i)				(0x0140 + ((i) * 4))
#define PHYTMAC_INT_DR(i)				(0x0180 + ((i) * 4))
#define PHYTMAC_INT_MR(i)				(0x01c0 + ((i) * 4))
#define PHYTMAC_INT_SR(i)				(0x0200 + ((i) * 4))

#define PHYTMAC_LINK_INDEX				0	 /* PCS link status */
#define PHYTMAC_LINK_WIDTH				1
#define PHYTMAC_MIDLE_INDEX				2	 /* Mdio idle */
#define PHYTMAC_MIDLE_WIDTH				1

/* Int stauts/Enable/Disable/Mask Register */
#define PHYTMAC_RXCOMP_INDEX			1	 /* Rx complete */
#define PHYTMAC_RXCOMP_WIDTH			1
#define PHYTMAC_RUSED_INDEX				2	 /* Rx used bit read */
#define PHYTMAC_RUSED_WIDTH				1
#define PHYTMAC_DMA_ERR_INDEX			6	 /* AMBA error */
#define PHYTMAC_DMA_ERR_WIDTH			1
#define PHYTMAC_TXCOMP_INDEX			7	 /* Tx complete */
#define PHYTMAC_TXCOMP_WIDTH			1
#define PHYTMAC_RXOVERRUN_INDEX			10	 /* Rx overrun */
#define PHYTMAC_RXOVERRUN_WIDTH			1
#define PHYTMAC_RESP_ERR_INDEX			11	 /* Resp not ok */
#define PHYTMAC_RESP_ERR_WIDTH			1

/* pcs an lp */
#define PHYTMAC_AUTO_NEG_INDEX			12
#define PHYTMAC_AUTO_NEG_WIDTH			1

/* Bitfields in USX_STATUS. */
#define PHYTMAC_USX_LINK_INDEX			0
#define PHYTMAC_USX_LINK_WIDTH			1

/* Mdio read/write Register */
#define PHYTMAC_VALUE_INDEX				0	 /* value */
#define PHYTMAC_VALUE_WIDTH				16
#define PHYTMAC_CONST_INDEX				16	 /* Must Be 10 */
#define PHYTMAC_CONST_WIDTH				2
#define PHYTMAC_REGADDR_INDEX			18	 /* Register address */
#define PHYTMAC_REGADDR_WIDTH			5
#define PHYTMAC_PHYADDR_INDEX			23	 /* Phy address */
#define PHYTMAC_PHYADDR_WIDTH			5
#define PHYTMAC_MDCOPS_INDEX			28
#define PHYTMAC_MDCOPS_WIDTH			2
#define PHYTMAC_CLAUSESEL_INDEX			30
#define PHYTMAC_CLAUSESEL_WIDTH			1
#define PHYTMAC_C22					1
#define PHYTMAC_C45					0
#define PHYTMAC_C45_ADDR				0
#define PHYTMAC_C45_WRITE				1
#define PHYTMAC_C45_READ				3
#define PHYTMAC_C22_WRITE				1
#define PHYTMAC_C22_READ				2

/* rx dma desc bit */
/* DMA descriptor bitfields */
#define PHYTMAC_RXUSED_INDEX				0
#define PHYTMAC_RXUSED_WIDTH			1
#define PHYTMAC_RXWRAP_INDEX			1
#define PHYTMAC_RXWRAP_WIDTH			1
#define PHYTMAC_RXTSVALID_INDEX			2
#define PHYTMAC_RXTSVALID_WIDTH			1
#define PHYTMAC_RXWADDR_INDEX			2
#define PHYTMAC_RXWADDR_WIDTH			30

#define PHYTMAC_RXFRMLEN_INDEX			0
#define PHYTMAC_RXFRMLEN_WIDTH			12
#define PHYTMAC_RXINDEX_INDEX			12
#define PHYTMAC_RXINDEX_WIDTH			2
#define PHYTMAC_RXSOF_INDEX			14
#define PHYTMAC_RXSOF_WIDTH			1
#define PHYTMAC_RXEOF_INDEX			15
#define PHYTMAC_RXEOF_WIDTH			1

#define PHYTMAC_RXFRMLEN_MASK			0x1FFF
#define PHYTMAC_RXJFRMLEN_MASK			0x3FFF

#define PHYTMAC_RXTYPEID_MATCH_INDEX		22
#define PHYTMAC_RXTYPEID_MATCH_WIDTH		2
#define PHYTMAC_RXCSUM_INDEX			22
#define PHYTMAC_RXCSUM_WIDTH			2

/* Buffer descriptor constants */
#define PHYTMAC_RXCSUM_NONE			0
#define PHYTMAC_RXCSUM_IP			1
#define PHYTMAC_RXCSUM_IP_TCP		2
#define PHYTMAC_RXCSUM_IP_UDP		3

#define PHYTMAC_TXFRMLEN_INDEX			0
#define PHYTMAC_TXFRMLEN_WIDTH			14
#define PHYTMAC_TXLAST_INDEX			15
#define PHYTMAC_TXLAST_WIDTH			1
#define PHYTMAC_TXNOCRC_INDEX			16
#define PHYTMAC_TXNOCRC_WIDTH			1
#define PHYTMAC_MSSMFS_INDEX			16
#define PHYTMAC_MSSMFS_WIDTH			14
#define PHYTMAC_TXLSO_INDEX				17
#define PHYTMAC_TXLSO_WIDTH				2
#define PHYTMAC_TXTCP_SEQ_SRC_INDEX		19
#define PHYTMAC_TXTCP_SEQ_SRC_WIDTH	1
#define PHYTMAC_TXTSVALID_INDEX			23
#define PHYTMAC_TXTSVALID_WIDTH			1
#define PHYTMAC_TXWRAP_INDEX			30
#define PHYTMAC_TXWRAP_WIDTH			1
#define PHYTMAC_TXUSED_INDEX			31
#define PHYTMAC_TXUSED_WIDTH			1

/* dma ts */
#define PHYTMAC_TS_NSEC_INDEX		0
#define PHYTMAC_TS_NSEC_WIDTH		30
#define PHYTMAC_TS_SECL_INDEX			30
#define PHYTMAC_TS_SECL_WIDTH		2
#define PHYTMAC_TS_SECH_INDEX		0
#define PHYTMAC_TS_SECH_WIDTH		4
#define PHYTMAC_TS_SEC_MASK			0x3f
#define PHYTMAC_TS_SEC_TOP			0x40

/* WOL register */
#define PHYTMAC_ARP_IP_INDEX		0
#define PHYTMAC_ARP_IP_WIDTH		16
#define PHYTMAC_MAGIC_INDEX			16
#define PHYTMAC_MAGIC_WIDTH			1
#define PHYTMAC_ARP_INDEX			17
#define PHYTMAC_ARP_WIDTH			1
#define PHYTMAC_UCAST_INDEX			18
#define PHYTMAC_UCAST_WIDTH			1
#define PHYTMAC_MCAST_INDEX			19
#define PHYTMAC_MCAST_WIDTH			1

#define HW_DMA_CAP_64B		0x1
#define HW_DMA_CAP_CSUM		0x2
#define HW_DMA_CAP_PTP		0x4
#define HW_DMA_CAP_DDW32	0x8
#define HW_DMA_CAP_DDW64	0x10
#define HW_DMA_CAP_DDW128	0x20

#define PHYTMAC_DBW32				1
#define PHYTMAC_DBW64				2
#define PHYTMAC_DBW128				4

#define PHYTMAC_CLK_DIV8	0
#define PHYTMAC_CLK_DIV16	1
#define PHYTMAC_CLK_DIV32	2
#define PHYTMAC_CLK_DIV48	3
#define PHYTMAC_CLK_DIV64	4
#define PHYTMAC_CLK_DIV96	5
#define PHYTMAC_CLK_DIV128	6
#define PHYTMAC_CLK_DIV224	7

#define PHYTMAC_RETRY_TIMES	50000

#define PHYTMAC_READ_NSR(pdata)	PHYTMAC_READ(pdata, PHYTMAC_NETWORK_STATUS)

enum phytmac_msg_cmd_id {
	PHYTMAC_MSG_CMD_DEFAULT = 0,
	PHYTMAC_MSG_CMD_SET,
	PHYTMAC_MSG_CMD_GET,
	PHYTMAC_MSG_CMD_DATA,
	PHYTMAC_MSG_CMD_REPORT,
};

enum phytmac_default_subid {
	PHYTMAC_MSG_CMD_DEFAULT_RESET_HW = 1,
	PHYTMAC_MSG_CMD_DEFAULT_RESET_TX_QUEUE,
	PHYTMAC_MSG_CMD_DEFAULT_RESET_RX_QUEUE,
};

enum phytmac_set_subid {
	PHYTMAC_MSG_CMD_SET_INIT_ALL = 0,
	PHYTMAC_MSG_CMD_SET_INIT_RING = 1,
	PHYTMAC_MSG_CMD_SET_INIT_TX_RING = 2,
	PHYTMAC_MSG_CMD_SET_INIT_RX_RING = 3,
	PHYTMAC_MSG_CMD_SET_INIT_MAC_CONFIG = 4,
	PHYTMAC_MSG_CMD_SET_ADDR = 5,
	PHYTMAC_MSG_CMD_SET_DMA_RX_BUFSIZE = 6,
	PHYTMAC_MSG_CMD_SET_DMA = 7,
	PHYTMAC_MSG_CMD_SET_CAPS = 8,
	PHYTMAC_MSG_CMD_SET_TS_CONFIG = 9,
	PHYTMAC_MSG_CMD_SET_INIT_TX_ENABLE_TRANSMIT = 10,
	PHYTMAC_MSG_CMD_SET_INIT_RX_ENABLE_RECEIVE = 11,
	PHYTMAC_MSG_CMD_SET_ENABLE_NETWORK = 12,
	PHYTMAC_MSG_CMD_SET_DISABLE_NETWORK = 13,
	PHYTMAC_MSG_CMD_SET_ENABLE_MDIO = 14,
	PHYTMAC_MSG_CMD_SET_DISABLE_MDIO = 15,
	PHYTMAC_MSG_CMD_SET_ENABLE_TXCSUM = 16,
	PHYTMAC_MSG_CMD_SET_DISABLE_TXCSUM = 17,
	PHYTMAC_MSG_CMD_SET_ENABLE_RXCSUM = 18,
	PHYTMAC_MSG_CMD_SET_DISABLE_RXCSUM = 19,
	PHYTMAC_MSG_CMD_SET_ENABLE_PROMISE = 20,
	PHYTMAC_MSG_CMD_SET_DISABLE_PROMISE = 21,
	PHYTMAC_MSG_CMD_SET_ENABLE_MC = 22,
	PHYTMAC_MSG_CMD_SET_DISABLE_MC = 23,
	PHYTMAC_MSG_CMD_SET_ENABLE_HASH_MC = 24,
	PHYTMAC_MSG_CMD_SET_ENABLE_PAUSE = 25,
	PHYTMAC_MSG_CMD_SET_DISABLE_PAUSE = 26,
	PHYTMAC_MSG_CMD_SET_ENABLE_JUMBO = 27,
	PHYTMAC_MSG_CMD_SET_DISABLE_JUMBO = 28,
	PHYTMAC_MSG_CMD_SET_ENABLE_1536_FRAMES = 29,
	PHYTMAC_MSG_CMD_SET_ENABLE_STRIPCRC = 30,
	PHYTMAC_MSG_CMD_SET_DISABLE_STRIPCRC = 31,
	PHYTMAC_MSG_CMD_SET_PCS_LINK_UP = 32,
	PHYTMAC_MSG_CMD_SET_PCS_LINK_DOWN = 33,
	PHYTMAC_MSG_CMD_SET_MAC_LINK_CONFIG = 34,
	PHYTMAC_MSG_CMD_SET_REG_WRITE = 35,
	PHYTMAC_MSG_CMD_SET_ENABLE_BC = 36,
	PHYTMAC_MSG_CMD_SET_DISABLE_BC = 37,
	PHYTMAC_MSG_CMD_SET_ETH_MATCH = 38,
	PHYTMAC_MSG_CMD_SET_ADD_FDIR = 39,
	PHYTMAC_MSG_CMD_SET_DEL_FDIR = 40,
	PHYTMAC_MSG_CMD_SET_ENABLE_AUTONEG = 41,
	PHYTMAC_MSG_CMD_SET_DISABLE_AUTONEG = 42,
	PHYTMAC_MSG_CMD_SET_RX_DATA_OFFSET = 43,
	PHYTMAC_MSG_CMD_SET_WOL = 44,
	PHYTMAC_MSG_CMD_SET_ENABLE_RSC = 45,
	PHYTMAC_MSG_CMD_SET_DISABLE_RSC = 46,
	PHYTMAC_MSG_CMD_SET_ENABLE_TX_START = 47,
	PHYTMAC_MSG_CMD_SET_ENABLE_PCS_RESET = 48,
	PHYTMAC_MSG_CMD_SET_DISABLE_PCS_RESET = 49,
	PHYTMAC_MSG_CMD_SET_MDC = 50,
	PHYTMAC_MSG_CMD_SET_OUTSTANDING = 51,
};

enum phytmac_get_subid {
	PHYTMAC_MSG_CMD_GET_ADDR,
	PHYTMAC_MSG_CMD_GET_QUEUENUMS,
	PHYTMAC_MSG_CMD_GET_CAPS,
	PHYTMAC_MSG_CMD_GET_BD_PREFETCH,
	PHYTMAC_MSG_CMD_GET_STATS,
	PHYTMAC_MSG_CMD_GET_REG_READ,
	PHYTMAC_MSG_CMD_GET_RX_FLOW,
	PHYTMAC_MSG_CMD_GET_REGS_FOR_ETHTOOL,
};

struct phytmac_interface_info {
	u8 interface;
	u8 autoneg;
	u16 duplex;
	u32 speed;
} __packed;

struct phytmac_mc_info {
	u32 mc_bottom;
	u32 mc_top;
} __packed;

struct phytmac_reg_info {
	u32 offset;
	u16 regnum;
} __packed;

struct phytmac_fdir_info {
	u32 ip4src;
	u32 ip4dst;
	u16 srcport;
	u16 srcport_mask;
	u16 dstport;
	u16 dstport_mask;
	u8 location;
	u8 queue;
	u8 ipsrc_en;
	u8 ipdst_en;
	u8 port_en;
} __packed;

struct phytmac_ts_config {
	u8 tx_mode;
	u8 rx_mode;
	u8 one_step;
} __packed;

struct phytmac_ring_info {
	u64 addr[4];
	u8 queue_num;
	u8 hw_dma_cap;
} __packed;

struct phytmac_rxbuf_info {
	u8 queue_num;
	u8 buffer_size;
} __packed;

struct phytmac_dma_info {
	u16 dma_burst_length;
	u8 hw_dma_cap;
} __packed;

struct phytmac_eth_info {
	u16 index;
	u16 etype;
} __packed;

struct phytmac_mac {
	u32 addrl;
	u16 addrh;
} __packed;

struct phytmac_feature {
	u8 irq_read_clear;
	u8 dma_data_width;
	u8 dma_addr_width;
	u8 tx_pkt_buffer;
	u8 rx_pkt_buffer;
	u8 pbuf_lso;
	u8 queue_num;
	u8 tx_bd_prefetch;
	u8 rx_bd_prefetch;
	u8 max_rx_fs;
} __packed;

struct phytmac_wol {
	u32 wol_type;
	u8 wake;
} __packed;

struct phytmac_msg_info {
	u8 reserved;
	u8 seq;
	u8 cmd_type;
	u8 cmd_subid;
	u16 len;
	u8 status1;
	u8 status0;
	u8 para[PHYTMAC_MSG_PARA_LEN];
} __packed;

struct phytmac_ots_config {
	u32 axi_rd;
	u32 axi_wr;
	u8 queuenum;
} __packed;

struct phytmac_ethtool_reg {
	u8 interface;
	u8 cnt;
} __packed;

static inline unsigned int phytmac_v2_tx_ring_wrap(struct phytmac *pdata, unsigned int index)
{
	return index & (pdata->msg_ring.tx_msg_ring_size - 1);
}

#endif
