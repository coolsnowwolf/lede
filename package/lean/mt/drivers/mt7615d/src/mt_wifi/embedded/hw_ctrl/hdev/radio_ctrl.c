/*
 ***************************************************************************
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
#include "hdev/hdev.h"

/*Radio controller*/

/*
 *
*/

/*Local functions*/
static UCHAR rcGetRfByIdx(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
#ifdef DBDC_MODE
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (ctrl->chip_ops.BandGetByIdx && DbdcMode)
		return ctrl->chip_ops.BandGetByIdx(pAd, BandIdx);
	else
		return RFIC_DUAL_BAND;

#endif /*DBDC_MODE*/
	return RFIC_DUAL_BAND;
}


/*Get RfIC Band from EEPORM content*/
static UINT8 rcGetBandSupport(struct hdev_ctrl *ctrl, UCHAR DbdcMode, UCHAR BandIdx)
{
	/* TODO: Should remove when antenna move to rdev */
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (BOARD_IS_5G_ONLY(pAd))
		return RFIC_5GHZ;
	else if (BOARD_IS_2G_ONLY(pAd))
		return RFIC_24GHZ;
	else if (RFIC_IS_5G_BAND(pAd))
		return rcGetRfByIdx(ctrl, DbdcMode, BandIdx);
	else
		return RFIC_24GHZ;
}

static UCHAR rcGetDefaultChannel(UCHAR PhyMode)
{
	/*priority must the same as Default PhyMode*/
	if (WMODE_CAP_2G(PhyMode))
		return 1;
	else if (WMODE_CAP_5G(PhyMode))
		return 36;

	return 0;
}


static UCHAR rcGetDefaultPhyMode(UCHAR Channel)
{
	/*priority must the same as Default Channel*/
	if (Channel <= 14)
		return WMODE_B;
	else
		return WMODE_A;
}



static struct radio_dev *rcGetHdevByRf(struct hdev_ctrl *ctrl, UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	RADIO_CTRL *radio_ctrl = NULL;
	/*get hdev by phymode first*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		radio_ctrl = &pHwResource->PhyCtrl[i].RadioCtrl;
		if (wmode_2_rfic(radio_ctrl->PhyMode) & RfType)
			return &ctrl->rdev[i];
	}
	/*get hdev by cap*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if(pHwResource->PhyCtrl[i].rf_band_cap & RfType)
			return &ctrl->rdev[i];
	}
	return NULL;
}

static BOOLEAN rcCheckIsTheSameBand(UCHAR PhyMode, UCHAR Channel)
{
	if (WMODE_CAP_5G(PhyMode) && WMODE_CAP_2G(PhyMode))
		return TRUE;
	else if (WMODE_CAP_5G(PhyMode) && Channel  > 14)
		return TRUE;
	else if (WMODE_CAP_2G(PhyMode) && Channel <= 14)
		return TRUE;

	return FALSE;
}




#ifdef DBDC_MODE
static RADIO_CTRL *rcGetRadioCtrlByRf(struct hdev_ctrl *ctrl, UCHAR RfType)
{
	INT i;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (RfType &  pHwResource->PhyCtrl[i].rf_band_cap)
			return &pHwResource->PhyCtrl[i].RadioCtrl;
	}

	return NULL;
}


static VOID rcFillEntry(BCTRL_ENTRY_T *pEntry, UINT8 Type, UINT8 BandIdx, UINT8 Index)
{
	pEntry->Type = Type;
	pEntry->BandIdx = BandIdx;
	pEntry->Index = Index;
}



static INT32 rcUpdateBandForMBSS(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	UCHAR MbssIdx;
	struct hdev_ctrl *ctrl = rdev->priv;

	if (obj->OmacIdx == 0)
		rcFillEntry(pEntry, DBDC_TYPE_BSS, pRadioCtrl->BandIdx, 0);
	else {
		/*ctrl->chipCap.ExtMbssOmacStartIdx+1 since 0x10 will control by 0x10*/
		MbssIdx = obj->OmacIdx - (ctrl->chip_cap.ExtMbssOmacStartIdx+1);
		rcFillEntry(pEntry, DBDC_TYPE_MBSS, pRadioCtrl->BandIdx, MbssIdx);
	}

	return 0;
}


static INT32 rcUpdateBandForBSS(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	rcFillEntry(pEntry, DBDC_TYPE_BSS, pRadioCtrl->BandIdx, obj->OmacIdx);
	return 0;
}


static INT32 rcUpdateBandByType(struct hdev_obj *obj, BCTRL_ENTRY_T *pEntry)
{
	switch (obj->Type) {
	case WDEV_TYPE_AP: {
		rcUpdateBandForMBSS(obj, pEntry);
	}
	break;

	case WDEV_TYPE_STA:
	case WDEV_TYPE_ADHOC:
	case WDEV_TYPE_GO:
	case WDEV_TYPE_GC:
	case WDEV_TYPE_APCLI: {
		rcUpdateBandForBSS(obj, pEntry);
	}
	break;

	case WDEV_TYPE_WDS:
	case WDEV_TYPE_MESH:
	default: {
		/* TODO: STAR for DBDC */
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s(): Current not support this type of WdevType=%d\n", __func__, obj->Type));
		return -1;
	}
	break;
	}

	return 0;
}



/*Must call after update ownmac*/
static INT32 rcUpdateBandForBFMU(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	struct radio_dev *rdev;
	RADIO_CTRL *pRadioCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry = NULL;
	UINT32 i;
#ifdef TXBF_SUPPORT
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;
#endif /*TXBF_SUPPORT*/

	/*first choice 5G as the BF/MU band*/
	rdev = rcGetHdevByRf(ctrl, RFIC_5GHZ);

	/*else 2.4G*/
	if (!rdev || (rdev->DevNum == 0))
		rdev = rcGetHdevByRf(ctrl, RFIC_24GHZ);

	/*If MU is not enable & 5G not support , else select first dev as bf band*/
	if (!rdev)
		rdev = &ctrl->rdev[0];

	pRadioCtrl = rdev->pRadioCtrl;

	/*If get phyCtrl, set bf to this band*/
	if (pRadioCtrl == NULL)
		return -1;

	/*support MU & enable MU, BF & MU should be 5G only*/
	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	rcFillEntry(pEntry, DBDC_TYPE_MU, pRadioCtrl->BandIdx, 0);
	pBInfo->TotalNum++;

	for (i = 0; i < 3; i++) {
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_BF, pRadioCtrl->BandIdx, i);
		pBInfo->TotalNum++;
	}

	pRadioCtrl->IsBfBand = TRUE;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		if (pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand &&
			(pHwResource->PhyCtrl[i].RadioCtrl.BandIdx != pRadioCtrl->BandIdx))
			pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand = FALSE;
	}

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
	   MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s	PhyCtrl[%d].RadioCtrl.IsBfBand = %d\n", __func__, i, pHwResource->PhyCtrl[i].RadioCtrl.IsBfBand));
	}
#ifdef TXBF_SUPPORT
	TxBfModuleEnCtrl(pAd);
#endif /*TXBF_SUPPORT*/
	return 0;
}


static INT32 rcUpdateBandForRepeater(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;
	struct radio_dev  *rdev;
	struct hdev_obj *obj;
	HD_REPT_ENRTY *pReptEntry = NULL, *tmp = NULL;

	if (ctrl->HwResourceCfg.concurrent_bands == 2) { /* DBDC mode */
		for (i = 0; i < ctrl->HwResourceCfg.concurrent_bands; i++) {
			/* search repeater entry from all obj */
			rdev = &ctrl->rdev[i];
			DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
				DlListForEachSafe(pReptEntry, tmp, &obj->RepeaterList, struct _HD_REPT_ENRTY, list) {
					pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
					rcFillEntry(pEntry, DBDC_TYPE_REPEATER,
						rdev->pRadioCtrl->BandIdx, pReptEntry->CliIdx);
					pBInfo->TotalNum++;
				}
			}
		}
	} else {
		for (i = 0; i < ctrl->chip_cap.MaxRepeaterNum; i++) {
			pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
			/*always bind band 0*/
			rcFillEntry(pEntry, DBDC_TYPE_REPEATER, 0, i);
			pBInfo->TotalNum++;
		}
	}

	return 0;
}



static INT32 rcUpdateBandForWMM(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i, WmmNum = WcGetWmmNum(ctrl);
	EDCA_PARM *pEdca = NULL;
	BCTRL_ENTRY_T *pEntry;

	for (i = 0; i < WmmNum; i++) {
		pEdca = WcGetWmmByIdx(ctrl, i);

		if (!pEdca->bValid)
			continue;

		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_WMM, pEdca->BandIdx, i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForMGMT(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i;
	BCTRL_ENTRY_T *pEntry;

	for (i = 0; i < 2; i++) {
		pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
		rcFillEntry(pEntry, DBDC_TYPE_MGMT, i, i);
		pBInfo->TotalNum++;
	}

	return 0;
}


static INT32 rcUpdateBandForPTA(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	BCTRL_ENTRY_T *pEntry;
	RADIO_CTRL *pRadioCtrl;

	pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
	/*fix to bind band 0 for 2.4G band*/
	pRadioCtrl = rcGetRadioCtrlByRf(ctrl, RFIC_24GHZ);

	if (pRadioCtrl != NULL)
		rcFillEntry(pEntry, DBDC_TYPE_PTA, pRadioCtrl->BandIdx, 0);
	else
		rcFillEntry(pEntry, DBDC_TYPE_PTA, 0, 0);

	pBInfo->TotalNum++;
	return 0;
}

static INT32 rcUpdateBandForOwnMac(struct hdev_ctrl *ctrl, BCTRL_INFO_T *pBInfo)
{
	INT32 i, ret = 0;
	struct hdev_obj *obj;
	struct radio_dev *rdev;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	BCTRL_ENTRY_T *pEntry = NULL;

	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		rdev = &ctrl->rdev[i];
		DlListForEach(obj, &rdev->DevObjList, struct hdev_obj, list) {
			pEntry = &pBInfo->BctrlEntries[pBInfo->TotalNum];
			rcUpdateBandByType(obj, pEntry);
			pBInfo->TotalNum++;
		}
	}

	return ret;
}
#endif /*DBDC_MODE*/


/*Export functions*/
/*
*
*/
INT32 RcUpdateBandCtrl(struct hdev_ctrl *ctrl)
{
#ifdef DBDC_MODE
	INT32 ret = 0;
	BCTRL_INFO_T BctrlInfo;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	os_zero_mem(&BctrlInfo, sizeof(BCTRL_INFO_T));
	BctrlInfo.DBDCEnable = pAd->CommonCfg.dbdc_mode;

	/*if enable dbdc, run band selection algorithm*/
	if (IS_CAP_DBDC(ctrl->chip_cap) && BctrlInfo.DBDCEnable) {
		/*Since phyctrl  need to update */
		rcUpdateBandForOwnMac(ctrl, &BctrlInfo);
		rcUpdateBandForBFMU(ctrl, &BctrlInfo);
		rcUpdateBandForWMM(ctrl, &BctrlInfo);
		rcUpdateBandForMGMT(ctrl, &BctrlInfo);
		rcUpdateBandForPTA(ctrl, &BctrlInfo);
		rcUpdateBandForRepeater(ctrl, &BctrlInfo);
		/*Since will add one more time, must minus 1*/
		BctrlInfo.TotalNum = (BctrlInfo.TotalNum-1);

		if (BctrlInfo.TotalNum > MAX_BCTRL_ENTRY)
			BctrlInfo.TotalNum = MAX_BCTRL_ENTRY;
	}

	ret = AsicSetDbdcCtrl(pAd, &BctrlInfo);

	if (ret != NDIS_STATUS_SUCCESS)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n", __func__, ret));

#endif /*DBDC_MODE*/
	return 0;
}


/*
*
*/
INT32 RcUpdateRepeaterEntry(struct radio_dev *rdev, UINT32 ReptIdx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	struct hdev_ctrl *ctrl = rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (IS_CAP_DBDC(ctrl->chip_cap) && pAd->CommonCfg.dbdc_mode) {
		os_zero_mem(&BandInfoValue, sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/
		rcFillEntry(pEntry, DBDC_TYPE_REPEATER, rdev->pRadioCtrl->BandIdx, ReptIdx);
		BandInfoValue.TotalNum++;
		ret = AsicSetDbdcCtrl(pAd, &BandInfoValue);

		if (ret != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n", __func__, ret));
	}

#endif /*DBDC_MODE*/
	return ret;
}


/*
*
*/
INT32 RcUpdateWmmEntry(struct radio_dev *rdev, struct hdev_obj *obj, UINT32 WmmIdx)
{
	INT32 ret = 0;
#ifdef DBDC_MODE
	BCTRL_ENTRY_T *pEntry;
	BCTRL_INFO_T BandInfoValue;
	struct hdev_ctrl *ctrl = rdev->priv;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)ctrl->priv;

	if (obj && IS_CAP_DBDC(ctrl->chip_cap) && pAd->CommonCfg.dbdc_mode) {
		os_zero_mem(&BandInfoValue, sizeof(BCTRL_INFO_T));
		BandInfoValue.DBDCEnable = pAd->CommonCfg.dbdc_mode;
		pEntry = &BandInfoValue.BctrlEntries[0];
		/*fix to bind band 0 currently*/
		obj->WmmIdx = WmmIdx;
		rcFillEntry(pEntry, DBDC_TYPE_WMM, rdev->pRadioCtrl->BandIdx, WmmIdx);
		BandInfoValue.TotalNum++;
		ret = AsicSetDbdcCtrl(pAd, &BandInfoValue);

		if (ret != NDIS_STATUS_SUCCESS)
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Error for conifgure dbdc, ret = %d !\n", __func__, ret));
	}

#endif /*DBDC_MODE*/
	return ret;
}


/*
* Used for DATA path can get
*/
/*
*
*/
UINT32 RcGetMgmtQueueIdx(struct hdev_obj *obj, enum PACKET_TYPE pkt_type)
{
	struct radio_dev *rdev = obj->rdev;

	if (pkt_type == TX_ALTX) {
		if (rdev->pRadioCtrl && rdev->pRadioCtrl->BandIdx)
			return TxQ_IDX_ALTX1;

		return TxQ_IDX_ALTX0;
	} else {
		return dmac_wmm_swq_2_hw_ac_que[obj->WmmIdx][QID_AC_BE];
	}
}



/*
*
*/
UINT32 RcGetBcnQueueIdx(struct hdev_obj *obj)
{
#if defined(MT7615) || defined(MT7622) || defined(P18) || defined(MT7663)
	struct radio_dev *rdev = obj->rdev;

	if (rdev->pRadioCtrl->BandIdx)
		return TxQ_IDX_BCN1;

	return TxQ_IDX_BCN0;
#else
	return Q_IDX_BCN;
#endif
}


/*
*
*/
UINT32 RcGetTxRingIdx(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;

	if (rdev->pRadioCtrl && rdev->pRadioCtrl->BandIdx)
		return  1;

	return 0;
}



/*
*
*/
UINT32 RcGetWmmIdx(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;

	if (rdev->pRadioCtrl)
		return obj->WmmIdx;

	return 0;
}
UINT32 MAPRcGetBandIdxByChannelCheck(struct hdev_ctrl *ctrl, UCHAR Channel)
{
#ifdef DBDC_MODE
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) ctrl->priv;

	/*not enable dbdc mode band should always in band0*/
	if (!pAd->CommonCfg.dbdc_mode)
		return 0;

	/*enable dbdc mode, chose bandIdx from channel*/
	if (Channel > 14)
		return BAND1;
	else
		return BAND0;
#endif /*DBDC_MODE*/
	return 0;
}

/*
*
*/
UINT32 RcGetBandIdxByChannel(struct hdev_ctrl *ctrl, UCHAR Channel)
{
#ifdef DBDC_MODE
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *) ctrl->priv;
	RADIO_CTRL *pRadioCtrl = NULL;

	/*not enable dbdc mode band should always in band0*/
	if (!pAd->CommonCfg.dbdc_mode)
		return 0;

	/*enable dbdc mode, chose bandIdx from channel*/
	if (Channel > 14)
		pRadioCtrl = rcGetRadioCtrlByRf(ctrl, RFIC_5GHZ);
	else
		pRadioCtrl = rcGetRadioCtrlByRf(ctrl, RFIC_24GHZ);

	if (pRadioCtrl)
		return pRadioCtrl->BandIdx;

#endif /*DBDC_MODE*/
	return 0;
}


/*
*
*/
VOID RcRadioInit(struct hdev_ctrl *ctrl, UCHAR RfIC, UCHAR DbdcMode)
{
	RADIO_CTRL *pRadioCtrl = NULL;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	HD_RESOURCE_CFG *pHwResource = &ctrl->HwResourceCfg;
	UCHAR i;

	if (IS_CAP_DBDC(ctrl->chip_cap) && DbdcMode)
		pHwResource->concurrent_bands = 2;
	else
		pHwResource->concurrent_bands = 1;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DbdcMode=%d, ConcurrentBand=%d\n",
			 __func__, DbdcMode, pHwResource->concurrent_bands));

	/*Allocate PhyCtrl for HwResource*/
	for (i = 0; i < pHwResource->concurrent_bands; i++) {
		pPhyCtrl =  &pHwResource->PhyCtrl[i];
		pRadioCtrl =  &pPhyCtrl->RadioCtrl;
		os_zero_mem(pRadioCtrl, sizeof(*pRadioCtrl));
		pPhyCtrl->rf_band_cap = rcGetBandSupport(ctrl, DbdcMode, i);
		pRadioCtrl->BandIdx = i;
		pRadioCtrl->ExtCha = EXTCHA_NOASSIGN;

		if ((pPhyCtrl->rf_band_cap) & RFIC_24GHZ)
			pRadioCtrl->Channel = rcGetDefaultChannel(WMODE_B);
		else
			pRadioCtrl->Channel = rcGetDefaultChannel(WMODE_A);

		pRadioCtrl->PhyMode = rcGetDefaultPhyMode(pRadioCtrl->Channel);
		pRadioCtrl->CurStat = PHY_IDLE;
		/*if only one band, band to band 0*/
#ifdef TXBF_SUPPORT

		if (pHwResource->concurrent_bands == 1)
			pRadioCtrl->IsBfBand = 1;

#endif /*TXBF_SUPPORT*/
#ifdef GREENAP_SUPPORT
		pRadioCtrl->bGreenAPActive = FALSE;
#endif /* GREENAP_SUPPORT */
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): pRadioCtrl=%p,Band=%d,rfcap=%d,channel=%d,PhyMode=%d extCha=0x%x\n",
				 __func__, pRadioCtrl, i, pPhyCtrl->rf_band_cap, pRadioCtrl->Channel, pRadioCtrl->PhyMode,pRadioCtrl->ExtCha));
		HdevInit(ctrl, i, pRadioCtrl);
	}

	RcUpdateBandCtrl(ctrl);
}


/*
*
*/
VOID RcReleaseBandForObj(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	struct radio_dev *rdev = NULL;

	if (!obj) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():can't find obj\n", __func__));
		return;
	}

	ReleaseOmacIdx(ctrl, obj->Type, obj->OmacIdx);
	rdev = obj->rdev;

	if (rdev) {
		if (obj->bWmmAcquired)
			WcReleaseEdca(obj);

		HdevObjDel(rdev, obj);
		NdisFreeSpinLock(&obj->RefCntLock);
	}

	return;
}


/*
* Refine when OmacIdx is ready
*/
struct radio_dev *RcAcquiredBandForObj(
	struct hdev_ctrl *ctrl,
	struct hdev_obj *obj,
	UCHAR obj_idx,
	UCHAR PhyMode,
	UCHAR Channel,
	UCHAR ObjType)
{
	struct radio_dev *rdev = NULL;
	UCHAR is_default = 0;
	RADIO_CTRL *pRadioCtrl = NULL;

	/*Release first*/
	if (obj->state == HOBJ_STATE_USED)
		RcReleaseBandForObj(ctrl, obj);

	rdev = RcGetHdevByPhyMode(ctrl, PhyMode, Channel);

	/*can't get hdev by phymode, use default band*/
	if (!rdev) {
		rdev = &ctrl->rdev[0];
		if (WMODE_CAP_5G(PhyMode)) {
			printk("[%s] rdev received NULL in 5G mode\n", __func__);
		}
		is_default = 1;
	}

	/*update phy mode for radio control*/
	pRadioCtrl = rdev->pRadioCtrl;
	/*Can get rdev. change phyCtrl to INUSED state*/
	if(pRadioCtrl->CurStat == PHY_IDLE)
		pRadioCtrl->CurStat = PHY_INUSE;
	/*if mixed mode*/
	if ((ObjType == WDEV_TYPE_STA) && (!WMODE_5G_ONLY(PhyMode) || !WMODE_2G_ONLY(PhyMode))) {
		pRadioCtrl->PhyMode = PhyMode;
	} else if (!is_default) {
		/*Make phymode of band should be the maxize*/
		if (wmode_band_equal(pRadioCtrl->PhyMode, PhyMode))
			pRadioCtrl->PhyMode |=  PhyMode;
		else
			pRadioCtrl->PhyMode = PhyMode;
	}
	/*update hdev_obj information*/
	obj->Idx = obj_idx;
	obj->Type = ObjType;
	obj->OmacIdx = GetOmacIdx(ctrl, ObjType, obj_idx);
	HdevObjAdd(rdev, obj);
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): BandIdx:%d, PhyMode=%d,Channel=%d,rdev=%p,pHdevObj=%p\n",
		__func__, pRadioCtrl->BandIdx, pRadioCtrl->PhyMode, pRadioCtrl->Channel, rdev, obj));
	RcUpdateBandCtrl(ctrl);
	NdisAllocateSpinLock(NULL, &obj->RefCntLock);
	return rdev;
}



/*
*
*/
struct radio_dev *RcGetHdevByChannel(struct hdev_ctrl *ctrl, UCHAR Channel)
{
	struct radio_dev *rdev = NULL;
	UCHAR i = 0;

	for (i = 0 ; i < ctrl->HwResourceCfg.concurrent_bands ; i++) {
		rdev = &ctrl->rdev[i];
		if (rdev != NULL && rdev->pRadioCtrl->Channel == Channel) {
			return rdev;
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s():Err! Update PhyMode failed, no phyctrl support this channel=%d!\n",
		__func__, Channel));
	return NULL;
}


/*
* Get rdev by PhyMode & Channel, can't find pHdev may rdev is full
*/
struct radio_dev *RcGetHdevByPhyMode(struct hdev_ctrl *ctrl, UCHAR PhyMode, UCHAR channel)
{
	UCHAR i;
	struct radio_dev *rdev = NULL;
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	CHAR *str;
	CHANNEL_CTRL *pChCtrl = NULL;

	/*check == first*/
	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];

		if (WMODE_CAP_2G(PhyMode) &&  (pPhyCtrl->rf_band_cap == RFIC_24GHZ))
			rdev = &ctrl->rdev[i];
		else if (WMODE_CAP_5G(PhyMode) && (pPhyCtrl->rf_band_cap == RFIC_5GHZ))
			rdev = &ctrl->rdev[i];

		if (rdev) {
			pChCtrl = hc_get_channel_ctrl(ctrl, rdev->pRadioCtrl->BandIdx);
			/* if ACS Enabled channel is 0 initially */
			if (channel) {
				if ((rdev->DevNum == 0) || (rdev->pRadioCtrl->Channel == channel) ||
						MTChGrpChannelChk(pChCtrl, channel))
					break;
			} else {
				printk("[%s] channel 0 fix for rdev fetching\n", __func__);
				break;
			}
		}
		rdev = NULL;
	}

	if (rdev)
		return rdev;

	/*check & case*/
	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];

		if (WMODE_CAP_2G(PhyMode) &&  (pPhyCtrl->rf_band_cap & RFIC_24GHZ))
			rdev = &ctrl->rdev[i];
		else if (WMODE_CAP_5G(PhyMode) && (pPhyCtrl->rf_band_cap & RFIC_5GHZ))
			rdev = &ctrl->rdev[i];
		if (channel) {
			if (rdev &&
				((rdev->DevNum == 0) || (rdev->pRadioCtrl->Channel == channel))) {
				break;
			}
		} else {
			if (rdev) {
				printk("[%s]-- channel 0 fix for rdev fetching\n",  __func__);
				break;
			}

		}

	}

	if (!rdev) {
		str = wmode_2_str(PhyMode);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s():Err! chip not support this PhyMode:%s !\n", __func__, str));
		if (str)
			os_free_mem(str);
	}

	return rdev;
}




/*
*
*/
INT32 RcUpdateChannel(struct radio_dev *rdev, UCHAR Channel, BOOLEAN scan)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
#ifdef TR181_SUPPORT
	if ((pRadioCtrl->Channel != Channel) && (scan == 0)) {
		ULONG CurJiffies;

		NdisGetSystemUpTime(&CurJiffies);
		pRadioCtrl->CurChannelUpTime = jiffies_to_usecs(CurJiffies);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:orig_chan=%d, new_chan=%d, CurChanUpTime=%u\n",
						__func__, pRadioCtrl->Channel, Channel, pRadioCtrl->CurChannelUpTime));
	}
#endif
	pRadioCtrl->Channel = Channel;
	pRadioCtrl->scan_state = scan;
	return ret;
}



/*
*
*/
INT32 RcUpdateRadio(struct radio_dev *rdev, UCHAR bw, UCHAR central_ch1, UCHAR control_ch2, UCHAR ext_cha)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	pRadioCtrl->CentralCh = central_ch1;
	pRadioCtrl->Bw = bw;
	pRadioCtrl->Channel2 = control_ch2;
	pRadioCtrl->ExtCha = ext_cha;
	return ret;
}


/*
*
*/
INT32 RcUpdatePhyMode(struct radio_dev *rdev, UCHAR PhyMode)
{
	INT32 ret = 0;
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;
	/*band is not changed or not*/
	if (rcCheckIsTheSameBand(PhyMode, pRadioCtrl->Channel)) {
		pRadioCtrl->PhyMode |= PhyMode;
		return ret;
	}

	/*band is changed*/
	pRadioCtrl->PhyMode = PhyMode;
	pRadioCtrl->Channel = rcGetDefaultChannel(PhyMode);
	RcUpdateBandCtrl(rdev->priv);
	return -1;
}


/*
*
*/
UCHAR RcUpdateBw(struct radio_dev *rdev, UCHAR Bw)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	/*Legacy mode, only can support BW20*/
	if (!WMODE_CAP_N(pRadioCtrl->PhyMode) && Bw > BW_20)
		pRadioCtrl->Bw = BW_20;
	else if (!WMODE_CAP_AC(pRadioCtrl->PhyMode) && Bw > BW_40)
		pRadioCtrl->Bw = BW_40;
	else
		pRadioCtrl->Bw = Bw;

	return Bw;
}

/*
*
*/
INT32 RcUpdateExtCha(struct radio_dev *rdev, UCHAR ExtCha)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	pRadioCtrl->ExtCha = ExtCha;
	return -1;
}

/*
*
*/
UCHAR RcGetExtCha(struct radio_dev *rdev)
{
	RADIO_CTRL *pRadioCtrl = rdev->pRadioCtrl;

	return pRadioCtrl->ExtCha;
}

/*
*
*/
UCHAR RcGetPhyMode(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->PhyMode;
}


/*
*
*/
UCHAR RcGetChannel(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->Channel;
}


/*
*
*/
UCHAR RcGetCentralCh(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->CentralCh;
}


/*
*
*/
UCHAR RcGetBandIdx(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->BandIdx;
}

/*
*
*/
PHY_STATUS RcGetRadioCurStat(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->CurStat;
}

/*
*
*/
VOID RcSetRadioCurStat(struct radio_dev *rdev, PHY_STATUS CurStat)
{
	rdev->pRadioCtrl->CurStat = CurStat;
}


/*
*
*/
UCHAR RcGetBw(struct radio_dev *rdev)
{
	return rdev->pRadioCtrl->Bw;
}


/*
*
*/
UCHAR RcGetBandIdxByRf(struct hdev_ctrl *ctrl, UCHAR RfIC)
{
	struct radio_dev *rdev = rcGetHdevByRf(ctrl, RfIC);

	if (rdev)
		return RcGetBandIdx(rdev);

	return 0;
}


/*
*
*/
struct radio_dev *RcGetBandIdxByBf(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RADIO_CTRL *pRadioCtrl = NULL;
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		pRadioCtrl = &pHwResourceCfg->PhyCtrl[i].RadioCtrl;

		if (pRadioCtrl->IsBfBand)
			return &ctrl->rdev[i];
	}

	return NULL;
}


/*
*
*/
struct radio_dev *RcInit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pHwResourceCfg = &ctrl->HwResourceCfg;
	RTMP_PHY_CTRL *pPhyCtrl = NULL;
	UCHAR i;

	for (i = 0; i < DBDC_BAND_NUM; i++) {
		pPhyCtrl = &pHwResourceCfg->PhyCtrl[i];
		os_zero_mem(pPhyCtrl, sizeof(RTMP_PHY_CTRL));
	}

	return NULL;
}


/*
*
*/
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg)
{
	UCHAR i;

	for (i = 0; i < pHwResourceCfg->concurrent_bands; i++) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("band\t: %d,rfic: %d, bf_cap: %d\n",
				 i, pHwResourceCfg->PhyCtrl[i].rf_band_cap, pHwResourceCfg->PhyCtrl[i].RadioCtrl.IsBfBand ? TRUE:FALSE));
	}
}

/*
*
*/
BOOLEAN RcIsBfCapSupport(struct hdev_obj *obj)
{
	struct radio_dev *rdev = obj->rdev;
	RADIO_CTRL *rc;

	if (!rdev)
		return FALSE;

	rc = rdev->pRadioCtrl;
	return rc->IsBfBand;
}

/*
*
*/
BOOLEAN rc_radio_equal(struct radio_dev *dev, struct freq_oper *oper)
{
	RADIO_CTRL *rc = dev->pRadioCtrl;

	/*if previous action is for scan, always allow to switch channel*/
	if (rc->scan_state == TRUE)
		return FALSE;

	if (rc->Channel != oper->prim_ch)
		return FALSE;

	if (rc->Bw != oper->bw)
		return FALSE;

	if (rc->CentralCh != oper->cen_ch_1)
		return FALSE;

	if (rc->Channel2 != oper->cen_ch_2)
		return FALSE;

	return TRUE;
}

/*
*
*/
BOOLEAN rc_radio_res_acquire(struct radio_dev *dev, struct radio_res *res)
{
	RADIO_CTRL *rc = dev->pRadioCtrl;

	if (rc->CurStat != PHY_INUSE)
		return FALSE;

	return TRUE;
}
