/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	apcli_ctrl.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Fonchi		2006-06-23      modified for rt61-APClinent
*/
#ifdef APCLI_SUPPORT
#ifdef APCLI_LINK_COVER_SUPPORT
#include "rt_config.h"

extern VOID*	p5G_pAd;
extern VOID*	p2G_pAd;

INT ApcliLinkMonitorThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_ADAPTER *pAd_other_band;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);
	
	RtmpOSTaskCustomize(pTask);

	if (p2G_pAd == NULL) {
		printk("##### no 2G pAd!!!\n");
		//RtmpOSTaskNotifyToExit(pTask);
	//	return 0;
	} else if (p5G_pAd == NULL) {
		printk("##### no 5G pAd!!!\n");
		//RtmpOSTaskNotifyToExit(pTask);
	//	return 0;
	}
	if (p5G_pAd == pAd) {
		printk("we are 5G interface, wait 2G link update\n");
		pAd_other_band = p2G_pAd;
	}
	else {
		printk("we are 2G interface, wait 5G link update\n");
		pAd_other_band = p5G_pAd;
	}

	while (pTask && !RTMP_OS_TASK_IS_KILLED(pTask) && (pAd_other_band != NULL))
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}
		if (status != 0)
			break;
		//TODO: wait_for_completion
	}

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
	printk("<---ApcliLinkMonitorThread\n");

	//if (pTask)
	//	RtmpOSTaskNotifyToExit(pTask);

	return 0;
}

NDIS_STATUS RtmpApcliLinkTaskInit(IN PRTMP_ADAPTER pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;

	printk("##### %s\n", __func__);
	/* Creat ApCli Link Monitor Thread */
	pTask = &pAd->apcliLinkTask;
	RTMP_OS_TASK_INIT(pTask, "LinkMonitorTask", pAd);
	//status = RtmpOSTaskAttach(pTask, RTPCICmdThread, (ULONG)pTask);
	status = RtmpOSTaskAttach(pTask, ApcliLinkMonitorThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE) 
	{
/*		printk ("%s: unable to start RTPCICmdThread\n", RTMP_OS_NETDEV_GET_DEVNAME(pAd->net_dev)); */
		printk ("%s: Unable to start ApcliLinkMonitorThread!\n", get_dev_name_prefix(pAd, INT_APCLI));
		return NDIS_STATUS_FAILURE;
	}

	return NDIS_STATUS_SUCCESS;
}

VOID RtmpApcliLinkTaskExit(
	IN RTMP_ADAPTER *pAd)
{
	INT ret;

	printk("##### %s\n", __func__);
	/* Terminate cmdQ thread */
	RTMP_OS_TASK_LEGALITY(&pAd->apcliLinkTask)
	{
		/*RTUSBCMDUp(&pAd->cmdQTask); */
		ret = RtmpOSTaskKill(&pAd->apcliLinkTask);
		if (ret == NDIS_STATUS_FAILURE)
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Kill command task fail!\n"));
		}
	}
	return;
}


#endif /* APCLI_LINK_COVER_SUPPORT */
#endif /* APCLI_SUPPORT */

