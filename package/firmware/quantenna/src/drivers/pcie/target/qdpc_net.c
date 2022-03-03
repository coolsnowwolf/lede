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
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/inetdevice.h>
#include <linux/ip.h>
#include <net/sock.h>
#include <net/netlink.h>

#include <qtn/skb_recycle.h>
#include <qtn/qtn_global.h>

#include "qdpc_config.h"
#include "qdpc_debug.h"
#include "qdpc_init.h"
#include "qdpc_emac.h"

#ifdef CONFIG_QVSP
#include <trace/ippkt.h>
#include "qtn/qdrv_sch.h"
#include "qtn/qvsp.h"

static struct qvsp_wrapper qdpc_qvsp = {NULL, NULL};
#endif

#define MAX(X,Y) ((X) > (Y) ? X : Y)

/* Net device function prototypes */
int32_t qdpc_veth_open(struct net_device *ndev);
int32_t qdpc_veth_release(struct net_device *ndev);
struct net_device_stats *qdpc_veth_stats(struct net_device *ndev);
int32_t qdpc_veth_change_mtu(struct net_device *ndev, int new_mtu);
int32_t qdpc_veth_set_mac_addr(struct net_device *ndev, void *paddr);
int32_t qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev);

/* Alignment helper functions */
inline static unsigned long qdpc_align_val_up(unsigned long val, unsigned long step)
{
	return ((val + step - 1) & (~(step - 1)));
}
inline static unsigned long qdpc_align_val_down(unsigned long val, unsigned long step)
{
	return (val & (~(step - 1)));
}
inline static void* qdpc_align_buf_dma(void *addr)
{
	return (void*)qdpc_align_val_up((unsigned long)addr, dma_get_cache_alignment());
}
inline static unsigned long qdpc_align_buf_dma_offset(void *addr)
{
	return (qdpc_align_buf_dma(addr) - addr);
}
inline static void* qdpc_align_buf_cache(void *addr)
{
	return (void*)qdpc_align_val_down((unsigned long)addr, dma_get_cache_alignment());
}
inline static unsigned long qdpc_align_buf_cache_offset(void *addr)
{
	return (addr - qdpc_align_buf_cache(addr));
}
inline static unsigned long qdpc_align_buf_cache_size(void *addr, unsigned long size)
{
	return qdpc_align_val_up(size + qdpc_align_buf_cache_offset(addr),
			dma_get_cache_alignment());
}

/* Net device operations structure */
static struct net_device_ops veth_ops = {
	.ndo_open               = qdpc_veth_open,
	.ndo_stop               = qdpc_veth_release,
	.ndo_get_stats          = qdpc_veth_stats,
	.ndo_set_mac_address    = qdpc_veth_set_mac_addr,
	.ndo_change_mtu         = qdpc_veth_change_mtu,
	.ndo_start_xmit         = qdpc_veth_tx,
};

static inline bool check_netlink_magic(qdpc_cmd_hdr_t *cmd_hdr)
{
	return ((memcmp(cmd_hdr->dst_magic, QDPC_NETLINK_DST_MAGIC, ETH_ALEN) == 0)
		&& (memcmp(cmd_hdr->src_magic, QDPC_NETLINK_SRC_MAGIC, ETH_ALEN) == 0));
}

int32_t qdpc_veth_open(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);


	if (!qdpc_isbootstate(priv, QDPC_BDA_FW_RUNNING)) {
		netif_stop_queue(ndev); /* can't transmit any more */
		return SUCCESS;
	}

	flush_scheduled_work();
	netif_start_queue(ndev);

	return SUCCESS;
}

int32_t qdpc_veth_release(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);

	netif_stop_queue(ndev); /* can't transmit any more */
	qdpc_emac_disable(priv);

	return SUCCESS;
}

#ifdef CONFIG_QVSP
void qvsp_wrapper_init(struct qvsp_ext_s *qvsp, QVSP_CHECK_FUNC_PROTOTYPE(*check_func))
{
	qdpc_qvsp.qvsp = qvsp;
	qdpc_qvsp.qvsp_check_func = check_func;
}
EXPORT_SYMBOL(qvsp_wrapper_init);

void qvsp_wrapper_exit(void)
{
	qdpc_qvsp.qvsp_check_func = NULL;
	qdpc_qvsp.qvsp = NULL;
}
EXPORT_SYMBOL(qvsp_wrapper_exit);

static __sram_text int qdpc_rx_vsp_should_drop(struct sk_buff *skb, struct ethhdr *eh)
{
	u8 *data_start;
	u16 ether_type = 0;

	if (qvsp_is_active(qdpc_qvsp.qvsp) && qdpc_qvsp.qvsp_check_func) {
		data_start = qdrv_sch_find_data_start(skb, (struct ether_header *)eh, &ether_type);
		qdrv_sch_classify(skb, ether_type, data_start);
		if (qdpc_qvsp.qvsp_check_func(qdpc_qvsp.qvsp, QVSP_IF_PCIE_RX, skb,
				data_start, skb->len - (data_start - skb->data),
				skb->priority)) {
			trace_ippkt_dropped(TRACE_IPPKT_DROP_RSN_VSP, 1, 0);
			return 1;
		}
	}

	return 0;
}
#else
#define qdpc_rx_vsp_should_drop(_skb, _data_start)	(0)
#endif /* CONFIG_QVSP */

static inline SRAM_TEXT void qdpc_tx_skb_recycle(struct sk_buff *skb)
{
#if defined(QDPC_USE_SKB_RECYCLE)
	struct qtn_skb_recycle_list *recycle_list = qtn_get_shared_recycle_list();

	if (!qtn_skb_recycle_list_push(recycle_list,
			&recycle_list->stats_pcie, skb)) {
		dev_kfree_skb_any(skb);
	}
#else
	dev_kfree_skb_any(skb);
#endif
}

/* function runs in tasklet/softirq priority */
void qdpc_netlink_rx(struct net_device *ndev, void *buf, size_t len)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	struct sk_buff *skb = nlmsg_new(len, 0);
	struct nlmsghdr *nlh;
	int ret;

	if (skb == NULL) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN, "WARNING: out of netlink SKBs\n");
		return ;
	}

	nlh = nlmsg_put(skb, 0, 0, NLMSG_DONE, len, 0);  ;
	memcpy(nlmsg_data(nlh), buf, len);
	NETLINK_CB(skb).dst_group = 0;

	/* Send the netlink message to user application */
	ret = nlmsg_unicast(priv->netlink_socket, skb, priv->netlink_pid);
	if (ret < 0) {
		DBGPRINTF(DBG_LL_NOTICE, QDRV_LF_TRACE,
			"pid %d socket 0x%p ret %d\n",
			priv->netlink_pid, priv->netlink_socket, ret);
	}
}

static SRAM_TEXT size_t qdpc_rx_process_frame(struct net_device *ndev,
	qdpc_desc_t *rx_desc, uint32_t dma_status, bool lastdesc)
{
	qdpc_dmadesc_t *rx_hwdesc = rx_desc->dd_hwdesc;
	const uint32_t buffer_size = QDPC_DMA_MAXBUF;
	uint32_t dma_control = 0;
	uint32_t dma_data = 0;
	struct sk_buff *skb;
	struct ethhdr *eth;
	qdpc_cmd_hdr_t *cmd_hdr;
	size_t dmalen;

	skb = (struct sk_buff *)rx_desc->dd_metadata;
	dmalen = QDPC_DMA_RXLEN(dma_status);

	/* Check for runt packets */
	if (skb && (dmalen >= QDPC_DMA_MINBUF) && (dmalen <= QDPC_DMA_MAXBUF)
		&& (QDPC_DMA_SINGLE_BUFFER(dma_status))){
		dma_unmap_single((struct device *)ndev, rx_hwdesc->dma_data,
				buffer_size, DMA_FROM_DEVICE);
		eth = (struct ethhdr *)skb->data;

		switch (ntohs(eth->h_proto)) {
		case QDPC_APP_NETLINK_TYPE:
			/* Double Check if it's netlink packet*/
			cmd_hdr = (qdpc_cmd_hdr_t *)skb->data;
			if (check_netlink_magic(cmd_hdr)) {
				qdpc_netlink_rx(ndev, skb->data + sizeof(qdpc_cmd_hdr_t), ntohs(cmd_hdr->len));
			}
			break;
		default:
			skb->dev = ndev;
			skb->len = 0;
			skb_reset_tail_pointer(skb);
			skb_put(skb, dmalen);
			skb->ip_summed = CHECKSUM_UNNECESSARY;
			skb->protocol = eth_type_trans(skb, ndev);

			if (!is_multicast_ether_addr((uint8_t *)eth) &&
				qdpc_rx_vsp_should_drop(skb, eth)) {
				dev_kfree_skb_any(skb);
			} else {
				netif_rx(skb);
			}
			skb = NULL;
			break;
		}
	} else {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"RX: Drop packet. Skb: 0x%p Status:0x%x Len:%u\n", skb, dma_status, dmalen);
	}

	if (skb)
		dev_kfree_skb_any(skb);

	skb = qdpc_get_skb(buffer_size);
	if (skb == NULL) {
		dma_data = 0;
		dma_control = (lastdesc ? QDPC_DMA_LAST_DESC : 0);
	} else {
		/*
		 * skb->data has already cache line alignment when returned from qdpc_get_skb,
		 * no need to do cache line alignment again.
		 */
		dma_data = dma_map_single((struct device *)ndev, (void*)skb->data,
				buffer_size, DMA_FROM_DEVICE);
		dma_control = (buffer_size & QDPC_DMA_LEN_MASK) | (lastdesc ? QDPC_DMA_LAST_DESC : 0);
	}
	dma_status = (QDPC_DMA_OWN);
	rx_desc->dd_metadata = (void *)skb;

	arc_write_uncached_32(&rx_hwdesc->dma_control, dma_control);
	arc_write_uncached_32(&rx_hwdesc->dma_data, dma_data);
	arc_write_uncached_32(&rx_hwdesc->dma_status, dma_status);

	return dmalen;
}

static SRAM_TEXT void qdpc_indicate_peer_rx_nfree(struct qdpc_priv *priv,
	qdpc_pdring_t *rxq, qdpc_desc_t *rx_desc, uint32_t dma_status)
{
	uint32_t nfree = rxq->pd_ringsize;
	qdpc_desc_t *rx_last_desc;
	unsigned long flags;

	local_irq_save(flags);
	if (unlikely(QDPC_DMA_OWNED(dma_status) == 0)) {
		rx_last_desc = rx_desc;
		for (rx_desc = rxq->pd_nextdesc; (rx_desc != rx_last_desc) && (nfree > 0);) {
			dma_status = arc_read_uncached_32(&rx_desc->dd_hwdesc->dma_status);
			if (QDPC_DMA_OWNED(dma_status) == 0) {
				nfree--;
			} else {
				break;
			}

			if (rx_desc == rxq->pd_lastdesc) {
				rx_desc = rxq->pd_firstdesc;
			} else {
				rx_desc++;
			}
		}
		if (nfree <= QDPC_VETH_RX_LOW_WATERMARK)
			nfree = 0;
	}

	arc_write_uncached_32(priv->host_ep2h_txd_budget, nfree);
	local_irq_restore(flags);
}

SRAM_TEXT void qdpc_veth_rx(struct net_device  *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *rxq = &priv->pktq.pkt_dsq;
	uint32_t budget = QDPC_DESC_RING_SIZE << 1;
	uint32_t dma_status = 0;
	qdpc_desc_t *rx_desc;
	qdpc_dmadesc_t *rx_hwdesc;
	bool lastdesc;
	uint32_t pktlen;

	if (rxq->pd_nextdesc == NULL)
		return;

	rx_desc = rxq->pd_nextdesc;
	rx_hwdesc = rx_desc->dd_hwdesc;
	dma_status = arc_read_uncached_32(&rx_hwdesc->dma_status);

	while (budget-- > 0 && (QDPC_DMA_OWNED(dma_status) == 0)) {
		rx_desc = rxq->pd_nextdesc;
		lastdesc = (rxq->pd_nextdesc == rxq->pd_lastdesc);
		rx_hwdesc = rx_desc->dd_hwdesc;

		pktlen = qdpc_rx_process_frame(ndev, rx_desc, dma_status, lastdesc);

		/* Check for end of ring, and loop around */
		if (lastdesc) {
			rxq->pd_nextdesc = rxq->pd_firstdesc;
		} else {
			rxq->pd_nextdesc++;
		}

		/*  Update the statistics */
		priv->stats.rx_packets++;
		priv->stats.rx_bytes += pktlen;

		dma_status = arc_read_uncached_32(&rxq->pd_nextdesc->dd_hwdesc->dma_status);
	}

	qdpc_indicate_peer_rx_nfree(priv, rxq, rx_desc, dma_status);
	qdpc_intr_ep2host(priv, QDPC_EP_TXDONE);
}

SRAM_TEXT uint32_t qdpc_dma_state(qdpc_pdring_t *q)
{
	return readl(q->pd_rx_basereg + EMAC_DMASTATUS);

}
static inline SRAM_TEXT void qdpc_start_txdma(struct qdpc_priv *priv)
{
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;

	/* Must hold TXQ lock before running */
	//ASSERT(spin_is_locked(&txq->pd_lock));
	if (txq->pd_npending) {
		qdpc_emac_startdma(txq);
	}
}

static SRAM_TEXT uint32_t qdpc_veth_txprocessq(struct net_device *ndev, uint32_t maxpkts)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;
	qdpc_desc_t *tx_desc;
	uint32_t nprocessed = 0;
	uint32_t dma_data = 0;
	uint32_t dma_len = 0;
	struct sk_buff * skb;

	/* Must hold TXQ lock before running */
	//ASSERT(spin_is_locked(&txq->pd_lock));

	for(nprocessed = 0; nprocessed < maxpkts; nprocessed++) {
		tx_desc = STAILQ_FIRST(&txq->pd_pending);
		if (!tx_desc || !((arc_read_uncached_32(&tx_desc->dd_hwdesc->dma_status) & QDPC_DMA_OWN) == 0)) {
			break;
		}
		STAILQ_REMOVE_HEAD(&txq->pd_pending, dd_entry);

		skb = (struct sk_buff *)tx_desc->dd_metadata;
		dma_data = tx_desc->dd_hwdesc->dma_data;
		if (skb->len <= QDPC_DMA_MINBUF) {
			dma_len = QDPC_DMA_MINBUF;
		} else {
			dma_len = skb->len & QDPC_DMA_LEN_MASK;
		}

		dma_unmap_single((struct device *)ndev, qdpc_align_buf_cache(dma_data),
			qdpc_align_buf_cache_size(dma_data, dma_len), DMA_TO_DEVICE);
		qdpc_tx_skb_recycle(skb);
		tx_desc->dd_metadata = NULL;
		tx_desc->dd_qtime = 0;
	}

	txq->pd_nfree += nprocessed;
	txq->pd_npending -= nprocessed;
	return nprocessed;

}

inline static SRAM_TEXT void qdpc_send_rxdone(struct qdpc_priv *priv)
{
	qdpc_intr_ep2host(priv, QDPC_EP_RXDONE);
}


SRAM_DATA static int peer_rx_nfree = QDPC_DESC_RING_SIZE;

static SRAM_TEXT int32_t qdpc_update_peer_nfree(struct qdpc_priv *priv)
{
	int32_t budget;

	budget = arc_read_uncached_32(priv->host_h2ep_txd_budget);
	if (budget < 0) {
		budget = peer_rx_nfree;
	} else {
		peer_rx_nfree = budget;
		arc_write_uncached_32(priv->host_h2ep_txd_budget, -1);
	}

	return budget;
}

/* TX completion routine, Runs as tasklet/softirq priority */
SRAM_TEXT void qdpc_veth_txdone(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;
	int32_t npending = (int32_t)txq->pd_npending;

	spin_lock_bh(&txq->pd_lock);

	qdpc_veth_txprocessq(ndev, npending >> 1);
	if (netif_queue_stopped(ndev)) {
		del_timer(&priv->txq_enable_timer);
		qdpc_send_rxdone(priv);
		netif_wake_queue(ndev);
	}

	spin_unlock_bh(&txq->pd_lock);
}

static SRAM_TEXT int qdpc_send_desc_check(struct qdpc_priv *priv,
	qdpc_pdring_t *txq)
{
	int32_t budget;
	int32_t ret = 1;
	unsigned long flags;

	local_irq_save(flags);
	if (txq->pd_nfree > QDPC_VETH_TX_LOW_WATERMARK) {
		budget = qdpc_update_peer_nfree(priv);
		if (budget > (txq->pd_npending + QDPC_VETH_RX_LOW_WATERMARK)) {
			peer_rx_nfree--;
			ret = 0;
		}
	} else {
		qdpc_start_txdma(priv);
	}
	local_irq_restore(flags);

	return ret;
}

SRAM_TEXT int32_t qdpc_send_packet(struct sk_buff *skb, struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	qdpc_pdring_t *txq = &priv->pktq.pkt_usq;
	qdpc_dmadesc_t *tx_hwdesc;
	qdpc_desc_t *tx_desc;
	struct sk_buff *skb_cpy = NULL;
	bool lastdesc;
	uint32_t dma_control = 0 ;
	uint32_t dma_status = 0;
	uint32_t dma_data = 0;
	uint32_t dma_len = 0;

	if (unlikely((skb->len == 0) || (skb->len > QDPC_MAX_MTU))) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"Xmit packet invalid: len %d\n", skb->len);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	spin_lock_bh(&txq->pd_lock);

	if (qdpc_send_desc_check(priv, txq)) {
		netif_stop_queue(ndev);
		mod_timer(&priv->txq_enable_timer, jiffies + msecs_to_jiffies(QDPC_STOP_QUEUE_TIMER_DELAY));
		spin_unlock_bh(&txq->pd_lock);

		priv->stats.tx_errors++;
		return NETDEV_TX_BUSY;
	}

	/* Map data buffer. */
	if (skb->len <= QDPC_DMA_MINBUF) {
		dma_len = QDPC_DMA_MINBUF;
	} else {
		dma_len = skb->len & QDPC_DMA_LEN_MASK;
	}
	if (unlikely(dma_len >= QDPC_DMA_MAXBUF)) {
		spin_unlock_bh(&txq->pd_lock);
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"Xmit packet too big: len %d dmalen %d\n", skb->len, dma_len);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}


	if (unlikely(skb_linearize(skb) != 0)) {
		DBGPRINTF(DBG_LL_WARNING, QDRV_LF_WARN,
			"WARNING:%u Linearize failed\n", __LINE__);
		dev_kfree_skb_any(skb);
		priv->stats.tx_dropped++;
		return NETDEV_TX_OK;
	}

	tx_desc = txq->pd_nextdesc;
	tx_hwdesc = tx_desc->dd_hwdesc;
	lastdesc = (tx_desc == txq->pd_lastdesc);

	/* Hold onto skb. Release when we get a txdone */
	tx_desc->dd_metadata = (void *)skb;

	if (lastdesc) {
		txq->pd_nextdesc = txq->pd_firstdesc;
	} else {
		txq->pd_nextdesc++;
	}

	dma_data = dma_map_single((struct device *)ndev, qdpc_align_buf_cache(skb->data),
			qdpc_align_buf_cache_size(skb->data, dma_len), DMA_TO_DEVICE) +
			qdpc_align_buf_cache_offset(skb->data);
	dma_control = (QDPC_DMA_SINGLE_TXBUFFER | QDPC_DMA_TX_NOCRC
		|(lastdesc ? QDPC_DMA_LAST_DESC : 0) | (QDPC_DMA_LEN_MASK & dma_len));
	dma_status = QDPC_DMA_OWN;
	arc_write_uncached_32(&tx_hwdesc->dma_control, dma_control);
	arc_write_uncached_32(&tx_hwdesc->dma_data, dma_data);
	arc_write_uncached_32(&tx_hwdesc->dma_status, dma_status);

	tx_desc->dd_qtime = jiffies;
	STAILQ_INSERT_TAIL(&txq->pd_pending, tx_desc, dd_entry);
	txq->pd_nfree--;
	txq->pd_npending++;

	/* DMA engine setup for auto poll so no doorbell push needed */

	spin_unlock_bh(&txq->pd_lock);

	priv->stats.tx_packets++;
	priv->stats.tx_bytes += skb->len;

	return NETDEV_TX_OK;
}

SRAM_TEXT int32_t qdpc_veth_tx(struct sk_buff *skb, struct net_device *ndev)
{
	return qdpc_send_packet(skb, ndev);
}

struct net_device_stats *qdpc_veth_stats(struct net_device *ndev)
{
	struct qdpc_priv *priv = netdev_priv(ndev);

	return &priv->stats;
}

int32_t qdpc_veth_change_mtu(struct net_device *ndev,
			int    new_mtu)
{
	struct qdpc_priv *priv = netdev_priv(ndev);
	spinlock_t      *lock = &priv->lock;
	unsigned long   flags;

	/* check ranges */
	if ((new_mtu < QDPC_MIN_MTU) || (new_mtu > QDPC_MAX_MTU))
		return -EINVAL;
	/*
	 * Do anything you need, and the accept the value
	 */
	spin_lock_irqsave(lock, flags);
	ndev->mtu = new_mtu;
	spin_unlock_irqrestore(lock, flags);

	return SUCCESS; /* success */
}

int32_t qdpc_veth_set_mac_addr(struct net_device *ndev,
			void              *paddr)
{
	struct sockaddr *addr = (struct sockaddr *)paddr;

	if (netif_running(ndev))
		return -EBUSY;

	if (!is_valid_ether_addr((u8 *)addr->sa_data))
		return -EINVAL;

	memcpy(ndev->dev_addr, addr->sa_data, ndev->addr_len);

	return SUCCESS;
}

int32_t qdpc_init_netdev(struct net_device **net_dev)
{
	struct qdpc_priv *priv;
	struct net_device *ndev;
	unsigned char macaddr[ETH_ALEN];
	int ret = 0;

	/* Net device initialization */
	/* Allocate the devices */
	ndev = alloc_netdev(sizeof(struct qdpc_priv), "pcie%d",
			ether_setup);
	if (!ndev) {
		PRINT_ERROR("Error in allocating the net device \n");
		return -ENOMEM;
	}

	/* No interesting netdevice features in this card... */
	priv = netdev_priv(ndev);
	priv->ndev = ndev;

	memcpy(macaddr, qdpc_basemac, ETH_ALEN);
	macaddr[0] = (macaddr[0] & 0x1F) | (((macaddr[0] & 0xE0) + 0x20) & 0xE0) | 0x02;
	memcpy(ndev->dev_addr, macaddr, ETH_ALEN);

	*net_dev = ndev; /* Storing ndev in global dev */

	/* Assign net device operations structure */

	ndev->netdev_ops = &veth_ops;

	/* Register net device */
	ret = register_netdev(ndev);
	if (ret) {
		PRINT_ERROR("veth: error %i registering net device \"%s\"\n",
				ret, ndev->name);
	}

	if (ret) {
		free_netdev(ndev);
	}

	return ret;
}
