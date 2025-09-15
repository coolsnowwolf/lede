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
	mwds.c
 
    Abstract:
    This is MWDS feature used to process those 4-addr of connected APClient or STA.
    
    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rt_config.h"

VOID MWDSConnEntryListInit(
	IN PRTMP_ADAPTER pAd)
{
	if(pAd->ApCfg.bMWDSAPInit) 
		return;
    
	NdisAllocateSpinLock(pAd, &pAd->ApCfg.MWDSConnEntryLock);
	DlListInit(&pAd->ApCfg.MWDSConnEntryList);
}

VOID MWDSConnEntryListClear(
	IN PRTMP_ADAPTER pAd)
{
	PMWDS_CONNECT_ENTRY pConnEntry = NULL,pConnEntryTmp = NULL;
	PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;
	
	if(!pAd->ApCfg.bMWDSAPInit) 
		return;

	DlListForEachSafe(pConnEntry, pConnEntryTmp, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
	{
		if(pConnEntry)
		{
			DlListDel(&pConnEntry->List);
			os_free_mem(NULL, pConnEntry);
		}
	}
    
	NdisFreeSpinLock(&pAd->ApCfg.MWDSConnEntryLock);
}

BOOLEAN MWDSConnEntryLookupByWCID(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid)
{
	ULONG idx;
	PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;
	PMWDS_CONNECT_ENTRY pConnEntry = NULL;
	BOOLEAN bFound = FALSE;

	if(MWDSGetConnEntryCount(pAd) == 0) 
		return FALSE;
	
	DlListForEach(pConnEntry, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
	{
		if(pConnEntry && 
		   pConnEntry->Valid &&
		  (pConnEntry->wcid == wcid))
		{
			bFound = TRUE;
			break;
		}
	}

	return bFound;
}

BOOLEAN MWDSConnEntryLookupByAddr(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac)
{
	ULONG idx;
	PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;
	PMWDS_CONNECT_ENTRY pConnEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	BOOLEAN bFound = FALSE;

	if(MWDSGetConnEntryCount(pAd) == 0 || pMac == NULL) 
		return FALSE;
	
	DlListForEach(pConnEntry, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
	{
		if(pConnEntry && pConnEntry->Valid && VALID_WCID(pConnEntry->wcid))
		{
			pEntry = &pAd->MacTab.Content[pConnEntry->wcid];
			if(MAC_ADDR_EQUAL(pMac,pEntry->Addr))
			{
				bFound = TRUE;
				break;
			}
		}
	}

	return bFound;
}

VOID MWDSConnEntryUpdate(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid)
{
	PMWDS_CONNECT_ENTRY pNewConnEntry = NULL;

	if(MWDSConnEntryLookupByWCID(pAd, wcid)) 
		return;
	
	os_alloc_mem(pAd, (UCHAR **)&pNewConnEntry, sizeof(MWDS_CONNECT_ENTRY));
	if(pNewConnEntry)
	{
		NdisZeroMemory(pNewConnEntry, sizeof(MWDS_CONNECT_ENTRY));
		pNewConnEntry->Valid = 1;
		pNewConnEntry->wcid = wcid;
		RTMP_SEM_LOCK(&pAd->ApCfg.MWDSConnEntryLock);
		DlListAddTail(&pAd->ApCfg.MWDSConnEntryList,&pNewConnEntry->List);
		RTMP_SEM_UNLOCK(&pAd->ApCfg.MWDSConnEntryLock);
	}
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s: Fail to alloc memory for pNewConnEntry", __FUNCTION__));
}

VOID MWDSConnEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid)
{
	PMWDS_CONNECT_ENTRY pConnEntry = NULL;
	PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;

	if(MWDSGetConnEntryCount(pAd) == 0) 
		return;
	
	RTMP_SEM_LOCK(&pAd->ApCfg.MWDSConnEntryLock);
	DlListForEach(pConnEntry, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
	{
		if(pConnEntry &&
		   pConnEntry->Valid &&
		  (pConnEntry->wcid == wcid))
		{
			DlListDel(&pConnEntry->List);
			os_free_mem(NULL, pConnEntry);
			RoutingTabSetAllFree(pAd, wcid, ROUTING_ENTRY_MWDS);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pAd->ApCfg.MWDSConnEntryLock);
}

INT MWDSGetConnEntryCount(
	IN PRTMP_ADAPTER pAd )
{
	int count = 0;
	
	if(!pAd->ApCfg.bMWDSAPInit) 
		return 0;

	count = DlListLen(&pAd->ApCfg.MWDSConnEntryList);
	return count;
}

BOOLEAN ISMWDSValid(
    IN PRTMP_ADAPTER pAd)
{
    if(!pAd->ApCfg.bMWDSAPInit || (MWDSGetConnEntryCount(pAd) == 0))
        return FALSE;

    return TRUE;
}

VOID MWDSProxyEntryDelete(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac)
{
    if(!ISMWDSValid(pAd))
        return;

    RoutingTabSetOneFree(pAd, pMac, ROUTING_ENTRY_MWDS);
}

BOOLEAN MWDSProxyLookup(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMac,
	IN BOOLEAN bUpdateAliveTime,
	OUT UCHAR *pWcid)
{
    UCHAR Wcid;
    
    if(!ISMWDSValid(pAd))
        return FALSE;
    
    if(RoutingTabLookup(pAd, pMac, bUpdateAliveTime, &Wcid) != NULL)
    {
        *pWcid = Wcid;
        return TRUE;
    }
    else
        return FALSE;
}

VOID MWDSProxyTabUpdate(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR wcid,
	IN PUCHAR pMac)
{
    UCHAR ProxyAPWcid = 0;
    BOOLEAN bFound = FALSE;
    PROUTING_ENTRY pRoutingEntry = NULL;

    if(!ISMWDSValid(pAd))
        return;

	if(!VALID_WCID(wcid) || !pMac) 
		return;
    
    pRoutingEntry = RoutingTabLookup(pAd, pMac, TRUE, &ProxyAPWcid);
    bFound = (pRoutingEntry != NULL)?TRUE:FALSE;
    if(bFound)
    {
        if(ROUTING_ENTRY_TEST_FLAG(pRoutingEntry, ROUTING_ENTRY_MWDS))
        {
             /* Mean the target change to other ProxyAP */
            if((ProxyAPWcid != wcid))
                RoutingTabSetOneFree(pAd, pMac, ROUTING_ENTRY_MWDS);
            else
                return;
        }
        else
        {
            /* Assign MWDS falg to this one if found. */
            SET_ROUTING_ENTRY(pRoutingEntry, ROUTING_ENTRY_MWDS);
        }
    }

    if(!bFound)
    {
        /* Allocate a new one if not found. */
	    pRoutingEntry = RoutingTabGetFree(pAd);
        if (pRoutingEntry)
    	{
    	    SET_ROUTING_ENTRY(pRoutingEntry, ROUTING_ENTRY_MWDS);
    		RoutingEntrySet(pAd, wcid, pMac, pRoutingEntry);
    	}
    }
}

VOID MWDSProxyTabMaintain(
	IN PRTMP_ADAPTER pAd)
{
    if(!ISMWDSValid(pAd))
        return;
    
    RoutingTabMaintain(pAd);
}

PNDIS_PACKET MWDSClonePacket(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket)
{
	PNDIS_PACKET pPacketClone = NULL;
	OS_PKT_CLONE(pAd, pPacket, pPacketClone, GFP_ATOMIC);
	if (pPacketClone == NULL) 
		return NULL;

	return pPacketClone;
}

void MWDSSendClonePacket(
	IN PRTMP_ADAPTER pAd,
	IN PNDIS_PACKET pPacket,
	IN PUCHAR pExcludeMac)
{
	PNDIS_PACKET pPacketClone = NULL;
	PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;
	PMWDS_CONNECT_ENTRY pConnEntry = NULL;
    PROUTING_ENTRY pRoutingEntry = NULL;
	PMAC_TABLE_ENTRY pEntry = NULL;
	BOOLEAN bFound = FALSE;
    UCHAR Wcid = 0;
	
	if(pPacket && (MWDSGetConnEntryCount(pAd) > 0))
	{
	    if(pExcludeMac)
        {
            pRoutingEntry = RoutingTabLookup(pAd,pExcludeMac,FALSE,&Wcid);
            if(pRoutingEntry && ROUTING_ENTRY_TEST_FLAG(pRoutingEntry, ROUTING_ENTRY_MWDS))
                bFound = TRUE;
            else
                bFound = FALSE;
        }
        
		DlListForEach(pConnEntry, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
		{
			if (pConnEntry && pConnEntry->Valid && VALID_WCID(pConnEntry->wcid))
			{
			    if(bFound && (Wcid == pConnEntry->wcid))
                    continue;
                
				OS_PKT_CLONE(pAd, pPacket, pPacketClone, GFP_ATOMIC);
				if (pPacketClone == NULL)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("%s Fail to alloc memory for pPacketClone", __FUNCTION__));
					return;
				}
                pEntry = &pAd->MacTab.Content[pConnEntry->wcid];
				RTMP_SET_PACKET_WCID(pPacketClone, pEntry->wcid);
				RTMP_SET_PACKET_WDEV(pPacketClone, pEntry->wdev->wdev_idx);
				pAd->RalinkCounters.PendingNdisPacketCount++;
				APSendPacket(pAd, pPacketClone);
			}
		}
	}
}

BOOLEAN MWDSARPLookupUpdate(
    IN PRTMP_ADAPTER pAd,
    IN PUCHAR pData)
{
    if(!ISMWDSValid(pAd))
        return FALSE;

    RoutingTabARPLookupUpdate(pAd, pData);
    return TRUE;
}

INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP)
{
	if(isAP)
	{
		if(ifIndex < HW_BEACON_MAX_NUM)
			pAd->ApCfg.MBSSID[ifIndex].bSupportMWDS = TRUE;

		MWDSAPUP(pAd);
	}
#ifdef APCLI_SUPPORT
	else
	{
		if(ifIndex < MAX_APCLI_NUM)
			pAd->ApCfg.ApCliTab[ifIndex].bSupportMWDS = TRUE;
	}
#endif /* APCLI_SUPPORT */
	return TRUE;
}

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP)
{
	if(isAP)
	{
		if(ifIndex < HW_BEACON_MAX_NUM)
			pAd->ApCfg.MBSSID[ifIndex].bSupportMWDS = FALSE;

		MWDSAPDown(pAd);
	}
#ifdef APCLI_SUPPORT
	else
	{
		if(ifIndex < MAX_APCLI_NUM)
			pAd->ApCfg.ApCliTab[ifIndex].bSupportMWDS = FALSE;
	}
#endif /* APCLI_SUPPORT */

	return TRUE;
}

INT MWDSAPUP(
	IN PRTMP_ADAPTER pAd)
{
	MWDSConnEntryListInit(pAd);
	RoutingTabInit(pAd, ROUTING_ENTRY_MWDS);
	pAd->ApCfg.bMWDSAPInit = TRUE;

	return TRUE;
}

INT MWDSAPDown(
	IN PRTMP_ADAPTER pAd)
{
	MWDSConnEntryListClear(pAd);
	RoutingTabDestory(pAd, ROUTING_ENTRY_MWDS);
	pAd->ApCfg.bMWDSAPInit = FALSE;

	return TRUE;
}

INT	Set_Enable_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  BOOLEAN Enable,
	IN  BOOLEAN isAP)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
	if(isAP)
	{
		ifIndex = pObj->ioctl_if;
	}
#ifdef APCLI_SUPPORT
	else
	{
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;
		ifIndex = pObj->ioctl_if;
	}
#endif /* APCLI_SUPPORT */

	if (Enable)
		MWDSEnable(pAd,ifIndex,isAP);
	else
		MWDSDisable(pAd,ifIndex,isAP);

	return TRUE;
}

INT Set_Ap_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	
	return Set_Enable_MWDS_Proc(pAd,Enable,TRUE);
}

INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	
	return Set_Enable_MWDS_Proc(pAd,Enable,FALSE);
}

INT Set_Ap_MWDS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE 		pObj;
	UCHAR 			ifIndex;
	BOOLEAN bEnable;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
#ifdef SMART_MESH
    PSMART_MESH_CFG pSmartMeshCfg = NULL;
    PNTGR_IE pNtgr_IE = NULL;
#endif /* SMART_MESH */

	if(ifIndex < HW_BEACON_MAX_NUM)
    {
#ifdef SMART_MESH
		pSmartMeshCfg = &pAd->ApCfg.MBSSID[ifIndex].SmartMeshCfg;
#endif /* SMART_MESH */
    }
	else
		return FALSE;
#ifdef SMART_MESH
	if(pSmartMeshCfg)
	{
        pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
        if((pNtgr_IE->private[0]&0x1))
        	bEnable = TRUE;
        else
        	bEnable = FALSE;
	}
    else
#endif /* SMART_MESH */
    {
        bEnable = FALSE;
	}

    DBGPRINT(RT_DEBUG_OFF,("%d\n", bEnable));
	return TRUE;
}

INT Set_ApCli_MWDS_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	POS_COOKIE		pObj;
	UCHAR			ifIndex;
	BOOLEAN bEnable, bSupportMWDS;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
#ifdef SMART_MESH
    PSMART_MESH_CFG pSmartMeshCfg = NULL;
    PNTGR_IE pNtgr_IE = NULL;
#endif /* SMART_MESH */

	if((pObj->ioctl_if_type == INT_APCLI) && (ifIndex < MAX_APCLI_NUM))
    {
 #ifdef SMART_MESH
		pSmartMeshCfg = &pAd->ApCfg.ApCliTab[ifIndex].SmartMeshCfg;
 #endif /* SMART_MESH */
        bSupportMWDS = pAd->ApCfg.ApCliTab[ifIndex].bSupportMWDS;
    }
	else
		return FALSE;
#ifdef SMART_MESH
	if(pSmartMeshCfg)
	{
		pNtgr_IE = &pSmartMeshCfg->Ntgr_IE;
		if(bSupportMWDS && (pNtgr_IE->private[0]&0x1))
			bEnable = TRUE;
		else
			bEnable = FALSE;
	}
    else
#endif /* SMART_MESH */
    {
        bEnable = bSupportMWDS;
    }
    DBGPRINT(RT_DEBUG_OFF,("%d\n", bEnable));

	return TRUE;
}

INT Set_APProxy_Status_Show_Proc(
	IN  PRTMP_ADAPTER pAd, 
	IN  PSTRING arg)
{
	INT32 i,count=0;
    UINT32 ip_addr=0;
    ULONG Now=0, AliveTime=0;
    PDL_LIST pMWDSConnEntryList = &pAd->ApCfg.MWDSConnEntryList;
	PMWDS_CONNECT_ENTRY pConnEntry = NULL;
    PROUTING_ENTRY pRoutingEntry = NULL, *RoutingEntryList[ROUTING_POOL_SIZE];
    UCHAR *pProxyMac = NULL, ProxyMacIP[64];

	if((MWDSGetConnEntryCount(pAd) == 0)) 
		return TRUE;

    NdisGetSystemUpTime(&Now);
    DlListForEach(pConnEntry, pMWDSConnEntryList, MWDS_CONNECT_ENTRY, List)
	{
	    if (pConnEntry && pConnEntry->Valid && VALID_WCID(pConnEntry->wcid))
        {
            count = 0;
            pProxyMac = pAd->MacTab.Content[pConnEntry->wcid].Addr;
            DBGPRINT(RT_DEBUG_OFF,("Proxy Mac: %02X:%02X:%02X:%02X:%02X:%02X\n", PRINT_MAC(pProxyMac)));
            if(GetRoutingEntryAll(pAd, pConnEntry->wcid, ROUTING_ENTRY_MWDS,
                                    ROUTING_POOL_SIZE, &RoutingEntryList, &count))
            {
                for (i = 0; i < count; i++)
                {
                    pRoutingEntry = RoutingEntryList[i];
                    if(!pRoutingEntry)
                        continue;
                    
                    if(pRoutingEntry->KeepAliveTime >= Now)
                        AliveTime = ((pRoutingEntry->KeepAliveTime - Now) / OS_HZ);
                    else
                        AliveTime = 0;
                    if(pRoutingEntry->IPAddr != 0)
                    {
                        ip_addr = pRoutingEntry->IPAddr;
                        sprintf(ProxyMacIP, "%d.%d.%d.%d", (ip_addr & 0xff), ((ip_addr & (0xff << 8)) >> 8),
                                ((ip_addr & (0xff << 16)) >> 16), ((ip_addr & (0xff << 24)) >> 24));
                    }
                    else
                        strcpy(ProxyMacIP,"0.0.0.0");
                    DBGPRINT(RT_DEBUG_OFF,("\tMAC: %02X:%02X:%02X:%02X:%02X:%02X\tIP: %s\tAgeOut: %lus\tRetry: (%d,%d)\n",
                                 PRINT_MAC(pRoutingEntry->Mac), ProxyMacIP, AliveTime, 
                                 pRoutingEntry->RetryKeepAlive,ROUTING_ENTRY_MAX_RETRY));
                }
                DBGPRINT(RT_DEBUG_OFF,("Total Count = %d\n\n",count));
            }
        }   
    }
    
	return TRUE;
}

VOID rtmp_read_MWDS_from_file(
	IN  PRTMP_ADAPTER pAd,
	PSTRING tmpbuf,
	PSTRING buffer)
{
	PSTRING	tmpptr = NULL;
	
#ifdef CONFIG_AP_SUPPORT
	/* ApMWDS */
	if(RTMPGetKeyParameter("ApMWDS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= pAd->ApCfg.BssidNum)
				break;

			if (Value == 0)
			{
				MWDSDisable(pAd,i,TRUE);
			}
		 	else
		 	{
		 		MWDSEnable(pAd,i,TRUE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApMWDS=%d\n", Value));
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
	/* ApCliMWDS */
	if(RTMPGetKeyParameter("ApCliMWDS", tmpbuf, 256, buffer, TRUE))
	{
		INT	Value;
		UCHAR i=0;
		
		Value = (INT) simple_strtol(tmpbuf, 0, 10);
		for (i = 0, tmpptr = rstrtok(tmpbuf,";"); tmpptr; tmpptr = rstrtok(NULL,";"), i++)
		{
			if (i >= MAX_APCLI_NUM)
				break;

			if (Value == 0)
			{
				MWDSDisable(pAd,i,FALSE);
			}
		 	else
		 	{
		 		MWDSEnable(pAd,i,FALSE);
		 	}
			DBGPRINT(RT_DEBUG_TRACE, ("ApCliMWDS=%d\n", Value));
		}
	}
#endif /* APCLI_SUPPORT */
}

#endif /* MWDS */
