/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2008, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
	rtmp_timer.h

    Abstract:
	Ralink Wireless Driver timer related data structures and delcarations

    Revision History:
	Who           When                What
	--------    ----------      ----------------------------------------------
	Name          Date                 Modification logs
	Shiang Tu    Aug-28-2008	init version

*/

#ifndef __RTMP_TIMER_H__
#define  __RTMP_TIMER_H__

#include "rtmp_os.h"

struct _RTMP_ADAPTER;

#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)

#define GET_TIMER_FUNCTION(_func)				\
	(PVOID)rtmp_timer_##_func

/* ----------------- Timer Related MARCO ---------------*/
/* In some os or chipset, we have a lot of timer functions and will read/write register, */
/*   it's not allowed in Linux USB sub-system to do it ( because of sleep issue when */
/*  submit to ctrl pipe). So we need a wrapper function to take care it. */

#ifdef RTMP_TIMER_TASK_SUPPORT
typedef VOID(
	*RTMP_TIMER_TASK_HANDLE) (
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3);
#endif /* RTMP_TIMER_TASK_SUPPORT */

typedef struct _RALINK_TIMER_STRUCT {
	RTMP_OS_TIMER TimerObj;	/* Ndis Timer object */
	BOOLEAN Valid;		/* Set to True when call RTMPInitTimer */
	BOOLEAN State;		/* True if timer cancelled */
	BOOLEAN PeriodicType;	/* True if timer is periodic timer */
	BOOLEAN Repeat;		/* True if periodic timer */
	ULONG TimerValue;	/* Timer value in milliseconds */
	ULONG cookie;		/* os specific object */
	void *pAd;
	NDIS_SPIN_LOCK *timer_lock;
#ifdef RTMP_TIMER_TASK_SUPPORT
	RTMP_TIMER_TASK_HANDLE handle;
#endif				/* RTMP_TIMER_TASK_SUPPORT */
	VOID *pCaller;
} RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;

typedef struct _TIMER_FUNC_CONTEXT {
	struct _RTMP_ADAPTER *pAd;
	struct wifi_dev *wdev;
	UCHAR BandIdx;
} TIMER_FUNC_CONTEXT, *PTIMER_FUNC_CONTEXT;


#ifdef RTMP_TIMER_TASK_SUPPORT
typedef struct _RTMP_TIMER_TASK_ENTRY_ {
	RALINK_TIMER_STRUCT *pRaTimer;
	struct _RTMP_TIMER_TASK_ENTRY_ *pNext;
} RTMP_TIMER_TASK_ENTRY;

#define TIMER_QUEUE_SIZE_MAX	128
typedef struct _RTMP_TIMER_TASK_QUEUE_ {
	unsigned int status;
	unsigned char *pTimerQPoll;
	RTMP_TIMER_TASK_ENTRY *pQPollFreeList;
	RTMP_TIMER_TASK_ENTRY *pQHead;
	RTMP_TIMER_TASK_ENTRY *pQTail;
} RTMP_TIMER_TASK_QUEUE;


INT RtmpTimerQThread(ULONG Context);


RTMP_TIMER_TASK_ENTRY *RtmpTimerQInsert(
	IN struct _RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

BOOLEAN RtmpTimerQRemove(
	IN struct _RTMP_ADAPTER *pAd,
	IN RALINK_TIMER_STRUCT *pTimer);

void RtmpTimerQExit(struct _RTMP_ADAPTER *pAd);
void RtmpTimerQInit(struct _RTMP_ADAPTER *pAd);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0))
#define BUILD_TIMER_FUNCTION(_func)										\
	void rtmp_timer_##_func(struct timer_list *_timer)										\
	{																			\
		PRALINK_TIMER_STRUCT	_pTimer = from_timer(_pTimer,_timer,_timer);				\
		RTMP_TIMER_TASK_ENTRY	*_pQNode;										\
		RTMP_ADAPTER			*_pAd;											\
		\
		_pTimer->handle = _func;													\
		_pAd = (RTMP_ADAPTER *)_pTimer->pAd;										\
		_pQNode = RtmpTimerQInsert(_pAd, _pTimer);								\
		if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))	\
			RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);							\
	}

#else
#define BUILD_TIMER_FUNCTION(_func)										\
	void rtmp_timer_##_func(unsigned long data)										\
	{																			\
		PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data;				\
		RTMP_TIMER_TASK_ENTRY	*_pQNode;										\
		RTMP_ADAPTER			*_pAd;											\
		\
		_pTimer->handle = _func;													\
		_pAd = (RTMP_ADAPTER *)_pTimer->pAd;										\
		_pQNode = RtmpTimerQInsert(_pAd, _pTimer);								\
		if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))	\
			RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);							\
	}
#endif
#else /* !RTMP_TIMER_TASK_SUPPORT */
#define BUILD_TIMER_FUNCTION(_func)										\
	void rtmp_timer_##_func(unsigned long data)										\
	{																			\
		PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;				\
		\
		_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer);							\
		if (pTimer->Repeat)														\
			RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
	}
#endif /* RTMP_TIMER_TASK_SUPPORT */

DECLARE_TIMER_FUNCTION(MlmePeriodicExecTimer);
DECLARE_TIMER_FUNCTION(MlmeRssiReportExec);
DECLARE_TIMER_FUNCTION(AsicRxAntEvalTimeout);
DECLARE_TIMER_FUNCTION(APSDPeriodicExec);

#ifdef DOT11W_PMF_SUPPORT
DECLARE_TIMER_FUNCTION(PMF_SAQueryTimeOut);
DECLARE_TIMER_FUNCTION(PMF_SAQueryConfirmTimeOut);
#endif /* DOT11W_PMF_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
DECLARE_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif /* DOT11N_DRAFT3 */

DECLARE_TIMER_FUNCTION(CMTimerExec);
#ifdef AP_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(APScanTimeout);
#endif /* AP_SCAN_SUPPORT */
DECLARE_TIMER_FUNCTION(APQuickResponeForRateUpExec);

#ifdef IDS_SUPPORT
DECLARE_TIMER_FUNCTION(RTMPIdsPeriodicExec);
#endif /* IDS_SUPPORT */

#ifdef DOT11R_FT_SUPPORT
DECLARE_TIMER_FUNCTION(FT_KDP_InfoBroadcast);
#endif /* DOT11R_FT_SUPPORT */

#endif /* CONFIG_AP_SUPPORT */


#ifdef TXBF_SUPPORT
DECLARE_TIMER_FUNCTION(eTxBfProbeTimerExec);
#endif /* TXBF_SUPPORT */

#ifdef WSC_INCLUDED
DECLARE_TIMER_FUNCTION(WscEAPOLTimeOutAction);
DECLARE_TIMER_FUNCTION(Wsc2MinsTimeOutAction);
DECLARE_TIMER_FUNCTION(WscUPnPMsgTimeOutAction);
DECLARE_TIMER_FUNCTION(WscM2DTimeOutAction);
DECLARE_TIMER_FUNCTION(WscPBCTimeOutAction);
#ifdef CON_WPS
DECLARE_TIMER_FUNCTION(WscScanDoneCheckTimeOutAction);
#endif /*CON_WPS*/
#ifdef WSC_STA_SUPPORT
DECLARE_TIMER_FUNCTION(WscPINTimeOutAction);
#endif
DECLARE_TIMER_FUNCTION(WscScanTimeOutAction);
DECLARE_TIMER_FUNCTION(WscProfileRetryTimeout);
#ifdef WSC_LED_SUPPORT
DECLARE_TIMER_FUNCTION(WscLEDTimer);
DECLARE_TIMER_FUNCTION(WscSkipTurnOffLEDTimer);
#endif /* WSC_LED_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(WscUpdatePortCfgTimeout);
#ifdef WSC_V2_SUPPORT
DECLARE_TIMER_FUNCTION(WscSetupLockTimeout);
#endif /* WSC_V2_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
#endif /* WSC_INCLUDED */


#ifdef CONFIG_HOTSPOT

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(PostReplyTimeout);
#endif /* CONFIG_AP_SUPPORT */
#endif /* CONFIG_HOTSPOT */



#ifdef CONFIG_ATE
DECLARE_TIMER_FUNCTION(ATEPeriodicExec);
#endif /* CONFIG_ATE */
#ifdef FTM_SUPPORT
DECLARE_TIMER_FUNCTION(FtmTimerCallback);
DECLARE_TIMER_FUNCTION(FtmTxTimerCallback);
DECLARE_TIMER_FUNCTION(FtmReqTxTimerCallback);
#endif /* FTM_SUPPORT */

#ifdef BACKGROUND_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(BackgroundScanTimeout);
DECLARE_TIMER_FUNCTION(DfsZeroWaitTimeout);
#endif /* BACKGROUND_SCAN_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(AutoChSelScanTimeout);
#endif/* CONFIG_AP_SUPPORT */


#ifdef CHANNEL_SWITCH_MONITOR_CONFIG
DECLARE_TIMER_FUNCTION(ch_switch_monitor_timeout);
#endif
#endif /* __RTMP_TIMER_H__ */

