#ifdef IGMP_SNOOP_SUPPORT

#include "rt_config.h"
#include "ipv6.h"
#include "igmp_snoop.h"

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
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMember(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList,
	IN PUCHAR pMemberAddr);

static VOID DeleteIgmpMemberList(
	IN PMULTICAST_FILTER_TABLE pMulticastFilterTable,
	IN PLIST_HEADER pList);


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

	/* check the rule is in table already or not. */
	pEntry = MulticastFilterTableLookup(pMulticastFilterTable, pGrpId, dev);
	if (pEntry) {
		/* doesn't indicate member mac address. */
		if (pMemberAddr == NULL)
			return FALSE;

		pMemberEntry = (PMEMBER_ENTRY)pEntry->MemberList.pHead;

		while (pMemberEntry) {
			if (MAC_ADDR_EQUAL(pMemberAddr, pMemberEntry->Addr)) {
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_ERROR, ("%s: already in Members list.\n", __func__));
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
			InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);
			break;
		}

		/* allocate one MAC entry */
		for (i = 0; i < MAX_LEN_OF_MULTICAST_FILTER_TABLE; i++) {
			/* pick up the first available vacancy */
			pEntry = &pMulticastFilterTable->Content[i];
			NdisGetSystemUpTime(&Now);

			if ((pEntry->Valid == TRUE) && (pEntry->type == MCAT_FILTER_DYNAMIC)
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime + IGMPMAC_TB_ENTRY_AGEOUT_TIME)) {
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
				pEntry->type = type;
				initList(&pEntry->MemberList);

				if (pMemberAddr != NULL)
					InsertIgmpMember(pMulticastFilterTable, &pEntry->MemberList, pMemberAddr);

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
				&& RTMP_TIME_AFTER(Now, pEntry->lastTime + IGMPMAC_TB_ENTRY_AGEOUT_TIME)) {
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

VOID IGMPSnooping(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pDstMacAddr,
	IN PUCHAR pSrcMacAddr,
	IN PUCHAR pIpHeader,
	IN struct wifi_dev *wdev,
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

	if (isIgmpPkt(pDstMacAddr, pIpHeader)) {
		IpHeaderLen = (*(pIpHeader + 2) & 0x0f) * 4;
		pIgmpHeader = pIpHeader + 2 + IpHeaderLen;
		IgmpVerType = (UCHAR)(*(pIgmpHeader));
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP type=%0x\n", IgmpVerType));

		switch (IgmpVerType) {
		case IGMP_V1_MEMBERSHIP_REPORT: /* IGMP version 1 membership report. */
		case IGMP_V2_MEMBERSHIP_REPORT: /* IGMP version 2 membership report. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
			AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case IGMP_LEAVE_GROUP: /* IGMP version 1 and version 2 leave group. */
			pGroupIpAddr = (PUCHAR)(pIgmpHeader + 4);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IP);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("IGMP Group=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
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
						AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0)
							AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
						else
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);

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
	IN PUCHAR pMemberAddr)
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
	UINT Enable;
	
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	
#ifdef CONFIG_AP_SUPPORT
	    wdev = &pAd->ApCfg.MBSSID[pObj->ioctl_if].wdev;
#endif

	Enable = (UINT) simple_strtol(arg, 0, 10);

	wdev->IgmpSnoopEnable = (BOOLEAN)(Enable == 0 ? FALSE : TRUE);

	MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE,
		("%s:: wdev[%d],OMAC[%d]-%s\n", __FUNCTION__, wdev->wdev_idx,
		wdev->DevInfo.OwnMacIdx,
		Enable == TRUE ? "Enable IGMP Snooping":"Disable IGMP Snooping"));

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

		/* Group-Id must be a MCAST address. */
		if ((bGroupId == 1) && IS_MULTICAST_MAC_ADDR(Addr)) {
			AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, NULL, pDev, 0);
		}
		/* Group-Member must be a UCAST address. */
		else if ((bGroupId == 0) && !IS_MULTICAST_MAC_ADDR(Addr)) {
			if (pEntry)
				AsicMcastEntryInsert(pAd, GroupId, pAd->ApCfg.MBSSID[ifIndex].wdev.bss_info_argument.ucBssIndex, MCAT_FILTER_STATIC, Addr, pDev, pEntry->wcid);
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

	/*IgmpSnEnable */
	if (RTMPGetKeyParameter("IgmpSnEnable", tmpbuf, 128, buffer, FALSE)) {
		for (i = 0, macptr = rstrtok(tmpbuf, ";"); macptr ; macptr = rstrtok(NULL, ";"), i++) {
			if (i < DBDC_BAND_NUM) {
				band_idx = rtmp_band_index_get_by_order(pAd, i);
				pAd->ApCfg.IgmpSnoopEnable[band_idx] = simple_strtol(macptr, 0, 10);
				MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, (" Band[%d]-IGMP Snooping Enable=%d\n", band_idx, pAd->ApCfg.IgmpSnoopEnable[band_idx]));
			} else
				break;
		}
	}
}

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
		if (IgmpMldPkt) {
			*ppGroupEntry = NULL;
			*pInIgmpGroup = IGMP_PKT;
		} else if (*ppGroupEntry == NULL) {
			RELEASE_NDIS_PACKET(pAd, pPacket, NDIS_STATUS_FAILURE);
			return NDIS_STATUS_FAILURE;
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
		if (pMacEntry && (Sst == SST_ASSOC) &&
			(pAd->MacTab.tr_entry[pMacEntry->wcid].PortSecured == WPA_802_1X_PORT_SECURED)) {
			tr_entry = &pAd->MacTab.tr_entry[pMacEntry->wcid];
			OS_PKT_CLONE(pAd, pPacket, pSkbClone, MEM_ALLOC_FLAG);

			if ((pSkbClone)
			   ) {
				RTMP_SET_PACKET_WCID(pSkbClone, (UCHAR)pMacEntry->wcid);
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

	return NDIS_STATUS_SUCCESS;
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
	IN struct wifi_dev *wdev,
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

	if (isMldPkt(pDstMacAddr, pIpHeader, &MldType, &pMldHeader) == TRUE) {
		MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("MLD type=%0x\n", MldType));

		switch (MldType) {
		case MLD_V1_LISTENER_REPORT:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
			AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);
			break;

		case MLD_V1_LISTENER_DONE:
			/* skip Type(1 Byte), code(1 Byte), checksum(2 Bytes), Maximum Rsp Delay(2 Bytes), Reserve(2 Bytes). */
			pGroupIpAddr = (PUCHAR)(pMldHeader + 8);
			ConvertMulticastIP2MAC(pGroupIpAddr, (PUCHAR *)&pGroupMacAddr, ETH_P_IPV6);
			MTWF_LOG(DBG_CAT_PROTO, CATPROTO_IGMP, DBG_LVL_TRACE, ("Group Id=%02x:%02x:%02x:%02x:%02x:%02x\n",
					 GroupMacAddr[0], GroupMacAddr[1], GroupMacAddr[2], GroupMacAddr[3], GroupMacAddr[4], GroupMacAddr[5]));
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
						AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);
						break;
					}

					if ((GroupType == CHANGE_TO_INCLUDE_MODE)
						|| (GroupType == MODE_IS_INCLUDE)
						|| (GroupType == BLOCK_OLD_SOURCES)) {
						if (numOfSources == 0)
							AsicMcastEntryDelete(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, pSrcMacAddr, wdev->if_dev, Wcid);
						else
							AsicMcastEntryInsert(pAd, GroupMacAddr, wdev->bss_info_argument.ucBssIndex, MCAT_FILTER_DYNAMIC, pSrcMacAddr, wdev->if_dev, Wcid);

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

#endif /* IGMP_SNOOP_SUPPORT */
