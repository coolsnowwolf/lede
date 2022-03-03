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
  
#define AUTH_TIMEOUT	300         /* unit: msec */
#define ASSOC_TIMEOUT	300         /* unit: msec */
/*#define JOIN_TIMEOUT	2000        // unit: msec // not used in Ap-client mode, remove it */
#define PROBE_TIMEOUT	1000        /* unit: msec */
#ifdef APCLI_CONNECTION_TRIAL
#define TRIAL_TIMEOUT	400	/* unit: msec */
#endif /* APCLI_CONNECTION_TRIAL */
#define OPENWEP_ERRPKT_MAX_COUNT  	  3

  
#define APCLI_ROOT_BSSID_GET(pAd, wcid) ((pAd)->MacTab.Content[(wcid)].Addr)

/* sanity check for apidx */
#define APCLI_MR_APIDX_SANITY_CHECK(idx) \
{ \
	if ((idx) >= MAX_APCLI_NUM) \
	{ \
		(idx) = 0; \
		DBGPRINT(RT_DEBUG_ERROR, ("%s> Error! apcli-idx > MAX_APCLI_NUM!\n", __FUNCTION__)); \
	} \
}

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
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliSyncStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAuthStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID ApCliAssocStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

MAC_TABLE_ENTRY *ApCliTableLookUpByWcid(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR wcid,
	IN UCHAR *pAddrs);
	
BOOLEAN 	ApCliValidateRSNIE(
	IN RTMP_ADAPTER *pAd, 
	IN 		PEID_STRUCT    	pEid_ptr,
	IN		USHORT			eid_len,
	IN		USHORT			idx);


VOID ApCli_Remove(
	IN PRTMP_ADAPTER 	pAd);

VOID RT28xx_ApCli_Close(
	IN PRTMP_ADAPTER 	pAd);



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
	IN		PRTMP_ADAPTER 		pAd,
	IN		USHORT 				IfIndex,
	IN OUT	HT_CAPABILITY_IE 	*pHtCapability,
	IN OUT	ADD_HT_INFO_IE 		*pAddHtInfo);
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
BOOLEAN ApCliCheckVht(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN MAC_TABLE_ENTRY  *pEntry,
	IN VHT_CAP_IE *vht_cap,
	IN VHT_OP_IE *vht_op);
#endif /* DOT11_VHT_AC */
	
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
    OUT HT_CAPABILITY_IE *pHtCapability,
    OUT ADD_HT_INFO_IE *pAddHtInfo,	/* AP might use this additional ht info IE */
    OUT UCHAR *pHtCapabilityLen,
    OUT UCHAR *pAddHtInfoLen,
    OUT UCHAR *pNewExtChannelOffset,
    OUT PEDCA_PARM pEdcaParm,
    OUT UCHAR *pCkipFlag,
    OUT IE_LISTS *ie_list);

VOID	ApCliPeerPairMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerPairMsg3Action(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

VOID	ApCliPeerGroupMsg1Action(
	IN PRTMP_ADAPTER    pAd, 
    IN MAC_TABLE_ENTRY  *pEntry,
    IN MLME_QUEUE_ELEM  *Elem);

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

BOOLEAN  ApCliHandleRxBroadcastFrame(
	IN  PRTMP_ADAPTER   pAd,
	IN	RX_BLK			*pRxBlk,
	IN  MAC_TABLE_ENTRY *pEntry,
	IN	UCHAR			FromWhichBSSID);

VOID APCliInstallPairwiseKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  MAC_TABLE_ENTRY *pEntry);

BOOLEAN APCliInstallSharedKey(
	IN  PRTMP_ADAPTER   pAd,
	IN  PUCHAR          pKey,
	IN  UCHAR           KeyLen,
	IN	UCHAR			DefaultKeyIdx,
	IN  MAC_TABLE_ENTRY *pEntry);

VOID ApCliCheckPeerExistence(
	IN RTMP_ADAPTER *pAd,
	IN CHAR *Ssid,
	IN UCHAR SsidLen,
	IN UCHAR Channel);

VOID ApCliUpdateMlmeRate(RTMP_ADAPTER *pAd, USHORT ifIndex);

VOID APCli_Init(
	IN RTMP_ADAPTER *pAd,
	IN	RTMP_OS_NETDEV_OP_HOOK		*pNetDevOps);

BOOLEAN ApCli_Open(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
BOOLEAN ApCli_Close(RTMP_ADAPTER *pAd, PNET_DEV dev_p);
BOOLEAN ApCli_StatsGet(RTMP_ADAPTER *pAd, RT_CMD_STATS64 *pStats);
BOOLEAN ApCliWaitProbRsp(RTMP_ADAPTER *pAd, USHORT ifIndex);
VOID ApCliSimulateRecvBeacon(RTMP_ADAPTER *pAd);
#ifdef APCLI_AUTO_BW_SUPPORT
BOOLEAN ApCliAutoBwAction(PRTMP_ADAPTER pAd, USHORT ifIndex);
BOOLEAN ApCliSetPhyMode(PRTMP_ADAPTER pAd, USHORT ifIndex, UINT wmode);
INT Set_ApCli_Bw_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
INT Set_ApCli_PhyMode_Proc(PRTMP_ADAPTER pAd, PSTRING arg);
#endif /* APCLI_AUTO_BW_SUPPORT */

#ifdef APCLI_AUTO_CONNECT_SUPPORT
BOOLEAN ApCliSetIfState(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN state);

BOOLEAN ApCliSetBssid(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN UCHAR *Bssid);

BOOLEAN ApCliAutoConnectStart(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

BOOLEAN ApCliAutoConnectExec(
	IN  PRTMP_ADAPTER   pAd);

BOOLEAN ApcliCompareAuthEncryp(
	IN PAPCLI_STRUCT					pApCliEntry,
	IN NDIS_802_11_AUTHENTICATION_MODE	AuthMode,
	IN NDIS_802_11_AUTHENTICATION_MODE	AuthModeAux,
	IN NDIS_802_11_WEP_STATUS			WEPstatus,
	IN CIPHER_SUITE						WPA);

VOID ApCliSwitchCandidateAP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex);

VOID RTMPApCliReconnectionCheck(
	IN PRTMP_ADAPTER pAd);

#endif /* APCLI_AUTO_CONNECT_SUPPORT */
VOID ApCliRxOpenWEPCheck(
	IN RTMP_ADAPTER *pAd,
	IN RX_BLK *pRxBlk,
	IN BOOLEAN bSuccessPkt);

VOID ApCliSiteSurvey(
	IN	RTMP_ADAPTER  		*pAd,
	IN	UCHAR			ifIndex,
	IN	NDIS_802_11_SSID	*pSsid,
	IN	UCHAR				ScanType,
	IN	BOOLEAN				ChannelSel);
#endif /* APCLI_SUPPORT */
#endif /* _AP_APCLI_H_ */

