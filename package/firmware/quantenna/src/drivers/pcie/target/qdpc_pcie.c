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
#include <linux/pci.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/kthread.h>

#include <common/topaz_platform.h>
#include <common/ruby_pcie_bda.h>
#include <qtn/mproc_sync.h>
#include <asm/hardware.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_regs.h"
#include "qdpc_ruby.h"
#include "qdpc_emac.h"


/* Utility macros. Move ? */
#ifndef MAX
#define MAX(X,Y) ((X) > (Y) ? X : Y)
#endif
#ifndef MIN
#define MIN(X,Y) ((X) < (Y) ? X : Y)
#endif

#define REG_WRITE(x,y)	(writel((y),(x)))
#define REG_READ(x)		(readl(x))

static void qdpc_tx_runout_func(unsigned long data)
{
	struct qdpc_priv *priv = (struct qdpc_priv*)data;
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;
	int32_t budget = arc_read_uncached_32(priv->host_h2ep_txd_budget);

	tasklet_schedule(&priv->txd_tasklet);

	DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "Restarting tx queue\n");
	DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "budget %d, free %d, pending %d\n",
		budget, txq->pd_nfree, txq->pd_npending);
}

static SRAM_TEXT inline void qdpc_intr_ep2host_unlocked(struct qdpc_priv *priv, uint32_t intr)
{
	intr |= arc_read_uncached_32(priv->ep2host_irqstatus);
	arc_write_uncached_32(priv->ep2host_irqstatus, intr);
	priv->ep2host_irq(priv);
}

SRAM_TEXT void qdpc_intr_ep2host(struct qdpc_priv *priv, uint32_t intr)
{
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);
	qdpc_intr_ep2host_unlocked(priv, intr);
	spin_unlock_irqrestore(&priv->lock, flags);
}

static SRAM_TEXT void qdpc_irq_msi(struct qdpc_priv *priv)
{
	writel(priv->host_msi_data, priv->msiaddr);
}

static SRAM_TEXT void qdpc_irq_legacy(struct qdpc_priv *priv)
{
	/* Enable legacy interrupt mechanism */
	unsigned long pcie_cfg0 = readl(RUBY_SYS_CTL_PCIE_CFG0);
	if (!(pcie_cfg0 & BIT(9))) {
		pcie_cfg0 |= BIT(9);
		writel(pcie_cfg0, RUBY_SYS_CTL_PCIE_CFG0);
	}
}

void qdpc_pcie_irqsetup(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);

	if ((arc_read_uncached_32(&priv->bda->bda_flags))& PCIE_BDA_MSI){
		priv->msiaddr = arc_read_uncached_32(&priv->bda->bda_msi_addr);
		priv->ep2host_irq = qdpc_irq_msi;
		PRINT_INFO("MSI Enabled@0x%x\n", priv->msiaddr);
	} else {
		priv->msiaddr = 0;
		priv->ep2host_irq = qdpc_irq_legacy;
		PRINT_INFO("MSI disabled using INTA\n");
	}
	priv->host_msi_data =  readl(RUBY_PCIE_REG_BASE + MSI_CTL_OFFSET + 0xc) & 0xffff;

	return ;
}

static SRAM_TEXT irqreturn_t qdpc_dsdma_isr(int irq,  void   *dev)
{
	struct net_device *ndev = (struct net_device *)dev;
	struct qdpc_priv   *priv = netdev_priv(ndev);
	uint32_t dsdma_rx_base = priv->pktq.pkt_dsq.pd_rx_basereg;
	uint32_t dma_status = REG_READ(dsdma_rx_base + EMAC_DMASTATUS);
	uint32_t isr_status = dma_status & DmaAllInts;

	if (isr_status & DmaRxDone) {
		tasklet_schedule(&priv->rx_tasklet);
	}

	/* Check for out of descriptor condition. Issue RX demand poll if desc avail  */
	if (isr_status & (DmaNoRxDesc | DmaRxMissedFrame)) {
		priv->dsdma_desc = REG_READ(dsdma_rx_base + EMAC_DMARDP);
		priv->dsdma_status = REG_READ(0x80000000 + priv->dsdma_desc );
		if (QDPC_DMA_OWNED(priv->dsdma_status)) {
			REG_WRITE(dsdma_rx_base + EMAC_DMARPD, 1);
		}
	}

	/* Restart RX if stopped */
	if ((isr_status & DmaRxStopped) && (QDPC_DMA_RX_STATUS(isr_status) == 0)) {
		REG_WRITE(dsdma_rx_base + EMAC_DMACTRL, DmaStartRx);
	}

	if (isr_status & (QDPC_DMA_DS_ERROR)) {
		if (isr_status & DmaMacInterrupt) {
			uint32_t macintr_status =  REG_READ(dsdma_rx_base + EMAC_MACINTR);

			if (macintr_status & QDPC_MAC_DS_INTR) {
				priv->dsisr_status |= (QDPC_MAC_DS_INTR << 9);
				REG_WRITE(dsdma_rx_base + EMAC_MACINTR,  QDPC_MAC_DS_INTR);
			}
		}
		priv->dsisr_status = dma_status;
		tasklet_schedule(&priv->dmastatus_tasklet);
		printk(KERN_EMERG "%d:%s: out of rx descriptors\n", __LINE__, __FUNCTION__);
	}

	REG_WRITE(dsdma_rx_base + EMAC_DMASTATUS , isr_status);
	return IRQ_HANDLED;
}

static SRAM_TEXT irqreturn_t qdpc_usdma_isr(int irq, void *dev)
{
	struct net_device *ndev = (struct net_device *)dev;
	struct qdpc_priv *priv = netdev_priv(ndev);
	uint32_t usdma_rx_base = priv->pktq.pkt_usq.pd_rx_basereg;
	uint32_t isr_status = (REG_READ(usdma_rx_base + EMAC_DMASTATUS)) & DmaAllInts;

	if (isr_status & (DmaRxDone | DmaNoRxDesc | DmaRxMissedFrame)) {
		qdpc_intr_ep2host_unlocked(priv, QDPC_EP_RXDONE);

		/* Issue demand poll if RX is stuck because of no descriptors */
		if (isr_status & (DmaNoRxDesc | DmaRxMissedFrame)) {
			REG_WRITE(usdma_rx_base + EMAC_DMARPD, 1);
		}

		/* Schedule the tx done anyway to clear up tx descriptors and let flow control run */
		tasklet_schedule(&priv->txd_tasklet);
	}

	/* Restart RX if stopped */
	if ((isr_status & DmaRxStopped) && (QDPC_DMA_RX_STATUS(isr_status) == 0)) {
		REG_WRITE(usdma_rx_base + EMAC_DMACTRL, DmaStartRx);
	}

	/* Clear DMA Rx Transfer Done IRQ */
	if (isr_status & (QDPC_DMA_US_MISC)) {

		if (isr_status & DmaMacInterrupt) {
			uint32_t macintr_status =  (REG_READ(usdma_rx_base + EMAC_MACINTR) & QDPC_MAC_US_INTR);

			if (macintr_status) {
				priv->usisr_status |= (macintr_status << 9);
				REG_WRITE(usdma_rx_base + EMAC_MACINTR, macintr_status);
			}
		}
		priv->usisr_status |= (isr_status & QDPC_DMA_US_INTR);
		tasklet_schedule(&priv->dmastatus_tasklet);
		printk(KERN_EMERG "%d:%s: out of rx descriptors\n", __LINE__, __FUNCTION__);
	}

	REG_WRITE(usdma_rx_base + EMAC_DMASTATUS, isr_status);
	return IRQ_HANDLED;
}
static SRAM_TEXT irqreturn_t qdpc_dma_isr(int irq, void *dev)
{
	qdpc_usdma_isr(irq, dev);
	qdpc_dsdma_isr(irq, dev);
	return IRQ_HANDLED;
}

static SRAM_TEXT irqreturn_t qdpc_host_isr(int irq, void *dev)
{
	struct net_device *ndev = (struct net_device *)dev;
	struct qdpc_priv   *priv = netdev_priv(ndev);
	qdpc_pdring_t *dsq = &priv->pktq.pkt_dsq;
	uint32_t irqstatus = arc_read_uncached_32(priv->host2ep_irqstatus);

	arc_write_uncached_32(priv->host2ep_irqstatus, 0);
	qtn_mproc_sync_irq_ack(RUBY_SYS_CTL_D2L_INT, QDPC_H2EP_INTERRUPT_BIT);

	if (irqstatus & QDPC_HOST_TXREADY) {
		REG_WRITE(dsq->pd_rx_basereg + EMAC_DMARPD, 1);
		udelay(1);
		REG_WRITE(dsq->pd_tx_basereg + EMAC_DMATPD, 1);
	}

	if (irqstatus & QDPC_HOST_TXDONE) {
		tasklet_schedule(&priv->txd_tasklet);
	}

	if (irqstatus & QDPC_HOST_START_RX) {
		tasklet_schedule(&priv->rx_tasklet);
	}

	return IRQ_HANDLED;
}

static SRAM_TEXT void qdpc_dmastatus_tasklet(void *dev)
{
	struct net_device *ndev = (struct net_device *)dev;
	struct qdpc_priv  *priv = netdev_priv(ndev);
	static uint32_t count=0;

	printk ("EDMA(%u) DS:0x%x/0x%x/0x%x US:0x%x\n",++count, priv->dsdma_desc, priv->dsdma_status,
														priv->dsisr_status, priv->usisr_status);
	priv->dsisr_status = 0;
	priv->usisr_status = 0;
	priv->dsdma_status = 0;
	priv->usdma_status = 0;
	priv->dsdma_desc = 0;
	priv->usdma_desc = 0;
}

static SRAM_TEXT void qdpc_txd_tasklet(void *dev)
{
	struct net_device  *ndev = (struct net_device *)dev;

	qdpc_veth_txdone(ndev);

	return;
}

static SRAM_TEXT void qdpc_rx_tasklet(void *dev)
{
	/* Data Rx function */
	qdpc_veth_rx((struct net_device *)dev);
}

inline static __sram_text unsigned long qdpc_align_val_up(unsigned long val, unsigned long step)
{
	return ((val + step - 1) & (~(step - 1)));
}

int qdpc_pcie_init_mem(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	const uint32_t align = MAX(dma_get_cache_alignment(), QDPC_DMA_ALIGN);
	uint32_t offset;
	uint32_t usdma_offset;
	uint32_t dsdma_offset;

	/* setup boot data area mapping */
	priv->bda = (void *)(RUBY_PCIE_BDA_ADDR + 0x80000000);

	/* ISR status area */
	priv->host2ep_irqstatus = &priv->bda->bda_h2ep_irqstatus;
	priv->ep2host_irqstatus = &priv->bda->bda_ep2h_irqstatus;
	priv->host_ep2h_txd_budget = &priv->bda->bda_ep2h_txd_budget;
	priv->host_h2ep_txd_budget = &priv->bda->bda_h2ep_txd_budget;

	/* Setup descriptor shared memory area */
	priv->shmem = (qdpc_epshmem_hdr_t *)&priv->bda[1];
	priv->shmem_busaddr= (dma_addr_t)RUBY_PCIE_BDA_ADDR ;
	printk("Shmem: VA:0x%p PA:0x%p\n", (void *)priv->shmem, (void *)priv->shmem_busaddr);

	/* Initial offset past shared memory header aligned on a cache line size boundary */
	offset = qdpc_align_val_up(sizeof(qdpc_pcie_bda_t) + sizeof(qdpc_epshmem_hdr_t), align);

	/* Assign to upstream DMA host descriptors */
	usdma_offset = offset;

	/* Recalculate  and align offset past upstream descriptors */
	offset += (sizeof(qdpc_dmadesc_t) * priv->ep_config.cf_usdma_ndesc);
	offset = qdpc_align_val_up(offset, align);

	/* Assign to downstream descriptors */
	dsdma_offset = offset;

	/* Recalculate  and align offset past downstream descriptors */
	offset += (sizeof(qdpc_dmadesc_t) * priv->ep_config.cf_dsdma_ndesc);
	offset = qdpc_align_val_up(offset, align);

	arc_write_uncached_32(&priv->shmem->eps_dsdma_desc, dsdma_offset);
	arc_write_uncached_32(&priv->shmem->eps_usdma_desc, usdma_offset);

	arc_write_uncached_32(&priv->shmem->eps_dsdma_ndesc, priv->ep_config.cf_dsdma_ndesc);
	arc_write_uncached_32(&priv->shmem->eps_usdma_ndesc,priv->ep_config.cf_usdma_ndesc);

	arc_write_uncached_32(&priv->shmem->eps_size, offset);
	arc_write_uncached_32(&priv->shmem->eps_dma_offset, arc_read_uncached_32(&priv->bda->bda_dma_offset));

	arc_write_uncached_32(&priv->shmem->eps_ver, 1);
	arc_write_uncached_32(&priv->shmem->eps_maxbuf, QDPC_DMA_MAXBUF);
	arc_write_uncached_32(&priv->shmem->eps_minbuf, QDPC_DMA_MINBUF);
	arc_write_uncached_32(&priv->shmem->eps_align, align);

	printk("%s() Sz:%u US_offset:%u DS_offset:%u\n",
				__func__, offset, usdma_offset, dsdma_offset);

	return SUCCESS;
}

static void qdpc_pcie_enable_tasklets(struct qdpc_priv *priv)
{
	tasklet_enable(&priv->rx_tasklet);
	tasklet_enable(&priv->txd_tasklet);
	tasklet_enable(&priv->dmastatus_tasklet);
}

static void qdpc_pcie_disable_tasklets(struct qdpc_priv *priv)
{
	tasklet_disable(&priv->rx_tasklet);
	tasklet_disable(&priv->txd_tasklet);
	tasklet_disable(&priv->dmastatus_tasklet);
}

int qdpc_pcie_init_intr(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	int ret = 0;

	ret = request_irq(RUBY_IRQ_DSP, qdpc_host_isr, IRQF_DISABLED, "PCIe(host)", ndev);
	if (ret) {
		PRINT_ERROR(KERN_ERR " Host Interrupt initialization failed\n");
		return FAILURE;
	}

	ret = request_irq(RUBY_IRQ_ENET0, qdpc_dma_isr, IRQF_DISABLED, "PCIe(dsdma)", ndev);
	if (ret) {
		PRINT_ERROR(KERN_ERR " PCIe(dsdma) Interrupt initialization failed\n");
		return FAILURE;
	}
	ret = request_irq(RUBY_IRQ_ENET1, qdpc_dma_isr, IRQF_DISABLED, "PCIe(usdma)", ndev);
	if (ret) {
		PRINT_ERROR(KERN_ERR " PCIe(usdma) Interrupt initialization failed\n");
		return FAILURE;
	}

	/* Initialize tasklets */
	tasklet_init(&priv->rx_tasklet, (void *)qdpc_rx_tasklet,  (unsigned long)ndev);
	tasklet_init(&priv->txd_tasklet, (void *)qdpc_txd_tasklet,  (unsigned long)ndev);
	tasklet_init(&priv->dmastatus_tasklet, (void *)qdpc_dmastatus_tasklet,  (unsigned long)ndev);

	priv->txq_enable_timer.function = qdpc_tx_runout_func;
	priv->txq_enable_timer.data = (unsigned long)priv;
	init_timer(&priv->txq_enable_timer);

	qdpc_pcie_disable_tasklets(priv);

	ret = qdpc_pcie_init_mem(priv->ndev);
	if (ret == FAILURE) {
		qdpc_free_interrupt(ndev);
		return FAILURE;
	}

	ret = qdpc_emac_init(priv);
	if (ret == FAILURE) {
		qdpc_free_interrupt(ndev);
		return FAILURE;
	}

	return SUCCESS;
}

void qdpc_free_interrupt(struct net_device *ndev)
{
	struct qdpc_priv    *priv = netdev_priv(ndev);

	free_irq(RUBY_IRQ_ENET0, ndev);
	free_irq(RUBY_IRQ_ENET1, ndev);
	free_irq(RUBY_IRQ_DSP, ndev);

	qdpc_pcie_disable_tasklets(priv);

	tasklet_kill(&priv->rx_tasklet);
	tasklet_kill(&priv->dmastatus_tasklet);
	tasklet_kill(&priv->txd_tasklet);

	del_timer(&priv->txq_enable_timer);
}

static inline void qdpc_setbootstate(struct qdpc_priv *p, uint32_t state) {
	__iomem qdpc_pcie_bda_t *bda = p->bda;

	qdpc_pcie_posted_write(state, &bda->bda_bootstate);
}

static  int qdpc_bootpoll(struct qdpc_priv *p, uint32_t state) {
	while (!kthread_should_stop() && (qdpc_isbootstate(p,state) == 0)) {
				set_current_state(TASK_UNINTERRUPTIBLE);
				schedule_timeout(HZ/20);
	}
	return 0;
}

void qdpc_init_work(struct work_struct *task)
{
	struct qdpc_priv *priv;
	unsigned char macaddr[ETH_ALEN];
	priv = container_of(task, struct qdpc_priv, init_work);

	PRINT_INFO("Waiting for host start signal\n");
	qdpc_bootpoll(priv, QDPC_BDA_FW_START);

	qdpc_pcie_irqsetup(priv->ndev);

	qdpc_setbootstate(priv, QDPC_BDA_FW_CONFIG);

	PRINT_INFO("Enable DMA engines\n");
	qdpc_bootpoll(priv, QDPC_BDA_FW_RUN);
	qdpc_pcie_enable_tasklets(priv);
	qdpc_emac_enable(priv);
	netif_start_queue(priv->ndev);

	/* Set MAC address used by host side */
	memcpy(macaddr, qdpc_basemac, ETH_ALEN);
	macaddr[0] = (macaddr[0] & 0x1F) | (((macaddr[0] & 0xE0) + 0x40) & 0xE0) | 0x02;
	/*
	 * The bda_pci_pre_status and bda_pci_endian fields are not used at runtime, so the
	 * MAC address is stored here in order to avoid updating the bootloader.
	 */
	memcpy(&priv->bda->bda_pci_pre_status, macaddr, ETH_ALEN);

	/* Enable IRQ */
	writel(QDPC_H2EP_INTERRUPT_MASK, RUBY_SYS_CTL_D2L_INT_MASK);

	qdpc_setbootstate(priv, QDPC_BDA_FW_RUNNING);
	PRINT_INFO("Connection established with Host\n");

}
