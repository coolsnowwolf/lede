/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/

#ifndef __WNM_H__
#define __WNM_H__

#include "ipv6.h"					
#include "mat.h"

#define BTM_MACHINE_BASE 0
#define WaitPeerBTMRspTimeoutVale 1024

/* BTM states */
enum BTM_STATE {
	WAIT_BTM_QUERY,
	WAIT_PEER_BTM_QUERY,
	WAIT_BTM_REQ,
	WAIT_BTM_RSP,
	WAIT_PEER_BTM_REQ,
	WAIT_PEER_BTM_RSP,
	BTM_UNKNOWN,
	MAX_BTM_STATE,
};


/* BTM events */
enum BTM_EVENT {
	BTM_QUERY,
	PEER_BTM_QUERY,
	BTM_REQ,
	BTM_RSP,
	PEER_BTM_REQ,
	PEER_BTM_RSP,
	MAX_BTM_MSG,
};

#define BTM_FUNC_SIZE (MAX_BTM_STATE * MAX_BTM_MSG)

enum IPV6_TYPE{
	IPV6_LINK_LOCAL,
	IPV6_GLOBAL,
};

typedef struct GNU_PACKED _BTM_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {
#ifdef CONFIG_STA_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 BTMQueryLen;
			UCHAR BTMQuery[0];
		} GNU_PACKED BTM_QUERY_DATA;
	
		struct {
			UCHAR DialogToken;
			UINT16 BTMRspLen;
			UCHAR BTMRsp[0];
		} GNU_PACKED BTM_RSP_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMReqLen;
			UCHAR BTMReq[0];
		} GNU_PACKED PEER_BTM_REQ_DATA;
#endif /* CONFIG_STA_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 BTMReqLen;
			UCHAR BTMReq[0];
		} GNU_PACKED BTM_REQ_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMQueryLen;
			UCHAR BTMQuery[0];
		} GNU_PACKED PEER_BTM_QUERY_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 BTMRspLen;
			UCHAR BTMRsp[0];
		} GNU_PACKED PEER_BTM_RSP_DATA;
#endif /* CONFIG_AP_SUPPORT */
	}u;
} BTM_EVENT_DATA, *PBTM_EVENT_DATA;

typedef struct _BTM_PEER_ENTRY {
	DL_LIST List;
	enum BTM_STATE CurrentState;
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UCHAR DialogToken;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT WaitPeerBTMRspTimer;
#endif /* CONFIG_AP_SUPPORT */
} BTM_PEER_ENTRY, *PBTM_PEER_ENTRY;

typedef struct _PROXY_ARP_IPV4_ENTRY {
	DL_LIST List;
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPAddr[4];
} PROXY_ARP_IPV4_ENTRY, *PPROXY_ARP_IPV4_ENTRY;

typedef struct _PROXY_ARP_IPV4_UNIT {
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPAddr[4];
} PROXY_ARP_IPV4_UNIT, *PPROXY_ARP_IPV4_UNIT;

typedef struct _PROXY_ARP_IPV6_ENTRY {
	DL_LIST List;
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPType;
	UCHAR TargetIPAddr[16];
} PROXY_ARP_IPV6_ENTRY, *PPROXY_ARP_IPV6_ENTRY;

typedef struct _PROXY_ARP_IPV6_UNIT {
	UCHAR TargetMACAddr[MAC_ADDR_LEN];
	UCHAR TargetIPType;
	UCHAR TargetIPAddr[16];
} PROXY_ARP_IPV6_UNIT, *PPROXY_ARP_IPV6_UNIT;

typedef struct _WNM_CTRL {
	UINT32 TimeadvertisementIELen;
	UINT32 TimezoneIELen;
	PUCHAR TimeadvertisementIE;
	PUCHAR TimezoneIE;
	RTMP_OS_SEM BTMPeerListLock;
	RTMP_OS_SEM WNMNotifyPeerListLock;
	BOOLEAN ProxyARPEnable;
	BOOLEAN WNMNotifyEnable;
	RTMP_OS_SEM ProxyARPListLock;
	RTMP_OS_SEM ProxyARPIPv6ListLock;
	DL_LIST IPv4ProxyARPList;
	DL_LIST IPv6ProxyARPList;
	DL_LIST BTMPeerList;
	DL_LIST WNMNotifyPeerList;
} WNM_CTRL, *PWNM_CTRL;


enum IPTYPE {
	IPV4,
	IPV6
};

struct _MULTISSID_STRUCT;

BOOLEAN IsGratuitousARP(IN PRTMP_ADAPTER pAd,
						IN PUCHAR pData,
						IN UCHAR *DAMacAddr,
						IN struct _MULTISSID_STRUCT *pMbss);

BOOLEAN IsUnsolicitedNeighborAdver(PRTMP_ADAPTER pAd,
								   PUCHAR pData);

BOOLEAN IsIPv4ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData);

BOOLEAN IsIPv6ProxyARPCandidate(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData);

BOOLEAN IsIPv6DHCPv6Solicitation(IN PRTMP_ADAPTER pAd,
								IN PUCHAR pData);

BOOLEAN IsIPv6RouterSolicitation(IN PRTMP_ADAPTER pAd,
								 IN PUCHAR pData);

BOOLEAN IsIPv6RouterAdvertisement(IN PRTMP_ADAPTER pAd,
								  IN PUCHAR pData,
								  IN PUCHAR pOffset);

BOOLEAN IsTDLSPacket(IN PRTMP_ADAPTER pAd,
					 IN PUCHAR pData);

BOOLEAN IPv4ProxyARP(IN PRTMP_ADAPTER pAd,
				 	 IN struct _MULTISSID_STRUCT *pMbss,
				 	 IN PUCHAR pData,
					 IN BOOLEAN FromDS);

BOOLEAN IsIpv6DuplicateAddrDetect(PRTMP_ADAPTER pAd,
								  PUCHAR pData,
								  PUCHAR pOffset);


BOOLEAN IPv6ProxyARP(IN PRTMP_ADAPTER pAd,
					 IN struct _MULTISSID_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS);

UINT32 AddIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr);

UINT32 AddIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr);

UINT32 IPv4ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _MULTISSID_STRUCT *pMbss);

UINT32 IPv6ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _MULTISSID_STRUCT *pMbss);

BOOLEAN GetIPv4ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _MULTISSID_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

BOOLEAN GetIPv6ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _MULTISSID_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

VOID RemoveIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
					   		IN struct _MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr);

VOID RemoveIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN struct _MULTISSID_STRUCT *pMbss,
							PUCHAR pTargetMACAddr);	
							
VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd);
VOID WNMCtrlExit(IN PRTMP_ADAPTER pAd);
VOID Clear_All_PROXY_TABLE(IN PRTMP_ADAPTER pAd);
#ifdef CONFIG_AP_SUPPORT
VOID WNMIPv4ProxyARPCheck(
			IN PRTMP_ADAPTER pAd,
			PNDIS_PACKET pPacket,
			USHORT srcPort,
			USHORT dstPort,
			PUCHAR pSrcBuf);

VOID WNMIPv6ProxyARPCheck(
			IN PRTMP_ADAPTER pAd,
			PNDIS_PACKET pPacket,
			PUCHAR pSrcBuf);

DECLARE_TIMER_FUNCTION(WaitPeerBTMRspTimeout);
DECLARE_TIMER_FUNCTION(WaitPeerWNMNotifyRspTimeout);

VOID BTMStateMachineInit(
			IN	PRTMP_ADAPTER pAd, 
			IN	STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[]);

enum BTM_STATE BTMPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID ReceiveWNMNotifyRsp(IN PRTMP_ADAPTER pAd,
						  IN MLME_QUEUE_ELEM *Elem);
						  
VOID SendWNMNotifyConfirm(IN PRTMP_ADAPTER    pAd, 
    						IN MLME_QUEUE_ELEM  *Elem);

VOID WNMNotifyStateMachineInit(
			IN	PRTMP_ADAPTER pAd, 
			IN	STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[]);
	
#define WNM_NOTIFY_MACHINE_BASE 0
#define WaitPeerWNMNotifyRspTimeoutVale 1024

/* WNM Notification states */
enum WNM_NOTIFY_STATE {
	WAIT_WNM_NOTIFY_REQ,
	WAIT_WNM_NOTIFY_RSP,
	WNM_NOTIFY_UNKNOWN,
	MAX_WNM_NOTIFY_STATE,
};

/* WNM Notification events */
enum WNM_NOTIFY_EVENT {
	WNM_NOTIFY_REQ,
	WNM_NOTIFY_RSP,
	MAX_WNM_NOTIFY_MSG,
};

#define WNM_NOTIFY_FUNC_SIZE (MAX_WNM_NOTIFY_STATE * MAX_WNM_NOTIFY_MSG)

typedef struct GNU_PACKED _WNM_NOTIFY_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {

#ifdef CONFIG_AP_SUPPORT
		struct {
			UCHAR DialogToken;
			UINT16 WNMNotifyReqLen;
			UCHAR WNMNotifyReq[0];
		} GNU_PACKED WNM_NOTIFY_REQ_DATA;

		struct {
			UCHAR DialogToken;
			UINT16 WNMNotifyRspLen;
			UCHAR WNMNotifyRsp[0];
		} GNU_PACKED WNM_NOTIFY_RSP_DATA;
#endif /* CONFIG_AP_SUPPORT */
	}u;
} WNM_NOTIFY_EVENT_DATA, *PWNM_NOTIFY_EVENT_DATA;

typedef struct _WNM_NOTIFY_PEER_ENTRY {
	DL_LIST List;
	enum WNM_NOTIFY_STATE CurrentState;
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UCHAR DialogToken;
	void *Priv;
#ifdef CONFIG_AP_SUPPORT
	RALINK_TIMER_STRUCT WaitPeerWNMNotifyRspTimer;
#endif /* CONFIG_AP_SUPPORT */
} WNM_NOTIFY_PEER_ENTRY, *PWNM_NOTIFY_PEER_ENTRY;
	
INT Send_BTM_Req(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING PeerMACAddr,
	IN PSTRING BTMReq,
	IN UINT32 BTMReqLen);
	
INT Send_WNM_Notify_Req(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING PeerMACAddr,
	IN PSTRING WNMNotifyReq,
	IN UINT32 WNMNotifyReqLen,
	IN UINT32 type);		

INT Send_QOSMAP_Configure(
	IN PRTMP_ADAPTER pAd,
	IN PSTRING  PeerMACAddr,
	IN PSTRING  QosMapBuf,
	IN UINT32 	QosMapLen,
	IN UINT8	Apidx);	

#ifdef CONFIG_HOTSPOT_R2
VOID WNMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem,
	IN enum WNM_NOTIFY_STATE State);

enum WNM_NOTIFY_STATE WNMNotifyPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM *Elem);

VOID WNMNotifyStateMachineInit(
			IN	PRTMP_ADAPTER pAd, 
			IN	STATE_MACHINE *S, 
			OUT STATE_MACHINE_FUNC	Trans[]);
#endif /* CONFIG_HOTSPOT_R2 */

#endif /* CONFIG_AP_SUPPORT */

#ifdef DOT11V_WNM_SUPPORT
#include "rtmp_type.h"
#include "wnm_cmm.h"

#ifdef CONFIG_AP_SUPPORT
#define IS_BSS_TRANSIT_MANMT_SUPPORT(_P, _I) \
	((_P)->ApCfg.MBSSID[(_I)].WnmCfg.bDot11vWNM_BSSEnable == TRUE)

#define IS_WNMDMS_SUPPORT(_P, _I) \
	((_P)->ApCfg.MBSSID[(_I)].WnmCfg.bDot11vWNM_DMSEnable == TRUE)


#define IS_WNMFMS_SUPPORT(_P, _I) \
	((_P)->ApCfg.MBSSID[(_I)].WnmCfg.bDot11vWNM_FMSEnable == TRUE)

#define IS_WNMSleepMode_SUPPORT(_P, _I) \
	((_P)->ApCfg.MBSSID[(_I)].WnmCfg.bDot11vWNM_SleepModeEnable == TRUE)

#define IS_WNMTFS_SUPPORT(_P, _I) \
	((_P)->ApCfg.MBSSID[(_I)].WnmCfg.bDot11vWNM_TFSEnable == TRUE)


#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT

#define IS_BSS_TRANSIT_MANMT_SUPPORT(_P) \
	((_P)->StaCfg.WnmCfg.bDot11vWNM_BSSEnable == TRUE)

#define IS_WNMDMS_SUPPORT(_P) \
	((_P)->StaCfg.WnmCfg.bDot11vWNM_DMSEnable == TRUE)

#define IS_WNMFMS_SUPPORT(_P) \
	((_P)->StaCfg.WnmCfg.bDot11vWNM_FMSEnable == TRUE)

#define IS_WNMSleepMode_SUPPORT(_P) \
	((_P)->StaCfg.WnmCfg.bDot11vWNM_SleepModeEnable == TRUE)

#define IS_WNMTFS_SUPPORT(_P) \
	((_P)->StaCfg.WnmCfg.bDot11vWNM_TFSEnable == TRUE)


#endif /* CONFIG_STA_SUPPORT */


#define WNM_MEM_COPY(__Dst, __Src, __Len)	memcpy(__Dst, __Src, __Len)
#define WNMR_ARG_ATOI(__pArgv)				simple_strtol((PSTRING) __pArgv, 0, 10)
#define WNMR_ARG_ATOH(__Buf, __Hex)			AtoH((PSTRING) __Buf, __Hex, 1)



VOID WNMAPBTMStateMachineInit(
    IN  PRTMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);


VOID WNMSTABTMStateMachineInit(
    IN  PRTMP_ADAPTER   pAd, 
    IN  STATE_MACHINE *Sm, 
    OUT STATE_MACHINE_FUNC Trans[]);


/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID WNM_Action(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);


/*
	==========================================================================
	Description:
		
	Parametrs:
	
	Return	: None.
	==========================================================================
 */
void WNM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer);


#ifdef CONFIG_AP_SUPPORT

BOOLEAN DeleteDMSEntry(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry);

VOID FMSTable_Release(
	IN PRTMP_ADAPTER pAd);

VOID DMSTable_Release(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DeleteTFSID(
    IN  PRTMP_ADAPTER	pAd,
    IN  MAC_TABLE_ENTRY *pEntry);

NDIS_STATUS FMSPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN UCHAR apidx,
	IN UCHAR QueIdx,
	IN UINT8 UserPriority);

NDIS_STATUS TFSPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN UCHAR apidx,
	IN UCHAR QueIdx,
	IN UINT8 UserPriority);

/* */
/*
	==========================================================================
	Description:
		
	Parametrs:
	
	Return	: None.
	==========================================================================
 */
INT	Set_WNMTransMantREQ_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_APWNMDMSShow_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

/*
	==========================================================================
	Description:
		Insert WNM Max Idle Capabilitys IE into frame.
		
	Parametrs:
		1. frame buffer pointer.
		2. frame length.
	
	Return	: None.
	==========================================================================
 */
VOID WNM_InsertMaxIdleCapIE(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR aoidx,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen);

VOID WNM_InsertFMSDescripotr(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ElementID,
	IN UINT8 Length,
	IN UINT8 NumOfFMSCs);

VOID WNM_InsertFMSStSubEelment(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_FMS_STATUS_SUBELMENT FMSSubElement);

VOID InsertFMSRspSubElement(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_FMS_RESPONSE_ELEMENT FMSReqElement);
#endif /* CONFIG_AP_SUPPORT */
VOID WNM_Init(IN PRTMP_ADAPTER pAd);

VOID IS_WNM_DMS(
	IN PRTMP_ADAPTER pAd, 
	IN PNDIS_PACKET pRxPacket, 
	IN PHEADER_802_11 pHeader);

VOID InsertDMSReqElement(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_DMS_REQUEST_ELEMENT DMSReqElement);


VOID WNM_InsertDMS(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Len,
	IN UCHAR DMSID,
	IN WNM_TCLAS wmn_tclas,
	IN ULONG IpAddr);

VOID WNM_InsertFMSReqSubEelment(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_FMS_SUBELEMENT FMSSubElement,
	IN WNM_TCLAS wmn_tclas,
	IN ULONG IpAddr);

VOID InsertFMSReqElement(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_FMS_REQUEST_ELEMENT FMSReqElement);

VOID WNM_InsertTFSReqSubEelment(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_TFS_SUBELEMENT TFSSubElement,
	IN WNM_TCLAS wmn_tclas,
	IN ULONG IpAddr);


VOID InsertTFSReqElement(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_TFS_REQUEST_ELEMENT FMSReqElement);


VOID WNM_InsertSleepModeEelment(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN WNM_SLEEP_MODE_ELEMENT Sleep_Mode_Elmt);
	
VOID InsertRequestTyppe(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UCHAR RequestTyppe);


BOOLEAN RxDMSHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPkt);

#endif /* DOT11V_WNM_SUPPORT */

#endif /* __WNM_H__ */

