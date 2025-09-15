/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2007, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    cmm_mat.c

    Abstract:
	    Support Mac Address Translation function.

    Note:
		MAC Address Translation(MAT) engine subroutines, we should just take care 
	 packet to bridge.
		
    Revision History:
    Who             When            What
    --------------  ----------      ----------------------------------------------
    Shiang  		02-26-2007      Init version
*/

#ifdef MAT_SUPPORT

#include "rt_config.h"


extern MATProtoOps MATProtoIPHandle;
extern MATProtoOps MATProtoARPHandle;
extern MATProtoOps MATProtoPPPoEDisHandle;
extern MATProtoOps MATProtoPPPoESesHandle;
extern MATProtoOps MATProtoIPv6Handle;

extern UCHAR SNAP_802_1H[];
extern UCHAR SNAP_BRIDGE_TUNNEL[];

#define MAX_MAT_NODE_ENTRY_NUM	128	/* We support maximum 128 node entry for our system */
#define MAT_NODE_ENTRY_SIZE	40 /*28	// bytes   //change to 40 for IPv6Mac Table */

typedef struct _MATNodeEntry
{
	UCHAR data[MAT_NODE_ENTRY_SIZE];
	struct _MATNodeEntry *next;
}MATNodeEntry, *PMATNodeEntry;


#ifdef KMALLOC_BATCH
/*static MATNodeEntry *MATNodeEntryPoll = NULL; */
#endif

static MATProtoTable MATProtoTb[]=
{
	{ETH_P_IP, 			&MATProtoIPHandle},			/* IP handler */
	{ETH_P_ARP, 		&MATProtoARPHandle},		/* ARP handler */
	{ETH_P_PPP_DISC,	&MATProtoPPPoEDisHandle}, 	/* PPPoE discovery stage handler */
	{ETH_P_PPP_SES,		&MATProtoPPPoESesHandle},	/* PPPoE session stage handler */
	{ETH_P_IPV6,		&MATProtoIPv6Handle},		/* IPv6 handler */
};

#define MAX_MAT_SUPPORT_PROTO_NUM (sizeof(MATProtoTb)/sizeof(MATProtoTable))


/* --------------------------------- Public Function-------------------------------- */
NDIS_STATUS MATDBEntryFree(
	IN MAT_STRUCT 	*pMatStruct, 
	IN PUCHAR 		NodeEntry)
{
#ifdef KMALLOC_BATCH
	MATNodeEntry *pPtr, *pMATNodeEntryPoll;

	pMATNodeEntryPoll = (MATNodeEntry *)pAd->MatCfg.MATNodeEntryPoll;
	pPtr = (MATNodeEntry *)NodeEntry;
	NdisZeroMemory(pPtr, sizeof(MATNodeEntry));
	if (pMATNodeEntryPoll->next)
	{
		pPtr->next = pMATNodeEntryPoll->next;
		pMATNodeEntryPoll->next = pPtr;
	} else {
		pMATNodeEntryPoll->next = pPtr;
	}
#else
	os_free_mem(NULL, NodeEntry);
	NodeEntry = NULL;
#endif

	return TRUE;

}

PUCHAR MATDBEntryAlloc(IN MAT_STRUCT *pMatStruct, IN UINT32 size)
{
#ifdef KMALLOC_BATCH
	MATNodeEntry *pPtr = NULL, *pMATNodeEntryPoll;
	pMATNodeEntryPoll = (MATNodeEntry *)pMatStruct->pMATNodeEntryPoll;
	
	if (pMATNodeEntryPoll->next)
	{
		pPtr = pMATNodeEntryPoll->next;
		pMATNodeEntryPoll->next = pPtr->next;
	}
	
#else
	UCHAR *pPtr = NULL;

	os_alloc_mem(NULL, (PUCHAR *)&pPtr, size);
	/*pPtr = kmalloc(size, MEM_ALLOC_FLAG); */

#endif

	return (PUCHAR)pPtr;
}


VOID dumpPkt(PUCHAR pHeader, int len)
{
	int i;
	PSTRING tmp;

	tmp = (PSTRING)pHeader;

	DBGPRINT(RT_DEBUG_OFF, ("--StartDump\n"));
	for(i=0;i<len; i++)
	{
		if ( (i%16==0) && (i!=0))
			DBGPRINT(RT_DEBUG_OFF, ("\n"));
		DBGPRINT(RT_DEBUG_OFF, ("%02x ", tmp[i]& 0xff));
	}
	DBGPRINT(RT_DEBUG_OFF, ("\n--EndDump\n"));

	return;
}


/*
	========================================================================
	Routine	Description:
		For each out-going packet, check the upper layer protocol type if need
		to handled by our APCLI convert engine. If yes, call corresponding handler 
		to handle it.
		
	Arguments:
		pAd		=>Pointer to our adapter
		pPkt 	=>pointer to the 802.11 header of outgoing packet 
		ifIdx   =>Interface Index want to dispatch to.

	Return Value:
		Success	=>
			TRUE
			Mapped mac address if found, else return specific default mac address 
			depends on the upper layer protocol type.
		Error	=>
			FALSE.

	Note:
		1.the pPktHdr must be a 802.3 packet.
		2.Maybe we need a TxD arguments?
		3.We check every packet here including group mac address becasue we need to
		  handle DHCP packet.
	========================================================================
 */
PUCHAR MATEngineTxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET	    pPkt,
	IN UINT				ifIdx,
	IN UCHAR    OpMode)
{
	PUCHAR 		pLayerHdr = NULL, pPktHdr = NULL,  pMacAddr = NULL;
	UINT16		protoType, protoType_ori;
	INT			i;
	struct _MATProtoOps 	*pHandle = NULL;
	PUCHAR  retSkb = NULL;
	BOOLEAN bVLANPkt = FALSE;


	if(pAd->MatCfg.status != MAT_ENGINE_STAT_INITED)
		return NULL;
	
	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return NULL;
	
	protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12));
	
	/* Get the upper layer protocol type of this 802.3 pkt. */
	protoType = OS_NTOHS(protoType_ori);

	/* handle 802.1q enabled packet. Skip the VLAN tag field to get the protocol type. */
	if (protoType == 0x8100)
	{
		protoType_ori = get_unaligned((PUINT16)(pPktHdr + 12 + 4));
		protoType = OS_NTOHS(protoType_ori);
		bVLANPkt = TRUE;
	}

	
	/* For differnet protocol, dispatch to specific handler */
	for (i=0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++)
	{
		if (protoType == MATProtoTb[i].protocol)
		{
			pHandle = MATProtoTb[i].pHandle;	/* the pHandle must not be null! */
			pLayerHdr = bVLANPkt ? (pPktHdr + MAT_VLAN_ETH_HDR_LEN) : (pPktHdr + MAT_ETHER_HDR_LEN);
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
			{
#ifdef MAC_REPEATER_SUPPORT
				UCHAR tempIdx = ifIdx;
				UCHAR CliIdx = 0xFF;

				if (tempIdx >= 64)
				{
					CliIdx = ((tempIdx - 64) % 16);
					tempIdx = ((tempIdx - 64) / 16);

					pMacAddr = &pAd->ApCfg.ApCliTab[tempIdx].RepeaterCli[CliIdx].CurrentAddress[0];
				}
				else
#endif /* MAC_REPEATER_SUPPORT */
					pMacAddr = &pAd->ApCfg.ApCliTab[ifIdx].wdev.if_addr[0];
			}
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
				pMacAddr = &pAd->CurrentAddress[0];
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

			if (pHandle->tx!=NULL)
				retSkb = pHandle->tx((PVOID)&pAd->MatCfg, RTPKT_TO_OSPKT(pPkt), pLayerHdr, pMacAddr);

			return retSkb;
		}
	}
	return retSkb;
}


/*
	========================================================================
	Routine	Description:
		Depends on the Received packet, check the upper layer protocol type
		and search for specific mapping table to find out the real destination 
		MAC address.
		
	Arguments:
		pAd		=>Pointer to our adapter
		pPkt	=>pointer to the 802.11 header of receviced packet 
		infIdx	=>Interface Index want to dispatch to.

	Return Value:
		Success	=>
			Mapped mac address if found, else return specific default mac address 
			depends on the upper layer protocol type.
		Error	=>
			NULL

	Note:
	========================================================================
 */
PUCHAR MATEngineRxHandle(
	IN PRTMP_ADAPTER	pAd,
	IN PNDIS_PACKET		pPkt,
	IN UINT				infIdx)
{
	PUCHAR				pMacAddr = NULL;
	PUCHAR 		pLayerHdr = NULL, pPktHdr = NULL;
	UINT16		protoType;
	INT			i =0;
	struct _MATProtoOps 	*pHandle = NULL;


	if(pAd->MatCfg.status != MAT_ENGINE_STAT_INITED)
		return NULL;

	pPktHdr = GET_OS_PKT_DATAPTR(pPkt);
	if (!pPktHdr)
		return NULL;

	/* If it's a multicast/broadcast packet, we do nothing. */
	if (IS_GROUP_MAC(pPktHdr))
		return NULL;

	/* Get the upper layer protocol type of this 802.3 pkt and dispatch to specific handler */
	protoType = OS_NTOHS(get_unaligned((PUINT16)(pPktHdr + 12)));
	
	for (i=0; i<MAX_MAT_SUPPORT_PROTO_NUM; i++)
	{
		if (protoType == MATProtoTb[i].protocol)
		{
			pHandle = MATProtoTb[i].pHandle;	/* the pHandle must not be null! */
			pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);
/*			RTMP_SEM_LOCK(&MATDBLock); */
			if(pHandle->rx!=NULL)
				pMacAddr = pHandle->rx((PVOID)&pAd->MatCfg, RTPKT_TO_OSPKT(pPkt), pLayerHdr, NULL);
/*			RTMP_SEM_UNLOCK(&MATDBLock); */
			break;
		}
	}

	if (pMacAddr)
		NdisMoveMemory(pPktHdr, pMacAddr, MAC_ADDR_LEN);

	return NULL;

}


BOOLEAN MATPktRxNeedConvert(
		IN PRTMP_ADAPTER	pAd, 
		IN PNET_DEV			net_dev)
{

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef APCLI_SUPPORT
		int i = 0;
		
		/* Check if the packet will be send to apcli interface. */
		while(i<MAX_APCLI_NUM)
		{
			/*BSSID match the ApCliBssid ?(from a valid AP) */
			if ((pAd->ApCfg.ApCliTab[i].Valid == TRUE) 
				&& (net_dev == pAd->ApCfg.ApCliTab[i].wdev.if_dev)
#ifdef MWDS
				&& (pAd->ApCfg.ApCliTab[i].bEnableMWDS == FALSE)
#endif /* MWDS */
				)
				return TRUE;
			i++;
		}
#endif /* APCLI_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
#ifdef ETH_CONVERT_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{

		if (pAd->EthConvert.ECMode & ETH_CONVERT_MODE_DONGLE)
			return TRUE;
	}
#endif /* ETH_CONVERT_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

	return FALSE;
	
}


NDIS_STATUS MATEngineExit(
	IN RTMP_ADAPTER *pAd)
{
	struct _MATProtoOps *pHandle = NULL;
	int i;

	if(pAd->MatCfg.status == MAT_ENGINE_STAT_EXITED)
		return TRUE;
	
	pAd->MatCfg.status = MAT_ENGINE_STAT_EXITED;

	/* For each registered protocol, we call it's exit handler. */
	for (i=0; i<MAX_MAT_SUPPORT_PROTO_NUM; i++)
	{
			pHandle = MATProtoTb[i].pHandle;
			if (pHandle->exit!=NULL)
			pHandle->exit(&pAd->MatCfg);
	}

#ifdef KMALLOC_BATCH
	/* Free the memory used to store node entries. */
	if (pAd->MatCfg.pMATNodeEntryPoll)
	{
		os_free_mem(pAd, pAd->MatCfg.pMATNodeEntryPoll);
		pAd->MatCfg.pMATNodeEntryPoll = NULL;
	}
#endif

	return TRUE;
	
}


NDIS_STATUS MATEngineInit(
	IN RTMP_ADAPTER *pAd)
{
	MATProtoOps 	*pHandle = NULL;
	int i, status;

	if(pAd->MatCfg.status == MAT_ENGINE_STAT_INITED)
		return TRUE;
	
#ifdef KMALLOC_BATCH
	/* Allocate memory for node entry, we totally allocate 128 entries and link them together. */
/*	pAd->MatCfg.pMATNodeEntryPoll = kmalloc(sizeof(MATNodeEntry) * MAX_MAT_NODE_ENTRY_NUM, GFP_KERNEL); */
	os_alloc_mem_suspend(NULL, (UCHAR **)&(pAd->MatCfg.pMATNodeEntryPoll), sizeof(MATNodeEntry) * MAX_MAT_NODE_ENTRY_NUM);
	if (pAd->MatCfg.pMATNodeEntryPoll != NULL)
	{
		MATNodeEntry *pPtr=NULL;

		NdisZeroMemory(pAd->MatCfg.pMATNodeEntryPoll, sizeof(MATNodeEntry) * MAX_MAT_NODE_ENTRY_NUM);
		pPtr = pAd->MatCfg.pMATNodeEntryPoll;
		for (i = 0; i < (MAX_MAT_NODE_ENTRY_NUM -1); i++)
		{
			pPtr->next = (MATNodeEntry *)(pPtr+1);
			pPtr = pPtr->next;
		}
		pPtr->next = NULL;
	} else {
		return FALSE;
	}
#endif

	/* For each specific protocol, call it's init function. */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++)
	{
			pHandle = MATProtoTb[i].pHandle;
		ASSERT(pHandle);
			if (pHandle->init != NULL)
		{
			status = pHandle->init(&pAd->MatCfg);
			if (status == FALSE) 
			{
				DBGPRINT(RT_DEBUG_ERROR, ("MATEngine Init Protocol (0x%x) failed, Stop the MAT Funciton initialization failed!\n", MATProtoTb[i].protocol));
				goto init_failed;
			}
			DBGPRINT(RT_DEBUG_TRACE, ("MATEngine Init Protocol (0x%04x) success!\n", MATProtoTb[i].protocol));
		}
	}

	NdisAllocateSpinLock(pAd, &pAd->MatCfg.MATDBLock);
#ifdef MAC_REPEATER_SUPPORT
	pAd->MatCfg.bMACRepeaterEn = FALSE;
#endif /* MAC_REPEATER_SUPPORT */
	pAd->MatCfg.pPriv = (VOID *)pAd;
	pAd->MatCfg.status = MAT_ENGINE_STAT_INITED;

	return TRUE;

init_failed:
	/* For each specific protocol, call it's exit function. */
	for (i = 0; i < MAX_MAT_SUPPORT_PROTO_NUM; i++)
	{
		if ((pHandle = MATProtoTb[i].pHandle) != NULL)
		{
			if (pHandle->exit != NULL)
			{
				status = pHandle->exit(&pAd->MatCfg);
				if (status == FALSE)
					goto init_failed;
			}
		}
	}

#ifdef KMALLOC_BATCH
	if (pAd->MatCfg.pMATNodeEntryPoll)
		os_free_mem(pAd, pAd->MatCfg.pMATNodeEntryPoll);
	pAd->MatCfg.status = MAT_ENGINE_STAT_EXITED;
#endif /* KMALLOC_BATCH */

	return FALSE;
	
}

#endif /* MAT_SUPPORT */

