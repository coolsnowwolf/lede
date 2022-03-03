/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2010, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

    Abstract:

	All related POWER SAVE function body.

***************************************************************************/

#include "rt_config.h"


/*
	========================================================================
	Routine Description:
		This routine is used to do insert packet into power-saveing queue.
	
	Arguments:
		pAd: Pointer to our adapter
		pPacket: Pointer to send packet
		pMacEntry: portint to entry of MacTab. the pMacEntry store attribute of client (STA).
		QueIdx: Priority queue idex.

	Return Value:
		NDIS_STATUS_SUCCESS:If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE: If failed to do en-queue.
========================================================================
*/
NDIS_STATUS RtmpInsertPsQueue(
	RTMP_ADAPTER *pAd,
	PNDIS_PACKET pPacket,
	MAC_TABLE_ENTRY *pMacEntry,
	UCHAR QueIdx)
{
	ULONG IrqFlags = 0;
#ifdef UAPSD_SUPPORT
	UINT32 ac_id;
#endif
	STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];;
#ifdef UAPSD_SUPPORT
	/* put the U-APSD packet to its U-APSD queue by AC ID */
	ac_id = QueIdx - QID_AC_BE; /* should be >= 0 */

	tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];

	if (UAPSD_MR_IS_UAPSD_AC(pMacEntry, ac_id))
	{
		UAPSD_PacketEnqueue(pAd, pMacEntry, pPacket, ac_id, FALSE);

#ifdef CFG_TDLS_SUPPORT
		cfg_tdls_send_PeerTrafficIndication(pAd,pMacEntry->Addr);
#endif /* CFG_TDLS_SUPPORT */
	}
	else
#endif /* UAPSD_SUPPORT */
	{
		if (tr_entry->ps_queue.Number >= MAX_PACKETS_IN_PS_QUEUE)
		{
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;			
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("legacy ps> queue a packet!\n"));
			RTMP_IRQ_LOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
			InsertTailQueue(&tr_entry->ps_queue, PACKET_TO_QUEUE_ENTRY(pPacket));
			RTMP_IRQ_UNLOCK(&pAd->irq_lock /*&tr_entry->ps_queue_lock*/, IrqFlags);
		}
	}

#ifdef CONFIG_AP_SUPPORT
	/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT
	if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(pMacEntry, QueIdx))
	{
		/* 1. the station is UAPSD station;
		2. one of AC is non-UAPSD (legacy) AC;
		3. the destinated AC of the packet is UAPSD AC. */
		/* So we can not set TIM bit due to one of AC is legacy AC */
	}
	else
#endif /* UAPSD_SUPPORT */
	{
		WLAN_MR_TIM_BIT_SET(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);

	}
#endif /* CONFIG_AP_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID RtmpCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue)
{
	QUEUE_ENTRY *pQEntry;
	PNDIS_PACKET pPacket;

	DBGPRINT(RT_DEBUG_TRACE, ("RtmpCleanupPsQueue (0x%08lx)...\n", (ULONG)pQueue));

	while (pQueue->Head)
	{
		DBGPRINT(RT_DEBUG_TRACE,
					("RtmpCleanupPsQueue %d...\n",pQueue->Number));

		pQEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);

		DBGPRINT(RT_DEBUG_TRACE, ("RtmpCleanupPsQueue pkt = %lx...\n", (ULONG)pPacket));
	}
}


/*
  ========================================================================
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL 
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID RtmpHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry = NULL;

	ASSERT(wcid < MAX_LEN_OF_MAC_TABLE);

	pMacEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (!RTMPEqualMemory(pMacEntry->Addr, pAddr, MAC_ADDR_LEN))
	{
		DBGPRINT(RT_DEBUG_WARN | DBG_FUNC_PS,("%s(%d) PS-POLL (MAC addr not match) from %02x:%02x:%02x:%02x:%02x:%02x. Why???\n", 
			__FUNCTION__, __LINE__, PRINT_MAC(pAddr)));
		return;
	}

#ifdef UAPSD_SUPPORT00
	if (UAPSD_MR_IS_ALL_AC_UAPSD(isActive, pMacEntry))
	{
		/*
			IEEE802.11e spec.
			11.2.1.7 Receive operation for STAs in PS mode during the CP
			When a non-AP QSTA that is using U-APSD and has all ACs
			delivery-enabled detects that the bit corresponding to its AID
			is set in the TIM, the non-AP QSTA shall issue a trigger frame
			or a PS-Poll frame to retrieve the buffered MSDU or management
			frames.

			WMM Spec. v1.1a 070601
			3.6.2	U-APSD STA Operation
			3.6.2.3	In case one or more ACs are not
			delivery-enabled ACs, the WMM STA may retrieve MSDUs and
			MMPDUs belonging to those ACs by sending PS-Polls to the WMM AP.
			In case all ACs are delivery enabled ACs, WMM STA should only
			use trigger frames to retrieve MSDUs and MMPDUs belonging to
			those ACs, and it should not send PS-Poll frames.

			Different definitions in IEEE802.11e and WMM spec.
			But we follow the WiFi WMM Spec.
		*/

		DBGPRINT(RT_DEBUG_TRACE, ("All AC are UAPSD, can not use PS-Poll\n"));
		return; /* all AC are U-APSD, can not use PS-Poll */
	}
#endif /* UAPSD_SUPPORT */

	/* Reset ContinueTxFailCnt */
	pMacEntry->ContinueTxFailCnt = 0;
	pAd->MacTab.tr_entry[pMacEntry->wcid].ContinueTxFailCnt = 0;

	if (isActive == FALSE)
	{
		if (tr_entry->PsDeQWaitCnt == 0) {
			tr_entry->PsDeQWaitCnt = 1;
		} else {
			DBGPRINT(RT_DEBUG_TRACE,
					("%s(): : packet not send by HW then ignore other PS-Poll Aid[%d]!\n",
					__FUNCTION__, pMacEntry->Aid));
			return;
		}
	}
	else
		tr_entry->PsDeQWaitCnt = 0;

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		MtHandleRxPsPoll(pAd, pAddr, wcid, isActive);
	}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) 
		|| (pAd->chipCap.hif_type == HIF_RLT))
	{
		RalHandleRxPsPoll(pAd, pAddr, wcid, isActive);
	}
#endif /* RTMP_MAC || RLT_MAC */
#endif /* CONFIG_AP_SUPPORT */
}


/*
	==========================================================================
	Description:
		Update the station current power save mode. Calling this routine also
		prove the specified client is still alive. Otherwise AP will age-out
		this client once IdleCount exceeds a threshold.
	==========================================================================
 */
BOOLEAN RtmpPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm)
{
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
	{
		return MtPsIndicate(pAd, pAddr, wcid, Psm);
	}
#endif /* MT_MAC */

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((pAd->chipCap.hif_type == HIF_RTMP) 
		|| (pAd->chipCap.hif_type == HIF_RLT))
	{
		return RalPsIndicate(pAd, pAddr, wcid, Psm);
	}
#endif /* RTMP_MAC || RLT_MAC */
	return FALSE;
}



