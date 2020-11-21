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
	rt_sdio.c
*/


#include "rt_config.h"

void InitSDIODevice(VOID *ad_src)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)ad_src;
	pAd->infType = RTMP_DEV_INF_SDIO;
	UINT32 value=0;

	pAd->BlockSize =512;   //shoud read block size from sdio cccr info

	RTMP_SDIO_READ32(pAd, WCIR,&value);
	DBGPRINT(RT_DEBUG_ERROR, ("%s(): WCIR1:%x\n",__FUNCTION__,value));

	pAd->ChipID = GET_CHIP_ID(value);

	if(IS_MT7636(pAd)){
	DBGPRINT(RT_DEBUG_ERROR, ("%s():chip is MT7636\n",__FUNCTION__));
	//return FALSE;
	}else{
	DBGPRINT(RT_DEBUG_ERROR, ("%s():chip is not MT7636\n",__FUNCTION__));
	}

      RtmpRaDevCtrlInit(pAd, pAd->infType);

   
   return;
}

NDIS_STATUS	 RtmpMgmtTaskInit(RTMP_ADAPTER *pAd)
{



	return NDIS_STATUS_SUCCESS;
}


VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd)
{




	return NDIS_STATUS_SUCCESS;
}


static VOID MTSdioWorker(struct work_struct *work)
{
	unsigned long flags;
	POS_COOKIE pObj = container_of(work, struct os_cookie, SdioWork); 
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
	SDIOWorkTask *CurTask, *TmpTask;

	RTMP_SPIN_LOCK_IRQSAVE(&pAd->SdioWorkTaskLock, &flags);
	DlListForEachSafe(CurTask, TmpTask,
					&pAd->SdioWorkTaskList, SDIOWorkTask, List)
	{
		RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->SdioWorkTaskLock, &flags);

		MTSDIODataWorkerTask(pAd);

		RTMP_SPIN_LOCK_IRQSAVE(&pAd->SdioWorkTaskLock, &flags);
			
		DlListDel(&CurTask->List);
		os_free_mem(NULL, CurTask);
	}

	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->SdioWorkTaskLock, &flags);
}


NDIS_STATUS RtmpNetTaskInit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	unsigned long flags;

	INIT_WORK(&pObj->SdioWork, MTSdioWorker);

	pObj->SdioWq = create_singlethread_workqueue("mtk_wifi_sdio_wq");

	if (pObj->SdioWq == NULL)
	{
		DBGPRINT(RT_DEBUG_OFF, ("%s: create mtk_wifi_sdio_wq workqueue fail\n"));
		cancel_work_sync(&pObj->SdioWork);
	}
	
	NdisAllocateSpinLock(pAd, &pAd->SdioWorkTaskLock);
	NdisAllocateSpinLock(pAd, &pAd->IntStatusLock);
	NdisAllocateSpinLock(pAd, &pAd->TcCountLock);
	
	RTMP_SPIN_LOCK_IRQSAVE(&pAd->SdioWorkTaskLock, &flags);
	DlListInit(&pAd->SdioWorkTaskList);
	RTMP_SPIN_UNLOCK_IRQRESTORE(&pAd->SdioWorkTaskLock, &flags);

	
	return NDIS_STATUS_SUCCESS;
}


VOID RtmpNetTaskExit(IN RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE)pAd->OS_Cookie;
	
	cancel_work_sync(&pObj->SdioWork);
	
	if (pObj->SdioWq)
		destroy_workqueue(pObj->SdioWq); 

	NdisFreeSpinLock(&pAd->SdioWorkTaskLock);
	NdisFreeSpinLock(&pAd->IntStatusLock);
	NdisFreeSpinLock(&pAd->TcCountLock);
}


VOID rt_sdio_interrupt(struct sdio_func *func)
{

	VOID* pAd;
	struct net_device *net_dev = sdio_get_drvdata(func);
   
	GET_PAD_FROM_NET_DEV(pAd, net_dev);

	DBGPRINT(RT_DEBUG_ERROR, ("%s()!!!!\n", __FUNCTION__));
	
	MTSDIODataIsr(pAd);
}

