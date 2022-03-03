/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	ral_omac_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RAL_OMAC_PCI_H__
#define __RAL_OMAC_PCI_H__


#define RTMP_INT_RX_DLY				(1<<0)		/* bit 0 */
#define RTMP_INT_TX_DLY				(1<<1)		/* bit 1 */
#define RTMP_INT_RX_DONE				(1<<2)		/* bit 2 */
#define RTMP_INT_AC0_DMA_DONE		(1<<3)		/* bit 3 */
#define RTMP_INT_AC1_DMA_DONE		(1<<4)		/* bit 4 */
#define RTMP_INT_AC2_DMA_DONE		(1<<5)		/* bit 5 */
#define RTMP_INT_AC3_DMA_DONE		(1<<6)		/* bit 6 */
#define RTMP_INT_HCCA_DMA_DONE		(1<<7)		/* bit 7 */
#define RTMP_INT_MGMT_DONE			(1<<8)		/* bit 8 */
#define RTMP_INT_MCU_CMD				(1<<9)
#define RTMP_INT_ANY_COHERENT			(1<<10)
#define RTMP_INT_TBTT					(1<<11)
#define RTMP_INT_PRE_TBTT				(1<<12)
#define RTMP_INT_FIFO_STAT				(1<<13)
#define RTMP_INT_AUTO_WAKEUP			(1<<14)
#define RTMP_INT_GPIO_TIMER			(1<<15)
#define RTMP_INT_RX_COHERENT			(1<<16)
#define RTMP_INT_TX_COHERENT			(1<<17)
#define RTMP_INT_TONE_RADAR			(1<<20)		/* bit 20 */

#define RTMP_RxINT				(RTMP_INT_RX_DLY | RTMP_INT_RX_DONE) /* 0x00000005 */	/* Delayed Rx or indivi rx */
#define RTMP_TxDataInt			(RTMP_INT_TX_DLY |\
							 RTMP_INT_AC0_DMA_DONE | \
							 RTMP_INT_AC1_DMA_DONE | \
							 RTMP_INT_AC2_DMA_DONE | \
							 RTMP_INT_AC3_DMA_DONE | \
							 RTMP_INT_HCCA_DMA_DONE) /* 0x000000fa */	/* Delayed Tx or indivi tx */
#define RTMP_TxMgmtInt			(RTMP_INT_TX_DLY | RTMP_INT_MGMT_DONE) /* 0x00000102 */	/* Delayed Tx or indivi tx */
#define RTMP_TxCoherent			(RTMP_INT_TX_COHERENT) /* 0x00020000 */	/* tx coherent */
#define RTMP_RxCoherent			(RTMP_INT_RX_COHERENT) /* 0x00010000 */	/* rx coherent */
#define RTMP_TxRxCoherent			(RTMP_INT_ANY_COHERENT) /* 0x00000400 */	/* tx rx coherent */
#define RTMP_McuCommand			(RTMP_INT_MCU_CMD) /* 0x00000200 */	/* mcu */
#define RTMP_PreTBTTInt			(RTMP_INT_PRE_TBTT) /* 0x00001000 */	/* Pre-TBTT interrupt */
#define RTMP_TBTTInt				(RTMP_INT_TBTT) /* 0x00000800 */		/* TBTT interrupt */
#define RTMP_GPTimeOutInt			(RTMP_INT_GPIO_TIMER) /* 0x00008000 */		/* GPtimeout interrupt */
#define RTMP_AutoWakeupInt		(RTMP_INT_AUTO_WAKEUP) /* 0x00004000 */		/* AutoWakeupInt interrupt */
#define RTMP_FifoStaFullInt			(RTMP_INT_FIFO_STAT) /* 0x00002000 */	/*  fifo statistics full interrupt */
#define RTMP_RadarInt				(RTMP_INT_TONE_RADAR) /* 0x00100000 */

#define RTMP_INT_RX			RTMP_INT_RX_DONE

#define RTMP_INT_AC0_DLY		(RTMP_INT_AC0_DMA_DONE) /*| RTMP_INT_TX_DLY) */
#define RTMP_INT_AC1_DLY		(RTMP_INT_AC1_DMA_DONE) /*| RTMP_INT_TX_DLY) */
#define RTMP_INT_AC2_DLY		(RTMP_INT_AC2_DMA_DONE) /*| RTMP_INT_TX_DLY) */
#define RTMP_INT_AC3_DLY		(RTMP_INT_AC3_DMA_DONE) /*| RTMP_INT_TX_DLY) */
#define RTMP_INT_HCCA_DLY 	(RTMP_INT_HCCA_DMA_DONE) /*| RTMP_INT_TX_DLY) */
#define RTMP_INT_MGMT_DLY	(RTMP_INT_MGMT_DONE)


#ifdef CARRIER_DETECTION_SUPPORT
#define RTMP_DELAYINTMASK	(0x0003FFFB | (RTMP_RadarInt))
#define RTMP_INTMASK			(0x0003FFFB | (RTMP_RadarInt))
#else
#define RTMP_DELAYINTMASK	0x0003FFFB
#define RTMP_INTMASK			0x0003FFFB
#endif /* CARRIER_DETECTION_SUPPORT */


/*
	INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit
	Refer to INT_SOURCE_CSR(0x200)
*/
#ifdef RT_BIG_ENDIAN
typedef	union _RTMP_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 rsv21:11;
		UINT32 RadarINT:1;
		UINT32 rsv18:2;
		UINT32 TxCoherent:1;
		UINT32 RxCoherent:1;
		UINT32 GPTimer:1;
		UINT32 AutoWakeup:1;/*bit14 */
		UINT32 TXFifoStatusInt:1;/*FIFO Statistics is full, sw should read 0x171c */
		UINT32 PreTBTT:1;
		UINT32 TBTTInt:1;
		UINT32 RxTxCoherent:1;
		UINT32 MCUCommandINT:1;
		UINT32 MgmtDmaDone:1;
		UINT32 HccaDmaDone:1;
		UINT32 Ac3DmaDone:1;
		UINT32 Ac2DmaDone:1;
		UINT32 Ac1DmaDone:1;
		UINT32 Ac0DmaDone:1;
		UINT32 RxDone:1;
		UINT32 TxDelayINT:1;	/*delayed interrupt, not interrupt until several int or time limit hit */
		UINT32 RxDelayINT:1; /*dealyed interrupt */
	} field;
	UINT32 word;
}	RTMP_INT_SOURCE_CSR_STRUC;
#else
typedef	union _RTMP_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 RxDelayINT:1;
		UINT32 TxDelayINT:1;
		UINT32 RxDone:1;
		UINT32 Ac0DmaDone:1;/*4 */
		UINT32 Ac1DmaDone:1;
		UINT32 Ac2DmaDone:1;
		UINT32 Ac3DmaDone:1;
		UINT32 HccaDmaDone:1; /* bit7 */
		UINT32 MgmtDmaDone:1;
		UINT32 MCUCommandINT:1;/*bit 9 */
		UINT32 RxTxCoherent:1;
		UINT32 TBTTInt:1;
		UINT32 PreTBTT:1;
		UINT32 TXFifoStatusInt:1;/*FIFO Statistics is full, sw should read 0x171c */
		UINT32 AutoWakeup:1;/*bit14 */
		UINT32 GPTimer:1;
		UINT32 RxCoherent:1;/*bit16 */
		UINT32 TxCoherent:1;
		UINT32 rsv18:2;
		UINT32 RadarINT:1;
		UINT32 rsv21:11;
	} field;
	UINT32 word;
} RTMP_INT_SOURCE_CSR_STRUC;
#endif


/*
	INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF
	Refer to INT_MASK_CSR(0x204)
*/
#ifdef RT_BIG_ENDIAN
typedef	union _INT_MASK_CSR_STRUC {
	struct {
		UINT32       	TxCoherent:1;
		UINT32       	RxCoherent:1;
		UINT32		:9;
		UINT32		RadarINT:1;
		UINT32       	rsv:10;
		UINT32       	MCUCommandINT:1;
		UINT32       	MgmtDmaDone:1;
		UINT32       	HccaDmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac1DmaDone:1;
		UINT32		Ac0DmaDone:1;
		UINT32		RxDone:1;
		UINT32		TxDelay:1;
		UINT32		RXDelay_INT_MSK:1;
	} field;
	UINT32 word;
}INT_MASK_CSR_STRUC;
#else
typedef	union _INT_MASK_CSR_STRUC {
	struct {
		UINT32		RXDelay_INT_MSK:1;
		UINT32		TxDelay:1;
		UINT32		RxDone:1;
		UINT32		Ac0DmaDone:1;
		UINT32       	Ac1DmaDone:1;
		UINT32       	Ac2DmaDone:1;
		UINT32       	Ac3DmaDone:1;
		UINT32       	HccaDmaDone:1;
		UINT32       	MgmtDmaDone:1;
		UINT32       	MCUCommandINT:1;
		UINT32       	rsv:10;
		UINT32		RadarINT:1;
		UINT32		:9;
		UINT32       	RxCoherent:1;
		UINT32       	TxCoherent:1;
	} field;
	UINT32 word;
} INT_MASK_CSR_STRUC;
#endif


#define RINGREG_DIFF			0x10
#define TX_BASE_PTR0     0x0230	/*AC_BK base address */
#define TX_MAX_CNT0      0x0234
#define TX_CTX_IDX0       0x0238
#define TX_DTX_IDX0      0x023c
#define TX_BASE_PTR1     0x0240 	/*AC_BE base address */
#define TX_MAX_CNT1      0x0244
#define TX_CTX_IDX1       0x0248
#define TX_DTX_IDX1      0x024c
#define TX_BASE_PTR2     0x0250 	/*AC_VI base address */
#define TX_MAX_CNT2      0x0254
#define TX_CTX_IDX2       0x0258
#define TX_DTX_IDX2      0x025c
#define TX_BASE_PTR3     0x0260 	/*AC_VO base address */
#define TX_MAX_CNT3      0x0264
#define TX_CTX_IDX3       0x0268
#define TX_DTX_IDX3      0x026c
#define TX_BASE_PTR4     0x0270 	/*HCCA base address */
#define TX_MAX_CNT4      0x0274
#define TX_CTX_IDX4       0x0278
#define TX_DTX_IDX4      0x027c
#define TX_BASE_PTR5     0x0280 	/*MGMT base address */
#define  TX_MAX_CNT5     0x0284
#define TX_CTX_IDX5       0x0288
#define TX_DTX_IDX5      0x028c
#define TX_MGMTMAX_CNT      TX_MAX_CNT5
#define TX_MGMTCTX_IDX       TX_CTX_IDX5
#define TX_MGMTDTX_IDX      TX_DTX_IDX5
#define RX_BASE_PTR     0x0290 	/*RX base address */
#define RX_MAX_CNT      0x0294
#define RX_CRX_IDX       0x0298
#define RX_DRX_IDX      0x029c


#endif /*__RAL_OMAC_PCI_H__ */

