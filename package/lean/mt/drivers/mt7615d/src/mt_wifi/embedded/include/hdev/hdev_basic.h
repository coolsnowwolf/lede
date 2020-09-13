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

#ifndef __HDEV_BASIC_H
#define __HDEV_BASIC_H

#ifdef RTMP_MAC_PCI
struct _PCI_HIF_T;
#endif /*RTMP_MAC_PCI*/

#include "common/link_list.h"

struct _RTMP_CHIP_CAP;
struct _RTMP_CHIP_OP;
struct _EDCA_PARM;
struct MCU_CTRL;
struct _BCTRL_INFO_T;
struct _BCTRL_ENTRY;
struct freq_oper;

struct radio_res {
	UCHAR reason;
	struct freq_oper *oper;
};

enum {
	REASON_NORMAL_SW,
	REASON_NORMAL_SCAN,
};

/*
* state machine:
* case1: NONE_OCCUPIED ->SW_OCCUPIED->NONE_OCCUPIED
*/

enum {
	WTBL_STATE_NONE_OCCUPIED = 0,
	WTBL_STATE_SW_OCCUPIED,
};

enum {
	WTBL_TYPE_NONE = 0,
	WTBL_TYPE_UCAST,
	WTBL_TYPE_MCAST,
};

typedef struct _WTBL_IDX_PARAMETER {
	UCHAR	State;
	UCHAR   LinkToOmacIdx;
	UCHAR   LinkToWdevType;
	UCHAR   WtblIdx;
	UCHAR	type;
	DL_LIST list;
} WTBL_IDX_PARAMETER, *PWTBL_IDX_PARAMETER;

typedef struct _WTBL_CFG {
	UCHAR MaxUcastEntryNum;
	UCHAR MinMcastWcid;
	BOOLEAN mcast_wait;
	WTBL_IDX_PARAMETER WtblIdxRec[MAX_LEN_OF_MAC_TABLE];
	NDIS_SPIN_LOCK WtblIdxRecLock;
} WTBL_CFG;

#define WTC_WAIT_TIMEOUT CMD_MSG_TIMEOUT

typedef struct _OMAC_BSS_CTRL {
	UINT32 OmacBitMap;
	UINT32 HwMbssBitMap;
	UINT32 RepeaterBitMap;
} OMAC_BSS_CTRL, *POMAC_BSS_CTRL;

typedef struct _WMM_CTRL {
	EDCA_PARM *pWmmSet;
} MT_WMMCTRL_T, WMM_CTRL_T;

typedef struct _HD_RESOURCE_CFG {
	struct rtmp_phy_ctrl	PhyCtrl[DBDC_BAND_NUM];
	struct _WMM_CTRL		WmmCtrl;
	struct _OMAC_BSS_CTRL	OmacBssCtl;
	/* struct _REPEATER_CFG	RepeaterCfg; */
	struct _WTBL_CFG		WtblCfg;
	UCHAR concurrent_bands;
} HD_RESOURCE_CFG;

struct radio_dev {
	UCHAR Idx;
	RADIO_CTRL *pRadioCtrl;
	DL_LIST DevObjList;
	UCHAR DevNum;
	/*implicit point to hdev_ctrl for sharing resource*/
	VOID     *priv;
};

enum {
	HOBJ_STATE_NONE = 0,
	HOBJ_STATE_USED,
};

struct hdev_obj {
	UCHAR Idx;
	UCHAR Type;
	UCHAR OmacIdx;
	UCHAR WmmIdx;
	BOOLEAN bWmmAcquired;
	struct radio_dev *rdev;
	DL_LIST RepeaterList;
	DL_LIST list;
	UCHAR state;
	UCHAR RefCnt;
	NDIS_SPIN_LOCK RefCntLock;
};

struct hdev_ctrl {
	struct radio_dev rdev[DBDC_BAND_NUM];
	CHANNEL_CTRL ChCtrl[DBDC_BAND_NUM];
	/* PSE_CFG				PseCfg; */
	/* HIF_CFG				HifCfg; */
	struct _RTMP_CHIP_CAP chip_cap;
	struct _RTMP_CHIP_OP chip_ops;
	struct _RTMP_ARCH_OP *arch_ops;
	struct _RTMP_CHIP_DBG chip_dbg;
	HD_RESOURCE_CFG	HwResourceCfg;
	struct hdev_obj HObjList[WDEV_NUM_MAX];
	VOID *mcu_ctrl;
	VOID *hif;
	VOID *cookie;
#ifdef CUT_THROUGH
	VOID *PktTokenCb;
#endif /* CUT_THROUGH */
	VOID *priv; /*implicit point to pAd*/
};

typedef struct _HD_REPT_ENRTY {
	UCHAR CliIdx;
	UCHAR ReptOmacIdx;
	DL_LIST list;
} HD_REPT_ENRTY;

/*for hdev base functions*/
VOID HdevObjAdd(struct radio_dev *rdev, struct hdev_obj *obj);
VOID HdevObjDel(struct radio_dev *rdev, struct hdev_obj *obj);
BOOLEAN hdev_obj_state_ready(struct hdev_obj *obj);

INT32 HdevInit(struct hdev_ctrl *hdev_ctrl, UCHAR HdevIdx, RADIO_CTRL *pRadioCtrl);
INT32 HdevExit(struct hdev_ctrl *hdev_ctrl, UCHAR HdevIdx);
VOID HdevCfgShow(struct hdev_ctrl *hdev_ctrl);
VOID HdevObjShow(struct hdev_obj *obj);
VOID HdevHwResourceExit(struct hdev_ctrl *ctrl);
VOID HdevHwResourceInit(struct hdev_ctrl *ctrl);

#endif /*__HDEV_BASIC_H*/
