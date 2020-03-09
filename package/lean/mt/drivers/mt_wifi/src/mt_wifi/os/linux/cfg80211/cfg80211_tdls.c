/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
 *
 *	Abstract:
 *
 *	All related CFG80211 TDLS function body.
 *
 *	History:
 *
 ***************************************************************************/

#define RTMP_MODULE_OS

#ifdef RT_CFG80211_SUPPORT

#include "rt_config.h"

#ifdef CFG_TDLS_SUPPORT
#define	TDLS_AKM_SUITE_PSK			7
UCHAR	CipherSuiteTDLSWpa2PskAes[] = {
	0x30,					/* RSN IE */
	0x14,					/* Length */
	0x01, 0x00,				/* Version */
	0x00, 0x0F, 0xAC, 0x07,	/* no group cipher */
	0x01, 0x00,				/* number of pairwise */
	0x00, 0x0f, 0xAC, 0x04,	/* unicast, AES */
	0x01, 0x00,				/* number of authentication method */
	0x00, 0x0f, 0xAC, TDLS_AKM_SUITE_PSK,	/* TDLS authentication */
	0x00, 0x02,				/* RSN capability, peer key enabled */
};
UCHAR	CipherSuiteTDLSLen = sizeof(CipherSuiteTDLSWpa2PskAes) / sizeof(UCHAR);

typedef struct {
	UCHAR	regclass;		/* regulatory class */
	UCHAR	spacing;		/* 0: 20Mhz, 1: 40Mhz */
	UCHAR	channelset[16];	/* max 15 channels, use 0 as terminator */
} CFG_REG_CLASS;

CFG_REG_CLASS cfg_reg_class[] = {
	{  1,  0, {36, 40, 44, 48, 0} },
	{  2,  0, {52, 56, 60, 64, 0} },
	{  3,  0, {149, 153, 157, 161, 0} },
	{  4,  0, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 0} },
	{  5,  0, {165, 0} },
	{ 12, 0, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0} },
	{ 22, 1, {36, 44, 0} },
	{ 23, 1, {52, 60, 0} },
	{ 24, 1, {100, 108, 116, 124, 132, 0} },
	{ 25, 1, {149, 157, 0} },
	{ 26, 1, {149, 157, 0} },
	{ 27, 1, {40, 48, 0} },
	{ 28, 1, {56, 64, 0} },
	{ 29, 1, {104, 112, 120, 128, 136, 0} },
	{ 30, 1, {153, 161, 0} },
	{ 31, 1, {153, 161, 0} },
	{ 32, 1, {1, 2, 3, 4, 5, 6, 7, 0} },
	{ 33, 1, {5, 6, 7, 8, 9, 10, 11, 0} },
	{ 0,   0, {0} }			/* end */
};

UCHAR cfg_tdls_GetRegulatoryClass(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	ChannelWidth,
	IN UCHAR	TargetChannel)
{
	int i = 0;
	UCHAR regclass = 0;

	do {
		if (cfg_reg_class[i].spacing == ChannelWidth) {
			int j = 0;

			do {
				if (cfg_reg_class[i].channelset[j] == TargetChannel) {
					regclass = cfg_reg_class[i].regclass;
					break;
				}

				j++;
			} while (cfg_reg_class[i].channelset[j] != 0);
		}

		i++;
	} while (cfg_reg_class[i].regclass != 0);

	/* ASSERT(regclass); */
	return regclass;
}


VOID InitPeerEntryRateCapability(
	IN	PRTMP_ADAPTER pAd,
	IN	MAC_TABLE_ENTRY * pEntry,
	IN USHORT * pCapabilityInfo,
	IN UCHAR SupportRateLens,
	IN UCHAR *pSupportRates,
	IN UCHAR HtCapabilityLen,
	IN HT_CAPABILITY_IE * pHtCapability)
{
	UCHAR MaxSupportedRate = RATE_11;
	UCHAR MaxSupportedRateIn500Kbps = 0;
	UCHAR idx;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
	UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);

	for (idx = 0; idx < SupportRateLens; idx++) {
		if (MaxSupportedRateIn500Kbps < (pSupportRates[idx] & 0x7f))
			MaxSupportedRateIn500Kbps = pSupportRates[idx] & 0x7f;
	}

	switch (MaxSupportedRateIn500Kbps) {
	case 108:
		MaxSupportedRate = RATE_54;
		break;

	case 96:
		MaxSupportedRate = RATE_48;
		break;

	case 72:
		MaxSupportedRate = RATE_36;
		break;

	case 48:
		MaxSupportedRate = RATE_24;
		break;

	case 36:
		MaxSupportedRate = RATE_18;
		break;

	case 24:
		MaxSupportedRate = RATE_12;
		break;

	case 18:
		MaxSupportedRate = RATE_9;
		break;

	case 12:
		MaxSupportedRate = RATE_6;
		break;

	case 22:
		MaxSupportedRate = RATE_11;
		break;

	case 11:
		MaxSupportedRate = RATE_5_5;
		break;

	case 4:
		MaxSupportedRate = RATE_2;
		break;

	case 2:
		MaxSupportedRate = RATE_1;
		break;

	default:
		MaxSupportedRate = RATE_11;
		break;
	}

	pEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxDesiredRate, MaxSupportedRate);

	if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE) {
		pEntry->MaxHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MaxHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->MinHTPhyMode.field.MODE = MODE_CCK;
		pEntry->MinHTPhyMode.field.MCS = pEntry->MaxSupportedRate;
		pEntry->HTPhyMode.field.MODE = MODE_CCK;
		pEntry->HTPhyMode.field.MCS = pEntry->MaxSupportedRate;
	} else {
		pEntry->MaxHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MaxHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->MinHTPhyMode.field.MODE = MODE_OFDM;
		pEntry->MinHTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
		pEntry->HTPhyMode.field.MODE = MODE_OFDM;
		pEntry->HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pEntry->MaxSupportedRate];
	}

	pEntry->MaxHTPhyMode.field.BW = BW_20;
	pEntry->MinHTPhyMode.field.BW = BW_20;
#ifdef DOT11_N_SUPPORT
	pEntry->HTCapability.MCSSet[0] = 0;
	pEntry->HTCapability.MCSSet[1] = 0;
	pEntry->HTCapability.MCSSet[2] = 0;
	pEntry->HTCapability.MCSSet[3] = 0;

	/* If this Entry supports 802.11n, upgrade to HT rate. */
	if ((HtCapabilityLen != 0) && (wdev->PhyMode >= PHY_11ABGN_MIXED)) {
		UCHAR	j, bitmask; /*k,bitmask; */
		CHAR    ii;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE,
				 ("TDLS - Receive Peer HT Capable STA from %02x:%02x:%02x:%02x:%02x:%02x\n",
				  PRINT_MAC(pEntry->Addr)));

		if ((pHtCapability->HtCapInfo.GF) &&
			wlan_config_get_greenfield(wdev) &&
			(pAd->StaActive.SupportedHtPhy.GF))
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
		else {
			pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
			pAd->MacTab.fAnyStationNonGF = TRUE;
			addht->AddHtInfo2.NonGfPresent = 1;
		}

		if ((pHtCapability->HtCapInfo.ChannelWidth) && cfg_ht_bw) {
			pEntry->MaxHTPhyMode.field.BW = BW_40;
			pEntry->MaxHTPhyMode.field.ShortGI = (wlan_config_get_ht_gi(wdev) & (pHtCapability->HtCapInfo.ShortGIfor40));
		} else {
			pEntry->MaxHTPhyMode.field.BW = BW_20;
			pEntry->MaxHTPhyMode.field.ShortGI = (wlan_config_get_ht_gi(wdev) & (pHtCapability->HtCapInfo.ShortGIfor20));
			pAd->MacTab.fAnyStation20Only = TRUE;
		}

		/* find max fixed rate */
		for (ii = 31; ii >= 0; ii--) { /* 3*3 */
			j = ii / 8;
			bitmask = (1 << (ii - (j * 8)));

			if ((pAd->StaCfg[0].wdev.DesiredHtPhyInfo.MCSSet[j]&bitmask) && (pHtCapability->MCSSet[j]&bitmask)) {
				pEntry->MaxHTPhyMode.field.MCS = ii;
				break;
			}

			if (ii == 0)
				break;
		}

		if (pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.MCS != MCS_AUTO) {
			if (pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.MCS == 32) {
				/* Fix MCS as HT Duplicated Mode */
				pEntry->MaxHTPhyMode.field.BW = 1;
				pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
				pEntry->MaxHTPhyMode.field.STBC = 0;
				pEntry->MaxHTPhyMode.field.ShortGI = 0;
				pEntry->MaxHTPhyMode.field.MCS = 32;
			} else if (pEntry->MaxHTPhyMode.field.MCS > pAd->StaCfg[0].wdev.HTPhyMode.field.MCS) {
				/* STA supports fixed MCS */
				pEntry->MaxHTPhyMode.field.MCS = pAd->StaCfg[0].wdev.HTPhyMode.field.MCS;
			}
		}

		pEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & wlan_config_get_ht_stbc(wdev));
		pEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
		pEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
		pEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
		pEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;
		pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

		if (wlan_config_get_ht_gi(wdev) == GI_400) {
			if (pHtCapability->HtCapInfo.ShortGIfor20)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI20_CAPABLE);

			if (pHtCapability->HtCapInfo.ShortGIfor40)
				CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI40_CAPABLE);
		}

		if (pHtCapability->HtCapInfo.TxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);

		if (pHtCapability->HtCapInfo.RxSTBC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);

		if (pHtCapability->ExtHtCapInfo.PlusHTC)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_HTC_CAPABLE);

		if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_RDG_CAPABLE);

		if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);

		os_move_mem(&pEntry->HTCapability, pHtCapability, sizeof(HT_CAPABILITY_IE));
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
	} else
#endif /* DOT11_N_SUPPORT */
	{
		os_zero_mem(&pEntry->HTCapability, sizeof(HT_CAPABILITY_IE));
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("TDLS - Receive Peer Legacy STA\n"));
	}

	pEntry->HTPhyMode.word = pEntry->MaxHTPhyMode.word;

	if ((pHtCapability->HtCapInfo.ChannelWidth) &&
		cfg_ht_bw)
		pEntry->HTPhyMode.field.BW = BW_40;
	else
		pEntry->HTPhyMode.field.BW = BW_20;

	pEntry->CurrTxRate = pEntry->MaxSupportedRate;

	if (pAd->StaCfg[0].wdev.bAutoTxRateSwitch == TRUE) {
		PUCHAR pTable;
		UCHAR TableSize = 0;

		pEntry->bAutoTxRateSwitch = TRUE;
		/* TODO: MlmeSelectTxRateTable() is not supported on MT_MAC, please refer to TDLS_InitPeerEntryRateCapability() */
		MlmeSelectTxRateTable(pAd, pEntry, &pTable, &TableSize, &pEntry->CurrTxRateIndex);
		pEntry->pTable = pTable;
	} else {
		pEntry->HTPhyMode.field.MODE	= pAd->StaCfg[0].wdev.HTPhyMode.field.MODE;
		pEntry->HTPhyMode.field.MCS	= pAd->StaCfg[0].wdev.HTPhyMode.field.MCS;
		pEntry->bAutoTxRateSwitch = FALSE;
		RTMPUpdateLegacyTxSetting((UCHAR)pAd->StaCfg[0].wdev.DesiredTransmitSetting.field.FixedTxMode, pEntry);
	}

	pEntry->RateLen = SupportRateLens;
}

BOOLEAN CFG80211DRV_StaTdlsSetKeyCopyFlag(
	VOID						*pAdOrg)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));
	pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IneedKey = 1;
	return TRUE;
}

BOOLEAN CFG80211DRV_StaTdlsInsertDeletepEntry(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT						Data)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdOrg;
	UCHAR *peer;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	UCHAR HtLen;
	HT_CAPABILITY_IE HtCapabilityTmp;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);

	peer = (UCHAR *)pData;
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("=====>  %s() peer: %02x:%02x:%02x:%02x:%02x:%02x ,op: %d\n", __func__, PRINT_MAC(peer), Data));
	pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IneedKey = 0;

	if (Data == tdls_insert_entry) {
		int i = 0, tdls_entry_index = 0;

		for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
			if (MAC_ADDR_EQUAL(peer, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr)) {
				tdls_entry_index = i;  /* found prebuild entry */
				break;
			}
		}

		if (i == MAX_NUM_OF_CFG_TDLS_ENTRY) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - EXCEED MAX_NUM_OF_CFG_TDLS_ENTRY %d. Abort this setup\n", __func__, MAX_NUM_OF_CFG_TDLS_ENTRY));
			return FALSE;
		}

		/* allocate one MAC entry */
		pMacEntry = MacTableLookup(pAd, peer);

		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - MacTable Entry exist !!!\n", __func__));
		else
			pMacEntry = MacTableInsertEntry(pAd,
											peer,
											wdev,
											ENTRY_TDLS,
											OPMODE_STA,
											TRUE);

		if (pMacEntry) {
			/* pTDLS->MacTabMatchWCID = pMacEntry->Aid; */
			STA_TR_ENTRY *tr_entry;

			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->tr_tb_idx];
			/* update tdls link info */
			pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].EntryValid = TRUE;
			pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].MacTabMatchWCID = pMacEntry->wcid;
			pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsLinkCount++;
			pMacEntry->AuthMode = pAd->StaCfg[0].wdev.AuthMode;
			pMacEntry->WepStatus = pAd->StaCfg[0].wdev.WepStatus;
			tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
			pMacEntry->Sst = SST_ASSOC;
#ifdef UAPSD_SUPPORT
			/* update UAPSD */
			UAPSD_AssocParse(pAd, pMacEntry, &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].QosCapability,
							 wdev->UapsdInfo.bAPSDCapable);
#endif /* UAPSD_SUPPORT */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE,
					 ("MacTableInsertTDlsEntry - allocate entry #%d, Total= %d\n",
					  pMacEntry->Aid, pAd->MacTab.Size));

			/* Set WMM capability */
			if ((wdev->PhyMode >= PHY_11ABGN_MIXED) || (pAd->CommonCfg.bWmmCapable)) {
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("TDLS -  WMM Capable\n"));
			} else
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);

			/* build HT_CAPABILITY_IE */
			if (WMODE_CAP_N(wdev->PhyMode)) {
				HtLen = sizeof(HT_CAPABILITY_IE);
#ifndef RT_BIG_ENDIAN
				os_zero_mem(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
				os_move_mem(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
				HtCapabilityTmp.HtCapInfo.ChannelWidth = op_ht_bw;
#else
				os_zero_mem(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
				os_move_mem(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
				HtCapabilityTmp.HtCapInfo.ChannelWidth = op_ht_bw;
				*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
				*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif
			}

			InitPeerEntryRateCapability(pAd,
										pMacEntry,
										&pAd->StaActive.CapabilityInfo,
										pAd->StaActive.SupRateLen,
										pAd->StaActive.SupRate,
										pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].HtCapabilityLen,
										&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].HtCapability);
			RTMPSetSupportMCS(pAd,
							  OPMODE_STA,
							  pMacEntry,
							  pAd->StaActive.SupRate,
							  pAd->StaActive.SupRateLen,
							  pAd->StaActive.ExtRate,
							  pAd->StaActive.ExtRateLen,
#ifdef DOT11_VHT_AC
							  0,
							  NULL,
#endif /* DOT11_VHT_AC */
							  &HtCapabilityTmp,
							  HtLen);

			/*  */
			/* Install Peer Key if RSNA Enabled */
			/*  */
			if (IS_SECURITY(&wdev->SecConfig)) {
				/* Write to ASIC on-chip table. */
				if (pMacEntry->Aid > 1) {
					ASIC_SEC_INFO Info = {0};

					CLEAR_SEC_AKM(pMacEntry->SecConfig.AKMMap);
					CLEAR_PAIRWISE_CIPHER(&pMacEntry->SecConfig);
					CLEAR_GROUP_CIPHER(&pMacEntry->SecConfig);
					SET_AKM_WPA2PSK(pMacEntry->SecConfig.AKMMap);
					SET_CIPHER_CCMP128(pMacEntry->SecConfig.PairwiseCipher);
					os_move_mem(pMacEntry->SecConfig.PTK, &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TPK[LEN_TK], LEN_TK);
					/* Set key material to Asic */
					os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
					Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
					Info.Direction = SEC_ASIC_KEY_BOTH;
					Info.Wcid = pMacEntry->Aid;
					Info.BssIndex = BSS0;
					Info.Cipher = pMacEntry->SecConfig.PairwiseCipher;
					Info.KeyIdx = 0;
					os_move_mem(&Info.PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(&Info.Key, &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TPK[LEN_TK], LEN_TK);
					WPAInstallKey(pAd, &Info, FALSE, TRUE);
					pMacEntry->wdev->PortSecured = WPA_802_1X_PORT_SECURED;
					pMacEntry->PrivacyFilter = Ndis802_11PrivFilterAcceptAll;
				}
			}

			if (wdev_do_disconn_act(wdev, pMacEntry) != TRUE)
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() dis connect fail!\n", __func__));

			cfg_tdls_prepare_null_frame(pAd, 0, 0, peer);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - MacTableInsertEntry failed\n", __func__));
			return FALSE;
		}
	} else if (Data == tdls_delete_entry) {
		int i = 0;

		pMacEntry = MacTableLookup(pAd, peer);

		if (pMacEntry && IS_ENTRY_TDLS(pMacEntry))
			MacTableDeleteEntry(pAd, pMacEntry->wcid, peer);

		/* clear tdls link info */
		for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
			if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].EntryValid == TRUE &&
				MAC_ADDR_EQUAL(peer, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() clear TDLS entry %d\n", __func__, i));
				os_zero_mem(&(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i]), sizeof(CFG_TDLS_ENTRY));
				pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsLinkCount--;
#ifdef UAPSD_SUPPORT
				/* re-init timer for this TDLS entry */
				RTMPInitTimer(pAd, &(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].Timer)
							  , GET_TIMER_FUNCTION(cfg_tdls_PTITimeoutAction)
							  , &(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i]), FALSE);
#endif /*UAPSD_SUPPORT*/
				break;
			}
		}
	} else
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() op: %d  DO NOTHING\n", __func__, Data));

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("<======  %s() out\n", __func__));
	return TRUE;
}
#ifdef UAPSD_SUPPORT
BOOLEAN cfg_tdls_UAPSDP_AsicCanSleep(PRTMP_ADAPTER pAd)
{
	UINT32 i;
	BOOLEAN FlgAllSpClosed = TRUE;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));

	for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
		PCFG_TDLS_ENTRY pTDLS = &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i];

		if (pTDLS->EntryValid == TRUE) {
			UINT32 Wcid = pTDLS->MacTabMatchWCID;
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[Wcid];

			if (!IS_ENTRY_TDLS(pEntry))
				continue;

			/*
			 *	Two cases we can not sleep:
			 *	1. One of any SP is not ended.
			 *	2. A traffic indication is sent and no response is received.
			 */
			if ((pEntry->bAPSDFlagSPStart != 0) ||
				(pTDLS->FlgIsWaitingUapsdTraRsp == TRUE)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE,
						 ("tdls uapsd> SP not close or Ind sent (%d %d)!\n",
						  pEntry->bAPSDFlagSPStart,
						  pTDLS->FlgIsWaitingUapsdTraRsp));
				hex_dump("pEntry=", pEntry->Addr, 6);
				FlgAllSpClosed = FALSE;
				break;
			}
		}
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
	return FlgAllSpClosed;
}

VOID cfg_tdls_UAPSDP_PsmModeChange(
	IN	PRTMP_ADAPTER				pAd,
	IN	USHORT						PsmOld,
	IN	USHORT						PsmNew)
{
	MAC_TABLE_ENTRY	*pMacEntry;
	UINT32 IdTdls;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));

	if (PsmOld == PsmNew)
		return; /* no inform needs */

	/* sanity check */
	/* WPA 802.1x secured port control */
	if (((wdev->AuthMode == Ndis802_11AuthModeWPA) ||
		 (wdev->AuthMode == Ndis802_11AuthModeWPAPSK) ||
		 (wdev->AuthMode == Ndis802_11AuthModeWPA2) ||
		 (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
		) &&
		(wdev->PortSecured == WPA_802_1X_PORT_NOT_SECURED)) {
		return; /* port not yet secure */
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("tdls uapsd> our PSM mode change!\n"));

	/* indicate the peer */

	for (IdTdls = 0; IdTdls < MAX_NUM_OF_CFG_TDLS_ENTRY; IdTdls++) {
		if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[IdTdls].EntryValid == TRUE) {
			/* get MAC Entry */
			pMacEntry = &pAd->MacTab.Content[pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[IdTdls].MacTabMatchWCID];

			if (pMacEntry == NULL)
				continue; /* check next one */

			/* check if the peer is in ACTIVE mode */
			if (pMacEntry->PsMode != PWR_SAVE)
			{
				/*
				 *	pMacEntry->RssiSample.LastRssi0 is used to check if
				 *	we have ever received any packet from the peer.
				 */
				/* send a null frame to the peer directly */
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("tdls uapsd> send a NULL frame!\n"));
				RtmpEnqueueNullFrame(pAd, pMacEntry->Addr,
									 pAd->CommonCfg.TxRate, pMacEntry->Aid,
									 pMacEntry->apidx, TRUE, FALSE, 0);
				continue;
			}

			/*
			 *	Send traffic indication frame to the peer when the peer
			 *	is in power-save mode.
			 */
			cfg_tdls_send_PeerTrafficIndication(pAd, pMacEntry->Addr);
		}
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
}

VOID cfg_tdls_rcv_PeerTrafficIndication(PRTMP_ADAPTER pAd, UINT8 dialog_token, UINT8 *peer)
{
	int tdls_entry_wcid = 0;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	/* Not BSS mode, ignore it */
	if (!INFRA_ON(pAd))
		return;

	/* sanity check */
	if (pAd->StaCfg[0].PwrMgmt.Psm != PWR_SAVE)
		return; /* we are not in power-save mode */

	tdls_entry_wcid = cfg_tdls_search_wcid(pAd, peer);

	if (tdls_entry_wcid == -1) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s(): can't find valid link ID, tdls link to (%02X:%02X:%02X:%02X:%02X:%02X) doesn't exist.\n", __func__, PRINT_MAC(peer)));
		return;
	}

	cfg_tdls_build_frame(pAd, peer, dialog_token, TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE, 0, NULL, 0, FALSE, tdls_entry_wcid, 0);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
}

VOID cfg_tdls_rcv_PeerTrafficResponse(PRTMP_ADAPTER pAd, UINT8 *peer)
{
	int i = 0;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("======>  %s()\n", __func__));

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	/* Not BSS mode, ignore it */
	if (!INFRA_ON(pAd))
		return;

	i = cfg_tdls_search_ValidLinkIndex(pAd, peer);

	if (i == -1) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s(): can't find valid link ID, tdls link to (%02X:%02X:%02X:%02X:%02X:%02X) doesn't exist.\n", __func__, PRINT_MAC(peer)));
		return;
	}

	pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].FlgIsWaitingUapsdTraRsp = FALSE;
	/* check if we can sleep if we are sleep mode */
	RtmpAsicSleepHandle(pAd);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("<======  %s() out\n", __func__));
}

VOID cfg_tdls_send_PeerTrafficIndication(PRTMP_ADAPTER pAd, UINT8 *peer)
{
	int tdls_entry_link_index = 0;
	UINT8 dialog_token = 0;
	BOOLEAN TimerCancelled;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("======>  %s()\n", __func__));

	if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsDialogToken == 0)
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsDialogToken = 1;

	dialog_token = pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsDialogToken++;

	/* Not TDLS Capable, ignore it */
	if (!IS_TDLS_SUPPORT(pAd))
		return;

	/* Not BSS mode, ignore it */
	if (!INFRA_ON(pAd))
		return;

	tdls_entry_link_index = cfg_tdls_search_ValidLinkIndex(pAd, peer);

	if (tdls_entry_link_index == -1) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s(): can't find valid link ID, tdls link to (%02X:%02X:%02X:%02X:%02X:%02X) doesn't exist.\n", __func__, PRINT_MAC(peer)));
		return;
	}

	if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].FlgIsWaitingUapsdTraRsp == TRUE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls uapsd> traffic ind of index %d was sent before!\n", tdls_entry_link_index));
		return; /* has sent it */
	}

	pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].FlgIsWaitingUapsdTraRsp = TRUE;
	cfg_tdls_build_frame(pAd, peer, dialog_token, TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION, 0, NULL, 0, FALSE, tdls_entry_link_index, 0);
	/*
	 *	11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA
	 *	When no corresponding TDLS Peer Traffic Response frame has been
	 *	received within dot11TDLSResponseTimeout after sending a TDLS Peer
	 *	Traffic Indication frame, the STA shall tear down the direct link.
	 *
	 *	The default value is 5 seconds.
	 */
	/* set traffic indication timer */
	RTMPCancelTimer(&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].Timer, &TimerCancelled);
	RTMPSetTimer(&pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].Timer, TDLS_TIMEOUT);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("<======  %s() out\n", __func__));
}
VOID cfg_tdls_TimerInit(PRTMP_ADAPTER pAd)
{
	int i = 0;

	for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
		PCFG_TDLS_ENTRY pCfgTdls = NULL;

		pCfgTdls = &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i];
		pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].pAd = pAd;
		RTMPInitTimer(pAd, &(pCfgTdls->Timer), GET_TIMER_FUNCTION(cfg_tdls_PTITimeoutAction), pCfgTdls, FALSE);
	}
}

VOID cfg_tdls_PTITimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PCFG_TDLS_ENTRY		pTDLS = (PCFG_TDLS_ENTRY)FunctionContext;
	PRTMP_ADAPTER			pAd;
	/*
	 *	11.2.1.14.1 Peer U-APSD Behavior at the PU buffer STA
	 *	When no corresponding TDLS Peer Traffic Response frame has been
	 *	received within dot11TDLSResponseTimeout after sending a TDLS Peer
	 *	Traffic Indication frame, the STA shall tear down the direct link.
	 */
	pAd = pTDLS->pAd;

	if (pTDLS->FlgIsWaitingUapsdTraRsp == TRUE) {
		MAC_TABLE_ENTRY	*pMacEntry = NULL;
		/* timeout for traffic response frame */
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("tdls uapsd> traffic rsp timeout!!!\npeerMAC(%02X:%02X:%02X:%02X:%02X:%02X), send link teardown\n"
				 , PRINT_MAC(pTDLS->MacAddr)));
		pTDLS->FlgIsWaitingUapsdTraRsp = FALSE;
		pMacEntry = MacTableLookup(pAd, pTDLS->MacAddr);

		if (pMacEntry == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
					 __func__));
			return;
		}

		RTEnqueueInternalCmd(pAd, CMDTHREAD_TDLS_AUTO_TEARDOWN, pMacEntry, sizeof(MAC_TABLE_ENTRY));
		return;
	}
}

#endif /* UAPSD_SUPPORT */

INT cfg_tdls_search_wcid(PRTMP_ADAPTER pAd, UINT8 *peer)
{
	int i = 0, tdls_entry_wcid;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));

	for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
		if (MAC_ADDR_EQUAL(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr, peer) &&
			pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].EntryValid == TRUE) {
			tdls_entry_wcid = pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacTabMatchWCID;
			break;
		}
	}

	if (i == MAX_NUM_OF_CFG_TDLS_ENTRY)
		tdls_entry_wcid = -1;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
	return tdls_entry_wcid;
}

INT cfg_tdls_search_ValidLinkIndex(PRTMP_ADAPTER pAd, UINT8 *peer)
{
	int i = 0, tdls_link_index;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));

	for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
		if (MAC_ADDR_EQUAL(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr, peer) &&
			pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].EntryValid == TRUE) {
			tdls_link_index = i;
			break;
		}
	}

	if (i == MAX_NUM_OF_CFG_TDLS_ENTRY)
		tdls_link_index = -1;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
	return tdls_link_index;
}

INT cfg_tdls_build_frame(PRTMP_ADAPTER pAd, u8 *peer, u8 dialog_token, u8 action_code, u16 status_code, const u8 *extra_ies, size_t extra_ies_len, BOOLEAN send_by_tdls_link, u8 tdls_entry_link_index, u8 reason_code)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	HEADER_802_11 Hdr80211;
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UINT8	category = CATEGORY_TDLS;
	UCHAR	TDLS_IE = IE_TDLS_LINK_IDENTIFIER;
	UCHAR	TDLS_IE_LEN = TDLS_ELM_LEN_LINK_IDENTIFIER;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;
	UCHAR Length = sizeof(EXT_CAP_INFO_ELEMENT);
	EXT_CAP_INFO_ELEMENT	extCapInfo;
	BSS_2040_COEXIST_IE BssCoexistence;
	struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;
	UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%s() - TDLSEntry[%d] Initiator = %d\n", __func__, tdls_entry_link_index, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].bInitiator));
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("ACT - TDLS_SetupRequestAction() allocate memory failed\n"));
		return NStatus;
	}

	if (action_code != WLAN_PUB_ACTION_TDLS_DISCOVER_RES)
		MAKE_802_3_HEADER(Header802_3, peer, pAd->CurrentAddress, TDLS_ETHERTYPE);
	else { /* Discovery response is a MGMT Action frame, others are 890d data frame */
		os_zero_mem(&Hdr80211, sizeof(HEADER_802_11));
		Hdr80211.FC.Type = FC_TYPE_MGMT;
		Hdr80211.FC.SubType = SUBTYPE_ACTION;
		Hdr80211.FC.FrDs = 0;
		Hdr80211.FC.ToDs = 0;
		COPY_MAC_ADDR(Hdr80211.Addr1, peer);
		COPY_MAC_ADDR(Hdr80211.Addr2, pAd->CurrentAddress);
		COPY_MAC_ADDR(Hdr80211.Addr3, pAd->CommonCfg.Bssid);
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  sizeof(HEADER_802_11),			&Hdr80211,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* enter ACTIVE mode */
	/* TDLS_CHANGE_TO_ACTIVE(pAd); */

	/* fill action code */
	if (action_code != WLAN_PUB_ACTION_TDLS_DISCOVER_RES) {
		u8 tmp_action_code = action_code;

		if (action_code == TDLS_ACTION_CODE_AUTO_TEARDOWN)
			tmp_action_code = 3;

		MakeOutgoingFrame(pOutBuffer,		&TempLen,
						  1,				&RemoteFrameType,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&category,
						  1,				&tmp_action_code,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} else {
		category = CATEGORY_PUBLIC;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&category,
						  1,				&action_code,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	if (action_code == WLAN_TDLS_SETUP_RESPONSE || action_code == WLAN_TDLS_SETUP_CONFIRM) {
		/* fill status code */
		status_code = cpu2le16(status_code);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("########## status code : 0x%04x\n", status_code));
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  2,				&status_code,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	if (action_code == WLAN_TDLS_TEARDOWN || action_code == TDLS_ACTION_CODE_AUTO_TEARDOWN) {
		/* fill reason code (unspecified reason 0x001a) */
		UINT16 ReasonCode;

		if (reason_code == 0)
			ReasonCode = 0x001a;
		else
			ReasonCode = reason_code;

		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  2,				&ReasonCode,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	if (action_code != WLAN_TDLS_TEARDOWN && action_code != TDLS_ACTION_CODE_AUTO_TEARDOWN) {
		/* fill Dialog Token */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&dialog_token,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	if (action_code == WLAN_TDLS_SETUP_REQUEST
		|| action_code == WLAN_PUB_ACTION_TDLS_DISCOVER_RES
		|| action_code == WLAN_TDLS_SETUP_RESPONSE) {
		/* USHORT capa = 0x420; */
		/* fill capability */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  2,				&pAd->StaActive.CapabilityInfo,
						  /* 2,		&capa, */
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
		/* fill support rate */
		MakeOutgoingFrame(pOutBuffer + FrameLen,					&TempLen,
						  1,							&SupRateIe,
						  1,						&pAd->StaActive.SupRateLen,
						  pAd->StaActive.SupRateLen,	pAd->StaActive.SupRate,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;

		/* fill country */
		/* add country IE, power constraint IE */
		if (pAd->CommonCfg.bCountryFlag) {
			ULONG TmpLen, TmpLen2 = 0;
			UCHAR TmpFrame[256];
			UCHAR CountryIe = IE_COUNTRY;

			os_zero_mem(TmpFrame, sizeof(TmpFrame));
			/* prepare channel information */
			{
				UCHAR regclass;
				UCHAR RegluatoryRxtIdent = 221;
				UCHAR CoverageClass = 0;

				regclass = cfg_tdls_GetRegulatoryClass(pAd, op_ht_bw, wdev->channel);
				MakeOutgoingFrame(TmpFrame + TmpLen2, &TmpLen,
								  1,				&RegluatoryRxtIdent,
								  1,				&regclass,
								  1,				&CoverageClass,
								  END_OF_ARGS);
				TmpLen2 += TmpLen;
			}

			/* need to do the padding bit check, and concatenate it */
			if ((TmpLen2 % 2) == 0) {
				UCHAR	TmpLen3 = TmpLen2 + 4;

				MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
								  1,				&CountryIe,
								  1,				&TmpLen3,
								  3,				pAd->CommonCfg.CountryCode,
								  TmpLen2 + 1,		TmpFrame,
								  END_OF_ARGS);
			} else {
				UCHAR	TmpLen3 = TmpLen2 + 3;

				MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
								  1,				&CountryIe,
								  1,				&TmpLen3,
								  3,				pAd->CommonCfg.CountryCode,
								  TmpLen2,			TmpFrame,
								  END_OF_ARGS);
			}

			FrameLen = FrameLen + TempLen;
		}

		/* fill ext rate */
		if (pAd->StaActive.ExtRateLen != 0) {
			MakeOutgoingFrame(pOutBuffer + FrameLen,					&TempLen,
							  1,							&ExtRateIe,
							  1,						&pAd->StaActive.ExtRateLen,
							  pAd->StaActive.ExtRateLen,	pAd->StaActive.ExtRate,
							  END_OF_ARGS);
			FrameLen = FrameLen + TempLen;
		}

		/* fill support channels */
		if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsChSwitchSupp) {
			UCHAR SupportChIe = IE_SUPP_CHANNELS;
			UCHAR ch_set[32], list_len;

			os_zero_mem(ch_set, sizeof(ch_set));
			list_len = 0;

			if (WMODE_CAP_2G(wdev->PhyMode)) {
				ch_set[list_len] = 1; /* first channel */
				ch_set[list_len + 1] = 11; /* channel number */
				list_len += 2;
			}

			if (WMODE_CAP_5G(wdev->PhyMode)) {
				ch_set[list_len] = 36; /* first channel */
				ch_set[list_len + 1] = 8; /* channel number */
				list_len += 2;
				ch_set[list_len] = 149; /* first channel */
				ch_set[list_len + 1] = 4; /* channel number */
				list_len += 2;
			}

			if (list_len > 0) {
				MakeOutgoingFrame(pOutBuffer + FrameLen, &TempLen,
								  1,	&SupportChIe,
								  1,	&list_len,
								  list_len,	&ch_set[0],
								  END_OF_ARGS);
				FrameLen = FrameLen + TempLen;
			}
		}

		/* fill RSN for discovery response if security is not NONE */
		if (action_code == WLAN_PUB_ACTION_TDLS_DISCOVER_RES &&
			pAd->StaCfg[0].wdev.WepStatus != Ndis802_11EncryptionDisabled) {
			UCHAR			CipherTmp[64] = {0};
			UCHAR			CipherTmpLen = 0;
			/* RSNIE (7.3.2.25) */
			CipherTmpLen = CipherSuiteTDLSLen;
			os_move_mem(CipherTmp, CipherSuiteTDLSWpa2PskAes, CipherTmpLen);
			/* Insert RSN_IE to outgoing frame */
			MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
							  CipherTmpLen,						&CipherTmp,
							  END_OF_ARGS);
			FrameLen = FrameLen + TempLen;
		}

		/* fill  Extended Capabilities (7.3.2.27) */
		Length = sizeof(EXT_CAP_INFO_ELEMENT);
		os_zero_mem(&extCapInfo, Length);
#ifdef DOT11N_DRAFT3

		if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
			(wdev->PhyMode >= PHY_11ABGN_MIXED) &&
			(wdev->Channel <= 14))
			extCapInfo.BssCoexistMgmtSupport = 1;

#endif /* DOT11N_DRAFT3 // */

		if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TdlsChSwitchSupp)
			extCapInfo.TDLSChSwitchSupport = 1;

		if (wdev->UapsdInfo.bAPSDCapable)
			extCapInfo.UAPSDBufSTASupport = 1;

		extCapInfo.TDLSSupport = 1;
#ifdef RT_BIG_ENDIAN
		RTMPEndianChange((UCHAR *)&extCapInfo, 8);
#endif

		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&ExtCapIe,
						  1,				&Length,
						  sizeof(EXT_CAP_INFO_ELEMENT), &extCapInfo,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;

		/* fill FTIE & Timeout Interval for discovery response if security is not NONE */
		if (action_code == WLAN_PUB_ACTION_TDLS_DISCOVER_RES &&
			pAd->StaCfg[0].wdev.WepStatus != Ndis802_11EncryptionDisabled) {
			FT_FTIE			FtIe;
			UCHAR			Length;
			UCHAR			SNonce[32];	/* Generated in Message 2, random variable */
			/* FTIE (7.3.2.48) */
			os_zero_mem(&FtIe, sizeof(FtIe));
			Length =  sizeof(FtIe);
			/* generate SNonce */
			GenRandom(pAd, pAd->CurrentAddress, FtIe.SNonce);
			os_move_mem(SNonce, FtIe.SNonce, 32);

			do {
				UINT16 MICCtrBuf;
				UCHAR FTIE = IE_FT_FTIE;

				MICCtrBuf = cpu2le16(FtIe.MICCtr.word);
				MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
								  1,				&FTIE,
								  1,				&Length,
								  2,				(PUCHAR)&MICCtrBuf,
								  16,				(PUCHAR)FtIe.MIC,
								  32,				(PUCHAR)FtIe.ANonce,
								  32,				(PUCHAR)FtIe.SNonce,
								  END_OF_ARGS);
				FrameLen = FrameLen + TempLen;
			} while (0);

			/* Timeout Interval (7.3.2.49) */
			do {
				UINT8 Length;
				UINT8 TimeOutIntervalIE;
				UINT8 TimeoutType;
				UINT32 TimeoutValueBuf;

				Length = 5;
				TimeOutIntervalIE = IE_FT_TIMEOUT_INTERVAL;
				TimeoutType = 2;
				TimeoutValueBuf = cpu2le32(3600);
				MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
								  1,				&TimeOutIntervalIE,
								  1,				&Length,
								  1,				(PUCHAR)&TimeoutType,
								  4,				(PUCHAR)&TimeoutValueBuf,
								  END_OF_ARGS);
				FrameLen = FrameLen + TempLen;
			} while (0);
		}

		/* fill Supported Regulatory Classes */
		do {
			UCHAR TDLS_IE = IE_SUPP_REG_CLASS;
			UCHAR Length = 16;
			UCHAR SuppClassesList[] = {1, 2, 3, 4, 12, 22, 23, 24, 25, 27, 28, 29, 30, 32, 33};
			UCHAR regclass;

			regclass = cfg_tdls_GetRegulatoryClass(pAd, op_ht_bw, wdev->channel);
			MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
							  1,				&TDLS_IE,
							  1,				&Length,
							  1,				&regclass,
							  15,				SuppClassesList,
							  END_OF_ARGS);
			FrameLen = FrameLen + TempLen;
		} while (0);

		/* fill HT Capability */
#ifdef DOT11_N_SUPPORT

		if (WMODE_CAP_N(wdev->PhyMode)) {
			UCHAR HtLen;
			HT_CAPABILITY_IE HtCapabilityTmp;

			HtLen = sizeof(HT_CAPABILITY_IE);
#ifndef RT_BIG_ENDIAN
			os_zero_mem(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			os_move_mem(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
			HtCapabilityTmp.HtCapInfo.ChannelWidth = op_ht_bw;
			MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
							  1,			&HtCapIe,
							  1,			&HtLen,
							  HtLen,		&HtCapabilityTmp,
							  END_OF_ARGS);
#else
			os_zero_mem(&HtCapabilityTmp, sizeof(HT_CAPABILITY_IE));
			os_move_mem(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, HtLen);
			HtCapabilityTmp.HtCapInfo.ChannelWidth = op_ht_bw;
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
			MakeOutgoingFrame(pOutBuffer + FrameLen,	&TempLen,
							  1,			&HtCapIe,
							  1,			&HtLen,
							  HtLen,		&HtCapabilityTmp,
							  END_OF_ARGS);
#endif
			FrameLen = FrameLen + TempLen;
		}

#ifdef DOT11N_DRAFT3
		/* fill 20/40 BSS Coexistence (7.3.2.61) */
		Length = 1;
		memset(&BssCoexistence, 0, sizeof(BSS_2040_COEXIST_IE));
		BssCoexistence.field.InfoReq = 1;
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&BssCoexistIe,
						  1,				&Length,
						  1,				&BssCoexistence.word,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
#endif /* DOT11N_DRAFT3 // */
#endif /* DOT11_N_SUPPORT // */
	}

	/* fill link identifier */
	if (action_code == WLAN_TDLS_SETUP_REQUEST ||
		action_code == WLAN_TDLS_SETUP_CONFIRM ||
		action_code == WLAN_TDLS_DISCOVERY_REQUEST ||
		action_code == WLAN_TDLS_TEARDOWN	   ||
		action_code == TDLS_ACTION_CODE_AUTO_TEARDOWN ||
		(action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION && pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].bInitiator) ||
		(action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE && pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].bInitiator)
	   ) {
		/* We are initiator. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				pAd->CurrentAddress,
						  6,				peer,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} else {
		/* We are repsonder. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				peer,
						  6,				pAd->CurrentAddress,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* fill WMM */
	if (action_code == WLAN_TDLS_SETUP_REQUEST ||
		action_code == WLAN_TDLS_SETUP_RESPONSE
	   ) {
		if (pAd->CommonCfg.bWmmCapable) {
			QBSS_STA_INFO_PARM QosInfo;
			UCHAR WmeParmIe[8] = {IE_VENDOR_SPECIFIC, 7, 0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};

			os_zero_mem(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

			if (wdev->UapsdInfo.bAPSDCapable) {
				QosInfo.UAPSD_AC_BE = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_BE; */
				QosInfo.UAPSD_AC_BK = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_BK; */
				QosInfo.UAPSD_AC_VI = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_VI; */
				QosInfo.UAPSD_AC_VO = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_VO; */
				QosInfo.MaxSPLength = 0;/* pAd->CommonCfg.TDLS_MaxSPLength; */
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls our uapsd> UAPSD %d %d %d %d %d!\n",
						 QosInfo.UAPSD_AC_BE,
						 QosInfo.UAPSD_AC_BK,
						 QosInfo.UAPSD_AC_VI,
						 QosInfo.UAPSD_AC_VO,
						 QosInfo.MaxSPLength));
			}

			MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
							  8,				WmeParmIe,
							  1,				&QosInfo,
							  END_OF_ARGS);
			FrameLen = FrameLen + TempLen;
		}
	} else if (action_code == WLAN_TDLS_SETUP_CONFIRM) {
		/* UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 0x18, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0x0f, 0x00, 0x03, 0xa4, 0x00, 0x00, */
		/*	      0x27, 0xa4, 0x00, 0x00, 0x42, 0x43, 0x5e, 0x00, 0x62, 0x32, 0x2f, 0x00};*/
		QBSS_STA_INFO_PARM QosInfo;
		ULONG TempLen;
		USHORT	idx;
		/* When the BSS is QoS capable, then the BSS QoS parameters shall be
		 * used by the TDLS peer STAs on the AP's channel, and the values
		 * indicated inside the TDLS Setup Confirm frame apply only for the
		 * off-channel. The EDCA parameters for the off-channel should be
		 * the same as those on the AP's channel when QoS is supported by the BSS,
		 * because this may optimize the channel switching process.
		 */
		UCHAR WmeParmIe[26] = {IE_VENDOR_SPECIFIC, 24, 0x00, 0x50, 0xf2, 0x02, 0x01, 0x01, 0, 0};

		os_zero_mem(&QosInfo, sizeof(QBSS_STA_INFO_PARM));

		if (wdev->UapsdInfo.bAPSDCapable) {
			QosInfo.UAPSD_AC_BE = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_BE; */
			QosInfo.UAPSD_AC_BK = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_BK; */
			QosInfo.UAPSD_AC_VI = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_VI; */
			QosInfo.UAPSD_AC_VO = 1;/* pAd->CommonCfg.TDLS_bAPSDAC_VO; */
			QosInfo.MaxSPLength = 0;/* pAd->CommonCfg.TDLS_MaxSPLength; */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls our uapsd> UAPSD %d %d %d %d %d!\n",
					 QosInfo.UAPSD_AC_BE,
					 QosInfo.UAPSD_AC_BK,
					 QosInfo.UAPSD_AC_VI,
					 QosInfo.UAPSD_AC_VO,
					 QosInfo.MaxSPLength));
		}

		WmeParmIe[8] |= *(PUCHAR)&QosInfo;

		for (idx = QID_AC_BK; idx <= QID_AC_VO; idx++) {
			WmeParmIe[10 + (idx * 4)] = (idx << 5)								+	 /* b5-6 is ACI */
										((UCHAR)pAd->CommonCfg.APEdcaParm[0].bACM[idx] << 4)	+	  /* b4 is ACM */
										(pAd->CommonCfg.APEdcaParm[0].Aifsn[idx] & 0x0f);			  /* b0-3 is AIFSN */
			WmeParmIe[11 + (idx * 4)] = (pAd->CommonCfg.APEdcaParm[0].Cwmax[idx] << 4)		+	 /* b5-8 is CWMAX */
										(pAd->CommonCfg.APEdcaParm[0].Cwmin[idx] & 0x0f);			  /* b0-3 is CWMIN */
			WmeParmIe[12 + (idx * 4)] = (UCHAR)(pAd->CommonCfg.APEdcaParm[0].Txop[idx] & 0xff);	 /* low byte of TXOP */
			WmeParmIe[13 + (idx * 4)] = (UCHAR)(pAd->CommonCfg.APEdcaParm[0].Txop[idx] >> 8);	 /* high byte of TXOP */
		}

		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
						  26,				WmeParmIe,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	if (action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE || (action_code == WLAN_TDLS_TEARDOWN && reason_code != 25)) {
		pMacEntry = MacTableLookup(pAd, peer);

		if (pMacEntry == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
					 __func__));
			return -1;
		}
	}

	if (action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION) {
		/* fill PU buffer status */
		MAC_TABLE_ENTRY	*pEntry;
		ULONG TempLen;
		UINT8 Length;
		UINT8 IeIdPuBufferStatus;
		UINT8 PuBufferStatus;
		UINT8 FlgIsAnyPktForBK, FlgIsAnyPktForBE;
		UINT8 FlgIsAnyPktForVI, FlgIsAnyPktForVO;
		/* get pEntry */
		pEntry = MacTableLookup(pAd, peer);

		if (pEntry == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
					 __func__));
			return -1;
		}

		/* init */
		Length = 1;
		IeIdPuBufferStatus = IE_TDLS_PU_BUFFER_STATUS;
		PuBufferStatus = 0;
#ifdef UAPSD_SUPPORT
		/* get queue status */
		UAPSD_QueueStatusGet(pAd, pEntry,
							 &FlgIsAnyPktForBK, &FlgIsAnyPktForBE,
							 &FlgIsAnyPktForVI, &FlgIsAnyPktForVO);
		PuBufferStatus |= (FlgIsAnyPktForBK == TRUE) ? 0x01 : 0x00;
		PuBufferStatus |= (FlgIsAnyPktForBE == TRUE) ? 0x02 : 0x00;
		PuBufferStatus |= (FlgIsAnyPktForVI == TRUE) ? 0x04 : 0x00;
		PuBufferStatus |= (FlgIsAnyPktForVO == TRUE) ? 0x08 : 0x00;
#endif /*UAPSD_SUPPORT*/
		/* init element */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&IeIdPuBufferStatus,
						  1,				&Length,
						  1,				&PuBufferStatus,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* add extra_ies */
	if (extra_ies_len != 0) {
		/* hex_dump("!!! extra_ies ", extra_ies, extra_ies_len); */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  extra_ies_len,				extra_ies,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* ///////////////// debug output ///////////////////////////// */
	switch (action_code) {
	case WLAN_TDLS_SETUP_REQUEST:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_TDLS_SETUP_REQUEST ==>\n"));
		break;

	case WLAN_TDLS_SETUP_RESPONSE:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_TDLS_SETUP_RESPONSE ==>\n"));
		break;

	case WLAN_TDLS_SETUP_CONFIRM:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_TDLS_SETUP_CONFIRM ==>\n"));
		break;

	case WLAN_TDLS_TEARDOWN:
	case TDLS_ACTION_CODE_AUTO_TEARDOWN:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_TDLS_TEARDOWN ==>\n"));
		break;

	case WLAN_TDLS_DISCOVERY_REQUEST:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_TDLS_DISCOVERY_REQUEST ==>\n"));
		break;

	case WLAN_PUB_ACTION_TDLS_DISCOVER_RES:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> WLAN_PUB_ACTION_TDLS_DISCOVERY_RESPONSE ==>\n"));
		break;

	case TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION ==>\n"));
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_REQ ==>\n"));
		break;

	case TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> TDLS_ACTION_CODE_WFD_TUNNELED_PROBE_RSP ==>\n"));
		break;

	default:
		CFG80211DBG(DBG_LVL_ERROR, ("80211> UNKNOWN TDLS PACKET ==>\n"));
		break;
	}

	hex_dump("TDLS send pack", pOutBuffer, FrameLen);

	if (send_by_tdls_link == TRUE || (action_code == WLAN_TDLS_TEARDOWN && reason_code != 25)) {	/* only uapsd PTR, normal teardown */
#ifdef UAPSD_SUPPORT
		if (action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE)
			RTMP_PS_VIRTUAL_WAKEUP_PEER(pMacEntry); /* peer can not sleep for a while */

#endif
		CFG80211DBG(DBG_LVL_ERROR, ("80211> SEND BY TDLS LINK! ==>\n"));
		RTMPToWirelessSta(pAd, pMacEntry, Header802_3,
						  LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);
	} else if (action_code == TDLS_ACTION_CODE_AUTO_TEARDOWN) {
		PNDIS_PACKET pPacket;
		NDIS_STATUS Status;

		Status = RTMPAllocateNdisPacket(pAd, &pPacket, Header802_3, LENGTH_802_3, pOutBuffer, (UINT)FrameLen);

		if (Status != NDIS_STATUS_SUCCESS)
			return -1;

		Status = ge_enq_req(pAd, pPacket, QID_AC_VI, &pAd->MacTab.tr_entry[BSSID_WCID], TRUE, NULL);

		if (Status != NDIS_STATUS_SUCCESS)
			CFG80211DBG(DBG_LVL_ERROR, ("########### enq ERR %d\n", Status));
	} else if (action_code != WLAN_PUB_ACTION_TDLS_DISCOVER_RES) {	/* discover_req,setup,teardown,uapsd PTI */
#ifdef UAPSD_SUPPORT
		if (action_code == TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION)
			ASIC_PS_CAN_NOT_SLEEP(pAd);

#endif /*UAPSD_SUPPORT*/
		RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
						  LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);
	} else {													/* dicovery_response */
		INT stat = 0;

		stat = MiniportMMRequest(pAd, QID_AC_VI, pOutBuffer, FrameLen);

		if (stat != 0)
			CFG80211DBG(DBG_LVL_ERROR, ("########### MINIPORT ERR %d\n", stat));
	}

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
	return 0;
}

INT cfg_tdls_EntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR arg)
{
	INT i;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n%-19s\n", "MAC\n"));

	for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("TDLS Entry %d MAC %02x:%02x:%02x:%02x:%02x:%02x\n", i, PRINT_MAC(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr)));

		if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].EntryValid == TRUE) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacTabMatchWCID];
			/* MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].MacAddr))); */
			/*MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-8d\n", pAd->StaCfg[0].DLSEntry[i].TimeOut)); */
			/* MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n")); */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("TDLS Entry %d is valid, bInitiator = %d\n"
					 , i, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[i].bInitiator));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n"));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n%-19s%-4s%-4s%-4s%-4s%-7s%-7s%-7s", "MAC", "AID", "BSS", "PSM", "WMM", "RSSI0", "RSSI1", "RSSI2"));
#ifdef DOT11_N_SUPPORT
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-8s%-10s%-6s%-6s%-6s%-6s", "MIMOPS", "PhMd", "BW", "MCS", "SGI", "STBC"));
#endif /* DOT11_N_SUPPORT */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n%02X:%02X:%02X:%02X:%02X:%02X  ",
					 pEntry->Addr[0], pEntry->Addr[1], pEntry->Addr[2],
					 pEntry->Addr[3], pEntry->Addr[4], pEntry->Addr[5]));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-4d", (int)pEntry->Aid));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-4d", (int)pEntry->apidx));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-4d", (int)pEntry->PsMode));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-4d", (int)CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE)));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[0]));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[1]));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-7d", pEntry->RssiSample.AvgRssi[2]));
#ifdef DOT11_N_SUPPORT
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-8d", (int)pEntry->MmpsMode));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-10s", get_phymode_str(pEntry->HTPhyMode.field.MODE)));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-6s", get_bw_str(pEntry->HTPhyMode.field.BW)));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.MCS));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.ShortGI));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-6d", pEntry->HTPhyMode.field.STBC));
#endif /* DOT11_N_SUPPORT */
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%-10d, %d, %d%%\n", pEntry->DebugFIFOCount, pEntry->DebugTxCount,
					 (pEntry->DebugTxCount) ? ((pEntry->DebugTxCount - pEntry->DebugFIFOCount) * 100 / pEntry->DebugTxCount) : 0));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("\n"));
		}
	}

	return TRUE;
}


VOID cfg_tdls_rx_parsing(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk)
{
	UCHAR	TDLS_LLC_SNAP_WITH_CATEGORY[] = {0xaa, 0xaa, 0x03, 0x00, 0x00, 0x00, 0x89, 0x0d, PROTO_NAME_TDLS, CATEGORY_TDLS};

	if ((pRxBlk->DataSize >= (LENGTH_802_1_H + LENGTH_TDLS_H))
		&& os_equal_mem(TDLS_LLC_SNAP_WITH_CATEGORY, pRxBlk->pData, LENGTH_802_1_H + 2)) {
		UCHAR	TDLSType, dialog_token;
		UCHAR	*peerMAC;
		UCHAR	nullAddr[MAC_ADDR_LEN];
		CHAR	*Ptr = pRxBlk->pData;
		ULONG	Length = 0;
		PEID_STRUCT		pEid;
		ULONG	RemainLen = pRxBlk->DataSize;
		PCFG_TDLS_STRUCT pCfgTdls = &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info;
		struct wifi_dev *wdev = &pAd->StaCfg[0].wdev;

		os_zero_mem(nullAddr, MAC_ADDR_LEN);
		TDLSType = *(pRxBlk->pData + LENGTH_802_1_H + 2);

		if (TDLSType == TDLS_ACTION_CODE_SETUP_REQUEST || TDLSType == TDLS_ACTION_CODE_SETUP_RESPONSE) {
			int i = 0, tdls_entry_index = 0;

			peerMAC = pRxBlk->pHeader->Addr3;

			for (i = 0; i < MAX_NUM_OF_CFG_TDLS_ENTRY; i++) {
				if (pCfgTdls->TDLSEntry[i].EntryValid == FALSE &&
					(MAC_ADDR_EQUAL(pCfgTdls->TDLSEntry[i].MacAddr, nullAddr) || MAC_ADDR_EQUAL(pCfgTdls->TDLSEntry[i].MacAddr, peerMAC))) {
					tdls_entry_index = i;  /* found an empty entry */
					break;
				}
			}

			if (i == MAX_NUM_OF_CFG_TDLS_ENTRY) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%s() - TDLS LINK NUMBER EXCEED, WE ONLY SUPPORT CONCURRENT %d LINKs, Discard this setup\n", __func__, MAX_NUM_OF_CFG_TDLS_ENTRY));
				/* RELEASE_NDIS_PACKET(pAd, pRxBlk->pRxPacket,NDIS_STATUS_FAILURE); */
				return;
			}

			COPY_MAC_ADDR(pCfgTdls->TDLSEntry[tdls_entry_index].MacAddr, peerMAC);

			if (TDLSType == TDLS_ACTION_CODE_SETUP_REQUEST) {
				Ptr += LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H + 1 + 2; /* token 1 + capa 2 */
				RemainLen -= LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H + 1 + 2; /* token 1 + capa 2 */
			} else {
				pCfgTdls->TDLSEntry[tdls_entry_index].bInitiator = TRUE;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%s() - TDLSEntry[%d] Initiator = %d\n", __func__, tdls_entry_index, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].bInitiator));
				Ptr += LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H + 2 + 1 + 2; /* status 2 + token 1 + capa 2 */
				RemainLen -= LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H + 2 + 1 + 2; /* status 2 + token 1 + capa 2 */
			}

#ifdef UAPSD_SUPPORT
			/* Add for 2 necessary EID field check */
			pEid = (PEID_STRUCT) Ptr;

			/* get variable fields from payload and advance the pointer */
			while ((Length + 2 + pEid->Len) <= RemainLen) {
				switch (pEid->Eid) {
				case IE_QOS_CAPABILITY:
					if (pEid->Len == 1)
						pCfgTdls->TDLSEntry[tdls_entry_index].QosCapability = *(pEid->Octet);

					break;

				case IE_VENDOR_SPECIFIC:

					/* handle WME PARAMTER ELEMENT */
					if (os_equal_mem(pEid->Octet, WME_INFO_ELEM, 6) && (pEid->Len == 7))
						pCfgTdls->TDLSEntry[tdls_entry_index].QosCapability = pEid->Octet[6];

					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s : RCV QosCapa 0x[%x] from peer MAC: (%02X:%02X:%02X:%02X:%02X:%02X)\n"
							 , __func__, pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_index].QosCapability, PRINT_MAC(peerMAC)));
					break;

				case IE_HT_CAP:
					if (wdev->PhyMode >= PHY_11ABGN_MIXED) {
						if (pEid->Len >= SIZE_HT_CAP_IE) { /* Note: allow extension.!! */
							os_move_mem(&pCfgTdls->TDLSEntry[tdls_entry_index].HtCapability, pEid->Octet, sizeof(HT_CAPABILITY_IE));
							pCfgTdls->TDLSEntry[tdls_entry_index].HtCapabilityLen = SIZE_HT_CAP_IE;	/* Nnow we only support 26 bytes. */
						}
					}

					break;

				default:
					break;
				}

				Length = Length + 2 + pEid->Len;
				pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
			}

#endif /*UAPSD_SUPPORT*/
		} else if (TDLSType == TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST) {
			/* DBGPRINT(RT_DEBUG_ERROR, ("\nPeerTdlsChannelSwitchReq!!\n")); */
			CFG_TDLS_CHSW_PARAM TdlsCHSWParam;

			os_zero_mem(&TdlsCHSWParam, sizeof(CFG_TDLS_CHSW_PARAM));
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("GOT CHSW REQ! %ld !!!\n", (jiffies * 1000) / OS_HZ));
			peerMAC = pRxBlk->pHeader->Addr2;

			if (pCfgTdls->IamInOffChannel) {
				/* Got Req in off channel */
				/* Offset to Target Channel */
				Ptr += LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;
				RemainLen -= LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;

				/* Get the value of target channel from payload and advance the pointer */
				if (RemainLen < 1) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - Invaild packet length - (target channel)\n", __func__));
					return;
				}

				pCfgTdls->TargetOffChannel = *Ptr;

				if (pCfgTdls->TargetOffChannel == pCfgTdls->BaseChannel) {	/* STOPCHSW */
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%s() - GOT SW Req in off channel with target = base %d, STOP CH SWITCH!!\n", __func__, pCfgTdls->BaseChannel));
					pCfgTdls->bDoingPeriodChannelSwitch = FALSE;
				}

				/* gen chsw_resp */
				cfg_tdls_chsw_resp(pAd, peerMAC, 11000, 19000, 0);
				/* CFG_TDLS_KYLE Send TDLS_BACK_TO_BASE */
				/* TdlsCHSWParam.cmd = 1; */
				/* RTEnqueueInternalCmd(pAd, CMDTHREAD_TDLS_SEND_CH_SW_SETUP, &TdlsCHSWParam, sizeof(CFG_TDLS_CHSW_PARAM)); */
			} else {
				/* in base channel , parse request for CHSW time */
				/* Offset to Target Channel */
				Ptr += LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;
				RemainLen -= LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;

				/* Get the value of target channel from payload and advance the pointer */
				if (RemainLen < 1) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - Invaild packet length - (target channel)\n", __func__));
					return;
				}

				pCfgTdls->TargetOffChannel = *Ptr;
				/* Offset to Regulatory Class */
				Ptr += 1;
				RemainLen -= 1;

				/* Get the value of regulatory class from payload and advance the pointer */
				if (RemainLen < 1) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - Invaild packet length - (regulatory class)\n", __func__));
					return;
				}

				/* *pRegulatoryClass = *Ptr; */
				/* DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s() - Regulatory class = %d\n", __FUNCTION__, *pRegulatoryClass)); */
				/* Offset to other elements */
				Ptr += 1;
				RemainLen -= 1;
				pEid = (PEID_STRUCT) Ptr;

				/* get variable fields from payload and advance the pointer */
				while ((Length + 2 + pEid->Len) <= RemainLen) {
					switch (pEid->Eid) {
					case IE_SECONDARY_CH_OFFSET:
						if (pEid->Len == 1)
							pCfgTdls->TargetOffChannelExt = pEid->Octet[0];
						else
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - wrong IE_SECONDARY_CH_OFFSET.\n", __func__));

						break;

					case IE_TDLS_LINK_IDENTIFIER:
						if (pEid->Len != TDLS_ELM_LEN_LINK_IDENTIFIER)
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - wrong IE_TDLS_LINK_IDENTIFIER.\n", __func__));

						break;

					case IE_TDLS_CHANNEL_SWITCH_TIMING:
						if (pEid->Len == 4) {
							os_move_mem(&pCfgTdls->ChSwitchTime, &pEid->Octet[0], 2);
							os_move_mem(&pCfgTdls->ChSwitchTimeout, &pEid->Octet[2], 2);
						} else
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() - wrong IE_TDLS_CHANNEL_SWITCH_TIMING.\n", __func__));

						break;

					default:
						/* Unknown IE, we have to pass it as variable IEs */
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_WARN, ("%s() - unrecognized EID = %d\n", __func__, pEid->Eid));
						break;
					}

					Length = Length + 2 + pEid->Len;
					pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
				}
			}

			if (pCfgTdls->ChSwitchTime <= TDLS_CHANNEL_SWITCH_TIME)
				pCfgTdls->ChSwitchTime = TDLS_CHANNEL_SWITCH_TIME;

			if (pCfgTdls->ChSwitchTimeout <= TDLS_CHANNEL_SWITCH_TIMEOUT)
				pCfgTdls->ChSwitchTimeout = TDLS_CHANNEL_SWITCH_TIMEOUT;

			if (pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited == TRUE) {
				/* gen CHSW Resp with status=37  return; */
				cfg_tdls_chsw_resp(pAd, peerMAC, pCfgTdls->ChSwitchTime, pCfgTdls->ChSwitchTimeout, 37);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_OFF, ("%s() - AP prohibit TDLS CH SWITH (%d), STOP!!\n", __func__, pAd->StaActive.ExtCapInfo.TDLSChSwitchProhibited));
				return;
			}

			/* STARTCHSW			//RX TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST */
			/* pCfgTdls->OffChannelStayTime = pAd->StaCfg[0].DtimPeriod *100 - pCfgTdls->BaseChannelStayTime; */
			pCfgTdls->BaseChannel = wlan_operate_get_cen_ch_1(wdev);
			pCfgTdls->OrigTargetOffChannel = pCfgTdls->TargetOffChannel;

			if (pCfgTdls->TargetOffChannelExt == 0x1)
				pCfgTdls->TargetOffChannel += 2;
			else if (pCfgTdls->TargetOffChannelExt == 0x3)
				pCfgTdls->TargetOffChannel -= 2;

			/* gen CHSW Resp */
			/* cfg_tdls_chsw_resp(pAd,peerMAC,pCfgTdls->ChSwitchTime,pCfgTdls->ChSwitchTimeout,0); */
			pCfgTdls->bDoingPeriodChannelSwitch = TRUE;
			/* RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev); */
			/* RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE); */
			/* CFG_TDLS_KYLE Send TDLS_CH_SW_START */
			COPY_MAC_ADDR(pCfgTdls->CHSWPeerMacAddr, peerMAC);
			pCfgTdls->TargetOffChannelBW = (pCfgTdls->TargetOffChannelExt) ? 1 : 0;
			TdlsCHSWParam.cmd = 0;
			TdlsCHSWParam.bw_off = pCfgTdls->TargetOffChannelBW;
			TdlsCHSWParam.offch_center = pCfgTdls->TargetOffChannel;
			TdlsCHSWParam.offch_prim = pCfgTdls->OrigTargetOffChannel;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("TdlsCHSWParam.offch_center : %d TdlsCHSWParam.offch_prim: %d pCfgTdls->TargetOffChannelExt: %d\n"
					  , TdlsCHSWParam.offch_center, TdlsCHSWParam.offch_prim, pCfgTdls->TargetOffChannelExt));
			TdlsCHSWParam.role = pCfgTdls->bChannelSwitchInitiator;
			/* TdlsCHSWParam.stay_time = pCfgTdls->OffChannelStayTime; */
			TdlsCHSWParam.switch_time = pCfgTdls->ChSwitchTime;
			TdlsCHSWParam.switch_timeout = pCfgTdls->ChSwitchTimeout;
			TdlsCHSWParam.start_time_tsf = pRxBlk->TimeStamp;
			RTEnqueueInternalCmd(pAd, CMDTHREAD_TDLS_SEND_CH_SW_SETUP, &TdlsCHSWParam, sizeof(CFG_TDLS_CHSW_PARAM));
			mdelay(1);
			cfg_tdls_chsw_resp(pAd, peerMAC, pCfgTdls->ChSwitchTime, pCfgTdls->ChSwitchTimeout, 0);
		} else if (TDLSType == TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE) {
			CFG_TDLS_CHSW_PARAM TdlsCHSWParam;
			USHORT StatusCode = 0;

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("GOT CHSW RESP! %ld !!!\n", (jiffies * 1000) / OS_HZ));
			peerMAC = pRxBlk->pHeader->Addr2;

			if (pCfgTdls->IamInOffChannel) {
				/* CFG_TDLS_KYLE Send TDLS_BACK_TO_BASE */
				/* TdlsCHSWParam.cmd = 1; */
				/* RTEnqueueInternalCmd(pAd, CMDTHREAD_TDLS_SEND_CH_SW_SETUP, &TdlsCHSWParam, sizeof(CFG_TDLS_CHSW_PARAM)); */
			} else if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IsentCHSW == TRUE && pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.bChannelSwitchInitiator == TRUE) {
				pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IsentCHSW = FALSE;  /* to prevent receiving off channel CHSW resp in base channel */
				/* Now in Base Channel, parse response for CHSW time */
				/* Offset to Status Code */
				Ptr += LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;
				RemainLen -= LENGTH_802_1_H + LENGTH_TDLS_PAYLOAD_H;

				/* Get the value of Status Code from payload and advance the pointer */
				if (RemainLen < 2) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("PeerTdlsChannelSwitchRspSanity --> Invaild packet length - (status code)\n"));
					return;
				}

				os_move_mem(&StatusCode, Ptr, 2);
				/* Offset to other elements */
				Ptr += 2;
				RemainLen -= 2;
				pEid = (PEID_STRUCT) Ptr;

				/* get variable fields from payload and advance the pointer */
				while ((Length + 2 + pEid->Len) <= RemainLen) {
					switch (pEid->Eid) {
					case IE_TDLS_LINK_IDENTIFIER:
						if (pEid->Len != TDLS_ELM_LEN_LINK_IDENTIFIER)
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_WARN, ("PeerTdlsChannelSwitchRspSanity - wrong IE_TDLS_LINK_IDENTIFIER.\n"));

						break;

					case IE_TDLS_CHANNEL_SWITCH_TIMING:
						if (pEid->Len == 4) {
							/* TDLS_CH_SWITCH_TIMING_ELEMENT ChSwitchTiming; */
							os_move_mem(&pCfgTdls->ChSwitchTime, &pEid->Octet[0], 2);
							os_move_mem(&pCfgTdls->ChSwitchTimeout, &pEid->Octet[2], 2);
							/* *pChSwitchTime = ChSwitchTiming.ChSwitchTime; */
							/* *pChSwitchTimeOut = ChSwitchTiming.ChSwitchTimeOut; */
						} else
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("PeerTdlsChannelSwitchRspSanity - wrong IE_TDLS_CHANNEL_SWITCH_TIMING.\n"));

						break;

					default:
						/* Unknown IE, we have to pass it as variable IEs */
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("PeerTdlsChannelSwitchRspSanity - unrecognized EID = %d\n", pEid->Eid));
						break;
					}

					Length = Length + 2 + pEid->Len;
					pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
				}

				/* STARTCHSW		//RX TDLS_ACTION_CODE_CHANNEL_SWITCH_RESP */
				pCfgTdls->OffChannelStayTime = pAd->StaCfg[0].DtimPeriod * 100 - pCfgTdls->BaseChannelStayTime;
				pCfgTdls->BaseChannel = wdev->channel;
				pCfgTdls->bDoingPeriodChannelSwitch = TRUE;
				/* RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev); */
				/* RTMP_SET_MORE_FLAG(pAd, fRTMP_ADAPTER_DISABLE_DEQUEUE); */
				/* CFG_TDLS_KYLE Send TDLS_CH_SW_START */
				COPY_MAC_ADDR(pCfgTdls->CHSWPeerMacAddr, peerMAC);
				/* pCfgTdls->TargetOffChannelBW = (pCfgTdls->TargetOffChannelExt)?1:0; */
				TdlsCHSWParam.cmd = 0;
				TdlsCHSWParam.offch_center = pCfgTdls->TargetOffChannel;
				TdlsCHSWParam.offch_prim = pCfgTdls->OrigTargetOffChannel;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						 ("TdlsCHSWParam.offch_center : %d TdlsCHSWParam.offch_prim: %d pCfgTdls->TargetOffChannelExt: %d\n"
						  , TdlsCHSWParam.offch_center, TdlsCHSWParam.offch_prim, pCfgTdls->TargetOffChannelExt));
				TdlsCHSWParam.bw_off = pCfgTdls->TargetOffChannelBW;
				TdlsCHSWParam.role = pCfgTdls->bChannelSwitchInitiator;
				/* TdlsCHSWParam.stay_time = pCfgTdls->OffChannelStayTime; */
				TdlsCHSWParam.switch_time = pCfgTdls->ChSwitchTime;
				TdlsCHSWParam.switch_timeout = pCfgTdls->ChSwitchTimeout;
				TdlsCHSWParam.start_time_tsf = pRxBlk->TimeStamp;
				RTEnqueueInternalCmd(pAd, CMDTHREAD_TDLS_SEND_CH_SW_SETUP, &TdlsCHSWParam, sizeof(CFG_TDLS_CHSW_PARAM));
			}
		}

#ifdef UAPSD_SUPPORT
		else if (TDLSType == TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION) { /* TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION */
			peerMAC = pRxBlk->pHeader->Addr3;
			dialog_token = *(pRxBlk->pData + LENGTH_802_1_H + 3);
			cfg_tdls_rcv_PeerTrafficIndication(pAd, dialog_token, peerMAC);
		} else if (TDLSType == TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE) {
			peerMAC = pRxBlk->pHeader->Addr2;
			dialog_token = *(pRxBlk->pData + LENGTH_802_1_H + 3);
			cfg_tdls_rcv_PeerTrafficResponse(pAd, peerMAC);
		}

#endif /*UAPSD_SUPPORT*/
	}
}

VOID cfg_tdls_BaseChannelTimeoutAction(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3)
{
	PRTMP_ADAPTER	pAd = (PRTMP_ADAPTER)FunctionContext;
	PCFG_TDLS_STRUCT pCfgTdls = &pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls CHSW base channel timeout!!!\n"));

	if (pCfgTdls->bChannelSwitchInitiator && pCfgTdls->bDoingPeriodChannelSwitch)
		cfg_tdls_chsw_req(pAd, pCfgTdls->CHSWPeerMacAddr, pCfgTdls->OrigTargetOffChannel, pCfgTdls->TargetOffChannelBW);
}

INT cfg_tdls_chsw_req(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8 *peer,
	IN UINT8 target_channel,
	IN UINT8 target_bw
)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UINT8	category = CATEGORY_TDLS;
	UCHAR	TDLS_IE = IE_TDLS_LINK_IDENTIFIER;
	UCHAR	TDLS_IE_LEN = TDLS_ELM_LEN_LINK_IDENTIFIER;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;
	UCHAR regclass;
	u8 action_code = TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST;
	UCHAR target_ext_ch_offset = EXTCHA_NONE;
	INT tdls_entry_link_index = 0;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("======>  %s()\n", __func__));
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("ACT - TDLS_SetupRequestAction() allocate memory failed\n"));
		return NStatus;
	}

	MAKE_802_3_HEADER(Header802_3, peer, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* fill action code */
	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
					  1,				&category,
					  1,				&action_code,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	/* fill target channel */
	MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
					  1,				&target_channel,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;

	/* fill reg class */

	if (target_bw != 0)
		target_bw = 1;

	/* regclass = cfg_tdls_GetRegulatoryClass(pAd, target_bw, target_channel); */
	regclass = 22;
	MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
					  1,				&regclass,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;

	if (target_bw != 0) {
		UCHAR length = 1;

		target_ext_ch_offset = pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TargetOffChannelExt;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("@@@ send CHSW request with TargetOffChannelExt : %d, Channel:%d\n", target_ext_ch_offset, target_channel));
		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
						  1,				&NewExtChanIe,
						  1,				&length,
						  1,				&target_ext_ch_offset,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} else {
		;/* target_ext_ch_offset = EXTCHA_NONE; */
	}

	tdls_entry_link_index = cfg_tdls_search_ValidLinkIndex(pAd, peer);

	if (tdls_entry_link_index == -1)
		return -1;

	if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].bInitiator) {
		/* We are initiator. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				pAd->CurrentAddress,
						  6,				peer,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} else {
		/* We are repsonder. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				peer,
						  6,				pAd->CurrentAddress,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* TDLS_InsertChannelSwitchTimingIE(pAd, (pOutBuffer + FrameLen), &FrameLen, SwitchTime, SwitchTimeout); */
	do {
		UCHAR TDLS_IE = IE_TDLS_CHANNEL_SWITCH_TIMING;
		UCHAR TDLS_IE_LEN = 4;
		UINT16	SwitchTimeBuf = cpu2le16(11000);
		UINT16	SwitchTimeOutBuf = cpu2le16(19000);

		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  2,				&SwitchTimeBuf,
						  2,				&SwitchTimeOutBuf,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} while (0);

	hex_dump("TDLS send chsw req pack", pOutBuffer, FrameLen);
	pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IsentCHSW = TRUE;
	{
		/* get pEntry */
		pMacEntry = MacTableLookup(pAd, peer);

		if (pMacEntry == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
					 __func__));
			return -1;
		}

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("Send CHSW REQ! %ld !!!\n", (jiffies * 1000) / OS_HZ));
		RTMPToWirelessSta(pAd, pMacEntry, Header802_3,
						  LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);
	}
	return 0;
}

INT cfg_tdls_chsw_resp(
	IN PRTMP_ADAPTER	pAd,
	IN UINT8 *peer,
	IN UINT32 ch_sw_time,
	IN UINT32 ch_sw_timeout,
	IN UINT16 reason_code
)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UINT8	category = CATEGORY_TDLS;
	UCHAR	TDLS_IE = IE_TDLS_LINK_IDENTIFIER;
	UCHAR	TDLS_IE_LEN = TDLS_ELM_LEN_LINK_IDENTIFIER;
	u8 action_code = TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE;
	INT tdls_entry_link_index = 0;
	MAC_TABLE_ENTRY	*pMacEntry = NULL;
	PNDIS_PACKET    pPacket;
	NDIS_STATUS     Status;
	struct wifi_dev *wdev;
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("ACT - TDLS_SetupRequestAction() allocate memory failed\n"));
		return NStatus;
	}

	MAKE_802_3_HEADER(Header802_3, peer, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* fill action code */
	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
					  1,				&category,
					  1,				&action_code,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	/* fill reason code */
	reason_code = cpu2le16(reason_code);
	MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
					  2,				&reason_code,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;
	/* fill link identifier */
	tdls_entry_link_index = cfg_tdls_search_ValidLinkIndex(pAd, peer);

	if (tdls_entry_link_index == -1)
		return -1;

	if (pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.TDLSEntry[tdls_entry_link_index].bInitiator) {
		/* We are initiator. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				pAd->CurrentAddress,
						  6,				peer,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} else {
		/* We are repsonder. */
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  6,				pAd->CommonCfg.Bssid,
						  6,				peer,
						  6,				pAd->CurrentAddress,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	/* Channel Switch Timing */
	/* TDLS_InsertChannelSwitchTimingIE(pAd, (pOutBuffer + FrameLen), &FrameLen, SwitchTime, SwitchTimeout); */
	do {
		UCHAR TDLS_IE = IE_TDLS_CHANNEL_SWITCH_TIMING;
		UCHAR TDLS_IE_LEN = 4;
		UINT16	SwitchTimeBuf = cpu2le16(ch_sw_time);
		UINT16	SwitchTimeOutBuf = cpu2le16(ch_sw_timeout);

		MakeOutgoingFrame(pOutBuffer + FrameLen,			&TempLen,
						  1,				&TDLS_IE,
						  1,				&TDLS_IE_LEN,
						  2,				&SwitchTimeBuf,
						  2,				&SwitchTimeOutBuf,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	} while (0);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s(): CHSW Resp Status:%d , SwitchTime:%d , SwitchTimeout:%d\n"
			 , __func__, reason_code, ch_sw_time, ch_sw_timeout));
	hex_dump("TDLS send chsw resp pack", pOutBuffer, FrameLen);
	/* get pEntry */
	pMacEntry = MacTableLookup(pAd, peer);

	if (pMacEntry == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("tdls_cmd> ERROR! No such peer in %s!\n",
				 __func__));
		return -1;
	}

	/* RTMPToWirelessSta(pAd, pMacEntry, Header802_3,LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE); */
	Status = RTMPAllocateNdisPacket(pAd, &pPacket, Header802_3, LENGTH_802_3, pOutBuffer, (UINT)FrameLen);

	if (Status	!= NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() ===> NOT ENOUGH MEMORY!\n", __func__));
		return Status;
	}

	RTMP_SET_PACKET_CLEAR_EAP_FRAME(pPacket, 0);
	RTMP_SET_PACKET_WCID(pPacket, (UCHAR)pMacEntry->wcid);
	/* TODO: shiang-usw, fix this! */
	RTMP_SET_PACKET_WDEV(pPacket, pMacEntry->wdev->wdev_idx);
	RTMP_SET_PACKET_MOREDATA(pPacket, FALSE);

	if (!pAd->StaCfg[0].wpa_supplicant_info.CFG_Tdls_info.IamInOffChannel)
		RTMP_SET_PACKET_TDLS_WAIT_ACK(pPacket, 1); /* SET TDLS WAIT ACK */

	/* send out the packet */
	wdev = pMacEntry->wdev;

	if (wdev && wdev->tx_pkt_handle)
		wdev->tx_pkt_handle(pAd, pPacket);
	else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s():Invalid wdev(%p) or tx_pkt_handle(%p)!\n",
				 __func__, wdev, (wdev ? wdev->tx_pkt_handle : NULL)));
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS))
		RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_INFO, ("<======  %s() out\n", __func__));
	return 0;
}
VOID cfg_tdls_prepare_null_frame(PRTMP_ADAPTER	pAd, BOOLEAN powersave, UCHAR dir, UCHAR *peerAddr)
{
#ifdef MT_MAC
	return;
#else
	PMAC_TABLE_ENTRY pEntry_TDLS = MacTableLookup(pAd, peerAddr);

	RtmpPrepareHwNullFrame(pAd,
						   &pAd->MacTab.Content[BSSID_WCID],
						   FALSE,
						   FALSE,
						   0,
						   OPMODE_STA,
						   PWR_SAVE,
						   0,
						   0);
	RtmpPrepareHwNullFrame(pAd,
						   pEntry_TDLS,
						   TRUE,
						   FALSE,
						   0,
						   OPMODE_STA,
						   PWR_ACTIVE,
						   TRUE,
						   1);
#endif
}


VOID cfg_tdls_auto_teardown(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry)
{
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s(): auto teardown link with (%02X:%02X:%02X:%02X:%02X:%02X)!!\n"
			 , __func__, PRINT_MAC(pEntry->Addr)));
#if (KERNEL_VERSION(3, 8, 0) <= LINUX_VERSION_CODE)
	cfg80211_tdls_oper_request(pAd->net_dev, pEntry->Addr, NL80211_TDLS_TEARDOWN, 25, GFP_KERNEL);
	mdelay(1);
	cfg80211_tdls_oper_request(pAd->net_dev, pEntry->Addr, NL80211_TDLS_DISABLE_LINK, 0, GFP_KERNEL);
#else
	cfg_tdls_build_frame(pAd, pEntry->Addr, 1, TDLS_ACTION_CODE_TEARDOWN, 0, NULL, 0, FALSE, 0, 25);
	mdelay(1);
	CFG80211DRV_StaTdlsInsertDeletepEntry(pAd, pEntry->Addr, tdls_delete_entry);
#endif /* LINUX_VERSION_CODE: 3.8 */
}

/*
 * ==========================================================================
 *	Description: WFD
 *
 *	IRQL = PASSIVE_LEVEL
 * ==========================================================================
 */
VOID cfg_tdls_TunneledProbeRequest(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR	Category = 0x7F;
	UCHAR	OUI[3] = {0x50, 0x6F, 0x9A};
	UCHAR	FrameBodyType = 4; /* 4: Request. 5: Response. */

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("TDLS ===> TDLS_TunneledProbeRequest\n"));
	MAKE_802_3_HEADER(Header802_3, pMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() ===> NOT ENOUGH MEMORY!\n", __func__));
		return;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  1,				&Category,
					  3,				&OUI,
					  1,				&FrameBodyType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;

	/* add extra_ies */
	if (extra_ies_len != 0) {
		hex_dump("extra_ies from supplicant ", extra_ies, extra_ies_len);
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  extra_ies_len,				extra_ies,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
					  LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);
	hex_dump("TDLS tunneled request send pack", pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("TDLS <=== TDLS_TunneledProbeRequest\n"));
}

/*
 * ==========================================================================
 *	Description: WFD
 *
 *	IRQL = PASSIVE_LEVEL
 * ==========================================================================
 */
VOID cfg_tdls_TunneledProbeResponse(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len)
{
	UCHAR	TDLS_ETHERTYPE[] = {0x89, 0x0d};
	UCHAR	Header802_3[14];
	PUCHAR	pOutBuffer = NULL;
	ULONG	FrameLen = 0;
	ULONG	TempLen;
	UCHAR	RemoteFrameType = PROTO_NAME_TDLS;
	NDIS_STATUS	NStatus = NDIS_STATUS_SUCCESS;
	UCHAR	Category = 0x7F;
	UCHAR	OUI[3] = {0x50, 0x6F, 0x9A};
	UCHAR	FrameBodyType = 5; /* 4: Request. 5: Response. */
	MAKE_802_3_HEADER(Header802_3, pMacAddr, pAd->CurrentAddress, TDLS_ETHERTYPE);
	/* Allocate buffer for transmitting message */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus	!= NDIS_STATUS_SUCCESS) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_ERROR, ("%s() ===> NOT ENOUGH MEMORY!\n", __func__));
		return;
	}

	MakeOutgoingFrame(pOutBuffer,		&TempLen,
					  1,				&RemoteFrameType,
					  1,				&Category,
					  3,				&OUI,
					  1,				&FrameBodyType,
					  END_OF_ARGS);
	FrameLen = FrameLen + TempLen;

	/* add extra_ies */
	if (extra_ies_len != 0) {
		hex_dump("extra_ies from supplicant ", extra_ies, extra_ies_len);
		MakeOutgoingFrame(pOutBuffer + FrameLen,		&TempLen,
						  extra_ies_len,				extra_ies,
						  END_OF_ARGS);
		FrameLen = FrameLen + TempLen;
	}

	RTMPToWirelessSta(pAd, &pAd->MacTab.Content[BSSID_WCID], Header802_3,
					  LENGTH_802_3, pOutBuffer, (UINT)FrameLen, FALSE);
	hex_dump("TDLS tunneled response send pack", pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_TDLS, DBG_LVL_TRACE, ("TDLS <=== TDLS_TunneledProbeResponse\n"));
}


#endif /*CFG_TDLS_SUPPORT*/

#endif /* RT_CFG80211_SUPPORT */
