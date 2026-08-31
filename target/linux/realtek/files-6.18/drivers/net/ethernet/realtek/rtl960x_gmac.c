// SPDX-License-Identifier: GPL-2.0-only
/* Realtek RTL960X ethernet driver */

#include <linux/bitfield.h>
#include <linux/cleanup.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/reset.h>
#include <linux/timer.h>
#include <net/dsa.h>
#include <net/dst_metadata.h>

#include "rtl960x_gmac.h"

#define RTL9607C_CPUTAG_CONFIG							\
	(FIELD_PREP(CPUTAGCR_TSIZE, CPUTAGCR_SIZE_8B) |			\
	 FIELD_PREP(CPUTAGCR_SWITCH, CPUTAGCR_FMT_APOLLOPRO) |		\
	 FIELD_PREP(CPUTAGCR_RSIZE_L, CPUTAGCR_SIZE_8B) |		\
	 FIELD_PREP(CPUTAGCR_PROTOCOL_MASK, CPUTAGCR_PROTO_MASK_8370) | \
	 FIELD_PREP(CPUTAGCR_PROTOCOL_VAL, CPUTAGCR_PROTO_8370))

#define RTL9607C_IO_CMD_CONFIG							\
	(IO_CMD_MAX_DMA_128B |						\
	 IO_CMD_EN_EARLY_TX |						\
	 FIELD_PREP(IO_CMD_RX_INT_TRIG_L, IO_CMD_RX_INT_4PKTS) |	\
	 FIELD_PREP(IO_CMD_RX_FIFO_THRESH, IO_CMD_RX_FIFO_64B) |	\
	 FIELD_PREP(IO_CMD_RX_PKT_TMR_L, IO_CMD_RX_PKT_TMR_4TU) |	\
	 FIELD_PREP(IO_CMD_TX_INT_TRIG_L, IO_CMD_TX_INT_28PKTS) |	\
	 FIELD_PREP(IO_CMD_TX_FIFO_THRESH, IO_CMD_TX_FIFO_THRESH_1024B) | \
	 FIELD_PREP(IO_CMD_TX_PKT_TMR, IO_CMD_TX_PKT_TMR_7TU) |		\
	 FIELD_PREP(IO_CMD_INI_TMR_SEL, IO_CMD_INI_TMR_SEL_3) |		\
	 IO_CMD_SHORT_DES_FMT)

#define RTL9607C_IO_CMD1_CONFIG							\
	(FIELD_PREP(IO_CMD1_DESC_FMT_EXTRA, IO_CMD1_DESC_FMT_APOLLO) | \
	 IO_CMD1_RX_RING1)

/*
 * DSA glue. Convert the rtl_otto TX trailer to descriptor port steering, and
 * pass the RX descriptor's source port through METADATA_HW_PORT_MUX. The DSA
 * core derives skb->offload_fwd_mark from bridge membership, so this path
 * cannot preserve the descriptor's per-frame trap reason.
 */
#define DSA_TRAILER_LEN		4

/* Datapath sizing */
#define RX_RING_SIZE		128	/* must be a power of two, 16..256 */
#define TX_RING_SIZE		128
#define RX_BUF_SIZE		1536	/* 0x600, per U-Boot RX_DESC_BUFFER_SIZE */
#define RX_SHIFT		2	/* HW writes the frame at buf + 2 bytes */
#define RX_MAX_FRAME_LEN	(RX_BUF_SIZE - RX_SHIFT)
#define TX_MIN_LEN		ETH_ZLEN
/* SW failure backstop if a TX completion interrupt is missed. */
#define TX_TIMER_DELAY		msecs_to_jiffies(10)

#define RTL960X_GMAC_REGS_DUMP_LEN	0x100

/* The ring strides are the HW descriptor sizes; guard the layout. */
static_assert(sizeof(struct rtl960x_rx_desc) == 16);
static_assert(sizeof(struct rtl960x_tx_desc) == 20);

struct rtl960x_gmac {
	struct net_device *dev;
	struct device *dma_dev;
	struct regmap *map8;
	struct regmap *map16;
	struct regmap *map32;
	int irq;
	struct napi_struct napi;
	struct reset_control *gmac_rst;

	/* RX ring */
	struct rtl960x_rx_desc *rx_ring;	/* RX_RING_SIZE entries */
	dma_addr_t rx_ring_dma;
	struct sk_buff *rx_skb[RX_RING_SIZE];
	dma_addr_t rx_buf_dma[RX_RING_SIZE];
	u32 rx_head;

	/* TX ring */
	struct rtl960x_tx_desc *tx_ring;	/* TX_RING_SIZE entries */
	dma_addr_t tx_ring_dma;
	struct sk_buff *tx_skb[TX_RING_SIZE];
	u32 tx_head;			/* next slot to fill */
	u32 tx_tail;			/* next slot to reclaim */
	spinlock_t tx_lock;		/* protects tx_head/tx_tail and the ring */
	struct timer_list tx_timer;	/* TX completion failure backstop */
	struct metadata_dst *dsa_meta[RTL960X_CPU_PORT];
};

/*
 * Standard MMIO accessors do not swap values on this target, so keep the
 * regmap-MMIO default to preserve the previous raw access semantics.
 */
static const struct regmap_config rtl960x_gmac_regmap8_config = {
	.name = "8-bit",
	.reg_bits = 32,
	.val_bits = 8,
	.reg_stride = 1,
};

static const struct regmap_config rtl960x_gmac_regmap16_config = {
	.name = "16-bit",
	.reg_bits = 32,
	.val_bits = 16,
	.reg_stride = 2,
};

static const struct regmap_config rtl960x_gmac_regmap32_config = {
	.name = "32-bit",
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
};

static void rtl960x_gmac_stop_hw(struct rtl960x_gmac *priv)
{
	regmap_write(priv->map32, GMAC_IO_CMD, 0);
	regmap_write(priv->map32, GMAC_IO_CMD1, 0);
	regmap_write(priv->map16, GMAC_IMR, 0);
	regmap_write(priv->map32, GMAC_IMR0, 0);
	regmap_write(priv->map16, GMAC_ISR, 0xffff);
	regmap_write(priv->map32, GMAC_ISR1, 0xffffffff);
	usleep_range(10, 20);
}

static void rtl960x_gmac_set_hwaddr(struct rtl960x_gmac *priv)
{
	const u8 *a = priv->dev->dev_addr;

	regmap_write(priv->map32, GMAC_IDR0,
		     (a[0] << 24) | (a[1] << 16) | (a[2] << 8) | a[3]);
	regmap_write(priv->map32, GMAC_IDR4, (a[4] << 24) | (a[5] << 16));
}

static void rtl960x_gmac_set_rx_mode(struct net_device *dev)
{
	u32 rcr = RCR_ACCEPT_BROADCAST | RCR_ACCEPT_MYPHYS;
	struct rtl960x_gmac *priv = netdev_priv(dev);
	u32 mar = 0;

	/*
	 * The driver does not advertise IFF_UNICAST_FLT, so DSA keeps an
	 * attached conduit promiscuous while user ports may use other MACs.
	 */
	if (dev->flags & IFF_PROMISC)
		rcr |= RCR_ACCEPT_ALLPHYS;

	if ((dev->flags & (IFF_PROMISC | IFF_ALLMULTI)) ||
	    !netdev_mc_empty(dev)) {
		rcr |= RCR_ACCEPT_MULTICAST;
		mar = ~0U;
	}

	regmap_write(priv->map32, GMAC_MAR0, mar);
	regmap_write(priv->map32, GMAC_MAR4, mar);
	regmap_write(priv->map32, GMAC_RCR, rcr);
}

static void rtl960x_gmac_free_rx(struct rtl960x_gmac *priv)
{
	int i;

	for (i = 0; i < RX_RING_SIZE; i++) {
		if (!priv->rx_skb[i])
			continue;
		dma_unmap_single(priv->dma_dev, priv->rx_buf_dma[i], RX_BUF_SIZE,
				 DMA_FROM_DEVICE);
		dev_kfree_skb(priv->rx_skb[i]);
		priv->rx_skb[i] = NULL;
	}
}

static int rtl960x_gmac_init_rings(struct rtl960x_gmac *priv)
{
	int i;

	/* Populate RX ring with buffers; HW owns each filled descriptor. */
	for (i = 0; i < RX_RING_SIZE; i++) {
		struct rtl960x_rx_desc *d = &priv->rx_ring[i];
		struct sk_buff *skb;
		dma_addr_t dma;

		skb = netdev_alloc_skb(priv->dev, RX_BUF_SIZE);
		if (!skb)
			goto err_free_rx_bufs;

		dma = dma_map_single(priv->dma_dev, skb->data, RX_BUF_SIZE,
				     DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dma_dev, dma)) {
			dev_kfree_skb(skb);
			goto err_free_rx_bufs;
		}

		priv->rx_skb[i] = skb;
		priv->rx_buf_dma[i] = dma;
		d->addr = dma;
		d->opts2 = 0;
		d->opts3 = 0;
		d->opts1 = DESC_OWN | FIELD_PREP(DESC_LEN_MASK, RX_BUF_SIZE);
	}
	priv->rx_ring[RX_RING_SIZE - 1].opts1 |= DESC_EOR;

	/* TX ring starts idle (CPU-owned); only EOR is preset on the last. */
	for (i = 0; i < TX_RING_SIZE; i++) {
		struct rtl960x_tx_desc *d = &priv->tx_ring[i];

		d->addr = 0;
		d->opts2 = 0;
		d->opts3 = 0;
		d->opts4 = 0;
		d->opts1 = 0;
	}
	priv->tx_ring[TX_RING_SIZE - 1].opts1 |= DESC_EOR;

	priv->rx_head = 0;
	priv->tx_head = 0;
	priv->tx_tail = 0;
	dma_wmb();

	return 0;

err_free_rx_bufs:
	rtl960x_gmac_free_rx(priv);
	return -ENOMEM;
}

static void rtl960x_gmac_init_hw(struct rtl960x_gmac *priv)
{
	rtl960x_gmac_stop_hw(priv);

	/*
	 * Keep RXJUMBO set like the vendor driver always does; the accept
	 * cutoff with the bit cleared is unvalidated on this hardware, and
	 * oversize frames are dropped by the FS|LS + length check in the
	 * RX path anyway.
	 */
	regmap_write(priv->map8, GMAC_CMD, CMD_RXCHKSUM | CMD_RXJUMBO);
	regmap_write(priv->map32, GMAC_TCR, TCR_CONFIG);
	regmap_update_bits(priv->map32, GMAC_CFG, CFG_RFIFO_SIZE,
			   FIELD_PREP(CFG_RFIFO_SIZE, CFG_RFIFO_SIZE_2KB));

	/* CPU tag: parsed into the descriptor opts fields, not inline. */
	regmap_write(priv->map32, GMAC_CPUTAGCR,
		     RTL9607C_CPUTAG_CONFIG | CPUTAGCR_EN_RX);
	regmap_write(priv->map32, GMAC_CPUTAG1CR, CPUTAG1CR_SID);

	/* Program the RX/TX ring bases. */
	regmap_write(priv->map32, GMAC_RXFDP, priv->rx_ring_dma);
	regmap_write(priv->map16, GMAC_RXCDO, 0);
	regmap_write(priv->map8, GMAC_RXRINGSIZE, RX_RING_SIZE - 1);
	regmap_write(priv->map8, GMAC_RXCPU_DES_NUM, RX_RING_SIZE - 1);
	regmap_write(priv->map8, GMAC_RX_FC_ASSERT_THRES,
		     RX_FC_ASSERT_THRES_VAL);
	regmap_write(priv->map8, GMAC_RX_FC_DEASSERT_THRES,
		     RX_FC_DEASSERT_THRES_VAL);

	regmap_write(priv->map32, GMAC_TXFDP1, priv->tx_ring_dma);
	regmap_write(priv->map16, GMAC_TXCDO1, 0);

	rtl960x_gmac_set_hwaddr(priv);
	rtl960x_gmac_set_rx_mode(priv->dev);

	/* Enable the datapath. */
	regmap_write(priv->map32, GMAC_IO_CMD1, RTL9607C_IO_CMD1_CONFIG);
	regmap_write(priv->map32, GMAC_IO_CMD, RTL9607C_IO_CMD_CONFIG |
		     IO_CMD_RX_ENABLE | IO_CMD_TX_ENABLE);

	/* Clear and unmask RX + TX-completion interrupts. */
	regmap_write(priv->map16, GMAC_ISR, 0xffff);
	regmap_write(priv->map32, GMAC_ISR1, 0xffffffff);
	regmap_write(priv->map16, GMAC_IMR, INTR_NAPI);
}

static void rtl960x_gmac_mask_irqs(struct rtl960x_gmac *priv)
{
	regmap_clear_bits(priv->map16, GMAC_IMR, INTR_NAPI);
}

static void rtl960x_gmac_unmask_irqs(struct rtl960x_gmac *priv)
{
	regmap_set_bits(priv->map16, GMAC_IMR, INTR_NAPI);
}

static int rtl960x_gmac_rx(struct rtl960x_gmac *priv, int budget)
{
	struct net_device *dev = priv->dev;
	bool dsa = netdev_uses_dsa(dev);
	int done = 0;

	while (done < budget) {
		struct rtl960x_rx_desc *d = &priv->rx_ring[priv->rx_head];
		bool last = (priv->rx_head == RX_RING_SIZE - 1);
		struct sk_buff *skb, *new_skb;
		u32 opts1, opts3;
		dma_addr_t new_dma;
		u8 src_port;
		int len;

		opts1 = d->opts1;
		if (opts1 & DESC_OWN)		/* still owned by HW */
			break;
		dma_rmb();

		opts3 = d->opts3;
		src_port = FIELD_GET(RX_OPTS3_SRC_PORT, opts3);

		/*
		 * This single-buffer datapath only handles whole frames: each
		 * descriptor must be both the first and last segment and carry a
		 * sane length. The bound applies to the raw descriptor length --
		 * that is what the hardware wrote at buf + RX_SHIFT -- before any
		 * FCS adjustment. Reject anything else and rearm the same buffer.
		 * Note no allocation happens on this error path -- a fresh skb is
		 * taken only once the frame is known good, so runts, oversize and
		 * multi-segment descriptors reuse the existing buffer as-is.
		 */
		len = FIELD_GET(DESC_LEN_MASK, opts1);
		if ((opts1 & (DESC_FS | DESC_LS)) != (DESC_FS | DESC_LS) ||
		    len < ETH_ZLEN + ETH_FCS_LEN || len > RX_MAX_FRAME_LEN) {
			dev->stats.rx_errors++;
			goto rearm_same;
		}

		len -= ETH_FCS_LEN;

		new_skb = netdev_alloc_skb(dev, RX_BUF_SIZE);
		if (!new_skb) {
			dev->stats.rx_dropped++;
			goto rearm_same;
		}
		new_dma = dma_map_single(priv->dma_dev, new_skb->data, RX_BUF_SIZE,
					 DMA_FROM_DEVICE);
		if (dma_mapping_error(priv->dma_dev, new_dma)) {
			dev_kfree_skb(new_skb);
			dev->stats.rx_dropped++;
			goto rearm_same;
		}

		/* Good frame: swap the filled buffer out for the fresh one. */
		skb = priv->rx_skb[priv->rx_head];
		dma_unmap_single(priv->dma_dev, priv->rx_buf_dma[priv->rx_head],
				 RX_BUF_SIZE, DMA_FROM_DEVICE);

		skb_reserve(skb, RX_SHIFT);	/* HW wrote the frame at buf + 2 */
		skb_put(skb, len);
		if (dsa && src_port < RTL960X_CPU_PORT)
			skb_dst_set_noref(skb,
					  &priv->dsa_meta[src_port]->dst);
		skb->protocol = eth_type_trans(skb, dev);
		dev->stats.rx_packets++;
		dev->stats.rx_bytes += len;
		napi_gro_receive(&priv->napi, skb);

		priv->rx_skb[priv->rx_head] = new_skb;
		priv->rx_buf_dma[priv->rx_head] = new_dma;
		d->addr = new_dma;

rearm_same:
		d->opts2 = 0;
		d->opts3 = 0;
		dma_wmb();
		d->opts1 = DESC_OWN | FIELD_PREP(DESC_LEN_MASK, RX_BUF_SIZE) |
			   (last ? DESC_EOR : 0);

		priv->rx_head = (priv->rx_head + 1) % RX_RING_SIZE;
		done++;
	}

	return done;
}

/* Reclaim TX descriptors the HW has finished with. Caller holds tx_lock. */
static void rtl960x_gmac_tx_reclaim(struct rtl960x_gmac *priv)
{
	while (priv->tx_tail != priv->tx_head) {
		struct rtl960x_tx_desc *d = &priv->tx_ring[priv->tx_tail];
		struct sk_buff *skb;

		if (d->opts1 & DESC_OWN)
			break;

		skb = priv->tx_skb[priv->tx_tail];
		if (skb) {
			dma_unmap_single(priv->dma_dev, d->addr, skb->len,
					 DMA_TO_DEVICE);
			priv->dev->stats.tx_packets++;
			priv->dev->stats.tx_bytes += skb->len;
			dev_consume_skb_any(skb);
			priv->tx_skb[priv->tx_tail] = NULL;
		}
		priv->tx_tail = (priv->tx_tail + 1) % TX_RING_SIZE;
	}
}

static int rtl960x_gmac_poll(struct napi_struct *napi, int budget)
{
	struct rtl960x_gmac *priv = container_of(napi, struct rtl960x_gmac, napi);
	int done;

	/* Reclaim finished TX descriptors and wake the queue if it had filled. */
	spin_lock(&priv->tx_lock);
	rtl960x_gmac_tx_reclaim(priv);
	if (priv->tx_tail != priv->tx_head)
		mod_timer(&priv->tx_timer, jiffies + TX_TIMER_DELAY);
	else
		timer_delete(&priv->tx_timer);
	if (netif_queue_stopped(priv->dev) &&
	    (priv->tx_head + 1) % TX_RING_SIZE != priv->tx_tail)
		netif_wake_queue(priv->dev);
	spin_unlock(&priv->tx_lock);

	done = rtl960x_gmac_rx(priv, budget);

	if (done < budget && napi_complete_done(napi, done))
		rtl960x_gmac_unmask_irqs(priv);

	return done;
}

static irqreturn_t rtl960x_gmac_isr(int irq, void *dev_id)
{
	struct rtl960x_gmac *priv = dev_id;
	unsigned int status = 0;

	regmap_read(priv->map16, GMAC_ISR, &status);
	status &= INTR_NAPI;

	if (!status)
		return IRQ_NONE;

	regmap_write(priv->map16, GMAC_ISR, status);

	rtl960x_gmac_mask_irqs(priv);
	napi_schedule_irqoff(&priv->napi);

	return IRQ_HANDLED;
}

/*
 * Failure backstop for TX completions not reclaimed through the hardware TOK
 * path. Scheduling the combined NAPI is idempotent.
 */
static void rtl960x_gmac_tx_timer(struct timer_list *t)
{
	struct rtl960x_gmac *priv = timer_container_of(priv, t, tx_timer);

	napi_schedule(&priv->napi);
}

static netdev_tx_t rtl960x_gmac_start_xmit(struct sk_buff *skb,
					   struct net_device *dev)
{
	struct rtl960x_gmac *priv = netdev_priv(dev);
	struct rtl960x_tx_desc *d;
	bool tx_empty, last;
	int dest_port = -1;
	u32 opts1, next;
	dma_addr_t dma;
	int len;

	scoped_guard(spinlock, &priv->tx_lock) {
		rtl960x_gmac_tx_reclaim(priv);

		next = (priv->tx_head + 1) % TX_RING_SIZE;
		if (next == priv->tx_tail) {
			/*
			 * Ring full: stop the queue and ask the stack to requeue. The
			 * skb must stay pristine here -- it is retried as-is, so the DSA
			 * trailer (pulled off below) must not be removed before we know
			 * the frame is actually going on the ring.
			 */
			netif_stop_queue(dev);
			return NETDEV_TX_BUSY;
		}

		/* Convert the rtl_otto trailer to descriptor port steering. */
		if (netdev_uses_dsa(dev) && skb->len >= DSA_TRAILER_LEN) {
			const u8 *t = skb->data + skb->len - DSA_TRAILER_LEN;

			if (t[0] < RTL960X_CPU_PORT && t[1] == 0xab &&
			    t[2] == 0xcd && t[3] == 0xef) {
				dest_port = t[0];
				skb_trim(skb, skb->len - DSA_TRAILER_LEN);
			}
		}

		if (skb_put_padto(skb, TX_MIN_LEN))
			return NETDEV_TX_OK;	/* skb freed by skb_put_padto */
		len = skb->len;

		dma = dma_map_single(priv->dma_dev, skb->data, len, DMA_TO_DEVICE);
		if (dma_mapping_error(priv->dma_dev, dma)) {
			dev->stats.tx_dropped++;
			dev_kfree_skb_any(skb);
			return NETDEV_TX_OK;
		}

		tx_empty = priv->tx_tail == priv->tx_head;

		last = (priv->tx_head == TX_RING_SIZE - 1);
		d = &priv->tx_ring[priv->tx_head];
		priv->tx_skb[priv->tx_head] = skb;
		d->addr = dma;
		if (dest_port >= 0) {
			d->opts2 = TX_OPTS2_CPUTAG |
				   FIELD_PREP(TX_OPTS2_PORTMASK, BIT(dest_port));
			d->opts3 = TX_OPTS3_KEEP | TX_OPTS3_DISLRN | TX_OPTS3_L34_KEEP;
		} else {
			d->opts2 = 0;
			d->opts3 = 0;
		}
		d->opts4 = 0;
		opts1 = DESC_OWN | DESC_FS | DESC_LS | TX_DESC_CRC |
			FIELD_PREP(DESC_LEN_MASK, len) | (last ? DESC_EOR : 0);
		dma_wmb();
		d->opts1 = opts1;
		dma_wmb();

		priv->tx_head = next;
		if (tx_empty)
			mod_timer(&priv->tx_timer, jiffies + TX_TIMER_DELAY);

		/* No room for one more frame: stop now, NAPI wakes us on TX reclaim. */
		if ((next + 1) % TX_RING_SIZE == priv->tx_tail)
			netif_stop_queue(dev);

		/* TX_POLL is a command bit; write it even if it already reads as set. */
		regmap_write_bits(priv->map32, GMAC_IO_CMD, IO_CMD_TX_POLL,
				  IO_CMD_TX_POLL);
	}

	return NETDEV_TX_OK;
}

static void rtl960x_gmac_free_tx(struct rtl960x_gmac *priv)
{
	int i;

	for (i = 0; i < TX_RING_SIZE; i++) {
		struct rtl960x_tx_desc *d = &priv->tx_ring[i];

		if (!priv->tx_skb[i])
			continue;
		dma_unmap_single(priv->dma_dev, d->addr, priv->tx_skb[i]->len,
				 DMA_TO_DEVICE);
		dev_kfree_skb(priv->tx_skb[i]);
		priv->tx_skb[i] = NULL;
	}
}

static int rtl960x_gmac_open(struct net_device *dev)
{
	struct rtl960x_gmac *priv = netdev_priv(dev);
	int ret;

	ret = rtl960x_gmac_init_rings(priv);
	if (ret)
		return ret;

	ret = request_irq(priv->irq, rtl960x_gmac_isr, 0, dev->name, priv);
	if (ret) {
		netdev_err(dev, "failed to request irq %d: %d\n", priv->irq, ret);
		goto err_free_rx;
	}

	napi_enable(&priv->napi);
	rtl960x_gmac_init_hw(priv);

	netif_start_queue(dev);
	/*
	 * The conduit is wired to the switch CPU port over a fixed link, so it
	 * has no link state of its own -- the user ports carry it. Force the
	 * carrier on so the stack will pass traffic.
	 */
	netif_carrier_on(dev);

	netdev_info(dev, "datapath up (RX ring %d, TX ring %d)\n",
		    RX_RING_SIZE, TX_RING_SIZE);

	return 0;

err_free_rx:
	rtl960x_gmac_free_rx(priv);
	return ret;
}

static int rtl960x_gmac_stop(struct net_device *dev)
{
	struct rtl960x_gmac *priv = netdev_priv(dev);

	netif_stop_queue(dev);
	netif_carrier_off(dev);

	rtl960x_gmac_stop_hw(priv);

	napi_disable(&priv->napi);
	timer_delete_sync(&priv->tx_timer);
	free_irq(priv->irq, priv);

	rtl960x_gmac_free_tx(priv);
	rtl960x_gmac_free_rx(priv);

	return 0;
}

static int rtl960x_gmac_set_mac_address(struct net_device *dev, void *p)
{
	struct rtl960x_gmac *priv = netdev_priv(dev);
	int ret;

	ret = eth_mac_addr(dev, p);
	if (ret)
		return ret;

	if (netif_running(dev))
		rtl960x_gmac_set_hwaddr(priv);

	return 0;
}

static const struct net_device_ops rtl960x_gmac_netdev_ops = {
	.ndo_open		= rtl960x_gmac_open,
	.ndo_stop		= rtl960x_gmac_stop,
	.ndo_start_xmit		= rtl960x_gmac_start_xmit,
	.ndo_validate_addr	= eth_validate_addr,
	.ndo_set_mac_address	= rtl960x_gmac_set_mac_address,
	.ndo_set_rx_mode	= rtl960x_gmac_set_rx_mode,
};

static int rtl960x_gmac_get_regs_len(struct net_device *dev)
{
	return RTL960X_GMAC_REGS_DUMP_LEN;
}

static void rtl960x_gmac_get_regs(struct net_device *dev,
				  struct ethtool_regs *regs, void *p)
{
	struct rtl960x_gmac *priv = netdev_priv(dev);
	u32 *buf = p;
	int i;

	regs->version = 1;
	memset(p, 0, RTL960X_GMAC_REGS_DUMP_LEN);

	for (i = 0; i < RTL960X_GMAC_REGS_DUMP_LEN / sizeof(u32); i++)
		regmap_read(priv->map32, i * sizeof(u32), &buf[i]);
}

static const struct ethtool_ops rtl960x_gmac_ethtool_ops = {
	.get_regs_len	= rtl960x_gmac_get_regs_len,
	.get_regs	= rtl960x_gmac_get_regs,
};

static void rtl960x_gmac_assert_reset(void *gmac_rst)
{
	reset_control_assert(gmac_rst);
}

static void rtl960x_gmac_free_metadata_dst(void *gmac)
{
	struct rtl960x_gmac *priv = gmac;

	for (int i = 0; i < ARRAY_SIZE(priv->dsa_meta); i++) {
		if (!priv->dsa_meta[i])
			continue;

		metadata_dst_free(priv->dsa_meta[i]);
		priv->dsa_meta[i] = NULL;
	}
}

static int rtl960x_gmac_alloc_metadata_dst(struct rtl960x_gmac *priv)
{
	for (int i = 0; i < ARRAY_SIZE(priv->dsa_meta); i++) {
		struct metadata_dst *md_dst;

		md_dst = metadata_dst_alloc(0, METADATA_HW_PORT_MUX,
					    GFP_KERNEL);
		if (!md_dst)
			return -ENOMEM;

		md_dst->u.port_info.port_id = i;
		priv->dsa_meta[i] = md_dst;
	}

	return 0;
}

static int rtl960x_gmac_probe(struct platform_device *pdev)
{
	struct rtl960x_gmac *priv;
	struct net_device *dev;
	struct resource *res;
	void __iomem *base;
	int irq;
	int ret;

	dev = devm_alloc_etherdev(&pdev->dev, sizeof(*priv));
	if (!dev)
		return -ENOMEM;

	SET_NETDEV_DEV(dev, &pdev->dev);
	dev->netdev_ops = &rtl960x_gmac_netdev_ops;
	dev->ethtool_ops = &rtl960x_gmac_ethtool_ops;
	dev->min_mtu = ETH_MIN_MTU;
	dev->max_mtu = RX_MAX_FRAME_LEN - ETH_HLEN - ETH_FCS_LEN;

	ret = platform_get_ethdev_address(&pdev->dev, dev);
	if (ret == -EPROBE_DEFER)
		return ret;
	if (ret)
		eth_hw_addr_random(dev);

	netif_carrier_off(dev);

	priv = netdev_priv(dev);
	priv->dev = dev;
	priv->dma_dev = &pdev->dev;
	spin_lock_init(&priv->tx_lock);
	timer_setup(&priv->tx_timer, rtl960x_gmac_tx_timer, 0);

	ret = rtl960x_gmac_alloc_metadata_dst(priv);
	if (ret) {
		rtl960x_gmac_free_metadata_dst(priv);
		return ret;
	}
	ret = devm_add_action_or_reset(&pdev->dev,
				       rtl960x_gmac_free_metadata_dst, priv);
	if (ret)
		return ret;

	base = devm_platform_get_and_ioremap_resource(pdev, 0, &res);
	if (IS_ERR(base))
		return PTR_ERR(base);
	priv->map8 = devm_regmap_init_mmio(&pdev->dev, base,
					   &rtl960x_gmac_regmap8_config);
	if (IS_ERR(priv->map8))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->map8),
				     "failed to initialize 8-bit regmap\n");
	priv->map16 = devm_regmap_init_mmio(&pdev->dev, base,
					    &rtl960x_gmac_regmap16_config);
	if (IS_ERR(priv->map16))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->map16),
				     "failed to initialize 16-bit regmap\n");
	priv->map32 = devm_regmap_init_mmio(&pdev->dev, base,
					    &rtl960x_gmac_regmap32_config);
	if (IS_ERR(priv->map32))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->map32),
				     "failed to initialize 32-bit regmap\n");

	ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(32));
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to set DMA mask\n");
	priv->rx_ring = dmam_alloc_coherent(&pdev->dev,
					    RX_RING_SIZE * sizeof(*priv->rx_ring),
					    &priv->rx_ring_dma, GFP_KERNEL);
	if (!priv->rx_ring)
		return -ENOMEM;
	priv->tx_ring = dmam_alloc_coherent(&pdev->dev,
					    TX_RING_SIZE * sizeof(*priv->tx_ring),
					    &priv->tx_ring_dma, GFP_KERNEL);
	if (!priv->tx_ring)
		return -ENOMEM;

	priv->gmac_rst = devm_reset_control_get_optional_exclusive(&pdev->dev,
								   NULL);
	if (IS_ERR(priv->gmac_rst))
		return dev_err_probe(&pdev->dev, PTR_ERR(priv->gmac_rst),
				     "failed to get gmac reset\n");
	ret = reset_control_deassert(priv->gmac_rst);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to enable GMAC IP\n");
	ret = devm_add_action_or_reset(&pdev->dev, rtl960x_gmac_assert_reset,
				       priv->gmac_rst);
	if (ret)
		return ret;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;
	priv->irq = irq;

	netif_napi_add(dev, &priv->napi, rtl960x_gmac_poll);

	ret = devm_register_netdev(&pdev->dev, dev);
	if (ret)
		return dev_err_probe(&pdev->dev, ret,
				     "failed to register netdev\n");

	netdev_info(dev, "RTL960X GMAC %pR irq %d\n", res, priv->irq);

	return 0;
}

static const struct of_device_id rtl960x_gmac_of_match[] = {
	{ .compatible = "realtek,rtl9607c-gmac" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl960x_gmac_of_match);

static struct platform_driver rtl960x_gmac_driver = {
	.probe = rtl960x_gmac_probe,
	.driver = {
		.name = "rtl960x-gmac",
		.of_match_table = rtl960x_gmac_of_match,
	},
};
module_platform_driver(rtl960x_gmac_driver);

MODULE_DESCRIPTION("Realtek RTL960X Ethernet driver");
MODULE_AUTHOR("Taiga Ogawa <zectaiga@gmail.com>");
MODULE_LICENSE("GPL");
