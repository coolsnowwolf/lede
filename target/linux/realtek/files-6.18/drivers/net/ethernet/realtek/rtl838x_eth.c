// SPDX-License-Identifier: GPL-2.0-only
/* linux/drivers/net/ethernet/rtl838x_eth.c
 * Copyright (C) 2020 B. Koblitz
 */

#include <linux/cacheflush.h>
#include <linux/dma-mapping.h>
#include <linux/etherdevice.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/mfd/syscon.h>
#include <linux/minmax.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_net.h>
#include <linux/of_mdio.h>
#include <linux/module.h>
#include <linux/phylink.h>
#include <linux/pkt_sched.h>
#include <linux/regmap.h>
#include <linux/rtnetlink.h>
#include <linux/workqueue.h>
#include <net/dsa.h>
#include <net/dst_metadata.h>
#include <net/page_pool/helpers.h>
#include <net/switchdev.h>

#include "rtl838x_eth.h"

#define RING_OWN_HW			BIT(0)
#define RING_WRAP			BIT(1)

#define RTETH_RX_RING_SIZE		128
#define RTETH_RX_RINGS			2
#define RTETH_TX_RING_SIZE		16
#define RTETH_TX_RINGS			2
#define RTETH_TX_TRIGGER(ctrl, ring)	((0x16 >> ring) & ctrl->r->tx_trigger_mask)

#define NOTIFY_EVENTS			10
#define NOTIFY_BLOCKS			10

#define RX_TRUNCATE_EN_93XX		BIT(6)
#define RX_TRUNCATE_EN_83XX		BIT(4)
#define TX_PAD_EN_838X			BIT(5)

/* Ethernet header, two stacked VLAN tags (802.1ad QinQ) and FCS */
#define RTETH_FRAME_OVERHEAD		(ETH_HLEN + 2 * VLAN_HLEN + ETH_FCS_LEN)
/* Largest frame each family switches, as its datasheet and DSA rmon range have it */
#define RTETH_838X_MAX_FRAME		10000
#define RTETH_839X_MAX_FRAME		12288
#define RTETH_930X_MAX_FRAME		12288
#define RTETH_931X_MAX_FRAME		12288
#define SKB_FRAG_SIZE			1568
#define SKB_PAD				MAX(32, L1_CACHE_BYTES)
#define SKB_HEADROOM_FAST		(SKB_PAD + NET_IP_ALIGN)
#define SKB_HEADROOM_SLOW		SKB_PAD

/* Define page pool that holds 2KB fragments in 4KB pages and has 8 safety pages */
#define PPOOL_FRAG_SIZE			2048
#define PPOOL_SIZE			(DIV_ROUND_UP(RTETH_RX_RING_SIZE, \
					 PAGE_SIZE / PPOOL_FRAG_SIZE) + 8)

struct rteth_frag {
	/* hardware header part as required by SoC */
	dma_addr_t		dma;
	u16			reserved;
	u16			size;
	u16			more:1;
	u16			offset:15;
	u16			len;
	u16			cpu_tag[10];
} __packed __aligned(1);

/* SOC/driver shared coherent ring descriptors */
struct rteth_rx_data {
	dma_addr_t		ring[RTETH_RX_RING_SIZE];
	struct rteth_frag	frag[RTETH_RX_RING_SIZE];
};

struct rteth_tx_data {
	dma_addr_t		ring[RTETH_TX_RING_SIZE];
	struct rteth_frag	frag[RTETH_TX_RING_SIZE];
};

/* driver-only ring descriptors */
struct rteth_rx_info {
	int			id;
	int			slot;
	struct rteth_ctrl	*ctrl;
	struct napi_struct	napi;
	struct page_pool	*pool;
	struct sk_buff		*skb; /* unprocessed SKB from last receive loop */
	struct page		*page[RTETH_RX_RING_SIZE];
	unsigned int		offset[RTETH_RX_RING_SIZE];
};

struct rteth_tx_info {
	unsigned int		send_count;  /* skbs handed to the hardware */
	unsigned int		clean_count; /* skbs released after completion */
	struct sk_buff		*skb[RTETH_TX_RING_SIZE];
};

struct n_event {
	u32	type:2;
	u32	fidVid:12;
	u64	mac:48;
	u32	slp:6;
	u32	valid:1;
	u32	reserved:27;
} __packed __aligned(1);

struct notify_block {
	struct n_event	events[NOTIFY_EVENTS];
};

struct notify_b {
	struct notify_block	blocks[NOTIFY_BLOCKS];
	u32			reserved1[8];
	u32			ring[NOTIFY_BLOCKS];
	u32			reserved2[8];
};

struct rteth_ctrl {
	struct regmap *map;
	struct net_device *dev;
	struct platform_device *pdev;
	void *membase;
	spinlock_t lock;
	struct mii_bus *mii_bus;
	struct phylink *phylink;
	struct phylink_config phylink_config;
	const struct rteth_config *r;
	u32 lastEvent;
	struct metadata_dst *dsa_meta[RTETH_931X_CPU_PORT];
	/* receive handling */
	dma_addr_t		rx_dma;
	spinlock_t		rx_lock;
	struct rteth_rx_info	rx_info[RTETH_RX_RINGS];
	struct rteth_rx_data	*rx_data;
	bool			napi_enabled;
	/* transmit handling */
	dma_addr_t		tx_dma;
	spinlock_t		tx_lock;
	struct rteth_tx_info	tx_info[RTETH_TX_RINGS];
	struct rteth_tx_data	*tx_data;
	struct work_struct	reset_work;
};

static void rteth_838x_create_tx_header(struct rteth_frag *frag, unsigned int port, int prio)
{
	/* cpu_tag[0] is reserved on the RTL83XX SoCs */
	frag->cpu_tag[1] = 0x0400;  /* BIT 10: RTL8380_CPU_TAG */
	frag->cpu_tag[2] = 0x0200;  /* Set only AS_DPM, to enable DPM settings below */
	frag->cpu_tag[3] = 0x0000;
	frag->cpu_tag[4] = BIT(port) >> 16;
	frag->cpu_tag[5] = BIT(port) & 0xffff;

	/* Set internal priority (PRI) and enable (AS_PRI) */
	if (prio >= 0)
		frag->cpu_tag[2] |= ((prio & 0x7) | BIT(3)) << 12;
}

static void rteth_839x_create_tx_header(struct rteth_frag *frag, unsigned int port, int prio)
{
	/* cpu_tag[0] is reserved on the RTL83XX SoCs */
	frag->cpu_tag[1] = 0x0100; /* RTL8390_CPU_TAG marker */
	frag->cpu_tag[2] = BIT(4); /* AS_DPM flag */
	frag->cpu_tag[3] = frag->cpu_tag[4] = frag->cpu_tag[5] = 0;
	/* h->cpu_tag[1] |= BIT(1) | BIT(0); */ /* Bypass filter 1/2 */
	if (port >= 32) {
		port -= 32;
		frag->cpu_tag[2] |= (BIT(port) >> 16) & 0xf;
		frag->cpu_tag[3] = BIT(port) & 0xffff;
	} else {
		frag->cpu_tag[4] = BIT(port) >> 16;
		frag->cpu_tag[5] = BIT(port) & 0xffff;
	}

	/* Set internal priority (PRI) and enable (AS_PRI) */
	if (prio >= 0)
		frag->cpu_tag[2] |= ((prio & 0x7) | BIT(3)) << 8;
}

static void rteth_93xx_create_tx_header(struct rteth_frag *frag, unsigned int port, int prio)
{
	frag->cpu_tag[0] = 0x8000;  /* CPU tag marker */
	frag->cpu_tag[1] = FIELD_PREP(RTL93XX_CPU_TAG1_FWD_MASK, RTL93XX_CPU_TAG1_FWD_PHYSICAL) |
			   FIELD_PREP(RTL93XX_CPU_TAG1_IGNORE_STP_MASK, 1);

	frag->cpu_tag[2] = (prio >= 0) ? (BIT(5) | (prio & 0x1f)) << 8 : 0;
	frag->cpu_tag[3] = 0;
	frag->cpu_tag[4] = BIT_ULL(port) >> 48;
	frag->cpu_tag[5] = BIT_ULL(port) >> 32;
	frag->cpu_tag[6] = BIT_ULL(port) >> 16;
	frag->cpu_tag[7] = BIT_ULL(port) & 0xffff;
}

static int rteth_free_skb(struct sk_buff **skb)
{
	if (*skb) {
		dev_kfree_skb_any(*skb);
		*skb = NULL;
		return 1;
	}

	return 0;
}

static void rteth_83xx_enable_rx_irq(struct rteth_ctrl *ctrl, int ring)
{
	unsigned long flags;

	spin_lock_irqsave(&ctrl->lock, flags);
	regmap_set_bits(ctrl->map, ctrl->r->dma_if_intr_msk, BIT(ring) | BIT(ring + 8));
	spin_unlock_irqrestore(&ctrl->lock, flags);
}

static void rteth_93xx_enable_rx_irq(struct rteth_ctrl *ctrl, int ring)
{
	unsigned long flags;

	spin_lock_irqsave(&ctrl->lock, flags);
	regmap_set_bits(ctrl->map, ctrl->r->dma_if_intr_msk, BIT(ring));
	regmap_set_bits(ctrl->map, ctrl->r->dma_if_intr_msk + 4, BIT(ring));
	spin_unlock_irqrestore(&ctrl->lock, flags);
}

static void rteth_83xx_confirm_and_disable_irqs(struct rteth_ctrl *ctrl,
						unsigned long *rings, bool *l2)
{
	unsigned long flags;
	u32 disable, state;

	spin_lock_irqsave(&ctrl->lock, flags);

	regmap_read(ctrl->map, ctrl->r->dma_if_intr_sts, &state);
	*rings = FIELD_GET(GENMASK(7, 0), state) | FIELD_GET(GENMASK(15, 8), state);
	*l2 = !!(state & RTL839X_DMA_IF_INTR_NOTIFY_MASK);
	disable = FIELD_PREP(GENMASK(7, 0), *rings) | FIELD_PREP(GENMASK(15, 8), *rings);

	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_intr_msk, disable);
	regmap_write(ctrl->map, ctrl->r->dma_if_intr_sts, state);

	spin_unlock_irqrestore(&ctrl->lock, flags);
}

static void rteth_93xx_confirm_and_disable_irqs(struct rteth_ctrl *ctrl,
						unsigned long *rings, bool *l2)
{
	u32 state_done, state_runout;
	unsigned long flags;

	spin_lock_irqsave(&ctrl->lock, flags);

	regmap_read(ctrl->map, ctrl->r->dma_if_intr_sts, &state_runout);
	regmap_read(ctrl->map, ctrl->r->dma_if_intr_sts + 4, &state_done);
	*rings = state_runout | state_done;
	*l2 = false;

	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_intr_msk, *rings);
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_intr_msk + 4, *rings);
	regmap_write(ctrl->map, ctrl->r->dma_if_intr_sts, state_runout);
	regmap_write(ctrl->map, ctrl->r->dma_if_intr_sts + 4, state_done);

	spin_unlock_irqrestore(&ctrl->lock, flags);
}

static void rteth_disable_all_irqs(struct rteth_ctrl *ctrl)
{
	int registers = DIV_ROUND_UP(ctrl->r->rx_rings * 2 + 7, 32);

	for (int reg = 0; reg < registers; reg++) {
		regmap_write(ctrl->map, ctrl->r->dma_if_intr_msk + reg * 4, 0);
		regmap_write(ctrl->map, ctrl->r->dma_if_intr_sts + reg * 4, GENMASK(31, 0));
	}
}

static void rteth_enable_all_rx_irqs(struct rteth_ctrl *ctrl)
{
	int mask, reg;

	for (int ring = 0; ring < RTETH_RX_RINGS; ring++)
		ctrl->r->enable_rx_irq(ctrl, ring);

	/*
	 * RTL839x has additional L2 notification interrupts. Simply activate them. All other
	 * devices that do not have the feature have adequate reserved bit space and ignore it.
	 */
	mask = GENMASK(2, 0) << ((ctrl->r->rx_rings * 2 + 4) % 32);
	reg = (ctrl->r->rx_rings * 2 + 4) / 32;
	regmap_update_bits(ctrl->map, ctrl->r->dma_if_intr_msk + reg * 4, mask, mask);
}

static void rteth_83xx_update_counter(struct rteth_ctrl *ctrl, int ring, int released)
{
	/* Free floating rings without space tracking */
}

static void rteth_93xx_update_counter(struct rteth_ctrl *ctrl, int ring, int released)
{
	int shift = (ring % 3) * 10;
	int reg = (ring / 3) * 4;

	/* writing x to the ring counter increases ring free space by x */
	regmap_write(ctrl->map, ctrl->r->dma_if_rx_ring_cntr + reg, released << shift);
}

struct dsa_tag {
	u8	reason;
	u8	queue;
	u16	port;
	u8	l2_offloaded;
	u8	prio;
	bool	crc_error;
};

static bool rteth_838x_decode_tag(struct rteth_frag *frag, struct dsa_tag *t)
{
	/* cpu_tag[0] is reserved. Fields are off-by-one */
	t->reason = frag->cpu_tag[4] & 0xf;
	t->queue = (frag->cpu_tag[1] & 0xe0) >> 5;
	t->port = frag->cpu_tag[1] & 0x1f;
	t->crc_error = t->reason == 13;

	pr_debug("Reason: %d\n", t->reason);
	if (t->reason != 6) /* NIC_RX_REASON_SPECIAL_TRAP */
		t->l2_offloaded = 1;
	else
		t->l2_offloaded = 0;

	return t->l2_offloaded;
}

static bool rteth_839x_decode_tag(struct rteth_frag *frag, struct dsa_tag *t)
{
	/* cpu_tag[0] is reserved. Fields are off-by-one */
	t->reason = frag->cpu_tag[5] & 0x1f;
	t->queue = (frag->cpu_tag[4] & 0xe000) >> 13;
	t->port = frag->cpu_tag[1] & 0x3f;
	t->crc_error = frag->cpu_tag[4] & BIT(6);

	pr_debug("Reason: %d\n", t->reason);
	if ((t->reason >= 7 && t->reason <= 13) || /* NIC_RX_REASON_RMA */
	    (t->reason >= 23 && t->reason <= 25))  /* NIC_RX_REASON_SPECIAL_TRAP */
		t->l2_offloaded = 0;
	else
		t->l2_offloaded = 1;

	return t->l2_offloaded;
}

static bool rteth_93xx_decode_tag(struct rteth_frag *frag, struct dsa_tag *t)
{
	t->port = (frag->cpu_tag[0] >> 8) & 0x3f;
	t->queue = (frag->cpu_tag[2] >> 11) & 0x1f;
	t->reason = frag->cpu_tag[7] & 0x3f;
	t->crc_error = frag->cpu_tag[1] & BIT(6);
	t->l2_offloaded = (t->reason >= 19 && t->reason <= 27) ? 0 : 1;

	if (t->reason != 63)
		pr_debug("%s: Reason %d, port %d, queue %d\n", __func__, t->reason, t->port, t->queue);

	return t->l2_offloaded;
}

struct fdb_update_work {
	struct work_struct work;
	struct net_device *dev;
	u64 macs[NOTIFY_EVENTS + 1];
};

static void rtl838x_fdb_sync(struct work_struct *work)
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
		call_switchdev_notifiers(action, uw->dev, &info.info, NULL);
	}
	kfree(work);
}

static void rtl839x_l2_notification_handler(struct rteth_ctrl *ctrl)
{
	struct notify_b *nb = ctrl->membase;
	u32 e = ctrl->lastEvent;

	while (!(nb->ring[e] & 1)) {
		struct fdb_update_work *w;
		struct n_event *event;
		u64 mac;
		int i;

		w = kzalloc(sizeof(*w), GFP_ATOMIC);
		if (!w)
			return;

		INIT_WORK(&w->work, rtl838x_fdb_sync);

		for (i = 0; i < NOTIFY_EVENTS; i++) {
			event = &nb->blocks[e].events[i];
			if (!event->valid)
				continue;
			mac = event->mac;
			if (event->type)
				mac |= 1ULL << 63;
			w->dev = ctrl->dev;
			w->macs[i] = mac;
		}

		/* Hand the ring entry back to the switch */
		nb->ring[e] = nb->ring[e] | 1;
		e = (e + 1) % NOTIFY_BLOCKS;

		w->macs[i] = 0ULL;
		schedule_work(&w->work);
	}
	ctrl->lastEvent = e;
}

static irqreturn_t rteth_net_irq(int irq, void *dev_id)
{
	struct net_device *dev = dev_id;
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	unsigned long ring, rings;
	bool l2;

	ctrl->r->confirm_and_disable_irqs(ctrl, &rings, &l2);
	for_each_set_bit(ring, &rings, RTETH_RX_RINGS) {
		netdev_dbg(dev, "schedule rx ring %lu\n", ring);
		napi_schedule(&ctrl->rx_info[ring].napi);
	}

	if (unlikely(l2))
		rtl839x_l2_notification_handler(ctrl);

	return IRQ_HANDLED;
}

static void rteth_nic_reset(struct rteth_ctrl *ctrl, int reset_mask)
{
	int val;

	pr_info("RESETTING CPU_PORT %d\n", ctrl->r->cpu_port);
	regmap_update_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3, 0x0);
	msleep(100);

	/* Reset NIC (SW_NIC_RST) and queues (SW_Q_RST) */
	regmap_update_bits(ctrl->map, ctrl->r->rst_glb_ctrl, reset_mask, reset_mask);
	regmap_read_poll_timeout(ctrl->map, ctrl->r->rst_glb_ctrl, val,
				 !(val & reset_mask), 1000, 1000000);

	msleep(100);
}

static void rteth_83xx_set_hol(struct rteth_ctrl *ctrl)
{
	/* Free floating rings without space tracking */
	regmap_write(ctrl->map, ctrl->r->dma_if_rx_ring_size, 0);
}

static void rteth_838x_hw_reset(struct rteth_ctrl *ctrl)
{
	rteth_nic_reset(ctrl, 0xc);
}

static void rteth_839x_hw_reset(struct rteth_ctrl *ctrl)
{
	u32 int_saved, nbuf;

	/* Preserve L2 notification and NBUF settings */
	regmap_read(ctrl->map, ctrl->r->dma_if_intr_msk, &int_saved);
	regmap_read(ctrl->map, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL, &nbuf);

	/* Disable link change interrupt on RTL839x */
	regmap_write(ctrl->map, RTL839X_IMR_PORT_LINK_STS_CHG, 0);
	regmap_write(ctrl->map, RTL839X_IMR_PORT_LINK_STS_CHG + 4, 0);

	rteth_nic_reset(ctrl, 0xc);

	/* Re-enable link change interrupt */
	regmap_write(ctrl->map, RTL839X_ISR_PORT_LINK_STS_CHG, 0xffffffff);
	regmap_write(ctrl->map, RTL839X_ISR_PORT_LINK_STS_CHG + 4, 0xffffffff);
	regmap_write(ctrl->map, RTL839X_IMR_PORT_LINK_STS_CHG, 0xffffffff);
	regmap_write(ctrl->map, RTL839X_IMR_PORT_LINK_STS_CHG + 4, 0xffffffff);

	/* Restore notification settings: on RTL838x these bits are null */
	regmap_update_bits(ctrl->map, ctrl->r->dma_if_intr_msk, 7 << 20, int_saved & (7 << 20));
	regmap_write(ctrl->map, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL, nbuf);
}

static void rteth_93xx_set_hol(struct rteth_ctrl *ctrl)
{
	/*
	 * The counter registers track the number of packets that are allowed to be appended to
	 * the ring buffer. On RTL93xx a ring overflow must be avoided at all cost. Defensively
	 * calculate the space by anticipating that only fully filled packets are received.
	 */
	int max_frame_size = ctrl->dev->mtu + RTETH_FRAME_OVERHEAD;
	int frags_per_pkt = DIV_ROUND_UP(max_frame_size, SKB_FRAG_SIZE);
	int cnt = min(RTETH_RX_RING_SIZE / frags_per_pkt, 0x3ff);

	for (int ring = 0; ring < RTETH_RX_RINGS; ring++) {
		int shift = (ring % 3) * 10;
		int reg = (ring / 3) * 4;

		regmap_update_bits(ctrl->map, ctrl->r->dma_if_rx_ring_size + reg,
				   0x3ff << shift, cnt << shift);
	}
}

static void rteth_93xx_hw_reset(struct rteth_ctrl *ctrl)
{
	rteth_nic_reset(ctrl, 0x6);

	for (int ring = 0; ring < RTETH_RX_RINGS; ring++) {
		int reg = (ring / 3) * 4;
		u32 v;

		/* clear counters by simply writing the current register values back */
		regmap_read(ctrl->map, ctrl->r->dma_if_rx_ring_cntr + reg, &v);
		regmap_write(ctrl->map, ctrl->r->dma_if_rx_ring_cntr + reg, v);
	}
}

static void rteth_setup_cpu_rx_rings(struct rteth_ctrl *ctrl)
{
	/*
	 * Realtek switches either have 8 (RTL83xx) or 32 (RTL93xx) receive queues. Whenever
	 * a frag is trapped/received for the CPU it is put into one of these queues. This
	 * is configured via mapping registers in two ways:
	 *
	 * - Switching queue/priority to CPU queue mapping (RTL83xx)
	 * - Reason (why it is sent to CPU) to CPU queue mapping (all devices)
	 *
	 * With only low performance CPUs there is not much benefit of using all of these
	 * queues in parallel. Especially because each queue needs buffer space. To keep
	 * the queue limit simple, just write the desired CPU queue in a round robin style
	 * to the registers.
	 */

	if (ctrl->r->qm_pkt2cpu_intpri_map) {
		for (int priority = 0; priority < 8; priority++) {
			int reg = ctrl->r->qm_pkt2cpu_intpri_map;
			int ring = priority % RTETH_RX_RINGS;
			int shift = priority * 3;

			regmap_update_bits(ctrl->map, reg, 0x7 << shift, ring << shift);
		}
	}

	if (ctrl->r->qm_rsn2cpuqid_ctrl) {
		int mask, bits_per_field, fields_per_reg, reason_cnt;

		mask = ctrl->r->rx_rings - 1;
		bits_per_field = fls(mask);
		fields_per_reg = 32 / bits_per_field;
		reason_cnt = ctrl->r->qm_rsn2cpuqid_cnt * fields_per_reg;

		/* Reason registers have gaps. Do not care for now. */
		for (int reason = 0; reason < reason_cnt; reason++) {
			int reg = ctrl->r->qm_rsn2cpuqid_ctrl + 4 * (reason / fields_per_reg);
			int shift = (reason % fields_per_reg) * bits_per_field;
			int ring = reason % RTETH_RX_RINGS;

			regmap_update_bits(ctrl->map, reg, mask << shift, ring << shift);
		}
	}
}

static void rteth_hw_ring_setup(struct rteth_ctrl *ctrl)
{
	for (int r = 0; r < RTETH_RX_RINGS; r++)
		regmap_write(ctrl->map, ctrl->r->dma_rx_base + r * 4,
			     ctrl->rx_dma + r * sizeof(struct rteth_rx_data) +
			     offsetof(struct rteth_rx_data, ring));

	for (int r = 0; r < RTETH_TX_RINGS; r++)
		regmap_write(ctrl->map, ctrl->r->dma_tx_base + r * 4,
			     ctrl->tx_dma + r * sizeof(struct rteth_tx_data) +
			     offsetof(struct rteth_tx_data, ring));
}

/*
 * Three limits bound the frames exchanged with the CPU port, and each family spreads them
 * over registers of its own, whose length fields are named after the block holding them:
 * what the driver receives is transmitted by the switch, but received by the NIC.
 *
 * - the DMA truncation length cuts a frame short. It still reaches the CPU, carrying only
 *   as many bytes as the field allows, so switch it off and leave the length to the others.
 * - the largest frame the switch hands to the CPU, which drops the longer ones. Both
 *   measured on an RTL9303 by lowering one at a time while frames of growing size were
 *   sent into a user port.
 * - the largest frame the switch takes from the CPU. Longan keeps it in the port register
 *   of the CPU port, Maple in a register of its own, the others in a second field of the
 *   register above.
 */
static void rteth_838x_set_max_packet_length(struct rteth_ctrl *ctrl, int len)
{
	regmap_update_bits(ctrl->map, RTETH_838X_DMA_IF_PKT_RX_FLTR_CTRL, GENMASK(13, 0), len);
	regmap_update_bits(ctrl->map, RTETH_838X_DMA_IF_PKT_TX_FLTR_CTRL, GENMASK(13, 0), len);
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_ctrl, RX_TRUNCATE_EN_83XX);
}

static void rteth_839x_set_max_packet_length(struct rteth_ctrl *ctrl, int len)
{
	regmap_update_bits(ctrl->map, RTETH_839X_DMA_IF_PKT_FLTR_CTRL,
			   GENMASK(27, 14) | GENMASK(13, 0),
			   FIELD_PREP(GENMASK(27, 14), len) | FIELD_PREP(GENMASK(13, 0), len));
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_ctrl, RX_TRUNCATE_EN_83XX);
}

static void rteth_930x_set_max_packet_length(struct rteth_ctrl *ctrl, int len)
{
	regmap_update_bits(ctrl->map, RTETH_930X_MAC_L2_CPU_MAX_LEN_CTRL, GENMASK(13, 0), len);
	regmap_update_bits(ctrl->map, RTETH_930X_MAC_L2_PORT_MAX_LEN_CTRL,
			   GENMASK(27, 14) | GENMASK(13, 0),
			   FIELD_PREP(GENMASK(27, 14), len) | FIELD_PREP(GENMASK(13, 0), len));
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_ctrl, RX_TRUNCATE_EN_93XX);
}

static void rteth_931x_set_max_packet_length(struct rteth_ctrl *ctrl, int len)
{
	regmap_update_bits(ctrl->map, RTETH_931X_MAC_L2_CPU_MAX_LEN_CTRL,
			   GENMASK(27, 14) | GENMASK(13, 0),
			   FIELD_PREP(GENMASK(27, 14), len) | FIELD_PREP(GENMASK(13, 0), len));
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_ctrl, RX_TRUNCATE_EN_93XX);
}

static void rteth_set_max_packet_length(struct rteth_ctrl *ctrl)
{
	ctrl->r->set_max_packet_length(ctrl, ctrl->dev->mtu + RTETH_FRAME_OVERHEAD);
}

static void rteth_838x_hw_en_rxtx(struct rteth_ctrl *ctrl)
{
	/* Pad TX */
	regmap_write(ctrl->map, ctrl->r->dma_if_ctrl, TX_PAD_EN_838X);

	rteth_set_max_packet_length(ctrl);

	rteth_enable_all_rx_irqs(ctrl);

	/* Enable DMA, engine expects empty FCS field */
	regmap_update_bits(ctrl->map, ctrl->r->dma_if_ctrl,
			   ctrl->r->tx_rx_enable, ctrl->r->tx_rx_enable);

	/* Restart TX/RX to CPU port */
	regmap_update_bits(ctrl->map, ctrl->r->dma_if_ctrl, 0x3, 0x3);
	/* Set Speed, duplex, flow control
	 * FORCE_EN | LINK_EN | NWAY_EN | DUP_SEL
	 * | SPD_SEL = 0b10 | FORCE_FC_EN | PHY_MASTER_SLV_MANUAL_EN
	 * | MEDIA_SEL
	 */
	regmap_write(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x6192F);

	/* Enable CRC checks on CPU-port */
	regmap_update_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, BIT(3), BIT(3));
}

static void rteth_839x_hw_en_rxtx(struct rteth_ctrl *ctrl)
{
	rteth_set_max_packet_length(ctrl);

	rteth_enable_all_rx_irqs(ctrl);

	/* Enable DMA */
	regmap_update_bits(ctrl->map, ctrl->r->dma_if_ctrl,
			   ctrl->r->tx_rx_enable, ctrl->r->tx_rx_enable);

	/* Restart TX/RX to CPU port, enable CRC checking */
	regmap_update_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3 | BIT(3), 0x3 | BIT(3));

	/* CPU port joins Lookup Miss Flooding Portmask */
	/* TODO: The code below should also work for the RTL838x */
	regmap_write(ctrl->map, RTL839X_TBL_ACCESS_L2_CTRL, 0x28000);
	regmap_update_bits(ctrl->map, RTL839X_TBL_ACCESS_L2_DATA(0), BIT(31), BIT(31));
	regmap_write(ctrl->map, RTL839X_TBL_ACCESS_L2_CTRL, 0x38000);

	/* Force CPU port link up */
	regmap_update_bits(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x3, 0x3);
}

static void rteth_930x_hw_en_rxtx(struct rteth_ctrl *ctrl)
{
	rteth_set_max_packet_length(ctrl);

	rteth_enable_all_rx_irqs(ctrl);

	/* Enable DMA */
	regmap_set_bits(ctrl->map, ctrl->r->dma_if_ctrl, ctrl->r->tx_rx_enable);

	/* Restart TX/RX to CPU port, enable CRC checking */
	regmap_set_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3 | BIT(4));

	regmap_set_bits(ctrl->map, RTL930X_L2_UNKN_UC_FLD_PMSK, BIT(ctrl->r->cpu_port));
	regmap_write(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x217);
}

static void rteth_931x_hw_en_rxtx(struct rteth_ctrl *ctrl)
{
	rteth_set_max_packet_length(ctrl);

	rteth_enable_all_rx_irqs(ctrl);

	/* Enable DMA */
	regmap_set_bits(ctrl->map, ctrl->r->dma_if_ctrl, ctrl->r->tx_rx_enable);

	/* Restart TX/RX to CPU port, enable CRC checking */
	regmap_set_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3 | BIT(4));

	regmap_set_bits(ctrl->map, RTL931X_L2_UNKN_UC_FLD_PMSK, BIT(ctrl->r->cpu_port));
	regmap_write(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x2a1d);
}

static void rteth_free_tx_buffers(struct rteth_ctrl *ctrl)
{
	struct rteth_tx_info *tx_info;

	for (int r = 0; r < RTETH_TX_RINGS; r++) {
		tx_info = &ctrl->tx_info[r];
		for (int i = 0; i < RTETH_TX_RING_SIZE; i++) {
			dma_addr_t dma = ctrl->tx_data[r].frag[i].dma;

			if (!tx_info->skb[i])
				continue;

			dma_unmap_single(&ctrl->pdev->dev, dma,
					 tx_info->skb[i]->len, DMA_TO_DEVICE);
			rteth_free_skb(&tx_info->skb[i]);
		}
		tx_info->send_count = 0;
		tx_info->clean_count = 0;
	}
}

static void rteth_reclaim_tx_ring(struct rteth_ctrl *ctrl, int r)
{
	struct rteth_tx_info *tx_info = &ctrl->tx_info[r];

	BUILD_BUG_ON(RTETH_TX_RING_SIZE & (RTETH_TX_RING_SIZE - 1));

	while (tx_info->send_count != tx_info->clean_count) {
		int i = tx_info->clean_count & (RTETH_TX_RING_SIZE - 1);

		if (ctrl->tx_data[r].ring[i] & RING_OWN_HW)
			break;

		dma_unmap_single(&ctrl->pdev->dev, ctrl->tx_data[r].frag[i].dma,
				 tx_info->skb[i]->len, DMA_TO_DEVICE);
		dev_consume_skb_any(tx_info->skb[i]);
		tx_info->skb[i] = NULL;
		tx_info->clean_count++;
	}
}

static void rteth_reclaim_tx_rings(struct rteth_ctrl *ctrl)
{
	for (int r = 0; r < RTETH_TX_RINGS; r++) {
		struct netdev_queue *txq;

		/* Cached-memory fast path, made stable by the lock below */
		if (READ_ONCE(ctrl->tx_info[r].send_count) ==
		    READ_ONCE(ctrl->tx_info[r].clean_count))
			continue;

		txq = netdev_get_tx_queue(ctrl->dev, r);

		__netif_tx_lock(txq, smp_processor_id());

		if (!netif_xmit_frozen_or_stopped(txq))
			rteth_reclaim_tx_ring(ctrl, r);

		__netif_tx_unlock(txq);
	}
}

static void rteth_free_rx_buffers(struct rteth_ctrl *ctrl)
{
	struct rteth_rx_info *rx_info;

	for (int r = 0; r < RTETH_RX_RINGS; r++) {
		rx_info = &ctrl->rx_info[r];
		for (int i = 0; i < RTETH_RX_RING_SIZE; i++) {
			if (!rx_info->page[i])
				continue;

			page_pool_put_full_page(rx_info->pool, rx_info->page[i], true);
			rx_info->page[i] = NULL;
		}
		rteth_free_skb(&rx_info->skb);
	}
}

static int rteth_setup_ring_buffer(struct rteth_ctrl *ctrl)
{
	struct rteth_frag *frag;
	phys_addr_t paddr = 0;
	bool highmem = false;
	unsigned int offset;
	struct page *page;

	/*
	 * The conversion to page_pool raised some questions about the memory consumption of SKBs
	 * and the DMA capabilities of the network adapter. Be defensive and add some checks to
	 * assist further error analysis.
	 */
	BUILD_BUG_ON(SKB_HEADROOM_FAST + SKB_FRAG_SIZE +
		     SKB_DATA_ALIGN(sizeof(struct skb_shared_info)) > PPOOL_FRAG_SIZE);

	for (int r = 0; r < RTETH_RX_RINGS; r++) {
		for (int i = 0; i < RTETH_RX_RING_SIZE; i++) {
			frag = &ctrl->rx_data[r].frag[i];
			page = page_pool_dev_alloc_frag(ctrl->rx_info[r].pool,
							&offset, PPOOL_FRAG_SIZE);
			if (!page) {
				dev_err(&ctrl->pdev->dev,
					"Failed to allocate RX fragment from pool\n");
				rteth_free_rx_buffers(ctrl);
				return -ENOMEM;
			}

			highmem |= PageHighMem(page);
			paddr = max(paddr, page_to_phys(page));

			frag->size = SKB_FRAG_SIZE;
			frag->dma = page_pool_get_dma_addr(page)
				    + ctrl->r->skb_headroom + offset;
			ctrl->rx_info[r].page[i] = page;
			ctrl->rx_info[r].offset[i] = offset;
			ctrl->rx_data[r].ring[i] = ctrl->rx_dma +
						   sizeof(struct rteth_rx_data) * r +
						   offsetof(struct rteth_rx_data, frag) +
						   sizeof(struct rteth_frag) * i +
						   RING_OWN_HW;
		}

		ctrl->rx_info[r].slot = 0;
		ctrl->rx_info[r].skb = NULL;
		ctrl->rx_data[r].ring[RTETH_RX_RING_SIZE - 1] |= RING_WRAP;
	}

	for (int r = 0; r < RTETH_TX_RINGS; r++) {
		for (int i = 0; i < RTETH_TX_RING_SIZE; i++) {
			ctrl->tx_info[r].skb[i] = NULL;
			ctrl->tx_data[r].ring[i] = ctrl->tx_dma +
						   sizeof(struct rteth_tx_data) * r +
						   offsetof(struct rteth_tx_data, frag) +
						   sizeof(struct rteth_frag) * i;
		}

		ctrl->tx_data[r].ring[RTETH_TX_RING_SIZE - 1] |= RING_WRAP;
		ctrl->tx_info[r].send_count = 0;
		ctrl->tx_info[r].clean_count = 0;
	}

	if (highmem)
		netdev_info(ctrl->dev, "receive page pool in highmem\n");
	else
		netdev_info(ctrl->dev, "receive page pool below %d MB\n", (paddr >> 20) + 1);

	return 0;
}

static void rteth_839x_setup_notify_ring_buffer(struct rteth_ctrl *ctrl)
{
	struct notify_b *b = ctrl->membase;

	for (int i = 0; i < NOTIFY_BLOCKS; i++)
		b->ring[i] = KSEG1ADDR(&b->blocks[i]) | RING_OWN_HW;
	b->ring[NOTIFY_BLOCKS - 1] |= RING_WRAP;

	regmap_write(ctrl->map, RTL839X_DMA_IF_NBUF_BASE_DESC_ADDR_CTRL, (u32)b->ring);
	regmap_update_bits(ctrl->map, RTL839X_L2_NOTIFICATION_CTRL, 0x3ff << 2, 100 << 2);

	/* Setup notification events */

	/* RTL8390_L2_CTRL_0_FLUSH_NOTIFY_EN */
	regmap_set_bits(ctrl->map, RTL839X_L2_CTRL_0, BIT(14));
	/* SUSPEND_NOTIFICATION_EN */
	regmap_set_bits(ctrl->map, RTL839X_L2_NOTIFICATION_CTRL, BIT(12));

	/* Enable Notification */
	regmap_set_bits(ctrl->map, RTL839X_L2_NOTIFICATION_CTRL, BIT(0));
	ctrl->lastEvent = 0;

	/* Make sure the ring structure is visible to the ASIC */
	mb();
	flush_cache_all();
}

static void rteth_838x_hw_init(struct rteth_ctrl *ctrl)
{
	/* Trap IGMP/MLD traffic to CPU-Port */
	regmap_write(ctrl->map, RTL838X_SPCL_TRAP_IGMP_CTRL, 0x3);
	/* Flush learned FDB entries on link down of a port */
	regmap_set_bits(ctrl->map, RTL838X_L2_CTRL_0, BIT(7));
}

static void rteth_839x_hw_init(struct rteth_ctrl *ctrl)
{
	/* Trap MLD and IGMP messages to CPU_PORT */
	regmap_write(ctrl->map, RTL839X_SPCL_TRAP_IGMP_CTRL, 0x3);
	/* Flush learned FDB entries on link down of a port */
	regmap_set_bits(ctrl->map, RTL839X_L2_CTRL_0, BIT(7));
}

static void rteth_930x_hw_init(struct rteth_ctrl *ctrl)
{
	/* Trap MLD and IGMP messages to CPU_PORT */
	regmap_write(ctrl->map, RTL930X_VLAN_APP_PKT_CTRL, 0x12);
	/* Flush learned FDB entries on link down of a port */
	regmap_set_bits(ctrl->map, RTL930X_L2_CTRL, BIT(7));
}

static void rteth_931x_hw_init(struct rteth_ctrl *ctrl)
{
	/* Trap MLD and IGMP messages to CPU_PORT */
	regmap_write(ctrl->map, RTL931X_VLAN_APP_PKT_CTRL, 0x12);
	/* Set PCIE_PWR_DOWN */
	regmap_set_bits(ctrl->map, RTL931X_PS_SOC_CTRL, BIT(1));
}

static int rteth_open(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	int ret;

	ctrl->r->hw_reset(ctrl);
	ctrl->r->set_hol(ctrl);
	rteth_setup_cpu_rx_rings(ctrl);
	ret = rteth_setup_ring_buffer(ctrl);
	if (ret)
		return ret;

	if (ctrl->r->setup_notify_ring_buffer)
		ctrl->r->setup_notify_ring_buffer(ctrl);

	rteth_hw_ring_setup(ctrl);
	phylink_start(ctrl->phylink);

	for (int i = 0; i < RTETH_RX_RINGS; i++)
		napi_enable(&ctrl->rx_info[i].napi);
	ctrl->napi_enabled = true;

	ctrl->r->hw_init(ctrl);
	ctrl->r->hw_en_rxtx(ctrl);
	netif_tx_start_all_queues(dev);

	return 0;
}

static void rteth_838x_hw_stop(struct rteth_ctrl *ctrl)
{
	u32 val;

	/* Block all ports. TODO: this is an unprotected table access */
	regmap_write(ctrl->map, RTL838X_TBL_ACCESS_DATA_0(0), 0x3000000);
	regmap_write(ctrl->map, RTL838X_TBL_ACCESS_DATA_0(1), 0x0);
	regmap_write(ctrl->map, RTL838X_TBL_ACCESS_CTRL_0, 1 << 15 | 2 << 12);

	/* Disable FAST_AGE_OUT otherwise flush will hang */
	regmap_clear_bits(ctrl->map, RTL838X_L2_CTRL_1, BIT(23));

	/* Flush L2 address cache */
	for (int i = 0; i <= ctrl->r->cpu_port; i++) {
		regmap_write(ctrl->map, ctrl->r->l2_tbl_flush_ctrl, BIT(26) | BIT(23) | i << 5);
		regmap_read_poll_timeout(ctrl->map, ctrl->r->l2_tbl_flush_ctrl,
					 val, !(val & BIT(26)), 100, 100000);
	}

	/* CPU-Port: Link down */
	regmap_write(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x6192C);
}

static void rteth_839x_hw_stop(struct rteth_ctrl *ctrl)
{
	u32 val;

	/* Flush L2 address cache */
	for (int i = 0; i <= ctrl->r->cpu_port; i++) {
		regmap_write(ctrl->map, ctrl->r->l2_tbl_flush_ctrl, BIT(28) | BIT(25) | i << 5);
		regmap_read_poll_timeout(ctrl->map, ctrl->r->l2_tbl_flush_ctrl,
					 val, !(val & BIT(28)), 100, 100000);
	}

	regmap_write(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x75);
}

static void rteth_930x_hw_stop(struct rteth_ctrl *ctrl)
{
	/* TODO: L2 flush needed */

	/* CPU-Port: Link down */
	regmap_clear_bits(ctrl->map, ctrl->r->mac_force_mode_ctrl, 0x3);
}

static void rteth_931x_hw_stop(struct rteth_ctrl *ctrl)
{
	/* TODO: L2 flush needed */

	/* CPU-Port: Link down */
	regmap_clear_bits(ctrl->map, ctrl->r->mac_force_mode_ctrl, BIT(0) | BIT(9));
}

static void rteth_hw_stop(struct rteth_ctrl *ctrl)
{
	/* Disable RX/TX from/to CPU-port */
	regmap_clear_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3);

	/* Disable traffic */
	regmap_clear_bits(ctrl->map, ctrl->r->dma_if_ctrl, ctrl->r->tx_rx_enable);
	mdelay(200); /* Test, whether this is needed */

	/* family specific stop */
	ctrl->r->hw_stop(ctrl);
	mdelay(100);

	rteth_disable_all_irqs(ctrl);

	/* Disable TX/RX DMA */
	regmap_write(ctrl->map, ctrl->r->dma_if_ctrl, 0);
	mdelay(200);
}

static int rteth_stop(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	netif_tx_stop_all_queues(dev);
	phylink_stop(ctrl->phylink);
	rteth_hw_stop(ctrl);

	if (ctrl->napi_enabled) {
		ctrl->napi_enabled = false;
		for (int i = 0; i < RTETH_RX_RINGS; i++)
			napi_disable(&ctrl->rx_info[i].napi);
	}

	rteth_free_tx_buffers(ctrl);
	rteth_free_rx_buffers(ctrl);

	return 0;
}

static int rteth_change_mtu(struct net_device *dev, int mtu)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	bool grow = mtu > dev->mtu;

	/* A frame of that size must still fit into the linear part and fragments of a single skb */
	if (mtu + RTETH_FRAME_OVERHEAD > (MAX_SKB_FRAGS + 1) * SKB_FRAG_SIZE)
		return -EINVAL;

	WRITE_ONCE(dev->mtu, mtu);

	/* Opening the interface programs both limits from the MTU, so leave the hardware alone */
	if (!netif_running(dev))
		return 0;

	netdev_warn(dev, "changing the MTU may have unexpected effects under high receive load\n");

	/*
	 * A larger MTU buys a smaller budget, so hand the rings their new limit before the
	 * switch may deliver larger frames, and the other way around when the MTU shrinks.
	 */
	if (grow)
		ctrl->r->set_hol(ctrl);

	rteth_set_max_packet_length(ctrl);

	if (!grow)
		ctrl->r->set_hol(ctrl);

	return 0;
}

static void rteth_838x_set_rx_mode(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(21, 0) = 0x3FFFFF
	 */
	if (!(dev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		regmap_write(ctrl->map, RTETH_838X_RMA_CTRL_0, 0);
		regmap_write(ctrl->map, RTETH_838X_RMA_CTRL_1, 0);
	}
	if (dev->flags & IFF_ALLMULTI)
		regmap_write(ctrl->map, RTETH_838X_RMA_CTRL_0, GENMASK(21, 0));
	if (dev->flags & IFF_PROMISC) {
		regmap_write(ctrl->map, RTETH_838X_RMA_CTRL_0, GENMASK(21, 0));
		regmap_write(ctrl->map, RTETH_838X_RMA_CTRL_1, GENMASK(14, 0));
	}
}

static void rteth_839x_set_rx_mode(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (!(dev->flags & (IFF_PROMISC | IFF_ALLMULTI))) {
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_0, 0);
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_1, 0);
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_2, 0);
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_3, 0);
	}
	if (dev->flags & IFF_ALLMULTI) {
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_0, GENMASK(31, 2));
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_1, GENMASK(31, 0));
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_2, GENMASK(31, 0));
	}
	if (dev->flags & IFF_PROMISC) {
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_0, GENMASK(31, 2));
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_1, GENMASK(31, 0));
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_2, GENMASK(31, 0));
		regmap_write(ctrl->map, RTETH_839X_RMA_CTRL_3, GENMASK(9, 0));
	}
}

static void rteth_930x_set_rx_mode(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (dev->flags & (IFF_ALLMULTI | IFF_PROMISC)) {
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_0, GENMASK(31, 2));
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_1, GENMASK(31, 0));
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_2, GENMASK(31, 0));
	} else {
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_0, 0);
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_1, 0);
		regmap_write(ctrl->map, RTETH_930X_RMA_CTRL_2, 0);
	}
}

static void rteth_931x_set_rx_mode(struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	/* Flood all classes of RMA addresses (01-80-C2-00-00-{01..2F})
	 * CTRL_0_FULL = GENMASK(31, 2) = 0xFFFFFFFC
	 * Lower two bits are reserved, corresponding to RMA 01-80-C2-00-00-00.
	 * CTRL_1_FULL = CTRL_2_FULL = GENMASK(31, 0)
	 */
	if (dev->flags & (IFF_ALLMULTI | IFF_PROMISC)) {
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_0, GENMASK(31, 2));
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_1, GENMASK(31, 0));
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_2, GENMASK(31, 0));
	} else {
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_0, 0);
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_1, 0);
		regmap_write(ctrl->map, RTETH_931X_RMA_CTRL_2, 0);
	}
}

static void rteth_reset_work(struct work_struct *work)
{
	struct rteth_ctrl *ctrl = container_of(work, struct rteth_ctrl, reset_work);
	struct net_device *dev = ctrl->dev;
	int ret;

	rtnl_lock();

	if (!netif_running(dev))
		goto out;

	rteth_stop(dev);
	ret = rteth_open(dev);
	if (ret) {
		netdev_err(dev, "tx timeout recovery failed: %d\n", ret);
		netif_device_detach(dev);
		goto out;
	}

	netif_trans_update(dev);

out:
	rtnl_unlock();
}

static void rteth_tx_timeout(struct net_device *dev, unsigned int txqueue)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	netif_tx_stop_all_queues(dev);
	schedule_work(&ctrl->reset_work);
}

static int rteth_get_dsa_port(struct sk_buff *skb, struct net_device *dev)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	u8 *trailer;

	if (skb->len < 4)
		return -ENOENT;

	trailer = &skb->data[skb->len - 4];
	if (netdev_uses_dsa(dev) &&
	    dev->dsa_ptr->tag_ops->proto == DSA_TAG_PROTO_RTL_OTTO &&
	    trailer[0] < ctrl->r->cpu_port &&
	    trailer[1] == 0xab &&
	    trailer[2] == 0xcd &&
	    trailer[3] == 0xef)
		return trailer[0];

	return -ENOENT;
}

static int rteth_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	int port, val, slot, len = skb->len, ring = skb_get_queue_mapping(skb);
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	struct sk_buff **packet_skb;
	struct rteth_frag *frag;
	dma_addr_t packet_dma;

	port = rteth_get_dsa_port(skb, dev);
	if (port < 0)
		len += ETH_FCS_LEN; /* No reusable 4 byte tag, add space for 4 byte layer 2 FCS */

	len = max(ETH_ZLEN + ETH_FCS_LEN, len);
	if (unlikely(skb_put_padto(skb, len))) {
		dev->stats.tx_errors++;
		netdev_warn(dev, "skb pad failed\n");

		return NETDEV_TX_OK;
	}

	slot = ctrl->tx_info[ring].send_count & (RTETH_TX_RING_SIZE - 1);
	frag = &ctrl->tx_data[ring].frag[slot];
	packet_dma = ctrl->tx_data[ring].ring[slot];
	packet_skb = &ctrl->tx_info[ring].skb[slot];

	if (unlikely(packet_dma & RING_OWN_HW)) {
		netif_stop_subqueue(dev, ring);
		if (net_ratelimit())
			netdev_warn(dev, "tx ring %d busy, waiting for slot %d\n", ring, slot);

		return NETDEV_TX_BUSY;
	}

	if (unlikely(*packet_skb))
		rteth_reclaim_tx_ring(ctrl, ring);

	*packet_skb = skb;
	frag->len = len;
	frag->dma = dma_map_single(&ctrl->pdev->dev, skb->data, len, DMA_TO_DEVICE);
	if (unlikely(dma_mapping_error(&ctrl->pdev->dev, frag->dma))) {
		dev->stats.tx_errors += rteth_free_skb(packet_skb);
		return NETDEV_TX_OK;
	}

	if (port >= 0)
		ctrl->r->create_tx_header(frag, port, 0); // TODO ok to set prio to 0?

	/* Hand frag over to switch */
	dma_wmb();
	ctrl->tx_data[ring].ring[slot] = packet_dma | RING_OWN_HW;
	ctrl->tx_info[ring].send_count++;
	wmb();

	spin_lock(&ctrl->tx_lock);

	/*
	 * Issue send for 1 or 2 triggers. On some SoCs (especially RTL838x) there is a known
	 * bug, where the hardware sometimes reads empty values from the register. Work around
	 * that with a poll that checks if TX/RX is enabled in the register.
	 */
	if (regmap_read_poll_timeout(ctrl->map, ctrl->r->dma_if_ctrl,
				     val, val & ctrl->r->tx_rx_enable, 0, 5000))
		netdev_warn_once(dev, "DMA interface ctrl register read failed\n");

	regmap_write(ctrl->map, ctrl->r->dma_if_ctrl, val | RTETH_TX_TRIGGER(ctrl, ring));

	dev->stats.tx_packets++;
	dev->stats.tx_bytes += len - ETH_FCS_LEN;

	spin_unlock(&ctrl->tx_lock);

	return NETDEV_TX_OK;
}

static struct sk_buff *rteth_create_skb(struct rteth_ctrl *ctrl, int ring, int slot)
{
	struct rteth_frag *frag = &ctrl->rx_data[ring].frag[slot];
	unsigned int offset = ctrl->rx_info[ring].offset[slot];
	struct page *page = ctrl->rx_info[ring].page[slot];
	struct page_pool *pool = ctrl->rx_info[ring].pool;
	struct net_device *dev = ctrl->dev;
	unsigned int len = frag->len;
	struct sk_buff *skb;
	struct dsa_tag tag;

	page_pool_dma_sync_for_cpu(pool, page, offset + ctrl->r->skb_headroom, len);
	skb = napi_build_skb(page_address(page) + offset, PPOOL_FRAG_SIZE);
	if (unlikely(!skb)) {
		page_pool_put_full_page(pool, page, true);
		return NULL;
	}

	skb_reserve(skb, ctrl->r->skb_headroom);
	skb_mark_for_recycle(skb);
	skb_put(skb, len);

	ctrl->r->decode_tag(frag, &tag);
	if (netdev_uses_dsa(dev)) {
		if (tag.port < ctrl->r->cpu_port)
			skb_dst_set_noref(skb, &ctrl->dsa_meta[tag.port]->dst);
		if (tag.l2_offloaded)
			skb->offload_fwd_mark = 1;
	}

	if (dev->features & NETIF_F_RXCSUM) {
		if (tag.crc_error)
			skb_checksum_none_assert(skb);
		else
			skb->ip_summed = CHECKSUM_UNNECESSARY;
	}

	return skb;
}

static int rteth_append_skb(struct sk_buff *skb, struct rteth_ctrl *ctrl, int ring, int slot)
{
	struct rteth_frag *frag = &ctrl->rx_data[ring].frag[slot];
	unsigned int offset = ctrl->rx_info[ring].offset[slot];
	struct page *page = ctrl->rx_info[ring].page[slot];
	unsigned int nr_frags = skb_shinfo(skb)->nr_frags;
	struct page_pool *pool = ctrl->rx_info[ring].pool;
	unsigned int len = frag->len;

	if (nr_frags >= MAX_SKB_FRAGS) {
		page_pool_put_full_page(pool, page, true);
		return -ENOMEM;
	}

	page_pool_dma_sync_for_cpu(pool, page, offset + ctrl->r->skb_headroom, len);
	skb_add_rx_frag(skb, nr_frags, page, offset + ctrl->r->skb_headroom,
			len, PPOOL_FRAG_SIZE);

	return 0;
}

static int rteth_hw_receive(struct net_device *dev, int ring, int budget)
{
	int slot, work_done = 0, rx_packets = 0, rx_bytes = 0, rx_dropped = 0, rx_errors = 0;
	struct rteth_ctrl *ctrl = netdev_priv(dev);
	struct rteth_rx_info *rx_info = &ctrl->rx_info[ring];
	unsigned int len, new_offset;
	struct rteth_frag *frag;
	struct page_pool *pool;
	struct page *new_page;
	dma_addr_t packet_dma;
	bool is_head, is_tail;
	struct sk_buff *skb;

	pool = rx_info->pool;
	skb = rx_info->skb;
	is_tail = !skb;

	while (work_done < budget) {
		slot = rx_info->slot;
		packet_dma = ctrl->rx_data[ring].ring[slot];
		rmb();

		if (packet_dma & RING_OWN_HW)
			break;

		frag = &ctrl->rx_data[ring].frag[slot];
		len = frag->len;
		is_head = is_tail;
		is_tail = !frag->more;
		if (is_tail)
			work_done++;

		if (unlikely(len > SKB_FRAG_SIZE)) {
			netdev_err(dev, "invalid fragment with %d bytes received\n", len);
			rx_errors += rteth_free_skb(&skb);
			goto recycle;
		}

		if (unlikely(!skb && !is_head))
			goto recycle;

		/*
		 * Avoid complex error handling by allocating the new page before SKB consumption.
		 * In case of failure drop the data and reuse the existing page.
		 */
		new_page = page_pool_dev_alloc_frag(pool, &new_offset, PPOOL_FRAG_SIZE);
		if (unlikely(!new_page)) {
			netdev_err(dev, "fragment allocation failed\n");
			rx_dropped += rteth_free_skb(&skb);
			goto recycle;
		}

		if (is_head) {
			skb = rteth_create_skb(ctrl, ring, slot);
			if (unlikely(!skb)) {
				netdev_err(dev, "skb creation failed\n");
				rx_dropped++;
			}
		} else {
			if (unlikely(rteth_append_skb(skb, ctrl, ring, slot))) {
				netdev_err(dev, "skb append failed\n");
				rx_dropped += rteth_free_skb(&skb);
			}
		}

		if (is_tail && skb) {
			if (unlikely(skb->len < ETH_HLEN + ETH_FCS_LEN)) {
				rx_errors += rteth_free_skb(&skb);
			} else {
				pskb_trim(skb, skb->len - ETH_FCS_LEN);
				rx_bytes += skb->len;
				rx_packets++;
				skb->protocol = eth_type_trans(skb, dev);
				napi_gro_receive(&rx_info->napi, skb);
				skb = NULL;
			}
		}

		rx_info->page[slot] = new_page;
		rx_info->offset[slot] = new_offset;
		frag->dma = page_pool_get_dma_addr(new_page) +
			    new_offset + ctrl->r->skb_headroom;
recycle:
		dma_wmb();
		ctrl->rx_data[ring].ring[slot] = packet_dma | RING_OWN_HW;
		rx_info->slot = (slot + 1) % RTETH_RX_RING_SIZE;
	}

	spin_lock(&ctrl->rx_lock);
	ctrl->r->update_counter(ctrl, ring, work_done);
	dev->stats.rx_packets += rx_packets;
	dev->stats.rx_dropped += rx_dropped;
	dev->stats.rx_errors += rx_errors;
	dev->stats.rx_bytes += rx_bytes;
	spin_unlock(&ctrl->rx_lock);

	rx_info->skb = skb;

	return work_done;
}

static int rteth_poll_rx(struct napi_struct *napi, int budget)
{
	struct rteth_rx_info *rx_q = container_of(napi, struct rteth_rx_info, napi);
	struct rteth_ctrl *ctrl = rx_q->ctrl;
	int work_done, ring = rx_q->id;

	rteth_reclaim_tx_rings(ctrl);

	work_done = rteth_hw_receive(ctrl->dev, ring, budget);
	if (work_done < budget && napi_complete_done(napi, work_done))
		ctrl->r->enable_rx_irq(ctrl, ring);

	return work_done;
}

static void rteth_mac_config(struct phylink_config *config,
			     unsigned int mode,
			     const struct phylink_link_state *state)
{
	/* This is only being called for the master device,
	 * i.e. the CPU-Port. We don't need to do anything.
	 */

	pr_info("In %s, mode %x\n", __func__, mode);
}

static void rteth_mac_link_down(struct phylink_config *config,
				unsigned int mode,
				phy_interface_t interface)
{
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	pr_debug("In %s\n", __func__);
	/* Stop TX/RX to port */
	regmap_clear_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3);
}

static void rteth_mac_link_up(struct phylink_config *config,
			      struct phy_device *phy, unsigned int mode,
			      phy_interface_t interface, int speed, int duplex,
			      bool tx_pause, bool rx_pause)
{
	struct net_device *dev = container_of(config->dev, struct net_device, dev);
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	pr_debug("In %s\n", __func__);
	/* Restart TX/RX to port */
	regmap_set_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, 0x3);
}

static void rteth_set_mac_hw(struct net_device *dev, u8 *mac)
{
	u32 mac_lo = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];
	u32 mac_hi = (mac[0] << 8) | mac[1];
	struct rteth_ctrl *ctrl;

	ctrl = netdev_priv(dev);

	for (int i = 0; i < RTETH_MAX_MAC_REGS; i++)
		if (ctrl->r->mac_reg[i]) {
			regmap_write(ctrl->map, ctrl->r->mac_reg[i], mac_hi);
			regmap_write(ctrl->map, ctrl->r->mac_reg[i] + 4, mac_lo);
		}
}

static int rteth_set_mac_address(struct net_device *dev, void *p)
{
	const struct sockaddr *addr = p;
	u8 *mac = (u8 *)(addr->sa_data);

	if (!is_valid_ether_addr(addr->sa_data))
		return -EADDRNOTAVAIL;

	dev_addr_set(dev, addr->sa_data);
	rteth_set_mac_hw(dev, mac);

	pr_info("Using MAC %pM\n", dev->dev_addr);

	return 0;
}

static int rteth_838x_init_mac(struct rteth_ctrl *ctrl)
{
	pr_info("%s\n", __func__);
	/* fix timer for EEE */
	regmap_write(ctrl->map, RTL838X_EEE_TX_TIMER_GIGA_CTRL, 0x5001411);
	regmap_write(ctrl->map, RTL838X_EEE_TX_TIMER_GELITE_CTRL, 0x5001417);

	/* Init VLAN. TODO: Understand what is being done, here */
	for (int i = 0; i <= 28; i++)
		regmap_write(ctrl->map, 0xd57c + i * 0x80, 0);

	return 0;
}

static int rteth_839x_init_mac(struct rteth_ctrl *ctrl)
{
	/* We will need to set-up EEE and the egress-rate limitation */
	return 0;
}

static int rteth_930x_init_mac(struct rteth_ctrl *ctrl)
{
	return 0;
}

static int rteth_931x_init_mac(struct rteth_ctrl *ctrl)
{
	unsigned int val;
	int ret;

	/* Initialize Encapsulation memory and wait until finished */
	regmap_write(ctrl->map, RTL931X_MEM_ENCAP_INIT, 0x1);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_ENCAP_INIT,
				       val, !(val & 1), 0, 100000);
	if (ret)
		return ret;

	/* Initialize Management Information Base memory and wait until finished */
	regmap_write(ctrl->map, RTL931X_MEM_MIB_INIT, 0x1);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_MIB_INIT,
				       val, !(val & 1), 0, 100000);
	if (ret)
		return ret;

	/* Initialize ACL (PIE) memory and wait until finished */
	regmap_write(ctrl->map, RTL931X_MEM_ACL_INIT, 0x1);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_ACL_INIT,
				       val, !(val & 1), 0, 100000);
	if (ret)
		return ret;

	/* Initialize ALE memory and wait until finished */
	regmap_write(ctrl->map, RTL931X_MEM_ALE_INIT_0, 0xffffffff);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_ALE_INIT_0,
				       val, !val, 0, 100000);
	if (ret)
		return ret;

	regmap_write(ctrl->map, RTL931X_MEM_ALE_INIT_1, 0x7f);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_ALE_INIT_1,
				       val, !val, 0, 100000);
	if (ret)
		return ret;

	regmap_write(ctrl->map, RTL931X_MEM_ALE_INIT_2, 0x7ff);
	ret = regmap_read_poll_timeout(ctrl->map, RTL931X_MEM_ALE_INIT_2,
				       val, !val, 0, 100000);
	if (ret)
		return ret;

	/* Enable ESD auto recovery */
	return regmap_write(ctrl->map, RTL931X_MDX_CTRL_RSVD, 0x1);
}

static int rteth_get_link_ksettings(struct net_device *dev,
				    struct ethtool_link_ksettings *cmd)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	pr_debug("%s called\n", __func__);

	return phylink_ethtool_ksettings_get(ctrl->phylink, cmd);
}

static int rteth_set_link_ksettings(struct net_device *dev,
				    const struct ethtool_link_ksettings *cmd)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	pr_debug("%s called\n", __func__);

	return phylink_ethtool_ksettings_set(ctrl->phylink, cmd);
}

static netdev_features_t rteth_fix_features(struct net_device *dev,
					      netdev_features_t features)
{
	return features;
}

static int rteth_83xx_set_features(struct net_device *dev, netdev_features_t features)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	if ((features ^ dev->features) & NETIF_F_RXCSUM)
		regmap_assign_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, BIT(3), features & NETIF_F_RXCSUM);

	return 0;
}

static int rteth_93xx_set_features(struct net_device *dev, netdev_features_t features)
{
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	if ((features ^ dev->features) & NETIF_F_RXCSUM)
		regmap_assign_bits(ctrl->map, ctrl->r->mac_l2_port_ctrl, BIT(4), features & NETIF_F_RXCSUM);

	return 0;
}

static int rteth_setup_tc(struct net_device *dev, enum tc_setup_type type, void *type_data)
{
    struct dsa_switch *ds;
    struct dsa_port *dp;

    if (!netdev_uses_dsa(dev))
        return -EOPNOTSUPP;

    dp = dev->dsa_ptr;
    ds = dp->ds;

    if (!ds->ops->port_setup_tc)
        return -EOPNOTSUPP;

    return ds->ops->port_setup_tc(ds, dp->index, type, type_data);
}

static const struct net_device_ops rteth_838x_netdev_ops = {
	.ndo_open = rteth_open,
	.ndo_stop = rteth_stop,
	.ndo_change_mtu = rteth_change_mtu,
	.ndo_start_xmit = rteth_start_xmit,
	.ndo_set_mac_address = rteth_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rteth_838x_set_rx_mode,
	.ndo_tx_timeout = rteth_tx_timeout,
	.ndo_set_features = rteth_83xx_set_features,
	.ndo_fix_features = rteth_fix_features,
	.ndo_setup_tc = rteth_setup_tc,
};

static const struct rteth_config rteth_838x_cfg = {
	.cpu_port = RTETH_838X_CPU_PORT,
	.max_mtu = RTETH_838X_MAX_FRAME - RTETH_FRAME_OVERHEAD,
	.rx_rings = 8,
	.tx_rx_enable = 0xc,
	.tx_trigger_mask = BIT(1),
	.mac_l2_port_ctrl = RTETH_838X_MAC_L2_PORT_CTRL,
	.qm_pkt2cpu_intpri_map = RTETH_838X_QM_PKT2CPU_INTPRI_MAP,
	.qm_rsn2cpuqid_ctrl = RTETH_838X_QM_PKT2CPU_INTPRI_0,
	.qm_rsn2cpuqid_cnt = RTETH_838X_QM_PKT2CPU_INTPRI_CNT,
	.dma_if_ctrl = RTETH_838X_DMA_IF_CTRL,
	.dma_if_intr_sts = RTETH_838X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTETH_838X_DMA_IF_INTR_MSK,
	.dma_if_rx_ring_cntr = RTETH_838X_DMA_IF_RX_RING_CNTR,
	.dma_if_rx_ring_size = RTETH_838X_DMA_IF_RX_RING_SIZE,
	.dma_rx_base = RTETH_838X_DMA_RX_BASE,
	.dma_tx_base = RTETH_838X_DMA_TX_BASE,
	.mac_force_mode_ctrl = RTETH_838X_MAC_FORCE_MODE_CTRL,
	.rst_glb_ctrl = RTL838X_RST_GLB_CTRL_0,
	.skb_headroom = SKB_HEADROOM_SLOW,
	.mac_reg = { RTETH_838X_MAC_ADDR_CTRL,
		     RTETH_838X_MAC_ADDR_CTRL_ALE,
		     RTETH_838X_MAC_ADDR_CTRL_MAC },
	.l2_tbl_flush_ctrl = RTL838X_L2_TBL_FLUSH_CTRL,
	.confirm_and_disable_irqs = rteth_83xx_confirm_and_disable_irqs,
	.enable_rx_irq = rteth_83xx_enable_rx_irq,
	.update_counter = rteth_83xx_update_counter,
	.create_tx_header = rteth_838x_create_tx_header,
	.decode_tag = rteth_838x_decode_tag,
	.hw_en_rxtx = rteth_838x_hw_en_rxtx,
	.hw_init = &rteth_838x_hw_init,
	.hw_stop = &rteth_838x_hw_stop,
	.hw_reset = &rteth_838x_hw_reset,
	.init_mac = &rteth_838x_init_mac,
	.set_hol = rteth_83xx_set_hol,
	.set_max_packet_length = rteth_838x_set_max_packet_length,
	.netdev_ops = &rteth_838x_netdev_ops,
};

static const struct net_device_ops rteth_839x_netdev_ops = {
	.ndo_open = rteth_open,
	.ndo_stop = rteth_stop,
	.ndo_change_mtu = rteth_change_mtu,
	.ndo_start_xmit = rteth_start_xmit,
	.ndo_set_mac_address = rteth_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rteth_839x_set_rx_mode,
	.ndo_tx_timeout = rteth_tx_timeout,
	.ndo_set_features = rteth_83xx_set_features,
	.ndo_fix_features = rteth_fix_features,
	.ndo_setup_tc = rteth_setup_tc,
};

static const struct rteth_config rteth_839x_cfg = {
	.cpu_port = RTETH_839X_CPU_PORT,
	.max_mtu = RTETH_839X_MAX_FRAME - RTETH_FRAME_OVERHEAD,
	.rx_rings = 8,
	.tx_rx_enable = 0xc,
	.tx_trigger_mask = BIT(1),
	.mac_l2_port_ctrl = RTETH_839X_MAC_L2_PORT_CTRL,
	.qm_pkt2cpu_intpri_map = RTETH_839X_QM_PKT2CPU_INTPRI_MAP,
	.qm_rsn2cpuqid_ctrl = RTETH_839X_QM_PKT2CPU_INTPRI_0,
	.qm_rsn2cpuqid_cnt = RTETH_839X_QM_PKT2CPU_INTPRI_CNT,
	.dma_if_ctrl = RTETH_839X_DMA_IF_CTRL,
	.dma_if_intr_sts = RTETH_839X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTETH_839X_DMA_IF_INTR_MSK,
	.dma_if_rx_ring_cntr = RTETH_839X_DMA_IF_RX_RING_CNTR,
	.dma_if_rx_ring_size = RTETH_839X_DMA_IF_RX_RING_SIZE,
	.dma_rx_base = RTETH_839X_DMA_RX_BASE,
	.dma_tx_base = RTETH_839X_DMA_TX_BASE,
	.mac_force_mode_ctrl = RTETH_839X_MAC_FORCE_MODE_CTRL,
	.rst_glb_ctrl = RTL839X_RST_GLB_CTRL,
	.skb_headroom = SKB_HEADROOM_FAST,
	.mac_reg = { RTETH_839X_MAC_ADDR_CTRL },
	.l2_tbl_flush_ctrl = RTL839X_L2_TBL_FLUSH_CTRL,
	.confirm_and_disable_irqs = rteth_83xx_confirm_and_disable_irqs,
	.enable_rx_irq = rteth_83xx_enable_rx_irq,
	.update_counter = rteth_83xx_update_counter,
	.create_tx_header = rteth_839x_create_tx_header,
	.decode_tag = rteth_839x_decode_tag,
	.hw_en_rxtx = rteth_839x_hw_en_rxtx,
	.hw_init = &rteth_839x_hw_init,
	.hw_stop = &rteth_839x_hw_stop,
	.hw_reset = &rteth_839x_hw_reset,
	.init_mac = &rteth_839x_init_mac,
	.set_hol = rteth_83xx_set_hol,
	.set_max_packet_length = rteth_839x_set_max_packet_length,
	.setup_notify_ring_buffer = &rteth_839x_setup_notify_ring_buffer,
	.netdev_ops = &rteth_839x_netdev_ops,
};

static const struct net_device_ops rteth_930x_netdev_ops = {
	.ndo_open = rteth_open,
	.ndo_stop = rteth_stop,
	.ndo_change_mtu = rteth_change_mtu,
	.ndo_start_xmit = rteth_start_xmit,
	.ndo_set_mac_address = rteth_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rteth_930x_set_rx_mode,
	.ndo_tx_timeout = rteth_tx_timeout,
	.ndo_set_features = rteth_93xx_set_features,
	.ndo_fix_features = rteth_fix_features,
	.ndo_setup_tc = rteth_setup_tc,
};

static const struct rteth_config rteth_930x_cfg = {
	.cpu_port = RTETH_930X_CPU_PORT,
	.max_mtu = RTETH_930X_MAX_FRAME - RTETH_FRAME_OVERHEAD,
	.rx_rings = 32,
	.tx_rx_enable = 0x30,
	.tx_trigger_mask = GENMASK(3, 2),
	.mac_l2_port_ctrl = RTETH_930X_MAC_L2_PORT_CTRL,
	.qm_rsn2cpuqid_ctrl = RTETH_930X_QM_RSN2CPUQID_CTRL_0,
	.qm_rsn2cpuqid_cnt = RTETH_930X_QM_RSN2CPUQID_CTRL_CNT,
	.dma_if_ctrl = RTETH_930X_DMA_IF_CTRL,
	.dma_if_intr_sts = RTETH_930X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTETH_930X_DMA_IF_INTR_MSK,
	.dma_if_rx_ring_cntr = RTETH_930X_DMA_IF_RX_RING_CNTR,
	.dma_if_rx_ring_size = RTETH_930X_DMA_IF_RX_RING_SIZE,
	.dma_rx_base = RTETH_930X_DMA_RX_BASE,
	.dma_tx_base = RTETH_930X_DMA_TX_BASE,
	.l2_ntfy_if_intr_sts = RTL930X_L2_NTFY_IF_INTR_STS,
	.l2_ntfy_if_intr_msk = RTL930X_L2_NTFY_IF_INTR_MSK,
	.mac_force_mode_ctrl = RTETH_930X_MAC_FORCE_MODE_CTRL,
	.rst_glb_ctrl = RTL930X_RST_GLB_CTRL_0,
	.skb_headroom = SKB_HEADROOM_FAST,
	.mac_reg = { RTETH_930X_MAC_L2_ADDR_CTRL },
	.l2_tbl_flush_ctrl = RTL930X_L2_TBL_FLUSH_CTRL,
	.confirm_and_disable_irqs = rteth_93xx_confirm_and_disable_irqs,
	.enable_rx_irq = rteth_93xx_enable_rx_irq,
	.update_counter = rteth_93xx_update_counter,
	.create_tx_header = rteth_93xx_create_tx_header,
	.decode_tag = rteth_93xx_decode_tag,
	.hw_en_rxtx = rteth_930x_hw_en_rxtx,
	.hw_init = &rteth_930x_hw_init,
	.hw_stop = &rteth_930x_hw_stop,
	.hw_reset = &rteth_93xx_hw_reset,
	.init_mac = &rteth_930x_init_mac,
	.set_hol = rteth_93xx_set_hol,
	.set_max_packet_length = rteth_930x_set_max_packet_length,
	.netdev_ops = &rteth_930x_netdev_ops,
};

static const struct net_device_ops rteth_931x_netdev_ops = {
	.ndo_open = rteth_open,
	.ndo_stop = rteth_stop,
	.ndo_change_mtu = rteth_change_mtu,
	.ndo_start_xmit = rteth_start_xmit,
	.ndo_set_mac_address = rteth_set_mac_address,
	.ndo_validate_addr = eth_validate_addr,
	.ndo_set_rx_mode = rteth_931x_set_rx_mode,
	.ndo_tx_timeout = rteth_tx_timeout,
	.ndo_set_features = rteth_93xx_set_features,
	.ndo_fix_features = rteth_fix_features,
	.ndo_setup_tc = rteth_setup_tc,
};

static const struct rteth_config rteth_931x_cfg = {
	.cpu_port = RTETH_931X_CPU_PORT,
	.max_mtu = RTETH_931X_MAX_FRAME - RTETH_FRAME_OVERHEAD,
	.rx_rings = 32,
	.tx_rx_enable = 0x30,
	.tx_trigger_mask = GENMASK(3, 2),
	.mac_l2_port_ctrl = RTETH_931X_MAC_L2_PORT_CTRL,
	.qm_rsn2cpuqid_ctrl = RTETH_931X_QM_RSN2CPUQID_CTRL_0,
	.qm_rsn2cpuqid_cnt = RTETH_931X_QM_RSN2CPUQID_CTRL_CNT,
	.dma_if_ctrl = RTETH_931X_DMA_IF_CTRL,
	.dma_if_intr_sts = RTETH_931X_DMA_IF_INTR_STS,
	.dma_if_intr_msk = RTETH_931X_DMA_IF_INTR_MSK,
	.dma_if_rx_ring_cntr = RTETH_931X_DMA_IF_RX_RING_CNTR,
	.dma_if_rx_ring_size = RTETH_931X_DMA_IF_RX_RING_SIZE,
	.dma_rx_base = RTETH_931X_DMA_RX_BASE,
	.dma_tx_base = RTETH_931X_DMA_TX_BASE,
	.l2_ntfy_if_intr_sts = RTL931X_L2_NTFY_IF_INTR_STS,
	.l2_ntfy_if_intr_msk = RTL931X_L2_NTFY_IF_INTR_MSK,
	.mac_force_mode_ctrl = RTETH_931X_MAC_FORCE_MODE_CTRL,
	.rst_glb_ctrl = RTL931X_RST_GLB_CTRL,
	.skb_headroom = SKB_HEADROOM_FAST,
	.mac_reg = { RTETH_930X_MAC_L2_ADDR_CTRL },
	.l2_tbl_flush_ctrl = RTL931X_L2_TBL_FLUSH_CTRL,
	.confirm_and_disable_irqs = rteth_93xx_confirm_and_disable_irqs,
	.enable_rx_irq = rteth_93xx_enable_rx_irq,
	.update_counter = rteth_93xx_update_counter,
	.create_tx_header = rteth_93xx_create_tx_header,
	.decode_tag = rteth_93xx_decode_tag,
	.hw_en_rxtx = rteth_931x_hw_en_rxtx,
	.hw_init = &rteth_931x_hw_init,
	.hw_stop = &rteth_931x_hw_stop,
	.hw_reset = &rteth_93xx_hw_reset,
	.init_mac = &rteth_931x_init_mac,
	.set_hol = rteth_93xx_set_hol,
	.set_max_packet_length = rteth_931x_set_max_packet_length,
	.netdev_ops = &rteth_931x_netdev_ops,
};

static const struct phylink_mac_ops rteth_mac_ops = {
	.mac_config = rteth_mac_config,
	.mac_link_down = rteth_mac_link_down,
	.mac_link_up = rteth_mac_link_up,
};

static const struct ethtool_ops rteth_ethtool_ops = {
	.get_link_ksettings = rteth_get_link_ksettings,
	.set_link_ksettings = rteth_set_link_ksettings,
};

static int rteth_metadata_dst_alloc(struct rteth_ctrl *ctrl)
{
	struct metadata_dst *md_dst;

	for (int i = 0; i < ARRAY_SIZE(ctrl->dsa_meta); i++) {
		md_dst = metadata_dst_alloc(0, METADATA_HW_PORT_MUX, GFP_KERNEL);
		if (!md_dst)
			return -ENOMEM;

		md_dst->u.port_info.port_id = i;
		ctrl->dsa_meta[i] = md_dst;
	}

	return 0;
}

static void rteth_metadata_dst_free(struct rteth_ctrl *ctrl)
{
	for (int i = 0; i < ARRAY_SIZE(ctrl->dsa_meta); i++) {
		if (!ctrl->dsa_meta[i])
			continue;

		metadata_dst_free(ctrl->dsa_meta[i]);
	}
}

static int rteth_probe(struct platform_device *pdev)
{
	struct page_pool_params pp_params = {
		.order = 0,
		.flags = PP_FLAG_DMA_MAP | PP_FLAG_DMA_SYNC_DEV,
		.pool_size = PPOOL_SIZE,
		.max_len = PAGE_SIZE,
		.nid = dev_to_node(&pdev->dev),
		.dev = &pdev->dev,
		.dma_dir = DMA_FROM_DEVICE,
	};

	struct device_node *dn = pdev->dev.of_node;
	const struct rteth_config *cfg;
	u8 mac_addr[ETH_ALEN] = {0};
	phy_interface_t phy_mode;
	struct rteth_ctrl *ctrl;
	struct net_device *dev;
	int err = 0;

	pr_info("Probing RTL838X eth device pdev: %x, dev: %x\n",
		(u32)pdev, (u32)(&pdev->dev));

	cfg = device_get_match_data(&pdev->dev);

	dev = devm_alloc_etherdev_mqs(&pdev->dev, sizeof(struct rteth_ctrl), RTETH_TX_RINGS, RTETH_RX_RINGS);
	if (!dev)
		return -ENOMEM;
	SET_NETDEV_DEV(dev, &pdev->dev);

	ctrl = netdev_priv(dev);
	ctrl->pdev = pdev;
	ctrl->dev = dev;
	ctrl->r = cfg;

	ctrl->map = syscon_node_to_regmap(dn->parent);
	if (IS_ERR(ctrl->map))
		return PTR_ERR(ctrl->map);

	/* Allocate buffer memory */
	ctrl->membase = dmam_alloc_coherent(&pdev->dev, sizeof(struct notify_b),
					    (void *)&dev->mem_start, GFP_KERNEL);
	if (!ctrl->membase) {
		dev_err(&pdev->dev, "cannot allocate DMA buffer\n");
		return -ENOMEM;
	}

	ctrl->rx_data = dmam_alloc_coherent(&pdev->dev, sizeof(struct rteth_rx_data) * RTETH_RX_RINGS,
					    &ctrl->rx_dma, GFP_KERNEL);
	if (!ctrl->rx_data)
		return dev_err_probe(&pdev->dev, -ENOMEM, "failed to allocate RX ring memory\n");

	ctrl->tx_data = dmam_alloc_coherent(&pdev->dev, sizeof(struct rteth_tx_data) * RTETH_TX_RINGS,
					    &ctrl->tx_dma, GFP_KERNEL);
	if (!ctrl->tx_data)
		return dev_err_probe(&pdev->dev, -ENOMEM, "failed to allocate TX ring memory\n");

	spin_lock_init(&ctrl->lock);
	spin_lock_init(&ctrl->rx_lock);
	spin_lock_init(&ctrl->tx_lock);
	INIT_WORK(&ctrl->reset_work, rteth_reset_work);

	dev->ethtool_ops = &rteth_ethtool_ops;
	dev->min_mtu = ETH_ZLEN;
	dev->max_mtu = ctrl->r->max_mtu;
	dev->features = NETIF_F_RXCSUM;
	dev->hw_features = NETIF_F_RXCSUM;
	dev->netdev_ops = ctrl->r->netdev_ops;

	/* Obtain device IRQ number */
	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0)
		return dev_err_probe(&pdev->dev, dev->irq, "could not determine interrupt\n");

	rteth_disable_all_irqs(ctrl);
	err = devm_request_irq(&pdev->dev, dev->irq, rteth_net_irq, IRQF_SHARED, dev->name, dev);
	if (err)
		return dev_err_probe(&pdev->dev, err, "could not acquire interrupt\n");

	err = ctrl->r->init_mac(ctrl);
	if (err)
		return dev_err_probe(&pdev->dev, err, "failed to initialize MAC\n");

	/* Try to get mac address in the following order:
	 * 1) from device tree data
	 * 2) from internal registers set by bootloader
	 */
	err = of_get_mac_address(pdev->dev.of_node, mac_addr);
	if (err == -EPROBE_DEFER)
		return err;

	if (is_valid_ether_addr(mac_addr)) {
		rteth_set_mac_hw(dev, mac_addr);
	} else {
		u32 mac_hi, mac_lo;

		regmap_read(ctrl->map, ctrl->r->mac_reg[0], &mac_hi);
		regmap_read(ctrl->map, ctrl->r->mac_reg[0] + 4, &mac_lo);

		mac_addr[0] = (mac_hi >> 8) & 0xff;
		mac_addr[1] = mac_hi & 0xff;
		mac_addr[2] = (mac_lo >> 24) & 0xff;
		mac_addr[3] = (mac_lo >> 16) & 0xff;
		mac_addr[4] = (mac_lo >> 8) & 0xff;
		mac_addr[5] = mac_lo & 0xff;
	}
	dev_addr_set(dev, mac_addr);
	/* if the address is invalid, use a random value */
	if (!is_valid_ether_addr(dev->dev_addr)) {
		struct sockaddr sa = { AF_UNSPEC };

		dev_warn(&pdev->dev, "Invalid MAC address, using random\n");
		eth_hw_addr_random(dev);
		memcpy(sa.sa_data, dev->dev_addr, ETH_ALEN);
		if (rteth_set_mac_address(dev, &sa))
			dev_warn(&pdev->dev, "Failed to set MAC address.\n");
	}
	dev_info(&pdev->dev, "Using MAC %pM\n", dev->dev_addr);
	strscpy(dev->name, "eth%d", sizeof(dev->name));

	for (int i = 0; i < RTETH_RX_RINGS; i++) {
		ctrl->rx_info[i].id = i;
		ctrl->rx_info[i].ctrl = ctrl;
		netif_napi_add(dev, &ctrl->rx_info[i].napi, rteth_poll_rx);
	}

	platform_set_drvdata(pdev, dev);

	phy_mode = PHY_INTERFACE_MODE_NA;
	err = of_get_phy_mode(dn, &phy_mode);
	if (err < 0) {
		err = dev_err_probe(&pdev->dev, err, "incorrect phy-mode\n");
		goto cleanup;
	}

	ctrl->phylink_config.dev = &dev->dev;
	ctrl->phylink_config.type = PHYLINK_NETDEV;
	ctrl->phylink_config.mac_capabilities =
		MAC_10 | MAC_100 | MAC_1000FD |	MAC_SYM_PAUSE | MAC_ASYM_PAUSE;

	__set_bit(PHY_INTERFACE_MODE_INTERNAL, ctrl->phylink_config.supported_interfaces);

	ctrl->phylink = phylink_create(&ctrl->phylink_config, pdev->dev.fwnode,
				       phy_mode, &rteth_mac_ops);
	if (IS_ERR(ctrl->phylink)) {
		err = dev_err_probe(&pdev->dev, PTR_ERR(ctrl->phylink),
				     "could not create phylink\n");
		ctrl->phylink = NULL;
		goto cleanup;
	}

	for (int i = 0; i < RTETH_RX_RINGS; i++) {
		pp_params.napi = &ctrl->rx_info[i].napi;
		ctrl->rx_info[i].pool = page_pool_create(&pp_params);
		if (IS_ERR(ctrl->rx_info[i].pool)) {
			err = dev_err_probe(&pdev->dev, PTR_ERR(ctrl->rx_info[i].pool),
					    "Failed to create page pool for ring %d\n", i);
			ctrl->rx_info[i].pool = NULL;
			goto cleanup;
		}
	}

	err = rteth_metadata_dst_alloc(ctrl);
	if (err)
		goto cleanup;

	err = register_netdev(dev);
	if (err)
		goto cleanup;

	return 0;

cleanup:
	rteth_metadata_dst_free(ctrl);
	if (ctrl->phylink)
		phylink_destroy(ctrl->phylink);
	for (int i = 0; i < RTETH_RX_RINGS; i++) {
		netif_napi_del(&ctrl->rx_info[i].napi);
		if (ctrl->rx_info[i].pool)
			page_pool_destroy(ctrl->rx_info[i].pool);
	}

	return err;
}

static void rteth_remove(struct platform_device *pdev)
{
	struct net_device *dev = platform_get_drvdata(pdev);
	struct rteth_ctrl *ctrl = netdev_priv(dev);

	pr_info("Removing platform driver for rtl838x-eth\n");
	unregister_netdev(dev);
	cancel_work_sync(&ctrl->reset_work);
	rteth_metadata_dst_free(ctrl);

	if (ctrl->phylink)
		phylink_destroy(ctrl->phylink);

	for (int i = 0; i < RTETH_RX_RINGS; i++) {
		netif_napi_del(&ctrl->rx_info[i].napi);
		if (ctrl->rx_info[i].pool)
			page_pool_destroy(ctrl->rx_info[i].pool);
	}
}

static const struct of_device_id rteth_of_ids[] = {
	{ .compatible = "realtek,rtl8380-eth", .data = &rteth_838x_cfg, },
	{ .compatible = "realtek,rtl8392-eth", .data = &rteth_839x_cfg, },
	{ .compatible = "realtek,rtl9301-eth", .data = &rteth_930x_cfg, },
	{ .compatible = "realtek,rtl9311-eth", .data = &rteth_931x_cfg, },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, rteth_of_ids);

static struct platform_driver rtl838x_eth_driver = {
	.probe  = rteth_probe,
	.remove = rteth_remove,
	.driver = {
		.name = KBUILD_MODNAME,
		.pm = NULL,
		.of_match_table = rteth_of_ids,
	},
};

module_platform_driver(rtl838x_eth_driver);

MODULE_AUTHOR("B. Koblitz");
MODULE_DESCRIPTION("RTL838X SoC Ethernet Driver");
MODULE_LICENSE("GPL");
