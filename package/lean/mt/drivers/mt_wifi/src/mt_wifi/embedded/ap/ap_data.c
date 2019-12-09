/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include "rt_config.h"

static VOID ap_tx_drop_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, TX_BLK *txblk)
{
#ifdef STATS_COUNT_SUPPORT
	BSS_STRUCT *mbss = txblk->pMbss;

	if (mbss != NULL)
		mbss->TxDropCount++;

#endif /* STATS_COUNT_SUPPORT */
}

static VOID ap_tx_ok_update(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, TX_BLK *txblk)
{
#ifdef STATS_COUNT_SUPPORT
	MAC_TABLE_ENTRY *entry = txblk->pMacEntry;

	if (!entry)
		return ;
#ifdef WHNAT_SUPPORT
	/*if WHNAT enable, query from CR4 and then update it*/
	if ((ad->CommonCfg.whnat_en) && (IS_ASIC_CAP(ad, fASIC_CAP_MCU_OFFLOAD)))
		return;
#endif /*WHNAT_SUPPORT*/

	/* calculate Tx count and ByteCount per BSS */
	{
		BSS_STRUCT *mbss = txblk->pMbss;

		if (mbss != NULL) {
			mbss->TransmittedByteCount += txblk->SrcBufLen;
			mbss->TxCount++;

			if (IS_MULTICAST_MAC_ADDR(txblk->pSrcBufHeader))
				mbss->mcPktsTx++;
			else if (IS_BROADCAST_MAC_ADDR(txblk->pSrcBufHeader))
				mbss->bcPktsTx++;
			else
				mbss->ucPktsTx++;
		}

		if (entry->Sst == SST_ASSOC) {
			INC_COUNTER64(entry->TxPackets);
			entry->TxBytes += txblk->SrcBufLen;
			ad->TxTotalByteCnt += txblk->SrcBufLen;
		}
	}

#ifdef APCLI_SUPPORT
	if (IS_ENTRY_APCLI(entry)) {
		struct _APCLI_STRUCT *apcli = txblk->pApCliEntry;

		if (apcli != NULL) {
			apcli->TxCount++;
			apcli->TransmittedByteCount += txblk->SrcBufLen;
		}
	}
#endif

#ifdef WDS_SUPPORT

	if (IS_ENTRY_WDS(entry)) {
		INC_COUNTER64(ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedFragmentCount);
		ad->WdsTab.WdsEntry[entry->func_tb_idx].WdsCounter.TransmittedByteCount += txblk->SrcBufLen;
	}

#endif /* WDS_SUPPORT */
#endif /* STATS_COUNT_SUPPORT */

}

INT ap_fp_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	PACKET_INFO pkt_info;
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UCHAR wcid = RESERVED_WCID;
	UCHAR frag_nums;

	RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);

	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pkt))
			return FALSE;
	}

#endif /* CONFIG_WIFI_PKT_FWD */

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return FALSE;
		wcid = wdev->tr_tb_idx;
	} else {
		entry = MacTableLookup(pAd, pkt_va);

		if (entry && (entry->Sst == SST_ASSOC)){
			wcid = (UCHAR)entry->wcid;
		}

	}

	if (!(VALID_TR_WCID(wcid) && IS_VALID_ENTRY(&pAd->MacTab.tr_entry[wcid])))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef CONFIG_AP_SUPPORT
#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */
#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	/*  ethertype check is not offload to mcu for fragment frame*/
	if (frag_nums > 1) {
		if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev)) {
			return FALSE;
		}
	}

	return TRUE;
}

INT ap_tx_pkt_allowed(
	RTMP_ADAPTER *pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt)
{
	PACKET_INFO pkt_info;
	UCHAR *pkt_va;
	UINT pkt_len;
	MAC_TABLE_ENTRY *entry = NULL;
	UCHAR wcid = RESERVED_WCID;
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR frag_nums;

	RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);

	if ((!pkt_va) || (pkt_len <= 14))
		return FALSE;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

	if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE)) {
		if (is_looping_packet(pAd, pkt))
			return FALSE;
	}

#endif /* CONFIG_WIFI_PKT_FWD */

	if (MAC_ADDR_IS_GROUP(pkt_va)) {
		if (wdev->PortSecured != WPA_802_1X_PORT_SECURED)
			return FALSE;
		wcid = wdev->tr_tb_idx;
	} else {
		entry = MacTableLookup(pAd, pkt_va);

		if (entry && (entry->Sst == SST_ASSOC)){
			wcid = (UCHAR)entry->wcid;
		}
	}

	tr_entry = &pAd->MacTab.tr_entry[wcid];

	if (!(VALID_TR_WCID(wcid) && IS_VALID_ENTRY(tr_entry)))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
#ifdef CONFIG_HOTSPOT

	/* Drop broadcast/multicast packet if disable dgaf */
	if (IS_ENTRY_CLIENT(tr_entry)) {
		BSS_STRUCT *pMbss = (BSS_STRUCT *)wdev->func_dev;

		if ((wcid == wdev->bss_info_argument.ucBcMcWlanIdx) &&
			(pMbss->HotSpotCtrl.HotSpotEnable || pMbss->HotSpotCtrl.bASANEnable) &&
			pMbss->HotSpotCtrl.DGAFDisable) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Drop broadcast/multicast packet when dgaf disable\n"));
			return FALSE;
		}
	}

#endif
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	if (!RTMPCheckEtherType(pAd, pkt, tr_entry, wdev)) {
		return FALSE;
	}

	if (tr_entry->PortSecured == WPA_802_1X_PORT_NOT_SECURED) {
		if (!((IS_AKM_WPA_CAPABILITY_Entry(wdev)
#ifdef DOT1X_SUPPORT
			   || (IS_IEEE8021X_Entry(wdev))
#endif /* DOT1X_SUPPORT */
			  ) && ((RTMP_GET_PACKET_EAPOL(pkt) ||
					 RTMP_GET_PACKET_WAI(pkt))))
		   )
			return FALSE;
	}

#ifdef CFG80211_SUPPORT

	/* CFG_TODO: POS NO GOOD */
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		RTMP_SET_PACKET_OPMODE(pkt, OPMODE_AP);

#endif /* CFG80211_SUPPORT */
	return TRUE;
}

UCHAR ap_mlme_search_wcid(RTMP_ADAPTER *pAd, UCHAR *addr1, UCHAR *addr2, PNDIS_PACKET pkt)
{
	MAC_TABLE_ENTRY *mac_entry = MacTableLookup(pAd, addr1);

#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif

#ifdef MAC_REPEATER_SUPPORT
	if ((mac_entry != NULL) && (IS_ENTRY_APCLI(mac_entry) || IS_ENTRY_REPEATER(mac_entry))) {
		rept_entry = lookup_rept_entry(pAd, addr2);

		if (rept_entry) { /*repeater case*/
			if ((rept_entry->CliEnable == TRUE) && (rept_entry->CliValid == TRUE))
				mac_entry = &pAd->MacTab.Content[rept_entry->MacTabWCID];
		} else { /*apcli case*/
			UCHAR apcli_wcid = 0;

			if (mac_entry->wdev && (mac_entry->wdev->func_idx < pAd->ApCfg.ApCliNum))
				apcli_wcid = pAd->ApCfg.ApCliTab[mac_entry->wdev->func_idx].MacTabWCID;
			else   /* use default apcli0 */
				apcli_wcid = pAd->ApCfg.ApCliTab[0].MacTabWCID;

			mac_entry = &pAd->MacTab.Content[apcli_wcid];
		}
	}

#endif
	if (mac_entry)
		return mac_entry->wcid;
	else 
		return 0;
}

INT ap_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue)
{
	HEADER_802_11 *pHeader_802_11;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PACKET_INFO PacketInfo;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen;
	INT ret;
	struct qm_ops *ops = pAd->qm_ops;

	RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
	RTMP_SET_PACKET_MGMT_PKT(pkt, 1);
	RTMP_QueryPacketInfo(pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);

	if (pSrcBufVA == NULL) {
		RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pHeader_802_11 = (HEADER_802_11 *)(pSrcBufVA + tx_hw_hdr_len);
	RTMP_SET_PACKET_WCID(pkt, ap_mlme_search_wcid(pAd, pHeader_802_11->Addr1, pHeader_802_11->Addr2, pkt));

	if (in_altx_filter_list(pHeader_802_11->FC.SubType)) {
		if (!(RTMP_GET_PACKET_TXTYPE(pkt) & TX_ATE_FRAME))
			RTMP_SET_PACKET_TYPE(pkt, TX_ALTX);
	}

	if  (!is_data_queue) {
	} else {
#ifdef UAPSD_SUPPORT
		{
			UAPSD_MR_QOS_NULL_HANDLE(pAd, pHeader_802_11, pkt);
		}

#endif /* UAPSD_SUPPORT */
		RTMP_SET_PACKET_MGMT_PKT_DATA_QUE(pkt, 1);
	}

	ret = ops->enq_mgmtq_pkt(pAd, wdev, pkt);

	return ret;
}

static INT ap_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, STA_TR_ENTRY *tr_entry,
					PNDIS_PACKET pkt, UCHAR q_idx)
{
	struct qm_ctl *qm_ctl = &pAd->qm_ctl;
	struct qm_ops *qm_ops = pAd->qm_ops;
	UINT16 occupy_cnt = (tr_entry->token_cnt + tr_entry->enqCount);

	if (occupy_cnt >= SQ_ENQ_PS_MAX) {

		if ((tr_entry->ps_queue.Number < SQ_ENQ_PSQ_MAX) &&
				(qm_ctl->total_psq_cnt < SQ_ENQ_PSQ_TOTAL_MAX)) {
			qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
			qm_ctl->total_psq_cnt++;
		} else {
			RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
		}
	} else {
		if (tr_entry->ps_queue.Number != 0) {
			NDIS_PACKET *ps_pkt = NULL;
			UCHAR ps_pkt_q_idx;
			UINT16 quota = (SQ_ENQ_PS_MAX - occupy_cnt);

			do {
				ps_pkt = qm_ops->get_psq_pkt(pAd, tr_entry);

				if (ps_pkt) {
					quota--;
					qm_ctl->total_psq_cnt--;
					ps_pkt_q_idx = RTMP_GET_PACKET_QUEIDX(ps_pkt);
					qm_ops->enq_dataq_pkt(pAd, wdev, ps_pkt, ps_pkt_q_idx);
				}
			} while (ps_pkt && (quota > 0));

			if (quota > 0) {
				qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
			} else {
				qm_ops->enq_psq_pkt(pAd, wdev, tr_entry, pkt);
				qm_ctl->total_psq_cnt++;
			}

		} else {
			qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
		}
	}

	return NDIS_STATUS_SUCCESS;
}

INT ap_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt)
{
	UCHAR wcid = RESERVED_WCID;
#ifdef IGMP_SNOOP_SUPPORT
	INT InIgmpGroup = IGMP_NONE;
	MULTICAST_FILTER_TABLE_ENTRY *pGroupEntry = NULL;
#endif /* IGMP_SNOOP_SUPPORT */
	STA_TR_ENTRY *tr_entry = NULL;
	UCHAR user_prio = 0;
	UCHAR q_idx;
	struct qm_ops *qm_ops = pAd->qm_ops;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	PACKET_INFO pkt_info;
	UCHAR *pkt_va;
	UINT pkt_len;

	RTMP_QueryPacketInfo(pkt, &pkt_info, &pkt_va, &pkt_len);
	wcid = RTMP_GET_PACKET_WCID(pkt);
	tr_entry = &pAd->MacTab.tr_entry[wcid];
	user_prio = RTMP_GET_PACKET_UP(pkt);
	q_idx = RTMP_GET_PACKET_QUEIDX(pkt);

	if (tr_entry->EntryType != ENTRY_CAT_MCAST)
		wdev_ops->detect_wmm_traffic(pAd, wdev, user_prio, FLG_IS_OUTPUT);
	else {
#ifdef IGMP_SNOOP_SUPPORT
		if (wdev->IgmpSnoopEnable) {
			if (IgmpPktInfoQuery(pAd, pkt_va, pkt, wdev,
						&InIgmpGroup, &pGroupEntry) != NDIS_STATUS_SUCCESS)
				return NDIS_STATUS_FAILURE;

			/* if it's a mcast packet in igmp gourp. ucast clone it for all members in the gourp. */
			if ((InIgmpGroup == IGMP_IN_GROUP)
				 && pGroupEntry
				 && (IgmpMemberCnt(&pGroupEntry->MemberList) > 0)) {
				NDIS_STATUS PktCloneResult = IgmpPktClone(pAd, wdev, pkt, InIgmpGroup, pGroupEntry,
								q_idx, user_prio, GET_OS_PKT_NETDEV(pkt));
				RELEASE_NDIS_PACKET(pAd, pkt, NDIS_STATUS_SUCCESS);
				return PktCloneResult;
			}

			RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);
		} else
#endif /* IGMP_SNOOP_SUPPORT */
			RTMP_SET_PACKET_TXTYPE(pkt, TX_MCAST_FRAME);
	}

	RTMP_SET_PACKET_UP(pkt, user_prio);

	RTMP_SEM_LOCK(&tr_entry->ps_sync_lock);

	if (tr_entry->ps_state == PWR_ACTIVE)
		qm_ops->enq_dataq_pkt(pAd, wdev, pkt, q_idx);
	else
		ap_ps_handle(pAd, wdev, tr_entry, pkt, q_idx);

	RTMP_SEM_UNLOCK(&tr_entry->ps_sync_lock);

	ba_ori_session_start(pAd, tr_entry, user_prio);
	return NDIS_STATUS_SUCCESS;
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
VOID ap_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;

	/* TODO:Eddy, Confirm MESH/Apcli.WAPI */
	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) {
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
		pTxBlk->pKey =  NULL;
	} else if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		pTxBlk->CipherAlg = wdev->SecConfig.GroupCipher;
		pTxBlk->KeyIdx =  wdev->SecConfig.GroupKeyId;

		if (IS_CIPHER_WEP(wdev->SecConfig.GroupCipher))
			pTxBlk->pKey = wdev->SecConfig.WepKey[pTxBlk->KeyIdx].Key;
		else
			pTxBlk->pKey = wdev->SecConfig.GTK;
	} else if (pMacEntry) {
		pTxBlk->CipherAlg = pMacEntry->SecConfig.PairwiseCipher;
		pTxBlk->KeyIdx =  pMacEntry->SecConfig.PairwiseKeyId;

		if (IS_CIPHER_WEP(pMacEntry->SecConfig.PairwiseCipher))
			pTxBlk->pKey = pMacEntry->SecConfig.WepKey[pTxBlk->KeyIdx].Key;
		else if (IS_AKM_SHA384(pMacEntry->SecConfig.AKMMap))
			pTxBlk->pKey = &pMacEntry->SecConfig.PTK[LEN_PTK_KCK_SHA384 + LEN_PTK_KEK_SHA384];
		else
			pTxBlk->pKey = &pMacEntry->SecConfig.PTK[LEN_PTK_KCK + LEN_PTK_KEK];
	}

	/* For  BMcast pMacEntry is not initial */
	if (pTxBlk->CipherAlg == 0x0)
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
}

static inline VOID ap_build_cache_802_11_header(
	IN RTMP_ADAPTER *pAd,
	IN TX_BLK *pTxBlk,
	IN UCHAR *pHeader)
{
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader80211;
	MAC_TABLE_ENTRY *pMacEntry;

	pHeader80211 = (PHEADER_802_11)pHeader;
	pMacEntry = pTxBlk->pMacEntry;
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
		   ) {
			/* The addr3 of WDS packet is Destination Mac address and Addr4 is the Source Mac address. */
			COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			COPY_MAC_ADDR(pHeader80211->Octet, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
			pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
		} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
#ifdef APCLI_SUPPORT
			if (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) {
				/* The addr3 of Ap-client packet is Destination Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader);
			} else
#endif /* APCLI_SUPPORT */
			{	/* The addr3 of normal packet send from DS is Src Mac address. */
				COPY_MAC_ADDR(pHeader80211->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);
			}
}

static inline VOID ap_build_802_11_header(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	struct wifi_dev *wdev = pTxBlk->wdev;
	STA_TR_ENTRY *tr_entry = pTxBlk->tr_entry;
	/*
		MAKE A COMMON 802.11 HEADER
	*/
	/* normal wlan header size : 24 octets */
	pTxBlk->MpduHeaderLen = sizeof(HEADER_802_11);
	pTxBlk->wifi_hdr_len = sizeof(HEADER_802_11);
	/* TODO: shiang-7603 */
	pTxBlk->wifi_hdr = &pTxBlk->HeaderBuf[tx_hw_hdr_len];
	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;
	NdisZeroMemory(wifi_hdr, sizeof(HEADER_802_11));
	wifi_hdr->FC.FrDs = 1;
	wifi_hdr->FC.Type = FC_TYPE_DATA;
	wifi_hdr->FC.SubType = ((TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) ? SUBTYPE_QDATA : SUBTYPE_DATA);

	/* TODO: shiang-usw, for BCAST/MCAST, original it's sequence assigned by "pAd->Sequence", how about now? */
	if (tr_entry) {
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
			wifi_hdr->Sequence = tr_entry->TxSeq[pTxBlk->UserPriority];
			tr_entry->TxSeq[pTxBlk->UserPriority] = (tr_entry->TxSeq[pTxBlk->UserPriority] + 1) & MAXSEQ;
		} else {
			wifi_hdr->Sequence = tr_entry->NonQosDataSeq;
			tr_entry->NonQosDataSeq = (tr_entry->NonQosDataSeq + 1) & MAXSEQ;
		}
	} else {
		wifi_hdr->Sequence = pAd->Sequence;
		pAd->Sequence = (pAd->Sequence + 1) & MAXSEQ; /* next sequence */
	}

	wifi_hdr->Frag = 0;
	wifi_hdr->FC.MoreData = TX_BLK_TEST_FLAG(pTxBlk, fTX_bMoreData);


#ifdef APCLI_SUPPORT
		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bApCliPacket)) {
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
		} else
#endif /* APCLI_SUPPORT */
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS)
			if (FALSE
#ifdef WDS_SUPPORT
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry)
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
				|| TX_BLK_TEST_FLAG(pTxBlk, fTX_bClientWDSFrame)
#endif /* CLIENT_WDS */
			   ) {
				wifi_hdr->FC.ToDs = 1;

				if (pTxBlk->pMacEntry == NULL)
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s pTxBlk->pMacEntry == NULL!\n", __func__));
				else
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr);				/* to AP2 */

				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->CurrentAddress);						/* from AP1 */
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader);					/* DA */
				COPY_MAC_ADDR(&wifi_hdr->Octet[0], pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);/* ADDR4 = SA */
				pTxBlk->MpduHeaderLen += MAC_ADDR_LEN;
				pTxBlk->wifi_hdr_len += MAC_ADDR_LEN;
			} else
#endif /* WDS_SUPPORT || CLIENT_WDS */
			{
				/* TODO: how about "MoreData" bit? AP need to set this bit especially for PS-POLL response */
#if defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT)
				if (pTxBlk->tr_entry->EntryType != ENTRY_CAT_MCAST) {
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pMacEntry->Addr); /* DA */
				} else
#endif /* defined(IGMP_SNOOP_SUPPORT) || defined(DOT11V_WNM_SUPPORT) */
				{
					COPY_MAC_ADDR(wifi_hdr->Addr1, pTxBlk->pSrcBufHeader);
				}

				COPY_MAC_ADDR(wifi_hdr->Addr2, pAd->ApCfg.MBSSID[wdev->func_idx].wdev.bssid);		/* BSSID */
				COPY_MAC_ADDR(wifi_hdr->Addr3, pTxBlk->pSrcBufHeader + MAC_ADDR_LEN);			/* SA */
			}


	if (!IS_CIPHER_NONE(pTxBlk->CipherAlg))
		wifi_hdr->FC.Wep = 1;

	pTxBlk->dot11_type = wifi_hdr->FC.Type;
	pTxBlk->dot11_subtype = wifi_hdr->FC.SubType;
}

BOOLEAN ap_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	pPacket = pTxBlk->pPacket;
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->UserPriority = RTMP_GET_PACKET_UP(pPacket);
	pTxBlk->FrameGap = IFS_HTTXOP;
	pTxBlk->pMbss = NULL;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) {
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pTxBlk->pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);
	else
		TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bClearEAPFrame);


	if (pTxBlk->tr_entry->EntryType == ENTRY_CAT_MCAST) {
		pTxBlk->pMacEntry = NULL;
		TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);
		{
#ifdef MCAST_RATE_SPECIFIC
			PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);

			if (((*pDA & 0x01) == 0x01) && (*pDA != 0xff))
				pTxBlk->pTransmit = &pAd->CommonCfg.MCastPhyMode;
			else
#endif /* MCAST_RATE_SPECIFIC */
			{
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;

				if (pTxBlk->wdev->channel > 14) {
					pTxBlk->pTransmit->field.MODE = MODE_OFDM;
					pTxBlk->pTransmit->field.MCS = MCS_RATE_6;
				}
			}
		}
		/* AckRequired = FALSE, when broadcast packet in Adhoc mode.*/
		TX_BLK_CLEAR_FLAG(pTxBlk, (fTX_bAckRequired | fTX_bAllowFrag | fTX_bWMM));

		if (RTMP_GET_PACKET_MOREDATA(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);
	} else {
		pTxBlk->pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];
		pTxBlk->pTransmit = &pTxBlk->pMacEntry->HTPhyMode;
		pMacEntry = pTxBlk->pMacEntry;

		if (!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
		else
			pTxBlk->pMbss = pMacEntry->pMbss;

#ifdef MULTI_WMM_SUPPORT

		if (IS_ENTRY_APCLI(pMacEntry))
			pTxBlk->QueIdx = EDCA_WMM1_AC0_PIPE;

#endif /* MULTI_WMM_SUPPORT */
		/* For all unicast packets, need Ack unless the Ack Policy is not set as NORMAL_ACK.*/
#ifdef MULTI_WMM_SUPPORT

		if (pTxBlk->QueIdx >= EDCA_WMM1_AC0_PIPE) {
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx - EDCA_WMM1_AC0_PIPE] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		} else
#endif /* MULTI_WMM_SUPPORT */
		{
			if (pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx] != NORMAL_ACK)
				TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bAckRequired);
			else
				TX_BLK_SET_FLAG(pTxBlk, fTX_bAckRequired);
		}

		{
#if defined(P2P_SUPPORT) || defined(RT_CFG80211_P2P_SUPPORT) || defined(CFG80211_MULTI_STA)

			if (pTxBlk->OpMode == OPMODE_AP)
#else
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* P2P_SUPPORT || RT_CFG80211_P2P_SUPPORT*/
			{
#ifdef WDS_SUPPORT

				if (IS_ENTRY_WDS(pMacEntry))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWDSEntry);
				else
#endif /* WDS_SUPPORT */
#ifdef CLIENT_WDS
				if (IS_ENTRY_CLIWDS(pMacEntry)) {
					PUCHAR pDA = GET_OS_PKT_DATAPTR(pPacket);
					PUCHAR pSA = GET_OS_PKT_DATAPTR(pPacket) + MAC_ADDR_LEN;
					UCHAR idx = pMacEntry->func_tb_idx;

					if (((idx < MAX_MBSSID_NUM(pAd))
						 && !MAC_ADDR_EQUAL(pSA, pAd->ApCfg.MBSSID[idx].Bssid))
						|| !MAC_ADDR_EQUAL(pDA, pMacEntry->Addr)
						)
						TX_BLK_SET_FLAG(pTxBlk, fTX_bClientWDSFrame);
				} else
#endif /* CLIENT_WDS */
				if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
				} else
					return FALSE;

				/* If both of peer and us support WMM, enable it.*/
				if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_WMM_INUSED) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
					TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM);
			}
		}

		if (pTxBlk->TxFrameType == TX_LEGACY_FRAME) {
			if (((RTMP_GET_PACKET_LOWRATE(pPacket))
#ifdef UAPSD_SUPPORT
				  && (!(pMacEntry && (pMacEntry->bAPSDFlagSPStart)))
#endif /* UAPSD_SUPPORT */
				 ) ||
				 ((pAd->OpMode == OPMODE_AP) && (pMacEntry->MaxHTPhyMode.field.MODE == MODE_CCK) && (pMacEntry->MaxHTPhyMode.field.MCS == RATE_1))
			   ) {
				/* Specific packet, i.e., bDHCPFrame, bEAPOLFrame, bWAIFrame, need force low rate. */
				pTxBlk->pTransmit = &pAd->MacTab.Content[MCAST_WCID_TO_REMOVE].HTPhyMode;
				TX_BLK_SET_FLAG(pTxBlk, fTX_ForceRate);

				/* Modify the WMM bit for ICV issue. If we have a packet with EOSP field need to set as 1, how to handle it? */
				if (!pTxBlk->pMacEntry)
					MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pTxBlk->pMacEntry is NULL!!\n", __func__));
				else if (IS_HT_STA(pTxBlk->pMacEntry) &&
						 (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET)) &&
						 ((pAd->CommonCfg.bRdg == TRUE) && CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE)))
					TX_BLK_CLEAR_FLAG(pTxBlk, fTX_bWMM);
			}

			if ((IS_HT_RATE(pMacEntry) == FALSE) &&
					  (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))) {
				/* Currently piggy-back only support when peer is operate in b/g mode.*/
				TX_BLK_SET_FLAG(pTxBlk, fTX_bPiggyBack);
			}

			if (RTMP_GET_PACKET_MOREDATA(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bMoreData);

#ifdef UAPSD_SUPPORT

			if (RTMP_GET_PACKET_EOSP(pPacket))
				TX_BLK_SET_FLAG(pTxBlk, fTX_bWMM_UAPSD_EOSP);

#endif /* UAPSD_SUPPORT */
		} else if (pTxBlk->TxFrameType == TX_FRAG_FRAME)
			TX_BLK_SET_FLAG(pTxBlk, fTX_bAllowFrag);

		pMacEntry->DebugTxCount++;

#ifdef IGMP_SNOOP_SUPPORT
		if (RTMP_GET_PACKET_MCAST_CLONE(pPacket))
			TX_BLK_SET_FLAG(pTxBlk, fTX_MCAST_CLONE);
#endif	

	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN ap_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;

	pPacket = pTxBlk->pPacket;
	pTxBlk->Wcid = RTMP_GET_PACKET_WCID(pPacket);
	RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);

	if (RTMP_GET_PACKET_MGMT_PKT(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_CT_WithTxD);

	if (RTMP_GET_PACKET_CLEAR_EAP_FRAME(pPacket))
		TX_BLK_SET_FLAG(pTxBlk, fTX_bClearEAPFrame);

	if (IS_ASIC_CAP(pAd, fASIC_CAP_TX_HDR_TRANS)) { 
		if ((pTxBlk->TxFrameType == TX_LEGACY_FRAME) ||
			(pTxBlk->TxFrameType == TX_AMSDU_FRAME) ||
			(pTxBlk->TxFrameType == TX_MCAST_FRAME))
			TX_BLK_SET_FLAG(pTxBlk, fTX_HDR_TRANS);
	}

	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	/*get MBSS for tx counter usage*/
	if (pTxBlk->TxFrameType != TX_MCAST_FRAME)
		pTxBlk->pMbss = pTxBlk->pMacEntry->pMbss;
	return TRUE;
}

INT ap_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	PQUEUE_ENTRY q_entry;
	UCHAR *tmac_info;
	HTTRANSMIT_SETTING *transmit;
	UCHAR MlmeRate;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	PHEADER_802_11 pHeader_802_11;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	RTMP_ARCH_OP *op = &pAd->archOps;
	struct dev_rate_info *rate;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR PID, wcid = tx_blk->Wcid, tx_rate;
	UCHAR prot = 0;
	UCHAR apidx = 0;
	MAC_TX_INFO mac_info;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);
	tmac_info = tx_blk->pSrcBufHeader;
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/* Verify Mlme rate for a / g bands.*/
	if ((wdev->channel > 14) && (MlmeRate < RATE_6)) /* 11A band*/
		MlmeRate = RATE_6;

	rate = &wdev->rate;
	pHeader_802_11->FC.MoreData = RTMP_GET_PACKET_MOREDATA(tx_blk->pPacket);
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) { /* must be PS-POLL*/
		bAckRequired = FALSE;
#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration = 100;
		}

#endif /* VHT_TXBF_SUPPORT */
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST*/
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
#ifdef SOFT_SOUNDING

			if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
				&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
				bAckRequired = FALSE;
				pHeader_802_11->Duration = 0;
			} else
#endif /* SOFT_SOUNDING */
			{
				bAckRequired = TRUE;
				pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

				if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bInsertTimestamp = TRUE;
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Response*/
#ifdef SPECIFIC_TX_POWER_SUPPORT
					/* Find which MBSSID to be send this probeRsp */
					UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

					if (!(apidx >= pAd->ApCfg.BssidNum) &&
						 (pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
						 (rate->MlmeTransmit.field.MODE == MODE_CCK) &&
						 (rate->MlmeTransmit.field.MCS == RATE_1))
						TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_PROBE_REQ) && (pHeader_802_11->FC.Type == FC_TYPE_MGMT)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Probe Request*/
				} else if ((pHeader_802_11->FC.SubType == SUBTYPE_DEAUTH) &&
						   (pMacEntry == NULL)) {
					bAckRequired = FALSE; /* Disable ACK to prevent retry 0x1f for Deauth */
				}
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/*
		Before radar detection done, mgmt frame can not be sent but probe req
		Because we need to use probe req to trigger driver to send probe req in passive scan
	*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode != RD_NORMAL_MODE)) {
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	/*
		fill scatter-and-gather buffer list into TXD. Internally created NDIS PACKET
		should always has only one physical buffer, and the whole frame size equals
		to the first scatter buffer size

		Initialize TX Descriptor
		For inter-frame gap, the number is for this frame and next frame
		For MLME rate, we will fix as 2Mb to match other vendor's implement

		management frame doesn't need encryption.
		so use RESERVED_WCID no matter u are sending to specific wcid or not
	*/
	PID = PID_MGMT;
#ifdef DOT11W_PMF_SUPPORT
	PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &prot);
#endif

	if (pMacEntry) {
		wcid = pMacEntry->wcid;
	} else {
		wcid = 0;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pMacEntry is null !!\n"));
	}

	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
	transmit = &rate->MlmeTransmit;

	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));

	if (prot)
		mac_info.prot = prot;

	if (prot == 2 || prot == 3)
		mac_info.bss_idx = apidx;

	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.Ack = bAckRequired;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.PsmBySw = 1;
	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1) {
			mac_info.AntPri = BAND1_SPE_IDX;
		}
	}
	/* check if the pkt is Tmr frame. */
#ifdef FTM_SUPPORT

	if (IsPublicActionFrame(pAd, pHeader_802_11)) {
		PFTM_PEER_ENTRY pFtmEntry = FtmEntrySearch(pAd, pHeader_802_11->Addr1);

		if (pFtmEntry && pFtmEntry->bNeedTmr) {
			mac_info.IsTmr = TRUE;
			PID = FtmGetNewPid(pAd);
			FtmAddPidPendingNode(pAd, pFtmEntry, PID);
			mac_info.ftm_bw = pFtmEntry->VerdictParm.ftm_format_and_bw;
			pFtmEntry->bTxOK = 0;	/* init value */
			pFtmEntry->bTxSCallbackCheck = 1;
			RTMPSetTimer(&pFtmEntry->FtmTxTimer, 30);
			pFtmEntry->bNeedTmr = FALSE;
		}
	}

#endif /* FTM_SUPPORT */
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		default:
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 __func__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
			hex_dump("DataFrame", (char *)pHeader_802_11, 24);
			break;
		}

		if (pMacEntry && pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			PID = PID_PS_DATA;

		mac_info.WCID = wcid;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_PS_POLL:
			mac_info.hdr_len = sizeof(PSPOLL_FRAME);
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		default:
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 __func__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
			break;
		}
	} else {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame send to MgmtRing, need to assign the length!\n",
				 __func__));
	}

	mac_info.PID = PID;
	mac_info.TID = 0;
	mac_info.TxRate = tx_rate;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));

	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry))
		mac_info.OmacIdx = pAd->MacTab.tr_entry[pMacEntry->wcid].OmacIdx;
	else
		mac_info.OmacIdx = wdev->OmacIdx;

	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s(): %d, WMMSET=%d,QId=%d\n",
			  __func__, __LINE__, mac_info.wmm_set, mac_info.q_idx));
#ifdef CONFIG_MULTI_CHANNEL

	if (pAd->Mlme.bStartMcc == TRUE) {
		if ((NdisEqualMemory(pAd->cfg80211_ctrl.P2PCurrentAddress, pHeader_802_11->Addr2, MAC_ADDR_LEN))
			|| (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP))
			mac_info.q_idx = Q_IDX_AC10;
		else
			mac_info.q_idx = Q_IDX_AC0;
	}

#endif /* CONFIG_MULTI_CHANNEL */
#ifdef APCLI_SUPPORT

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_APCLI(pMacEntry)
#ifdef MAC_REPEATER_SUPPORT
									|| IS_ENTRY_REPEATER(pMacEntry)
#endif /* MAC_REPEATER_SUPPORT */
								   )) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			if (IS_MT7615(pAd) || IS_MT7622(pAd) || IS_P18(pAd) || IS_MT7663(pAd))
				mac_info.IsAutoRate = TRUE;
		}
	}

#endif /* APCLI_SUPPORT */
#ifdef SOFT_SOUNDING

	if (((pHeader_802_11->FC.Type == FC_TYPE_DATA) && (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))
		&& pMacEntry && (pMacEntry->snd_reqired == TRUE)) {
		wcid = RESERVED_WCID;
		tx_rate = (UCHAR)pMacEntry->snd_rate.field.MCS;
		transmit = &pMacEntry->snd_rate;
		mac_info.Txopmode = IFS_PIFS;
		pMacEntry->snd_reqired = FALSE;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():Kick Sounding to %02x:%02x:%02x:%02x:%02x:%02x, dataRate(PhyMode:%s, BW:%sHz, %dSS, MCS%d)\n",
				 __func__, PRINT_MAC(pMacEntry->Addr),
				 get_phymode_str(transmit->field.MODE),
				 get_bw_str(transmit->field.BW),
				 (transmit->field.MCS>>4) + 1, (transmit->field.MCS & 0xf)));
	} else
#endif /* SOFT_SOUNDING */
	{
		mac_info.Txopmode = IFS_BACKOFF;
	}

	/* if we are going to send out FTM action. enable CR to report TMR report.*/
	if ((pAd->pTmrCtrlStruct != NULL) && (pAd->pTmrCtrlStruct->TmrEnable != TMR_DISABLE)) {
		if (mac_info.IsTmr == TRUE) {
			/* Leo: already set at TmrCtrlInit() MtSetTmrCR(pAd, TMR_INITIATOR); */
			pAd->pTmrCtrlStruct->TmrState = SEND_OUT;
		}
	}

	return op->mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT ap_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	PQUEUE_ENTRY q_entry;
	UCHAR *tmac_info, *frm_buf;
	UINT frm_len;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR hw_hdr_info[TXD_SIZE];
#endif
	PHEADER_802_11 pHeader_802_11;
	PFRAME_BAR pBar = NULL;
	BOOLEAN bAckRequired, bInsertTimestamp;
	UCHAR MlmeRate, wcid = tx_blk->Wcid, tx_rate;
	MAC_TABLE_ENTRY *pMacEntry = tx_blk->pMacEntry;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;
	HTTRANSMIT_SETTING *transmit, TransmitSetting;
	MAC_TX_INFO mac_info;
	RTMP_ARCH_OP *op = &pAd->archOps;
#ifdef SPECIFIC_TX_POWER_SUPPORT
	UCHAR TxPwrAdj = 0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */
	struct dev_rate_info *rate;
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return NDIS_STATUS_FAILURE;

	pDot11h = wdev->pDot11_H;
	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	q_entry = RemoveHeadQueue(&tx_blk->TxPacketList);
	tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(q_entry);
	ap_fill_offload_tx_blk(pAd, wdev, tx_blk);
	pHeader_802_11 = (HEADER_802_11 *)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
	
	rate = &wdev->rate;
	frm_buf = tx_blk->pSrcBufHeader;
	frm_len = tx_blk->SrcBufLen;
	tmac_info = tx_blk->pSrcBufHeader;

	if (pHeader_802_11->Addr1[0] & 0x01)
		MlmeRate = pAd->CommonCfg.BasicMlmeRate;
	else
		MlmeRate = pAd->CommonCfg.MlmeRate;

	/* Verify Mlme rate for a/g bands.*/
	if ((wdev->channel > 14) && (MlmeRate < RATE_6)) { /* 11A band*/
		MlmeRate = RATE_6;
		transmit = &rate->MlmeTransmit;
		transmit->field.MCS = MCS_RATE_6;
		transmit->field.MODE = MODE_OFDM;
	}

	/*
		Should not be hard code to set PwrMgmt to 0 (PWR_ACTIVE)
		Snice it's been set to 0 while on MgtMacHeaderInit
		By the way this will cause frame to be send on PWR_SAVE failed.
	*/
	/* In WMM-UAPSD, mlme frame should be set psm as power saving but probe request frame */
	bInsertTimestamp = FALSE;

	if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		if (pHeader_802_11->FC.SubType == SUBTYPE_BLOCK_ACK_REQ) {
			pBar = (PFRAME_BAR)(tx_blk->pSrcBufHeader + tx_hw_hdr_len);
			bAckRequired = TRUE;
		} else
			bAckRequired = FALSE;

#ifdef VHT_TXBF_SUPPORT

		if (pHeader_802_11->FC.SubType == SUBTYPE_VHT_NDPA) {
			pHeader_802_11->Duration =
				RTMPCalcDuration(pAd, MlmeRate, (tx_blk->SrcBufLen - TXINFO_SIZE - cap->TXWISize - TSO_SIZE));
		}
#endif /* VHT_TXBF_SUPPORT*/
	} else { /* FC_TYPE_MGMT or FC_TYPE_DATA(must be NULL frame)*/
		if (pHeader_802_11->Addr1[0] & 0x01) { /* MULTICAST, BROADCAST */
			bAckRequired = FALSE;
			pHeader_802_11->Duration = 0;
		} else {
			bAckRequired = TRUE;
			pHeader_802_11->Duration = RTMPCalcDuration(pAd, MlmeRate, 14);

			if (pHeader_802_11->FC.SubType == SUBTYPE_PROBE_RSP) {
				bInsertTimestamp = TRUE;
				bAckRequired = FALSE;
#ifdef SPECIFIC_TX_POWER_SUPPORT
				/* Find which MBSSID to be send this probeRsp */
				UINT32 apidx = get_apidx_by_addr(pAd, pHeader_802_11->Addr2);

				if (!(apidx >= pAd->ApCfg.BssidNum) &&
					(pAd->ApCfg.MBSSID[apidx].TxPwrAdj != -1) &&
					(rate->MlmeTransmit.field.MODE == MODE_CCK) &&
					(rate->MlmeTransmit.field.MCS == RATE_1))
					TxPwrAdj = pAd->ApCfg.MBSSID[apidx].TxPwrAdj;

#endif /* SPECIFIC_TX_POWER_SUPPORT */
			}
		}
	}

	pHeader_802_11->Sequence = pAd->Sequence++;

	if (pAd->Sequence > 0xfff)
		pAd->Sequence = 0;

	/* Before radar detection done, mgmt frame can not be sent but probe req*/
	/* Because we need to use probe req to trigger driver to send probe req in passive scan*/
	if ((pHeader_802_11->FC.SubType != SUBTYPE_PROBE_REQ)
		&& (pAd->CommonCfg.bIEEE80211H == 1)
		&& (pDot11h->RDMode != RD_NORMAL_MODE)) {
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("MlmeHardTransmit --> radar detect not in normal mode !!!\n"));
		RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (pMacEntry) {
		wcid = pMacEntry->wcid;
	} else {
		wcid = 0;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pMacEntry is null !!\n"));
	}

	tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
	transmit = &rate->MlmeTransmit;
	NdisZeroMemory((UCHAR *)&mac_info, sizeof(mac_info));
	mac_info.FRAG = FALSE;
	mac_info.CFACK = FALSE;
	mac_info.InsTimestamp = bInsertTimestamp;
	mac_info.AMPDU = FALSE;
	mac_info.BM = IS_BM_MAC_ADDR(pHeader_802_11->Addr1);
	mac_info.Ack = bAckRequired;
	mac_info.NSeq = FALSE;
	mac_info.BASize = 0;
	mac_info.WCID = wcid;
	mac_info.TID = 0;
	mac_info.wmm_set = HcGetWmmIdx(pAd, wdev);
	mac_info.q_idx  = HcGetMgmtQueueIdx(pAd, wdev, RTMP_GET_PACKET_TYPE(tx_blk->pPacket));
	if (pAd->CommonCfg.bSeOff != TRUE) {
		if (HcGetBandByWdev(wdev) == BAND0)
			mac_info.AntPri = BAND0_SPE_IDX;
		else if (HcGetBandByWdev(wdev) == BAND1)
			mac_info.AntPri = BAND1_SPE_IDX;
	}
	if (pMacEntry && IS_ENTRY_REPEATER(pMacEntry))
		mac_info.OmacIdx = pAd->MacTab.tr_entry[pMacEntry->wcid].OmacIdx;
	else
		mac_info.OmacIdx = wdev->OmacIdx;

	mac_info.Type = pHeader_802_11->FC.Type;
	mac_info.SubType = pHeader_802_11->FC.SubType;
	mac_info.Length = (tx_blk->SrcBufLen - tx_hw_hdr_len);

	if (pHeader_802_11->FC.Type == FC_TYPE_MGMT) {
		mac_info.hdr_len = 24;

		if (pHeader_802_11->FC.Order == 1)
			mac_info.hdr_len += 4;

		mac_info.PID = PID_MGMT;

#ifdef DOT11W_PMF_SUPPORT
		PMF_PerformTxFrameAction(pAd, pHeader_802_11, tx_blk->SrcBufLen, tx_hw_hdr_len, &mac_info.prot);
#endif
	} else if (pHeader_802_11->FC.Type == FC_TYPE_DATA) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_DATA_NULL:
			mac_info.hdr_len = 24;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		case SUBTYPE_QOS_NULL:
			mac_info.hdr_len = 26;
			tx_rate = (UCHAR)rate->MlmeTransmit.field.MCS;
			transmit = &rate->MlmeTransmit;
			break;

		default:
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					 __func__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
			hex_dump("DataFrame", frm_buf, frm_len);
			break;
		}

		mac_info.WCID = wcid;

		if (pMacEntry && pAd->MacTab.tr_entry[wcid].PsDeQWaitCnt)
			mac_info.PID = PID_PS_DATA;
		else
			mac_info.PID = PID_MGMT;
	} else if (pHeader_802_11->FC.Type == FC_TYPE_CNTL) {
		switch (pHeader_802_11->FC.SubType) {
		case SUBTYPE_BLOCK_ACK_REQ:
			mac_info.PID = PID_CTL_BAR;
			mac_info.hdr_len = 16;
			mac_info.SpeEn = 0;
			mac_info.TID = pBar->BarControl.TID;

			if (wdev->channel > 14) {
				/* 5G */
				TransmitSetting.field.MODE = MODE_OFDM;
			} else {
				/* 2.4G */
				TransmitSetting.field.MODE = MODE_CCK;
			}

			TransmitSetting.field.BW = BW_20;
			TransmitSetting.field.STBC = 0;
			TransmitSetting.field.ShortGI = 0;
			TransmitSetting.field.MCS = 0;
			TransmitSetting.field.ldpc = 0;
			transmit = &TransmitSetting;
			break;

		default:
			MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR,
					 ("%s(): FIXME!!!Unexpected frame(Type=%d, SubType=%d) send to MgmtRing, need to assign the length!\n",
					  __func__, pHeader_802_11->FC.Type, pHeader_802_11->FC.SubType));
			hex_dump("Control Frame", frm_buf, frm_len);
			break;
		}
	}

	mac_info.TxRate = tx_rate;
	mac_info.Txopmode = IFS_BACKOFF;
	mac_info.SpeEn = 1;
	mac_info.Preamble = LONG_PREAMBLE;
	mac_info.IsAutoRate = FALSE;
#ifdef APCLI_SUPPORT

	if ((pHeader_802_11->FC.Type == FC_TYPE_DATA)
		&& ((pHeader_802_11->FC.SubType == SUBTYPE_DATA_NULL) || (pHeader_802_11->FC.SubType == SUBTYPE_QOS_NULL))) {
		if ((pMacEntry != NULL) && (IS_ENTRY_APCLI(pMacEntry)
#ifdef MAC_REPEATER_SUPPORT
									|| IS_ENTRY_REPEATER(pMacEntry)
#endif /* MAC_REPEATER_SUPPORT */
								   )) {
			/* CURRENT_BW_TX_CNT/CURRENT_BW_FAIL_CNT only count for aute rate */
			mac_info.IsAutoRate = TRUE;
		}
	}

#endif /* APCLI_SUPPORT */
	return op->mlme_hw_tx(pAd, tmac_info, &mac_info, transmit, tx_blk);
}

INT ap_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr = NULL, *src_ptr;
	USHORT freeCnt = 1;
	BOOLEAN bVLANPkt;
	MAC_TABLE_ENTRY *pMacEntry;
	STA_TR_ENTRY *tr_entry;
	PQUEUE_ENTRY pQEntry;
#ifndef MT_MAC
	BOOLEAN bHTCPlus = FALSE;
#endif /* !MT_MAC */
	UINT hdr_offset, cache_sz = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 TXWISize = cap->TXWISize;
	UINT8 tx_hw_hdr_len = cap->tx_hw_hdr_len;

	ASSERT(pTxBlk);
	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {

		ap_tx_drop_update(pAd, wdev, pTxBlk);
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	pMacEntry = pTxBlk->pMacEntry;
	tr_entry = pTxBlk->tr_entry;

	if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
		if (IS_HIF_TYPE(pAd, HIF_MT))
			hdr_offset = tx_hw_hdr_len;
		else
			hdr_offset = TXINFO_SIZE + TXWISize + TSO_SIZE;

		if ((tr_entry->isCached)
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			&& (pMacEntry->TxSndgType == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
		   ) {
#ifndef VENDOR_FEATURE1_SUPPORT
			NdisMoveMemory((UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]),
						   (UCHAR *)(&tr_entry->CachedBuf[0]),
						   TXWISize + sizeof(HEADER_802_11));
#else
			pTxBlk->HeaderBuf = (UCHAR *)(tr_entry->HeaderBuf);
#endif /* VENDOR_FEATURE1_SUPPORT */
			pHeaderBufPtr = (UCHAR *)(&pTxBlk->HeaderBuf[hdr_offset]);
			ap_build_cache_802_11_header(pAd, pTxBlk, pHeaderBufPtr);
#ifdef SOFT_ENCRYPT
			RTMPUpdateSwCacheCipherInfo(pAd, pTxBlk, pHeaderBufPtr);
#endif /* SOFT_ENCRYPT */
		} else {
			ap_build_802_11_header(pAd, pTxBlk);
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
		/* skip common header */
		pHeaderBufPtr += pTxBlk->MpduHeaderLen;
#ifdef VENDOR_FEATURE1_SUPPORT

		if (tr_entry->isCached
			&& (tr_entry->Protocol == (RTMP_GET_PACKET_PROTOCOL(pTxBlk->pPacket)))
#ifdef SOFT_ENCRYPT
			&& !TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)
#endif /* SOFT_ENCRYPT */
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			&& (pMacEntry->TxSndgType == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
		   ) {
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT

			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			   ) {
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
		} else
#endif /* VENDOR_FEATURE1_SUPPORT */
		{
			/* build QOS Control bytes */
			*pHeaderBufPtr = (pTxBlk->UserPriority & 0x0F);
#ifdef UAPSD_SUPPORT

			if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
				&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
			   ) {
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

			/* TODO: Shiang-usw, we need a more proper way to handle this instead of ifndef MT_MAC ! */
			/* For MT_MAC, SW not to prepare the HTC field for RDG enable */
			/* build HTC control field after QoS field */
			if ((pAd->CommonCfg.bRdg == TRUE)
				&& (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_RDG_CAPABLE))
#ifdef TXBF_SUPPORT
				&& (pMacEntry->TxSndgType != SNDG_TYPE_NDP)
#endif /* TXBF_SUPPORT */
			   ) {
				NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
				((PHT_CONTROL)pHeaderBufPtr)->RDG = 1;
				bHTCPlus = TRUE;
			}

#endif /* MT_MAC */
#ifdef TXBF_SUPPORT
#ifndef MT_MAC

			if (cap->FlgHwTxBfCap) {
				pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;
				/* TODO: shiang-lock, fix ME!! */
				NdisAcquireSpinLock(&pMacEntry->TxSndgLock);

				if (pMacEntry->TxSndgType >= SNDG_TYPE_SOUNDING) {
					if (bHTCPlus == FALSE) {
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}

					if (pMacEntry->TxSndgType == SNDG_TYPE_SOUNDING) {
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0))
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
						else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Clear NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 0;
					} else if (pMacEntry->TxSndgType == SNDG_TYPE_NDP) {
						/* Select compress if supported. Otherwise select noncompress */
						if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
							(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0) &&
							(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs/8))
						   )
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
						else
							((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

						/* Set NDP Announcement */
						((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 1;
						pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
						pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
					}

					pTxBlk->TxSndgPkt = pMacEntry->TxSndgType;
					pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
				}

				NdisReleaseSpinLock(&pMacEntry->TxSndgLock);
#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX)
				/* have to replace this by the correct condition!!! */
				pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif

				/*
					Ignore sounding frame because the signal format of sounding frmae may
					be different from normal data frame, which may result in different MFB
				*/
				if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >= MCSFBK_MRQ) &&
					(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE)) {
					if (bHTCPlus == FALSE) {
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}

					MFB_PerPareMRQ(pAd, pHeaderBufPtr, pMacEntry);
				}

				if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >= MCSFBK_MRQ &&
					pMacEntry->toTxMfb == 1) {
					if (bHTCPlus == FALSE) {
						NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
						bHTCPlus = TRUE;
					}

					MFB_PerPareMFB(pAd, pHeaderBufPtr, pMacEntry);/* not complete yet!!! */
					pMacEntry->toTxMfb = 0;
				}

#endif /* MFB_SUPPORT */
			}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */

#ifndef MT_MAC
			if (bHTCPlus == TRUE) {
				wifi_hdr->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->MpduHeaderLen += 4;
				pTxBlk->wifi_hdr_len += 4;
			}
#endif /* MT_MAC */

			/*pTxBlk->MpduHeaderLen = pHeaderBufPtr - pTxBlk->HeaderBuf - TXWI_SIZE - TXINFO_SIZE; */
			ASSERT(pTxBlk->MpduHeaderLen >= 24);
			/* skip 802.3 header */
			pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader + LENGTH_802_3;
			pTxBlk->SrcBufLen -= LENGTH_802_3;

			/* skip vlan tag */
			if (RTMP_GET_PACKET_VLAN(pTxBlk->pPacket)) {
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

			if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
				tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
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
	} else {
		pTxBlk->MpduHeaderLen = 0;
		pTxBlk->HdrPadLen = 2;
		pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	}

	if ((tr_entry->isCached)
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
		&& (pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE)
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
	   )
		write_tmac_info_Cache(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);
	else {
		pAd->archOps.write_tmac_info(pAd, &pTxBlk->HeaderBuf[0], pTxBlk);

		if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
			tr_entry->isCached = FALSE;

		NdisZeroMemory((UCHAR *)(&tr_entry->CachedBuf[0]), sizeof(tr_entry->CachedBuf));

		if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			cache_sz = (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]));
			src_ptr = (UCHAR *)(&pTxBlk->HeaderBuf[TXINFO_SIZE]);
			NdisMoveMemory((UCHAR *)(&tr_entry->CachedBuf[0]), src_ptr, cache_sz);
		}

#ifdef VENDOR_FEATURE1_SUPPORT
		/* use space to get performance enhancement */
		NdisZeroMemory((UCHAR *)(&tr_entry->HeaderBuf[0]), sizeof(tr_entry->HeaderBuf));
		NdisMoveMemory((UCHAR *)(&tr_entry->HeaderBuf[0]),
					   (UCHAR *)(&pTxBlk->HeaderBuf[0]),
					   (pHeaderBufPtr - (UCHAR *)(&pTxBlk->HeaderBuf[0])));
#endif /* VENDOR_FEATURE1_SUPPORT */
	}

#ifdef TXBF_SUPPORT
#ifndef MT_MAC

	if (pTxBlk->TxSndgPkt != SNDG_TYPE_DISABLE)
		tr_entry->isCached = FALSE;

#endif
#endif /* TXBF_SUPPORT */

	ap_tx_ok_update(pAd, wdev, pTxBlk);
	pAd->archOps.write_tx_resource(pAd, pTxBlk, TRUE, &freeCnt);
#ifdef SMART_ANTENNA

	if (pMacEntry)
		pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
	return NDIS_STATUS_SUCCESS;
}

INT ap_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;
	PQUEUE_ENTRY pQEntry;
	INT32 ret = NDIS_STATUS_SUCCESS;
	UINT index = 0;

	ASSERT((tx_blk->TxPacketList.Number > 1));

	while (tx_blk->TxPacketList.Head) {
		pQEntry = RemoveHeadQueue(&tx_blk->TxPacketList);
		tx_blk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

		if (index == 0)
			tx_blk->amsdu_state = TX_AMSDU_ID_FIRST;
		else if (index == (tx_blk->TotalFrameNum - 1))
			tx_blk->amsdu_state = TX_AMSDU_ID_LAST;
		else
			tx_blk->amsdu_state = TX_AMSDU_ID_MIDDLE;

		if (!fill_tx_blk(pAd, wdev, tx_blk)) {

			ap_tx_drop_update(pAd, wdev, tx_blk);
			RELEASE_NDIS_PACKET(pAd, tx_blk->pPacket, NDIS_STATUS_FAILURE);
			continue;
		}

		if (TX_BLK_TEST_FLAG(tx_blk, fTX_HDR_TRANS))
			wdev_ops->ieee_802_3_data_tx(pAd, wdev, tx_blk);
		else
			wdev_ops->ieee_802_11_data_tx(pAd, wdev, tx_blk);

		ap_tx_ok_update(pAd, wdev, tx_blk);

		ret = arch_ops->hw_tx(pAd, tx_blk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
		tx_blk->frame_idx++;
		index++;
	}

	return NDIS_STATUS_SUCCESS;
}

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
BOOLEAN vow_is_queue_full(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR qidx)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	("%s: STA %d, qidx %d, vow_queue_len %d \n", __func__, wcid, qidx, pAd->vow_queue_len[wcid][qidx]));
	if (pAd->vow_queue_len[wcid][qidx] >= pAd->vow_q_len)
		return TRUE;
	else
		return FALSE;
}

UINT32 vow_clone_legacy_frame(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk)
{
	UINT32 i;
	/*PNDIS_PACKET pkt;*/
	TX_BLK txb, *pTemp_TxBlk;
	struct wifi_dev *wdev = pTxBlk->wdev;
	UINT32 KickRingBitMap = 0;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	pTemp_TxBlk = &txb;

	/* return BC/MC */
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		UCHAR wmm_set = 0;

		if (wdev) {
			wmm_set = HcGetWmmIdx(pAd, wdev);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, wmm set %d\n",
			__func__, pTxBlk->Wcid, wmm_set));
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d, wdev is null!\n",
			__func__, pTxBlk->Wcid));
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("\x1b[32m%s: bc/mc packet ........ wcid %d\x1b[m\n", __func__, pTxBlk->Wcid));


		return KickRingBitMap;
	}

	/*pkt = DuplicatePacket(wdev->if_dev, pTxBlk->pPacket);*/
	/* backup TXBLK */
	os_move_mem(&txb, pTxBlk, sizeof(TX_BLK));


	/*if (pkt == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: DuplicatePacket failed!!\n", __func__));
		return;
	}
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: clone 1 pkt %p, vow_cloned_wtbl_num %d, pkt %p, clone pkt %p\n",
		__func__, pkt, pAd->vow_cloned_wtbl_max, pTxBlk->pPacket, pkt));*/


	if (pAd->vow_cloned_wtbl_max) {
		UINT32 end, start;
		struct wifi_dev *wdev = pTxBlk->wdev;
		UCHAR wmm_set;

		if (wdev) {
			wmm_set = HcGetWmmIdx(pAd, wdev);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: wcid %d, wmm set %d\n",
			__func__, pTxBlk->Wcid, wmm_set));
		} else {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wcid %d, wdev is null!\n",
			__func__, pTxBlk->Wcid));
		}


		if (pAd->CommonCfg.dbdc_mode) {
			/* if (wmm_set == 0) */
			if (pTxBlk->Wcid == 1) {
				start = 3;
				end = pAd->vow_cloned_wtbl_num[0];
			} else {
				start = pAd->vow_cloned_wtbl_num[0] + 1;
				end = pAd->vow_cloned_wtbl_num[1];
			}
		} else {
			start = 2;
			end = pAd->vow_cloned_wtbl_num[0];
		}

		/* for (i = 3; i <= pAd->vow_cloned_wtbl_num; i++) */
		for (i = start; i <= end; i++) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: sta%d, tx_en %d\n",
					 __func__, i, pAd->vow_tx_en[i]));

			/* check queue status */
			if (vow_is_queue_full(pAd, i, pAd->vow_sta_ac[i])) {
				/*printk("\x1b[31m%s: full wcid %d, ac %d\x1b[m\n", __FUNCTION__, i,  pAd->vow_sta_ac[i]);*/
				continue;
			}

			if (pAd->vow_tx_en[i] && (pAd->archOps.get_tx_resource_free_num(pAd, 0) > 2)) {
				USHORT free_cnt = 1, tx_idx;
				/* clone packet */
				/*PNDIS_PACKET clone = DuplicatePacket(wdev->if_dev, pkt);*/
				PNDIS_PACKET clone = DuplicatePacket(wdev->if_dev, pTxBlk->pPacket);

				if (clone == NULL) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s: DuplicatePacket failed!!\n", __func__));
					/*pAd->archOps.kickout_data_tx(pAd, pTxBlk, pTxBlk->resource_idx);*/ /* move to ge_tx_pkt_deq_bh() */
					goto CLONE_DONE;
				}

				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: sta%d, free no %d, clone pkt %p\n",
					 __func__, i, pAd->archOps.get_tx_resource_free_num(pAd, 0), clone));
				/* cloned PKT */
				pTemp_TxBlk->pPacket = clone;

				/* pTxBlk->Wcid = 1; */
				pTemp_TxBlk->Wcid = i;

				if (pAd->vow_sta_ac[i] != QID_AC_VO)
					pTemp_TxBlk->resource_idx = pAd->vow_sta_ac[i]; /* BK/BE/VI */
				else
					pTemp_TxBlk->resource_idx = HIF_TX_IDX4; /* VO */


				/* get MAC TXD buffer */
				pTemp_TxBlk->HeaderBuf = pAd->archOps.get_hif_buf(pAd,
							pTemp_TxBlk, pTemp_TxBlk->resource_idx,
							pTemp_TxBlk->TxFrameType);

				/* modified DA */
				if (!TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
					HEADER_802_11 *hdr = (HEADER_802_11 *)pTemp_TxBlk->wifi_hdr;

					hdr->Addr1[4] = i;
				} else if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
					INT32 ret = NDIS_STATUS_SUCCESS;

					/* modified DA */

					/* fill TXD */
					if ((pTxBlk->amsdu_state == TX_AMSDU_ID_NO) ||
						(pTxBlk->amsdu_state == TX_AMSDU_ID_LAST))
						pAd->archOps.write_tmac_info(pAd, &pTemp_TxBlk->HeaderBuf[0], pTemp_TxBlk);

					/* fill TXP in TXD */
					ret = pAd->archOps.write_txp_info(
						pAd, &pTemp_TxBlk->HeaderBuf[cap->tx_hw_hdr_len], pTemp_TxBlk);

					if (ret != NDIS_STATUS_SUCCESS) {
						RELEASE_NDIS_PACKET(pAd, clone, NDIS_STATUS_FAILURE);
						goto CLONE_DONE;
					}
					/* fill DMAD */
					if ((pTxBlk->amsdu_state == TX_AMSDU_ID_NO) ||
						(pTxBlk->amsdu_state == TX_AMSDU_ID_LAST))
						tx_idx = pAd->archOps.write_tx_resource(pAd, pTemp_TxBlk, TRUE, &free_cnt);

					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: sta%d, tx idx %d, resource_idx %d\n",
						 __func__, i, tx_idx, pTemp_TxBlk->resource_idx));
				}
			}
			KickRingBitMap |= (1 << pTemp_TxBlk->resource_idx);
		}
	}

CLONE_DONE:
	/* release original pkt */
	if (pAd->vow_need_drop_cnt[pTxBlk->Wcid] > 0) {
		/*printk("\x1b[31m%s release....wcid %d, drop cnt %d\n\x1b[m\n", __FUNCTION__, pTxBlk->Wcid,
		pAd->vow_need_drop_cnt[pTxBlk->Wcid]);*/
		pAd->vow_need_drop_cnt[pTxBlk->Wcid]--;
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_SUCCESS);
	}

	return KickRingBitMap;
}
#endif /* defined(VOW_SUPPORT) && (defined(VOW_DVT) */

VOID ap_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	UCHAR *pHeaderBufPtr;
	BOOLEAN bVLANPkt;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	ap_build_802_11_header(pAd, pTxBlk);
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
	if (pTxBlk->TxFrameType == TX_MCAST_FRAME) {
		INT idx;
#ifdef STATS_COUNT_SUPPORT
		INC_COUNTER64(pAd->WlanCounters[0].MulticastTransmittedFrameCount);
#endif /* STATS_COUNT_SUPPORT */

		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++) {
			if (pAd->ApCfg.MBSSID[idx].WPAREKEY.ReKeyMethod == PKT_REKEY)
				pAd->ApCfg.MBSSID[idx].REKEYCOUNTER += (pTxBlk->SrcBufLen);

		}
	}

#ifdef MT_MAC
	else {
		/* Unicast */
		if (pTxBlk->tr_entry && pTxBlk->tr_entry->PsDeQWaitCnt)
			pTxBlk->Pid = PID_PS_DATA;
	}

#endif /* MT_MAC */
	pHeaderBufPtr = pTxBlk->wifi_hdr;
	wifi_hdr = (HEADER_802_11 *)pHeaderBufPtr;
	/* skip common header */
	pHeaderBufPtr += pTxBlk->wifi_hdr_len;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWMM)) {
		struct wifi_dev *wdev_tmp = NULL;
		UCHAR ack_policy = pAd->CommonCfg.AckPolicy[pTxBlk->QueIdx];
		wdev_tmp = pTxBlk->wdev;
		if (wdev_tmp)
			ack_policy = wlan_config_get_ack_policy(wdev_tmp, pTxBlk->QueIdx);

		/* build QOS Control bytes */
		*pHeaderBufPtr = ((pTxBlk->UserPriority & 0x0F) | (ack_policy << 5));
#if defined(VOW_SUPPORT) && defined(VOW_DVT)
		*pHeaderBufPtr |= (pAd->vow_sta_ack[pTxBlk->Wcid] << 5);
#endif /* defined(VOW_SUPPORT) && (defined(VOW_DVT) */
#ifdef WFA_VHT_PF

		if (pAd->force_noack)
			*pHeaderBufPtr |= (1 << 5);

#endif /* WFA_VHT_PF */
#ifdef UAPSD_SUPPORT
		if (CLIENT_STATUS_TEST_FLAG(pTxBlk->pMacEntry, fCLIENT_STATUS_APSD_CAPABLE)
#ifdef WDS_SUPPORT
			&& (TX_BLK_TEST_FLAG(pTxBlk, fTX_bWDSEntry) == FALSE)
#endif /* WDS_SUPPORT */
		   ) {
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
		pTxBlk->wifi_hdr_len += 2;

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_LOUD, ("%s: tx_bf: %d\n",
					 __func__, cap->FlgHwTxBfCap));
#ifdef TXBF_SUPPORT
#ifndef MT_MAC

		if (cap->FlgHwTxBfCap &&
			(pTxBlk->pMacEntry) &&
			(pTxBlk->pTransmit->field.MODE >= MODE_HTMIX)) {
			MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;
			BOOLEAN bHTCPlus = FALSE;

			pTxBlk->TxSndgPkt = SNDG_TYPE_DISABLE;
			/* TODO: shiang-usw, fix ME!! */
			NdisAcquireSpinLock(&pMacEntry->TxSndgLock);

			if (pMacEntry->TxSndgType >= SNDG_TYPE_SOUNDING) {
				NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));

				if (pMacEntry->TxSndgType == SNDG_TYPE_SOUNDING) {
					/* Select compress if supported. Otherwise select noncompress */
					if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
						(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0))
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
					else
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

					/* Clear NDP Announcement */
					((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 0;
				} else if (pMacEntry->TxSndgType == SNDG_TYPE_NDP) {
					/* Select compress if supported. Otherwise select noncompress */
					if ((pAd->CommonCfg.ETxBfNoncompress == 0) &&
						(pMacEntry->HTCapability.TxBFCap.ExpComBF > 0) &&
						(pMacEntry->HTCapability.TxBFCap.ComSteerBFAntSup >= (pMacEntry->sndgMcs/8))
					   )
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 3;
					else
						((PHT_CONTROL)pHeaderBufPtr)->CSISTEERING = 2;

					/* Set NDP Announcement */
					((PHT_CONTROL)pHeaderBufPtr)->NDPAnnounce = 1;
					pTxBlk->TxNDPSndgBW = pMacEntry->sndgBW;
					pTxBlk->TxNDPSndgMcs = pMacEntry->sndgMcs;
				}

				pTxBlk->TxSndgPkt = pMacEntry->TxSndgType;
				pMacEntry->TxSndgType = SNDG_TYPE_DISABLE;
				bHTCPlus = TRUE;
			}

			NdisReleaseSpinLock(&pMacEntry->TxSndgLock);
#ifdef MFB_SUPPORT
#if defined(MRQ_FORCE_TX)
			/* have to replace this by the correct condition!!! */
			pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback = MCSFBK_MRQ;
#endif

			/*
				Ignore sounding frame because the signal format of sounding frmae may
				be different from normal data frame, which may result in different MFB
			*/
			if ((pMacEntry->HTCapability.ExtHtCapInfo.MCSFeedback >= MCSFBK_MRQ) &&
				(pTxBlk->TxSndgPkt == SNDG_TYPE_DISABLE)) {
				if (bHTCPlus == FALSE) {
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
					bHTCPlus = TRUE;
				}

				MFB_PerPareMRQ(pAd, pHeaderBufPtr, pMacEntry);
			}

			if (pAd->CommonCfg.HtCapability.ExtHtCapInfo.MCSFeedback >= MCSFBK_MRQ &&
				pMacEntry->toTxMfb == 1) {
				if (bHTCPlus == FALSE) {
					NdisZeroMemory(pHeaderBufPtr, sizeof(HT_CONTROL));
					bHTCPlus = TRUE;
				}

				MFB_PerPareMFB(pAd, pHeaderBufPtr, pMacEntry);/* not complete yet!!! */
				pMacEntry->toTxMfb = 0;
			}

#endif /* MFB_SUPPORT */

			if (bHTCPlus == TRUE) {
				/* mark HTC bit */
				wifi_hdr->FC.Order = 1;
				pHeaderBufPtr += 4;
				pTxBlk->wifi_hdr_len += 4;
			}
		}

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
	}

	/* The remaining content of MPDU header should locate at 4-octets aligment */
	pTxBlk->HdrPadLen = (ULONG)pHeaderBufPtr;
	pHeaderBufPtr = (UCHAR *)ROUND_UP(pHeaderBufPtr, 4);
	pTxBlk->HdrPadLen = (ULONG)(pHeaderBufPtr - pTxBlk->HdrPadLen);
	pTxBlk->MpduHeaderLen = pTxBlk->wifi_hdr_len;
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt))
		tx_sw_encrypt(pAd, pTxBlk, pHeaderBufPtr, wifi_hdr);
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
}

VOID ap_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	pTxBlk->MpduHeaderLen = 0;
	pTxBlk->HdrPadLen = 0;
	pTxBlk->wifi_hdr_len = 0;
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
}

INT ap_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	QUEUE_ENTRY *pQEntry;
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *wdev_ops = wdev->wdev_ops;

	ASSERT(pTxBlk);
	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {
		ap_tx_drop_update(pAd, wdev, pTxBlk);
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS))
		wdev_ops->ieee_802_3_data_tx(pAd, wdev, pTxBlk);
	else
		wdev_ops->ieee_802_11_data_tx(pAd, wdev, pTxBlk);

	ap_tx_ok_update(pAd, wdev, pTxBlk);

	ret = arch_ops->hw_tx(pAd, pTxBlk);

	if (ret != NDIS_STATUS_SUCCESS)
		return ret;

	if (RTMP_GET_PACKET_LOWRATE(pTxBlk->pPacket))
		if (pTxBlk->tr_entry)
			pTxBlk->tr_entry->isCached = FALSE;

	pAd->RalinkCounters.KickTxCount++;
	pAd->RalinkCounters.OneSecTxDoneCount++;
	return NDIS_STATUS_SUCCESS;
}

INT ap_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	HEADER_802_11 *wifi_hdr;
	QUEUE_ENTRY *pQEntry;
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
	RTMP_ARCH_OP *op = &pAd->archOps;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	ASSERT(pTxBlk);
	pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
	pTxBlk->pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

	if (!fill_tx_blk(pAd, wdev, pTxBlk)) {

		ap_tx_drop_update(pAd, wdev, pTxBlk);
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<--%s(%d): ##########Fail#########\n", __func__, __LINE__));
		return NDIS_STATUS_SUCCESS;
	}

#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		if (RTMPExpandPacketForSwEncrypt(pAd, pTxBlk) == FALSE) {
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}
	}

#endif /* SOFT_ENCRYPT */

	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg)) {
		pTxBlk->pPacket = duplicate_pkt_with_TKIP_MIC(pAd, pTxBlk->pPacket);

		if (pTxBlk->pPacket == NULL)
			return NDIS_STATUS_FAILURE;

		RTMP_QueryPacketInfo(pTxBlk->pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
	}

	ops->ieee_802_11_data_tx(pAd, wdev, pTxBlk);

	/*  1. If TKIP is used and fragmentation is required. Driver has to
		   append TKIP MIC at tail of the scatter buffer
		2. When TXWI->FRAG is set as 1 in TKIP mode,
		   MAC ASIC will only perform IV/EIV/ICV insertion but no TKIP MIC */
	/*  TKIP appends the computed MIC to the MSDU data prior to fragmentation into MPDUs. */
	if (IS_CIPHER_TKIP(pTxBlk->CipherAlg)) {
		RTMPCalculateMICValue(pAd, pTxBlk->pPacket, pTxBlk->pExtraLlcSnapEncap, pTxBlk->pKey, &pTxBlk->pKey[LEN_TK], wdev->func_idx);
		/*
			NOTE: DON'T refer the skb->len directly after following copy. Becasue the length is not adjust
				to correct lenght, refer to pTxBlk->SrcBufLen for the packet length in following progress.
		*/
		NdisMoveMemory(pTxBlk->pSrcBufData + pTxBlk->SrcBufLen, &pAd->PrivateInfo.Tx.MIC[0], 8);
		pTxBlk->SrcBufLen += 8;
		pTxBlk->TotalFrameLen += 8;
	}

	ap_tx_ok_update(pAd, wdev, pTxBlk);

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

	/*MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("!!!Fragment AckDuration(%d), TxRate(%d)!!!\n", AckDuration, pTxBlk->TxRate)); */
#ifdef SOFT_ENCRYPT

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
		/* store the outgoing frame for calculating MIC per fragmented frame */
		os_alloc_mem(pAd, (PUCHAR *)&tmp_ptr, pTxBlk->SrcBufLen);

		if (tmp_ptr == NULL) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():no memory for MIC calculation!\n",
					 __func__));
			RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
		}

		NdisMoveMemory(tmp_ptr, pTxBlk->pSrcBufData, pTxBlk->SrcBufLen);
	}

#endif /* SOFT_ENCRYPT */
	/* Init the total payload length of this frame. */
	SrcRemainingBytes = pTxBlk->SrcBufLen;
	pTxBlk->TotalFragNum = 0xff;
	wifi_hdr = (HEADER_802_11 *)pTxBlk->wifi_hdr;

	do {
		FreeMpduSize = wlan_operate_get_frag_thld(&pTxBlk->pMbss->wdev);
		FreeMpduSize -= LENGTH_CRC;
		FreeMpduSize -= pTxBlk->MpduHeaderLen;

		if (SrcRemainingBytes <= FreeMpduSize) {
			/* This is the last or only fragment */
			pTxBlk->SrcBufLen = SrcRemainingBytes;
			wifi_hdr->FC.MoreFrag = 0;
			wifi_hdr->Duration = pAd->CommonCfg.Dsifs + AckDuration;
			/* Indicate the lower layer that this's the last fragment. */
			pTxBlk->TotalFragNum = fragNum;
#ifdef MT_MAC
			pTxBlk->FragIdx = ((fragNum == 0) ? TX_FRAG_ID_NO : TX_FRAG_ID_LAST);
#endif /* MT_MAC */
		} else {
			/* more fragment is required */
			pTxBlk->SrcBufLen = FreeMpduSize;
			NextMpduSize = min(((UINT)SrcRemainingBytes - pTxBlk->SrcBufLen),
							   ((UINT)wlan_operate_get_frag_thld(&pTxBlk->pMbss->wdev)));
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

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
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
		ret = op->hw_tx(pAd, pTxBlk);

		if (ret != NDIS_STATUS_SUCCESS)
			return ret;

#ifdef SMART_ANTENNA

		if (pTxBlk->pMacEntry)
			pTxBlk->pMacEntry->saTxCnt++;

#endif /* SMART_ANTENNA */
		pAd->RalinkCounters.KickTxCount++;
		pAd->RalinkCounters.OneSecTxDoneCount++;
#ifdef SOFT_ENCRYPT

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bSwEncrypt)) {
				if ((pTxBlk->CipherAlg == CIPHER_WEP64) || (pTxBlk->CipherAlg == CIPHER_WEP128)) {
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WEP_TSC, 1);
					/* Construct and insert 4-bytes WEP IV header to MPDU header */
					RTMPConstructWEPIVHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										  pHeaderBufPtr - (LEN_WEP_IV_HDR));
				} else if (pTxBlk->CipherAlg == CIPHER_TKIP)
					;
				else if (pTxBlk->CipherAlg == CIPHER_AES) {
					inc_iv_byte(pTxBlk->pKey->TxTsc, LEN_WPA_TSC, 1);
					/* Construct and insert 8-bytes CCMP header to MPDU header */
					RTMPConstructCCMPHdr(pTxBlk->KeyIdx, pTxBlk->pKey->TxTsc,
										 pHeaderBufPtr - (LEN_CCMP_HDR));
				}
		} else
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
		os_free_mem(tmp_ptr);

#endif /* SOFT_ENCRYPT */
	return NDIS_STATUS_SUCCESS;
}

INT ap_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PQUEUE_ENTRY pQEntry;
	PNDIS_PACKET pPacket = NULL;
	struct wifi_dev_ops *ops = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT32 ret = NDIS_STATUS_SUCCESS;
	struct DOT11_H *pDot11h = NULL;

	if (!wdev) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

	ops = wdev->wdev_ops;
	pDot11h = wdev->pDot11_H;

	if (pDot11h == NULL)
		return NDIS_STATUS_FAILURE;

	if ((pDot11h->RDMode != RD_NORMAL_MODE)
#ifdef CARRIER_DETECTION_SUPPORT
		|| (isCarrierDetectExist(pAd) == TRUE)
#endif /* CARRIER_DETECTION_SUPPORT */
	   ) {
		while (pTxBlk->TxPacketList.Head) {
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("<--%s(%d)\n", __func__, __LINE__));
		return NDIS_STATUS_FAILURE;
	}

	if (wdev->bVLAN_Tag == TRUE)
		RTMP_SET_PACKET_VLAN(pTxBlk->pPacket, FALSE);

#ifdef DOT11K_RRM_SUPPORT
#ifdef QUIET_SUPPORT

	if (IS_RRM_QUIET(wdev)) {
		RELEASE_NDIS_PACKET(pAd, pTxBlk->pPacket, NDIS_STATUS_FAILURE);
		return NDIS_STATUS_FAILURE;
	}

#endif /* QUIET_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

	switch (pTxBlk->TxFrameType) {
	case TX_AMPDU_FRAME:
		ret = ops->ampdu_tx(pAd, wdev, pTxBlk);
		break;

	case TX_LEGACY_FRAME:
		ret = ops->legacy_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MCAST_FRAME:
		ret = ops->legacy_tx(pAd, wdev, pTxBlk);
		break;

	case TX_AMSDU_FRAME:
		ret = ops->amsdu_tx(pAd, wdev, pTxBlk);
		break;

	case TX_FRAG_FRAME:
		ret = ops->frag_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MLME_MGMTQ_FRAME:
		ret = ops->mlme_mgmtq_tx(pAd, wdev, pTxBlk);
		break;

	case TX_MLME_DATAQ_FRAME:
		ret = ops->mlme_dataq_tx(pAd, wdev, pTxBlk);
		break;
#ifdef CONFIG_ATE

	case TX_ATE_FRAME:
		ret = ops->ate_tx(pAd, wdev, pTxBlk);
		break;
#endif /* CONFIG_ATE */

	default:
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Send a pacekt was not classified!!\n"));

		while (pTxBlk->TxPacketList.Head) {
			pQEntry = RemoveHeadQueue(&pTxBlk->TxPacketList);
			pPacket = QUEUE_ENTRY_TO_PACKET(pQEntry);

			if (pPacket)
				RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
		}

		return NDIS_STATUS_FAILURE;
	}

	pEntry = &pAd->MacTab.Content[(UCHAR)RTMP_GET_PACKET_WCID(pTxBlk->pPacket)];
	INC_COUNTER64(pEntry->TxPackets);
	pEntry->TxBytes += pTxBlk->SrcBufLen;
	pEntry->OneSecTxBytes += pTxBlk->SrcBufLen;
	pEntry->one_sec_tx_pkts++;
	return ret;
}

/*
  ========================================================================
  Description:
	This routine checks if a received frame causes class 2 or class 3
	error, and perform error action (DEAUTH or DISASSOC) accordingly
  ========================================================================
*/
BOOLEAN ap_chk_cl2_cl3_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	/* software MAC table might be smaller than ASIC on-chip total size. */
	/* If no mathed wcid index in ASIC on chip, do we need more check???  need to check again. 06-06-2006 */
	if (!VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
		MAC_TABLE_ENTRY *pEntry;

		pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

		if (pEntry)
			return FALSE;

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Rx a frame from %02x:%02x:%02x:%02x:%02x:%02x with WCID(%d) > %d\n",
				 __func__, PRINT_MAC(pRxBlk->Addr2),
				 pRxBlk->wcid, GET_MAX_UCAST_NUM(pAd)));
		APCls2errAction(pAd, pRxBlk);
		return TRUE;
	}

	if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_ASSOC)
		/* okay to receive this DATA frame */
		return FALSE;
	else if (pAd->MacTab.Content[pRxBlk->wcid].Sst == SST_AUTH) {
		APCls3errAction(pAd, pRxBlk);
		return TRUE;
	}
	APCls2errAction(pAd, pRxBlk);
	return TRUE;
}

VOID ap_rx_error_handle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;

	if (pRxInfo->CipherErr) {
		INC_COUNTER64(pAd->WlanCounters[0].WEPUndecryptableCount);

		if ((pRxInfo->U2M) && VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid)) {
#ifdef APCLI_SUPPORT
#if defined(APCLI_CERT_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT)
			UCHAR Wcid;

			Wcid = pRxBlk->wcid;

			if (VALID_UCAST_ENTRY_WCID(pAd, Wcid))
				pEntry = ApCliTableLookUpByWcid(pAd, Wcid, pRxBlk->Addr2);
			else
				pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

			if (pEntry &&
				(IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
				 || IS_ENTRY_REPEATER(pEntry)
#endif /* MAC_REPEATER_SUPPORT */
				)) {
				if ((IS_CIPHER_TKIP_Entry(pEntry))
					&& (pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH))) {
					ApCliRTMPReportMicError(pAd, 1, pEntry->func_tb_idx);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Rx MIC Value error\n"));
				}
			} else
#endif /* defined(APCLI_CERT_SUPPORT) || defined(WPA_SUPPLICANT_SUPPORT) */
#endif /* APCLI_SUPPORT */
			{
				pEntry = &pAd->MacTab.Content[pRxBlk->wcid];

				/*
					MIC error
					Before verifying the MIC, the receiver shall check FCS, ICV and TSC.
					This avoids unnecessary MIC failure events.
				*/
				if ((IS_CIPHER_TKIP_Entry(pEntry))
					&& (pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH))) {
#ifdef HOSTAPD_SUPPORT

					if (pAd->ApCfg.MBSSID[pEntry->func_tb_idx].Hostapd == Hostapd_EXT)
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
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx u2me Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
					 pRxBlk->MPDUtotalByteCnt, pRxBlk->wcid, pRxInfo->CipherErr));
		}

#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
		else if (pRxInfo->Mcast || pRxInfo->Bcast) {
			pEntry = MacTableLookup(pAd, pRxBlk->Addr2);

			if (pEntry &&
				(IS_ENTRY_APCLI(pEntry)
#ifdef MAC_REPEATER_SUPPORT
				 || IS_ENTRY_REPEATER(pEntry)
#endif /* MAC_REPEATER_SUPPORT */
				)) {
				if ((pRxInfo->CipherErr == 2)
					&& !(RX_BLK_TEST_FLAG(pRxBlk, fRX_WCID_MISMATCH))) {
					ApCliRTMPReportMicError(pAd, 0, pEntry->func_tb_idx);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Rx MIC Value error\n"));
				}
			}

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Rx bc/mc Cipher Err(MPDUsize=%d, WCID=%d, CipherErr=%d)\n",
					 pRxBlk->MPDUtotalByteCnt, pRxBlk->wcid, pRxInfo->CipherErr));
		}

#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
	}
}

#ifdef RLT_MAC_DBG
static int dump_next_valid;
#endif /* RLT_MAC_DBG */
BOOLEAN ap_check_valid_frame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN isVaild = FALSE;

	do {
		if (FC->ToDs == 0)
			break;

#ifdef IDS_SUPPORT

		if ((FC->FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) { /* not in RX WCID MAC table */
			if (++pAd->ApCfg.RcvdMaliciousDataCount > pAd->ApCfg.DataFloodThreshold)
				break;
		}

#endif /* IDS_SUPPORT */

		/* check if Class2 or 3 error */
		if ((FC->FrDs == 0) && (ap_chk_cl2_cl3_err(pAd, pRxBlk)))
			break;

		if (pAd->ApCfg.BANClass3Data == TRUE)
			break;

		isVaild = TRUE;
	} while (0);

	return isVaild;
}

INT ap_rx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = 0;

	pEntry = PACInquiry(pAd, pRxBlk->wcid);
#if defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(A4_CONN)

	if ((pFmeCtrl->FrDs == 1) && (pFmeCtrl->ToDs == 1)) {
#ifdef CLIENT_WDS

		if (pEntry) {
			/* The CLIENT WDS must be a associated STA */
			if (IS_ENTRY_CLIWDS(pEntry))
				;
			else if (IS_ENTRY_CLIENT(pEntry) && (pEntry->Sst == SST_ASSOC))
				SET_ENTRY_CLIWDS(pEntry);
			else
				return FALSE;

			CliWds_ProxyTabUpdate(pAd, pEntry->Aid, pRxBlk->Addr4);
		}

#endif /* CLIENT_WDS */


#ifdef WDS_SUPPORT

		if (!pEntry) {
			/*
				The WDS frame only can go here when in auto learning mode and
				this is the first trigger frame from peer

				So we check if this is un-registered WDS entry by call function
					"FindWdsEntry()"
			*/
			if (MAC_ADDR_EQUAL(pRxBlk->Addr1, pAd->CurrentAddress))
				pEntry = FindWdsEntry(pAd, pRxBlk);

			/* have no valid wds entry exist, then discard the incoming packet.*/
			if (!(pEntry && WDS_IF_UP_CHECK(pAd, pEntry->func_tb_idx)))
				return FALSE;

			/*receive corresponding WDS packet, disable TX lock state (fix WDS jam issue) */
			if (pEntry && (pEntry->LockEntryTx == TRUE)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Receive WDS packet, disable TX lock state!\n"));
				pEntry->ContinueTxFailCnt = 0;
				pEntry->LockEntryTx = FALSE;
				/* TODO: shiang-usw, remove upper setting because we need to mirgate to tr_entry! */
				pAd->MacTab.tr_entry[pEntry->wcid].ContinueTxFailCnt = 0;
				pAd->MacTab.tr_entry[pEntry->wcid].LockEntryTx = FALSE;
			}
		}

#endif /* WDS_SUPPORT */

		if (pEntry) {
#ifdef WDS_SUPPORT
#ifdef STATS_COUNT_SUPPORT
			if (IS_ENTRY_WDS(pEntry)) {
				RT_802_11_WDS_ENTRY *pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

				pWdsEntry->WdsCounter.ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
				INC_COUNTER64(pWdsEntry->WdsCounter.ReceivedFragmentCount);

				if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
					INC_COUNTER64(pWdsEntry->WdsCounter.MulticastReceivedFrameCount);
			}
#endif /* STATS_COUNT_SUPPORT */
#endif /* WDS_SUPPORT */
			RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
			hdr_len = LENGTH_802_11_WITH_ADDR4;
			return hdr_len;
		}

	}

#endif /* defined(WDS_SUPPORT) || defined(CLIENT_WDS) || defined(A4_CONN) */

	if (!pEntry) {
#ifdef IDS_SUPPORT

		if ((pFmeCtrl->FrDs == 0) && (pRxBlk->wcid == RESERVED_WCID)) /* not in RX WCID MAC table */
			pAd->ApCfg.RcvdMaliciousDataCount++;

#endif /* IDS_SUPPORT */
		return FALSE;
	}

	if (!((pFmeCtrl->FrDs == 0) && (pFmeCtrl->ToDs == 1))) {
#ifdef IDS_SUPPORT

		/*
			Replay attack detection,
			drop it if detect a spoofed data frame from a rogue AP
		*/
		if (pFmeCtrl->FrDs == 1)
			RTMPReplayAttackDetection(pAd, pRxBlk->Addr2, pRxBlk);

#endif /* IDS_SUPPORT */
		return FALSE;
	}


	/* check if Class2 or 3 error */
	if (ap_chk_cl2_cl3_err(pAd, pRxBlk))
		return FALSE;

	if (pAd->ApCfg.BANClass3Data == TRUE)
		return FALSE;

#ifdef STATS_COUNT_SUPPORT

	/* Increase received byte counter per BSS */
	if (pFmeCtrl->FrDs == 0 && pRxInfo->U2M) {
		BSS_STRUCT *pMbss = pEntry->pMbss;

		if (pMbss != NULL) {
			pMbss->ReceivedByteCount += pRxBlk->MPDUtotalByteCnt;
			pMbss->RxCount++;
		}
	}

	if (IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3))
		INC_COUNTER64(pAd->WlanCounters[0].MulticastReceivedFrameCount);

#endif /* STATS_COUNT_SUPPORT */


	hdr_len = LENGTH_802_11;
	RX_BLK_SET_FLAG(pRxBlk, fRX_STA);
	ASSERT(pEntry->wcid == pRxBlk->wcid);
	return hdr_len;
}

INT ap_rx_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[pRxBlk->wcid];
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
	UCHAR OldPwrMgmt = PWR_ACTIVE; /* 1: PWR_SAVE, 0: PWR_ACTIVE */
	/* 1: PWR_SAVE, 0: PWR_ACTIVE */
	OldPwrMgmt = RtmpPsIndicate(pAd, pRxBlk->Addr2, pEntry->wcid, FC->PwrMgmt);
#ifdef UAPSD_SUPPORT

	if ((FC->PwrMgmt == PWR_SAVE) &&
		(OldPwrMgmt == PWR_SAVE) &&
		(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_APSD_CAPABLE)) &&
		(FC->SubType & 0x08)) {
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

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			OldUP = (*(pRxBlk->pData + 32) & 0x07);
		else
			OldUP = (*(pRxBlk->pData + LENGTH_802_11) & 0x07);

		UAPSD_TriggerFrameHandle(pAd, pEntry, OldUP);
	}

#endif /* UAPSD_SUPPORT */
	return TRUE;
}

INT ap_rx_pkt_foward(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	BOOLEAN to_os, to_air;
	UCHAR *pHeader802_3;
	PNDIS_PACKET pForwardPacket;
	BSS_STRUCT *pMbss;
	struct wifi_dev *dst_wdev = NULL;
	UCHAR wcid;

	if (wdev->func_idx >= HW_BEACON_MAX_NUM) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Invalid func_idx(%d), type(%d)!\n",
				 __func__, wdev->func_idx, wdev->wdev_type));
		return FALSE;
	}

	/* only one connected sta, directly to upper layer */
	if (pAd->MacTab.Size <= 1)
		return TRUE;

	/* TODO: shiang-usw, remove pMbss structure here to make it more generic! */
	pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
	pHeader802_3 = GET_OS_PKT_DATAPTR(pPacket);
	/* by default, announce this pkt to upper layer (bridge) and not to air */
	to_os = TRUE;
	to_air = FALSE;

	if (pHeader802_3[0] & 0x01) {
		if ((pMbss->StaCount > 1)
		   ) {
			/* forward the M/Bcast packet back to air if connected STA > 1 */
			to_air = TRUE;
		}
	} else {
		/* if destinated STA is a associated wireless STA */
		pEntry = MacTableLookup(pAd, pHeader802_3);

		if (pEntry && pEntry->Sst == SST_ASSOC && pEntry->wdev) {
			dst_wdev = pEntry->wdev;

			if (wdev == dst_wdev) {
				/*
					STAs in same SSID, default send to air and not to os,
					but not to air if match following case:
						a). pMbss->IsolateInterStaTraffic == TRUE
				*/
				to_air = TRUE;
				to_os = FALSE;

				if (pMbss->IsolateInterStaTraffic == 1)
					to_air = FALSE;
			} else {
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
		}
	}

	if (to_air) {

		pForwardPacket = DuplicatePacket(wdev->if_dev, pPacket);
#ifdef RTMP_UDMA_SUPPORT
	if (to_os) {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		if ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE))
			set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */
			announce_802_3_packet(pAd, pPacket, pAd->OpMode);
	}
#endif
		if (pForwardPacket == NULL)
			return to_os;

		/* 1.1 apidx != 0, then we need set packet mbssid attribute. */
		if (pEntry) {
			wcid = pEntry->wcid;
			RTMP_SET_PACKET_WDEV(pForwardPacket, dst_wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		} else { /* send bc/mc frame back to the same bss */
			wcid = wdev->tr_tb_idx;
			RTMP_SET_PACKET_WDEV(pForwardPacket, wdev->wdev_idx);
			RTMP_SET_PACKET_WCID(pForwardPacket, wcid);
		}

		RTMP_SET_PACKET_MOREDATA(pForwardPacket, FALSE);


#ifdef REDUCE_TCP_ACK_SUPPORT
		ReduceAckUpdateDataCnx(pAd, pForwardPacket);

		if (ReduceTcpAck(pAd, pForwardPacket) == FALSE)
#endif
		{
			send_data_pkt(pAd, wdev, pForwardPacket);
		}
	}
#ifdef RTMP_UDMA_SUPPORT
	if (to_os == FALSE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): No need to send to OS!\n", __func__));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
#ifdef CUT_THROUGH_DBG
		pAd->RxDropPacket++;
#endif
	}
	return (to_os & (!to_air));
#endif

	return to_os;
}

INT ap_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	UCHAR wdev_idx = BSS0;
	BOOLEAN bFragment = FALSE;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	wdev_idx = wdev->wdev_idx;
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 __func__, pEntry->wcid, wdev->wdev_idx,
			 pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			 pFmeCtrl->Type, pFmeCtrl->SubType,
			 pFmeCtrl->FrDs, pFmeCtrl->ToDs));

	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
	/* must be here, before no DATA check */
	if (ops->rx_ps_handle)
		ops->rx_ps_handle(pAd, wdev, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
	pEntry->OneSecRxBytes += pRxBlk->MPDUtotalByteCnt;
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
	INC_COUNTER64(pEntry->RxPackets);
	pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;

	if (((FRAME_CONTROL *)pRxBlk->FC)->SubType & 0x08) {

		if (pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;

		if (pRxBlk->AmsduState)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);

		if (pRxInfo->BA)
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
	}

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): duplicate frame drop it!\n", __func__));
		return FALSE;
	}

	if ((pRxBlk->FN == 0) && (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxInfo->U2M)
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);

#ifdef IGMP_SNOOP_SUPPORT

	if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
		&& (wdev->IgmpSnoopEnable)
		&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
		PUCHAR pDA = pRxBlk->Addr3;
		PUCHAR pSA = pRxBlk->Addr2;
		PUCHAR pData = pRxBlk->pData + 12;
		UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

		if (protoType == ETH_P_IP)
			IGMPSnooping(pAd, pDA, pSA, pData, wdev, pRxBlk->wcid);
		else if (protoType == ETH_P_IPV6)
			MLDSnooping(pAd, pDA, pSA,  pData, wdev, pRxBlk->wcid);
	}

#endif /* IGMP_SNOOP_SUPPORT */

	if (pRxBlk->pRxPacket) {
		RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);
		rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, pEntry->wdev);
	}

	return TRUE;
}

INT ap_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	FRAME_CONTROL *pFmeCtrl = (FRAME_CONTROL *)pRxBlk->FC;
	BOOLEAN bFragment = FALSE;
	UCHAR wdev_idx = BSS0;
	UCHAR UserPriority = 0;
	INT hdr_len = LENGTH_802_11;
	COUNTER_RALINK *pCounter = &pAd->RalinkCounters;
	UCHAR *pData;
	BOOLEAN drop_err = TRUE;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)
	NDIS_STATUS status;
#endif /* defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT) */
	struct wifi_dev_ops *ops = wdev->wdev_ops;

	wdev_idx = wdev->wdev_idx;
	MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): wcid=%d, wdev_idx=%d, pRxBlk->Flags=0x%x, fRX_AP/STA/ADHOC=0x%x/0x%x/0x%x, Type/SubType=%d/%d, FrmDS/ToDS=%d/%d\n",
			 __func__, pEntry->wcid, wdev->wdev_idx,
			 pRxBlk->Flags,
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_AP),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_STA),
			 RX_BLK_TEST_FLAG(pRxBlk, fRX_ADHOC),
			 pFmeCtrl->Type, pFmeCtrl->SubType,
			 pFmeCtrl->FrDs, pFmeCtrl->ToDs));
	/* Gather PowerSave information from all valid DATA frames. IEEE 802.11/1999 p.461 */
	/* must be here, before no DATA check */
	pData = pRxBlk->FC;

	if (ops->rx_ps_handle)
		ops->rx_ps_handle(pAd, wdev, pRxBlk);

	pEntry->NoDataIdleCount = 0;
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
	/*
		update RxBlk->pData, DataSize, 802.11 Header, QOS, HTC, Hw Padding
	*/


	pData = pRxBlk->FC;
	/* 1. skip 802.11 HEADER */
	pData += hdr_len;
	pRxBlk->DataSize -= hdr_len;

	/* 2. QOS */
	if (pFmeCtrl->SubType & 0x08) {
		UserPriority = *(pData) & 0x0f;

		if (pAd->MacTab.Content[pRxBlk->wcid].BARecWcidArray[pRxBlk->TID] != 0)
			pRxInfo->BA = 1;
		else
			pRxInfo->BA = 0;


		/* bit 7 in QoS Control field signals the HT A-MSDU format */
		if ((*pData) & 0x80) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMSDU);
			pCounter->RxAMSDUCount.u.LowPart++;
		}

		if (pRxInfo->BA) {
			RX_BLK_SET_FLAG(pRxBlk, fRX_AMPDU);
			/* incremented by the number of MPDUs */
			/* received in the A-MPDU when an A-MPDU is received. */
			pCounter->MPDUInReceivedAMPDUCount.u.LowPart++;
		}

		/* skip QOS contorl field */
		pData += 2;
		pRxBlk->DataSize -= 2;
	}

	pRxBlk->UserPriority = UserPriority;

	/*check if duplicate frame, ignore it and then drop*/
	if (rx_chk_duplicate_frame(pAd, pRxBlk, wdev) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): duplicate frame drop it!\n", __func__));
		return FALSE;
	}

	/* 3. Order bit: A-Ralink or HTC+ */
	if (pFmeCtrl->Order) {
#ifdef AGGREGATION_SUPPORT

		/* TODO: shiang-MT7603, fix me, because now we don't have rx_rate.field.MODE can refer */
		if ((pRxBlk->rx_rate.field.MODE <= MODE_OFDM) &&
			(CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE)))
			RX_BLK_SET_FLAG(pRxBlk, fRX_ARALINK);
		else
#endif /* AGGREGATION_SUPPORT */
		{
#ifdef TXBF_SUPPORT
#ifndef MT_MAC
			struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

			if (cap->FlgHwTxBfCap && (pFmeCtrl->SubType & 0x08))
				handleHtcField(pAd, pRxBlk);

#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
			/* skip HTC control field */
			pData += 4;
			pRxBlk->DataSize -= 4;
		}
	}

	/* Drop NULL, CF-ACK(no data), CF-POLL(no data), and CF-ACK+CF-POLL(no data) data frame */
	if (pFmeCtrl->SubType & 0x04) { /* bit 2 : no DATA */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Null/QosNull frame!\n", __func__));
		drop_err = FALSE;
		return FALSE;
	}

	/* 4. skip HW padding */
	if (pRxInfo->L2PAD) {
		/* just move pData pointer because DataSize excluding HW padding */
		RX_BLK_SET_FLAG(pRxBlk, fRX_PAD);
		pData += 2;
	}

	pRxBlk->pData = pData;
#if defined(SOFT_ENCRYPT) || defined(ADHOC_WPA2PSK_SUPPORT)

	/* Use software to decrypt the encrypted frame if necessary.
	   If a received "encrypted" unicast packet(its WEP bit as 1)
	   and it's passed to driver with "Decrypted" marked as 0 in RxInfo.
	*/
	if (!IS_HIF_TYPE(pAd, HIF_MT)) {
		if ((pFmeCtrl->Wep == 1) && (pRxInfo->Decrypted == 0)) {
#ifdef HDR_TRANS_SUPPORT

			if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS)) {
				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  &pEntry->PairwiseKey,
												  pRxBlk->pTransData + 14,
												  &(pRxBlk->TransDataSize));
			} else
#endif /* HDR_TRANS_SUPPORT */
			{
				CIPHER_KEY *pSwKey = &pEntry->PairwiseKey;

				status = RTMPSoftDecryptionAction(pAd,
												  pRxBlk->FC,
												  UserPriority,
												  pSwKey,
												  pRxBlk->pData,
												  &(pRxBlk->DataSize));
			}

			if (status != NDIS_STATUS_SUCCESS) {
				RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket, NDIS_STATUS_FAILURE);
				return;
			}

			/* Record the Decrypted bit as 1 */
			pRxInfo->Decrypted = 1;
		}
	}

#endif /* SOFT_ENCRYPT || ADHOC_WPA2PSK_SUPPORT */
#ifdef SMART_ANTENNA

	if (RTMP_SA_WORK_ON(pAd))
		sa_pkt_radio_info_update(pAd, pRxBlk, pEntry);

#endif /* SMART_ANTENNA */
	Update_Rssi_Sample(pAd, &pEntry->RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
	pEntry->NoDataIdleCount = 0;
	/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
	pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;

	if (pRxInfo->U2M) {
		Update_Rssi_Sample(pAd, &pAd->ApCfg.RssiSample, &pRxBlk->rx_signal, pRxBlk->rx_rate.field.MODE, pRxBlk->rx_rate.field.BW);
		pAd->ApCfg.NumOfAvgRssiSample++;
		pEntry->LastRxRate = (ULONG)(pRxBlk->rx_rate.word);
#ifdef TXBF_SUPPORT

		if (pRxBlk->rx_rate.field.ShortGI)
			pEntry->OneSecRxSGICount++;
		else
			pEntry->OneSecRxLGICount++;

#endif /* TXBF_SUPPORT */
#ifdef DBG_DIAGNOSE

		if (pAd->DiagStruct.inited) {
			struct dbg_diag_info *diag_info;

			diag_info = &pAd->DiagStruct.diag_info[pAd->DiagStruct.ArrayCurIdx];
			diag_info->RxDataCnt++;
		}

#endif /* DBG_DIAGNOSE */
	}

	wdev->LastSNR0 = (UCHAR)(pRxBlk->rx_signal.raw_snr[0]);
	wdev->LastSNR1 = (UCHAR)(pRxBlk->rx_signal.raw_snr[1]);
	pEntry->freqOffset = (CHAR)(pRxBlk->rx_signal.freq_offset);
	pEntry->freqOffsetValid = TRUE;

	if ((pRxBlk->FN != 0) || (pFmeCtrl->MoreFrag != 0)) {
		bFragment = TRUE;
		de_fragment_data_pkt(pAd, pRxBlk);
	}

	if (pRxBlk->pRxPacket) {
		/*
			process complete frame which encrypted by TKIP,
			Minus MIC length and calculate the MIC value
		*/
		if (bFragment && (pFmeCtrl->Wep) && IS_CIPHER_TKIP_Entry(pEntry)) {
			pRxBlk->DataSize -= 8;

			if (rtmp_chk_tkip_mic(pAd, pEntry, pRxBlk) == FALSE)
				return TRUE;
		}

		pEntry->RxBytes += pRxBlk->MPDUtotalByteCnt;
		pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
		INC_COUNTER64(pEntry->RxPackets);
		pAd->RxTotalByteCnt += pRxBlk->MPDUtotalByteCnt;
#ifdef MAC_REPEATER_SUPPORT

		if (IS_ENTRY_APCLI(pEntry))
			RTMP_SET_PACKET_WCID(pRxBlk->pRxPacket, pRxBlk->wcid);

#endif /* MAC_REPEATER_SUPPORT */
#ifdef IGMP_SNOOP_SUPPORT

		if ((IS_ENTRY_CLIENT(pEntry) || IS_ENTRY_WDS(pEntry))
			&& (wdev->IgmpSnoopEnable)
			&& IS_MULTICAST_MAC_ADDR(pRxBlk->Addr3)) {
			PUCHAR pDA = pRxBlk->Addr3;
			PUCHAR pSA = pRxBlk->Addr2;
			PUCHAR pData = NdisEqualMemory(SNAP_802_1H, pRxBlk->pData, 6) ? (pRxBlk->pData + 6) : pRxBlk->pData;
			UINT16 protoType = OS_NTOHS(*((UINT16 *)(pData)));

			if (protoType == ETH_P_IP)
				IGMPSnooping(pAd, pDA, pSA, pData, wdev, pRxBlk->wcid);
			else if (protoType == ETH_P_IPV6)
				MLDSnooping(pAd, pDA, pSA,  pData, wdev, pRxBlk->wcid);
		}

#endif /* IGMP_SNOOP_SUPPORT */

		if (RX_BLK_TEST_FLAG(pRxBlk, fRX_HDR_TRANS))
			rx_802_3_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
		else
			rx_data_frm_announce(pAd, pEntry, pRxBlk, wdev);
	}

	return TRUE;
}

BOOLEAN ap_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	INT op_mode = OPMODE_AP;
	FRAME_CONTROL *FC = (FRAME_CONTROL *)pRxBlk->FC;
#ifdef IDS_SUPPORT

	/*
		Check if a rogue AP impersonats our mgmt frame to spoof clients
		drop it if it's a spoofed frame
	*/
	if (RTMPSpoofedMgmtDetection(pAd, pRxBlk))
		return FALSE;

	/* update sta statistics for traffic flooding detection later */
	RTMPUpdateStaMgmtCounter(pAd, FC->SubType);
#endif /* IDS_SUPPORT */

	if (!pRxInfo->U2M) {
		if ((FC->SubType != SUBTYPE_BEACON) && (FC->SubType != SUBTYPE_PROBE_REQ)) {
			BOOLEAN bDrop = TRUE;
#ifdef DOT11W_PMF_SUPPORT
			/* For PMF TEST Plan 5.4.3.1 & 5.4.3.2 */
#ifdef APCLI_SUPPORT

			if (pEntry && ((FC->SubType == SUBTYPE_DISASSOC) || (FC->SubType == SUBTYPE_DEAUTH))) {
				if (IS_ENTRY_APCLI(pEntry))
					bDrop = FALSE;
			}

#endif /* APCLI_SUPPORT */
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT

			if  (pAd->bApCliCertTest == TRUE) {
				if ((FC->SubType == SUBTYPE_ACTION) && (pEntry) && IS_ENTRY_APCLI(pEntry))
					bDrop = FALSE;
			}

#endif /* APCLI_CERT_SUPPOR */
#endif /* APCLI_SUPPORT */
#if  defined(FTM_SUPPORT) || defined(CONFIG_HOTSPOT)

			if (IsPublicActionFrame(pAd, (VOID *)FC))
				bDrop = FALSE;

#endif /* defined(FTM_SUPPORT) || defined(CONFIG_HOTSPOT) */

			if (bDrop == TRUE)
				return FALSE;
		}
	}

	/* Software decrypt WEP data during shared WEP negotiation */
	if ((FC->SubType == SUBTYPE_AUTH) &&
		(FC->Wep == 1) && (pRxInfo->Decrypted == 0)) {
		UCHAR *pMgmt = (PUCHAR)FC;
		UINT16 mgmt_len = pRxBlk->MPDUtotalByteCnt;
		UCHAR DefaultKeyId;

		if (!pEntry) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: SW decrypt WEP data fails - the Entry is empty.\n"));
			return FALSE;
		}

		/* Skip 802.11 header */
		pMgmt += LENGTH_802_11;
		mgmt_len -= LENGTH_802_11;
#ifdef CONFIG_AP_SUPPORT
		DefaultKeyId = pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.PairwiseKeyId;
#endif /*  CONFIG_AP_SUPPORT */

		/* handle WEP decryption */
		if (RTMPSoftDecryptWEP(
				&pAd->ApCfg.MBSSID[pEntry->func_tb_idx].wdev.SecConfig.WepKey[DefaultKeyId],
				pMgmt,
				&mgmt_len) == FALSE) {
			MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR: SW decrypt WEP data fails.\n"));
			return FALSE;
		}

#ifdef RT_BIG_ENDIAN
		/* swap 16 bit fields - Auth Alg No. field */
		*(USHORT *)pMgmt = SWAP16(*(USHORT *)pMgmt);
		/* swap 16 bit fields - Auth Seq No. field */
		*(USHORT *)(pMgmt + 2) = SWAP16(*(USHORT *)(pMgmt + 2));
		/* swap 16 bit fields - Status Code field */
		*(USHORT *)(pMgmt + 4) = SWAP16(*(USHORT *)(pMgmt + 4));
#endif /* RT_BIG_ENDIAN */
		MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Decrypt AUTH seq#3 successfully\n"));
		/* Update the total length */
		pRxBlk->DataSize -= (LEN_WEP_IV_HDR + LEN_ICV);
	}

	if (pEntry && (FC->SubType == SUBTYPE_ACTION)) {
		/* only PM bit of ACTION frame can be set */
		if (((op_mode == OPMODE_AP) && IS_ENTRY_CLIENT(pEntry)) ||
			((op_mode == OPMODE_STA) && (IS_ENTRY_TDLS(pEntry))))
			RtmpPsIndicate(pAd, pRxBlk->Addr2, pRxBlk->wcid, FC->PwrMgmt);

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

	/* Signal in MLME_QUEUE isn't used, therefore take this item to save min SNR. */
	{
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;

		struct wifi_dev *wdev = NULL;

		if (pRxBlk->wcid != RESERVED_WCID)
			wdev =  pAd->MacTab.Content[pRxBlk->wcid].wdev;
		else
			wdev = pAd->wdev_list[0];
		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
								  FC,
								  pRxBlk->DataSize,
								  pRxBlk->rx_signal.raw_rssi[0],
								  pRxBlk->rx_signal.raw_rssi[1],
								  pRxBlk->rx_signal.raw_rssi[2],
								  pRxBlk->rx_signal.raw_rssi[3],
								  min(pRxBlk->rx_signal.raw_snr[0], pRxBlk->rx_signal.raw_snr[1]),
								  (rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
								  op_mode,
								  wdev,/*pAd->wdev_list[0],*/
								  pRxBlk->rx_rate.field.MODE);
	}
	return TRUE;
}

struct wifi_dev_ops ap_wdev_ops = {
	.tx_pkt_allowed = ap_tx_pkt_allowed,
	.fp_tx_pkt_allowed = ap_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.fill_non_offload_tx_blk = ap_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = ap_fill_offload_tx_blk,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
#ifdef CONFIG_ATE
	.ate_tx = mt_ate_tx,
#endif
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_ps_handle = ap_rx_ps_handle,
	.rx_pkt_foward = ap_rx_pkt_foward,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
};
