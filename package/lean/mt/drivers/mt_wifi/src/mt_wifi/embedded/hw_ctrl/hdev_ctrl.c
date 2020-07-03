/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hdev_ctrl.c
*/
#include	"rt_config.h"
#include "hdev/hdev.h"

/*
* local function
*/
#ifdef DBDC_MODE
static VOID hcGetBandTypeName(UCHAR Type, UCHAR *Str, UINT32 max_len)
{
	switch (Type) {
	case DBDC_TYPE_WMM:
		snprintf(Str, max_len, "%s", "WMM");
		break;

	case DBDC_TYPE_MGMT:
		snprintf(Str, max_len, "%s", "MGMT");
		break;

	case DBDC_TYPE_BSS:
		snprintf(Str, max_len, "%s", "BSS");
		break;

	case DBDC_TYPE_MBSS:
		snprintf(Str, max_len, "%s", "MBSS");
		break;

	case DBDC_TYPE_REPEATER:
		snprintf(Str, max_len, "%s", "REPEATER");
		break;

	case DBDC_TYPE_MU:
		snprintf(Str, max_len, "%s", "MU");
		break;

	case DBDC_TYPE_BF:
		snprintf(Str, max_len, "%s", "BF");
		break;

	case DBDC_TYPE_PTA:
		snprintf(Str, max_len, "%s", "PTA");
		break;
	}
}
#endif

/*
 *
*/
/*Only this function can use pAd*/
INT32 hdev_ctrl_init(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl  *ctrl = NULL;
	UINT32  ret;

	ret  =  os_alloc_mem(NULL, (UCHAR **)&ctrl, sizeof(struct hdev_ctrl));

	if (ctrl == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				 ("%s(): Allocate Hardware device Configure  fail!!\n", __func__));
		return -1;
	}

	os_zero_mem(ctrl, sizeof(struct hdev_ctrl));
	ctrl->priv  = (VOID *)pAd;
	pAd->hdev_ctrl = (VOID *)ctrl;
	return 0;
}

/*
*
*/
VOID hdev_resource_init(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	struct _RTMP_ADAPTER *ad = ctrl->priv;

	/*initial hardware resource*/
	HdevHwResourceInit(ctrl);
	/*initial resource*/
	/*hook hif*/
#if defined(RTMP_MAC_PCI)
	if (IS_PCIE_INF(ad) || IS_RBUS_INF(ad))
		ctrl->hif = &ad->PciHif;
#endif /*RTMP_MAC_PCI*/
	ctrl->cookie = ad->OS_Cookie;
	ctrl->mcu_ctrl = &ad->MCUCtrl;
	ctrl->arch_ops = &ad->archOps;
}

/*
 *
*/
VOID hdev_ctrl_exit(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	/*exist hw resource*/
	HdevHwResourceExit(ctrl);
	/*exist hdevcfg*/
	pAd->hdev_ctrl = NULL;
	os_free_mem(ctrl);
}

/*
 *
*/
VOID HcDevExit(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UCHAR i;
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++)
		HdevExit(ctrl, i);
}

/*
*
*/
INT32 HcAcquireRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = HC_STATUS_OK;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	struct hdev_obj *obj = wdev->pHObj;

	rdev = RcAcquiredBandForObj(ctrl, obj, wdev->wdev_idx, wdev->PhyMode, wdev->channel, wdev->wdev_type);

	/*correct wdev configure, if configure is not sync with hdev */
	if (!wmode_band_equal(wdev->PhyMode, RcGetPhyMode(rdev))) {
		wdev->PhyMode = RcGetPhyMode(rdev);
		wdev->channel = RcGetChannel(rdev);
	}

	BuildChannelList(pAd, wdev);
	/*temporal set, will be repaced by HcGetOmacIdx*/
	wdev->OmacIdx = obj->OmacIdx;
	/* Initialize the pDot11H of wdev */
	UpdateDot11hForWdev(wdev->sys_handle, wdev, TRUE);
	/*re-init operation*/
	wlan_operate_init(wdev);
	return ret;
}

/*
*
*/
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct hdev_obj *obj = wdev->pHObj;

	OS_SPIN_LOCK(&obj->RefCntLock);

	if (obj->RefCnt > 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				 ("%s(): there are other link reference the Obj\n", __func__));
		OS_SPIN_UNLOCK(&obj->RefCntLock);
		return ret;
	}

	OS_SPIN_UNLOCK(&obj->RefCntLock);
	RcReleaseBandForObj(ctrl, obj);
	return ret;
}

/*
*
*/
UCHAR HcGetBandByWdev(struct wifi_dev *wdev)
{
	UCHAR BandIdx = 0;
	struct hdev_obj *obj;
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;

	ASSERT(wdev);
	/* exit from here if wdev is null */
	if (!wdev)
		return 0;

	obj = wdev->pHObj;
	if (hdev_obj_state_ready(obj))
		BandIdx = RcGetBandIdx(obj->rdev);
	else {
		if ((ad->CommonCfg.dbdc_mode) && (!BOARD_IS_5G_ONLY(ad))) {
			if (WMODE_CAP_5G(wdev->PhyMode))
				BandIdx = DBDC_BAND1;
			else
				BandIdx = DBDC_BAND0;
		} else
			BandIdx = 0;
	}

	return BandIdx;
}

/*
*
*/
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (hdev_obj_state_ready(obj))
		RcSetRadioCurStat(obj->rdev, CurStat);
	else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): obj is not ready!!\n", __func__));
}

/*
*
*/
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): no hdev parking on channel:%d !!!\n",
				 __func__, Channel));
		return;
	}

	RcSetRadioCurStat(rdev, CurStat);
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = (struct hdev_ctrl *)pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		rdev->pRadioCtrl->CurStat = PHY_RADIOOFF;
	}
}

/*
*
*/
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		rdev->pRadioCtrl->CurStat = PHY_INUSE;
	}
}

/*
*
*/
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev_idx %d obj is not ready, return TRUE !!!\n",
			__func__, wdev->wdev_idx));
		return TRUE;
	}

	if (RcGetRadioCurStat(obj->rdev) == PHY_RADIOOFF)
		return TRUE;
	else
		return FALSE;
}

/*
*
*/
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): no hdev parking on channel:%d!!!\n",
				 __func__, Channel));
		return TRUE;
	}

	if (RcGetRadioCurStat(rdev) == PHY_RADIOOFF)
		return TRUE;
	else
		return FALSE;
}

/*
*
*/
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResourceCfg =  &ctrl->HwResourceCfg;
	struct radio_dev *rdev = NULL;
	UCHAR i;
	BOOLEAN AllSupportedBandsRadioOff = TRUE;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];

		if ((rdev->pRadioCtrl->CurStat == PHY_INUSE) && (rdev->pRadioCtrl->CurStat != PHY_RADIOOFF)) {
			AllSupportedBandsRadioOff = FALSE;
			break;
		}
	}

	return AllSupportedBandsRadioOff;
}

#ifdef GREENAP_SUPPORT
/*
*
*/
VOID HcSetGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return;

	rdev = &ctrl->rdev[BandIdx];

	if (!rdev)
		return;

	rdev->pRadioCtrl->bGreenAPActive = bGreenAPActive;
}

/*
*
*/
BOOLEAN IsHcGreenAPActiveByBand(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;

	if (!ctrl)
		return FALSE;

	rdev = &ctrl->rdev[BandIdx];

	if (!rdev)
		return FALSE;

	return rdev->pRadioCtrl->bGreenAPActive;
}

/*
*
*/
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev = NULL;

	if (!hdev_obj_state_ready(obj))
		return FALSE;

	rdev = obj->rdev;

	return rdev->pRadioCtrl->bGreenAPActive;
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
UCHAR HcGetChannelByBf(RTMP_ADAPTER *pAd)
{
	struct radio_dev *rdev = RcGetBandIdxByBf(pAd->hdev_ctrl);

	if (rdev == NULL) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): no hdev can support beamform!\n", __func__));
		return 0;
	}

	return rdev->pRadioCtrl->Channel;
}

/*
*
*/
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev)
{
	if (!hdev_obj_state_ready(wdev->pHObj))
		return FALSE;

	return RcIsBfCapSupport(wdev->pHObj);
}

#ifdef MAC_REPEATER_SUPPORT
/*
*
*/
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx)
{
	INT32 ret = 0;
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return HC_STATUS_FAIL;
	}

	/*Acquire Repeater OMACIdx*/
	OcAddRepeaterEntry(obj, ReptIdx);
	RcUpdateRepeaterEntry(obj->rdev, ReptIdx);
	return ret;
}

/*
*
*/
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx)
{
	INT32 ret = 0;
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return HC_STATUS_FAIL;
	}

	/*Acquire Repeater OMACIdx*/
	OcDelRepeaterEntry(obj, ReptIdx);
	return ret;
}

/*
*
*/
UCHAR HcGetRepeaterOmac(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	REPEATER_CLIENT_ENTRY *pReptEntry = NULL;
	HD_REPT_ENRTY *pHReptEntry = NULL;
	UCHAR ReptOmacIdx = 0xff;

	pReptEntry = RTMPLookupRepeaterCliEntry(
					 pAd,
					 FALSE,
					 pEntry->ReptCliAddr,
					 TRUE);

	if (pReptEntry) {
		pHReptEntry = OcGetRepeaterEntry(pReptEntry->wdev->pHObj, pReptEntry->MatchLinkIdx);

		if (pHReptEntry)
			ReptOmacIdx = pHReptEntry->ReptOmacIdx;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			 ("%s(): Get ReptOmacIdx: %d!\n", __func__, ReptOmacIdx));
	return ReptOmacIdx;
}
#endif /*#MAC_REPEATER_SUPPORT*/

/*
*
*/
INT32 HcRadioInit(RTMP_ADAPTER *pAd, UCHAR RfIC, UCHAR DbdcMode)
{
	INT32 ret = 0;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	RcRadioInit(ctrl, RfIC, DbdcMode);
	return ret;
}

/*
*
*/
INT32 HcUpdateCsaCntByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	INT32 ret = 0;
	struct radio_dev *rdev = NULL;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct hdev_obj *obj;
	struct wifi_dev *wdev;
	struct DOT11_H *pDot11h = NULL;

	rdev = RcGetHdevByChannel(ctrl, Channel);
	if (!rdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): Update Channel %d faild, not support this RF\n",
				  __func__, Channel));
		return -1;
	}

	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = pAd->wdev_list[obj->Idx];

		if (wdev == NULL)
			continue;

		pDot11h = wdev->pDot11_H;

		if (pDot11h == NULL)
			return -1;
#ifdef CUSTOMER_DCC_FEATURE
		if (pAd->CommonCfg.channelSwitch.CHSWMode == CHANNEL_SWITCHING_MODE) {
			wdev->csa_count = pAd->CommonCfg.channelSwitch.CHSWPeriod;
			UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
		} else
#endif
			if (pDot11h->RDMode != RD_SILENCE_MODE) {
				pDot11h->wdev_count++;
				wdev->csa_count = pDot11h->CSPeriod;
				UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);
			}
	}
	return ret;
}

#ifdef DBDC_MODE
/*
*
*/
VOID HcShowBandInfo(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	BCTRL_INFO_T BctrlInfo;
	BCTRL_ENTRY_T *pEntry = NULL;
	CHAR TempStr[16] = "";

	os_zero_mem(&BctrlInfo, sizeof(BCTRL_INFO_T));
	AsicGetDbdcCtrl(pAd, &BctrlInfo);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDbdcEnable: %d\n", BctrlInfo.DBDCEnable));

	for (i = 0; i < BctrlInfo.TotalNum; i++) {
		pEntry = &BctrlInfo.BctrlEntries[i];
		hcGetBandTypeName(pEntry->Type, TempStr, sizeof(TempStr));

		if (pEntry->Type != DBDC_TYPE_MBSS)
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t(%s,%d): Band %d\n", TempStr, pEntry->Index, pEntry->BandIdx));
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t(%s,0-%d): Band %d\n", TempStr, pEntry->Index+1, pEntry->BandIdx));
	}
}
#endif

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx, ChIdx;
	CHANNEL_CTRL *pChCtrl;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================START====================\n "));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("---------------------------------------------\n "));

	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);

		if (pChCtrl->ChListNum == 0) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\t\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d (it is not available)\n ", BandIdx, pChCtrl->ChListNum));
			break;
		} else {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t\x1b[1;33mBandIdx = %d\x1b[m, ChannelListNum = %d\n ", BandIdx, pChCtrl->ChListNum));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChGrpABandEn = %d\n ", pChCtrl->ChGrpABandEn));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannel list information:\n "));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tChannel \tPwr0/1 \t\tFlags\n "));
			for (ChIdx = 0; ChIdx < pChCtrl->ChListNum; ChIdx++) {
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t#%d \t\t%d/%d \t\t%x\n ",
					pChCtrl->ChList[ChIdx].Channel,
					pChCtrl->ChList[ChIdx].Power,
					pChCtrl->ChList[ChIdx].Power2,
					pChCtrl->ChList[ChIdx].Flags));
			}
		}
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("---------------------------------------------\n "));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=====================END=====================\n "));
}
#ifdef GREENAP_SUPPORT
/*
 *
 */
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd)
{
	struct greenap_ctrl *greenap = &pAd->ApCfg.greenap;

	greenap_show(pAd, greenap);
}
#endif /* GREENAP_SUPPORT */

/*
*
*/
void hc_show_edca_info(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	WcShowEdca(ctrl);
}

/*
*
*/
void hc_show_radio_info(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	HdevCfgShow(ctrl);
}

/*
*
*/
void hc_show_hdev_obj(struct wifi_dev *wdev)
{
	if (!hdev_obj_state_ready(wdev->pHObj))
		return;

	HdevObjShow(wdev->pHObj);
}

/*
*
*/
VOID HcAcquiredEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, EDCA_PARM *pEdca)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return;
	}
	WcAcquiredEdca(obj, pEdca);
}

/*
*
*/
VOID HcReleaseEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return;
	}
	WcReleaseEdca(obj);
}

/*
*
*/
VOID HcSetEdca(struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return;
	}
	WcSetEdca(obj);
}

/*
*
*/
UCHAR HcGetOmacIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return 0xff;
	}
	return obj->OmacIdx;
}

/*
*  Need refine
*/


/*
* Only temporal usage, should remove when cmm_asic_xxx.c is not apply pAd
*/

UCHAR  HcGetChannelByRf(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return pHwResource->PhyCtrl[i].RadioCtrl.Channel;
	}

	return 0;
}

/*
* for Single Band Usage
*/
UCHAR  HcGetRadioChannel(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[0].RadioCtrl.Channel;
}

/*
*
*/
UCHAR HcGetRadioPhyMode(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[0].RadioCtrl.PhyMode;
}

UCHAR HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return pHwResource->PhyCtrl[BandIdx].RadioCtrl.PhyMode;
}
/*
*
*/
UCHAR HcGetRadioRfIC(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	return ctrl->HwResourceCfg.PhyCtrl[0].rf_band_cap;
}

/*
*
*/
BOOLEAN  HcIsRfSupport(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return TRUE;
	}

	return FALSE;
}

/*
*
*/
BOOLEAN  HcIsRfRun(RTMP_ADAPTER *pAd, UCHAR RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	struct radio_dev *rdev;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];

		if (WMODE_CAP_2G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_24GHZ))
			return TRUE;
		else if (WMODE_CAP_5G(rdev->pRadioCtrl->PhyMode) && (RfIC & RFIC_5GHZ))
			return TRUE;
	}

	return FALSE;
}

#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
/*
*
*/
QLOAD_CTRL *HcGetQloadCtrlByRf(RTMP_ADAPTER *pAd, UINT32 RfIC)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].rf_band_cap & RfIC)
			return &pHwResource->PhyCtrl[i].QloadCtrl;
	}

	return 0;
}

/*
*
*/
QLOAD_CTRL *HcGetQloadCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].QloadCtrl;
}
#endif /*AP_QLOAD_SUPPORT*/

/*
*
*/
AUTO_CH_CTRL *HcGetAutoChCtrl(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource =  &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[0].AutoChCtrl;
}
#endif /*CONFIG_AP_SUPPORT*/
#ifdef CONFIG_AP_SUPPORT
AUTO_CH_CTRL *HcGetAutoChCtrlbyBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	HD_RESOURCE_CFG *pHwResource =  &ctrl->HwResourceCfg;

	return &pHwResource->PhyCtrl[BandIdx].AutoChCtrl;
}
#endif

/*
*
*/
UCHAR HcGetBw(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return 0xff;
	}

	return RcGetBw(obj->rdev);
}

/*
*
*/
UINT32 HcGetMgmtQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return TxQ_IDX_ALTX0;
	}

	return RcGetMgmtQueueIdx(obj, pkt_type);
}

/*
*
*/
UINT32 HcGetBcnQueueIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return TxQ_IDX_BCN0;
	}

	return RcGetBcnQueueIdx(obj);
}

/*
*
*/
UINT32 HcGetTxRingIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type, UCHAR q_idx)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return 0;
	}

	return RcGetTxRingIdx(obj);
}

/*
*
*/
UINT32 HcGetWmmIdx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return 0;
	}

	return RcGetWmmIdx(obj);
}

#ifdef CUSTOMER_DCC_FEATURE
INT32 HcUpdateExtCha(RTMP_ADAPTER *pAd, UCHAR Channel, UCHAR ExtCha)
{
	INT32 ret = 0;
	struct radio_dev *pHdev = NULL;
	struct hdev_ctrl *pHdCfg = (struct hdev_ctrl *)pAd->hdev_ctrl;

	pHdev = RcGetHdevByChannel(pHdCfg, Channel);
	if (!pHdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get Hdev by Channel %d faild, not support this RF\n",
		__FUNCTION__, Channel));
		return -1;
	}

	/*Update ExtCha to radio*/
	ret  = RcUpdateExtCha(pHdev, ExtCha);

	return ret;
}

UCHAR HcGetExtCha(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct radio_dev *rdev = NULL;
	struct hdev_ctrl *pHdCfg = pAd->hdev_ctrl;

	rdev = RcGetHdevByChannel(pHdCfg, Channel);

	if (!rdev) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Get Hdev by Channel %d faild, not support this RF\n",
		__FUNCTION__, Channel));
		return 0;
	}
	return RcGetExtCha(rdev);
}
#endif

/*
*
*/
UCHAR HcGetBandByChannel(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	UCHAR BandIdx;

	rdev = RcGetHdevByChannel(ctrl, Channel);

	if (!rdev) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): no hdev parking on channel:%d!\n", __func__, Channel));
		return 0;
	}

	BandIdx = RcGetBandIdx(rdev);
	return BandIdx;
}

/*
*
*/
EDCA_PARM *HcGetEdca(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	EDCA_PARM *pEdca = NULL;
	struct hdev_obj *obj = wdev->pHObj;
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return NULL;
	}

	pEdca = WcGetWmmByIdx(ctrl, obj->WmmIdx);
	return pEdca;
}

/*
*
*/
VOID HcCrossChannelCheck(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel)
{
	UCHAR PhyMode = wdev->PhyMode;
	UCHAR WChannel = wdev->channel;

	/*check channel is belong to differet band*/
	if (Channel > 14 && WChannel > 14)
		return;

	if (Channel <= 14 && WChannel <= 14)
		return;

	/*is mixed mode, change default channel and */
	if (!WMODE_5G_ONLY(PhyMode)	|| !WMODE_2G_ONLY(PhyMode)) {
		/*update wdev channel to new band*/
		wdev->channel = Channel;
		/*need change to other band*/
		HcAcquireRadioForWdev(pAd, wdev);
	}

	return;
}

/*
 * Description:
 *
 * the function will check all enabled function,
 * check the bssid num is defined,
 *
 * preserve the group key wtbl num will be used.
 * then decide the max station number could be used.
 */
UCHAR HcGetMaxStaNum(RTMP_ADAPTER *pAd)
{
	UCHAR MaxStaNum = WtcGetMaxStaNum(pAd->hdev_ctrl);

	if (MaxStaNum > MAX_LEN_OF_MAC_TABLE)
		MaxStaNum = MAX_LEN_OF_MAC_TABLE;

	return MaxStaNum;
}

UCHAR HcSetMaxStaNum(RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;
	UCHAR BssidNum = 0, MSTANum = 0;
#ifdef CONFIG_AP_SUPPORT
	BssidNum = pAd->ApCfg.BssidNum;
#endif /*CONFIG_AP_SUPPORT*/
	return WtcSetMaxStaNum(ctrl, BssidNum, MSTANum);
}

/*
*
*/
UCHAR HcAcquireGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return INVAILD_WCID;
	}

	wdev->tr_tb_idx = WtcAcquireGroupKeyWcid(pAd->hdev_ctrl, obj);
	return wdev->tr_tb_idx;
}

/*
*
*/
VOID HcReleaseGroupKeyWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return;
	}

	wdev->tr_tb_idx = WtcReleaseGroupKeyWcid(pAd->hdev_ctrl, obj, idx);
}

/*
*
*/
UCHAR HcGetWcidLinkType(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	return WtcGetWcidLinkType(pAd->hdev_ctrl, Wcid);
}


/*
*
*/
UCHAR HcAcquireUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return INVAILD_WCID;
	}

	return WtcAcquireUcastWcid(pAd->hdev_ctrl, obj);
}


/*
*
*/
UCHAR HcReleaseUcastWcid(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx)
{
	struct hdev_obj *obj = wdev->pHObj;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return INVAILD_WCID;
	}

	return WtcReleaseUcastWcid(pAd->hdev_ctrl, obj, idx);
}

/*
*
*/
VOID HcWtblRecDump(RTMP_ADAPTER *pAd)
{
	WtcRecDump(pAd->hdev_ctrl);
}


/*
*
*/
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev)
{
	return hdev_obj_state_ready(wdev->pHObj);
}

UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd)
{
	struct hdev_ctrl *ctrl = pAd->hdev_ctrl;

	return ctrl->HwResourceCfg.concurrent_bands;
}

static INT32 HcSuspendMSDUTx(struct radio_dev *rdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj;
	struct hdev_ctrl *ctrl = rdev->priv;
	struct _RTMP_ADAPTER *ad = ctrl->priv;
	struct wifi_dev *wdev;

	/*update all of wdev*/
	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = ad->wdev_list[obj->Idx];
		RTMPSuspendMsduTransmission(wdev->sys_handle, wdev);
	}
	return ret;
}

static INT32 HcUpdateMSDUTxAllow(struct radio_dev *rdev)
{
	INT32 ret = 0;
	struct hdev_obj *obj;
	struct hdev_ctrl *ctrl = rdev->priv;
	struct _RTMP_ADAPTER *ad = ctrl->priv;
	struct wifi_dev *wdev;

	/*update all of wdev*/
	DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
		wdev = ad->wdev_list[obj->Idx];

		if (wdev->channel == rdev->pRadioCtrl->Channel)
			RTMPResumeMsduTransmission(wdev->sys_handle, wdev);
		else
			RTMPSuspendMsduTransmission(wdev->sys_handle, wdev);
	}
	return ret;
}

/*
*
*/
static VOID hc_radio_update(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev  *rdev;
	struct freq_oper *oper = res->oper;
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif
	BOOLEAN scan = (res->reason == REASON_NORMAL_SCAN) ? TRUE:FALSE;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return;
	}
	rdev = obj->rdev;
#ifdef CONFIG_AP_SUPPORT
#ifdef AP_QLOAD_SUPPORT
	/* clear all statistics count for QBSS Load */
	QBSS_LoadStatusClear(wdev->sys_handle, oper->prim_ch);
#endif /* AP_QLOAD_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	HcSuspendMSDUTx(rdev);
	AsicSwitchChannel(wdev->sys_handle, rdev->Idx, oper, scan);
	AsicSetBW(wdev->sys_handle, oper->bw, rdev->Idx);
	RcUpdateRadio(rdev, oper->bw, oper->cen_ch_1, oper->cen_ch_2, oper->ext_cha);
	RcUpdateChannel(rdev, oper->prim_ch, scan);
	/*after update channel resum tx*/
	HcUpdateMSDUTxAllow(rdev);
#if defined(MT_DFS_SUPPORT) && defined(BACKGROUND_SCAN_SUPPORT)
	DfsInitDedicatedScanStart(ad);
#endif
}

/*
*
*/
BOOLEAN hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev;
#ifdef MT_WOW_SUPPORT
	struct _RTMP_ADAPTER *ad = (struct _RTMP_ADAPTER *)wdev->sys_handle;
#endif /*MT_WOW_SUPPORT*/

#ifdef BW_VENDOR10_CUSTOM_FEATURE
	/* Sync SoftAp BW for Down Case */
	if (wdev->wdev_type == WDEV_TYPE_AP && wlan_operate_get_state(wdev) == WLAN_OPER_STATE_INVALID) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s(): AP wdev=%d, Interface Down!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}
#endif

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return FALSE;
	}

	rdev = obj->rdev;

	if (rc_radio_equal(rdev, res->oper)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): radio is equal, prim_ch=%d!\n", __func__, res->oper->prim_ch));
		return TRUE;
	}

	if (rc_radio_res_acquire(rdev, res) != TRUE) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): can't acquire radio resource!\n", __func__));
		return FALSE;
	}

#ifdef MT_WOW_SUPPORT

	if (ad->WOW_Cfg.bWoWRunning) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("[%s] WoW is running, skip!\n", __func__));
		return FALSE;
	}

#endif /*MT_WOW_SUPPORT*/
	/*update to radio resouce*/
	hc_radio_update(wdev, res);
	return TRUE;
}

/*
*
*/
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad)
{
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct radio_dev *rdev = NULL;
	struct freq_oper freq;
	struct _RADIO_CTRL *radio_ctrl;
	UCHAR i;

	for (i = 0 ; i < ctrl->HwResourceCfg.concurrent_bands; i++) {
		os_zero_mem(&freq, sizeof(freq));
		rdev = &ctrl->rdev[i];
		radio_ctrl = rdev->pRadioCtrl;
		freq.bw = radio_ctrl->Bw;
		freq.prim_ch = radio_ctrl->Channel;
		freq.cen_ch_1 = radio_ctrl->CentralCh;
		freq.cen_ch_2 = radio_ctrl->Channel2;
		AsicSwitchChannel(ad, i, &freq, FALSE);
	}
	return TRUE;
}

/*
*
*/
VOID hc_set_rrm_init(struct wifi_dev *wdev)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	UCHAR band_idx = HcGetBandByWdev(wdev);

	AsicSetTxStream(wdev->sys_handle, ad->Antenna.field.TxPath, OPMODE_AP, TRUE, band_idx);
	AsicSetRxStream(wdev->sys_handle, ad->Antenna.field.RxPath, band_idx);
}

/*
*
*/
INT  hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper)
{
	struct hdev_obj *obj = wdev->pHObj;
	struct radio_dev *rdev;
	struct _RADIO_CTRL *radio;

	if (!hdev_obj_state_ready(obj)) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s(): wdev=%d, hobj is not ready!\n", __func__, wdev->wdev_idx));
		return HC_STATUS_FAIL;
	}

	rdev = obj->rdev;
	radio = rdev->pRadioCtrl;
	oper->bw = radio->Bw;
	oper->cen_ch_1 = radio->CentralCh;
	oper->cen_ch_2 = radio->Channel2;
	oper->ext_cha = radio->ExtCha;
	oper->prim_ch = radio->Channel;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
	return HC_STATUS_OK;
}

/*
*
*/
INT  hc_radio_query_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel, struct freq_oper *oper)
{
	INT ret = HC_STATUS_FAIL;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct _RADIO_CTRL *radio = NULL;
	UCHAR i;

	for (i = 0 ; i < res->concurrent_bands; i++) {
		radio = &res->PhyCtrl[i].RadioCtrl;
		if (radio->Channel == channel) {
			oper->bw = radio->Bw;
			oper->cen_ch_1 = radio->CentralCh;
			oper->cen_ch_2 = radio->Channel2;
			oper->ext_cha = radio->ExtCha;
			oper->prim_ch = radio->Channel;
			oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
			oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
			ret = HC_STATUS_OK;
			break;
		}
	}
	return ret;
}

/*
* suggest only used by phy related features, others should use hc_radio_query_by_wdev
*/
INT  hc_radio_query_by_index(struct _RTMP_ADAPTER *ad, UCHAR index, struct freq_oper *oper)
{
	INT ret = HC_STATUS_OK;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct _RADIO_CTRL *radio = NULL;

	radio = &res->PhyCtrl[index].RadioCtrl;
	oper->bw = radio->Bw;
	oper->cen_ch_1 = radio->CentralCh;
	oper->cen_ch_2 = radio->Channel2;
	oper->ext_cha = radio->ExtCha;
	oper->prim_ch = radio->Channel;
	oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
	oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
	return ret;
}

/*
* temporally use, only query first freq_oper by rfic, not support in 5G+5G or 2G+2G case
*/
INT hc_radio_query_by_rf(struct _RTMP_ADAPTER *ad, UCHAR rfic, struct freq_oper *oper)
{
	INT ret = HC_STATUS_FAIL;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;
	struct _HD_RESOURCE_CFG *res = &ctrl->HwResourceCfg;
	struct _RADIO_CTRL *radio = NULL;
	UCHAR i;

	for (i = 0 ; i < res->concurrent_bands; i++) {
		radio = &res->PhyCtrl[i].RadioCtrl;
		if (wmode_2_rfic(radio->PhyMode) & rfic) {
			oper->bw = radio->Bw;
			oper->cen_ch_1 = radio->CentralCh;
			oper->cen_ch_2 = radio->Channel2;
			oper->ext_cha = radio->ExtCha;
			oper->prim_ch = radio->Channel;
			oper->ht_bw = (oper->bw > BW_20) ? HT_BW_40 : HT_BW_20;
			oper->vht_bw = rf_bw_2_vht_bw(oper->bw);
			ret = HC_STATUS_OK;
			break;
		}
	}
	return ret;
}

/*
*
*/
INT hc_obj_init(struct wifi_dev *wdev, INT idx)
{
	struct _RTMP_ADAPTER *ad = wdev->sys_handle;
	struct hdev_ctrl *ctrl = ad->hdev_ctrl;

	wdev->pHObj = &ctrl->HObjList[idx];
	return HC_STATUS_OK;
}

/*
*
*/
VOID hc_obj_exit(struct wifi_dev *wdev)
{
	wdev->pHObj = NULL;
}

/*
*
*/
inline struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	return &ctrl->chip_cap;
}
EXPORT_SYMBOL(hc_get_chip_cap);

/*
*
*/
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_ops;
}
EXPORT_SYMBOL(hc_get_chip_ops);

/*
*
*/
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	os_move_mem(&ctrl->chip_ops, ops, sizeof(*ops));
}

/*
*
*/
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2)
{
	os_move_mem(&ChCtrl->ChList[ChIdx], &pAd->TxPower[ChIdx2], sizeof(CHANNEL_TX_POWER));
	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx)
{
	ChCtrl->ChList[ChIdx].Flags |= ChannelListFlag;
	return HC_STATUS_OK;
}

UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	ChCtrl->ChListStat = ChListStat;
	return HC_STATUS_OK;
}

CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl, UCHAR BandIdx)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;
	return &ctrl->ChCtrl[BandIdx];
}

UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx;
	CHANNEL_CTRL *pChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pChCtrl = hc_get_channel_ctrl(pAd->hdev_ctrl, BandIdx);
		os_zero_mem(pChCtrl, sizeof(CHANNEL_CTRL));
	}
	return HC_STATUS_OK;
}
#ifdef CONFIG_AP_SUPPORT
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd)
{
	UCHAR BandIdx;
	AUTO_CH_CTRL *pAutoChCtrl;
	for (BandIdx = 0; BandIdx < DBDC_BAND_NUM; BandIdx++) {
		pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
		pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = NULL;
		pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0;
		pAutoChCtrl->AutoChSelCtrl.ChListNum = 0;
		pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
		os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));
	}
	return HC_STATUS_OK;
}
UCHAR hc_init_ACSChCtrlByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	AUTO_CH_CTRL *pAutoChCtrl = HcGetAutoChCtrlbyBandIdx(pAd, BandIdx);
	pAutoChCtrl->AutoChSelCtrl.pScanReqwdev = NULL;
	pAutoChCtrl->AutoChSelCtrl.ScanChIdx = 0;
	pAutoChCtrl->AutoChSelCtrl.ChListNum = 0;
	pAutoChCtrl->AutoChSelCtrl.ACSChStat = ACS_CH_STATE_NONE;
	os_zero_mem(pAutoChCtrl->AutoChSelCtrl.AutoChSelChList, (MAX_NUM_OF_CHANNELS+1)*sizeof(AUTOCH_SEL_CH_LIST));
	return HC_STATUS_OK;
}
#endif

UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat)
{
	return (ChCtrl->ChListStat == ChListStat);
}
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return &ctrl->chip_dbg;
}

/*
*
*/
UINT32 hc_get_hif_type(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.hif_type;
}

/*
*
*/
UINT32 hc_get_asic_cap(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.asic_caps;
}

/*
*
*/
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.asic_caps |= caps;
}

/*
*
*/
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	ctrl->chip_cap.asic_caps &= ~(caps);
}

/*
*
*/
UINT8 hc_get_chip_bcn_max_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.BcnMaxNum;
}

/*
*
*/
UINT8 hc_get_chip_bcn_hw_num(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.BcnMaxHwNum;
}

/*
*
*/
BOOLEAN hc_get_chip_wapi_sup(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->chip_cap.FlgIsHwWapiSup;
}

/*
*
*/
inline VOID *hc_get_hif_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->hif;
}

/*
*
*/
VOID *hc_get_os_cookie(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->cookie;
}

/*
*
*/
VOID *hc_get_mcu_ctrl(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->mcu_ctrl;
}

/*
*
*/
struct _RTMP_ARCH_OP *hc_get_asic_ops(void *hdev_ctrl)
{
	struct hdev_ctrl *ctrl = hdev_ctrl;

	return ctrl->arch_ops;
}

