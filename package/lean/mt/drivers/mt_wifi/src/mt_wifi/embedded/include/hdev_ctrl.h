/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	hdev_ctrl.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __HDEV_CTRL_H__
#define __HDEV_CTRL_H__

struct _RTMP_ADAPTER;
struct _EDCA_PARM;
struct _QLOAD_CTRL;
struct _AUTO_CH_CTRL;
struct wifi_dev;
struct _OMAC_BSS_CTRL;
struct _REPEATER_CLIENT_ENTRY;
struct radio_res;
struct freq_oper;


enum {
	HC_STATUS_OK,
	HC_STATUS_FAIL
};

#define INVAILD_WCID 0xff

INT32 HcRadioInit(struct _RTMP_ADAPTER *pAd, UCHAR RfIC, UCHAR DbdcMode);
INT32 HcAcquireRadioForWdev(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
INT32 HcReleaseRadioForWdev(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef CUSTOMER_DCC_FEATURE
INT32 HcUpdateChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR HcGetExtCha(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
#endif
INT32 HcUpdateCsaCntByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR HcGetBandByWdev(struct wifi_dev *wdev);
VOID HcSetRadioCurStatByWdev(struct wifi_dev *wdev, PHY_STATUS CurStat);
VOID HcSetRadioCurStatByChannel(RTMP_ADAPTER *pAd, UCHAR Channel, PHY_STATUS CurStat);
VOID HcSetAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
VOID HcSetAllSupportedBandsRadioOn(RTMP_ADAPTER *pAd);
BOOLEAN IsHcRadioCurStatOffByWdev(struct wifi_dev *wdev);
BOOLEAN IsHcRadioCurStatOffByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
BOOLEAN IsHcAllSupportedBandsRadioOff(RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcSetGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, BOOLEAN bGreenAPActive);
BOOLEAN IsHcGreenAPActiveByBand(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
BOOLEAN IsHcGreenAPActiveByWdev(struct wifi_dev *wdev);
#endif /* GREENAP_SUPPORT */
UCHAR HcGetChannelByBf(struct _RTMP_ADAPTER *pAd);
BOOLEAN HcIsRadioAcq(struct wifi_dev *wdev);
BOOLEAN HcIsBfCapSupport(struct wifi_dev *wdev);

/*Wtable Ctrl*/
UCHAR HcAcquireGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcReleaseGroupKeyWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx);
UCHAR HcGetMaxStaNum(struct _RTMP_ADAPTER *pAd);
UCHAR HcSetMaxStaNum(struct _RTMP_ADAPTER *pAd);

UCHAR HcGetWcidLinkType(struct _RTMP_ADAPTER *pAd, UCHAR Wcid);
UCHAR HcAcquireUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcReleaseUcastWcid(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR idx);
VOID HcWtblRecDump(struct _RTMP_ADAPTER *pAd);

#ifdef DBDC_MODE
VOID HcShowBandInfo(struct _RTMP_ADAPTER *pAd);
#endif /*DBDC_MODE*/

VOID HcShowChCtrlInfo(struct _RTMP_ADAPTER *pAd);
#ifdef GREENAP_SUPPORT
VOID HcShowGreenAPInfo(RTMP_ADAPTER *pAd);
#endif /* GREENAP_SUPPORT */

INT32 hdev_ctrl_init(struct _RTMP_ADAPTER *pAd);
VOID hdev_ctrl_exit(struct _RTMP_ADAPTER *pAd);
VOID hdev_resource_init(void *hdev_ctrl);


/*
WMM
*/
VOID HcAcquiredEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _EDCA_PARM *pEdca);
VOID HcReleaseEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


/*
* OmacCtrl
*/
UCHAR HcGetOmacIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);


/*Should remove it*/
UCHAR HcGetChannelByRf(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
UCHAR HcGetRadioPhyMode(struct _RTMP_ADAPTER *pAd);
UCHAR HcGetRadioPhyModeByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UCHAR HcGetRadioChannel(struct _RTMP_ADAPTER *pAd);
BOOLEAN  HcIsRfSupport(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);
BOOLEAN  HcIsRfRun(struct _RTMP_ADAPTER *pAd, UCHAR RfIC);

UCHAR HcGetBw(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

UCHAR HcGetRadioRfIC(struct _RTMP_ADAPTER *pAd);

struct _QLOAD_CTRL *HcGetQloadCtrlByRf(struct _RTMP_ADAPTER *pAd, UINT32 RfIC);
struct _QLOAD_CTRL *HcGetQloadCtrl(struct _RTMP_ADAPTER *pAd);
struct _AUTO_CH_CTRL *HcGetAutoChCtrl(struct _RTMP_ADAPTER *pAd);

struct _AUTO_CH_CTRL *HcGetAutoChCtrlbyBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UINT32 HcGetMgmtQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE pkt_type);
UINT32 HcGetBcnQueueIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UINT32 HcGetTxRingIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, enum PACKET_TYPE, UCHAR q_idx);
UINT32 HcGetWmmIdx(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
UCHAR HcGetBandByChannel(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
EDCA_PARM *HcGetEdca(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID HcSetEdca(struct wifi_dev *wdev);
VOID HcCrossChannelCheck(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR Channel);

#ifdef MAC_REPEATER_SUPPORT
INT32 HcAddRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx);
INT32 HcDelRepeaterEntry(struct wifi_dev *wdev, UINT32 ReptIdx);
UCHAR HcGetRepeaterOmac(struct _RTMP_ADAPTER *pAd, struct _MAC_TABLE_ENTRY *pEntry);
#endif /*#MAC_REPEATER_SUPPORT*/
UCHAR HcGetAmountOfBand(struct _RTMP_ADAPTER *pAd);
INT32 HcUpdateMSDUTxAllowByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);
INT32 HcSuspendMSDUTxByChannel(RTMP_ADAPTER *pAd, UCHAR Channel);

/*radio resource mgmt*/
BOOLEAN hc_radio_res_request(struct wifi_dev *wdev, struct radio_res *res);
UCHAR hc_reset_radio(struct _RTMP_ADAPTER *ad);
VOID hc_set_rrm_init(struct wifi_dev *wdev);
INT hc_radio_query_by_wdev(struct wifi_dev *wdev, struct freq_oper *oper);
INT hc_radio_query_by_channel(struct _RTMP_ADAPTER *ad, UCHAR channel,struct freq_oper *oper);
INT hc_radio_query_by_index(struct _RTMP_ADAPTER *ad, UCHAR index, struct freq_oper *oper);
/*temporally use*/
INT hc_radio_query_by_rf(struct _RTMP_ADAPTER *ad, UCHAR rfic, struct freq_oper *oper);

INT hc_obj_init(struct wifi_dev *wdev, INT idx);
VOID hc_obj_exit(struct wifi_dev *wdev);

CHANNEL_CTRL *hc_get_channel_ctrl(void *hdev_ctrl, UCHAR BandIdx);
UCHAR hc_init_ChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_init_ACSChCtrl(RTMP_ADAPTER *pAd);
UCHAR hc_init_ACSChCtrlByBandIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx);
UCHAR hc_set_ChCtrl(CHANNEL_CTRL *ChCtrl, RTMP_ADAPTER *pAd, UCHAR ChIdx, UCHAR ChIdx2);
UCHAR hc_set_ChCtrlFlags_CAP(CHANNEL_CTRL *ChCtrl, UINT ChannelListFlag, UCHAR ChIdx);
UCHAR hc_set_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);
UCHAR hc_check_ChCtrlChListStat(CHANNEL_CTRL *ChCtrl, CH_LIST_STATE ChListStat);
/*chipcap & chipop related*/
struct _RTMP_CHIP_CAP *hc_get_chip_cap(void *hdev_ctrl);
struct _RTMP_CHIP_OP *hc_get_chip_ops(void *hdev_ctrl);
void hc_register_chip_ops(void *hdev_ctrl, struct _RTMP_CHIP_OP *ops);
struct _RTMP_ARCH_OP *hc_get_asic_ops(void *hdev_ctrl);
struct _RTMP_CHIP_DBG *hc_get_chip_dbg(void *hdev_ctrl);
UINT32 hc_get_hif_type(void *hdev_ctrl);
VOID hc_clear_asic_cap(void *hdev_ctrl, UINT32 caps);
VOID hc_set_asic_cap(void *hdev_ctrl, UINT32 caps);
UINT32 hc_get_asic_cap(void *hdev_ctrl);
UINT8 hc_get_chip_bcn_max_num(void *hdev_ctrl);
UINT8 hc_get_chip_bcn_hw_num(void *hdev_ctrl);
BOOLEAN hc_get_chip_wapi_sup(void *hdev_ctrl);
VOID *hc_get_hif_ctrl(void *hdev_ctrl);
VOID *hc_get_os_cookie(void *hdev_ctrl);
VOID *hc_get_mcu_ctrl(void *hdev_ctrl);

#endif /*__HDEV_CTRL_H__*/
