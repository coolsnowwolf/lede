#ifndef __ROUTING_TAB_H__


#define __ROUTING_TAB_H__


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


	routing_tab.h


 


    Abstract:


    This is a tab used to record all entries behind associated APClinet or STA/PC.


    


    Revision History:


    Who          When          What


    ---------    ----------    ----------------------------------------------


 */





#ifdef ROUTING_TAB_SUPPORT


#include "rtmp_def.h"




#define ROUTING_ENTRY_AGEOUT (10*OS_HZ)  /* seconds */


#define ROUTING_ENTRY_RETRY_TIME (2*OS_HZ)  /* seconds */


#define ROUTING_ENTRY_MAX_RETRY 5


#define ROUTING_POOL_SIZE 128


#define ROUTING_HASH_TAB_SIZE 64  /* the legth of hash table must be power of 2. */


typedef struct _RTMP_ADAPTER *PRTMP_ADAPTER;


enum ROUTING_ENTRY_FLAG {


	ROUTING_ENTRY_NONE = 0x0,


	ROUTING_ENTRY_A4 = 0x1,


	ROUTING_ENTRY_IGMP = 0x2,


	ROUTING_ALL_MASK = 0x3


};





#define IS_VALID_ROUTING_ENTRY_FLAG(_x)	 (((_x) & ROUTING_ALL_MASK) != 0)


#define IS_ROUTING_ENTRY(_x)			 ((_x)->EntryFlag != ROUTING_ENTRY_NONE)


#define SET_ROUTING_ENTRY(_x, _y)	     ((_x)->EntryFlag |= (_y))


#define CLEAR_ROUTING_ENTRY(_x, _y)	     ((_x)->EntryFlag &= ~(_y))


#define ROUTING_ENTRY_TEST_FLAG(_x, _y)  (((_x)->EntryFlag & (_y)) != 0)





typedef struct _ROUTING_ENTRY {


	struct _ROUTING_ENTRY *pNext;


    UINT32 EntryFlag;


    ULONG KeepAliveTime;


    ULONG RetryTime;


    UCHAR Retry;


    UCHAR Valid;


    UCHAR Wcid;


    UINT32 IPAddr;


    UCHAR Mac[MAC_ADDR_LEN];
#ifdef A4_CONN
	UCHAR NeedRefresh;
#endif
} ROUTING_ENTRY, *PROUTING_ENTRY;








VOID RoutingTabInit(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UINT32 Flag);





VOID RoutingTabDestory(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UINT32 Flag);





VOID RoutingTabClear(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UINT32 Flag);





PROUTING_ENTRY RoutingTabGetFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex);





VOID RoutingTabSetAllFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN UINT32 Flag);





VOID  RoutingTabSetOneFree(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PUCHAR pMac,


	IN UINT32 Flag);





VOID RoutingEntryRefresh(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PROUTING_ENTRY pRoutingEntry);





VOID RoutingEntrySet(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN PUCHAR pMac,


	IN PROUTING_ENTRY pRoutingEntry);





INT RoutingTabGetEntryCount(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex);





INT32 GetHashID(


    IN PUCHAR pMac);





PROUTING_ENTRY GetRoutingTabHead(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN INT32 Index);





BOOLEAN GetRoutingEntryAll(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN UCHAR Wcid,


	IN UINT32 Flag,


	IN INT32 BufMaxCount,


	OUT ROUTING_ENTRY **pEntryListBuf,


	OUT PUINT32 pCount);





PROUTING_ENTRY RoutingTabLookup(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex,


	IN PUCHAR pMac,


	IN BOOLEAN bUpdateAliveTime,


	OUT UCHAR* pWcid);





VOID RoutingTabARPLookupUpdate(


    IN PRTMP_ADAPTER pAd,


    IN UCHAR ifIndex,


    IN PROUTING_ENTRY pRoutingEntry,


    IN UINT32 ARPSenderIP);





INT RoutingEntrySendAliveCheck(


    IN PRTMP_ADAPTER pAd,


    IN UCHAR ifIndex,


    IN PROUTING_ENTRY pRoutingEntry,


    IN UCHAR *pSrcMAC,


    IN UINT32 SrcIP);





VOID RoutingTabMaintain(


	IN PRTMP_ADAPTER pAd,


	IN UCHAR ifIndex);





#endif /* ROUTING_TAB_SUPPORT */


#endif /* __ROUTING_TAB_H__ */


