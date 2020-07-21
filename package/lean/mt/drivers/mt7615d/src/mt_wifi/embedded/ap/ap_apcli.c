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

    5. ApCli index (0) of different interface is got in tx_pkt_handle() by using

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

#ifdef DOT11_VHT_AC
#ifdef APCLI_CERT_SUPPORT
extern UINT16 vht_max_mpdu_size[];
#endif
#endif /* DOT11_VHT_AC */
#ifdef MAC_REPEATER_SUPPORT
VOID ReptWaitLinkDown(struct _REPEATER_CLIENT_ENTRY *pReptEntry);
#endif /*MAC_REPEATER_SUPPORT*/

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
#ifdef MULTI_INF_SUPPORT
/* Index 0 for Card_1, Index 1 for Card_2 */
extern VOID *adapt_list[MAX_NUM_OF_INF];
#endif /* MULTI_INF_SUPPORT */
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

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

	for (loop = 0; loop < MAX_APCLI_NUM; loop++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[loop];

		if ((pApCliEntry->Valid == TRUE) && (VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))) {
			/*
			      When we are connected and do the scan progress, it's very possible we cannot receive
			      the beacon of the AP. So, here we simulate that we received the beacon.
			     */
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_BSS_SCAN_IN_PROGRESS) &&
				(RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1 * OS_HZ)))) {
				BPtoJiffies = (((pApCliEntry->ApCliBeaconPeriod * 1024 / 1000) * OS_HZ) / 1000);
				timeDiff = (pAd->Mlme.Now32 - pApCliEntry->ApCliRcvBeaconTime) / BPtoJiffies;

				if (timeDiff > 0)
					pApCliEntry->ApCliRcvBeaconTime += (timeDiff * BPtoJiffies);

				if (RTMP_TIME_AFTER(pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32)) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("MMCHK - APCli BeaconRxTime adjust wrong(BeaconRx=0x%lx, Now=0x%lx)\n",
							 pApCliEntry->ApCliRcvBeaconTime, pAd->Mlme.Now32));
				}
			}

			/* update channel quality for Roaming and UI LinkQuality display */
			MlmeCalculateChannelQuality(pAd, &pAd->MacTab.Content[pApCliEntry->MacTabWCID], Now32);
		}
	}
}


/*
* Synchronized function
*/

static VOID ApCliCompleteInit(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_INIT_COMPLETION(&pApCliEntry->ifdown_complete);
	RTMP_OS_INIT_COMPLETION(&pApCliEntry->linkdown_complete);
#ifdef APCLI_CFG80211_SUPPORT
	RTMP_OS_INIT_COMPLETION(&pApCliEntry->scan_complete);
#endif /* APCLI_CFG80211_SUPPORT */
}

static VOID ApCliLinkDownComplete(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_COMPLETE(&pApCliEntry->linkdown_complete);
}

static VOID ApCliWaitLinkDown(APCLI_STRUCT *pApCliEntry)
{
	if (pApCliEntry->Valid && !RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pApCliEntry->linkdown_complete, APCLI_WAIT_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s) ApCli [%d] can't done.\n", __func__, pApCliEntry->wdev.func_idx));
	}
}

static VOID ApCliWaitStateDisconnect(APCLI_STRUCT *pApCliEntry)
{
	/*
	 * Before doing wdev_do_close,we have to make sure the ctrl
	* state machine has switched to APCLI_CTRL_DISCONNECTED
	 */
	int wait_cnt = 0;
	int wait_times = 50;
	int delay_time = 100;

	while (pApCliEntry->CtrlCurrState != APCLI_CTRL_DISCONNECTED) {
		if (wait_cnt >= wait_times) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("Need to debug apcli ctrl state machine(Ctrl State=%lu)\n\r", pApCliEntry->CtrlCurrState));
			break;
		}

		OS_WAIT(delay_time);
		wait_cnt++;
	}
}


static VOID ApCliIfDownComplete(APCLI_STRUCT *pApCliEntry)
{
	RTMP_OS_COMPLETE(&pApCliEntry->ifdown_complete);
}

static VOID ApCliWaitIfDown(APCLI_STRUCT *pApCliEntry)
{
	if (pApCliEntry->Valid  &&
		!RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&pApCliEntry->ifdown_complete, APCLI_WAIT_TIMEOUT)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("%s: wait ApCli [%d] interface down faild!!\n", __func__, pApCliEntry->wdev.func_idx));
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

	for (index = 0; index < MAX_APCLI_NUM; index++) {
		if (ad_p->ApCfg.ApCliTab[index].wdev.if_dev)
			RtmpOSNetDevClose(ad_p->ApCfg.ApCliTab[index].wdev.if_dev);
	}

}


/* --------------------------------- Private -------------------------------- */
INT ApCliIfLookUp(RTMP_ADAPTER *pAd, UCHAR *pAddr)
{
	SHORT if_idx;

	for (if_idx = 0; if_idx < MAX_APCLI_NUM; if_idx++) {
		if (MAC_ADDR_EQUAL(pAd->ApCfg.ApCliTab[if_idx].wdev.if_addr, pAddr)) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():ApCliIfIndex=%d\n",
					 __func__, if_idx));
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
	INOUT HEADER_802_11 * pHdr80211,
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
		pAd				Pointer to our adapter

	Return Value:
		FALSE if pAd->CommonCfg.SupportedHtPhy doesn't accept the pHtCapability.  (AP Mode)

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
BOOLEAN ApCliCheckHt(
	IN RTMP_ADAPTER *pAd,
	IN USHORT IfIndex,
	INOUT HT_CAPABILITY_IE * pHtCapability,
	INOUT ADD_HT_INFO_IE * pAddHtInfo)
{
	APCLI_STRUCT *pApCliEntry = NULL;
	HT_CAPABILITY_IE *aux_ht_cap;
	HT_CAPABILITY_IE *curr_ht_cap;
	struct wifi_dev *wdev;
	UCHAR cfg_ht_bw;
	UCHAR op_ext_cha;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	if (IfIndex >= MAX_APCLI_NUM)
		return FALSE;

	pApCliEntry = &pAd->ApCfg.ApCliTab[IfIndex];
	wdev = &pApCliEntry->wdev;
	cfg_ht_bw = wlan_config_get_ht_bw(wdev);
	op_ext_cha = wlan_operate_get_ext_cha(wdev);
	curr_ht_cap = (HT_CAPABILITY_IE *)wlan_operate_get_ht_cap(wdev);
	aux_ht_cap = &pApCliEntry->MlmeAux.HtCapability;
	aux_ht_cap->MCSSet[0] = 0xff;

	/* For 20MHz bandwidth, setting MCS 32 bit is not required */
	if (cfg_ht_bw != HT_BW_20)
		aux_ht_cap->MCSSet[4] = 0x1;

	switch (wlan_operate_get_rx_stream(wdev)) {
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

	case 4:
		aux_ht_cap->MCSSet[0] = 0xff;
		aux_ht_cap->MCSSet[1] = 0xff;
		aux_ht_cap->MCSSet[2] = 0xff;
		aux_ht_cap->MCSSet[3] = 0xff;
		break;
	}

	aux_ht_cap->MCSSet[0] &= pHtCapability->MCSSet[0];
	aux_ht_cap->MCSSet[1] &= pHtCapability->MCSSet[1];
	aux_ht_cap->MCSSet[2] &= pHtCapability->MCSSet[2];
	aux_ht_cap->MCSSet[3] &= pHtCapability->MCSSet[3];
	/* Record the RxMcs of AP */
	NdisMoveMemory(pApCliEntry->RxMcsSet, pHtCapability->MCSSet, 16);
	/* choose smaller setting */
#ifdef CONFIG_MULTI_CHANNEL
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth;
#else /* CONFIG_MULTI_CHANNEL */
	aux_ht_cap->HtCapInfo.ChannelWidth = pAddHtInfo->AddHtInfo.RecomWidth & cfg_ht_bw;
#endif /* !CONFIG_MULTI_CHANNEL */

	/* it should go back to bw 20 if extension channel is different with root ap */
	if (op_ext_cha != pAddHtInfo->AddHtInfo.ExtChanOffset) {
		if (pApCliEntry->wdev.channel > 14) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliCheckHt :  channel=%u,  my extcha=%u, root ap extcha=%u, inconsistent!!\n",
					 pApCliEntry->wdev.channel, op_ext_cha, pAddHtInfo->AddHtInfo.ExtChanOffset));
		}

		aux_ht_cap->HtCapInfo.ChannelWidth = BW_20;
		wlan_operate_set_ht_bw(wdev, HT_BW_20, EXTCHA_NONE);
	} else
		wlan_operate_set_ht_bw(wdev, pAddHtInfo->AddHtInfo.RecomWidth, pAddHtInfo->AddHtInfo.ExtChanOffset);

	/* If selcected BW of APCLI is changed to 20MHZ, reset MCS 32 bit */
	if (aux_ht_cap->HtCapInfo.ChannelWidth == BW_20) {
		aux_ht_cap->MCSSet[4] = 0x0;
	}

	aux_ht_cap->HtCapInfo.GF =  pHtCapability->HtCapInfo.GF & curr_ht_cap->HtCapInfo.GF;
#ifdef CONFIG_MULTI_CHANNEL /* APCLI's bw , Central , channel */

	if (RTMP_CFG80211_VIF_P2P_CLI_ON(pAd)) {
		UCHAR ht_bw = aux_ht_cap->HtCapInfo.ChannelWidth;
		UCHAR ext_cha;
		UCHAR cen_ch;

		if (ht_bw == HT_BW_20) {
			pApCliEntry->wdev.channel = pAddHtInfo->ControlChan;
			ext_cha = EXTCHA_NONE;
		} else if (ht_bw == HT_BW_40) {
			pApCliEntry->wdev.channel = pAddHtInfo->ControlChan;

			if (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_ABOVE)
				ext_cha = EXTCHA_ABOVE;
			else if (pAddHtInfo->AddHtInfo.ExtChanOffset == EXTCHA_BELOW)
				ext_cha = EXTCHA_BELOW;
			else /* EXTCHA_NONE , should not be here!*/
				ext_cha = EXTCHA_NONE;
		}
		wlan_operate_set_ht_bw(wdev, ht_bw, ext_cha);
		cen_ch = wlan_operate_get_cen_ch_1(wdev);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliCheckHt :  channel=%u,  CentralChannel=%u, bw=%u\n",
			wdev->channel, cen_ch, ht_bw));
	}

#endif /* CONFIG_MULTI_CHANNEL */
	/* Send Assoc Req with my HT capability. */
	aux_ht_cap->HtCapInfo.AMsduSize = curr_ht_cap->HtCapInfo.AMsduSize;
	aux_ht_cap->HtCapInfo.MimoPs = pHtCapability->HtCapInfo.MimoPs;
	aux_ht_cap->HtCapInfo.ShortGIfor20 =
		(curr_ht_cap->HtCapInfo.ShortGIfor20) & (pHtCapability->HtCapInfo.ShortGIfor20);
	aux_ht_cap->HtCapInfo.ShortGIfor40 =
		(curr_ht_cap->HtCapInfo.ShortGIfor40) & (pHtCapability->HtCapInfo.ShortGIfor40);
	aux_ht_cap->HtCapInfo.TxSTBC = (curr_ht_cap->HtCapInfo.TxSTBC)&(pHtCapability->HtCapInfo.RxSTBC);
	aux_ht_cap->HtCapInfo.RxSTBC = (curr_ht_cap->HtCapInfo.RxSTBC)&(pHtCapability->HtCapInfo.TxSTBC);

	/* Fix throughput issue for some vendor AP with AES mode */
	if (pAddHtInfo->AddHtInfo.RecomWidth & cfg_ht_bw)
		aux_ht_cap->HtCapInfo.CCKmodein40 = pHtCapability->HtCapInfo.CCKmodein40;
	else
		aux_ht_cap->HtCapInfo.CCKmodein40 = 0;

	aux_ht_cap->HtCapParm.MaxRAmpduFactor = curr_ht_cap->HtCapParm.MaxRAmpduFactor;
	aux_ht_cap->HtCapParm.MpduDensity = pHtCapability->HtCapParm.MpduDensity;
	aux_ht_cap->ExtHtCapInfo.PlusHTC = pHtCapability->ExtHtCapInfo.PlusHTC;

	if (pAd->CommonCfg.bRdg)
		aux_ht_cap->ExtHtCapInfo.RDGSupport = pHtCapability->ExtHtCapInfo.RDGSupport;

	if (wlan_config_get_ht_ldpc(wdev) && (cap->phy_caps & fPHY_CAP_LDPC))
		aux_ht_cap->HtCapInfo.ht_rx_ldpc = pHtCapability->HtCapInfo.ht_rx_ldpc;
	else
		aux_ht_cap->HtCapInfo.ht_rx_ldpc = 0;

	aux_ht_cap->TxBFCap = pHtCapability->TxBFCap;

	/*COPY_AP_HTSETTINGS_FROM_BEACON(pAd, pHtCapability); */
	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


#ifdef APCLI_CERT_SUPPORT
void ApCliCertEDCAAdjust(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PEDCA_PARM pEdcaParm)
{
	UCHAR       Cwmin[WMM_NUM_OF_AC] = {3, 3, 3, 3};
	UCHAR       Cwmax[WMM_NUM_OF_AC] = {4, 4, 4, 4};

	if (pAd->bApCliCertTest && (wdev->wdev_type == WDEV_TYPE_APCLI)) {
		PAPCLI_STRUCT pApCliEntry = NULL;
		/* SSID for TGn TC 5.2.7 */
		UCHAR Ssid1[] = "GLKDAJ98~@";
		/* SSID for TGn TC 5.2.13 */
		UCHAR Ssid2[] = "WPA2";
		UCHAR Ssid1Equal = 0;
		UCHAR Ssid2Equal = 0;

		pApCliEntry = (PAPCLI_STRUCT)wdev->func_dev;
		if (pApCliEntry) {
			Ssid1Equal = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen,
						Ssid1, strlen(Ssid1));
			Ssid2Equal = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen,
						Ssid2, strlen(Ssid2));
		}
		/* To tame down the BE aggresiveness increasing the Cwmin */
		if (Ssid1Equal || Ssid2Equal) {
			if (pEdcaParm->Cwmin[0] == 4)
				pEdcaParm->Cwmin[0]++;
			return;
		}

		/* fix 5.2.29 step 7 fail */
		if ((pEdcaParm->Cwmin[2] == 3) &&
				(pEdcaParm->Cwmax[2] == 4)) {
			pEdcaParm->Cwmin[2]++;
			pEdcaParm->Cwmax[2]++;
			if (pEdcaParm->Txop[2] == 94)
				pEdcaParm->Txop[2] = pEdcaParm->Txop[2] - 9;
		}
	}

	if ((memcmp(pEdcaParm->Cwmin, Cwmin, 4) == 0) &&
		(memcmp(pEdcaParm->Cwmax, Cwmax, 4) == 0)) {
		/* ignore 5.2.32*/
		return;
	}
}
#endif


#ifdef CONVERTER_MODE_SWITCH_SUPPORT
void V10ConverterModeStartStop(RTMP_ADAPTER *pAd, BOOLEAN BeaconStart)
{

	UCHAR idx = 0;
	UCHAR adIdx = 0;
	RTMP_ADAPTER *pAdapter = pAd;

#ifdef MULTI_INF_SUPPORT
	for (adIdx = 0; adIdx < MAX_NUM_OF_INF; adIdx++)
#endif /* MULTI_INF_SUPPORT */
	{
#ifdef MULTI_INF_SUPPORT
		pAdapter = (RTMP_ADAPTER *)adapt_list[adIdx];
#endif /* MULTI_INF_SUPPORT */
			if (pAdapter) {
				if (BeaconStart) {
#ifdef MULTI_INF_SUPPORT
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("%s() : Resume Beaconing, Interface = %u\n", __func__, multi_inf_get_idx(pAdapter)));
#else
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s() : Resume Beaconing\n", __func__));
#endif /* MULTI_INF_SUPPORT */
					for (idx = 0; idx < pAdapter->ApCfg.BssidNum; idx++) {
						BSS_STRUCT *pMbss = &pAdapter->ApCfg.MBSSID[idx];
						pMbss->APStartPseduState = AP_STATE_ALWAYS_START_AP_DEFAULT;
						if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
							pMbss->wdev.bAllowBeaconing = TRUE;
							if (wdev_do_linkup(&pMbss->wdev, NULL) != TRUE)
								MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									 ("%s: link up fail!!\n", __func__));
						}
					}
				} else {
#ifdef MULTI_INF_SUPPORT
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						("%s() : Pause Beaconing, Interface = %u\n", __func__, multi_inf_get_idx(pAdapter)));
#else
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s() : Pause Beaconing\n", __func__));
#endif /* MULTI_INF_SUPPORT */
						for (idx = 0; idx < pAdapter->ApCfg.BssidNum; idx++) {
							BSS_STRUCT *pMbss = &pAdapter->ApCfg.MBSSID[idx];
							if (WDEV_WITH_BCN_ABILITY(&pMbss->wdev)) {
								pMbss->wdev.bAllowBeaconing = FALSE;
								if (wdev_do_linkdown(&pMbss->wdev) != TRUE)
									MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
									 ("%s: link down fail!!\n", __func__));
							}
							pMbss->APStartPseduState = AP_STATE_START_AFTER_APCLI_CONNECTION;
						}
				}
			}
	}
}
#endif /*CONVERTER_MODE_SWITCH_SUPPORT*/



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
	struct _REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* defined(MAC_REPEATER_SUPPORT) || defined(MT_MAC) */
#ifdef APCLI_AUTO_CONNECT_SUPPORT
	USHORT apcli_ifIndex;
#endif
	UCHAR ext_cha;

	do {
		if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
			|| (ifIndex >= REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
		) {
#ifdef MAC_REPEATER_SUPPORT

			if (pAd->ApCfg.bMACRepeaterEn) {
				if (ifIndex < MAX_APCLI_NUM) {
#ifdef LINUX
					struct net_device *pNetDev;
					struct net *net = &init_net;
					/* old kernerl older than 2.6.21 didn't have for_each_netdev()*/
#ifndef for_each_netdev

					for (pNetDev = dev_base; pNetDev != NULL; pNetDev = pNetDev->next)
#else
					for_each_netdev(net, pNetDev)
#endif
					{
						if (pNetDev->priv_flags == IFF_EBRIDGE) {
							COPY_MAC_ADDR(pAd->ApCfg.BridgeAddress, pNetDev->dev_addr);
							MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" Bridge Addr = %02X:%02X:%02X:%02X:%02X:%02X. !!!\n",
									 PRINT_MAC(pAd->ApCfg.BridgeAddress)));
						}

						pSkipEntry = RepeaterInvaildMacLookup(pAd, pNetDev->dev_addr);

						if (pSkipEntry == NULL)
							InsertIgnoreAsRepeaterEntryTable(pAd, pNetDev->dev_addr);
					}

					if (!MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, ZERO_MAC_ADDR)) {
						pSkipEntry = RepeaterInvaildMacLookup(pAd, pAd->ApCfg.BridgeAddress);

						if (pSkipEntry) {
							UCHAR MacAddr[MAC_ADDR_LEN];
							UCHAR entry_idx;

							COPY_MAC_ADDR(MacAddr, pSkipEntry->MacAddr);
							entry_idx = pSkipEntry->entry_idx;
							RepeaterRemoveIngoreEntry(pAd, entry_idx, MacAddr);
						}
					}

#endif
				}

				if (ifIndex >= REPT_MLME_START_IDX) {
					CliIdx = ifIndex - REPT_MLME_START_IDX;
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
					ifIndex = pReptEntry->wdev->func_idx;
					pMacEntry = MacTableLookup(pAd, pReptEntry->OriginalAddress);

					if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry))
						pReptEntry->bEthCli = FALSE;
					else
						pReptEntry->bEthCli = TRUE;

					pMacEntry = NULL;
				}
			}

#endif /* MAC_REPEATER_SUPPORT */
		} else {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("!!! ERROR : APCLI LINK UP - IF(apcli%d)!!!\n", ifIndex));
			result = FALSE;
			break;
		}

#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx != 0xFF)
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("(%s) ifIndex = %d, CliIdx = %d !!!\n",
					  __func__, ifIndex, CliIdx));
		else
#endif /* MAC_REPEATER_SUPPORT */
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("(%s) ifIndex = %d!!!\n",
					  __func__, ifIndex));
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		apcli_ifIndex = (USHORT)(ifIndex);
#endif /* APCLI_AUTO_CONNECT_SUPPORT */
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

		if ((pApCliEntry->Valid)
#ifdef MAC_REPEATER_SUPPORT
			&& (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		   ) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("!!! ERROR : This link had existed - IF(apcli%d)!!!\n",
					  ifIndex));
			result = FALSE;
			break;
		}

		wdev = &pApCliEntry->wdev;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
		if (pApCliEntry->ApCliMode == APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION) {
#ifdef WAPP_SUPPORT
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s() :APCLI Linkup Event send to wapp\n", __func__));
		wapp_send_apcli_association_change_vendor10(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
#else
		V10ConverterModeStartStop(pAd, TRUE);
#endif
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT

		if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (wf_drv_tbl.wf_fwd_get_rep_hook)
				wf_drv_tbl.wf_fwd_get_rep_hook(pAd->CommonCfg.EtherTrafficBand);

			if (wf_drv_tbl.wf_fwd_check_device_hook)
				wf_drv_tbl.wf_fwd_check_device_hook(wdev->if_dev, INT_APCLI, CliIdx, wdev->channel, 1);

			if (wf_drv_tbl.wf_fwd_entry_insert_hook)
				wf_drv_tbl.wf_fwd_entry_insert_hook(wdev->if_dev, pAd->net_dev, pAd);

			if (wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook)
#ifdef MAC_REPEATER_SUPPORT
				wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook(pAd, &pAd->ApCfg.ReptCliEntryLock,
				&pAd->ApCfg.ReptCliHash[0], &pAd->ApCfg.ReptMapHash[0],
				&pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
#else
				wf_drv_tbl.wf_fwd_insert_repeater_mapping_hook(pAd,
				NULL, NULL, NULL, &pAd->ApCfg.ApCliTab[ifIndex].wdev.if_addr);
#endif /* MAC_REPEATER_SUPPORT */
		}

#endif /* CONFIG_WIFI_PKT_FWD */
		/* Insert the Remote AP to our MacTable. */
		/*pMacEntry = MacTableInsertApCliEntry(pAd, (PUCHAR)(pAd->ApCfg.ApCliTab[0].MlmeAux.Bssid)); */
#ifdef FAST_EAPOL_WAR
#ifdef MAC_REPEATER_SUPPORT

		if ((pAd->ApCfg.bMACRepeaterEn) &&
			(IS_HIF_TYPE(pAd, HIF_MT)) &&
			(CliIdx != 0xFF)) {
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

			if (pReptEntry->pre_entry_alloc == TRUE)
				pMacEntry = &pAd->MacTab.Content[pReptEntry->MacTabWCID];
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (pApCliEntry->pre_entry_alloc == TRUE)
				pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];
		}

#else /* FAST_EAPOL_WAR */
#ifdef MAC_REPEATER_SUPPORT

		if ((pAd->ApCfg.bMACRepeaterEn) &&
			(IS_HIF_TYPE(pAd, HIF_MT)) &&
			(CliIdx != 0xFF)) {
			pMacEntry = MacTableInsertEntry(
							pAd,
							(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
							wdev,
							ENTRY_REPEATER,
							OPMODE_AP,
							TRUE);
		} else
#endif /* MAC_REPEATER_SUPPORT */
			pMacEntry = MacTableInsertEntry(pAd, (PUCHAR)(pApCliEntry->MlmeAux.Bssid),
											wdev, ENTRY_APCLI,
											OPMODE_AP, TRUE);

#endif /* !FAST_EAPOL_WAR */
#if (!(defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)))
/*Shifted to before Auth as now mac table entry is *
created before Auth wi Fast Eapol WAR */

		if (pMacEntry) {
			struct _SECURITY_CONFIG *pProfile_SecConfig = &wdev->SecConfig;
			struct _SECURITY_CONFIG *pEntry_SecConfig = &pMacEntry->SecConfig;
#ifdef DOT11W_PMF_SUPPORT
			/*fill the pMacEntry's PMF parameters*/
			{
				RSN_CAPABILITIES RsnCap;

				NdisMoveMemory(&RsnCap, &pApCliEntry->MlmeAux.RsnCap, sizeof(RSN_CAPABILITIES));
				RsnCap.word = cpu2le16(RsnCap.word);

				/*mismatch case*/
				if (((pProfile_SecConfig->PmfCfg.MFPR) && (RsnCap.field.MFPC == FALSE))
					|| ((pProfile_SecConfig->PmfCfg.MFPC == FALSE) && (RsnCap.field.MFPR))) {
					pEntry_SecConfig->PmfCfg.UsePMFConnect = FALSE;
					pEntry_SecConfig->key_deri_alg = SEC_KEY_DERI_SHA1;
				}

				if ((pProfile_SecConfig->PmfCfg.MFPC) && (RsnCap.field.MFPC)) {
					pEntry_SecConfig->PmfCfg.UsePMFConnect = TRUE;

					if ((pApCliEntry->MlmeAux.IsSupportSHA256KeyDerivation) || (RsnCap.field.MFPR))
						pEntry_SecConfig->key_deri_alg = SEC_KEY_DERI_SHA256;

					pEntry_SecConfig->PmfCfg.MFPC = RsnCap.field.MFPC;
					pEntry_SecConfig->PmfCfg.MFPR = RsnCap.field.MFPR;
				}

				pEntry_SecConfig->PmfCfg.igtk_cipher = pApCliEntry->MlmeAux.IntegrityGroupCipher;
			}
#endif /* DOT11W_PMF_SUPPORT */

			if (IS_CIPHER_WEP(pEntry_SecConfig->PairwiseCipher)) {
				os_move_mem(pEntry_SecConfig->WepKey, pProfile_SecConfig->WepKey,  sizeof(SEC_KEY_INFO)*SEC_KEY_NUM);
				pProfile_SecConfig->GroupKeyId = pProfile_SecConfig->PairwiseKeyId;
				pEntry_SecConfig->PairwiseKeyId = pProfile_SecConfig->PairwiseKeyId;
			} else {
				CHAR rsne_idx = 0;
				{

				NdisCopyMemory(pEntry_SecConfig->PMK,
					pProfile_SecConfig->PMK,
					LEN_PMK);
				}

#ifdef MAC_REPEATER_SUPPORT

				if ((pAd->ApCfg.bMACRepeaterEn) && (IS_HIF_TYPE(pAd, HIF_MT)) && (CliIdx != 0xFF)) {
					os_move_mem(pEntry_SecConfig->Handshake.AAddr, pMacEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(pEntry_SecConfig->Handshake.SAddr, pReptEntry->CurrentAddress, MAC_ADDR_LEN);
				} else
#endif /* MAC_REPEATER_SUPPORT */
				{
					os_move_mem(pEntry_SecConfig->Handshake.AAddr, pMacEntry->Addr, MAC_ADDR_LEN);
					os_move_mem(pEntry_SecConfig->Handshake.SAddr, wdev->if_addr, MAC_ADDR_LEN);
				}

				os_zero_mem(pEntry_SecConfig->Handshake.ReplayCounter, LEN_KEY_DESC_REPLAY);

				for (rsne_idx = 0; rsne_idx < SEC_RSNIE_NUM; rsne_idx++) {
					pEntry_SecConfig->RSNE_Type[rsne_idx] = pProfile_SecConfig->RSNE_Type[rsne_idx];

					if (pEntry_SecConfig->RSNE_Type[rsne_idx] == SEC_RSNIE_NONE)
						continue;

					os_move_mem(pEntry_SecConfig->RSNE_EID[rsne_idx], pProfile_SecConfig->RSNE_EID[rsne_idx],  sizeof(UCHAR));
					pEntry_SecConfig->RSNE_Len[rsne_idx] = pProfile_SecConfig->RSNE_Len[rsne_idx];
					os_move_mem(pEntry_SecConfig->RSNE_Content[rsne_idx], pProfile_SecConfig->RSNE_Content[rsne_idx],  sizeof(UCHAR)*MAX_LEN_OF_RSNIE);
				}

				pMacEntry->SecConfig.Handshake.WpaState = AS_INITPSK;
			}

			pEntry_SecConfig->GroupKeyId = pProfile_SecConfig->GroupKeyId;
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: (apcli%d) connect AKM(0x%x)=%s, PairwiseCipher(0x%x)=%s, GroupCipher(0x%x)=%s\n",
					  __func__, ifIndex,
					  pEntry_SecConfig->AKMMap, GetAuthModeStr(pEntry_SecConfig->AKMMap),
					  pEntry_SecConfig->PairwiseCipher, GetEncryModeStr(pEntry_SecConfig->PairwiseCipher),
					  pEntry_SecConfig->GroupCipher, GetEncryModeStr(pEntry_SecConfig->GroupCipher)));
			MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s(): PairwiseKeyId=%d, GroupKeyId=%d\n",
					  __func__, pEntry_SecConfig->PairwiseKeyId, pEntry_SecConfig->GroupKeyId));
		}
#endif /*(!(defined(APCLI_SAE_SUPPORT)|| defined(APCLI_OWE_SUPPORT)))*/
		if (pMacEntry) {
			UCHAR Rates[MAX_LEN_OF_SUPPORTED_RATES];
			PUCHAR pRates = Rates;
			UCHAR RatesLen;
			UCHAR MaxSupportedRate = 0;

			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
			pMacEntry->Sst = SST_ASSOC;
#ifdef HTC_DECRYPT_IOT

			if ((pMacEntry->HTC_ICVErrCnt)
				|| (pMacEntry->HTC_AAD_OM_Force)
				|| (pMacEntry->HTC_AAD_OM_CountDown)
				|| (pMacEntry->HTC_AAD_OM_Freeze)
			   ) {
				MTWF_LOG(DBG_CAT_RX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("@@@ %s(): (wcid=%u), HTC_ICVErrCnt(%u), HTC_AAD_OM_Freeze(%u), HTC_AAD_OM_CountDown(%u),  HTC_AAD_OM_Freeze(%u) is in Asso. stage!\n",
						 __func__, pMacEntry->wcid, pMacEntry->HTC_ICVErrCnt, pMacEntry->HTC_AAD_OM_Force, pMacEntry->HTC_AAD_OM_CountDown, pMacEntry->HTC_AAD_OM_Freeze));
				/* Force clean. */
				pMacEntry->HTC_ICVErrCnt = 0;
				pMacEntry->HTC_AAD_OM_Force = 0;
				pMacEntry->HTC_AAD_OM_CountDown = 0;
				pMacEntry->HTC_AAD_OM_Freeze = 0;
			}

#endif /* HTC_DECRYPT_IOT */
			/* pMacEntry->wdev = &pApCliEntry->wdev;//duplicate assign. */
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx != 0xFF) {
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
				pReptEntry->MacTabWCID = pMacEntry->wcid;
				pReptEntry->CliValid = TRUE;
				pMacEntry->bReptCli = TRUE;
				pMacEntry->bReptEthBridgeCli = FALSE;
				pMacEntry->MatchReptCliIdx = CliIdx;
				pMacEntry->ReptCliIdleCount = 0;
				COPY_MAC_ADDR(pMacEntry->ReptCliAddr, pReptEntry->CurrentAddress);
				tr_entry->OmacIdx = HcGetRepeaterOmac(pAd, pMacEntry);

				if (pReptEntry->bEthCli == TRUE) {
					pMacEntry->bReptEthCli = TRUE;

					if (MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, pAd->ApCfg.pRepeaterCliPool[CliIdx].OriginalAddress))
						pMacEntry->bReptEthBridgeCli = TRUE;
				} else
					pMacEntry->bReptEthCli = FALSE;
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pApCliEntry->Valid = TRUE;
				pApCliEntry->MacTabWCID = pMacEntry->wcid;
#ifdef MAC_REPEATER_SUPPORT
				pMacEntry->bReptCli = FALSE;
#endif /* MAC_REPEATER_SUPPORT */
				COPY_MAC_ADDR(&wdev->bssid[0], &pApCliEntry->MlmeAux.Bssid[0]);
				os_move_mem(wdev->bss_info_argument.Bssid, wdev->bssid, MAC_ADDR_LEN);
				COPY_MAC_ADDR(APCLI_ROOT_BSSID_GET(pAd, pApCliEntry->MacTabWCID), pApCliEntry->MlmeAux.Bssid);
				pApCliEntry->SsidLen = pApCliEntry->MlmeAux.SsidLen;
				NdisMoveMemory(pApCliEntry->Ssid, pApCliEntry->MlmeAux.Ssid, pApCliEntry->SsidLen);
				ComposePsPoll(pAd, &(pApCliEntry->PsPollFrame), pApCliEntry->MlmeAux.Aid,
							  pApCliEntry->MlmeAux.Bssid, pApCliEntry->wdev.if_addr);
				ComposeNullFrame(pAd, &(pApCliEntry->NullFrame), pApCliEntry->MlmeAux.Bssid,
								 pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid);
			}


			if (IS_AKM_WPA_CAPABILITY_Entry(pMacEntry)
#ifdef WSC_AP_SUPPORT
				&& ((pApCliEntry->wdev.WscControl.WscConfMode == WSC_DISABLE) ||
					(pApCliEntry->wdev.WscControl.bWscTrigger == FALSE))
#endif /* WSC_AP_SUPPORT */
			   )
				tr_entry->PortSecured = WPA_802_1X_PORT_NOT_SECURED;
			else {
					tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;

#ifdef MAC_REPEATER_SUPPORT

				if (CliIdx != 0xFF)
					pReptEntry->CliConnectState = REPT_ENTRY_CONNTED;

#endif /* MAC_REPEATER_SUPPORT */
			}

#ifdef APCLI_AUTO_CONNECT_SUPPORT

			if ((pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] == TRUE) &&
#ifdef MAC_REPEATER_SUPPORT
				(CliIdx == 0xFF) &&
#endif /* MAC_REPEATER_SUPPORT */
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCli auto connected: ApCliLinkUp()\n"));
				pAd->ApCfg.ApCliAutoConnectRunning[apcli_ifIndex] = FALSE;
			}

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
#ifdef MAC_REPEATER_SUPPORT

			if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
				NdisGetSystemUpTime(&pApCliEntry->ApCliLinkUpTime);

			SupportRate(pApCliEntry->MlmeAux.SupRate, pApCliEntry->MlmeAux.SupRateLen, pApCliEntry->MlmeAux.ExtRate,
						pApCliEntry->MlmeAux.ExtRateLen, &pRates, &RatesLen, &MaxSupportedRate);
			pMacEntry->MaxSupportedRate = min(wdev->rate.MaxTxRate, MaxSupportedRate);
			pMacEntry->RateLen = RatesLen;
			MacTableSetEntryPhyCfg(pAd, pMacEntry);
			pMacEntry->CapabilityInfo = pApCliEntry->MlmeAux.CapabilityInfo;
			pApCliEntry->ApCliBeaconPeriod = pApCliEntry->MlmeAux.BeaconPeriod;
#ifdef DOT11_N_SUPPORT

			/* If this Entry supports 802.11n, upgrade to HT rate. */
			if (pApCliEntry->MlmeAux.HtCapabilityLen != 0) {
				PHT_CAPABILITY_IE pHtCapability = (PHT_CAPABILITY_IE)&pApCliEntry->MlmeAux.HtCapability;

				ht_mode_adjust(pAd, pMacEntry, pHtCapability);
				/* find max fixed rate */
				pMacEntry->MaxHTPhyMode.field.MCS = get_ht_max_mcs(pAd, &wdev->DesiredHtPhyInfo.MCSSet[0],
													&pHtCapability->MCSSet[0]);

				if (wdev->DesiredTransmitSetting.field.MCS != MCS_AUTO) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("IF-apcli%d : Desired MCS = %d\n",
							 ifIndex, wdev->DesiredTransmitSetting.field.MCS));
					set_ht_fixed_mcs(pAd, pMacEntry, wdev->DesiredTransmitSetting.field.MCS, wdev->HTPhyMode.field.MCS);
				}

				pMacEntry->HTPhyMode.word = pMacEntry->MaxHTPhyMode.word;
				set_sta_ht_cap(pAd, pMacEntry, pHtCapability);
				NdisMoveMemory(&pMacEntry->HTCapability, &pApCliEntry->MlmeAux.HtCapability, sizeof(HT_CAPABILITY_IE));
				NdisMoveMemory(pMacEntry->HTCapability.MCSSet, pApCliEntry->RxMcsSet, 16);
				assoc_ht_info_debugshow(pAd, pMacEntry, sizeof(HT_CAPABILITY_IE), &pApCliEntry->MlmeAux.HtCapability);
#ifdef APCLI_CERT_SUPPORT

				if (pAd->bApCliCertTest == TRUE) {
					ADD_HTINFO2 *ht_info2 = &pApCliEntry->MlmeAux.AddHtInfo.AddHtInfo2;

					wdev->protection = 0;

					wdev->protection |= SET_PROTECT(ht_info2->OperaionMode);

					if (ht_info2->NonGfPresent == 1)
						wdev->protection |= SET_PROTECT(GREEN_FIELD_PROTECT);
					else
						wdev->protection &= ~(SET_PROTECT(GREEN_FIELD_PROTECT));

					AsicUpdateProtect(pAd);
					MTWF_LOG(DBG_CAT_CLIENT, DBG_SUBCAT_ALL, DBG_LVL_WARN,
							 ("SYNC - Root AP changed N OperaionMode to %d\n", ht_info2->OperaionMode));
				}

#endif /* APCLI_CERT_SUPPORT */
			} else {
				pAd->MacTab.fAnyStationIsLegacy = TRUE;
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliLinkUp - MaxSupRate=%d Mbps\n",
						 RateIdToMbps[pMacEntry->MaxSupportedRate]));
			}

#endif /* DOT11_N_SUPPORT */
#ifdef DOT11_VHT_AC

			if (WMODE_CAP_AC(wdev->PhyMode) && pApCliEntry->MlmeAux.vht_cap_len &&  pApCliEntry->MlmeAux.vht_op_len) {
				vht_mode_adjust(pAd, pMacEntry, &(pApCliEntry->MlmeAux.vht_cap), &(pApCliEntry->MlmeAux.vht_op));
				dot11_vht_mcs_to_internal_mcs(pAd, wdev, &pApCliEntry->MlmeAux.vht_cap, &pMacEntry->MaxHTPhyMode);
				/* Move to set_vht_cap(), which could be reused by AP, WDS and P2P */
				/*
						pMacEntry->MaxRAmpduFactor =
							(pApCliEntry->MlmeAux.vht_cap.vht_cap.max_ampdu_exp > pMacEntry->MaxRAmpduFactor) ?
							pApCliEntry->MlmeAux.vht_cap.vht_cap.max_ampdu_exp : pMacEntry->MaxRAmpduFactor;
						pMacEntry->AMsduSize = pApCliEntry->MlmeAux.vht_cap.vht_cap.max_mpdu_len;
				*/
				set_vht_cap(pAd, pMacEntry, &(pApCliEntry->MlmeAux.vht_cap));
#ifdef APCLI_CERT_SUPPORT
/*	For TGac Test case 5.2.47 AMSDU+AMPDU aggregation  is to large for WFA sniffer to handle
during 3x3 throughput check,so we limit the AMSDU len to 3839*
*/
				if (pAd->bApCliCertTest == TRUE) {

					UCHAR Ssid[] = "VHT-5.2.47-AP3";
					UCHAR SsidEqual = 0;

					SsidEqual = SSID_EQUAL(pApCliEntry->CfgSsid, pApCliEntry->CfgSsidLen, Ssid, strlen(Ssid));

					if (SsidEqual && (pMacEntry->AMsduSize > 0)) {
						pMacEntry->AMsduSize = 0;
						pMacEntry->amsdu_limit_len = vht_max_mpdu_size[pMacEntry->AMsduSize];
						pMacEntry->amsdu_limit_len_adjust = pMacEntry->amsdu_limit_len;
					}
				}
#endif
				NdisMoveMemory(&pMacEntry->vht_cap_ie, &pApCliEntry->MlmeAux.vht_cap, sizeof(VHT_CAP_IE));
				assoc_vht_info_debugshow(pAd, pMacEntry, &pApCliEntry->MlmeAux.vht_cap, &pApCliEntry->MlmeAux.vht_op);
			}

#endif /* DOT11_VHT_AC */
			/* update per wdev bw */
			ext_cha = wlan_config_get_ext_cha(wdev);

			if (pMacEntry->MaxHTPhyMode.field.BW < BW_80)
				wlan_operate_set_ht_bw(wdev, pMacEntry->MaxHTPhyMode.field.BW, ext_cha);
			else {
#ifdef DOT11_VHT_AC

				switch (pMacEntry->MaxHTPhyMode.field.BW) {
				case BW_80:
					wlan_operate_set_vht_bw(wdev, VHT_BW_80);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;

				case BW_160:
					wlan_operate_set_vht_bw(wdev, VHT_BW_160);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;

				default:
					wlan_operate_set_vht_bw(wdev, VHT_BW_2040);
					wlan_operate_set_ht_bw(wdev, HT_BW_40, ext_cha);
					break;
				}

#endif
			}

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
#ifdef MT_MAC
#ifdef MT7615

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				if (pApCliEntry->MlmeAux.APEdcaParm.bValid) {
					pMacEntry->bACMBit[WMM_AC_BK] = pApCliEntry->MlmeAux.APEdcaParm.bACM[WMM_AC_BK];
					pMacEntry->bACMBit[WMM_AC_BE] = pApCliEntry->MlmeAux.APEdcaParm.bACM[WMM_AC_BE];
					pMacEntry->bACMBit[WMM_AC_VI] = pApCliEntry->MlmeAux.APEdcaParm.bACM[WMM_AC_VI];
					pMacEntry->bACMBit[WMM_AC_VO] = pApCliEntry->MlmeAux.APEdcaParm.bACM[WMM_AC_VO];
				}
			}

#endif /* MT7615 */
#endif /* MT_MAC */

			/*
				set this entry WMM capable or not
				It need to before linkup, or it cannot link non_wmm AP
			*/
			if ((pApCliEntry->MlmeAux.APEdcaParm.bValid)
#ifdef DOT11_N_SUPPORT
				|| IS_HT_STA(pMacEntry)
#endif /* DOT11_N_SUPPORT */
			   )
				CLIENT_STATUS_SET_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);
			else
				CLIENT_STATUS_CLEAR_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE);

			NdisGetSystemUpTime(&pApCliEntry->ApCliRcvBeaconTime);

			/* set the apcli interface be valid. */
			/*only apcli should linkup*/
			if (CliIdx == 0xFF) {
				if (wdev_do_linkup(wdev, pMacEntry) != TRUE)
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): linkup fail!!\n", __func__));
			}

			if (wdev_do_conn_act(wdev, pMacEntry) != TRUE)
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): connect fail!!\n", __func__));

			MacTableSetEntryRaCap(pAd, pMacEntry, &pApCliEntry->MlmeAux.vendor_ie);
#ifdef MT_MAC

			if (IS_HIF_TYPE(pAd, HIF_MT)) {
				if (wdev->bAutoTxRateSwitch == TRUE)
					pMacEntry->bAutoTxRateSwitch = TRUE;
				else {
					pMacEntry->HTPhyMode.field.MCS = wdev->HTPhyMode.field.MCS;
					pMacEntry->bAutoTxRateSwitch = FALSE;
					/* If the legacy mode is set, overwrite the transmit setting of this entry. */
					RTMPUpdateLegacyTxSetting((UCHAR)wdev->DesiredTransmitSetting.field.FixedTxMode, pMacEntry);
				}

				RAInit(pAd, pMacEntry);
#ifdef TXBF_SUPPORT

				if (HcIsBfCapSupport(wdev))
					HW_APCLI_BF_CAP_CONFIG(pAd, pMacEntry);

#endif /* TXBF_SUPPORT */
			}

#endif

			if (IS_CIPHER_WEP_Entry(pMacEntry)
			) {
				INT BssIdx;
				ASIC_SEC_INFO Info = {0};
				struct _SECURITY_CONFIG *pSecConfig = &pMacEntry->SecConfig;

				BssIdx = pAd->ApCfg.BssidNum + MAX_MESH_NUM + ifIndex;
#ifdef MAC_APCLI_SUPPORT
				BssIdx = APCLI_BSS_BASE + ifIndex;
#endif /* MAC_APCLI_SUPPORT */

				/* Set Group key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_GROUP_KEY;
				Info.Direction = SEC_ASIC_KEY_RX;
				Info.Wcid = wdev->bss_info_argument.ucBcMcWlanIdx;
				Info.BssIndex = BssIdx;
				Info.Cipher = pSecConfig->GroupCipher;
				Info.KeyIdx = pSecConfig->GroupKeyId;
				os_move_mem(&Info.PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));

				HW_ADDREMOVE_KEYTABLE(pAd, &Info);


				/* Set Pairwise key material to Asic */
				os_zero_mem(&Info, sizeof(ASIC_SEC_INFO));
				Info.Operation = SEC_ASIC_ADD_PAIRWISE_KEY;
				Info.Direction = SEC_ASIC_KEY_BOTH;
				Info.Wcid = pMacEntry->wcid;
				Info.BssIndex = BssIdx;
				Info.Cipher = pSecConfig->PairwiseCipher;
				Info.KeyIdx = pSecConfig->PairwiseKeyId;
				os_move_mem(&Info.PeerAddr[0], pMacEntry->Addr, MAC_ADDR_LEN);
				os_move_mem(&Info.Key, &pSecConfig->WepKey[Info.KeyIdx], sizeof(SEC_KEY_INFO));

				HW_ADDREMOVE_KEYTABLE(pAd, &Info);
			}



#ifdef PIGGYBACK_SUPPORT

			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_PIGGYBACK_CAPABLE)) {
				AsicSetPiggyBack(pAd, TRUE);
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Turn on Piggy-Back\n"));
			}

#endif /* PIGGYBACK_SUPPORT */


#ifdef A4_CONN
	if ((CliIdx == 0xff) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
	}
#endif /* A4_CONN */

#ifdef MT_MAC
#ifdef MAC_REPEATER_SUPPORT

			if ((CliIdx != 0xff && IS_HIF_TYPE(pAd, HIF_MT))) {
				AsicInsertRepeaterRootEntry(
					pAd,
					pMacEntry->wcid,
					(PUCHAR)(pApCliEntry->MlmeAux.Bssid),
					CliIdx);
#ifdef TXBF_SUPPORT

				if ((pAd->fgClonedStaWithBfeeSelected) && (pAd->ReptClonedStaEntry_CliIdx == CliIdx))
					HW_APCLI_BF_REPEATER_CONFIG(pAd, pMacEntry); /* Move cloned STA's MAC addr from MUAR table to ownmac */

#endif /* TXBF_SUPPORT */
			}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* MT_MAC */
			result = TRUE;
			pAd->ApCfg.ApCliInfRunned++;
#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT
			if (CliIdx == 0xFF)
#endif
			wf_apcli_active_links++;

#ifdef CONFIG_FAST_NAT_SUPPORT

			if ((wf_apcli_active_links >= 2)
				&& (ra_sw_nat_hook_rx != NULL)
				&& (wf_ra_sw_nat_hook_rx_bkup == NULL)) {
				wf_ra_sw_nat_hook_rx_bkup =	ra_sw_nat_hook_rx;
				ra_sw_nat_hook_rx = NULL;
			}

#endif /*CONFIG_FAST_NAT_SUPPORT*/
#endif
			break;
		}

		result = FALSE;
	} while (FALSE);

	if (result == FALSE) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, (" (%s) alloc mac entry fail!!!\n", __func__));
		return result;
	}

#ifdef WSC_AP_SUPPORT

	/* WSC initial connect to AP, jump to Wsc start action and set the correct parameters */
	if ((result == TRUE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscConfMode == WSC_ENROLLEE) &&
		(pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.bWscTrigger == TRUE)) {
		pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscState = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscStatus = WSC_STATE_LINK_UP;
		pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl.WscConfStatus = WSC_SCSTATE_UNCONFIGURED;
		NdisZeroMemory(pApCliEntry->wdev.WscControl.EntryAddr, MAC_ADDR_LEN);
		NdisMoveMemory(pApCliEntry->wdev.WscControl.EntryAddr, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN);
		WscSendEapolStart(pAd,
						  pMacEntry->Addr,
						  AP_MODE,
						  &pAd->ApCfg.ApCliTab[ifIndex].wdev);
	} else
		WscStop(pAd, TRUE, &pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl);

#endif /* WSC_AP_SUPPORT */
#ifdef DOT11_N_SUPPORT
#ifdef DOT11N_DRAFT3
#ifdef APCLI_CERT_SUPPORT

	if (pAd->bApCliCertTest == TRUE) {
		if ((pAd->CommonCfg.bBssCoexEnable == TRUE) &&
			(wdev->channel <= 14) &&
			(wdev->DesiredHtPhyInfo.bHtEnable == TRUE) &&
			(pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport == 1)) {
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_SCAN_2040);
			BuildEffectedChannelList(pAd, wdev);
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("LinkUP AP supports 20/40 BSS COEX !!! Dot11BssWidthTriggerScanInt[%d]\n",
					 pAd->CommonCfg.Dot11BssWidthTriggerScanInt));
		} else {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("not supports 20/40 BSS COEX !!!\n"));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("pAd->CommonCfg.bBssCoexEnable %d !!!\n",
					 pAd->CommonCfg.bBssCoexEnable));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Channel %d !!!\n",
					 wdev->channel));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("pApCliEntry->DesiredHtPhyInfo.bHtEnable %d !!!\n",
					 pApCliEntry->wdev.DesiredHtPhyInfo.bHtEnable));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("pAd->MlmeAux.ExtCapInfo.BssCoexstSup %d !!!\n",
					 pApCliEntry->MlmeAux.ExtCapInfo.BssCoexistMgmtSupport));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("CentralChannel: %d !!!\n",
					 wlan_operate_get_cen_ch_1(wdev)));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("PhyMode %d !!!\n",
					 wdev->PhyMode));
		}
	}

#endif /* APCLI_CERT_SUPPORT */
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

#if defined(CONFIG_MAP_SUPPORT) && defined(WAPP_SUPPORT)
	/*For security NONE & WEP case*/
	if ((CliIdx == 0xFF) && (tr_entry->PortSecured == WPA_802_1X_PORT_SECURED)) {
		wapp_send_apcli_association_change(WAPP_APCLI_ASSOCIATED, pAd, pApCliEntry);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
					("APCLIENT MAP_ENABLE (No Security)\n"));
#ifdef A4_CONN
		map_a4_peer_enable(pAd, pMacEntry, FALSE);
#endif
		if (pApCliEntry->wdev.WscControl.bWscTrigger == FALSE)
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

static CHAR LinkDownReason[APCLI_LINKDOWN_PEER_DEASSOC_RSP+1][30] = {
	"none",
	"send deauth req",
	"send deassociate req",
	"recv deassociate req",
	"got disconnect req",
	"recv deassociate resp"
};

static CHAR SubLinkDownReason[APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG+1][30] = {
	"none",
	"rept connect too long",
	"idle too long",
	"remove sta",
	"apcli if down",
	"beacon miss",
	"recv sta disassociate req",
	"recv sta deauth req",
	"manually del mac entry",
	"repter bind to other apcli",
	"apcli connect too long"
};
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
	struct wifi_dev *wdev = NULL;
	UCHAR CliIdx = 0xFF;
#ifdef MAC_REPEATER_SUPPORT
	struct _REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	UCHAR MacTabWCID = 0;

	if ((ifIndex < MAX_APCLI_NUM)
#ifdef MAC_REPEATER_SUPPORT
		|| (ifIndex >= REPT_MLME_START_IDX)
#endif /* MAC_REPEATER_SUPPORT */
	   ) {
#ifdef MAC_REPEATER_SUPPORT

		if (ifIndex >= REPT_MLME_START_IDX) {
			CliIdx = ifIndex - REPT_MLME_START_IDX;
			pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];
			ifIndex = pReptEntry->wdev->func_idx;

			if (pReptEntry->LinkDownReason == APCLI_LINKDOWN_PEER_DEASSOC_REQ) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						 ("!!! REPEATER CLI LINK DOWN - IF(apcli%d) Cli %d (Reason=%s,Reason code=%lu)!!!\n",
						  ifIndex, CliIdx, LinkDownReason[pReptEntry->LinkDownReason], pReptEntry->Disconnect_Sub_Reason));
			} else {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
						 ("!!! REPEATER CLI LINK DOWN - IF(apcli%d) Cli %d (Reason=%s,sub=%s)!!!\n",
						  ifIndex, CliIdx, LinkDownReason[pReptEntry->LinkDownReason], SubLinkDownReason[pReptEntry->Disconnect_Sub_Reason]));
			}
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
			if (pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason == APCLI_LINKDOWN_PEER_DEASSOC_REQ)
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("!!! APCLI LINK DOWN - IF(apcli%d) (Reason=%s,Reason code=%lu)!!!\n", ifIndex, LinkDownReason[pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason], pAd->ApCfg.ApCliTab[ifIndex].Disconnect_Sub_Reason));
			else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("!!! APCLI LINK DOWN - IF(apcli%d) (Reason=%s,sub=%s)!!!\n", ifIndex, LinkDownReason[pAd->ApCfg.ApCliTab[ifIndex].LinkDownReason], SubLinkDownReason[pAd->ApCfg.ApCliTab[ifIndex].Disconnect_Sub_Reason]));
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("!!! ERROR : APCLI LINK DOWN - IF(apcli%d)!!!\n", ifIndex));
		return;
	}

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;
#ifdef CONVERTER_MODE_SWITCH_SUPPORT
	if (pApCliEntry->ApCliMode == APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION) {
#ifdef WAPP_SUPPORT
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s() :APCLI LinkDown Event send to wapp\n", __func__));
			wapp_send_apcli_association_change_vendor10(WAPP_APCLI_DISASSOCIATED, pAd, pApCliEntry);
#else
			V10ConverterModeStartStop(pAd, FALSE);
#endif
	}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

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
	RTMP_OS_NETDEV_CARRIER_OFF(wdev->if_dev);
#endif

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
	{
		if (wf_drv_tbl.wf_fwd_entry_delete_hook)
			wf_drv_tbl.wf_fwd_entry_delete_hook(pApCliEntry->wdev.if_dev, pAd->net_dev, 1);

		if (wf_drv_tbl.wf_fwd_check_device_hook)
			wf_drv_tbl.wf_fwd_check_device_hook(pApCliEntry->wdev.if_dev,
							INT_APCLI, CliIdx, pApCliEntry->wdev.channel, 0);
	}

#ifdef MAC_REPEATER_SUPPORT
	else {
		if (wf_drv_tbl.packet_source_delete_entry_hook)
			wf_drv_tbl.packet_source_delete_entry_hook(100);
	}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		pAd->ApCfg.ApCliInfRunned--;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
#ifdef MAC_REPEATER_SUPPORT
	if (CliIdx == 0xFF)
#endif /* MAC_REPEATER_SUPPORT */
		wf_apcli_active_links--;

#ifdef CONFIG_FAST_NAT_SUPPORT
	if ((wf_apcli_active_links < 2) && (ra_sw_nat_hook_rx == NULL) && (wf_ra_sw_nat_hook_rx_bkup != NULL)) {
		ra_sw_nat_hook_rx = wf_ra_sw_nat_hook_rx_bkup;
		wf_ra_sw_nat_hook_rx_bkup = NULL;
	}

#endif /*CONFIG_FAST_NAT_SUPPORT*/
#endif


#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF) {
		MacTabWCID = pReptEntry->MacTabWCID;
		MacTableDeleteEntry(pAd, MacTabWCID, pAd->MacTab.Content[MacTabWCID].Addr);
#ifdef FAST_EAPOL_WAR
		pReptEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
		MacTabWCID = pApCliEntry->MacTabWCID;
		MacTableDeleteEntry(pAd, MacTabWCID, APCLI_ROOT_BSSID_GET(pAd, MacTabWCID));
#ifdef FAST_EAPOL_WAR
		pApCliEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
	}

#ifdef MAC_REPEATER_SUPPORT

	if (CliIdx != 0xFF)
		HW_REMOVE_REPT_ENTRY(pAd, ifIndex, CliIdx);
	else
#endif /* MAC_REPEATER_SUPPORT */
	{
		pApCliEntry->Valid = FALSE;	/* This link doesn't associated with any remote-AP */
		MSDU_FORBID_SET(&pApCliEntry->wdev, MSDU_FORBID_CONNECTION_NOT_READY);
		pApCliEntry->wdev.PortSecured = WPA_802_1X_PORT_NOT_SECURED;
#ifdef DOT11W_PMF_SUPPORT
		BssTableDeleteEntry(&pAd->ScanTab, pApCliEntry->MlmeAux.Bssid, wdev->channel);
#else
#endif /* DOT11W_PMF_SUPPORT */
	}

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	map_a4_peer_disable(pAd, &pAd->MacTab.Content[MacTabWCID], FALSE);
#endif

	pAd->ApCfg.ApCliTab[ifIndex].bPeerExist = FALSE;

	/*TODO & FIXME: Carter, REPEATER CASE */
	if (CliIdx == 0xFF) {
		if (wdev_do_linkdown(wdev) != TRUE)
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("(%s) linkdown fail!\n", __func__));
	}

#ifdef TXBF_SUPPORT

	if ((pAd->fgClonedStaWithBfeeSelected) && (pAd->ReptClonedStaEntry_CliIdx == CliIdx)) {
		pAd->fgClonedStaWithBfeeSelected = FALSE;
		/* Remove cloned STA's MAC addr from ownmac */
	}

#endif /* TXBF_SUPPORT */
#if defined(RT_CFG80211_P2P_CONCURRENT_DEVICE) || defined(CFG80211_MULTI_STA)
	RT_CFG80211_LOST_GO_INFORM(pAd);
	/* TODO: need to consider driver without no FW offload @20140728 */
	/* NoA Stop */
	/* if (bP2pCliPmEnable) */
	CmdP2pNoaOffloadCtrl(pAd, P2P_NOA_DISABLED);
#endif /* RT_CFG80211_P2P_CONCURRENT_DEVICE || CFG80211_MULTI_STA */

#ifdef APCLI_CFG80211_SUPPORT
	RT_CFG80211_LOST_AP_INFORM(pAd);
#endif /* APCLI_CFG80211_SUPPORT */

	/*for APCLI linkdown*/
	if (CliIdx == 0xFF) {
#ifdef DOT11_N_SUPPORT
		wlan_operate_set_ht_bw(&pApCliEntry->wdev,
							   wlan_config_get_ht_bw(&pApCliEntry->wdev),
							   wlan_config_get_ext_cha(&pApCliEntry->wdev));
#endif
#ifdef DOT11_VHT_AC
		wlan_operate_set_vht_bw(&pApCliEntry->wdev, wlan_config_get_vht_bw(&pApCliEntry->wdev));
#endif
		ApCliLinkDownComplete(pApCliEntry);
	}
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
	struct DOT11_H *pDot11h = NULL;

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS|fRTMP_ADAPTER_RADIO_OFF) ||
		(!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP)))
		return;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApCliInit != TRUE)
			continue;

#ifdef APCLI_CONNECTION_TRIAL
		pCurrState = &pAd->ApCfg.ApCliTab[ifIndex].CtrlCurrState;
#endif /* APCLI_CONNECTION_TRIAL */

		if (!HcIsRadioAcq(&pApCliEntry->wdev))
			continue;

		if (APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (pApCliEntry->Enable == TRUE)
			&& (pApCliEntry->Valid == FALSE)
#ifdef APCLI_CONNECTION_TRIAL
			&& (ifIndex != (pAd->ApCfg.ApCliNum-1)) /* last IF is for apcli connection trial */
#endif /* APCLI_CONNECTION_TRIAL */
		   ) {
			pDot11h = pApCliEntry->wdev.pDot11_H;
			if (pDot11h == NULL)
				return;
			if (IS_DOT11_H_RADAR_STATE(pAd, RD_SILENCE_MODE, pApCliEntry->wdev.channel, pDot11h)) {
				if (pApCliEntry->bPeerExist == TRUE) {
					/* Got peer's beacon; change to normal mode */
					pDot11h->RDCount = pDot11h->ChMovingTime;
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							 ("ApCliIfUp - PeerExist\n"));
				} else
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							 ("ApCliIfUp - Stop probing while Radar state is silent\n"));

				continue;
			}

			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("(%s) ApCli interface[%d] startup.\n", __func__, ifIndex));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_JOIN_REQ, 0, NULL, ifIndex);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->bPeerExist = FALSE;
		}

#ifdef APCLI_CONNECTION_TRIAL
		else if (
			APCLI_IF_UP_CHECK(pAd, ifIndex)
			&& (*pCurrState == APCLI_CTRL_DISCONNECTED)/* Apcli1 is not connected state. */
			&& (pApCliEntry->TrialCh != 0)
			/* && NdisCmpMemory(pApCliEntry->ApCliMlmeAux.Ssid, pApCliEntry->CfgSsid, pApCliEntry->SsidLen) != 0 */
			&& (pApCliEntry->CfgSsidLen != 0)
			&& (pApCliEntry->Enable != 0)
			/* new ap ssid shall different from the origin one. */
		) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Enqueue APCLI_CTRL_TRIAL_CONNECT\n", __func__));
			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_TRIAL_CONNECT, 0, NULL, ifIndex);
			/* Reset bPeerExist each time in case we could keep old status */
			pApCliEntry->bPeerExist = FALSE;
		}

#endif /* APCLI_CONNECTION_TRIAL */
	}
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
	UCHAR CliIdx, idx;
	INVAILD_TRIGGER_MAC_ENTRY *pEntry = NULL;
	struct _REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():ApCli interface[%d] start down.\n", __func__, ifIndex));

		if (pApCliEntry->Enable == TRUE)
			continue;

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)

		if (wf_drv_tbl.wf_fwd_entry_delete_hook)
			wf_drv_tbl.wf_fwd_entry_delete_hook(pApCliEntry->wdev.if_dev, pAd->net_dev, 1);

#endif /* CONFIG_WIFI_PKT_FWD */
#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn) {
			for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

				/*disconnect the ReptEntry which is bind on the CliLink*/
				if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &pApCliEntry->wdev)) {
					RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
					pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
					MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
								APCLI_CTRL_DISCONNECT_REQ,
								0,
								NULL,
								(REPT_MLME_START_IDX + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					ReptWaitLinkDown(pReptEntry);
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
		RTMP_OS_INIT_COMPLETION(&pApCliEntry->linkdown_complete);
		pApCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_IF_DOWN;
		MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, ifIndex);
		RTMP_MLME_HANDLER(pAd);
		ApCliWaitLinkDown(pApCliEntry);
	}

#ifdef MAC_REPEATER_SUPPORT

	for (idx = 0; idx < MAX_IGNORE_AS_REPEATER_ENTRY_NUM; idx++) {
		pEntry = &pAd->ApCfg.ReptControl.IgnoreAsRepeaterEntry[idx];

		if (pAd->ApCfg.ApCliInfRunned == 0)
			RepeaterRemoveIngoreEntry(pAd, idx, pEntry->MacAddr);
	}

#endif /* MAC_REPEATER_SUPPORT */
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
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* Reset is in progress, stop immediately */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS) ||
		!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	if (ScanRunning(pAd) == TRUE)
		return;

	for (index = 0; index < MAX_APCLI_NUM; index++) {
		UCHAR Wcid;
		PMAC_TABLE_ENTRY pMacEntry;
		STA_TR_ENTRY *tr_entry;
		BOOLEAN bForceBrocken = FALSE;
		BOOLEAN bWpa_4way_too_log = FALSE;
		BOOLEAN bBeacon_miss = FALSE;
#ifdef APCLI_CONNECTION_TRIAL

		if (index == (pAd->ApCfg.ApCliNum-1))/* last IF is for apcli connection trial */
			continue;/* skip apcli1 monitor. FIXME:Carter shall find a better way. */

#endif /* APCLI_CONNECTION_TRIAL */
		pApCliEntry = &pAd->ApCfg.ApCliTab[index];

		/* sanity check whether the interface is initialized. */
		if (pApCliEntry->ApCliInit != TRUE)
			continue;

#ifdef MAC_REPEATER_SUPPORT
		RepeaterLinkMonitor(pAd);
#endif /* MAC_REPEATER_SUPPORT */

		if (pApCliEntry->Valid == TRUE) {
			BOOLEAN ApclibQosNull = FALSE;

			Wcid = pAd->ApCfg.ApCliTab[index].MacTabWCID;

			if (!VALID_UCAST_ENTRY_WCID(pAd, Wcid))
				continue;

			pMacEntry = &pAd->MacTab.Content[Wcid];
			tr_entry = &pAd->MacTab.tr_entry[Wcid];

			if ((IS_AKM_WPA_CAPABILITY(pMacEntry->SecConfig.AKMMap))
				&& (tr_entry->PortSecured != WPA_802_1X_PORT_SECURED)
				&& (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApCliLinkUpTime + (30 * OS_HZ))))) {
				bWpa_4way_too_log = TRUE;
				bForceBrocken = TRUE;
			}

			{
#ifdef CONFIG_MULTI_CHANNEL
				/* increase to 12 */
				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApCliRcvBeaconTime + (12 * OS_HZ)))) {
#else
#ifdef MT7615
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

				if (cap->fgRateAdaptFWOffload == TRUE) {
					if ((RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1 * OS_HZ)))
						&& (RTMP_TIME_BEFORE(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (3 * OS_HZ)))) {
						pMacEntry->TxStatRspCnt = 0;
						pMacEntry->TotalTxSuccessCnt = 0;
					}

					if (RTMP_TIME_AFTER(pAd->Mlme.Now32, pApCliEntry->ApCliRcvBeaconTime + (1 * OS_HZ)))
						HW_GET_TX_STATISTIC(pAd, GET_TX_STAT_ENTRY_TX_CNT, pMacEntry->wcid);
				}

#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#endif /* MT7615 */

				if (RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApCliRcvBeaconTime + (6 * OS_HZ)))) {
#endif /* CONFIG_MULTI_CHANNEL */
#ifdef MT7615
#ifdef RACTRL_FW_OFFLOAD_SUPPORT

					if ((cap->fgRateAdaptFWOffload == TRUE) &&
						(pMacEntry->TxStatRspCnt > 1) && (pMacEntry->TotalTxSuccessCnt)) {
#ifdef BEACON_MISS_ON_PRIMARY_CHANNEL
			/*When Root AP changes the primary channel within the same group of bandwidth, APCLI not disconnects from Root AP.
			This happens as the NULL packet transmits in the configured bandwidth only, the transmitted NULL packet is succeeding
			which update TX Success count.
			Example, BW is configured for 80 MHz, Root AP switches primary channel from 36 to 40,
			NULL packet transmits will happen in 80 MHz only*/
						if ((pApCliEntry->wdev.channel > 14) && (pMacEntry->MaxHTPhyMode.field.BW > 0) &&
							(RTMP_TIME_AFTER(pAd->Mlme.Now32, (pApCliEntry->ApCliRcvBeaconTime_MlmeEnqueueForRecv + (6 * OS_HZ))))) {

								bBeacon_miss = TRUE;
								bForceBrocken = TRUE;
						} else
#endif
						pApCliEntry->ApCliRcvBeaconTime = pAd->Mlme.Now32;
					} else
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */
#endif /* MT7615 */
					{
						bBeacon_miss = TRUE;
						bForceBrocken = TRUE;
					}
				}
			}
			if (CLIENT_STATUS_TEST_FLAG(pMacEntry, fCLIENT_STATUS_WMM_CAPABLE))
				ApclibQosNull = TRUE;
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
			if (bWpa_4way_too_log == TRUE) {
				if (IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) || IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) {
					UCHAR pmkid[LEN_PMKID];
					UCHAR pmk[LEN_PMK];
					INT cached_idx;
					UCHAR if_index = pApCliEntry->wdev.func_idx;
					UCHAR cli_idx = 0xFF;

					/* Connection taking too long update PMK cache  and delete sae instance*/
					if (
#ifdef APCLI_SAE_SUPPORT

						(IS_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap) &&
							sae_get_pmk_cache(&pAd->SaeCfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid, pmkid, pmk))
#endif

#ifdef APCLI_OWE_SUPPORT
						|| IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)
#endif
					) {

						cached_idx = apcli_search_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, cli_idx);
						if (cached_idx != INVALID_PMKID_IDX) {
#ifdef APCLI_SAE_SUPPORT
							SAE_INSTANCE *pSaeIns = search_sae_instance(&pAd->SaeCfg, pApCliEntry->wdev.if_addr, pApCliEntry->MlmeAux.Bssid);

							MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
										("Reconnection falied with pmkid ,delete cache entry and sae instance \n"));

							if (pSaeIns != NULL) {
								delete_sae_instance(pSaeIns);
							}
#endif
							apcli_delete_pmkid_cache(pAd, pApCliEntry->MlmeAux.Bssid, if_index, cli_idx);
						}
					}

				}
			}
#endif
			if ((bForceBrocken == FALSE)
#ifdef CONFIG_MULTI_CHANNEL
				&& (pAd->Mlme.bStartMcc == FALSE)
#endif /* CONFIG_MULTI_CHANNEL */
			   )
            {
#if (defined(WH_EZ_SETUP) && defined(RACTRL_FW_OFFLOAD_SUPPORT))
                UCHAR idx, Total;

				if(IS_ADPTR_EZ_SETUP_ENABLED(pAd)){
					if (cap->fgRateAdaptFWOffload == TRUE)
                    	Total = 3;
                	else
                    	Total = 1;

	                for(idx = 0; idx < Total; idx++)
		        		ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PWR_ACTIVE);
				}
				else
#endif
				ApCliRTMPSendNullFrame(pAd, pMacEntry->CurrTxRate, ApclibQosNull, pMacEntry, PWR_ACTIVE);
			}
		} else
			continue;

		if (bForceBrocken == TRUE) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor: IF(apcli%d) - no Beancon is received from root-AP.\n", index));
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliIfMonitor: Reconnect the Root-Ap again.\n"));
#ifdef CONFIG_MULTI_CHANNEL

			if (pAd->Mlme.bStartMcc == TRUE)
				return;

#endif /* CONFIG_MULTI_CHANNEL */

			if (bBeacon_miss) {
				ULONG Now32;

				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliIfMonitor apcli%d time1: %lu\n", index, pApCliEntry->ApCliRcvBeaconTime_MlmeEnqueueForRecv));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliIfMonitor apcli%d time2: %lu\n", index, pApCliEntry->ApCliRcvBeaconTime_MlmeEnqueueForRecv_2));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliIfMonitor apcli%d time3: %lu\n", index, pApCliEntry->ApCliRcvBeaconTime));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliIfMonitor apcli%d OS_HZ: %d\n", index, OS_HZ));
				NdisGetSystemUpTime(&Now32);
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("ApCliIfMonitor apcli%d time now: %lu\n", index, Now32));
			}

			/* MCC TODO: WCID Not Correct when MCC on */
#ifdef MCC_TEST
#else
#ifdef MAC_REPEATER_SUPPORT

			if (pAd->ApCfg.bMACRepeaterEn) {
				APCLI_STRUCT *apcli_entry = pApCliEntry;
				REPEATER_CLIENT_ENTRY *pReptEntry;
				UCHAR CliIdx;

				for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
					pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

					if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &apcli_entry->wdev)) {
						if (bBeacon_miss)
							pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
						else
							pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG;

						MlmeEnqueue(pAd,
									APCLI_CTRL_STATE_MACHINE,
									APCLI_CTRL_DISCONNECT_REQ,
									0,
									NULL,
									(REPT_MLME_START_IDX + CliIdx));
						RTMP_MLME_HANDLER(pAd);
					}
				}
			}

#endif /* MAC_REPEATER_SUPPORT */

			if (bBeacon_miss)
				pApCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_MNT_NO_BEACON;
			else
				pApCliEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_APCLI_TRIGGER_TOO_LONG;

			MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_CTRL_DISCONNECT_REQ, 0, NULL, index);
			RTMP_MLME_HANDLER(pAd);
#endif /* MCC_TEST */
		}
	}
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_LOUD, ("ra offload=%d\n", cap->fgRateAdaptFWOffload));
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
	if (pFrame->Hdr.FC.Type == FC_TYPE_DATA) {
#ifdef WSC_AP_SUPPORT
		/*WSC EAPOL PACKET */
		pEntry = MacTableLookup(pAd, pFrame->Hdr.Addr2);

		if (pEntry && IS_ENTRY_APCLI(pEntry) && pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].wdev.WscControl.WscConfMode == WSC_ENROLLEE) {
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
	} else if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
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
			if (Seq == 2 || Seq == 4) {
				*Machine = APCLI_AUTH_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_AUTH_EVEN;
			} else
				return FALSE;

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
	if (pFrame->Hdr.FC.Type == FC_TYPE_MGMT) {
		switch (pFrame->Hdr.FC.SubType) {
		/* Beacon must be processed by AP Sync state machine. */
		case SUBTYPE_BEACON:
			*Machine = AP_SYNC_STATE_MACHINE;
			*MsgType = APMT2_PEER_BEACON;
			break;

		/* Only Sta have chance to receive Probe-Rsp. */
		case SUBTYPE_PROBE_RSP:
			if (pAd->Mlme.ApSyncMachine.CurrState == AP_SCAN_LISTEN) {
				*Machine = AP_SYNC_STATE_MACHINE;
				*MsgType = APMT2_PEER_PROBE_RSP;
			} else {
				*Machine = APCLI_SYNC_STATE_MACHINE;
				*MsgType = APCLI_MT2_PEER_PROBE_RSP;
			}
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
	OUT ADD_HT_INFO_IE * pAddHtInfo,	/* AP might use this additional ht info IE */
	OUT UCHAR *pHtCapabilityLen,
	OUT UCHAR *pAddHtInfoLen,
	OUT UCHAR *pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT UCHAR *pCkipFlag,
	OUT IE_LISTS * ie_list)
{
	CHAR          IeType, *Ptr;
	PFRAME_802_11 pFrame = (PFRAME_802_11)pMsg;
	PEID_STRUCT   pEid;
	ULONG         Length = 0;
#ifdef CONFIG_MAP_SUPPORT
	unsigned char map_cap;
#endif

	*pNewExtChannelOffset = 0xff;
	*pHtCapabilityLen = 0;
	*pAddHtInfoLen = 0;
	COPY_MAC_ADDR(pAddr2, pFrame->Hdr.Addr2);
	Ptr = (CHAR *) pFrame->Octet;
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

	if ((IeType != IE_SUPP_RATES) || (*pSupRateLen > MAX_LEN_OF_SUPPORTED_RATES)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s(): fail - wrong SupportedRates IE\n", __func__));
		return FALSE;
	}
	NdisMoveMemory(SupRate, &pFrame->Octet[8], *pSupRateLen);

	Length = Length + 2 + *pSupRateLen;
	/* many AP implement proprietary IEs in non-standard order, we'd better */
	/* tolerate mis-ordered IEs to get best compatibility */
	pEid = (PEID_STRUCT) &pFrame->Octet[8 + (*pSupRateLen)];

	/* get variable fields from payload and advance the pointer */
	while ((Length + 2 + pEid->Len) <= MsgLen) {
		switch (pEid->Eid) {
		case IE_EXT_SUPP_RATES:
			if (pEid->Len <= MAX_LEN_OF_SUPPORTED_RATES) {
				NdisMoveMemory(ExtRate, pEid->Octet, pEid->Len);
				*pExtRateLen = pEid->Len;
			}

			break;
#ifdef DOT11_N_SUPPORT

		case IE_HT_CAP:
		case IE_HT_CAP2:
			if (pEid->Len >= SIZE_HT_CAP_IE) { /*Note: allow extension.!! */
				NdisMoveMemory(pHtCapability, pEid->Octet, SIZE_HT_CAP_IE);
				*(USHORT *) (&pHtCapability->HtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->HtCapInfo));
				*(USHORT *) (&pHtCapability->ExtHtCapInfo) = cpu2le16(*(USHORT *)(&pHtCapability->ExtHtCapInfo));
				*(UINT32 *) (&pHtCapability->TxBFCap) = cpu2le32(*(UINT32 *)(&pHtCapability->TxBFCap));
				*pHtCapabilityLen = SIZE_HT_CAP_IE;
			} else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, ("%s():wrong IE_HT_CAP\n", __func__));

			break;

		case IE_ADD_HT:
		case IE_ADD_HT2:
			if (pEid->Len >= sizeof(ADD_HT_INFO_IE)) {
				/* This IE allows extension, but we can ignore extra bytes beyond our knowledge , so only */
				/* copy first sizeof(ADD_HT_INFO_IE) */
				NdisMoveMemory(pAddHtInfo, pEid->Octet, sizeof(ADD_HT_INFO_IE));
				*pAddHtInfoLen = SIZE_ADD_HT_INFO_IE;
			} else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, ("%s():wrong IE_ADD_HT\n", __func__));

			break;

		case IE_SECONDARY_CH_OFFSET:
			if (pEid->Len == 1)
				*pNewExtChannelOffset = pEid->Octet[0];
			else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, ("%s():wrong IE_SECONDARY_CH_OFFSET\n", __func__));

			break;
#ifdef DOT11_VHT_AC

		case IE_VHT_CAP:
			if (pEid->Len == sizeof(VHT_CAP_IE)) {
				NdisMoveMemory(&ie_list->vht_cap, pEid->Octet, sizeof(VHT_CAP_IE));
				ie_list->vht_cap_len = sizeof(VHT_CAP_IE);
			} else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, ("%s():wrong IE_VHT_CAP\n", __func__));

			break;

		case IE_VHT_OP:
			if (pEid->Len == sizeof(VHT_OP_IE)) {
				NdisMoveMemory(&ie_list->vht_op, pEid->Octet, sizeof(VHT_OP_IE));
				ie_list->vht_op_len = sizeof(VHT_OP_IE);
			} else
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_WARN, ("%s():wrong IE_VHT_OP\n", __func__));

			break;
#endif /* DOT11_VHT_AC */
#endif /* DOT11_N_SUPPORT */

		/* CCX2, WMM use the same IE value */
		/* case IE_CCX_V2: */
		case IE_VENDOR_SPECIFIC:
#ifdef CONFIG_MAP_SUPPORT
			if (map_check_cap_ie(pEid, &map_cap) == TRUE)
				ie_list->MAP_AttriValue = map_cap;
#endif /* CONFIG_MAP_SUPPORT */

			/* handle WME PARAMTER ELEMENT */
			if (NdisEqualMemory(pEid->Octet, WME_PARM_ELEM, 6) && (pEid->Len == 24)) {
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

				for (i = 0; i < 4; i++) {
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
		case IE_RSN:
			/* Copy whole RSNIE context */
			NdisMoveMemory(&ie_list->RSN_IE[0], pEid, pEid->Len + 2);
			ie_list->RSNIE_Len = pEid->Len + 2;
			break;

		case IE_WLAN_EXTENSION:
		{
			/*parse EXTENSION EID*/
			UCHAR *extension_id = (UCHAR *)pEid + 2;
			switch (*extension_id) {
			case IE_EXTENSION_ID_ECDH:
#ifdef APCLI_OWE_SUPPORT
			{
				UCHAR *ext_ie_length = (UCHAR *)pEid + 1;
				os_zero_mem(ie_list->ecdh_ie.public_key, *ext_ie_length-3);
				ie_list->ecdh_ie.ext_ie_id = IE_WLAN_EXTENSION;
				ie_list->ecdh_ie.length = pEid->Len;
				NdisMoveMemory(&ie_list->ecdh_ie.ext_id_ecdh, pEid->Octet, pEid->Len);
			}
#endif /*APCLI_OWE_SUPPORT*/
			break;

			default:
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("RESP IE_WLAN_EXTENSION: no handler for extension_id:%d\n", *extension_id));
			break;
			}
		}

		default:
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():ignore unrecognized EID = %d\n", __func__, pEid->Eid));
			break;
		}

		Length = Length + 2 + pEid->Len;
		pEid = (PEID_STRUCT)((UCHAR *)pEid + 2 + pEid->Len);
	}

	return TRUE;
}


MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR *pAddrs)
{
	ULONG ApCliIndex;
	PMAC_TABLE_ENTRY pCurEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;

	if (!VALID_UCAST_ENTRY_WCID(pAd, wcid))
		return NULL;

	NdisAcquireSpinLock(&pAd->MacTabLock);

	do {
		pCurEntry = &pAd->MacTab.Content[wcid];
		ApCliIndex = 0xff;

		if ((pCurEntry) &&
			(IS_ENTRY_APCLI(pCurEntry) || IS_ENTRY_REPEATER(pCurEntry))
		   )
			ApCliIndex = pCurEntry->func_tb_idx;

		if ((ApCliIndex == 0xff) || (ApCliIndex >= MAX_APCLI_NUM))
			break;

		if (pAd->ApCfg.ApCliTab[ApCliIndex].Valid != TRUE)
			break;

		if (MAC_ADDR_EQUAL(pCurEntry->Addr, pAddrs)) {
			pEntry = pCurEntry;
			break;
		}
	} while (FALSE);

	NdisReleaseSpinLock(&pAd->MacTabLock);
	return pEntry;
}


VOID APCLIerr_Action(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR Idx)
{
	HEADER_802_11         DisassocHdr;
	PUCHAR                pOutBuffer = NULL;
	ULONG                 FrameLen = 0;
	NDIS_STATUS           NStatus;
	USHORT                Reason = REASON_CLS3ERR;
	MAC_TABLE_ENTRY       *pEntry = NULL;

	if (VALID_UCAST_ENTRY_WCID(pAd, pRxBlk->wcid))
		pEntry = &(pAd->MacTab.Content[pRxBlk->wcid]);


	if (pEntry) {
		mac_entry_delete(pAd, pEntry);
	}

	/* 2. send out a DISASSOC request frame */
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("APCLI ASSOC - Class 3 Error, Send DISASSOC frame to %02x:%02x:%02x:%02x:%02x:%02x\n",
			  PRINT_MAC(pRxBlk->Addr2)));

	ApCliMgtMacHeaderInit(pAd, &DisassocHdr, SUBTYPE_DISASSOC, 0, pRxBlk->Addr2, pRxBlk->Addr2, Idx);

	MakeOutgoingFrame(pOutBuffer,            &FrameLen,
					  sizeof(HEADER_802_11), &DisassocHdr,
					  2,                     &Reason,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);

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

	do {
		if ((apCliIdx < 0) || (apCliIdx >= MAX_APCLI_NUM)) {
			result = FALSE;
			break;
		}

		pApCliEntry = (APCLI_STRUCT *)&pAd->ApCfg.ApCliTab[apCliIdx];

		if (pApCliEntry->Valid != TRUE) {
			result = FALSE;
			break;
		}

		if (pApCliEntry->Enable != TRUE) {
			result = FALSE;
			break;
		}

		if ((!VALID_UCAST_ENTRY_WCID(pAd, pApCliEntry->MacTabWCID))
			/* || (pApCliEntry->MacTabWCID < 0)  //MacTabWCID is UCHAR, no need to check */
		   ) {
			result = FALSE;
			break;
		}

		pMacEntry = &pAd->MacTab.Content[pApCliEntry->MacTabWCID];

		if (!IS_ENTRY_APCLI(pMacEntry)) {
			result = FALSE;
			break;
		}

		result = TRUE;
	} while (FALSE);

	return result;
}

INT apcli_fp_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	APCLI_STRUCT *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	UINT Ret = 0;
#endif
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	UCHAR frag_nums;

	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		apcli_entry = &pAd->ApCfg.ApCliTab[idx];

		if (&apcli_entry->wdev == wdev) {
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef A4_CONN
				&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */
				) {
				Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);

				if (Ret == REPEATER_ENTRY_EXIST) {
					allowed = TRUE;
				} else if (Ret == INSERT_REPT_ENTRY) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					    ("apcli_fp_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n"));
					allowed = FALSE;
				} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW) {
					allowed = TRUE;
				} else if (Ret == USE_CLI_LINK_INFO) {
					wcid = apcli_entry->MacTabWCID;
					allowed = TRUE;
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount++;
				RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
				wcid = apcli_entry->MacTabWCID;
				allowed = TRUE;
			}

			break;
		}
	}

	if (allowed) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

		/*  ethertype check is not offload to mcu for fragment frame*/
		if (frag_nums > 1) {
			if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev))
				allowed = FALSE;
		}
	}

	return allowed;
}

INT apcli_tx_pkt_allowed(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN PNDIS_PACKET pkt)
{
	UCHAR idx;
	BOOLEAN	allowed = FALSE;
	APCLI_STRUCT *apcli_entry;
#ifdef MAC_REPEATER_SUPPORT
	UINT Ret = 0;
#endif
	UCHAR wcid = RTMP_GET_PACKET_WCID(pkt);
	UCHAR frag_nums;

	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		apcli_entry = &pAd->ApCfg.ApCliTab[idx];

		if (&apcli_entry->wdev == wdev) {
			if (ValidApCliEntry(pAd, idx) == FALSE)
				break;

#ifdef MAC_REPEATER_SUPPORT
			if ((pAd->ApCfg.bMACRepeaterEn == TRUE)
#ifdef A4_CONN
				&& (IS_APCLI_A4(apcli_entry) == FALSE)
#endif /* A4_CONN */
				) {
				Ret = ReptTxPktCheckHandler(pAd, wdev, pkt, &wcid);

				if (Ret == REPEATER_ENTRY_EXIST) {
					allowed = TRUE;
				} else if (Ret == INSERT_REPT_ENTRY) {
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("apcli_tx_pkt_allowed: return FALSE as ReptTxPktCheckHandler indicated INSERT_REPT_ENTRY\n"));
					allowed = FALSE;
				} else if (Ret == INSERT_REPT_ENTRY_AND_ALLOW) {
					allowed = TRUE;
				} else if (Ret == USE_CLI_LINK_INFO) {
					wcid = apcli_entry->MacTabWCID;
					allowed = TRUE;
				}
			} else
#endif /* MAC_REPEATER_SUPPORT */
			{
				pAd->RalinkCounters.PendingNdisPacketCount++;
				RTMP_SET_PACKET_WDEV(pkt, wdev->wdev_idx);
				wcid = apcli_entry->MacTabWCID;
				allowed = TRUE;
			}
			break;
		}
	}

	if (allowed) {
		RTMP_SET_PACKET_WCID(pkt, wcid);
		frag_nums = get_frag_num(pAd, wdev, pkt);
		RTMP_SET_PACKET_FRAGMENTS(pkt, frag_nums);

		if (!RTMPCheckEtherType(pAd, pkt, &pAd->MacTab.tr_entry[wcid], wdev))
			allowed = FALSE;
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
		eid_ptr	Pointer to VIE

	Return Value:
		TRUE	for configuration match
		FALSE	for otherwise

	Note:

	========================================================================
*/
BOOLEAN ApCliValidateRSNIE(
	IN RTMP_ADAPTER *pAd,
	IN PEID_STRUCT pEid_ptr,
	IN USHORT eid_len,
	IN USHORT idx,
	IN UCHAR Privacy)
{
	PUCHAR pVIE, pTmp;
	UCHAR len;
	PEID_STRUCT pEid;
	PAPCLI_STRUCT pApCliEntry = NULL;
	USHORT Count;
	PRSN_IE_HEADER_STRUCT pRsnHeader;
	PCIPHER_SUITE_STRUCT pCipher;
	PAKM_SUITE_STRUCT pAKM;
	struct _SECURITY_CONFIG *pSecConfig;
	UINT32 AKMMap = 0;
	UINT32 PairwiseCipher = 0;
	UINT32 GroupCipher = 0;
	UCHAR end_field = 0;
	UCHAR res = TRUE;
	RSN_CAPABILITIES *pRSN_Cap = NULL;

	pVIE = (PUCHAR) pEid_ptr;
	len  = eid_len;
	pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
	pSecConfig = &pApCliEntry->wdev.SecConfig;

#ifdef APCLI_OWE_SUPPORT
	if (IS_AKM_OWE(pSecConfig->AKMMap) && (Privacy == 0)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
		CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
		SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
		SET_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher);
		SET_CIPHER_NONE(pApCliEntry->MlmeAux.GroupCipher);
		return TRUE; /* No Security */
	} else
#endif
	if (IS_SECURITY(pSecConfig) && (Privacy == 0)) {
		return FALSE; /* None matched*/
	} else if (IS_NO_SECURITY(pSecConfig) && (Privacy == 1)) {
		return FALSE; /* None matched*/
	} else if (IS_NO_SECURITY(pSecConfig) && (Privacy == 0)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
		CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);
		SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
		SET_CIPHER_NONE(pApCliEntry->MlmeAux.PairwiseCipher);
		SET_CIPHER_NONE(pApCliEntry->MlmeAux.GroupCipher);
		return TRUE; /* No Security */
	}

	AKMMap = pApCliEntry->MlmeAux.AKMMap;
	PairwiseCipher = pApCliEntry->MlmeAux.PairwiseCipher;
	GroupCipher = pApCliEntry->MlmeAux.GroupCipher;


	CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
	CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
	CLEAR_CIPHER(pApCliEntry->MlmeAux.GroupCipher);

	/* 1. Parse Cipher this received RSNIE */
	while (len > 0) {
		pTmp = pVIE;
		pEid = (PEID_STRUCT) pTmp;

		switch (pEid->Eid) {
		case IE_WPA:
			if (NdisEqualMemory(pEid->Octet, WPA_OUI, 4) != 1) {
				/* if unsupported vendor specific IE */
				break;
			}

			/* Skip OUI ,version and multicast suite OUI */
			pTmp += 11;

			/*
			    Cipher Suite Selectors from Spec P802.11i/D3.2 P26.
			    Value	   Meaning
			    0			None
			    1			WEP-40
			    2			Tkip
			    3			WRAP
			    4			AES
			    5			WEP-104
			*/
			/* Parse group cipher*/
			switch (*pTmp) {
			case 1:
				SET_CIPHER_WEP40(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.GroupCipher);
				break;

			default:
				break;
			}

			/* number of unicast suite*/
			pTmp   += 1;
			/* skip all unicast cipher suites*/
			Count = (pTmp[1]<<8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pTmp += 3;

				switch (*pTmp) {
				case 1:
					SET_CIPHER_WEP40(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 5: /* Although WEP is not allowed in WPA related auth mode, we parse it anyway*/
					SET_CIPHER_WEP104(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp++;
				Count--;
			}

			/* 4. get AKM suite counts*/
			Count = (pTmp[1]<<8) + pTmp[0];
			pTmp   += sizeof(USHORT);
			pTmp   += 3;

			switch (*pTmp) {
			case 1:
				/* Set AP support WPA-enterprise mode*/
				SET_AKM_WPA1(pApCliEntry->MlmeAux.AKMMap);
				break;

			case 2:
				/* Set AP support WPA-PSK mode*/
				SET_AKM_WPA1PSK(pApCliEntry->MlmeAux.AKMMap);
				break;

			default:
				break;
			}

			pTmp   += 1;
			break; /* End of case IE_WPA */

		case IE_RSN:
			pRsnHeader = (PRSN_IE_HEADER_STRUCT) pTmp;
			res = wpa_rsne_sanity(pTmp, le2cpu16(pRsnHeader->Length) + 2, &end_field);

			if (res == FALSE)
				break;

			if (end_field < RSN_FIELD_GROUP_CIPHER)
				SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.GroupCipher);
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.PairwiseCipher);
			if (end_field < RSN_FIELD_AKM)
				SET_AKM_WPA2(pApCliEntry->MlmeAux.AKMMap);

			/* 0. Version must be 1*/
			if (le2cpu16(pRsnHeader->Version) != 1)
				break;

			/* 1. Check group cipher*/
			if (end_field < RSN_FIELD_GROUP_CIPHER)
				break;

			pTmp   += sizeof(RSN_IE_HEADER_STRUCT);
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

			if (!RTMPEqualMemory(&pCipher->Oui, RSN_OUI, 3))
				break;

			/* Parse group cipher*/
			switch (pCipher->Type) {
			case 1:
				SET_CIPHER_WEP40(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 2:
				SET_CIPHER_TKIP(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 4:
				SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 5:
				SET_CIPHER_WEP104(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 8:
				SET_CIPHER_GCMP128(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 9:
				SET_CIPHER_GCMP256(pApCliEntry->MlmeAux.GroupCipher);
				break;

			case 10:
				SET_CIPHER_CCMP256(pApCliEntry->MlmeAux.GroupCipher);
				break;

			default:
				break;
			}

			/* set to correct offset for next parsing*/
			pTmp   += sizeof(CIPHER_SUITE_STRUCT);
			/* 2. Get pairwise cipher counts*/
			if (end_field < RSN_FIELD_PAIRWISE_CIPHER)
				break;
			Count = (pTmp[1]<<8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 3. Get pairwise cipher*/
			/* Parsing all unicast cipher suite*/
			while (Count > 0) {
				/* Skip OUI*/
				pCipher = (PCIPHER_SUITE_STRUCT) pTmp;

				switch (pCipher->Type) {
				case 1:
					SET_CIPHER_WEP40(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 2:
					SET_CIPHER_TKIP(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 4:
					SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 5:
					SET_CIPHER_WEP104(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 8:
					SET_CIPHER_GCMP128(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 9:
					SET_CIPHER_GCMP256(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				case 10:
					SET_CIPHER_CCMP256(pApCliEntry->MlmeAux.PairwiseCipher);
					break;

				default:
					break;
				}

				pTmp += sizeof(CIPHER_SUITE_STRUCT);
				Count--;
			}

			/* 4. get AKM suite counts*/
			if (end_field < RSN_FIELD_AKM)
				break;
			Count = (pTmp[1]<<8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 5. Get AKM ciphers*/
			/* Parsing all AKM ciphers*/
			while (Count > 0) {
				pAKM = (PAKM_SUITE_STRUCT) pTmp;

				if (!RTMPEqualMemory(pTmp, RSN_OUI, 3))
					break;

				switch (pAKM->Type) {
				case 0:
					SET_AKM_WPANONE(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 1:
					SET_AKM_WPA2(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 2:
					SET_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 3:
					SET_AKM_FT_WPA2(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 4:
					SET_AKM_FT_WPA2PSK(pApCliEntry->MlmeAux.AKMMap);
					break;
#ifdef DOT11W_PMF_SUPPORT

				case 5:
					SET_AKM_WPA2(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 6:
					SET_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap);
					break;
#else /* DOT11W_PMF_SUPPORT */

				case 5:
					SET_AKM_WPA2_SHA256(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 6:
					SET_AKM_WPA2PSK_SHA256(pApCliEntry->MlmeAux.AKMMap);
					break;
#endif /* !DOT11W_PMF_SUPPORT */

				case 7:
					SET_AKM_TDLS(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 8:
					SET_AKM_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 9:
					SET_AKM_FT_SAE_SHA256(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 11:
					SET_AKM_SUITEB_SHA256(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 12:
					SET_AKM_SUITEB_SHA384(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 13:
					SET_AKM_FT_WPA2_SHA384(pApCliEntry->MlmeAux.AKMMap);
					break;

				case 18:
					SET_AKM_OWE(pApCliEntry->MlmeAux.AKMMap);
					break;

				default:
					break;
				}

				pTmp   += sizeof(AKM_SUITE_STRUCT);
				Count--;
			}

			/* 6. Get RSN capability*/
			if (end_field < RSN_FIELD_RSN_CAP)
				break;



			pRSN_Cap = (RSN_CAPABILITIES *) pTmp;
#ifdef APCLI_OWE_SUPPORT
			if (IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap)) {
			/*OWE connection should be allowed with only OWE AP which has PMF Capable and PMF required set*/
				if ((pRSN_Cap->field.MFPC == 0) || (pRSN_Cap->field.MFPR == 0))
					return FALSE;
			}
#endif

			if ((pRSN_Cap->field.MFPC == 1) && (end_field < RSN_FIELD_GROUP_MGMT_CIPHER))
				SET_CIPHER_BIP_CMAC128(pApCliEntry->MlmeAux.IntegrityGroupCipher);


			/* skip RSN capability, update in ApCliPeerProbeRspAtJoinAction */
			pTmp += sizeof(USHORT);

			/* 7. get PMKID counts*/
			if (end_field < RSN_FIELD_PMKID)
				break;
			/*Count	= *(PUSHORT) pTmp;*/
			Count = (pTmp[1] << 8) + pTmp[0];
			pTmp   += sizeof(USHORT);

			/* 8. ignore PMKID */
			pTmp += 16 * Count;

			/* 9. Get Group Management Cipher Suite*/
			if (end_field < RSN_FIELD_GROUP_MGMT_CIPHER)
				break;
			pCipher = (PCIPHER_SUITE_STRUCT) pTmp;
			if (!RTMPEqualMemory(&pCipher->Oui, RSN_OUI, 3))
				break;
			switch (pCipher->Type) {
			case 6:
				SET_CIPHER_BIP_CMAC128(pApCliEntry->MlmeAux.IntegrityGroupCipher);
				break;

			case 11:
				SET_CIPHER_BIP_GMAC128(pApCliEntry->MlmeAux.IntegrityGroupCipher);
				break;

			case 12:
				SET_CIPHER_BIP_GMAC256(pApCliEntry->MlmeAux.IntegrityGroupCipher);
				break;

			case 13:
				SET_CIPHER_BIP_CMAC256(pApCliEntry->MlmeAux.IntegrityGroupCipher);
				break;
			default:
				MTWF_LOG(DBG_CAT_SEC, DBG_SUBCAT_ALL, DBG_LVL_OFF,
					("%s: unknown Group Management Cipher Suite %d\n", __func__, pCipher->Type));
				break;
			}
			pTmp += sizeof(CIPHER_SUITE_STRUCT);
		}

		/* skip this Eid */
		pVIE += (pEid->Len + 2);
		len  -= (pEid->Len + 2);
	}

	if ((pApCliEntry->MlmeAux.AKMMap == 0x0) && (Privacy == 1)) {
		/* WEP mode */
		if (IS_AKM_AUTOSWITCH(pSecConfig->AKMMap))
			SET_AKM_AUTOSWITCH(pApCliEntry->MlmeAux.AKMMap);
		else if (IS_AKM_OPEN(pSecConfig->AKMMap))
			SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);
		else if (IS_AKM_SHARED(pSecConfig->AKMMap))
			SET_AKM_SHARED(pApCliEntry->MlmeAux.AKMMap);
		else
			SET_AKM_OPEN(pApCliEntry->MlmeAux.AKMMap);

		SET_CIPHER_WEP(pApCliEntry->MlmeAux.PairwiseCipher);
		SET_CIPHER_WEP(pApCliEntry->MlmeAux.GroupCipher);
	}
#ifdef APCLI_SAE_SUPPORT
	if (IS_AKM_WPA2PSK_ONLY(pApCliEntry->MlmeAux.AKMMap) &&
		IS_AKM_WPA3PSK_ONLY(pSecConfig->AKMMap)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		SET_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap);
		pApCliEntry->MlmeAux.PairwiseCipher &=  pSecConfig->PairwiseCipher;
	} else
#endif
#ifdef APCLI_OWE_SUPPORT
	/*Disallow TKIP cipher with OWE*/
	if (IS_AKM_OWE(pApCliEntry->MlmeAux.AKMMap) && IS_CIPHER_TKIP(pApCliEntry->MlmeAux.PairwiseCipher))
		return FALSE;
	else
#endif
	{
		pApCliEntry->MlmeAux.AKMMap &=  pSecConfig->AKMMap;
		pApCliEntry->MlmeAux.PairwiseCipher &=  pSecConfig->PairwiseCipher;

	}




	if ((pApCliEntry->MlmeAux.AKMMap == 0)
		|| (pApCliEntry->MlmeAux.PairwiseCipher == 0)) {

		pApCliEntry->MlmeAux.AKMMap =  AKMMap;
		pApCliEntry->MlmeAux.PairwiseCipher = PairwiseCipher;
		pApCliEntry->MlmeAux.GroupCipher = GroupCipher;
		return FALSE; /* None matched*/
	}
	/* Decide Pairwise and group cipher with AP */
	if (IS_AKM_WPA1(pApCliEntry->MlmeAux.AKMMap) && IS_AKM_WPA2(pApCliEntry->MlmeAux.AKMMap)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		SET_AKM_WPA2(pApCliEntry->MlmeAux.AKMMap);
	} else if (IS_AKM_WPA1PSK(pApCliEntry->MlmeAux.AKMMap) && IS_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		SET_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap);
	}
#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	else if (IS_AKM_WPA2PSK(pApCliEntry->MlmeAux.AKMMap) && IS_AKM_WPA3PSK(pApCliEntry->MlmeAux.AKMMap)) {
		CLEAR_SEC_AKM(pApCliEntry->MlmeAux.AKMMap);
		SET_AKM_WPA3PSK(pApCliEntry->MlmeAux.AKMMap);
	}
#endif
	if (IS_CIPHER_TKIP(pApCliEntry->MlmeAux.PairwiseCipher) && IS_CIPHER_CCMP128(pApCliEntry->MlmeAux.PairwiseCipher)) {
		CLEAR_CIPHER(pApCliEntry->MlmeAux.PairwiseCipher);
		SET_CIPHER_CCMP128(pApCliEntry->MlmeAux.PairwiseCipher);
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s(): Candidate Security AKMMap=%s, PairwiseCipher=%s, GroupCipher=%s\n",
			 __func__,
			 GetAuthModeStr(pApCliEntry->MlmeAux.AKMMap),
			 GetEncryModeStr(pApCliEntry->MlmeAux.PairwiseCipher),
			 GetEncryModeStr(pApCliEntry->MlmeAux.GroupCipher)));
	return TRUE;
}

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK * pRxBlk,
	IN MAC_TABLE_ENTRY *pEntry)
{
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

	if (MAC_ADDR_EQUAL(pRxBlk->Addr3, pApCliEntry->wdev.if_addr))
		return FALSE;

	if (pEntry->PrivacyFilter != Ndis802_11PrivFilterAcceptAll)
		return FALSE;

#ifdef MAC_REPEATER_SUPPORT

	if (pAd->ApCfg.bMACRepeaterEn
#ifdef A4_CONN
		&& (IS_APCLI_A4(pApCliEntry) == FALSE)
#endif /* A4_CONN */
		) {
		pReptEntry = RTMPLookupRepeaterCliEntry(pAd, FALSE, pRxBlk->Addr3, TRUE);

		if (pReptEntry){
			//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			//    ("ApCliHandleRxBroadcastFrame: return FALSE  pReptEntry found\n"));
			return FALSE;	/* give up this frame */
		}
	}

#endif /* MAC_REPEATER_SUPPORT */
	return TRUE;
}

/*
	========================================================================

	Routine Description:
		Verify the support rate for different PHY type

	Arguments:
		pAd				Pointer to our adapter

	Return Value:
		None

	IRQL = PASSIVE_LEVEL

	========================================================================
*/
/* TODO: shiang-6590, modify this due to it's really a duplication of "RTMPUpdateMlmeRate()" in common/mlme.c */
VOID ApCliUpdateMlmeRate(RTMP_ADAPTER *pAd, USHORT ifIndex)
{
	UCHAR	MinimumRate;
	UCHAR	ProperMlmeRate; /*= RATE_54; */
	UCHAR	i, j, RateIdx = 12; /* 1, 2, 5.5, 11, 6, 9, 12, 18, 24, 36, 48, 54 */
	BOOLEAN	bMatch = FALSE;
	struct wifi_dev *wdev;
	struct dev_rate_info *rate;
	PAPCLI_STRUCT pApCliEntry = NULL;

	if (ifIndex >= MAX_APCLI_NUM)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	wdev = &pApCliEntry->wdev;
	rate = &wdev->rate;

	switch (wdev->PhyMode) {
	case (WMODE_B):
		ProperMlmeRate = RATE_11;
		MinimumRate = RATE_1;
		break;

	case (WMODE_B | WMODE_G):
#ifdef DOT11_N_SUPPORT
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN):
	case (WMODE_B | WMODE_G | WMODE_GN):
#ifdef DOT11_VHT_AC
	case (WMODE_A | WMODE_B | WMODE_G | WMODE_GN | WMODE_AN | WMODE_AC):
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

	for (i = 0; i < pApCliEntry->MlmeAux.SupRateLen; i++) {
		for (j = 0; j < RateIdx; j++) {
			if ((pApCliEntry->MlmeAux.SupRate[i] & 0x7f) == RateIdTo500Kbps[j]) {
				if (j == ProperMlmeRate) {
					bMatch = TRUE;
					break;
				}
			}
		}

		if (bMatch)
			break;
	}

	if (bMatch == FALSE) {
		for (i = 0; i < pApCliEntry->MlmeAux.ExtRateLen; i++) {
			for (j = 0; j < RateIdx; j++) {
				if ((pApCliEntry->MlmeAux.ExtRate[i] & 0x7f) == RateIdTo500Kbps[j]) {
					if (j == ProperMlmeRate) {
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

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_AP_STATUS_MEDIA_STATE_CONNECTED)) {
		pAd->CommonCfg.MlmeRate = MinimumRate;
		pAd->CommonCfg.RtsRate = ProperMlmeRate;

		if (pAd->CommonCfg.MlmeRate >= RATE_6) {
			rate->MlmeTransmit.field.MODE = MODE_OFDM;
			rate->MlmeTransmit.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_OFDM;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = OfdmRateToRxwiMCS[pAd->CommonCfg.MlmeRate];
		} else {
			rate->MlmeTransmit.field.MODE = MODE_CCK;
			rate->MlmeTransmit.field.MCS = pAd->CommonCfg.MlmeRate;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MODE = MODE_CCK;
			pAd->MacTab.Content[BSS0Mcast_WCID].HTPhyMode.field.MCS = pAd->CommonCfg.MlmeRate;
		}
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s():=>MlmeTransmit=0x%x, MinimumRate=%d, ProperMlmeRate=%d\n",
			 __func__, rate->MlmeTransmit.word, MinimumRate, ProperMlmeRate));
}
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR *Bssid, UCHAR Channel)
#else
VOID ApCliCheckPeerExistence(RTMP_ADAPTER *pAd, CHAR *Ssid, UCHAR SsidLen, UCHAR Channel)
#endif
{
	UCHAR ifIndex;
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
	UINT32 mbss_idx = 0;
	UCHAR ZeroSsid[MAX_LEN_OF_SSID] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
#endif
	APCLI_STRUCT *pApCliEntry;

	for (ifIndex = 0; ifIndex < MAX_APCLI_NUM; ifIndex++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
			if (pApCliEntry->bPeerExist == TRUE
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
				&& !NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)
#endif
				) {
				continue;
			}
			else if (Channel == pApCliEntry->wdev.channel &&
					 ((SsidLen == pApCliEntry->CfgSsidLen && NdisEqualMemory(Ssid, pApCliEntry->CfgSsid, SsidLen)) ||
		#ifdef FOLLOW_HIDDEN_SSID_FEATURE
					 ((SsidLen == 0 && NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)) ||
					 ((NdisEqualMemory(Ssid, ZeroSsid, SsidLen)) && NdisEqualMemory(Bssid, pApCliEntry->MlmeAux.Bssid, MAC_ADDR_LEN)))
		#else
					(SsidLen == 0)
		#endif
					 )) {
							pApCliEntry->bPeerExist = TRUE;
#ifdef FOLLOW_HIDDEN_SSID_FEATURE
							if (pApCliEntry->CtrlCurrState != APCLI_CTRL_CONNECTED)
								continue;

							if ((SsidLen == 0 || NdisEqualMemory(Ssid, ZeroSsid, SsidLen)) && !pApCliEntry->MlmeAux.Hidden) {
								pApCliEntry->MlmeAux.Hidden = 1;
								printk("(%s): !! Following Hidden SSID now !!\n", __func__);
								for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
									if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == pApCliEntry->wdev.PhyMode) {
										pAd->ApCfg.MBSSID[mbss_idx].bHideSsid = pApCliEntry->MlmeAux.Hidden;
										MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]-->Follow Root AP Hidden ssid\n", __func__));
										/*Feature:Update forwardhaul beaconing ssid to hidden if root ap hidden */
										UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[mbss_idx].wdev, BCN_UPDATE_IE_CHG);
									}
								}
							} else if (pApCliEntry->MlmeAux.Hidden && !(SsidLen == 0 || NdisEqualMemory(Ssid, ZeroSsid, SsidLen))) {
									pApCliEntry->MlmeAux.Hidden = 0;
									printk("(%s): !! Following Broadcast SSID now !!\n", __func__);
									for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum ; mbss_idx++) {
										if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == pApCliEntry->wdev.PhyMode) {
											pAd->ApCfg.MBSSID[mbss_idx].bHideSsid = pApCliEntry->MlmeAux.Hidden;
											MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("[%s]-->Follow Root AP Broadcast ssid\n", __func__));
											UpdateBeaconHandler(pAd, &pAd->ApCfg.MBSSID[mbss_idx].wdev, BCN_UPDATE_IE_CHG);
										}
									}
							} else {
								MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("[%s] No action required SSID Len = %d,Hidden status = %d\n", __func__, SsidLen, pApCliEntry->MlmeAux.Hidden));
							}
#endif
						} else {
						/* No Root AP match the SSID */
						}
	}
}

#ifdef CONFIG_MAP_SUPPORT
VOID ApCliCheckConConnectivity(RTMP_ADAPTER *pAd, APCLI_STRUCT *pApCliEntry, BCN_IE_LIST *ie_list)
{
	struct _vendor_ie_cap *vendor_ie = &ie_list->vendor_ie;
	UINT32 TotalLen = 0;
	UCHAR *msg;
	struct wifi_dev *wdev;
	struct wapp_event *event;
	wdev = &pApCliEntry->wdev;

	if (!IS_MAP_TURNKEY_ENABLE(pAd))
		return;

	if (pApCliEntry->last_controller_connectivity != vendor_ie->map_info.connectivity_to_controller) {
		TotalLen = sizeof(CHAR) * 2 + sizeof(struct map_vendor_ie) + sizeof(UINT32);
		os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);
		if (msg == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s:failed to allocated memory\n", __func__));
			return;
		}
		event = (struct wapp_event *)msg;
		event->event_id = WAPP_MAP_VENDOR_IE;
		event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		NdisCopyMemory(&event->data, &vendor_ie->map_info, sizeof(struct map_vendor_ie));
		RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM,
					OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("send connectivity change event to user space %u %u\n",
					pApCliEntry->last_controller_connectivity,
					vendor_ie->map_info.connectivity_to_controller));
		pApCliEntry->last_controller_connectivity = vendor_ie->map_info.connectivity_to_controller;
		os_free_mem((PUCHAR)msg);
	}
}
#else
VOID ApCliCheckConConnectivity(RTMP_ADAPTER *pAd, APCLI_STRUCT *pApCliEntry, BCN_IE_LIST *ie_list)
{ }
#endif
VOID ApCliPeerCsaAction(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *ie_list)
{
	struct DOT11_H *pDot11h = NULL;

	if (wdev == NULL)
		return;

	pDot11h = wdev->pDot11_H;

	if (pAd == NULL || ie_list == NULL)
		return;

	if (pDot11h == NULL)
		return;

	if ((pAd->CommonCfg.bIEEE80211H == 1) &&
		 ie_list->NewChannel != 0 &&
		 wdev->channel != ie_list->NewChannel &&
		 pDot11h->RDMode != RD_SWITCHING_MODE) {
#ifdef DOT11_VHT_AC
{
		struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
		if (IS_CAP_BW160(cap)) {

			VHT_OP_INFO *vht_op = &ie_list->vht_op_ie.vht_op_info;

			print_vht_op_info(vht_op);
			wlan_operate_set_cen_ch_2(wdev, vht_op->center_freq_2);
		}
}
#endif /* DOT11_VHT_AC */
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("[APCLI]  Following root AP to switch channel to ch%u\n",
				  ie_list->NewChannel));
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (pAd->bMAPQuickChChangeEn)
		wdev->quick_ch_change = TRUE;

		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
		("Channel Change due to csa\n"));
#endif
		rtmp_set_channel(pAd, wdev, ie_list->NewChannel);
#if defined(WAPP_SUPPORT) && defined(CONFIG_MAP_SUPPORT)
		if (pAd->bMAPQuickChChangeEn)
		wdev->quick_ch_change = FALSE;
		wapp_send_csa_event(pAd, RtmpOsGetNetIfIndex(wdev->if_dev), ie_list->NewChannel);
#endif
	}
}

static void apcli_sync_wdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct wifi_dev *ap_wdev = NULL;

	if (pAd->CommonCfg.dbdc_mode == TRUE) {
		int mbss_idx;

		/*for 5G+5G case choose both phymode & func_idx the same first.*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode && wdev->func_idx == mbss_idx) {
				ap_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
				update_att_from_wdev(wdev, ap_wdev);
			}
		}

		if (ap_wdev)
			return;

		/*original rule*/
		for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
			if (pAd->ApCfg.MBSSID[mbss_idx].wdev.PhyMode == wdev->PhyMode){
				update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[mbss_idx].wdev);
			}
		}
	} else {
		/* align phy mode to BSS0 by default */
		wdev->PhyMode = pAd->ApCfg.MBSSID[BSS0].wdev.PhyMode;
		update_att_from_wdev(wdev, &pAd->ApCfg.MBSSID[BSS0].wdev);
	}
}

BOOLEAN apcli_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
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
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd) {
#ifdef A4_CONN
				if (IS_ENTRY_A4(pMacEntry)) {
					pTxBlk->pMacEntry = pMacEntry;
					pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
				} else
#endif /* A4_CONN */
				if (pMacEntry && (IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
					((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
					 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
#ifdef MAT_SUPPORT
					PNDIS_PACKET apCliPkt = NULL;
					UCHAR *pMacAddr = NULL;
#ifdef MAC_REPEATER_SUPPORT

					if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn)) {
						UCHAR tmpIdx;

						pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

						if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
							tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
							/* TODO: shiang-lock, fix ME! */
							apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
							pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
						}
					} else
#endif /* MAC_REPEATER_SUPPORT */
					{
						/* For each tx packet, update our MAT convert engine databases.*/
						/* CFG_TODO */
#ifdef APCLI_AS_WDS_STA_SUPPORT
						if (pAd->ApCfg.ApCliTab[0].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
						apCliPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
						pMacAddr = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx].wdev.if_addr[0];
					}

					if (apCliPkt) {
						RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
						pPacket = apCliPkt;
						RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
						pTxBlk->pPacket = apCliPkt;
					}

					if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
						PUCHAR pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
							if (pMacAddr
#ifdef APCLI_AS_WDS_STA_SUPPORT
							&& (pAd->ApCfg.ApCliTab[0].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
							)
							NdisMoveMemory(pSrcBufVA+6, pMacAddr, MAC_ADDR_LEN);
					}

#endif /* MAT_SUPPORT */
					pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
					TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
				} else if (pMacEntry && IS_ENTRY_CLIENT(pMacEntry)) {
					;
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

			if (!pMacEntry)
				MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
			else if ((IS_HT_RATE(pMacEntry) == FALSE) &&
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

		if (!pMacEntry)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s():Err!! pMacEntry is NULL!!\n", __func__));
		else {
			pMacEntry->DebugTxCount++;
#ifdef MAC_REPEATER_SUPPORT

			if (pMacEntry->bReptCli)
				pMacEntry->ReptCliIdleCount = 0;

#endif
		}
	}

	pAd->LastTxRate = (USHORT)pTxBlk->pTransmit->word;
	ops->find_cipher_algorithm(pAd, wdev, pTxBlk);
	return TRUE;
}

BOOLEAN apcli_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk)
{
	PACKET_INFO PacketInfo;
	PNDIS_PACKET pPacket;
	UCHAR *pMacAddr = NULL;
	PMAC_TABLE_ENTRY pMacEntry = NULL;
#ifdef MAT_SUPPORT
	PUCHAR pSrcBufVA = NULL;
	PNDIS_PACKET convertPkt = NULL;
#endif
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

	pMacEntry = &pAd->MacTab.Content[pTxBlk->Wcid];

#ifdef A4_CONN
	if (IS_ENTRY_A4(pMacEntry)) {
		pTxBlk->pMacEntry = pMacEntry;
		pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
		TX_BLK_SET_FLAG(pTxBlk, fTX_bA4Frame);
	} else
#endif /* A4_CONN */
	if ((IS_ENTRY_APCLI(pMacEntry) || IS_ENTRY_REPEATER(pMacEntry)) &&
		((pTxBlk->TxFrameType != TX_MCAST_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_DATAQ_FRAME) &&
		 (pTxBlk->TxFrameType != TX_MLME_MGMTQ_FRAME))) {
#ifdef MAT_SUPPORT
#ifdef MAC_REPEATER_SUPPORT

		if ((pMacEntry->bReptCli) && (pAd->ApCfg.bMACRepeaterEn)) {
			UCHAR tmpIdx;

			pAd->MatCfg.bMACRepeaterEn = pAd->ApCfg.bMACRepeaterEn;

			if (pAd->ApCfg.MACRepeaterOuiMode != CASUALLY_DEFINE_MAC_ADDR) {
				tmpIdx = REPT_MLME_START_IDX + pMacEntry->MatchReptCliIdx;
				convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, tmpIdx, pTxBlk->OpMode);
				pMacAddr = &pAd->ApCfg.pRepeaterCliPool[pMacEntry->MatchReptCliIdx].CurrentAddress[0];
			}
		} else
#endif /* MAC_REPEATER_SUPPORT */
		{
#ifdef APCLI_AS_WDS_STA_SUPPORT
			if (pAd->ApCfg.ApCliTab[0].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
			/* For each tx packet, update our MAT convert engine databases.*/
			convertPkt = (PNDIS_PACKET)MATEngineTxHandle(pAd, pPacket, pMacEntry->func_tb_idx, pTxBlk->OpMode);
			pMacAddr = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx].wdev.if_addr[0];
		}

		if (convertPkt) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_SUCCESS);
			pPacket = convertPkt;
			RTMP_QueryPacketInfo(pPacket, &PacketInfo, &pTxBlk->pSrcBufHeader, &pTxBlk->SrcBufLen);
			pTxBlk->pPacket = convertPkt;
		}

		if (TX_BLK_TEST_FLAG(pTxBlk, fTX_HDR_TRANS)) {
			if (pMacAddr != NULL) {
				if (pMacAddr) {
					pSrcBufVA = GET_OS_PKT_DATAPTR(pPacket);
#ifdef APCLI_AS_WDS_STA_SUPPORT
					if (pAd->ApCfg.ApCliTab[0].wdev.wds_enable == 0)
#endif /* APCLI_AS_WDS_STA_SUPPORT */
					NdisMoveMemory(pSrcBufVA + 6, pMacAddr, MAC_ADDR_LEN);
				}
			}
		}
#endif /* MAT_SUPPORT */
		pTxBlk->pApCliEntry = &pAd->ApCfg.ApCliTab[pMacEntry->func_tb_idx];
		pTxBlk->pMacEntry = pMacEntry;
		TX_BLK_SET_FLAG(pTxBlk, fTX_bApCliPacket);
#ifdef MAC_REPEATER_SUPPORT

		if (pMacEntry->bReptCli)
			pMacEntry->ReptCliIdleCount = 0;

#endif
	}

	pTxBlk->wmm_set = HcGetWmmIdx(pAd, wdev);
	pTxBlk->pSrcBufData = pTxBlk->pSrcBufHeader;
	return TRUE;
}

static struct wifi_dev_ops apcli_wdev_ops = {
	.tx_pkt_allowed = apcli_tx_pkt_allowed,
	.fp_tx_pkt_allowed = apcli_fp_tx_pkt_allowed,
	.send_data_pkt = ap_send_data_pkt,
	.fp_send_data_pkt = fp_send_data_pkt,
	.send_mlme_pkt = ap_send_mlme_pkt,
	.tx_pkt_handle = ap_tx_pkt_handle,
	.fill_non_offload_tx_blk = apcli_fill_non_offload_tx_blk,
	.fill_offload_tx_blk = apcli_fill_offload_tx_blk,
	.legacy_tx = ap_legacy_tx,
	.ampdu_tx = ap_ampdu_tx,
	.frag_tx = ap_frag_tx,
	.amsdu_tx = ap_amsdu_tx,
	.mlme_mgmtq_tx = ap_mlme_mgmtq_tx,
	.mlme_dataq_tx = ap_mlme_dataq_tx,
	.ieee_802_11_data_tx = ap_ieee_802_11_data_tx,
	.ieee_802_3_data_tx = ap_ieee_802_3_data_tx,
	.rx_pkt_allowed = sta_rx_pkt_allow,
	.rx_pkt_foward = sta_rx_fwd_hnd,
	.ieee_802_3_data_rx = ap_ieee_802_3_data_rx,
	.ieee_802_11_data_rx = ap_ieee_802_11_data_rx,
	.find_cipher_algorithm = ap_find_cipher_algorithm,
	.mac_entry_lookup = mac_entry_lookup,
};

VOID APCli_Init(RTMP_ADAPTER *pAd, RTMP_OS_NETDEV_OP_HOOK *pNetDevOps)
{
#define APCLI_MAX_DEV_NUM	32
	PNET_DEV new_dev_p;
	INT idx;
	APCLI_STRUCT *pApCliEntry;
	struct wifi_dev *wdev;
	UINT8 MaxNumApcli;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	/* sanity check to avoid redundant virtual interfaces are created */
	if (pAd->flg_apcli_init != FALSE) {
		for (idx = 0; idx < pAd->ApCfg.ApCliNum; idx++) {
			pApCliEntry = &pAd->ApCfg.ApCliTab[idx];
			wdev = &pApCliEntry->wdev;
			apcli_sync_wdev(pAd, wdev);
		}

		return;
	}

	/* init */
	for (idx = 0; idx < MAX_APCLI_NUM; idx++) {
		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];

		if (pApCliEntry->ApCliInit != FALSE)
			continue;

		pApCliEntry->wdev.if_dev = NULL;
	}

	MaxNumApcli = pAd->ApCfg.ApCliNum;

	if (MaxNumApcli > MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("ApCliNum(%d) exceeds MAX_APCLI_NUM(%d)!\n", MaxNumApcli, MAX_APCLI_NUM));
		return;
	}

	/* create virtual network interface */
	for (idx = 0; idx < MaxNumApcli; idx++) {
		UINT32 MC_RowID = 0, IoctlIF = 0;
		INT32 Ret = 0;
		char *dev_name;
#ifdef MULTI_PROFILE
		UCHAR final_name[32] = "";
#endif

#ifdef MULTIPLE_CARD_SUPPORT
		MC_RowID = pAd->MC_RowID;
#endif /* MULTIPLE_CARD_SUPPORT */
#ifdef HOSTAPD_SUPPORT
		IoctlIF = pAd->IoctlIF;
#endif /* HOSTAPD_SUPPORT */
		pApCliEntry = &pAd->ApCfg.ApCliTab[idx];

		/* sanity check to avoid redundant virtual interfaces are created */
		if (pApCliEntry->ApCliInit != FALSE)
			continue;

		dev_name = get_dev_name_prefix(pAd, INT_APCLI);
#ifdef MULTI_PROFILE
		if (dev_name == NULL) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					("%s(): apcli interface name is null,apcli idx=%d!\n",
					 __func__, idx));
			break;
		}
		snprintf(final_name, sizeof(final_name), "%s", dev_name);
		multi_profile_apcli_devname_req(pAd, final_name, &idx);
		if (pAd->CommonCfg.dbdc_mode == TRUE) {
			/* MULTI_PROFILE enable, apcli interface name will be apcli0,apclix0*/
			new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, 0,
							sizeof(struct mt_dev_priv), final_name, TRUE);
		} else {
			new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, idx,
							sizeof(struct mt_dev_priv), final_name, TRUE);
		}
#else
		new_dev_p = RtmpOSNetDevCreate(MC_RowID, &IoctlIF, INT_APCLI, idx,
						sizeof(struct mt_dev_priv), dev_name, TRUE);
#endif /*MULTI_PROFILE*/
		if (!new_dev_p) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s(): Create net_device for %s(%d) fail!\n",
					 __func__, dev_name, idx));
			break;
		}

#ifdef HOSTAPD_SUPPORT
		pAd->IoctlIF = IoctlIF;
#endif /* HOSTAPD_SUPPORT */
#ifdef FAST_EAPOL_WAR
		pApCliEntry->MacTabWCID = 0;
		pApCliEntry->pre_entry_alloc = FALSE;
#endif /* FAST_EAPOL_WAR */
		pApCliEntry->ifIndex = idx;
		pApCliEntry->pAd = pAd;

#ifdef APCLI_SAE_SUPPORT
		pApCliEntry->sae_cfg_group = SAE_DEFAULT_GROUP;
#endif
#ifdef APCLI_OWE_SUPPORT
		pApCliEntry->curr_owe_group = ECDH_GROUP_256;
#endif


		ApCliCompleteInit(pApCliEntry);
		wdev = &pApCliEntry->wdev;
		Ret = wdev_init(pAd, wdev,
						WDEV_TYPE_APCLI,
						new_dev_p,
						idx,
						(VOID *)pApCliEntry,
						(VOID *)pAd);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("Assign wdev idx for %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		Ret = wdev_ops_register(wdev, WDEV_TYPE_APCLI, &apcli_wdev_ops,
								cap->wmm_detect_method);

		if (!Ret) {
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
					 ("register wdev_ops %s failed, free net device!\n",
					  RTMP_OS_NETDEV_GET_DEVNAME(new_dev_p)));
			RtmpOSNetDevFree(new_dev_p);
			break;
		}

		COPY_MAC_ADDR(wdev->if_addr, pAd->CurrentAddress);
		apcli_sync_wdev(pAd, wdev);
		/*update rate info*/
		RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);
		RTMP_OS_NETDEV_SET_PRIV(new_dev_p, pAd);
		RTMP_OS_NETDEV_SET_WDEV(new_dev_p, wdev);
#ifdef MT_MAC

		if (!IS_HIF_TYPE(pAd, HIF_MT)) {
#endif /* MT_MAC */

			if (cap->MBSSIDMode >= MBSSID_MODE1) {
				if ((pAd->ApCfg.BssidNum > 0) || (MAX_MESH_NUM > 0)) {
					UCHAR MacMask = 0;

					if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 2)
						MacMask = 0xFE;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 4)
						MacMask = 0xFC;
					else if ((pAd->ApCfg.BssidNum + MAX_APCLI_NUM + MAX_MESH_NUM) <= 8)
						MacMask = 0xF8;

					/*
						Refer to HW definition -
							Bit1 of MAC address Byte0 is local administration bit
							and should be set to 1 in extended multiple BSSIDs'
							Bit3~ of MAC address Byte0 is extended multiple BSSID index.
					*/
					if (cap->MBSSIDMode == MBSSID_MODE1) {
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
					else {
						wdev->if_addr[0] |= 0x2;
						wdev->if_addr[cap->MBSSIDMode - 1] &= (MacMask);
						wdev->if_addr[cap->MBSSIDMode - 1] +=
							((pAd->ApCfg.BssidNum + MAX_MESH_NUM) - 1);
					}

#endif /* ENHANCE_NEW_MBSSID_MODE */
				}
			} else
				wdev->if_addr[MAC_ADDR_LEN - 1] = (wdev->if_addr[MAC_ADDR_LEN - 1] + pAd->ApCfg.BssidNum + MAX_MESH_NUM) & 0xFF;

#ifdef MT_MAC
		} else {
			UCHAR MacByte = 0;
			UCHAR MacMask = 0xef;
			UINT32 Value = 0;
			RTMP_IO_READ32(pAd, LPON_BTEIR, &Value);
			MacByte = Value >> 29;
			if (MaxNumApcli <= 2)
				MacMask = 0xef;
			else if (MaxNumApcli <= 4)
				MacMask = 0xcf;

			wdev->if_addr[0] |= 0x2; /* bit 1 needs to turn on for local mac address definition*/
			/* apcli can not use the same mac as MBSS,so change if_addr[0] to separate */
			if ((wdev->if_addr[0] & 0x4) == 0x4)
				wdev->if_addr[0] &= ~0x4;
			else
				wdev->if_addr[0] |= 0x4;

			switch (MacByte) {
			case 0x1: /* choose bit[23:20]*/
				wdev->if_addr[2] = wdev->if_addr[2] & MacMask;/*clear high 4 bits,*/
				wdev->if_addr[2] = (wdev->if_addr[2] | (idx << 4));
				break;
			case 0x2: /* choose bit[31:28]*/
				wdev->if_addr[3] = wdev->if_addr[3] & MacMask;/*clear high 4 bits,*/
				wdev->if_addr[3] = (wdev->if_addr[3] | (idx << 4));
				break;
			case 0x3: /* choose bit[39:36]*/
				wdev->if_addr[4] = wdev->if_addr[4] & MacMask;/*clear high 4 bits,*/
				wdev->if_addr[4] = (wdev->if_addr[4] | (idx << 4));
				break;
			case 0x4: /* choose bit [47:44]*/
				wdev->if_addr[5] = wdev->if_addr[5] & MacMask;/*clear high 4 bits,*/
				wdev->if_addr[5] = (wdev->if_addr[5] | (idx << 4));
				break;
			default: /* choose bit[15:12]*/
				wdev->if_addr[1] = wdev->if_addr[1] & MacMask;/*clear high 4 bits,*/
				wdev->if_addr[1] = (wdev->if_addr[1] | (idx << 4));
				break;
			}
		}

#endif /* MT_MAC */
		pNetDevOps->priv_flags = INT_APCLI; /* we are virtual interface */
		pNetDevOps->needProtcted = TRUE;
		pNetDevOps->wdev = wdev;
		NdisMoveMemory(pNetDevOps->devAddr, &wdev->if_addr[0], MAC_ADDR_LEN);

#ifdef APCLI_CFG80211_SUPPORT
		{
			struct wireless_dev *pWdev;
			CFG80211_CB *p80211CB = pAd->pCfg80211_CB;
			UINT32 DevType = RT_CMD_80211_IFTYPE_STATION;

			pWdev = kzalloc(sizeof(*pWdev), GFP_KERNEL);
			new_dev_p->ieee80211_ptr = pWdev;
			pWdev->wiphy = p80211CB->pCfg80211_Wdev->wiphy;
			SET_NETDEV_DEV(new_dev_p, wiphy_dev(pWdev->wiphy));
			pWdev->netdev = new_dev_p;
			pWdev->iftype = DevType;
			pWdev->use_4addr = true;
		}
#endif /* APCLI_CFG80211_SUPPORT */

		/* register this device to OS */
		RtmpOSNetDevAttach(pAd->OpMode, new_dev_p, pNetDevOps);
		pApCliEntry->ApCliInit = TRUE;
	}

#ifdef MAC_REPEATER_SUPPORT
	CliLinkMapInit(pAd);
#endif

	pAd->flg_apcli_init = TRUE;

}


VOID ApCli_Remove(RTMP_ADAPTER *pAd)
{
	UINT index;
	struct wifi_dev *wdev;

	for (index = 0; index < MAX_APCLI_NUM; index++) {
		wdev = &pAd->ApCfg.ApCliTab[index].wdev;

		if (wdev->if_dev) {
			RtmpOSNetDevProtect(1);
			RtmpOSNetDevDetach(wdev->if_dev);
			RtmpOSNetDevProtect(0);
			wdev_deinit(pAd, wdev);
			RtmpOSNetDevFree(wdev->if_dev);
			/* Clear it as NULL to prevent latter access error. */
			pAd->ApCfg.ApCliTab[index].ApCliInit = FALSE;

			pAd->flg_apcli_init = FALSE;
			wdev->if_dev = NULL;
		}
	}
}

/*
* ApCli_Open
*/
INT apcli_inf_open(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	APCLI_STRUCT *pApCliEntry;


#if defined(CONFIG_FAST_NAT_SUPPORT) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))

	if (wf_drv_tbl.wf_fwd_probe_adapter)
		wf_drv_tbl.wf_fwd_probe_adapter(pAd);

	if (wf_ra_sw_nat_hook_rx_bkup != NULL)
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s:wf_ra_sw_nat_hook_rx_bkup:%p\n",
					 __func__, wf_ra_sw_nat_hook_rx_bkup));

	wf_ra_sw_nat_hook_rx_bkup = NULL;


#endif

#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */

	/* If AP channel is changed, the corresponding ApCli wdev channel is not updated */
	/* if ApCli interface is not up, sync apcli channel with binding radio channel */
	/*Enable this function by default (Don't keep under WH_EZ_SETUP)*/
	apcli_sync_wdev(pAd, wdev);
	pApCliEntry = &pAd->ApCfg.ApCliTab[wdev->func_idx];
	ApCliWaitIfDown(pApCliEntry);

	/* Security initial  */
	if (wdev->SecConfig.AKMMap == 0x0)
		SET_AKM_OPEN(wdev->SecConfig.AKMMap);

	if (wdev->SecConfig.PairwiseCipher == 0x0) {
		SET_CIPHER_NONE(wdev->SecConfig.PairwiseCipher);
		SET_CIPHER_NONE(wdev->SecConfig.GroupCipher);
	}

	if (wifi_sys_open(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() open fail!!!\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("APCLI inf up for apcli_%x(func_idx) OmacIdx=%d\n",
		wdev->func_idx, wdev->OmacIdx));

	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
	RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);

	BuildChannelList(pAd, wdev);
	RTMPSetPhyMode(pAd, wdev, wdev->PhyMode);
	RTMPUpdateRateInfo(wdev->PhyMode, &wdev->rate);

#ifdef WSC_INCLUDED
	WscUUIDInit(pAd, wdev->func_idx, TRUE);
#endif /* WSC_INCLUDED */

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_init(pAd, wdev->func_idx, FALSE);
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	NdisAllocateSpinLock(pAd, &pApCliEntry->SavedPMK_lock);
#endif


#ifndef APCLI_CFG80211_SUPPORT
	ApCliIfUp(pAd);
#endif /* APCLI_CFG80211_SUPPORT */

	{
		UCHAR ucBandIdx = 0;

		ucBandIdx = HcGetBandByWdev(&pAd->ApCfg.ApCliTab[wdev->func_idx].wdev);

#ifdef SINGLE_SKU_V2
#ifdef RF_LOCKDOWN

		/* Check RF lock Status */
		if (chip_check_rf_lock_down(pAd)) {
			pAd->CommonCfg.SKUenable[ucBandIdx] = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! SKUenable = 1!!\n", __func__));
		}

#endif /* RF_LOCKDOWN */

#if defined(MT7615) || defined(MT7622)
		/* enable/disable SKU via profile */
		TxPowerSKUCtrl(pAd, pAd->CommonCfg.SKUenable[ucBandIdx], ucBandIdx);

		/* enable/disable BF Backoff via profile */
		TxPowerBfBackoffCtrl(pAd, pAd->CommonCfg.BFBACKOFFenable[ucBandIdx], ucBandIdx);
#else
#endif /* defined(MT7615) || defined(MT7622) */
#endif /* SINGLE_SKU_V2*/
		/* enable/disable Power Percentage via profile */
		TxPowerPercentCtrl(pAd, pAd->CommonCfg.PERCENTAGEenable[ucBandIdx], ucBandIdx);

		/* Tx Power Percentage value via profile */
		TxPowerDropCtrl(pAd, pAd->CommonCfg.ucTxPowerPercentage[ucBandIdx], ucBandIdx);

	/* Config Tx CCK Stream */
		TxCCKStreamCtrl(pAd, pAd->CommonCfg.CCKTxStream[ucBandIdx], ucBandIdx);

#ifdef RF_LOCKDOWN

		/* Check RF lock Status */
		if (chip_check_rf_lock_down(pAd)) {
			pAd->CommonCfg.BFBACKOFFenable[ucBandIdx] = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: RF lock down!! BFBACKOFFenable = 1!!\n", __func__));
		}

#endif /* RF_LOCKDOWN */
#ifdef TX_POWER_CONTROL_SUPPORT
		/* config Power boost table via profile */
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_CCK_OFDM,
				pAd->CommonCfg.cPowerUpCckOfdm[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_HT20,
				pAd->CommonCfg.cPowerUpHt20[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_HT40,
				pAd->CommonCfg.cPowerUpHt40[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT20,
				pAd->CommonCfg.cPowerUpVht20[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT40,
				pAd->CommonCfg.cPowerUpVht40[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT80,
				pAd->CommonCfg.cPowerUpVht80[ucBandIdx]);
		TxPwrUpCtrl(pAd, ucBandIdx, POWER_UP_CATE_VHT160,
				pAd->CommonCfg.cPowerUpVht160[ucBandIdx]);
#endif /* TX_POWER_CONTROL_SUPPORT */
	}


	return TRUE;
}

/*
* ApCli_Close
*/
INT apcli_inf_close(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *pAd = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *pReptEntry;
	UCHAR CliIdx;
	RTMP_CHIP_CAP   *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif /* MAC_REPEATER_SUPPORT */
	APCLI_STRUCT *apcli_entry;


#ifdef GREENAP_SUPPORT
	/* This function will check and update allow status */
	if (greenap_check_when_if_down_up(pAd) == FALSE)
		return FALSE;
#endif /* GREENAP_SUPPORT */

	apcli_entry = &pAd->ApCfg.ApCliTab[wdev->func_idx];

#if defined(CONFIG_MAP_SUPPORT) && defined(A4_CONN)
	if (IS_MAP_ENABLE(pAd))
		map_a4_deinit(pAd, wdev->func_idx, FALSE);
#endif

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
	NdisFreeSpinLock(&apcli_entry->SavedPMK_lock);
#endif



#if defined(WPA_SUPPLICANT_SUPPORT) || defined(APCLI_CFG80211_SUPPORT)
	if (apcli_entry->wpa_supplicant_info.pWpaAssocIe) {
		os_free_mem(apcli_entry->wpa_supplicant_info.pWpaAssocIe);
		apcli_entry->wpa_supplicant_info.pWpaAssocIe = NULL;
		apcli_entry->wpa_supplicant_info.WpaAssocIeLen = 0;
	}
#endif /* defined(WPA_SUPPLICANT_SUPPORT) || defined(APCLI_CFG80211_SUPPORT) */

	/* send disconnect-req to sta State Machine. */
	if (apcli_entry->Enable) {
#ifdef MAC_REPEATER_SUPPORT

		if (pAd->ApCfg.bMACRepeaterEn) {
			for (CliIdx = 0; CliIdx < GET_MAX_REPEATER_ENTRY_NUM(cap); CliIdx++) {
				pReptEntry = &pAd->ApCfg.pRepeaterCliPool[CliIdx];

				if ((pReptEntry->CliEnable) && (pReptEntry->wdev == &apcli_entry->wdev)) {
					RTMP_OS_INIT_COMPLETION(&pReptEntry->free_ack);
					pReptEntry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
					MlmeEnqueue(pAd,
								APCLI_CTRL_STATE_MACHINE,
								APCLI_CTRL_DISCONNECT_REQ,
								0,
								NULL,
								(REPT_MLME_START_IDX + CliIdx));
					RTMP_MLME_HANDLER(pAd);
					ReptWaitLinkDown(pReptEntry);
				}
			}
		}

#endif /* MAC_REPEATER_SUPPORT */
		RTMP_OS_INIT_COMPLETION(&apcli_entry->linkdown_complete);
		apcli_entry->Disconnect_Sub_Reason = APCLI_DISCONNECT_SUB_REASON_NONE;
		MlmeEnqueue(pAd,
					APCLI_CTRL_STATE_MACHINE,
					APCLI_CTRL_DISCONNECT_REQ,
					0,
					NULL,
					wdev->func_idx);
		RTMP_MLME_HANDLER(pAd);
		ApCliWaitLinkDown(apcli_entry);
		ApCliWaitStateDisconnect(apcli_entry);
#ifdef APCLI_AUTO_CONNECT_SUPPORT
		pAd->ApCfg.ApCliAutoConnectRunning[wdev->func_idx] = FALSE;
#endif

#if defined(CONFIG_FAST_NAT_SUPPORT) && (defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE))
		if (wf_ra_sw_nat_hook_rx_bkup != NULL)
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:wf_ra_sw_nat_hook_rx_bkup:%p\n", __func__, wf_ra_sw_nat_hook_rx_bkup));

		wf_ra_sw_nat_hook_rx_bkup = NULL;

#endif


		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
				 ("(%s) ApCli interface[%d] startdown.\n", __func__, wdev->func_idx));
	}

	if (wifi_sys_close(wdev) != TRUE) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() close fail!!!\n", __func__));
		return FALSE;
	}

	/*send for ApOpen can know interface down is done*/
	ApCliIfDownComplete(apcli_entry);


	return TRUE;
}

#ifdef APCLI_AUTO_CONNECT_SUPPORT
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
BOOLEAN ApCliAutoConnectBWAdjust(
	IN RTMP_ADAPTER	*pAd,
	IN struct wifi_dev	*wdev,
	IN BSS_ENTRY * bss_entry)
{
	BOOLEAN bAdjust = FALSE;
	BOOLEAN bAdjust_by_channel = FALSE;
	BOOLEAN bAdjust_by_ht = FALSE;
	BOOLEAN bAdjust_by_vht = FALSE;
	UCHAR	orig_op_ht_bw;
#ifdef DOT11_VHT_AC
	UCHAR	orig_op_vht_bw;
	struct _RTMP_CHIP_CAP *cap = NULL;
#endif
	UCHAR	orig_ext_cha;

	if (pAd == NULL || wdev == NULL || bss_entry == NULL) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR,
				 ("(%s)  Error! entry is NULL.\n", __func__));
		return FALSE;
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
			 ("BW info of root AP (%s):\n", bss_entry->Ssid));
	orig_op_ht_bw = wlan_operate_get_ht_bw(wdev);
#ifdef DOT11_VHT_AC
	orig_op_vht_bw = wlan_operate_get_vht_bw(wdev);
#endif /*DOT11_VHT_AC*/
	orig_ext_cha = wlan_operate_get_ext_cha(wdev);

	if (wdev->channel != bss_entry->Channel) {
		bAdjust = TRUE;
		bAdjust_by_channel = TRUE;
	}

#ifdef DOT11_N_SUPPORT

	if (WMODE_CAP_N(wdev->PhyMode) && (bss_entry->AddHtInfoLen != 0)) {
		ADD_HTINFO *add_ht_info = &bss_entry->AddHtInfo.AddHtInfo;
		UCHAR op_ht_bw = wlan_operate_get_ht_bw(wdev);
		UCHAR cfg_ht_bw = wlan_config_get_ht_bw(wdev);
		UCHAR ext_cha = wlan_operate_get_ext_cha(wdev);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		UCHAR soft_ap_bw = wlan_operate_get_bw(&pAd->ApCfg.MBSSID[0].wdev);
#endif

		if (!bAdjust &&
			((ext_cha != add_ht_info->ExtChanOffset) ||
#ifdef BW_VENDOR10_CUSTOM_FEATURE
			/* Soft AP BW : Sync Required */
			(soft_ap_bw != add_ht_info->RecomWidth) ||
#endif
			(op_ht_bw != add_ht_info->RecomWidth)))
			bAdjust = TRUE;

		if (bAdjust) {
		switch (add_ht_info->RecomWidth) { /* peer side vht bw */
			case BW_20:
				if (op_ht_bw == BW_40) {
					wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth, EXTCHA_NONE);
					bAdjust_by_ht = TRUE;

#ifdef BW_VENDOR10_CUSTOM_FEATURE
					/* Sync new BW & Ext Channel for Soft AP */
					if (IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_4020_DOWN_ENBL)) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							("[%s] Enter 4020 HT Sync\n", __func__));
						wdev_sync_ht_bw(pAd, wdev, add_ht_info);
					}
#endif
				}

				break;
			case BW_40:
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (op_ht_bw == BW_20 || (soft_ap_bw == BW_20)) {
#else
				if (op_ht_bw == BW_20) {
#endif
#ifdef BT_APCLI_SUPPORT
					if (pAd->ApCfg.ApCliAutoBWBTSupport == TRUE) {
						/*set to config extension channel/bw to let ap use new configuration*/
						UCHAR mbss_idx = 0;
						/*Moving both AP and CLI to 40Mhz since RootAP is working in 40Mhz */
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;

							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;

							if (HcGetBandByWdev(mbss_wdev) ==
									HcGetBandByWdev(wdev)) {
								wlan_config_set_ht_bw(mbss_wdev,
										add_ht_info->RecomWidth);
								wlan_config_set_ext_cha(mbss_wdev,
										add_ht_info->ExtChanOffset);
							}
						}
						/*set Config BW of CLI to 40Mhz*/
						wlan_config_set_ht_bw(wdev, add_ht_info->RecomWidth);
						wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth,
								add_ht_info->ExtChanOffset);
						wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
						bAdjust_by_ht = TRUE;
					}
#endif
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					if (cfg_ht_bw == BW_40 && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							("[%s] Enter 2040 HT Sync\n", __func__));
						/*set Config BW of CLI to 40Mhz*/
						bAdjust_by_ht = TRUE;
					}
#endif
				} else {
					if (cfg_ht_bw == BW_40) {
						/* Control should not reach here. BW sync missing */
						UCHAR mbss_idx = 0;
						wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);
						bAdjust_by_ht = TRUE;

#ifdef BW_VENDOR10_CUSTOM_FEATURE
						if (IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL) == FALSE)
							break;
#endif
						for (mbss_idx = 0; mbss_idx < pAd->ApCfg.BssidNum; mbss_idx++) {
							struct wifi_dev *mbss_wdev;
							mbss_wdev = &pAd->ApCfg.MBSSID[mbss_idx].wdev;
							if (HcGetBandByWdev(mbss_wdev) == HcGetBandByWdev(wdev)) {
								wlan_config_set_ext_cha(mbss_wdev, add_ht_info->ExtChanOffset);
							}
						}
					}
				}
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				if (bAdjust_by_ht && IS_SYNC_BW_POLICY_VALID(pAd, TRUE, HT_2040_UP_ENBL)) {
					/* Soft AP Op BW 20 M / Root AP Link Up when Soft AP is Down */
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						("[%s] Enter 2040 HT Sync\n", __func__));

					/*set Config BW of CLI to 40Mhz*/
					wlan_config_set_ht_bw(wdev, add_ht_info->RecomWidth);
					wlan_operate_set_ht_bw(wdev, add_ht_info->RecomWidth,
							add_ht_info->ExtChanOffset);
					wlan_config_set_ext_cha(wdev, add_ht_info->ExtChanOffset);

					wdev_sync_ht_bw(pAd, wdev, add_ht_info);
				}
#endif
				break;
			}
		}
	}

#endif /* DOT11_N_SUPPORT */


#ifdef DOT11_VHT_AC
	cap = hc_get_chip_cap(pAd->hdev_ctrl);
	if (WMODE_CAP_AC(wdev->PhyMode) && IS_CAP_BW160(cap) &&
		(bss_entry->vht_cap_len != 0) && (bss_entry->vht_op_len != 0)) {
#ifdef BW_VENDOR10_CUSTOM_FEATURE
		BOOLEAN bDown80_2040 = FALSE, bDown160_80 = FALSE;
#endif
		BOOLEAN bResetVHTBw = FALSE, bDownBW = FALSE;
		UCHAR bw = VHT_BW_2040;
		VHT_OP_INFO *vht_op = &bss_entry->vht_op_ie.vht_op_info;
		UCHAR op_vht_bw = wlan_operate_get_vht_bw(wdev);
		UCHAR cfg_vht_bw = wlan_config_get_vht_bw(wdev);
		UCHAR BandIdx = HcGetBandByWdev(wdev);
		CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		bw = check_vht_op_bw(vht_op);

		if (!bAdjust &&
			(bw != op_vht_bw))
			bAdjust = TRUE;

		if (bAdjust) {
			switch (bw) { /* peer side vht bw */
			case VHT_BW_2040:
				if (cfg_vht_bw > VHT_BW_2040) {
					bResetVHTBw = TRUE;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					if (op_vht_bw == VHT_BW_80 && IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_80_2040_DOWN_CHK)) {
						MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
							("[%s] Enter 802040 HT Sync\n", __func__));
						bDown80_2040 = TRUE;
					}
#endif
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
				}

				break;

			case VHT_BW_80:
				if (cfg_vht_bw > VHT_BW_80) {
					bResetVHTBw = TRUE;
					bDownBW = TRUE;
					bAdjust_by_vht = TRUE;
#ifdef BW_VENDOR10_CUSTOM_FEATURE
					if (op_vht_bw == VHT_BW_160 && IS_SYNC_BW_POLICY_VALID(pAd, FALSE, VHT_160_80_DOWN_CHK))
						bDown160_80 = TRUE;
#endif
				}

				break;

			case VHT_BW_160:
				if (cfg_vht_bw == VHT_BW_160) {
					bAdjust_by_vht = TRUE;
					bResetVHTBw = 1;
				}

				break;

			case VHT_BW_8080:
				if (cfg_vht_bw == VHT_BW_8080) {
					wlan_operate_set_cen_ch_2(wdev, vht_op->center_freq_2);
					bResetVHTBw = 1;
					bAdjust_by_vht = TRUE;
				}

				break;
			}
		}

		if (bResetVHTBw) {
			INT Idx = -1;
			BOOLEAN bMatch = FALSE;

			for (Idx = 0; Idx < pChCtrl->ChListNum; Idx++) {
				if (bss_entry->Channel == pChCtrl->ChList[Idx].Channel) {
					bMatch = TRUE;
					break;
				}
			}

			if (bMatch && (Idx < MAX_NUM_OF_CHANNELS)) {
				if (bDownBW) {
					MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_OFF,
							 ("(%s): Follow BW info of root AP (%s) from vht_bw = %d to %d. (MAX=%d)\n",
							  __func__, bss_entry->Ssid,
							  op_vht_bw, bw,
							  cfg_vht_bw));
					wlan_operate_set_vht_bw(wdev, bw);
				} else if (!bDownBW && (pChCtrl->ChList[Idx].Flags & CHANNEL_80M_CAP))
					wlan_operate_set_vht_bw(wdev, cfg_vht_bw);

				wlan_operate_set_cen_ch_2(wdev, vht_op->center_freq_2);
#ifdef BW_VENDOR10_CUSTOM_FEATURE
				/* Sync new BW & Central Channel for Soft AP */
				if (bDown80_2040 || bDown160_80)
					wdev_sync_vht_bw(pAd, wdev,
					((bDownBW) ? (bw) : ((bw >= VHT_BW_160) ? (VHT_BW_160) : (cfg_vht_bw))), vht_op->center_freq_2);
#endif
			}
		}
	}

#endif /* DOT11_VHT_AC */
	bAdjust = FALSE;

	if (bAdjust_by_channel == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_ht == TRUE)
		bAdjust = TRUE;

	if (bAdjust_by_vht == TRUE)
		bAdjust = TRUE;

	if (bAdjust) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:Adjust (%d %d %d)\n\r", __func__,
				 bAdjust_by_channel, bAdjust_by_ht, bAdjust_by_vht));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:HT BW:%d to %d. MAX(%d)\n\r", __func__,
				 orig_op_ht_bw, wlan_operate_get_ht_bw(wdev), wlan_config_get_ht_bw(wdev)));
#ifdef DOT11_VHT_AC
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:VHT BW:%d to %d. MAX(%d)\n\r", __func__,
				 orig_op_vht_bw, wlan_operate_get_vht_bw(wdev), wlan_config_get_vht_bw(wdev)));
#endif /*DOT11_VHT_AC*/
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("%s:EXT CH:%d to %d\n\r", __func__,
				 orig_ext_cha, wlan_operate_get_ext_cha(wdev)));
	}

	return bAdjust;
}
#endif /* APCLI_AUTO_BW_TMP */

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
	IN  PRTMP_ADAPTER   pAd,
	IN struct wifi_dev *wdev)
{
	UCHAR			ifIdx, CfgSsidLen, entryIdx;
	RTMP_STRING *pCfgSsid;
	BSS_TABLE		*pScanTab, *pSsidBssTab;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_ioctl_if;
	INT old_ioctl_if_type;
	UCHAR Channel = 0;

	#ifdef APCLI_OWE_SUPPORT
	UCHAR switch_to_owe_channel = 0;
	BOOLEAN bupdate_owe_trans = FALSE;
	BSS_TABLE	*powe_bss_tab = NULL;
	APCLI_STRUCT *papcli_entry = NULL;
	#endif


	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliAutoConnectExec()\n"));

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return FALSE;

	if (ifIdx >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error  ifIdx=%d\n", ifIdx));
		return FALSE;
	}

	if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag != TRUE)
		return FALSE;

	CfgSsidLen = pAd->ApCfg.ApCliTab[ifIdx].CfgSsidLen;
	pCfgSsid = pAd->ApCfg.ApCliTab[ifIdx].CfgSsid;
	pScanTab = &pAd->ScanTab;
	pSsidBssTab = &pAd->ApCfg.ApCliTab[ifIdx].MlmeAux.SsidBssTab;
	pSsidBssTab->BssNr = 0;



#ifdef APCLI_OWE_SUPPORT
	papcli_entry = &pAd->ApCfg.ApCliTab[ifIdx];
	if (IS_AKM_OWE(wdev->SecConfig.AKMMap)) {

		powe_bss_tab = &papcli_entry->MlmeAux.owe_bss_tab;
		powe_bss_tab->BssNr = 0;


		BssTableInit(powe_bss_tab);

		apcli_reset_owe_parameters(pAd, ifIdx);


		/*
			Find out APs with the OWE transition IE and store in owe_bss_tab*
		*/
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
	for (entryIdx = 0; entryIdx < pScanTab->BssNr; entryIdx++) {
		BSS_ENTRY *pBssEntry = &pScanTab->BssEntry[entryIdx];

		if (pBssEntry->Channel == 0)
			break;

		if (NdisEqualMemory(pCfgSsid, pBssEntry->Ssid, CfgSsidLen) &&
			pBssEntry->SsidLen &&
			(pBssEntry->SsidLen == CfgSsidLen) &&
#if defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT)
/*
	double check the SSID in the same band could be in candidate list
*/
							(((pBssEntry->Channel > 14) && WMODE_CAP_5G(wdev->PhyMode)) || ((pBssEntry->Channel <= 14) && WMODE_CAP_2G(wdev->PhyMode))) &&
#endif /* defined(DBDC_MODE) && defined(DOT11K_RRM_SUPPORT) */
			(pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
			if ((((wdev->SecConfig.AKMMap & pBssEntry->AKMMap) != 0) ||
				 (IS_AKM_AUTOSWITCH(wdev->SecConfig.AKMMap) && IS_AKM_SHARED(pBssEntry->AKMMap))
#ifdef APCLI_SAE_SUPPORT
						|| (IS_AKM_WPA2PSK_ONLY(pBssEntry->AKMMap) && IS_AKM_WPA3PSK_ONLY(wdev->SecConfig.AKMMap))
#endif
/* While using OWE allow ApCli AutoConnect Scan to copy entries of OPEN BSS also*/
#ifdef APCLI_OWE_SUPPORT
						|| (IS_AKM_OPEN(pBssEntry->AKMMap) && IS_AKM_OWE(wdev->SecConfig.AKMMap))
#endif
							) && ((wdev->SecConfig.PairwiseCipher & pBssEntry->PairwiseCipher)
#ifdef APCLI_OWE_SUPPORT
									|| (IS_CIPHER_CCMP128(wdev->SecConfig.PairwiseCipher) && IS_CIPHER_NONE(pBssEntry->PairwiseCipher))
#endif
									) != 0) {
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("Found desired ssid in Entry %2d:\n", entryIdx));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("I/F(apcli%d) ApCliAutoConnectExec:(Len=%d,Ssid=%s, Channel=%d, Rssi=%d)\n",
						  ifIdx, pBssEntry->SsidLen, pBssEntry->Ssid,
						  pBssEntry->Channel, pBssEntry->Rssi));
				MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE,
						 ("I/F(apcli%d) ApCliAutoConnectExec::(AuthMode=%s, EncrypType=%s)\n", ifIdx,
						  GetAuthMode(pBssEntry->AuthMode),
						  GetEncryptType(pBssEntry->WepStatus)));
				NdisMoveMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr++],
							   pBssEntry, sizeof(BSS_ENTRY));
			}
		}
	}

	if (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)
		NdisZeroMemory(&pSsidBssTab->BssEntry[pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	/*
		Sort by Rssi in the increasing order, and connect to
		the last entry (strongest Rssi)
	*/
	BssTableSortByRssi(pSsidBssTab, TRUE);

	if ((pSsidBssTab->BssNr == 0)) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("No match entry.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning[ifIdx] = FALSE;
	}
	else if (pSsidBssTab->BssNr > 0 &&
			 pSsidBssTab->BssNr <= MAX_LEN_OF_BSS_TABLE) {
		/*
			Switch to the channel of the candidate AP
		*/
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj = FALSE;
#endif


#ifdef APCLI_OWE_SUPPORT

	if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) &&
		(IS_AKM_OPEN(pBssEntry->AKMMap) && IS_CIPHER_NONE(pBssEntry->PairwiseCipher))) {

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

						UCHAR BandIdx = HcGetBandByWdev(wdev);
						CHANNEL_CTRL *pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

						/*Single Chip Dual band 5g only case*/

						if (MTChGrpChannelChk(pChCtrl, pair_ch)) {
							if ((pair_ch != pBssEntry->Channel) || (papcli_entry->wdev.channel != pair_ch)) {
							/*Channel is valid in the current channel group*/


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
				} else  { 				/*Same Channel send directed probe request to OWE BSS*/
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
					CLEAR_SEC_AKM(papcli_entry->MlmeAux.AKMMap);
					CLEAR_CIPHER(papcli_entry->MlmeAux.PairwiseCipher);
					CLEAR_CIPHER(papcli_entry->MlmeAux.GroupCipher);

					/*Delete the Open Bss entry from Scan table because apcli does not ageout scan tab entries*/
					BssTableDeleteEntry(pScanTab, pBssEntry->Bssid, pBssEntry->Channel);

				}

			}
	}
#endif

#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
#ifdef APCLI_OWE_SUPPORT
	if ((IS_AKM_OWE(wdev->SecConfig.AKMMap)) && (pBssEntry->owe_trans_ie_len > 0)) {
		if (switch_to_owe_channel > 0)
			bw_adj = TRUE;
	} else
#endif
		bw_adj = (ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry)
				|| (!IS_INVALID_HT_SECURITY(pBssEntry->PairwiseCipher)));
		if (bw_adj)
#endif /* APCLI_AUTO_BW_TMP */
		{


#ifdef APCLI_OWE_SUPPORT
			if (switch_to_owe_channel)
				Channel = switch_to_owe_channel;
			else
#endif
			{
				Channel = pBssEntry->Channel;
				pAd->ApCfg.ApCliAutoBWAdjustCnt[ifIdx]++;
			}
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n", pBssEntry->Channel));
			rtmp_set_channel(pAd, wdev, Channel);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("Error! Out of table range: (BssNr=%d).\n", pSsidBssTab->BssNr));
		RtmpOSNetDevProtect(1);
		old_ioctl_if = pObj->ioctl_if;
		old_ioctl_if_type = pObj->ioctl_if_type;
		pObj->ioctl_if = ifIdx;
		pObj->ioctl_if_type = INT_APCLI;
		Set_ApCli_Enable_Proc(pAd, "1");
		pObj->ioctl_if = old_ioctl_if;
		pObj->ioctl_if_type = old_ioctl_if_type;
		RtmpOSNetDevProtect(0);
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
		return FALSE;
	}
	RtmpOSNetDevProtect(1);
	old_ioctl_if = pObj->ioctl_if;
	old_ioctl_if_type = pObj->ioctl_if_type;
	pObj->ioctl_if = ifIdx;
	pObj->ioctl_if_type = INT_APCLI;
	Set_ApCli_Enable_Proc(pAd, "1");
	pObj->ioctl_if = old_ioctl_if;
	pObj->ioctl_if_type = old_ioctl_if_type;
	RtmpOSNetDevProtect(0);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("<--- ApCliAutoConnectExec()\n"));
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
	IN struct wifi_dev *wdev)
{
	BSS_TABLE		*pSsidBssTab;
	PAPCLI_STRUCT	pApCliEntry;
	UCHAR			lastEntryIdx, ifIdx;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	INT old_ioctl_if;
	INT old_ioctl_if_type;

	if (pAd->ScanCtrl.PartialScan.bScanning == TRUE)
		return;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));

	if (wdev)
		ifIdx = wdev->func_idx;
	else
		return;

	if (ifIdx >= MAX_APCLI_NUM) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Error  ifIdx=%d\n", ifIdx));
		return;
	}

	if (pAd->ApCfg.ApCliTab[ifIdx].AutoConnectFlag != TRUE)
		return;

	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIdx];
	pSsidBssTab = &pApCliEntry->MlmeAux.SsidBssTab;

	if (pSsidBssTab->BssNr == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("No Bss\n"));
		pAd->ApCfg.ApCliAutoConnectRunning[ifIdx] = FALSE;
		return;
	}

	/*
		delete (zero) the previous connected-failled entry and always
		connect to the last entry in talbe until the talbe is empty.
	*/
	NdisZeroMemory(&pSsidBssTab->BssEntry[--pSsidBssTab->BssNr], sizeof(BSS_ENTRY));
	lastEntryIdx = pSsidBssTab->BssNr - 1;

	if ((pSsidBssTab->BssNr > 0) && (pSsidBssTab->BssNr < MAX_LEN_OF_BSS_TABLE)) {
		BSS_ENTRY *pBssEntry = &pSsidBssTab->BssEntry[pSsidBssTab->BssNr - 1];
#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
		BOOLEAN bw_adj;

		bw_adj = ApCliAutoConnectBWAdjust(pAd, wdev, pBssEntry);
		if (bw_adj)
#endif /* APCLI_AUTO_BW_TMP */
		{
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("Switch to channel :%d\n", pBssEntry->Channel));
			rtmp_set_channel(pAd, wdev, pBssEntry->Channel);
		}
	} else {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("No candidate AP, the process is about to stop.\n"));
		pAd->ApCfg.ApCliAutoConnectRunning[ifIdx] = FALSE;
	}
	RtmpOSNetDevProtect(1);
	old_ioctl_if = pObj->ioctl_if;
	old_ioctl_if_type = pObj->ioctl_if_type;
	pObj->ioctl_if = ifIdx;
	pObj->ioctl_if_type = INT_APCLI;
	Set_ApCli_Enable_Proc(pAd, "1");
	pObj->ioctl_if = old_ioctl_if;
	pObj->ioctl_if_type = old_ioctl_if_type;
	RtmpOSNetDevProtect(0);
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("---> ApCliSwitchCandidateAP()\n"));
}
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

VOID ApCliRTMPReportMicError(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR uniCastKey,
	IN INT ifIndex)
{
	ULONG	Now;
	PAPCLI_STRUCT pApCliEntry = NULL;

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (" ApCliRTMPReportMicError <---\n"));
	pApCliEntry = &pAd->ApCfg.ApCliTab[ifIndex];
	/* Record Last MIC error time and count */
	NdisGetSystemUpTime(&Now);

	if (pAd->ApCfg.ApCliTab[ifIndex].MicErrCnt == 0) {
		pAd->ApCfg.ApCliTab[ifIndex].MicErrCnt++;
		pAd->ApCfg.ApCliTab[ifIndex].LastMicErrorTime = Now;
		NdisZeroMemory(pAd->ApCfg.ApCliTab[ifIndex].ReplayCounter, 8);
	} else if (pAd->ApCfg.ApCliTab[ifIndex].MicErrCnt == 1) {
		if ((pAd->ApCfg.ApCliTab[ifIndex].LastMicErrorTime + (60 * OS_HZ)) < Now) {
			/* Update Last MIC error time, this did not violate two MIC errors within 60 seconds */
			pAd->ApCfg.ApCliTab[ifIndex].LastMicErrorTime = Now;
		} else {
			/* RTMPSendWirelessEvent(pAd, IW_COUNTER_MEASURES_EVENT_FLAG, pAd->MacTab.Content[BSSID_WCID].Addr, BSS0, 0); */
			pAd->ApCfg.ApCliTab[ifIndex].LastMicErrorTime = Now;
			/* Violate MIC error counts, MIC countermeasures kicks in */
			pAd->ApCfg.ApCliTab[ifIndex].MicErrCnt++;
		}
	} else {
		/* MIC error count >= 2 */
		/* This should not happen */
		;
	}

	MlmeEnqueue(pAd, APCLI_CTRL_STATE_MACHINE, APCLI_MIC_FAILURE_REPORT_FRAME, 1, &uniCastKey, ifIndex);

	if (pAd->ApCfg.ApCliTab[ifIndex].MicErrCnt == 2) {
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (" MIC Error count = 2 Trigger Block timer....\n"));
		MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, (" pAd->ApCfg.ApCliTab[%d].LastMicErrorTime = %ld\n", ifIndex,
				 pAd->ApCfg.ApCliTab[ifIndex].LastMicErrorTime));
#ifdef APCLI_CERT_SUPPORT

		if (pAd->bApCliCertTest == TRUE)
			RTMPSetTimer(&pApCliEntry->MlmeAux.WpaDisassocAndBlockAssocTimer, 100);

#endif /* APCLI_CERT_SUPPORT */
	}

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_TRACE, ("ApCliRTMPReportMicError --->\n"));
}

#ifdef DOT11W_PMF_SUPPORT
/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;
	struct wifi_dev *wdev = NULL;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pPmfCfg = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.SecConfig.PmfCfg;
	wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;

	if (!pPmfCfg || !wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p, wdev=%p\n",
				 __func__, pPmfCfg, wdev));
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_MFPC = TRUE;
	else {
		pPmfCfg->Desired_MFPC = FALSE;
		pPmfCfg->MFPC = FALSE;
		pPmfCfg->MFPR = FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired MFPC=%d\n",
			 __func__, pPmfCfg->Desired_MFPC));

	if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
#ifdef APCLI_SAE_SUPPORT
|| IS_AKM_WPA3PSK_Entry(wdev)
#endif
#ifdef APCLI_OWE_SUPPORT
				|| IS_AKM_OWE_Entry(wdev)
#endif
)
		&& IS_CIPHER_AES_Entry(wdev)) {
		pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

		if (pPmfCfg->Desired_MFPC) {
			pPmfCfg->MFPC = TRUE;
			pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

			if (pPmfCfg->MFPR)
				pPmfCfg->PMFSHA256 = TRUE;
		}
	} else if (pPmfCfg->Desired_MFPC)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK/WPA3PSK AES\n", __func__));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
			 __func__,
			 pPmfCfg->MFPC,
			 pPmfCfg->MFPR,
			 pPmfCfg->PMFSHA256));
	return TRUE;
}

/* chane the cmd depend on security mode first, and update to run time flag*/
INT Set_ApCliPMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;
	struct wifi_dev *wdev = NULL;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pPmfCfg = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.SecConfig.PmfCfg;
	wdev = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev;

	if (!pPmfCfg || !wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p, wdev=%p\n",
				 __func__, pPmfCfg, wdev));
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_MFPR = TRUE;
	else {
		pPmfCfg->Desired_MFPR = FALSE;
		/* only close the MFPR */
		pPmfCfg->MFPR = FALSE;
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired MFPR=%d\n",
			 __func__, pPmfCfg->Desired_MFPR));

	if ((IS_AKM_WPA2_Entry(wdev) || IS_AKM_WPA2PSK_Entry(wdev)
#ifdef APCLI_SAE_SUPPORT
|| IS_AKM_WPA3PSK_Entry(wdev)
#endif
#ifdef APCLI_OWE_SUPPORT
			|| IS_AKM_OWE_Entry(wdev)
#endif
)
		&& IS_CIPHER_AES_Entry(wdev)) {
		pPmfCfg->PMFSHA256 = pPmfCfg->Desired_PMFSHA256;

		if (pPmfCfg->Desired_MFPC) {
			pPmfCfg->MFPC = TRUE;
			pPmfCfg->MFPR = pPmfCfg->Desired_MFPR;

			if (pPmfCfg->MFPR)
				pPmfCfg->PMFSHA256 = TRUE;
		}
	} else if (pPmfCfg->Desired_MFPC)
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Security is not WPA2/WPA2PSK/WPA3PSK AES\n", __func__));

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: MFPC=%d, MFPR=%d, SHA256=%d\n",
			 __func__, pPmfCfg->MFPC,
			 pPmfCfg->MFPR,
			 pPmfCfg->PMFSHA256));
	return TRUE;
}

INT Set_ApCliPMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *arg)
{
	POS_COOKIE pObj;
	PMF_CFG *pPmfCfg = NULL;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pPmfCfg = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].wdev.SecConfig.PmfCfg;

	if (!pPmfCfg) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: pPmfCfg=%p\n",
				 __func__, pPmfCfg));
		return FALSE;
	}

	if (os_str_tol(arg, 0, 10))
		pPmfCfg->Desired_PMFSHA256 = TRUE;
	else
		pPmfCfg->Desired_PMFSHA256 = FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[PMF]%s:: Desired PMFSHA256=%d\n",
			 __func__, pPmfCfg->Desired_PMFSHA256));
	return TRUE;
}
#endif /* DOT11W_PMF_SUPPORT */
#ifdef APCLI_SAE_SUPPORT
INT set_apcli_sae_group_proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR *pSaeCfgGroup = NULL;
	UCHAR group = 0;

	if (strlen(arg) == 0)
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pSaeCfgGroup = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].sae_cfg_group;


	group = os_str_tol(arg, 0, 10);

	if ((group == 19) || (group == 20)) {
		*pSaeCfgGroup = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[SAE]%s:: Set group=%d \n",
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
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR group = 0;
	UCHAR *pcurr_group = NULL;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pcurr_group = &pAd->ApCfg.ApCliTab[pObj->ioctl_if].curr_owe_group;


	group = os_str_tol(arg, 0, 10);
/*OWE-currently allowing configuration of groups 19(mandatory) and 20(optional) */
	if ((group == 19) || (group == 20)) {
		*pcurr_group = (UCHAR) group;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("[OWE]%s:: Set group=%d \n",
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
	IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	UCHAR action = 0;

	if (0 == strlen(arg))
		return FALSE;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (pObj->ioctl_if_type != INT_APCLI)
		return FALSE;


	action = os_str_tol(arg, 0, 10);

/*Delete all pmkid list associated with this  ApCli Interface*/
	if (action == 1) {
		apcli_delete_pmkid_cache_all(pAd, pObj->ioctl_if);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: Delete PMKID list (%d)  \n",
				 __func__, action));

	}
	return TRUE;
}
#endif


VOID apcli_dync_txop_alg(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT tx_tp,
	UINT rx_tp)
{
#define COND3_COOL_DOWN_TIME 240

	if (!pAd || !wdev) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s:: pAd or wdev is NULL!\n",
				  __func__));
	} else {
		INT i;
		BOOLEAN cond[NUM_OF_APCLI_TXOP_COND] = {FALSE};
		UINT16 cond_txop_level[NUM_OF_APCLI_TXOP_COND] = {0};
		UINT16 cond_thrd[NUM_OF_APCLI_TXOP_COND] = {0};
		UINT16 txop_level = TXOP_0;
		UINT16 current_txop_level = TXOP_0;
		BOOLEAN apcli_txop_en = FALSE;
		APCLI_STRUCT *apcli_entry = NULL;

		cond_txop_level[1] = TXOP_30;
		cond_txop_level[2] = TXOP_FE;
		cond_txop_level[3] = TXOP_80;
		cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD;
		cond_thrd[2] = TX_MODE_TP_CHECK;
		current_txop_level = wdev->bss_info_argument.txop_level[PRIO_APCLI_REPEATER];
		apcli_entry = &pAd->ApCfg.ApCliTab[wdev->func_idx];

		/* if cond_1 is taking effect, adjust the threshold to prevent instability */
		if (current_txop_level == cond_txop_level[1]) {
			/* Adjust cond_thrd[1] */
			if (apcli_entry->dync_txop_histogram[1] >= 4) {
				UINT32 tolerance_adjust_factor = 10;
				UINT32 tolerance_adjust_value = 0;

				tolerance_adjust_value = TOLERANCE_OF_TP_THRESHOLD +
										 (apcli_entry->dync_txop_histogram[1]*tolerance_adjust_factor);

				if (tolerance_adjust_value > 150)
					tolerance_adjust_value = 150;

				cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD - tolerance_adjust_value;
			} else
				cond_thrd[1] = TP_PEEK_BOUND_THRESHOLD - TOLERANCE_OF_TP_THRESHOLD;

			/* Check if t.p. has degrade right after apply cond1 */
			if (tx_tp <= (TP_PEEK_BOUND_THRESHOLD - TOLERANCE_OF_TP_THRESHOLD) &&
				apcli_entry->dync_txop_histogram[1] < 4) {
				/* If t.p. is bad right after cond1, we trigger cond3 to recover old txop */
				cond[3] = TRUE;
			}
		} else if (current_txop_level == cond_txop_level[2]) {
			/* if cond_2 is taking effect, adjust the threshold to prevent instability */
			cond_thrd[2] = TX_MODE_TP_CHECK - TOLERANCE_OF_TP_THRESHOLD;
		}

		if (tx_tp > cond_thrd[1])
			cond[1] = TRUE;
		else if (tx_tp > cond_thrd[2]  && WMODE_CAP_2G(wdev->PhyMode)) {
			/* We don't check "divided by 0" because the "if condition" already do that */
			UINT tx_ratio = (tx_tp*100)/(tx_tp + rx_tp);

			if (tx_ratio > TX_MODE_RATIO_THRESHOLD)
				cond[2] = TRUE;
		}

		if (apcli_entry->dync_txop_histogram[3] != 0) {
			cond[3] = TRUE;
			txop_level = cond_txop_level[3];
			apcli_txop_en = TRUE;

			if (tx_tp < TP_PEEK_BOUND_THRESHOLD) {
				/* If cond3 triggered but t.p cannot keep high, we raise the decade rate */
				UINT8 cond3_decade_factor = 0;
				UINT32 cond3_accumulate_value = 0;

				cond[4] = TRUE;
				cond3_decade_factor = (1 << apcli_entry->dync_txop_histogram[4]); /* exponential decade */
				cond3_accumulate_value = apcli_entry->dync_txop_histogram[3] + cond3_decade_factor;
				apcli_entry->dync_txop_histogram[3] =
					(cond3_accumulate_value > COND3_COOL_DOWN_TIME) ?
					(COND3_COOL_DOWN_TIME):cond3_accumulate_value;
			}
		} else if (cond[1] == TRUE) {
			txop_level = cond_txop_level[1];
			apcli_txop_en = TRUE;
		} else if (cond[2] == TRUE) {
			txop_level = cond_txop_level[2];
			apcli_txop_en = TRUE;
		} else {
			txop_level = TXOP_0;
			apcli_txop_en = FALSE;
		}

		if (txop_level != current_txop_level) {
			if (apcli_txop_en == TRUE)
				enable_tx_burst(pAd, wdev, AC_BE, PRIO_APCLI_REPEATER, txop_level);
			else
				disable_tx_burst(pAd, wdev, AC_BE, PRIO_APCLI_REPEATER, txop_level);
		}

		/* update histogram */
		for (i = 0; i < NUM_OF_APCLI_TXOP_COND; i++) {
			if (cond[i] == TRUE)
				apcli_entry->dync_txop_histogram[i]++;
			else
				apcli_entry->dync_txop_histogram[i] = 0;
		}

		/* clear histogram */
		if (apcli_entry->dync_txop_histogram[3] > COND3_COOL_DOWN_TIME)
			apcli_entry->dync_txop_histogram[3] = 0;
	}
}

/*
* system layer api for APCLI
*/
INT apcli_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
	struct _APCLI_STRUCT *apcli = &ad->ApCfg.ApCliTab[wdev->func_idx];

	if (WDEV_BSS_STATE(wdev) == BSS_INIT) {
		apcli->Valid = TRUE;
		MSDU_FORBID_CLEAR(wdev, MSDU_FORBID_CONNECTION_NOT_READY);
		wdev->PortSecured = WPA_802_1X_PORT_SECURED;

		if (wifi_sys_linkup(wdev, entry) != TRUE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s():link up fail!\n", __func__));
		}
	}

	return TRUE;
}

#ifdef ROAMING_ENHANCE_SUPPORT
#ifndef ETH_HDR_LEN
#define ETH_HDR_LEN 14 /* dstMac(6) + srcMac(6) + protoType(2) */
#endif

#ifndef VLAN_ETH_HDR_LEN
#define VLAN_ETH_HDR_LEN (ETH_HDR_LEN + 4) /* 4 for h_vlan_TCI and h_vlan_encapsulated_proto */
#endif

#ifndef IP_HDR_SRC_OFFSET
#define IP_HDR_SRC_OFFSET 12 /* shift 12 for IP header len. */
#endif

BOOLEAN IsApCliLinkUp(IN PRTMP_ADAPTER pAd)
{
	if ((pAd->ApCfg.ApCliInfRunned > 0)
#ifdef CONFIG_WIFI_PKT_FWD
		|| ((wf_drv_tbl.wf_fwd_needed_hook != NULL) && (wf_drv_tbl.wf_fwd_needed_hook() == TRUE))
#endif /* CONFIG_WIFI_PKT_FWD */
	)
		return TRUE;

	return FALSE;
}

BOOLEAN ApCliDoRoamingRefresh(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PNDIS_PACKET pRxPacket,
	IN struct wifi_dev *wdev)
{
	UCHAR *pPktHdr, *pLayerHdr;
	UINT16 ProtoType;

	if (!pRxPacket || !wdev)
		return FALSE;

	/* Get the upper layer protocol type of this 802.3 pkt */
	pPktHdr = GET_OS_PKT_DATAPTR(pRxPacket);
	ProtoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + (ETH_HDR_LEN - 2))));
	if (IS_VLAN_PACKET(pPktHdr))
		pLayerHdr = (pPktHdr + VLAN_ETH_HDR_LEN);
	else
		pLayerHdr = (pPktHdr + ETH_HDR_LEN);

	if (ProtoType == ETH_P_ARP) {
		pEntry->bRoamingRefreshDone = TRUE;
		return TRUE;
	} else if (ProtoType == ETH_P_IP) {
		UINT32 SrcIP = 0;
		PNDIS_PACKET pPacket = NULL;

		NdisMoveMemory(&SrcIP, (pLayerHdr + IP_HDR_SRC_OFFSET), 4);
		if (SrcIP != 0) {
			pPacket = (PNDIS_PACKET)arp_create(ARPOP_REQUEST, ETH_P_ARP, SrcIP, wdev->if_dev,
							SrcIP, BROADCAST_ADDR, pEntry->Addr, BROADCAST_ADDR);
			if (pPacket != NULL) {
#ifdef CONFIG_WIFI_PKT_FWD
				set_wf_fwd_cb(pAd, pPacket, wdev);
#endif /* CONFIG_WIFI_PKT_FWD */
				RtmpOsPktProtocolAssign(pPacket);
				RtmpOsPktRcvHandle(pPacket);
				pEntry->bRoamingRefreshDone = TRUE;
				return TRUE;
			}
		}
	}

	return FALSE;
}
#endif /* ROAMING_ENHANCE_SUPPORT */

#if defined(APCLI_SAE_SUPPORT) || defined(APCLI_OWE_SUPPORT)
INT apcli_add_pmkid_cache(
	IN	PRTMP_ADAPTER	pAd,
	IN UCHAR *paddr,
	IN UCHAR *pmkid,
	IN UCHAR *pmk,
	IN UINT8 pmk_len,
	IN UINT8 if_index,
	IN UINT8 cli_idx)
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
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
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

	cached_idx = apcli_search_pmkid_cache(pAd, paddr, if_index, cli_idx);

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
	IN UCHAR if_index,
	IN UCHAR cli_idx)
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
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
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
	IN UCHAR if_index,
	IN UCHAR cli_idx)
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
		preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];
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


	cached_idx = apcli_search_pmkid_cache(pAd, paddr, if_index, cli_idx);

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
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
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

		for (cli_idx = 0; cli_idx < GET_MAX_REPEATER_ENTRY_NUM(cap); cli_idx++) {

			preptcli_entry = &pAd->ApCfg.pRepeaterCliPool[cli_idx];

			if (preptcli_entry && (preptcli_entry->CliValid == TRUE) && (preptcli_entry->MatchApCliIdx == papcli_entry->wdev.func_idx)) {

#ifdef APCLI_SAE_SUPPORT
			/*Delete all SAE instances for this Rept entry*/
				NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
				ins_cnt = 0;

				for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
					if (pSaeCfg->sae_ins[i].valid == FALSE)
						continue;

					if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, preptcli_entry->CurrentAddress, MAC_ADDR_LEN)) {
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
			&& IS_AKM_OWE(papcli_entry->wdev.SecConfig.AKMMap)
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
