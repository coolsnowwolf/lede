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

#include <linux/sizes.h>
#include <linux/of_net.h>
#include <linux/of_address.h>
#include <linux/of_platform.h>
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
	int ring_mask = BIT(ring->order) - 1;
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
			dma_unmap_single(&ag->pdev->dev, ring->buf[i].dma_addr,
					 ag->rx_buf_size, DMA_FROM_DEVICE);
			skb_free_frag(ring->buf[i].rx_buf);
		}
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
	buf->dma_addr = dma_map_single(&ag->pdev->dev, data, ag->rx_buf_size,
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

		if (!ag71xx_fill_rx_buf(ag, &ring->buf[i], ag->rx_buf_offset,
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
	int offset = ag->rx_buf_offset;

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

	tx->descs_cpu = dma_alloc_coherent(&ag->pdev->dev, ring_size * AG71XX_DESC_SIZE,
					   &tx->descs_dma, GFP_KERNEL);
	if (!tx->descs_cpu) {
		kfree(tx->buf);
		tx->buf = NULL;
		return -ENOMEM;
	}

	rx->buf = &tx->buf[tx_size];
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
		dma_free_coherent(&ag->pdev->dev, ring_size * AG71XX_DESC_SIZE,
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
	struct device_node *np = ag->pdev->dev.of_node;
	u32 init = MAC_CFG1_INIT;

	/* setup MAC configuration registers */
	if (of_property_read_bool(np, "flow-control"))
		init |= MAC_CFG1_TFC | MAC_CFG1_RFC;
	ag71xx_wr(ag, AG71XX_REG_MAC_CFG1, init);

	ag71xx_sb(ag, AG71XX_REG_MAC_CFG2,
		  MAC_CFG2_PAD_CRC_EN | MAC_CFG2_LEN_CHECK);

	/* setup max frame length to zero */
	ag71xx_wr(ag, AG71XX_REG_MAC_MFL, 0);

	/* setup FIFO configuration registers */
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG0, FIFO_CFG0_INIT);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG1, ag->fifodata[0]);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG2, ag->fifodata[1]);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG4, FIFO_CFG4_INIT);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, FIFO_CFG5_INIT);
}

static void ag71xx_hw_init(struct ag71xx *ag)
{
	ag71xx_hw_stop(ag);

	ag71xx_sb(ag, AG71XX_REG_MAC_CFG1, MAC_CFG1_SR);
	udelay(20);

	reset_control_assert(ag->mac_reset);
	if (ag->mdio_reset)
		reset_control_assert(ag->mdio_reset);
	msleep(100);
	reset_control_deassert(ag->mac_reset);
	if (ag->mdio_reset)
		reset_control_deassert(ag->mdio_reset);
	msleep(200);

	ag71xx_hw_setup(ag);

	ag71xx_dma_reset(ag);
}

static void ag71xx_fast_reset(struct ag71xx *ag)
{
	struct net_device *dev = ag->dev;
	u32 rx_ds;
	u32 mii_reg;

	ag71xx_hw_stop(ag);
	wmb();

	mii_reg = ag71xx_rr(ag, AG71XX_REG_MII_CFG);
	rx_ds = ag71xx_rr(ag, AG71XX_REG_RX_DESC);

	ag71xx_tx_packets(ag, true);

	reset_control_assert(ag->mac_reset);
	udelay(10);
	reset_control_deassert(ag->mac_reset);
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

static void ath79_set_pllval(struct ag71xx *ag)
{
	u32 pll_reg = ag->pllreg[1];
	u32 pll_val;

	if (!ag->pllregmap)
		return;

	switch (ag->speed) {
	case SPEED_10:
		pll_val = ag->plldata[2];
		break;
	case SPEED_100:
		pll_val = ag->plldata[1];
		break;
	case SPEED_1000:
		pll_val = ag->plldata[0];
		break;
	default:
		BUG();
	}

	if (pll_val)
		regmap_write(ag->pllregmap, pll_reg, pll_val);
}

static void ath79_set_pll(struct ag71xx *ag)
{
	u32 pll_cfg = ag->pllreg[0];
	u32 pll_shift = ag->pllreg[2];

	if (!ag->pllregmap)
		return;

	regmap_update_bits(ag->pllregmap, pll_cfg, 3 << pll_shift, 2 << pll_shift);
	udelay(100);

	ath79_set_pllval(ag);

	regmap_update_bits(ag->pllregmap, pll_cfg, 3 << pll_shift, 3 << pll_shift);
	udelay(100);

	regmap_update_bits(ag->pllregmap, pll_cfg, 3 << pll_shift, 0);
	udelay(100);
}

static void ag71xx_bit_set(void __iomem *reg, u32 bit)
{
	u32 val;

	val = __raw_readl(reg) | bit;
	__raw_writel(val, reg);
	__raw_readl(reg);
}

static void ag71xx_bit_clear(void __iomem *reg, u32 bit)
{
	u32 val;

	val = __raw_readl(reg) & ~bit;
	__raw_writel(val, reg);
	__raw_readl(reg);
}

static void ag71xx_sgmii_init_qca955x(struct device_node *np)
{
	struct device_node *np_dev;
	void __iomem *gmac_base;
	u32 mr_an_status;
	u32 sgmii_status;
	u8 tries = 0;
	int err = 0;

	np = of_get_child_by_name(np, "gmac-config");
	if (!np)
		return;

	np_dev = of_parse_phandle(np, "device", 0);
	if (!np_dev)
		goto out;

	gmac_base = of_iomap(np_dev, 0);
	if (!gmac_base) {
		pr_err("%pOF: can't map GMAC registers\n", np_dev);
		err = -ENOMEM;
		goto err_iomap;
	}

	mr_an_status = __raw_readl(gmac_base + QCA955X_GMAC_REG_MR_AN_STATUS);
	if (!(mr_an_status & QCA955X_MR_AN_STATUS_AN_ABILITY))
		goto sgmii_out;

	/* SGMII reset sequence */
	__raw_writel(QCA955X_SGMII_RESET_RX_CLK_N_RESET,
		     gmac_base + QCA955X_GMAC_REG_SGMII_RESET);
	__raw_readl(gmac_base + QCA955X_GMAC_REG_SGMII_RESET);
	udelay(10);

	ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_SGMII_RESET,
		       QCA955X_SGMII_RESET_HW_RX_125M_N);
	udelay(10);

	ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_SGMII_RESET,
		       QCA955X_SGMII_RESET_RX_125M_N);
	udelay(10);

	ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_SGMII_RESET,
		       QCA955X_SGMII_RESET_TX_125M_N);
	udelay(10);

	ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_SGMII_RESET,
		       QCA955X_SGMII_RESET_RX_CLK_N);
	udelay(10);

	ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_SGMII_RESET,
		       QCA955X_SGMII_RESET_TX_CLK_N);
	udelay(10);

	/*
	 * The following is what QCA has to say about what happens here:
	 *
	 * Across resets SGMII link status goes to weird state.
	 * If SGMII_DEBUG register reads other than 0x1f or 0x10,
	 * we are for sure in a bad  state.
	 *
	 * Issue a PHY reset in MR_AN_CONTROL to keep going.
	 */
	do {
		ag71xx_bit_set(gmac_base + QCA955X_GMAC_REG_MR_AN_CONTROL,
			       QCA955X_MR_AN_CONTROL_PHY_RESET |
			       QCA955X_MR_AN_CONTROL_AN_ENABLE);
		udelay(200);
		ag71xx_bit_clear(gmac_base + QCA955X_GMAC_REG_MR_AN_CONTROL,
				 QCA955X_MR_AN_CONTROL_PHY_RESET);
		mdelay(300);
		sgmii_status = __raw_readl(gmac_base + QCA955X_GMAC_REG_SGMII_DEBUG) &
					   QCA955X_SGMII_DEBUG_TX_STATE_MASK;

		if (tries++ >= 20) {
			pr_err("ag71xx: max retries for SGMII fixup exceeded\n");
			break;
		}
	} while (!(sgmii_status == 0xf || sgmii_status == 0x10));

sgmii_out:
	iounmap(gmac_base);
err_iomap:
	of_node_put(np_dev);
out:
	of_node_put(np);
}

static void ath79_mii_ctrl_set_if(struct ag71xx *ag, unsigned int mii_if)
{
	u32 t;

	t = __raw_readl(ag->mii_base);
	t &= ~(AR71XX_MII_CTRL_IF_MASK);
	t |= (mii_if & AR71XX_MII_CTRL_IF_MASK);
	__raw_writel(t, ag->mii_base);
}

static void ath79_mii0_ctrl_set_if(struct ag71xx *ag)
{
	unsigned int mii_if;

	switch (ag->phy_if_mode) {
	case PHY_INTERFACE_MODE_MII:
		mii_if = AR71XX_MII0_CTRL_IF_MII;
		break;
	case PHY_INTERFACE_MODE_GMII:
		mii_if = AR71XX_MII0_CTRL_IF_GMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		mii_if = AR71XX_MII0_CTRL_IF_RGMII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		mii_if = AR71XX_MII0_CTRL_IF_RMII;
		break;
	default:
		WARN(1, "Impossible PHY mode defined.\n");
		return;
	}

	ath79_mii_ctrl_set_if(ag, mii_if);
}

static void ath79_mii1_ctrl_set_if(struct ag71xx *ag)
{
	unsigned int mii_if;

	switch (ag->phy_if_mode) {
	case PHY_INTERFACE_MODE_RMII:
		mii_if = AR71XX_MII1_CTRL_IF_RMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
		mii_if = AR71XX_MII1_CTRL_IF_RGMII;
		break;
	default:
		WARN(1, "Impossible PHY mode defined.\n");
		return;
	}

	ath79_mii_ctrl_set_if(ag, mii_if);
}

static void ath79_mii_ctrl_set_speed(struct ag71xx *ag)
{
	unsigned int mii_speed;
	u32 t;

	if (!ag->mii_base)
		return;

	switch (ag->speed) {
	case SPEED_10:
		mii_speed =  AR71XX_MII_CTRL_SPEED_10;
		break;
	case SPEED_100:
		mii_speed =  AR71XX_MII_CTRL_SPEED_100;
		break;
	case SPEED_1000:
		mii_speed =  AR71XX_MII_CTRL_SPEED_1000;
		break;
	default:
		BUG();
	}

	t = __raw_readl(ag->mii_base);
	t &= ~(AR71XX_MII_CTRL_SPEED_MASK << AR71XX_MII_CTRL_SPEED_SHIFT);
	t |= mii_speed << AR71XX_MII_CTRL_SPEED_SHIFT;
	__raw_writel(t, ag->mii_base);
}

static void
__ag71xx_link_adjust(struct ag71xx *ag, bool update)
{
	struct device_node *np = ag->pdev->dev.of_node;
	u32 cfg2;
	u32 ifctl;
	u32 fifo5;

	if (!ag->link && update) {
		ag71xx_hw_stop(ag);
		netif_carrier_off(ag->dev);
		if (netif_msg_link(ag))
			pr_info("%s: link down\n", ag->dev->name);
		return;
	}

	if (!of_device_is_compatible(np, "qca,ar9130-eth") &&
	    !of_device_is_compatible(np, "qca,ar7100-eth"))
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

	if (ag->tx_ring.desc_split) {
		ag->fifodata[2] &= 0xffff;
		ag->fifodata[2] |= ((2048 - ag->tx_ring.desc_split) / 4) << 16;
	}

	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG3, ag->fifodata[2]);

	if (update) {
		if (of_device_is_compatible(np, "qca,ar7100-eth") ||
		    of_device_is_compatible(np, "qca,ar9130-eth")) {
			ath79_set_pll(ag);
			ath79_mii_ctrl_set_speed(ag);
		} else if (of_device_is_compatible(np, "qca,ar7242-eth") ||
			   of_device_is_compatible(np, "qca,ar9340-eth") ||
			   of_device_is_compatible(np, "qca,qca9550-eth") ||
			   of_device_is_compatible(np, "qca,qca9560-eth")) {
			ath79_set_pllval(ag);
			if (of_property_read_bool(np, "qca955x-sgmii-fixup"))
				ag71xx_sgmii_init_qca955x(np);
		}
	}

	ag71xx_wr(ag, AG71XX_REG_MAC_CFG2, cfg2);
	ag71xx_wr(ag, AG71XX_REG_FIFO_CFG5, fifo5);
	ag71xx_wr(ag, AG71XX_REG_MAC_IFCTL, ifctl);

	if (of_device_is_compatible(np, "qca,qca9530-eth") ||
	    of_device_is_compatible(np, "qca,qca9560-eth")) {
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

	ag71xx_dump_regs(ag);
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
	netif_stop_queue(ag->dev);

	ag71xx_hw_stop(ag);
	ag71xx_dma_reset(ag);

	napi_disable(&ag->napi);
	del_timer_sync(&ag->oom_timer);

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

	phy_start(ag->phy_dev);

	return 0;

err:
	ag71xx_rings_cleanup(ag);
	return ret;
}

static int ag71xx_stop(struct net_device *dev)
{
	unsigned long flags;
	struct ag71xx *ag = netdev_priv(dev);

	netif_carrier_off(dev);
	phy_stop(ag->phy_dev);

	spin_lock_irqsave(&ag->lock, flags);
	if (ag->link) {
		ag->link = 0;
		ag71xx_link_adjust(ag);
	}
	spin_unlock_irqrestore(&ag->lock, flags);

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

	if (skb->len <= 4) {
		DBG("%s: packet len is too small\n", ag->dev->name);
		goto err_drop;
	}

	dma_addr = dma_map_single(&ag->pdev->dev, skb->data, skb->len,
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
	dma_unmap_single(&ag->pdev->dev, dma_addr, skb->len, DMA_TO_DEVICE);

err_drop:
	dev->stats.tx_dropped++;

	dev_kfree_skb(skb);
	return NETDEV_TX_OK;
}

static int ag71xx_do_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct ag71xx *ag = netdev_priv(dev);


	switch (cmd) {
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

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))
static void ag71xx_oom_timer_handler(unsigned long data)
{
	struct net_device *dev = (struct net_device *) data;
	struct ag71xx *ag = netdev_priv(dev);
#else
static void ag71xx_oom_timer_handler(struct timer_list *t)
{
	struct ag71xx *ag = from_timer(ag, t, oom_timer);
#endif

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
			if (ag->tx_hang_workaround &&
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
	unsigned int pktlen_mask = ag->desc_pktlen_mask;
	unsigned int offset = ag->rx_buf_offset;
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

		dma_unmap_single(&ag->pdev->dev, ring->buf[i].dma_addr,
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
	struct net_device *dev = ag->dev;
	struct ag71xx_ring *rx_ring = &ag->rx_ring;
	int rx_ring_size = BIT(rx_ring->order);
	unsigned long flags;
	u32 status;
	int tx_done;
	int rx_done;

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

static int ag71xx_change_mtu(struct net_device *dev, int new_mtu)
{
	struct ag71xx *ag = netdev_priv(dev);

	dev->mtu = new_mtu;
	ag71xx_wr(ag, AG71XX_REG_MAC_MFL,
		  ag71xx_max_frame_len(dev->mtu));

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
};

static int ag71xx_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct net_device *dev;
	struct resource *res;
	struct ag71xx *ag;
	const void *mac_addr;
	u32 max_frame_len;
	int tx_size, err;

	if (!np)
		return -ENODEV;

	dev = devm_alloc_etherdev(&pdev->dev, sizeof(*ag));
	if (!dev)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
		return -EINVAL;

	err = ag71xx_setup_gmac(np);
	if (err)
		return err;

	SET_NETDEV_DEV(dev, &pdev->dev);

	ag = netdev_priv(dev);
	ag->pdev = pdev;
	ag->dev = dev;
	ag->msg_enable = netif_msg_init(ag71xx_msg_level,
					AG71XX_DEFAULT_MSG_ENABLE);
	spin_lock_init(&ag->lock);

	ag->mac_reset = devm_reset_control_get_exclusive(&pdev->dev, "mac");
	if (IS_ERR(ag->mac_reset)) {
		dev_err(&pdev->dev, "missing mac reset\n");
		return PTR_ERR(ag->mac_reset);
	}

	ag->mdio_reset = devm_reset_control_get_optional_exclusive(&pdev->dev, "mdio");

	if (of_property_read_u32_array(np, "fifo-data", ag->fifodata, 3)) {
		if (of_device_is_compatible(np, "qca,ar9130-eth") ||
		    of_device_is_compatible(np, "qca,ar7100-eth")) {
			ag->fifodata[0] = 0x0fff0000;
			ag->fifodata[1] = 0x00001fff;
		} else {
			ag->fifodata[0] = 0x0010ffff;
			ag->fifodata[1] = 0x015500aa;
			ag->fifodata[2] = 0x01f00140;
		}
		if (of_device_is_compatible(np, "qca,ar9130-eth"))
			ag->fifodata[2] = 0x00780fff;
		else if (of_device_is_compatible(np, "qca,ar7100-eth"))
			ag->fifodata[2] = 0x008001ff;
	}

	if (of_property_read_u32_array(np, "pll-data", ag->plldata, 3))
		dev_dbg(&pdev->dev, "failed to read pll-data property\n");

	if (of_property_read_u32_array(np, "pll-reg", ag->pllreg, 3))
		dev_dbg(&pdev->dev, "failed to read pll-reg property\n");

	ag->pllregmap = syscon_regmap_lookup_by_phandle(np, "pll-handle");
	if (IS_ERR(ag->pllregmap)) {
		dev_dbg(&pdev->dev, "failed to read pll-handle property\n");
		ag->pllregmap = NULL;
	}

	ag->mac_base = devm_ioremap_nocache(&pdev->dev, res->start,
					    res->end - res->start + 1);
	if (!ag->mac_base)
		return -ENOMEM;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 1);
	if (res) {
		ag->mii_base = devm_ioremap_nocache(&pdev->dev, res->start,
					    res->end - res->start + 1);
		if (!ag->mii_base)
			return -ENOMEM;
	}

	dev->irq = platform_get_irq(pdev, 0);
	err = devm_request_irq(&pdev->dev, dev->irq, ag71xx_interrupt,
			       0x0, dev_name(&pdev->dev), dev);
	if (err) {
		dev_err(&pdev->dev, "unable to request IRQ %d\n", dev->irq);
		return err;
	}

	dev->netdev_ops = &ag71xx_netdev_ops;
	dev->ethtool_ops = &ag71xx_ethtool_ops;

	INIT_DELAYED_WORK(&ag->restart_work, ag71xx_restart_work_func);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0))
	init_timer(&ag->oom_timer);
	ag->oom_timer.data = (unsigned long) dev;
	ag->oom_timer.function = ag71xx_oom_timer_handler;
#else
	timer_setup(&ag->oom_timer, ag71xx_oom_timer_handler, 0);
#endif

	tx_size = AG71XX_TX_RING_SIZE_DEFAULT;
	ag->rx_ring.order = ag71xx_ring_size_order(AG71XX_RX_RING_SIZE_DEFAULT);

	if (of_device_is_compatible(np, "qca,ar9340-eth") ||
	    of_device_is_compatible(np, "qca,qca9530-eth") ||
	    of_device_is_compatible(np, "qca,qca9550-eth") ||
	    of_device_is_compatible(np, "qca,qca9560-eth"))
		ag->desc_pktlen_mask = SZ_16K - 1;
	else
		ag->desc_pktlen_mask = SZ_4K - 1;

	if (ag->desc_pktlen_mask == SZ_16K - 1 &&
	    !of_device_is_compatible(np, "qca,qca9550-eth") &&
	    !of_device_is_compatible(np, "qca,qca9560-eth"))
		max_frame_len = ag->desc_pktlen_mask;
	else
		max_frame_len = 1540;

	dev->min_mtu = 68;
	dev->max_mtu = max_frame_len - ag71xx_max_frame_len(0);

	if (of_device_is_compatible(np, "qca,ar7240-eth") ||
	    of_device_is_compatible(np, "qca,ar7241-eth") ||
	    of_device_is_compatible(np, "qca,ar7242-eth") ||
	    of_device_is_compatible(np, "qca,ar9330-eth") ||
	    of_device_is_compatible(np, "qca,ar9340-eth") ||
	    of_device_is_compatible(np, "qca,qca9530-eth") ||
	    of_device_is_compatible(np, "qca,qca9550-eth") ||
	    of_device_is_compatible(np, "qca,qca9560-eth"))
		ag->tx_hang_workaround = 1;

	ag->rx_buf_offset = NET_SKB_PAD;
	if (!of_device_is_compatible(np, "qca,ar7100-eth") &&
	    !of_device_is_compatible(np, "qca,ar9130-eth"))
		ag->rx_buf_offset += NET_IP_ALIGN;

	if (of_device_is_compatible(np, "qca,ar7100-eth")) {
		ag->tx_ring.desc_split = AG71XX_TX_RING_SPLIT;
		tx_size *= AG71XX_TX_RING_DS_PER_PKT;
	}
	ag->tx_ring.order = ag71xx_ring_size_order(tx_size);

	ag->stop_desc = dmam_alloc_coherent(&pdev->dev,
					    sizeof(struct ag71xx_desc),
					    &ag->stop_desc_dma, GFP_KERNEL);
	if (!ag->stop_desc)
		return -ENOMEM;

	ag->stop_desc->data = 0;
	ag->stop_desc->ctrl = 0;
	ag->stop_desc->next = (u32) ag->stop_desc_dma;

	mac_addr = of_get_mac_address(np);
	if (mac_addr)
		memcpy(dev->dev_addr, mac_addr, ETH_ALEN);
	if (!mac_addr || !is_valid_ether_addr(dev->dev_addr)) {
		dev_err(&pdev->dev, "invalid MAC address, using random address\n");
		eth_random_addr(dev->dev_addr);
	}

	ag->phy_if_mode = of_get_phy_mode(np);
	if (ag->phy_if_mode < 0) {
		dev_err(&pdev->dev, "missing phy-mode property in DT\n");
		return ag->phy_if_mode;
	}

	if (of_property_read_u32(np, "qca,mac-idx", &ag->mac_idx))
		ag->mac_idx = -1;
	if (ag->mii_base)
		switch (ag->mac_idx) {
		case 0:
			ath79_mii0_ctrl_set_if(ag);
			break;
		case 1:
			ath79_mii1_ctrl_set_if(ag);
			break;
		default:
			break;
		}

	netif_napi_add(dev, &ag->napi, ag71xx_poll, AG71XX_NAPI_WEIGHT);

	ag71xx_dump_regs(ag);

	ag71xx_wr(ag, AG71XX_REG_MAC_CFG1, 0);

	ag71xx_hw_init(ag);

	ag71xx_dump_regs(ag);

	/*
	 * populate current node to register mdio-bus as a subdevice.
	 * the mdio bus works independently on ar7241 and later chips
	 * and we need to load mdio1 before gmac0, which can be done
	 * by adding a "simple-mfd" compatible to gmac node. The
	 * following code checks OF_POPULATED_BUS flag before populating
	 * to avoid duplicated population.
	 */
	if (!of_node_check_flag(np, OF_POPULATED_BUS)) {
		err = of_platform_populate(np, NULL, NULL, &pdev->dev);
		if (err)
			return err;
	}

	err = ag71xx_phy_connect(ag);
	if (err)
		return err;

	err = ag71xx_debugfs_init(ag);
	if (err)
		goto err_phy_disconnect;

	platform_set_drvdata(pdev, dev);

	err = register_netdev(dev);
	if (err) {
		dev_err(&pdev->dev, "unable to register net device\n");
		platform_set_drvdata(pdev, NULL);
		ag71xx_debugfs_exit(ag);
		goto err_phy_disconnect;
	}

	pr_info("%s: Atheros AG71xx at 0x%08lx, irq %d, mode: %s\n",
		dev->name, (unsigned long) ag->mac_base, dev->irq,
		phy_modes(ag->phy_if_mode));

	return 0;

err_phy_disconnect:
	ag71xx_phy_disconnect(ag);
	return err;
}

static int ag71xx_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct ag71xx *ag;

	if (!dev)
		return 0;

	ag = netdev_priv(dev);
	ag71xx_debugfs_exit(ag);
	ag71xx_phy_disconnect(ag);
	unregister_netdev(dev);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static const struct of_device_id ag71xx_match[] = {
	{ .compatible = "qca,ar7100-eth" },
	{ .compatible = "qca,ar7240-eth" },
	{ .compatible = "qca,ar7241-eth" },
	{ .compatible = "qca,ar7242-eth" },
	{ .compatible = "qca,ar9130-eth" },
	{ .compatible = "qca,ar9330-eth" },
	{ .compatible = "qca,ar9340-eth" },
	{ .compatible = "qca,qca9530-eth" },
	{ .compatible = "qca,qca9550-eth" },
	{ .compatible = "qca,qca9560-eth" },
	{}
};

static struct platform_driver ag71xx_driver = {
	.probe		= ag71xx_probe,
	.remove		= ag71xx_remove,
	.driver = {
		.name	= AG71XX_DRV_NAME,
		.of_match_table = ag71xx_match,
	}
};

static int __init ag71xx_module_init(void)
{
	int ret;

	ret = ag71xx_debugfs_root_init();
	if (ret)
		goto err_out;

	ret = platform_driver_register(&ag71xx_driver);
	if (ret)
		goto err_debugfs_exit;

	return 0;

err_debugfs_exit:
	ag71xx_debugfs_root_exit();
err_out:
	return ret;
}

static void __exit ag71xx_module_exit(void)
{
	platform_driver_unregister(&ag71xx_driver);
	ag71xx_debugfs_root_exit();
}

module_init(ag71xx_module_init);
module_exit(ag71xx_module_exit);

MODULE_AUTHOR("Gabor Juhos <juhosg@openwrt.org>");
MODULE_AUTHOR("Imre Kaloz <kaloz@openwrt.org>");
MODULE_AUTHOR("Felix Fietkau <nbd@nbd.name>");
MODULE_LICENSE("GPL v2");
MODULE_ALIAS("platform:" AG71XX_DRV_NAME);
