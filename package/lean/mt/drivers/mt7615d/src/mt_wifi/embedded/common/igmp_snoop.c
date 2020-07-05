#ifdef IGMP_SNOOP_SUPPORT

#include "rt_config.h"
#include "ipv6.h"
#include "igmp_snoop.h"

#ifdef IGMP_TVM_SUPPORT
extern UCHAR IGMP_TVM_OUI[];
#endif /* IGMP_TVM_SUPPORT */

UINT16 IPv6MulticastFilterExclued[] = {
	IPV6_NEXT_HEADER_ICMPV6,	/* ICMPv6. */
	IPV6_NEXT_HEADER_PIM,		/* PIM. */
};
#define IPV6_MULTICAST_FILTER_EXCLUED_SIZE  \
	(sizeof(IPv6MulticastFilterExclued) / sizeof(UINT16))

static inline void initFreeEntryList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	int i;

	for (i = 0; i < FREE_MEMBER_POOL_SIZE; i++)
		insertTailList(pList, (RT_LIST_ENTRY *)&(pMulticastFilterTable->freeMemberPool[i]));

	return;
}

static inline PMEMBER_ENTRY AllocaGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable)
{
	PMEMBER_ENTRY pMemberEntry;
	RTMP_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	pMemberEntry = (PMEMBER_ENTRY)removeHeadList(&pMulticastFilterTable->freeEntryList);
	RTMP_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	return (PMEMBER_ENTRY)pMemberEntry;
}

static inline VOID FreeGrpMemberEntry(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PMEMBER_ENTRY pEntry)
{
	RTMP_SEM_LOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
	insertTailList(&pMulticastFilterTable->freeEntryList, (RT_LIST_ENTRY *)pEntry);
	RTMP_SEM_UNLOCK(&pMulticastFilterTable->FreeMemberPoolTabLock);
}

static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd);

static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr);

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr,
	IN MulticastFilterEntryType type);

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList);

#ifdef A4_CONN
/* Whether member is present on MWDS link */
BOOLEAN isMemberOnMWDSLink(VOID *prMemberEntry);
#endif


/*
    ==========================================================================
    Description:
	This routine init the entire IGMP table.
    ==========================================================================
 */
VOID MulticastFilterTableInit(
	IN PRTMP_ADAPTER pAd,
	IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable)
{
#ifdef IGMP_TVM_SUPPORT
	UCHAR i = 0;
#endif /* IGMP_TVM_SUPPORT*/
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		return;

	/* Initialize MAC table and allocate spin lock */
	os_alloc_mem(NULL, (UCHAR **)ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));

	if (*ppMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s unable to alloc memory for Multicase filter table, size=%lu\n",
				 __func__, (ULONG)sizeof(MULTICAST_FILTER_TABLE)));
		return;
	}

	NdisZeroMemory(*ppMulticastFilterTable, sizeof(MULTICAST_FILTER_TABLE));
	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->MulticastFilterTabLock));
	NdisAllocateSpinLock(pAd, &((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	initList(&((*ppMulticastFilterTable)->freeEntryList));
	initFreeEntryList(*ppMulticastFilterTable, &((*ppMulticastFilterTable)->freeEntryList));

#ifdef IGMP_TVM_SUPPORT
	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		(*ppMulticastFilterTable)->Content[i].AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
	}
#endif /* IGMP_TVM_SUPPORT */
	return;
}

/*
    ==========================================================================
    Description:
	This routine reset the entire IGMP table.
    ==========================================================================
 */
VOID MultiCastFilterTableReset(RTMP_ADAPTER *pAd,
							   IN PMULTICAST_FILTER_TABLE * ppMulticastFilterTable)
{
	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD)) {
		return;
	}

	if (*ppMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __func__));
		return;
	}

	NdisFreeSpinLock(&((*ppMulticastFilterTable)->FreeMemberPoolTabLock));
	NdisFreeSpinLock(&((*ppMulticastFilterTable)->MulticastFilterTabLock));
	os_free_mem(*ppMulticastFilterTable);
	*ppMulticastFilterTable = NULL;
}

/*
    ==========================================================================
    Description:
	Display all entrys in IGMP table
    ==========================================================================
 */
static VOID IGMPTableDisplay(
	IN PRTMP_ADAPTER pAd)
{
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("%s Multicase filter table is not ready.\n", __func__));
		return;
	}

	/* if FULL, return */
	if (pMulticastFilterTable->Size == 0) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("Table empty.\n"));
		return;
	}

	/* allocate one MAC entry */
	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		/* pick up the first available vacancy */
		if (pMulticastFilterTable->Content[i].Valid == TRUE) {
			PMEMBER_ENTRY pMemberEntry = NULL;
			pEntry = &pMulticastFilterTable->Content[i];
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("IF(%s) entry #%d, type=%s, GrpId=(%02x:%02x:%02x:%02x:%02x:%02x) memberCnt=%d\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pEntry->net_dev), i, (pEntry->type == 0 ? "static" : "dynamic"),
					 PRINT_MAC(pEntry->Addr), IgmpMemberCnt(&pEntry->MemberList)));
			pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

			while (pMemberEntry) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("member mac=(%02x:%02x:%02x:%02x:%02x:%02x)\n",
						 PRINT_MAC(pMemberEntry->Addr)));
				pMemberEntry = pMemberEntry->pNext;
			}
		}
	}

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return;
}

/*
    ==========================================================================
    Description:
	Add and new entry into MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableInsertEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN UINT8 type,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex)
{
	UCHAR HashIdx;
	int i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL, *pCurrEntry, *pPrevEntry;
	PMEMBER_ENTRY pMemberEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
#ifdef IGMP_TVM_SUPPORT
	UINT32 AgeOutTime = IGMPMAC_TB_ENTRY_AGEOUT_TIME;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
#endif /* IGMP_TVM_SUPPORT */

	if (pMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __func__));
		return FALSE;
	}

	/* if FULL, return */
	if (pMulticastFilterTable->Size >= MAX_LEN_OF_MULTICAST_FILTER_TABLE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table full. max-entries = %d\n",
				 __func__, MAX_LEN_OF_MULTICAST_FILTER_TABLE));
		return FALSE;
	}

#ifdef IGMP_TVM_SUPPORT
	pMacEntry = &pAd->MacTab.Content[WlanIndex];
	if (pMacEntry && pMacEntry->wdev) {
		AgeOutTime = pMacEntry->wdev->u4AgeOutTime;
	}
#endif /* IGMP_TVM_SUPPORT */

	/* check the rule is in table already or not. */
	pEntry = MulticastFilterTableLookup(pMulticastFilterTable, pGrpId, dev);
	if (pEntry) {
		/* doesn't indicate member mac address. */
		if (pMemberAddr == NULL)
			return FALSE;

		pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

		while (pMemberEntry) {
			if (MAC_ADDR_EQUAL(pMemberAddr, pMemberEntry->Addr)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s: already in Members list.\n", __func__));
				return FALSE;
			}

			pMemberEntry = pMemberEntry->pNext;
		}
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	do {
		ULONG Now;

		/* the multicast entry already exist but doesn't include the member yet. */
		if (pEntry != NULL && pMemberAddr != NULL) {
#ifdef IGMP_TVM_SUPPORT
			pEntry->AgeOutTime = AgeOutTime;
#endif /* IGMP_TVM_SUPPORT */
			InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr, type);
			break;
		}

		/* allocate one MAC entry */
		for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
			/* pick up the first available vacancy */
			pEntry = &pMulticastFilterTable->Content[i];
			NdisGetSystemUpTime(&Now);

			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime +
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime
#else
				IGMPMAC_TB_ENTRY_AGEOUT_TIME
#endif /* IGMP_TVM_SUPPORT */
				)) {
				PMULTICAST_FILTER_TABLE_ENTRY pHashEntry;
				HashIdx = MULTICAST_ADDR_HASH_INDEX(pEntry->Addr);
				pHashEntry = pMulticastFilterTable->Hash[HashIdx];

				if ((pEntry->net_dev == pHashEntry->net_dev)
					&& MAC_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr)) {
					pMulticastFilterTable->Hash[HashIdx] = pHashEntry->pNext;
					pMulticastFilterTable->Size--;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size));
				} else {
					while (pHashEntry->pNext) {
						pPrevEntry = pHashEntry;
						pHashEntry = pHashEntry->pNext;

						if ((pEntry->net_dev == pHashEntry->net_dev)
							&& MAC_ADDR_EQUAL(pEntry->Addr, pHashEntry->Addr)) {
							pPrevEntry->pNext = pHashEntry->pNext;
							pMulticastFilterTable->Size--;
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
							break;
						}
					}
				}

				pEntry->Valid = FALSE;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
			}

			if (pEntry->Valid == FALSE) {
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pEntry->Valid = TRUE;
				COPY_MAC_ADDR(pEntry->Addr, pGrpId);
				pEntry->net_dev = dev;
				NdisGetSystemUpTime(&Now);
				pEntry->lastTime = Now;
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime = AgeOutTime;
#endif /* IGMP_TVM_SUPPORT */
				pEntry->type = (MulticastFilterEntryType)(((UINT8)type) & GROUP_ENTRY_TYPE_BITMASK); /* remove member detail*/
				initList(&pEntry->MemberList);

				if (pMemberAddr != NULL)
					InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr, type);

				pMulticastFilterTable->Size++;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MulticastFilterTableInsertEntry -IF(%s) allocate entry #%d, Total= %d\n", RTMP_OS_NETDEV_GET_DEVNAME(dev), i, pMulticastFilterTable->Size));
				break;
			}
		}

		/* add this MAC entry into HASH table */
		if (pEntry) {
			HashIdx = MULTICAST_ADDR_HASH_INDEX(pGrpId);

			if (pMulticastFilterTable->Hash[HashIdx] == NULL)
				pMulticastFilterTable->Hash[HashIdx] = pEntry;
			else {
				pCurrEntry = pMulticastFilterTable->Hash[HashIdx];

				while (pCurrEntry->pNext != NULL)
					pCurrEntry = pCurrEntry->pNext;

				pCurrEntry->pNext = pEntry;
			}
		}
	} while (FALSE);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return TRUE;
}


/*
    ==========================================================================
    Description:
	Delete a specified client from MAC table
    ==========================================================================
 */
BOOLEAN MulticastFilterTableDeleteEntry(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pGrpId,
	UINT8 BssIdx,
	IN PUCHAR pMemberAddr,
	IN PNET_DEV dev,
	IN UINT8 WlanIndex)
{
	USHORT HashIdx;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry, *pPrevEntry;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (pMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __func__));
		return FALSE;
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	do {
		HashIdx = MULTICAST_ADDR_HASH_INDEX(pGrpId);
		pPrevEntry = pEntry = pMulticastFilterTable->Hash[HashIdx];

		while (pEntry && pEntry->Valid) {
			if ((pEntry->net_dev ==  dev)
				&& MAC_ADDR_EQUAL(pEntry->Addr, pGrpId))
				break;
			else {
				pPrevEntry = pEntry;
				pEntry = pEntry->pNext;
			}
		}

		/* check the rule is in table already or not. */
		if (pEntry && (pMemberAddr != NULL)) {
			DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);

			if (IgmpMemberCnt(&pEntry->MemberList) > 0)
				break;
		}

		if (pEntry) {
			if (pEntry == pMulticastFilterTable->Hash[HashIdx]) {
				pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size--;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 1 - Total= %d\n", pMulticastFilterTable->Size));
			} else {
				pPrevEntry->pNext = pEntry->pNext;
				DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
				NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
				pMulticastFilterTable->Size--;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
			}
		} else
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: the Group doesn't exist.\n", __func__));
	} while (FALSE);

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return TRUE;
}

/*
    ==========================================================================
    Description:
	Look up the MAC address in the IGMP table. Return NULL if not found.
    Return:
	pEntry - pointer to the MAC entry; NULL is not found
    ==========================================================================
*/
PMULTICAST_FILTER_TABLE_ENTRY MulticastFilterTableLookup(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PUCHAR pAddr,
	IN PNET_DEV dev)
{
	ULONG HashIdx, Now;
	PMULTICAST_FILTER_TABLE_ENTRY pEntry = NULL, pPrev = NULL;

	if (pMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __func__));
		return NULL;
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	HashIdx = MULTICAST_ADDR_HASH_INDEX(pAddr);
	pEntry = pPrev = pMulticastFilterTable->Hash[HashIdx];

	while (pEntry && pEntry->Valid) {
		if ((pEntry->net_dev ==  dev)
			&& MAC_ADDR_EQUAL(pEntry->Addr, pAddr)) {
			NdisGetSystemUpTime(&Now);
			pEntry->lastTime = Now;
			break;
		} else {
			NdisGetSystemUpTime(&Now);

			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime +
#ifdef IGMP_TVM_SUPPORT
				pEntry->AgeOutTime
#else
				IGMPMAC_TB_ENTRY_AGEOUT_TIME
#endif /* IGMP_TVM_SUPPORT */
				)) {
				/* Remove the aged entry */
				if (pEntry == pMulticastFilterTable->Hash[HashIdx]) {
					pMulticastFilterTable->Hash[HashIdx] = pEntry->pNext;
					pPrev = pMulticastFilterTable->Hash[HashIdx];
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size--;
					pEntry = pPrev;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
				} else {
					pPrev->pNext = pEntry->pNext;
					DeleteIgmpMemberList(pMulticastFilterTable, &pEntry->MemberList);
					NdisZeroMemory(pEntry, sizeof(MULTICAST_FILTER_TABLE_ENTRY));
					pMulticastFilterTable->Size--;
					pEntry = pPrev->pNext;
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MCastFilterTableDeleteEntry 2 - Total= %d\n", pMulticastFilterTable->Size));
				}
			} else {
				pPrev = pEntry;
				pEntry = pEntry->pNext;
			}
		}
	}

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
	return pEntry;
}

#ifdef IGMP_TVM_SUPPORT
INT IgmpSnEnableTVMode(IN RTMP_ADAPTER * pAd, struct wifi_dev *wdev, UINT8 IsTVModeEnable, UINT8 TVModeType)
{
	INT Result = FALSE;
#ifdef APCLI_SUPPORT
	struct wifi_dev *ApcliWdev = NULL;
#endif /* APCLI_SUPPORT */

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s: Before: Enable = %u, IsTVModeEnable = %u/%u, TVModeType = %u/%u\n", __func__,
		wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, IsTVModeEnable, wdev->TVModeType, TVModeType));

	do {
		if (wdev->IgmpSnoopEnable == FALSE) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("%s Igmp Disabled\n", __func__));
			IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
			TVModeType = IGMP_TVM_MODE_DISABLE;
		}

		if (IsTVModeEnable > IGMP_TVM_SWITCH_ENABLE) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("Invalid Value: Please input, %u to DISABLE or %u to ENABLE IGMP Enhanced mode\n",
				IGMP_TVM_SWITCH_DISABLE, IGMP_TVM_SWITCH_ENABLE));
			break;
		}

		if (IS_IGMP_TVM_MODE_EN(IsTVModeEnable)) {

			if (TVModeType > IGMP_TVM_MODE_AUTO) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					("Invalid Value: Please input, %u to DISABLE or %u to ENABLE or %u to AUTO mode\n",
					IGMP_TVM_MODE_DISABLE, IGMP_TVM_MODE_ENABLE, IGMP_TVM_MODE_AUTO));
				break;
			}

			wdev->IsTVModeEnable = TRUE;

			/* Add few exceptional list which is hardcoded, for the first time */
			if ((wdev->TVModeType == IGMP_TVM_MODE_DISABLE) &&
				(TVModeType != IGMP_TVM_MODE_DISABLE)) {
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-224.0.0.1/24-1");
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-239.255.255.250/0-1");
				Set_IgmpSn_BlackList_Proc(pAd, (RTMP_STRING *)"1-ff02:0:0:0:0:0:0:fb/0-1");
			}

			wdev->TVModeType = TVModeType;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
				("%s:: wdev[%d],OMAC[%d], TVMode : %s\n", __FUNCTION__, wdev->wdev_idx,
				wdev->DevInfo.OwnMacIdx,
				((TVModeType == IGMP_TVM_MODE_AUTO) ? "AUTO" :
				((TVModeType == IGMP_TVM_MODE_ENABLE) ? "ENABLE" :
				((TVModeType == IGMP_TVM_MODE_DISABLE) ? "DISABLE" : "Invalid Value, Not Changed")))
				));

			Result = TRUE;
		} else {
			wdev->IsTVModeEnable = FALSE;
			wdev->TVModeType = IGMP_TVM_MODE_DISABLE;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
				("%s:: wdev[%d],OMAC[%d]-%s\n", __FUNCTION__, wdev->wdev_idx,
				wdev->DevInfo.OwnMacIdx,
				IsTVModeEnable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping"));
		}

		UpdateBeaconHandler(pAd, wdev, BCN_UPDATE_IE_CHG);

#ifdef APCLI_SUPPORT
		if ((wdev->func_idx < MAX_APCLI_NUM) &&
			pAd->ApCfg.ApCliTab[wdev->func_idx].ApCliInit) {
		ApcliWdev = &pAd->ApCfg.ApCliTab[wdev->func_idx].wdev;
		if (ApcliWdev)
			ApcliWdev->IsTVModeEnable = wdev->IsTVModeEnable;
		}
#endif /* APCLI_SUPPORT */
	} while (FALSE);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s: Enable = %u, IsTVModeEnable = %u, TVModeType = %u\n", __func__,
		wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, wdev->TVModeType));

	return Result;
}

VOID ConvertUnicastMacToMulticast(IN RTMP_ADAPTER * pAd, IN struct wifi_dev *wdev, IN RX_BLK * pRxBlk)
{
	UCHAR *Header802_3 = GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket);
	UINT16 protoType = OS_NTOHS(*((UINT16 *)(GET_OS_PKT_DATAPTR(pRxBlk->pRxPacket) + 12)));
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;

	if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("%s: TV Mode not enabled\n", __func__));
		return;
	}

	if (protoType == ETH_P_IP) {
		UINT32 GroupIpAddr;

		pGroupIpAddr = Header802_3 + LENGTH_802_3 + 16;
		GroupIpAddr = OS_NTOHL(*((UINT32 *)pGroupIpAddr));

		if (IS_MULTICAST_IP(GroupIpAddr)) {
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			COPY_MAC_ADDR(Header802_3, GroupMacAddr);
		}
	} else if (protoType == ETH_P_IPV6) {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(Header802_3 + LENGTH_802_3);

		pGroupIpAddr = pIpv6Hdr->dstAddr.ipv6_addr;

		if (IS_MULTICAST_IPV6_ADDR(pIpv6Hdr->dstAddr)) {
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			COPY_MAC_ADDR(Header802_3, GroupMacAddr);
		}

	}
}

VOID MakeTVMIE(
	IN RTMP_ADAPTER * pAd,
	IN struct wifi_dev *wdev,
	IN OUT PUCHAR pOutBuffer,
	IN OUT PULONG pFrameLen)
{
	if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		struct _nec_tvm_ie tvm_ie = {0};
		ULONG TVIeLen = 0;

		tvm_ie.eid = IE_VENDOR_SPECIFIC;
		tvm_ie.len = IGMP_TVM_IE_LENGTH;
		NdisMoveMemory(tvm_ie.oui_oitype, IGMP_TVM_OUI, 4);
		tvm_ie.version1 = IGMP_TVM_IE_VERSION_1;
		tvm_ie.version2 = IGMP_TVM_IE_VERSION_2;
		tvm_ie.data.field.rsvd = 0;

		if (wdev->wdev_type == WDEV_TYPE_AP) {
			if ((wdev->TVModeType == IGMP_TVM_MODE_AUTO)
				|| (wdev->TVModeType == IGMP_TVM_MODE_DISABLE))
				tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_AUTO;
			else if (wdev->TVModeType == IGMP_TVM_MODE_ENABLE)
				tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_ENABLE;
		} else if (wdev->wdev_type == WDEV_TYPE_APCLI) {
			tvm_ie.data.field.TVMode = IGMP_TVM_IE_MODE_AUTO;
		}

		MakeOutgoingFrame(pOutBuffer + *pFrameLen,	&TVIeLen,
							IGMP_TVM_IE_LENGTH+2,	(PUCHAR)&tvm_ie,
							END_OF_ARGS);

		*pFrameLen += TVIeLen;
	}
}

INT Set_IgmpSn_BlackList_Proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg)
{
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	UCHAR *pOperationType = NULL; /* 1=Add or 0=Del */
	UCHAR *pIP = NULL; /* IPv4 or IPv6 */
	UCHAR *pPrefix = NULL; /* IPv4 or IPv6 */
	UCHAR Prefix = 0;
	UCHAR *pIsStatic = NULL; /* Static or Dynamic */
	UCHAR IsStatic = FALSE;
	UCHAR *value = 0;
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	MULTICAST_BLACK_LIST_ENTRY ThisMcastEntry = {0};
	BOOLEAN bPrintCmdUsages = FALSE;
	INT Result = FALSE;
	BOOLEAN bAdd = FALSE; /* bAdd == FALSE means Delete operation */
	UCHAR i = 0;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
	RTMP_STRING IPString[100] = {'\0'};
	RTMP_STRING *pIPString = NULL;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev)
		pMcastBLTable = &wdev->McastBLTable;

	do {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() wdev not initialized\n", __func__));
			break;
		}

		if (wdev->IgmpSnoopEnable == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
				("IGMP Snooping is disabled\n"));
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s: TV Mode not enabled\n", __func__));
			break;
		}

		if (pMcastBLTable == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() Mcast Black List Not init, skip this operation\n", __func__));
			break;
		}

		pIPString = IPString;
		NdisMoveMemory(pIPString, arg, strlen(arg));
		pIPString[strlen(arg)] = '\0';

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
			("%s() arg = [%s]\n", __func__, pIPString));

		/* Parse, if user supplied, Operation/IP/Prefix-Len */
		pOperationType = strsep((char **)&pIPString, "-");
		if (pIPString && (*pIPString != '\0')) {
			pIP = strsep((char **)&pIPString, "/");
			if (pIPString && (*pIPString != '\0')) {
				pPrefix = strsep((char **)&pIPString, "-");
				if (pPrefix) {
					Prefix = (UCHAR)os_str_tol(pPrefix, NULL, 10);
					if (pIPString && (*pIPString != '\0')) {
						pIsStatic = (PUCHAR)pIPString;
						IsStatic = ((UCHAR)os_str_tol(pIsStatic, NULL, 10) == 0) ? 0 : 1;
					}
				}
			}
		}

		/* Check if incorrect or no operation entered */
		if ((pOperationType == NULL) || ((*pOperationType != '0') && (*pOperationType != '1'))) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() Incorrect Operation Type entered, skip this operation\n", __func__));
			bPrintCmdUsages = TRUE;
			break;
		}

		bAdd = (BOOLEAN)(((UCHAR)os_str_tol(pOperationType, NULL, 10) == 0) ? FALSE : TRUE);

		/* Check if add operation but IP not entered */
		if ((bAdd == TRUE) && (pIP == NULL)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() Mcast IP address not entered, skip this operation\n", __func__));
			bPrintCmdUsages = TRUE;
			break;
		}

		if ((bAdd == TRUE) && (pMcastBLTable->EntryNum >= MULTICAST_BLACK_LIST_SIZE_MAX)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() Mcast List Already full, skip this operation\n", __func__));
			bPrintCmdUsages = TRUE;
			break;
		}

		if ((bAdd == FALSE) && (pIP == NULL)) {
			PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() Delete full list\n", __func__));
			for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
				pEntryTab = &pMcastBLTable->EntryTab[i];
				if ((pEntryTab->bValid == TRUE) && (pEntryTab->bStatic == FALSE)) {
					NdisZeroMemory((PUCHAR)pEntryTab,
							sizeof(MULTICAST_BLACK_LIST_ENTRY));
					pEntryTab->bValid = FALSE;
					pMcastBLTable->EntryNum -= 1;
				}
			}
			Result = TRUE;
			break;
		}


		/* IPv4 address */
		if ((strlen(pIP) >= 9) && (strlen(pIP) <= 15)) {
			for (i = 0, value = rstrtok(pIP, "."); value; value = rstrtok(NULL, "."), i++) {
				UCHAR ii = 0;
				if (strlen(value) > 3) {
					bPrintCmdUsages = TRUE;
					break;
				}
				for (ii = 0; ii < strlen(value); ii++) {
					if (!isdigit(*(value + ii))) {
						bPrintCmdUsages = TRUE;
						break;
					}
				}
				if (bPrintCmdUsages == TRUE)
					break;

				ThisMcastEntry.IPData.IPv4[i] = (UCHAR)os_str_tol(value, NULL, 10);
			}
			/* Check if any invalid multicast address specified */
			if ((bPrintCmdUsages == TRUE) ||
				(ThisMcastEntry.IPData.IPv4[0] < 224) ||
				(ThisMcastEntry.IPData.IPv4[0] > 239)) {
				bPrintCmdUsages = TRUE;
				break;
			}
			ThisMcastEntry.bValid = TRUE;
			ThisMcastEntry.EntryIPType = IP_V4;
			if (Prefix == 0)
				Prefix = 32; /* Use all 32 bits (IPv4 has 32 bits) to mask, in case user spcified 0 Prefix length */

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("Blacklist %sING: IPv4 addr = %d.%d.%d.%d%s%s\n",
					 ((bAdd == TRUE) ? "ADD":"DELETE"),
					 ThisMcastEntry.IPData.IPv4[0], ThisMcastEntry.IPData.IPv4[1], ThisMcastEntry.IPData.IPv4[2],
					 ThisMcastEntry.IPData.IPv4[3],
					 (PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)"")));
		} else if ((strlen(pIP) >= 15) && (strlen(pIP) <= 39)) { /* IPv6 address */
			for (i = 0, value = rstrtok(pIP, ":"); value; value = rstrtok(NULL, ":"), i++) {
				UCHAR ii = 0;
				UCHAR pIPVal[4] = {'0'};

				NdisFillMemory(pIPVal, 4, '0');

				if (strlen(value) > sizeof(UINT32)) {
					bPrintCmdUsages = TRUE;
					break;
				}
				for (ii = 0; ii < strlen(value); ii++) {
					if (!isxdigit(*(value + ii))) {
						bPrintCmdUsages = TRUE;
						break;
					}
				}
				if (bPrintCmdUsages == TRUE)
					break;
				NdisMoveMemory((PUCHAR)&pIPVal[sizeof(UINT32)-strlen(value)],
								(PUCHAR)value, min((UINT)strlen(value),
								(UINT)sizeof(UINT32)));
				AtoH(pIPVal, &ThisMcastEntry.IPData.IPv6[i*2], 4);
			}
			/* Check if any invalid multicast address specified */
			if ((bPrintCmdUsages == TRUE) ||
				(ThisMcastEntry.IPData.IPv6[0] != 0xFF)) {
				bPrintCmdUsages = TRUE;
				break;
			}
			ThisMcastEntry.bValid = TRUE;
			ThisMcastEntry.EntryIPType = IP_V6;
			if (Prefix == 0)
				Prefix = 128; /* Use all 128 bits (IPv6 has  128 bits) to mask, in case user spcified 0 Prefix length */

			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("Blacklist %sING: IPv6 addr = "
				"%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X%s%s\n",
					 ((bAdd == TRUE) ? "ADD":"DELETE"),
					 ThisMcastEntry.IPData.IPv6[0], ThisMcastEntry.IPData.IPv6[1], ThisMcastEntry.IPData.IPv6[2],
					 ThisMcastEntry.IPData.IPv6[3], ThisMcastEntry.IPData.IPv6[4], ThisMcastEntry.IPData.IPv6[5],
					 ThisMcastEntry.IPData.IPv6[6], ThisMcastEntry.IPData.IPv6[7], ThisMcastEntry.IPData.IPv6[8],
					 ThisMcastEntry.IPData.IPv6[9], ThisMcastEntry.IPData.IPv6[10], ThisMcastEntry.IPData.IPv6[11],
					 ThisMcastEntry.IPData.IPv6[12], ThisMcastEntry.IPData.IPv6[13], ThisMcastEntry.IPData.IPv6[14],
					 ThisMcastEntry.IPData.IPv6[15],
					 (PUCHAR)((pPrefix) ? (PUCHAR)"/" : (PUCHAR)""), (PUCHAR)((pPrefix) ? pPrefix : (PUCHAR)"")));

		}

		if (ThisMcastEntry.bValid == TRUE) {
			/* Store prefix length */
			ThisMcastEntry.PrefixLen = Prefix;
			ThisMcastEntry.bStatic = (BOOLEAN)((IsStatic == 0) ? FALSE : TRUE);
			if (bAdd == TRUE) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
				/* FIrst check if there is any existing entry */
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					pEntryTab = &pMcastBLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))
						|| ((ThisMcastEntry.EntryIPType == IP_V6) && (pEntryTab->EntryIPType == IP_V6) &&
						NdisEqualMemory(pEntryTab->IPData.IPv6, ThisMcastEntry.IPData.IPv6, IPV6_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						Result = TRUE;
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
							("This IP address already present in Igmp Snooping Exemption table\n\n"));
						break;
					}
				}
				/* IP entry already present, no need to add again, so exit */
				if (Result == TRUE)
					break;

				/* Find the empty entry in black list entry to add new entry */
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					UCHAR Index = 0;
					UCHAR Bits = 0;
					UINT32 Mask = 0;
					if (pMcastBLTable->EntryTab[i].bValid == FALSE) {
						NdisZeroMemory(&pMcastBLTable->EntryTab[i],
										sizeof(MULTICAST_BLACK_LIST_ENTRY));
						NdisMoveMemory(&pMcastBLTable->EntryTab[i],
										&ThisMcastEntry,
										sizeof(MULTICAST_BLACK_LIST_ENTRY));
						pMcastBLTable->EntryNum += 1;

						if (pMcastBLTable->EntryTab[i].EntryIPType == IP_V4)
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv4, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
						else
							ConvertMulticastIP2MAC(ThisMcastEntry.IPData.IPv6, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);

						AsicMcastEntryDelete(pAd,
											GroupMacAddr,
											wdev->bss_info_argument.ucBssIndex,
											NULL,
											wdev->if_dev,
											0);

						/* Prepare Mask of bytes from Prefix Length to be matched with IP address of entery and packets received */
						Index = 0;
						do {
							/* here 32 = 32 bits in a DWord */
							Bits = ((Prefix%32)?(Prefix%32):32);
							if (Bits == 32)
								Mask = ((UINT32)~0);
							else
								Mask = (UINT32)~(((UINT32)~0)<<(UINT32)Bits);
							pMcastBLTable->EntryTab[i].PrefixMask.DWord[Index] = Mask;
							Prefix = Prefix - ((Prefix%32)?(Prefix%32):32);
							Index += 1;
						} while (Prefix > 0);
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
							("This IP address added in Igmp Snooping Exemption table\n\n"));
						break;
					}
				}
			} else {
				/* Find the this entry in black list entry to delete it from list */
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = NULL;
				Result = TRUE;
				for (i = 0; i < MULTICAST_BLACK_LIST_SIZE_MAX; i++) {
					pEntryTab = &pMcastBLTable->EntryTab[i];
					if ((pEntryTab->bValid == TRUE) && (pEntryTab->bStatic == FALSE) &&
						(((ThisMcastEntry.EntryIPType == IP_V4) && (pEntryTab->EntryIPType == IP_V4) &&
						NdisEqualMemory(pEntryTab->IPData.IPv4, ThisMcastEntry.IPData.IPv4, IPV4_ADDR_LEN))
						|| ((ThisMcastEntry.EntryIPType == IP_V6) && (pEntryTab->EntryIPType == IP_V6) &&
						NdisEqualMemory(pEntryTab->IPData.IPv6, ThisMcastEntry.IPData.IPv6, IPV6_ADDR_LEN))) &&
						(pEntryTab->PrefixLen == ThisMcastEntry.PrefixLen)) {
						NdisZeroMemory((PUCHAR)pEntryTab,
								sizeof(MULTICAST_BLACK_LIST_ENTRY));
						pEntryTab->bValid = FALSE;
						pMcastBLTable->EntryNum -= 1;
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
							("This IP address deleted from Igmp Snooping Exemption table\n\n"));
						break;
					}
				}
			}
			Result = TRUE;
		}
	} while (FALSE);

	if (bPrintCmdUsages == TRUE) {
		/* CR4 FW already has 224.0.0.x and FF02:0:0:0:0:0:0:FB exempted */
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
			("\nCommand usages:\n"
			"	iwpriv ra0 set IgmpSnExemptIP=<Operation>-<IP Addr>/<PrefixLength>\n"
			"		<Operation>     :	1 for ADD and 0 for DELETE\n"
			"		<IP Addr>       :	IPv4 or IPv6 address to exempt from being snooped\n"
			"		<PrefixLength>  :	Prefix Length, Value between 1 to 32 or 128 for IPv4 or IPv6\n"
			"\n"
			"		IPv4 address format :\n"
			"			Example = 225.0.0.0\n"
			"		IPv6 address format = No short notation.Only expanded format is accepted, with or without Prefix\n"
			"			Example = FF:0:0:0:0:0:0:0 or FF00:0000:0000:0000:0000:0000:0000:0000\n"
			"\n"
			"	Example : iwpriv ra0 set IgmpSnExemptIP=1-FF01:0:0:0:0:0:0:1\n"
			"		or  : iwpriv ra0 set IgmpSnExemptIP=1-FF01:0:0:0:0:0:0:1/60\n"
			"		or  : iwpriv ra0 set IgmpSnExemptIP=0-225.1.2.3/16\n"
			"		or  : iwpriv ra0 set IgmpSnExemptIP=0-225.1.2.3\n\n"));

	}

	return Result;
}

INT Show_IgmpSn_BlackList_Proc(IN RTMP_ADAPTER * pAd, IN RTMP_STRING * arg)
{
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	UCHAR idx = 0;
	POS_COOKIE pObj;
	struct wifi_dev *wdev = NULL;
	UCHAR Count = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif /* CONFIG_AP_SUPPORT */
	if (wdev)
		pMcastBLTable = &wdev->McastBLTable;

	do {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s() wdev not initialized\n", __func__));
			break;
		}

		if (wdev->IgmpSnoopEnable == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
				("IGMP Snooping is disabled\n"));
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s: TV Mode not enabled\n", __func__));
			break;
		}

		if (pMcastBLTable->EntryNum == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
				("IGMP/MLD snooping exemption list Empty\n"));
			break;
		}

		Count = 0;
		/* First dump IPv4 address */
		for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
			if ((pMcastBLTable->EntryTab[idx].bValid) &&
				(pMcastBLTable->EntryTab[idx].EntryIPType == IP_V4)) {
				Count += 1;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
					("IPv4 addr to Exempt snooping:[%u] = %d.%d.%d.%d", Count,
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[0],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[1],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[2],
						 pMcastBLTable->EntryTab[idx].IPData.IPv4[3]));
				if (pMcastBLTable->EntryTab[idx].PrefixLen < 32) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
						("/%u : %s\n", pMcastBLTable->EntryTab[idx].PrefixLen,
						((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC")));
				} else {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
						(" : %s\n", ((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC")));
				}
			}
		}

		Count = 0;
		/* Second dump IPv6 address */
		for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
			if ((pMcastBLTable->EntryTab[idx].bValid) &&
				(pMcastBLTable->EntryTab[idx].EntryIPType == IP_V6)) {
				Count += 1;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
					("IPv6 addr to Exempt snooping:[%u] = "
					"%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X", Count,
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[0],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[1],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[2],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[3],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[4],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[5],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[6],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[7],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[8],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[9],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[10],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[11],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[12],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[13],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[14],
						 pMcastBLTable->EntryTab[idx].IPData.IPv6[15]));
				if (pMcastBLTable->EntryTab[idx].PrefixLen < 128) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
						("/%u : %s\n", pMcastBLTable->EntryTab[idx].PrefixLen,
						((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC")));
				} else {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
						(" : %s\n", ((pMcastBLTable->EntryTab[idx].bStatic == 0) ? "DYNAMIC":"STATIC")));
				}
			}
		}

	} while (FALSE);

	return TRUE;
}

BOOLEAN isIgmpMldExemptPkt(
	IN PRTMP_ADAPTER pAd,
	IN struct wifi_dev *wdev,
	IN PUCHAR pGroupIpAddr,
	IN UINT16 ProtoType)
{
	BOOLEAN bExempt = FALSE;
	PMULTICAST_BLACK_LIST_FILTER_TABLE pMcastBLTable = NULL;
	UCHAR idx = 0;
	UCHAR MaskIdx = 0;
	UCHAR Mask = 0;

	do {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("%s: Invalid wdev pointer\n", __func__));
			break;
		}

		if (!IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("%s: TV Mode not enabled\n", __func__));
			break;
		}

		pMcastBLTable = &wdev->McastBLTable;
		if (pMcastBLTable->EntryNum == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("%s: Exemption list Empty\n", __func__));
			break;
		}

		if (ProtoType == ETH_P_IP) {
			for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = &pMcastBLTable->EntryTab[idx];
				if (pEntryTab->bValid && (pEntryTab->EntryIPType == IP_V4)) {
					for (MaskIdx = 0; MaskIdx < IPV4_ADDR_LEN; MaskIdx++) {
						Mask = pEntryTab->PrefixMask.Byte[MaskIdx];
						if (Mask > 0) {
							bExempt = TRUE;
							if ((pEntryTab->IPData.IPv4[MaskIdx] & Mask) != (pGroupIpAddr[MaskIdx] & Mask)) {
								bExempt = FALSE;
								break;
							}
						}
					}
					if (bExempt == TRUE) {
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
							("Exempt from snooping: IPv4 addr (%d.%d.%d.%d)\n",
								 pEntryTab->IPData.IPv4[0],
								 pEntryTab->IPData.IPv4[1],
								 pEntryTab->IPData.IPv4[2],
								 pEntryTab->IPData.IPv4[3]));
					}

				}
			}
		} else if (ProtoType == ETH_P_IPV6) {
			for (idx = 0; idx < MULTICAST_BLACK_LIST_SIZE_MAX; idx++) {
				PMULTICAST_BLACK_LIST_ENTRY pEntryTab = &pMcastBLTable->EntryTab[idx];
				if (pEntryTab->bValid && (pEntryTab->EntryIPType == IP_V6)) {

					for (MaskIdx = 0; MaskIdx < IPV6_ADDR_LEN; MaskIdx++) {
						Mask = pEntryTab->PrefixMask.Byte[MaskIdx];
						if (Mask > 0) {
							bExempt = TRUE;
							if ((pEntryTab->IPData.IPv4[MaskIdx] & Mask) != (pGroupIpAddr[MaskIdx] & Mask)) {
								bExempt = FALSE;
								break;
							}
						}
					}
					if (bExempt == TRUE) {
						MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
							("Exempt from snooping: IPv6 addr "
								"(%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X)\n",
									 pEntryTab->IPData.IPv6[0],
									 pEntryTab->IPData.IPv6[1],
									 pEntryTab->IPData.IPv6[2],
									 pEntryTab->IPData.IPv6[3],
									 pEntryTab->IPData.IPv6[4],
									 pEntryTab->IPData.IPv6[5],
									 pEntryTab->IPData.IPv6[6],
									 pEntryTab->IPData.IPv6[7],
									 pEntryTab->IPData.IPv6[8],
									 pEntryTab->IPData.IPv6[9],
									 pEntryTab->IPData.IPv6[10],
									 pEntryTab->IPData.IPv6[11],
									 pEntryTab->IPData.IPv6[12],
									 pEntryTab->IPData.IPv6[13],
									 pEntryTab->IPData.IPv6[14],
									 pEntryTab->IPData.IPv6[15]));
					}
				}
			}
		}
	} while (FALSE);

	return bExempt;
}


INT Set_IgmpSn_AgeOut_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;
	UINT32 AgeOut = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	AgeOut = (UINT32) simple_strtol(arg, 0, 10);

	if (wdev) {
		wdev->u4AgeOutTime = AgeOut;
		if ((wdev->IgmpSnoopEnable) && (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable))) {
			AsicMcastConfigAgeOut(pAd, AgeOut, wdev->DevInfo.OwnMacIdx);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("IgmpSnooping or TV Mode is disabled\n"));
		}
	}
	return TRUE;
}

INT Show_IgmpSn_McastTable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev = NULL;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

#ifdef CONFIG_AP_SUPPORT
	wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	if (wdev) {
		if ((wdev->IgmpSnoopEnable) && (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)))
			AsicMcastGetMcastTable(pAd, wdev->DevInfo.OwnMacIdx, wdev);
		else
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("IgmpSnooping or TV Mode is disabled\n"));
		}

	return TRUE;
}

BOOLEAN IgmpSnoopingGetMulticastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, P_IGMP_MULTICAST_TABLE pEvtMcastTable)
{
	P_IGMP_MULTICAST_TABLE_MEMBER pEvtMcastMember = NULL, pDrvMcastMember = NULL;
	P_IGMP_MULTICAST_TABLE_ENTRY pEvtMcastTableEntry = NULL, pDrvMcastTableEntry = NULL;
	UINT_32 GroupIdx = 0, MemberIdx = 0;
	struct wifi_dev *wdev = NULL;
	BOOLEAN bNeedToShowTable = FALSE;

	do {

		if (ucOwnMacIdx >= HW_BSSID_MAX) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("Invalid MacIndex = %u\n", ucOwnMacIdx));
			return bNeedToShowTable;
		}

		wdev = wdev_search_by_omac_idx(pAd, ucOwnMacIdx);
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("Invalid wdev\n"));
			return bNeedToShowTable;
		}

		if ((wdev->IgmpTableSize == 0) || (wdev->pIgmpMcastTable == NULL)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("IgmpMcastTable was not allocated\n"));
			return bNeedToShowTable;
		}

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("Event sequence = %u\n", pEvtMcastTable->EvtSeqNum));

		/* This allocation will happen for the first time, and for rest event attempt, only entries will be added */
		if (pEvtMcastTable->EvtSeqNum == 1) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("First Event\n"));
			NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
			wdev->pIgmpMcastTable->pNxtFreeGroupLocation = &wdev->pIgmpMcastTable->IgmpMcastTableEntry[0];
			wdev->pIgmpMcastTable->TotalGroup = pEvtMcastTable->TotalGroup;
			wdev->pIgmpMcastTable->TotalSize = pEvtMcastTable->TotalSize;
		}

		pDrvMcastTableEntry = wdev->pIgmpMcastTable->pNxtFreeGroupLocation;

		GroupIdx = 0;
		pEvtMcastTableEntry = &pEvtMcastTable->IgmpMcastTableEntry[GroupIdx];

		for (;
			GroupIdx < min((UINT_32)pEvtMcastTable->NumOfGroup, (UINT_32)MAX_LEN_OF_MULTICAST_FILTER_TABLE);
			GroupIdx++) {


			/* Check if the size do not exceed and overwrite the next buffer which is outside the size of this table */
			if ((((ULONG)pDrvMcastTableEntry - (ULONG)wdev->pIgmpMcastTable)
				+ pDrvMcastTableEntry->ThisGroupSize) > (ULONG)wdev->IgmpTableSize) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
					("%s: Potential buffer overflow, skip write operation to prevent buffer overwrite\n", __func__));
				break;
			}

			/* hex_dump_with_lvl("Group", (UCHAR*)pEvtMcastTableEntry, 200, DBG_LVL_OFF); */

			NdisMoveMemory(pDrvMcastTableEntry, pEvtMcastTableEntry, sizeof(IGMP_MULTICAST_TABLE_ENTRY));

			for (MemberIdx = 0;
				MemberIdx < min((UINT_32)pEvtMcastTableEntry->NumOfMember, (UINT_32)FREE_MEMBER_POOL_SIZE);
				MemberIdx++) {
				pDrvMcastMember = &pDrvMcastTableEntry->IgmpMcastMember[MemberIdx];
				pEvtMcastMember = &pEvtMcastTableEntry->IgmpMcastMember[MemberIdx];

				COPY_MAC_ADDR(pDrvMcastMember->Addr, pEvtMcastMember->Addr);
				pDrvMcastMember->TVMode = pEvtMcastMember->TVMode;
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
													("Member[%u] = %02X:%02X:%02X:%02X:%02X:%02X\n",
													MemberIdx, PRINT_MAC(pDrvMcastMember->Addr)));
			}

			wdev->pIgmpMcastTable->NumOfGroup += 1;

			pDrvMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
				((PUCHAR)(pDrvMcastTableEntry) + pDrvMcastTableEntry->ThisGroupSize);
			pEvtMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
					((PUCHAR)(pEvtMcastTableEntry) + pEvtMcastTableEntry->ThisGroupSize);

		}

		wdev->pIgmpMcastTable->ThisTableSize += pEvtMcastTable->ThisTableSize;
		/* Reinitialize the next free location, which points to the end of last member in last group */
		wdev->pIgmpMcastTable->pNxtFreeGroupLocation = pDrvMcastTableEntry;

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO,
			("Local No of Group = %u, No Of Group = %u, Total Group = %u, ThisTableSize = %u\n",
			wdev->pIgmpMcastTable->NumOfGroup, pEvtMcastTable->NumOfGroup, pEvtMcastTable->TotalGroup,
			pEvtMcastTable->ThisTableSize));
	} while (FALSE);

	return bNeedToShowTable;
}


VOID IgmpSnoopingShowMulticastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev)
{
	P_IGMP_MULTICAST_TABLE_MEMBER pDrvMcastMember = NULL;
	P_IGMP_MULTICAST_TABLE_ENTRY pDrvMcastTableEntry = NULL;
	UINT_32 GroupIdx = 0, MemberIdx = 0;

	do {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("Invalid wdev\n"));
			return;
		}

		if ((wdev->IgmpTableSize == 0) || (wdev->pIgmpMcastTable == NULL)) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("IgmpMcastTable was not allocated\n"));
			return;
		}

		if (wdev->pIgmpMcastTable->NumOfGroup == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("IgmpMcastTable empty\n"));
			return;
		}

		/* hex_dump_with_lvl("Local Group", (UCHAR*)&wdev->pIgmpMcastTable->IgmpMcastTableEntry[0], 200, DBG_LVL_OFF); */

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("*************************************************"
								"*************************************************\n"));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("	S.No.		   GROUP ID                "
								"MEMBER 			   "
								"TVM				"
								"AGEOUT(Sec)\n"));
		GroupIdx = 0;
		pDrvMcastTableEntry = &wdev->pIgmpMcastTable->IgmpMcastTableEntry[GroupIdx];
		for (;
			GroupIdx < min((UINT_32)wdev->pIgmpMcastTable->NumOfGroup, (UINT_32)MAX_LEN_OF_MULTICAST_FILTER_TABLE);
			GroupIdx++) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
						("	%-2u	      %02X:%02X:%02X:%02X:%02X:%02X				  "
								"				 "
								"				 "
								"    %5u\n",
								(GroupIdx+1), PRINT_MAC(pDrvMcastTableEntry->GroupAddr),
													pDrvMcastTableEntry->AgeOut));
			for (MemberIdx = 0;
				MemberIdx < min((UINT_32)pDrvMcastTableEntry->NumOfMember, (UINT_32)FREE_MEMBER_POOL_SIZE);
				MemberIdx++) {
				pDrvMcastMember = &pDrvMcastTableEntry->IgmpMcastMember[MemberIdx];
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("	%3u.%-2u                            "
					"%02X:%02X:%02X:%02X:%02X:%02X		 "
					"%s\n",
					(GroupIdx+1), (MemberIdx+1),
					PRINT_MAC(pDrvMcastMember->Addr),
					((pDrvMcastMember->TVMode == 0) ? "AUTO":((pDrvMcastMember->TVMode == 1) ? "ENABLE":"NO TVM IE"))));
			}
			/* The next group table will start from the end of last member of last group */
			pDrvMcastTableEntry =
				(P_IGMP_MULTICAST_TABLE_ENTRY)
				((PUCHAR)(pDrvMcastTableEntry) + pDrvMcastTableEntry->ThisGroupSize);
		}
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("-------------------------------------------------"
								"-------------------------------------------------\n"));
		NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
	} while (FALSE);
}

BOOLEAN MulticastFilterConfigAgeOut(RTMP_ADAPTER *pAd, UINT8 AgeOutTime, UINT8 ucOwnMacIdx)
{
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;
	UINT i = 0;

	if (pMulticastFilterTable == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s Multicase filter table is not ready.\n", __func__));
		return FALSE;
	}

	RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		pMulticastFilterTable->Content[i].AgeOutTime = (AgeOutTime * OS_HZ);
	}

	RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

	return TRUE;
}

BOOLEAN MulticastFilterInitMcastTable(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, BOOLEAN bActive)
{
	if (bActive == TRUE) {
		wdev->IgmpTableSize = sizeof(IGMP_MULTICAST_TABLE)
			+ ((sizeof(IGMP_MULTICAST_TABLE_ENTRY) * MAX_LEN_OF_MULTICAST_FILTER_TABLE)
			- sizeof(IGMP_MULTICAST_TABLE_ENTRY))
			+ ((sizeof(IGMP_MULTICAST_TABLE_MEMBER) * FREE_MEMBER_POOL_SIZE)
			- sizeof(IGMP_MULTICAST_TABLE_MEMBER));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
			("%s: Allocate IGMP Multicast Memory size = %u\n", __func__, wdev->IgmpTableSize));

		wdev->pIgmpMcastTable =
			(P_IGMP_MULTICAST_TABLE)kmalloc(wdev->IgmpTableSize, 0);
		if (wdev->pIgmpMcastTable == NULL) {
			wdev->IgmpTableSize = 0;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
				("%s: Failed to allocate IGMP Multicast Memory\n", __func__));
		} else {
			NdisZeroMemory((UCHAR *)wdev->pIgmpMcastTable, wdev->IgmpTableSize);
		}
	} else {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN,
			("%s: Deallocate IGMP Multicast Memory\n", __func__));
		kfree(wdev->pIgmpMcastTable);
		wdev->pIgmpMcastTable = NULL;
		wdev->IgmpTableSize = 0;
	}

	return TRUE;
}

BOOLEAN MulticastFilterGetMcastTable(RTMP_ADAPTER *pAd, UINT8 ucOwnMacIdx, struct wifi_dev *wdev)
{
	UINT_32 GroupIdx = 0, MemberIdx = 0;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	do {
		if (wdev == NULL) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_WARN, ("Invalid wdev\n"));
			return FALSE;
		}

		/* if empty, return */
		if (pMulticastFilterTable->Size == 0) {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("Table empty.\n"));
			return FALSE;
		}

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("*************************************************"
								"*************************************************\n"));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("	S.No.		   GROUP ID                "
								"MEMBER 			   "
								"TVM				"
								"AGEOUT(Sec)\n"));
		/* allocate one MAC entry */
		RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);

		for (GroupIdx = 0; GroupIdx < MAX_LEN_OF_MULTICAST_FILTER_TABLE; GroupIdx++) {
			/* pick up the valid entry */
			if (pMulticastFilterTable->Content[GroupIdx].Valid == TRUE) {
				PMEMBER_ENTRY pMemberEntry = NULL;
				pEntry = &pMulticastFilterTable->Content[GroupIdx];

				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
							("	%-2u		  %02X:%02X:%02X:%02X:%02X:%02X 			  "
									"				 "
									"				 "
									"	 %5u\n",
									(GroupIdx+1), PRINT_MAC(pEntry->Addr),
														(pEntry->AgeOutTime / OS_HZ)));

				pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

				MemberIdx = 0;

				while (pMemberEntry) {
					MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("	%3u.%-2u							"
						"%02X:%02X:%02X:%02X:%02X:%02X		 "
						"%s\n",
						(GroupIdx+1), (MemberIdx+1),
						PRINT_MAC(pMemberEntry->Addr),
						((pMemberEntry->TVMode == 0) ? "AUTO":((pMemberEntry->TVMode == 1) ? "ENABLE":"NO TVM IE"))));

					pMemberEntry = pMemberEntry->pNext;
					MemberIdx += 1;
				}
			}
		}
		RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF, ("-------------------------------------------------"
								"-------------------------------------------------\n"));

	} while (FALSE);

	return TRUE;
}

#endif /* IGMP_TVM_SUPPORT */

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,
	UINT8 Wcid
)
{
	INT i;
	INT IpHeaderLen;
	UCHAR GroupType;
	UINT16 numOfGroup;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVModeType = 0;
#endif /* IGMP_TVM_SUPPORT */

	if (isIgmpPkt(pDstMacAddr, pIpHeader)) {
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));
#ifdef A4_CONN
				if (pEntry && (wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))
					Type |= MCAT_FILTER_MWDS_CLI;	/* set info about message on MWDS link*/
#endif

		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP type=%0x\n", IgmpVerType));

#ifdef IGMP_TVM_SUPPORT
		pEntry = &pAd->MacTab.Content[Wcid];

		if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
			if (pEntry->TVMode == IGMP_TVM_IE_MODE_AUTO)
				TVModeType = MCAT_FILTER_TVM_AUTO;
			else if (pEntry->TVMode == IGMP_TVM_IE_MODE_ENABLE)
				TVModeType = MCAT_FILTER_TVM_ENABLE;
		}
#endif /* IGMP_TVM_SUPPORT */

		switch (IgmpVerType) {
		case IGMP_V1_MEMBERSHIP_REPORT: /* IGMP version 1 membership report. */
		case IGMP_V2_MEMBERSHIP_REPORT: /* IGMP version 2 membership report. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
#ifdef IGMP_TVM_SUPPORT
			if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
					("Exempt Snooping IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
				break;
			}
#endif /* IGMP_TVM_SUPPORT */
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryInsert IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
			AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
				(Type
#ifdef IGMP_TVM_SUPPORT
				|TVModeType
#endif /* IGMP_TVM_SUPPORT */
				), pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_LEAVE_GROUP: /* IGMP version 1 and version 2 leave group. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryDelete IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
#ifdef A4_CONN
			if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
				AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_V3_MEMBERSHIP_REPORT: /* IGMP version 3 membership report. */
			numOfGroup = ntohs(*((UINT16 *)(pIgmpHeader + 6)));
			pGroup = (PUCHAR)(pIgmpHeader + 8);

			for (i = 0; i < numOfGroup; i++) {
				GroupType = (UCHAR)(*pGroup);
				AuxDataLen = (UCHAR)(*(pGroup + 1));
				numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
				pGroupIpAddr = (PUCHAR)(pGroup + 4);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMPv3 Type=%d, ADL=%d, numOfSource=%d\n",
						 GroupType, AuxDataLen, numOfSources));
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
						 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
						 GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));

				do {
					if ((GroupType == MODE_IS_EXCLUDE)
						|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
						|| (GroupType == ALLOW_NEW_SOURCES)) {
#ifdef IGMP_TVM_SUPPORT
						if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
								("Exempt Snooping IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
								 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
							break;
						}
#endif /* IGMP_TVM_SUPPORT */
						AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
							(Type
#ifdef IGMP_TVM_SUPPORT
							|TVModeType
#endif /* IGMP_TVM_SUPPORT */
							), pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0) {

/* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#ifdef A4_CONN

						if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)))
#endif
							AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
						} else {
#ifdef IGMP_TVM_SUPPORT
							if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IP) == TRUE) {
								MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
									("Exempt Snooping IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
									 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
								break;
							}
#endif /* IGMP_TVM_SUPPORT */
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
								(Type
#ifdef IGMP_TVM_SUPPORT
								|TVModeType
#endif /* IGMP_TVM_SUPPORT */
								), pSrcMacAddr, wdev->if_dev, Wcid);
						}
						break;
					}
				} while (FALSE);

				pGroup += (8 + (numOfSources * 4) + AuxDataLen);
			}

			break;

		default:
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("unknow IGMP Type=%d\n", IgmpVerType));
			break;
		}
	}

	return;
}


#ifdef A4_CONN
/* Indicate if Specific Pkt is an IGMP query message*/
BOOLEAN isIGMPquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	INT IpHeaderLen;
	UCHAR IgmpVerType;
	PUCHAR pIgmpHeader;
	BOOLEAN isIGMPquery = FALSE;

	if (isIgmpPkt(pDstMacAddr, pIpHeader)) {
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));

		switch (IgmpVerType) {
		case IGMP_MEMBERSHIP_QUERY:
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, ("isIGMPquery-> IGMP Type=0x%x IGMP_MEMBERSHIP_QUERY\n", IgmpVerType));
				isIGMPquery = TRUE;
				break;
		}
	}

	return isIGMPquery;
}
#endif


static BOOLEAN isIgmpMacAddr(
	IN PUCHAR pMacAddr)
{
	if ((pMacAddr[0] == 0x01)
		&& (pMacAddr[1] == 0x00)
		&& (pMacAddr[2] == 0x5e))
		return TRUE;

	return FALSE;
}

BOOLEAN isIgmpPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	UCHAR IgmpProtocol;

	if (!isIgmpMacAddr(pDstMacAddr))
		return FALSE;

	if (IpProtocol == ETH_P_IP) {
		IgmpProtocol  = (UCHAR)*(pIpHeader + 11);

		if (IgmpProtocol == IGMP_PROTOCOL_DESCRIPTOR)
			return TRUE;
	}

	return FALSE;
}

static VOID InsertIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr,
	IN MulticastFilterEntryType type)
{
	PMEMBER_ENTRY pMemberEntry;

	if (pList == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __func__));
		return;
	}

	if (pMemberAddr == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: invalid member.\n", __func__));
		return;
	}

	pMemberEntry = (PMEMBER_ENTRY)AllocaGrpMemberEntry(pMulticastFilterTable);
	if (pMemberEntry != NULL) {
		NdisZeroMemory(pMemberEntry, sizeof(MEMBER_ENTRY));
		COPY_MAC_ADDR(pMemberEntry->Addr, pMemberAddr);
#ifdef IGMP_TVM_SUPPORT
		if (type & MCAT_FILTER_TVM_ENABLE)
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_ENABLE;
		else if (type & MCAT_FILTER_TVM_AUTO)
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_AUTO;
		else
			pMemberEntry->TVMode = IGMP_TVM_IE_MODE_DISABLE;
#endif /* IGMP_TVM_SUPPORT */
#ifdef A4_CONN
			/* Extract detail regarding presence on MWDS link*/
			if (type & MCAT_FILTER_MWDS_CLI) {
				pMemberEntry->onMWDSLink = TRUE;
			} else {
				pMemberEntry->onMWDSLink = FALSE;
			}
#endif
		insertTailList(pList, (RT_LIST_ENTRY *)pMemberEntry);
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s Member Mac=%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
				 pMemberEntry->Addr[0], pMemberEntry->Addr[1], pMemberEntry->Addr[2],
				 pMemberEntry->Addr[3], pMemberEntry->Addr[4], pMemberEntry->Addr[5]));
	}

	return;
}

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr)
{
	PMEMBER_ENTRY pCurEntry;

	if (pList == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __func__));
		return;
	}

	if (pList->pHead == NULL)
		return;

	if (pMemberAddr == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: invalid member.\n", __func__));
		return;
	}

	pCurEntry = (PMEMBER_ENTRY)pList->pHead;

	while (pCurEntry) {
		PMEMBER_ENTRY pCurEntryNext = pCurEntry->pNext;

		if (MAC_ADDR_EQUAL(pMemberAddr, pCurEntry->Addr)) {
			delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
			FreeGrpMemberEntry(pMulticastFilterTable, pCurEntry);
			break;
		}

		pCurEntry = pCurEntryNext;
	}

	return;
}

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList)
{
	PMEMBER_ENTRY pCurEntry, pPrvEntry;

	if (pList == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __func__));
		return;
	}

	if (pList->pHead == NULL)
		return;

	pPrvEntry = pCurEntry = (PMEMBER_ENTRY)pList->pHead;

	while (pCurEntry) {
		delEntryList(pList, (RT_LIST_ENTRY *)pCurEntry);
		pPrvEntry = pCurEntry;
		pCurEntry = pCurEntry->pNext;
		FreeGrpMemberEntry(pMulticastFilterTable, pPrvEntry);
	}

	initList(pList);
	return;
}


UCHAR IgmpMemberCnt(
	IN PLIST_HEADER pList)
{
	if (pList == NULL) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: membert list doesn't exist.\n", __func__));
		return 0;
	}

	return getListSize(pList);
}

VOID IgmpGroupDelMembers(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pMemberAddr,
	IN struct wifi_dev *wdev,
	UINT8 Wcid)
{
	INT i;
	MULTICAST_FILTER_TABLE_ENTRY *pEntry = NULL;
	PMULTICAST_FILTER_TABLE pMulticastFilterTable = pAd->pMulticastFilterTable;

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		return;

	for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
		/* pick up the first available vacancy */
		pEntry = &pMulticastFilterTable->Content[i];

		if (pEntry->Valid == TRUE) {
			if (pMemberAddr != NULL) {
				RTMP_SEM_LOCK(&pMulticastFilterTable->MulticastFilterTabLock);
				DeleteIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
				RTMP_SEM_UNLOCK(&pMulticastFilterTable->MulticastFilterTabLock);
			}

			if ((pEntry->type == MCAT_FILTER_DYNAMIC)
				&& (IgmpMemberCnt(&pEntry->MemberList) == 0))
				AsicMcastEntryDelete(pAd, pEntry->Addr, wdev->bss_info_argument.ucBssIndex, pMemberAddr, wdev->if_dev, Wcid);
		}
	}
}

INT Set_IgmpSn_Enable_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	struct wifi_dev *wdev;
	POS_COOKIE pObj;
	UCHAR Enable;
#ifdef IGMP_TVM_SUPPORT
	UINT32 TVMode = IGMP_TVM_MODE_DISABLE;
	RTMP_STRING *pTVMode = NULL;
	UINT32 IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
	RTMP_STRING *pIgmpEnhancedEn = NULL;
#endif /* IGMP_TVM_SUPPORT */
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
#ifdef CONFIG_AP_SUPPORT
	    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	Enable = (UINT) simple_strtol(arg, 0, 10);

	wdev->IgmpSnoopEnable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);

	pAd->ApCfg.IgmpSnoopEnable[wdev->DevInfo.BandIdx] = wdev->IgmpSnoopEnable;

#ifdef IGMP_TVM_SUPPORT

	if (wdev->IgmpSnoopEnable) {
		/* Just remove First Enable Parameter from arg */
		if (strsep(&arg, "-")) {
			pIgmpEnhancedEn = strsep(&arg, "-");
			if (pIgmpEnhancedEn) {
				IsTVModeEnable = os_str_toul(pIgmpEnhancedEn, 0, 10);

				pTVMode = strsep(&arg, "-");
				if (pTVMode)
					TVMode = os_str_toul(pTVMode, 0, 10);
			}
		}
	}
	if (IgmpSnEnableTVMode(pAd, wdev, IsTVModeEnable, TVMode)) {
		Enable = TVMode;
	}

	pAd->ApCfg.IsTVModeEnable[wdev->DevInfo.BandIdx] = wdev->IsTVModeEnable;
	pAd->ApCfg.TVModeType[wdev->DevInfo.BandIdx] = wdev->TVModeType;

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s: IgmpSnoopEnable = %u, IsTVModeEnable = %u, TVModeType = %u\n",
		__func__, wdev->IgmpSnoopEnable, wdev->IsTVModeEnable, wdev->TVModeType));
#else
	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s:: wdev[%d],OMAC[%d]-%s\n", __FUNCTION__, wdev->wdev_idx,
		wdev->DevInfo.OwnMacIdx,
		Enable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping"));
#endif /* IGMP_TVM_SUPPORT */

	if (IS_ASIC_CAP(pAd, fASIC_CAP_MCU_OFFLOAD))
		CmdMcastCloneEnable(pAd, Enable, wdev->DevInfo.OwnMacIdx);

	return TRUE;
}

INT Set_IgmpSn_AddEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4];
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[MAC_ADDR_LEN];
	PUCHAR *pAddr = (PUCHAR *)&Addr;
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	UCHAR mwds_type = 0;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		/* refuse the Member if it's not a MAC address. */
		if ((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if (strlen(thisChar) == 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return FALSE;  /*Invalid */
		} else {
			for (i = 0, value = rstrtok(thisChar, "."); value; value = rstrtok(NULL, ".")) {
				if ((strlen(value) > 0) && (strlen(value) <= 3)) {
					int ii;

					for (ii = 0; ii < strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				} else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);
				i++;
			}

			if (i != 4)
				return FALSE;  /*Invalid */

			ConvertMulticastIP2MAC(IpAddr, (PUCHAR *)&pAddr, ETH_P_IP);
		}

		if (bGroupId == 1)
			COPY_MAC_ADDR(GroupId, Addr);

		if (bGroupId == 0) {
			pEntry = MacTableLookup(pAd, Addr);
		}

#ifdef A4_CONN
		if ((bGroupId == 0) && pEntry && (pAd->ApCfg.MBSSID[ifIndex].wdev.wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry)) {
			mwds_type |= MCAT_FILTER_MWDS_CLI;	/* set info about message on MWDS link*/
		}
#endif


		/* Group-Id must be a MCAST address. */
		if ((bGroupId == 1) && IS_MULTICAST_MAC_ADDR(Addr)) {
			AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, NULL, pDev, 0);
		}
		/* Group-Member must be a UCAST address. */
		else if ((bGroupId == 0) && !IS_MULTICAST_MAC_ADDR(Addr)) {
			if (pEntry)
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC | mwds_type, Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, 0);
		} else {
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X) is not a acceptable address.\n",
					 __func__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));
			return FALSE;
		}

		bGroupId = 0;
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X)\n",
				 __func__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));
	}

	return TRUE;
}

INT Set_IgmpSn_DelEntry_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT i, memberCnt = 0;
	BOOLEAN bGroupId = 1;
	RTMP_STRING *value;
	RTMP_STRING *thisChar;
	UCHAR IpAddr[4];
	UCHAR Addr[MAC_ADDR_LEN];
	UCHAR GroupId[MAC_ADDR_LEN];
	PUCHAR *pAddr = (PUCHAR *)&Addr;
	PNET_DEV pDev;
	POS_COOKIE pObj;
	UCHAR ifIndex;
	MAC_TABLE_ENTRY *pEntry = NULL;
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	ifIndex = pObj->ioctl_if;
	pDev = (ifIndex == MAIN_MBSSID) ? (pAd->net_dev) : (pAd->ApCfg.MBSSID[ifIndex].wdev.if_dev);

	while ((thisChar = strsep((char **)&arg, "-")) != NULL) {
		/* refuse the Member if it's not a MAC address. */
		if ((bGroupId == 0) && (strlen(thisChar) != 17))
			continue;

		if (strlen(thisChar) == 17) { /*Mac address acceptable format 01:02:03:04:05:06 length 17 */
			for (i = 0, value = rstrtok(thisChar, ":"); value; value = rstrtok(NULL, ":")) {
				if ((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value + 1))))
					return FALSE;  /*Invalid */

				AtoH(value, &Addr[i++], 1);
			}

			if (i != 6)
				return FALSE;  /*Invalid */
		} else {
			for (i = 0, value = rstrtok(thisChar, "."); value; value = rstrtok(NULL, ".")) {
				if ((strlen(value) > 0) && (strlen(value) <= 3)) {
					int ii;

					for (ii = 0; ii < strlen(value); ii++)
						if (!isxdigit(*(value + ii)))
							return FALSE;
				} else
					return FALSE;  /*Invalid */

				IpAddr[i] = (UCHAR)os_str_tol(value, NULL, 10);
				i++;
			}

			if (i != 4)
				return FALSE;  /*Invalid */

			ConvertMulticastIP2MAC(IpAddr, (PUCHAR *)&pAddr, ETH_P_IP);
		}

		pEntry = MacTableLookup(pAd, Addr);

		if (bGroupId == 1) {
			COPY_MAC_ADDR(GroupId, Addr);
		} else {
			pEntry = MacTableLookup(pAd, Addr);
			memberCnt++;
		}

		if (memberCnt > 0) {
			if (pEntry)
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, 
							Addr, pDev, pEntry->wcid);
			else
				AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, 
							Addr, pDev, 0);
		}

		bGroupId = 0;
	}

	if (memberCnt == 0)
		AsicMcastEntryDelete(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, NULL, pDev, 0);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("%s (%2X:%2X:%2X:%2X:%2X:%2X)\n",
			 __func__, Addr[0], Addr[1], Addr[2], Addr[3], Addr[4], Addr[5]));
	return TRUE;
}

INT Set_IgmpSn_TabDisplay_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	IGMPTableDisplay(pAd);
	return TRUE;
}

void rtmp_read_igmp_snoop_from_file(
	IN  PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	INT		i;
	INT		band_idx;
	RTMP_STRING	*macptr = NULL;
#ifdef IGMP_TVM_SUPPORT
	UINT32 TVMode = IGMP_TVM_MODE_DISABLE;
	RTMP_STRING *pTVMode = NULL;
	UINT32 IsTVModeEnable = IGMP_TVM_SWITCH_DISABLE;
	RTMP_STRING *pIgmpEnhancedEn = NULL;
#endif /* IGMP_TVM_SUPPORT */

	/*IgmpSnEnable */
	if (RTMPGetKeyParameter("IgmpSnEnable", tmpbuf, 128, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr ; macptr = rstrtok(NULL, ";"), i++) {
			if (i < DBDC_BAND_NUM) {
				band_idx = rtmp_band_index_get_by_order(pAd, i);
				pAd->ApCfg.IgmpSnoopEnable[band_idx] = simple_strtol(macptr, 0, 10);
#ifdef IGMP_TVM_SUPPORT
			/* Just remove First Enable Parameter from arg */
			if (strsep(&macptr, "-")) {
				pIgmpEnhancedEn = strsep(&macptr, "-");
				if (pIgmpEnhancedEn) {
					IsTVModeEnable = os_str_toul(pIgmpEnhancedEn, 0, 10);

					pTVMode = strsep(&macptr, "-");
					if (pTVMode)
						TVMode = os_str_toul(pTVMode, 0, 10);
				}
			}
			pAd->ApCfg.IsTVModeEnable[band_idx] = IsTVModeEnable;
			pAd->ApCfg.TVModeType[band_idx] = TVMode;
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_OFF,
				("%s: Idx = %u, Enable = %u, IsTVModeEnable = %u, TVModeType = %u\n", __func__,
				band_idx, pAd->ApCfg.IgmpSnoopEnable[band_idx], pAd->ApCfg.IsTVModeEnable[band_idx],
				pAd->ApCfg.TVModeType[band_idx]));
#endif /* IGMP_TVM_SUPPORT */
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, (" Band[%d]-IGMP Snooping Enable=%d\n", band_idx, pAd->ApCfg.IgmpSnoopEnable[band_idx]));
			} else
				break;
		}
	}
}

#ifdef A4_CONN
/* Indicate Whether specified member is present on MWDS link */
BOOLEAN isMemberOnMWDSLink(VOID *prMemberEntry)
{
	PMEMBER_ENTRY pMemberEntry = (PMEMBER_ENTRY) prMemberEntry;

	if (pMemberEntry->onMWDSLink != FALSE)
		return TRUE;
	else
		return FALSE;

}
#endif



#ifdef VENDOR_FEATURE7_SUPPORT
static inline BOOLEAN IsExcludedMldMsg(
	IN UINT8 MsgType)
{
	BOOLEAN result = FALSE;

	switch (MsgType) {
	case ICMPV6_MSG_TYPE_ROUTER_SOLICITATION:
	case ICMPV6_MSG_TYPE_ROUTER_ADVERTISEMENT:
	case ICMPV6_MSG_TYPE_NEIGHBOR_SOLICITATION:
	case ICMPV6_MSG_TYPE_NEIGHBOR_ADVERTISEMENT:
	case ICMPV6_MSG_TYPE_REDIRECT:
		result = TRUE;
		break;
	default:
		result = FALSE;
		break;
	}
	return result;
}
#endif

BOOLEAN ismDNS(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT16 IpProtocol = ntohs(*((UINT16 *) (pIpHeader)));
	UCHAR IpUDP;


	if (IpProtocol == ETH_P_IP) {
		IpUDP = (UCHAR)*(pIpHeader + 11);
		if (IpUDP == 0x11) {
			/* check the ip address : 224.0.0.x  reserved for mDNS & well known Protocol*/
			if ((((UCHAR)*(pIpHeader + (11+7)) == 0xE0)
				&& ((UCHAR)*(pIpHeader + (11+8)) == 0x00)
				&& ((UCHAR)*(pIpHeader + (11+9)) == 0x00))
#ifdef IGMP_TVM_SUPPORT
				|| (((UCHAR)*(pIpHeader + (11+7)) == 0xEF)
				&& ((UCHAR)*(pIpHeader + (11+8)) == 0xFF)
				&& ((UCHAR)*(pIpHeader + (11+9)) == 0xFF)
				&& ((UCHAR)*(pIpHeader + (11+10)) == 0xFA))
#endif /* IGMP_TVM_SUPPORT */
			)
			return TRUE;
		}
	} else if (IpProtocol == ETH_P_IPV6) {
		IpUDP = (UCHAR)*(pIpHeader + 8);
		if (IpUDP == 0x11) {
				/* check the ipv6 address : ff02::fb  reserved for mDNSv6 */
				if (((UCHAR)*(pIpHeader + (8+18)) == 0xFF)
					&& ((UCHAR)*(pIpHeader + (8+19)) == 0x02)
					&& ((UCHAR)*(pIpHeader + (8+20)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+21)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+22)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+23)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+24)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+25)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+26)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+27)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+28)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+29)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+30)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+31)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+32)) == 0x00)
					&& ((UCHAR)*(pIpHeader + (8+33)) == 0xFB)
				)
				return TRUE;

		}
	}
	return FALSE;
}

/*
 * If Packet is IGMP or MLD type multicast packet, send packet OUT
 * Else check whether multicast destination address matches any group-id
 * in the multicast filter table, if no match, drop the packet, else have two case
 * If the member-list of the matching entry is empty and AP just forwards packet to all stations
 * Else if AP will do the MC-to-UC conversation base one memberships
 */
NDIS_STATUS IgmpPktInfoQuery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pSrcBufVA,
	IN PNDIS_PACKET pPacket,
	IN struct wifi_dev *wdev,
	OUT INT *pInIgmpGroup,
	OUT PMULTICAST_FILTER_TABLE_ENTRY *ppGroupEntry)
{
	if (IS_MULTICAST_MAC_ADDR(pSrcBufVA)) {
		BOOLEAN IgmpMldPkt = FALSE;
		PUCHAR pIpHeader = pSrcBufVA + 12;
#ifdef VENDOR_FEATURE7_SUPPORT
		UINT16 TypeLen;

		TypeLen = (pSrcBufVA[12] << 8) | pSrcBufVA[13];
		if (TypeLen == ETH_TYPE_VLAN)
			pIpHeader = pSrcBufVA + 16;
		if (ntohs(*((UINT16 *)(pIpHeader))) == ETH_P_IPV6) {
			PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader + 2);
			UINT32 offset = IPV6_HDR_LEN;
			UINT8 nextProtocol = pIpv6Hdr->nextHdr;

			if (nextProtocol == IPV6_NEXT_HEADER_ICMPV6) {
				PRT_ICMPV6_HDR pICMPv6Hdr = (PRT_ICMPV6_HDR)(pIpHeader + 2 + offset);

				if (IsExcludedMldMsg(pICMPv6Hdr->type) == TRUE)
					return NDIS_STATUS_SUCCESS;
			}
		}
#endif
		if (ntohs(*((UINT16 *)(pIpHeader))) == ETH_P_IPV6)
			IgmpMldPkt = IPv6MulticastFilterExcluded(pSrcBufVA, pIpHeader);
		else
			IgmpMldPkt = isIgmpPkt(pSrcBufVA, pIpHeader);

		*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pSrcBufVA,
									wdev->if_dev);

		if (ismDNS(pSrcBufVA, pIpHeader)) {
			*pInIgmpGroup = IGMP_NONE;
			return NDIS_STATUS_SUCCESS;
		}

		if (IgmpMldPkt) {
			*ppGroupEntry = NULL;
			*pInIgmpGroup = IGMP_PKT;
		} else if (*ppGroupEntry == NULL) {
#ifdef IGMP_TVM_SUPPORT
			/* This code is for following case: */
			/* CASE: [When the group has been formed first, and then the IgmpSnooping is enabled] */
			/* Earlier it was seen, that for the above case, after Group formation but IgmpSnooping OFF */
			/* Packet used to go as Mcast packet, but after enabling IgmpSnooping, */
			/* we used to drop all the Mcast packet beloning to this group */
			*pInIgmpGroup = IGMP_NONE;
			return NDIS_STATUS_SUCCESS;
#else
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
#endif /* IGMP_TVM_SUPPORT */
		} else
			*pInIgmpGroup = IGMP_IN_GROUP;
	} else if (IS_BROADCAST_MAC_ADDR(pSrcBufVA)) {
		PUCHAR pDstIpAddr = pSrcBufVA + 30; /* point to Destination of Ip address of IP header. */
		UCHAR GroupMacAddr[6];
		PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
		ConvertMulticastIP2MAC(pDstIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);

		*ppGroupEntry = MulticastFilterTableLookup(pAd->pMulticastFilterTable, pGroupMacAddr,
							 wdev->if_dev);
		if (*ppGroupEntry != NULL)
			*pInIgmpGroup = IGMP_IN_GROUP;
	}

	return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS IgmpPktClone(
	PRTMP_ADAPTER pAd,
	struct wifi_dev *wdev,
	PNDIS_PACKET pPacket,
	INT IgmpPktInGroup,
	PMULTICAST_FILTER_TABLE_ENTRY pGroupEntry,
	UCHAR QueIdx,
	UINT8 UserPriority,
	PNET_DEV pNetDev)
{
#ifdef IGMP_TVM_SUPPORT
	NDIS_STATUS nStatus = NDIS_STATUS_SUCCESS;
#endif /* IGMP_TVM_SUPPORT*/
	struct qm_ops *qm_ops = pAd->qm_ops;
	PNDIS_PACKET pSkbClone = NULL;
	PMEMBER_ENTRY pMemberEntry = NULL;
	MAC_TABLE_ENTRY *pMacEntry = NULL;
	STA_TR_ENTRY *tr_entry = NULL;
	USHORT Aid;
	SST Sst = SST_ASSOC;
	UCHAR PsMode = PWR_ACTIVE;
	UCHAR Rate;
	BOOLEAN bContinue;
	PUCHAR pMemberAddr = NULL;
	bContinue = FALSE;

	if ((IgmpPktInGroup == IGMP_IN_GROUP) && (pGroupEntry == NULL))
		return NDIS_STATUS_FAILURE;

	if (IgmpPktInGroup == IGMP_IN_GROUP) {
		pMemberEntry = (PMEMBER_ENTRY)pGroupEntry->MemberList.pHead;

		if (pMemberEntry) {
			pMemberAddr = pMemberEntry->Addr;
			pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
			bContinue = TRUE;
		}
	} else
		return NDIS_STATUS_FAILURE;

	/* check all members of the IGMP group. */
	while (bContinue == TRUE) {

#ifdef IGMP_TVM_SUPPORT
		/* If TV Mode is enabled in AP, then we need to send unicast packet to all connected STA's */
		if (wdev->IsTVModeEnable &&
			((wdev->TVModeType == IGMP_TVM_MODE_DISABLE) ||
			((wdev->TVModeType == IGMP_TVM_MODE_AUTO) &&
			(pMemberEntry->TVMode == IGMP_TVM_IE_MODE_DISABLE)))) {

			nStatus = NDIS_STATUS_MORE_PROCESSING_REQUIRED;

			pMemberEntry = pMemberEntry->pNext;

			if (pMemberEntry != NULL) {
				pMemberAddr = pMemberEntry->Addr;
				pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
				continue;
			} else
				break;
		}
#endif /* IGMP_TVM_SUPPORT */

		if (pMacEntry && (Sst == SST_ASSOC) &&
			(pAd->MacTab.tr_entry[pMacEntry->wcid].PortSecured == WPA_802_1X_PORT_SECURED)) {
			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
			OS_PKT_CLONE(pAd, pPacket, pSkbClone, MEM_ALLOC_FLAG);

			if ((pSkbClone)
			   ) {
				RTMP_SET_PACKET_WCID(pSkbClone, (UCHAR)pMacEntry->wcid);

				/*For MWDS member clone use DA from MPDU*/
#ifdef A4_CONN
				if (!isMemberOnMWDSLink(pMemberEntry))
#endif
					RTMP_SET_PACKET_MCAST_CLONE(pSkbClone, 1);

				RTMP_SET_PACKET_UP(pSkbClone, UserPriority);

				qm_ops->enq_dataq_pkt(pAd, wdev, pSkbClone, QueIdx);

			} else {
				if (IgmpPktInGroup == IGMP_IN_GROUP) {
					pMemberEntry = pMemberEntry->pNext;

					if (pMemberEntry != NULL) {
						pMemberAddr = pMemberEntry->Addr;
						pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
						bContinue = TRUE;
					} else
						bContinue = FALSE;
				} else
					bContinue = FALSE;

				continue;
			}

			ba_ori_session_start(pAd, tr_entry, UserPriority);
		}


		if (IgmpPktInGroup == IGMP_IN_GROUP) {
			pMemberEntry = pMemberEntry->pNext;

			if (pMemberEntry != NULL) {
				pMemberAddr = pMemberEntry->Addr;
				pMacEntry = APSsPsInquiry(pAd, pMemberAddr, &Sst, &Aid, &PsMode, &Rate);
				bContinue = TRUE;
			} else
				bContinue = FALSE;
		} else
			bContinue = FALSE;
	}

#ifdef IGMP_TVM_SUPPORT
	return nStatus;
#else
	return NDIS_STATUS_SUCCESS;
#endif /* IGMP_TVM_SUPPORT */
}

static inline BOOLEAN isMldMacAddr(
	IN PUCHAR pMacAddr)
{
	return ((pMacAddr[0] == 0x33) && (pMacAddr[1] == 0x33)) ? TRUE : FALSE;
}

static inline BOOLEAN IsSupportedMldMsg(
	IN UINT8 MsgType)
{
	BOOLEAN result = FALSE;

	switch (MsgType) {
	case MLD_V1_LISTENER_REPORT:
	case MLD_V1_LISTENER_DONE:
	case MLD_V2_LISTERNER_REPORT:
	case MLD_LISTENER_QUERY:
		result = TRUE;
		break;

	default:
		result = FALSE;
		break;
	}

	return result;
}

BOOLEAN isMldPkt(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader,
	OUT UINT8 *pProtoType,
	OUT PUCHAR *pMldHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));

	if (!isMldMacAddr(pDstMacAddr))
		return FALSE;

	if (IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;

	do {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		UINT8 nextProtocol = pIpv6Hdr->nextHdr;
		UINT32 offset = IPV6_HDR_LEN;

		while (nextProtocol != IPV6_NEXT_HEADER_ICMPV6) {
			if (IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
		}

		if (nextProtocol == IPV6_NEXT_HEADER_ICMPV6) {
			PRT_ICMPV6_HDR pICMPv6Hdr = (PRT_ICMPV6_HDR)(pIpHeader + offset);

			if (IsSupportedMldMsg(pICMPv6Hdr->type) == TRUE) {
				if (pProtoType != NULL)
					*pProtoType = pICMPv6Hdr->type;

				if (pMldHeader != NULL)
					*pMldHeader = (PUCHAR)pICMPv6Hdr;

				result = TRUE;
			}
		}
	} while (FALSE);

	return result;
}

BOOLEAN IPv6MulticastFilterExcluded(
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	BOOLEAN result = FALSE;
	UINT16 IpProtocol = ntohs(*((UINT16 *)(pIpHeader)));
	INT idx;
	UINT8 nextProtocol;

	if (!IS_IPV6_MULTICAST_MAC_ADDR(pDstMacAddr))
		return FALSE;

	if (IpProtocol != ETH_P_IPV6)
		return FALSE;

	/* skip protocol (2 Bytes). */
	pIpHeader += 2;

	do {
		PRT_IPV6_HDR pIpv6Hdr = (PRT_IPV6_HDR)(pIpHeader);
		UINT32 offset = IPV6_HDR_LEN;
		nextProtocol = pIpv6Hdr->nextHdr;

		while (nextProtocol == IPV6_NEXT_HEADER_HOP_BY_HOP) {
			if (IPv6ExtHdrHandle((RT_IPV6_EXT_HDR *)(pIpHeader + offset), &nextProtocol, &offset) == FALSE)
				break;
		}
	} while (FALSE);

	for (idx = 0; idx < IPV6_MULTICAST_FILTER_EXCLUED_SIZE; idx++) {
		if (nextProtocol == IPv6MulticastFilterExclued[idx]) {
			result = TRUE;
			break;
		}
	}

	return result;
}

/*  MLD v1 messages have the following format:
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Type      |     Code      |          Checksum             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|     Maximum Response Delay    |          Reserved             |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	+                                                               +
	|                                                               |
	+                       Multicast Address                       +
	|                                                               |
	+                                                               +
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

/*	Version 3 Membership Report Message
	0                   1                   2                   3
	0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Type = 143   |    Reserved   |           Checksum            |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|           Reserved            |  Number of Group Records (M)  |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [1]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [2]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                               .                               |
	.                               .                               .
	|                               .                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|                                                               |
	.                                                               .
	.               Multicast Address Record [M]                    .
	.                                                               .
	|                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


	where each Group Record has the following internal format:
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	|  Record Type  |  Aux Data Len |     Number of Sources (N)     |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Multicast Address                       *
    |                                                               |
    *                                                               *
    |                                                               |
	+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [1]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [2]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-                                                             -+
    .                               .                               .
    .                               .                               .
    .                               .                               .
    +-                                                             -+
    |                                                               |
    *                                                               *
    |                                                               |
    *                       Source Address [N]                      *
    |                                                               |
    *                                                               *
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    |                                                               |
    .                                                               .
    .                         Auxiliary Data                        .
    .                                                               .
    |                                                               |
    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
*/

VOID MLDSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN MAC_TABLE_ENTRY *pEntry,

	UINT8 Wcid)
{
	INT i;
	UCHAR GroupType;
	UINT16 numOfGroup;
	PUCHAR pGroup;
	UCHAR AuxDataLen;
	UINT16 numOfSources;
	PUCHAR pGroupIpAddr;
	UCHAR GroupMacAddr[6];
	PUCHAR pGroupMacAddr = (PUCHAR)&GroupMacAddr;
	UINT8 MldType;
	PUCHAR pMldHeader;
	UINT8 Type = MCAT_FILTER_DYNAMIC;
	struct wifi_dev *wdev = pEntry->wdev;
#ifdef IGMP_TVM_SUPPORT
	UCHAR TVModeType = 0;
#endif /* IGMP_TVM_SUPPORT */

	if (isMldPkt(pDstMacAddr, pIpHeader, &MldType, &pMldHeader) == TRUE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLD type=%0x\n", MldType));

#ifdef IGMP_TVM_SUPPORT
	pEntry = &pAd->MacTab.Content[Wcid];

	if (IS_IGMP_TVM_MODE_EN(wdev->IsTVModeEnable)) {
		if (pEntry->TVMode == IGMP_TVM_IE_MODE_AUTO)
			TVModeType = MCAT_FILTER_TVM_AUTO;
		else if (pEntry->TVMode == IGMP_TVM_IE_MODE_ENABLE)
			TVModeType = MCAT_FILTER_TVM_ENABLE;
	}
#endif /* IGMP_TVM_SUPPORT */

		switch (MldType) {
		case MLD_V1_LISTENER_REPORT:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
#ifdef IGMP_TVM_SUPPORT
			if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
					("Exempt Snooping IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
					GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
				break;
			}
#endif /* IGMP_TVM_SUPPORT */
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryInsert Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
			AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
				(Type
#ifdef IGMP_TVM_SUPPORT
				|TVModeType
#endif /* IGMP_TVM_SUPPORT */
				), pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case MLD_V1_LISTENER_DONE:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("EntryDelete Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
#ifdef A4_CONN
			if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
				AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case MLD_V2_LISTERNER_REPORT: /* IGMP version 3 membership report. */
			numOfGroup = ntohs(*((UINT16 *)(pMldHeader + 6)));
			pGroup = (PUCHAR)(pMldHeader + 8);

			for (i = 0; i < numOfGroup; i++) {
				GroupType = (UCHAR)(*pGroup);
				AuxDataLen = (UCHAR)(*(pGroup + 1));
				numOfSources = ntohs(*((UINT16 *)(pGroup + 2)));
				pGroupIpAddr = (PUCHAR)(pGroup + 4);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLDv2 Type=%d, ADL=%d, numOfSource=%d\n",
						 GroupType, AuxDataLen, numOfSources));
				ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLD Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
						 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
						 GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));

				do {
					if ((GroupType == MODE_IS_EXCLUDE)
						|| (GroupType == CHANGE_TO_EXCLUDE_MODE)
						|| (GroupType == ALLOW_NEW_SOURCES)) {
#ifdef IGMP_TVM_SUPPORT
						if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
							MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
								("Exempt Snooping MLD Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
								GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2],
								GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
							break;
						}
#endif /* IGMP_TVM_SUPPORT */
						AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
							(Type
#ifdef IGMP_TVM_SUPPORT
							|TVModeType
#endif /* IGMP_TVM_SUPPORT */
							), pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0) {
#ifdef A4_CONN
							if (pEntry && !((wdev->wdev_type == WDEV_TYPE_AP) && IS_ENTRY_A4(pEntry))) /* skip entry delete for member on MWDS Cli, rely on perioic membership query & aging activity for entry deletion*/
#endif
								AsicMcastEntryDelete(pAd,
											GroupMacAddr,
											wdev->bss_info_argument.ucBssIndex,
											pSrcMacAddr,
											wdev->if_dev,
											Wcid);
						} else {
#ifdef IGMP_TVM_SUPPORT
							if (isIgmpMldExemptPkt(pAd, wdev, pGroupIpAddr, ETH_P_IPV6) == TRUE) {
								MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
									("Exempt Snooping MLD Group="
									"%02x:%02x:%02x:%02x:%02x:%02x\n",
									GroupMacAddr[0], GroupMacAddr[1],
									GroupMacAddr[2], GroupMacAddr[3],
									GroupMacAddr[4], GroupMacAddr[5]));
								break;
							}
#endif /* IGMP_TVM_SUPPORT */
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex,
								(Type
#ifdef IGMP_TVM_SUPPORT
								|TVModeType
#endif /* IGMP_TVM_SUPPORT */
								), pSrcMacAddr, wdev->if_dev, Wcid);
						}
						break;
					}
				} while (FALSE);

				/* skip 4 Bytes (Record Type, Aux Data Len, Number of Sources) + a IPv6 address. */
				pGroup += (4 + IPV6_ADDR_LEN + (numOfSources * 16) + AuxDataLen);
			}

			break;

		default:
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("unknow MLD Type=%d\n", MldType));
			break;
		}
	}

	return;
}


#ifdef A4_CONN
/* Indicate if Specific Pkt is an MLD query message*/
BOOLEAN isMLDquery(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pIpHeader)
{
	UINT8 MldType = 0;
	PUCHAR pMldHeader;
	BOOLEAN isMLDquery = FALSE;

	if (isMldPkt(pDstMacAddr, pIpHeader, &MldType, &pMldHeader) == TRUE) {
		switch (MldType) {
		case MLD_LISTENER_QUERY:
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_INFO, ("isMLDquery-> MLD type=0x%x MLD_LISTENER_QUERY\n", MldType));
			isMLDquery = TRUE;
			break;

		}
	}

	return isMLDquery;
}

UCHAR	TYPEIPV4[] = {0x08, 0x00};

/* Send an IGMP query message on particular AP interface*/
void send_igmpv3_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x01, 0x00, 0x5e, 0x00, 0x00, 0x01}; /* as per IGMP spec */
	UCHAR Header802_3[14] = {0};
	UCHAR CustomPayload[36];

	UCHAR IpHdr[24] = {0x46, 0x00, /* version(4bit), hdr lenght(4bit), tos*/
							   0x00, 0x24, /* total ip datagram length*/
							   0x00, 0x01, /* identification (random)*/
							   0x00, 0x00, /* flag & fragmentation*/
							   0x01, 0x02, /* TTL, Protocol type (as per igmp spec)*/
							   0x44, 0xD2, /* hdr checksum (considered 0 for calculation and computed manually for this msg)*/
							   0x00, 0x00, /* Source IP (0.0.0.0)*/
							   0x00, 0x00, /* Source IP*/
							   0xE0, 0x00, /* Dest IP (224.0.0.1 - All Host addr as per IGMP spec)*/
							   0x00, 0x01, /* Dest IP*/
							   0x94, 0x04, /* Router Alert (as per IPv4 Router alert spec & IGMP spec)*/
							   0x00, 0x00}; /* Router Alert*/

	UCHAR               IgmpGenQuery[12] = {0x11, 0x0A, /* type(Mmbrship Query), Max Rsp Code (10 i.e 1 sec)*/
						0xEE, 0xF5, /* chksum (considered 0 for calculation and computed manually for this msg)*/
						0x00, 0x00, /* Grp addrss (general query)*/
						0x00, 0x00, /* Grp addr*/
						0x00, 0x00, /* rsv, S, QRC	QQIC (other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("send_igmpv3_gen_query_pkt---->\n"));

	NdisZeroMemory(Header802_3, sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV4);

	/* Using fixed payload due to checksum calculation required using one's complement*/
	NdisCopyMemory(&CustomPayload[0], IpHdr, 24);
	NdisCopyMemory(&CustomPayload[24], IgmpGenQuery, 12);

	/* Copy frame to Tx ring*/

	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 36, FALSE);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- send_igmpv3_gen_query_pkt\n"));
}

UCHAR	TYPEIPV6[] = {0x86, 0xDD};

/* Send a MLD query message on particular AP interface*/
void send_mldv2_gen_query_pkt(
	IN	PRTMP_ADAPTER	pAd,
	IN  PMAC_TABLE_ENTRY pMacEntry)
{
	UCHAR ALL_HOST_ADDR[MAC_ADDR_LEN] = {0x33, 0x33, 0x00, 0x00, 0x00, 0x01}; /*as per MLD spec*/
	UCHAR Header802_3[14] = {0};
	UCHAR CustomPayload[76];
	BSS_STRUCT *pMbss = &pAd->ApCfg.MBSSID[pMacEntry->wdev->func_idx];

	UCHAR Ipv6Hdr[40] = {0x60, 0x00, /* version(4bit), Tclass(8bit), Flow label (4bit+)*/
					0x00, 0x00, /* Flow Label (16 bit)*/
					0x00, 0x24, /* Payload length	(Ipv6 Router alert + MLD Gen Query)*/
					0x00, 0x01, /*Next hdr (Hop y Hop Options for Router Alert), Hop Limit*/
									0x00, 0x00, /* ipv6 src addr	(Ipv6 link-local addr)		<-------------- to update from preformed address*/
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0xFF, 0x02, /* ipv6 dst addr	(All node Ipv6 Multcast addr, as per MLD spec)*/
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x00,
									0x00, 0x01};

	UCHAR Ipv6RouterAlert[8] = {0x3A, 0x00, /* NxtHdr (ICMPv6-MLD), Hdr Ext len*/
									   0x05, 0x02, /* Option Type - Router Alert, Length*/
									   0x00, 0x00, /* Value - MLD*/
									   0x01, 0x00}; /* Padding - Pad2*/

	UCHAR MldGenQuery[28] = {0x82, 0x00, /* type(MLD Mmbrship Query), Code*/
						0x00, 0x00, /* chksum						<------- to update from precomputed checksum for each mbss*/
						0x03, 0xE8, /* max rsp code (1000 ms i.e. 1 second)*/
						0x00, 0x00, /* rsvd*/
						0x00, 0x00, /* ipv6 grp addr (general query)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00, /* rsv, S, QRC	QQIC	(other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	/* Get the Link Local Src Addr for this interface*/
	NdisCopyMemory(&Ipv6Hdr[8], &pMbss->ipv6LinkLocalSrcAddr[0], 16);

	/* Get Checksum*/
	MldGenQuery[2] = (pMbss->MldQryChkSum >> 8);
	MldGenQuery[3] = (pMbss->MldQryChkSum & 0xff);

	/* Form the pkt to be sent*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("send_mldv2_gen_query_pkt---->\n"));

	NdisZeroMemory(Header802_3, sizeof(UCHAR)*14);

	MAKE_802_3_HEADER(Header802_3, ALL_HOST_ADDR, &pMacEntry->wdev->if_addr[0], TYPEIPV6);

	/* Using fixed payload due to checksum calculation required using one's complement*/
	NdisZeroMemory(CustomPayload, 76);
	NdisCopyMemory(&CustomPayload[0], Ipv6Hdr, 40);
	NdisCopyMemory(&CustomPayload[40], Ipv6RouterAlert, 8);
	NdisCopyMemory(&CustomPayload[48], MldGenQuery, 28);

	/* Copy frame to Tx ring*/

	RTMPToWirelessSta((PRTMP_ADAPTER)pAd, pMacEntry,
					 Header802_3, LENGTH_802_3, (PUCHAR)CustomPayload, 76, FALSE);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- send_mldv2_gen_query_pkt\n"));
}

/* For specifed MBSS, compute & store IPv6 format checksum for MLD query message to be sent on that interface*/
void calc_mldv2_gen_query_chksum(
	IN	PRTMP_ADAPTER	pAd,
	IN  BSS_STRUCT *pMbss)
{
	UCHAR CustomPayload[68];
	UINT32 sum = 0, operand = 0, exCarry = 0;
	UINT16 chksum = 0, dataLen = 0;
	UINT8 ctr = 0;

	UCHAR ipv6LinkLocalSrcAddr[16] = {0xFE, 0x80, /*ipv6 src addr	(Ipv6 link-local addr)		<------------------to form*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0xFF,
						0xFE, 0x00,
						0x00, 0x00};

	UCHAR Ipv6PsuedoHdr[40] = { 0x00, 0x00, /* ipv6 src addr (Ipv6 link-local addr)		<------------------to form*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0xFF, 0x02, /* ipv6 dst addr (All node Ipv6 Multcast addr as per spec)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x01,
						0x00, 0x00, /* Next Hdr (MLD) Pkt Len*/
						0x00, 0x1C, /* Next Hdr (MLD) Pkt Len*/
						0x00, 0x00, /* Zero*/
						0x00, 0x3A}; /* Zero, Next hdr (ICMPv6 - MLD)*/

	UCHAR               MldGenQuery[28] = {0x82, 0x00, /* type(MLD Mmbrship Query), Code*/
						0x00, 0x00, /* chksum		<--------- calculate*/
						0x03, 0xE8, /* max rsp code (1000 ms i.e. 1 second)*/
						0x00, 0x00, /* rsvd*/
						0x00, 0x00, /* ipv6 grp addr (general query)*/
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00,
						0x00, 0x00, /* rsv, S, QRC	QQIC	(other routers to use defaults)*/
						0x00, 0x00}; /* No of Sources	(general query)*/

	/* Form the Link Local Src Addr for this interface in EUI-64 format, as per spec*/
	ipv6LinkLocalSrcAddr[8] = pMbss->wdev.if_addr[0];
	ipv6LinkLocalSrcAddr[9] = pMbss->wdev.if_addr[1];
	ipv6LinkLocalSrcAddr[10] = pMbss->wdev.if_addr[2];
	ipv6LinkLocalSrcAddr[13] = pMbss->wdev.if_addr[3];
	ipv6LinkLocalSrcAddr[14] = pMbss->wdev.if_addr[4];
	ipv6LinkLocalSrcAddr[15] = pMbss->wdev.if_addr[5];

	ipv6LinkLocalSrcAddr[8] = ipv6LinkLocalSrcAddr[8] ^ 0x02; /* togle universal/local bit*/

	NdisCopyMemory(&pMbss->ipv6LinkLocalSrcAddr[0], &ipv6LinkLocalSrcAddr[0], 16);
	NdisCopyMemory(&Ipv6PsuedoHdr[0], &ipv6LinkLocalSrcAddr[0], 16);

	/* Rakesh: A print is required here to avoid crash during reboot. Please don't remove !!!*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("calc_mldv2_gen_query_chksum -->"));

	/*MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("calc_mldv2_gen_query_chksum --> LinkLocal Ipv6Addr[%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x] \n",
		pMbss->ipv6LinkLocalSrcAddr[0],pMbss->ipv6LinkLocalSrcAddr[1],pMbss->ipv6LinkLocalSrcAddr[2],
		pMbss->ipv6LinkLocalSrcAddr[3],pMbss->ipv6LinkLocalSrcAddr[4],pMbss->ipv6LinkLocalSrcAddr[5],
		pMbss->ipv6LinkLocalSrcAddr[6],pMbss->ipv6LinkLocalSrcAddr[7],pMbss->ipv6LinkLocalSrcAddr[8],
		pMbss->ipv6LinkLocalSrcAddr[9],pMbss->ipv6LinkLocalSrcAddr[10],pMbss->ipv6LinkLocalSrcAddr[11],
		pMbss->ipv6LinkLocalSrcAddr[12],pMbss->ipv6LinkLocalSrcAddr[13],pMbss->ipv6LinkLocalSrcAddr[14],
		pMbss->ipv6LinkLocalSrcAddr[15]));*/

	/* Calculate Checksum*/
	NdisZeroMemory(CustomPayload, 68);
	NdisCopyMemory(&CustomPayload[0], Ipv6PsuedoHdr, 40);
	NdisCopyMemory(&CustomPayload[40], MldGenQuery, 28);
	dataLen = 68; /* total size of Pseudo Hdr & MLD*/

	/* Note: current logic assumes even data len, as per IP checksum format*/
	sum = 0;
	operand = 0;
	for (ctr = 0; ctr < dataLen; ctr += 2) {
		operand = (UINT32)(CustomPayload[ctr] << 8);
		operand |= (UINT32)(CustomPayload[ctr+1]);

		sum += operand;
	}
	exCarry = sum >> 16;
	sum = sum & 0x0000ffff;

	while (exCarry != 0) {
		sum += exCarry;
		exCarry = sum >> 16;
		sum = sum & 0x0000ffff;
	}
	chksum = (UINT16)sum;
	chksum = chksum ^ 0xFFFF;
	if (chksum == 0)
		chksum = 0xFFFF;
	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("ChkSum Done: chksum: %04x \n", chksum));

	NdisCopyMemory(&pMbss->MldQryChkSum, &chksum, 2);

	MTWF_LOG(DBG_CAT_CLIENT, CATCLIENT_APCLI, DBG_LVL_INFO, ("<----- calc_mldv2_gen_query_chksum\n"));
}
#endif


#endif /* IGMP_SNOOP_SUPPORT */
