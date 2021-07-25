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
#ifndef __QDPC_INIT_H_
#define __QDPC_INIT_H_

#include <linux/workqueue.h>

#define QDPC_MODULE_NAME                 "qdpc_ruby"
#define QDPC_DEV_NAME                    "qdpc_ruby"
#define QDPC_MODULE_VERSION              "1.0"

#define	SRAM_TEXT				__sram_text
#define	SRAM_DATA				__sram_data

/*
 * Netlink Message types.
 */
#define QDPC_NETLINK_RPC_PCI              31
#define QDPC_NETLINK_TYPE_SVC_REGISTER    10
#define QDPC_NETLINK_TYPE_SVC_RESPONSE    11

#define qdpc_phys_addr(x)     ((u32)(x) & 0x0fffffff)

extern struct  net_device  *qdpc_ndev;
extern uint8_t qdpc_basemac[ETH_ALEN];

typedef struct qdpc_config {
	uint32_t	cf_usdma:1,
				cf_dsdma:1,
				cf_msi:1,
				cf_64bit:1;

	uint32_t	cf_usdma_ndesc;
	uint32_t	cf_dsdma_ndesc;
	uint32_t	cf_miipllclk;
} qdpc_config_t;

/* This stores an anchor to packet buffers. */

struct qdpc_priv {
	struct net_device		*ndev;		/* points net device */
	struct net_device_stats		stats;		/* Network statistics */
	struct tasklet_struct		rx_tasklet;	/* Tasklet scheduled in interrupt handler */
	struct tasklet_struct		txd_tasklet;	/* Tasklet scheduled in interrupt handler */
	struct tasklet_struct		dmastatus_tasklet;
	struct timer_list		txq_enable_timer;	/* timer for enable tx */
	uint32_t			dsisr_status;
	uint32_t			usisr_status;
	uint32_t			dsdma_status;
	uint32_t			usdma_status;
	uint32_t			dsdma_desc;
	uint32_t			usdma_desc;
	uint16_t			host_msi_data;	/* MSI data */
	struct work_struct		init_work;	/* INIT handshake work */
	void				(*ep2host_irq)(struct qdpc_priv*);
	qdpc_pktring_t			pktq;
	uint32_t			mii_pllclk;
	uint32_t			*ep2host_irqstatus;
	uint32_t			*host2ep_irqstatus;
	int32_t				*host_ep2h_txd_budget;
	int32_t				*host_h2ep_txd_budget;
	qdpc_epshmem_hdr_t		*shmem;
	dma_addr_t			shmem_busaddr;
	struct sock			*netlink_socket;
	uint32_t			netlink_pid;
	qdpc_config_t			ep_config;
	spinlock_t			lock;		/* Private structure lock */
	qdpc_pcie_bda_t			*bda;
	uint32_t			msiaddr;
};

#define le32_readl(x)		le32_to_cpu(readl((x)))
#define le32_writel(x, addr)	writel(cpu_to_le32((x)), addr)

static inline void qdpc_pcie_posted_write(uint32_t val, void *basereg)
{
	writel(val, basereg);
	/* flush posted write */
	readl(basereg);
}

static inline int qdpc_isbootstate(struct qdpc_priv *p, uint32_t state) {
	__iomem uint32_t *status = &p->bda->bda_bootstate;
	uint32_t s = le32_readl(status);
	return ( s == state);
}

/* Function prototypes */
void qdpc_veth_rx(struct net_device  *ndev);
void qdpc_pcie_irqsetup(struct net_device *ndev);
void qdpc_disable_irqs(struct qdpc_priv *priv);
void qdpc_enable_irqs(struct qdpc_priv *priv);
void qdpc_init_work(struct work_struct *task);
void qdpc_free_interrupt(struct net_device *ndev);
int qdpc_pcie_init_intr(struct net_device *ndev);
void qdpc_disable_irqs(struct qdpc_priv *priv);
void qdpc_enable_irqs(struct qdpc_priv *priv);
int qdpc_pcie_init_mem(struct net_device *ndev);
struct sk_buff *qdpc_get_skb(size_t len);
int qdpc_init_rxq(struct qdpc_priv *priv, size_t ringsize);
int qdpc_init_txq(struct qdpc_priv *priv, size_t ringsize);
void qdpc_veth_txdone(struct net_device *ndev);
void qdpc_netlink_rx(struct net_device *ndev, void *buf, size_t len);
void qdpc_intr_ep2host(struct qdpc_priv *priv, uint32_t intr);
int32_t qdpc_send_packet(struct sk_buff *skb, struct net_device *ndev);

#endif /*__QDPC_INIT_H_ */
