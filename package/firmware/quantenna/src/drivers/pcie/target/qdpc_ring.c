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
#include <common/topaz_platform.h>

#include <qtn/skb_recycle.h>
#include <qtn/qtn_global.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_emac.h"

SRAM_TEXT struct sk_buff *qdpc_get_skb(size_t len)
{
	const uint32_t align = dma_get_cache_alignment();
	struct sk_buff *skb = NULL;
	uint32_t off;
#if defined(QDPC_USE_SKB_RECYCLE)
	struct qtn_skb_recycle_list *recycle_list = qtn_get_shared_recycle_list();
	uint32_t size;
	if (recycle_list) {
		skb = qtn_skb_recycle_list_pop(recycle_list, &recycle_list->stats_pcie);
	}
	if (!skb) {
		size = qtn_rx_buf_size();
		if (len > size)
			size = len;

		skb = dev_alloc_skb(size + align);
	}
#else
	skb = dev_alloc_skb(len + align);
#endif
	if (skb) {
		off = ((uint32_t)((unsigned long)skb->data)) & (align - 1);
		if (off) {
			skb_reserve(skb, align - off);
		}
	}

	return skb;

}

int qdpc_init_rxq(struct qdpc_priv *priv, size_t ringsize)
{
	uint32_t i;
	uint32_t dma_control = 0 ;
	uint32_t dma_status = 0;
	uint32_t dma_data = 0;
	qdpc_dmadesc_t *rx_hwdesc = NULL;
	uint32_t skb_bufsize = QDPC_DMA_MAXBUF;
	uint32_t buffer_size = QDPC_DMA_MAXBUF;
	qdpc_pdring_t *rxq = &priv->pktq.pkt_dsq;

	spin_lock_init(&rxq->pd_lock);
	rxq->pd_ringsize = ringsize;

	rxq->pd_hwdesc = (qdpc_dmadesc_t*)dma_alloc_coherent(NULL,
		(rxq->pd_ringsize * sizeof(qdpc_dmadesc_t)),
		&rxq->pd_dst_busaddr, GFP_KERNEL | GFP_DMA);

	memset(rxq->pd_desc, 0, sizeof(rxq->pd_desc));
	for (i = 0 ; i < rxq->pd_ringsize ; i++ ) {
		struct sk_buff *skb = qdpc_get_skb(skb_bufsize);

		if (!skb)
			break;
		skb->len=0;
		rx_hwdesc = &rxq->pd_hwdesc[i];
		rxq->pd_desc[i].dd_hwdesc = &rxq->pd_hwdesc[i];
		rxq->pd_desc[i].dd_metadata = (void*)skb;
		memset(skb->data, 0, buffer_size);

		dma_status = (QDPC_DMA_OWN);
		dma_control = (buffer_size & QDPC_DMA_LEN_MASK);
		dma_data = dma_map_single((struct device *)priv->ndev, (void*)skb->data, buffer_size, DMA_FROM_DEVICE);

		arc_write_uncached_32(&rx_hwdesc->dma_control, dma_control);
		arc_write_uncached_32(&rx_hwdesc->dma_data, dma_data);
		arc_write_uncached_32(&rx_hwdesc->dma_ptr, 0);
		arc_write_uncached_32(&rx_hwdesc->dma_status, dma_status);
	}
	/* Mark end of buffer */
	arc_write_uncached_32(&rx_hwdesc->dma_control,(dma_control |QDPC_DMA_LAST_DESC));
	rxq->pd_desc[rxq->pd_ringsize - 1].dd_flags |= QDPC_DMA_LAST_DESC;
	rxq->pd_lastdesc = &rxq->pd_desc[rxq->pd_ringsize - 1];
	rxq->pd_firstdesc = &rxq->pd_desc[0];
	rxq->pd_nextdesc = rxq->pd_firstdesc;

	return 0;
}


int qdpc_init_txq(struct qdpc_priv *priv, size_t ringsize)
{
	uint32_t i;
	qdpc_dmadesc_t *tx_hwdesc = NULL;
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;

	spin_lock_init(&txq->pd_lock);
	STAILQ_INIT(&txq->pd_pending);
	txq->pd_ringsize = ringsize;
	txq->pd_nfree = txq->pd_ringsize;
	txq->pd_npending = 0;
	txq->pd_hwdesc = (qdpc_dmadesc_t*)dma_alloc_coherent(NULL,
		(txq->pd_ringsize * sizeof(qdpc_dmadesc_t)),
		&txq->pd_src_busaddr, GFP_KERNEL | GFP_DMA);

	memset(txq->pd_desc, 0, sizeof(txq->pd_desc));

	for (i = 0 ; i < txq->pd_ringsize ; i++ ) {

		tx_hwdesc = &txq->pd_hwdesc[i];
		txq->pd_desc[i].dd_hwdesc = &txq->pd_hwdesc[i];
		txq->pd_desc[i].dd_metadata = NULL;

		arc_write_uncached_32(&tx_hwdesc->dma_control, (QDPC_DMA_SINGLE_TXBUFFER));
		arc_write_uncached_32(&tx_hwdesc->dma_data, 0);
		arc_write_uncached_32(&tx_hwdesc->dma_ptr, 0);
		arc_write_uncached_32(&tx_hwdesc->dma_status, 0);

	}
	/* Mark end of buffer */
	arc_write_uncached_32(&tx_hwdesc->dma_control, QDPC_DMA_LAST_DESC);
	txq->pd_desc[txq->pd_ringsize - 1].dd_flags |= QDPC_DMA_LAST_DESC;
	txq->pd_lastdesc = &txq->pd_desc[txq->pd_ringsize - 1];
	txq->pd_firstdesc = &txq->pd_desc[0];
	txq->pd_nextdesc = txq->pd_firstdesc;

	return 0;
}

