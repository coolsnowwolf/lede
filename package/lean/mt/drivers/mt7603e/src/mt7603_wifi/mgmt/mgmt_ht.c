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


#ifdef DOT11_N_SUPPORT


INT ht_mode_adjust(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, HT_CAPABILITY_IE *peer, RT_HT_CAPABILITY *my)
{
	if ((peer->HtCapInfo.GF) && (my->GF))
	{
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTGREENFIELD;
	}
	else
	{	
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pAd->CommonCfg.AddHTInfo.AddHtInfo2.NonGfPresent = 1;
		pAd->MacTab.fAnyStationNonGF = TRUE;
	}
	if ((peer->HtCapInfo.ChannelWidth) && (my->ChannelWidth) && (pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth == 1))
	{
		pEntry->MaxHTPhyMode.field.BW= BW_40;
		pEntry->MaxHTPhyMode.field.ShortGI = ((my->ShortGIfor40) & (peer->HtCapInfo.ShortGIfor40));
	}
	else
	{	
		pEntry->MaxHTPhyMode.field.BW = BW_20;
		pEntry->MaxHTPhyMode.field.ShortGI = ((my->ShortGIfor20) & (peer->HtCapInfo.ShortGIfor20));
		pAd->MacTab.fAnyStation20Only = TRUE;
	}
				
	return TRUE;
}


INT set_ht_fixed_mcs(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, UCHAR fixed_mcs, UCHAR mcs_bound)
{
	if (fixed_mcs == 32)
	{
		/* Fix MCS as HT Duplicated Mode */
		pEntry->MaxHTPhyMode.field.BW = 1;
		pEntry->MaxHTPhyMode.field.MODE = MODE_HTMIX;
		pEntry->MaxHTPhyMode.field.STBC = 0;
		pEntry->MaxHTPhyMode.field.ShortGI = 0;
		pEntry->MaxHTPhyMode.field.MCS = 32;
	}
	else if (pEntry->MaxHTPhyMode.field.MCS > mcs_bound)
	{
		/* STA supports fixed MCS */
		pEntry->MaxHTPhyMode.field.MCS = mcs_bound;
	}

	return TRUE;
}


INT get_ht_max_mcs(RTMP_ADAPTER *pAd, UCHAR *desire_mcs, UCHAR *cap_mcs)
{
	INT i, j;
	UCHAR bitmask;


	for (i=23; i>=0; i--)
	{	
		j = i/8;	
		bitmask = (1<<(i-(j*8)));
		if ((desire_mcs[j] & bitmask) && (cap_mcs[j] & bitmask))
		{
			/*pEntry->MaxHTPhyMode.field.MCS = i; */
			/* find it !!*/
			break;
		}
		if (i==0)
			break;
	}

	return i;
}


INT get_ht_cent_ch(RTMP_ADAPTER *pAd, UINT8 *rf_bw, UINT8 *ext_ch)
{
	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
		(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_ABOVE)
	)
	{
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_ABOVE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel + 2;
	}
	else if ((pAd->CommonCfg.Channel > 2) && 
			(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth  == BW_40) && 
			(pAd->CommonCfg.RegTransmitSetting.field.EXTCHA == EXTCHA_BELOW))
	{
		*rf_bw = BW_40;
		*ext_ch = EXTCHA_BELOW;
		if (pAd->CommonCfg.Channel == 14)
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 1;
		else
			pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel - 2;
	} else {
		return FALSE;
	}

	return TRUE;
}


UCHAR get_cent_ch_by_htinfo(
	RTMP_ADAPTER *pAd,
	ADD_HT_INFO_IE *ht_op,
	HT_CAPABILITY_IE *ht_cap)
{
	UCHAR cent_ch;
	
	if ((ht_op->ControlChan > 2)&&
		(ht_op->AddHtInfo.ExtChanOffset == EXTCHA_BELOW) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan - 2;
	else if ((ht_op->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE) &&
		(ht_cap->HtCapInfo.ChannelWidth == BW_40))
		cent_ch = ht_op->ControlChan + 2;
	else
		cent_ch = ht_op->ControlChan;
	
	return cent_ch;
}


VOID set_sta_ht_cap(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *ent, HT_CAPABILITY_IE *ht_ie)
{
	if (ht_ie->HtCapInfo.ShortGIfor20)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_SGI20_CAPABLE);

	if (ht_ie->HtCapInfo.ShortGIfor40)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_SGI40_CAPABLE);

	if (ht_ie->HtCapInfo.TxSTBC)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_TxSTBC_CAPABLE);

	if (ht_ie->HtCapInfo.RxSTBC)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RxSTBC_CAPABLE);

	if (ht_ie->ExtHtCapInfo.PlusHTC) {
		CLIENT_CAP_SET_FLAG(ent, fCLIENT_STATUS_HTC_CAPABLE);

		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_HTC_CAPABLE);
	}

	if (pAd->CommonCfg.bRdg && ht_ie->ExtHtCapInfo.RDGSupport)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_RDG_CAPABLE);

	if (ht_ie->ExtHtCapInfo.MCSFeedback == 0x03)
		CLIENT_STATUS_SET_FLAG(ent, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);	

}


/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID RTMPSetHT(
	IN RTMP_ADAPTER *pAd,
	IN OID_SET_HT_PHYMODE *pHTPhyMode)
{
	UCHAR RxStream = pAd->CommonCfg.RxStream;
#ifdef CONFIG_AP_SUPPORT
	INT apidx;
#endif /* CONFIG_AP_SUPPORT */

	INT bw;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;
	HT_CAPABILITY_IE *ht_cap= &pAd->CommonCfg.HtCapability;
	
#ifdef CONFIG_AP_SUPPORT
	/* sanity check for extention channel */
	if (CHAN_PropertyCheck(pAd, pAd->CommonCfg.Channel,
						CHANNEL_NO_FAT_BELOW | CHANNEL_NO_FAT_ABOVE) == TRUE)
	{
		/* only 20MHz is allowed */
		pHTPhyMode->BW = 0;
	}
	else if (pHTPhyMode->ExtOffset == EXTCHA_BELOW)
	{
		/* extension channel below this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pAd->CommonCfg.Channel,
						CHANNEL_NO_FAT_BELOW) == TRUE)
		{
			pHTPhyMode->ExtOffset = EXTCHA_ABOVE;
		}
	}
	else if (pHTPhyMode->ExtOffset == EXTCHA_ABOVE)
	{
		/* extension channel above this channel is not allowed */
		if (CHAN_PropertyCheck(pAd, pAd->CommonCfg.Channel,
						CHANNEL_NO_FAT_ABOVE) == TRUE)
		{
			pHTPhyMode->ExtOffset = EXTCHA_BELOW;
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	//
	// Added to provide sync between Bandwidth and Extended channel settings
	//
	if(pHTPhyMode->BW && pHTPhyMode->ExtOffset == EXTCHA_NONE)
			pHTPhyMode->BW = 0;
		
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : HT_mode(%d), ExtOffset(%d), MCS(%d), BW(%d), STBC(%d), SHORTGI(%d)\n",
										pHTPhyMode->HtMode, pHTPhyMode->ExtOffset, 
										pHTPhyMode->MCS, pHTPhyMode->BW,
										pHTPhyMode->STBC, pHTPhyMode->SHORTGI));
			
	/* Don't zero supportedHyPhy structure.*/
	RTMPZeroMemory(ht_cap, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(&pAd->CommonCfg.AddHTInfo, sizeof(pAd->CommonCfg.AddHTInfo));
	RTMPZeroMemory(&pAd->CommonCfg.NewExtChanOffset, sizeof(pAd->CommonCfg.NewExtChanOffset));
	RTMPZeroMemory(rt_ht_cap, sizeof(RT_HT_CAPABILITY));

   	if (pAd->CommonCfg.bRdg)
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 1;
		ht_cap->ExtHtCapInfo.RDGSupport = 1;
	}
	else
	{
		ht_cap->ExtHtCapInfo.PlusHTC = 0;
		ht_cap->ExtHtCapInfo.RDGSupport = 0;
	}

	ht_cap->HtCapParm.MaxRAmpduFactor = pAd->chipCap.AMPDUFactor;
	rt_ht_cap->MaxRAmpduFactor = pAd->chipCap.AMPDUFactor;

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : RxBAWinLimit = %d\n", pAd->CommonCfg.BACapability.field.RxBAWinLimit));

	/* Mimo power save, A-MSDU size, */
	rt_ht_cap->AmsduEnable = (USHORT)pAd->CommonCfg.BACapability.field.AmsduEnable;
	rt_ht_cap->AmsduSize = (UCHAR)pAd->CommonCfg.BACapability.field.AmsduSize;
	rt_ht_cap->MimoPs = (UCHAR)pAd->CommonCfg.BACapability.field.MMPSmode;
	rt_ht_cap->MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;

	ht_cap->HtCapInfo.AMsduSize = (USHORT)pAd->CommonCfg.BACapability.field.AmsduSize;
	ht_cap->HtCapInfo.MimoPs = (USHORT)pAd->CommonCfg.BACapability.field.MMPSmode;
	
	if (pAd->CommonCfg.ht_ldpc && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC))
		ht_cap->HtCapInfo.ht_rx_ldpc = 1;
	else
		ht_cap->HtCapInfo.ht_rx_ldpc = 0;
	
	ht_cap->HtCapParm.MpduDensity = (UCHAR)pAd->CommonCfg.BACapability.field.MpduDensity;
	
	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetHT : AMsduSize = %d, MimoPs = %d, MpduDensity = %d, MaxRAmpduFactor = %d\n", 
													rt_ht_cap->AmsduSize, 
													rt_ht_cap->MimoPs,
													rt_ht_cap->MpduDensity,
													rt_ht_cap->MaxRAmpduFactor));
	
	if(pHTPhyMode->HtMode == HTMODE_GF)
	{
		ht_cap->HtCapInfo.GF = 1;
		rt_ht_cap->GF = 1;
	}
	else
		rt_ht_cap->GF = 0;
	
	/* Decide Rx MCSSet*/
	switch (RxStream)
	{
		case 3:
			ht_cap->MCSSet[2] =  0xff;
		case 2:
			ht_cap->MCSSet[1] =  0xff;
		case 1:
		default:
			ht_cap->MCSSet[0] =  0xff;
			break;
	}

	if (pAd->CommonCfg.bForty_Mhz_Intolerant && (pHTPhyMode->BW == BW_40))
	{
		pHTPhyMode->BW = BW_20;
		ht_cap->HtCapInfo.Forty_Mhz_Intolerant = 1;
	}

	// TODO: shiang-6590, how about the "bw" when channel 14 for JP region???
	//CFG_TODO
	bw = BW_20;
	if(pHTPhyMode->BW == BW_40)
	{
		ht_cap->MCSSet[4] = 0x1; /* MCS 32*/
		ht_cap->HtCapInfo.ChannelWidth = 1;
		if (pAd->CommonCfg.Channel <= 14) 		
			ht_cap->HtCapInfo.CCKmodein40 = 1;

		rt_ht_cap->ChannelWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 1;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = (pHTPhyMode->ExtOffset == EXTCHA_BELOW)? (EXTCHA_BELOW): EXTCHA_ABOVE;
		/* Set Regsiter for extension channel position.*/

#if defined(RTMP_MAC) || defined(RLT_MAC)
		if (pAd->chipCap.hif_type == HIF_RTMP || pAd->chipCap.hif_type == HIF_RLT)
			rtmp_mac_set_ctrlch(pAd, pHTPhyMode->ExtOffset);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
			mt_mac_set_ctrlch(pAd, pHTPhyMode->ExtOffset);
#endif /* MT_MAC */

		/* Turn on BBP 40MHz mode now only as AP . */
		/* Sta can turn on BBP 40MHz after connection with 40MHz AP. Sta only broadcast 40MHz capability before connection.*/
		if ((pAd->OpMode == OPMODE_AP) || INFRA_ON(pAd) || ADHOC_ON(pAd)
		)
		{
			bbp_set_ctrlch(pAd, pHTPhyMode->ExtOffset);	
#ifdef GREENAP_SUPPORT
			if (pAd->ApCfg.bGreenAPActive == 1)
				bw = BW_20;
			else
#endif /* GREENAP_SUPPORT */
				bw = BW_40;
		}
	}
	else
	{
		ht_cap->HtCapInfo.ChannelWidth = 0;
		rt_ht_cap->ChannelWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
		/* Turn on BBP 20MHz mode by request here.*/
		bw = BW_20;
	}

#ifdef DOT11_VHT_AC
	if (pHTPhyMode->BW == BW_40 &&
		pAd->CommonCfg.vht_bw == VHT_BW_80 && 
		pAd->CommonCfg.vht_cent_ch)
		bw = BW_80;
#endif /* DOT11_VHT_AC */

	bbp_set_bw(pAd, bw);


	if(pHTPhyMode->STBC == STBC_USE)
	{
		if (pAd->Antenna.field.TxPath >= 2)
		{
			ht_cap->HtCapInfo.TxSTBC = 1;
			rt_ht_cap->TxSTBC = 1;
		}
		else
		{
			ht_cap->HtCapInfo.TxSTBC = 0;
			rt_ht_cap->TxSTBC = 0; 	
		}
		
		/*
			RxSTBC
				0: not support,
				1: support for 1SS
				2: support for 1SS, 2SS
				3: support for 1SS, 2SS, 3SS
		*/
		if (pAd->Antenna.field.RxPath >= 1)
		{
			ht_cap->HtCapInfo.RxSTBC = 1;
			rt_ht_cap->RxSTBC = 1;
		}
		else
		{
			ht_cap->HtCapInfo.RxSTBC = 0; 
			rt_ht_cap->RxSTBC = 0; 	
		}
	}
	else
	{
		rt_ht_cap->TxSTBC = 0;
		rt_ht_cap->RxSTBC = 0;
	}

	if(pHTPhyMode->SHORTGI == GI_400)
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 1;
		//ht_cap->HtCapInfo.ShortGIfor40 = 1;
		rt_ht_cap->ShortGIfor20 = 1;
		if(pHTPhyMode->BW == BW_40)
		{
			ht_cap->HtCapInfo.ShortGIfor40 = 1;
			rt_ht_cap->ShortGIfor40 = 1;
		}
		else
		{
			ht_cap->HtCapInfo.ShortGIfor40 = 0;
			rt_ht_cap->ShortGIfor40 = 0;		
		}
	}
	else
	{
		ht_cap->HtCapInfo.ShortGIfor20 = 0;
		ht_cap->HtCapInfo.ShortGIfor40 = 0;
		rt_ht_cap->ShortGIfor20 = 0;
		rt_ht_cap->ShortGIfor40 = 0;
	}
	
#ifdef DYNAMIC_RX_RATE_ADJ
	UpdateSuppHTRateBitmap(pAd);
#endif /* DYNAMIC_RX_RATE_ADJ */

	/* We support link adaptation for unsolicit MCS feedback, set to 2.*/
	pAd->CommonCfg.AddHTInfo.ControlChan = pAd->CommonCfg.Channel;
	/* 1, the extension channel above the control channel. */
	
	/* EDCA parameters used for AP's own transmission*/
	if (pAd->CommonCfg.APEdcaParm.bValid == FALSE)
		set_default_ap_edca_param(pAd);
	AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm);


#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (apidx = 0; apidx < pAd->ApCfg.BssidNum; apidx++)
			RTMPSetIndividualHT(pAd, apidx);
#ifdef WDS_SUPPORT
		for (apidx = 0; apidx < MAX_WDS_ENTRY; apidx++)
			RTMPSetIndividualHT(pAd, apidx + MIN_NET_DEVICE_FOR_WDS);
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (apidx = 0; apidx < MAX_APCLI_NUM; apidx++)
			RTMPSetIndividualHT(pAd, apidx + MIN_NET_DEVICE_FOR_APCLI);
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


}

/*
	========================================================================
	Routine Description:
		Caller ensures we has 802.11n support.
		Calls at setting HT from AP/STASetinformation

	Arguments:
		pAd - Pointer to our adapter
		phymode  - 

	========================================================================
*/
VOID RTMPSetIndividualHT(RTMP_ADAPTER *pAd, UCHAR apidx)
{	
	RT_PHY_INFO *pDesired_ht_phy = NULL;
	UCHAR TxStream = pAd->CommonCfg.TxStream;		
	UCHAR DesiredMcs = MCS_AUTO;
	UCHAR encrypt_mode = Ndis802_11EncryptionDisabled;
	struct wifi_dev *wdev;
		
	do
	{



#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT	
			if (apidx >= MIN_NET_DEVICE_FOR_APCLI)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_APCLI;
						
				if (idx < MAX_APCLI_NUM)
				{
					pDesired_ht_phy = &pAd->ApCfg.ApCliTab[idx].wdev.DesiredHtPhyInfo;
					DesiredMcs = pAd->ApCfg.ApCliTab[idx].wdev.DesiredTransmitSetting.field.MCS;
					encrypt_mode = pAd->ApCfg.ApCliTab[idx].wdev.WepStatus;
					pAd->ApCfg.ApCliTab[idx].wdev.bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid idx(%d)\n", idx));
					return;
				}
			}
#endif /* APCLI_SUPPORT */

		IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
		{
#ifdef WDS_SUPPORT
			if (apidx >= MIN_NET_DEVICE_FOR_WDS)
			{				
				UCHAR	idx = apidx - MIN_NET_DEVICE_FOR_WDS;
						
				if (idx < MAX_WDS_ENTRY)
				{
					pDesired_ht_phy = &pAd->WdsTab.WdsEntry[idx].wdev.DesiredHtPhyInfo;									
					DesiredMcs = pAd->WdsTab.WdsEntry[idx].wdev.DesiredTransmitSetting.field.MCS;							
					/*encrypt_mode = pAd->WdsTab.WdsEntry[idx].WepStatus;*/
					pAd->WdsTab.WdsEntry[idx].wdev.bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
					break;
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
					return;
				}
			}
#endif /* WDS_SUPPORT */
			if ((apidx < pAd->ApCfg.BssidNum) && (apidx < MAX_MBSSID_NUM(pAd)) && (apidx < HW_BEACON_MAX_NUM))
			{
				wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

				pDesired_ht_phy = &wdev->DesiredHtPhyInfo;
				DesiredMcs = wdev->DesiredTransmitSetting.field.MCS;
#ifdef WFA_VHT_PF
				// TODO: Sigma, this code segment used to work around for Sigma Automation!
				if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && (DesiredMcs != MCS_AUTO)) {
					DesiredMcs += ((TxStream - 1) << 4);
					pAd->ApCfg.MBSSID[apidx].DesiredTransmitSetting.field.FixedTxMode = FIXED_TXMODE_VHT;
					RT_CfgSetAutoFallBack(pAd, "0");
				} else {
					RT_CfgSetAutoFallBack(pAd, "1");
				}
#endif /* WFA_VHT_PF */
				encrypt_mode = wdev->WepStatus;
				pAd->ApCfg.MBSSID[apidx].wdev.bWmmCapable = TRUE;
				wdev->bAutoTxRateSwitch = (DesiredMcs == MCS_AUTO) ? TRUE : FALSE;
				break;
			}

			DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
			return;
		}			
#endif /* CONFIG_AP_SUPPORT */

	} while (FALSE);

	if (pDesired_ht_phy == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("RTMPSetIndividualHT: invalid apidx(%d)\n", apidx));
		return;
	}
	RTMPZeroMemory(pDesired_ht_phy, sizeof(RT_PHY_INFO));

	DBGPRINT(RT_DEBUG_TRACE, ("RTMPSetIndividualHT : Desired MCS = %d\n", DesiredMcs));
	/* Check the validity of MCS */
	if ((TxStream == 1) && ((DesiredMcs >= MCS_8) && (DesiredMcs <= MCS_15)))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS(%d) is invalid in 1S, reset it as MCS_7\n", DesiredMcs));
		DesiredMcs = MCS_7;		
	}

	if ((pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_20) && (DesiredMcs == MCS_32))
	{
		DBGPRINT(RT_DEBUG_WARN, ("RTMPSetIndividualHT: MCS_32 is only supported in 40-MHz, reset it as MCS_0\n"));
		DesiredMcs = MCS_0;		
	}
	   		
	/*
		WFA recommend to restrict the encryption type in 11n-HT mode.
	 	So, the WEP and TKIP are not allowed in HT rate. 
	*/
	if (pAd->CommonCfg.HT_DisallowTKIP && IS_INVALID_HT_SECURITY(encrypt_mode))
	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : Use legacy rate in WEP/TKIP encryption mode (apidx=%d)\n", 
									__FUNCTION__, apidx));
		return;
	}

	if (pAd->CommonCfg.HT_Disable)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s : HT is disabled\n", __FUNCTION__));
		return;
	}
			
	pDesired_ht_phy->bHtEnable = TRUE;
					 
	/* Decide desired Tx MCS*/
	switch (TxStream)
	{
		case 1:
			if (DesiredMcs == MCS_AUTO)
				pDesired_ht_phy->MCSSet[0]= 0xff;
			else if (DesiredMcs <= MCS_7)
				pDesired_ht_phy->MCSSet[0]= 1<<DesiredMcs;
			break;

		case 2:
			if (DesiredMcs == MCS_AUTO)
			{
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
			}
			else if (DesiredMcs <= MCS_15)
			{
				ULONG mode;
				
				mode = DesiredMcs / 8;
				if (mode < 2)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}			
			break;

		case 3:
			if (DesiredMcs == MCS_AUTO)
			{
				/* MCS0 ~ MCS23, 3 bytes */
				pDesired_ht_phy->MCSSet[0]= 0xff;
				pDesired_ht_phy->MCSSet[1]= 0xff;
				pDesired_ht_phy->MCSSet[2]= 0xff;
			}
			else if (DesiredMcs <= MCS_23)
			{
				ULONG mode;

				mode = DesiredMcs / 8;
				if (mode < 3)
					pDesired_ht_phy->MCSSet[mode] = (1 << (DesiredMcs - mode * 8));
			}
			break;
	}							

	if(pAd->CommonCfg.DesiredHtPhy.ChannelWidth == BW_40)
	{
		if (DesiredMcs == MCS_AUTO || DesiredMcs == MCS_32)
			pDesired_ht_phy->MCSSet[4] = 0x1;
	}

	/* update HT Rate setting */
	if (pAd->OpMode == OPMODE_STA)
	{
			MlmeUpdateHtTxRates(pAd, BSS0);
	}
	else
		MlmeUpdateHtTxRates(pAd, apidx);

#ifdef DOT11_VHT_AC
	if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode)) {
		pDesired_ht_phy->bVhtEnable = TRUE;
		rtmp_set_vht(pAd, pDesired_ht_phy);
	}
#endif /* DOT11_VHT_AC */
}

/*
	========================================================================
	Routine Description:
		Clear the desire HT info per interface
		
	Arguments:
	
	========================================================================
*/
VOID RTMPDisableDesiredHtInfo(RTMP_ADAPTER *pAd)
{
	struct wifi_dev *wdev;

#ifdef CONFIG_AP_SUPPORT
	UINT8 idx = 0;

	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		for (idx = 0; idx < pAd->ApCfg.BssidNum; idx++)
		{
			wdev = &pAd->ApCfg.MBSSID[idx].wdev;
			RTMPZeroMemory(&wdev->DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
		}
#ifdef WDS_SUPPORT
		for (idx = 0; idx < MAX_WDS_ENTRY; idx++)
		{				
			RTMPZeroMemory(&pAd->WdsTab.WdsEntry[idx].wdev.DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
		}
#endif /* WDS_SUPPORT */
#ifdef APCLI_SUPPORT
		for (idx = 0; idx < MAX_APCLI_NUM; idx++)
		{				
			RTMPZeroMemory(&pAd->ApCfg.ApCliTab[idx].wdev.DesiredHtPhyInfo, sizeof(RT_PHY_INFO));
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */


}


INT	SetCommonHT(RTMP_ADAPTER *pAd)
{
	OID_SET_HT_PHYMODE SetHT;

	if (!WMODE_CAP_N(pAd->CommonCfg.PhyMode))
	{
		/* Clear previous HT information */
		RTMPDisableDesiredHtInfo(pAd);
		return FALSE;
	}

#ifdef DOT11_VHT_AC
	SetCommonVHT(pAd);
#endif /* DOT11_VHT_AC */

	SetHT.PhyMode = (RT_802_11_PHY_MODE)pAd->CommonCfg.PhyMode;
	SetHT.TransmitNo = ((UCHAR)pAd->Antenna.field.TxPath);
	SetHT.HtMode = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.HTMODE;
	SetHT.ExtOffset = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.EXTCHA;
	SetHT.MCS = MCS_AUTO;
	SetHT.BW = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.BW;
	SetHT.STBC = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.STBC;
	SetHT.SHORTGI = (UCHAR)pAd->CommonCfg.RegTransmitSetting.field.ShortGI;		

	RTMPSetHT(pAd, &SetHT);

#ifdef DOT11N_DRAFT3
	if(pAd->CommonCfg.bBssCoexEnable && pAd->CommonCfg.Bss2040NeedFallBack)
	{
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
		pAd->CommonCfg.LastBSSCoexist2040.field.BSS20WidthReq = 1;
		pAd->CommonCfg.Bss2040CoexistFlag |= BSS_2040_COEXIST_INFO_SYNC;
		pAd->CommonCfg.Bss2040NeedFallBack = 1;
	}
#endif /* DOT11N_DRAFT3 */

	return TRUE;
}

/*
	========================================================================
	Routine Description:
		Update HT IE from our capability.
		
	Arguments:
		Send all HT IE in beacon/probe rsp/assoc rsp/action frame.
		
	
	========================================================================
*/
VOID RTMPUpdateHTIE(
	IN RT_HT_CAPABILITY	*pRtHt,
	IN UCHAR *pMcsSet,
	OUT HT_CAPABILITY_IE *pHtCapability,
	OUT ADD_HT_INFO_IE *pAddHtInfo)
{
	RTMPZeroMemory(pHtCapability, sizeof(HT_CAPABILITY_IE));
	RTMPZeroMemory(pAddHtInfo, sizeof(ADD_HT_INFO_IE));
	
		pHtCapability->HtCapInfo.ChannelWidth = pRtHt->ChannelWidth;
		pHtCapability->HtCapInfo.MimoPs = pRtHt->MimoPs;
		pHtCapability->HtCapInfo.GF = pRtHt->GF;
		pHtCapability->HtCapInfo.ShortGIfor20 = pRtHt->ShortGIfor20;
		pHtCapability->HtCapInfo.ShortGIfor40 = pRtHt->ShortGIfor40;
		pHtCapability->HtCapInfo.TxSTBC = pRtHt->TxSTBC;
		pHtCapability->HtCapInfo.RxSTBC = pRtHt->RxSTBC;
		pHtCapability->HtCapInfo.AMsduSize = pRtHt->AmsduSize;
		pHtCapability->HtCapParm.MaxRAmpduFactor = pRtHt->MaxRAmpduFactor;
		pHtCapability->HtCapParm.MpduDensity = pRtHt->MpduDensity;

		pAddHtInfo->AddHtInfo.ExtChanOffset = pRtHt->ExtChanOffset ;
		pAddHtInfo->AddHtInfo.RecomWidth = pRtHt->RecomWidth;
		pAddHtInfo->AddHtInfo2.OperaionMode = pRtHt->OperaionMode;
		pAddHtInfo->AddHtInfo2.NonGfPresent = pRtHt->NonGfPresent;
		RTMPMoveMemory(pAddHtInfo->MCSSet, /*pRtHt->MCSSet*/pMcsSet, 4); /* rt2860 only support MCS max=32, no need to copy all 16 uchar.*/
	
        DBGPRINT(RT_DEBUG_TRACE,("RTMPUpdateHTIE <== \n"));
}

#endif /* DOT11_N_SUPPORT */

