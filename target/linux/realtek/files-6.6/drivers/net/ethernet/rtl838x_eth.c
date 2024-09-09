// SPDX-License-Identifier: GPL-2.0-only
/* linux/drivers/net/ethernet/rtl838x_eth.c
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

/* Maximum number of RX rings is 8 on RTL83XX and 32 on the 93XX
 * The ring is assigned by switch based on packet/port priortity
 * Maximum number of TX rings is 2, Ring 2 being the high priority
 * ring on the RTL93xx SoCs. MAX_RXLEN gives the maximum length
 * for an RX ring, MAX_ENTRIES the maximum number of entries
 * available in total for all queues.
 */
#define MAX_RXRINGS	32
#define MAX_RXLEN	300
#define MAX_ENTRIES	(300 * 8)
#define TXRINGS		2
#define TXRINGLEN	160
#define NOTIFY_EVENTS	10
#define NOTIFY_BLOCKS	10
#define TX_EN		0x8
#define RX_EN		0x4
#define TX_EN_93XX	0x20
#define RX_EN_93XX	0x10
#define RX_TRUNCATE_EN_93XX BIT(6)
#define RX_TRUNCATE_EN_83XX BIT(4)
#define TX_PAD_EN_838X BIT(5)
#define TX_DO		0x2
#define WRAP		0x2
#define MAX_PORTS	57
#define MAX_SMI_BUSSES	4

#define RING_BUFFER	1600

struct p_hdr {
	uint8_t		*buf;
	uint16_t	reserved;
	uint16_t	size;		/* buffer size */
	uint16_t	offset;
	uint16_t	len;		/* pkt len */
	/* cpu_tag[0] is a reserved uint16_t on RTL83xx */
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

static void rtl838x_create_tx_header(struct p_hdr *h, unsigned int dest_port, int prio)
{
	/* cpu_tag[0] is reserved on the RTL83XX SoCs */
	h->cpu_tag[1] = 0x0400;  /* BIT 10: RTL8380_CPU_TAG */
	h->cpu_tag[2] = 0x0200;  /* Set only AS_DPM, to enable DPM settings below */
	h->cpu_tag[3] = 0x0000;
	h->cpu_tag[4] = BIT(dest_port) >> 16;
	h->cpu_tag[5] = BIT(dest_port) & 0xffff;

	/* Set internal priority (PRI) and enable (AS_PRI) */
	if (prio >= 0)
		h->cpu_tag[2] |= ((prio & 0x7) | BIT(3)) << 12;
}

static void rtl839x_create_tx_header(struct p_hdr *h, unsigned int dest_port, int prio)
{
	/* cpu_tag[0] is reserved on the RTL83XX SoCs */
	h->cpu_tag[1] = 0x0100; /* RTL8390_CPU_TAG marker */
	h->cpu_tag[2] = BIT(4); /* AS_DPM flag */
	h->cpu_tag[3] = h->cpu_tag[4] = h->cpu_tag[5] = 0;
	/* h->cpu_tag[1] |= BIT(1) | BIT(0); */ /* Bypass filter 1/2 */
	if (dest_port >= 32) {
		dest_port -= 32;
		h->cpu_tag[2] |= (BIT(dest_port) >> 16) & 0xf;
		h->cpu_tag[3] = BIT(dest_port) & 0xffff;
	} else {
		h->cpu_tag[4] = BIT(dest_port) >> 16;
		h->cpu_tag[5] = BIT(dest_port) & 0xffff;
	}

	/* Set internal priority (PRI) and enable (AS_PRI) */
	if (prio >= 0)
		h->cpu_tag[2] |= ((prio & 0x7) | BIT(3)) << 8;
}

static void rtl930x_create_tx_header(struct p_hdr *h, unsigned int dest_port, int prio)
{
	h->cpu_tag[0] = 0x8000;  /* CPU tag marker */
	h->cpu_tag[1] = h->cpu_tag[2] = 0;
	h->cpu_tag[3] = 0;
	h->cpu_tag[4] = 0;
	h->cpu_tag[5] = 0;
	h->cpu_tag[6] = BIT(dest_port) >> 16;
	h->cpu_tag[7] = BIT(dest_port) & 0xffff;

	/* Enable (AS_QID) and set priority queue (QID) */
	if (prio >= 0)
		h->cpu_tag[2] = (BIT(5) | (prio & 0x1f)) << 8;
}

static void rtl931x_create_tx_header(struct p_hdr *h, unsigned int dest_port, int prio)
{
	h->cpu_tag[0] = 0x8000;  /* CPU tag marker */
	h->cpu_tag[1] = h->cpu_tag[2] = 0;
	h->cpu_tag[3] = 0;
	h->cpu_tag[4] = h->cpu_tag[5] = h->cpu_tag[6] = h->cpu_tag[7] = 0;
	if (dest_port >= 32) {
		dest_port -= 32;
		h->cpu_tag[4] = BIT(dest_port) >> 16;
		h->cpu_tag[5] = BIT(dest_port) & 0xffff;
	} else {
		h->cpu_tag[6] = BIT(dest_port) >> 16;
		h->cpu_tag[7] = BIT(dest_port) & 0xffff;
	}

	/* Enable (AS_QID) and set priority queue (QID) */
	if (prio >= 0)
		h->cpu_tag[2] = (BIT(5) | (prio & 0x1f)) << 8;
}

// Currently unused
// static void rtl93xx_header_vlan_set(struct p_hdr *h, int vlan)
// {
// 	h->cpu_tag[2] |= BIT(4); /* Enable VLAN forwarding offload */
// 	h->cpu_tag[2] |= (vlan >> 8) & 0xf;
// 	h->cpu_tag[3] |= (vlan & 0xff) << 8;
// }

struct rtl838x_rx_q {
	int id;
	struct rtl838x_eth_priv *priv;
	struct napi_struct napi;
};

struct rtl838x_eth_priv {
	struct net_device *netdev;
	struct platform_device *pdev;
	void *membase;
	spinlock_t lock;
	struct mii_bus *mii_bus;
	struct rtl838x_rx_q rx_qs[MAX_RXRINGS];
	struct phylink *phylink;
	struct phylink_config phylink_config;
	struct phylink_pcs pcs;
	u16 id;
	u16 family_id;
	const struct rtl838x_eth_reg *r;
	u8 cpu_port;
	u32 lastEvent;
	u16 rxrings;
	u16 rxringlen;
	int smi_bus[MAX_PORTS];
	u8 smi_addr[MAX_PORTS];
	u32 sds_id[MAX_PORTS];
	bool smi_bus_isc45[MAX_SMI_BUSSES];
	bool phy_is_internal[MAX_PORTS];
	phy_interface_t interfaces[MAX_PORTS];
};

extern int rtl838x_phy_init(struct rtl838x_eth_priv *priv);
extern int rtl838x_read_sds_phy(int phy_addr, int phy_reg);
extern int rtl839x_read_sds_phy(int phy_addr, int phy_reg);
extern int rtl839x_write_sds_phy(int phy_addr, int phy_reg, u16 v);
extern int rtl930x_read_sds_phy(int phy_addr, int page, int phy_reg);
extern int rtl930x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v);
extern int rtl931x_read_sds_phy(int phy_addr, int page, int phy_reg);
extern int rtl931x_write_sds_phy(int phy_addr, int page, int phy_reg, u16 v);
extern int rtl930x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
extern int rtl930x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);
extern int rtl931x_read_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 *val);
extern int rtl931x_write_mmd_phy(u32 port, u32 devnum, u32 regnum, u32 val);

/* On the RTL93XX, the RTL93XX_DMA_IF_RX_RING_CNTR track the fill level of
 * the rings. Writing x into these registers substracts x from its content.
 * When the content reaches the ring size, the ASIC no longer adds
 * packets to this receive queue.
 */
void rtl838x_update_cntr(int r, int released)
{
	/* This feature is not available on RTL838x SoCs */
}

void rtl839x_update_cntr(int r, int released)
{
	/* This feature is not available on RTL839x SoCs */
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
	u32 v = sw_r32(reg);

	v = (v >> pos) & 0x3ff;
	sw_w32_mask(0x3ff << pos, released << pos, reg);
	sw_w32(v, reg);
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
	/* cpu_tag[0] is reserved. Fields are off-by-one */
	t->reason = h->cpu_tag[4] & 0xf;
	t->queue = (h->cpu_tag[1] & 0xe0) >> 5;
	t->port = h->cpu_tag[1] & 0x1f;
	t->crc_error = t->reason == 13;

	pr_debug("Reason: %d\n", t->reason);
	if (t->reason != 6) /* NIC_RX_REASON_SPECIAL_TRAP */
		t->l2_offloaded = 1;
	else
		t->l2_offloaded = 0;

	return t->l2_offloaded;
}

bool rtl839x_decode_tag(struct p_hdr *h, struct dsa_tag *t)
{
	/* cpu_tag[0] is reserved. Fields are off-by-one */
	t->reason = h->cpu_tag[5] & 0x1f;
	t->queue = (h->cpu_tag[4] & 0xe000) >> 13;
	t->port = h->cpu_tag[1] & 0x3f;
	t->crc_error = h->cpu_tag[4] & BIT(6);

	pr_debug("Reason: %d\n", t->reason);
	if ((t->reason >= 7 && t->reason <= 13) || /* NIC_RX_REASON_RMA */
	    (t->reason >= 23 && t->reason <= 25))  /* NIC_RX_REASON_SPECIAL_TRAP */
		t->l2_offloaded = 0;
	else
		t->l2_offloaded = 1;

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

	if (t->reason != 63)
		pr_info("%s: Reason %d, port %d, queue %d\n", __func__, t->reason, t->port, t->queue);
	if (t->reason >= 19 && t->reason <= 27)	/* NIC_RX_REASON_RMA */
		t->l2_offloaded = 0;
	else
		t->l2_offloaded = 1;

	return t->l2_offloaded;
}

/* Discard the RX ring-buffers, called as part of the net-ISR
 * when the buffer runs over
 */
static void rtl838x_rb_cleanup(struct rtl838x_eth_priv *priv, int status)
{
	for (int r = 0; r < priv->rxrings; r++) {
		struct ring_b *ring = priv->membase;
		struct p_hdr *h;
		u32 *last;

		pr_debug("In %s working on r: %d\n", __func__, r);
		last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));
		do {
			if ((ring->rx_r[r][ring->c_rx[r]] & 0x1))
				break;
			pr_debug("Got something: %d\n", ring->c_rx[r]);
			h = &ring->rx_header[r][ring->c_rx[r]];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->rx_space +
			                         r * priv->rxringlen * RING_BUFFER +
			                         ring->c_rx[r] * RING_BUFFER);
			h->size = RING_BUFFER;
			/* make sure the header is visible to the ASIC */
			mb();

			ring->rx_r[r][ring->c_rx[r]] = KSEG1ADDR(h) | 0x1 | (ring->c_rx[r] == (priv->rxringlen - 1) ?
			                               WRAP :
			                               0x1);
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
	const struct fdb_update_work *uw = container_of(work, struct fdb_update_work, work);

	for (int i = 0; uw->macs[i]; i++) {
	        struct switchdev_notifier_fdb_info info;
	        u8 addr[ETH_ALEN];
	        int action;

		action = (uw->macs[i] & (1ULL << 63)) ?
		         SWITCHDEV_FDB_ADD_TO_BRIDGE :
		         SWITCHDEV_FDB_DEL_TO_BRIDGE;
		u64_to_ether_addr(uw->macs[i] & 0xffffffffffffULL, addr);
		info.addr = &addr[0];
		info.vid = 0;
		info.offloaded = 1;
		pr_debug("FDB entry %d: %llx, action %d\n", i, uw->macs[0], action);
		call_switchdev_notifiers(action, uw->ndev, &info.info, NULL);
	}
	kfree(work);
}

static void rtl839x_l2_notification_handler(struct rtl838x_eth_priv *priv)
{
	struct notify_b *nb = priv->membase + sizeof(struct ring_b);
        u32 e = priv->lastEvent;

	while (!(nb->ring[e] & 1)) {
                struct fdb_update_work *w;
                struct n_event *event;
                u64 mac;
                int i;

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
	struct net_device *ndev = dev_id;
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);
	u32 status = sw_r32(priv->r->dma_if_intr_sts);

	netdev_dbg(ndev, "RX IRQ received: %08x\n", status);

	if ((status & RTL83XX_DMA_IF_INTR_STS_RX_RUN_OUT_MASK) && net_ratelimit())
		netdev_warn(ndev, "RX buffer overrun: status 0x%x, mask: 0x%x\n",
			   status, sw_r32(priv->r->dma_if_intr_msk));

	if (status & RTL83XX_DMA_IF_INTR_STS_RX_DONE_MASK) {
		/* Disable rx interrupts */
		sw_w32_mask(0xff00 & status, 0, priv->r->dma_if_intr_msk);
		for (int i = 0; i < priv->rxrings; i++) {
			if (status & BIT(i + 8)) {
				pr_debug("Scheduling queue: %d\n", i);
				napi_schedule(&priv->rx_qs[i].napi);
			}
		}
	}

	if ((status & RTL83XX_DMA_IF_INTR_STS_NOTIFY_MASK) && priv->family_id == RTL8390_FAMILY_ID)
		rtl839x_l2_notification_handler(priv);

	/* Acknowledge all interrupts */
	sw_w32(status, priv->r->dma_if_intr_sts);

	return IRQ_HANDLED;
}

static irqreturn_t rtl93xx_net_irq(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	u32 status_rx_r = sw_r32(priv->r->dma_if_intr_rx_runout_sts);
	u32 status_rx = sw_r32(priv->r->dma_if_intr_rx_done_sts);
	u32 status_tx = sw_r32(priv->r->dma_if_intr_tx_done_sts);

	pr_debug("In %s, status_tx: %08x, status_rx: %08x, status_rx_r: %08x\n",
		__func__, status_tx, status_rx, status_rx_r);

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
		for (int i = 0; i < priv->rxrings; i++) {
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

	return IRQ_HANDLED;
}

static const struct rtl838x_eth_reg rtl838x_reg = {
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

static const struct rtl838x_eth_reg rtl839x_reg = {
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

static const struct rtl838x_eth_reg rtl930x_reg = {
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

static const struct rtl838x_eth_reg rtl931x_reg = {
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
	u32 reset_mask;

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

	/* Reset NIC (SW_NIC_RST) and queues (SW_Q_RST) */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID)
		reset_mask = 0x6;
	else
		reset_mask = 0xc;

	sw_w32_mask(0, reset_mask, priv->r->rst_glb_ctrl);

	do { /* Wait for reset of NIC and Queues done */
		udelay(20);
	} while (sw_r32(priv->r->rst_glb_ctrl) & reset_mask);
	mdelay(100);

	/* Setup Head of Line */
	if (priv->family_id == RTL8380_FAMILY_ID)
		sw_w32(0, RTL838X_DMA_IF_RX_RING_SIZE);  /* Disabled on RTL8380 */
	if (priv->family_id == RTL8390_FAMILY_ID)
		sw_w32(0xffffffff, RTL839X_DMA_IF_RX_RING_CNTR);
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID) {
		for (int i = 0; i < priv->rxrings; i++) {
			int pos = (i % 3) * 10;

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
	struct ring_b *ring = priv->membase;

	for (int i = 0; i < priv->rxrings; i++)
		sw_w32(KSEG1ADDR(&ring->rx_r[i]), priv->r->dma_rx_base + i * 4);

	for (int i = 0; i < TXRINGS; i++)
		sw_w32(KSEG1ADDR(&ring->tx_r[i]), priv->r->dma_tx_base + i * 4);
}

static void rtl838x_hw_en_rxtx(struct rtl838x_eth_priv *priv)
{
	/* Disable Head of Line features for all RX rings */
	sw_w32(0xffffffff, priv->r->dma_if_rx_ring_size(0));

	/* Truncate RX buffer to DEFAULT_MTU bytes, pad TX */
	sw_w32((DEFAULT_MTU << 16) | RX_TRUNCATE_EN_83XX | TX_PAD_EN_838X, priv->r->dma_if_ctrl);

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
	sw_w32((DEFAULT_MTU << 5) | RX_TRUNCATE_EN_83XX, priv->r->dma_if_ctrl);

	/* Enable Notify, RX done, RX overflow and TX done interrupts */
	sw_w32(0x007fffff, priv->r->dma_if_intr_msk); /* Notify IRQ! */

	/* Enable DMA */
	sw_w32_mask(0, RX_EN | TX_EN, priv->r->dma_if_ctrl);

	/* Restart TX/RX to CPU port, enable CRC checking */
	sw_w32_mask(0x0, 0x3 | BIT(3), priv->r->mac_port_ctrl(priv->cpu_port));

	/* CPU port joins Lookup Miss Flooding Portmask */
	/* TODO: The code below should also work for the RTL838x */
	sw_w32(0x28000, RTL839X_TBL_ACCESS_L2_CTRL);
	sw_w32_mask(0, 0x80000000, RTL839X_TBL_ACCESS_L2_DATA(0));
	sw_w32(0x38000, RTL839X_TBL_ACCESS_L2_CTRL);

	/* Force CPU port link up */
	sw_w32_mask(0, 3, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static void rtl93xx_hw_en_rxtx(struct rtl838x_eth_priv *priv)
{
	/* Setup CPU-Port: RX Buffer truncated at DEFAULT_MTU Bytes */
	sw_w32((DEFAULT_MTU << 16) | RX_TRUNCATE_EN_93XX, priv->r->dma_if_ctrl);

	for (int i = 0; i < priv->rxrings; i++) {
		int pos = (i % 3) * 10;
		u32 v;

		sw_w32_mask(0x3ff << pos, priv->rxringlen << pos, priv->r->dma_if_rx_ring_size(i));

		/* Some SoCs have issues with missing underflow protection */
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

	if (priv->family_id == RTL9300_FAMILY_ID)
		sw_w32_mask(0, BIT(priv->cpu_port), RTL930X_L2_UNKN_UC_FLD_PMSK);
	else
		sw_w32_mask(0, BIT(priv->cpu_port), RTL931X_L2_UNKN_UC_FLD_PMSK);

	if (priv->family_id == RTL9300_FAMILY_ID)
		sw_w32(0x217, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
	else
		sw_w32(0x2a1d, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static void rtl838x_setup_ring_buffer(struct rtl838x_eth_priv *priv, struct ring_b *ring)
{
	for (int i = 0; i < priv->rxrings; i++) {
	        struct p_hdr *h;
                int j;

		for (j = 0; j < priv->rxringlen; j++) {
			h = &ring->rx_header[i][j];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->rx_space +
			                         i * priv->rxringlen * RING_BUFFER +
			                         j * RING_BUFFER);
			h->size = RING_BUFFER;
			/* All rings owned by switch, last one wraps */
			ring->rx_r[i][j] = KSEG1ADDR(h) | 1 | (j == (priv->rxringlen - 1) ?
			                   WRAP :
			                   0);
		}
		ring->c_rx[i] = 0;
	}

	for (int i = 0; i < TXRINGS; i++) {
		struct p_hdr *h;
		int j;

		for (j = 0; j < TXRINGLEN; j++) {
			h = &ring->tx_header[i][j];
			memset(h, 0, sizeof(struct p_hdr));
			h->buf = (u8 *)KSEG1ADDR(ring->tx_space +
			                         i * TXRINGLEN * RING_BUFFER +
			                         j * RING_BUFFER);
			h->size = RING_BUFFER;
			ring->tx_r[i][j] = KSEG1ADDR(&ring->tx_header[i][j]);
		}
		/* Last header is wrapping around */
		ring->tx_r[i][j - 1] |= WRAP;
		ring->c_tx[i] = 0;
	}
}

static void rtl839x_setup_notify_ring_buffer(struct rtl838x_eth_priv *priv)
{
	struct notify_b *b = priv->membase + sizeof(struct ring_b);

	for (int i = 0; i < NOTIFY_BLOCKS; i++)
		b->ring[i] = KSEG1ADDR(&b->blocks[i]) | 1 | (i == (NOTIFY_BLOCKS - 1) ? WRAP : 0);

	sw_w32((u32) b->ring, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL);
	sw_w32_mask(0x3ff << 2, 100 << 2, RTL839X_L2_NOTIFICATION_CTRL);

	/* Setup notification events */
	sw_w32_mask(0, 1 << 14, RTL839X_L2_CTRL_0); /* RTL8390_L2_CTRL_0_FLUSH_NOTIFY_EN */
	sw_w32_mask(0, 1 << 12, RTL839X_L2_NOTIFICATION_CTRL); /* SUSPEND_NOTIFICATION_EN */

	/* Enable Notification */
	sw_w32_mask(0, 1 << 0, RTL839X_L2_NOTIFICATION_CTRL);
	priv->lastEvent = 0;
}

static int rtl838x_eth_open(struct net_device *ndev)
{
	unsigned long flags;
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);
	struct ring_b *ring = priv->membase;

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
	phylink_start(priv->phylink);

	for (int i = 0; i < priv->rxrings; i++)
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
		/* Trap MLD and IGMP messages to CPU_PORT */
		sw_w32(0x3, RTL839X_SPCL_TRAP_IGMP_CTRL);
		/* Flush learned FDB entries on link down of a port */
		sw_w32_mask(0, BIT(7), RTL839X_L2_CTRL_0);
		break;

	case RTL9300_FAMILY_ID:
		rtl93xx_hw_en_rxtx(priv);
		/* Flush learned FDB entries on link down of a port */
		sw_w32_mask(0, BIT(7), RTL930X_L2_CTRL);
		/* Trap MLD and IGMP messages to CPU_PORT */
		sw_w32((0x2 << 3) | 0x2,  RTL930X_VLAN_APP_PKT_CTRL);
		break;

	case RTL9310_FAMILY_ID:
		rtl93xx_hw_en_rxtx(priv);

		/* Trap MLD and IGMP messages to CPU_PORT */
		sw_w32((0x2 << 3) | 0x2,  RTL931X_VLAN_APP_PKT_CTRL);

		/* Disable External CPU access to switch, clear EXT_CPU_EN */
		sw_w32_mask(BIT(2), 0, RTL931X_MAC_L2_GLOBAL_CTRL2);

		/* Set PCIE_PWR_DOWN */
		sw_w32_mask(0, BIT(1), RTL931X_PS_SOC_CTRL);
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

	/* Disable RX/TX from/to CPU-port */
	sw_w32_mask(0x3, 0, priv->r->mac_port_ctrl(priv->cpu_port));

	/* Disable traffic */
	if (priv->family_id == RTL9300_FAMILY_ID || priv->family_id == RTL9310_FAMILY_ID)
		sw_w32_mask(RX_EN_93XX | TX_EN_93XX, 0, priv->r->dma_if_ctrl);
	else
		sw_w32_mask(RX_EN | TX_EN, 0, priv->r->dma_if_ctrl);
	mdelay(200); /* Test, whether this is needed */

	/* Block all ports */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		sw_w32(0x03000000, RTL838X_TBL_ACCESS_DATA_0(0));
		sw_w32(0x00000000, RTL838X_TBL_ACCESS_DATA_0(1));
		sw_w32(1 << 15 | 2 << 12, RTL838X_TBL_ACCESS_CTRL_0);
	}

	/* Flush L2 address cache */
	if (priv->family_id == RTL8380_FAMILY_ID) {
		for (int i = 0; i <= priv->cpu_port; i++) {
			sw_w32(1 << 26 | 1 << 23 | i << 5, priv->r->l2_tbl_flush_ctrl);
			do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & (1 << 26));
		}
	} else if (priv->family_id == RTL8390_FAMILY_ID) {
		for (int i = 0; i <= priv->cpu_port; i++) {
			sw_w32(1 << 28 | 1 << 25 | i << 5, priv->r->l2_tbl_flush_ctrl);
			do { } while (sw_r32(priv->r->l2_tbl_flush_ctrl) & (1 << 28));
		}
	}
	/* TODO: L2 flush register is 64 bit on RTL931X and 930X */

	/* CPU-Port: Link down */
	if (priv->family_id == RTL8380_FAMILY_ID || priv->family_id == RTL8390_FAMILY_ID)
		sw_w32(force_mac, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
	else if (priv->family_id == RTL9300_FAMILY_ID)
		sw_w32_mask(0x3, 0, priv->r->mac_force_mode_ctrl + priv->cpu_port *4);
	else if (priv->family_id == RTL9310_FAMILY_ID)
		sw_w32_mask(BIT(0) | BIT(9), 0, priv->r->mac_force_mode_ctrl + priv->cpu_port *4);
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
	struct rtl838x_eth_priv *priv = netdev_priv(ndev);

	pr_info("in %s\n", __func__);

	phylink_stop(priv->phylink);
	rtl838x_hw_stop(priv);

	for (int i = 0; i < priv->rxrings; i++)
		napi_disable(&priv->rx_qs[i].napi);

	netif_tx_stop_all_queues(ndev);

	return 0;
}

static void rtl838x_eth_set_multicast_list(struct net_device *ndev)
{
	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(21, 0) = 0x3FFFFF
	 */
	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL838X_RMA_CTRL_0);
		sw_w32(0x0, RTL838X_RMA_CTRL_1);
	}
	if (ndev->flags & IFF_ALLMULTI)
		sw_w32(GENMASK(21, 0), RTL838X_RMA_CTRL_0);
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(GENMASK(21, 0), RTL838X_RMA_CTRL_0);
		sw_w32(0x7fff, RTL838X_RMA_CTRL_1);
	}
}

static void rtl839x_eth_set_multicast_list(struct net_device *ndev)
{
	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (!(ndev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		sw_w32(0x0, RTL839X_RMA_CTRL_0);
		sw_w32(0x0, RTL839X_RMA_CTRL_1);
		sw_w32(0x0, RTL839X_RMA_CTRL_2);
		sw_w32(0x0, RTL839X_RMA_CTRL_3);
	}
	if (ndev->flags & IFF_ALLMULTI) {
		sw_w32(GENMASK(31, 2), RTL839X_RMA_CTRL_0);
		sw_w32(GENMASK(31, 0), RTL839X_RMA_CTRL_1);
		sw_w32(GENMASK(31, 0), RTL839X_RMA_CTRL_2);
	}
	if (ndev->flags & IFF_PROMISC) {
		sw_w32(GENMASK(31, 2), RTL839X_RMA_CTRL_0);
		sw_w32(GENMASK(31, 0), RTL839X_RMA_CTRL_1);
		sw_w32(GENMASK(31, 0), RTL839X_RMA_CTRL_2);
		sw_w32(0x3ff, RTL839X_RMA_CTRL_3);
	}
}

static void rtl930x_eth_set_multicast_list(struct net_device *ndev)
{
	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (ndev->flags & (IFF_ALLMULTI | IFF_PROMISC)) {
		sw_w32(GENMASK(31, 2), RTL930X_RMA_CTRL_0);
		sw_w32(GENMASK(31, 0), RTL930X_RMA_CTRL_1);
		sw_w32(GENMASK(31, 0), RTL930X_RMA_CTRL_2);
	} else {
		sw_w32(0x0, RTL930X_RMA_CTRL_0);
		sw_w32(0x0, RTL930X_RMA_CTRL_1);
		sw_w32(0x0, RTL930X_RMA_CTRL_2);
	}
}

static void rtl931x_eth_set_multicast_list(struct net_device *ndev)
{
	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00.
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (ndev->flags & (IFF_ALLMULTI | IFF_PROMISC)) {
		sw_w32(GENMASK(31, 2), RTL931X_RMA_CTRL_0);
		sw_w32(GENMASK(31, 0), RTL931X_RMA_CTRL_1);
		sw_w32(GENMASK(31, 0), RTL931X_RMA_CTRL_2);
	} else {
		sw_w32(0x0, RTL931X_RMA_CTRL_0);
		sw_w32(0x0, RTL931X_RMA_CTRL_1);
		sw_w32(0x0, RTL931X_RMA_CTRL_2);
	}
}

static void rtl838x_eth_tx_timeout(struct net_device *ndev, unsigned int txqueue)
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
	int len;
	struct rtl838x_eth_priv *priv = netdev_priv(dev);
	struct ring_b *ring = priv->membase;
	int ret;
	unsigned long flags;
	struct p_hdr *h;
	int dest_port = -1;
	int q = skb_get_queue_mapping(skb) % TXRINGS;

	if (q) /* Check for high prio queue */
		pr_debug("SKB priority: %d\n", skb->priority);

	spin_lock_irqsave(&priv->lock, flags);
	len = skb->len;

	/* Check for DSA tagging at the end of the buffer */
	if (netdev_uses_dsa(dev) &&
	    skb->data[len - 4] == 0x80 &&
	    skb->data[len - 3] < priv->cpu_port &&
	    skb->data[len - 2] == 0x10 &&
	    skb->data[len - 1] == 0x00) {
		/* Reuse tag space for CRC if possible */
		dest_port = skb->data[len - 3];
		skb->data[len - 4] = skb->data[len - 3] = skb->data[len - 2] = skb->data[len - 1] = 0x00;
		len -= 4;
	}

	len += 4; /* Add space for CRC */

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
		/* On RTL8380 SoCs, small packet lengths being sent need adjustments */
		if (priv->family_id == RTL8380_FAMILY_ID) {
			if (len < ETH_ZLEN - 4)
				h->len -= 4;
		}

		if (dest_port >= 0)
			priv->r->create_tx_header(h, dest_port, skb->priority >> 1);

		/* Copy packet data to tx buffer */
		memcpy((void *)KSEG1ADDR(h->buf), skb->data, len);
		/* Make sure packet data is visible to ASIC */
		wmb();

		/* Hand over to switch */
		ring->tx_r[q][ring->c_tx[q]] |= 1;

		/* Before starting TX, prevent a Lextra bus bug on RTL8380 SoCs */
		if (priv->family_id == RTL8380_FAMILY_ID) {
			for (int i = 0; i < 10; i++) {
				u32 val = sw_r32(priv->r->dma_if_ctrl);
				if ((val & 0xc) == 0xc)
					break;
			}
		}

		/* Tell switch to send data */
		if (priv->family_id == RTL9310_FAMILY_ID || priv->family_id == RTL9300_FAMILY_ID) {
			/* Ring ID q == 0: Low priority, Ring ID = 1: High prio queue */
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

/* Return queue number for TX. On the RTL83XX, these queues have equal priority
 * so we do round-robin
 */
u16 rtl83xx_pick_tx_queue(struct net_device *dev, struct sk_buff *skb,
			  struct net_device *sb_dev)
{
	static u8 last = 0;

	last++;
	return last % TXRINGS;
}

/* Return queue number for TX. On the RTL93XX, queue 1 is the high priority queue
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
	LIST_HEAD(rx_list);
	unsigned long flags;
	int work_done = 0;
	u32	*last;
	bool dsa = netdev_uses_dsa(dev);

	pr_debug("---------------------------------------------------------- RX - %d\n", r);
	spin_lock_irqsave(&priv->lock, flags);
	last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));

	do {
		struct sk_buff *skb;
		struct dsa_tag tag;
		struct p_hdr *h;
		u8 *skb_data;
		u8 *data;
		int len;

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

		skb = netdev_alloc_skb(dev, len + 4);
		skb_reserve(skb, NET_IP_ALIGN);

		if (likely(skb)) {
			/* BUG: Prevent bug on RTL838x SoCs */
			if (priv->family_id == RTL8380_FAMILY_ID) {
				sw_w32(0xffffffff, priv->r->dma_if_rx_ring_size(0));
				for (int i = 0; i < priv->rxrings; i++) {
					unsigned int val;

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
				skb->data[len - 4] = 0x80;
				skb->data[len - 3] = tag.port;
				skb->data[len - 2] = 0x10;
				skb->data[len - 1] = 0x00;
				if (tag.l2_offloaded)
					skb->data[len - 3] |= 0x40;
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

			list_add_tail(&skb->list, &rx_list);
		} else {
			if (net_ratelimit())
				dev_warn(&dev->dev, "low on memory - packet dropped\n");
			dev->stats.rx_dropped++;
		}

		/* Reset header structure */
		memset(h, 0, sizeof(struct p_hdr));
		h->buf = data;
		h->size = RING_BUFFER;

		ring->rx_r[r][ring->c_rx[r]] = KSEG1ADDR(h) | 0x1 | (ring->c_rx[r] == (priv->rxringlen - 1) ?
		                               WRAP :
		                               0x1);
		ring->c_rx[r] = (ring->c_rx[r] + 1) % priv->rxringlen;
		last = (u32 *)KSEG1ADDR(sw_r32(priv->r->dma_if_rx_cur + r * 4));
	} while (&ring->rx_r[r][ring->c_rx[r]] != last && work_done < budget);

	netif_receive_skb_list(&rx_list);

	/* Update counters */
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

static void rtl838x_pcs_an_restart(struct phylink_pcs *pcs)
{
	struct rtl838x_eth_priv *priv = container_of(pcs, struct rtl838x_eth_priv, pcs);

	/* This works only on RTL838x chips */
	if (priv->family_id != RTL8380_FAMILY_ID)
		return;

	pr_debug("In %s\n", __func__);
	/* Restart by disabling and re-enabling link */
	sw_w32(0x6192D, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
	mdelay(20);
	sw_w32(0x6192F, priv->r->mac_force_mode_ctrl + priv->cpu_port * 4);
}

static void rtl838x_pcs_get_state(struct phylink_pcs *pcs,
				  struct phylink_link_state *state)
{
	u32 speed;
	struct rtl838x_eth_priv *priv = container_of(pcs, struct rtl838x_eth_priv, pcs);
	int port = priv->cpu_port;

	pr_info("In %s\n", __func__);

	state->link = priv->r->get_mac_link_sts(port) ? 1 : 0;
	state->duplex = priv->r->get_mac_link_dup_sts(port) ? 1 : 0;

	pr_info("%s link status is %d\n", __func__, state->link);
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
	case 5:
		state->speed = SPEED_2500;
		break;
	case 6:
		state->speed = SPEED_5000;
		break;
	case 4:
		state->speed = SPEED_10000;
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
}

static int rtl838x_pcs_config(struct phylink_pcs *pcs, unsigned int neg_mode,
			      phy_interface_t interface,
			      const unsigned long *advertising,
			      bool permit_pause_to_mac)
{
	return 0;
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

static void rtl838x_mac_link_up(struct phylink_config *config,
			    struct phy_device *phy, unsigned int mode,
			    phy_interface_t interface, int speed, int duplex,
			    bool tx_pause, bool rx_pause)
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

	dev_addr_set(dev, addr->sa_data);
	rtl838x_set_mac_hw(dev, mac);

	pr_info("Using MAC %08x%08x\n", sw_r32(priv->r->mac), sw_r32(priv->r->mac + 4));

	return 0;
}

static int rtl8390_init_mac(struct rtl838x_eth_priv *priv)
{
	/* We will need to set-up EEE and the egress-rate limitation */
	return 0;
}

static int rtl8380_init_mac(struct rtl838x_eth_priv *priv)
{
	if (priv->family_id == 0x8390)
		return rtl8390_init_mac(priv);

	/* At present we do not know how to set up EEE on any other SoC than RTL8380 */
	if (priv->family_id != 0x8380)
		return 0;

	pr_info("%s\n", __func__);
	/* fix timer for EEE */
	sw_w32(0x5001411, RTL838X_EEE_TX_TIMER_GIGA_CTRL);
	sw_w32(0x5001417, RTL838X_EEE_TX_TIMER_GELITE_CTRL);

	/* Init VLAN. TODO: Understand what is being done, here */
	if (priv->id == 0x8382) {
		for (int i = 0; i <= 28; i++)
			sw_w32(0, 0xd57c + i * 0x80);
	}
	if (priv->id == 0x8380) {
		for (int i = 8; i <= 28; i++)
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

/*
 * On all Realtek switch platforms the hardware periodically reads the link status of all
 * PHYs. This is to some degree programmable, so that one can tell the hardware to read
 * specific C22 registers from specific pages, or C45 registers, to determine the current
 * link speed, duplex, flow-control, ...
 *
 * This happens without any need for the driver to do anything at runtime, completely
 * invisible and in a parallel hardware thread, independent of the CPU running Linux.
 * All one needs to do is to set it up once. Having the MAC link settings automatically
 * follow the PHY link status also happens to be the only way to control MAC port status
 * in a meaningful way, or at least it's the only way we fully understand, as this is
 * what every vendor firmware is doing.
 *
 * The hardware PHY polling unit doesn't care about bus locking, it just assumes that all
 * paged PHY operations are also done via the same hardware unit offering this PHY access
 * abstractions.
 *
 * Additionally at least the RTL838x and RTL839x devices are known to have a so called
 * raw mode. Using the special MAX_PAGE-1 with the MDIO controller found in Realtek
 * SoCs allows to access the PHY in raw mode, ie. bypassing the cache and paging engine
 * of the MDIO controller. E.g. for RTL838x this is 0xfff.
 *
 * On the other hand Realtek PHYs usually make use of select register 0x1f to switch
 * pages. There is no problem to issue separate page and access bus calls to the PHYs
 * when they are not attached to an Realtek SoC. The paradigm should be to keep the PHY
 * implementation bus independent.
 *
 * As if this is not enough the PHY packages consist of 4 or 8 ports that all can be
 * programmed individually. Some registers are only available on port 0 and configure
 * the whole package.
 *
 * To bring all this together we need a tricky bus design that intercepts select page
 * calls but lets raw page accesses through. And especially knows how to handle raw
 * accesses to the select register. Additionally we need the possibility to write to
 * all 8 ports of the PHY individually.
 *
 * While the C45 clause stuff is pretty standard the legacy functions basically track
 * the accesses and the state of the bus with the attributes page[], raw[] and portaddr
 * of the bus_priv structure. The page selection works as follows:
 *
 * phy_write(phydev, RTMDIO_PAGE_SELECT, 12)	: store internal page 12 in driver
 * phy_write(phydev, 7, 33)			: write page=12, reg=7, val=33
 *
 * or simply
 *
 * phy_write_paged(phydev, 12, 7, 33)		: write page=12, reg=7, val=33
 *
 * The port selection works as follows and must be called under a held mdio bus lock
 *
 * __mdiobus_write(bus, RTMDIO_PORT_SELECT, 4)	: switch to port 4
 * __phy_write(phydev, RTMDIO_PAGE_SELECT, 11)	: store internal page 11 in driver
 * __phy_write(phydev, 8, 19)			: write page=11, reg=8, val=19, port=4
 *
 * Any Realtek PHY that will be connected to this bus must simply provide the standard
 * page functions:
 *
 * define RTL821X_PAGE_SELECT 0x1f
 *
 * static int rtl821x_read_page(struct phy_device *phydev)
 * {
 *   return __phy_read(phydev, RTL821X_PAGE_SELECT);
 * }
 *
 * static int rtl821x_write_page(struct phy_device *phydev, int page)
 * {
 *   return __phy_write(phydev, RTL821X_PAGE_SELECT, page);
 * }
 *
 * In case there are non Realtek PHYs attached to the bus the logic might need to be
 * reimplemented. For now it should be sufficient.
 */

#define RTMDIO_PAGE_SELECT	0x1f
#define RTMDIO_PORT_SELECT	0x2000
#define RTMDIO_READ		0x1
#define RTMDIO_WRITE		0x2
#define RTMDIO_ABS		0x4
#define RTMDIO_PKG		0x8

/*
 * Provide a generic read/write function so we can access arbitrary ports on the bus.
 * E.g. other ports of a PHY package on the bus. This basically resembles the kernel
 * phy_read_paged() and phy_write_paged() functions. To inform the bus that we are
 * working on a not default port send a RTMDIO_PORT_SELECT command at the beginning
 * and the end to switch the port handling logic.
 */

static int rtmdio_access(struct phy_device *phydev, int op, int port,
			 int page, u32 regnum, u16 val)
{
	int r, ret = 0, oldpage;

	if (op & RTMDIO_PKG) {
		if (!phydev->shared)
			return -EIO;
		port = phydev->shared->base_addr + port;
	}

	/* lock and inform bus about non default addressing */
	phy_lock_mdio_bus(phydev);
	__mdiobus_write(phydev->mdio.bus, phydev->mdio.addr,
			RTMDIO_PORT_SELECT, port);

	oldpage = ret = __phy_read(phydev, RTMDIO_PAGE_SELECT);
	if (oldpage >= 0 && oldpage != page) {
		ret = __phy_write(phydev, RTMDIO_PAGE_SELECT, page);
		if (ret < 0)
			oldpage = ret;
	}

	if (oldpage >= 0) {
		if (op & RTMDIO_WRITE)
			ret = __phy_write(phydev, regnum, val);
		else
			ret = __phy_read(phydev, regnum);
	}

	if (oldpage >= 0) {
		r = __phy_write(phydev, RTMDIO_PAGE_SELECT, oldpage);
		if (ret >= 0 && r < 0)
			ret = r;
	} else
		ret = oldpage;

	/* reset bus to default adressing and unlock it */
	__mdiobus_write(phydev->mdio.bus, phydev->mdio.addr,
			RTMDIO_PORT_SELECT, -1);
	phy_unlock_mdio_bus(phydev);

	return ret;
}

/*
 * To make use of the shared package functions provide wrappers that align with kernel
 * naming conventions. The package() functions are useful to change settings on the
 * package as a whole. The package_port() functions will allow to target the PHYs
 * of a package individually. The port() only functions allow to access arbitrary ports
 * on the bus through a PHY.
 */

int phy_package_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val)
{
	return rtmdio_access(phydev, RTMDIO_WRITE | RTMDIO_PKG, port, page, regnum, val);
}

int phy_package_write_paged(struct phy_device *phydev, int page, u32 regnum, u16 val)
{
	return rtmdio_access(phydev, RTMDIO_WRITE | RTMDIO_PKG, 0, page, regnum, val);
}

int phy_port_write_paged(struct phy_device *phydev, int port, int page, u32 regnum, u16 val)
{
	return rtmdio_access(phydev, RTMDIO_WRITE | RTMDIO_ABS, port, page, regnum, val);
}

int phy_package_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum)
{
	return rtmdio_access(phydev, RTMDIO_READ | RTMDIO_PKG, port, page, regnum, 0);
}

int phy_package_read_paged(struct phy_device *phydev, int page, u32 regnum)
{
	return rtmdio_access(phydev, RTMDIO_READ | RTMDIO_PKG, 0, page, regnum, 0);
}

int phy_port_read_paged(struct phy_device *phydev, int port, int page, u32 regnum)
{
	return rtmdio_access(phydev, RTMDIO_READ | RTMDIO_ABS, port, page, regnum, 0);
}

/* These are the core functions of our new Realtek SoC MDIO bus. */

static int rtmdio_read_c45(struct mii_bus *bus, int addr, int devnum, int regnum)
{
	int err, val;
	struct rtl838x_bus_priv *bus_priv = bus->priv;

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;

	err = (*bus_priv->read_mmd_phy)(addr, devnum, regnum, &val);
	pr_debug("rd_MMD(adr=%d, dev=%d, reg=%d) = %d, err = %d\n",
		 addr, devnum, regnum, val, err);
	return err ? err : val;
}

static int rtmdio_83xx_read(struct mii_bus *bus, int addr, int regnum)
{
	int err, val;
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *eth_priv = bus_priv->eth_priv;

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;

	if (addr >= 24 && addr <= 27 && eth_priv->id == 0x8380)
		return rtl838x_read_sds_phy(addr, regnum);

	if (eth_priv->family_id == RTL8390_FAMILY_ID && eth_priv->phy_is_internal[addr])
		return rtl839x_read_sds_phy(addr, regnum);

	if (regnum == RTMDIO_PAGE_SELECT && bus_priv->page[addr] != bus_priv->rawpage)
		return bus_priv->page[addr];

	bus_priv->raw[addr] = (bus_priv->page[addr] == bus_priv->rawpage);
	err = (*bus_priv->read_phy)(addr, bus_priv->page[addr], regnum, &val);
	pr_debug("rd_PHY(adr=%d, pag=%d, reg=%d) = %d, err = %d\n",
		 addr, bus_priv->page[addr], regnum, val, err);
	return err ? err : val;
}

static int rtmdio_93xx_read(struct mii_bus *bus, int addr, int regnum)
{
	int err, val;
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *eth_priv = bus_priv->eth_priv;

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;

	if (regnum == RTMDIO_PAGE_SELECT && bus_priv->page[addr] != bus_priv->rawpage)
		return bus_priv->page[addr];

	bus_priv->raw[addr] = (bus_priv->page[addr] == bus_priv->rawpage);
	if (eth_priv->phy_is_internal[addr]) {
		if (eth_priv->family_id == RTL9300_FAMILY_ID)
			return rtl930x_read_sds_phy(eth_priv->sds_id[addr],
						    bus_priv->page[addr], regnum);
		else
			return rtl931x_read_sds_phy(eth_priv->sds_id[addr],
						    bus_priv->page[addr], regnum);
	}

	err = (*bus_priv->read_phy)(addr, bus_priv->page[addr], regnum, &val);
	pr_debug("rd_PHY(adr=%d, pag=%d, reg=%d) = %d, err = %d\n",
		 addr, bus_priv->page[addr], regnum, val, err);
	return err ? err : val;
}

static int rtmdio_write_c45(struct mii_bus *bus, int addr, int devnum, int regnum, u16 val)
{
	int err;
	struct rtl838x_bus_priv *bus_priv = bus->priv;

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;

	err = (*bus_priv->write_mmd_phy)(addr, devnum, regnum, val);
	pr_debug("wr_MMD(adr=%d, dev=%d, reg=%d, val=%d) err = %d\n",
		 addr, devnum, regnum, val, err);
	return err;
}

static int rtmdio_83xx_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	int err, page, offset = 0;
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *eth_priv = bus_priv->eth_priv;

	if (regnum == RTMDIO_PORT_SELECT) {
		bus_priv->extaddr = (s16)val;
		return 0;
	}

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;
	page = bus_priv->page[addr];

	if (addr >= 24 && addr <= 27 && eth_priv->id == 0x8380) {
		if (addr == 26)
			offset = 0x100;
		sw_w32(val, RTL838X_SDS4_FIB_REG0 + offset + (regnum << 2));
		return 0;
	}

	if (eth_priv->family_id == RTL8390_FAMILY_ID && eth_priv->phy_is_internal[addr])
		return rtl839x_write_sds_phy(addr, regnum, val);

	if (regnum == RTMDIO_PAGE_SELECT)
		bus_priv->page[addr] = val;

	if (!bus_priv->raw[addr] && (regnum != RTMDIO_PAGE_SELECT || page == bus_priv->rawpage)) {
		bus_priv->raw[addr] = (page == bus_priv->rawpage);
		err = (*bus_priv->write_phy)(addr, page, regnum, val);
		pr_debug("wr_PHY(adr=%d, pag=%d, reg=%d, val=%d) err = %d\n",
			 addr, page, regnum, val, err);
		return err;
	}

	bus_priv->raw[addr] = false;
	return 0;
}

static int rtmdio_93xx_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	int err, page;
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *eth_priv = bus_priv->eth_priv;

	if (regnum == RTMDIO_PORT_SELECT) {
		bus_priv->extaddr = (s16)val;
		return 0;
	}

	if (bus_priv->extaddr >= 0)
		addr = bus_priv->extaddr;
	page = bus_priv->page[addr];

	if (regnum == RTMDIO_PAGE_SELECT)
		bus_priv->page[addr] = val;

	if (!bus_priv->raw[addr] && (regnum != RTMDIO_PAGE_SELECT || page == bus_priv->rawpage)) {
		bus_priv->raw[addr] = (page == bus_priv->rawpage);
		if (eth_priv->phy_is_internal[addr]) {
			if (eth_priv->family_id == RTL9300_FAMILY_ID)
				return rtl930x_write_sds_phy(eth_priv->sds_id[addr],
							     page, regnum, val);
			else
				return rtl931x_write_sds_phy(eth_priv->sds_id[addr],
							     page, regnum, val);
		}

		err = (*bus_priv->write_phy)(addr, page, regnum, val);
		pr_debug("wr_PHY(adr=%d, pag=%d, reg=%d, val=%d) err = %d\n",
			 addr, page, regnum, val, err);
	}

	bus_priv->raw[addr] = false;
	return 0;
}

static int rtmdio_838x_reset(struct mii_bus *bus)
{
	pr_debug("%s called\n", __func__);
	/* Disable MAC polling the PHY so that we can start configuration */
	sw_w32(0x00000000, RTL838X_SMI_POLL_CTRL);

	/* Enable PHY control via SoC */
	sw_w32_mask(0, 1 << 15, RTL838X_SMI_GLB_CTRL);

	/* Probably should reset all PHYs here... */
	return 0;
}

static int rtmdio_839x_reset(struct mii_bus *bus)
{
	return 0;

	pr_debug("%s called\n", __func__);
	/* BUG: The following does not work, but should! */
	/* Disable MAC polling the PHY so that we can start configuration */
	sw_w32(0x00000000, RTL839X_SMI_PORT_POLLING_CTRL);
	sw_w32(0x00000000, RTL839X_SMI_PORT_POLLING_CTRL + 4);
	/* Disable PHY polling via SoC */
	sw_w32_mask(1 << 7, 0, RTL839X_SMI_GLB_CTRL);

	/* Probably should reset all PHYs here... */
	return 0;
}

u8 mac_type_bit[RTL930X_CPU_PORT] = {0, 0, 0, 0, 2, 2, 2, 2, 4, 4, 4, 4, 6, 6, 6, 6,
				     8, 8, 8, 8, 10, 10, 10, 10, 12, 15, 18, 21};

static int rtmdio_930x_reset(struct mii_bus *bus)
{
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *priv = bus_priv->eth_priv;
	u32 c45_mask = 0;
	u32 poll_sel[2];
	u32 poll_ctrl = 0;
	u32 private_poll_mask = 0;
	u32 v;
	bool uses_usxgmii = false; /* For the Aquantia PHYs */
	bool uses_hisgmii = false; /* For the RTL8221/8226 */

	/* Mapping of port to phy-addresses on an SMI bus */
	poll_sel[0] = poll_sel[1] = 0;
	for (int i = 0; i < RTL930X_CPU_PORT; i++) {
		int pos;

		if (priv->smi_bus[i] < 0)
			continue;

		pos = (i % 6) * 5;
		sw_w32_mask(0x1f << pos, priv->smi_addr[i] << pos,
			    RTL930X_SMI_PORT0_5_ADDR + (i / 6) * 4);

		pos = (i * 2) % 32;
		poll_sel[i / 16] |= priv->smi_bus[i] << pos;
		poll_ctrl |= BIT(20 + priv->smi_bus[i]);
	}

	/* Configure which SMI bus is behind which port number */
	sw_w32(poll_sel[0], RTL930X_SMI_PORT0_15_POLLING_SEL);
	sw_w32(poll_sel[1], RTL930X_SMI_PORT16_27_POLLING_SEL);

	/* Disable POLL_SEL for any SMI bus with a normal PHY (not RTL8295R for SFP+) */
	sw_w32_mask(poll_ctrl, 0, RTL930X_SMI_GLB_CTRL);

	/* Configure which SMI busses are polled in c45 based on a c45 PHY being on that bus */
	for (int i = 0; i < 4; i++)
		if (priv->smi_bus_isc45[i])
			c45_mask |= BIT(i + 16);

	pr_info("c45_mask: %08x\n", c45_mask);
	sw_w32_mask(0, c45_mask, RTL930X_SMI_GLB_CTRL);

	/* Set the MAC type of each port according to the PHY-interface */
	/* Values are FE: 2, GE: 3, XGE/2.5G: 0(SERDES) or 1(otherwise), SXGE: 0 */
	v = 0;
	for (int i = 0; i < RTL930X_CPU_PORT; i++) {
		switch (priv->interfaces[i]) {
		case PHY_INTERFACE_MODE_10GBASER:
			break;			/* Serdes: Value = 0 */
		case PHY_INTERFACE_MODE_HSGMII:
			private_poll_mask |= BIT(i);
			fallthrough;
		case PHY_INTERFACE_MODE_USXGMII:
			v |= BIT(mac_type_bit[i]);
			uses_usxgmii = true;
			break;
		case PHY_INTERFACE_MODE_QSGMII:
			private_poll_mask |= BIT(i);
			v |= 3 << mac_type_bit[i];
			break;
		default:
			break;
		}
	}
	sw_w32(v, RTL930X_SMI_MAC_TYPE_CTRL);

	/* Set the private polling mask for all Realtek PHYs (i.e. not the 10GBit Aquantia ones) */
	sw_w32(private_poll_mask, RTL930X_SMI_PRVTE_POLLING_CTRL);

	/* The following magic values are found in the port configuration, they seem to
	 * define different ways of polling a PHY. The below is for the Aquantia PHYs of
	 * the XGS1250 and the RTL8226 of the XGS1210
	 */
	if (uses_usxgmii) {
		sw_w32(0x01010000, RTL930X_SMI_10GPHY_POLLING_REG0_CFG);
		sw_w32(0x01E7C400, RTL930X_SMI_10GPHY_POLLING_REG9_CFG);
		sw_w32(0x01E7E820, RTL930X_SMI_10GPHY_POLLING_REG10_CFG);
	}
	if (uses_hisgmii) {
		sw_w32(0x011FA400, RTL930X_SMI_10GPHY_POLLING_REG0_CFG);
		sw_w32(0x013FA412, RTL930X_SMI_10GPHY_POLLING_REG9_CFG);
		sw_w32(0x017FA414, RTL930X_SMI_10GPHY_POLLING_REG10_CFG);
	}

	pr_debug("%s: RTL930X_SMI_GLB_CTRL %08x\n", __func__,
		 sw_r32(RTL930X_SMI_GLB_CTRL));
	pr_debug("%s: RTL930X_SMI_PORT0_15_POLLING_SEL %08x\n", __func__,
		 sw_r32(RTL930X_SMI_PORT0_15_POLLING_SEL));
	pr_debug("%s: RTL930X_SMI_PORT16_27_POLLING_SEL %08x\n", __func__,
		 sw_r32(RTL930X_SMI_PORT16_27_POLLING_SEL));
	pr_debug("%s: RTL930X_SMI_MAC_TYPE_CTRL %08x\n", __func__,
		 sw_r32(RTL930X_SMI_MAC_TYPE_CTRL));
	pr_debug("%s: RTL930X_SMI_10GPHY_POLLING_REG0_CFG %08x\n", __func__,
		 sw_r32(RTL930X_SMI_10GPHY_POLLING_REG0_CFG));
	pr_debug("%s: RTL930X_SMI_10GPHY_POLLING_REG9_CFG %08x\n", __func__,
		 sw_r32(RTL930X_SMI_10GPHY_POLLING_REG9_CFG));
	pr_debug("%s: RTL930X_SMI_10GPHY_POLLING_REG10_CFG %08x\n", __func__,
		 sw_r32(RTL930X_SMI_10GPHY_POLLING_REG10_CFG));
	pr_debug("%s: RTL930X_SMI_PRVTE_POLLING_CTRL %08x\n", __func__,
		 sw_r32(RTL930X_SMI_PRVTE_POLLING_CTRL));

	return 0;
}

static int rtmdio_931x_reset(struct mii_bus *bus)
{
	struct rtl838x_bus_priv *bus_priv = bus->priv;
	struct rtl838x_eth_priv *priv = bus_priv->eth_priv;
	u32 c45_mask = 0;
	u32 poll_sel[4];
	u32 poll_ctrl = 0;
	bool mdc_on[4];

	pr_info("%s called\n", __func__);
	/* Disable port polling for configuration purposes */
	sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL);
	sw_w32(0, RTL931X_SMI_PORT_POLLING_CTRL + 4);
	msleep(100);

	mdc_on[0] = mdc_on[1] = mdc_on[2] = mdc_on[3] = false;
	/* Mapping of port to phy-addresses on an SMI bus */
	poll_sel[0] = poll_sel[1] = poll_sel[2] = poll_sel[3] = 0;
	for (int i = 0; i < RTL931X_CPU_PORT; i++) {
		u32 pos;

		if (priv->smi_bus[i] < 0)
			continue;

		pos = (i % 6) * 5;
		sw_w32_mask(0x1f << pos, priv->smi_addr[i] << pos, RTL931X_SMI_PORT_ADDR + (i / 6) * 4);
		pos = (i * 2) % 32;
		poll_sel[i / 16] |= priv->smi_bus[i] << pos;
		poll_ctrl |= BIT(20 + priv->smi_bus[i]);
		mdc_on[priv->smi_bus[i]] = true;
	}

	/* Configure which SMI bus is behind which port number */
	for (int i = 0; i < 4; i++) {
		pr_info("poll sel %d, %08x\n", i, poll_sel[i]);
		sw_w32(poll_sel[i], RTL931X_SMI_PORT_POLLING_SEL + (i * 4));
	}

	/* Configure which SMI busses */
	pr_info("%s: WAS RTL931X_MAC_L2_GLOBAL_CTRL2 %08x\n", __func__, sw_r32(RTL931X_MAC_L2_GLOBAL_CTRL2));
	pr_info("c45_mask: %08x, RTL931X_SMI_GLB_CTRL0 was %X", c45_mask, sw_r32(RTL931X_SMI_GLB_CTRL0));
	for (int i = 0; i < 4; i++) {
		/* bus is polled in c45 */
		if (priv->smi_bus_isc45[i])
			c45_mask |= 0x2 << (i * 2);  /* Std. C45, non-standard is 0x3 */
		/* Enable bus access via MDC */
		if (mdc_on[i])
			sw_w32_mask(0, BIT(9 + i), RTL931X_MAC_L2_GLOBAL_CTRL2);
	}

	pr_info("%s: RTL931X_MAC_L2_GLOBAL_CTRL2 %08x\n", __func__, sw_r32(RTL931X_MAC_L2_GLOBAL_CTRL2));
	pr_info("c45_mask: %08x, RTL931X_SMI_GLB_CTRL0 was %X", c45_mask, sw_r32(RTL931X_SMI_GLB_CTRL0));

	/* We have a 10G PHY enable polling
	 * sw_w32(0x01010000, RTL931X_SMI_10GPHY_POLLING_SEL2);
	 * sw_w32(0x01E7C400, RTL931X_SMI_10GPHY_POLLING_SEL3);
	 * sw_w32(0x01E7E820, RTL931X_SMI_10GPHY_POLLING_SEL4);
	 */
	sw_w32_mask(0xff, c45_mask, RTL931X_SMI_GLB_CTRL1);

	return 0;
}

static int rtl931x_chip_init(struct rtl838x_eth_priv *priv)
{
	pr_info("In %s\n", __func__);

	/* Initialize Encapsulation memory and wait until finished */
	sw_w32(0x1, RTL931X_MEM_ENCAP_INIT);
	do { } while (sw_r32(RTL931X_MEM_ENCAP_INIT) & 1);
	pr_info("%s: init ENCAP done\n", __func__);

	/* Initialize Managemen Information Base memory and wait until finished */
	sw_w32(0x1, RTL931X_MEM_MIB_INIT);
	do { } while (sw_r32(RTL931X_MEM_MIB_INIT) & 1);
	pr_info("%s: init MIB done\n", __func__);

	/* Initialize ACL (PIE) memory and wait until finished */
	sw_w32(0x1, RTL931X_MEM_ACL_INIT);
	do { } while (sw_r32(RTL931X_MEM_ACL_INIT) & 1);
	pr_info("%s: init ACL done\n", __func__);

	/* Initialize ALE memory and wait until finished */
	sw_w32(0xFFFFFFFF, RTL931X_MEM_ALE_INIT_0);
	do { } while (sw_r32(RTL931X_MEM_ALE_INIT_0));
	sw_w32(0x7F, RTL931X_MEM_ALE_INIT_1);
	sw_w32(0x7ff, RTL931X_MEM_ALE_INIT_2);
	do { } while (sw_r32(RTL931X_MEM_ALE_INIT_2) & 0x7ff);
	pr_info("%s: init ALE done\n", __func__);

	/* Enable ESD auto recovery */
	sw_w32(0x1, RTL931X_MDX_CTRL_RSVD);

	/* Init SPI, is this for thermal control or what? */
	sw_w32_mask(0x7 << 11, 0x2 << 11, RTL931X_SPI_CTRL0);

	return 0;
}

static int rtl838x_mdio_init(struct rtl838x_eth_priv *priv)
{
	struct device_node *mii_np, *dn;
	struct rtl838x_bus_priv *bus_priv;
	u32 pn;
	int i, ret;

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

	priv->mii_bus = devm_mdiobus_alloc_size(&priv->pdev->dev, sizeof(*bus_priv));
	if (!priv->mii_bus) {
		ret = -ENOMEM;
		goto err_put_node;
	}

	bus_priv = priv->mii_bus->priv;
	bus_priv->eth_priv = priv;
	for (i=0; i < 64; i++) {
		bus_priv->page[i] = 0;
		bus_priv->raw[i] = false;
	}
	bus_priv->extaddr = -1;

	switch(priv->family_id) {
	case RTL8380_FAMILY_ID:
		priv->mii_bus->name = "rtl838x-eth-mdio";
		priv->mii_bus->read = rtmdio_83xx_read;
		priv->mii_bus->write = rtmdio_83xx_write;
		priv->mii_bus->reset = rtmdio_838x_reset;
		bus_priv->read_mmd_phy = rtl838x_read_mmd_phy;
		bus_priv->write_mmd_phy = rtl838x_write_mmd_phy;
		bus_priv->read_phy = rtl838x_read_phy;
		bus_priv->write_phy = rtl838x_write_phy;
		bus_priv->rawpage = 0xfff;
		break;
	case RTL8390_FAMILY_ID:
		priv->mii_bus->name = "rtl839x-eth-mdio";
		priv->mii_bus->read = rtmdio_83xx_read;
		priv->mii_bus->write = rtmdio_83xx_write;
		priv->mii_bus->reset = rtmdio_839x_reset;
		bus_priv->read_mmd_phy = rtl839x_read_mmd_phy;
		bus_priv->write_mmd_phy = rtl839x_write_mmd_phy;
		bus_priv->read_phy = rtl839x_read_phy;
		bus_priv->write_phy = rtl839x_write_phy;
		bus_priv->rawpage = 0x1fff;
		break;
	case RTL9300_FAMILY_ID:
		priv->mii_bus->name = "rtl930x-eth-mdio";
		priv->mii_bus->read = rtmdio_93xx_read;
		priv->mii_bus->write = rtmdio_93xx_write;
		priv->mii_bus->reset = rtmdio_930x_reset;
		bus_priv->read_mmd_phy = rtl930x_read_mmd_phy;
		bus_priv->write_mmd_phy = rtl930x_write_mmd_phy;
		bus_priv->read_phy = rtl930x_read_phy;
		bus_priv->write_phy = rtl930x_write_phy;
		bus_priv->rawpage = 0xfff;
		break;
	case RTL9310_FAMILY_ID:
		priv->mii_bus->name = "rtl931x-eth-mdio";
		priv->mii_bus->read = rtmdio_93xx_read;
		priv->mii_bus->write = rtmdio_93xx_write;
		priv->mii_bus->reset = rtmdio_931x_reset;
		bus_priv->read_mmd_phy = rtl931x_read_mmd_phy;
		bus_priv->write_mmd_phy = rtl931x_write_mmd_phy;
		bus_priv->read_phy = rtl931x_read_phy;
		bus_priv->write_phy = rtl931x_write_phy;
		bus_priv->rawpage = 0x1fff;
		break;
	}
	priv->mii_bus->read_c45 = rtmdio_read_c45;
	priv->mii_bus->write_c45 = rtmdio_write_c45;
	priv->mii_bus->parent = &priv->pdev->dev;

	for_each_node_by_name(dn, "ethernet-phy") {
		u32 smi_addr[2];

		if (of_property_read_u32(dn, "reg", &pn))
			continue;

		if (pn >= MAX_PORTS) {
			pr_err("%s: illegal port number %d\n", __func__, pn);
			return -ENODEV;
		}

		if (of_property_read_u32(dn, "sds", &priv->sds_id[pn]))
			priv->sds_id[pn] = -1;
		else
			pr_info("set sds port %d to %d\n", pn, priv->sds_id[pn]);

		if (of_property_read_u32_array(dn, "rtl9300,smi-address", &smi_addr[0], 2)) {
			priv->smi_bus[pn] = 0;
			priv->smi_addr[pn] = pn;
		} else {
			priv->smi_bus[pn] = smi_addr[0];
			priv->smi_addr[pn] = smi_addr[1];
		}

		if (priv->smi_bus[pn] >= MAX_SMI_BUSSES) {
			pr_err("%s: illegal SMI bus number %d\n", __func__, priv->smi_bus[pn]);
			return -ENODEV;
		}

		priv->phy_is_internal[pn] = of_property_read_bool(dn, "phy-is-integrated");

		if (priv->phy_is_internal[pn] && priv->sds_id[pn] >= 0)
			priv->smi_bus[pn]= -1;
		else if (of_device_is_compatible(dn, "ethernet-phy-ieee802.3-c45"))
			priv->smi_bus_isc45[priv->smi_bus[pn]] = true;
	}

	dn = of_find_compatible_node(NULL, NULL, "realtek,rtl83xx-switch");
	if (!dn) {
		dev_err(&priv->pdev->dev, "No RTL switch node in DTS\n");
		return -ENODEV;
	}

	for_each_node_by_name(dn, "port") {
		if (of_property_read_u32(dn, "reg", &pn))
			continue;
		pr_debug("%s Looking at port %d\n", __func__, pn);
		if (pn > priv->cpu_port)
			continue;
		if (of_get_phy_mode(dn, &priv->interfaces[pn]))
			priv->interfaces[pn] = PHY_INTERFACE_MODE_NA;
		pr_debug("%s phy mode of port %d is %s\n", __func__, pn, phy_modes(priv->interfaces[pn]));
	}

	snprintf(priv->mii_bus->id, MII_BUS_ID_SIZE, "%pOFn", mii_np);
	ret = devm_of_mdiobus_register(&priv->pdev->dev, priv->mii_bus, mii_np);

err_put_node:
	of_node_put(mii_np);

	return ret;
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
			sw_w32_mask(0, BIT(3), priv->r->mac_port_ctrl(priv->cpu_port));
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

static struct phylink_pcs *rtl838x_mac_select_pcs(struct phylink_config *config,
						  phy_interface_t interface)
{
	struct net_device *dev = to_net_dev(config->dev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	return &priv->pcs;
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
	.ndo_setup_tc = rtl83xx_setup_tc,
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
	.ndo_setup_tc = rtl83xx_setup_tc,
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
	.ndo_setup_tc = rtl83xx_setup_tc,
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

static const struct phylink_pcs_ops rtl838x_pcs_ops = {
	.pcs_get_state = rtl838x_pcs_get_state,
	.pcs_an_restart = rtl838x_pcs_an_restart,
	.pcs_config = rtl838x_pcs_config,
};

static const struct phylink_mac_ops rtl838x_phylink_ops = {
	.validate = rtl838x_validate,
	.mac_select_pcs = rtl838x_mac_select_pcs,
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
	phy_interface_t phy_mode;
	struct phylink *phylink;
	u8 mac_addr[ETH_ALEN];
	int err = 0, rxrings, rxringlen;
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

	dev = devm_alloc_etherdev_mqs(&pdev->dev, sizeof(struct rtl838x_eth_priv), TXRINGS, rxrings);
	if (!dev)
		return -ENOMEM;
	SET_NETDEV_DEV(dev, &pdev->dev);
	priv = netdev_priv(dev);

	/* obtain buffer memory space */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res) {
		mem = devm_request_mem_region(&pdev->dev, res->start,
			resource_size(res), res->name);
		if (!mem) {
			dev_err(&pdev->dev, "cannot request memory space\n");
			return -ENXIO;
		}

		dev->mem_start = mem->start;
		dev->mem_end   = mem->end;
	} else {
		dev_err(&pdev->dev, "cannot request IO resource\n");
		return -ENXIO;
	}

	/* Allocate buffer memory */
	priv->membase = dmam_alloc_coherent(&pdev->dev, rxrings * rxringlen * RING_BUFFER +
	                                    sizeof(struct ring_b) + sizeof(struct notify_b),
	                                    (void *)&dev->mem_start, GFP_KERNEL);
	if (!priv->membase) {
		dev_err(&pdev->dev, "cannot allocate DMA buffer\n");
		return -ENOMEM;
	}

	/* Allocate ring-buffer space at the end of the allocated memory */
	ring = priv->membase;
	ring->rx_space = priv->membase + sizeof(struct ring_b) + sizeof(struct notify_b);

	spin_lock_init(&priv->lock);

	dev->ethtool_ops = &rtl838x_ethtool_ops;
	dev->min_mtu = ETH_ZLEN;
	dev->max_mtu = DEFAULT_MTU;
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
		rtl931x_chip_init(priv);
		break;
	default:
		pr_err("Unknown SoC family\n");
		return -ENODEV;
	}
	priv->rxringlen = rxringlen;
	priv->rxrings = rxrings;

	/* Obtain device IRQ number */
	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0) {
		dev_err(&pdev->dev, "cannot obtain network-device IRQ\n");
		return err;
	}

	err = devm_request_irq(&pdev->dev, dev->irq, priv->r->net_irq,
			       IRQF_SHARED, dev->name, dev);
	if (err) {
		dev_err(&pdev->dev, "%s: could not acquire interrupt: %d\n",
			   __func__, err);
		return err;
	}

	rtl8380_init_mac(priv);

	/* Try to get mac address in the following order:
	 * 1) from device tree data
	 * 2) from internal registers set by bootloader
	 */
	of_get_mac_address(pdev->dev.of_node, mac_addr);
	if (is_valid_ether_addr(mac_addr)) {
		rtl838x_set_mac_hw(dev, mac_addr);
	} else {
		mac_addr[0] = (sw_r32(priv->r->mac) >> 8) & 0xff;
		mac_addr[1] = sw_r32(priv->r->mac) & 0xff;
		mac_addr[2] = (sw_r32(priv->r->mac + 4) >> 24) & 0xff;
		mac_addr[3] = (sw_r32(priv->r->mac + 4) >> 16) & 0xff;
		mac_addr[4] = (sw_r32(priv->r->mac + 4) >> 8) & 0xff;
		mac_addr[5] = sw_r32(priv->r->mac + 4) & 0xff;
	}
	dev_addr_set(dev, mac_addr);
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
		return err;

	err = devm_register_netdev(&pdev->dev, dev);
	if (err)
		return err;

	for (int i = 0; i < priv->rxrings; i++) {
		priv->rx_qs[i].id = i;
		priv->rx_qs[i].priv = priv;
		netif_napi_add(dev, &priv->rx_qs[i].napi, rtl838x_poll_rx);
	}

	platform_set_drvdata(pdev, dev);

	phy_mode = PHY_INTERFACE_MODE_NA;
	err = of_get_phy_mode(dn, &phy_mode);
	if (err < 0) {
		dev_err(&pdev->dev, "incorrect phy-mode\n");
		return -EINVAL;
	}

	priv->pcs.ops = &rtl838x_pcs_ops;
	priv->phylink_config.dev = &dev->dev;
	priv->phylink_config.type = PHYLINK_NETDEV;
	__set_bit(PHY_INTERFACE_MODE_INTERNAL, priv->phylink_config.supported_interfaces);

	phylink = phylink_create(&priv->phylink_config, pdev->dev.fwnode,
				 phy_mode, &rtl838x_phylink_ops);

	if (IS_ERR(phylink))
		return PTR_ERR(phylink);
	priv->phylink = phylink;

	return 0;
}

static int rtl838x_eth_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct rtl838x_eth_priv *priv = netdev_priv(dev);

	if (dev) {
		pr_info("Removing platform driver for rtl838x-eth\n");
		rtl838x_hw_stop(priv);

		netif_tx_stop_all_queues(dev);

		for (int i = 0; i < priv->rxrings; i++)
			netif_napi_del(&priv->rx_qs[i].napi);
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
