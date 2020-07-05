/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_H__
#define __AP_H__

#ifdef DOT11R_FT_SUPPORT
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

#define INFRA_TP_PEEK_BOUND_THRESHOLD 50
#define VERIWAVE_TP_PEEK_BOUND_TH 30
#define VERIWAVE_2G_PKT_CNT_TH 100
#define VERIWAVE_5G_PKT_CNT_TH 200
#define BYTES_PER_SEC_TO_MBPS	17
#define TX_MODE_RATIO_THRESHOLD	70
#define RX_MODE_RATIO_THRESHOLD	70
#define STA_TP_IDLE_THRESHOLD 10
#define STA_NUMBER_FOR_TRIGGER                1
#define MULTI_CLIENT_NUMS_TH 16
#define MULTI_CLIENT_2G_NUMS_TH 16
#define INFRA_KEEP_STA_PKT_TH 1
#define VERIWAVE_TP_AMSDU_DIS_TH 1400
#define VERIWAVE_2G_TP_AMSDU_DIS_TH 1024
#define VERIWAVE_PER_RTS_DIS_TH_LOW_MARK 3
#define VERIWAVE_PER_RTS_DIS_TH_HIGH_MARK 9
#define VERIWAVE_PKT_LEN_LOW 60
#define VERIWAVE_INVALID_PKT_LEN_HIGH 2000
#define VERIWAVE_INVALID_PKT_LEN_LOW 64
#define	TRAFFIC_0	0
#define	TRAFFIC_DL_MODE	1
#define	TRAFFIC_UL_MODE	2
#define TRAFFIC_BIDIR_ACTIVE_MODE 3
#define TRAFFIC_BIDIR_IDLE_MODE 4


/* ============================================================= */
/*      Common definition */
/* ============================================================= */
#define MBSS_VLAN_INFO_GET(__pAd, __VLAN_VID, __VLAN_Priority, __func_idx) \
	{																		\
		if ((__func_idx < __pAd->ApCfg.BssidNum) &&					\
			(__func_idx < HW_BEACON_MAX_NUM) &&						\
			(__pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_VID != 0)) {			\
				__VLAN_VID = __pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_VID;	\
				__VLAN_Priority = __pAd->ApCfg.MBSSID[__func_idx].wdev.VLAN_Priority; \
		}																	\
	}

#define WDEV_VLAN_INFO_GET(__pAd, __VLAN_VID, __VLAN_Priority, __wdev) \
	{																		\
		if ((__wdev->VLAN_VID != 0)) {			\
			__VLAN_VID = __wdev->VLAN_VID;	\
			__VLAN_Priority = __wdev->VLAN_Priority; \
		}																	\
	}

#ifdef CUSTOMER_RSG_FEATURE
#define TIMESTAMP_GET(__pAd, __TimeStamp)			\
	{													\
		UINT32 tsf_l = 0, tsf_h = 0; UINT64 __Value64;				\
		AsicGetTsfTime((__pAd), &tsf_h, &tsf_l);\
		__TimeStamp = (UINT64)tsf_l;					\
		__Value64 = (UINT64)tsf_h;						\
		__TimeStamp |= (tsf_h << 32);				\
	}
#endif


#ifndef HOSTAPD_11R_SUPPORT
typedef struct _AUTH_FRAME_INFO {
	UCHAR addr1[MAC_ADDR_LEN];
	UCHAR addr2[MAC_ADDR_LEN];
	USHORT auth_alg;
	USHORT auth_seq;
	USHORT auth_status;
	CHAR Chtxt[CIPHER_TEXT_LEN];
#ifdef DOT11R_FT_SUPPORT
	FT_INFO FtInfo;
#endif /* DOT11R_FT_SUPPORT */
} AUTH_FRAME_INFO;
#endif /* HOSTAPD_11R_SUPPORT */

#ifdef CONVERTER_MODE_SWITCH_SUPPORT
typedef enum _ENUM_AP_START_STATE_T	{
	AP_STATE_ALWAYS_START_AP_DEFAULT = 0,
	AP_STATE_START_AFTER_APCLI_CONNECTION,
	AP_STATE_NEVER_START_AP,
	AP_STATE_INVALID_MAX
} ENUM_AP_START_STATE;

typedef enum _ENUM_APCLI_MODE_T	{
	APCLI_MODE_ALWAYS_START_AP_DEFAULT = 0,
	APCLI_MODE_START_AP_AFTER_APCLI_CONNECTION,
	APCLI_MODE_NEVER_START_AP,
	APCLI_MODE_INVALID_MAX
} ENUM_APCLI_MODE;

#endif /* CONVERTER_MODE_SWITCH_SUPPORT */

typedef enum _ENUM_AP_BSS_OPER_T {
	AP_BSS_OPER_ALL = 0,
	AP_BSS_OPER_BY_RF,
	AP_BSS_OPER_SINGLE,
	AP_BSS_OPER_NUM
} ENUM_AP_BSS_OPER;

/* ============================================================= */
/*      Function Prototypes */
/* ============================================================= */

BOOLEAN APBridgeToWirelessSta(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pHeader,
	IN  UINT            HdrLen,
	IN  PUCHAR          pData,
	IN  UINT            DataLen,
	IN  ULONG           fromwdsidx);

INT ap_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
INT ap_fp_tx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT ap_rx_pkt_allowed(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
INT ap_rx_ps_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk);
BOOLEAN ap_dev_rx_mgmt_frm(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_rx_pkt_foward(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pPacket);
INT ap_mlme_mgmtq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
INT ap_mlme_dataq_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *tx_blk);
INT ap_ieee_802_3_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_ieee_802_11_data_rx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, RX_BLK *pRxBlk, MAC_TABLE_ENTRY *pEntry);
INT ap_conn_act(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT ap_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT ap_link_down(struct wifi_dev *wdev);
INT ap_inf_open(struct wifi_dev *wdev);
INT ap_inf_close(struct wifi_dev *wdev);
INT ap_send_data_pkt(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, PNDIS_PACKET pkt);
INT ap_send_mlme_pkt(RTMP_ADAPTER *pAd, PNDIS_PACKET pkt, struct wifi_dev *wdev, UCHAR q_idx, BOOLEAN is_data_queue);
UINT32 starec_ap_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);

NDIS_STATUS APInsertPsQueue(
	IN RTMP_ADAPTER *pAd,
	IN PNDIS_PACKET pPacket,
	IN STA_TR_ENTRY * tr_entry,
	IN UCHAR QueIdx);

INT ap_tx_pkt_handle(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_legacy_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_ampdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_amsdu_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
INT ap_frag_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_ieee_802_11_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_ieee_802_3_data_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
VOID ap_find_cipher_algorithm(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
BOOLEAN ap_fill_non_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);
BOOLEAN ap_fill_offload_tx_blk(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, TX_BLK *pTxBlk);

VOID rx_eapol_frm_handle(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN RX_BLK *pRxBlk,
	IN UCHAR wdev_idx);

VOID ap_rx_error_handle(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
BOOLEAN ap_chk_cl2_cl3_err(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

VOID RTMPDescriptorEndianChange(UCHAR *pData, ULONG DescriptorType);

VOID RTMPFrameEndianChange(
	IN  RTMP_ADAPTER *pAd,
	IN  UCHAR *pData,
	IN  ULONG Dir,
	IN  BOOLEAN FromRxDoneInt);

/* ap_assoc.c */

VOID APAssocStateMachineInit(
	IN  PRTMP_ADAPTER   pAd,
	IN  STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID MbssKickOutStas(RTMP_ADAPTER *pAd, INT apidx, USHORT Reason);
VOID APMlmeKickOutSta(RTMP_ADAPTER *pAd, UCHAR *staAddr, UCHAR Wcid, USHORT Reason);

#ifdef BW_VENDOR10_CUSTOM_FEATURE
BOOLEAN IsClientConnected(RTMP_ADAPTER *pAd);
#endif


#ifdef DOT11W_PMF_SUPPORT
VOID APMlmeKickOutAllSta(RTMP_ADAPTER *pAd, UCHAR apidx, USHORT Reason);
#endif /* DOT11W_PMF_SUPPORT */

VOID  APCls3errAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

/* ap_auth.c */

void APAuthStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID APCls2errAction(RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);

#ifdef HOSTAPD_11R_SUPPORT
/*for ap_assoc in cfg mode*/
BOOLEAN PeerAssocReqCmmSanity(
	RTMP_ADAPTER *pAd,
	BOOLEAN isReassoc,
	VOID *Msg,
	INT MsgLen,
	IE_LISTS * ie_lists);

USHORT APBuildAssociation(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN IE_LISTS * ie_list,
	IN UCHAR MaxSupportedRateIn500Kbps,
	OUT USHORT *pAid,
	IN BOOLEAN isReassoc);
#endif /* HOSTAPD_11R_SUPPORT */
/* ap_connect.c */

#ifdef CONFIG_AP_SUPPORT
#ifdef MT_MAC
VOID APMakeAllTimFrame(RTMP_ADAPTER *pAd);
VOID APMakeTimFrame(RTMP_ADAPTER *pAd, INT apidx);
VOID APCheckBcnQHandler(RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int);
#endif
#endif /* CONFIG_AP_SUPPORT */

/* ap_sync.c */
VOID APSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID APScanTimeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ApSiteSurvey(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel);

VOID ApSiteSurvey_by_wdev(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel,
	IN  struct wifi_dev	*wdev);

#ifdef TXRX_STAT_SUPPORT
VOID Update_LastSec_TXRX_Stats(
	IN PRTMP_ADAPTER   pAd);
#endif
#if defined(CUSTOMER_RSG_FEATURE) || defined (CUSTOMER_DCC_FEATURE)
VOID Update_Wtbl_Counters(
	IN PRTMP_ADAPTER   pAd);

#endif
#ifdef CUSTOMER_RSG_FEATURE
VOID UpdateRadioStatCounters(
	IN PRTMP_ADAPTER   	pAd);

VOID ClearChannelStatsCr(
	IN PRTMP_ADAPTER  	pAd);

VOID ResetChannelStats(
	IN PRTMP_ADAPTER 	pAd);

#endif
#ifdef CUSTOMER_DCC_FEATURE
VOID GetTxRxActivityTime(
	IN PRTMP_ADAPTER   pAd,
	IN UINT wcid);
VOID RemoveOldStaList(
	IN PRTMP_ADAPTER 	pAd);

VOID APResetStreamingStatus(
	IN PRTMP_ADAPTER  	pAd);
#endif

#if defined(CUSTOMER_DCC_FEATURE) || defined(CONFIG_MAP_SUPPORT)
VOID RemoveOldBssEntry(
	IN PRTMP_ADAPTER 	pAd);
#endif

#ifdef APCLI_CFG80211_SUPPORT
VOID ApCliSiteSurvey(
	IN	PRTMP_ADAPTER		pAd,
	IN	PNDIS_802_11_SSID	pSsid,
	IN	UCHAR			ScanType,
	IN	BOOLEAN			ChannelSel,
	IN	struct wifi_dev		*wdev);
#endif /* APCLI_CFG80211_SUPPORT */

VOID SupportRate(
	IN PUCHAR SupRate,
	IN UCHAR SupRateLen,
	IN PUCHAR ExtRate,
	IN UCHAR ExtRateLen,
	OUT PUCHAR * Rates,
	OUT PUCHAR RatesLen,
	OUT PUCHAR pMaxSupportRate);


BOOLEAN ApScanRunning(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef OFFCHANNEL_SCAN_FEATURE
UCHAR Channel2Index(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			channel);

INT ApSiteSurveyNew_by_wdev(
	IN	PRTMP_ADAPTER	pAd,
	IN	UINT			Channel,
	IN UINT			Timeout,
	IN	UCHAR			ScanType,
	IN	BOOLEAN			ChannelSel,
	IN  struct wifi_dev *wdev);
#endif


#ifdef DOT11_N_SUPPORT
VOID APUpdateOperationMode(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

#ifdef DOT11N_DRAFT3
VOID APOverlappingBSSScan(RTMP_ADAPTER *pAd, struct wifi_dev *wdev);

INT GetBssCoexEffectedChRange(
	IN RTMP_ADAPTER *pAd,
	IN struct wifi_dev *wdev,
	IN BSS_COEX_CH_RANGE * pCoexChRange,
	IN UCHAR Channel);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */


/* ap_mlme.c */
VOID APMlmePeriodicExec(RTMP_ADAPTER *pAd);

BOOLEAN APMsgTypeSubst(
	IN PRTMP_ADAPTER pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

VOID APQuickResponeForRateUpExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID APAsicEvaluateRxAnt(RTMP_ADAPTER *pAd);
VOID APAsicRxAntEvalTimeout(RTMP_ADAPTER *pAd);

/* ap.c */
UCHAR get_apidx_by_addr(RTMP_ADAPTER *pAd, UCHAR *addr);

NDIS_STATUS APOneShotSettingInitialize(RTMP_ADAPTER *pAd);
#ifdef CONFIG_INIT_RADIO_ONOFF
VOID APStartUpForMain(RTMP_ADAPTER *pAd);
#endif

/* INT ap_func_init(RTMP_ADAPTER *pAd); */

VOID APShutdown(RTMP_ADAPTER *pAd);

VOID APStartUpForMbss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);
VOID APStartUp(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper);
VOID APStartUpByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);

VOID APStop(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss, ENUM_AP_BSS_OPER oper);
VOID APStopByBss(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);

VOID APCleanupPsQueue(RTMP_ADAPTER *pAd, QUEUE_HEADER *pQueue);


VOID MacTableMaintenance(RTMP_ADAPTER *pAd);

UINT32 MacTableAssocStaNumGet(RTMP_ADAPTER *pAd);

MAC_TABLE_ENTRY *APSsPsInquiry(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pAddr,
	OUT SST * Sst,
	OUT USHORT          *Aid,
	OUT UCHAR           *PsMode,
	OUT UCHAR           *Rate);

#ifdef SYSTEM_LOG_SUPPORT
VOID ApLogEvent(
	IN PRTMP_ADAPTER    pAd,
	IN PUCHAR           pAddr,
	IN USHORT           Event);
#else
#define ApLogEvent(_pAd, _pAddr, _Event)
#endif /* SYSTEM_LOG_SUPPORT */


VOID ApUpdateCapabilityAndErpIe(RTMP_ADAPTER *pAd, struct _BSS_STRUCT *mbss);

BOOLEAN ApCheckAccessControlList(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR apidx);
VOID ApUpdateAccessControlList(RTMP_ADAPTER *pAd, UCHAR apidx);


#ifdef AP_QLOAD_SUPPORT
VOID QBSS_LoadInit(RTMP_ADAPTER *pAd);
VOID QBSS_LoadAlarmReset(RTMP_ADAPTER *pAd);
VOID QBSS_LoadAlarmResume(RTMP_ADAPTER *pAd);
UINT32 QBSS_LoadBusyTimeGet(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsAlarmIssued(RTMP_ADAPTER *pAd);
BOOLEAN QBSS_LoadIsBusyTimeAccepted(RTMP_ADAPTER *pAd, UINT32 BusyTime);
UINT32 QBSS_LoadElementAppend(RTMP_ADAPTER *pAd, UINT8 *pBeaconBuf, QLOAD_CTRL *pQloadCtrl);
VOID QBSS_LoadUpdate(RTMP_ADAPTER *pAd, ULONG UpTime);
VOID QBSS_LoadStatusClear(RTMP_ADAPTER	*pAd, UCHAR	Channel);


INT	Show_QoSLoad_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#ifdef CONFIG_HOTSPOT_R2
UINT32 QBSS_LoadElementAppend_HSTEST(RTMP_ADAPTER *pAd, UINT8 *pBeaconBuf, UCHAR apidx);
#endif /* CONFIG_HOTSPOT_R2 */
#endif /* AP_QLOAD_SUPPORT */


#ifdef DOT1X_SUPPORT
INT	Set_OwnIPAddr_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_EAPIfName_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT	Set_PreAuthIfName_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/* Define in ap.c */
BOOLEAN DOT1X_InternalCmdAction(
	IN RTMP_ADAPTER *pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN UINT8 cmd);

BOOLEAN DOT1X_EapTriggerAction(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry);
#endif /* DOT1X_SUPPORT */

VOID AP_E2PROM_IOCTL_PostCtrl(RTMP_IOCTL_INPUT_STRUCT *wrq, RTMP_STRING *msg);

VOID IAPP_L2_UpdatePostCtrl(RTMP_ADAPTER *pAd, UINT8 *mac, INT wdev_idx);

INT rtmp_ap_init(RTMP_ADAPTER *pAd);
VOID rtmp_ap_exit(RTMP_ADAPTER *pAd);

#if defined(VOW_SUPPORT) && defined(VOW_DVT)
UINT32 vow_clone_legacy_frame(RTMP_ADAPTER *pAd, TX_BLK *pTxBlk);
#endif
VOID ap_over_lapping_scan(RTMP_ADAPTER *pAd, BSS_STRUCT *pMbss);


#endif  /* __AP_H__ */

