/****************************************************************************

    Module Name:
	mt_os_util.h

	Abstract:
	All function prototypes are provided from UTIL modules.

	Note:
	But can not use any OS key word and compile option here.
	All functions are provided from UTIL modules.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------

***************************************************************************/

#ifndef __MT_OS_UTIL_H__
#define __MT_OS_UTIL_H__

INT os_alloc_mem(
	VOID * pAd,
	UCHAR **mem,
	ULONG size
);

VOID os_free_mem(
	PVOID mem
);

VOID os_zero_mem(
	PVOID ptr,
	ULONG length
);

VOID os_move_mem(
	PVOID pDest,
	PVOID pSrc,
	ULONG length
);

VOID os_fill_mem(
	PVOID pBuf,
	ULONG Length,
	UCHAR Fill
);

INT os_cmp_mem(
	PVOID Destination,
	PVOID Source,
	ULONG Length
);

INT os_equal_mem(
	PVOID pBuf1,
	PVOID pBuf2,
	ULONG length
);

RTMP_OS_FD_EXT os_file_open(
	CHAR * pPath,
	INT32  flag, /* CreateDisposition */
	INT32 file_mode
);

INT os_file_close(
	RTMP_OS_FD_EXT osfd
);

VOID os_file_seek(
	RTMP_OS_FD_EXT osfd,
	INT32 offset
);

INT os_file_write(
	RTMP_OS_FD_EXT osfd,
	CHAR *pDataPtr,
	INT32 writeLen
);

INT os_file_read(
	RTMP_OS_FD_EXT osfd,
	CHAR *pDataPtr,
	INT32 readLen
);

VOID os_msec_delay(UINT msec);
VOID os_usec_delay(UINT usec);
VOID os_load_code_from_bin(void *pAd, unsigned char **image, char *bin_name, UINT32 *code_len);
CHAR *os_str_chr(CHAR *str, INT32 character);
UINT32 os_str_spn(CHAR *str1, CHAR *str2);
CHAR *os_str_pbrk(CHAR *str1, CHAR *str2);
LONG os_str_tol(const CHAR *str, CHAR **endptr, INT32 base);
LONG os_str_toul(const CHAR * str,CHAR * * endptr,INT32 base);
CHAR *os_str_str(const CHAR *str1, const CHAR *str2);

INT os_usb_vendor_req(
	struct _RTMP_ADAPTER *pAd,
	UINT32   TransferFlags,
	UCHAR   RequestType,
	UCHAR   Request,
	USHORT   Value,
	USHORT   Index,
	PVOID   TransferBuffer,
	UINT32   TransferBufferLength
);

#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
VOID
os_alloc_dma_buf(
	IN VOID *pHandler,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT OS_PHYSICAL_ADDRESS * phy_addr);

VOID
os_free_dma_buf(
	IN VOID *pHandler,
	IN ULONG Length,
	IN PVOID VirtualAddress,
	IN OS_PHYSICAL_ADDRESS PhysicalAddress);

OS_PACKET
os_alloc_packet(
	IN VOID *pHandler,
	IN ULONG Length);

VOID
os_free_packet(
	IN VOID *pHandler,
	IN OS_PACKET *pPacket);

VOID os_spin_lock_init(OS_LOCK *pLock);
VOID os_spin_lock_free(OS_LOCK *pLock);

/*Lock for Passive Level*/
VOID os_spin_lock(OS_LOCK *pLock);
VOID os_spin_unlock(OS_LOCK *pLock);
/*Lock for IRQ Level*/
VOID os_spin_lock_irqsave(OS_LOCK *pLock);
VOID os_spin_unlock_irqrestore(OS_LOCK *pLock);

VOID os_event_init(OS_EVENT *pEvent);
VOID os_event_wait(OS_EVENT *pEvent, UINT32 TimeOut);
VOID os_event_set(OS_EVENT *pEvent);
#endif /*#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)*/
#endif
