
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


#define VAILD_KEY_INDEX( _X ) ((((_X) >= 0) && ((_X) < 4)) ? (TRUE) : (FALSE))


INT ApWdsAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR *pWcid)
{
	UCHAR idx;
	INT allowed = FALSE;
	RT_802_11_WDS_ENTRY *wds_entry;

	if (!wdev)
		return FALSE;

	for (idx = 0; idx < MAX_WDS_ENTRY; idx++)
	{
		wds_entry = &pAd->WdsTab.WdsEntry[idx];
		if (ValidWdsEntry(pAd, idx) && (wdev == (&wds_entry->wdev)))
		{
			RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
			*pWcid = (UCHAR)pAd->WdsTab.WdsEntry[idx].MacTabMatchWCID;
			allowed = TRUE;

			break;
		}
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


INT wds_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk)
{
	HEADER_802_11 *pHeader = pRxBlk->pHeader;
	MAC_TABLE_ENTRY *pEntry = NULL;
	INT hdr_len = 0;

	/* handle WDS */
	if (VALID_WCID(pRxBlk->wcid)
		&& IS_ENTRY_WDS(&pAd->MacTab.Content[pRxBlk->wcid]))
	{
		if (MAC_ADDR_EQUAL(pHeader->Addr1, pAd->CurrentAddress))
			pEntry = FindWdsEntry(pAd, pRxBlk->wcid, pHeader->Addr2, pRxBlk->rx_rate.field.MODE);
		else
			pEntry = NULL;

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

		RX_BLK_SET_FLAG(pRxBlk, fRX_WDS);
		hdr_len = LENGTH_802_11_WITH_ADDR4;

		return hdr_len;
	}

	return FALSE;
}


INT WdsEntryAlloc(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i, WdsTabIdx = -1;
	RT_802_11_WDS_ENTRY *wds_entry;

	NdisAcquireSpinLock(&pAd->WdsTabLock);
	for (i = 0; i < MAX_WDS_ENTRY; i++)
	{
		if ((pAd->WdsTab.Mode >= WDS_LAZY_MODE) && !WDS_IF_UP_CHECK(pAd, i))
			continue;

		wds_entry = &pAd->WdsTab.WdsEntry[i];
		if (wds_entry->Valid == FALSE)
		{
			wds_entry->Valid = TRUE;
			pAd->WdsTab.Size ++;
			COPY_MAC_ADDR(wds_entry->PeerWdsAddr, pAddr);
			WdsTabIdx = i;
			break;
		}
		else if (MAC_ADDR_EQUAL(wds_entry->PeerWdsAddr, pAddr))
		{
			WdsTabIdx = i;
			break;
		}
	}
	NdisReleaseSpinLock(&pAd->WdsTabLock);
	
	if (i == MAX_WDS_ENTRY)
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Unable to allocate WdsEntry.\n", __FUNCTION__));

	return WdsTabIdx;
}


VOID WdsEntryDel(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	/* delete one WDS entry */
	NdisAcquireSpinLock(&pAd->WdsTabLock);

	for (i = 0; i < MAX_WDS_ENTRY; i++)
	{
		wds_entry = &pAd->WdsTab.WdsEntry[i];
		if (MAC_ADDR_EQUAL(pAddr, wds_entry->PeerWdsAddr) && (wds_entry->Valid == TRUE))
		{
			wds_entry->Valid = FALSE;
			NdisZeroMemory(wds_entry->PeerWdsAddr, MAC_ADDR_LEN);
			pAd->WdsTab.Size--;
			break;
		}
	}

	NdisReleaseSpinLock(&pAd->WdsTabLock);
}


/*
	==========================================================================
	Description:
		Delete all WDS Entry in pAd->MacTab
	==========================================================================
 */
BOOLEAN MacTableDeleteWDSEntry(
	IN PRTMP_ADAPTER pAd,
	IN USHORT wcid,
	IN PUCHAR pAddr)
{
	if (wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;
	else
		return MacTableDeleteEntry(pAd, wcid, pAddr);
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


	/* if FULL, return */
	if (pAd->MacTab.Size >= MAX_LEN_OF_MAC_TABLE)
		return NULL;

	if((pEntry = WdsTableLookup(pAd, pAddr, TRUE)) != NULL)
		return pEntry;

	wds_entry = &pAd->WdsTab.WdsEntry[WdsTabIdx];
	wdev = &wds_entry->wdev;
		
	/* allocate one WDS entry */
	do
	{
		/* allocate one MAC entry */
		pEntry = MacTableInsertEntry(pAd, pAddr, wdev, ENTRY_WDS, OPMODE_AP, TRUE);
		if (pEntry)
		{
			tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

			/* specific Max Tx Rate for Wds link. */
			NdisZeroMemory(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));
			switch (wdev->PhyMode)
			{
				case 0xff: /* user doesn't specific a Mode for WDS link. */
				case MODE_OFDM: /* specific OFDM mode. */
					HTPhyMode.field.MODE = MODE_OFDM;
					HTPhyMode.field.MCS = 7;
					pEntry->RateLen = 8;
					break;

				case MODE_CCK:
					HTPhyMode.field.MODE = MODE_CCK;
					HTPhyMode.field.MCS = 3;
					pEntry->RateLen = 4;
					break;

#ifdef DOT11_N_SUPPORT
				case MODE_HTMIX:
					HTPhyMode.field.MCS = 7;
					HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
					HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
					HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
					HTPhyMode.field.MODE = MODE_HTMIX;
					pEntry->RateLen = 12;
					break;

				case MODE_HTGREENFIELD:
					HTPhyMode.field.MCS = 7;
					HTPhyMode.field.ShortGI = wdev->HTPhyMode.field.ShortGI;
					HTPhyMode.field.BW = wdev->HTPhyMode.field.BW;
					HTPhyMode.field.STBC = wdev->HTPhyMode.field.STBC;
					HTPhyMode.field.MODE = MODE_HTGREENFIELD;
					pEntry->RateLen = 12;
					break;
#endif /* DOT11_N_SUPPORT */

				default:
					break;
			}

			pEntry->MaxHTPhyMode.word = HTPhyMode.word;
			pEntry->MinHTPhyMode.word = wdev->MinHTPhyMode.word;
			pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;
			/* default */
			pEntry->MpduDensity = 5;
			pEntry->MaxRAmpduFactor = 3;

#ifdef DOT11_N_SUPPORT
			if (wdev->PhyMode >= MODE_HTMIX)
			{
				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("IF-wds%d : Desired MCS = %d\n", WdsTabIdx,
					wdev->DesiredTransmitSetting.field.MCS));

					set_ht_fixed_mcs(pAd, pEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pEntry->MmpsMode = MMPS_DISABLE;
				NdisMoveMemory(&pEntry->HTCapability, &pAd->CommonCfg.HtCapability, sizeof(HT_CAPABILITY_IE));
				if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
					CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);

				set_sta_ht_cap(pAd, pEntry, &pEntry->HTCapability);
				
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
#endif /* DOT11_N_SUPPORT */
			else
			{
				NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
			}

			// for now, we set this by default!
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			if (wdev->bAutoTxRateSwitch == FALSE)
			{
				pEntry->HTPhyMode.field.MCS = wdev->DesiredTransmitSetting.field.MCS;
				pEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pEntry);
			}
			else
			{
				// TODO: shiang-MT7603, fix me for this, because we may need to set this only when we have WTBL entry for tx_rate!
				pEntry->bAutoTxRateSwitch = TRUE;
			}
			
			wds_entry->MacTabMatchWCID = (UCHAR)pEntry->wcid;
			pEntry->func_tb_idx = WdsTabIdx;
			pEntry->wdev = wdev;
			COPY_MAC_ADDR(&wdev->bssid[0], &pEntry->Addr[0]);

			AsicUpdateRxWCIDTable(pAd, pEntry->wcid, pAddr);
			AsicUpdateWdsEncryption(pAd, pEntry->wcid);
			
			DBGPRINT(RT_DEBUG_OFF, ("%s() - allocate entry #%d(link to WCID %d), Total= %d\n",
						__FUNCTION__, WdsTabIdx, wds_entry->MacTabMatchWCID, pAd->WdsTab.Size));
			break;
		}
	}while(FALSE);

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

	if (wcid <=0 || wcid >= MAX_LEN_OF_MAC_TABLE )
		return NULL;

	NdisAcquireSpinLock(&pAd->WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);

	do
	{
		pCurEntry = &pAd->MacTab.Content[wcid];
		WdsIndex = 0xff;
		if ((pCurEntry) && IS_ENTRY_WDS(pCurEntry))
			WdsIndex = pCurEntry->func_tb_idx;

		if (WdsIndex == 0xff)
			break;

		if (pAd->WdsTab.WdsEntry[WdsIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddr))
		{
			if(bResetIdelCount) {
				pCurEntry->NoDataIdleCount = 0;
				// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
				pAd->MacTab.tr_entry[pCurEntry->wcid].NoDataIdleCount = 0;
			}
			pEntry = pCurEntry;
			break;
		}
	} while(FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTabLock);

	return pEntry;
}


MAC_TABLE_ENTRY *WdsTableLookup(RTMP_ADAPTER *pAd, UCHAR *addr, BOOLEAN bResetIdelCount)
{
	USHORT HashIdx;
	PMAC_TABLE_ENTRY pEntry = NULL;

	NdisAcquireSpinLock(&pAd->WdsTabLock);
	NdisAcquireSpinLock(&pAd->MacTabLock);

	HashIdx = MAC_ADDR_HASH_INDEX(addr);
	pEntry = pAd->MacTab.Hash[HashIdx];

	while (pEntry)
	{
		if (IS_ENTRY_WDS(pEntry) && MAC_ADDR_EQUAL(pEntry->Addr, addr))
		{
			if(bResetIdelCount) {
				pEntry->NoDataIdleCount = 0;
				// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
				pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount = 0;
			}
			break;
		}
		else
			pEntry = pEntry->pNext;
	}

	NdisReleaseSpinLock(&pAd->MacTabLock);
	NdisReleaseSpinLock(&pAd->WdsTabLock);

	return pEntry;
}


MAC_TABLE_ENTRY *FindWdsEntry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UINT32 PhyMode)
{
	MAC_TABLE_ENTRY *pEntry;
	RT_802_11_WDS_ENTRY *wds_entry;

	/* lookup the match wds entry for the incoming packet. */
	pEntry = WdsTableLookupByWcid(pAd, Wcid, pAddr, TRUE);
	if (pEntry == NULL)
		pEntry = WdsTableLookup(pAd, pAddr, TRUE);

	/* Only Lazy mode will auto learning, match with FrDs=1 and ToDs=1 */
	if((pEntry == NULL) && (pAd->WdsTab.Mode >= WDS_LAZY_MODE))
	{
		INT WdsIdx = WdsEntryAlloc(pAd, pAddr);
		if (WdsIdx >= 0 && WdsIdx < MAX_WDS_ENTRY)
		{
			wds_entry = &pAd->WdsTab.WdsEntry[WdsIdx];

			/* user doesn't specific a phy mode for WDS link. */
			if (wds_entry->wdev.PhyMode == 0xff)
				wds_entry->wdev.PhyMode = PhyMode;
			pEntry = MacTableInsertWDSEntry(pAd, pAddr, (UCHAR)WdsIdx);

			RTMPSetSupportMCS(pAd,
							OPMODE_AP,
							pEntry,
							pAd->CommonCfg.SupRate,
							pAd->CommonCfg.SupRateLen,
							pAd->CommonCfg.ExtRate,
							pAd->CommonCfg.ExtRateLen,
#ifdef DOT11_VHT_AC
							0,
							NULL,
#endif /* DOT11_VHT_AC */
							&pAd->CommonCfg.HtCapability,
							sizeof(pAd->CommonCfg.HtCapability));
		}
		else 
			pEntry = NULL;
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

	if (pAd->WdsTab.Mode == WDS_DISABLE_MODE)
		return;

	for (idx = 0; idx < pAd->WdsTab.Size; idx++)
	{
		UCHAR wcid = pAd->WdsTab.WdsEntry[idx].MacTabMatchWCID;
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[wcid];

		if(!IS_ENTRY_WDS(pEntry))
			continue;

		NdisAcquireSpinLock(&pAd->WdsTabLock);
		NdisAcquireSpinLock(&pAd->MacTabLock);
		pEntry->NoDataIdleCount ++;
		// TODO: shiang-usw,  remove upper setting becasue we need to migrate to tr_entry!
		pAd->MacTab.tr_entry[pEntry->wcid].NoDataIdleCount++;
		NdisReleaseSpinLock(&pAd->MacTabLock);
		NdisReleaseSpinLock(&pAd->WdsTabLock);

		/* delete those MAC entry that has been idle for a long time */
		if (pEntry->NoDataIdleCount >= MAC_TABLE_AGEOUT_TIME)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ageout %02x:%02x:%02x:%02x:%02x:%02x from WDS #%d after %d-sec silence\n",
					PRINT_MAC(pEntry->Addr), idx, MAC_TABLE_AGEOUT_TIME));
			if (pAd->WdsTab.Mode != WDS_LAZY_MODE) {
				pEntry->NoDataIdleCount = 0;
				BASessionTearDownALL(pAd, pEntry->wcid);
			} else {
				WdsEntryDel(pAd, pEntry->Addr);
				MacTableDeleteWDSEntry(pAd, pEntry->wcid, pEntry->Addr);
			}
		}
	}

}


VOID RT28xx_WDS_Close(RTMP_ADAPTER *pAd)
{
	UINT index;

	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		if (pAd->WdsTab.WdsEntry[index].wdev.if_dev)
			RtmpOSNetDevClose(pAd->WdsTab.WdsEntry[index].wdev.if_dev);
	}
	return;
}




VOID WdsDown(RTMP_ADAPTER *pAd)
{
	int i;

	for (i=0; i<MAX_WDS_ENTRY; i++)
	{
		if(WdsTableLookup(pAd, pAd->WdsTab.WdsEntry[i].PeerWdsAddr, TRUE))
			MacTableDeleteWDSEntry(pAd, pAd->WdsTab.WdsEntry[i].MacTabMatchWCID,
				pAd->WdsTab.WdsEntry[i].PeerWdsAddr);
	}
}


VOID AsicUpdateWdsRxWCIDTable(RTMP_ADAPTER *pAd)
{
	UINT index;
	MAC_TABLE_ENTRY *pEntry = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	
	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		wds_entry = &pAd->WdsTab.WdsEntry[index];
		if (pAd->WdsTab.Mode >= WDS_LAZY_MODE) {
			wds_entry->wdev.PhyMode = 0xff;
			if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
				wds_entry->wdev.PhyMode = MODE_HTMIX;
			else {
				if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
					wds_entry->wdev.PhyMode = MODE_CCK;
				else
					wds_entry->wdev.PhyMode = MODE_OFDM;
			}
		}

		if (wds_entry->Valid != TRUE)
			continue;

		pEntry = MacTableInsertWDSEntry(pAd, wds_entry->PeerWdsAddr, index);

		RTMPSetSupportMCS(pAd,
						OPMODE_AP,
						pEntry,
						pAd->CommonCfg.SupRate,
						pAd->CommonCfg.SupRateLen,
						pAd->CommonCfg.ExtRate,
						pAd->CommonCfg.ExtRateLen,
#ifdef DOT11_VHT_AC
						0,
						NULL,
#endif /* DOT11_VHT_AC */
						&pAd->CommonCfg.HtCapability,
						sizeof(pAd->CommonCfg.HtCapability));

		switch (wds_entry->wdev.PhyMode)
		{
			case 0xff: /* user doesn't specific a Mode for WDS link. */
			case MODE_OFDM: /* specific OFDM mode. */
				pEntry->SupportRateMode = SUPPORT_OFDM_MODE;
				if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
					pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;

			case MODE_CCK:
				pEntry->SupportRateMode = SUPPORT_CCK_MODE;
				break;

#ifdef DOT11_N_SUPPORT
			case MODE_HTMIX:
			case MODE_HTGREENFIELD:
				pEntry->SupportRateMode = (SUPPORT_HT_MODE | SUPPORT_OFDM_MODE);
				if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
					pEntry->SupportRateMode |= SUPPORT_CCK_MODE;
				break;
#endif /* DOT11_N_SUPPORT */
			default:
				break;
		}
	}

	return;
}


VOID AsicUpdateWdsEncryption(RTMP_ADAPTER *pAd, UCHAR wcid)
{
	UINT WdsIdex;
	PMAC_TABLE_ENTRY pEntry = NULL;
	RT_802_11_WDS_ENTRY *wds_entry;
	struct wifi_dev *wdev;

	do
	{
		if (wcid >= MAX_LEN_OF_MAC_TABLE)
			break;

		pEntry = &pAd->MacTab.Content[wcid];
		if (pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].Valid != TRUE)
			break;

		if (!IS_ENTRY_WDS(pEntry))
			break;

		WdsIdex = pEntry->func_tb_idx;
		wds_entry = &pAd->WdsTab.WdsEntry[WdsIdex];
		wdev = &wds_entry->wdev;
		if (((wdev->WepStatus == Ndis802_11WEPEnabled) || 
			(wdev->WepStatus == Ndis802_11TKIPEnable) ||
			(wdev->WepStatus == Ndis802_11AESEnable))
			&& (wds_entry->WdsKey.KeyLen > 0))
		{
			
			INT DefaultKeyId = 0;

			if (wdev->WepStatus == Ndis802_11WEPEnabled)
				DefaultKeyId = wds_entry->KeyIdx;

			if (!VAILD_KEY_INDEX(DefaultKeyId))
				break;

			/* Update key into Asic Pairwise key table */
			RTMP_ASIC_PAIRWISE_KEY_TABLE(
				pAd,
				pEntry->wcid,
				&wds_entry->WdsKey);

			/* update WCID attribute table and IVEIV table for this entry */
			RTMP_SET_WCID_SEC_INFO(
				pAd, 
				MAIN_MBSSID + MIN_NET_DEVICE_FOR_WDS,
				DefaultKeyId, 
				wds_entry->WdsKey.CipherAlg,
				pEntry->wcid, 
				PAIRWISEKEY);

#ifdef MT_MAC
			if (pAd->chipCap.hif_type == HIF_MT)
				CmdProcAddRemoveKey(pAd, 0, pEntry->func_tb_idx, DefaultKeyId, pEntry->wcid, PAIRWISEKEYTABLE, &wds_entry->WdsKey, pEntry->Addr);
#endif /* MT_MAC */
		}
	} while (FALSE);

	return;
}


VOID WdsPeerBeaconProc(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN USHORT CapabilityInfo,
	IN UCHAR MaxSupportedRateIn500Kbps,
	IN UCHAR MaxSupportedRateLen,
	IN BOOLEAN bWmmCapable,
	IN ULONG ClientRalinkIe,
	IN HT_CAPABILITY_IE *pHtCapability,
	IN UCHAR HtCapabilityLen)
{
	UCHAR MaxSupportedRate = RATE_11;

	if (pEntry && IS_ENTRY_WDS(pEntry))
	{
		MaxSupportedRate = dot11_2_ra_rate(MaxSupportedRateIn500Kbps);
		pEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
		pEntry->RateLen = MaxSupportedRateLen;

		pEntry->CapabilityInfo = CapabilityInfo;
		set_sta_ra_cap(pAd, pEntry, ClientRalinkIe);

		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
			pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
			pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
			pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		}
		else
		{
			pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
			pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
			pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
			pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		}

#ifdef DOT11_N_SUPPORT
		/* If this Entry supports 802.11n, upgrade to HT rate. */
		if ((HtCapabilityLen != 0) 
			&& WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		{
			ht_mode_adjust(pAd, pEntry, pHtCapability, &pAd->CommonCfg.DesiredHtPhy);

			/* find max fixed rate */
			pEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.DesiredHtPhyInfo.MCSSet[0],
																&pHtCapability->MCSSet[0]);

			if ((pEntry->MaxHTPhyMode.field.MCS > pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.HTPhyMode.field.MCS) && (pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.HTPhyMode.field.MCS != MCS_AUTO))
				pEntry->MaxHTPhyMode.field.MCS = pAd->WdsTab.WdsEntry[pEntry->func_tb_idx].wdev.HTPhyMode.field.MCS;
			pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
			pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
			pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
			pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
			pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;
			if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_AMSDU_INUSED);

			set_sta_ht_cap(pAd, pEntry, pHtCapability);

			NdisMoveMemory(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));
		}
		else
#endif /* DOT11_N_SUPPORT */
		{
			pEntry->MaxHTPhyMode.field.BW = BW_20;
			pEntry->MinHTPhyMode.field.BW = BW_20;
#ifdef DOT11_N_SUPPORT
			pEntry->HTCapability.MCSSet[0] = 0;
			pEntry->HTCapability.MCSSet[1] = 0;
			NdisZeroMemory(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
			pAd->MacTab.fAnyStationIsLegacy = TRUE;
#endif /* DOT11_N_SUPPORT */
		}

		CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);

		if (bWmmCapable
#ifdef DOT11_N_SUPPORT
			|| (pEntry->MaxHTPhyMode.field.MODE >= MODE_HTMIX)
#endif /* DOT11_N_SUPPORT */
			)
		{
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		}
		else
		{
			CLIENT_STATUS_CLEAR_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		}

		pEntry->HTPhyMode.field.MODE = pEntry->MaxHTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.STBC = pEntry->MaxHTPhyMode.field.STBC;
		pEntry->HTPhyMode.field.ShortGI = pEntry->MaxHTPhyMode.field.ShortGI;
		pEntry->HTPhyMode.field.BW = pEntry->MaxHTPhyMode.field.BW;

		switch (pEntry->HTPhyMode.field.MODE)
		{
			case MODE_OFDM: /* specific OFDM mode. */
				pEntry->SupportRateMode = SUPPORT_OFDM_MODE;
				break;
	
			case MODE_CCK:
				pEntry->SupportRateMode = SUPPORT_CCK_MODE;
				break;

#ifdef DOT11_N_SUPPORT
			case MODE_HTMIX:
			case MODE_HTGREENFIELD:
				pEntry->SupportRateMode = (SUPPORT_HT_MODE | SUPPORT_OFDM_MODE | SUPPORT_CCK_MODE);
				break;
#endif /* DOT11_N_SUPPORT */

			default:
				break;
		}
	}
}


VOID APWdsInitialize(RTMP_ADAPTER *pAd)
{
	INT i;
	RT_802_11_WDS_ENTRY *wds_entry;

	pAd->WdsTab.Mode = WDS_DISABLE_MODE;
	pAd->WdsTab.Size = 0;
	for (i = 0; i < MAX_WDS_ENTRY; i++)
	{
		wds_entry = &pAd->WdsTab.WdsEntry[i];

		wds_entry->wdev.PhyMode = 0xff;
		wds_entry->wdev.WepStatus = Ndis802_11EncryptionDisabled;
		wds_entry->wdev.bAutoTxRateSwitch = TRUE;
		wds_entry->wdev.DesiredTransmitSetting.field.MCS = MCS_AUTO;

		wds_entry->Valid = FALSE;
		wds_entry->MacTabMatchWCID = 0;
		wds_entry->KeyIdx = 0;
		NdisZeroMemory(&wds_entry->WdsKey, sizeof(CIPHER_KEY));
		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));

	}
	return;	
}


#ifdef DBG
INT Show_WdsTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT 	i;

	for(i = 0; i < MAX_WDS_ENTRY; i++)
	{					
		DBGPRINT(RT_DEBUG_OFF, ("IF/WDS%d-%02x:%02x:%02x:%02x:%02x:%02x(%s) ,%s, KeyId=%d\n", i, 
								PRINT_MAC(pAd->WdsTab.WdsEntry[i].PeerWdsAddr), 
								pAd->WdsTab.WdsEntry[i].Valid == 1 ? "Valid" : "Invalid",
								GetEncryptType(pAd->WdsTab.WdsEntry[i].wdev.WepStatus), 
								pAd->WdsTab.WdsEntry[i].KeyIdx));

		if (pAd->WdsTab.WdsEntry[i].WdsKey.KeyLen > 0)
			hex_dump("Wds Key", pAd->WdsTab.WdsEntry[i].WdsKey.Key, pAd->WdsTab.WdsEntry[i].WdsKey.KeyLen);
	}
	
	DBGPRINT(RT_DEBUG_OFF, ("\n%-19s%-4s%-4s%-4s%-7s%-7s%-7s%-10s%-6s%-6s%-6s%-6s\n",
				"MAC", "IDX", "AID", "PSM", "RSSI0", "RSSI1", "RSSI2", "PhMd", "BW", "MCS", "SGI", "STBC"));
	
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_WDS(pEntry))
		{
			DBGPRINT(RT_DEBUG_OFF, ("%02X:%02X:%02X:%02X:%02X:%02X  ", PRINT_MAC(pEntry->Addr)));
			DBGPRINT(RT_DEBUG_OFF,("%-4d", (int)pEntry->func_tb_idx));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->Aid));
			DBGPRINT(RT_DEBUG_OFF, ("%-4d", (int)pEntry->PsMode));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[0]));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[1]));
			DBGPRINT(RT_DEBUG_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[2]));
			DBGPRINT(RT_DEBUG_OFF, ("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			DBGPRINT(RT_DEBUG_OFF, ("%-6d\n", pEntry->HTPhyMode.field.STBC));
		}
	} 

	return TRUE;
}
#endif /* DBG */

VOID rtmp_read_wds_from_file(RTMP_ADAPTER *pAd, RTMP_STRING *tmpbuf, RTMP_STRING *buffer)
{
	RTMP_STRING *macptr;
	INT			i=0, j;
	RTMP_STRING tok_str[16];
	BOOLEAN		bUsePrevFormat = FALSE;
	UCHAR		macAddress[MAC_ADDR_LEN];
	UCHAR	    keyMaterial[40];	
	UCHAR		KeyLen, CipherAlg = CIPHER_NONE, KeyIdx;
	PRT_802_11_WDS_ENTRY pWdsEntry;
	struct wifi_dev *wdev;


	/*WdsPhyMode */
	if (RTMPGetKeyParameter("WdsPhyMode", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
	{	
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++) 
		{
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
			wdev = &pWdsEntry->wdev;
	        if (rtstrcasecmp(macptr, "CCK") == TRUE)
	            wdev->PhyMode = MODE_CCK;
	        else if (rtstrcasecmp(macptr, "OFDM") == TRUE)
	            wdev->PhyMode = MODE_OFDM;
#ifdef DOT11_N_SUPPORT
	        else if (rtstrcasecmp(macptr, "HTMIX") == TRUE)
	            wdev->PhyMode = MODE_HTMIX;
	        else if (rtstrcasecmp(macptr, "GREENFIELD") == TRUE)
	            wdev->PhyMode = MODE_HTGREENFIELD;
#endif /* DOT11_N_SUPPORT */
	        else
	            wdev->PhyMode = 0xff;
		
			DBGPRINT(RT_DEBUG_TRACE, ("If/wds%d - PeerPhyMode=%d\n", i, wdev->PhyMode));
		}
	}
	
	/*WdsList */
	if (RTMPGetKeyParameter("WdsList", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
	{
		if (pAd->WdsTab.Mode != WDS_LAZY_MODE)
		{
			for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++) 
			{				
				if(strlen(macptr) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
					continue; 
				if(strcmp(macptr,"00:00:00:00:00:00") == 0)
					continue; 
				if(i >= MAX_WDS_ENTRY)
					break; 

				for (j=0; j<MAC_ADDR_LEN; j++)
				{
					AtoH(macptr, &macAddress[j], 1);
					macptr=macptr+3;
				}	

				WdsEntryAlloc(pAd, macAddress);
			}
		}
	}
	/*WdsEncrypType */
	if (RTMPGetKeyParameter("WdsEncrypType", tmpbuf, 128, buffer, TRUE))
	{				
	    for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++)
	    {
		pWdsEntry = &pAd->WdsTab.WdsEntry[i];
		wdev = &pWdsEntry->wdev;
			
	        if (rtstrcasecmp(macptr, "NONE") == TRUE)
	            wdev->WepStatus = Ndis802_11WEPDisabled;
	        else if (rtstrcasecmp(macptr, "WEP") == TRUE)
	            wdev->WepStatus = Ndis802_11WEPEnabled;
	        else if (rtstrcasecmp(macptr, "TKIP") ==TRUE)
	            wdev->WepStatus = Ndis802_11TKIPEnable;
	        else if (rtstrcasecmp(macptr, "AES") == TRUE)
	            wdev->WepStatus = Ndis802_11AESEnable;
	        else
	            wdev->WepStatus = Ndis802_11WEPDisabled;

	        DBGPRINT(RT_DEBUG_TRACE, ("WdsEncrypType[%d]=%d(%s)\n",
					i, wdev->WepStatus, GetEncryptType(wdev->WepStatus)));
	    }
		
		/* Previous WDS only supports single encryption type. */
		/* For backward compatible, other wds link encryption type shall be the same with the first. */
		if (i == 1)
		{
			for (j = 1; j < MAX_WDS_ENTRY; j++)
			{
				wdev = &pAd->WdsTab.WdsEntry[j].wdev;
				wdev->WepStatus = pAd->WdsTab.WdsEntry[0].wdev.WepStatus;	
				DBGPRINT(RT_DEBUG_TRACE, ("WdsEncrypType[%d]=%d(%s)\n",
							j, wdev->WepStatus, GetEncryptType(wdev->WepStatus)));	
			}
		}
			
	}
	/* WdsKey */
	/* This is a previous parameter and it only stores WPA key material, not WEP key */
	if (RTMPGetKeyParameter("WdsKey", tmpbuf, 255, buffer, FALSE))
	{
		for (i = 0; i < MAX_WDS_ENTRY; i++)
			NdisZeroMemory(&pAd->WdsTab.WdsEntry[i].WdsKey, sizeof(CIPHER_KEY));

		if (strlen(tmpbuf) > 0)
			bUsePrevFormat = TRUE;

		wdev = &pAd->WdsTab.WdsEntry[0].wdev;
		pWdsEntry = &pAd->WdsTab.WdsEntry[0];
		/* check if the wds-0 link key material is valid */
		if (((wdev->WepStatus == Ndis802_11TKIPEnable)
			|| (wdev->WepStatus == Ndis802_11AESEnable))
			&& (strlen(tmpbuf) >= 8) && (strlen(tmpbuf) <= 64))
		{
			RT_CfgSetWPAPSKKey(pAd, tmpbuf, strlen(tmpbuf), (PUCHAR)RALINK_PASSPHRASE, sizeof(RALINK_PASSPHRASE), keyMaterial);
			if (wdev->WepStatus == Ndis802_11AESEnable)
				pWdsEntry->WdsKey.CipherAlg = CIPHER_AES;
			else
				pWdsEntry->WdsKey.CipherAlg = CIPHER_TKIP;
			
			NdisMoveMemory(&pWdsEntry->WdsKey.Key, keyMaterial, 16);
			pWdsEntry->WdsKey.KeyLen = 16;
			NdisMoveMemory(&pWdsEntry->WdsKey.RxMic, keyMaterial+16, 8);
			NdisMoveMemory(&pWdsEntry->WdsKey.TxMic, keyMaterial+16, 8);
		}

		/* Previous WDS only supports single key-material. */
		/* For backward compatible, other wds link key-material shall be the same with the first. */
		if (pAd->WdsTab.WdsEntry[0].WdsKey.KeyLen == 16)
		{
			for (j = 1; j < MAX_WDS_ENTRY; j++)
			{
				NdisMoveMemory(&pAd->WdsTab.WdsEntry[j].WdsKey, &pAd->WdsTab.WdsEntry[0].WdsKey, sizeof(CIPHER_KEY));								
			}
		}
	
	}

	/* The parameters can provide different key information for each WDS-Link */
	/* no matter WEP or WPA */
	if (!bUsePrevFormat)
	{
		for (i = 0; i < MAX_WDS_ENTRY; i++)
		{
			AP_WDS_KeyNameMakeUp(tok_str, sizeof(tok_str), i);
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
			wdev = &pWdsEntry->wdev;
			/* WdsXKey (X=0~MAX_WDS_ENTRY-1) */
			if (RTMPGetKeyParameter(tok_str, tmpbuf, 128, buffer, FALSE))
			{			
				if (wdev->WepStatus == Ndis802_11WEPEnabled)
				{
					/* Ascii type */
					if (strlen(tmpbuf) == 5 || strlen(tmpbuf) == 13)
					{		
						KeyLen = strlen(tmpbuf);
						pWdsEntry->WdsKey.KeyLen = KeyLen;
						NdisMoveMemory(pWdsEntry->WdsKey.Key, tmpbuf, KeyLen);
						if (KeyLen == 5)
							CipherAlg = CIPHER_WEP64;
						else
							CipherAlg = CIPHER_WEP128;	

						pWdsEntry->WdsKey.CipherAlg = CipherAlg;
						DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d Key=%s ,type=Ascii, CipherAlg(%s)\n",
									i, tmpbuf, (CipherAlg == CIPHER_WEP64 ? "wep64" : "wep128")));
					}
					/* Hex type */
					else if (strlen(tmpbuf) == 10 || strlen(tmpbuf) == 26)
					{		
						KeyLen = strlen(tmpbuf);
						pWdsEntry->WdsKey.KeyLen = KeyLen / 2;
						AtoH(tmpbuf, pWdsEntry->WdsKey.Key, KeyLen / 2);						
						if (KeyLen == 10)
							CipherAlg = CIPHER_WEP64;
						else
							CipherAlg = CIPHER_WEP128;	

						pWdsEntry->WdsKey.CipherAlg = CipherAlg;
						DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d Key=%s ,type=Hex, CipherAlg(%s)\n",
									i, tmpbuf, (CipherAlg == CIPHER_WEP64 ? "wep64" : "wep128")));
					}
					/* Invalid type */
					else
					{
						wdev->WepStatus = Ndis802_11EncryptionDisabled;
						NdisZeroMemory(&pWdsEntry->WdsKey, sizeof(CIPHER_KEY));
						DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d has invalid key for WEP, reset encryption to OPEN\n", i));
					}
				}
				else if ((wdev->WepStatus == Ndis802_11TKIPEnable)
					|| (wdev->WepStatus == Ndis802_11AESEnable))					
				{
					if ((strlen(tmpbuf) >= 8) && (strlen(tmpbuf) <= 64))
					{
						RT_CfgSetWPAPSKKey(pAd, tmpbuf, strlen(tmpbuf), (PUCHAR) RALINK_PASSPHRASE, sizeof(RALINK_PASSPHRASE), keyMaterial);
						if (wdev->WepStatus == Ndis802_11AESEnable)
							pWdsEntry->WdsKey.CipherAlg = CIPHER_AES;
						else
							pWdsEntry->WdsKey.CipherAlg = CIPHER_TKIP;
						
						NdisMoveMemory(&pWdsEntry->WdsKey.Key, keyMaterial, 16);
						pWdsEntry->WdsKey.KeyLen = 16;
						NdisMoveMemory(&pWdsEntry->WdsKey.RxMic, keyMaterial+16, 8);
						NdisMoveMemory(&pWdsEntry->WdsKey.TxMic, keyMaterial+16, 8);
						DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d Key=%s, CipherAlg(%s)\n", i, tmpbuf, (CipherAlg == CIPHER_AES ? "AES" : "TKIP")));
					}
					else
					{
						DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d has invalid key for WPA, reset encryption to OPEN\n", i));
						wdev->WepStatus = Ndis802_11EncryptionDisabled;
						NdisZeroMemory(&pWdsEntry->WdsKey, sizeof(CIPHER_KEY));
					}

				}
				else
				{									
					wdev->WepStatus = Ndis802_11EncryptionDisabled;
					NdisZeroMemory(&pWdsEntry->WdsKey, sizeof(CIPHER_KEY));
				}								
			}
		}
	}

	/* WdsDefaultKeyID */
	if(RTMPGetKeyParameter("WdsDefaultKeyID", tmpbuf, 10, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++)
		{
			pWdsEntry = &pAd->WdsTab.WdsEntry[i];
			wdev = &pWdsEntry->wdev;

			KeyIdx = (UCHAR) simple_strtol(macptr, 0, 10);
			if((KeyIdx >= 1 ) && (KeyIdx <= 4))
				pWdsEntry->KeyIdx = (UCHAR) (KeyIdx - 1);
			else
				pWdsEntry->KeyIdx = 0;

			if ((wdev->WepStatus == Ndis802_11TKIPEnable)
				|| (wdev->WepStatus == Ndis802_11AESEnable))
				pWdsEntry->KeyIdx = 0;	

			DBGPRINT(RT_DEBUG_TRACE, ("IF/wds%d - WdsDefaultKeyID(0~3)=%d\n",
						i, pWdsEntry->KeyIdx));
		}				
	}

	/* WdsTxMode */
	if (RTMPGetKeyParameter("WdsTxMode", tmpbuf, 25, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++)
		{
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;

			wdev->DesiredTransmitSetting.field.FixedTxMode = 
										RT_CfgSetFixedTxPhyMode(macptr);
			DBGPRINT(RT_DEBUG_TRACE, ("I/F(wds%d) Tx Mode = %d\n", i,
											wdev->DesiredTransmitSetting.field.FixedTxMode));					
		}	
	}

	/* WdsTxMcs */
	if (RTMPGetKeyParameter("WdsTxMcs", tmpbuf, 50, buffer, TRUE))
	{
		for (i = 0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++)
		{
			wdev = &pAd->WdsTab.WdsEntry[i].wdev;

			wdev->DesiredTransmitSetting.field.MCS = 
					RT_CfgSetTxMCSProc(macptr, &wdev->bAutoTxRateSwitch);

			if (wdev->DesiredTransmitSetting.field.MCS == MCS_AUTO)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("I/F(wds%d) Tx MCS = AUTO\n", i));
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("I/F(wds%d) Tx MCS = %d\n", i, 
									wdev->DesiredTransmitSetting.field.MCS));
			}
		}	
	}
	
	/*WdsEnable */
	if(RTMPGetKeyParameter("WdsEnable", tmpbuf, 10, buffer, TRUE))
	{						
		RT_802_11_WDS_ENTRY *pWdsEntry;
		switch(simple_strtol(tmpbuf, 0, 10))
		{
			case WDS_BRIDGE_MODE: /* Bridge mode, DisAllow association(stop Beacon generation and Probe Req. */
				pAd->WdsTab.Mode = WDS_BRIDGE_MODE;
				break;
			case WDS_RESTRICT_MODE:	
			case WDS_REPEATER_MODE: /* Repeater mode */
				pAd->WdsTab.Mode = WDS_REPEATER_MODE;
				break;
			case WDS_LAZY_MODE: /* Lazy mode, Auto learn wds entry by same SSID, channel, security policy */
				for(i = 0; i < MAX_WDS_ENTRY; i++)
				{
					pWdsEntry = &pAd->WdsTab.WdsEntry[i];
					if (pWdsEntry->Valid)
						WdsEntryDel(pAd, pWdsEntry->PeerWdsAddr);
				
					/* When Lazy mode is enabled, the all wds-link shall share the same encryption type and key material */
					if (i > 0)
					{
						pWdsEntry->wdev.WepStatus = pAd->WdsTab.WdsEntry[0].wdev.WepStatus;
						pWdsEntry->KeyIdx = pAd->WdsTab.WdsEntry[0].KeyIdx;
						NdisMoveMemory(&pWdsEntry->WdsKey, &pAd->WdsTab.WdsEntry[0].WdsKey, sizeof(CIPHER_KEY));
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

		DBGPRINT(RT_DEBUG_TRACE, ("WDS-Enable mode=%d\n", pAd->WdsTab.Mode));

	}
	
#ifdef WDS_VLAN_SUPPORT
	/* WdsVlan */
	if (RTMPGetKeyParameter("WDS_VLANID", tmpbuf, MAX_PARAM_BUFFER_SIZE, buffer, TRUE))
	{	
		for (i=0, macptr = rstrtok(tmpbuf,";"); (macptr && i < MAX_WDS_ENTRY); macptr = rstrtok(NULL,";"), i++) 
		{
            pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID = simple_strtol(macptr, 0, 10);
            pAd->WdsTab.WdsEntry[i].wdev.VLAN_Priority = 0;
		
	        DBGPRINT(RT_DEBUG_TRACE, ("If/wds%d - WdsVlanId=%d\n", i, pAd->WdsTab.WdsEntry[i].wdev.VLAN_VID));
		}
	}
#endif /* WDS_VLAN_SUPPORT */
}


VOID WdsPrepareWepKeyFromMainBss(RTMP_ADAPTER *pAd)
{
	INT	i;
	
	/* Prepare WEP key for each wds-link if necessary */
	for (i = 0; i < MAX_WDS_ENTRY; i++)
	{	
		/* For WDS backward compatible, refer to the WEP key of Main BSS in WEP mode */
		if (pAd->WdsTab.WdsEntry[i].wdev.WepStatus == Ndis802_11WEPEnabled &&
			pAd->WdsTab.WdsEntry[i].WdsKey.KeyLen == 0)
		{
			UCHAR	main_bss_keyid = pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.DefaultKeyId;
		
			if (pAd->ApCfg.MBSSID[MAIN_MBSSID].wdev.WepStatus == Ndis802_11WEPEnabled && 
				(pAd->SharedKey[MAIN_MBSSID][main_bss_keyid].KeyLen == 5 ||
				 pAd->SharedKey[MAIN_MBSSID][main_bss_keyid].KeyLen == 13))	
			{
				DBGPRINT(RT_DEBUG_TRACE, ("Duplicate IF/WDS%d wep key from main_bssid \n", (UCHAR)i));
				pAd->WdsTab.WdsEntry[i].KeyIdx = main_bss_keyid;
				NdisMoveMemory(&pAd->WdsTab.WdsEntry[i].WdsKey, &pAd->SharedKey[MAIN_MBSSID][main_bss_keyid], sizeof(CIPHER_KEY));
			}
			else
			{
				DBGPRINT(RT_DEBUG_TRACE, ("No available wep key for IF/WDS%d, reset its encryption as OPEN \n", (UCHAR)i));
				pAd->WdsTab.WdsEntry[i].wdev.WepStatus = Ndis802_11EncryptionDisabled;
				NdisZeroMemory(&pAd->WdsTab.WdsEntry[i].WdsKey, sizeof(CIPHER_KEY));
			}
		}
	}

}


VOID WDS_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	INT index;
	PNET_DEV pWdsNetDev;
	
	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_wds_init != FALSE)
		return;

	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
		RT_802_11_WDS_ENTRY *wds_entry;
		struct wifi_dev *wdev;
		char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		dev_name = get_dev_name_prefix(pAd, INT_WDS);
		pWdsNetDev = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_WDS, index, sizeof(struct mt_dev_priv), dev_name);
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */

		wds_entry = &pAd->WdsTab.WdsEntry[index];
		wdev = &wds_entry->wdev;
		
		if (pWdsNetDev == NULL)
		{
			/* allocation fail, exit */
			DBGPRINT(RT_DEBUG_ERROR, ("Allocate network device fail (WDS)...\n"));
			break;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("The new WDS interface MAC = %02X:%02X:%02X:%02X:%02X:%02X\n", 
				PRINT_MAC(pAd->MacTab.Content[wds_entry->MacTabMatchWCID].Addr)));

		NdisZeroMemory(&wds_entry->WdsCounter, sizeof(WDS_COUNTER));

		wdev->wdev_type = WDEV_TYPE_WDS;
		wdev->func_dev = wds_entry;
		wdev->func_idx = index;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = pWdsNetDev;
		wdev->tx_pkt_allowed = ApWdsAllowToSendPacket;
		// TODO: shiang-usw, modify this to WDSSendPacket
		wdev->tx_pkt_handle = APSendPacket;
		wdev->wdev_hard_tx = APHardTransmit;
		wdev->rx_pkt_allowed = ap_rx_pkt_allow;
		wdev->rx_pkt_foward = wds_rx_foward_handle;

		wdev->PhyMode = 0xff;
		wdev->allow_data_tx = TRUE;	// let tx_pkt_allowed() to check it!
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;
		NdisMoveMemory(&wdev->if_addr[0], &pNetDevOps->devAddr[0], MAC_ADDR_LEN);

		RTMP_OS_NETDEV_SET_PRIV(pWdsNetDev, pAd);
		RTMP_OS_NETDEV_SET_WDEV(pWdsNetDev, wdev);

		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(pWdsNetDev)));
			RtmpOSNetDevFree(pWdsNetDev);
			break;
		}
		
		pNetDevOps->priv_flags = INT_WDS;
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		/* Register this device */
		RtmpOSNetDevAttach(pAd->OpMode, pWdsNetDev, pNetDevOps);
	}

	if (index > 0)
		pAd->flg_wds_init = TRUE;


	NdisAllocateSpinLock(pAd, &pAd->WdsTabLock);

	DBGPRINT(RT_DEBUG_TRACE, ("Total allocated %d WDS interfaces!\n", index));
	
}


VOID WDS_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		wdev = &pAd->WdsTab.WdsEntry[index].wdev;
		if (wdev->if_dev)
		{
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);

			rtmp_wdev_idx_unreg(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);

			/* Clear it as NULL to prevent latter access error. */
			wdev->if_dev = NULL;
		}
	}
}


BOOLEAN WDS_StatsGet(RTMP_ADAPTER *pAd, RT_CMD_STATS *pStats)
{
	INT WDS_apidx = 0,index;
	RT_802_11_WDS_ENTRY *wds_entry;


	for(index = 0; index < MAX_WDS_ENTRY; index++)
	{
		if (pAd->WdsTab.WdsEntry[index].wdev.if_dev == pStats->pNetDev)
		{
			WDS_apidx = index;
			break;
		}
	}
		
	if(index >= MAX_WDS_ENTRY)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s(): can not find wds I/F\n", __FUNCTION__));
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
	pStats->rx_crc_errors = 0;/*pAd->WlanCounters.FCSErrorCount;     // recved pkt with crc error */
	pStats->rx_frame_errors = 0; /* recv'd frame alignment error */
	pStats->rx_fifo_errors = wds_entry->WdsCounter.RxNoBuffer;                   /* recv'r fifo overrun */

	return TRUE;
}

#endif /* WDS_SUPPORT */

