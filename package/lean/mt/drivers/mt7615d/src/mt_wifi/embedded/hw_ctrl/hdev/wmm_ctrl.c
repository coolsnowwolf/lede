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

/*WMM control*/


/*Local functions*/
static  BOOLEAN wcCompareEdca(EDCA_PARM *pEdcaParm, EDCA_PARM *pEdca)
{
	if ((!pEdcaParm->bValid) || (!pEdca->bValid))
		return FALSE;

	if ((pEdca->BandIdx != pEdcaParm->BandIdx))
		return FALSE;

	if (os_cmp_mem(pEdca->Aifsn, pEdcaParm->Aifsn, 4))
		return FALSE;

	if (os_cmp_mem(pEdca->Txop, pEdcaParm->Txop, sizeof(USHORT)*4))
		return FALSE;

	if (os_cmp_mem(pEdca->Cwmax, pEdcaParm->Cwmax, 4))
		return FALSE;

	if (os_cmp_mem(pEdca->Cwmin, pEdcaParm->Cwmin, 4))
		return FALSE;

	return TRUE;
}



/*Export Functions*/


/*
  *
 */
UINT32 WcGetWmmNum(struct hdev_ctrl *ctrl)
{
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	return cap->WmmHwNum;
}


/*
 *
*/
EDCA_PARM *WcGetWmmByIdx(struct hdev_ctrl *ctrl, UINT32 Idx)
{
	return &ctrl->HwResourceCfg.WmmCtrl.pWmmSet[Idx];
}


/*
 *
*/
VOID WcReleaseEdca(struct hdev_obj *obj)
{
	EDCA_PARM *pEdca;
	struct radio_dev *rdev = NULL;
	struct hdev_ctrl *ctrl = NULL;
	UINT32 WmmIdx;
	/* TODO: Star, should remove it. */
	RTMP_ADAPTER *pAd;

	if (!obj || !obj->bWmmAcquired) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Can't find HdevObj or Edca not required\n", __func__));
		return;
	}

	rdev = obj->rdev;
	ctrl = rdev->priv;
	pAd = (RTMP_ADAPTER *) ctrl->priv;
	pEdca = WcGetWmmByIdx(ctrl, obj->WmmIdx);

	if (!pEdca || !pEdca->bValid) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Can't find Edca for rdev: %d, Obj: %d\n",
				 __func__, rdev->Idx, obj->Idx));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(): ObjIdx=%d,WmmIdx=%d\n", __func__, obj->Idx, obj->WmmIdx));
	WmmIdx = pEdca->WmmSet;
	pEdca->RefCnt--;

	if (pEdca->RefCnt <= 0) {
		os_zero_mem(pEdca, sizeof(EDCA_PARM));
		pEdca->bValid = FALSE;
		pEdca->RefCnt = 0;
		pEdca->WmmSet = WmmIdx;
		AsicSetEdcaParm(pAd, pEdca, pAd->wdev_list[obj->Idx]);
	}

	obj->WmmIdx = 0;
	obj->bWmmAcquired = FALSE;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Release a WMM for ObjIdx: %d\n", obj->Idx));
}


/*
 *
*/
VOID  WcAcquiredEdca(struct hdev_obj *obj, EDCA_PARM *pEdcaParm)
{
	struct radio_dev *rdev;
	struct hdev_ctrl *ctrl;
	INT32 i;
	UINT32 NumOfWmm;
	EDCA_PARM *pEdca, ReleaseEdca;
	/* TODO: Star, should remove it. */

	if (!obj) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): Can't find HdevObj\n", __func__));
		return;
	}

	rdev = obj->rdev;
	ctrl = rdev->priv;
	NumOfWmm = WcGetWmmNum(ctrl);
	os_zero_mem(&ReleaseEdca, sizeof(EDCA_PARM));

	/*if input edca is all zero, assign default APEdca parameter*/
	if (!os_cmp_mem(&ReleaseEdca, pEdcaParm, sizeof(EDCA_PARM)) || pEdcaParm->bValid != TRUE)
		set_default_sta_edca_param(pEdcaParm);

	pEdcaParm->BandIdx = RcGetBandIdx(rdev);

	/*if can't search and WmmAcquired is not found*/
	if (obj->bWmmAcquired) {
		pEdca = WcGetWmmByIdx(ctrl, obj->WmmIdx);

		if (wcCompareEdca(pEdca, pEdcaParm)) {
			/*do nothing*/
			return;
		}

		/*Reacquire wmm*/
		WcReleaseEdca(obj);
	}

	for (i = 0; i < NumOfWmm; i++) {
		pEdca = WcGetWmmByIdx(ctrl, i);

		if (wcCompareEdca(pEdca, pEdcaParm)) {
			pEdca->RefCnt++;
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("WMM already created, assign  WmmIdx:%d to  ObjIdx: %d\n", i, obj->Idx));
			return;
		}
	}

	/*Bind a new WMM for band*/
	for (i = 0; i < NumOfWmm; i++) {
		pEdca = WcGetWmmByIdx(ctrl, i);

		if (!pEdca->bValid) {
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			os_move_mem(pEdca->Aifsn, pEdcaParm->Aifsn, 4);
			os_move_mem(pEdca->Cwmax, pEdcaParm->Cwmax, 4);
			os_move_mem(pEdca->Cwmin, pEdcaParm->Cwmin, 4);
			os_move_mem(pEdca->Txop, pEdcaParm->Txop, sizeof(USHORT)*4);
			pEdca->RefCnt = 1;
			pEdca->bValid = TRUE;
			pEdca->BandIdx = RcGetBandIdx(rdev);
			pEdcaParm->WmmSet = i;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("Create a new WmmIdx:%d to ObjIdx: %d\n", i, obj->Idx));
			return;
		}
	}

	/*allocate a default wmm set when obj can't allocate a new once*/
	for (i = 0; i < NumOfWmm; i++) {
		pEdca = WcGetWmmByIdx(ctrl, i);

		if (pEdca->bValid && pEdca->BandIdx == RcGetBandIdx(rdev)) {
			obj->WmmIdx = i;
			obj->bWmmAcquired = TRUE;
			pEdca->RefCnt++;
			pEdcaParm->WmmSet = i;
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("assign a old WmmIdx:%d to ObjIdx: %d, but not apply new parameter\n", i, obj->Idx));
			return;
		}
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("Allocate WmmSet to ObjIdx:%d  fail since Wmm is full and no WmmSet can match band\n", obj->Idx));
}


/*
*
*/
VOID WcSetEdca(struct hdev_obj *obj)
{
	struct radio_dev *rdev;
	struct hdev_ctrl	*ctrl;
	EDCA_PARM *pEdca;
	RTMP_ADAPTER *pAd;

	rdev = obj->rdev;
	ctrl = (struct hdev_ctrl *)rdev->priv;
	pAd = (RTMP_ADAPTER *) ctrl->priv;

	if (obj->bWmmAcquired) {
		pEdca = WcGetWmmByIdx(ctrl, obj->WmmIdx);
		/*set EDCA parameters from AP*/
		AsicSetEdcaParm(pAd, pEdca, pAd->wdev_list[obj->Idx]);
		/*Update band control */
		RcUpdateWmmEntry(rdev, obj, obj->WmmIdx);
	}
}


/*
 *
*/
INT32 WcInit(struct hdev_ctrl *ctrl, WMM_CTRL_T *pWmmCtrl)
{
	INT32 NumOfWmm = WcGetWmmNum(ctrl);
	EDCA_PARM *pEdcaArray = NULL;
	INT32 i = 0;

	os_alloc_mem(NULL, (UCHAR **)&pEdcaArray, sizeof(EDCA_PARM)*NumOfWmm);

	if (pEdcaArray == NULL)
		return -1;

	os_zero_mem(pEdcaArray, sizeof(EDCA_PARM)*NumOfWmm);
	pWmmCtrl->pWmmSet = pEdcaArray;

	for (i = 0; i < NumOfWmm; i++) {
		pEdcaArray[i].bValid = FALSE;
		pEdcaArray[i].WmmSet = i;
		pEdcaArray[i].RefCnt = 0;
	}

	return 0;
}


/*
 *
*/
INT32 WcExit(WMM_CTRL_T *pWmmCtrl)
{
	if (pWmmCtrl->pWmmSet) {
		os_free_mem(pWmmCtrl->pWmmSet);
		pWmmCtrl->pWmmSet = NULL;
	}

	return 0;
}



/*
 *
*/
VOID WcShowEdca(struct hdev_ctrl *ctrl)
{
	INT i;
	EDCA_PARM *pEdca = NULL;
	INT32 NumOfWmm = WcGetWmmNum(ctrl);

	for (i = 0; i < NumOfWmm; i++) {
		pEdca = WcGetWmmByIdx(ctrl, i);

		if (pEdca->bValid) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tEdcaIdx: %d,BandIdx: %d, RfCnt: %d\n", pEdca->WmmSet, pEdca->BandIdx, pEdca->RefCnt));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tAifs: %d/%d/%d/%d\n",
					 pEdca->Aifsn[0], pEdca->Aifsn[1], pEdca->Aifsn[2], pEdca->Aifsn[3]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTxop: %d/%d/%d/%d\n",
					 pEdca->Txop[0], pEdca->Txop[1], pEdca->Txop[2], pEdca->Txop[3]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCwmin: %d/%d/%d/%d\n",
					 pEdca->Cwmin[0], pEdca->Cwmin[1], pEdca->Cwmin[2], pEdca->Cwmin[3]));
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCwmax: %d/%d/%d/%d\n",
					 pEdca->Cwmax[0], pEdca->Cwmax[1], pEdca->Cwmax[2], pEdca->Cwmax[3]));
		}
	}
}

