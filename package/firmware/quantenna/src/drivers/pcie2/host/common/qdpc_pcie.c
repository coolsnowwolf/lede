/**
 * Copyright (c) 2012-2012 Quantenna Communications, Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <asm/byteorder.h>
#include <linux/pci.h>
#include <linux/moduleparam.h>
#include <asm-generic/pci-dma-compat.h>
#include <linux/module.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_regs.h"
#include <qdpc_platform.h>

static int use_msi = 1;
module_param(use_msi, int, 0644);
MODULE_PARM_DESC(use_msi, "Set 0 to use Legacy interrupt");

static int qdpc_pcie_init_intr(struct vmac_priv *priv);
static int qdpc_pcie_init_mem(struct vmac_priv *priv);
static int g_msi = 1;
int32_t qdpc_pcie_init_intr_and_mem(struct vmac_priv *priv)
{
	struct pci_dev *pdev = priv->pdev;
	int result  = 0;

	/*  Initialize interrupts */
	if (( result = qdpc_pcie_init_intr(priv)) < 0) {
		PRINT_ERROR("PCIe Interrupt Initialization failed \n");
		return result;
	}

	/* Memory Initialization */
	if (( result = qdpc_pcie_init_mem(priv)) < 0) {
		PRINT_ERROR("PCIe Memory Initialization failed \n");
		qdpc_free_interrupt(pdev);
	}

	return result;
}

static int32_t qdpc_pcie_init_intr(struct vmac_priv *priv)
{
	struct pci_dev *pdev = priv->pdev;

	priv->msi_enabled = 0; /* Set default to use Legacy INTx interrupt */

	/* Check if the device has MSI capability */
	if (use_msi) {
		if (!pci_enable_msi(pdev)) {
			PRINT_INFO("PCIe MSI Interrupt Enabled\n");
			priv->msi_enabled = 1;
		} else {
			PRINT_ERROR("PCIe MSI Interrupt enabling failed. Fall back to Legacy IRQ\n");
		}
	}

	if(!priv->msi_enabled) {
		PRINT_INFO("PCIe Legacy Interrupt Enabled\n");
		pci_intx(pdev, 1);
	}

	return 0;
}

static bool qdpc_bar_check(struct vmac_priv *priv, qdpc_bar_t *bar)
{
	uint32_t offset = bar->b_offset;
	size_t len = bar->b_len;
	dma_addr_t busaddr = bar->b_busaddr;
	uint8_t index = bar->b_index;

	if (index > 5) {
		printk("Invalid BAR index:%u. Must be between 0 and 5\n", index);
		return 0;
	}

	if (!len) {
		/* NOTE:
		  * Do not use an implicit length such as the BAR length
		  * if the map length is too large say > 16Mb this leaves
		  * the implementation vulnerable to
		  * Linux and the attack of the Silent  "S" (one between the n and u)
		  */
		printk("Zero length BAR\n");
		return 0;
	}

	if (busaddr) { /*initialized BAR */
		unsigned long bar_start =  pci_resource_start(priv->pdev , index);
		unsigned long bar_end =  pci_resource_end(priv->pdev , index);

		if (!bar_start) {
			printk("Invalid BAR address: 0x%p.\n", (void *)busaddr);
			return 0;
		}

		if ((busaddr - offset) != bar_start) {
			printk("Invalid BAR offset:0x%p. BAR starts at 0x%p\n",
				(void *)(busaddr -offset), (void *)bar_start);
			return 0;
		}
		/* Check the span of the BAR including the offset + length, bar_end points to the last byte of BAR */
		if ((busaddr + len - 1) > bar_end) {
			printk("Invalid BAR end address:0x%p. BAR ends at 0x%p\n",
				(void *)(busaddr + len), (void *)bar_end);
			return 0;
		}
	} else { /* Unitialized bar */
		unsigned long bar_end =  pci_resource_end(priv->pdev , index);
		busaddr = pci_resource_start(priv->pdev , index);

		if (!busaddr) {
			printk("Invalid BAR address: 0x%p.\n", (void *)busaddr);
			return 0;
		}

		/* Checks that offset area is within bar */
		if ( (busaddr + offset) > bar_end) {
			printk("Invalid BAR offset 0x%p, extends beyond end of BAR(0x%p).\n",
				(void *)(busaddr + offset), (void *)bar_end);
			return 0;
		}

		/* Checks that mapped area is within bar */
		if ((busaddr + len + offset - 1) > bar_end) {
			printk("Mapped area 0x%p, extends beyond end of BAR(0x%p).\n",
				(void *)(busaddr + len + offset - 1), (void *)bar_end);
			return 0;
		}
	}

	return 1;
}

static qdpc_bar_t *qdpc_map_bar(struct vmac_priv *priv, qdpc_bar_t *bar,
						uint8_t index, size_t len, uint32_t offset)
{
	void *vaddr = NULL;
	dma_addr_t busaddr = 0;
	qdpc_bar_t temp_bar;

	memset(&temp_bar, 0 ,sizeof(qdpc_bar_t));

	temp_bar.b_len = len;
	temp_bar.b_offset = offset;
	temp_bar.b_index = index;

	if (!qdpc_bar_check(priv, &temp_bar)) {
		printk("Failed bar mapping sanity check in %s\n", __FUNCTION__);
		return NULL;
	}

	/* Reserve PCIe memory region*/
	busaddr = pci_resource_start(priv->pdev , index) + offset;
	if (!request_mem_region(busaddr, len , QDPC_DEV_NAME)) {
		printk("Failed to reserve %u bytes of PCIe memory "
			"region starting at 0x%p\n", (uint32_t)len, (void *)busaddr);
		return NULL;
	}

#ifndef DISABLE_PCIE_UPDATA_HW_BAR
	qdpc_update_hw_bar(priv->pdev, index);
#endif

	vaddr = ioremap_nocache(busaddr, len);
	if (!vaddr) {
		printk("Failed to map %u bytes at BAR%u at bus address 0x%p.\n",
			(uint32_t)len, index, (void *)busaddr);
		release_mem_region(busaddr, len);
		return NULL;
	}

	memset(&temp_bar, 0 ,sizeof(qdpc_bar_t));

	bar->b_vaddr = vaddr;
	bar->b_busaddr = busaddr;
	bar->b_len = len;
	bar->b_index = index;
	bar->b_offset = offset;

	printk("BAR:%u vaddr=0x%p busaddr=%p offset=%u len=%u\n",
		bar->b_index, bar->b_vaddr, (void *)bar->b_busaddr,
		bar->b_offset, (uint32_t)bar->b_len);
	return bar;
}

static bool qdpc_unmap_bar(struct vmac_priv *priv, qdpc_bar_t *bar)
{
	if (!qdpc_bar_check(priv, bar)) {
		PRINT_ERROR("Failed bar mapping sanity check in %s\n", __FUNCTION__);
		return 0;
	}

	iounmap(bar->b_vaddr);
	release_mem_region(bar->b_busaddr - bar->b_offset, bar->b_len);
	memset(bar, 0 , sizeof(qdpc_bar_t));

	return 1;

}
static void qdpc_map_epmem(struct vmac_priv *priv)
{
	printk("%s() Mapping epmem\n", __FUNCTION__);
	qdpc_map_bar(priv, &priv->epmem_bar, QDPC_SHMEM_BAR,
					pci_resource_len(priv->pdev, QDPC_SHMEM_BAR) , 0);

	priv->bda =(qdpc_pcie_bda_t *)QDPC_BAR_VADDR(priv->epmem_bar, 0);
	priv->bda->bda_rc_msi_enabled = g_msi;
}

static void qdpc_map_sysctl_regs(struct vmac_priv *priv)
{
	printk("%s() Mapping sysctl\n", __FUNCTION__);
	qdpc_map_bar(priv, &priv->sysctl_bar, QDPC_SYSCTL_BAR, pci_resource_len(priv->pdev, QDPC_SYSCTL_BAR) , 0);
}

static void qdpc_map_dma_regs(struct vmac_priv *priv)
{
	printk("%s() Mapping dma registers\n", __FUNCTION__);
	qdpc_map_bar(priv, &priv->dmareg_bar, QDPC_DMA_BAR, pci_resource_len(priv->pdev, QDPC_DMA_BAR), 0);
}

static void qdpc_unmap_epmem(struct vmac_priv *priv)
{
	printk("%s() Unmapping sysctl\n", __FUNCTION__);
	priv->bda = NULL;
	qdpc_unmap_bar(priv, &priv->epmem_bar);
}

static void qdpc_unmap_sysctl_regs(struct vmac_priv *priv)
{
	printk("%s() Unmapping sysctl\n", __FUNCTION__);

	qdpc_unmap_bar(priv, &priv->sysctl_bar);
}

static void qdpc_unmap_dma_regs(struct vmac_priv *priv)
{
	printk("%s() Unmapping dma regs\n", __FUNCTION__);
	qdpc_unmap_bar(priv, &priv->dmareg_bar);
}

int32_t qdpc_set_dma_mask(struct vmac_priv *priv) {
	int result = 0;
	uint64_t dma_mask = qdpc_pci_readl(&priv->bda->bda_dma_mask);

	printk("Requested DMA mask:0x%llx\n", dma_mask);

	result = pci_set_dma_mask(priv->pdev, dma_mask);
	if (!result) {
			result = pci_set_consistent_dma_mask(priv->pdev, dma_mask);
			if (result) {
				printk(" pci_set_consistent_dma_mask() error %d. Mask:0x%llx\n", result, dma_mask);
				return 1;
			}
	} else {
		printk(" pci_set_dma_mask() error %d. Mask:0x%llx\n", result, dma_mask);
		return 1;
	}

	return 0;
}
static int32_t qdpc_pcie_init_mem(struct vmac_priv *priv)
{
	int ret = 0;

	/* Map SynControl registers and Host to Endpoint interrupt registers to BAR-2 */
	qdpc_map_sysctl_regs(priv);
	qdpc_map_epmem(priv);
	qdpc_map_dma_regs(priv);

	return ret;
}

int qdpc_unmap_iomem(struct vmac_priv *priv)
{
	qdpc_unmap_dma_regs(priv);
	qdpc_unmap_epmem(priv);
	qdpc_unmap_sysctl_regs(priv);

	return SUCCESS;
}

void qdpc_free_interrupt(struct pci_dev *pdev)
{
	struct net_device *ndev = pci_get_drvdata(pdev);
	struct vmac_priv *priv;

	if (ndev == NULL)
		return;

	priv = netdev_priv(ndev);
	if(priv->msi_enabled)
		pci_disable_msi(pdev);
	else
		pci_intx(pdev, 0);
}

void qdpc_pcie_free_mem(struct pci_dev *pdev)
{
	return;
}

void *qdpc_map_pciemem(unsigned long busaddr, size_t len)
{
	/* Reserve PCIe memory region*/
	if (!request_mem_region(busaddr, len, QDPC_DEV_NAME)) {
		PRINT_ERROR(KERN_ERR "Failed to reserve %u bytes of "
			"PCIe memory region starting at 0x%lx\n", (uint32_t)len, busaddr);
		return NULL;
	}
	return ioremap_nocache(busaddr, len);
}

void qdpc_unmap_pciemem(unsigned long busaddr, void *vaddr, size_t len)
{
	if (!vaddr || !busaddr)
		return;
	iounmap(vaddr);
	release_mem_region(busaddr, len);
}

void qdpc_deassert_intx(struct vmac_priv *priv)
{
	void *basereg = QDPC_BAR_VADDR(priv->sysctl_bar, TOPAZ_PCIE_CFG0_OFFSET);

	qdpc_pcie_posted_write(priv->ep_pciecfg0_val & ~TOPAZ_ASSERT_INTX, basereg);
}

