/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef _RTL838X_ETH_H
#define _RTL838X_ETH_H

#define RTETH_MAX_MAC_REGS			3

/* Register definition */

#define RTETH_838X_CPU_PORT			28
#define RTETH_838X_DMA_IF_CTRL			(0x9f58)
#define RTETH_838X_DMA_IF_INTR_MSK		(0x9f50)
#define RTETH_838X_DMA_IF_INTR_STS		(0x9f54)
#define RTETH_838X_DMA_IF_PKT_RX_FLTR_CTRL	(0x6b10)
#define RTETH_838X_DMA_IF_PKT_TX_FLTR_CTRL	(0xaa6c)
#define RTETH_838X_DMA_IF_RX_RING_CNTR		(0xb7e8)
#define RTETH_838X_DMA_IF_RX_RING_SIZE		(0xb7e4)
#define RTETH_838X_DMA_RX_BASE			(0x9f00)
#define RTETH_838X_DMA_TX_BASE			(0x9f40)
#define RTETH_838X_MAC_ADDR_CTRL		(0xa9ec)
#define RTETH_838X_MAC_ADDR_CTRL_ALE		(0x6b04)
#define RTETH_838X_MAC_ADDR_CTRL_MAC		(0xa320)
#define RTETH_838X_MAC_FORCE_MODE_CTRL		(0xa104 + RTETH_838X_CPU_PORT * 4)
#define RTETH_838X_MAC_L2_PORT_CTRL		(0xd560 + RTETH_838X_CPU_PORT * 128)
#define RTETH_838X_QM_PKT2CPU_INTPRI_MAP	(0x5f10)
#define RTETH_838X_QM_PKT2CPU_INTPRI_0		(0x5f04)
#define RTETH_838X_QM_PKT2CPU_INTPRI_CNT	3
#define RTETH_838X_RMA_CTRL_0			(0x4300)
#define RTETH_838X_RMA_CTRL_1			(0x4304)

#define RTETH_839X_CPU_PORT			52
#define RTETH_839X_DMA_IF_CTRL			(0x786c)
#define RTETH_839X_DMA_IF_INTR_MSK		(0x7864)
#define RTETH_839X_DMA_IF_INTR_STS		(0x7868)
#define RTETH_839X_DMA_IF_PKT_FLTR_CTRL		(0x1000)
#define RTETH_839X_DMA_IF_RX_RING_CNTR		(0x603c)
#define RTETH_839X_DMA_IF_RX_RING_SIZE		(0x6038)
#define RTETH_839X_DMA_RX_BASE			(0x780c)
#define RTETH_839X_DMA_TX_BASE			(0x784c)
#define RTETH_839X_MAC_ADDR_CTRL		(0x02b4)
#define RTETH_839X_MAC_FORCE_MODE_CTRL		(0x02bc + RTETH_839X_CPU_PORT * 4)
#define RTETH_839X_MAC_L2_PORT_CTRL		(0x8004 + RTETH_839X_CPU_PORT * 128)
#define RTETH_839X_QM_PKT2CPU_INTPRI_MAP	(0x1154)
#define RTETH_839X_QM_PKT2CPU_INTPRI_0		(0x1148)
#define RTETH_839X_QM_PKT2CPU_INTPRI_CNT	3
#define RTETH_839X_RMA_CTRL_0			(0x1200)
#define RTETH_839X_RMA_CTRL_1			(0x1204)
#define RTETH_839X_RMA_CTRL_2			(0x1208)
#define RTETH_839X_RMA_CTRL_3			(0x120c)

#define RTETH_930X_CPU_PORT			28
#define RTETH_930X_DMA_IF_CTRL			(0xe028)
#define RTETH_930X_DMA_IF_INTR_MSK		(0xe010)
#define RTETH_930X_DMA_IF_INTR_STS		(0xe01c)
#define RTETH_930X_DMA_IF_RX_RING_CNTR		(0x7c8c)
#define RTETH_930X_DMA_IF_RX_RING_SIZE		(0x7c60)
#define RTETH_930X_DMA_RX_BASE			(0xdf00)
#define RTETH_930X_DMA_TX_BASE			(0xe000)
#define RTETH_930X_MAC_FORCE_MODE_CTRL		(0xca1c + RTETH_930X_CPU_PORT * 4)
#define RTETH_930X_MAC_L2_ADDR_CTRL		(0xc714)
#define RTETH_930X_MAC_L2_CPU_MAX_LEN_CTRL	(0xa3a0)
#define RTETH_930X_MAC_L2_PORT_CTRL		(0x3268 + RTETH_930X_CPU_PORT * 64)
#define RTETH_930X_MAC_L2_PORT_MAX_LEN_CTRL	(0x326c + RTETH_930X_CPU_PORT * 64)
#define RTETH_930X_QM_RSN2CPUQID_CTRL_0		(0xa344)
#define RTETH_930X_QM_RSN2CPUQID_CTRL_CNT	11
#define RTETH_930X_RMA_CTRL_0			(0x9e60)
#define RTETH_930X_RMA_CTRL_1			(0x9e64)
#define RTETH_930X_RMA_CTRL_2			(0x9e68)

#define RTETH_931X_CPU_PORT			56
#define RTETH_931X_DMA_IF_CTRL			(0x0928)
#define RTETH_931X_DMA_IF_INTR_MSK		(0x0910)
#define RTETH_931X_DMA_IF_INTR_STS		(0x091c)
#define RTETH_931X_DMA_IF_RX_RING_CNTR		(0x20ac)
#define RTETH_931X_DMA_IF_RX_RING_SIZE		(0x2080)
#define RTETH_931X_DMA_RX_BASE			(0x0800)
#define RTETH_931X_DMA_TX_BASE			(0x0900)
#define RTETH_931X_MAC_FORCE_MODE_CTRL		(0x0dcc + RTETH_931X_CPU_PORT * 4)
#define RTETH_931X_MAC_L2_ADDR_CTRL		(0x135c)
#define RTETH_931X_MAC_L2_CPU_MAX_LEN_CTRL	(0x1368)
#define RTETH_931X_MAC_L2_PORT_CTRL		(0x6000 + RTETH_931X_CPU_PORT * 128)
#define RTETH_931X_QM_RSN2CPUQID_CTRL_0		(0xa9f4)
#define RTETH_931X_QM_RSN2CPUQID_CTRL_CNT	14
#define RTETH_931X_RMA_CTRL_0			(0x8800)
#define RTETH_931X_RMA_CTRL_1			(0x8804)
#define RTETH_931X_RMA_CTRL_2			(0x8808)

/*
 * Reset
 */
#define RTL838X_RST_GLB_CTRL_0			(0x003c)
#define RTL839X_RST_GLB_CTRL			(0x0014)
#define RTL930X_RST_GLB_CTRL_0			(0x000c)
#define RTL931X_RST_GLB_CTRL			(0x0400)

/* Switch interrupts */
#define RTL839X_IMR_PORT_LINK_STS_CHG		(0x0068)
#define RTL839X_ISR_PORT_LINK_STS_CHG		(0x00a0)

/*
 * CPU port MAC control. On RTL93XX the functionality of the MAC port control register is
 * split into MAC_L2_PORT_CTRL and MAC_PORT_CTRL and the L2 register holds the important
 * bits for the driver. To avoid confusion on splitted models use the L2 naming convention
 * for all targets.
 */

/* DMA interrupt control and status registers */
#define RTL930X_L2_NTFY_IF_INTR_MSK		(0xe04C)
#define RTL930X_L2_NTFY_IF_INTR_STS		(0xe050)

/* TODO: RTL931X_DMA_IF_CTRL has different bits meanings */
#define RTL931X_L2_NTFY_IF_INTR_MSK		(0x09E4)
#define RTL931X_L2_NTFY_IF_INTR_STS		(0x09E8)

#define RTL839X_DMA_IF_INTR_NOTIFY_MASK		GENMASK(22, 20)

#define RTL838X_DMA_IF_TX_CUR_DESC_ADDR_CTRL	(0x9F48)
#define RTL930X_DMA_IF_TX_CUR_DESC_ADDR_CTRL	(0xE008)

/* L2 features */
#define RTL839X_TBL_ACCESS_L2_CTRL		(0x1180)
#define RTL839X_TBL_ACCESS_L2_DATA(idx)		(0x1184 + ((idx) << 2))
#define RTL838X_TBL_ACCESS_CTRL_0		(0x6914)
#define RTL838X_TBL_ACCESS_DATA_0(idx)		(0x6918 + ((idx) << 2))

#define RTL838X_EEE_TX_TIMER_GIGA_CTRL		(0xaa04)
#define RTL838X_EEE_TX_TIMER_GELITE_CTRL	(0xaa08)

#define RTL930X_L2_UNKN_UC_FLD_PMSK		(0x9064)
#define RTL931X_L2_UNKN_UC_FLD_PMSK		(0xC8F4)

#define RTL838X_L2_TBL_FLUSH_CTRL		(0x3370)
#define RTL839X_L2_TBL_FLUSH_CTRL		(0x3ba0)
#define RTL930X_L2_TBL_FLUSH_CTRL		(0x9404)
#define RTL931X_L2_TBL_FLUSH_CTRL		(0xCD9C)

/* MAC link state bits */
#define FORCE_EN				BIT(0)
#define FORCE_LINK_EN				BIT(1)
#define NWAY_EN					BIT(2)
#define DUPLX_MODE				BIT(3)
#define TX_PAUSE_EN				BIT(6)
#define RX_PAUSE_EN				BIT(7)

/* L2 Notification DMA interface */
#define RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL	(0x785C)
#define RTL839X_L2_NOTIFICATION_CTRL		(0x7808)
#define RTL931X_L2_NTFY_RING_BASE_ADDR		(0x09DC)
#define RTL931X_L2_NTFY_RING_CUR_ADDR		(0x09E0)
#define RTL839X_L2_NOTIFICATION_CTRL		(0x7808)
#define RTL931X_L2_NTFY_CTRL			(0xCDC8)
#define RTL838X_L2_CTRL_0			(0x3200)
#define RTL838X_L2_CTRL_1			(0x3204)
#define RTL839X_L2_CTRL_0			(0x3800)
#define RTL930X_L2_CTRL				(0x8FD8)
#define RTL931X_L2_CTRL				(0xC800)

/* TRAPPING to CPU-PORT */
#define RTL838X_SPCL_TRAP_IGMP_CTRL		(0x6984)
#define RTL839X_SPCL_TRAP_IGMP_CTRL		(0x1058)

#define RTL930X_VLAN_APP_PKT_CTRL		(0xA23C)
#define RTL931X_VLAN_APP_PKT_CTRL		(0x96b0)

/* Chip configuration registers of the RTL9310 */
#define RTL931X_MEM_ENCAP_INIT			(0x4854)
#define RTL931X_MEM_MIB_INIT			(0x7E18)
#define RTL931X_MEM_ACL_INIT			(0x40BC)
#define RTL931X_MEM_ALE_INIT_0			(0x83F0)
#define RTL931X_MEM_ALE_INIT_1			(0x83F4)
#define RTL931X_MEM_ALE_INIT_2			(0x82E4)
#define RTL931X_MDX_CTRL_RSVD			(0x0fcc)
#define RTL931X_PS_SOC_CTRL			(0x13f8)

/* shared CPU tag definitions for RTL930X/RTL931X */
#define RTL93XX_CPU_TAG1_FWD_MASK		GENMASK(11, 8)

#define RTL93XX_CPU_TAG1_FWD_ALE		0
#define RTL93XX_CPU_TAG1_FWD_PHYSICAL		1
#define RTL93XX_CPU_TAG1_FWD_LOGICAL		2
#define RTL93XX_CPU_TAG1_FWD_TRUNK		3
#define RTL93XX_CPU_TAG1_FWD_ONE_HOP		4
#define RTL93XX_CPU_TAG1_FWD_LOGICAL_ONE_HOP	5
#define RTL93XX_CPU_TAG1_FWD_UCST_CPU_MIN_PORT	6
#define RTL93XX_CPU_TAG1_FWD_UCST_CPU		7
#define RTL93XX_CPU_TAG1_FWD_BCST_CPU		8

#define RTL93XX_CPU_TAG1_IGNORE_STP_MASK	GENMASK(2, 2)

struct p_hdr;
struct dsa_tag;
struct rteth_ctrl;
struct rteth_frag;

struct rteth_config {
	int cpu_port;
	int max_mtu;
	int rx_rings;
	int tx_rx_enable;
	int tx_trigger_mask;
	int mac_l2_port_ctrl;
	int qm_pkt2cpu_intpri_map;
	int qm_rsn2cpuqid_ctrl;
	int qm_rsn2cpuqid_cnt;
	int dma_if_intr_sts;
	int dma_if_intr_msk;
	int dma_if_rx_ring_cntr;
	int dma_if_rx_ring_size;
	int l2_ntfy_if_intr_sts;
	int l2_ntfy_if_intr_msk;
	int dma_if_ctrl;
	int mac_force_mode_ctrl;
	int dma_rx_base;
	int dma_tx_base;
	int rst_glb_ctrl;
	int skb_headroom;
	u32 mac_reg[RTETH_MAX_MAC_REGS];
	int l2_tbl_flush_ctrl;
	void (*confirm_and_disable_irqs)(struct rteth_ctrl *ctrl, unsigned long *rings, bool *l2);
	void (*enable_rx_irq)(struct rteth_ctrl *ctrl, int ring);
	void (*create_tx_header)(struct rteth_frag *frag, unsigned int dest_port, int prio);
	bool (*decode_tag)(struct rteth_frag *frag, struct dsa_tag *tag);
	void (*hw_en_rxtx)(struct rteth_ctrl *ctrl);
	void (*hw_init)(struct rteth_ctrl *ctrl);
	void (*hw_stop)(struct rteth_ctrl *ctrl);
	void (*hw_reset)(struct rteth_ctrl *ctrl);
	int (*init_mac)(struct rteth_ctrl *ctrl);
	void (*set_hol)(struct rteth_ctrl *ctrl);
	void (*set_max_packet_length)(struct rteth_ctrl *ctrl, int len);
	void (*setup_notify_ring_buffer)(struct rteth_ctrl *ctrl);
	void (*update_counter)(struct rteth_ctrl *ctrl, int ring, int released);
	const struct net_device_ops *netdev_ops;
};

#endif /* _RTL838X_ETH_H */
