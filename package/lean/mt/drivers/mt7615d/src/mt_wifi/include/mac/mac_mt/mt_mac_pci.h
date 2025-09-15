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



/* ----------------- Interface Related MACRO ----------------- */

typedef enum _RTMP_TX_DONE_MASK {
	TX_AC0_DONE = 0,
	TX_AC1_DONE = 1,
	TX_AC2_DONE = 2,
	TX_AC3_DONE = 3,
	TX_HCCA_DONE = 4,
	TX_MGMT_DONE = 5,
	TX_BMC_DONE = 6,
} RTMP_TX_DONE_MASK;


/*
	Enable & Disable NIC interrupt via writing interrupt mask register
	Since it use ADAPTER structure, it have to be put after structure definition.
*/
#define RTMP_ASIC_INTERRUPT_DISABLE(_pAd)		\
	do {			\
		HIF_IO_WRITE32((_pAd), MT_INT_MASK_CSR, 0); /*0: disable*/\
		RTMP_CLEAR_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);		\
	} while (0)

#define RTMP_ASIC_INTERRUPT_ENABLE(_pAd)\
	do {				\
		HIF_IO_WRITE32((_pAd), MT_INT_MASK_CSR, (_pAd)->PciHif.IntEnableReg);     /* 1:enable */	\
		RTMP_SET_FLAG((_pAd), fRTMP_ADAPTER_INTERRUPT_ACTIVE);	\
	} while (0)

#define RTMP_IRQ_ENABLE(_pAd)	\
	do {				\
		unsigned long _irqFlags;\
		RTMP_INT_LOCK(&(_pAd)->irq_lock, _irqFlags);\
		/* clear garbage ints */\
		HIF_IO_WRITE32((_pAd), MT_INT_SOURCE_CSR, 0xffffffff); \
		RTMP_ASIC_INTERRUPT_ENABLE(_pAd); \
		RTMP_INT_UNLOCK(&(_pAd)->irq_lock, _irqFlags);\
	} while (0)



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

struct _RTMP_ADAPTER;
enum _RTMP_TX_DONE_MASK;

BOOLEAN mtd_free_txd(struct _RTMP_ADAPTER *pAd, UINT8 resource_idx);
BOOLEAN mtd_tx_dma_done_handle(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);
BOOLEAN mt_cmd_dma_done_handle(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);
BOOLEAN mt_fwdl_dma_done_handle(struct _RTMP_ADAPTER *pAd, UINT8 hif_idx);
VOID RTMPHandleMgmtRingDmaDoneInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandlePreTBTTInterrupt(struct _RTMP_ADAPTER *pAd);
VOID RTMPHandleRxCoherentInterrupt(struct _RTMP_ADAPTER *pAd);

VOID RTMPHandleMcuInterrupt(struct _RTMP_ADAPTER *pAd);
void RTMPHandleTwakeupInterrupt(struct _RTMP_ADAPTER *pAd);

VOID mt_asic_init_txrx_ring(struct _RTMP_ADAPTER *pAd);

#endif /*__MAC_PCI_H__ */

