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
    soft_ap.c
 
    Abstract:
    Access Point specific routines and MAC table maintenance routines
 
    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
    John Chang  08-04-2003    created for 11g soft-AP

 */

#include "rt_config.h"

BOOLEAN ApCheckLongPreambleSTA(
    IN PRTMP_ADAPTER pAd);

char const *pEventText[EVENT_MAX_EVENT_TYPE] = {
	"restart access point",
	"successfully associated",
	"has disassociated",
	"has been aged-out and disassociated" ,    
	"active countermeasures",
	"has disassociated with invalid PSK password"};


UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr)
{
	UCHAR apidx;

	for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
	{	
		if (RTMPEqualMemory(addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN))
			break;
	}

	return apidx;
}


/*
	==========================================================================
	Description:
		Initialize AP specific data especially the NDIS packet pool that's
		used for wireless client bridging.
	==========================================================================
 */
NDIS_STATUS APInitialize(RTMP_ADAPTER *pAd)
{
	NDIS_STATUS     Status = NDIS_STATUS_SUCCESS;
	INT				i;			

	DBGPRINT(RT_DEBUG_TRACE, ("---> APInitialize\n"));

	/* Init Group key update timer, and countermeasures timer */
	for (i = 0; i < MAX_MBSSID_NUM(pAd); i++)
		RTMPInitTimer(pAd, &pAd->ApCfg.MBSSID[i].REKEYTimer, GET_TIMER_FUNCTION(GREKEYPeriodicExec), pAd,  TRUE); 
	
	RTMPInitTimer(pAd, &pAd->ApCfg.CounterMeasureTimer, GET_TIMER_FUNCTION(CMTimerExec), pAd, FALSE);

#ifdef IDS_SUPPORT
	/* Init intrusion detection timer */
	RTMPInitTimer(pAd, &pAd->ApCfg.IDSTimer, GET_TIMER_FUNCTION(RTMPIdsPeriodicExec), pAd, FALSE);
	pAd->ApCfg.IDSTimerRunning = FALSE;
#endif /* IDS_SUPPORT */

#ifdef WAPI_SUPPORT
	/* Init WAPI rekey timer */
	RTMPInitWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

#ifdef WDS_SUPPORT
	NdisAllocateSpinLock(pAd, &pAd->WdsTabLock);
#endif /* WDS_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
	MulticastFilterTableInit(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DROP_MASK_SUPPORT
	NdisAllocateSpinLock(pAd, &pAd->drop_mask_lock);
	asic_drop_mask_reset(pAd);
#endif /* DROP_MASK_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	initList(&pAd->DMSEntryList);
#endif /* DOT11V_WNM_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- APInitialize\n"));
	return Status;
}

/*
	==========================================================================
	Description:
		Shutdown AP and free AP specific resources
	==========================================================================
 */
VOID APShutdown(RTMP_ADAPTER *pAd)
{
	DBGPRINT(RT_DEBUG_TRACE, ("---> APShutdown\n"));
/*	if (pAd->OpMode == OPMODE_AP) */
#ifdef RTMP_MAC_PCI
		APStop(pAd);
#endif /* RTMP_MAC_PCI */

#ifdef RT65xx
	if (!IS_MT76x0E(pAd) && !IS_MT76x2E(pAd))
#endif /* RT65xx */
		MlmeRadioOff(pAd);

#ifdef IGMP_SNOOP_SUPPORT
	MultiCastFilterTableReset(pAd, &pAd->pMulticastFilterTable);
#endif /* IGMP_SNOOP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */

	NdisFreeSpinLock(&pAd->MacTabLock);

#ifdef WDS_SUPPORT
	NdisFreeSpinLock(&pAd->WdsTabLock);
#endif /* WDS_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("<--- APShutdown\n"));
}

/*
	==========================================================================
	Description:
		Start AP service. If any vital AP parameter is changed, a STOP-START
		sequence is required to disassociate all STAs.

	IRQL = DISPATCH_LEVEL.(from SetInformationHandler)
	IRQL = PASSIVE_LEVEL. (from InitializeHandler)  

	Note:
		Can't call NdisMIndicateStatus on this routine.

		RT61 is a serialized driver on Win2KXP and is a deserialized on Win9X
		Serialized callers of NdisMIndicateStatus must run at IRQL = DISPATCH_LEVEL.

	==========================================================================
 */
VOID APStartUp(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	BOOLEAN bWmmCapable = FALSE;
	UCHAR idx;
	BOOLEAN TxPreamble, SpectrumMgmt = FALSE;
	UCHAR phy_mode = pAd->CommonCfg.cfg_wmode;
#ifdef DOT1X_SUPPORT
	/* BOOLEAN bDot1xReload = FALSE; */
#endif /* DOT1X_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("===> APStartUp\n"));

#ifdef INF_AMAZON_SE
	for (i=0;i<NUM_OF_TX_RING;i++)
		pAd->BulkOutDataSizeLimit[i]=24576;
#endif /* INF_AMAZON_SE */
		
	AsicDisableSync(pAd);

	TxPreamble = (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong ? 0 : 1);

#ifdef A_BAND_SUPPORT
	/* Decide the Capability information field */
	/* In IEEE Std 802.1h-2003, the spectrum management bit is enabled in the 5 GHz band */
	if ((pAd->CommonCfg.Channel > 14) && pAd->CommonCfg.bIEEE80211H == TRUE)
		SpectrumMgmt = TRUE;
#endif /* A_BAND_SUPPORT */	
			
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
		struct wifi_dev *wdev = &pMbss->wdev;

		wdev->channel = pAd->CommonCfg.Channel;

		if ((pMbss->SsidLen <= 0) || (pMbss->SsidLen > MAX_LEN_OF_SSID))
		{
			NdisMoveMemory(pMbss->Ssid, "HT_AP", 5);
			pMbss->Ssid[5] = '0' + idx;
			pMbss->SsidLen = 6;			
		}

#ifdef SPECIFIC_TX_POWER_SUPPORT
		pMbss->TxPwrAdj=0;
#endif /* SPECIFIC_TX_POWER_SUPPORT */

		/* re-copy the MAC to virtual interface to avoid these MAC = all zero,
		   when re-open the ra0,
		   i.e. ifconfig ra0 down, ifconfig ra0 up, ifconfig ra0 down, ifconfig up ... */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
		if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
		{
			UCHAR MacMask = 0;

			if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
				MacMask = 0xFE;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
				MacMask = 0xFC;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
				MacMask = 0xF8;
			else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
				MacMask = 0xF0;

			if (idx > 0) 
			{
				wdev->if_addr[0] |= 0x2;
				if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
				{
					/*
						Refer to HW definition - 
						Bit1 of MAC address Byte0 is local administration bit 
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					 */
					wdev->if_addr[0] &= ((MacMask << 2) + 3);
					wdev->if_addr[0] |= ((idx - 1) << 2);
				}
#ifdef ENHANCE_NEW_MBSSID_MODE
				else
				{
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] |= (idx - 1);
				}
#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		}
		else
			wdev->if_addr[5] += idx;

		if (wdev->if_dev != NULL)
		{
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev), 
								wdev->if_addr, MAC_ADDR_LEN);
		}
		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
		wdev->wdev_type = WDEV_TYPE_AP;
		wdev->tx_pkt_allowed = ApAllowToSendPacket;

		if (pMbss->wdev.bWmmCapable)
	        	bWmmCapable = TRUE;

#ifdef AIRPLAY_SUPPORT
        if (AIRPLAY_ON(pAd))
                pMbss->bBcnSntReq = TRUE;
        else
                pMbss->bBcnSntReq = FALSE;
#endif /* AIRPLAY_SUPPORT */
		
		pMbss->CapabilityInfo = CAP_GENERATE(1, 0, (wdev->WepStatus != Ndis802_11EncryptionDisabled),
											TxPreamble, pAd->CommonCfg.bUseShortSlotTime,
											SpectrumMgmt);

#ifdef DOT11K_RRM_SUPPORT
		if (IS_RRM_ENABLE(pAd, idx))
			pMbss->CapabilityInfo |= RRM_CAP_BIT;
#endif /* DOT11K_RRM_SUPPORT */
		
		if (bWmmCapable == TRUE)
		{
			/*
				In WMM spec v1.1, A WMM-only AP or STA does not set the "QoS"
				bit in the capability field of association, beacon and probe
				management frames.
			*/
/*			pMbss->CapabilityInfo |= 0x0200; */
		}

#ifdef UAPSD_SUPPORT
		if (pMbss->UapsdInfo.bAPSDCapable == TRUE)
		{
			/*
				QAPs set the APSD subfield to 1 within the Capability
				Information field when the MIB attribute
				dot11APSDOptionImplemented is true and set it to 0 otherwise.
				STAs always set this subfield to 0.
			*/
			pMbss->CapabilityInfo |= 0x0800;
		}
#endif /* UAPSD_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
		/* 
		   IEEE 802.11W/P.10 -
		   A STA that has associated with Management Frame Protection enabled 
		   shall not use pairwise cipher suite selectors WEP-40, WEP-104, 
		   TKIP, or "Use Group cipher suite". 

		   IEEE 802.11W/P.3 -
		   IEEE Std 802.11 provides one security protocol, CCMP, for protection
		   of unicast Robust Management frames.
		 */
		pMbss->PmfCfg.MFPC = FALSE;
		pMbss->PmfCfg.MFPR = FALSE;
                pMbss->PmfCfg.PMFSHA256 = FALSE;
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA2 || 
			 wdev->AuthMode == Ndis802_11AuthModeWPA2PSK) &&
			(wdev->WepStatus == Ndis802_11AESEnable) &&
			(pMbss->PmfCfg.Desired_MFPC))
		{
			pMbss->PmfCfg.MFPC = TRUE;
			pMbss->PmfCfg.MFPR = pMbss->PmfCfg.Desired_MFPR;

			/* IGTK default key index as 4 */
			pMbss->PmfCfg.IGTK_KeyIdx = 4;

			/* Derive IGTK */
			PMF_DeriveIGTK(pAd, &pMbss->PmfCfg.IGTK[0][0]);

                        if ((pMbss->PmfCfg.Desired_PMFSHA256) || (pMbss->PmfCfg.MFPR))
                                pMbss->PmfCfg.PMFSHA256 = TRUE;
                                
		} else if (pMbss->PmfCfg.Desired_MFPC) {
                        DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __FUNCTION__));
		}

                DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: apidx=%d, MFPC=%d, MFPR=%d, SHA256=%d\n",
							__FUNCTION__, idx, pMbss->PmfCfg.MFPC,
							pMbss->PmfCfg.MFPR, pMbss->PmfCfg.PMFSHA256));
#endif /* DOT11W_PMF_SUPPORT */
		
		/* decide the mixed WPA cipher combination */
		if (wdev->WepStatus == Ndis802_11TKIPAESMix)
		{
			switch ((UCHAR)wdev->AuthMode)
			{
				/* WPA mode */
				case Ndis802_11AuthModeWPA:
				case Ndis802_11AuthModeWPAPSK:
					wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_NONE;
					break;	

				/* WPA2 mode */
				case Ndis802_11AuthModeWPA2:
				case Ndis802_11AuthModeWPA2PSK:
					wdev->WpaMixPairCipher = WPA_NONE_WPA2_TKIPAES;
					break;

				/* WPA and WPA2 both mode */
				case Ndis802_11AuthModeWPA1WPA2:
				case Ndis802_11AuthModeWPA1PSKWPA2PSK:	

					/* In WPA-WPA2 and TKIP-AES mixed mode, it shall use the maximum */
					/* cipher capability unless users assign the desired setting. */
					if (wdev->WpaMixPairCipher == MIX_CIPHER_NOTUSE || 
						wdev->WpaMixPairCipher == WPA_TKIPAES_WPA2_NONE || 
						wdev->WpaMixPairCipher == WPA_NONE_WPA2_TKIPAES)
						wdev->WpaMixPairCipher = WPA_TKIPAES_WPA2_TKIPAES;										
					break;				
			}
		}
		else
			wdev->WpaMixPairCipher = MIX_CIPHER_NOTUSE;

		if (wdev->WepStatus == Ndis802_11Encryption2Enabled ||
			wdev->WepStatus == Ndis802_11Encryption3Enabled ||
			wdev->WepStatus == Ndis802_11Encryption4Enabled)
		{
			RT_CfgSetWPAPSKKey(pAd, pMbss->WPAKeyString,
								strlen(pMbss->WPAKeyString),
								(PUCHAR)pAd->ApCfg.MBSSID[idx].Ssid,
								pAd->ApCfg.MBSSID[idx].SsidLen,
								pAd->ApCfg.MBSSID[idx].PMK);
		}
		
		/* Generate the corresponding RSNIE */
		RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, idx);

#ifdef WSC_V2_SUPPORT
		if (pMbss->WscControl.WscV2Info.bEnableWpsV2)
		{
			/* WPS V2 doesn't support WEP and WPA/WPAPSK-TKIP. */
			if ((wdev->WepStatus == Ndis802_11WEPEnabled) || 
				(wdev->WepStatus == Ndis802_11TKIPEnable) ||
				(pMbss->bHideSsid))
				WscOnOff(pAd, idx, TRUE);
			else
				WscOnOff(pAd, idx, FALSE);
		}
#endif /* WSC_V2_SUPPORT */
#ifdef BAND_STEERING
		if (pAd->ApCfg.BandSteering && idx == BSS0)
			BndStrg_Init(pAd);
#endif /* BAND_STEERING */
	}

#ifdef DOT11_N_SUPPORT
	if (phy_mode != pAd->CommonCfg.PhyMode)
		RTMPSetPhyMode(pAd, phy_mode);

	SetCommonHT(pAd);
#endif /* DOT11_N_SUPPORT */
	
	COPY_MAC_ADDR(pAd->CommonCfg.Bssid, pAd->CurrentAddress);

	/* Select DAC according to HT or Legacy, write to BBP R1(bit4:3) */
	/* In HT mode and two stream mode, both DACs are selected. */
	/* In legacy mode or one stream mode, DAC-0 is selected. */
	{
#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) && (pAd->Antenna.field.TxPath == 2))
			bbp_set_txdac(pAd, 2);
		else
#endif /* DOT11_N_SUPPORT */
			bbp_set_txdac(pAd, 0);
	}

	/* Receiver Antenna selection */
	bbp_set_rxpath(pAd, pAd->Antenna.field.RxPath);

	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode) || bWmmCapable)
	{
		/* EDCA parameters used for AP's own transmission */
		if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
			set_default_ap_edca_param(pAd);

		/* EDCA parameters to be annouced in outgoing BEACON, used by WMM STA */
		if (pAd->ApCfg.BssEdcaParm.bValid == FALSE)
			set_default_sta_edca_param(pAd);

		AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);
	}
	else
		AsicSetEdcaParm(pAd, NULL);

#ifdef DOT11_N_SUPPORT
	if (!WMODE_CAP_N(pAd->CommonCfg.PhyMode))
		pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth = BW_20; /* Patch UI */

	AsicSetRDG(pAd, pAd->CommonCfg.bRdg);

	AsicSetRalinkBurstMode(pAd, pAd->CommonCfg.bRalinkBurstMode);
#endif /* DOT11_N_SUPPORT */

#ifdef TMP_PROGRESS_TEST
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#endif /* TMP_PROGRESS_TEST */
	AsicSetBssid(pAd, pAd->CurrentAddress); 
	mgmt_tb_set_mcast_entry(pAd);


	/*@!Release
		Reset whole WCID table
		
		In AP mode,  First WCID Table in ASIC will never be used.
		To prevent it's 0xff-ff-ff-ff-ff-ff, Write 0 here.

		p.s ASIC use all 0xff as termination of WCID table search.
	*/
	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset WCID Table\n", __FUNCTION__));
		AsicDelWcidTab(pAd, WCID_ALL);

#ifdef FIFO_EXT_SUPPORT
	FifoExtTableInit(pAd);
	AsicFifoExtSet(pAd);
#endif /* FIFO_EXT_SUPPORT */

	pAd->MacTab.MsduLifeTime = 5; /* default 5 seconds */
	
	pAd->MacTab.Content[0].Addr[0] = 0x01;
	pAd->MacTab.Content[0].HTPhyMode.field.MODE = MODE_OFDM;
	pAd->MacTab.Content[0].HTPhyMode.field.MCS = 3;
	pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	
	AsicBBPAdjust(pAd);

	/* Clear BG-Protection flag */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
#ifdef DOT11_VHT_AC
	if (pAd->CommonCfg.BBPCurrentBW == BW_80)
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.vht_cent_ch;
	else
#endif /* DOT11_VHT_AC */
		pAd->hw_cfg.cent_ch = pAd->CommonCfg.CentralChannel;
	AsicSwitchChannel(pAd, pAd->hw_cfg.cent_ch, FALSE);
	AsicLockChannel(pAd, pAd->hw_cfg.cent_ch);

#ifdef DOT11_VHT_AC
//+++Add by shiang for debug
DBGPRINT(RT_DEBUG_OFF, ("%s(): AP Set CentralFreq at %d(Prim=%d, HT-CentCh=%d, VHT-CentCh=%d, BBP_BW=%d)\n",
						__FUNCTION__, pAd->hw_cfg.cent_ch, pAd->CommonCfg.Channel, 
						pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch,
						pAd->CommonCfg.BBPCurrentBW));
//---Add by shiang for debug
#endif /* DOT11_VHT_AC */

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (pAd->ApCfg.bGreenAPEnable == TRUE)
	{
		RTMP_CHIP_ENABLE_AP_MIMOPS(pAd,TRUE);
		pAd->ApCfg.GreenAPLevel=GREENAP_WITHOUT_ANY_STAS_CONNECT;
	}
#endif /* GREENAP_SUPPORT */
#endif /* DOT11_N_SUPPORT */

	MlmeSetTxPreamble(pAd, (USHORT)pAd->CommonCfg.TxPreamble);	
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		MlmeUpdateTxRates(pAd, FALSE, idx);
#ifdef DOT11_N_SUPPORT
		if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
			MlmeUpdateHtTxRates(pAd, idx);
#endif /* DOT11_N_SUPPORT */
	}
	
	/* Set the RadarDetect Mode as Normal, bc the APUpdateAllBeaconFram() will refer this parameter. */
	RadarStateCheck(pAd);

#ifdef CUSTOMER_DCC_FEATURE
	pAd->CommonCfg.channelSwitch.CHSWMode = NORMAL_MODE;
#endif
	/* Disable Protection first. */
	AsicUpdateProtect(pAd, 0, (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);
	
	APUpdateCapabilityAndErpIe(pAd);
#ifdef DOT11_N_SUPPORT
	APUpdateOperationMode(pAd);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_N_SUPPORT
#endif /* DOT11_N_SUPPORT */

#ifdef LED_CONTROL_SUPPORT
	/* Set LED */
	RTMPSetLED(pAd, LED_LINK_UP);
#endif /* LED_CONTROL_SUPPORT */

	/* Initialize security variable per entry, 
		1. 	pairwise key table, re-set all WCID entry as NO-security mode.
		2.	access control port status
	*/
	DBGPRINT(RT_DEBUG_TRACE, ("%s():Reset Sec Table\n", __FUNCTION__));
	for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		pAd->MacTab.Content[i].PortSecured  = WPA_802_1X_PORT_NOT_SECURED;
		AsicRemovePairwiseKeyEntry(pAd, (UCHAR)i);
	}
		
	/* Init Security variables */
	for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
	{
		USHORT Wcid = 0;
		MULTISSID_STRUCT *pMbss = &pAd->ApCfg.MBSSID[idx];
		struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[idx].wdev;

		wdev->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
		if (IS_WPA_CAPABILITY(wdev->AuthMode))
			wdev->DefaultKeyId = 1;

		/* Get a specific WCID to record this MBSS key attribute */
		GET_GroupKey_WCID(pAd, Wcid, idx);

		/* When WEP, TKIP or AES is enabled, set group key info to Asic */
		if (wdev->WepStatus == Ndis802_11WEPEnabled)
		{
			UCHAR CipherAlg, key_idx;

			for (key_idx=0; key_idx < SHARE_KEY_NUM; key_idx++)
			{
				CipherAlg = pAd->SharedKey[idx][key_idx].CipherAlg;

				if (pAd->SharedKey[idx][key_idx].KeyLen > 0)
				{
					/* Set key material to Asic */
					AsicAddSharedKeyEntry(pAd, idx, key_idx, &pAd->SharedKey[idx][key_idx]);	
		
					if (key_idx == wdev->DefaultKeyId)
					{
						/* Generate 3-bytes IV randomly for software encryption using */						
					    	for(i = 0; i < LEN_WEP_TSC; i++)
							pAd->SharedKey[idx][key_idx].TxTsc[i] = RandomByte(pAd);   
											
						/* Update WCID attribute table and IVEIV table */
						RTMPSetWcidSecurityInfo(pAd, 
												idx, 
												key_idx, 												
												CipherAlg,
												Wcid, 
												SHAREDKEYTABLE);					
					}
				}
			}
    	}
		else if ((wdev->WepStatus == Ndis802_11TKIPEnable) ||
				 (wdev->WepStatus == Ndis802_11AESEnable) ||
				 (wdev->WepStatus == Ndis802_11TKIPAESMix))
		{
			/* Generate GMK and GNonce randomly per MBSS */
			GenRandom(pAd, wdev->bssid, pMbss->GMK);
			GenRandom(pAd, wdev->bssid, pMbss->GNonce);		

			/* Derive GTK per BSSID */
			WpaDeriveGTK(pMbss->GMK, 
						(UCHAR*)pMbss->GNonce, 
						wdev->bssid, 
						pMbss->GTK, 
						LEN_TKIP_GTK);

			/* Install Shared key */
			WPAInstallSharedKey(pAd, 
								wdev->GroupKeyWepStatus, 
								idx, 
								wdev->DefaultKeyId, 
								Wcid,
								TRUE,
								pMbss->GTK,
								LEN_TKIP_GTK);
		
		}
#ifdef WAPI_SUPPORT
		else if (pMbss->wdev.WepStatus == Ndis802_11EncryptionSMS4Enabled)
		{	
			INT	cnt;
		
			/* Initial the related variables */
			pMbss->wdev.DefaultKeyId = 0;
			NdisMoveMemory(pMbss->key_announce_flag, AE_BCAST_PN, LEN_WAPI_TSC);
			if (IS_HW_WAPI_SUPPORT(pAd))
				pMbss->sw_wpi_encrypt = FALSE;					
			else
				pMbss->sw_wpi_encrypt = TRUE;

			/* Generate NMK randomly */
			for (cnt = 0; cnt < LEN_WAPI_NMK; cnt++)
				pMbss->NMK[cnt] = RandomByte(pAd);
			
			/* Count GTK for this BSSID */
			RTMPDeriveWapiGTK(pMbss->NMK, pMbss->GTK);

			/* Install Shared key */
			WAPIInstallSharedKey(pAd, 
								 wdev->GroupKeyWepStatus, 
								 idx, 
								 wdev->DefaultKeyId, 
								 Wcid,
								 pMbss->GTK);
			
		}
#endif /* WAPI_SUPPORT */

#ifdef DOT1X_SUPPORT
		/* Send singal to daemon to indicate driver had restarted */
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPA2)
        		|| (wdev->AuthMode == Ndis802_11AuthModeWPA1WPA2) || (wdev->IEEE8021X == TRUE))
		{
			;/*bDot1xReload = TRUE; */
    	}
#endif /* DOT1X_SUPPORT */

		DBGPRINT(RT_DEBUG_TRACE, ("### BSS(%d) AuthMode(%d)=%s, WepStatus(%d)=%s, AccessControlList.Policy=%ld\n",
					idx, wdev->AuthMode, GetAuthMode(wdev->AuthMode), 
					wdev->WepStatus, GetEncryptType(wdev->WepStatus),
					pMbss->AccessControlList.Policy));
	}


#ifdef PIGGYBACK_SUPPORT
	RTMPSetPiggyBack(pAd, pAd->CommonCfg.bPiggyBackCapable);
#endif /* PIGGYBACK_SUPPORT */

	ApLogEvent(pAd, pAd->CurrentAddress, EVENT_RESET_ACCESS_POINT);
	pAd->Mlme.PeriodicRound = 0;
	pAd->Mlme.OneSecPeriodicRound = 0;

	OPSTATUS_SET_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);

	RTMP_IndicateMediaState(pAd, NdisMediaStateConnected);


	/*
		NOTE!!!:
			All timer setting shall be set after following flag be cleared
				fRTMP_ADAPTER_HALT_IN_PROGRESS
	*/
	/* 
		Note : Owing to the flags setting too late to USB In-band commnad cannnot receive rsp
			   For this test,change to up setting
			   If test have no side effect , we will change this
	*/
#ifndef TMP_PROGRESS_TEST	
	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
#endif /* TMP_PROGRESS_TEST */


#ifdef MT76x2
	if (IS_MT76x2(pAd))
	{
#ifdef TXBF_SUPPORT
        if (pAd->hw_cfg.cent_ch > 14) 
           rtmp_ate_txbf_fix_tank_code(pAd, pAd->hw_cfg.cent_ch, 0);  // load tank code from efuse, iBF only for A band
#endif /*TXBF_SUPPORT*/
		mt76x2_calibration(pAd, pAd->hw_cfg.cent_ch);
    }
#endif /* MT76x2 */

	/* start sending BEACON out */
	APMakeAllBssBeacon(pAd);
	APUpdateAllBeaconFrame(pAd);

#ifdef DFS_SUPPORT
	if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE))
		NewRadarDetectionStart(pAd);
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		CarrierDetectionStart(pAd);
#endif /* CARRIER_DETECTION_SUPPORT */

	if (pAd->Dot11_H.RDMode == RD_NORMAL_MODE)
		AsicEnableBssSync(pAd);

	/* Pre-tbtt interrupt setting. */
	AsicSetPreTbttInt(pAd, TRUE);

#ifdef RTMP_MAC_PCI
	RTMP_MAC_RX_ENABLE(pAd);
#endif /*RTMP_MAC_PCI*/

#ifdef WAPI_SUPPORT
	RTMPStartWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */

	/*
		Set group re-key timer if necessary. 
		It must be processed after clear flag "fRTMP_ADAPTER_HALT_IN_PROGRESS"
	*/
	WPA_APSetGroupRekeyAction(pAd);

#ifdef WDS_SUPPORT
	/* Prepare WEP key */
	WdsPrepareWepKeyFromMainBss(pAd);

	/* Add wds key infomation to ASIC */
	AsicUpdateWdsRxWCIDTable(pAd);
#endif /* WDS_SUPPORT */

#ifdef IDS_SUPPORT
	/* Start IDS timer */
	if (pAd->ApCfg.IdsEnable)
	{
#ifdef SYSTEM_LOG_SUPPORT	
		if (pAd->CommonCfg.bWirelessEvent == FALSE)
			DBGPRINT(RT_DEBUG_WARN, ("!!! WARNING !!! The WirelessEvent parameter doesn't be enabled \n"));
#endif /* SYSTEM_LOG_SUPPORT */
		
		RTMPIdsStart(pAd);
	}
#endif /* IDS_SUPPORT */



#ifdef DOT11R_FT_SUPPORT
	FT_Init(pAd);
#endif /* DOT11R_FT_SUPPORT */



#ifdef ED_MONITOR
{
	BOOLEAN bEdcca = FALSE;

	bEdcca = GetEDCCASupport(pAd);

	if (bEdcca)
	{
		ed_monitor_init(pAd);
	}
	else
		ed_monitor_exit(pAd);
}
#endif /* ED_MONITOR */
#ifdef CUSTOMER_DCC_FEATURE
	pAd->CommonCfg.NewExtChanOffset.NewExtChanOffset = pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
#endif

	RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS);

	DBGPRINT(RT_DEBUG_TRACE, ("<=== APStartUp\n"));
}

/*
	==========================================================================
	Description:
		disassociate all STAs and stop AP service.
	Note:
	==========================================================================
 */
VOID APStop(
	IN PRTMP_ADAPTER pAd) 
{
	BOOLEAN Cancelled;
	UINT32 Value;
	INT idx;
	MULTISSID_STRUCT *pMbss;
			
	DBGPRINT(RT_DEBUG_TRACE, ("!!! APStop !!!\n"));

#ifdef ED_MONITOR
	if (pAd->ed_chk)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("@@@ %s: go to ed_monitor_exit()!!\n", __FUNCTION__));		
		ed_monitor_exit(pAd);
	}
#endif /* ED_MONITOR */

#ifdef DFS_SUPPORT
		NewRadarDetectionStop(pAd);
#endif /* DFS_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
		if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
		{
			/* make sure CarrierDetect wont send CTS */
			CarrierDetectionStop(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#ifdef WDS_SUPPORT
	WdsDown(pAd);
#endif /* WDS_SUPPORT */

#ifdef APCLI_SUPPORT
	ApCliIfDown(pAd);
#endif /* APCLI_SUPPORT */

#ifdef RTMP_MAC_PCI

	RTMP_MAC_RX_DISABLE(pAd);
#endif /*RTMP_MAC_PCI*/
	MacTableReset(pAd);

	RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);

	/* Disable pre-tbtt interrupt */
	RTMP_IO_READ32(pAd, INT_TIMER_EN, &Value);
	Value &=0xe;
	RTMP_IO_WRITE32(pAd, INT_TIMER_EN, Value);
	/* Disable piggyback */
	RTMPSetPiggyBack(pAd, FALSE);

   	AsicUpdateProtect(pAd, 0,  (ALLN_SETPROTECT|CCKSETPROTECT|OFDMSETPROTECT), TRUE, FALSE);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		/*RTMP_ASIC_INTERRUPT_DISABLE(pAd); */
		AsicDisableSync(pAd);

#ifdef LED_CONTROL_SUPPORT
		/* Set LED */
		RTMPSetLED(pAd, LED_LINK_DOWN);
#endif /* LED_CONTROL_SUPPORT */
	}



	for (idx = 0; idx < MAX_MBSSID_NUM(pAd); idx++)
	{
		pMbss = &pAd->ApCfg.MBSSID[idx];
		if (pMbss->REKEYTimerRunning == TRUE)
		{
			RTMPCancelTimer(&pMbss->REKEYTimer, &Cancelled);
			pMbss->REKEYTimerRunning = FALSE;
		}
	}

	if (pAd->ApCfg.CMTimerRunning == TRUE)
	{
		RTMPCancelTimer(&pAd->ApCfg.CounterMeasureTimer, &Cancelled);
		pAd->ApCfg.CMTimerRunning = FALSE;
	}
	
#ifdef WAPI_SUPPORT
	RTMPCancelWapiRekeyTimerAction(pAd, NULL);
#endif /* WAPI_SUPPORT */
	
	/* */
	/* Cancel the Timer, to make sure the timer was not queued. */
	/* */
	OPSTATUS_CLEAR_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED);
	RTMP_IndicateMediaState(pAd, NdisMediaStateDisconnected);

#ifdef IDS_SUPPORT
	/* if necessary, cancel IDS timer */
	RTMPIdsStop(pAd);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
	FT_Release(pAd);
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
	DMSTable_Release(pAd);
#endif /* DOT11V_WNM_SUPPORT */

}

/*
	==========================================================================
	Description:
		This routine is used to clean up a specified power-saving queue. It's
		used whenever a wireless client is deleted.
	==========================================================================
 */
VOID APCleanupPsQueue(
	IN  PRTMP_ADAPTER   pAd,
	IN  PQUEUE_HEADER   pQueue)
{
	PQUEUE_ENTRY pEntry;
	PNDIS_PACKET pPacket;

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): (0x%08lx)...\n", __FUNCTION__, (ULONG)pQueue));

	while (pQueue->Head)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():%u...\n", __FUNCTION__, pQueue->Number));

		pEntry = RemoveHeadQueue(pQueue);
		/*pPacket = CONTAINING_RECORD(pEntry, NDIS_PACKET, MiniportReservedEx); */
		pPacket = QUEUE_ENTRY_TO_PACKET(pEntry);
		RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
	}
}


#ifdef APCLI_SUPPORT
#ifdef TRAFFIC_BASED_TXOP
static VOID CheckApEntryInTraffic(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	UINT32 TxTotalByteCnt = 0;
	UINT32 RxTotalByteCnt = 0;

	if ((IS_ENTRY_APCLI(pEntry) || IS_ENTRY_CLIENT(pEntry))
		&& (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		TxTotalByteCnt = pEntry->OneSecTxBytes;
		RxTotalByteCnt = pEntry->OneSecRxBytes;

		DBGPRINT(RT_DEBUG_TRACE,("WCID%d, %dM, TxBytes:%d, RxBytes:%d\n",
				pEntry->wcid,
				(((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20), 
				TxTotalByteCnt, 
				RxTotalByteCnt));

		if ((TxTotalByteCnt == 0) || (RxTotalByteCnt == 0)) 
		{
		} 
		else if ((((TxTotalByteCnt + RxTotalByteCnt) << 3) >> 20) > pAd->CommonCfg.ManualTxopThreshold) 
		{
			if (TxTotalByteCnt > RxTotalByteCnt) 
			{
				if ((TxTotalByteCnt/RxTotalByteCnt) >= pAd->CommonCfg.ManualTxopUpBound)
				{
					if (IS_ENTRY_CLIENT(pEntry))
						pAd->StaTxopAbledCnt++;
					else
						pAd->ApClientTxopAbledCnt++;

					DBGPRINT(RT_DEBUG_TRACE,("WCID%d, StaTxopAbledCnt:%d, ApClientTxopAbledCnt:%d\n",
						pEntry->wcid,
						pAd->StaTxopAbledCnt, 
						pAd->ApClientTxopAbledCnt));
				}
			}
			else
			{
				if ((RxTotalByteCnt/TxTotalByteCnt) <= pAd->CommonCfg.ManualTxopLowBound)
				{
				 	if (IS_ENTRY_CLIENT(pEntry))
               			pAd->StaTxopAbledCnt++;
                	else
                    	pAd->ApClientTxopAbledCnt++;					
				}
			}
		}
	}

	pEntry->OneSecTxBytes = 0;
	pEntry->OneSecRxBytes = 0;

}
#endif /* TRAFFIC_BASED_TXOP */
#endif /* APCLI_SUPPORT */


/*
	==========================================================================
	Description:
		This routine is called by APMlmePeriodicExec() every second to check if
		1. any associated client in PSM. If yes, then TX MCAST/BCAST should be
		   out in DTIM only
		2. any client being idle for too long and should be aged-out from MAC table
		3. garbage collect PSQ
	==========================================================================
*/
VOID MacTableMaintenance(RTMP_ADAPTER *pAd)
{
	int i, startWcid;
#ifdef DOT11_N_SUPPORT
	ULONG MinimumAMPDUSize = pAd->CommonCfg.DesiredHtPhy.MaxRAmpduFactor; /*Default set minimum AMPDU Size to 2, i.e. 32K */
	BOOLEAN	bRdgActive;
	BOOLEAN bRalinkBurstMode;
#endif /* DOT11_N_SUPPORT */
#ifdef RTMP_MAC_PCI
	unsigned long	IrqFlags;
#endif /* RTMP_MAC_PCI */
	UINT fAnyStationPortSecured[HW_BEACON_MAX_NUM];
 	UINT bss_index;
	MAC_TABLE *pMacTable;
#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
	int lastClient=0;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */
	MULTISSID_STRUCT *pMbss;
#ifdef WFA_VHT_PF
	RSSI_SAMPLE *worst_rssi = NULL;
	int worst_rssi_sta_idx = 0;
#endif /* WFA_VHT_PF */

#ifdef ED_MONITOR
	INT total_sta = 0;
#endif /* ED_MONITOR */

	NdisZeroMemory(fAnyStationPortSecured, sizeof(fAnyStationPortSecured));

	pMacTable = &pAd->MacTab;
	pMacTable->fAnyStationInPsm = FALSE;
	pMacTable->fAnyStationBadAtheros = FALSE;
	pMacTable->fAnyTxOPForceDisable = FALSE;
	pMacTable->fAllStationAsRalink = TRUE;
#ifdef DOT11_N_SUPPORT
	pMacTable->fAnyStationNonGF = FALSE;
	pMacTable->fAnyStation20Only = FALSE;
	pMacTable->fAnyStationIsLegacy = FALSE;
	pMacTable->fAnyStationMIMOPSDynamic = FALSE;
#ifdef GREENAP_SUPPORT
	/*Support Green AP */
	pMacTable->fAnyStationIsHT=FALSE;
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
	pMacTable->fAnyStaFortyIntolerant = FALSE;
#endif /* DOT11N_DRAFT3 */
	pMacTable->fAllStationGainGoodMCS = TRUE;
#endif /* DOT11_N_SUPPORT */

#ifdef WAPI_SUPPORT
	pMacTable->fAnyWapiStation = FALSE;
#endif /* WAPI_SUPPORT */

	startWcid = 1;
	
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	/* Skip the Infra Side */
	startWcid = 2;
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */

#ifdef APCLI_SUPPORT
#ifdef TRAFFIC_BASED_TXOP
	pAd->StaTxopAbledCnt = 0;
	pAd->ApClientTxopAbledCnt = 0;
#endif /* TRAFFIC_BASED_TXOP */
#endif /* APCLI_SUPPORT */

	for (i = startWcid; i < MAX_LEN_OF_MAC_TABLE; i++) 
	{
		MAC_TABLE_ENTRY *pEntry = &pMacTable->Content[i];
		BOOLEAN bDisconnectSta = FALSE;
#ifdef APCLI_SUPPORT
#ifdef TRAFFIC_BASED_TXOP	
		CheckApEntryInTraffic(pAd, pEntry);
#endif /* TRAFFIC_BASED_TXOP */

		if(IS_ENTRY_APCLI(pEntry) && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED))
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pEntry->bReptCli)
			{
				pEntry->ReptCliIdleCount++;

				if ((pEntry->bReptEthCli) && (pEntry->ReptCliIdleCount >= MAC_TABLE_AGEOUT_TIME))
				{
#ifdef DOT11_N_SUPPORT
					/* free resources of BA*/
					BASessionTearDownALL(pAd, pEntry->wcid);
#endif /* DOT11_N_SUPPORT */
					RTMPRemoveRepeaterDisconnectEntry(pAd, pEntry->wdev_idx, pEntry->MatchReptCliIdx);
					RTMPRemoveRepeaterEntry(pAd, pEntry->wdev_idx, pEntry->MatchReptCliIdx);
					continue;
				}
			}
#endif /* MAC_REPEATER_SUPPORT */

			if ((pAd->Mlme.OneSecPeriodicRound % 10) == 8)
			{
				/* use Null or QoS Null to detect the ACTIVE station*/
				BOOLEAN ApclibQosNull = FALSE;
		
				if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
					ApclibQosNull = TRUE;
				
			       ApCliRTMPSendNullFrame(pAd,pEntry->CurrTxRate, ApclibQosNull, pEntry, PWR_ACTIVE);

				continue;
			}
		}
#endif /* APCLI_SUPPORT */

#ifdef WDS_SUPPORT
		if (IS_ENTRY_WDS(pEntry))
		{
			if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
				pMacTable->fAllStationAsRalink = FALSE;	

			if (pAd->CommonCfg.bRdg && pMacTable->fAllStationAsRalink)
			{
				bRdgActive = TRUE;
			}
			else
			{
				bRdgActive = FALSE;
			}

			if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
				AsicSetRDG(pAd, bRdgActive);

			continue;
		}
#endif /* WDS_SUPPORT */

#ifdef MWDS
		if (!IS_ENTRY_NONE(pEntry))
		{
			if (pEntry->bEnableMWDS == TRUE)
				pMacTable->fAllStationAsRalink = FALSE;
		}
#endif /* MWDS */

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->NoDataIdleCount == 0)
			pEntry->StationKeepAliveCount = 0;

		pEntry->NoDataIdleCount ++;  
		pEntry->StaConnectTime ++;

		pMbss = &pAd->ApCfg.MBSSID[pEntry->apidx];

		/* 0. STA failed to complete association should be removed to save MAC table space. */
		if ((pEntry->Sst != SST_ASSOC) && (pEntry->NoDataIdleCount >= pEntry->AssocDeadLine))
		{
			DBGPRINT(RT_DEBUG_TRACE,
					("%02x:%02x:%02x:%02x:%02x:%02x fail to complete ASSOC in %d sec\n",
					PRINT_MAC(pEntry->Addr), MAC_TABLE_ASSOC_TIMEOUT));
#ifdef WSC_AP_SUPPORT
			if (NdisEqualMemory(pEntry->Addr, pMbss->WscControl.EntryAddr, MAC_ADDR_LEN))
				NdisZeroMemory(pMbss->WscControl.EntryAddr, MAC_ADDR_LEN);
#endif /* WSC_AP_SUPPORT */
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			continue;
		}

		/* 1. check if there's any associated STA in power-save mode. this affects outgoing */
		/*    MCAST/BCAST frames should be stored in PSQ till DtimCount=0 */
		if (pEntry->PsMode == PWR_SAVE)
		{
			pMacTable->fAnyStationInPsm = TRUE;
#ifdef PS_ENTRY_MAITENANCE
			pEntry->continuous_ps_count++;
#endif /* PS_ENTRY_MAITENANCE */
		}

#ifdef DOT11_N_SUPPORT
		if (pEntry->MmpsMode == MMPS_DYNAMIC)
			pMacTable->fAnyStationMIMOPSDynamic = TRUE;

		if (pEntry->MaxHTPhyMode.field.BW == BW_20)
			pMacTable->fAnyStation20Only = TRUE;

		if (pEntry->MaxHTPhyMode.field.MODE != MODE_HTGREENFIELD)
			pMacTable->fAnyStationNonGF = TRUE;

		if ((pEntry->MaxHTPhyMode.field.MODE == MODE_OFDM) || (pEntry->MaxHTPhyMode.field.MODE == MODE_CCK))
			pMacTable->fAnyStationIsLegacy = TRUE;
#ifdef GREENAP_SUPPORT
		else
			pMacTable->fAnyStationIsHT=TRUE;
#endif /* GREENAP_SUPPORT */

#ifdef DOT11N_DRAFT3
		if (pEntry->bForty_Mhz_Intolerant)
			pMacTable->fAnyStaFortyIntolerant = TRUE;
#endif /* DOT11N_DRAFT3 */

		/* Get minimum AMPDU size from STA */
		if (MinimumAMPDUSize > pEntry->MaxRAmpduFactor)
			MinimumAMPDUSize = pEntry->MaxRAmpduFactor;
#endif /* DOT11_N_SUPPORT */
		
		if (pEntry->bIAmBadAtheros)
		{
			pMacTable->fAnyStationBadAtheros = TRUE;
#ifdef DOT11_N_SUPPORT
			if (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == FALSE)
				AsicUpdateProtect(pAd, 8, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
#endif /* DOT11_N_SUPPORT */
		}

		/* detect the station alive status */
		/* detect the station alive status */
		if ((pMbss->StationKeepAliveTime > 0) &&
			(pEntry->NoDataIdleCount >= pMbss->StationKeepAliveTime))
		{
			/*
				If no any data success between ap and the station for
				StationKeepAliveTime, try to detect whether the station is
				still alive.

				Note: Just only keepalive station function, no disassociation
				function if too many no response.
			*/

			/*
				For example as below:

				1. Station in ACTIVE mode,

		        ......
		        sam> tx ok!
		        sam> count = 1!	 ==> 1 second after the Null Frame is acked
		        sam> count = 2!	 ==> 2 second after the Null Frame is acked
		        sam> count = 3!
		        sam> count = 4!
		        sam> count = 5!
		        sam> count = 6!
		        sam> count = 7!
		        sam> count = 8!
		        sam> count = 9!
		        sam> count = 10!
		        sam> count = 11!
		        sam> count = 12!
		        sam> count = 13!
		        sam> count = 14!
		        sam> count = 15! ==> 15 second after the Null Frame is acked
		        sam> tx ok!      ==> (KeepAlive Mechanism) send a Null Frame to
										detect the STA life status
		        sam> count = 1!  ==> 1 second after the Null Frame is acked
		        sam> count = 2!
		        sam> count = 3!
		        sam> count = 4!
		        ......

				If the station acknowledges the QoS Null Frame,
				the NoDataIdleCount will be reset to 0.


				2. Station in legacy PS mode,

				We will set TIM bit after 15 seconds, the station will send a
				PS-Poll frame and we will send a QoS Null frame to it.
				If the station acknowledges the QoS Null Frame, the
				NoDataIdleCount will be reset to 0.


				3. Station in legacy UAPSD mode,

				Currently we do not support the keep alive mechanism.
				So if your station is in UAPSD mode, the station will be
				kicked out after 300 seconds.

				Note: the rate of QoS Null frame can not be 1M of 2.4GHz or
				6M of 5GHz, or no any statistics count will occur.
			*/

			if (pEntry->StationKeepAliveCount++ == 0)
			{
					if (pEntry->PsMode == PWR_SAVE)
					{
						/* use TIM bit to detect the PS station */
						WLAN_MR_TIM_BIT_SET(pAd, pEntry->apidx, pEntry->Aid);
					}
					else
					{
						/* use Null or QoS Null to detect the ACTIVE station */
						BOOLEAN bQosNull = FALSE;
	
						if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
							bQosNull = TRUE;

						RtmpEnqueueNullFrame(pAd, pEntry->Addr, pEntry->CurrTxRate,
	    	                           						pEntry->Aid, pEntry->apidx, bQosNull, TRUE, 0);
					}
			}
			else
			{
				if (pEntry->StationKeepAliveCount >= pMbss->StationKeepAliveTime)
					pEntry->StationKeepAliveCount = 0;
			}
		}

		/* 2. delete those MAC entry that has been idle for a long time */
		if ((pEntry->NoDataIdleCount >= pEntry->StaIdleTimeout)
#ifdef PS_ENTRY_MAITENANCE
			|| (pEntry->continuous_ps_count > pAd->ps_timeout)
#endif /* PS_ENTRY_MAITENANCE */
			)
		{
			bDisconnectSta = TRUE;

#ifdef PS_ENTRY_MAITENANCE
			if (pEntry->continuous_ps_count > pAd->ps_timeout)
				DBGPRINT(RT_DEBUG_WARN, ("ps_timeout(%u) !!!\n", pEntry->Aid));
#endif /* PS_ENTRY_MAITENANCE */
			DBGPRINT(RT_DEBUG_WARN, ("ageout %02x:%02x:%02x:%02x:%02x:%02x after %d-sec silence\n",
					PRINT_MAC(pEntry->Addr), pEntry->StaIdleTimeout));
			ApLogEvent(pAd, pEntry->Addr, EVENT_AGED_OUT);
#ifdef SMART_MESH_MONITOR
			{
				struct nsmpif_drvevnt_buf drvevnt;
				drvevnt.data.leave.type = NSMPIF_DRVEVNT_STA_TIMEOUT;
				drvevnt.data.leave.channel = pAd->CommonCfg.Channel;
				NdisCopyMemory(drvevnt.data.leave.sta_mac, pEntry->Addr, MAC_ADDR_LEN);
				RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_STA_TIMEOUT,
										NULL, (PUCHAR)&drvevnt.data.leave, sizeof(drvevnt.data.leave));
			}
#endif /* SMART_MESH_MONITOR */
		}
		else if (pEntry->ContinueTxFailCnt >= pAd->ApCfg.EntryLifeCheck)
		{
			/*
				AP have no way to know that the PwrSaving STA is leaving or not.
				So do not disconnect for PwrSaving STA.
			*/
			if (pEntry->PsMode != PWR_SAVE)
			{
				bDisconnectSta = TRUE;
#ifdef DROP_MASK_SUPPORT
				asic_set_drop_mask(pAd, pEntry->Aid, TRUE);
#else
				ClearTxRingClientAck(pAd, pEntry);
#endif
				DBGPRINT(RT_DEBUG_WARN, ("STA-%02x:%02x:%02x:%02x:%02x:%02x had left (%d %lu)\n",
					PRINT_MAC(pEntry->Addr),
					pEntry->ContinueTxFailCnt, pAd->ApCfg.EntryLifeCheck));
			}
		}
#ifdef BAND_STEERING
		else if (pAd->ApCfg.BndStrgTable.bEnabled == TRUE)
		{
			if (BndStrg_IsClientStay(pAd, pEntry) == FALSE)
			{
				bDisconnectSta = TRUE;
			}
		}
#endif /* BAND_STEERING */
		
		//YF: kickout sta when 3 of 5 exceeds the threshold.
		if (pMbss->RssiLowForStaKickOut != 0)
		{
#define CHECK_DATA_RSSI_UP_BOUND 3
			CHAR rssiIndex = 0, overRssiThresCount = 0; 

			for (rssiIndex=0; rssiIndex<MAX_LAST_DATA_RSSI_LEN; rssiIndex++)
			{
				if ((pEntry->LastDataRssi[rssiIndex] !=0 ) && 
				    (pEntry->LastDataRssi[rssiIndex] < pMbss->RssiLowForStaKickOut))
				{
					DBGPRINT(RT_DEBUG_TRACE, ("%d:[%d] Fail.\n",rssiIndex,pEntry->LastDataRssi[rssiIndex]));
					overRssiThresCount++;
				}
			}
	
			if (overRssiThresCount >= CHECK_DATA_RSSI_UP_BOUND)
			{
				bDisconnectSta = TRUE;
				DBGPRINT(RT_DEBUG_WARN, ("Dis STA %02x:%02x:%02x:%02x:%02x:%02x , RSSI Kickout Thres[%d]-[%d]Times\n",
							PRINT_MAC(pEntry->Addr), pMbss->RssiLowForStaKickOut, overRssiThresCount));
			}	

		}

		if (bDisconnectSta)
		{
			/* send wireless event - for ageout */
			RTMPSendWirelessEvent(pAd, IW_AGEOUT_EVENT_FLAG, pEntry->Addr, 0, 0); 

			if (pEntry->Sst == SST_ASSOC)
			{
				PUCHAR pOutBuffer = NULL;
				NDIS_STATUS NStatus;
				ULONG FrameLen = 0;
				HEADER_802_11 DeAuthHdr;
				USHORT Reason;

				/*  send out a DISASSOC request frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS) 
				{
					DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					/*NdisReleaseSpinLock(&pAd->MacTabLock); */
					continue;
				}
				Reason = REASON_DEAUTH_STA_LEAVING;
				DBGPRINT(RT_DEBUG_WARN, ("Send DEAUTH - Reason = %d frame  TO %x %x %x %x %x %x \n",
										Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DeAuthHdr, SUBTYPE_DEAUTH, 0, pEntry->Addr, 
								pMbss->wdev.if_addr,
								pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, 
								sizeof(HEADER_802_11), &DeAuthHdr, 
								2, &Reason, 
								END_OF_ARGS);				
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pAd, pOutBuffer);

				/* wait for DEAUTH processed */
				OS_WAIT(5);

#ifdef MAC_REPEATER_SUPPORT
				if ((pAd->ApCfg.bMACRepeaterEn == TRUE) && IS_ENTRY_CLIENT(pEntry)
#ifdef MWDS
					&& (pEntry->bEnableMWDS == FALSE)
#endif /* MWDS */
				)
				{
					UCHAR apCliIdx, CliIdx, isLinkValid;
					REPEATER_CLIENT_ENTRY *pReptEntry = NULL;

					pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, pEntry->Addr ,TRUE, &isLinkValid);
					if (pReptEntry && (pReptEntry->CliConnectState != 0))
					{
						apCliIdx = pReptEntry->MatchApCliIdx;
						CliIdx = pReptEntry->MatchLinkIdx;
#ifdef DOT11_N_SUPPORT
						/* free resources of BA*/
						BASessionTearDownALL(pAd, pReptEntry->MacTabWCID);
#endif /* DOT11_N_SUPPORT */
						RTMPRemoveRepeaterDisconnectEntry(pAd, apCliIdx, CliIdx);
								RTMPRemoveRepeaterEntry(pAd, apCliIdx, CliIdx);
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
			}

			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
			continue;
		}
#ifdef CONFIG_HOTSPOT_R2
		if (pEntry->BTMDisassocCount == 1)
		{
			PUCHAR		pOutBuffer = NULL;
			NDIS_STATUS NStatus;
			ULONG		FrameLen = 0;
			HEADER_802_11 DisassocHdr;
			USHORT		Reason;

			/*	send out a DISASSOC request frame */
			NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
			if (NStatus != NDIS_STATUS_SUCCESS) 
			{
				DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
				/*NdisReleaseSpinLock(&pAd->MacTabLock); */
				continue;
			}

			Reason = REASON_DISASSOC_INACTIVE;
			DBGPRINT(RT_DEBUG_ERROR, ("BTM ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",Reason,pEntry->Addr[0],
				pEntry->Addr[1],pEntry->Addr[2],pEntry->Addr[3],pEntry->Addr[4],pEntry->Addr[5]));
			MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pEntry->Addr, pMbss->wdev.if_addr, pMbss->wdev.bssid);
			MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
			MiniportMMRequest(pAd, MGMT_USE_PS_FLAG, pOutBuffer, FrameLen);
			MlmeFreeMemory(pAd, pOutBuffer);
			//JERRY
			if (!pEntry->IsKeep)
				MacTableDeleteEntry(pAd, pEntry->Aid, pEntry->Addr);
			continue;
		}
		if (pEntry->BTMDisassocCount != 0)
			pEntry->BTMDisassocCount--;
#endif /* CONFIG_HOTSPOT_R2 */

		/* 3. garbage collect the PsQueue if the STA has being idle for a while */
		if (pEntry->PsQueue.Head)
		{
			pEntry->PsQIdleCount++;
#ifdef NOISE_TEST_ADJUST
			if (pEntry->PsQIdleCount > 4)
#else
			if (pEntry->PsQIdleCount > 2)
#endif /* NOISE_TEST_ADJUST */
			{
				ULONG irq_flags;
				RTMP_IRQ_LOCK(&pAd->irq_lock, irq_flags);
				APCleanupPsQueue(pAd, &pEntry->PsQueue);
				RTMP_IRQ_UNLOCK(&pAd->irq_lock, irq_flags);
				pEntry->PsQIdleCount = 0;
				WLAN_MR_TIM_BIT_CLEAR(pAd, pEntry->apidx, pEntry->Aid);
			}
		}
		else
			pEntry->PsQIdleCount = 0;
	
#ifdef UAPSD_SUPPORT
		UAPSD_QueueMaintenance(pAd, pEntry);
#endif /* UAPSD_SUPPORT */

		/* check if this STA is Ralink-chipset */
		if (!CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
			pMacTable->fAllStationAsRalink = FALSE;

		/* Check if the port is secured */
		if (pEntry->PortSecured == WPA_802_1X_PORT_SECURED)
			fAnyStationPortSecured[pEntry->apidx]++;
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
		if ((pEntry->BSS2040CoexistenceMgmtSupport) 
			&& (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
			&& (pAd->CommonCfg.bBssCoexEnable == TRUE)
		)
		{
			SendNotifyBWActionFrame(pAd, pEntry->wcid, pEntry->apidx);
		}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
#ifdef WAPI_SUPPORT
		if (pEntry->WepStatus == Ndis802_11EncryptionSMS4Enabled)
			pMacTable->fAnyWapiStation = TRUE;
#endif /* WAPI_SUPPORT */

#if defined(PRE_ANT_SWITCH) || defined(CFO_TRACK)
		lastClient = i;
#endif /* defined(PRE_ANT_SWITCH) || defined(CFO_TRACK) */

		/* only apply burst when run in MCS0,1,8,9,16,17, not care about phymode */
		if ((pEntry->HTPhyMode.field.MCS != 32) && 
			((pEntry->HTPhyMode.field.MCS % 8 == 0) || (pEntry->HTPhyMode.field.MCS % 8 == 1)))
		{
			pMacTable->fAllStationGainGoodMCS = FALSE;
		}

#ifdef WFA_VHT_PF
		if (worst_rssi == NULL) {
			worst_rssi = &pEntry->RssiSample;
			worst_rssi_sta_idx = i;
		} else {
			if (worst_rssi->AvgRssi0 > pEntry->RssiSample.AvgRssi0) {
				worst_rssi = &pEntry->RssiSample;
				worst_rssi_sta_idx = i;
			}
		}
#endif /* WFA_VHT_PF */
	}

#ifdef WFA_VHT_PF
	if (worst_rssi != NULL && 
		((pAd->Mlme.OneSecPeriodicRound % 10) == 5) &&
		(worst_rssi_sta_idx >= 1))
	{
		CHAR gain = 2;
		if (worst_rssi->AvgRssi0 >= -40)
			gain = 1;
		else if (worst_rssi->AvgRssi0 <= -50)
			gain = 2;
		rt85592_lna_gain_adjust(pAd, gain);
		DBGPRINT(RT_DEBUG_TRACE, ("%s():WorstRSSI for STA(%02x:%02x:%02x:%02x:%02x:%02x):%d,%d,%d, Set Gain as %s\n",
					__FUNCTION__,
					PRINT_MAC(pMacTable->Content[worst_rssi_sta_idx].Addr),
					worst_rssi->AvgRssi0, worst_rssi->AvgRssi1, worst_rssi->AvgRssi2,
					(gain == 2 ? "Mid" : "Low")));
	}
#endif /* WFA_VHT_PF */

#ifdef PRE_ANT_SWITCH
#endif /* PRE_ANT_SWITCH */

#ifdef CFO_TRACK
#endif /* CFO_TRACK */

	/* Update the state of port per MBSS */
	for (bss_index = BSS0; bss_index < MAX_MBSSID_NUM(pAd); bss_index++)
	{
#ifdef ED_MONITOR
		total_sta += pAd->ApCfg.MBSSID[bss_index].StaCount;
#endif /* ED_MONITOR */
		if (fAnyStationPortSecured[bss_index] > 0)
			pAd->ApCfg.MBSSID[bss_index].wdev.PortSecured = WPA_802_1X_PORT_SECURED;
		else
			pAd->ApCfg.MBSSID[bss_index].wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.Bss2040CoexistFlag & BSS_2040_COEXIST_INFO_NOTIFY)
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_NOTIFY);
#endif /* DOT11N_DRAFT3 */

	/* If all associated STAs are Ralink-chipset, AP shall enable RDG. */
	if (pAd->CommonCfg.bRdg && pMacTable->fAllStationAsRalink)
	{
#ifdef MWDS
		if (pAd->MacTab.Size >= 2)
			bRdgActive = FALSE;
		else
#endif /* MWDS */
		bRdgActive = TRUE;
	}
	else
		bRdgActive = FALSE;

	if (pAd->CommonCfg.bRalinkBurstMode && pMacTable->fAllStationGainGoodMCS)
		bRalinkBurstMode = TRUE;
	else
		bRalinkBurstMode = FALSE;

#ifdef DOT11_N_SUPPORT
#ifdef GREENAP_SUPPORT
	if (WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		if(pAd->MacTab.fAnyStationIsHT == FALSE
			&& pAd->ApCfg.bGreenAPEnable == TRUE)
		{
#ifdef RTMP_RBUS_SUPPORT
#ifdef COC_SUPPORT
			if ((pAd->MacTab.Size==0) &&
				(pAd->ApCfg.GreenAPLevel != GREENAP_WITHOUT_ANY_STAS_CONNECT))
			{
					RTMP_CHIP_ENABLE_AP_MIMOPS(pAd,TRUE);
					pAd->ApCfg.GreenAPLevel = GREENAP_WITHOUT_ANY_STAS_CONNECT;
				
			}
			else
#endif /* COC_SUPPORT */
#endif /* RTMP_RBUS_SUPPORT */
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_ONLY_11BG_STAS)
			{
				RTMP_CHIP_ENABLE_AP_MIMOPS(pAd,FALSE);
				pAd->ApCfg.GreenAPLevel=GREENAP_ONLY_11BG_STAS;
			}
		}
		else
		{
			if (pAd->ApCfg.GreenAPLevel!=GREENAP_11BGN_STAS)
			{
				RTMP_CHIP_DISABLE_AP_MIMOPS(pAd);
				pAd->ApCfg.GreenAPLevel=GREENAP_11BGN_STAS;
			}
		}
	}
#endif /* GREENAP_SUPPORT */

	if (bRdgActive != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
		AsicSetRDG(pAd, bRdgActive);

	if (bRalinkBurstMode != RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RALINK_BURST_MODE))
		AsicSetRalinkBurstMode(pAd, bRalinkBurstMode);
#endif /* DOT11_N_SUPPORT */


	if ((pMacTable->fAnyStationBadAtheros == FALSE) && (pAd->CommonCfg.IOTestParm.bRTSLongProtOn == TRUE))
	{
		AsicUpdateProtect(pAd, pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, ALLN_SETPROTECT, FALSE, pMacTable->fAnyStationNonGF);
	}
#endif /* DOT11_N_SUPPORT */

#ifdef RTMP_MAC_PCI
	RTMP_IRQ_LOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
	/*
		4. 
		garbage collect pAd->MacTab.McastPsQueue if backlogged MCAST/BCAST frames
		stale in queue. Since MCAST/BCAST frames always been sent out whenever
		DtimCount==0, the only case to let them stale is surprise removal of the NIC,
		so that ASIC-based Tbcn interrupt stops and DtimCount dead.
	*/
	if (pMacTable->McastPsQueue.Head)
	{
		UINT bss_index;

		pMacTable->PsQIdleCount ++;
#ifdef NOISE_TEST_ADJUST
		if (pMacTable->PsQIdleCount > 2)
#else
		if (pMacTable->PsQIdleCount > 1)
#endif /* NOISE_TEST_ADJUST */
		{

			/*NdisAcquireSpinLock(&pAd->MacTabLock); */
			APCleanupPsQueue(pAd, &pMacTable->McastPsQueue);
			/*NdisReleaseSpinLock(&pAd->MacTabLock); */
			pMacTable->PsQIdleCount = 0;

			if (pAd->ApCfg.BssidNum > MAX_MBSSID_NUM(pAd))
				pAd->ApCfg.BssidNum = MAX_MBSSID_NUM(pAd);
	        
		        /* clear MCAST/BCAST backlog bit for all BSS */
			for(bss_index=BSS0; bss_index<pAd->ApCfg.BssidNum; bss_index++)
				WLAN_MR_TIM_BCMC_CLEAR(bss_index);
		}
	}
	else
		pMacTable->PsQIdleCount = 0;
#ifdef RTMP_MAC_PCI
	RTMP_IRQ_UNLOCK(&pAd->irq_lock, IrqFlags);
#endif /* RTMP_MAC_PCI */
}


UINT32 MacTableAssocStaNumGet(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 num = 0;
	UINT32 i;


	for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++) 
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];

		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		if (pEntry->Sst == SST_ASSOC)
			num ++;
	}

	return num;
}


/*
	==========================================================================
	Description:
		Look up a STA MAC table. Return its Sst to decide if an incoming
		frame from this STA or an outgoing frame to this STA is permitted.
	Return:
	==========================================================================
*/
MAC_TABLE_ENTRY *APSsPsInquiry(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr, 
	OUT SST *Sst, 
	OUT USHORT *Aid,
	OUT UCHAR *PsMode,
	OUT UCHAR *Rate) 
{
	MAC_TABLE_ENTRY *pEntry = NULL;

	if (MAC_ADDR_IS_GROUP(pAddr)) /* mcast & broadcast address */
	{
		*Sst = SST_ASSOC;
		*Aid = MCAST_WCID;	/* Softap supports 1 BSSID and use WCID=0 as multicast Wcid index */
		*PsMode = PWR_ACTIVE;
		*Rate = pAd->CommonCfg.MlmeRate; 
	} 
	else /* unicast address */
	{
		pEntry = MacTableLookup(pAd, pAddr);
		if (pEntry)
		{
			*Sst = pEntry->Sst;
			*Aid = pEntry->Aid;
			*PsMode = pEntry->PsMode;
			if ((pEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pEntry->GTKState != REKEY_ESTABLISHED))
				*Rate = pAd->CommonCfg.MlmeRate;
			else
				*Rate = pEntry->CurrTxRate;
		} 
		else 
		{
			*Sst = SST_NOT_AUTH;
			*Aid = MCAST_WCID;
			*PsMode = PWR_ACTIVE;
			*Rate = pAd->CommonCfg.MlmeRate; 
		}
	}

	return pEntry;
}


#ifdef SYSTEM_LOG_SUPPORT
/*
	==========================================================================
	Description:
		This routine is called to log a specific event into the event table.
		The table is a QUERY-n-CLEAR array that stop at full.
	==========================================================================
 */
VOID ApLogEvent(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR   pAddr,
	IN USHORT   Event)
{
	if (pAd->EventTab.Num < MAX_NUM_OF_EVENT)
	{
		RT_802_11_EVENT_LOG *pLog = &pAd->EventTab.Log[pAd->EventTab.Num];
		RTMP_GetCurrentSystemTime(&pLog->SystemTime);
		COPY_MAC_ADDR(pLog->Addr, pAddr);
		pLog->Event = Event;
		DBGPRINT_RAW(RT_DEBUG_TRACE,("LOG#%ld %02x:%02x:%02x:%02x:%02x:%02x %s\n",
			pAd->EventTab.Num, pAddr[0], pAddr[1], pAddr[2], 
			pAddr[3], pAddr[4], pAddr[5], pEventText[Event]));
		pAd->EventTab.Num += 1;
	}
}
#endif /* SYSTEM_LOG_SUPPORT */


#ifdef DOT11_N_SUPPORT
/*
	==========================================================================
	Description:
		Operationg mode is as defined at 802.11n for how proteciton in this BSS operates. 
		Ap broadcast the operation mode at Additional HT Infroamtion Element Operating Mode fields.
		802.11n D1.0 might has bugs so this operating mode use  EWC MAC 1.24 definition first.

		Called when receiving my bssid beacon or beaconAtJoin to update protection mode.
		40MHz or 20MHz protection mode in HT 40/20 capabale BSS.
		As STA, this obeys the operation mode in ADDHT IE.
		As AP, update protection when setting ADDHT IE and after new STA joined.
	==========================================================================
*/
VOID APUpdateOperationMode(
	IN PRTMP_ADAPTER pAd)
{
	BOOLEAN bDisableBGProtect = FALSE, bNonGFExist = FALSE;

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 0;
	if ((pAd->ApCfg.LastNoneHTOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) /* non HT BSS exist within 5 sec */
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 1;
		bDisableBGProtect = FALSE;
		bNonGFExist = TRUE;
	}

   	/* If I am 40MHz BSS, and there exist HT-20MHz station. */
	/* Update to 2 when it's zero.  Because OperaionMode = 1 or 3 has more protection. */
	if ((pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode == 0) &&
		(pAd->MacTab.fAnyStation20Only) &&
		(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == 1))
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 2;
		bDisableBGProtect = TRUE;
	}
		
	if (pAd->MacTab.fAnyStationIsLegacy)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode = 3;
		bDisableBGProtect = TRUE;
	}

	if (pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode == 0 &&
		!ERP_IS_USE_PROTECTION(pAd->ApCfg.ErpIeContent))
	{
		bDisableBGProtect = TRUE;
	}
	
	if (bNonGFExist == FALSE)
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;

	AsicUpdateProtect(pAd,
						pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode, 
						(ALLN_SETPROTECT),
						bDisableBGProtect,
						bNonGFExist);

	pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = pAd->MacTab.fAnyStationNonGF;
}
#endif /* DOT11_N_SUPPORT */

/*
	==========================================================================
	Description:
		Update ERP IE and CapabilityInfo based on STA association status.
		The result will be auto updated into the next outgoing BEACON in next
		TBTT interrupt service routine
	==========================================================================
 */
VOID APUpdateCapabilityAndErpIe(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR  i, ErpIeContent = 0;
	BOOLEAN ShortSlotCapable = pAd->CommonCfg.bUseShortSlotTime;
	UCHAR	apidx;
	BOOLEAN	bUseBGProtection;
	BOOLEAN	LegacyBssExist;


	if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
		return;

	for (i=1; i<MAX_LEN_OF_MAC_TABLE; i++)
	{
		MAC_TABLE_ENTRY *pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;

		/* at least one 11b client associated, turn on ERP.NonERPPresent bit */
		/* almost all 11b client won't support "Short Slot" time, turn off for maximum compatibility */
		if (pEntry->MaxSupportedRate < RATE_FIRST_OFDM_RATE)
		{
			ShortSlotCapable = FALSE;
			ErpIeContent |= 0x01;
		}

		/* at least one client can't support short slot */
		if ((pEntry->CapabilityInfo & 0x0400) == 0)
			ShortSlotCapable = FALSE;
	}

	/* legacy BSS exist within 5 sec */
	if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) 
		LegacyBssExist = TRUE;
	else
		LegacyBssExist = FALSE;
	
	/* decide ErpIR.UseProtection bit, depending on pAd->CommonCfg.UseBGProtection
		AUTO (0): UseProtection = 1 if any 11b STA associated
		ON (1): always USE protection
		OFF (2): always NOT USE protection
	*/
	if (pAd->CommonCfg.UseBGProtection == 0)
	{
		ErpIeContent = (ErpIeContent)? 0x03 : 0x00;
		/*if ((pAd->ApCfg.LastOLBCDetectTime + (5 * OS_HZ)) > pAd->Mlme.Now32) // legacy BSS exist within 5 sec */
		if (LegacyBssExist)
		{
			ErpIeContent |= 0x02;                                     /* set Use_Protection bit */
		}
	}
	else if (pAd->CommonCfg.UseBGProtection == 1)   
		ErpIeContent |= 0x02;


	bUseBGProtection = (pAd->CommonCfg.UseBGProtection == 1) ||    /* always use */
						((pAd->CommonCfg.UseBGProtection == 0) && ERP_IS_USE_PROTECTION(ErpIeContent));

#ifdef A_BAND_SUPPORT
	/* always no BG protection in A-band. falsely happened when switching A/G band to a dual-band AP */
	if (pAd->CommonCfg.Channel > 14) 
		bUseBGProtection = FALSE;
#endif /* A_BAND_SUPPORT */

	if (bUseBGProtection != OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED))
	{
		USHORT OperationMode = 0;
		BOOLEAN	bNonGFExist = 0;

#ifdef DOT11_N_SUPPORT
		OperationMode = pAd->CommonCfg.AddHTInfo.AddHtInfo2.OperaionMode;
		bNonGFExist = pAd->MacTab.fAnyStationNonGF;
#endif /* DOT11_N_SUPPORT */
		if (bUseBGProtection)
		{
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), FALSE, bNonGFExist);
		}
		else
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_BG_PROTECTION_INUSED);
			AsicUpdateProtect(pAd, OperationMode, (OFDMSETPROTECT), TRUE, bNonGFExist);
		}
	}

	/* Decide Barker Preamble bit of ERP IE */
	if ((pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong) || (ApCheckLongPreambleSTA(pAd) == TRUE))
		pAd->ApCfg.ErpIeContent = (ErpIeContent | 0x04);
	else
		pAd->ApCfg.ErpIeContent = ErpIeContent;

#ifdef A_BAND_SUPPORT
	/* Force to use ShortSlotTime at A-band */
	if (pAd->CommonCfg.Channel > 14)
		ShortSlotCapable = TRUE;
#endif /* A_BAND_SUPPORT */
	
	/* deicide CapabilityInfo.ShortSlotTime bit */
    for (apidx=0; apidx<pAd->ApCfg.BssidNum; apidx++)
    {
		USHORT *pCapInfo = &(pAd->ApCfg.MBSSID[apidx].CapabilityInfo);

		/* In A-band, the ShortSlotTime bit should be ignored. */
		if (ShortSlotCapable 
#ifdef A_BAND_SUPPORT
			&& (pAd->CommonCfg.Channel <= 14)
#endif /* A_BAND_SUPPORT */
			)
    		(*pCapInfo) |= 0x0400;
		else
    		(*pCapInfo) &= 0xfbff;


   		if (pAd->CommonCfg.TxPreamble == Rt802_11PreambleLong)
			(*pCapInfo) &= (~0x020);
		else
			(*pCapInfo) |= 0x020;

	}

	AsicSetSlotTime(pAd, ShortSlotCapable);

}

/*
	==========================================================================
	Description:
        Check to see the exist of long preamble STA in associated list
    ==========================================================================
 */
BOOLEAN ApCheckLongPreambleSTA(
    IN PRTMP_ADAPTER pAd)
{
    UCHAR   i;
    
    for (i=0; i<MAX_LEN_OF_MAC_TABLE; i++)
    {
		PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];
		if (!IS_ENTRY_CLIENT(pEntry) || (pEntry->Sst != SST_ASSOC))
			continue;
	            
        if (!CAP_IS_SHORT_PREAMBLE_ON(pEntry->CapabilityInfo))
        {
            return TRUE;
        }
    }

    return FALSE;
}    

/*
	==========================================================================
	Description:
		Check if the specified STA pass the Access Control List checking.
		If fails to pass the checking, then no authentication nor association 
		is allowed
	Return:
		MLME_SUCCESS - this STA passes ACL checking

	==========================================================================
*/
BOOLEAN ApCheckAccessControlList(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR        pAddr,
	IN UCHAR         Apidx)
{
	BOOLEAN Result = TRUE;

#ifdef CUSTOMER_DCC_FEATURE
	if (pAd->ApDisableSTAConnectFlag == TRUE)
	{
		INT count, i;
		UINT32 time;
		
		Result = FALSE;
		time = jiffies_to_msecs(jiffies);
		count = pAd->AllowedStaList.StaCount;
		for (i = 0; i < count; i++)
		{
			if (NdisEqualMemory(&(pAd->AllowedStaList.AllowedSta[i].MacAddr[0]), pAddr, MAC_ADDR_LEN))
			{
				if ((time - pAd->AllowedStaList.AllowedSta[i].DissocTime) < 30000)
				{
					Result = TRUE;
				}
			}
		}
		
		if (!Result)
			return Result;
	}
#endif
    if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 0)       /* ACL is disabled */
        Result = TRUE;
    else
    {
        ULONG i;
        if (pAd->ApCfg.MBSSID[Apidx].AccessControlList.Policy == 1)   /* ACL is a positive list */
            Result = FALSE;
        else                                              /* ACL is a negative list */
            Result = TRUE;
        for (i=0; i<pAd->ApCfg.MBSSID[Apidx].AccessControlList.Num; i++)
        {
            if (MAC_ADDR_EQUAL(pAddr, pAd->ApCfg.MBSSID[Apidx].AccessControlList.Entry[i].Addr))
            {
                Result = !Result;
                break;
            }
        }
    }

#ifdef SMART_MESH
	if (pAd->ApCfg.MBSSID[Apidx].SmartMeshCfg.bSupportSmartMesh &&
		!pAd->ApCfg.MBSSID[Apidx].bSmartMeshACL)
		 Result = TRUE;
#endif /* SMART_MESH */

    if (Result == FALSE)
    {
        CHAR *pRange = (pAd->CommonCfg.Channel <= 14) ? "2.4" : "5";
        printk("AP %sGHz - access denied for client MAC [%02x:%02x:%02x:%02x:%02x:%02x]!\n",
                pRange, pAddr[0], pAddr[1], pAddr[2], pAddr[3], pAddr[4], pAddr[5]);
    }

    return Result;
}

/*
	==========================================================================
	Description:
		This routine update the current MAC table based on the current ACL.
		If ACL change causing an associated STA become un-authorized. This STA
		will be kicked out immediately.
	==========================================================================
*/
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR Apidx)
{
	USHORT   AclIdx, MacIdx;
	BOOLEAN  Matched;

	PUCHAR      pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	ULONG       FrameLen = 0;
	HEADER_802_11 DisassocHdr;
	USHORT      Reason;
	MAC_TABLE_ENTRY *pEntry;
	MULTISSID_STRUCT *pMbss;
	BOOLEAN drop;

	ASSERT(Apidx < MAX_MBSSID_NUM(pAd));
	if (Apidx >= MAX_MBSSID_NUM(pAd))
		return;
	DBGPRINT(RT_DEBUG_TRACE, ("ApUpdateAccessControlList : Apidx = %d\n", Apidx));
	
    /* ACL is disabled. Do nothing about the MAC table. */
	pMbss = &pAd->ApCfg.MBSSID[Apidx];
	if (pMbss->AccessControlList.Policy == 0)
		return;

	for (MacIdx=0; MacIdx < MAX_LEN_OF_MAC_TABLE; MacIdx++)
	{
		pEntry = &pAd->MacTab.Content[MacIdx];
		if (!IS_ENTRY_CLIENT(pEntry))
			continue;

		/* We only need to update associations related to ACL of MBSSID[Apidx]. */
		if (pEntry->apidx != Apidx) 
			continue;
    
		drop = FALSE;
		Matched = FALSE;
		 for (AclIdx = 0; AclIdx < pMbss->AccessControlList.Num; AclIdx++)
		{
			if (MAC_ADDR_EQUAL(&pEntry->Addr[0], pMbss->AccessControlList.Entry[AclIdx].Addr))
			{
				Matched = TRUE;
				break;
			}
		}

		if ((Matched == FALSE) && (pMbss->AccessControlList.Policy == 1))
		{
			drop = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("STA not on positive ACL. remove it...\n"));
		}
	       else if ((Matched == TRUE) && (pMbss->AccessControlList.Policy == 2))
			{
			drop = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("STA on negative ACL. remove it...\n"));
				}

		if (drop == TRUE) {
			DBGPRINT(RT_DEBUG_TRACE, ("Apidx = %d\n", Apidx));
			DBGPRINT(RT_DEBUG_TRACE, ("pAd->ApCfg.MBSSID[%d].AccessControlList.Policy = %ld\n", Apidx,
				pMbss->AccessControlList.Policy));
			
			/* Before delete the entry from MacTable, send disassociation packet to client. */
			if (pEntry->Sst == SST_ASSOC)
			{
				/* send out a DISASSOC frame */
				NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);
				if (NStatus != NDIS_STATUS_SUCCESS) 
				{
					DBGPRINT(RT_DEBUG_TRACE, (" MlmeAllocateMemory fail  ..\n"));
					return;
				}

				Reason = REASON_DECLINED;
				DBGPRINT(RT_DEBUG_ERROR, ("ASSOC - Send DISASSOC  Reason = %d frame  TO %x %x %x %x %x %x \n",
							Reason, PRINT_MAC(pEntry->Addr)));
				MgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0,
									pEntry->Addr, 
									pMbss->wdev.if_addr,
									pMbss->wdev.bssid);
				MakeOutgoingFrame(pOutBuffer, &FrameLen, sizeof(HEADER_802_11), &DisassocHdr, 2, &Reason, END_OF_ARGS);
				MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
				MlmeFreeMemory(pAd, pOutBuffer);

				RtmpusecDelay(5000);
			}
			MacTableDeleteEntry(pAd, pEntry->wcid, pEntry->Addr);
		}
	}
}


#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
/*
	Depends on the 802.11n Draft 4.0, Before the HT AP start a BSS, it should scan some specific channels to
collect information of existing BSSs, then depens on the collected channel information, adjust the primary channel 
and secondary channel setting.

	For 5GHz,
		Rule 1: If the AP chooses to start a 20/40 MHz BSS in 5GHz and that occupies the same two channels
				as any existing 20/40 MHz BSSs, then the AP shall ensure that the primary channel of the 
				new BSS is identical to the primary channel of the existing 20/40 MHz BSSs and that the 
				secondary channel of the new 20/40 MHz BSS is identical to the secondary channel of the 
				existing 20/40 MHz BSSs, unless the AP discoverr that on those two channels are existing
				20/40 MHz BSSs with different primary and secondary channels.
		Rule 2: If the AP chooses to start a 20/40MHz BSS in 5GHz, the selected secondary channel should
				correspond to a channel on which no beacons are detected during the overlapping BSS
				scan time performed by the AP, unless there are beacons detected on both the selected
				primary and secondary channels.
		Rule 3: An HT AP should not start a 20 MHz BSS in 5GHz on a channel that is the secondary channel 
				of a 20/40 MHz BSS.
	For 2.4GHz,
		Rule 1: The AP shall not start a 20/40 MHz BSS in 2.4GHz if the value of the local variable "20/40
				Operation Permitted" is FALSE.

		20/40OperationPermitted =  (P == OPi for all values of i) AND
								(P == OTi for all values of i) AND
								(S == OSi for all values if i)
		where
			P 	is the operating or intended primary channel of the 20/40 MHz BSS
			S	is the operating or intended secondary channel of the 20/40 MHz BSS
			OPi  is member i of the set of channels that are members of the channel set C and that are the
				primary operating channel of at least one 20/40 MHz BSS that is detected within the AP's 
				BSA during the previous X seconds
			OSi  is member i of the set of channels that are members of the channel set C and that are the
				secondary operating channel of at least one 20/40 MHz BSS that is detected within AP's
				BSA during the previous X seconds
			OTi  is member i of the set of channels that comparises all channels that are members of the 
				channel set C that were listed once in the Channel List fields of 20/40 BSS Intolerant Channel
				Report elements receved during the previous X seconds and all channels that are members
				of the channel set C and that are the primary operating channel of at least one 20/40 MHz
				BSS that were detected within the AP's BSA during the previous X seconds.
			C	is the set of all channels that are allowed operating channels within the current operational
				regulatory domain and whose center frequency falls within the 40 MHz affected channel 
				range given by following equation:
					                                                 Fp + Fs                  Fp + Fs
					40MHz affected channel range = [ ------  - 25MHz,  ------- + 25MHz ]
					                                                      2                          2
					Where 
						Fp = the center frequency of channel P
						Fs = the center frequency of channel S

			"==" means that the values on either side of the "==" are to be tested for equaliy with a resulting 
				 Boolean value.
			        =>When the value of OPi is the empty set, then the expression (P == OPi for all values of i) 
			        	is defined to be TRUE
			        =>When the value of OTi is the empty set, then the expression (P == OTi for all values of i) 
			        	is defined to be TRUE
			        =>When the value of OSi is the empty set, then the expression (S == OSi for all values of i) 
			        	is defined to be TRUE
*/


INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN BSS_COEX_CH_RANGE *pCoexChRange)
{
	INT index, cntrCh = 0;

	memset(pCoexChRange, 0, sizeof(BSS_COEX_CH_RANGE));
	
	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = pAd->ChannelList[index].Channel;
			
			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh + 4;
				pCoexChRange->secondaryCh = pCoexChRange->effectChEnd;
			}
			else
			{
				pCoexChRange->effectChStart = pCoexChRange->primaryCh -4;
				pCoexChRange->effectChEnd = pCoexChRange->primaryCh;
				pCoexChRange->secondaryCh = pCoexChRange->effectChStart;
			}

			DBGPRINT(RT_DEBUG_TRACE,("5.0GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PriCh=[Idx:%d, CH:%d], SecCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n", 
										index, 
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"), 
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel, 
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			return TRUE;
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the CtrlCh(%d) in ChList, something wrong?\n", 
						pAd->CommonCfg.Channel));
		}
	}
	else
#endif /* A_BAND_SUPPORT */		
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			/* First get the primary channel */
			pCoexChRange->primaryCh = index;
			
			/* Now check about the secondary and central channel */
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				if ((index + 4) < pAd->ChannelListNum)
				{
					cntrCh = index + 2;
					pCoexChRange->secondaryCh = index + 4;
				}
			}
			else
			{
				if ((index - 4) >=0)
				{
					cntrCh = index - 2;
					pCoexChRange->secondaryCh = index - 4;
				}
			}

			if (cntrCh)
			{
				pCoexChRange->effectChStart = (cntrCh - 5) > 0 ? (cntrCh - 5) : 0;
				pCoexChRange->effectChEnd= (cntrCh + 5) > 0 ? (cntrCh + 5) : 0;
				DBGPRINT(RT_DEBUG_TRACE,("2.4GHz: Found CtrlCh idx(%d) from the ChList, ExtCh=%s, PrimaryCh=[Idx:%d, CH:%d], SecondaryCh=[Idx:%d, CH:%d], effected Ch=[CH:%d~CH:%d]!\n", 
										index, 
										((pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE) ? "ABOVE" : "BELOW"), 
										pCoexChRange->primaryCh, pAd->ChannelList[pCoexChRange->primaryCh].Channel, 
										pCoexChRange->secondaryCh, pAd->ChannelList[pCoexChRange->secondaryCh].Channel,
										pAd->ChannelList[pCoexChRange->effectChStart].Channel,
										pAd->ChannelList[pCoexChRange->effectChEnd].Channel));
			}
			return TRUE;
		}

		/* It should not happened! */
		DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Didn't found valid channel range, Ch index=%d, ChListNum=%d, CtrlCh=%d\n", 
									index, pAd->ChannelListNum, pAd->CommonCfg.Channel));
	}

	return FALSE;
}


VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd)
{
	BOOLEAN needFallBack = FALSE;
	UCHAR Channel = pAd->CommonCfg.Channel;
	INT chStartIdx, chEndIdx, index,curPriChIdx, curSecChIdx;

	if ((!WMODE_CAP_N(pAd->CommonCfg.PhyMode)) || (pAd->CommonCfg.Channel > 14)) {
		DBGPRINT(RT_DEBUG_TRACE, ("The PhyMode=%d, Channel=%d didn't need channel adjustment!\n", 
				pAd->CommonCfg.PhyMode, pAd->CommonCfg.Channel));
		return;
	}

	/* Build the effected channel list, if something wrong, return directly. */
#ifdef A_BAND_SUPPORT
	if (pAd->CommonCfg.Channel > 14)
	{	/* For 5GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{
			curPriChIdx = index;
			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				chStartIdx = index;
				chEndIdx = chStartIdx + 1;
				curSecChIdx = chEndIdx;
			}
			else
			{
				chStartIdx = index - 1;
				chEndIdx = index;
				curSecChIdx = chStartIdx;
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("5GHz: Cannot found the ControlChannel(%d) in ChannelList, something wrong?\n", 
						pAd->CommonCfg.Channel));
			return;
		}
	}
	else
#endif /* A_BAND_SUPPORT */		
	{	/* For 2.4GHz band */
		for (index = 0; index < pAd->ChannelListNum; index++)
		{
			if(pAd->ChannelList[index].Channel == pAd->CommonCfg.Channel)
				break;
		}

		if (index < pAd->ChannelListNum)
		{

			if(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
			{
				curPriChIdx = index;
				curSecChIdx = ((index + 4) < pAd->ChannelListNum) ? (index + 4) : (pAd->ChannelListNum - 1);
				
				chStartIdx = (curPriChIdx >= 3) ? (curPriChIdx - 3) : 0;
				chEndIdx = ((curSecChIdx + 3) < pAd->ChannelListNum) ? (curSecChIdx + 3) : (pAd->ChannelListNum - 1);
			}
			else
			{
				curPriChIdx = index;
				curSecChIdx = ((index - 4) >=0 ) ? (index - 4) : 0;
				chStartIdx =(curSecChIdx >= 3) ? (curSecChIdx - 3) : 0;
				chEndIdx =  ((curPriChIdx + 3) < pAd->ChannelListNum) ? (curPriChIdx + 3) : (pAd->ChannelListNum - 1);
			}
		}
		else
		{
			/* It should not happened! */
			DBGPRINT(RT_DEBUG_ERROR, ("2.4GHz: Cannot found the Control Channel(%d) in ChannelList, something wrong?\n", 
						pAd->CommonCfg.Channel));
			return;
		}
	}

{
	BSS_COEX_CH_RANGE  coexChRange;
	GetBssCoexEffectedChRange(pAd, &coexChRange);
}

	/* Before we do the scanning, clear the bEffectedChannel as zero for latter use. */
	for (index = 0; index < pAd->ChannelListNum; index++)
		pAd->ChannelList[index].bEffectedChannel = 0;
	
	pAd->CommonCfg.BssCoexApCnt = 0;	
		
	/* If we are not ready for Tx/Rx Pakcet, enable it now for receiving Beacons. */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP) == 0)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Card still not enable Tx/Rx, enable it now!\n"));
#ifdef RTMP_MAC_PCI
		/* Enable Interrupt */
		rtmp_irq_init(pAd);

		RTMP_IRQ_ENABLE(pAd);
#endif /* RTMP_MAC_PCI */


		/* Now Enable RxTx */
		RTMPEnableRxTx(pAd);

		/* rtmp_rx_done_handle() API will check this flag to decide accept incoming packet or not. */
		/* Set the flag be ready to receive Beacon frame for autochannel select. */
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_START_UP);
	}


	DBGPRINT(RT_DEBUG_TRACE, ("Ready to do passive scanning for Channel[%d] to Channel[%d]!\n", 
			pAd->ChannelList[chStartIdx].Channel, pAd->ChannelList[chEndIdx].Channel));
	
	/* Now start to do the passive scanning. */
	pAd->CommonCfg.bOverlapScanning = TRUE;
	for (index = chStartIdx; index<=chEndIdx; index++)
	{
		Channel = pAd->ChannelList[index].Channel;
		AsicSetChannel(pAd, Channel, BW_20,  EXTCHA_NONE, TRUE);

		DBGPRINT(RT_DEBUG_ERROR, ("SYNC - BBP R4 to 20MHz.l\n"));
		/*DBGPRINT(RT_DEBUG_TRACE, ("Passive scanning for Channel %d.....\n", Channel)); */
		OS_WAIT(300); /* wait for 200 ms at each channel. */
	}
	pAd->CommonCfg.bOverlapScanning = FALSE;	
	
	/* After scan all relate channels, now check the scan result to find out if we need fallback to 20MHz. */
	for (index = chStartIdx; index <= chEndIdx; index++)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Channel[Idx=%d, Ch=%d].bEffectedChannel=0x%x!\n", 
					index, pAd->ChannelList[index].Channel, pAd->ChannelList[index].bEffectedChannel));
		if ((pAd->ChannelList[index].bEffectedChannel & (EFFECTED_CH_PRIMARY | EFFECTED_CH_LEGACY))  && (index != curPriChIdx) )
		{
			needFallBack = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=TRUE due to OP/OT!\n"));
		}
		if ((pAd->ChannelList[index].bEffectedChannel & EFFECTED_CH_SECONDARY)  && (index != curSecChIdx))
		{
			needFallBack = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("needFallBack=TRUE due to OS!\n"));
		}
	}
	
	/* If need fallback, now do it. */
	if ((needFallBack == TRUE)
		&& (pAd->CommonCfg.BssCoexApCnt > pAd->CommonCfg.BssCoexApCntThr)
	)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
		//pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = 0;
	}

	/* Recover the bandwidth to support 20/40Mhz if the original setting does support that, and no need to fallback */
	if ((needFallBack == FALSE)
		&& (pAd->CommonCfg.ori_bw_before_2040_coex == BW_40)) {
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = pAd->CommonCfg.ori_bw_before_2040_coex;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = pAd->CommonCfg.ori_ext_channel_before_2040_coex;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 0;
		pAd->CommonCfg.Bss2040CoexistFlag &= (~BSS_2040_COEXIST_INFO_SYNC);
		pAd->CommonCfg.Bss2040NeedFallBack = 0;
		pAd->CommonCfg.RegTransmitSetting.field.EXTCHA = pAd->CommonCfg.ori_ext_channel_before_2040_coex;
		DBGPRINT(RT_DEBUG_ERROR, ("rollback the bandwidth setting to support 20/40Mhz\n"));
	}

	return;	
}
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef DOT1X_SUPPORT
/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to notify 802.1x daemon. This is a internal command

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_InternalCmdAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry,
    IN	UINT8			cmd)
{
	INT				apidx = MAIN_MBSSID;	
	UCHAR 			RalinkIe[9] = {221, 7, 0x00, 0x0c, 0x43, 0x00, 0x00, 0x00, 0x00};
	UCHAR			s_addr[MAC_ADDR_LEN];
	UCHAR			EAPOL_IE[] = {0x88, 0x8e};
	UINT8			frame_len = LENGTH_802_3 + sizeof(RalinkIe);
	UCHAR			FrameBuf[frame_len];
	UINT8			offset = 0;
	
	/* Init the frame buffer */
	NdisZeroMemory(FrameBuf, frame_len);
	
	if (pEntry)
	{
		apidx = pEntry->apidx;
		NdisMoveMemory(s_addr, pEntry->Addr, MAC_ADDR_LEN);
	}
	else
	{
		/* Fake a Source Address for transmission */
		NdisMoveMemory(s_addr, pAd->ApCfg.MBSSID[apidx].wdev.bssid, MAC_ADDR_LEN);
		s_addr[0] |= 0x80;
	}

	/* Assign internal command for Ralink dot1x daemon */
	RalinkIe[5] = cmd;

	/* Prepare the 802.3 header */
	MAKE_802_3_HEADER(FrameBuf, 
					  pAd->ApCfg.MBSSID[apidx].wdev.bssid, 
					  s_addr, 
					  EAPOL_IE);
	offset += LENGTH_802_3;
		
	/* Prepare the specific header of internal command */
	NdisMoveMemory(&FrameBuf[offset], RalinkIe, sizeof(RalinkIe));

	/* Report to upper layer */
	if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
		return FALSE;	

	DBGPRINT(RT_DEBUG_TRACE, ("%s done. (cmd=%d)\n", __FUNCTION__, cmd));

	return TRUE;
}

/*
 ========================================================================
 Routine Description:
    Send Leyer 2 Frame to trigger 802.1x EAP state machine.

 Arguments:

 Return Value:
    TRUE - send successfully
    FAIL - send fail

 Note:
 ========================================================================
*/
BOOLEAN DOT1X_EapTriggerAction(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry)
{
	INT				apidx = MAIN_MBSSID;
	UCHAR 			eapol_start_1x_hdr[4] = {0x01, 0x01, 0x00, 0x00};
	UINT8			frame_len = LENGTH_802_3 + sizeof(eapol_start_1x_hdr);
	UCHAR			FrameBuf[frame_len + 32];
	UINT8			offset = 0;

    if((pEntry->AuthMode == Ndis802_11AuthModeWPA) || (pEntry->AuthMode == Ndis802_11AuthModeWPA2) || (pAd->ApCfg.MBSSID[apidx].wdev.IEEE8021X == TRUE))
	{
		/* Init the frame buffer */
		NdisZeroMemory(FrameBuf, frame_len);

		/* Assign apidx */
		apidx = pEntry->apidx;

		/* Prepare the 802.3 header */
		MAKE_802_3_HEADER(FrameBuf, pAd->ApCfg.MBSSID[apidx].wdev.bssid, pEntry->Addr, EAPOL);
		offset += LENGTH_802_3;

		/* Prepare a fake eapol-start body */
		NdisMoveMemory(&FrameBuf[offset], eapol_start_1x_hdr, sizeof(eapol_start_1x_hdr));

#ifdef CONFIG_HOTSPOT_R2
		if (pEntry)
		{
			MULTISSID_STRUCT *pMbss = pEntry->pMbss;
			if ((pMbss->HotSpotCtrl.HotSpotEnable == 1) && (pMbss->wdev.AuthMode == Ndis802_11AuthModeWPA2) 
			&& (pEntry->hs_info.ppsmo_exist == 1))
			{
				UCHAR HS2_Header[4] = {0x50,0x6f,0x9a,0x12};
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)], HS2_Header, 4);
				memcpy(&FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4], &pEntry->hs_info, sizeof(struct _sta_hs_info));
				frame_len += 4+sizeof(struct _sta_hs_info);
				printk("event eapol start, %x:%x:%x:%x\n", 
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+4],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+5], 
						FrameBuf[offset+sizeof(eapol_start_1x_hdr)+6],FrameBuf[offset+sizeof(eapol_start_1x_hdr)+7]);
				}
		}
#endif	

		/* Report to upper layer */
		if (RTMP_L2_FRAME_TX_ACTION(pAd, apidx, FrameBuf, frame_len) == FALSE)
			return FALSE;			

		DBGPRINT(RT_DEBUG_TRACE, ("Notify 8021.x daemon to trigger EAP-SM for this sta(%02x:%02x:%02x:%02x:%02x:%02x)\n", PRINT_MAC(pEntry->Addr)));

	}

	return TRUE;
}

#endif /* DOT1X_SUPPORT */

