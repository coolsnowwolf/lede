/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	wnm.h

	Abstract:

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifndef __WNM_H__
#define __WNM_H__

#include "ipv6.h"
#include "mat.h"

#define BTM_MACHINE_BASE 0
#ifdef CONFIG_11KV_API_SUPPORT
#define WaitPeerBTMRspTimeoutVale (15*1000)
#define WaitPeerBTMReqTimeoutVale (30*1000)
#else
#define WaitPeerBTMRspTimeoutVale 2048
#define WaitPeerBTMReqTimeoutVale (1024)
#endif /* CONFIG_11KV_API_SUPPORT */


#define BTM_ENABLE_OFFSET   (1<<0)
#ifdef CONFIG_11KV_API_SUPPORT
#define	BTM_CANDIDATE_OFFSET (1<<0)
#define	BTM_ABRIDGED_OFFSET (1<<1)
#define	BTM_DISASSOC_OFFSET (1<<2)
#define	BTM_TERMINATION_OFFSET (1<<3)
#define	BTM_URL_OFFSET (1<<4)
#endif /* CONFIG_11KV_API_SUPPORT */

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
	BTM_REQ_IE,
	BTM_REQ_PARAM,
	BTM_RSP,
	PEER_BTM_REQ,
	PEER_BTM_RSP,
	BTM_REQ_TIMEOUT,
	PEER_BTM_RSP_TIMEOUT,
	MAX_BTM_MSG,
};

#define BTM_FUNC_SIZE (MAX_BTM_STATE * MAX_BTM_MSG)

enum IPV6_TYPE {
	IPV6_LINK_LOCAL,
	IPV6_GLOBAL,
};

typedef struct GNU_PACKED _BTM_EVENT_DATA {
	UCHAR ControlIndex;
	UCHAR PeerMACAddr[MAC_ADDR_LEN];
	UINT16 EventType;
	union {

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
	} u;
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
	RALINK_TIMER_STRUCT WaitPeerBTMReqTimer;
#endif /* CONFIG_AP_SUPPORT */
	UINT32 WaitPeerBTMRspTime;
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
	BOOLEAN WNMBTMEnable;
	BOOLEAN WNMCertEnable;
	RTMP_OS_SEM ProxyARPListLock;
	RTMP_OS_SEM ProxyARPIPv6ListLock;
	DL_LIST IPv4ProxyARPList;
	DL_LIST IPv6ProxyARPList;
	DL_LIST BTMPeerList;
	DL_LIST WNMNotifyPeerList;
} WNM_CTRL, *PWNM_CTRL;

#ifdef BB_SOC
#ifdef IPV6
#undef IPV6
enum IPTYPE {
	IPV4,
	IPV6
};
#endif
#else
enum IPTYPE {
	IPV4,
	IPV6
};
#endif /* BB_SOC */

struct _BSS_STRUCT;

BOOLEAN IsGratuitousARP(IN PRTMP_ADAPTER pAd,
						IN PUCHAR pData,
						IN UCHAR *DAMacAddr,
						IN struct _BSS_STRUCT *pMbss);

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
					 IN struct _BSS_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS);

BOOLEAN IsIpv6DuplicateAddrDetect(PRTMP_ADAPTER pAd,
								  PUCHAR pData,
								  PUCHAR pOffset);

BOOLEAN IPv6ProxyARP(IN PRTMP_ADAPTER pAd,
					 IN struct _BSS_STRUCT *pMbss,
					 IN PUCHAR pData,
					 IN BOOLEAN FromDS);

UINT32 AddIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr);

UINT32 AddIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss,
							IN PUCHAR pTargetMACAddr,
							IN PUCHAR pTargetIPAddr);

UINT32 IPv4ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss);

UINT32 IPv6ProxyARPTableLen(IN PRTMP_ADAPTER pAd,
							IN struct _BSS_STRUCT *pMbss);

BOOLEAN GetIPv4ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

BOOLEAN GetIPv6ProxyARPTable(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 OUT	PUCHAR *ProxyARPTable);

VOID RemoveIPv4ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 PUCHAR pTargetMACAddr);

VOID RemoveIPv6ProxyARPEntry(IN PRTMP_ADAPTER pAd,
							 IN struct _BSS_STRUCT *pMbss,
							 PUCHAR pTargetMACAddr);

VOID WNMCtrlInit(IN PRTMP_ADAPTER pAd);
VOID WNMCtrlExit(IN PRTMP_ADAPTER pAd);
VOID Clear_All_PROXY_TABLE(IN PRTMP_ADAPTER pAd);
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
DECLARE_TIMER_FUNCTION(WaitPeerBTMReqTimeout);

VOID BTMStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);

enum BTM_STATE BTMPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID ReceiveWNMNotifyReq(IN PRTMP_ADAPTER pAd,
			  IN MLME_QUEUE_ELEM *Elem);

#ifndef CONFIG_HOTSPOT_R2/* #ifdef WNM_NEW_API */
NDIS_STATUS wnm_handle_command(IN PRTMP_ADAPTER pAd,
										IN struct wnm_command *pCmd_data);
#endif
void WNM_ReadParametersFromFile(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer);

INT Set_WNMCertEnable_Proc(
	IN RTMP_ADAPTER * pAd,
	IN RTMP_STRING *arg);

VOID ReceiveWNMNotifyRsp(IN PRTMP_ADAPTER pAd,
						 IN MLME_QUEUE_ELEM * Elem);

VOID SendWNMNotifyConfirm(IN PRTMP_ADAPTER    pAd,
						  IN MLME_QUEUE_ELEM * Elem);

VOID WNMNotifyStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);

#define WNM_NOTIFY_MACHINE_BASE 0
#define WaitPeerWNMNotifyRspTimeoutVale 1024


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
	} u;
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
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * BTMReq,
	IN UINT32 BTMReqLen);

INT Send_WNM_Notify_Req(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * WNMNotifyReq,
	IN UINT32 WNMNotifyReqLen,
	IN UINT32 type);

INT Send_QOSMAP_Configure(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * PeerMACAddr,
	IN RTMP_STRING * QosMapBuf,
	IN UINT32	QosMapLen,
	IN UINT8	Apidx);

#ifdef CONFIG_HOTSPOT_R2
VOID WNMSetPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem,
	IN enum WNM_NOTIFY_STATE State);

enum WNM_NOTIFY_STATE WNMNotifyPeerCurrentState(
	IN PRTMP_ADAPTER pAd,
	IN MLME_QUEUE_ELEM * Elem);

VOID WNMNotifyStateMachineInit(
	IN	PRTMP_ADAPTER pAd,
	IN	STATE_MACHINE * S,
	OUT STATE_MACHINE_FUNC	Trans[]);
#endif /* CONFIG_HOTSPOT_R2 */
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_11KV_API_SUPPORT
int send_btm_req_param(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

int send_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_req_ie_data_t p_btm_req_data,
	IN UINT32 btm_req_data_len);


int check_btm_custom_params(
	IN PRTMP_ADAPTER pAd,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

int compose_btm_req_ie(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR p_btm_req_ie,
	OUT PUINT32 p_btm_req_ie_len,
	IN p_btm_reqinfo_t p_btm_req_data,
	IN UINT32 btm_req_data_len);

VOID WNM_InsertBSSTerminationSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT64 TSF,
	IN UINT16 Duration);

VOID RRM_InsertPreferenceSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PUINT32 pFrameLen,
	IN UINT8 preference);
#endif /* CONFIG_11KV_API_SUPPORT */



#endif /* __WNM_H__ */

