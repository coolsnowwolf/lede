
#ifndef __LINK_LIST_H__
#define __LINK_LIST_H__

typedef struct _LIST_ENTRY
{
	struct _LIST_ENTRY *pNext;
} LIST_ENTRY, *PLIST_ENTRY;

typedef struct _LIST_HEADR
{
	PLIST_ENTRY pHead;
	PLIST_ENTRY pTail;
	UCHAR size;
} LIST_HEADER, *PLIST_HEADER;

static inline VOID initList(
	IN PLIST_HEADER pList)
{
	pList->pHead = pList->pTail = NULL;
	pList->size = 0;
	return;
}

static inline VOID insertTailList(
	IN PLIST_HEADER pList,
	IN PLIST_ENTRY pEntry)
{
	pEntry->pNext = NULL;
	if (pList->pTail)
		pList->pTail->pNext = pEntry;
	else
		pList->pHead = pEntry;
	pList->pTail = pEntry;
	pList->size++;

	return;
}

static inline PLIST_ENTRY removeHeadList(
	IN PLIST_HEADER pList)
{
	PLIST_ENTRY pNext;
	PLIST_ENTRY pEntry;

	pEntry = pList->pHead;
	if (pList->pHead != NULL)
	{
		pNext = pList->pHead->pNext;
		pList->pHead = pNext;
		if (pNext == NULL)
			pList->pTail = NULL;
		pList->size--;
	}
	return pEntry;
}

static inline int getListSize(
	IN PLIST_HEADER pList)
{
	return pList->size;
}

static inline PLIST_ENTRY delEntryList(
	IN PLIST_HEADER pList,
	IN PLIST_ENTRY pEntry)
{
	PLIST_ENTRY pCurEntry;
	PLIST_ENTRY pPrvEntry;

	if(pList->pHead == NULL)
		return NULL;

	if(pEntry == pList->pHead)
	{
		pCurEntry = pList->pHead;
		pList->pHead = pCurEntry->pNext;

		if(pList->pHead == NULL)
			pList->pTail = NULL;

		pList->size--;
		return pCurEntry;
	}

	pPrvEntry = pList->pHead;
	pCurEntry = pPrvEntry->pNext;
	while(pCurEntry != NULL)
	{
		if (pEntry == pCurEntry)
		{
			pPrvEntry->pNext = pCurEntry->pNext;

			if(pEntry == pList->pTail)
				pList->pTail = pPrvEntry;

			pList->size--;
			break;
		}
		pPrvEntry = pCurEntry;
		pCurEntry = pPrvEntry->pNext;
	}

	return pCurEntry;
}


typedef struct _DL_LIST
{
	struct _DL_LIST *Next;
	struct _DL_LIST *Prev;
}DL_LIST, *PDL_LIST;

static inline void DlListInit(struct _DL_LIST *List)
{
	List->Next = List;
	List->Prev = List;
}

static inline void DlListAdd(struct _DL_LIST *List, struct _DL_LIST *Item)
{
	Item->Next = List->Next;
	Item->Prev = List;
	List->Next->Prev = Item;
	List->Next = Item; 	
}

static inline void DlListAddTail(struct _DL_LIST *List, struct _DL_LIST *Item)
{
	DlListAdd(List->Prev, Item);
}

static inline void DlListDel(struct _DL_LIST *Item)
{
	Item->Next->Prev = Item->Prev;
	Item->Prev->Next = Item->Next;
	Item->Next = NULL;
	Item->Prev = NULL;
}

static inline int DlListEmpty(struct _DL_LIST *List)
{
	return List->Next == List;
}

static inline unsigned int DlListLen(struct _DL_LIST *List)
{
	struct _DL_LIST *Item;
	unsigned int Count = 0;
	
	for (Item = List->Next; Item != List; Item = Item->Next)
		Count++;
	
	return Count;
}


#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif

#define DlListEntry(item, type, member) \
    ((type *) ((char *) item - offsetof(type, member)))

#define DlListFirst(list, type, member) \
    (DlListEmpty((list)) ? NULL : \
     DlListEntry((list)->Next, type, member))

#define DlListForEach(item, list, type, member) \
    for (item = DlListEntry((list)->Next, type, member); \
         &item->member != (list); \
         item = DlListEntry(item->member.Next, type, member))

#define DlListForEachSafe(item, n, list, type, member) \
    for (item = DlListEntry((list)->Next, type, member), \
             n = DlListEntry(item->member.Next, type, member); \
         &item->member != (list); \
         item = n, n = DlListEntry(n->member.Next, type, member))

#endif /* ___LINK_LIST_H__ */

