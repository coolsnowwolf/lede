#ifdef WIFI_SYS_FW_V1

#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"

static NTSTATUS hw_ctrl_flow_v1_open(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _DEV_INFO_CTRL_T *devinfo =  &wsys->DevInfoCtrl;

	if (devinfo->EnableFeature) {
		AsicDevInfoUpdate(
			ad,
			devinfo->OwnMacIdx,
			devinfo->OwnMacAddr,
			devinfo->BandIdx,
			devinfo->Active,
			devinfo->EnableFeature
		);
		/*update devinfo to wdev*/
		wifi_sys_update_devinfo(ad, wdev, devinfo);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_close(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _DEV_INFO_CTRL_T *devinfo =  &wsys->DevInfoCtrl;

	if (devinfo->EnableFeature) {
		AsicDevInfoUpdate(
			ad,
			devinfo->OwnMacIdx,
			devinfo->OwnMacAddr,
			devinfo->BandIdx,
			devinfo->Active,
			devinfo->EnableFeature
		);
		/*update devinfo to wdev*/
		wifi_sys_update_devinfo(ad, wdev, devinfo);
	}
	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_link_up(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UINT16 txop_level = TXOP_0;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *)sta_rec->priv;
	struct _BSS_INFO_ARGUMENT_T *bss = &wsys->BssInfoCtrl;

	if (bss->u4BssInfoFeature) {
		AsicBssInfoUpdate(ad, wsys->BssInfoCtrl);
		HcSetEdca(wdev);
		/*update bssinfo to wdev*/
		wifi_sys_update_bssinfo(ad, wdev, bss);
	}

	if (sta_rec->EnableFeature & ~STA_REC_INSTALL_KEY_FEATURE) {
		AsicUpdateRxWCIDTable(ad, sta_rec->WlanIdx, tr_entry->Addr, TRUE, FALSE); /* Haipin: check IsReset */
		AsicStaRecUpdate(ad, sta_rec);
	}

	if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE)
		AsicAddRemoveKeyTab(ad, &sta_rec->asic_sec_info);

	/*update bmc starec to tr_entry*/
	if (sta_rec->EnableFeature)
		wifi_sys_update_starec(ad, sta_rec);

	if (ad->CommonCfg.bEnableTxBurst) {
		txop_level = TXOP_80;

		if (ad->CommonCfg.bRdg)
			txop_level = TXOP_80;
	} else
		txop_level = TXOP_0;

	hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);
#ifdef CONFIG_AP_SUPPORT

	if (WDEV_WITH_BCN_ABILITY(wdev)) {
		UpdateBeaconHandler(
			ad,
			wdev,
			BCN_UPDATE_INIT);
	}

#endif /*CONFIG_AP_SUPPORT*/
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_link_down(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _BSS_INFO_ARGUMENT_T *bss = &wsys->BssInfoCtrl;

	if (sta_rec->EnableFeature & ~STA_REC_INSTALL_KEY_FEATURE) {
		AsicStaRecUpdate(ad, sta_rec);
		AsicDelWcidTab(ad, sta_rec->WlanIdx);
		wifi_sys_update_starec(ad, sta_rec);
	}

	if (!wsys->skip_set_txop)
		hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);

	if (bss->u4BssInfoFeature) {
		AsicBssInfoUpdate(ad, wsys->BssInfoCtrl);
		wifi_sys_update_bssinfo(ad, wdev, bss);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_disconnt_act(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;

	if (sta_rec->EnableFeature & ~STA_REC_INSTALL_KEY_FEATURE)
		AsicStaRecUpdate(ad, sta_rec);

	/* Delete this entry from ASIC on-chip WCID Table*/
	if (!((sta_rec->WlanIdx >= GET_MAX_UCAST_NUM(ad)) && (sta_rec->WlanIdx != WCID_ALL)))
		AsicDelWcidTab(ad, sta_rec->WlanIdx);

	if (!wsys->skip_set_txop)
		hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);
	/*update starec to tr_entry*/
	wifi_sys_update_starec(ad, sta_rec);

	switch (wdev->wdev_type) {
#ifdef CONFIG_AP_SUPPORT

	case WDEV_TYPE_AP: {
		ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
		/* back to default protection */
		wdev->protection = 0;
		addht->AddHtInfo2.OperaionMode = 0;
		UpdateBeaconHandler(ad, wdev, BCN_UPDATE_IE_CHG);
		AsicUpdateProtect(ad);
	}
	break;
#endif /*CONFIG_AP_SUPPORT*/
	}

	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_connt_act(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	PEER_LINKUP_HWCTRL *lu_ctrl = (PEER_LINKUP_HWCTRL *)wsys->priv;
	UINT16 txop_level = TXOP_0;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
	struct _STA_TR_ENTRY *tr_entry = (struct _STA_TR_ENTRY *)sta_rec->priv;

	/*check starec is exist should not add new starec for this wcid*/
	if (get_starec_by_wcid(ad, sta_rec->WlanIdx))
		goto end;

	if (sta_rec->EnableFeature & ~STA_REC_INSTALL_KEY_FEATURE) {
		AsicUpdateRxWCIDTable(ad, sta_rec->WlanIdx, tr_entry->Addr, FALSE, FALSE); /* Haipin: Check IsReset */
		AsicStaRecUpdate(ad, sta_rec);
	}

	if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE)
		AsicAddRemoveKeyTab(ad, &sta_rec->asic_sec_info);

	/*update starec to tr_entry*/
	wifi_sys_update_starec(ad, sta_rec);

	if (ad->CommonCfg.bEnableTxBurst) {
		txop_level = TXOP_80;

		if (ad->CommonCfg.bRdg)
			txop_level = TXOP_80;
	} else
		txop_level = TXOP_0;

	hw_set_tx_burst(ad, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);

	if (wdev->wdev_type == WDEV_TYPE_AP) {
#ifdef DOT11_N_SUPPORT

		if (lu_ctrl->bRdgCap) {
		}

#endif
	}

end:
	if (lu_ctrl)
		os_free_mem(lu_ctrl);

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
static NTSTATUS hw_ctrl_flow_v1_peer_update(struct WIFI_SYS_CTRL *wsys)
{
	struct wifi_dev *wdev = wsys->wdev;
	struct _RTMP_ADAPTER *ad = (PRTMP_ADAPTER)wdev->sys_handle;
	struct _STA_REC_CTRL_T *sta_rec = &wsys->StaRecCtrl;
#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	UINT32 featues = 0;

	/*update ra rate*/
	if ((sta_rec->EnableFeature & STA_REC_RA_UPDATE_FEATURE) && wsys->priv) {
		AsicRaParamStaRecUpdate(ad,
								sta_rec->WlanIdx,
								(CMD_STAREC_AUTO_RATE_UPDATE_T *)wsys->priv,
								STA_REC_RA_UPDATE_FEATURE);

		if (wsys->priv)
			os_free_mem(wsys->priv);

		return NDIS_STATUS_SUCCESS;
	}

	/*update ra reldated setting, can't change the order*/
	if (sta_rec->EnableFeature & STA_REC_RA_COMMON_INFO_FEATURE) {
		featues = sta_rec->EnableFeature;
		sta_rec->EnableFeature = STA_REC_RA_COMMON_INFO_FEATURE;
		AsicStaRecUpdate(ad, sta_rec);
		sta_rec->EnableFeature = featues & (~STA_REC_RA_COMMON_INFO_FEATURE);
	}

	if (sta_rec->EnableFeature & STA_REC_RA_FEATURE) {
		featues = sta_rec->EnableFeature;
		sta_rec->EnableFeature = STA_REC_RA_FEATURE;
		AsicStaRecUpdate(ad, sta_rec);
		sta_rec->EnableFeature = featues & (~STA_REC_RA_FEATURE);
	}

#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

	if (sta_rec->EnableFeature & STA_REC_INSTALL_KEY_FEATURE) {
		AsicAddRemoveKeyTab(ad, &sta_rec->asic_sec_info);
		sta_rec->EnableFeature &= ~STA_REC_INSTALL_KEY_FEATURE;
	}

	/*normal update*/
	if (sta_rec->EnableFeature) {
		AsicStaRecUpdate(ad, sta_rec);
		/*update starec to tr_entry*/
		wifi_sys_update_starec_info(ad, sta_rec);
	}

	return NDIS_STATUS_SUCCESS;
}

/*
*
*/
VOID hw_ctrl_ops_v1_register(struct _HWCTRL_OP *hwctrl_ops)
{
	hwctrl_ops->wifi_sys_open = hw_ctrl_flow_v1_open;
	hwctrl_ops->wifi_sys_close = hw_ctrl_flow_v1_close;
	hwctrl_ops->wifi_sys_link_up = hw_ctrl_flow_v1_link_up;
	hwctrl_ops->wifi_sys_link_down = hw_ctrl_flow_v1_link_down;
	hwctrl_ops->wifi_sys_connt_act = hw_ctrl_flow_v1_connt_act;
	hwctrl_ops->wifi_sys_disconnt_act = hw_ctrl_flow_v1_disconnt_act;
	hwctrl_ops->wifi_sys_peer_update = hw_ctrl_flow_v1_peer_update;
}

#endif /*WIFI_SYS_FW_V1*/
