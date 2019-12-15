/****************************************************************************

    Module Name:
	rt_linux.h

    Abstract:
	Any OS related definition/MACRO is defined here.

    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_LINUX_H__
#define __RT_LINUX_H__

#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/pci.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/ethtool.h>
#include <linux/wireless.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/if_arp.h>
#include <linux/ctype.h>
#include <linux/vmalloc.h>


#include <linux/wireless.h>
#include <net/iw_handler.h>
#if defined(CONFIG_TSO_SUPPORT) || defined(CONFIG_CSO_SUPPORT)
#include <net/ip6_checksum.h>
#endif /* defined(CONFIG_TSO_SUPPORT) || defined(CONFIG_CSO_SUPPORT) */

#ifdef INF_PPA_SUPPORT
#include <net/ifx_ppa_api.h>
#include <net/ifx_ppa_hook.h>
#endif /* INF_PPA_SUPPORT */

/* load firmware */
#define __KERNEL_SYSCALLS__
#include <linux/unistd.h>
#include <asm/uaccess.h>
#include <asm/types.h>
#include <asm/unaligned.h>	/* for get_unaligned() */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
#include <linux/pid.h>
#endif

#include "common/link_list.h"

#ifdef RT_CFG80211_SUPPORT
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 28))
#include <net/mac80211.h>
/* #define EXT_BUILD_CHANNEL_LIST */	/* must define with CRDA */
#else /* LINUX_VERSION_CODE */
#undef RT_CFG80211_SUPPORT
#endif /* LINUX_VERSION_CODE */
#endif /* RT_CFG80211_SUPPORT */

#ifdef MAT_SUPPORT
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/ip.h>
#endif /* MAT_SUPPORT */

/* must put the definition before include "os/rt_linux_cmm.h" */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#define KTHREAD_SUPPORT 1
#endif /* LINUX_VERSION_CODE */

#ifdef KTHREAD_SUPPORT
#include <linux/err.h>
#include <linux/kthread.h>
#endif /* KTHREAD_SUPPORT */

#include "os/rt_linux_cmm.h"

#ifdef RT_CFG80211_SUPPORT
#include "cfg80211.h"
#endif /* RT_CFG80211_SUPPORT */

#include <linux/firmware.h>

#undef AP_WSC_INCLUDED
#undef STA_WSC_INCLUDED
#undef WSC_INCLUDED

#ifdef CONFIG_AP_SUPPORT
#ifdef WSC_AP_SUPPORT
#define AP_WSC_INCLUDED
#endif /* WSC_AP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


#if defined(WSC_AP_SUPPORT) || defined(WSC_STA_SUPPORT)
#define WSC_INCLUDED
#endif

#ifdef KTHREAD_SUPPORT
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 4)
#error "This kerne version doesn't support kthread!!"
#endif
#endif /* KTHREAD_SUPPORT */

/*#ifdef RTMP_USB_SUPPORT // os abl move */
typedef struct usb_device	*PUSB_DEV;
typedef struct urb *purbb_t;
typedef struct usb_ctrlrequest devctrlrequest;
/*#endif */

/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/

#define L1_PROFILE_PATH	"/etc/wireless/l1profile.dat"
#define L1PROFILE_INDEX_LEN		10
#define	L1PROFILE_ATTRNAME_LEN	30
#define	L2PROFILE_PATH_LEN		50

#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#if CONFIG_RTPCI_AP_RF_OFFSET == 0x48000
#define AP_PROFILE_PATH			"/etc/Wireless/iNIC/iNIC_ap.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc_ro/Wireless/iNIC/RT2860AP.bin"
#else
#define AP_PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc/Wireless/RT2860AP/RT2860AP.bin"
#endif


#define AP_DRIVER_VERSION			"5.0.2.0"
#ifdef MULTIPLE_CARD_SUPPORT
#define CARD_INFO_PATH			"/etc/Wireless/RT2860AP/RT2860APCard.dat"
#endif /* MULTIPLE_CARD_SUPPORT */
#endif /* RTMP_MAC_PCI */



#ifdef RTMP_RBUS_SUPPORT
/* This used for rbus-based chip, maybe we can integrate it together. */
#define RTMP_FIRMWARE_FILE_NAME		"/etc_ro/Wireless/RT2860AP/RT2860AP.bin"
#define PROFILE_PATH			"/etc/Wireless/RT2860i.dat"
#define AP_PROFILE_PATH_RBUS		"/etc/Wireless/RT2860/RT2860.dat"
#define RT2880_AP_DRIVER_VERSION	"1.0.0.0"
#endif /* RTMP_RBUS_SUPPORT */

#ifdef SINGLE_SKU_V2
#define SINGLE_SKU_TABLE_FILE_NAME	"/etc_ro/Wireless/RT2860AP/SingleSKU.dat"
#define BF_SKU_TABLE_FILE_NAME      "/etc_ro/Wireless/RT2860AP/SingleSKU_BF.dat"
#endif /* SINGLE_SKU_V2 */

#endif /* CONFIG_AP_SUPPORT */



#ifdef CONFIG_APSTA_MIXED_SUPPORT
extern	const struct iw_handler_def rt28xx_ap_iw_handler_def;
#endif /* CONFIG_APSTA_MIXED_SUPPORT */

/***********************************************************************************
 *	Compiler related definitions
 ***********************************************************************************/
#define IN
#define OUT
#define INOUT
#define NDIS_STATUS		INT

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
#define DEVEXIT
#define DEVEXIT_P
#define DEVINIT
#define DEVINITDATA
#else
#define DEVEXIT __devexit
#define DEVEXIT_P __devexit_p
#define DEVINIT __devinit
#define DEVINITDATA __devinitdata
#endif

/***********************************************************************************
 *	OS Specific definitions and data structures
 ***********************************************************************************/
typedef struct net_device_stats	NET_DEV_STATS;
/* #ifdef RTMP_RBUS_SUPPORT */
/* typedef struct platform_device * PPCI_DEV; */
/* #else */
typedef struct pci_dev		*PPCI_DEV;
/* #endif */	/*RTMP_RBUS_SUPPORT*/
typedef struct net_device	*PNET_DEV;
typedef struct wireless_dev	*PWIRELESS_DEV;
typedef void				*PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;
typedef	ra_dma_addr_t			NDIS_PHYSICAL_ADDRESS;
typedef	ra_dma_addr_t			*PNDIS_PHYSICAL_ADDRESS;
typedef void				*NDIS_HANDLE;
typedef char				*PNDIS_BUFFER;
typedef struct ifreq		NET_IOCTL;
typedef struct ifreq		*PNET_IOCTL;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
typedef	struct pid *RTMP_OS_PID;
#else
typedef pid_t				RTMP_OS_PID;
#endif

typedef struct semaphore	OS_SEM;

typedef int (*HARD_START_XMIT_FUNC)(struct sk_buff *skb, struct net_device *net_dev);

#ifdef RTMP_MAC_PCI
#ifndef PCI_DEVICE
#define PCI_DEVICE(vend, dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif /* PCI_DEVICE */
#endif /* RTMP_MAC_PCI */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#define RT_MOD_INC_USE_COUNT() \
	do {\
		if (!try_module_get(THIS_MODULE)) { \
			MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: cannot reserve module\n", __func__)); \
			return -1; \
		}	\
	} while (0)

#define RT_MOD_DEC_USE_COUNT() module_put(THIS_MODULE);
#else
#define RT_MOD_INC_USE_COUNT()	MOD_INC_USE_COUNT;
#define RT_MOD_DEC_USE_COUNT() MOD_DEC_USE_COUNT;
#endif

#define RTMP_INC_REF(_A)		0
#define RTMP_DEC_REF(_A)		0
#define RTMP_GET_REF(_A)		0

#if defined(CONFIG_FAST_NAT_SUPPORT)
#define RT_MOD_HNAT_DEREG(_net_dev) \
	do {\
		if (ppe_dev_unregister_hook != NULL) \
			ppe_dev_unregister_hook(_net_dev);	\
	} while (0)
#define RT_MOD_HNAT_REG(_net_dev) \
	do {\
		if (ppe_dev_register_hook != NULL) \
			ppe_dev_register_hook(_net_dev);	\
	} while (0)
#else
#define RT_MOD_HNAT_DEREG(_net_dev) do {} while (0)
#define RT_MOD_HNAT_REG(_net_dev) do {} while (0)
#endif /*CONFIG_FAST_NAT_SUPPORT*/


#if WIRELESS_EXT >= 12
/* This function will be called when query /proc */
struct iw_statistics *rt28xx_get_wireless_stats(
	IN struct net_device *net_dev);
#endif


/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define MAC_ADDR_LEN		6

#define NDIS_STATUS_SUCCESS                     0x00
#define NDIS_STATUS_FAILURE                     0x01
#define NDIS_STATUS_INVALID_DATA		0x02
#define NDIS_STATUS_RESOURCES                   0x03
#define NDIS_STATUS_PKT_REQUEUE			0x04

#define NDIS_SET_PACKET_STATUS(_p, _status)			do {} while (0)
#define NdisWriteErrorLogEntry(_a, _b, _c, _d)		do {} while (0)

/* statistics counter */
#define STATS_INC_RX_PACKETS(_pAd, _dev)
#define STATS_INC_TX_PACKETS(_pAd, _dev)

#define STATS_INC_RX_BYTESS(_pAd, _dev, len)
#define STATS_INC_TX_BYTESS(_pAd, _dev, len)

#define STATS_INC_RX_ERRORS(_pAd, _dev)
#define STATS_INC_TX_ERRORS(_pAd, _dev)

#define STATS_INC_RX_DROPPED(_pAd, _dev)
#define STATS_INC_TX_DROPPED(_pAd, _dev)


/***********************************************************************************
 *	Ralink Specific network related constant definitions
 ***********************************************************************************/

#ifdef DOT11_VHT_AC
#define MAX_PACKETS_IN_QUEUE		1024 /*(512)*/
#else
#define MAX_PACKETS_IN_QUEUE		(512)
#endif /* DOT11_VHT_AC */


/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/


/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
typedef struct file *RTMP_OS_FD;

typedef struct _OS_FS_INFO_ {
	int				fsuid;
	int				fsgid;
	mm_segment_t	fs;
} OS_FS_INFO;

typedef struct _RTMP_OS_FD {
	RTMP_OS_FD fsFd;
	OS_FS_INFO fsInfo;
	INT Status;
	UINT32 fsize;
} RTMP_OS_FD_EXT;

#define IS_FILE_OPEN_ERR(_fd)	((_fd == NULL) || IS_ERR((_fd)))


/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
typedef struct os_lock  {
	spinlock_t	lock;
	unsigned long	flags;
} OS_LOCK;

typedef struct completion OS_EVENT;
typedef NDIS_PACKET OS_PACKET;
typedef NDIS_PHYSICAL_ADDRESS OS_PHYSICAL_ADDRESS;


typedef spinlock_t			OS_NDIS_SPIN_LOCK;

/* */
/*  spin_lock enhanced for Nested spin lock */
/* */
#define OS_NdisAllocateSpinLock(__lock)			\
	spin_lock_init((spinlock_t *)(__lock))

#define OS_NdisFreeSpinLock(lock)				\
	do {} while (0)


#define OS_SEM_LOCK(__lock)						\
	spin_lock_bh((spinlock_t *)(__lock))

#define OS_SEM_UNLOCK(__lock)					\
	spin_unlock_bh((spinlock_t *)(__lock))


/* sample, use semaphore lock to replace IRQ lock, 2007/11/15 */
#ifdef MULTI_CORE_SUPPORT
#define OS_IRQ_LOCK(__lock, __irqflags)			\
	{													\
		__irqflags = 0;									\
		spin_lock_irqsave((spinlock_t *)(__lock), __irqflags);			\
	}

#define OS_IRQ_UNLOCK(__lock, __irqflag)			\
	spin_unlock_irqrestore((spinlock_t *)(__lock), __irqflag)
#else
#define OS_IRQ_LOCK(__lock, __irqflags)			\
	{												\
		__irqflags = 0;								\
		spin_lock_bh((spinlock_t *)(__lock));		\
	}

#define OS_IRQ_UNLOCK(__lock, __irqflag)		\
	spin_unlock_bh((spinlock_t *)(__lock))

#endif /* MULTI_CORE_SUPPORT // */
#define OS_INT_LOCK(__lock, __irqflags)			\
	spin_lock_irqsave((spinlock_t *)__lock, __irqflags)

#define OS_INT_UNLOCK(__lock, __irqflag)		\
	spin_unlock_irqrestore((spinlock_t *)(__lock), ((unsigned long)__irqflag))

#define OS_NdisAcquireSpinLock		OS_SEM_LOCK
#define OS_NdisReleaseSpinLock		OS_SEM_UNLOCK

/*
	Following lock/unlock definition used for BBP/RF register read/write.
	Currently we don't use it to protect MAC register access.

	For USB:
			we use binary semaphore to do the protection because all register
			access done in kernel thread and should allow task go sleep when
			in protected status.

	For PCI/PCI-E/RBUS:
			We use interrupt to do the protection because the register may accessed
			in thread/tasklet/timer/inteerupt, so we use interrupt_disable to protect
			the access.
*/
#define RTMP_MCU_RW_LOCK(_pAd, _irqflags)	\
	do {								\
		if (_pAd->infType == RTMP_DEV_INF_USB)	\
			RTMP_SEM_EVENT_WAIT(&_pAd->McuCmdSem, _irqflags);\
		else\
			RTMP_SEM_LOCK(&_pAd->McuCmdLock, _irqflags);\
	} while (0)

#define RTMP_MCU_RW_UNLOCK(_pAd, _irqflags)	\
	do {				\
		if (_pAd->infType == RTMP_DEV_INF_USB)\
			RTMP_SEM_EVENT_UP(&_pAd->McuCmdSem);\
		else\
			RTMP_SEM_UNLOCK(&_pAd->McuCmdLock, _irqflags);\
	} while (0)


#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret) \
	do { \
		wait_queue_t __wait; \
		init_waitqueue_entry(&__wait, current); \
		add_wait_queue(&wq, &__wait); \
		for (;;) { \
			set_current_state(TASK_INTERRUPTIBLE); \
			if (condition) \
				break; \
			if (!signal_pending(current)) { \
				ret = schedule_timeout(ret); \
				if (!ret) \
					break; \
				continue; \
			} \
			ret = -ERESTARTSYS; \
			break; \
		} \
		current->state = TASK_RUNNING; \
		remove_wait_queue(&wq, &__wait); \
	} while (0)

#define wait_event_interruptible_timeout(wq, condition, timeout) \
	({ \
		long __ret = timeout; \
		if (!(condition)) \
			__wait_event_interruptible_timeout(wq, condition, __ret); \
		__ret; \
	})
#endif

#define OS_SEM_EVENT_INIT_LOCKED(_pSema)	sema_init((_pSema), 0)
#define OS_SEM_EVENT_INIT(_pSema)		sema_init((_pSema), 1)
#define OS_SEM_EVENT_DESTORY(_pSema)		do {} while (0)
#define OS_SEM_EVENT_WAIT(_pSema, _status)	((_status) = down_interruptible((_pSema)))
#define OS_SEM_EVENT_UP(_pSema)			up(_pSema)

#define RTCMDUp					OS_RTCMDUp

#ifdef KTHREAD_SUPPORT
#define RTMP_WAIT_EVENT_INTERRUPTIBLE(_Status, _pTask) \
	do { \
		wait_event_interruptible(_pTask->kthread_q, _pTask->kthread_running || kthread_should_stop()); \
		_pTask->kthread_running = FALSE; \
		if (kthread_should_stop()) {	\
			(_Status) = -1;		\
			break;			\
		} else				\
			(_Status) = 0;		\
	} while (0)
#endif

#ifdef KTHREAD_SUPPORT
#define RTMP_WAIT_EVENT_INTERRUPTIBLE_COND(_Status, _pTask, _Cond) \
	do { \
		wait_event_interruptible(_pTask->kthread_q, _pTask->kthread_running || kthread_should_stop() || (_Cond)); \
		_pTask->kthread_running = FALSE; \
		if (kthread_should_stop()) {	\
			(_Status) = -1;		\
			break;			\
		} else				\
			(_Status) = 0;		\
	} while (0)
#endif

#ifdef KTHREAD_SUPPORT
#define WAKE_UP(_pTask) \
	do { \
		if ((_pTask)->kthread_task) { \
			(_pTask)->kthread_running = TRUE; \
			wake_up(&(_pTask)->kthread_q); \
		} \
	} while (0)
#endif

/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define MEM_ALLOC_FLAG      (GFP_ATOMIC) /*(GFP_DMA | GFP_ATOMIC) */

#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))

#define MlmeAllocateMemory(_pAd, _ppVA)		os_alloc_mem(_pAd, _ppVA, MAX_MGMT_PKT_LEN)
#define MlmeFreeMemory(_pVA)			os_free_mem(_pVA)

#define COPY_MAC_ADDR(Addr1, Addr2)             memcpy((Addr1), (Addr2), MAC_ADDR_LEN)

#define CONTAINER_OF(ptr, type, member)   container_of(ptr, type, member)


/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
#define RTMP_OS_MGMT_TASK_FLAGS	CLONE_VM

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
/*typedef	struct pid *	THREAD_PID; // no use */
#define	THREAD_PID_INIT_VALUE	NULL
/* TODO: Use this IOCTL carefully when linux kernel version larger than 2.6.27, because the PID only correct when the user space task do this ioctl itself. */
/*#define RTMP_GET_OS_PID(_x, _y)    _x = get_task_pid(current, PIDTYPE_PID); */
#ifdef OS_ABL_FUNC_SUPPORT
#define RT_GET_OS_PID(_x, _y)		do {rcu_read_lock(); _x = (ULONG)current->pids[PIDTYPE_PID].pid; rcu_read_unlock(); } while (0)
#define RTMP_GET_OS_PID(_a, _b)			RtmpOsGetPid(&_a, _b)
#else
#ifdef CONFIG_PREEMPT_RCU
#define RT_GET_OS_PID(_x, _y)
#else /* else CONFIG_PREEMPT_RCU */
#define RT_GET_OS_PID(_x, _y)		do {rcu_read_lock(); _x = current->pids[PIDTYPE_PID].pid; rcu_read_unlock(); } while (0)
#endif /* CONFIG_PREEMPT_RCU */
#define RTMP_GET_OS_PID(_a, _b)			RT_GET_OS_PID(_a, _b)
#endif /* OS_ABL_FUNC_SUPPORT */
#define	GET_PID_NUMBER(_v)	pid_nr((_v))
#define CHECK_PID_LEGALITY(_pid)	(pid_nr((_pid)) > 0)
#define KILL_THREAD_PID(_A, _B, _C)	kill_pid((_A), (_B), (_C))
#else
/*typedef	pid_t	THREAD_PID; // no use */
#define	THREAD_PID_INIT_VALUE	-1
#define RT_GET_OS_PID(_x, _pid)		(_x = _pid)
#define RTMP_GET_OS_PID(_x, _pid)	(_x = _pid)
#define	GET_PID_NUMBER(_v)	(_v)
#define CHECK_PID_LEGALITY(_pid)	((_pid) >= 0)
#define KILL_THREAD_PID(_A, _B, _C)	kill_proc((_A), (_B), (_C))
#endif

#define ATE_KILL_THREAD_PID(PID)		KILL_THREAD_PID(PID, SIGTERM, 1)

typedef int (*cast_fn)(void *);
typedef INT (*RTMP_OS_TASK_CALLBACK)(ULONG);

#ifdef WORKQUEUE_BH
typedef struct work_struct OS_NET_TASK_STRUCT;
typedef struct work_struct *POS_NET_TASK_STRUCT;
#else
typedef struct tasklet_struct OS_NET_TASK_STRUCT;
typedef struct tasklet_struct  *POS_NET_TASK_STRUCT;
#endif /* WORKQUEUE_BH */

/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
#define OS_HZ			HZ

typedef struct timer_list	OS_NDIS_MINIPORT_TIMER;
typedef struct timer_list	OS_TIMER;

typedef void (*TIMER_FUNCTION)(unsigned long);


#define OS_WAIT(_time) \
	{	\
		if (in_interrupt()) \
			RtmpusecDelay(_time * 1000);\
		else { \
			int _i; \
			long _loop = ((_time)/(1000/OS_HZ)) > 0 ? ((_time)/(1000/OS_HZ)) : 1;\
			wait_queue_head_t _wait; \
			init_waitqueue_head(&_wait); \
			for (_i = 0; _i < (_loop); _i++) \
				wait_event_interruptible_timeout(_wait, 0, ONE_TICK); \
		} \
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#define RTMP_TIME_AFTER(a, b)		\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(b) - (long)(a) < 0))

#define RTMP_TIME_AFTER_EQ(a, b)	\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(a) - (long)(b) >= 0))
#define RTMP_TIME_BEFORE(a, b)	RTMP_TIME_AFTER_EQ(b, a)
#else
#define typecheck(type, x) \
	({      type __dummy; \
		typeof(x) __dummy2; \
		(void)(&__dummy == &__dummy2); \
		1; \
	})
#define RTMP_TIME_AFTER_EQ(a, b)	\
	(typecheck(unsigned long, (unsigned long)a) && \
	 typecheck(unsigned long, (unsigned long)b) && \
	 ((long)(a) - (long)(b) >= 0))
#define RTMP_TIME_BEFORE(a, b)	RTMP_TIME_AFTER_EQ(b, a)
#define RTMP_TIME_AFTER(a, b) time_after(a, b)
#endif

#define ONE_TICK 1

static inline void NdisGetSystemUpTime(ULONG *time)
{
	*time = jiffies;
}


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

struct os_cookie {
#ifdef RTMP_MAC_PCI
	PPCI_DEV pci_dev;
	PPCI_DEV parent_pci_dev;
	USHORT DeviceID;
#endif /* RTMP_MAC_PCI */




	struct device *pDev;
	UINT32 pAd_va;


#if defined(RTMP_MAC_PCI) || defined(RTMP_MAC_USB)
	RTMP_NET_TASK_STRUCT rx_done_task[2];
	RTMP_NET_TASK_STRUCT cmd_rsp_event_task;
	RTMP_NET_TASK_STRUCT tx_dma_done_task;
	RTMP_NET_TASK_STRUCT tr_done_task;

#ifdef ERR_RECOVERY
	RTMP_NET_TASK_STRUCT mac_error_recovey_task;
#endif /* ERR_RECOVERY */

#ifdef CONFIG_FWOWN_SUPPORT
	RTMP_NET_TASK_STRUCT mt_mac_fw_own_task;
#endif /* CONFIG_FWOWN_SUPPORT */

#ifdef MULTI_LAYER_INTERRUPT
	RTMP_NET_TASK_STRUCT subsys_int_task;
#endif

#ifdef UAPSD_SUPPORT
	RTMP_NET_TASK_STRUCT uapsd_eosp_sent_task;
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

#ifdef CARRIER_DETECTION_SUPPORT
	RTMP_NET_TASK_STRUCT carrier_sense_task;
#endif /* CARRIER_DETECTION_SUPPORT */


#endif /* CONFIG_AP_SUPPORT */

#endif

	RTMP_OS_PID			apd_pid; /*802.1x daemon pid */
	unsigned long			apd_pid_nr;
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	/*	RT_SIGNAL_STRUC			RTSignal; */
	RTMP_OS_PID			IappPid; /*IAPP daemon pid */
	unsigned long			IappPid_nr;
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	INT						ioctl_if_type;
	INT					ioctl_if;
	struct _SECURITY_CONFIG *pSecConfig;
#ifdef FW_DUMP_SUPPORT
	struct proc_dir_entry *proc_fwdump_dir;
	struct proc_dir_entry *proc_fwdump_file;
	CHAR fwdump_dir_name[11];
#endif

	struct workqueue_struct *hif_wq;
	struct work_struct tx_dma_done_work;
	struct work_struct rx_done_work[2];
	struct work_struct mac_recovery_work;
	struct work_struct mac_fw_own_work;
#ifdef MULTI_LAYER_INTERRUPT
	struct work_struct subsys_int_work;
#endif
};

typedef struct os_cookie	*POS_COOKIE;



/***********************************************************************************
 *	OS debugging and printing related definitions and data structure
 ***********************************************************************************/
#define MTWF_PRINT	printk

#undef  ASSERT
#ifdef DBG
#define ASSERT(x)                                                               \
	{                                                                               \
		if (!(x)) {                                                                   \
			printk(KERN_WARNING __FILE__ ":%d assert " #x "failed\n", __LINE__);    \
			dump_stack();\
			/* panic("Unexpected error occurs!\n");					*/\
		}                                                                           \
	}
#else
#define ASSERT(x)
#endif /* DBG */


/*********************************************************************************************************
	The following code are not revised, temporary put it here.
  *********************************************************************************************************/


/***********************************************************************************
 * Device DMA Access related definitions and data structures.
 **********************************************************************************/
ra_dma_addr_t linux_pci_map_single(void *handle, void *ptr, size_t size, int sd_idx, int direction);
void linux_pci_unmap_single(void *handle, ra_dma_addr_t dma_addr, size_t size, int direction);

#define PCI_MAP_SINGLE_DEV(_pAd, _ptr, _size, _sd_idx, _dir)				\
	linux_pci_map_single(((POS_COOKIE)(_pAd->OS_Cookie))->pDev, _ptr, _size, _sd_idx, _dir)

#define DMA_MAPPING_ERROR(_handle, _ptr)	\
	dma_mapping_error(&((struct pci_dev *)(_handle))->dev, _ptr)

#define PCI_UNMAP_SINGLE(_pAd, _ptr, _size, _dir)						\
	linux_pci_unmap_single(((POS_COOKIE)(_pAd->OS_Cookie))->pDev, _ptr, _size, _dir)

#define PCI_ALLOC_CONSISTENT(_pci_dev, _size, _ptr) \
	pci_alloc_consistent(_pci_dev, _size, _ptr)

#define PCI_FREE_CONSISTENT(_pci_dev, _size, _virtual_addr, _physical_addr) \
	pci_free_consistent(_pci_dev, _size, _virtual_addr, _physical_addr)

#if !defined(CONFIG_WIFI_PAGE_ALLOC_SKB) && !defined(CONFIG_WIFI_SLAB_ALLOC_SKB)
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)) && defined (CONFIG_MT7621_ASIC)
#define CONFIG_WIFI_SLAB_ALLOC_SKB
#else
#define CONFIG_WIFI_PAGE_ALLOC_SKB
#endif
#else
#define CONFIG_WIFI_PAGE_ALLOC_SKB
#endif

#ifdef CONFIG_WIFI_PAGE_ALLOC_SKB
#define DEV_ALLOC_SKB(_Pkt, _length)		\
	do {\
		_Pkt = dev_alloc_skb(_length);				\
		if (_Pkt != NULL) {	\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);		\
		};	\
	} while (0)
#else	/* CONFIG_WIFI_PAGE_ALLOC_SKB */
#ifndef NET_SKB_PAD
#define NET_SKB_PAD	32
#endif
#define DEV_ALLOC_SKB(_Pkt, _length)		\
	do {\
		_Pkt = alloc_skb(_length + NET_SKB_PAD, GFP_ATOMIC);		\
		if (_Pkt != NULL) {							\
			skb_reserve(_Pkt, NET_SKB_PAD);		\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);	\
		};	\
	} while (0)
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0))
#define CONFIG_WIFI_BUILD_SKB
#define CONFIG_WIFI_PREFETCH_RXDATA
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0)) && defined(CONFIG_ARCH_MT7622)
#define CONFIG_DBG_QDISC
#endif

/*only for debug usage, need to strip on MP release*/
#define CONFIG_DBG_OOM

#define SKB_BUF_HEADROOM_RSV	(NET_SKB_PAD)
#define SKB_BUF_TAILROOM_RSV	(sizeof(struct skb_shared_info))

#define SKB_BUF_HEADTAIL_RSV	(SKB_BUF_HEADROOM_RSV + SKB_BUF_TAILROOM_RSV)
/* Need to do below miniume size protect for build_skb method, to avoid DATAABORT issue. */
#define SKB_BUF_MINIMUN_SIZE	(2048)


#ifdef CONFIG_WIFI_BUILD_SKB
#define DEV_ALLOC_FRAG(_Pkt, _length)		\
	do {\
		_Pkt = netdev_alloc_frag(_length);\
		if (_Pkt != NULL) {	\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);		\
		};	\
	} while (0)

#define DEV_FREE_FRAG_BUF(_Pkt)		\
	do {\
		if (_Pkt != NULL) {	\
			MEM_DBG_PKT_FREE_INC(_Pkt);		\
			put_page(virt_to_head_page(_Pkt));	\
		};	\
	} while (0)

#define DEV_BUILD_SKB(_Pkt, data_ptr, _length)				\
	do {\
		if ((_length) == 0) {	\
			_Pkt = build_skb(data_ptr, 0);	\
		} else {								\
			_Pkt = build_skb(data_ptr, (_length));\
		};	\
	} while (0)

#define DEV_SKB_PTR_ADJUST(_Pkt, _length, _buf_ptr)		\
	do {\
		if (_Pkt != NULL) {						\
			MEM_DBG_PKT_FREE_INC(_buf_ptr);		\
			skb_reserve((_Pkt), (SKB_BUF_HEADROOM_RSV));	\
			skb_put((_Pkt), (_length));		\
			MEM_DBG_PKT_ALLOC_INC(_Pkt);		\
		};	\
	} while (0)

#endif /* CONFIG_WIFI_BUILD_SKB */

/*#define PCI_MAP_SINGLE(_handle, _ptr, _size, _dir) (ULONG)0 */
/*#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir) */


/*
 * ULONG
 * RTMP_GetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressLow(PhysicalAddress)		(PhysicalAddress)

/*
 * ULONG
 * RTMP_GetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressHigh(PhysicalAddress)		(0)

/*
 * VOID
 * RTMP_SetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressLow(PhysicalAddress, Value)	\
	(PhysicalAddress = Value)

/*
 * VOID
 * RTMP_SetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressHigh(PhysicalAddress, Value)

#define NdisMIndicateStatus(_w, _x, _y, _z)



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/
#ifdef RTMP_MAC_PCI
#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)									\
	do {																	\
		(*(_pV) = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
		(*(_pV) = SWAP32(*((UINT32 *)(_pV))));                           \
	} while (0)

#define RTMP_IO_READ32(_A, _R, _pV)									\
	do {																	\
		if ((_A)->bPCIclkOff == FALSE) {                                      \
			(*(_pV) = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
			(*(_pV) = SWAP32(*((UINT32 *)(_pV))));                           \
		}                                                                   \
	} while (0)

#define RTMP_IO_READ8(_A, _R, _pV)									\
	(*(_pV) = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#define _RTMP_IO_WRITE32(_A, _R, _V)									\
	do {																	\
		if ((_A)->bPCIclkOff == FALSE) {                                      \
			UINT32	_Val;													\
			_Val = SWAP32(_V);												\
			writel(_Val, (void *)((_A)->PciHif.CSRBaseAddress + (_R)));			\
		}                                                                   \
	} while (0)

#ifdef INF_VR9
#define RTMP_IO_WRITE8(_A, _R, _V)            \
	do {                    \
		ULONG Val;                \
		UCHAR _i;                \
		UINT32 _Val;		\
		_i = ((_R) & 0x3);             \
		Val = readl((void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));   \
		Val = SWAP32(Val);				\
		Val = Val & (~(0x000000ff << ((_i)*8)));         \
		Val = Val | ((ULONG)(_V) << ((_i)*8));         \
		Val = SWAP32(Val);				\
		writel((Val), (void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));    \
	} while (0)
#else
#define RTMP_IO_WRITE8(_A, _R, _V)									\
	writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)))
#endif

#define RTMP_IO_WRITE16(_A, _R, _V)									\
	writew(SWAP16((_V)), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)))

#elif defined(PLATFORM_M_STB)
#define RTMP_IO_READ32(_A, _R, _pV)									\
do{																	\
    if ((_A)->bPCIclkOff == FALSE)                                  \
    {                                                               \
		(pci_read_config_dword(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _pV));	\
    }                                                               \
    else															\
		*(_pV) = 0;													\
}while(0)

#define _RTMP_IO_WRITE32(_A, _R, _V) 								\
do{ 																\
	if ((_A)->bPCIclkOff == FALSE) 									\
		(pci_write_config_dword(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _V));	\
}while(0)

#define RTMP_IO_READ16(_A, _R, _pV)									\
do{ 																\
	if ((_A)->bPCIclkOff == FALSE)									\
	{																\
		(pci_read_config_word(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _pV));	\
	}																\
	else															\
		*(_pV) = 0; 												\
}while(0)

#define RTMP_IO_WRITE16(_A, _R, _V) 								\
do{ 																\
	if ((_A)->bPCIclkOff == FALSE) 									\
		(pci_write_config_word(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _V));	\
}while(0)

#define RTMP_IO_READ8(_A, _R, _pV)									\
do{ 																\
	if ((_A)->bPCIclkOff == FALSE)									\
	{																\
		(pci_read_config_byte(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _pV));	\
	}																\
	else															\
		*(_pV) = 0; 												\
}while(0)

#define RTMP_IO_WRITE8(_A, _R, _V) 									\
do{ 																\
	if ((_A)->bPCIclkOff == FALSE) 									\
		(pci_write_config_byte(((POS_COOKIE)(_A->OS_Cookie))->pci_dev, _R + 0x80000000, _V));	\
}while(0)

#else
#define RTMP_IO_READ32(_A, _R, _pV)								\
	do {																\
		if ((_A)->bPCIclkOff == FALSE)                                  \
			(*(_pV) = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
		else															\
			*(_pV) = 0;													\
	} while (0)

#define RTMP_IO_FORCE_READ32(_A, _R, _pV)							\
	(*(_pV) = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT) || defined(BB_SOC)
#define RTMP_IO_READ8(_A, _R, _V)            \
	do {                    \
		ULONG Val;                \
		UCHAR _i;                \
		_i = ((_R) & 0x3);             \
		Val = readl((void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));   \
		Val = (Val >> ((_i)*8)) & (0x000000ff);         \
		*((PUCHAR)_V) = (UCHAR) Val;				\
	} while (0)
#else
#define RTMP_IO_READ8(_A, _R, _pV)								\
	(*(_pV) = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))))
#endif /* #if defined(BRCM_6358) || defined(RALINK_2880) */

#define _RTMP_IO_WRITE32(_A, _R, _V) \
	do { \
		if ((_A)->bPCIclkOff == FALSE) \
			writel((_V), (void *)((_A)->PciHif.CSRBaseAddress + (_R))); \
	} while (0)

#define RTMP_IO_FORCE_WRITE32(_A, _R, _V)												\
	writel(_V, (void *)((_A)->PciHif.CSRBaseAddress + (_R)))

/* This is actually system IO */
#define RTMP_SYS_IO_READ32(_R, _pV)		\
	(*_pV = readl((void *)(_R)))

#define RTMP_SYS_IO_WRITE32(_R, _V)		\
	writel(_V, (void *)(_R))



#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT) || defined(MT76x2) || defined(BB_SOC)
#define RTMP_IO_WRITE8(_A, _R, _V)            \
	do {                    \
		ULONG Val;                \
		UCHAR _i;                \
		_i = ((_R) & 0x3);             \
		Val = readl((void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));   \
		Val = Val & (~(0x000000ff << ((_i)*8)));         \
		Val = Val | ((ULONG)(_V) << ((_i)*8));         \
		writel((Val), (void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));    \
	} while (0)
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							\
	writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)))
#endif /* #if defined(BRCM_6358) || defined(RALINK_2880) */

#define RTMP_IO_WRITE16(_A, _R, _V)							\
	writew((_V), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)))
#endif /* #if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0) */


#ifndef VENDOR_FEATURE3_SUPPORT
#define RTMP_IO_WRITE32		_RTMP_IO_WRITE32
#endif /* VENDOR_FEATURE3_SUPPORT */
#endif /* RTMP_MAC_PCI */

#ifdef MT_MAC
#define RTMP_MCU_IO_READ32(_A, _R, _pV)	\
	CmdIORead32((_A), (_R), (_pV))
#define RTMP_MCU_IO_WRITE32(_A, _R, _V)	\
	CmdIOWrite32((_A), (_R), (_V))

#endif /* MT_MAC */





#define MAC_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define MAC_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)


#if defined(RTMP_MAC_PCI) && defined(WHNAT_SUPPORT)
#define HIF_IO_READ32(_A, _R, _pV)			pci_io_read32(_A, _R, _pV)
#define HIF_IO_WRITE32(_A, _R, _V)			pci_io_write32(_A, _R, _V)
#else
#define HIF_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define HIF_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)
#endif

#define PHY_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define PHY_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)

#define HW_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define HW_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)

#define MCU_IO_READ32(_A, _R, _pV)			RTMP_MCU_IO_READ32(_A, _R, _pV)
#define MCU_IO_WRITE32(_A, _R, _V)			RTMP_MCU_IO_WRITE32(_A, _R, _V)


#define RTMP_USB_URB_DATA_GET(__pUrb)			(((purbb_t)__pUrb)->context)
#define RTMP_USB_URB_STATUS_GET(__pUrb)			(((purbb_t)__pUrb)->status)
#define RTMP_USB_URB_LEN_GET(__pUrb)			(((purbb_t)__pUrb)->actual_length)

/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/
#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	((_pNetDev)->flags & IFF_UP)

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29)
#define _RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		(NULL /*(_pNetDev)->ml_priv*/)
#else
#define _RTMP_OS_NETDEV_GET_PRIV(_pNetDev)		((_pNetDev)->priv)
#endif

#define RTMP_OS_NETDEV_GET_DEVNAME(_pNetDev)	((_pNetDev)->name)
#define RTMP_OS_NETDEV_GET_PHYADDR(_pNetDev)	((_pNetDev)->dev_addr)

/* Get & Set NETDEV interface hardware type */
#define RTMP_OS_NETDEV_GET_TYPE(_pNetDev)			((_pNetDev)->type)
#define RTMP_OS_NETDEV_SET_TYPE(_pNetDev, _type)	((_pNetDev)->type = (_type))
#ifdef SNIFFER_MT7615
#define RTMP_OS_NETDEV_SET_TYPE_MONITOR(_pNetDev)	RTMP_OS_NETDEV_SET_TYPE(_pNetDev, ARPHRD_IEEE80211_RADIOTAP)
#else
#define RTMP_OS_NETDEV_SET_TYPE_MONITOR(_pNetDev)	RTMP_OS_NETDEV_SET_TYPE(_pNetDev, ARPHRD_IEEE80211_PRISM)
#endif
#define RTMP_OS_NETDEV_START_QUEUE(_pNetDev)	netif_start_queue((_pNetDev))
#define RTMP_OS_NETDEV_STOP_QUEUE(_pNetDev)	netif_stop_queue((_pNetDev))
#define RTMP_OS_NETDEV_WAKE_QUEUE(_pNetDev)	netif_wake_queue((_pNetDev))
#define RTMP_OS_NETDEV_CARRIER_OFF(_pNetDev)	netif_carrier_off((_pNetDev))

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)

#ifdef CONFIG_5VT_ENHANCE
#define BRIDGE_TAG 0x35564252    /* depends on 5VT define in br_input.c */
#endif

#define GET_SG_LIST_FROM_PACKET(_p, _sc)	\
	rt_get_sg_list_from_packet(_p, _sc)

#define RELEASE_NDIS_PACKET(_pAd, _pPacket, _Status)	\
	RTMPFreeNdisPacket(_pAd, _pPacket)


#define RELEASE_NDIS_PACKET_IRQ(_pAd, _pPacket, _Status)	\
	RTMPFreeNdisPacketIRQ(_pAd, _pPacket)


/*
 * packet helper
 *	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
#define RTPKT_TO_OSPKT(_p)		((struct sk_buff *)(_p))
#define OSPKT_TO_RTPKT(_p)		((PNDIS_PACKET)(_p))

#define GET_OS_PKT_DATAPTR(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->data)
#define SET_OS_PKT_DATAPTR(_pkt, _dataPtr)	\
	((RTPKT_TO_OSPKT(_pkt)->data) = (_dataPtr))

#define GET_OS_PKT_LEN(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->len)
#define SET_OS_PKT_LEN(_pkt, _len)	\
	((RTPKT_TO_OSPKT(_pkt)->len) = (_len))

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22))
#define GET_OS_PKT_DATATAIL(_pkt) \
	skb_tail_pointer(RTPKT_TO_OSPKT(_pkt))
#define SET_OS_PKT_DATATAIL(_pkt, _len)	\
	skb_set_tail_pointer(RTPKT_TO_OSPKT(_pkt), _len)
#else
#define GET_OS_PKT_DATATAIL(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->tail)
#define SET_OS_PKT_DATATAIL(_pkt, _len)	\
	((RTPKT_TO_OSPKT(_pkt))->tail) = (PUCHAR)(((RTPKT_TO_OSPKT(_pkt))->data) + (_len))
#endif

#define GET_OS_PKT_HEAD(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->head)

#define GET_OS_PKT_END(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->end)

#define GET_OS_PKT_NETDEV(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->dev)
#define SET_OS_PKT_NETDEV(_pkt, _pNetDev)	\
	((RTPKT_TO_OSPKT(_pkt)->dev) = (_pNetDev))

#define GET_OS_PKT_TYPE(_pkt) \
	(RTPKT_TO_OSPKT(_pkt))

#define GET_OS_PKT_NEXT(_pkt) \
	(RTPKT_TO_OSPKT(_pkt)->next)


#define OS_PKT_CLONED(_pkt)		skb_cloned(RTPKT_TO_OSPKT(_pkt))
#define OS_PKT_COPY(_pkt, _newSkb)  \
	do {\
		_newSkb = skb_copy(RTPKT_TO_OSPKT(_pkt), GFP_ATOMIC);	\
		if (_newSkb != NULL)					\
			MEM_DBG_PKT_ALLOC_INC(_newSkb);			\
	} while (0)

#define OS_PKT_TAIL_ADJUST(_pkt, _removedTagLen)							\
	skb_trim(RTPKT_TO_OSPKT(_pkt), GET_OS_PKT_LEN(_pkt) - _removedTagLen)

#define OS_PKT_HEAD_BUF_EXTEND(_pkt, _offset)								\
	skb_push(RTPKT_TO_OSPKT(_pkt), _offset)

#define OS_PKT_TAIL_BUF_EXTEND(_pkt, _Len)									\
	skb_put(RTPKT_TO_OSPKT(_pkt), _Len)

#define OS_PKT_RESERVE(_pkt, _Len)											\
	skb_reserve(RTPKT_TO_OSPKT(_pkt), _Len)

#define RTMP_OS_PKT_INIT(__pRxPacket, __pNetDev, __pData, __DataSize)		\
	{																			\
		PNDIS_PACKET __pRxPkt;													\
		__pRxPkt = RTPKT_TO_OSPKT(__pRxPacket);									\
		SET_OS_PKT_NETDEV(__pRxPkt, __pNetDev);									\
		SET_OS_PKT_DATAPTR(__pRxPkt, __pData);									\
		SET_OS_PKT_LEN(__pRxPkt, __DataSize);									\
		SET_OS_PKT_DATATAIL(__pRxPkt, __DataSize);								\
	}

#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	do {\
		_src = skb_clone(RTPKT_TO_OSPKT(_pkt), _flag);	\
		if (_src != NULL)				\
			MEM_DBG_PKT_ALLOC_INC(_src);		\
	} while (0)

#define get_unaligned32							get_unaligned
#define get_unalignedlong						get_unaligned

#define OS_NTOHS(_Val) \
	(ntohs((_Val)))
#define OS_HTONS(_Val) \
	(htons((_Val)))
#define OS_NTOHL(_Val) \
	(ntohl((_Val)))
#define OS_HTONL(_Val) \
	(htonl((_Val)))

#define CB_OFF  10
#define CB_LEN	36
#define GET_OS_PKT_CB(_p)		(RTPKT_TO_OSPKT(_p)->cb)
#define PACKET_CB(_p, _offset)	((RTPKT_TO_OSPKT(_p)->cb[CB_OFF + (_offset)]))




/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/

#ifdef CONFIG_FAST_NAT_SUPPORT
extern int (*ra_sw_nat_hook_tx)(struct sk_buff *skb, int gmac_no);
extern int (*ra_sw_nat_hook_rx)(struct sk_buff *skb);
extern void (*ppe_dev_register_hook) (VOID  *dev);
extern void (*ppe_dev_unregister_hook) (VOID  *dev);

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
extern int (*wf_ra_sw_nat_hook_tx_bkup)(struct sk_buff *skb, int gmac_no);
extern int (*wf_ra_sw_nat_hook_rx_bkup)(struct sk_buff *skb);
#endif
#endif /*CONFIG_FAST_NAT_SUPPORT*/

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT {
	struct net_device *rcvd_net_dev;
	unsigned char	src_addr[ETH_ALEN];
	unsigned char	entry_from;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pBefore;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pNext;
};

struct APCLI_BRIDGE_LEARNING_MAPPING_MAP {
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pHead;
	struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT *pTail;
	unsigned int entry_num;
};

extern int (*wf_fwd_tx_hook)(struct sk_buff *skb);
extern int (*wf_fwd_rx_hook)(struct sk_buff *skb);
extern unsigned char (*wf_fwd_entry_insert_hook)
	(struct net_device *src, struct net_device *dest, void *adapter);
extern unsigned char (*wf_fwd_entry_delete_hook)
	(struct net_device *src, struct net_device *dest, unsigned char link_down);

extern void (*wf_fwd_check_device_hook)
	(struct net_device *net_dev, signed int type, signed int mbss_idx, unsigned char channel, unsigned char link);

extern void (*wf_fwd_set_cb_num)(unsigned int band_cb_num, unsigned int receive_cb_num);
extern bool (*wf_fwd_check_active_hook)(void);

extern void (*wf_fwd_get_rep_hook)(unsigned char idx);
extern void (*wf_fwd_pro_active_hook)(void);
extern void (*wf_fwd_pro_halt_hook)(void);
extern void (*wf_fwd_pro_enabled_hook)(void);
extern void (*wf_fwd_pro_disabled_hook)(void);
extern void (*wf_fwd_access_schedule_active_hook)(void);
extern void (*wf_fwd_access_schedule_halt_hook)(void);
extern void (*wf_fwd_hijack_active_hook)(void);
extern void (*wf_fwd_hijack_halt_hook)(void);
extern void (*wf_fwd_bpdu_active_hook)(void);
extern void (*wf_fwd_bpdu_halt_hook)(void);
extern void (*wf_fwd_show_entry_hook)(void);
extern void (*wf_fwd_delete_entry_hook)(unsigned char idx);
extern void (*packet_source_show_entry_hook)(void);
extern void (*packet_source_delete_entry_hook)(unsigned char idx);
extern void (*wf_fwd_feedback_map_table)(void *adapter, void *peer, void *opp_peer, void *oth_peer);
extern void (*wf_fwd_probe_adapter)(void *adapter);
extern void (*wf_fwd_probe_apcli_device_hook)(void *device);
extern void (*wf_fwd_insert_bridge_mapping_hook)(struct sk_buff *skb);
extern void (*wf_fwd_insert_repeater_mapping_hook)
	(void *apcli_device, void *lock, void *cli_mapping, void *map_mapping, void *ifAddr_mapping);
extern int (*wf_fwd_search_mapping_table_hook)
	(struct sk_buff *skb, struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT **tbl_entry);
extern void (*wf_fwd_delete_entry_inform_hook)(unsigned char *addr);
extern bool (*wf_fwd_needed_hook) (void);
extern void (*wf_fwd_add_entry_inform_hook)(unsigned char *addr);
extern void (*wf_fwd_set_bridge_hook) (unsigned char* bridge_intf_name,unsigned int len);

extern void (*wf_fwd_probe_dev_hook)(struct net_device *net_dev_p);
#endif /* CONFIG_WIFI_PKT_FWD */

#if defined(CONFIG_WIFI_PKT_FWD) || defined(CONFIG_WIFI_PKT_FWD_MODULE)
struct wifi_fwd_func_table{
	int (*wf_fwd_rx_hook) (struct sk_buff *skb);
	int (*wf_fwd_tx_hook) (struct sk_buff *skb);
	unsigned char (*wf_fwd_entry_insert_hook) (struct net_device *src, struct net_device *dest, void *adapter);
	unsigned char (*wf_fwd_entry_delete_hook) (struct net_device *src, struct net_device *dest, unsigned char link_down);
	void (*wf_fwd_set_cb_num) (unsigned int band_cb_num, unsigned int receive_cb_num);
	void (*wf_fwd_set_debug_level_hook) (unsigned char level);
	bool (*wf_fwd_check_active_hook) (void);
	void (*wf_fwd_get_rep_hook) (unsigned char idx);
	void (*wf_fwd_pro_active_hook) (void);
	void (*wf_fwd_pro_halt_hook) (void);
	void (*wf_fwd_pro_enabled_hook) (void);
	void (*wf_fwd_pro_disabled_hook) (void);
	void (*wf_fwd_access_schedule_active_hook) (void);
	void (*wf_fwd_access_schedule_halt_hook) (void);
	void (*wf_fwd_hijack_active_hook) (void);
	void (*wf_fwd_hijack_halt_hook) (void);
	void (*wf_fwd_bpdu_active_hook) (void);
	void (*wf_fwd_bpdu_halt_hook) (void);
	void (*wf_fwd_show_entry_hook) (void);
	bool (*wf_fwd_needed_hook) (void);
	void (*wf_fwd_delete_entry_hook) (unsigned char idx);
	void (*packet_source_show_entry_hook) (void);
	void (*packet_source_delete_entry_hook) (unsigned char idx);
	void (*wf_fwd_feedback_map_table) (void *adapter, void **peer, void **opp_peer, void **oth_peer);
	void (*wf_fwd_probe_adapter) (void *adapter);
	void (*wf_fwd_remove_adapter)(void *adapter);
	void (*wf_fwd_insert_repeater_mapping_hook)(void *adapter, void *lock, void *cli_mapping, void *map_mapping, void *ifAddr_mapping);
	void (*wf_fwd_delete_repeater_mapping) (void *apcli_device);
	void (*wf_fwd_insert_bridge_mapping_hook) (struct sk_buff *skb);
	int (*wf_fwd_search_mapping_table_hook) (struct sk_buff *skb, struct APCLI_BRIDGE_LEARNING_MAPPING_STRUCT **tbl_entry);
	void (*wf_fwd_delete_entry_inform_hook) (unsigned char *addr);
	void (*wf_fwd_check_device_hook) (struct net_device *net_dev, signed int type, signed int mbss_idx, unsigned char channel, unsigned char link);
	void (*wf_fwd_add_entry_inform_hook) (unsigned char *addr);
	void (*wf_fwd_debug_level_hook) (unsigned char level);
	void (*wf_fwd_set_easy_setup_mode) (unsigned int easy_setup_mode);
	void (*wf_fwd_set_bridge_hook) (unsigned char* bridge_intf_name,unsigned int len);
};

#endif


void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);
int rt28xx_packet_xmit(VOID *skb);

#ifdef RTMP_RBUS_SUPPORT
#ifndef CONFIG_RALINK_FLASH_API
void FlashWrite(UCHAR *p, ULONG a, ULONG b);
void FlashRead(UCHAR *p, ULONG a, ULONG b);
#endif /* CONFIG_RALINK_FLASH_API */

int wl_proc_init(void);
int wl_proc_exit(void);

#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
extern volatile unsigned long classifier_cur_cycle;
extern int (*ra_classifier_hook_rx) (struct sk_buff *skb, unsigned long cycle);
#endif /* defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE) */
#endif /* RTMP_RBUS_SUPPORT */

#if LINUX_VERSION_CODE <= 0x20402	/* Red Hat 7.1 */
struct net_device *alloc_netdev(int sizeof_priv, const char *mask, void (*setup)(struct net_device *));
#endif /* LINUX_VERSION_CODE */


#ifdef RTMP_MAC_PCI
/* function declarations */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#define IRQ_HANDLE_TYPE  irqreturn_t
#else
#define IRQ_HANDLE_TYPE  void
#endif

IRQ_HANDLE_TYPE
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19))
rt2860_interrupt(int irq, void *dev_instance);
#else
rt2860_interrupt(int irq, void *dev_instance, struct pt_regs *regs);
#endif

#endif /* RTMP_MAC_PCI */

INT rt28xx_ioctl(PNET_DEV net_dev, struct ifreq *rq, INT cmd);
int rt28xx_send_packets(struct sk_buff *skb, struct net_device *ndev);

extern int ra_mtd_write(int num, loff_t to, size_t len, const u_char *buf);
extern int ra_mtd_read(int num, loff_t from, size_t len, u_char *buf);


#define GET_PAD_FROM_NET_DEV(_pAd, _net_dev)						\
	(_pAd = RTMP_OS_NETDEV_GET_PRIV(_net_dev))
#define GET_WDEV_FROM_NET_DEV(_wdev, _net_dev)	\
	(_wdev = RTMP_OS_NETDEV_GET_WDEV(_net_dev))


/*#ifdef RTMP_USB_SUPPORT */
/******************************************************************************

	USB related definitions

******************************************************************************/

#define RTMP_USB_PKT_COPY(__pNetDev, __pNetPkt, __Len, __pData)			\
	do {\
		memcpy(skb_put(__pNetPkt, __Len), __pData, __Len);					\
		GET_OS_PKT_NETDEV(__pNetPkt) = __pNetDev;							\
	} while (0)

typedef struct usb_device_id USB_DEVICE_ID;

#define BULKAGGRE_SIZE				100 /* 100 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)

#ifndef OS_ABL_SUPPORT
#define RTUSB_ALLOC_URB(iso)		usb_alloc_urb(iso, GFP_ATOMIC)
#ifdef USB_IOT_WORKAROUND2
void usb_iot_add_padding(struct urb *urb, UINT8 *buf, ra_dma_addr_t dma);
#endif
#define RTUSB_SUBMIT_URB(pUrb)		usb_submit_urb(pUrb, GFP_ATOMIC)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 35)
#define RTUSB_URB_ALLOC_BUFFER(_dev, _size, _dma)	usb_alloc_coherent(_dev, _size, GFP_ATOMIC, _dma)
#define RTUSB_URB_FREE_BUFFER(_dev, _size, _addr, _dma)	usb_free_coherent(_dev, _size, _addr, _dma)
#else
#define RTUSB_URB_ALLOC_BUFFER(_dev, _size, _dma)	usb_buffer_alloc(_dev, _size, GFP_ATOMIC, _dma)
#define RTUSB_URB_FREE_BUFFER(_dev, _size, _addr, _dma)	usb_buffer_free(_dev, _size, _addr, _dma)
#endif

#define RTUSB_FILL_BULK_URB(_urb, _dev, _pipe, _buffer, _buffer_len, _complete_fn, _context) usb_fill_bulk_urb(_urb, _dev, _pipe, _buffer, _buffer_len, _complete_fn, _context)
#else

#define RTUSB_ALLOC_URB(iso)		rausb_alloc_urb(iso)
#define RTUSB_SUBMIT_URB(pUrb)		rausb_submit_urb(pUrb)
#define RTUSB_URB_ALLOC_BUFFER		rausb_buffer_alloc
#define RTUSB_URB_FREE_BUFFER		rausb_buffer_free
#endif /* OS_ABL_SUPPORT */

#else

#define RT28XX_PUT_DEVICE(dev_p)

#ifndef OS_ABL_SUPPORT
#define RTUSB_ALLOC_URB(iso)		usb_alloc_urb(iso)
#define RTUSB_SUBMIT_URB(pUrb)		usb_submit_urb(pUrb)
#else
#define RTUSB_ALLOC_URB(iso)		rausb_alloc_urb(iso)
#define RTUSB_SUBMIT_URB(pUrb)		rausb_submit_urb(pUrb)
#endif /* OS_ABL_SUPPORT */

#define RTUSB_URB_ALLOC_BUFFER(pUsb_Dev, BufSize, pDma_addr)			kmalloc(BufSize, GFP_ATOMIC)
#define RTUSB_URB_FREE_BUFFER(pUsb_Dev, BufSize, pTransferBuf, Dma_addr)	kfree(pTransferBuf)
#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) */


#ifndef OS_ABL_SUPPORT
#define RTUSB_FREE_URB(pUrb)	usb_free_urb(pUrb)
#else
#define RTUSB_FREE_URB(pUrb)	rausb_free_urb(pUrb)
#endif /* OS_ABL_SUPPORT */

/* unlink urb */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 7)

#ifndef OS_ABL_SUPPORT
#define RTUSB_UNLINK_URB(pUrb)		usb_kill_urb(pUrb)
#else
#define RTUSB_UNLINK_URB(pUrb)		rausb_kill_urb(pUrb)
#endif /* OS_ABL_SUPPORT */

#else
#define RTUSB_UNLINK_URB(pUrb)		usb_unlink_urb(pUrb)
#endif /* LINUX_VERSION_CODE */

/* Prototypes of completion funuc. */
#define RtmpUsbBulkOutDataPacketComplete		RTUSBBulkOutDataPacketComplete
#define RtmpUsbBulkOutMLMEPacketComplete		RTUSBBulkOutMLMEPacketComplete
#define RtmpUsbBulkOutNullFrameComplete			RTUSBBulkOutNullFrameComplete
#define RtmpUsbBulkOutRTSFrameComplete			RTUSBBulkOutRTSFrameComplete
#define RtmpUsbBulkOutPsPollComplete			RTUSBBulkOutPsPollComplete
#define RtmpUsbBulkRxComplete				RTUSBBulkRxComplete
#define RtmpUsbBulkCmdRspEventComplete			RTUSBBulkCmdRspEventComplete
#ifdef MT_MAC
#define RtmpUsbBulkOutBCNPacketComplete			RTUSBBulkOutBCNPacketComplete
#endif

#if ((LINUX_VERSION_CODE < KERNEL_VERSION(2, 5, 51)) || (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 18)))
#define RTUSBBulkOutDataPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutDataPacketComplete(pURB)
#define RTUSBBulkOutMLMEPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutMLMEPacketComplete(pURB)
#define RTUSBBulkOutNullFrameComplete(Status, pURB, pt_regs)	RTUSBBulkOutNullFrameComplete(pURB)
#define RTUSBBulkOutRTSFrameComplete(Status, pURB, pt_regs)	RTUSBBulkOutRTSFrameComplete(pURB)
#define RTUSBBulkOutPsPollComplete(Status, pURB, pt_regs)	RTUSBBulkOutPsPollComplete(pURB)
#define RTUSBBulkRxComplete(Status, pURB, pt_regs)		RTUSBBulkRxComplete(pURB)
#define RTUSBBulkCmdRspEventComplete(Status, pURB, pt_regs)	RTUSBBulkCmdRspEventComplete(pURB)
#ifdef MT_MAC
#define RTUSBBulkOutBCNPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutBCNPacketComplete(pURB)
#endif
#else
#define RTUSBBulkOutDataPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutDataPacketComplete(pURB, pt_regs)
#define RTUSBBulkOutMLMEPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutMLMEPacketComplete(pURB, pt_regs)
#define RTUSBBulkOutNullFrameComplete(Status, pURB, pt_regs)	RTUSBBulkOutNullFrameComplete(pURB, pt_regs)
#define RTUSBBulkOutRTSFrameComplete(Status, pURB, pt_regs)	RTUSBBulkOutRTSFrameComplete(pURB, pt_regs)
#define RTUSBBulkOutPsPollComplete(Status, pURB, pt_regs)	RTUSBBulkOutPsPollComplete(pURB, pt_regs)
#define RTUSBBulkRxComplete(Status, pURB, pt_regs)		RTUSBBulkRxComplete(pURB, pt_regs)
#define RTUSBBulkCmdRspEventComplete(Status, pURB, pt_regs)	RTUSBBulkCmdRspEventComplete(pURB, pt_regs)
#ifdef MT_MAC
#define RTUSBBulkOutBCNPacketComplete(Status, pURB, pt_regs)	RTUSBBulkOutBCNPacketComplete(pURB, pt_regs)
#endif
#endif

/*extern void dump_urb(struct urb *purb); */

#define InterlockedIncrement 		atomic_inc
#define NdisInterlockedIncrement	atomic_inc
#define InterlockedDecrement		atomic_dec
#define NdisInterlockedDecrement	atomic_dec
#define InterlockedExchange			atomic_set

typedef void USBHST_STATUS;
typedef INT32 URBCompleteStatus;
typedef struct pt_regs pregs;

USBHST_STATUS RTUSBBulkOutDataPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkOutMLMEPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkOutNullFrameComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkOutRTSFrameComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkOutPsPollComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkRxComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
USBHST_STATUS RTUSBBulkCmdRspEventComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
#ifdef MT_MAC
USBHST_STATUS RTUSBBulkOutBCNPacketComplete(URBCompleteStatus Status, purbb_t pURB, pregs *pt_regs);
#endif

/* Fill Bulk URB Macro */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#define RTUSB_FILL_TX_BULK_URB(pUrb,	\
							   pUsb_Dev,	\
							   uEndpointAddress,		\
							   pTransferBuf,			\
							   BufSize,				\
							   Complete,	\
							   pContext,		\
							   TransferDma)	\
do {	\
	usb_fill_bulk_urb(pUrb, pUsb_Dev, usb_sndbulkpipe(pUsb_Dev, uEndpointAddress),	\
					  pTransferBuf, BufSize, Complete, pContext);	\
	pUrb->transfer_dma	= TransferDma;	\
	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;	\
} while (0)

#define RTUSB_FILL_HTTX_BULK_URB(pUrb,	\
								 pUsb_Dev,	\
								 uEndpointAddress,		\
								 pTransferBuf,			\
								 BufSize,				\
								 Complete,	\
								 pContext,				\
								 TransferDma)				\
do {	\
	usb_fill_bulk_urb(pUrb, pUsb_Dev, usb_sndbulkpipe(pUsb_Dev, uEndpointAddress),	\
					  pTransferBuf, BufSize, Complete, pContext);	\
	pUrb->transfer_dma	= TransferDma; \
	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;	\
} while (0)

#define RTUSB_FILL_RX_BULK_URB(pUrb,	\
							   pUsb_Dev,				\
							   uEndpointAddress,		\
							   pTransferBuf,			\
							   BufSize,				\
							   Complete,				\
							   pContext,				\
							   TransferDma)			\
do {	\
	usb_fill_bulk_urb(pUrb, pUsb_Dev, usb_rcvbulkpipe(pUsb_Dev, uEndpointAddress),	\
					  pTransferBuf, BufSize, Complete, pContext);	\
	pUrb->transfer_dma	= TransferDma;	\
	pUrb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;	\
} while (0)
/* pRxContext->data_dma + pAd->NextRxBulkInPosition; */

#define RTUSB_URB_DMA_MAPPING(pUrb)	\
	{	\
		pUrb->transfer_dma	= 0;	\
		pUrb->transfer_flags &= (~URB_NO_TRANSFER_DMA_MAP);	\
	}

#else
#define RTUSB_FILL_TX_BULK_URB(pUrb, pUsb_Dev, uEndpointAddress,	\
	pTransferBuf, BufSize, Complete, pContext, TransferDma)		\
	FILL_BULK_URB(pUrb, pUsb_Dev,	\
	usb_sndbulkpipe(pUsb_Dev, uEndpointAddress),	\
	pTransferBuf, BufSize, Complete, pContext)

#define RTUSB_FILL_HTTX_BULK_URB(pUrb, pUsb_Dev, uEndpointAddress,	\
	pTransferBuf, BufSize, Complete, pContext, TransferDma)		\
	FILL_BULK_URB(pUrb, pUsb_Dev,	\
	usb_sndbulkpipe(pUsb_Dev, uEndpointAddress),	\
	pTransferBuf, BufSize, Complete, pContext)

#define RTUSB_FILL_RX_BULK_URB(pUrb, pUsb_Dev, uEndpointAddress,	\
	pTransferBuf, BufSize,	Complete, pContext, TransferDma)	\
	FILL_BULK_URB(pUrb, pUsb_Dev,	\
		usb_rcvbulkpipe(pUsb_Dev, uEndpointAddress),	\
		pTransferBuf, BufSize, Complete, pContext)

#define	RTUSB_URB_DMA_MAPPING(pUrb)
#endif


#define RTUSB_CONTROL_MSG(pUsb_Dev, uEndpointAddress, Request, RequestType, Value, Index, tmpBuf, TransferBufferLength, timeout, ret)	\
	do {	\
		if ((RequestType == DEVICE_VENDOR_REQUEST_OUT) || (RequestType == DEVICE_CLASS_REQUEST_OUT))	\
			ret = USB_CONTROL_MSG(pUsb_Dev, usb_sndctrlpipe(pUsb_Dev, uEndpointAddress), Request, RequestType, Value, Index, tmpBuf, TransferBufferLength, timeout);	\
		else if (RequestType == DEVICE_VENDOR_REQUEST_IN)	\
			ret = USB_CONTROL_MSG(pUsb_Dev, usb_rcvctrlpipe(pUsb_Dev, uEndpointAddress), Request, RequestType, Value, Index, tmpBuf, TransferBufferLength, timeout);	\
		else {	\
			MTWF_LOG(DBG_CAT_HIF, CATHIF_USB, DBG_LVL_ERROR, ("vendor request direction is failed\n"));	\
			ret = -1;	\
		}	\
	} while (0)

#define rtusb_urb_context  context
#define rtusb_urb_status   status
#define rtusb_urb_transfer_len transfer_buffer_length
#define rtusb_urb_actual_len actual_length

#define RTMP_OS_USB_CONTEXT_GET(__pURB)		((__pURB)->rtusb_urb_context)
#define RTMP_OS_USB_STATUS_GET(__pURB)		((__pURB)->rtusb_urb_status)
#define RTMP_OS_USB_TRANSFER_LEN_GET(__pURB)	((__pURB)->rtusb_urb_transfer_len)
#define RTMP_OS_USB_ACTUAL_LEN_GET(__pURB)	((__pURB)->rtusb_urb_actual_len)

#ifndef OS_ABL_SUPPORT
#define USB_CONTROL_MSG		usb_control_msg

#else

#define USB_CONTROL_MSG		rausb_control_msg

/*extern int rausb_register(struct usb_driver * new_driver); */
/*extern void rausb_deregister(struct usb_driver * driver); */

extern struct urb *rausb_alloc_urb(int iso_packets);
extern void rausb_free_urb(VOID *urb);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
extern void rausb_put_dev(VOID *dev);
extern struct usb_device *rausb_get_dev(VOID *dev);
#endif /* LINUX_VERSION_CODE */

extern int rausb_submit_urb(VOID *urb);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 5, 0)
#ifndef gfp_t
#define gfp_t		INT32
#endif /* gfp_t */

extern void *rausb_buffer_alloc(VOID *dev,
								size_t size,
								ra_dma_addr_t *dma);
extern void rausb_buffer_free(VOID *dev,
							  size_t size,
							  void *addr,
							  ra_dma_addr_t dma);
#endif /* LINUX_VERSION_CODE */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 7)
extern void rausb_kill_urb(VOID *urb);
#endif /* LINUX_VERSION_CODE */

extern int rausb_control_msg(VOID *dev,
							 unsigned int pipe,
							 __u8 request,
							 __u8 requesttype,
							 __u16 value,
							 __u16 index,
							 void *data,
							 __u16 size,
							 int timeout);

#endif /* OS_ABL_SUPPORT */

/*#endif // RTMP_USB_SUPPORT */

#ifdef CONFIG_ATE
/******************************************************************************

	ATE related definitions

******************************************************************************/
#define ate_print printk
#ifdef RTMP_MAC_PCI
#ifdef CONFIG_AP_SUPPORT
#define EEPROM_BIN_FILE_NAME  "/etc/Wireless/RT2860AP/e2p.bin"
#endif /* CONFIG_AP_SUPPORT */
#endif /* RTMP_MAC_PCI */






#endif /* CONFIG_ATE */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 31)
INT RtmpOSNetDevOpsAlloc(
	IN PVOID * pNetDevOps);
#endif

#define RTMP_OS_MAX_SCAN_DATA_GET()		IW_SCAN_MAX_DATA

#include "os/rt_os.h"


#ifdef MULTI_INF_SUPPORT
#ifdef RTMP_PCI_SUPPORT
int __init rt_pci_init_module(void);
void __exit rt_pci_cleanup_module(void);
#endif /* RTMP_PCI_SUPPORT */


#ifdef RTMP_RBUS_SUPPORT
int __init wbsys_module_init(void);
VOID __exit wbsys_module_exit(void);
#endif /* RTMP_RBUS_SUPPORT */

int multi_inf_adapt_reg(VOID *pAd);
int multi_inf_adapt_unreg(VOID *pAd);
int multi_inf_get_idx(VOID *pAd);
#endif /* MULTI_INF_SUPPORT */

struct device *rtmp_get_dev(void *ad);

#define RA_WEXT	0
#define RA_NETLINK	1

#ifdef FW_DUMP_SUPPORT
BOOLEAN FWDumpProcInit(VOID *pAd);
BOOLEAN FWDumpProcCreate(VOID *pAd, PCHAR suffix);
VOID FWDumpProcRemove(VOID *pAd);
INT os_set_fwdump_path(VOID *pAd, VOID *arg);
#define RTMP_OS_FWDUMP_PROCINIT(ad)	FWDumpProcInit(ad)
#define RTMP_OS_FWDUMP_PROCCREATE(ad, _suffix)	FWDumpProcCreate(ad, _suffix)
#define RTMP_OS_FWDUMP_PROCREMOVE(ad)	FWDumpProcRemove(ad)
#define RTMP_OS_FWDUMP_SETPATH(ad, ar)	os_set_fwdump_path(ad, ar)
#endif

#define OS_TRACE __builtin_return_address(0)

typedef struct kref os_kref;


#ifdef DBG_STARVATION
#define STARV_ENTRY_SIZE 2000

struct starv_log_entry;
struct starv_dbg;
struct starv_dbg_block;

struct starv_dbg {
	unsigned int msec;
	unsigned long start_jiffies;
	unsigned long end_jiffies;
	struct starv_dbg_block *block;
};

struct starv_dbg_block {
	char name[32];
	unsigned int timeout;
	struct starv_log *ctrl;
	void (*timeout_fn)(struct starv_dbg *starv, struct starv_log_entry *entry);
	void (*leave_fn)(struct starv_dbg *starv);
	void (*entry_fn)(struct starv_dbg *starv);
	void (*log_fn)(struct starv_log_entry *entry);
	void *priv;
	/*statistic*/
	unsigned int avg_dur;
	unsigned int max_dur;
	unsigned int min_dur;
	unsigned int timeout_cnt;
	unsigned int count;
	DL_LIST log_head;
	DL_LIST list;
};

struct starv_log_basic {
	unsigned int id;
	unsigned int qsize;
};

struct starv_log_entry {
	void *log;
	unsigned int duration;
	struct starv_dbg_block *block;
	DL_LIST list;
};

struct starv_log {
	struct starv_log_entry pool[STARV_ENTRY_SIZE];
	DL_LIST free;
	DL_LIST block_head;
	NDIS_SPIN_LOCK lock;
};

VOID starv_dbg_init(struct starv_dbg_block *block, struct starv_dbg *starv);
VOID starv_dbg_get(struct starv_dbg *starv);
VOID starv_dbg_put(struct starv_dbg *starv);
VOID starv_log_dump(struct starv_log *ctrl);
INT starv_log_init(struct starv_log *ctrl);
VOID starv_log_exit(struct starv_log *ctrl);
INT register_starv_block(struct starv_dbg_block *bk);
VOID unregister_starv_block(struct starv_dbg_block *bk);
VOID starv_timeout_log_basic(struct starv_log_entry *entry);

#endif /*DBG_STARVATION*/


#ifdef CONFIG_DBG_QDISC
void os_system_tx_queue_dump(PNET_DEV dev);
#endif /*CONFIG_DBG_QDISC*/

#endif /* __RT_LINUX_H__ */

