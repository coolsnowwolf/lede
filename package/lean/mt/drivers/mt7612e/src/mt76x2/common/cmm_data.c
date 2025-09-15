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
	cmm_data.c
 
    Abstract:
 
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

 
#include "rt_config.h"

#ifdef LED_SOFT_SUPPORT
#include <ralink/ralink_gpio.h>
#endif

#define IS_MULTICAST_MAC_ADDR(Addr)			((((Addr[0]) & 0x01) == 0x01) && ((Addr[0]) != 0xff))
#define IS_BROADCAST_MAC_ADDR(Addr)			((((Addr[0]) & 0xff) == 0xff))

UCHAR	SNAP_802_1H[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00};
UCHAR	SNAP_BRIDGE_TUNNEL[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0xf8};
UCHAR	EAPOL[] = {0x88, 0x8e};
UCHAR   TPID[] = {0x81, 0x00}; /* VLAN related */

UCHAR	IPX[] = {0x81, 0x37};
UCHAR	APPLE_TALK[] = {0x80, 0xf3};


//  UserPriority To AccessCategory mapping
UCHAR WMM_UP2AC_MAP[8] = {QID_AC_BE, QID_AC_BK,
							QID_AC_BK, QID_AC_BE, 
							QID_AC_VI, QID_AC_VI,
							QID_AC_VO, QID_AC_VO};

#ifdef DBG
VOID dump_rxinfo(RTMP_ADAPTER *pAd, RXINFO_STRUC *pRxInfo)
{
	hex_dump("RxInfo Raw Data", (UCHAR *)pRxInfo, sizeof(RXINFO_STRUC));

	DBGPRINT(RT_DEBUG_OFF, ("RxInfo Fields:\n"));

	DBGPRINT(RT_DEBUG_OFF, ("\tBA=%d\n", pRxInfo->BA));
	DBGPRINT(RT_DEBUG_OFF, ("\tDATA=%d\n", pRxInfo->DATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tNULLDATA=%d\n", pRxInfo->NULLDATA));
	DBGPRINT(RT_DEBUG_OFF, ("\tFRAG=%d\n", pRxInfo->FRAG));
	DBGPRINT(RT_DEBUG_OFF, ("\tU2M=%d\n", pRxInfo->U2M));
	DBGPRINT(RT_DEBUG_OFF, ("\tMcast=%d\n", pRxInfo->Mcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tBcast=%d\n", pRxInfo->Bcast));
	DBGPRINT(RT_DEBUG_OFF, ("\tMyBss=%d\n", pRxInfo->MyBss));
	DBGPRINT(RT_DEBUG_OFF, ("\tCrc=%d\n", pRxInfo->Crc));
	DBGPRINT(RT_DEBUG_OFF, ("\tCipherErr=%d\n", pRxInfo->CipherErr));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMSDU=%d\n", pRxInfo->AMSDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tHTC=%d\n", pRxInfo->HTC));
	DBGPRINT(RT_DEBUG_OFF, ("\tRSSI=%d\n", pRxInfo->RSSI));
	DBGPRINT(RT_DEBUG_OFF, ("\tL2PAD=%d\n", pRxInfo->L2PAD));
	DBGPRINT(RT_DEBUG_OFF, ("\tAMPDU=%d\n", pRxInfo->AMPDU));
	DBGPRINT(RT_DEBUG_OFF, ("\tDecrypted=%d\n", pRxInfo->Decrypted));
	DBGPRINT(RT_DEBUG_OFF, ("\tBssIdx3=%d\n", pRxInfo->BssIdx3));
	DBGPRINT(RT_DEBUG_OFF, ("\twapi_kidx=%d\n", pRxInfo->wapi_kidx));
	DBGPRINT(RT_DEBUG_OFF, ("\tpn_len=%d\n", pRxInfo->pn_len));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type0=%d\n", pRxInfo->action_wanted));
	DBGPRINT(RT_DEBUG_OFF, ("\tsw_fc_type1=%d\n", pRxInfo->sw_fc_type1));
	DBGPRINT(RT_DEBUG_OFF, ("\tprobe_rsp=%d\n", pRxInfo->probe_rsp));
	DBGPRINT(RT_DEBUG_OFF, ("\tbeacon=%d\n", pRxInfo->beacon));
	DBGPRINT(RT_DEBUG_OFF, ("\tdisasso=%d\n", pRxInfo->disasso));
	DBGPRINT(RT_DEBUG_OFF, ("\tdeauth=%d\n", pRxInfo->deauth));
	DBGPRINT(RT_DEBUG_OFF, ("\taction_wanted=%d\n", pRxInfo->action_wanted));
	DBGPRINT(RT_DEBUG_OFF, ("\trsv=%d\n", pRxInfo->rsv));	

#ifdef RTMP_MAC
	DBGPRINT(RT_DEBUG_OFF, ("\t"));
#endif /* RTMP_MAC */
}


VOID dump_txinfo(RTMP_ADAPTER *pAd, TXINFO_STRUC *pTxInfo)
{
	hex_dump("TxInfo Raw Data: ", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
	
	DBGPRINT(RT_DEBUG_OFF, ("TxInfo Fields:\n"));

#ifdef RLT_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_txinfo(pAd, pTxInfo);
#endif
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif /* RTMP_MAC */
}


VOID dump_txwi(RTMP_ADAPTER *pAd, TXWI_STRUC *pTxWI)
{
	hex_dump("TxWI Raw Data: ", (UCHAR *)pTxWI, pAd->chipCap.TXWISize);

	DBGPRINT(RT_DEBUG_OFF, ("TxWI Fields:\n"));
#ifdef RLT_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_txwi(pAd, pTxWI);
#endif
#endif /* RLT_MAC */

#ifdef RTMP_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RTMP)
		dump_rtmp_txwi(pAd, pTxWI);
#endif
#endif /* RTMP_MAC */
}


VOID dump_rxwi(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI)
{
	hex_dump("RxWI Raw Data", (UCHAR *)pRxWI, pAd->chipCap.RXWISize);

	DBGPRINT(RT_DEBUG_OFF, ("RxWI Fields:\n"));
#ifdef RLT_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RLT)
		dump_rlt_rxwi(pAd, pRxWI);	
#endif
#endif /* RLT_MAC */

#ifdef RTMP_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RTMP)
		dump_rtmp_rxwi(pAd, pRxWI);
#endif
#endif /* RTMP_MAC */
}
#endif /* DBG */

#ifdef DBG_DIAGNOSE
static VOID dump_txblk(TX_BLK *pTxBlk)
{
	NDIS_PACKET *pPacket;
	int i, frameNum;
	PQUEUE_ENTRY	pQEntry;

	DBGPRINT(RT_DEBUG_TRACE,("Dump TX_BLK Structure:\n"));
	DBGPRINT(RT_DEBUG_TRACE,("\tTxFrameType=%d!\n", pTxBlk->TxFrameType));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFrameLen=%d\n", pTxBlk->TotalFrameLen));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFrameNum=%d!\n", pTxBlk->TxPacketList.Number));
	DBGPRINT(RT_DEBUG_TRACE,("\tTotalFragNum=%d!\n", pTxBlk->TotalFragNum));
	DBGPRINT(RT_DEBUG_TRACE,("\tpPacketList=\n"));

	frameNum = pTxBlk->TxPacketList.Number;

	for(i=0; i < frameNum; i++)
	{	int j;
		UCHAR	*pBuf;

		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (pPacket)
		{
			pBuf = GET_OS_PKT_DATAPTR(pPacket);
			DBGPRINT(RT_DEBUG_TRACE,("\t\t[%d]:ptr=0x%x, Len=%d!\n", i, (UINT32)(GET_OS_PKT_DATAPTR(pPacket)), GET_OS_PKT_LEN(pPacket)));
			DBGPRINT(RT_DEBUG_TRACE,("\t\t"));
			for (j =0 ; j < GET_OS_PKT_LEN(pPacket); j++)
			{
				DBGPRINT(RT_DEBUG_TRACE,("%02x ", (pBuf[j] & 0xff)));
				if (j == 16)
					break;
			}
			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
		}
	}
	DBGPRINT(RT_DEBUG_TRACE,("\tWcid=%d!\n", pTxBlk->Wcid));
	DBGPRINT(RT_DEBUG_TRACE,("\tapidx=%d!\n", pTxBlk->apidx));
	DBGPRINT(RT_DEBUG_TRACE,("----EndOfDump\n"));
}

VOID dump_rxblk(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	DBGPRINT(RT_DEBUG_TRACE,("Dump RX_BLK Structure:\n"));

	DBGPRINT(RT_DEBUG_TRACE,("\tHW rx info:\n"));
	hex_dump("RawData", &pRxBlk->hw_rx_info[0], RXD_SIZE);
	
	DBGPRINT(RT_DEBUG_TRACE,("\tData Pointer info:\n"));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tpRxInfo=0x%p\n", pRxBlk->pRxInfo));
	dump_rxinfo(pAd, pRxBlk->pRxInfo);
#ifdef RLT_MAC
#ifdef DBG
	if (pAd->chipCap.hif_type == HIF_RLT) {
		DBGPRINT(RT_DEBUG_TRACE,("\t\tpRxFceInfo=0x%p\n", pRxBlk->pRxFceInfo));
		dumpRxFCEInfo(pAd, pRxBlk->pRxFceInfo);
	}
#endif
#endif /* RLT_MAC */
	DBGPRINT(RT_DEBUG_TRACE,("\t\tpRxWI=0x%p\n", pRxBlk->pRxWI));
	dump_rxwi(pAd, pRxBlk->pRxWI);
	DBGPRINT(RT_DEBUG_TRACE,("\t\tpRxPacket=0x%p, len=%d\n", pRxBlk->pRxPacket, pRxBlk->MPDUtotalByteCnt));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tpHeader=0x%p\n", pRxBlk->pHeader));
	hex_dump("Dump RxPacket in dump_rxblk", (UCHAR *)pRxBlk->pHeader, pRxBlk->MPDUtotalByteCnt > 48 ? 48 : pRxBlk->MPDUtotalByteCnt);
	DBGPRINT(RT_DEBUG_TRACE,("\t\tpData=0x%p\n", pRxBlk->pData));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tDataSize=%d\n", pRxBlk->DataSize));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tFlags=0x%x\n", pRxBlk->Flags));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tUserPriority=%d\n", pRxBlk->UserPriority));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tOpMode=%d\n", pRxBlk->OpMode));

	DBGPRINT(RT_DEBUG_TRACE,("\tMirror Info from RXWI:\n"));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tWCID=%d\n", pRxBlk->wcid));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tPhyMode=%d(%s)\n", pRxBlk->rx_rate.field.MODE, get_phymode_str(pRxBlk->rx_rate.field.MODE)));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tMCS=%d\n", pRxBlk->rx_rate.field.MCS));
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		DBGPRINT(RT_DEBUG_TRACE,("\t\tldpc=%d\n", pRxBlk->rx_rate.field.ldpc));
#endif /* RLT_MAC */
	DBGPRINT(RT_DEBUG_TRACE,("\t\tBW=%d\n", pRxBlk->rx_rate.field.BW));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tSGI=%d\n", pRxBlk->rx_rate.field.ShortGI));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tMPDUtotalByteCnt=%d\n", pRxBlk->MPDUtotalByteCnt));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tTID=%d\n", pRxBlk->TID));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tSTBC=%d\n", pRxBlk->rx_rate.field.STBC));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tkey_idx=%d\n", pRxBlk->key_idx));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tBSS_IDX=%d\n", pRxBlk->bss_idx));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tRSSI=%d:%d:%d\n", pRxBlk->rssi[0], pRxBlk->rssi[1], pRxBlk->rssi[2]));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tSNR=%d:%d:%d\n", pRxBlk->snr[0], pRxBlk->snr[1], pRxBlk->snr[2]));
	DBGPRINT(RT_DEBUG_TRACE,("\t\tFreqOffset=%d\n", pRxBlk->freq_offset));
}
#endif /* DBG_DIAGNOSE */


#ifdef DOT11_N_SUPPORT
VOID RTMP_BASetup(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UINT8 UPriority)
{
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pEntry && (pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry))
		{
			BOOLEAN isRalink = FALSE;
			/* Don't care the status of the portSecured status. */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pEntry))
			{
				if (pEntry->apidx < MAX_APCLI_NUM) {
					if (pAd->ApCfg.ApCliTab[pEntry->apidx].MlmeAux.APRalinkIe != 0)
					isRalink = TRUE;
				}
			}
#endif /* APCLI_SUPPORT */

			if (((pEntry->TXBAbitmap & (1<<UPriority)) == 0) 
				&& (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
				&& ((IS_ENTRY_CLIENT(pEntry) && CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET)) || 
					 IS_ENTRY_MESH(pEntry) || IS_ENTRY_WDS(pEntry) ||
				 	 (IS_ENTRY_APCLI(pEntry) && (isRalink == TRUE) && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)) || 
				 	 (pEntry->WepStatus == Ndis802_11WEPDisabled ||
						pEntry->WepStatus == Ndis802_11AESEnable
#ifdef WAPI_SUPPORT
						|| pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled
#endif /* WAPI_SUPPORT */
						))
				)
			{
				BAOriSessionSetUp(pAd, pEntry, UPriority, 0, 10, FALSE);
			}
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if ((pAd->CommonCfg.BACapability.field.AutoBA == TRUE) && (pEntry->NoBADataCountDown == 0) && IS_HT_STA(pEntry))
		{
			if (((pEntry->TXBAbitmap & (1 << UPriority)) == 0) &&
			    ((pEntry->BADeclineBitmap & (1 << UPriority)) == 0) &&
			    (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			    &&
			    ((IS_ENTRY_CLIENT(pEntry) && pAd->MlmeAux.APRalinkIe != 0x0) || (pEntry->WepStatus != Ndis802_11WEPEnabled && pEntry->WepStatus != Ndis802_11TKIPEnable))
			    &&
			    (!(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS)))
			    )
			{
				BAOriSessionSetUp(pAd, pEntry, UPriority, 0, 10, FALSE);
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

}
#endif /* DOT11_N_SUPPORT */


/*
	========================================================================

	Routine Description:
		API for MLME to transmit management frame to AP (BSS Mode)
	or station (IBSS Mode)
		
	Arguments:
		pAd Pointer to our adapter
		pData		Pointer to the outgoing 802.11 frame
		Length		Size of outgoing management frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
NDIS_STATUS MiniportMMRequest(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN UCHAR *pData,
	IN UINT Length)
{
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	ULONG FreeNum;
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags = 0;
#endif /* RTMP_MAC_PCI */
	BOOLEAN bUseDataQ = FALSE, FlgDataQForce = FALSE, FlgIsLocked = FALSE;
	int retryCnt = 0;
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
	PHEADER_802_11 pHdr80211 = NULL;
	MULTISSID_STRUCT *pMbss = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif // MBSS_802_11_STATISTICS //
#endif
	BOOLEAN FlgIsCheckPS = FALSE;

	ASSERT(Length <= MGMT_DMA_BUFFER_SIZE);

	if ((QueIdx & MGMT_USE_QUEUE_FLAG) == MGMT_USE_QUEUE_FLAG)
	{
		bUseDataQ = TRUE;
		QueIdx &= (~MGMT_USE_QUEUE_FLAG);
	}

#ifdef CONFIG_FPGA_MODE
	if (pAd->fpga_ctl.fpga_on & 0x1) {
		if (pAd->fpga_ctl.tx_kick_cnt > 0) {
			if (pAd->fpga_ctl.tx_kick_cnt < 0xffff) 
				pAd->fpga_ctl.tx_kick_cnt--;
		}
		else
			return NDIS_STATUS_FAILURE;
		
		QueIdx = 0;
		bUseDataQ = TRUE;
	}
#endif /* CONFIG_FPGA_MODE */
	if ((QueIdx & MGMT_USE_PS_FLAG) == MGMT_USE_PS_FLAG)
	{
		FlgIsCheckPS = TRUE;
		QueIdx &= (~MGMT_USE_PS_FLAG);
	}

#ifdef RTMP_MAC_PCI
	if (pAd->MACVersion == 0x28600100)
	{
		/* do not care about the version */
		QueIdx = (bUseDataQ ==TRUE ? QueIdx : 3);
		bUseDataQ = TRUE;
	}

	if (bUseDataQ)
	{
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		FlgIsLocked = TRUE;
		retryCnt = MAX_DATAMM_RETRY;		
	}
#endif /* RTMP_MAC_PCI */
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
	// get MBSS for management frame TX counter per BSS
	pHdr80211 = (PHEADER_802_11) pData;
	pEntry = MacTableLookup(pAd, pHdr80211->Addr1);
	if (pEntry != NULL && (pEntry->apidx < pAd->ApCfg.BssidNum))
		pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];
	else
	{
		UCHAR apidx;
		for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
		{
			if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev == NULL) || ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
				!(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))))
				{
					/* the interface is down */
					continue;
				}
			if(RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pHdr80211->Addr2,MAC_ADDR_LEN))
			{
				pMbss = &pAd->ApCfg.MBSSID[apidx];
			}
		}
	}
#endif
#endif

	do
	{
		/* Reset is in progress, stop immediately*/
		if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST |
								fRTMP_ADAPTER_RADIO_OFF)) ||
			 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)
#ifdef RT_CFG80211_P2P_SUPPORT
			|| IS_CFG80211_P2P_ABSENCE(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
			)
		{
			Status = NDIS_STATUS_FAILURE;
			break;
		}

#ifdef CONFIG_STA_SUPPORT
#endif /* CONFIG_STA_SUPPORT */

		/* Check Free priority queue*/
		/* Since we use PBF Queue2 for management frame.  Its corresponding DMA ring should be using TxRing.*/
#ifdef RTMP_MAC_PCI
		if (bUseDataQ)
		{
			FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
			if (FreeNum <= 5)
			{
				/* free Tx(QueIdx) resources*/
				RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
			}
			FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
		}
		else
#endif /* RTMP_MAC_PCI */
		{
			FreeNum = GET_MGMTRING_FREENO(pAd);
		}

		if ((FreeNum > 0))
		{
			INT hw_len = TXINFO_SIZE + pAd->chipCap.TXWISize + TSO_SIZE;
			UCHAR rtmpHwHdr[40];

			ASSERT((sizeof(rtmpHwHdr) > hw_len));

			/* We need to reserve space for rtmp hardware header. i.e., TxWI for RT2860 and TxInfo+TxWI for RT2870*/
			NdisZeroMemory(&rtmpHwHdr, hw_len);
			Status = RTMPAllocateNdisPacket(pAd, &pPacket, (PUCHAR)&rtmpHwHdr[0], hw_len, pData, Length);
			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_WARN, ("MiniportMMRequest (error:: can't allocate NDIS PACKET)\n"));
				break;
			}

#ifdef DOT11W_PMF_SUPPORT
#ifdef CONFIG_STA_SUPPORT
			if (INFRA_ON(pAd))
#endif /* CONFIG_STA_SUPPORT */
				PMF_PerformTxFrameAction(pAd, pPacket);
#endif /* DOT11W_PMF_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
#ifdef UAPSD_SUPPORT
			UAPSD_MR_QOS_NULL_HANDLE(pAd, pData, pPacket);
#endif /* UAPSD_SUPPORT */
#else
#ifdef CONFIG_AP_SUPPORT
#ifdef UAPSD_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
      		if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd))
#else
            IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
			{
				UAPSD_MR_QOS_NULL_HANDLE(pAd, pData, pPacket);
			}
#endif /* UAPSD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				FlgDataQForce = TRUE;
				retryCnt--;
			}
#endif /* RTMP_MAC_PCI */

			Status = MlmeHardTransmit(pAd, QueIdx, pPacket, FlgDataQForce, FlgIsLocked, FlgIsCheckPS);			
			if (Status == NDIS_STATUS_SUCCESS)
				retryCnt = 0;
			else
				RELEASE_NDIS_PACKET(pAd, pPacket, Status);
		}
		else
		{
			pAd->RalinkCounters.MgmtRingFullCount++;
#ifdef RTMP_MAC_PCI
			if (bUseDataQ)
			{
				retryCnt--;
				DBGPRINT(RT_DEBUG_TRACE, ("retryCnt %d\n", retryCnt));
				if (retryCnt == 0)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in DataRing, MgmtRingFullCount=%ld!\n",
											QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
				}
			}
#endif /* RTMP_MAC_PCI */
			DBGPRINT(RT_DEBUG_ERROR, ("Qidx(%d), not enough space in MgmtRing, MgmtRingFullCount=%ld!\n",
										QueIdx, pAd->RalinkCounters.MgmtRingFullCount));
		}
	} while (retryCnt > 0);
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
	// management frame TX counter per BSS
	if (pMbss != NULL)
	{
		if (Status == NDIS_STATUS_SUCCESS)
		{
			pMbss->MGMTTxCount++;
			pMbss->MGMTTransmittedByteCount += Length;
		}
		else
		{
			pMbss->MGMTTxDropCount++;
			pMbss->MGMTTxErrorCount++;
		}
	}
#endif
#endif

	
#ifdef RTMP_MAC_PCI
	if (bUseDataQ)
		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */

	return Status;
}


#ifdef CONFIG_AP_SUPPORT
/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function

	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		FlgIsDeltsFrame 1: the frame is a DELTS frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	Note:

	========================================================================
*/
void AP_QueuePsActionPacket(
	IN	PRTMP_ADAPTER	pAd,
	IN	MAC_TABLE_ENTRY	*pMacEntry,
	IN	PNDIS_PACKET	pPacket,
	IN	BOOLEAN			FlgIsDeltsFrame,
	IN	BOOLEAN			FlgIsLocked,
	IN	UCHAR			MgmtQid)
{
#ifdef UAPSD_SUPPORT
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
	PNDIS_PACKET DuplicatePkt = NULL;
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
#endif /* UAPSD_SUPPORT */

	/* Note: for original mode of 4 AC are UAPSD, if station want to change
			the mode of a AC to legacy PS, we dont know where to put the
			response;
			1. send the response;
			2. but the station is in ps mode, so queue the response;
			3. we should queue the reponse to UAPSD queue because the station
				is not yet change its mode to legacy ps AC;
			4. so AP should change its mode to legacy ps AC only when the station
				sends a trigger frame and we send out the reponse;
			5. the mechanism is too complicate; */

#ifdef UAPSD_SUPPORT
	/*
		If the frame is action frame and the VO is UAPSD, we can not send the
		frame to VO queue, we need to send to legacy PS queue; or the frame
		maybe not got from QSTA.
	*/
/*    if ((pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid]) &&*/
/*		(FlgIsDeltsFrame == 0))*/
	if (pMacEntry->bAPSDDeliverEnabledPerAC[MgmtQid])
	{
		/* queue the management frame to VO queue if VO is deliver-enabled */
		DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to UAPSD queue %d ... (IsDelts: %d)\n",
				MgmtQid, FlgIsDeltsFrame));

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
		if (!pMacEntry->bAPSDAllAC)
		{
			/* duplicate one packet to legacy PS queue */
			RTMP_SET_PACKET_UAPSD(pPacket, 0, MgmtQid);
			DuplicatePkt = RTMP_DUPLICATE_PACKET(pAd, pPacket, pMacEntry->apidx);
		}
		else
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		{
			RTMP_SET_PACKET_UAPSD(pPacket, 1, MgmtQid);
		}

		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, MgmtQid);

		if (pMacEntry->bAPSDAllAC)
		{
			/* mark corresponding TIM bit in outgoing BEACON frame*/
			WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
		}
		else
		{
#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
			/* duplicate one packet to legacy PS queue */

			/*
				Sometimes AP will send DELTS frame to STA but STA will not
				send any trigger frame to get the DELTS frame.
				We must force to send it so put another one in legacy PS
				queue.
			*/
			if (DuplicatePkt != NULL)
			{
				pPacket = DuplicatePkt;
				goto Label_Legacy_PS;
			}
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		}
	}
	else
#endif /* UAPSD_SUPPORT */
	{
		/* DuplicatePkt = DuplicatePacket(get_netdev_from_bssid(pAd, pMacEntry->apidx), pPacket, pMacEntry->apidx);*/

#ifdef UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY
Label_Legacy_PS:
#endif /* UAPSD_CC_FUNC_PS_MGMT_TO_LEGACY */
		if (pMacEntry->PsQueue.Number >= MAX_PACKETS_IN_PS_QUEUE)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_RESOURCES);
			return;
		}
		else
		{
			ULONG IrqFlags=0;

			DBGPRINT(RT_DEBUG_TRACE, ("ps> mgmt to legacy ps queue... (%d)\n", FlgIsDeltsFrame));

			if (FlgIsLocked == FALSE)
				RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
			InsertTailQueue(&pMacEntry->PsQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
			if (FlgIsLocked == FALSE)
				RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
		}

		/* mark corresponding TIM bit in outgoing BEACON frame*/
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->apidx, pMacEntry->Aid);
	}
}
#endif /* CONFIG_AP_SUPPORT */


/*
	========================================================================

	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set
	appropriate ASIC register to kick hardware transmit function
	
	Arguments:
		pAd Pointer to our adapter
		pBuffer 	Pointer to	memory of outgoing frame
		Length		Size of outgoing management frame
		
	Return Value:
		NDIS_STATUS_FAILURE
		NDIS_STATUS_PENDING
		NDIS_STATUS_SUCCESS

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
NDIS_STATUS MlmeHardTransmit(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket,
	IN BOOLEAN FlgDataQForce,
	IN BOOLEAN FlgIsLocked,
	IN	BOOLEAN			FlgIsCheckPS)
{
#ifdef CONFIG_AP_SUPPORT
	MAC_TABLE_ENTRY *pEntry = NULL;
	HEADER_802_11 *pHeader_802_11;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#endif /* CONFIG_AP_SUPPORT */
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;

	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef CONFIG_AP_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif /* CONFIG_AP_SUPPORT */
#endif /* CARRIER_DETECTION_SUPPORT */
		)
	{
		return NDIS_STATUS_FAILURE;
	}

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if (pSrcBufVA == NULL)
		return NDIS_STATUS_FAILURE;

#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TXWISize + TSO_SIZE);

	/*
		Section 11.2.1.1 STA Power Management modes of IEEE802.11-2007:
		The Power Managment bit shall not be set in any management frame,
		except an Action frame.

		So in the 'baseline' test plan
		(Wi-Fi 802.11 WPA2, WPA, WEP Interoperability Test Plan),
		Section 2.2.6, the following Requirement:
        APs shall ignore the power save bit in any received Authenticate and
		(Re) Associate, and shall assume that the station is awake for the
		response.
	*/

	/*
		IEEE802.11, 11.2.1.4 AP operation during the contention period f)
		A single buffered MSDU or management frame for a STA in the PS mode shall
		be forwarded to the STA after a PS-Poll has been received from that STA.
		The More Data field shall be set to indicate the presence of further
		buffered MSDUs or "management frames" for the polling STA.
	*/

	/*
		IEEE802.11e, 11.2.1.4 Power management with APSD,
		An unscheduled SP ends after the QAP has attempted to transmit at least
		one MSDU or MMPDU associated with a delivery-enabled AC and destined for
		the non-AP QSTA, but no more than the number indicated in the Max SP
		Length field if the field has a nonzero value.
	*/

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA) ||
		(pHeader_802_11->FC.Type == FC_TYPE_MGMT))
	{
		if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) || (pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL))		
			pEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
	if ((pEntry != NULL)
		&& (pEntry->apidx < pAd->ApCfg.BssidNum)
		&& IS_RRM_QUIET(pAd, pEntry->apidx))
	{
		return NDIS_STATUS_FAILURE;
	}
#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */


	if ((pEntry != NULL) &&
		(pEntry->PsMode == PWR_SAVE) &&
		(((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_DATA_NULL) &&
			(pHeader_802_11->FC.SubType != SUBTYPE_QOS_NULL)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION)) ||
		((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
		(pHeader_802_11->FC.SubType == SUBTYPE_ACTION_NO_ACK)) ||
		(FlgIsCheckPS == 1)))			
	{
		/* the peer is in PS mode, we need to queue the management frame */
		UINT8 FlgIsDeltsFrame = 0, MgmtQid = QID_AC_VO;

		/*
			1. Data & Not QoS Null, or
			2. Management & Action, or
			3. Management & Action No ACK;
		*/
		DBGPRINT(RT_DEBUG_TRACE, ("STA in ps mode, queue the mgmt frame\n"));
		RTMP_SET_PACKET_WCID(pPacket, pEntry->wcid);
		RTMP_SET_PACKET_MGMT_PKT(pPacket, 1); /* is management frame */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 0); /* default to management queue */


#ifdef RT_CFG80211_P2P_SUPPORT
		if(pEntry->wdev->wdev_type == WDEV_TYPE_AP)
		{
			RTMP_SET_PACKET_NET_DEVICE_MBSSID(pPacket, MAIN_MBSSID);
			RTMP_SET_PACKET_OPMODE(pPacket, OPMODE_AP);
		}
#endif /* RT_CFG80211_P2P_SUPPORT */
		
		if (FlgDataQForce == TRUE)
			RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pPacket, 1); /* force to data queue */

		if ((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_ACTION))
		{
			FRAME_ADDBA_REQ *pFrameBa = (FRAME_ADDBA_REQ *)pHeader_802_11;
			if (pFrameBa->Category == CATEGORY_BA)
				MgmtQid = QueIdx;
		}
#ifdef CONFIG_HOTSPOT_R2
		if (((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC)) || 
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH)))
		{
			RTMP_SET_PACKET_DISASSOC(pPacket, 1);
			pEntry->IsKeep = 1;
		}
#endif /* CONFIG_HOTSPOT_R2 */

		AP_QueuePsActionPacket(pAd, pEntry, pPacket, FlgIsDeltsFrame,
								FlgIsLocked, MgmtQid);
		return NDIS_STATUS_SUCCESS;
	}
    else
#endif /* CONFIG_AP_SUPPORT */
    {
#ifdef RTMP_MAC_PCI
    	if (FlgDataQForce == TRUE)
    		return MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
    	else
#endif /* RTMP_MAC_PCI */
    		return MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);
    }
}


NDIS_STATUS MlmeHardTransmitMgmtRing(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	HEADER_802_11 *pHeader_802_11;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate;
	TXWI_STRUC *pFirstTxWI;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	UCHAR PID, wcid/*, tx_rate*/;
	HTTRANSMIT_SETTING *transmit;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	/* Make sure MGMT ring resource won't be used by other threads*/
	RTMP_SEM_LOCK(&pAd->MgmtRingLock);
	if (pSrcBufVA == NULL)
	{
		/* The buffer shouldn't be NULL*/
			RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* outgoing frame always wakeup PHY to prevent frame lost*/
		if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE))
			AsicForceWakeup(pAd, TRUE);
	}
#endif /* CONFIG_STA_SUPPORT */

	pFirstTxWI = (TXWI_STRUC *)(pSrcBufVA +  TXINFO_SIZE);
	pHeader_802_11 = (PHEADER_802_11) (pSrcBufVA + TXINFO_SIZE + TSO_SIZE + TXWISize);
	
	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;
	
	/* Verify Mlme rate for a / g bands.*/
	if ((pAd->LatchRfRegs.Channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;
	
	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
	    (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)) ||
	    ((pHeader_802_11->FC.Type == FC_TYPE_CNTL) &&
	    (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)))
	{
		pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
	}

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		/* Fixed W52 with Activity scan issue in ABG_MIXED and ABGN_MIXED mode.*/
		// TODO: shiang-6590, why we need this condition check here?
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A | WMODE_B | WMODE_G)
#ifdef DOT11_N_SUPPORT
			|| WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_A | WMODE_B | WMODE_G | WMODE_AN | WMODE_GN)
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
			|| WMODE_CAP(pAd->CommonCfg.PhyMode, WMODE_AC)
#endif /* DOT11_VHT_AC*/
		)
		{
			if (pAd->LatchRfRegs.Channel > 14)
			{
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
				pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_RATE_6;
			}
			else
			{
				pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
				pAd->CommonCfg.MlmeTransmit.field.MCS = MCS_0;
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */

//CFG_TODO YF


	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		{
			/* We are in scan progress, just let the PwrMgmt bit keep as it orginally should be.*/
		}
		else
#endif /* CONFIG_STA_SUPPORT */
			pHeader_802_11->FC.PwrMgmt = PWR_ACTIVE;
#ifdef CONFIG_STA_SUPPORT
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	/*
		In WMM-UAPSD, mlme frame should be set psm as power saving but probe 
		request frame, Data-Null packets alse pass through MMRequest in RT2860,
		however, we hope control the psm bit to pass APSD
	*/
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if ((pHeader_802_11->FC.SubType == SUBTYPE_ACTION) ||
			((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
			((pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL) ||
			(pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL))))
		{
			if (RtmpPktPmBitCheck(pAd) == TRUE)
				pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
			else if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_MEDIA_STATE_CONNECTED) && 
					INFRA_ON(pAd) && 
					RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
			{
				/* We are in scan progress, just let the PwrMgmt bit keep as it orginally should be */
			}
			else
			{
				pHeader_802_11->FC.PwrMgmt = pAd->CommonCfg.bAPSDForcePowerSave;
			}
		}
	}
#endif /* CONFIG_STA_SUPPORT */
	

#ifdef CONFIG_AP_SUPPORT
	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(pPacket);
#endif /* CONFIG_AP_SUPPORT */


	bInsertTimestamp = FALSE;
	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) /* must be PS-POLL*/
	{
#ifdef CONFIG_STA_SUPPORT
		/*Set PM bit in ps-poll, to fix WLK 1.2  PowerSaveMode_ext failure issue.*/
		if ((pAd->OpMode == OPMODE_STA) && (pHeader_802_11->FC.SubType == SUBTYPE_PS_POLL))
		{
			pHeader_802_11->FC.PwrMgmt = PWR_SAVE;
		}
#endif /* CONFIG_STA_SUPPORT */
		bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT
		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA)
		{
			pHeader_802_11->Duration = 100;
			//DBGPRINT(RT_DEBUG_OFF, ("%s(): VHT_NDPA frame, rate=%d, len=%d, duration=%d\n",
			//			__FUNCTION__, MlmeRate, SrcBufLen, pHeader_802_11->Duration));
			//hex_dump("VHT_NDPA after update Duration", (UCHAR *)pHeader_802_11, SrcBufLen);
			
		}
#endif /* VHT_TXBF_SUPPORT */
	}
	else /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
	{
		if (pHeader_802_11->Addr1[0] & 0x01) /* MULTICAST, BROADCAST*/
		{
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		}
		else
		{
#ifdef SOFT_SOUNDING
			if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
				&& pMacEntry && (pMacEntry->snd_reqired == TRUE))
			{
				bAckRequired = FALSE;
				pHeader_802_11->Duration = 0;
			}
			else
#endif /* SOFT_SOUNDING */
			{
				bAckRequired = TRUE;
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);
				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bInsertTimestamp = TRUE;
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
#ifdef CONFIG_AP_SUPPORT
#ifdef SPECIFIC_TX_POWER_SUPPORT
					{
						/* Find which MBSSID to be send this probeRsp */
						UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

						if ( !(apidx >= pAd->ApCfg.BssidNum) && 
						     (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) /* &&
						     (pAd->CommonCfg.MlmeTransmit.field.MODE == MODE_CCK) &&
						     (pAd->CommonCfg.MlmeTransmit.field.MCS == RATE_1)*/)
						{
							TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;
						}
					}
#endif /* SPECIFIC_TX_POWER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT))
				{
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				}
				else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) && 
						 (MacTableLookup(pAd, pHeader_802_11->Addr1) == NULL))
				{
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;
	if (pAd->Sequence >0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pAd->Dot11_H.RDMode != RD_NORMAL_MODE))
	{
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
		return NDIS_STATUS_FAILURE;
	}

#ifdef RT_BIG_ENDIAN
	RTMPFrameEndianChange(pAd, (PUCHAR)pHeader_802_11, DIR_WRITE, FALSE);
#endif

	
	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size
	*/
	

	/*
		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement
	*/
	/*pAd->CommonCfg.MlmeTransmit.field.MODE = 1;*/
	
	/*
		management frame doesn't need encryption. 
		so use RESERVED_WCID no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;


	if (pMacEntry == NULL)
	{
		wcid = RESERVED_WCID;
		/*tx_rate = (UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS;*/
		transmit = &pAd->CommonCfg.MlmeTransmit;
#ifdef VHT_TXBF_SUPPORT
		if (pAd->NDPA_Request)
		{
			transmit->field.MODE = MODE_VHT;
			transmit->field.MCS = MCS_RATE_6;
		}
#endif
	}
	else
	{
//#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		/* P2P Test Case 6.1.12, only OFDM rate can be captured by sniffer */
		if(
#ifdef RT_CFG80211_P2P_SUPPORT
		   (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&	
#endif /* RT_CFG80211_P2P_SUPPORT */
		   ((pHeader_802_11->FC.Type == FC_TYPE_DATA) &&
		   (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL)))
		{

			HTTRANSMIT_SETTING NullFramePhyMode;
			NdisZeroMemory(&NullFramePhyMode, sizeof(NullFramePhyMode));
			NdisCopyMemory(&NullFramePhyMode, &pMacEntry->HTPhyMode, sizeof(NullFramePhyMode));		
		
			DBGPRINT(RT_DEBUG_TRACE, ("%s:: Using Low Rate to send QOS NULL!!\n", __FUNCTION__));
			
			if ((pAd->LatchRfRegs.Channel > 14)
			)
			{
				NullFramePhyMode.field.MODE = MODE_OFDM;
				NullFramePhyMode.field.MCS = MCS_RATE_24;
			}
			else
			{
				NullFramePhyMode.field.MODE = MODE_CCK;
				NullFramePhyMode.field.MCS = MCS_1;
			}

			wcid = pMacEntry->wcid;
			/*tx_rate = (UCHAR)NullFramePhyMode.field.MCS;*/
			transmit = &NullFramePhyMode;			

		} 
		else 
//#endif /* P2P_SUPPORT || RT_CFG80211_SUPPORT */
		{
			/* dont use low rate to send QoS Null data frame */
			wcid = pMacEntry->wcid;
			/*tx_rate = (UCHAR)pMacEntry->MaxHTPhyMode.field.MCS;*/
			transmit = &pMacEntry->MaxHTPhyMode;
		}
	}

#ifdef SOFT_SOUNDING
	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE))
	{
		wcid = RESERVED_WCID;
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		transmit = &pMacEntry->snd_rate;
		
		RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE,
				0, wcid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), PID, 0,
				tx_rate, IFS_PIFS, transmit);
		pMacEntry->snd_reqired = FALSE;
		DBGPRINT(RT_DEBUG_OFF, ("%s():Kick Sounding to %02x:%02x:%02x:%02x:%02x:%02x, dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
					__FUNCTION__, PRINT_MAC(pMacEntry->Addr),
					get_phymode_str(transmit->field.MODE),
					get_bw_str(transmit->field.BW),
					(transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));
	}
	else
#endif /* SOFT_SOUNDING */
       	{   
#ifdef VHT_TXBF_SUPPORT	      	
     	    if ((pHeader_802_11->FC.Type == FC_TYPE_CNTL) && (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA))
	    {
	    	  UINT8 PrevMcs, PrevMode;

                PrevMcs = transmit->field.MCS;
                PrevMode = transmit->field.MODE;

                transmit->field.MODE = MODE_OFDM;
                transmit->field.MCS = MCS_RATE_24;
                RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, 
                            bAckRequired, FALSE, 0, wcid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), 
			    PID, 0, SNDG_TYPE_NDP, IFS_BACKOFF, transmit);

                transmit->field.MCS = PrevMcs;
                transmit->field.MODE = PrevMode;
            }
            else
#endif		    
            {
                //set tx_rate be zero for eBF with iPhone6 IOT issue
                RTMPWriteTxWI(pAd, pFirstTxWI, FALSE, FALSE, bInsertTimestamp, FALSE, bAckRequired, FALSE, 
				0, wcid, (SrcBufLen - TXINFO_SIZE - TXWISize - TSO_SIZE), PID, 0, 
				0, IFS_BACKOFF, transmit);
            }   

#ifdef SPECIFIC_TX_POWER_SUPPORT
#ifdef RTMP_MAC
		if ((IS_RT6352(pAd) || IS_MT76x2(pAd)) &&
			(pAd->chipCap.hif_type == HIF_RTMP) && (pMacEntry == NULL))
        	pFirstTxWI->TXWI_O.TxPwrAdj = TxPwrAdj;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
		if ((IS_RT6352(pAd) || IS_MT76x2(pAd)) &&
			(pAd->chipCap.hif_type == HIF_RLT) && (pMacEntry == NULL))
			pFirstTxWI->TXWI_N.TxPwrAdj = TxPwrAdj;
#endif /* RLT_MAC */
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	}

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef RT_BIG_ENDIAN
	RTMPWIEndianChange(pAd, (PUCHAR)pFirstTxWI, TYPE_TXWI);
#endif


	/* Now do hardware-depened kick out.*/
	HAL_KickOutMgmtTx(pAd, QueIdx, pPacket, pSrcBufVA, SrcBufLen);
#ifdef CONFIG_HOTSPOT_R2
	if (RTMP_GET_PACKET_DISASSOC(pPacket))
	{
		if (((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_DISASSOC)) ||
			((pHeader_802_11->FC.Type == FC_TYPE_MGMT) &&
			(pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH)))
		{
			pMacEntry = MacTableLookup(pAd, pHeader_802_11->Addr1);
		}
		if ((pMacEntry) && (pMacEntry->IsKeep == 1))
			MacTableDeleteEntry(pAd, pMacEntry->Aid, pMacEntry->Addr);	
	}
#endif /* CONFIG_HOTSPOT_R2 */

#ifdef CUSTOMER_DCC_FEATURE
	if(!(ApScanRunning(pAd)))
	{
		UINT32 Index, Length;
		HTTRANSMIT_SETTING HTSetting;
		MULTISSID_STRUCT *pMbss = NULL;
		NdisZeroMemory(&HTSetting, sizeof(HTTRANSMIT_SETTING));
		HTSetting.field.MODE = transmit->field.MODE;
		HTSetting.field.BW = transmit->field.BW;
		HTSetting.field.ShortGI = transmit->field.ShortGI;
		HTSetting.field.MCS = transmit->field.MCS;
		Length = SrcBufLen - TXINFO_SIZE - TXWISize;
		GetMultShiftFactorIndex(HTSetting, &Index);
		if ((pMacEntry != NULL) && (pMacEntry->apidx < pAd->ApCfg.BssidNum))
			pMbss = &pAd->ApCfg.MBSSID[pMacEntry->apidx];
		else
			{
				UCHAR apidx;
				for(apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
				{
					if ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev == NULL) || ((pAd->ApCfg.MBSSID[apidx].wdev.if_dev != NULL) &&
						!(RTMP_OS_NETDEV_STATE_RUNNING(pAd->ApCfg.MBSSID[apidx].wdev.if_dev))))
						{
							/* the interface is down */
							continue;
						}
					if(RTMPEqualMemory(pAd->ApCfg.MBSSID[apidx].wdev.bssid, pHeader_802_11->Addr2,MAC_ADDR_LEN))
					{
						pMbss = &pAd->ApCfg.MBSSID[apidx];
					}
				}
			}
		RTMPCalculateAPTxRxActivityTime(pAd, Index, Length, pMbss, pMacEntry);
	}
#endif

	/* Make sure to release MGMT ring resource*/
/*	if (!IrqState)*/
		RTMP_SEM_UNLOCK(&pAd->MgmtRingLock);
	return NDIS_STATUS_SUCCESS;
}


/********************************************************************************
		
	New DeQueue Procedures.

 ********************************************************************************/
#define DEQUEUE_LOCK(lock, bIntContext, IrqFlags) 				\
	RTMP_IRQ_LOCK((lock), IrqFlags)

#define DEQUEUE_UNLOCK(lock, bIntContext, IrqFlags)				\
	RTMP_IRQ_UNLOCK((lock), IrqFlags)


/*
	========================================================================
	Tx Path design algorithm:
		Basically, we divide the packets into four types, Broadcast/Multicast, 11N Rate(AMPDU, AMSDU, Normal), B/G Rate(ARALINK, Normal),
		Specific Packet Type. Following show the classification rule and policy for each kinds of packets.
				Classification Rule=>
					Multicast: (*addr1 & 0x01) == 0x01
					Specific : bDHCPFrame, bARPFrame, bEAPOLFrame, etc.
					11N Rate : If peer support HT
								(1).AMPDU  -- If TXBA is negotiated.
								(2).AMSDU  -- If AMSDU is capable for both peer and ourself.
											*). AMSDU can embedded in a AMPDU, but now we didn't support it.
								(3).Normal -- Other packets which send as 11n rate.
								
					B/G Rate : If peer is b/g only.
								(1).ARALINK-- If both of peer/us supprot Ralink proprietary Aggregation and the TxRate is large than RATE_6
								(2).Normal -- Other packets which send as b/g rate.
					Fragment:
								The packet must be unicast, NOT A-RALINK, NOT A-MSDU, NOT 11n, then can consider about fragment.
								
				Classified Packet Handle Rule=>
					Multicast:
								No ACK, 		pTxBlk->bAckRequired = FALSE;
								No WMM, 		pTxBlk->bWMM = FALSE;
								No piggyback,   pTxBlk->bPiggyBack = FALSE;
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;
					Specific :	Basically, for specific packet, we should handle it specifically, but now all specific packets are use
									the same policy to handle it.
								Force LowRate,  pTxBlk->bForceLowRate = TRUE;
								
					11N Rate :
								No piggyback,	pTxBlk->bPiggyBack = FALSE;
								
								(1).AMSDU
									pTxBlk->bWMM = TRUE;
								(2).AMPDU
									pTxBlk->bWMM = TRUE;
								(3).Normal
									
					B/G Rate :
								(1).ARALINK
									
								(2).Normal
	========================================================================
*/
#ifdef VHT_TXBF_SUPPORT
static UCHAR TxPktClassification(RTMP_ADAPTER *pAd, PNDIS_PACKET  pPacket, TX_BLK *pTxBlk, PUCHAR pTxSndgTypePerEntry)
#else
static UCHAR TxPktClassification(RTMP_ADAPTER *pAd, PNDIS_PACKET  pPacket, TX_BLK *pTxBlk)
#endif
{
	UCHAR TxFrameType = TX_UNKOWN_FRAME;
	UCHAR Wcid;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	Wcid = RTMP_GET_PACKET_WCID(pPacket);
	if (Wcid == MCAST_WCID)
	{	/* Handle for RA is Broadcast/Multicast Address.*/
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
	//CFG_TODO: sigh...
		if (pTxBlk->OpMode == OPMODE_AP)
#else
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */		
		{
			// TODO: shiang-6590, fix me!
			UCHAR apidx = RTMP_GET_PACKET_NET_DEVICE_MBSSID(pPacket);
			if (apidx < pAd->ApCfg.BssidNum)
				pTxBlk->wdev = &pAd->ApCfg.MBSSID[apidx].wdev;
			else {
				DBGPRINT(RT_DEBUG_ERROR, ("%s(%d): WARN! pTxBlk->wdev not assigned!\n", __FUNCTION__, __LINE__));
			}
		}
#endif /* CONFIG_AP_SUPPORT */
		return TX_MCAST_FRAME;
	}

	/* Handle for unicast packets*/
	pMacEntry = &pAd->MacTab.Content[Wcid];
	pTxBlk->wdev = pMacEntry->wdev;
	if (RTMP_GET_PACKET_LOWRATE(pPacket))
	{	/* It's a specific packet need to force low rate, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame*/
		TxFrameType = TX_LEGACY_FRAME;
	}
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	else if (RTMP_GET_PACKET_TDLS(pPacket))
	{
		TxFrameType = TX_LEGACY_FRAME;		
	}
#endif /* DOT11Z_TDLS_SUPPORT */
#ifdef DOT11_N_SUPPORT
	else if (IS_HT_RATE(pMacEntry))
	{	/* it's a 11n capable packet*/

		/* Depends on HTPhyMode to check if the peer support the HTRate transmission.*/
		/* 	Currently didn't support A-MSDU embedded in A-MPDU*/
		TxFrameType = TX_UNKOWN_FRAME;

#ifdef VHT_TXBF_SUPPORT		
        // Because MT7621 is the multi-core MCU, it will have multi-thread program execution.
        // Timer interrupt will be executed in one CPU and driver will be executed in other CPUs.
        // pMacEntry->TxSndgType is set in the timer interrupt and it will also be refered in 
        // driver. There is no problem in MT7620 but pMacEntry->TxSndgType will have asynchronous 
        // problem in MT7621. For example,
        // When pMacEntry->TxSndgType is in SNDG_TYPE_DISABLE state and refered by driver at the beginning.
        // At the meanwhile of program execution, the status of pMacEntry->TxSndgType is changed suddenly.
        // It will make wrong status between enqueued packet and TXWI, and will also make MAC HW to be crashed.
        // How to solve this problem ? Driver could read pMacEntry->TxSndgType before packet enqueue process
        // and copy it with a local variable. By this local variable, it can avoid the asynchronous problem.
        //iPhone6 IOT patch
        if (!IS_VHT_RATE(pMacEntry))
        {
            *pTxSndgTypePerEntry = pMacEntry->TxSndgType;
            pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
	}
#endif		
		
		if (RTMP_GET_PACKET_MOREDATA(pPacket) || (pMacEntry->PsMode == PWR_SAVE))
			TxFrameType |= TX_LEGACY_FRAME;
#ifdef UAPSD_SUPPORT
		else if (RTMP_GET_PACKET_EOSP(pPacket))
			TxFrameType |= TX_LEGACY_FRAME;
#endif /* UAPSD_SUPPORT */
#ifdef WFA_VHT_PF
		else if (pAd->force_amsdu == TRUE)
			return (TxFrameType | TX_AMSDU_FRAME);
#endif /* WFA_VHT_PF */
		else if ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) != 0)
			return (TxFrameType | TX_AMPDU_FRAME);
		else if(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED)
		)
			return (TxFrameType | TX_AMSDU_FRAME);
		else
			TxFrameType |= TX_LEGACY_FRAME;
	
	}
#endif /* DOT11_N_SUPPORT */
	else
	{	/* it's a legacy b/g packet.*/
	
		if ((CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE) && pAd->CommonCfg.bAggregationCapable) &&
			(RTMP_GET_PACKET_TXRATE(pPacket) >= RATE_6) &&
			(!(OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)))
		)
		{	/* if peer support Ralink Aggregation, we use it.*/
			TxFrameType |= TX_RALINK_FRAME;
		}
		else
		{
			TxFrameType |= TX_LEGACY_FRAME;
			
		}
	}

	/* Currently, our fragment only support when a unicast packet send as NOT-ARALINK, NOT-AMSDU and NOT-AMPDU.*/
	
	if ((RTMP_GET_PACKET_FRAGMENTS(pPacket) > 1)
		 && (TxFrameType == TX_LEGACY_FRAME
#ifdef VHT_TXBF_SUPPORT
		 || TxFrameType == (TX_LEGACY_FRAME | TX_NDPA_FRAME)
#endif
		)
#ifdef DOT11_N_SUPPORT
		&& ((pMacEntry->TXBAbitmap & (1<<(RTMP_GET_PACKET_UP(pPacket)))) == 0)
#endif /* DOT11_N_SUPPORT */
		)		
		TxFrameType = TX_FRAG_FRAME;
		
	return TxFrameType;
}


BOOLEAN RTMP_FillTxBlkInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
#ifdef TX_PKT_SG
	NdisMoveMemory( &pTxBlk->pkt_info, &PacketInfo, sizeof(PacketInfo));
#endif /* TX_PKT_SG */
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->apidx = RTMP_GET_PACKET_IF(pPacket);
	pTxBlk->wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);
	
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
#ifdef CONFIG_AP_SUPPORT
	pTxBlk->pMbss = NULL;
#endif /* CONFIG_AP_SUPPORT */

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);

	/* Default to clear this flag*/
	TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bForceNonQoS);

#ifdef WAPI_SUPPORT
	/* Check if this is an WPI data frame*/
	if ((RTMPIsWapiCipher(pAd, pTxBlk->apidx) == TRUE) &&
		 (RTMP_GET_PACKET_WAI(pTxBlk->pPacket) == FALSE))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bWPIDataFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWPIDataFrame);
#endif /* WAPI_SUPPORT */
	
	if (pTxBlk->Wcid == MCAST_WCID)
	{
		pTxBlk->pMacEntry = NULL;
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff))
				pTxBlk->pTransmit = &pAd->CommonCfg.MCastPhyMode;
			else
#endif /* MCAST_RATE_SPECIFIC */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;
		}
		
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);	/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAllowFrag);
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
		if (RTMP_GET_PACKET_MOREDATA(pPacket))
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
		}
#ifdef ETH_CONVERT_SUPPORT
		if (ADHOC_ON(pAd))
		{
			/* If we are running as Ethernet Converter, update MAT DataBase and duplicant the packet if necessary.*/
			if (pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE)
			{
				PNDIS_PACKET donglePkt;

				if(!NdisEqualMemory(pAd->CurrentAddress, (GET_OS_PKT_DATAPTR(pPacket) + 6), MAC_ADDR_LEN))
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bDonglePkt);
				}

				/* For each tx packet, update our MAT convert engine databases.*/
				donglePkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, 0, pTxBlk->OpMode);
				if(donglePkt)
				{
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
					pPacket = donglePkt;
					RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
					pTxBlk->pPacket = donglePkt;
				}
			}
		}
#endif /* ETH_CONVERT_SUPPORT */
	}
	else
	{
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;

		pMacEntry = pTxBlk->pMacEntry;
#ifdef CONFIG_AP_SUPPORT
		pTxBlk->pMbss = pMacEntry->pMbss;
#endif /* CONFIG_AP_SUPPORT */
		
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
		if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
			TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		else
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);

#ifdef CONFIG_STA_SUPPORT
#ifdef XLINK_SUPPORT
		if ((pAd->OpMode == OPMODE_STA) &&
			(ADHOC_ON(pAd)) /*&& 
			(RX_FILTER_TEST_FLAG(pAd, fRX_FILTER_ACCEPT_PROMISCUOUS))*/)
		{
			if(pAd->StaCfg.PSPXlink)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
		}
#endif /* XLINK_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

		{
#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			if (pTxBlk->OpMode == OPMODE_AP)
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
			{
#ifdef WDS_SUPPORT
				if(IS_ENTRY_WDS(pMacEntry))
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
				}
				else
#endif /* WDS_SUPPORT */
#ifdef MWDS
				if(IS_ENTRY_MWDS(pMacEntry))
				{
#ifdef APCLI_SUPPORT
					if(IS_MWDS_OPMODE_APCLI(pMacEntry))
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
						if(!RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
							TX_BLK_SET_FLAG(pTxBlk, fTX_bMWDSFrame);
						
						pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->wdev_idx];
					}
					else
#endif /* APCLI_SUPPORT */
					if(IS_MWDS_OPMODE_AP(pMacEntry))
					{
						if (VALID_MBSS(pAd, pMacEntry->apidx) && !RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket))
							TX_BLK_SET_FLAG(pTxBlk, fTX_bMWDSFrame);
					}
				}
				else
#endif /* MWDS */
#ifdef APCLI_SUPPORT
				if(IS_ENTRY_APCLI(pMacEntry))
				{
#ifdef MAT_SUPPORT
					PNDIS_PACKET apCliPkt = NULL;
					UCHAR tmpIdx = pMacEntry->wdev_idx;
					BOOLEAN bNeedMATHandle = TRUE;
						
#ifdef MAC_REPEATER_SUPPORT
					if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn))
					{
						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;
						if(pAd->ApCfg.MACRepeaterOuiMode != 1)
							tmpIdx = (64 + (MAX_EXT_MAC_ADDR_SIZE * pMacEntry->wdev_idx) + pMacEntry->MatchReptCliIdx);
						else
							bNeedMATHandle = FALSE;
						}
#endif /* MAC_REPEATER_SUPPORT */
					if(bNeedMATHandle)
					{
						/* For each tx packet, update our MAT convert engine databases.*/
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
						if(apCliPkt)
						{
							RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
							pPacket = apCliPkt;
							RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
							pTxBlk->pPacket = apCliPkt;
						}
					}
#endif /* MAT_SUPPORT */
					pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->wdev_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);

				}
				else
#endif /* APCLI_SUPPORT */
#ifdef CLIENT_WDS
				if (IS_ENTRY_CLIWDS(pMacEntry))
				{
					PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
					PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
					if (((pMacEntry->apidx < MAX_MBSSID_NUM(pAd))
						&& !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[pMacEntry->apidx].Bssid))
						|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
						)
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
					}
				}
				else
#endif /* CLIENT_WDS */
				if (IS_ENTRY_CLIENT(pMacEntry))
				{ }
				else
					return FALSE;
					
				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			if (pTxBlk->OpMode == OPMODE_STA)
#else
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			{
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				if(IS_ENTRY_TDLS(pMacEntry))
				{
					TX_BLK_SET_FLAG(pTxBlk, fTX_bTdlsEntry);
				}
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef ETH_CONVERT_SUPPORT
				/* If we are running as Ethernet Converter, update MAT DataBase and duplicant the packet if necessary.*/
				if (pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE)
				{
					PNDIS_PACKET donglePkt;
		
					if(!NdisEqualMemory(pAd->CurrentAddress, (GET_OS_PKT_DATAPTR(pPacket) + 6), MAC_ADDR_LEN))
					{
						TX_BLK_SET_FLAG(pTxBlk, fTX_bDonglePkt);
					}
				
					/* For each tx packet, update our MAT convert engine databases.*/
					donglePkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, 0, pTxBlk->OpMode);
					if(donglePkt)
					{
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = donglePkt;
						RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
						pTxBlk->pPacket = donglePkt;
					}
				}
#endif /* ETH_CONVERT_SUPPORT */

				/* If support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) &&
					CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
#endif /* CONFIG_STA_SUPPORT */
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME)
		{
			if ( ((RTMP_GET_PACKET_LOWRATE(pPacket)) 
#ifdef UAPSD_SUPPORT
				&& (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */				
				) ||
				((pAd->OpMode == OPMODE_AP) && 
				 (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			)
			{	/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate.*/
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID].HTPhyMode;

#ifdef WAPI_SUPPORT
				/* 	According to WAPIA certification description, WAI packets can not
					include QoS header */
				if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
#endif /* WAPI_SUPPORT */		
#ifdef DOT11_N_SUPPORT
				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (IS_HT_STA(pTxBlk->pMacEntry) &&
					(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
					((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
				{
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
					TX_BLK_SET_FLAG(pTxBlk, fTX_bForceNonQoS);
				}
#endif /* DOT11_N_SUPPORT */
			}
			
#ifdef DOT11_N_SUPPORT
			if ( (IS_HT_RATE(pMacEntry) == FALSE) &&
				(CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)))
			{	/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}
#endif /* DOT11_N_SUPPORT */

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
			}
#ifdef UAPSD_SUPPORT
			if (RTMP_GET_PACKET_EOSP(pPacket))
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);
			}
#endif /* UAPSD_SUPPORT */
		}
		else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
		{
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);
		}
		
		pMacEntry->DebugTxCount++;
	}


	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;

	return TRUE;
}


BOOLEAN CanDoAggregateTransmit(
	IN RTMP_ADAPTER *pAd,
	IN NDIS_PACKET *pPacket,
	IN TX_BLK		*pTxBlk)
{
	int minLen = LENGTH_802_3;

	/*DBGPRINT(RT_DEBUG_TRACE, ("Check if can do aggregation! TxFrameType=%d!\n", pTxBlk->TxFrameType));*/
	
	if (RTMP_GET_PACKET_WCID(pPacket) == MCAST_WCID)
		return FALSE;

	if (RTMP_GET_PACKET_DHCP(pPacket) ||
		RTMP_GET_PACKET_EAPOL(pPacket) ||
		RTMP_GET_PACKET_WAI(pPacket)
	)
		return FALSE;
	
	/* Make sure the first packet has non-zero-length data payload */
	if (RTMP_GET_PACKET_VLAN(pPacket))
		minLen += LENGTH_802_1Q; /* VLAN tag */
	else if (RTMP_GET_PACKET_LLCSNAP(pPacket))
		minLen += 8; /* SNAP hdr Len*/
	if (minLen >= GET_OS_PKT_LEN(pPacket))
		return FALSE;
	
	if ((pTxBlk->TxFrameType == TX_AMSDU_FRAME) &&
		((pTxBlk->TotalFrameLen + GET_OS_PKT_LEN(pPacket))> (RX_BUFFER_AGGRESIZE - 100)))
	{	/* For AMSDU, allow the packets with total length < max-amsdu size*/
		return FALSE;
	}
	
	if ((pTxBlk->TxFrameType == TX_RALINK_FRAME) &&
		(pTxBlk->TxPacketList.Number == 2))
	{	/* For RALINK-Aggregation, allow two frames in one batch.*/
		return FALSE;
	}

#ifdef CONFIG_STA_SUPPORT
	if ((INFRA_ON(pAd)) && (pAd->OpMode == OPMODE_STA)) /* must be unicast to AP*/
		return TRUE;
	else
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	/* CFG_TODO */
	if ((MAC_ADDR_EQUAL(GET_OS_PKT_DATAPTR(pTxBlk->pPacket), GET_OS_PKT_DATAPTR(pPacket))) 
	    && (pAd->OpMode == OPMODE_AP)) /* unicast to same STA*/
		return TRUE;
	else
#endif /* CONFIG_AP_SUPPORT */
		return FALSE;
	
}


#ifdef RTMP_MAC_PCI
VOID TxDoneCleanupExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)FunctionContext;
	unsigned long IrqFlags = 0;
	ULONG FreeNum;
	int NeedCleanupTimer = 0;
	UCHAR QueIdx;

	if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
				 fRTMP_ADAPTER_RESET_IN_PROGRESS |
				 fRTMP_ADAPTER_HALT_IN_PROGRESS)))
		return;

	DEQUEUE_LOCK(&pAd->irq_lock, FALSE, IrqFlags);
	for (QueIdx=0; QueIdx<NUM_OF_TX_RING; QueIdx++) {
		FreeNum = GET_TXRING_FREENO(pAd, QueIdx);
		if (FreeNum < (TX_RING_SIZE-1)) {
			RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
			NeedCleanupTimer = 1;
		}
	}
	DEQUEUE_UNLOCK(&pAd->irq_lock, FALSE, IrqFlags);

	if (NeedCleanupTimer)
		RTMPModTimer(&pAd->TxDoneCleanupTimer, 50);
}
#endif /* RTMP_MAC_PCI */


/*
	========================================================================

	Routine Description:
		To do the enqueue operation and extract the first item of waiting
		list. If a number of available shared memory segments could meet
		the request of extracted item, the extracted item will be fragmented
		into shared memory segments.
	
	Arguments:
		pAd Pointer to our adapter
		pQueue		Pointer to Waiting Queue
		
	Return Value:
		None

	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPDeQueuePacket(
	IN RTMP_ADAPTER *pAd,
	IN BOOLEAN bIntContext,
	IN UCHAR QIdx,
	IN INT Max_Tx_Packets)
{
	PQUEUE_ENTRY pEntry = NULL;
	PNDIS_PACKET pPacket;
	NDIS_STATUS Status = NDIS_STATUS_SUCCESS;
	UCHAR Count=0;
	PQUEUE_HEADER   pQueue;
	ULONG FreeNumber[NUM_OF_TX_RING];
	CHAR QueIdx, sQIdx, eQIdx;
	unsigned long	IrqFlags = 0;
	BOOLEAN hasTxDesc = FALSE;
	TX_BLK TxBlk, *pTxBlk;
	UCHAR RAWcid;
#ifdef TXBF_SUPPORT 
	UCHAR TxSndgType = SNDG_TYPE_DISABLE;
#endif /* TXBF_SUPPORT */	
	MAC_TABLE_ENTRY *pMacEntry = NULL;

#ifdef DBG_DIAGNOSE
	BOOLEAN firstRound;
	RtmpDiagStruct *pDiagStruct = &pAd->DiagStruct;
#endif
#if defined (LED_SOFT_SUPPORT)
	ULONG HardTxTotal = 0;
#endif

	if (QIdx == NUM_OF_TX_RING)
	{
		sQIdx = 3;
		eQIdx = 0;	/* 4 ACs, start from 0.*/
	}
	else
	{
		sQIdx = eQIdx = QIdx;
	}

	for (QueIdx=sQIdx; QueIdx >= eQIdx; QueIdx--)
	{
		Count=0;

		RTMP_START_DEQUEUE(pAd, QueIdx, IrqFlags);

#ifdef DBG_DIAGNOSE
		firstRound = ((QueIdx == sQIdx) ? TRUE : FALSE);
#endif /* DBG_DIAGNOSE */

		while (1)
		{
			if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS |
										fRTMP_ADAPTER_RADIO_OFF |
										fRTMP_ADAPTER_RESET_IN_PROGRESS |
										fRTMP_ADAPTER_HALT_IN_PROGRESS |
										fRTMP_ADAPTER_NIC_NOT_EXIST |
										fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET)))
#ifdef RT_CFG80211_P2P_SUPPORT
				|| IS_CFG80211_P2P_ABSENCE(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
				|| RTMP_TEST_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE)
#endif /* DOT11Z_TDLS_SUPPORT */
				)
			{
				RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);
				return;
			}
			
			if (Count >= Max_Tx_Packets)
				break;

			DEQUEUE_LOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			if (&pAd->TxSwQueue[QueIdx] == NULL)
			{
#ifdef DBG_DIAGNOSE
#ifdef DBG_TXQ_DEPTH
				if (firstRound == TRUE)
					pDiagStruct->TxSWQueCnt[pDiagStruct->ArrayCurIdx][0]++;
#endif /* DBG_TXQ_DEPTH */
#endif /* DBG_DIAGNOSE */
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

#ifdef RTMP_MAC_PCI
			FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);

#ifdef DBG_DIAGNOSE
#ifdef DBG_TX_RING_DEPTH
			if (firstRound == TRUE)
			{
				UCHAR	txDescNumLevel, txSwQNumLevel;

				txDescNumLevel = (TX_RING_SIZE - FreeNumber[QueIdx]); /* Number of occupied hw desc.*/
				txDescNumLevel = ((txDescNumLevel <=15) ? txDescNumLevel : 15);
				pDiagStruct->TxDescCnt[pDiagStruct->ArrayCurIdx][txDescNumLevel]++;

				txSwQNumLevel = ((pAd->TxSwQueue[QueIdx].Number <=7) ? pAd->TxSwQueue[QueIdx].Number : 8);
				pDiagStruct->TxSWQueCnt[pDiagStruct->ArrayCurIdx][txSwQNumLevel]++;

				firstRound = FALSE;
			}
#endif /* DBG_TX_RING_DEPTH */
#endif /* DBG_DIAGNOSE */

#ifdef CONFIG_ARCH_MT7623
			if (1)
#else
			/* We should never let more than 64kBytes of tx data not be
			 * cleaned up, or the TCP TX throughput will suffer as the
			 * window will not fill and so expand. 64kBytes ~ 44 full
			 * size ethernet packets. Use 32 as a nice constant. There
			 * is also a timer function which prevents packets getting
			 * "stuck" when data stops flowing
			 */

			if ((TX_RING_SIZE - FreeNumber[QueIdx]) > 32)
#endif
			{
				/* free Tx(QueIdx) resources*/
				RTMPFreeTXDUponTxDmaDone(pAd, QueIdx);
				FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
			}
#endif /* RTMP_MAC_PCI */

			/* probe the Queue Head*/
			pQueue = &pAd->TxSwQueue[QueIdx];
			if ((pEntry = pQueue->Head) == NULL)
			{
				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				break;
			}

			pTxBlk = &TxBlk;
			NdisZeroMemory((PUCHAR)pTxBlk, sizeof(TX_BLK));
			
			pTxBlk->QueIdx = QueIdx;

#ifdef VENDOR_FEATURE1_SUPPORT
			pTxBlk->HeaderBuf = (UCHAR *)pTxBlk->HeaderBuffer;
#endif /* VENDOR_FEATURE1_SUPPORT */

			pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);

#ifdef CONFIG_AP_SUPPORT
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			if (RTMP_GET_PACKET_OPMODE(pPacket))
			{
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
			if (RTMP_GET_PACKET_MGMT_PKT(pPacket) == 1)
			{
				/* this is a management frame */
				NDIS_STATUS status;

				pEntry = RemoveHeadQueue(pQueue);

#ifdef RTMP_MAC_PCI
				if ((pAd->MACVersion == 0x28600100) ||
					(RTMP_GET_PACKET_MGMT_PKT_DATA_QUE(pPacket) == 1))
					status = MlmeHardTransmitTxRing(pAd,QueIdx,pPacket);
				else
#endif /* RTMP_MAC_PCI */
					status = MlmeHardTransmitMgmtRing(pAd,QueIdx,pPacket);

				if (status != NDIS_STATUS_SUCCESS)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("tx queued mgmt frame error!\n"));
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				Count++;
				continue;
			}
			else 
			{
				/*when WDS Jam happen, drop following 1min to HW TxRing Pkts*/
				RAWcid = RTMP_GET_PACKET_WCID(pPacket);
				pMacEntry = &pAd->MacTab.Content[RAWcid];

#ifdef WDS_SUPPORT
				/*
					It WDS life checking.
					WDS need to check the peer is come back or not
					by sending few (2 ~3) WDS Packet out to peer.
					It must be checked first.
				*/
				if(IS_ENTRY_WDS(pMacEntry))
				{
					ULONG Now32;
					NdisGetSystemUpTime(&Now32);
					if(pMacEntry->LockEntryTx && RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL))
					{
						pEntry = RemoveHeadQueue(pQueue);
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
						Count++;
						continue;
					}
					else
					    NdisGetSystemUpTime(&pMacEntry->TimeStamp_toTxRing);
				}
				else
#endif /* WDS_SUPPORT */
				if (!IS_ENTRY_NONE(pMacEntry)
					&& (pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck))
				{
					/*
						Sample Lin, 20100412

						For non-WDS interface, we need to send packet to detect
						the link periodically; Or when
						pMacEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck,
						no any chance to clear pMacEntry->ContinueTxFailCnt.

						EX: When pMacEntry->ContinueTxFailCnt >=
						pAd->ApCfg.EntryLifeCheck, the condition will not be
						removed and we will drop all packets for the pEntry.
						But maybe the signal becomes better.
						So we try to send a packet periodically and we will
						get the tx status in tx done interrupt.
						If the tx status is success, pMacEntry->ContinueTxFailCnt
						will be cleared to 0.
					*/
#define ENTRY_RETRY_INTERVAL	(100 * OS_HZ / 1000)
					ULONG Now32;
					NdisGetSystemUpTime(&Now32);
					if(RTMP_TIME_BEFORE(Now32, pMacEntry->TimeStamp_toTxRing + ENTRY_RETRY_INTERVAL))
					{
						pEntry = RemoveHeadQueue(pQueue);
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
						DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
						Count++;
						continue;
					}
					else
					    NdisGetSystemUpTime(&pMacEntry->TimeStamp_toTxRing);
				}
			}
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			}
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
#endif /* CONFIG_AP_SUPPORT */
			
			/* Early check to make sure we have enoguh Tx Resource.*/
			hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
			if (!hasTxDesc)
			{
				pAd->PrivateInfo.TxRingFullCnt++;

				DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
				
				break;
			}

#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			pTxBlk->OpMode = RTMP_GET_PACKET_OPMODE(pPacket);
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
#ifdef VHT_TXBF_SUPPORT
			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket, pTxBlk, &TxSndgType);
#else
			pTxBlk->TxFrameType = TxPktClassification(pAd, pPacket, pTxBlk);
#endif
			pEntry = RemoveHeadQueue(pQueue);
			pTxBlk->TotalFrameNum++;
			pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
			pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
			pTxBlk->pPacket = pPacket;

			InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));

			if (pTxBlk->TxFrameType & (TX_RALINK_FRAME | TX_AMSDU_FRAME))
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}
			}


			if (pTxBlk->TxFrameType == TX_RALINK_FRAME || pTxBlk->TxFrameType == TX_AMSDU_FRAME)
			{
				// Enhance SW Aggregation Mechanism
				if (NEED_QUEUE_BACK_FOR_AGG(pAd, QueIdx, FreeNumber[QueIdx], pTxBlk->TxFrameType))
				{
					InsertHeadQueue(pQueue, PACKET_TO_QUEUE_ENTRY(pPacket));
					DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
					break;
				}

				do{
					if((pEntry = pQueue->Head) == NULL)
						break;

					/* For TX_AMSDU_FRAME/TX_RALINK_FRAME, Need to check if next pakcet can do aggregation.*/
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
					FreeNumber[QueIdx] = GET_TXRING_FREENO(pAd, QueIdx);
					hasTxDesc = RTMP_HAS_ENOUGH_FREE_DESC(pAd, pTxBlk, FreeNumber[QueIdx], pPacket);
					if ((hasTxDesc == FALSE) || (CanDoAggregateTransmit(pAd, pPacket, pTxBlk) == FALSE))
						break;

					/*Remove the packet from the TxSwQueue and insert into pTxBlk*/
					pEntry = RemoveHeadQueue(pQueue);
					ASSERT(pEntry);
					pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);


					pTxBlk->TotalFrameNum++;
					pTxBlk->TotalFragNum += RTMP_GET_PACKET_FRAGMENTS(pPacket);	/* The real fragment number maybe vary*/
					pTxBlk->TotalFrameLen += GET_OS_PKT_LEN(pPacket);
					InsertTailQueue(&pTxBlk->TxPacketList, PACKET_TO_QUEUE_ENTRY(pPacket));
				}while(1);

				if (pTxBlk->TxPacketList.Number == 1)
					pTxBlk->TxFrameType = TX_LEGACY_FRAME;
			}

					
			Count += pTxBlk->TxPacketList.Number;

			/* Do HardTransmit now.*/
#ifdef CONFIG_AP_SUPPORT
#ifdef CUSTOMER_DCC_FEATURE
			if(pTxBlk->TxFrameType != TX_MCAST_FRAME)
				APGetStreamingStatus(pAd, QueIdx, *pTxBlk);
#endif
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#ifdef TXBF_SUPPORT			
				Status = APHardTransmit(pAd, pTxBlk, QueIdx, TxSndgType);
#else
				Status = APHardTransmit(pAd, pTxBlk, QueIdx);
#endif
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
			//if (FromWhichBSSID >= MIN_NET_DEVICE_FOR_APCLI)
			if (RTMP_GET_PACKET_OPMODE(pPacket))
#ifdef TXBF_SUPPORT			    
				Status = APHardTransmit(pAd, pTxBlk, QueIdx, TxSndgType);
#else
				Status = APHardTransmit(pAd, pTxBlk, QueIdx);
#endif
			else
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT */
#ifdef TXBF_SUPPORT
				Status = STAHardTransmit(pAd, pTxBlk, QueIdx, TxSndgType);
#else
				Status = STAHardTransmit(pAd, pTxBlk, QueIdx);
#endif
			}
#endif /* CONFIG_STA_SUPPORT */

			if (Status != NDIS_STATUS_SUCCESS)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("Errror!! APHardTransmit() failed. ErrorCode = %d\n", Status));
			}

#ifdef RTMP_MAC_PCI
			DEQUEUE_UNLOCK(&pAd->irq_lock, bIntContext, IrqFlags);
			/* static rate also need NICUpdateFifoStaCounters() function.*/
			/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED))*/
			if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD)
			{
				// TODO: shiang-usw, check this because of REG access here!!
				NICUpdateFifoStaCounters(pAd);
				pAd->FifoUpdateDone = 0;
			}
#endif /* RTMP_MAC_PCI */

#if defined (LED_SOFT_SUPPORT)
			HardTxTotal++;
#endif
		}

		RTMP_STOP_DEQUEUE(pAd, QueIdx, IrqFlags);

		
#ifdef BLOCK_NET_IF
		if ((pAd->blockQueueTab[QueIdx].SwTxQueueBlockFlag == TRUE)
			&& (pAd->TxSwQueue[QueIdx].Number < 1))
		{
			releaseNetIf(&pAd->blockQueueTab[QueIdx]);
		}
#endif /* BLOCK_NET_IF */
	}

#if defined (LED_SOFT_SUPPORT)
	if (HardTxTotal > 0 && pAd->MacTab.Size > 0) {
#if defined (DRIVER_HAS_MULTI_DEV) && (LED_SOFT_BLINK_GPIO_DEV1 >= 0) && (LED_SOFT_BLINK_GPIO_DEV1 != LED_SOFT_BLINK_GPIO)
		if (pAd->dev_idx == 1)
			ralink_gpio_led_blink(LED_SOFT_BLINK_GPIO_DEV1);
		else
#endif
			ralink_gpio_led_blink(LED_SOFT_BLINK_GPIO);
	}
#endif
}

#ifdef CUSTOMER_DCC_FEATURE
/* Get the status of the current Streaming stataus ( BE, BK, VI, VO) which is getting transmitted throught the AP */
VOID APGetStreamingStatus(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			QueIdx,
	IN TX_BLK			TxBlk)
{
	UINT64 Time = jiffies_to_msecs(jiffies);
	static	UCHAR packet_count[4] = {0};

	switch(QueIdx)
	{
		case 0:
				if((pAd->StreamingTypeStatus.BE == FALSE) && ((Time - pAd->StreamingTypeStatus.BE_Time ) > 1000))
					packet_count[0] = 0;

				packet_count[0] += (TxBlk.TotalFragNum > TxBlk.TotalFrameNum ? TxBlk.TotalFragNum : TxBlk.TotalFrameNum);

				if(packet_count[0] > 5)
					pAd->StreamingTypeStatus.BE = TRUE;

				pAd->StreamingTypeStatus.BE_Time = Time;
				break;

		case 1:
				if((pAd->StreamingTypeStatus.BK == FALSE) && ((Time - pAd->StreamingTypeStatus.BK_Time ) > 1000))
					packet_count[1] = 0;

				packet_count[1] += (TxBlk.TotalFragNum > TxBlk.TotalFrameNum ? TxBlk.TotalFragNum : TxBlk.TotalFrameNum);

				if(packet_count[1] > 5)
					pAd->StreamingTypeStatus.BK = TRUE;

				pAd->StreamingTypeStatus.BK_Time = Time;
				break;

		case 2:
				if((pAd->StreamingTypeStatus.VI == FALSE) && ((Time - pAd->StreamingTypeStatus.VI_Time ) > 1000))
					packet_count[2] = 0;

				packet_count[2] += (TxBlk.TotalFragNum > TxBlk.TotalFrameNum ? TxBlk.TotalFragNum : TxBlk.TotalFrameNum);

				if(packet_count[2] > 5)
					pAd->StreamingTypeStatus.VI = TRUE;

				pAd->StreamingTypeStatus.VI_Time = Time;
				break;

		case 3:
				if((pAd->StreamingTypeStatus.VO == FALSE) && ((Time - pAd->StreamingTypeStatus.VO_Time ) > 1000))
					packet_count[3] = 0;

				packet_count[3] += (TxBlk.TotalFragNum > TxBlk.TotalFrameNum ? TxBlk.TotalFragNum : TxBlk.TotalFrameNum);

				if(packet_count[3] > 5)
					pAd->StreamingTypeStatus.VO = TRUE;

				pAd->StreamingTypeStatus.VO_Time = Time;
				break;

		default:
				/* nothing to do */
				break;
	}
}

VOID RTMPCalculateAPTxRxActivityTime(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT32			Index,
	IN	UINT32			Length,
	IN	PMULTISSID_STRUCT pMbss,
	IN	PMAC_TABLE_ENTRY  pMacEntry)
{
	UINT64	Time;

	//Time = ((Length * 1000 * 1000 * 1000) / (Rate * 125000)); 
	Time = ((UINT64)Length * RateMultiplicationShiftFactor[Index].Multiplication) >> RateMultiplicationShiftFactor[Index].Shift;
	// printk("index %u length %u time	%llu \n", Index, Length, Time);	
	if(pAd->EnableChannelStatsCheck)
	{
		pAd->ChannelStats.ChannelApActivity1secValue += Time;
		pAd->ChannelStats.ChannelApActivity += Time;
	}
	if (pMbss != NULL)
	{
		pMbss->ChannelUseTime +=Time;
		if ((pMacEntry != NULL) && (IS_ENTRY_CLIENT(pMacEntry)))
		{
			pMacEntry->ChannelUseTime += Time;
		}
	}
}
#endif

/*
	========================================================================

	Routine Description:
		Calculates the duration which is required to transmit out frames
	with given size and specified rate.
		
	Arguments:
		pAd 	Pointer to our adapter
		Rate			Transmit rate
		Size			Frame size in units of byte
		
	Return Value:
		Duration number in units of usec

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
USHORT	RTMPCalcDuration(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Rate,
	IN	ULONG			Size)
{
	ULONG	Duration = 0;

	if (Rate < RATE_FIRST_OFDM_RATE) /* CCK*/
	{
		if ((Rate > RATE_1) && OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
			Duration = 96;	/* 72+24 preamble+plcp*/
		else
			Duration = 192; /* 144+48 preamble+plcp*/

		Duration += (USHORT)((Size << 4) / RateIdTo500Kbps[Rate]);
		if ((Size << 4) % RateIdTo500Kbps[Rate])
			Duration ++;
	}
	else if (Rate <= RATE_LAST_OFDM_RATE)/* OFDM rates*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
		Duration += 4 * (USHORT)((11 + Size * 4) / RateIdTo500Kbps[Rate]);
		if ((11 + Size * 4) % RateIdTo500Kbps[Rate])
			Duration += 4;
	}
	else	/*mimo rate*/
	{
		Duration = 20 + 6;		/* 16+4 preamble+plcp + Signal Extension*/
	}
	
	return (USHORT)Duration;
}


/*
	========================================================================

	Routine Description:
		Suspend MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	Note:
	
	========================================================================
*/
VOID RTMPSuspendMsduTransmission(
	IN PRTMP_ADAPTER pAd)
{
	DBGPRINT(RT_DEBUG_TRACE,("SCANNING, suspend MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT /* Roger sync Carrier */
	/* no carrier detection when scanning */
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStop(pAd);
#endif
#endif /* CONFIG_AP_SUPPORT */
	
	/*
		Before BSS_SCAN_IN_PROGRESS, we need to keep Current R66 value and
		use Lowbound as R66 value on ScanNextChannel(...)
	*/
	bbp_get_agc(pAd, &pAd->BbpTuning.R66CurrentValue, RX_CHAIN_0);

	pAd->hw_cfg.bbp_bw = pAd->CommonCfg.BBPCurrentBW;

	RTMPSetAGCInitValue(pAd, BW_20);
	
	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
	/* abort all TX rings */
	/*RTMP_IO_WRITE32(pAd, TX_CNTL_CSR, 0x000f0000);	*/
}


/*
	========================================================================

	Routine Description:
		Resume MSDU transmission
		
	Arguments:
		pAd 	Pointer to our adapter
		
	Return Value:
		None
		
	IRQL = DISPATCH_LEVEL
	
	Note:
	
	========================================================================
*/
VOID RTMPResumeMsduTransmission(
	IN PRTMP_ADAPTER pAd)
{  
	DBGPRINT(RT_DEBUG_TRACE,("SCAN done, resume MSDU transmission ...\n"));

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
	/* no carrier detection when scanning*/
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
			CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	/*
		After finish BSS_SCAN_IN_PROGRESS, we need to restore Current R66 value
		R66 should not be 0
	*/
	if (pAd->BbpTuning.R66CurrentValue == 0)
	{
		pAd->BbpTuning.R66CurrentValue = 0x38;
		DBGPRINT_ERR(("RTMPResumeMsduTransmission, R66CurrentValue=0...\n"));
	}
	bbp_set_agc(pAd, pAd->BbpTuning.R66CurrentValue, RX_CHAIN_ALL);
	
	pAd->CommonCfg.BBPCurrentBW = pAd->hw_cfg.bbp_bw;
	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);
/* sample, for IRQ LOCK to SEM LOCK */
/*
	IrqState = pAd->irq_disabled;
	if (IrqState)
		RTMPDeQueuePacket(pAd, TRUE, NUM_OF_TX_RING, MAX_TX_PROCESS);
	else
*/
	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}


#ifdef DOT11_N_SUPPORT
UINT deaggregate_AMSDU_announce(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,	
	PNDIS_PACKET		pPacket,
	IN	PUCHAR			pData,
	IN	ULONG			DataSize,
	IN	UCHAR			OpMode)
{
	USHORT 			PayloadSize;
	USHORT 			SubFrameSize;
	PHEADER_802_3 	pAMSDUsubheader;
	UINT			nMSDU;
    UCHAR			Header802_3[14];

	PUCHAR			pPayload, pDA, pSA, pRemovedLLCSNAP;
	PNDIS_PACKET	pClonePacket;

#ifdef CONFIG_AP_SUPPORT
	UCHAR FromWhichBSSID = RTMP_GET_PACKET_IF(pPacket);
	UCHAR VLAN_Size;
	USHORT VLAN_VID = 0, VLAN_Priority = 0;


	if ((FromWhichBSSID < pAd->ApCfg.BssidNum)
		)
		VLAN_Size = (pAd->ApCfg.MBSSID[FromWhichBSSID].wdev.VLAN_VID != 0) ? LENGTH_802_1Q : 0;
#ifdef WDS_VLAN_SUPPORT
	else if ((FromWhichBSSID >= MIN_NET_DEVICE_FOR_WDS) &&
			(FromWhichBSSID < (MIN_NET_DEVICE_FOR_WDS + MAX_WDS_ENTRY)))
	{
		VLAN_Size = (pAd->WdsTab.WdsEntry[FromWhichBSSID - MIN_NET_DEVICE_FOR_WDS].wdev.VLAN_VID != 0) ? LENGTH_802_1Q : 0;
	}
#endif /* WDS_VLAN_SUPPORT */
	else /* only MBssid support VLAN.*/
		VLAN_Size = 0;
#endif /* CONFIG_AP_SUPPORT */

	nMSDU = 0;

	while (DataSize > LENGTH_802_3)
	{
		nMSDU++;

		/*hex_dump("subheader", pData, 64);*/
		pAMSDUsubheader = (PHEADER_802_3)pData;
		/*pData += LENGTH_802_3;*/
		PayloadSize = pAMSDUsubheader->Octet[1] + (pAMSDUsubheader->Octet[0]<<8);
		SubFrameSize = PayloadSize + LENGTH_802_3;

		if ((DataSize < SubFrameSize) || (PayloadSize > 1518 ))
			break;

		/*DBGPRINT(RT_DEBUG_TRACE,("%d subframe: Size = %d\n",  nMSDU, PayloadSize));*/
		pPayload = pData + LENGTH_802_3;
		pDA = pData;
		pSA = pData + MAC_ADDR_LEN;

		/* convert to 802.3 header*/
        CONVERT_TO_802_3(Header802_3, pDA, pSA, pPayload, PayloadSize, pRemovedLLCSNAP);

#ifdef CONFIG_STA_SUPPORT
		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E)
			)
		{
			MLME_QUEUE_ELEM *Elem;

			os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
			if (Elem != NULL)
			{
				memmove(Elem->Msg+(LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
				Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
				REPORT_MGMT_FRAME_TO_MLME(pAd, BSSID_WCID, Elem->Msg, Elem->MsgLen, 0, 0, 0, 0, OPMODE_STA);
				os_free_mem(NULL, Elem);
				Elem = NULL;
			}
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef APCLI_SUPPORT
		if ((Header802_3[12] == 0x88) && (Header802_3[13] == 0x8E))
		{
			/* avoid local heap overflow, use dyanamic allocation */
			MLME_QUEUE_ELEM *Elem; /* = (MLME_QUEUE_ELEM *) kmalloc(sizeof(MLME_QUEUE_ELEM), MEM_ALLOC_FLAG);*/
			
			if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid, pSA))
			{
				APCLI_STRUCT *apcli_entry;
				apcli_entry = &pAd->ApCfg.ApCliTab[0];
				os_alloc_mem(pAd, (UCHAR **)&Elem, sizeof(MLME_QUEUE_ELEM));
				if (Elem != NULL)
				{
					if (pRxBlk != NULL)
					{
						memmove(Elem->Msg, pRxBlk->pHeader, LENGTH_802_11);
						memmove(Elem->Msg+(LENGTH_802_11 + LENGTH_802_1_H), pPayload, PayloadSize);
						Elem->MsgLen = LENGTH_802_11 + LENGTH_802_1_H + PayloadSize;
#ifdef CUSTOMER_DCC_FEATURE						
						REPORT_MGMT_FRAME_TO_MLME(pAd, apcli_entry->MacTabWCID, Elem->Msg, Elem->MsgLen, 0, 0, 0, 0, 0, 0, OPMODE_AP);
#else
						REPORT_MGMT_FRAME_TO_MLME(pAd, apcli_entry->MacTabWCID, Elem->Msg, Elem->MsgLen, 0, 0, 0, 0, OPMODE_AP);
#endif
					}
					os_free_mem(NULL, Elem);
				}
			}
			/* A-MSDU has padding to multiple of 4 including subframe header.*/
			/* align SubFrameSize up to multiple of 4*/
			SubFrameSize = (SubFrameSize+3)&(~0x3);


			if (SubFrameSize > 1528 || SubFrameSize < 32)
				break;

			if (DataSize > SubFrameSize)
			{
				pData += SubFrameSize;
				DataSize -= SubFrameSize;
			}
			else
			{
				/* end of A-MSDU*/
				DataSize = 0;
			}	
			continue;
		}
#endif


#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			UCHAR	WhichBSSID = FromWhichBSSID;
			if (pRemovedLLCSNAP)
			{
				pPayload -= (LENGTH_802_3 + VLAN_Size);
				PayloadSize += (LENGTH_802_3 + VLAN_Size);
				/*NdisMoveMemory(pPayload, &Header802_3, LENGTH_802_3);*/
			}
			else
			{
				pPayload -= VLAN_Size;
				PayloadSize += VLAN_Size;
			}

			MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, WhichBSSID);

#ifdef WDS_VLAN_SUPPORT
			if (VLAN_VID == 0) /* maybe WDS packet */
				WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

			RT_VLAN_8023_HEADER_COPY(pAd, VLAN_VID, VLAN_Priority,
									Header802_3, LENGTH_802_3, pPayload,
									FromWhichBSSID, TPID);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
	        	if (pRemovedLLCSNAP)
	        	{
	    			pPayload -= LENGTH_802_3;
	    			PayloadSize += LENGTH_802_3;
	    			NdisMoveMemory(pPayload, &Header802_3[0], LENGTH_802_3);
	        	}
		}
#endif /* CONFIG_STA_SUPPORT */

		pClonePacket = ClonePacket(pAd, pPacket, pPayload, PayloadSize);
		if (pClonePacket)
		{
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
				AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pClonePacket, RTMP_GET_PACKET_IF(pPacket));
#endif /* CONFIG_STA_SUPPORT */
		}


		/* A-MSDU has padding to multiple of 4 including subframe header.*/
		/* align SubFrameSize up to multiple of 4*/
		SubFrameSize = (SubFrameSize+3)&(~0x3);


		if (SubFrameSize > 1528 || SubFrameSize < 32)
			break;

		if (DataSize > SubFrameSize)
		{
			pData += SubFrameSize;
			DataSize -= SubFrameSize;
		}
		else
		{
			/* end of A-MSDU*/
			DataSize = 0;
		}
	}
	
	/* finally release original rx packet*/
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);

	return nMSDU;
}


UINT BA_Reorder_AMSDU_Annnounce(
	IN	PRTMP_ADAPTER	pAd,
	IN	PNDIS_PACKET	pPacket,
	IN	UCHAR			OpMode)
{
	PUCHAR			pData;
	USHORT			DataSize;
	UINT			nMSDU = 0;

	pData = (PUCHAR) GET_OS_PKT_DATAPTR(pPacket);
	DataSize = (USHORT) GET_OS_PKT_LEN(pPacket);

	nMSDU = deaggregate_AMSDU_announce(pAd, NULL, pPacket, pData, DataSize, OpMode);

	return nMSDU;
}

VOID Indicate_AMSDU_Packet(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
#ifdef APCLI_SUPPORT
	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s:drop packet by PN mismatch!\n", __func__));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
#endif /* APCLI_SUPPORT */
	RTMP_UPDATE_OS_PACKET_INFO(pAd, pRxBlk, FromWhichBSSID);
	RTMP_SET_PACKET_IF(pRxBlk->pRxPacket, FromWhichBSSID);
	deaggregate_AMSDU_announce(pAd, pRxBlk, pRxBlk->pRxPacket, pRxBlk->pData, pRxBlk->DataSize, pRxBlk->OpMode);
}
#endif /* DOT11_N_SUPPORT */


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID AssocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_ASSOC_REQ_STRUCT *AssocReq,
	IN PUCHAR                     pAddr,
	IN USHORT                     CapabilityInfo,
	IN ULONG                      Timeout,
	IN USHORT                     ListenIntv)
{
	COPY_MAC_ADDR(AssocReq->Addr, pAddr);
	/* Add mask to support 802.11b mode only */
	AssocReq->CapabilityInfo = CapabilityInfo & SUPPORTED_CAPABILITY_INFO; /* not cf-pollable, not cf-poll-request*/
	AssocReq->Timeout = Timeout;
	AssocReq->ListenIntv = ListenIntv;
}


/*
	==========================================================================
	Description:

	IRQL = DISPATCH_LEVEL
	
	==========================================================================
*/
VOID DisassocParmFill(
	IN PRTMP_ADAPTER pAd,
	IN OUT MLME_DISASSOC_REQ_STRUCT *DisassocReq,
	IN PUCHAR pAddr,
	IN USHORT Reason)
{
	COPY_MAC_ADDR(DisassocReq->Addr, pAddr);
	DisassocReq->Reason = Reason;
}


/*
	NOTE: we do have an assumption here, that Byte0 and Byte1
		always reasid at the same scatter gather buffer
 */
static inline VOID Sniff2BytesFromNdisBuffer(
	IN PNDIS_BUFFER buf,
	IN UCHAR offset,
	OUT UCHAR *p0,
	OUT UCHAR *p1)
{
	UCHAR *ptr = (UCHAR *)(buf + offset);
	*p0 = *ptr;
	*p1 = *(ptr + 1);
}


#define ETH_TYPE_VLAN	0x8100
#define ETH_TYPE_IPv4	0x0800
#define ETH_TYPE_IPv6	0x86dd
#define ETH_TYPE_ARP	0x0806
#define ETH_TYPE_EAPOL	0x888e
#define ETH_TYPE_WAI	0x88b4

#define IP_VER_CODE_V4	0x40
#define IP_VER_CODE_V6	0x60
#define IP_PROTO_UDP	0x11
#define IP_HDR_LEN		20
#define ETH_HDR_LEN		14

VOID CheckQosMapUP(PMAC_TABLE_ENTRY pEntry, UCHAR DSCP, PUCHAR pUserPriority)
{				
#if defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2)
	UCHAR i = 0, find_up = 0, dscpL = 0, dscpH = 0;
	MULTISSID_STRUCT *pMbss = NULL;
	RTMP_ADAPTER *pAd = NULL;
	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: %d Entry is NULL\n",__FUNCTION__, __LINE__));
		return;
	}

	if(IS_ENTRY_CLIENT(pEntry))
		pMbss = pEntry->pMbss;
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s: %d Entry is not client\n",__FUNCTION__, __LINE__));
		return;
	}
	
	if (pEntry->QosMapSupport && pMbss->HotSpotCtrl.QosMapEnable) {
		for (i=0;i<(pEntry->DscpExceptionCount/2);i++) {
			if ((pEntry->DscpException[i] & 0xff) == DSCP) {
				*pUserPriority = (pEntry->DscpException[i]>>8) & 0xff;
				find_up = 1;
				break;
			}
		}
					
		if (!find_up) {
			for (i=0;i<8;i++) {
				dscpL = pEntry->DscpRange[i] & 0xff;
				dscpH = (pEntry->DscpRange[i]>>8) & 0xff;	
				if ((DSCP <= dscpH) && (DSCP >= dscpL)) {
					*pUserPriority = i;
					break;
				}
			}
		}
	}
#endif /* defined(CONFIG_AP_SUPPORT) && defined(CONFIG_HOTSPOT_R2) */
}

#if defined (CONFIG_WIFI_PKT_FWD)
BOOLEAN is_gratuitous_arp(UCHAR *pData)
{
	UCHAR *Pos = pData;
	UINT16 ProtoType;
	UCHAR *SenderIP;
	UCHAR *TargetIP;

	NdisMoveMemory(&ProtoType, pData, 2);
	ProtoType = OS_NTOHS(ProtoType);
	Pos += 2;

	if (ProtoType == ETH_P_ARP)
	{
		/* 
 		 * Check if Gratuitous ARP, Sender IP equal Target IP
 		 */
		SenderIP = Pos + 14;
		TargetIP = Pos + 24;
		if (NdisCmpMemory(SenderIP, TargetIP, 4) == 0) {
			DBGPRINT(RT_DEBUG_TRACE, ("The Packet is GratuitousARP\n"));
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN is_dad_packet(RTMP_ADAPTER *pAd, UCHAR *pData)
{
	//UCHAR *Pos = pData;
	//UINT16 ProtoType;
	UCHAR isLinkValid;
	UCHAR *pSenderIP = pData + 16;
	//UCHAR *pTargetIP = pData + 26;
	UCHAR *pSourceMac = pData + 10;
	UCHAR *pDestMac = pData + 20;
	UCHAR ZERO_IP_ADDR[4] = {0x00, 0x00, 0x00, 0x00};
	UCHAR ZERO_MAC_ADDR[MAC_ADDR_LEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	UCHAR BROADCAST_ADDR[MAC_ADDR_LEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	/* 
	* Check if DAD packet
	*/
	if ((RTMPLookupRepeaterCliEntry(pAd, FALSE, pSourceMac, TRUE, &isLinkValid) != NULL) &&
		((MAC_ADDR_EQUAL(pDestMac, BROADCAST_ADDR) == TRUE) ||
		(MAC_ADDR_EQUAL(pDestMac, ZERO_MAC_ADDR) == TRUE)) &&
		(RTMPEqualMemory(pSenderIP, ZERO_IP_ADDR, 4) == TRUE)) {
		DBGPRINT(RT_DEBUG_TRACE, ("DAD found, and do not send this packet\n"));
		return TRUE;
	}

	return FALSE;
}
#endif /* CONFIG_WIFI_PKT_FWD */

BOOLEAN RTMPCheckEtherType(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET	pPacket,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN struct wifi_dev *wdev,
	OUT UCHAR *pUserPriority,
	OUT UCHAR *pQueIdx)
{
	UINT16 TypeLen;
	UCHAR Byte0, Byte1, *pSrcBuf, up = 0;
#ifdef CONFIG_AP_SUPPORT
	BOOLEAN isMcast = FALSE;
#endif /* CONFIG_AP_SUPPORT */

	pSrcBuf = GET_OS_PKT_DATAPTR(pPacket);
	ASSERT(pSrcBuf);

	RTMP_SET_PACKET_SPECIFIC(pPacket, 0);

#ifdef CONFIG_AP_SUPPORT
	if(IS_MULTICAST_MAC_ADDR(pSrcBuf))
		isMcast = TRUE;
#endif /* CONFIG_AP_SUPPORT */

	/* get Ethernet protocol field and skip the Ethernet Header */
	TypeLen = (pSrcBuf[12] << 8) | pSrcBuf[13];
	pSrcBuf += LENGTH_802_3;
	if (TypeLen <= 1500)
	{	/* 802.3, 802.3 LLC*/
		/*
			DestMAC(6) + SrcMAC(6) + Lenght(2) +
			DSAP(1) + SSAP(1) + Control(1) +
			if the DSAP = 0xAA, SSAP=0xAA, Contorl = 0x03, it has a 5-bytes SNAP header.
				=> + SNAP (5, OriginationID(3) + etherType(2))
			else
				=> It just has 3-byte LLC header, maybe a legacy ether type frame. we didn't handle it
		*/
		if (pSrcBuf[0] == 0xAA && pSrcBuf[1] == 0xAA && pSrcBuf[2] == 0x03)
		{
			Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 6, &Byte0, &Byte1);
			RTMP_SET_PACKET_LLCSNAP(pPacket, 1);
			TypeLen = (USHORT)((Byte0 << 8) + Byte1);
			pSrcBuf += 8; /* Skip this LLC/SNAP header*/
		} else {
			return FALSE;
		}
	}
	
	/* If it's a VLAN packet, get the real Type/Length field.*/
	if (TypeLen == ETH_TYPE_VLAN)
	{
#ifdef CONFIG_AP_SUPPORT
		USHORT VLAN_VID = 0;

		/*
			802.3 VLAN packets format:
			
			DstMAC(6B) + SrcMAC(6B)
			+ 802.1Q Tag Type (2B = 0x8100) + Tag Control Information (2-bytes) 
			+ Length/Type(2B) 
			+ data payload (42-1500 bytes)
			+ FCS(4B)

			VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID
		*/

		/* No matter unicast or multicast, discard it if not my VLAN packet. */
		VLAN_VID = wdev->VLAN_VID;
		if (VLAN_VID != 0)
		{
			/* check if the packet is my VLAN */
			/* VLAN tag: 3-bit UP + 1-bit CFI + 12-bit VLAN ID */
			USHORT vlan_id = *(USHORT *)pSrcBuf;

			vlan_id = cpu2be16(vlan_id);
			vlan_id = vlan_id & 0x0FFF; /* 12 bit */
			if (vlan_id != VLAN_VID)	/* not my VLAN packet, discard it */
				return FALSE;
		}
#endif /* CONFIG_AP_SUPPORT */

		RTMP_SET_PACKET_VLAN(pPacket, 1);
		Sniff2BytesFromNdisBuffer((PNDIS_BUFFER)pSrcBuf, 2, &Byte0, &Byte1);
		TypeLen = (USHORT)((Byte0 << 8) + Byte1);

		/* only use VLAN tag as UserPriority setting */
		up = (*pSrcBuf & 0xe0) >> 5;

		CheckQosMapUP(pMacEntry, (*pSrcBuf & 0xfc) >> 2, &up);

		pSrcBuf += 4; /* Skip the VLAN Header.*/
	} 
	else if (TypeLen == ETH_TYPE_IPv4)
	{
		/*
			0       4       8          14  15                      31(Bits)
			+---+----+-----+----+---------------+
			|Ver |  IHL |DSCP |ECN |    TotalLen           |
			+---+----+-----+----+---------------+
			Ver    - 4bit Internet Protocol version number.
			IHL    - 4bit Internet Header Length
			DSCP - 6bit Differentiated Services Code Point(TOS)
			ECN   - 2bit Explicit Congestion Notification
			TotalLen - 16bit IP entire packet length(include IP hdr)
		*/
		up = (*(pSrcBuf + 1) & 0xe0) >> 5;
		CheckQosMapUP(pMacEntry, (*(pSrcBuf+1) & 0xfc) >> 2, &up);
		
	}
	else if (TypeLen == ETH_TYPE_IPv6)
	{
		/*
			0       4       8        12     16                      31(Bits)
			+---+----+----+----+---------------+
			|Ver | TrafficClas |  Flow Label                   |
			+---+----+----+--------------------+
			Ver           - 4bit Internet Protocol version number.
			TrafficClas - 8bit traffic class field, the 6 most-significant bits used for DSCP
		*/
		up = ((*pSrcBuf) & 0x0e) >> 1;
		CheckQosMapUP(pMacEntry, ((*pSrcBuf & 0x0f) << 2)|((*(pSrcBuf+1)) & 0xc0) >> 6, &up);
	}

#ifdef RTMP_RBUS_SUPPORT
#ifdef VIDEO_TURBINE_SUPPORT
	if (pAd->VideoTurbine.Enable)
	{
		/* Ralink_VideoTurbine Out-band QoS */
		struct sk_buff *pSkbPkt = RTPKT_TO_OSPKT(pPacket);

		if(pSkbPkt->cb[40]==0x0E)
			up = (pSkbPkt->cb[41] & 0xe0) >> 5;
	}
#endif /* VIDEO_TURBINE_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT	
	if (wdev->wdev_type == WDEV_TYPE_AP)
	{
		MULTISSID_STRUCT *pMbss = (MULTISSID_STRUCT *)wdev->func_dev;
		if ((pMbss->FilterUnusedPacket & FILTER_IPV6_MC) == FILTER_IPV6_MC) 
		{
			if (isMcast && (TypeLen == ETH_TYPE_IPv6))
				return FALSE;
		}

		if ((pMbss->FilterUnusedPacket & FILTER_IPV4_MC) == FILTER_IPV4_MC)
		{
			if (isMcast && (TypeLen == ETH_TYPE_IPv4))
				return FALSE;
		}		

		if ((pMbss->FilterUnusedPacket & FILTER_IPV6_ALL) == FILTER_IPV6_ALL)
		{
			if (TypeLen == ETH_TYPE_IPv6)
				return FALSE;
		}				
	}	
#endif /* CONFIG_AP_SUPPORT */

	switch (TypeLen)
	{
		case ETH_TYPE_IPv4:
			{
				UINT8 ipv4_proto = *(pSrcBuf + 9);

				ASSERT((GET_OS_PKT_LEN(pPacket) > (ETH_HDR_LEN + IP_HDR_LEN)));	/* 14 for ethernet header, 20 for IP header*/
				RTMP_SET_PACKET_IPV4(pPacket, 1);

#ifdef DATA_QUEUE_RESERVE
				if (ipv4_proto == 0x01)
				{
					RTMP_SET_PACKET_ICMP(pPacket, 1);
				}
				else
#endif /* DATA_QUEUE_RESERVE */
				if (ipv4_proto == IP_PROTO_UDP)
				{
					UINT16 srcPort, dstPort;
	
					pSrcBuf += IP_HDR_LEN;
					srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
					dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf+2)));
		
					if ((srcPort==0x44 && dstPort==0x43) || (srcPort==0x43 && dstPort==0x44))
					{
						/*It's a BOOTP/DHCP packet*/
						RTMP_SET_PACKET_DHCP(pPacket, 1);
					}

#ifdef AIRPLAY_SUPPORT
					if ((srcPort==5353 && dstPort==5353))
					{
						DBGPRINT(RT_DEBUG_INFO, ("%s: this is mDns packet from upper layer ..... on V4\n", __FUNCTION__));
						RTMP_SET_PACKET_EAPOL(pPacket, 1);
					}		
#endif /* AIRPLAY_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
					if (wdev->wdev_type == WDEV_TYPE_AP)
					{
#ifdef CONFIG_DOT11V_WNM
						WNMIPv4ProxyARPCheck(pAd, pPacket, srcPort, dstPort, pSrcBuf); 
#endif /* CONFIG_DOT11V_WNM */
#ifdef CONFIG_HOTSPOT
						{
							USHORT Wcid = RTMP_GET_PACKET_WCID(pPacket);
							if (!HSIPv4Check(pAd, &Wcid, pPacket, pSrcBuf, srcPort, dstPort))
							return FALSE;
						}
#endif /* CONFIG_HOTSPOT */
					}
#endif /* CONFIG_AP_SUPPORT */
				}
			}
			break;
		case ETH_TYPE_ARP:
			{
#ifdef CONFIG_AP_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_AP)
				{
#ifdef CONFIG_DOT11V_WNM
					MULTISSID_STRUCT *pMbss = (MULTISSID_STRUCT *)wdev->func_dev;
					if (pMbss->WNMCtrl.ProxyARPEnable)
					{
						/* Check if IPv4 Proxy ARP Candidate from DS */
						if (IsIPv4ProxyARPCandidate(pAd, pSrcBuf - 2))
						{
							BOOLEAN FoundProxyARPEntry;
							FoundProxyARPEntry = IPv4ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE);
							if (!FoundProxyARPEntry)
								DBGPRINT(RT_DEBUG_TRACE, ("Can not find proxy entry\n"));

							return FALSE;
						}
					}
#endif /* CONFIG_DOT11V_WNM */
#ifdef CONFIG_HOTSPOT
					if (pMbss->HotSpotCtrl.HotSpotEnable)
					{
						if (!pMbss->HotSpotCtrl.DGAFDisable)
						{
							if (IsGratuitousARP(pAd, pSrcBuf - 2, pSrcBuf-14, pMbss))
								return FALSE;
						}
					}
#endif /* CONFIG_HOTSPOT */
				}
#endif /* CONFIG_AP_SUPPORT */
#if defined (CONFIG_WIFI_PKT_FWD)
				if (wdev->wdev_type == WDEV_TYPE_AP) {
					/* AP's tx shall check DAD.*/
					if (is_dad_packet(pAd, pSrcBuf - 2) || is_gratuitous_arp(pSrcBuf - 2))
						return FALSE;
				}
#endif /* CONFIG_WIFI_PKT_FWD */
				RTMP_SET_PACKET_DHCP(pPacket, 1);
			}
			break;
		case ETH_P_IPV6:
			{
#ifdef CONFIG_AP_SUPPORT
				if (wdev->wdev_type == WDEV_TYPE_AP)
				{
#ifdef CONFIG_DOT11V_WNM
					MULTISSID_STRUCT *pMbss = (MULTISSID_STRUCT *)wdev->func_dev;
					WNMIPv6ProxyARPCheck(pAd, pPacket, pSrcBuf);
					if (pMbss->WNMCtrl.ProxyARPEnable)
					{
						/* Check if IPv6 Proxy ARP Candidate from DS */
						if (IsIPv6ProxyARPCandidate(pAd, pSrcBuf - 2))
						{
							BOOLEAN FoundProxyARPEntry;
							FoundProxyARPEntry = IPv6ProxyARP(pAd, pMbss, pSrcBuf - 2, TRUE);
							if (!FoundProxyARPEntry)
								DBGPRINT(RT_DEBUG_TRACE, ("Can not find IPv6 proxy entry\n"));
	
							return FALSE;
						}
					}
#endif /* CONFIG_DOT11V_WNM */
#ifdef CONFIG_HOTSPOT
					if (pMbss->HotSpotCtrl.HotSpotEnable)
					{
						if (!pMbss->HotSpotCtrl.DGAFDisable)
						{
							if (IsUnsolicitedNeighborAdver(pAd, pSrcBuf - 2))
								return FALSE;
						}
					}
#endif /* CONFIG_HOTSPOT */
				}
				/* 
					Check if DHCPv6 Packet, and Convert group-address DHCP
					packets to individually-addressed 802.11 frames
 				 */
#endif /* CONFIG_AP_SUPPORT */
				/* return AC_BE if packet is not IPv6 */
				if ((*pSrcBuf & 0xf0) != 0x60)
					up = 0;

#ifdef AIRPLAY_SUPPORT					

				/* point to the Next Header */
				if (*(pSrcBuf + 6) == IP_PROTO_UDP)
				{
					UINT16 srcPort = 0, dstPort = 0;

					pSrcBuf += 40; /* IPV6 Header */
					srcPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf)));
					dstPort = OS_NTOHS(get_unaligned((PUINT16)(pSrcBuf+2)));

					if ((srcPort==5353 && dstPort==5353))
					{						
						DBGPRINT(RT_DEBUG_INFO, ("%s: this is mDns packet from upper layer ..... on V6\n", __FUNCTION__));
						RTMP_SET_PACKET_EAPOL(pPacket, 1);
					}	
				}	

#endif /* AIRPLAY_SUPPORT */				

			}
			break;
		case ETH_TYPE_EAPOL:
			RTMP_SET_PACKET_EAPOL(pPacket, 1);
			break;
#ifdef WAPI_SUPPORT
		case ETH_TYPE_WAI:
			RTMP_SET_PACKET_WAI(pPacket, 1);
			break;
#endif /* WAPI_SUPPORT */

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
		case 0x890d:
			{
				RTMP_SET_PACKET_TDLS(pPacket, 1);
				up = 5;
			}
			break;
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */

#ifdef SMART_MESH
        case ETH_TYPE_SMART_MESH:
            if(pMacEntry && IS_ENTRY_APCLI(pMacEntry))
                return FALSE;
            break;
#endif /* SMART_MESH */

		default:
			break;
	}

#ifdef VENDOR_FEATURE1_SUPPORT
	RTMP_SET_PACKET_PROTOCOL(pPacket, TypeLen);
#endif /* VENDOR_FEATURE1_SUPPORT */

	/* have to check ACM bit. downgrade UP & QueIdx before passing ACM*/
	/* NOTE: AP doesn't have to negotiate TSPEC. ACM is controlled purely via user setup, not protocol handshaking*/
	/*
		Under WMM ACM control, we dont need to check the bit;
		Or when a TSPEC is built for VO but we will change priority to
		BE here and when we issue a BA session, the BA session will
		be BE session, not VO session.
	*/
	if (pAd->CommonCfg.APEdcaParm.bACM[WMM_UP2AC_MAP[up]])
		up = 0;


	/*
		Set WMM when
		1. wdev->bWmmCapable == TRUE
		2. Receiver's capability
			a). bc/mc packets
				->Need to get UP for IGMP use
			b). unicast packets
				-> CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE)
		3. has VLAN tag or DSCP fields in IPv4/IPv6 hdr		
	*/
	if ((wdev->bWmmCapable == TRUE) && (up <= 7))
	{
		*pUserPriority = up;
		*pQueIdx = WMM_UP2AC_MAP[up];
	}

	return TRUE;
}

#ifdef FORCE_ANNOUNCE_CRITICAL_AMPDU
VOID RTMP_RxPacketClassify(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK		*pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry)
{
	PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
	UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

	if (protoType == ETH_P_ARP)
	{
		pRxBlk->CriticalPkt = 1;	// ARP

		DBGPRINT(RT_DEBUG_TRACE, ("rx path arp #(aid=%d,wcid=%d, pHeader seq=%d, ampdu = %d)\n",
			pEntry->Aid, pRxBlk->wcid, pRxBlk->pHeader->Sequence, RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU))); 
	}
	else if (protoType == ETH_P_IP)
	{
	
		UINT8 protocol = *(pData + 11);
		//UINT8 icmp_type = *(pData + 22);

		if (protocol == 0x1)
		{
			pRxBlk->CriticalPkt = 1;	// ICMP

			DBGPRINT(RT_DEBUG_TRACE, ("rx path PING #(aid=%d,wcid=%d, pHeader seq=%d, ampdu = %d)\n",
				pEntry->Aid, pRxBlk->wcid, pRxBlk->pHeader->Sequence, RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)));
		}
#if 0
		else if (protocol == IP_PROTO_UDP)
		{
			PUCHAR pUdpHdr = pData + 22;
			UINT16 srcPort, dstPort;

			srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
			dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));
			if ((srcPort==67 && dstPort==68)||(srcPort==68 && dstPort==67)) /*It's a DHCP packet */
			{
				pRxBlk->CriticalPkt = 1;	// DHCP

				DBGPRINT(RT_DEBUG_TRACE, ("rx path dhcp #(aid=%d,wcid=%d, pHeader seq=%d, ampdu = %d)\n",
					pEntry->Aid, pRxBlk->wcid, pRxBlk->pHeader->Sequence, RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU)));
			}
		}
#endif
	}
}
#endif /* FORCE_ANNOUNCE_CRITICAL_AMPDU */

VOID Update_Rssi_Sample(
	IN RTMP_ADAPTER *pAd,
	IN RSSI_SAMPLE *pRssi,
	IN RXWI_STRUC *pRxWI)
{
	CHAR rssi[3] = {0};
	UCHAR snr[3] = {0};
	BOOLEAN bInitial = FALSE;
	CHAR Phymode = get_pkt_phymode_by_rxwi(pAd, pRxWI);

	if (!(pRssi->AvgRssi0 | pRssi->AvgRssi0X8 | pRssi->LastRssi0))
		bInitial = TRUE;

	get_pkt_rssi_by_rxwi(pAd, pRxWI, 3, &rssi[0]);
	get_pkt_snr_by_rxwi(pAd, pRxWI, 3, &snr[0]);
	
	if (snr[0] != 0)
	{			
		pRssi->LastSnr0 = ConvertToSnr(pAd, (UCHAR)snr[0]);
		if (bInitial)
		{
			pRssi->AvgSnr0X8 = pRssi->LastSnr0 << 3;
			pRssi->AvgSnr0  = pRssi->LastSnr0;
		}
		else
			pRssi->AvgSnr0X8 = (pRssi->AvgSnr0X8 - pRssi->AvgSnr0) + pRssi->LastSnr0;

		pRssi->AvgSnr0 = pRssi->AvgSnr0X8 >> 3;
	}

	if (rssi[0] != 0)
	{
		pRssi->LastRssi0 = ConvertToRssi(pAd, (CHAR)rssi[0], RSSI_0);

#ifdef RLT_MAC
		if (IS_MT76x2(pAd)) {
			if (Phymode == MODE_CCK) {
				pRssi->LastRssi0 -= 2;
			}
		} 
#endif /* RLT_MAC */

		if (bInitial)
		{
			pRssi->AvgRssi0X8 = pRssi->LastRssi0 << 3;
			pRssi->AvgRssi0  = pRssi->LastRssi0;
		}
		else
			pRssi->AvgRssi0X8 = (pRssi->AvgRssi0X8 - pRssi->AvgRssi0) + pRssi->LastRssi0;
 
		pRssi->AvgRssi0 = pRssi->AvgRssi0X8 >> 3;
	}
	
	if (snr[1] != 0)
	{			
		pRssi->LastSnr1 = ConvertToSnr(pAd, (UCHAR)snr[1]);
		if (bInitial)
		{
			pRssi->AvgSnr1X8 = pRssi->LastSnr1 << 3;
			pRssi->AvgSnr1  = pRssi->LastSnr1;
		}
		else
			pRssi->AvgSnr1X8 = (pRssi->AvgSnr1X8 - pRssi->AvgSnr1) + pRssi->LastSnr1;
		
		pRssi->AvgSnr1 = pRssi->AvgSnr1X8 >> 3;
	}
	
	if (rssi[1] != 0)
	{   
		pRssi->LastRssi1 = ConvertToRssi(pAd, (CHAR)rssi[1], RSSI_1);
		
#ifdef RLT_MAC
		if (IS_MT76x2(pAd)) {
			if (Phymode == MODE_CCK) {
				pRssi->LastRssi1 -= 2;
			}
		}
#endif /* RLT_MAC */
 
		if (bInitial)
		{
			pRssi->AvgRssi1X8 = pRssi->LastRssi1 << 3;
			pRssi->AvgRssi1  = pRssi->LastRssi1;
		}
		else
			pRssi->AvgRssi1X8 = (pRssi->AvgRssi1X8 - pRssi->AvgRssi1) + pRssi->LastRssi1;

		pRssi->AvgRssi1 = pRssi->AvgRssi1X8 >> 3;
	}

	if (rssi[2] != 0)
	{
		pRssi->LastRssi2 = ConvertToRssi(pAd, (CHAR)rssi[2], RSSI_2);

		if (bInitial)
		{
			pRssi->AvgRssi2X8 = pRssi->LastRssi2 << 3;
			pRssi->AvgRssi2  = pRssi->LastRssi2;
		}
		else
			pRssi->AvgRssi2X8 = (pRssi->AvgRssi2X8 - pRssi->AvgRssi2) + pRssi->LastRssi2;

		pRssi->AvgRssi2 = pRssi->AvgRssi2X8 >> 3;
	}
}



/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Indicate_Legacy_Packet", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("802_11_hdr", (UCHAR *)pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug
#endif

#ifdef APCLI_SUPPORT
	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s:drop packet by PN mismatch!\n", __func__));
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
#endif /* APCLI_SUPPORT */

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/
#ifdef RT_CFG80211_SUPPORT
	CFG80211_Convert802_3Packet(pAd, pRxBlk, Header802_3);
#else
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN)
	{
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);


#ifdef CONFIG_AP_SUPPORT
	MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pRxBlk->pData, pRxBlk->DataSize);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug
#endif

	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), GET_OS_PKT_LEN(pRxBlk->pRxPacket));
}
//---Add by shiang for debug
#endif

	
	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
#ifdef RT_CFG80211_SUPPORT
	CFG80211_Announce802_3Packet(pAd, pRxBlk, FromWhichBSSID);
#else	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef MAC_REPEATER_SUPPORT /* This should be moved to some where else */
		if (pRxBlk->pRxInfo->Bcast && (pAd->ApCfg.bMACRepeaterEn) && (pAd->ApCfg.MACRepeaterOuiMode != 1))
		{
			PUCHAR pPktHdr, pLayerHdr;

			pPktHdr = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
			
			/*For UDP packet, we need to check about the DHCP packet. */
			if (*(pLayerHdr + 9) == 0x11)
			{
				PUCHAR pUdpHdr;
				UINT16 srcPort, dstPort;
				BOOLEAN bHdrChanged = FALSE;
				
				pUdpHdr = pLayerHdr + 20;
				srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
				dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));

				if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
				{
					PUCHAR bootpHdr, pCliHwAddr;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
					UCHAR isLinkValid;

					bootpHdr = pUdpHdr + 8;
					pCliHwAddr = (bootpHdr+28);
					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pCliHwAddr, TRUE, &isLinkValid);
					if (pReptEntry)
						NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
#if defined (CONFIG_WIFI_PKT_FWD)
					else
					{	
						REPEATER_ADAPTER_DATA_TABLE *opp_band_tbl = NULL;
						REPEATER_ADAPTER_DATA_TABLE *band_tbl = NULL;

						if (wf_fwd_feedback_map_table)
							wf_fwd_feedback_map_table(pAd, &band_tbl, &opp_band_tbl);
						
						if (opp_band_tbl != NULL) {
							/* 
								check the ReptTable of the opposite band due to dhcp packet (BC)
							    	may come-in 2/5G band when STA send dhcp broadcast to Root AP 
							*/
							
							pReptEntry = RTMPLookupRepeaterCliEntry(opp_band_tbl, FALSE, pCliHwAddr, FALSE, &isLinkValid);
							if (pReptEntry)
								NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						}
						else
							DBGPRINT(RT_DEBUG_INFO, ("cannot find the adapter of the oppsite band\n"));	
					}
#endif /* CONFIG_WIFI_PKT_FWD */
					bHdrChanged = TRUE;
				}
				
				if (bHdrChanged == TRUE)
					NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */
#endif /* RT_CFG80211_SUPPORT */
}


#ifdef HDR_TRANS_SUPPORT
/* Normal legacy Rx packet indication*/
VOID Indicate_Legacy_Packet_Hdr_Trns(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR Header802_3[LENGTH_802_3];
	USHORT VLAN_VID = 0, VLAN_Priority = 0;

	struct sk_buff *pOSPkt;

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Indicate_Legacy_Packet", pRxBlk->pTransData, pRxBlk->TransDataSize);
	hex_dump("802_11_hdr", pRxBlk->pHeader, LENGTH_802_11);
}
//---Add by shiang for debug
#endif

	/*
		1. get 802.3 Header
		2. remove LLC
			a. pointer pRxBlk->pData to payload
			b. modify pRxBlk->DataSize
	*/

	if (pRxBlk->TransDataSize > 1514 )
	{

		/* release packet*/
		RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	STATS_INC_RX_PACKETS(pAd, FromWhichBSSID);


#ifdef CONFIG_AP_SUPPORT
	MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("Before80211_2_8023", pRxBlk->pData, pRxBlk->TransDataSize);
	hex_dump("header802_3", &Header802_3[0], LENGTH_802_3);
}
//---Add by shiang for debug
#endif

	{
		pOSPkt = RTPKT_TO_OSPKT(pRxPacket);

		/*get_netdev_from_bssid(pAd, FromWhichBSSID); */
		pOSPkt->dev = get_netdev_from_bssid(pAd, FromWhichBSSID);
		pOSPkt->data = pRxBlk->pTransData;
		pOSPkt->len = pRxBlk->TransDataSize;
		pOSPkt->tail = pOSPkt->data + pOSPkt->len;
		//printk("\x1b[31m%s: rx trans ...%d\x1b[m\n", __FUNCTION__, __LINE__);
	}

#ifdef DBG
//+++Add by shiang for debug
if (0) {
	hex_dump("After80211_2_8023", GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket), GET_OS_PKT_LEN(pRxBlk->pRxPacket));
}
//---Add by shiang for debug
#endif

	/* pass this 802.3 packet to upper layer or forward this packet to WM directly*/
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxPacket, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */

}
#endif /* HDR_TRANS_SUPPORT */


/* Normal, AMPDU or AMSDU*/
VOID CmmRxnonRalinkFrameIndicate(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
#ifdef DOT11_N_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet(pAd, pRxBlk, FromWhichBSSID);
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		else
#endif /* DOT11_N_SUPPORT */
		{
			Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}


/* Normal, AMPDU or AMSDU*/
#ifdef HDR_TRANS_SUPPORT
VOID CmmRxnonRalinkFrameIndicate_Hdr_Trns(
	IN	PRTMP_ADAPTER	pAd,
	IN	RX_BLK			*pRxBlk,
	IN	UCHAR			FromWhichBSSID)
{
#ifdef DOT11_N_SUPPORT
	if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMPDU) && (pAd->CommonCfg.bDisableReordering == 0))
	{
		Indicate_AMPDU_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
	}
	else
#endif /* DOT11_N_SUPPORT */
	{
#ifdef DOT11_N_SUPPORT
		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_AMSDU))
			Indicate_AMSDU_Packet(pAd, pRxBlk, FromWhichBSSID);
		else
#endif /* DOT11_N_SUPPORT */
		{
			Indicate_Legacy_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
		}
	}
}
#endif /* HDR_TRANS_SUPPORT */


VOID CmmRxRalinkFrameIndicate(
	IN	RTMP_ADAPTER *pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN	RX_BLK *pRxBlk,
	IN	UCHAR FromWhichBSSID)
{
	UCHAR			Header802_3[LENGTH_802_3];
	UINT16			Msdu2Size;
	UINT16 			Payload1Size, Payload2Size;
	PUCHAR 			pData2;
	PNDIS_PACKET	pPacket2 = NULL;
	USHORT			VLAN_VID = 0, VLAN_Priority = 0;


	Msdu2Size = *(pRxBlk->pData) + (*(pRxBlk->pData+1) << 8);
	if ((Msdu2Size <= 1536) && (Msdu2Size < pRxBlk->DataSize))
	{
		/* skip two byte MSDU2 len */
		pRxBlk->pData += 2;
		pRxBlk->DataSize -= 2;
	}
	else
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* get 802.3 Header and  remove LLC*/
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		RTMP_AP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		RTMP_802_11_REMOVE_LLC_AND_CONVERT_TO_802_3(pRxBlk, Header802_3);
#endif /* CONFIG_STA_SUPPORT */


	ASSERT(pRxBlk->pRxPacket);

	/* Ralink Aggregation frame*/
	pAd->RalinkCounters.OneSecRxAggregationCount ++;
	Payload1Size = pRxBlk->DataSize - Msdu2Size;
	Payload2Size = Msdu2Size - LENGTH_802_3;

	pData2 = pRxBlk->pData + Payload1Size + LENGTH_802_3;
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		USHORT VLAN_VID = 0, VLAN_Priority = 0;

		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#ifdef WDS_VLAN_SUPPORT
	if (VLAN_VID == 0) /* maybe WDS packet */
		WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */

		RT_VLAN_PKT_DUPLICATE(pPacket2, pAd, VLAN_VID, VLAN_Priority,
							(pData2-LENGTH_802_3), LENGTH_802_3, pData2,
							Payload2Size, FromWhichBSSID, TPID);
	}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		pPacket2 = duplicate_pkt(get_netdev_from_bssid(pAd, FromWhichBSSID),
								(pData2-LENGTH_802_3), LENGTH_802_3, pData2,
								Payload2Size, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */

	if (!pPacket2)
	{
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	/* update payload size of 1st packet*/
	pRxBlk->DataSize = Payload1Size;
	
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MBSS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);

#ifdef WDS_VLAN_SUPPORT
		if (VLAN_VID == 0) /* maybe WDS packet */
			WDS_VLAN_INFO_GET(pAd, VLAN_VID, VLAN_Priority, FromWhichBSSID);
#endif /* WDS_VLAN_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


	RT_80211_TO_8023_PACKET(pAd, VLAN_VID, VLAN_Priority,
							pRxBlk, Header802_3, FromWhichBSSID, TPID);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pRxBlk->pRxPacket, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */

	if (pPacket2)
	{
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			AP_ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			ANNOUNCE_OR_FORWARD_802_3_PACKET(pAd, pPacket2, FromWhichBSSID);
#endif /* CONFIG_STA_SUPPORT */
	}
}


#define RESET_FRAGFRAME(_fragFrame) \
	{								\
		_fragFrame.RxSize = 0;		\
		_fragFrame.Sequence = 0;	\
		_fragFrame.LastFrag = 0;	\
		_fragFrame.Flags = 0;		\
	}


PNDIS_PACKET RTMPDeFragmentDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	UCHAR *pData = pRxBlk->pData;
	USHORT DataSize = pRxBlk->DataSize;
	PNDIS_PACKET pRetPacket = NULL;
	UCHAR *pFragBuffer = NULL;
	BOOLEAN bReassDone = FALSE;
	UCHAR HeaderRoom = 0;
	RXWI_STRUC *pRxWI = pRxBlk->pRxWI;
	UINT8 RXWISize = pAd->chipCap.RXWISize;

	ASSERT(pHeader);

	HeaderRoom = pData - (UCHAR *)pHeader;

	/* Re-assemble the fragmented packets*/
	if (pHeader->Frag == 0)
	{	/* Frag. Number is 0 : First frag or only one pkt*/
		/* the first pkt of fragment, record it.*/
		if (pHeader->FC.MoreFrag)
		{
			ASSERT(pAd->FragFrame.pFragPacket);
			pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items.
			    Copy RxWI content to pFragBuffer.
			*/
			//pAd->FragFrame.RxSize = DataSize + HeaderRoom;
			//NdisMoveMemory(pFragBuffer, pHeader, pAd->FragFrame.RxSize);
			pAd->FragFrame.RxSize = DataSize + HeaderRoom + RXWISize;
			NdisMoveMemory(pFragBuffer, pRxWI, RXWISize);
			NdisMoveMemory(pFragBuffer + RXWISize,	 pHeader, pAd->FragFrame.RxSize - RXWISize);
			pAd->FragFrame.Sequence = pHeader->Sequence;
			pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Should be 0*/
			ASSERT(pAd->FragFrame.LastFrag == 0);
			goto done;	/* end of processing this frame*/
		}
	}
	else	
	{	/*Middle & End of fragment*/
		if ((pHeader->Sequence != pAd->FragFrame.Sequence) ||
			(pHeader->Frag != (pAd->FragFrame.LastFrag + 1)))
		{
			/* Fragment is not the same sequence or out of fragment number order*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment is not the same sequence or out of fragment number order.\n"));
			goto done;
		}
		/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
		//else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE)
		else if ((pAd->FragFrame.RxSize + DataSize) > MAX_FRAME_SIZE + RXWISize)
		{
			/* Fragment frame is too large, it exeeds the maximum frame size.*/
			/* Reset Fragment control blk*/
			RESET_FRAGFRAME(pAd->FragFrame);
			DBGPRINT(RT_DEBUG_ERROR, ("Fragment frame is too large, it exeeds the maximum frame size.\n"));
			goto done;
		}

        
		/* Broadcom AP(BCM94704AGR) will send out LLC in fragment's packet, LLC only can accpet at first fragment.*/
		/* In this case, we will drop it.*/
		if (NdisEqualMemory(pData, SNAP_802_1H, sizeof(SNAP_802_1H)))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Find another LLC at Middle or End fragment(SN=%d, Frag=%d)\n", pHeader->Sequence, pHeader->Frag));
			goto done;
		}

		pFragBuffer = GET_OS_PKT_DATAPTR(pAd->FragFrame.pFragPacket);

		/* concatenate this fragment into the re-assembly buffer*/
		NdisMoveMemory((pFragBuffer + pAd->FragFrame.RxSize), pData, DataSize);
		pAd->FragFrame.RxSize  += DataSize;
		pAd->FragFrame.LastFrag = pHeader->Frag;	   /* Update fragment number*/

		/* Last fragment*/
		if (pHeader->FC.MoreFrag == FALSE)
			bReassDone = TRUE;
	}

done:
	/* always release rx fragmented packet*/
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

	/* return defragmented packet if packet is reassembled completely*/
	/* otherwise return NULL*/
	if (bReassDone)
	{
		PNDIS_PACKET pNewFragPacket;

		/* allocate a new packet buffer for fragment*/
		pNewFragPacket = RTMP_AllocateFragPacketBuffer(pAd, RX_BUFFER_NORMSIZE);
		if (pNewFragPacket)
		{
			/* update RxBlk*/
			pRetPacket = pAd->FragFrame.pFragPacket;
			pAd->FragFrame.pFragPacket = pNewFragPacket;
			/* Fix MT5396 crash issue when Rx fragmentation frame for Wi-Fi TGn 5.2.4 & 5.2.13 test items. */
			//pRxBlk->pHeader = (PHEADER_802_11) GET_OS_PKT_DATAPTR(pRetPacket);
			//pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			//pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom;
			//pRxBlk->pRxPacket = pRetPacket;
			pRxBlk->pRxWI = (RXWI_STRUC *) GET_OS_PKT_DATAPTR(pRetPacket);
			pRxBlk->pHeader = (PHEADER_802_11) ((UCHAR *)pRxBlk->pRxWI + RXWISize);
			pRxBlk->pData = (UCHAR *)pRxBlk->pHeader + HeaderRoom;
			pRxBlk->DataSize = pAd->FragFrame.RxSize - HeaderRoom - RXWISize;
			pRxBlk->pRxPacket = pRetPacket;
		}
		else
		{
			RESET_FRAGFRAME(pAd->FragFrame);
		}
	}

	return pRetPacket;
}


VOID Indicate_EAPOL_Packet(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN UCHAR FromWhichBSSID)
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: invalid wcid.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}

	pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	if (pEntry == NULL)
	{
		DBGPRINT(RT_DEBUG_WARN, ("Indicate_EAPOL_Packet: drop and release the invalid packet.\n"));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}
#ifdef APCLI_SUPPORT
	if (check_rx_pkt_pn_allowed(pAd, pRxBlk) == FALSE) {
		DBGPRINT(RT_DEBUG_WARN, ("%s:drop packet by PN mismatch!\n", __func__));
		RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
		return;
	}	
#endif /* APCLI_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{		
		APRxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
		return;
	}
#endif /* CONFIG_AP_SUPPORT */
	
#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
#if defined(P2P_APCLI_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT)
		if (IS_PKT_OPMODE_AP(pRxBlk))
		{	
			APRxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
		}
		else
#endif /* P2P_SUPPORT */
		{
			ASSERT((pRxBlk->wcid == BSSID_WCID));
			STARxEAPOLFrameIndicate(pAd, pEntry, pRxBlk, FromWhichBSSID);
			return;
		}
	}
#endif /* CONFIG_STA_SUPPORT */

}


#ifdef SOFT_ENCRYPT
BOOLEAN RTMPExpandPacketForSwEncrypt(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	UINT32	ex_head = 0, ex_tail = 0;
	UCHAR 	NumberOfFrag = RTMP_GET_PACKET_FRAGMENTS(pTxBlk->pPacket);

#ifdef WAPI_SUPPORT
	if (pTxBlk->CipherAlg == CIPHER_SMS4)
		ex_tail = LEN_WPI_MIC;
	else
#endif /* WAPI_SUPPORT */			
	if (pTxBlk->CipherAlg == CIPHER_AES)
		ex_tail = LEN_CCMP_MIC;

	ex_tail = (NumberOfFrag * ex_tail);

	pTxBlk->pPacket = ExpandPacket(pAd, pTxBlk->pPacket, ex_head, ex_tail);
	if (pTxBlk->pPacket == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s: out of resource.\n", __FUNCTION__));
		return FALSE;
	}
	RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);									

	return TRUE;
}


VOID RTMPUpdateSwCacheCipherInfo(	
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHdr)
{
	HEADER_802_11 *pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader_802_11 = (HEADER_802_11 *) pHdr;
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry && pHeader_802_11->FC.Wep && 
		CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))	
	{
		PCIPHER_KEY pKey = &pMacEntry->PairwiseKey;
	
		TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

		pTxBlk->CipherAlg = pKey->CipherAlg;
		pTxBlk->pKey = pKey;
#ifdef WAPI_SUPPORT
		pTxBlk->KeyIdx = pMacEntry->usk_id;

		/* TSC increment pre encryption transmittion */
		if (pKey->CipherAlg == CIPHER_SMS4)						
			inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);			
		else 
#endif /* WAPI_SUPPORT */		
		if ((pKey->CipherAlg == CIPHER_WEP64) || (pKey->CipherAlg == CIPHER_WEP128))
			inc_iv_byte(pKey->TxTsc, LEN_WEP_TSC, 1);
		else if ((pKey->CipherAlg == CIPHER_TKIP) || (pKey->CipherAlg == CIPHER_AES))
			inc_iv_byte(pKey->TxTsc, LEN_WPA_TSC, 1);
		
	}

}

#endif /* SOFT_ENCRYPT */

/* 
	==========================================================================
	Description:
		Send out a NULL frame to a specified STA at a higher TX rate. The 
		purpose is to ensure the designated client is okay to received at this
		rate.
	==========================================================================
 */
VOID RtmpEnqueueNullFrame(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR TxRate,
	IN UCHAR AID,
	IN UCHAR apidx,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP)
{
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	UCHAR *pFrame;
	UINT frm_len;
	MAC_TABLE_ENTRY *pEntry;

	NState = MlmeAllocateMemory(pAd, (UCHAR **)&pFrame);
	pNullFr = (PHEADER_802_11) pFrame;

	if (NState == NDIS_STATUS_SUCCESS) 
	{
		frm_len = sizeof(HEADER_802_11);

		pEntry = MacTableLookup(pAd, pAddr);
#ifdef CONFIG_AP_SUPPORT
		if ((pEntry != NULL) && (IS_ENTRY_CLIENT(pEntry)))
		{
			MgtMacHeaderInit(pAd, pNullFr, SUBTYPE_DATA_NULL, 0, pAddr, 
							pAd->ApCfg.MBSSID[apidx].wdev.if_addr,
							pAd->ApCfg.MBSSID[apidx].wdev.bssid);
			pNullFr->FC.FrDs = 1;
			pNullFr->FC.ToDs = 0;	
			
			goto body;
		}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd) {
			pNullFr->FC.FrDs = 0;
			pNullFr->FC.ToDs = 1;

#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			if ((pEntry != NULL) && (IS_ENTRY_TDLS(pEntry)))
			{
				pNullFr->FC.FrDs = 0;
				pNullFr->FC.ToDs = 0;

				COPY_MAC_ADDR(pNullFr->Addr1, pAddr);
				COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
				COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);
			}
#endif /* DOT11Z_TDLS_SUPPORT */
		}
#endif /* CONFIG_STA_SUPPORT */

body:
		pNullFr->FC.Type = FC_TYPE_DATA;

		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + frm_len;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			frm_len += 2;
		} else
			pNullFr->FC.SubType = SUBTYPE_DATA_NULL;

		/* since TxRate may change, we have to change Duration each time */
		pNullFr->Duration = RTMPCalcDuration(pAd, TxRate, frm_len);

		DBGPRINT(RT_DEBUG_INFO, ("send NULL Frame @%d Mbps to AID#%d...\n", RateIdToMbps[TxRate], AID & 0x3f));
		MiniportMMRequest(pAd, WMM_UP2AC_MAP[7], (PUCHAR)pNullFr, frm_len);

		MlmeFreeMemory(pAd, pFrame);
	}
}


VOID RtmpPrepareHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN BOOLEAN bQosNull,
	IN BOOLEAN bEOSP,
	IN UCHAR OldUP,
	IN UCHAR OpMode,
	IN UCHAR PwrMgmt,
	IN BOOLEAN bWaitACK,
	IN CHAR Index)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TXWI_STRUC *pTxWI = &pAd->NullTxWI;
	PUCHAR pNullFrame;
	NDIS_STATUS NState;
	HEADER_802_11 *pNullFr;
	ULONG Length;
	UCHAR *ptr;
	UINT i;
	UINT32 longValue;

	NState = MlmeAllocateMemory(pAd, (PUCHAR *)&pNullFrame);

	NdisZeroMemory(pNullFrame, 48);
	NdisZeroMemory(pTxWI, TXWISize);

	if (NState == NDIS_STATUS_SUCCESS) 
	{
		pNullFr = (PHEADER_802_11) pNullFrame;
		Length = sizeof(HEADER_802_11);
		
		pNullFr->FC.Type = FC_TYPE_DATA;
		pNullFr->FC.SubType = SUBTYPE_DATA_NULL;
		if (Index == 1)
			pNullFr->FC.ToDs = 0;
		else
		pNullFr->FC.ToDs = 1;
		pNullFr->FC.FrDs = 0;

		COPY_MAC_ADDR(pNullFr->Addr1, pEntry->Addr);
		{
			COPY_MAC_ADDR(pNullFr->Addr2, pAd->CurrentAddress);
			COPY_MAC_ADDR(pNullFr->Addr3, pAd->CommonCfg.Bssid);
		}

		pNullFr->FC.PwrMgmt = PwrMgmt;
	
		pNullFr->Duration = pAd->CommonCfg.Dsifs + RTMPCalcDuration(pAd, pAd->CommonCfg.TxRate, 14);

		/* sequence is increased in MlmeHardTx */
		pNullFr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence  */

		if (bQosNull)
		{
			UCHAR *qos_p = ((UCHAR *)pNullFr) + Length;

			pNullFr->FC.SubType = SUBTYPE_QOS_NULL;

			/* copy QOS control bytes */
			qos_p[0] = ((bEOSP) ? (1 << 4) : 0) | OldUP;
			qos_p[1] = 0;
			Length += 2;
		}

		if (Index == 1)
		{
			HTTRANSMIT_SETTING Transmit;

			Transmit.word = pEntry->MaxHTPhyMode.word;
			Transmit.field.BW = 0;
			if (Transmit.field.MCS > 7)
				Transmit.field.MCS = 7;

			RTMPWriteTxWI(pAd,
						pTxWI,
						FALSE,
						FALSE,
						FALSE,
						FALSE,
						TRUE,
						TRUE,
						0,
						pEntry->Aid,
						Length,
						(UCHAR)Transmit.field.MCS,
						0,
						(UCHAR)Transmit.field.MCS,
						IFS_HTTXOP,
						&Transmit);
		}
		else
		{
			RTMPWriteTxWI(pAd,
						pTxWI,
						FALSE,
						FALSE,
						FALSE,
						FALSE,
						TRUE,
						0,
						0,
						pEntry->Aid,
						Length,
						(UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS,
						0,
						(UCHAR)pAd->CommonCfg.MlmeTransmit.field.MCS,
						IFS_HTTXOP,
						&pAd->CommonCfg.MlmeTransmit);
		}

		if (bWaitACK) {
#ifdef RTMP_MAC
			if (pAd->chipCap.hif_type == HIF_RTMP)
				pTxWI->TXWI_O.TXRPT = 1;
#endif /* RTMP_MAC */
#ifdef RLT_MAC
			// TODO: shiang, how about RT65xx series??
#endif /* RLT_MAC */

		}

		ptr = (PUCHAR)&pAd->NullTxWI;
#ifdef RT_BIG_ENDIAN
		RTMPWIEndianChange(pAd, ptr, TYPE_TXWI);
#endif /* RT_BIG_ENDIAN */
		for (i=0; i < TXWISize; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0)
				RTMP_IO_WRITE32(pAd, pAd->chipCap.BcnBase[14] + i, longValue);
			else if (Index == 1)
				RTMP_IO_WRITE32(pAd, pAd->chipCap.BcnBase[15] + i, longValue);
				
			ptr += 4;
		}
		
		ptr = pNullFrame;
#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, ptr, DIR_WRITE, FALSE);
#endif /* RT_BIG_ENDIAN */
		for (i= 0; i< Length; i+=4)
		{
			longValue =  *ptr + (*(ptr + 1) << 8) + (*(ptr + 2) << 16) + (*(ptr + 3) << 24);
			if (Index == 0) //for ra0 
				RTMP_IO_WRITE32(pAd, pAd->chipCap.BcnBase[14] + TXWISize+ i, longValue);
			else if (Index == 1) //for p2p0
				RTMP_IO_WRITE32(pAd, pAd->chipCap.BcnBase[15] + TXWISize+ i, longValue);
				
			ptr += 4;
		}
	}

	if (pNullFrame)
		MlmeFreeMemory(pAd, pNullFrame);

}


// TODO: shiang-usw, modify the op_mode assignment for this function!!!
VOID dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;
	INT op_mode = pRxBlk->OpMode;
	BOOLEAN 	bPassTheBcastPkt = FALSE;

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT	
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif /* APCLI_CERT_SUPPOR */
#endif /* APCLI_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
	if (CFG80211_HandleP2pMgmtFrame(pAd, pRxBlk, op_mode))
		goto done;
#endif /* RT_CFG80211_SUPPORT */


#ifdef DOT11W_PMF_SUPPORT
	if (PMF_PerformRxFrameAction(pAd, pRxBlk) == FALSE)
		goto done;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

		op_mode = OPMODE_AP;
#ifdef IDS_SUPPORT
		/*
			Check if a rogue AP impersonats our mgmt frame to spoof clients
			drop it if it's a spoofed frame
		*/
		if (RTMPSpoofedMgmtDetection(pAd, pHeader, pRxBlk))
			goto done;

		/* update sta statistics for traffic flooding detection later */
		RTMPUpdateStaMgmtCounter(pAd, pHeader->FC.SubType);
#endif /* IDS_SUPPORT */

		if (!pRxInfo->U2M)
		{
			if ((pHeader->FC.SubType != SUBTYPE_BEACON) && (pHeader->FC.SubType != SUBTYPE_PROBE_REQ))
			{

				if (pHeader->FC.SubType == SUBTYPE_ACTION)
				{					
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
					if  (pAd->bApCliCertTest == TRUE)
					{
						INT i;

						for (i = 0; i < MAX_APCLI_NUM; i++)
						{
							pApCliEntry = &pAd->ApCfg.ApCliTab[i];
							if (MAC_ADDR_EQUAL(pApCliEntry->wdev.if_addr, pHeader->Addr1))
								bPassTheBcastPkt = TRUE; /* Let this Action Frame pass */
						}						
					}					
#endif /* APCLI_CERT_SUPPOR */
#endif /* APCLI_SUPPORT */						

					if (!bPassTheBcastPkt)
						goto done; /* Skip this packet */
				}
				else
					goto done; /* Skip this packet */
			}
		}

		if (pAd->ApCfg.BANClass3Data == TRUE)
		{
			/* disallow new association */
			if ((pHeader->FC.SubType == SUBTYPE_ASSOC_REQ) || (pHeader->FC.SubType == SUBTYPE_AUTH))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Disallow new Association\n"));
				goto done;
			}
		}

		/* Software decrypts WEP data during shared WEP negotiation */
		if ((pHeader->FC.SubType == SUBTYPE_AUTH) && 
			(pHeader->FC.Wep == 1) && (pRxInfo->Decrypted == 0))
		{
			UCHAR *pMgmt = (PUCHAR)pHeader;
			UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;
			MAC_TABLE_ENTRY *pEntry = NULL;
				
			/* Skip 802.11 headre */
			pMgmt += LENGTH_802_11;
			mgmt_len -= LENGTH_802_11;

			if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			else
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails - the Entry is empty.\n"));
				goto done;
			}

			/* handle WEP decryption */
			if (RTMPSoftDecryptWEP(pAd, 
								   &pAd->SharedKey[pEntry->apidx][pRxBlk->key_idx], 
								   pMgmt, 
								   &mgmt_len) == FALSE)		
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ERROR: SW decrypt WEP data fails.\n"));	
				goto done;
			}
#ifdef RT_BIG_ENDIAN
			/* swap 16 bit fields - Auth Alg No. field */
			*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);

			/* swap 16 bit fields - Auth Seq No. field */
			*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));

			/* swap 16 bit fields - Status Code field */
			*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */
													
			DBGPRINT(RT_DEBUG_TRACE, ("Decrypt AUTH seq#3 successfully\n"));	

			/* Update the total length */
			pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);			
		}
	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		op_mode = OPMODE_STA;

#ifdef RT_CFG80211_P2P_SUPPORT		
		/* CFG_TODO */
		op_mode = pRxBlk->OpMode;
#endif /* RT_CFG80211_P2P_SUPPORT */

		/* check if need to resend PS Poll when received packet with MoreData = 1 */
		if ((RtmpPktPmBitCheck(pAd) == TRUE) && (pHeader->FC.MoreData == 1)) {
			/* for UAPSD, all management frames will be VO priority */
			if (pAd->CommonCfg.bAPSDAC_VO == 0) {
				/* non-UAPSD delivery-enabled AC */
				RTMP_PS_POLL_ENQUEUE(pAd);
			}
		}

		/* TODO: if MoreData == 0, station can go to sleep */

		/* We should collect RSSI not only U2M data but also my beacon */
		if ((pHeader->FC.SubType == SUBTYPE_BEACON)
		    && (MAC_ADDR_EQUAL(&pAd->CommonCfg.Bssid, &pHeader->Addr2))
		    && (pAd->RxAnt.EvaluatePeriod == 0)) {
			Update_Rssi_Sample(pAd, &pAd->StaCfg.RssiSample, pRxBlk->pRxWI);

			pAd->StaCfg.LastSNR0 = (UCHAR) (pRxBlk->snr[0]);
			pAd->StaCfg.LastSNR1 = (UCHAR) (pRxBlk->snr[1]);
#ifdef DOT11N_SS3_SUPPORT
			pAd->StaCfg.LastSNR2 = (UCHAR) (pRxBlk->snr[2]);
#endif /* DOT11N_SS3_SUPPORT */

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */
		}

		if ((pHeader->FC.SubType == SUBTYPE_BEACON) &&
		    (ADHOC_ON(pAd)) &&
		    (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)) {
			MAC_TABLE_ENTRY *pEntry = NULL;
			pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
			if (pEntry)
				Update_Rssi_Sample(pAd, &pEntry->RssiSample, pRxBlk->pRxWI);
		}
	}
#endif /* CONFIG_STA_SUPPORT */

	if (pRxBlk->DataSize > MAX_RX_PKT_LEN) {
		DBGPRINT(RT_DEBUG_TRACE, ("DataSize=%d\n", pRxBlk->DataSize));
		hex_dump("MGMT ???", (UCHAR *)pHeader, pRxBlk->pData - (UCHAR *) pHeader);
		goto done;
	}

#if defined(CONFIG_AP_SUPPORT) || defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
	if (pHeader->FC.SubType == SUBTYPE_ACTION)
	{
		MAC_TABLE_ENTRY *pMacEntry = NULL;

		/* only PM bit of ACTION frame can be set */
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
			pMacEntry = &pAd->MacTab.Content[pRxBlk->wcid];

		if (pMacEntry && ((op_mode == OPMODE_AP && IS_ENTRY_CLIENT(pMacEntry))
#if defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT)
			|| (op_mode == OPMODE_STA && IS_ENTRY_TDLS(pMacEntry))
#endif /* defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
		))
		   	RtmpPsIndicate(pAd, pHeader->Addr2, pRxBlk->wcid, pHeader->FC.PwrMgmt);

		/*
			In IEEE802.11, 11.2.1.1 STA Power Management modes,
			The Power Managment bit shall not be set in any management
			frame, except an Action frame.

			In IEEE802.11e, 11.2.1.4 Power management with APSD,
			If there is no unscheduled SP in progress, the unscheduled SP
			begins when the QAP receives a trigger frame from a non-AP QSTA,
			which is a QoS data or QoS Null frame associated with an AC the
			STA has configured to be trigger-enabled.
			So a management action frame is not trigger frame.
		*/
	}
#endif /* defined(CONFIG_AP_SUPPORT) defined(DOT11Z_TDLS_SUPPORT) || defined(CFG_TDLS_SUPPORT) */
#ifdef SMART_MESH_MONITOR
	HTTRANSMIT_SETTING HTSetting;
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		HTSetting.field.MODE = pRxBlk->pRxWI->RXWI_N.phy_mode;
		HTSetting.field.MCS = pRxBlk->pRxWI->RXWI_N.mcs;
		HTSetting.field.BW = pRxBlk->pRxWI->RXWI_N.bw;
		HTSetting.field.ShortGI = pRxBlk->pRxWI->RXWI_N.sgi;	
	}
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		HTSetting.field.MODE = pRxBlk->pRxWI->RXWI_O.phy_mode;
		HTSetting.field.MCS = pRxBlk->pRxWI->RXWI_O.mcs;
		HTSetting.field.BW = pRxBlk->pRxWI->RXWI_O.bw;
		HTSetting.field.ShortGI = pRxBlk->pRxWI->RXWI_O.sgi;
	}
#endif /* RTMP_MAC */
	getRate(HTSetting, &pAd->LastMgmtRxRate);

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
	{
		if (IS_MT76x2(pAd))
		{
			pRxBlk->snr[0] = pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[2];
			pRxBlk->snr[1] = pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[2];
		}
	}
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
	{
		if (IS_RT6352(pAd))
		{
			pRxBlk->snr[1] = pRxBlk->snr[0];
		}
	}
#endif /* RTMP_MAC */
#endif /*SMART_MESH_MONITOR*/

#ifdef CUSTOMER_DCC_FEATURE
	REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid, pHeader,
					pRxBlk->DataSize,
					pRxBlk->rssi[0], pRxBlk->rssi[1], pRxBlk->rssi[2],
					pRxBlk->snr[0], pRxBlk->snr[1], min((CHAR)pRxBlk->snr[0], (CHAR)pRxBlk->snr[1]),
					op_mode);
#else
	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid, pHeader,
					pRxBlk->DataSize,
					pRxBlk->rssi[0], pRxBlk->rssi[1], pRxBlk->rssi[2],
					min((CHAR)pRxBlk->snr[0], (CHAR)pRxBlk->snr[1]),
					op_mode);
#endif
#ifdef TXBF_SUPPORT
	if (pAd->chipCap.FlgHwTxBfCap)
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
		if (pHeader->FC.Order)
		{
			handleHtcField(pAd, pRxBlk);
			pRxBlk->pData += 4;
			pRxBlk->DataSize -= 4;
		}

		/* Check for compressed or non-compressed Sounding Response */
		if (((pHeader->FC.SubType == SUBTYPE_ACTION) || (pHeader->FC.SubType == SUBTYPE_ACTION_NO_ACK)) 
			&& (pRxBlk->pData[0] == CATEGORY_HT)
			&& ((pRxBlk->pData[1] == MIMO_N_BEACONFORM) || (pRxBlk ->pData[1] == MIMO_BEACONFORM))
		)
		{
			handleBfFb(pAd, pRxBlk);
		}
	}
#endif /* TXBF_SUPPORT */

done:
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
	// management frame RX counter per BSS
	if(!(ApScanRunning(pAd)))
	{
		MAC_TABLE_ENTRY *pEntry = NULL;
		pEntry = PACInquiry(pAd, pRxBlk->wcid);
		pAd->RadioStatsCounter.TotalRxCount++;
		if (pEntry != NULL)
		{
			if (pHeader->FC.FrDs == 0 && (pEntry->apidx < pAd->ApCfg.BssidNum))
			{
				MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];
				if (pMbss != NULL)
				{
					pMbss->MGMTReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
					pMbss->MGMTRxCount++;
					{
						UINT32 Length, Index;
						HTTRANSMIT_SETTING HTSetting;
						NdisZeroMemory(&HTSetting, sizeof(HTTRANSMIT_SETTING));
						HTSetting.field.MODE = pRxBlk->rx_rate.field.MODE;
						HTSetting.field.BW = pRxBlk->rx_rate.field.BW;
						HTSetting.field.ShortGI = pRxBlk->rx_rate.field.ShortGI;
						HTSetting.field.MCS = pRxBlk->rx_rate.field.MCS;
						Length = pRxBlk->MPDUtotalByteCnt;
						GetMultShiftFactorIndex(HTSetting, &Index);
						RTMPCalculateAPTxRxActivityTime(pAd, Index, Length, pMbss, pEntry);
					}
				}
			}
		}
	}
#endif
#endif
	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


VOID dev_rx_ctrl_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	PNDIS_PACKET pRxPacket = pRxBlk->pRxPacket;

#ifdef CUSTOMER_DCC_FEATURE
	if(!(ApScanRunning(pAd)))
	{
		MAC_TABLE_ENTRY *pEntry = NULL;
		pEntry = PACInquiry(pAd,pRxBlk->wcid);

		pAd->RadioStatsCounter.TotalRxCount++;

		if (pEntry != NULL)
		{
			if (pHeader->FC.FrDs == 0 && (pEntry->apidx < pAd->ApCfg.BssidNum))
			{
				MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];
				if (pMbss != NULL)
				{
					UINT32 Length, Index;
					HTTRANSMIT_SETTING HTSetting;
					NdisZeroMemory(&HTSetting, sizeof(HTTRANSMIT_SETTING));
					HTSetting.field.MODE = pRxBlk->rx_rate.field.MODE;
					HTSetting.field.BW = pRxBlk->rx_rate.field.BW;
					HTSetting.field.ShortGI = pRxBlk->rx_rate.field.ShortGI;
					HTSetting.field.MCS = pRxBlk->rx_rate.field.MCS;
					Length = pRxBlk->MPDUtotalByteCnt;
					GetMultShiftFactorIndex(HTSetting, &Index);
					RTMPCalculateAPTxRxActivityTime(pAd, Index, Length, pMbss, pEntry);
				}
			}
		}
	}
#endif

	switch (pHeader->FC.SubType)
	{
#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK_REQ:
			{
				FRAME_BA_REQ *bar = (FRAME_BA_REQ *)pHeader;
#ifdef SMART_MESH_MONITOR
#ifdef CONFIG_AP_SUPPORT
                IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
                {
                     /*
                                        To avoid control frames captured by sniffer confusing due to lack of Addr 3
                                   */
                    if(!IsValidUnicastToMe(pAd, pRxBlk->wcid, bar->Addr1))
                    {
                        RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
                        return;
                    }
                }
#endif /* CONFIG_AP_SUPPORT */
#endif /* SMART_MESH_MONITOR */
			    CntlEnqueueForRecv(pAd, pRxBlk->wcid, (pRxBlk->MPDUtotalByteCnt), (PFRAME_BA_REQ)pHeader);
				if (bar->BARControl.Compressed == 0) {
					UCHAR tid = bar->BARControl.TID;
					BARecSessionTearDown(pAd, pRxBlk->wcid, tid, FALSE);
				}
			}
			break;
#endif /* DOT11_N_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		case SUBTYPE_PS_POLL:
#ifdef RT_CFG80211_P2P_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
			if (IS_PKT_OPMODE_AP(pRxBlk))
#else 
			/* SINGLE Device: TBD */			
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
			{
				USHORT Aid = pHeader->Duration & 0x3fff;
				PUCHAR pAddr = pHeader->Addr2;
				MAC_TABLE_ENTRY *pEntry;

				if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE) {
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	
					if (Aid == 0)
					{
						Aid = pEntry->Aid;
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Aid == 0 (pkt:%d, But Entry:%d)!\n",
                                                                        __FUNCTION__, Aid, pEntry->Aid));						
					}

					if (pEntry->Aid == Aid)
						RtmpHandleRxPsPoll(pAd, pAddr, pRxBlk->wcid, FALSE);
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s(): Aid mismatch(pkt:%d, Entry:%d)!\n",
									__FUNCTION__, Aid, pEntry->Aid));
					}
				}
			}
			break;
#endif /* CONFIG_AP_SUPPORT */

#ifdef WFA_VHT_PF
		case SUBTYPE_RTS:
			if (pAd->CommonCfg.vht_bw_signal && pRxBlk->wcid <= MAX_LEN_OF_MAC_TABLE)
			{
				PLCP_SERVICE_FIELD *srv_field;
				RTS_FRAME *rts = (RTS_FRAME *)pRxBlk->pHeader;

				if ((rts->Addr1[0] & 0x1) == 0x1) {
					srv_field = (PLCP_SERVICE_FIELD *)&pRxBlk->pRxWI->RXWI_N.bbp_rxinfo[15];
					if (srv_field->dyn_bw == 1) {
						DBGPRINT(RT_DEBUG_TRACE, ("%02x:%02x:%02x:%02x:%02x:%02x, WCID:%d, DYN,BW=%d\n",
									PRINT_MAC(rts->Addr1), pRxBlk->wcid, srv_field->cbw_in_non_ht));
					}
				}
			}
			break;

		case SUBTYPE_CTS:
			break;
#endif /* WFA_VHT_PF */

#ifdef DOT11_N_SUPPORT
		case SUBTYPE_BLOCK_ACK:
#endif /* DOT11_N_SUPPORT */
		case SUBTYPE_ACK:
		default:
			break;
	}

	RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
}


/*
		========================================================================
		Routine Description:
			Process RxDone interrupt, running in DPC level

		Arguments:
			pAd    Pointer to our adapter

		Return Value:
			None

		Note:
			This routine has to maintain Rx ring read pointer.
	========================================================================
*/


#undef MAX_RX_PROCESS_CNT
#define MAX_RX_PROCESS_CNT	(256)

BOOLEAN rtmp_rx_done_handle(RTMP_ADAPTER *pAd)
{
	UINT32 RxProcessed, RxPending;
	BOOLEAN bReschedule = FALSE;
	RXD_STRUC *pRxD;
	RXINFO_STRUC *pRxInfo;
	UCHAR *pData;
	RXWI_STRUC *pRxWI;
	PNDIS_PACKET pRxPacket;
	HEADER_802_11 *pHeader;
	RX_BLK rxblk, *pRxBlk;
	BOOLEAN bCmdRspPacket = FALSE;

#ifdef LINUX
#ifdef RTMP_RBUS_SUPPORT
	if (pAd->infType == RTMP_DEV_INF_RBUS)
	{
#if defined(CONFIG_RA_CLASSIFIER) ||defined(CONFIG_RA_CLASSIFIER_MODULE)
#if defined(CONFIG_RALINK_EXTERNAL_TIMER)
		classifier_cur_cycle = (*((volatile u32 *)(0xB0000D08))&0x0FFFF);
#else
		classifier_cur_cycle = read_c0_count();
#endif /* CONFIG_RALINK_EXTERNAL_TIMER */
#endif /* CONFIG_RA_CLASSIFIER */
	}
#endif /* RTMP_RBUS_SUPPORT */
#endif /* LINUX */

	RxProcessed = RxPending = 0;

	/* process whole rx ring */
	while (1)
	{
		if ((RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RADIO_OFF |
								fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS |
								fRTMP_ADAPTER_NIC_NOT_EXIST)) ||
				(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
			)
			&& (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_POLL_IDLE)))
		{
			break;
		}

#ifdef RTMP_MAC_PCI
#ifdef UAPSD_SUPPORT
		UAPSD_TIMING_RECORD_INDEX(RxProcessed);
#endif /* UAPSD_SUPPORT */

		if (RxProcessed++ > MAX_RX_PROCESS_CNT)
		{
			bReschedule = TRUE;
			pAd->rx_tasklet_resche_counter++;
			break;
		}

#ifdef UAPSD_SUPPORT
		/* static rate also need NICUpdateFifoStaCounters() function. */
		/*if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_TX_RATE_SWITCH_ENABLED)) */
		UAPSD_MR_SP_SUSPEND(pAd);
#endif /* UAPSD_SUPPORT */

		/*
			Note:

			Can not take off the NICUpdateFifoStaCounters(); Or the
			FIFO overflow rate will be high, i.e. > 3%
			(see the rate by "iwpriv ra0 show stainfo")

			Based on different platform, try to find the best value to
			replace '4' here (overflow rate target is about 0%).
		*/
		if (++pAd->FifoUpdateDone >= FIFO_STAT_READ_PERIOD)
		{
			NICUpdateFifoStaCounters(pAd);
			pAd->FifoUpdateDone = 0;
		}
#endif /* RTMP_MAC_PCI */

		/* 
			1. allocate a new data packet into rx ring to replace received packet
				then processing the received packet
			2. the callee must take charge of release of packet
			3. As far as driver is concerned, the rx packet must
				a. be indicated to upper layer or
				b. be released if it is discarded
		*/

#ifdef FORCE_ANNOUNCE_CRITICAL_AMPDU
		rxblk.CriticalPkt = 0;
#endif /* FORCE_ANNOUNCE_CRITICAL_AMPDU */

		pRxBlk = &rxblk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, &bCmdRspPacket, 0);
		if ((pRxPacket == NULL) && !bCmdRspPacket)
			break;

		if ((pRxPacket == NULL) && bCmdRspPacket)
			continue;

		/* get rx descriptor and data buffer */
		pRxD = (RXD_STRUC *)&pRxBlk->hw_rx_info[0];
		pRxInfo = rxblk.pRxInfo;
		pData = GET_OS_PKT_DATAPTR(pRxPacket);
		pRxWI = (RXWI_STRUC *)pData;
		pHeader = rxblk.pHeader;// (PHEADER_802_11)(pData + RXWISize);

#ifdef SMART_MESH_MONITOR
		MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			if(pAd->MntEnable && !pRxInfo->CipherErr && !pRxInfo->Crc)
			{
				if(VALID_WCID(pRxBlk->wcid))
					pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				if(pEntry && IS_VALID_ENTRY(pEntry))
				{
					// One who has been authenticated needs to do report to upper.
					if(pEntry->MonitorWCID >= WCID_OF_MONITOR_STA_BASE && pEntry->MonitorWCID <= MAX_WCID_OF_MONITOR_STA)
						Smart_Mesh_Pkt_Report_Action(pAd, pEntry->MonitorWCID, pRxWI, (PUCHAR)pHeader);
				}
				else if (pRxBlk->wcid >= WCID_OF_MONITOR_STA_BASE && pRxBlk->wcid <= MAX_WCID_OF_MONITOR_STA)
				{
					// One who matchs range of monitor wcid needs to do report to upper.
					DBGPRINT(RT_DEBUG_TRACE, ("Monitor pRxWI->wcid=%d\n",pRxBlk->wcid));

					Smart_Mesh_Pkt_Report_Action(pAd, pRxBlk->wcid, pRxWI, (PUCHAR)pHeader);
                    if(!((pHeader->FC.Type == FC_TYPE_MGMT) && (pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
                    {
    					if (!MAC_ADDR_EQUAL(pHeader->Addr3, pAd->CurrentAddress))
    					{
    						RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
    						continue;
    					}
                    }
				}
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#endif /* SMART_MESH_MONITOR */

#ifndef HDR_TRANS_SUPPORT
#ifdef RLT_MAC
		// TODO: shiang-6590, handle packet from other ports
		if (pAd->chipCap.hif_type == HIF_RLT)
		{
			RXFCE_INFO *pFceInfo = rxblk.pRxFceInfo;
#ifdef RTMP_MAC_PCI
#ifdef MT76x2

            if (pFceInfo->info_type == CMD_PACKET)
            {
                pci_rx_cmd_msg_complete(pAd, pFceInfo, (PUCHAR)pRxInfo);
                RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
                continue;
            } 
            else
#endif /* MT76x2 */                
#endif /* RTMP_MAC_PCI */
			if ((pFceInfo->info_type != 0) || (pFceInfo->pkt_80211 != 1))
			{
#ifdef DBG
				DBGPRINT(RT_DEBUG_OFF, ("==>%s(): GetFrameFromOtherPorts!\n", __FUNCTION__));
				hex_dump("hw_rx_info", &rxblk.hw_rx_info[0], sizeof(rxblk.hw_rx_info));
				DBGPRINT(RT_DEBUG_TRACE, ("Dump the RxD, RxFCEInfo and RxInfo:\n"));
				hex_dump("RxD", (UCHAR *)pRxD, sizeof(RXD_STRUC));
#ifdef RTMP_MAC_PCI
				dump_rxd(pAd, pRxD);
#endif /* RTMP_MAC_PCI */
				dumpRxFCEInfo(pAd, pFceInfo);
				dump_rxinfo(pAd, pRxInfo);
				hex_dump("RxFrame", (UCHAR *)pData, (pFceInfo->pkt_len));
				DBGPRINT(RT_DEBUG_OFF, ("<==\n"));
#endif
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
				continue;
			}
		}
#endif /* RLT_MAC */
#endif /* HDR_TRANS_SUPPORT */

#ifdef RT_BIG_ENDIAN
		RTMPFrameEndianChange(pAd, (PUCHAR)pHeader, DIR_READ, TRUE);
		RTMPWIEndianChange(pAd , (PUCHAR)pRxWI, TYPE_RXWI);
#endif

//+++Add by shiang for debug
//---Add by shiang for debug

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_RXWI)
			dbQueueEnqueueRxFrame(pData, (UCHAR *)pHeader, pAd->CommonCfg.DebugFlags);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */
		
		/* build RX_BLK */
		rxblk.pRxWI = pRxWI;
		rxblk.pHeader = pHeader;
		rxblk.pRxPacket = pRxPacket;
		rxblk.pData = (UCHAR *)pHeader;
		rxblk.DataSize = pRxBlk->MPDUtotalByteCnt;
		rxblk.Flags = 0;
#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			SET_PKT_OPMODE_AP(&rxblk);
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
			SET_PKT_OPMODE_STA(&rxblk);
		}
#endif /* CONFIG_STA_SUPPORT */

#ifdef RT_CFG80211_P2P_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE

	if (RTMP_CFG80211_VIF_P2P_GO_ON(pAd) && 
		(NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress, pHeader->Addr1, MAC_ADDR_LEN) ||
		(pHeader->FC.SubType == SUBTYPE_PROBE_REQ)))
	{	
		SET_PKT_OPMODE_AP(&rxblk);
	}
	else if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd) &&
			(((pHeader->FC.SubType == SUBTYPE_BEACON || pHeader->FC.SubType == SUBTYPE_PROBE_RSP) && 
                 	NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].CfgApCliBssid, pHeader->Addr2, MAC_ADDR_LEN)) ||
			(pHeader->FC.SubType == SUBTYPE_PROBE_REQ) ||
			NdisEqualMemory(pAd->ApCfg.ApCliTab[MAIN_MBSSID].MlmeAux.Bssid, pHeader->Addr2, MAC_ADDR_LEN)))
	{

		SET_PKT_OPMODE_AP(&rxblk);
	}
	else
	{	
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		SET_PKT_OPMODE_AP(&rxblk);	
	else
		SET_PKT_OPMODE_STA(&rxblk);
	}
#else
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		SET_PKT_OPMODE_AP(&rxblk);	
	else
		SET_PKT_OPMODE_STA(&rxblk);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */	
#endif /* RT_CFG80211_P2P_SUPPORT */

#ifdef HDR_TRANS_SUPPORT
		rxblk.bHdrRxTrans = pRxInfo->ip_sum_err;			/* RXINFO bit 31 */
		rxblk.bHdrVlanTaged = pRxInfo->tcp_sum_err;			/* RXINFO bit 30 */
		rxblk.pTransData = (UCHAR *) pHeader +  36; /* 36 byte - 802.11 MAC header (RX Wifi Info */
		rxblk.TransDataSize = pRxBlk->MPDUtotalByteCnt;
#endif	/* HDR_TRANS_SUPPORT */

		/* Increase Total receive byte counter after real data received no mater any error or not */
		pAd->RalinkCounters.ReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.OneSecReceivedByteCount += rxblk.DataSize;
		pAd->RalinkCounters.RxCount++;
		pAd->RalinkCounters.OneSecRxCount++;

#ifdef RALINK_ATE
		if (ATE_ON(pAd))
		{
				INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);

			pAd->ate.RxCntPerSec++;
			ATESampleRssi(pAd, pRxWI);

#ifdef RALINK_QA
			if ((pAd->ate.bQARxStart == TRUE) || (pAd->ate.Mode & ATE_RXFRAME))
			{
				/* GetPacketFromRxRing() has copy the endian-changed RxD if it is necessary. */
				ATE_QA_Statistics(pAd, pRxWI, pRxInfo, pHeader);
			}

#ifdef TXBF_SUPPORT
			/* Check sounding frame */
			if ((pAd->chipCap.FlgHwTxBfCap) && (pHeader->FC.Type == FC_TYPE_MGMT))
			{
				pRxBlk->pData += LENGTH_802_11;
				pRxBlk->DataSize -= LENGTH_802_11;

				if (pHeader->FC.Order) {
					pRxBlk->pData += 4;
					pRxBlk->DataSize -= 4;
				}

				if ((((pHeader->FC.SubType == SUBTYPE_ACTION) || (pHeader->FC.SubType == SUBTYPE_ACTION_NO_ACK)) 
					&&  (pRxBlk ->pData)[ 0] == CATEGORY_HT 
					&&  ((pRxBlk ->pData)[ 1] == MIMO_N_BEACONFORM /*non-compressed beamforming report */
					|| (pRxBlk ->pData)[1] == MIMO_BEACONFORM)  )) /*compressed beamforming report */
				{
					/* sounding frame */
					if (pAd->ate.sounding == 1) {
						int i, Nc = ((pRxBlk ->pData)[2] & 0x3) + 1;
						pAd->ate.soundingSNR[0] = (CHAR)((pRxBlk ->pData)[8]);
						pAd->ate.soundingSNR[1] = (Nc<2)? 0: (CHAR)((pRxBlk ->pData)[9]);
						pAd->ate.soundingSNR[2] = (Nc<3)? 0: (CHAR)((pRxBlk ->pData)[10]);
						pAd->ate.sounding = 2;
						pAd->ate.soundingRespSize = pRxBlk->DataSize;
						for (i=0; i<pRxBlk->DataSize && i<MAX_SOUNDING_RESPONSE_SIZE; i++)
							pAd->ate.soundingResp[i] = pRxBlk->pData[i];
					}
				}
				/* Roger Debug : Fix Me */
				else
				{
					if (pHeader->FC.Order)
						DBGPRINT( RT_DEBUG_WARN, ("fcsubtype=%x\ndata[0]=%x\ndata[1]=%x\n",
									pHeader->FC.SubType, (pRxBlk ->pData)[0], (pRxBlk ->pData)[1]));
				}
			}

#ifdef MT76x2
#ifdef RTMP_MAC_PCI

			if ((pHeader->FC.Type == FC_TYPE_DATA))
			{
				if ((pAd->chipCap.FlgHwTxBfCap) && (pAd->ate.bITxBf_Cal == TRUE))
				{
					pRxBlk->pData += LENGTH_802_11;
					pRxBlk->DataSize -= LENGTH_802_11;

					if (pHeader->FC.Order) {
						pRxBlk->pData += 4;
						pRxBlk->DataSize -= 4;
					}

					if (((pRxBlk ->pData)[0] == 0xC0) && ((pRxBlk ->pData)[1] == 0x01)) {
						RtmpOsMsDelay(1); // Delay 1ms
						RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0xc);
						//ATESendNullFrame(pAd, 0, 0, 0);
						mt76x2_ate_SendNullFrame(pAd);
					}

				}
			}
#endif /* RTMP_MAC_PCI */			
#endif /* MT76x2 */			
#endif /* TXBF_SUPPORT */
#endif /* RALINK_QA */

			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
			continue;
		}
#endif /* RALINK_ATE */

#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.ReceivedFragmentCount);
#endif /* STATS_COUNT_SUPPORT */

#ifdef CONFIG_SNIFFER_SUPPORT
		if (MONITOR_ON(pAd) && pAd->monitor_ctrl.current_monitor_mode != MONITOR_MODE_OFF)
		{
			PNDIS_PACKET	pClonePacket;
			PNDIS_PACKET    pTmpRxPacket;

			if(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_REGULAR_RX)
			{	
				/* only report Probe_Req */
				if((pHeader->FC.Type == FC_TYPE_MGMT) && (pHeader->FC.SubType == SUBTYPE_PROBE_REQ))	
		    	{					
					pTmpRxPacket = rxblk.pRxPacket;
					pClonePacket = ClonePacket(pAd, rxblk.pRxPacket, rxblk.pData, rxblk.DataSize);
					rxblk.pRxPacket = pClonePacket;
					STA_MonPktSend(pAd, &rxblk);
					
					rxblk.pRxPacket = pTmpRxPacket;	
				}
			}
			else if(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_FULL)
			{
					pTmpRxPacket = rxblk.pRxPacket;
					pClonePacket = ClonePacket(pAd, rxblk.pRxPacket, rxblk.pData, rxblk.DataSize);
					rxblk.pRxPacket = pClonePacket;
					STA_MonPktSend(pAd, &rxblk);
					//RELEASE_NDIS_PACKET(pAd, rxblk.pRxPacket , NDIS_STATUS_SUCCESS);
					rxblk.pRxPacket = pTmpRxPacket;	
			}
			else if(pAd->monitor_ctrl.current_monitor_mode == MONITOR_MODE_FULL_NO_CLONE)
			{
				STA_MonPktSend(pAd, &rxblk);
				continue;
			}

		}

		if (rxblk.DataSize < 14)
		{
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
			continue;
		}
#endif /* CONFIG_SNIFFER_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
			/* Check for all RxD errors */
			if (APCheckRxError(pAd, pRxInfo, pRxBlk) != NDIS_STATUS_SUCCESS)
			{
				APRxErrorHandle(pAd, &rxblk);

				/* Increase received error packet counter per BSS */
				if (pHeader->FC.FrDs == 0 &&
					pRxInfo->U2M &&
					pRxBlk->bss_idx < pAd->ApCfg.BssidNum)
				{
					MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pRxBlk->bss_idx];
#ifdef CUSTOMER_DCC_FEATURE
#ifdef MBSS_802_11_STATISTICS
					if (pHeader->FC.Type == FC_TYPE_DATA)
					{
						pMbss->RxDropCount ++;
						pMbss->RxErrorCount ++;
					}
					else if (pHeader->FC.Type == FC_TYPE_MGMT)
					{
						pMbss->MGMTRxDropCount++;
						pMbss->MGMTRxErrorCount++;
					}
#endif
#else
					pMbss->RxDropCount ++;
					pMbss->RxErrorCount ++;
#endif
				}

#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
				if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 1))
				{
					MAC_TABLE_ENTRY *pEntry = NULL;

					if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
						pEntry = FindWdsEntry(pAd, pRxBlk->wcid, pHeader->Addr2, pRxBlk->rx_rate.field.MODE);
					if(pEntry)
						pAd->WdsTab.WdsEntry[pEntry->wdev_idx].WdsCounter.RxErrors++;
				}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef STATS_COUNT_SUPPORT
				if ((pHeader->FC.FrDs == 1) && (pHeader->FC.ToDs == 0) && (pRxInfo->U2M))
				{
					MAC_TABLE_ENTRY *pEntry = NULL;

					if (VALID_WCID(pRxBlk->wcid))
						pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
					else
						pEntry = MacTableLookup(pAd, pHeader->Addr2);
					
					if (pEntry && IS_ENTRY_APCLI(pEntry))
						pAd->ApCfg.ApCliTab[pEntry->wdev_idx].ApCliCounter.RxErrors++;
				}
#endif /* STATS_COUNT_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef DBG_DIAGNOSE
				if (!pRxInfo->Crc)
					DBGPRINT(RT_DEBUG_TRACE, ("%s(): CheckRxError!\n", __FUNCTION__));
#endif

				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);

				continue;
			}
		}
#endif /* CONFIG_AP_SUPPORT */
#ifdef APCLI_SUPPORT
		/* When root AP is Open WEP, it will cause a fake connection state if user keys in wrong password. */
		if(pHeader->FC.Wep == 1)
		{
			if(pRxInfo->Decrypted == 1)
				ApCliRxOpenWEPCheck(pAd,pRxBlk,TRUE);
			else
				ApCliRxOpenWEPCheck(pAd,pRxBlk,FALSE);
		}
#endif /* APCLI_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
		{
#ifdef RT_CFG80211_P2P_SUPPORT
			//CFG_TOOD : NO GOOD 
			if (IS_PKT_OPMODE_STA(pRxBlk))
#endif /* RT_CFG80211_P2P_SUPPORT */		
			/* Check for all RxD errors */
			if (RTMPCheckRxError(pAd, pHeader, &rxblk, pRxInfo) != NDIS_STATUS_SUCCESS)
			{
				pAd->Counters8023.RxErrors++;
				/* discard this frame */
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				continue;
			}

		}
#endif /* CONFIG_STA_SUPPORT */


		// TODO: shiang-usw, for P2P, we original has following code, need to check it and merge to correct place!!!

		switch (pHeader->FC.Type)
		{
			case FC_TYPE_DATA:
#ifdef CONFIG_STA_SUPPORT
				IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				{
				}
#endif /* CONFIG_STA_SUPPORT */


#ifdef HDR_TRANS_SUPPORT
					rxblk.bHdrRxTrans = pRxInfo->ip_sum_err; /* RXINFO bit 31 */
					//printk("sn - rxblk.bHdrRxTrans = %d\n", rxblk.bHdrRxTrans);
					if ( rxblk.bHdrRxTrans)
					{
						rxblk.bHdrVlanTaged = pRxInfo->tcp_sum_err;	/* RXINFO bit 30 */
						rxblk.pTransData = (UCHAR *) pHeader +  36; /* 36 byte - RX WIFI Size ( 802.11 Header ) */
						rxblk.TransDataSize = pRxBlk->MPDUtotalByteCnt;
						rxblk.DataSize += 36;

#ifdef CONFIG_STA_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
						{
							STAHandleRxDataFrame_Hdr_Trns(pAd, &rxblk);
						}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
						{
							APHandleRxDataFrame_Hdr_Trns(pAd, &rxblk);
						}
#endif /* CONFIG_AP_SUPPORT */
					}
					else
#endif	/* HDR_TRANS_SUPPORT */
					{
#ifdef RT_CFG80211_P2P_SUPPORT
						if (IS_PKT_OPMODE_AP(pRxBlk))
						{
							APHandleRxDataFrame(pAd, &rxblk);
							break;
						}
#endif /* RT_CFG80211_P2P_SUPPORT */
					
#ifdef CONFIG_STA_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
						{
							STAHandleRxDataFrame(pAd, &rxblk);
							break;
						}
#endif /* CONFIG_STA_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
						IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
						{
							APHandleRxDataFrame(pAd, &rxblk);
							break;
						}
#endif /* CONFIG_AP_SUPPORT */
					}
					break;

			case FC_TYPE_MGMT:
					dev_rx_mgmt_frm(pAd, &rxblk);
					break;

			case FC_TYPE_CNTL:
					dev_rx_ctrl_frm(pAd, &rxblk);
					break;

			default:
				RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_FAILURE);
				break;
		}
	}

#ifdef UAPSD_SUPPORT
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
    if (IS_PKT_OPMODE_AP(pRxBlk))
#else                                                                                           
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
	{
		/* dont remove the function or UAPSD will fail */
		UAPSD_MR_SP_RESUME(pAd);
		UAPSD_SP_CloseInRVDone(pAd);
	}
#endif /* CONFIG_AP_SUPPORT */
#endif /* UAPSD_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
	//if (pAd->int_cpu_analysis > 0) {
		//DBGPRINT(RT_DEBUG_OFF, ("APRxDoneInterruptHandle:: RxProcessed=%d\n", RxProcessed));
		pAd->rx_packet_counter += RxProcessed;

		if (RxProcessed == 1)
			pAd->rx_packet_1_counter++;
		else if (RxProcessed == 2)
			pAd->rx_packet_2_counter++;
		else if (RxProcessed == 3)
			pAd->rx_packet_3_counter++;
		else if (RxProcessed == 4)
			pAd->rx_packet_4_counter++;
		else if (RxProcessed == 5)
			pAd->rx_packet_5_counter++;
		else if ((RxProcessed > 5) && (RxProcessed <= 10))
			pAd->rx_packet_6_10_counter++;
		else if ((RxProcessed > 10) && (RxProcessed <= 15))
			pAd->rx_packet_11_15_counter++;
		else if ((RxProcessed > 15) && (RxProcessed <= 20))
			pAd->rx_packet_16_20_counter++;
		else if ((RxProcessed > 20) && (RxProcessed <= 25))
			pAd->rx_packet_21_25_counter++;
		else if ((RxProcessed > 25) && (RxProcessed <= 31))
			pAd->rx_packet_26_31_counter++;
		else if ((RxProcessed > 31) && (RxProcessed <= (MAX_RX_PROCESS_CNT - 1)))
			pAd->rx_packet_32_max_counter++;
	//}
#endif

	return bReschedule;
}

#ifdef DROP_MASK_SUPPORT
VOID drop_mask_init_per_client(
	PRTMP_ADAPTER	ad,
	PMAC_TABLE_ENTRY entry)
{
	BOOLEAN cancelled = FALSE;

	if (entry->dropmask_timer.Valid)
		RTMPCancelTimer(&entry->dropmask_timer, &cancelled);

	RTMPInitTimer(ad, &entry->dropmask_timer, GET_TIMER_FUNCTION(drop_mask_timer_action), entry, FALSE);

	asic_set_drop_mask(ad, entry->Aid, FALSE);

#ifdef NOISE_TEST_ADJUST
	if ((ad->ApCfg.EntryClientCount >= 3) && IS_ENTRY_CLIENT(entry))
		entry->pMbss->WPAREKEY.ReKeyMethod |= MAX_REKEY;
#endif /* NOISE_TEST_ADJUST */
}

VOID drop_mask_release_per_client(
	PRTMP_ADAPTER	ad,
	PMAC_TABLE_ENTRY entry)
{
	BOOLEAN cancelled = FALSE;

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);
	RTMPReleaseTimer(&entry->dropmask_timer, &cancelled);
}

VOID drop_mask_set_per_client(
	PRTMP_ADAPTER		ad,
	PMAC_TABLE_ENTRY 	entry,
	BOOLEAN				enable)
{
	BOOLEAN cancelled = FALSE;

#ifdef NOISE_TEST_ADJUST
	if (ad->ApCfg.EntryClientCount < 3)
		return;
#endif /* NOISE_TEST_ADJUST */

	RTMPCancelTimer(&entry->dropmask_timer, &cancelled);

	asic_set_drop_mask(ad, entry->Aid, enable);

	if (enable)
		RTMPSetTimer(&entry->dropmask_timer, 1000);
}

VOID  drop_mask_timer_action(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3)
{
	PMAC_TABLE_ENTRY entry = (MAC_TABLE_ENTRY *)FunctionContext;
	PRTMP_ADAPTER ad = (PRTMP_ADAPTER)entry->pAd;

	/* Disable drop mask */
	asic_set_drop_mask(ad, entry->Aid, FALSE);
}

#endif /* DROP_MASK_SUPPORT */

