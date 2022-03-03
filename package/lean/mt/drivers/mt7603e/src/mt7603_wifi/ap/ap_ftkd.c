/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
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

	All related IEEE802.11r Key Distribution Protocol (FT KDP) body.

	Overview:

	1. A station associates to us, send out a broadcast ADD-Notify packet.

		ASSOC -->
		FT_KDP_EVENT_INFORM(FT_KDP_SIG_FT_ASSOCIATION) -->
		FT_KDP_EventInform(FT_KDP_SIG_FT_ASSOCIATION) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send ADD-Notify packet, IAPP_UDP_PacketSend(), IAPP_L2UpdateFrameSend()

	2. When receiving a ADD-Notify packet, send a unicast SSB packet to
		request PMK-R1 key for the station with our R0KH.

		IAPP daemon, IAPP_RcvHandlerUdp() -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_StationInform() -->
		Notify IAPP daemon, FT_KDP_EventInform(FT_KDP_SIG_KEY_REQ_AUTO) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send SSB packet with R0KHID = 0, by using TCP or UDP based on peerIP

	3. A station reassociates to us, send out a MOVE-Request packet.

		REASSOC -->
		FT_KDP_EVENT_INFORM(FT_KDP_SIG_FT_REASSOCIATION) -->
		FT_KDP_EventInform(FT_KDP_SIG_FT_REASSOCIATION) -->
		Notify IAPP daemon, IAPP_RcvHandlerRawDrv(), IAPP_SIG_Process() -->
		Send MOVE-Request packet by using TCP or UDP, IAPP_L2UpdateFrameSend()

	4. When receiving a MOVE-Request packet, delete the STA MAC entry.

		IAPP daemon, IAPP_RcvHandlerUdp()/ IAPP_RcvHandlerTcp() -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		RT_SET_DEL_MAC_ENTRY -->
		Send MOVE-Response packet by using TCP, FT_KDP_MoveFrameSend()

	5. When receiving a MOVE-Response packet, nothing to do.

	6. When receiving a SSB packet (i.e. key request), send a unicast SAB
		packet to response the key to the R1KH.

		IAPP daemon -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_IOCTL_KEY_REQ() -->
		Notify IAPP daemon, FT_KDP_EventInform(FT_KDP_SIG_KEY_RSP_AUTO) -->
		Send SAB packet with my R0KHID, FT_KDP_SecurityBlockSend() by using TCP

	7. When receiving a SAB packet (i.e. key response), set the PMK-R1 key.

		IAPP daemon -->
		Notify driver, IAPP_MsgProcess() -->
		IOCTL, RTMPAPSetInformation() -->
		FT_KDP_KeyResponseToUs()

	8. Send a information broadcast to the LAN periodically.

***************************************************************************/

#ifdef DOT11R_FT_SUPPORT

#include "rt_config.h"
#include "ft_cmm.h"

#define TYPE_FUNC
#define FT_KDP_DEBUG
#define FT_KDP_FUNC_TEST
/*#define FT_KDP_EMPTY */ /* empty codes to debug */

#define IAPP_SHOW_IP_HTONL(__IP)	\
	(htonl(__IP) >> 24) & 0xFF,		\
	(htonl(__IP) >> 16) & 0xFF,		\
	(htonl(__IP) >> 8) & 0xFF,		\
	(htonl(__IP) & 0xFF)

#define IAPP_DAEMON_CMD_PARSE(__pInfo, __InfoLen, __PeerIP, __pData, __DataLen)	\
	NdisMoveMemory(&__PeerIP, __pInfo, FT_IP_ADDRESS_SIZE);						\
	__pData = (UCHAR *)(__pInfo + FT_IP_ADDRESS_SIZE);							\
	__DataLen = __InfoLen - FT_IP_ADDRESS_SIZE;

/* private variable */
UINT32 FT_KDP_MemAllocNum, FT_KDP_MemFreeNum;

/* extern variable */
#ifdef FT_KDP_FUNC_TEST
extern UCHAR gFT_MAC_OldAP[];
#endif /* FT_KDP_FUNC_TEST */

/* private function prototype */
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
BOOLEAN FT_KDP_R0KH_InfoAdd(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pR0KHID,
	IN	UCHAR				*pMAC,
	IN	UINT32				IP);
VOID FT_KDP_R0KH_InfoDel(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pR0KHID,
	IN	UCHAR				*pMAC,
	IN	UINT32				IP);
static BOOLEAN FT_KDP_R0KH_InfoGet(
	IN		PRTMP_ADAPTER	pAd,
	INOUT	UCHAR			*pR0KHID,
	INOUT	UCHAR			*pMAC,
	OUT		UINT32			*pIP);
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */




/* public function */
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
VOID TYPE_FUNC FT_KDP_Init(
	IN	PRTMP_ADAPTER		pAd)
{
#ifndef FT_KDP_EMPTY
	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 1)
		return;

	DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Initialize FT KDP Module...\n"));

	/* allocate control block */
	FT_MEM_ALLOC(pAd, &pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK, sizeof(FT_KDP_CTRL_BLOCK));
	if (pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Allocate control block fail!\n"));
		return;
	}

	NdisAllocateSpinLock(pAd, &(pAd->ApCfg.FtTab.FT_KdpLock));

#ifndef FT_KDP_FUNC_SOCK_COMM
	/* init event list */
	initList(&(FT_KDP_CB->EventList));
#endif /* FT_KDP_FUNC_SOCK_COMM */

#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
	/* init R0KH information record */
	FT_KDP_CB->R0KH_InfoHead = NULL;
	FT_KDP_CB->R0KH_InfoTail = NULL;
	FT_KDP_CB->R0KH_Size = 0;
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

#ifdef FT_KDP_FUNC_INFO_BROADCAST
	RTMPInitTimer(pAd, &FT_KDP_CB->TimerReport,
				GET_TIMER_FUNCTION(FT_KDP_InfoBroadcast),
				(PVOID)pAd, TRUE);
	RTMPSetTimer(&FT_KDP_CB->TimerReport, FT_KDP_INFO_BC_PERIOD_TIME);
#endif /* FT_KDP_FUNC_INFO_BROADCAST */
#endif /* FT_KDP_EMPTY */

	pAd->ApCfg.FtTab.FlgIsFtKdpInit = 1;
}


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
VOID TYPE_FUNC FT_KDP_Release(
	IN	PRTMP_ADAPTER		pAd)
{
#ifndef FT_KDP_EMPTY
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
	FT_KDP_R0KH_INFO *pInfoHead, *pInfo;
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
		return;

	DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Release FT KDP Module...\n"));

#ifndef FT_KDP_FUNC_SOCK_COMM
	/* free event list */
	FT_KDP_EVT_LIST_EMPTY(pAd, &(FT_KDP_CB->EventList));
#endif /* FT_KDP_FUNC_SOCK_COMM */

#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
	/* free all R0KH information */
	pInfoHead = FT_KDP_CB->R0KH_InfoHead;
	while(pInfoHead != NULL)
	{
		pInfo = pInfoHead;
		pInfoHead = pInfoHead->pNext;

		FT_MEM_FREE(pAd, pInfo);
	}
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */

	if (pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK != NULL)
	{
		FT_MEM_FREE(pAd, pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK);
		pAd->ApCfg.FtTab.pFT_KDP_Ctrl_BK = NULL;
	}

#ifdef FT_KDP_FUNC_INFO_BROADCAST
{
	BOOLEAN Status;
	RTMPCancelTimer(&FT_KDP_CB->TimerReport, &Status);
}
#endif /* FT_KDP_FUNC_INFO_BROADCAST */

	/* free spin lock */
	NdisFreeSpinLock(&(pAd->ApCfg.FtTab.FT_KdpLock));
#endif /* FT_KDP_EMPTY */

	pAd->ApCfg.FtTab.FlgIsFtKdpInit = 0;
}


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
VOID TYPE_FUNC FT_KDP_EventInform(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT32				ApIdx,
	IN	UCHAR				EventId,
	IN	VOID				*pEvent,
	IN	UINT16				EventLen,
	IN	UINT32				PeerIP,
	IN	VOID				*pCB)
{
#ifndef FT_KDP_EMPTY
#ifndef FT_KDP_FUNC_SOCK_COMM
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	UCHAR Size;
#endif /* FT_KDP_FUNC_SOCK_COMM */
	FT_KDP_SIGNAL *pFtKdp;
	FT_KDP_EVT_HEADER EvtHdr;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

#ifndef FT_KDP_FUNC_SOCK_COMM
	CHECK_PID_LEGALITY(pObj->IappPid)
	{
		/* valid, can do next step */
	}
	else
		return;

	FT_KDP_EVT_LIST_SIZE_GET(pAd, &FT_KDP_CB->EventList, Size);
	if (Size > FT_KDP_EVENT_MAX)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ap_ftkd> Queued Events are too much!\n"));
		return;
	}
#endif /* FT_KDP_FUNC_SOCK_COMM */

	/* make up event content */
	NdisZeroMemory(&EvtHdr, sizeof(FT_KDP_EVT_HEADER));
	FT_MEM_ALLOC(pAd, &pFtKdp, sizeof(FT_KDP_SIGNAL));
	if (pFtKdp == NULL)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ap_ftkd> Allocate signal content fail!\n"));
		return;
	}
	pFtKdp->Sig = EventId;

	/* handle different event */
	switch(EventId)
	{
		case FT_KDP_SIG_FT_ASSOCIATION:
		{
			/* a station associates to us */

			/*
				When a STA uses the 802.11 Association Request, rather than
				the Reassociation Request, the IAPP may not be able to notify
				the AP at which the STA was previously associated of the new
				association.
			*/

			FT_KDP_EVT_ASSOC *pEvtAssoc;

			pEvtAssoc = (FT_KDP_EVT_ASSOC *)pEvent;

			pFtKdp->Sequence = pEvtAssoc->SeqNum;
			NdisMoveMemory(pFtKdp->MacAddr, pEvtAssoc->MacAddr, MAC_ADDR_LEN);
		}
		break;

		case FT_KDP_SIG_FT_REASSOCIATION:
		{
			/* a station reassociates to us */

			/*
				IEEE802.11f, 1.3 Inter-AP recommended practice overview

				In particular, the IAPP entity must be able to find and use
				a RADIUS server to look up the IP addresses of other APs in
				the ESS when given the Basic Service Set Identifier (BSSIDs)
				of those other APs (if a local capability to perform such a
				translation is not present), and to obtain security information
				to protect the content of certain IAPP packets.

				Here we dont use any Radius server so sometimes we can not
				send the MOVE-notify frame.
			*/

			/*
				The IAPP is not a routing protocol. The IAPP does not deal
				directly with the delivery of 802.11 data frames to the STA;
				instead the DS utilizes existing network functionality for
				data frame delivery.

				When a STA associates or reassociates, the STA must ascertain
				that its network layer address(es) is configured such that the
				normal routing functions of the network attaching to the BSS
				will correctly deliver the STA¡¦s traffic to the BSS to which
				it is associated.

				Two mechanisms for a STA to accomplish this are to renew a
				Dynamic Host Configuration Protocol (DHCP) lease for its IP
				address and to use Mobile IP to obtain a local IP address.
			*/

			FT_KDP_EVT_REASSOC *pEvtReAssoc;

			pEvtReAssoc = (FT_KDP_EVT_REASSOC *)pEvent;

			pFtKdp->Sequence = pEvtReAssoc->SeqNum;
			NdisMoveMemory(pFtKdp->MacAddr, pEvtReAssoc->MacAddr, MAC_ADDR_LEN);

			/* try to get the IP of old AP */
			if (FT_KDP_R0KH_InfoGet(pAd, NULL,
									pEvtReAssoc->OldApMacAddr,
									&PeerIP) == FALSE)
			{
			}
		}
		break;


		case FT_KDP_SIG_KEY_TIMEOUT:
			/* PMK-R1 Key timeout */
			break;


		case FT_KDP_SIG_KEY_REQ:
		{
			FT_KDP_EVT_KEY_ELM *pEvtKeyReq;

			/* PMK-R1 Key Request */
			if (PeerIP != 0)
				break;

			/* try to find the Peer IP address */
			pEvtKeyReq = (FT_KDP_EVT_KEY_ELM *)pEvent;

			if (FT_KDP_R0KH_InfoGet(pAd, pEvtKeyReq->KeyInfo.R0KHID,
									NULL, &PeerIP) == FALSE)
			{

				/* send a broadcast key request packet */
			}
		}
		break;


		case FT_KDP_SIG_ACTION:
		{
			/* forward FT Action frame by using RRB protocol */
			FT_KDP_EVT_ACTION *pActionCB = (FT_KDP_EVT_ACTION *)pCB;

			if (pCB == NULL)
			{
				DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> pCB == NULL!\n"));
				FT_MEM_FREE(pAd, pFtKdp);
				return;
			}

			pFtKdp->Sequence = pActionCB->RequestType;
			NdisMoveMemory(pFtKdp->MacAddr, pActionCB->MacDa, ETH_ALEN);
			NdisMoveMemory(pFtKdp->MacAddrSa, pActionCB->MacSa, ETH_ALEN);
			NdisMoveMemory(pFtKdp->CurrAPAddr, pActionCB->MacAp, ETH_ALEN);
		}
		break;


		case FT_KDP_SIG_KEY_REQ_AUTO:
			/* PMK-R1 Key Request */
			break;


		case FT_KDP_SIG_KEY_RSP_AUTO:
			/* PMK-R1 Key Response */
			break;


		case FT_KDP_SIG_INFO_BROADCAST:
			/* broadcast our AP information to neighbor APs */
			break;


		case FT_KDP_SIG_AP_INFO_REQ:
			/* request neighbor AP information */
			break;


		case FT_KDP_SIG_AP_INFO_RSP:
			/* response my information to a AP */
			break;


		case FT_KSP_SIG_DEBUG_TRACE:
			/* change the debug level of IAPP daemon */
			break;


		case FT_KDP_SIG_TERMINATE:
			/* terminate FT KDP daemon */
			break;


		default:
			/* error event */
			DBGPRINT(RT_DEBUG_TRACE,
					("ap_ftkd> Signal is not supported %d!\n", EventId));
			FT_MEM_FREE(pAd, pFtKdp);
			return;
	}

	/* init event content */
	EvtHdr.PeerIpAddr = PeerIP;
	EvtHdr.EventLen = EventLen;

	NdisMoveMemory(pFtKdp->Content, &EvtHdr, sizeof(FT_KDP_EVT_HEADER));

	if (pEvent != NULL)
	{
		NdisMoveMemory(pFtKdp->Content+sizeof(FT_KDP_EVT_HEADER),
						pEvent, EventLen);
	}

	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Event ID = %d, EventLen = %d (%d, %d)\n",
			EventId, EventLen, FT_KDP_MemAllocNum, FT_KDP_MemFreeNum));

#ifndef FT_KDP_FUNC_SOCK_COMM
	/* insert to the event queue */
	FT_KDP_EVT_LIST_INSERT_TAIL(pAd, &FT_KDP_CB->EventList, pFtKdp);

	/* inform FT KDP daemon to handle the event */
	if ((EventId == FT_KDP_SIG_TERMINATE) ||
		(EventId == FT_KSP_SIG_DEBUG_TRACE) ||
		(EventId == FT_KDP_SIG_FT_ASSOCIATION) ||
		(EventId == FT_KDP_SIG_FT_REASSOCIATION) ||
		(EventId == FT_KDP_SIG_KEY_REQ) ||
		(EventId == FT_KDP_SIG_KEY_TIMEOUT))
	{
		/*
			Note: in VxWorks, we can not send any signal to same task
			which issues a ioctl path.
		*/

		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Send signal to ft kdp daemon... (EventLen = %d)\n",
				EventLen));
		SendSignalToDaemon(SIGUSR2, pObj->IappPid, pObj->IappPid_nr);
	} /* End of if */
#endif /* FT_KDP_FUNC_SOCK_COMM */

#ifdef FT_KDP_FUNC_SOCK_COMM
	/* inform FT KDP daemon to handle the event */
{
	FT_DRV_ETH_HEADER Hdr8023, *pHdr8023 = &Hdr8023;
	NDIS_PACKET *pPktComm;
	UINT32 FrameLen;
#ifdef FT_OS_VXWORKS
	END_OBJ *pEndEth;
#endif /* FT_OS_VXWORKS */

	/* allocate a rx packet */
	pPktComm = RtmpOSNetPktAlloc(pAd, sizeof(RT_SIGNAL_STRUC)+LENGTH_802_3);
	if (pPktComm != NULL)
	{
		/* make up 802.3 header */
		NdisMoveMemory(pHdr8023->DA, pAd->ApCfg.MBSSID[ApIdx].wdev.bssid, MAC_ADDR_LEN);

		/* can not send a packet with same SA & DA in 5VT board */
/*		NdisMoveMemory(pHdr8023->SA, pAd->ApCfg.MBSSID[ApIdx].Bssid, 6); */
		NdisZeroMemory(pHdr8023->SA, ETH_ALEN);
		pHdr8023->SA[5] = 0x01;
		pHdr8023->Len = IAPP_ETH_PRO;

		/* convert 802.11 to 802.3 packet */
#ifdef FT_OS_VXWORKS
		pEndEth = endFindByName("mirror", 0);
		if (pEndEth != NULL)
		{
			/*
				Sometimes if I use ra0 to send the command packet, daemon
				can not receive the command packet!
				So I use bridge to do the receive network interface.

				Support IEEE802.11r DS, must have bridge network interface.
			*/
			SET_OS_PKT_NETDEV(pPktComm, pEndEth->devObject.pDevice);
		}
		else
#endif /* FT_OS_VXWORKS */
		SET_OS_PKT_NETDEV(pPktComm, get_netdev_from_bssid(pAd, ApIdx));

		OS_PKT_RESERVE(RTPKT_TO_OSPKT(pPktComm), 2); /* 16 byte align the IP header */
		NdisMoveMemory(OS_PKT_TAIL_BUF_EXTEND(RTPKT_TO_OSPKT(pPktComm), LENGTH_802_3),
						pHdr8023, LENGTH_802_3);

		FrameLen = RT_SIGNAL_STRUC_HDR_SIZE+sizeof(FT_KDP_EVT_HEADER)+EventLen;
		NdisMoveMemory(OS_PKT_TAIL_BUF_EXTEND(RTPKT_TO_OSPKT(pPktComm), FrameLen),
						pFtKdp, FrameLen);

		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Send a command (MBSS%d, Frame Len%d) to IAPP daemon.\n",
				ApIdx, FrameLen));

		/* pass this packet to upper layer */
		announce_802_3_packet(pAd, pPktComm, OPMODE_AP);
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Allocate signal to ft kdp daemon fail!\n"));
	}
}
#endif

#ifdef FT_KDP_FUNC_SOCK_COMM
	FT_MEM_FREE(pAd, pFtKdp);
#endif /* FT_KDP_FUNC_SOCK_COMM */
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_KDP_EventGet(
	IN	PRTMP_ADAPTER		pAd,
	OUT	FT_KDP_SIGNAL		**pFtKdp)
{
#ifndef FT_KDP_EMPTY
#ifndef FT_KDP_FUNC_SOCK_COMM
	UCHAR Size;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	FT_KDP_EVT_LIST_SIZE_GET(pAd, &FT_KDP_CB->EventList, Size);

	/* check if any event is queued */
	if (Size <= 0)
	{
		*pFtKdp = NULL;
		return;
	}

	/* return a event */
	FT_KDP_EVT_LIST_REMOVE_HEAD(pAd, &FT_KDP_CB->EventList, (*pFtKdp));
#else

	*pFtKdp = NULL;
#endif /* FT_KDP_FUNC_SOCK_COMM */
#endif /* FT_KDP_EMPTY */
}


/*
========================================================================
Routine Description:
	Inform us a AP request the key.

Arguments:
	pAd				- WLAN control block pointer
	PeerIP			- the IP of R1KH, must not be 0
	*pNonce			- the nonce for key request
	*pEvtKeyReq		- information for key request

Return Value:
	TRUE			- request ok
	FALSE			- can not find the IP of R0KH

Note:
	If PeerIP == 0, search its IP from our information record.
========================================================================
*/
BOOLEAN TYPE_FUNC FT_KDP_KeyRequestToUs(
	IN	PRTMP_ADAPTER		pAd,
	IN	UINT32				PeerIP,
	IN	UCHAR				*pNonce,
	IN	FT_KDP_EVT_KEY_ELM	*pEvtKeyReq)
{
#ifndef FT_KDP_EMPTY
	UINT32 IDR0KH;
	UINT32 ApIdx;


	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Key Req from Peer IP = %d.%d.%d.%d!\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return FALSE;
	}

	if (PeerIP == 0)
		return FALSE;

	/* sanity check for R0KHID */
#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Key Req: R0KH-ID = "
			"0x%02x:%02x:%02x:%02x:%02x:%02x!\n",
			pEvtKeyReq->KeyInfo.R0KHID[0],
			pEvtKeyReq->KeyInfo.R0KHID[1],
			pEvtKeyReq->KeyInfo.R0KHID[2],
			pEvtKeyReq->KeyInfo.R0KHID[3],
			pEvtKeyReq->KeyInfo.R0KHID[4],
			pEvtKeyReq->KeyInfo.R0KHID[5]));
#endif /* FT_KDP_DEBUG */

	ApIdx = BSS0;

	for(IDR0KH=0; IDR0KH<FT_KDP_R0KHID_MAX_SIZE; IDR0KH++)
	{
		if (pEvtKeyReq->KeyInfo.R0KHID[IDR0KH] != 0x00)
		{
			/* check if the R0KHID is same as us (MBSS); if not, return FALSE */
			/* yet implement */
		}
	}

	/* check if the request format is correct */
	if ((pEvtKeyReq->ElmId == FT_KDP_ELM_ID_PRI) &&
		(pEvtKeyReq->OUI[0] == FT_KDP_ELM_PRI_OUI_0) &&
		(pEvtKeyReq->OUI[1] == FT_KDP_ELM_PRI_OUI_1) &&
		(pEvtKeyReq->OUI[2] == FT_KDP_ELM_PRI_OUI_2))
	{
#ifdef FT_KDP_DEBUG
		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Key Req: Station MAC = "
				"0x%02x:%02x:%02x:%02x:%02x:%02x!\n",
				pEvtKeyReq->MacAddr[0],
				pEvtKeyReq->MacAddr[1],
				pEvtKeyReq->MacAddr[2],
				pEvtKeyReq->MacAddr[3],
				pEvtKeyReq->MacAddr[4],
				pEvtKeyReq->MacAddr[5]));

		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Key Req: R1KH-ID = "
				"0x%02x:%02x:%02x:%02x:%02x:%02x!\n",
				pEvtKeyReq->KeyInfo.R1KHID[0],
				pEvtKeyReq->KeyInfo.R1KHID[1],
				pEvtKeyReq->KeyInfo.R1KHID[2],
				pEvtKeyReq->KeyInfo.R1KHID[3],
				pEvtKeyReq->KeyInfo.R1KHID[4],
				pEvtKeyReq->KeyInfo.R1KHID[5]));
#endif /* FT_KDP_DEBUG */

		/* calculate the PMK-R1 Key for the station vs. the AP */
		if (FT_QueryKeyInfoForKDP(pAd, ApIdx, pEvtKeyReq) == FALSE)
		{
			return FALSE;
		}

		/* response the requested PMK-R1 Key to the R1KH */
		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Key Rsp to Peer IP = %d.%d.%d.%d!\n",
				IAPP_SHOW_IP_HTONL(PeerIP)));

		FT_KDP_EventInform(pAd, ApIdx, FT_KDP_SIG_KEY_RSP_AUTO,
							(VOID *)pEvtKeyReq, sizeof(FT_KDP_EVT_KEY_ELM),
							PeerIP, NULL);
		return TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ap_ftkd> Key Req format incorrect!\n"));
	}
#endif /* FT_KDP_EMPTY */

	return FALSE;
}


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
VOID TYPE_FUNC FT_KDP_KeyResponseToUs(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	UINT32 PeerIP, DataLen = 0;
	UCHAR *pData;

	FT_KDP_EVT_KEY_ELM *pEvtKeyRsp;
	UCHAR *pNonce;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	IAPP_DAEMON_CMD_PARSE(pInfo, InfoLen, PeerIP, pData, DataLen);
	DBGPRINT(RT_DEBUG_ERROR,
			("ap_ftkd> DataLen = %d!\n", DataLen));

	pNonce = pData;
	pEvtKeyRsp = (FT_KDP_EVT_KEY_ELM *)(pNonce + FT_NONCE_SIZE);

#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Key Rsp from Peer IP = %d.%d.%d.%d!\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));
#endif /* FT_KDP_DEBUG */

	/* check if the response format is correct */
	if ((pEvtKeyRsp->ElmId == FT_KDP_ELM_ID_PRI) &&
		(pEvtKeyRsp->OUI[0] == FT_KDP_ELM_PRI_OUI_0) &&
		(pEvtKeyRsp->OUI[1] == FT_KDP_ELM_PRI_OUI_1) &&
		(pEvtKeyRsp->OUI[2] == FT_KDP_ELM_PRI_OUI_2))
	{
		PFT_R1HK_ENTRY 	pR1hkEntry;
		BOOLEAN			bUpdateR1kh = FALSE;
	
#ifdef FT_KDP_DEBUG
		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Key Rsp: Station MAC = "
				"0x%02x:%02x:%02x:%02x:%02x:%02x!\n",
				pEvtKeyRsp->MacAddr[0],
				pEvtKeyRsp->MacAddr[1],
				pEvtKeyRsp->MacAddr[2],
				pEvtKeyRsp->MacAddr[3],
				pEvtKeyRsp->MacAddr[4],
				pEvtKeyRsp->MacAddr[5]));
	
		DBGPRINT(RT_DEBUG_TRACE,
				("ap_ftkd> Key Rsp: R1KH-ID = "
				"0x%02x:%02x:%02x:%02x:%02x:%02x!\n",
				pEvtKeyRsp->KeyInfo.R1KHID[0],
				pEvtKeyRsp->KeyInfo.R1KHID[1],
				pEvtKeyRsp->KeyInfo.R1KHID[2],
				pEvtKeyRsp->KeyInfo.R1KHID[3],
				pEvtKeyRsp->KeyInfo.R1KHID[4],
				pEvtKeyRsp->KeyInfo.R1KHID[5]));

		DBGPRINT(RT_DEBUG_TRACE, ("MacAddr"));
		hex_dump("PMKR1Name", pEvtKeyRsp->PMKR1Name, FT_KDP_WPA_NAME_MAX_SIZE);
		hex_dump("R0KH_MAC", pEvtKeyRsp->R0KH_MAC, ETH_ALEN);
		hex_dump("AKM Suite", pEvtKeyRsp->AkmSuite, 4);

#endif /* FT_KDP_DEBUG */

		/*	Upon receiving a new PMK-R1 key for a STA, the target AP shall
			delete the prior PMK-R1 SA and PTKSAs derived from the prior 
			PMK-R1 key */
		pR1hkEntry = FT_R1khEntryTabLookup(pAd, pEvtKeyRsp->PMKR1Name);
		if (pR1hkEntry == NULL)
		{
			bUpdateR1kh = TRUE;
		}
		if (pR1hkEntry && 
			RTMPEqualMemory(pR1hkEntry->PmkR1Key, pEvtKeyRsp->PMKR1, LEN_PMK) == FALSE)
		{
			FT_R1khEntryDelete(pAd, pR1hkEntry);
			bUpdateR1kh = TRUE;
		}

		if (bUpdateR1kh)
		{
			MAC_TABLE_ENTRY *pEntry = NULL;

			pEntry = MacTableLookup(pAd, pEvtKeyRsp->MacAddr);
			/* assign the PMK-R1 key to FT kernel */
			FT_R1khEntryInsert(pAd,
							pEvtKeyRsp->KeyInfo.PMKR0Name,
							pEvtKeyRsp->PMKR1Name,
							pEvtKeyRsp->PMKR1,
							pEvtKeyRsp->PairwisChipher,
							pEvtKeyRsp->AkmSuite,
							pEvtKeyRsp->KeyLifeTime,
							pEvtKeyRsp->ReassocDeadline,
							(PUINT8)pEvtKeyRsp->KeyInfo.R0KHID,
							pEvtKeyRsp->KeyInfo.R0KHIDLen,
							pEvtKeyRsp->MacAddr);
			/* YF_FT */
			if (pEntry && ((pEntry->FT_R1kh_CacheMiss_Times > 0)
#ifdef R1KH_HARD_RETRY	/* yiwei no give up! */
				|| (pEntry->FT_R1kh_CacheMiss_Hard > 0)
#endif /*R1KH_HARD_RETRY */
				)) {
				MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s - Reset FT_R1kh_CacheMiss_Times to Zero (Wcid%d, value:%d), time=%ld\n",
				__func__, pEntry->wcid, pEntry->FT_R1kh_CacheMiss_Times, (jiffies * 1000) / OS_HZ));
				pEntry->FT_R1kh_CacheMiss_Times = 0;
#ifdef R1KH_HARD_RETRY	/* yiwei no give up! */
				pEntry->FT_R1kh_CacheMiss_Hard = 0;
				RTMP_OS_COMPLETE(&pEntry->ack_r1kh);
#endif /* R1KH_HARD_RETRY */
				}
		}

#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
		/* add the R0KH information */
		FT_KDP_R0KH_InfoAdd(pAd,
							pEvtKeyRsp->KeyInfo.R0KHID,
							pEvtKeyRsp->R0KH_MAC,
							PeerIP);
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ap_ftkd> Key Rsp format incorrect!\n"));
	}
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_KDP_StationInform(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	UINT32 PeerIP, DataLen = 0;
	UCHAR *pData;

	FT_KDP_EVT_KEY_ELM EvtKeyReq, *pEvtKeyReq;
	UCHAR StaMac[ETH_ALEN];
	UINT32 IdBssNum;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	IAPP_DAEMON_CMD_PARSE(pInfo, InfoLen, PeerIP, pData, DataLen);
	DBGPRINT(RT_DEBUG_ERROR,
		("ap_ftkd> %s: DataLen = %d!\n", __FUNCTION__, DataLen));

#ifndef FT_KDP_FUNC_TEST
	/* check if we are in security mode; if not, return */
	for(IdBssNum=0; IdBssNum<pAd->ApCfg.BssidNum; IdBssNum++)
	{
		if (pAd->ApCfg.MBSSID[IdBssNum].WepStatus != Ndis802_11WEPDisabled)
			break;
	}

	if (IdBssNum == pAd->ApCfg.BssidNum)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("ap_ftkd> All BSS are open mode!\n"));
		return; /* all BSS are open mode */
	}
#endif

	NdisMoveMemory(StaMac, pData, ETH_ALEN);

	NdisZeroMemory(&EvtKeyReq, sizeof(FT_KDP_EVT_KEY_ELM));
	pEvtKeyReq = &EvtKeyReq;

#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Station Inform from Peer IP = %d.%d.%d.%d!\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));
#endif

	/* make up request content */
	pEvtKeyReq->ElmId = FT_KDP_ELM_ID_PRI;
	pEvtKeyReq->ElmLen = FT_KDP_ELM_PRI_LEN;
	pEvtKeyReq->OUI[0] = FT_KDP_ELM_PRI_OUI_0;
	pEvtKeyReq->OUI[1] = FT_KDP_ELM_PRI_OUI_1;
	pEvtKeyReq->OUI[2] = FT_KDP_ELM_PRI_OUI_2;

	NdisMoveMemory(pEvtKeyReq->MacAddr, StaMac, ETH_ALEN);
	NdisMoveMemory(pEvtKeyReq->KeyInfo.S1KHID, StaMac, FT_KDP_S1KHID_MAX_SIZE);

	/* request PMK-R1 Key (our R1KH vs. the station) from the R0KH */
#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Send Key Req to Peer IP = %d.%d.%d.%d!\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));
#endif /* FT_KDP_DEBUG */

	for(IdBssNum=0; IdBssNum<pAd->ApCfg.BssidNum; IdBssNum++)
	{
#ifndef FT_KDP_FUNC_TEST
		if (pAd->ApCfg.MBSSID[IdBssNum].WepStatus != Ndis802_11WEPDisabled)
#endif /* FT_KDP_FUNC_TEST */
		{
			/* copy our MAC address to be the R1KHID */
			/* different MAC for different BSS in MBSS mode */
			NdisMoveMemory(pEvtKeyReq->KeyInfo.R1KHID,
							pAd->ApCfg.MBSSID[IdBssNum].wdev.bssid,
							FT_KDP_R1KHID_MAX_SIZE);

			FT_KDP_EventInform(pAd, IdBssNum, FT_KDP_SIG_KEY_REQ_AUTO,
								(VOID *)pEvtKeyReq, sizeof(FT_KDP_EVT_KEY_ELM),
								PeerIP, NULL);
		}
	}
#endif /* FT_KDP_EMPTY */
}


/*
========================================================================
Routine Description:
	Set the encryption/decryption key from daemon or driver.

Arguments:
	pAd				- WLAN control block pointer
	*pKey			- the key
	KeyLen			- the key length

Return Value:
	None

Note:
	Key length can be 8B ~ 64B.

	We support key set from daemon or driver.

	Only one key for multiple BSS mode.
========================================================================
*/
VOID TYPE_FUNC FT_KDP_CryptKeySet(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pKey,
	IN	INT32				KeyLen)
{
#ifndef FT_KDP_EMPTY
	UCHAR KeyMaterial[40];

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* key set */
	if ((KeyLen >= 8) && (KeyLen <= 64))
	{
		RtmpPasswordHash((RTMP_STRING *)pKey,
					(PUCHAR)FT_KDP_RALINK_PASSPHRASE,
					strlen(FT_KDP_RALINK_PASSPHRASE),
					KeyMaterial);
	}
	else
	{
		RtmpPasswordHash((RTMP_STRING *)FT_KDP_KEY_DEFAULT,
					(PUCHAR)FT_KDP_RALINK_PASSPHRASE,
					strlen(FT_KDP_RALINK_PASSPHRASE),
					KeyMaterial);
	}

	/* use the first 16B of KeyMaterial[] */
	NdisMoveMemory(FT_KDP_CB->CryptKey, KeyMaterial, sizeof(KeyMaterial));
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_KDP_DataEncrypt(
	IN		PRTMP_ADAPTER		pAd,
	INOUT	UCHAR				*pData,
	IN		UINT32				*pDataLen)
{
#ifndef FT_KDP_EMPTY
	UCHAR *pOutputBuf;
	UINT   OutputLen = 0;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	FT_MEM_ALLOC(pAd, &pOutputBuf, *pDataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);
	if (pOutputBuf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Allocate encryption buffer fail!\n"));
		return;
	}
	NdisZeroMemory(pOutputBuf, *pDataLen + FT_KDP_KEY_ENCRYPTION_EXTEND);

	/* encrypt */
	AES_Key_Wrap(pData, *pDataLen, FT_KDP_CB->CryptKey, 16, 
				 pOutputBuf, &OutputLen);	
	*pDataLen = OutputLen;

	NdisMoveMemory(pData, pOutputBuf, *pDataLen);

	FT_MEM_FREE(pAd, pOutputBuf);
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_KDP_DataDecrypt(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pData,
	IN	UINT32				*pDataLen)
{
#ifndef FT_KDP_EMPTY
	UCHAR *pOutputBuf;
	UINT   OutputLen = 0;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	if ((*pDataLen) <= FT_KDP_KEY_ENCRYPTION_EXTEND)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd>Decryption length is too small!\n"));
		return;
	}

	/* init */
	FT_MEM_ALLOC(pAd, &pOutputBuf, *pDataLen);
	if (pOutputBuf == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Allocate decryption buffer fail!\n"));
		return;
	}

	/* decrypt */
	AES_Key_Unwrap(pData, *pDataLen, 
				   FT_KDP_CB->CryptKey, 16, 
				   pOutputBuf, &OutputLen);
	*pDataLen = OutputLen;

	NdisMoveMemory(pData, pOutputBuf, *pDataLen);

	FT_MEM_FREE(pAd, pOutputBuf);
#endif /* FT_KDP_EMPTY */
}


/* private function */
#ifdef FT_KDP_FUNC_R0KH_IP_RECORD
/*
========================================================================
Routine Description:
	Add a R0KH Information.

Arguments:
	pAd				- WLAN control block pointer
	*pR0KHID		- R0KH ID
	*pMAC			- R0KH MAC
	IP				- R0KH IP

Return Value:
	None

Note:
========================================================================
*/
BOOLEAN TYPE_FUNC FT_KDP_R0KH_InfoAdd(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pR0KHID,
	IN	UCHAR				*pMAC,
	IN	UINT32				IP)
{
#ifndef FT_KDP_EMPTY
	FT_KDP_R0KH_INFO *pInfo;
	BOOLEAN FlgIsSameRecordFound;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return FALSE;
	}

	/* sanity check for maximum size */
	if (FT_KDP_CB->R0KH_Size >= FT_KDP_R0KH_INFO_MAX_SIZE)
	{
		/* delete oldest record first */
		FT_KDP_R0KH_InfoDel(pAd, NULL, NULL, 0);
	}

	RTMP_SEM_LOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));

	/* sanity check if same information exists */
	FlgIsSameRecordFound = FALSE;
	pInfo = FT_KDP_CB->R0KH_InfoHead;

	while(pInfo != NULL)
	{
		if ((NdisEqualMemory(pInfo->R0KHID, pR0KHID, sizeof(pInfo->R0KHID))) ||
			(NdisEqualMemory(pInfo->MAC, pMAC, sizeof(pInfo->MAC))) ||
			(pInfo->IP == IP))
		{
			/* need to update the new information to the entry */
			FlgIsSameRecordFound = TRUE;
			break;
		}

		pInfo = pInfo->pNext;
	}

	/* init or update */
	if (FlgIsSameRecordFound == FALSE)
	{
		/* allocate a new record */
		FT_MEM_ALLOC(pAd, &pInfo, sizeof(FT_KDP_R0KH_INFO));
		if (pInfo == NULL)
		{
			RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
			DBGPRINT(RT_DEBUG_ERROR, ("ap_ftkd> Allocate R0KH INFO fail!\n"));
			return FALSE;
		}

		pInfo->pNext = NULL;
	}

	/* update other R0KH device information */
	NdisMoveMemory(pInfo->R0KHID, pR0KHID, sizeof(pInfo->R0KHID));
	NdisMoveMemory(pInfo->MAC, pMAC, sizeof(pInfo->MAC));
	pInfo->IP = IP;
	NdisGetSystemUpTime(&pInfo->TimeUpdate);

	/* append the new information to the information list */
	if (FlgIsSameRecordFound == FALSE)
	{
		if (FT_KDP_CB->R0KH_InfoTail == NULL)
		{
			FT_KDP_CB->R0KH_InfoHead = pInfo;
			FT_KDP_CB->R0KH_InfoTail = pInfo;
		}
		else
		{
			(FT_KDP_CB->R0KH_InfoTail)->pNext = pInfo;
			FT_KDP_CB->R0KH_InfoTail = pInfo;
		}

		FT_KDP_CB->R0KH_Size ++;
	}

	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
#endif /* FT_KDP_EMPTY */

	return TRUE;
}


/*
========================================================================
Routine Description:
	Delete a R0KH Information.

Arguments:
	pAd				- WLAN control block pointer
	*pR0KHID		- R0KH ID
	*pMAC			- R0KH MAC
	IP				- R0KH IP

Return Value:
	None

Note:
	If pR0KHID == NULL, pMAC == NULL, and IP = 0, means deleting the oldest.
========================================================================
*/
VOID TYPE_FUNC FT_KDP_R0KH_InfoDel(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pR0KHID,
	IN	UCHAR				*pMAC,
	IN	UINT32				IP)
{
#ifndef FT_KDP_EMPTY
	FT_KDP_R0KH_INFO *pInfo;
	FT_KDP_R0KH_INFO *pInfoBackup, *pInfoDelPrev, *pInfoDel;
	ULONG TimeOldest;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	pInfoDelPrev = NULL;
	pInfoDel = NULL;
	TimeOldest = 0xFFFFFFFF;

	RTMP_SEM_LOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));

	/* try to find the one expected to delete */
	pInfo = FT_KDP_CB->R0KH_InfoHead;
	if (pInfo == NULL)
	{
		RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
		return;
	}

	pInfoBackup = pInfo;

	while(pInfo != NULL)
	{
		if ((pR0KHID == NULL) && (pMAC == NULL) && (IP == 0))
		{
			/* check if the info is the oldest one */
			if (pInfo->TimeUpdate < TimeOldest)
			{
				TimeOldest = pInfo->TimeUpdate;

				pInfoDelPrev = pInfoBackup;
				pInfoDel = pInfo;
			}
		}
		else
		{
			/* check if the info is the one expected to delete */
			if ((pR0KHID != NULL) &&
				(NdisEqualMemory(pR0KHID, pInfo->R0KHID, sizeof(pInfo->R0KHID))))
			{
				pInfoDelPrev = pInfoBackup;
				pInfoDel = pInfo;
				break;
			}

			if ((pMAC != NULL) &&
				(NdisEqualMemory(pMAC, pInfo->MAC, sizeof(pInfo->MAC))))
			{
				pInfoDelPrev = pInfoBackup;
				pInfoDel = pInfo;
				break;
			}

			if (IP == pInfo->IP)
			{
				pInfoDelPrev = pInfoBackup;
				pInfoDel = pInfo;
				break;
			}
		}

		/* check next one */
		pInfoBackup = pInfo;
		pInfo = pInfo->pNext;
	}

	/* delete the information record */
	if (pInfoDel != NULL)
	{
		FT_KDP_CB->R0KH_Size --;

		if (FT_KDP_CB->R0KH_Size == 0)
		{
			/* list is empty */
			FT_KDP_CB->R0KH_InfoHead = NULL;
			FT_KDP_CB->R0KH_InfoTail = NULL;
		}
		else
		{
			if (pInfoDelPrev == pInfoDel)
			{
				/* must be the head one */
				FT_KDP_CB->R0KH_InfoHead = pInfoDel->pNext;
			}
			else
			{
				/* must not be the head one */
				pInfoDelPrev->pNext = pInfoDel->pNext;
			}
		}
	}
	else
	{
		/* sorry! can not find the one in the list */
	}

	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));

	/* free the one */
	if (pInfoDel != NULL)
		FT_MEM_FREE(pAd, pInfoDel);

#endif /* FT_KDP_EMPTY */
}


/*
========================================================================
Routine Description:
	Get the IP of R0KH by R0KHID or MAC.

Arguments:
	pAd				- WLAN control block pointer
	*pR0KHID		- R0KH ID
	*pMAC			- R0KH MAC
	pIP				- R0KH IP

Return Value:
	TRUE			- get ok
	FALSE			- get fail

Note:
========================================================================
*/
static BOOLEAN TYPE_FUNC FT_KDP_R0KH_InfoGet(
	IN		PRTMP_ADAPTER	pAd,
	INOUT	UCHAR			*pR0KHID,
	INOUT	UCHAR			*pMAC,
	OUT		UINT32			*pIP)
{
#ifndef FT_KDP_EMPTY
	FT_KDP_R0KH_INFO *pInfo;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return FALSE;
	}

	RTMP_SEM_LOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));

	pInfo = FT_KDP_CB->R0KH_InfoHead;

	while(pInfo != NULL)
	{
		/* check if the info is the one expected to get */
		if ((pR0KHID != NULL) &&
			(NdisEqualMemory(pR0KHID, pInfo->R0KHID, sizeof(pInfo->R0KHID))))
		{
			*pIP = pInfo->IP;
			goto LabelOk;
		}

		if ((pMAC != NULL) &&
			(NdisEqualMemory(pMAC, pInfo->MAC, sizeof(pInfo->MAC))))
		{
			*pIP = pInfo->IP;
			goto LabelOk;
		}

		/* check next one */
		pInfo = pInfo->pNext;
	}

	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
	return FALSE;

LabelOk:
	RTMP_SEM_UNLOCK(&(pAd->ApCfg.FtTab.FT_KdpLock));
	return TRUE;
#else

	return FALSE;
#endif /* FT_KDP_EMPTY */
}
#endif /* FT_KDP_FUNC_R0KH_IP_RECORD */


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
VOID TYPE_FUNC FT_KDP_InfoBroadcast(
	IN	PVOID				SystemSpecific1,
	IN	PVOID				FunctionContext,
	IN	PVOID				SystemSpecific2,
	IN	PVOID				SystemSpecific3)
{
#ifndef FT_KDP_EMPTY
#ifdef FT_KDP_FUNC_INFO_BROADCAST
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)FunctionContext;
	UINT32 IdBssNum;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* get my neighbor report element information */
	/* yet implement */

	for(IdBssNum=0; IdBssNum<pAd->ApCfg.BssidNum; IdBssNum++)
	{
		/* PeerIP = 0 means destination MAC = 0xFF FF FF FF FF FF */
		FT_KDP_EventInform(pAd, IdBssNum, FT_KDP_SIG_INFO_BROADCAST,
							"test", 4, 0, NULL);
	}
#endif /* FT_KDP_FUNC_INFO_BROADCAST */
#endif /* FT_KDP_EMPTY */
}


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
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	UINT32 PeerIP, DataLen;
	UCHAR *pData;
	UINT32 IdBssNum;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	IAPP_DAEMON_CMD_PARSE(pInfo, InfoLen, PeerIP, pData, DataLen);

#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Neighbor Report from Peer IP = %d.%d.%d.%d!\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));
#endif /* FT_KDP_DEBUG */

	/* yet implememnt */

#endif /* FT_KDP_EMPTY */
}
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
VOID TYPE_FUNC FT_KDP_NeighborRequestHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	UINT32 PeerIP, DataLen = 0;
	UCHAR *pData;
	UINT16 SsidLen;
	RTMP_STRING *pSSID;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	IAPP_DAEMON_CMD_PARSE(pInfo, InfoLen, PeerIP, pData, DataLen);
	
	SsidLen = DataLen;
	pSSID = (RTMP_STRING *)pData;
	pSSID[SsidLen] = 0x00;

#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Neighbor Request from Peer IP = %d.%d.%d.%d, SSID = %s\n",
			IAPP_SHOW_IP_HTONL(PeerIP), pSSID));
#endif /* FT_KDP_DEBUG */

	/* check if the requested SSID is for us */

	/* 11k yet implememnt */
/*	11K_ApInfoGet(&ApInfo); */

	/* reponse the neighbor request to the AP */
/*	FT_KDP_EventInform(pAd, IdBssNum, FT_KDP_SIG_AP_INFO_RSP, */
/*						&ApInfo, sizeof(ApInfo), PeerIP, NULL); */
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_KDP_NeighborResponseHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	UINT32 PeerIP, DataLen = 0;
	UCHAR *pData = NULL;

	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* init */
	IAPP_DAEMON_CMD_PARSE(pInfo, InfoLen, PeerIP, pData, DataLen);

	if (pData == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: pData is NULL????? (DataLen=%d)\n", __FUNCTION__, DataLen));
		return;
	}
#ifdef FT_KDP_DEBUG
	DBGPRINT(RT_DEBUG_TRACE,
			("ap_ftkd> Neighbor Response from Peer IP = %d.%d.%d.%d\n",
			IAPP_SHOW_IP_HTONL(PeerIP)));
#endif /* FT_KDP_DEBUG */

	/* 11k yet implememnt */
/*	11K_ResponseHandle(pData, DataLen); */
#endif /* FT_KDP_EMPTY */
}


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
VOID TYPE_FUNC FT_RRB_ActionHandle(
	IN	PRTMP_ADAPTER		pAd,
	IN	UCHAR				*pInfo,
	IN	INT32				InfoLen)
{
#ifndef FT_KDP_EMPTY
	if (pAd->ApCfg.FtTab.FlgIsFtKdpInit == 0)
	{
		DBGPRINT(RT_DEBUG_ERROR,
				("ap_ftkd> %s: FT KDP is not init!\n", __FUNCTION__));
		return;
	}

	/* handle it */
	FT_RrbHandler(pAd, pInfo, InfoLen);
#endif /* FT_KDP_EMPTY */
}

#endif /* DOT11R_FT_SUPPORT */

