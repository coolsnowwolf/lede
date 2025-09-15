/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	meminfo_list.h
*/
#ifndef __MEMINFO_LIST_H__
#define __MEMINFO_LIST_H__

#ifdef MEM_ALLOC_INFO_SUPPORT

#include "rtmp_comm.h"

#define POOL_ENTRY_NUMBER 2000
#define HASH_SIZE 1024

typedef struct _MEM_INFO_LIST_ENTRY {
	DL_LIST mList;
	UINT32 MemSize;
	VOID *pMemAddr;
	VOID *pCaller;
} MEM_INFO_LIST_ENTRY;
typedef struct _MEM_INFO_LIST_POOL {
	MEM_INFO_LIST_ENTRY Entry[POOL_ENTRY_NUMBER];
	DL_LIST List;
} MEM_INFO_LIST_POOL;

typedef struct _FREE_LIST_POOL {
	MEM_INFO_LIST_ENTRY head;
	MEM_INFO_LIST_POOL Poolhead;
	UINT32 EntryNumber;
	NDIS_SPIN_LOCK Lock;
	UINT32 MLlistCnt;
} FREE_LIST_POOL;

typedef struct _MEM_INFO_LIST {
	MEM_INFO_LIST_ENTRY pHead[HASH_SIZE];
	NDIS_SPIN_LOCK Lock;
	UINT32 EntryNumber;
	UINT32 CurAlcSize;
	UINT32 MaxAlcSize;
	FREE_LIST_POOL *pFreeEntrylist;
} MEM_INFO_LIST;


static FREE_LIST_POOL FreeEntrylist;

static inline MEM_INFO_LIST_ENTRY *GetEntryFromFreeList(MEM_INFO_LIST *MIList)
{
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;
	ULONG IrqFlags = 0;
	UINT32 i;

	OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags);

	if (DlListEmpty(&pFreeEntrylist->head.mList)) {
		MEM_INFO_LIST_POOL *Pool = NULL;
		MEM_INFO_LIST_POOL *pFreepool = NULL;

		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: allocated new pool\n", __func__));
		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), GFP_ATOMIC);
		pFreepool = &pFreeEntrylist->Poolhead;
		DlListAdd(&pFreepool->List, &Pool->List);
		pheadEntry = &pFreeEntrylist->head;

		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			pEntry = &Pool->Entry[i];
			DlListAdd(&pheadEntry->mList, &pEntry->mList);
		}

		pFreeEntrylist->EntryNumber += POOL_ENTRY_NUMBER;
	}

	pheadEntry = &pFreeEntrylist->head;
	pEntry = DlListFirst(&pheadEntry->mList, MEM_INFO_LIST_ENTRY, mList);
	DlListDel(&pEntry->mList);

	if (pEntry != NULL)
		pFreeEntrylist->EntryNumber -= 1;

	OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags);
	return pEntry;
}

static inline UINT32 HashF(VOID *pMemAddr)
{
	LONG addr = (LONG)pMemAddr;
	UINT32 a = addr & 0xF;
	UINT32 b = (addr & 0xF0) >> 4;
	UINT32 c = (addr & 0xF00) >> 8;
	UINT32 d = (addr & 0xF000) >> 12;
	UINT32 e = (addr & 0xF0000) >> 16;
	UINT32 f = (addr & 0xF00000) >> 20;
	UINT32 g = (addr & 0xF000000) >> 24;
	UINT32 h = (addr & 0xF0000000) >> 28;

	return (a + b * 3 + c * 5 + d * 7 + e * 11 + f * 13 + g * 17 + h * 19) % HASH_SIZE;
}
static inline VOID PoolInit(VOID)
{
	if (FreeEntrylist.MLlistCnt == 0) {
		NdisAllocateSpinLock(NULL, &FreeEntrylist.Lock);
		DlListInit(&FreeEntrylist.Poolhead.List);
		DlListInit(&FreeEntrylist.head.mList);
	}

	FreeEntrylist.MLlistCnt++;
}

static inline VOID PoolUnInit(VOID)
{
	FreeEntrylist.MLlistCnt--;

	if (FreeEntrylist.MLlistCnt == 0) {
		MEM_INFO_LIST_POOL *pEntry = NULL;

		while (!DlListEmpty(&FreeEntrylist.Poolhead.List)) {
			pEntry = DlListFirst(&FreeEntrylist.Poolhead.List, MEM_INFO_LIST_POOL, List);
			DlListDel(&pEntry->List);
			kfree(pEntry);
		}
	}
}


static inline VOID MIListInit(MEM_INFO_LIST *MIList)
{
	UINT32 i;

	for (i = 0; i < HASH_SIZE; i++)
		DlListInit(&MIList->pHead[i].mList);

	NdisAllocateSpinLock(NULL, &MIList->Lock);
	MIList->EntryNumber = 0;
	MIList->CurAlcSize = 0;
	MIList->MaxAlcSize = 0;
	PoolInit();

	if (DlListEmpty(&FreeEntrylist.Poolhead.List)) {
		MEM_INFO_LIST_POOL *Pool = NULL;
		MEM_INFO_LIST_POOL *pFreepool = NULL;
		MEM_INFO_LIST_ENTRY *pEntry = NULL;
		MEM_INFO_LIST_ENTRY *newEntry = NULL;

		Pool = kmalloc(sizeof(MEM_INFO_LIST_POOL), GFP_ATOMIC);
		pFreepool = &FreeEntrylist.Poolhead;
		DlListAdd(&pFreepool->List, &Pool->List);
		pEntry = &FreeEntrylist.head;

		for (i = 0; i < POOL_ENTRY_NUMBER; i++) {
			newEntry = &Pool->Entry[i];
			DlListAdd(&pEntry->mList, &newEntry->mList);
		}

		FreeEntrylist.EntryNumber += POOL_ENTRY_NUMBER;
	}

	MIList->pFreeEntrylist = &FreeEntrylist;
}
static inline VOID MIListExit(MEM_INFO_LIST *MIList)
{
	UINT32 i = 0;
	ULONG IrqFlags = 0;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);

	for (i = 0; i < HASH_SIZE; i++)
		DlListInit(&MIList->pHead[i].mList);

	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	NdisFreeSpinLock(&MIList->Lock);
	MIList->EntryNumber = 0;
	PoolUnInit();
}

static inline MEM_INFO_LIST_ENTRY *MIListRemove(MEM_INFO_LIST *MIList, VOID *pMemAddr)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	ULONG IrqFlags2 = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;
	MEM_INFO_LIST_ENTRY *pheadEntry = NULL;
	FREE_LIST_POOL *pFreeEntrylist = MIList->pFreeEntrylist;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	DlListForEach(pEntry, &MIList->pHead[Index].mList, MEM_INFO_LIST_ENTRY, mList) {
		if (pEntry->pMemAddr == pMemAddr) {
			DlListDel(&pEntry->mList);
			MIList->EntryNumber--;
			MIList->CurAlcSize -= pEntry->MemSize;
			OS_INT_LOCK(&pFreeEntrylist->Lock, IrqFlags2);
			pheadEntry = &pFreeEntrylist->head;
			DlListAddTail(&pheadEntry->mList, &pEntry->mList);
			pFreeEntrylist->EntryNumber += 1;
			OS_INT_UNLOCK(&pFreeEntrylist->Lock, IrqFlags2);
			break;
		}
	}
	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
	return pEntry;
}


static inline VOID MIListAddHead(MEM_INFO_LIST *MIList, UINT32 Size, VOID *pMemAddr, VOID *pCaller)
{
	UINT32 Index = HashF(pMemAddr);
	ULONG IrqFlags = 0;
	MEM_INFO_LIST_ENTRY *pEntry;
	MEM_INFO_LIST_ENTRY *pheadEntry;

	OS_INT_LOCK(&MIList->Lock, IrqFlags);
	pEntry = GetEntryFromFreeList(MIList);

	if (!pEntry) {
		OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pEntry is null!!!\n", __func__));
		return;
	}

	pEntry->MemSize = Size;
	pEntry->pMemAddr = pMemAddr;
	pEntry->pCaller = pCaller;
	pheadEntry = &MIList->pHead[Index];
	DlListAdd(&pheadEntry->mList, &pEntry->mList);
	MIList->EntryNumber++;
	MIList->CurAlcSize += pEntry->MemSize;

	if (MIList->CurAlcSize > MIList->MaxAlcSize)
		MIList->MaxAlcSize = MIList->CurAlcSize;

	OS_INT_UNLOCK(&MIList->Lock, IrqFlags);
}

static inline VOID ShowMIList(MEM_INFO_LIST *MIList)
{
	UINT32 i, total = 0;
	MEM_INFO_LIST_ENTRY *pEntry = NULL;

	for (i = 0; i < HASH_SIZE; i++) {
		DlListForEach(pEntry, &MIList->pHead[i].mList, MEM_INFO_LIST_ENTRY, mList) {
			if (pEntry->MemSize == 0)
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%u: addr = %p, caller is %pS\n", ++total, pEntry->pMemAddr, pEntry->pCaller));
			else
				MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						 ("%u: addr = %p, size = %u, caller is %pS\n", ++total, pEntry->pMemAddr, pEntry->MemSize, pEntry->pCaller));
		}
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("the number of allocated memory = %u\n", MIList->EntryNumber));
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("the number of free pool entry = %u\n", MIList->pFreeEntrylist->EntryNumber));
}

#endif /* MEM_ALLOC_INFO_SUPPORT */
#endif

