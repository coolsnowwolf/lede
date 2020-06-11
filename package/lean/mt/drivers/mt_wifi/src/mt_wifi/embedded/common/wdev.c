/***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/
#include "rt_config.h"

/*define extern function*/
INT wdev_edca_acquire(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	EDCA_PARM *edca;

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
#ifdef WDS_SUPPORT
	case WDEV_TYPE_WDS:
#endif /*WDS_SUPPORT*/
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;

#ifdef APCLI_SUPPORT

	case WDEV_TYPE_APCLI:
		edca = &ad->ApCfg.ApCliTab[wdev->func_idx].MlmeAux.APEdcaParm;
		break;
#endif /* APCLI_SUPPORT */
#endif /*CONFIG_AP_SUPPORT*/

	default:
		edca = &ad->CommonCfg.APEdcaParm[wdev->EdcaIdx];
		break;
	}

	HcAcquiredEdca(ad, wdev, edca);
	return TRUE;
}

/*define global function*/
struct wifi_dev *get_default_wdev(struct _RTMP_ADAPTER *ad)
{
#ifdef CONFIG_AP_SUPPORT
	RT_CONFIG_IF_OPMODE_ON_AP(ad->OpMode) {
		return &ad->ApCfg.MBSSID[MAIN_MBSSID].wdev;
	}
#endif
	return NULL;
}

/*define local function*/
static INT wdev_idx_unreg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("unregister wdev(type:%d, idx:%d) from wdev_list\n",
					  wdev->wdev_type, wdev->wdev_idx));
			pAd->wdev_list[idx] = NULL;
			wdev->wdev_idx = WDEV_NUM_MAX;
			break;
		}
	}

	if (idx == WDEV_NUM_MAX) {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Cannot found wdev(%p, type:%d, idx:%d) in wdev_list\n",
				  wdev, wdev->wdev_type, wdev->wdev_idx));
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dump wdev_list:\n"));

		for (idx = 0; idx < WDEV_NUM_MAX; idx++)
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Idx %d: 0x%p\n", idx, pAd->wdev_list[idx]));
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? 0 : -1);
}


static INT32 wdev_idx_reg(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 idx;

	if (!wdev)
		return -1;

	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (idx = 0; idx < WDEV_NUM_MAX; idx++) {
		if (pAd->wdev_list[idx] == wdev) {
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					 ("wdev(type:%d) already registered and idx(%d) %smatch\n",
					  wdev->wdev_type, wdev->wdev_idx,
					  ((idx != wdev->wdev_idx) ? "mis" : "")));
			break;
		}

		if (pAd->wdev_list[idx] == NULL) {
			pAd->wdev_list[idx] = wdev;
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s::Assign wdev_idx=%d with OmacIdx = %d\n",
					 __func__,
					 idx,
					 wdev->OmacIdx));
			break;
		}
	}

	wdev->wdev_idx = idx;

	if (idx < WDEV_NUM_MAX)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Assign wdev_idx=%d\n", idx));

	NdisReleaseSpinLock(&pAd->WdevListLock);
	return ((idx < WDEV_NUM_MAX) ? idx : -1);
}


static INT32 GetBssIdx(RTMP_ADAPTER *pAd)
{
	UINT32 BssInfoIdxBitMap;
	UCHAR i;
	INT32 no_usable_entry = -1;
#ifdef MAC_REPEATER_SUPPORT
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
#endif
	UCHAR BssInfoMax = BSSINFO_NUM_MAX(cap);
	NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);
	BssInfoIdxBitMap = pAd->BssInfoIdxBitMap0;

	for (i = 0; i < 32; i++) {
		/* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
		if ((BssInfoIdxBitMap & (1 << i)) == 0) {
			pAd->BssInfoIdxBitMap0 = (BssInfoIdxBitMap | (1 << i));
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: found non-used BssInfoIdx: %d\n", __func__, i));
			NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
			return i;
		}
	}

	BssInfoIdxBitMap = pAd->BssInfoIdxBitMap1;

	for (i = 32; i < BssInfoMax; i++) {
		/* find the first 0 bitfield, then return the bit idx as BssInfoIdx. */
		if ((BssInfoIdxBitMap & (1 << (i - 32))) == 0) {
			pAd->BssInfoIdxBitMap1 = (BssInfoIdxBitMap | (1 << (i - 32)));
			MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					 ("%s: found non-used BssInfoIdx: %d\n", __func__, i));
			NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
			return i;
		}
	}

	NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);

	if (i >= BssInfoMax) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: could not find usable BssInfoIdx\n", __func__));
		return no_usable_entry;
	}

	return no_usable_entry;
}

VOID ReleaseBssIdx(RTMP_ADAPTER *pAd, UINT32 BssIdx)
{
	NdisAcquireSpinLock(&pAd->BssInfoIdxBitMapLock);

	if (BssIdx < 32)
		pAd->BssInfoIdxBitMap0 = pAd->BssInfoIdxBitMap0 & (0xffffffff & ~(1 << BssIdx));
	else
		pAd->BssInfoIdxBitMap1 = pAd->BssInfoIdxBitMap1 & (0xffffffff & ~(1 << (BssIdx - 32)));

	NdisReleaseSpinLock(&pAd->BssInfoIdxBitMapLock);
}

/*
*
*/
VOID BssInfoArgumentLink(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev, struct _BSS_INFO_ARGUMENT_T *bssinfo)
{
	HTTRANSMIT_SETTING HTPhyMode;

	bssinfo->OwnMacIdx = wdev->OmacIdx;
	bssinfo->ucBssIndex = GetBssIdx(ad);
	os_move_mem(bssinfo->Bssid, wdev->bssid, MAC_ADDR_LEN);
	bssinfo->CipherSuit = SecHWCipherSuitMapping(wdev->SecConfig.PairwiseCipher);
	bssinfo->ucPhyMode = wdev->PhyMode;
	hc_radio_query_by_wdev(wdev, &bssinfo->chan_oper);

	switch (wdev->wdev_type) {
	case WDEV_TYPE_STA:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_STA;
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->ucBcMcWlanIdx, wdev);
#ifdef CONFIG_KEEP_ALIVE_OFFLOAD
		/* STA LP offload */
		bssinfo->rBssInfoPm.ucKeepAliveEn = TRUE;
		bssinfo->rBssInfoPm.ucKeepAlivePeriod = KEEP_ALIVE_INTERVAL_IN_SEC;
#endif
		bssinfo->rBssInfoPm.ucBeaconLossReportEn = TRUE;
		bssinfo->rBssInfoPm.ucBeaconLossCount = BEACON_OFFLOAD_LOST_TIME;
		break;
	case WDEV_TYPE_APCLI:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_STA;
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->ucBcMcWlanIdx, wdev);
		break;

	case WDEV_TYPE_ADHOC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_IBSS;
		bssinfo->u4ConnectionType = CONNECTION_IBSS_ADHOC;
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		break;

	case WDEV_TYPE_WDS:
		bssinfo->bssinfo_type = WDS;
		bssinfo->NetworkType = NETWORK_WDS;
		bssinfo->u4ConnectionType = CONNECTION_WDS;
		break;

	case WDEV_TYPE_GO:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GO;
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->ucBcMcWlanIdx, wdev);
		MgmtTableSetMcastEntry(ad, bssinfo->ucBcMcWlanIdx);
		break;

	case WDEV_TYPE_GC:
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->NetworkType = NETWORK_P2P;
		bssinfo->u4ConnectionType = CONNECTION_P2P_GC;
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		break;

	case WDEV_TYPE_AP:
	default:
		/* Get a specific WCID to record this MBSS key attribute */
		bssinfo->bssinfo_type = HW_BSSID;
		bssinfo->ucBcMcWlanIdx = HcAcquireGroupKeyWcid(ad, wdev);
		TRTableInsertMcastEntry(ad, bssinfo->ucBcMcWlanIdx, wdev);
		MgmtTableSetMcastEntry(ad, bssinfo->ucBcMcWlanIdx);
		bssinfo->NetworkType = NETWORK_INFRA;
		bssinfo->u4ConnectionType = CONNECTION_INFRA_AP;
#ifdef CONFIG_AP_SUPPORT
		bssinfo->bcn_period = ad->CommonCfg.BeaconPeriod;
		bssinfo->dtim_period = ad->ApCfg.DtimPeriod;
#endif /*CONFIG_AP_SUPPORT*/
		break;
	}

	wdev_edca_acquire(ad, wdev);
	bssinfo->WmmIdx = HcGetWmmIdx(ad, wdev);
	/* Get a specific Tx rate for BMcast frame */
	os_zero_mem(&HTPhyMode, sizeof(HTTRANSMIT_SETTING));

	if (WMODE_CAP(wdev->PhyMode, WMODE_B) &&
		(wdev->channel <= 14)) {
		HTPhyMode.field.MODE = MODE_CCK;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = RATE_1;
	} else {
		HTPhyMode.field.MODE = MODE_OFDM;
		HTPhyMode.field.BW = BW_20;
		HTPhyMode.field.MCS = MCS_RATE_6;
	}

#ifdef MCAST_RATE_SPECIFIC

	if (wdev->channel > 14)
		bssinfo->McTransmit = ad->CommonCfg.MCastPhyMode_5G;
	else
		bssinfo->McTransmit = ad->CommonCfg.MCastPhyMode;

#else
	bssinfo->McTransmit = HTPhyMode;
#endif /* MCAST_RATE_SPECIFIC */
	bssinfo->BcTransmit = HTPhyMode;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	raWrapperConfigSet(ad, wdev, &bssinfo->ra_cfg);
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/
	bssinfo->bss_state = BSS_INITED;
}


VOID BssInfoArgumentUnLink(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct _BSS_INFO_ARGUMENT_T *bss = &wdev->bss_info_argument;

	ReleaseBssIdx(pAd, bss->ucBssIndex);
	HcReleaseGroupKeyWcid(pAd, wdev, bss->ucBcMcWlanIdx);
	HcReleaseEdca(pAd, wdev);
	os_zero_mem(bss, sizeof(wdev->bss_info_argument));
	WDEV_BSS_STATE(wdev) = BSS_INIT;
}

INT wdev_ops_register(struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
					  struct wifi_dev_ops *ops, UCHAR wmm_detect_method)
{
	wdev->wdev_ops = ops;

	if (wmm_detect_method == WMM_DETECT_METHOD1)
		ops->detect_wmm_traffic = mt_detect_wmm_traffic;
	else if (wmm_detect_method == WMM_DETECT_METHOD2)
		ops->detect_wmm_traffic = detect_wmm_traffic;

	/*wifi system architecture register*/
	wifi_sys_ops_register(wdev);
	return TRUE;
}

/**
 * @pAd
 * @wdev wifi device
 * @wdev_type wifi device type
 * @IfDev pointer to interface NET_DEV
 * @func_idx  _STA_TR_ENTRY index for BC/MC packet
 * @func_dev function device
 * @sys_handle pointer to pAd
 *
 * Initialize a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_init(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum WDEV_TYPE wdev_type,
				PNET_DEV IfDev, INT8 func_idx, VOID *func_dev, VOID *sys_handle)
{
	INT32 wdev_idx = 0;
	wdev->wdev_type = wdev_type;
	wdev->if_dev = IfDev;
	wdev->func_idx = func_idx;
	wdev->func_dev = func_dev;
	wdev->sys_handle = sys_handle;
	wdev->tr_tb_idx = 0xff;
	wdev->OpStatusFlags = 0;
	wdev->forbid_data_tx = 0x1 << MSDU_FORBID_CONNECTION_NOT_READY;
	wdev->bAllowBeaconing = FALSE;
	wdev_idx = wdev_idx_reg(pAd, wdev);

	init_vie_ctrl(wdev);

	if (wdev_idx < 0)
		return FALSE;

	hc_obj_init(wdev, wdev_idx);

	return TRUE;
}

INT32 wdev_attr_update(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP:
	case WDEV_TYPE_GO:
		AsicSetMbssWdevIfAddr(pAd, wdev->func_idx, (UCHAR *)wdev->if_addr, OPMODE_AP);
		MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): wdevId%d = %02x:%02x:%02x:%02x:%02x:%02x\n",
				 __func__, wdev->wdev_idx, PRINT_MAC(wdev->if_addr)));

		if (wdev->if_dev) {
			NdisMoveMemory(RTMP_OS_NETDEV_GET_PHYADDR(wdev->if_dev),
						   wdev->if_addr, MAC_ADDR_LEN);
		}

		COPY_MAC_ADDR(wdev->bssid, wdev->if_addr);
		break;
#endif /* CONFIG_AP_SUPPORT */

	default:
		break;
	}

	HcAcquireRadioForWdev(pAd, wdev);
	return TRUE;
}


/**
 * @param pAd
 * @param wdev wifi device
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_deinit(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	deinit_vie_ctrl(wdev);

	wlan_operate_exit(wdev);
	hc_obj_exit(wdev);
	wdev_idx_unreg(pAd, wdev);
	return TRUE;
}


/**
 * @param pAd
 *
 * DeInit a wifi_dev embedded in a funtion device according to wdev_type
 *
 * @return TURE/FALSE
 */
INT32 wdev_config_init(RTMP_ADAPTER *pAd)
{
	UCHAR i;
	struct wifi_dev *wdev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = pAd->wdev_list[i];

		if (wdev) {
			wdev->channel = 0;
			wdev->PhyMode = 0;
		}
	}

	return TRUE;
}


/**
 * @param pAd
 * @param Address input address
 *
 * Search wifi_dev according to Address
 *
 * @return wifi_dev
 */
struct wifi_dev *wdev_search_by_address(RTMP_ADAPTER *pAd, UCHAR *address)
{
	UINT16 Index;
	struct wifi_dev *wdev;
#ifdef MAC_REPEATER_SUPPORT
	REPEATER_CLIENT_ENTRY *rept_entry = NULL;
#endif
	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (MAC_ADDR_EQUAL(address, wdev->if_addr)) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
#ifdef MAC_REPEATER_SUPPORT

	/* if we cannot found wdev from A2, it might comes from Rept entry.
	 * cause rept must bind the bssid of apcli_link,
	 * search A3(Bssid) to find the corresponding wdev.
	 */
	if (pAd->ApCfg.bMACRepeaterEn) {
		rept_entry = lookup_rept_entry(pAd, address);

		if (rept_entry)
			return rept_entry->wdev;
	}

#endif
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

struct wifi_dev *wdev_search_by_omac_idx(RTMP_ADAPTER *pAd, UINT8 BssIndex)
{
	UINT16 Index;
	struct wifi_dev *wdev;
	NdisAcquireSpinLock(&pAd->WdevListLock);

	for (Index = 0; Index < WDEV_NUM_MAX; Index++) {
		wdev = pAd->wdev_list[Index];

		if (wdev) {
			if (wdev->OmacIdx == BssIndex) {
				NdisReleaseSpinLock(&pAd->WdevListLock);
				return wdev;
			}
		}
	}

	NdisReleaseSpinLock(&pAd->WdevListLock);
	MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: can not find registered wdev\n",
			 __func__));
	return NULL;
}

inline struct wifi_dev *wdev_search_by_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt)
{
	struct wifi_dev *wdev = NULL;
	UINT32 wdev_idx = RTMP_GET_PACKET_WDEV(pkt);
	wdev = pAd->wdev_list[wdev_idx];

	if (!wdev)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("error: wdev(wdev_idx = %d) is null from pkt\n", wdev_idx));

	return wdev;
}

inline struct wifi_dev *wdev_search_by_idx(RTMP_ADAPTER *pAd, UINT32 idx)
{
	struct wifi_dev *wdev = NULL;
	wdev = pAd->wdev_list[idx];

	if (!wdev)
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("error: wdev(wdev_idx = %d) is null from idx\n", idx));

	return wdev;
}

inline struct wifi_dev *wdev_search_by_wcid(RTMP_ADAPTER *pAd, UINT8 wcid)
{
	struct _STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[wcid];
	struct wifi_dev *wdev = NULL;

	if (tr_entry)
		wdev = tr_entry->wdev;
	else {
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				 ("%s: can not a valid wdev by wcid (%u)\n",
				  __func__, wcid));
	}

	return wdev;
}

struct wifi_dev *wdev_search_by_netdev(RTMP_ADAPTER *pAd, VOID *pDev)
{
	UCHAR i = 0;
	struct net_device *pNetDev = (struct net_device *)pDev;
	struct wifi_dev *wdev = NULL;

	if (pNetDev != NULL) {
		for (i = 0; i < WDEV_NUM_MAX; i++) {
			wdev = pAd->wdev_list[i];

			if (wdev == NULL)
				continue;

			if (wdev->if_dev == pNetDev)
				return wdev;
		}
	}

	return wdev;
}

UCHAR decide_phy_bw_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel)
{
	int i;
	struct wifi_dev *wdev;
	UCHAR phy_bw = BW_20;
	UCHAR wdev_bw;
	UCHAR rfic;

	if (channel <= 14)
		rfic = RFIC_24GHZ;
	else
		rfic = RFIC_5GHZ;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		wdev = ad->wdev_list[i];

		/*only when wdev is up & operting init done can join to decision*/
		if (wdev && (wlan_operate_get_state(wdev) != WLAN_OPER_STATE_INVALID) && (rfic & wmode_2_rfic(wdev->PhyMode))) {
			wdev_bw = wlan_operate_get_bw(wdev);

			if (wdev_bw > phy_bw)
				phy_bw = wdev_bw;
		}
	}

	if (rfic == RFIC_24GHZ && phy_bw > BW_40)
		phy_bw = BW_40;

	return phy_bw;
}


void update_att_from_wdev(struct wifi_dev *dev1, struct wifi_dev *dev2)
{
	UCHAR ht_bw = wlan_operate_get_ht_bw(dev2);
#ifdef DOT11_VHT_AC
	UCHAR vht_bw = wlan_operate_get_vht_bw(dev2);
#endif /*DOT11_VHT_AC*/
	UCHAR ext_cha;
	UCHAR stbc;
	UCHAR ldpc;
	UCHAR tx_stream;
	UCHAR rx_stream;
#ifdef TXBF_SUPPORT
	UCHAR txbf;
#endif

	/*update configure*/
	if (wlan_config_get_ext_cha(dev1) == EXTCHA_NOASSIGN) {
		ext_cha = wlan_config_get_ext_cha(dev2);
		wlan_config_set_ext_cha(dev1, ext_cha);
	}

#ifdef TXBF_SUPPORT
	txbf = wlan_config_get_etxbf(dev2);
	wlan_config_set_etxbf(dev1, txbf);
	txbf = wlan_config_get_itxbf(dev2);
	wlan_config_set_itxbf(dev1, txbf);
#endif
	stbc = wlan_config_get_ht_stbc(dev2);
	wlan_config_set_ht_stbc(dev1, stbc);
	ldpc = wlan_config_get_ht_ldpc(dev2);
	wlan_config_set_ht_ldpc(dev1, ldpc);
	stbc = wlan_config_get_vht_stbc(dev2);
	wlan_config_set_vht_stbc(dev1, stbc);
	ldpc = wlan_config_get_vht_ldpc(dev2);
	wlan_config_set_vht_ldpc(dev1, ldpc);

	ht_bw = wlan_config_get_ht_bw(dev2);
	vht_bw = wlan_config_get_vht_bw(dev2);
	wlan_config_set_ht_bw(dev1, ht_bw);
	wlan_config_set_vht_bw(dev1, vht_bw);
	wlan_config_set_cen_ch_2(dev1, wlan_config_get_cen_ch_2(dev2));

	tx_stream = wlan_config_get_tx_stream(dev2);
	wlan_config_set_tx_stream(dev1, tx_stream);
	rx_stream = wlan_config_get_rx_stream(dev2);
	wlan_config_set_rx_stream(dev1, rx_stream);
	dev1->channel = dev2->channel;
	dev1->bWmmCapable = dev2->bWmmCapable;
	wlan_operate_update_ht_cap(dev1);
}


void wdev_sync_prim_ch(struct _RTMP_ADAPTER *ad, struct wifi_dev *wdev)
{
	UCHAR i = 0;
	struct wifi_dev *tdev;
	UCHAR band_idx = HcGetBandByWdev(wdev);
	for (i = 0; i < WDEV_NUM_MAX; i++) {

		tdev = ad->wdev_list[i];
		if (tdev && HcIsRadioAcq(tdev) && (band_idx == HcGetBandByWdev(tdev)))
			tdev->channel = wdev->channel;
		else if ((wdev->wdev_type == WDEV_TYPE_APCLI) &&
				(tdev != NULL) &&
				(tdev->wdev_type == WDEV_TYPE_AP) &&
				(tdev->if_up_down_state == 0) &&
				(((tdev->channel > 14) && (wdev->channel > 14)) ||
				 ((tdev->channel <= 14) && (wdev->channel <= 14))))
			tdev->channel = wdev->channel;
	}
}


VOID wdev_if_up_down(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN if_up_down_state)
{
	if (wdev == NULL)
		return;

	wdev->if_up_down_state = if_up_down_state;
}

void wdev_sync_ch_by_rfic(struct _RTMP_ADAPTER *ad, UCHAR rfic, UCHAR ch)
{
	UCHAR i;
	struct wifi_dev *dev;

	for (i = 0; i < WDEV_NUM_MAX; i++) {
		dev = ad->wdev_list[i];

		if (dev &&
			(wmode_2_rfic(dev->PhyMode) & rfic) &&
			(dev->channel != ch)
		   )
			dev->channel = ch;
	}
}

/*
* wifi sys operate action, must used in dispatch level task, do not use in irq/tasklet/timer context
*/

INT wdev_do_open(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->open)
		return wdev->wdev_ops->open(wdev);

	return FALSE;
}

INT wdev_do_close(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->close)
		return wdev->wdev_ops->close(wdev);

	return FALSE;
}

INT wdev_do_linkup(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->linkup)
		return wdev->wdev_ops->linkup(wdev, entry);

	return FALSE;
}

INT wdev_do_linkdown(struct wifi_dev *wdev)
{
	if (wdev->wdev_ops && wdev->wdev_ops->linkdown)
		return wdev->wdev_ops->linkdown(wdev);

	return FALSE;
}

INT wdev_do_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->conn_act)
		return wdev->wdev_ops->conn_act(wdev, entry);

	return FALSE;
}

INT wdev_do_disconn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry)
{
	if (wdev->wdev_ops && wdev->wdev_ops->disconn_act)
		return wdev->wdev_ops->disconn_act(wdev, entry);

	return FALSE;
}

