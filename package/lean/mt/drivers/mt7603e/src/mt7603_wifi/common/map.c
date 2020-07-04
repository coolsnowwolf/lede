

#include "rt_config.h"
#include "map.h"


UCHAR MAP_OUI[3] = {0x50, 0x6F, 0x9A};
UCHAR MAP_OUI_TYPE[1] = {0x1B};
UCHAR MAP_EXT_ATTRI[1] = {0x06};
UCHAR MAP_ATTRI_LEN[1] = {1};

UCHAR multicast_mac_1905[MAC_ADDR_LEN] = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x13};

static UCHAR MAP_CheckDevRole(
	PRTMP_ADAPTER pAd,
	UCHAR wdev_type
)
{
	UCHAR res = 0;

	switch (wdev_type) {
	case WDEV_TYPE_AP:
		res = BIT(MAP_ROLE_FRONTHAUL_BSS); /* BH_BSS will be set by map cmd */
	break;

	/*case WDEV_TYPE_APCLI:*/
	case WDEV_TYPE_STA:
		res = BIT(MAP_ROLE_BACKHAUL_STA);
	break;

	default:
		res = 0;
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s():AP Role not set (Fixed me)\n", __func__));
	}

	return res;
}

INT MAP_InsertMapWscAttr(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf
)
{
	UCHAR MapVendorExt[10] = {0};
	UCHAR va[2] = {0x10, 0x49};
	UCHAR vl[2] = {0x00, 0x06};
	UCHAR vi[3] = {0x00, 0x37, 0x2A};

	/*WPS Vendor Extension */
	NdisMoveMemory(MapVendorExt, va, 2);
	NdisMoveMemory(MapVendorExt + 2, vl, 2);
	NdisMoveMemory(MapVendorExt + 4, vi, 3);
	NdisMoveMemory(MapVendorExt + 7, MAP_EXT_ATTRI, 1);
	NdisMoveMemory(MapVendorExt + 8, MAP_ATTRI_LEN, 1);
	NdisMoveMemory(MapVendorExt + 9, &wdev->MAPCfg.DevOwnRole, 1);

	NdisMoveMemory(pFrameBuf, MapVendorExt, sizeof(MapVendorExt));

	return sizeof(MapVendorExt);
}

VOID MAP_InsertMapCapIE(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen
)
{
	ULONG TmpLen;
	UCHAR IEType = IE_VENDOR_SPECIFIC;
	UCHAR IELen = 7;
	UCHAR MAP_EXT_ATTRI_LEN = 1;
	UCHAR MAP_EXT_ATTRI_VAL = wdev->MAPCfg.DevOwnRole;

	MakeOutgoingFrame(pFrameBuf, &TmpLen,
						1, &IEType,
						1, &IELen,
						3, MAP_OUI,
						1, MAP_OUI_TYPE,
						1, MAP_EXT_ATTRI,
						1, &MAP_EXT_ATTRI_LEN,
						1, &MAP_EXT_ATTRI_VAL,
						END_OF_ARGS);

	*pFrameLen = *pFrameLen + TmpLen;
}


/* return map attribute*/
BOOLEAN map_check_cap_ie(
	IN PEID_STRUCT   eid,
	OUT  unsigned char *cap
)
{
	BOOLEAN Ret = FALSE;

	if (NdisEqualMemory(eid->Octet, MAP_OUI, sizeof(MAP_OUI)) && (eid->Len >= 7)) {
		if (NdisEqualMemory((UCHAR *)&eid->Octet[3], MAP_OUI_TYPE, sizeof(MAP_OUI_TYPE)) &&
			NdisEqualMemory((UCHAR *)&eid->Octet[4], MAP_EXT_ATTRI, sizeof(MAP_EXT_ATTRI)) &&
			eid->Octet[5] == 1) {

			*cap = eid->Octet[6];
			Ret = TRUE;
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s:[MAP] STA Attri = %d\n", __func__, eid->Octet[6]));
		}
	}

	return Ret;
}


UCHAR getNonOpChnNum(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN UCHAR op_class
)
{
	UCHAR i = 0, j = 0;
	UCHAR nonOpChnNum = 0, opChnNum = 0;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class);
	UCHAR opChListLen = get_channel_set_num(opChList);

	for (i = 0; i < opChListLen; i++) {
		for (j = -0; j < pAd->ChannelListNum; j++) {
			if (opChList[i] == pAd->ChannelList[j].Channel) {
				opChnNum++;
				break;
			}
		}
	}
	nonOpChnNum = opChListLen - opChnNum;

	return nonOpChnNum;
}

VOID setNonOpChnList(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PCHAR nonOpChnList,
	IN UCHAR op_class,
	IN UCHAR nonOpChnNum
)
{
	UCHAR i = 0, j = 0, k = 0;
	BOOLEAN found = false;
	UCHAR *opChList = get_channelset_by_reg_class(pAd, op_class);
	UCHAR opChListLen = get_channel_set_num(opChList);

	if (nonOpChnNum > 0) {
		for (i = 0; i < opChListLen; i++) {
			for (j = -0; j < pAd->ChannelListNum; j++) {
				if (opChList[i] == pAd->ChannelList[j].Channel)
					found = true;
			}

			if (found == false) {
				nonOpChnList[k] = opChList[i];
				k++;
			} else
				found = false;
		}
	} else
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("No Non Op Channel\n"));


}

VOID MAP_Init(
	IN PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	IN UCHAR wdev_type
)
{
	wdev->MAPCfg.DevOwnRole = MAP_CheckDevRole(pAd, wdev_type);

	wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss = TRUE;/*by default*/
	wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss = FALSE;/*by default*/
	pAd->ApCfg.SteerPolicy.steer_policy = 0;
	pAd->ApCfg.SteerPolicy.cu_thr = 0;
	pAd->ApCfg.SteerPolicy.rcpi_thr = 0;
	NdisZeroMemory(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
}

INT map_send_bh_sta_wps_done_event(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY mac_entry,
	IN BOOLEAN is_ap)
{
	struct wifi_dev *wdev;
	struct wapp_event event;
	BOOLEAN send_event = FALSE;

	if (mac_entry) {

#ifdef APCLI_SUPPORT
		PAPCLI_STRUCT apcli_entry;
		struct wapp_bhsta_info *bsta_info = &event.data.bhsta_info;
#endif

		if (is_ap) {
			if (IS_MAP_ENABLE(adapter) && (mac_entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
				send_event = TRUE;
		}
#ifdef APCLI_SUPPORT
		else {
			apcli_entry =  &adapter->ApCfg.ApCliTab[mac_entry->func_tb_idx];
			if (IS_MAP_ENABLE(adapter) &&
				(mac_entry->DevPeerRole &
					(BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS))))
				COPY_MAC_ADDR(bsta_info->connected_bssid, apcli_entry->wdev.bssid);
				COPY_MAC_ADDR(bsta_info->mac_addr, apcli_entry->wdev.if_addr);
				send_event = TRUE;
		}
#endif
		if (send_event) {
			wdev = mac_entry->wdev;
			event.event_id = MAP_BH_STA_WPS_DONE;
			event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			wext_send_wapp_qry_rsp(adapter->net_dev, &event);
		}
	}

	return 0;
}

void wapp_send_rssi_steer_event(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	char rssi_thrd)
{
	struct wifi_dev *wdev;
	wdev_steer_sta *str_sta;
	struct wapp_event event;

	/* send event to daemon */
	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Trigger Rssi steering!\n"));
	pEntry->isTriggerSteering = TRUE;

	wdev = pEntry->wdev;
	event.event_id = MAP_TRIGGER_RSSI_STEER;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	str_sta = &event.data.str_sta;
	COPY_MAC_ADDR(str_sta->mac_addr, pEntry->Addr);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
}

VOID map_rssi_status_check(
	IN PRTMP_ADAPTER pAd)
{
	int i = 0;
	char rssi_thrd = 0;

	if (pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_ALLOW ||
		pAd->ApCfg.SteerPolicy.steer_policy == AGENT_INIT_RSSI_STEER_MANDATE) {
		rssi_thrd = (pAd->ApCfg.SteerPolicy.rcpi_thr >> 1) - 110;

		for (i = 0; VALID_UCAST_ENTRY_WCID(i); i++) {
			PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[i];

			if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
				if (pEntry->RssiSample.AvgRssi[0] < rssi_thrd) {
					pEntry->cur_rssi_status = BELOW_THRESHOLD;

					/*
					*	If sta's rssi is within RCPI tollenant boundary,
					*	ignore this rssi detection to avoid sending event
					*	to wapp constantly
					*/
					if (pEntry->isTriggerSteering == TRUE &&
						pEntry->pre_rssi_status == ABOVE_THRESHOLD &&
						pEntry->cur_rssi_status == BELOW_THRESHOLD &&
						abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) <= RCPI_TOLLENACE)
						return;

					wapp_send_rssi_steer_event(pAd, pEntry, rssi_thrd);
				} else
					pEntry->cur_rssi_status = ABOVE_THRESHOLD;

				if ((pEntry->pre_rssi_status == ABOVE_THRESHOLD) &&
					(pEntry->cur_rssi_status == BELOW_THRESHOLD) &&
					abs(pEntry->RssiSample.AvgRssi[0] - rssi_thrd) > RCPI_TOLLENACE)
						pEntry->isTriggerSteering = FALSE;

				pEntry->pre_rssi_status = pEntry->cur_rssi_status;
			}
		}
	}
}

INT ReadMapParameterFromFile(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer)
{
	INT i;
	RTMP_STRING *macptr;

	return TRUE;
}


#ifdef A4_CONN
BOOLEAN map_a4_peer_enable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
#ifdef APCLI_SUPPORT
	PAPCLI_STRUCT apcli_entry;
#endif

	if (is_ap) {
		if (IS_MAP_ENABLE(adapter) && (entry->DevPeerRole & BIT(MAP_ROLE_BACKHAUL_STA)))
			return a4_ap_peer_enable(adapter, entry, A4_TYPE_MAP);
	}
#ifdef APCLI_SUPPORT
	else {
		apcli_entry =  &adapter->ApCfg.ApCliTab[entry->func_tb_idx];
		if (IS_MAP_ENABLE(adapter) &&
			(entry->DevPeerRole & (BIT(MAP_ROLE_FRONTHAUL_BSS) | BIT(MAP_ROLE_BACKHAUL_BSS)))) {
			return a4_apcli_peer_enable(adapter,
										apcli_entry,
										entry,
										A4_TYPE_MAP);
		}
	}
#endif

	return FALSE;
}

BOOLEAN map_a4_peer_disable(
	IN PRTMP_ADAPTER adapter,
	IN PMAC_TABLE_ENTRY entry,
	IN BOOLEAN is_ap /*if i'm AP or not*/
)
{
	if (is_ap)
		return a4_ap_peer_disable(adapter, entry, A4_TYPE_MAP);
#ifdef APCLI_SUPPORT
	else
		return a4_apcli_peer_disable(adapter, &adapter->ApCfg.ApCliTab[entry->func_tb_idx], entry, A4_TYPE_MAP);
#else
	return FALSE;
#endif
}


BOOLEAN map_a4_init(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_init(adapter, if_index, is_ap, A4_TYPE_MAP);
}


BOOLEAN map_a4_deinit(
	IN PRTMP_ADAPTER adapter,
	IN UCHAR if_index,
	IN BOOLEAN is_ap
)
{
	return a4_interface_deinit(adapter, if_index, is_ap, A4_TYPE_MAP);
}

BOOLEAN MapNotRequestedChannel(struct wifi_dev *wdev, unsigned char channel)
{
	int i = 0;

	if (wdev->MAPCfg.scan_bh_ssids.scan_channel_count == 0)
		return FALSE;
	for (i = 0; i < wdev->MAPCfg.scan_bh_ssids.scan_channel_count; i++) {
		if (channel == wdev->MAPCfg.scan_bh_ssids.scan_channel_list[i])
			return FALSE;
	}
	return TRUE;
}

#endif
