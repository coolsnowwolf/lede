/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	ap_data.c

	Abstract:
	Data path subroutines

	Revision History:
	Who 		When			What
	--------	----------		----------------------------------------------
*/
#include "rt_config.h"


INT ApAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	IN UCHAR *pWcid)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	MAC_TABLE_ENTRY *pEntry = NULL;
#ifdef NEW_IXIA_METHOD
	MAC_TABLE_ENTRY *pIXIAEntry = NULL;
	UINT16 TypeLen;
#endif
	STA_TR_ENTRY *tr_entry = NULL;

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
#ifdef NEW_IXIA_METHOD
	if (pAd->ContinousTxCnt != 1) {
		pIXIAEntry = &pAd->MacTab.Content[1];
		TypeLen = (pSrcBufVA[12] << 8) | pSrcBufVA[13];
		if (pIXIAEntry && IS_ENTRY_CLIENT(pIXIAEntry)
			&& (TypeLen == 0x0800)) {
			COPY_MAC_ADDR(pSrcBufVA, pIXIAEntry->Addr);
		}
	}
#endif
	/* 0 is main BSS, FIRST_MBSSID = 1 */
	ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
	ASSERT (wdev->wdev_type == WDEV_TYPE_AP);

	if (wdev != &pAd->ApCfg.MBSSID[wdev->func_idx].wdev) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): wdev(0x%p) not equal MBSS(0x%p), func_idx=%d\n",
				__FUNCTION__, wdev, &pAd->ApCfg.MBSSID[wdev->func_idx].wdev, wdev->func_idx));
	}

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
	if ((wf_fwd_needed_hook != NULL) && (wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pPacket))
			return FALSE;
	}
#endif /* CONFIG_WIFI_PKT_FWD */
#endif
	if (MAC_ADDR_IS_GROUP(pSrcBufVA))
	{
		*pWcid = wdev->tr_tb_idx;
#ifdef A4_CONN
		/* If we check an ethernet source move to this device, we should remove it. */
		a4_proxy_delete(pAd, wdev->func_idx, (pSrcBufVA + MAC_ADDR_LEN));
#endif /* A4_CONN */
		return TRUE;
	} 
	else
	{
		pEntry = MacTableLookup(pAd, pSrcBufVA);
		if (pEntry && (pEntry->Sst == SST_ASSOC))
		{
#ifdef WH_EVENT_NOTIFIER
			if(IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
		&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
				)
			pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */ 
		
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			if (tr_entry && (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
				DBGPRINT(RT_DEBUG_ERROR, ("sta port not secure, os should not send packets!!!\n"));
				return FALSE;
			}
			*pWcid = (UCHAR)pEntry->wcid;
			return TRUE;
		}

#ifdef CLIENT_WDS
		if (pEntry == NULL) {
			SST Sst;
			USHORT Aid;
			UCHAR PsMode, Rate;

			PUCHAR pEntryAddr = CliWds_ProxyLookup(pAd, pSrcBufVA);
			if (pEntryAddr != NULL) {
				pEntry = APSsPsInquiry(pAd, pEntryAddr, &Sst, &Aid, &PsMode, &Rate);
				if ((pEntry && (pEntry->Sst == SST_ASSOC)) {
					*pWcid = (UCHAR)Aid;
					return TRUE;
				}
			}
		}
#endif /* CLIENT_WDS */
	}
#ifdef A4_CONN
	if (pEntry == NULL) {
		UCHAR main_wcid = 0;
		/* If we check an ethernet source move to this device, we should remove it. */
		a4_proxy_delete(pAd, wdev->func_idx, (pSrcBufVA + MAC_ADDR_LEN));
		if (a4_proxy_lookup(pAd, wdev->func_idx, pSrcBufVA, FALSE, FALSE, &main_wcid)) {
			if (VALID_WCID(main_wcid))
				pEntry = &pAd->MacTab.Content[main_wcid];

			if (pEntry && (pEntry->Sst == SST_ASSOC)){
				tr_entry = &pAd->MacTab.tr_entry[main_wcid];
				if (tr_entry && (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
					DBGPRINT(RT_DEBUG_ERROR,
						("a4 port not secure, os should not send packets!!!\n"));
					return FALSE;
				}
				*pWcid = main_wcid;
				return TRUE;
			}
		}
	}
#endif

	return FALSE;
}

#ifndef NEW_IXIA_METHOD
enum pkt_tx_status{
	PKT_SUCCESS = 0,
	INVALID_PKT_LEN = 1, 
	INVALID_TR_WCID = 2,
	INVALID_TR_ENTRY = 3,
	INVALID_WDEV = 4,
	INVALID_ETH_TYPE = 5,
	DROP_PORT_SECURE = 6,
	DROP_PSQ_FULL = 7,
	DROP_TXQ_FULL = 8,
	DROP_TX_JAM = 9,
	DROP_TXQ_ENQ_FAIL = 10,
};
#endif
struct reason_id_str{
	INT id;
	RTMP_STRING *code_str;
};

/*Nobody uses it currently*/

/*
	========================================================================
	Routine Description:
		This routine is used to do packet parsing and classification for Tx packet 
		to AP device, and it will en-queue packets to our TxSwQ depends on AC 
		class.
	
	Arguments:
		pAd    Pointer to our adapter
		pPacket 	Pointer to send packet

	Return Value:
		NDIS_STATUS_SUCCESS		If succes to queue the packet into TxSwQ.
		NDIS_STATUS_FAILURE			If failed to do en-queue.

	pre: Before calling this routine, caller should have filled the following fields

		pPacket->MiniportReserved[6] - contains packet source
		pPacket->MiniportReserved[5] - contains RA's WDS index (if RA on WDS link) or AID 
										(if RA directly associated to this AP)
	post:This routine should decide the remaining pPacket->MiniportReserved[] fields 
		before calling APHardTransmit(), such as:

		pPacket->MiniportReserved[4] - Fragment # and User PRiority
		pPacket->MiniportReserved[7] - RTS/CTS-to-self protection method and TX rate

	Note:
		You only can put OS-indepened & AP related code in here.
========================================================================
*/
INT APSendPacket(RTMP_ADAPTER *pAd, PNDIS_PACKET pPacket)
{
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen, frag_sz, pkt_len;
	UCHAR NumberOfFrag;
	UCHAR wcid = RESERVED_WCID, QueIdx, UserPriority;
#ifdef IGMP_SNOOP_SUPPORT
	UCHAR Repeat;
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
#endif /* IGMP_SNOOP_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	struct wifi_dev *wdev;
#ifdef DBG
#ifndef NEW_IXIA_METHOD
	enum pkt_tx_status drop_reason = PKT_SUCCESS;
#else
	T_DROP_RESON drop_reason = 0;
#endif
#endif /* DBG */
	INT ret=0;
	

	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	if ((!pSrcBufVA) || (SrcBufLen <= 14)) {
#ifdef DBG
		drop_reason = INVALID_PKT_LEN;
#endif /* DBG */
		goto drop_pkt;
	}

	wcid = RTMP_GET_PACKET_WCID(pPacket);
#ifdef IGMP_SNOOP_SUPPORT
	if (VALID_WCID(wcid))
		pEntry = &pAd->MacTab.Content[wcid];
#endif /* IGMP_SNOOP_SUPPORT */
    DBGPRINT(RT_DEBUG_INFO, ("%s(): wcid=%d\n", __FUNCTION__, wcid));
	if (!(VALID_TR_WCID(wcid) && IS_VALID_ENTRY(&pAd->MacTab.tr_entry[wcid]))) {
#ifdef DBG
		drop_reason = INVALID_TR_WCID;
#endif /* DBG */
		goto drop_pkt;
	}

	tr_entry = &pAd->MacTab.tr_entry[wcid];
	if (!tr_entry->wdev) {
#ifdef DBG
		drop_reason = INVALID_WDEV;
#endif /* DBG */
		goto drop_pkt;
	}

	wdev = tr_entry->wdev;
	UserPriority = 0;
	QueIdx = QID_AC_BE;
	if (RTMPCheckEtherType(pAd, pPacket, tr_entry, wdev, &UserPriority, &QueIdx, &wcid) == FALSE) {
#ifdef DBG
		drop_reason = INVALID_ETH_TYPE;
#endif /* DBG */
		goto drop_pkt;
	}
	/*add hook point when enqueue*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_ENQUEUE,pPacket,QueIdx,pAd);

#ifdef CONFIG_HOTSPOT
	/* 
		Re-check the wcid, it maybe broadcast to unicast by RTMPCheckEtherType.
	*/
	if (wcid != RTMP_GET_PACKET_WCID(pPacket))
	{
		wcid = RTMP_GET_PACKET_WCID(pPacket);
		//pMacEntry = &pAd->MacTab.Content[wcid];
		tr_entry = &pAd->MacTab.tr_entry[wcid];
		wdev = tr_entry->wdev;
	}

	/* Drop broadcast/multicast packet if disable dgaf */
	// TODO: shiang-usw, fix me because MCAST_WCID is not used now!
	if (IS_ENTRY_CLIENT(tr_entry)) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

		if ((wcid == pMbss->wdev.tr_tb_idx) && 
			(pMbss->HotSpotCtrl.HotSpotEnable || pMbss->HotSpotCtrl.bASANEnable) &&
			pMbss->HotSpotCtrl.DGAFDisable) {
			DBGPRINT(RT_DEBUG_INFO, ("Drop broadcast/multicast packet when dgaf disable\n"));
			goto drop_pkt;
		}
	}
#endif

	/* AP does not send packets before port secured */
	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED && FALSE == pAd->BSendBMToAir)
	{
		if (!(((wdev->AuthMode >= Ndis802_11AuthModeWPA)
#ifdef DOT1X_SUPPORT
			|| (wdev->IEEE8021X == TRUE)
#endif /* DOT1X_SUPPORT */
			) && ((RTMP_GET_PACKET_EAPOL(pPacket) ||
				RTMP_GET_PACKET_WAI(pPacket))))
		)
		{
#ifdef DBG
			drop_reason = DROP_PORT_SECURE;
#endif /* DBG */
			goto drop_pkt;
		}
	}

#ifdef MAC_REPEATER_SUPPORT
	if (VALID_WCID(wcid)){
		// TODO: shiang-usw, remove pMacEntry here!
		MAC_TABLE_ENTRY *pMacEntry = &pAd->MacTab.Content[wcid];
		if (pMacEntry->bReptCli)
			pMacEntry->ReptCliIdleCount = 0;
	}
#endif /* MAC_REPEATER_SUPPORT */

	/* 
		STEP 1. Decide number of fragments required to deliver this MSDU.
			The estimation here is not very accurate because difficult to
			take encryption overhead into consideration here. The result
			"NumberOfFrag" is then just used to pre-check if enough free
			TXD are available to hold this MSDU.

			The calculated "NumberOfFrag" is a rough estimation because of various
			encryption/encapsulation overhead not taken into consideration. This number is just
			used to make sure enough free TXD are available before fragmentation takes place.
			In case the actual required number of fragments of an NDIS packet
			excceeds "NumberOfFrag"caculated here and not enough free TXD available, the
			last fragment (i.e. last MPDU) will be dropped in RTMPHardTransmit() due to out of
			resource, and the NDIS packet will be indicated NDIS_STATUS_FAILURE. This should
			rarely happen and the penalty is just like a TX RETRY fail. Affordable.

		exception:
			a). fragmentation not allowed on multicast & broadcast
			b). Aggregation overwhelms fragmentation (fCLIENT_STATUS_AGGREGATION_CAPABLE)
			c). TSO/CSO not do fragmentation
	*/
	// TODO: shiang-usw. we need to modify the TxPktClassificatio to adjust the NumberOfFrag!
	pkt_len = PacketInfo.TotalPacketLength - LENGTH_802_3 + LENGTH_802_1_H;
	frag_sz = (pAd->CommonCfg.FragmentThreshold) - LENGTH_802_11 - LENGTH_CRC;
	if (pkt_len < frag_sz)
		NumberOfFrag = 1;
	else
		NumberOfFrag = (pkt_len / frag_sz) + 1;

	/* Save fragment number to Ndis packet reserved field */
	RTMP_SET_PACKET_FRAGMENTS(pPacket, NumberOfFrag);


	/*
		3. Put to corrsponding TxSwQ or Power-saving queue

		a).WDS/ApClient/Mesh link should never go into power-save mode; just send out the frame
		b).multicast packets in IgmpSn table should never send to Power-Saving queue.
		c). M/BCAST frames are put to PSQ as long as there's any associated STA in power-save mode
	*/
	// TODO: shiang-usw, remove "ApCfg.IgmpSnoopEnable" and use "wdev->IgmpSnoopEnable" replace it!
	if (tr_entry->EntryType == ENTRY_CAT_MCAST)
	{
#ifdef IGMP_SNOOP_SUPPORT
		//TODO :  check global setting only!
		if (pAd->ApCfg.IgmpSnoopEnable /*&& wdev->IgmpSnoopEnable*/)
		{
			if (IgmpPktInfoQuery(pAd, pSrcBufVA, pPacket, wdev,
									&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS)
				return NDIS_STATUS_FAILURE;
		}

		// TODO: shiang-usw, need to revise for Igmp snooping case!!
		if (InIgmpGroup)
		{
			/* if it's a mcast packet in igmp gourp. ucast clone it for all members in the gourp. */
			if (((InIgmpGroup == IGMP_IN_GROUP)
				&& pGroupEntry
				&& (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)
				))
			{
#ifdef A4_CONN
				if (VALID_WCID(wcid) && pEntry && IS_ENTRY_A4(pEntry)) {
					/* do nothing send as it is*/
				} else
#endif
				{
					NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, pPacket, InIgmpGroup,
						pGroupEntry, QueIdx, UserPriority, GET_OS_PKT_NETDEV(pPacket));
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
					return PktCloneResult; /* need to alway return to prevent skb double free.*/
				}
			} else if (InIgmpGroup == IGMP_PKT) {
				NDIS_STATUS CloneResult =0;
				UserPriority = 0;
				QueIdx = QID_AC_BE;
				RTMP_SET_PACKET_UP(pPacket, UserPriority);
				RTMP_SET_PACKET_TXTYPE(pPacket, TX_MCAST_FRAME);

				for (Repeat=0; Repeat<3; Repeat++)
				{
					CloneResult = IgmpProtocolPktClone(pAd, pPacket, InIgmpGroup, NULL,
														QueIdx, UserPriority, GET_OS_PKT_NETDEV(pPacket));
					if (CloneResult == NDIS_STATUS_FAILURE)
						break;
#ifdef A4_CONN
					if (VALID_WCID(wcid) && pEntry && IS_ENTRY_A4(pEntry)) {
						/* Do not need retry because it is a unicast packet */
						break;
					}
#endif
				}
					RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
					return CloneResult;
			}
		}
		else 
#endif /* IGMP_SNOOP_SUPPORT */
		{
			UserPriority = 0;
			QueIdx = QID_AC_BE;
			RTMP_SET_PACKET_UP(pPacket, UserPriority);
		}
		RTMP_SET_PACKET_TXTYPE(pPacket, TX_MCAST_FRAME);
	}
	else
	{
#if defined(RTMP_MAC) || defined(RLT_MAC)
		/* detect AC Category of tx packets to tune AC0(BE) TX_OP (MAC reg 0x1300) */
		// TODO: shiang-usw, check this for REG access, it should not be here!
		if  ((pAd->chipCap.hif_type == HIF_RTMP) || (pAd->chipCap.hif_type == HIF_RLT))
			detect_wmm_traffic(pAd, UserPriority, 1);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

		RTMP_SET_PACKET_UP(pPacket, UserPriority);

	}

//DBGPRINT(RT_DEBUG_OFF, ("%s(%d): shiang-dbg, QueIdx=%d, tr_entry=%p\n", __FUNCTION__, __LINE__,  QueIdx, tr_entry));
	if (pAd->TxSwQueue[QueIdx].Number >= pAd->TxSwQMaxLen)
	{
		{
#ifdef BLOCK_NET_IF
			StopNetIfQueue(pAd, QueIdx, pPacket);
#endif /* BLOCK_NET_IF */
#ifdef DBG
			drop_reason = DROP_TXQ_FULL;
#endif /* DBG */
			goto drop_pkt;
		}
	}
#ifdef WDS_SUPPORT
	else if(IS_ENTRY_WDS(tr_entry)) {
		/* when WDS Jam happen, drop following 1min to SWQueue Pkts */
		ULONG Now32;
		NdisGetSystemUpTime(&Now32);

		if ((tr_entry->LockEntryTx == TRUE) 
			&& RTMP_TIME_BEFORE(Now32, tr_entry->TimeStamp_toTxRing + WDS_ENTRY_RETRY_INTERVAL)) {
#ifdef DBG
			drop_reason = DROP_TX_JAM;
#endif /* DBG */
			goto drop_pkt;
		} else {
			if (rtmp_enq_req(pAd, pPacket, QueIdx, tr_entry, FALSE,NULL) == FALSE) {
#ifdef DBG
				drop_reason = DROP_TXQ_ENQ_FAIL;
#endif /* DBG */
				goto drop_pkt;
			}
		}
	}
#endif /* WDS_SUPPORT */
	else
	{
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			if ((pAd->MacTab.fAnyStationInPsm == 1) && (tr_entry->EntryType == ENTRY_CAT_MCAST)) {
				if (tr_entry->tx_queue[QID_AC_BE].Number > MAX_PACKETS_IN_MCAST_PS_QUEUE) {
					DBGPRINT(RT_DEBUG_INFO, ("%s(%d): BSS tx_queue full\n", __FUNCTION__, __LINE__));
#ifdef DBG
		#ifdef NEW_IXIA_METHOD
					drop_reason = DROP_TXQ_ENQ_PS;
		#else
					drop_reason = DROP_TXQ_ENQ_FAIL;
		#endif
#endif /* DBG */
					goto drop_pkt;
				}
			} else if ((tr_entry->EntryType != ENTRY_CAT_MCAST) && (tr_entry->PsMode == PWR_SAVE)) {
				if (tr_entry->tx_queue[QID_AC_BE].Number+tr_entry->tx_queue[QID_AC_BK].Number+tr_entry->tx_queue[QID_AC_VI].Number+tr_entry->tx_queue[QID_AC_VO].Number > MAX_PACKETS_IN_PS_QUEUE) {
					DBGPRINT(RT_DEBUG_INFO, ("%s(%d): STA tx_queue full\n", __FUNCTION__, __LINE__));
#ifdef DBG
		#ifdef NEW_IXIA_METHOD
					drop_reason = DROP_TXQ_ENQ_PS;
		#else
					drop_reason = DROP_TXQ_ENQ_FAIL;
		#endif
#endif /* DBG */
					goto drop_pkt;
				}
			}
		}
#endif /* MT_MAC */
		
#ifdef UAPSD_SUPPORT
		if (IS_ENTRY_CLIENT(tr_entry) 
			&& (tr_entry->PsMode == PWR_SAVE)
			&& UAPSD_MR_IS_UAPSD_AC(&pAd->MacTab.Content[wcid], QueIdx))
		{
			UAPSD_PacketEnqueue(pAd, &pAd->MacTab.Content[wcid], pPacket, QueIdx, FALSE);
		}
		else
#endif /* UAPSD_SUPPORT */
#ifdef IP_ASSEMBLY
		if ((pAd->CommonCfg.BACapability.field.AutoBA == FALSE)  && (ret = rtmp_IpAssembleHandle(pAd,tr_entry, pPacket,QueIdx,PacketInfo))!=NDIS_STATUS_INVALID_DATA) 
		{
			if(ret == NDIS_STATUS_FAILURE)
			{
				goto nofree_drop_pkt;
			}
			/*else if success do normal path means*/
			
		}else
#endif /* IP_ASSEMBLY */
		if (rtmp_enq_req(pAd, pPacket, QueIdx, tr_entry, FALSE,NULL) == FALSE) {
#ifdef DBG
			drop_reason = DROP_TXQ_ENQ_FAIL;
#endif /* DBG */
			goto drop_pkt;
		}

		/* If the data is broadcast/multicast and any stations are in PWR_SAVE, we set BCAST TIM bit. */
		/* If the data is unicast and the station is in PWR_SAVE, we set STA TIM bit */
		if (tr_entry->EntryType == ENTRY_CAT_MCAST)
		{
			if (pAd->MacTab.fAnyStationInPsm == TRUE)
				WLAN_MR_TIM_BCMC_SET(tr_entry->func_tb_idx); /* mark MCAST/BCAST TIM bit */
		}
		else
		{
			if (IS_ENTRY_CLIENT(tr_entry) && (tr_entry->PsMode == PWR_SAVE))
			{
				/* mark corresponding TIM bit in outgoing BEACON frame */
#ifdef UAPSD_SUPPORT
				if (UAPSD_MR_IS_NOT_TIM_BIT_NEEDED_HANDLED(&pAd->MacTab.Content[wcid], QueIdx))
				{
					/*
						1. the station is UAPSD station;
						2. one of AC is non-UAPSD (legacy) AC;
						3. the destinated AC of the packet is UAPSD AC. 
					*/
					/* So we can not set TIM bit due to one of AC is legacy AC */
				}
				else
#endif /* UAPSD_SUPPORT */
				{
					WLAN_MR_TIM_BIT_SET(pAd, tr_entry->func_tb_idx, tr_entry->wcid);
				}
			}
		}
	}

//dump_tr_entry(pAd, wcid, __FUNCTION__, __LINE__);
//DBGPRINT(RT_DEBUG_OFF, ("%s(%d): shiang-dbg EnQDone done\n", __FUNCTION__, __LINE__));

	// TODO: shiang-usw, backup code here for ACM/WDS->LockTx related functions

#ifdef DOT11_N_SUPPORT
	RTMP_BASetup(pAd, tr_entry, UserPriority);
#endif /* DOT11_N_SUPPORT */

	return NDIS_STATUS_SUCCESS;

drop_pkt:	
	RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef NEW_IXIA_METHOD
	if (IS_EXPECTED_LENGTH(RTPKT_TO_OSPKT(pPacket)->len))
		pAd->tr_ststic.tx[drop_reason]++;
#endif
nofree_drop_pkt:
	/*add hook point when drop*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_DROP,NULL,QueIdx,pAd);
	DBGPRINT(RT_DEBUG_INFO, ("%s():drop pkt, drop_reason=%d!, wcid = %d\n", __FUNCTION__, drop_reason, wcid));

	return NDIS_STATUS_FAILURE;
}


/*
	--------------------------------------------------------
	FIND ENCRYPT KEY AND DECIDE CIPHER ALGORITHM
		Find the WPA key, either Group or Pairwise Key
		LEAP + TKIP also use WPA key.
	--------------------------------------------------------
	Decide WEP bit and cipher suite to be used. 
	Same cipher suite should be used for whole fragment burst
	In Cisco CCX 2.0 Leap Authentication
		WepStatus is Ndis802_11WEPEnabled but the key will use PairwiseKey
		Instead of the SharedKey, SharedKey Length may be Zero.
*/
static inline VOID APFindCipherAlgorithm(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	CIPHER_KEY *pKey = NULL;
	UCHAR KeyIdx = 0, CipherAlg = CIPHER_NONE;
	UCHAR RAWcid = pTxBlk->Wcid;
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
#if defined(APCLI_SUPPORT) && defined(WPA_SUPPLICANT_SUPPORT)
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
#endif /* defined(APCLI_SUPPORT) && defined(WPA_SUPPLICANT_SUPPORT) */
#ifdef WAPI_SUPPORT
	BSS_STRUCT *pMbss;
#endif /* WAPI_SUPPORT */
	struct wifi_dev *wdev;

	ASSERT(pTxBlk->wdev_idx < WDEV_NUM_MAX);
	wdev = pAd->wdev_list[pTxBlk->wdev_idx];

	if (wdev->wdev_type == WDEV_TYPE_WDS) {
		ASSERT(wdev->func_idx < MAX_WDS_ENTRY);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():WDS func_idx %d\n", __func__, wdev->func_idx));
	} else {
		ASSERT(wdev->func_idx < pAd->ApCfg.BssidNum);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():BSS func_idx %d\n", __func__, wdev->func_idx));
	}

#ifdef WAPI_SUPPORT
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
#endif /* WAPI_SUPPORT */
	// TODO: shiang-usw, we should use this check to replace rest of the codes!

	/* These EAPoL frames must be clear before 4-way handshaking is completed. */
	if ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)))
	{
		CipherAlg = CIPHER_NONE;
		pKey = NULL;
	} else
#ifdef APCLI_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
	{	
		APCLI_STRUCT *pApCliEntry = pTxBlk->pApCliEntry;
		wdev = &pApCliEntry->wdev;

		if (RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket)) 
		{			
			/* These EAPoL frames must be clear before 4-way handshaking is completed. */
			if ((pMacEntry->PairwiseKey.CipherAlg) && (pMacEntry->PairwiseKey.KeyLen))
			{
				CipherAlg  = pMacEntry->PairwiseKey.CipherAlg;
				pKey = &pMacEntry->PairwiseKey;
			}
			else
			{
				CipherAlg = CIPHER_NONE;
				pKey = NULL;
			}
		}
		else if (pMacEntry->WepStatus == Ndis802_11WEPEnabled)
		{
			CipherAlg  = pApCliEntry->SharedKey[wdev->DefaultKeyId].CipherAlg;
			pKey = &pApCliEntry->SharedKey[wdev->DefaultKeyId];
		}		
		else if (pMacEntry->WepStatus == Ndis802_11TKIPEnable ||
	 			 pMacEntry->WepStatus == Ndis802_11AESEnable)
		{
			CipherAlg  = pMacEntry->PairwiseKey.CipherAlg;
			pKey = &pMacEntry->PairwiseKey;
		}
		else
		{
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}			
	}
	else
#endif /* APCLI_SUPPORT */
#ifdef WDS_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk,fTX_bWDSEntry))
	{
		wdev = &pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].wdev;
		if (wdev->WepStatus == Ndis802_11WEPEnabled ||
			wdev->WepStatus == Ndis802_11TKIPEnable ||
			wdev->WepStatus == Ndis802_11AESEnable)		
		{
			CipherAlg  = pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsKey.CipherAlg;
			pKey = &pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsKey;
		}
		else
		{
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}
	}
	else
#endif /* WDS_SUPPORT */
#ifdef WAPI_SUPPORT
	if (pMbss->wdev.WepStatus == Ndis802_11EncryptionSMS4Enabled)
	{
		if (RTMP_GET_PACKET_WAI(pTxBlk->pPacket)) {
			/* WAI negotiation packet is always clear. */
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}	
		else if (!pMacEntry)
		{
			KeyIdx = pMbss->wdev.DefaultKeyId; /* MSK ID */
			CipherAlg  = pAd->SharedKey[wdev->func_idx][KeyIdx].CipherAlg;
			if (CipherAlg == CIPHER_SMS4) {
				pKey = &pAd->SharedKey[wdev->func_idx][KeyIdx];	
#ifdef SOFT_ENCRYPT
				if (pMbss->sw_wpi_encrypt) {
					TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
					/* TSC increment pre encryption transmittion */				
					inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 1);								
				}
#endif /* SOFT_ENCRYPT */
			}
		}	
		else		
		{			
			KeyIdx = pTxBlk->pMacEntry->usk_id; /* USK ID */
			CipherAlg  = pAd->MacTab.Content[RAWcid].PairwiseKey.CipherAlg;
			if (CipherAlg == CIPHER_SMS4)
			{
				pKey = &pAd->MacTab.Content[RAWcid].PairwiseKey;
#ifdef SOFT_ENCRYPT
				if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT)) {
					TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);
					/* TSC increment pre encryption transmittion */
					inc_iv_byte(pKey->TxTsc, LEN_WAPI_TSC, 2);
				}
#endif /* SOFT_ENCRYPT */
			}
		}
	}
	else
#endif /* WAPI_SUPPORT */
	if ((RTMP_GET_PACKET_EAPOL(pTxBlk->pPacket)) ||
#ifdef DOT1X_SUPPORT		
		((wdev->WepStatus == Ndis802_11WEPEnabled) && (wdev->IEEE8021X == TRUE)) || 
#endif /* DOT1X_SUPPORT */		
		(wdev->WepStatus == Ndis802_11TKIPEnable)	||
		(wdev->WepStatus == Ndis802_11AESEnable)	||
		(wdev->WepStatus == Ndis802_11TKIPAESMix))
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame))
		{
			DBGPRINT(RT_DEBUG_TRACE,("%s():clear EAP frm\n", __FUNCTION__));
			CipherAlg = CIPHER_NONE;
			pKey = NULL;
		}
		else if (!pMacEntry) /* M/BCAST to local BSS, use default key in shared key table */
		{
			KeyIdx = wdev->DefaultKeyId;
				CipherAlg  = pAd->SharedKey[wdev->func_idx][KeyIdx].CipherAlg;
			pKey = &pAd->SharedKey[wdev->func_idx][KeyIdx];			
		}
		else /* unicast to local BSS */
		{
			CipherAlg = pAd->MacTab.Content[RAWcid].PairwiseKey.CipherAlg;
			pKey = &pAd->MacTab.Content[RAWcid].PairwiseKey;

#ifdef SOFT_ENCRYPT
			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_SOFTWARE_ENCRYPT))	
			{
				TX_BLK_SET_FLAG(pTxBlk, fTX_bSwEncrypt);

				/* TSC increment pre encryption transmittion */				
				if (pKey == NULL)
					DBGPRINT(RT_DEBUG_ERROR, ("%s pKey == NULL!\n", __FUNCTION__));
				else
				{
					INC_TX_TSC(pKey->TxTsc, LEN_WPA_TSC);
				}
			}
#endif /* SOFT_ENCRYPT */
		}
	}
	else if (wdev->WepStatus == Ndis802_11WEPEnabled) /* WEP always uses shared key table */
	{
		KeyIdx = wdev->DefaultKeyId;
		CipherAlg  = pAd->SharedKey[wdev->func_idx][KeyIdx].CipherAlg;
		pKey = &pAd->SharedKey[wdev->func_idx][KeyIdx];
	}
	else
	{
		CipherAlg = CIPHER_NONE;
		pKey = NULL;
	}

	pTxBlk->CipherAlg = CipherAlg;
	if (CipherAlg)
		pTxBlk->pKey = pKey;
	pTxBlk->KeyIdx = KeyIdx;
}


#ifdef DOT11_N_SUPPORT
static inline VOID APBuildCache802_11Header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader80211;
#if defined(MESH_SUPPORT) || defined(APCLI_SUPPORT)
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
#endif
	pHeader80211 = (PHEADER_802_11)pHeader;
	tr_entry = pTxBlk->tr_entry;

	/*
		Update the cached 802.11 HEADER
	*/
	
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	
	/* More Bit */
	pHeader80211->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	
	/* Sequence */
	pHeader80211->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	
	/* SA */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
	)
	{	/* The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
		COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
		pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
	}
	else
#endif /* WDS_SUPPORT || CLIENT_WDS */
#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame))
	{
		pHeader80211->FC.ToDs = 1;
		pHeader80211->FC.FrDs = 1;
		if(pTxBlk->pMacEntry)
		{
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(pHeader80211->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);	
			}
			else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(pHeader80211->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(pHeader80211->Addr2, pTxBlk->pMacEntry->wdev->bssid);
			}
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		}
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
	}
	else
#endif /* A4_CONN */
#ifdef APCLI_SUPPORT
	if(IS_ENTRY_APCLI(pMacEntry))
	{	/* The addr3 of Ap-client packet is Destination Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
	}
	else
#endif /* APCLI_SUPPORT */
	{	/* The addr3 of normal packet send from DS is Src Mac address. */
		COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
	}
}


#ifdef HDR_TRANS_TX_SUPPORT
static inline VOID APBuildCacheWifiInfo(
	IN RTMP_ADAPTER		*pAd,
	IN TX_BLK			*pTxBlk,
	IN UCHAR			*pWiInfo)
{
	STA_TR_ENTRY *tr_entry;

	TX_WIFI_INFO *pWI;

	pWI = (TX_WIFI_INFO *)pWiInfo;
	tr_entry = pTxBlk->tr_entry;

	/* WIFI INFO size : 4 octets */
	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	/* More Bit */
	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);
	
	/* Sequence */
	pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
	tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
}
#endif /* HDR_TRANS_TX_SUPPORT */
#endif /* DOT11_N_SUPPORT */


#ifdef HDR_TRANS_TX_SUPPORT
static inline VOID APBuildWifiInfo(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;

	/* WIFI INFO size : 4 octets */
	pTxBlk->MpduHeaderLen = TX_WIFI_INFO_SIZE;

	pWI = (TX_WIFI_INFO *)&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];

	NdisZeroMemory(pWI, TX_WIFI_INFO_SIZE);

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_APCLI(pTxBlk->pMacEntry))
		pWI->field.Mode = 2;	/* STA */
	else
#endif /* APCLI_SUPPORT */
	pWI->field.Mode = 1;	/* AP */

	pWI->field.QoS = (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? 1 : 0;

	if (pTxBlk->pMacEntry && tr_entry)
	{
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM))
		{		
			pWI->field.Seq_Num = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority]+1) & MAXSEQ;
    	} else {
			pWI->field.Seq_Num = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq+1) & MAXSEQ;
    	}
		pWI->field.BssIdx = pTxBlk->pMacEntry->func_tb_idx;
	}
	else
	{
		pWI->field.Seq_Num = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence+1) & MAXSEQ; /* next sequence */
	}
	
	pWI->field.More_Data = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		pWI->field.WEP = 1;
}
#endif /* HDR_TRANS_TX_SUPPORT */


static inline VOID APBuildCommon802_11Header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;

	/*
		MAKE A COMMON 802.11 HEADER
	*/

	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	// TODO: shiang-7603
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];

	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));

	wifi_hdr->FC.FrDs = 1;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	// TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now?
	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
	    } else {
			wifi_hdr->Sequence = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
	    }
	}
	else
	{
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence */
	}
	
	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);

#ifdef A4_CONN
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bA4Frame))
	{
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 1;
		if(pTxBlk->pMacEntry)
		{
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid)); /* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);	
			}
			else
#endif /* APCLI_SUPPORT */
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
			{
				COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);/* to AP2 */
				COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pMacEntry->wdev->bssid);
			}
			COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);	/* DA */
			COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN; 
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		}
		else
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
	}
	else
#endif /* A4_CONN */
#ifdef APCLI_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
	{
		wifi_hdr->FC.ToDs = 1;
		wifi_hdr->FC.FrDs = 0;
		COPY_MAC_ADDR(wifi_hdr->Addr1, APCLI_ROOT_BSSID_GET(pAd, pTxBlk->Wcid));	/* to AP2 */
#ifdef MAC_REPEATER_SUPPORT
		if (pTxBlk->pMacEntry && (pTxBlk->pMacEntry->bReptCli == TRUE))
			COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pMacEntry->ReptCliAddr);
		else
#endif /* MAC_REPEATER_SUPPORT */
		COPY_MAC_ADDR(wifi_hdr->Addr2, pTxBlk->pApCliEntry->wdev.if_addr);		/* from AP1 */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
	}
	else
#endif /* APCLI_SUPPORT */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
	if (FALSE
#ifdef WDS_SUPPORT
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
		|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
	)
	{
		wifi_hdr->FC.ToDs = 1;
		if (pTxBlk->pMacEntry == NULL)
			DBGPRINT(RT_DEBUG_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __FUNCTION__));
		else
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);				/* to AP2 */

		COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);						/* from AP1 */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
		COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
		pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
		pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
	}
	else
#endif /* WDS_SUPPORT || CLIENT_WDS */
	{
		/* TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response */
#if defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT)
		if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST)
		{
			COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr); /* DA */
		}
		else
#endif /* defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT) */
		{
		   	COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
		}
		COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid);		/* BSSID */
		COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			/* SA */
	}

	if (pTxBlk->CipherAlg != CIPHER_NONE)
		wifi_hdr->FC.Wep = 1;
}


static inline PUCHAR AP_Build_ARalink_Frame_Header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *pHeaderBufPtr;
	HEADER_802_11 *wifi_hdr;
	PNDIS_PACKET pNextPacket;
	UINT32 nextBufLen;
	PQUEUE_ENTRY pQEntry;
		
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

	pHeaderBufPtr = pTxBlk->wifi_hdr;

	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

	/* steal "order" bit to mark "aggregation" */
	wifi_hdr->FC.Order = 1;
	
	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			 if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */
	
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;
	}

	/* padding at front of LLC header. LLC header should at 4-bytes aligment. */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

	
	/*
		For RA Aggregation, put the 2nd MSDU length(extra 2-byte field) after 
		QOS_CONTROL in little endian format
	*/
	pQEntry = pTxBlk->TxPacketList.Head;
	pNextPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	nextBufLen = GET_OS_PKT_LEN(pNextPacket);
	if (RTMP_GET_PACKET_VLAN(pNextPacket))
		nextBufLen -= LENGTH_802_1Q;
	
	*pHeaderBufPtr = (UCHAR)nextBufLen & 0xff;
	*(pHeaderBufPtr + 1) = (UCHAR)(nextBufLen >> 8);

	pHeaderBufPtr += 2;
	pTxBlk->MpduHeaderLen += 2;
	pTxBlk->wifi_hdr_len += 2;
	
	return pHeaderBufPtr;
}


#ifdef DOT11_N_SUPPORT
static inline BOOLEAN BuildHtcField(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN  MAC_TABLE_ENTRY *pMacEntry, 
	IN PUCHAR pHeaderBufPtr)
{
	BOOLEAN bHTCPlus = FALSE;
	

	return bHTCPlus;
}


static inline PUCHAR AP_Build_AMSDU_Frame_Header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf_ptr;
	HEADER_802_11 *wifi_hdr;

	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

	buf_ptr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)buf_ptr;

	if(wifi_hdr == NULL)
		DBGPRINT(RT_DEBUG_TRACE, ("wifi_hdr is NULL.\n"));

	/* skip common header */
	buf_ptr += pTxBlk->wifi_hdr_len;

	/* build QOS Control bytes */
	*buf_ptr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
		&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
	)
	{
		/* 
		 * we can not use bMoreData bit to get EOSP bit because
		 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
		 */
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
			*buf_ptr |= (1 << 4);
	}
#endif /* UAPSD_SUPPORT */

	/* A-MSDU packet */
	*buf_ptr |= 0x80;

	*(buf_ptr + 1) = 0;
	buf_ptr += 2;
	pTxBlk->MpduHeaderLen += 2;
	pTxBlk->wifi_hdr_len += 2;


	/*
		padding at front of LLC header
		LLC header should locate at 4-octets aligment
		@@@ MpduHeaderLen excluding padding @@@
	*/
	pTxBlk->HdrPadLen = (ULONG)buf_ptr;
	buf_ptr = (UCHAR *)(ROUND_UP(buf_ptr, 4));
	pTxBlk->HdrPadLen = (ULONG)(buf_ptr - pTxBlk->HdrPadLen);
		
	return buf_ptr;

}

#ifdef NEW_IXIA_METHOD
int tx_pkt_to_hw = 1;
int rx_pkt_len = 9999;
int rx_pkt_from_hw = 1;
UINT rxpktdetect2s = 1;
#endif
VOID AP_AMPDU_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr, *src_ptr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	PQUEUE_ENTRY pQEntry;
	BOOLEAN bHTCPlus = FALSE;
	UINT hdr_offset, cache_sz;
#ifdef NEW_IXIA_METHOD
	struct sk_buff *kickout_pkt = (struct sk_buff *)(pTxBlk->pPacket);
#endif
	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
		{
			pMbss->TxDropCount++;
		}
#endif /* STATS_COUNT_SUPPORT */
#ifdef NEW_IXIA_METHOD
		if (IS_EXPECTED_LENGTH(kickout_pkt->len))
			pAd->tr_ststic.tx[DROP_BLK_INFO_ERROR]++;
#endif
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

#if defined(MT7603) || defined(MT7628)
	// TODO: shiang-7603
	hdr_offset = pAd->chipCap.tx_hw_hdr_len;
#else
	hdr_offset = TXINFO_SIZE + pAd->chipCap.TXWISize + TSO_SIZE;
#endif /* defined(MT7603) || defined(MT7628) */
	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;
	if ((tr_entry->isCached)
	)
	{
#if defined(MT7603) || defined(MT7628)
#ifndef VENDOR_FEATURE1_SUPPORT
		NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[0]), 
							(UCHAR *)(&tr_entry->CachedBuf[0]),
							pAd->chipCap.tx_hw_hdr_len + sizeof(HEADER_802_11));
#else
		pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */
#else
#ifndef VENDOR_FEATURE1_SUPPORT
		NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
							(UCHAR *)(&tr_entry->CachedBuf[0]),
							pAd->chipCap.TXWISize + sizeof(HEADER_802_11));
#else
		pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */
#endif /* defined(MT7603) || defined(MT7628) */

		pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[hdr_offset]);
		APBuildCache802_11Header(pAd, pTxBlk, pHeaderBufPtr);

#ifdef SOFT_ENCRYPT
		RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
	}
	else
	{
		APFindCipherAlgorithm(pAd, pTxBlk);
		APBuildCommon802_11Header(pAd, pTxBlk);
			
		pHeaderBufPtr = &pTxBlk->HeaderBuf[hdr_offset];
	}

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */

	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

//+++Add by shiang for debug
//---Add by shiang for debug

	/* skip common header */
	pHeaderBufPtr += pTxBlk->MpduHeaderLen;
		
#ifdef VENDOR_FEATURE1_SUPPORT
	if (tr_entry->isCached
		&& (tr_entry->Protocol == (RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket)))
#ifdef SOFT_ENCRYPT
		&& !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif /* SOFT_ENCRYPT */
	)
	{
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */
		pTxBlk->MpduHeaderLen = tr_entry->MpduHeaderLen;
		pTxBlk->wifi_hdr_len = tr_entry->wifi_hdr_len;
		pHeaderBufPtr = ((UCHAR *)wifi_hdr) + pTxBlk->MpduHeaderLen;

		pTxBlk->HdrPadLen = tr_entry->HdrPadLen;

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}
	}
	else
#endif /* VENDOR_FEATURE1_SUPPORT */
	{
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			)
		{
			/* 
			 * we can not use bMoreData bit to get EOSP bit because
			 * maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */

		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;

#ifndef MT_MAC
		// TODO: Shiang-usw, we need a more proper way to handle this instead of ifndef MT_MAC !
		/* For MT_MAC, SW not to prepare the HTC field for RDG enable */
		/* build HTC control field after QoS field */
		if ((pAd->CommonCfg.bRdg == TRUE)
			&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
		)
		{
			NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
			((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
			bHTCPlus = TRUE;
		}
#endif /* MT_MAC */


		if (bHTCPlus == TRUE)
		{
			wifi_hdr->FC.Order = 1;
			pHeaderBufPtr += 4;
			pTxBlk->MpduHeaderLen += 4;
			pTxBlk->wifi_hdr_len += 4;
		}

		/*pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE; */
		ASSERT(pTxBlk->MpduHeaderLen >= 24);

		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
		{
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}

		/*
		   padding at front of LLC header
		   LLC header should locate at 4-octets aligment

		   @@@ MpduHeaderLen excluding padding @@@
		*/
		pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
		pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
		pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);

#ifdef VENDOR_FEATURE1_SUPPORT
		tr_entry->HdrPadLen = pTxBlk->HdrPadLen;
#endif /* VENDOR_FEATURE1_SUPPORT */

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
			tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
		}
		else
#endif /* SOFT_ENCRYPT */
		{

			/*
				Insert LLC-SNAP encapsulation - 8 octets
				if original Ethernet frame contains no LLC/SNAP,
				then an extra LLC/SNAP encap is required
			*/
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);
			if (pTxBlk->pExtraLlcSnapEncap) {
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
				pHeaderBufPtr += 6;
				/* get 2 octets (TypeofLen) */
				NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);

				pHeaderBufPtr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		tr_entry->Protocol = RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket);
		tr_entry->MpduHeaderLen = pTxBlk->MpduHeaderLen;
		tr_entry->wifi_hdr_len = pTxBlk->wifi_hdr_len;
#endif /* VENDOR_FEATURE1_SUPPORT */
	}

	if ((tr_entry->isCached)
	)
	{
		write_tmac_info_Cache(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
	}
	else
	{
		write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

		if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
			tr_entry->isCached = FALSE;


		NdisZeroMemory((UCHAR *)(&tr_entry->CachedBuf[0]), sizeof(tr_entry->CachedBuf));
#if defined(MT7603) || defined(MT7628)
		cache_sz = (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0]));
		src_ptr = (UCHAR *)&pTxBlk->HeaderBuf[0];
#else
		cache_sz = (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]));
		src_ptr = (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);
#endif /* defined(MT7603) || defined(MT7628) */
		NdisMoveMemory((UCHAR *)(&tr_entry->CachedBuf[0]), src_ptr, cache_sz);

#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), sizeof(tr_entry->HeaderBuf));
#if defined(MT7603) || defined(MT7628)
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), 
						(UCHAR *)(&pTxBlk->HeaderBuf[0]), cache_sz);
#else
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]),
						(UCHAR *)(&pTxBlk->HeaderBuf[0]),
						(pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0])));
#endif /* defined(MT7603) || defined(MT7628) */
#endif /* VENDOR_FEATURE1_SUPPORT */

//+++Mark by shiang test
//		tr_entry->isCached = TRUE;
//---Mark by shiang for test
	}

//+++Add by shiang for debug
#if defined(MT7603) || defined(MT7628)
#endif /* MT7603 */
//---Add by shiang for debug


#ifdef STATS_COUNT_SUPPORT
	pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart++;
	pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;

	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

#ifdef WAPI_SUPPORT
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
			if (pMacEntry->WapiUskRekeyTimerRunning && 
				pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
		}
#endif /* WAPI_SUPPORT */
			
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;			
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef DYNAMIC_WMM
            pAd->TxTotalByteCnt_Dyn[pTxBlk->QueIdx] += pTxBlk->SrcBufLen;
#endif /* DYNAMIC_WMM */
		}
	}

#ifdef WDS_SUPPORT
		if (pMacEntry && IS_ENTRY_WDS(pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);				
			pAd->WdsTab.WdsEntry[pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
#ifdef NEW_IXIA_METHOD
	if (IS_EXPECTED_LENGTH(kickout_pkt->len))
		tx_pkt_to_hw++;
#endif
	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
}


#ifdef HDR_TRANS_TX_SUPPORT
VOID AP_AMPDU_Frame_Tx_Hdr_Trns(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
	PUCHAR			pWiBufPtr;
	USHORT			FreeNumber = 1; /* no use */
	MAC_TABLE_ENTRY	*pMacEntry;
	STA_TR_ENTRY *tr_entry;
	PQUEUE_ENTRY	pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;
	
	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount ++;
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_2[index]));
	}
*/

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;
	if ((tr_entry->isCached)
	)
	{
		/* It should be cleared!!! */
		/*NdisZeroMemory((PUCHAR)(&pTxBlk->HeaderBuf[0]), sizeof(pTxBlk->HeaderBuf)); */ 
		NdisMoveMemory((PUCHAR)
			       (&pTxBlk->HeaderBuf[TXINFO_SIZE]),
			       (PUCHAR) (&tr_entry->CachedBuf[0]),
			       TXWISize + TX_WIFI_INFO_SIZE);

		pWiBufPtr = (PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize]);
		APBuildCacheWifiInfo(pAd, pTxBlk, pWiBufPtr);
	}
	else 
	{
		APFindCipherAlgorithm(pAd, pTxBlk);
		APBuildWifiInfo(pAd, pTxBlk);
			
		pWiBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
	}

	pWI = (TX_WIFI_INFO *)pWiBufPtr;

	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket))
		pWI->field.VLAN = TRUE;

	pWI->field.TID = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
	if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
		&& TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
		pWI->field.EOSP = TRUE;
#endif /* UAPSD_SUPPORT */

	{

		/*
			build HTC+
			HTC control filed following QoS field
		*/
		if ((pAd->CommonCfg.bRdg == TRUE) 
			&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
		)
		{
			pWI->field.RDG = 1;
		}

	}

/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_3[index]));
	}
*/

	if ((tr_entry->isCached)
	)
	{
		write_tmac_info_Cache(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
	}
	else
	{
		write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

		NdisZeroMemory((PUCHAR)(&tr_entry->CachedBuf[0]), sizeof(tr_entry->CachedBuf));
		NdisMoveMemory((PUCHAR)(&tr_entry->CachedBuf[0]), 
						(PUCHAR)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), 
						TXWISize + TX_WIFI_INFO_SIZE);


		tr_entry->isCached = TRUE;

	}

#ifdef STATS_COUNT_SUPPORT
	pAd->RalinkCounters.TransmittedMPDUsInAMPDUCount.u.LowPart ++;
	pAd->RalinkCounters.TransmittedOctetsInAMPDUCount.QuadPart += pTxBlk->SrcBufLen;		

	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

#ifdef WAPI_SUPPORT
		if (IS_ENTRY_CLIENT(pMacEntry))
		{
		if (pMacEntry->WapiUskRekeyTimerRunning && 
			pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
		}
#endif /* WAPI_SUPPORT */
			
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;
#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;			
		}
	}

#endif /* STATS_COUNT_SUPPORT */

	/*FreeNumber = GET_TXRING_FREENO(pAd, QueIdx); */

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);


	/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_4[index]));
	}
*/

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
	
/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_5[index])); 
		if ( pAd->debug_index < 201 )
		{
			pAd->debug_index ++;
		} else {
			pAd->debug_on = 0;
		}
	}
*/

}
#endif /* HDR_TRANS_TX_SUPPORT */


VOID AP_AMSDU_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *pHeaderBufPtr, *subFrameHeader;
	USHORT freeCnt = 1; /* no use */
	USHORT subFramePayloadLen = 0;	/* AMSDU Subframe length without AMSDU-Header / Padding */
	USHORT totalMPDUSize = 0;
	UCHAR padding = 0;
	USHORT FirstTx = 0/*, LastTxIdx = 0*/;
	int frameNum = 0;
	PQUEUE_ENTRY pQEntry;
	//STA_TR_ENTRY *tr_entry;
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	PAPCLI_STRUCT pApCliEntry = NULL;
#endif /* APCLI_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT
REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#ifdef WFA_VHT_PF
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
#endif /* WFA_VHT_PF */
	
	ASSERT((pTxBlk->TxPacketList.Number > 1));

	while (pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
			BSS_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
			{
				pMbss->TxDropCount++;
			}
#endif /* STATS_COUNT_SUPPORT */
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
		
		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket)) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}
		
		if (frameNum == 0)
		{
			pHeaderBufPtr = AP_Build_AMSDU_Frame_Header(pAd, pTxBlk);

			/* NOTE: TxWI->TxWIMPDUByteCnt will be updated after final frame was handled. */
#ifdef WFA_VHT_PF
			if (pAd->force_amsdu)
			{
				UCHAR RABAOriIdx;

				if (pMacEntry) {
					 RABAOriIdx = pMacEntry->BAOriWcidArray[pTxBlk->UserPriority];
					if (((pMacEntry->TXBAbitmap & (1<<pTxBlk->UserPriority)) != 0) &&
						(pAd->BATable.BAOriEntry[RABAOriIdx].amsdu_cap == TRUE))
						TX_BLK_SET_FLAG (pTxBlk, fTX_AmsduInAmpdu);
				}
			}
#endif /* WFA_VHT_PF */

			write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);


			if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
				if (pTxBlk->tr_entry) 
					pTxBlk->tr_entry->isCached = FALSE;
		}
		else
		{
#if defined(MT7603) || defined(MT7628)
			pHeaderBufPtr = &pTxBlk->HeaderBuf[0];
#else
			// TODO: shiang-usw, check this, original code is use pTxBlk->HeaderBuf[0]
			pHeaderBufPtr = &pTxBlk->HeaderBuf[TXINFO_SIZE];
#endif /* defined(MT7603) || defined(MT7628) */
			padding = ROUND_UP(AMSDU_SUBHEAD_LEN + subFramePayloadLen, 4) - (AMSDU_SUBHEAD_LEN + subFramePayloadLen);
			NdisZeroMemory(pHeaderBufPtr, padding + AMSDU_SUBHEAD_LEN);
			pHeaderBufPtr += padding;
			pTxBlk->MpduHeaderLen = padding;
		}

		/*
			A-MSDU subframe
				DA(6)+SA(6)+Length(2) + LLC/SNAP Encap
		*/
		subFrameHeader = pHeaderBufPtr;
		subFramePayloadLen = pTxBlk->SrcBufLen;

		NdisMoveMemory(subFrameHeader, pTxBlk->pSrcBufHeader, 12);

#ifdef APCLI_SUPPORT
		if(TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket))
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pTxBlk->pMacEntry->bReptCli)
			{
				pReptEntry = &pAd->ApCfg.ApCliTab[pTxBlk->pMacEntry->func_tb_idx].RepeaterCli[pTxBlk->pMacEntry->MatchReptCliIdx];
				if (pReptEntry->CliValid)
					NdisMoveMemory(&subFrameHeader[6] , pReptEntry->CurrentAddress, 6);
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry = &pAd->ApCfg.ApCliTab[pTxBlk->pMacEntry->func_tb_idx];
				if (pApCliEntry->Valid)
					NdisMoveMemory(&subFrameHeader[6] , pApCliEntry->wdev.if_addr, 6);
			}
		}
#endif /* APCLI_SUPPORT */


		pHeaderBufPtr += AMSDU_SUBHEAD_LEN;
		pTxBlk->MpduHeaderLen += AMSDU_SUBHEAD_LEN;



		/* Insert LLC-SNAP encapsulation - 8 octets */
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2, pTxBlk->pExtraLlcSnapEncap);

		subFramePayloadLen = pTxBlk->SrcBufLen;

		if (pTxBlk->pExtraLlcSnapEncap) {
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pSrcBufData - 2, 2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			subFramePayloadLen += LENGTH_802_1_H;
		}

		/* update subFrame Length field */
		subFrameHeader[12] = (subFramePayloadLen & 0xFF00) >> 8;
		subFrameHeader[13] = subFramePayloadLen & 0xFF;

#if defined(MT7603) || defined(MT7628)
		if (frameNum == 0)
			totalMPDUSize += pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen + pTxBlk->SrcBufLen;
		else
		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
#else
		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
#endif /* MT7603 */

		if (frameNum == 0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);
		else
			/*LastTxIdx = */HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;


		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef STATS_COUNT_SUPPORT
		{
			/* calculate Transmitted AMSDU Count and ByteCount */
			pAd->RalinkCounters.TxAMSDUCount.u.LowPart++;
		}

		/* calculate Tx count and ByteCount per BSS */
		{
			BSS_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
			if (IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
			{
			if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->wapi_usk_rekey_cnt += totalMPDUSize;
			}
#endif /* WAPI_SUPPORT */
		
			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;
#ifdef STATS_COUNT_SUPPORT
				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
			}

			if(pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
				pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;
                pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef DYNAMIC_WMM
            pAd->TxTotalByteCnt_Dyn[pTxBlk->QueIdx] += pTxBlk->SrcBufLen;
#endif /* DYNAMIC_WMM */
			}
		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}	
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, 0/*LastTxIdx*/);
	
	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
}
#endif /* DOT11_N_SUPPORT */


VOID AP_Legacy_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	QUEUE_ENTRY *pQEntry;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;
	ASSERT(pTxBlk);

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
		{
			pMbss->TxDropCount++;
		}
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
DBGPRINT(RT_DEBUG_TRACE, ("<--%s(%d): ##########Fail#########\n", __FUNCTION__, __LINE__));
		return;
	}
	
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */

	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;

	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	/* record these MCAST_TX frames for group key rekey */
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
	{				
		INT	idx;

#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
		{
			if (pAd->ApCfg.MBSSID[idx].REKEYTimerRunning && 
				pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
			{
				pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);
			}
		}
#ifdef WAPI_SUPPORT
		if (pAd->CommonCfg.WapiMskRekeyTimerRunning &&
			pAd->CommonCfg.wapi_msk_rekey_method == REKEY_METHOD_PKT)
		{
			pAd->CommonCfg.wapi_msk_rekey_cnt += (pTxBlk->SrcBufLen);
		}		
#endif /* WAPI_SUPPORT */
	}
	else
	{
		/* Unicast */
		if (pTxBlk->tr_entry && pTxBlk->tr_entry->PsDeQWaitCnt)
			pTxBlk->Pid = PID_PS_DATA;
	}

#if defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT)	
	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket) & 0x80)
	{
		pTxBlk->Pid = PID_WSC_EAP;
	}
#endif /* defined(MT_MAC) && defined(WSC_INCLUDED) && defined(CONFIG_AP_SUPPORT) */

	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;
	if(wifi_hdr == NULL)
		DBGPRINT(RT_DEBUG_TRACE, ("wifi_hdr is NULL.\n"));

	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] << 5));
#ifdef WFA_VHT_PF
		if (pAd->force_noack) 
			*pHeaderBufPtr |= (1 << 5);
#endif /* WFA_VHT_PF */

#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		)
		{
			/* 
				we can not use bMoreData bit to get EOSP bit because
				maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */
	
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;

	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
	}
	else
#endif /* SOFT_ENCRYPT */
	{

		/*
			Insert LLC-SNAP encapsulation - 8 octets
			if original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
							pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;

			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
						pTxBlk->pSrcBufHeader + 12 + vlan_size,
						2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

#ifdef STATS_COUNT_SUPPORT
	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
		{
		if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
		}
#endif /* WAPI_SUPPORT */
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef DYNAMIC_WMM
            pAd->TxTotalByteCnt_Dyn[pTxBlk->QueIdx] += pTxBlk->SrcBufLen;
#endif /* DYNAMIC_WMM */
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	/*
		prepare for TXWI
		use Wcid as Hardware Key Index
	*/

	/* update Hardware Group Key Index */
	if (!pTxBlk->pMacEntry) {
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
		ASSERT(wdev != NULL);
		if(wdev != NULL)
		{
			GET_GroupKey_WCID(pAd, pTxBlk->Wcid, wdev->func_idx);
		}
	}

	write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

//hex_dump("Legacy_Frame-FirstBufContent", pTxBlk->HeaderBuf, 128);
//hex_dump("Legacy_Frame-FirstBufContent - WiFi Hdr Segment", pTxBlk->wifi_hdr, pTxBlk->wifi_hdr_len);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): pTxBlk->MpduHeaderLen=%d, wifi_hdr_len=%d, HdrPadLen=%d, hw_rsv_len=%d\n",
//			__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen, pTxBlk->hw_rsv_len));
//dump_tmac_info(pAd, &pTxBlk->HeaderBuf[pTxBlk->hw_rsv_len]);

	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
		if (pTxBlk->tr_entry)
			pTxBlk->tr_entry->isCached = FALSE;

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &freeCnt);
	

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

//DBGPRINT(RT_DEBUG_TRACE, ("<--%s(%d):Success\n", __FUNCTION__, __LINE__));
}


#ifdef HDR_TRANS_TX_SUPPORT
VOID AP_Legacy_Frame_Tx_Hdr_Trns(
	IN	PRTMP_ADAPTER	pAd,
	IN	TX_BLK			*pTxBlk)
{
/*	UCHAR QueIdx = pTxBlk->QueIdx; */
	USHORT FreeNumber = 1; /* no use */
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	UINT8 TXWISize = pAd->chipCap.TXWISize;
	TX_WIFI_INFO *pWI;
	
	ASSERT(pTxBlk);


	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE)
	{
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
			pMbss->TxDropCount++;
#endif /* STATS_COUNT_SUPPORT */
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return;
	}

#ifdef STATS_COUNT_SUPPORT
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME)
	{
		INC_COUNTER64(pAd->WlanCounters.MulticastTransmittedFrameCount);
	}
#endif /* STATS_COUNT_SUPPORT */

	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	
	APFindCipherAlgorithm(pAd, pTxBlk);

/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_2[index]));
	}
*/

	APBuildWifiInfo(pAd, pTxBlk);

	pWI = (TX_WIFI_INFO *)&pTxBlk->HeaderBuf[TXINFO_SIZE + TXWISize];
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	if (bVLANPkt)
		pWI->field.VLAN = TRUE;
	
	pWI->field.TID = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM) 
		&& CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
		&& TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
		pWI->field.EOSP = TRUE;
#endif /* UAPSD_SUPPORT */

#ifdef STATS_COUNT_SUPPORT
	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
		if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
		{
		if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
		}
#endif /* WAPI_SUPPORT */		
	
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;
		}
	}

#endif /* STATS_COUNT_SUPPORT */

	/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_3[index]));
	}
*/

	/*
		prepare for TXWI
	*/

	/* update Hardware Group Key Index */
	if (!pTxBlk->pMacEntry)
	{
		/* use Wcid as Hardware Key Index */
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
		ASSERT(wdev != NULL);
		GET_GroupKey_WCID(pAd, pTxBlk->Wcid, wdev->func_idx);
	}

	write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

	/*FreeNumber = GET_TXRING_FREENO(pAd, QueIdx); */

	HAL_WriteTxResource(pAd, pTxBlk, TRUE, &FreeNumber);
	

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
	if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
		dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)pHeader_802_11);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;

	/*
	if (pAd->debug_on)
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_4[index]));
	}
*/

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

/*
	if ( pAd->debug_on )
	{
		UCHAR index = RTMP_GET_DEBUG_INDEX(pTxBlk->pPacket);
		do_gettimeofday(&(pAd->debug_time_5[index])); 
		if ( pAd->debug_index < 201 )
		{
			pAd->debug_index ++;
		} else {
			pAd->debug_on = 0;
		}
			
	}
*/

}
#endif /* HDR_TRANS_TX_SUPPORT */


VOID AP_Fragment_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	QUEUE_ENTRY *pQEntry;
	//UINT8 TXWISize = pAd->chipCap.TXWISize;
	PACKET_INFO PacketInfo;
#ifdef SOFT_ENCRYPT
	UCHAR *tmp_ptr = NULL;
	UINT32 buf_offset = 0;
#endif /* SOFT_ENCRYPT */
	HTTRANSMIT_SETTING *pTransmit;
	UCHAR fragNum = 0;
	USHORT EncryptionOverhead = 0;
	UINT32 FreeMpduSize, SrcRemainingBytes;
	USHORT AckDuration;
	UINT NextMpduSize;
	//UINT8 tx_hw_hdr_len = pAd->chipCap.tx_hw_hdr_len;
	
	ASSERT(pTxBlk);
	ASSERT(TX_BLK_TEST_FLAG(pTxBlk, fTX_bAllowFrag));

//DBGPRINT(RT_DEBUG_TRACE, ("%s()-->\n", __FUNCTION__));

	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
	if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
		BSS_STRUCT *pMbss = pTxBlk->pMbss;

		if (pMbss != NULL)
		{
			pMbss->TxDropCount++;
		}
#endif /* STATS_COUNT_SUPPORT */

		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
DBGPRINT(RT_DEBUG_TRACE, ("<--%s(%d): ##########Fail#########\n", __FUNCTION__, __LINE__));
		return;
	}
	
	APFindCipherAlgorithm(pAd, pTxBlk);
	APBuildCommon802_11Header(pAd, pTxBlk);
	
#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
	}
#endif /* SOFT_ENCRYPT */
	
	if (pTxBlk->CipherAlg == CIPHER_TKIP) {
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);
		if (pTxBlk->pPacket == NULL)
			return;
		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}
	
	/* skip 802.3 header */
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
	pTxBlk->SrcBufLen -= LENGTH_802_3;

	/* skip vlan tag */
	bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
	if (bVLANPkt) {
		pTxBlk->pSrcBufData += LENGTH_802_1Q;
		pTxBlk->SrcBufLen -= LENGTH_802_1Q;
	}

	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;

DBGPRINT(RT_DEBUG_TRACE, ("%s(): Before Frag, pTxBlk->MpduHeaderLen=%d, wifi_hdr_len=%d, HdrPadLen=%d, hw_rsv_len=%d\n",
			__FUNCTION__, pTxBlk->MpduHeaderLen, pTxBlk->wifi_hdr_len, pTxBlk->HdrPadLen, pTxBlk->hw_rsv_len));

	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		/* build QOS Control bytes */
		*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT
		if (pTxBlk->pMacEntry &&
			CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		)
		{
			/* 
				we can not use bMoreData bit to get EOSP bit because
				maybe bMoreData = 1 & EOSP = 1 when Max SP Length != 0 
			 */
			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP))
				*pHeaderBufPtr |= (1 << 4);
		}
#endif /* UAPSD_SUPPORT */
	
		*(pHeaderBufPtr + 1) = 0;
		pHeaderBufPtr += 2;
		pTxBlk->MpduHeaderLen += 2;
		pTxBlk->wifi_hdr_len += 2;
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		UCHAR iv_offset = 0;
	
		/*
			If original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
						    pTxBlk->pExtraLlcSnapEncap);
		
		/* Insert LLC-SNAP encapsulation (8 octets) to MPDU data buffer */
		if (pTxBlk->pExtraLlcSnapEncap) {
			/* Reserve the front 8 bytes of data for LLC header */
			pTxBlk->pSrcBufData -= LENGTH_802_1_H;
			pTxBlk->SrcBufLen += LENGTH_802_1_H;

			NdisMoveMemory(pTxBlk->pSrcBufData, pTxBlk->pExtraLlcSnapEncap, 6);
		}
			
		/* Construct and insert specific IV header to MPDU header */
		RTMPSoftConstructIVHdr(pTxBlk->CipherAlg,
							   pTxBlk->KeyIdx,
							   pTxBlk->pKey->TxTsc,
							   pHeaderBufPtr, &iv_offset);
		pHeaderBufPtr += iv_offset;
		pTxBlk->MpduHeaderLen += iv_offset;

	}
	else
#endif /* SOFT_ENCRYPT */
	{

		/*
			Insert LLC-SNAP encapsulation - 8 octets
			if original Ethernet frame contains no LLC/SNAP,
			then an extra LLC/SNAP encap is required
		*/
		EXTRA_LLCSNAP_ENCAP_FROM_PKT_START(pTxBlk->pSrcBufHeader,
							pTxBlk->pExtraLlcSnapEncap);
		if (pTxBlk->pExtraLlcSnapEncap) {
			UCHAR vlan_size;
	
			NdisMoveMemory(pHeaderBufPtr, pTxBlk->pExtraLlcSnapEncap, 6);
			pHeaderBufPtr += 6;
			/* skip vlan tag */
			vlan_size = (bVLANPkt) ? LENGTH_802_1Q : 0;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(pHeaderBufPtr,
						pTxBlk->pSrcBufHeader + 12 + vlan_size,
						2);
			pHeaderBufPtr += 2;
			pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
		}
	}

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode, 
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (pTxBlk->CipherAlg == CIPHER_TKIP) {
		struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
		ASSERT(wdev != NULL);
		if(wdev != NULL)
		{
			RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey, wdev->func_idx);
		}

		/*
			NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
				to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress.
		*/
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

#ifdef STATS_COUNT_SUPPORT
	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *pMbss = pTxBlk->pMbss;
		MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
		{
		if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
			pTxBlk->pMacEntry->wapi_usk_rekey_cnt += pTxBlk->SrcBufLen;
		}
#endif /* WAPI_SUPPORT */
	
		if (pMbss != NULL)
		{
			pMbss->TransmittedByteCount += pTxBlk->SrcBufLen;
			pMbss->TxCount ++;

#ifdef STATS_COUNT_SUPPORT
			if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->mcPktsTx++;
			else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
				pMbss->bcPktsTx++;
			else
				pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
		}

		if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
		{
			INC_COUNTER64(pMacEntry->TxPackets);
			pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
			pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;
            pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef DYNAMIC_WMM
            pAd->TxTotalByteCnt_Dyn[pTxBlk->QueIdx] += pTxBlk->SrcBufLen;
#endif /* DYNAMIC_WMM */
		}
	}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

	/*
		calcuate the overhead bytes that encryption algorithm may add. This
		affects the calculate of "duration" field
	*/
	if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128) || (pTxBlk->CipherAlg == CIPHER_WEP152))
		EncryptionOverhead = 8; /* WEP: IV[4] + ICV[4]; */
	else if (pTxBlk->CipherAlg == CIPHER_TKIP)
		EncryptionOverhead = 12; /* TKIP: IV[4] + EIV[4] + ICV[4], MIC will be added to TotalPacketLength */
	else if (pTxBlk->CipherAlg == CIPHER_AES)
		EncryptionOverhead = 16;	/* AES: IV[4] + EIV[4] + MIC[8] */
#ifdef WAPI_SUPPORT
	else if (pTxBlk->CipherAlg == CIPHER_SMS4)
		EncryptionOverhead = 16;	/* SMS4: MIC[16] */
#endif /* WAPI_SUPPORT */
	else
		EncryptionOverhead = 0;

	pTransmit = pTxBlk->pTransmit;
	/* Decide the TX rate */
	if (pTransmit->field.MODE == MODE_CCK)
		pTxBlk->TxRate = pTransmit->field.MCS;
	else if (pTransmit->field.MODE == MODE_OFDM)
		pTxBlk->TxRate = pTransmit->field.MCS + RATE_FIRST_OFDM_RATE;
	else
		pTxBlk->TxRate = RATE_6_5;

	/* decide how much time an ACK/CTS frame will consume in the air */
	if (pTxBlk->TxRate <= RATE_LAST_OFDM_RATE)
		AckDuration = RTMPCalcDuration(pAd, pAd->CommonCfg.ExpectedACKRate[pTxBlk->TxRate], 14);
	else
		AckDuration = RTMPCalcDuration(pAd, RATE_6_5, 14);
	/*DBGPRINT(RT_DEBUG_INFO, ("!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, pTxBlk->TxRate)); */

#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, pTxBlk->SrcBufLen);
		if (tmp_ptr == NULL) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s():no memory for MIC calculation!\n",
										__FUNCTION__));
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return;
		}
		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}
#endif /* SOFT_ENCRYPT */

	/* Init the total payload length of this frame. */
	SrcRemainingBytes = pTxBlk->SrcBufLen;
	pTxBlk->TotalFragNum = 0xff;

	do {
		FreeMpduSize = pAd->CommonCfg.FragmentThreshold - LENGTH_CRC;
		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize)
		{
			/* This is the last or only fragment */
			pTxBlk->SrcBufLen = SrcRemainingBytes;
			
			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			
			/* Indicate the lower layer that this's the last fragment. */
			pTxBlk->TotalFragNum = fragNum;
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
#endif /* MT_MAC */
		}
		else
		{	/* more fragment is required */
			pTxBlk->SrcBufLen = FreeMpduSize;
			
			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen),
								((UINT)pAd->CommonCfg.FragmentThreshold));
			wifi_hdr->FC.MoreFrag = 1;
			wifi_hdr->Duration = (3 * pAd->CommonCfg.Dsifs) + (2 * AckDuration) + 
								RTMPCalcDuration(pAd, pTxBlk->TxRate, NextMpduSize + EncryptionOverhead);
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_FIRST : TX_FRAG_ID_MIDDLE);
#endif /* MT_MAC */
		}

		SrcRemainingBytes -= pTxBlk->SrcBufLen;

		if (fragNum == 0)
			pTxBlk->FrameGap = IFS_HTTXOP;
		else
			pTxBlk->FrameGap = IFS_SIFS;
		
#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{
			UCHAR ext_offset = 0;
		
			NdisMoveMemory(pTxBlk->pSrcBufData, tmp_ptr + buf_offset, pTxBlk->SrcBufLen);
			buf_offset += pTxBlk->SrcBufLen;

			/* Encrypt the MPDU data by software */
			RTMPSoftEncryptionAction(pAd,
									pTxBlk->CipherAlg,
									(UCHAR *)wifi_hdr,
									pTxBlk->pSrcBufData,
									pTxBlk->SrcBufLen,
									pTxBlk->KeyIdx,
									   pTxBlk->pKey,
									 &ext_offset);
			pTxBlk->SrcBufLen += ext_offset;
			pTxBlk->TotalFrameLen += ext_offset;
		}
#endif /* SOFT_ENCRYPT */
		write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);


		HAL_WriteFragTxResource(pAd, pTxBlk, fragNum, &freeCnt);
		

#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), (UCHAR *)wifi_hdr);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */
		
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef SOFT_ENCRYPT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		{
#ifdef WAPI_SUPPORT
			if (pTxBlk->CipherAlg == CIPHER_SMS4)
			{
				/* incease WPI IV for next MPDU */
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WAPI_TSC, 2);
				/* Construct and insert WPI-SMS4 IV header to MPDU header */
				RTMPConstructWPIIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
								 pHeaderBufPtr - (LEN_WPI_IV_HDR));
			}
			else
#endif /* WAPI_SUPPORT */
			if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128))
			{
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
				/* Construct and insert 4-bytes WEP IV header to MPDU header */
				RTMPConstructWEPIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										pHeaderBufPtr - (LEN_WEP_IV_HDR));
			}
			else if (pTxBlk->CipherAlg == CIPHER_TKIP)
				;
			else if (pTxBlk->CipherAlg == CIPHER_AES)
			{
				inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
				/* Construct and insert 8-bytes CCMP header to MPDU header */
				RTMPConstructCCMPHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										pHeaderBufPtr - (LEN_CCMP_HDR));
			}
		}
		else
#endif /* SOFT_ENCRYPT */
		{
			/* Update the frame number, remaining size of the NDIS packet payload. */
			if (fragNum == 0 && pTxBlk->pExtraLlcSnapEncap)
				pTxBlk->MpduHeaderLen -= LENGTH_802_1_H;	/* space for 802.11 header. */
		}

		fragNum++;
		/* SrcRemainingBytes -= pTxBlk->SrcBufLen; */
		pTxBlk->pSrcBufData += pTxBlk->SrcBufLen;
		
		wifi_hdr->Frag++;	/* increase Frag # */
		
	} while (SrcRemainingBytes > 0);

#ifdef SOFT_ENCRYPT
	if (tmp_ptr != NULL)
		os_free_mem(pAd, tmp_ptr);
#endif /* SOFT_ENCRYPT */

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);
		
//DBGPRINT(RT_DEBUG_TRACE, ("<--%s(%d):Success\n", __FUNCTION__, __LINE__));
}


VOID AP_ARalink_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf_ptr;
	USHORT freeCnt = 1;
	USHORT totalMPDUSize = 0;
	USHORT FirstTx, LastTxIdx;
	int frameNum = 0;
	BOOLEAN bVLANPkt;
	PQUEUE_ENTRY pQEntry;
	ASSERT(pTxBlk);
	ASSERT((pTxBlk->TxPacketList.Number == 2));

	FirstTx = LastTxIdx = 0;  /* Is it ok init they as 0? */
	while (pTxBlk->TxPacketList.Head)
	{
		pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
		pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (RTMP_FillTxBlkInfo(pAd, pTxBlk) != TRUE) {
#ifdef STATS_COUNT_SUPPORT
			BSS_STRUCT *pMbss = pTxBlk->pMbss;

			if (pMbss != NULL)
			{
				pMbss->TxDropCount++;
			}
#endif /* STATS_COUNT_SUPPORT */

			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}
		
		/* skip 802.3 header */
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
		pTxBlk->SrcBufLen -= LENGTH_802_3;

		/* skip vlan tag */
		bVLANPkt = (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket) ? TRUE : FALSE);
		if (bVLANPkt) {
			pTxBlk->pSrcBufData += LENGTH_802_1Q;
			pTxBlk->SrcBufLen -= LENGTH_802_1Q;
		}
		
		/*
			For first frame, we need to create:
				802.11 header + padding(optional) + RA-AGG-LEN + SNAP Header
			For second aggregated frame, we need create:
				the 802.3 header to headerBuf, because PCI will copy it to SDPtr0
		*/
		if (frameNum == 0)
		{
			buf_ptr = AP_Build_ARalink_Frame_Header(pAd, pTxBlk);
			
			/*
				It's ok write the TxWI here, because the TxWI->TxWIMPDUByteCnt
				will be updated after final frame was handled.
			*/
			write_tmac_info_Data(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);


			/* Insert LLC-SNAP encapsulation - 8 octets */
			EXTRA_LLCSNAP_ENCAP_FROM_PKT_OFFSET(pTxBlk->pSrcBufData - 2,
								pTxBlk->pExtraLlcSnapEncap);

			if (pTxBlk->pExtraLlcSnapEncap) {
				NdisMoveMemory(buf_ptr, pTxBlk->pExtraLlcSnapEncap, 6);
				buf_ptr += 6;

				/* get 2 octets (TypeofLen) */
				NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufData - 2, 2);
				buf_ptr += 2;
				pTxBlk->MpduHeaderLen += LENGTH_802_1_H;
			}
		}
		else
		{
			buf_ptr = &pTxBlk->HeaderBuf[0];
			pTxBlk->MpduHeaderLen = 0;
			
			/* 
				A-Ralink sub-sequent frame header is the same as 802.3 header.
					DA(6)+SA(6)+FrameType(2)
			*/
			NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufHeader, 12);
			buf_ptr += 12;
			/* get 2 octets (TypeofLen) */
			NdisMoveMemory(buf_ptr, pTxBlk->pSrcBufData - 2, 2);
			buf_ptr += 2;
			pTxBlk->MpduHeaderLen = ARALINK_SUBHEAD_LEN;
		}

#if defined(MT7603) || defined(MT7628)
		if (frameNum == 0)
			totalMPDUSize += pAd->chipCap.tx_hw_hdr_len - pTxBlk->hw_rsv_len + pTxBlk->MpduHeaderLen + pTxBlk->HdrPadLen + pTxBlk->SrcBufLen;
		else
			totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
#else
		totalMPDUSize += pTxBlk->MpduHeaderLen + pTxBlk->SrcBufLen;
#endif /* defined(MT7603) || defined(MT7628) */

		if (frameNum == 0)
			FirstTx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);
		else
			LastTxIdx = HAL_WriteMultiTxResource(pAd, pTxBlk, frameNum, &freeCnt);


#ifdef DBG_CTRL_SUPPORT
#ifdef INCLUDE_DEBUG_QUEUE
		if (pAd->CommonCfg.DebugFlags & DBF_DBQ_TXFRAME)
			dbQueueEnqueueTxFrame((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]), NULL);
#endif /* INCLUDE_DEBUG_QUEUE */
#endif /* DBG_CTRL_SUPPORT */

		frameNum++;
		
		pAd->RalinkCounters.OneSecTxARalinkCnt++;
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;

#ifdef STATS_COUNT_SUPPORT
		/* calculate Tx count and ByteCount per BSS */
		{
			BSS_STRUCT *pMbss = pTxBlk->pMbss;
			MAC_TABLE_ENTRY *pMacEntry=pTxBlk->pMacEntry;

#ifdef WAPI_SUPPORT	
			if (pTxBlk->pMacEntry && IS_ENTRY_CLIENT(pTxBlk->pMacEntry))
			{
			if (pTxBlk->pMacEntry->WapiUskRekeyTimerRunning && pAd->CommonCfg.wapi_usk_rekey_method == REKEY_METHOD_PKT)
				pTxBlk->pMacEntry->wapi_usk_rekey_cnt += totalMPDUSize;
			}
#endif /* WAPI_SUPPORT */

			if (pMbss != NULL)
			{
				pMbss->TransmittedByteCount += totalMPDUSize;
				pMbss->TxCount ++;
#ifdef STATS_COUNT_SUPPORT
				if(IS_MULTICAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->mcPktsTx++;
				else if(IS_BROADCAST_MAC_ADDR(pTxBlk->pSrcBufHeader))
					pMbss->bcPktsTx++;
				else
					pMbss->ucPktsTx++;
#endif /* STATS_COUNT_SUPPORT */
			}

			if(pMacEntry && pMacEntry->Sst == SST_ASSOC)
			{
				INC_COUNTER64(pMacEntry->TxPackets);
				pMacEntry->TxBytes+=pTxBlk->SrcBufLen;
				pMacEntry->OneSecTxBytes+=pTxBlk->SrcBufLen;
                pAd->TxTotalByteCnt += pTxBlk->SrcBufLen;
#ifdef DYNAMIC_WMM
            pAd->TxTotalByteCnt_Dyn[pTxBlk->QueIdx] += pTxBlk->SrcBufLen;
#endif /* DYNAMIC_WMM */
			}

		}

#ifdef WDS_SUPPORT
		if (pTxBlk->pMacEntry && IS_ENTRY_WDS(pTxBlk->pMacEntry))
		{
			INC_COUNTER64(pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
			pAd->WdsTab.WdsEntry[pTxBlk->pMacEntry->func_tb_idx].WdsCounter.TransmittedByteCount+= pTxBlk->SrcBufLen;
		}
#endif /* WDS_SUPPORT */

#endif /* STATS_COUNT_SUPPORT */
	}

	HAL_FinalWriteTxResource(pAd, pTxBlk, totalMPDUSize, FirstTx);
	HAL_LastTxIdx(pAd, pTxBlk->QueIdx, LastTxIdx);

	/*
		Kick out Tx
	*/
	HAL_KickOutTx(pAd, pTxBlk, pTxBlk->QueIdx);

}


#ifdef VHT_TXBF_SUPPORT
VOID AP_NDPA_Frame_Tx(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UCHAR *buf;
	VHT_NDPA_FRAME *vht_ndpa;
	struct wifi_dev *wdev;
	UINT frm_len, sta_cnt;
	SNDING_STA_INFO *sta_info;
	MAC_TABLE_ENTRY *pMacEntry;
	
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pTxBlk->pPacket);
	pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
	pMacEntry = pTxBlk->pMacEntry;

	if (pMacEntry) 
	{
		wdev = pMacEntry->wdev;

		if (MlmeAllocateMemory(pAd, &buf) != NDIS_STATUS_SUCCESS)
			return;

		NdisZeroMemory(buf, MGMT_DMA_BUFFER_SIZE);

		vht_ndpa = (VHT_NDPA_FRAME *)buf;
		frm_len = sizeof(VHT_NDPA_FRAME);
		vht_ndpa->fc.Type = FC_TYPE_CNTL;
		vht_ndpa->fc.SubType = SUBTYPE_VHT_NDPA;
		COPY_MAC_ADDR(vht_ndpa->ra, pMacEntry->Addr);
		COPY_MAC_ADDR(vht_ndpa->ta, wdev->if_addr);

		/* Currnetly we only support 1 STA for a VHT DNPA */
		sta_info = vht_ndpa->sta_info;
		for (sta_cnt = 0; sta_cnt < 1; sta_cnt++) {
			sta_info->aid12 = pMacEntry->Aid;
			sta_info->fb_type = SNDING_FB_SU;
			sta_info->nc_idx = 0;
			vht_ndpa->token.token_num = pMacEntry->snd_dialog_token;
			frm_len += sizeof(SNDING_STA_INFO);
			sta_info++;
			if (frm_len >= (MGMT_DMA_BUFFER_SIZE - sizeof(SNDING_STA_INFO))) {
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): len(%d) too large!cnt=%d\n",
							__FUNCTION__, frm_len, sta_cnt));
				break;
			}
		}
		if (pMacEntry->snd_dialog_token & 0xc0)
			pMacEntry->snd_dialog_token = 0;
		else
			pMacEntry->snd_dialog_token++;

		vht_ndpa->duration = 100;

		//DBGPRINT(RT_DEBUG_OFF, ("Send VHT NDPA Frame to STA(%02x:%02x:%02x:%02x:%02x:%02x)\n",
		//						PRINT_MAC(pMacEntry->Addr)));
		//hex_dump("VHT NDPA Frame", buf, frm_len);

		// NDPA's BW needs to sync with Tx BW
		pAd->CommonCfg.MlmeTransmit.field.BW = pMacEntry->HTPhyMode.field.BW;

		pTxBlk->Flags = FALSE; // No Acq Request
		
		// TODO: shiang-lock, fix ME!!
		MiniportMMRequest(pAd, 0, buf, frm_len);
		MlmeFreeMemory(pAd, buf);
	}

	pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
}
#endif /* VHT_TXBF_SUPPORT */


/*
	========================================================================
	Routine Description:
		Copy frame from waiting queue into relative ring buffer and set 
	appropriate ASIC register to kick hardware encryption before really
	sent out to air.

	Arguments:
		pAd 	   		Pointer to our adapter
		pTxBlk			Pointer to outgoing TxBlk structure.
		QueIdx			Queue index for processing

	Return Value:
		None
	========================================================================
*/
NDIS_STATUS APHardTransmit(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	PQUEUE_ENTRY pQEntry;
	PNDIS_PACKET pPacket = NULL;

    DBGPRINT(RT_DEBUG_INFO, ("%s(%d)-->\n", __FUNCTION__, __LINE__));

	if ((pAd->Dot11_H.RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		||(isCarrierDetectExist(pAd) == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
		)
	{
		while(pTxBlk->TxPacketList.Head)
		{
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef NEW_IXIA_METHOD
			if (IS_EXPECTED_LENGTH(RTPKT_TO_OSPKT(pPacket)->len))
				pAd->tr_ststic.tx[DROP_80211H_MODE]++;
#endif
        }
        DBGPRINT(RT_DEBUG_INFO, ("<--%s(%d)\n", __FUNCTION__, __LINE__));
		return NDIS_STATUS_FAILURE;
	}

	if (pTxBlk->wdev->bVLAN_Tag == TRUE)
	{
		RTMP_SET_PACKET_VLAN(pTxBlk->pPacket, FALSE);
	}

	/*add hook point when dequeue*/
	RTMP_OS_TXRXHOOK_CALL(WLAN_TX_DEQUEUE,pPacket,pTxBlk->QueIdx,pAd);

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT
{
	struct wifi_dev *wdev = pAd->wdev_list[pTxBlk->wdev_idx];
	
	if ((wdev->func_idx < pAd->ApCfg.BssidNum)
		&& IS_RRM_QUIET(pAd, wdev->func_idx))
	{
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}
}
#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

#ifdef HDR_TRANS_TX_SUPPORT
#ifdef SOFT_ENCRYPT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) /* need LLC, not yet generated */
		pTxBlk->NeedTrans = FALSE;
	else
#endif /* SOFT_ENCRYPT */
#ifdef WDS_SUPPORT
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry))
		pTxBlk->NeedTrans = FALSE;
	else
#endif /* WDS_SUPPORT */
	{
		pTxBlk->NeedTrans = TRUE;
	}
#endif /* HDR_TRANS_TX_SUPPORT */

#ifdef VHT_TXBF_SUPPORT			
	if ((pTxBlk->TxFrameType & TX_NDPA_FRAME) > 0)
	{
		UCHAR mlmeMCS, mlmeBW, mlmeMode;

		mlmeMCS  = pAd->CommonCfg.MlmeTransmit.field.MCS;
		mlmeBW   = pAd->CommonCfg.MlmeTransmit.field.BW;
		mlmeMode = pAd->CommonCfg.MlmeTransmit.field.MODE;
		
		pAd->NDPA_Request = TRUE;
	
		AP_NDPA_Frame_Tx(pAd, pTxBlk);
		
		pAd->NDPA_Request = FALSE;
		pTxBlk->TxFrameType &= ~TX_NDPA_FRAME;

		// Finish NDPA and then recover to mlme's own setting
		pAd->CommonCfg.MlmeTransmit.field.MCS  = mlmeMCS;
		pAd->CommonCfg.MlmeTransmit.field.BW   = mlmeBW;
		pAd->CommonCfg.MlmeTransmit.field.MODE = mlmeMode;
	}
#endif	

#ifdef FAST_DETECT_STA_OFF
	if ((Flag_fast_detect_sta_off == 1)&&(pTxBlk->pMacEntry->detect_deauth == TRUE)) {
		while(pTxBlk->TxPacketList.Head) {
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}
		return NDIS_STATUS_FAILURE;
	}
#endif

	switch (pTxBlk->TxFrameType)
	{
#ifdef DOT11_N_SUPPORT
		case TX_AMPDU_FRAME:
#ifdef HDR_TRANS_TX_SUPPORT
				if (pTxBlk->NeedTrans)
					AP_AMPDU_Frame_Tx_Hdr_Trns(pAd, pTxBlk);
				else
#endif /* HDR_TRANS_TX_SUPPORT */
				AP_AMPDU_Frame_Tx(pAd, pTxBlk);
				break;
#endif /* DOT11_N_SUPPORT */
		case TX_LEGACY_FRAME:
#ifdef HDR_TRANS_TX_SUPPORT
				if (pTxBlk->NeedTrans)
					AP_Legacy_Frame_Tx_Hdr_Trns(pAd, pTxBlk);
				else
#endif /* HDR_TRANS_TX_SUPPORT */
					AP_Legacy_Frame_Tx(pAd, pTxBlk);
				break;
		case TX_MCAST_FRAME:
#ifdef HDR_TRANS_TX_SUPPORT
				pTxBlk->NeedTrans = FALSE;
#endif /* HDR_TRANS_TX_SUPPORT */
				AP_Legacy_Frame_Tx(pAd, pTxBlk);
				break;
#ifdef DOT11_N_SUPPORT
		case TX_AMSDU_FRAME:
				AP_AMSDU_Frame_Tx(pAd, pTxBlk);
				break;
#endif /* DOT11_N_SUPPORT */
		case TX_RALINK_FRAME:
				AP_ARalink_Frame_Tx(pAd, pTxBlk);
				break;
		case TX_FRAG_FRAME:
				AP_Fragment_Frame_Tx(pAd, pTxBlk);
				break;
		default:
			{
				/* It should not happened! */
				DBGPRINT(RT_DEBUG_ERROR, ("Send a pacekt was not classified!!\n"));
				while (pTxBlk->TxPacketList.Head) {
					pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
					pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);
					if (pPacket)
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
				}
			}
			break;
	}

    DBGPRINT(RT_DEBUG_INFO, ("<--%s(%d)\n", __FUNCTION__, __LINE__));

	return (NDIS_STATUS_SUCCESS);
	
}


/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN APChkCls2Cls3Err(RTMP_ADAPTER *pAd, UCHAR wcid, HEADER_802_11 *hdr)
{
	/* software MAC table might be smaller than ASIC on-chip total size. */
	/* If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006 */
	if (wcid >= MAX_LEN_OF_MAC_TABLE)
	{
		MAC_TABLE_ENTRY *pEntry;
		
		DBGPRINT(RT_DEBUG_WARN, ("%s():Rx a frame from %02x:%02x:%02x:%02x:%02x:%02x with WCID(%d) > %d\n",
					__FUNCTION__, PRINT_MAC(hdr->Addr2), 
					wcid, MAX_LEN_OF_MAC_TABLE));
//+++Add by shiang for debug
		pEntry = MacTableLookup(pAd, hdr->Addr2);
		if (pEntry)
			return FALSE;
//---Add by shiang for debug

		APCls2errAction(pAd, MAX_LEN_OF_MAC_TABLE, hdr);
		return TRUE;
	}

	if (pAd->MacTab.Content[wcid].Sst == SST_ASSOC)
		; /* okay to receive this DATA frame */
	else if (pAd->MacTab.Content[wcid].Sst == SST_AUTH)
	{
		APCls3errAction(pAd, wcid, hdr);
		return TRUE; 
	}
	else
	{
		APCls2errAction(pAd, wcid, hdr);
		return TRUE; 
	}
	return FALSE;
}


/*
	detect AC Category of trasmitting packets
	to turn AC0(BE) TX_OP (MAC reg 0x1300)
*/
// TODO: shiang-usw, this function should move to other place!!
VOID detect_wmm_traffic(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR UserPriority,
	IN UCHAR FlgIsOutput)
{
	/* For BE & BK case and TxBurst function is disabled */
	if ((pAd->CommonCfg.bEnableTxBurst == FALSE) 
#ifdef DOT11_N_SUPPORT
		&& (pAd->CommonCfg.bRdg == FALSE)
		&& (pAd->CommonCfg.bRalinkBurstMode == FALSE)
#endif /* DOT11_N_SUPPORT */
		&& (FlgIsOutput == 1)
	)
	{
		if (WMM_UP2AC_MAP[UserPriority] == QID_AC_BK)
		{
			/* has any BK traffic */
			if (pAd->flg_be_adjust == 0)
			{
				/* yet adjust */
#ifdef RTMP_MAC_PCI
				AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x20);
#endif /* RTMP_MAC_PCI */
				pAd->flg_be_adjust = 1;
				NdisGetSystemUpTime(&pAd->be_adjust_last_time);

				DBGPRINT(RT_DEBUG_TRACE, ("wmm> adjust be!\n"));
			}
		}
		else
		{
			if (pAd->flg_be_adjust != 0)
			{
				QUEUE_HEADER *pQueue;

				/* has adjusted */
				pQueue = &pAd->TxSwQueue[QID_AC_BK];

				if ((pQueue == NULL) ||
					((pQueue != NULL) && (pQueue->Head == NULL)))
				{
					ULONG	now;
					NdisGetSystemUpTime(&now);
					if ((now - pAd->be_adjust_last_time) > TIME_ONE_SECOND)
					{
						/* no any BK traffic */
#ifdef RTMP_MAC_PCI
						// TODO: shiang-7603
						AsicSetWmmParam(pAd, WMM_PARAM_AC_0, WMM_PARAM_TXOP, 0x0);
#endif /* RTMP_MAC_PCI */
						pAd->flg_be_adjust = 0;

						DBGPRINT(RT_DEBUG_TRACE, ("wmm> recover be!\n"));
					}
				}
				else
					NdisGetSystemUpTime(&pAd->be_adjust_last_time);
			}
		}
	}

	/* count packets which priority is more than BE */
	if (UserPriority > 3)
	{
		pAd->OneSecondnonBEpackets++;
		if (pAd->OneSecondnonBEpackets > 100 
#ifdef DOT11_N_SUPPORT
			&& pAd->MacTab.fAnyStationMIMOPSDynamic
#endif /* DOT11_N_SUPPORT */
		)
		{
			if (!pAd->is_on)
			{
#ifdef RTMP_MAC_PCI
				// TODO: shiang-7603
				RTMP_IO_WRITE32(pAd,  EXP_ACK_TIME, 0x005400ca );
#endif /* RTMP_MAC_PCI */
				pAd->is_on = 1;
			}
		}
		else
		{
			if (pAd->is_on)
			{
#ifdef RTMP_MAC_PCI
				// TODO: shiang-7603
				RTMP_IO_WRITE32(pAd,  EXP_ACK_TIME, 0x002400ca );
#endif /* RTMP_MAC_PCI */
				pAd->is_on = 0;
			}
		}
	}
}


VOID APRxErrorHandle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

	if (pRxInfo->CipherErr)
		INC_COUNTER64(pAd->WlanCounters.WEPUndecryptableCount);

	if (pRxInfo->U2M && pRxInfo->CipherErr)
	{		
		if (pRxBlk->wcid < MAX_LEN_OF_MAC_TABLE)
		{
#ifdef APCLI_SUPPORT
#endif /* APCLI_SUPPORT */
			{
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				/*
					MIC error
					Before verifying the MIC, the receiver shall check FCS, ICV and TSC.
					This avoids unnecessary MIC failure events.
				*/
				if ((pEntry->WepStatus == Ndis802_11TKIPEnable)
					&& (pRxInfo->CipherErr == 2))
				{
#ifdef HOSTAPD_SUPPORT
					if(pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT)
						ieee80211_notify_michael_failure(pAd, pRxBlk->pHeader, (UINT32)pRxBlk->key_idx, 0);
			      		else
#endif/*HOSTAPD_SUPPORT*/
		      			{
		      				RTMP_HANDLE_COUNTER_MEASURE(pAd, pEntry);
		      			}
				}

				/* send wireless event - for icv error */
				if ((pRxInfo->CipherErr & 1) == 1)
					RTMPSendWirelessEvent(pAd, IW_ICV_ERROR_EVENT_FLAG, pEntry->Addr, 0, 0); 
#ifdef WIFI_DIAG
					/* WEP + open, wrong passowrd can association success, but rx data error */
					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->WepStatus == Ndis802_11WEPEnabled))
						DiagConnError(pAd, pEntry->func_tb_idx, pEntry->Addr,
							DIAG_CONN_AUTH_FAIL, REASON_MIC_FAILURE);
#endif
#ifdef CONN_FAIL_EVENT
					if (IS_ENTRY_CLIENT(pEntry) && (pEntry->WepStatus == Ndis802_11WEPEnabled))
						ApSendConnFailMsg(pAd,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Ssid,
							pAd->ApCfg.MBSSID[pEntry->func_tb_idx].SsidLen,
							pEntry->Addr,
							REASON_MIC_FAILURE);
#endif
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n", 
					pRxBlk->MPDUtotalByteCnt, pRxBlk->wcid, pRxInfo->CipherErr));

	}
}

#ifdef RLT_MAC_DBG
static int dump_next_valid = 0;
#endif
BOOLEAN APCheckVaildDataFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	BOOLEAN isVaild = FALSE;

	do
	{
		if (pHeader->FC.ToDs == 0)
			break;

#ifdef IDS_SUPPORT
		if ((pHeader->FC.FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) /* not in RX WCID MAC table */
		{
			if (++pAd->ApCfg.RcvdMaliciousDataCount > pAd->ApCfg.DataFloodThreshold)
				break;
		}
#endif /* IDS_SUPPORT */
	
		/* check if Class2 or 3 error */
		if ((pHeader->FC.FrDs == 0) && (APChkCls2Cls3Err(pAd, pRxBlk->wcid, pHeader))) 
			break;

//+++Add by shiang for debug
#ifdef RLT_MAC_DBG
		if (pAd->chipCap.hif_type == HIF_RLT) {
			if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE) {
				MAC_TABLE_ENTRY *pEntry = NULL;

				DBGPRINT(RT_DEBUG_WARN, ("ErrWcidPkt: seq=%d\n", pHeader->Sequence));
				pEntry = MacTableLookup(pAd, pHeader->Addr2);
				if (pEntry && (pEntry->Sst == SST_ASSOC) && IS_ENTRY_CLIENT(pEntry))
					pRxBlk->wcid = pEntry->wcid;

				dump_next_valid = 1;
			}
			else if (dump_next_valid)
			{
				DBGPRINT(RT_DEBUG_WARN, ("NextValidWcidPkt: seq=%d\n", pHeader->Sequence));
				dump_next_valid = 0;
			}
		}
#endif /* RLT_MAC_DBG */
//---Add by shiang for debug

		if(pAd->ApCfg.BANClass3Data == TRUE)
			break;

		isVaild = TRUE;
	} while (0);

	return isVaild;
}


INT ap_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(IDS_SUPPORT)\
	|| defined(A4_CONN) || defined(WH_EVENT_NOTIFIER)
	FRAME_CONTROL *pFmeCtrl = &pHeader->FC;
#endif
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = 0;

	pEntry = PACInquiry(pAd, pRxBlk->wcid);

#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(A4_CONN)
	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1))
	{
#ifdef CLIENT_WDS
		if (pEntry) {
			/* The CLIENT WDS must be a associated STA */
			if (IS_ENTRY_CLIWDS(pEntry))
				;
			else if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
				SET_ENTRY_CLIWDS(pEntry);
			else
				return FALSE;

			CliWds_ProxyTabUpdate(pAd, pEntry->Aid, pHeader->Octet);
		}
#endif /* CLIENT_WDS */
#ifdef A4_CONN
		if(!pEntry)
			pEntry = MacTableLookup(pAd, pHeader->Addr2);
		if (pEntry && IS_ENTRY_A4(pEntry))
        {
			MAC_TABLE_ENTRY *pMovedEntry = NULL;
            UINT16 ProtoType = 0;
            UINT32 ARPSenderIP = 0;
			UINT8 ARPSenderIPSkipLen=0;
			UCHAR *Pos = NULL;
			BOOLEAN bTAMatchSA = FALSE;
			/*this is a 4-addr pkt*/
			ARPSenderIPSkipLen += sizeof(*pHeader)+6;
			if (pFmeCtrl->SubType & 0x08) {
				ARPSenderIPSkipLen += 2;
			}
			if (pFmeCtrl->Order  
#ifdef AGGREGATION_SUPPORT
				 && !(pRxBlk->rx_rate.field.MODE <= MODE_OFDM &&
				 		 CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE))
#endif				  
				) {	
					ARPSenderIPSkipLen += 4;
			}
			if (pRxInfo->L2PAD) {
				ARPSenderIPSkipLen += 2;
			}
			if (NdisEqualMemory(SNAP_802_1H, pRxBlk->pData + ARPSenderIPSkipLen, 6) ||
				NdisEqualMemory(SNAP_BRIDGE_TUNNEL, pRxBlk->pData + ARPSenderIPSkipLen, 6)) {
				ARPSenderIPSkipLen += 6;
			}
            
            Pos = (pRxBlk->pData + ARPSenderIPSkipLen);
            bTAMatchSA = MAC_ADDR_EQUAL(pEntry->Addr, pHeader->Octet);
			
			hex_dump("RA",pHeader->Addr1,6);
			hex_dump("TA",pHeader->Addr2,6);
			hex_dump("DA",pHeader->Addr3,6);
			hex_dump("SA",pHeader->Octet,6);
            ProtoType = OS_NTOHS(*((UINT16*)Pos));
            if(ProtoType == 0x0806) /* ETH_P_ARP */
                NdisCopyMemory(&ARPSenderIP, (Pos + 16), 4);

            /* 
                           It means this source entry has moved to another one and hidden behind it. 
                           So delete this source entry! 
                    */
            if(!bTAMatchSA) /* TA isn't same with SA case*/
            {
                pMovedEntry = MacTableLookup(pAd, pHeader->Octet);
                if(pMovedEntry
#ifdef AIR_MONITOR
                    && !IS_ENTRY_MONITOR(pMovedEntry)
#endif /* AIR_MONITOR */
                    && IS_ENTRY_CLIENT(pMovedEntry)
                    )
                {
                     MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("APRxPktAllow: AP found a entry(%02X:%02X:%02X:%02X:%02X:%02X) who has moved to another side! Delete it from MAC table.\n",
                        PRINT_MAC(pMovedEntry->Addr)));
                     
#ifdef WH_EVENT_NOTIFIER
			        {
			            EventHdlr pEventHdlrHook = NULL;
			            pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_STA_LEAVE);
			            if(pEventHdlrHook && pMovedEntry->wdev)
			                pEventHdlrHook(pAd, pMovedEntry->wdev, pMovedEntry->Addr, pAd->CommonCfg.Channel);
			        }
#endif /* WH_EVENT_NOTIFIER */
                    MacTableDeleteEntry(pAd, pMovedEntry->wcid, pMovedEntry->Addr);
                }
            }
			a4_proxy_update(pAd, pEntry->func_tb_idx, pEntry->wcid, pHeader->Octet, ARPSenderIP);
		} else {
				pEntry = NULL;
		}
#endif /* A4_CONN */

#ifdef WDS_SUPPORT
		if (!pEntry)
		{
			/*
				The WDS frame only can go here when in auto learning mode and
				this is the first trigger frame from peer

				So we check if this is un-registered WDS entry by call function
					"FindWdsEntry()"
			*/
			if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
				pEntry = FindWdsEntry(pAd, pRxBlk->wcid, pHeader->Addr2, pRxBlk->rx_rate.field.MODE);

			/* have no valid wds entry exist, then discard the incoming packet.*/
			if (!(pEntry && WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)))
				return FALSE;

			/*receive corresponding WDS packet, disable TX lock state (fix WDS jam issue) */
			if(pEntry && (pEntry->LockEntryTx == TRUE)) 
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Receive WDS packet, disable TX lock state!\n"));
				pEntry->ContinueTxFailCnt = 0;
				pEntry->LockEntryTx = FALSE;
				// TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry!
				pAd->MacTab.tr_entry[pEntry->wcid].ContinueTxFailCnt = 0;
				pAd->MacTab.tr_entry[pEntry->wcid].LockEntryTx = FALSE;
			}
		}
#endif /* WDS_SUPPORT */

		if (pEntry)
		{
#ifdef STATS_COUNT_SUPPORT
#ifdef WDS_SUPPORT
			RT_802_11_WDS_ENTRY *pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

			pWdsEntry->WdsCounter.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			INC_COUNTER64(pWdsEntry->WdsCounter.ReceivedFragmentCount);

			if(IS_MULTICAST_MAC_ADDR(pHeader->Addr3))
				INC_COUNTER64(pWdsEntry->WdsCounter.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */
#endif
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			return hdr_len;
		}

		return FALSE;
	}
#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) */

	if (!pEntry) {
#ifdef IDS_SUPPORT
		if ((pHeader->FC.FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) /* not in RX WCID MAC table */
			pAd->ApCfg.RcvdMaliciousDataCount++;
#endif /* IDS_SUPPORT */
		
		return FALSE;
	}

	if (!((pHeader->FC.FrDs == 0) && (pHeader->FC.ToDs == 1))) {
#ifdef IDS_SUPPORT
		/*
			Replay attack detection, 
			drop it if detect a spoofed data frame from a rogue AP
		*/
		if (pFmeCtrl->FrDs == 1)
			RTMPReplayAttackDetection(pAd, pHeader->Addr2, pRxBlk);
#endif /* IDS_SUPPORT */

		return FALSE;
	}
#ifdef A4_CONN
	if (((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
		if((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL)) {
#ifdef MWDS
		if (pEntry && GET_ENTRY_A4(pEntry) == A4_TYPE_MWDS) {
			return FALSE;
		}
#endif
		}
	}
#endif /* A4_CONN */

	/* check if Class2 or 3 error */
	if (APChkCls2Cls3Err(pAd, pRxBlk->wcid, pHeader))
		return FALSE;

//+++Add by shiang for debug
#ifdef RLT_MAC_DBG
	if (pAd->chipCap.hif_type == HIF_RLT) {
		if (pRxBlk->wcid >= MAX_LEN_OF_MAC_TABLE) {
			MAC_TABLE_ENTRY *pEntry = NULL;

			DBGPRINT(RT_DEBUG_WARN, ("ErrWcidPkt: seq=%d\n", pHeader->Sequence));
			pEntry = MacTableLookup(pAd, pHeader->Addr2);
			if (pEntry && (pEntry->Sst == SST_ASSOC) && IS_ENTRY_CLIENT(pEntry))
				pRxBlk->wcid = pEntry->wcid;

			dump_next_valid = 1;
		}
		else if (dump_next_valid)
		{
			DBGPRINT(RT_DEBUG_WARN, ("NextValidWcidPkt: seq=%d\n", pHeader->Sequence));
			dump_next_valid = 0;
		}
	}
#endif /* RLT_MAC_DBG */
//---Add by shiang for debug

	if(pAd->ApCfg.BANClass3Data == TRUE)
		return FALSE;

#ifdef STATS_COUNT_SUPPORT
	/* Increase received byte counter per BSS */
	if (pHeader->FC.FrDs == 0 && pRxInfo->U2M)
	{
		BSS_STRUCT *pMbss = pEntry->pMbss;
		if (pMbss != NULL)
		{
			pMbss->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			pMbss->RxCount ++;
		}
	}

	/* update multicast counter */
        if (IS_MULTICAST_MAC_ADDR(pHeader->Addr3))
                INC_COUNTER64(pAd->WlanCounters.MulticastReceivedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

#ifdef WH_EVENT_NOTIFIER
	if(pEntry && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
		&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
	   && ((pFmeCtrl->SubType != SUBTYPE_DATA_NULL) && (pFmeCtrl->SubType != SUBTYPE_QOS_NULL))
		)
		pEntry->rx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */  

	hdr_len = LENGTH_802_11;
	RX_BLK_SET_FLAG(pRxBlk, fRX_STA);

	ASSERT(pEntry->Aid == pRxBlk->wcid);

	return hdr_len;

}


INT ap_rx_ps_handle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{

	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	UCHAR OldPwrMgmt = PWR_ACTIVE; /* 1: PWR_SAVE, 0: PWR_ACTIVE */
	
#ifdef UAPSD_SUPPORT
	UCHAR *pData = pRxBlk->pData;
#endif /* UAPSD_SUPPORT */
   	/* 1: PWR_SAVE, 0: PWR_ACTIVE */

   	OldPwrMgmt = RtmpPsIndicate(pAd, pHeader->Addr2, pEntry->wcid, pHeader->FC.PwrMgmt);

	if(OldPwrMgmt > 2)
		DBGPRINT(RT_DEBUG_TRACE, ("OldPwrMgmt is %d\n", OldPwrMgmt));
	
#ifdef UAPSD_SUPPORT

	if ((pHeader->FC.PwrMgmt == PWR_SAVE) && 
		(OldPwrMgmt == PWR_SAVE) &&
		(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
		(pHeader->FC.SubType & 0x08))
	   	{
			/*
				In IEEE802.11e, 11.2.1.4 Power management with APSD,
				If there is no unscheduled SP in progress, the unscheduled SP begins
				when the QAP receives a trigger frame from a non-AP QSTA, which is a
				QoS data or QoS Null frame associated with an AC the STA has
				configured to be trigger-enabled.

				In WMM v1.1, A QoS Data or QoS Null frame that indicates transition
				to/from Power Save Mode is not considered to be a Trigger Frame and
				the AP shall not respond with a QoS Null frame.
			*/
			/* Trigger frame must be QoS data or QoS Null frame */
	   		UCHAR  OldUP;

			// TODO: shiang-usw, check this!!!
#ifdef HDR_TRANS_SUPPORT
			if (pRxBlk->bHdrRxTrans) {
				// TODO: different chip has different position @20130129. (+32 is for MT7650)
				OldUP = (*(pData+32) & 0x07);
			}
			else
#endif /* HDR_TRANS_SUPPORT */
				OldUP = (*(pData+LENGTH_802_11) & 0x07);
#ifdef MT_PS
		if (pEntry->i_psm == I_PSM_DISABLE)
        {
			MT_SET_IGNORE_PSM(pAd, pEntry, I_PSM_ENABLE);
        }
#endif /* MT_PS */
		UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
    }
#endif /* UAPSD_SUPPORT */

	return TRUE;
}


INT ap_rx_foward_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN to_os, to_air;
	UCHAR *pHeader802_3;
	PNDIS_PACKET pForwardPacket;
	BSS_STRUCT *pMbss;
	struct wifi_dev *dst_wdev = NULL;
#ifdef A4_CONN
	UCHAR wcid;
	UCHAR *pSrcAddr = NULL;
	UCHAR *pSrcBufVA = NULL;
#endif /* A4_CONN */

	if ((wdev->func_idx >= MAX_MBSSID_NUM(pAd)) ||
		(wdev->func_idx >= HW_BEACON_MAX_NUM))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Invalid func_idx(%d), type(%d)!\n",
					__FUNCTION__, wdev->func_idx, wdev->wdev_type));
		return FALSE;
	}

	/* only one connected sta, directly to upper layer */
	if (pAd->MacTab.Size <= 1)
		return TRUE;

	// TODO: shiang-usw, remove pMbss structure here to make it more generic!
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);

	/* by default, announce this pkt to upper layer (bridge) and not to air */
	to_os = TRUE;
	to_air = FALSE;

	if (pHeader802_3[0] & 0x01) 
	{
		if ((pMbss->StaCount > 1)
		) {
			/* forward the M/Bcast packet back to air if connected STA > 1 */
			to_air = TRUE;
		}
	}
	else
	{
		/* if destinated STA is a associated wireless STA */
		pEntry = MacTableLookup(pAd, pHeader802_3);
		if (pEntry && pEntry->Sst == SST_ASSOC && pEntry->wdev)
		{
			dst_wdev = pEntry->wdev;
			if (wdev == dst_wdev)
			{
				/*
					STAs in same SSID, default send to air and not to os,
					but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
				*/
				to_air = TRUE;
				to_os = FALSE;
				if (pMbss->IsolateInterStaTraffic == 1)
					to_air = FALSE;
			}
			else
			{
				/* 
					STAs in different SSID, default send to os and not to air
					but not to os if match any of following cases:
						a). destination VLAN ID != source VLAN ID
						b). pAd->ApCfg.IsolateInterStaTrafficBTNBSSID
				*/
				to_os = TRUE;
				to_air = FALSE;
				if (pAd->ApCfg.IsolateInterStaTrafficBTNBSSID == 1 ||
					(wdev->VLAN_VID != dst_wdev->VLAN_VID))
					to_os = FALSE;
			}
#ifdef WH_EVENT_NOTIFIER
			if(to_air && IS_ENTRY_CLIENT(pEntry)
#ifdef A4_CONN
				&& !IS_ENTRY_A4(pEntry)
#endif /* A4_CONN */
			   )
				 pEntry->tx_state.PacketCount++;
#endif /* WH_EVENT_NOTIFIER */			
		}
#ifdef A4_CONN
		else if (!pEntry && a4_proxy_lookup(pAd, wdev->func_idx, pHeader802_3, FALSE, TRUE, &wcid))
		{
			if (VALID_WCID(wcid))
				pEntry = &pAd->MacTab.Content[wcid];

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				to_os = FALSE;
				to_air = TRUE;
				dst_wdev = pEntry->wdev;
			}
		}
#endif /* A4_CONN */
	}

#ifdef PREVENT_ARP_SPOOFING
	if (pAd->ApCfg.ARPSpoofChk)
	{
		/* 
		    This is on AP Rx data Path :
		    sofar both direction will be checked, maybe we can check to_air case only
		*/
		if (to_os || to_air) 
		{
			if (check_arp_spoofing(wdev->if_dev, pPacket))
			{
				DBGPRINT(RT_DEBUG_ERROR, ("%s(): DROP: Detected ARP spoofing (func_idx=%d,wdev_type=%d)!\n",
							__FUNCTION__, wdev->func_idx, wdev->wdev_type));
				to_os = FALSE;
				return to_os; //Free pPacket out side
			}
		}
	}
#endif /* PREVENT_ARP_SPOOFING */

	if (to_air)
	{
#ifdef INF_AMAZON_SE
		/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
		RTMP_SET_PACKET_NOBULKOUT(pPacket, FALSE);
#endif /* INF_AMAZON_SE */

		pForwardPacket = DuplicatePacket(wdev->if_dev, pPacket);
		if (pForwardPacket == NULL)
			return to_os;

		/* 1.1 apidx != 0, then we need set packet mbssid attribute. */
		if (pEntry) {
			RTMP_SET_PACKET_WDEV(pForwardPacket, dst_wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, pEntry->wcid);
		}
		else /* send bc/mc frame back to the same bss */
		{
			RTMP_SET_PACKET_WDEV(pForwardPacket, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wdev->tr_tb_idx);
		}
		
		RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);

#ifdef INF_AMAZON_SE
		/*Iverson patch for WMM A5-T07 ,WirelessStaToWirelessSta do not bulk out aggregate */
		RTMP_SET_PACKET_NOBULKOUT(pForwardPacket, TRUE);
#endif /* INF_AMAZON_SE */

#ifndef A4_CONN
		APSendPacket(pAd, pForwardPacket);
#else
		/* send bc/mc frame back to the same bss */
		pSrcBufVA = pHeader802_3;
		pSrcAddr = pHeader802_3 + MAC_ADDR_LEN;

		if (pSrcBufVA && IS_BROADCAST_MAC_ADDR(pSrcBufVA)) {
			a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, pSrcAddr);
		} else if (pSrcBufVA && IS_MULTICAST_MAC_ADDR(pSrcBufVA)
#ifdef IGMP_SNOOP_SUPPORT
			&& (!pAd->ApCfg.IgmpSnoopEnable)
#endif
			)
		{
			a4_send_clone_pkt(pAd, wdev->func_idx, pPacket, pSrcAddr);
		} else {
			DBGPRINT(RT_DEBUG_TRACE, ("This packet is a uc or IGMP is on, no need MWDS CLONE\n"));
		}

		APSendPacket(pAd, pForwardPacket);
#endif /* A4_CONN */

		RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);
	}
	
	return to_os;
}


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
NDIS_STATUS APInsertPsQueue(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN STA_TR_ENTRY *tr_entry,
	IN UCHAR QueIdx)
{
	return NDIS_STATUS_SUCCESS;
}

