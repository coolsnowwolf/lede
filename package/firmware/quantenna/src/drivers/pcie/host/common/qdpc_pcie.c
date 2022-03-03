/**
 * Copyright (c) 2011-2013 Quantenna Communications, Inc.
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
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <asm/byteorder.h>
#include <linux/pci.h>
#include <asm-generic/pci-dma-compat.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_regs.h"
#include "qdpc_ruby.h"
#include <qdpc_platform.h>

static irqreturn_t qdpc_isr(int irq, void *dev_id);
static void qdpc_rx_tasklet(void *dev);
static void qdpc_txd_tasklet(void *dev);
static int qdpc_pcie_init_intr(struct net_device *pdev);
static int qdpc_pcie_init_mem(struct net_device *ndev);

#define MSI_64_EN (1 << 7)

static void qdpc_tx_runout_func(unsigned long data)
{
	struct qdpc_priv *priv = (struct qdpc_priv*)data;
	qdpc_pdring_t *txq = &priv->pktq.pkt_dsq;
	int32_t budget = qdpc_pci_readl(priv->host_ep2h_txd_budget);

	tasklet_schedule(&priv->txd_tasklet);

	DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "Restarting tx queue\n");
	DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "budget %d, free %d, pending %d\n",
		budget, txq->pd_nfree, txq->pd_npending);
}

int32_t qdpc_pcie_init_intr_and_mem(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	struct pci_dev *pdev = priv->pdev;
	int result  = 0;

	/*  Initialize interrupts */
	if (( result = qdpc_pcie_init_intr(ndev)) < 0) {
		PRINT_ERROR("PCIe Interrupt Initialization failed \n");
		return result;
	}

	/* Memory Initialization */
	if (( result = qdpc_pcie_init_mem(ndev)) < 0) {
		PRINT_ERROR("PCIe Memory Initialization failed \n");
		qdpc_free_interrupt(pdev);
	}

	return result;
}

static int32_t qdpc_pcie_init_intr(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	struct pci_dev  *pdev = priv->pdev;
	int ret  = 0, pos = 0;
	u16 msi_ctl = 0 ;

	/* Check if the device has MSI capability */
	pos = pci_find_capability(pdev, PCI_CAP_ID_MSI);
	if (!pos) {
		PRINT_ERROR("Device doesn't have MSI capability INTx will be instead\n");
	} else {
		pci_read_config_word(pdev, pos + PCI_MSI_FLAGS, &msi_ctl);
	    /* Check if the device has enabled MSI */
		if (msi_ctl & PCI_MSI_FLAGS_ENABLE) {
			/*  Enable MSI support */
			if (!pci_enable_msi(pdev)) {
				priv->msi_enabled = 1;
				printk("Host using MSI interrupt\n");
			} else {
				PRINT_ERROR("Enable MSI fail\n");
				return -1;
			}
		} else {
			PRINT_ERROR("Device doesn't enable MSI capability, INTx will be instead\n");
		}
	}

	tasklet_init(&priv->rx_tasklet, (void *)qdpc_rx_tasklet, (unsigned long)ndev);
	tasklet_disable(&priv->rx_tasklet);
	tasklet_init(&priv->txd_tasklet, (void *)qdpc_txd_tasklet, (unsigned long)ndev);
	tasklet_disable(&priv->txd_tasklet);

	priv->txq_enable_timer.function = qdpc_tx_runout_func;
	priv->txq_enable_timer.data = (unsigned long)priv;
	init_timer(&priv->txq_enable_timer);

	/*  Request the interrupt line with HSM device driver name */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22))
	ret = request_irq(pdev->irq, qdpc_isr,
		SA_SHIRQ, QDPC_DEV_NAME, ndev);
#else
	ret = request_irq(pdev->irq, qdpc_isr,
		          IRQF_SHARED | IRQF_DISABLED, QDPC_DEV_NAME, ndev);
#endif

	if (ret) {
		PRINT_ERROR("Failed to allocate interrupt line %d\n", pdev->irq);
		goto out;
	}

	return ret;

out:
	if (1 == priv->msi_enabled) {
		pci_disable_msi(pdev);
	}

	return ret;
}

static void qdpc_deassert_intx(struct qdpc_priv *priv)
{
	 unsigned long pcie_cfg0 = 0x0;
	 void *basereg = QDPC_BAR_VADDR(priv->sysctl_bar, QDPC_SYSCFG_REG);

	 pcie_cfg0 = qdpc_pci_readl(basereg);
	 if (pcie_cfg0 & BIT(9)) {
		 pcie_cfg0 &= ~(BIT(9));
		 qdpc_pcie_posted_write(pcie_cfg0, basereg);
	 }
}

static irqreturn_t qdpc_isr(int irq,   void *dev)
{
	struct net_device *ndev = (struct net_device *)dev;
	struct qdpc_priv *priv = netdev_priv(ndev);

	uint32_t isrstatus = qdpc_pci_readl(priv->ep2host_irqstatus);
	qdpc_pcie_posted_write(0, priv->ep2host_irqstatus);

	if (isrstatus & QDPC_EP_RXDONE) {
		tasklet_schedule(&priv->rx_tasklet);
	}

	if (isrstatus & QDPC_EP_TXDONE) {
		tasklet_schedule(&priv->txd_tasklet);
	}

	/* This still needs to be investigated further  as this alters the
	  * contention timing if removed between PCIe and
	  * the other parts of the BBIC register file
	  */
	//if (priv->msi_enabled) {
		qdpc_deassert_intx(priv);
	//}

	return IRQ_HANDLED;
}

void qdpc_free_interrupt(struct pci_dev *pdev)
{
	struct net_device *ndev = pci_get_drvdata(pdev);
	struct qdpc_priv *priv = netdev_priv(ndev);

	free_irq(pdev->irq, ndev);

	if (1 == priv->msi_enabled) {
		pci_disable_msi(pdev);
	}

	tasklet_kill(&priv->rx_tasklet);
	tasklet_kill(&priv->txd_tasklet);

	del_timer(&priv->txq_enable_timer);

	return;
}

static void qdpc_rx_tasklet(void *dev)
{
	qdpc_veth_rx((struct net_device *)dev);
}

static void qdpc_txd_tasklet(void *dev)
{
	qdpc_veth_txdone((struct net_device *)dev);
}

void qdpc_interrupt_target(struct qdpc_priv *priv, uint32_t intr)
{
	unsigned long flags;
	void *basereg = QDPC_BAR_VADDR(priv->sysctl_bar, QDPC_H2EP_INTERRUPT);
	__iomem uint32_t *irqstatus = priv->host2ep_irqstatus;

	spin_lock_irqsave(&priv->lock, flags);
	qdpc_pcie_posted_write((intr)|qdpc_pci_readl(irqstatus), irqstatus);
	qdpc_pcie_posted_write(QDPC_H2EP_INTERRUPT_BIT, basereg);
	spin_unlock_irqrestore(&priv->lock, flags);
}

static bool qdpc_bar_check(struct qdpc_priv *priv, qdpc_bar_t *bar)
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

static qdpc_bar_t *qdpc_map_bar(struct qdpc_priv *priv, qdpc_bar_t *bar,
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

static bool qdpc_unmap_bar(struct qdpc_priv *priv, qdpc_bar_t *bar)
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
static void qdpc_map_epmem(struct qdpc_priv *priv)
{
	printk("%s() Mapping epmem\n", __FUNCTION__);
	qdpc_map_bar(priv, &priv->epmem_bar, QDPC_SHMEM_BAR,
					pci_resource_len(priv->pdev, QDPC_SHMEM_BAR) , 0);

	priv->bda =(qdpc_pcie_bda_t *)QDPC_BAR_VADDR(priv->epmem_bar, 0);

	/* Init IRQ status pointers */
	priv->host2ep_irqstatus = &priv->bda->bda_h2ep_irqstatus;
	priv->ep2host_irqstatus = &priv->bda->bda_ep2h_irqstatus;

	priv->host_ep2h_txd_budget = &priv->bda->bda_ep2h_txd_budget;
	priv->host_h2ep_txd_budget = &priv->bda->bda_h2ep_txd_budget;
}

static void qdpc_map_sysctl_regs(struct qdpc_priv *priv)
{
	printk("%s() Mapping sysctl\n", __FUNCTION__);
	qdpc_map_bar(priv, &priv->sysctl_bar, QDPC_SYSCTL_BAR, pci_resource_len(priv->pdev, QDPC_SYSCTL_BAR) , 0);
}

static void qdpc_unmap_epmem(struct qdpc_priv *priv)
{
	printk("%s() Unmapping sysctl\n", __FUNCTION__);
	priv->bda = NULL;
	qdpc_unmap_bar(priv, &priv->epmem_bar);
}

static void qdpc_unmap_sysctl_regs(struct qdpc_priv *priv)
{
	printk("%s() Unmapping sysctl\n", __FUNCTION__);

	qdpc_unmap_bar(priv, &priv->sysctl_bar);
}

int32_t qdpc_set_dma_mask(struct qdpc_priv *priv) {
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
static int32_t qdpc_pcie_init_mem(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	int ret = 0;

	/* Map SynControl registers and Host to Endpoint interrupt registers to BAR-2 */
	qdpc_map_sysctl_regs(priv);
	qdpc_map_epmem(priv);

	/* Initialize Tx and Rx buffers*/
	qdpc_datapath_init(priv);

	return ret;
}

int qdpc_unmap_iomem(struct qdpc_priv *priv)
{
	qdpc_unmap_epmem(priv);
	qdpc_unmap_sysctl_regs(priv);

	return SUCCESS;
}

int qdpc_dma_setup(struct qdpc_priv *priv)
{
	qdpc_epshmem_hdr_t *shm;

	shm = (qdpc_epshmem_hdr_t *)QDPC_BAR_VADDR(priv->epmem_bar, sizeof(qdpc_pcie_bda_t));

	priv->epmem.eps_mapsize = qdpc_pci_readl(&shm->eps_mapsize);
	priv->epmem.eps_ver = qdpc_pci_readl(&shm->eps_ver);
	priv->epmem.eps_size = qdpc_pci_readl(&shm->eps_size);
	priv->epmem.eps_dma_offset = qdpc_pci_readl(&shm->eps_dma_offset);

	priv->epmem.eps_dsdma_desc = qdpc_pci_readl(&shm->eps_dsdma_desc);
	priv->epmem.eps_usdma_desc = qdpc_pci_readl(&shm->eps_usdma_desc);

	priv->epmem.eps_dsdma_ndesc = qdpc_pci_readl(&shm->eps_dsdma_ndesc);
	priv->epmem.eps_usdma_ndesc = qdpc_pci_readl(&shm->eps_usdma_ndesc);

	priv->epmem.eps_maxbuf = qdpc_pci_readl(&shm->eps_maxbuf);
	priv->epmem.eps_minbuf = qdpc_pci_readl(&shm->eps_minbuf);
	priv->epmem.eps_align = qdpc_pci_readl(&shm->eps_align);

	printk("Remaddr: 0x%p Size:%u\n", (void *)shm, priv->epmem.eps_mapsize);

	printk("Shmem: Len:%u Maplen:%u Ver0x%x\nUSDesc %u@0x%x DSDesc %u@0x%x DMA:%u/%u/%u\n",
		priv->epmem.eps_size, priv->epmem.eps_mapsize, priv->epmem.eps_ver,
		priv->epmem.eps_usdma_ndesc, priv->epmem.eps_usdma_desc,
		priv->epmem.eps_dsdma_ndesc, priv->epmem.eps_dsdma_desc,
		priv->epmem.eps_maxbuf, priv->epmem.eps_minbuf, priv->epmem.eps_align);

	if (qdpc_dmainit_rxq(priv) < 0) {
		printk("RX Queue DMA fail to initialize.\n");
		return FAILURE;
	}

	if (qdpc_dmainit_txq(priv) < 0) {
		printk("TX Queue DMA fail to initialize.\n");
		return FAILURE;
	}

	return SUCCESS;
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
