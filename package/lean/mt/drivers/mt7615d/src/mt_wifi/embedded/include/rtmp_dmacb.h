#ifndef __RTMP_DMACB_H__
#define __RTMP_DMACB_H__

/*
	Data buffer for DMA operation, the buffer must be contiguous physical memory
	Both DMA to / from CPU use the same structure.
*/
typedef struct _RTMP_DMABUF {
	ULONG AllocSize;
	PVOID AllocVa;
	NDIS_PHYSICAL_ADDRESS AllocPa;
} RTMP_DMABUF, *PRTMP_DMABUF;

/*
	Control block (Descriptor) for all ring descriptor DMA operation, buffer must be
	contiguous physical memory. NDIS_PACKET stored the binding Rx packet descriptor
	which won't be released, driver has to wait until upper layer return the packet
	before giveing up this rx ring descriptor to ASIC. NDIS_BUFFER is assocaited pair
	to describe the packet buffer. For Tx, NDIS_PACKET stored the tx packet descriptor
	which driver should ACK upper layer when the tx is physically done or failed.
*/
typedef struct _RTMP_DMACB {
	ULONG AllocSize;	/* Control block size */
	PVOID AllocVa;		/* Control block virtual address */
	NDIS_PHYSICAL_ADDRESS AllocPa;	/* Control block physical address */
	PNDIS_PACKET pNdisPacket;
	PNDIS_PACKET pNextNdisPacket;
	NDIS_PHYSICAL_ADDRESS PacketPa;
	RTMP_DMABUF DmaBuf;
#ifdef CUT_THROUGH
	UINT16 token_id;
#endif /* CUT_THROUGH */
} RTMP_DMACB, *PRTMP_DMACB ____cacheline_aligned;

#endif
