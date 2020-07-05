/****************************************************************************
 *
 *  Module Name:
 *	rt_rbus_pci_util.c
 *
 *  Abstract:
 *	Any utility is used in UTIL module for PCI/RBUS function.
 *
 *  Revision History:
 *  Who        When          What
 *  ---------  ----------    ----------------------------------------------
 *
 ***************************************************************************/
#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rtmp_osabl.h"
#include "rt_os_util.h"

#ifdef RTMP_MAC_PCI
VOID *alloc_rx_buf_1k(void *hif_resource);
static inline void *dma_zalloc_coherent(struct device *dev, size_t size, dma_addr_t *dma_handle, gfp_t flag)

{

      void *ret = dma_alloc_coherent(dev, size, dma_handle, flag);

      if (ret)

         memset(ret, 0, size);

      return ret;

}
/* Function for Tx/Rx/Mgmt Desc Memory allocation. */
void RtmpAllocDescBuf(
	IN VOID * pDev,
	IN UINT Index,
	IN ULONG Length,
	IN BOOLEAN Cached,
	OUT VOID * *VirtualAddress,
	OUT PNDIS_PHYSICAL_ADDRESS	phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(*phy_addr);
	struct device *pdev = (struct device *)pDev;
	*VirtualAddress = (PVOID)dma_zalloc_coherent(pdev, sizeof(char) * Length, &DmaAddr, GFP_KERNEL);
	*phy_addr = (NDIS_PHYSICAL_ADDRESS)DmaAddr;
}


/* Function for free allocated Desc Memory. */
void RtmpFreeDescBuf(
	IN VOID * pDev,
	IN ULONG Length,
	IN VOID * VirtualAddress,
	IN NDIS_PHYSICAL_ADDRESS phy_addr)
{
	dma_addr_t DmaAddr = (dma_addr_t)(phy_addr);
	struct device *pdev = (struct device *)pDev;

	dma_free_coherent(pdev, Length, VirtualAddress, DmaAddr);
}

void RTMP_AllocateFirstTxBuffer(
		VOID * pDev,
		UINT Index,
		ULONG Length,
		BOOLEAN Cached,
		PVOID *va,
		PNDIS_PHYSICAL_ADDRESS phy_addr)
{
	*va = kmalloc(Length, GFP_KERNEL);
	phy_addr = NULL;
}

void RTMP_FreeFirstTxBuffer(
	IN VOID * pDev,
	IN ULONG Length,
	IN BOOLEAN Cached,
	IN VOID *va,
	IN NDIS_PHYSICAL_ADDRESS phy_addr)
{
	kfree(va);
}

#ifdef BB_SOC
__IMEM
#endif
PNDIS_PACKET RTMP_AllocateRxPacketBuffer(
	VOID *reserved,
	VOID *dev,
	enum  MEM_ALLOC_TYPE type,
	ULONG size,
	VOID **va,
	PNDIS_PHYSICAL_ADDRESS pa)
{
	struct sk_buff *pkt = NULL;
	struct device *pdev = (struct device *)dev;

	switch (type) {
#ifdef CONFIG_WIFI_BUILD_SKB
	case DYNAMIC_PAGE_ALLOC:
		/* pkt is dynamic-alloc dma va address */
		DEV_ALLOC_FRAG(pkt, SKB_DATA_ALIGN(SKB_BUF_HEADROOM_RSV + size)
				+ SKB_DATA_ALIGN(SKB_BUF_TAILROOM_RSV));

		if (pkt)
			*va = ((PVOID)pkt + SKB_BUF_HEADROOM_RSV);

		break;
#else
	case DYNAMIC_PAGE_ALLOC:
		/* not support. alloc as DYNAMIC_SLAB_ALLOC*/
#endif

	case DYNAMIC_SLAB_ALLOC:
		/* pkt is dynamic-alloc skb */
#ifdef BB_SOC
		pkt = skbmgr_dev_alloc_skb4k();
#else
		DEV_ALLOC_SKB(pkt, size);
#endif

		if (pkt)
			*va = (PVOID)pkt->data;

		break;

	case PRE_SLAB_ALLOC:
		/* pkt is pre-alloc dma va adrress */
		pkt = alloc_rx_buf_1k(reserved);

		if (pkt)
			*va = ((PVOID)pkt);

		break;

	default:

		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("%s: unknown allocate type %d\n", __func__, type));
		break;
	}

	if (pkt) {
		*pa = dma_map_single(pdev, *va, size, PCI_DMA_FROMDEVICE);
	} else {
		*va = (PVOID)NULL;
		*pa = (NDIS_PHYSICAL_ADDRESS)0;
		MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_ERROR, ("can't allocate rx %ld size packet\n", size));
	}

	return (PNDIS_PACKET)pkt;
}

/*
 * invaild or writeback cache
 * and convert virtual address to physical address
 */
ra_dma_addr_t linux_pci_map_single(void *pDev, void *ptr, size_t size, int sd_idx, int direction)
{
	struct device *pdev = (struct device *)pDev;

	if (direction == RTMP_PCI_DMA_TODEVICE)
		direction = PCI_DMA_TODEVICE;

	if (direction == RTMP_PCI_DMA_FROMDEVICE)
		direction = PCI_DMA_FROMDEVICE;

	/*
	 *	------ Porting Information ------
	 *	> For Tx Alloc:
	 *		mgmt packets => sd_idx = 0
	 *		SwIdx: pAd->MgmtRing.TxCpuIdx
	 *		pTxD : pAd->MgmtRing.Cell[SwIdx].AllocVa;
	 *
	 *		data packets => sd_idx = 1
	 *		TxIdx : pAd->PciHif.TxRing[pTxBlk->QueIdx].TxCpuIdx
	 *		QueIdx: pTxBlk->QueIdx
	 *		pTxD  : pAd->PciHif.TxRing[pTxBlk->QueIdx].Cell[TxIdx].AllocVa;
	 *
	 *	> For Rx Alloc:
	 *		sd_idx = -1
	 */
	/*	pAd = (PRTMP_ADAPTER)handle; */
	/*	pObj = (POS_COOKIE)pAd->OS_Cookie; */
	{
		return (ra_dma_addr_t)dma_map_single(pdev, ptr, size, direction);
	}
}

void linux_pci_unmap_single(void *pDev, ra_dma_addr_t radma_addr, size_t size, int direction)
{
	dma_addr_t DmaAddr = (dma_addr_t)radma_addr;
	struct device *pdev = (struct device *)pDev;

	if (direction == RTMP_PCI_DMA_TODEVICE)
		direction = PCI_DMA_TODEVICE;

	if (direction == RTMP_PCI_DMA_FROMDEVICE)
		direction = PCI_DMA_FROMDEVICE;

	if (size > 0)
		dma_unmap_single(pdev, DmaAddr, size, direction);
}


#ifdef RTMP_PCI_SUPPORT
VOID *RTMPFindHostPCIDev(VOID *pPciDevSrc)
{
	struct pci_dev *pci_dev = (struct pci_dev *)pPciDevSrc;
	struct pci_dev *parent_pci_dev;
	USHORT reg16;
	UCHAR reg8;
	UINT DevFn;
	PPCI_DEV pPci_dev;

	MTWF_LOG(DBG_CAT_HIF, CATHIF_PCI, DBG_LVL_TRACE, ("%s.===>\n", __func__));
	parent_pci_dev = NULL;

	if (pci_dev->bus->parent) {
		for (DevFn = 0; DevFn < 255; DevFn++) {
#if (KERNEL_VERSION(2, 6, 0) <= LINUX_VERSION_CODE)
			pPci_dev = pci_get_slot(pci_dev->bus->parent, DevFn);
#else
			pPci_dev = pci_find_slot(pci_dev->bus->parent->number, DevFn);
#endif

			if (pPci_dev) {
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
 * ========================================================================
 * Routine Description:
 *	Write 32-bit to a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID RTMP_PCI_Writel(ULONG Value, VOID *pAddr)
{
	writel(Value, pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Write 16-bit to a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
VOID RTMP_PCI_Writew(ULONG Value, VOID *pAddr)
{
	writew(Value, pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Write 8-bit to a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
VOID RTMP_PCI_Writeb(ULONG Value, VOID *pAddr)
{
	writeb(Value, pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Read 32-bit from a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
ULONG RTMP_PCI_Readl(VOID *pAddr)
{
	return readl(pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Read 16-bit from a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 16-bit value
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
ULONG RTMP_PCI_Readw(VOID *pAddr)
{
	return readw(pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Read 8-bit from a register.
 *
 * Arguments:
 *	pAd				- WLAN control block pointer
 *	Offset			- Register offset
 *	Value			- 16-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
ULONG RTMP_PCI_Readb(VOID *pAddr)
{
	return readb(pAddr);
}


/*
 * ========================================================================
 * Routine Description:
 *	Read 16-bit from the PCI config space.
 *
 * Arguments:
 *	pDev			- PCI device
 *	Offset			- Register offset
 *	Value			- 16-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
int RtmpOsPciConfigReadWord(VOID *pDev, UINT32 Offset, UINT16 *pValue)
{
	return pci_read_config_word((struct pci_dev *)pDev, Offset, pValue);
}


/*
 * ========================================================================
 * Routine Description:
 *	Write 16-bit to the PCI config space.
 *
 * Arguments:
 *	pDev			- PCI device
 *	Offset			- Register offset
 *	Value			- 16-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
int RtmpOsPciConfigWriteWord(VOID *pDev, UINT32 Offset, UINT16 Value)
{
	return pci_write_config_word((struct pci_dev *)pDev, Offset, Value);
}


/*
 * ========================================================================
 * Routine Description:
 *	Read 32-bit from the PCI config space.
 *
 * Arguments:
 *	pDev			- PCI device
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 * ========================================================================
 */
int RtmpOsPciConfigReadDWord(VOID *pDev, UINT32 Offset, UINT32 *pValue)
{
	return pci_read_config_dword((struct pci_dev *)pDev, Offset, pValue);
}


/*
 * ========================================================================
 * Routine Description:
 *	Write 32-bit to the PCI config space.
 *
 * Arguments:
 *	pDev			- PCI device
 *	Offset			- Register offset
 *	Value			- 32-bit value
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
int RtmpOsPciConfigWriteDWord(VOID *pDev, UINT32 Offset, UINT32 Value)
{
	return pci_write_config_dword((struct pci_dev *)pDev, Offset, Value);
}


/*
 * ========================================================================
 * Routine Description:
 *	Query for devices' capabilities.
 *
 * Arguments:
 *	pDev			- PCI device
 *	Cap				- Capability code
 *
 * Return Value:
 *	None
 *
 * ========================================================================
 */
int RtmpOsPciFindCapability(VOID *pDev, INT Cap)
{
	return pci_find_capability(pDev, Cap);
}

/*
 * ========================================================================
 * Routine Description:
 *	Enable MSI function for PCI.
 *
 * Arguments:
 *	pDev			- PCI device
 *
 * Return Value:
 *	None
 * ========================================================================
 */
int RtmpOsPciMsiEnable(VOID *pDev)
{
	return pci_enable_msi(pDev);
}


/*
 * ========================================================================
 * Routine Description:
 *	Disable MSI function for PCI.
 *
 * Arguments:
 *	pDev			- PCI device
 *
 * Return Value:
 *	None
 * ========================================================================
 */
VOID RtmpOsPciMsiDisable(VOID *pDev)
{
	pci_disable_msi(pDev);
}

#endif /* RTMP_MAC_PCI */

