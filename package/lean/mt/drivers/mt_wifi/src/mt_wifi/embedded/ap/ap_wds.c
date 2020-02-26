
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
    ap_wds.c

    Abstract:
    Support WDS function.

    Revision History:
    Who       When            What
    ------    ----------      ----------------------------------------------
*/

#ifdef WDS_SUPPORT

#include "rt_config.h"


#define VAILD_KEY_INDEX(_X) ((((_X) >= 0) && ((_X) < 4)) ? (TRUE) : (FALSE))


BOOLEAN ValidWdsEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR WdsIndex)
{
	BOOLEAN result = FALSE;
	PMAC_TABLE_ENTRY pMacEntry;

	if ((WdsIndex < MAX_WDS_ENTRY) &&
		(pAd->WdsTab.WdsEntry[WdsIndex].Valid == TRUE)) {
		if ((pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID > 0) &&
			(pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID < GET_MAX_UCAST_NUM(pAd))) {
			pMacEntry = &pAd->MacTab.Content[pAd->WdsTab.WdsEntry[WdsIndex].MacTabMatchWCID];

			if (IS_ENTRY_WDS(pMacEntry))
				result = TRUE;
		}
	}

	return result;
}

INT wds_fp_tx_pkt_allowed(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	INT allowed = FALSE;
	RT_802_11_WDS_ENTRY *wds_entry;
	UCHAR wcid = RESERVED_WCID;
	UCHAR frag_nums;

	if (!wdev)
		return FALSE;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
		wds_entry = &pAd->WdsTab.WdsEntry[idx];

		if (ValidWdsEntry(pAd, idx) && (wdev == (&wds_entry->wdev))) {
			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			wcid = (UCHAR)pAd->WdsTab.WdsEntry[idx].MacTabMatchWCID;
			allowed = TRUE;
			break;
		}
	}

	if (!VALID_WCID(wcid))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	/*  ethertype check is not offload to mcu for fragment frame*/
	if (frag_nums > 1) {
		if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev)) {
			return FALSE;
		}
	}

	return allowed;
}

INT wds_tx_pkt_allowed(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	INT allowed = FALSE;
	RT_802_11_WDS_ENTRY *wds_entry;
	UCHAR wcid = RESERVED_WCID;
	UCHAR frag_nums;

	if (!wdev)
		return FALSE;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++) {
		wds_entry = &pAd->WdsTab.WdsEntry[idx];

		if (ValidWdsEntry(pAd, idx) && (wdev == (&wds_entry->wdev))) {
			RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
			wcid = (UCHAR)pAd->WdsTab.WdsEntry[idx].MacTabMatchWCID;
			allowed = TRUE;
			break;
		}
	}

	if (!VALID_WCID(wcid))
		return FALSE;

	RTMP_SET_PACKET_WCID(pkt, wcid);
	frag_nums = get_frag_num(pAd, wdev, pkt);
	RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

	if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev)) {
		return FALSE;
	}

	return allowed;
}

INT wds_rx_foward_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket)
{
	/*
		For WDS, direct to OS and no need to forwad the packet to WM
	*/
	return TRUE;
}

/* Since not used */
INT WdsEntryAlloc(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i, WdsTabIdx = -1;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): %02x-%02x-%02x-%02x-%02x-%02x, WdsMode = %d\n", __func__, PRINT_MAC(pAddr), pAd->WdsTab.Mode));
	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" Alloc WdsEntry[%d]\n", i));

		if ((pAd->WdsTab.Mode >= WDS_LAZY_MODE) && !WDS_IF_UP_CHECK(pAd, i))
			continue;

		wds_entry = &pAd->WdsTab.WdsEntry[i];

		if (wds_entry->Valid == FALSE) {
			wds_entry->Valid = TRUE;
			pAd->WdsTab.Size++;
			COPY_MAC_ADDR(wds_entry->PeerWdsAddr, pAddr);
			WdsTabIdx = i;
			break;
		} else if (MAC_ADDR_EQUAL(wds_entry->PeerWdsAddr, pAddr)) {
			WdsTabIdx = i;
			break;
		}
	}

	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);

	if (i == MAX_WDS_ENTRY)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unable to allocate WdsEntry.\n", __func__));

	return WdsTabIdx;
}


VOID WdsEntryDel(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): %02x-%02x-%02x-%02x-%02x-%02x\n", __func__,
			 PRINT_MAC(pAddr)));
	/* delete one WDS entry */
	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];

		if (MAC_ADDR_EQUAL(pAddr, wds_entry->PeerWdsAddr) && (wds_entry->Valid == TRUE)) {
			wds_entry->Valid = FALSE;
			NdisZeroMemory(wds_entry->PeerWdsAddr, MAC_ADDR_LEN);
			pAd->WdsTab.Size--;
			break;
		}
	}

	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
}


/*
	==========================================================================
	Description:
		Delete all WDS Entry in pAd->MacTab
	==========================================================================
 */
BOOLEAN MacTableDeleteWDSEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid)
{
	MAC_TABLE_ENTRY *pEntry;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return FALSE;

	pEntry = &pAd->MacTab.Content[wcid];

	if (!pEntry || !pEntry->wdev) {
		ASSERT(FALSE);
		return FALSE;
	}

	mac_entry_delete(pAd, pEntry);

	return TRUE;
}

BOOLEAN wds_bss_linkdown(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid)
{
	MAC_TABLE_ENTRY *pEntry;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return FALSE;

	pEntry = &pAd->MacTab.Content[wcid];

	if (!pEntry || !pEntry->wdev) {
		ASSERT(FALSE);
		return FALSE;
	}

	MlmeEnqueueWithWdev(pAd, WDS_STATE_MACHINE, WDS_BSS_LINKDOWN, sizeof(USHORT),
			&wcid, 0, pEntry->wdev);

	return TRUE;
}

/*
================================================================
Description : because WDS and CLI share the same WCID table in ASIC.
WDS entry also insert to pAd->MacTab.content[].
Also fills the pairwise key.
Because front MAX_AID_BA entries have direct mapping to BAEntry, which is only used as CLI. So we insert WDS
from index MAX_AID_BA.
================================================================
*/
MAC_TABLE_ENTRY *MacTableInsertWDSEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	UINT WdsTabIdx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	STA_TR_ENTRY *tr_entry;
	HTTRANSMIT_SETTING HTPhyMode;
	RT_802_11_WDS_ENTRY *wds_entry;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
	HT_CAPABILITY_IE *curr_ht_cap;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
	UCHAR ucETxBfCap;
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WdsTabIdx = %d, Addr %02x-%02x-%02x-%02x-%02x-%02x\n", __func__,
			 WdsTabIdx, PRINT_MAC(pAddr)));

	/* if FULL, return */
	if (pAd->MacTab.Size >= GET_MAX_UCAST_NUM(pAd))
		return NULL;

	pEntry = WdsTableLookup(pAd, pAddr, TRUE);
	if (pEntry != NULL)
		return pEntry;

	wds_entry = &pAd->WdsTab.WdsEntry[WdsTabIdx];
	wdev = &wds_entry->wdev;
	rate = &wdev->rate;
	curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);

	/* allocate one WDS entry */
	do {
		/* allocate one MAC entry */
		pEntry = MacTableInsertEntry(pAd, pAddr, wdev, ENTRY_WDS, OPMODE_AP, TRUE);

		if (pEntry) {
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			tr_entry->OmacIdx = wdev->OmacIdx;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  OmacIdx = %d, wcid = %d, PhyOpMode = %d\n",
					 tr_entry->OmacIdx, pEntry->wcid, wds_entry->PhyOpMode));
			/* specific Max Tx Rate for Wds link. */
			NdisZeroMemory(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));

			switch (wds_entry->PhyOpMode) {
			case 0xff: /* user doesn't specific a Mode for WDS link. */
			case MODE_OFDM: /* specific OFDM mode. */
				HTPhyMode.field.MODE = MODE_OFDM;
				HTPhyMode.field.MCS = 7;
				pEntry->RateLen = 8;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODE_OFDM\n"));
				break;

			case MODE_CCK:
				HTPhyMode.field.MODE = MODE_CCK;
				HTPhyMode.field.MCS = 3;
				pEntry->RateLen = 4;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODE_CCK\n"));
				break;
#ifdef DOT11_N_SUPPORT

			case MODE_HTMIX:
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				HTPhyMode.field.MODE = MODE_HTMIX;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODE_HTMIX\n"));
				break;

			case MODE_HTGREENFIELD:
				HTPhyMode.field.MCS = 7;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				HTPhyMode.field.MODE = MODE_HTGREENFIELD;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODE_HTGREENFIELD\n"));
				break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

			case MODE_VHT:
				HTPhyMode.field.MCS = 9;
				HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
				HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
				HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
				HTPhyMode.field.MODE = MODE_VHT;
				pEntry->RateLen = 12;
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  MODE_VHT\n"));
				break;
#endif /* DOT11_VHT_AC */

			default:
				break;
			}

			pEntry->MaxHTPhyMode.word = HTPhyMode.word;
			pEntry->MinHTPhyMode.word = wdev->MinHTPhyMode.word;
			pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
#ifdef DOT11_N_SUPPORT

			if (wds_entry->PhyOpMode >= MODE_HTMIX) {
				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IF-wds%d : Desired MCS = %d\n", WdsTabIdx,
							 wdev->DesiredTransmitSetting.field.MCS));
					set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pEntry->MmpsMode = MMPS_DISABLE;
				NdisMoveMemory(&pEntry->HTCapability, curr_ht_cap, sizeof(HT_CAPABILITY_IE));
#ifdef TXBF_SUPPORT

				if (HcIsBfCapSupport(wdev) == FALSE) {
					UCHAR ucEBfCap;

					ucEBfCap = wlan_config_get_etxbf(wdev);
					wlan_config_set_etxbf(wdev, SUBF_OFF);
					mt_WrapSetETxBFCap(pAd, wdev, &pEntry->HTCapability.TxBFCap);
					wlan_config_set_etxbf(wdev, ucEBfCap);
				}

#endif /* TXBF_SUPPORT */
				set_sta_ht_cap(pAd, pEntry, &pEntry->HTCapability);
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
#ifdef DOT11_VHT_AC

				if (WMODE_CAP_AC(wdev->PhyMode) && (wdev->channel > 14)) {
					VHT_CAP_IE vht_cap;
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
					ucETxBfCap = wlan_config_get_etxbf(wdev);

					if (HcIsBfCapSupport(wdev) == FALSE)
						wlan_config_set_etxbf(wdev, SUBF_OFF);

#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
					build_vht_cap_ie(pAd, wdev, (UCHAR *)&vht_cap);
#if defined(TXBF_SUPPORT) && defined(VHT_TXBF_SUPPORT)
					wlan_config_set_etxbf(wdev, ucETxBfCap);
#endif /* TXBF_SUPPORT && VHT_TXBF_SUPPORT */
					vht_mode_adjust(pAd, pEntry, &vht_cap, NULL);
					dot11_vht_mcs_to_internal_mcs(pAd, wdev, &vht_cap, &pEntry->MaxHTPhyMode);
					set_vht_cap(pAd, pEntry, &vht_cap);
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n",
							 __func__,
							 get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
							 get_bw_str(pEntry->MaxHTPhyMode.field.BW),
							 pEntry->MaxHTPhyMode.field.MCS,
							 pEntry->MaxHTPhyMode.word));
					NdisMoveMemory(&pEntry->vht_cap_ie, &vht_cap, sizeof(VHT_CAP_IE));
					assoc_vht_info_debugshow(pAd, pEntry, &vht_cap, NULL);
				} else
					NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));

				pEntry->force_op_mode = FALSE;
#endif /* DOT11_VHT_AC */
			}

#endif /* DOT11_N_SUPPORT */
			else
				NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));

			RTMPSetSupportMCS(pAd,
							  OPMODE_AP,
							  pEntry,
							  rate->SupRate,
							  rate->SupRateLen,
							  rate->ExtRate,
							  rate->ExtRateLen,
#ifdef DOT11_VHT_AC
							  sizeof(VHT_CAP_IE),
							  &pEntry->vht_cap_ie,
#endif /* DOT11_VHT_AC */
							  curr_ht_cap,
							  sizeof(HT_CAPABILITY_IE));
			/* for now, we set this by default! */
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			if (wdev->bAutoTxRateSwitch == FALSE) {
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS;
				pEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
			} else {
				/* TODO: shiang-MT7603, fix me for this, because we may need to set this only when we have WTBL entry for tx_rate! */
				pEntry->bAutoTxRateSwitch = TRUE;
			}

			wds_entry->MacTabMatchWCID = (UCHAR)pEntry->wcid;
			pEntry->func_tb_idx = WdsTabIdx;
			pEntry->wdev = wdev;
			COPY_MAC_ADDR(&wdev->bssid[0], &pEntry->Addr[0]);
			/* update per wdev bw */
			wlan_operate_set_ht_bw(wdev, wdev->MaxHTPhyMode.field.BW, wlan_operate_get_ext_cha(wdev));

			if (wdev_do_linkup(wdev, pEntry) != TRUE)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): linkup fail!!\n", __func__));

			if (wdev_do_conn_act(wdev, pEntry) != TRUE)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): connect fail!!\n", __func__));

			AsicUpdateWdsEncryption(pAd, pEntry->wcid);

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  PhyMode = 0x%x, Channel = %d\n",
					 wdev->PhyMode, wdev->channel));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() - allocate entry #%d(link to WCID %d), Total= %d\n",
					 __func__, WdsTabIdx, wds_entry->MacTabMatchWCID, pAd->WdsTab.Size));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("  Cap.MaxRAmpduFactor = %d, Cap.MpduDensity = %d (%d/%d), pAd = (%d) (%d)\n",
					 pEntry->HTCapability.HtCapParm.MaxRAmpduFactor,
					 pEntry->HTCapability.HtCapParm.MpduDensity,
					 pEntry->MaxRAmpduFactor,
					 pEntry->MpduDensity,
					 curr_ht_cap->HtCapParm.MaxRAmpduFactor,
					 curr_ht_cap->HtCapParm.MpduDensity));
#ifdef DOT11_VHT_AC
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("  DesiredHtPhyInfo HtEn = %d, PreHt = %d, VhtEn = %d, vht_bw = %d\n",
					 wdev->DesiredHtPhyInfo.bHtEnable, wdev->DesiredHtPhyInfo.bPreNHt,
					 wdev->DesiredHtPhyInfo.bVhtEnable, wdev->DesiredHtPhyInfo.vht_bw));
#endif
			break;
		}
	} while (FALSE);

	return pEntry;
}


MAC_TABLE_ENTRY *WdsTableLookupByWcid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pAddr,
	IN BOOLEAN bResetIdelCount)
{
	ULONG WdsIndex;
	MAC_TABLE_ENTRY *pCurEntry = NULL, *pEntry = NULL;

	RETURN_ZERO_IF_PAD_NULL(pAd);

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);

	do {
		pCurEntry = &pAd->MacTab.Content[wcid];
		WdsIndex = 0xff;

		if ((pCurEntry) && IS_ENTRY_WDS(pCurEntry))
			WdsIndex = pCurEntry->func_tb_idx;

		if (WdsIndex == 0xff)
			break;

		if (pAd->WdsTab.WdsEntry[WdsIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddr)) {
			if (bResetIdelCount) {
				pCurEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
				pAd->MacTab.tr_entry[pCurEntry->tr_tb_idx].NoDataIdleCount = 0;
			}

			pEntry = pCurEntry;
			break;
		}
	} while (FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}


MAC_TABLE_ENTRY *WdsTableLookup(RTMP_ADAPTER *pAd, UCHAR *addr, BOOLEAN bResetIdelCount)
{
	USHORT HashIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;

	NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);
	HashIdx = MAC_ADDR_HASH_INDEX(addr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry) {
		if (IS_ENTRY_WDS(pEntry) && MAC_ADDR_EQUAL(pEntry->Addr, addr)) {
			if (bResetIdelCount) {
				pEntry->NoDataIdleCount = 0;
				/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
				pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
			}

			break;
		}
		pEntry = pEntry->pNext;
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	return pEntry;
}


MAC_TABLE_ENTRY *FindWdsEntry(
	IN RTMP_ADAPTER *pAd,
	IN struct _RX_BLK *pRxBlk)
{
	MAC_TABLE_ENTRY *pEntry;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Wcid = %d, PhyMode = 0x%x\n", __func__,
		 pRxBlk->wcid, pRxBlk->rx_rate.field.MODE));
	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, pRxBlk->wcid, pRxBlk->Addr2, TRUE);

	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pRxBlk->Addr2, TRUE);

	/*  Report to MLME, add WDS entry */
	if ((pEntry == NULL) && (pAd->WdsTab.Mode >= WDS_LAZY_MODE)) {
		UCHAR *pTmpBuf = pRxBlk->pData - LENGTH_802_11;
		RXD_BASE_STRUCT *rxd_base = (RXD_BASE_STRUCT *)pRxBlk->rmac_info;
		struct wifi_dev *wdev = wdev_search_by_address(pAd, pRxBlk->Addr1);

		if (!wdev) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): No matched wdev (%02x-%02x-%02x-%02x-%02x-%02x), wcid = %d!!!\n", __func__,
				 PRINT_MAC(pRxBlk->Addr1), pRxBlk->wcid));
			return NULL;
		}

		NdisMoveMemory(pTmpBuf, pRxBlk->FC, LENGTH_802_11);

		REPORT_MGMT_FRAME_TO_MLME(pAd, pRxBlk->wcid,
					  pTmpBuf,
					  pRxBlk->DataSize + LENGTH_802_11,
					  pRxBlk->rx_signal.raw_rssi[0],
					  pRxBlk->rx_signal.raw_rssi[1],
					  pRxBlk->rx_signal.raw_rssi[2],
					  pRxBlk->rx_signal.raw_rssi[3],
					  (rxd_base != NULL) ? rxd_base->RxD1.ChFreq : 0,
					  0,
					  OPMODE_AP,
					  wdev,
					  pRxBlk->rx_rate.field.MODE);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("!!! report WDS UC DATA (from %02x-%02x-%02x-%02x-%02x-%02x) to MLME (len=%d) !!!\n",
			 PRINT_MAC(pRxBlk->Addr2), pRxBlk->DataSize));
	}

	return pEntry;
}


/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any WDS client being idle for too long and should be aged-out from MAC table
	==========================================================================
*/
VOID WdsTableMaintenance(RTMP_ADAPTER *pAd)
{
	UCHAR idx;

	if (pAd->WdsTab.Mode != WDS_LAZY_MODE)
		return;

	for (idx = 0; idx < pAd->WdsTab.Size; idx++) {
		UCHAR wcid = pAd->WdsTab.WdsEntry[idx].MacTabMatchWCID;
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[wcid];

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Entry[%d], Type = %d, Wcid = %d, %02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, idx, pEntry->EntryType, wcid, PRINT_MAC(pEntry->Addr)));

		if (!IS_ENTRY_WDS(pEntry))
			continue;

		NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
		NdisAcquireSpinLock(&pAd->MacTabLock);
		pEntry->NoDataIdleCount++;
		/* TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry! */
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount++;
		NdisReleaseSpinLock(&pAd->MacTabLock);
		NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);

		/* delete those MAC entry that has been idle for a long time */
		if (pEntry->NoDataIdleCount >= MAC_TABLE_AGEOUT_TIME) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from WDS #%d after %d-sec silence\n",
					 PRINT_MAC(pEntry->Addr), idx, MAC_TABLE_AGEOUT_TIME));
			WdsEntryDel(pAd, pEntry->Addr);
			wds_bss_linkdown(pAd, pEntry->wcid);
		}
	}
}

VOID AsicUpdateWdsRxWCIDTable(RTMP_ADAPTER *pAd, UINT WdsTabIdx)
{
	MAC_TABLE_ENTRY *pEntry = NULL;
	RT_802_11_WDS_ENTRY *wds_entry = &pAd->WdsTab.WdsEntry[WdsTabIdx];
	UCHAR ApPhyMode = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;
	UINT32 encrypt_mode = wds_entry->wdev.SecConfig.PairwiseCipher;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WdsEntry[%d] = %p, WDS_Mode = %d, PhyOpMode = %d\n", __func__,
			 WdsTabIdx, wds_entry, pAd->WdsTab.Mode, wds_entry->PhyOpMode));

	if (pAd->WdsTab.Mode >= WDS_LAZY_MODE) {
		wds_entry->PhyOpMode = 0xff;

		if (WMODE_CAP_AC(ApPhyMode))
			wds_entry->PhyOpMode = MODE_VHT;
		else if (WMODE_CAP_N(ApPhyMode))
			wds_entry->PhyOpMode = MODE_HTMIX;
		else {
			if (WMODE_EQUAL(ApPhyMode, WMODE_B))
				wds_entry->PhyOpMode = MODE_CCK;
			else
				wds_entry->PhyOpMode = MODE_OFDM;
		}
	}

	if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode)) {
		wds_entry->PhyOpMode = (wds_entry->PhyOpMode >= MODE_OFDM) ? MODE_OFDM : MODE_CCK;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s : Use legacy rate in WEP/TKIP encryption mode (wdsidx=%d)\n",
				  __func__, WdsTabIdx));
	}

	wds_entry->wdev.PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, wds_entry);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("  PhyOpMode = %d, PhyMode = 0x%x (ApPhyMode = 0x%x)\n",
			 wds_entry->PhyOpMode, wds_entry->wdev.PhyMode, ApPhyMode));

	if (wds_entry->Valid != TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid WDS entry!", __func__));
		return;
	}

	pEntry = MacTableInsertWDSEntry(pAd, wds_entry->PeerWdsAddr, WdsTabIdx);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can't insert a new WDS entry!", __func__));
		return;
	}

	pEntry->SupportRateMode = WdsPhyOpModeToSuppRateMode(pAd, wds_entry);
	RAInit(pAd, pEntry);
}


#define RALINK_PASSPHRASE	"Ralink"
VOID AsicUpdateWdsEncryption(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	PMAC_TABLE_ENTRY pEntry = NULL;
	struct wifi_dev *wdev = NULL;
	struct _SECURITY_CONFIG *pSecConfig = NULL;
	ASIC_SEC_INFO Info = {0};

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return;

	pEntry = &pAd->MacTab.Content[wcid];

	if (pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].Valid != TRUE)
		return;

	if (!IS_ENTRY_WDS(pEntry))
		return;

	wdev = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev;
	pSecConfig = &wdev->SecConfig;

	if (pSecConfig->AKMMap == 0x0)
		SET_AKM_OPEN(pSecConfig->AKMMap);

	if (pSecConfig->PairwiseCipher == 0x0)
		SET_CIPHER_NONE(pSecConfig->PairwiseCipher);

	/* Set key material to Asic */
	os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
	Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
	Info.Direction = SEC_ASIC_KEY_BOTH;
	Info.Wcid = wcid;
	Info.BssIndex = wdev->bss_info_argument.ucBssIndex;
	Info.Cipher = pSecConfig->PairwiseCipher;
	Info.KeyIdx = pSecConfig->PairwiseKeyId;
	os_move_mem(&Info.PeerAddr[0], pEntry->Addr, MAC_ADDR_LEN);

	/* When WEP, TKIP or AES is enabled, set group key info to Asic */
	if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher)) {
		os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));
		HW_ADDREMOVE_KEYTABLE(pAd, &Info);
	} else if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_CCMP128(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_CCMP256(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_GCMP128(pSecConfig->PairwiseCipher)
			   || IS_CIPHER_GCMP256(pSecConfig->PairwiseCipher)) {
		/* Calculate Key */
		SetWPAPSKKey(pAd, pSecConfig->PSK, strlen(pSecConfig->PSK), (PUCHAR) RALINK_PASSPHRASE, sizeof(RALINK_PASSPHRASE), pSecConfig->PMK);
		os_move_mem(Info.Key.Key, pSecConfig->PMK, LEN_PMK);

		if (IS_CIPHER_TKIP(pSecConfig->PairwiseCipher)) {
			/*WDS: RxMic/TxMic use the same value */
			os_move_mem(&Info.Key.Key[LEN_TK + LEN_TKIP_MIC], &Info.Key.Key[LEN_TK], LEN_TKIP_MIC);
		}

		WPAInstallKey(pAd, &Info, TRUE, TRUE);
	}
}

UCHAR WdsGetPeerSuppPhyModeLegacy(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN UCHAR Channel)
{
	UCHAR PeerPhyModeLegacy = 0;
	INT i;

	if ((SupRateLen > 0) && (SupRateLen < MAX_LEN_OF_SUPPORTED_RATES)) {
		for (i = 0; i < SupRateLen; i++) {
			/* CCK Rates: 1, 2, 5.5, 11 */
			if (((SupRate[i] & 0x7f) == 0x2) || ((SupRate[i] & 0x7f) == 0x4) ||
				((SupRate[i] & 0x7f) == 0xb) || ((SupRate[i] & 0x7f) == 0x16))
				PeerPhyModeLegacy |= WMODE_B;
			/* OFDM Rates: 6, 9, 12, 18, 24, 36, 48, 54 */
			else if (((SupRate[i] & 0x7f) == 0xc) || ((SupRate[i] & 0x7f) == 0x12) ||
					 ((SupRate[i] & 0x7f) == 0x18) || ((SupRate[i] & 0x7f) == 0x24) ||
					 ((SupRate[i] & 0x7f) == 0x30) || ((SupRate[i] & 0x7f) == 0x48) ||
					 ((SupRate[i] & 0x7f) == 0x60) || ((SupRate[i] & 0x7f) == 0x6c)) {
				if (Channel > 14)
					PeerPhyModeLegacy |= WMODE_A;
				else
					PeerPhyModeLegacy |= WMODE_G;
			}
		}
	}

	return PeerPhyModeLegacy;
}

UCHAR WdsGetPeerSuppPhyMode(
	IN BCN_IE_LIST * ie_list)
{
	UCHAR PeerPhyMode = 0;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11_VHT_AC

	/* Check VHT capability */
	if (ie_list->vht_cap_len && ie_list->Channel > 14)
		PeerPhyMode |= WMODE_AC;

#endif

	/* Check HT capability */
	if (ie_list->HtCapabilityLen) {
		if (ie_list->Channel > 14)
			PeerPhyMode |= WMODE_AN;
		else
			PeerPhyMode |= WMODE_GN;
	}

#endif
	/* Check OFDM/CCK capability */
	PeerPhyMode |= WdsGetPeerSuppPhyModeLegacy(ie_list->SupRate, ie_list->SupRateLen, ie_list->Channel);
	PeerPhyMode |= WdsGetPeerSuppPhyModeLegacy(ie_list->ExtRate, ie_list->ExtRateLen, ie_list->Channel);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): %02x-%02x-%02x-%02x-%02x-%02x, PeerPhyMode = %d\n", __func__,
			 PRINT_MAC(ie_list->Addr2), PeerPhyMode));

	if (((ie_list->Channel > 0) && (ie_list->Channel <= 14) && WMODE_CAP_5G(PeerPhyMode)) ||
		((ie_list->Channel > 14) && WMODE_CAP_2G(PeerPhyMode)))
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("ERROR!! Wrong PeerPhyMode 0x%x, Channel %d\n", PeerPhyMode, ie_list->Channel));

	return PeerPhyMode;
}

VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BOOLEAN bWmmCapable,
	IN BCN_IE_LIST * ie_list)
{
	UCHAR MaxSupportedRate = RATE_11;
	UCHAR ApPhyMode, PeerPhyMode;
	BOOLEAN bRaReInit = FALSE;
	RT_802_11_WDS_ENTRY *pWdsEntry = NULL;
	USHORT CapabilityInfo = 0;
	struct _vendor_ie_cap *pVendorIe = NULL;
	HT_CAPABILITY_IE *pHtCap = NULL;
	HT_CAPABILITY_IE *curr_ht_cap;
	UCHAR HtCapLen = 0;
#ifdef DOT11_VHT_AC
	VHT_CAP_IE *pVhtCap = NULL;
	UCHAR VhtCapLen = 0;
#endif /* DOT11_VHT_AC */
	struct dev_rate_info *rate;
#ifdef DOT11_N_SUPPORT
	UCHAR new_bw = HT_BW_20;
#endif

	RETURN_IF_PAD_NULL(pAd);
	ApPhyMode = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid pEntry!", __func__));
		return;
	}
	pWdsEntry = &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx];

	if (!pEntry->wdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid wdev!", __func__));
		return;
	}

	if (!pWdsEntry) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid WDS entry!", __func__));
		return;
	}

	if (ie_list) {
		CapabilityInfo = ie_list->CapabilityInfo;
		pVendorIe = &ie_list->vendor_ie;
		pHtCap = &ie_list->HtCapability;
		HtCapLen = ie_list->HtCapabilityLen;
#ifdef DOT11_VHT_AC
		pVhtCap = &ie_list->vht_cap_ie;
		VhtCapLen = ie_list->vht_cap_len;
#endif /* DOT11_VHT_AC */
	} else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Invalid ie_list!", __func__));
		return;
	}

	PeerPhyMode = WdsGetPeerSuppPhyMode(ie_list);
	MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): wcid=%d, MaxSupportedRate = %d, HtCapLen = %d, VhtCapLen = %d\n", __func__,
			 pEntry->wcid, MaxSupportedRate,
			 HtCapLen,
#ifdef DOT11_VHT_AC
			 VhtCapLen));
#else
			 0));
#endif

	if (pEntry && IS_ENTRY_WDS(pEntry)) {
#ifdef DOT11_N_SUPPORT
		UINT32 encrypt_mode = pEntry->wdev->SecConfig.PairwiseCipher;
#endif /* DOT11_N_SUPPORT */
		pEntry->MaxSupportedRate = min(pEntry->wdev->rate.MaxTxRate, MaxSupportedRate);
		pEntry->RateLen = MaxSupportedRateLen;

		/* Set Init PhyMode as OFDM or CCK*/
		if (((ApPhyMode & WMODE_G) && (PeerPhyMode & WMODE_G)) ||
			((ApPhyMode & WMODE_A) && (PeerPhyMode & WMODE_A))) {
			pWdsEntry->PhyOpMode = MODE_OFDM;
			pEntry->wdev->PhyMode = (PeerPhyMode & WMODE_G) ? WMODE_G : WMODE_A;
			pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
			pEntry->MaxHTPhyMode.field.MCS = 7;
		} else {
			pWdsEntry->PhyOpMode = MODE_CCK;
			pEntry->wdev->PhyMode = WMODE_B;
			pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
			pEntry->MaxHTPhyMode.field.MCS = 3;
		}

		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->MinHTPhyMode.field.BW = BW_20;
#ifdef DOT11_N_SUPPORT
		pEntry->HTCapability.MCSSet[0] = 0;
		pEntry->HTCapability.MCSSet[1] = 0;
		pEntry->HTCapability.MCSSet[2] = 0;
		pEntry->HTCapability.MCSSet[3] = 0;
#endif /* DOT11_N_SUPPORT */
		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		pEntry->CapabilityInfo = CapabilityInfo;
		MacTableSetEntryRaCap(pAd, pEntry, pVendorIe);
#ifdef DOT11_N_SUPPORT
		curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(pEntry->wdev);

		/* If this Entry supports 802.11n, upgrade to HT rate. */
		if ((HtCapLen != 0)	&& WMODE_CAP_N(ApPhyMode)
			&& !(pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode))) {
			pWdsEntry->PhyOpMode = MODE_HTMIX;
			pWdsEntry->wdev.PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, pWdsEntry) & PeerPhyMode;

			if (pEntry->wdev) {
				/* ap and wds_peer both support BW40 */
				if (wlan_operate_get_ht_bw(&(pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev)) == HT_BW_40 &&
					pHtCap->HtCapInfo.ChannelWidth == HT_BW_40) {
						new_bw = HT_BW_40;
				}

				if (wlan_operate_get_ht_bw(pEntry->wdev) != new_bw) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): set new_bw = %d\n", __func__, new_bw));
					wlan_operate_set_ht_bw(pEntry->wdev, new_bw, wlan_operate_get_ext_cha(pEntry->wdev));
				}
			}

			ht_mode_adjust(pAd, pEntry, pHtCap);
			/* find max fixed rate */
			pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &pWdsEntry->wdev.DesiredHtPhyInfo.MCSSet[0],
											 &pHtCap->MCSSet[0]);

			if ((pEntry->MaxHTPhyMode.field.MCS > pWdsEntry->wdev.HTPhyMode.field.MCS) && (pWdsEntry->wdev.HTPhyMode.field.MCS != MCS_AUTO))
				pEntry->MaxHTPhyMode.field.MCS = pWdsEntry->wdev.HTPhyMode.field.MCS;

			pEntry->MaxHTPhyMode.field.STBC = (pHtCap->HtCapInfo.RxSTBC & (curr_ht_cap->HtCapInfo.RxSTBC));
			set_sta_ht_cap(pAd, pEntry, pHtCap);
			NdisMoveMemory(&pEntry->HTCapability, pHtCap, sizeof(HT_CAPABILITY_IE));
			NdisMoveMemory(curr_ht_cap, pHtCap, sizeof(HT_CAPABILITY_IE));
#ifdef DOT11_VHT_AC
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("  vhtCapLen = %d\n", VhtCapLen));

			if (WMODE_CAP_AC(ApPhyMode) && (pWdsEntry->wdev.channel > 14) && VhtCapLen) {
				RT_PHY_INFO *pDesired_ht_phy = &pEntry->wdev->DesiredHtPhyInfo;

				pWdsEntry->PhyOpMode = MODE_VHT;

				/* Set desired phy to VHT mode */
				if (pDesired_ht_phy && !pDesired_ht_phy->bVhtEnable) {
					pDesired_ht_phy->bVhtEnable = TRUE;
					rtmp_set_vht(pAd, &pWdsEntry->wdev, pDesired_ht_phy);
				}

				vht_mode_adjust(pAd, pEntry, pVhtCap, NULL);
				dot11_vht_mcs_to_internal_mcs(pAd, pEntry->wdev, pVhtCap, &pEntry->MaxHTPhyMode);
				set_vht_cap(pAd, pEntry, pVhtCap);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Peer's PhyCap=>Mode:%s, BW:%s, MCS: 0x%x (Word = 0x%x)\n",
						 __func__,
						 get_phymode_str(pEntry->MaxHTPhyMode.field.MODE),
						 get_bw_str(pEntry->MaxHTPhyMode.field.BW),
						 pEntry->MaxHTPhyMode.field.MCS,
						 pEntry->MaxHTPhyMode.word));
				NdisMoveMemory(&pEntry->vht_cap_ie, pVhtCap, sizeof(VHT_CAP_IE));

				if (DebugLevel >= DBG_LVL_INFO)
					assoc_vht_info_debugshow(pAd, pEntry, pVhtCap, NULL);
			} else
				NdisZeroMemory(&pEntry->vht_cap_ie, sizeof(VHT_CAP_IE));

			pEntry->force_op_mode = FALSE;
#endif /* DOT11_VHT_AC */
		} else {
			NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
			pAd->MacTab.fAnyStationIsLegacy = TRUE;
		}

#endif /* DOT11_N_SUPPORT */
		pWdsEntry->wdev.PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, pWdsEntry) & PeerPhyMode;
		rate = &pWdsEntry->wdev.rate;
		RTMPSetSupportMCS(pAd,
						  OPMODE_AP,
						  pEntry,
						  rate->SupRate,
						  rate->SupRateLen,
						  rate->ExtRate,
						  rate->ExtRateLen,
#ifdef DOT11_VHT_AC
						  sizeof(VHT_CAP_IE),
						  &pEntry->vht_cap_ie,
#endif /* DOT11_VHT_AC */
						  curr_ht_cap,
						  sizeof(HT_CAPABILITY_IE));

		if (bWmmCapable
#ifdef DOT11_N_SUPPORT
			|| (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)
#endif /* DOT11_N_SUPPORT */
		   )
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		else
			CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, (" Mode %d, MCS %d, STBC %d SGI %d, BW %d / MAX Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
				 pEntry->HTPhyMode.field.MODE,
				 pEntry->HTPhyMode.field.MCS,
				 pEntry->HTPhyMode.field.STBC,
				 pEntry->HTPhyMode.field.ShortGI,
				 pEntry->HTPhyMode.field.BW,
				 pEntry->MaxHTPhyMode.field.MODE,
				 pEntry->MaxHTPhyMode.field.MCS,
				 pEntry->MaxHTPhyMode.field.STBC,
				 pEntry->MaxHTPhyMode.field.ShortGI,
				 pEntry->MaxHTPhyMode.field.BW));

		if ((pEntry->HTPhyMode.field.MODE != pEntry->MaxHTPhyMode.field.MODE) ||
			(pEntry->HTPhyMode.field.STBC != pEntry->MaxHTPhyMode.field.STBC) ||
			(pEntry->HTPhyMode.field.ShortGI != pEntry->MaxHTPhyMode.field.ShortGI) ||
			(pEntry->HTPhyMode.field.BW != pEntry->MaxHTPhyMode.field.BW) ||
			(pEntry->HTPhyMode.field.MCS != pEntry->MaxHTPhyMode.field.MCS)) {
			pEntry->HTPhyMode.field.MODE = pEntry->MaxHTPhyMode.field.MODE;
			pEntry->HTPhyMode.field.STBC = pEntry->MaxHTPhyMode.field.STBC;
			pEntry->HTPhyMode.field.ShortGI = pEntry->MaxHTPhyMode.field.ShortGI;
			pEntry->HTPhyMode.field.BW = pEntry->MaxHTPhyMode.field.BW;
			pEntry->HTPhyMode.field.MCS = pEntry->MaxHTPhyMode.field.MCS;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" bRaReInit, New Mode %d, MCS %d, STBC %d SGI %d, BW %d\n",
					 pEntry->HTPhyMode.field.MODE,
					 pEntry->HTPhyMode.field.MCS,
					 pEntry->HTPhyMode.field.STBC,
					 pEntry->HTPhyMode.field.ShortGI,
					 pEntry->HTPhyMode.field.BW));
			bRaReInit = TRUE;
		}

		pEntry->SupportRateMode = WdsPhyOpModeToSuppRateMode(pAd, pWdsEntry);
	}

	if (bRaReInit) {
		/* StaRec update */
		wifi_sys_update_wds(pAd, pEntry);
		AsicUpdateWdsEncryption(pAd, pEntry->wcid);
		RAInit(pAd, pEntry);
	}
}


VOID APWdsInitialize(RTMP_ADAPTER *pAd)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	pAd->WdsTab.Mode = WDS_DISABLE_MODE;
	pAd->WdsTab.Size = 0;

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		wds_entry = &pAd->WdsTab.WdsEntry[i];
		wds_entry->PhyOpMode = 0xff;
		wds_entry->wdev.bAutoTxRateSwitch = TRUE;
		wds_entry->wdev.DesiredTransmitSetting.field.MCS = MCS_AUTO;
		wds_entry->Valid = FALSE;
		wds_entry->MacTabMatchWCID = 0;
		wds_entry->KeyIdx = 0;
		NdisZeroMemory(&wds_entry->WdsKey, sizeof(CIPHER_KEY));
		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():WdsEntry Number=%d\n", __func__, MAX_WDS_ENTRY));
}


INT Show_WdsTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT	i;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nWdsEnable=%d\n",
			 pAd->WdsTab.Mode));

	for (i = 0; i < MAX_WDS_ENTRY; i++) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IF/WDS%d-%02x:%02x:%02x:%02x:%02x:%02x(%s), OpState=%d, Cipher=%s, KeyId=%d\n", i,
				 PRINT_MAC(pAd->WdsTab.WdsEntry[i].PeerWdsAddr),
				 pAd->WdsTab.WdsEntry[i].Valid == 1 ? " Valid " : "Invalid",
				 wlan_operate_get_state(&pAd->WdsTab.WdsEntry[i].wdev),
				 GetEncryModeStr(pAd->WdsTab.WdsEntry[i].wdev.SecConfig.PairwiseCipher),
				 pAd->WdsTab.WdsEntry[i].wdev.SecConfig.PairwiseKeyId));

		if (pAd->WdsTab.WdsEntry[i].WdsKey.KeyLen > 0)
			hex_dump("Wds Key", pAd->WdsTab.WdsEntry[i].WdsKey.Key, pAd->WdsTab.WdsEntry[i].WdsKey.KeyLen);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n%-19s%-4s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
			 "MAC", "IDX", "AID", "PSM", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC"));

	for (i = 0; VALID_UCAST_ENTRY_WCID(pAd, i); i++) {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

		if (IS_ENTRY_WDS(pEntry)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr)));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-4d", (int)pEntry->func_tb_idx));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-4d", (int)pEntry->Aid));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-4d", (int)pEntry->PsMode));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[0]));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[1]));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[2]));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
#ifdef DOT11_VHT_AC

			if (pEntry->HTPhyMode.field.MODE == MODE_VHT) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%dS-M%-2d",
						 ((pEntry->HTPhyMode.field.MCS>>4) + 1), (pEntry->HTPhyMode.field.MCS & 0xf)));
			} else
#endif /* DOT11_VHT_AC */
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d\n", pEntry->HTPhyMode.field.STBC));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" MaxCap:%-10s", get_phymode_str(pEntry->MaxHTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6s", get_bw_str(pEntry->MaxHTPhyMode.field.BW)));
#ifdef DOT11_VHT_AC

			if (pEntry->MaxHTPhyMode.field.MODE == MODE_VHT) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%dS-M%d",
						 ((pEntry->MaxHTPhyMode.field.MCS>>4) + 1), (pEntry->MaxHTPhyMode.field.MCS & 0xf)));
			} else
#endif /* DOT11_VHT_AC */
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d", pEntry->MaxHTPhyMode.field.MCS));

			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d", pEntry->MaxHTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%-6d\n", pEntry->MaxHTPhyMode.field.STBC));
		}
	}

	return TRUE;
}


VOID rtmp_read_wds_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT			i = 0, j;
	UCHAR		macAddress[MAC_ADDR_LEN];
	PRT_802_11_WDS_ENTRY pWdsEntry;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WDS Profile\n", __func__));

	/*WdsPhyMode */
	if (RTMPGetKeyParameter("WdsPhyMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
			wdev = &pWdsEntry->wdev;

			if (rtstrcasecmp(macptr, "CCK") == TRUE)
				pWdsEntry->PhyOpMode = MODE_CCK;
			else if (rtstrcasecmp(macptr, "OFDM") == TRUE)
				pWdsEntry->PhyOpMode = MODE_OFDM;

#ifdef DOT11_N_SUPPORT
			else if (rtstrcasecmp(macptr, "HTMIX") == TRUE)
				pWdsEntry->PhyOpMode = MODE_HTMIX;
			else if (rtstrcasecmp(macptr, "GREENFIELD") == TRUE)
				pWdsEntry->PhyOpMode = MODE_HTGREENFIELD;

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC
			else if (rtstrcasecmp(macptr, "VHT") == TRUE)
				pWdsEntry->PhyOpMode = MODE_VHT;

#endif /* DOT11_VHT_AC */
			else
				pWdsEntry->PhyOpMode = 0xff;

			wdev->PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, pWdsEntry);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("If/wds%d - PeerPhyOpMode=%d\n", i, pWdsEntry->PhyOpMode));
		}
	}

	/*WdsList */
	if (RTMPGetKeyParameter("WdsList", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		if (pAd->WdsTab.Mode != WDS_LAZY_MODE) {
			for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
				if (strlen(macptr) != 17) /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
					continue;

				if (strcmp(macptr, "00:00:00:00:00:00") == 0)
					continue;

				if (i >= MAX_WDS_ENTRY - 1)
					break;

				for (j = 0; j < MAC_ADDR_LEN; j++) {
					AtoH(macptr, &macAddress[j], 1);
					macptr = macptr+3;
				}

				WdsEntryAlloc(pAd, macAddress);
			}
		}
	}

	/* WdsTxMode */
	if (RTMPGetKeyParameter("WdsTxMode", tmpbuf, 25, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.FixedTxMode =
				RT_CfgSetFixedTxPhyMode(macptr);
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("I/F(wds%d) Tx Mode = %d\n", i,
					 wdev->DesiredTransmitSetting.field.FixedTxMode));
		}
	}

	/* WdsTxMcs */
	if (RTMPGetKeyParameter("WdsTxMcs", tmpbuf, 50, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;
			wdev->DesiredTransmitSetting.field.MCS =
				RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("I/F(wds%d) Tx MCS = AUTO\n", i));
			else {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("I/F(wds%d) Tx MCS = %d\n", i,
						 wdev->DesiredTransmitSetting.field.MCS));
			}
		}
	}

	/*WdsEnable */
	if (RTMPGetKeyParameter("WdsEnable", tmpbuf, 10, buffer, TRUE)) {
		RT_802_11_WDS_ENTRY *pWdsEntry;

		switch (os_str_tol(tmpbuf, 0, 10)) {
		case WDS_BRIDGE_MODE: /* Bridge mode, DisAllow association(stop Beacon generation and Probe Req. */
			pAd->WdsTab.Mode = WDS_BRIDGE_MODE;
			break;

		case WDS_RESTRICT_MODE:
		case WDS_REPEATER_MODE: /* Repeater mode */
			pAd->WdsTab.Mode = WDS_REPEATER_MODE;
			break;

		case WDS_LAZY_MODE: /* Lazy mode, Auto learn wds entry by same SSID, channel, security policy */
			for (i = 0; i < MAX_WDS_ENTRY; i++) {
				pWdsEntry = &pAd->WdsTab.WdsEntry[i];

				if (pWdsEntry->Valid)
					WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);

				/* When Lazy mode is enabled, the all wds-link shall share the same encryption type and key material */
				if (i > 0) {
					os_move_mem(pWdsEntry->wdev.SecConfig.PSK, pAd->WdsTab.WdsEntry[0].wdev.SecConfig.PSK, sizeof(pWdsEntry->wdev.SecConfig.PSK));
					pWdsEntry->wdev.SecConfig.AKMMap = pAd->WdsTab.WdsEntry[0].wdev.SecConfig.AKMMap;
					pWdsEntry->wdev.SecConfig.PairwiseCipher = pAd->WdsTab.WdsEntry[0].wdev.SecConfig.PairwiseCipher;
					pWdsEntry->wdev.SecConfig.PairwiseKeyId = pAd->WdsTab.WdsEntry[0].wdev.SecConfig.PairwiseKeyId;
				}
			}

			pAd->WdsTab.Mode = WDS_LAZY_MODE;
			break;

		case WDS_DISABLE_MODE: /* Disable mode */
		default:
			APWdsInitialize(pAd);
			pAd->WdsTab.Mode = WDS_DISABLE_MODE;
			break;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WDS-Enable mode=%d\n", pAd->WdsTab.Mode));
	}

#ifdef WDS_VLAN_SUPPORT

	/* WdsVlan */
	if (RTMPGetKeyParameter("WDS_VLANID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL, ";"), i++) {
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID = os_str_tol(macptr, 0, 10);
			pAd->WdsTab.WdsEntry[i].wdev.VLAN_Priority = 0;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("If/wds%d - WdsVlanId=%d\n", i, pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID));
		}
	}

#endif /* WDS_VLAN_SUPPORT */
}

VOID wds_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	MAC_TABLE_ENTRY *pMacEntry = pTxBlk->pMacEntry;

	if (TX_BLK_TEST_FLAG(pTxBlk, fTX_bClearEAPFrame)) {
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
		pTxBlk->pKey =  NULL;
	} else if (pMacEntry) {
		struct _SECURITY_CONFIG *pSecConfig = &wdev->SecConfig;

		pTxBlk->CipherAlg = pSecConfig->PairwiseCipher;
		pTxBlk->KeyIdx =  pSecConfig->PairwiseKeyId;
		if (IS_CIPHER_WEP(pSecConfig->PairwiseCipher))
			pTxBlk->pKey = pSecConfig->WepKey[pTxBlk->KeyIdx].Key;
		else
			pTxBlk->pKey = &pSecConfig->PTK[LEN_PTK_KCK + LEN_PTK_KEK];
	}

	/* For  BMcast pMacEntry is not initial */
	if (pTxBlk->CipherAlg == 0x0)
		SET_CIPHER_NONE(pTxBlk->CipherAlg);
}

static struct wifi_dev_ops wds_wdev_ops = {
	.tx_pkt_allowed = wds_tx_pkt_allowed,
	.fp_tx_pkt_allowed = wds_fp_tx_pkt_allowed,
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
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = ap_rx_pkt_allowed,
	.rx_pkt_foward = wds_rx_foward_handle,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = wds_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
};

VOID WDS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	INT index;
	PNET_DEV pWdsNetDev;
	struct wifi_dev *wdev;
	RT_802_11_WDS_ENTRY *wds_entry;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->WdsTab.flg_wds_init != FALSE) {
		for (index = 0; index < MAX_WDS_ENTRY; index++) {
			wds_entry = &pAd->WdsTab.WdsEntry[index];
			wdev = &wds_entry->wdev;
			wdev->PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, wds_entry);
			update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		}

		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():\n", __func__));

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name;
		INT32 Ret;
#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		dev_name = get_dev_name_prefix(pAd, INT_WDS);
		pWdsNetDev = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_WDS, index,
						sizeof(struct mt_dev_priv), dev_name, TRUE);
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		wds_entry = &pAd->WdsTab.WdsEntry[index];
		wdev = &wds_entry->wdev;

		if (pWdsNetDev == NULL) {
			/* allocation fail, exit */
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Allocate network device fail (WDS)...\n"));
			break;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X, MacTabMatchWCID = %d\n",
				 PRINT_MAC(pAd->MacTab.Content[wds_entry->MacTabMatchWCID].Addr), wds_entry->MacTabMatchWCID));
		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));
		Ret = wdev_init(pAd, wdev, WDEV_TYPE_WDS, pWdsNetDev, index, wds_entry, (VOID *)pAd);

		if (Ret == FALSE) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev)));
			RtmpOSNetDevFree(pWdsNetDev);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_WDS, &wds_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev)));
			RtmpOSNetDevFree(pWdsNetDev);
			break;
		}

		if (pAd->WdsTab.Mode >= WDS_LAZY_MODE)
			wds_entry->PhyOpMode = 0xff;

		wdev->PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, wds_entry);
		update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev);
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
		/*update rate info for wdev*/
		SetCommonHtVht(pAd, wdev);
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
		NdisMoveMemory(&wdev->if_addr[0], &pNetDevOps->devAddr[0], MAC_ADDR_LEN);
		os_move_mem(wdev->bss_info_argument.Bssid, wdev->if_addr, MAC_ADDR_LEN);
		RTMP_OS_NETDEV_SET_PRIV(pWdsNetDev, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pWdsNetDev, wdev);
		pNetDevOps->priv_flags = INT_WDS;
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		/* Register this device */
		RtmpOSNetDevAttach(pAd->OpMode, pWdsNetDev, pNetDevOps);
	}

	NdisAllocateSpinLock(pAd, &pAd->WdsTab.WdsTabLock);

	if (index > 0) {
		NdisAcquireSpinLock(&pAd->WdsTab.WdsTabLock);
		pAd->WdsTab.flg_wds_init = TRUE;
		NdisReleaseSpinLock(&pAd->WdsTab.WdsTabLock);
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Total allocated %d WDS interfaces!\n", index));
}


VOID WDS_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():\n", __func__));

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		wdev = &pAd->WdsTab.WdsEntry[index].wdev;

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);
			/* Clear it as NULL to prevent latter access error. */
			pAd->WdsTab.flg_wds_init = FALSE;
			wdev->if_dev = NULL;
		}
	}
}


BOOLEAN WDS_StatsGet(RTMP_ADAPTER *pAd, RT_CMD_STATS *pStats)
{
	INT WDS_apidx = 0, index;
	RT_802_11_WDS_ENTRY *wds_entry;

	for (index = 0; index < MAX_WDS_ENTRY; index++) {
		if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pStats->pNetDev) {
			WDS_apidx = index;
			break;
		}
	}

	if (index >= MAX_WDS_ENTRY) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): can not find wds I/F\n", __func__));
		return FALSE;
	}

	wds_entry = &pAd->WdsTab.WdsEntry[WDS_apidx];
	pStats->pStats = pAd->stats;
	pStats->rx_packets = wds_entry->WdsCounter.ReceivedFragmentCount.QuadPart;
	pStats->tx_packets = wds_entry->WdsCounter.TransmittedFragmentCount.QuadPart;
	pStats->rx_bytes = wds_entry->WdsCounter.ReceivedByteCount;
	pStats->tx_bytes = wds_entry->WdsCounter.TransmittedByteCount;
	pStats->rx_errors = wds_entry->WdsCounter.RxErrorCount;
	pStats->tx_errors = wds_entry->WdsCounter.TxErrors;
	pStats->multicast = wds_entry->WdsCounter.MulticastReceivedFrameCount.QuadPart;   /* multicast packets received */
	pStats->collisions = 0;  /* Collision packets */
	pStats->rx_over_errors = wds_entry->WdsCounter.RxNoBuffer;                   /* receiver ring buff overflow */
	pStats->rx_crc_errors = 0;/*pAd->WlanCounters[0].FCSErrorCount;     // recved pkt with crc error */
	pStats->rx_frame_errors = 0; /* recv'd frame alignment error */
	pStats->rx_fifo_errors = wds_entry->WdsCounter.RxNoBuffer;                   /* recv'r fifo overrun */
	return TRUE;
}

UCHAR WdsPhyOpModeToSuppPhyMode(PRTMP_ADAPTER pAd, PRT_802_11_WDS_ENTRY pWdsEntry)
{
	UCHAR ApPhyMode = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;
	UCHAR SupportPhyMode = 0;

	if (pWdsEntry->PhyOpMode == Legacy_CCK)
		SupportPhyMode = (WMODE_B) & ApPhyMode;
	else if (pWdsEntry->PhyOpMode == Legacy_OFDM)
		SupportPhyMode = (WMODE_B | WMODE_G | WMODE_A) & ApPhyMode;
	else if (pWdsEntry->PhyOpMode == HT_MIXED)
		SupportPhyMode = (WMODE_B | WMODE_G | WMODE_GN | WMODE_A | WMODE_AN) & ApPhyMode;
	else if (pWdsEntry->PhyOpMode == HT_GF)
		SupportPhyMode = (WMODE_GN | WMODE_AN) & ApPhyMode;
	else if (pWdsEntry->PhyOpMode == VHT)
		SupportPhyMode = (WMODE_A | WMODE_AN | WMODE_AC) & ApPhyMode;
	else
		SupportPhyMode = ApPhyMode;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): PhyOpMode=%d, SupportPhyMode = 0x%x, ApPhyMode = 0x%x\n", __func__,
			 pWdsEntry->PhyOpMode,
			 SupportPhyMode,
			 ApPhyMode));

	if (SupportPhyMode == 0) {
		SupportPhyMode = ApPhyMode;

		if (WMODE_CAP_AC(ApPhyMode))
			pWdsEntry->PhyOpMode = VHT;
		else if (WMODE_CAP_N(ApPhyMode)) {
			if (WMODE_HT_ONLY(ApPhyMode))
				pWdsEntry->PhyOpMode = HT_GF;
			else
				pWdsEntry->PhyOpMode = HT_MIXED;
		} else {
			if (ApPhyMode & (WMODE_G | WMODE_A))
				pWdsEntry->PhyOpMode = Legacy_OFDM;
			else
				pWdsEntry->PhyOpMode = Legacy_CCK;
		}

		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("!!! WARNING !!! No matched PhyMode can be found, align to default 0x%x\n",
				 ApPhyMode));
	}

	return SupportPhyMode;
}

UCHAR WdsPhyOpModeToSuppRateMode(PRTMP_ADAPTER pAd, PRT_802_11_WDS_ENTRY pWdsEntry)
{
	UCHAR ApPhyMode = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.PhyMode;
	UCHAR SupportRateMode = 0;

	switch (pWdsEntry->PhyOpMode) {
	case 0xff: /* user doesn't specific a Mode for WDS link. */
	case MODE_OFDM: /* specific OFDM mode. */
		SupportRateMode = SUPPORT_OFDM_MODE;

		if (WMODE_CAP_2G(ApPhyMode))
			SupportRateMode |= SUPPORT_CCK_MODE;

		break;

	case MODE_CCK:
		SupportRateMode = SUPPORT_CCK_MODE;
		break;
#ifdef DOT11_N_SUPPORT

	case MODE_HTMIX:
	case MODE_HTGREENFIELD:
		SupportRateMode = (SUPPORT_HT_MODE | SUPPORT_OFDM_MODE);

		if (ApPhyMode)
			SupportRateMode |= SUPPORT_CCK_MODE;

		break;
#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

	case MODE_VHT:
		SupportRateMode = (SUPPORT_VHT_MODE | SUPPORT_HT_MODE | SUPPORT_OFDM_MODE);
		break;
#endif /* DOT11_VHT_AC */

	default:
		break;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): PhyOpMode=%d, SupportRateMode = 0x%x, ApPhyMode = 0x%x\n", __func__,
			 pWdsEntry->PhyOpMode,
			 SupportRateMode,
			 ApPhyMode));
	return SupportRateMode;
}

/*
* WDS_Open
*/
INT wds_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;


	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() wdev is NULL\n", __func__));
		return FALSE;
	}

	pAd =  (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */

	wds_entry = &pAd->WdsTab.WdsEntry[wdev->func_idx];
	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("WDS inf up for wds_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx));

	RTMPSetIndividualHT(pAd, wdev->func_idx + MIN_NET_DEVICE_FOR_WDS);

	if (wds_entry->Valid)
		AsicUpdateWdsRxWCIDTable(pAd, wdev->func_idx);


	return TRUE;
}

/*
* WDS_Close
*/
INT wds_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	UCHAR wcid;


	if (wdev == NULL) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s() wdev is NULL\n", __func__));
		return FALSE;
	}

	pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */

	wds_entry = &pAd->WdsTab.WdsEntry[wdev->func_idx];
	if (WdsTableLookup(pAd, wds_entry->PeerWdsAddr, TRUE)) {
		wcid = wds_entry->MacTabMatchWCID;
		wds_bss_linkdown(pAd, wcid);
	}

	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));
		return FALSE;
	}


	return TRUE;
}

/*! \brief   To substitute the message type if the message is coming from external
 *  \param  *Fr            The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN WdsMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT PINT Machine,
	OUT PINT MsgType)
{
	/* wds data packet */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
		if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s, AP WDS recv UC data from %02x-%02x-%02x-%02x-%02x-%02x\n",
				__func__, PRINT_MAC(pFrame->Hdr.Addr2)));

			*Machine = WDS_STATE_MACHINE;
			*MsgType = APMT2_WDS_RECV_UC_DATA;

			return TRUE;
		}
	}

	return FALSE;
}

VOID ap_wds_rcv_uc_data_action(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem)
{
	PFRAME_802_11 pFrame = (PFRAME_802_11)Elem->Msg;
	MAC_TABLE_ENTRY *pEntry;
	RT_802_11_WDS_ENTRY *wds_entry;

	RETURN_IF_PAD_NULL(pAd);

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): PhyMode = %d\n", __func__, Elem->RxPhyMode));

	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, Elem->Wcid, pFrame->Hdr.Addr2, TRUE);
	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pFrame->Hdr.Addr2, TRUE);

	/* Only Lazy mode will auto learning, match with FrDs=1 and ToDs=1 */
	if ((pEntry == NULL) && (pAd->WdsTab.Mode >= WDS_LAZY_MODE)) {
		INT WdsIdx = WdsEntryAlloc(pAd, pFrame->Hdr.Addr2);

		if (WdsIdx >= 0 && WdsIdx < MAX_WDS_ENTRY) {
			wds_entry = &pAd->WdsTab.WdsEntry[WdsIdx];

			/* user doesn't specific a phy mode for WDS link. */
			if (wds_entry->PhyOpMode == 0xff) {
			    UINT32 encrypt_mode = wds_entry->wdev.SecConfig.PairwiseCipher;

			    if ((pAd->CommonCfg.HT_DisallowTKIP) && IS_INVALID_HT_SECURITY(encrypt_mode))
					wds_entry->PhyOpMode =
						(wds_entry->PhyOpMode >= MODE_OFDM) ? MODE_OFDM : MODE_CCK;
				else
					wds_entry->PhyOpMode = Elem->RxPhyMode;
				wds_entry->wdev.PhyMode = WdsPhyOpModeToSuppPhyMode(pAd, wds_entry);
			}
			pEntry = MacTableInsertWDSEntry(pAd, pFrame->Hdr.Addr2, (UCHAR)WdsIdx);

			if (!pEntry) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): can't insert new pEntry\n", __func__));
				return;
			}

			pEntry->SupportRateMode = WdsPhyOpModeToSuppRateMode(pAd, wds_entry);
			RAInit(pAd, pEntry);
		}
	}
}

VOID ap_wds_bss_linkdown(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MLME_QUEUE_ELEM *Elem)
{
	struct wifi_dev *wdev;
	USHORT wcid;
	MAC_TABLE_ENTRY *pEntry;

	wdev = Elem->wdev;
	if (wdev_do_linkdown(wdev) != TRUE)
		 MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): linkdown fail!!\n", __func__));

	NdisMoveMemory(&wcid, Elem->Msg, sizeof(USHORT));
	pEntry = &pAd->MacTab.Content[wcid];
	mac_entry_delete(pAd, pEntry);

	return;
}

VOID WdsStateMachineInit(
	IN RTMP_ADAPTER * pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[])
{
	StateMachineInit(Sm, (STATE_MACHINE_FUNC *)Trans, WDS_MAX_STATE, WDS_MAX_MSG,
		(STATE_MACHINE_FUNC)Drop, WDS_IDLE, WDS_MACHINE_BASE);
	StateMachineSetAction(Sm, WDS_IDLE, APMT2_WDS_RECV_UC_DATA,
		(STATE_MACHINE_FUNC)ap_wds_rcv_uc_data_action);
	StateMachineSetAction(Sm, WDS_IDLE, WDS_BSS_LINKDOWN,
		(STATE_MACHINE_FUNC)ap_wds_bss_linkdown);
}

#endif /* WDS_SUPPORT */

