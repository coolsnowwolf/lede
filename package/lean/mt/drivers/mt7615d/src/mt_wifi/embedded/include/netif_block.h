
#ifndef __NET_IF_BLOCK_H__
#define __NET_IF_BLOCK_H__

#include "common/link_list.h"
#include "rtmp.h"

#define FREE_NETIF_POOL_SIZE 32

typedef struct _NETIF_ENTRY {
	struct _NETIF_ENTRY *pNext;
	PNET_DEV pNetDev;
} NETIF_ENTRY, *PNETIF_ENTRY;

void initblockQueueTab(
	IN PRTMP_ADAPTER pAd);

BOOLEAN blockNetIf(
	IN PBLOCK_QUEUE_ENTRY pBlockQueueEntry,
	IN PNET_DEV pNetDev);

VOID releaseNetIf(
	IN PBLOCK_QUEUE_ENTRY pBlockQueueEntry);

VOID StopNetIfQueue(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR QueIdx,
	IN PNDIS_PACKET pPacket);
#endif /* __NET_IF_BLOCK_H__ */

