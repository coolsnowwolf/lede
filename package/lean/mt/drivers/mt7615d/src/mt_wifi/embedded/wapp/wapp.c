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

UINT8 get_channel_utilization(PRTMP_ADAPTER pAd, u32 ifindex)
{
	UCHAR	i, Channel;
	UINT16 	l;
	UINT32  ObssAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyTxAirTime[DBDC_BAND_NUM] = {0};
	UINT32  MyRxAirTime[DBDC_BAND_NUM] = {0};
	UCHAR ObssAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UCHAR MyAirOccupyPercentage[DBDC_BAND_NUM] = {0};
	UINT32  res;
	struct wifi_dev *wdev = NULL;

	for (l = 0; l < WDEV_NUM_MAX; l++) {
		if (pAd->wdev_list[l] != NULL) {
			wdev = pAd->wdev_list[l];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == ifindex) {
				break;
			}
		}
	}

	if (wdev == NULL) {
		return 0;
	}

	Channel = wdev->channel;
	i = HcGetBandByChannel(pAd, Channel);
	ObssAirTime[i] = Get_OBSS_AirTime(pAd, i);
	MyTxAirTime[i] = Get_My_Tx_AirTime(pAd, i);
	MyRxAirTime[i] = Get_My_Rx_AirTime(pAd, i);

	if (ObssAirTime[i] != 0)
		ObssAirOccupyPercentage[i] = (ObssAirTime[i]*100)/ONE_SEC_2_US;

	if (MyTxAirTime[i] != 0 || MyRxAirTime[i] != 0)
		MyAirOccupyPercentage[i] = ((MyTxAirTime[i] + MyRxAirTime[i]) * 100)/ONE_SEC_2_US;

	res = (MyAirOccupyPercentage[i] + ObssAirOccupyPercentage[i]);
	/* convert to a scale of 255 */
	res *= 255;
	res = (res / 100);

	return res;
}

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
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wapp_dev_info *dev_info;

	event.event_id = WAPP_DEV_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	dev_info = &event.data.dev_info;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				dev_info->ifindex = event.ifindex;
				dev_info->dev_type = wdev->wdev_type;
				COPY_MAC_ADDR(dev_info->mac_addr, wdev->if_addr);
				NdisCopyMemory(dev_info->ifname, RtmpOsGetNetDevName(wdev->if_dev), IFNAMSIZ);
				dev_info->radio_id = HcGetBandByWdev(wdev);
				dev_info->adpt_id = (uintptr_t) pAd;
				dev_info->wireless_mode = wdev->PhyMode;
				dev_info->dev_active = HcIsRadioAcq(wdev);
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
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

				drv_vht_op.basic_mcs_set.mcs_ss1 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss2 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss3 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss4 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss5 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss6 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss7 = VHT_MCS_CAP_NA;
				drv_vht_op.basic_mcs_set.mcs_ss8 = VHT_MCS_CAP_NA;

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
	NdisZeroMemory(misc_cap, sizeof(wdev_misc_cap));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				event.ifindex = req->data.ifindex;
				misc_cap->max_num_of_cli = 64;
				misc_cap->max_num_of_bss = HW_BEACON_MAX_NUM;
				misc_cap->num_of_bss = pAd->ApCfg.BssidNum;
#ifdef CONFIG_MAP_SUPPORT
				misc_cap->max_num_of_block_cli = BLOCK_LIST_NUM;
#endif
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

	ULONG DataRate = 0, DataRate_r = 0;
	HTTRANSMIT_SETTING HTPhyMode;
	EXT_EVENT_TX_STATISTIC_RESULT_T rTxStatResult;
	HTTRANSMIT_SETTING LastTxRate;

	MtCmdGetTxStatistic(pAd, GET_TX_STAT_ENTRY_TX_RATE, 0/*Don't Care*/, mac_entry->wcid, &rTxStatResult);
	LastTxRate.field.MODE = rTxStatResult.rEntryTxRate.MODE;
	LastTxRate.field.BW = rTxStatResult.rEntryTxRate.BW;
	LastTxRate.field.ldpc = rTxStatResult.rEntryTxRate.ldpc ? 1 : 0;
	LastTxRate.field.ShortGI = rTxStatResult.rEntryTxRate.ShortGI ? 1 : 0;
	LastTxRate.field.STBC = rTxStatResult.rEntryTxRate.STBC;

	if (LastTxRate.field.MODE == MODE_VHT)
		LastTxRate.field.MCS = (((rTxStatResult.rEntryTxRate.VhtNss - 1) & 0x3) << 4) + rTxStatResult.rEntryTxRate.MCS;
	else if (LastTxRate.field.MODE == MODE_OFDM)
		LastTxRate.field.MCS = getLegacyOFDMMCSIndex(rTxStatResult.rEntryTxRate.MCS) & 0x0000003F;
	else
		LastTxRate.field.MCS = rTxStatResult.rEntryTxRate.MCS;

	mac_entry->LastTxRate = (UINT32)LastTxRate.word;

	tr_entry = &pAd->MacTab.tr_entry[mac_entry->wcid];

	COPY_MAC_ADDR(cli_info->mac_addr, mac_entry->Addr);
	COPY_MAC_ADDR(cli_info->bssid, mac_entry->wdev->bssid);
	cli_info->sta_status = \
				(tr_entry->PortSecured == WPA_802_1X_PORT_SECURED) ?
				WAPP_STA_CONNECTED : WAPP_STA_DISCONNECTED;
	cli_info->assoc_time = mac_entry->StaConnectTime;
#ifdef CONFIG_MAP_SUPPORT
	NdisCopyMemory(cli_info->assoc_req, mac_entry->assoc_req_frame, ASSOC_REQ_LEN);
	cli_info->assoc_req_len = mac_entry->assoc_req_len;
#endif /* MAP_SUPPORT */
	HTPhyMode.word = (USHORT)mac_entry->LastTxRate;
	getRate(HTPhyMode, &DataRate);
	cli_info->downlink = (u16) DataRate;

	/* Though NSS1VHT20MCS9 and NSS2VHT20MCS9 rates are not specified in
	 * IEEE802.11, we do use them */
	if ((HTPhyMode.field.MODE == MODE_VHT) && (HTPhyMode.field.BW == BW_20) &&
			((HTPhyMode.field.MCS & 0xf) == 9)) {
		u8 vht_nss = ((HTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;
		if (vht_nss == 1)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 96 : 86;
		else if (vht_nss == 2)
			cli_info->downlink = HTPhyMode.field.ShortGI ? 192 : 173;
	}

	HTPhyMode.word = (USHORT) mac_entry->LastRxRate;
	getRate(HTPhyMode, &DataRate_r);
	cli_info->uplink = (u16) DataRate_r;

	cli_info->uplink_rssi = RTMPAvgRssi(pAd, &mac_entry->RssiSample);
#ifdef CONFIG_DOT11V_WNM
	cli_info->cli_caps.btm_capable = mac_entry->bBSSMantSTASupport == TRUE ? 1 : 0;
#endif
	cli_info->bLocalSteerDisallow = false;
#ifdef DOT11K_RRM_SUPPORT
	if ((mac_entry->RrmEnCap.field.BeaconPassiveMeasureCap ||
		 mac_entry->RrmEnCap.field.BeaconActiveMeasureCap)) {
		cli_info->cli_caps.rrm_capable = 1;
	} else {
		cli_info->cli_caps.rrm_capable = 0;
	}
#endif
#ifdef MBO_SUPPORT
	cli_info->cli_caps.mbo_capable = (mac_entry->bIndicateCDC || mac_entry->bIndicateNPC) ? 1 : 0;
#endif
	/* Phy Caps */
	cli_info->cli_caps.phy_mode = mac_entry->MaxHTPhyMode.field.MODE;
	cli_info->cli_caps.bw = mac_entry->MaxHTPhyMode.field.BW;
	cli_info->cli_caps.nss = ((mac_entry->MaxHTPhyMode.field.MCS & (0x3 << 4)) >> 4) + 1;

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
	cli_info->tx_tp = (u32)(mac_entry->AvgTxBytes);
	cli_info->rx_tp = (u32)(mac_entry->AvgRxBytes);

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
#ifdef APCLI_SUPPORT
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
#endif

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
	if (elem->MsgLen <= PREQ_IE_LEN) {
		NdisCopyMemory(probe_info->preq, elem->Msg, elem->MsgLen);
		probe_info->preq_len = elem->MsgLen;
	} else {
		NdisCopyMemory(probe_info->preq, elem->Msg, PREQ_IE_LEN);
		probe_info->preq_len = PREQ_IE_LEN;
	}

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
	struct wapp_event *event = NULL;
	UCHAR count = 0, i = 0;
	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		os_alloc_mem(pAd, (UCHAR **)&event, sizeof(*event));
		if (!event)
			return;
		NdisZeroMemory(event, sizeof(*event));
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("(%s) Sta Addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
				__func__, PRINT_MAC(pEntry->Addr)));
		wdev = pEntry->wdev;
		event->event_id = WAPP_RCEV_BCN_REPORT;
		event->ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);

		COPY_MAC_ADDR(event->data.bcn_rpt_info.sta_addr, pEntry->Addr);

		if (report_len % BCN_RPT_LEN)
			count = report_len/BCN_RPT_LEN + 1;
		else
			count = report_len/BCN_RPT_LEN;

		for (i = 0 ; i < count ; i++) {
			if (i == (count - 1)) {
				event->data.bcn_rpt_info.bcn_rpt_len = report_len;
				event->data.bcn_rpt_info.last_fragment = 1;
				NdisCopyMemory(event->data.bcn_rpt_info.bcn_rpt, &report[i*BCN_RPT_LEN], report_len);
			} else {
				event->data.bcn_rpt_info.bcn_rpt_len = BCN_RPT_LEN;
				event->data.bcn_rpt_info.last_fragment = 0;
				NdisCopyMemory(event->data.bcn_rpt_info.bcn_rpt, &report[i*BCN_RPT_LEN], BCN_RPT_LEN);
				report_len -= BCN_RPT_LEN;
			}
			wext_send_wapp_qry_rsp(pAd->net_dev, event);
		}
		os_free_mem(event);
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
		COPY_MAC_ADDR(event.data.mnt_info.sta_addr, pMntEntry->addr);
		event.data.mnt_info.rssi = RTMPMaxRssi(pAd,
									pMntEntry->RssiSample.AvgRssi[0],
									pMntEntry->RssiSample.AvgRssi[1],
									pMntEntry->RssiSample.AvgRssi[2]
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
									, pMntEntry->RssiSample.AvgRssi[3]
#endif
									);
		wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	}
}
#endif

VOID wapp_send_csa_event(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 ifindex,
	IN UCHAR new_channel)
{
	struct wapp_event event;
	wapp_csa_info *csa_info;

	event.event_id = WAPP_CSA_EVENT;
	event.ifindex = ifindex;
	csa_info = &event.data.csa_info;
	csa_info->new_channel = new_channel;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
}

VOID wapp_send_cli_active_change(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN STA_STATUS stat)
{
	struct wifi_dev *wdev;
	struct wapp_event event;

	if (pEntry && IS_ENTRY_CLIENT(pEntry)) {
		wdev = pEntry->wdev;
		event.event_id = WAPP_CLI_ACTIVE_CHANGE;
		event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
		event.data.cli_info.status = stat;
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
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
			chn_list->ch_list[i].pref |= (OP_DISALLOWED_DUE_TO_DFS | NON_PREF);
#endif /* CONFIG_MAP_SUPPORT */
		}
		if (pAd->ChannelList[i].Channel == wlan_operate_get_cen_ch_1(wdev)) {
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
			chn_list->ch_list[i].pref |= PREF_SCORE_14;
#endif /* CONFIG_MAP_SUPPORT */
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
				chn_list->dl_mcs = wdev->HTPhyMode.field.MCS;
				setChannelList(pAd, wdev, chn_list);
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
				chn_list->non_op_chn_num = getNonOpChnNum(pAd, wdev, chn_list->op_class);
				setNonOpChnList(pAd,
								wdev,
								chn_list->non_op_ch_list,
								chn_list->non_op_chn_num,
								chn_list->op_class);
#endif /* CONFIG_MAP_SUPPORT */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

#ifdef CONFIG_MAP_SUPPORT
UCHAR map_set_op_class_info(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	wdev_op_class_info *op_class);
#endif /* CONFIG_MAP_SUPPORT */

INT wapp_send_op_class_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i;
	struct wifi_dev *wdev;
	struct wapp_event event;
	wdev_op_class_info *op_class;

	NdisZeroMemory((void *)&event, sizeof(struct wapp_event));
	event.event_id = WAPP_OP_CLASS_RSP;
	event.ifindex = req->data.ifindex;
	op_class = &event.data.op_class;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
#ifdef CONFIG_MAP_SUPPORT
				op_class->num_of_op_class = map_set_op_class_info(pAd, wdev, op_class);
#endif /* CONFIG_MAP_SUPPORT */
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

	NdisZeroMemory(&event, sizeof(event));
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
#ifdef CONFIG_MAP_SUPPORT
				bss_info->map_role = wdev->MAPCfg.DevOwnRole;
				bss_info->auth_mode = pAd->ApCfg.MBSSID[i].wdev.SecConfig.AKMMap;
				bss_info->enc_type = pAd->ApCfg.MBSSID[i].wdev.SecConfig.PairwiseCipher;
#ifdef WSC_AP_SUPPORT
				bss_info->key_len = strlen(pAd->ApCfg.MBSSID[i].wdev.WscControl.WpaPsk);
				NdisMoveMemory(bss_info->key,
					pAd->ApCfg.MBSSID[i].wdev.WscControl.WpaPsk, bss_info->key_len);
#else
				bss_info->key_len = strlen(pAd->ApCfg.MBSSID[i].PSK);
				NdisMoveMemory(bss_info->key, pAd->ApCfg.MBSSID[i].PSK, bss_info->key_len);
#endif
				bss_info->hidden_ssid = pAd->ApCfg.MBSSID[i].bHideSsid;
#endif
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

	event.event_id = WAPP_AP_METRIC_RSP;
	event.ifindex = req->data.ifindex;
	ap_metric = &event.data.ap_metrics;
	NdisZeroMemory(ap_metric, sizeof(wdev_ap_metric));

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				mbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				NdisMoveMemory(ap_metric->bssid, wdev->bssid, MAC_ADDR_LEN);
				ap_metric->cu = get_channel_utilization(pAd, event.ifindex);
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
	event.event_id = WAPP_CH_UTIL_QUERY_RSP;
	event.ifindex = req->data.ifindex;
	event.data.ch_util = get_channel_utilization(pAd, event.ifindex);
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
#ifdef CONFIG_MAP_SUPPORT /* TODO: move to MAP */
				ap_conf->sta_report_on_cop = wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss;
				ap_conf->sta_report_not_cop = wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss;
				ap_conf->rssi_steer = wdev->MAPCfg.bAgentInitRssiSteering;
#endif /* CONFIG_MAP_SUPPORT */
				wext_send_wapp_qry_rsp(pAd->net_dev, &event);
			}
		}
	}
	return 0;
}

INT wapp_send_bss_state_change(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UINT8 bss_state)
{
	struct wapp_event event;

	event.event_id = WAPP_BSS_STATE_CHANGE;
	event.ifindex = wdev->if_dev->ifindex;
	event.data.bss_state_info.interface_index = event.ifindex;
	event.data.bss_state_info.bss_state = bss_state;
	wext_send_wapp_qry_rsp(ad->net_dev, &event);
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
	UINT8 apcli_assoc_state,
	struct _RTMP_ADAPTER *ad,
	struct _APCLI_STRUCT *ApCliEntry)
{
	struct wapp_event event;

	event.event_id = WAPP_APCLI_ASSOC_STATE_CHANGE;
	event.ifindex = ApCliEntry->wdev.if_dev->ifindex;
	event.data.apcli_association_info.interface_index = event.ifindex;
	event.data.apcli_association_info.apcli_assoc_state = apcli_assoc_state;
	wext_send_wapp_qry_rsp(ad->net_dev, &event);
	return 0;
}

#ifdef CONVERTER_MODE_SWITCH_SUPPORT

INT wapp_send_apcli_association_change_vendor10(
	UINT8 apcli_assoc_state,
	struct _RTMP_ADAPTER *ad,
	struct _APCLI_STRUCT *ApCliEntry)
{
	struct wapp_event event;

	event.event_id = WAPP_APCLI_ASSOC_STATE_CHANGE_VENDOR10;
	event.ifindex = ApCliEntry->wdev.if_dev->ifindex;
	event.data.apcli_association_info.interface_index = event.ifindex;
	event.data.apcli_association_info.apcli_assoc_state = apcli_assoc_state;
	wext_send_wapp_qry_rsp(ad->net_dev, &event);
	return 0;
}
#endif /* CONVERTER_MODE_SWITCH_SUPPORT */


INT wapp_send_bssload_crossing(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR bssload_high_thrd,
	UCHAR bssload_low_thrd,
	UCHAR bssload)
{
	struct wapp_event event;

	event.event_id = WAPP_BSSLOAD_CROSSING;
	event.ifindex = wdev->if_dev->ifindex;
	event.data.bssload_crossing_info.interface_index = event.ifindex;
	event.data.bssload_crossing_info.bssload = bssload;
	event.data.bssload_crossing_info.bssload_high_thrd = bssload_high_thrd;
	event.data.bssload_crossing_info.bssload_low_thrd = bssload_low_thrd;
	wext_send_wapp_qry_rsp(ad->net_dev, &event);
	return 0;
}

INT wapp_send_sta_connect_rejected(
	struct _RTMP_ADAPTER *ad,
	struct wifi_dev *wdev,
	UCHAR *sta_mac_addr,
	UCHAR *bssid,
	UINT8 connect_stage,
	UINT16 reason)
{
	struct wapp_event event;

	event.event_id = WAPP_STA_CNNCT_REJ;
	event.ifindex = wdev->if_dev->ifindex;
	event.data.sta_cnnct_rej_info.interface_index = event.ifindex;
	os_move_mem(event.data.sta_cnnct_rej_info.bssid, bssid, MAC_ADDR_LEN);
	os_move_mem(event.data.sta_cnnct_rej_info.sta_mac, sta_mac_addr, MAC_ADDR_LEN);
	event.data.sta_cnnct_rej_info.cnnct_fail.connect_stage = connect_stage;
	event.data.sta_cnnct_rej_info.cnnct_fail.reason = reason;
	wext_send_wapp_qry_rsp(ad->net_dev, &event);
	return TRUE;
}

INT wapp_bss_start(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	INT i, j;
	struct wifi_dev *wdev, *wdev_active;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				for (j = 0; j < WDEV_NUM_MAX; j++) {
					wdev_active = pAd->wdev_list[j];
					if (wdev_active && HcIsRadioAcq(wdev_active) &&
						(HcGetBandByWdev(wdev) == HcGetBandByWdev(wdev_active))) {
						wdev->channel = wdev_active->channel;
						break;
					}
				}
				APStartUpByBss(pAd, &pAd->ApCfg.MBSSID[wdev->func_idx]);
				break;
			}
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
	BSS_STRUCT *pMbss;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == req->data.ifindex) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_INFO,
						("%s():req->data.ifindex = %d\n", __func__, req->data.ifindex));
				pMbss = &pAd->ApCfg.MBSSID[wdev->func_idx];
				APStop(pAd, pMbss, AP_BSS_OPER_SINGLE);
				RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_SYSEM_READY);
			}
		}
	}
	return 0;
}

INT wapp_bss_load_thrd_set(
	struct _RTMP_ADAPTER *ad,
	struct wapp_req *req)
{
	int i = 0;
	UINT8 band_id;
	UINT8 high_thrd;
	UINT8 low_thrd;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (ad->wdev_list[i] != NULL) {
			wdev = ad->wdev_list[i];
			if (wdev->if_dev->ifindex == req->data.ifindex) {
				band_id = HcGetBandByWdev(wdev);
				high_thrd = req->data.bssload_thrd.high_bssload_thrd;
				low_thrd = req->data.bssload_thrd.low_bssload_thrd;
				ad->bss_load_info.high_thrd[band_id] = high_thrd;
				ad->bss_load_info.low_thrd[band_id] = low_thrd;
				return 0;
			}
		}
	}

	MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
						("wdev not found\n"));
	return 1;
}

/*
 *==========================================================================
 *	Description:
 *		Periodically check the BSS load. Once the load status changes into an abnormal situation (above the
 *		high threshold/below the low threshold), the WAPP event will be sent. Notice that the event won't
 *		be sent if the load status stays in the same abnormal situation.
 *==========================================================================
 */
VOID wapp_bss_load_check(
	struct _RTMP_ADAPTER *ad)
{
	int i;
	UINT8 band_id;
	UINT8 new_load;
	UINT8 new_status;
	UINT8 current_status;
	UINT8 high_thrd;
	UINT8 low_thrd;
	ULONG up_time;
	struct wifi_dev *wdev;

	NdisGetSystemUpTime(&up_time);
#ifdef AP_QLOAD_SUPPORT
	QBSS_LoadUpdate(ad, up_time);
#endif /* AP_QLOAD_SUPPORT */
	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		if ((wdev != NULL) && (wdev->wdev_type == WDEV_TYPE_AP)) {
			band_id = HcGetBandByWdev(wdev);
			high_thrd = ad->bss_load_info.high_thrd[band_id];
			low_thrd = ad->bss_load_info.low_thrd[band_id];
			current_status = ad->bss_load_info.current_status[band_id];
			new_load = get_channel_utilization(ad, wdev->if_dev->ifindex);

			if (new_load >= high_thrd)
				new_status = WAPP_BSSLOAD_HIGH;
			else if (new_load <= low_thrd)
				new_status = WAPP_BSSLOAD_LOW;
			else
				new_status = WAPP_BSSLOAD_NORMAL;

			/*The status changes & the new status is an abnormal situation*/
			if (new_status != current_status && new_status != WAPP_BSSLOAD_NORMAL)
				wapp_send_bssload_crossing(ad, wdev, high_thrd, low_thrd, new_load);

			ad->bss_load_info.current_load[band_id] = new_load;
			ad->bss_load_info.current_status[band_id] = new_status;
		}
	}
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
#ifdef CONFIG_MAP_SUPPORT
				wdev->MAPCfg.bUnAssocStaLinkMetricRptOpBss = req->data.ap_conf.sta_report_on_cop;
				wdev->MAPCfg.bUnAssocStaLinkMetricRptNonOpBss = req->data.ap_conf.sta_report_not_cop;
				wdev->MAPCfg.bAgentInitRssiSteering = req->data.ap_conf.rssi_steer;
#endif
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
#ifdef CONFIG_MAP_SUPPORT
				pAd->ApCfg.SteerPolicy.steer_policy = req->data.str_policy.steer_policy;
				pAd->ApCfg.SteerPolicy.cu_thr = req->data.str_policy.cu_thr;
				pAd->ApCfg.SteerPolicy.rcpi_thr = req->data.str_policy.rcpi_thr;
#endif
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

	event.event_id = WAPP_BSSLOAD_RSP;
	event.ifindex = req->data.ifindex;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		if (pAd->wdev_list[i] != NULL) {
			wdev = pAd->wdev_list[i];
			if (RtmpOsGetNetIfIndex(wdev->if_dev) == event.ifindex) {
				event.data.bssload_info.sta_cnt = MacTableAssocStaNumGet(pAd);
				event.data.bssload_info.ch_util = get_channel_utilization(pAd, event.ifindex);
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

INT wapp_send_apcli_rssi_query_rsp(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	struct wapp_event event;
	PMAC_TABLE_ENTRY mac_entry;
	wapp_apcli_association_info *apcli_info;

	event.event_id = WAPP_APCLI_RSSI_RSP;
	event.ifindex = req->data.ifindex;
	apcli_info = &event.data.apcli_association_info;

	mac_entry = MacTableLookup(pAd, req->data.mac_addr);
	if (mac_entry && IS_ENTRY_APCLI(mac_entry)) {
		if (req->data.ifindex == RtmpOsGetNetIfIndex(mac_entry->wdev->if_dev)) {
			apcli_info->rssi = RTMPMaxRssi(pAd,
									mac_entry->RssiSample.AvgRssi[0],
									mac_entry->RssiSample.AvgRssi[1],
									mac_entry->RssiSample.AvgRssi[2]
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
									, mac_entry->RssiSample.AvgRssi[3]
#endif
									);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

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
			cli_info->uplink_rssi = RTMPMaxRssi(pAd,
									mac_entry->RssiSample.AvgRssi[0],
									mac_entry->RssiSample.AvgRssi[1],
									mac_entry->RssiSample.AvgRssi[2]
#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
									, mac_entry->RssiSample.AvgRssi[3]
#endif
									);
			wext_send_wapp_qry_rsp(pAd->net_dev, &event);
		}
	}

	return 0;
}


INT wapp_send_wsc_scan_complete_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;
	WSC_CTRL *pWscControl = &wdev->WscControl;

	event.len = 0;
	event.event_id = WAPP_WSC_SCAN_COMP_NOTIF;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event.data.wsc_scan_info.bss_count =
		pWscControl->WscPBCBssCount;
	if (pWscControl->WscPBCBssCount == 1) {
		NdisCopyMemory(event.data.wsc_scan_info.Uuid,
			pWscControl->WscPeerUuid, sizeof(pWscControl->WscPeerUuid));
	}
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}


INT wapp_send_wsc_eapol_start_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	event.len = 0;
	event.event_id = WAPP_WSC_EAPOL_START_NOTIF;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

INT wapp_send_wsc_eapol_complete_notif(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	event.len = 0;
	event.event_id = WAPP_WSC_EAPOL_COMPLETE_NOTIF;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}
#ifdef CONFIG_MAP_SUPPORT
INT wapp_send_scan_complete_notification(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev)
{
	struct wapp_event event;

	event.len = 0;
	event.event_id = WAPP_SCAN_COMPLETE_NOTIF;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}
#endif
#ifdef A4_CONN
/* client disaccos */
INT wapp_send_a4_entry_missing(
	PRTMP_ADAPTER pAd,
	UINT32 ifindex,
	UCHAR *ip)
{
	struct wapp_event event;
	UCHAR *a4_missing_entry_ip;

	event.event_id = WAPP_A4_ENTRY_MISSING_NOTIF;
	event.ifindex = ifindex;
	a4_missing_entry_ip = (UCHAR *)&event.data.a4_missing_entry_ip;

	NdisCopyMemory(a4_missing_entry_ip, ip, 4);
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);

	return 0;
}
#endif
/* client disaccos */
INT wapp_send_radar_detect_notif(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	unsigned char channel,
	unsigned char ch_status
	)
{
	struct wapp_event event;

	event.event_id = WAPP_RADAR_DETECT_NOTIF;
	event.ifindex = RtmpOsGetNetIfIndex(wdev->if_dev);
	event.data.radar_notif.channel = channel;
	event.data.radar_notif.status = ch_status;
	wext_send_wapp_qry_rsp(pAd->net_dev, &event);
	return 0;
}

VOID RTMPIoctlGetScanResults(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req)
{
	struct wapp_event *event;
	INT last_bss_cnt = 0;
	RTMP_STRING *msg;
	INT		i = 0;
	INT			WaitCnt = 0;
	UINT32		bss_start_idx;
	BSS_ENTRY *bss;
	struct scan_bss_info *pBss;
	wdev_ht_cap *ht_cap;
	wdev_vht_cap *vht_cap;
#ifndef IWEVCUSTOM_PAYLOD_MAX_LEN
#define IWEVCUSTOM_PAYLOD_MAX_LEN 220
#endif
	INT custom_event_length = IWEVCUSTOM_PAYLOD_MAX_LEN;
	UINT32 TotalLen = custom_event_length;
	INT count = 0, max_bss = 0;


	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPIoctlGetSiteSurvey - enter\n"));
	/*
	 * At the moment there is a single scan tab across the pAd
	 * that means if scan id or if_index has changed, it doesn't
	 * make sense of continuing the last get scan command.
	 */
	if ((pAdapter->last_scan_req.scan_id != req->data.value) ||
	    (pAdapter->last_scan_req.if_index != req->data.ifindex)) {
		pAdapter->last_scan_req.scan_id = req->data.value;
		pAdapter->last_scan_req.if_index = req->data.ifindex;
	} else
		last_bss_cnt = pAdapter->last_scan_req.last_bss_cnt;

	if (pAdapter->ScanTab.BssNr == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s: no bss present in scan tab\n", __func__));
		/*return;*/
	}
	max_bss = custom_event_length / sizeof(struct scan_bss_info);

	bss_start_idx = last_bss_cnt;

	TotalLen = sizeof(CHAR) * 2 + ((sizeof(struct scan_bss_info)) * max_bss) + sizeof(UINT32);
	if ((max_bss > 0) && (TotalLen > custom_event_length))
		max_bss--;

	if (max_bss == 0) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("bss size is too big for wireless send event\n"));
		return;
	}
	TotalLen = sizeof(CHAR) * 2 + ((sizeof(struct scan_bss_info)) * max_bss) + sizeof(UINT32);
	os_alloc_mem(NULL, (PUCHAR *)&msg, TotalLen);

	if (msg == NULL) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("RTMPIoctlGetSiteSurvey - msg memory alloc fail.\n"));
		return;
	}

	NdisZeroMemory(msg, TotalLen);

	event = (struct wapp_event *)msg;
	event->event_id = WAPP_SCAN_RESULT_RSP;
	event->ifindex = req->data.ifindex;

	if (bss_start_idx > (pAdapter->ScanTab.BssNr - 1)) {
		event->data.scan_info.bss_count = 0;
		goto send_event;
	}

	while ((ScanRunning(pAdapter) == TRUE) && (WaitCnt++ < 200))
		OS_WAIT(500);

	for (i = bss_start_idx; i < pAdapter->ScanTab.BssNr && count < max_bss; i++) {
		bss = &pAdapter->ScanTab.BssEntry[i];
		pBss = &event->data.scan_info.bss[count];

		ht_cap = &pBss->ht_cap;
		vht_cap = &pBss->vht_cap;
		if (bss->Channel == 0)
			break;
		pBss->Channel = bss->Channel;
		pBss->CentralChannel = bss->CentralChannel;
#ifdef CONFIG_MAP_SUPPORT
		if (pAdapter->ApEnableBeaconTable == TRUE) {
			ULONG	Idx;

			RemoveOldBssEntry(pAdapter);
			Idx = BssTableSearch(&pAdapter->AvailableBSS, bss->Bssid, bss->Channel);
			if (Idx != BSS_NOT_FOUND && (pAdapter->AvailableBSS.BssEntry[Idx].Channel == bss->Channel))
				pBss->Rssi = pAdapter->AvailableBSS.BssEntry[Idx].Rssi;
			else
				pBss->Rssi = bss->Rssi;
		} else
#endif
		pBss->Rssi = bss->Rssi;
		ht_cap->tx_stream = 0; /* TODO */
		ht_cap->rx_stream = 0; /* TODO */
		ht_cap->sgi_20 = bss->HtCapability.HtCapInfo.ShortGIfor20;
		ht_cap->sgi_40 = bss->HtCapability.HtCapInfo.ShortGIfor40;
		ht_cap->ht_40 = bss->HtCapability.HtCapInfo.ChannelWidth;

		NdisMoveMemory(vht_cap->sup_tx_mcs,
						&bss->vht_cap_ie.mcs_set.tx_mcs_map,
						sizeof(vht_cap->sup_tx_mcs));
		NdisMoveMemory(vht_cap->sup_rx_mcs,
						&bss->vht_cap_ie.mcs_set.rx_mcs_map,
						sizeof(vht_cap->sup_rx_mcs));
		if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.rx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->rx_stream = 2;
		else
			vht_cap->rx_stream = 1;

		if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss4 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 4;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss3 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 3;
		else if (bss->vht_cap_ie.mcs_set.tx_mcs_map.mcs_ss2 != VHT_MCS_CAP_NA)
			vht_cap->tx_stream = 2;
		else
			vht_cap->tx_stream = 1;
		vht_cap->sgi_80 = bss->vht_cap_ie.vht_cap.sgi_80M;
		vht_cap->sgi_160 = bss->vht_cap_ie.vht_cap.sgi_160M;
		vht_cap->vht_160 = bss->vht_op_ie.vht_op_info.ch_width;
		vht_cap->vht_8080 = 0; /* TODO */
		vht_cap->su_bf = bss->vht_cap_ie.vht_cap.bfee_cap_su;
		vht_cap->mu_bf = bss->vht_cap_ie.vht_cap.bfee_cap_mu;
		pBss->MinSNR = bss->MinSNR;
		pBss->Privacy = bss->Privacy;
		pBss->SsidLen = bss->SsidLen;
		NdisCopyMemory(pBss->Ssid, bss->Ssid, pBss->SsidLen);
		COPY_MAC_ADDR(pBss->Bssid, bss->Bssid);
		pBss->AuthMode = bss->AuthMode;
#ifdef CONFIG_MAP_SUPPORT
		NdisCopyMemory(&pBss->map_info, &bss->map_info, sizeof(struct map_vendor_ie));
		pBss->map_vendor_ie_found = bss->map_vendor_ie_found;
#endif
		count++;
		if (pBss->SsidLen)
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("RTMPIoctlGetSiteSurvey - ssid %s\n", pBss->Ssid));
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("RTMPIoctlGetSiteSurvey - max_bss%d\n len=%u count=%d", max_bss, TotalLen, count));
	event->data.scan_info.bss_count = count;
	pAdapter->last_scan_req.last_bss_cnt = i;
	if (i < pAdapter->ScanTab.BssNr)
		event->data.scan_info.more_bss = 1;
send_event:
	RtmpOSWrielessEventSend(pAdapter->net_dev, RT_WLAN_EVENT_CUSTOM,
			OID_WAPP_EVENT, NULL, (PUCHAR)event, TotalLen);

	os_free_mem((PUCHAR)msg);
}

VOID RTMPIoctlSendNullDataFrame(
	IN	PRTMP_ADAPTER	pAdapter, struct wapp_req *req)
{
	INT count, pkt_count;
	MAC_TABLE_ENTRY *pEntry = MacTableLookup(pAdapter, req->data.mac_addr);

	if (!pEntry) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("given station not found\n"));
		return;
	}

	pkt_count = req->data.value;

	if (pEntry->PsMode == PWR_SAVE) {
		/* use TIM bit to detect the PS station */
		WLAN_MR_TIM_BIT_SET(pAdapter, pEntry->func_tb_idx, pEntry->Aid);
		OS_WAIT(200);
	} else {
		/* use Null or QoS Null to detect the ACTIVE station */
		BOOLEAN bQosNull = FALSE;

		if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_WMM_CAPABLE))
			bQosNull = TRUE;

		for (count = 0; count < pkt_count; count++) {
			/* TODO status */
			RtmpEnqueueNullFrame(pAdapter, pEntry->Addr, pEntry->CurrTxRate,
							 pEntry->Aid, pEntry->func_tb_idx, bQosNull, TRUE, 0);
		}
	}
}

void wapp_prepare_nop_channel_list(PRTMP_ADAPTER pAd,
	struct nop_channel_list_s *nop_list)
{
	UINT_8 i;
	PDFS_PARAM pDfsParam = &pAd->CommonCfg.DfsParameter;

	for (i = 0; i < pDfsParam->ChannelListNum; i++) {
		if (pDfsParam->DfsChannelList[i].NonOccupancy > 0 &&
			(nop_list->channel_count < MAX_NUM_OF_CHANNELS)) {
			nop_list->channel_list[nop_list->channel_count] = pDfsParam->DfsChannelList[i].Channel;
			nop_list->channel_count++;
		}
	}
}
INT	wapp_event_handle(
	PRTMP_ADAPTER pAd,
	struct wapp_req *req)
{
	switch (req->req_id) {
	case WAPP_DEV_QUERY_REQ:
		wapp_send_wdev_query_rsp(pAd, req);
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
	case WAPP_APCLI_QUERY_REQ:
		wapp_send_apcli_query_rsp(pAd, req);
		break;
	case WAPP_BSS_START_REQ:
		wapp_bss_start(pAd, req);
		break;
	case WAPP_BSS_STOP_REQ:
		wapp_bss_stop(pAd, req);
		break;
	case WAPP_BSS_LOAD_THRD_SET_REQ:
		wapp_bss_load_thrd_set(pAd, req);
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
	case WAPP_APCLI_RSSI_QUERY_REQ:
		wapp_send_apcli_rssi_query_rsp(pAd, req);
		break;
	case WAPP_GET_SCAN_RESULTS:
		RTMPIoctlGetScanResults(pAd, req);
		break;
	case WAPP_WSC_PBC_EXEC:
		{
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;
			PWSC_CTRL pWscControl = NULL;

			if (ifIndex >= MAX_APCLI_NUM) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_RRM, DBG_LVL_OFF,
					("Unknown If index (%d)", ifIndex));
				return NDIS_STATUS_FAILURE;
			}
			pWscControl = &pAd->ApCfg.ApCliTab[ifIndex].wdev.WscControl;
			WscPBCExec(pAd, FALSE, pWscControl);
			break;
		}
	case WAPP_SEND_NULL_FRAMES:
		RTMPIoctlSendNullDataFrame(pAd, req);
		break;
	case WAPP_WSC_SET_BH_PROFILE:
		{
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;
			PWSC_CTRL pWscControl = NULL;
			int i = 0;

			pWscControl = &pAd->ApCfg.MBSSID[ifIndex].wdev.WscControl;
			for (i = 0; i < pWscControl->WscBhProfiles.ProfileCnt; i++) {
				if (MAC_ADDR_EQUAL(pWscControl->WscBhProfiles.Profile[i].MacAddr,
					req->data.bh_wsc_profile.MacAddr)) {
					NdisCopyMemory(&pWscControl->WscBhProfiles.Profile[i],
						&req->data.bh_wsc_profile, sizeof(WSC_CREDENTIAL));
					break;
				}
			}
			if (i == pWscControl->WscBhProfiles.ProfileCnt) {
				NdisCopyMemory(&pWscControl->WscBhProfiles.Profile[i],
					&req->data.bh_wsc_profile, sizeof(WSC_CREDENTIAL));
				pWscControl->WscBhProfiles.ProfileCnt++;
			}
			break;
		}
#ifdef CONFIG_MAP_SUPPORT
	case WAPP_SET_SCAN_BH_SSIDS:
		{
			struct wifi_dev *wdev;
			POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
			UCHAR ifIndex = pObj->ioctl_if;

			if (ifIndex > MAX_APCLI_NUM) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("break if index is wrong\n"));
				break;
			}
			wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
			NdisZeroMemory(&wdev->MAPCfg.scan_bh_ssids,
				sizeof(wdev->MAPCfg.scan_bh_ssids));
			NdisCopyMemory(&wdev->MAPCfg.scan_bh_ssids,
				&req->data.scan_bh_ssids, sizeof(wdev->MAPCfg.scan_bh_ssids));
			break;
		}
#endif
	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("invalid wapp req id=%d\n", req->req_id));
		break;
	}
	return 0;
}

INT set_wapp_param(
	IN PRTMP_ADAPTER pAd,
	UINT32 Param,
	UINT32 Value)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	UCHAR APIndex = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl;
	PGAS_CTRL pGASCtrl;

#ifdef CONFIG_HOTSPOT_R2
	PHOTSPOT_CTRL pHSCtrl;
	pHSCtrl = &pAd->ApCfg.MBSSID[APIndex].HotSpotCtrl;
#endif /* CONFIG_HOTSPOT_R2 */

	pWNMCtrl = &pAd->ApCfg.MBSSID[APIndex].WNMCtrl;
	pGASCtrl = &pAd->ApCfg.MBSSID[APIndex].GASCtrl;

	switch (Param) {
	case PARAM_WNM_BSS_TRANSITION_MANAGEMENT:
		pWNMCtrl->WNMBTMEnable = Value;
		break;

	case PARAM_EXTERNAL_ANQP_SERVER_TEST:
		pGASCtrl->ExternalANQPServerTest = Value;
		break;

	case PARAM_GAS_COME_BACK_DELAY:
		pGASCtrl->cb_delay = Value;
		break;

	case PARAM_MMPDU_SIZE:
		pGASCtrl->MMPDUSize = Value;
		break;

	case PARAM_WNM_NOTIFICATION:
		pWNMCtrl->WNMNotifyEnable = Value;
		break;
#ifdef CONFIG_HOTSPOT_R2

	case PARAM_QOSMAP:
		pHSCtrl->QosMapEnable = Value;
		hotspot_update_bssflag(pAd, fgQosMapEnable, Value, pHSCtrl);
		break;
	case PARAM_DGAF_DISABLED:
		pHSCtrl->DGAFDisable = Value;
		hotspot_update_bssflag(pAd, fgDGAFDisable, Value, pHSCtrl);
		break;

	case PARAM_PROXY_ARP:
		pWNMCtrl->ProxyARPEnable = Value;
		hotspot_update_bssflag(pAd, fgProxyArpEnable, Value, pHSCtrl);
		break;

	case PARAM_L2_FILTER:
		pHSCtrl->L2Filter = Value;
		break;

	case PARAM_ICMPV4_DENY:
		pHSCtrl->ICMPv4Deny = Value;
		break;

#endif /* CONFIG_HOTSPOT_R2 */
	default:
		MTWF_LOG(DBG_CAT_PROTO, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknow Parameter:%d\n", Param));
		break;
	}
#ifdef CONFIG_HOTSPOT_R2
	/* for 7615 offload to CR4 */
	hotspot_update_bss_info_to_cr4(pAd, APIndex);
#endif /* CONFIG_HOTSPOT_R2 */

	return 0;
}

static INT set_wapp_cmm_ie(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 EID,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PWNM_CTRL pWNMCtrl =  &pAd->ApCfg.MBSSID[apidx].WNMCtrl;
	PGAS_CTRL pGasCtrl = &pAd->ApCfg.MBSSID[apidx].GASCtrl;
#ifdef CONFIG_HOTSPOT_R2
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
#endif /* CONFIG_HOTSPOT_R2 */

	switch (EID) {
	case IE_INTERWORKING:
		if (pGasCtrl->InterWorkingIE != NULL) {
			os_free_mem(pGasCtrl->InterWorkingIE);
			pGasCtrl->InterWorkingIE = NULL;
			pGasCtrl->InterWorkingIELen = 0;
		}
		os_alloc_mem(NULL, &pGasCtrl->InterWorkingIE, IELen);
		NdisMoveMemory(pGasCtrl->InterWorkingIE, IE, IELen);
		pGasCtrl->InterWorkingIELen = IELen;
#ifdef CONFIG_HOTSPOT_R2
		pHSCtrl->AccessNetWorkType  = (*(IE + 2)) & 0x0F;
		if (IELen > 3) {
			pHSCtrl->IsHessid = TRUE;

			if (IELen == 7)
				NdisMoveMemory(pHSCtrl->Hessid, IE + 3, MAC_ADDR_LEN);
			else
				NdisMoveMemory(pHSCtrl->Hessid, IE + 5, MAC_ADDR_LEN);
		}
#endif /* CONFIG_HOTSPOT_R2 */
		break;
	case IE_ADVERTISEMENT_PROTO:
		if (pGasCtrl->AdvertisementProtoIE != NULL) {
			os_free_mem(pGasCtrl->AdvertisementProtoIE);
			pGasCtrl->AdvertisementProtoIE = NULL;
			pGasCtrl->AdvertisementProtoIELen = 0;
		}
		os_alloc_mem(NULL, &pGasCtrl->AdvertisementProtoIE, IELen);
		NdisMoveMemory(pGasCtrl->AdvertisementProtoIE, IE, IELen);
		pGasCtrl->AdvertisementProtoIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Advertisement Protocol IE\n"));
			break;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Interworking IE\n"));
		break;

	case IE_TIME_ADVERTISEMENT:
		if (pWNMCtrl->TimeadvertisementIE != NULL) {
			os_free_mem(pWNMCtrl->TimeadvertisementIE);
			pWNMCtrl->TimeadvertisementIE = NULL;
			pWNMCtrl->TimeadvertisementIELen = 0;
		}
		os_alloc_mem(NULL, &pWNMCtrl->TimeadvertisementIE, IELen);
		NdisMoveMemory(pWNMCtrl->TimeadvertisementIE, IE, IELen);
		pWNMCtrl->TimeadvertisementIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Time Advertisement IE\n"));
		break;

	case IE_TIME_ZONE:
		if (pWNMCtrl->TimezoneIE != NULL) {
			os_free_mem(pWNMCtrl->TimezoneIE);
			pWNMCtrl->TimezoneIE = NULL;
			pWNMCtrl->TimezoneIELen = 0;
		}
		os_alloc_mem(NULL, &pWNMCtrl->TimezoneIE, IELen);
		NdisMoveMemory(pWNMCtrl->TimezoneIE, IE, IELen);
		pWNMCtrl->TimezoneIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Time Zone IE\n"));
		break;
#ifdef CONFIG_HOTSPOT_R2
	case IE_QOS_MAP_SET: {
		int tmp = 0;
		char *pos = (char *)(IE + 2);
		if (pHSCtrl->QosMapSetIE != NULL) {
			os_free_mem(pHSCtrl->QosMapSetIE);
			pHSCtrl->QosMapSetIE = NULL;
			pHSCtrl->QosMapSetIELen = 0;
		}
		os_alloc_mem(NULL, &pHSCtrl->QosMapSetIE, IELen);
		NdisMoveMemory(pHSCtrl->QosMapSetIE, IE, IELen);
		pHSCtrl->QosMapSetIELen = IELen;

		for (tmp = 0; tmp < (IELen - 16 - 2) / 2; tmp++) {
			pHSCtrl->DscpException[tmp] = *pos & 0xff;
			pHSCtrl->DscpException[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DscpException[%d]:0x%x\n", tmp, pHSCtrl->DscpException[tmp]));
		}

		for (tmp = 0; tmp < 8; tmp++) {
			pHSCtrl->DscpRange[tmp] = *pos & 0xff;
			pHSCtrl->DscpRange[tmp] |= (*(pos + 1) & 0xff) << 8;
			pos += 2;
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("DscpRange[%d]:0x%x\n", tmp, pHSCtrl->DscpRange[tmp]));
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("=========================================== Set Qos MAP Set IE\n"));
		break;
	}

	case IE_ROAMING_CONSORTIUM:
		if (pHSCtrl->RoamingConsortiumIE != NULL) {
			os_free_mem(pHSCtrl->RoamingConsortiumIE);
			pHSCtrl->RoamingConsortiumIE = NULL;
			pHSCtrl->RoamingConsortiumIELen = 0;
		}
		os_alloc_mem(NULL, &pHSCtrl->RoamingConsortiumIE, IELen);
		NdisMoveMemory(pHSCtrl->RoamingConsortiumIE, IE, IELen);
		pHSCtrl->RoamingConsortiumIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set Roaming Consortium IE\n"));
		break;
#endif /* CONFIG_HOTSPOT_R2 */

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown IE(EID = %d)\n", __func__, EID));
		break;
	}

	return TRUE;
}

static void Set_MTK_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING * IE,
	IN UINT32 IELen,
	IN UCHAR apidx)
{
#ifdef CONFIG_MAP_SUPPORT
	struct wifi_dev *wdev = &pAd->ApCfg.MBSSID[apidx].wdev;

	if (!IS_MAP_TURNKEY_ENABLE(pAd)) {
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("MAP turnkey is not enabled, skipping vendor ie settings\n"));
		return;
	}
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set MTK IE\n"));
	os_zero_mem(wdev->MAPCfg.vendor_ie_buf, VENDOR_SPECIFIC_LEN);
	wdev->MAPCfg.vendor_ie_len = IELen;
	NdisMoveMemory(wdev->MAPCfg.vendor_ie_buf, IE, wdev->MAPCfg.vendor_ie_len);
	if (wdev->bAllowBeaconing)
		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
#endif
}


static INT Set_AP_VENDOR_SPECIFIC_IE(
	IN PRTMP_ADAPTER pAd,
	IN UINT8 OUIType,
	IN RTMP_STRING *IE,
	IN UINT32 IELen)
{
#ifdef CONFIG_HOTSPOT
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR apidx = pObj->ioctl_if;
	PHOTSPOT_CTRL pHSCtrl =  &pAd->ApCfg.MBSSID[apidx].HotSpotCtrl;
#endif

	switch (OUIType) {
#ifdef CONFIG_HOTSPOT

	case OUI_P2P:
		os_alloc_mem(NULL, &pHSCtrl->P2PIE, IELen);
		NdisMoveMemory(pHSCtrl->P2PIE, IE, IELen);
		pHSCtrl->P2PIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set P2P IE\n"));
		break;

	case OUI_HS2_INDICATION:
		os_alloc_mem(NULL, &pHSCtrl->HSIndicationIE, IELen);
		NdisMoveMemory(pHSCtrl->HSIndicationIE, IE, IELen);
		pHSCtrl->HSIndicationIELen = IELen;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set HS2.0 Indication IE\n"));
		break;
#endif

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown OUIType = %d\n", __func__, OUIType));
		break;
	}

	return TRUE;
}

INT wapp_set_ap_ie(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING *IE,
	IN UINT32 IELen,
	IN UCHAR ApIdx)
{
	UINT8 EID;
	UINT8 OUIType;

	EID = *IE;

	switch (EID) {
	case IE_INTERWORKING:
	case IE_ADVERTISEMENT_PROTO:
	case IE_TIME_ADVERTISEMENT:
	case IE_TIME_ZONE:
	case IE_QOS_MAP_SET:
	case IE_ROAMING_CONSORTIUM:
		set_wapp_cmm_ie(pAd, EID, IE, IELen);
		break;

	case IE_VENDOR_SPECIFIC:
		OUIType = *(IE + 5);
		if (NdisEqualMemory(&IE[2], &MTK_OUI[0], 3))
			Set_MTK_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen, ApIdx);
		else
			Set_AP_VENDOR_SPECIFIC_IE(pAd, OUIType, IE, IELen);
		break;

	default:
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown IE(EID = %d)\n", __func__, EID));
		break;
	}

	return TRUE;
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
