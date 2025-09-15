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
	mt_mac_pci.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MAC_PCI_H__
#define __MAC_PCI_H__

#include "rtmp_type.h"
#include "phy/phy.h"
#include "rtmp_iface.h"
#include "rtmp_dot11.h"



/* =================================================================================
	PCI/RBUS TX / RX Frame Descriptors format

	Memory Layout

	1. Tx Descriptor
			TxD (12 bytes) + TXINFO (4 bytes)
	2. Packet Buffer
			TXWI + 802.11
         31                                                                                                                             0
	+--------------------------------------------------------------------------+
	|                                   SDP0[31:0]                                                                               |
	+-+--+---------------------+-+--+-----------------------------------------+
	|D |L0|       SDL0[13:0]              |B|L1|                    SDL1[13:0]                                    |
	+-+--+---------------------+-+--+-----------------------------------------+
	|                                   SDP1[31:0]                                                                               |
	+--------------------------------------------------------------------------+
	|                                        TXINFO                                                                                |
	+--------------------------------------------------------------------------+
================================================================================= */
/*
	TX descriptor format for Tx Data/Mgmt Rings
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word 0 */
	UINT32		SDPtr0;
	/* Word 1 */
	UINT32		DMADONE:1;
	UINT32		LastSec0:1;
	UINT32		SDLen0:14;
	UINT32		Burst:1;
	UINT32		LastSec1:1;
	UINT32		SDLen1:14;
	/* Word 2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#else
typedef	struct GNU_PACKED _TXD_STRUC {
	/* Word	0 */
	UINT32		SDPtr0;
	/* Word	1 */
	UINT32		SDLen1:14;
	UINT32		LastSec1:1;
	UINT32		Burst:1;
	UINT32		SDLen0:14;
	UINT32		LastSec0:1;
	UINT32		DMADONE:1;
	/*Word2 */
	UINT32		SDPtr1;
} TXD_STRUC;
#endif /* RT_BIG_ENDIAN */


/*
	Rx descriptor format for Rx Rings
*/
#ifdef RT_BIG_ENDIAN
typedef	struct GNU_PACKED _RXD_STRUC{
	/* Word 0 */
	UINT32		SDP0;
	/* Word 1 */
	UINT32		DDONE:1;
	UINT32		LS0:1;
	UINT32		SDL0:14;
	UINT32		BURST:1;
	UINT32		LS1:1;
	UINT32		SDL1:14;
	/* Word 2 */
	UINT32		SDP1;
}RXD_STRUC;
#else
typedef	struct GNU_PACKED _RXD_STRUC{
	/* Word	0 */
	UINT32		SDP0;
	/* Word	1 */
	UINT32		SDL1:14;
	UINT32		LS1:1;
	UINT32		BURST:1;
	UINT32		SDL0:14;
	UINT32		LS0:1;
	UINT32		DDONE:1;
	/* Word	2 */
	UINT32		SDP1;
}RXD_STRUC;
#endif /* RT_BIG_ENDIAN */


#define HIF_BASE		0x4000


/*
	CR definitions
*/
#define MT_INT_R0_DONE		(1<<0)
#define MT_INT_R1_DONE		(1<<1)

#define MT_INT_ERR_DET0	(1<<2)
#define MT_INT_ERR_DET1	(1<<3)

#define MT_INT_T0_DONE		(1<<4)
#define MT_INT_T1_DONE		(1<<5)
#define MT_INT_T2_DONE		(1<<6)
#define MT_INT_T3_DONE		(1<<7)
#define MT_INT_T4_DONE		(1<<8)
#define MT_INT_T5_DONE		(1<<9)
#define MT_INT_T6_DONE		(1<<10)
#define MT_INT_T7_DONE		(1<<11)
#define MT_INT_T8_DONE		(1<<12)
#define MT_INT_T9_DONE		(1<<13)
#define MT_INT_T10_DONE	(1<<14)
#define MT_INT_T11_DONE	(1<<15)
#define MT_INT_T12_DONE	(1<<16)
#define MT_INT_T13_DONE	(1<<17)

#define MT_INT_RESVD		(1<<18)

#define MT_INT_T15_DONE	(1<<19)

#define MT_INT_RX_COHE		(1<<20)
#define MT_INT_TX_COHE		(1<<21)

#define MT_INT_RX_DLY		(1<<22)
#define MT_INT_TX_DLY		(1<<23)

#define WF_MAC_INT_0		(1<<24)
#define WF_MAC_INT_1		(1<<25)
#define WF_MAC_INT_2		(1<<26)
#define WF_MAC_INT_3		(1<<27)
#define WF_MAC_INT_4		(1<<28)
#define WF_MAC_INT_5		(1<<29)

#define MT_INT_MCU_CMD		(1<<30)

#define MT_INT_RESVD_2		(1<<31)


#define MT_RxINT		(MT_INT_R0_DONE | MT_INT_R1_DONE /* | MT_INT_RX_DLY */)	 /* Delayed Rx or indivi rx */
#define MT_TxDataInt		(MT_INT_T0_DONE | \
						 MT_INT_T1_DONE | \
						 MT_INT_T2_DONE | \
						 MT_INT_T3_DONE /*| MT_INT_TX_DLY*/)	/* Delayed Tx or indivi tx */

#define MT_TxMgmtInt		(MT_INT_T4_DONE /*| INT_TX_DLY*/)

#define MT_TxCoherent		MT_INT_TX_COHE
#define MT_RxCoherent		MT_INT_RX_COHE
#define MT_TxRxCoherent		(MT_INT_TX_COHE | MT_INT_RX_COHE)

#define MT_McuCommand		MT_INT_MCU_CMD	/* mcu */


#define MT_INT_RX			(MT_INT_R0_DONE | MT_INT_R1_DONE)
#define MT_INT_RX_DATA		(MT_INT_R0_DONE)
#define MT_INT_RX_CMD		(MT_INT_R1_DONE)

#define MT_INT_AC0_DLY		(MT_INT_T0_DONE)
#define MT_INT_AC1_DLY		(MT_INT_T1_DONE)
#define MT_INT_AC2_DLY		(MT_INT_T2_DONE)
#define MT_INT_AC3_DLY		(MT_INT_T3_DONE)
#define MT_INT_CMD			(MT_INT_T5_DONE)

#define MT_INT_MGMT_DLY	(MT_INT_T4_DONE)
#define MT_INT_BMC_DLY		(MT_INT_T8_DONE) /* (MT_INT_T6_DONE) */
#define MT_INT_BCN_DLY		(MT_INT_T7_DONE) /* (MT_INT_T15_DONE) */

#define MT_DELAYINTMASK	0x3FFBFFFF
#define MT_INTMASK			0x3FFBFFFF


/*
	INT_SOURCE_CSR: Interrupt source register. Write one to clear corresponding bit
	Refer to MT_INT_SOURCE_CSR(0x4000)
*/
#ifdef RT_BIG_ENDIAN
typedef union _MT_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 rsv_31:1;
		UINT32 mcu_cmd_int:1;
		UINT32 wf_mac_int_5:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_0:1;
		UINT32 tx_dly_int:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_coherent:1;
		UINT32 rx_coherent:1;
		UINT32 tx_done_15:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_13:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_0:1;
		UINT32 err_det_int_1:1;
		UINT32 err_det_int_0:1;
		UINT32 rx_done_1:1;
		UINT32 rx_done_0:1;
	}field;
	UINT32 word;
}MT_INT_SOURCE_CSR_STRUC;
#else
typedef union _MT_INT_SOURCE_CSR_STRUC {
	struct {
		UINT32 rx_done_0:1;
		UINT32 rx_done_1:1;
		UINT32 err_det_int_0:1;
		UINT32 err_det_int_1:1;
		UINT32 tx_done_0:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_13:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_15:1;
		UINT32 rx_coherent:1;
		UINT32 tx_coherent:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_dly_int:1;
		UINT32 wf_mac_int_0:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_5:1;
		UINT32 mcu_cmd_int:1;
		UINT32 rsv_31:1;
	}field;
	UINT32 word;
}MT_INT_SOURCE_CSR_STRUC;
#endif /* RT_BIG_ENDIAN */


/* INT_MASK_CSR:   Interrupt MASK register.   1: the interrupt is mask OFF */
#define WPDMA_INT_MSK        (HIF_BASE + 0x204)
#ifdef RT_BIG_ENDIAN
typedef union _MT_WPDMA_INT_MASK{
	struct {
		UINT32 rsv_31:1;
		UINT32 mcu_cmd_int:1;
		UINT32 wf_mac_int_5:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_0:1;
		UINT32 tx_dly_int:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_coherent:1;
		UINT32 rx_coherent:1;
		UINT32 tx_done_15:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_13:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_0:1;
		UINT32 err_det_int_1:1;
		UINT32 err_det_int_0:1;
		UINT32 rx_done_1:1;
		UINT32 rx_done_0:1;
	}field;
	UINT32 word;
}MT_WPMDA_INT_MASK;
#else
typedef union _MT_WPDMA_INT_MASK{
	struct {
		UINT32 rx_done_0:1;
		UINT32 rx_done_1:1;
		UINT32 err_det_int_0:1;
		UINT32 err_det_int_1:1;
		UINT32 tx_done_0:1;
		UINT32 tx_done_1:1;
		UINT32 tx_done_2:1;
		UINT32 tx_done_3:1;
		UINT32 tx_done_4:1;
		UINT32 tx_done_5:1;
		UINT32 tx_done_6:1;
		UINT32 tx_done_7:1;
		UINT32 tx_done_8:1;
		UINT32 tx_done_9:1;
		UINT32 tx_done_10:1;
		UINT32 tx_done_11:1;
		UINT32 tx_done_12:1;
		UINT32 tx_done_13:1;
		UINT32 rsv_18:1;
		UINT32 tx_done_15:1;
		UINT32 rx_coherent:1;
		UINT32 tx_coherent:1;
		UINT32 rx_dly_int:1;
		UINT32 tx_dly_int:1;
		UINT32 wf_mac_int_0:1;
		UINT32 wf_mac_int_1:1;
		UINT32 wf_mac_int_2:1;
		UINT32 wf_mac_int_3:1;
		UINT32 wf_mac_int_4:1;
		UINT32 wf_mac_int_5:1;
		UINT32 mcu_cmd_int:1;
		UINT32 rsv_31:1;
	}field;
	UINT32 word;
}MT_WPMDA_INT_MASK;
#endif /* RT_BIG_ENDIAN */


//#define WPDMA_RST_PTR 	(HIF_BASE + 0x00c)    /* WPDMA_RST_IDX */


/*@!Release
	Tx Ring Layout and assignments

	Totally we have 10 Tx Rings and assigned as following usage:
	1. RT85592
		TxRing 0~3: for TxQ Channel 1 with AC_BK/BE/VI/VO
		TxRing 4    : for TxQ CTRL
		TxRing 5    : for TxQ MGMT
		TxRing 6~9: for TxQ Channel 2 with AC_BK/BE/VI/VO

	2. MT7650
		TxRing 0~3: for TxQ Channel 1 with AC_BK/BE/VI/VO
		TxRing 4~7: for TxQ Channel 2 with AC_BK/BE/VI/VO
		TxRing 8    : for TxQ CTRL
		TxRing 9    : for TxQ MGMT

	For each TxRing, we have four register to control it
		TX_RINGn_CTRL0 (0x0): base address of this ring(4-DWORD aligned address)
		TX_RINGn_CTRL1 (0x4): maximum number of TxD count in this ring
		TX_RINGn_CTRL2 (0x8): Point to the next TxD CPU wants to use
		TX_RINGn_CTRL3 (0xc): Point to the next TxD DMA wants to use
*/
#define MT_RINGREG_DIFF		0x10

#define MT_TX_RING_BASE		(HIF_BASE + 0x0300)
#define MT_TX_RING_NUM			13
#define MT_TX_RING_PTR			(MT_TX_RING_BASE + 0x0)
#define MT_TX_RING_CNT			(MT_TX_RING_BASE + 0x4)
#define MT_TX_RING_CIDX		(MT_TX_RING_BASE + 0x8)
#define MT_TX_RING_DIDX		(MT_TX_RING_BASE + 0xc)

#define TX_CHAN_BASE_1			(MT_TX_RING_BASE + MT_RINGREG_DIFF * 0)
#define TX_CHAN_BASE_2			(MT_TX_RING_BASE + MT_RINGREG_DIFF * 6)

/* following address is base on TX_CHAN_BASE_X */
#define MT_TX_RING_BK_BASE		0x0
#define MT_TX_RING_BK_CNT		(MT_TX_RING_BK_BASE + 0x04)
#define MT_TX_RING_BK_CIDX		(MT_TX_RING_BK_BASE + 0x08)
#define MT_TX_RING_BK_DIDX		(MT_TX_RING_BK_BASE + 0x0c)

#define MT_TX_RING_BE_BASE		(MT_TX_RING_BK_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_BE_CNT		(MT_TX_RING_BE_BASE + 0x04)
#define MT_TX_RING_BE_CIDX		(MT_TX_RING_BE_BASE + 0x08)
#define MT_TX_RING_BE_DIDX		(MT_TX_RING_BE_BASE + 0x0c)

#define MT_TX_RING_VI_BASE		(MT_TX_RING_BE_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_VI_CNT		(MT_TX_RING_VI_BASE + 0x04)
#define MT_TX_RING_VI_CIDX		(MT_TX_RING_VI_BASE + 0x08)
#define MT_TX_RING_VI_DIDX		(MT_TX_RING_VI_BASE + 0x0c)

#define MT_TX_RING_VO_BASE	(MT_TX_RING_VI_BASE + MT_RINGREG_DIFF)
#define MT_TX_RING_VO_CNT		(MT_TX_RING_VO_BASE + 0x04)
#define MT_TX_RING_VO_CIDX		(MT_TX_RING_VO_BASE + 0x08)
#define MT_TX_RING_VO_DIDX		(MT_TX_RING_VO_BASE + 0x0c)


#define MT_TX_RING_BCN_IDX		7

/*
	Rx Ring Layput and assignments

	Totally we have 2 Rx Rings and assigned as following usage:
		RxRing 0: for all received data packets
		RxRing 1: for internal ctrl/info packets generated by on-chip CPU.

	For each TxRing, we have four register to control it
		RX_RING_CTRL0 (0x0): base address of this ring(4-DWORD aligned address)
		RX_RING_CTRL1 (0x4): maximum number of RxD count in this ring
		RX_RING_CTRL2 (0x8): Point to the next RxD CPU wants to use
		RX_RING_CTRL3 (0xc): Point to the next RxD DMA wants to use
*/
#define MT_RX_RING_BASE	(HIF_BASE + 0x0400)
#define MT_RX_RING1_BASE	(HIF_BASE + 0x0410)
#define MT_RX_RING_NUM	2
#define MT_RX_RING_PTR		(MT_RX_RING_BASE + 0x00)
#define MT_RX_RING_CNT		(MT_RX_RING_BASE + 0x04)
#define MT_RX_RING_CIDX	(MT_RX_RING_BASE + 0x08)
#define MT_RX_RING_DIDX	(MT_RX_RING_BASE + 0x0c)

#define MT_RX_RING1_PTR	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1)
#define MT_RX_RING1_CNT	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x04)
#define MT_RX_RING1_CIDX	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x08)
#define MT_RX_RING1_DIDX	(MT_RX_RING_BASE + MT_RINGREG_DIFF * 1 + 0x0c)

#define MT_TXTIME_THD_0		(HIF_BASE + 0x500)
#define MT_TXTIME_THD_1		(HIF_BASE + 0x504)
#define MT_TXTIME_THD_2		(HIF_BASE + 0x508)
#define MT_TXTIME_THD_3		(HIF_BASE + 0x50c)
#define MT_TXTIME_THD_4		(HIF_BASE + 0x510)
#define MT_TXTIME_THD_5		(HIF_BASE + 0x514)
#define MT_TXTIME_THD_6		(HIF_BASE + 0x518)
#define MT_TXTIME_THD_7		(HIF_BASE + 0x51c)
#define MT_TXTIME_THD_8		(HIF_BASE + 0x520)
#define MT_TXTIME_THD_9		(HIF_BASE + 0x524)
#define MT_TXTIME_THD_10	(HIF_BASE + 0x528)
#define MT_TXTIME_THD_11	(HIF_BASE + 0x52c)
#define MT_TXTIME_THD_12	(HIF_BASE + 0x530)
#define MT_TXTIME_THD_13	(HIF_BASE + 0x534)
#define MT_TXTIME_THD_14	(HIF_BASE + 0x538)
#define MT_TXTIME_THD_15	(HIF_BASE + 0x53c)

#define MT_PAGE_CNT_0		(HIF_BASE + 0x540)
#define MT_PAGE_CNT_1		(HIF_BASE + 0x544)
#define MT_PAGE_CNT_2		(HIF_BASE + 0x548)
#define MT_PAGE_CNT_3		(HIF_BASE + 0x54c)
#define MT_PAGE_CNT_4		(HIF_BASE + 0x550)
#define MT_PAGE_CNT_5		(HIF_BASE + 0x554)
#define MT_PAGE_CNT_6		(HIF_BASE + 0x558)
#define MT_PAGE_CNT_7		(HIF_BASE + 0x55c)
#define MT_PAGE_CNT_8		(HIF_BASE + 0x560)
#define MT_PAGE_CNT_9		(HIF_BASE + 0x564)
#define MT_PAGE_CNT_10		(HIF_BASE + 0x568)
#define MT_PAGE_CNT_11		(HIF_BASE + 0x56c)
#define MT_PAGE_CNT_12		(HIF_BASE + 0x570)
#define MT_PAGE_CNT_13		(HIF_BASE + 0x574)
#define MT_PAGE_CNT_14		(HIF_BASE + 0x578)
#define MT_PAGE_CNT_15		(HIF_BASE + 0x57c)

#define MT_QUEUE_PRIORITY_1	(HIF_BASE + 0x580)
#define MT_QUEUE_PRIORITY_2	(HIF_BASE + 0x584)

#define MT_SCH_REG_1		(HIF_BASE + 0x588)
#define MT_SCH_REG_2		(HIF_BASE + 0x58c)
#define MT_SCH_REG_3		(HIF_BASE + 0x590)
#define MT_SCH_REG_4		(HIF_BASE + 0x594)

#define MT_GROUP_THD_0		(HIF_BASE + 0x598)
#define MT_GROUP_THD_1		(HIF_BASE + 0x59c)
#define MT_GROUP_THD_2		(HIF_BASE + 0x5a0)
#define MT_GROUP_THD_3		(HIF_BASE + 0x5a4)
#define MT_GROUP_THD_4		(HIF_BASE + 0x5a8)
#define MT_GROUP_THD_5		(HIF_BASE + 0x5ac)

#define MT_BMAP_0		(HIF_BASE + 0x5b0)
#define MT_BMAP_1		(HIF_BASE + 0x5b4)
#define MT_BMAP_2		(HIF_BASE + 0x5b8)

#define MT_HIGH_PRIORITY_1	(HIF_BASE + 0x5bc)
#define MT_HIGH_PRIORITY_2	(HIF_BASE + 0x5c0)
#define MT_PRIORITY_MASK	(HIF_BASE + 0x5c4)

#define MT_RSV_MAX_THD		(HIF_BASE + 0x5c8)



/* ----------------- ASIC Related MACRO ----------------- */
/* reset MAC of a station entry to 0x000000000000 */
#define RTMP_STA_ENTRY_MAC_RESET(pAd, Wcid)	\
	AsicDelWcidTab(pAd, Wcid);

/* add this entry into ASIC RX WCID search table */
#define RTMP_STA_ENTRY_ADD(pAd, pEntry)		\
	AsicUpdateRxWCIDTable(pAd, pEntry->wcid, pEntry->Addr);

#define RTMP_UPDATE_PROTECT(_pAd, _OperationMode, _SetMask, _bDisableBGProtect, _bNonGFExist)	\
	AsicUpdateProtect(pAd, _OperationMode, _SetMask, _bDisableBGProtect, _bNonGFExist);

#ifdef CONFIG_AP_SUPPORT
#define RTMP_AP_UPDATE_CAPABILITY_AND_ERPIE(pAd)	\
	APUpdateCapabilityAndErpIe(pAd);
#endif /* CONFIG_AP_SUPPORT */

/* Insert the BA bitmap to ASIC for the Wcid entry */
#define RTMP_ADD_BA_SESSION_TO_ASIC(_pAd, _wcid, _TID, _SN, _basize, _type)	\
		AsicUpdateBASession(_pAd, _wcid, _TID, _SN, _basize, 1, _type);


/* Remove the BA bitmap from ASIC for the Wcid entry */
/*		bitmap field starts at 0x10000 in ASIC WCID table */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _wcid, _TID, _type) \
		AsicUpdateBASession(_pAd, _wcid, _TID, 0, 0, 0, _type);


/* ----------------- Interface Related MACRO ----------------- */

typedef enum _RTMP_TX_DONE_MASK{
	TX_AC0_DONE = 0x1,
	TX_AC1_DONE = 0x2,
	TX_AC2_DONE = 0x4,
	TX_AC3_DONE = 0x8,
	TX_HCCA_DONE = 0x10,
	TX_MGMT_DONE = 0x20,
	TX_BMC_DONE = 0x40,
}RTMP_TX_DONE_MASK;


/*
	Enable & Disable NIC interrupt via writing interrupt mask register
	Since it use ADAPTER structure, it have to be put after structure definition.
*/
#define RTMP_ASIC_INTERRUPT_DISABLE(_pAd)		\
	do{			\
		RTMP_IO_WRITE32((_pAd), MT_INT_MASK_CSR, 0x0);     /* 0: disable */	\
		RTMP_CLEAR_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);		\
	}while(0)

#define RTMP_ASIC_INTERRUPT_ENABLE(_pAd)\
	do{				\
		RTMP_IO_WRITE32((_pAd), MT_INT_MASK_CSR, (_pAd)->int_enable_reg);     /* 1:enable */	\
		RTMP_SET_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);	\
	}while(0)

#define RTMP_IRQ_ENABLE(pAd)	\
	do{				\
		unsigned long _irqFlags;\
		RTMP_INT_LOCK(&pAd->irq_lock, _irqFlags);\
		/* clear garbage ints */\
		RTMP_IO_WRITE32(pAd, MT_INT_SOURCE_CSR, 0xffffffff); \
		RTMP_ASIC_INTERRUPT_ENABLE(pAd); \
		RTMP_INT_UNLOCK(&pAd->irq_lock, _irqFlags);\
	} while(0)



/* ----------------- MLME Related MACRO ----------------- */
#define RTMP_MLME_PRE_SANITY_CHECK(pAd)

#define RTMP_MLME_RESET_STATE_MACHINE(pAd)	\
		MlmeRestartStateMachine(pAd)

#define RTMP_HANDLE_COUNTER_MEASURE(_pAd, _pEntry)\
		HandleCounterMeasure(_pAd, _pEntry)

/* ----------------- Power Save Related MACRO ----------------- */
#define RTMP_PS_POLL_ENQUEUE(pAd)				EnqueuePsPoll(pAd)


/* For RTMPPCIePowerLinkCtrlRestore () function */
#define RESTORE_HALT		1
#define RESTORE_WAKEUP		2
#define RESTORE_CLOSE           3

#define PowerSafeCID		1
#define PowerRadioOffCID	2
#define PowerWakeCID		3
#define CID0MASK		0x000000ff
#define CID1MASK		0x0000ff00
#define CID2MASK		0x00ff0000
#define CID3MASK		0xff000000




/* ----------------- Security Related MACRO ----------------- */

/* Set Asic WCID Attribute table */
#define RTMP_SET_WCID_SEC_INFO(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)	\
	RTMPSetWcidSecurityInfo(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)

/* Set Asic WCID IV/EIV table */
#define RTMP_ASIC_WCID_IVEIV_TABLE(_pAd, _Wcid, _uIV, _uEIV)	\
	AsicUpdateWCIDIVEIV(_pAd, _Wcid, _uIV, _uEIV)

/* Set Asic WCID Attribute table (offset:0x6800) */
#define RTMP_ASIC_WCID_ATTR_TABLE(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)\
	AsicUpdateWcidAttributeEntry(_pAd, _BssIdx, _KeyIdx, _CipherAlg, _Wcid, _KeyTabFlag)

/* Set Asic Pairwise key table */
#define RTMP_ASIC_PAIRWISE_KEY_TABLE(_pAd, _WCID, _pCipherKey)	\
	AsicAddPairwiseKeyEntry(_pAd, _WCID, _pCipherKey)

/* Set Asic Shared key table */
#define RTMP_ASIC_SHARED_KEY_TABLE(_pAd, _BssIndex, _KeyIdx, _pCipherKey) \
	AsicAddSharedKeyEntry(_pAd, _BssIndex, _KeyIdx, _pCipherKey)


/* Remove Pairwise Key table */
#define RTMP_REMOVE_PAIRWISE_KEY_ENTRY(_pAd, _Wcid)\
	AsicRemovePairwiseKeyEntry(_pAd, _Wcid)

#define MT_ADDREMOVE_KEY(_pAd, _AddRemove, _BssIdx, _KeyIdx, _Wcid, _KeyTabFlag, _pCipherKey, _PeerAddr)\
	CmdProcAddRemoveKey(_pAd, _AddRemove, _BssIdx, _KeyIdx, _Wcid, _KeyTabFlag, _pCipherKey, _PeerAddr)

struct _RTMP_ADAPTER;
enum _RTMP_TX_DONE_MASK;

BOOLEAN RTMPHandleTxRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd, enum _RTMP_TX_DONE_MASK tx_mask);
VOID RTMPHandleMgmtRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandlePreTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleRxCoherentInterrupt(struct _RTMP_ADAPTER *pAd);

VOID RTMPHandleMcuInterrupt(struct _RTMP_ADAPTER *pAd);
void RTMPHandleTwakeupInterrupt(struct _RTMP_ADAPTER *pAd);

VOID mt_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

#endif /*__MAC_PCI_H__ */

