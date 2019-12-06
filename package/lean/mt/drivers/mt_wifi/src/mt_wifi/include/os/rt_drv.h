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
	rt_drv.h

    Abstract:
	Put all virtual OS related definition/structure/MACRO here except
	standard ANSI C function.

	Note:
	No any OS related definition/MACRO is defined here.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
*/

/* same as rt_linux.h to warn users the two files can not be used simultaneously */
#ifndef __RT_LINUX_H__
#define __RT_LINUX_H__

#include "os/rt_linux_cmm.h"
#include <linux/string.h>
#include <linux/ctype.h>

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

/*#ifdef RTMP_USB_SUPPORT */
typedef VOID * PUSB_DEV;
typedef VOID * purbb_t;
typedef VOID	pregs;
/*typedef struct usb_ctrlrequest devctrlrequest; */
/*#endif */

/***********************************************************************************
 *	Profile related sections
 ***********************************************************************************/
#ifdef CONFIG_AP_SUPPORT
#ifdef RTMP_MAC_PCI
#define AP_PROFILE_PATH			"/etc/Wireless/RT2860AP/RT2860AP.dat"
#define AP_RTMP_FIRMWARE_FILE_NAME "/etc/Wireless/RT2860AP/RT2860AP.bin"
#define AP_DRIVER_VERSION			"3.0.0.0"
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
#endif /* CONFIG_AP_SUPPORT */



#ifdef SINGLE_SKU_V2
#define SINGLE_SKU_TABLE_FILE_NAME	"/etc/Wireless/RT2870STA/SingleSKU.dat"
#endif /* SINGLE_SKU_V2 */


/***********************************************************************************
 *	Compiler related definitions
 ***********************************************************************************/
#define IN
#define OUT
#define INOUT
#define NDIS_STATUS		INT


/***********************************************************************************
 *	OS Specific definitions and data structures
 ***********************************************************************************/
typedef void				*PPCI_DEV;
typedef void				*PNET_DEV;
typedef void				*PNDIS_PACKET;
typedef char				NDIS_PACKET;
typedef PNDIS_PACKET		* PPNDIS_PACKET;
typedef ra_dma_addr_t		NDIS_PHYSICAL_ADDRESS;
typedef ra_dma_addr_t		*PNDIS_PHYSICAL_ADDRESS;
typedef void				*NDIS_HANDLE;
typedef char				*PNDIS_BUFFER;


/***********************************************************************************
 *	Network related constant definitions
 ***********************************************************************************/
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif

#define MAC_ADDR_LEN	6

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
#define MAX_PACKETS_IN_QUEUE				1024 /*(512)*/
#else
#define MAX_PACKETS_IN_QUEUE				(512)
#endif /* DOT11_VHT_AC */


/***********************************************************************************
 *	OS signaling related constant definitions
 ***********************************************************************************/

/***********************************************************************************
 *	OS file operation related data structure definitions
 ***********************************************************************************/
typedef VOID *			RTMP_OS_FD;

#define IS_FILE_OPEN_ERR(_fd)	RtmpOsFileIsErr((_fd))

#ifndef O_RDONLY
#define O_RDONLY			RTMP_FILE_RDONLY
#endif /* O_RDONLY */

#ifndef O_WRONLY
#define O_WRONLY			RTMP_FILE_WRONLY
#endif /* O_WRONLY */

#ifndef O_CREAT
#define O_CREAT				RTMP_FILE_CREAT
#endif /* O_CREAT */

#ifndef O_TRUNC
#define O_TRUNC				RTMP_FILE_TRUNC
#endif /* O_TRUNC */


/***********************************************************************************
 *	OS semaphore related data structure and definitions
 ***********************************************************************************/
#define RTCMDUp				RtmpOsCmdUp


/***********************************************************************************
 *	OS Memory Access related data structure and definitions
 ***********************************************************************************/
#define NdisMoveMemory(Destination, Source, Length) memmove(Destination, Source, Length)
#define NdisCopyMemory(Destination, Source, Length) memcpy(Destination, Source, Length)
#define NdisZeroMemory(Destination, Length)         memset(Destination, 0, Length)
#define NdisFillMemory(Destination, Length, Fill)   memset(Destination, Fill, Length)
#define NdisCmpMemory(Destination, Source, Length)  memcmp(Destination, Source, Length)
#define NdisEqualMemory(Source1, Source2, Length)   (!memcmp(Source1, Source2, Length))
#define RTMPEqualMemory(Source1, Source2, Length)	(!memcmp(Source1, Source2, Length))

#define MlmeAllocateMemory(_pAd, _ppVA)			os_alloc_mem(_pAd, _ppVA, MAX_MGMT_PKT_LEN)
#define MlmeFreeMemory(_pVA)				os_free_mem(_pVA)

#define COPY_MAC_ADDR(Addr1, Addr2)			memcpy((Addr1), (Addr2), MAC_ADDR_LEN)


/***********************************************************************************
 *	OS task related data structure and definitions
 ***********************************************************************************/
#define RTMP_OS_PID						ULONG /* value or pointer */

#define RTMP_GET_OS_PID(_a, _b)			RtmpOsGetPid(&_a, _b);
#define CHECK_TASK_LEGALITY(_task)		RtmpOsCheckTaskLegality(_task)
#define ATE_KILL_THREAD_PID				RtmpThreadPidKill

typedef INT(*RTMP_OS_TASK_CALLBACK)(ULONG);


/***********************************************************************************
 * IOCTL related definitions and data structures.
 **********************************************************************************/
#define NET_IOCTL				VOID
#define PNET_IOCTL				(VOID *)

/* undef them to avoid compile errors in rt_symb.c */
#undef EINVAL
#undef EOPNOTSUPP
#undef EFAULT
#undef ENETDOWN
#undef E2BIG
#undef ENOMEM
#undef EAGAIN
#undef ENOTCONN

#define EINVAL		(-RTMP_IO_EINVAL)
#define EOPNOTSUPP	(-RTMP_IO_EOPNOTSUPP)
#define EFAULT		(-RTMP_IO_EFAULT)
#define ENETDOWN	(-RTMP_IO_ENETDOWN)
#define E2BIG		(-RTMP_IO_E2BIG)
#define ENOMEM		(-RTMP_IO_ENOMEM)
#define EAGAIN		(-RTMP_IO_EAGAIN)
#define ENOTCONN	(-RTMP_IO_ENOTCONN)


/***********************************************************************************
 * Timer related definitions and data structures.
 **********************************************************************************/
#define OS_HZ					RtmpOsTickUnitGet()

typedef void (*TIMER_FUNCTION)(ULONG);

#define OS_WAIT					RtmpOsWait

#define RTMP_TIME_AFTER			RtmpOsTimerAfter
#define RTMP_TIME_BEFORE		RtmpOsTimerBefore

#define ONE_TICK				1

#define NdisGetSystemUpTime		RtmpOsGetSystemUpTime


/***********************************************************************************
 *	OS specific cookie data structure binding to RTMP_ADAPTER
 ***********************************************************************************/

/* do not have compile option in the structure for UTIL module */
struct os_cookie {
#ifdef RTMP_MAC_PCI
	VOID *pci_dev;
	VOID *parent_pci_dev;
	USHORT DeviceID;
#endif /* RTMP_MAC_PCI */



	VOID	*pDev;

	UINT32 pAd_va;

	RTMP_NET_TASK_STRUCT	rx_done_task;
	RTMP_NET_TASK_STRUCT	cmd_rsp_event_task;
	RTMP_NET_TASK_STRUCT	mgmt_dma_done_task;
	RTMP_NET_TASK_STRUCT	ac0_dma_done_task;
	RTMP_NET_TASK_STRUCT	ac1_dma_done_task;
	RTMP_NET_TASK_STRUCT	ac2_dma_done_task;
	RTMP_NET_TASK_STRUCT	ac3_dma_done_task;
#if defined(MT7622) || defined(P18) || defined(MT7663)
	RTMP_NET_TASK_STRUCT	ac15_dma_done_task;
#endif
	RTMP_NET_TASK_STRUCT	hcca_dma_done_task;
	RTMP_NET_TASK_STRUCT	wmm1_ac0_dma_done_task;
	RTMP_NET_TASK_STRUCT	tbtt_task;

#ifdef MT_MAC
	RTMP_NET_TASK_STRUCT	mt_mac_int_0_task;
	RTMP_NET_TASK_STRUCT	mt_mac_int_1_task;
	RTMP_NET_TASK_STRUCT	mt_mac_int_2_task;
	RTMP_NET_TASK_STRUCT	mt_mac_int_3_task;
	RTMP_NET_TASK_STRUCT	mt_mac_int_4_task;
	RTMP_NET_TASK_STRUCT	bcn_dma_done_task;
#endif /*MT_MAC */

#ifdef RTMP_MAC_PCI
#ifdef CONFIG_ANDES_SUPPORT
	RTMP_NET_TASK_STRUCT	rx1_done_task;
#endif /* CONFIG_ANDES_SUPPORT */
#endif /* RTMP_MAC_PCI */

#ifdef UAPSD_SUPPORT
	RTMP_NET_TASK_STRUCT	uapsd_eosp_sent_task;
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT

#ifdef CARRIER_DETECTION_SUPPORT
	RTMP_NET_TASK_STRUCT	carrier_sense_task;
#endif /* CARRIER_DETECTION_SUPPORT */


#endif /* CONFIG_AP_SUPPORT */


	RTMP_OS_PID				apd_pid; /*802.1x daemon pid */
	unsigned long			apd_pid_nr;
#ifdef CONFIG_AP_SUPPORT
#ifdef IAPP_SUPPORT
	/*	RT_SIGNAL_STRUC			RTSignal; */
	RTMP_OS_PID				IappPid; /*IAPP daemon pid */
	unsigned long			IappPid_nr;
#endif /* IAPP_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */
	INT						ioctl_if_type;
	INT					ioctl_if;
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
		if (!(x)) {							\
			printk(__FILE__ ":%d assert " #x "failed\n", __LINE__);	\
			dump_stack(); /*panic("Unexpected error occurs!\n"); */	\
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
/*#ifdef RTMP_MAC_PCI*/
#define size_t						ULONG

ra_dma_addr_t linux_pci_map_single(void *handle, void *ptr, size_t size, int sd_idx, int direction);
void linux_pci_unmap_single(void *handle, ra_dma_addr_t dma_addr, size_t size, int direction);

#define pci_enable_msi		RtmpOsPciMsiEnable
#define pci_disable_msi		RtmpOsPciMsiDisable

#define PCI_MAP_SINGLE_DEV(_handle, _ptr, _size, _sd_idx, _dir)				\
	linux_pci_map_single(_handle, _ptr, _size, _sd_idx, _dir)

#define DMA_MAPPING_ERROR(_handle, _ptr)	\
	dma_mapping_error(&((struct pci_dev *)(_handle))->dev, _ptr)

#define PCI_UNMAP_SINGLE(_pAd, _ptr, _size, _dir)						\
	linux_pci_unmap_single(((POS_COOKIE)(_pAd->OS_Cookie))->pci_dev, _ptr, _size, _dir)

#define PCI_ALLOC_CONSISTENT(_pci_dev, _size, _ptr)							\
	pci_alloc_consistent(_pci_dev, _size, _ptr)

#define PCI_FREE_CONSISTENT(_pci_dev, _size, _virtual_addr, _physical_addr)	\
	pci_free_consistent(_pci_dev, _size, _virtual_addr, _physical_addr)
/*#endif RTMP_MAC_PCI*/

#define DEV_ALLOC_SKB(_pAd, _Pkt, _length)									\
	(_Pkt = RtmpOSNetPktAlloc(_pAd, _length))

/*#ifdef RTMP_MAC_USB */
/*#define PCI_MAP_SINGLE(_handle, _ptr, _size, _dir)				(ULONG)0 */
/*#define PCI_UNMAP_SINGLE(_handle, _ptr, _size, _dir) */
/*#endif RTMP_MAC_USB */


/*
 * ULONG
 * RTMP_GetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressLow(phy_addr)		(phy_addr)

/*
 * ULONG
 * RTMP_GetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress);
 */
#define RTMP_GetPhysicalAddressHigh(phy_addr)		(0)

/*
 * VOID
 * RTMP_SetPhysicalAddressLow(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressLow(phy_addr, Value)	\
	(phy_addr = Value)

/*
 * VOID
 * RTMP_SetPhysicalAddressHigh(
 *   IN NDIS_PHYSICAL_ADDRESS  PhysicalAddress,
 *   IN ULONG  Value);
 */
#define RTMP_SetPhysicalAddressHigh(phy_addr, Value)

#define NdisMIndicateStatus(_w, _x, _y, _z)



/***********************************************************************************
 * Device Register I/O Access related definitions and data structures.
 **********************************************************************************/
#define readl		RTMP_PCI_Readl
#define readw		RTMP_PCI_Readw
#define readb		RTMP_PCI_Readb
#define writel		RTMP_PCI_Writel
#define writew		RTMP_PCI_Writew
#define writeb		RTMP_PCI_Writeb

/* TODO: We can merge two readl to a function to speed up or one real/writel */

#ifdef RTMP_MAC_PCI
#if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(INF_AR9) || defined(IKANOS_VX_1X0)
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)									\
	do {																	\
		(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
		(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
	} while (0)

#define RTMP_IO_READ32(_A, _R, _pV)									\
	do {																	\
		if ((_A)->bPCIclkOff == FALSE) {                                      \
			(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
			(*_pV = SWAP32(*((UINT32 *)(_pV))));                           \
		}                                                                   \
	} while (0)

#define RTMP_IO_READ8(_A, _R, _pV)									\
	(*_pV = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#define RTMP_IO_WRITE32(_A, _R, _V) \
	do { \
		if ((_A)->bPCIclkOff == FALSE) { \
			UINT32 _Val = SWAP32(_V);\
			writel(_Val, (void *)((_A)->PciHif.CSRBaseAddress + (_R)));\
		} \
	} while (0)

#ifdef INF_VR9
#define RTMP_IO_WRITE8(_A, _R, _V)            \
	{                    \
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
	}
#else
#define RTMP_IO_WRITE8(_A, _R, _V)									\
	writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)))
#endif

#define RTMP_IO_WRITE16(_A, _R, _V)									\
	writew(SWAP16((_V)), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)))
#else
#define RTMP_IO_FORCE_READ32(_A, _R, _pV)								\
	(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#define RTMP_IO_READ32(_A, _R, _pV)								\
	do {																\
		if ((_A)->bPCIclkOff == FALSE)                                  \
			(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))));			\
		else															\
			*_pV = 0;													\
	} while (0)

#define RTMP_IO_FORCE_READ32(_A, _R, _pV)							\
	(*_pV = readl((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#define RTMP_IO_READ8(_A, _R, _pV)								\
	(*_pV = readb((void *)((_A)->PciHif.CSRBaseAddress + (_R))))

#define RTMP_IO_WRITE32(_A, _R, _V)												\
	{																				\
		if ((_A)->bPCIclkOff == FALSE)                                  \
			writel((_V), (void *)((_A)->PciHif.CSRBaseAddress + (_R)));								\
	}

#define RTMP_IO_FORCE_WRITE32(_A, _R, _V)												\
	writel(_V, (void *)((_A)->PciHif.CSRBaseAddress + (_R)))

#ifdef RTMP_RBUS_SUPPORT
/* This is actually system IO */
#define RTMP_SYS_IO_READ32(_R, _pV)		\
	(*_pV = readl((void *)(_R)))

#define RTMP_SYS_IO_WRITE32(_R, _V)		\
	writel(_V, (void *)(_R))
#endif /* RTMP_RBUS_SUPPORT */


#if defined(BRCM_6358) || defined(RALINK_2880) || defined(RALINK_3052) || defined(RALINK_2883) || defined(RTMP_RBUS_SUPPORT) || defined(MT76x2)
#define RTMP_IO_WRITE8(_A, _R, _V)            \
	{                    \
		ULONG Val;                \
		UCHAR _i;                \
		_i = ((_R) & 0x3);             \
		Val = readl((void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));   \
		Val = Val & (~(0x000000ff << ((_i)*8)));         \
		Val = Val | ((ULONG)(_V) << ((_i)*8));         \
		writel((Val), (void *)((_A)->PciHif.CSRBaseAddress + ((_R) - _i)));    \
	}
#else
#define RTMP_IO_WRITE8(_A, _R, _V)							\
	{															\
		writeb((_V), (PUCHAR)((_A)->PciHif.CSRBaseAddress + (_R)));		\
	}
#endif /* #if defined(BRCM_6358) || defined(RALINK_2880) */

#define RTMP_IO_WRITE16(_A, _R, _V)							\
	{															\
		writew((_V), (PUSHORT)((_A)->PciHif.CSRBaseAddress + (_R)));	\
	}
#endif /* #if defined(INF_TWINPASS) || defined(INF_DANUBE) || defined(IKANOS_VX_1X0) */
#endif /* RTMP_MAC_PCI */



#define MAC_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define MAC_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)

#define HIF_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define HIF_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)

#define PHY_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define PHY_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)

#define HW_IO_READ32(_A, _R, _pV)			RTMP_IO_READ32(_A, _R, _pV)
#define HW_IO_WRITE32(_A, _R, _V)			RTMP_IO_WRITE32(_A, _R, _V)


#define pci_read_config_word	RtmpOsPciConfigReadWord
#define pci_write_config_word	RtmpOsPciConfigWriteWord
#define pci_read_config_dword	RtmpOsPciConfigReadDWord
#define pci_write_config_dword	RtmpOsPciConfigWriteDWord
#define pci_find_capability		RtmpOsPciFindCapability

#define RTMP_USB_URB_DATA_GET	RtmpOsUsbUrbDataGet
#define RTMP_USB_URB_STATUS_GET	RtmpOsUsbUrbStatusGet
#define RTMP_USB_URB_LEN_GET	RtmpOsUsbUrbLenGet

#define IW_SCAN_MAX_DATA		RTMP_OS_MAX_SCAN_DATA_GET()

/***********************************************************************************
 *	Network Related data structure and marco definitions
 ***********************************************************************************/
#define PKTSRC_NDIS             0x7f
#define PKTSRC_DRIVER           0x0f

#define RTMP_OS_NETDEV_GET_PHYADDR	RtmpOsNetDevGetPhyAddr
#define SET_OS_PKT_NETDEV			RtmpOsSetPktNetDev
#define RTMP_OS_NETDEV_GET_DEVNAME	RtmpOsGetNetDevName
#define RTMP_OS_NETDEV_SET_TYPE		RtmpOsSetNetDevType
#define RTMP_OS_NETDEV_SET_TYPE_MONITOR		RtmpOsSetNetDevTypeMonitor

#define QUEUE_ENTRY_TO_PACKET(pEntry) \
	(PNDIS_PACKET)(pEntry)

#define PACKET_TO_QUEUE_ENTRY(pPacket) \
	(PQUEUE_ENTRY)(pPacket)

#define RTMP_OS_NETDEV_STATE_RUNNING(_pNetDev)	(RtmpOSNetDevIsUp(_pNetDev) == TRUE)

#define RELEASE_NDIS_PACKET(_pReserved, _pPacket, _Status)				\
	{																		\
		RTMPFreeNdisPacket(_pReserved, _pPacket);						\
	}

#define RELEASE_NDIS_PACKET_IRQ(_pReserved, _pPacket, _Status)			\
	{																		\
		RTMPFreeNdisPacketIRQ(_pReserved, _pPacket);					\
	}

/*
 * packet helper
 *	- convert internal rt packet to os packet or
 *             os packet to rt packet
 */
extern ULONG RTPktOffsetData, RTPktOffsetLen, RTPktOffsetCB;

#define RTPKT_TO_OSPKT(_p)		(_p)
#define OSPKT_TO_RTPKT(_p)		(_p)

#define GET_OS_PKT_DATAPTR(_pkt) \
	((UCHAR *)(*(ULONG *)((UCHAR *)_pkt + RTPktOffsetData)))

#define SET_OS_PKT_DATAPTR		\
	RtmpOsPktDataPtrAssign

#define GET_OS_PKT_LEN(_pkt)	\
	(*(UINT32 *)((UCHAR *)_pkt + RTPktOffsetLen))

#define SET_OS_PKT_LEN			\
	RtmpOsPktLenAssign

#define GET_OS_PKT_CB(_pkt)	\
	((UCHAR *)((UCHAR *)_pkt + RTPktOffsetCB))

#define GET_OS_PKT_NETDEV(_pkt) RtmpOsPktNetDevGet

#define OS_PKT_CLONED			\
	RtmpOsIsPktCloned

#define OS_PKT_COPY				\
	RtmpOsPktCopy

#define OS_PKT_TAIL_ADJUST		\
	RtmpOsPktTailAdjust

#define OS_PKT_HEAD_BUF_EXTEND	\
	RtmpOsPktHeadBufExtend

#define OS_PKT_TAIL_BUF_EXTEND	\
	RtmpOsPktTailBufExtend

#define OS_PKT_RESERVE			\
	RtmpOsPktReserve

#define OS_PKT_CLONE(_pAd, _pkt, _src, _flag)		\
	(_src = RtmpOsPktClone((_pkt)))

#define RTMP_OS_PKT_INIT			RtmpOsPktInit

extern UINT32 RtmpOsGetUnaligned32(
	IN UINT32				*pWord);

extern ULONG RtmpOsGetUnalignedlong(
	IN ULONG				*pWord);

#define get_unaligned		RtmpOsGetUnaligned
#define get_unaligned32		RtmpOsGetUnaligned32
#define get_unalignedlong	RtmpOsGetUnalignedlong

#define OS_NTOHS		RtmpOsNtohs
#define OS_HTONS		RtmpOsHtons
#define OS_NTOHL		RtmpOsNtohl
#define OS_HTONL		RtmpOsHtonl

#ifndef ntohs
#define ntohs			OS_NTOHS
#endif /* ntohs */
#ifndef htons
#define htons			OS_HTONS
#endif /* htons */
#ifndef ntohl
#define ntohl			OS_NTOHL
#endif /* ntohl */
#ifndef htonl
#define htonl			OS_HTONL
#endif /* htonl */

#define RTMP_OS_NETDEV_START_QUEUE				RtmpOsNetQueueStart
#define RTMP_OS_NETDEV_STOP_QUEUE				RtmpOsNetQueueStop
#define RTMP_OS_NETDEV_WAKE_QUEUE				RtmpOsNetQueueWake


#define CB_OFF  10
#define CB_LEN	36

#define PACKET_CB_ASSIGN(_p, _offset)	\
	(*((UINT8 *)_p + RTPktOffsetCB + _offset))

#define PACKET_CB(_p, _offset)	\
	(*((UINT8 *)_p + RTPktOffsetCB + CB_OFF + _offset))




/***********************************************************************************
 *	Other function prototypes definitions
 ***********************************************************************************/
void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);


#ifdef RTMP_RBUS_SUPPORT
#ifndef CONFIG_RALINK_FLASH_API
void FlashWrite(UCHAR *p, ULONG a, ULONG b);
void FlashRead(UCHAR *p, ULONG a, ULONG b);
#endif /* CONFIG_RALINK_FLASH_API */

int wl_proc_init(void);
int wl_proc_exit(void);

#ifdef LINUX
#if defined(CONFIG_RA_CLASSIFIER) || defined(CONFIG_RA_CLASSIFIER_MODULE)
extern volatile unsigned long classifier_cur_cycle;
extern int (*ra_classifier_hook_rx) (struct sk_buff *skb, unsigned long cycle);
#endif /* defined(CONFIG_RA_CLASSIFIER)||defined(CONFIG_RA_CLASSIFIER_MODULE) */
#endif /* LINUX */
#endif /* RTMP_RBUS_SUPPORT */




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


/* OS definition re-declaration */
#ifndef NULL
#define NULL			0
#endif

#ifndef ETH_P_IPV6
#define ETH_P_IPV6		0x86DD
#endif

#ifndef ETH_P_IP
#define ETH_P_IP        0x0800          /* Internet Protocol packet     */
#endif

#ifndef ETH_ALEN
#define ETH_ALEN		6
#endif

#undef KERN_EMERG
#define KERN_EMERG
#undef KERN_WARNING
#define KERN_WARNING

#undef copy_from_user
#undef copy_to_user

#define RTMP_OS_MAX_SCAN_DATA_GET		RtmpOsMaxScanDataGet
#define vmalloc							RtmpOsVmalloc
#define vfree							RtmpOsVfree
#define copy_from_user					RtmpOsCopyFromUser
#define copy_to_user					RtmpOsCopyToUser
#define simple_strtol					RtmpOsSimpleStrtol

#undef atomic_read
#undef atomic_dec
#undef InterlockedExchange
#define atomic_read						RtmpOsAtomicRead
#define atomic_dec						RtmpOsAtomicDec
#define InterlockedExchange				RtmpOsAtomicInterlockedExchange

extern int sprintf(char *buf, const char *fmt, ...);
extern int sscanf(const char *, const char *, ...);

#define printk						pRaOsOps->ra_printk
#define snprintf						pRaOsOps->ra_snprintf

#define RA_WEXT		0
#define RA_NETLINK	1

#endif /* __RT_LINUX_H__ */
