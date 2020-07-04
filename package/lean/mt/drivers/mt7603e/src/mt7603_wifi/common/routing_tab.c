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


	IN UCHAR ifIndex,


	IN UINT32 Flag)


{


	UINT32 i;


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


	if(pMbss->bRoutingTabInit)


    {


        pMbss->RoutingTabFlag |= Flag;


		return;


    }





	if(pMbss->pRoutingEntryPool== NULL)


		os_alloc_mem(pAd, (UCHAR **)&pMbss->pRoutingEntryPool, sizeof(ROUTING_ENTRY) * ROUTING_POOL_SIZE);


	


	if (pMbss->pRoutingEntryPool)


	{


		NdisZeroMemory(pMbss->pRoutingEntryPool, sizeof(ROUTING_ENTRY) * ROUTING_POOL_SIZE);


		initList(&pMbss->RoutingEntryFreeList);


		for (i = 0; i < ROUTING_POOL_SIZE; i++)


			insertTailList(&pMbss->RoutingEntryFreeList, (RT_LIST_ENTRY *)(pMbss->pRoutingEntryPool+i));





        for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)


		    initList(&pMbss->RoutingTab[i]);





        NdisAllocateSpinLock(pAd, &pMbss->RoutingTabLock);


        pMbss->RoutingTabFlag |= Flag;


        pMbss->bRoutingTabInit = TRUE;


	}


	else


        DBGPRINT(RT_DEBUG_ERROR, ("%s: Fail to alloc memory for MBSSID[%d].pRoutingEntryPool\n"


                , __FUNCTION__, ifIndex));


}





VOID RoutingTabDestory(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UINT32 Flag)


{


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;


    


    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit) 


		return;





    RoutingTabClear(pAd, ifIndex, Flag);


    


    pMbss->RoutingTabFlag &= ~Flag;


    if(pMbss->RoutingTabFlag == 0)


    {


        NdisFreeSpinLock(&pMbss->RoutingTabLock);


    	if (pMbss->pRoutingEntryPool)


    		os_free_mem(NULL,pMbss->pRoutingEntryPool);


    	pMbss->pRoutingEntryPool = NULL;


    	initList(&pMbss->RoutingEntryFreeList);


        pMbss->bRoutingTabInit = FALSE;


    }


}





VOID RoutingTabClear(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UINT32 Flag)


{


    INT32 i;


    BSS_STRUCT *pMbss = NULL;


    PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


    for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)


    {


    	pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, i);


    	while(pRoutingEntry)


    	{


    		pRoutingEntryNext = pRoutingEntry->pNext;


    		if(pRoutingEntry->Valid && ROUTING_ENTRY_TEST_FLAG(pRoutingEntry, Flag))


    		{


    		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);


                /* If not any entry type, then recycle to free list. */


                if(!IS_ROUTING_ENTRY(pRoutingEntry))


                {


    			    delEntryList(&pMbss->RoutingTab[i], (RT_LIST_ENTRY *)pRoutingEntry);


    			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));


    			    insertTailList(&pMbss->RoutingEntryFreeList, (RT_LIST_ENTRY *)pRoutingEntry);


                }


    		}


    		pRoutingEntry = pRoutingEntryNext;


    	}


    }


    RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);


}





PROUTING_ENTRY RoutingTabGetFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex)


{


    BSS_STRUCT *pMbss = NULL;


	PROUTING_ENTRY pRoutingEntry = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return NULL;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit)


        return NULL;





	RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


	pRoutingEntry = (PROUTING_ENTRY)removeHeadList(&pMbss->RoutingEntryFreeList);


	RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);





	return pRoutingEntry;


}





VOID RoutingTabSetAllFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN UINT32 Flag)


{


    INT32 i;


    BSS_STRUCT *pMbss = NULL;


    PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit) 


    	return;





    RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


    for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++)


    {


    	pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, i);


    	while(pRoutingEntry)


    	{


    		pRoutingEntryNext = pRoutingEntry->pNext;


    		if(pRoutingEntry->Valid && (pRoutingEntry->Wcid == Wcid))


    		{


    		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);


                /* If not any entry type, then recycle to free list. */


                if(!IS_ROUTING_ENTRY(pRoutingEntry))


                {


    			    delEntryList(&pMbss->RoutingTab[i], (RT_LIST_ENTRY *)pRoutingEntry);


    			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));


    			    insertTailList(&pMbss->RoutingEntryFreeList, (RT_LIST_ENTRY *)pRoutingEntry);


                }


    		}


    		pRoutingEntry = pRoutingEntryNext;


    	}


    }


    RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);


}





VOID  RoutingTabSetOneFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PUCHAR pMac,


	IN UINT32 Flag)


{


	INT32 HashId;


    BSS_STRUCT *pMbss = NULL;


	PROUTING_ENTRY pRoutingEntry = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit) 


    	return;





	HashId = GetHashID(pMac);


    if(HashId < 0)


    {


        DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));


        return;


    }


    


	RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


	pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, HashId);


	while (pRoutingEntry)


	{


		if (pRoutingEntry->Valid && MAC_ADDR_EQUAL(pMac, pRoutingEntry->Mac))


		{


		    CLEAR_ROUTING_ENTRY(pRoutingEntry, Flag);


             /* If not any entry type, then recycle to free list. */


            if(!IS_ROUTING_ENTRY(pRoutingEntry))


            {


                delEntryList(&pMbss->RoutingTab[HashId], (RT_LIST_ENTRY *)pRoutingEntry);


			    NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));


			    insertTailList(&pMbss->RoutingEntryFreeList, (RT_LIST_ENTRY *)pRoutingEntry);


            }


			break;


		}


		pRoutingEntry = pRoutingEntry->pNext;


	}


	RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);


}





VOID RoutingEntryRefresh(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PROUTING_ENTRY pRoutingEntry)


{


    ULONG Now;


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit || !pRoutingEntry)


        return;


     


    NdisGetSystemUpTime(&Now);


	pRoutingEntry->KeepAliveTime = Now + ROUTING_ENTRY_AGEOUT;


}





VOID RoutingEntrySet(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN PUCHAR pMac,


	IN PROUTING_ENTRY pRoutingEntry)


{


    INT32 HashId;


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit || !pRoutingEntry)


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


	RoutingEntryRefresh(pAd, ifIndex, pRoutingEntry);


    


    RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


    insertTailList(&pMbss->RoutingTab[HashId], (RT_LIST_ENTRY *)pRoutingEntry);


    RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);


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


	IN UCHAR ifIndex,


	IN INT32 Index)


{


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return NULL;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit) 


        return NULL;


    


    if(Index < ROUTING_HASH_TAB_SIZE)


        return (PROUTING_ENTRY)pMbss->RoutingTab[Index].pHead;


    else


        return NULL;


}





BOOLEAN GetRoutingEntryAll(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN UINT32 Flag,


	IN INT32 BufMaxCount,


	OUT ROUTING_ENTRY **pEntryListBuf,


	OUT PUINT32 pCount)


{


    INT32 i, Total;


    BSS_STRUCT *pMbss = NULL;


    PROUTING_ENTRY pRoutingEntry = NULL;


    BOOLEAN bFull = FALSE;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return FALSE;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


    if(!pMbss->bRoutingTabInit || !pEntryListBuf)


        return FALSE;





    RTMP_SEM_LOCK(&pMbss->RoutingTabLock);


    for (i = 0, Total = 0; i < ROUTING_HASH_TAB_SIZE; i++)


    {


        pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, i);


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


    RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);





    *pCount = Total;


    return TRUE;


}





INT RoutingTabGetEntryCount(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex)


{


	int count = 0;


    BSS_STRUCT *pMbss = NULL;





    if((ifIndex >= HW_BEACON_MAX_NUM))


        return 0;





    pMbss = &pAd->ApCfg.MBSSID[ifIndex];


	if(!pMbss->bRoutingTabInit) 


		return 0;





	count = (ROUTING_POOL_SIZE - getListSize(&pMbss->RoutingEntryFreeList));


	return count;


}





PROUTING_ENTRY RoutingTabLookup(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PUCHAR pMac,


	IN BOOLEAN bUpdateAliveTime,


	OUT UCHAR* pWcid)


{


    INT32 HashId;


    PROUTING_ENTRY pRoutingEntry = NULL;


    


    if(RoutingTabGetEntryCount(pAd, ifIndex) == 0) 


    	return NULL;


    


	HashId = GetHashID(pMac);


    if(HashId < 0)


    {


        DBGPRINT(RT_DEBUG_ERROR, ("%s: Hash Id isn't correct!\n", __FUNCTION__));


        return NULL;


    }





	pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, HashId);


	while (pRoutingEntry)


	{


		if (pRoutingEntry->Valid && MAC_ADDR_EQUAL(pMac, pRoutingEntry->Mac))


		{


			if (VALID_WCID(pRoutingEntry->Wcid))


			{


				if(bUpdateAliveTime)


				{


                    RoutingEntryRefresh(pAd, ifIndex, pRoutingEntry);


                    pRoutingEntry->Retry = 0;
#ifdef A4_CONN
					pRoutingEntry->NeedRefresh = FALSE;
#endif

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


    IN UCHAR ifIndex,


    IN PROUTING_ENTRY pRoutingEntry,


    IN UINT32 ARPSenderIP)


{


    if(!pRoutingEntry || (RoutingTabGetEntryCount(pAd, ifIndex) == 0))


        return;





    NdisCopyMemory(&pRoutingEntry->IPAddr, &ARPSenderIP, 4);


}





INT RoutingEntrySendAliveCheck(


    IN PRTMP_ADAPTER pAd,


    IN UCHAR ifIndex,


    IN PROUTING_ENTRY pRoutingEntry,


    IN UCHAR *pSrcMAC,


    IN UINT32 SrcIP)


{


    struct wifi_dev *wdev = NULL;


    struct sk_buff *skb = NULL;


    PMAC_TABLE_ENTRY pEntry = NULL;





    if(!pRoutingEntry || !pSrcMAC || 


        !VALID_WCID(pRoutingEntry->Wcid) || 


        (SrcIP == 0) || (pRoutingEntry->IPAddr == 0))


        return FALSE;





    if(RoutingTabGetEntryCount(pAd, ifIndex) == 0)


        return FALSE;





    pEntry = &pAd->MacTab.Content[pRoutingEntry->Wcid];


    wdev = pEntry->wdev;


    if(!wdev)


        return FALSE;


    


    skb = arp_create(ARPOP_REQUEST, ETH_P_ARP, pRoutingEntry->IPAddr, wdev->if_dev,


                     SrcIP, BROADCAST_ADDR, pSrcMAC, ZERO_MAC_ADDR);


    if(!skb)


    {


        DBGPRINT(RT_DEBUG_ERROR, ("%s: Fail to alloc memory for arp request!\n", __FUNCTION__));


		return FALSE;


	}





    RTMP_SET_PACKET_WCID(skb, pRoutingEntry->Wcid);


	RTMP_SET_PACKET_WDEV(skb, wdev->wdev_idx);


    RTMP_SET_PACKET_MOREDATA(skb, FALSE);


#ifdef CUT_THROUGH


#ifdef CUT_THROUGH_FULL_OFFLOAD


	if(wdev->tx_pkt_ct_handle)


    {


        UCHAR que_idx = 0, user_prio = QID_AC_BE;


        wdev->tx_pkt_ct_handle(pAd, skb, que_idx, user_prio);


    }


    else


#endif /* CUT_THROUGH_FULL_OFFLOAD */ 


#endif /* CUT_THROUGH */


    if(wdev->tx_pkt_handle)


		wdev->tx_pkt_handle(pAd, skb);





    return TRUE;


}





VOID RoutingTabMaintain(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex)
{
	UINT32 i = 0, BridgeIP = 0;
	ULONG Now = 0;
	BSS_STRUCT *pMbss = NULL;
	PROUTING_ENTRY pRoutingEntry = NULL, pRoutingEntryNext = NULL;
	BOOLEAN bNeedSend = FALSE, bBridgeFound = FALSE, bNeedDelete = FALSE, bCreateARP = FALSE;
	UCHAR BridgeMAC[MAC_ADDR_LEN] = {0};

	if (ifIndex >= HW_BEACON_MAX_NUM)
		return;

	if (RoutingTabGetEntryCount(pAd, ifIndex) == 0)
		return;

#ifdef MAC_REPEATER_SUPPORT
	if (!MAC_ADDR_EQUAL(pAd->ApCfg.BridgeAddress, ZERO_MAC_ADDR)) {
		COPY_MAC_ADDR(BridgeMAC, pAd->ApCfg.BridgeAddress);
		bBridgeFound = TRUE;
	} else
#endif /* MAC_REPEATER_SUPPORT */
	{
#ifdef LINUX
		struct net_device *pBridgeNetDev = NULL;
		struct net *net = &init_net;

		for_each_netdev(net, pBridgeNetDev) {
			if (pBridgeNetDev->priv_flags == IFF_EBRIDGE) {
				const struct in_device *pBridgeInDev = pBridgeNetDev->ip_ptr;

				if (pBridgeInDev) {
					const struct in_ifaddr *ifa = pBridgeInDev->ifa_list;

					if (ifa) {
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

	if (!bBridgeFound)
		return;

    pMbss = &pAd->ApCfg.MBSSID[ifIndex];
    NdisGetSystemUpTime(&Now);
	RTMP_SEM_LOCK(&pMbss->RoutingTabLock);

	for (i = 0; i < ROUTING_HASH_TAB_SIZE; i++) {
		pRoutingEntry = GetRoutingTabHead(pAd, ifIndex, i);

		while (pRoutingEntry) {
			bCreateARP = FALSE;
			bNeedDelete = FALSE;
			pRoutingEntryNext = pRoutingEntry->pNext;
#ifdef A4_CONN
			if (pAd->a4_need_refresh)
				pRoutingEntry->NeedRefresh = TRUE;
#endif

            /* Stage 1 Check*/
			if ((pRoutingEntry->Valid && RTMP_TIME_AFTER(Now, pRoutingEntry->KeepAliveTime)
				&& pRoutingEntry->IPAddr != 0)/*porting from mt7615*/
#ifdef A4_CONN
				|| pRoutingEntry->NeedRefresh
#endif
			) {
			    if (bBridgeFound) {
					/* Stage 2 Check*/

					/*porting from mt7615*/
					if (pRoutingEntry->Retry == 0 || RTMP_TIME_AFTER(Now, pRoutingEntry->RetryTime)
#ifdef A4_CONN
						|| pRoutingEntry->NeedRefresh
#endif
					) {
						if (pRoutingEntry->Retry >= ROUTING_ENTRY_MAX_RETRY)
							bNeedDelete = TRUE;
						else {
							bCreateARP = TRUE;
							pRoutingEntry->Retry++;
							pRoutingEntry->RetryTime = Now + ROUTING_ENTRY_RETRY_TIME;
						}
					}

					if (bCreateARP) {
						if (RoutingEntrySendAliveCheck(pAd, ifIndex, pRoutingEntry, BridgeMAC, BridgeIP))
							bNeedSend = TRUE;
					}
				} else
					bNeedDelete = TRUE;

				if (bNeedDelete) {
					delEntryList(&pMbss->RoutingTab[i], (RT_LIST_ENTRY *)pRoutingEntry);
					NdisZeroMemory(pRoutingEntry, sizeof(ROUTING_ENTRY));
					insertTailList(&pMbss->RoutingEntryFreeList, (RT_LIST_ENTRY *)pRoutingEntry);
				}
			} else
				pRoutingEntry->Retry = 0;

			pRoutingEntry = pRoutingEntryNext;
		}
	}
	RTMP_SEM_UNLOCK(&pMbss->RoutingTabLock);

	/* Dequeue outgoing frames from TxSwQueue[] and process it */
	if (bNeedSend)
		RTMPDeQueuePacket(pAd, FALSE, WMM_NUM_OF_AC, WCID_ALL, MAX_TX_PROCESS);
}

#endif /* ROUTING_TAB_SUPPORT */



