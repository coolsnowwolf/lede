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
    ap_apcli.h

    Abstract:
    Support AP-Client function.

    Revision History:
    Who               When            What
    --------------    ----------      ----------------------------------------------
    Shiang, Fonchi    02-13-2007      created
*/

#ifndef _AP_APCLI_H_
#define _AP_APCLI_H_

#ifdef APCLI_SUPPORT

#include "rtmp.h"

#define PROBE_TIMEOUT	1000        /* unit: msec */

#define TOLERANCE_OF_TP_THRESHOLD 50
#define TP_PEEK_BOUND_THRESHOLD	830
/* #define BYTES_PER_SEC_TO_MBPS	17 */
/* #define TX_MODE_RATIO_THRESHOLD	70 */
#define TX_MODE_TP_CHECK	250
#define NUM_OF_APCLI_TXOP_COND 5

#ifdef APCLI_CONNECTION_TRIAL
#define TRIAL_TIMEOUT	400	/* unit: msec */
#endif /* APCLI_CONNECTION_TRIAL */
#define APCLI_WAIT_TIMEOUT RTMPMsecsToJiffies(300)
#define REPT_WAIT_TIMEOUT RTMPMsecsToJiffies(5000)

#define APCLI_ROOT_BSSID_GET(pAd, wcid) ((pAd)->MacTab.Content[(wcid)].Addr)

/* sanity check for apidx */
#define APCLI_MR_APIDX_SANITY_CHECK(idx) \
	{ \
		if ((idx) >= MAX_APCLI_NUM) {	\
			(idx) = 0; \
			MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_ERROR, ("%s> Error! apcli-idx > MAX_APCLI_NUM!\n", __func__)); \
		} \
	}

#ifdef ROAMING_ENHANCE_SUPPORT
#define APCLI_ROAMING_ENHANCE_CHECK(pAd, pEntry, pRxBlk, wdev) \
{\
	if (pAd->ApCfg.bRoamingEnhance && IsApCliLinkUp(pAd)) {\
		if ((pEntry->bRoamingRefreshDone == FALSE) && IS_ENTRY_CLIENT(pEntry))\
			ApCliDoRoamingRefresh(pAd, pEntry, pRxBlk->pRxPacket, wdev);\
	} \
}
#endif /* ROAMING_ENHANCE_SUPPORT */

typedef struct _APCLI_MLME_JOIN_REQ_STRUCT {
	UCHAR	Bssid[MAC_ADDR_LEN];
	UCHAR	SsidLen;
	UCHAR	Ssid[MAX_LEN_OF_SSID];
} APCLI_MLME_JOIN_REQ_STRUCT;

typedef struct _APCLI_CTRL_MSG_STRUCT {
	USHORT Status;
	UCHAR SrcAddr[MAC_ADDR_LEN];
#ifdef MAC_REPEATER_SUPPORT
	UCHAR BssIdx;
	UCHAR CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
} APCLI_CTRL_MSG_STRUCT, *PSTA_CTRL_MSG_STRUCT;

BOOLEAN isValidApCliIf(
	SHORT ifIndex);

VOID ApCliCtrlStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliSyncStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAuthStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAssocStateMachineInit(
	IN PRTMP_ADAPTER pAd,
	IN STATE_MACHINE * Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR wcid,
	IN UCHAR *pAddrs);

BOOLEAN ApCliValidateRSNIE(
	IN RTMP_ADAPTER * pAd,
	IN PEID_STRUCT pEid_ptr,
	IN USHORT eid_len,
	IN USHORT idx,
	IN UCHAR Privacy);

VOID ApCli_Remove(
	IN PRTMP_ADAPTER	pAd);

VOID RT28xx_ApCli_Close(
	IN PRTMP_ADAPTER	pAd);

INT ApCliIfLookUp(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pAddr);

VOID ApCliMgtMacHeaderInit(
	IN	PRTMP_ADAPTER	pAd,
	IN OUT PHEADER_802_11 pHdr80211,
	IN UCHAR SubType,
	IN UCHAR ToDs,
	IN PUCHAR pDA,
	IN PUCHAR pBssid,
	IN USHORT ifIndex);

#ifdef DOT11_N_SUPPORT
BOOLEAN ApCliCheckHt(
	IN		PRTMP_ADAPTER		pAd,
	IN		USHORT				IfIndex,
	IN OUT	HT_CAPABILITY_IE * pHtCapability,
	IN OUT	ADD_HT_INFO_IE * pAddHtInfo);
#endif /* DOT11_N_SUPPORT */

#ifdef APCLI_CERT_SUPPORT
void ApCliCertEDCAAdjust(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PEDCA_PARM pEdcaParm);
#endif

BOOLEAN ApCliLinkUp(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID ApCliLinkDown(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID ApCliIfUp(
	IN PRTMP_ADAPTER pAd);

VOID ApCliIfDown(
	IN PRTMP_ADAPTER pAd);

VOID ApCliIfMonitor(
	IN PRTMP_ADAPTER pAd);

BOOLEAN ApCliMsgTypeSubst(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN preCheckMsgTypeSubset(
	IN PRTMP_ADAPTER  pAd,
	IN PFRAME_802_11 pFrame,
	OUT INT *Machine,
	OUT INT *MsgType);

BOOLEAN ApCliPeerAssocRspSanity(
	IN PRTMP_ADAPTER pAd,
	IN VOID *pMsg,
	IN ULONG MsgLen,
	OUT PUCHAR pAddr2,
	OUT USHORT *pCapabilityInfo,
	OUT USHORT *pStatus,
	OUT USHORT *pAid,
	OUT UCHAR SupRate[],
	OUT UCHAR *pSupRateLen,
	OUT UCHAR ExtRate[],
	OUT UCHAR *pExtRateLen,
	OUT HT_CAPABILITY_IE * pHtCapability,
	OUT ADD_HT_INFO_IE * pAddHtInfo,	/* AP might use this additional ht info IE */
	OUT UCHAR *pHtCapabilityLen,
	OUT UCHAR *pAddHtInfoLen,
	OUT UCHAR *pNewExtChannelOffset,
	OUT PEDCA_PARM pEdcaParm,
	OUT UCHAR *pCkipFlag,
	OUT IE_LISTS * le_list);

VOID	ApCliPeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM * Elem);

VOID	ApCliPeerPairMsg3Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM * Elem);

VOID	ApCliPeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN MLME_QUEUE_ELEM * Elem);

BOOLEAN ApCliCheckRSNIE(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pData,
	IN  UCHAR           DataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	OUT	UCHAR			*Offset);

BOOLEAN ApCliParseKeyData(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKeyData,
	IN  UCHAR           KeyDataLen,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			IfIdx,
	IN	UCHAR			bPairewise);

INT apcli_tx_pkt_allowed(
	RTMP_ADAPTER * pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pkt);

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK * pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID ApCliUpdateMlmeRate(RTMP_ADAPTER *pAd, USHORT ifIndex);

VOID ApCliCheckPeerExistence(
	IN RTMP_ADAPTER *pAd,
	IN CHAR * Ssid,
	IN UCHAR SsidLen,
	IN UCHAR Channel);

VOID ApCliPeerCsaAction(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BCN_IE_LIST *ie_list);

VOID APCli_Init(
	IN RTMP_ADAPTER *pAd,
	IN RTMP_OS_NETDEV_OP_HOOK * pNetDevOps);

BOOLEAN ApCli_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
BOOLEAN ApCli_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p);

BOOLEAN ApCliWaitProbRsp(RTMP_ADAPTER *pAd, USHORT ifIndex);
VOID ApCliSimulateRecvBeacon(RTMP_ADAPTER *pAd);

#ifdef APCLI_AUTO_CONNECT_SUPPORT
extern INT Set_ApCli_Enable_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

extern INT Set_ApCli_Bssid_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd,
	IN struct wifi_dev *wdev);

VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev);

#ifdef APCLI_AUTO_BW_TMP /* should be removed after apcli auto-bw is applied */
BOOLEAN ApCliAutoConnectBWAdjust(
	IN RTMP_ADAPTER	*pAd,
	IN struct wifi_dev	*wdev,
	IN BSS_ENTRY * bss_entry);
#endif /* APCLI_AUTO_BW_TMP */
#endif /* APCLI_AUTO_CONNECT_SUPPORT */

#ifdef DOT11W_PMF_SUPPORT
INT Set_ApCliPMFMFPC_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT Set_ApCliPMFMFPR_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);

INT Set_ApCliPMFSHA256_Proc(
	IN PRTMP_ADAPTER pAd,
	IN	RTMP_STRING * arg);
#endif /* DOT11W_PMF_SUPPORT */

VOID ApCliRTMPReportMicError(
	IN RTMP_ADAPTER	*pAd,
	IN UCHAR uniCastKey,
	IN INT	ifIndex);

VOID apcli_dync_txop_alg(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	UINT tx_tp,
	UINT rx_tp);

INT apcli_phy_rrm_init_byRf(RTMP_ADAPTER *pAd, UCHAR RfIC);
INT apcli_link_up(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry);
INT apcli_inf_open(struct wifi_dev *wdev);
INT apcli_inf_close(struct wifi_dev *wdev);


VOID ApCliMlmeDeauthReqAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem); 



#endif /* APCLI_SUPPORT */

#endif /* _AP_APCLI_H_ */

