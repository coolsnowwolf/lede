/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include "ag71xx.h"

#define AG71XX_DEFAULT_MSG_ENABLE	\
	(NETIF_MSG_DRV			\
	| NETIF_MSG_PROBE		\
	| NETIF_MSG_LINK		\
	| NETIF_MSG_TIMER		\
	| NETIF_MSG_IFDOWN		\
	| NETIF_MSG_IFUP		\
	| NETIF_MSG_RX_ERR		\
	| NETIF_MSG_TX_ERR)

static int ag71xx_msg_level = -1;

module_param_named(msg_level, ag71xx_msg_level, int, 0);
MODULE_PARM_DESC(msg_level, "Message level (-1=defaults,0=none,...,16=all)");

#define ETH_SWITCH_HEADER_LEN	2

static int ag71xx_tx_packets(struct ag71xx *ag, bool flush);

static inline unsigned int ag71xx_max_frame_len(unsigned int mtu)
{
	return ETH_SWITCH_HEADER_LEN + ETH_HLEN + VLAN_HLEN + mtu + ETH_FCS_LEN;
}

static void ag71xx_dump_dma_regs(struct ag71xx *ag)
{
	DBG("%s: dma_tx_ctrl=%08x, dma_tx_desc=%08x, dma_tx_status=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_TX_CTRL),
		ag71xx_rr(ag, AG71XX_REG_TX_DESC),
		ag71xx_rr(ag, AG71XX_REG_TX_STATUS));

	DBG("%s: dma_rx_ctrl=%08x, dma_rx_desc=%08x, dma_rx_status=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_RX_CTRL),
		ag71xx_rr(ag, AG71XX_REG_RX_DESC),
		ag71xx_rr(ag, AG71XX_REG_RX_STATUS));
}

static void ag71xx_dump_regs(struct ag71xx *ag)
{
	DBG("%s: mac_cfg1=%08x, mac_cfg2=%08x, ipg=%08x, hdx=%08x, mfl=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_MAC_CFG1),
		ag71xx_rr(ag, AG71XX_REG_MAC_CFG2),
		ag71xx_rr(ag, AG71XX_REG_MAC_IPG),
		ag71xx_rr(ag, AG71XX_REG_MAC_HDX),
		ag71xx_rr(ag, AG71XX_REG_MAC_MFL));
	DBG("%s: mac_ifctl=%08x, mac_addr1=%08x, mac_addr2=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_MAC_IFCTL),
		ag71xx_rr(ag, AG71XX_REG_MAC_ADDR1),
		ag71xx_rr(ag, AG71XX_REG_MAC_ADDR2));
	DBG("%s: fifo_cfg0=%08x, fifo_cfg1=%08x, fifo_cfg2=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG0),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG1),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG2));
	DBG("%s: fifo_cfg3=%08x, fifo_cfg4=%08x, fifo_cfg5=%08x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG3),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG4),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG5));
}

static inline void ag71xx_dump_intr(struct ag71xx *ag, char *label, u32 intr)
{
	DBG("%s: %s intr=%08x %s%s%s%s%s%s\n",
		ag->dev->name, label, intr,
		(intr & AG71XX_INT_TX_PS) ? "TXPS " : "",
		(intr & AG71XX_INT_TX_UR) ? "TXUR " : "",
		(intr & AG71XX_INT_TX_BE) ? "TXBE " : "",
		(intr & AG71XX_INT_RX_PR) ? "RXPR " : "",
		(intr & AG71XX_INT_RX_OF) ? "RXOF " : "",
		(intr & AG71XX_INT_RX_BE) ? "RXBE " : "");
}

static void ag71xx_ring_tx_clean(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->tx_ring;
	struct net_device *dev = ag->dev;
	int ring_mask = BIT(ring->order) - 1;
	u32 bytes_compl = 0, pkts_compl = 0;

	while (ring->curr != ring->dirty) {
		struct ag71xx_desc *desc;
		u32 i = ring->dirty & ring_mask;

		desc = ag71xx_ring_desc(ring, i);
		if (!ag71xx_desc_empty(desc)) {
			desc->ctrl = 0;
			dev->stats.tx_errors++;
		}

		if (ring->buf[i].skb) {
			bytes_compl += ring->buf[i].len;
			pkts_compl++;
			dev_kfree_skb_any(ring->buf[i].skb);
		}
		ring->buf[i].skb = NULL;
		ring->dirty++;
	}

	/* flush descriptors */
	wmb();

	netdev_completed_queue(dev, pkts_compl, bytes_compl);
}

static void ag71xx_ring_tx_init(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->tx_ring;
	int ring_size = BIT(ring->order);
	int ring_mask = ring_size - 1;
	int i;

	for (i = 0; i < ring_size; i++) {
		struct ag71xx_desc *desc = ag71xx_ring_desc(ring, i);

		desc->next = (u32) (ring->descs_dma +
			AG71XX_DESC_SIZE * ((i + 1) & ring_mask));

		desc->ctrl = DESC_EMPTY;
		ring->buf[i].skb = NULL;
	}

	/* flush descriptors */
	wmb();

	ring->curr = 0;
	ring->dirty = 0;
	netdev_reset_queue(ag->dev);
}

static void ag71xx_ring_rx_clean(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	int ring_size = BIT(ring->order);
	int i;

	if (!ring->buf)
		return;

	for (i = 0; i < ring_size; i++)
		if (ring->buf[i].rx_buf) {
			dma_unmap_single(&ag->dev->dev, ring->buf[i].dma_addr,
					 ag->rx_buf_size, DMA_FROM_DEVICE);
			skb_free_frag(ring->buf[i].rx_buf);
		}
}

static int ag71xx_buffer_offset(struct ag71xx *ag)
{
	int offset = NET_SKB_PAD;

	/*
	 * On AR71xx/AR91xx packets must be 4-byte aligned.
	 *
	 * When using builtin AR8216 support, hardware adds a 2-byte header,
	 * so we don't need any extra alignment in that case.
	 */
	if (!ag71xx_get_pdata(ag)->is_ar724x || ag71xx_has_ar8216(ag))
		return offset;

	return offset + NET_IP_ALIGN;
}

static int ag71xx_buffer_size(struct ag71xx *ag)
{
	return ag->rx_buf_size +
	       SKB_DATA_ALIGN(sizeof(struct skb_shared_info));
}

static bool ag71xx_fill_rx_buf(struct ag71xx *ag, struct ag71xx_buf *buf,
			       int offset,
			       void *(*alloc)(unsigned int size))
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	struct ag71xx_desc *desc = ag71xx_ring_desc(ring, buf - &ring->buf[0]);
	void *data;

	data = alloc(ag71xx_buffer_size(ag));
	if (!data)
		return false;

	buf->rx_buf = data;
	buf->dma_addr = dma_map_single(&ag->dev->dev, data, ag->rx_buf_size,
				       DMA_FROM_DEVICE);
	desc->data = (u32) buf->dma_addr + offset;
	return true;
}

static int ag71xx_ring_rx_init(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	int ring_size = BIT(ring->order);
	int ring_mask = BIT(ring->order) - 1;
	unsigned int i;
	int ret;
	int offset = ag71xx_buffer_offset(ag);

	ret = 0;
	for (i = 0; i < ring_size; i++) {
		struct ag71xx_desc *desc = ag71xx_ring_desc(ring, i);

		desc->next = (u32) (ring->descs_dma +
			AG71XX_DESC_SIZE * ((i + 1) & ring_mask));

		DBG("ag71xx: RX desc at %p, next is %08x\n",
			desc, desc->next);
	}

	for (i = 0; i < ring_size; i++) {
		struct ag71xx_desc *desc = ag71xx_ring_desc(ring, i);

		if (!ag71xx_fill_rx_buf(ag, &ring->buf[i], offset,
					netdev_alloc_frag)) {
			ret = -ENOMEM;
			break;
		}

		desc->ctrl = DESC_EMPTY;
	}

	/* flush descriptors */
	wmb();

	ring->curr = 0;
	ring->dirty = 0;

	return ret;
}

static int ag71xx_ring_rx_refill(struct ag71xx *ag)
{
	struct ag71xx_ring *ring = &ag->rx_ring;
	int ring_mask = BIT(ring->order) - 1;
	unsigned int count;
	int offset = ag71xx_buffer_offset(ag);

	count = 0;
	for (; ring->curr - ring->dirty > 0; ring->dirty++) {
		struct ag71xx_desc *desc;
		unsigned int i;

		i = ring->dirty & ring_mask;
		desc = ag71xx_ring_desc(ring, i);

		if (!ring->buf[i].rx_buf &&
		    !ag71xx_fill_rx_buf(ag, &ring->buf[i], offset,
					napi_alloc_frag))
			break;

		desc->ctrl = DESC_EMPTY;
		count++;
	}

	/* flush descriptors */
	wmb();

	DBG("%s: %u rx descriptors refilled\n", ag->dev->name, count);

	return count;
}

static int ag71xx_rings_init(struct ag71xx *ag)
{
	struct ag71xx_ring *tx = &ag->tx_ring;
	struct ag71xx_ring *rx = &ag->rx_ring;
	int ring_size = BIT(tx->order) + BIT(rx->order);
	int tx_size = BIT(tx->order);

	tx->buf = kzalloc(ring_size * sizeof(*tx->buf), GFP_KERNEL);
	if (!tx->buf)
		return -ENOMEM;

	tx->descs_cpu = dma_alloc_coherent(NULL, ring_size * AG71XX_DESC_SIZE,
					   &tx->descs_dma, GFP_ATOMIC);
	if (!tx->descs_cpu) {
		kfree(tx->buf);
		tx->buf = NULL;
		return -ENOMEM;
	}

	rx->buf = &tx->buf[BIT(tx->order)];
	rx->descs_cpu = ((void *)tx->descs_cpu) + tx_size * AG71XX_DESC_SIZE;
	rx->descs_dma = tx->descs_dma + tx_size * AG71XX_DESC_SIZE;

	ag71xx_ring_tx_init(ag);
	return ag71xx_ring_rx_init(ag);
}

static void ag71xx_rings_free(struct ag71xx *ag)
{
	struct ag71xx_ring *tx = &ag->tx_ring;
	struct ag71xx_ring *rx = &ag->rx_ring;
	int ring_size = BIT(tx->order) + BIT(rx->order);

	if (tx->descs_cpu)
		dma_free_coherent(NULL, ring_size * AG71XX_DESC_SIZE,
				  tx->descs_cpu, tx->descs_dma);

	kfree(tx->buf);

	tx->descs_cpu = NULL;
	rx->descs_cpu = NULL;
	tx->buf = NULL;
	rx->buf = NULL;
}

static void ag71xx_rings_cleanup(struct ag71xx *ag)
{
	ag71xx_ring_rx_clean(ag);
	ag71xx_ring_tx_clean(ag);
	ag71xx_rings_free(ag);

	netdev_reset_queue(ag->dev);
}

static unsigned char *ag71xx_speed_str(struct ag71xx *ag)
{
	switch (ag->speed) {
	case SPEED_1000:
		return "1000";
	case SPEED_100:
		return "100";
	case SPEED_10:
		return "10";
	}

	return "?";
}

static void ag71xx_hw_set_macaddr(struct ag71xx *ag, unsigned char *mac)
{
	u32 t;

	t = (((u32) mac[5]) << 24) | (((u32) mac[4]) << 16)
	  | (((u32) mac[3]) << 8) | ((u32) mac[2]);

	ag71xx_wr(ag, AG71XX_REG_MAC_ADDR1, t);

	t = (((u32) mac[1]) << 24) | (((u32) mac[0]) << 16);
	ag71xx_wr(ag, AG71XX_REG_MAC_ADDR2, t);
}

static void ag71xx_dma_reset(struct ag71xx *ag)
{
	u32 val;
	int i;

	ag71xx_dump_dma_regs(ag);

	/* stop RX and TX */
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, 0);
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, 0);

	/*
	 * give the hardware some time to really stop all rx/tx activity
	 * clearing the descriptors too early causes random memory corruption
	 */
	mdelay(1);

	/* clear descriptor addresses */
	ag71xx_wr(ag, AG71XX_REG_TX_DESC, ag->stop_desc_dma);
	ag71xx_wr(ag, AG71XX_REG_RX_DESC, ag->stop_desc_dma);

	/* clear pending RX/TX interrupts */
	for (i = 0; i < 256; i++) {
		ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_PR);
		ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_PS);
	}

	/* clear pending errors */
	ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_BE | RX_STATUS_OF);
	ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_BE | TX_STATUS_UR);

	val = ag71xx_rr(ag, AG71XX_REG_RX_STATUS);
	if (val)
		pr_alert("%s: unable to clear DMA Rx status: %08x\n",
			 ag->dev->name, val);

	val = ag71xx_rr(ag, AG71XX_REG_TX_STATUS);

	/* mask out reserved bits */
	val &= ~0xff000000;

	if (val)
		pr_alert("%s: unable to clear DMA Tx status: %08x\n",
			 ag->dev->name, val);

	ag71xx_dump_dma_regs(ag);
}

#define MAC_CFG1_INIT	(MAC_CFG1_RXE | MAC_CFG1_TXE | \
			 MAC_CFG1_SRX | MAC_CFG1_STX)

#define FIFO_CFG0_INIT	(FIFO_CFG0_ALL << FIFO_CFG0_ENABLE_SHIFT)

#define FIFO_CFG4_INIT	(FIFO_CFG4_DE | FIFO_CFG4_DV | FIFO_CFG4_FC | \
			 FIFO_CFG4_CE | FIFO_CFG4_CR | FIFO_CFG4_LM | \
			 FIFO_CFG4_LO | FIFO_CFG4_OK | FIFO_CFG4_MC | \
			 FIFO_CFG4_BC | FIFO_CFG4_DR | FIFO_CFG4_LE | \
			 FIFO_CFG4_CF | FIFO_CFG4_PF | FIFO_CFG4_UO | \
			 FIFO_CFG4_VT)

#define FIFO_CFG5_INIT	(FIFO_CFG5_DE | FIFO_CFG5_DV | FIFO_CFG5_FC | \
			 FIFO_CFG5_CE | FIFO_CFG5_LO | FIFO_CFG5_OK | \
			 FIFO_CFG5_MC | FIFO_CFG5_BC | FIFO_CFG5_DR | \
			 FIFO_CFG5_CF | FIFO_CFG5_PF | FIFO_CFG5_VT | \
			 FIFO_CFG5_LE | FIFO_CFG5_FT | FIFO_CFG5_16 | \
			 FIFO_CFG5_17 | FIFO_CFG5_SF)

static void ag71xx_hw_stop(struct ag71xx *ag)
{
	/* disable all interrupts and stop the rx/tx engine */
	ag71xx_wr(ag, AG71XX_REG_INT_ENABLE, 0);
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, 0);
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, 0);
}

static void ag71xx_hw_setup(struct ag71xx *ag)
{
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	u32 init = MAC_CFG1_INIT;

	/* setup MAC configuration registers */
	if (pdata->use_flow_control)
		init |= MAC_CFG1_TFC | MAC_CFG1_RFC;
	ag71xx_wr(ag, AG71XX_REG_MAC_CFG1, init);

	ag71xx_sb(ag, AG71XX_REG_MAC_CFG2,
		  MAC_CFG2_PAD_CRC_EN | MAC_CFG2_LEN_CHECK);

	/* setup max frame length to zero */
	ag71xx_wr(ag, AG71XX_REG_MAC_MFL, 0);

	/* setup FIFO configuration registers */
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG0, FIFO_CFG0_INIT);
	if (pdata->is_ar724x) {
		ag71xx_wr(ag, AG71XX_REG_FIFO_CFG1, 0x0010ffff);
		ag71xx_wr(ag, AG71XX_REG_FIFO_CFG2, 0x015500aa);
	} else {
		ag71xx_wr(ag, AG71XX_REG_FIFO_CFG1, 0x0fff0000);
		ag71xx_wr(ag, AG71XX_REG_FIFO_CFG2, 0x00001fff);
	}
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG4, FIFO_CFG4_INIT);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, FIFO_CFG5_INIT);
}

static void ag71xx_hw_init(struct ag71xx *ag)
{
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	u32 reset_mask = pdata->reset_bit;

	ag71xx_hw_stop(ag);

	if (pdata->is_ar724x) {
		u32 reset_phy = reset_mask;

		reset_phy &= AR71XX_RESET_GE0_PHY | AR71XX_RESET_GE1_PHY;
		reset_mask &= ~(AR71XX_RESET_GE0_PHY | AR71XX_RESET_GE1_PHY);

		ath79_device_reset_set(reset_phy);
		msleep(50);
		ath79_device_reset_clear(reset_phy);
		msleep(200);
	}

	ag71xx_sb(ag, AG71XX_REG_MAC_CFG1, MAC_CFG1_SR);
	udelay(20);

	ath79_device_reset_set(reset_mask);
	msleep(100);
	ath79_device_reset_clear(reset_mask);
	msleep(200);

	ag71xx_hw_setup(ag);

	ag71xx_dma_reset(ag);
}

static void ag71xx_fast_reset(struct ag71xx *ag)
{
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	struct net_device *dev = ag->dev;
	u32 reset_mask = pdata->reset_bit;
	u32 rx_ds;
	u32 mii_reg;

	reset_mask &= AR71XX_RESET_GE0_MAC | AR71XX_RESET_GE1_MAC;

	ag71xx_hw_stop(ag);
	wmb();

	mii_reg = ag71xx_rr(ag, AG71XX_REG_MII_CFG);
	rx_ds = ag71xx_rr(ag, AG71XX_REG_RX_DESC);

	ag71xx_tx_packets(ag, true);

	ath79_device_reset_set(reset_mask);
	udelay(10);
	ath79_device_reset_clear(reset_mask);
	udelay(10);

	ag71xx_dma_reset(ag);
	ag71xx_hw_setup(ag);
	ag->tx_ring.curr = 0;
	ag->tx_ring.dirty = 0;
	netdev_reset_queue(ag->dev);

	/* setup max frame length */
	ag71xx_wr(ag, AG71XX_REG_MAC_MFL,
		  ag71xx_max_frame_len(ag->dev->mtu));

	ag71xx_wr(ag, AG71XX_REG_RX_DESC, rx_ds);
	ag71xx_wr(ag, AG71XX_REG_TX_DESC, ag->tx_ring.descs_dma);
	ag71xx_wr(ag, AG71XX_REG_MII_CFG, mii_reg);

	ag71xx_hw_set_macaddr(ag, dev->dev_addr);
}

static void ag71xx_hw_start(struct ag71xx *ag)
{
	/* start RX engine */
	ag71xx_wr(ag, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);

	/* enable interrupts */
	ag71xx_wr(ag, AG71XX_REG_INT_ENABLE, AG71XX_INT_INIT);

	netif_wake_queue(ag->dev);
}

static void
__ag71xx_link_adjust(struct ag71xx *ag, bool update)
{
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	u32 cfg2;
	u32 ifctl;
	u32 fifo5;
	u32 fifo3;

	if (!ag->link && update) {
		ag71xx_hw_stop(ag);
		netif_carrier_off(ag->dev);
		if (netif_msg_link(ag))
			pr_info("%s: link down\n", ag->dev->name);
		return;
	}

	if (pdata->is_ar724x)
		ag71xx_fast_reset(ag);

	cfg2 = ag71xx_rr(ag, AG71XX_REG_MAC_CFG2);
	cfg2 &= ~(MAC_CFG2_IF_1000 | MAC_CFG2_IF_10_100 | MAC_CFG2_FDX);
	cfg2 |= (ag->duplex) ? MAC_CFG2_FDX : 0;

	ifctl = ag71xx_rr(ag, AG71XX_REG_MAC_IFCTL);
	ifctl &= ~(MAC_IFCTL_SPEED);

	fifo5 = ag71xx_rr(ag, AG71XX_REG_FIFO_CFG5);
	fifo5 &= ~FIFO_CFG5_BM;

	switch (ag->speed) {
	case SPEED_1000:
		cfg2 |= MAC_CFG2_IF_1000;
		fifo5 |= FIFO_CFG5_BM;
		break;
	case SPEED_100:
		cfg2 |= MAC_CFG2_IF_10_100;
		ifctl |= MAC_IFCTL_SPEED;
		break;
	case SPEED_10:
		cfg2 |= MAC_CFG2_IF_10_100;
		break;
	default:
		BUG();
		return;
	}

	if (pdata->is_ar91xx)
		fifo3 = 0x00780fff;
	else if (pdata->is_ar724x)
		fifo3 = 0x01f00140;
	else
		fifo3 = 0x008001ff;

	if (ag->tx_ring.desc_split) {
		fifo3 &= 0xffff;
		fifo3 |= ((2048 - ag->tx_ring.desc_split) / 4) << 16;
	}

	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG3, fifo3);

	if (update && pdata->set_speed)
		pdata->set_speed(ag->speed);

	ag71xx_wr(ag, AG71XX_REG_MAC_CFG2, cfg2);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, fifo5);
	ag71xx_wr(ag, AG71XX_REG_MAC_IFCTL, ifctl);

	if (pdata->disable_inline_checksum_engine) {
		/*
		 * The rx ring buffer can stall on small packets on QCA953x and
		 * QCA956x. Disabling the inline checksum engine fixes the stall.
		 * The wr, rr functions cannot be used since this hidden register
		 * is outside of the normal ag71xx register block.
		 */
		void __iomem *dam = ioremap_nocache(0xb90001bc, 0x4);
		if (dam) {
			__raw_writel(__raw_readl(dam) & ~BIT(27), dam);
			(void)__raw_readl(dam);
			iounmap(dam);
		}
	}

	ag71xx_hw_start(ag);

	netif_carrier_on(ag->dev);
	if (update && netif_msg_link(ag))
		pr_info("%s: link up (%sMbps/%s duplex)\n",
			ag->dev->name,
			ag71xx_speed_str(ag),
			(DUPLEX_FULL == ag->duplex) ? "Full" : "Half");

	DBG("%s: fifo_cfg0=%#x, fifo_cfg1=%#x, fifo_cfg2=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG0),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG1),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG2));

	DBG("%s: fifo_cfg3=%#x, fifo_cfg4=%#x, fifo_cfg5=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG3),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG4),
		ag71xx_rr(ag, AG71XX_REG_FIFO_CFG5));

	DBG("%s: mac_cfg2=%#x, mac_ifctl=%#x\n",
		ag->dev->name,
		ag71xx_rr(ag, AG71XX_REG_MAC_CFG2),
		ag71xx_rr(ag, AG71XX_REG_MAC_IFCTL));
}

void ag71xx_link_adjust(struct ag71xx *ag)
{
	__ag71xx_link_adjust(ag, true);
}

static int ag71xx_hw_enable(struct ag71xx *ag)
{
	int ret;

	ret = ag71xx_rings_init(ag);
	if (ret)
		return ret;

	napi_enable(&ag->napi);
	ag71xx_wr(ag, AG71XX_REG_TX_DESC, ag->tx_ring.descs_dma);
	ag71xx_wr(ag, AG71XX_REG_RX_DESC, ag->rx_ring.descs_dma);
	netif_start_queue(ag->dev);

	return 0;
}

static void ag71xx_hw_disable(struct ag71xx *ag)
{
	unsigned long flags;

	spin_lock_irqsave(&ag->lock, flags);

	netif_stop_queue(ag->dev);

	ag71xx_hw_stop(ag);
	ag71xx_dma_reset(ag);

	napi_disable(&ag->napi);
	del_timer_sync(&ag->oom_timer);

	spin_unlock_irqrestore(&ag->lock, flags);

	ag71xx_rings_cleanup(ag);
}

static int ag71xx_open(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);
	unsigned int max_frame_len;
	int ret;

	netif_carrier_off(dev);
	max_frame_len = ag71xx_max_frame_len(dev->mtu);
	ag->rx_buf_size = SKB_DATA_ALIGN(max_frame_len + NET_SKB_PAD + NET_IP_ALIGN);

	/* setup max frame length */
	ag71xx_wr(ag, AG71XX_REG_MAC_MFL, max_frame_len);
	ag71xx_hw_set_macaddr(ag, dev->dev_addr);

	ret = ag71xx_hw_enable(ag);
	if (ret)
		goto err;

	ag71xx_phy_start(ag);

	return 0;

err:
	ag71xx_rings_cleanup(ag);
	return ret;
}

static int ag71xx_stop(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);

	netif_carrier_off(dev);
	ag71xx_phy_stop(ag);
	ag71xx_hw_disable(ag);

	return 0;
}

static int ag71xx_fill_dma_desc(struct ag71xx_ring *ring, u32 addr, int len)
{
	int i;
	struct ag71xx_desc *desc;
	int ring_mask = BIT(ring->order) - 1;
	int ndesc = 0;
	int split = ring->desc_split;

	if (!split)
		split = len;

	while (len > 0) {
		unsigned int cur_len = len;

		i = (ring->curr + ndesc) & ring_mask;
		desc = ag71xx_ring_desc(ring, i);

		if (!ag71xx_desc_empty(desc))
			return -1;

		if (cur_len > split) {
			cur_len = split;

			/*
			 * TX will hang if DMA transfers <= 4 bytes,
			 * make sure next segment is more than 4 bytes long.
			 */
			if (len <= split + 4)
				cur_len -= 4;
		}

		desc->data = addr;
		addr += cur_len;
		len -= cur_len;

		if (len > 0)
			cur_len |= DESC_MORE;

		/* prevent early tx attempt of this descriptor */
		if (!ndesc)
			cur_len |= DESC_EMPTY;

		desc->ctrl = cur_len;
		ndesc++;
	}

	return ndesc;
}

static netdev_tx_t ag71xx_hard_start_xmit(struct sk_buff *skb,
					  struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);
	struct ag71xx_ring *ring = &ag->tx_ring;
	int ring_mask = BIT(ring->order) - 1;
	int ring_size = BIT(ring->order);
	struct ag71xx_desc *desc;
	dma_addr_t dma_addr;
	int i, n, ring_min;

	if (ag71xx_has_ar8216(ag))
		ag71xx_add_ar8216_header(ag, skb);

	if (skb->len <= 4) {
		DBG("%s: packet len is too small\n", ag->dev->name);
		goto err_drop;
	}

	dma_addr = dma_map_single(&dev->dev, skb->data, skb->len,
				  DMA_TO_DEVICE);

	i = ring->curr & ring_mask;
	desc = ag71xx_ring_desc(ring, i);

	/* setup descriptor fields */
	n = ag71xx_fill_dma_desc(ring, (u32) dma_addr, skb->len & ag->desc_pktlen_mask);
	if (n < 0)
		goto err_drop_unmap;

	i = (ring->curr + n - 1) & ring_mask;
	ring->buf[i].len = skb->len;
	ring->buf[i].skb = skb;

	netdev_sent_queue(dev, skb->len);

	skb_tx_timestamp(skb);

	desc->ctrl &= ~DESC_EMPTY;
	ring->curr += n;

	/* flush descriptor */
	wmb();

	ring_min = 2;
	if (ring->desc_split)
	    ring_min *= AG71XX_TX_RING_DS_PER_PKT;

	if (ring->curr - ring->dirty >= ring_size - ring_min) {
		DBG("%s: tx queue full\n", dev->name);
		netif_stop_queue(dev);
	}

	DBG("%s: packet injected into TX queue\n", ag->dev->name);

	/* enable TX engine */
	ag71xx_wr(ag, AG71XX_REG_TX_CTRL, TX_CTRL_TXE);

	return NETDEV_TX_OK;

err_drop_unmap:
	dma_unmap_single(&dev->dev, dma_addr, skb->len, DMA_TO_DEVICE);

err_drop:
	dev->stats.tx_dropped++;

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static int ag71xx_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct ag71xx *ag = netdev_priv(dev);
	int ret;

	switch (cmd) {
	case SIOCETHTOOL:
		if (ag->phy_dev == NULL)
			break;

		spin_lock_irq(&ag->lock);
		ret = phy_ethtool_ioctl(ag->phy_dev, (void *) ifr->ifr_data);
		spin_unlock_irq(&ag->lock);
		return ret;

	case SIOCSIFHWADDR:
		if (copy_from_user
			(dev->dev_addr, ifr->ifr_data, sizeof(dev->dev_addr)))
			return -EFAULT;
		return 0;

	case SIOCGIFHWADDR:
		if (copy_to_user
			(ifr->ifr_data, dev->dev_addr, sizeof(dev->dev_addr)))
			return -EFAULT;
		return 0;

	case SIOCGMIIPHY:
	case SIOCGMIIREG:
	case SIOCSMIIREG:
		if (ag->phy_dev == NULL)
			break;

		return phy_mii_ioctl(ag->phy_dev, ifr, cmd);

	default:
		break;
	}

	return -EOPNOTSUPP;
}

static void ag71xx_oom_timer_handler(unsigned long data)
{
	struct net_device *dev = (struct net_device *) data;
	struct ag71xx *ag = netdev_priv(dev);

	napi_schedule(&ag->napi);
}

static void ag71xx_tx_timeout(struct net_device *dev)
{
	struct ag71xx *ag = netdev_priv(dev);

	if (netif_msg_tx_err(ag))
		pr_info("%s: tx timeout\n", ag->dev->name);

	schedule_delayed_work(&ag->restart_work, 1);
}

static void ag71xx_restart_work_func(struct work_struct *work)
{
	struct ag71xx *ag = container_of(work, struct ag71xx, restart_work.work);

	rtnl_lock();
	ag71xx_hw_disable(ag);
	ag71xx_hw_enable(ag);
	if (ag->link)
		__ag71xx_link_adjust(ag, false);
	rtnl_unlock();
}

static bool ag71xx_check_dma_stuck(struct ag71xx *ag)
{
	unsigned long timestamp;
	u32 rx_sm, tx_sm, rx_fd;

	timestamp = netdev_get_tx_queue(ag->dev, 0)->trans_start;
	if (likely(time_before(jiffies, timestamp + HZ/10)))
		return false;

	if (!netif_carrier_ok(ag->dev))
		return false;

	rx_sm = ag71xx_rr(ag, AG71XX_REG_RX_SM);
	if ((rx_sm & 0x7) == 0x3 && ((rx_sm >> 4) & 0x7) == 0x6)
		return true;

	tx_sm = ag71xx_rr(ag, AG71XX_REG_TX_SM);
	rx_fd = ag71xx_rr(ag, AG71XX_REG_FIFO_DEPTH);
	if (((tx_sm >> 4) & 0x7) == 0 && ((rx_sm & 0x7) == 0) &&
	    ((rx_sm >> 4) & 0x7) == 0 && rx_fd == 0)
		return true;

	return false;
}

static int ag71xx_tx_packets(struct ag71xx *ag, bool flush)
{
	struct ag71xx_ring *ring = &ag->tx_ring;
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	bool dma_stuck = false;
	int ring_mask = BIT(ring->order) - 1;
	int ring_size = BIT(ring->order);
	int sent = 0;
	int bytes_compl = 0;
	int n = 0;

	DBG("%s: processing TX ring\n", ag->dev->name);

	while (ring->dirty + n != ring->curr) {
		unsigned int i = (ring->dirty + n) & ring_mask;
		struct ag71xx_desc *desc = ag71xx_ring_desc(ring, i);
		struct sk_buff *skb = ring->buf[i].skb;

		if (!flush && !ag71xx_desc_empty(desc)) {
			if (pdata->is_ar724x &&
			    ag71xx_check_dma_stuck(ag)) {
				schedule_delayed_work(&ag->restart_work, HZ / 2);
				dma_stuck = true;
			}
			break;
		}

		if (flush)
			desc->ctrl |= DESC_EMPTY;

		n++;
		if (!skb)
			continue;

		dev_kfree_skb_any(skb);
		ring->buf[i].skb = NULL;

		bytes_compl += ring->buf[i].len;

		sent++;
		ring->dirty += n;

		while (n > 0) {
			ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_PS);
			n--;
		}
	}

	DBG("%s: %d packets sent out\n", ag->dev->name, sent);

	if (!sent)
		return 0;

	ag->dev->stats.tx_bytes += bytes_compl;
	ag->dev->stats.tx_packets += sent;

	netdev_completed_queue(ag->dev, sent, bytes_compl);
	if ((ring->curr - ring->dirty) < (ring_size * 3) / 4)
		netif_wake_queue(ag->dev);

	if (!dma_stuck)
		cancel_delayed_work(&ag->restart_work);

	return sent;
}

static int ag71xx_rx_packets(struct ag71xx *ag, int limit)
{
	struct net_device *dev = ag->dev;
	struct ag71xx_ring *ring = &ag->rx_ring;
	int offset = ag71xx_buffer_offset(ag);
	unsigned int pktlen_mask = ag->desc_pktlen_mask;
	int ring_mask = BIT(ring->order) - 1;
	int ring_size = BIT(ring->order);
	struct sk_buff_head queue;
	struct sk_buff *skb;
	int done = 0;

	DBG("%s: rx packets, limit=%d, curr=%u, dirty=%u\n",
			dev->name, limit, ring->curr, ring->dirty);

	skb_queue_head_init(&queue);

	while (done < limit) {
		unsigned int i = ring->curr & ring_mask;
		struct ag71xx_desc *desc = ag71xx_ring_desc(ring, i);
		int pktlen;
		int err = 0;

		if (ag71xx_desc_empty(desc))
			break;

		if ((ring->dirty + ring_size) == ring->curr) {
			ag71xx_assert(0);
			break;
		}

		ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_PR);

		pktlen = desc->ctrl & pktlen_mask;
		pktlen -= ETH_FCS_LEN;

		dma_unmap_single(&dev->dev, ring->buf[i].dma_addr,
				 ag->rx_buf_size, DMA_FROM_DEVICE);

		dev->stats.rx_packets++;
		dev->stats.rx_bytes += pktlen;

		skb = build_skb(ring->buf[i].rx_buf, ag71xx_buffer_size(ag));
		if (!skb) {
			skb_free_frag(ring->buf[i].rx_buf);
			goto next;
		}

		skb_reserve(skb, offset);
		skb_put(skb, pktlen);

		if (ag71xx_has_ar8216(ag))
			err = ag71xx_remove_ar8216_header(ag, skb, pktlen);

		if (err) {
			dev->stats.rx_dropped++;
			kfree_skb(skb);
		} else {
			skb->dev = dev;
			skb->ip_summed = CHECKSUM_NONE;
			__skb_queue_tail(&queue, skb);
		}

next:
		ring->buf[i].rx_buf = NULL;
		done++;

		ring->curr++;
	}

	ag71xx_ring_rx_refill(ag);

	while ((skb = __skb_dequeue(&queue)) != NULL) {
		skb->protocol = eth_type_trans(skb, dev);
		netif_receive_skb(skb);
	}

	DBG("%s: rx finish, curr=%u, dirty=%u, done=%d\n",
		dev->name, ring->curr, ring->dirty, done);

	return done;
}

static int ag71xx_poll(struct napi_struct *napi, int limit)
{
	struct ag71xx *ag = container_of(napi, struct ag71xx, napi);
	struct ag71xx_platform_data *pdata = ag71xx_get_pdata(ag);
	struct net_device *dev = ag->dev;
	struct ag71xx_ring *rx_ring = &ag->rx_ring;
	int rx_ring_size = BIT(rx_ring->order);
	unsigned long flags;
	u32 status;
	int tx_done;
	int rx_done;

	pdata->ddr_flush();
	tx_done = ag71xx_tx_packets(ag, false);

	DBG("%s: processing RX ring\n", dev->name);
	rx_done = ag71xx_rx_packets(ag, limit);

	ag71xx_debugfs_update_napi_stats(ag, rx_done, tx_done);

	if (rx_ring->buf[rx_ring->dirty % rx_ring_size].rx_buf == NULL)
		goto oom;

	status = ag71xx_rr(ag, AG71XX_REG_RX_STATUS);
	if (unlikely(status & RX_STATUS_OF)) {
		ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_OF);
		dev->stats.rx_fifo_errors++;

		/* restart RX */
		ag71xx_wr(ag, AG71XX_REG_RX_CTRL, RX_CTRL_RXE);
	}

	if (rx_done < limit) {
		if (status & RX_STATUS_PR)
			goto more;

		status = ag71xx_rr(ag, AG71XX_REG_TX_STATUS);
		if (status & TX_STATUS_PS)
			goto more;

		DBG("%s: disable polling mode, rx=%d, tx=%d,limit=%d\n",
			dev->name, rx_done, tx_done, limit);

		napi_complete(napi);

		/* enable interrupts */
		spin_lock_irqsave(&ag->lock, flags);
		ag71xx_int_enable(ag, AG71XX_INT_POLL);
		spin_unlock_irqrestore(&ag->lock, flags);
		return rx_done;
	}

more:
	DBG("%s: stay in polling mode, rx=%d, tx=%d, limit=%d\n",
			dev->name, rx_done, tx_done, limit);
	return limit;

oom:
	if (netif_msg_rx_err(ag))
		pr_info("%s: out of memory\n", dev->name);

	mod_timer(&ag->oom_timer, jiffies + AG71XX_OOM_REFILL);
	napi_complete(napi);
	return 0;
}

static irqreturn_t ag71xx_interrupt(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct ag71xx *ag = netdev_priv(dev);
	u32 status;

	status = ag71xx_rr(ag, AG71XX_REG_INT_STATUS);
	ag71xx_dump_intr(ag, "raw", status);

	if (unlikely(!status))
		return IRQ_NONE;

	if (unlikely(status & AG71XX_INT_ERR)) {
		if (status & AG71XX_INT_TX_BE) {
			ag71xx_wr(ag, AG71XX_REG_TX_STATUS, TX_STATUS_BE);
			dev_err(&dev->dev, "TX BUS error\n");
		}
		if (status & AG71XX_INT_RX_BE) {
			ag71xx_wr(ag, AG71XX_REG_RX_STATUS, RX_STATUS_BE);
			dev_err(&dev->dev, "RX BUS error\n");
		}
	}

	if (likely(status & AG71XX_INT_POLL)) {
		ag71xx_int_disable(ag, AG71XX_INT_POLL);
		DBG("%s: enable polling mode\n", dev->name);
		napi_schedule(&ag->napi);
	}

	ag71xx_debugfs_update_int_stats(ag, status);

	return IRQ_HANDLED;
}

#ifdef CONFIG_NET_POLL_CONTROLLER
/*
 * Polling 'interrupt' - used by things like netconsole to send skbs
 * without having to re-enable interrupts. It's not called while
 * the interrupt routine is executing.
 */
static void ag71xx_netpoll(struct net_device *dev)
{
	disable_irq(dev->irq);
	ag71xx_interrupt(dev->irq, dev);
	enable_irq(dev->irq);
}
#endif

static int ag71xx_change_mtu(struct net_device *dev, int new_mtu)
{
	struct ag71xx *ag = netdev_priv(dev);
	unsigned int max_frame_len;

	max_frame_len = ag71xx_max_frame_len(new_mtu);
	if (new_mtu < 68 || max_frame_len > ag->max_frame_len)
		return -EINVAL;

	if (netif_running(dev))
		return -EBUSY;

	dev->mtu = new_mtu;
	return 0;
}

static const struct net_device_ops ag71xx_netdev_ops = {
	.ndo_open		= ag71xx_open,
	.ndo_stop		= ag71xx_stop,
	.ndo_start_xmit		= ag71xx_hard_start_xmit,
	.ndo_do_ioctl		= ag71xx_do_ioctl,
	.ndo_tx_timeout		= ag71xx_tx_timeout,
	.ndo_change_mtu		= ag71xx_change_mtu,
	.ndo_set_mac_address	= eth_mac_addr,
	.ndo_validate_addr	= eth_validate_addr,
#ifdef CONFIG_NET_POLL_CONTROLLER
	.ndo_poll_controller	= ag71xx_netpoll,
#endif
};

static const char *ag71xx_get_phy_if_mode_name(phy_interface_t mode)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_MII:
		return "MII";
	case PHY_INTERFACE_MODE_GMII:
		return "GMII";
	case PHY_INTERFACE_MODE_RMII:
		return "RMII";
	case PHY_INTERFACE_MODE_RGMII:
		return "RGMII";
	case PHY_INTERFACE_MODE_SGMII:
		return "SGMII";
	default:
		break;
	}

	return "unknown";
}


static int ag71xx_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct resource *res;
	struct ag71xx *ag;
	struct ag71xx_platform_data *pdata;
	int tx_size, err;

	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_err(&pdev->dev, "no platform data specified\n");
		err = -ENXIO;
		goto err_out;
	}

	if (pdata->mii_bus_dev == NULL && pdata->phy_mask) {
		dev_err(&pdev->dev, "no MII bus device specified\n");
		err = -EINVAL;
		goto err_out;
	}

	dev = alloc_etherdev(sizeof(*ag));
	if (!dev) {
		dev_err(&pdev->dev, "alloc_etherdev failed\n");
		err = -ENOMEM;
		goto err_out;
	}

	if (!pdata->max_frame_len || !pdata->desc_pktlen_mask)
		return -EINVAL;

	SET_NETDEV_DEV(dev, &pdev->dev);

	ag = netdev_priv(dev);
	ag->pdev = pdev;
	ag->dev = dev;
	ag->msg_enable = netif_msg_init(ag71xx_msg_level,
					AG71XX_DEFAULT_MSG_ENABLE);
	spin_lock_init(&ag->lock);

	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mac_base");
	if (!res) {
		dev_err(&pdev->dev, "no mac_base resource found\n");
		err = -ENXIO;
		goto err_out;
	}

	ag->mac_base = ioremap_nocache(res->start, res->end - res->start + 1);
	if (!ag->mac_base) {
		dev_err(&pdev->dev, "unable to ioremap mac_base\n");
		err = -ENOMEM;
		goto err_free_dev;
	}

	dev->irq = platform_get_irq(pdev, 0);
	err = request_irq(dev->irq, ag71xx_interrupt,
			  0x0,
			  dev->name, dev);
	if (err) {
		dev_err(&pdev->dev, "unable to request IRQ %d\n", dev->irq);
		goto err_unmap_base;
	}

	dev->base_addr = (unsigned long)ag->mac_base;
	dev->netdev_ops = &ag71xx_netdev_ops;
	dev->ethtool_ops = &ag71xx_ethtool_ops;

	INIT_DELAYED_WORK(&ag->restart_work, ag71xx_restart_work_func);

	init_timer(&ag->oom_timer);
	ag->oom_timer.data = (unsigned long) dev;
	ag->oom_timer.function = ag71xx_oom_timer_handler;

	tx_size = AG71XX_TX_RING_SIZE_DEFAULT;
	ag->rx_ring.order = ag71xx_ring_size_order(AG71XX_RX_RING_SIZE_DEFAULT);

	ag->max_frame_len = pdata->max_frame_len;
	ag->desc_pktlen_mask = pdata->desc_pktlen_mask;

	if (!pdata->is_ar724x && !pdata->is_ar91xx) {
		ag->tx_ring.desc_split = AG71XX_TX_RING_SPLIT;
		tx_size *= AG71XX_TX_RING_DS_PER_PKT;
	}
	ag->tx_ring.order = ag71xx_ring_size_order(tx_size);

	ag->stop_desc = dma_alloc_coherent(NULL,
		sizeof(struct ag71xx_desc), &ag->stop_desc_dma, GFP_KERNEL);

	if (!ag->stop_desc)
		goto err_free_irq;

	ag->stop_desc->data = 0;
	ag->stop_desc->ctrl = 0;
	ag->stop_desc->next = (u32) ag->stop_desc_dma;

	memcpy(dev->dev_addr, pdata->mac_addr, ETH_ALEN);

	netif_napi_add(dev, &ag->napi, ag71xx_poll, AG71XX_NAPI_WEIGHT);

	ag71xx_dump_regs(ag);

	ag71xx_hw_init(ag);

	ag71xx_dump_regs(ag);

	err = ag71xx_phy_connect(ag);
	if (err)
		goto err_free_desc;

	err = ag71xx_debugfs_init(ag);
	if (err)
		goto err_phy_disconnect;

	platform_set_drvdata(pdev, dev);

	err = register_netdev(dev);
	if (err) {
		dev_err(&pdev->dev, "unable to register net device\n");
		goto err_debugfs_exit;
	}

	pr_info("%s: Atheros AG71xx at 0x%08lx, irq %d, mode:%s\n",
		dev->name, dev->base_addr, dev->irq,
		ag71xx_get_phy_if_mode_name(pdata->phy_if_mode));

	return 0;

err_debugfs_exit:
	ag71xx_debugfs_exit(ag);
err_phy_disconnect:
	ag71xx_phy_disconnect(ag);
err_free_desc:
	dma_free_coherent(NULL, sizeof(struct ag71xx_desc), ag->stop_desc,
			  ag->stop_desc_dma);
err_free_irq:
	free_irq(dev->irq, dev);
err_unmap_base:
	iounmap(ag->mac_base);
err_free_dev:
	kfree(dev);
err_out:
	platform_set_drvdata(pdev, NULL);
	return err;
}

static int ag71xx_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);

	if (dev) {
		struct ag71xx *ag = netdev_priv(dev);

		ag71xx_debugfs_exit(ag);
		ag71xx_phy_disconnect(ag);
		unregister_netdev(dev);
		free_irq(dev->irq, dev);
		iounmap(ag->mac_base);
		kfree(dev);
		platform_set_drvdata(pdev, NULL);
	}

	return 0;
}

static struct platform_driver ag71xx_driver = {
	.probe		= ag71xx_probe,
	.remove		= ag71xx_remove,
	.driver = {
		.name	= AG71XX_DRV_NAME,
	}
};

static int __init ag71xx_module_init(void)
{
	int ret;

	ret = ag71xx_debugfs_root_init();
	if (ret)
		goto err_out;

	ret = ag71xx_mdio_driver_init();
	if (ret)
		goto err_debugfs_exit;

	ret = platform_driver_register(&ag71xx_driver);
	if (ret)
		goto err_mdio_exit;

	return 0;

err_mdio_exit:
	ag71xx_mdio_driver_exit();
err_debugfs_exit:
	ag71xx_debugfs_root_exit();
err_out:
	return ret;
}

static void __exit ag71xx_module_exit(void)
{
	platform_driver_unregister(&ag71xx_driver);
	ag71xx_mdio_driver_exit();
	ag71xx_debugfs_root_exit();
}

module_init(ag71xx_module_init);
module_exit(ag71xx_module_exit);

MODULE_VERSION(AG71XX_DRV_VERSION);
MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" AG71XX_DRV_NAME);
