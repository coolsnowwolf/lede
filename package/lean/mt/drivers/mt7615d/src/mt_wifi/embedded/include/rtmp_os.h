/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
	rtmp_os.h

    Abstract:

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */

#ifndef __RTMP_OS_H__
#define __RTMP_OS_H__

/* Driver Operators */
typedef int (*RTMP_PRINTK)(const char *ftm, ...);
typedef int (*RTMP_SNPRINTF)(char *, ULONG, const char *ftm, ...);

typedef struct _RTMP_OS_ABL_OPS {
	int (*ra_printk)(const char *ftm, ...);
	int (*ra_snprintf)(char *, ULONG, const char *ftm, ...);
} RTMP_OS_ABL_OPS;

extern RTMP_OS_ABL_OPS *pRaOsOps;

#ifndef CONFIG_RTPCI_AP_RF_OFFSET
#define CONFIG_RTPCI_AP_RF_OFFSET 0
#endif

#ifdef LINUX
#ifndef OS_ABL_FUNC_SUPPORT
#include "os/rt_linux.h"

#else

#ifdef RTMP_MODULE_OS
/* for util/netif */
#include "os/rt_linux.h"
#else
/* for core */
#include "os/rt_drv.h"
#endif /* RTMP_MODULE_OS */
#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */





/* TODO: shiang, temporary put it here! */
#include "os/pkt_meta.h"

/*
	This data structure mainly strip some callback function defined in
	"struct net_device" in kernel source "include/linux/netdevice.h".

	The definition of this data structure may various depends on different
	OS. Use it carefully.
*/
typedef struct _RTMP_OS_NETDEV_OP_HOOK_ {
	void *open;
	void *stop;
	void *xmit;
	void *ioctl;
	void *get_stats;
	void *priv;
	void *get_wstats;
	void *iw_handler;
	void *wdev;
	int priv_flags;
	unsigned char devAddr[6];
	unsigned char devName[16];
	unsigned char needProtcted;
} RTMP_OS_NETDEV_OP_HOOK, *PRTMP_OS_NETDEV_OP_HOOK;


typedef enum _RTMP_TASK_STATUS_ {
	RTMP_TASK_STAT_UNKNOWN = 0,
	RTMP_TASK_STAT_INITED = 1,
	RTMP_TASK_STAT_RUNNING = 2,
	RTMP_TASK_STAT_STOPED = 4,
} RTMP_TASK_STATUS;
#define RTMP_TASK_CAN_DO_INSERT		(RTMP_TASK_STAT_INITED | RTMP_TASK_STAT_RUNNING)

#define RTMP_OS_TASK_NAME_LEN	16

#if defined(RTMP_MODULE_OS) || !defined(OS_ABL_FUNC_SUPPORT)
/* used in UTIL/NETIF module */
typedef struct _RTMP_OS_TASK_ {
	char taskName[RTMP_OS_TASK_NAME_LEN];
	void *priv;
	/*unsigned long                 taskFlags; */
	RTMP_TASK_STATUS taskStatus;
#ifndef KTHREAD_SUPPORT
	RTMP_OS_SEM taskSema;
	RTMP_OS_PID taskPID;
#ifdef LINUX
	struct completion taskComplete;
#endif /* LINUX */
#endif
	unsigned char task_killed;
#ifdef KTHREAD_SUPPORT
	struct task_struct *kthread_task;
	wait_queue_head_t kthread_q;
	BOOLEAN kthread_running;
#endif
} OS_TASK;
#endif /* RTMP_MODULE_OS || ! OS_ABL_FUNC_SUPPORT */


int RtmpOSIRQRequest(PNET_DEV pNetDev);


#ifndef OS_ABL_SUPPORT
#define RTMP_MATOpsInit(__pAd)
#define RTMP_MATPktRxNeedConvert(__pAd, __pDev)				\
	MATPktRxNeedConvert(__pAd, __pDev)
#define RTMP_MATEngineRxHandle(__pAd, __pPkt, __InfIdx)		\
	MATEngineRxHandle(__pAd, __pPkt, __InfIdx)
#else

#define RTMP_MATOpsInit(__pAd)					\
	do {								\
		(__pAd)->MATPktRxNeedConvert = MATPktRxNeedConvert;	\
		(__pAd)->MATEngineRxHandle = MATEngineRxHandle;		\
	} while (0)

#define RTMP_MATPktRxNeedConvert(__pAd, __pDev)				\
	((__pAd)->MATPktRxNeedConvert(__pAd, __pDev))
#define RTMP_MATEngineRxHandle(__pAd, __pPkt, __InfIdx)		\
	((__pAd)->MATEngineRxHandle(__pAd, __pPkt, __InfIdx))
#endif /* OS_ABL_SUPPORT */

#ifndef LINUX
typedef struct _os_kref {
	NDIS_SPIN_LOCK lock;
    UCHAR refcount;
} os_kref;
#endif

void os_kref_init(os_kref *kref);
void os_kref_get(os_kref *kref);
int os_kref_put(os_kref *kref, void (*release)(os_kref *kref));

#endif /* __RMTP_OS_H__ */
