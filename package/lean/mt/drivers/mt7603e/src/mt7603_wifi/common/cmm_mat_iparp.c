/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2007, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_mat_iparp.c

	Abstract:
		MAT convert engine subroutine for ip base protocols, currently now we 
	just handle IP/ARP protocols.

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Shiang      02/26/07      Init version
*/
#ifdef MAT_SUPPORT

#include "rt_config.h"

static NDIS_STATUS MATProto_IP_Init(MAT_STRUCT *pMatCfg);
static NDIS_STATUS MATProto_IP_Exit(MAT_STRUCT *pMatCfg);
static PUCHAR MATProto_IP_Rx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pMacAddr);
static PUCHAR MATProto_IP_Tx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pMacAddr);

static NDIS_STATUS MATProto_ARP_Init(MAT_STRUCT *pMatCfg);
static NDIS_STATUS MATProto_ARP_Exit(MAT_STRUCT *pMatCfg);
static PUCHAR MATProto_ARP_Rx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb, PUCHAR pLayerHdr, PUCHAR pMacAddr);
static PUCHAR MATProto_ARP_Tx(MAT_STRUCT *pMatCfg, PNDIS_PACKET pSkb,PUCHAR pLayerHdr, PUCHAR pMacAddr);

#define IPV4_ADDR_LEN 4

#define NEED_UPDATE_IPMAC_TB(Mac, IP) (IS_UCAST_MAC(Mac) && IS_GOOD_IP(IP))


typedef struct _IPMacMappingEntry
{
	UINT	ipAddr;	/* In network order */
	UCHAR	macAddr[MAC_ADDR_LEN];
	ULONG	lastTime;
	struct _IPMacMappingEntry *pNext;
}IPMacMappingEntry, *PIPMacMappingEntry;


typedef struct _IPMacMappingTable
{
	BOOLEAN			valid;
	IPMacMappingEntry *hash[MAT_MAX_HASH_ENTRY_SUPPORT+1]; /*0~63 for specific station, 64 for broadcast MacAddress */
	UCHAR			curMcastAddr[MAC_ADDR_LEN]; /* The multicast mac addr for currecnt received packet destined to ipv4 multicast addr */
}IPMacMappingTable;


struct _MATProtoOps MATProtoIPHandle =
{
	.init = MATProto_IP_Init,
	.tx = MATProto_IP_Tx,
	.rx = MATProto_IP_Rx,
	.exit = MATProto_IP_Exit,
};

struct _MATProtoOps MATProtoARPHandle =
{
	.init = MATProto_ARP_Init,
	.tx = MATProto_ARP_Tx,
	.rx = MATProto_ARP_Rx,
	.exit =MATProto_ARP_Exit,
};


VOID dumpIPMacTb(
	IN MAT_STRUCT	*pMatCfg,
	IN int 			index)
{
	IPMacMappingTable *pIPMacTable;
	IPMacMappingEntry *pHead;
	int startIdx, endIdx;

	pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;
	if (!pIPMacTable)
		return;
	
	if (!pIPMacTable->valid)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s():IPMacTable not init yet, so cannot do dump!\n", __FUNCTION__));
		return;
	}
	
	
	if(index < 0)
	{	/* dump all. */
		startIdx = 0;
		endIdx = MAT_MAX_HASH_ENTRY_SUPPORT;
	}
	else
	{	/* dump specific hash index. */
		startIdx = endIdx = index;
	}

	DBGPRINT(RT_DEBUG_OFF, ("%s():\n", __FUNCTION__));
	for(; startIdx<= endIdx; startIdx++)
	{
		pHead = pIPMacTable->hash[startIdx];
		while(pHead)
		{
				DBGPRINT(RT_DEBUG_OFF, ("IPMac[%d]:\n", startIdx));
				DBGPRINT(RT_DEBUG_OFF, ("\t:IP=0x%x,Mac=%02x:%02x:%02x:%02x:%02x:%02x, lastTime=0x%lx, next=%p\n", 
								pHead->ipAddr, pHead->macAddr[0],pHead->macAddr[1],pHead->macAddr[2],
								pHead->macAddr[3],pHead->macAddr[4],pHead->macAddr[5], pHead->lastTime,
								pHead->pNext));
			pHead = pHead->pNext;
		}
	}
	DBGPRINT(RT_DEBUG_OFF, ("\t----EndOfDump!\n"));
	
}


static inline NDIS_STATUS getDstIPFromIpPkt(
	IN PUCHAR pIpHdr, 
	IN UINT *dstIP)
{
	
	if (!pIpHdr)
		return FALSE;
	
	NdisMoveMemory(dstIP, (pIpHdr + 16), 4); /*shift 16 for IP header len before DstIP. */
/*	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Get the dstIP=0x%x\n", __FUNCTION__, *dstIP)); */
	
	return TRUE;
}

static inline NDIS_STATUS getSrcIPFromIpPkt(
	IN PUCHAR pIpHdr,
	IN UINT   *pSrcIP)
{
	
	if (!pIpHdr)
		return FALSE;
	
	NdisMoveMemory(pSrcIP, (pIpHdr + 12), 4); /*shift 12 for IP header len before DstIP. */
/*	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Get the srcIP=0x%x\n", __FUNCTION__, *pSrcIP)); */
	
	return TRUE;
	
}

static NDIS_STATUS IPMacTableUpdate(
	IN MAT_STRUCT		*pMatCfg,
	IN PUCHAR			pMacAddr,
	IN UINT				ipAddr)
{
	UINT 				hashIdx;
	IPMacMappingTable *pIPMacTable;
	IPMacMappingEntry	*pEntry = NULL, *pPrev = NULL, *pNewEntry =NULL;
	ULONG			now;

	pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;

	if (!pIPMacTable)
		return FALSE;
		
	if (!pIPMacTable->valid)
		return FALSE;

	hashIdx = MAT_IP_ADDR_HASH_INDEX(ipAddr);

	pEntry = pPrev = pIPMacTable->hash[hashIdx];
	while(pEntry)
	{
		NdisGetSystemUpTime(&now);
		
		/* Find a existed IP-MAC Mapping entry */
		if (ipAddr == pEntry->ipAddr)
		{
			/*	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Got the Mac(%02x:%02x:%02x:%02x:%02x:%02x) of mapped IP(%d.%d.%d.%d)\n",
						__FUNCTION__, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2], pEntry->macAddr[3],pEntry->macAddr[4],
						pEntry->macAddr[5], (ipAddr>>24) & 0xff, (ipAddr>>16) & 0xff, (ipAddr>>8) & 0xff, ipAddr & 0xff)); 
			*/
			/* compare is useless. So we directly copy it into the entry. */
			NdisMoveMemory(pEntry->macAddr, pMacAddr, 6);
			pEntry->lastTime = now;
			return TRUE;
		}
		else
		{	/* handle the age-out situation */
			/*if ((Now - pEntry->lastTime) > MAT_TB_ENTRY_AGEOUT_TIME) */
			if (RTMP_TIME_AFTER(now, pEntry->lastTime + MAT_TB_ENTRY_AGEOUT_TIME))
			{
				/* Remove the aged entry */
				if (pEntry == pIPMacTable->hash[hashIdx])
				{
					pIPMacTable->hash[hashIdx]= pEntry->pNext;
					pPrev = pIPMacTable->hash[hashIdx];
				}
				else 
				{	
					pPrev->pNext = pEntry->pNext;
				}
				MATDBEntryFree(pMatCfg, (PUCHAR)pEntry);

				pEntry = (pPrev == NULL ? NULL: pPrev->pNext);
				pMatCfg->nodeCount--;
			} 
			else
			{
				pPrev = pEntry;
				pEntry = pEntry->pNext;
			}
		}
	}


	/* Allocate a new IPMacMapping entry and insert into the hash */
	pNewEntry = (IPMacMappingEntry *)MATDBEntryAlloc(pMatCfg, sizeof(IPMacMappingEntry));
	if (pNewEntry != NULL)
	{	
		pNewEntry->ipAddr = ipAddr;
		NdisMoveMemory(pNewEntry->macAddr, pMacAddr, 6);
		pNewEntry->pNext = NULL;
		NdisGetSystemUpTime(&pNewEntry->lastTime);

		if (pIPMacTable->hash[hashIdx] == NULL)
		{	/* Hash list is empty, directly assign it. */
			pIPMacTable->hash[hashIdx] = pNewEntry;
		} 
		else 
		{
			/* Ok, we insert the new entry into the root of hash[hashIdx] */
			pNewEntry->pNext = pIPMacTable->hash[hashIdx];
			pIPMacTable->hash[hashIdx] = pNewEntry;
		}
		/*dumpIPMacTb(pMatCfg, hashIdx); //for debug */

		pMatCfg->nodeCount++;

		return TRUE;
	}

	return FALSE;
}


static PUCHAR IPMacTableLookUp(
	IN MAT_STRUCT	*pMatCfg,
	IN UINT			ipAddr)
{
	IPMacMappingTable *pIPMacTable;
	UINT 				hashIdx, ip;
	IPMacMappingEntry	*pEntry = NULL;
	PUCHAR				pGroupMacAddr;

	pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;

	if (!pIPMacTable)
		return NULL;
		
	if (!pIPMacTable->valid)
		return NULL;
	
	/*if multicast ip, need converting multicast group address to ethernet address. */
	ip = ntohl(ipAddr);	
	if (IS_MULTICAST_IP(ip))	
	{
		pGroupMacAddr = (PUCHAR)(&pIPMacTable->curMcastAddr);
		ConvertMulticastIP2MAC((PUCHAR) &ipAddr, (UCHAR **)(&pGroupMacAddr), ETH_P_IP);
		return pIPMacTable->curMcastAddr;	
	}

	/* Use hash to find out the location of that entry and get the Mac address. */
	hashIdx = MAT_IP_ADDR_HASH_INDEX(ipAddr);

/*	spin_lock_irqsave(&IPMacTabLock, irqFlag); */
	pEntry = pIPMacTable->hash[hashIdx];
	while(pEntry)
	{
		if (pEntry->ipAddr == ipAddr)
		{
/*			DBGPRINT(RT_DEBUG_TRACE, ("%s(): dstMac=%02x:%02x:%02x:%02x:%02x:%02x for mapped dstIP(%d.%d.%d.%d)\n", 
					__FUNCTION__, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
					pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5],
					(ipAddr>>24) & 0xff, (ipAddr>>16) & 0xff, (ipAddr>>8) & 0xff, ipAddr & 0xff)); 
*/
			
			/*Update the lastTime to prevent the aging before pDA processed! */
			NdisGetSystemUpTime(&pEntry->lastTime);
			
			return pEntry->macAddr;
		}
		else
			pEntry = pEntry->pNext;
	}
	
	/*
		We didn't find any matched Mac address, our policy is treat it as
		broadcast packet and send to all.
	*/
	return pIPMacTable->hash[IPMAC_TB_HASH_INDEX_OF_BCAST]->macAddr;
	
}


static NDIS_STATUS IPMacTable_RemoveAll(
	IN MAT_STRUCT *pMatCfg)
{
	IPMacMappingEntry *pEntry;
	IPMacMappingTable *pIPMacTable;
	INT		i;


	pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;

	if (!pIPMacTable)
		return TRUE;
	
	if (pIPMacTable->valid)
	{	
		pIPMacTable->valid = FALSE;
		for (i=0; i<IPMAC_TB_HASH_ENTRY_NUM; i++)
		{
			while((pEntry = pIPMacTable->hash[i]) != NULL)
			{
				pIPMacTable->hash[i] = pEntry->pNext;
				MATDBEntryFree(pMatCfg, (PUCHAR)pEntry);
			}
		}
	}
	DBGPRINT(RT_DEBUG_TRACE, ("%s() Set nodeCount to 0,nodeCount = %d\n", __FUNCTION__, pMatCfg->nodeCount));
	pMatCfg->nodeCount = 0;
	os_free_mem(NULL, pIPMacTable);
	pMatCfg->MatTableSet.IPMacTable = NULL;
	
	return TRUE;
	
}


static NDIS_STATUS IPMacTable_init(
	IN MAT_STRUCT *pMatCfg)
{
	IPMacMappingTable *pIPMacTable;
	IPMacMappingEntry *pEntry = NULL;


	if (pMatCfg->MatTableSet.IPMacTable != NULL)
	{
		pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;
	}
	else
	{
		os_alloc_mem_suspend(NULL, (UCHAR **)&(pMatCfg->MatTableSet.IPMacTable), sizeof(IPMacMappingTable));
		if (pMatCfg->MatTableSet.IPMacTable)
		{
			pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;
			NdisZeroMemory(pIPMacTable, sizeof(IPMacMappingTable));
		} 
		else
		{
			DBGPRINT(RT_DEBUG_ERROR, ("IPMacTable_init(): Allocate memory for IPMacTable failed!\n"));
			return FALSE;
		}
	}
	
	if (pIPMacTable->valid == FALSE)
	{
		/*Set the last hash entry (hash[64]) as our default broadcast Mac address */
		pEntry = (IPMacMappingEntry *)MATDBEntryAlloc(pMatCfg, sizeof(IPMacMappingEntry));
		if (!pEntry)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("IPMacTable_init(): Allocate memory for IPMacTable broadcast entry failed!\n"));
			return FALSE;
		}
		
		NdisZeroMemory(pEntry, sizeof(IPMacMappingEntry));
		NdisMoveMemory(&pEntry->macAddr[0], &BROADCAST_ADDR[0], 6);
		pEntry->pNext = NULL;
		pIPMacTable->hash[IPMAC_TB_HASH_INDEX_OF_BCAST] = pEntry;
	
		pIPMacTable->valid = TRUE;
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s(): IPMacTable already inited!\n", __FUNCTION__));
	}

	return TRUE;
	
}


static NDIS_STATUS MATProto_ARP_Exit(
	IN MAT_STRUCT *pMatCfg)
{
	INT status;
		
	status = IPMacTable_RemoveAll(pMatCfg);

	return status;
}

static PUCHAR MATProto_ARP_Rx(
	IN MAT_STRUCT 		*pMatCfg, 
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR			pLayerHdr,
	IN PUCHAR 			pMacAddr)
{
	PUCHAR pArpHdr = NULL, pRealMac = NULL;
	PUCHAR	tgtMac, tgtIP;
	BOOLEAN isUcastMac, isGoodIP;

	
	pArpHdr = pLayerHdr;

/*dumpPkt(RTPKT_TO_OSPKT(pSkb)->data, RTPKT_TO_OSPKT(pSkb)->len); */
	/* We just take care about the target(Mac/IP address) fields. */
	tgtMac = pArpHdr + 18;
	tgtIP = tgtMac + 6;
		
	/* isUcastMac = !(00:00:00:00:00:00|| mcastMac); */
	isUcastMac = ((tgtMac[0]|tgtMac[1]|tgtMac[2]|tgtMac[3]|tgtMac[4]|tgtMac[5])!=0);
	isUcastMac &= ((tgtMac[0] & 0x1)==0);

	/* isGoodIP = ip address is not 0.0.0.0 */
	isGoodIP = (*(UINT *)tgtIP != 0);
	
		
	if (isUcastMac && isGoodIP)
		pRealMac = IPMacTableLookUp(pMatCfg, *(UINT *)tgtIP);
		
	/*
		For need replaced mac, we need to replace the targetMAC as correct one to make
		the real receiver can receive that.
	*/
	if (isUcastMac && pRealMac)
		NdisMoveMemory(tgtMac, pRealMac, MAC_ADDR_LEN);

	if (pRealMac == NULL)
		pRealMac = &BROADCAST_ADDR[0];
/*		pRealMac = pIPMacTable->hash[IPMAC_TB_HASH_INDEX_OF_BCAST]->macAddr; */
	
	return pRealMac;
}

static PUCHAR MATProto_ARP_Tx(
	IN MAT_STRUCT 		*pMatCfg,
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR 			pLayerHdr,
	IN PUCHAR 			pMacAddr)
{
	PUCHAR	pSMac, pSIP;
	BOOLEAN isUcastMac, isGoodIP;
	NET_PRO_ARP_HDR *arpHdr;
	//PUCHAR pPktHdr;
	PNDIS_PACKET newSkb = NULL;

	//pPktHdr = GET_OS_PKT_DATAPTR(pSkb);
	
	arpHdr = (NET_PRO_ARP_HDR *)pLayerHdr;

	/*
		Check the arp header.
		We just handle ether type hardware address and IPv4 internet
		address type and opcode is  ARP reuqest/response.
	*/
	if ((arpHdr->ar_hrd != OS_HTONS(ARPHRD_ETHER)) || (arpHdr->ar_pro != OS_HTONS(ETH_P_IP)) ||
		(arpHdr->ar_op != OS_HTONS(ARPOP_REPLY) && arpHdr->ar_op != OS_HTONS(ARPOP_REQUEST)))
		return NULL;

	/* We just take care about the sender(Mac/IP address) fields. */
	pSMac =(PUCHAR)(pLayerHdr + 8);
	pSIP = (PUCHAR)(pSMac + MAC_ADDR_LEN);
	
	isUcastMac = IS_UCAST_MAC(pSMac);
	isGoodIP = IS_GOOD_IP(get_unaligned32((PUINT) pSIP));
	
/*	
	DBGPRINT(RT_DEBUG_TRACE,("%s(): ARP Pkt=>senderIP=%d.%d.%d.%d, senderMac=%02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, pSIP[0], pSIP[1], pSIP[2], pSIP[3],
			pSMac[0],pSMac[1],pSMac[2],pSMac[3],pSMac[4],pSMac[5]));	
*/
	if (isUcastMac && isGoodIP)
		IPMacTableUpdate(pMatCfg, pSMac, get_unaligned32((PUINT) pSIP));

	/*
		For outgoing unicast mac, we need to replace the senderMAC as ourself to make
		the receiver can send to us.
	*/
	if (isUcastMac)
	{
		if(OS_PKT_CLONED(pSkb)) 
		{
			newSkb = (PNDIS_PACKET)OS_PKT_COPY(pSkb);
			if(newSkb) 
			{
				if (IS_VLAN_PACKET(GET_OS_PKT_DATAPTR(newSkb)))
					pSMac = (PUCHAR)(GET_OS_PKT_DATAPTR(newSkb) + MAT_VLAN_ETH_HDR_LEN + 8);
				else
					pSMac = (PUCHAR)(GET_OS_PKT_DATAPTR(newSkb) + MAT_ETHER_HDR_LEN + 8);
			}
		}
		
		ASSERT(pMacAddr);
		NdisMoveMemory(pSMac, pMacAddr, MAC_ADDR_LEN);
	}

	return (PUCHAR)newSkb;
}


static NDIS_STATUS MATProto_ARP_Init(
	IN MAT_STRUCT 	*pMatCfg)
{
	BOOLEAN status = FALSE;

	status = IPMacTable_init(pMatCfg);
	
	return status;
}


static NDIS_STATUS MATProto_IP_Exit(
	IN MAT_STRUCT	*pMatCfg)
{
	INT status;
		
	status = IPMacTable_RemoveAll(pMatCfg);

	return status;
}


static PUCHAR MATProto_IP_Rx(
	IN MAT_STRUCT 		*pMatCfg, 
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR 			pLayerHdr,
	IN PUCHAR 			pDevMacAdr)
{
#ifdef MAC_REPEATER_SUPPORT
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) pMatCfg->pPriv;
	USHORT wcid = 0xFF;
	PMAC_TABLE_ENTRY pEntry = NULL;
	PREPEATER_CLIENT_ENTRY pReptEntry = NULL;
#endif /* MAC_REPEATER_SUPPORT */
	PUCHAR	 pMacAddr;
	UINT   	dstIP;
	
	/* Fetch the IP addres from the packet header. */
	getDstIPFromIpPkt(pLayerHdr, &dstIP);
	pMacAddr = IPMacTableLookUp(pMatCfg, dstIP); 
	
#ifdef MAC_REPEATER_SUPPORT
	if (pAd == NULL)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s():ERROR! pAd is null!\n", __FUNCTION__));
		return pMacAddr;
	}

	wcid = RTMP_GET_PACKET_WCID(pSkb);
		
	pEntry = &pAd->MacTab.Content[wcid];
	pReptEntry = &pAd->ApCfg.ApCliTab[pEntry->func_tb_idx].RepeaterCli[pEntry->MatchReptCliIdx];

	if (pAd->ApCfg.bMACRepeaterEn)
	{
		if (pEntry->bReptCli)
		{
			UINT  ip;

			ip = ntohl(dstIP);

			/* avoid duplicate packet in IGMP case */
			if (IS_MULTICAST_IP(ip))
				return pReptEntry->OriginalAddress;
		}
		

		if ((pAd->ApCfg.MACRepeaterOuiMode != 1))
		{
			if (pEntry->bReptCli)
			{
				PUCHAR pPktHdr, pLayerHdr;

				pPktHdr = GET_OS_PKT_DATAPTR(pSkb);
				pLayerHdr = (pPktHdr + MAT_ETHER_HDR_LEN);

				/*For UDP packet, we need to check about the DHCP packet. */
				if (*(pLayerHdr + 9) == 0x11)
				{
					PUCHAR pUdpHdr;
					UINT16 srcPort, dstPort;
					BOOLEAN bHdrChanged = FALSE;

					pUdpHdr = pLayerHdr + 20;
					srcPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr)));
					dstPort = OS_NTOHS(get_unaligned((PUINT16)(pUdpHdr+2)));

					if (srcPort==67 && dstPort==68) /*It's a DHCP packet */
					{
						PUCHAR bootpHdr/*, dhcpHdr*/, pCliHwAddr;
						bootpHdr = pUdpHdr + 8;
						//dhcpHdr = bootpHdr + 236;
						pCliHwAddr = (bootpHdr+28);
						
						if (pReptEntry)
							NdisMoveMemory(pCliHwAddr, pReptEntry->OriginalAddress, MAC_ADDR_LEN);
						else
						{
							DBGPRINT(RT_DEBUG_OFF, ("%s() MatchAPCLITabIdx = %u\n", __FUNCTION__, pEntry->func_tb_idx));
						}
						bHdrChanged = TRUE;
					}
				
					if (bHdrChanged == TRUE)
						NdisZeroMemory((pUdpHdr+6), 2); /*modify the UDP chksum as zero */
				}
			}
		}
	}
#endif /* MAC_REPEATER_SUPPORT */

	return pMacAddr;
}

static UCHAR  DHCP_MAGIC[]= {0x63, 0x82, 0x53, 0x63};
static PUCHAR MATProto_IP_Tx(
	IN MAT_STRUCT 		*pMatCfg,
	IN PNDIS_PACKET		pSkb,
	IN PUCHAR 			pLayerHdr,
	IN PUCHAR 			pDevMacAdr)
{
	PUCHAR pSrcMac;
	PUCHAR pSrcIP;
	BOOLEAN needUpdate;
	PUCHAR pPktHdr;

	pPktHdr = GET_OS_PKT_DATAPTR(pSkb);
	
	pSrcMac = pPktHdr + 6;
	pSrcIP = pLayerHdr + 12;

	needUpdate = NEED_UPDATE_IPMAC_TB(pSrcMac, get_unaligned32((PUINT)(pSrcIP)));
	if (needUpdate)
		IPMacTableUpdate(pMatCfg, pSrcMac, get_unaligned32((PUINT)(pSrcIP)));

	/*For UDP packet, we need to check about the DHCP packet, to modify the flag of DHCP discovey/request as broadcast. */
	if (*(pLayerHdr + 9) == 0x11)
	{
		PUCHAR udpHdr;
		UINT16 srcPort, dstPort;
		BOOLEAN bHdrChanged = FALSE;
		
		udpHdr = pLayerHdr + 20;
		srcPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr)));
		dstPort = OS_NTOHS(get_unaligned((PUINT16)(udpHdr+2)));
		
		if (srcPort==68 && dstPort==67) /*It's a DHCP packet */
		{
			PUCHAR bootpHdr;
			UINT16 bootpFlag;	
			PUCHAR dhcpHdr;
			
			bootpHdr = udpHdr + 8;
			bootpFlag = OS_NTOHS(get_unaligned((PUINT16)(bootpHdr+10)));
			DBGPRINT(RT_DEBUG_TRACE, ("is bootp packet! bootpFlag=0x%x\n", bootpFlag));
				dhcpHdr = bootpHdr + 236;
				
			if (bootpFlag != 0x8000) /*check if it's a broadcast request. */
			{
				DBGPRINT(RT_DEBUG_TRACE, ("the DHCP flag is a unicast, dhcp_magic=%02x:%02x:%02x:%02x\n", 
										dhcpHdr[0], dhcpHdr[1], dhcpHdr[2], dhcpHdr[3]));
				if (NdisEqualMemory(dhcpHdr, DHCP_MAGIC, 4))
				{	
					DBGPRINT(RT_DEBUG_TRACE, ("dhcp magic macthed!\n"));	
					bootpFlag = OS_HTONS(0x8000);
					NdisMoveMemory((bootpHdr+10), &bootpFlag, 2);	/*Set the bootp flag as broadcast */
					bHdrChanged = TRUE;
				}
			}	
#ifdef MAC_REPEATER_SUPPORT
			if (pMatCfg->bMACRepeaterEn)
			{
				UCHAR isLinkValid;
			
				if (RTMPLookupRepeaterCliEntry(pMatCfg->pPriv, FALSE, pDevMacAdr, TRUE, &isLinkValid) != NULL)
				{
					NdisMoveMemory((bootpHdr+28), pDevMacAdr, MAC_ADDR_LEN);

					if (NdisEqualMemory(dhcpHdr, DHCP_MAGIC, 4))
					{
						PUCHAR pOptCode, pOptLen;
						UINT16 udpLen;
					
						udpLen = OS_NTOHS(get_unaligned((PUINT16)(udpHdr+4)));
						pOptCode = (dhcpHdr + 4);
						do
						{
							pOptLen = pOptCode + 1;
							if (*pOptCode == 61)	/* Client Identifier */
							{
								DBGPRINT(RT_DEBUG_TRACE,
										("Client Identifier found, change Hardware Address to "
										 "%02x:%02x:%02x:%02x:%02x:%02x\n", 
										 pDevMacAdr[0], pDevMacAdr[1], pDevMacAdr[2],
										 pDevMacAdr[3], pDevMacAdr[4], pDevMacAdr[5]));						
								/* Change Hardware Address */
								NdisMoveMemory((pOptCode+3), pDevMacAdr, MAC_ADDR_LEN);
								break;
							}
							pOptCode += (2+*pOptLen);
						}while ((*pOptCode != 0xFF) && ((pOptCode - udpHdr) <= udpLen));
					}
					bHdrChanged = TRUE;
				}	
			}
#endif /* MAC_REPEATER_SUPPORT */	
		}

		if (bHdrChanged == TRUE)
			NdisZeroMemory((udpHdr+6), 2); /* Modify the UDP chksum as zero */
	}

	return NULL;
}


static NDIS_STATUS MATProto_IP_Init(
	IN MAT_STRUCT *pMatCfg)
{
	BOOLEAN status;
	
	status = IPMacTable_init(pMatCfg);
	
	return status;
}


static inline void IPintToIPstr(int ipint, char Ipstr[20], ULONG BufLen)
{
	 int temp = 0;
	 
	 temp = ipint & 0x000FF;
	 snprintf(Ipstr, BufLen, "%d.", temp);
	 temp = (ipint>>8) & 0x000FF;
	 snprintf(Ipstr, BufLen, "%s%d.", Ipstr, temp);
	 temp = (ipint>>16) & 0x000FF;
	 snprintf(Ipstr, BufLen, "%s%d.", Ipstr, temp);
	 temp = (ipint>>24) & 0x000FF;
	 snprintf(Ipstr, BufLen, "%s%d", Ipstr, temp);
}


VOID getIPMacTbInfo(
	IN MAT_STRUCT *pMatCfg, 
	IN char *pOutBuf,
	IN ULONG BufLen)
{
	IPMacMappingTable *pIPMacTable;
	IPMacMappingEntry *pHead;
	int startIdx, endIdx;
	char Ipstr[20] = {0};


	pIPMacTable = (IPMacMappingTable *)pMatCfg->MatTableSet.IPMacTable;
	if ((!pIPMacTable) || (!pIPMacTable->valid))
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s():IPMacTable not init yet!\n", __FUNCTION__));
		return;
	}
		
	/* dump all. */
	startIdx = 0;
	endIdx = MAT_MAX_HASH_ENTRY_SUPPORT;

	sprintf(pOutBuf, "\n");
	sprintf(pOutBuf+strlen(pOutBuf), "%-18s%-20s\n", "IP", "MAC");
	for(; startIdx< endIdx; startIdx++)
	{
		pHead = pIPMacTable->hash[startIdx];
		while(pHead)
		{
/*			if (strlen(pOutBuf) > (IW_PRIV_SIZE_MASK - 30)) */
			if (RtmpOsCmdDisplayLenCheck(strlen(pOutBuf), 30) == FALSE)
			    break;
			NdisZeroMemory(Ipstr, 20);
			IPintToIPstr(pHead->ipAddr, Ipstr, sizeof(Ipstr));
			sprintf(pOutBuf+strlen(pOutBuf), "%-18s%02x:%02x:%02x:%02x:%02x:%02x\n",
				Ipstr, pHead->macAddr[0],pHead->macAddr[1],pHead->macAddr[2],
				pHead->macAddr[3],pHead->macAddr[4],pHead->macAddr[5]);
			pHead = pHead->pNext;
		}
	}
}

#endif /* MAT_SUPPORT */

