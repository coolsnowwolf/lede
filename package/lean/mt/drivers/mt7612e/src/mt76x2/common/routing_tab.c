/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


    Module Name:
	routing_tab.c
 
    Abstract:
    This is a tab used to record all entries behind associated APClinet or STA/PC.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef ROUTING_TAB_SUPPORT
#include "rt_config.h"
#include <linux/inetdevice.h>
#include <net/arp.h>


VOID RoutingTabInit(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 Flag)
{
	UINT32 i;

	if(pAd->ApCfg.bRoutingTabInit)
    {
        pAd->ApCfg.RoutingTabFlag |= Flag;
		return;
    }

	if(pAd->ApCfg.pRoutingEntryPool== NULL)
		os_alloc_mem(pAd, (UCHAR **)&(pAd->ApCfg.pRoutingEntryPool), sizeof(ROUTING_ENTRY) * ROUTING_POOL_SIZE);
	
	if (pAd->ApCfg.pRoutingEntryPool)
	{
		NdisZeroMemory(pAd->ApCfg.pRoutingEntryPool, sizeof(ROUTING_ENTRY) * ROUTING_POOL_SIZE);
		initList(&pAd->ApCfg.RoutingEntryFreeList);
		for (i = 0; i < ROUTING_POOL_SIZE; i++)
			insertTailList(&pAd->ApCfg.RoutingEntryFreeList, (PLIST_ENTRY)(pAd->ApCfg.pRoutingEntryPool+i));

        for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)
		    initList(&pAd->ApCfg.RoutingTab[i]);

        NdisAllocateSpinLock(pAd, &pAd->ApCfg.RoutingTabLock);
        pAd->ApCfg.RoutingTabFlag |= Flag;
        pAd->ApCfg.bRoutingTabInit = TRUE;
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Fail to alloc memory for pAd->ApCfg.pRoutingEntryPool\n", __FUNCTION__));
}

VOID RoutingTabDestory(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 Flag)
{
    if(!pAd->ApCfg.bRoutingTabInit) 
		return;

    RoutingTabClear(pAd, Flag);
    
    pAd->ApCfg.RoutingTabFlag &= ~Flag;
    if(pAd->ApCfg.RoutingTabFlag == 0)
    {
        NdisFreeSpinLock(&pAd->ApCfg.RoutingTabLock);
    	if (pAd->ApCfg.pRoutingEntryPool)
    		os_free_mem(NULL, pAd->ApCfg.pRoutingEntryPool);
    	pAd->ApCfg.pRoutingEntryPool = NULL;
    	initList(&pAd->ApCfg.RoutingEntryFreeList);
        pAd->ApCfg.bRoutingTabInit = FALSE;
    }
}

VOID RoutingTabClear(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 Flag)
{
    INT32 i;
    PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;

    RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
    for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)
    {
    	pRoutingEntry = GetRoutingTabHead(pAd, i);
    	while(pRoutingEntry)
    	{
    		pRoutingEntryNext = pRoutingEntry->pNext;
    		if(pRoutingEntry->Valid && ROUTING_ENTRY_TEST_FLAG(pRoutingEntry, Flag))
    		{
    		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);
                /* If not any entry type, then recycle to free list. */
                if(!IS_ROUTING_ENTRY(pRoutingEntry))
                {
    			    delEntryList(&pAd->ApCfg.RoutingTab[i], (PLIST_ENTRY)pRoutingEntry);
    			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));
    			    insertTailList(&pAd->ApCfg.RoutingEntryFreeList, (PLIST_ENTRY)pRoutingEntry);
                }
    		}
    		pRoutingEntry = pRoutingEntryNext;
    	}
    }
    RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);
}

PROUTING_ENTRY RoutingTabGetFree(
	IN PRTMP_ADAPTER pAd)
{
	PROUTING_ENTRY pRoutingEntry = NULL;

    if(!pAd->ApCfg.bRoutingTabInit)
        return NULL;
    
	RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
	pRoutingEntry = (PROUTING_ENTRY)removeHeadList(&pAd->ApCfg.RoutingEntryFreeList);
	RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);

	return pRoutingEntry;
}

VOID RoutingTabSetAllFree(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UINT32 Flag)
{
    INT32 i;
    PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;

    if(!pAd->ApCfg.bRoutingTabInit) 
    	return;

    RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
    for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)
    {
    	pRoutingEntry = GetRoutingTabHead(pAd, i);
    	while(pRoutingEntry)
    	{
    		pRoutingEntryNext = pRoutingEntry->pNext;
    		if(pRoutingEntry->Valid && (pRoutingEntry->Wcid == Wcid))
    		{
    		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);
                /* If not any entry type, then recycle to free list. */
                if(!IS_ROUTING_ENTRY(pRoutingEntry))
                {
    			    delEntryList(&pAd->ApCfg.RoutingTab[i], (PLIST_ENTRY)pRoutingEntry);
    			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));
    			    insertTailList(&pAd->ApCfg.RoutingEntryFreeList, (PLIST_ENTRY)pRoutingEntry);
                }
    		}
    		pRoutingEntry = pRoutingEntryNext;
    	}
    }
    RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);
}

VOID  RoutingTabSetOneFree(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac,
	IN UINT32 Flag)
{
	INT32 HashId;
	PROUTING_ENTRY pRoutingEntry = NULL;

    if(!pAd->ApCfg.bRoutingTabInit) 
    	return;

	HashId = GetHashID(pMac);
    if(HashId < 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));
        return;
    }
    
	RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
	pRoutingEntry = GetRoutingTabHead(pAd, HashId);
	while (pRoutingEntry)
	{
		if (pRoutingEntry->Valid && MAC_ADDR_EQUAL(pMac, pRoutingEntry->Mac))
		{
		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);
             /* If not any entry type, then recycle to free list. */
            if(!IS_ROUTING_ENTRY(pRoutingEntry))
            {
                delEntryList(&pAd->ApCfg.RoutingTab[HashId], (PLIST_ENTRY)pRoutingEntry);
			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));
			    insertTailList(&pAd->ApCfg.RoutingEntryFreeList, (PLIST_ENTRY)pRoutingEntry);
            }
			break;
		}
		pRoutingEntry = pRoutingEntry->pNext;
	}
	RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);
}

VOID RoutingEntryRefresh(
	IN PRTMP_ADAPTER pAd,
	IN PROUTING_ENTRY pRoutingEntry)
{
    ULONG Now;
    
    if(!pAd->ApCfg.bRoutingTabInit || !pRoutingEntry)
        return;
     
    NdisGetSystemUpTime(&Now);
	pRoutingEntry->KeepAliveTime = Now + ROUTING_ENTRY_AGEOUT;
}

VOID RoutingEntrySet(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN PUCHAR pMac,
	IN PROUTING_ENTRY pRoutingEntry)
{
    INT32 HashId;
    
    if(!pAd->ApCfg.bRoutingTabInit || !pRoutingEntry)
        return;

    HashId = GetHashID(pMac);
    if(HashId < 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));
        return;
    }
    
    pRoutingEntry->Valid = 1;
    pRoutingEntry->Wcid = Wcid;
    COPY_MAC_ADDR(&pRoutingEntry->Mac, pMac);
    pRoutingEntry->pNext = NULL;
	RoutingEntryRefresh(pAd, pRoutingEntry);
    
    RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
    insertTailList(&pAd->ApCfg.RoutingTab[HashId], (PLIST_ENTRY)pRoutingEntry);
    RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);
}

INT32 GetHashID(
    IN PUCHAR pMac)
{
    INT32 HashId = -1;
    if(pMac)
        HashId = (*(pMac + 5)&(ROUTING_HASH_TAB_SIZE - 1));

    return HashId;
}

PROUTING_ENTRY GetRoutingTabHead(
	IN PRTMP_ADAPTER pAd,
	IN INT32 Index)
{
    if(!pAd->ApCfg.bRoutingTabInit) 
        return NULL;
    
    if(Index < ROUTING_HASH_TAB_SIZE)
        return (PROUTING_ENTRY)pAd->ApCfg.RoutingTab[Index].pHead;
    else
        return NULL;
}

BOOLEAN GetRoutingEntryAll(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UINT32 Flag,
	IN INT32 BufMaxCount,
	OUT const ROUTING_ENTRY **pEntryListBuf,
	OUT PUINT32 pCount)
{
    INT32 i, Total;
    PROUTING_ENTRY pRoutingEntry = NULL;
    BOOLEAN bFull = FALSE;

     if(!pAd->ApCfg.bRoutingTabInit || !pEntryListBuf)
        return FALSE;

    RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
    for (i = 0, Total = 0; i < ROUTING_HASH_TAB_SIZE; i++)
    {
        pRoutingEntry = GetRoutingTabHead(pAd, i);
        while(pRoutingEntry)
        {
        	if(pRoutingEntry->Valid && 
               (pRoutingEntry->Wcid == Wcid) &&
               ROUTING_ENTRY_TEST_FLAG(pRoutingEntry, Flag))
            {
                if(Total < BufMaxCount)
                    *(pEntryListBuf+Total++) = pRoutingEntry;
                else
                {
                    bFull = TRUE;
                    break;
                }
            }
            pRoutingEntry = pRoutingEntry->pNext;
        }

        if(bFull)
            break;
    }
    RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);

    *pCount = Total;
    return TRUE;
}

INT RoutingTabGetEntryCount(
	IN PRTMP_ADAPTER pAd)
{
	int count = 0;
	
	if(!pAd->ApCfg.bRoutingTabInit) 
		return 0;

	count = (ROUTING_POOL_SIZE - getListSize(&pAd->ApCfg.RoutingEntryFreeList));
	return count;
}

PROUTING_ENTRY RoutingTabLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac,
	IN BOOLEAN bUpdateAliveTime,
	OUT UCHAR* pWcid)
{
    PROUTING_ENTRY pRoutingEntry = NULL;
	INT32 HashId;

    if(RoutingTabGetEntryCount(pAd) == 0) 
    	return NULL;
    
	HashId = GetHashID(pMac);
    if(HashId < 0)
    {
        DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));
        return NULL;
    }
    
	pRoutingEntry = GetRoutingTabHead(pAd, HashId);
	while (pRoutingEntry)
	{
		if (pRoutingEntry->Valid && MAC_ADDR_EQUAL(pMac, pRoutingEntry->Mac))
		{
			if (VALID_WCID(pRoutingEntry->Wcid))
			{
				if(bUpdateAliveTime)
				{
                    RoutingEntryRefresh(pAd, pRoutingEntry);
                    pRoutingEntry->RetryKeepAlive = 0;
				}
							
				if(pWcid) *pWcid = pRoutingEntry->Wcid;
				return pRoutingEntry;
			}
			else
				return NULL;
		}
		pRoutingEntry = pRoutingEntry->pNext;
	}
	
	return NULL;
}

VOID RoutingTabARPLookupUpdate(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR pData)
{
    UINT16 ProtoType, ArpType;
    UCHAR *Pos = pData;
	UCHAR *SenderMAC, *SenderIP;
    INT32 HashId;
    PROUTING_ENTRY pRoutingEntry = NULL;

    if(RoutingTabGetEntryCount(pAd) == 0)
        return;

	NdisMoveMemory(&ProtoType, Pos, 2);
	ProtoType = OS_NTOHS(ProtoType);
    Pos += 2;

    if (ProtoType == 0x0806) /* ETH_P_ARP */
    {
        //NdisMoveMemory(&ArpType, (Pos+6), 2);
        //ArpType = OS_NTOHS(ArpType);
        //if(ArpType == 2) /* ARP Response */
        {
            SenderMAC = Pos + 8;
        	SenderIP = Pos + 14;

            HashId = GetHashID(SenderMAC);
            if(HashId < 0)
            {
                DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));
                return;
            }
            
        	pRoutingEntry = GetRoutingTabHead(pAd, HashId);
            while (pRoutingEntry)
        	{
        		if (pRoutingEntry->Valid && MAC_ADDR_EQUAL(SenderMAC, pRoutingEntry->Mac))
        		{
                    NdisCopyMemory(&pRoutingEntry->IPAddr,SenderIP,4);
        			return;
        		}
        		pRoutingEntry = pRoutingEntry->pNext;
        	}
        }
    }
}

INT RoutingEntrySendAliveCheck(
    IN PRTMP_ADAPTER pAd,
    IN PROUTING_ENTRY pRoutingEntry,
    IN UCHAR *pSrcMAC,
    IN UINT32 SrcIP)
{
    struct sk_buff *skb = NULL;
    PMAC_TABLE_ENTRY pEntry = NULL;

    if(!pRoutingEntry || !pSrcMAC || 
        !VALID_WCID(pRoutingEntry->Wcid) || 
        (SrcIP == 0))
        return FALSE;

    if(RoutingTabGetEntryCount(pAd) == 0)
        return FALSE;
    
    pEntry = &pAd->MacTab.Content[pRoutingEntry->Wcid];
    skb = arp_create(ARPOP_REQUEST, ETH_P_ARP, pRoutingEntry->IPAddr, pEntry->wdev->if_dev,
                     SrcIP, BROADCAST_ADDR, pSrcMAC, ZERO_MAC_ADDR);
    if(!skb)
    {
		DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for arp request", __FUNCTION__));
		return FALSE;
	}

    RTMP_SET_PACKET_WCID(skb, pRoutingEntry->Wcid);
	RTMP_SET_PACKET_WDEV(skb, pEntry->wdev->wdev_idx);
	pAd->RalinkCounters.PendingNdisPacketCount++;
	APSendPacket(pAd, skb);

    return TRUE;
}

VOID RoutingTabMaintain(
	IN PRTMP_ADAPTER pAd)
{
    UINT32 i,BridgeIP;
	ULONG Now;
	PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;
    BOOLEAN bNeedSend = FALSE, bBridgeFound = FALSE, bNeedDelete = FALSE;
    UCHAR BridgeMAC[MAC_ADDR_LEN];

	if(RoutingTabGetEntryCount(pAd) == 0) 
		return;

#ifdef MAC_REPEATER_SUPPORT
    if(!MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, ZERO_MAC_ADDR))
    {
        COPY_MAC_ADDR(BridgeMAC, pAd->ApCfg.BridgeAddress);
        bBridgeFound = TRUE;
    }
    else
#endif /* MAC_REPEATER_SUPPORT */
    {
#ifdef LINUX
        struct net_device *pBridgeNetDev = NULL;
        struct net *net= &init_net;
        for_each_netdev(net, pBridgeNetDev)
        {
            if (pBridgeNetDev->priv_flags == IFF_EBRIDGE)
            {
                const struct in_device *pBridgeInDev = pBridgeNetDev->ip_ptr;
                if (pBridgeInDev)
                {
                    const struct in_ifaddr *ifa = pBridgeInDev->ifa_list;
                    if (ifa)
                    {
                        COPY_MAC_ADDR(BridgeMAC, pBridgeNetDev->dev_addr);
                        NdisCopyMemory(&BridgeIP, &ifa->ifa_address, 4);
                        bBridgeFound = TRUE;
                        break;
                    }
                }
            }
        }
#endif /* LINUX */
    }

    if(!bBridgeFound)
        return;

    NdisGetSystemUpTime(&Now);
	RTMP_SEM_LOCK(&pAd->ApCfg.RoutingTabLock);
	for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)
	{
	    pRoutingEntry = GetRoutingTabHead(pAd, i);
		while(pRoutingEntry)
		{
		    bNeedDelete = FALSE;
			pRoutingEntryNext = pRoutingEntry->pNext;
			if (pRoutingEntry->Valid && RTMP_TIME_AFTER(Now, pRoutingEntry->KeepAliveTime))
			{
			    if(bBridgeFound)
                {
                    if(pRoutingEntry->RetryKeepAlive >= ROUTING_ENTRY_MAX_RETRY)
                        bNeedDelete = TRUE;
                    else
                    {
                        if(RoutingEntrySendAliveCheck(pAd, pRoutingEntry, BridgeMAC, BridgeIP))
                          bNeedSend = TRUE;
                        RoutingEntryRefresh(pAd, pRoutingEntry);
                        pRoutingEntry->RetryKeepAlive++;
                    }
                }
                else
                    bNeedDelete = TRUE;
                
                if(bNeedDelete)
                {
                    delEntryList(&pAd->ApCfg.RoutingTab[i], (PLIST_ENTRY)pRoutingEntry);
                    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));
                    insertTailList(&pAd->ApCfg.RoutingEntryFreeList, (PLIST_ENTRY)pRoutingEntry);
                }
			}
			pRoutingEntry = pRoutingEntryNext;
		}
	}
	RTMP_SEM_UNLOCK(&pAd->ApCfg.RoutingTabLock);

    /* Dequeue outgoing frames from TxSwQueue[] and process it */
    if(bNeedSend)
	    RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, MAX_TX_PROCESS);
}

#endif /* ROUTING_TAB_SUPPORT */
