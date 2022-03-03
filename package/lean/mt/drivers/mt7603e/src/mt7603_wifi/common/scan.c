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

#ifdef SCAN_SUPPORT
static INT scan_ch_restore(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
	INT bw, ch;

#ifdef WH_EZ_SETUP
	struct wifi_dev *wdev = pAd->ApCfg.ScanReqwdev;	

	if (IS_EZ_SETUP_ENABLED(wdev) && ez_handle_scan_channel_restore(pAd->ApCfg.ScanReqwdev)) {
		
	}
#endif /* WH_EZ_SETUP */
		

        if (pAd->CommonCfg.BBPCurrentBW != pAd->hw_cfg.bbp_bw)
                bbp_set_bw(pAd, pAd->hw_cfg.bbp_bw);

#ifdef DOT11_VHT_AC
	if (pAd->hw_cfg.bbp_bw == BW_80)
		ch = pAd->CommonCfg.vht_cent_ch;
	else 
#endif /* DOT11_VHT_AC */
	if (pAd->hw_cfg.bbp_bw == BW_40)
		ch = pAd->CommonCfg.CentralChannel;
        else
		ch = pAd->CommonCfg.Channel;

	switch(pAd->CommonCfg.BBPCurrentBW)
	{
		case BW_80:
			bw = 80;
			break;
		case BW_40:
			bw = 40;
			break;
		case BW_10:
			bw = 10;
			break;
		case BW_20:
		default:
			bw =20;
			break;
	}
		

	ASSERT((ch != 0));
       AsicSwitchChannel(pAd, ch, FALSE); 
       AsicLockChannel(pAd, ch);

	DBGPRINT(RT_DEBUG_TRACE, ("SYNC - End of SCAN, restore to %dMHz channel %d, Total BSS[%02d]\n",
				bw, ch, pAd->ScanTab.BssNr));
		

#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
	{
#ifdef APCLI_SUPPORT
#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE)
#ifdef AP_PARTIAL_SCAN_SUPPORT
				&& (pAd->ApCfg.bPartialScanning == FALSE)
#endif /* AP_PARTIAL_SCAN_SUPPORT */
			)
			{
				if (!ApCliAutoConnectExec(pAd))
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Error in  %s\n", __FUNCTION__));
				}
			}			
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#endif /* APCLI_SUPPORT */
		pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
		RTMPResumeMsduTransmission(pAd);

		/* iwpriv set auto channel selection*/
		/* scanned all channels*/
		if (pAd->ApCfg.bAutoChannelAtBootup==TRUE)
		{
#ifdef ACS_CTCC_SUPPORT
			if (pAd->ApCfg.AutoChannelScoreFlag == FALSE) {
#endif
			pAd->CommonCfg.Channel = SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
			pAd->ApCfg.bAutoChannelAtBootup = FALSE;
#ifdef DOT11_N_SUPPORT
			N_ChannelCheck(pAd);
#endif /* DOT11_N_SUPPORT */
			APStop(pAd);
			APStartUp(pAd);
		}
#ifdef ACS_CTCC_SUPPORT
			else {
				SelectBestChannel(pAd, pAd->ApCfg.AutoChannelAlg);
				pAd->ApCfg.bAutoChannelAtBootup = FALSE;
				pAd->ApCfg.AutoChannelScoreFlag = FALSE;
				RTMPSendWirelessEvent(pAd, IW_CHANNEL_CHANGE_EVENT_FLAG, 0, 0, 0);
			}
		}
#endif
		if (!((pAd->CommonCfg.Channel > 14) && (pAd->CommonCfg.bIEEE80211H == TRUE) && (pAd->Dot11_H.RDMode != RD_NORMAL_MODE)))
			AsicEnableBssSync(pAd, pAd->CommonCfg.BeaconPeriod);

#ifdef APCLI_SUPPORT
#ifdef WSC_AP_SUPPORT
		if (pwdev && (pwdev->wdev_type == WDEV_TYPE_STA)
			&& (pwdev->func_idx < MAX_APCLI_NUM)) {
			WSC_CTRL *pWpsCtrlTemp = &pAd->ApCfg.ApCliTab[pwdev->func_idx].WscControl;

			if ((pWpsCtrlTemp->WscConfMode != WSC_DISABLE) &&
				(pWpsCtrlTemp->bWscTrigger == TRUE) &&
				(pWpsCtrlTemp->WscMode == WSC_PBC_MODE)) {
				if (pWpsCtrlTemp->WscApCliScanMode == TRIGGER_PARTIAL_SCAN) {
					if ((pAd->ScanCtrl.PartialScan.bScanning == FALSE) &&
						(pAd->ScanCtrl.PartialScan.LastScanChannel == 0)) {
						DBGPRINT(RT_DEBUG_TRACE,
								("[%s] %s AP-Client WPS Partial Scan done!!!\n",
								__func__, (ch > 14 ? "5G" : "2G")));

						{
							if (!pWpsCtrlTemp->WscPBCTimerRunning) {
								RTMPSetTimer(&pWpsCtrlTemp->WscPBCTimer, 1000);
								pWpsCtrlTemp->WscPBCTimerRunning = TRUE;
							}
						}
					}
				} else {
					}
				}
			}
#endif /* WSC_AP_SUPPORT */
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


	return TRUE;
}



static INT scan_active(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType)
{
	UCHAR *frm_buf = NULL;
	HEADER_802_11 Hdr80211;
	ULONG FrameLen = 0;
	UCHAR SsidLen = 0;


	if (MlmeAllocateMemory(pAd, &frm_buf) != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():allocate memory fail\n", __FUNCTION__));

#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif /* CONFIG_AP_SUPPORT */
		return FALSE;
	}

#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (ScanType == SCAN_2040_BSS_COEXIST)
	{
		DBGPRINT(RT_DEBUG_INFO, ("SYNC - SCAN_2040_BSS_COEXIST !! Prepare to send Probe Request\n"));
	}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
	
	/* There is no need to send broadcast probe request if active scan is in effect.*/
	SsidLen = 0;
	if ((ScanType == SCAN_ACTIVE) || (ScanType == FAST_SCAN_ACTIVE)
#ifdef WSC_STA_SUPPORT
		|| ((ScanType == SCAN_WSC_ACTIVE) && (OpMode == OPMODE_STA))
#endif /* WSC_STA_SUPPORT */
		)
		SsidLen = pAd->ScanCtrl.SsidLen;

	{
#ifdef CONFIG_AP_SUPPORT
		/*IF_DEV_CONFIG_OPMODE_ON_AP(pAd) */
		if (OpMode == OPMODE_AP)
		{
			MgtMacHeaderInitExt(pAd, &Hdr80211, SUBTYPE_PROBE_REQ, 0, BROADCAST_ADDR,
								pAd->ApCfg.MBSSID[0].wdev.bssid,
								BROADCAST_ADDR);
		}
#endif /* CONFIG_AP_SUPPORT */

		MakeOutgoingFrame(frm_buf,               &FrameLen,
						  sizeof(HEADER_802_11),    &Hdr80211,
						  1,                        &SsidIe,
						  1,                        &SsidLen,
						  SsidLen,			        pAd->ScanCtrl.Ssid,
						  1,                        &SupRateIe,
						  1,                        &pAd->CommonCfg.SupRateLen,
						  pAd->CommonCfg.SupRateLen,  pAd->CommonCfg.SupRate, 
						  END_OF_ARGS);

		if (pAd->CommonCfg.ExtRateLen)
		{
			ULONG Tmp;
			MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
							  1,                                &ExtRateIe,
							  1,                                &pAd->CommonCfg.ExtRateLen,
							  pAd->CommonCfg.ExtRateLen,          pAd->CommonCfg.ExtRate, 
							  END_OF_ARGS);
			FrameLen += Tmp;
		}
	}

#ifdef DOT11_N_SUPPORT
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		ULONG	Tmp;
		UCHAR	HtLen;
#ifdef RT_BIG_ENDIAN
		HT_CAPABILITY_IE HtCapabilityTmp;
#endif

		{
			HtLen = sizeof(HT_CAPABILITY_IE);
#ifdef RT_BIG_ENDIAN
			NdisMoveMemory(&HtCapabilityTmp, &pAd->CommonCfg.HtCapability, SIZE_HT_CAP_IE);
			*(USHORT *)(&HtCapabilityTmp.HtCapInfo) = SWAP16(*(USHORT *)(&HtCapabilityTmp.HtCapInfo));
#ifdef UNALIGNMENT_SUPPORT
			{
				EXT_HT_CAP_INFO extHtCapInfo;

				NdisMoveMemory((PUCHAR)(&extHtCapInfo), (PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), sizeof(EXT_HT_CAP_INFO));
				*(USHORT *)(&extHtCapInfo) = cpu2le16(*(USHORT *)(&extHtCapInfo));
				NdisMoveMemory((PUCHAR)(&HtCapabilityTmp.ExtHtCapInfo), (PUCHAR)(&extHtCapInfo), sizeof(EXT_HT_CAP_INFO));		
			}
#else				
			*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo) = cpu2le16(*(USHORT *)(&HtCapabilityTmp.ExtHtCapInfo));
#endif /* UNALIGNMENT_SUPPORT */

			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &HtCapabilityTmp, 
							END_OF_ARGS);
#else
			MakeOutgoingFrame(frm_buf + FrameLen,          &Tmp,
							1,                                &HtCapIe,
							1,                                &HtLen,
							HtLen,                            &pAd->CommonCfg.HtCapability, 
							END_OF_ARGS);
#endif /* RT_BIG_ENDIAN */
		}
		FrameLen += Tmp;

#ifdef DOT11N_DRAFT3
		if ((pAd->ScanCtrl.Channel <= 14) && (pAd->CommonCfg.bBssCoexEnable == TRUE))
		{
			ULONG Tmp;
			HtLen = 1;
			MakeOutgoingFrame(frm_buf + FrameLen,            &Tmp,
							  1,					&ExtHtCapIe,
							  1,					&HtLen,
							  1,          			&pAd->CommonCfg.BSSCoexist2040.word, 
							  END_OF_ARGS);

			FrameLen += Tmp;
		}
#endif /* DOT11N_DRAFT3 */
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) &&
		(pAd->ScanCtrl.Channel > 14)) {		
		FrameLen += build_vht_ies(pAd, (UCHAR *)(frm_buf + FrameLen), SUBTYPE_PROBE_REQ);
	}
#endif /* DOT11_VHT_AC */

#ifdef WSC_STA_SUPPORT
	if (OpMode == OPMODE_STA)
	{
		BOOLEAN bHasWscIe = FALSE;
		/* 
			Append WSC information in probe request if WSC state is running
		*/
		if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg.WscControl.WscConfMode != WSC_DISABLE) &&
			(pAd->StaCfg.WscControl.bWscTrigger == TRUE))
			bHasWscIe = TRUE;
#ifdef WSC_V2_SUPPORT
		else if ((pAd->StaCfg.WscControl.WscEnProbeReqIE) && 
			(pAd->StaCfg.WscControl.WscV2Info.bEnableWpsV2))
			bHasWscIe = TRUE;
#endif /* WSC_V2_SUPPORT */


		if (bHasWscIe)
		{
			UCHAR *pWscBuf = NULL, WscIeLen = 0;
			ULONG WscTmpLen = 0;

			os_alloc_mem(NULL, (UCHAR **)&pWscBuf, 512);
			if (pWscBuf != NULL)
			{
				NdisZeroMemory(pWscBuf, 512);
				WscBuildProbeReqIE(pAd, STA_MODE, &pAd->StaCfg.WscControl, pWscBuf, &WscIeLen);

				MakeOutgoingFrame(frm_buf + FrameLen,              &WscTmpLen,
								WscIeLen,                             pWscBuf,
								END_OF_ARGS);

				FrameLen += WscTmpLen;
				os_free_mem(NULL, pWscBuf);
			}
			else
				DBGPRINT(RT_DEBUG_WARN, ("%s:: WscBuf Allocate failed!\n", __FUNCTION__));
		}
	}

#endif /* WSC_STA_SUPPORT */



#ifdef WH_EZ_SETUP
			if (ez_is_triband()) {
#ifdef EZ_NETWORK_MERGE_SUPPORT	
					ULONG  tmp_len;
					/*
						Insert capability TLV
					*/
					ez_triband_insert_tlv(pAd, EZ_TAG_CAPABILITY_INFO, 
						frm_buf + FrameLen, 
						&tmp_len);
					FrameLen += tmp_len;

#endif		
			}
#endif

	MiniportMMRequest(pAd, 0, frm_buf, FrameLen);


	MlmeFreeMemory(pAd, frm_buf);

	return TRUE;
}

/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */

#if defined(WH_EZ_SETUP) || defined(MAP_SUPPORT)
#ifdef APCLI_SUPPORT
static void FireExtraProbeReq(RTMP_ADAPTER *pAd, UCHAR OpMode, UCHAR ScanType, 
	struct wifi_dev *wdev,  UCHAR *desSsid, UCHAR desSsidLen)
{
	UCHAR backSsid[MAX_LEN_OF_SSID];
	UCHAR backSsidLen = 0;

	NdisZeroMemory(backSsid, MAX_LEN_OF_SSID);

	//1. backup the original MlmeAux
	backSsidLen = pAd->ScanCtrl.SsidLen;
	NdisCopyMemory(backSsid, pAd->ScanCtrl.Ssid, backSsidLen);
	
	//2. fill the desried ssid into SM
	pAd->ScanCtrl.SsidLen = desSsidLen;
	NdisCopyMemory(pAd->ScanCtrl.Ssid, desSsid, desSsidLen);

	//3. scan action
	scan_active(pAd, OpMode, ScanType);

	//4. restore to ScanCtrl
	pAd->ScanCtrl.SsidLen  = backSsidLen;
	NdisCopyMemory(pAd->ScanCtrl.Ssid, backSsid, backSsidLen);
}
#endif /* APCLI_SUPPORT */
#endif

/*
	==========================================================================
	Description:
		Scan next channel
	==========================================================================
 */
VOID ScanNextChannel(RTMP_ADAPTER *pAd, UCHAR OpMode, struct wifi_dev *pwdev)
{
	UCHAR ScanType = SCAN_TYPE_MAX;
	UINT ScanTimeIn5gChannel = SHORT_CHANNEL_TIME;
	BOOLEAN ScanPending = FALSE;
	RALINK_TIMER_STRUCT *sc_timer = NULL;
	UINT stay_time = 0;
#ifdef APCLI_SUPPORT
#endif
#ifdef WH_EZ_SETUP
	CHAR apcli_idx = -1;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	struct wifi_dev *wdev = pAd->ApCfg.ScanReqwdev;
	APCLI_STRUCT *pApCliTable = NULL;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	if(wdev) {
		apcli_idx = wdev->func_idx;
		pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[apcli_idx].wdev.bssid);
	}
#endif
#endif
	if(apcli_idx != -1) 
		pApCliTable = &pAd->ApCfg.ApCliTab[apcli_idx];

#endif

#ifdef CONFIG_ATE
	/* Nothing to do in ATE mode. */
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */



#ifdef CONFIG_AP_SUPPORT
	if (OpMode == OPMODE_AP)
		ScanType = pAd->ScanCtrl.ScanType;
#endif /* CONFIG_AP_SUPPORT */
	if (ScanType == SCAN_TYPE_MAX) {
#ifdef WH_EZ_SETUP
		if(pApCliTable && (IS_EZ_SETUP_ENABLED(&pApCliTable->wdev))
			&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
			pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif
		DBGPRINT(RT_DEBUG_ERROR, ("%s():Incorrect ScanType!\n", __FUNCTION__));
		return;
	}
	if ((pAd->ScanCtrl.Channel == 0) || ScanPending) 
	{
		scan_ch_restore(pAd, OpMode, pwdev);
#ifdef NEIGHBORING_AP_STAT
		if (!pAd->ApCfg.bPartialScanning) {
			RtmpOSWrielessEventSend(pAd->net_dev,
				RT_WLAN_EVENT_CUSTOM,
				OID_CUST_SCAN_DONE_EVENT,
				NULL,
				(UCHAR *)&pAd->ScanTab.BssNr,
				sizeof(pAd->ScanTab.BssNr));
			DBGPRINT(RT_DEBUG_OFF, ("Send Scan Result of Size:%d\n", pAd->ScanTab.BssNr));
		}
#endif

#ifdef WH_EZ_SETUP
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
		if(pMacEntry && pApCliTable && pApCliTable->Valid 
			&& IS_EZ_SETUP_ENABLED(&pApCliTable->wdev))
		{
			ApCliRTMPSendNullFrame(pAd,pMacEntry->CurrTxRate, FALSE, pMacEntry, PWR_ACTIVE);
		}
		if (IS_EZ_SETUP_ENABLED(&pApCliTable->wdev) && wdev->ez_driver_params.ez_scan == TRUE)
		{
			wdev->ez_driver_params.ez_scan = FALSE;
			EZ_DEBUG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Start Join For  wdev-> type = %d wdev = %d\n", wdev->wdev_type,wdev->wdev_idx));
			ApCliIfUp(pAd);
			
		} 
	
#ifdef MOBILE_APP_SUPPORT
		else if (pApCliTable && IS_EZ_SETUP_ENABLED(&pApCliTable->wdev) ){
			int i=0;
			unsigned char bssid[6] = {0};
					
			for (i = 0; i < pAd->ScanTab.BssNr; i++) 
				{
					BSS_ENTRY *bss_entry = NULL;					
					PAPCLI_STRUCT apcli_entry = &pAd->ApCfg.ApCliTab[apcli_idx];
					bss_entry = &pAd->ScanTab.BssEntry[i];
											

					if (bss_entry->non_ez_beacon)
					{
						continue;
					}
					if (bss_entry->support_easy_setup == TRUE)
					{
						continue;		
					}
					if (SSID_EQUAL(apcli_entry->CfgSsid, apcli_entry->CfgSsidLen,bss_entry->Ssid, bss_entry->SsidLen))
					{
						
						COPY_MAC_ADDR(bssid, bss_entry->Bssid);
					}
				}
				{
					RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, OID_WH_EZ_MAN_TRIBAND_SCAN_COMPLETE_EVENT,
							NULL, bssid, MAC_ADDR_LEN);
				}	
		}
#endif			
#endif	//APCLI_SUPPORT
#endif	//CONFIG_AP_SUPPORT
#endif	// WH_EZ_SETUP

	} 
	else 
	{

#ifdef WIFI_REGION32_HIDDEN_SSID_SUPPORT
                if (((pAd->ScanCtrl.Channel == 12) || (pAd->ScanCtrl.Channel == 13)) &&
					((pAd->CommonCfg.CountryRegion & 0x7f) == REGION_32_BG_BAND))
                        CmdIdConfigInternalSetting(pAd, DPD_CONF, DPD_OFF);
                else
                        CmdIdConfigInternalSetting(pAd, DPD_CONF, DPD_ON);
#endif /* WIFI_REGION32_HIDDEN_SSID_SUPPORT */


#ifdef WH_EZ_SETUP
		if (IS_EZ_SETUP_ENABLED(wdev) && (wdev && wdev->ez_driver_params.scan_one_channel)) {
			EZ_DEBUG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("AP SYNC %s() - Only scan ch.%d and keep original channel setting.\n", 
				__FUNCTION__, pAd->ScanCtrl.Channel));
		}
		else
#endif /* WH_EZ_SETUP */
		{
			AsicSwitchChannel(pAd, pAd->ScanCtrl.Channel, TRUE);
			AsicLockChannel(pAd, pAd->ScanCtrl.Channel);
		}

		/* Check if channel if passive scan under current regulatory domain */
#ifdef P2P_CHANNEL_LIST_SEPARATE
		if ((pAd->ScanCtrl.ScanType == SCAN_P2P))
		{
			if (P2P_CHAN_PropertyCheck(pAd, pAd->ScanCtrl.Channel, CHANNEL_PASSIVE_SCAN) == TRUE)
				ScanType = SCAN_PASSIVE;
		}
		else
#endif /* P2P_CHANNEL_LIST_SEPARATE */
		if (CHAN_PropertyCheck(pAd, pAd->ScanCtrl.Channel, CHANNEL_PASSIVE_SCAN) == TRUE)
			ScanType = SCAN_PASSIVE;

#if defined(DPA_T) || defined(WIFI_REGION32_HIDDEN_SSID_SUPPORT)
		/* Ch 12~14 is passive scan, No matter DFS and 80211H setting is y or n */
		if ((pAd->ScanCtrl.Channel >= 12) && (pAd->ScanCtrl.Channel <= 14) 
			&& ((pAd->CommonCfg.CountryRegion & 0x7f) == REGION_32_BG_BAND))
			ScanType = SCAN_PASSIVE;
#endif /* DPA_T */

#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			sc_timer = &pAd->ScanCtrl.APScanTimer;
#endif /* CONFIG_AP_SUPPORT */
		if (!sc_timer) {
#ifdef WH_EZ_SETUP
			if((IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[apcli_idx].wdev))
				&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
				pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif						
			DBGPRINT(RT_DEBUG_ERROR, ("%s():ScanTimer not assigned!\n", __FUNCTION__));
			return;
		}
			
		/* We need to shorten active scan time in order for WZC connect issue */
		/* Chnage the channel scan time for CISCO stuff based on its IAPP announcement */
		if (ScanType == FAST_SCAN_ACTIVE)
			stay_time = FAST_ACTIVE_SCAN_TIME;
		else /* must be SCAN_PASSIVE or SCAN_ACTIVE*/
		{

#ifdef CONFIG_AP_SUPPORT
			if ((OpMode == OPMODE_AP) && (pAd->ApCfg.bAutoChannelAtBootup))
				stay_time = AUTO_CHANNEL_SEL_TIMEOUT;
			else
#endif /* CONFIG_AP_SUPPORT */
			if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode) &&
				WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
			{
				if (pAd->ScanCtrl.Channel > 14)
					stay_time = ScanTimeIn5gChannel;
				else
					stay_time = MIN_CHANNEL_TIME;
#ifdef WH_EZ_SETUP
				if ((IS_EZ_SETUP_ENABLED(wdev)) && ScanType != SCAN_PASSIVE)
					stay_time = FAST_ACTIVE_SCAN_TIME;
#endif
				
			}
			else
			{
#if defined (EZ_NETWORK_MERGE_SUPPORT) && defined (WH_EZ_SETUP)
				if (IS_EZ_SETUP_ENABLED(wdev))
				{
					if (ScanType != SCAN_PASSIVE)
						stay_time = FAST_ACTIVE_SCAN_TIME;
					else
						stay_time = MIN_CHANNEL_TIME;
				}
				else
#endif			
				stay_time = MAX_CHANNEL_TIME;
			}		
#ifdef CONFIG_AP_SUPPORT					
#endif			
		}
		RTMPSetTimer(sc_timer, stay_time);
#ifdef APCLI_SUPPORT
#endif

		if (SCAN_MODE_ACT(ScanType))
		{
#ifdef APCLI_SUPPORT
#ifdef WH_EZ_SETUP
            PAPCLI_STRUCT pApCliEntry = NULL;
            UINT index = 0;
#endif
#endif	
			if (scan_active(pAd, OpMode, ScanType) == FALSE){
#ifdef WH_EZ_SETUP
				if(pApCliTable&&(IS_EZ_SETUP_ENABLED(&pApCliTable->wdev))
					&& (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN)) 
					pAd->Mlme.ApSyncMachine.CurrState = AP_SYNC_IDLE;
#endif
				
				return;
			}

#ifdef APCLI_SUPPORT
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(wdev)) {
                for(index = 0; index < MAX_APCLI_NUM; index++)
                {
                    pApCliEntry = &pAd->ApCfg.ApCliTab[index];
                    if(pApCliEntry->CfgHideSsidLen > 0){
                        FireExtraProbeReq(pAd,  OpMode, ScanType, wdev,
                            pApCliEntry->CfgHideSsid, pApCliEntry->CfgHideSsidLen);
                    }
                }
			}
#endif
#endif			


#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef AP_PARTIAL_SCAN_SUPPORT
			if (pAd->ApCfg.bPartialScanning == TRUE)
			{
				/* Enhance Connectivity & for Hidden Ssid Scanning*/
				CHAR backSsid[MAX_LEN_OF_SSID];
				UCHAR desiredSsidLen, backSsidLen;

				desiredSsidLen= pAd->ApCfg.ApCliTab[0].CfgSsidLen;

				if (desiredSsidLen	> 0)
				{
					//printk("=====================>specific the %s scanning\n", pAd->ApCfg.ApCliTab[0].CfgSsid);
					/* 1. backup the original ScanCtrl */
					backSsidLen = pAd->ScanCtrl.SsidLen;
					NdisCopyMemory(backSsid, pAd->ScanCtrl.Ssid, backSsidLen);
					
					/* 2. fill the desried ssid into SM */
					pAd->ScanCtrl.SsidLen = desiredSsidLen;
					NdisCopyMemory(pAd->ScanCtrl.Ssid, pAd->ApCfg.ApCliTab[0].CfgSsid, desiredSsidLen);

					/* 3. scan action */
					scan_active(pAd, OpMode, ScanType);
			
					/* 4. restore to ScanCtrl */
					pAd->ScanCtrl.SsidLen = backSsidLen;
					NdisCopyMemory(pAd->ScanCtrl.Ssid, backSsid, backSsidLen);
				}
			}
#endif /* AP_PARTIAL_SCAN_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

		}

		/* For SCAN_CISCO_PASSIVE, do nothing and silently wait for beacon or other probe reponse*/
		
#ifdef CONFIG_AP_SUPPORT
		if (OpMode == OPMODE_AP)
			pAd->Mlme.ApSyncMachine.CurrState = AP_SCAN_LISTEN;
#endif /* CONFIG_AP_SUPPORT */
	}
}


BOOLEAN ScanRunning(RTMP_ADAPTER *pAd)
{
	BOOLEAN	rv = FALSE;

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_SCAN_SUPPORT
		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			rv = ((pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) ? TRUE : FALSE);
#endif /* AP_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	return rv;
}

#endif /* SCAN_SUPPORT */

