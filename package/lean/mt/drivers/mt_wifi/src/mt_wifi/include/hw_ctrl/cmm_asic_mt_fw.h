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


#ifndef __CMM_ASIC_MT_FW_H__
#define __CMM_ASIC_MT_FW_H__


struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _EXT_CMD_CHAN_SWITCH_T;
struct _BCTRL_INFO_T;
struct _BSS_INFO_ARGUMENT_T;


/* Function by FW */
INT32 MtAsicSetDevMacByFw(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature);

INT32 MtAsicSetBssidByFw(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T bss_info_argument);

INT32 MtAsicSetStaRecByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg);

INT32 MtAsicUpdateStaRecBaByFw(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_BA_CFG_T StaRecBaCfg);

VOID MtAsicDelWcidTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR wcid_idx);

#ifdef HTC_DECRYPT_IOT
VOID MtAsicSetWcidAAD_OMByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR wcid_idx,
	IN UCHAR value);
#endif /* HTC_DECRYPT_IOT */

VOID MtAsicUpdateRxWCIDTableByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo);

INT32 MtAsicUpdateBASessionByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_BA_CTRL_T BaCtrl);

UINT16 MtAsicGetTidSnByFw(
	IN struct _RTMP_ADAPTER *pAd,
	UCHAR wcid,
	UCHAR tid);

VOID MtAsicAddRemoveKeyTabByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _ASIC_SEC_INFO *pInfo);

VOID MtAsicSetSMPSByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR Smps);


VOID mt_wtbltlv_debug(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, UCHAR ucCmdId, UCHAR ucAtion);


VOID MtAsicUpdateProtectByFw(
	struct _RTMP_ADAPTER *pAd, MT_PROTECT_CTRL_T *ProtectCtrl);


VOID MtAsicUpdateRtsThldByFw(
	struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR pkt_num, UINT32 length);

VOID MtSetTmrCRByFw(struct _RTMP_ADAPTER *pAd, UCHAR enable, UCHAR BandIdx);
#define BA_TRIGGER_OFFLOAD_TIMEOUT 1000
VOID MtAsicAutoBATrigger(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Timeout);

INT MtAsicSetRDGByFw(struct _RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg);

INT MtAsicGetTsfTimeByFirmware(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx);

INT32 MtAsicSetAid(
	struct _RTMP_ADAPTER *pAd,
	UINT16 Aid);

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT MtAsicSetReptFuncEnableByFw(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
VOID MtAsicInsertRepeaterEntryByFw(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr);
VOID MtAsicRemoveRepeaterEntryByFw(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);

VOID MtAsicInsertRepeaterRootEntryByFw(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifdef DBDC_MODE
INT32 MtAsicGetDbdcCtrlByFw(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pbInfo);
INT32 MtAsicSetDbdcCtrlByFw(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pbInfo);
#endif /*DBDC_MODE*/


UINT32 MtAsicGetChBusyCntByFw(struct _RTMP_ADAPTER *pAd, UCHAR ch_idx);
UINT32 MtAsicGetWmmParamByFw(struct _RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType);

INT32 MtAsicSetMacTxRxByFw(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT32 MtAsicSetRxvFilter(RTMP_ADAPTER *pAd, BOOLEAN Enable, UCHAR BandIdx);

VOID MtAsicDisableSyncByFw(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);
VOID MtAsicEnableBssSyncByFw(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);

#endif
