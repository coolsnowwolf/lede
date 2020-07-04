/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	event_notifier.c
 
    Abstract:
    This is event notify feature used to send wireless event to upper layer.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifdef WH_EVENT_NOTIFIER
#include "rt_config.h"
//#include "hdev/hdev.h"

static NTSTATUS WHCStaProbeReq(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    PEER_PROBE_REQ_PARAM *pProbeReqParam = NULL;
    MLME_QUEUE_ELEM *Elem = NULL;
    PFRAME_802_11 pFrame = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_probe_req *pProbeReq = &drvevnt.data.probereq;
    HTTRANSMIT_SETTING HTSetting;
    
    NdisZeroMemory(pProbeReq, sizeof(*pProbeReq));
    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    pProbeReqParam = va_arg(Args, PEER_PROBE_REQ_PARAM *);
    Elem = va_arg(Args, MLME_QUEUE_ELEM *);
    va_end(Args);
    pFrame = (PFRAME_802_11)Elem->Msg;

    pProbeReq->type = WHC_DRVEVNT_STA_PROBE_REQ;
    COPY_MAC_ADDR(pProbeReq->sta_mac, pProbeReqParam->Addr2);
    pProbeReq->is_ucast = IS_BROADCAST_MAC_ADDR(pFrame->Hdr.Addr1)?FALSE:TRUE;
    pProbeReq->rx_info.channel = Elem->Channel;
    pProbeReq->rx_info.rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						                ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							            ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
    if(Elem->Channel > 14)
    {
    	HTSetting.field.MODE = MODE_OFDM;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = MCS_RATE_6;
    }
    else
    {
    	HTSetting.field.MODE = MODE_CCK;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = RATE_1;
    }
    getRate(HTSetting, &pProbeReq->rx_info.rate);
    
    pProbeReq->cap = 0;
	if(pProbeReqParam->ie_list.ht_cap_len > 0)
    {   
		pProbeReq->cap |= WHC_WLCAP_80211_N;
        if(pProbeReqParam->ie_list.HTCapability.HtCapInfo.ChannelWidth == BW_40)
            pProbeReq->cap |= WHC_WLCAP_HT40;
    }

#ifdef DOT11_VHT_AC
    if (pProbeReqParam->ie_list.vht_cap_len > 0)
    {
        VHT_CAP_IE *vht_cap_ie = &pProbeReqParam->ie_list.vht_cap;
        pProbeReq->cap |= WHC_WLCAP_80211_AC;

        if(vht_cap_ie->vht_cap.ch_width == VHT_BW_80)
            pProbeReq->cap |= WHC_WLCAP_HT80;
        else if(vht_cap_ie->vht_cap.ch_width == VHT_BW_160)
            pProbeReq->cap |= WHC_WLCAP_HT160;
        else if(vht_cap_ie->vht_cap.ch_width == VHT_BW_8080)
            pProbeReq->cap |= (WHC_WLCAP_HT80|WHC_WLCAP_HT160);
        else
            pProbeReq->cap |= WHC_WLCAP_HT40;
    }
#endif /* DOT11_VHT_AC */

    pProbeReq->cap |= \
	    (pProbeReqParam->ie_list.HTCapability.MCSSet[3] != 0x00) ? (WHC_WLCAP_RX_4_STREAMS|WHC_WLCAP_TX_4_STREAMS) :\
	    (pProbeReqParam->ie_list.HTCapability.MCSSet[2] != 0x00) ? (WHC_WLCAP_RX_3_STREAMS|WHC_WLCAP_TX_3_STREAMS) :\
	    (pProbeReqParam->ie_list.HTCapability.MCSSet[1] != 0x00) ? (WHC_WLCAP_RX_2_STREAMS|WHC_WLCAP_TX_2_STREAMS) : 0;

    if(pProbeReqParam->ie_list.vendor_ie.custom_ie_len > 0)
    {
        pProbeReq->custom_ie_len = pProbeReqParam->ie_list.vendor_ie.custom_ie_len;
        NdisMoveMemory(pProbeReq->custom_ie, pProbeReqParam->ie_list.vendor_ie.custom_ie,
                        pProbeReqParam->ie_list.vendor_ie.custom_ie_len);
    }

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_PROBE_REQ,
						    NULL, (PUCHAR)pProbeReq, sizeof(*pProbeReq));
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCApProbeRsp(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    BCN_IE_LIST *ie_list = NULL;
    MLME_QUEUE_ELEM *Elem = NULL;
    PFRAME_802_11 pFrame = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_ap_probe_rsp *pProbeRsp = &drvevnt.data.probersp;
    HTTRANSMIT_SETTING HTSetting;
    
    NdisZeroMemory(pProbeRsp, sizeof(*pProbeRsp));

    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    ie_list = va_arg(Args, BCN_IE_LIST *);
    Elem = va_arg(Args, MLME_QUEUE_ELEM *);
    va_end(Args);
    pFrame = (PFRAME_802_11)Elem->Msg;

    pProbeRsp->type = WHC_DRVEVNT_AP_PROBE_RSP;
    COPY_MAC_ADDR(pProbeRsp->ap_mac, pFrame->Hdr.Addr2);
    pProbeRsp->rx_info.channel = Elem->Channel;
    pProbeRsp->rx_info.rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						                ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							            ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
    if(Elem->Channel > 14)
    {
    	HTSetting.field.MODE = MODE_OFDM;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = MCS_RATE_6;
    }
    else
    {
    	HTSetting.field.MODE = MODE_CCK;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = RATE_1;
    }
    getRate(HTSetting, &pProbeRsp->rx_info.rate);
    
    pProbeRsp->cap = 0;
    if(ie_list->HtCapabilityLen > 0)
    {   
		pProbeRsp->cap |= WHC_WLCAP_80211_N;
        if(ie_list->HtCapability.HtCapInfo.ChannelWidth == BW_40)
            pProbeRsp->cap |= WHC_WLCAP_HT40;
    }

#ifdef DOT11_VHT_AC
    if (ie_list->vht_cap_len > 0)
    {
        VHT_CAP_IE *vht_cap_ie = &ie_list->vht_cap_ie;
        pProbeRsp->cap |= WHC_WLCAP_80211_AC;

        if(vht_cap_ie->vht_cap.ch_width == VHT_BW_80)
            pProbeRsp->cap |= WHC_WLCAP_HT80;
        else if(vht_cap_ie->vht_cap.ch_width == VHT_BW_160)
            pProbeRsp->cap |= WHC_WLCAP_HT160;
        else if(vht_cap_ie->vht_cap.ch_width == VHT_BW_8080)
            pProbeRsp->cap |= (WHC_WLCAP_HT80|WHC_WLCAP_HT160);
        else
            pProbeRsp->cap |= WHC_WLCAP_HT40;
    }
#endif /* DOT11_VHT_AC */
    pProbeRsp->cap |= \
	    (ie_list->HtCapability.MCSSet[3] != 0x00) ? (WHC_WLCAP_RX_4_STREAMS|WHC_WLCAP_TX_4_STREAMS) :\
	    (ie_list->HtCapability.MCSSet[2] != 0x00) ? (WHC_WLCAP_RX_3_STREAMS|WHC_WLCAP_TX_3_STREAMS) :\
	    (ie_list->HtCapability.MCSSet[1] != 0x00) ? (WHC_WLCAP_RX_2_STREAMS|WHC_WLCAP_TX_2_STREAMS) : 0;

    if(ie_list->vendor_ie.custom_ie_len > 0)
    {
        pProbeRsp->custom_ie_len = ie_list->vendor_ie.custom_ie_len;
        NdisMoveMemory(pProbeRsp->custom_ie, ie_list->vendor_ie.custom_ie, ie_list->vendor_ie.custom_ie_len);
    }
    
    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_AP_PROBE_RSP,
						    NULL, (PUCHAR)pProbeRsp, sizeof(*pProbeRsp));
    
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaJoin(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    MAC_TABLE_ENTRY *pEntry = NULL;
    MLME_QUEUE_ELEM *Elem = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_join *pStaJoin = &drvevnt.data.join;
    
    NdisZeroMemory(pStaJoin, sizeof(*pStaJoin));

    va_start(Args, pAd);
    pEntry = va_arg(Args, MAC_TABLE_ENTRY *);
    Elem = va_arg(Args, MLME_QUEUE_ELEM *);
    va_end(Args);
    
    wdev = pEntry->wdev;
    pStaJoin->type = WHC_DRVEVNT_STA_JOIN;
    pStaJoin->channel = Elem->Channel;
    COPY_MAC_ADDR(pStaJoin->sta_mac, pEntry->Addr);
    pStaJoin->aid = pEntry->Aid;

#ifdef DOT11K_RRM_SUPPORT    
    pStaJoin->capability |= (pEntry->CapabilityInfo & RRM_CAP_BIT) >> 12; /* Radio Measurement Enable or Disable */
#endif
#ifdef DOT11V_WNM_SUPPORT
    pStaJoin->capability |= (pEntry->BssTransitionManmtSupport) << 1; /* BSS Transition Management Enable or Disable */
#endif
    if(pEntry->custom_ie_len > 0)
    {
        pStaJoin->custom_ie_len = pEntry->custom_ie_len;
        NdisMoveMemory(pStaJoin->custom_ie, pEntry->custom_ie, pEntry->custom_ie_len);
    }
    
    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_JOIN,
						    NULL, (PUCHAR)pStaJoin, sizeof(*pStaJoin));

	DBGPRINT(RT_DEBUG_TRACE, ("%s WHC_DRVEVNT_STA_JOIN send ....\n", __FUNCTION__));

    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaLeave(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    UCHAR *pStaMac = NULL;
    UCHAR Channel;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_leave *pStaLeave = &drvevnt.data.leave;

    NdisZeroMemory(pStaLeave, sizeof(*pStaLeave));
    
    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    pStaMac = va_arg(Args, UCHAR *);
    Channel = (UCHAR)va_arg(Args, UINT32);
    va_end(Args);

    pStaLeave->type = WHC_DRVEVNT_STA_LEAVE;
    COPY_MAC_ADDR(pStaLeave->sta_mac, pStaMac);
    pStaLeave->channel = Channel;

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_LEAVE,
						    NULL, (PUCHAR)pStaLeave, sizeof(*pStaLeave));
    
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCExtUpLinkStatus(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    MAC_TABLE_ENTRY *pEntry = NULL;
    UINT32 link_state;
    struct wifi_dev *wdev = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_ext_uplink_stat *pExtUplink = &drvevnt.data.link_state;

    NdisZeroMemory(pExtUplink, sizeof(*pExtUplink));
    
    va_start(Args, pAd);
    pEntry = va_arg(Args, MAC_TABLE_ENTRY *);
    link_state = va_arg(Args, UINT32);
    va_end(Args);
    wdev = pEntry->wdev;

    pExtUplink->type = WHC_DRVEVNT_EXT_UPLINK_STAT;
    COPY_MAC_ADDR(pExtUplink->ap_mac, pEntry->Addr);
    pExtUplink->channel = pAd->CommonCfg.Channel;
    pExtUplink->link_state = link_state;

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_EXT_UPLINK_STAT,
						    NULL, (PUCHAR)pExtUplink, sizeof(*pExtUplink));
    
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaTimeout(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    MAC_TABLE_ENTRY *pEntry = NULL;
    struct wifi_dev *wdev = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_timeout *pStaTimeout = &drvevnt.data.timeout;

    NdisZeroMemory(pStaTimeout, sizeof(*pStaTimeout));
    
    va_start(Args, pAd);
    pEntry = va_arg(Args, MAC_TABLE_ENTRY *);
    va_end(Args);
    wdev = pEntry->wdev;

    pStaTimeout->type = WHC_DRVEVNT_STA_TIMEOUT;
    COPY_MAC_ADDR(pStaTimeout->sta_mac, pEntry->Addr);
    pStaTimeout->channel = pAd->CommonCfg.Channel;

    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_TIMEOUT,
						    NULL, (PUCHAR)pStaTimeout, sizeof(*pStaTimeout));
    
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaAuthReject(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    UCHAR *pStaMac = NULL;
    MLME_QUEUE_ELEM *Elem = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_auth_reject *pStaAuthReject = &drvevnt.data.auth_reject;
    HTTRANSMIT_SETTING HTSetting;

    NdisZeroMemory(pStaAuthReject, sizeof(*pStaAuthReject));
    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    pStaMac = va_arg(Args, UCHAR *);
    Elem = va_arg(Args, MLME_QUEUE_ELEM *);
    va_end(Args);

    pStaAuthReject->type = WHC_DRVEVNT_STA_AUTH_REJECT;
    pStaAuthReject->rx_info.channel = Elem->Channel;
    COPY_MAC_ADDR(pStaAuthReject->sta_mac, pStaMac);
    pStaAuthReject->rx_info.rssi = RTMPMaxRssi(pAd, ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_0),
						                ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_1),
							            ConvertToRssi(pAd, &Elem->rssi_info, RSSI_IDX_2));
    if(Elem->Channel > 14)
    {
    	HTSetting.field.MODE = MODE_OFDM;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = MCS_RATE_6;
    }
    else
    {
    	HTSetting.field.MODE = MODE_CCK;
    	HTSetting.field.BW = BW_20;
    	HTSetting.field.MCS = RATE_1;
    }
    getRate(HTSetting, &pStaAuthReject->rx_info.rate);
    
    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_AUTH_REJECT,
						    NULL, (PUCHAR)pStaAuthReject, sizeof(*pStaAuthReject));
    
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaRssiTooLow(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    struct wifi_dev *wdev = NULL;
    CHAR Rssi;
    UCHAR *pStaMac = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_rssi_too_low *pStaRssi = &drvevnt.data.sta_rssi;

    NdisZeroMemory(pStaRssi, sizeof(*pStaRssi));

    va_start(Args, pAd);
    wdev = va_arg(Args, struct wifi_dev *);
    pStaMac = va_arg(Args, UCHAR *);
    Rssi = (CHAR)va_arg(Args, INT);
    va_end(Args);

    pStaRssi->type = WHC_DRVEVNT_STA_AUTH_REJECT;
    pStaRssi->rssi = Rssi;
    COPY_MAC_ADDR(pStaRssi->sta_mac, pStaMac);
    
    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_RSSI_TOO_LOW,
						    NULL, (PUCHAR)pStaRssi, sizeof(*pStaRssi));
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS WHCStaActivityState(IN PRTMP_ADAPTER pAd, ...)
{
    va_list Args;
    BOOLEAN bTx;
    MAC_TABLE_ENTRY *pEntry = NULL;
    StaActivityItem *pStaActivity = NULL;
    struct wifi_dev *wdev = NULL;
    struct drvevnt_type_buf drvevnt;
    struct drvevnt_sta_activity_stat *pStaStat = &drvevnt.data.sta_activity_stat;

    NdisZeroMemory(pStaStat, sizeof(*pStaStat));

    va_start(Args, pAd);
    pEntry = va_arg(Args, MAC_TABLE_ENTRY *);
    bTx = (BOOLEAN)va_arg(Args, INT);
    va_end(Args);
    wdev = pEntry->wdev;
    if(bTx)
       pStaActivity = &pEntry->tx_state;
    else
       pStaActivity = &pEntry->rx_state;

    pStaStat->type = WHC_DRVEVNT_STA_ACTIVITY_STATE;
    pStaStat->bTx = bTx;
    NdisMoveMemory(&pStaStat->sta_state, pStaActivity, sizeof(*pStaActivity));
    COPY_MAC_ADDR(pStaStat->sta_mac, pEntry->Addr);
    
    RtmpOSWrielessEventSend(wdev->if_dev, RT_WLAN_EVENT_CUSTOM, WHC_DRVEVNT_STA_ACTIVITY_STATE,
						    NULL, (PUCHAR)pStaStat, sizeof(*pStaStat));
    
    return NDIS_STATUS_SUCCESS;
}

static EVENT_TABLE_T EventHdlrTable[] = {
    {WHC_DRVEVNT_STA_PROBE_REQ,     WHCStaProbeReq},
    {WHC_DRVEVNT_AP_PROBE_RSP,      WHCApProbeRsp},
    {WHC_DRVEVNT_STA_JOIN,          WHCStaJoin},
    {WHC_DRVEVNT_STA_LEAVE,         WHCStaLeave},
    {WHC_DRVEVNT_EXT_UPLINK_STAT,   WHCExtUpLinkStatus},
    {WHC_DRVEVNT_STA_TIMEOUT,       WHCStaTimeout},
    {WHC_DRVEVNT_STA_AUTH_REJECT,   WHCStaAuthReject},
    {WHC_DRVEVNT_STA_RSSI_TOO_LOW,  WHCStaRssiTooLow},
    {WHC_DRVEVNT_STA_ACTIVITY_STATE,  WHCStaActivityState},
    {DRVEVNT_END_ID, NULL}
};

EventHdlr GetEventNotiferHook(IN UINT32 EventID)
{
	UINT32 CurIndex = 0;
	UINT32 EventHdlrTableLength= sizeof(EventHdlrTable) / sizeof(EVENT_TABLE_T);
	EventHdlr Handler = NULL;

	if(EventID > DRVEVNT_END_ID)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Event ID(%d) is out of boundary.\n", EventID));
		return NULL;
	}

	for(CurIndex=0; CurIndex < EventHdlrTableLength; CurIndex++)
	{
		if (EventHdlrTable[CurIndex].EventID == EventID)
		{
			Handler = EventHdlrTable[CurIndex].EventHandler;
			break;
		}
	}

	if(Handler == NULL)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("No corresponding EventHdlr for this Event ID(%d).\n",  EventID));
    
	return Handler;
}

INT SetCustomOUIProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    UCHAR tmp[256] = {0};
    INT len = 0;

    pAd->ApCfg.EventNotifyCfg.CustomOUILen = 0;
    NdisZeroMemory(pAd->ApCfg.EventNotifyCfg.CustomOUI, sizeof(pAd->ApCfg.EventNotifyCfg.CustomOUI));

    strncpy(tmp, arg, sizeof(tmp));
	len = strlen(tmp);
	if(len > 0)
	{
		if(len%2 != 0)
		{
			tmp[len]='0';
			len++;
		}
		
		AtoH(tmp, pAd->ApCfg.EventNotifyCfg.CustomOUI, len);
		pAd->ApCfg.EventNotifyCfg.CustomOUILen = len/2;
	}

    return TRUE;
}

INT SetCustomVIEProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0, tmp[256] = {0};
    INT len = 0;    
    struct wifi_dev *wdev = NULL;
    BOOLEAN bAP = FALSE;
    
    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    switch (pObj->ioctl_if_type)
    {
        case INT_MAIN:
        case INT_MBSSID:
            if(ifIndex < HW_BEACON_MAX_NUM)
            {
                wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
                bAP = TRUE;
            }
            break;
#ifdef APCLI_SUPPORT
        case INT_APCLI:
            if(ifIndex < MAX_APCLI_NUM)
                wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
            break;
#endif /* APCLI_SUPPORT */
        default:
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
            return FALSE;
            break;
    }

    if(!wdev)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s(): wdev is NULL!\n", __FUNCTION__));
		return FALSE;
    }
    wdev->custom_vie.ie_hdr.eid = IE_VENDOR_SPECIFIC;
    wdev->custom_vie.ie_hdr.len = 0;
    NdisZeroMemory(wdev->custom_vie.custom_ie, CUSTOM_IE_TOTAL_LEN);
    
    strncpy(tmp, arg, sizeof(tmp));
	len = strlen(tmp);
	if(len > 0)
	{
		if(len%2 != 0)
		{
			tmp[len]='0';
			len++;
		}
		
		AtoH(tmp, wdev->custom_vie.custom_ie, len);
		wdev->custom_vie.ie_hdr.len = len/2;
        if(bAP)
            UpdateBeaconHandler(pAd, wdev, IE_CHANGE);
	}
    
    return TRUE;
}

INT SetChannelLoadDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    struct EventNotifierCfg *pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
     
    pEventNotifyCfg->ChLoadDetectPeriod = simple_strtol(arg, 0, 10);
    if(pEventNotifyCfg->ChLoadDetectPeriod > 0)
    {
        pEventNotifyCfg->bChLoadDetect = TRUE;
        pEventNotifyCfg->ChLoadRound = 0;
    }
    else
    {
        pEventNotifyCfg->bChLoadDetect = FALSE;
    }
    return TRUE;
}

INT SetStaRssiDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0;
    struct EventNotifierCfg *pEventNotifyCfg = NULL;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    if((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
        return FALSE;
    }

    pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
    pEventNotifyCfg->StaRssiDetectThreshold = simple_strtol(arg, 0, 10);
    if(pEventNotifyCfg->StaRssiDetectThreshold >= 0)
        pEventNotifyCfg->bStaRssiDetect = FALSE;
    else
        pEventNotifyCfg->bStaRssiDetect = TRUE;
    
    return TRUE;
}

INT SetStaTxPktDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0;
    struct EventNotifierCfg *pEventNotifyCfg = NULL;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    if((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
        return FALSE;
    }

    pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
    pEventNotifyCfg->StaTxPktDetectPeriod = simple_strtol(arg, 0, 10);
    pEventNotifyCfg->StaTxPktDetectRound = 0;
    
    return TRUE;
}

INT SetStaTxPacketDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0;
    struct EventNotifierCfg *pEventNotifyCfg = NULL;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    if((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
        return FALSE;
    }

    pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
    pEventNotifyCfg->StaStateTxThreshold = simple_strtol(arg, 0, 10);
    if(pEventNotifyCfg->StaStateTxThreshold > 0)
    {
        pEventNotifyCfg->bStaStateTxDetect = TRUE;
        pEventNotifyCfg->StaTxPktDetectRound = 0;
    }
    else
        pEventNotifyCfg->bStaStateTxDetect = FALSE;
    
    return TRUE;
}

INT SetStaRxPktDetectPeriodProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0;
    struct EventNotifierCfg *pEventNotifyCfg = NULL;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    if((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
        return FALSE;
    }

    pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
    pEventNotifyCfg->StaRxPktDetectPeriod = simple_strtol(arg, 0, 10);
    pEventNotifyCfg->StaRxPktDetectRound = 0;

    return TRUE;
}

INT SetStaRxPacketDetectThresholdProc(PRTMP_ADAPTER pAd, PSTRING arg)
{
    POS_COOKIE pObj = NULL;
    UCHAR ifIndex = 0;
    struct EventNotifierCfg *pEventNotifyCfg = NULL;

    pObj = (POS_COOKIE) pAd->OS_Cookie;
    ifIndex = pObj->ioctl_if;

    if((pObj->ioctl_if_type != INT_MAIN) && (pObj->ioctl_if_type != INT_MBSSID))
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s(): Invalid interface type(%u).\n", __FUNCTION__, pObj->ioctl_if_type));
        return FALSE;
    }

    pEventNotifyCfg = &pAd->ApCfg.EventNotifyCfg;
    pEventNotifyCfg->StaStateRxThreshold = simple_strtol(arg, 0, 10);
    if(pEventNotifyCfg->StaStateRxThreshold > 0)
    {
        pEventNotifyCfg->bStaStateRxDetect = TRUE;
        pEventNotifyCfg->StaRxPktDetectRound = 0 ;
    }
    else
        pEventNotifyCfg->bStaStateRxDetect = FALSE;

    return TRUE;
}
#endif /* WH_EVENT_NOTIFIER */
