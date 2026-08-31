/* SPDX-License-Identifier: GPL-2.0-only */

#ifndef __RTL960X_GMAC_H
#define __RTL960X_GMAC_H

#include <linux/bitfield.h>
#include <linux/bits.h>
#include <linux/types.h>

/* GMAC registers (offsets from the per-GMAC base) */
#define GMAC_IDR0		0x00
#define GMAC_IDR4		0x04
#define GMAC_MAR0		0x08
#define GMAC_MAR4		0x0c

/* 16-bit statistics counters */
#define GMAC_TX_OK_CNT		0x10
#define GMAC_RX_OK_CNT		0x12
#define GMAC_TX_ERR_CNT		0x14
#define GMAC_RX_ERR_CNT		0x16
#define GMAC_MISS_PKT_CNT	0x18
#define GMAC_FAE_CNT		0x1a
#define GMAC_TX_1_COL_CNT	0x1c
#define GMAC_TX_MUL_COL_CNT	0x1e
#define GMAC_RX_OK_PHY_CNT	0x20
#define GMAC_RX_OK_BC_CNT	0x22
#define GMAC_RX_OK_MC_CNT	0x24
#define GMAC_TX_ABORT_CNT	0x26
#define GMAC_TX_UNDERRUN_CNT	0x28
#define GMAC_RDU_MISS_PKT_CNT	0x2a

#define GMAC_TRSR		0x34

#define GMAC_CMD		0x3b
#define CMD_RXJUMBO		BIT(3)
#define CMD_RX_VLAN_DETAG	BIT(2)
#define CMD_RXCHKSUM		BIT(1)

#define GMAC_IMR		0x3c
#define GMAC_ISR		0x3e
#define INTR_RDU6		BIT(15)
#define INTR_RDU5		BIT(14)
#define INTR_RDU4		BIT(13)
#define INTR_RDU3		BIT(12)
#define INTR_RDU2		BIT(11)
#define INTR_SW_INT		BIT(10)
#define INTR_TDU		BIT(9)
#define INTR_LINK_CHG		BIT(8)
#define INTR_TER		BIT(7)
#define INTR_TOK		BIT(6)
#define INTR_RDU		BIT(5)
#define INTR_RER_OVF		BIT(4)
#define INTR_RER_RUNT		BIT(2)
#define INTR_RX_OK		BIT(0)
#define INTR_RX_ALL		(INTR_SW_INT | INTR_RX_OK | INTR_RER_RUNT | \
				 INTR_RER_OVF | INTR_RDU)
/* Interrupts that drive NAPI: RX events plus TX completion (TOK). */
#define INTR_NAPI		(INTR_RX_ALL | INTR_TOK)

#define GMAC_TCR		0x40
#define TCR_IFG			GENMASK(12, 10)
#define TCR_IFG_SHORT		3
#define TCR_PADDING_DISABLE	BIT(0)
#define TCR_CONFIG		FIELD_PREP(TCR_IFG, TCR_IFG_SHORT)

#define GMAC_RCR		0x44
#define RCR_ACCEPT_ERR		BIT(5)
#define RCR_ACCEPT_RUNT		BIT(4)
#define RCR_ACCEPT_BROADCAST	BIT(3)
#define RCR_ACCEPT_MULTICAST	BIT(2)
#define RCR_ACCEPT_MYPHYS	BIT(1)
#define RCR_ACCEPT_ALLPHYS	BIT(0)

#define GMAC_CPUTAGCR		0x48
#define CPUTAGCR_EN_RX		BIT(31)
#define CPUTAGCR_TSIZE		GENMASK(30, 27)
#define CPUTAGCR_RSIZE_H	GENMASK(26, 25)
#define CPUTAGCR_SWITCH		GENMASK(21, 18)
#define CPUTAGCR_RSIZE_L	GENMASK(17, 16)
#define CPUTAGCR_PROTOCOL_MASK	GENMASK(15, 8)
#define CPUTAGCR_PROTOCOL_VAL	GENMASK(7, 0)
#define CPUTAGCR_SIZE_8B	2
#define CPUTAGCR_FMT_APOLLOPRO	8
#define CPUTAGCR_PROTO_MASK_8370	0xff
#define CPUTAGCR_PROTO_8370	0x04

#define GMAC_CFG		0x4c
#define CFG_RFIFO_SIZE		GENMASK(29, 28)
#define CFG_RFIFO_SIZE_2KB	2

#define GMAC_CPUTAG1CR		0x50
#define CPUTAG1CR_SID		BIT(14)

#define GMAC_MSR		0x58
#define MSR_FORCE_TRXFCE	BIT(31)
#define MSR_RXFCE		BIT(30)
#define MSR_TXFCE		BIT(29)
#define MSR_SPEED_1000		BIT(28)
#define MSR_SPEED_10		BIT(27)
#define MSR_LINKB		BIT(26)
#define MSR_TXPF		BIT(25)
#define MSR_RXPF		BIT(24)
#define MSR_SEL_RGMII		BIT(23)
#define MSR_FULLDUPREG		BIT(22)
#define MSR_NWCOMPLETE		BIT(21)
#define MSR_SEL_MII		BIT(20)
#define MSR_FORCEDFULLDUP	BIT(19)
#define MSR_FORCELINK		BIT(18)
#define MSR_FORCE_SPD		GENMASK(17, 16)
#define MSR_SEL_PHYIF_0		BIT(15)
#define MSR_PHY_MODE		BIT(13)
#define MSR_RGMII_RX_STS	BIT(12)
#define MSR_RGMII_TX_STS	BIT(11)
#define MSR_FORCE_SPD_MODE	BIT(10)

#define GMAC_IMR0		0xd0
#define IMR0_ROK6		BIT(5)
#define IMR0_ROK5		BIT(4)
#define IMR0_ROK4		BIT(3)
#define IMR0_ROK3		BIT(2)
#define IMR0_ROK2		BIT(1)
#define IMR0_ROK1		BIT(0)

#define GMAC_ISR1		0xd8

#define GMAC_TXFDP1		0x1300
#define GMAC_TXCDO1		0x1304

#define GMAC_RX_RING_ROUTE	0x1370
#define RX_RING_ROUTE_PRI_7	GENMASK(30, 28)
#define RX_RING_ROUTE_PRI_6	GENMASK(26, 24)
#define RX_RING_ROUTE_PRI_5	GENMASK(22, 20)
#define RX_RING_ROUTE_PRI_4	GENMASK(18, 16)
#define RX_RING_ROUTE_PRI_3	GENMASK(14, 12)
#define RX_RING_ROUTE_PRI_2	GENMASK(10, 8)
#define RX_RING_ROUTE_PRI_1	GENMASK(6, 4)
#define RX_RING_ROUTE_PRI_0	GENMASK(2, 0)

#define GMAC_RXFDP2		0x1390
#define GMAC_RX_DESC_OFFSET_AND_SIZE2	0x1394
#define GMAC_RX_CPU_DESC_NUM2	0x1398
#define GMAC_RX_DESC_THRESHOLD2	0x139c

#define GMAC_RXFDP		0x13f0
#define GMAC_RXCDO		0x13f4
#define GMAC_RXRINGSIZE		0x13f6
#define GMAC_RXCPU_DES_NUM	0x1430
#define GMAC_RX_FC_ASSERT_THRES	0x1431
#define GMAC_RX_FC_DEASSERT_THRES	0x1432
#define RX_FC_ASSERT_THRES_VAL	16
#define RX_FC_DEASSERT_THRES_VAL	48

#define GMAC_IO_CMD		0x1434
#define IO_CMD_MAX_DMA_SEL_0	BIT(31)
#define IO_CMD_MAX_DMA_128B	IO_CMD_MAX_DMA_SEL_0
#define IO_CMD_SHORT_DES_FMT	BIT(30)
#define IO_CMD_MAX_DMA_SEL_1	BIT(29)
#define IO_CMD_EN_EARLY_TX	BIT(28)
#define IO_CMD_TX_PKT_TMR	GENMASK(27, 24)
#define IO_CMD_TX_NO_TIMER	0
#define IO_CMD_TX_PKT_TMR_7TU	7
#define IO_CMD_TX_INT_TRIG_H	BIT(23)
#define IO_CMD_RX_PKT_TMR_H	BIT(22)
#define IO_CMD_RX_INT_TRIG_H	BIT(21)
#define IO_CMD_TX_FIFO_THRESH	GENMASK(20, 19)
#define IO_CMD_TX_FIFO_THRESH_256B	1
#define IO_CMD_TX_FIFO_THRESH_1024B	3
#define IO_CMD_TX_INT_TRIG_L	GENMASK(18, 16)
#define IO_CMD_TX_INT_28PKTS	7
#define IO_CMD_RX_PKT_TMR_L	GENMASK(15, 13)
#define IO_CMD_RX_PKT_TMR_4TU	1
#define IO_CMD_RX_FIFO_THRESH	GENMASK(12, 11)
#define IO_CMD_RX_FIFO_64B	2
#define IO_CMD_RX_INT_TRIG_L	GENMASK(10, 8)
#define IO_CMD_RX_INT_4PKTS	1
#define IO_CMD_RX_INT_12PKTS	3
#define IO_CMD_INI_TMR_SEL	GENMASK(7, 6)
#define IO_CMD_INI_TMR_SEL_3	3
#define IO_CMD_RX_ENABLE	BIT(5)
#define IO_CMD_TX_ENABLE	BIT(4)
#define IO_CMD_TX_POLL4		BIT(3)
#define IO_CMD_TX_POLL3		BIT(2)
#define IO_CMD_TX_POLL2		BIT(1)
#define IO_CMD_TX_POLL		BIT(0)

#define GMAC_IO_CMD1		0x1438
#define IO_CMD1_DESC_FMT_EXTRA	GENMASK(30, 28)
#define IO_CMD1_RXOKINT_MSK_128B	BIT(26)
#define IO_CMD1_EN_RX_MRING	BIT(25)
#define IO_CMD1_EN_1GB		BIT(24)
#define IO_CMD1_RX_RING6	BIT(21)
#define IO_CMD1_RX_RING5	BIT(20)
#define IO_CMD1_RX_RING4	BIT(19)
#define IO_CMD1_RX_RING3	BIT(18)
#define IO_CMD1_RX_RING2	BIT(17)
#define IO_CMD1_RX_RING1	BIT(16)
#define IO_CMD1_TX_HL_PRI_SEL	GENMASK(15, 14)
#define IO_CMD1_TX_FN5		BIT(8)
#define IO_CMD1_TXQ5_H		BIT(4)
#define IO_CMD1_TXQ4_H		BIT(3)
#define IO_CMD1_TXQ3_H		BIT(2)
#define IO_CMD1_TXQ2_H		BIT(1)
#define IO_CMD1_TXQ1_H		BIT(0)
#define IO_CMD1_DESC_FMT_APOLLO	3

/* Descriptor fields */
#define DESC_OWN		BIT(31)
#define DESC_EOR		BIT(30)
#define DESC_FS			BIT(29)
#define DESC_LS			BIT(28)
#define DESC_LEN_MASK		GENMASK(11, 0)
#define TX_DESC_CRC		BIT(23)

/* CPU-tag sideband fields in descriptor opts2/opts3 */
#define RX_OPTS3_SRC_PORT	GENMASK(19, 16)
#define TX_OPTS2_CPUTAG		BIT(31)
#define TX_OPTS2_PORTMASK	GENMASK(26, 16)
#define TX_OPTS3_KEEP		BIT(23)
#define TX_OPTS3_DISLRN		BIT(21)
#define TX_OPTS3_L34_KEEP	BIT(17)

#define RTL960X_CPU_PORT	9

/* RX descriptors are four words; TX descriptors include an opts4 word. */
struct rtl960x_rx_desc {
	u32 opts1;	/* own/eor/fs/ls + length */
	u32 addr;	/* DMA buffer address */
	u32 opts2;	/* CPU tag: trap reason */
	u32 opts3;	/* CPU tag: source port */
};

struct rtl960x_tx_desc {
	u32 opts1;	/* own/eor/fs/ls + length */
	u32 addr;	/* DMA buffer address */
	u32 opts2;	/* CPU tag: enable + destination port mask */
	u32 opts3;	/* CPU tag: flags */
	u32 opts4;
};

#endif /* __RTL960X_GMAC_H */
