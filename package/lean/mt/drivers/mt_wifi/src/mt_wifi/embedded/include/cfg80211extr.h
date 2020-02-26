/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2013, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************

	Abstract:

	All CFG80211 Function Prototype.

***************************************************************************/

#ifndef __CFG80211EXTR_H__
#define __CFG80211EXTR_H__

#ifdef RT_CFG80211_SUPPORT

#define CFG80211CB				    (pAd->pCfg80211_CB)
#define RT_CFG80211_DEBUG			/* debug use */
#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, __Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif /* RT_CFG80211_DEBUG */

/* CFG_TODO */
#include "wfa_p2p.h"

#define RT_CFG80211_REGISTER(__pDev, __pNetDev)								\
	CFG80211_Register(__pDev, __pNetDev);

#define RT_CFG80211_BEACON_CR_PARSE(__pAd, __pVIE, __LenVIE)				\
	CFG80211_BeaconCountryRegionParse((VOID *)__pAd, __pVIE, __LenVIE);

#define RT_CFG80211_BEACON_TIM_UPDATE(__pAd)                                \
	CFG80211_UpdateBeacon((VOID *)__pAd, NULL, 0, NULL, 0, FALSE);

#define RT_CFG80211_CRDA_REG_HINT(__pAd, __pCountryIe, __CountryIeLen)		\
	CFG80211_RegHint((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_HINT11D(__pAd, __pCountryIe, __CountryIeLen)	\
	CFG80211_RegHint11D((VOID *)__pAd, __pCountryIe, __CountryIeLen);

#define RT_CFG80211_CRDA_REG_RULE_APPLY(__pAd)								\
	CFG80211_RegRuleApply((VOID *)__pAd, NULL, __pAd->cfg80211_ctrl.Cfg80211_Alpha2);

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,			\
									   __ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
CFG80211_ConnectResultInform((VOID *)__pAd, __pBSSID,					\
							 __pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,	\
									__FrameLen, __RSSI)									\
CFG80211_Scaning((VOID *)__pAd, __BssIdx, __ChanId, __pFrame,			\
				 __FrameLen, __RSSI);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)							\
	CFG80211_ScanEnd((VOID *)__pAd, __FlgIsAborted);
#define RT_CFG80211_REINIT(__pAd)											\
	CFG80211_SupBandReInit((VOID *)__pAd, (VOID *)__wdev);

#define RT_CFG80211_RFKILL_STATUS_UPDATE(_pAd, _active)					\
	CFG80211_RFKillStatusUpdate(_pAd, _active);

#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,   \
		__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
CFG80211_P2pClientConnectResultInform(__pAd, __pBSSID,				    \
									  __pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(_pAd, _PwrMgmt)					\
	CFG80211_P2pClientSendNullFrame(_pAd, _PwrMgmt);

#define RT_CFG80211_JOIN_IBSS(_pAd, _pBssid) \
	CFG80211_JoinIBSS(_pAd, _pBssid);


#define CFG80211_BANDINFO_FILL(__pAd, __wdev, __pBandInfo)\
	{\
		do {\
			(__pBandInfo)->RFICType = HcGetRadioRfIC(__pAd);\
			(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
			(__pBandInfo)->TxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? 1 : wlan_operate_get_tx_stream(__wdev);			\
			(__pBandInfo)->RxStream = ((__wdev == NULL) || (__wdev->wpf_op == NULL)) ? 1 : wlan_operate_get_rx_stream(__wdev);			\
			(__pBandInfo)->MaxTxPwr = 0;\
			if (WMODE_EQUAL(HcGetRadioPhyMode(__pAd), WMODE_B))\
				(__pBandInfo)->FlgIsBMode = TRUE;\
			else\
				(__pBandInfo)->FlgIsBMode = FALSE;\
			(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;\
			(__pBandInfo)->RtsThreshold = (__wdev == NULL) ? DEFAULT_RTS_LEN_THLD : wlan_operate_get_rts_len_thld(__wdev);\
			(__pBandInfo)->FragmentThreshold = (__wdev == NULL) ? DEFAULT_FRAG_THLD : wlan_operate_get_frag_thld(__wdev);\
			(__pBandInfo)->RetryMaxCnt = 0; \
		} while (0); \
	}

/* NoA Command Parm */
#define P2P_NOA_DISABLED 0x00
#define P2P_NOA_TX_ON    0x01
#define P2P_NOA_RX_ON    0x02

#define WLAN_AKM_SUITE_8021X		0x000FAC01
#define WDEV_NOT_FOUND				-1


/* Scan Releated */

BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID *pAdOrg, VOID *pData, UINT8 dataLen);

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID *pAdOrg, UINT8	IfType);

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(VOID *pAdOrg);

VOID CFG80211DRV_OpsScanInLinkDownAction(VOID *pAdOrg);

#ifdef CONFIG_MULTI_CHANNEL
VOID CFG80211DRV_Set_NOA(VOID *pAdOrg, VOID *pData);
#endif /* CONFIG_MULTI_CHANNEL */

INT CFG80211DRV_OpsScanGetNextChannel(VOID *pAdOrg);

VOID CFG80211_ScanStatusLockInit(VOID *pAdCB, UINT init);

VOID CFG80211_Scaning(VOID *pAdCB, UINT32 BssIdx, UINT32 ChanId,
					  UCHAR *pFrame, UINT32 FrameLen, INT32 RSSI);

VOID CFG80211_ScanEnd(VOID *pAdCB, BOOLEAN FlgIsAborted);

/* Connect Releated */
BOOLEAN CFG80211DRV_OpsJoinIbss(VOID *pAdOrg, VOID *pData);
BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, PNET_DEV pNetDev);
BOOLEAN CFG80211DRV_Connect(VOID *pAdOrg, VOID *pData);
VOID CFG80211_P2pClientConnectResultInform(
	IN VOID                                         *pAdCB,
	IN UCHAR                                        *pBSSID,
	IN UCHAR                                        *pReqIe,
	IN UINT32                                       ReqIeLen,
	IN UCHAR                                        *pRspIe,
	IN UINT32                                       RspIeLen,
	IN UCHAR                                        FlgIsSuccess);


VOID CFG80211_ConnectResultInform(
	VOID *pAdCB, UCHAR *pBSSID,	UCHAR *pReqIe, UINT32 ReqIeLen,
	UCHAR *pRspIe, UINT32 RspIeLen,	UCHAR FlgIsSuccess);
VOID CFG80211DRV_PmkidConfig(VOID *pAdOrg, VOID *pData);
VOID CFG80211_LostApInform(VOID *pAdCB);

INT CFG80211_StaPortSecured(
	VOID                         *pAdCB,
	UCHAR                        *pMac,
	UINT						flag);

/* AP Related*/
INT CFG80211_ApStaDel(VOID *pAdCB, UCHAR *pMac);

VOID CFG80211_UpdateBeacon(
	VOID                           *pAdOrg,
	UCHAR                          *beacon_head_buf,
	UINT32                          beacon_head_len,
	UCHAR                          *beacon_tail_buf,
	UINT32                          beacon_tail_len,
	BOOLEAN                         isAllUpdate);

INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr, IN PNET_DEV pNetDevIn);
INT CFG80211_FindMbssApIdxByNetDevice(RTMP_ADAPTER *pAd, PNET_DEV pNetDev);



/* Information Releated */
BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData);

VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData);

INT CFG80211_reSetToDefault(
	VOID						*pAdCB);


/* Key Releated */
BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_ApKeyAdd(
	VOID                    *pAdOrg,
	VOID                    *pData);

VOID CFG80211DRV_RtsThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev	*wdev,
	UINT                                            threshold);

VOID CFG80211DRV_FragThresholdAdd(
	VOID                                            *pAdOrg,
	struct wifi_dev	*wdev,
	UINT                                            threshold);

BOOLEAN CFG80211DRV_ApKeyDel(
	VOID						*pAdOrg,
	VOID						*pData);

INT CFG80211_setApDefaultKey(
	VOID                        *pAdCB,
	UINT                         Data);
INT CFG80211_setPowerMgmt(
	VOID                     *pAdCB,
	UINT			Enable);

/* General Releated */
BOOLEAN CFG80211DRV_OpsSetChannel(RTMP_ADAPTER *pAd, VOID *pData);

BOOLEAN CFG80211DRV_OpsChgVirtualInf(RTMP_ADAPTER *pAd, VOID *pData);

VOID CFG80211DRV_OpsChangeBssParm(VOID *pAdOrg, VOID *pData);

VOID CFG80211_UnRegister(VOID *pAdOrg,	VOID *pNetDev);

INT CFG80211DRV_IoctlHandle(
	VOID						*pAdSrc,
	RTMP_IOCTL_INPUT_STRUCT		*wrq,
	INT							cmd,
	USHORT						subcmd,
	VOID						*pData,
	ULONG						Data);

UCHAR CFG80211_getCenCh(RTMP_ADAPTER *pAd, UCHAR prim_ch);

/* CRDA Releatd */
VOID CFG80211DRV_RegNotify(
	VOID						*pAdOrg,
	VOID						*pData);

VOID CFG80211_RegHint(
	VOID						*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegHint11D(
	VOID						*pAdCB,
	UCHAR						*pCountryIe,
	ULONG						CountryIeLen);

VOID CFG80211_RegRuleApply(
	VOID						*pAdCB,
	VOID						*pWiphy,
	UCHAR						*pAlpha2);

BOOLEAN CFG80211_SupBandReInit(
	VOID						*pAdCB,
	VOID	*wdev);

#ifdef RFKILL_HW_SUPPORT
VOID CFG80211_RFKillStatusUpdate(
	PVOID						pAd,
	BOOLEAN						active);
#endif /* RFKILL_HW_SUPPORT */

/* P2P Related */
VOID CFG80211DRV_SetP2pCliAssocIe(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT                         ie_len);

VOID CFG80211DRV_P2pClientKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_P2pClientConnect(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211_checkScanTable(
	IN VOID                      *pAdCB);

VOID CFG80211_P2pClientSendNullFrame(
	VOID						*pAdCB,
	INT							 PwrMgmt);

VOID CFG80211RemainOnChannelTimeout(
	PVOID						SystemSpecific1,
	PVOID						FunctionContext,
	PVOID						SystemSpecific2,
	PVOID						SystemSpecific3);

BOOLEAN CFG80211DRV_OpsRemainOnChannel(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT32						duration);

BOOLEAN CFG80211DRV_OpsCancelRemainOnChannel(
	VOID                                            *pAdOrg,
	UINT32                                          cookie);


VOID CFG80211DRV_OpsMgmtFrameProbeRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg);

VOID CFG80211DRV_OpsMgmtFrameActionRegister(
	VOID                                            *pAdOrg,
	VOID                                            *pData,
	BOOLEAN                                          isReg);

BOOLEAN CFG80211_CheckActionFrameType(
	IN  RTMP_ADAPTER								 *pAd,
	IN	PUCHAR										 preStr,
	IN	PUCHAR										 pData,
	IN	UINT32										length);

BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen);
BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode);
INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data);



/* -------------------------------- */
/* VOID CFG80211_Convert802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR *pHeader802_3); */
VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID);
VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence, BOOLEAN ack);
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, struct wifi_dev *wdev, UCHAR *wpa_ie, UCHAR *rsn_ie);
#endif /*CONFIG_AP_SUPPORT*/
VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data);

BOOLEAN CFG80211DRV_OpsBeaconSet(
	VOID                                            *pAdOrg,
	VOID                                            *pData);

BOOLEAN CFG80211DRV_OpsBeaconAdd(
	VOID                                            *pAdOrg,
	VOID                                            *pData);


INT CFG80211_setStaDefaultKey(
	IN VOID                                         *pAdCB,
	IN UINT                                         Data);

VOID CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd);

BOOLEAN CFG80211DRV_OpsVifAdd(VOID *pAdOrg, VOID *pData);

#ifdef CFG_TDLS_SUPPORT
BOOLEAN CFG80211DRV_StaTdlsInsertDeletepEntry(
	VOID						*pAdOrg,
	VOID						*pData,
	UINT						Data);
BOOLEAN CFG80211DRV_StaTdlsSetKeyCopyFlag(
	VOID						*pAdOrg);
BOOLEAN CFG80211_HandleTdlsDiscoverRespFrame(
	RTMP_ADAPTER				*pAd,
	RX_BLK						*pRxBlk,
	UCHAR						OpMode);

VOID cfg_tdls_send_PeerTrafficIndication(PRTMP_ADAPTER pAd, u8 *peer);
VOID cfg_tdls_rcv_PeerTrafficIndication(PRTMP_ADAPTER pAd, u8 dialog_token, u8 *peer);
VOID cfg_tdls_rcv_PeerTrafficResponse(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_search_wcid(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_search_ValidLinkIndex(PRTMP_ADAPTER pAd, u8 *peer);
INT cfg_tdls_build_frame(PRTMP_ADAPTER	pAd, u8 *peer, u8 dialog_token, u8 action_code, u16 status_code
						 , const u8 *extra_ies, size_t extra_ies_len, BOOLEAN send_by_tdls_link, u8 tdls_entry_wcid, u8 reason_code);
VOID cfg_tdls_UAPSDP_PsmModeChange(PRTMP_ADAPTER pAd, USHORT	PsmOld, USHORT PsmNew);
BOOLEAN cfg_tdls_UAPSDP_AsicCanSleep(PRTMP_ADAPTER	pAd);
INT cfg_tdls_EntryInfo_Display_Proc(PRTMP_ADAPTER pAd, PUCHAR arg);
VOID cfg_tdls_TimerInit(PRTMP_ADAPTER pAd);
VOID cfg_tdls_PTITimeoutAction(IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
VOID cfg_tdls_BaseChannelTimeoutAction(IN PVOID SystemSpecific1, IN PVOID FunctionContext, IN PVOID SystemSpecific2, IN PVOID SystemSpecific3);
VOID cfg_tdls_rx_parsing(PRTMP_ADAPTER pAd, RX_BLK *pRxBlk);
INT cfg_tdls_chsw_req(PRTMP_ADAPTER	pAd, u8 *peer, u8 target_channel, u8 target_bw);
INT cfg_tdls_chsw_resp(PRTMP_ADAPTER	pAd, u8 *peer, UINT32 ch_sw_time, UINT32 ch_sw_timeout, u16 reason_code);
VOID cfg_tdls_prepare_null_frame(PRTMP_ADAPTER	pAd, BOOLEAN powersave, UCHAR dir, UCHAR *peerAddr);
VOID cfg_tdls_TunneledProbeRequest(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len);
VOID cfg_tdls_TunneledProbeResponse(PRTMP_ADAPTER pAd, PUCHAR pMacAddr, const u8  *extra_ies,	size_t extra_ies_len);
VOID cfg_tdls_auto_teardown(PRTMP_ADAPTER pAd, PMAC_TABLE_ENTRY pEntry);
INT cfg_tdls_send_CH_SW_SETUP(RTMP_ADAPTER *ad, UCHAR cmd, UCHAR offch_prim, UCHAR offch_center, UCHAR bw_off, UCHAR role, UINT16 stay_time, UINT32 start_time_tsf, UINT16 switch_time,	UINT16 switch_timeout);



#endif /*CFG_TDLS_SUPPORT*/
#endif /* RT_CFG80211_SUPPORT */

VOID CFG80211_JoinIBSS(
	IN VOID						*pAdCB,
	IN UCHAR					*pBSSID);

#ifdef MT_MAC
VOID CFG80211_InitTxSCallBack(RTMP_ADAPTER *pAd);
#endif /* MT_MAC */

#endif /* __CFG80211EXTR_H__ */

