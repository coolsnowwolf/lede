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
#include <linux/etherdevice.h> 
#include <linux/skbuff.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_regs.h"
#include "qdpc_ruby.h"

int qdpc_dmainit_txq(struct qdpc_priv *priv)
{
	uint32_t i;
	qdpc_pdring_t *txq;
	uint32_t ringsize = priv->epmem.eps_dsdma_ndesc;

	/* Downstream TX queue */
	txq = &priv->pktq.pkt_dsq;
	txq->pd_hwdesc = (qdpc_dmadesc_t *)QDPC_BAR_VADDR(priv->epmem_bar, priv->epmem.eps_dsdma_desc);

#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	/* Downstream TX data . Temporary buffer used for bringup on X86*/
	priv->dsdata = kmalloc(ringsize * sizeof(qdpc_dmadata_t), GFP_KERNEL | GFP_DMA);
	if (!(priv->dsdata)) {
		printk("ERROR: TX DMA allocation fails.\n");
		return -1;
	}
	printk("TX DMA bounce buffers: 0x%p\n", priv->dsdata);
#endif

	/* Initialize Downstream TX queue */
	spin_lock_init(&txq->pd_lock);
	STAILQ_INIT(&txq->pd_pending);
	txq->pd_ringsize = ringsize;
	txq->pd_nfree = txq->pd_ringsize;
	txq->pd_npending = 0;

	memset(txq->pd_desc, 0, sizeof(txq->pd_desc));
	memset(txq->pd_hwdesc, 0, (ringsize*sizeof(qdpc_dmadesc_t)));
	for (i = 0 ; i < txq->pd_ringsize ; i++)
	{
		/* Buffer contains entire packet */
		txq->pd_desc[i].dd_hwdesc = &txq->pd_hwdesc[i];
		txq->pd_desc[i].dd_paddr = 0;
		qdpc_pci_writel((QDPC_DMA_SINGLE_TXBUFFER | QDPC_DMA_TX_NOCRC),
			&txq->pd_hwdesc[i].dma_control);
		qdpc_pci_writel(0, &txq->pd_hwdesc[i].dma_status);
		mmiowb(); /* Flush out writes */

#ifdef QTN_PCIE_USE_LOCAL_BUFFER
		/* X86 only attach local buffer */
		txq->pd_desc[i].dd_metadata = &priv->dsdata[i];
#endif
	}
	/* Mark end of buffer */
	qdpc_pci_writel((QDPC_DMA_LAST_DESC) | qdpc_pci_readl(&txq->pd_hwdesc[i-1].dma_control),
		&txq->pd_hwdesc[i-1].dma_control);
	txq->pd_lastdesc = &txq->pd_desc[i-1];
	txq->pd_firstdesc = &txq->pd_desc[0];
	txq->pd_nextdesc = txq->pd_firstdesc;

	printk("ringsize = %d txq->pd_hwdesc = 0x%p\n", ringsize, &txq->pd_hwdesc[0]);
	for (i = 0; i < (ringsize < 16 ? ringsize : 16); i++)
		printk("0x%p:td0=0x%x t1=0x%x t2=0x%x\n", &txq->pd_hwdesc[i],
			qdpc_pci_readl(&(txq->pd_hwdesc[i].dma_status)),
			qdpc_pci_readl(&(txq->pd_hwdesc[i].dma_control)),
			qdpc_pci_readl(&(txq->pd_hwdesc[i].dma_data)));

	return 0;

}

int qdpc_dmainit_rxq(struct qdpc_priv *priv)
{
	uint32_t i;
	qdpc_dmadesc_t *rx_hwdesc = NULL;
	uint32_t buffer_size = QDPC_DMA_MAXBUF;

	qdpc_pdring_t *rxq = &priv->pktq.pkt_usq;
	uint32_t ringsize = priv->epmem.eps_usdma_ndesc;
#if !defined(QTN_PCIE_USE_LOCAL_BUFFER)
	struct sk_buff *skb;
#endif

	spin_lock_init(&rxq->pd_lock);
	rxq->pd_ringsize = ringsize;

/* X86 workaround to use local buffer */
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	priv->usdata = kmalloc(ringsize * sizeof(qdpc_dmadata_t), GFP_KERNEL | GFP_DMA);
	if (!(priv->usdata)) {
		printk("ERROR: RX DMA allocation fails.\n");
		return -1;
	}
	printk("RX DMA bounce buffers: 0x%p\n", priv->usdata);
#endif
	rxq->pd_hwdesc = (qdpc_dmadesc_t *)QDPC_BAR_VADDR(priv->epmem_bar, priv->epmem.eps_usdma_desc);
	for (i = 0 ; i < ringsize; i++) {
		rx_hwdesc = &rxq->pd_hwdesc[i];
		rxq->pd_desc[i].dd_hwdesc = &rxq->pd_hwdesc[i];

/* X86 workaround to use local buffer */
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
		rxq->pd_desc[i].dd_metadata = &priv->usdata[i];
		rxq->pd_desc[i].dd_vaddr = rxq->pd_desc[i].dd_metadata;
#else
		skb = qdpc_get_skb(priv, buffer_size);
		skb->len = 0;
		rxq->pd_desc[i].dd_metadata = (void*)skb;
		rxq->pd_desc[i].dd_vaddr = skb->data;
		memset(skb->data, 0, buffer_size);
#endif

		rxq->pd_desc[i].dd_size = buffer_size;
		rxq->pd_desc[i].dd_paddr = pci_map_single(priv->pdev, rxq->pd_desc[i].dd_vaddr,
			rxq->pd_desc[i].dd_size, PCI_DMA_FROMDEVICE);

#if !defined(CONFIG_ARCH_ARC)
		if (pci_dma_mapping_error(priv->pdev, rxq->pd_desc[i].dd_paddr)) {
			printk("%s(), ERROR: pci_map_single fails.\n", __func__);
			return -1;
		}
#endif

		qdpc_pci_writel(rxq->pd_desc[i].dd_paddr + priv->epmem.eps_dma_offset,
			&rx_hwdesc->dma_data);
		qdpc_pci_writel((buffer_size & QDPC_DMA_LEN_MASK), &rx_hwdesc->dma_control);
		qdpc_pci_writel(0, &rx_hwdesc->dma_ptr);
		qdpc_pci_writel(QDPC_DMA_OWN, &rx_hwdesc->dma_status);
		mmiowb(); /* Flush out writes */

	}

	/* Mark end of buffer */
	qdpc_pci_writel((QDPC_DMA_LAST_DESC) | qdpc_pci_readl(&rx_hwdesc->dma_control),
		&rx_hwdesc->dma_control);
	rxq->pd_desc[rxq->pd_ringsize - 1].dd_flags |= QDPC_DMA_LAST_DESC;
	rxq->pd_lastdesc = &rxq->pd_desc[rxq->pd_ringsize - 1];
	rxq->pd_firstdesc = &rxq->pd_desc[0];
	rxq->pd_nextdesc = rxq->pd_firstdesc;

	printk("ringsize = %d rxq->pd_hwdesc = 0x%p\n", ringsize, &rxq->pd_hwdesc[0]);
	for (i = 0; i < (ringsize < 16 ? ringsize : 16); i++) {
		printk("0x%p:rd0=0x%x r1=0x%x r2=0x%x\n", &rxq->pd_hwdesc[i],
			qdpc_pci_readl(&(rxq->pd_hwdesc[i].dma_status)),
			qdpc_pci_readl(&(rxq->pd_hwdesc[i].dma_control)),
			qdpc_pci_readl(&(rxq->pd_hwdesc[i].dma_data)));
	}
	return 0;
}

int qdpc_dmauninit_txq(struct qdpc_priv *priv)
{
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	kfree(priv->dsdata);
#endif
	return 0;
}

int qdpc_dmauninit_rxq(struct qdpc_priv *priv)
{
#ifdef QTN_PCIE_USE_LOCAL_BUFFER
	kfree(priv->usdata);
#endif
	return 0;
}

void qdpc_datapath_init(struct qdpc_priv *priv)
{
	/* Init DMA ring buffers */
	priv->pktq.pkt_usq.pd_nextdesc = NULL;
	priv->pktq.pkt_dsq.pd_nextdesc = NULL;

	return;
}

void qdpc_datapath_uninit(struct qdpc_priv *priv)
{
	qdpc_dmauninit_txq(priv);
	qdpc_dmauninit_rxq(priv);
}
