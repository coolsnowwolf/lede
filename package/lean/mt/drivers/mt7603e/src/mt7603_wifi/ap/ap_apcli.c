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

    4. ApCli index (0) of different tx packet is assigned in

    5. ApCli index (0) of different interface is got in APHardTransmit() by using

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
#ifdef ROAMING_ENHANCE_SUPPORT
#include <net/arp.h>
#endif /* ROAMING_ENHANCE_SUPPORT */

static VOID set_mlme_rsn_ie(PRTMP_ADAPTER pAd, struct wifi_dev *wdev, PMAC_TABLE_ENTRY pEntry)
{
	UINT32 Idx;
	
	/* Set New WPA information */
	Idx = BssTableSearch(&pAd->ScanTab, pEntry->Addr, wdev->channel);
	if (Idx == BSS_NOT_FOUND) {
		DBGPRINT(RT_DEBUG_TRACE, ("ASSOC - Can't find BSS after receiving Assoc response\n"));
	} else {
		/* Init variable */
		pEntry->RSNIE_Len = 0;
		NdisZeroMemory(pEntry->RSN_IE, MAX_LEN_OF_RSNIE);

		/* Store appropriate RSN_IE for WPA SM negotiation later */
		if ((wdev->AuthMode >= Ndis802_11AuthModeWPA)
			&& (Idx < MAX_LEN_OF_BSS_TABLE)
			&& (pAd->ScanTab.BssEntry[Idx].VarIELen != 0)) {
			PUCHAR pVIE;
			USHORT len;
			PEID_STRUCT pEid;

			pVIE = pAd->ScanTab.BssEntry[Idx].VarIEs;
			len = pAd->ScanTab.BssEntry[Idx].VarIELen;

			while (len > 0) {
				pEid = (PEID_STRUCT) pVIE;
				/* For WPA/WPAPSK */
				if ((pEid->Eid == IE_WPA)
					&& (NdisEqualMemory(pEid->Octet, WPA_OUI, 4))
					&& ((Ndis802_11AuthModeWPA == wdev->AuthMode)
					|| (Ndis802_11AuthModeWPAPSK == wdev->AuthMode))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					DBGPRINT(RT_DEBUG_TRACE,("%s():=> Store RSN_IE for WPA SM negotiation\n", __FUNCTION__));
				}
				/* For WPA2/WPA2PSK */
				else if ((pEid->Eid == IE_RSN)
					 && (NdisEqualMemory(pEid->Octet + 2, RSN_OUI, 3))
					 && ((Ndis802_11AuthModeWPA2 == wdev->AuthMode)
					|| (Ndis802_11AuthModeWPA2PSK == wdev->AuthMode)
#ifdef APCLI_SAE_SUPPORT
					|| (Ndis802_11AuthModeWPA3PSK == wdev->AuthMode)
#endif
#ifdef APCLI_OWE_SUPPORT
					|| (Ndis802_11AuthModeOWE == wdev->AuthMode)
#endif
					)) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					DBGPRINT(RT_DEBUG_TRACE,("%s():=> Store RSN_IE for WPA2 SM negotiation\n", __FUNCTION__));
				}
				
#ifdef WAPI_SUPPORT
				/* For WAPI */
				else if ((pEid->Eid == IE_WAPI)
					 && (NdisEqualMemory(pEid->Octet + 4, WAPI_OUI, 3))
					 && ((Ndis802_11AuthModeWAICERT == wdev->AuthMode)
					|| (Ndis802_11AuthModeWAIPSK == wdev->AuthMode))) {
					NdisMoveMemory(pEntry->RSN_IE, pVIE, (pEid->Len + 2));
					pEntry->RSNIE_Len = (pEid->Len + 2);
					DBGPRINT(RT_DEBUG_TRACE,("%s():=> Store RSN_IE for WAPI SM negotiation\n", __FUNCTION__));
				}
#endif /* WAPI_SUPPORT */

				pVIE += (pEid->Len + 2);
				len -= (pEid->Len + 2);
			}

		}

		if (pEntry->RSNIE_Len == 0) {
			DBGPRINT(RT_DEBUG_TRACE, ("%s():=> no RSN_IE\n", __FUNCTION__));
		} else {
			hex_dump("RSN_IE", pEntry->RSN_IE, pEntry->RSNIE_Len);
		}
	}

}



BOOLEAN ApCliWaitProbRsp(PRTMP_ADAPTER pAd, USHORT ifIndex)
{
        if (ifIndex >= MAX_APCLI_NUM)
                return FALSE;

	printk("%s()[%d]: %lu\n", __FUNCTION__, ifIndex, pAd->ApCfg.ApCliTab[ifIndex].SyncCurrState);
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

BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE WPA);


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
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth;
	aux_ht_cap->HtCapInfo.GF =  pHtCapability->HtCapInfo.GF & rt_ht_cap->GF;



	/* Send Assoc Req with my HT capability. */
	aux_ht_cap->HtCapInfo.AMsduSize =  rt_ht_cap->AmsduSize;
	aux_ht_cap->HtCapInfo.MimoPs = pHtCapability->HtCapInfo.MimoPs;
	aux_ht_cap->HtCapInfo.ShortGIfor20 =  (rt_ht_cap->ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	aux_ht_cap->HtCapInfo.ShortGIfor40 =  (rt_ht_cap->ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	aux_ht_cap->HtCapInfo.TxSTBC =  (rt_ht_cap->TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	aux_ht_cap->HtCapInfo.RxSTBC =  (rt_ht_cap->RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);
	
	/* Fix throughput issue for some vendor AP with AES mode */
	if (pAddHtInfo->AddHtInfo.RecomWidth & rt_ht_cap->ChannelWidth)
		aux_ht_cap->HtCapInfo.CCKmodein40 = pHtCapability->HtCapInfo.CCKmodein40;
	else 
		aux_ht_cap->HtCapInfo.CCKmodein40 = 0;
	
	aux_ht_cap->HtCapParm.MaxRAmpduFactor = rt_ht_cap->MaxRAmpduFactor;
	aux_ht_cap->HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	aux_ht_cap->ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;
	if (pAd->CommonCfg.bRdg)
	{
		aux_ht_cap->ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;
	}

	/*COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability); */
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */

#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN

static VOID ApCliCompleteInit(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_INIT_COMPLETION(&pApCliEntry->ifdown_complete);
	RTMP_OS_INIT_COMPLETION(&pApCliEntry->linkdown_complete);
}

static VOID ApCliLinkDownComplete(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_COMPLETE(&pApCliEntry->linkdown_complete);
}

static VOID ApCliWaitLinkDown(APCLI_STRUCT *pApCliEntry)
{
	if (pApCliEntry->Valid &&
		!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pApCliEntry->linkdown_complete, APCLI_WAIT_TIMEOUT)) {
		DBGPRINT(RT_DEBUG_ERROR,
		("(%s) ApCli [%d] can't done.\n", __func__, pApCliEntry->wdev.func_idx));
	}
}

static VOID ApCliWaitIfDown(APCLI_STRUCT *pApCliEntry)
{
	if (pApCliEntry->Valid &&
		!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pApCliEntry->ifdown_complete, APCLI_WAIT_TIMEOUT)) {
		DBGPRINT(RT_DEBUG_ERROR,
			("%s: wait ApCli [%d] interface down failed!!\n", __func__, pApCliEntry->wdev.func_idx));
	}
}

static VOID ApCliWaitStateDisconnect(APCLI_STRUCT *pApCliEntry)
{
	/*
	* Before doing WifiSysClose,we have to make sure the ctrl
	* state machine has switched to APCLI_CTRL_DISCONNECTED
	*/
	int wait_cnt = 0;
	int wait_times = 50;
	int delay_time = 100;

	while(pApCliEntry->CtrlCurrState != APCLI_CTRL_DISCONNECTED) {
	if (wait_cnt >= wait_times) {
		DBGPRINT(RT_DEBUG_ERROR,
			("Need to debug apcli ctrl state machine(Ctrl State=%lu)\n\r",
			pApCliEntry->CtrlCurrState));
		break;
	}
	RtmpOsMsDelay(delay_time);
	wait_cnt++;
	}
}

static VOID ApCliIfDownComplete(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_COMPLETE(&pApCliEntry->ifdown_complete);
}
#endif
#endif

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
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
#if defined(MAC_REPEATER_SUPPORT) || defined(MT_MAC)
	UCHAR CliIdx = 0xFF;
#ifdef MAC_REPEATER_SUPPORT
	INVAILD_TRIGGER_MAC_ENTRY *pSkipEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* defined(MAC_REPEATER_SUPPORT) || defined(MT_MAC) */

	do
	{
		if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			|| (ifIndex >= 64)
#endif /* MAC_REPEATER_SUPPORT */
		)
		{
#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				if (ifIndex < MAX_APCLI_NUM)
				{
#ifdef LINUX
					struct net_device *pNetDev;
					struct net *net= &init_net;

/* old kernerl older than 2.6.21 didn't have for_each_netdev()*/
#ifndef for_each_netdev
					for(pNetDev=dev_base; pNetDev!=NULL; pNetDev=pNetDev->next)
#else
					for_each_netdev(net, pNetDev)
#endif
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
#endif
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

			if (ifIndex < MAX_APCLI_NUM) {
				DBGPRINT(RT_DEBUG_TRACE, 
("!!! APCLI LINK UP - IF(apcli%d) AuthMode(%d)=%s, WepStatus(%d)=%s !!!\n", ifIndex, 
pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode, 
GetAuthMode(pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode), 
pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus, 
GetEncryptType(pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus)));
			}
		}
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : APCLI LINK UP - IF(apcli%d)!!!\n", ifIndex));
			result = FALSE;
			break;
		}

#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx != 0xFF)
			DBGPRINT(RT_DEBUG_ERROR, ("(%s) ifIndex = %d, CliIdx = %d !!!\n", __FUNCTION__, ifIndex, CliIdx));
#endif /* MAC_REPEATER_SUPPORT */

		if (ifIndex >= MAX_APCLI_NUM) {
			result = FALSE;
			break;
		}
		
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		if ((pApCliEntry != NULL) && (pApCliEntry->Valid)
#ifdef MAC_REPEATER_SUPPORT
			 && (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
			)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : This link had existed - IF(apcli%d)!!!\n", ifIndex));
			result = FALSE;
			break;
		}

		wdev = &pApCliEntry->wdev;
		DBGPRINT(RT_DEBUG_TRACE, ("!!! APCLI LINK UP - IF(apcli%d) AuthMode(%d)=%s, WepStatus(%d)=%s!\n",
					ifIndex,
					wdev->AuthMode, GetAuthMode(wdev->AuthMode),
					wdev->WepStatus, GetEncryptType(wdev->WepStatus)));

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT
		if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (wf_fwd_get_rep_hook)
				wf_fwd_get_rep_hook(pAd->CommonCfg.EtherTrafficBand);

#if (MT7615_MT7603_COMBO_FORWARDING == 1)
			if (wf_fwd_check_device_hook)
				wf_fwd_check_device_hook(wdev->if_dev, INT_APCLI, CliIdx, pAd->CommonCfg.Channel, 1);			
#endif
			if (wf_fwd_entry_insert_hook)
				wf_fwd_entry_insert_hook (wdev->if_dev, pAd->net_dev, pAd);

			if (wf_fwd_insert_repeater_mapping_hook)
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#ifdef MAC_REPEATER_SUPPORT
				wf_fwd_insert_repeater_mapping_hook (pAd, &pAd->ApCfg.ReptCliEntryLock, &pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0], &pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
#else
				wf_fwd_insert_repeater_mapping_hook (pAd, NULL, NULL, NULL, &pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
#endif /* MAC_REPEATER_SUPPORT */
#else
				wf_fwd_insert_repeater_mapping_hook (pAd, &pAd->ApCfg.ReptCliEntryLock, &pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0], &pAd->ApCfg.ApCliTab[0].wdev.if_addr);
#endif			
		}
#endif /* CONFIG_WIFI_PKT_FWD */

		/* Insert the Remote AP to our MacTable. */
		/*pMacEntry = MacTableInsertApCliEntry(pAd, (PUCHAR)(pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid)); */
#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn && (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_RTMP))
			pMacEntry = RTMPInsertRepeaterMacEntry(
							pAd,
							(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
							wdev,
							(ifIndex + MIN_NET_DEVICE_FOR_APCLI),
							CliIdx,
							TRUE);
		else
#endif /* MAC_REPEATER_SUPPORT */
			pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pApCliEntry->MlmeAux.Bssid),
										wdev, ENTRY_APCLI,
										OPMODE_AP, TRUE);

#ifdef MT_MAC
{
		if (CliIdx == 0xff && pAd->chipCap.hif_type == HIF_MT)
		{
			#ifdef MULTI_APCLI_SUPPORT
                	AsicUpdateRxWCIDTable(pAd, APCLI_MCAST_WCID(ifIndex), (PUCHAR)(pApCliEntry->MlmeAux.Bssid));	
			#else /* MULTI_APCLI_SUPPORT */
                	AsicUpdateRxWCIDTable(pAd, APCLI_MCAST_WCID, (PUCHAR)(pApCliEntry->MlmeAux.Bssid));
			#endif /*! MULTI_APCLI_SUPPORT */
		}
}
#endif /* MT_MAC */

		if (pMacEntry)
		{
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
			pMacEntry->Sst = SST_ASSOC;
			pMacEntry->wdev = &pApCliEntry->wdev;

#ifdef MULTI_APCLI_SUPPORT
			pMacEntry->wdev ->tr_tb_idx = APCLI_MCAST_WCID(ifIndex);
			tr_tb_set_mcast_entry(pAd, APCLI_MCAST_WCID(ifIndex), pMacEntry->wdev);
#endif /* MULTI_APCLI_SUPPORT */

#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx != 0xFF)
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

			set_mlme_rsn_ie(pAd, &pApCliEntry->wdev, pMacEntry);


#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
			if ((pApCliEntry->PmfCfg.MFPC) &&
				(pApCliEntry->MlmeAux.RsnCap.field.MFPC))
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_PMF_CAPABLE);

			if ((pApCliEntry->PmfCfg.PMFSHA256 &&
				pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation)
				|| (pApCliEntry->PmfCfg.MFPC &&
				pApCliEntry->MlmeAux.RsnCap.field.MFPR)
				|| (pApCliEntry->PmfCfg.MFPC &&
				pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation))
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_USE_SHA256);

			if ((pApCliEntry->PmfCfg.MFPR == TRUE) &&
				(pApCliEntry->MlmeAux.RsnCap.field.MFPC == FALSE)) {
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_PMF_CAPABLE);
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_USE_SHA256);
			}

			if (((pApCliEntry->PmfCfg.MFPC == FALSE) &&
				(pApCliEntry->MlmeAux.RsnCap.field.MFPC == FALSE)) ||
				((pApCliEntry->PmfCfg.MFPC == TRUE) &&
				(pApCliEntry->MlmeAux.RsnCap.field.MFPC == TRUE) &&
				(pApCliEntry->PmfCfg.MFPR == FALSE) &&
				(pApCliEntry->MlmeAux.RsnCap.field.MFPR == FALSE))) {
				if ((pApCliEntry->PmfCfg.PMFSHA256) &&
					(pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation == FALSE)) {
					CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_PMF_CAPABLE);
					CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_USE_SHA256);
				}
			}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */

			if (pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else
			{

					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_REPEATER_SUPPORT
				if (CliIdx != 0xFF)
					pApCliEntry->RepeaterCli[CliIdx].CliConnectState = 2;
#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
			if ((pAd->ApCfg.ApCliAutoConnectRunning == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
				(CliIdx == 0xFF) &&
#endif /* MAC_REPEATER_SUPPORT */
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
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
						&& (pMacEntry->AuthMode == Ndis802_11AuthModeWPA2
						|| pMacEntry->AuthMode == Ndis802_11AuthModeWPA2PSK
#ifdef APCLI_SAE_SUPPORT
						|| pMacEntry->AuthMode == Ndis802_11AuthModeWPA3PSK
#endif
#ifdef APCLI_OWE_SUPPORT
						|| pMacEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
						))
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

#ifdef MT_MAC										
							if (pAd->chipCap.hif_type == HIF_MT)
							{
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, pMacEntry->wcid, PAIRWISEKEYTABLE, pKey, pMacEntry->Addr);
#ifdef MULTI_APCLI_SUPPORT
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, APCLI_MCAST_WCID(ifIndex), SHAREDKEYTABLE, pKey, BROADCAST_ADDR);
#else /* MULTI_APCLI_SUPPORT */
								CmdProcAddRemoveKey(pAd, 0, pMacEntry->func_tb_idx, idx, APCLI_MCAST_WCID, SHAREDKEYTABLE, pKey, BROADCAST_ADDR);
#endif /* !MULTI_APCLI_SUPPORT */
							}
#endif /* MT_MAC */

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
					DBGPRINT(RT_DEBUG_TRACE, ("IF-apcli%d : Desired MCS = %d\n",
								ifIndex, wdev->DesiredTransmitSetting.field.MCS));

					set_ht_fixed_mcs(pAd, pMacEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pMacEntry->MaxHTPhyMode.field.STBC = (pHtCapability->HtCapInfo.RxSTBC & (pAd->CommonCfg.DesiredHtPhy.TxSTBC));
				pMacEntry->MpduDensity = pHtCapability->HtCapParm.MpduDensity;
				pMacEntry->MaxRAmpduFactor = pHtCapability->HtCapParm.MaxRAmpduFactor;
				pMacEntry->MmpsMode = (UCHAR)pHtCapability->HtCapInfo.MimoPs;
				pMacEntry->AMsduSize = (UCHAR)pHtCapability->HtCapInfo.AMsduSize;
				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
				if (pAd->CommonCfg.DesiredHtPhy.AmsduEnable && (pAd->CommonCfg.REGBACapability.field.AutoBA == FALSE))
					CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_AMSDU_INUSED);

				set_sta_ht_cap(pAd, pMacEntry, pHtCapability);

				NdisMoveMemory(&pMacEntry->HTCapability, &pApCliEntry->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);

#ifdef MT_MAC
                if (pAd->chipCap.hif_type == HIF_MT)
                    AsicUpdateRxWCIDTable(pAd, pMacEntry->wcid, (PUCHAR)(pApCliEntry->MlmeAux.Bssid));
#endif
			}
			else
			{
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				DBGPRINT(RT_DEBUG_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
								  RateIdToMbps[pMacEntry->MaxSupportedRate]));
			}
#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(pAd->CommonCfg.PhyMode) && pApCliEntry->MlmeAux.vht_cap_len &&  pApCliEntry->MlmeAux.vht_op_len)
				vht_mode_adjust(pAd, pMacEntry, &(pApCliEntry->MlmeAux.vht_cap), &(pApCliEntry->MlmeAux.vht_op));
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
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}
			else
			{
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			}

			set_sta_ra_cap(pAd, pMacEntry, pApCliEntry->MlmeAux.APRalinkIe);

#ifdef PIGGYBACK_SUPPORT
			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE))
			{
				RTMPSetPiggyBack(pAd, TRUE);
				DBGPRINT(RT_DEBUG_TRACE, ("Turn on Piggy-Back\n"));
			}
#endif /* PIGGYBACK_SUPPORT */

#ifdef WH_EZ_SETUP
			if (CliIdx == 0xff) {
				if (IS_EZ_SETUP_ENABLED(wdev)) {
					if (pApCliEntry->MlmeAux.support_easy_setup) {
						result = ez_port_secured(pAd, pMacEntry, ifIndex, FALSE);
						if(result == FALSE){
							EZ_DEBUG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,("ApCliLinkUp: connection failed/broken\n"));
							break;
						}
					}

#ifdef IF_UP_DOWN
					wdev->ez_driver_params.ez_apcli_peer_ap_config_cnt = 0;
#endif
				}
			}
#endif /* WH_EZ_SETUP */


#ifdef MWDS
			if((CliIdx == 0xff) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
			{
#ifdef WH_EZ_SETUP
				if (IS_AKM_PSK_Entry(pMacEntry))
					MWDSAPCliPeerEnable(pAd, pApCliEntry, pMacEntry);
				else
#endif /* WH_EZ_SETUP */
				{
					MWDSAPCliPeerEnable(pAd, pApCliEntry, pMacEntry);
#ifdef WH_EZ_SETUP
					//! If security is disabled on this link, 
					//!ez_hanle_pairmsg4 needs to be called to trigger config push
					if (IS_EZ_SETUP_ENABLED(wdev)
#ifdef WSC_AP_SUPPORT
						&& (!((pApCliEntry->WscControl.WscConfMode != WSC_DISABLE)
						&& (pApCliEntry->WscControl.bWscTrigger == TRUE)))
#endif /* WSC_AP_SUPPORT */
					) {
						ez_handle_pairmsg4(pAd, pMacEntry);
					}
#endif				
				}
			}
#endif /* MWDS */


			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);
			/* set the apcli interface be valid. */
#ifdef MAC_APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
#endif /* MAC_APCLI_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->wdev.allow_data_tx = TRUE;
				pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_SECURED;
#ifdef MAC_APCLI_SUPPORT
				AsicSetApCliBssid(pAd, pApCliEntry->MlmeAux.Bssid, ifIndex);
#endif /* MAC_APCLI_SUPPORT */
#ifdef MT_MAC
				if (pAd->chipCap.hif_type == HIF_MT)
				{
					#ifdef MULTI_APCLI_SUPPORT
					AsicSetBssid(pAd, pApCliEntry->MlmeAux.Bssid, (0x1 + ifIndex));					
					#else /* MULTI_APCLI_SUPPORT */
					AsicSetBssid(pAd, pApCliEntry->MlmeAux.Bssid, 0x1);
					#endif /* !MULTI_APCLI_SUPPORT */
				}
#endif
			}

#ifdef MT_MAC
			if (CliIdx != 0xff && pAd->chipCap.hif_type == HIF_MT)
			{
#ifdef MAC_REPEATER_SUPPORT
				insert_repeater_root_entry(pAd, pMacEntry, (PUCHAR)(pApCliEntry->MlmeAux.Bssid), CliIdx);
#endif /* MAC_REPEATER_SUPPORT */
			}
#endif /* MT_MAC */

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


#ifdef WSC_AP_SUPPORT
    /* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */

	if ((result == TRUE) && (ifIndex < MAX_APCLI_NUM) && 
		(pAd->ApCfg.ApCliTab[ifIndex].WscControl.WscConfMode == WSC_ENROLLEE) &&
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
        WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[ifIndex].WscControl);
    }
#endif /* WSC_AP_SUPPORT */

/* When root AP is Open WEP, it will cause a fake connection state if user keys in wrong password. */
	if((result == TRUE) &&
	   (wdev->AuthMode == Ndis802_11AuthModeOpen) &&
	   (wdev->WepStatus == Ndis802_11WEPEnabled))
		pApCliEntry->OpenWEPErrPktChk = TRUE;
	else
		pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;

#ifdef WH_EVENT_NOTIFIER
	if ((result == TRUE) && pMacEntry &&
		tr_entry && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED))
	{
		EventHdlr pEventHdlrHook = NULL;
		pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
		if(pEventHdlrHook && pMacEntry->wdev)
			pEventHdlrHook(pAd, pMacEntry, (UINT32)WHC_UPLINK_STAT_CONNECTED);
	}
#endif /* WH_EVENT_NOTIFIER */
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
	/*For security NONE & WEP case*/
	if ((CliIdx == 0xFF) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
		DBGPRINT(RT_DEBUG_OFF, ("APCLIENT MAP_ENABLE (No Security)\n"));
#ifdef A4_CONN
		map_a4_peer_enable(pAd, pMacEntry, FALSE);
#endif
		if (pApCliEntry->WscControl.bWscTrigger == FALSE)
			map_send_bh_sta_wps_done_event(pAd, pMacEntry, FALSE);
	}
#endif /*WAPP_SUPPORT*/
#ifdef MTFWD
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	RTMP_OS_NETDEV_CARRIER_ON(wdev->if_dev);
#endif

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
	UCHAR CliIdx = 0xFF;
#if (defined(WH_EVENT_NOTIFIER) && defined(MAC_REPEATER_SUPPORT))
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
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
			DBGPRINT(RT_DEBUG_ERROR, ("!!! REPEATER CLI LINK DOWN - IF(apcli%d) Cli %d !!!\n", ifIndex, CliIdx));
		}
		else
#endif /* MAC_REPEATER_SUPPORT */
		DBGPRINT(RT_DEBUG_OFF, ("!!! APCLI LINK DOWN - IF(apcli%d)!!!\n", ifIndex));
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("!!! ERROR : APCLI LINK DOWN - IF(apcli%d)!!!\n", ifIndex));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	if ((pApCliEntry->Valid == FALSE)
#ifdef MAC_REPEATER_SUPPORT
		&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		)
		return;

#ifdef MTFWD
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	RTMP_OS_NETDEV_CARRIER_OFF(pApCliEntry->wdev.if_dev);
#endif

#if defined (CONFIG_WIFI_PKT_FWD)
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		if (wf_fwd_entry_delete_hook)
			wf_fwd_entry_delete_hook (pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
		if (wf_fwd_check_device_hook)
			wf_fwd_check_device_hook (pApCliEntry->wdev.if_dev, INT_APCLI, CliIdx, pApCliEntry->wdev.channel, 0);
#endif
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

#ifdef WH_EVENT_NOTIFIER
    {
        MAC_TABLE_ENTRY *pMacEntry = NULL;
        EventHdlr pEventHdlrHook = NULL;

#ifdef MAC_REPEATER_SUPPORT
        if (CliIdx != 0xFF)
        {        	
            pReptEntry = &pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx];
            if(VALID_UCAST_ENTRY_WCID(pReptEntry->MacTabWCID))
                pMacEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
        }
        else
#endif /* MAC_REPEATER_SUPPORT */
        {
            if(VALID_UCAST_ENTRY_WCID(pApCliEntry->MacTabWCID))
                pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
        }

        pEventHdlrHook = GetEventNotiferHook(WHC_DRVEVNT_EXT_UPLINK_STAT);
        if(pEventHdlrHook && pMacEntry->wdev)
            pEventHdlrHook(pAd, pMacEntry, (UINT32)WHC_UPLINK_STAT_DISCONNECT);
    }
#endif /* WH_EVENT_NOTIFIER */

#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx != 0xFF)
		MacTabWCID = pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].MacTabWCID;
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
#ifdef WH_EZ_SETUP
		if(IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev))
			ez_apcli_link_down(pAd, pApCliEntry, ifIndex);
#endif /* WH_EZ_SETUP */	
		MacTabWCID = pApCliEntry->MacTabWCID;
	}
	MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID));
#ifdef MT_MAC
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif		
#ifdef MULTI_APCLI_SUPPORT
    RTMP_STA_ENTRY_MAC_RESET(pAd, APCLI_MCAST_WCID(ifIndex));//MT_MAC clear mcast entry of rootAP.
#else /* MULTI_APCLI_SUPPORT */
    RTMP_STA_ENTRY_MAC_RESET(pAd, APCLI_MCAST_WCID);//MT_MAC clear mcast entry of rootAP.
#endif /* !MULTI_APCLI_SUPPORT */
#endif

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
#ifdef WH_EZ_SETUP
		if(IS_ADPTR_EZ_SETUP_ENABLED(pAd))
			BssTableDeleteEntry(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid, pApCliEntry->wdev.channel);
#endif
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
		BssTableDeleteEntry(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid, pAd->CommonCfg.Channel);
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
	}
	pApCliEntry->OpenWEPErrPktChk = FALSE;
	pApCliEntry->OpenWEPErrPktCnt = 0;
	pApCliEntry->OpenWEPErrMCPktCnt = 0;
#ifdef MWDS
	MWDSAPCliPeerDisable(pAd, pApCliEntry, &pAd->MacTab.Content[MacTabWCID]);
#endif /* MWDS */

#if defined(MAP_SUPPORT) && defined(A4_CONN)
	map_a4_peer_disable(pAd, &pAd->MacTab.Content[MacTabWCID], FALSE);
#endif



	/*for APCLI linkdown*/
#if defined(MAP_SUPPORT) && defined(WAPP_SUPPORT)
	if (CliIdx == 0xFF)
		wapp_send_apcli_association_change(WAPP_APCLI_DISASSOCIATED, pAd, pApCliEntry);
#endif

#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN
	if (CliIdx == 0xFF)
		ApCliLinkDownComplete(pApCliEntry);
#endif
#endif
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

		if (APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE))
		{
			DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startup.\n", __FUNCTION__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
		}
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
			wf_fwd_entry_delete_hook(pApCliEntry->wdev.if_dev, pAd->net_dev, 1);
#endif /* CONFIG_WIFI_PKT_FWD */  
		DBGPRINT(RT_DEBUG_TRACE, ("%s():ApCli interface[%d] start down.\n", __FUNCTION__, ifIndex));

		if (pApCliEntry->Enable == TRUE)
			continue;

#ifdef MAC_REPEATER_SUPPORT
		if (pAd->ApCfg.bMACRepeaterEn)
		{
			for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
			{
				if (pAd->ApCfg.ApCliTab[ifIndex].RepeaterCli[CliIdx].CliEnable)
				{
					MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					//RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
		RTMP_MLME_HANDLER(pAd);
#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN
		ApCliWaitLinkDown(pApCliEntry);
#endif
#endif
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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	BOOLEAN bWpa_4way_too_log = FALSE;
	UCHAR TimeoutVal = 30;

#endif

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
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		STA_TR_ENTRY *tr_entry;
		BOOLEAN bForceBrocken = FALSE;
#ifdef WH_EZ_SETUP
		BOOLEAN bBeacon_miss = FALSE;
#endif
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
						tr_entry = &pAd->MacTab.tr_entry[Wcid];
						if ((tr_entry->PortSecured != WPA_802_1X_PORT_SECURED) &&
							RTMP_TIME_AFTER(pAd->Mlme.Now32
								, (pReptCliEntry->CliTriggerTime + (
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
								TimeoutVal
#else
								5
#endif
								* OS_HZ)))) {
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
							APCLI_STRUCT *papcli_entry = pApCliEntry;

							if (papcli_entry->pre_mac_entry.AuthMode == Ndis802_11AuthModeWPA3PSK
								|| papcli_entry->pre_mac_entry.AuthMode == Ndis802_11AuthModeOWE) {
								UCHAR pmkid[80];
								UCHAR pmk[LEN_PMK];
								INT cached_idx;
								UCHAR if_index = papcli_entry->wdev.func_idx;

								/*Update PMK cache and delete sae instance*/
								if (FALSE ||
#ifdef APCLI_SAE_SUPPORT
									(papcli_entry->pre_mac_entry.AuthMode == Ndis802_11AuthModeWPA3PSK &&
									sae_get_pmk_cache(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, papcli_entry->MlmeAux.Bssid, pmkid, pmk))
#endif
#ifdef APCLI_OWE_SUPPORT
									|| papcli_entry->pre_mac_entry.AuthMode == Ndis802_11AuthModeOWE
#endif
									) {

									cached_idx = apcli_search_pmkid_cache(pAd, papcli_entry->MlmeAux.Bssid, if_index
#ifdef MAC_REPEATER_SUPPORT
										, CliIdx
#endif
										);

									if (cached_idx != INVALID_PMKID_IDX) {
#ifdef APCLI_SAE_SUPPORT
										SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg, pReptCliEntry->CurrentAddress, papcli_entry->MlmeAux.Bssid);

										MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
													("%s:Connection falied with pmkid ,delete cache entry and sae instance\n", __func__));
										if (pSaeIns != NULL) {
											delete_sae_instance(pSaeIns);
											pSaeIns = NULL;
										}
#endif
										apcli_delete_pmkid_cache(pAd, papcli_entry->MlmeAux.Bssid, if_index
#ifdef MAC_REPEATER_SUPPORT
											, CliIdx
#endif
											);
									}
								}
							}
#endif

							MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
							RTMP_MLME_HANDLER(pAd);
						}
					}
					else
					{
						if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pReptCliEntry->CliTriggerTime + (5 * OS_HZ))))
						{
							DBGPRINT(RT_DEBUG_OFF,
								("%s, ApCli%d, CliIdx = %d\n",
								__func__, index, CliIdx));
							MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
								APCLI_CTRL_DEL_MACREPENTRY,
								0,
								NULL,
								(64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
							RTMP_MLME_HANDLER(pAd);
						}
					}
				}
			}
		}
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->Valid == TRUE)
		{
			BOOLEAN ApclibQosNull = FALSE;

			Wcid = pAd->ApCfg.ApCliTab[index].MacTabWCID;
			if (!VALID_WCID(Wcid))
				continue;
			pMacEntry = &pAd->MacTab.Content[Wcid];
 			tr_entry = &pAd->MacTab.tr_entry[Wcid];

			if ((pMacEntry->AuthMode >= Ndis802_11AuthModeWPA)
				&& (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApCliLinkUpTime + (30 * OS_HZ))))) {
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
				bWpa_4way_too_log = TRUE;
#endif
				bForceBrocken = TRUE;
			}

			if (RTMP_TIME_AFTER(pAd->Mlme.Now32 , (pApCliEntry->ApCliRcvBeaconTime + (4 * OS_HZ))))
			{
#ifdef WH_EZ_SETUP
#ifdef EZ_NETWORK_MERGE_SUPPORT
				if(IS_EZ_SETUP_ENABLED(&pApCliEntry->wdev)){
					unsigned char delay_disconnect_count = ez_get_delay_disconnect_count(&pApCliEntry->wdev);
					if (delay_disconnect_count > 0) {
						delay_disconnect_count--;
						ez_set_delay_disconnect_count(&pApCliEntry->wdev, delay_disconnect_count);
					}
					else
					{
						bForceBrocken = TRUE;
#ifdef WH_EZ_SETUP
						bBeacon_miss = TRUE;
#endif
					}
				}
				else
#endif
#endif
				{
					bForceBrocken = TRUE;
				}
				
			}
			
		if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				ApclibQosNull = TRUE;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
		if (bWpa_4way_too_log == TRUE) {
			if (pMacEntry->AuthMode == Ndis802_11AuthModeWPA3PSK ||
				pMacEntry->AuthMode == Ndis802_11AuthModeOWE) {
				UCHAR pmkid[80];
				UCHAR pmk[LEN_PMK];
				INT cached_idx;
				UCHAR if_index = pApCliEntry->wdev.func_idx;
				UCHAR cli_idx = 0xFF;

				/* Connection taking too long update PMK cache	and delete sae instance*/
				if (
#ifdef APCLI_SAE_SUPPORT

					(pMacEntry->AuthMode >= Ndis802_11AuthModeWPA3PSK &&
						sae_get_pmk_cache(&pAd->SaeCfg, pApCliEntry->wdev.if_addr,
						pApCliEntry->MlmeAux.Bssid, pmkid, pmk))
#endif

#ifdef APCLI_OWE_SUPPORT
					|| pMacEntry->AuthMode == Ndis802_11AuthModeOWE
#endif
				) {

					cached_idx = apcli_search_pmkid_cache(pAd,
						pApCliEntry->MlmeAux.Bssid, if_index
#ifdef MAC_REPEATER_SUPPORT
						, cli_idx
#endif
						);
					if (cached_idx != INVALID_PMKID_IDX) {
#ifdef APCLI_SAE_SUPPORT
						SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg,
							pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid);

						MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
							("Reconnection falied with pmkid ,delete cache entry and sae instance\n"));

						if (pSaeIns != NULL)
							delete_sae_instance(pSaeIns);
#endif
						apcli_delete_pmkid_cache(pAd,
							pApCliEntry->MlmeAux.Bssid, if_index
#ifdef MAC_REPEATER_SUPPORT
							, cli_idx
#endif
							);
					}
				}

			}
		}
#endif

			if (bForceBrocken == FALSE)
				ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PWR_ACTIVE);
		}
		else
			continue;

		if (bForceBrocken == TRUE)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfMonitor: IF(apcli%d) - no Beancon is received from root-AP.\n", index));
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));

#ifdef MAC_REPEATER_SUPPORT
			if (pAd->ApCfg.bMACRepeaterEn)
			{
				for(CliIdx = 0; CliIdx < MAX_EXT_MAC_ADDR_SIZE; CliIdx++)
				{
					if (pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].CliEnable)
					{
#ifdef WH_EZ_SETUP
						if (bBeacon_miss)
						{
							pAd->ApCfg.ApCliTab[index].RepeaterCli[CliIdx].Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
						}
#endif
						MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*index + CliIdx));
						RTMP_MLME_HANDLER(pAd);
						//RTMPRemoveRepeaterEntry(pAd, index, CliIdx);
					}
				}
			}
#endif /* MAC_REPEATER_SUPPORT */
			//MCC TODO: WCID Not Correct when MCC on
#ifdef WH_EZ_SETUP
			if (bBeacon_miss)
			{
				pApCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
			}
#endif
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
#ifdef APCLI_SAE_SUPPORT
	USHORT Alg;
#endif
	UCHAR EAPType;
	BOOLEAN Return = FALSE;
#ifdef WSC_AP_SUPPORT
	UCHAR EAPCode;
    PMAC_TABLE_ENTRY pEntry;
#endif /* WSC_AP_SUPPORT */
	unsigned char hdr_len = LENGTH_802_11;

#ifdef A4_CONN
	if ((pFrame->Hdr.FC.FrDs == 1) && (pFrame->Hdr.FC.ToDs == 1))
		hdr_len = LENGTH_802_11_WITH_ADDR4;
#endif

	/* only PROBE_REQ can be broadcast, all others must be unicast-to-me && is_mybssid; otherwise, */
	/* ignore this frame */

	/* WPA EAPOL PACKET */
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA)
	{
#ifdef WSC_AP_SUPPORT
	        /*WSC EAPOL PACKET */
	        pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);
	        if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].WscControl.WscConfMode == WSC_ENROLLEE)
	        {
	            *Machine = WSC_STATE_MACHINE;
				EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
				EAPCode = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 4);
	            Return = WscMsgTypeSubst(EAPType, EAPCode, MsgType);
	        }
	        if (!Return)
#endif /* WSC_AP_SUPPORT */
	        {
	    		*Machine = WPA_STATE_MACHINE;
				EAPType = *((UCHAR *)pFrame + hdr_len + LENGTH_802_1_H + 1);
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
#ifdef APCLI_SAE_SUPPORT
				NdisMoveMemory(&Alg, &pFrame->Octet[0], sizeof(USHORT));
#endif /* APCLI_SAE_SUPPORT */

				NdisMoveMemory(&Seq, &pFrame->Octet[2], sizeof(USHORT));
#ifdef APCLI_SAE_SUPPORT
				if (Alg == AUTH_MODE_SAE && Seq == 1) {
					*Machine = APCLI_AUTH_STATE_MACHINE;
					*MsgType = APCLI_MT2_MLME_SAE_AUTH_COMMIT;
				} else if (Alg == AUTH_MODE_SAE && Seq == 2) {
					*Machine = APCLI_AUTH_STATE_MACHINE;
					*MsgType = APCLI_MT2_MLME_SAE_AUTH_CONFIRM;
				} else
#endif /* APCLI_SAE_SUPPORT */

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
#ifdef MULTI_APCLI_SUPPORT
				DBGPRINT(RT_DEBUG_TRACE,("\x1b[36m preCheckMsgTypeSubset SUBTYPE_PROBE_RSP....\x1b[m\n"));
#endif /* MULTI_APCLI_SUPPORT */

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
	CHAR          IeType/*, *Ptr*/;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT   pEid;
	ULONG         Length = 0;
#ifdef APCLI_OWE_SUPPORT
	UCHAR *extension_id;
	UCHAR *ext_ie_length;
#endif

	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	//Ptr = (CHAR *) pFrame->Octet;
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
#ifdef APCLI_OWE_SUPPORT
			case IE_RSN:
				/* Copy whole RSNIE context */
				NdisMoveMemory(&ie_list->RSN_IE[0], pEid, pEid->Len + 2);
				ie_list->RSNIE_Len = pEid->Len + 2;
				break;

			case IE_WLAN_EXTENSION:
				/*parse EXTENSION EID*/
				extension_id = (UCHAR *)pEid + 2;
				switch (*extension_id) {
				case IE_EXTENSION_ID_ECDH:
					ext_ie_length = (UCHAR *)pEid + 1;
					if (*ext_ie_length - 3 <= sizeof(ie_list->ecdh_ie.public_key))
						os_zero_mem(ie_list->ecdh_ie.public_key, *ext_ie_length - 3);
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s():IE_ECDH error\n", __func__));
						break;
					}
					ie_list->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
					ie_list->ecdh_ie.length = pEid->Len;
					if (pEid->Len <= sizeof(ie_list->ecdh_ie) - 2)
						NdisMoveMemory(&ie_list->ecdh_ie.ext_id_ecdh, pEid->Octet, pEid->Len);
					else {
						DBGPRINT(RT_DEBUG_ERROR, ("%s():IE_ECDH error\n", __func__));
						break;
					}
					break;

				default:
				DBGPRINT(RT_DEBUG_ERROR,
					("RESP IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id));
				break;
				}
#endif
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
			ApCliIndex = pCurEntry->func_tb_idx;
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
	STA_TR_ENTRY *rpt_peer_entry = NULL, *apcli_peer_entry = NULL;


	for(idx = 0; idx < MAX_APCLI_NUM; idx++)
	{
		apcli_entry = &pAd->ApCfg.ApCliTab[idx];
		if (&apcli_entry->wdev == wdev)
		{
			if (ValidApCliEntry(pAd, idx) == FALSE)
				return FALSE;

#ifdef WSC_AP_SUPPORT
			if ((apcli_entry->WscControl.WscConfMode != WSC_DISABLE) &&
					(apcli_entry->WscControl.bWscTrigger == TRUE)) {
				DBGPRINT(RT_DEBUG_ERROR, ("apcli during wps, os should not send packets!!!\n"));
				return FALSE;
			}
#endif /* WSC_AP_SUPPORT */

			apcli_peer_entry = &pAd->MacTab.tr_entry[apcli_entry->MacTabWCID];
			if (apcli_peer_entry &&
				(apcli_peer_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
				DBGPRINT(RT_DEBUG_ERROR, ("apcli peer entry port not secure, drop os packets!!!\n"));
				return FALSE;
			}

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef A4_CONN
			&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */

			)
			{
				PUCHAR pSrcBufVA = NULL;
				PACKET_INFO PacketInfo;
				UINT SrcBufLen;
				REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
				UCHAR isLinkValid;

				RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pSrcBufVA, &SrcBufLen);

				pReptEntry = RTMPLookupRepeaterCliEntry(pAd, TRUE, (pSrcBufVA + MAC_ADDR_LEN), TRUE, &isLinkValid);
				if (pReptEntry)
				{
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
				if ((pReptEntry->MatchApCliIdx != apcli_entry->wdev.func_idx) &&					
					(wf_fwd_check_active_hook && wf_fwd_check_active_hook()))
				{
					UCHAR apCliIdx, CliIdx;
					apCliIdx = pReptEntry->MatchApCliIdx;
					CliIdx = pReptEntry->MatchLinkIdx;
					MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
								APCLI_CTRL_DISCONNECT_REQ,
								0,
								NULL,
								(64 + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					return FALSE;
				}
#endif /* CONFIG_WIFI_PKT_FWD */		
#endif
					rpt_peer_entry = &pAd->MacTab.tr_entry[pReptEntry->MacTabWCID];
					if (rpt_peer_entry &&
						(rpt_peer_entry->PortSecured != WPA_802_1X_PORT_SECURED)) {
						allowed = FALSE;
						DBGPRINT(RT_DEBUG_ERROR,
							("rpt peer entry not secure, drop os packets!!!\n"));
					}
					*pWcid = pReptEntry->MacTabWCID;
					return TRUE;
				}
				else
				{
					if (!isLinkValid)
						return FALSE;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
				if ((wf_fwd_check_active_hook && wf_fwd_check_active_hook())) 
#endif
                {
#endif
					if (RTMPRepeaterVaildMacEntry(pAd, pSrcBufVA + MAC_ADDR_LEN))
					{
						RTMPInsertRepeaterEntry(pAd, idx, (pSrcBufVA + MAC_ADDR_LEN));
						DBGPRINT(RT_DEBUG_ERROR, (" Receive trigger packet !!!\n"));
						return FALSE;
#if (MT7615_MT7603_COMBO_FORWARDING == 1)
					}
#endif
					}
				}

				*pWcid = apcli_entry->MacTabWCID;
			}
			else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount ++;
				RTMP_SET_PACKET_WDEV(pPacket, wdev->wdev_idx);
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
	IN USHORT idx
#ifdef APCLI_OWE_SUPPORT
	, IN UCHAR Privacy
#endif
	)
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
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
	UCHAR end_field = 0;
	UCHAR res = TRUE;
#ifdef APCLI_OWE_SUPPORT
	RSN_CAPABILITIES *pRSN_Cap = NULL;
#endif
#endif

#if defined(APCLI_OWE_SUPPORT) || defined(APCLI_SAE_SUPPORT)
	pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
#endif

#ifdef APCLI_OWE_SUPPORT
	if (pAd->ApCfg.ApCliTab[idx].wdev.AuthMode == Ndis802_11AuthModeOWE && (Privacy == 0)) {
		pApCliEntry->pre_mac_entry.AuthMode = Ndis802_11AuthModeOpen;
		pApCliEntry->pre_mac_entry.WepStatus = Ndis802_11WEPDisabled;
		pApCliEntry->pre_mac_entry.GroupKeyWepStatus = Ndis802_11WEPDisabled;
		pApCliEntry->PairCipher = Ndis802_11WEPDisabled;
		pApCliEntry->GroupCipher = Ndis802_11WEPDisabled;
		pApCliEntry->RsnCapability = 0;
		pApCliEntry->bMixCipher = FALSE;
		return TRUE; /* No Security */
	}
#endif


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
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				res = wpa_rsne_sanity(pTmp, le2cpu16(pRsnHeader->Length) + 2, &end_field);
				if (res == FALSE)
					return FALSE;

				if (end_field < RSN_FIELD_GROUP_CIPHER)
					WPA2.GroupCipher = Ndis802_11AESEnable;
				if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
					WPA2.PairCipher = Ndis802_11AESEnable;
				if (end_field < RSN_FIELD_AKM)
					WPA2_AuthMode = Ndis802_11AuthModeWPA2PSK;
#endif
				/* 0. Version must be 1 */
				/*  The pRsnHeader->Version exists in native little-endian order, so we may need swap it for RT_BIG_ENDIAN systems. */
				if (le2cpu16(pRsnHeader->Version) != 1)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("ApCliValidateRSNIE - RSN Version isn't 1(%d) \n", pRsnHeader->Version));
					break;
				}
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				/*Check group cipher*/
				if (end_field < RSN_FIELD_GROUP_CIPHER)
					break;
#endif

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
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
					break;
#endif
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				pTmp   += sizeof(USHORT);

				/* 3. Get pairwise cipher */
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (Count == 0) {
					DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]%s : pairwise cipher count =0 !!!\n", __func__));
					return FALSE;
				}
#endif
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
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (end_field < RSN_FIELD_AKM)
					break;
#endif
				NdisMoveMemory(&Count, pTmp, sizeof(USHORT));
				Count = cpu2le16(Count);

				pTmp   += sizeof(USHORT);

				/* Parse AKM ciphers */
#ifdef APCLI_SECURITY_IMPROVEMENT_SUPPORT
				if (Count == 0) {
					DBGPRINT(RT_DEBUG_ERROR, ("[ERROR]%s : AKM cipher count =0 !!!\n", __func__));
					return FALSE;
				}
#endif
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
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
						case 6:
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
							/* WPA2-personal */
							TmpAuthMode = Ndis802_11AuthModeWPA2PSK;
							break;
#ifdef APCLI_SAE_SUPPORT
						case 8:
							TmpAuthMode = Ndis802_11AuthModeWPA3PSK;
							break;
#endif
#ifdef APCLI_OWE_SUPPORT
						case 18:
							TmpAuthMode = Ndis802_11AuthModeOWE;
							break;
#endif
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

#ifdef APCLI_OWE_SUPPORT
				if (end_field >= RSN_FIELD_RSN_CAP) {

					pRSN_Cap = (RSN_CAPABILITIES *) pTmp;

					if (pApCliEntry->pre_mac_entry.AuthMode == Ndis802_11AuthModeOWE) {
					/*OWE connection should be allowed with only OWE AP*/
					/*which has PMF Capable and PMF required set*/
						if ((pRSN_Cap->field.MFPC == 0) || (pRSN_Cap->field.MFPR == 0))
							return FALSE;
					}
				}
#endif

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
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	else if (wdev->AuthMode == Ndis802_11AuthModeWPA3PSK || wdev->AuthMode == Ndis802_11AuthModeOWE) {

#ifdef APCLI_SAE_SUPPORT
		if (WPA2_AuthMode == Ndis802_11AuthModeWPA2PSK &&
			wdev->AuthMode == Ndis802_11AuthModeWPA3PSK) {
			pApCliEntry->pre_mac_entry.AuthMode = Ndis802_11AuthModeWPA2PSK;
			Sanity |= 0x2;
		}
#endif
#ifdef APCLI_OWE_SUPPORT
		/*Disallow TKIP cipher with OWE*/
		if (WPA2_AuthMode == Ndis802_11AuthModeOWE &&
			pApCliEntry->PairCipher == Ndis802_11TKIPEnable)
			Sanity = 0;
#endif
#ifdef APCLI_SAE_SUPPORT
		else if ((WPA2_AuthMode == Ndis802_11AuthModeWPA3PSK &&
			WPA2_AuthModeAux == Ndis802_11AuthModeWPA2PSK) &&
			(wdev->AuthMode == Ndis802_11AuthModeWPA3PSK)) {
			pApCliEntry->pre_mac_entry.AuthMode = Ndis802_11AuthModeWPA3PSK;
			Sanity |= 0x2;
		}
#endif

	}
#endif
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
	} else if ((wdev->AuthMode == Ndis802_11AuthModeWPA2)
			|| (wdev->AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
			|| (wdev->AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
			|| (wdev->AuthMode == Ndis802_11AuthModeOWE)
#endif
			) {
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
	IN UCHAR wdev_idx)
{
	RXINFO_STRUC *pRxInfo = pRxBlk->pRxInfo;
	PHEADER_802_11 pHeader = pRxBlk->pHeader;
	APCLI_STRUCT *pApCliEntry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */

	/*
		It is possible to receive the multicast packet when in AP Client mode
		ex: broadcast from remote AP to AP-client,
				addr1=ffffff, addr2=remote AP's bssid, addr3=sta4_mac_addr
	*/
	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];

	/* Filter out Bcast frame which AP relayed for us */
	/* Multicast packet send from AP1 , received by AP2 and send back to AP1, drop this frame */
	if (MAC_ADDR_EQUAL(pHeader->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT
	if ((pAd->ApCfg.bMACRepeaterEn)
#ifdef A4_CONN
	&& (IS_APCLI_A4(pApCliEntry) == FALSE)
#endif /* A4_CONN */
	    )
	{
		UCHAR isLinkValid;
		
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pHeader->Addr3, TRUE, &isLinkValid);
		if (pReptEntry)
			return FALSE;	/* give up this frame */
	}
#endif /* MAC_REPEATER_SUPPORT */



#ifdef A4_CONN
	if (IS_ENTRY_A4(pEntry)) {
		pRxBlk->pData += LENGTH_802_11_WITH_ADDR4;
		pRxBlk->DataSize -= LENGTH_802_11_WITH_ADDR4;
	}
	else
#endif

	/* skip the 802.11 header */
	{
		pRxBlk->pData += LENGTH_802_11;
		pRxBlk->DataSize -= LENGTH_802_11;
	}
	/* Use software to decrypt the encrypted frame. */
	/* Because this received frame isn't my BSS frame, Asic passed to driver without decrypting it. */
	/* If receiving an "encrypted" unicast packet(its WEP bit as 1) and doesn't match my BSSID, it */
	/* pass to driver with "Decrypted" marked as 0 in RxD. */
	if ((pRxInfo->MyBss == 0) && (pRxInfo->Decrypted == 0) && (pHeader->FC.Wep == 1))
	{
		if (RTMPSoftDecryptionAction(pAd,
									 (PUCHAR)pHeader, 0,
									 &pApCliEntry->SharedKey[pRxBlk->key_idx],
									 pRxBlk->pData,
									 &(pRxBlk->DataSize)) == NDIS_STATUS_FAILURE)
		{
			return FALSE;  /* give up this frame */
		}
	}
	pRxInfo->MyBss = 1;

	Indicate_Legacy_Packet(pAd, pRxBlk, wdev_idx);

	return TRUE;
}


VOID APCliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry)
{
	UCHAR	IfIdx;
	UINT8	BssIdx;

	IfIdx = pEntry->func_tb_idx;
	BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + IfIdx;
#ifdef MAC_APCLI_SUPPORT
	BssIdx = APCLI_BSSID_IDX + IfIdx;
#endif /* MAC_APCLI_SUPPORT */

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

	IfIdx = pEntry->func_tb_idx;
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

	PAPCLI_STRUCT pApCliEntry = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

	switch (pAd->CommonCfg.PhyMode)
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


extern INT sta_rx_fwd_hnd(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
extern INT sta_rx_pkt_allow(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID APCli_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
#define APCLI_MAX_DEV_NUM	32
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
#ifdef APCLI_SAE_SUPPORT
		pApCliEntry->sae_cfg_group = SAE_DEFAULT_GROUP;
#endif
#ifdef APCLI_OWE_SUPPORT
		pApCliEntry->curr_owe_group = ECDH_GROUP_256;
#endif
		pApCliEntry->OpenWEPErrPktChk = FALSE;
		pApCliEntry->OpenWEPErrPktCnt = 0;
		pApCliEntry->OpenWEPErrMCPktCnt = 0;
		wdev = &pApCliEntry->wdev;
		wdev->wdev_type = WDEV_TYPE_STA;
		wdev->func_dev = pApCliEntry;
		wdev->func_idx = idx;
		wdev->sys_handle = (void *)pAd;
		wdev->if_dev = new_dev_p;
		wdev->tx_pkt_allowed = ApCliAllowToSendPacket;
		// TODO: shiang-usw, modify this to STASendPacket!
		wdev->tx_pkt_handle = APSendPacket;
		wdev->wdev_hard_tx = APHardTransmit;
		wdev->rx_pkt_allowed = sta_rx_pkt_allow;
		wdev->rx_pkt_foward = sta_rx_fwd_hnd;
#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN
		ApCliCompleteInit(pApCliEntry);
#endif
#endif

		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
		if (rtmp_wdev_idx_reg(pAd, wdev) < 0) {
			DBGPRINT(RT_DEBUG_ERROR, ("Assign wdev idx for %s failed, free net device!\n",
						RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		/* init MAC address of virtual network interface */
		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);

#ifdef MT_MAC
		if (pAd->chipCap.hif_type != HIF_MT)
		{
#endif /* MT_MAC */
			if (pAd->chipCap.MBSSIDMode >= MBSSID_MODE1)
			{
				if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0))
				{
#ifdef ENHANCE_NEW_MBSSID_MODE
					UCHAR MacMask = 0;

					if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
						MacMask = 0xFE;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
						MacMask = 0xFC;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
						MacMask = 0xF8;
#endif  /* ENHANCE_NEW_MBSSID_MODE */
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
#ifdef ENHANCE_NEW_MBSSID_MODE
						wdev->if_addr[0] &= (MacMask << 2);
#endif /* ENHANCE_NEW_MBSSID_MODE */
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[0] += (((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1) << 2);
					}
#ifdef ENHANCE_NEW_MBSSID_MODE
					else
					{
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] &= (MacMask);
						wdev->if_addr[pAd->chipCap.MBSSIDMode - 1] += ((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
					}
#endif /* ENHANCE_NEW_MBSSID_MODE */
				}
			}
			else
			{
				wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;
			}
#ifdef MT_MAC
		}
		else {
			UINT32 Value;

			//TODO: shall we make choosing which byte to be selectable???
			Value = 0x00000000L;
			RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);//read BTEIR bit[31:29] for determine to choose which byte to extend BSSID mac address.
			Value = Value | ((pAd->chipCap.MBSSIDMode -2) << 29);


			//Carter, I make apcli interface use HWBSSID1 to go.
			//so fill own_mac and BSSID here.
			wdev->if_addr[0] |= 0x2;

			switch ((pAd->chipCap.MBSSIDMode -2)) {
				case 0x1: /* choose bit[23:20]*/
					//mapping to MBSSID_MODE3
					wdev->if_addr[2] = (wdev->if_addr[2] & 0x0f);
					break;
				case 0x2: /* choose bit[31:28]*/
					//mapping to MBSSID_MODE4					
					wdev->if_addr[3] = (wdev->if_addr[3] & 0x0f);
					break;
				case 0x3: /* choose bit[39:36]*/
					//mapping to MBSSID_MODE5					
					wdev->if_addr[4] = (wdev->if_addr[4] & 0x0f);
					break;
				case 0x4: /* choose bit [47:44]*/
					//mapping to MBSSID_MODE6					
					wdev->if_addr[5] = (wdev->if_addr[5] & 0x0f);
					break;
				default: /* choose bit[15:12]*/
					//mapping to MBSSID_MODE2					
					wdev->if_addr[1] = (wdev->if_addr[1] & 0x0f);
					break;
			}
			
#ifdef MULTI_APCLI_SUPPORT //tmp use byte5 to distinguish, may refine this by setting
			wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + idx);
#endif /* MULTI_APCLI_SUPPORT */
//			AsicSetDevMac(pAd, wdev->if_addr, 0x1);//set own_mac to HWBSSID1
			//AsicSetBssid(pAd, wdev->if_addr, 0x1);
		}
#endif /* MT_MAC */

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

    APCLI_STRUCT *pApCliEntry;
    struct wifi_dev *wdev = NULL;

#if defined (CONFIG_WIFI_PKT_FWD)
	if (wf_fwd_probe_adapter)
		wf_fwd_probe_adapter(pAd);
#endif 	

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++)
	{
		if (pAd->ApCfg.ApCliTab[ifIndex].wdev.if_dev == dev_p)
		{
			RTMP_OS_NETDEV_START_QUEUE(dev_p);

			pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
			wdev = &pApCliEntry->wdev;

#ifdef IF_UP_DOWN
			ApCliWaitIfDown(pApCliEntry);
#endif

#ifdef MULTI_APCLI_SUPPORT
			AsicSetDevMac(pAd, wdev->if_addr, (1+ifIndex));//Apcli OwnMac start from HWBSSID 1.	
#else /* MULTI_APCLI_SUPPORT */
			AsicSetDevMac(pAd, wdev->if_addr, 1);//Apcli OwnMac start from HWBSSID 1.
#endif /* !MULTI_APCLI_SUPPORT */

#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
			pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPC = FALSE;
			pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPR = FALSE;
			pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.PMFSHA256 = FALSE;
			if (((pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPA2) ||
				(pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
				|| (pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeWPA3PSK)
				|| (pAd->ApCfg.ApCliTab[ifIndex].wdev.AuthMode == Ndis802_11AuthModeOWE)
#endif
				) && (pAd->ApCfg.ApCliTab[ifIndex].wdev.WepStatus == Ndis802_11AESEnable)) {
				pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.PMFSHA256 =
					pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.Desired_PMFSHA256;

				if (pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.Desired_MFPC) {
					pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPC = TRUE;
					pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPR =
						pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.Desired_MFPR;

					if (pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPR)
						pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.PMFSHA256 = TRUE;
				}
			} else if (pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.Desired_MFPC) {
				DBGPRINT(RT_DEBUG_ERROR,
					("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __func__));
			}
			DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
				__func__,
				pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPC,
				pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.MFPR,
				pAd->ApCfg.ApCliTab[ifIndex].PmfCfg.PMFSHA256));
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */
#ifdef MWDS
			if (wdev->bDefaultMwdsStatus == TRUE)
				MWDSEnable(pAd, ifIndex, FALSE, TRUE);
#endif

#if defined(MAP_SUPPORT) && defined(A4_CONN)
			if (IS_MAP_ENABLE(pAd))
				map_a4_init(pAd, wdev->func_idx, FALSE);
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			NdisAllocateSpinLock(pAd, &pApCliEntry->SavedPMK_lock);
#endif

#ifdef WH_EZ_SETUP
		if (IS_CONF_EZ_SETUP_ENABLED(wdev))
			ez_start(wdev, FALSE);
#endif /* WH_EZ_SETUP */

			ApCliIfUp(pAd);

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
#ifdef MWDS
			MWDSDisable(pAd, ifIndex, FALSE, TRUE);
#endif /* MWDS */

#if defined(MAP_SUPPORT) && defined(A4_CONN)
		if (IS_MAP_ENABLE(pAd))
			map_a4_deinit(pAd, wdev->func_idx, FALSE);
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			NdisFreeSpinLock(&apcli_entry->SavedPMK_lock);
#endif
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
			if (wf_fwd_delete_repeater_mapping) {
				DBGPRINT(RT_DEBUG_OFF, ("%s() 7603 wf_fwd_delete_repeater_mapping.\n", __func__));
				wf_fwd_delete_repeater_mapping(pAd);
			}

			if (wf_fwd_remove_adapter) {
				DBGPRINT(RT_DEBUG_OFF, ("%s() 7603 wf_fwd_remove_adapter.\n", __func__));
				wf_fwd_remove_adapter(pAd);
			}
#endif

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
							MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, (64 + MAX_EXT_MAC_ADDR_SIZE*ifIndex + CliIdx));
							RTMP_MLME_HANDLER(pAd);
							//RTMPRemoveRepeaterEntry(pAd, ifIndex, CliIdx);
						}
					}
				}
#endif /* MAC_REPEATER_SUPPORT */
#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN
				RTMP_OS_INIT_COMPLETION(&apcli_entry->linkdown_complete);
#endif
#endif

				MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
				RTMP_MLME_HANDLER(pAd);
				DBGPRINT(RT_DEBUG_TRACE, ("(%s) ApCli interface[%d] startdown.\n", __FUNCTION__, ifIndex));
#ifdef WH_EZ_SETUP
#ifdef IF_UP_DOWN
				ApCliWaitLinkDown(apcli_entry);
				ApCliWaitStateDisconnect(apcli_entry);
#endif
#endif

				//clean CfgApCliBssid
				NdisZeroMemory(&(apcli_entry->CfgApCliBssid), MAC_ADDR_LEN);
			}
#ifdef WH_EZ_SETUP
			if (IS_CONF_EZ_SETUP_ENABLED(wdev))
				ez_stop(wdev);
#ifdef IF_UP_DOWN
			ApCliIfDownComplete(apcli_entry);
#endif
#endif /* WH_EZ_SETUP */
			return TRUE;
		}
	}

	return FALSE;
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
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
	RTMP_STRING *pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;

	BSS_ENTRY *pBssEntry = NULL;

#ifdef APCLI_OWE_SUPPORT
	UCHAR switch_to_owe_channel = 0;
	BOOLEAN bupdate_owe_trans = FALSE;
	BSS_TABLE	*powe_bss_tab = NULL;
	APCLI_STRUCT *papcli_entry = NULL;
	UCHAR Channel = 0;
	struct wifi_dev *wdev = NULL;
#endif
	UCHAR tempBuf[20];
	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliAutoConnectExec()\n"));

	ifIdx = pObj->ioctl_if;
	CfgSsidLen = pAd->ApCfg.ApCliTab[ifIdx].CfgSsidLen;
	pCfgSsid = pAd->ApCfg.ApCliTab[ifIdx].CfgSsid;
	pScanTab = &pAd->ScanTab;
	pSsidBssTab = &pAd->ApCfg.ApCliTab[ifIdx].MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;

	if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag != TRUE) {
		DBGPRINT(RT_DEBUG_TRACE, ("AutoConnect is not Enable.\n"));
		return FALSE;
	}
#ifdef APCLI_OWE_SUPPORT
	papcli_entry = &pAd->ApCfg.ApCliTab[ifIdx];
	if (papcli_entry->wdev.AuthMode == Ndis802_11AuthModeOWE) {

		powe_bss_tab = &papcli_entry->MlmeAux.owe_bss_tab;
		powe_bss_tab->BssNr = 0;


		BssTableInit(powe_bss_tab);

		apcli_reset_owe_parameters(pAd, ifIdx);


		/*Find out APs with the OWE transition IE and store in owe_bss_tab**/
		for (entryIdx = 0; entryIdx < pScanTab->BssNr; entryIdx++) {
			BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

			if (pBssEntry->Channel == 0)
				continue;

			if ((pBssEntry->owe_trans_ie_len > 0) &&
				(powe_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
				NdisMoveMemory(&powe_bss_tab->BssEntry[powe_bss_tab->BssNr++],
								pBssEntry, sizeof(BSS_ENTRY));

			}
		}

		if (powe_bss_tab->BssNr < MAX_LEN_OF_BSS_TABLE)
			NdisZeroMemory(&powe_bss_tab->BssEntry[powe_bss_tab->BssNr], sizeof(BSS_ENTRY));
	}

#endif

	/*
		Find out APs with the desired SSID.
	*/
	for (entryIdx=0; entryIdx<pScanTab->BssNr;entryIdx++)
	{
		pBssEntry = &pScanTab->BssEntry[entryIdx];

		if ( pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
							(pBssEntry->SsidLen) &&
							(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE))
		{
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
						("I/F(apcli%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n",
						ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						pBssEntry->Channel, pBssEntry->Rssi));
				DBGPRINT(RT_DEBUG_TRACE,
						("I/F(apcli%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", ifIdx,
						GetAuthMode(pBssEntry->AuthMode),
						GetEncryptType(pBssEntry->WepStatus)) );
				NdisMoveMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++],
								pBssEntry, sizeof(BSS_ENTRY));
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

#ifdef APCLI_OWE_SUPPORT
		pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
		wdev = &papcli_entry->wdev;

		if (wdev->AuthMode == Ndis802_11AuthModeOWE &&
			(pBssEntry->AuthMode == Ndis802_11AuthModeOpen && pBssEntry->WepStatus == Ndis802_11WEPDisabled)) {

			if (pBssEntry->owe_trans_ie_len > 0) {


				UCHAR pair_ch = 0;
				UCHAR pair_bssid[MAC_ADDR_LEN] = {0};
				UCHAR pair_ssid[MAX_LEN_OF_SSID] = {0};
				UCHAR pair_band = 0;
				UCHAR pair_ssid_len = 0;


				extract_pair_owe_bss_info(pBssEntry->owe_trans_ie,
					pBssEntry->owe_trans_ie_len,
					pair_bssid,
					pair_ssid,
					&pair_ssid_len,
					&pair_band,
					&pair_ch);


				if (pair_ch != 0) {
					/*OWE Entry found ,update OweTransBssid and OweTranSsid*/


					/*OWE bss is on different channel*/

					if (BOARD_IS_5G_ONLY(pAd)) {

					} else {


						/*Check if the OWE bss is on the same band as the CLI,then check if channel change required*/
						if ((WMODE_2G_ONLY(papcli_entry->wdev.PhyMode) && (pair_ch <= 14))
							|| (WMODE_5G_ONLY(papcli_entry->wdev.PhyMode) && (pair_ch > 14))) {
							if ((pair_ch != pBssEntry->Channel) || (papcli_entry->wdev.channel != pair_ch)) {

								/*OWE send EVENT to host for OWE  indicating different channel*/
								MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
										("%s:%d Different channel same band\n", __func__, __LINE__));
								switch_to_owe_channel = pair_ch;
								bupdate_owe_trans = TRUE;
								wext_send_owe_trans_chan_event(wdev->if_dev,
									OID_802_11_OWE_EVT_SAME_BAND_DIFF_CHANNEL,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);
							} else	/*Same Channel send directed probe request to OWE BSS*/
								bupdate_owe_trans = TRUE;

						} else {
							/*Channel not in group of current band , but entry exists so send event to host to trigger connection on other band*/

								wext_send_owe_trans_chan_event(wdev->if_dev,
									OID_802_11_OWE_EVT_DIFF_BAND,
									pair_bssid,
									pair_ssid,
									&pair_ssid_len,
									&pair_band,
									&pair_ch);

						}

					}
				} else	{				/*Same Channel send directed probe request to OWE BSS*/
						bupdate_owe_trans = TRUE;
						if (papcli_entry->wdev.channel != pBssEntry->Channel)
							switch_to_owe_channel = pBssEntry->Channel;

				}

				if (bupdate_owe_trans) {
					NdisMoveMemory(&papcli_entry->owe_trans_bssid, pair_bssid, MAC_ADDR_LEN);
					NdisMoveMemory(&papcli_entry->owe_trans_ssid, pair_ssid, pair_ssid_len);
					papcli_entry->owe_trans_ssid_len = pair_ssid_len;

					NdisMoveMemory(&papcli_entry->owe_trans_open_bssid, pBssEntry->Bssid, MAC_ADDR_LEN);
					NdisMoveMemory(&papcli_entry->owe_trans_open_ssid, pBssEntry->Ssid, pBssEntry->SsidLen);
					papcli_entry->owe_trans_open_ssid_len = pBssEntry->SsidLen;

					/*Clear last update settings before trying connection with OWE BSS*/
					papcli_entry->pre_mac_entry.AuthMode = 0;
					papcli_entry->pre_mac_entry.WepStatus = 0;
					papcli_entry->pre_mac_entry.GroupKeyWepStatus = 0;

					/*Delete the Open Bss entry from Scan table*/
					/*because apcli does not ageout scan tab entries*/
					BssTableDeleteEntry(pScanTab, pBssEntry->Bssid, pBssEntry->Channel);
				}
			}
		}
#endif

#ifdef APCLI_OWE_SUPPORT
		if (papcli_entry->wdev.AuthMode == Ndis802_11AuthModeOWE && (pBssEntry->owe_trans_ie_len > 0)) {
			if (switch_to_owe_channel > 0)
				Channel = switch_to_owe_channel;
		}
#endif


		if (pAd->CommonCfg.Channel != pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1].Channel
#ifdef APCLI_OWE_SUPPORT
			|| Channel != 0
#endif
			)
		{
#ifdef APCLI_OWE_SUPPORT
			if (Channel != 0)
				sprintf(tempBuf, "%d", Channel);
			else
#endif
			sprintf(tempBuf, "%d", pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1].Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", tempBuf));
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[ifIdx].wdev))
				rtmp_set_channel(pAd, &pAd->ApCfg.ApCliTab[ifIdx].wdev,
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1].Channel);
			else
#endif
			Set_Channel_Proc(pAd, tempBuf);
		}
			sprintf(tempBuf, "%02X:%02X:%02X:%02X:%02X:%02X",
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[0],
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[1],
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[2],
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[3],
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[4],
					pSsidBssTab->BssEntry[pSsidBssTab->BssNr -1].Bssid[5]);
			Set_ApCli_Bssid_Proc(pAd, tempBuf);
	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr) );
		Set_ApCli_Enable_Proc(pAd, "1");
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}

	Set_ApCli_Enable_Proc(pAd, "1");
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
	IN PRTMP_ADAPTER pAd)
{
	POS_COOKIE  	pObj = (POS_COOKIE) pAd->OS_Cookie;
	BSS_TABLE 		*pSsidBssTab;
	PAPCLI_STRUCT	pApCliEntry;
	UCHAR			lastEntryIdx, ifIdx = pObj->ioctl_if;

#ifdef AP_PARTIAL_SCAN_SUPPORT
	if (pAd->ApCfg.bPartialScanning == TRUE)
		return;
#endif /* AP_PARTIAL_SCAN_SUPPORT */

	DBGPRINT(RT_DEBUG_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIdx];
	pSsidBssTab = &pApCliEntry->MlmeAux.SsidBssTab;

	if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag != TRUE) {
		DBGPRINT(RT_DEBUG_TRACE, ("AutoConnect is not Enable.\n"));
		return;
	}

	/*
		delete (zero) the previous connected-failled entry and always
		connect to the last entry in talbe until the talbe is empty.
	*/
	NdisZeroMemory(&pSsidBssTab->BssEntry[--pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	lastEntryIdx = pSsidBssTab->BssNr -1;

	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE))
	{
		UCHAR	tempBuf[20];

		sprintf(tempBuf, "%02X:%02X:%02X:%02X:%02X:%02X",
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[0],
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[1],
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[2],
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[3],
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[4],
				pSsidBssTab->BssEntry[lastEntryIdx].Bssid[5]);
		Set_ApCli_Bssid_Proc(pAd, tempBuf);
		if (pAd->CommonCfg.Channel != pSsidBssTab->BssEntry[lastEntryIdx].Channel)
		{
			Set_ApCli_Enable_Proc(pAd, "0");
			sprintf(tempBuf, "%d", pSsidBssTab->BssEntry[lastEntryIdx].Channel);
			DBGPRINT(RT_DEBUG_TRACE, ("Switch to channel :%s\n", tempBuf));
#ifdef WH_EZ_SETUP
			if (IS_EZ_SETUP_ENABLED(&pAd->ApCfg.ApCliTab[ifIdx].wdev))
				rtmp_set_channel(pAd, &pAd->ApCfg.ApCliTab[ifIdx].wdev,
					pSsidBssTab->BssEntry[lastEntryIdx].Channel);
			else
#endif				
				Set_Channel_Proc(pAd, tempBuf);
		}
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning = FALSE;
	}

	Set_ApCli_Enable_Proc(pAd, "1");
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
	NDIS_802_11_WEP_STATUS				tempWEPstatus = pApCliEntry->wdev.WepStatus;

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
#ifdef APCLI_SAE_SUPPORT
	else if (tempAuthMode == Ndis802_11AuthModeWPA3PSK) {
		return ((tempAuthMode == AuthMode ||
			tempAuthMode == AuthModeAux ||
			AuthMode == Ndis802_11AuthModeWPA2PSK) &&
			(tempWEPstatus == WPA.GroupCipher ||
			tempWEPstatus == WPA.PairCipher));
	}
#endif
#ifdef APCLI_OWE_SUPPORT
	else if (tempAuthMode == Ndis802_11AuthModeOWE) {
		return ((tempAuthMode == AuthMode ||
			tempAuthMode == AuthModeAux ||
			AuthMode == Ndis802_11AuthModeOpen) &&
			(tempWEPstatus == WPA.GroupCipher ||
			tempWEPstatus == WPA.PairCipher));
	}
#endif

	else {
		/* not supported cases */
		return FALSE;
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
	STA_TR_ENTRY *tr_entry;
	HEADER_802_11 *pHeader = NULL;
	struct rxd_base_struct *rxd_base = NULL;
	int FirstWcid = 1, idx, wcid;
	BOOLEAN isMCPkt = FALSE;

	if(!pRxBlk)
		return;
		
	if(!bSuccessPkt)
	{
		rxd_base = (struct rxd_base_struct *)pRxBlk->rmac_info;
		idx = rxd_base->rxd_2.wlan_idx;
	}
	else
		idx = pRxBlk->wcid;
	wcid = idx;

	if((wcid >= WCID_ALL) || (wcid == MCAST_WCID))
		return;
	
#ifdef MT_MAC
	if(pAd->chipCap.hif_type == HIF_MT)
	{
#ifdef MULTI_APCLI_SUPPORT
		if((wcid >= APCLI_MCAST_WCID(0)))
			wcid = (idx - APCLI_MCAST_WCID(0))+FirstWcid;
#else
		if((wcid >= APCLI_MCAST_WCID))
			wcid = (idx - APCLI_MCAST_WCID)+FirstWcid;
#endif
		isMCPkt = TRUE;
	}
#endif

	if(!VALID_WCID(wcid))
	{
		DBGPRINT(RT_DEBUG_OFF, ("(%s:%d): Invalid wcid (%d) found! From MCPkt: %d\n",
				__func__,__LINE__,wcid,isMCPkt));
		return;	
	}

	pEntry = &pAd->MacTab.Content[wcid];
	if(!pEntry || !IS_ENTRY_APCLI(pEntry)) 
	   return;

	if(!VALID_TR_WCID(pEntry->wcid)) 
		return;

	tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
	if(!tr_entry) 
		return;

	if (pEntry->func_tb_idx >= MAX_APCLI_NUM) {
		DBGPRINT(RT_DEBUG_OFF, ("(%s:%d): func_tb_idx over MAX_APCLI_NUM!\n",__func__,__LINE__));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx];
	if(!pApCliEntry || 
	   !pApCliEntry->Valid ||
	   !pApCliEntry->OpenWEPErrPktChk) 
	   return;

	if(bSuccessPkt)
	{
		pHeader = pRxBlk->pHeader;
		if( pHeader &&
		   (pHeader->FC.Type == FC_TYPE_DATA) &&
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
			DBGPRINT(RT_DEBUG_OFF, ("(%s:%d): Stop Open WEP check!\n",__func__,__LINE__));
			tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			pApCliEntry->OpenWEPErrPktChk = FALSE;
		}
		if(isMCPkt) pApCliEntry->OpenWEPErrMCPktCnt++;
	}
}
#ifdef ROAMING_ENHANCE_SUPPORT
#ifndef ETH_HDR_LEN
#define ETH_HDR_LEN 14 /* dstMac(6) + srcMac(6) + protoType(2) */
#endif

#ifndef ETH_P_VLAN
#define ETH_P_VLAN       	0x8100          /* 802.1q (VLAN)  */
#endif

#ifndef VLAN_ETH_HDR_LEN
#define VLAN_ETH_HDR_LEN (ETH_HDR_LEN+4) /* 4 for h_vlan_TCI and h_vlan_encapsulated_proto */
#endif

#ifndef IP_HDR_SRC_OFFSET
#define IP_HDR_SRC_OFFSET 12 /* shift 12 for IP header len. */
#endif

#ifndef ARP_OP_OFFSET
#define ARP_OP_OFFSET 6 /* shift 6 len for ARP. */
#endif

BOOLEAN ApCliDoRoamingRefresh(
    IN RTMP_ADAPTER *pAd, 
    IN MAC_TABLE_ENTRY *pEntry, 
    IN PNDIS_PACKET pRxPacket, 
    IN struct wifi_dev *wdev,
    IN UCHAR *DestAddr)
{
    UCHAR *pPktHdr = NULL, *pLayerHdr = NULL;
    UINT16 ProtoType;
    BOOLEAN bUnicastARPReq = FALSE, bSendARP = FALSE;
    PNDIS_PACKET pPacket = NULL;
	struct sk_buff *skb = NULL;

    if(!pRxPacket || !wdev)
        return FALSE;

    /* Get the upper layer protocol type of this 802.3 pkt */
    pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
    ProtoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + (ETH_HDR_LEN-2))));
    if (ProtoType == ETH_P_VLAN)
    {
		pLayerHdr = (pPktHdr + VLAN_ETH_HDR_LEN);
        ProtoType = OS_NTOHS(get_unaligned((PUINT16)pLayerHdr));
    }
    else
		pLayerHdr = (pPktHdr + ETH_HDR_LEN);

    if(ProtoType == ETH_P_ARP)
    {
        UINT16 OpType;
        OpType = OS_NTOHS(get_unaligned((PUINT16)(pLayerHdr+ARP_OP_OFFSET))); /* ARP Operation */
        if(OpType == 0x0001) // ARP Request
        {
            if(DestAddr && !MAC_ADDR_IS_GROUP(DestAddr))
                bUnicastARPReq = TRUE;

            if(bUnicastARPReq)
            {
                skb = skb_copy(RTPKT_TO_OSPKT(pRxPacket), GFP_ATOMIC);
                if(skb)
                {
                    bSendARP = TRUE;
                    skb->dev = wdev->if_dev;
                    pPacket = OSPKT_TO_RTPKT(skb);
                    NdisMoveMemory(GET_OS_PKT_DATAPTR(pPacket), BROADCAST_ADDR, MAC_ADDR_LEN);
                }
            }
            else
                pEntry->bRoamingRefreshDone = TRUE;
        //    MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
          //      ("Got original ARP Request(Unicast:%d) from wireless STA!\n",bUnicastARPReq));
        }
    }
    else if(ProtoType == ETH_P_IP)
    {
        UINT32 SrcIP = 0;
        NdisMoveMemory(&SrcIP, (pLayerHdr + IP_HDR_SRC_OFFSET), 4);
        if(SrcIP != 0)
        {
            bSendARP = TRUE;
            pPacket = (PNDIS_PACKET)arp_create(ARPOP_REQUEST, ETH_P_ARP, SrcIP, wdev->if_dev,
                                    SrcIP, BROADCAST_ADDR, pEntry->Addr, BROADCAST_ADDR);
        }
    }

    if(bSendARP && pPacket)
    {
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
		/*Avishad Not needed*/
        //set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */
        RtmpOsPktProtocolAssign(pPacket);
        RtmpOsPktRcvHandle(pPacket);
        pEntry->bRoamingRefreshDone = TRUE;
        //MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,("Send roaming refresh done!\n"));
        return TRUE;
    }

    return FALSE;
}
#endif /* ROAMING_ENHANCE_SUPPORT */
#ifdef APCLI_DOT11W_PMF_SUPPORT
#ifdef DOT11W_PMF_SUPPORT
/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (simple_strtol(arg, 0, 10))
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC = TRUE;
	else {
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC = FALSE;
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPC = FALSE;
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR = FALSE;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired MFPC=%d\n",
				__func__, pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC));

	if (((pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA2) ||
			(pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
		|| (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
		|| (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeOWE)
#endif
		) && (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.WepStatus == Ndis802_11AESEnable)) {
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256 =
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256;
		if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC) {
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPC = TRUE;
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR =
				pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPR;

			if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR)
				pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256 = TRUE;
		}
	} else if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC)
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __func__));

	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
		__func__,
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPC,
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR,
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256));

	return TRUE;
}

/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (simple_strtol(arg, 0, 10))
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPR = TRUE;
	else {
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPR = FALSE;

		/* only close the MFPR */
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR = FALSE;
	}

	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired MFPR=%d\n",
					__func__, pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPR));

	if (((pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA2) ||
			(pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA2PSK)
#ifdef APCLI_SAE_SUPPORT
			|| (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeWPA3PSK)
#endif
#ifdef APCLI_OWE_SUPPORT
			|| (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.AuthMode == Ndis802_11AuthModeOWE)
#endif
			)
			&& (pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.WepStatus == Ndis802_11AESEnable)) {
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256 =
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256;
		if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC) {
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPC = TRUE;
			pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR =
				pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPR;

			if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR)
				pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256 = TRUE;
		}
	} else if (pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_MFPC)
		DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK AES\n", __func__));

	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
		__func__, pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPC,
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.MFPR,
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.PMFSHA256));

	return TRUE;
}

INT Set_ApCliPMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING *arg)
{
	POS_COOKIE pObj;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (simple_strtol(arg, 0, 10))
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256 = TRUE;
	else
		pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256 = FALSE;

	DBGPRINT(RT_DEBUG_ERROR, ("[PMF]%s:: Desired PMFSHA256=%d\n",
			__func__, pAd->ApCfg.ApCliTab[pObj->ioctl_if].PmfCfg.Desired_PMFSHA256));

	return TRUE;
}
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_DOT11W_PMF_SUPPORT */

#ifdef APCLI_SAE_SUPPORT
INT set_apcli_sae_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaeCfgGroup = NULL;
	UCHAR group = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pSaeCfgGroup = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].sae_cfg_group;


	group = simple_strtol(arg, 0, 10);

	if ((group == 19) || (group == 20)) {
		*pSaeCfgGroup = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SAE]%s:: Set group=%d\n",
				 __func__, group));

	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SAE]%s:: group=%d not supported\n",
				 __func__, group));

		return FALSE;
	}
	return TRUE;
}
#endif/*APCLI_SAE_SUPPORT*/



#ifdef APCLI_OWE_SUPPORT
INT set_apcli_owe_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR group = 0;
	UCHAR *pcurr_group = NULL;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pcurr_group = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].curr_owe_group;


	group = simple_strtol(arg, 0, 10);
/*OWE-currently allowing configuration of groups 19(mandatory) and 20(optional) */
	if ((group == 19) || (group == 20)) {
		*pcurr_group = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[OWE]%s:: Set group=%d\n",
				 __func__, group));

	} else {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[OWE]%s:: group=%d not supported\n",
				 __func__, group));

		return FALSE;
	}
	return TRUE;
}
#endif/*APCLI_SAE_SUPPORT*/


#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
INT set_apcli_del_pmkid_list(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	UCHAR action = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;


	action = simple_strtol(arg, 0, 10);

/*Delete all pmkid list associated with this  ApCli Interface*/
	if (action == 1) {
		apcli_delete_pmkid_cache_all(pAd, pObj->ioctl_if);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s:: Delete PMKID list (%d)\n",
				 __func__, action));

	}
	return TRUE;
}



INT apcli_add_pmkid_cache(
	IN	PRTMP_ADAPTER	pAd,
	IN UCHAR *paddr,
	IN UCHAR *pmkid,
	IN UCHAR *pmk,
	IN UINT8 pmk_len,
	IN UINT8 if_index
#ifdef MAC_REPEATER_SUPPORT
	, IN UINT8 cli_idx
#endif
	)

{
	PAPCLI_STRUCT papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PUINT psaved_pmk_num = NULL;
	UCHAR update_pmkid = FALSE;
	VOID *psaved_pmk_lock = NULL;


#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.ApCliTab[if_index].RepeaterCli[cli_idx];
		papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


	} else
#endif
	{
		papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;
	}

	cached_idx = apcli_search_pmkid_cache(pAd, paddr, if_index
#ifdef MAC_REPEATER_SUPPORT
		, cli_idx
#endif
		);

	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);


	if (cached_idx != INVALID_PMKID_IDX) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
			("%s :PMKID found, %d\n", __func__, cached_idx));
	} else {
/* Find free cache entry */
		for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {
			if (psaved_pmk[cached_idx].Valid == FALSE)
				break;
		}

		if (cached_idx < PMKID_NO) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
				("Free Cache entry found,cached_idx %d\n", cached_idx));
			*psaved_pmk_num = *psaved_pmk_num + 1;

		} else {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF,
				("cache full, overwrite cached_idx 0\n"));

			cached_idx = 0;
		}
		update_pmkid = TRUE;
	}

	if (update_pmkid == TRUE) {
		psaved_pmk[cached_idx].Valid = TRUE;
		COPY_MAC_ADDR(&psaved_pmk[cached_idx].BSSID, paddr);
		NdisMoveMemory(&psaved_pmk[cached_idx].PMKID, pmkid, LEN_PMKID);
		NdisMoveMemory(&psaved_pmk[cached_idx].PMK, pmk, pmk_len);
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): add %02x:%02x:%02x:%02x:%02x:%02x cache(%d)\n",
			__func__, PRINT_MAC(paddr), cached_idx));
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);


	return cached_idx;
}


INT apcli_search_pmkid_cache(
	IN	PRTMP_ADAPTER	pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index
#ifdef MAC_REPEATER_SUPPORT
	, IN UINT8 cli_idx
#endif
	)

{
	INT	i = 0;
	PBSSID_INFO psaved_pmk = NULL;
	PAPCLI_STRUCT papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;



#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.ApCliTab[if_index].RepeaterCli[cli_idx];
		papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


		} else
#endif
	{
		papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

	}

	if (psaved_pmk_lock)
		NdisAcquireSpinLock(psaved_pmk_lock);

	for (i = 0; i < PMKID_NO; i++) {
		if ((psaved_pmk[i].Valid == TRUE)
			&& MAC_ADDR_EQUAL(&psaved_pmk[i].BSSID, paddr)) {
			MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s():%02x:%02x:%02x:%02x:%02x:%02x cache(%d)\n",
				__func__, PRINT_MAC(paddr), i));
			break;
		}
	}

	if (psaved_pmk_lock)
		NdisReleaseSpinLock(psaved_pmk_lock);

	if (i >= PMKID_NO) {
		MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s():  not found\n", __func__));
		return INVALID_PMKID_IDX;
	}

	return i;
}



VOID apcli_delete_pmkid_cache(
	IN	PRTMP_ADAPTER	pAd,
	IN UCHAR *paddr,
	IN UCHAR if_index
#ifdef MAC_REPEATER_SUPPORT
	, IN UINT8 cli_idx
#endif
	)

{

	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PAPCLI_STRUCT papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;


#ifdef MAC_REPEATER_SUPPORT
	if (cli_idx != 0xff) {
		preptcli_entry = &pAd->ApCfg.ApCliTab[if_index].RepeaterCli[cli_idx];
		psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
		psaved_pmk_num = &preptcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


	} else
#endif
	{
		papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
		psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
		psaved_pmk_num = &papcli_entry->SavedPMKNum;
		psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

	}


	cached_idx = apcli_search_pmkid_cache(pAd, paddr, if_index
#ifdef MAC_REPEATER_SUPPORT
		, cli_idx
#endif
		);

	if (cached_idx != INVALID_PMKID_IDX) {

		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			psaved_pmk[cached_idx].Valid = FALSE;

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;
		}

		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);


	}

}

VOID apcli_delete_pmkid_cache_all(
	IN	PRTMP_ADAPTER	pAd,
	IN UCHAR if_index)
{

	INT cli_idx = 0;
	INT cached_idx;
	PBSSID_INFO psaved_pmk = NULL;
	PAPCLI_STRUCT papcli_entry = NULL;
#ifdef MAC_REPEATER_SUPPORT
	PREPEATER_CLIENT_ENTRY preptcli_entry = NULL;
#endif
#ifdef APCLI_SAE_SUPPORT
	SAE_INSTANCE *pSaeIns = NULL;
	SAE_CFG *pSaeCfg = NULL;
	UINT32 i;
	UINT32 ins_cnt = 0;
#endif

	VOID *psaved_pmk_lock = NULL;
	PUINT psaved_pmk_num = NULL;



	papcli_entry = &pAd->ApCfg.ApCliTab[if_index];
	psaved_pmk = (PBSSID_INFO)&papcli_entry->SavedPMK[0];
	psaved_pmk_num = &papcli_entry->SavedPMKNum;
	psaved_pmk_lock = (VOID *)&papcli_entry->SavedPMK_lock;

#ifdef APCLI_SAE_SUPPORT
	pSaeCfg = &pAd->SaeCfg;
/*Delete all SAE instances for this ApCli Interface*/
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {

		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, papcli_entry->wdev.if_addr, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
				delete_sae_instance(pSaeIns);
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
			}
		}

		ins_cnt++;

		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif



/*Delete ApCli PMKID list*/
	for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {

		if (psaved_pmk_lock)
			NdisAcquireSpinLock(psaved_pmk_lock);

		if (psaved_pmk[cached_idx].Valid == TRUE) {
			psaved_pmk[cached_idx].Valid = FALSE;

			if (*psaved_pmk_num)
				*psaved_pmk_num = *psaved_pmk_num - 1;

		}

		if (psaved_pmk_lock)
			NdisReleaseSpinLock(psaved_pmk_lock);

	}
/*Delete  PMKID list for MacRepeater linked with ApCli */

#ifdef MAC_REPEATER_SUPPORT
	if (pAd->ApCfg.bMACRepeaterEn == TRUE) {

		for (cli_idx = 0; cli_idx < MAX_EXT_MAC_ADDR_SIZE; cli_idx++) {

			preptcli_entry = &pAd->ApCfg.ApCliTab[if_index].RepeaterCli[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE) &&
				(preptcli_entry->MatchApCliIdx == papcli_entry->wdev.func_idx)) {

#ifdef APCLI_SAE_SUPPORT
			/*Delete all SAE instances for this Rept entry*/
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				ins_cnt = 0;

				for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
					if (pSaeCfg->sae_ins[i].valid == FALSE)
						continue;

					if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac,
						preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
						pSaeIns = &pSaeCfg->sae_ins[i];
						if (pSaeIns != NULL && (pSaeIns->valid == TRUE)) {
							NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
							delete_sae_instance(pSaeIns);
							NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

						}
					}

					ins_cnt++;

				if (ins_cnt == pSaeCfg->total_ins)
					break;
			}

				NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
#endif
				psaved_pmk = (PBSSID_INFO)&preptcli_entry->SavedPMK[0];
				psaved_pmk_num = &preptcli_entry->SavedPMKNum;
				psaved_pmk_lock = (VOID *)&preptcli_entry->SavedPMK_lock;


				for (cached_idx = 0; cached_idx < PMKID_NO; cached_idx++) {

					if (psaved_pmk_lock)
						NdisAcquireSpinLock(psaved_pmk_lock);

					if (psaved_pmk[cached_idx].Valid == TRUE) {
						psaved_pmk[cached_idx].Valid = FALSE;

						if (*psaved_pmk_num)
							*psaved_pmk_num = *psaved_pmk_num - 1;
					}

					if (psaved_pmk_lock)
						NdisReleaseSpinLock(psaved_pmk_lock);
				}

			}
		}
	}
#endif

}
#endif


#ifdef APCLI_OWE_SUPPORT

VOID apcli_reset_owe_parameters(
		IN	PRTMP_ADAPTER	pAd,
		IN UCHAR if_index) {

	PAPCLI_STRUCT papcli_entry = NULL;

	papcli_entry = &pAd->ApCfg.ApCliTab[if_index];

	/*OWE Trans reset the OWE trans bssid and ssid*/

	if (papcli_entry
		&& papcli_entry->wdev.AuthMode == Ndis802_11AuthModeOWE
		&& (papcli_entry->owe_trans_ssid_len > 0)) {
		NdisZeroMemory(papcli_entry->owe_trans_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_ssid_len = 0;

		NdisZeroMemory(papcli_entry->owe_trans_open_bssid, MAC_ADDR_LEN);
		NdisZeroMemory(papcli_entry->owe_trans_open_ssid, MAX_LEN_OF_SSID);
		papcli_entry->owe_trans_open_ssid_len = 0;
	}
}

#endif


#endif /* APCLI_SUPPORT */
