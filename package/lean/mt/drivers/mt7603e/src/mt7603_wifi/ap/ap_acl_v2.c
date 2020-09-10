
#ifdef ACL_V2_SUPPORT

#include "rt_config.h"

// check http://www.coffer.com/mac_find/ not all can be found, need huawei to refine this table!!
OUI_ENTRY OUI_ACL_TABLE[] =
{
	{FBT_IOT_PEER_UNKNOWN, {0x00,0x00,0x00}},

	/*Realtek why need 5 IDs ??, only 3 can be found*/
	{FBT_IOT_PEER_REALTEK, {0x00,0x48,0x54}}, //first three
	{FBT_IOT_PEER_REALTEK_92SE, {0x00,0xe0,0x4c}}, //first three
	{FBT_IOT_PEER_REALTEK_SOFTAP, {0x52,0x54,0x05}}, //first three
	//{FBT_IOT_PEER_RTK_APCLIENT, {0x00,0x00,0x00}}, //can't be found
	//{FBT_IOT_PEER_REALTEK_81XX, {0x00,0x00,0x00}}, //can't be found
	//{FBT_IOT_PEER_REALTEK_WOW, {0x00,0x00,0x00}}, //can't be found

	{FBT_IOT_PEER_BROADCOM, {0x00,0x05,0xb5}},

	{FBT_IOT_PEER_RALINK, {0x00,0x0c,0x43}},

	{FBT_IOT_PEER_ATHEROS, {0x00,0x03,0x7f}},
	{FBT_IOT_PEER_ATHEROS, {0x00,0x13,0x74}},
	{FBT_IOT_PEER_ATHEROS, {0x88,0x12,0x4e}},

	/*too many cisco OUIs, only keep 3*/
	{FBT_IOT_PEER_CISCO, {0x00,0x00,0x0c}},
	{FBT_IOT_PEER_CISCO, {0x00,0x01,0x42}},
	{FBT_IOT_PEER_CISCO, {0x00,0x01,0x43}},

	{FBT_IOT_PEER_MERU, {0x00,0x0c,0xe6}},

	{FBT_IOT_PEER_MARVEL, {0x00,0x50,0x43}},
	
	//{FBT_IOT_PEER_SELF_SOFTAP, {000000}}, // can't be found

	{FBT_IOT_PEER_AIRGO, {0x00,0x0a,0xf5}},

	/*too many cisco OUIs, only keep 3*/
	{FBT_IOT_PEER_INTEL, {0x00,0x02,0xb3}},
	{FBT_IOT_PEER_INTEL, {0x00,0x03,0x47}},
	{FBT_IOT_PEER_INTEL, {0x00,0x04,0x23}},

	/*too many , only keep 1! */
	{FBT_IOT_PEER_HTC, {0x00,0x04,0x23}},
	//{FBT_IOT_PEER_MAX, {0x00,0x00,0x00}}, // can't be found
};

INT	OUI_ACL_TABLE_NUM = (sizeof(OUI_ACL_TABLE)/sizeof(OUI_ENTRY));



VOID ACL_V2_Timeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3);


BUILD_TIMER_FUNCTION(ACL_V2_Timeout);



VOID ACL_V2_CtrlInit(IN PRTMP_ADAPTER pAd)
{
	PACL_V2_CTRL pACLCtrl;
#ifdef CONFIG_AP_SUPPORT
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pACLCtrl = &pAd->ApCfg.MBSSID[APIndex].AccessControlList_V2;
		
		NdisZeroMemory(pACLCtrl, sizeof(*pACLCtrl));
		pACLCtrl->BlockTime = 2; //spec is 2 seconds by default

		pACLCtrl->pAd = pAd;
		
		RTMPInitTimer(pAd, &pACLCtrl->AgeOutTimer, 
						GET_TIMER_FUNCTION(ACL_V2_Timeout), pACLCtrl, FALSE);
		
		pACLCtrl->AgeOutTimer_Running = FALSE;
		NdisAllocateSpinLock(pAd, &pACLCtrl->MAC_ListLock);
		NdisAllocateSpinLock(pAd, &pACLCtrl->OUI_ListLock);

		DlListInit(&pACLCtrl->MAC_List);
		DlListInit(&pACLCtrl->OUI_List);
	}
#endif 
}

VOID ACL_V2_CtrlExit(IN PRTMP_ADAPTER pAd)
{
	PACL_V2_CTRL pACLCtrl;
	UINT32 Ret;
	BOOLEAN Cancelled;
	
#ifdef CONFIG_AP_SUPPORT
	ACL_MAC_CTRL *pMACEntry, *pMACEntryTmp;
	ACL_OUI_CTRL *pOUIEntry, *pOUITmp;
	UCHAR APIndex;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	for (APIndex = 0; APIndex < MAX_MBSSID_NUM(pAd); APIndex++)
	{
		pACLCtrl = &pAd->ApCfg.MBSSID[APIndex].AccessControlList_V2;
		
		RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);

		DlListForEachSafe(pMACEntry, pMACEntryTmp, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
		{
			DlListDel(&pMACEntry->List);
			os_free_mem(NULL, pMACEntry);
		}
		DlListInit(&pACLCtrl->MAC_List);
		RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);

		RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
		DlListForEachSafe(pOUIEntry, pOUITmp,	&pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
		{
			DlListDel(&pOUIEntry->List);
			os_free_mem(NULL, pOUIEntry);
		}
		DlListInit(&pACLCtrl->OUI_List);
		RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
		
		if(pACLCtrl->AgeOutTimer_Running == TRUE)
		{
			RTMPCancelTimer(&pACLCtrl->AgeOutTimer, &Cancelled);			
			pACLCtrl->AgeOutTimer_Running = FALSE;
		}
		RTMPReleaseTimer(&pACLCtrl->AgeOutTimer, &Cancelled);

		NdisFreeSpinLock(&pACLCtrl->MAC_ListLock);
		NdisFreeSpinLock(&pACLCtrl->OUI_ListLock);

	}
#endif /* CONFIG_AP_SUPPORT */
}

INT	Set_ACL_V2_DisConnectSta_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	UCHAR					macAddr[MAC_ADDR_LEN];
	RTMP_STRING *value;
	INT						i;
	MAC_TABLE_ENTRY *pEntry = NULL;

	if(strlen(arg) != 17)  /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
		return FALSE;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":"))
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) )
			return FALSE;  /*Invalid */

		AtoH(value, (UCHAR *)&macAddr[i++], 1);
	}

	pEntry = MacTableLookup(pAd, macAddr);


	Set_ACL_V2_AddSTAEntry_Proc(pAd, macAddr);

	if (pEntry)
	{
		MlmeDeAuthAction(pAd, pEntry, REASON_DISASSOC_STA_LEAVING, FALSE);
	}

	DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,PRINT_MAC(macAddr)));	

	return TRUE;
}


INT	Set_ACL_V2_DisConnectOUI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT	i=0;
	INT32 Ret;	
	BOOLEAN Cancelled;
	PACL_V2_CTRL pACLCtrl;
	ACL_OUI_CTRL *pOUIEntry, *pOUIEntryTmp;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	ULONG Value = (ULONG) simple_strtol(arg, 0, 16);

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	if(pACLCtrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&pACLCtrl->AgeOutTimer, &Cancelled);
		pACLCtrl->AgeOutTimer_Running = FALSE;
	}
	if (Value)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: OUI=0x%08x\n", __FUNCTION__,Value));
		RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
		if(pACLCtrl->OUI_Enable)
		{
			DlListForEachSafe(pOUIEntry, pOUIEntryTmp, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
			{
				DlListDel(&pOUIEntry->List);
				pACLCtrl->OUI_ListNum --;
				if(pACLCtrl->OUI_ListNum == 0)
				{
					pACLCtrl->OUI_Enable = 0;
				}
				os_free_mem(NULL, pOUIEntry);
			}
		}

		DlListInit(&pACLCtrl->OUI_List);		
		RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
		
		for (i=0; i <OUI_ACL_TABLE_NUM ; i++)
		{
			if(Value & OUI_ACL_TABLE[i].ID)
			{
				Set_ACL_V2_AddOUIEntry_Proc(pAd, OUI_ACL_TABLE[i].OUI);
			}
		}
	}
	else
	{

		RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
		if(pACLCtrl->OUI_Enable)
		{
			DlListForEachSafe(pOUIEntry, pOUIEntryTmp, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
			{
				DlListDel(&pOUIEntry->List);
				pACLCtrl->OUI_ListNum --;
				if(pACLCtrl->OUI_ListNum == 0)
				{
					pACLCtrl->OUI_Enable = 0;
				}
				os_free_mem(NULL, pOUIEntry);
			}
		}

		DlListInit(&pACLCtrl->OUI_List);		
		RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
	}

	i=0;
	
	//dump all OUI list
	RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
	DlListForEach(pOUIEntry, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: OUI [%d]=%02x:%02x:%02x\n", __FUNCTION__,i++, pOUIEntry->OUI[0],pOUIEntry->OUI[1],pOUIEntry->OUI[2]));

	}
	RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
	if(pACLCtrl->BlockTime)
	{
		RTMPSetTimer(&pACLCtrl->AgeOutTimer, pACLCtrl->BlockTime * 1000);
		pACLCtrl->AgeOutTimer_Running = TRUE;
	}
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: OUI count=%d\n", __FUNCTION__,i));	

	return TRUE;
}

UINT32 Set_ACL_V2_AddSTAEntry_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pTargetMACAddr)
{
	BOOLEAN find_list = FALSE;
	PACL_V2_CTRL pACLCtrl;
	ACL_MAC_CTRL *pMACEntry;
	ULONG Now32;
	INT32 Ret;
	BOOLEAN Cancelled;
	
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;

	if (pACLCtrl->MAC_ListNum >= (MAX_NUM_OF_ACL_V2_MAC - 1))
    	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : AccessControlList is full, and no more entry can join the list!\n",__FUNCTION__));
		return FALSE;
	}
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if(pACLCtrl->AgeOutTimer_Running == TRUE)
	{
		RTMPCancelTimer(&pACLCtrl->AgeOutTimer, &Cancelled);
		pACLCtrl->AgeOutTimer_Running = FALSE;
	}
	RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);
	DlListForEach(pMACEntry, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
	{
		if (MAC_ADDR_EQUAL(pMACEntry->MACAddr, pTargetMACAddr))
		{
			find_list = TRUE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s : duplicate mac =%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,PRINT_MAC(pTargetMACAddr)));			
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);

	if (find_list == FALSE)
		os_alloc_mem(NULL, (UCHAR **)&pMACEntry, sizeof(*pMACEntry));
	
	if (!pMACEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	if (find_list == FALSE) 
	{
		RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);
		
		NdisMoveMemory(pMACEntry->MACAddr, pTargetMACAddr, 6);
		NdisGetSystemUpTime(&Now32);
		pMACEntry->SetTime = Now32;
		
		DlListAddTail(&pACLCtrl->MAC_List, &pMACEntry->List);

		pACLCtrl->MAC_ListNum ++;

		if(pACLCtrl->MAC_ListNum)
		{
			pACLCtrl->MAC_Enable = 1;
		}
			
		RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);
	}
	else
	{
		NdisGetSystemUpTime(&Now32);
		pMACEntry->SetTime = Now32;
	}
	if(pACLCtrl->BlockTime)
	{
		RTMPSetTimer(&pACLCtrl->AgeOutTimer, pACLCtrl->BlockTime * 1000);
		pACLCtrl->AgeOutTimer_Running = TRUE;
	}

	return TRUE;
}


INT	Show_ACL_V2_OUI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT	i=0;
	INT32 Ret;	
	PACL_V2_CTRL pACLCtrl;
	ACL_OUI_CTRL *pOUIEntry;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	
	//dump all OUI list
	RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
	DlListForEach(pOUIEntry, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: OUI [%d]=%02x:%02x:%02x\n", __FUNCTION__,i++, pOUIEntry->OUI[0],pOUIEntry->OUI[1],pOUIEntry->OUI[2]));
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: OUI count=%d\n", __FUNCTION__,i));	

	return TRUE;
}


INT	Show_ACL_V2_STAEntry_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	INT	i=0;
	INT32 Ret;	
	PACL_V2_CTRL pACLCtrl;
	ACL_MAC_CTRL *pMACEntry;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	
	//dump all OUI list
	RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);
	DlListForEach(pMACEntry, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC [%d]=%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,i++, PRINT_MAC(pMACEntry->MACAddr)));
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);
	
	DBGPRINT(RT_DEBUG_ERROR, ("%s:: MAC count=%d\n", __FUNCTION__,i));	

	return TRUE;
}

VOID Set_ACL_V2_DelSTAEntry_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pTargetMACAddr)
{
	PACL_V2_CTRL pACLCtrl;
	ACL_MAC_CTRL *pMACEntry, *pMACEntryTmp;
	
	INT32 Ret;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);
	
	DlListForEachSafe(pMACEntry, pMACEntryTmp, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
	{
		if (!pMACEntry)
			break;
			
		if (MAC_ADDR_EQUAL(pMACEntry->MACAddr, pTargetMACAddr))
		{
			DlListDel(&pMACEntry->List);

			pACLCtrl->MAC_ListNum --;

			if(pACLCtrl->MAC_ListNum == 0)
			{
				pACLCtrl->MAC_Enable = 0;
			}
			
			os_free_mem(NULL, pMACEntry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);
}


UINT32 Set_ACL_V2_AddOUIEntry_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pTargetOUIAddr)
{
	BOOLEAN find_list = FALSE;
	PACL_V2_CTRL pACLCtrl;
	ACL_OUI_CTRL *pOUIEntry;
	ULONG Now32;
	INT32 Ret;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;

	if (pACLCtrl->OUI_ListNum >= (MAX_NUM_OF_ACL_V2_OUI - 1))
    	{
		DBGPRINT(RT_DEBUG_WARN, ("%s : AccessControlList is full, and no more entry can join the list!\n",__FUNCTION__));
		return FALSE;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
	DlListForEach(pOUIEntry, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
	{
		if (RTMPEqualMemory(pOUIEntry->OUI, pTargetOUIAddr, OUI_LEN))
		{
			find_list = TRUE;
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);

	if (find_list == FALSE)
		os_alloc_mem(NULL, (UCHAR **)&pOUIEntry, sizeof(*pOUIEntry));
	
	if (!pOUIEntry)
	{
		DBGPRINT(RT_DEBUG_ERROR, ("%s Not available memory\n", __FUNCTION__));
		return FALSE;
	}

	
	if (find_list == FALSE) 
	{
		RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
		
		NdisMoveMemory(pOUIEntry->OUI, pTargetOUIAddr, OUI_LEN);
		NdisGetSystemUpTime(&Now32);
		pOUIEntry->SetTime = Now32;
		
		DlListAddTail(&pACLCtrl->OUI_List, &pOUIEntry->List);

		pACLCtrl->OUI_ListNum ++;
		if (pACLCtrl->OUI_ListNum)
		{
			pACLCtrl->OUI_Enable = 1;
		}
		RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
	}
	else
	{ // only update timestamp
		NdisGetSystemUpTime(&Now32);
		pOUIEntry->SetTime = Now32;
	}

	return TRUE;
}

VOID Set_ACL_V2_DelOUIEntry_Proc(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pTargetOUIAddr)
{
	PACL_V2_CTRL pACLCtrl;
	ACL_OUI_CTRL *pOUIEntry, *pOUIEntryTmp;
	
	INT32 Ret;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
	
	RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
	
	DlListForEachSafe(pOUIEntry, pOUIEntryTmp, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
	{
		if (!pOUIEntry)
			break;
			
		if (RTMPEqualMemory(pOUIEntry->OUI, pTargetOUIAddr, OUI_LEN))
		{
			DlListDel(&pOUIEntry->List);

			pACLCtrl->OUI_ListNum --;
			if(pACLCtrl->OUI_ListNum == 0)
			{
				pACLCtrl->OUI_Enable = 0;
			}
			os_free_mem(NULL, pOUIEntry);
			break;
		}
	}
	RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);
}

VOID Set_ACL_V2_EntryExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PACL_V2_CTRL pACLCtrl;
	ULONG Value;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;
	
	Value = (ULONG) simple_strtol(arg, 0, 10);
	pACLCtrl->BlockTime = Value;

	DBGPRINT(RT_DEBUG_ERROR, ("%s : pACLCtrl->BlockTime=%lu\n", __FUNCTION__,pACLCtrl->BlockTime));
	
}


VOID Show_ACL_V2_EntryExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	RTMP_STRING *arg)
{
	PACL_V2_CTRL pACLCtrl;
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;
	pACLCtrl = &pAd->ApCfg.MBSSID[pObj->ioctl_if].AccessControlList_V2;

	DBGPRINT(RT_DEBUG_ERROR, ("%s : pACLCtrl->BlockTime=%lu\n", __FUNCTION__,pACLCtrl->BlockTime));
	
}

BOOLEAN ACL_V2_List_Check(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *pAddr,
	IN UCHAR Apidx)
{
    BOOLEAN Result = TRUE;
    PACL_V2_CTRL pACLCtrl;
    ACL_MAC_CTRL *pMACEntry;
    ACL_OUI_CTRL *pOUIEntry;
	
    INT32 Ret;

    pACLCtrl = &pAd->ApCfg.MBSSID[Apidx].AccessControlList_V2;


check_oui:
    RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
    DlListForEach(pOUIEntry, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
    {
	       if (RTMPEqualMemory(pOUIEntry->OUI, pAddr, OUI_LEN))
		{
			Result = FALSE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s : hit  OUI =%02x:%02x:%02x\n", __FUNCTION__,pOUIEntry->OUI[0],pOUIEntry->OUI[1],pOUIEntry->OUI[2]));			
			break;
		}
    }
    RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);


if (Result == FALSE)
    goto done;


check_mac:
    RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);
    DlListForEach(pMACEntry, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
    {
	       if (MAC_ADDR_EQUAL(pMACEntry->MACAddr, pAddr))
		{
			Result = FALSE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s : hit  mac =%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,PRINT_MAC(pAddr)));			
			break;
		}
    }
    RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);
	
done:
    return Result;
}


VOID ACL_V2_AgeOut(
IN	PACL_V2_CTRL	pACLCtrl)
{
	ACL_MAC_CTRL *pMACEntry, *pMACEntryTmp;
	ACL_OUI_CTRL *pOUIEntry, *pOUIEntryTmp;

	INT32 Ret;
	UINT32 Now32;
	INT i = 0;
	
#ifdef CONFIG_AP_SUPPORT
    	UCHAR Apidx;

   	NdisGetSystemUpTime(&Now32);
	{
		RTMP_SEM_LOCK(&pACLCtrl->OUI_ListLock);
		if(pACLCtrl->OUI_Enable)
		{
			DlListForEachSafe(pOUIEntry, pOUIEntryTmp, &pACLCtrl->OUI_List, ACL_OUI_CTRL, List)
			{
				if(RTMP_TIME_AFTER(Now32, pOUIEntry->SetTime + (pACLCtrl->BlockTime * OS_HZ)))
				{
					DlListDel(&pOUIEntry->List);

					DBGPRINT(RT_DEBUG_ERROR, ("%s : age OUI[%d] =%02x:%02x:%02x\n", __FUNCTION__, i, pOUIEntry->OUI[0], pOUIEntry->OUI[1], pOUIEntry->OUI[2]));
					i++;
					pACLCtrl->OUI_ListNum --;
					if(pACLCtrl->OUI_ListNum == 0)
					{
						pACLCtrl->OUI_Enable = 0;
					}
					os_free_mem(NULL, pOUIEntry);
				}
			}

			DlListInit(&pACLCtrl->OUI_List);

		}
		RTMP_SEM_UNLOCK(&pACLCtrl->OUI_ListLock);


		i=0;

		RTMP_SEM_LOCK(&pACLCtrl->MAC_ListLock);

		if(pACLCtrl->MAC_Enable)
		{
			DlListForEachSafe(pMACEntry, pMACEntryTmp, &pACLCtrl->MAC_List, ACL_MAC_CTRL, List)
			{
				if(RTMP_TIME_AFTER(Now32, pMACEntry->SetTime + (pACLCtrl->BlockTime * OS_HZ)))
				{
					DlListDel(&pMACEntry->List);

					DBGPRINT(RT_DEBUG_ERROR, ("%s : age MAC[%d] =%02x:%02x:%02x,%02x:%02x:%02x\n", __FUNCTION__, i, PRINT_MAC(pMACEntry->MACAddr)));
					i++;
					
					pACLCtrl->MAC_ListNum --;
					if(pACLCtrl->MAC_ListNum == 0)
					{
						pACLCtrl->MAC_Enable = 0;
					}
					os_free_mem(NULL, pMACEntry);
				}
			}

			DlListInit(&pACLCtrl->MAC_List);			
		}
		RTMP_SEM_UNLOCK(&pACLCtrl->MAC_ListLock);

		}
#endif 

}

void ACL_V2_Timeout(
    IN PVOID SystemSpecific1, 
    IN PVOID FunctionContext, 
    IN PVOID SystemSpecific2, 
    IN PVOID SystemSpecific3)
{
	PACL_V2_CTRL pACLCtrl = (ACL_V2_CTRL *)FunctionContext;
	PRTMP_ADAPTER  pAd  = pACLCtrl->pAd;

	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));
	
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS))
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: fRTMP_ADAPTER_HALT_IN_PROGRESS\n", __FUNCTION__));
		return;
	}
	
	ACL_V2_AgeOut(pACLCtrl);

}

#endif /* ACL_V2_SUPPORT */
