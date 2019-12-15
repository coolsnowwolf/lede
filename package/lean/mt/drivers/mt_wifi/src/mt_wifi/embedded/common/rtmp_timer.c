/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2008, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source code is an unpublished work and	the
 * use of a copyright notice does not imply	otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering,	decoding, reverse engineering or in any
 * way altering the	source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    rtmp_timer.c

    Abstract:
    task for timer handling

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name          Date            Modification logs
    Shiang Tu	08-28-2008   init version

*/

#include "rt_config.h"


BUILD_TIMER_FUNCTION(MlmePeriodicExecTimer);
/*BUILD_TIMER_FUNCTION(MlmeRssiReportExec);*/
BUILD_TIMER_FUNCTION(AsicRxAntEvalTimeout);
BUILD_TIMER_FUNCTION(APSDPeriodicExec);

#ifdef DOT11W_PMF_SUPPORT
BUILD_TIMER_FUNCTION(PMF_SAQueryTimeOut);
BUILD_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
extern VOID APDetectOverlappingExec(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

BUILD_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
BUILD_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif /* DOT11N_DRAFT3 */

BUILD_TIMER_FUNCTION(CMTimerExec);
#ifdef AP_SCAN_SUPPORT
BUILD_TIMER_FUNCTION(APScanTimeout);
#endif /* AP_SCAN_SUPPORT */
BUILD_TIMER_FUNCTION(APQuickResponeForRateUpExec);
#ifdef IDS_SUPPORT
BUILD_TIMER_FUNCTION(RTMPIdsPeriodicExec);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
BUILD_TIMER_FUNCTION(FT_KDP_InfoBroadcast);
#endif /* DOT11R_FT_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */


#ifdef WSC_INCLUDED
BUILD_TIMER_FUNCTION(WscEAPOLTimeOutAction);
BUILD_TIMER_FUNCTION(Wsc2MinsTimeOutAction);
BUILD_TIMER_FUNCTION(WscUPnPMsgTimeOutAction);
BUILD_TIMER_FUNCTION(WscM2DTimeOutAction);

BUILD_TIMER_FUNCTION(WscPBCTimeOutAction);
#ifdef CON_WPS
BUILD_TIMER_FUNCTION(WscScanDoneCheckTimeOutAction);
#endif /*CPN_WPS*/
#ifdef WSC_STA_SUPPORT
BUILD_TIMER_FUNCTION(WscPINTimeOutAction);
#endif
BUILD_TIMER_FUNCTION(WscScanTimeOutAction);
BUILD_TIMER_FUNCTION(WscProfileRetryTimeout);
#ifdef WSC_LED_SUPPORT
BUILD_TIMER_FUNCTION(WscLEDTimer);
BUILD_TIMER_FUNCTION(WscSkipTurnOffLEDTimer);
#endif /* WSC_LED_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
BUILD_TIMER_FUNCTION(WscUpdatePortCfgTimeout);
#ifdef WSC_V2_SUPPORT
BUILD_TIMER_FUNCTION(WscSetupLockTimeout);
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#endif /* WSC_INCLUDED */



#ifdef TXBF_SUPPORT
#ifndef MT_MAC
BUILD_TIMER_FUNCTION(eTxBfProbeTimerExec);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */


#ifdef CONFIG_ATE
BUILD_TIMER_FUNCTION(ATEPeriodicExec);
#endif /* CONFIG_ATE */

#ifdef FTM_SUPPORT
BUILD_TIMER_FUNCTION(FtmTimerCallback);
BUILD_TIMER_FUNCTION(FtmTxTimerCallback);
BUILD_TIMER_FUNCTION(FtmReqTxTimerCallback);
#endif /* FTM_SUPPORT */

#ifdef BACKGROUND_SCAN_SUPPORT
BUILD_TIMER_FUNCTION(BackgroundScanTimeout);
BUILD_TIMER_FUNCTION(DfsZeroWaitTimeout);
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
BUILD_TIMER_FUNCTION(AutoChSelScanTimeout);
#endif/* CONFIG_AP_SUPPORT */


#ifdef RTMP_TIMER_TASK_SUPPORT
static void RtmpTimerQHandle(RTMP_ADAPTER *pAd)
{
	int status;
	RALINK_TIMER_STRUCT *pTimer;
	RTMP_TIMER_TASK_ENTRY *pEntry;
	unsigned long	irqFlag;
	RTMP_OS_TASK *pTask;
	pTask = &pAd->timerTask;

	while (!RTMP_OS_TASK_IS_KILLED(pTask)) {
		pTimer = NULL;

		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE) {
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->TimerQ.status == RTMP_TASK_STAT_STOPED)
			break;

		/* event happened.*/
		while (pAd->TimerQ.pQHead) {
			RTMP_INT_LOCK(&pAd->TimerQLock, irqFlag);
			pEntry = pAd->TimerQ.pQHead;

			if (pEntry) {
				pTimer = pEntry->pRaTimer;
				/* update pQHead*/
				pAd->TimerQ.pQHead = pEntry->pNext;

				if (pEntry == pAd->TimerQ.pQTail)
					pAd->TimerQ.pQTail = NULL;

				/* return this queue entry to timerQFreeList.*/
				pEntry->pNext = pAd->TimerQ.pQPollFreeList;
				pAd->TimerQ.pQPollFreeList = pEntry;
			}

			RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlag);

			if (pTimer) {
				if ((pTimer->handle != NULL) && (!pAd->PM_FlgSuspend))
					pTimer->handle(NULL, (PVOID) pTimer->cookie, NULL, pTimer);

				if ((pTimer->Repeat) && (pTimer->State == FALSE))
					RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);
			}
		}

		if (status != 0) {
			pAd->TimerQ.status = RTMP_TASK_STAT_STOPED;
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
	}
}


INT RtmpTimerQThread(
	IN ULONG Context)
{
	RTMP_OS_TASK	*pTask;
	PRTMP_ADAPTER	pAd = NULL;
	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	if (pAd == NULL) {
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:: pAd is NULL!\n", __func__));
		return 0;
	}

	RtmpOSTaskCustomize(pTask);
	RtmpTimerQHandle(pAd);
	MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<---%s\n", __func__));
	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	RtmpOSTaskNotifyToExit(pTask);
	return 0;
}


RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer)
{
	RTMP_TIMER_TASK_ENTRY *pQNode = NULL, *pQTail;
	unsigned long irqFlags;
	RTMP_OS_TASK	*pTask = &pAd->timerTask;
	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);

	if (pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT) {
		if (pAd->TimerQ.pQPollFreeList) {
			pQNode = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pQNode->pNext;
			pQNode->pRaTimer = pTimer;
			pQNode->pNext = NULL;
			pQTail = pAd->TimerQ.pQTail;

			if (pAd->TimerQ.pQTail != NULL)
				pQTail->pNext = pQNode;

			pAd->TimerQ.pQTail = pQNode;

			if (pAd->TimerQ.pQHead == NULL)
				pAd->TimerQ.pQHead = pQNode;
		}
	}

	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);

	if (pQNode)
		RTMP_OS_TASK_WAKE_UP(pTask);

	return pQNode;
}


BOOLEAN RtmpTimerQRemove(
	IN RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer)
{
	RTMP_TIMER_TASK_ENTRY *pNode, *pPrev = NULL;
	unsigned long irqFlags;
	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);

	if (pAd->TimerQ.status >= RTMP_TASK_STAT_INITED) {
		pNode = pAd->TimerQ.pQHead;

		while (pNode) {
			if (pNode->pRaTimer == pTimer)
				break;

			pPrev = pNode;
			pNode = pNode->pNext;
		}

		/* Now move it to freeList queue.*/
		if (pNode) {
			if (pNode == pAd->TimerQ.pQHead)
				pAd->TimerQ.pQHead = pNode->pNext;

			if (pNode == pAd->TimerQ.pQTail)
				pAd->TimerQ.pQTail = pPrev;

			if (pPrev != NULL)
				pPrev->pNext = pNode->pNext;

			/* return this queue entry to timerQFreeList.*/
			pNode->pNext = pAd->TimerQ.pQPollFreeList;
			pAd->TimerQ.pQPollFreeList = pNode;
		}
	}

	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);
	return TRUE;
}


void RtmpTimerQExit(RTMP_ADAPTER *pAd)
{
	RTMP_TIMER_TASK_ENTRY *pTimerQ;
	unsigned long irqFlags;
	RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);

	while (pAd->TimerQ.pQHead) {
		pTimerQ = pAd->TimerQ.pQHead;
		pAd->TimerQ.pQHead = pTimerQ->pNext;
		/* remove the timeQ*/
	}

	pAd->TimerQ.pQPollFreeList = NULL;

	if (pAd->TimerQ.pTimerQPoll) {
		os_free_mem(pAd->TimerQ.pTimerQPoll);
		pAd->TimerQ.pTimerQPoll = NULL;
	} else
		MTWF_LOG(DBG_CAT_MLME, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s null pTimerQPoll\n", __func__));

	pAd->TimerQ.pQTail = NULL;
	pAd->TimerQ.pQHead = NULL;
	/*#ifndef KTHREAD_SUPPORT*/
	pAd->TimerQ.status = RTMP_TASK_STAT_STOPED;
	/*#endif*/
	RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);
	/*	NdisFreeSpinLock(&pAd->TimerQLock); */
}


void RtmpTimerQInit(RTMP_ADAPTER *pAd)
{
	int	i;
	RTMP_TIMER_TASK_ENTRY *pQNode, *pEntry;
	unsigned long irqFlags;
	NdisAllocateSpinLock(pAd, &pAd->TimerQLock);
	NdisZeroMemory(&pAd->TimerQ, sizeof(pAd->TimerQ));
	os_alloc_mem(pAd, &pAd->TimerQ.pTimerQPoll, sizeof(RTMP_TIMER_TASK_ENTRY) * TIMER_QUEUE_SIZE_MAX);

	if (pAd->TimerQ.pTimerQPoll) {
		pEntry = NULL;
		pQNode = (RTMP_TIMER_TASK_ENTRY *)pAd->TimerQ.pTimerQPoll;
		NdisZeroMemory(pAd->TimerQ.pTimerQPoll, sizeof(RTMP_TIMER_TASK_ENTRY) * TIMER_QUEUE_SIZE_MAX);
		RTMP_INT_LOCK(&pAd->TimerQLock, irqFlags);

		for (i = 0; i < TIMER_QUEUE_SIZE_MAX; i++) {
			pQNode->pNext = pEntry;
			pEntry = pQNode;
			pQNode++;
		}

		pAd->TimerQ.pQPollFreeList = pEntry;
		pAd->TimerQ.pQHead = NULL;
		pAd->TimerQ.pQTail = NULL;
		pAd->TimerQ.status = RTMP_TASK_STAT_INITED;
		RTMP_INT_UNLOCK(&pAd->TimerQLock, irqFlags);
	}
}
#endif /* RTMP_TIMER_TASK_SUPPORT */

