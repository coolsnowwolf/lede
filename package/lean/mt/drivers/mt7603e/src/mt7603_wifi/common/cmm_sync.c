/*
 ***************************************************************************
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
 ***************************************************************************

	Module Name:
	cmm_sync.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2004-09-01      modified for rt2561/2661
*/
#include "rt_config.h"

/*BaSizeArray follows the 802.11n definition as MaxRxFactor.  2^(13+factor) bytes. When factor =0, it's about Ba buffer size =8.*/
UCHAR BaSizeArray[4] = {8,16,32,64};

extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_2GHZ[];
extern UINT16 const Country_Region_GroupNum_2GHZ;
extern COUNTRY_REGION_CH_DESC Country_Region_ChDesc_5GHZ[];
extern UINT16 const Country_Region_GroupNum_5GHZ;

/*
	==========================================================================
	Description:
		Update StaCfg->ChannelList[] according to 1) Country Region 2) RF IC type,
		and 3) PHY-mode user selected.
		The outcome is used by driver when doing site survey.

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID BuildChannelList(RTMP_ADAPTER *pAd)
{
	UCHAR i, j, index=0, num=0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PUCHAR pChannelList = NULL;
	PUCHAR pChannelListFlag = NULL;



	NdisZeroMemory(pAd->ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	/* if not 11a-only mode, channel list starts from 2.4Ghz band*/
	if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
	{
		for (i = 0; i < Country_Region_GroupNum_2GHZ; i++)
		{
			if ((pAd->CommonCfg.CountryRegion & 0x7f) ==
				Country_Region_ChDesc_2GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_2GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;

				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("CountryRegion=%d not support", pAd->CommonCfg.CountryRegion));
			return;
		}

		if (num > 0)
		{
			os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

			if (!pChannelList)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelList failed\n", __FUNCTION__));
				return;
			}

			os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

			if (!pChannelListFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelListFlag failed\n", __FUNCTION__));
				os_free_mem(NULL, pChannelList);
				return;
			}

			for (i = 0; i < num; i++)
			{
				pChannelList[i] = GetChannel_2GHZ(pChDesc, i);
				pChannelListFlag[i] = GetChannelFlag(pChDesc, i);
			}

			for (i = 0; i < num; i++)
			{
				for (j = 0; j < MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel)
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
				}

				pAd->ChannelList[index + i].Flags = pChannelListFlag[i];
				/* TODO: shiang-7603, NdisMoveMemory may replace the
				 * pAd->ChannelList[index+i].Channel as other values!
				 */
				pAd->ChannelList[index+i].Channel = pChannelList[i];
				

#ifdef DOT11_N_SUPPORT
						if (N_ChannelGroupCheck(pAd, pAd->ChannelList[index + i].Channel))
							pAd->ChannelList[index + i].Flags |= CHANNEL_40M_CAP;
#ifdef DOT11_VHT_AC
						if (vht80_channel_group(pAd, pAd->ChannelList[index + i].Channel))
							pAd->ChannelList[index + i].Flags |= CHANNEL_80M_CAP;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
				if (!strncmp((RTMP_STRING *) pAd->CommonCfg.CountryCode, "CN", 2))
					pAd->ChannelList[index+i].MaxTxPwr = pAd->MaxTxPwr;/*for CN CountryCode*/
				else
					pAd->ChannelList[index+i].MaxTxPwr = 20;
			}

			index += num;

			os_free_mem(NULL, pChannelList);
			os_free_mem(NULL, pChannelListFlag);
		}
		bRegionFound = FALSE;
		num = 0;
	}

	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
	{
		for (i = 0; i < Country_Region_GroupNum_5GHZ; i++)
		{
			if ((pAd->CommonCfg.CountryRegionForABand & 0x7f) ==
				Country_Region_ChDesc_5GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_5GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;

				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("CountryRegionABand=%d not support", pAd->CommonCfg.CountryRegionForABand));
			return;
		}

		if (num > 0)
		{
			UCHAR RadarCh[15]={52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140};
#ifdef CONFIG_AP_SUPPORT
			UCHAR q=0;
#endif /* CONFIG_AP_SUPPORT */
			os_alloc_mem(NULL, (UCHAR **)&pChannelList, num * sizeof(UCHAR));

			if (!pChannelList)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelList failed\n", __FUNCTION__));
				return;
			}

			os_alloc_mem(NULL, (UCHAR **)&pChannelListFlag, num * sizeof(UCHAR));

			if (!pChannelListFlag)
			{
				DBGPRINT(RT_DEBUG_ERROR,("%s:Allocate memory for ChannelListFlag failed\n", __FUNCTION__));
				os_free_mem(NULL, pChannelList);
				return;
			}

			for (i = 0; i < num; i++)
			{
				pChannelList[i] = GetChannel_5GHZ(pChDesc, i);
				pChannelListFlag[i] = GetChannelFlag(pChDesc, i);
			}

#ifdef CONFIG_AP_SUPPORT
			for (i = 0; i < num; i++)
			{
				if((pAd->CommonCfg.bIEEE80211H == 0)|| ((pAd->CommonCfg.bIEEE80211H == 1) && (pAd->CommonCfg.RDDurRegion != FCC)))
				{
					pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
					pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
					q++;
				}
				/*Based on the requiremnt of FCC, some channles could not be used anymore when test DFS function.*/
				else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
						(pAd->CommonCfg.RDDurRegion == FCC) &&
						(pAd->Dot11_H.bDFSIndoor == 1))
				{
					if((GetChannel_5GHZ(pChDesc, i) < 116) || (GetChannel_5GHZ(pChDesc, i) > 128))
					{
						pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
						pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
						q++;
					}
				}
				else if ((pAd->CommonCfg.bIEEE80211H == 1) &&
						(pAd->CommonCfg.RDDurRegion == FCC) &&
						(pAd->Dot11_H.bDFSIndoor == 0))
				{
					if((GetChannel_5GHZ(pChDesc, i) < 100) || (GetChannel_5GHZ(pChDesc, i) > 140) )
					{
						pChannelList[q] = GetChannel_5GHZ(pChDesc, i);
						pChannelListFlag[q] = GetChannelFlag(pChDesc, i);
						q++;
					}
				}

			}
			num = q;
#endif /* CONFIG_AP_SUPPORT */

			for (i=0; i<num; i++)
			{
				for (j=0; j<MAX_NUM_OF_CHANNELS; j++)
				{
					if (pChannelList[i] == pAd->TxPower[j].Channel) 
						NdisMoveMemory(&pAd->ChannelList[index+i], &pAd->TxPower[j], sizeof(CHANNEL_TX_POWER));
				}
				
				pAd->ChannelList[index + i].Flags = pChannelListFlag[i];
				/* TODO: shiang-7603, NdisMoveMemory may replace the
				 * pAd->ChannelList[index+i].Channel as other values!
				 */
				pAd->ChannelList[index+i].Channel = pChannelList[i];

#ifdef DOT11_N_SUPPORT
				if (N_ChannelGroupCheck(pAd, pAd->ChannelList[index + i].Channel))
					pAd->ChannelList[index + i].Flags |= CHANNEL_40M_CAP;
#ifdef DOT11_VHT_AC
				if (vht80_channel_group(pAd, pAd->ChannelList[index + i].Channel))
					pAd->ChannelList[index + i].Flags |= CHANNEL_80M_CAP;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

				for (j=0; j<15; j++)
				{
					if (pChannelList[i] == RadarCh[j])
						pAd->ChannelList[index+i].DfsReq = TRUE;
				}
				pAd->ChannelList[index+i].MaxTxPwr = 20;
			}
			index += num;

			os_free_mem(NULL, pChannelList);
			os_free_mem(NULL, pChannelListFlag);
		}
	}

	pAd->ChannelListNum = index;
	DBGPRINT(RT_DEBUG_TRACE,("CountryCode(2.4G/5G)=%d/%d, RFIC=%d, PHY mode=%d, support %d channels\n",
		pAd->CommonCfg.CountryRegion, pAd->CommonCfg.CountryRegionForABand, pAd->RfIcType, pAd->CommonCfg.PhyMode, pAd->ChannelListNum));


#ifdef DBG
	DBGPRINT(RT_DEBUG_TRACE, ("SupportedChannelList:\n"));
	for (i=0; i<pAd->ChannelListNum; i++)
	{
		DBGPRINT_RAW(RT_DEBUG_TRACE,("\tChannel # %d: Pwr0/1 = %d/%d, Flags = %x\n ",
									 pAd->ChannelList[i].Channel,
									 pAd->ChannelList[i].Power,
									 pAd->ChannelList[i].Power2,
									 pAd->ChannelList[i].Flags));
	}
#endif
}


#ifdef P2P_CHANNEL_LIST_SEPARATE
VOID P2PBuildChannelList(
	IN PRTMP_ADAPTER pAd)
{
	UCHAR i, index=0, num=0;
	PCH_DESC pChDesc = NULL;
	BOOLEAN bRegionFound = FALSE;
	PCFG80211_CTRL pCfg80211_Ctrl = &pAd->cfg80211_ctrl;

	NdisZeroMemory(pCfg80211_Ctrl->ChannelList, MAX_NUM_OF_CHANNELS * sizeof(CHANNEL_TX_POWER));

	/* if not 11a-only mode, channel list starts from 2.4Ghz band*/
	if (WMODE_CAP_2G(pAd->CommonCfg.PhyMode))
	{
		for (i = 0; i < Country_Region_GroupNum_2GHZ; i++)
		{
			if ((pCfg80211_Ctrl->CountryRegion & 0x7f) ==
				Country_Region_ChDesc_2GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_2GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;
				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s::P2PCountryRegion=%d not support", __FUNCTION__, pCfg80211_Ctrl->CountryRegion));
			return;		
		}

		if (num > 0)
		{
			for (i = 0; i < num; i++)
			{
				pCfg80211_Ctrl->ChannelList[i].Channel = GetChannel_2GHZ(pChDesc, i);
				pCfg80211_Ctrl->ChannelList[i].Flags = GetChannelFlag(pChDesc, i);
			}

			index += num;
		}
		num = 0;
	}

	if (WMODE_CAP_5G(pAd->CommonCfg.PhyMode))
	{
		for (i = 0; i < Country_Region_GroupNum_5GHZ; i++)
		{
			if ((pCfg80211_Ctrl->CountryRegionForABand & 0x7f) ==
				Country_Region_ChDesc_5GHZ[i].RegionIndex)
			{
				pChDesc = Country_Region_ChDesc_5GHZ[i].pChDesc;
				num = TotalChNum(pChDesc);
				bRegionFound = TRUE;
				break;
			}
		}

		if (!bRegionFound)
		{
			DBGPRINT(RT_DEBUG_ERROR,("%s::P2PCountryRegionForABand=%d not support", __FUNCTION__, pCfg80211_Ctrl->CountryRegionForABand));
			return;		
		}

		if (num > 0)
		{
			for (i = 0; i < num; i++)
			{
				pCfg80211_Ctrl->ChannelList[index + i].Channel = GetChannel_5GHZ(pChDesc, i);
				pCfg80211_Ctrl->ChannelList[index + i].Flags = GetChannelFlag(pChDesc, i);
			}

			index += num;
		}
	}

	pCfg80211_Ctrl->ChannelListNum = index;	

	DBGPRINT(RT_DEBUG_TRACE, ("%s::Channel list nubmer = %d\n", __FUNCTION__, pCfg80211_Ctrl->ChannelListNum));
}
#endif /* P2P_CHANNEL_LIST_SEPARATE */



/*
	==========================================================================
	Description:
		This routine return the first channel number according to the country
		code selection and RF IC selection (signal band or dual band). It is called
		whenever driver need to start a site survey of all supported channels.
	Return:
		ch - the first channel number of current country code setting

	IRQL = PASSIVE_LEVEL

	==========================================================================
 */
UCHAR FirstChannel(RTMP_ADAPTER *pAd)
{
	return pAd->ChannelList[0].Channel;
}


/*
	==========================================================================
	Description:
		This routine returns the next channel number. This routine is called
		during driver need to start a site survey of all supported channels.
	Return:
		next_channel - the next channel number valid in current country code setting.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR NextChannel(RTMP_ADAPTER *pAd, UCHAR channel)
{
	int i;
	UCHAR next_channel = 0;

#ifdef P2P_CHANNEL_LIST_SEPARATE
	PCFG80211_CTRL pCfg80211_Ctrl = &pAd->cfg80211_ctrl;
#endif /* P2P_CHANNEL_LIST_SEPARATE */

#ifdef P2P_CHANNEL_LIST_SEPARATE
	if ((pAd->ScanCtrl.ScanType == SCAN_P2P))
	{
		for (i = 0; i < (pCfg80211_Ctrl->ChannelListNum - 1); i++)
		{
			if (channel == pCfg80211_Ctrl->ChannelList[i].Channel)
			{
				next_channel = pCfg80211_Ctrl->ChannelList[i+1].Channel;
				return next_channel;
			}
		}
	}
#endif /* P2P_CHANNEL_LIST_SEPARATE */




	for (i = 0; i < (pAd->ChannelListNum - 1); i++)
	{
		if (channel == pAd->ChannelList[i].Channel)
		{
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
			/* Only scan effected channel if this is a SCAN_2040_BSS_COEXIST*/
			/* 2009 PF#2: Nee to handle the second channel of AP fall into affected channel range.*/
			if ((pAd->ScanCtrl.ScanType == SCAN_2040_BSS_COEXIST) && (pAd->ChannelList[i+1].Channel >14))
			{
				channel = pAd->ChannelList[i+1].Channel;
				continue;
			}
			else
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */
			{
				/* Record this channel's idx in ChannelList array.*/
			next_channel = pAd->ChannelList[i+1].Channel;
			break;
	}
		}
	}
	return next_channel;
}

/* 
	==========================================================================
	Description:

	Return:
		scan_channel - channel to scan.
	Note:
		return 0 if no more next channel
	==========================================================================
 */
UCHAR RTMPFindScanChannel(
	IN PRTMP_ADAPTER pAd, 
	UINT8			  LastScanChannel)
{
	UCHAR scan_channel = 0;
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_PARTIAL_SCAN_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if (pAd->ApCfg.bPartialScanning == TRUE)
		{
			scan_channel = FindPartialScanChannel(pAd);
			return scan_channel;
		}
	}
#endif /* AP_PARTIAL_SCAN_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	if (LastScanChannel == 0)
		scan_channel = FirstChannel(pAd);
	else
		scan_channel = NextChannel(pAd, LastScanChannel);

	return scan_channel;
}

/*
	==========================================================================
	Description:
		This routine is for Cisco Compatible Extensions 2.X
		Spec31. AP Control of Client Transmit Power
	Return:
		None
	Note:
	   Required by Aironet dBm(mW)
		   0dBm(1mW),   1dBm(5mW), 13dBm(20mW), 15dBm(30mW),
		  17dBm(50mw), 20dBm(100mW)

	   We supported
		   3dBm(Lowest), 6dBm(10%), 9dBm(25%), 12dBm(50%),
		  14dBm(75%),   15dBm(100%)

		The client station's actual transmit power shall be within +/- 5dB of
		the minimum value or next lower value.
	==========================================================================
 */
VOID ChangeToCellPowerLimit(RTMP_ADAPTER *pAd, UCHAR AironetCellPowerLimit)
{
	/*
		valud 0xFF means that hasn't found power limit information
		from the AP's Beacon/Probe response
	*/
	if (AironetCellPowerLimit == 0xFF)
		return;

	if (AironetCellPowerLimit < 6) /*Used Lowest Power Percentage.*/
		pAd->CommonCfg.TxPowerPercentage = 6;
	else if (AironetCellPowerLimit < 9)
		pAd->CommonCfg.TxPowerPercentage = 10;
	else if (AironetCellPowerLimit < 12)
		pAd->CommonCfg.TxPowerPercentage = 25;
	else if (AironetCellPowerLimit < 14)
		pAd->CommonCfg.TxPowerPercentage = 50;
	else if (AironetCellPowerLimit < 15)
		pAd->CommonCfg.TxPowerPercentage = 75;
	else
		pAd->CommonCfg.TxPowerPercentage = 100; /*else used maximum*/

	if (pAd->CommonCfg.TxPowerPercentage > pAd->CommonCfg.TxPowerDefault)
		pAd->CommonCfg.TxPowerPercentage = pAd->CommonCfg.TxPowerDefault;

}


CHAR ConvertToRssi(RTMP_ADAPTER *pAd, struct raw_rssi_info *rssi_info, UCHAR rssi_idx)
{
	UCHAR RssiOffset, LNAGain;
	CHAR BaseVal;
	CHAR rssi;

	/* Rssi equals to zero or rssi_idx larger than 3 should be an invalid value*/
	if (rssi_idx >= 3)
		return -99;

	rssi = rssi_info->raw_rssi[rssi_idx];

	if (rssi == 0)
		return -99;

	LNAGain = pAd->hw_cfg.lan_gain;

	if (pAd->LatchRfRegs.Channel > 14)
		RssiOffset = pAd->ARssiOffset[rssi_idx];
	else
		RssiOffset = pAd->BGRssiOffset[rssi_idx];

	BaseVal = -12;

#if defined (MT7603) || defined (MT7628)
	if (IS_MT7603(pAd) || IS_MT7628(pAd))
		return (rssi + (CHAR)RssiOffset - (CHAR)LNAGain);
#endif


		return (BaseVal - RssiOffset - LNAGain - rssi);
}


CHAR ConvertToSnr(RTMP_ADAPTER *pAd, UCHAR Snr)
{
	if (pAd->chipCap.SnrFormula == SNR_FORMULA2)
		return (Snr * 3 + 8) >> 4;
	else if (pAd->chipCap.SnrFormula == SNR_FORMULA3)
		return (Snr * 3 / 16 ); /* * 0.1881 */
	else
		return ((0xeb	- Snr) * 3) /	16 ;
}




#ifdef CONFIG_AP_SUPPORT
#ifdef DOT11_N_SUPPORT
extern int DetectOverlappingPeriodicRound;

VOID Handle_BSS_Width_Trigger_Events(RTMP_ADAPTER *pAd)
{
	ULONG Now32;
#ifdef	WH_EZ_SETUP
	UCHAR i;
#endif
#ifdef DOT11N_DRAFT3
	if (pAd->CommonCfg.bBssCoexEnable == FALSE)
		return;
#endif /* DOT11N_DRAFT3 */

	if ((pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40) &&
		(pAd->CommonCfg.Channel <=14))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("Rcv BSS Width Trigger Event: 40Mhz --> 20Mhz \n"));
        NdisGetSystemUpTime(&Now32);
		pAd->CommonCfg.LastRcvBSSWidthTriggerEventsTime = Now32;
		pAd->CommonCfg.bRcvBSSWidthTriggerEvents = TRUE;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.RecomWidth = 0;
		pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset = 0;
#ifdef WH_EZ_SETUP
		for(i=0;i<WDEV_NUM_MAX;i++)
		{
			struct wifi_dev *wdev;

			wdev = pAd->wdev_list[i];
			if(!wdev /*|| (wdev->channel != pAd->CommonCfg.Channel)*/) // Arvind : assume only one wdev
				continue;
			
#if  defined(EZ_NETWORK_MERGE_SUPPORT)
			if (IS_EZ_SETUP_ENABLED(wdev) && (wdev->wdev_type == WDEV_TYPE_AP)){
				EZ_DEBUG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("\nHandle_BSS_Width_Trigger_Events: do fallback ****\n"));
				ez_set_ap_fallback_context(wdev,TRUE, pAd->CommonCfg.Channel);
			}
#endif /* WH_EZ_SETUP */

		}
#endif /* WH_EZ_SETUP */
		
        DetectOverlappingPeriodicRound = 31;
	}
}
#endif /* DOT11_N_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

