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
	mac_pci.h
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __MAC_PCI_H__
#define __MAC_PCI_H__

#include "rtmp_type.h"

#ifdef RLT_MAC
#include "mac/mac_ral/nmac/ral_nmac_pci.h"
#endif /* RLT_MAC */
#ifdef RTMP_MAC
#include "mac/mac_ral/omac/ral_omac_pci.h"
#endif /* RTMP_MAC */

#include "mac/mac_ral/rtmp_mac.h"
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
#endif


/*
	Rx descriptor format, Rx Ring
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
#endif


/* ----------------- EEPROM Related MACRO ----------------- */

/* 8051 firmware image for RT2860 - base address = 0x4000 */
#define FIRMWARE_IMAGE_BASE     0x2000
#define MAX_FIRMWARE_IMAGE_SIZE 0x2000    /* 8kbyte */


/* ----------------- Frimware Related MACRO ----------------- */
#define RTMP_WRITE_FIRMWARE(_pAd, _pFwImage, _FwLen)		\
	do{														\
		ULONG	_i, _firm;									\
		/* protect 8051 we will do firmware upload */		\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x10000);		\
															\
		for(_i=0; _i<_FwLen; _i+=4)						\
		{													\
			_firm = _pFwImage[_i] +							\
			   (_pFwImage[_i+3] << 24) +					\
			   (_pFwImage[_i+2] << 16) +					\
			   (_pFwImage[_i+1] << 8);						\
			RTMP_IO_WRITE32(_pAd, FIRMWARE_IMAGE_BASE + _i, _firm);	\
		}													\
		/* do 8051 firmware reset */						\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x00000);		\
		RTMP_IO_WRITE32(_pAd, PBF_SYS_CTRL, 0x00001);		\
															\
		/* initialize BBP R/W access agent */				\
		RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, 0);			\
		RTMP_IO_WRITE32(_pAd, H2M_MAILBOX_CSR, 0);			\
	}while(0)


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
		AsicUpdateBASession(_pAd, _wcid, _TID, _SN, _basize, 1, _type)


/* Remove the BA bitmap from ASIC for the Wcid entry */
/*		bitmap field starts at 0x10000 in ASIC WCID table */
#define RTMP_DEL_BA_SESSION_FROM_ASIC(_pAd, _Wcid, _TID, _type)				\
		AsicUpdateBASession(_pAd, _Wcid, _TID, 0, 0, 0, _type)


/* ----------------- Interface Related MACRO ----------------- */

typedef enum _RTMP_TX_DONE_MASK{
	TX_AC0_DONE = 0x1,
	TX_AC1_DONE = 0x2,
	TX_AC2_DONE = 0x4,
	TX_AC3_DONE = 0x8,
	TX_HCCA_DONE = 0x10,
	TX_MGMT_DONE = 0x20,
}RTMP_TX_DONE_MASK;


/*
	Enable & Disable NIC interrupt via writing interrupt mask register
	Since it use ADAPTER structure, it have to be put after structure definition.
*/
#define RTMP_ASIC_INTERRUPT_DISABLE(_pAd)		\
	do{			\
		RTMP_IO_WRITE32((_pAd), INT_MASK_CSR, 0x0);     /* 0: disable */	\
		RTMP_CLEAR_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);		\
	}while(0)

#define RTMP_ASIC_INTERRUPT_ENABLE(_pAd)\
	do{				\
		RTMP_IO_WRITE32((_pAd), INT_MASK_CSR, (_pAd)->int_enable_reg);     /* 1:enable */	\
		RTMP_SET_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);	\
	}while(0)

#define RTMP_IRQ_ENABLE(pAd)	\
	do{				\
		unsigned long _irqFlags;\
		RTMP_INT_LOCK(&pAd->irq_lock, _irqFlags);\
		/* clear garbage ints */\
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, 0xffffffff); \
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


struct _RTMP_ADAPTER;
enum _RTMP_TX_DONE_MASK;

BOOLEAN RTMPHandleTxRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd, enum _RTMP_TX_DONE_MASK tx_mask);
VOID RTMPHandleMgmtRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandlePreTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleRxCoherentInterrupt(struct _RTMP_ADAPTER *pAd);

VOID RTMPHandleMcuInterrupt(struct _RTMP_ADAPTER *pAd);
void RTMPHandleTwakeupInterrupt(struct _RTMP_ADAPTER *pAd);

#endif /*__MAC_PCI_H__ */

