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
#define RT_CFG80211_DEBUG 			/* debug use */
#ifdef RT_CFG80211_DEBUG
#define CFG80211DBG(__Flg, __pMsg)		DBGPRINT(__Flg, __pMsg)
#else
#define CFG80211DBG(__Flg, __pMsg)
#endif /* RT_CFG80211_DEBUG */

//CFG_TODO
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

#define RT_CFG80211_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe, 			\
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
	CFG80211_ConnectResultInform((VOID *)__pAd, __pBSSID,					\
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_SCANNING_INFORM(__pAd, __BssIdx, __ChanId, __pFrame,	\
			__FrameLen, __RSSI)									\
	CFG80211_Scaning((VOID *)__pAd, __BssIdx, __ChanId, __pFrame,			\
						__FrameLen, __RSSI);

#define RT_CFG80211_SCAN_END(__pAd, __FlgIsAborted)							\
	CFG80211_ScanEnd((VOID *)__pAd, __FlgIsAborted);
#ifdef CONFIG_STA_SUPPORT
#define RT_CFG80211_LOST_AP_INFORM(__pAd) 									\
	CFG80211_LostApInform((VOID *)__pAd);	
#endif /*CONFIG_STA_SUPPORT*/
#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
#define RT_CFG80211_LOST_GO_INFORM(__pAd) 									\
	CFG80211_LostP2pGoInform((VOID *)__pAd);	
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
#define RT_CFG80211_REINIT(__pAd)											\
	CFG80211_SupBandReInit((VOID *)__pAd);	

#define RT_CFG80211_RFKILL_STATUS_UPDATE(_pAd, _active) 					\
	CFG80211_RFKillStatusUpdate(_pAd, _active);

#define RT_CFG80211_P2P_CLI_CONN_RESULT_INFORM(__pAd, __pBSSID, __pReqIe,   \
			__ReqIeLen,	__pRspIe, __RspIeLen, __FlgIsSuccess)				\
	CFG80211_P2pClientConnectResultInform(__pAd, __pBSSID,				    \
			__pReqIe, __ReqIeLen, __pRspIe, __RspIeLen, __FlgIsSuccess);

#define RT_CFG80211_P2P_CLI_SEND_NULL_FRAME(_pAd, _PwrMgmt)					\
	CFG80211_P2pClientSendNullFrame(_pAd, _PwrMgmt);
	

#ifdef SINGLE_SKU
#define CFG80211_BANDINFO_FILL(__pAd, __pBandInfo)							\
{																			\
	(__pBandInfo)->RFICType = __pAd->phy_ctrl.rf_band_cap;								\
	(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
	(__pBandInfo)->TxStream = __pAd->CommonCfg.TxStream;					\
	(__pBandInfo)->RxStream = __pAd->CommonCfg.RxStream;					\
	(__pBandInfo)->MaxTxPwr = __pAd->CommonCfg.DefineMaxTxPwr;				\
	if (WMODE_EQUAL(__pAd->CommonCfg.PhyMode, WMODE_B))				\
		(__pBandInfo)->FlgIsBMode = TRUE;									\
	else																	\
		(__pBandInfo)->FlgIsBMode = FALSE;									\
	(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;						\
	(__pBandInfo)->RtsThreshold = pAd->CommonCfg.RtsThreshold;				\
	(__pBandInfo)->FragmentThreshold = pAd->CommonCfg.FragmentThreshold;	\
	(__pBandInfo)->RetryMaxCnt = 0;											\
	RTMP_IO_READ32(__pAd, TX_RTY_CFG, &((__pBandInfo)->RetryMaxCnt));		\
}
#else
#define CFG80211_BANDINFO_FILL(__pAd, __pBandInfo)							\
{																			\
	(__pBandInfo)->RFICType = __pAd->phy_ctrl.rf_band_cap;								\
	(__pBandInfo)->MpduDensity = __pAd->CommonCfg.BACapability.field.MpduDensity;\
	(__pBandInfo)->TxStream = __pAd->CommonCfg.TxStream;					\
	(__pBandInfo)->RxStream = __pAd->CommonCfg.RxStream;					\
	(__pBandInfo)->MaxTxPwr = 0;											\
	if (WMODE_EQUAL(__pAd->CommonCfg.PhyMode, WMODE_B))				\
		(__pBandInfo)->FlgIsBMode = TRUE;									\
	else																	\
		(__pBandInfo)->FlgIsBMode = FALSE;									\
	(__pBandInfo)->MaxBssTable = MAX_LEN_OF_BSS_TABLE;						\
	(__pBandInfo)->RtsThreshold = pAd->CommonCfg.RtsThreshold;				\
	(__pBandInfo)->FragmentThreshold = pAd->CommonCfg.FragmentThreshold;	\
	(__pBandInfo)->RetryMaxCnt = 0;											\
	RTMP_IO_READ32(__pAd, TX_RTY_CFG, &((__pBandInfo)->RetryMaxCnt));		\
}
#endif /* SINGLE_SKU */

/* Scan Releated */
#ifdef CONFIG_STA_SUPPORT
BOOLEAN CFG80211DRV_OpsScanRunning(VOID *pAdOrg);
#endif /*CONFIG_STA_SUPPORT*/
BOOLEAN CFG80211DRV_OpsScanSetSpecifyChannel(
	VOID *pAdOrg, VOID *pData, UINT8 dataLen);

BOOLEAN CFG80211DRV_OpsScanCheckStatus(
	VOID *pAdOrg, UINT8	IfType);

BOOLEAN CFG80211DRV_OpsScanExtraIesSet(VOID *pAdOrg);

VOID CFG80211DRV_OpsScanInLinkDownAction(VOID *pAdOrg);

INT CFG80211DRV_OpsScanGetNextChannel(VOID *pAdOrg);

VOID CFG80211_ScanStatusLockInit(VOID *pAdCB, UINT init);

VOID CFG80211_Scaning(
	VOID *pAdCB, UINT32	BssIdx, UINT32 ChanId, UCHAR *pFrame, UINT32 FrameLen, INT32 RSSI);

VOID CFG80211_ScanEnd(VOID *pAdCB, BOOLEAN FlgIsAborted);

/* Connect Releated */
BOOLEAN CFG80211DRV_OpsJoinIbss(VOID *pAdOrg, VOID *pData);
BOOLEAN CFG80211DRV_OpsLeave(VOID *pAdOrg, UINT8	 IfType);
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

#ifdef RT_CFG80211_P2P_CONCURRENT_DEVICE
VOID CFG80211_LostP2pGoInform(VOID *pAdCB);
#endif /*RT_CFG80211_P2P_CONCURRENT_DEVICE*/
VOID CFG80211_LostApInform(VOID *pAdCB);

INT CFG80211_StaPortSecured(
    VOID                         *pAdCB,
    UCHAR                        *pMac,
    UINT    					  flag);	

/* AP Related*/
INT CFG80211_ApStaDel(VOID *pAdCB, UCHAR *pMac);

VOID CFG80211_UpdateBeacon(
   VOID                           *pAdOrg,
   UCHAR                          *beacon_head_buf,
   UINT32                          beacon_head_len,
   UCHAR                          *beacon_tail_buf,
   UINT32                          beacon_tail_len,
   BOOLEAN                         isAllUpdate);

INT CFG80211_ApStaDelSendEvent(PRTMP_ADAPTER pAd, const PUCHAR mac_addr);


/* Information Releated */
BOOLEAN CFG80211DRV_StaGet(
	VOID						*pAdOrg,
	VOID						*pData);

VOID CFG80211DRV_SurveyGet(
	VOID						*pAdOrg,
	VOID						*pData);

INT CFG80211_reSetToDefault(
	VOID                    	*pAdCB);



/* Key Releated */
BOOLEAN CFG80211DRV_StaKeyAdd(
	VOID						*pAdOrg,
	VOID						*pData);

BOOLEAN CFG80211DRV_ApKeyAdd(
        VOID                    *pAdOrg,
        VOID                    *pData);

BOOLEAN CFG80211DRV_ApKeyDel(
	VOID						*pAdOrg,
	VOID						*pData);

VOID CFG80211DRV_RtsThresholdAdd(
	VOID                                            *pAdOrg,
	UINT                                            threshold);

VOID CFG80211DRV_FragThresholdAdd(
	VOID                                            *pAdOrg,
	UINT                                            threshold);


INT CFG80211_setApDefaultKey(
    VOID                        *pAdCB,
    UINT                         Data);

INT CFG80211_setPowerMgmt(
	VOID                     *pAdCB,
	UINT 			Enable);
		
#ifdef CONFIG_STA_SUPPORT
INT CFG80211_setStaDefaultKey(
    VOID                        *pAdCB,
    UINT                         Data);

#ifdef DOT11W_PMF_SUPPORT
INT CFG80211_setStaMgmtDefaultKey(
    VOID                        *pAdCB,
    UINT                         Data);
#endif /* DOT11W_PMF_SUPPORT */

#endif /*CONFIG_STA_SUPPORT*/
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
	VOID						*pAdCB);

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
        IN VOID                                         *pAdCB);

VOID CFG80211_P2pClientSendNullFrame(
	VOID 						*pAdCB,
	INT							 PwrMgmt);

VOID CFG80211RemainOnChannelTimeout(
	PVOID 						SystemSpecific1,
	PVOID 						FunctionContext,
	PVOID 						SystemSpecific2,
	PVOID 						SystemSpecific3);

BOOLEAN CFG80211DRV_OpsRemainOnChannel(	
	VOID						*pAdOrg,	
	VOID						*pData,	
	UINT32 						duration);

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
        IN  RTMP_ADAPTER 								 *pAd,
		IN	PUCHAR										 preStr,
		IN	PUCHAR										 pData,
		IN	UINT32                              		 length);


BOOLEAN CFG80211_SyncPacketWmmIe(RTMP_ADAPTER *pAd, VOID *pData, ULONG dataLen);
BOOLEAN CFG80211_HandleP2pMgmtFrame(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR OpMode);
INT CFG80211_SendMgmtFrame(RTMP_ADAPTER *pAd, VOID *pData, ULONG Data);

#ifdef RT_CFG80211_P2P_SUPPORT
VOID CFG80211_PeerP2pBeacon(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR	pAddr2,
	IN MLME_QUEUE_ELEM *Elem,
	IN LARGE_INTEGER   TimeStamp);


VOID CFG80211_P2pStopNoA(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient);


BOOLEAN CFG80211_P2pResetNoATimer(
	IN PRTMP_ADAPTER pAd,
	IN	ULONG	DiffTimeInus);


BOOLEAN CFG80211_P2pHandleNoAAttri(
	IN PRTMP_ADAPTER pAd, 
	IN PMAC_TABLE_ENTRY	pMacClient,
	IN PUCHAR pData);

#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
VOID CFG80211_ParseBeaconIE(RTMP_ADAPTER *pAd, MULTISSID_STRUCT *pMbss, struct wifi_dev *wdev,UCHAR *wpa_ie,UCHAR *rsn_ie);
#endif

//--------------------------------
VOID CFG80211_Convert802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR *pHeader802_3);
VOID CFG80211_Announce802_3Packet(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, UCHAR FromWhichBSSID);
VOID CFG80211_SendMgmtFrameDone(RTMP_ADAPTER *pAd, USHORT Sequence);

VOID CFG80211_SwitchTxChannel(RTMP_ADAPTER *pAd, ULONG Data);

BOOLEAN CFG80211DRV_OpsBeaconSet(
        VOID                                            *pAdOrg,
        VOID                                            *pData);

BOOLEAN CFG80211DRV_OpsBeaconAdd(
        VOID                                            *pAdOrg,
        VOID                                            *pData);


VOID CFG80211DRV_DisableApInterface(PRTMP_ADAPTER pAd);


BOOLEAN CFG80211DRV_OpsVifAdd(
        VOID                                            *pAdOrg,
        VOID                                            *pData);

#endif /* RT_CFG80211_SUPPORT */

#endif /* __CFG80211EXTR_H__ */
