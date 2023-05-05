/* SPDX-License-Identifier: GPL-2.0-only */
/*
 *
 *   Copyright (C) 2009-2016 John Crispin <blogic@openwrt.org>
 *   Copyright (C) 2009-2016 Felix Fietkau <nbd@openwrt.org>
 *   Copyright (C) 2013-2016 Michael Lee <igvtee@gmail.com>
 */

#ifndef MTK_ETH_H
#define MTK_ETH_H

#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/of_net.h>
#include <linux/u64_stats_sync.h>
#include <linux/refcount.h>
#include <linux/phylink.h>

#define MTK_QDMA_PAGE_SIZE	2048
#define	MTK_MAX_RX_LENGTH	1536
#define MTK_DMA_SIZE		2048
#define MTK_NAPI_WEIGHT		256
#define MTK_MAC_COUNT		2
#define MTK_RX_ETH_HLEN		(VLAN_ETH_HLEN + VLAN_HLEN + ETH_FCS_LEN)
#define MTK_RX_HLEN		(NET_SKB_PAD + MTK_RX_ETH_HLEN + NET_IP_ALIGN)
#define MTK_DMA_DUMMY_DESC	0xffffffff
#define MTK_DEFAULT_MSG_ENABLE	(NETIF_MSG_DRV | \
				 NETIF_MSG_PROBE | \
				 NETIF_MSG_LINK | \
				 NETIF_MSG_TIMER | \
				 NETIF_MSG_IFDOWN | \
				 NETIF_MSG_IFUP | \
				 NETIF_MSG_RX_ERR | \
				 NETIF_MSG_TX_ERR)
#define MTK_HW_FEATURES		(NETIF_F_IP_CSUM | \
				 NETIF_F_RXCSUM | \
				 NETIF_F_HW_VLAN_CTAG_TX | \
				 NETIF_F_SG | NETIF_F_TSO | \
				 NETIF_F_TSO6 | \
				 NETIF_F_IPV6_CSUM)
#define MTK_SET_FEATURES	(NETIF_F_LRO | \
				 NETIF_F_HW_VLAN_CTAG_RX)
#define MTK_HW_FEATURES_MT7628	(NETIF_F_SG | NETIF_F_RXCSUM)
#define NEXT_DESP_IDX(X, Y)	(((X) + 1) & ((Y) - 1))

#define MTK_HW_LRO_DMA_SIZE	8

#define	MTK_MAX_LRO_RX_LENGTH		(4096 * 3)
#define	MTK_MAX_LRO_IP_CNT		2
#define	MTK_HW_LRO_TIMER_UNIT		1	/* 20 us */
#define	MTK_HW_LRO_REFRESH_TIME		50000	/* 1 sec. */
#define	MTK_HW_LRO_AGG_TIME		10	/* 200us */
#define	MTK_HW_LRO_AGE_TIME		50	/* 1ms */
#define	MTK_HW_LRO_MAX_AGG_CNT		64
#define	MTK_HW_LRO_BW_THRE		3000
#define	MTK_HW_LRO_REPLACE_DELTA	1000
#define	MTK_HW_LRO_SDL_REMAIN_ROOM	1522

/* Frame Engine Global Configuration */
#define MTK_FE_GLO_CFG		0x00
#define MTK_FE_LINK_DOWN_P3	BIT(11)
#define MTK_FE_LINK_DOWN_P4	BIT(12)

/* Frame Engine Global Reset Register */
#define MTK_RST_GL		0x04
#define RST_GL_PSE		BIT(0)

/* Frame Engine Interrupt Status Register */
#define MTK_FE_INT_STATUS	0x08
#define MTK_FE_INT_STATUS2	0x28
#define MTK_FE_INT_ENABLE	0x0C
#define MTK_FE_INT_FQ_EMPTY	BIT(8)
#define MTK_FE_INT_TSO_FAIL	BIT(12)
#define MTK_FE_INT_TSO_ILLEGAL	BIT(13)
#define MTK_FE_INT_TSO_ALIGN	BIT(14)
#define MTK_FE_INT_RFIFO_OV	BIT(18)
#define MTK_FE_INT_RFIFO_UF	BIT(19)
#define MTK_GDM1_AF		BIT(28)
#define MTK_GDM2_AF		BIT(29)

/* PDMA HW LRO Alter Flow Timer Register */
#define MTK_PDMA_LRO_ALT_REFRESH_TIMER	0x1c

/* Frame Engine Interrupt Grouping Register */
#define MTK_FE_INT_GRP		0x20

/* Frame Engine LRO auto-learn table info */
#define MTK_FE_ALT_CF8		0x300
#define MTK_FE_ALT_SGL_CFC	0x304
#define MTK_FE_ALT_SEQ_CFC	0x308

/* CDMP Ingress Control Register */
#define MTK_CDMQ_IG_CTRL	0x1400
#define MTK_CDMQ_STAG_EN	BIT(0)

/* CDMP Ingress Control Register */
#define MTK_CDMP_IG_CTRL	0x400
#define MTK_CDMP_STAG_EN	BIT(0)

/* CDMP Exgress Control Register */
#define MTK_CDMP_EG_CTRL	0x404

/* GDM Exgress Control Register */
#define MTK_GDMA_FWD_CFG(x)	(0x500 + (x * 0x1000))
#define MTK_GDMA_SPECIAL_TAG	BIT(24)
#define MTK_GDMA_ICS_EN		BIT(22)
#define MTK_GDMA_TCS_EN		BIT(21)
#define MTK_GDMA_UCS_EN		BIT(20)
#define MTK_GDMA_TO_PDMA	0x0
#define MTK_GDMA_DROP_ALL	0x7777

/* Unicast Filter MAC Address Register - Low */
#define MTK_GDMA_MAC_ADRL(x)	(0x508 + (x * 0x1000))

/* Unicast Filter MAC Address Register - High */
#define MTK_GDMA_MAC_ADRH(x)	(0x50C + (x * 0x1000))

/* Internal SRAM offset */
#define MTK_ETH_SRAM_OFFSET	0x40000

/* FE global misc reg*/
#define MTK_FE_GLO_MISC         0x124

/* PSE Free Queue Flow Control  */
#define PSE_FQFC_CFG1		0x100
#define PSE_FQFC_CFG2		0x104
#define PSE_DROP_CFG		0x108

/* PSE Input Queue Reservation Register*/
#define PSE_IQ_REV(x)		(0x140 + ((x - 1) * 0x4))

/* PSE Output Queue Threshold Register*/
#define PSE_OQ_TH(x)		(0x160 + ((x - 1) * 0x4))

/* GDM and CDM Threshold */
#define MTK_GDM2_THRES		0x1530
#define MTK_CDMW0_THRES		0x164c
#define MTK_CDMW1_THRES		0x1650
#define MTK_CDME0_THRES		0x1654
#define MTK_CDME1_THRES		0x1658
#define MTK_CDMM_THRES		0x165c

#define MTK_PDMA_V2		BIT(4)

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define PDMA_BASE               0x6000
#define QDMA_BASE               0x4400
#define WDMA_BASE(x)		(0x4800 + ((x) * 0x400))
#define PPE_BASE(x)		(0x2200 + ((x) * 0x400))
#else
#define PDMA_BASE               0x0800
#define QDMA_BASE               0x1800
#define WDMA_BASE(x)		(0x2800 + ((x) * 0x400))
#define PPE_BASE(x)		(0xE00 + ((x) * 0x400))
#endif
/* PDMA RX Base Pointer Register */
#define MTK_PRX_BASE_PTR0	(PDMA_BASE + 0x100)
#define MTK_PRX_BASE_PTR_CFG(x)	(MTK_PRX_BASE_PTR0 + (x * 0x10))

/* PDMA RX Maximum Count Register */
#define MTK_PRX_MAX_CNT0	(MTK_PRX_BASE_PTR0 + 0x04)
#define MTK_PRX_MAX_CNT_CFG(x)	(MTK_PRX_MAX_CNT0 + (x * 0x10))

/* PDMA RX CPU Pointer Register */
#define MTK_PRX_CRX_IDX0	(MTK_PRX_BASE_PTR0 + 0x08)
#define MTK_PRX_CRX_IDX_CFG(x)	(MTK_PRX_CRX_IDX0 + (x * 0x10))

/* PDMA RX DMA Pointer Register */
#define MTK_PRX_DRX_IDX0	(MTK_PRX_BASE_PTR0 + 0x0c)
#define MTK_PRX_DRX_IDX_CFG(x)	(MTK_PRX_DRX_IDX0 + (x * 0x10))

/* PDMA HW LRO Control Registers */
#define BITS(m, n)			(~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_MAX_RX_RING_NUM		(8)
#define MTK_HW_LRO_RING_NUM		(4)
#define IS_HW_LRO_RING(ring_no)		(((ring_no) > 3) && ((ring_no) < 8))
#define MTK_PDMA_LRO_CTRL_DW0		(PDMA_BASE + 0x408)
#define MTK_LRO_ALT_SCORE_DELTA		(PDMA_BASE + 0x41c)
#define MTK_LRO_RX_RING0_CTRL_DW1	(PDMA_BASE + 0x438)
#define MTK_LRO_RX_RING0_CTRL_DW2	(PDMA_BASE + 0x43c)
#define MTK_LRO_RX_RING0_CTRL_DW3	(PDMA_BASE + 0x440)
#define MTK_L3_CKS_UPD_EN		BIT(19)
#define MTK_LRO_CRSN_BNW		BIT(22)
#define MTK_LRO_RING_RELINGUISH_REQ	(0xf << 24)
#define MTK_LRO_RING_RELINGUISH_DONE	(0xf << 28)
#else
#define MTK_MAX_RX_RING_NUM		(4)
#define MTK_HW_LRO_RING_NUM		(3)
#define IS_HW_LRO_RING(ring_no)		(((ring_no) > 0) && ((ring_no) < 4))
#define MTK_PDMA_LRO_CTRL_DW0		(PDMA_BASE + 0x180)
#define MTK_LRO_ALT_SCORE_DELTA		(PDMA_BASE + 0x24c)
#define MTK_LRO_RX_RING0_CTRL_DW1	(PDMA_BASE + 0x328)
#define MTK_LRO_RX_RING0_CTRL_DW2	(PDMA_BASE + 0x32c)
#define MTK_LRO_RX_RING0_CTRL_DW3	(PDMA_BASE + 0x330)
#define MTK_LRO_CRSN_BNW		BIT(6)
#define MTK_L3_CKS_UPD_EN		BIT(7)
#define MTK_LRO_RING_RELINGUISH_REQ	(0x7 << 26)
#define MTK_LRO_RING_RELINGUISH_DONE	(0x7 << 29)
#endif

#define IS_NORMAL_RING(ring_no)		((ring_no) == 0)
#define MTK_LRO_EN			BIT(0)
#define MTK_NON_LRO_MULTI_EN   		BIT(2)
#define MTK_LRO_DLY_INT_EN		BIT(5)
#define MTK_LRO_ALT_PKT_CNT_MODE	BIT(21)
#define MTK_LRO_L4_CTRL_PSH_EN		BIT(23)
#define MTK_CTRL_DW0_SDL_OFFSET		(3)
#define MTK_CTRL_DW0_SDL_MASK		BITS(3, 18)

#define MTK_PDMA_LRO_CTRL_DW1	(MTK_PDMA_LRO_CTRL_DW0 + 0x04)
#define MTK_PDMA_LRO_CTRL_DW2	(MTK_PDMA_LRO_CTRL_DW0 + 0x08)
#define MTK_PDMA_LRO_CTRL_DW3	(MTK_PDMA_LRO_CTRL_DW0 + 0x0c)
#define MTK_ADMA_MODE		BIT(15)
#define MTK_LRO_MIN_RXD_SDL	(MTK_HW_LRO_SDL_REMAIN_ROOM << 16)

/* PDMA RSS Control Registers */
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_PDMA_RSS_GLO_CFG		(PDMA_BASE + 0x800)
#define MTK_RX_NAPI_NUM			(2)
#define MTK_MAX_IRQ_NUM			(4)
#else
#define MTK_PDMA_RSS_GLO_CFG		0x3000
#define MTK_RX_NAPI_NUM			(1)
#define MTK_MAX_IRQ_NUM			(3)
#endif
#define MTK_RSS_RING1			(1)
#define MTK_RSS_EN			BIT(0)
#define MTK_RSS_CFG_REQ			BIT(2)
#define MTK_RSS_IPV6_STATIC_HASH	(0x7 << 8)
#define MTK_RSS_IPV4_STATIC_HASH	(0x7 << 12)
#define MTK_RSS_INDR_TABLE_DW0		(MTK_PDMA_RSS_GLO_CFG + 0x50)
#define MTK_RSS_INDR_TABLE_DW1		(MTK_PDMA_RSS_GLO_CFG + 0x54)
#define MTK_RSS_INDR_TABLE_DW2		(MTK_PDMA_RSS_GLO_CFG + 0x58)
#define MTK_RSS_INDR_TABLE_DW3		(MTK_PDMA_RSS_GLO_CFG + 0x5C)
#define MTK_RSS_INDR_TABLE_DW4		(MTK_PDMA_RSS_GLO_CFG + 0x60)
#define MTK_RSS_INDR_TABLE_DW5		(MTK_PDMA_RSS_GLO_CFG + 0x64)
#define MTK_RSS_INDR_TABLE_DW6		(MTK_PDMA_RSS_GLO_CFG + 0x68)
#define MTK_RSS_INDR_TABLE_DW7		(MTK_PDMA_RSS_GLO_CFG + 0x6C)
#define MTK_RSS_INDR_TABLE_SIZE4	0x44444444

/* PDMA Global Configuration Register */
#define MTK_PDMA_GLO_CFG	(PDMA_BASE + 0x204)
#define MTK_RX_DMA_LRO_EN	BIT(8)
#define MTK_MULTI_EN		BIT(10)
#define MTK_PDMA_SIZE_8DWORDS	(1 << 4)

/* PDMA Global Configuration Register */
#define MTK_PDMA_RX_CFG		(PDMA_BASE + 0x210)
#define MTK_PDMA_LRO_SDL	(0x3000)
#define MTK_RX_CFG_SDL_OFFSET	(16)

/* PDMA Reset Index Register */
#define MTK_PDMA_RST_IDX	(PDMA_BASE + 0x208)
#define MTK_PST_DRX_IDX0	BIT(16)
#define MTK_PST_DRX_IDX_CFG(x)	(MTK_PST_DRX_IDX0 << (x))

/* PDMA Delay Interrupt Register */
#define MTK_PDMA_DELAY_INT		(PDMA_BASE + 0x20c)
#define MTK_PDMA_DELAY_RX_EN		BIT(15)
#define MTK_PDMA_DELAY_RX_PINT		4
#define MTK_PDMA_DELAY_RX_PINT_SHIFT	8
#define MTK_PDMA_DELAY_RX_PTIME		4
#define MTK_PDMA_DELAY_RX_DELAY		\
	(MTK_PDMA_DELAY_RX_EN | MTK_PDMA_DELAY_RX_PTIME | \
	(MTK_PDMA_DELAY_RX_PINT << MTK_PDMA_DELAY_RX_PINT_SHIFT))

/* PDMA Interrupt Status Register */
#define MTK_PDMA_INT_STATUS	(PDMA_BASE + 0x220)

/* PDMA Interrupt Mask Register */
#define MTK_PDMA_INT_MASK	(PDMA_BASE + 0x228)

/* PDMA Interrupt grouping registers */
#define MTK_PDMA_INT_GRP1	(PDMA_BASE + 0x250)
#define MTK_PDMA_INT_GRP2	(PDMA_BASE + 0x254)
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_PDMA_INT_GRP3	(PDMA_BASE + 0x258)
#else
#define MTK_PDMA_INT_GRP3	(PDMA_BASE + 0x22c)
#endif
#define MTK_LRO_RX1_DLY_INT	0xa70
#define MTK_MAX_DELAY_INT	0x8f0f8f0f

/* PDMA HW LRO IP Setting Registers */
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_LRO_RX_RING0_DIP_DW0	(PDMA_BASE + 0x414)
#else
#define MTK_LRO_RX_RING0_DIP_DW0	(PDMA_BASE + 0x304)
#endif
#define MTK_LRO_DIP_DW0_CFG(x)		(MTK_LRO_RX_RING0_DIP_DW0 + (x * 0x40))
#define MTK_RING_MYIP_VLD		BIT(9)

/* PDMA HW LRO ALT Debug Registers */
#define MTK_LRO_ALT_DBG			(PDMA_BASE + 0x440)
#define MTK_LRO_ALT_INDEX_OFFSET	(8)

/* PDMA HW LRO ALT Data Registers */
#define MTK_LRO_ALT_DBG_DATA		(PDMA_BASE + 0x444)

/* PDMA HW LRO Ring Control Registers */
#define MTK_LRO_CTRL_DW1_CFG(x)		(MTK_LRO_RX_RING0_CTRL_DW1 + (x * 0x40))
#define MTK_LRO_CTRL_DW2_CFG(x)		(MTK_LRO_RX_RING0_CTRL_DW2 + (x * 0x40))
#define MTK_LRO_CTRL_DW3_CFG(x)		(MTK_LRO_RX_RING0_CTRL_DW3 + (x * 0x40))
#define MTK_RING_AGE_TIME_L		((MTK_HW_LRO_AGE_TIME & 0x3ff) << 22)
#define MTK_RING_AGE_TIME_H		((MTK_HW_LRO_AGE_TIME >> 10) & 0x3f)
#define MTK_RING_PSE_MODE        	(1 << 6)
#define MTK_RING_AUTO_LERAN_MODE	(3 << 6)
#define MTK_RING_VLD			BIT(8)
#define MTK_RING_MAX_AGG_TIME		((MTK_HW_LRO_AGG_TIME & 0xffff) << 10)
#define MTK_RING_MAX_AGG_CNT_L		((MTK_HW_LRO_MAX_AGG_CNT & 0x3f) << 26)
#define MTK_RING_MAX_AGG_CNT_H		((MTK_HW_LRO_MAX_AGG_CNT >> 6) & 0x3)

/* LRO_RX_RING_CTRL_DW masks */
#define MTK_LRO_RING_AGG_TIME_MASK	BITS(10, 25)
#define MTK_LRO_RING_AGG_CNT_L_MASK	BITS(26, 31)
#define MTK_LRO_RING_AGG_CNT_H_MASK	BITS(0, 1)
#define MTK_LRO_RING_AGE_TIME_L_MASK	BITS(22, 31)
#define MTK_LRO_RING_AGE_TIME_H_MASK	BITS(0, 5)

/* LRO_RX_RING_CTRL_DW0 offsets */
#define MTK_RX_IPV6_FORCE_OFFSET	(0)
#define MTK_RX_IPV4_FORCE_OFFSET	(1)

/* LRO_RX_RING_CTRL_DW1 offsets  */
#define MTK_LRO_RING_AGE_TIME_L_OFFSET	(22)

/* LRO_RX_RING_CTRL_DW2 offsets  */
#define MTK_LRO_RING_AGE_TIME_H_OFFSET	(0)
#define MTK_RX_MODE_OFFSET		(6)
#define MTK_RX_PORT_VALID_OFFSET	(8)
#define MTK_RX_MYIP_VALID_OFFSET	(9)
#define MTK_LRO_RING_AGG_TIME_OFFSET	(10)
#define MTK_LRO_RING_AGG_CNT_L_OFFSET	(26)

/* LRO_RX_RING_CTRL_DW3 offsets  */
#define MTK_LRO_RING_AGG_CNT_H_OFFSET	(0)

/* LRO_RX_RING_STP_DTP_DW offsets */
#define MTK_RX_TCP_DEST_PORT_OFFSET	(0)
#define MTK_RX_TCP_SRC_PORT_OFFSET	(16)

/* QDMA TX Queue Configuration Registers */
#define MTK_QTX_CFG(x)		(QDMA_BASE + (x * 0x10))
#define QDMA_RES_THRES		4

/* QDMA TX Queue Scheduler Registers */
#define MTK_QTX_SCH(x)		(QDMA_BASE + 4 + (x * 0x10))

/* QDMA RX Base Pointer Register */
#define MTK_QRX_BASE_PTR0	(QDMA_BASE + 0x100)
#define MTK_QRX_BASE_PTR_CFG(x)	(MTK_QRX_BASE_PTR0 + ((x) * 0x10))

/* QDMA RX Maximum Count Register */
#define MTK_QRX_MAX_CNT0	(QDMA_BASE + 0x104)
#define MTK_QRX_MAX_CNT_CFG(x)	(MTK_QRX_MAX_CNT0 + ((x) * 0x10))

/* QDMA RX CPU Pointer Register */
#define MTK_QRX_CRX_IDX0	(QDMA_BASE + 0x108)
#define MTK_QRX_CRX_IDX_CFG(x)	(MTK_QRX_CRX_IDX0 + ((x) * 0x10))

/* QDMA RX DMA Pointer Register */
#define MTK_QRX_DRX_IDX0	(QDMA_BASE + 0x10c)

/* QDMA Global Configuration Register */
#define MTK_QDMA_GLO_CFG	(QDMA_BASE + 0x204)
#define MTK_RX_2B_OFFSET	BIT(31)
#define MTK_RX_BT_32DWORDS	(3 << 11)
#define MTK_NDP_CO_PRO		BIT(10)
#define MTK_TX_WB_DDONE		BIT(6)
#define MTK_DMA_SIZE_16DWORDS	(2 << 4)
#define MTK_DMA_SIZE_32DWORDS	(3 << 4)
#define MTK_RX_DMA_BUSY		BIT(3)
#define MTK_TX_DMA_BUSY		BIT(1)
#define MTK_RX_DMA_EN		BIT(2)
#define MTK_TX_DMA_EN		BIT(0)
#define MTK_DMA_BUSY_TIMEOUT	HZ

/* QDMA V2 Global Configuration Register */
#define MTK_CHK_DDONE_EN	BIT(28)
#define MTK_DMAD_WR_WDONE	BIT(26)
#define MTK_WCOMP_EN		BIT(24)
#define MTK_RESV_BUF		(0x80 << 16)
#define MTK_MUTLI_CNT		(0x4 << 12)
#define MTK_RESV_BUF_MASK	(0xff << 16)

/* QDMA Reset Index Register */
#define MTK_QDMA_RST_IDX	(QDMA_BASE + 0x208)

/* QDMA Delay Interrupt Register */
#define MTK_QDMA_DELAY_INT	(QDMA_BASE + 0x20c)

/* QDMA Flow Control Register */
#define MTK_QDMA_FC_THRES	(QDMA_BASE + 0x210)
#define FC_THRES_DROP_MODE	BIT(20)
#define FC_THRES_DROP_EN	(7 << 16)
#define FC_THRES_MIN		0x4444

/* QDMA Interrupt Status Register */
#define MTK_QDMA_INT_STATUS	(QDMA_BASE + 0x218)
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_RX_DONE_INT(ring_no)		\
	((ring_no)? BIT(16 + (ring_no)) : BIT(14))
#else
#define MTK_RX_DONE_INT(ring_no)		\
	((ring_no)? BIT(24 + (ring_no)) : BIT(30))
#endif
#define MTK_RX_DONE_INT3	BIT(19)
#define MTK_RX_DONE_INT2	BIT(18)
#define MTK_RX_DONE_INT1	BIT(17)
#define MTK_RX_DONE_INT0	BIT(16)
#define MTK_TX_DONE_INT3	BIT(3)
#define MTK_TX_DONE_INT2	BIT(2)
#define MTK_TX_DONE_INT1	BIT(1)
#define MTK_TX_DONE_INT0	BIT(0)
#define MTK_TX_DONE_DLY         BIT(28)
#define MTK_TX_DONE_INT         MTK_TX_DONE_DLY

/* QDMA Interrupt grouping registers */
#define MTK_QDMA_INT_GRP1	(QDMA_BASE + 0x220)
#define MTK_QDMA_INT_GRP2	(QDMA_BASE + 0x224)
#define MTK_RLS_DONE_INT	BIT(0)

/* QDMA Interrupt Status Register */
#define MTK_QDMA_INT_MASK	(QDMA_BASE + 0x21c)

/* QDMA DMA FSM */
#define MTK_QDMA_FSM		(QDMA_BASE + 0x234)

/* QDMA Interrupt Mask Register */
#define MTK_QDMA_HRED2		(QDMA_BASE + 0x244)

/* QDMA TX Forward CPU Pointer Register */
#define MTK_QTX_CTX_PTR		(QDMA_BASE +0x300)

/* QDMA TX Forward DMA Pointer Register */
#define MTK_QTX_DTX_PTR		(QDMA_BASE +0x304)

/* QDMA TX Forward DMA Counter */
#define MTK_QDMA_FWD_CNT	(QDMA_BASE + 0x308)

/* QDMA TX Release CPU Pointer Register */
#define MTK_QTX_CRX_PTR		(QDMA_BASE +0x310)

/* QDMA TX Release DMA Pointer Register */
#define MTK_QTX_DRX_PTR		(QDMA_BASE +0x314)

/* QDMA FQ Head Pointer Register */
#define MTK_QDMA_FQ_HEAD	(QDMA_BASE +0x320)

/* QDMA FQ Head Pointer Register */
#define MTK_QDMA_FQ_TAIL	(QDMA_BASE +0x324)

/* QDMA FQ Free Page Counter Register */
#define MTK_QDMA_FQ_CNT		(QDMA_BASE +0x328)

/* QDMA FQ Free Page Buffer Length Register */
#define MTK_QDMA_FQ_BLEN	(QDMA_BASE +0x32c)

/* WDMA Registers */
#define MTK_WDMA_DTX_PTR(x)	(WDMA_BASE(x) + 0xC)
#define MTK_WDMA_GLO_CFG(x)	(WDMA_BASE(x) + 0x204)
#define MTK_WDMA_TX_DBG_MON0(x)	(WDMA_BASE(x) + 0x230)
#define MTK_CDM_TXFIFO_RDY	BIT(7)

/* GMA1 Received Good Byte Count Register */
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_GDM1_TX_GBCNT       0x1C00
#else
#define MTK_GDM1_TX_GBCNT	0x2400
#endif
#define MTK_STAT_OFFSET		0x40

/* QDMA TX NUM */
#define MTK_QDMA_TX_NUM		16
#define MTK_QDMA_TX_MASK	((MTK_QDMA_TX_NUM) - 1)
#define QID_LOW_BITS(x)         ((x) & 0xf)
#define QID_HIGH_BITS(x)        ((((x) >> 4) & 0x3) << 20)
#define QID_BITS_V2(x)		(((x) & 0x3f) << 16)

#define MTK_QDMA_GMAC2_QID	8

/* QDMA V2 descriptor txd6 */
#define TX_DMA_INS_VLAN_V2         BIT(16)

/* QDMA V2 descriptor txd5 */
#define TX_DMA_CHKSUM_V2           (0x7 << 28)
#define TX_DMA_TSO_V2              BIT(31)

/* QDMA V2 descriptor txd4 */
#define TX_DMA_FPORT_SHIFT_V2      8
#define TX_DMA_FPORT_MASK_V2       0xf
#define TX_DMA_SWC_V2              BIT(30)

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_TX_DMA_BUF_LEN      0xffff
#define MTK_TX_DMA_BUF_SHIFT    8
#else
#define MTK_TX_DMA_BUF_LEN      0x3fff
#define MTK_TX_DMA_BUF_SHIFT    16
#endif

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MTK_RX_DMA_BUF_LEN      0xffff
#define MTK_RX_DMA_BUF_SHIFT    8
#define RX_DMA_SPORT_SHIFT      26
#define RX_DMA_SPORT_MASK       0xf
#else
#define MTK_RX_DMA_BUF_LEN      0x3fff
#define MTK_RX_DMA_BUF_SHIFT    16
#define RX_DMA_SPORT_SHIFT      19
#define RX_DMA_SPORT_MASK       0x7
#endif

/* QDMA descriptor txd4 */
#define TX_DMA_CHKSUM		(0x7 << 29)
#define TX_DMA_TSO		BIT(28)
#define TX_DMA_FPORT_SHIFT	25
#define TX_DMA_FPORT_MASK	0x7
#define TX_DMA_INS_VLAN		BIT(16)

/* QDMA descriptor txd3 */
#define TX_DMA_OWNER_CPU	BIT(31)
#define TX_DMA_LS0		BIT(30)
#define TX_DMA_PLEN0(_x)	(((_x) & MTK_TX_DMA_BUF_LEN) << MTK_TX_DMA_BUF_SHIFT)
#define TX_DMA_PLEN1(_x)	((_x) & MTK_TX_DMA_BUF_LEN)
#define TX_DMA_SWC		BIT(14)
#define TX_DMA_SDL(_x)		(TX_DMA_PLEN0(_x))

/* PDMA on MT7628 */
#define TX_DMA_DONE		BIT(31)
#define TX_DMA_LS1		BIT(14)
#define TX_DMA_DESP2_DEF	(TX_DMA_LS0 | TX_DMA_DONE)

/* QDMA descriptor rxd2 */
#define RX_DMA_DONE		BIT(31)
#define RX_DMA_LSO		BIT(30)
#define RX_DMA_PLEN0(_x)	(((_x) & MTK_RX_DMA_BUF_LEN) << MTK_RX_DMA_BUF_SHIFT)
#define RX_DMA_GET_PLEN0(_x)	(((_x) >> MTK_RX_DMA_BUF_SHIFT) & MTK_RX_DMA_BUF_LEN)
#define RX_DMA_GET_AGG_CNT(_x)	(((_x) >> 2) & 0xff)
#define RX_DMA_GET_REV(_x)	(((_x) >> 10) & 0x1f)
#define RX_DMA_VTAG		BIT(15)

/* QDMA descriptor rxd3 */
#define RX_DMA_VID(_x)		((_x) & VLAN_VID_MASK)
#define RX_DMA_TCI(_x)		((_x) & (VLAN_PRIO_MASK | VLAN_VID_MASK))
#define RX_DMA_VPID(_x)		(((_x) >> 16) & 0xffff)

/* QDMA descriptor rxd4 */
#define RX_DMA_L4_VALID		BIT(24)
#define RX_DMA_L4_VALID_PDMA	BIT(30)		/* when PDMA is used */
#define RX_DMA_SPECIAL_TAG	BIT(22)		/* switch header in packet */

#define RX_DMA_GET_SPORT(_x) 	(((_x) >> RX_DMA_SPORT_SHIFT) & RX_DMA_SPORT_MASK)

/* PDMA V2 descriptor rxd3 */
#define RX_DMA_VTAG_V2          BIT(0)
#define RX_DMA_L4_VALID_V2      BIT(2)

/* PDMA V2 descriptor rxd4 */
#define RX_DMA_VID_V2(_x)       RX_DMA_VID(_x)
#define RX_DMA_TCI_V2(_x)	RX_DMA_TCI(_x)
#define RX_DMA_VPID_V2(_x)	RX_DMA_VPID(_x)

/* PDMA V2 descriptor rxd6 */
#define RX_DMA_GET_FLUSH_RSN_V2(_x)	((_x) & 0x7)
#define RX_DMA_GET_AGG_CNT_V2(_x)	(((_x) >> 16) & 0xff)

/* PHY Indirect Access Control registers */
#define MTK_PHY_IAC		0x10004
#define PHY_IAC_ACCESS		BIT(31)
#define PHY_IAC_READ		BIT(19)
#define PHY_IAC_WRITE		BIT(18)
#define PHY_IAC_START		BIT(16)
#define PHY_IAC_ADDR_SHIFT	20
#define PHY_IAC_REG_SHIFT	25
#define PHY_IAC_TIMEOUT		HZ

#define MTK_MAC_MISC		0x1000c
#define MTK_MUX_TO_ESW		BIT(0)

/* Mac control registers */
#define MTK_MAC_MCR(x)		(0x10100 + (x * 0x100))
#define MAC_MCR_MAX_RX_1536	BIT(24)
#define MAC_MCR_IPG_CFG		(BIT(18) | BIT(16))
#define MAC_MCR_FORCE_MODE	BIT(15)
#define MAC_MCR_TX_EN		BIT(14)
#define MAC_MCR_RX_EN		BIT(13)
#define MAC_MCR_BACKOFF_EN	BIT(9)
#define MAC_MCR_BACKPR_EN	BIT(8)
#define MAC_MCR_FORCE_RX_FC	BIT(5)
#define MAC_MCR_FORCE_TX_FC	BIT(4)
#define MAC_MCR_SPEED_1000	BIT(3)
#define MAC_MCR_SPEED_100	BIT(2)
#define MAC_MCR_FORCE_DPX	BIT(1)
#define MAC_MCR_FORCE_LINK	BIT(0)
#define MAC_MCR_FORCE_LINK_DOWN	(MAC_MCR_FORCE_MODE)

/* Mac status registers */
#define MTK_MAC_MSR(x)		(0x10108 + (x * 0x100))
#define MAC_MSR_EEE1G		BIT(7)
#define MAC_MSR_EEE100M		BIT(6)
#define MAC_MSR_RX_FC		BIT(5)
#define MAC_MSR_TX_FC		BIT(4)
#define MAC_MSR_SPEED_1000	BIT(3)
#define MAC_MSR_SPEED_100	BIT(2)
#define MAC_MSR_SPEED_MASK	(MAC_MSR_SPEED_1000 | MAC_MSR_SPEED_100)
#define MAC_MSR_DPX		BIT(1)
#define MAC_MSR_LINK		BIT(0)

/* TRGMII RXC control register */
#define TRGMII_RCK_CTRL		0x10300
#define DQSI0(x)		((x << 0) & GENMASK(6, 0))
#define DQSI1(x)		((x << 8) & GENMASK(14, 8))
#define RXCTL_DMWTLAT(x)	((x << 16) & GENMASK(18, 16))
#define RXC_RST			BIT(31)
#define RXC_DQSISEL		BIT(30)
#define RCK_CTRL_RGMII_1000	(RXC_DQSISEL | RXCTL_DMWTLAT(2) | DQSI1(16))
#define RCK_CTRL_RGMII_10_100	RXCTL_DMWTLAT(2)

#define NUM_TRGMII_CTRL		5

/* TRGMII RXC control register */
#define TRGMII_TCK_CTRL		0x10340
#define TXCTL_DMWTLAT(x)	((x << 16) & GENMASK(18, 16))
#define TXC_INV			BIT(30)
#define TCK_CTRL_RGMII_1000	TXCTL_DMWTLAT(2)
#define TCK_CTRL_RGMII_10_100	(TXC_INV | TXCTL_DMWTLAT(2))

/* TRGMII TX Drive Strength */
#define TRGMII_TD_ODT(i)	(0x10354 + 8 * (i))
#define  TD_DM_DRVP(x)		((x) & 0xf)
#define  TD_DM_DRVN(x)		(((x) & 0xf) << 4)

/* TRGMII Interface mode register */
#define INTF_MODE		0x10390
#define TRGMII_INTF_DIS		BIT(0)
#define TRGMII_MODE		BIT(1)
#define TRGMII_CENTRAL_ALIGNED	BIT(2)
#define INTF_MODE_RGMII_1000    (TRGMII_MODE | TRGMII_CENTRAL_ALIGNED)
#define INTF_MODE_RGMII_10_100  0

/* GPIO port control registers for GMAC 2*/
#define GPIO_OD33_CTRL8		0x4c0
#define GPIO_BIAS_CTRL		0xed0
#define GPIO_DRV_SEL10		0xf00

/* ethernet subsystem chip id register */
#define ETHSYS_CHIPID0_3	0x0
#define ETHSYS_CHIPID4_7	0x4
#define MT7623_ETH		7623
#define MT7622_ETH		7622
#define MT7621_ETH		7621

/* ethernet system control register */
#define ETHSYS_SYSCFG		0x10
#define SYSCFG_DRAM_TYPE_DDR2	BIT(4)

/* ethernet subsystem config register */
#define ETHSYS_SYSCFG0		0x14
#define SYSCFG0_GE_MASK		0x3
#define SYSCFG0_GE_MODE(x, y)	(x << (12 + (y * 2)))
#define SYSCFG0_SGMII_MASK     GENMASK(9, 8)
#define SYSCFG0_SGMII_GMAC1    ((2 << 8) & SYSCFG0_SGMII_MASK)
#define SYSCFG0_SGMII_GMAC2    ((3 << 8) & SYSCFG0_SGMII_MASK)
#define SYSCFG0_SGMII_GMAC1_V2 BIT(9)
#define SYSCFG0_SGMII_GMAC2_V2 BIT(8)


/* ethernet subsystem clock register */
#define ETHSYS_CLKCFG0		0x2c
#define ETHSYS_TRGMII_CLK_SEL362_5	BIT(11)
#define ETHSYS_TRGMII_MT7621_MASK	(BIT(5) | BIT(6))
#define ETHSYS_TRGMII_MT7621_APLL	BIT(6)
#define ETHSYS_TRGMII_MT7621_DDR_PLL	BIT(5)

/* ethernet reset control register */
#define ETHSYS_RSTCTRL	0x34
#define RSTCTRL_FE	BIT(6)
#define RSTCTRL_ETH 	BIT(23)
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define RSTCTRL_PPE0	BIT(30)
#define RSTCTRL_PPE1 	BIT(31)
#else
#define RSTCTRL_PPE0	BIT(31)
#define RSTCTRL_PPE1 	0
#endif

/* ethernet reset check idle register */
#define ETHSYS_FE_RST_CHK_IDLE_EN 	0x28


/* SGMII subsystem config registers */
/* Register to auto-negotiation restart */
#define SGMSYS_PCS_CONTROL_1	0x0
#define SGMII_AN_RESTART	BIT(9)
#define SGMII_ISOLATE		BIT(10)
#define SGMII_AN_ENABLE		BIT(12)
#define SGMII_LINK_STATYS	BIT(18)
#define SGMII_AN_ABILITY	BIT(19)
#define SGMII_AN_COMPLETE	BIT(21)
#define SGMII_PCS_FAULT		BIT(23)
#define SGMII_AN_EXPANSION_CLR	BIT(30)

/* Register to programmable link timer, the unit in 2 * 8ns */
#define SGMSYS_PCS_LINK_TIMER	0x18
#define SGMII_LINK_TIMER_DEFAULT	(0x186a0 & GENMASK(19, 0))

/* Register to control remote fault */
#define SGMSYS_SGMII_MODE		0x20
#define SGMII_IF_MODE_BIT0		BIT(0)
#define SGMII_SPEED_DUPLEX_AN		BIT(1)
#define SGMII_SPEED_10			0x0
#define SGMII_SPEED_100			BIT(2)
#define SGMII_SPEED_1000		BIT(3)
#define SGMII_DUPLEX_FULL		BIT(4)
#define SGMII_IF_MODE_BIT5		BIT(5)
#define SGMII_REMOTE_FAULT_DIS		BIT(8)
#define SGMII_CODE_SYNC_SET_VAL		BIT(9)
#define SGMII_CODE_SYNC_SET_EN		BIT(10)
#define SGMII_SEND_AN_ERROR_EN		BIT(11)
#define SGMII_IF_MODE_MASK		GENMASK(5, 1)

/* Register to set SGMII speed, ANA RG_ Control Signals III*/
#define SGMSYS_ANA_RG_CS3	0x2028
#define RG_PHY_SPEED_MASK	(BIT(2) | BIT(3))
#define RG_PHY_SPEED_1_25G	0x0
#define RG_PHY_SPEED_3_125G	BIT(2)

/* Register to power up QPHY */
#define SGMSYS_QPHY_PWR_STATE_CTRL 0xe8
#define	SGMII_PHYA_PWD		BIT(4)

/* Register to QPHY wrapper control */
#define SGMSYS_QPHY_WRAP_CTRL	0xec
#define SGMII_PN_SWAP_MASK	GENMASK(1, 0)
#define SGMII_PN_SWAP_TX_RX	(BIT(0) | BIT(1))

/* Infrasys subsystem config registers */
#define INFRA_MISC2            0x70c
#define CO_QPHY_SEL            BIT(0)
#define GEPHY_MAC_SEL          BIT(1)

/* Top misc registers */
#define USB_PHY_SWITCH_REG     0x218
#define QPHY_SEL_MASK          GENMASK(1, 0)
#define SGMII_QPHY_SEL	       0x2

/*MDIO control*/
#define MII_MMD_ACC_CTL_REG             0x0d
#define MII_MMD_ADDR_DATA_REG           0x0e
#define MMD_OP_MODE_DATA BIT(14)

/* MT7628/88 specific stuff */
#define MT7628_PDMA_OFFSET	0x0800
#define MT7628_SDM_OFFSET	0x0c00

#define MT7628_TX_BASE_PTR0	(MT7628_PDMA_OFFSET + 0x00)
#define MT7628_TX_MAX_CNT0	(MT7628_PDMA_OFFSET + 0x04)
#define MT7628_TX_CTX_IDX0	(MT7628_PDMA_OFFSET + 0x08)
#define MT7628_TX_DTX_IDX0	(MT7628_PDMA_OFFSET + 0x0c)
#define MT7628_PST_DTX_IDX0	BIT(0)

#define MT7628_SDM_MAC_ADRL	(MT7628_SDM_OFFSET + 0x0c)
#define MT7628_SDM_MAC_ADRH	(MT7628_SDM_OFFSET + 0x10)

struct mtk_rx_dma {
	unsigned int rxd1;
	unsigned int rxd2;
	unsigned int rxd3;
	unsigned int rxd4;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	unsigned int rxd5;
	unsigned int rxd6;
	unsigned int rxd7;
	unsigned int rxd8;
#endif
} __packed __aligned(4);

struct mtk_tx_dma {
	unsigned int txd1;
	unsigned int txd2;
	unsigned int txd3;
	unsigned int txd4;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	unsigned int txd5;
	unsigned int txd6;
	unsigned int txd7;
	unsigned int txd8;
#endif
} __packed __aligned(4);

struct mtk_eth;
struct mtk_mac;

/* struct mtk_hw_stats - the structure that holds the traffic statistics.
 * @stats_lock:		make sure that stats operations are atomic
 * @reg_offset:		the status register offset of the SoC
 * @syncp:		the refcount
 *
 * All of the supported SoCs have hardware counters for traffic statistics.
 * Whenever the status IRQ triggers we can read the latest stats from these
 * counters and store them in this struct.
 */
struct mtk_hw_stats {
	u64 tx_bytes;
	u64 tx_packets;
	u64 tx_skip;
	u64 tx_collisions;
	u64 rx_bytes;
	u64 rx_packets;
	u64 rx_overflow;
	u64 rx_fcs_errors;
	u64 rx_short_errors;
	u64 rx_long_errors;
	u64 rx_checksum_errors;
	u64 rx_flow_control_packets;

	spinlock_t		stats_lock;
	u32			reg_offset;
	struct u64_stats_sync	syncp;
};

enum mtk_tx_flags {
	/* PDMA descriptor can point at 1-2 segments. This enum allows us to
	 * track how memory was allocated so that it can be freed properly.
	 */
	MTK_TX_FLAGS_SINGLE0	= 0x01,
	MTK_TX_FLAGS_PAGE0	= 0x02,

	/* MTK_TX_FLAGS_FPORTx allows tracking which port the transmitted
	 * SKB out instead of looking up through hardware TX descriptor.
	 */
	MTK_TX_FLAGS_FPORT0	= 0x04,
	MTK_TX_FLAGS_FPORT1	= 0x08,
};

/* This enum allows us to identify how the clock is defined on the array of the
 * clock in the order
 */
enum mtk_clks_map {
	MTK_CLK_ETHIF,
	MTK_CLK_SGMIITOP,
	MTK_CLK_ESW,
	MTK_CLK_GP0,
	MTK_CLK_GP1,
	MTK_CLK_GP2,
	MTK_CLK_FE,
	MTK_CLK_TRGPLL,
	MTK_CLK_SGMII_TX_250M,
	MTK_CLK_SGMII_RX_250M,
	MTK_CLK_SGMII_CDR_REF,
	MTK_CLK_SGMII_CDR_FB,
	MTK_CLK_SGMII2_TX_250M,
	MTK_CLK_SGMII2_RX_250M,
	MTK_CLK_SGMII2_CDR_REF,
	MTK_CLK_SGMII2_CDR_FB,
	MTK_CLK_SGMII_CK,
	MTK_CLK_ETH2PLL,
	MTK_CLK_WOCPU0,
	MTK_CLK_WOCPU1,
	MTK_CLK_MAX
};

#define MT7623_CLKS_BITMAP	(BIT(MTK_CLK_ETHIF) | BIT(MTK_CLK_ESW) |  \
				 BIT(MTK_CLK_GP1) | BIT(MTK_CLK_GP2) | \
				 BIT(MTK_CLK_TRGPLL))
#define MT7622_CLKS_BITMAP	(BIT(MTK_CLK_ETHIF) | BIT(MTK_CLK_ESW) |  \
				 BIT(MTK_CLK_GP0) | BIT(MTK_CLK_GP1) | \
				 BIT(MTK_CLK_GP2) | \
				 BIT(MTK_CLK_SGMII_TX_250M) | \
				 BIT(MTK_CLK_SGMII_RX_250M) | \
				 BIT(MTK_CLK_SGMII_CDR_REF) | \
				 BIT(MTK_CLK_SGMII_CDR_FB) | \
				 BIT(MTK_CLK_SGMII_CK) | \
				 BIT(MTK_CLK_ETH2PLL))
#define MT7621_CLKS_BITMAP	(0)
#define MT7628_CLKS_BITMAP	(0)
#define MT7629_CLKS_BITMAP	(BIT(MTK_CLK_ETHIF) | BIT(MTK_CLK_ESW) |  \
				 BIT(MTK_CLK_GP0) | BIT(MTK_CLK_GP1) | \
				 BIT(MTK_CLK_GP2) | BIT(MTK_CLK_FE) | \
				 BIT(MTK_CLK_SGMII_TX_250M) | \
				 BIT(MTK_CLK_SGMII_RX_250M) | \
				 BIT(MTK_CLK_SGMII_CDR_REF) | \
				 BIT(MTK_CLK_SGMII_CDR_FB) | \
				 BIT(MTK_CLK_SGMII2_TX_250M) | \
				 BIT(MTK_CLK_SGMII2_RX_250M) | \
				 BIT(MTK_CLK_SGMII2_CDR_REF) | \
				 BIT(MTK_CLK_SGMII2_CDR_FB) | \
				 BIT(MTK_CLK_SGMII_CK) | \
				 BIT(MTK_CLK_ETH2PLL) | BIT(MTK_CLK_SGMIITOP))

#define MT7986_CLKS_BITMAP	(BIT(MTK_CLK_FE) | BIT(MTK_CLK_GP2) | BIT(MTK_CLK_GP1) | \
                                 BIT(MTK_CLK_WOCPU1) | BIT(MTK_CLK_WOCPU0) | \
                                 BIT(MTK_CLK_SGMII_TX_250M) | \
                                 BIT(MTK_CLK_SGMII_RX_250M) | \
                                 BIT(MTK_CLK_SGMII_CDR_REF) | \
                                 BIT(MTK_CLK_SGMII_CDR_FB) | \
                                 BIT(MTK_CLK_SGMII2_TX_250M) | \
                                 BIT(MTK_CLK_SGMII2_RX_250M) | \
                                 BIT(MTK_CLK_SGMII2_CDR_REF) | \
                                 BIT(MTK_CLK_SGMII2_CDR_FB))


#define MT7981_CLKS_BITMAP	(BIT(MTK_CLK_FE) | BIT(MTK_CLK_GP2) | BIT(MTK_CLK_GP1) | \
                                 BIT(MTK_CLK_WOCPU0) | \
                                 BIT(MTK_CLK_SGMII_TX_250M) | \
                                 BIT(MTK_CLK_SGMII_RX_250M) | \
                                 BIT(MTK_CLK_SGMII_CDR_REF) | \
                                 BIT(MTK_CLK_SGMII_CDR_FB) | \
                                 BIT(MTK_CLK_SGMII2_TX_250M) | \
                                 BIT(MTK_CLK_SGMII2_RX_250M) | \
                                 BIT(MTK_CLK_SGMII2_CDR_REF) | \
                                 BIT(MTK_CLK_SGMII2_CDR_FB))
enum mtk_dev_state {
	MTK_HW_INIT,
	MTK_RESETTING
};

/* struct mtk_tx_buf -	This struct holds the pointers to the memory pointed at
 *			by the TX descriptor	s
 * @skb:		The SKB pointer of the packet being sent
 * @dma_addr0:		The base addr of the first segment
 * @dma_len0:		The length of the first segment
 * @dma_addr1:		The base addr of the second segment
 * @dma_len1:		The length of the second segment
 */
struct mtk_tx_buf {
	struct sk_buff *skb;
	u32 flags;
	DEFINE_DMA_UNMAP_ADDR(dma_addr0);
	DEFINE_DMA_UNMAP_LEN(dma_len0);
	DEFINE_DMA_UNMAP_ADDR(dma_addr1);
	DEFINE_DMA_UNMAP_LEN(dma_len1);
};

/* struct mtk_tx_ring -	This struct holds info describing a TX ring
 * @dma:		The descriptor ring
 * @buf:		The memory pointed at by the ring
 * @phys:		The physical addr of tx_buf
 * @next_free:		Pointer to the next free descriptor
 * @last_free:		Pointer to the last free descriptor
 * @last_free_ptr:	Hardware pointer value of the last free descriptor
 * @thresh:		The threshold of minimum amount of free descriptors
 * @free_count:		QDMA uses a linked list. Track how many free descriptors
 *			are present
 */
struct mtk_tx_ring {
	struct mtk_tx_dma *dma;
	struct mtk_tx_buf *buf;
	dma_addr_t phys;
	struct mtk_tx_dma *next_free;
	struct mtk_tx_dma *last_free;
	u32 last_free_ptr;
	u16 thresh;
	atomic_t free_count;
	int dma_size;
	struct mtk_tx_dma *dma_pdma;	/* For MT7628/88 PDMA handling */
	dma_addr_t phys_pdma;
	int cpu_idx;
};

/* PDMA rx ring mode */
enum mtk_rx_flags {
	MTK_RX_FLAGS_NORMAL = 0,
	MTK_RX_FLAGS_HWLRO,
	MTK_RX_FLAGS_QDMA,
};

/* struct mtk_rx_ring -	This struct holds info describing a RX ring
 * @dma:		The descriptor ring
 * @data:		The memory pointed at by the ring
 * @phys:		The physical addr of rx_buf
 * @frag_size:		How big can each fragment be
 * @buf_size:		The size of each packet buffer
 * @calc_idx:		The current head of ring
 * @ring_no:		The index of ring
 */
struct mtk_rx_ring {
	struct mtk_rx_dma *dma;
	u8 **data;
	dma_addr_t phys;
	u16 frag_size;
	u16 buf_size;
	u16 dma_size;
	bool calc_idx_update;
	u16 calc_idx;
	u32 crx_idx_reg;
	u32 ring_no;
};

/* struct mtk_napi -	This is the structure holding NAPI-related information,
 *			and a mtk_napi struct is binding to one interrupt group
 * @napi:		The NAPI struct
 * @rx_ring:		Pointer to the memory holding info about the RX ring
 * @irq_grp_idx:	The index indicates which interrupt group that this
 *			mtk_napi is binding to
 */
struct mtk_napi {
	struct napi_struct	napi;
	struct mtk_eth		*eth;
	struct mtk_rx_ring	*rx_ring;
	u32			irq_grp_no;
};

enum mkt_eth_capabilities {
	MTK_RGMII_BIT = 0,
	MTK_TRGMII_BIT,
	MTK_SGMII_BIT,
	MTK_ESW_BIT,
	MTK_GEPHY_BIT,
	MTK_MUX_BIT,
	MTK_INFRA_BIT,
	MTK_SHARED_SGMII_BIT,
	MTK_HWLRO_BIT,
	MTK_RSS_BIT,
	MTK_SHARED_INT_BIT,
	MTK_TRGMII_MT7621_CLK_BIT,
	MTK_QDMA_BIT,
	MTK_NETSYS_V2_BIT,
	MTK_SOC_MT7628_BIT,
	MTK_RSTCTRL_PPE1_BIT,
	MTK_U3_COPHY_V2_BIT,

	/* MUX BITS*/
	MTK_ETH_MUX_GDM1_TO_GMAC1_ESW_BIT,
	MTK_ETH_MUX_GMAC2_GMAC0_TO_GEPHY_BIT,
	MTK_ETH_MUX_U3_GMAC2_TO_QPHY_BIT,
	MTK_ETH_MUX_GMAC1_GMAC2_TO_SGMII_RGMII_BIT,
	MTK_ETH_MUX_GMAC12_TO_GEPHY_SGMII_BIT,

	/* PATH BITS */
	MTK_ETH_PATH_GMAC1_RGMII_BIT,
	MTK_ETH_PATH_GMAC1_TRGMII_BIT,
	MTK_ETH_PATH_GMAC1_SGMII_BIT,
	MTK_ETH_PATH_GMAC2_RGMII_BIT,
	MTK_ETH_PATH_GMAC2_SGMII_BIT,
	MTK_ETH_PATH_GMAC2_GEPHY_BIT,
	MTK_ETH_PATH_GDM1_ESW_BIT,
};

/* Supported hardware group on SoCs */
#define MTK_RGMII		BIT(MTK_RGMII_BIT)
#define MTK_TRGMII		BIT(MTK_TRGMII_BIT)
#define MTK_SGMII		BIT(MTK_SGMII_BIT)
#define MTK_ESW			BIT(MTK_ESW_BIT)
#define MTK_GEPHY		BIT(MTK_GEPHY_BIT)
#define MTK_MUX			BIT(MTK_MUX_BIT)
#define MTK_INFRA		BIT(MTK_INFRA_BIT)
#define MTK_SHARED_SGMII	BIT(MTK_SHARED_SGMII_BIT)
#define MTK_HWLRO		BIT(MTK_HWLRO_BIT)
#define MTK_RSS			BIT(MTK_RSS_BIT)
#define MTK_SHARED_INT		BIT(MTK_SHARED_INT_BIT)
#define MTK_TRGMII_MT7621_CLK	BIT(MTK_TRGMII_MT7621_CLK_BIT)
#define MTK_QDMA		BIT(MTK_QDMA_BIT)
#define MTK_NETSYS_V2		BIT(MTK_NETSYS_V2_BIT)
#define MTK_SOC_MT7628		BIT(MTK_SOC_MT7628_BIT)
#define MTK_RSTCTRL_PPE1	BIT(MTK_RSTCTRL_PPE1_BIT)
#define MTK_U3_COPHY_V2		BIT(MTK_U3_COPHY_V2_BIT)

#define MTK_ETH_MUX_GDM1_TO_GMAC1_ESW		\
	BIT(MTK_ETH_MUX_GDM1_TO_GMAC1_ESW_BIT)
#define MTK_ETH_MUX_GMAC2_GMAC0_TO_GEPHY	\
	BIT(MTK_ETH_MUX_GMAC2_GMAC0_TO_GEPHY_BIT)
#define MTK_ETH_MUX_U3_GMAC2_TO_QPHY		\
	BIT(MTK_ETH_MUX_U3_GMAC2_TO_QPHY_BIT)
#define MTK_ETH_MUX_GMAC1_GMAC2_TO_SGMII_RGMII	\
	BIT(MTK_ETH_MUX_GMAC1_GMAC2_TO_SGMII_RGMII_BIT)
#define MTK_ETH_MUX_GMAC12_TO_GEPHY_SGMII	\
	BIT(MTK_ETH_MUX_GMAC12_TO_GEPHY_SGMII_BIT)

/* Supported path present on SoCs */
#define MTK_ETH_PATH_GMAC1_RGMII	BIT(MTK_ETH_PATH_GMAC1_RGMII_BIT)
#define MTK_ETH_PATH_GMAC1_TRGMII	BIT(MTK_ETH_PATH_GMAC1_TRGMII_BIT)
#define MTK_ETH_PATH_GMAC1_SGMII	BIT(MTK_ETH_PATH_GMAC1_SGMII_BIT)
#define MTK_ETH_PATH_GMAC2_RGMII	BIT(MTK_ETH_PATH_GMAC2_RGMII_BIT)
#define MTK_ETH_PATH_GMAC2_SGMII	BIT(MTK_ETH_PATH_GMAC2_SGMII_BIT)
#define MTK_ETH_PATH_GMAC2_GEPHY	BIT(MTK_ETH_PATH_GMAC2_GEPHY_BIT)
#define MTK_ETH_PATH_GDM1_ESW		BIT(MTK_ETH_PATH_GDM1_ESW_BIT)

#define MTK_GMAC1_RGMII		(MTK_ETH_PATH_GMAC1_RGMII | MTK_RGMII)
#define MTK_GMAC1_TRGMII	(MTK_ETH_PATH_GMAC1_TRGMII | MTK_TRGMII)
#define MTK_GMAC1_SGMII		(MTK_ETH_PATH_GMAC1_SGMII | MTK_SGMII)
#define MTK_GMAC2_RGMII		(MTK_ETH_PATH_GMAC2_RGMII | MTK_RGMII)
#define MTK_GMAC2_SGMII		(MTK_ETH_PATH_GMAC2_SGMII | MTK_SGMII)
#define MTK_GMAC2_GEPHY		(MTK_ETH_PATH_GMAC2_GEPHY | MTK_GEPHY)
#define MTK_GDM1_ESW		(MTK_ETH_PATH_GDM1_ESW | MTK_ESW)

/* MUXes present on SoCs */
/* 0: GDM1 -> GMAC1, 1: GDM1 -> ESW */
#define MTK_MUX_GDM1_TO_GMAC1_ESW (MTK_ETH_MUX_GDM1_TO_GMAC1_ESW | MTK_MUX)

/* 0: GMAC2 -> GEPHY, 1: GMAC0 -> GePHY */
#define MTK_MUX_GMAC2_GMAC0_TO_GEPHY    \
	(MTK_ETH_MUX_GMAC2_GMAC0_TO_GEPHY | MTK_MUX | MTK_INFRA)

/* 0: U3 -> QPHY, 1: GMAC2 -> QPHY */
#define MTK_MUX_U3_GMAC2_TO_QPHY        \
	(MTK_ETH_MUX_U3_GMAC2_TO_QPHY | MTK_MUX | MTK_INFRA)

/* 2: GMAC1 -> SGMII, 3: GMAC2 -> SGMII */
#define MTK_MUX_GMAC1_GMAC2_TO_SGMII_RGMII      \
	(MTK_ETH_MUX_GMAC1_GMAC2_TO_SGMII_RGMII | MTK_MUX | \
	MTK_SHARED_SGMII)

/* 0: GMACx -> GEPHY, 1: GMACx -> SGMII where x is 1 or 2 */
#define MTK_MUX_GMAC12_TO_GEPHY_SGMII   \
	(MTK_ETH_MUX_GMAC12_TO_GEPHY_SGMII | MTK_MUX)

#define MTK_HAS_CAPS(caps, _x)		(((caps) & (_x)) == (_x))

#define MT7621_CAPS  (MTK_GMAC1_RGMII | MTK_GMAC1_TRGMII | \
		      MTK_GMAC2_RGMII | MTK_SHARED_INT | \
		      MTK_TRGMII_MT7621_CLK | MTK_QDMA)

#define MT7622_CAPS  (MTK_GMAC1_RGMII | MTK_GMAC1_SGMII | MTK_GMAC2_RGMII | \
		      MTK_GMAC2_SGMII | MTK_GDM1_ESW | \
		      MTK_MUX_GDM1_TO_GMAC1_ESW | \
		      MTK_MUX_GMAC1_GMAC2_TO_SGMII_RGMII | MTK_QDMA)

#define MT7623_CAPS  (MTK_GMAC1_RGMII | MTK_GMAC1_TRGMII | MTK_GMAC2_RGMII | \
		      MTK_QDMA)

#define MT7628_CAPS  (MTK_SHARED_INT | MTK_SOC_MT7628)

#define MT7629_CAPS  (MTK_GMAC1_SGMII | MTK_GMAC2_SGMII | MTK_GMAC2_GEPHY | \
		      MTK_GDM1_ESW | MTK_MUX_GDM1_TO_GMAC1_ESW | \
		      MTK_MUX_GMAC2_GMAC0_TO_GEPHY | \
		      MTK_MUX_U3_GMAC2_TO_QPHY | \
		      MTK_MUX_GMAC12_TO_GEPHY_SGMII | MTK_QDMA)

#define MT7986_CAPS   (MTK_GMAC1_SGMII | MTK_GMAC2_SGMII | \
                       MTK_MUX_GMAC12_TO_GEPHY_SGMII | MTK_QDMA | \
                       MTK_NETSYS_V2)

#define MT7981_CAPS   (MTK_GMAC1_SGMII | MTK_GMAC2_SGMII | MTK_GMAC2_GEPHY | \
			MTK_MUX_GMAC12_TO_GEPHY_SGMII | MTK_QDMA | \
			MTK_MUX_U3_GMAC2_TO_QPHY | MTK_U3_COPHY_V2 | \
			MTK_NETSYS_V2)

/* struct mtk_eth_data -	This is the structure holding all differences
 *				among various plaforms
 * @ana_rgc3:                   The offset for register ANA_RGC3 related to
 *				sgmiisys syscon
 * @caps			Flags shown the extra capability for the SoC
 * @hw_features			Flags shown HW features
 * @required_clks		Flags shown the bitmap for required clocks on
 *				the target SoC
 * @required_pctl		A bool value to show whether the SoC requires
 *				the extra setup for those pins used by GMAC.
 */
struct mtk_soc_data {
	u32             ana_rgc3;
	u32		caps;
	u32		required_clks;
	bool		required_pctl;
	netdev_features_t hw_features;
	bool		has_sram;
};

/* currently no SoC has more than 2 macs */
#define MTK_MAX_DEVS			2

#define MTK_SGMII_PHYSPEED_AN          BIT(31)
#define MTK_SGMII_PHYSPEED_MASK        GENMASK(2, 0)
#define MTK_SGMII_PHYSPEED_1000        BIT(0)
#define MTK_SGMII_PHYSPEED_2500        BIT(1)
#define MTK_SGMII_PN_SWAP	       BIT(16)
#define MTK_HAS_FLAGS(flags, _x)       (((flags) & (_x)) == (_x))

/* struct mtk_sgmii -  This is the structure holding sgmii regmap and its
 *                     characteristics
 * @regmap:            The register map pointing at the range used to setup
 *                     SGMII modes
 * @flags:             The enum refers to which mode the sgmii wants to run on
 * @ana_rgc3:          The offset refers to register ANA_RGC3 related to regmap
 */

struct mtk_sgmii {
	struct regmap   *regmap[MTK_MAX_DEVS];
	u32             flags[MTK_MAX_DEVS];
	u32             ana_rgc3;
};


/* struct mtk_reset_event - This is the structure holding statistics counters
 *			for reset events
 * @count:		The counter is used to record the number of events
 */
struct mtk_reset_event {
	u32 count[32];
};

/* struct mtk_eth -	This is the main datasructure for holding the state
 *			of the driver
 * @dev:		The device pointer
 * @base:		The mapped register i/o base
 * @page_lock:		Make sure that register operations are atomic
 * @tx_irq__lock:	Make sure that IRQ register operations are atomic
 * @rx_irq__lock:	Make sure that IRQ register operations are atomic
 * @dummy_dev:		we run 2 netdevs on 1 physical DMA ring and need a
 *			dummy for NAPI to work
 * @netdev:		The netdev instances
 * @mac:		Each netdev is linked to a physical MAC
 * @irq:		The IRQ that we are using
 * @msg_enable:		Ethtool msg level
 * @ethsys:		The register map pointing at the range used to setup
 *			MII modes
 * @infra:              The register map pointing at the range used to setup
 *                      SGMII and GePHY path
 * @pctl:		The register map pointing at the range used to setup
 *			GMAC port drive/slew values
 * @dma_refcnt:		track how many netdevs are using the DMA engine
 * @tx_ring:		Pointer to the memory holding info about the TX ring
 * @rx_ring:		Pointer to the memory holding info about the RX ring
 * @rx_ring_qdma:	Pointer to the memory holding info about the QDMA RX ring
 * @tx_napi:		The TX NAPI struct
 * @rx_napi:		The RX NAPI struct
 * @scratch_ring:	Newer SoCs need memory for a second HW managed TX ring
 * @phy_scratch_ring:	physical address of scratch_ring
 * @scratch_head:	The scratch memory that scratch_ring points to.
 * @clks:		clock array for all clocks required
 * @mii_bus:		If there is a bus we need to create an instance for it
 * @pending_work:	The workqueue used to reset the dma ring
 * @state:		Initialization and runtime state of the device
 * @soc:		Holding specific data among vaious SoCs
 */

struct mtk_eth {
	struct device			*dev;
	void __iomem			*base;
	spinlock_t			page_lock;
	spinlock_t			tx_irq_lock;
	spinlock_t			rx_irq_lock;
	struct net_device		dummy_dev;
	struct net_device		*netdev[MTK_MAX_DEVS];
	struct mtk_mac			*mac[MTK_MAX_DEVS];
	int				irq[MTK_MAX_IRQ_NUM];
	u32				msg_enable;
	unsigned long			sysclk;
	struct regmap			*ethsys;
	struct regmap                   *infra;
	struct mtk_sgmii                *sgmii;
	struct regmap			*pctl;
	bool				hwlro;
	refcount_t			dma_refcnt;
	struct mtk_tx_ring		tx_ring;
	struct mtk_rx_ring		rx_ring[MTK_MAX_RX_RING_NUM];
	struct mtk_rx_ring		rx_ring_qdma;
	struct napi_struct		tx_napi;
	struct mtk_napi			rx_napi[MTK_RX_NAPI_NUM];
	struct mtk_tx_dma		*scratch_ring;
	struct mtk_reset_event		reset_event;
	dma_addr_t			phy_scratch_ring;
	void				*scratch_head;
	struct clk			*clks[MTK_CLK_MAX];

	struct mii_bus			*mii_bus;
	struct work_struct		pending_work;
	unsigned long			state;

	const struct mtk_soc_data	*soc;

	u32				tx_int_mask_reg;
	u32				tx_int_status_reg;
	u32				rx_dma_l4_valid;
	int				ip_align;
	spinlock_t			syscfg0_lock;
	struct timer_list		mtk_dma_monitor_timer;
};

/* struct mtk_mac -	the structure that holds the info about the MACs of the
 *			SoC
 * @id:			The number of the MAC
 * @interface:		Interface mode kept for detecting change in hw settings
 * @of_node:		Our devicetree node
 * @hw:			Backpointer to our main datastruture
 * @hw_stats:		Packet statistics counter
 */
struct mtk_mac {
	unsigned int			id;
	phy_interface_t			interface;
	unsigned int			mode;
	int				speed;
	struct device_node		*of_node;
	struct phylink			*phylink;
	struct phylink_config		phylink_config;
	struct mtk_eth			*hw;
	struct mtk_hw_stats		*hw_stats;
	__be32				hwlro_ip[MTK_MAX_LRO_IP_CNT];
	int				hwlro_ip_cnt;
};

/* the struct describing the SoC. these are declared in the soc_xyz.c files */
extern const struct of_device_id of_mtk_match[];
extern u32 mtk_hwlro_stats_ebl;

/* read the hardware status register */
void mtk_stats_update_mac(struct mtk_mac *mac);

void mtk_w32(struct mtk_eth *eth, u32 val, unsigned reg);
u32 mtk_r32(struct mtk_eth *eth, unsigned reg);
u32 mtk_m32(struct mtk_eth *eth, u32 mask, u32 set, unsigned reg);

int mtk_sgmii_init(struct mtk_sgmii *ss, struct device_node *np,
		   u32 ana_rgc3);
int mtk_sgmii_setup_mode_an(struct mtk_sgmii *ss, unsigned int id);
int mtk_sgmii_setup_mode_force(struct mtk_sgmii *ss, unsigned int id,
			       const struct phylink_link_state *state);
void mtk_sgmii_restart_an(struct mtk_eth *eth, int mac_id);

int mtk_gmac_sgmii_path_setup(struct mtk_eth *eth, int mac_id);
int mtk_gmac_gephy_path_setup(struct mtk_eth *eth, int mac_id);
int mtk_gmac_rgmii_path_setup(struct mtk_eth *eth, int mac_id);
void mtk_gdm_config(struct mtk_eth *eth, u32 config);
void ethsys_reset(struct mtk_eth *eth, u32 reset_bits);

#endif /* MTK_ETH_H */
