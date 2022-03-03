/****************************************************************************

    Module Name:
	rt_os_util.h

	Abstract:
	All function prototypes are provided from UTIL modules.

	Note:
	But can not use any OS key word and compile option here.
	All functions are provided from UTIL modules.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/
#include "rt_config.h"
#ifndef __RT_OS_UTIL_H__
#define __RT_OS_UTIL_H__

/* ============================ rt_linux.c ================================== */
/* General */
VOID RtmpUtilInit(VOID);

/* OS Time */
VOID RtmpusecDelay(ULONG usec);
VOID RtmpOsMsDelay(ULONG msec);

void RTMP_GetCurrentSystemTime(LARGE_INTEGER *time);

ULONG RTMPMsecsToJiffies(UINT msec);
void RTMP_GetCurrentSystemTick(ULONG *pNow);

VOID RtmpOsWait(UINT32 Time);
UINT32 RtmpOsTimerAfter(ULONG a, ULONG b);
UINT32 RtmpOsTimerBefore(ULONG a, ULONG b);
VOID RtmpOsGetSystemUpTime(ULONG *pTime);
UINT32 RtmpOsTickUnitGet(VOID);


/*
	OS Memory
*/
/*
	Function:
		allocate memory 
	Parameters:
		
	Return:
		
		mem shall be set as NULL if allocation failed
*/
NDIS_STATUS os_alloc_mem(VOID *pReserved, UCHAR **mem, ULONG size);
NDIS_STATUS os_alloc_mem_suspend(VOID *pReserved, UCHAR **mem, ULONG size);
NDIS_STATUS os_move_mem(PVOID pDest, PVOID pSrc, ULONG length);
NDIS_STATUS os_free_mem(VOID *pReserved, VOID *mem);

NDIS_STATUS os_zero_mem(PVOID ptr, ULONG length);

NDIS_STATUS AdapterBlockAllocateMemory(
	IN	VOID *handle,
	OUT	VOID **ppAd,
	IN	UINT32 SizeOfpAd);

VOID *RtmpOsVmalloc(ULONG Size);
VOID RtmpOsVfree(VOID *pMem);

ULONG RtmpOsCopyFromUser(VOID *to, const void *from, ULONG n);
ULONG RtmpOsCopyToUser(VOID *to, const void *from, ULONG n);

BOOLEAN RtmpOsStatsAlloc(VOID **ppStats, VOID **ppIwStats);

/* OS Packet */
PNDIS_PACKET RtmpOSNetPktAlloc(VOID *pReserved, int size);

PNDIS_PACKET RTMP_AllocateFragPacketBuffer(VOID *pReserved, ULONG Length);

NDIS_STATUS RTMPAllocateNdisPacket(
	IN	VOID					*pReserved,
	OUT PNDIS_PACKET			*ppPacket,
	IN	PUCHAR					pHeader,
	IN	UINT					HeaderLen,
	IN	PUCHAR					pData,
	IN	UINT					DataLen);

VOID RTMPFreeNdisPacket(VOID *pReserved, PNDIS_PACKET pPacket);

void RTMP_QueryPacketInfo(
	IN  PNDIS_PACKET pPacket,
	OUT PACKET_INFO *pPacketInfo,
	OUT PUCHAR *pSrcBufVA,
	OUT	UINT *pSrcBufLen);


PNDIS_PACKET ClonePacket(PNET_DEV ndev, PNDIS_PACKET pkt, UCHAR *buf, ULONG sz);
PNDIS_PACKET CopyPacket(PNET_DEV ndev, PNDIS_PACKET pkt,  ULONG sz);

PNDIS_PACKET DuplicatePacket(PNET_DEV pNetDev, PNDIS_PACKET pPacket);

PNDIS_PACKET duplicate_pkt_with_TKIP_MIC(
	IN VOID *pReserved,
	IN PNDIS_PACKET pOldPkt);

PNDIS_PACKET duplicate_pkt_vlan(
	IN	PNET_DEV				pNetDev,
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	PUCHAR					pHeader802_3,
    IN  UINT            		HdrLen,
	IN	PUCHAR					pData,
	IN	ULONG					DataSize,
	IN	UCHAR					*TPID);

typedef void (*RTMP_CB_8023_PACKET_ANNOUNCE)(
	IN VOID *pCtrlBkPtr, 
	IN PNDIS_PACKET pPacket,
	IN UCHAR OpMode);

BOOLEAN RTMPL2FrameTxAction(
	IN  VOID					*pCtrlBkPtr,
	IN	PNET_DEV				pNetDev,
	IN	RTMP_CB_8023_PACKET_ANNOUNCE _announce_802_3_packet,
	IN	UCHAR					apidx,
	IN	PUCHAR					pData,
	IN	UINT32					data_len,
	IN	UCHAR			OpMode);

PNDIS_PACKET ExpandPacket(
	IN	VOID					*pReserved,
	IN	PNDIS_PACKET			pPacket,
	IN	UINT32					ext_head_len,
	IN	UINT32					ext_tail_len);

void wlan_802_11_to_802_3_packet(
	IN	PNET_DEV				pNetDev,
	IN	UCHAR					OpMode,
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	PNDIS_PACKET			pRxPacket,
	IN	UCHAR					*pData,
	IN	ULONG					DataSize,
	IN	PUCHAR					pHeader802_3,
	IN	UCHAR					*TPID);

#ifdef CONFIG_SNIFFER_SUPPORT
void send_prism_monitor_packets(
		PNET_DEV pNetDev,
		PNDIS_PACKET pRxPacket,
		VOID *dot11_hdr,
		UCHAR *pData,
		USHORT DataSize,
		UCHAR L2PAD,
		UCHAR PHYMODE,
		UCHAR BW,
		UCHAR ShortGI,
		UCHAR MCS,
		UCHAR AMPDU,
		UCHAR STBC,
		UCHAR RSSI1,
		UCHAR BssMonitorFlag11n,
		UCHAR *pDevName,
		UCHAR Channel,
		UCHAR CentralChannel,
		UINT32 MaxRssi);

void send_radiotap_monitor_packets(
		PNET_DEV pNetDev,
		PNDIS_PACKET pRxPacket,
		VOID *dot11_hdr,
		UCHAR *pData,
		USHORT DataSize,
		UCHAR L2PAD,
		UCHAR PHYMODE,
		UCHAR BW,
		UCHAR ShortGI,
		UCHAR MCS,
		UCHAR LDPC,
		UCHAR LDPC_EX_SYM,
		UCHAR AMPDU,
		UCHAR STBC,
		UCHAR RSSI1,
		UCHAR *pDevName,
		UCHAR Channel,
		UCHAR CentralChannel,
		UCHAR sideband_index,
		CHAR MaxRssi,
		UINT32 timestamp);

#endif /* CONFIG_SNIFFER_SUPPORT */

UCHAR VLAN_8023_Header_Copy(
	IN	USHORT					VLAN_VID,
	IN	USHORT					VLAN_Priority,
	IN	PUCHAR					pHeader802_3,
	IN	UINT            		HdrLen,
	OUT PUCHAR					pData,
	IN	UCHAR					*TPID);

VOID RtmpOsPktBodyCopy(
	IN	PNET_DEV				pNetDev,
	IN	PNDIS_PACKET			pNetPkt,
	IN	ULONG					ThisFrameLen,
	IN	PUCHAR					pData);

INT RtmpOsIsPktCloned(PNDIS_PACKET pNetPkt);
PNDIS_PACKET RtmpOsPktCopy(PNDIS_PACKET pNetPkt);
PNDIS_PACKET RtmpOsPktClone(PNDIS_PACKET pNetPkt);

VOID RtmpOsPktDataPtrAssign(PNDIS_PACKET pNetPkt, UCHAR *pData);

VOID RtmpOsPktLenAssign(PNDIS_PACKET pNetPkt, LONG Len);
VOID RtmpOsPktTailAdjust(PNDIS_PACKET pNetPkt, UINT removedTagLen);

PUCHAR RtmpOsPktTailBufExtend(PNDIS_PACKET pNetPkt, UINT len);
PUCHAR RtmpOsPktHeadBufExtend(PNDIS_PACKET pNetPkt, UINT len);
VOID RtmpOsPktReserve(PNDIS_PACKET pNetPkt, UINT len);

VOID RtmpOsPktProtocolAssign(PNDIS_PACKET pNetPkt);
VOID RtmpOsPktInfPpaSend(PNDIS_PACKET pNetPkt);
VOID RtmpOsPktRcvHandle(PNDIS_PACKET pNetPkt);
VOID RtmpOsPktNatMagicTag(PNDIS_PACKET pNetPkt);
VOID RtmpOsPktNatNone(PNDIS_PACKET pNetPkt);
VOID RtmpOsPktInit(PNDIS_PACKET pNetPkt, PNET_DEV pNetDev, UCHAR *buf, USHORT len);

PNDIS_PACKET RtmpOsPktIappMakeUp(PNET_DEV pNetDev, UINT8 *pMac);

BOOLEAN RtmpOsPktOffsetInit(VOID);

UINT16 RtmpOsNtohs(UINT16 Value);
UINT16 RtmpOsHtons(UINT16 Value);
UINT32 RtmpOsNtohl(UINT32 Value);
UINT32 RtmpOsHtonl(UINT32 Value);

/* OS File */
RTMP_OS_FD RtmpOSFileOpen(char *pPath,  int flag, int mode);
int RtmpOSFileClose(RTMP_OS_FD osfd);
void RtmpOSFileSeek(RTMP_OS_FD osfd, int offset);
int RtmpOSFileRead(RTMP_OS_FD osfd, char *pDataPtr, int readLen);
int RtmpOSFileWrite(RTMP_OS_FD osfd, char *pDataPtr, int writeLen);

INT32 RtmpOsFileIsErr(VOID *pFile);

void RtmpOSFSInfoChange(RTMP_OS_FS_INFO *pOSFSInfoOrg, BOOLEAN bSet);

/* OS Network Interface */
int RtmpOSNetDevAddrSet(
	IN UCHAR OpMode,
	IN PNET_DEV pNetDev,
	IN PUCHAR pMacAddr,
	IN PUCHAR dev_name);

void RtmpOSNetDevClose(PNET_DEV pNetDev);
void RtmpOSNetDevFree(PNET_DEV pNetDev);
INT RtmpOSNetDevAlloc(PNET_DEV *new_dev_p, UINT32 privDataSize);
INT RtmpOSNetDevOpsAlloc(PVOID *pNetDevOps);



PNET_DEV RtmpOSNetDevGetByName(PNET_DEV pNetDev, RTMP_STRING *pDevName);

void RtmpOSNetDeviceRefPut(PNET_DEV pNetDev);

INT RtmpOSNetDevDestory(VOID *pReserved, PNET_DEV pNetDev);
void RtmpOSNetDevDetach(PNET_DEV pNetDev);
int RtmpOSNetDevAttach(
	IN	UCHAR					OpMode,
	IN	PNET_DEV				pNetDev, 
	IN	RTMP_OS_NETDEV_OP_HOOK	*pDevOpHook);

void RtmpOSNetDevProtect(
	IN BOOLEAN lock_it);

PNET_DEV RtmpOSNetDevCreate(
	IN	INT32					MC_RowID,
	IN	UINT32					*pIoctlIF,
	IN	INT 					devType,
	IN	INT						devNum,
	IN	INT						privMemSize,
	IN	RTMP_STRING *pNamePrefix);

BOOLEAN RtmpOSNetDevIsUp(VOID *pDev);

unsigned char *RtmpOsNetDevGetPhyAddr(VOID *pDev);

VOID RtmpOsNetQueueStart(PNET_DEV pDev);
VOID RtmpOsNetQueueStop(PNET_DEV pDev);
VOID RtmpOsNetQueueWake(PNET_DEV pDev);

VOID RtmpOsSetPktNetDev(VOID *pPkt, VOID *pDev);
PNET_DEV RtmpOsPktNetDevGet(VOID *pPkt);

char *RtmpOsGetNetDevName(VOID *pDev);

UINT32 RtmpOsGetNetIfIndex(IN VOID *pDev);

VOID RtmpOsSetNetDevPriv(VOID *pDev, VOID *pPriv);
VOID *RtmpOsGetNetDevPriv(VOID *pDev);

VOID RtmpOsSetNetDevWdev(VOID *net_dev, VOID *wdev);
VOID *RtmpOsGetNetDevWdev(VOID *pDev);

USHORT RtmpDevPrivFlagsGet(VOID *pDev);
VOID RtmpDevPrivFlagsSet(VOID *pDev, USHORT PrivFlags);

VOID RtmpOsSetNetDevType(VOID *pDev, USHORT Type);
VOID RtmpOsSetNetDevTypeMonitor(VOID *pDev);
UCHAR get_sniffer_mode(VOID *pDev);
VOID set_sniffer_mode(VOID *pDev, UCHAR mode);

/* OS Semaphore */
VOID RtmpOsCmdUp(RTMP_OS_TASK *pCmdQTask);
BOOLEAN RtmpOsSemaInitLocked(RTMP_OS_SEM *pSemOrg, LIST_HEADER *pSemList);
BOOLEAN RtmpOsSemaInit(RTMP_OS_SEM *pSemOrg, LIST_HEADER *pSemList);
BOOLEAN RtmpOsSemaDestory(RTMP_OS_SEM *pSemOrg);
INT RtmpOsSemaWaitInterruptible(RTMP_OS_SEM *pSemOrg);
VOID RtmpOsSemaWakeUp(RTMP_OS_SEM *pSemOrg);
VOID RtmpOsMlmeUp(RTMP_OS_TASK *pMlmeQTask);

VOID RtmpOsInitCompletion(RTMP_OS_COMPLETION *pCompletion);
VOID RtmpOsExitCompletion(RTMP_OS_COMPLETION *pCompletion);
VOID RtmpOsComplete(RTMP_OS_COMPLETION *pCompletion);
ULONG RtmpOsWaitForCompletionTimeout(RTMP_OS_COMPLETION *pCompletion, ULONG Timeout); 

/* OS Task */
BOOLEAN RtmpOsTaskletSche(RTMP_NET_TASK_STRUCT *pTasklet);

BOOLEAN RtmpOsTaskletInit(
	RTMP_NET_TASK_STRUCT *pTasklet,
	VOID (*pFunc)(unsigned long data),
	ULONG Data,
	LIST_HEADER *pTaskletList);

BOOLEAN RtmpOsTaskletKill(RTMP_NET_TASK_STRUCT *pTasklet);
VOID RtmpOsTaskletDataAssign(RTMP_NET_TASK_STRUCT *pTasklet, ULONG Data);

VOID RtmpOsTaskWakeUp(RTMP_OS_TASK *pTaskOrg);
INT32 RtmpOsTaskIsKilled(RTMP_OS_TASK *pTaskOrg);

BOOLEAN RtmpOsCheckTaskLegality(RTMP_OS_TASK *pTaskOrg);

BOOLEAN RtmpOSTaskAlloc(RTMP_OS_TASK *pTask, LIST_HEADER *pTaskList);

VOID RtmpOSTaskFree(RTMP_OS_TASK *pTask);

NDIS_STATUS RtmpOSTaskKill(RTMP_OS_TASK *pTaskOrg);

INT RtmpOSTaskNotifyToExit(RTMP_OS_TASK *pTaskOrg);

VOID RtmpOSTaskCustomize(RTMP_OS_TASK *pTaskOrg);

NDIS_STATUS RtmpOSTaskAttach(
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	RTMP_OS_TASK_CALLBACK	fn,
	IN	ULONG arg);

NDIS_STATUS RtmpOSTaskInit(
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	RTMP_STRING *pTaskName,
	IN	VOID *pPriv,
	IN	LIST_HEADER *pTaskList,
	IN	LIST_HEADER *pSemList);

BOOLEAN RtmpOSTaskWait(
	IN	VOID *pReserved,
	IN	RTMP_OS_TASK *pTaskOrg,
	IN	INT32 *pStatus);

VOID *RtmpOsTaskDataGet(RTMP_OS_TASK *pTaskOrg);

INT32 RtmpThreadPidKill(RTMP_OS_PID	 PID);

/* OS Cache */
VOID RtmpOsDCacheFlush(ULONG AddrStart, ULONG Size);

/* OS Timer */
VOID RTMP_SetPeriodicTimer(
	IN	NDIS_MINIPORT_TIMER *pTimerOrg, 
	IN	unsigned long timeout);

VOID RTMP_OS_Init_Timer(
	IN	VOID *pReserved,
	IN	NDIS_MINIPORT_TIMER *pTimerOrg, 
	IN	TIMER_FUNCTION function,
	IN	PVOID data,
	IN	LIST_HEADER *pTimerList);

VOID RTMP_OS_Add_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout);
VOID RTMP_OS_Mod_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, unsigned long timeout);
VOID RTMP_OS_Del_Timer(NDIS_MINIPORT_TIMER *pTimerOrg, BOOLEAN *pCancelled);
VOID RTMP_OS_Release_Timer(NDIS_MINIPORT_TIMER *pTimerOrg);

BOOLEAN RTMP_OS_Alloc_Rsc(LIST_HEADER *pRscList, VOID *pRsc, UINT32 RscLen);
VOID RTMP_OS_Free_Rscs(LIST_HEADER *pRscList);

/* OS Lock */
BOOLEAN RtmpOsAllocateLock(NDIS_SPIN_LOCK *pLock, LIST_HEADER *pLockList);
VOID RtmpOsFreeSpinLock(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsSpinLockBh(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsSpinUnLockBh(NDIS_SPIN_LOCK *pLockOrg);
VOID RtmpOsIntLock(NDIS_SPIN_LOCK *pLockOrg, ULONG *pIrqFlags);
VOID RtmpOsIntUnLock(NDIS_SPIN_LOCK *pLockOrg, ULONG IrqFlags);

/* OS PID */
VOID RtmpOsGetPid(ULONG *pDst, ULONG PID);
VOID RtmpOsTaskPidInit(RTMP_OS_PID *pPid);

/* OS I/O */
VOID RTMP_PCI_Writel(ULONG Value, VOID *pAddr);
VOID RTMP_PCI_Writew(ULONG Value, VOID *pAddr);
VOID RTMP_PCI_Writeb(ULONG Value, VOID *pAddr);
ULONG RTMP_PCI_Readl(VOID *pAddr);
ULONG RTMP_PCI_Readw(VOID *pAddr);
ULONG RTMP_PCI_Readb(VOID *pAddr);

int RtmpOsPciConfigReadWord(
	IN	VOID					*pDev,
	IN	UINT32					Offset,
	OUT UINT16					*pValue);

int RtmpOsPciConfigWriteWord(VOID *pDev, UINT32 Offset, UINT16 Value);
int RtmpOsPciConfigReadDWord(VOID *pDev, UINT32 Offset, UINT32 *pValue);
int RtmpOsPciConfigWriteDWord(VOID *pDev, UINT32 Offset, UINT32 Value);

int RtmpOsPciFindCapability(VOID *pDev, INT Cap);

VOID *RTMPFindHostPCIDev(VOID *pPciDevSrc);

int RtmpOsPciMsiEnable(VOID *pDev);
VOID RtmpOsPciMsiDisable(VOID *pDev);

/* OS Wireless */
ULONG RtmpOsMaxScanDataGet(VOID);

/* OS Interrutp */
INT32 RtmpOsIsInInterrupt(VOID);

/* OS USB */
VOID *RtmpOsUsbUrbDataGet(VOID *pUrb);
NTSTATUS RtmpOsUsbUrbStatusGet(VOID *pUrb);
ULONG RtmpOsUsbUrbLenGet(VOID *pUrb);

/* OS Atomic */
BOOLEAN RtmpOsAtomicInit(RTMP_OS_ATOMIC *pAtomic, LIST_HEADER *pAtomicList);
VOID RtmpOsAtomicDestroy(RTMP_OS_ATOMIC *pAtomic);
LONG RtmpOsAtomicRead(RTMP_OS_ATOMIC *pAtomic);
VOID RtmpOsAtomicDec(RTMP_OS_ATOMIC *pAtomic);
VOID RtmpOsAtomicInterlockedExchange(RTMP_OS_ATOMIC *pAtomicSrc, LONG Value);

/* OS Utility */
void hex_dump(char *str, unsigned char *pSrcBufVA, unsigned int SrcBufLen);

typedef VOID (*RTMP_OS_SEND_WLAN_EVENT)(
	IN	VOID					*pAdSrc,
	IN	USHORT					Event_flag,
	IN	PUCHAR 					pAddr,
	IN  UCHAR					BssIdx,
	IN	CHAR					Rssi);

VOID RtmpOsSendWirelessEvent(
	IN	VOID			*pAd,
	IN	USHORT			Event_flag,
	IN	PUCHAR 			pAddr,
	IN	UCHAR			BssIdx,
	IN	CHAR			Rssi,
	IN	RTMP_OS_SEND_WLAN_EVENT pFunc);

#ifdef CONFIG_AP_SUPPORT
void SendSignalToDaemon(
	IN	INT sig,
	IN	RTMP_OS_PID	 pid,
	IN	unsigned long pid_no);
#endif /* CONFIG_AP_SUPPORT */

int RtmpOSWrielessEventSend(
	IN	PNET_DEV				pNetDev,
	IN	UINT32					eventType,
	IN	INT						flags,
	IN	PUCHAR					pSrcMac,
	IN	PUCHAR					pData,
	IN	UINT32					dataLen);

int RtmpOSWrielessEventSendExt(
	IN	PNET_DEV				pNetDev,
	IN	UINT32					eventType,
	IN	INT						flags,
	IN	PUCHAR					pSrcMac,
	IN	PUCHAR					pData,
	IN	UINT32					dataLen,
	IN	UINT32					family);

UINT RtmpOsWirelessExtVerGet(VOID);

VOID RtmpDrvAllMacPrint(
	IN VOID						*pReserved,
	IN UINT32					*pBufMac,
	IN UINT32					AddrStart,
	IN UINT32					AddrEnd,
	IN UINT32					AddrStep);

VOID RtmpDrvAllE2PPrint(
	IN	VOID					*pReserved,
	IN	USHORT					*pMacContent,
	IN	UINT32					AddrEnd,
	IN	UINT32					AddrStep);

VOID RtmpDrvAllRFPrint(
	IN VOID *pReserved,
	IN UCHAR *pBuf,
	IN UINT32 BufLen);

int RtmpOSIRQRelease(
	IN	PNET_DEV				pNetDev,
	IN	UINT32					infType,
	IN	PPCI_DEV				pci_dev,
	IN	BOOLEAN					*pHaveMsi);

VOID RtmpOsWlanEventSet(
	IN	VOID					*pReserved,
	IN	BOOLEAN					*pCfgWEnt,
	IN	BOOLEAN					FlgIsWEntSup);

UINT16 RtmpOsGetUnaligned(UINT16 *pWord);

UINT32 RtmpOsGetUnaligned32(UINT32 *pWord);

ULONG RtmpOsGetUnalignedlong(ULONG *pWord);

long RtmpOsSimpleStrtol(
	IN	const char				*cp,
	IN	char 					**endp,
	IN	unsigned int			base);

VOID RtmpOsOpsInit(RTMP_OS_ABL_OPS *pOps);

/* ============================ rt_os_util.c ================================ */
VOID RtmpDrvMaxRateGet(
	IN VOID *pReserved,
	IN UINT8 MODE,
	IN UINT8 ShortGI,
	IN UINT8 BW,
	IN UINT8 MCS,
	OUT UINT32 *pRate);

char * rtstrchr(const char * s, int c);

RTMP_STRING *WscGetAuthTypeStr(USHORT authFlag);

RTMP_STRING *WscGetEncryTypeStr(USHORT encryFlag);

USHORT WscGetAuthTypeFromStr(RTMP_STRING *arg);

USHORT WscGetEncrypTypeFromStr(RTMP_STRING *arg);

VOID RtmpMeshDown(
	IN VOID *pDrvCtrlBK,
	IN BOOLEAN WaitFlag,
	IN BOOLEAN (*RtmpMeshLinkCheck)(IN VOID *pAd));

USHORT RtmpOsNetPrivGet(PNET_DEV pDev);

BOOLEAN RtmpOsCmdDisplayLenCheck(
	IN	UINT32					LenSrc,
	IN	UINT32					Offset);

VOID WpaSendMicFailureToWpaSupplicant(
	IN PNET_DEV pNetDev,
	IN const PUCHAR src_addr,
	IN BOOLEAN bUnicast,
	IN INT key_id,
	IN const PUCHAR tsc);

int wext_notify_event_assoc(
	IN	PNET_DEV				pNetDev,
	IN	UCHAR					*ReqVarIEs,
	IN	UINT32					ReqVarIELen);

VOID    SendAssocIEsToWpaSupplicant( 
	IN	PNET_DEV				pNetDev,
	IN	UCHAR					*ReqVarIEs,
	IN	UINT32					ReqVarIELen);

/* ============================ rt_rbus_pci_util.c ========================== */
void RtmpAllocDescBuf(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RtmpFreeDescBuf(
	IN PPCI_DEV pPciDev,
	IN ULONG Length,
	IN VOID *VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RTMP_AllocateFirstTxBuffer(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);

void RTMP_FreeFirstTxBuffer(
	IN	PPCI_DEV				pPciDev,
	IN	ULONG					Length,
	IN	BOOLEAN					Cached,
	IN	PVOID					VirtualAddress,
	IN	NDIS_PHYSICAL_ADDRESS	PhysicalAddress);

PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	IN	VOID					*pReserved,
	IN	VOID					*pPciDev,
	IN	ULONG					Length,
	IN	BOOLEAN					Cached,
	OUT	PVOID					*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS	PhysicalAddress);




ra_dma_addr_t linux_pci_map_single(void *pPciDev, void *ptr, size_t size, int sd_idx, int direction);

void linux_pci_unmap_single(void *pPciDev, ra_dma_addr_t dma_addr, size_t size, int direction);

/* ============================ rt_usb_util.c =============================== */

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
void RtmpFlashRead(
	RTMP_ADAPTER *ad,
	UCHAR * p,
	ULONG a,
	ULONG b);

void RtmpFlashWrite(
	UCHAR * p,
	ULONG a,
	ULONG b);
#endif /* defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT) */

UINT32 RtmpOsGetUsbDevVendorID(
	IN VOID *pUsbDev);

UINT32 RtmpOsGetUsbDevProductID(
	IN VOID *pUsbDev);

/* CFG80211 */




/* ================================ MACRO =================================== */
#define RTMP_UTIL_DCACHE_FLUSH(__AddrStart, __Size)

/* ================================ EXTERN ================================== */
extern UCHAR SNAP_802_1H[6];
extern UCHAR SNAP_BRIDGE_TUNNEL[6];
extern UCHAR EAPOL[2];
extern UCHAR TPID[];
extern UCHAR IPX[2];
extern UCHAR APPLE_TALK[2];
extern UCHAR NUM_BIT8[8];
extern ULONG RTPktOffsetData, RTPktOffsetLen, RTPktOffsetCB;

extern ULONG OS_NumOfMemAlloc, OS_NumOfMemFree;

extern INT32 ralinkrate[];
extern UINT32 RT_RateSize;

#ifdef PLATFORM_UBM_IPX8
#include "vrut_ubm.h"
#endif /* PLATFORM_UBM_IPX8 */

INT32  RtPrivIoctlSetVal(VOID);

void OS_SPIN_LOCK(NDIS_SPIN_LOCK *lock);
void OS_SPIN_UNLOCK(NDIS_SPIN_LOCK *lock);
void OS_SPIN_LOCK_IRQSAVE(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void OS_SPIN_UNLOCK_IRQRESTORE(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void OS_SPIN_LOCK_IRQ(NDIS_SPIN_LOCK *lock);
void OS_SPIN_UNLOCK_IRQ(NDIS_SPIN_LOCK *lock);
void RtmpOsSpinLockIrqSave(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void RtmpOsSpinUnlockIrqRestore(NDIS_SPIN_LOCK *lock, unsigned long *flags);
void RtmpOsSpinLockIrq(NDIS_SPIN_LOCK *lock);
void RtmpOsSpinUnlockIrq(NDIS_SPIN_LOCK *lock);
int OS_TEST_BIT(int bit, unsigned long *flags);
void OS_SET_BIT(int bit, unsigned long *flags);
void OS_CLEAR_BIT(int bit, unsigned long *flags);
void OS_LOAD_CODE_FROM_BIN(unsigned char **image, char *bin_name, void *inf_dev, UINT32 *code_len);
VOID os_load_code_from_bin(void *pAd, unsigned char **image, char *bin_name, UINT32 *code_len);

#endif /* __RT_OS_UTIL_H__ */

