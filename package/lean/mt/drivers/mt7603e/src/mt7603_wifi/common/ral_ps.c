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
  Description:
	This routine frees all packets in PSQ that's destined to a specific DA.
	BCAST/MCAST in DTIMCount=0 case is also handled here, just like a PS-POLL 
	is received from a WSTA which has MAC address FF:FF:FF:FF:FF:FF
  ========================================================================
*/
VOID RalHandleRxPsPoll(RTMP_ADAPTER *pAd, UCHAR *pAddr, USHORT wcid, BOOLEAN isActive)
{ 
	QUEUE_ENTRY *pQEntry;
	MAC_TABLE_ENTRY *pMacEntry;
	unsigned long IrqFlags;
	STA_TR_ENTRY *tr_entry;

	/*
	DBGPRINT(RT_DEBUG_TRACE, ("rcv PS-POLL (AID=%d) from %02x:%02x:%02x:%02x:%02x:%02x\n",
				Aid, PRINT_MAC(pAddr)));
	*/

	pMacEntry = &pAd->MacTab.Content[wcid];
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	
	{
		/*NdisAcquireSpinLock(&pAd->MacTabLock); */
		/*NdisAcquireSpinLock(&pAd->TxSwQueueLock); */
		RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
		if (isActive == FALSE)
		{
			if (tr_entry->ps_queue.Head)
			{
#ifdef UAPSD_SUPPORT
				UINT32 NumOfOldPsPkt;
				NumOfOldPsPkt = pAd->TxSwQueue[QID_AC_BE].Number;
#endif /* UAPSD_SUPPORT */

				pQEntry = RemoveHeadQueue(&tr_entry->ps_queue);
				if (tr_entry->ps_queue.Number >= 1)
				{
					RTMP_SET_PACKET_MOREDATA(RTPKT_TO_OSPKT(pQEntry), TRUE);
					RTMP_SET_PACKET_TXTYPE(RTPKT_TO_OSPKT(pQEntry), TX_LEGACY_FRAME);
				}
				InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QID_AC_BE], pQEntry);

#ifdef UAPSD_SUPPORT
				/* we need to call RTMPDeQueuePacket() immediately as below */
				if (NumOfOldPsPkt != pAd->TxSwQueue[QID_AC_BE].Number)
				{
					if (RTMP_GET_PACKET_DHCP(RTPKT_TO_OSPKT(pQEntry)) ||
						RTMP_GET_PACKET_EAPOL(RTPKT_TO_OSPKT(pQEntry)) ||
						RTMP_GET_PACKET_WAI(RTPKT_TO_OSPKT(pQEntry)))
					{
						/*
							These packets will use 1M/6M rate to send.
							If you use 1M(2.4G)/6M(5G) to send, no statistics
							count in NICUpdateFifoStaCounters().

							So we can not count it for UAPSD; Or the SP will
							not closed until timeout.
						*/
					}
					else
						UAPSD_MR_MIX_PS_POLL_RCV(pAd, pMacEntry);
				}
#endif /* UAPSD_SUPPORT */
			}
			else
			{
				/*
					or transmit a (QoS) Null Frame;

					In addtion, in Station Keep Alive mechanism, we need to
					send a QoS Null frame to detect the station live status.
				*/
				BOOLEAN bQosNull = FALSE;

				if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					bQosNull = TRUE;

				RtmpEnqueueNullFrame(pAd, pMacEntry->Addr, tr_entry->CurrTxRate,
										pMacEntry->Aid, pMacEntry->func_tb_idx,
										bQosNull, TRUE, 0);
			}
		}
		else
		{
#ifdef UAPSD_SUPPORT
			/* deliver all queued UAPSD packets */
			UAPSD_AllPacketDeliver(pAd, pMacEntry);

			/* end the SP if exists */
			UAPSD_MR_ENTRY_RESET(pAd, pMacEntry);
#endif /* UAPSD_SUPPORT */

			while(tr_entry->ps_queue.Head)
			{
				pQEntry = RemoveHeadQueue(&tr_entry->ps_queue);
				InsertTailQueueAc(pAd, pMacEntry, &pAd->TxSwQueue[QID_AC_BE], pQEntry);
			}
		}

		/*NdisReleaseSpinLock(&pAd->TxSwQueueLock); */
		/*NdisReleaseSpinLock(&pAd->MacTabLock); */

		if ((pMacEntry->Aid > 0) && (pMacEntry->Aid < MAX_LEN_OF_MAC_TABLE) &&
			(tr_entry->ps_queue.Number == 0))
		{
			/* clear corresponding TIM bit because no any PS packet */
#ifdef CONFIG_AP_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
				WLAN_MR_TIM_BIT_CLEAR(pAd, pMacEntry->func_tb_idx, pMacEntry->Aid);
			}
#endif /* CONFIG_AP_SUPPORT */
			tr_entry->PsQIdleCount = 0;
		}

		RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);

		/*
			Dequeue outgoing frames from TxSwQueue0..3 queue and process it
			TODO: 2004-12-27 it's not a good idea to handle "More Data" bit here. 
				because the RTMPDeQueue process doesn't guarantee to de-queue the 
				desired MSDU from the corresponding TxSwQueue/PsQueue when QOS 
				in-used. We should consider "HardTransmt" this MPDU using MGMT 
				queue or things like that.
		*/
		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, wcid, MAX_TX_PROCESS);
	}
}


/*
	==========================================================================
	Description:
		Update the station current power save mode. Calling this routine also
		prove the specified client is still alive. Otherwise AP will age-out
		this client once IdleCount exceeds a threshold.
	==========================================================================
 */
BOOLEAN RalPsIndicate(RTMP_ADAPTER *pAd, UCHAR *pAddr, UCHAR wcid, UCHAR Psm)
{
	MAC_TABLE_ENTRY *pEntry;
	UCHAR old_psmode;
	STA_TR_ENTRY *tr_entry;
	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return PWR_ACTIVE;

	pEntry = &pAd->MacTab.Content[wcid];
	old_psmode = pEntry->PsMode;
/*	if (pEntry) */
	{
		/*
			Change power save mode first because we will call
			RTMPDeQueuePacket() in RtmpHandleRxPsPoll().

			Or when Psm = PWR_ACTIVE, we will not do Aggregation in
			RTMPDeQueuePacket().
		*/
		pEntry->NoDataIdleCount = 0;
		pEntry->PsMode = Psm;
		// TODO: shiang-usw, we need to rmove upper setting and migrate to tr_entry->PsMode	
		pAd->MacTab.tr_entry[wcid].PsMode = Psm;

		if (old_psmode != Psm) {
			DBGPRINT(RT_DEBUG_INFO, ("%s():%02x:%02x:%02x:%02x:%02x:%02x %s!\n",
					__FUNCTION__, PRINT_MAC(pAddr),
					(Psm == PWR_SAVE ? "Sleep" : "wakes up, act like rx PS-POLL")));
		}

		if ((old_psmode == PWR_SAVE) && (Psm == PWR_ACTIVE))
		{
#ifdef RTMP_MAC_PCI
#ifdef DOT11_N_SUPPORT
			/*
				When sta wake up, we send BAR to refresh the BA sequence.
				TODO:
					For RT2870, how to handle BA when STA in PS mode?
			*/
			SendRefreshBAR(pAd, pEntry);
#endif /* DOT11_N_SUPPORT */
#endif /* RTMP_MAC_PCI */

			/* sleep station awakes, move all pending frames from PSQ to TXQ if any */
			RtmpHandleRxPsPoll(pAd, pAddr, pEntry->wcid, TRUE);
		}
	}
	return old_psmode;
}

