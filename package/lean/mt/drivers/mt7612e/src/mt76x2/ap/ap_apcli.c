/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_apcli.c

    Abstract:
    Support AP-Client function.

    Note:
    1. Call RT28xx_ApCli_Init() in init function and
       call RT28xx_ApCli_Remove() in close function

    2. MAC of ApCli-interface is initialized in RT28xx_ApCli_Init()

    3. ApCli index (0) of different rx packet is got in
       APHandleRxDoneInterrupt() by using FromWhichBSSID = pEntry->apidx;
       Or FromWhichBSSID = BSS0;

    4. ApCli index (0) of different tx packet is assigned in
       rt28xx_send_packets() by using RTMP_SET_PACKET_NET_DEVICE_APCLI()
    5. ApCli index (0) of different interface is got in APHardTransmit() by using
       RTMP_GET_PACKET_IF()

    6. ApCli index (0) of IOCTL command is put in pAd->OS_Cookie->ioctl_if

    8. The number of ApCli only can be 1

	9. apcli convert engine subroutines, we should just take care data packet.
    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang, Fonchi  02-13-2007      created
*/

#ifdef APCLI_SUPPORT

#include "rt_config.h"

BOOLEAN ApCliWaitProbRsp(PRTMP_ADAPTER pAd, USHORT ifIndex)
{
        if (ifIndex >= MAX_APCLI_NUM)
                return FALSE;

        return (pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState == APCLI_JOIN_WAIT_PROBE_RSP) ?
                TRUE : FALSE;
}

VOID ApCliSimulateRecvBeacon(RTMP_ADAPTER *pAd)
{
	INT loop;
        ULONG Now32, BPtoJiffies;
        PAPCLI_STRUCT pApCliEntry = NULL;
	LONG timeDiff;
	
	NdisGetSystemUpTime(&Now32);
        for (loop = 0; loop < MAX_APCLI_NUM; loop++)
        {
        	pApCliEntry = &pAd->ApCfg.ApCliTab[loop];
                if ((pApCliEntry->Valid == TRUE) && (pApCliEntry->MacTabWCID < MAX_LEN_OF_MAC_TABLE))
                {
                	/*
                          When we are connected and do the scan progress, it's very possible we cannot receive
                          the beacon of the AP. So, here we simulate that we received the beacon.
                         */
                        if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
                            (RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1 * OS_HZ))))
                        {
                        	BPtoJiffies = (((pApCliEntry->ApCliBeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);
                                timeDiff = (pAd->Mlme.Now32 - pApCliEntry->ApCliRcvBeaconTime) / BPtoJiffies;
                                if (timeDiff > 0)
                                	pApCliEntry->ApCliRcvBeaconTime += (timeDiff * BPtoJiffies);

                                if (RTMP_TIME_AFTER(pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32))
                                {
                                	DBGPRINT(RT_DEBUG_TRACE, ("MMCHK - APCli BeaconRxTime adjust wrong(BeaconRx=0x%lx, Now=0x%lx)\n",
                                                                   pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32));
                                }
                        }

                        /* update channel quality for Roaming and UI LinkQuality display */
                        MlmeCalculateChannelQuality(pAd, &pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
		}
	}
}

/*
========================================================================
Routine Description:
    Close ApCli network interface.

Arguments:
    ad_p            points to our adapter

Return Value:
    None

Note:
========================================================================
*/
VOID RT28xx_ApCli_Close(RTMP_ADAPTER *ad_p)
{
	UINT index;


	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (ad_p->ApCfg.ApCliTab[index].wdev.if_dev)
			RtmpOSNetDevClose(ad_p->ApCfg.ApCliTab[index].wdev.if_dev);
	}

}


/* --------------------------------- Private -------------------------------- */
INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	SHORT if_idx;

	for(if_idx = 0; if_idx < MAX_APCLI_NUM; if_idx++)
	{
		if(MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[if_idx].wdev.if_addr, pAddr))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s():ApCliIfIndex=%d\n",
						__FUNCTION__, if_idx));
			return if_idx;
		}
	}

	return -1;
}


BOOLEAN isValidApCliIf(SHORT if_idx)
{
	return (((if_idx >= 0) && (if_idx < MAX_APCLI_NUM)) ? TRUE : FALSE);
}


/*! \brief init the management mac frame header
 *  \param p_hdr mac header
 *  \param subtype subtype of the frame
 *  \param p_ds destination address, don't care if it is a broadcast address
 *  \return none
 *  \pre the station has the following information in the pAd->UserCfg
 *   - bssid
 *   - station address
 *  \post
 *  \note this function initializes the following field
 */
VOID ApCliMgtMacHeaderInit(
    IN RTMP_ADAPTER *pAd,
    INOUT HEADER_802_11 *pHdr80211,
    IN UCHAR SubType,
    IN UCHAR ToDs,
    IN UCHAR *pDA,
    IN UCHAR *pBssid,
    IN USHORT ifIndex)
{
    NdisZeroMemory(pHdr80211, sizeof(HEADER_802_11));
    pHdr80211->FC.Type = FC_TYPE_MGMT;
    pHdr80211->FC.SubType = SubType;
    pHdr80211->FC.ToDs = ToDs;
    COPY_MAC_ADDR(pHdr80211->Addr1, pDA);
    COPY_MAC_ADDR(pHdr80211->Addr2, pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
    COPY_MAC_ADDR(pHdr80211->Addr3, pBssid);
}


#ifdef DOT11_N_SUPPORT
/*
	========================================================================

	Routine Description:
		Verify the support rate for HT phy type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
BOOLEAN ApCliCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN USHORT IfIndex,
	INOUT HT_CAPABILITY_IE *pHtCapability,
	INOUT ADD_HT_INFO_IE *pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;
	HT_CAPABILITY_IE *aux_ht_cap;
	RT_HT_CAPABILITY *rt_ht_cap = &pAd->CommonCfg.DesiredHtPhy;


	if (IfIndex >= MAX_APCLI_NUM)
		return FALSE;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];

	aux_ht_cap = &pApCliEntry->MlmeAux.HtCapability;
	aux_ht_cap->MCSSet[0] = 0xff;
	aux_ht_cap->MCSSet[4] = 0x1;
	 switch (pAd->CommonCfg.RxStream)
	{
		case 1:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0x00;
			aux_ht_cap->MCSSet[2] = 0x00;
			aux_ht_cap->MCSSet[3] = 0x00;
			break;
		case 2:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0xff;
            aux_ht_cap->MCSSet[2] = 0x00;
            aux_ht_cap->MCSSet[3] = 0x00;
			break;
		case 3:
			aux_ht_cap->MCSSet[0] = 0xff;
			aux_ht_cap->MCSSet[1] = 0xff;
            aux_ht_cap->MCSSet[2] = 0xff;
            aux_ht_cap->MCSSet[3] = 0x00;
			break;
	}

	/* Record the RxMcs of AP */
	NdisMoveMemory(pApCliEntry->RxMcsSet, pHtCapability->MCSSet, 16);

	/* choose smaller setting */
#ifdef APCLI_AUTO_BW_SUPPORT
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & pApCliEntry->wdev.bw;
#else
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth;
#endif /* APCLI_AUTO_BW_SUPPORT */

	aux_ht_cap->HtCapInfo.GF =  pHtCapability->HtCapInfo.GF & rt_ht_cap->GF;

	/* Send Assoc Req with my HT capability. */
	aux_ht_cap->HtCapInfo.AMsduSize =  rt_ht_cap->AmsduSize;
	aux_ht_cap->HtCapInfo.MimoPs = pHtCapability->HtCapInfo.MimoPs;
	aux_ht_cap->HtCapInfo.ShortGIfor20 =  (rt_ht_cap->ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	aux_ht_cap->HtCapInfo.ShortGIfor40 =  (rt_ht_cap->ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	aux_ht_cap->HtCapInfo.TxSTBC =  (rt_ht_cap->TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	aux_ht_cap->HtCapInfo.RxSTBC =  (rt_ht_cap->RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	aux_ht_cap->HtCapParm.MaxRAmpduFactor = rt_ht_cap->MaxRAmpduFactor;
	aux_ht_cap->HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	aux_ht_cap->ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		aux_ht_cap->ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
	}
	
	if (pAd->CommonCfg.Channel < 14) {
		pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo.RecomWidth = pAddHtInfo->AddHtInfo.RecomWidth;
		pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset = pAddHtInfo->AddHtInfo.ExtChanOffset;
		pApCliEntry->MlmeAux.AddHtInfo.ControlChan = pAddHtInfo->ControlChan;
	}
	
	/*COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability); */
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
BOOLEAN ApCliCheckVht(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op)
{
	VHT_CAP_INFO *vht_cap_info = &vht_cap->vht_cap;

	// TODO: shiang-6590, not finish yet!!!!

	if (Wcid >= MAX_LEN_OF_MAC_TABLE)
		return FALSE;
	DBGPRINT(RT_DEBUG_TRACE, ("============>ApCliCheckVht\n"));
	/* Save Peer Capability*/
	if (vht_cap_info->sgi_80M)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI80_CAPABLE);
	
	if (vht_cap_info->sgi_160M)
		CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_SGI160_CAPABLE);

	if (pAd->CommonCfg.vht_stbc) {
		if (vht_cap_info->tx_stbc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_TXSTBC_CAPABLE);
		if (vht_cap_info->rx_stbc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_RXSTBC_CAPABLE);
	}

	if (pAd->CommonCfg.vht_ldpc && (pAd->chipCap.phy_caps & fPHY_CAP_LDPC)) {
		if (vht_cap_info->rx_ldpc)
			CLIENT_STATUS_SET_FLAG(pEntry, fCLIENT_STATUS_VHT_RX_LDPC_CAPABLE);
	}

	/* Will check ChannelWidth for MCSSet[4] below */
	//NdisZeroMemory(&pApCliEntry->MlmeAux.vht_cap.mcs_set, sizeof(VHT_MCS_SET));
	//pApCliEntry->MlmeAux.vht_cap.mcs_set.rx_high_rate = pAd->CommonCfg.RxStream * 325;
	//pApCliEntry->MlmeAux.vht_cap.mcs_set.tx_high_rate = pAd->CommonCfg.TxStream * 325;

	//pAd->MlmeAux.vht_cap.vht_cap.ch_width = vht_cap_info->ch_width;
#ifdef VHT_TXBF_SUPPORT
    //Disable beamformee capability in Associate Request with 3x3 AP to avoid throughput drop issue
    // MT76x2 only supports up to 2x2 sounding feedback 
    if(IS_MT76x2(pAd))
    {           
        if(vht_cap_info->num_snd_dimension >=2 )
        {
            pAd->MlmeAux.vht_cap.vht_cap.bfee_cap_su = FALSE;
            pAd->MlmeAux.vht_cap.vht_cap.bfee_cap_mu = FALSE;
             
        }
    }
#endif /* TXBF_SUPPORT */	
	pEntry->VhtMaxRAmpduFactor = vht_cap_info->max_ampdu_exp;
	DBGPRINT(RT_DEBUG_WARN, ("%s():pEntry->VhtMaxRAmpduFactor =%d\n",
		__FUNCTION__, pEntry->VhtMaxRAmpduFactor)); 

	return TRUE;
}	
#endif /* DOT11_VHT_AC */

VOID ComposeP2PPsPoll(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry)
{
    	struct wifi_dev *wdev;
       wdev = &pApCliEntry->wdev;

	NdisZeroMemory(&pApCliEntry->PsPollFrame, sizeof (PSPOLL_FRAME));
	pApCliEntry->PsPollFrame.FC.Type = FC_TYPE_CNTL;
	pApCliEntry->PsPollFrame.FC.SubType = SUBTYPE_PS_POLL;
	pApCliEntry->PsPollFrame.Aid = pAd->ApCfg.ApCliTab[0].MlmeAux.Aid | 0xC000;

	COPY_MAC_ADDR(pApCliEntry->PsPollFrame.Bssid, pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid);
	COPY_MAC_ADDR(pApCliEntry->PsPollFrame.Ta, wdev->if_addr);
}

VOID ComposeP2PNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry)
{
   	struct wifi_dev *wdev;
       wdev = &pApCliEntry->wdev;

	NdisZeroMemory(&pApCliEntry->NullFrame, sizeof (HEADER_802_11));
	pApCliEntry->NullFrame.FC.Type = FC_TYPE_DATA;
	pApCliEntry->NullFrame.FC.SubType = SUBTYPE_DATA_NULL;
	pApCliEntry->NullFrame.FC.ToDs = 1;

	COPY_MAC_ADDR(pApCliEntry->NullFrame.Addr1, pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid);
	COPY_MAC_ADDR(pApCliEntry->NullFrame.Addr2, wdev->if_addr);
	COPY_MAC_ADDR(pApCliEntry->NullFrame.Addr3, pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid);
}

/*
    ==========================================================================

	Routine	Description:
		Connected to the BSSID

	Arguments:
		pAd				- Pointer to our adapter
		ApCliIdx		- Which ApCli interface		
	Return Value:		
		FALSE: fail to alloc Mac entry.

	Note:

	==========================================================================
*/
BOOLEAN ApCliLinkUp(RTMP_ADAPTER *pAd, UCHAR ifIndex)
{
	BOOLEAN result = FALSE;
	PAPCLI_STRUCT pApCliEntry = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
	INVAILD_TRIGGER_MAC_ENTRY *pSkipEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef APCLI_CERT_SUPPORT	
	UINT32 Data;
#endif /* APCLI_CERT_SUPPORT */
	UCHAR   PhyMode = pAd->CommonCfg.PhyMode;
#ifdef MWDS
	UCHAR ApCliIdx = 0; /* default use apcli0 */
#endif /* MWDS */	
	do
	{
#ifdef MWDS
		if (ifIndex < MAX_APCLI_NUM)
			ApCliIdx = ifIndex;
		else if(ifIndex >= 64)
			ApCliIdx = ((ifIndex - 64) / 16);
#endif /* MWDS */

		if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			|| (ifIndex >= 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		{
#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef MWDS
				&& ((pAd->ApCfg.ApCliTab[ApCliIdx].bSupportMWDS == FALSE) ||
					(pAd->ApCfg.ApCliTab[ApCliIdx].MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */			
			)
			{
				if (ifIndex < MAX_APCLI_NUM)
				{
#ifdef LINUX				
					struct net_device *pNetDev;
					struct net *net= &init_net;

					for_each_netdev(net, pNetDev)
					{
						if (pNetDev->priv_flags == IFF_EBRIDGE)
						{
							COPY_MAC_ADDR(pAd->ApCfg.BridgeAddress, pNetDev->dev_addr);
							DBGPRINT(RT_DEBUG_ERROR, (" Bridge Addr = %02X:%02X:%02X:%02X:%02X:%02X. !!!\n",
										PRINT_MAC(pAd->ApCfg.BridgeAddress)));

						}
						pSkipEntry = RepeaterInvaildMacLookup(pAd, pNetDev->dev_addr);

						if (pSkipEntry == NULL)
							RTMPRepeaterInsertInvaildMacEntry(pAd, pNetDev->dev_addr);
					}
#endif /* LINUX */
				}

				if (ifIndex >= 64)
				{
					CliIdx = ((ifIndex - 64) % 16);
					ifIndex = ((ifIndex - 64) / 16);

					pMacEntry = MacTableLookup(pAd, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].OriginalAddress);
					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
						pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = FALSE;
					else
						pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli = TRUE;

					pMacEntry = NULL;
				}
			}
#endif /* MAC_REPEATER_SUPPORT */

			DBGPRINT(RT_DEBUG_TRACE, ("!!! APCLI LINK UP - IF(%s%d) AuthMode(%d)=%s, WepStatus(%d)=%s !!!\n", 
					INF_APCLI_DEV_NAME, ifIndex, 
										pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode, GetAuthMode(pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode),
										pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus, GetEncryptType(pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus)));			
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : APCLI LINK UP - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
			result = FALSE;
			break;
		}

#ifdef MAC_REPEATER_SUPPORT
		if ((CliIdx != 0xFF)
#ifdef MWDS
			&& ((pAd->ApCfg.ApCliTab[ApCliIdx].bSupportMWDS == FALSE) ||
				(pAd->ApCfg.ApCliTab[ApCliIdx].MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */
		)
			DBGPRINT(RT_DEBUG_ERROR, ("(%s) ifIndex = %d, CliIdx = %d !!!\n", __FUNCTION__, ifIndex, CliIdx));
#endif /* MAC_REPEATER_SUPPORT */

		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#ifndef APCLI_CONNECTION_TRIAL
		if ((pApCliEntry->Valid)
#ifdef MAC_REPEATER_SUPPORT
			 && (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : This link had existed - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
			result = FALSE;
			break;
		}
#endif /* APCLI_CONNECTION_TRIAL */	

		wdev = &pApCliEntry->wdev;
#ifdef APCLI_AUTO_BW_SUPPORT
		PhyMode = wdev->PhyMode;
#endif /* APCLI_AUTO_BW_SUPPORT */

		printk("!!! APCLI LINK UP - IF(%s%d) AuthMode(%d)=%s, WepStatus(%d)=%s!\n",
					INF_APCLI_DEV_NAME, ifIndex, 
					wdev->AuthMode, GetAuthMode(wdev->AuthMode),
					wdev->WepStatus, GetEncryptType(wdev->WepStatus));

#if defined (CONFIG_WIFI_PKT_FWD)
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (wf_fwd_get_rep_hook)
				wf_fwd_get_rep_hook(pAd->CommonCfg.EtherTrafficBand);
			
			if (wf_fwd_entry_insert_hook) 
				wf_fwd_entry_insert_hook (wdev->if_dev, pAd->net_dev, pAd);
			
			if (wf_fwd_insert_repeater_mapping_hook)
				wf_fwd_insert_repeater_mapping_hook (pAd, &pAd->ApCfg.ReptCliEntryLock, &pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0], &pAd->ApCfg.ApCliTab[0].wdev.if_addr);
		}

#endif /* CONFIG_WIFI_PKT_FWD */

		/* Insert the Remote AP to our MacTable. */
		/*pMacEntry = MacTableInsertApCliEntry(pAd, (PUCHAR)(pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid)); */
#ifdef MAC_REPEATER_SUPPORT
		if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef MWDS
			&& (pApCliEntry != NULL)
			&& ((pApCliEntry->bSupportMWDS == FALSE) ||
				(pApCliEntry->MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */
		)
			pMacEntry = RTMPInsertRepeaterMacEntry(pAd,
							(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
							wdev,
							(ifIndex + MIN_NET_DEVICE_FOR_APCLI),
							CliIdx,
							TRUE);
		else
#endif /* MAC_REPEATER_SUPPORT */
		pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pApCliEntry->MlmeAux.Bssid),
										wdev, (ifIndex + MIN_NET_DEVICE_FOR_APCLI),
										OPMODE_AP, TRUE);
		if (pMacEntry)
		{
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			pMacEntry->Sst = SST_ASSOC;
			pMacEntry->wdev = &pApCliEntry->wdev;
#ifdef MAC_REPEATER_SUPPORT
			if ((CliIdx != 0xFF)
#ifdef MWDS
				&& (pApCliEntry != NULL)
				&& ((pApCliEntry->bSupportMWDS == FALSE) ||
					(pApCliEntry->MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */
			)
			{
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID = pMacEntry->Aid;
				pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = TRUE;
				pMacEntry->bReptCli = TRUE;
				pMacEntry->MatchReptCliIdx = CliIdx;
				pMacEntry->ReptCliIdleCount = 0;
				COPY_MAC_ADDR(pMacEntry->ReptCliAddr, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CurrentAddress);
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].bEthCli == TRUE)
					pMacEntry->bReptEthCli = TRUE;
				else
					pMacEntry->bReptEthCli = FALSE;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->MacTabWCID = pMacEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT
				pMacEntry->bReptCli = FALSE;
#endif /* MAC_REPEATER_SUPPORT */
				COPY_MAC_ADDR(&wdev->bssid[0], &pApCliEntry->MlmeAux.Bssid[0]);
				COPY_MAC_ADDR(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID), pApCliEntry->MlmeAux.Bssid);
				pApCliEntry->SsidLen = pApCliEntry->MlmeAux.SsidLen;
				NdisMoveMemory(pApCliEntry->Ssid, pApCliEntry->MlmeAux.Ssid, pApCliEntry->SsidLen);
			}

#ifdef WPA_SUPPLICANT_SUPPORT
		/*
			If ApCli connects to different AP, ApCli couldn't send EAPOL_Start for WpaSupplicant.
		*/
		if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) &&
			(NdisEqualMemory(pAd->MlmeAux.Bssid, pApCliEntry->LastBssid, MAC_ADDR_LEN) == FALSE) &&
			(pApCliEntry->wpa_supplicant_info.bLostAp == TRUE))
		{
			pApCliEntry->wpa_supplicant_info.bLostAp = FALSE;
		}

		COPY_MAC_ADDR(pApCliEntry->LastBssid, pAd->MlmeAux.Bssid);
#endif /* WPA_SUPPLICANT_SUPPORT */

#ifdef SMART_MESH
			if(pApCliEntry->MlmeAux.bHyperFiPeer)
				pMacEntry->bHyperFiPeer = TRUE;
			else
				pMacEntry->bHyperFiPeer = FALSE;
#endif /* SMART_MESH */

			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				pMacEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{

#ifdef WPA_SUPPLICANT_SUPPORT
				if (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP &&
					(pMacEntry->WepStatus == Ndis802_11WEPEnabled) &&
					(wdev->IEEE8021X == TRUE))
					pMacEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
				else
#endif /*WPA_SUPPLICANT_SUPPORT*/
				{
					pMacEntry->PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef SMART_MESH
				if(pApCliEntry &&
				   pApCliEntry->MlmeAux.bSupportSmartMesh && 
				   pApCliEntry->SmartMeshCfg.bSupportSmartMesh)
					pMacEntry->bEnableSmartMesh = TRUE;
				else
					pMacEntry->bEnableSmartMesh = FALSE;
#endif /* SMART_MESH */

#ifdef MWDS
#ifdef WSC_AP_SUPPORT
				BOOLEAN bWPSRunning = FALSE;
				if (((pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
					(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE)))
					bWPSRunning = TRUE;
#endif /* WSC_AP_SUPPORT */
				if(
#ifdef WSC_AP_SUPPORT
				   !bWPSRunning &&
#endif /* WSC_AP_SUPPORT */
				   pApCliEntry->MlmeAux.bSupportMWDS && 
				   pApCliEntry->bSupportMWDS)
				{
					pApCliEntry->bEnableMWDS = TRUE;
					pMacEntry->bEnableMWDS = TRUE;
					if((pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED))
					{
						SET_MWDS_OPMODE_APCLI(pMacEntry);
						DBGPRINT(RT_DEBUG_ERROR, ("SET_MWDS_OPMODE_APCLI OK!\n"));
					}
				}
				else
				{
					pApCliEntry->bEnableMWDS = FALSE;
					pMacEntry->bEnableMWDS = FALSE;
					SET_MWDS_OPMODE_NONE(pMacEntry);
				}
#endif /* MWDS */
				}
#ifdef MAC_REPEATER_SUPPORT
				if ((CliIdx != 0xFF)
#ifdef MWDS
				&& (pApCliEntry != NULL)
				&& ((pApCliEntry->bSupportMWDS == FALSE) ||
					(pApCliEntry->MlmeAux.bSupportMWDS == FALSE))
#endif /* MWDS */
				)
					pApCliEntry->RepeaterCli[CliIdx].CliConnectState = 2;
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
				(CliIdx == 0xFF) &&
#endif /* MAC_REPEATER_SUPPORT */
				(pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
				DBGPRINT(RT_DEBUG_TRACE, ("ApCli auto connected: ApCliLinkUp()\n"));
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
			}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				NdisGetSystemUpTime(&pApCliEntry->ApCliLinkUpTime);

			/*
				Store appropriate RSN_IE for WPA SM negotiation later
				If WPAPSK/WPA2SPK mix mode, driver just stores either WPAPSK or 
				WPA2PSK RSNIE. It depends on the AP-Client's authentication mode
				to store the corresponding RSNIE.
			*/
			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) && (pApCliEntry->MlmeAux.VarIELen != 0))
			{
				PUCHAR pVIE;
				UCHAR len;
				PEID_STRUCT pEid;

				pVIE = pApCliEntry->MlmeAux.VarIEs;
				len	 = pApCliEntry->MlmeAux.VarIELen;

				while (len > 0)
				{
					pEid = (PEID_STRUCT) pVIE;	
					/* For WPA/WPAPSK */
					if ((pEid->Eid == IE_WPA) && (NdisEqualMemory(pEid->Octet, WPA_OUI, 4)) 
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA || pMacEntry->AuthMode == Ndis802_11AuthModeWPAPSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);							
						DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA SM negotiation \n"));
					}
					/* For WPA2/WPA2PSK */
					else if ((pEid->Eid == IE_RSN) && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA2 || pMacEntry->AuthMode == Ndis802_11AuthModeWPA2PSK))
					{
						NdisMoveMemory(pMacEntry->RSN_IE, pVIE, (pEid->Len + 2));
						pMacEntry->RSNIE_Len = (pEid->Len + 2);	
						DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: Store RSN_IE for WPA2 SM negotiation \n"));
					}

					pVIE += (pEid->Len + 2);
					len  -= (pEid->Len + 2);
				}							
			}

			if (pMacEntry->RSNIE_Len == 0)
			{			
				DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp: root-AP has no RSN_IE \n"));			
			}
			else
			{
				hex_dump("The RSN_IE of root-AP", pMacEntry->RSN_IE, pMacEntry->RSNIE_Len);
			}		

			SupportRate(pApCliEntry->MlmeAux.SupRate, pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->MlmeAux.ExtRate,
				pApCliEntry->MlmeAux.ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);

			pMacEntry->MaxSupportedRate = min(pAd->CommonCfg.MaxTxRate, MaxSupportedRate);
			pMacEntry->RateLen = RatesLen;
			set_entry_phy_cfg(pAd, pMacEntry);

			pMacEntry->CapabilityInfo = pApCliEntry->MlmeAux.CapabilityInfo;

			pApCliEntry->ApCliBeaconPeriod = pApCliEntry->MlmeAux.BeaconPeriod;
			
			if ((wdev->WepStatus == Ndis802_11WEPEnabled)
#ifdef WPA_SUPPLICANT_SUPPORT
				&& (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP == WPA_SUPPLICANT_DISABLE)
#endif /* WPA_SUPPLICANT_SUPPORT */
			)
			{			
				CIPHER_KEY *pKey;
				INT idx, BssIdx;

				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
#ifdef MAC_APCLI_SUPPORT
				BssIdx = APCLI_BSS_BASE + ifIndex;
#endif /* MAC_APCLI_SUPPORT */
				for (idx=0; idx < SHARE_KEY_NUM; idx++)
				{
					pKey = &pApCliEntry->SharedKey[idx];
					if (pKey->KeyLen > 0)
					{
						/* Set key material and cipherAlg to Asic */
#ifdef MAC_REPEATER_SUPPORT
						if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
						{
							RTMP_ASIC_SHARED_KEY_TABLE(pAd, 
	    									  		BssIdx, 
	    									  		idx, 
		    										pKey);	
						}

						if (idx == wdev->DefaultKeyId)
						{						
							INT	cnt;
					
							/* Generate 3-bytes IV randomly for software encryption using */						
							for(cnt = 0; cnt < LEN_WEP_TSC; cnt++)
								pKey->TxTsc[cnt] = RandomByte(pAd); 
					
							RTMP_SET_WCID_SEC_INFO(pAd, 
												BssIdx, 
												idx, 
												pKey->CipherAlg, 
												pMacEntry->wcid, 
												SHAREDKEYTABLE);
						}
					}	
				}    		   		  		   					
			}

#ifdef DOT11_N_SUPPORT
			/* If this Entry supports 802.11n, upgrade to HT rate. */
			if (pApCliEntry->MlmeAux.HtCapabilityLen != 0)
			{
				PHT_CAPABILITY_IE pHtCapability = (PHT_CAPABILITY_IE)&pApCliEntry->MlmeAux.HtCapability;

				ht_mode_adjust(pAd, pMacEntry, pHtCapability, &pAd->CommonCfg.DesiredHtPhy);

				/* find max fixed rate */
				pMacEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
																	&pHtCapability->MCSSet[0]);
				 
				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("IF-%s%d : Desired MCS = %d\n",
								INF_APCLI_DEV_NAME, ifIndex, wdev->DesiredTransmitSetting.field.MCS));

					set_ht_fixed_mcs(pAd, pMacEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pMacEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
				if (pHtCapability->HtCapParm.MpduDensity < 5)
					pMacEntry->MpduDensity = 5;
				else
				pMacEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
				pMacEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
				pMacEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
				pMacEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;				
				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
				if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);
				if (pHtCapability->HtCapInfo.ShortGIfor20)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI20_CAPABLE);
				if (pHtCapability->HtCapInfo.ShortGIfor40)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_SGI40_CAPABLE);
				if (pHtCapability->HtCapInfo.TxSTBC)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_TxSTBC_CAPABLE);
				if (pHtCapability->HtCapInfo.RxSTBC)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RxSTBC_CAPABLE);
				if (pHtCapability->ExtHtCapInfo.PlusHTC)				
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_HTC_CAPABLE);
				if (pAd->CommonCfg.bRdg && pHtCapability->ExtHtCapInfo.RDGSupport)				
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RDG_CAPABLE);
				if (pHtCapability->ExtHtCapInfo.MCSFeedback == 0x03)
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_MCSFEEDBACK_CAPABLE);		
				NdisMoveMemory(&pMacEntry->HTCapability, &pApCliEntry->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);
			}
			else
			{
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
								  RateIdToMbps[pMacEntry->MaxSupportedRate]));
			}
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(PhyMode) && pApCliEntry->MlmeAux.vht_cap_len &&  pApCliEntry->MlmeAux.vht_op_len)
			{
			    vht_mode_adjust(pAd, pMacEntry, &(pApCliEntry->MlmeAux.vht_cap), &(pApCliEntry->MlmeAux.vht_op));
				ApCliCheckVht(pAd,pMacEntry->Aid, pMacEntry,&(pApCliEntry->MlmeAux.vht_cap), &(pApCliEntry->MlmeAux.vht_op));
			}
#endif /* DOT11_VHT_AC */

			pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
			pMacEntry->CurrTxRate = pMacEntry->MaxSupportedRate;
			
			RTMPSetSupportMCS(pAd,
							OPMODE_AP,
							pMacEntry,
							pApCliEntry->MlmeAux.SupRate,
							pApCliEntry->MlmeAux.SupRateLen,
							pApCliEntry->MlmeAux.ExtRate,
							pApCliEntry->MlmeAux.ExtRateLen,
#ifdef DOT11_VHT_AC
							pApCliEntry->MlmeAux.vht_cap_len,
							&pApCliEntry->MlmeAux.vht_cap,
#endif /* DOT11_VHT_AC */
							&pApCliEntry->MlmeAux.HtCapability,
							pApCliEntry->MlmeAux.HtCapabilityLen);
			
			if (wdev->bAutoTxRateSwitch == FALSE)
			{
				pMacEntry->bAutoTxRateSwitch = FALSE;
				/* If the legacy mode is set, overwrite the transmit setting of this entry. */
				RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pMacEntry);	
			}
			else
			{
				UCHAR TableSize = 0;

				pMacEntry->bAutoTxRateSwitch = TRUE;
				MlmeSelectTxRateTable(pAd, pMacEntry, &pMacEntry->pTable, &TableSize, &pMacEntry->CurrTxRateIndex);
				MlmeNewTxRate(pAd, pMacEntry);
#ifdef NEW_RATE_ADAPT_SUPPORT
				if (! ADAPT_RATE_TABLE(pMacEntry->pTable))
#endif /* NEW_RATE_ADAPT_SUPPORT */
					pMacEntry->HTPhyMode.field.ShortGI = GI_800;
			}
			
			/* set this entry WMM capable or not */
			if ((pApCliEntry->MlmeAux.APEdcaParm.bValid)
#ifdef DOT11_N_SUPPORT
				|| IS_HT_STA(pMacEntry)
#endif /* DOT11_N_SUPPORT */
			)
			{
#ifdef APCLI_CERT_SUPPORT
				if (pApCliEntry->wdev.bWmmCapable == TRUE)
				{
					AsicSetEdcaParm(pAd, &pApCliEntry->MlmeAux.APEdcaParm);
					pAd->ApCfg.BssEdcaParm.EdcaUpdateCount++;
				}	
#endif /* APCLI_CERT_SUPPORT */			
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
			if (pAd->CommonCfg.bAggregationCapable)
			{
				if ((pAd->CommonCfg.bPiggyBackCapable) && (pApCliEntry->MlmeAux.APRalinkIe & 0x00000003) == 3)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_PIGGYBACK_INUSED);
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE);
					RTMPSetPiggyBack(pAd, TRUE);
					DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
				}
				else if (pApCliEntry->MlmeAux.APRalinkIe & 0x00000001)
				{
					OPSTATUS_SET_FLAG(pAd, fOP_STATUS_AGGREGATION_INUSED);
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AGGREGATION_CAPABLE);
					DBGPRINT(RT_DEBUG_TRACE, ("Ralink Aggregation\n"));
				}
			}

			/* Set falg to identify if peer AP is Ralink chipset */
			if (pApCliEntry->MlmeAux.APRalinkIe != 0x0)
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);
			else
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_RALINK_CHIPSET);

			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
			/* set the apcli interface be valid. */
#ifdef MAC_APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if ((CliIdx == 0xFF)
#ifdef APCLI_CONNECTION_TRIAL
				&& (pApCliEntry->TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
				)
#endif /* MAC_REPEATER_SUPPORT */
#endif /* MAC_APCLI_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->wdev.allow_data_tx = TRUE;
				pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_APCLI_SUPPORT
				AsicSetApCliBssid(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex);
#endif /* MAC_APCLI_SUPPORT */
			}
			result = TRUE;
			pAd->ApCfg.ApCliInfRunned++;

			break;
		}
		result = FALSE;
	} while(FALSE);


	if (result == FALSE)
	{
	 	DBGPRINT(RT_DEBUG_ERROR, (" (%s) alloc mac entry fail!!!\n", __FUNCTION__));
		return result;
	}

#ifdef WPA_SUPPLICANT_SUPPORT
	/*
		When AuthMode is WPA2-Enterprise and AP reboot or STA lost AP, 
		WpaSupplicant would not send EapolStart to AP after STA re-connect to AP again.
		In this case, driver would send EapolStart to AP.
	*/
	if ((pMacEntry->AuthMode == Ndis802_11AuthModeWPA2) &&
	    (NdisEqualMemory(pAd->MlmeAux.Bssid, pAd->ApCfg.ApCliTab[ifIndex].LastBssid, MAC_ADDR_LEN)) &&
	    (pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.bLostAp == TRUE))
	{
		ApcliWpaSendEapolStart(pAd, pAd->MlmeAux.Bssid,pMacEntry,&pAd->ApCfg.ApCliTab[ifIndex]);		
		pAd->ApCfg.ApCliTab[ifIndex].wpa_supplicant_info.bLostAp = FALSE;
	}
#endif /*WPA_SUPPLICANT_SUPPORT */

	RTMPSetSupportMCS(pAd,
					OPMODE_AP,
					pMacEntry,
					pApCliEntry->MlmeAux.SupRate,
					pApCliEntry->MlmeAux.SupRateLen,
					pApCliEntry->MlmeAux.ExtRate,
					pApCliEntry->MlmeAux.ExtRateLen,
#ifdef DOT11_VHT_AC
					pApCliEntry->MlmeAux.vht_cap_len,
					&pApCliEntry->MlmeAux.vht_cap,
#endif /* DOT11_VHT_AC */
					&pApCliEntry->MlmeAux.HtCapability,
					pApCliEntry->MlmeAux.HtCapabilityLen);

#ifdef WSC_AP_SUPPORT
    /* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */
	if ((result == TRUE) && 
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscState = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscStatus = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
		NdisZeroMemory(pApCliEntry->WscControl.EntryAddr, MAC_ADDR_LEN);        
		NdisMoveMemory(pApCliEntry->WscControl.EntryAddr, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		WscSendEapolStart(pAd, pMacEntry->Addr, AP_MODE);
	}
    else
    {
        DBGPRINT(RT_DEBUG_TRACE, ("WSCSTOP: %d, %d, %d\n", result,
                pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode, 
				pAd->ApCfg.ApCliTab[ifIndex].WscControl.bWscTrigger));	
	    WscStop(pAd, TRUE, &pApCliEntry->WscControl);    
    }
#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#ifdef APCLI_CERT_SUPPORT
	if (pAd->bApCliCertTest == TRUE)
	{
		if ((pAd->CommonCfg.bBssCoexEnable == TRUE)
		    && (pAd->CommonCfg.Channel <= 14)
		    && (pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable == TRUE)
		    && (pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport == 1)) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SCAN_2040);
			BuildEffectedChannelList(pAd);
			/*pAd->CommonCfg.ScanParameter.Dot11BssWidthTriggerScanInt = 150; */
			DBGPRINT(RT_DEBUG_TRACE,
				 ("LinkUP AP supports 20/40 BSS COEX !!! Dot11BssWidthTriggerScanInt[%d]\n",
				  pAd->CommonCfg.Dot11BssWidthTriggerScanInt));
		} 
		else
		{
			DBGPRINT(RT_DEBUG_TRACE,
				 ("not supports 20/40 BSS COEX !!! \n"));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pAd->CommonCfg.bBssCoexEnable %d !!! \n",
				  pAd->CommonCfg.bBssCoexEnable));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pAd->CommonCfg.Channel %d !!! \n",
				  pAd->CommonCfg.Channel));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pApCliEntry->DesiredHtPhyInfo.bHtEnable %d !!! \n",
				  pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pAd->MlmeAux.ExtCapInfo.BssCoexstSup %d !!! \n",
				  pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pAd->CommonCfg.CentralChannel %d !!! \n",
				  pAd->CommonCfg.CentralChannel));
			DBGPRINT(RT_DEBUG_TRACE,
				 ("pAd->CommonCfg.PhyMode %d !!! \n",
				  pAd->CommonCfg.PhyMode));
		}
	}
#endif /* APCLI_CERT_SUPPORT */	
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#ifdef SMART_MESH_MONITOR
	if((result == TRUE) && (pMacEntry->PortSecured == WPA_802_1X_PORT_SECURED))
	{
		struct nsmpif_drvevnt_buf drvevnt;
		drvevnt.data.linkstate.type = NSMPIF_DRVEVNT_EXT_UPLINK_STAT;
		drvevnt.data.linkstate.link_state = NSMP_UPLINK_STAT_CONNECTED;
		NdisCopyMemory(drvevnt.data.linkstate.bssid,pMacEntry->wdev->bssid,MAC_ADDR_LEN);
		drvevnt.data.linkstate.channel = pAd->CommonCfg.Channel;
		NdisZeroMemory(drvevnt.data.linkstate.op_channels,sizeof(drvevnt.data.linkstate.op_channels));
		drvevnt.data.linkstate.op_channels[0] = pAd->CommonCfg.Channel;
#ifdef DOT11_N_SUPPORT
		if(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
			drvevnt.data.linkstate.op_channels[1] = N_GetSecondaryChannel(pAd);
#endif /* DOT11_N_SUPPORT */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_EXT_UPLINK_STAT,
								NULL, (PUCHAR)&drvevnt.data.linkstate, sizeof(drvevnt.data.linkstate));
	}
#endif /* SMART_MESH_MONITOR */

	/* When root AP is Open WEP, it will cause a fake connection state if user keys in wrong password. */
	if((result == TRUE) &&
	   (wdev->AuthMode == Ndis802_11AuthModeOpen) &&
	   (wdev->WepStatus == Ndis802_11WEPEnabled))
		pApCliEntry->OpenWEPErrPktChk = TRUE;
	else
		pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;
	return result;
}


/*
    ==========================================================================

	Routine	Description:
		Disconnect current BSSID

	Arguments:
		pAd				- Pointer to our adapter
		ApCliIdx		- Which ApCli interface		
	Return Value:		
		None

	Note:

	==========================================================================
*/
VOID ApCliLinkDown(RTMP_ADAPTER *pAd, UCHAR ifIndex)
{
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx = 0xFF;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR MacTabWCID = 0;

	if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		|| (ifIndex >= 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
	{
#ifdef MAC_REPEATER_SUPPORT
		if (ifIndex >= 64)
		{
			CliIdx = ((ifIndex - 64) % 16);
			ifIndex = ((ifIndex - 64) / 16);
			printk("!!! REPEATER CLI LINK DOWN - IF(%s%d) Cli %d !!!\n", INF_APCLI_DEV_NAME, ifIndex, CliIdx);
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		printk("!!! APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex);
	}
	else
	{
		DBGPRINT(RT_DEBUG_OFF, ("!!! ERROR : APCLI LINK DOWN - IF(%s%d)!!!\n", INF_APCLI_DEV_NAME, ifIndex));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if ((pApCliEntry->Valid == FALSE)	
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#if defined (CONFIG_WIFI_PKT_FWD)
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		if (wf_fwd_entry_delete_hook)
			wf_fwd_entry_delete_hook (pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
	}
#ifdef MAC_REPEATER_SUPPORT
	else
	{
		if (packet_source_delete_entry_hook)
			packet_source_delete_entry_hook(100);
	}
#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_WIFI_PKT_FWD */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	pAd->ApCfg.ApCliInfRunned--;

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		MacTabWCID = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID;
	else
#endif /* MAC_REPEATER_SUPPORT */
	MacTabWCID = pApCliEntry->MacTabWCID;

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx != 0xFF(1) MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			MacTabWCID,PRINT_MAC(pAd->MacTab.Content[MacTabWCID].Addr)));
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx != 0xFF(2) pApCliEntry->MacTabWCID =%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			pApCliEntry->MacTabWCID,PRINT_MAC(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID))));
		
	} else
#endif /* MAC_REPEATER_SUPPORT */	
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx == 0xFF(1) MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			MacTabWCID,PRINT_MAC(pAd->MacTab.Content[MacTabWCID].Addr)));
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx == 0xFF(2) pApCliEntry->MacTabWCID =%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			pApCliEntry->MacTabWCID,PRINT_MAC(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID))));
	
	}
	
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx != 0xFF MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			MacTabWCID,PRINT_MAC(pAd->MacTab.Content[MacTabWCID].Addr)));
		MacTableDeleteEntry(pAd, MacTabWCID, pAd->MacTab.Content[MacTabWCID].Addr);
	}
  	else		
#endif /* MAC_REPEATER_SUPPORT */			
  	{
		DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkDown CliIdx == 0xFF MacTabWCID=%d addr=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
			MacTabWCID, PRINT_MAC(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID))));
		MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));
  	}

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
	{
		pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliValid = FALSE;
	}
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;	/* This link doesn't associated with any remote-AP */
		pApCliEntry->wdev.allow_data_tx = FALSE;
		pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
	}
#ifdef MWDS
	pApCliEntry->bEnableMWDS = FALSE;
#endif /* MWDS */
	pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;

	pAd->ApCfg.ApCliTab[ifIndex].bPeerExist = FALSE;

#ifdef WPA_SUPPLICANT_SUPPORT
	if (pApCliEntry->wpa_supplicant_info.WpaSupplicantUP) 
	{
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] Send RT_DISASSOC_EVENT_FLAG.\n", __FUNCTION__, ifIndex));
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_DISASSOC_EVENT_FLAG, NULL, NULL, 0);
	}   
#endif /* WPA_SUPPLICANT_SUPPORT */  

#ifdef APCLI_CERT_SUPPORT
	if (pApCliEntry->wdev.bWmmCapable == TRUE)
	{
		AsicSetEdcaParm(pAd, &pAd->CommonCfg.APEdcaParm); /* Restore AP's EDCA parameters. */
		pAd->ApCfg.BssEdcaParm.EdcaUpdateCount++;
	}	
#endif /* APCLI_CERT_SUPPORT */

#ifdef RT_CFG80211_SUPPORT
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
	RT_CFG80211_LOST_GO_INFORM(pAd);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE */
#endif /* RT_CFG80211_SUPPORT */

#ifdef SMART_MESH_MONITOR
	{
		struct nsmpif_drvevnt_buf drvevnt;
		drvevnt.data.linkstate.type = NSMPIF_DRVEVNT_EXT_UPLINK_STAT;
		drvevnt.data.linkstate.link_state = NSMP_UPLINK_STAT_DISCONNECT;
		drvevnt.data.linkstate.channel = pAd->CommonCfg.Channel;
		NdisZeroMemory(drvevnt.data.linkstate.op_channels,sizeof(drvevnt.data.linkstate.op_channels));
		drvevnt.data.linkstate.op_channels[0] = pAd->CommonCfg.Channel;
#ifdef DOT11_N_SUPPORT
		if(pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth == BW_40)
			drvevnt.data.linkstate.op_channels[1] = N_GetSecondaryChannel(pAd);
#endif /* DOT11_N_SUPPORT */
		RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM,NSMPIF_DRVEVNT_EXT_UPLINK_STAT,
								NULL, (PUCHAR)&drvevnt.data.linkstate, sizeof(drvevnt.data.linkstate));
	}
#endif /* SMART_MESH_MONITOR */

}


/* 
    ==========================================================================
    Description:
        APCLI Interface Up.
    ==========================================================================
 */
VOID ApCliIfUp(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
#ifdef APCLI_CONNECTION_TRIAL
	PULONG pCurrState = NULL;
#endif /* APCLI_CONNECTION_TRIAL */

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS |
								fRTMP_ADAPTER_HALT_IN_PROGRESS)) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	/* sanity check whether the interface is initialized. */
	if (pAd->flg_apcli_init != TRUE)
		return;

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef APCLI_CONNECTION_TRIAL				
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#endif /* APCLI_CONNECTION_TRIAL */		
		
		if (APCLI_IF_UP_CHECK(pAd, ifIndex) 
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			&& (ifIndex == 0)
#endif /* APCLI_CONNECTION_TRIAL */
			)
		{
			if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE))
			{	
				if (pApCliEntry->bPeerExist == TRUE)
				{
					/* Got peer's beacon; change to normal mode */
					pAd->Dot11_H.RDCount = pAd->Dot11_H.ChMovingTime;
					DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfUp - PeerExist\n"));
				}
				else
					DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfUp - Stop probing while Radar state is silent\n"));
				
				continue;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->bPeerExist = FALSE;
		}
#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (*pCurrState == APCLI_CTRL_DISCONNECTED)//Apcli1 is not connected state.
			&& (pApCliEntry->TrialCh != 0)
			//&& NdisCmpMemory(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->CfgSsid, pApCliEntry->SsidLen) != 0
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->Enable != 0)
			//new ap ssid shall different from the origin one.
		)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) Enqueue APCLI_CTRL_TRIAL_CONNECT\n", __func__));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT, 0, NULL, ifIndex);
		}
#endif /* APCLI_CONNECTION_TRIAL */

	}

	return;
}


/* 
    ==========================================================================
    Description:
        APCLI Interface Down.
    ==========================================================================
 */
VOID ApCliIfDown(RTMP_ADAPTER *pAd)
{
	UCHAR ifIndex;
	PAPCLI_STRUCT pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx,idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	for(ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

#if defined (CONFIG_WIFI_PKT_FWD)
        if (wf_fwd_entry_delete_hook)
            wf_fwd_entry_delete_hook (pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
#endif /* CONFIG_WIFI_PKT_FWD */                                
		DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
				{
#ifdef DOT11_N_SUPPORT
					/* free resources of BA*/
					BASessionTearDownALL(pAd, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID);
#endif /* DOT11_N_SUPPORT */
					RTMPRemoveRepeaterDisconnectEntry(pAd, ifIndex, CliIdx);
					RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
	}

#ifdef MAC_REPEATER_SUPPORT
	for (idx = 0; idx< 32; idx++)
	{
		pEntry = &pAd->ApCfg.ReptControl.RepeaterInvaildEntry[idx];

		RTMPRepeaterRemoveInvaildMacEntry(pAd, idx, pEntry->MacAddr);
	}
#endif /* MAC_REPEATER_SUPPORT */

	return;
}


/* 
    ==========================================================================
    Description:
        APCLI Interface Monitor.
    ==========================================================================
 */
VOID ApCliIfMonitor(RTMP_ADAPTER *pAd)
{
	UCHAR index;
	APCLI_STRUCT *pApCliEntry;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx;
	PREPEATER_CLIENT_ENTRY pReptCliEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	/* Reset is in progress, stop immediately */
	if ( RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RESET_IN_PROGRESS) ||
		 RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		 !RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	/* sanity check whether the interface is initialized. */
	if (pAd->flg_apcli_init != TRUE)
		return;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
#ifdef APCLI_CONNECTION_TRIAL
		if (index == 1)
			continue;//skip apcli1 monitor. FIXME:Carter shall find a better way.
#endif /* APCLI_CONNECTION_TRIAL */	
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		BOOLEAN bForceBrocken = FALSE;
 
		pApCliEntry = &pAd->ApCfg.ApCliTab[index];
#ifdef MAC_REPEATER_SUPPORT
		if ((pAd->ApCfg.bMACRepeaterEn) && (pApCliEntry->Enable))
		{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				pReptCliEntry = &pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx];

				if (pReptCliEntry->CliEnable)
				{
					if (pReptCliEntry->CliValid)
					{
						Wcid = pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].MacTabWCID;

						if (!VALID_WCID(Wcid))
							continue;

						pMacEntry = &pAd->MacTab.Content[Wcid];

						if ((pMacEntry->PortSecured != WPA_802_1X_PORT_SECURED) &&
							RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("%s, wcid = %d, %s%d, CliIdx = %d\n", __func__, Wcid, INF_APCLI_DEV_NAME, index, CliIdx));
							RTMPRemoveRepeaterDisconnectEntry(pAd, index, CliIdx);
#ifdef DOT11_N_SUPPORT
							/* free resources of BA*/
							BASessionTearDownALL(pAd, Wcid);
#endif /* DOT11_N_SUPPORT */
							RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
						}
					}
					else
					{
						if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
						{
							DBGPRINT(RT_DEBUG_TRACE, ("%s, %s%d, CliIdx = %d\n", __func__, INF_APCLI_DEV_NAME, index, CliIdx));
						RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->Valid == TRUE)
		{
			Wcid = pAd->ApCfg.ApCliTab[index].MacTabWCID;
			if (!VALID_WCID(Wcid))
				continue;

			pMacEntry = &pAd->MacTab.Content[Wcid];
#ifdef WSC_AP_SUPPORT
            if(!((pAd->ApCfg.ApCliTab[index].WscControl.WscConfMode != WSC_DISABLE) &&
				 (pAd->ApCfg.ApCliTab[index].WscControl.bWscTrigger == TRUE)))
#endif /* WSC_AP_SUPPORT */
            {
                if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA) &&
				     (pMacEntry->PortSecured != WPA_802_1X_PORT_SECURED) &&
				     (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliLinkUpTime + (30 * OS_HZ)))))
				bForceBrocken = TRUE;
            }
 
			if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliRcvBeaconTime + (8 * OS_HZ))))
			{
				printk("ApCliIfMonitor: IF(%s%d) - no Beacon is received from Root-AP.\n", INF_APCLI_DEV_NAME, index);
				bForceBrocken = TRUE;
			}
		}
		else
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
				{
					if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliEnable)
					{
#ifdef DOT11_N_SUPPORT
						/* free resources of BA*/
						if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliValid)
						{
							Wcid = pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].MacTabWCID;
							if (VALID_WCID(Wcid))
								BASessionTearDownALL(pAd, Wcid);
						}
#endif /* DOT11_N_SUPPORT */
						RTMPRemoveRepeaterDisconnectEntry(pAd, index, CliIdx);
						RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
#endif /* MAC_REPEATER_SUPPORT */

			continue;
		}
 
		if (bForceBrocken == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));

#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
				{
					if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliEnable)
					{
#ifdef DOT11_N_SUPPORT
						/* free resources of BA*/
						BASessionTearDownALL(pAd, pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].MacTabWCID);
#endif /* DOT11_N_SUPPORT */
						RTMPRemoveRepeaterDisconnectEntry(pAd, index, CliIdx);
						RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
#endif /* MAC_REPEATER_SUPPORT */
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, index);
			RTMP_MLME_HANDLER(pAd);
		}
	}

	return;
}


/*! \brief   To substitute the message type if the message is coming from external
 *  \param  pFrame         The frame received
 *  \param  *Machine       The state machine
 *  \param  *MsgType       the message type for the state machine
 *  \return TRUE if the substitution is successful, FALSE otherwise
 *  \pre
 *  \post
 */
BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame, 
	OUT INT *Machine, 
	OUT INT *MsgType)
{
	USHORT Seq;
	UCHAR EAPType; 
	BOOLEAN Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
    PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */


	/* only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, */
	/* ignore this frame */

	/* WPA EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA)
	{		
#ifdef WSC_AP_SUPPORT    
	        /*WSC EAPOL PACKET */
	        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
            if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->apidx].WscControl.WscConfMode != WSC_DISABLE)
	        {
	            *Machine = WSC_STATE_MACHINE;
	            EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	            EAPCode = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 4);
	            Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
	        }
	        if (!Return)
#endif /* WSC_AP_SUPPORT */
	        {
	    		*Machine = WPA_STATE_MACHINE;
	    		EAPType = *((UCHAR*)pFrame + LENGTH_802_11 + LENGTH_802_1_H + 1);
	    		Return = WpaMsgTypeSubst(EAPType, MsgType);
	        }
		return Return;
	}
	else if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT)
	{
		switch (pFrame->Hdr.FC.SubType) 
		{
			case SUBTYPE_ASSOC_RSP:
				*Machine = APCLI_ASSOC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_ASSOC_RSP;
				break;

			case SUBTYPE_DISASSOC:
				*Machine = APCLI_ASSOC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_DISASSOC_REQ;
				break;

			case SUBTYPE_DEAUTH:
				*Machine = APCLI_AUTH_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_DEAUTH;
				break;

			case SUBTYPE_AUTH:
				/* get the sequence number from payload 24 Mac Header + 2 bytes algorithm */
				NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
				if (Seq == 2 || Seq == 4)
				{
					*Machine = APCLI_AUTH_STATE_MACHINE;
					*MsgType = APCLI_MT2_PEER_AUTH_EVEN;
				}
				else 
				{
					return FALSE;
				}
				break;

			case SUBTYPE_ACTION:
				*Machine = ACTION_STATE_MACHINE;
				/*  Sometimes Sta will return with category bytes with MSB = 1, if they receive catogory out of their support */
				if ((pFrame->Octet[0]&0x7F) > MAX_PEER_CATE_MSG) 
					*MsgType = MT2_ACT_INVALID;
				else
					*MsgType = (pFrame->Octet[0]&0x7F);
				break;

			default:
				return FALSE;
		}

		return TRUE;
	}

	return FALSE;
}


BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame, 
	OUT INT *Machine, 
	OUT INT *MsgType)
{
	if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT)
	{
		switch (pFrame->Hdr.FC.SubType) 
		{
			/* Beacon must be processed by AP Sync state machine. */
			case SUBTYPE_BEACON:
				*Machine = AP_SYNC_STATE_MACHINE;
				*MsgType = APMT2_PEER_BEACON;
				break;

			/* Only Sta have chance to receive Probe-Rsp. */
			case SUBTYPE_PROBE_RSP:
				*Machine = APCLI_SYNC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_PROBE_RSP;
				break;

			default:
				return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}


/* 
    ==========================================================================
    Description:
        MLME message sanity check
    Return:
        TRUE if all parameters are OK, FALSE otherwise
        
    IRQL = DISPATCH_LEVEL

    ==========================================================================
 */
BOOLEAN ApCliPeerAssocRspSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *pMsg, 
    IN ULONG MsgLen, 
    OUT PUCHAR pAddr2, 
    OUT USHORT *pCapabilityInfo, 
    OUT USHORT *pStatus, 
    OUT USHORT *pAid, 
    OUT UCHAR SupRate[], 
    OUT UCHAR *pSupRateLen,
    OUT UCHAR ExtRate[], 
    OUT UCHAR *pExtRateLen,
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag,
    OUT IE_LISTS *ie_list) 
{
	CHAR          IeType;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT   pEid;
	ULONG         Length = 0;
    
	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	Length += LENGTH_802_11;
        
	NdisMoveMemory(pCapabilityInfo, &pFrame->Octet[0], 2);
	Length += 2;
	NdisMoveMemory(pStatus,         &pFrame->Octet[2], 2);
	Length += 2;
	*pCkipFlag = 0;
	*pExtRateLen = 0;
	pEdcaParm->bValid = FALSE;
    
	if (*pStatus != MLME_SUCCESS) 
		return TRUE;
    
	NdisMoveMemory(pAid, &pFrame->Octet[4], 2);
	Length += 2;

	/* Aid already swaped byte order in RTMPFrameEndianChange() for big endian platform */
	*pAid = (*pAid) & 0x3fff; /* AID is low 14-bit */
        
	/* -- get supported rates from payload and advance the pointer */
	IeType = pFrame->Octet[6];
	*pSupRateLen = pFrame->Octet[7];
	if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): fail - wrong SupportedRates IE\n", __FUNCTION__));
		return FALSE;
	}
	else 
		NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

	Length = Length + 2 + *pSupRateLen;

	/* many AP implement proprietary IEs in non-standard order, we'd better */
	/* tolerate mis-ordered IEs to get best compatibility */
	pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];
            
	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen)
	{
		switch (pEid->Eid)
		{
			case IE_EXT_SUPP_RATES:
				if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES)
				{
					NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
					*pExtRateLen = pEid->Len;
				}
				break;
#ifdef DOT11_N_SUPPORT
			case IE_HT_CAP:
			case IE_HT_CAP2:
				if (pEid->Len >= SIZE_HT_CAP_IE)  /*Note: allow extension.!! */
				{
					NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);
					*(USHORT *) (&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
					*(USHORT *) (&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
					*pHtCapabilityLen = SIZE_HT_CAP_IE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_HT_CAP\n", __FUNCTION__));
				}
				
				break;
			case IE_ADD_HT:
			case IE_ADD_HT2:
				if (pEid->Len >= sizeof(ADD_HT_INFO_IE))				
				{
					/* This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only */
					/* copy first sizeof(ADD_HT_INFO_IE) */
					NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
					*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_ADD_HT\n", __FUNCTION__));
				}
				break;
			case IE_SECONDARY_CH_OFFSET:
				if (pEid->Len == 1)
				{
					*pNewExtChannelOffset = pEid->Octet[0];
				}
				else
				{
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_SECONDARY_CH_OFFSET\n", __FUNCTION__));
				}
				break;
#ifdef DOT11_VHT_AC
			case IE_VHT_CAP:
				if (pEid->Len == sizeof(VHT_CAP_IE)) {
					NdisMoveMemory(&ie_list->vht_cap, pEid->Octet, sizeof(VHT_CAP_IE));
					ie_list->vht_cap_len = sizeof(VHT_CAP_IE);
				} else {
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_VHT_CAP\n", __FUNCTION__));
				}
				break;
			case IE_VHT_OP:
				if (pEid->Len == sizeof(VHT_OP_IE)) {
					NdisMoveMemory(&ie_list->vht_op, pEid->Octet, sizeof(VHT_OP_IE));
					ie_list->vht_op_len = sizeof(VHT_OP_IE);
				}else {
					DBGPRINT(RT_DEBUG_WARN, ("%s():wrong IE_VHT_OP\n", __FUNCTION__));
				}
				break;		
#endif /* DOT11_VHT_AC */				
#endif /* DOT11_N_SUPPORT */
			/* CCX2, WMM use the same IE value */
			/* case IE_CCX_V2: */
			case IE_VENDOR_SPECIFIC:
				/* handle WME PARAMTER ELEMENT */
				if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24))
				{
					PUCHAR ptr;
					int i;
        
					/* parsing EDCA parameters */
					pEdcaParm->bValid          = TRUE;
					pEdcaParm->bQAck           = FALSE; /* pEid->Octet[0] & 0x10; */
					pEdcaParm->bQueueRequest   = FALSE; /* pEid->Octet[0] & 0x20; */
					pEdcaParm->bTxopRequest    = FALSE; /* pEid->Octet[0] & 0x40; */
					/*pEdcaParm->bMoreDataAck    = FALSE; // pEid->Octet[0] & 0x80; */
					pEdcaParm->EdcaUpdateCount = pEid->Octet[6] & 0x0f;
					pEdcaParm->bAPSDCapable    = (pEid->Octet[6] & 0x80) ? 1 : 0;
					ptr = (PUCHAR) &pEid->Octet[8];
					for (i=0; i<4; i++)
					{
						UCHAR aci = (*ptr & 0x60) >> 5; /* b5~6 is AC INDEX */
						pEdcaParm->bACM[aci]  = (((*ptr) & 0x10) == 0x10);   /* b5 is ACM */
						pEdcaParm->Aifsn[aci] = (*ptr) & 0x0f;               /* b0~3 is AIFSN */
						pEdcaParm->Cwmin[aci] = *(ptr+1) & 0x0f;             /* b0~4 is Cwmin */
						pEdcaParm->Cwmax[aci] = *(ptr+1) >> 4;               /* b5~8 is Cwmax */
						pEdcaParm->Txop[aci]  = *(ptr+2) + 256 * (*(ptr+3)); /* in unit of 32-us */
						ptr += 4; /* point to next AC */
					}
				}
				break;
				default:
					DBGPRINT(RT_DEBUG_TRACE, ("%s():ignore unrecognized EID = %d\n", __FUNCTION__, pEid->Eid));
					break;
		}

		Length = Length + 2 + pEid->Len; 
		pEid = (PEID_STRUCT)((UCHAR*)pEid + 2 + pEid->Len);        
	}

	return TRUE;
}


MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR *pAddrs)
{
	ULONG ApCliIndex;
	PMAC_TABLE_ENTRY pCurEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (!VALID_WCID(wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	do
	{
		pCurEntry = &pAd->MacTab.Content[wcid];

		ApCliIndex = 0xff;
		if ((pCurEntry) && IS_ENTRY_APCLI(pCurEntry))
		{
			ApCliIndex = pCurEntry->wdev_idx;
		}

		if ((ApCliIndex == 0xff) || (ApCliIndex >= MAX_APCLI_NUM))
			break;

		if (pAd->ApCfg.ApCliTab[ApCliIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddrs))
		{
			pEntry = pCurEntry;
			break;
		}
	} while(FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);

	return pEntry;
}


/*
	==========================================================================
	Description:
		Check the Apcli Entry is valid or not.
	==========================================================================
 */
static inline BOOLEAN ValidApCliEntry(RTMP_ADAPTER *pAd, INT apCliIdx)
{
	BOOLEAN result;
	PMAC_TABLE_ENTRY pMacEntry;
	APCLI_STRUCT *pApCliEntry;

	do
	{
		if ((apCliIdx < 0) || (apCliIdx >= MAX_APCLI_NUM))
		{
			result = FALSE;
			break;
		}

		pApCliEntry = (APCLI_STRUCT *)&pAd->ApCfg.ApCliTab[apCliIdx];
		if (pApCliEntry->Valid != TRUE)
		{
			result = FALSE;
			break;
		}

		if ((pApCliEntry->MacTabWCID <= 0) 
			|| (pApCliEntry->MacTabWCID >= MAX_LEN_OF_MAC_TABLE))
		{
			result = FALSE;
			break;
		}
	
		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		if (!IS_ENTRY_APCLI(pMacEntry))
		{
			result = FALSE;
			break;
		}
			
		result = TRUE;
	} while(FALSE);

	return result;
}


INT ApCliAllowToSendPacket(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pPacket,
	OUT UCHAR *pWcid)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	APCLI_STRUCT *apcli_entry;


	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
	{	
		apcli_entry = &pAd->ApCfg.ApCliTab[idx];
		if (apcli_entry && (&apcli_entry->wdev == wdev))
		{
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef MWDS
				&& (apcli_entry->bEnableMWDS == FALSE)
#endif /* MWDS */
				)
			{
				PUCHAR pSrcBufVA = NULL;
				PACKET_INFO PacketInfo;
				UINT SrcBufLen;
				PMAC_TABLE_ENTRY pEntry = NULL;
				REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
				UCHAR isLinkValid;

				RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, (pSrcBufVA + MAC_ADDR_LEN), TRUE, &isLinkValid);
				if (pReptEntry)
				{
					*pWcid = pReptEntry->MacTabWCID;
					return TRUE;
				}
				else
				{
					if (!isLinkValid)
						return FALSE;
				

					if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN))
					{
						pEntry = &pAd->MacTab.Content[pAd->ApCfg.ApCliTab[idx].MacTabWCID];
						if ((pEntry) && (pEntry->PortSecured == WPA_802_1X_PORT_SECURED))
						{
							RTMPInsertRepeaterEntry(pAd, idx, (pSrcBufVA + MAC_ADDR_LEN));
							DBGPRINT(RT_DEBUG_TRACE, (" Receive trigger packet !!!\n"));
							return FALSE;
						}
					}
				}

				*pWcid = apcli_entry->MacTabWCID;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount ++;
				RTMP_SET_PACKET_NET_DEVICE_APCLI(pPacket, idx);
				*pWcid = apcli_entry->MacTabWCID;
			}
			allowed = TRUE;
			break;
		}
	}

	return allowed;
}


/*
	========================================================================
	
	Routine Description:
		Validate the security configuration against the RSN information 
		element

	Arguments:
		pAdapter	Pointer	to our adapter
		eid_ptr 	Pointer to VIE
		
	Return Value:
		TRUE 	for configuration match 
		FALSE	for otherwise
		
	Note:
		
	========================================================================
*/
BOOLEAN ApCliValidateRSNIE(
	IN RTMP_ADAPTER *pAd, 
	IN PEID_STRUCT pEid_ptr,
	IN USHORT eid_len,
	IN USHORT idx)
{
	PUCHAR pVIE, pTmp;
	UCHAR len;
	PEID_STRUCT         pEid;			
	CIPHER_SUITE		WPA;			/* AP announced WPA cipher suite */
	CIPHER_SUITE		WPA2;			/* AP announced WPA2 cipher suite */
	USHORT Count;
	UCHAR Sanity;	 
	PAPCLI_STRUCT pApCliEntry = NULL;
	PRSN_IE_HEADER_STRUCT pRsnHeader;
	NDIS_802_11_ENCRYPTION_STATUS	TmpCipher;
	NDIS_802_11_AUTHENTICATION_MODE TmpAuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA_AuthModeAux;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthMode;
	NDIS_802_11_AUTHENTICATION_MODE WPA2_AuthModeAux;
	struct wifi_dev *wdev;

	pVIE = (PUCHAR) pEid_ptr;
	len	 = eid_len;

	/*if (len >= MAX_LEN_OF_RSNIE || len <= MIN_LEN_OF_RSNIE) */
	/*	return FALSE; */

	/* Init WPA setting */
	WPA.PairCipher    	= Ndis802_11WEPDisabled;
	WPA.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA.RsnCapability 	= 0;
	WPA.bMixMode      	= FALSE;
	WPA_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA_AuthModeAux		= Ndis802_11AuthModeOpen;	

	/* Init WPA2 setting */
	WPA2.PairCipher    	= Ndis802_11WEPDisabled;
	WPA2.PairCipherAux 	= Ndis802_11WEPDisabled;
	WPA2.GroupCipher   	= Ndis802_11WEPDisabled;
	WPA2.RsnCapability 	= 0;
	WPA2.bMixMode      	= FALSE;
	WPA2_AuthMode	  	= Ndis802_11AuthModeOpen;
	WPA2_AuthModeAux	= Ndis802_11AuthModeOpen;

	Sanity = 0;

	/* 1. Parse Cipher this received RSNIE */
	while (len > 0)
	{		
		pTmp = pVIE;
		pEid = (PEID_STRUCT) pTmp;	

		switch(pEid->Eid)
		{
			case IE_WPA:
				if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1)
				{
					/* if unsupported vendor specific IE */
					break;
				}	
				/* Skip OUI ,version and multicast suite OUI */
				pTmp += 11;

				/* 
					Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
						Value      Meaning
						0           None
						1           WEP-40
						2           Tkip
						3           WRAP
						4           AES
						5           WEP-104
				*/
				/* Parse group cipher */
				switch (*pTmp)
				{
					case 1:
					case 5:	/* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
						WPA.GroupCipher = Ndis802_11WEPEnabled;
						break;
					case 2:
						WPA.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						WPA.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}

				/* number of unicast suite */
				pTmp += 1;

				/* Store unicast cipher count */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);		

				/* pointer to unicast cipher */
			    pTmp += sizeof(USHORT);	

				/* Parsing all unicast cipher suite */
				while (Count > 0)
				{
					/* Skip cipher suite OUI */
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;							
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;						
							break;
						default:
							break;
					}
					if (TmpCipher > WPA.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux */
						WPA.PairCipherAux = WPA.PairCipher;
						WPA.PairCipher    = TmpCipher;
					}
					else
					{
						WPA.PairCipherAux = TmpCipher;
					}
					pTmp++;
					Count--;
				}
			
				/* Get AKM suite counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);		

				pTmp   += sizeof(USHORT);

				/* Parse AKM ciphers */
				/* Parsing all AKM cipher suite */
				while (Count > 0)
				{
			    	/* Skip cipher suite OUI */
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{	
						case 1:
							/* WPA-enterprise */
							TmpAuthMode = Ndis802_11AuthModeWPA;							
							break;
						case 2:
							/* WPA-personal */
							TmpAuthMode = Ndis802_11AuthModeWPAPSK;									    	
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA_AuthMode)
					{
						/* Move the lower AKM suite to WPA_AuthModeAux */
						WPA_AuthModeAux = WPA_AuthMode;
						WPA_AuthMode    = TmpAuthMode;
					}
					else
					{
						WPA_AuthModeAux = TmpAuthMode;
					}
				    pTmp++;
					Count--;										
				}

				/* ToDo - Support WPA-None ? */

				/* Check the Pair & Group, if different, turn on mixed mode flag */
				if (WPA.GroupCipher != WPA.PairCipher)
					WPA.bMixMode = TRUE;

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA1 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
											((WPA.bMixMode) ? "Mix" : GetEncryptType(WPA.PairCipher)), 
											GetEncryptType(WPA.GroupCipher),
											GetAuthMode(WPA_AuthMode)));

				Sanity |= 0x1;
				break; /* End of case IE_WPA */
			case IE_RSN:
				pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
				
				/* 0. Version must be 1 */
				/*  The pRsnHeader->Version exists in native little-endian order, so we may need swap it for RT_BIG_ENDIAN systems. */
				if (le2cpu16(pRsnHeader->Version) != 1)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - RSN Version isn't 1(%d) \n", pRsnHeader->Version));
					break;
				}	

				pTmp   += sizeof(RSN_IE_HEADER_STRUCT);

				/* 1. Check cipher OUI */
				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
				{
					/* if unsupported vendor specific IE */
					break;
				}

				/* Skip cipher suite OUI */
				pTmp += 3;

				/* Parse group cipher */
				switch (*pTmp)
				{
					case 1:
					case 5:	/* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
						WPA2.GroupCipher = Ndis802_11WEPEnabled;
						break;
					case 2:
						WPA2.GroupCipher = Ndis802_11TKIPEnable;
						break;
					case 4:
						WPA2.GroupCipher = Ndis802_11AESEnable;
						break;
					default:
						break;
				}

				/* number of unicast suite */
				pTmp += 1;

				/* Get pairwise cipher counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);
				
				pTmp   += sizeof(USHORT);

				/* 3. Get pairwise cipher */
				/* Parsing all unicast cipher suite */
				while (Count > 0)
				{
					/* Skip OUI */
					pTmp += 3;
					TmpCipher = Ndis802_11WEPDisabled;
					switch (*pTmp)
					{
						case 1:
						case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway */
							TmpCipher = Ndis802_11WEPEnabled;
							break;
						case 2:
							TmpCipher = Ndis802_11TKIPEnable;							
							break;
						case 4:
							TmpCipher = Ndis802_11AESEnable;							
							break;
						default:
							break;
					}
					if (TmpCipher > WPA2.PairCipher)
					{
						/* Move the lower cipher suite to PairCipherAux */
						WPA2.PairCipherAux = WPA2.PairCipher;
						WPA2.PairCipher    = TmpCipher;
					}
					else
					{
						WPA2.PairCipherAux = TmpCipher;
					}
					pTmp ++;
					Count--;
				}

				/* Get AKM suite counts */
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);		

				pTmp   += sizeof(USHORT);

				/* Parse AKM ciphers */
				/* Parsing all AKM cipher suite */
				while (Count > 0)
				{
			    	/* Skip cipher suite OUI */
					pTmp   += 3;
					TmpAuthMode = Ndis802_11AuthModeOpen;
					switch (*pTmp)
					{	
						case 1:
							/* WPA2-enterprise */
							TmpAuthMode = Ndis802_11AuthModeWPA2;							
							break;
						case 2:
							/* WPA2-personal */
							TmpAuthMode = Ndis802_11AuthModeWPA2PSK;									    	
							break;
						default:
							break;
					}
					if (TmpAuthMode > WPA2_AuthMode)
					{
						/* Move the lower AKM suite to WPA2_AuthModeAux */
						WPA2_AuthModeAux = WPA2_AuthMode;
						WPA2_AuthMode    = TmpAuthMode;
					}
					else
					{
						WPA2_AuthModeAux = TmpAuthMode;
					}
				    pTmp++;
					Count--;										
				}

				/* Check the Pair & Group, if different, turn on mixed mode flag */
				if (WPA2.GroupCipher != WPA2.PairCipher)
					WPA2.bMixMode = TRUE;

				DBGPRINT(RT_DEBUG_TRACE, ("ApCliValidateRSNIE - RSN-WPA2 PairWiseCipher(%s), GroupCipher(%s), AuthMode(%s)\n",
									(WPA2.bMixMode ? "Mix" : GetEncryptType(WPA2.PairCipher)), GetEncryptType(WPA2.GroupCipher),
									GetAuthMode(WPA2_AuthMode)));

				Sanity |= 0x2;
				break; /* End of case IE_RSN */
			default:
					DBGPRINT(RT_DEBUG_WARN, ("ApCliValidateRSNIE - Unknown pEid->Eid(%d) \n", pEid->Eid));
				break;
		}

		/* skip this Eid */
		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);
	
	}

	/* 2. Validate this RSNIE with mine */
	pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
	wdev = &pApCliEntry->wdev;

	/* Peer AP doesn't include WPA/WPA2 capable */
	if (Sanity == 0) 
	{
		/* Check the authenticaton mode */		
		if (wdev->AuthMode >= Ndis802_11AuthModeWPA)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("%s - The authentication mode doesn't match \n", __FUNCTION__));
			return FALSE;
		}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("%s - The pre-RSNA authentication mode is used. \n", __FUNCTION__));
			return TRUE;
		}
	}
	else if (wdev->AuthMode < Ndis802_11AuthModeWPA)
	{
		/* Peer AP has RSN capability, but our AP-Client is pre-RSNA. Discard this */
		DBGPRINT(RT_DEBUG_ERROR, ("%s - The authentication mode doesn't match. AP is WPA security but APCLI is not. \n", __FUNCTION__));
		return FALSE;
	}


	/* Recovery user-defined cipher suite */
	pApCliEntry->PairCipher  = wdev->WepStatus;
	pApCliEntry->GroupCipher = wdev->WepStatus;
	pApCliEntry->bMixCipher  = FALSE;

	if (wdev->bWpaAutoMode == TRUE)
	{
		if (Sanity == 0x2)
		{
			DBGPRINT(RT_DEBUG_TRACE,("WPA_AUTO Mode ==> peerAp: Rsn IE\n"));
			wdev->AuthMode = WPA2_AuthMode;
		}
		else if (Sanity == 0x3)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode ==> peerAp: including Rsn/WPA IE (DUAL)\n"));
			wdev->AuthMode = WPA2_AuthMode;
		}
		else if (Sanity == 0x1)
        	{
                	DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode ==> peerAp: WPA IE\n"));
                	wdev->AuthMode = WPA_AuthMode;
    	}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode ==> peerAp: no Rsn/WPA IE\n"));
			return FALSE;
		}
	}

    if(wdev->bEncryptAutoMode == TRUE)
    {
        if (Sanity == 0x2)
		{
			DBGPRINT(RT_DEBUG_TRACE,("ENCRYPT_AUTO Mode ==> peerAp: Rsn IE\n"));
			wdev->WepStatus = WPA2.PairCipher;
		}
		else if (Sanity == 0x3)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode ==> peerAp: including Rsn/WPA IE (DUAL)\n"));
            wdev->WepStatus = WPA2.PairCipher;
		}
		else if (Sanity == 0x1)
    	{
        	DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode ==> peerAp: WPA IE\n"));
                	wdev->WepStatus = WPA.PairCipher;
        	}
		else
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode ==> peerAp: no Rsn/WPA IE\n"));
			return FALSE;
		}
	}

	Sanity = 0;	
	
	/* Check AuthMode and WPA_AuthModeAux for matching, in case AP support dual-AuthMode */
	/* WPAPSK */
	if ((WPA_AuthMode == wdev->AuthMode) || 
		((WPA_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA_AuthModeAux == wdev->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA.bMixMode == FALSE)
		{
			if (wdev->WepStatus != WPA.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (wdev->WepStatus < WPA.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
			(wdev->WepStatus != WPA.PairCipher) && 
			(wdev->WepStatus != WPA.PairCipherAux))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA validate pairwise cipher error \n"));
			return FALSE;
		}	

		Sanity |= 0x1;
	}
	/* WPA2PSK */
	else if ((WPA2_AuthMode == wdev->AuthMode) || 
			 ((WPA2_AuthModeAux != Ndis802_11AuthModeOpen) && (WPA2_AuthModeAux == wdev->AuthMode)))
	{
		/* Check cipher suite, AP must have more secured cipher than station setting */
		if (WPA2.bMixMode == FALSE)
		{
			if (wdev->WepStatus != WPA2.GroupCipher)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate cipher suite error \n"));
				return FALSE;
			}
		}

		/* check group cipher */
		if (wdev->WepStatus < WPA2.GroupCipher)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate group cipher error \n"));
			return FALSE;
		}

		/*
			check pairwise cipher, skip if none matched
				If profile set to AES, let it pass without question.
				If profile set to TKIP, we must find one mateched
		*/
		if ((wdev->WepStatus == Ndis802_11TKIPEnable) && 
			(wdev->WepStatus != WPA2.PairCipher) && 
			(wdev->WepStatus != WPA2.PairCipherAux))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - WPA2 validate pairwise cipher error \n"));
			return FALSE;
		}

		Sanity |= 0x2;
	}

	if (Sanity == 0) 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - Validate RSIE Failure \n"));
		return FALSE;
	}

	/*Re-assign pairwise-cipher and group-cipher. Re-build RSNIE. */
	if ((wdev->AuthMode == Ndis802_11AuthModeWPA) || (wdev->AuthMode == Ndis802_11AuthModeWPAPSK))
	{
		pApCliEntry->GroupCipher = WPA.GroupCipher;
			
		if (wdev->WepStatus == WPA.PairCipher)
			pApCliEntry->PairCipher = WPA.PairCipher;
		else if (WPA.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11TKIPEnable;			
	}
	else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2) || (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK))
	{
		pApCliEntry->GroupCipher = WPA2.GroupCipher;
			
		if (wdev->WepStatus == WPA2.PairCipher)
			pApCliEntry->PairCipher = WPA2.PairCipher;
		else if (WPA2.PairCipherAux != Ndis802_11WEPDisabled)
			pApCliEntry->PairCipher = WPA2.PairCipherAux;
		else	/* There is no PairCipher Aux, downgrade our capability to TKIP */
			pApCliEntry->PairCipher = Ndis802_11TKIPEnable;					
	}

	/* Set Mix cipher flag */
	if (pApCliEntry->PairCipher != pApCliEntry->GroupCipher)
	{
		pApCliEntry->bMixCipher = TRUE;	

		/* re-build RSNIE */
		/*RTMPMakeRSNIE(pAd, pApCliEntry->AuthMode, pApCliEntry->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI)); */
	}
	
	/* re-build RSNIE */
	RTMPMakeRSNIE(pAd, wdev->AuthMode, wdev->WepStatus, (idx + MIN_NET_DEVICE_FOR_APCLI));
	
	return TRUE;	
}


BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UCHAR FromWhichBSSID)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#ifdef WSC_AP_SUPPORT
        PWSC_CTRL    pWpsCtrl = &pAd->ApCfg.ApCliTab[0].WscControl;
#endif /* WSC_AP_SUPPORT */

	/* 
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client, 
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta4_mac_addr
	*/
	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev_idx];																											
			
	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */
#ifdef MWDS
	if(IS_MWDS_OPMODE_APCLI(pEntry) &&
	   MAC_ADDR_EQUAL(pHeader->Octet, pApCliEntry->wdev.if_addr))
			return FALSE;
#endif /* MWDS */

	if (MAC_ADDR_EQUAL(pHeader->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn)
	{
		UCHAR isLinkValid;
	
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader->Addr3, TRUE, &isLinkValid);
		if (pReptEntry)
			return FALSE;	/* give up this frame */
	}
#endif /* MAC_REPEATER_SUPPORT */


							
	/* skip the 802.11 header */
#ifdef MWDS
	if(IS_MWDS_OPMODE_APCLI(pEntry))
	{
		pRxBlk->pData += LENGTH_802_11_WITH_ADDR4;
		pRxBlk->DataSize -= LENGTH_802_11_WITH_ADDR4;
	}
	else
#endif /* MWDS */
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
	}

	/* Use software to decrypt the encrypted frame. */
	/* Because this received frame isn't my BSS frame, Asic passed to driver without decrypting it. */
	/* If receiving an "encrypted" unicast packet(its WEP bit as 1) and doesn't match my BSSID, it */
	/* pass to driver with "Decrypted" marked as 0 in RxD. */
	if ((pRxInfo->MyBss == 0) && (pRxInfo->Decrypted == 0) && (pHeader->FC.Wep == 1)
		&& pEntry->PortSecured==WPA_802_1X_PORT_SECURED
#ifdef WSC_AP_SUPPORT		
		&& pWpsCtrl->bWscTrigger == FALSE 	
#endif /*WSC_AP_SUPPORT*/		
		) 
	{
		NDIS_STATUS NStatus;
		
		NStatus = RTMPSoftDecryptionAction(pAd, 
									 (PUCHAR)pHeader, 0, 
									 &pApCliEntry->SharedKey[pRxBlk->key_idx], 
									 pRxBlk->pData, 
									 &(pRxBlk->DataSize));
		if (NStatus != NDIS_STATUS_SUCCESS)
		{						
#ifdef APCLI_CERT_SUPPORT						
			if (NStatus == NDIS_STATUS_MICERROR && pApCliEntry->GroupCipher == Ndis802_11TKIPEnable)
			ApCliRTMPReportMicError( pAd,&pApCliEntry->SharedKey[pRxBlk->key_idx],pEntry->wdev_idx);
#endif /* APCLI_CERT_SUPPORT */
			return FALSE;  /* give up this frame */
		}
	}
	pRxInfo->MyBss = 1;
#ifdef APCLI_SUPPORT
	rx_get_pn(pRxBlk,pRxInfo);
#endif /* APCLI_SUPPORT */
#ifdef HDR_TRANS_SUPPORT
	if (pRxBlk->bHdrRxTrans)
		Indicate_Legacy_Packet_Hdr_Trns(pAd, pRxBlk, FromWhichBSSID);
	else
#endif /* HDR_TRANS_SUPPORT */
	Indicate_Legacy_Packet(pAd, pRxBlk, FromWhichBSSID);								

	return TRUE;
}


VOID APCliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UINT8	BssIdx;

	IfIdx = pEntry->wdev_idx;
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + IfIdx;
#ifdef MAC_APCLI_SUPPORT
	BssIdx = APCLI_BSSID_IDX + IfIdx;
#endif /* MAC_APCLI_SUPPORT */
	NdisMoveMemory(pAd->ApCfg.ApCliTab[IfIdx].PTK, pEntry->PTK, LEN_PTK);

	WPAInstallPairwiseKey(pAd, BssIdx, pEntry, FALSE);
}


BOOLEAN APCliInstallSharedKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UCHAR	GTK_len = 0;
	APCLI_STRUCT *apcli_entry;


	if (!pEntry || !IS_ENTRY_APCLI(pEntry))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : This Entry doesn't exist!!! \n", __FUNCTION__));		
		return FALSE;
	}

	IfIdx = pEntry->wdev_idx;
	ASSERT((IfIdx < MAX_APCLI_NUM));

	apcli_entry = &pAd->ApCfg.ApCliTab[IfIdx];
	if (apcli_entry->GroupCipher == Ndis802_11TKIPEnable && KeyLen >= LEN_TKIP_GTK)
		GTK_len = LEN_TKIP_GTK;
	else if (apcli_entry->GroupCipher == Ndis802_11AESEnable && KeyLen >= LEN_AES_GTK)
		GTK_len = LEN_AES_GTK;
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s : GTK is invalid (GroupCipher=%d, DataLen=%d) !!! \n", 
								__FUNCTION__, apcli_entry->GroupCipher, KeyLen));		
		return FALSE;
	}

	/* Update GTK */
	/* set key material, TxMic and RxMic for WPAPSK */
	NdisMoveMemory(apcli_entry->GTK, pKey, GTK_len);
	apcli_entry->wdev.DefaultKeyId = DefaultKeyIdx;

	/* Update shared key table */
	NdisZeroMemory(&apcli_entry->SharedKey[DefaultKeyIdx], sizeof(CIPHER_KEY));  
	apcli_entry->SharedKey[DefaultKeyIdx].KeyLen = GTK_len;
	NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].Key, pKey, LEN_TK);
	if (GTK_len == LEN_TKIP_GTK)
	{
		NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].RxMic, pKey + 16, LEN_TKIP_MIC);
		NdisMoveMemory(apcli_entry->SharedKey[DefaultKeyIdx].TxMic, pKey + 24, LEN_TKIP_MIC);
	}

	/* Update Shared Key CipherAlg */
	apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_NONE;
	if (apcli_entry->GroupCipher == Ndis802_11TKIPEnable)
		apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_TKIP;
	else if (apcli_entry->GroupCipher == Ndis802_11AESEnable)
		apcli_entry->SharedKey[DefaultKeyIdx].CipherAlg = CIPHER_AES;

#ifdef MAC_APCLI_SUPPORT
	RTMP_ASIC_SHARED_KEY_TABLE(pAd, 
								APCLI_BSS_BASE + IfIdx, 
								DefaultKeyIdx, 
								&apcli_entry->SharedKey[DefaultKeyIdx]);
#endif /* MAC_APCLI_SUPPORT */

	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd 				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
// TODO: shiang-6590, modify this due to it's really a duplication of "RTMPUpdateMlmeRate()" in common/mlme.c
VOID ApCliUpdateMlmeRate(RTMP_ADAPTER *pAd, USHORT ifIndex)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54; */
	UCHAR	i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;
	UCHAR   PhyMode = pAd->CommonCfg.PhyMode;

	PAPCLI_STRUCT pApCliEntry = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
#ifdef APCLI_AUTO_BW_SUPPORT
	PhyMode = pApCliEntry->wdev.PhyMode;	
#endif /* APCLI_AUTO_BW_SUPPORT */

	switch (PhyMode) 
	{
		case (WMODE_B):
			ProperMlmeRate = RATE_11;
			MinimumRate = RATE_1;
			break;
		case (WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
		case (WMODE_A |WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
		case (WMODE_B | WMODE_G | WMODE_GN):
#ifdef DOT11_VHT_AC
		case (WMODE_A |WMODE_B | WMODE_G | WMODE_GN | WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			if ((pApCliEntry->MlmeAux.SupRateLen == 4) &&
				(pApCliEntry->MlmeAux.ExtRateLen == 0))
				ProperMlmeRate = RATE_11; /* B only AP */
			else
				ProperMlmeRate = RATE_24;
			
			if (pApCliEntry->MlmeAux.Channel <= 14)
				MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		case (WMODE_A):
#ifdef DOT11_N_SUPPORT
		case (WMODE_GN):
		case (WMODE_G | WMODE_GN):
		case (WMODE_A | WMODE_G | WMODE_AN | WMODE_GN):
		case (WMODE_A | WMODE_AN):
		case (WMODE_AN):	
#ifdef DOT11_VHT_AC
		case (WMODE_AC):
		case (WMODE_AN | WMODE_AC):
		case (WMODE_A | WMODE_AN | WMODE_AC):
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */
			ProperMlmeRate = RATE_24;
			MinimumRate = RATE_6;
			break;
		case (WMODE_B | WMODE_A | WMODE_G):
			ProperMlmeRate = RATE_24;
			if (pApCliEntry->MlmeAux.Channel <= 14)
			   MinimumRate = RATE_1;
			else
				MinimumRate = RATE_6;
			break;
		default: /* error */
			ProperMlmeRate = RATE_1;
			MinimumRate = RATE_1;
			break;
	}

	for (i = 0; i < pApCliEntry->MlmeAux.SupRateLen; i++)
	{
		for (j = 0; j < RateIdx; j++)
		{
			if ((pApCliEntry->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j])
			{
				if (j == ProperMlmeRate)
				{
					bMatch = TRUE;
					break;
				}
			}			
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE)
	{
		for (i = 0; i < pApCliEntry->MlmeAux.ExtRateLen; i++)
		{
			for (j = 0; j < RateIdx; j++)
			{
				if ((pApCliEntry->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j])
				{
					if (j == ProperMlmeRate)
					{
						bMatch = TRUE;
						break;
					}
				}			
			}
		
			if (bMatch)
				break;
		}
	}

	if (bMatch == FALSE)
		ProperMlmeRate = MinimumRate;

	if(!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED))
	{	
		pAd->CommonCfg.MlmeRate = MinimumRate;
		pAd->CommonCfg.RtsRate = ProperMlmeRate;
		if (pAd->CommonCfg.MlmeRate >= RATE_6)
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_OFDM;
			pAd->CommonCfg.MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		}
		else
		{
			pAd->CommonCfg.MlmeTransmit.field.MODE = MODE_CCK;
			pAd->CommonCfg.MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s():=>MlmeTransmit=0x%x, MinimumRate=%d, ProperMlmeRate=%d\n",
				__FUNCTION__, pAd->CommonCfg.MlmeTransmit.word, MinimumRate, ProperMlmeRate));
}


VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR Channel)
{
	UCHAR ifIndex;
	APCLI_STRUCT *pApCliEntry;
	
	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];


		if (pApCliEntry->bPeerExist == TRUE)
			continue;

		else if (Channel == pAd->CommonCfg.Channel &&
			((SsidLen == pApCliEntry->CfgSsidLen && NdisEqualMemory(Ssid, pApCliEntry->CfgSsid, SsidLen)) ||
			SsidLen == 0 /* Hidden */))
		{
			pApCliEntry->bPeerExist = TRUE;
		}
		else
		{
			/* No Root AP match the SSID */
		}
	}
}


VOID APCli_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
	PNET_DEV new_dev_p;
	INT idx;
	APCLI_STRUCT *pApCliEntry;
	struct wifi_dev *wdev;

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_apcli_init != FALSE)
		return;


	/* init */
	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
		pAd->ApCfg.ApCliTab[idx].wdev.if_dev = NULL;

	/* create virtual network interface */
	for (idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		UINT32 MC_RowID = 0, IoctlIF = 0;
		char *dev_name;

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */

	dev_name = get_dev_name_prefix(pAd, INT_APCLI);
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, idx,
									sizeof(struct mt_dev_priv), dev_name);
		if (!new_dev_p) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s(): Create net_device for %s(%d) fail!\n",
						__FUNCTION__, dev_name, idx));
			break;
		}
#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		
		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];

		NdisZeroMemory(&pApCliEntry->ApCliCounter, sizeof(APCLI_COUNTER));

#ifdef APCLI_CONNECTION_TRIAL
		pApCliEntry->ifIndex = idx;
		pApCliEntry->pAd = pAd;
#endif /* APCLI_CONNECTION_TRIAL */
		pApCliEntry->OpenWEPErrPktChk = FALSE;
		pApCliEntry->OpenWEPErrPktCnt = 0;
		pApCliEntry->OpenWEPErrMCPktCnt = 0;
		wdev = &pApCliEntry->wdev;
		wdev->wdev_type = WDEV_TYPE_STA;
		wdev->func_dev = pApCliEntry;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = new_dev_p;
		wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		NdisZeroMemory(&pApCliEntry->ApCliCounter, sizeof(APCLI_COUNTER));

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);

		if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
		{
			if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0)) 
			{
				UCHAR MacMask = 0;

				if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
					MacMask = 0xFE;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
					MacMask = 0xFC;
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
					MacMask = 0xF8;
#ifdef SPECIFIC_BCN_BUF_SUPPORT
				else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 16)
					MacMask = 0xF0;
#endif /* SPECIFIC_BCN_BUF_SUPPORT */

				idx = (pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1;

				/* 	
					Refer to HW definition - 
						Bit1 of MAC address Byte0 is local administration bit 
						and should be set to 1 in extended multiple BSSIDs'
						Bit3~ of MAC address Byte0 is extended multiple BSSID index.
				 */
				if (pAd->chipCap.MBSSIDMode == MBSSID_MODE1)
				{
					/* 	
						Refer to HW definition - 
							Bit1 of MAC address Byte0 is local administration bit 
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
			 		*/ 
					wdev->if_addr[0] &= (MacMask << 2);
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[0] |= (idx << 2);
				}
#ifdef ENHANCE_NEW_MBSSID_MODE
				else
				{
					wdev->if_addr[0] |= 0x2;
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
					wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] |= idx;
				}
#endif /* ENHANCE_NEW_MBSSID_MODE */
			}
		}
		else
		{
			wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
		}
		
		pNetDevOps->priv_flags = INT_APCLI; /* we are virtual interface */
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
	}

	pAd->flg_apcli_init = TRUE;
}


VOID ApCli_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		wdev = &pAd->ApCfg.ApCliTab[index].wdev;
		if (wdev->if_dev)
		{
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			rtmp_wdev_idx_unreg(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);

			/* Clear it as NULL to prevent latter access error. */
			pAd->flg_apcli_init = FALSE;
			wdev->if_dev = NULL;
		}
	}
}


BOOLEAN ApCli_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	UCHAR ifIndex;
#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_probe_adapter)
		wf_fwd_probe_adapter(pAd);
#endif 

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].wdev.if_dev == dev_p)
		{
			RTMP_OS_NETDEV_START_QUEUE(dev_p);
			pAd->ApCfg.ApCliTab[ifIndex].Valid = FALSE;
			ApCliIfUp(pAd);
#ifdef WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_UP, NULL, NULL, 0);
#endif /* WPA_SUPPLICANT_SUPPORT */
			return TRUE;
		}
	}

	return FALSE;
}


BOOLEAN ApCli_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p)
{
	UCHAR ifIndex;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
	struct wifi_dev *wdev;
	APCLI_STRUCT *apcli_entry;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[ifIndex];
		wdev = &apcli_entry->wdev;
		if (wdev->if_dev == dev_p)
		{
#ifdef WPA_SUPPLICANT_SUPPORT
			RtmpOSWrielessEventSend(pAd->net_dev, RT_WLAN_EVENT_CUSTOM, RT_INTERFACE_DOWN, NULL, NULL, 0);

			if (apcli_entry->wpa_supplicant_info.pWpaAssocIe)
			{
				os_free_mem(NULL, apcli_entry->wpa_supplicant_info.pWpaAssocIe);
				apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
				apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
			}
#endif /* WPA_SUPPLICANT_SUPPORT */

			RTMP_OS_NETDEV_STOP_QUEUE(dev_p);

			/* send disconnect-req to sta State Machine. */
			if (apcli_entry->Enable)
			{
#ifdef MAC_REPEATER_SUPPORT
				if (pAd->ApCfg.bMACRepeaterEn)
				{
					for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
					{
						if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
						{
#ifdef DOT11_N_SUPPORT
							/* free resources of BA*/
							BASessionTearDownALL(pAd, pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID);
#endif /* DOT11_N_SUPPORT */
							RTMPRemoveRepeaterDisconnectEntry(pAd, ifIndex, CliIdx);
							RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
						}
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef SMART_MESH
				/* To avoid  to connect to previous bssid setting after interface up later. */
				NdisZeroMemory(apcli_entry->CfgApCliBssid, MAC_ADDR_LEN);
#endif /* SMART_MESH */

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
			}
			return TRUE;
		}
	}

	return FALSE;
}

BOOLEAN ApCli_StatsGet(
	IN	PRTMP_ADAPTER pAd,
	IN	RT_CMD_STATS64 *pStats)
{
	INT ifIndex = 0, index;
	APCLI_STRUCT *pApCliTab;

	for(index = 0; index < MAX_APCLI_NUM; index++)
	{
		if (pAd->ApCfg.ApCliTab[index].wdev.if_dev == pStats->pNetDev)
		{
			ifIndex = index;
			break;
		}
	}

	if (index >= MAX_APCLI_NUM)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("rt28xx_ioctl apcli_statsGet can not find apcli I/F\n"));
		return FALSE;
	}

	pApCliTab = &pAd->ApCfg.ApCliTab[ifIndex];

	pStats->rx_bytes = pApCliTab->ApCliCounter.ReceivedByteCount.QuadPart;
	pStats->tx_bytes = pApCliTab->ApCliCounter.TransmittedByteCount.QuadPart;

	pStats->rx_packets = pApCliTab->ApCliCounter.ReceivedFragmentCount;
	pStats->tx_packets = pApCliTab->ApCliCounter.TransmittedFragmentCount;

	pStats->rx_errors = pApCliTab->ApCliCounter.RxErrors;
	pStats->multicast = pApCliTab->ApCliCounter.MulticastReceivedFrameCount;

	return TRUE;
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT

BOOLEAN ApCliSetIfState(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN state)
{
	pAd->ApCfg.ApCliTab[ifIndex].Enable = state;

#ifdef APCLI_CONNECTION_TRIAL
	if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
	ApCliIfDown(pAd);

	return TRUE;
}


BOOLEAN ApCliSetBssid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR *Bssid)
{
	BOOLEAN apcliEn = pAd->ApCfg.ApCliTab[ifIndex].Enable;

	/* bring apcli interface down first */
	if (apcliEn == TRUE)
	{
		pAd->ApCfg.ApCliTab[ifIndex].Enable = FALSE;
#ifdef APCLI_CONNECTION_TRIAL
		if (pAd->ApCfg.ApCliTab[ifIndex].TrialCh == 0)
#endif /* APCLI_CONNECTION_TRIAL */
		ApCliIfDown(pAd);
	}

	NdisCopyMemory(pAd->ApCfg.ApCliTab[ifIndex].CfgApCliBssid, Bssid, MAC_ADDR_LEN);

	pAd->ApCfg.ApCliTab[ifIndex].Enable = apcliEn;

	return TRUE;
}


BOOLEAN ApCliAutoConnectStart(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	AP_ADMIN_CONFIG *pApCfg = &pAd->ApCfg;
	NDIS_802_11_SSID Ssid;

#ifdef WSC_AP_SUPPORT
	if ((pApCfg->ApCliTab[ifIndex].WscControl.WscConfMode != WSC_DISABLE) &&
	    (pApCfg->ApCliTab[ifIndex].WscControl.bWscTrigger == TRUE))
		return FALSE;
#endif /* WSC_AP_SUPPORT */

	if (pApCfg->ApCliAutoConnectRunning == FALSE)
	{
		ApCliSetIfState(pAd, ifIndex, FALSE);
		pApCfg->ApCliAutoConnectRunning = TRUE;
	}
	else
	{
		return TRUE;
	}

	/*
		use site survey function to trigger auto connecting (when pAd->ApCfg.ApAutoConnectRunning == TRUE)
	*/
	NdisZeroMemory(&Ssid, sizeof(NDIS_802_11_SSID));
	ApCliSiteSurvey(pAd, ifIndex, &Ssid, SCAN_ACTIVE, FALSE);

	return TRUE;
}

/* 
	===================================================
	
	Description:
		Find the AP that is configured in the ApcliTab, and switch to 
		the channel of that AP
		
	Arguments:
		pAd: pointer to our adapter

	Return Value:
		TRUE: no error occured 
		FALSE: otherwise

	Note:
	===================================================
*/
BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd)
{
	POS_COOKIE  	pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR			ifIdx, CfgSsidLen, entryIdx;
	STRING			*pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;
	PAPCLI_STRUCT pApCliEntry = NULL;
	struct wifi_dev *wdev;
	
	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliAutoConnectExec()\n"));

	pObj->ioctl_if_type = INT_APCLI;

	for(ifIdx=0; ifIdx<MAX_APCLI_NUM; ifIdx++)
	{
		if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag == TRUE)
			break;
	}

	if(ifIdx >= MAX_APCLI_NUM)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error  ifIdx=%d\n", ifIdx));
		return FALSE;
	}

#ifdef WSC_AP_SUPPORT
	if ((pAd->ApCfg.ApCliTab[ifIdx].WscControl.WscConfMode != WSC_DISABLE) &&
	    (pAd->ApCfg.ApCliTab[ifIdx].WscControl.bWscTrigger == TRUE))
	{
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		return FALSE;
	}
#endif /* WSC_AP_SUPPORT */

	CfgSsidLen = pAd->ApCfg.ApCliTab[ifIdx].CfgSsidLen;
	pCfgSsid = pAd->ApCfg.ApCliTab[ifIdx].CfgSsid;
	pScanTab = &pAd->ScanTab;
	pSsidBssTab = &pAd->MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;
	
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIdx];
    wdev = &pApCliEntry->wdev;
	/*
		Find out APs with the desired SSID.  
	*/
	for (entryIdx=0; entryIdx<pScanTab->BssNr;entryIdx++)
	{
		BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];
        BOOLEAN bAddEntry = FALSE;
		
		if ( pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
							(CfgSsidLen == pBssEntry->SsidLen) &&
							(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE))
		{

			if (wdev->bWpaAutoMode == TRUE)
        	{
				DBGPRINT(RT_DEBUG_TRACE,("WPA_AUTO Mode under the APCLI_AUTO_CONNECT mode\n"));
				if (pBssEntry->WPA2.PairCipher != Ndis802_11WEPDisabled)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode peerAp: RSN IE (ApCliAutoConnect)\n"));
					wdev->AuthMode =  pBssEntry->AuthMode;
				}
				else if (pBssEntry->WPA.PairCipher != Ndis802_11WEPDisabled)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode peerAp: WPA IE (ApCliAutoConnect)\n"));
					wdev->AuthMode =  pBssEntry->AuthMode;
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("WPA_AUTO Mode peerAp: no Rsn/WPA IE (ApCliAutoConnect)\n"));
				}
			}
      
            if (wdev->bEncryptAutoMode == TRUE)
			{
				DBGPRINT(RT_DEBUG_TRACE,("ENCRYPT_AUTO Mode under the APCLI_AUTO_CONNECT mode\n"));
				if (pBssEntry->WPA2.PairCipher != Ndis802_11WEPDisabled)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode peerAp: RSN IE (ApCliAutoConnect)\n"));
					wdev->WepStatus = pBssEntry->WPA2.PairCipher;
				}
				else if (pBssEntry->WPA.PairCipher != Ndis802_11WEPDisabled)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode peerAp: WPA IE (ApCliAutoConnect)\n"));
                                        wdev->WepStatus = pBssEntry->WPA.PairCipher;
				}
				else
				{
					DBGPRINT(RT_DEBUG_TRACE, ("ENCRYPT_AUTO Mode peerAp: no Rsn/WPA IE (ApCliAutoConnect)\n"));
				}
			}
			
			if (ApcliCompareAuthEncryp(&pAd->ApCfg.ApCliTab[ifIdx],
										pBssEntry->AuthMode,
										pBssEntry->AuthModeAux,
										pBssEntry->WepStatus,
										pBssEntry->WPA) ||
				ApcliCompareAuthEncryp(&pAd->ApCfg.ApCliTab[ifIdx],
										pBssEntry->AuthMode,
										pBssEntry->AuthModeAux,
										pBssEntry->WepStatus,
										pBssEntry->WPA2))
			{
				DBGPRINT(RT_DEBUG_TRACE, 
						("Found desired ssid in Entry %2d:\n", entryIdx));
				DBGPRINT(RT_DEBUG_TRACE,
						("I/F(%s%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n", 
						INF_APCLI_DEV_NAME, ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						pBssEntry->Channel, pBssEntry->Rssi));
				DBGPRINT(RT_DEBUG_TRACE,
						("I/F(%s%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", INF_APCLI_DEV_NAME, ifIdx,
						GetAuthMode(pBssEntry->AuthMode),
						GetEncryptType(pBssEntry->WepStatus)) );
                
                if(MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
                {
                    bAddEntry = TRUE;
#ifdef SMART_MESH
                    if(!pApCliEntry->SmartMeshCfg.bHiFiPeerFilter || pBssEntry->bHyperFiPeer)
                        bAddEntry = TRUE;
                    else
                        bAddEntry = FALSE;
#endif /* SMART_MESH */
                }
                else if(MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, pBssEntry->Bssid))
                    bAddEntry = TRUE;
                
                if(bAddEntry)
                    NdisMoveMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++],pBssEntry, sizeof(BSS_ENTRY));
			} 
		}		
	}

	NdisZeroMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	
	/*
		Sort by Rssi in the increasing order, and connect to
		the last entry (strongest Rssi)
	*/
	BssTableSortByRssi(pSsidBssTab, TRUE);

	
	if ((pSsidBssTab->BssNr == 0))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("No match entry.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}
	else if (pSsidBssTab->BssNr > 0 &&
			pSsidBssTab->BssNr <=MAX_LEN_OF_BSS_TABLE)
	{	
		/*
			Switch to the channel of the candidate AP
		*/
		UCHAR	CandidateAPChannel[8];
		UCHAR *pCandidateAPBssid = NULL;
		BOOLEAN bChangeToCandidateAP = TRUE;
		
		if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		{
			/*
				We don't want this feature to impact desired bssid.
			*/
			if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid))
			{
				bChangeToCandidateAP = FALSE;
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
				DBGPRINT(RT_DEBUG_TRACE, 
						("%s: This candidate AP(%02X:%02X:%02X:%02X:%02X:%02X) not match with CfgApCliBssid(%02X:%02X:%02X:%02X:%02X:%02X).\n",
						__FUNCTION__,PRINT_MAC(pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid),
						PRINT_MAC(pApCliEntry->CfgApCliBssid)));
			}
		}
		else
		{
			pCandidateAPBssid = pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid;
		}
		
		if(bChangeToCandidateAP)
		{
			BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1];
			
			if (pAd->CommonCfg.Channel != pBssEntry->Channel)
			{
				ApCliSetIfState(pAd, ifIdx, FALSE);
				
				sprintf(CandidateAPChannel, "%d", pBssEntry->Channel);
				DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", CandidateAPChannel));
				Set_Channel_Proc(pAd, CandidateAPChannel);
			}
			
			if (pCandidateAPBssid)
				ApCliSetBssid(pAd, ifIdx, pCandidateAPBssid);
		}
	}
	else 
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr) );
		ApCliSetIfState(pAd, ifIdx, TRUE);
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}

	ApCliSetIfState(pAd, ifIdx, TRUE);
	DBGPRINT(RT_DEBUG_TRACE, ("<--- ApCliAutoConnectExec()\n"));

	return TRUE;
}

/* 
	===================================================
	
	Description:
		If the previous selected entry connected failed, this function will
		choose next entry to connect. The previous entry will be deleted.
				
	Arguments:
		pAd: pointer to our adapter

	Note:
		Note that the table is sorted by Rssi in the "increasing" order, thus
		the last entry in table has stringest Rssi.
	===================================================
*/

VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	BSS_TABLE 		*pSsidBssTab;
	PAPCLI_STRUCT	pApCliEntry;
	UCHAR			lastEntryIdx;

#ifdef AP_PARTIAL_SCAN_SUPPORT
	if (pAd->ApCfg.bPartialScanning == TRUE)
		return;
#endif /* AP_PARTIAL_SCAN_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
	pSsidBssTab = &pAd->MlmeAux.SsidBssTab;
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	if (pSsidBssTab->BssNr == 0)
	{
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		goto exit_and_enable;
	}

	/*
		delete (zero) the previous connected-failled entry and always 
		connect to the last entry in talbe until the talbe is empty.
	*/
	NdisZeroMemory(&pSsidBssTab->BssEntry[--pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	lastEntryIdx = pSsidBssTab->BssNr -1;
	
	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE))
	{
		UCHAR	CandidateAPChannel[8];
		UCHAR *pCandidateAPBssid = NULL;
		BOOLEAN bChangeToCandidateAP = TRUE;

		if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, ZERO_MAC_ADDR))
		{
			/*
			We don't want this feature to impact desired bssid.
			*/
			if(!MAC_ADDR_EQUAL(pApCliEntry->CfgApCliBssid, pSsidBssTab->BssEntry[lastEntryIdx].Bssid))
			{
				bChangeToCandidateAP = FALSE;
				pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
				DBGPRINT(RT_DEBUG_TRACE, 
						("%s: This candidate AP(%02X:%02X:%02X:%02X:%02X:%02X) not match with CfgApCliBssid(%02X:%02X:%02X:%02X:%02X:%02X).\n",
						__FUNCTION__,PRINT_MAC(pSsidBssTab->BssEntry[lastEntryIdx].Bssid),PRINT_MAC(pApCliEntry->CfgApCliBssid)));
			}
		}
		else
		{
			pCandidateAPBssid = pSsidBssTab->BssEntry[lastEntryIdx].Bssid;
		}

		if(bChangeToCandidateAP)
		{
			BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[lastEntryIdx];
			
			if (pAd->CommonCfg.Channel != pBssEntry->Channel)
			{
				ApCliSetIfState(pAd, ifIndex, FALSE);
				
				sprintf(CandidateAPChannel, "%d", pBssEntry->Channel);
				DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", CandidateAPChannel));
				Set_Channel_Proc(pAd, CandidateAPChannel);
			}
			
			if (pCandidateAPBssid)
				ApCliSetBssid(pAd, ifIndex, pCandidateAPBssid);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}

exit_and_enable:

	ApCliSetIfState(pAd, ifIndex, TRUE);
	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
}


BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE WPA)
{
	NDIS_802_11_AUTHENTICATION_MODE	tempAuthMode = pApCliEntry->wdev.AuthMode;
	NDIS_802_11_WEP_STATUS tempWEPstatus = pApCliEntry->wdev.WepStatus;

	DBGPRINT(RT_DEBUG_TRACE, ("ApcliAuthMode=%s, AuthMode=%s, AuthModeAux=%s, ApcliWepStatus=%s,	WepStatus=%s, GroupCipher=%s, PairCipher=%s,  \n",
					GetAuthMode(pApCliEntry->wdev.AuthMode),
					GetAuthMode(AuthMode),
					GetAuthMode(AuthModeAux),
					GetEncryptType(pApCliEntry->wdev.WepStatus),
					GetEncryptType(WEPstatus),
					GetEncryptType(WPA.GroupCipher),
					GetEncryptType(WPA.PairCipher)));

	if (tempAuthMode <= Ndis802_11AuthModeAutoSwitch)
	{
		tempAuthMode = Ndis802_11AuthModeOpen;
		return ((tempAuthMode == AuthMode || 		
				tempAuthMode == AuthModeAux) &&
				(tempWEPstatus == WEPstatus) );
	}
	else if (tempAuthMode <= Ndis802_11AuthModeWPA2PSK)
	{
		return ((tempAuthMode == AuthMode || 		
			tempAuthMode == AuthModeAux) &&
			(tempWEPstatus == WPA.GroupCipher||
			tempWEPstatus == WPA.PairCipher) );
	}
	else
	{
		/* not supported cases */
		return FALSE;
	}

}

VOID RTMPApCliReconnectionCheck(
	IN PRTMP_ADAPTER pAd)
{
	INT i;
	PCHAR	pApCliSsid, pApCliCfgSsid;
	UCHAR	CfgSsidLen;
	NDIS_802_11_SSID Ssid;
	
	if (pAd->ApCfg.ApCliAutoConnectRunning == FALSE)
	{
		for (i = 0; i < MAX_APCLI_NUM; i++)
		{
			pApCliSsid = pAd->ApCfg.ApCliTab[i].Ssid;
			pApCliCfgSsid = pAd->ApCfg.ApCliTab[i].CfgSsid;
			CfgSsidLen = pAd->ApCfg.ApCliTab[i].CfgSsidLen;
			if ((pAd->ApCfg.ApCliTab[i].CtrlCurrState < APCLI_CTRL_AUTH ||
				!NdisEqualMemory(pApCliSsid, pApCliCfgSsid, CfgSsidLen)) &&
				pAd->ApCfg.ApCliTab[i].CfgSsidLen > 0 && 
				pAd->Mlme.OneSecPeriodicRound % 10 == 0)
			{
				DBGPRINT(RT_DEBUG_TRACE, (" %s(): Scan channels for AP (%s)\n", 
							__FUNCTION__, pApCliCfgSsid));
				pAd->ApCfg.ApCliAutoConnectRunning = TRUE;
				Ssid.SsidLength = CfgSsidLen;
				NdisCopyMemory(Ssid.Ssid, pApCliCfgSsid, CfgSsidLen);
				ApSiteSurvey(pAd, &Ssid, SCAN_ACTIVE, FALSE);
				
			}	
		}
	}
}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
/*
	===================================================

	Description:
		
	When root AP is Open WEP, it will cause a fake connection state if user keys in
	wrong password. So we need this to fix the issue.		

	Arguments:
		pAd: pointer to our adapter
		pRxBlk: carry necessary packet info 802.11 format
		bSuccessPkt: see if it is a successfully decrypted packet.
	Note:
	===================================================
*/
VOID ApCliRxOpenWEPCheck(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN BOOLEAN bSuccessPkt)
{
	APCLI_STRUCT *pApCliEntry = NULL;
	MAC_TABLE_ENTRY *pEntry = NULL;
	PHEADER_802_11	pHeader = pRxBlk->pHeader;
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	BOOLEAN isMCPkt = FALSE;
	int wcid;

	if(!pRxBlk || !pHeader || !pRxInfo)
		return;
	
	wcid = pRxBlk->wcid;
	if((wcid >= WCID_ALL) || (wcid == MCAST_WCID))
		return;

	if (VALID_WCID(wcid))
		pEntry = ApCliTableLookUpByWcid(pAd, wcid, pHeader->Addr2);
	else
		pEntry = MacTableLookup(pAd, pHeader->Addr2);

	if(!pEntry || !IS_ENTRY_APCLI(pEntry)) 
	   return;

	if((pEntry->wdev_idx == 0xff) || 
	   (pEntry->wdev_idx >= MAX_APCLI_NUM))
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->wdev_idx];
	if(!pApCliEntry || 
	   !pApCliEntry->Valid ||
	   !pApCliEntry->OpenWEPErrPktChk) 
	   return;

	if (pRxInfo->Mcast || pRxInfo->Bcast)
		isMCPkt = TRUE;
		
	if(bSuccessPkt)
	{
		if((pHeader->FC.Type == FC_TYPE_DATA) &&
		   (pHeader->FC.SubType != SUBTYPE_DATA_NULL) &&
		   (pHeader->FC.SubType != SUBTYPE_QOS_NULL) &&
		   (pRxBlk->DataSize > 0))
		{
			pApCliEntry->OpenWEPErrPktCnt = 0;
			pApCliEntry->OpenWEPErrMCPktCnt = 0;
			pApCliEntry->OpenWEPErrPktChk = FALSE;
		}
	}
	else
	{
		pApCliEntry->OpenWEPErrPktCnt++;
		if(pApCliEntry->OpenWEPErrPktCnt >= OPENWEP_ERRPKT_MAX_COUNT)
		{
			pEntry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pApCliEntry->OpenWEPErrPktChk = FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("(%s:%d): Stop Open WEP check!\n",__func__,__LINE__));
		}
		if(isMCPkt) pApCliEntry->OpenWEPErrMCPktCnt++;
	}
}

#endif /* APCLI_SUPPORT */

