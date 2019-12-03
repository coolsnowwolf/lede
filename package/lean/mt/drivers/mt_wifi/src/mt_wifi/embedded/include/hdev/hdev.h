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

#ifndef __HDEV_H
#define __HDEV_H

#include "hdev/hdev_basic.h"

/*Radio Control*/
VOID RcRadioInit(struct hdev_ctrl *ctrl, UCHAR RfIC, UCHAR DbdcMode);
struct radio_dev *RcInit(struct hdev_ctrl *ctrl);
VOID RcRadioShow(HD_RESOURCE_CFG *pHwResourceCfg);

struct radio_dev *RcAcquiredBandForObj(
	struct hdev_ctrl *ctrl,
	struct hdev_obj *obj,
	UCHAR ObjIdx,
	UCHAR PhyMode,
	UCHAR Channel,
	UCHAR ObjType);

VOID RcReleaseBandForObj(struct hdev_ctrl *ctrl, struct hdev_obj *obj);

INT32 RcUpdateChannel(struct radio_dev *rdev, UCHAR Channel, BOOLEAN scan_state);
INT32 RcUpdatePhyMode(struct radio_dev *rdev, UCHAR Wmode);
struct radio_dev *RcGetHdevByChannel(struct hdev_ctrl *ctrl, UCHAR Channel);
struct radio_dev *RcGetHdevByPhyMode(struct hdev_ctrl *ctrl, UCHAR PhyMode, UCHAR channel);
UCHAR RcGetBandIdxByRf(struct hdev_ctrl *ctrl, UCHAR RfIC);


INT32 RcUpdateBandCtrl(struct hdev_ctrl *ctrl);
INT32 RcUpdateWmmEntry(struct radio_dev *rdev, struct hdev_obj *obj, UINT32 WmmIdx);
INT32 RcUpdateRepeaterEntry(struct radio_dev *rdev, UINT32 ReptIdx);
UCHAR RcUpdateBw(struct radio_dev *rdev, UCHAR Bw);
INT32 RcUpdateRadio(struct radio_dev *rdev, UCHAR bw, UCHAR central_ch1, UCHAR control_ch2, UCHAR ext_cha);
INT32 RcUpdateExtCha(struct radio_dev *rdev, UCHAR ExtCha);
UCHAR RcGetExtCha(struct radio_dev *rdev);
UINT32 RcGetMgmtQueueIdx(struct hdev_obj *obj, enum PACKET_TYPE pkt_type);
UINT32 RcGetBcnQueueIdx(struct hdev_obj *obj);
UINT32 RcGetTxRingIdx(struct hdev_obj *obj);
UINT32 RcGetWmmIdx(struct hdev_obj *obj);
UINT32 RcGetBandIdxByChannel(struct hdev_ctrl *ctrl, UCHAR Channel);
UCHAR RcGetPhyMode(struct radio_dev *rdev);
UCHAR RcGetChannel(struct radio_dev *rdev);
UCHAR RcGetCentralCh(struct radio_dev *rdev);
UCHAR RcGetBandIdx(struct radio_dev *rdev);
PHY_STATUS RcGetRadioCurStat(struct radio_dev *rdev);
VOID RcSetRadioCurStat(struct radio_dev *rdev, PHY_STATUS CurStat);
UCHAR RcGetBw(struct radio_dev *rdev);
struct radio_dev *RcGetBandIdxByBf(struct hdev_ctrl *ctrl);
BOOLEAN RcIsBfCapSupport(struct hdev_obj *obj);
BOOLEAN rc_radio_equal(struct radio_dev *rdev, struct freq_oper *oper);
BOOLEAN rc_radio_res_acquire(struct radio_dev *rdev, struct radio_res *res);


/*WMM Control*/
VOID WcReleaseEdca(struct hdev_obj *obj);
VOID  WcAcquiredEdca(struct hdev_obj *obj, EDCA_PARM *pEdcaParm);
INT32 WcInit(struct hdev_ctrl *ctrl, WMM_CTRL_T *pWmmCtrl);
INT32 WcExit(WMM_CTRL_T *pWmmCtrl);
VOID WcShowEdca(struct hdev_ctrl *ctrl);
UINT32 WcGetWmmNum(struct hdev_ctrl *ctrl);
EDCA_PARM *WcGetWmmByIdx(struct hdev_ctrl *ctrl, UINT32 Idx);
VOID WcSetEdca(struct hdev_obj *obj);


/*Omac Control*/
INT32 GetOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, INT8 Idx);
VOID ReleaseOmacIdx(struct hdev_ctrl *ctrl, UINT32 OmacType, UINT32 Idx);
VOID OcDelRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx);
INT32 OcAddRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx);
HD_REPT_ENRTY *OcGetRepeaterEntry(struct hdev_obj *obj, UCHAR ReptIdx);


/*Wctl Control*/
VOID WtcInit(struct hdev_ctrl *ctrl);
VOID WtcExit(struct hdev_ctrl *ctrl);
UCHAR WtcSetMaxStaNum(struct hdev_ctrl *ctrl, UCHAR BssidNum, UCHAR MSTANum);
UCHAR WtcGetMaxStaNum(struct hdev_ctrl *ctrl);
UCHAR WtcAcquireGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj);
UCHAR WtcReleaseGroupKeyWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UCHAR idx);
UCHAR WtcGetWcidLinkType(struct hdev_ctrl *ctrl, UCHAR idx);
UCHAR WtcAcquireUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj);
UCHAR WtcReleaseUcastWcid(struct hdev_ctrl *ctrl, struct hdev_obj *obj, UCHAR idx);
VOID WtcRecDump(struct hdev_ctrl *ctrl);
UCHAR WtcHwAcquireWcid(struct hdev_ctrl *ctrl, UCHAR idx);
UCHAR WtcHwReleaseWcid(struct hdev_ctrl *ctrl, UCHAR idx);

#define INVAILD_WCID 0xff

#endif /*__HDEV_H*/
