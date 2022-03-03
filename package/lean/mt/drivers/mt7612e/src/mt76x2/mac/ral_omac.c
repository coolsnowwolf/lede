/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:

	Abstract:

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/

#include "rt_config.h"


INT rtmp_get_rxwi_phymode(RXWI_STRUC *rxwi)
{
	return rxwi->RXWI_O.phy_mode;
}

INT rtmp_get_rxwi_rssi(RXWI_STRUC *rxwi, INT size, CHAR *rssi)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)rxwi;

	switch (size) {
		case 3:
			rssi[2] = rxwi_o->RSSI2;
		case 2:
			rssi[1] = rxwi_o->RSSI1;
		case 1:
		default:
			rssi[0] = rxwi_o->RSSI0;
			break;
	}

	return 0;
}


INT rtmp_get_rxwi_snr(RXWI_STRUC *rxwi, INT size, UCHAR *snr)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)rxwi;
	
	switch (size) {
		case 3:
			snr[2] = rxwi_o->SNR2;
		case 2:
			snr[1] = rxwi_o->SNR1;
		case 1:
		default:
			snr[0] = rxwi_o->SNR0;
			break;
	}
	
	return 0;
}


#ifdef DBG
static UCHAR *txwi_txop_str[]={"HT_TXOP", "PIFS", "SIFS", "BACKOFF", "Invalid"};
#define TXWI_TXOP_STR(_x)	((_x) <= 3 ? txwi_txop_str[(_x)]: txwi_txop_str[4])
VOID dump_rtmp_txwi(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI)
{
	struct _TXWI_OMAC *txwi_o = (struct _TXWI_OMAC *)pTxWI;

	ASSERT((sizeof(struct _TXWI_OMAC) == pAd->chipCap.TXWISize));

	if (pAd->chipCap.TXWISize != (sizeof(struct _TXWI_OMAC)))
		DBGPRINT(RT_DEBUG_TRACE, ("%s():sizeof(struct _TXWI_OMAC)=%d, pAd->chipCap.TXWISize=%d\n",
					__FUNCTION__, sizeof(struct _TXWI_OMAC), pAd->chipCap.TXWISize));

	DBGPRINT(RT_DEBUG_OFF, ("\tPHYMODE=%d(%s)\n", txwi_o->PHYMODE, get_phymode_str(txwi_o->PHYMODE)));
	DBGPRINT(RT_DEBUG_OFF, ("\tiTxBF=%d\n", txwi_o->iTxBF));
	DBGPRINT(RT_DEBUG_OFF, ("\tSounding=%d\n", txwi_o->Sounding));
	DBGPRINT(RT_DEBUG_OFF, ("\teTxBF=%d\n", txwi_o->eTxBF));
	DBGPRINT(RT_DEBUG_OFF, ("\tSTBC=%d\n", txwi_o->STBC));
	DBGPRINT(RT_DEBUG_OFF, ("\tShortGI=%d\n", txwi_o->ShortGI));
	DBGPRINT(RT_DEBUG_OFF, ("\tBW=%d(%sMHz)\n", txwi_o->BW, get_bw_str(txwi_o->BW)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMCS=%d\n", txwi_o->MCS));
	DBGPRINT(RT_DEBUG_OFF, ("\tTxOP=%d(%s)\n", txwi_o->txop, TXWI_TXOP_STR(txwi_o->txop)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMpduDensity=%d\n", txwi_o->MpduDensity));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", txwi_o->AMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tTS=%d\n", txwi_o->TS));
	DBGPRINT(RT_DEBUG_OFF, ("\tCF-ACK=%d\n", txwi_o->CFACK));
	DBGPRINT(RT_DEBUG_OFF, ("\tMIMO-PS=%d\n", txwi_o->MIMOps));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", txwi_o->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tPID=%d\n", txwi_o->PacketId));
	DBGPRINT(RT_DEBUG_OFF, ("\tMPDUtotalByteCnt=%d\n", txwi_o->MPDUtotalByteCnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tWCID=%d\n", txwi_o->wcid));
	DBGPRINT(RT_DEBUG_OFF, ("\tBAWinSize=%d\n", txwi_o->BAWinSize));
	DBGPRINT(RT_DEBUG_OFF, ("\tNSEQ=%d\n", txwi_o->NSEQ));
	DBGPRINT(RT_DEBUG_OFF, ("\tACK=%d\n", txwi_o->ACK));
}


VOID dump_rtmp_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI)
{
	struct _RXWI_OMAC *rxwi_o = (struct _RXWI_OMAC *)pRxWI;

	ASSERT((sizeof(struct _RXWI_OMAC) == pAd->chipCap.RXWISize));

	if (pAd->chipCap.RXWISize != (sizeof(struct _RXWI_OMAC)))
		DBGPRINT(RT_DEBUG_TRACE, ("%s():sizeof(struct _RXWI_OMAC)=%d, pAd->chipCap.RXWISize=%d\n",
					__FUNCTION__, sizeof(struct _RXWI_OMAC), pAd->chipCap.RXWISize));

	DBGPRINT(RT_DEBUG_OFF, ("\tWCID=%d\n", rxwi_o->wcid));
	DBGPRINT(RT_DEBUG_OFF, ("\tMPDUtotalByteCnt=%d\n", rxwi_o->MPDUtotalByteCnt));
	DBGPRINT(RT_DEBUG_OFF, ("\tPhyMode=%d(%s)\n", rxwi_o->phy_mode, get_phymode_str(rxwi_o->phy_mode)));
	DBGPRINT(RT_DEBUG_OFF, ("\tMCS=%d\n", rxwi_o->mcs));
	DBGPRINT(RT_DEBUG_OFF, ("\tBW=%d\n", rxwi_o->bw));
	DBGPRINT(RT_DEBUG_OFF, ("\tSGI=%d\n", rxwi_o->sgi));
	DBGPRINT(RT_DEBUG_OFF, ("\tSTBC=%d\n", rxwi_o->stbc));


	DBGPRINT(RT_DEBUG_OFF, ("\tSequence=%d\n", rxwi_o->SEQUENCE));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", rxwi_o->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tTID=%d\n", rxwi_o->tid));

	DBGPRINT(RT_DEBUG_OFF, ("\tkey_idx=%d\n", rxwi_o->key_idx));
	DBGPRINT(RT_DEBUG_OFF, ("\tBSS_IDX=%d\n", rxwi_o->bss_idx));

	DBGPRINT(RT_DEBUG_OFF, ("\tRSSI=%d:%d:%d\n", rxwi_o->RSSI0, rxwi_o->RSSI1, rxwi_o->RSSI2));
	DBGPRINT(RT_DEBUG_OFF, ("\tSNR=%d:%d:%d\n", rxwi_o->SNR0, rxwi_o->SNR1, rxwi_o->SNR2));
	DBGPRINT(RT_DEBUG_OFF, ("\tFreqOffset=%d\n", rxwi_o->FOFFSET));
}
#endif


VOID rtmp_asic_init_txrx_ring(RTMP_ADAPTER *pAd)
{
	DELAY_INT_CFG_STRUC IntCfg;
	WPDMA_GLO_CFG_STRUC GloCfg;
	UINT32 phy_addr, offset;
	INT i;


	/*
		Write Tx Ring base address registers 
		
		The Tx Ring arrangement:
		RingIdx	SwRingIdx	AsicPriority	WMM QID
		0 		TxSw0		L			QID_AC_BE
		1		TxSw1		L			QID_AC_BK
		2		TxSw2		L			QID_AC_VI
		3		TxSw3		L			QID_AC_VO

		4		HCCA		M			-
		5		MGMT		H			-

		Ring 0~3 for TxChannel 0
		Ring 6~9 for TxChannel 1
	*/
	for (i = 0; i < NUM_OF_TX_RING; i++) {
		offset = i * 0x10;
		phy_addr = RTMP_GetPhysicalAddressLow(pAd->TxRing[i].Cell[0].AllocPa);
		pAd->TxRing[i].TxSwFreeIdx = 0;
		pAd->TxRing[i].TxCpuIdx = 0;
		pAd->TxRing[i].hw_desc_base = TX_BASE_PTR0 + offset;
		pAd->TxRing[i].hw_cidx_addr = TX_CTX_IDX0 + offset;
		pAd->TxRing[i].hw_didx_addr = TX_DTX_IDX0 + offset;
		RTMP_IO_WRITE32(pAd, pAd->TxRing[i].hw_desc_base, phy_addr);
		RTMP_IO_WRITE32(pAd, pAd->TxRing[i].hw_cidx_addr, pAd->TxRing[i].TxCpuIdx);
		RTMP_IO_WRITE32(pAd, TX_MAX_CNT0 + offset, TX_RING_SIZE);
		DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_%d[0x%x]: Base=0x%x, Cnt=%d!\n",
					i, pAd->TxRing[i].hw_desc_base, phy_addr, TX_RING_SIZE));
	}

	/* init MGMT ring index pointer */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->MgmtRing.Cell[0].AllocPa);
	pAd->MgmtRing.TxSwFreeIdx = 0;
	pAd->MgmtRing.TxCpuIdx = 0;
	pAd->MgmtRing.hw_desc_base = TX_BASE_PTR5;
	pAd->MgmtRing.hw_cidx_addr = TX_MGMTCTX_IDX;
	pAd->MgmtRing.hw_didx_addr = TX_MGMTDTX_IDX;
	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->MgmtRing.hw_cidx_addr, pAd->MgmtRing.TxCpuIdx);
	RTMP_IO_WRITE32(pAd, TX_MGMTMAX_CNT, MGMT_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->TX_RING_MGMT[0x%x]: Base=0x%x, Cnt=%d!\n",
					pAd->MgmtRing.hw_desc_base, phy_addr, MGMT_RING_SIZE));

	/* Init RX Ring Base/Size/Index pointer CSR */
	phy_addr = RTMP_GetPhysicalAddressLow(pAd->RxRing[0].Cell[0].AllocPa);
	pAd->RxRing[0].RxSwReadIdx = 0;
	pAd->RxRing[0].RxCpuIdx = RX_RING_SIZE - 1;
	pAd->RxRing[0].hw_desc_base = RX_BASE_PTR;
	pAd->RxRing[0].hw_cidx_addr = RX_CRX_IDX;
	pAd->RxRing[0].hw_didx_addr = RX_DRX_IDX;
	RTMP_IO_WRITE32(pAd, pAd->RxRing[0].hw_desc_base, phy_addr);
	RTMP_IO_WRITE32(pAd, pAd->RxRing[0].hw_cidx_addr, pAd->RxRing[0].RxCpuIdx);
	RTMP_IO_WRITE32(pAd, RX_MAX_CNT, RX_RING_SIZE);
	DBGPRINT(RT_DEBUG_TRACE, ("-->RX_RING[0x%x]: Base=0x%x, Cnt=%d\n",
				pAd->RxRing[0].hw_desc_base, phy_addr, RX_RING_SIZE));

	/* Set DMA global configuration except TX_DMA_EN and RX_DMA_EN bits */
	AsicWaitPDMAIdle(pAd, 100, 1000);
	RTMP_IO_READ32(pAd, WPDMA_GLO_CFG, &GloCfg.word);
	GloCfg.word &= 0xff0;
	GloCfg.field.EnTXWriteBackDDONE = 1;
	RTMP_IO_WRITE32(pAd, WPDMA_GLO_CFG, GloCfg.word);
	
	IntCfg.word = 0;
	RTMP_IO_WRITE32(pAd, DELAY_INT_CFG, IntCfg.word);
}

