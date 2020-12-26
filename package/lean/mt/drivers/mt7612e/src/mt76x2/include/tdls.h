/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#ifndef __TDLS_H
#define __TDLS_H

#include "dot11z_tdls.h"
//#include "dot11r_ft.h"

#define LENGTH_TDLS_H				24
#define LENGTH_TDLS_PAYLOAD_H		3  /* payload type(1 byte) + category(1 byte) + action(1 byte) */
#define TDLS_TIMEOUT				5000	// unit: msec
#define	TDLS_DISCOVERY_TRY_COUNT	2
#define TDLS_AUTO_DISCOVERY_INTERVAL	600		/* unit: msec */
#define TDLS_RSSI_MEASUREMENT_PERIOD	10		/* unit: sec */
#define TDLS_AUTO_DISCOVERY_PERIOD		120		/* unit: sec */
#define TDLS_DISABLE_PERIOD_BY_TEARDOWN	120		/* unit: sec */
#define TDLS_AUTO_SETUP_RSSI_THRESHOLD	-70		/* unit: dbm */
#define TDLS_AUTO_TEARDOWN_RSSI_THRESHOLD	-75 /* unit: dbm */

#define TDLS_AUTO_DISCOVERY_TRY_COUNT	2

extern UCHAR CipherSuiteTDLSWpa2PskAes[];
extern UCHAR CipherSuiteTDLSLen;

#define IS_TDLS_SUPPORT(_P) \
	((_P)->StaCfg.TdlsInfo.bTDLSCapable == TRUE)

// TDLS State
typedef enum _TDLS_STATE {
	TDLS_MODE_NONE,				/* Init state */
	TDLS_MODE_WAIT_RESPONSE,		/* Wait a response from the Responder */
	TDLS_MODE_WAIT_CONFIRM,			/* Wait an confirm from the Initiator */
	TDLS_MODE_CONNECTED,		/* Tunneled Direct Link estabilished */
/*	TDLS_MODE_SWITCH_CHANNEL, */
/*	TDLS_MODE_PSM, */
/*	TDLS_MODE_UAPSD */
} TDLS_STATE;

// TDLS State
typedef enum _TDLS_CHANNEL_SWITCH_STATE {
	TDLS_CHANNEL_SWITCH_NONE,					// Init state
	TDLS_CHANNEL_SWITCH_WAIT_RSP,		// Wait a response from the Responder
	TDLS_CHANNEL_SWITCH_DONE,			// Channel Switch Finish
} TDLS_CHANNEL_SWITCH_STATE;

typedef struct _MLME_TDLS_REQ_STRUCT {
	PRT_802_11_TDLS	pTDLS;
	USHORT			Reason;
	UCHAR			Action;
	BOOLEAN			IsViaAP;
} MLME_TDLS_REQ_STRUCT, *PMLME_TDLS_REQ_STRUCT;

typedef struct _MLME_TDLS_DISCOVERY_STRUCT {
	UCHAR	BSSID[MAC_ADDR_LEN];
	UCHAR	InitiatorAddr[MAC_ADDR_LEN];
	UCHAR	ResponderAddr[MAC_ADDR_LEN];
	UINT8	DialogToken;
} MLME_TDLS_DISCOVERY_STRUCT, *PMLME_TDLS_DISCOVERY_STRUCT;

typedef struct _MLME_TDLS_CH_SWITCH_STRUCT {
	UCHAR	PeerMacAddr[MAC_ADDR_LEN];
	//UINT8	DialogToken;
	UINT8	TargetChannel;
	UINT8	TargetChannelBW;
} MLME_TDLS_CH_SWITCH_STRUCT, *PMLME_TDLS_CH_SWITCH_STRUCT;

#define TDLS_DISCOVERY_PEER_ENTRY_SIZE	32
typedef struct _TDLS_DISCOVERY_ENTRY {
	struct _TDLS_DISCOVERY_ENTRY *pNext;
	ULONG	InitRefTime;
	UCHAR	Responder[MAC_ADDR_LEN];
	UCHAR	RetryCount;
	UCHAR	DialogToken;
	TDLS_CTRL_STATE	CurrentState;
	CHAR	AvgRssi0;
	BOOLEAN bTDLSCapable;
	BOOLEAN bConnected;
	BOOLEAN bFirstTime;
	BOOLEAN bConnectedFirstTime;
} TDLS_DISCOVERY_ENTRY, *PTDLS_DISCOVERY_ENTRY;

typedef struct _TDLS_BLACK_ENTRY {
	struct _TDLS_BLACK_ENTRY *pNext;
	ULONG	InitRefTime;
	UCHAR	MacAddr[MAC_ADDR_LEN];
	UCHAR	CurrentState;
} TDLS_BLACK_ENTRY, *PTDLS_BLACK_ENTRY;

VOID
TDLS_Table_Init(
	IN	PRTMP_ADAPTER	pAd);

VOID
TDLS_Table_Destory(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_SearchTabMaintain(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_StateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID
TDLS_ChSwStateMachineInit(
    IN PRTMP_ADAPTER pAd, 
    IN STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);

VOID
TDLS_CntlOidTDLSRequestProc(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

INT
TDLS_SearchLinkId(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR	pAddr);

VOID
TDLS_MlmeParmFill(
	IN PRTMP_ADAPTER pAd, 
	IN OUT MLME_TDLS_REQ_STRUCT *pTdlsReq,
	IN PRT_802_11_TDLS pTdls,
	IN USHORT Reason,
	IN BOOLEAN IsViaAP);

INT	Set_TdlsCapable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

INT	Set_TdlsSetup_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_TdlsTearDown_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

INT
Set_TdlsDiscoveryReq_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

#ifdef WFD_SUPPORT
INT Set_TdlsTunneledReqProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);
#endif /* WFD_SUPPORT */

INT Set_TdlsAcceptWeakSecurityProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);	

INT
Set_TdlsTPKLifeTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

#ifdef TDLS_AUTOLINK_SUPPORT
INT
Set_TdlsAutoLinkProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT
Set_TdlsRssiMeasurementPeriodProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT
Set_TdlsAutoDiscoveryPeriodProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT
Set_TdlsAutoSetupRssiThresholdProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT
Set_TdlsDisabledPeriodByTeardownProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT
Set_TdlsAutoTeardownRssiThresholdProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);
#endif /* TDLS_AUTOLINK_SUPPORT */

INT	Set_TdlsSendHwNullFrameProc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING	arg);

INT	Set_TdlsChannelSwitch_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING	arg);

INT	Set_TdlsChannelSwitchBW_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg);

INT	Set_TdlsChannelSwitchDisable_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg);

INT	Set_TdlsManualInsEntry_Proc(
	IN	PRTMP_ADAPTER pAd, 
	IN	PSTRING arg);

NDIS_STATUS
TDLS_SetupRequestAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS);

NDIS_STATUS
TDLS_SetupResponseAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

NDIS_STATUS
TDLS_SetupConfirmAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UCHAR	RsnLen,
	IN PUCHAR	pRsnIe,
	IN UCHAR	FTLen,
	IN PUCHAR	pFTIe,
	IN UCHAR	TILen,
	IN PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

NDIS_STATUS
TDLS_DiscoveryReqAction(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr);

#ifdef WFD_SUPPORT
NDIS_STATUS
TDLS_TunneledProbeRequest(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr);

NDIS_STATUS
TDLS_TunneledProbeResponse(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMacAddr);
#endif /* WFD_SUPPORT */

NDIS_STATUS
TDLS_DiscoveryRspAction(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR	PeerToken,
	IN PUCHAR	pPeerMac);

VOID
TDLS_DiscoveryRspPublicAction(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN VOID		*Msg, 
	IN ULONG	MsgLen);

NDIS_STATUS
TDLS_ChannelSwitchReqAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS pTDLS,
	IN PUCHAR pPeerAddr,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW);

NDIS_STATUS
TDLS_ChannelSwitchRspAction(
	IN	PRTMP_ADAPTER	pAd,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	StatusCode,
	IN	UCHAR	FrameType);

VOID
TDLS_TriggerChannelSwitchAction(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR DtimCount);

#ifdef TDLS_AUTOLINK_SUPPORT
VOID
TDLS_ClearEntryList(
	IN  PLIST_HEADER	pTdlsEnList);

PTDLS_DISCOVERY_ENTRY
TDLS_FindDiscoveryEntry(
	IN	PLIST_HEADER		pTdlsEnList,
	IN	PUCHAR			pMacAddr);

BOOLEAN
TDLS_InsertDiscoveryPeerEntryByMAC(
	IN	PLIST_HEADER pTdlsEnList,
	IN	PUCHAR pMacAddr,
	IN	BOOLEAN bConnected);

VOID
TDLS_DelDiscoveryEntryByMAC(
	IN	PLIST_HEADER		pTdlsEnList,
	IN  PUCHAR			pMacAddr);

VOID
TDLS_MaintainDiscoveryEntryList(
	IN	PRTMP_ADAPTER	pAd);

PTDLS_BLACK_ENTRY
TDLS_FindBlackEntry(
	IN	PLIST_HEADER	pTdlsEnList,
	IN	PUCHAR			pMacAddr);

VOID
TDLS_InsertBlackEntryByMAC(
	IN	PLIST_HEADER	pTdlsEnList,
	IN	PUCHAR		pMacAddr,
	IN	UCHAR	CurrentState);

VOID
TDLS_DelBlackEntryByMAC(
	IN	PLIST_HEADER		pTdlsEnList,
	IN  PUCHAR			pMacAddr);

VOID
TDLS_MaintainBlackList(
	IN	PRTMP_ADAPTER	pAd,
	IN	PLIST_HEADER	pTdlsBlackenList);
#endif // TDLS_AUTOLINK_SUPPORT //

VOID
TDLS_InitPeerEntryRateCapability(
	IN	PRTMP_ADAPTER pAd,
	IN	MAC_TABLE_ENTRY *pEntry,
	IN USHORT *pCapabilityInfo,
	IN UCHAR SupportRateLens,
	IN UCHAR *pSupportRates,
	IN UCHAR HtCapabilityLen,
	IN HT_CAPABILITY_IE *pHtCapability);

VOID
TDLS_BuildSetupRequest(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS);

VOID
TDLS_BuildSetupResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

VOID
TDLS_BuildSetupConfirm(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	UCHAR	RsnLen,
	IN	PUCHAR	pRsnIe,
	IN	UCHAR	FTLen,
	IN	PUCHAR	pFTIe,
	IN	UCHAR	TILen,
	IN	PUCHAR	pTIIe,
	IN	UINT16	StatusCode);

VOID
TDLS_BuildTeardown(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS,
	IN	UINT16	ReasonCode);

VOID
TDLS_BuildChannelSwitchRequest(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUCHAR pPeerAddr,
	IN USHORT ChSwitchTime,
	IN USHORT ChSwitchTimeOut,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW);

VOID
TDLS_BuildChannelSwitchResponse(
	IN	PRTMP_ADAPTER	pAd,
	OUT PUCHAR	pFrameBuf,
	OUT PULONG	pFrameLen,
	IN	PRT_802_11_TDLS	pTDLS,
	IN	USHORT	ChSwitchTime,
	IN	USHORT	ChSwitchTimeOut,
	IN	UINT16	ReasonCode);

VOID
TDLS_InsertActField(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	Category,
	IN	UINT8	ActCode);

VOID
TDLS_InsertStatusCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	StatusCode);

VOID
TDLS_InsertReasonCode(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT16	ReasonCode);

VOID
TDLS_InsertDialogToken(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	DialogToken);

VOID
TDLS_InsertLinkIdentifierIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	PUCHAR	pInitAddr,
	IN	PUCHAR	pPeerAddr);

VOID
TDLS_InsertCapIE(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertSSIDIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertSupportRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertCountryIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertSupportChannelIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertExtRateIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertQosCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertEDCAParameterSetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN PRT_802_11_TDLS	pTDLS);

VOID
TDLS_InsertWMMIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	BOOLEAN	bEnable);

VOID
TDLS_InsertWMMParameterIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

#ifdef DOT11_N_SUPPORT
VOID
TDLS_InsertHtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

#ifdef DOT11N_DRAFT3
VOID
TDLS_InsertBSSCoexistenceIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);
#endif /* DOT11N_DRAFT3 */
#endif /* DOT11_N_SUPPORT */

VOID
TDLS_InsertExtCapIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertFTIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce);

VOID
TDLS_InsertTimeoutIntervalIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue);

VOID
TDLS_InsertTargetChannel(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	TargetCh);

VOID
TDLS_InsertRegulatoryClass(
	IN	PRTMP_ADAPTER	pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UINT8	TargetCh,
	IN	UINT8	ChWidth);

VOID
TDLS_InsertSecondaryChOffsetIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	UCHAR	ChOffset);

VOID
TDLS_SupportedRegulatoryClasses(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen);

VOID
TDLS_InsertChannelSwitchTimingIE(
	IN	PRTMP_ADAPTER pAd,
	OUT	PUCHAR	pFrameBuf,
	OUT	PULONG	pFrameLen,
	IN	USHORT	SwitchTime,
	IN	USHORT	SwitchTimeOut);

UCHAR
TDLS_GetRegulatoryClass(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR	ChannelWidth,
	IN UCHAR	TargetChannel);

BOOLEAN
TDLS_IsValidChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR channel);

UCHAR
TDLS_GetExtCh(
	IN UCHAR Channel,
	IN UCHAR Direction);

VOID
TDLS_InsertPuBufferStatus(
	IN	PRTMP_ADAPTER				pAd,
	OUT PUCHAR						pFrameBuf,
	OUT PULONG						pFrameLen,
	IN	UCHAR						*pPeerMac);

VOID
TDLS_PeerSetupReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerSetupRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerSetupConfAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_MlmeSetupReqAction(
	IN  PRTMP_ADAPTER   pAd,
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_MlmeTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerTearDownAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

#ifdef WFD_SUPPORT
VOID TDLS_MlmeTunneledReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID TDLS_PeerTunneledReqRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);
#endif /* WFD_SUPPORT */

VOID
TDLS_MlmeDiscoveryReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerDiscoveryReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_MlmeChannelSwitchAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_MlmeChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerChannelSwitchReqAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_PeerChannelSwitchRspAction(
    IN PRTMP_ADAPTER pAd, 
    IN MLME_QUEUE_ELEM *Elem);

VOID
TDLS_SendChannelSwitchActionFrame(
    IN  PRTMP_ADAPTER pAd,
    IN  PMAC_TABLE_ENTRY pEntry,
    IN  PUCHAR pHeader802_3,
    IN  UINT HdrLen,
    IN  PUCHAR pData,
    IN  UINT DataLen,
    IN  UCHAR FrameType);

BOOLEAN
TDLS_MsgTypeSubst(
	IN	UCHAR	TDLSActionType,
	OUT	INT		*MsgType);

BOOLEAN
TDLS_CheckTDLSframe(
    IN PRTMP_ADAPTER    pAd,
    IN PUCHAR           pData,
    IN ULONG            DataByteCount);

VOID
TDLS_LinkTearDown(
	IN PRTMP_ADAPTER	pAd,
	IN BOOLEAN	bDirect);

VOID
TDLS_TearDownPeerLink(
	IN PRTMP_ADAPTER	pAd,
	IN PUCHAR	pPeerAddr,
	IN BOOLEAN	bDirect);

VOID
TDLS_ForceSendChannelSwitchResponse(
    IN PRTMP_ADAPTER pAd, 
    IN PRT_802_11_TDLS pTDLS);

BOOLEAN
MlmeTdlsReqSanity(
	IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
    OUT PRT_802_11_TDLS *pTDLS,
    OUT PUINT16 pReason,
    OUT BOOLEAN *pIsViaAP);

ULONG
PeerTdlsBasicSanity(
	IN	PRTMP_ADAPTER	pAd, 
	IN	VOID	*Msg, 
	IN	ULONG	MsgLen,
	IN	BOOLEAN	bInitiator,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA);

BOOLEAN
PeerTdlsSetupReqSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
#ifdef WFD_SUPPORT
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement,
#endif /* WFD_SUPPORT */
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsSetupRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
#ifdef WFD_SUPPORT
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement,
#endif /* WFD_SUPPORT */
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT BOOLEAN *pbWmmCapable,
	OUT UCHAR	*pQosCapability,
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pExtCap,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsSetupConfSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT EDCA_PARM	*pEdcaParm,
	OUT USHORT	*pStatusCode,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsTearDownSanity(
    IN PRTMP_ADAPTER pAd, 
    IN VOID *Msg, 
    IN ULONG MsgLen,
	OUT UCHAR	*pSA,
	OUT	BOOLEAN *pIsInitator,
    OUT USHORT *pReasonCode,
	OUT UCHAR	*pFTLen,    
	OUT UCHAR	FTIe[]);

BOOLEAN
PeerTdlsDiscovReqSanity(
	IN	PRTMP_ADAPTER	pAd, 
	IN	VOID	*Msg, 
	IN	ULONG	MsgLen,
	OUT UCHAR	*pSA,
	OUT UCHAR	*pToken);

BOOLEAN
PeerTdlsDiscovRspSanity(
	IN PRTMP_ADAPTER	pAd, 
	IN VOID		*Msg, 
	IN ULONG		MsgLen,
	OUT UCHAR	*pToken,
	OUT UCHAR	*pSA,
	OUT USHORT	*pCapabilityInfo,
	OUT UCHAR	*pSupRateLen,	
	OUT UCHAR	SupRate[],
	OUT UCHAR	*pExtRateLen,
	OUT UCHAR	ExtRate[],
	OUT UCHAR	*pHtCapLen,
	OUT HT_CAPABILITY_IE	*pHtCap,
	OUT UCHAR	*pTdlsExtCapLen,
	OUT EXT_CAP_INFO_ELEMENT	*pTdlsExtCap,
	OUT UCHAR	*pRsnLen,
	OUT UCHAR	RsnIe[],
	OUT UCHAR	*pFTLen,
	OUT UCHAR	FTIe[],
	OUT UCHAR	*pTILen,
	OUT UCHAR	TIIe[]);

BOOLEAN
PeerTdlsChannelSwitchReqSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen,
	OUT UCHAR *pPeerAddr,
	OUT	BOOLEAN *pIsInitator,
	OUT UCHAR *pTargetChannel,    
	OUT UCHAR *pRegulatoryClass,
	OUT UCHAR *pNewExtChannelOffset,
	OUT USHORT *pChSwitchTime,
	OUT USHORT *pChSwitchTimeOut);

BOOLEAN
PeerTdlsChannelSwitchRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID		*Msg, 
	IN ULONG	MsgLen,
	OUT UCHAR	*pPeerAddr,
	OUT USHORT	*pStatusCode,
	OUT USHORT	*pChSwitchTime,
	OUT USHORT	*pChSwitchTimeOut);

VOID
TDLS_FTDeriveTPK(
	IN	PUCHAR 	mac_i,
	IN	PUCHAR 	mac_r,
	IN	PUCHAR 	a_nonce,
	IN	PUCHAR 	s_nonce,
	IN	PUCHAR 	bssid,
	IN	UINT	key_len,
 	OUT	PUCHAR	tpk,
	OUT	PUCHAR	tpk_name);

USHORT
TDLS_TPKMsg1Process(
	IN	PRTMP_ADAPTER		pAd, 
#ifdef WFD_SUPPORT
	IN	ULONG				WfdSubelementLen,
#endif /* WFD_SUPPORT */
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen);

USHORT
TDLS_TPKMsg2Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen,
	OUT	PUCHAR				pTPK,
	OUT PUCHAR				pTPKName);

USHORT
TDLS_TPKMsg3Process(
	IN	PRTMP_ADAPTER		pAd, 
	IN	PRT_802_11_TDLS		pTDLS,
	IN	PUCHAR				pRsnIe, 
	IN	UCHAR				RsnLen, 
	IN	PUCHAR				pFTIe, 
	IN	UCHAR				FTLen, 
	IN	PUCHAR				pTIIe, 
	IN	UCHAR				TILen);

NDIS_STATUS
TDLS_TearDownAction(
	IN PRTMP_ADAPTER	pAd,
	IN PRT_802_11_TDLS	pTDLS,
	IN UINT16	ReasonCode,
	IN BOOLEAN	bDirect);

VOID
TDLS_SendNullFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			TxRate,
	IN	BOOLEAN 		bQosNull);

VOID
TDLS_LinkMaintenance(
	IN PRTMP_ADAPTER pAd);

INT
Set_TdlsEntryInfo_Display_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PUCHAR arg);

VOID
TDLS_LinkTimeoutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_OffChExpired(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_BaseChExpired(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_ChannelSwitchTimeAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_ChannelSwitchTimeOutAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_DisablePeriodChannelSwitchAction(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID
TDLS_DisableMacTxRx(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_EnableMacTx(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_EnableMacRx(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_EnableMacTxRx(
	IN PRTMP_ADAPTER pAd);

VOID
TDLS_DisablePktChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR QSel);

VOID
TDLS_EnablePktChannel(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR QSel);

VOID
TDLS_InitChannelRelatedValue(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN bEnableMACTxRx,
	IN UCHAR TargetChannel,
	IN UCHAR TargetChannelBW);

VOID
TDLS_UpdateHwNullFramePwr(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR PwrMgmt,
	IN CHAR Index);

VOID TDLS_KickOutHwNullFrame(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR PwrMgmt,
	IN CHAR Index);

NDIS_STATUS
TDLS_SendOutActionFrame(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PUCHAR pHeader802_3,
	IN UINT HdrLen,
	IN PUCHAR pData,
	IN UINT DataLen,
	IN UCHAR FrameType);

VOID
TDLS_SendOutNullFrame(
	IN	PRTMP_ADAPTER pAd,
	IN	PMAC_TABLE_ENTRY pEntry,
	IN	BOOLEAN bQosNull,
	IN	BOOLEAN bWaitACK);

VOID
TdlsMacHeaderInit(
	IN PRTMP_ADAPTER pAd, 
	IN OUT PHEADER_802_11 pHdr80211, 
	IN UCHAR SubType, 
	IN UCHAR ToDs, 
	IN PUCHAR pDA, 
	IN PUCHAR pBssid);

VOID
TDLS_SoftwareRecovery(
	IN PRTMP_ADAPTER pAd);

#ifdef TDLS_AUTOLINK_SUPPORT
UCHAR
TDLS_ValidIdLookup(
	IN	PRTMP_ADAPTER pAd,
	IN	PUCHAR pAddr);

VOID
TDLS_AutoSetupByRcvFrame(
	IN PRTMP_ADAPTER	pAd,
	IN PHEADER_802_11 pHeader);
#endif // TDLS_AUTOLINK_SUPPORT //

#ifdef WFD_SUPPORT
BOOLEAN TDLS_PeerTunneledProbeReqRspSanity(
	IN PRTMP_ADAPTER pAd, 
	IN VOID *Msg, 
	IN ULONG MsgLen, 
	OUT PUCHAR pAddr2, 
	OUT CHAR Ssid[], 
	OUT UCHAR *pSsidLen, 
	OUT ULONG *Peerip,
	OUT UCHAR *pChannel,
	OUT ULONG *P2PSubelementLen, 
	OUT PUCHAR pP2pSubelement,
	OUT ULONG *pWfdSubelementLen, 
	OUT PUCHAR pWfdSubelement);
#endif /* WFD_SUPPORT */

#endif /* __TDLS_H */
#endif /* DOT11Z_TDLS_SUPPORT */

