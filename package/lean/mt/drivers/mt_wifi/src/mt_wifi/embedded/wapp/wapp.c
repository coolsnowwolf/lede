/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	wapp.c

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef WAPP_SUPPORT
#include "wapp/wapp_cmm_type.h"

UCHAR ESPI_AC_BE_DEFAULT[3] = {0xF8, 0xFF, 0x00};
UCHAR ESPI_AC_BK_DEFAULT[3] = {0xF9, 0xFF, 0x00};
UCHAR ESPI_AC_VO_DEFAULT[3] = {0xFA, 0xFF, 0x00};
UCHAR ESPI_AC_VI_DEFAULT[3] = {0xFB, 0xFF, 0x00};

#define CLI_REQ_MIN_INTERVAL	5 /* sec */

VOID wext_send_wapp_qry_rsp(
	PNET_DEV pNetDev,
	struct wapp_event *event)
{

	UINT buflen = sizeof(struct wapp_event);

	event->len = buflen - sizeof(event->len) - sizeof(event->event_id);

	RtmpOSWrielessEventSend(pNetDev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT, NULL, (PUCHAR)event, sizeof(struct wapp_event));
}

INT wapp_send_wdev_query_rsp(
	PRTMP_ADAPTER pAd)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_dev_info *dev_info;

	event.event_id = WAPP_DEV_QUERY_RSP;
	event.ifindex = 0; /* wapp will not use event.ifindex in this case */
	dev_info = &event.data.dev_info;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			dev_info->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
			dev_info->dev_type = wdev->wdev_type;
			COPY_MAC_ADDR(dev_info->mac_addr, wdev->if_addr);
			NdisCopyMemory(dev_info->ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);
			dev_info->radio_id = HcGetBandByWdev(wdev);
			dev_info->adpt_id = (uintptr_t) pAd;
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

	return 0;
}

INT wapp_send_wdev_ht_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ht_cap *ht_cap;

	event.event_id = WAPP_HT_CAP_QUERY_RSP;
	event.ifindex = req->data.ifindex;

	ht_cap = &event.data.ht_cap;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				ht_cap->tx_stream = wlan_config_get_tx_stream(wdev);
				ht_cap->rx_stream = wlan_config_get_rx_stream(wdev);
				ht_cap->sgi_20 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				ht_cap->sgi_40 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				ht_cap->ht_40 = (wlan_operate_get_ht_bw(wdev) == BW_40) ? \
								 1:0;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_wdev_vht_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_vht_cap *vht_cap;
	VHT_CAP_INFO drv_vht_cap;
	VHT_OP_IE drv_vht_op;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	NdisZeroMemory(&drv_vht_op, sizeof(VHT_OP_IE));
	NdisCopyMemory(&drv_vht_cap, &pAd->CommonCfg.vht_cap_ie.vht_cap, sizeof(VHT_CAP_INFO));

	event.event_id = WAPP_VHT_CAP_QUERY_RSP;
	event.ifindex = req->data.ifindex;

	vht_cap = &event.data.vht_cap;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				mt_WrapSetVHTETxBFCap(pAd, wdev, &drv_vht_cap);
				/* printk("\033[1;33m wdev_list[%d] type = %u\033[0m\n", i, wdev->wdev_type);*/  /* Haipin Debug Print (Y)*/

				switch	(wlan_operate_get_rx_stream(wdev)) {
				case 4:
					drv_vht_op.basic_mcs_set.mcs_ss4 = cap->max_vht_mcs;
				case 3:
					drv_vht_op.basic_mcs_set.mcs_ss3 = cap->max_vht_mcs;
				case 2:
					drv_vht_op.basic_mcs_set.mcs_ss2 = cap->max_vht_mcs;
				case 1:
					drv_vht_op.basic_mcs_set.mcs_ss1 = cap->max_vht_mcs;
					break;
				}

				NdisMoveMemory(vht_cap->sup_tx_mcs,
								&drv_vht_op.basic_mcs_set,
								sizeof(vht_cap->sup_tx_mcs));
				NdisMoveMemory(vht_cap->sup_rx_mcs,
								&drv_vht_op.basic_mcs_set,
								sizeof(vht_cap->sup_rx_mcs));
				vht_cap->tx_stream = wlan_config_get_tx_stream(wdev);
				vht_cap->rx_stream = wlan_config_get_tx_stream(wdev);
				vht_cap->sgi_80 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				vht_cap->sgi_160 = (wlan_config_get_ht_gi(wdev) == GI_400) ? \
								 1:0;
				vht_cap->vht_160 = (wlan_operate_get_vht_bw(wdev) == BW_160) ? \
								 1:0;
				vht_cap->vht_8080 = (wlan_operate_get_vht_bw(wdev) == BW_8080) ? \
								 1:0;
				vht_cap->su_bf = (drv_vht_cap.bfer_cap_su) ? \
								 1:0;
				vht_cap->mu_bf = (drv_vht_cap.bfer_cap_mu) ? \
								 1:0;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_wdev_misc_cap_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_misc_cap *misc_cap;


	event.event_id = WAPP_MISC_CAP_QUERY_RSP;
	event.ifindex = 0;
	misc_cap = &event.data.misc_cap;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				event.ifindex = req->data.ifindex;
				misc_cap->max_num_of_cli = 64;
				misc_cap->max_num_of_bss = HW_BEACON_MAX_NUM;
				misc_cap->num_of_bss = pAd->ApCfg.BssidNum;
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_fill_client_info(
	PRTMP_ADAPTER pAd,
	wapp_client_info *cli_info,
	MAC_TABLE_ENTRY *mac_entry)
{
	STA_TR_ENTRY *tr_entry;
	struct wifi_dev *wdev;
	ULONG DataRate = 0, DataRate_r = 0;
	HTTRANSMIT_SETTING HTPhyMode;

	tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];
	wdev = mac_entry->wdev;

	COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);
	COPY_MAC_ADDR(cli_info->bssid, wdev->bssid);
	cli_info->sta_status = \
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ?
				WAPP_STA_CONNECTED : WAPP_STA_DISCONNECTED;
	cli_info->assoc_time = mac_entry->StaConnectTime;
	HTPhyMode.word = (USHORT)mac_entry->LastTxRate;
	getRate(HTPhyMode, &DataRate);
	cli_info->downlink = (int) DataRate;

	HTPhyMode.word = (USHORT) mac_entry->LastRxRate;
	getRate(HTPhyMode, &DataRate_r);
	cli_info->uplink = (int) DataRate_r;

	cli_info->uplink_rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
#ifdef CONFIG_DOT11V_WNM
	cli_info->bBSSMantSupport = mac_entry->bBSSMantSTASupport;
#endif
	cli_info->bLocalSteerDisallow = false;
	cli_info->bBTMSteerDisallow = false;
	/*traffic stats*/
	cli_info->bytes_sent = mac_entry->TxBytes;
	cli_info->bytes_received = mac_entry->RxBytes;
	cli_info->packets_sent = mac_entry->TxPackets.u.LowPart;
	cli_info->packets_received = mac_entry->RxPackets.u.LowPart;
	cli_info->tx_packets_errors = 0; /* to do */
	cli_info->rx_packets_errors = 0; /* to do */
	cli_info->retransmission_count = 0; /* to do */
	cli_info->link_availability = 50; /* to do */

	return 0;
}

INT wapp_send_cli_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;

	event.event_id = WAPP_CLI_QUERY_RSP;
	cli_info = &event.data.cli_info;

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		mac_entry = &pAd->MacTab.Content[i];
		if (IS_ENTRY_CLIENT(mac_entry)
			&& NdisCmpMemory(mac_entry->Addr, &req->data.mac_addr, MAC_ADDR_LEN) == 0
			&& req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			wapp_fill_client_info(pAd, cli_info, mac_entry);
			event.ifindex = req->data.ifindex;
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			return 0;
		}
	}

	COPY_MAC_ADDR(cli_info->mac_addr, req->data.mac_addr);
	cli_info->sta_status = WAPP_STA_DISCONNECTED;
	event.ifindex = req->data.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_cli_list_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;

	event.event_id = WAPP_CLI_LIST_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}


INT wapp_handle_cli_list_query(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;
	ULONG last_query_time = 0;
	ULONG now;

	NdisGetSystemUpTime(&now);
	if (RTMP_TIME_AFTER(now, last_query_time + (CLI_REQ_MIN_INTERVAL * OS_HZ))) {
		event.event_id = WAPP_CLI_QUERY_RSP;
		cli_info = &event.data.cli_info;

		for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
			mac_entry = &pAd->MacTab.Content[i];
			if (IS_ENTRY_CLIENT(mac_entry)) {/* report all entry no matter which wdev it is belonged */
				wdev = mac_entry->wdev;
				wapp_fill_client_info(pAd, cli_info, mac_entry);
				event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
#ifdef MBO_SUPPORT
				if (mac_entry->bIndicateNPC)
					MboIndicateStaInfoToDaemon(pAd,
											&mac_entry->MboStaInfoNPC,
											MBO_MSG_STA_PREF_UPDATE);
				if (mac_entry->bIndicateCDC)
					MboIndicateStaInfoToDaemon(pAd,
											&mac_entry->MboStaInfoCDC,
											MBO_MSG_CDC_UPDATE);
#endif /* MBO_SUPPORT */
			}
		}

		last_query_time = now;
	}
	wapp_send_cli_list_query_rsp(pAd, req);
	return 0;
}

/* client assoc */
INT wapp_send_cli_join_event(
	PRTMP_ADAPTER pAd,
	MAC_TABLE_ENTRY *mac_entry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;

	if (mac_entry) {
		wdev = mac_entry->wdev;
		event.event_id = WAPP_CLI_JOIN_EVENT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		cli_info = &event.data.cli_info;
		wapp_fill_client_info(pAd, cli_info, mac_entry);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
#ifdef MBO_SUPPORT
		if (mac_entry->bIndicateNPC)
			MboIndicateStaInfoToDaemon(pAd, &mac_entry->MboStaInfoNPC, MBO_MSG_STA_PREF_UPDATE);
		if (mac_entry->bIndicateCDC)
			MboIndicateStaInfoToDaemon(pAd, &mac_entry->MboStaInfoCDC, MBO_MSG_CDC_UPDATE);
#endif /* MBO_SUPPORT */

	}

	return 0;
}

/* client disaccos */
INT wapp_send_cli_leave_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr)
{
	struct wapp_event event;
	wapp_client_info *cli_info;

	event.event_id = WAPP_CLI_LEAVE_EVENT;
	event.ifindex = ifindex;
	cli_info = &event.data.cli_info;

	COPY_MAC_ADDR(cli_info->mac_addr, mac_addr);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);

	return 0;
}


INT wapp_send_apcli_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	u8 i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_client_info *cli_info;
	MAC_TABLE_ENTRY *mac_entry;
	PAPCLI_STRUCT apcli_entry;

	event.event_id = WAPP_APCLI_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	cli_info = &event.data.cli_info;

	for (i = 0; i < MAX_APCLI_NUM; i++) {
		apcli_entry = &pAd->ApCfg.ApCliTab[i];
		wdev = &apcli_entry->wdev;
		if (pAd->ApCfg.ApCliTab[i].Valid == TRUE
			&& wdev->if_dev
			&& RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
			mac_entry = &pAd->MacTab.Content[apcli_entry->MacTabWCID];
			if (IS_ENTRY_APCLI(mac_entry)) {
				wapp_fill_client_info(pAd, cli_info, mac_entry);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
				break;
			}
		}
	}

	return 0;
}


INT wapp_send_cli_probe_event(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *mac_addr,
	MLME_QUEUE_ELEM *elem)
{
	struct wapp_event event;
	wapp_probe_info *probe_info;
	struct raw_rssi_info *raw_rssi;
	RSSI_SAMPLE rssi;

	raw_rssi = &elem->rssi_info;
	rssi.AvgRssi[0] = raw_rssi->raw_rssi[0] ? ConvertToRssi(pAd, raw_rssi, RSSI_IDX_0) : 0;
	rssi.AvgRssi[1] = raw_rssi->raw_rssi[1] ? ConvertToRssi(pAd, raw_rssi, RSSI_IDX_1) : 0;
	rssi.AvgRssi[2] = raw_rssi->raw_rssi[2] ? ConvertToRssi(pAd, raw_rssi, RSSI_IDX_2) : 0;
	rssi.AvgRssi[3] = raw_rssi->raw_rssi[3] ? ConvertToRssi(pAd, raw_rssi, RSSI_IDX_3) : 0;

	event.event_id = WAPP_CLI_PROBE_EVENT;
	event.ifindex = ifindex;
	probe_info = &event.data.probe_info;
	COPY_MAC_ADDR(probe_info->mac_addr, mac_addr);
	probe_info->channel = elem->Channel;
	probe_info->rssi = RTMPAvgRssi(pAd, &rssi);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

VOID wapp_send_bcn_report(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR report,
	IN ULONG report_len)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Sta Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
				__func__, PRINT_MAC(pEntry->Addr)));
		wdev = pEntry->wdev;
		event.event_id = WAPP_RCEV_BCN_REPORT;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);

		COPY_MAC_ADDR(event.data.bcn_rpt_info.sta_addr, pEntry->Addr);
		event.data.bcn_rpt_info.bcn_rpt_len = report_len;
		if (report_len > BCN_RPT_LEN)
			report_len = BCN_RPT_LEN;
		NdisCopyMemory(event.data.bcn_rpt_info.bcn_rpt, report, report_len);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}


VOID wapp_send_bcn_report_complete(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Sta Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
				__func__, PRINT_MAC(pEntry->Addr)));
		wdev = pEntry->wdev;
		event.event_id = WAPP_RCEV_BCN_REPORT_COMPLETE;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		COPY_MAC_ADDR(event.data.bcn_rpt_info.sta_addr, pEntry->Addr);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}

#ifdef AIR_MONITOR
VOID wapp_send_air_mnt_rssi(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PMNT_STA_ENTRY pMntEntry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_MONITOR(pEntry)) {
		wdev = pEntry->wdev;
		event.event_id = WAPP_RCEV_MONITOR_INFO;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.mnt_info.rssi = RTMPAvgRssi(pAd, &pMntEntry->RssiSample);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}
#endif

VOID wapp_send_cli_active_change(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		wdev = pEntry->wdev;
		event.event_id = WAPP_CLI_ACTIVE_CHANGE;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.cli_info.bIsActiveChange = 1;
		COPY_MAC_ADDR(event.data.cli_info.mac_addr, pEntry->Addr);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}

VOID setChannelList(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_chn_info *chn_list)
{
	int i = 0;

	for (i = 0; i < pAd->ChannelListNum; i++) {
		chn_list->ch_list[i].channel =  pAd->ChannelList[i].Channel;

		/* Set Preference & reason */
		if (pAd->ChannelList[i].DfsReq) {
		}
		if (pAd->ChannelList[i].Channel == wlan_operate_get_cen_ch_1(wdev)) {
		}
	}

}


INT wapp_send_chn_list_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_chn_info *chn_list;

	event.event_id = WAPP_CHN_LIST_RSP;
	event.ifindex = req->data.ifindex;
	chn_list = &event.data.chn_list;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				chn_list->band = wdev->PhyMode;
				chn_list->op_ch = wlan_operate_get_prim_ch(wdev);
				chn_list->op_class = get_regulatory_class(pAd, wdev->channel, wdev->PhyMode, wdev);
				chn_list->ch_list_num = pAd->ChannelListNum;
				setChannelList(pAd, wdev, chn_list);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}


INT wapp_send_op_class_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_op_class_info *op_class;

	event.event_id = WAPP_OP_CLASS_RSP;
	event.ifindex = req->data.ifindex;
	op_class = &event.data.op_class;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_bss_info_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_bss_info *bss_info;

	event.event_id = WAPP_BSS_INFO_RSP;
	event.ifindex = req->data.ifindex;
	bss_info = &event.data.bss_info;


	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				if (i >= HW_BEACON_MAX_NUM || wdev->wdev_type != WDEV_TYPE_AP) {
					MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("%s():wdev %d is not an AP\n", __func__, i));
					break;
				}
				bss_info->SsidLen = pAd->ApCfg.MBSSID[i].SsidLen;
				NdisMoveMemory(bss_info->ssid, pAd->ApCfg.MBSSID[i].Ssid, (MAX_LEN_OF_SSID+1));
				NdisMoveMemory(bss_info->bssid, wdev->bssid, MAC_ADDR_LEN);
				NdisMoveMemory(bss_info->if_addr, wdev->if_addr, MAC_ADDR_LEN);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}


INT wapp_send_ap_metric_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ap_metric *ap_metric;
	BSS_STRUCT *mbss;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	event.event_id = WAPP_AP_METRIC_RSP;
	event.ifindex = req->data.ifindex;
	ap_metric = &event.data.ap_metrics;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				NdisMoveMemory(ap_metric->bssid, wdev->bssid, MAC_ADDR_LEN);
				ap_metric->cu = pQloadCtrl->QloadChanUtil;
				NdisCopyMemory(ap_metric->ESPI_AC_BE, mbss->ESPI_AC_BE, sizeof(mbss->ESPI_AC_BE));
				NdisCopyMemory(ap_metric->ESPI_AC_BK, mbss->ESPI_AC_BK, sizeof(mbss->ESPI_AC_BK));
				NdisCopyMemory(ap_metric->ESPI_AC_VO, mbss->ESPI_AC_VO, sizeof(mbss->ESPI_AC_VO));
				NdisCopyMemory(ap_metric->ESPI_AC_VI, mbss->ESPI_AC_VI, sizeof(mbss->ESPI_AC_VI));
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}


INT wapp_send_ch_util_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	event.event_id = WAPP_CH_UTIL_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	event.data.ch_util = pQloadCtrl->QloadChanUtil;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_ap_config_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_ap_config *ap_conf;

	event.event_id = WAPP_AP_CONFIG_RSP;
	event.ifindex = req->data.ifindex;
	ap_conf = &event.data.ap_conf;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}


INT wapp_send_tx_power_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_tx_power *pwr;

	event.event_id = WAPP_TX_POWER_RSP;
	event.ifindex = req->data.ifindex;
	pwr = &event.data.tx_pwr;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				pwr->tx_pwr = RTMP_GetTxPwr(pAd, wdev->rate.MlmeTransmit, wdev->channel, wdev);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_bss_stop_rsp(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	event.event_id = WAPP_BSS_STOP_RSP;
	event.ifindex = wdev->if_dev->ifindex;
	event.data.bss_stop_info.interface_index = event.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_ch_change_rsp(
	PRTMP_ADAPTER pAd,
	MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	struct wapp_event event;

	event.event_id = WAPP_CH_CHANGE;
	event.ifindex = pAd->ApCfg.MBSSID[0].wdev.if_dev->ifindex;
	event.data.ch_change_info.interface_index = event.ifindex;
	event.data.ch_change_info.new_ch = (u_int8_t)SwChCfg.ControlChannel;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

/*
*	INT wapp_send_txpower_change_rsp(
*	PRTMP_ADAPTER pAd,
*	)
*	{
*		struct wapp_event event;
*		event.event_id = WAPP_TX_POWER_CHANGE;
*		event.ifindex = ;
*		event.data.txpwr_change_info.interface_index =event.ifindex;
*		event.data.txpwr_change_info.new_tx_pwr =;
*		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
*		return 0;
*	}
*/

INT wapp_send_apcli_association_change(
	u8 event_id,
	PRTMP_ADAPTER pAd,
	PAPCLI_STRUCT pApCliEntry)
{
	struct wapp_event event;

	event.event_id = event_id;
	event.ifindex = pApCliEntry->wdev.if_dev->ifindex;
	event.data.apcli_association_info.interface_index = event.ifindex;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_bss_start(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex)
				APStartUpByBss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
		}
	}
	return 0;
}

INT wapp_bss_stop(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("%s():req->data.ifindex = %d\n", __func__, req->data.ifindex));
				APStopByBss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
			}
		}
	}
	return 0;
}

INT wapp_config_ap_setting(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
			}
		}
	}
	return 0;
}

/* set Tx Power Perventage */
INT wapp_set_tx_power_prctg(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	UINT8 prctg = (UINT8) req->data.value;
	UCHAR Band_Idx = 0;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				Band_Idx = HcGetBandByWdev(wdev);
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) ifindex = %u, prctg = %u\n",
						__func__, req->data.ifindex, prctg));
				if (prctg > 0) {
					TxPowerPercentCtrl(pAd, TRUE, Band_Idx);
					TxPowerDropCtrl(pAd, prctg, Band_Idx);
				} else {
					TxPowerPercentCtrl(pAd, FALSE, Band_Idx);
				}
			}
		}
	}
	return 0;
}

INT wapp_set_steer_policy(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
			}
		}
	}
	return 0;
}

INT wapp_send_bssload_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	QLOAD_CTRL *pQloadCtrl = HcGetQloadCtrl(pAd);

	event.event_id = WAPP_BSSLOAD_RSP;
	event.ifindex = req->data.ifindex;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				event.data.bssload_info.sta_cnt = MacTableAssocStaNumGet(pAd);
				event.data.bssload_info.ch_util = pQloadCtrl->QloadChanUtil;
				event.data.bssload_info.AvalAdmCap = (0x7a12); /* 0x7a12 * 32us = 1 second */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}

	return 0;
}

INT wapp_send_he_cap_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{

	return 0;
}

INT wapp_send_sta_rssi_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	PMAC_TABLE_ENTRY mac_entry;
	wapp_client_info *cli_info;

	event.event_id = WAPP_STA_RSSI_RSP;
	event.ifindex = req->data.ifindex;
	cli_info = &event.data.cli_info;

	mac_entry = MacTableLookup(pAd, req->data.mac_addr);
	if (mac_entry && IS_ENTRY_CLIENT(mac_entry)) {
		if (req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);
			cli_info->uplink_rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

	return 0;
}

INT	wapp_event_handle(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	switch (req->req_id) {
	case WAPP_DEV_QUERY_REQ:
		wapp_send_wdev_query_rsp(pAd);
		break;
	case WAPP_HT_CAP_QUERY_REQ:
		wapp_send_wdev_ht_cap_rsp(pAd, req);
		break;
	case WAPP_VHT_CAP_QUERY_REQ:
		wapp_send_wdev_vht_cap_rsp(pAd, req);
		break;
	case WAPP_MISC_CAP_QUERY_REQ:
		wapp_send_wdev_misc_cap_rsp(pAd, req);
		break;
	case WAPP_CLI_QUERY_REQ:
		wapp_send_cli_query_rsp(pAd, req);
		break;
	case WAPP_CLI_LIST_QUERY_REQ:
		wapp_handle_cli_list_query(pAd, req);
		break;
	case WAPP_CHN_LIST_QUERY_REQ:
		wapp_send_chn_list_query_rsp(pAd, req);
		break;
	case WAPP_OP_CLASS_QUERY_REQ:
		wapp_send_op_class_query_rsp(pAd, req);
		break;
	case WAPP_BSS_INFO_QUERY_REQ:
		wapp_send_bss_info_query_rsp(pAd, req);
		break;
	case WAPP_AP_METRIC_QUERY_REQ:
		wapp_send_ap_metric_query_rsp(pAd, req);
		break;
	case WAPP_CH_UTIL_QUERY_REQ:
		wapp_send_ch_util_query_rsp(pAd, req);
		break;
	case WAPP_TX_POWER_QUERT_REQ:
		wapp_send_tx_power_query_rsp(pAd, req);
		break;
	case WAPP_APCLI_QUERY_REQ:
		wapp_send_apcli_query_rsp(pAd, req);
		break;
	case WAPP_BSS_START_REQ:
		wapp_bss_start(pAd, req);
		break;
	case WAPP_BSS_STOP_REQ:
		wapp_bss_stop(pAd, req);
		break;
	case WAPP_TXPWR_PRCTG_REQ:
		wapp_set_tx_power_prctg(pAd, req);
		break;
	case WAPP_STEERING_POLICY_SET_REQ:
		wapp_set_steer_policy(pAd, req);
		break;
	case WAPP_AP_CONFIG_SET_REQ:
		wapp_config_ap_setting(pAd, req);
		break;
	case WAPP_BSSLOAD_QUERY_REQ:
		wapp_send_bssload_query_rsp(pAd, req);
		break;
	case WAPP_HECAP_QUERY_REQ:
		wapp_send_he_cap_query_rsp(pAd, req);
		break;
	case WAPP_STA_RSSI_QUERY_REQ:
		wapp_send_sta_rssi_query_rsp(pAd, req);
		break;
	default:
		break;
	}
	return 0;
}


BOOLEAN wapp_init(
	PRTMP_ADAPTER pAd,
	BSS_STRUCT *pMbss
)
{
	NdisCopyMemory(pMbss->ESPI_AC_BE, ESPI_AC_BE_DEFAULT, sizeof(pMbss->ESPI_AC_BE));
	NdisCopyMemory(pMbss->ESPI_AC_BK, ESPI_AC_BK_DEFAULT, sizeof(pMbss->ESPI_AC_BK));
	NdisCopyMemory(pMbss->ESPI_AC_VO, ESPI_AC_VO_DEFAULT, sizeof(pMbss->ESPI_AC_VO));
	NdisCopyMemory(pMbss->ESPI_AC_VI, ESPI_AC_VI_DEFAULT, sizeof(pMbss->ESPI_AC_VI));
	return 0;
}

#endif /* WAPP_SUPPORT */
