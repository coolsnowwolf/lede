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
	rt_linux_cmm.h

    Abstract:
	Common OS structure/definition in LINUX whatever OS ABL.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
 */


#ifndef __RT_LINUX_CMM_H__
#define __RT_LINUX_CMM_H__


typedef struct _OS_RSTRUC  {
	UCHAR *pContent; /* pointer to real structure content */
} OS_RSTRUC;


/* declare new chipset function here */
#ifdef OS_ABL_FUNC_SUPPORT

#define RTMP_DECLARE_DRV_OPS_FUNCTION(_func)					\
	void Rtmp_Drv_Ops_##_func(VOID * __pDrvOps, VOID * __pNetOps, \
							  VOID * __pPciConfig, VOID * __pUsbConfig)

#define RTMP_BUILD_DRV_OPS_FUNCTION(_func)						\
	void Rtmp_Drv_Ops_##_func(VOID * __pDrvOps, VOID * __pNetOps, \
							  VOID * __pPciConfig, VOID * __pUsbConfig)		\
	{																\
		RtmpDrvOpsInit(__pDrvOps, __pNetOps, __pPciConfig, __pUsbConfig);\
	}

/* #define RTMP_GET_DRV_OPS_FUNCTION(_func)	(PVOID)Rtmp_Drv_Ops_##_func */

#define RTMP_DRV_OPS_FUNCTION_BODY(_func)						\
	Rtmp_Drv_Ops_##_func


#define xdef_to_str(s)   def_to_str(s)
#define def_to_str(s)    #s

#ifdef RTMP_MAC_PCI
#ifdef MT_MAC
#define RTMP_DRV_NAME	"mtpci" xdef_to_str(RT28xx_MODE)
#else
#define RTMP_DRV_NAME	"rtpci" xdef_to_str(RT28xx_MODE)
#endif
RTMP_DECLARE_DRV_OPS_FUNCTION(pci);
#define RTMP_DRV_OPS_FUNCTION				RTMP_DRV_OPS_FUNCTION_BODY(pci)
#define RTMP_BUILD_DRV_OPS_FUNCTION_BODY	RTMP_BUILD_DRV_OPS_FUNCTION(pci)
#endif /* RTMP_MAC_PCI */


#else


#ifdef MT_MAC
#define RTMP_DRV_NAME   "mt_drv"
#else
#define RTMP_DRV_NAME	"rt2860"
#endif /* MT_MAC */

#endif /* OS_ABL_FUNC_SUPPORT */


/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/
#define RTMP_OS_TASK_INIT(__pTask, __pTaskName, __pAd)		\
	RtmpOSTaskInit(__pTask, __pTaskName, __pAd, &(__pAd)->RscTaskMemList, &(__pAd)->RscSemMemList);

#ifndef OS_ABL_FUNC_SUPPORT

/* rt_linux.h */
#define RTMP_OS_TASK				OS_TASK

#define RTMP_OS_TASK_GET(__pTask)							\
	(__pTask)

#define RTMP_OS_TASK_DATA_GET(__pTask)						\
	((__pTask)->priv)

#define RTMP_OS_TASK_IS_KILLED(__pTask)						\
	((__pTask)->task_killed)

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	WAKE_UP(pTask);
#else
#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	RTMP_SEM_EVENT_UP(&(pTask)->taskSema);
#endif /* KTHREAD_SUPPORT */

#ifdef KTHREAD_SUPPORT
#define RTMP_OS_TASK_LEGALITY(__pTask)						\
	((__pTask)->kthread_task != NULL)
#else
#define RTMP_OS_TASK_LEGALITY(__pTask)						\
	CHECK_PID_LEGALITY((__pTask)->taskPID)
#endif /* KTHREAD_SUPPORT */

#else

/* rt_linux_cmm.h */
#define RTMP_OS_TASK				OS_RSTRUC

#define RTMP_OS_TASK_GET(__pTask)							\
	((OS_TASK *)((__pTask)->pContent))

#define RTMP_OS_TASK_DATA_GET(__pTask)						\
	RtmpOsTaskDataGet(__pTask)

#define RTMP_OS_TASK_IS_KILLED(__pTask)						\
	RtmpOsTaskIsKilled(__pTask)

#define RTMP_OS_TASK_WAKE_UP(__pTask)						\
	RtmpOsTaskWakeUp(pTask)

#define RTMP_OS_TASK_LEGALITY(__pTask)						\
	RtmpOsCheckTaskLegality(__pTask)

#endif /* OS_ABL_FUNC_SUPPORT */


/*****************************************************************************
 * Timer related definitions and data structures.
 ******************************************************************************/
#ifndef OS_ABL_FUNC_SUPPORT

/* rt_linux.h */
#define NDIS_MINIPORT_TIMER			OS_NDIS_MINIPORT_TIMER
#define RTMP_OS_TIMER				OS_TIMER

#define RTMP_OS_FREE_TIMER(__pAd)
#define RTMP_OS_FREE_LOCK(__pAd)
#define RTMP_OS_FREE_TASKLET(__pAd)
#define RTMP_OS_FREE_TASK(__pAd)
#define RTMP_OS_FREE_SEM(__pAd)
#define RTMP_OS_FREE_ATOMIC(__pAd)

#else

/* rt_linux_cmm.h */
#define NDIS_MINIPORT_TIMER			OS_RSTRUC
#define RTMP_OS_TIMER				OS_RSTRUC

#define RTMP_OS_FREE_TIMER(__pAd)
#define RTMP_OS_FREE_LOCK(__pAd)
#define RTMP_OS_FREE_TASKLET(__pAd)
#define RTMP_OS_FREE_TASK(__pAd)
#define RTMP_OS_FREE_SEM(__pAd)
#define RTMP_OS_FREE_ATOMIC(__pAd)

#endif /* OS_ABL_FUNC_SUPPORT */


/*****************************************************************************
 *	OS file operation related data structure definitions
 ******************************************************************************/
/* if you add any new type, please also modify RtmpOSFileOpen() */
#define RTMP_FILE_RDONLY			0x0F01
#define RTMP_FILE_WRONLY			0x0F02
#define RTMP_FILE_CREAT				0x0F03
#define RTMP_FILE_TRUNC				0x0F04

#ifndef OS_ABL_FUNC_SUPPORT

/* rt_linux.h */
#define RTMP_OS_FS_INFO				OS_FS_INFO

#else

/* rt_linux_cmm.h */
#define RTMP_OS_FS_INFO				OS_RSTRUC

#endif /* OS_ABL_FUNC_SUPPORT */


/*****************************************************************************
 *	OS semaphore related data structure and definitions
 ******************************************************************************/

#ifndef OS_ABL_FUNC_SUPPORT

#define NDIS_SPIN_LOCK							OS_NDIS_SPIN_LOCK
#define NdisAllocateSpinLock(__pReserved, __pLock)	OS_NdisAllocateSpinLock(__pLock)
#define NdisFreeSpinLock						OS_NdisFreeSpinLock
#define RTMP_SEM_LOCK							OS_SEM_LOCK
#define RTMP_SEM_UNLOCK							OS_SEM_UNLOCK
#define RTMP_SPIN_LOCK							OS_SPIN_LOCK
#define RTMP_SPIN_UNLOCK						OS_SPIN_UNLOCK
#define RTMP_SPIN_LOCK_IRQ						OS_SPIN_LOCK_IRQ
#define RTMP_SPIN_UNLOCK_IRQ					OS_SPIN_UNLOCK_IRQ
#define RTMP_SPIN_LOCK_IRQSAVE					OS_SPIN_LOCK_IRQSAVE
#define RTMP_SPIN_UNLOCK_IRQRESTORE				OS_SPIN_UNLOCK_IRQRESTORE
#define RTMP_IRQ_LOCK							OS_IRQ_LOCK
#define RTMP_IRQ_UNLOCK							OS_IRQ_UNLOCK
#define RTMP_INT_LOCK							OS_INT_LOCK
#define RTMP_INT_UNLOCK							OS_INT_UNLOCK
#define RTMP_OS_SEM								OS_SEM
#define RTMP_OS_ATOMIC							atomic_t

#define NdisAcquireSpinLock						RTMP_SEM_LOCK
#define NdisReleaseSpinLock						RTMP_SEM_UNLOCK

#define RTMP_SEM_EVENT_INIT_LOCKED(__pSema, __pSemaList)	OS_SEM_EVENT_INIT_LOCKED(__pSema)
#define RTMP_SEM_EVENT_INIT(__pSema, __pSemaList)			OS_SEM_EVENT_INIT(__pSema)
#define RTMP_SEM_EVENT_DESTORY					OS_SEM_EVENT_DESTORY
#define RTMP_SEM_EVENT_WAIT						OS_SEM_EVENT_WAIT
#define RTMP_SEM_EVENT_UP						OS_SEM_EVENT_UP

#define RtmpMLMEUp								OS_RTMP_MlmeUp

#define RTMP_OS_ATMOIC_INIT(__pAtomic, __pAtomicList)
#define RTMP_OS_ATMOIC_DESTROY(__pAtomic)
#define RTMP_THREAD_PID_KILL(__PID)				KILL_THREAD_PID(__PID, SIGTERM, 1)

#else

#define NDIS_SPIN_LOCK							OS_RSTRUC
#define RTMP_OS_SEM								OS_RSTRUC
#define RTMP_OS_ATOMIC							OS_RSTRUC

#define RTMP_SEM_EVENT_INIT_LOCKED				RtmpOsSemaInitLocked
#define RTMP_SEM_EVENT_INIT						RtmpOsSemaInit
#define RTMP_SEM_EVENT_DESTORY					RtmpOsSemaDestory
#define RTMP_SEM_EVENT_WAIT(_pSema, _status)	((_status) = RtmpOsSemaWaitInterruptible((_pSema)))
#define RTMP_SEM_EVENT_UP						RtmpOsSemaWakeUp

#define RtmpMLMEUp								RtmpOsMlmeUp

#define RTMP_OS_ATMOIC_INIT						RtmpOsAtomicInit
#define RTMP_OS_ATMOIC_DESTROY					RtmpOsAtomicDestroy
#define RTMP_THREAD_PID_KILL					RtmpThreadPidKill

/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define NdisAllocateSpinLock(__pAd, __pLock)		RtmpOsAllocateLock(__pLock, &(__pAd)->RscLockMemList)
#define NdisFreeSpinLock							RtmpOsFreeSpinLock

#define RTMP_SEM_LOCK(__lock)					\
		RtmpOsSpinLockBh(__lock);

#define RTMP_SEM_UNLOCK(__lock)					\
		RtmpOsSpinUnLockBh(__lock);

#define RTMP_SPIN_LOCK_IRQ						RtmpOsSpinLockIrq
#define RTMP_SPIN_UNLOCK_IRQ					RtmpOsSpinUnlockIrq
#define RTMP_SPIN_LOCK_IRQSAVE					RtmpOsSpinLockIrqSave
#define RTMP_SPIN_UNLOCK_IRQRESTORE				RtmpOsSpinUnlockIrqRestore

/* sample, use semaphore lock to replace IRQ lock, 2007/11/15 */
#ifdef MULTI_CORE_SUPPORT

#define RTMP_IRQ_LOCK(__lock, __irqflags)			\
	{													\
		__irqflags = 0;									\
		spin_lock_irqsave((spinlock_t *)(__lock), __irqflags);			\
	}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)			\
	{													\
		spin_unlock_irqrestore((spinlock_t *)(__lock), __irqflag);			\
	}
#else
#define RTMP_IRQ_LOCK(__lock, __irqflags)		\
	{												\
		__irqflags = 0;								\
		RtmpOsSpinLockBh(__lock);					\
	}

#define RTMP_IRQ_UNLOCK(__lock, __irqflag)		\
	{												\
		RtmpOsSpinUnLockBh(__lock);					\
	}
#endif /* MULTI_CORE_SUPPORT // */
#define RTMP_INT_LOCK(__Lock, __Flag)	RtmpOsIntLock(__Lock, &__Flag)
#define RTMP_INT_UNLOCK					RtmpOsIntUnLock

#define NdisAcquireSpinLock				RTMP_SEM_LOCK
#define NdisReleaseSpinLock				RTMP_SEM_UNLOCK

#endif /* OS_ABL_FUNC_SUPPORT */


/*****************************************************************************
 *	OS task related data structure and definitions
 ******************************************************************************/

#ifndef OS_ABL_FUNC_SUPPORT

/* rt_linux.h */
#define RTMP_NET_TASK_STRUCT		OS_NET_TASK_STRUCT
#define PRTMP_NET_TASK_STRUCT		POS_NET_TASK_STRUCT

typedef struct completion RTMP_OS_COMPLETION;

#define RTMP_OS_INIT_COMPLETION(__pCompletion)	\
	init_completion(__pCompletion)

#define RTMP_OS_EXIT_COMPLETION(__pCompletion)	\
	complete(__pCompletion)

#define RTMP_OS_COMPLETE(__pCompletion)	\
	complete(__pCompletion)

#define RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(__pCompletion, __Timeout)	\
	wait_for_completion_timeout(__pCompletion, __Timeout)

#ifdef WORKQUEUE_BH
#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
	schedule_work(__pTasklet)
#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	INIT_WORK((struct work_struct *)__pTasklet, (work_func_t)__pFunc)
#define RTMP_OS_TASKLET_KILL(__pTasklet) \
	cancel_work_sync(__pTasklet)
#else
#define RTMP_OS_TASKLET_SCHE(__pTasklet)							\
	tasklet_hi_schedule(__pTasklet)
#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	tasklet_init(__pTasklet, __pFunc, __Data)
#define RTMP_OS_TASKLET_KILL(__pTasklet)							\
	tasklet_kill(__pTasklet)
#endif /* WORKQUEUE_BH */

#define RTMP_NET_TASK_DATA_ASSIGN(__Tasklet, __Data)		\
	((__Tasklet)->data = (unsigned long)__Data)

#else

/* rt_linux_cmm.h */
typedef OS_RSTRUC RTMP_NET_TASK_STRUCT;
typedef OS_RSTRUC * PRTMP_NET_TASK_STRUCT;
typedef OS_RSTRUC RTMP_OS_COMPLETION;
typedef OS_RSTRUC * PRTMP_OS_COMPLETION;

#define RTMP_OS_INIT_COMPLETION(__pCompletion)	\
	RtmpOsInitCompletion(__pCompletion)

#define RTMP_OS_EXIT_COMPLETION(__pCompletion)	\
	RtmpOsExitCompletion(__pCompletion)

#define RTMP_OS_COMPLETE(__pCompletion)	\
	RtmpOsComplete(__pCompletion)

#define RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(__pCompletion, __Timeout)	\
	RtmpOsWaitForCompletionTimeout(__pCompletion, __Timeout)

#define RTMP_OS_TASKLET_SCHE(__pTasklet)					\
	RtmpOsTaskletSche(__pTasklet)

#define RTMP_OS_TASKLET_INIT(__pAd, __pTasklet, __pFunc, __Data)	\
	RtmpOsTaskletInit(__pTasklet, __pFunc, __Data, &(__pAd)->RscTaskletMemList)

#define RTMP_OS_TASKLET_KILL(__pTasklet)					\
	RtmpOsTaskletKill(__pTasklet)

#define RTMP_NET_TASK_DATA_ASSIGN(__pTasklet, __Data)		\
	RtmpOsTaskletDataAssign(__pTasklet, __Data)

#endif /* OS_ABL_FUNC_SUPPORT */




/*****************************************************************************
 *	OS definition related data structure and definitions
 ******************************************************************************/

#ifdef OS_ABL_SUPPORT

#define RTMP_USB_CONTROL_MSG_ENODEV		-1
#define RTMP_USB_CONTROL_MSG_FAIL		-2

typedef struct __RTMP_PCI_CONFIG {
	UINT32	ConfigVendorID;
} RTMP_PCI_CONFIG;

typedef struct __RTMP_USB_CONFIG {
	UINT32	Reserved;
} RTMP_USB_CONFIG;

extern RTMP_PCI_CONFIG *pRtmpPciConfig;
extern RTMP_USB_CONFIG *pRtmpUsbConfig;

#define RTMP_OS_PCI_VENDOR_ID			pRtmpPciConfig->ConfigVendorID

/*
	Declare dma_addr_t here, can not define it in rt_drv.h

	If you define it in include/os/rt_drv.h, then the size in DRIVER module
	will be 64-bit, but in UTIL/NET modules, it maybe 32-bit.
	This will cause size mismatch problem when OS_ABL = yes.
*/
/*
	In big-endian & 32-bit DMA address platform, if you use long long to
	record DMA address, when you call kernel function to set DMA address,
	the address will be 0 because you need to do swap I think.
	So if you sure your DMA address is 32-bit, do not use RTMP_DMA_ADDR_64.
*/
#define ra_dma_addr_t					unsigned long long

#else


#define RTMP_OS_PCI_VENDOR_ID			PCI_VENDOR_ID
#define RTMP_OS_PCI_DEVICE_ID			PCI_DEVICE_ID

#define ra_dma_addr_t					dma_addr_t

#endif /* OS_ABL_SUPPORT */

#define PCI_MAP_SINGLE					RtmpDrvPciMapSingle


/***********************************************************************************
 *	Others
 ***********************************************************************************/
#define APCLI_IF_UP_CHECK(pAd, ifidx) (RtmpOSNetDevIsUp((pAd)->ApCfg.ApCliTab[(ifidx)].wdev.if_dev) == TRUE)

#ifdef MEMORY_OPTIMIZATION
#define MGMT_RING_SIZE          32
#else
#define MGMT_RING_SIZE          128
#endif

#ifdef MT_MAC
#define BCN_RING_SIZE		20
#endif /* MT_MAC */

#define MAX_TX_PROCESS          512
#define LOCAL_TXBUF_SIZE        2

#define CTL_RING_SIZE		128

#define RTMP_OS_NETDEV_SET_PRIV		RtmpOsSetNetDevPriv
#define RTMP_OS_NETDEV_GET_PRIV		RtmpOsGetNetDevPriv
#define RTMP_OS_NETDEV_SET_WDEV		RtmpOsSetNetDevWdev
#define RTMP_OS_NETDEV_GET_WDEV		RtmpOsGetNetDevWdev
#define RT_DEV_PRIV_FLAGS_GET		RtmpDevPrivFlagsGet
#define RT_DEV_PRIV_FLAGS_SET		RtmpDevPrivFlagsSet

#endif /* __RT_LINUX_CMM_H__ */

