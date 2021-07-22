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

/*
* local function
*/

/*
*
*/
static UCHAR wtc_acquire_groupkey_wcid(struct hdev_ctrl *ctrl, WTBL_CFG *pWtblCfg, struct hdev_obj *obj)
{
	UCHAR AvailableWcid = INVAILD_WCID;
	UCHAR OmacIdx, WdevType;
	int i;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR min_wcid = pWtblCfg->MinMcastWcid;
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;

	OmacIdx = obj->OmacIdx;
	WdevType = obj->Type;
	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	for (i = (cap->WtblHwNum - 1); i >= min_wcid; i--) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
			continue;
		else {
			pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
			pWtblIdxRec->WtblIdx = i;
			/*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
			pWtblIdxRec->LinkToOmacIdx = OmacIdx;
			pWtblIdxRec->LinkToWdevType = WdevType;
			pWtblIdxRec->type = WTBL_TYPE_MCAST;
			AvailableWcid = (UCHAR)i;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: Found a non-occupied wtbl_idx:%d for WDEV_TYPE:%d\n"
					  " LinkToOmacIdx = %x, LinkToWdevType = %d\n",
					  __func__, i, WdevType, OmacIdx, WdevType));
			NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
			return AvailableWcid;
		}
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);

	if (i < min_wcid) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: no available wtbl_idx for WDEV_TYPE:%d\n",
				  __func__, WdevType));
	}

	return AvailableWcid;
}


/*Wtable control*/
/*
*
*/
VOID WtcInit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblParm = NULL;
	UCHAR i = 0;

	os_zero_mem(pWtblCfg, sizeof(WTBL_CFG));
	NdisAllocateSpinLock(NULL, &pWtblCfg->WtblIdxRecLock);

	for (i = 0; i < MAX_LEN_OF_TR_TABLE; i++) {
		pWtblParm = &pWtblCfg->WtblIdxRec[i];
		pWtblParm->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblParm->type = WTBL_TYPE_NONE;
	}
}


/*
*
*/
VOID WtcExit(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;

	NdisFreeSpinLock(&pWtblCfg->WtblIdxRecLock);
	os_zero_mem(pWtblCfg, sizeof(WTBL_CFG));
}


/*
*
*/
UCHAR WtcSetMaxStaNum(struct hdev_ctrl *ctrl, UCHAR BssidNum, UCHAR MSTANum)
{
	UCHAR wtbl_num_resv_for_mcast = 0;
	UCHAR wtbl_num_use_for_ucast = 0;
	UCHAR wtbl_num_use_for_sta = 0;
	UCHAR MaxNumChipRept = 0;
	UCHAR ApcliNum = MAX_APCLI_NUM;
	UCHAR WdsNum = MAX_WDS_ENTRY;
	UCHAR MaxUcastEntryNum = 0;
	struct _RTMP_CHIP_CAP *cap = &ctrl->chip_cap;
#ifdef CONFIG_AP_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	MaxNumChipRept = GET_MAX_REPEATER_ENTRY_NUM(cap);
#endif /*MAC_REPEATER_SUPPROT*/
#endif /*CONFIG_AP_SUPPORT*/
	wtbl_num_resv_for_mcast = BssidNum + ApcliNum + MSTANum;
	wtbl_num_use_for_ucast = WdsNum + MaxNumChipRept + ApcliNum + MSTANum;
	wtbl_num_use_for_sta = cap->WtblHwNum -
						   wtbl_num_resv_for_mcast -
						   wtbl_num_use_for_ucast;
	MaxUcastEntryNum = wtbl_num_use_for_sta + wtbl_num_use_for_ucast;
	ctrl->HwResourceCfg.WtblCfg.MaxUcastEntryNum = MaxUcastEntryNum;
	ctrl->HwResourceCfg.WtblCfg.MinMcastWcid = cap->WtblHwNum - wtbl_num_resv_for_mcast;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s: BssidNum:%d, MaxStaNum:%d (WdsNum:%d, ApcliNum:%d, MaxNumChipRept:%d), MinMcastWcid:%d\n",
			  __func__,
			  BssidNum,
			  (MaxUcastEntryNum - 1), /* WTBL 0 for management use*/
			  WdsNum,
			  ApcliNum,
			  MaxNumChipRept,
			  ctrl->HwResourceCfg.WtblCfg.MinMcastWcid));
	return MaxUcastEntryNum;
}

/*
*
*/
UCHAR WtcAcquireGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	UCHAR wcid;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	wcid = wtc_acquire_groupkey_wcid(ctrl, pWtblCfg, obj);
	return wcid;
}

/*
*
*/
UCHAR WtcReleaseGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR ReleaseWcid = INVAILD_WCID;

	if (idx >= MAX_LEN_OF_MAC_TABLE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: idx:%d > MAX_LEN_OF_MAC_TABLE\n", __func__, idx));
		return idx;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

	if (pWtblIdxRec->type != WTBL_TYPE_MCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return idx;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: try to release non-occupied idx:%d, something wrong?\n",
				  __func__, idx));
		ReleaseWcid = idx;
	} else {
		os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
		/*make sure entry is cleared to usable one.*/
		pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblIdxRec->type = WTBL_TYPE_NONE;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return ReleaseWcid;
}


/*
*
*/
UCHAR WtcGetWcidLinkType(struct hdev_ctrl *ctrl, UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

	return pWtblIdxRec->LinkToWdevType;
}



/*
*
*/
UCHAR WtcGetMaxStaNum(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg =  &pResource->WtblCfg;

	if (pWtblCfg->MaxUcastEntryNum > MAX_LEN_OF_MAC_TABLE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: MaxUcastEntryNum=%d >= MAX_LEN_OF_MAC_TABLE(%d)\n",
				  __func__, pWtblCfg->MaxUcastEntryNum, MAX_LEN_OF_MAC_TABLE));
		return MAX_LEN_OF_MAC_TABLE;
	} else
		return pWtblCfg->MaxUcastEntryNum;
}



/*
*
*/
UCHAR WtcAcquireUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj)
{
	UCHAR FirstWcid = 1;
	UCHAR i;
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;

	if (obj == NULL || pResource == NULL || pWtblCfg == NULL) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: unexpected NULL please check!!\n", __func__));
		return INVAILD_WCID;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);

	/* skip entry#0 so that "entry index == AID" for fast lookup*/
	for (i = FirstWcid; i < pWtblCfg->MaxUcastEntryNum; i++) {
		/* sanity check to avoid out of bound with pAd->MacTab.Content */
		if (i >= MAX_LEN_OF_MAC_TABLE)
			continue;

		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec == NULL) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					 ("%s: unexpected NULL please check!!\n", __func__));
			return INVAILD_WCID;
		}

		if (pWtblIdxRec->State != WTBL_STATE_NONE_OCCUPIED)
			continue;

		pWtblIdxRec->State = WTBL_STATE_SW_OCCUPIED;
		pWtblIdxRec->WtblIdx = i;
		/*TODO: Carter, check flow when calling this function, OmacIdx might be erroed.*/
		pWtblIdxRec->LinkToOmacIdx = obj->OmacIdx;
		pWtblIdxRec->LinkToWdevType = obj->Type;
		pWtblIdxRec->type = WTBL_TYPE_UCAST;
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return i;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return INVAILD_WCID;
}


/*
*
*/
UCHAR WtcReleaseUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UCHAR idx)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR ReleaseWcid = INVAILD_WCID;

	if (idx >= MAX_LEN_OF_MAC_TABLE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: idx:%d > MAX_LEN_OF_MAC_TABLE\n", __func__, idx));
		return idx;
	}

	NdisAcquireSpinLock(&pWtblCfg->WtblIdxRecLock);
	pWtblIdxRec = &pWtblCfg->WtblIdxRec[idx];

	if (pWtblIdxRec->type != WTBL_TYPE_UCAST) {
		NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
		return idx;
	}

	if (pWtblIdxRec->State == WTBL_STATE_NONE_OCCUPIED) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s: try to release non-occupied idx:%d, something wrong?\n",
				  __func__, idx));
		ReleaseWcid = idx;
	} else {
		os_zero_mem(pWtblIdxRec, sizeof(WTBL_IDX_PARAMETER));
		/*make sure entry is cleared to usable one.*/
		pWtblIdxRec->State = WTBL_STATE_NONE_OCCUPIED;
		pWtblIdxRec->type = WTBL_TYPE_NONE;
	}

	NdisReleaseSpinLock(&pWtblCfg->WtblIdxRecLock);
	return ReleaseWcid;
}

/*
*
*/
VOID WtcRecDump(struct hdev_ctrl *ctrl)
{
	HD_RESOURCE_CFG *pResource = &ctrl->HwResourceCfg;
	WTBL_CFG *pWtblCfg = &pResource->WtblCfg;
	WTBL_IDX_PARAMETER *pWtblIdxRec = NULL;
	UCHAR i;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("\tWtblRecDump, Max Ucast is %d\n", pWtblCfg->MaxUcastEntryNum));

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		pWtblIdxRec = &pWtblCfg->WtblIdxRec[i];

		if (pWtblIdxRec->State) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tIdx:%d,State:%d,Omac:%d,Type:%d,Wcid:%d, WcidType:%d\n",
				i, pWtblIdxRec->State, pWtblIdxRec->LinkToOmacIdx,
				pWtblIdxRec->LinkToWdevType, pWtblIdxRec->WtblIdx, pWtblIdxRec->type));
		}
	}
}

