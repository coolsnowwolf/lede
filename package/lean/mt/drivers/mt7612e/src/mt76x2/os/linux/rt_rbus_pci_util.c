/****************************************************************************

    Module Name:
	rt_rbus_pci_util.c
 
    Abstract:
	Any utility is used in UTIL module for PCI/RBUS function.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"


#ifdef RTMP_MAC_PCI

/* Function for Tx/Rx/Mgmt Desc Memory allocation. */
void RtmpAllocDescBuf(
	IN PPCI_DEV pPciDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(*phy_addr);

	*VirtualAddress = (PVOID)pci_alloc_consistent(pPciDev,sizeof(char)*Length, &DmaAddr);
	*phy_addr = (NDIS_PHYSICAL_ADDRESS)DmaAddr;
}


/* Function for free allocated Desc Memory. */
void RtmpFreeDescBuf(
	IN PPCI_DEV pPciDev,
	IN ULONG Length,
	IN VOID *VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(phy_addr);

	pci_free_consistent(pPciDev, Length, VirtualAddress, DmaAddr);
}


/* Function for TxData DMA Memory allocation. */
void RTMP_AllocateFirstTxBuffer(
	IN	PPCI_DEV				pPciDev,
	IN	UINT					Index,
	IN	ULONG					Length,
	IN	BOOLEAN					Cached,
	OUT	PVOID					*VirtualAddress,
	OUT	PNDIS_PHYSICAL_ADDRESS	phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(*phy_addr);

	*VirtualAddress = (PVOID)pci_alloc_consistent(pPciDev,sizeof(char)*Length, &DmaAddr);
	*phy_addr = (NDIS_PHYSICAL_ADDRESS)DmaAddr;
}


void RTMP_FreeFirstTxBuffer(
	IN PPCI_DEV pPciDev,
	IN ULONG Length,
	IN BOOLEAN Cached,
	IN VOID *VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(phy_addr);
	pci_free_consistent(pPciDev, Length, VirtualAddress, DmaAddr);
}


/*
 * FUNCTION: Allocate a packet buffer for DMA
 * ARGUMENTS:
 *     AdapterHandle:  AdapterHandle
 *     Length:  Number of bytes to allocate
 *     Cached:  Whether or not the memory can be cached
 *     VirtualAddress:  Pointer to memory is returned here
 *     phy_addr:  Physical address corresponding to virtual address
 * Notes:
 *     Cached is ignored: always cached memory
 */
PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	IN VOID *pReserved,
	IN VOID *pPciDev,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID **VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	phy_addr)
{
	struct sk_buff *pkt;

/*	pkt = dev_alloc_skb(Length); */
	DEV_ALLOC_SKB(pReserved, pkt, Length);

	if (pkt == NULL) {
		DBGPRINT(RT_DEBUG_ERROR, ("can't allocate rx %ld size packet\n",Length));
	}

	if (pkt) {
		*VirtualAddress = (PVOID) pkt->data;	
		*phy_addr = PCI_MAP_SINGLE_DEV(pPciDev, *VirtualAddress, Length,  -1, RTMP_PCI_DMA_FROMDEVICE);
	} else {
		*VirtualAddress = (PVOID) NULL;
		*phy_addr = (NDIS_PHYSICAL_ADDRESS) 0;
	}	

	return (PNDIS_PACKET) pkt;
}


/*
 * invaild or writeback cache 
 * and convert virtual address to physical address 
 */
ra_dma_addr_t linux_pci_map_single(void *pPciDev, void *ptr, size_t size, int sd_idx, int direction)
{
	if (direction == RTMP_PCI_DMA_TODEVICE)
		direction = PCI_DMA_TODEVICE;

	if (direction == RTMP_PCI_DMA_FROMDEVICE)
		direction = PCI_DMA_FROMDEVICE;

	/* 
		------ Porting Information ------
		> For Tx Alloc:
			mgmt packets => sd_idx = 0
			SwIdx: pAd->MgmtRing.TxCpuIdx
			pTxD : pAd->MgmtRing.Cell[SwIdx].AllocVa;
	 
			data packets => sd_idx = 1
	 		TxIdx : pAd->TxRing[pTxBlk->QueIdx].TxCpuIdx 
	 		QueIdx: pTxBlk->QueIdx 
	 		pTxD  : pAd->TxRing[pTxBlk->QueIdx].Cell[TxIdx].AllocVa;

	 	> For Rx Alloc:
	 		sd_idx = -1
	*/

/*	pAd = (PRTMP_ADAPTER)handle; */
/*	pObj = (POS_COOKIE)pAd->OS_Cookie; */
	
	{
		return (ra_dma_addr_t)pci_map_single(pPciDev, ptr, size, direction);
	}

}

void linux_pci_unmap_single(void *pPciDev, ra_dma_addr_t radma_addr, size_t size, int direction)
{
	dma_addr_t DmaAddr = (dma_addr_t)radma_addr;


	if (direction == RTMP_PCI_DMA_TODEVICE)
		direction = PCI_DMA_TODEVICE;

	if (direction == RTMP_PCI_DMA_FROMDEVICE)
		direction = PCI_DMA_FROMDEVICE;
	
	if (size > 0)
		pci_unmap_single(pPciDev, DmaAddr, size, direction);
	
}


#ifdef RTMP_PCI_SUPPORT
VOID *RTMPFindHostPCIDev(VOID *pPciDevSrc)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pPciDevSrc;
	USHORT reg16;
	UCHAR reg8;
	UINT DevFn;
	PPCI_DEV pPci_dev;


	DBGPRINT(RT_DEBUG_TRACE, ("%s.===>\n", __FUNCTION__));

	if (pci_dev->bus->parent)
	{
		for (DevFn = 0; DevFn < 255; DevFn++)
		{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
			pPci_dev = pci_get_slot(pci_dev->bus->parent, DevFn);
#else
			pPci_dev = pci_find_slot(pci_dev->bus->parent->number, DevFn);
#endif
			if (pPci_dev)
			{                
				pci_read_config_word(pPci_dev, PCI_CLASS_DEVICE, &reg16);
				reg16 = le2cpu16(reg16);
				pci_read_config_byte(pPci_dev, PCI_CB_CARD_BUS, &reg8);
				if ((reg16 == PCI_CLASS_BRIDGE_PCI) && 
					(reg8 == pci_dev->bus->number))
					return pPci_dev;
			}
		}
	}
	return NULL;
}
#endif /* RTMP_PCI_SUPPORT */


/*
========================================================================
Routine Description:
	Write 32-bit to a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_PCI_Writel(ULONG Value, VOID *pAddr)
{
	writel(Value, pAddr);
}


/*
========================================================================
Routine Description:
	Write 16-bit to a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_PCI_Writew(ULONG Value, VOID *pAddr)
{
	writew(Value, pAddr);
}


/*
========================================================================
Routine Description:
	Write 8-bit to a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
VOID RTMP_PCI_Writeb(ULONG Value, VOID *pAddr)
{
	writeb(Value, pAddr);
}


/*
========================================================================
Routine Description:
	Read 32-bit from a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
ULONG RTMP_PCI_Readl(VOID *pAddr)
{
	return readl(pAddr);
}


/*
========================================================================
Routine Description:
	Read 16-bit from a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 16-bit value

Return Value:
	None

Note:
	No use.
========================================================================
*/
ULONG RTMP_PCI_Readw(VOID *pAddr)
{
	return readw(pAddr);
}


/*
========================================================================
Routine Description:
	Read 8-bit from a register.

Arguments:
	pAd				- WLAN control block pointer
	Offset			- Register offset
	Value			- 16-bit value

Return Value:
	None

Note:
========================================================================
*/
ULONG RTMP_PCI_Readb(VOID *pAddr)
{
	return readb(pAddr);
}


/*
========================================================================
Routine Description:
	Read 16-bit from the PCI config space.

Arguments:
	pDev			- PCI device
	Offset			- Register offset
	Value			- 16-bit value

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciConfigReadWord(VOID *pDev, UINT32 Offset, UINT16 *pValue)
{
	return pci_read_config_word((struct pci_dev *)pDev, Offset, pValue);
}


/*
========================================================================
Routine Description:
	Write 16-bit to the PCI config space.

Arguments:
	pDev			- PCI device
	Offset			- Register offset
	Value			- 16-bit value

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciConfigWriteWord(VOID *pDev, UINT32 Offset, UINT16 Value)
{
	return pci_write_config_word((struct pci_dev *)pDev, Offset, Value);
}


/*
========================================================================
Routine Description:
	Read 32-bit from the PCI config space.

Arguments:
	pDev			- PCI device
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciConfigReadDWord(VOID *pDev, UINT32 Offset, UINT32 *pValue)
{
	return pci_read_config_dword((struct pci_dev *)pDev, Offset, pValue);
}


/*
========================================================================
Routine Description:
	Write 32-bit to the PCI config space.

Arguments:
	pDev			- PCI device
	Offset			- Register offset
	Value			- 32-bit value

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciConfigWriteDWord(VOID *pDev, UINT32 Offset, UINT32 Value)
{
	return pci_write_config_dword((struct pci_dev *)pDev, Offset, Value);
}


/*
========================================================================
Routine Description:
	Query for devices' capabilities.

Arguments:
	pDev			- PCI device
	Cap				- Capability code

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciFindCapability(VOID *pDev, INT Cap)
{
	return pci_find_capability(pDev, Cap);
}


#ifdef RTMP_PCI_SUPPORT
/*
========================================================================
Routine Description:
	Enable MSI function for PCI.

Arguments:
	pDev			- PCI device

Return Value:
	None

Note:
========================================================================
*/
int RtmpOsPciMsiEnable(VOID *pDev)
{
#ifdef PCI_MSI_SUPPORT
	return pci_enable_msi(pDev);
#else
	return 0;
#endif
}


/*
========================================================================
Routine Description:
	Disable MSI function for PCI.

Arguments:
	pDev			- PCI device

Return Value:
	None

Note:
========================================================================
*/
VOID RtmpOsPciMsiDisable(VOID *pDev)
{
#ifdef PCI_MSI_SUPPORT
	pci_disable_msi(pDev);
#endif
}
#endif /* RTMP_PCI_SUPPORT */

#endif /* RTMP_MAC_PCI */

