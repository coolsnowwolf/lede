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

    Abstract:


 */

#ifdef DOT11R_FT_SUPPORT


#ifndef __FT_H
#define __FT_H

#include "link_list.h"
#include "ft_cmm.h"

#define IN
#define OUT
#define INOUT

/* Macro */
#define IS_FT_STA(__A) ((__A)->MdIeInfo.Len != 0)
#define IS_FT_ACTION_FRAME(__A) ((*(__A + 13) == 0x89)&&(*(__A + 14) == 0x0d))

/* ASCII to Integer */
#define FT_ARG_ATOI(__pArgv)			simple_strtol((RTMP_STRING *) __pArgv, 0, 10)

extern UINT32 FT_KDP_MemAllocNum, FT_KDP_MemFreeNum;
#define FT_MEM_ALLOC(__pAd, __pMem, __Size)	\
						os_alloc_mem(__pAd, (UCHAR **)(__pMem), __Size); \
						FT_KDP_MemAllocNum++;
#define FT_MEM_FREE(__pAd, __Mem)			\
						{ os_free_mem(__pAd, __Mem); \
						FT_KDP_MemFreeNum++; }

/*	atoi & atoh */
#define FT_ARG_ATOI(__pArgv)			simple_strtol((RTMP_STRING *) __pArgv, 0, 10)
#define FT_ARG_ATOH(__Buf, __Hex)		AtoH((RTMP_STRING *) __Buf, __Hex, 1)

#ifdef CONFIG_AP_SUPPORT
/* linked list, please reference to link_list.h */
#define FT_KDP_SIGNAL					RT_SIGNAL_STRUC

#define FT_KDP_IOCTL_KEY_REQ(__pAd, __pInfo, __InfoLen)				\
{																	\
	UINT32 __PeerIP;												\
	memcpy(&__PeerIP, __pInfo, FT_IP_ADDRESS_SIZE);					\
	FT_KDP_KeyRequestToUs(__pAd, __PeerIP,							\
				(__pInfo)+FT_IP_ADDRESS_SIZE,						\
				(FT_KDP_EVT_KEY_ELM *)								\
					((__pInfo)+FT_IP_ADDRESS_SIZE+FT_NONCE_SIZE));	\
}

/* Macros relative to R1KH Table. */
#define FT_R1NAME_HASH(_Data)		(_Data[0] ^ _Data[1] ^ _Data[2] ^_Data[3] \
 									^ _Data[4] ^ _Data[5] ^_Data[6] ^ _Data[7] \
 									^ _Data[8] ^_Data[9] ^ _Data[10] ^ _Data[11] \
										^_Data[12] ^ _Data[13] ^ _Data[14] ^_Data[15])
#define FT_R1KH_HASH_INDEX(_Data)	(FT_R1NAME_HASH(_Data) \
										% (FT_R1KH_ENTRY_HASH_TABLE_SIZE))

typedef struct GNU_PACKED _FT_KDP_PMK_KEY_INFO {

	UCHAR	R0KHID[FT_KDP_R0KHID_MAX_SIZE];
	UCHAR	R0KHIDLen;
	UCHAR	PMKR0Name[FT_KDP_WPA_NAME_MAX_SIZE]; /* an ID that names the PMK-R0 */

	UCHAR	R1KHID[FT_KDP_R1KHID_MAX_SIZE];
	UCHAR	S1KHID[FT_KDP_S1KHID_MAX_SIZE];

	/* reserved field */
	UCHAR	RSV[4];

} FT_KDP_PMK_KEY_INFO;

#ifdef IAPP_SUPPORT
typedef struct GNU_PACKED _RT_SIGNAL_STRUC {

	/* IEEE80211R_SUPPORT */
	VOID	*pNext; /* point to next signal */

	UINT16	Sequence;
	UCHAR	MacAddr[ETH_ALEN];
	UCHAR	CurrAPAddr[ETH_ALEN];
	/* YF_FT */
#ifdef R1KH_HARD_RETRY
	/* sw retry counts */
#define FT_R1KH_CACHE_MISS_THRESHOLD	 1
	/* hard retry counts */
#define FT_R1KH_CACHE_MISS_HARD_RETRY_THRESHOLD 1
#else /* R1KH_HARD_RETRY */
#define FT_R1KH_CACHE_MISS_THRESHOLD	 7
#endif /* !R1KH_HARD_RETRY */
#define FT_KDP_SIG_NOTHING				0x00 /* no signal */
#define FT_KDP_SIG_IAPP_ASSOCIATION		0x01 /* a station has associated */
#define FT_KDP_SIG_IAPP_REASSOCIATION	0x02 /* a station has re-associated */
#define FT_KDP_SIG_TERMINATE			0x03 /* terminate the daemon */

#define FT_KDP_SIG_FT_ASSOCIATION		0x50 /* a FT station has associated */
#define FT_KDP_SIG_FT_REASSOCIATION		0x51 /* a FT station has re-associated */
#define FT_KDP_SIG_KEY_TIMEOUT			0x52 /* PMK-R1 KEY Timeout */
#define FT_KDP_SIG_KEY_REQ				0x53 /* request PMK-R1 KEY from R0KH */
#define FT_KDP_SIG_ACTION				0x54 /* forward FT Action frame to DS */

#define FT_KDP_SIG_AP_INFO_REQ			0x70 /* request neighbor AP info. */
#define FT_KDP_SIG_AP_INFO_RSP			0x71 /* response my AP info. */

/* FT KDP internal use */
#define FT_KDP_SIG_KEY_REQ_AUTO			0xA0 /* request PMK-R1 KEY from R0KH */
#define FT_KDP_SIG_KEY_RSP_AUTO			0xA1 /* response PMK-R1 KEY to R1KH */
#define FT_KDP_SIG_INFO_BROADCAST		0xB0 /* broadcast our AP information */

#define FT_KSP_SIG_DEBUG_TRACE			0xC0 /* enable debug flag to TRACE */
	UCHAR	Sig;

	UCHAR	MacAddrSa[ETH_ALEN];

	/* IEEE80211R_SUPPORT */
	/* the first 6B are FT_KDP_EVT_HEADER */
	/*
		For FT_KDP_SIG_NOTHING:			nothing
		For FT_KDP_SIG_IAPP_ASSOCIATION:nothing
		For FT_KDP_SIG_REASSOCIATION:	nothing
		For FT_KDP_SIG_TERMINATE:		nothing
		For FT_KDP_SIG_FT_ASSOCIATION:	nothing
		FT_KDP_SIG_FT_REASSOCIATION:	nothing
		For FT_KDP_SIG_KEY_TIMEOUT:		it is 
		For FT_KDP_SIG_KEY_REQ_AUTO:	it is FT_KDP_EVT_KEY_REQ
		For FT_KDP_SIG_KEY_RSP_AUTO:	it is FT_KDP_SIG_KEY_RSP
	*/
	UCHAR	Reserved[3];			/* let address of Content[] 4B align */
	UCHAR	Content[1024];			/* signal content */

/* 1024 means size of Content[] */
#define RT_SIGNAL_STRUC_HDR_SIZE			(sizeof(RT_SIGNAL_STRUC)-1024)
} RT_SIGNAL_STRUC, *PRT_SIGNAL_STRUC;

/* definition of signal */
#define	SIG_NONE					0 /* same as FT_KDP_SIG_NOTHING */
#define SIG_ASSOCIATION				1 /* same as FT_KDP_SIG_ASSOCIATION */
#define SIG_REASSOCIATION			2 /* same as FT_KDP_SIG_REASSOCIATION */
#endif /* IAPP_SUPPORT */


/* event */
#define FT_KDP_EVENT_INFORM(__pAd, __ApIdx, __EventId, __pEvent, __EventLen, __CB)	\
	FT_KDP_EventInform(__pAd, __ApIdx, __EventId, (VOID *)__pEvent, __EventLen, 0, __CB)

/* ----- KDP Linked List ----- */
#define FT_KDP_EVT_LIST_INSERT_TAIL(__pAd, __pList, __pEntry)		\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
		insertTailList((__pList), (RT_LIST_ENTRY *)(__pEntry));		\
		RTMP_SEM_UNLOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
	}

#define FT_KDP_EVT_LIST_REMOVE_HEAD(__pAd, __pList, __pEntry)		\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
		__pEntry = (FT_KDP_SIGNAL *)removeHeadList((__pList));		\
		RTMP_SEM_UNLOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
	}

#define FT_KDP_EVT_LIST_SIZE_GET(__pAd, __pList, __Size)			\
	{																\
		RTMP_SEM_LOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
		__Size = getListSize((__pList));							\
		RTMP_SEM_UNLOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
	}

#define FT_KDP_EVT_LIST_EMPTY(__pAd, __pList)						\
	{																\
		FT_KDP_SIGNAL *__pEntry;									\
		RTMP_SEM_LOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
		__pEntry = (FT_KDP_SIGNAL *)removeHeadList((__pList));		\
		while(__pEntry != NULL)										\
		{															\
			FT_MEM_FREE(__pAd, __pEntry);							\
			__pEntry = (FT_KDP_SIGNAL *)removeHeadList((__pList));	\
		}															\
		RTMP_SEM_UNLOCK(&(__pAd)->ApCfg.FtTab.FT_KdpLock);			\
	}

/* general */
#define FT_KDP_CB						((FT_KDP_CTRL_BLOCK *)pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK)

typedef struct GNU_PACKED _FT_KDP_EVT_HEADER
{
	UINT32	EventLen;
	UINT32	PeerIpAddr; /* used in daemon, open TCP/IP socket to the IP */
} FT_KDP_EVT_HEADER;

typedef struct GNU_PACKED _FT_KDP_EVT_ASSOC
{
	/* same as IEEE802.11f IAPP */
	UINT16	SeqNum;
	UCHAR	MacAddr[ETH_ALEN];
} FT_KDP_EVT_ASSOC;

typedef struct GNU_PACKED _FT_KDP_EVT_REASSOC
{
	UINT16	SeqNum;
	UCHAR	MacAddr[ETH_ALEN];
	UCHAR	OldApMacAddr[ETH_ALEN];
} FT_KDP_EVT_REASSOC;

typedef struct GNU_PACKED _FT_KDP_EVT_KEY_ELM
{
	/* must be 65535, Proprietary Information */
#define FT_KDP_ELM_ID_PRI			65535
	UINT16	ElmId;

#define FT_KDP_ELM_PRI_LEN			(sizeof(FT_KDP_EVT_KEY_ELM)-4) 
	UINT16	ElmLen;

	/* must be 0x00 0x0E 0x2E, RALINK */
#define FT_KDP_ELM_PRI_OUI_0		0x00
#define FT_KDP_ELM_PRI_OUI_1		0x0E
#define FT_KDP_ELM_PRI_OUI_2		0x2E
#define FT_KDP_ELM_PRI_OUI_SIZE		3
	UCHAR	OUI[3];

	/* station MAC */
	UCHAR	MacAddr[ETH_ALEN];
	UCHAR	RSV[3];

	/* used in request */
	FT_KDP_PMK_KEY_INFO KeyInfo;

	/* used in response */
	UCHAR	PMKR1Name[FT_KDP_WPA_NAME_MAX_SIZE]; /* an ID that names the PMK-R1 */
	UCHAR	PMKR1[FT_KDP_PMKR1_MAX_SIZE]; /* PMK R1 Key */
	UCHAR	R0KH_MAC[ETH_ALEN]; /* MAC of R0KH */

	/*
		During a Fast BSS Transition a non-AP STA shall negotiate the same
		pairwise cipher suite with Target APs as was negotiated in the FT
		Initial Mobility Domain association. The target AP shall verify
		that the same pairwise cipher suite selector is used, using the
		pairwise cipher suite selector value in the PMK-R1 SA received from
		the R0KH.
	*/
	UCHAR	PairwisChipher[4];
	UCHAR	AkmSuite[4];

	UINT32	KeyLifeTime;
	UINT32	ReassocDeadline;
} FT_KDP_EVT_KEY_ELM;

typedef struct _FT_KDP_EVT_ACTION
{
	UCHAR	RequestType;
	UCHAR	MacDa[ETH_ALEN];
	UCHAR	MacSa[ETH_ALEN];
	UCHAR	MacAp[ETH_ALEN];
} FT_KDP_EVT_ACTION;

/* global function prototype for non-11r modules */
/*
========================================================================
Routine Description:
	Initialize FT KDP Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID FT_KDP_Init(
	IN	PRTMP_ADAPTER		pAd);

/*
========================================================================
Routine Description:
	Release FT KDP Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID FT_KDP_Release(
	IN	PRTMP_ADAPTER		pAd);

/*
========================================================================
Routine Description:
	Get a event.

Arguments:
	pAd				- WLAN control block pointer
	**pFtKdp		- the queued event; if NULL, no event is queued.

Return Value:
	None

Note:
	Used in IOCTL from FT KDP daemon.
========================================================================
*/
VOID FT_KDP_EventGet(
	IN	PRTMP_ADAPTER		pAd,
	OUT	FT_KDP_SIGNAL		**pFtKdp);

/*
========================================================================
Routine Description:
	Inform us a event occurs.

Arguments:
	pAd				- WLAN control block pointer
	ApIdx			- MBSS ID
	EventId			- which event, such as FT_KDP_SIG_FT_ASSOCIATION
	*pEvent			- event body
	EventLen		- event body length
	PeerIP			- peer IP address
	pCB				- for special use (such as FT_KDP_SIG_ACTION)

Return Value:
	None

Note:
	EventId							*pEvent
	--------------------------------------------------------------------
	FT_KDP_SIG_FT_ASSOCIATION		FT_KDP_EVT_ASSOC
	FT_KDP_SIG_FT_REASSOCIATION		FT_KDP_EVT_REASSOC
	FT_KDP_SIG_KEY_TIMEOUT			yet implement
	FT_KDP_SIG_KEY_REQ				FT_KDP_EVT_KEY_ELM
	FT_KDP_SIG_ACTION				WLAN Frame Body (Start from Category field)
									pCB == FT_KDP_EVT_ACTION
	
========================================================================
*/
VOID FT_KDP_EventInform(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT32				ApIdx,
	IN	UCHAR				EventId,
	IN	VOID				*pEvent,
	IN	UINT16				EventLen,
	IN	UINT32				PeerIP,
	IN	VOID				*pCB);

/*
========================================================================
Routine Description:
	Inform us a AP request the key.

Arguments:
	pAd				- WLAN control block pointer
	PeerIP			- the IP of R0KH
	*pNonce			- the nonce for key request
	*pEvtKeyReq		- information for key request

Return Value:
	TRUE			- request ok
	FALSE			- can not find the IP of R0KH

Note:
	If PeerIP == 0, search its IP from our information record.
========================================================================
*/
BOOLEAN FT_KDP_KeyRequestToUs(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT32				PeerIP,
	IN	UCHAR				*pNonce,
	IN	FT_KDP_EVT_KEY_ELM	*pEvtKeyReq);

/*
========================================================================
Routine Description:
	Inform us a AP response the key.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- key information
	InfoLen			- information length

Return Value:
	None

Note:
	pInfo format is PeerIP (4B) + Nonce (8B) + FT_KDP_EVT_KEY_ELM structure
========================================================================
*/
VOID FT_KDP_KeyResponseToUs(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT8				*pInfo,
	IN	INT32				InfoLen);

/*
========================================================================
Routine Description:
	Inform us a FT station joins the BSS domain.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- station information
	InfoLen			- information length

Return Value:
	None

Note:
	pInfo format is PeerIP (4B) + Station MAC (6B)
========================================================================
*/
VOID FT_KDP_StationInform(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT8				*pInfo,
	IN	INT32				InfoLen);

/*
========================================================================
Routine Description:
	Set the encryption/decryption key.

Arguments:
	pAd				- WLAN control block pointer
	*pKey			- the key
	KeyLen			- the key length

Return Value:
	None

Note:
	Key length can be 8B ~ 64B.
========================================================================
*/
VOID FT_KDP_CryptKeySet(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pKey,
	IN	INT32				KeyLen);

/*
========================================================================
Routine Description:
	Encrypt the data.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- data buffer
	*pDataLen		- data buffer length

Return Value:
	None

Note:
========================================================================
*/
VOID FT_KDP_DataEncrypt(
	IN		PRTMP_ADAPTER		pAd,
	INOUT	UCHAR				*pData,
	IN		UINT32				*pDataLen);

/*
========================================================================
Routine Description:
	Decrypt the data.

Arguments:
	pAd				- WLAN control block pointer
	*pData			- data buffer
	*pDataLen		- data buffer length

Return Value:
	None

Note:
========================================================================
*/
VOID FT_KDP_DataDecrypt(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pData,
	IN	UINT32				*pDataLen);

/*
========================================================================
Routine Description:
	Broadcast our AP information.

Arguments:
	SystemSpecific1	- no use
	FunctionContext - WLAN control block pointer
	SystemSpecific2 - no use
	SystemSpecific3 - no use

Return Value:
	NONE

Note:
========================================================================
*/
VOID FT_KDP_InfoBroadcast(
	IN	PVOID				SystemSpecific1,
	IN	PVOID				FunctionContext,
	IN	PVOID				SystemSpecific2,
	IN	PVOID				SystemSpecific3);

#ifdef FT_KDP_FUNC_INFO_BROADCAST
/*
========================================================================
Routine Description:
	Receive a neighbor report from another AP.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- report information
	InfoLen			- information length

Return Value:
	NONE

Note:
========================================================================
*/
VOID TYPE_FUNC FT_KDP_NeighborReportHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen);
#endif /* FT_KDP_FUNC_INFO_BROADCAST */

/*
========================================================================
Routine Description:
	Receive a neighbor request from another AP.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- report information
	InfoLen			- information length

Return Value:
	NONE

Note:
========================================================================
*/
VOID FT_KDP_NeighborRequestHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen);

/*
========================================================================
Routine Description:
	Receive a neighbor response from another AP.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- report information
	InfoLen			- information length

Return Value:
	NONE

Note:
========================================================================
*/
VOID FT_KDP_NeighborResponseHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen);

/*
========================================================================
Routine Description:
	Handle a RRB frame from DS.

Arguments:
	pAd				- WLAN control block pointer
	*pInfo			- report information
	InfoLen			- information length

Return Value:
	NONE

Note:
========================================================================
*/
VOID FT_RRB_ActionHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen);

BOOLEAN FT_QueryKeyInfoForKDP(
	IN 	PRTMP_ADAPTER 	pAd,
	IN	UINT32			ApIdx,
	OUT FT_KDP_EVT_KEY_ELM	*pEvtKeyReq);

#endif /* CONFIG_AP_SUPPORT */

/*
========================================================================
Routine Description:
	Initialize FT RIC Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID FT_RIC_Init(
	IN	PRTMP_ADAPTER		pAd);

/*
========================================================================
Routine Description:
	Release FT RIC Module.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:
========================================================================
*/
VOID FT_RIC_Release(
	IN	PRTMP_ADAPTER		pAd);

/*
========================================================================
Routine Description:
	IO control handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer
	pArgvIn			- the data flow information

Return Value:
	None

Note:
	All test commands are listed as follows:
		iwpriv ra0 set ft=[cmd id]_[arg1]_[arg2]_......_[argn]
		[cmd id] = xx, such as 00, 01, 02, 03, ...
========================================================================
*/
INT FT_Ioctl(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

/*
========================================================================
Routine Description:
	FT insert MDIE to packet frame for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_InsertMdIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN PUINT8 pMdId,
	IN FT_CAP_AND_POLICY FtCapPlc);

/*
========================================================================
Routine Description:
	FT configuration handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
 VOID FT_CfgInitial(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:
	FT function initialization for IEEE802.11r.
	FT_Init'll allocate tables, control blocks that FT needed.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_Init(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:
	FT function release for IEEE802.11r.
	Free all tables, control blocks which FT_Init allocated.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_Release(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:
	FT Peer Auth-Req handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
USHORT FT_AuthReqHandler(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PFT_INFO pFtInfo,
	OUT PFT_INFO pFtInfoBuf);

/*
========================================================================
Routine Description:

Arguments:

Return Value:
	None

Note:

========================================================================
*/
VOID FT_EnqueueAuthReply(
	IN PRTMP_ADAPTER pAd,
	IN PHEADER_802_11 pRcvHdr,
	IN USHORT Alg,
    IN USHORT Seq, 
    IN USHORT StatusCode,
	IN PFT_MDIE_INFO pMdIeInfo,
	IN PFT_FTIE_INFO pFtIeInfo,
	IN PFT_RIC_INFO pRicInfo,
	IN PUCHAR pRsnIe,
	IN UCHAR RsnIeLen);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
USHORT FT_AuthConfirmHandler(
	IN PRTMP_ADAPTER pAd,
	IN PMAC_TABLE_ENTRY pEntry,
	IN PFT_INFO pFtInfo,
	OUT PFT_INFO pFtInfoBuf);

/*
========================================================================
Routine Description:
	FT Peer Association Request handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
USHORT FT_AssocReqHandler(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN isReassoc,
	IN PFT_CFG pFtCfg,
    IN PMAC_TABLE_ENTRY pEntry,
	IN PFT_INFO			pPeer_FtInfo,
	OUT PFT_INFO	pFtInfoBuf);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_KDP_KeyResponse(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen);

/*
========================================================================
Routine Description:
	FT Peer Resource Request handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
UINT32 FT_RIC_ResourceRequestHandle(
	IN	PRTMP_ADAPTER			pAd,
	IN	MAC_TABLE_ENTRY			*pCdb,
	IN	UCHAR					*pBufReq,
	IN	UINT32					ReqLen,
	OUT	UCHAR					*pBufRsp,
	OUT	UINT32					*pRspLen);


/*
========================================================================
Routine Description:
	FT Peer Resource Request handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
UINT32 BA_ResourceAllocate(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pMacEntry,
	IN UCHAR *pBufRscReq,
	OUT UCHAR *pBufRscRsp,
	OUT ULONG *pBufRspLen);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_InsertFTIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 Length,
	IN FT_MIC_CTR_FIELD MICCtr,
	IN PUINT8 pMic,
	IN PUINT8 pANonce,
	IN PUINT8 pSNonce);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_FTIE_InsertKhIdSubIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_SUB_ELEMENT_ID SubId,
	IN PUINT8 pKhId,
	IN UINT8 KhIdLen);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_FTIE_InsertGTKSubIE(
	IN PRTMP_ADAPTER pAd,
	IN 	PUCHAR 	pFrameBuf,
	OUT PULONG pFrameLen,
	IN 	PUINT8 	pGtkSubIe,
	IN 	UINT8 	GtkSubIe_len);

/*
========================================================================
Routine Description:
	FT Peer Auth-Confirm handler for IEEE802.11r.

Arguments:
	pAd				- WLAN control block pointer

Return Value:
	None

Note:

========================================================================
*/
VOID FT_InsertTimeoutIntervalIE(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN FT_TIMEOUT_INTERVAL_TYPE Type,
	IN UINT32 TimeOutValue);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_R1khEntryTabInit(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
INT FT_R1khEntryInsert(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pPmkR0Name,
	IN PUINT8 pPmkR1Name,
	IN PUINT8 pPmkR1Key,
	IN PUINT8 pPairwisChipher,
	IN PUINT8 pAkmSuite,
	IN UINT32 KeyLifeTime,
	IN UINT32 RassocDeadline,
	IN PUINT8 pR0khId,
	IN UINT8 R0khIdLen,
	IN PUINT8 pStaMac);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_R1khEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN PFT_R1HK_ENTRY pEntry);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_R1khEntryTabDestroy(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
PFT_R1HK_ENTRY FT_R1khEntryTabLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUINT8 pPMKR1Name);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_FillMdIeInfo(
	PEID_STRUCT eid_ptr,
	PFT_MDIE_INFO pMdIeInfo);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_FillFtIeInfo(
	PEID_STRUCT eid_ptr,
	PFT_FTIE_INFO pFtIeInfo);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_FtAction(
	IN PRTMP_ADAPTER pAd, 
	IN MLME_QUEUE_ELEM *Elem);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_RrbHandler(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pPktSrc,
	IN INT32 PktLen);

/*
========================================================================
Routine Description:

Arguments:

Return Value:

Note:

========================================================================
*/
VOID FT_R1KHInfoMaintenance(
	IN PRTMP_ADAPTER pAd);

/*
========================================================================
Routine Description:
	Display all R0KH information.
 
Arguments:
	pAd				- WLAN control block pointer
	*pArgv			- input parameters

Return Value:
	TRUE

Note:
========================================================================
*/
INT FT_KDP_CMD_R0KH_InfoShow(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID FT_MakeFtActFrame(
	IN PRTMP_ADAPTER pAd,
	OUT PUCHAR pFrameBuf,
	OUT PULONG pFrameLen,
	IN UINT8 ActType,
	IN PUCHAR pStaMac,
	IN PUCHAR pTargetApMac,
	IN UINT16 StatusCode,
	IN PFT_INFO pFtInfo);

VOID FT_DerivePMKR0(
	IN	PUINT8	xxkey,
	IN	INT		xxkey_len,
	IN	PUINT8	ssid,
	IN	INT		ssid_len,
	IN	PUINT8	mdid,
	IN	PUINT8 	r0khid,
	IN	INT		r0khid_len,
	IN	PUINT8	s0khid,
	OUT	PUINT8	pmkr0,
	OUT	PUINT8	pmkr0_name);

VOID FT_DerivePMKR1Name(
	IN	PUINT8	pmkr0_name,
	IN	PUINT8 	r1khid,
	IN	PUINT8	s1khid,
	OUT	PUINT8	pmkr1_name);

VOID FT_DerivePMKR1(
	IN	PUINT8	pmkr0,
	IN	PUINT8	pmkr0_name,
	IN	PUINT8 	r1khid,
	IN	PUINT8	s1khid,
	OUT	PUINT8	pmkr1,
	OUT	PUINT8	pmkr1_name);

VOID FT_DerivePTK(
	IN	PUINT8	pmkr1,
	IN	PUINT8	pmkr1_name,
	IN	PUINT8 	a_nonce,
	IN	PUINT8 	s_nonce,
	IN	PUINT8 	bssid,
	IN	PUINT8 	sta_mac,
	IN	UINT	key_len,
	OUT	PUINT8	ptk,
	OUT	PUINT8	ptk_name);

VOID	FT_CalculateMIC(
	IN	PUINT8		sta_addr,	
	IN	PUINT8		ap_addr,
	IN	PUINT8		kck,
	IN	UINT8		seq,
	IN  PUINT8		rsnie,
	IN	UINT8		rsnie_len,
	IN	PUINT8		mdie,
	IN	UINT8		mdie_len,
	IN	PUINT8		ftie,
	IN	UINT8		ftie_len,
	IN	PUINT8		ric,
	IN	UINT8		ric_len,
	OUT PUINT8		mic);


void FT_rtmp_read_parameters_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *pBuffer);

INT Set_FT_Enable(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_FT_Mdid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_FT_R0khid(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_FT_RIC(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_FT_OTD(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT	Show_FTConfig_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* __FT_H */
#endif /* DOT11R_FT_SUPPORT */

