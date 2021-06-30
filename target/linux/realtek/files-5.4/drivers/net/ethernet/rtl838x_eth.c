// SPDX-License-Identifier: GPL-2.0-only
/*
 * linux/drivers/net/ethernet/rtl838x_eth.c
 * Copyright (C) 2020 B. Koblitz
 */

#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/module.h>
#include <linux/phylink.h>
#include <linux/pkt_sched.h>
#include <net/dsa.h>
#include <net/switchdev.h>
#include <asm/cacheflush.h>

#include <asm/mach-rtl838x/mach-rtl83xx.h>
#include "rtl838x_eth.h"

extern struct rtl83xx_soc_info soc_info;

/*
 * Maximum number of RX rings is 8 on RTL83XX and 32 on the 93XX
 * The ring is assigned by switch based on packet/port priortity
 * Maximum number of TX rings is 2, Ring 2 being the high priority
 * ring on the RTL93xx SoCs. MAX_RING_SIZE * RING_BUFFER gives
 * the memory used for the ring buffer.
 */
#define MAX_RXRINGS	32
#define MAX_RXLEN	100
#define MAX_ENTRIES	(200 * 8)
#define TXRINGS		2
// BUG: TXRINGLEN can be 160
#define TXRINGLEN	16
#define NOTIFY_EVENTS	10
#define NOTIFY_BLOCKS	10
#define TX_EN		0x8
#define RX_EN		0x4
#define TX_EN_93XX	0x20
#define RX_EN_93XX	0x10
#define TX_DO		0x2
#define WRAP		0x2

#define RING_BUFFER	1600

#define RTL838X_STORM_CTRL_PORT_BC_EXCEED	(0x470C)
#define RTL838X_STORM_CTRL_PORT_MC_EXCEED	(0x4710)
#define RTL838X_STORM_CTRL_PORT_UC_EXCEED	(0x4714)
#define RTL838X_ATK_PRVNT_STS			(0x5B1C)

struct p_hdr {
	uint8_t		*buf;
	uint16_t	reserved;
	uint16_t	size;		/* buffer size */
	uint16_t	offset;
	uint16_t	len;		/* pkt len */
	uint16_t	cpu_tag[10];
} __packed __aligned(1);

struct n_event {
	uint32_t	type:2;
	uint32_t	fidVid:12;
	uint64_t	mac:48;
	uint32_t	slp:6;
	uint32_t	valid:1;
	uint32_t	reserved:27;
} __packed __aligned(1);

struct ring_b {
	uint32_t	rx_r[MAX_RXRINGS][MAX_RXLEN];
	uint32_t	tx_r[TXRINGS][TXRINGLEN];
	struct	p_hdr	rx_header[MAX_RXRINGS][MAX_RXLEN];
	struct	p_hdr	tx_header[TXRINGS][TXRINGLEN];
	uint32_t	c_rx[MAX_RXRINGS];
	uint32_t	c_tx[TXRINGS];
	uint8_t		tx_space[TXRINGS * TXRINGLEN * RING_BUFFER];
	uint8_t		*rx_space;
};

struct notify_block {
	struct n_event	events[NOTIFY_EVENTS];
};

struct notify_b {
	struct notify_block	blocks[NOTIFY_BLOCKS];
	u32			reserved1[8];
	u32			ring[NOTIFY_BLOCKS];
	u32			reserved2[8];
};

void rtl838x_create_tx_header(struct p_hdr *h, int dest_port, int prio)
{
	prio &= 0x7;

	if (dest_port > 0) {
		// cpu_tag[0] is reserved on the RTL83XX SoCs
		h->cpu_tag[1] = 0x0400;
		h->cpu_tag[2] = 0x0200;
		h->cpu_tag[3] = 0x0000;
		h->cpu_tag[4] = BIT(dest_port) >> 16;
		h->cpu_tag[5] = BIT(dest_port) & 0xffff;
		// Set internal priority and AS_PRIO
		if (prio >= 0)
			h->cpu_tag[2] |= (prio | 0x8) << 12;
	}
}

void rtl839x_create_tx_header(struct p_hdr *h, int dest_port, int prio)
{
	prio &= 0x7;

	if (dest_port > 0) {
		// cpu_tag[0] is reserved on the RTL83XX SoCs
		h->cpu_tag[1] = 0x0100;
		h->cpu_tag[2] = h->cpu_tag[3] = h->cpu_tag[4] = h->cpu_tag[5] = 0;
		if (dest_port >= 32) {
			dest_port -= 32;
			h->cpu_tag[2] = BIT(dest_port) >> 16;
			h->cpu_tag[3] = BIT(dest_port) & 0xffff;
		} else {
			h->cpu_tag[4] = BIT(dest_port) >> 16;
			h->cpu_tag[5] = BIT(dest_port) & 0xffff;
		}
		h->cpu_tag[6] |= BIT(21); // Enable destination port mask use
		// Set internal priority and AS_PRIO
		if (prio >= 0)
			h->cpu_tag[1] |= prio | BIT(3);
	}
}

void rtl930x_create_tx_header(struct p_hdr *h, int dest_port, int prio)
{
	h->cpu_tag[0] = 0x8000;
	h->cpu_tag[1] = 0;  // TODO: Fill port and prio
	h->cpu_tag[2] = 0;
	h->cpu_tag[3] = 0;
	h->cpu_tag[4] = 0;
	h->cpu_tag[5] = 0;
	h->cpu_tag[6] = 0;
	h->cpu_tag[7] = 0xffff;
}

void rtl931x_create_tx_header(struct p_hdr *h, int dest_port, int prio)
{
	h->cpu_tag[0] = 0x8000;
	h->cpu_tag[1] = 0;  // TODO: Fill port and prio
	h->cpu_tag[2] = 0;
	h->cpu_tag[3] = 0;
	h->cpu_tag[4] = 0;
	h->cpu_tag[5] = 0;
	h->cpu_tag[6] = 0;
	h->cpu_tag[7] = 0xffff;
}

struct rtl838x_rx_q {
	int id;
	struct rtl838x_eth_priv *priv;
	struct napi_struct napi;
};

struct rtl838x_eth_priv {
	struct net_device *netdev;
	struct platform_device *pdev;
	void		*membase;
	spinlock_t	lock;
	struct mii_bus	*mii_bus;
	struct rtl838x_rx_q rx_qs[MAX_RXRINGS];
	struct phylink *phylink;
	struct phylink_config phylink_config;
	u16 id;
	u16 family_id;
	const struct rtl838x_reg *r;
	u8 cpu_port;
	u32 lastEvent;
	u16 rxrings;
	u16 rxringlen;
};

extern int rtl838x_phy_init(struct rtl838x_eth_priv *priv);
extern int rtl838x_read_sds_phy(int phy_addr, int phy_reg);
extern int rtl839x_read_sds_phy(int phy_addr, int phy_reg);
extern int rtl839x_write_sds_phy(int phy_addr, int phy_reg, u16 v);
extern int rtl930x_read_sds_phy(int phy_addr, int page, int phy_reg);
extern int rtl930x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v);
extern int rtl930x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
extern int rtl930x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);

/*
 * On the RTL93XX, the RTL93XX_DMA_IF_RX_RING_CNTR track the fill level of 
 * the rings. Writing x into these registers substracts x from its content.
 * When the content reaches the ring size, the ASIC no longer adds
 * packets to this receive queue.
 */
void rtl838x_update_cntr(int r, int released)
{
	// This feature is not available on RTL838x SoCs
}

void rtl839x_update_cntr(int r, int released)
{
	// This feature is not available on RTL839x SoCs
}

void rtl930x_update_cntr(int r, int released)
{
	int pos = (r % 3) * 10;
	u32 reg = RTL930X_DMA_IF_RX_RING_CNTR + ((r / 3) << 2);
	u32 v = sw_r32(reg);

	v = (v >> pos) & 0x3ff;
	pr_debug("RX: Work done %d, old value: %d, pos %d, reg %04x\n", released, v, pos, reg);
	sw_w32_mask(0x3ff << pos, released << pos, reg);
	sw_w32(v, reg);
}

void rtl931x_update_cntr(int r, int released)
{
	int pos = (r % 3) * 10;
	u32 reg = RTL931X_DMA_IF_RX_RING_CNTR + ((r / 3) << 2);

	sw_w32_mask(0x3ff << pos, released << pos, reg);
}

struct dsa_tag {
	u8	reason;
	u8	queue;
	u16	port;
	u8	l2_offloaded;
	u8	prio;
	bool	crc_error;
};

bool rtl838x_decode_tag(struct p_hdr *h, struct dsa_tag *t)
{
	t->reason = h->cpu_tag[3] & 0xf;
	t->queue = (h->cpu_tag[0] & 0xe0) >> 5;
	t->port = h->cpu_tag[1] & 0x1f;
	t->crc_error = t->reason == 13;

	pr_debug("Reason: %d\n", t->reason);
	if (t->reason != 4) // NIC_RX_REASON_SPECIAL_TRAP
		t->l2_offloaded = 1;
	else
		t->l2_offloaded = 0;

	return t->l2_offloaded;
}

bool rtl839x_decode_tag(struct p_hdr *h, struct dsa_tag *t)
{
	t->reason = h->cpu_tag[4] & 0x1f;
	t->queue = (h->cpu_tag[3] & 0xe000) >> 13;
	t->port = h->cpu_tag[1] & 0x3f;
	t->crc_error = h->cpu_tag[3] & BIT(2);

	pr_debug("Reason: %d\n", t->reason);
	if ((t->reason != 7) && (t->reason != 8)) // NIC_RX_REASON_RMA_USR
		t->l2_offloaded = 1;
	else
		t->l2_offloaded = 0;

	return t->l2_offloaded;
}

bool rtl930x_decode_tag(struct p_hdr *h, struct dsa_tag *t)
{
	t->reason = h->cpu_tag[7] & 0x3f;
	t->queue =  (h->cpu_tag[2] >> 11) & 0x1f;
	t->port = (h->cpu_tag[0] >> 8) & 0x1f;
	t->crc_error = h->cpu_tag[1] & BIT(6);

	pr_debug("Reason %d, port %d, queue %d\n", t->reason, t->port, t->queue);
	if (t->reason >= 19 && t->reason <= 27)
		t->l2_offloaded = 0;
	else
		t->l2_offloaded = 1;

	return t->l2_offloaded;
}

bool rtl931x_decode_tag(struct p_hdr *h, struct dsa_tag *t)
{
	t->reason = h->cpu_tag[7] & 0x3f;
	t->queue =  (h->cpu_tag[2] >> 11) & 0x1f;
	t->port = (h->cpu_tag[0] >> 8) & 0x3f;
	t->crc_error = h->cpu_tag[1] & BIT(6);

	pr_debug("Reason %d, port %d, queue %d\n", t->reason, t->port, t->queue);
	if (t->reason >= 19 && t->reason <= 27)
		t->l2_offloaded = 0;
	else
		t->l2_offloaded = 1;

	return t->l2_offloaded;
}

/*
 * Discard the RX ring-buffers, called as part of the net-ISR
 * when the buffer runs over
 * Caller needs to hold priv->lock
 */
static void rtl838x_rb_cleanup(struct rtl838x_eth_priv *priv, int status)
{
	int r;
	u32	*last;
	struct p_hdr *h;
	struct ring_b *ring = priv->membase;

	for (r = 0; r < priv->rxrings; r++) {
		pr_debug("In %s working on r: %d\n", __func__, r);
		last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));
		do {
			if ((ring->rx_r[r][ring->c_rx[r]] & 0x1))
				break;
			pr_debug("Got something: %d\n", ring->c_rx[r]);
			h = &ring->rx_header[r][ring->c_rx[r]];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->rx_space
					+ r * priv->rxringlen * RING_BUFFER
					+ ring->c_rx[r] * RING_BUFFER);
			h->size = RING_BUFFER;
			/* make sure the header is visible to the ASIC */
			mb();

			ring->rx_r[r][ring->c_rx[r]] = KSEG1ADDR(h) | 0x1
				| (ring->c_rx[r] == (priv->rxringlen - 1) ? WRAP : 0x1);
			ring->c_rx[r] = (ring->c_rx[r] + 1) % priv->rxringlen;
		} while (&ring->rx_r[r][ring->c_rx[r]] != last);
	}
}

struct fdb_update_work {
	struct work_struct work;
	struct net_device *ndev;
	u64 macs[NOTIFY_EVENTS + 1];
};

void rtl838x_fdb_sync(struct work_struct *work)
{
	const struct fdb_update_work *uw =
		container_of(work, struct fdb_update_work, work);
	struct switchdev_notifier_fdb_info info;
	u8 addr[ETH_ALEN];
	int i = 0;
	int action;

	while (uw->macs[i]) {
		action = (uw->macs[i] & (1ULL << 63)) ? SWITCHDEV_FDB_ADD_TO_BRIDGE
				: SWITCHDEV_FDB_DEL_TO_BRIDGE;
		u64_to_ether_addr(uw->macs[i] & 0xffffffffffffULL, addr);
		info.addr = &addr[0];
		info.vid = 0;
		info.offloaded = 1;
		pr_debug("FDB entry %d: %llx, action %d\n", i, uw->macs[0], action);
		call_switchdev_notifiers(action, uw->ndev, &info.info, NULL);
		i++;
	}
	kfree(work);
}

static void rtl839x_l2_notification_handler(struct rtl838x_eth_priv *priv)
{
	struct notify_b *nb = priv->membase + sizeof(struct ring_b);
	u32 e = priv->lastEvent;
	struct n_event *event;
	int i;
	u64 mac;
	struct fdb_update_work *w;

	while (!(nb->ring[e] & 1)) {
		w = kzalloc(sizeof(*w), GFP_ATOMIC);
		if (!w) {
			pr_err("Out of memory: %s", __func__);
			return;
		}
		INIT_WORK(&w->work, rtl838x_fdb_sync);

		for (i = 0; i < NOTIFY_EVENTS; i++) {
			event = &nb->blocks[e].events[i];
			if (!event->valid)
				continue;
			mac = event->mac;
			if (event->type)
				mac |= 1ULL << 63;
			w->ndev = priv->netdev;
			w->macs[i] = mac;
		}

		/* Hand the ring entry back to the switch */
		nb->ring[e] = nb->ring[e] | 1;
		e = (e + 1) % NOTIFY_BLOCKS;

		w->macs[i] = 0ULL;
		schedule_work(&w->work);
	}
	priv->lastEvent = e;
}

static irqreturn_t rtl83xx_net_irq(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	u32 status = sw_r32(priv->r->dma_if_intr_sts);
	bool triggered = false;
	u32 atk = sw_r32(RTL838X_ATK_PRVNT_STS);
	int i;
	u32 storm_uc = sw_r32(RTL838X_STORM_CTRL_PORT_UC_EXCEED);
	u32 storm_mc = sw_r32(RTL838X_STORM_CTRL_PORT_MC_EXCEED);
	u32 storm_bc = sw_r32(RTL838X_STORM_CTRL_PORT_BC_EXCEED);

	pr_debug("IRQ: %08x\n", status);
	if (storm_uc || storm_mc || storm_bc) {
		pr_warn("Storm control UC: %08x, MC: %08x, BC: %08x\n",
			storm_uc, storm_mc, storm_bc);

		sw_w32(storm_uc, RTL838X_STORM_CTRL_PORT_UC_EXCEED);
		sw_w32(storm_mc, RTL838X_STORM_CTRL_PORT_MC_EXCEED);
		sw_w32(storm_bc, RTL838X_STORM_CTRL_PORT_BC_EXCEED);

		triggered = true;
	}

	if (atk) {
		pr_debug("Attack prevention triggered: %08x\n", atk);
		sw_w32(atk, RTL838X_ATK_PRVNT_STS);
	}

	spin_lock(&priv->lock);
	/*  Ignore TX interrupt */
	if ((status & 0xf0000)) {
		/* Clear ISR */
		sw_w32(0x000f0000, priv->r->dma_if_intr_sts);
	}

	/* RX interrupt */
	if (status & 0x0ff00) {
		/* ACK and disable RX interrupt for this ring */
		sw_w32_mask(0xff00 & status, 0, priv->r->dma_if_intr_msk);
		sw_w32(0x0000ff00 & status, priv->r->dma_if_intr_sts);
		for (i = 0; i < priv->rxrings; i++) {
			if (status & BIT(i + 8)) {
				pr_debug("Scheduling queue: %d\n", i);
				napi_schedule(&priv->rx_qs[i].napi);
			}
		}
	}

	/* RX buffer overrun */
	if (status & 0x000ff) {
		pr_info("RX buffer overrun: status %x, mask: %x\n",
			 status, sw_r32(priv->r->dma_if_intr_msk));
		sw_w32(status, priv->r->dma_if_intr_sts);
		rtl838x_rb_cleanup(priv, status & 0xff);
	}

	if (priv->family_id == RTL8390_FAMILY_ID && status & 0x00100000) {
		sw_w32(0x00100000, priv->r->dma_if_intr_sts);
		rtl839x_l2_notification_handler(priv);
	}

	if (priv->family_id == RTL8390_FAMILY_ID && status & 0x00200000) {
		sw_w32(0x00200000, priv->r->dma_if_intr_sts);
		rtl839x_l2_notification_handler(priv);
	}

	if (priv->family_id == RTL8390_FAMILY_ID && status & 0x00400000) {
		sw_w32(0x00400000, priv->r->dma_if_intr_sts);
		rtl839x_l2_notification_handler(priv);
	}

	spin_unlock(&priv->lock);
	return IRQ_HANDLED;
}

static irqreturn_t rtl93xx_net_irq(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	u32 status_rx_r = sw_r32(priv->r->dma_if_intr_rx_runout_sts);
	u32 status_rx = sw_r32(priv->r->dma_if_intr_rx_done_sts);
	u32 status_tx = sw_r32(priv->r->dma_if_intr_tx_done_sts);
	int i;

	pr_debug("In %s, status_tx: %08x, status_rx: %08x, status_rx_r: %08x\n",
		__func__, status_tx, status_rx, status_rx_r);
	spin_lock(&priv->lock);

	/*  Ignore TX interrupt */
	if (status_tx) {
		/* Clear ISR */
		pr_debug("TX done\n");
		sw_w32(status_tx, priv->r->dma_if_intr_tx_done_sts);
	}

	/* RX interrupt */
	if (status_rx) {
		pr_debug("RX IRQ\n");
		/* ACK and disable RX interrupt for given rings */
		sw_w32(status_rx, priv->r->dma_if_intr_rx_done_sts);
		sw_w32_mask(status_rx, 0, priv->r->dma_if_intr_rx_done_msk);
		for (i = 0; i < priv->rxrings; i++) {
			if (status_rx & BIT(i)) {
				pr_debug("Scheduling queue: %d\n", i);
				napi_schedule(&priv->rx_qs[i].napi);
			}
		}
	}

	/* RX buffer overrun */
	if (status_rx_r) {
		pr_debug("RX buffer overrun: status %x, mask: %x\n",
			 status_rx_r, sw_r32(priv->r->dma_if_intr_rx_runout_msk));
		sw_w32(status_rx_r, priv->r->dma_if_intr_rx_runout_sts);
		rtl838x_rb_cleanup(priv, status_rx_r);
	}

	spin_unlock(&priv->lock);
	return IRQ_HANDLED;
}

static const struct rtl838x_reg rtl838x_reg = {
	.net_irq = rtl83xx_net_irq,
	.mac_port_ctrl = rtl838x_mac_port_ctrl,
	.dma_if_intr_sts = RTL838X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTL838X_DMA_IF_INTR_MSK,
	.dma_if_ctrl = RTL838X_DMA_IF_CTRL,
	.mac_force_mode_ctrl = RTL838X_MAC_FORCE_MODE_CTRL,
	.dma_rx_base = RTL838X_DMA_RX_BASE,
	.dma_tx_base = RTL838X_DMA_TX_BASE,
	.dma_if_rx_ring_size = rtl838x_dma_if_rx_ring_size,
	.dma_if_rx_ring_cntr = rtl838x_dma_if_rx_ring_cntr,
	.dma_if_rx_cur = RTL838X_DMA_IF_RX_CUR,
	.rst_glb_ctrl = RTL838X_RST_GLB_CTRL_0,
	.get_mac_link_sts = rtl838x_get_mac_link_sts,
	.get_mac_link_dup_sts = rtl838x_get_mac_link_dup_sts,
	.get_mac_link_spd_sts = rtl838x_get_mac_link_spd_sts,
	.get_mac_rx_pause_sts = rtl838x_get_mac_rx_pause_sts,
	.get_mac_tx_pause_sts = rtl838x_get_mac_tx_pause_sts,
	.mac = RTL838X_MAC,
	.l2_tbl_flush_ctrl = RTL838X_L2_TBL_FLUSH_CTRL,
	.update_cntr = rtl838x_update_cntr,
	.create_tx_header = rtl838x_create_tx_header,
	.decode_tag = rtl838x_decode_tag,
};

static const struct rtl838x_reg rtl839x_reg = {
	.net_irq = rtl83xx_net_irq,
	.mac_port_ctrl = rtl839x_mac_port_ctrl,
	.dma_if_intr_sts = RTL839X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTL839X_DMA_IF_INTR_MSK,
	.dma_if_ctrl = RTL839X_DMA_IF_CTRL,
	.mac_force_mode_ctrl = RTL839X_MAC_FORCE_MODE_CTRL,
	.dma_rx_base = RTL839X_DMA_RX_BASE,
	.dma_tx_base = RTL839X_DMA_TX_BASE,
	.dma_if_rx_ring_size = rtl839x_dma_if_rx_ring_size,
	.dma_if_rx_ring_cntr = rtl839x_dma_if_rx_ring_cntr,
	.dma_if_rx_cur = RTL839X_DMA_IF_RX_CUR,
	.rst_glb_ctrl = RTL839X_RST_GLB_CTRL,
	.get_mac_link_sts = rtl839x_get_mac_link_sts,
	.get_mac_link_dup_sts = rtl839x_get_mac_link_dup_sts,
	.get_mac_link_spd_sts = rtl839x_get_mac_link_spd_sts,
	.get_mac_rx_pause_sts = rtl839x_get_mac_rx_pause_sts,
	.get_mac_tx_pause_sts = rtl839x_get_mac_tx_pause_sts,
	.mac = RTL839X_MAC,
	.l2_tbl_flush_ctrl = RTL839X_L2_TBL_FLUSH_CTRL,
	.update_cntr = rtl839x_update_cntr,
	.create_tx_header = rtl839x_create_tx_header,
	.decode_tag = rtl839x_decode_tag,
};

static const struct rtl838x_reg rtl930x_reg = {
	.net_irq = rtl93xx_net_irq,
	.mac_port_ctrl = rtl930x_mac_port_ctrl,
	.dma_if_intr_rx_runout_sts = RTL930X_DMA_IF_INTR_RX_RUNOUT_STS,
	.dma_if_intr_rx_done_sts = RTL930X_DMA_IF_INTR_RX_DONE_STS,
	.dma_if_intr_tx_done_sts = RTL930X_DMA_IF_INTR_TX_DONE_STS,
	.dma_if_intr_rx_runout_msk = RTL930X_DMA_IF_INTR_RX_RUNOUT_MSK,
	.dma_if_intr_rx_done_msk = RTL930X_DMA_IF_INTR_RX_DONE_MSK,
	.dma_if_intr_tx_done_msk = RTL930X_DMA_IF_INTR_TX_DONE_MSK,
	.l2_ntfy_if_intr_sts = RTL930X_L2_NTFY_IF_INTR_STS,
	.l2_ntfy_if_intr_msk = RTL930X_L2_NTFY_IF_INTR_MSK,
	.dma_if_ctrl = RTL930X_DMA_IF_CTRL,
	.mac_force_mode_ctrl = RTL930X_MAC_FORCE_MODE_CTRL,
	.dma_rx_base = RTL930X_DMA_RX_BASE,
	.dma_tx_base = RTL930X_DMA_TX_BASE,
	.dma_if_rx_ring_size = rtl930x_dma_if_rx_ring_size,
	.dma_if_rx_ring_cntr = rtl930x_dma_if_rx_ring_cntr,
	.dma_if_rx_cur = RTL930X_DMA_IF_RX_CUR,
	.rst_glb_ctrl = RTL930X_RST_GLB_CTRL_0,
	.get_mac_link_sts = rtl930x_get_mac_link_sts,
	.get_mac_link_dup_sts = rtl930x_get_mac_link_dup_sts,
	.get_mac_link_spd_sts = rtl930x_get_mac_link_spd_sts,
	.get_mac_rx_pause_sts = rtl930x_get_mac_rx_pause_sts,
	.get_mac_tx_pause_sts = rtl930x_get_mac_tx_pause_sts,
	.mac = RTL930X_MAC_L2_ADDR_CTRL,
	.l2_tbl_flush_ctrl = RTL930X_L2_TBL_FLUSH_CTRL,
	.update_cntr = rtl930x_update_cntr,
	.create_tx_header = rtl930x_create_tx_header,
	.decode_tag = rtl930x_decode_tag,
};

static const struct rtl838x_reg rtl931x_reg = {
	.net_irq = rtl93xx_net_irq,
	.mac_port_ctrl = rtl931x_mac_port_ctrl,
	.dma_if_intr_rx_runout_sts = RTL931X_DMA_IF_INTR_RX_RUNOUT_STS,
	.dma_if_intr_rx_done_sts = RTL931X_DMA_IF_INTR_RX_DONE_STS,
	.dma_if_intr_tx_done_sts = RTL931X_DMA_IF_INTR_TX_DONE_STS,
	.dma_if_intr_rx_runout_msk = RTL931X_DMA_IF_INTR_RX_RUNOUT_MSK,
	.dma_if_intr_rx_done_msk = RTL931X_DMA_IF_INTR_RX_DONE_MSK,
	.dma_if_intr_tx_done_msk = RTL931X_DMA_IF_INTR_TX_DONE_MSK,
	.l2_ntfy_if_intr_sts = RTL931X_L2_NTFY_IF_INTR_STS,
	.l2_ntfy_if_intr_msk = RTL931X_L2_NTFY_IF_INTR_MSK,
	.dma_if_ctrl = RTL931X_DMA_IF_CTRL,
	.mac_force_mode_ctrl = RTL931X_MAC_FORCE_MODE_CTRL,
	.dma_rx_base = RTL931X_DMA_RX_BASE,
	.dma_tx_base = RTL931X_DMA_TX_BASE,
	.dma_if_rx_ring_size = rtl931x_dma_if_rx_ring_size,
	.dma_if_rx_ring_cntr = rtl931x_dma_if_rx_ring_cntr,
	.dma_if_rx_cur = RTL931X_DMA_IF_RX_CUR,
	.rst_glb_ctrl = RTL931X_RST_GLB_CTRL,
	.get_mac_link_sts = rtl931x_get_mac_link_sts,
	.get_mac_link_dup_sts = rtl931x_get_mac_link_dup_sts,
	.get_mac_link_spd_sts = rtl931x_get_mac_link_spd_sts,
	.get_mac_rx_pause_sts = rtl931x_get_mac_rx_pause_sts,
	.get_mac_tx_pause_sts = rtl931x_get_mac_tx_pause_sts,
	.mac = RTL931X_MAC_L2_ADDR_CTRL,
	.l2_tbl_flush_ctrl = RTL931X_L2_TBL_FLUSH_CTRL,
	.update_cntr = rtl931x_update_cntr,
	.create_tx_header = rtl931x_create_tx_header,
	.decode_tag = rtl931x_decode_tag,
};

static void rtl838x_hw_reset(struct rtl838x_eth_priv *priv)
{
	u32 int_saved, nbuf;
	int i, pos;
	
	pr_info("RESETTING %x, CPU_PORT %d\n", priv->family_id, priv->cpu_port);
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(priv->cpu_port));
	mdelay(100);

	/* Disable and clear interrupts */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID) {
		sw_w32(0x00000000, priv->r->dma_if_intr_rx_runout_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_rx_runout_sts);
		sw_w32(0x00000000, priv->r->dma_if_intr_rx_done_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_rx_done_sts);
		sw_w32(0x00000000, priv->r->dma_if_intr_tx_done_msk);
		sw_w32(0x0000000f, priv->r->dma_if_intr_tx_done_sts);
	} else {
		sw_w32(0x00000000, priv->r->dma_if_intr_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_sts);
	}

	if (priv->family_id == RTL8390_FAMILY_ID) {
		/* Preserve L2 notification and NBUF settings */
		int_saved = sw_r32(priv->r->dma_if_intr_msk);
		nbuf = sw_r32(RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL);

		/* Disable link change interrupt on RTL839x */
		sw_w32(0, RTL839X_IMR_PORT_LINK_STS_CHG);
		sw_w32(0, RTL839X_IMR_PORT_LINK_STS_CHG + 4);

		sw_w32(0x00000000, priv->r->dma_if_intr_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_sts);
	}

	/* Reset NIC  */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID)
		sw_w32(0x4, priv->r->rst_glb_ctrl);
	else
		sw_w32(0x8, priv->r->rst_glb_ctrl);

	do { /* Wait for reset of NIC and Queues done */
		udelay(20);
	} while (sw_r32(priv->r->rst_glb_ctrl) & 0xc);
	mdelay(100);

	/* Setup Head of Line */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32(0, RTL838X_DMA_IF_RX_RING_SIZE);  // Disabled on RTL8380
	if (priv->family_id == RTL8390_FAMILY_ID)
		sw_w32(0xffffffff, RTL839X_DMA_IF_RX_RING_CNTR);
	if (priv->family_id == RTL9300_FAMILY_ID) {
		for (i = 0; i < priv->rxrings; i++) {
			pos = (i % 3) * 10;
			sw_w32_mask(0x3ff << pos, 0, priv->r->dma_if_rx_ring_size(i));
			sw_w32_mask(0x3ff << pos, priv->rxringlen,
				    priv->r->dma_if_rx_ring_cntr(i));
		}
	}

	/* Re-enable link change interrupt */
	if (priv->family_id == RTL8390_FAMILY_ID) {
		sw_w32(0xffffffff, RTL839X_ISR_PORT_LINK_STS_CHG);
		sw_w32(0xffffffff, RTL839X_ISR_PORT_LINK_STS_CHG + 4);
		sw_w32(0xffffffff, RTL839X_IMR_PORT_LINK_STS_CHG);
		sw_w32(0xffffffff, RTL839X_IMR_PORT_LINK_STS_CHG + 4);

		/* Restore notification settings: on RTL838x these bits are null */
		sw_w32_mask(7 << 20, int_saved & (7 << 20), priv->r->dma_if_intr_msk);
		sw_w32(nbuf, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL);
	}
}

static void rtl838x_hw_ring_setup(struct rtl838x_eth_priv *priv)
{
	int i;
	struct ring_b *ring = priv->membase;

	for (i = 0; i < priv->rxrings; i++)
		sw_w32(KSEG1ADDR(&ring->rx_r[i]), priv->r->dma_rx_base + i * 4);

	for (i = 0; i < TXRINGS; i++)
		sw_w32(KSEG1ADDR(&ring->tx_r[i]), priv->r->dma_tx_base + i * 4);
}

static void rtl838x_hw_en_rxtx(struct rtl838x_eth_priv *priv)
{
	/* Disable Head of Line features for all RX rings */
	sw_w32(0xffffffff, priv->r->dma_if_rx_ring_size(0));

	/* Truncate RX buffer to 0x640 (1600) bytes, pad TX */
	sw_w32(0x06400020, priv->r->dma_if_ctrl);

	/* Enable RX done, RX overflow and TX done interrupts */
	sw_w32(0xfffff, priv->r->dma_if_intr_msk);

	/* Enable DMA, engine expects empty FCS field */
	sw_w32_mask(0, RX_EN | TX_EN, priv->r->dma_if_ctrl);

	/* Restart TX/RX to CPU port */
	sw_w32_mask(0x0, 0x3, priv->r->mac_port_ctrl(priv->cpu_port));
	/* Set Speed, duplex, flow control
	 * FORCE_EN | LINK_EN | NWAY_EN | DUP_SEL
	 * | SPD_SEL = 0b10 | FORCE_FC_EN | PHY_MASTER_SLV_MANUAL_EN
	 * | MEDIA_SEL
	 */
	sw_w32(0x6192F, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);

	/* Enable CRC checks on CPU-port */
	sw_w32_mask(0, BIT(3), priv->r->mac_port_ctrl(priv->cpu_port));
}

static void rtl839x_hw_en_rxtx(struct rtl838x_eth_priv *priv)
{
	/* Setup CPU-Port: RX Buffer */
	sw_w32(0x0000c808, priv->r->dma_if_ctrl);

	/* Enable Notify, RX done, RX overflow and TX done interrupts */
	sw_w32(0x007fffff, priv->r->dma_if_intr_msk); // Notify IRQ!

	/* Enable DMA */
	sw_w32_mask(0, RX_EN | TX_EN, priv->r->dma_if_ctrl);

	/* Restart TX/RX to CPU port, enable CRC checking */
	sw_w32_mask(0x0, 0x3 | BIT(3), priv->r->mac_port_ctrl(priv->cpu_port));

	/* CPU port joins Lookup Miss Flooding Portmask */
	// TODO: The code below should also work for the RTL838x
	sw_w32(0x28000, RTL839X_TBL_ACCESS_L2_CTRL);
	sw_w32_mask(0, 0x80000000, RTL839X_TBL_ACCESS_L2_DATA(0));
	sw_w32(0x38000, RTL839X_TBL_ACCESS_L2_CTRL);

	/* Force CPU port link up */
	sw_w32_mask(0, 3, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static void rtl93xx_hw_en_rxtx(struct rtl838x_eth_priv *priv)
{
	int i, pos;
	u32 v;

	/* Setup CPU-Port: RX Buffer truncated at 1600 Bytes */
	sw_w32(0x06400040, priv->r->dma_if_ctrl);

	for (i = 0; i < priv->rxrings; i++) {
		pos = (i % 3) * 10;
		sw_w32_mask(0x3ff << pos, priv->rxringlen << pos, priv->r->dma_if_rx_ring_size(i));

		// Some SoCs have issues with missing underflow protection
		v = (sw_r32(priv->r->dma_if_rx_ring_cntr(i)) >> pos) & 0x3ff;
		sw_w32_mask(0x3ff << pos, v, priv->r->dma_if_rx_ring_cntr(i));
	}

	/* Enable Notify, RX done, RX overflow and TX done interrupts */
	sw_w32(0xffffffff, priv->r->dma_if_intr_rx_runout_msk);
	sw_w32(0xffffffff, priv->r->dma_if_intr_rx_done_msk);
	sw_w32(0x0000000f, priv->r->dma_if_intr_tx_done_msk);

	/* Enable DMA */
	sw_w32_mask(0, RX_EN_93XX | TX_EN_93XX, priv->r->dma_if_ctrl);

	/* Restart TX/RX to CPU port, enable CRC checking */
	sw_w32_mask(0x0, 0x3 | BIT(4), priv->r->mac_port_ctrl(priv->cpu_port));

	sw_w32_mask(0, BIT(priv->cpu_port), RTL930X_L2_UNKN_UC_FLD_PMSK);
	sw_w32(0x217, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static void rtl838x_setup_ring_buffer(struct rtl838x_eth_priv *priv, struct ring_b *ring)
{
	int i, j;

	struct p_hdr *h;

	for (i = 0; i < priv->rxrings; i++) {
		for (j = 0; j < priv->rxringlen; j++) {
			h = &ring->rx_header[i][j];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->rx_space
					+ i * priv->rxringlen * RING_BUFFER
					+ j * RING_BUFFER);
			h->size = RING_BUFFER;
			/* All rings owned by switch, last one wraps */
			ring->rx_r[i][j] = KSEG1ADDR(h) | 1 
					   | (j == (priv->rxringlen - 1) ? WRAP : 0);
		}
		ring->c_rx[i] = 0;
	}

	for (i = 0; i < TXRINGS; i++) {
		for (j = 0; j < TXRINGLEN; j++) {
			h = &ring->tx_header[i][j];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->tx_space
					+ i * TXRINGLEN * RING_BUFFER
					+ j * RING_BUFFER);
			h->size = RING_BUFFER;
			ring->tx_r[i][j] = KSEG1ADDR(&ring->tx_header[i][j]);
		}
		/* Last header is wrapping around */
		ring->tx_r[i][j-1] |= WRAP;
		ring->c_tx[i] = 0;
	}
}

static void rtl839x_setup_notify_ring_buffer(struct rtl838x_eth_priv *priv)
{
	int i;
	struct notify_b *b = priv->membase + sizeof(struct ring_b);

	for (i = 0; i < NOTIFY_BLOCKS; i++)
		b->ring[i] = KSEG1ADDR(&b->blocks[i]) | 1 | (i == (NOTIFY_BLOCKS - 1) ? WRAP : 0);

	sw_w32((u32) b->ring, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL);
	sw_w32_mask(0x3ff << 2, 100 << 2, RTL839X_L2_NOTIFICATION_CTRL);

	/* Setup notification events */
	sw_w32_mask(0, 1 << 14, RTL839X_L2_CTRL_0); // RTL8390_L2_CTRL_0_FLUSH_NOTIFY_EN
	sw_w32_mask(0, 1 << 12, RTL839X_L2_NOTIFICATION_CTRL); // SUSPEND_NOTIFICATION_EN

	/* Enable Notification */
	sw_w32_mask(0, 1 << 0, RTL839X_L2_NOTIFICATION_CTRL);
	priv->lastEvent = 0;
}

static int rtl838x_eth_open(struct net_device *ndev)
{
	unsigned long flags;
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);
	struct ring_b *ring = priv->membase;
	int i, err;

	pr_debug("%s called: RX rings %d(length %d), TX rings %d(length %d)\n",
		__func__, priv->rxrings, priv->rxringlen, TXRINGS, TXRINGLEN);

	spin_lock_irqsave(&priv->lock, flags);
	rtl838x_hw_reset(priv);
	rtl838x_setup_ring_buffer(priv, ring);
	if (priv->family_id == RTL8390_FAMILY_ID) {
		rtl839x_setup_notify_ring_buffer(priv);
		/* Make sure the ring structure is visible to the ASIC */
		mb();
		flush_cache_all();
	}

	rtl838x_hw_ring_setup(priv);
	err = request_irq(ndev->irq, priv->r->net_irq, IRQF_SHARED, ndev->name, ndev);
	if (err) {
		netdev_err(ndev, "%s: could not acquire interrupt: %d\n",
			   __func__, err);
		return err;
	}
	phylink_start(priv->phylink);

	for (i = 0; i < priv->rxrings; i++)
		napi_enable(&priv->rx_qs[i].napi);

	switch (priv->family_id) {
	case RTL8380_FAMILY_ID:
		rtl838x_hw_en_rxtx(priv);
		/* Trap IGMP/MLD traffic to CPU-Port */
		sw_w32(0x3, RTL838X_SPCL_TRAP_IGMP_CTRL);
		/* Flush learned FDB entries on link down of a port */
		sw_w32_mask(0, BIT(7), RTL838X_L2_CTRL_0);
		break;

	case RTL8390_FAMILY_ID:
		rtl839x_hw_en_rxtx(priv);
		// Trap MLD and IGMP messages to CPU_PORT
		sw_w32(0x3, RTL839X_SPCL_TRAP_IGMP_CTRL);
		/* Flush learned FDB entries on link down of a port */
		sw_w32_mask(0, BIT(7), RTL839X_L2_CTRL_0);
		break;

	case RTL9300_FAMILY_ID:
		rtl93xx_hw_en_rxtx(priv);
		/* Flush learned FDB entries on link down of a port */
		sw_w32_mask(0, BIT(7), RTL930X_L2_CTRL);
		// Trap MLD and IGMP messages to CPU_PORT
		sw_w32((0x2 << 3) | 0x2,  RTL930X_VLAN_APP_PKT_CTRL);
		break;

	case RTL9310_FAMILY_ID:
		rtl93xx_hw_en_rxtx(priv);
		break;
	}

	netif_tx_start_all_queues(ndev);

	spin_unlock_irqrestore(&priv->lock, flags);

	return 0;
}

static void rtl838x_hw_stop(struct rtl838x_eth_priv *priv)
{
	u32 force_mac = priv->family_id == RTL8380_FAMILY_ID ? 0x6192C : 0x75;
	u32 clear_irq = priv->family_id == RTL8380_FAMILY_ID ? 0x000fffff : 0x007fffff;
	int i;

	// Disable RX/TX from/to CPU-port
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(priv->cpu_port));

	/* Disable traffic */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID)
		sw_w32_mask(RX_EN_93XX | TX_EN_93XX, 0, priv->r->dma_if_ctrl);
	else
		sw_w32_mask(RX_EN | TX_EN, 0, priv->r->dma_if_ctrl);
	mdelay(200); // Test, whether this is needed

	/* Block all ports */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		sw_w32(0x03000000, RTL838X_TBL_ACCESS_DATA_0(0));
		sw_w32(0x00000000, RTL838X_TBL_ACCESS_DATA_0(1));
		sw_w32(1 << 15 | 2 << 12, RTL838X_TBL_ACCESS_CTRL_0);
	}

	/* Flush L2 address cache */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		for (i = 0; i <= priv->cpu_port; i++) {
			sw_w32(1 << 26 | 1 << 23 | i << 5, priv->r->l2_tbl_flush_ctrl);
			do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & (1 << 26));
		}
	} else if (priv->family_id == RTL8390_FAMILY_ID) {
		for (i = 0; i <= priv->cpu_port; i++) {
			sw_w32(1 << 28 | 1 << 25 | i << 5, priv->r->l2_tbl_flush_ctrl);
			do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & (1 << 28));
		}
	}
	// TODO: L2 flush register is 64 bit on RTL931X and 930X

	/* CPU-Port: Link down */
	if (priv->family_id == RTL8380_FAMILY_ID || priv->family_id == RTL8390_FAMILY_ID)
		sw_w32(force_mac, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
	else
		sw_w32_mask(0x3, 0, priv->r->mac_force_mode_ctrl + priv->cpu_port *4);
	mdelay(100);

	/* Disable all TX/RX interrupts */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID) {
		sw_w32(0x00000000, priv->r->dma_if_intr_rx_runout_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_rx_runout_sts);
		sw_w32(0x00000000, priv->r->dma_if_intr_rx_done_msk);
		sw_w32(0xffffffff, priv->r->dma_if_intr_rx_done_sts);
		sw_w32(0x00000000, priv->r->dma_if_intr_tx_done_msk);
		sw_w32(0x0000000f, priv->r->dma_if_intr_tx_done_sts);
	} else {
		sw_w32(0x00000000, priv->r->dma_if_intr_msk);
		sw_w32(clear_irq, priv->r->dma_if_intr_sts);
	}

	/* Disable TX/RX DMA */
	sw_w32(0x00000000, priv->r->dma_if_ctrl);
	mdelay(200);
}

static int rtl838x_eth_stop(struct net_device *ndev)
{
	unsigned long flags;
	int i;
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	pr_info("in %s\n", __func__);

	spin_lock_irqsave(&priv->lock, flags);
	phylink_stop(priv->phylink);
	rtl838x_hw_stop(priv);
	free_irq(ndev->irq, ndev);

	for (i = 0; i < priv->rxrings; i++)
		napi_disable(&priv->rx_qs[i].napi);

	netif_tx_stop_all_queues(ndev);

	spin_unlock_irqrestore(&priv->lock, flags);

	return 0;
}

static void rtl839x_eth_set_multicast_list(struct net_device *ndev)
{
	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL839X_RMA_CTRL_0);
		sw_w32(0x0, RTL839X_RMA_CTRL_1);
		sw_w32(0x0, RTL839X_RMA_CTRL_2);
		sw_w32(0x0, RTL839X_RMA_CTRL_3);
	}
	if (ndev->flags & IFF_ALLMULTI) {
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL839X_RMA_CTRL_2);
		sw_w32(0x3ff, RTL839X_RMA_CTRL_3);
	}
}

static void rtl838x_eth_set_multicast_list(struct net_device *ndev)
{
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	if (priv->family_id == RTL8390_FAMILY_ID)
		return rtl839x_eth_set_multicast_list(ndev);

	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL838X_RMA_CTRL_0);
		sw_w32(0x0, RTL838X_RMA_CTRL_1);
	}
	if (ndev->flags & IFF_ALLMULTI)
		sw_w32(0x1fffff, RTL838X_RMA_CTRL_0);
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(0x1fffff, RTL838X_RMA_CTRL_0);
		sw_w32(0x7fff, RTL838X_RMA_CTRL_1);
	}
}

static void rtl930x_eth_set_multicast_list(struct net_device *ndev)
{
	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL930X_RMA_CTRL_0);
		sw_w32(0x0, RTL930X_RMA_CTRL_1);
		sw_w32(0x0, RTL930X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_ALLMULTI) {
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL930X_RMA_CTRL_2);
	}
}

static void rtl931x_eth_set_multicast_list(struct net_device *ndev)
{
	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL931X_RMA_CTRL_0);
		sw_w32(0x0, RTL931X_RMA_CTRL_1);
		sw_w32(0x0, RTL931X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_ALLMULTI) {
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_0);
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_1);
		sw_w32(0x7fffffff, RTL931X_RMA_CTRL_2);
	}
}

static void rtl838x_eth_tx_timeout(struct net_device *ndev)
{
	unsigned long flags;
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	pr_warn("%s\n", __func__);
	spin_lock_irqsave(&priv->lock, flags);
	rtl838x_hw_stop(priv);
	rtl838x_hw_ring_setup(priv);
	rtl838x_hw_en_rxtx(priv);
	netif_trans_update(ndev);
	netif_start_queue(ndev);
	spin_unlock_irqrestore(&priv->lock, flags);
}

static int rtl838x_eth_tx(struct sk_buff *skb, struct net_device *dev)
{
	int len, i;
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	struct ring_b *ring = priv->membase;
	uint32_t val;
	int ret;
	unsigned long flags;
	struct p_hdr *h;
	int dest_port = -1;
	int q = skb_get_queue_mapping(skb) % TXRINGS;

	if (q) // Check for high prio queue
		pr_debug("SKB priority: %d\n", skb->priority);

	spin_lock_irqsave(&priv->lock, flags);
	len = skb->len;

	/* Check for DSA tagging at the end of the buffer */
	if (netdev_uses_dsa(dev) && skb->data[len-4] == 0x80 && skb->data[len-3] > 0
			&& skb->data[len-3] < 28 &&  skb->data[len-2] == 0x10
			&&  skb->data[len-1] == 0x00) {
		/* Reuse tag space for CRC if possible */
		dest_port = skb->data[len-3];
		len -= 4;
	}

	len += 4; // Add space for CRC

	// On RTL8380 SoCs, the packet needs extra padding
	if (priv->family_id == RTL8380_FAMILY_ID) {
		if (len < ETH_ZLEN)
			len = ETH_ZLEN; // SoC not automatically padding to ETH_ZLEN
		else
			len += 4;
	}

	if (skb_padto(skb, len)) {
		ret = NETDEV_TX_OK;
		goto txdone;
	}

	/* We can send this packet if CPU owns the descriptor */
	if (!(ring->tx_r[q][ring->c_tx[q]] & 0x1)) {

		/* Set descriptor for tx */
		h = &ring->tx_header[q][ring->c_tx[q]];
		h->size = len;
		h->len = len;

		priv->r->create_tx_header(h, dest_port, skb->priority >> 1);

		/* Copy packet data to tx buffer */
		memcpy((void *)KSEG1ADDR(h->buf), skb->data, len);
		/* Make sure packet data is visible to ASIC */
		wmb();

		/* Hand over to switch */
		ring->tx_r[q][ring->c_tx[q]] |= 1;

		// Before starting TX, prevent a Lextra bus bug on RTL8380 SoCs
		if (priv->family_id == RTL8380_FAMILY_ID) {
			for (i = 0; i < 10; i++) {
				val = sw_r32(priv->r->dma_if_ctrl);
				if ((val & 0xc) == 0xc)
					break;
			}
		}

		/* Tell switch to send data */
		if (priv->family_id == RTL9310_FAMILY_ID
			|| priv->family_id == RTL9300_FAMILY_ID) {
			// Ring ID q == 0: Low priority, Ring ID = 1: High prio queue
			if (!q)
				sw_w32_mask(0, BIT(2), priv->r->dma_if_ctrl);
			else
				sw_w32_mask(0, BIT(3), priv->r->dma_if_ctrl);
		} else {
			sw_w32_mask(0, TX_DO, priv->r->dma_if_ctrl);
		}

		dev->stats.tx_packets++;
		dev->stats.tx_bytes += len;
		dev_kfree_skb(skb);
		ring->c_tx[q] = (ring->c_tx[q] + 1) % TXRINGLEN;
		ret = NETDEV_TX_OK;
	} else {
		dev_warn(&priv->pdev->dev, "Data is owned by switch\n");
		ret = NETDEV_TX_BUSY;
	}
txdone:
	spin_unlock_irqrestore(&priv->lock, flags);
	return ret;
}

/*
 * Return queue number for TX. On the RTL83XX, these queues have equal priority
 * so we do round-robin
 */
u16 rtl83xx_pick_tx_queue(struct net_device *dev, struct sk_buff *skb,
			  struct net_device *sb_dev)
{
	static u8 last = 0;

	last++;
	return last % TXRINGS;
}

/*
 * Return queue number for TX. On the RTL93XX, queue 1 is the high priority queue
 */
u16 rtl93xx_pick_tx_queue(struct net_device *dev, struct sk_buff *skb,
			  struct net_device *sb_dev)
{
	if (skb->priority >= TC_PRIO_CONTROL)
		return 1;
	return 0;
}

static int rtl838x_hw_receive(struct net_device *dev, int r, int budget)
{
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	struct ring_b *ring = priv->membase;
	struct sk_buff *skb;
	unsigned long flags;
	int i, len, work_done = 0;
	u8 *data, *skb_data;
	unsigned int val;
	u32	*last;
	struct p_hdr *h;
	bool dsa = netdev_uses_dsa(dev);
	struct dsa_tag tag;

	spin_lock_irqsave(&priv->lock, flags);
	last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));
	pr_debug("---------------------------------------------------------- RX - %d\n", r);

	do {
		if ((ring->rx_r[r][ring->c_rx[r]] & 0x1)) {
			if (&ring->rx_r[r][ring->c_rx[r]] != last) {
				netdev_warn(dev, "Ring contention: r: %x, last %x, cur %x\n",
				    r, (uint32_t)last, (u32) &ring->rx_r[r][ring->c_rx[r]]);
			}
			break;
		}

		h = &ring->rx_header[r][ring->c_rx[r]];
		data = (u8 *)KSEG1ADDR(h->buf);
		len = h->len;
		if (!len)
			break;
		work_done++;

		len -= 4; /* strip the CRC */
		/* Add 4 bytes for cpu_tag */
		if (dsa)
			len += 4;

		skb = alloc_skb(len + 4, GFP_KERNEL);
		skb_reserve(skb, NET_IP_ALIGN);

		if (likely(skb)) {
			/* BUG: Prevent bug on RTL838x SoCs*/
			if (priv->family_id == RTL8380_FAMILY_ID) {
				sw_w32(0xffffffff, priv->r->dma_if_rx_ring_size(0));
				for (i = 0; i < priv->rxrings; i++) {
					/* Update each ring cnt */
					val = sw_r32(priv->r->dma_if_rx_ring_cntr(i));
					sw_w32(val, priv->r->dma_if_rx_ring_cntr(i));
				}
			}

			skb_data = skb_put(skb, len);
			/* Make sure data is visible */
			mb();
			memcpy(skb->data, (u8 *)KSEG1ADDR(data), len);
			/* Overwrite CRC with cpu_tag */
			if (dsa) {
				priv->r->decode_tag(h, &tag);
				skb->data[len-4] = 0x80;
				skb->data[len-3] = tag.port;
				skb->data[len-2] = 0x10;
				skb->data[len-1] = 0x00;
				if (tag.l2_offloaded)
					skb->data[len-3] |= 0x40;
			}

			if (tag.queue >= 0)
				pr_debug("Queue: %d, len: %d, reason %d port %d\n",
					 tag.queue, len, tag.reason, tag.port);

			skb->protocol = eth_type_trans(skb, dev);
			if (dev->features & NETIF_F_RXCSUM) {
				if (tag.crc_error)
					skb_checksum_none_assert(skb);
				else
					skb->ip_summed = CHECKSUM_UNNECESSARY;
			}
			dev->stats.rx_packets++;
			dev->stats.rx_bytes += len;

			netif_receive_skb(skb);
		} else {
			if (net_ratelimit())
				dev_warn(&dev->dev, "low on memory - packet dropped\n");
			dev->stats.rx_dropped++;
		}

		/* Reset header structure */
		memset(h, 0, sizeof(struct p_hdr));
		h->buf = data;
		h->size = RING_BUFFER;

		ring->rx_r[r][ring->c_rx[r]] = KSEG1ADDR(h) | 0x1 
			| (ring->c_rx[r] == (priv->rxringlen - 1) ? WRAP : 0x1);
		ring->c_rx[r] = (ring->c_rx[r] + 1) % priv->rxringlen;
		last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));
	} while (&ring->rx_r[r][ring->c_rx[r]] != last && work_done < budget);

	// Update counters
	priv->r->update_cntr(r, 0);

	spin_unlock_irqrestore(&priv->lock, flags);
	return work_done;
}

static int rtl838x_poll_rx(struct napi_struct *napi, int budget)
{
	struct rtl838x_rx_q *rx_q = container_of(napi, struct rtl838x_rx_q, napi);
	struct rtl838x_eth_priv *priv = rx_q->priv;
	int work_done = 0;
	int r = rx_q->id;
	int work;

	while (work_done < budget) {
		work = rtl838x_hw_receive(priv->netdev, r, budget - work_done);
		if (!work)
			break;
		work_done += work;
	}

	if (work_done < budget) {
		napi_complete_done(napi, work_done);

		/* Enable RX interrupt */
		if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID)
			sw_w32(0xffffffff, priv->r->dma_if_intr_rx_done_msk);
		else
			sw_w32_mask(0, 0xf00ff | BIT(r + 8), priv->r->dma_if_intr_msk);
	}
	return work_done;
}


static void rtl838x_validate(struct phylink_config *config,
			 unsigned long *supported,
			 struct phylink_link_state *state)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(mask) = { 0, };

	pr_debug("In %s\n", __func__);

	if (!phy_interface_mode_is_rgmii(state->interface) &&
	    state->interface != PHY_INTERFACE_MODE_1000BASEX &&
	    state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII &&
	    state->interface != PHY_INTERFACE_MODE_GMII &&
	    state->interface != PHY_INTERFACE_MODE_QSGMII &&
	    state->interface != PHY_INTERFACE_MODE_INTERNAL &&
	    state->interface != PHY_INTERFACE_MODE_SGMII) {
		bitmap_zero(supported, __ETHTOOL_LINK_MODE_MASK_NBITS);
		pr_err("Unsupported interface: %d\n", state->interface);
		return;
	}

	/* Allow all the expected bits */
	phylink_set(mask, Autoneg);
	phylink_set_port_modes(mask);
	phylink_set(mask, Pause);
	phylink_set(mask, Asym_Pause);

	/* With the exclusion of MII and Reverse MII, we support Gigabit,
	 * including Half duplex
	 */
	if (state->interface != PHY_INTERFACE_MODE_MII &&
	    state->interface != PHY_INTERFACE_MODE_REVMII) {
		phylink_set(mask, 1000baseT_Full);
		phylink_set(mask, 1000baseT_Half);
	}

	phylink_set(mask, 10baseT_Half);
	phylink_set(mask, 10baseT_Full);
	phylink_set(mask, 100baseT_Half);
	phylink_set(mask, 100baseT_Full);

	bitmap_and(supported, supported, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);
	bitmap_and(state->advertising, state->advertising, mask,
		   __ETHTOOL_LINK_MODE_MASK_NBITS);
}


static void rtl838x_mac_config(struct phylink_config *config,
			       unsigned int mode,
			       const struct phylink_link_state *state)
{
	/* This is only being called for the master device,
	 * i.e. the CPU-Port. We don't need to do anything.
	 */

	pr_info("In %s, mode %x\n", __func__, mode);
}

static void rtl838x_mac_an_restart(struct phylink_config *config)
{
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	/* This works only on RTL838x chips */
	if (priv->family_id != RTL8380_FAMILY_ID)
		return;

	pr_debug("In %s\n", __func__);
	/* Restart by disabling and re-enabling link */
	sw_w32(0x6192D, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
	mdelay(20);
	sw_w32(0x6192F, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static int rtl838x_mac_pcs_get_state(struct phylink_config *config,
				  struct phylink_link_state *state)
{
	u32 speed;
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	int port = priv->cpu_port;

	pr_debug("In %s\n", __func__);

	state->link = priv->r->get_mac_link_sts(port) ? 1 : 0;
	state->duplex = priv->r->get_mac_link_dup_sts(port) ? 1 : 0;

	speed = priv->r->get_mac_link_spd_sts(port);
	switch (speed) {
	case 0:
		state->speed = SPEED_10;
		break;
	case 1:
		state->speed = SPEED_100;
		break;
	case 2:
		state->speed = SPEED_1000;
		break;
	default:
		state->speed = SPEED_UNKNOWN;
		break;
	}

	state->pause &= (MLO_PAUSE_RX | MLO_PAUSE_TX);
	if (priv->r->get_mac_rx_pause_sts(port))
		state->pause |= MLO_PAUSE_RX;
	if (priv->r->get_mac_tx_pause_sts(port))
		state->pause |= MLO_PAUSE_TX;

	return 1;
}

static void rtl838x_mac_link_down(struct phylink_config *config,
				  unsigned int mode,
				  phy_interface_t interface)
{
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	pr_debug("In %s\n", __func__);
	/* Stop TX/RX to port */
	sw_w32_mask(0x03, 0, priv->r->mac_port_ctrl(priv->cpu_port));
}

static void rtl838x_mac_link_up(struct phylink_config *config, unsigned int mode,
			    phy_interface_t interface,
			    struct phy_device *phy)
{
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	pr_debug("In %s\n", __func__);
	/* Restart TX/RX to port */
	sw_w32_mask(0, 0x03, priv->r->mac_port_ctrl(priv->cpu_port));
}

static void rtl838x_set_mac_hw(struct net_device *dev, u8 *mac)
{
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	unsigned long flags;

	spin_lock_irqsave(&priv->lock, flags);
	pr_debug("In %s\n", __func__);
	sw_w32((mac[0] << 8) | mac[1], priv->r->mac);
	sw_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5], priv->r->mac + 4);

	if (priv->family_id == RTL8380_FAMILY_ID) {
		/* 2 more registers, ALE/MAC block */
		sw_w32((mac[0] << 8) | mac[1], RTL838X_MAC_ALE);
		sw_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
		       (RTL838X_MAC_ALE + 4));

		sw_w32((mac[0] << 8) | mac[1], RTL838X_MAC2);
		sw_w32((mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5],
		       RTL838X_MAC2 + 4);
	}
	spin_unlock_irqrestore(&priv->lock, flags);
}

static int rtl838x_set_mac_address(struct net_device *dev, void *p)
{
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	const struct sockaddr *addr = p;
	u8 *mac = (u8 *) (addr->sa_data);

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	memcpy(dev->dev_addr, addr->sa_data, ETH_ALEN);
	rtl838x_set_mac_hw(dev, mac);

	pr_info("Using MAC %08x%08x\n", sw_r32(priv->r->mac), sw_r32(priv->r->mac + 4));
	return 0;
}

static int rtl8390_init_mac(struct rtl838x_eth_priv *priv)
{
	// We will need to set-up EEE and the egress-rate limitation
	return 0;
}

static int rtl8380_init_mac(struct rtl838x_eth_priv *priv)
{
	int i;

	if (priv->family_id == 0x8390)
		return rtl8390_init_mac(priv);

	pr_info("%s\n", __func__);
	/* fix timer for EEE */
	sw_w32(0x5001411, RTL838X_EEE_TX_TIMER_GIGA_CTRL);
	sw_w32(0x5001417, RTL838X_EEE_TX_TIMER_GELITE_CTRL);

	/* Init VLAN */
	if (priv->id == 0x8382) {
		for (i = 0; i <= 28; i++)
			sw_w32(0, 0xd57c + i * 0x80);
	}
	if (priv->id == 0x8380) {
		for (i = 8; i <= 28; i++)
			sw_w32(0, 0xd57c + i * 0x80);
	}
	return 0;
}

static int rtl838x_get_link_ksettings(struct net_device *ndev,
				      struct ethtool_link_ksettings *cmd)
{
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	pr_debug("%s called\n", __func__);
	return phylink_ethtool_ksettings_get(priv->phylink, cmd);
}

static int rtl838x_set_link_ksettings(struct net_device *ndev,
				      const struct ethtool_link_ksettings *cmd)
{
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	pr_debug("%s called\n", __func__);
	return phylink_ethtool_ksettings_set(priv->phylink, cmd);
}

static int rtl838x_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	u32 val;
	int err;
	struct rtl838x_eth_priv *priv = bus->priv;

	if (mii_id >= 24 && mii_id <= 27 && priv->id == 0x8380)
		return rtl838x_read_sds_phy(mii_id, regnum);
	err = rtl838x_read_phy(mii_id, 0, regnum, &val);
	if (err)
		return err;
	return val;
}

static int rtl839x_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	u32 val;
	int err;
	struct rtl838x_eth_priv *priv = bus->priv;

	if (mii_id >= 48 && mii_id <= 49 && priv->id == 0x8393)
		return rtl839x_read_sds_phy(mii_id, regnum);

	err = rtl839x_read_phy(mii_id, 0, regnum, &val);
	if (err)
		return err;
	return val;
}

static int rtl930x_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	u32 val;
	int err;

	// TODO: These are hard-coded for the 2 Fibre Ports of the XGS1210
	if (mii_id >= 26 && mii_id <= 27)
		return rtl930x_read_sds_phy(mii_id - 18, 0, regnum);

	if (regnum & MII_ADDR_C45) {
		regnum &= ~MII_ADDR_C45;
		err = rtl930x_read_mmd_phy(mii_id, regnum >> 16, regnum & 0xffff, &val);
	} else {
		err = rtl930x_read_phy(mii_id, 0, regnum, &val);
	}
	if (err)
		return err;
	return val;
}

static int rtl931x_mdio_read(struct mii_bus *bus, int mii_id, int regnum)
{
	u32 val;
	int err;
//	struct rtl838x_eth_priv *priv = bus->priv;

//	if (mii_id >= 48 && mii_id <= 49 && priv->id == 0x8393)
//		return rtl839x_read_sds_phy(mii_id, regnum);

	err = rtl931x_read_phy(mii_id, 0, regnum, &val);
	if (err)
		return err;
	return val;
}

static int rtl838x_mdio_write(struct mii_bus *bus, int mii_id,
			      int regnum, u16 value)
{
	u32 offset = 0;
	struct rtl838x_eth_priv *priv = bus->priv;

	if (mii_id >= 24 && mii_id <= 27 && priv->id == 0x8380) {
		if (mii_id == 26)
			offset = 0x100;
		sw_w32(value, RTL838X_SDS4_FIB_REG0 + offset + (regnum << 2));
		return 0;
	}
	return rtl838x_write_phy(mii_id, 0, regnum, value);
}

static int rtl839x_mdio_write(struct mii_bus *bus, int mii_id,
			      int regnum, u16 value)
{
	struct rtl838x_eth_priv *priv = bus->priv;

	if (mii_id >= 48 && mii_id <= 49 && priv->id == 0x8393)
		return rtl839x_write_sds_phy(mii_id, regnum, value);

	return rtl839x_write_phy(mii_id, 0, regnum, value);
}

static int rtl930x_mdio_write(struct mii_bus *bus, int mii_id,
			      int regnum, u16 value)
{
//	struct rtl838x_eth_priv *priv = bus->priv;

//	if (mii_id >= 48 && mii_id <= 49 && priv->id == 0x8393)
//		return rtl839x_write_sds_phy(mii_id, regnum, value);
	if (regnum & MII_ADDR_C45) {
		regnum &= ~MII_ADDR_C45;
		return rtl930x_write_mmd_phy(mii_id, regnum >> 16, regnum & 0xffff, value);
	}

	return rtl930x_write_phy(mii_id, 0, regnum, value);
}

static int rtl931x_mdio_write(struct mii_bus *bus, int mii_id,
			      int regnum, u16 value)
{
//	struct rtl838x_eth_priv *priv = bus->priv;

//	if (mii_id >= 48 && mii_id <= 49 && priv->id == 0x8393)
//		return rtl839x_write_sds_phy(mii_id, regnum, value);

	return rtl931x_write_phy(mii_id, 0, regnum, value);
}

static int rtl838x_mdio_reset(struct mii_bus *bus)
{
	pr_debug("%s called\n", __func__);
	/* Disable MAC polling the PHY so that we can start configuration */
	sw_w32(0x00000000, RTL838X_SMI_POLL_CTRL);

	/* Enable PHY control via SoC */
	sw_w32_mask(0, 1 << 15, RTL838X_SMI_GLB_CTRL);

	// Probably should reset all PHYs here...
	return 0;
}

static int rtl839x_mdio_reset(struct mii_bus *bus)
{
	return 0;

	pr_debug("%s called\n", __func__);
	/* BUG: The following does not work, but should! */
	/* Disable MAC polling the PHY so that we can start configuration */
	sw_w32(0x00000000, RTL839X_SMI_PORT_POLLING_CTRL);
	sw_w32(0x00000000, RTL839X_SMI_PORT_POLLING_CTRL + 4);
	/* Disable PHY polling via SoC */
	sw_w32_mask(1 << 7, 0, RTL839X_SMI_GLB_CTRL);

	// Probably should reset all PHYs here...
	return 0;
}

static int rtl931x_mdio_reset(struct mii_bus *bus)
{
	sw_w32(0x00000000, RTL931X_SMI_PORT_POLLING_CTRL);
	sw_w32(0x00000000, RTL931X_SMI_PORT_POLLING_CTRL + 4);

	pr_debug("%s called\n", __func__);

	return 0;
}

static int rtl930x_mdio_reset(struct mii_bus *bus)
{
	int i;
	int pos;

	pr_info("RTL930X_SMI_PORT0_15_POLLING_SEL %08x 16-27: %08x\n",
		sw_r32(RTL930X_SMI_PORT0_15_POLLING_SEL),
		sw_r32(RTL930X_SMI_PORT16_27_POLLING_SEL));

	pr_info("%s: Enable SMI polling on SMI bus 0, SMI1, SMI2, disable on SMI3\n", __func__);
	sw_w32_mask(BIT(20) | BIT(21) | BIT(22), BIT(23), RTL930X_SMI_GLB_CTRL);

	pr_info("RTL9300 Powering on SerDes ports\n");
	rtl9300_sds_power(24, 1);
	rtl9300_sds_power(25, 1);
	rtl9300_sds_power(26, 1);
	rtl9300_sds_power(27, 1);
	mdelay(200);

	// RTL930X_SMI_PORT0_15_POLLING_SEL 55550000 16-27: 00f9aaaa
	// i.e SMI=0 for all ports
	for (i = 0; i < 5; i++)
		pr_info("port phy: %08x\n", sw_r32(RTL930X_SMI_PORT0_5_ADDR + i *4));

	// 1-to-1 mapping of port to phy-address
	for (i = 0; i < 24; i++) {
		pos = (i % 6) * 5;
		sw_w32_mask(0x1f << pos, i << pos, RTL930X_SMI_PORT0_5_ADDR + (i / 6) * 4);
	}

	// ports 24 and 25 have PHY addresses 8 and 9, ports 26/27 PHY 26/27
	sw_w32(8 | 9 << 5 | 26 << 10 | 27 << 15, RTL930X_SMI_PORT0_5_ADDR + 4 * 4);

	// Ports 24 and 25 live on SMI bus 1 and 2
	sw_w32_mask(0x3 << 16, 0x1 << 16, RTL930X_SMI_PORT16_27_POLLING_SEL);
	sw_w32_mask(0x3 << 18, 0x2 << 18, RTL930X_SMI_PORT16_27_POLLING_SEL);

	// SMI bus 1 and 2 speak Clause 45 TODO: Configure from .dts
	sw_w32_mask(0, BIT(17) | BIT(18), RTL930X_SMI_GLB_CTRL);

	// Ports 24 and 25 are 2.5 Gig, set this type (1)
	sw_w32_mask(0x7 << 12, 1 << 12, RTL930X_SMI_MAC_TYPE_CTRL);
	sw_w32_mask(0x7 << 15, 1 << 15, RTL930X_SMI_MAC_TYPE_CTRL);

	return 0;
}

static int rtl838x_mdio_init(struct rtl838x_eth_priv *priv)
{
	struct device_node *mii_np;
	int ret;

	pr_debug("%s called\n", __func__);
	mii_np = of_get_child_by_name(priv->pdev->dev.of_node, "mdio-bus");

	if (!mii_np) {
		dev_err(&priv->pdev->dev, "no %s child node found", "mdio-bus");
		return -ENODEV;
	}

	if (!of_device_is_available(mii_np)) {
		ret = -ENODEV;
		goto err_put_node;
	}

	priv->mii_bus = devm_mdiobus_alloc(&priv->pdev->dev);
	if (!priv->mii_bus) {
		ret = -ENOMEM;
		goto err_put_node;
	}

	switch(priv->family_id) {
	case RTL8380_FAMILY_ID:
		priv->mii_bus->name = "rtl838x-eth-mdio";
		priv->mii_bus->read = rtl838x_mdio_read;
		priv->mii_bus->write = rtl838x_mdio_write;
		priv->mii_bus->reset = rtl838x_mdio_reset;
		break;
	case RTL8390_FAMILY_ID:
		priv->mii_bus->name = "rtl839x-eth-mdio";
		priv->mii_bus->read = rtl839x_mdio_read;
		priv->mii_bus->write = rtl839x_mdio_write;
		priv->mii_bus->reset = rtl839x_mdio_reset;
		break;
	case RTL9300_FAMILY_ID:
		priv->mii_bus->name = "rtl930x-eth-mdio";
		priv->mii_bus->read = rtl930x_mdio_read;
		priv->mii_bus->write = rtl930x_mdio_write;
		priv->mii_bus->reset = rtl930x_mdio_reset;
	//	priv->mii_bus->probe_capabilities = MDIOBUS_C22_C45; TODO for linux 5.9
		break;
	case RTL9310_FAMILY_ID:
		priv->mii_bus->name = "rtl931x-eth-mdio";
		priv->mii_bus->read = rtl931x_mdio_read;
		priv->mii_bus->write = rtl931x_mdio_write;
		priv->mii_bus->reset = rtl931x_mdio_reset;
//		priv->mii_bus->probe_capabilities = MDIOBUS_C22_C45;  TODO for linux 5.9
		break;
	}
	priv->mii_bus->priv = priv;
	priv->mii_bus->parent = &priv->pdev->dev;

	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%pOFn", mii_np);
	ret = of_mdiobus_register(priv->mii_bus, mii_np);

err_put_node:
	of_node_put(mii_np);
	return ret;
}

static int rtl838x_mdio_remove(struct rtl838x_eth_priv *priv)
{
	pr_debug("%s called\n", __func__);
	if (!priv->mii_bus)
		return 0;

	mdiobus_unregister(priv->mii_bus);
	mdiobus_free(priv->mii_bus);

	return 0;
}

static netdev_features_t rtl838x_fix_features(struct net_device *dev,
					  netdev_features_t features)
{
	return features;
}

static int rtl83xx_set_features(struct net_device *dev, netdev_features_t features)
{
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	if ((features ^ dev->features) & NETIF_F_RXCSUM) {
		if (!(features & NETIF_F_RXCSUM))
			sw_w32_mask(BIT(3), 0, priv->r->mac_port_ctrl(priv->cpu_port));
		else
			sw_w32_mask(0, BIT(4), priv->r->mac_port_ctrl(priv->cpu_port));
	}

	return 0;
}

static int rtl93xx_set_features(struct net_device *dev, netdev_features_t features)
{
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	if ((features ^ dev->features) & NETIF_F_RXCSUM) {
		if (!(features & NETIF_F_RXCSUM))
			sw_w32_mask(BIT(4), 0, priv->r->mac_port_ctrl(priv->cpu_port));
		else
			sw_w32_mask(0, BIT(4), priv->r->mac_port_ctrl(priv->cpu_port));
	}

	return 0;
}

static const struct net_device_ops rtl838x_eth_netdev_ops = {
	.ndo_open = rtl838x_eth_open,
	.ndo_stop = rtl838x_eth_stop,
	.ndo_start_xmit = rtl838x_eth_tx,
	.ndo_select_queue = rtl83xx_pick_tx_queue,
	.ndo_set_mac_address = rtl838x_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rtl838x_eth_set_multicast_list,
	.ndo_tx_timeout = rtl838x_eth_tx_timeout,
	.ndo_set_features = rtl83xx_set_features,
	.ndo_fix_features = rtl838x_fix_features,
};

static const struct net_device_ops rtl839x_eth_netdev_ops = {
	.ndo_open = rtl838x_eth_open,
	.ndo_stop = rtl838x_eth_stop,
	.ndo_start_xmit = rtl838x_eth_tx,
	.ndo_select_queue = rtl83xx_pick_tx_queue,
	.ndo_set_mac_address = rtl838x_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rtl839x_eth_set_multicast_list,
	.ndo_tx_timeout = rtl838x_eth_tx_timeout,
	.ndo_set_features = rtl83xx_set_features,
	.ndo_fix_features = rtl838x_fix_features,
};

static const struct net_device_ops rtl930x_eth_netdev_ops = {
	.ndo_open = rtl838x_eth_open,
	.ndo_stop = rtl838x_eth_stop,
	.ndo_start_xmit = rtl838x_eth_tx,
	.ndo_select_queue = rtl93xx_pick_tx_queue,
	.ndo_set_mac_address = rtl838x_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rtl930x_eth_set_multicast_list,
	.ndo_tx_timeout = rtl838x_eth_tx_timeout,
	.ndo_set_features = rtl93xx_set_features,
	.ndo_fix_features = rtl838x_fix_features,
};

static const struct net_device_ops rtl931x_eth_netdev_ops = {
	.ndo_open = rtl838x_eth_open,
	.ndo_stop = rtl838x_eth_stop,
	.ndo_start_xmit = rtl838x_eth_tx,
	.ndo_select_queue = rtl93xx_pick_tx_queue,
	.ndo_set_mac_address = rtl838x_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rtl931x_eth_set_multicast_list,
	.ndo_tx_timeout = rtl838x_eth_tx_timeout,
	.ndo_set_features = rtl93xx_set_features,
	.ndo_fix_features = rtl838x_fix_features,
};

static const struct phylink_mac_ops rtl838x_phylink_ops = {
	.validate = rtl838x_validate,
	.mac_link_state = rtl838x_mac_pcs_get_state,
	.mac_an_restart = rtl838x_mac_an_restart,
	.mac_config = rtl838x_mac_config,
	.mac_link_down = rtl838x_mac_link_down,
	.mac_link_up = rtl838x_mac_link_up,
};

static const struct ethtool_ops rtl838x_ethtool_ops = {
	.get_link_ksettings     = rtl838x_get_link_ksettings,
	.set_link_ksettings     = rtl838x_set_link_ksettings,
};

static int __init rtl838x_eth_probe(struct platform_device *pdev)
{
	struct net_device *dev;
	struct device_node *dn = pdev->dev.of_node;
	struct rtl838x_eth_priv *priv;
	struct resource *res, *mem;
	const void *mac;
	phy_interface_t phy_mode;
	struct phylink *phylink;
	int err = 0, i, rxrings, rxringlen;
	struct ring_b *ring;

	pr_info("Probing RTL838X eth device pdev: %x, dev: %x\n",
		(u32)pdev, (u32)(&(pdev->dev)));

	if (!dn) {
		dev_err(&pdev->dev, "No DT found\n");
		return -EINVAL;
	}

	rxrings = (soc_info.family == RTL8380_FAMILY_ID 
			|| soc_info.family == RTL8390_FAMILY_ID) ? 8 : 32;
	rxrings = rxrings > MAX_RXRINGS ? MAX_RXRINGS : rxrings;
	rxringlen = MAX_ENTRIES / rxrings;
	rxringlen = rxringlen > MAX_RXLEN ? MAX_RXLEN : rxringlen;

	dev = alloc_etherdev_mqs(sizeof(struct rtl838x_eth_priv), TXRINGS, rxrings);
	if (!dev) {
		err = -ENOMEM;
		goto err_free;
	}
	SET_NETDEV_DEV(dev, &pdev->dev);
	priv = netdev_priv(dev);

	/* obtain buffer memory space */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		mem = devm_request_mem_region(&pdev->dev, res->start,
			resource_size(res), res->name);
		if (!mem) {
			dev_err(&pdev->dev, "cannot request memory space\n");
			err = -ENXIO;
			goto err_free;
		}

		dev->mem_start = mem->start;
		dev->mem_end   = mem->end;
	} else {
		dev_err(&pdev->dev, "cannot request IO resource\n");
		err = -ENXIO;
		goto err_free;
	}

	/* Allocate buffer memory */
	priv->membase = dmam_alloc_coherent(&pdev->dev, rxrings * rxringlen * RING_BUFFER
				+ sizeof(struct ring_b) + sizeof(struct notify_b),
				(void *)&dev->mem_start, GFP_KERNEL);
	if (!priv->membase) {
		dev_err(&pdev->dev, "cannot allocate DMA buffer\n");
		err = -ENOMEM;
		goto err_free;
	}

	// Allocate ring-buffer space at the end of the allocated memory
	ring = priv->membase;
	ring->rx_space = priv->membase + sizeof(struct ring_b) + sizeof(struct notify_b);

	spin_lock_init(&priv->lock);

	/* obtain device IRQ number */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!res) {
		dev_err(&pdev->dev, "cannot obtain IRQ, using default 24\n");
		dev->irq = 24;
	} else {
		dev->irq = res->start;
	}
	dev->ethtool_ops = &rtl838x_ethtool_ops;
	dev->min_mtu = ETH_ZLEN;
	dev->max_mtu = 1536;
	dev->features = NETIF_F_RXCSUM | NETIF_F_HW_CSUM;
	dev->hw_features = NETIF_F_RXCSUM;

	priv->id = soc_info.id;
	priv->family_id = soc_info.family;
	if (priv->id) {
		pr_info("Found SoC ID: %4x: %s, family %x\n",
			priv->id, soc_info.name, priv->family_id);
	} else {
		pr_err("Unknown chip id (%04x)\n", priv->id);
		return -ENODEV;
	}

	switch (priv->family_id) {
	case RTL8380_FAMILY_ID:
		priv->cpu_port = RTL838X_CPU_PORT;
		priv->r = &rtl838x_reg;
		dev->netdev_ops = &rtl838x_eth_netdev_ops;
		break;
	case RTL8390_FAMILY_ID:
		priv->cpu_port = RTL839X_CPU_PORT;
		priv->r = &rtl839x_reg;
		dev->netdev_ops = &rtl839x_eth_netdev_ops;
		break;
	case RTL9300_FAMILY_ID:
		priv->cpu_port = RTL930X_CPU_PORT;
		priv->r = &rtl930x_reg;
		dev->netdev_ops = &rtl930x_eth_netdev_ops;
		break;
	case RTL9310_FAMILY_ID:
		priv->cpu_port = RTL931X_CPU_PORT;
		priv->r = &rtl931x_reg;
		dev->netdev_ops = &rtl931x_eth_netdev_ops;
		break;
	default:
		pr_err("Unknown SoC family\n");
		return -ENODEV;
	}
	priv->rxringlen = rxringlen;
	priv->rxrings = rxrings;

	rtl8380_init_mac(priv);

	/* try to get mac address in the following order:
	 * 1) from device tree data
	 * 2) from internal registers set by bootloader
	 */
	mac = of_get_mac_address(pdev->dev.of_node);
	if (!IS_ERR(mac)) {
		memcpy(dev->dev_addr, mac, ETH_ALEN);
		rtl838x_set_mac_hw(dev, (u8 *)mac);
	} else {
		dev->dev_addr[0] = (sw_r32(priv->r->mac) >> 8) & 0xff;
		dev->dev_addr[1] = sw_r32(priv->r->mac) & 0xff;
		dev->dev_addr[2] = (sw_r32(priv->r->mac + 4) >> 24) & 0xff;
		dev->dev_addr[3] = (sw_r32(priv->r->mac + 4) >> 16) & 0xff;
		dev->dev_addr[4] = (sw_r32(priv->r->mac + 4) >> 8) & 0xff;
		dev->dev_addr[5] = sw_r32(priv->r->mac + 4) & 0xff;
	}
	/* if the address is invalid, use a random value */
	if (!is_valid_ether_addr(dev->dev_addr)) {
		struct sockaddr sa = { AF_UNSPEC };

		netdev_warn(dev, "Invalid MAC address, using random\n");
		eth_hw_addr_random(dev);
		memcpy(sa.sa_data, dev->dev_addr, ETH_ALEN);
		if (rtl838x_set_mac_address(dev, &sa))
			netdev_warn(dev, "Failed to set MAC address.\n");
	}
	pr_info("Using MAC %08x%08x\n", sw_r32(priv->r->mac),
					sw_r32(priv->r->mac + 4));
	strcpy(dev->name, "eth%d");
	priv->pdev = pdev;
	priv->netdev = dev;

	err = rtl838x_mdio_init(priv);
	if (err)
		goto err_free;

	err = register_netdev(dev);
	if (err)
		goto err_free;

	for (i = 0; i < priv->rxrings; i++) {
		priv->rx_qs[i].id = i;
		priv->rx_qs[i].priv = priv;
		netif_napi_add(dev, &priv->rx_qs[i].napi, rtl838x_poll_rx, 64);
	}

	platform_set_drvdata(pdev, dev);

	phy_mode = of_get_phy_mode(dn);
	if (phy_mode < 0) {
		dev_err(&pdev->dev, "incorrect phy-mode\n");
		err = -EINVAL;
		goto err_free;
	}
	priv->phylink_config.dev = &dev->dev;
	priv->phylink_config.type = PHYLINK_NETDEV;

	phylink = phylink_create(&priv->phylink_config, pdev->dev.fwnode,
				 phy_mode, &rtl838x_phylink_ops);
	if (IS_ERR(phylink)) {
		err = PTR_ERR(phylink);
		goto err_free;
	}
	priv->phylink = phylink;

	return 0;

err_free:
	pr_err("Error setting up netdev, freeing it again.\n");
	free_netdev(dev);
	return err;
}

static int rtl838x_eth_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	int i;

	if (dev) {
		pr_info("Removing platform driver for rtl838x-eth\n");
		rtl838x_mdio_remove(priv);
		rtl838x_hw_stop(priv);

		netif_tx_stop_all_queues(dev);

		for (i = 0; i < priv->rxrings; i++)
			netif_napi_del(&priv->rx_qs[i].napi);

		unregister_netdev(dev);
		free_netdev(dev);
	}
	return 0;
}

static const struct of_device_id rtl838x_eth_of_ids[] = {
	{ .compatible = "realtek,rtl838x-eth"},
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rtl838x_eth_of_ids);

static struct platform_driver rtl838x_eth_driver = {
	.probe = rtl838x_eth_probe,
	.remove = rtl838x_eth_remove,
	.driver = {
		.name = "rtl838x-eth",
		.pm = NULL,
		.of_match_table = rtl838x_eth_of_ids,
	},
};

module_platform_driver(rtl838x_eth_driver);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL838X SoC Ethernet Driver");
MODULE_LICENSE("GPL");
