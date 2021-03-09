/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	wdma.h
*/

#ifndef _WDMA_H
#define _WDMA_H

#include "woe_basic.h"

#define WDMA_IRQ_NUM 3
#define WDMA_TX_BM_RING_SIZE 512
#define WHNAT_WDMA_PORT 3

struct wdma_rx_ring_ctrl {
	unsigned int ring_num;
	unsigned int ring_len;
	unsigned int rxd_len;
	struct whnat_dma_buf *desc;
	struct whnat_ring *ring;
};

struct wdma_rx_ctrl {
	struct wdma_rx_ring_ctrl rx_ring_ctrl;
};

struct wdma_res_ctrl {
	struct wdma_rx_ctrl rx_ctrl;
};

struct wdma_entry {
	struct platform_device *pdev;
	unsigned long base_addr;
	unsigned int irq[WDMA_IRQ_NUM];
	struct wdma_res_ctrl res_ctrl;
	void *proc;
	void *whnat;
};


#ifdef RT_BIG_ENDIAN
struct WDMA_RXD {
	/* Word 0 */
	unsigned int	sdp0;
	/* Word 1 */
	unsigned int	ddone:1;
	unsigned int	ls:1;
	unsigned int	sdl0:14;
	unsigned int	res1:16;
	/* Word 2 */
	unsigned int	res2;
	/* Word 3*/
	unsigned int	rx_info;
} __packed;
#else
struct WDMA_RXD {
	/* Word	0 */
	unsigned int	sdp0;
	/* Word	1 */
	unsigned int	res1:16;
	unsigned int	sdl0:14;
	unsigned int	ls:1;
	unsigned int	ddone:1;
	/*Word2 */
	unsigned int	res2;
	/*Word3*/
	unsigned int	rx_info;
} __packed;
#endif


enum {
	WDMA_PROC_BASIC = 0,
	WDMA_PROC_RX_CELL = 1,
	WDMA_PROC_END
};



int wdma_init(struct platform_device *pdev, unsigned char idx, struct wdma_entry *wdma);
int wdma_exit(struct platform_device *pdev, struct wdma_entry *wdma);

int	wdma_ring_init(struct wdma_entry *wdma);
void wdma_ring_exit(struct wdma_entry *wdma);
void wdma_dump_entry(struct wdma_entry *wdma);
void wdma_proc_handle(struct wdma_entry *wdma, char choice, char *value);
void dump_wdma_value(struct wdma_entry *wdma, char *name, unsigned int addr);


/*CR Definition*/

#define WDMA_OFFSET 0x400
#define WDMA0_OFST0 0x2a042a20
#define WDMA0_OFST1 0x29002800
#define WDMA1_OFST0 0x2e042e20
#define WDMA1_OFST1 0x2d002c00

#define WDMA_BIT(shift) shift
/*
 * WDMA0 Base: 0x1B102800
 * WDMA1 Base: 0x1B102C00
 */
/*WDMA TX Ring #0 Base Pointer*/
#define WDMA_TX_BASE_PTR_0							0x000
#define	WDMA_TX_BASE_PTR_0_TX_BASE_PTR				WDMA_BIT(0)
/*WDMA TX Ring #0 Maximum Count*/
#define WDMA_TX_MAX_CNT_0							0x004
#define WDMA_TX_MAX_CNT_0_TX_MAX_CNT				WDMA_BIT(0)
/*WDMA TX Ring #0 CPU pointer*/
#define WDMA_TX_CTX_IDX_0							0x008
#define	WDMA_TX_CTX_IDX_0_TX_CTX_IDX				WDMA_BIT(0)
/*WDMA TX Ring #0 DMA poitner*/
#define WDMA_TX_DTX_IDX_0							0x00c
#define	WDMA_TX_DTX_IDX_0_TX_DTX_IDX_MIRO			WDMA_BIT(0)
/*WDMA TX Ring #1 Base Pointer*/
#define WDMA_TX_BASE_PTR_1							0x010
#define	WDMA_TX_BASE_PTR_1_TX_BASE_PTR				WDMA_BIT(0)
/*WDMA TX Ring #1 Maximum Count*/
#define WDMA_TX_MAX_CNT_1							0x014
#define	WDMA_TX_MAX_CNT_1_TX_MAX_CNT				WDMA_BIT(0)
/*WDMA TX Ring #1 CPU pointer*/
#define WDMA_TX_CTX_IDX_1							0x018
#define	WDMA_TX_CTX_IDX_1_TX_CTX_IDX				WDMA_BIT(0)
/*WDMA TX Ring #1 DMA poitner*/
#define WDMA_TX_DTX_IDX_1							0x01c
#define	WDMA_TX_DTX_IDX_1_TX_DTX_IDX_MIRO			WDMA_BIT(0)
/*WDMA RX Ring #0 Base Pointer*/
#define WDMA_RX_BASE_PTR_0							0x100
#define	WDMA_RX_BASE_PTR_0_RX_BASE_PTR				WDMA_BIT(0)
/*WDMA RX Ring #0 Maximum Count*/
#define WDMA_RX_MAX_CNT_0							0x104
#define	WDMA_RX_MAX_CNT_0_RX_MAX_CNT				WDMA_BIT(0)
/*WDMA RX Ring #0 CPU pointer*/
#define WDMA_RX_CRX_IDX_0							0x108
#define	WDMA_RX_CRX_IDX_0_RX_CRX_IDX				WDMA_BIT(0)
/*WDMA RX Ring #0 DMA poitner*/
#define WDMA_RX_DRX_IDX_0						0x10c
#define	WDMA_RX_DRX_IDX_0_RX_DRX_IDX_MIRO       WDMA_BIT(0)
/*WDMA RX Ring #1 Base Pointer*/
#define WDMA_RX_BASE_PTR_1						0x110
#define	WDMA_RX_BASE_PTR_1_RX_BASE_PTR			WDMA_BIT(0)
/*WDMA RX Ring #1 Maximum Count*/
#define WDMA_RX_MAX_CNT_1						0x114
#define	WDMA_RX_MAX_CNT_1_RX_MAX_CNT			WDMA_BIT(0)
/*WDMA RX Ring #1 CPU pointer*/
#define WDMA_RX_CRX_IDX_1						0x118
#define	WDMA_RX_CRX_IDX_1_RX_CRX_IDX			WDMA_BIT(0)
/*WDMA RX Ring #1 DMA poitner*/
#define WDMA_RX_DRX_IDX_1						0x11c
#define	WDMA_RX_DRX_IDX_1_RX_DRX_IDX_MIRO		WDMA_BIT(0)
/*WDMA Information*/
#define WDMA_INFO								0x200
#define WDMA_INFO_REV							WDMA_BIT(28)
#define WDMA_INFO_INDEX_WIDTH					WDMA_BIT(24)
#define WDMA_INFO_BASE_PTR_WIDTH				WDMA_BIT(16)
#define WDMA_INFO_RX_RING_NUM					WDMA_BIT(8)
#define WDMA_INFO_TX_RING_NUM					WDMA_BIT(0)
/*WDMA Global Configuration*/
#define WDMA_GLO_CFG							0x204
#define	WDMA_GLO_CFG_RX_2B_OFFSET				WDMA_BIT(31)
#define	WDMA_GLO_CFG_CSR_CLKGATE_BYP			WDMA_BIT(30)
#define	WDMA_GLO_CFG_BYTE_SWAP					WDMA_BIT(29)
#define	WDMA_GLO_CFG_RX_INFO1_PRERESERVE		WDMA_BIT(28)
#define	WDMA_GLO_CFG_RX_INFO2_PRERESERVE		WDMA_BIT(27)
#define	WDMA_GLO_CFG_RX_INFO3_PRERESERVE		WDMA_BIT(26)
#define	WDMA_GLO_CFG_REV0						WDMA_BIT(11)
#define	WDMA_GLO_CFG_MULTI_EN					WDMA_BIT(10)
#define	WDMA_GLO_CFG_EXT_FIFO_EN				WDMA_BIT(9)
#define	WDMA_GLO_CFG_DESC_32B_E					WDMA_BIT(8)
#define	WDMA_GLO_CFG_BIG_ENDIAN					WDMA_BIT(7)
#define	WDMA_GLO_CFG_TX_WB_DDONE				WDMA_BIT(6)
#define	WDMA_GLO_CFG_WDMA_BT_SIZE				WDMA_BIT(4)
#define	WDMA_GLO_CFG_RX_DMA_BUSY				WDMA_BIT(3)
#define	WDMA_GLO_CFG_RX_DMA_EN					WDMA_BIT(2)
#define	WDMA_GLO_CFG_TX_DMA_BUSY				WDMA_BIT(1)
#define	WDMA_GLO_CFG_TX_DMA_EN					WDMA_BIT(0)
/*WDMA Reset Index*/
#define WDMA_RST_IDX							0x208
#define WDMA_RST_IDX_RST_DRX_IDX1				WDMA_BIT(17)
#define WDMA_RST_IDX_RST_DRX_IDX0				WDMA_BIT(16)
#define WDMA_RST_IDX_RST_DTX_IDX3				WDMA_BIT(3)
#define WDMA_RST_IDX_RST_DTX_IDX2				WDMA_BIT(2)
#define WDMA_RST_IDX_RST_DTX_IDX1				WDMA_BIT(1)
#define WDMA_RST_IDX_RST_DTX_IDX0				WDMA_BIT(0)

/*WDMA Interrupt Status Record*/
#define WDMA_INT_STA_REC						0x220
#define	WDMA_INT_STA_REC_RX_COHERENT			WDMA_BIT(31)
#define	WDMA_INT_STA_REC_RX_DLY_INT				WDMA_BIT(30)
#define	WDMA_INT_STA_REC_TX_COHERENT			WDMA_BIT(29)
#define	WDMA_INT_STA_REC_TX_DLY_INT				WDMA_BIT(28)
#define	WDMA_INT_STA_REC_RX_DONE_INT1			WDMA_BIT(17)
#define	WDMA_INT_STA_REC_RX_DONE_INT0			WDMA_BIT(16)
#define	WDMA_INT_STA_REC_TX_DONE_INT3			WDMA_BIT(3)
#define	WDMA_INT_STA_REC_TX_DONE_INT2			WDMA_BIT(2)
#define	WDMA_INT_STA_REC_TX_DONE_INT1			WDMA_BIT(1)
#define	WDMA_INT_STA_REC_TX_DONE_INT0			WDMA_BIT(0)
/*WDMA Interrupt Mask*/
#define WDMA_INT_MSK							0x228
#define WDMA_INT_MSK_RX_COHERENT				WDMA_BIT(31)
#define WDMA_INT_MSK_RX_DLY_INT					WDMA_BIT(30)
#define WDMA_INT_MSK_TX_COHERENT				WDMA_BIT(29)
#define WDMA_INT_MSK_TX_DLY_INT					WDMA_BIT(28)
#define WDMA_INT_MSK_RX_DONE_INT1				WDMA_BIT(17)
#define WDMA_INT_MSK_RX_DONE_INT0				WDMA_BIT(16)
#define WDMA_INT_MSK_TX_DONE_INT3				WDMA_BIT(3)
#define WDMA_INT_MSK_TX_DONE_INT2				WDMA_BIT(2)
#define WDMA_INT_MSK_TX_DONE_INT1				WDMA_BIT(1)
#define WDMA_INT_MSK_TX_DONE_INT0				WDMA_BIT(0)

#define WDMA_DELAY_INT_CFG						0x20C
#define	WDMA_FREEQ_THRES						0x210
#define	WDMA_INT_STS_GRP0						0x240
#define	WDMA_INT_STS_GRP1						0x244
#define	WDMA_INT_STS_GRP2						0x248
#define	WDMA_INT_GRP1							0x250
#define	WDMA_INT_GRP2							0x254
#define	WDMA_SCH_Q01_CFG						0x280
#define	WDMA_SCH_Q23_CFG						0x284

#endif /*_WDMA_H*/
