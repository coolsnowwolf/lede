#ifdef BLOCK_NET_IF

#include "rt_config.h"
#include "netif_block.h"

static NETIF_ENTRY freeNetIfEntryPool[FREE_NETIF_POOL_SIZE];
static LIST_HEADER freeNetIfEntryList;


void initblockQueueTab(RTMP_ADAPTER *pAd)
{
	int i;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT8 num_of_tx_ring = GET_NUM_OF_TX_RING(cap);

	initList(&freeNetIfEntryList);

	for (i = 0; i < FREE_NETIF_POOL_SIZE; i++)
		insertTailList(&freeNetIfEntryList, (RT_LIST_ENTRY *)&freeNetIfEntryPool[i]);

	for (i = 0; i < num_of_tx_ring; i++)
		initList(&pAd->blockQueueTab[i].NetIfList);

	return;
}


BOOLEAN blockNetIf(BLOCK_QUEUE_ENTRY *pBlockQueueEntry, PNET_DEV pNetDev)
{
	NETIF_ENTRY *pNetIfEntry = NULL;
	pNetIfEntry = (NETIF_ENTRY *)removeHeadList(&freeNetIfEntryList);
	if (pNetIfEntry != NULL) {
		RTMP_OS_NETDEV_STOP_QUEUE(pNetDev);
		pNetIfEntry->pNetDev = pNetDev;
		insertTailList(&pBlockQueueEntry->NetIfList, (RT_LIST_ENTRY *)pNetIfEntry);
		pBlockQueueEntry->SwTxQueueBlockFlag = TRUE;
		MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMP_OS_NETDEV_STOP_QUEUE(%s)\n", RTMP_OS_NETDEV_GET_DEVNAME(pNetDev)));
	} else
		return FALSE;

	return TRUE;
}


VOID releaseNetIf(BLOCK_QUEUE_ENTRY *pBlockQueueEntry)
{
	NETIF_ENTRY *pNetIfEntry = NULL;
	LIST_HEADER *pNetIfList = &pBlockQueueEntry->NetIfList;

	while ((pNetIfEntry = (NETIF_ENTRY *)removeHeadList(pNetIfList)) !=  NULL) {
		PNET_DEV pNetDev = pNetIfEntry->pNetDev;

		if (pNetDev) {
			RTMP_OS_NETDEV_WAKE_QUEUE(pNetDev);
			MTWF_LOG(DBG_CAT_TX, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMP_OS_NETDEV_WAKE_QUEUE(%s)\n",
					 RTMP_OS_NETDEV_GET_DEVNAME(pNetDev)));
		}

		insertTailList(&freeNetIfEntryList, (RT_LIST_ENTRY *)pNetIfEntry);
	}

	pBlockQueueEntry->SwTxQueueBlockFlag = FALSE;
	return;
}


VOID StopNetIfQueue(RTMP_ADAPTER *pAd, UCHAR QueIdx, PNDIS_PACKET pPacket)
{
	UCHAR wdev_idx;
	struct wifi_dev *wdev;
	wdev_idx = RTMP_GET_PACKET_WDEV(pPacket);

	if ((wdev_idx < WDEV_NUM_MAX) && (pAd->wdev_list[wdev_idx] != NULL)) {
		wdev = pAd->wdev_list[wdev_idx];

		/* WMM support 4 software queues.*/
		/* One software queue full doesn't mean device have no capbility to transmit packet.*/
		/* So disable block Net-If queue function while WMM enable.*/
		if ((wdev->bWmmCapable == FALSE) && (wdev->if_dev))
			blockNetIf(&pAd->blockQueueTab[QueIdx], wdev->if_dev);
	}

	return;
}

#endif /* BLOCK_NET_IF */

