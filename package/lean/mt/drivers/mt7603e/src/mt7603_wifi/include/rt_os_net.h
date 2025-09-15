/****************************************************************************

    Module Name:
	rt_os_net.h

	Abstract:
	All function prototypes are defined in NETIF modules.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/

#ifndef __RT_OS_NET_H__
#define __RT_OS_NET_H__

#include "chip/chip_id.h"

typedef VOID *(*RTMP_NET_ETH_CONVERT_DEV_SEARCH)(VOID *net_dev, UCHAR *pData);
typedef int (*RTMP_NET_PACKET_TRANSMIT)(VOID *pPacket);

#ifdef LINUX
#ifdef OS_ABL_FUNC_SUPPORT

/* ========================================================================== */
/* operators used in NETIF module */
/* Note: No need to put any compile option here */
typedef struct _RTMP_DRV_ABL_OPS {

NDIS_STATUS	(*RTMPAllocAdapterBlock)(PVOID handle, VOID **ppAdapter);
VOID (*RTMPFreeAdapter)(VOID *pAd);
BOOLEAN (*RtmpRaDevCtrlExit)(VOID *pAd);
INT (*RtmpRaDevCtrlInit)(VOID *pAd, RTMP_INF_TYPE infType);
VOID (*RTMPHandleInterrupt)(VOID *pAd);
INT (*RTMP_COM_IoctlHandle)(
	IN	VOID *pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN	INT cmd,
	IN	USHORT subcmd,
	IN	VOID *pData,
	IN	ULONG Data);

int (*RTMPSendPackets)(
	IN	NDIS_HANDLE MiniportAdapterContext,
	IN	PPNDIS_PACKET ppPacketArray,
	IN	UINT NumberOfPackets,
	IN	UINT32 PktTotalLen,
	IN	RTMP_NET_ETH_CONVERT_DEV_SEARCH Func);

int (*P2P_PacketSend)(
	IN	PNDIS_PACKET				pPktSrc, 
	IN	PNET_DEV					pDev,
	IN	RTMP_NET_PACKET_TRANSMIT	Func);

INT (*RTMP_AP_IoctlHandle)(
	IN	VOID					*pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);

INT (*RTMP_STA_IoctlHandle)(
	IN	VOID					*pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data,
	IN  USHORT                  priv_flags);

VOID (*RTMPDrvOpen)(VOID *pAd);
VOID (*RTMPDrvClose)(VOID *pAd, VOID *net_dev);
VOID (*RTMPInfClose)(VOID *pAd);
int (*rt28xx_init)(VOID *pAd,  RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName);
} RTMP_DRV_ABL_OPS;

extern RTMP_DRV_ABL_OPS *pRtmpDrvOps;

VOID RtmpDrvOpsInit(
	OUT 	VOID				*pDrvOpsOrg,
	INOUT	VOID				*pDrvNetOpsOrg,
	IN		RTMP_PCI_CONFIG		*pPciConfig,
	IN		RTMP_USB_CONFIG		*pUsbConfig);
#endif /* OS_ABL_FUNC_SUPPORT */
#endif /* LINUX */




/* ========================================================================== */
/* operators used in DRIVER module */
typedef void (*RTMP_DRV_USB_COMPLETE_HANDLER)(VOID *pURB);

typedef struct _RTMP_NET_ABL_OPS {


} RTMP_NET_ABL_OPS;

extern RTMP_NET_ABL_OPS *pRtmpDrvNetOps;

VOID RtmpNetOpsInit(VOID *pNetOpsOrg);
VOID RtmpNetOpsSet(VOID *pNetOpsOrg);


/* ========================================================================== */
#if defined(RTMP_MODULE_OS) && defined(OS_ABL_FUNC_SUPPORT)
/* for UTIL/NETIF module in OS ABL mode */

#define RTMPAllocAdapterBlock (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPAllocAdapterBlock)
#define RTMPFreeAdapter (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPFreeAdapter)
#define RtmpRaDevCtrlExit (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RtmpRaDevCtrlExit)
#define RtmpRaDevCtrlInit (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RtmpRaDevCtrlInit)
#define RTMPHandleInterrupt (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPHandleInterrupt)
#define RTMP_COM_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_COM_IoctlHandle)
#define RTMPSendPackets (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPSendPackets)
#define P2P_PacketSend (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->P2P_PacketSend)
#define RTMP_AP_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_AP_IoctlHandle)
#define RTMP_STA_IoctlHandle (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMP_STA_IoctlHandle)
#define RTMPDrvOpen (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPDrvOpen)
#define RTMPDrvClose (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPDrvClose)
#define RTMPInfClose (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->RTMPInfClose)
#define rt28xx_init (((RTMP_DRV_ABL_OPS *)(pRtmpDrvOps))->rt28xx_init)

#else /* RTMP_MODULE_OS && OS_ABL_FUNC_SUPPORT */

NDIS_STATUS RTMPAllocAdapterBlock(PVOID handle, VOID **ppAdapter);
VOID RTMPFreeAdapter(VOID *pAd);
BOOLEAN RtmpRaDevCtrlExit(VOID *pAd);
INT RtmpRaDevCtrlInit(VOID *pAd, RTMP_INF_TYPE infType);
VOID RTMPHandleInterrupt(VOID *pAd);

INT RTMP_COM_IoctlHandle(
	IN	VOID					*pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);

int	RTMPSendPackets(
	IN	NDIS_HANDLE		MiniportAdapterContext,
	IN	PPNDIS_PACKET	ppPacketArray,
	IN	UINT			NumberOfPackets,
	IN	UINT32			PktTotalLen,
	IN	RTMP_NET_ETH_CONVERT_DEV_SEARCH	Func);

int P2P_PacketSend(
	IN	PNDIS_PACKET				pPktSrc, 
	IN	PNET_DEV					pDev,
	IN	RTMP_NET_PACKET_TRANSMIT	Func);

#ifdef CONFIG_AP_SUPPORT
INT RTMP_AP_IoctlHandle(
	IN	VOID					*pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	INT						cmd,
	IN	USHORT					subcmd,
	IN	VOID					*pData,
	IN	ULONG					Data);
#endif /* CONFIG_AP_SUPPORT */


VOID RTMPDrvOpen(VOID *pAd);
VOID RTMPDrvClose(VOID *pAd, VOID *net_dev);
VOID RTMPInfClose(VOID *pAd);

int rt28xx_init(VOID *pAd, RTMP_STRING *pDefaultMac, RTMP_STRING *pHostName);

PNET_DEV RtmpPhyNetDevMainCreate(VOID *pAd);
#endif /* RTMP_MODULE_OS */

/* ========================================================================== */
int rt28xx_close(VOID *dev);
int rt28xx_open(VOID *dev);

__inline INT VIRTUAL_IF_UP(VOID *pAd)
{
	RT_CMD_INF_UP_DOWN InfConf = { rt28xx_open, rt28xx_close };
	if (RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_UP,
						0, &InfConf, 0) != NDIS_STATUS_SUCCESS)
		return -1;
	return 0;
}

__inline VOID VIRTUAL_IF_DOWN(VOID *pAd)
{
	RT_CMD_INF_UP_DOWN InfConf = { rt28xx_open, rt28xx_close };
	RTMP_COM_IoctlHandle(pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_DOWN,
						0, &InfConf, 0);
	return;
}

#ifdef RTMP_MODULE_OS

#ifdef CONFIG_AP_SUPPORT
INT rt28xx_ap_ioctl(
	IN	PNET_DEV		net_dev, 
	IN	OUT	struct ifreq	*rq, 
	IN	INT			cmd);
#endif /* CONFIG_AP_SUPPORT */


PNET_DEV RtmpPhyNetDevInit(
	IN VOID						*pAd,
	IN RTMP_OS_NETDEV_OP_HOOK	*pNetHook);

BOOLEAN RtmpPhyNetDevExit(
	IN VOID						*pAd, 
	IN PNET_DEV					net_dev);

#endif /* RTMP_MODULE_OS && OS_ABL_FUNC_SUPPORT */


VOID RT28xx_MBSS_Init(VOID *pAd, PNET_DEV main_dev_p);
INT MBSS_VirtualIF_Open(PNET_DEV dev_p);
INT MBSS_VirtualIF_Close(PNET_DEV dev_p);
VOID RT28xx_MBSS_Remove(VOID *pAd);


VOID RT28xx_WDS_Init(VOID *pAd, PNET_DEV net_dev);
INT WdsVirtualIF_open(PNET_DEV dev);
INT WdsVirtualIF_close(PNET_DEV dev);
VOID RT28xx_WDS_Remove(VOID *pAd);

VOID RT28xx_Monitor_Init(VOID *pAd, PNET_DEV main_dev_p);
VOID RT28xx_Monitor_Remove(VOID *pAd);

VOID RT28xx_ApCli_Init(VOID *pAd, PNET_DEV main_dev_p);
INT ApCli_VirtualIF_Open(PNET_DEV dev_p);
INT ApCli_VirtualIF_Close(PNET_DEV dev_p);
VOID RT28xx_ApCli_Remove(VOID *pAd);


VOID RTMP_Mesh_Init(VOID *pAd, PNET_DEV main_dev_p, RTMP_STRING *pHostName);
INT Mesh_VirtualIF_Open(PNET_DEV pDev);
INT Mesh_VirtualIF_Close(PNET_DEV pDev);
VOID RTMP_Mesh_Remove(VOID *pAd);

VOID RTMP_P2P_Init(VOID *pAd, PNET_DEV main_dev_p);
 INT P2P_VirtualIF_Open(PNET_DEV dev_p);
 INT P2P_VirtualIF_Close(PNET_DEV dev_p);
 INT P2P_VirtualIF_PacketSend(
	 IN PNDIS_PACKET	 skb_p, 
	 IN PNET_DEV		 dev_p);
 VOID RTMP_P2P_Remove(VOID *pAd);


PNET_DEV RTMP_CFG80211_FindVifEntry_ByType(
    IN      VOID     *pAdSrc,
    IN      UINT32    devType);

PWIRELESS_DEV RTMP_CFG80211_FindVifEntryWdev_ByType(
    VOID  *pAdSrc, UINT32 devType);
	
VOID RTMP_CFG80211_AddVifEntry(
    IN      VOID     *pAdSrc,
    IN      PNET_DEV pNewNetDev,
    IN      UINT32   DevType);

VOID RTMP_CFG80211_RemoveVifEntry(        
	IN      VOID     *pAdSrc,		
	IN      PNET_DEV pNewNetDev);

PNET_DEV RTMP_CFG80211_VirtualIF_Get(
    IN 		VOID     *pAdSrc);

VOID RTMP_CFG80211_VirtualIF_Init(
    IN VOID         *pAd,
    IN CHAR         *pIfName,
	IN UINT32        DevType);

VOID RTMP_CFG80211_VirtualIF_Remove(
    IN VOID         *pAd,
    IN	PNET_DEV	dev_p,
	IN  UINT32      DevType);

VOID RTMP_CFG80211_AllVirtualIF_Remove(	
	IN VOID 		*pAdSrc);


#ifdef RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT 
INT rt_android_private_command_entry(
	VOID *pAdSrc, struct net_device *net_dev, struct ifreq *ifr, int cmd);
#endif /* RT_CFG80211_ANDROID_PRIV_LIB_SUPPORT */


/* FOR communication with RALINK DRIVER module in NET module */
/* general */
#define RTMP_DRIVER_NET_DEV_GET(__pAd, __pNetDev)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NETDEV_GET, 0, __pNetDev, 0)

#define RTMP_DRIVER_NET_DEV_SET(__pAd, __pNetDev)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NETDEV_SET, 0, __pNetDev, 0)

#define RTMP_DRIVER_OP_MODE_GET(__pAd, __pOpMode)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_OPMODE_GET, 0, __pOpMode, 0)

#define RTMP_DRIVER_IW_STATS_GET(__pAd, __pIwStats)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_IW_STATUS_GET, 0, __pIwStats, 0)

#define RTMP_DRIVER_INF_STATS_GET(__pAd, __pInfStats)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_STATS_GET, 0, __pInfStats, 0)

#define RTMP_DRIVER_INF_TYPE_GET(__pAd, __pInfType)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_TYPE_GET, 0, __pInfType, 0)

#define RTMP_DRIVER_TASK_LIST_GET(__pAd, __pList)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_TASK_LIST_GET, 0, __pList, 0)

#define RTMP_DRIVER_NIC_NOT_EXIST_SET(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_NIC_NOT_EXIST, 0, NULL, 0)

#define RTMP_DRIVER_MCU_SLEEP_CLEAR(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MCU_SLEEP_CLEAR, 0, NULL, 0)


#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_OFF(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_OFF, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_RT28XX_USB_ASICRADIO_ON(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_RT28XX_USB_ASICRADIO_ON, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_SET(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_SET, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_SUSPEND_CLEAR(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_SUSPEND_CLEAR, 0, NULL, 0)

#define RTMP_DRIVER_VIRTUAL_INF_NUM_GET(__pAd, __pIfNum)					\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_VIRTUAL_INF_GET, 0, __pIfNum, 0)

#define RTMP_DRIVER_CHANNEL_GET(__pAd, __Channel)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SIOCGIWFREQ, 0, __Channel, 0)

#define RTMP_DRIVER_IOCTL_SANITY_CHECK(__pAd, __SetCmd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SANITY_CHECK, 0, __SetCmd, 0)

#define RTMP_DRIVER_BITRATE_GET(__pAd, __pBitRate)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, 0, __pBitRate, 0)

#define RTMP_DRIVER_MAIN_INF_CREATE(__pAd, __ppNetDev)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_CREATE, 0, __ppNetDev, 0)

#define RTMP_DRIVER_MAIN_INF_GET(__pAd, __pInfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_ID_GET, 0, __pInfId, 0)

#define RTMP_DRIVER_MAIN_INF_CHECK(__pAd, __InfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_MAIN_CHECK, 0, NULL, __InfId)

#define RTMP_DRIVER_P2P_INF_CHECK(__pAd, __InfId)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_P2P_CHECK, 0, NULL, __InfId)

#ifdef EXT_BUILD_CHANNEL_LIST
#define RTMP_DRIVER_SET_PRECONFIG_VALUE(__pAd)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_SET_PRECONFIG_VALUE, 0, NULL, 0)
#endif /* EXT_BUILD_CHANNEL_LIST */



/* mesh */
#define RTMP_DRIVER_MESH_REMOVE(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MESH_REMOVE, 0, NULL, 0)

/* inf ppa */
#define RTMP_DRIVER_INF_PPA_INIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_PPA_INIT, 0, NULL, 0)

#define RTMP_DRIVER_INF_PPA_EXIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_INF_PPA_EXIT, 0, NULL, 0)

/* pci */
#define RTMP_DRIVER_IRQ_INIT(__pAd)											\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_IRQ_INIT, 0, NULL, 0)

#define RTMP_DRIVER_IRQ_RELEASE(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_IRQ_RELEASE, 0, NULL, 0)

#define RTMP_DRIVER_PCI_MSI_ENABLE(__pAd, __pPciDev)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MSI_ENABLE, 0, __pPciDev, 0)

#define RTMP_DRIVER_PCI_SUSPEND(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_SUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_PCI_RESUME(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_RESUME, 0, NULL, 0)

#define RTMP_DRIVER_PCI_CSR_SET(__pAd, __Address)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCI_CSR_SET, 0, NULL, __Address)

#define RTMP_DRIVER_PCIE_INIT(__pAd, __pPciDev)								\
{																			\
	RT_CMD_PCIE_INIT __Config, *__pConfig = &__Config;						\
	__pConfig->pPciDev = __pPciDev;											\
	__pConfig->ConfigDeviceID = PCI_DEVICE_ID;								\
	__pConfig->ConfigSubsystemVendorID = PCI_SUBSYSTEM_VENDOR_ID;			\
	__pConfig->ConfigSubsystemID = PCI_SUBSYSTEM_ID;						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_PCIE_INIT, 0, __pConfig, 0);\
}

/* usb */
#define RTMP_DRIVER_USB_MORE_FLAG_SET(__pAd, __pConfig)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_MORE_FLAG_SET, 0, __pConfig, 0)

#define RTMP_DRIVER_USB_CONFIG_INIT(__pAd, __pConfig)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_CONFIG_INIT, 0, __pConfig, 0)

#define RTMP_DRIVER_USB_SUSPEND(__pAd, __bIsRunning)						\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_SUSPEND, 0, NULL, __bIsRunning)

#define RTMP_DRIVER_USB_RESUME(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_RESUME, 0, NULL, 0)

#define RTMP_DRIVER_USB_INIT(__pAd, __pUsbDev, __driver_info)	\
do {	\
	RT_CMD_USB_INIT __Config, *__pConfig = &__Config;	\
	__pConfig->pUsbDev = __pUsbDev;	\
	__pConfig->driver_info = __driver_info;	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_USB_INIT, 0, __pConfig, 0);	\
} while (0)

#define RTMP_DRIVER_SDIO_INIT(__pAd)										\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SDIO_INIT, 0, NULL, 0)

/* ap */
#define RTMP_DRIVER_AP_BITRATE_GET(__pAd, __pConfig)							\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_AP_SIOCGIWRATEQ, 0, __pConfig, 0)

#define RTMP_DRIVER_AP_MAIN_OPEN(__pAd)										\
	RTMP_AP_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MAIN_OPEN, 0, NULL, 0)

/* sta */
#define RTMP_DRIVER_STA_DEV_TYPE_SET(__pAd, __Type)							\
	RTMP_STA_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ORI_DEV_TYPE_SET, 0, NULL, __Type, __Type)

#define RTMP_DRIVER_MAC_ADDR_GET(__pAd, __pMacAddr)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_MAC_ADDR_GET, 0, __pMacAddr, 0)
	
#define RTMP_DRIVER_ADAPTER_CSO_SUPPORT_TEST(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_CSO_SUPPORT_TEST, 0,  __flag, 0)

#define RTMP_DRIVER_ADAPTER_TSO_SUPPORT_TEST(__pAd, __flag)								\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_ADAPTER_TSO_SUPPORT_TEST, 0,  __flag, 0)

#ifdef CONFIG_HAS_EARLYSUSPEND
#define RTMP_DRIVER_SET_SUSPEND_FLAG(__pAd) \
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_SET_SUSPEND_FLAG, 0, NULL, 0)

#define RTMP_DRIVER_LOAD_FIRMWARE_CHECK(__pAd)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_LOAD_FIRMWARE_CHECK, 0, NULL, 0)

#define RTMP_DRIVER_OS_COOKIE_GET(__pAd, __os_cookie)							\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_OS_COOKIE_GET, 0, __os_cookie, 0)

#define RTMP_DRIVER_ADAPTER_REGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_REGISTER_EARLYSUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_UNREGISTER_EARLYSUSPEND(__pAd)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_UNREGISTER_EARLYSUSPEND, 0, NULL, 0)

#define RTMP_DRIVER_ADAPTER_CHECK_EARLYSUSPEND(__pAd, __flag)	\
	RTMP_COM_IoctlHandle(__pAd, NULL, CMD_RTPRIV_IOCTL_CHECK_EARLYSUSPEND, 0, __flag, 0)
#endif /* CONFIG_HAS_EARLYSUSPEND */

#endif /* __RT_OS_NET_H__ */

