#include <linux/clk.h>
#include <linux/debugfs.h>
#include <linux/dma-mapping.h>
#include <linux/if_ether.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/seq_file.h>
#include <linux/tcp.h>
#include <net/page_pool/helpers.h>

#include "sfxgmac-ext.h"
#include "dma.h"
#include "eth.h"

struct xgmac_dma_desc_rx {
	struct xgmac_dma_desc norm;
	struct xgmac_dma_desc ctxt;
};

static void xgmac_dma_set_tx_head_ptr(struct xgmac_dma_priv *priv,
				      dma_addr_t addr, u32 queue)
{
	reg_write(priv, XGMAC_DMA_CH_TxDESC_LADDR(queue), lower_32_bits(addr));
}

static void xgmac_dma_set_rx_head_ptr(struct xgmac_dma_priv *priv,
				      dma_addr_t addr, u32 queue)
{
	reg_write(priv, XGMAC_DMA_CH_RxDESC_LADDR(queue), lower_32_bits(addr));
}

static void xgmac_dma_set_tx_tail_ptr(struct xgmac_dma_priv *priv,
				      dma_addr_t addr, u32 queue)
{
	reg_write(priv, XGMAC_DMA_CH_TxDESC_TAIL_LPTR(queue), lower_32_bits(addr));
}

static void xgmac_dma_set_rx_tail_ptr(struct xgmac_dma_priv *priv,
				      dma_addr_t addr, u32 queue)
{
	reg_write(priv, XGMAC_DMA_CH_RxDESC_TAIL_LPTR(queue), lower_32_bits(addr));
}

static void xgmac_dma_set_tx_desc_addr(struct xgmac_dma_desc *p,
				       dma_addr_t addr)
{
	p->des0 = cpu_to_le32(lower_32_bits(addr));
	p->des1 = 0;
}

static void xgmac_dma_set_rx_desc_addr(struct xgmac_dma_desc *p,
				       dma_addr_t addr)
{
	p->des0 = cpu_to_le32(lower_32_bits(addr));
	p->des1 = 0;
	p->des2 = 0;
}

static void xgmac_dma_set_rx_owner(struct xgmac_dma_desc *p, bool int_en)
{
	u32 val = XGMAC_RDES3_OWN;

	if (int_en)
		val |= XGMAC_RDES3_IOC;
	p->des3 = cpu_to_le32(val);
}

static void xgmac_dma_init_rx_desc(struct xgmac_dma_desc *p,
				   dma_addr_t addr, bool int_en)
{
	xgmac_dma_set_rx_desc_addr(p, addr);
	xgmac_dma_set_rx_owner(p, int_en);
}

static __always_inline void
xgmac_dma_rx_coe_hash(const struct net_device *dev, struct sk_buff *skb,
		      u32 rdes0, u32 rdes_ctx1)
{
	bool rxcoe, is_l3, is_l4, not_tunnel, not_frag;
	u32 hash;
	/* RX COE is only available if:
	 * 1. It's not an IP fragment
	 * 2. It's not a tunnel packet (4in6, PPPoE, etc.)
	 * 3. Its L4 protocol is known (TCP, UDP, or ICMP)
	 *
	 * If all of the above are true, mark the skb as
	 * CHECKSUM_UNNECESSARY
	 *
	 * Note: bit-wise ops are used to avoid branches
	 */
	not_frag = FIELD_GET(XGMAC_RDES0_IP_FRAG, rdes0) == FRAG_NONE;
	not_tunnel = FIELD_GET(XGMAC_RDES1_TNP, rdes_ctx1) != 1;
	is_l3 = (FIELD_GET(XGMAC_RDES0_L3_TYPE, rdes0) == L3_TYPE_IPV4) |
		(FIELD_GET(XGMAC_RDES0_L3_TYPE, rdes0) == L3_TYPE_IPV6);
	is_l4 = FIELD_GET(XGMAC_RDES0_L4_TYPE, rdes0) != L4_TYPE_UNKNOWN;

	rxcoe = !!(dev->features & NETIF_F_RXCSUM) & not_frag & not_tunnel & is_l3 & is_l4;

	skb->ip_summed = rxcoe ? CHECKSUM_UNNECESSARY : CHECKSUM_NONE;

	/* Fill in skb->hash */
	hash = FIELD_GET(XGMAC_RDES1_RXHASH, rdes_ctx1);
	__skb_set_hash(skb, hash, false, is_l4);
}

static u32 xgmac_dma_tx_avail(struct xgmac_txq *txq)
{
	if (txq->dirty_tx > txq->cur_tx)
		return txq->dirty_tx - txq->cur_tx - 1;
	else
		return DMA_TX_SIZE - txq->cur_tx + txq->dirty_tx - 1;
}

static void xgmac_dma_rx_refill(struct xgmac_dma_priv *priv,
				struct xgmac_rxq *rxq)
{
	u32 entry = rxq->dirty_rx;
	u16 channel = rxq->idx;
	s32 dirty;

	if (rxq->dirty_rx <= rxq->cur_rx)
		dirty = rxq->cur_rx - rxq->dirty_rx;
	else
		dirty = DMA_RX_SIZE - rxq->dirty_rx + rxq->cur_rx;

	for (; dirty > 0; dirty -= 2) {
		struct xgmac_dma_rx_buffer *buf = &rxq->buf_pool[entry];
		struct xgmac_dma_desc *p = &rxq->dma_rx[entry];

		if (likely(buf->page == NULL)) {
			buf->page = page_pool_dev_alloc_frag(rxq->page_pool,
							     &buf->offset,
							     priv->rx_alloc_size);
			if (unlikely(!buf->page))
				break;
		}

		xgmac_dma_init_rx_desc(p, page_pool_get_dma_addr(buf->page) + buf->offset + BUF_PAD, false);
		/* No buffer space required by context descs */
		xgmac_dma_init_rx_desc(p + 1, 0, false);

		entry = (entry + 2) % DMA_RX_SIZE;
	}

	rxq->dirty_rx = entry;
	rxq->rx_tail_addr = rxq->dirty_rx * sizeof(struct xgmac_dma_desc) +
			    rxq->dma_rx_phy;

	xgmac_dma_set_rx_tail_ptr(priv, rxq->rx_tail_addr, channel);
}

static int xgmac_dma_poll_rx(struct xgmac_rxq *rxq, int budget)
{
	struct xgmac_dma_priv *priv = container_of(rxq, struct xgmac_dma_priv,
						   rxq[rxq->idx]);
	unsigned int next_entry = rxq->cur_rx;
	int count = 0;

	for (; count < budget; count++) {
		u32 len, rdes0, rdes2, rdes3, rdes_ctx0, rdes_ctx1, rdes_ctx2, rdes_ctx3, sta_index, rpt_index;
		struct xgmac_dma_rx_buffer *buf;
		register struct xgmac_dma_desc_rx rx;
		struct net_device *netdev;
		struct sk_buff *skb;
		unsigned int entry;
		u8 id, up_reason, vlan_pri, no_frag;
		u16 ovid, sport, eth_type, dscp, pkt_type, tnp;
		u64 smac;

		entry = next_entry;
		buf = &rxq->buf_pool[entry];

		rx = __READ_ONCE(*(struct xgmac_dma_desc_rx *)&rxq->dma_rx[next_entry]);
		/* check if owned by the DMA otherwise go ahead */
		if (unlikely(le32_to_cpu(rx.ctxt.des3) & XGMAC_RDES3_OWN))
			break;

		rxq->cur_rx = (entry + 2) % DMA_RX_SIZE;
		next_entry = rxq->cur_rx;

		rdes3 = le32_to_cpu(rx.norm.des3);
		if (unlikely(!(rdes3 & XGMAC_RDES3_LD)))
			continue;

		if (unlikely(rdes3 & XGMAC_RDES3_ES)) {
			pr_debug_ratelimited("error type: 0x%lx\n",
					FIELD_GET(XGMAC_RDES3_ET, rdes3));
			continue;
		}

		rdes0 = le32_to_cpu(rx.norm.des0);
		/* get ivport */
		id = FIELD_GET(XGMAC_RDES0_IVPORT, rdes0);
		netdev = priv->ndevs[id];
		if (unlikely(!netdev))
			continue;

		/* When memory is tight, the buf->addr may be empty */
		if (unlikely(!buf->page))
			break;

		len = FIELD_GET(XGMAC_RDES3_PL, rdes3);
		dma_sync_single_for_cpu(priv->dev, page_pool_get_dma_addr(buf->page) + buf->offset + BUF_PAD, len, DMA_FROM_DEVICE);
		prefetch(page_address(buf->page) + buf->offset + BUF_PAD);
		skb = napi_build_skb(page_address(buf->page) + buf->offset, priv->rx_alloc_size);
		if (unlikely(!skb))
			break;

		buf->page = NULL;
		skb_mark_for_recycle(skb);
		skb_reserve(skb, BUF_PAD);
		__skb_put(skb, len);

		rdes2 = le32_to_cpu(rx.norm.des2);
		ovid = FIELD_GET(XGMAC_RDES2_OVID, rdes2);
		no_frag = FIELD_GET(XGMAC_RDES2_DFRAG, rdes2);
		vlan_pri = FIELD_GET(XGMAC_RDES3_TCI_PRI, rdes3);
		sta_index = FIELD_GET(XGMAC_RDES0_STA_INDEX, rdes0);
		rpt_index = FIELD_GET(XGMAC_RDES0_RPT_INDEX, rdes0);

		/* get the context descriptor */
		rdes_ctx0 = le32_to_cpu(rx.ctxt.des0);
		rdes_ctx1 = le32_to_cpu(rx.ctxt.des1);
		rdes_ctx2 = le32_to_cpu(rx.ctxt.des2);
		rdes_ctx3 = le32_to_cpu(rx.ctxt.des3);
		sport = FIELD_GET(XGMAC_RDES0_SPORT, rdes_ctx0);
		eth_type = FIELD_GET(XGMAC_RDES0_ETH_TYPE, rdes_ctx0);
		dscp = FIELD_GET(XGMAC_RDES1_DSCP, rdes_ctx1);
		tnp = FIELD_GET(XGMAC_RDES1_TNP, rdes_ctx1);
		up_reason = FIELD_GET(XGMAC_RDES1_UP_REASON, rdes_ctx1);
		smac = rdes_ctx2 | FIELD_GET(XGMAC_RDES3_SMAC_32_47, rdes_ctx3) << 32;
		pkt_type = FIELD_GET(XGMAC_RDES3_PKT_TYPE, rdes_ctx3);
		pr_debug_ratelimited("%s: up_reason:%02x sta_index:%u rpt_index:%u "
				"ovid:%u vlan_pri:%u no_frag:%u sport:%u eth_type:0x%x "
				"dscp:%u tnp:%u pkt_type:%u smac:%llx\n",
				netdev->name, up_reason, sta_index, rpt_index,
				ovid, vlan_pri, no_frag, sport, eth_type,
				dscp, tnp, pkt_type, smac);

		xgmac_dma_rx_coe_hash(netdev, skb, rdes0, rdes_ctx1);

		skb_record_rx_queue(skb, rxq->idx);
		skb->protocol = eth_type_trans(skb, netdev);
		napi_gro_receive(&rxq->napi, skb);
	}

	xgmac_dma_rx_refill(priv, rxq);

	return count;
}

static int xgmac_dma_poll_tx(struct xgmac_txq *txq, int budget)
{
	struct xgmac_dma_priv *priv = container_of(txq, struct xgmac_dma_priv,
						   txq[txq->idx]);
	u32 bytes_compl[DPNS_HOST_PORT] = {}, pkts_compl[DPNS_HOST_PORT] = {};
	u32 entry, count = 0, i;

	spin_lock_bh(&txq->lock);

	entry = txq->dirty_tx;
	while (entry != txq->cur_tx && count < budget) {
		struct xgmac_dma_desc *p = &txq->dma_tx[entry];
		struct sk_buff *skb = txq->tx_skbuff[entry];
		u32 tdes3 = le32_to_cpu(READ_ONCE(p->des3));

		/* Check if the descriptor is owned by the DMA */
		if (unlikely(tdes3 & XGMAC_TDES3_OWN))
			break;

		/* Check if the descriptor is a context descriptor */
		if (unlikely(tdes3 & XGMAC_TDES3_CTXT))
			goto next;

		count++;
		/* Make sure descriptor fields are read after reading
		 * the own bit.
		 */
		dma_rmb();

		if (likely(txq->tx_skbuff_dma[entry].buf)) {
			if (txq->tx_skbuff_dma[entry].map_as_page)
				dma_unmap_page(priv->dev,
					       txq->tx_skbuff_dma[entry].buf,
					       txq->tx_skbuff_dma[entry].len,
					       DMA_TO_DEVICE);
			else
				dma_unmap_single(priv->dev,
						 txq->tx_skbuff_dma[entry].buf,
						 txq->tx_skbuff_dma[entry].len,
						 DMA_TO_DEVICE);
			txq->tx_skbuff_dma[entry].buf = 0;
			txq->tx_skbuff_dma[entry].len = 0;
			txq->tx_skbuff_dma[entry].map_as_page = false;
		}

		txq->tx_skbuff_dma[entry].last_segment = false;

		if (likely(skb)) {
			u8 id = XGMAC_SKB_CB(skb)->id;

			if (XGMAC_SKB_CB(skb)->fastmode) {
				pkts_compl[id]++;
				bytes_compl[id] += skb->len;
			}
			napi_consume_skb(skb, budget);
			txq->tx_skbuff[entry] = NULL;
		}

next:
		entry = (entry + 1) % DMA_TX_SIZE;
	}
	txq->dirty_tx = entry;

	for (i = 0; i < DPNS_HOST_PORT; i++) {
		struct net_device *dev = priv->ndevs[i];
		struct netdev_queue *queue;

		if (!dev)
			continue;

		queue = netdev_get_tx_queue(dev, txq->idx);
		netdev_tx_completed_queue(queue, pkts_compl[i], bytes_compl[i]);

		if (unlikely(netif_tx_queue_stopped(queue) &&
			     xgmac_dma_tx_avail(txq) > DMA_TX_SIZE / 4))
			netif_tx_wake_queue(queue);
	}

	spin_unlock_bh(&txq->lock);

	return count;
}

static int xgmac_dma_napi_rx(struct napi_struct *napi, int budget)
{
	struct xgmac_rxq *rxq = container_of(napi, struct xgmac_rxq, napi);
	int work_done;

	work_done = xgmac_dma_poll_rx(rxq, budget);
	if (work_done < budget && napi_complete_done(napi, work_done))
		enable_irq(rxq->irq);

	return work_done;
}

static int xgmac_dma_napi_tx(struct napi_struct *napi, int budget)
{
	struct xgmac_txq *txq = container_of(napi, struct xgmac_txq, napi);
	int work_done = xgmac_dma_poll_tx(txq, budget);

	if (work_done < budget && napi_complete_done(napi, work_done))
		enable_irq(txq->irq);

	return work_done;
}

static irqreturn_t xgmac_dma_irq_misc(int irq, void *dev_id)
{
	struct xgmac_dma_priv *priv = dev_id;
	u32 mtl_status = reg_read(priv, XGMAC_MTL_INT_STATUS);
	u32 dma_status = reg_read(priv, XGMAC_DMA_INT_STATUS);
	u32 i;

	//dev_info(priv->dev, "irq %d mtl %#08x dma %#08x\n",
	//		irq, mtl_status, dma_status);

	for (i = 0; i < DMA_CH_MAX; i++) {
		if (BIT(i) & mtl_status) {
			u32 status = reg_read(priv, XGMAC_MTL_QINT_STATUS(i));

			if (status & XGMAC_RXOVFIS)
				pr_debug_ratelimited("RX queue %u overflow\n", i);

			/* Clear interrupts */
			reg_write(priv, XGMAC_MTL_QINT_STATUS(i), status);
		}

		if (BIT(i) & dma_status) {
			u32 status = reg_read(priv, XGMAC_DMA_CH_STATUS(i));
			status &= ~(XGMAC_TI | XGMAC_TBU | XGMAC_RI);

			/* ABNORMAL interrupts */
			if (unlikely(status & XGMAC_AIS)) {
				if (status & XGMAC_CDE)
					dev_info(priv->dev, "Trigger queue %u CDE\n", i);

				if (status & XGMAC_DDE)
					dev_info(priv->dev, "Trigger queue %u DDE\n", i);

				if (status & XGMAC_FBE) {
					dev_info(priv->dev, "Trigger queue %u FBE\n", i);
					/* TODO: restart TX */
				}

				if (status & XGMAC_RBU) {
					pr_debug_ratelimited("Trigger queue %u RBU\n", i);
				}
			}

			reg_write(priv, XGMAC_DMA_CH_STATUS(i), status);
		}
	}

	return IRQ_HANDLED;
}

static irqreturn_t xgmac_dma_irq_tx(int irq, void *dev_id)
{
	struct xgmac_txq *txq = dev_id;
	struct xgmac_dma_priv *priv = container_of(txq, struct xgmac_dma_priv,
						   txq[txq->idx]);
	u16 channel = txq->idx;
	u32 status = reg_read(priv, XGMAC_DMA_CH_STATUS(channel)) &
		(XGMAC_TBU | XGMAC_TI);

	if (unlikely(!status))
		return IRQ_NONE;

	/* Clear interrupts */
	reg_write(priv, XGMAC_DMA_CH_STATUS(channel), status);

	/* TX NORMAL interrupts */
	if (likely(napi_schedule_prep(&txq->napi))) {
		/* Disable TX interrupt */
		disable_irq_nosync(irq);

		/* Turn on polling */
		__napi_schedule_irqoff(&txq->napi);
	}

	return IRQ_HANDLED;
}

static irqreturn_t xgmac_dma_irq_rx(int irq, void *dev_id)
{
	struct xgmac_rxq *rxq = dev_id;
	struct xgmac_dma_priv *priv = container_of(rxq, struct xgmac_dma_priv,
						   rxq[rxq->idx]);
	u16 channel = rxq->idx;
	u32 status = reg_read(priv, XGMAC_DMA_CH_STATUS(channel)) &
		(XGMAC_RI);

	if (unlikely(!status))
		return IRQ_NONE;

	/* Clear interrupts */
	reg_write(priv, XGMAC_DMA_CH_STATUS(channel), status);

	/* RX NORMAL interrupts */
	if (likely(napi_schedule_prep(&rxq->napi))) {
		/* Disable RX interrupt */
		disable_irq_nosync(irq);

		/* Turn on polling */
		__napi_schedule_irqoff(&rxq->napi);
	}

	return IRQ_HANDLED;
}

static int xgmac_dma_soft_reset(struct xgmac_dma_priv *priv)
{
	unsigned long timeout = jiffies + HZ;

	reg_write(priv, XGMAC_DMA_MODE, XGMAC_SWR);
	do {
		if (!(reg_read(priv, XGMAC_DMA_MODE) & XGMAC_SWR))
			return 0;

		cond_resched();
	} while (time_after(timeout, jiffies));

	dev_err(priv->dev, "DMA reset timed out\n");
	return -ETIMEDOUT;
}

static int xgmac_dma_init(struct xgmac_dma_priv *priv)
{
	int ret;
	u32 i;

	/* DMA SW reset */
	ret = xgmac_dma_soft_reset(priv);
	if (ret)
		return ret;

	/* DMA Configuration */
	/* Exclude per-channel interrupts from sbd_intr_o */
	reg_write(priv, XGMAC_DMA_MODE, FIELD_PREP(XGMAC_INTM, 1));
	reg_rmw(priv, XGMAC_DMA_SYSBUS_MODE,
		XGMAC_RD_OSR_LMT | XGMAC_WR_OSR_LMT,
		FIELD_PREP(XGMAC_RD_OSR_LMT, 31) | XGMAC_EN_LPI |
		FIELD_PREP(XGMAC_WR_OSR_LMT, 31) | XGMAC_UNDEF);

	reg_write(priv, XGMAC_TX_EDMA_CTRL, 1);
	reg_write(priv, XGMAC_RX_EDMA_CTRL, 1);

	/* enable rx_queue 0 1 2 3 */
	regmap_write(priv->ethsys, ETHSYS_RX_QUEUE_ENABLE, 0xAA);

	/* Use static RX Queue to DMA mapping
	 * queue 0 to channel 0
	 * queue 1 to channel 1
	 * queue 2 to channel 2
	 * queue 3 to chennel 3
	 * queue 4 to channel 4
	*/
	reg_write(priv, XGMAC_MTL_RXQ_DMA_MAP0, 0x03020100);
	reg_write(priv, XGMAC_MTL_RXQ_DMA_MAP1, 0x4);

	/* DMA Channel Configuration
	 * TXQs share 8KB, RXQs share 16KB
	 *
	 * Configured queue size = 256B * (1 + (TQS or RQS field))
	 *
	 * The maximum limit of PBL is queue size / datawidth / 2
	 *
	 * TxPBL must be limited to 32 for Tx COE to work on 1500 MTU, see the
	 * comment in xgmac_dma_xmit() for detail.
	 */
	for (i = 0; i < DMA_CH_MAX; i++) {
		/* set RxPBL to 16 beats for ddr schedule with 128bit (16*8) */
		reg_rmw(priv, XGMAC_DMA_CH_RX_CONTROL(i), XGMAC_RxPBL,
			FIELD_PREP(XGMAC_RxPBL, 16));
		/* set TxPBL to 16 beats for ddr schedule with 128bit (16*8) */
		reg_rmw(priv, XGMAC_DMA_CH_TX_CONTROL(i), XGMAC_TxPBL,
			FIELD_PREP(XGMAC_TxPBL, 16) | XGMAC_TSE | XGMAC_OSP);

		/* Enable TX queue, store-and-forward mode
		 * each queue size 2k bytes
		 */
		reg_rmw(priv, XGMAC_MTL_TXQ_OPMODE(i), XGMAC_TQS | XGMAC_TXQEN,
			FIELD_PREP(XGMAC_TQS, 0x7) | XGMAC_TSF |
				FIELD_PREP(XGMAC_TXQEN, 0x2));

		/* Enable RX queue
		 * each queue size 4k bytes
		 */
		reg_write(priv, XGMAC_MTL_RXQ_OPMODE(i),
			  FIELD_PREP(XGMAC_RQS, 0xf));
	}

	return 0;
}

static void xgmac_dma_free_rx_buffer(struct xgmac_dma_priv *priv, struct xgmac_rxq *rxq, u32 i)
{
	struct xgmac_dma_rx_buffer *buf = &rxq->buf_pool[i];

	if (buf->page) {
		page_pool_put_full_page(rxq->page_pool, buf->page, false);
		buf->page = NULL;
	}
}

static int xgmac_dma_init_rx_buffers(struct xgmac_dma_priv *priv, struct xgmac_rxq *rxq, u32 i,
				     struct xgmac_dma_desc *p, u16 rx_alloc_size)
{
	struct xgmac_dma_rx_buffer *buf = &rxq->buf_pool[i];

	buf->page = page_pool_dev_alloc_frag(rxq->page_pool, &buf->offset, rx_alloc_size);
	if (!buf->page)
		return -ENOMEM;

	xgmac_dma_init_rx_desc(p, page_pool_get_dma_addr(buf->page) + buf->offset + BUF_PAD, false);

	return 0;
}

static int xgmac_dma_init_rx_rings(struct xgmac_dma_priv *priv)
{
	u32 queue, i;
	int ret;

	for (queue = 0; queue < DMA_CH_MAX; queue++) {
		struct xgmac_rxq *rxq = &priv->rxq[queue];

		for (i = 0; i < DMA_RX_SIZE; i += 2) {
			struct xgmac_dma_desc *p = &rxq->dma_rx[i];

			ret = xgmac_dma_init_rx_buffers(priv, rxq, i, p, priv->rx_alloc_size);
			if (ret)
				goto err_init_rx_buffers;
			/* No buffer space required by context descs */
			xgmac_dma_init_rx_desc(p + 1, 0, false);
		}

		rxq->cur_rx = 0;
		rxq->dirty_rx = (unsigned int)(i - DMA_RX_SIZE);
	}

	return 0;

err_init_rx_buffers:
	while (queue >= 0) {
		while (--i >= 0)
			xgmac_dma_free_rx_buffer(priv, &priv->rxq[queue], i);

		if (queue == 0)
			break;

		i = DMA_RX_SIZE;
		queue--;
	}

	return ret;
}

static void xgmac_dma_init_tx_rings(struct xgmac_dma_priv *priv)
{
	u32 queue, i;

	for (queue = 0; queue < DMA_CH_MAX; queue++) {
		struct xgmac_txq *txq = &priv->txq[queue];

		for (i = 0; i < DMA_TX_SIZE; i++) {
			struct xgmac_dma_desc *p = &txq->dma_tx[i];

			memset(p, 0, sizeof(*p));

			txq->tx_skbuff_dma[i].buf = 0;
			txq->tx_skbuff_dma[i].map_as_page = false;
			txq->tx_skbuff_dma[i].len = 0;
			txq->tx_skbuff_dma[i].last_segment = false;
			txq->tx_skbuff[i] = NULL;
		}

		txq->dirty_tx = 0;
		txq->cur_tx = 0;
	}
}

static void xgmac_dma_free_tx_buffer(struct xgmac_txq *txq, u32 i)
{
	struct xgmac_dma_priv *priv = container_of(txq, struct xgmac_dma_priv,
						   txq[txq->idx]);

	if (txq->tx_skbuff_dma[i].buf) {
		if (txq->tx_skbuff_dma[i].map_as_page)
			dma_unmap_page(priv->dev,
				       txq->tx_skbuff_dma[i].buf,
				       txq->tx_skbuff_dma[i].len,
				       DMA_TO_DEVICE);
		else
			dma_unmap_single(priv->dev,
					 txq->tx_skbuff_dma[i].buf,
					 txq->tx_skbuff_dma[i].len,
					 DMA_TO_DEVICE);
	}

	if (txq->tx_skbuff[i]) {
		dev_kfree_skb(txq->tx_skbuff[i]);
		txq->tx_skbuff[i] = NULL;
		txq->tx_skbuff_dma[i].buf = 0;
		txq->tx_skbuff_dma[i].map_as_page = false;
	}
}

static void xgmac_dma_free_rx_skbufs(struct xgmac_dma_priv *priv, struct xgmac_rxq *rxq)
{
	u32 i;

	for (i = 0; i < DMA_RX_SIZE; i++)
		xgmac_dma_free_rx_buffer(priv, rxq, i);
}

static void xgmac_dma_free_tx_skbufs(struct xgmac_txq *txq)
{
	u32 i;

	for (i = 0; i < DMA_TX_SIZE; i++)
		xgmac_dma_free_tx_buffer(txq, i);
}

static void xgmac_dma_free_rx_descs(struct xgmac_dma_priv *priv)
{
	u32 i;

	/* Free RX queue resources */
	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_rxq *rxq = &priv->rxq[i];

		/* Release the DMA RX socket buffers */
		xgmac_dma_free_rx_skbufs(priv, rxq);

		/* Free DMA RX descs */
#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
		gen_pool_free(priv->genpool, (uintptr_t)rxq->dma_rx,
			      DMA_RX_SIZE * sizeof(*rxq->dma_rx));
#else
		dma_free_coherent(priv->dev, DMA_RX_SIZE * sizeof(*rxq->dma_rx),
				  (void *)rxq->dma_rx, rxq->dma_rx_phy);
#endif

		kfree(rxq->buf_pool);
		if (rxq->page_pool)
			page_pool_destroy(rxq->page_pool);
	}
}

static void xgmac_dma_free_tx_descs(struct xgmac_dma_priv *priv)
{
	u32 i;

	/* Free TX queue resources */
	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_txq *txq = &priv->txq[i];
		spin_lock_bh(&txq->lock);
		txq->is_busy = true;
		spin_unlock_bh(&txq->lock);
		/* Release the DMA TX socket buffers */
		xgmac_dma_free_tx_skbufs(txq);

		/* Free DMA TX descs */
#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
		gen_pool_free(priv->genpool, (uintptr_t)txq->dma_tx,
			      DMA_TX_SIZE * sizeof(*txq->dma_tx));
#else
		dma_free_coherent(priv->dev, DMA_TX_SIZE * sizeof(*txq->dma_tx),
				  txq->dma_tx, txq->dma_tx_phy);
#endif
		txq->dma_tx = NULL;
		kfree(txq->tx_skbuff_dma);
		txq->tx_skbuff_dma = NULL;
		kfree(txq->tx_skbuff);
		txq->tx_skbuff = NULL;

		spin_lock_bh(&txq->lock);
		txq->is_busy = false;
		spin_unlock_bh(&txq->lock);
	}
}

static int xgmac_dma_alloc_rx_descs(struct xgmac_dma_priv *priv)
{
	struct page_pool_params pp_params = {};
	int ret = -ENOMEM;
	u32 i;

	pp_params.flags = PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV;
	pp_params.pool_size = DMA_RX_SIZE;
	pp_params.order = 0;
	pp_params.max_len = PAGE_SIZE;
	pp_params.nid = dev_to_node(priv->dev);
	pp_params.dev = priv->dev;
	pp_params.dma_dir = DMA_FROM_DEVICE;

	/* RX queues buffers and DMA */
	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_rxq *rxq = &priv->rxq[i];

		rxq->page_pool = page_pool_create(&pp_params);
		if (IS_ERR(rxq->page_pool)) {
			ret = PTR_ERR(rxq->page_pool);
			rxq->page_pool = NULL;
			goto err_dma;
		}

		rxq->buf_pool = kcalloc(DMA_RX_SIZE, sizeof(*rxq->buf_pool),
					GFP_KERNEL);
		if (!rxq->buf_pool)
			goto err_dma;

#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
		rxq->dma_rx = gen_pool_dma_alloc(priv->genpool, DMA_RX_SIZE *
						 sizeof(*rxq->dma_rx),
						 &rxq->dma_rx_phy);
#else
		rxq->dma_rx = dma_alloc_coherent(priv->dev, DMA_RX_SIZE *
						 sizeof(*rxq->dma_rx),
						 &rxq->dma_rx_phy, GFP_KERNEL);
#endif
		if (!rxq->dma_rx)
			goto err_dma;
	}

	return 0;

err_dma:
	xgmac_dma_free_rx_descs(priv);
	return ret;
}

static int xgmac_dma_alloc_tx_descs(struct xgmac_dma_priv *priv)
{
	int ret = -ENOMEM;
	u32 i;

	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_txq *txq = &priv->txq[i];

		txq->tx_skbuff_dma = kcalloc(DMA_TX_SIZE,
					     sizeof(*txq->tx_skbuff_dma),
					     GFP_KERNEL);
		if (!txq->tx_skbuff_dma)
			goto err_dma;

		txq->tx_skbuff = kcalloc(DMA_TX_SIZE,
					 sizeof(struct sk_buff *),
					 GFP_KERNEL);
		if (!txq->tx_skbuff)
			goto err_dma;

#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
		txq->dma_tx = gen_pool_dma_zalloc(priv->genpool, DMA_TX_SIZE *
						  sizeof(*txq->dma_tx),
						  &txq->dma_tx_phy);
#else
		txq->dma_tx = dma_alloc_coherent(priv->dev, DMA_TX_SIZE *
						 sizeof(*txq->dma_tx),
						 &txq->dma_tx_phy, GFP_KERNEL);
#endif
		if (!txq->dma_tx)
			goto err_dma;
	}

	return 0;

err_dma:
	xgmac_dma_free_tx_descs(priv);
	return ret;
}

static int xgmac_dma_enable(struct xgmac_dma_priv *priv)
{
	int ret;
	u32 i;

	ret = xgmac_dma_alloc_rx_descs(priv);
	if (ret)
		return ret;

	ret = xgmac_dma_init_rx_rings(priv);
	if (ret)
		return ret;

	ret = xgmac_dma_alloc_tx_descs(priv);
	if (ret)
		return ret;

	xgmac_dma_init_tx_rings(priv);

	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_rxq *rxq = &priv->rxq[i];
		struct xgmac_txq *txq = &priv->txq[i];

		/*
		 * Initiate the WDT with packet count of 32 to enable IRQ
		 * set watchdog timer 2048 * 100 * 2.5ns = 0.512ms
		 * */
		reg_write(priv, XGMAC_DMA_CH_Rx_WATCHDOG(i),
			  XGMAC_PSEL | FIELD_PREP(XGMAC_RBCT, 32) |
			  FIELD_PREP(XGMAC_RWTU, 3) | FIELD_PREP(XGMAC_RWT, 100));

		/* Set RX buffer size */
		reg_rmw(priv, XGMAC_DMA_CH_RX_CONTROL(i), XGMAC_RBSZ,
			FIELD_PREP(XGMAC_RBSZ, priv->rx_buffer_size));

		/* Set head pointer */
		xgmac_dma_set_rx_head_ptr(priv, rxq->dma_rx_phy, i);
		xgmac_dma_set_tx_head_ptr(priv, txq->dma_tx_phy, i);

		/* Set tail pointer */
		rxq->rx_tail_addr = DMA_RX_SIZE * sizeof(*rxq->dma_rx) +
				    rxq->dma_rx_phy;
		txq->tx_tail_addr = txq->dma_tx_phy;
		xgmac_dma_set_rx_tail_ptr(priv, rxq->rx_tail_addr, i);
		xgmac_dma_set_tx_tail_ptr(priv, txq->tx_tail_addr, i);

		/* Set ring length */
		reg_write(priv, XGMAC_DMA_CH_RxDESC_RING_LEN(i),
			  ((DMA_RX_SIZE - 1) | (FIELD_PREP(XGMAC_OWRQ, 7))));
		reg_write(priv, XGMAC_DMA_CH_TxDESC_RING_LEN(i),
			  DMA_TX_SIZE - 1);

		/* Enable NAPI poll */
		napi_enable(&rxq->napi);
		napi_enable(&txq->napi);

		/* Enable interrupt */
		reg_write(priv, XGMAC_DMA_CH_INT_EN(i),
			  XGMAC_DMA_INT_DEFAULT_EN);

		/* Enable MTL RX overflow interrupt */
		reg_write(priv, XGMAC_MTL_QINTEN(i), XGMAC_RXOIE);

		/* Start DMA */
		reg_set(priv, XGMAC_DMA_CH_RX_CONTROL(i), XGMAC_RXST);
		reg_set(priv, XGMAC_DMA_CH_TX_CONTROL(i), XGMAC_TXST);
	}

	return 0;
}

static void xgmac_dma_disable(struct xgmac_dma_priv *priv)
{
	u32 i;

	for (i = 0; i < DMA_CH_MAX; i++) {
		struct xgmac_rxq *rxq = &priv->rxq[i];
		struct xgmac_txq *txq = &priv->txq[i];

		/* Disable interrupts */
		reg_write(priv, XGMAC_DMA_CH_INT_EN(i), 0);
		reg_write(priv, XGMAC_MTL_QINTEN(i), 0);

		/* Disable DMA transfer */
		reg_clear(priv, XGMAC_DMA_CH_RX_CONTROL(i), XGMAC_RXST);
		reg_clear(priv, XGMAC_DMA_CH_TX_CONTROL(i), XGMAC_TXST);

		/* Disable NAPI poll */
		napi_disable(&rxq->napi);
		napi_disable(&txq->napi);

		/* Clear all pending interrupts */
		reg_write(priv, XGMAC_DMA_CH_STATUS(i), -1);
		reg_write(priv, XGMAC_MTL_QINT_STATUS(i), -1);
	}

	/* Free resources */
	xgmac_dma_free_rx_descs(priv);
	xgmac_dma_free_tx_descs(priv);
}

static void xgmac_dma_stop_queue(struct xgmac_dma_priv *priv, int queue)
{
	int i;

	for (i = 0; i < 6; i++) {
		struct net_device *dev = priv->ndevs[i];

		if (!dev)
			continue;

		netif_tx_stop_queue(netdev_get_tx_queue(dev, queue));
	}
}

static void xgmac_dma_tso_fill_desc(struct xgmac_txq *txq, dma_addr_t des,
				    unsigned int pay_len, bool last_segment)
{
	struct xgmac_dma_desc *desc;
	int tmp_len = pay_len;
	u32 entry;

	/* 1. put every 2 16K-1 buffers into one desc */
	while (tmp_len > TSO_MAX_BUFF_SIZE) {
		bool ld = (last_segment && tmp_len <= TSO_MAX_BUFF_SIZE * 2);
		entry = txq->cur_tx;
		entry = (entry + 1) % DMA_TX_SIZE;
		txq->cur_tx = entry;
		desc = &txq->dma_tx[entry];

		desc->des0 = cpu_to_le32(des);
		desc->des1 = cpu_to_le32(des + TSO_MAX_BUFF_SIZE);
		desc->des2 = cpu_to_le32((ld ? XGMAC_TDES2_IOC : 0) | XGMAC_TDES2_B1L |
					 FIELD_PREP(XGMAC_TDES2_B2L,
						    min(tmp_len - TSO_MAX_BUFF_SIZE,
						        TSO_MAX_BUFF_SIZE)));
		desc->des3 = cpu_to_le32(XGMAC_TDES3_OWN |
					 (ld ? XGMAC_TDES3_LD : 0));

		tmp_len -= TSO_MAX_BUFF_SIZE * 2;
		des += TSO_MAX_BUFF_SIZE * 2;
	}
	/* 2. put the last buffer, if exists */
	if (tmp_len > 0) {
		entry = txq->cur_tx;
		entry = (entry + 1) % DMA_TX_SIZE;
		txq->cur_tx = entry;
		desc = &txq->dma_tx[entry];

		desc->des0 = cpu_to_le32(des);
		desc->des1 = cpu_to_le32(0);
		desc->des2 = cpu_to_le32((last_segment ? XGMAC_TDES2_IOC : 0) |
					 FIELD_PREP(XGMAC_TDES2_B1L, tmp_len));
		desc->des3 = cpu_to_le32(XGMAC_TDES3_OWN |
					 (last_segment ? XGMAC_TDES3_LD : 0));
	}
}

static netdev_tx_t xgmac_dma_tso_xmit(struct sk_buff *skb,
				      struct xgmac_dma_priv *priv)
{
	u16 queue = skb_get_queue_mapping(skb);
	u16 channel = queue;
	u32 size = (queue == DMA_CH_DISABLE) ? SZ_2K : SZ_1_5K;
	struct xgmac_txq *txq = &priv->txq[queue];
	struct xgmac_dma_desc *desc, *first, *ctxt;
	struct xgmac_skb_cb *cb = XGMAC_SKB_CB(skb);
	u8 nfrags = skb_shinfo(skb)->nr_frags;
	struct net_device *dev = skb->dev;
	u32 first_entry, entry, i, tdes0, pay_len;
	u32 proto_hdr_len, hdr;
	bool last_segment;
	dma_addr_t des;
	u16 mss;

	hdr = tcp_hdrlen(skb);
	proto_hdr_len = skb_transport_offset(skb) + hdr;
	pay_len = skb_headlen(skb) - proto_hdr_len; /* no frags */

	spin_lock(&txq->lock);
	/* Desc availability based on threshold should be enough safe */
	if (unlikely(xgmac_dma_tx_avail(txq) <
		     max_t(u32, nfrags, (skb->len - proto_hdr_len) / TSO_MAX_BUFF_SIZE) + 2)) {
		xgmac_dma_stop_queue(priv, queue);
		pr_debug_ratelimited("%s: Tx Ring full when queue awake\n",
			__func__);
		spin_unlock(&txq->lock);
		return NETDEV_TX_BUSY;
	}

	mss = skb_shinfo(skb)->gso_size;
	/* The header length + MSS + TxPBL must be less than Tx Queue size */
	mss = min_t(u16, mss, size - 16 - proto_hdr_len - 1);

	entry = txq->cur_tx;
	desc = &txq->dma_tx[entry];
	ctxt = desc;
	/* Prepare TX context descriptor */
	tdes0 = XGMAC_TDES0_FAST_MODE |
		FIELD_PREP(XGMAC_TDES0_OVPORT, cb->id) |
		FIELD_PREP(XGMAC_TDES0_IVPORT, DPNS_HOST_PORT);
	ctxt->des0 = cpu_to_le32(tdes0);
	ctxt->des1 = 0;
	ctxt->des2 = cpu_to_le32(mss);

	entry = (entry + 1) % DMA_TX_SIZE;
	txq->cur_tx = first_entry = entry;
	desc = &txq->dma_tx[entry];
	first = desc;

	/* first descriptor: fill Headers on Buf1 */
	last_segment = (nfrags == 0);
	des = dma_map_single(priv->dev, skb->data, skb_headlen(skb),
			     DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(priv->dev, des)))
		goto drop;

	txq->tx_skbuff_dma[first_entry].buf = des;
	txq->tx_skbuff_dma[first_entry].len = skb_headlen(skb);
	first->des0 = cpu_to_le32(des);

	/* Fill start of payload in buff2 of first descriptor */
	first->des1 = cpu_to_le32(pay_len ? des + proto_hdr_len : 0);
	if (pay_len > TSO_MAX_BUFF_SIZE) {
		/* Need more descs if the buffer size > 16383 */
		pay_len -= TSO_MAX_BUFF_SIZE;
		first->des2 =
			cpu_to_le32(FIELD_PREP(XGMAC_TDES2_B1L, proto_hdr_len) |
				    XGMAC_TDES2_B2L);
		des += proto_hdr_len + TSO_MAX_BUFF_SIZE;
	} else {
		first->des2 =
			cpu_to_le32((last_segment ? XGMAC_TDES2_IOC : 0) |
				    FIELD_PREP(XGMAC_TDES2_B1L, proto_hdr_len) |
				    FIELD_PREP(XGMAC_TDES2_B2L, pay_len));
		pay_len = 0;
	}
	first->des3 = cpu_to_le32(
		XGMAC_TDES3_OWN | XGMAC_TDES3_FD |
		(last_segment && !pay_len ? XGMAC_TDES3_LD : 0) |
		FIELD_PREP(XGMAC_TDES3_THL, hdr / 4) | XGMAC_TDES3_TSE |
		FIELD_PREP(XGMAC_TDES3_TPL, skb->len - proto_hdr_len));

	/* Put the remaining headlen buffer */
	xgmac_dma_tso_fill_desc(txq, des, pay_len, last_segment);
	entry = txq->cur_tx;
	/* Prepare fragments */
	for (i = 0; i < nfrags; i++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];

		des = skb_frag_dma_map(priv->dev, frag, 0, skb_frag_size(frag),
				       DMA_TO_DEVICE);
		if (unlikely(dma_mapping_error(priv->dev, des)))
			goto drop;

		last_segment = (i == nfrags - 1);

		xgmac_dma_tso_fill_desc(txq, des, skb_frag_size(frag), last_segment);
		entry = txq->cur_tx;
		txq->tx_skbuff_dma[entry].buf = des;
		txq->tx_skbuff_dma[entry].len = skb_frag_size(frag);
		txq->tx_skbuff_dma[entry].map_as_page = true;
	}
	txq->tx_skbuff_dma[entry].last_segment = true;
	/* Only the last descriptor gets to point to the skb. */
	txq->tx_skbuff[entry] = skb;

	/* We've used all descriptors we need for this skb, however,
	 * advance cur_tx so that it references a fresh descriptor.
	 * ndo_start_xmit will fill this descriptor the next time it's
	 * called and xgmac_dma_poll_tx may clean up to this descriptor.
	 */
	entry = (entry + 1) % DMA_TX_SIZE;
	txq->cur_tx = entry;

	if (unlikely(xgmac_dma_tx_avail(txq) <= (MAX_SKB_FRAGS + 1)))
		xgmac_dma_stop_queue(priv, queue);

	skb_tx_timestamp(skb);
	ctxt->des3 = cpu_to_le32(XGMAC_TDES3_OWN | XGMAC_TDES3_CTXT |
				 XGMAC_TDES3_TCMSSV | XGMAC_TDES3_PIDV);
	dma_wmb();
	netdev_tx_sent_queue(netdev_get_tx_queue(dev, queue), skb->len);
	txq->tx_tail_addr = txq->dma_tx_phy + txq->cur_tx * sizeof(*desc);
	xgmac_dma_set_tx_tail_ptr(priv, txq->tx_tail_addr, channel);
	spin_unlock(&txq->lock);
	return NETDEV_TX_OK;
drop:
	dev->stats.tx_dropped++;
	spin_unlock(&txq->lock);
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}


static netdev_tx_t xgmac_dma_xmit(struct sk_buff *skb, struct xgmac_dma_priv *priv)
{
	u16 queue = skb_get_queue_mapping(skb);
	u16 channel = queue;
	u32 size = (queue == DMA_CH_DISABLE) ? SZ_2K : SZ_1_5K;
	struct xgmac_txq *txq = &priv->txq[queue];
	u8 nfrags = skb_shinfo(skb)->nr_frags;
	struct xgmac_dma_desc *desc, *first, *ctxt;
	struct xgmac_skb_cb *cb = XGMAC_SKB_CB(skb);
	struct net_device *dev = skb->dev;
	u32 nopaged_len = skb_headlen(skb);
	u32 first_entry, entry, i, tdes0, cic = 0;
	bool last_segment;
	dma_addr_t des;

	if (skb_is_gso(skb) && skb_is_gso_tcp(skb))
		return xgmac_dma_tso_xmit(skb, priv);

	if (likely(skb->ip_summed == CHECKSUM_PARTIAL)) {
		/* Tx COE only works with packets that are LESS THAN the
		 * following number of bytes in size:
		 * TXQ SIZE in bytes – ((PBL + 5)*(DATAWIDTH in bytes))
		 *
		 * Thus for queue size of 2K, Tx COE of 1500 MTU works only if
		 * PBL <= 32.
		 */
		const unsigned int txcoeovh = (16 + 5) * 8;

		if (unlikely(skb->len >= size - txcoeovh)) {
			if (unlikely(skb_checksum_help(skb)))
				goto drop_kfree;
		} else {
			cic = XGMAC_TDES3_CIC;
		}
	}

	spin_lock(&txq->lock);
	if (txq->is_busy || (NULL == txq->dma_tx) || (NULL == txq->tx_skbuff_dma) || (NULL == txq->tx_skbuff)) {
		spin_unlock(&txq->lock);
		return NETDEV_TX_BUSY;
	}

	/* We need at least 2 + nfrags free TX descriptors to xmit a packet */
	if (unlikely(xgmac_dma_tx_avail(txq) < nfrags + 2)) {
		xgmac_dma_stop_queue(priv, queue);
		pr_debug_ratelimited("%s: Tx Ring full when queue awake\n",
			__func__);
		spin_unlock(&txq->lock);
		return NETDEV_TX_BUSY;
	}

	entry = txq->cur_tx;
	desc = &txq->dma_tx[entry];
	ctxt = desc;

	/* Prepare TX context descriptor */
	if (cb->fastmode)
		tdes0 = XGMAC_TDES0_FAST_MODE |
			FIELD_PREP(XGMAC_TDES0_OVPORT, cb->id) |
			FIELD_PREP(XGMAC_TDES0_IVPORT, DPNS_HOST_PORT);
	else
		tdes0 = FIELD_PREP(XGMAC_TDES0_OVPORT, DPNS_HOST_PORT) |
			FIELD_PREP(XGMAC_TDES0_IVPORT, cb->id);

	ctxt->des0 = cpu_to_le32(tdes0);
	ctxt->des1 = 0;
	ctxt->des2 = 0;

	entry = (entry + 1) % DMA_TX_SIZE;
	first_entry = entry;
	desc = &txq->dma_tx[entry];
	first = desc;

	for (i = 0; i < nfrags; i++) {
		const skb_frag_t *frag = &skb_shinfo(skb)->frags[i];
		u32 len = skb_frag_size(frag);

		last_segment = (i == (nfrags - 1));
		entry = (entry + 1) % DMA_TX_SIZE;
		desc = &txq->dma_tx[entry];
		des = skb_frag_dma_map(priv->dev, frag, 0, len, DMA_TO_DEVICE);
		if (dma_mapping_error(priv->dev, des))
			goto drop;

		txq->tx_skbuff_dma[entry].buf = des;

		xgmac_dma_set_tx_desc_addr(desc, des);

		txq->tx_skbuff_dma[entry].map_as_page = true;
		txq->tx_skbuff_dma[entry].len = len;
		txq->tx_skbuff_dma[entry].last_segment = last_segment;
		/* Prepare the descriptor and set the own bit too */
		desc->des2 = cpu_to_le32(FIELD_PREP(XGMAC_TDES2_B1L, len) |
					 (last_segment ? XGMAC_TDES2_IOC : 0));
		desc->des3 = cpu_to_le32(XGMAC_TDES3_OWN | cic |
					 (last_segment ? XGMAC_TDES3_LD : 0) |
					 FIELD_PREP(XGMAC_TDES3_FL, skb->len));
	}

	/* Only the last descriptor gets to point to the skb. */
	txq->tx_skbuff[entry] = skb;

	/* We've used all descriptors we need for this skb, however,
	 * advance cur_tx so that it references a fresh descriptor.
	 * ndo_start_xmit will fill this descriptor the next time it's
	 * called and xgmac_dma_poll_tx may clean up to this descriptor.
	 */
	entry = (entry + 1) % DMA_TX_SIZE;
	txq->cur_tx = entry;

	if (unlikely(xgmac_dma_tx_avail(txq) <= (MAX_SKB_FRAGS + 1)))
		xgmac_dma_stop_queue(priv, queue);

	skb_tx_timestamp(skb);

	/* Ready to fill the first descriptor and set the OWN bit w/o any
	 * problems because all the descriptors are actually ready to be
	 * passed to the DMA engine.
	 */
	last_segment = (nfrags == 0);
	des = dma_map_single(priv->dev, skb->data, nopaged_len, DMA_TO_DEVICE);
	if (dma_mapping_error(priv->dev, des))
		goto drop;

	txq->tx_skbuff_dma[first_entry].buf = des;
	txq->tx_skbuff_dma[first_entry].len = nopaged_len;
	txq->tx_skbuff_dma[first_entry].last_segment = last_segment;
	/* Prepare the first descriptor setting the OWN bit too */
	xgmac_dma_set_tx_desc_addr(first, des);
	first->des2 = cpu_to_le32(FIELD_PREP(XGMAC_TDES2_B1L, nopaged_len) |
				  (last_segment ? XGMAC_TDES2_IOC : 0));
	first->des3 = cpu_to_le32(XGMAC_TDES3_OWN | XGMAC_TDES3_FD | cic |
				  (last_segment ? XGMAC_TDES3_LD : 0) |
				  FIELD_PREP(XGMAC_TDES3_FL, skb->len));

	ctxt->des3 = cpu_to_le32(XGMAC_TDES3_OWN | XGMAC_TDES3_CTXT | XGMAC_TDES3_PIDV);
	/* The descriptor must be set before tail poiner update and then barrier
	 * is needed to make sure that all is coherent before granting the
	 * DMA engine.
	 */
	dma_wmb();

	if (dev)
		netdev_tx_sent_queue(netdev_get_tx_queue(dev, queue), skb->len);
	txq->tx_tail_addr = txq->dma_tx_phy + txq->cur_tx * sizeof(*desc);
	xgmac_dma_set_tx_tail_ptr(priv, txq->tx_tail_addr, channel);
	spin_unlock(&txq->lock);
	return NETDEV_TX_OK;

drop:
	if (dev)
		dev->stats.tx_dropped++;
	spin_unlock(&txq->lock);
drop_kfree:
	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

netdev_tx_t xgmac_dma_xmit_fast(struct sk_buff *skb, struct net_device *dev)
{
	struct gmac_common *priv = netdev_priv(dev);
	struct xgmac_skb_cb *cb = XGMAC_SKB_CB(skb);
	netdev_tx_t ret;

	cb->id = priv->id;
	cb->fastmode = true;

	ret = xgmac_dma_xmit(skb, priv->dma);
	if (unlikely(ret != NETDEV_TX_OK)) {
		pr_debug_ratelimited("%s: Tx Ring full when queue awake\n",
			__func__);
	}
	return ret;
}
EXPORT_SYMBOL(xgmac_dma_xmit_fast);

int xgmac_dma_open(struct xgmac_dma_priv *priv, struct net_device *dev, u8 id)
{
	if (id >= ARRAY_SIZE(priv->ndevs))
		return -EINVAL;

	if (priv->ndevs[id])
		return -EBUSY;

	priv->ndevs[id] = dev;

	/* we run multiple netdevs on the same DMA ring so we only bring it
	 * up once
	 */
	if (!refcount_read(&priv->refcnt)) {
		int ret = xgmac_dma_enable(priv);
		if (ret) {
			priv->ndevs[id] = NULL;
			return ret;
		}

		refcount_set(&priv->refcnt, 1);
	} else {
		refcount_inc(&priv->refcnt);
	}

	return 0;
}
EXPORT_SYMBOL(xgmac_dma_open);

int xgmac_dma_stop(struct xgmac_dma_priv *priv, struct net_device *dev, u8 id)
{
	if (id >= ARRAY_SIZE(priv->ndevs) || priv->ndevs[id] != dev)
		return -EINVAL;

	/* only shutdown DMA if this is the last user */
	if (refcount_dec_and_test(&priv->refcnt))
		xgmac_dma_disable(priv);

	priv->ndevs[id] = NULL;

	return 0;
}
EXPORT_SYMBOL(xgmac_dma_stop);

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_PAGE_POOL_STATS)
static int xgmac_dma_stats_show(struct seq_file *m, void *v)
{
	struct xgmac_dma_priv *priv = m->private;
	int i;

	for (i = 0; i < DMA_CH_MAX; i++) {
		struct page_pool_stats stats = {};

		page_pool_get_stats(priv->rxq[i].page_pool, &stats);
		seq_printf(m, "RX alloc statistics:\n"
			"fast:\t%llu\n"
			"slow:\t%llu\n"
			"empty:\t%llu\n"
			"refill:\t%llu\n"
			"waive:\t%llu\n",
			stats.alloc_stats.fast, stats.alloc_stats.slow,
			stats.alloc_stats.empty, stats.alloc_stats.refill,
			stats.alloc_stats.waive);

		seq_printf(m, "RX recycle statistics:\n"
			"cached:\t%llu\n"
			"cache_full:\t%llu\n"
			"ring:\t%llu\n"
			"ring_full:\t%llu\n"
			"released_refcnt:\t%llu\n",
			stats.recycle_stats.cached, stats.recycle_stats.cache_full,
			stats.recycle_stats.ring, stats.recycle_stats.ring_full,
			stats.recycle_stats.released_refcnt);
	}
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(xgmac_dma_stats);
#endif

static int xgmac_dma_debug_show(struct seq_file *m, void *v)
{
	struct xgmac_dma_priv *priv = m->private;
	int i, j;

	for (i = 0; i < DMA_CH_MAX; i++) {
		j = (i == DMA_CH_DISABLE) ? DMA_OVPORT_CH : i;
		spin_lock_bh(&(priv->txq[i].lock));
		seq_printf(m, "txq %d curr:%d dirty:%d\n",
				i, priv->txq[i].cur_tx, priv->txq[i].dirty_tx);
		seq_printf(m, "      MTL Opmode:0x%x debug:0x%x\n",
				reg_read(priv, XGMAC_MTL_TXQ_OPMODE(j)),
				reg_read(priv, XGMAC_MTL_TXQ_DEBUG(j)));
		seq_printf(m, "rxq %d curr:%d dirty:%d\n",
				i, priv->rxq[i].cur_rx, priv->rxq[i].dirty_rx);
		seq_printf(m, "      MTL Opmode:0x%x debug:0x%x\n",
				reg_read(priv, XGMAC_MTL_RXQ_OPMODE(j)),
				reg_read(priv, XGMAC_MTL_RXQ_DEBUG(j)));
		spin_unlock_bh(&(priv->txq[i].lock));

		seq_printf(m, "DMA channel %d status:0x%x debug sts:0x%x\n", j,
				reg_read(priv, XGMAC_DMA_CH_STATUS(j)),
				reg_read(priv, XGMAC_DMA_CH_DEBUG_STATUS(j)));
		seq_printf(m, "              TxDesc HAddr:0x%x TAddr:0x%x\n",
				reg_read(priv, XGMAC_DMA_CH_TxDESC_LADDR(j)),
				reg_read(priv, XGMAC_DMA_CH_TxDESC_TAIL_LPTR(j)));
		seq_printf(m, "                     Cur desAddr:0x%x bufAddr:0x%x\n",
				reg_read(priv, XGMAC_DMA_CH_CUR_TxDESC_LADDR(j)),
				reg_read(priv, XGMAC_DMA_CH_CUR_TxBUFF_LADDR(j)));
		seq_printf(m, "              RxDesc HAddr:0x%x TAddr:0x%x\n",
				reg_read(priv, XGMAC_DMA_CH_RxDESC_LADDR(j)),
				reg_read(priv, XGMAC_DMA_CH_RxDESC_TAIL_LPTR(j)));
		seq_printf(m, "                     Cur desAddr:0x%x bufAddr:0x%x\n",
				reg_read(priv, XGMAC_DMA_CH_CUR_RxDESC_LADDR(j)),
				reg_read(priv, XGMAC_DMA_CH_CUR_RxBUFF_LADDR(j)));
	}

	seq_printf(m, "DMA debug sts0:0x%x sts1:0x%x sts3:0x%x\n",
			reg_read(priv, XGMAC_DMA_DEBUG_STATUS(0)),
			reg_read(priv, XGMAC_DMA_DEBUG_STATUS(1)),
			reg_read(priv, XGMAC_DMA_DEBUG_STATUS(3)));
	return 0;
}

DEFINE_SHOW_ATTRIBUTE(xgmac_dma_debug);

static int xgmac_dma_probe(struct platform_device *pdev)
{
	struct xgmac_dma_priv *priv;
	const char *irq_name;
	char buf[4];
	int ret;
	u32 i;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;
	priv->ioaddr = devm_platform_ioremap_resource(pdev, 0);
	if (IS_ERR(priv->ioaddr))
		return PTR_ERR(priv->ioaddr);

	/* Request all clocks at once */
	priv->clks[DMA_CLK_AXI].id = "axi";
	priv->clks[DMA_CLK_NPU].id = "npu";
	priv->clks[DMA_CLK_CSR].id = "csr";
	ret = devm_clk_bulk_get(&pdev->dev, DMA_NUM_CLKS, priv->clks);
	if (ret)
		return ret;

#ifdef CONFIG_NET_SIFLOWER_ETH_USE_INTERNAL_SRAM
	priv->genpool = of_gen_pool_get(pdev->dev.of_node, "iram", 0);
	if (!priv->genpool)
		return -ENODEV;
#endif

	priv->ethsys = syscon_regmap_lookup_by_phandle(pdev->dev.of_node,
						       "ethsys");
	if (IS_ERR(priv->ethsys))
		return PTR_ERR(priv->ethsys);

	/* lif/hif reset and release reset*/
	ret = regmap_clear_bits(priv->ethsys, ETHSYS_RST, BIT(7) | BIT(8));
	if (ret)
		return ret;

	ret = regmap_set_bits(priv->ethsys, ETHSYS_RST, BIT(7) | BIT(8));
	if (ret)
		return ret;

	/* set the mapping mode 0
	 * hash random use queue 0-3
	 */
	ret = regmap_write(priv->ethsys, ETHSYS_MRI_Q_EN, 0x003F003F);

	/* we run multiple netdevs on the same DMA ring so we need a dummy
	 * device for NAPI to work
	 */
	init_dummy_netdev(&priv->napi_dev);

	/* DMA IRQ */
	ret = platform_get_irq_byname(pdev, "sbd");
	if (ret < 0)
		return ret;

	priv->irq = ret;
	ret = devm_request_irq(&pdev->dev, ret, xgmac_dma_irq_misc, 0,
			       "xgmac_dma_sbd", priv);
	if (ret)
		return ret;

	irq_set_affinity_hint(priv->irq, cpumask_of(1));

	/* TX IRQ */
	for (i = 0; i < DMA_CH_MAX; i++) {
		snprintf(buf, sizeof(buf), "tx%u", i);
		ret = platform_get_irq_byname(pdev, buf);
		if (ret < 0)
			goto out_napi_del;

		priv->txq[i].irq = ret;

		irq_name = devm_kasprintf(&pdev->dev, GFP_KERNEL,
					  "xgmac_dma_txq%u", i);
		if (!irq_name) {
			ret = -ENOMEM;
			goto out_napi_del;
		}

		ret = devm_request_irq(&pdev->dev, ret, xgmac_dma_irq_tx, 0,
				       irq_name, &priv->txq[i]);
		if (ret)
			goto out_napi_del;

		priv->txq[i].idx = i;
		spin_lock_init(&priv->txq[i].lock);
		netif_napi_add_tx_weight(&priv->napi_dev, &priv->txq[i].napi,
				  xgmac_dma_napi_tx, NAPI_POLL_WEIGHT);
		irq_set_affinity_hint(priv->txq[i].irq, cpumask_of(i % NR_CPUS));
	}

	/* RX IRQ */
#ifdef CONFIG_NET_SIFLOWER_ETH_RX_THREAD
	strscpy(priv->napi_dev.name, KBUILD_MODNAME, IFNAMSIZ);
	priv->napi_dev.threaded = 1;
#endif
	for (i = 0; i < DMA_CH_MAX; i++) {
		snprintf(buf, sizeof(buf), "rx%u", i);
		ret = platform_get_irq_byname(pdev, buf);
		if (ret < 0)
			goto out_napi_del;

		priv->rxq[i].irq = ret;

		irq_name = devm_kasprintf(&pdev->dev, GFP_KERNEL,
					  "xgmac_dma_rxq%u", i);
		if (!irq_name) {
			ret = -ENOMEM;
			goto out_napi_del;
		}

		ret = devm_request_irq(&pdev->dev, ret, xgmac_dma_irq_rx, 0,
				       irq_name, &priv->rxq[i]);
		if (ret)
			goto out_napi_del;

		priv->rxq[i].idx = i;
		netif_napi_add_weight(&priv->napi_dev, &priv->rxq[i].napi,
			       xgmac_dma_napi_rx, NAPI_POLL_WEIGHT);
		irq_set_affinity_hint(priv->rxq[i].irq, cpumask_of(i % NR_CPUS));
	}

	priv->rx_alloc_size = BUF_SIZE_ALLOC(ETH_DATA_LEN);
	priv->rx_buffer_size = BUF_SIZE_ALIGN(ETH_DATA_LEN);
	platform_set_drvdata(pdev, priv);
	ret = clk_bulk_prepare_enable(DMA_NUM_CLKS, priv->clks);
	if (ret)
		goto out_napi_del;

	ret = xgmac_dma_init(priv);
	if (ret)
		goto out_clk_disable;

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_PAGE_POOL_STATS)
	priv->dbgdir = debugfs_create_dir(KBUILD_MODNAME, NULL);
	if (IS_ERR(priv->dbgdir)) {
		ret = PTR_ERR(priv->dbgdir);
		goto out_clk_disable;
	}
	debugfs_create_file("rx_stats", 0444, priv->dbgdir, priv, &xgmac_dma_stats_fops);
	debugfs_create_file("debug", 0444, priv->dbgdir, priv, &xgmac_dma_debug_fops);
#endif

	return ret;
out_clk_disable:
	clk_bulk_disable_unprepare(DMA_NUM_CLKS, priv->clks);
out_napi_del:
	for (i = 0; i < DMA_CH_MAX; i++) {
		irq_set_affinity_hint(priv->rxq[i].irq, NULL);
		irq_set_affinity_hint(priv->txq[i].irq, NULL);
		netif_napi_del(&priv->rxq[i].napi);
		netif_napi_del(&priv->txq[i].napi);
	}
	return ret;
}

static void xgmac_dma_remove(struct platform_device *pdev)
{
	struct xgmac_dma_priv *priv = platform_get_drvdata(pdev);
	int i;

#if defined(CONFIG_DEBUG_FS) && defined(CONFIG_PAGE_POOL_STATS)
	debugfs_remove(priv->dbgdir);
#endif
	xgmac_dma_soft_reset(priv);
	clk_bulk_disable_unprepare(DMA_NUM_CLKS, priv->clks);
	for (i = 0; i < DMA_CH_MAX; i++) {
		irq_set_affinity_hint(priv->rxq[i].irq, NULL);
		irq_set_affinity_hint(priv->txq[i].irq, NULL);
		netif_napi_del(&priv->rxq[i].napi);
		netif_napi_del(&priv->txq[i].napi);
	}
}

static const struct of_device_id xgmac_dma_match[] = {
	{ .compatible = "siflower,sf21-xgmac-dma" },
	{},
};
MODULE_DEVICE_TABLE(of, xgmac_dma_match);

static struct platform_driver xgmac_dma_driver = {
	.probe	= xgmac_dma_probe,
	.remove	= xgmac_dma_remove,
	.driver	= {
		.name		= "sfxgmac_dma",
		.of_match_table	= xgmac_dma_match,
	},
};
module_platform_driver(xgmac_dma_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Qingfang Deng <qingfang.deng@siflower.com.cn>");
MODULE_DESCRIPTION("Ethernet DMA driver for SF21A6826/SF21H8898 SoC");
