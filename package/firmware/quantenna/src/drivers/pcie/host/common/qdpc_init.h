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

#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>

#define QDPC_MODULE_NAME                 "qdpc_ruby"
#define QDPC_DEV_NAME                    "qdpc_ruby"
#define QDPC_MODULE_VERSION              "1.0"

/*
 * Netlink Message types.
 */
#define QDPC_NETLINK_RPC_PCI              31
#define QDPC_NETLINK_TYPE_CLNT_REGISTER   10
#define QDPC_NETLINK_TYPE_CLNT_REQUEST    11


/* PCIe device information declarations */
#define  QDPC_VENDOR_ID                   0x1bb5
#define  QDPC_DEVICE_ID                   0x0008
#define  QDPC_PCIE_NUM_BARS               6

typedef struct qdpc_bar {
	void		*b_vaddr;	/* PCIe bar virtual address */
	dma_addr_t	b_busaddr;	/* PCIe bar physical address */
	size_t   	b_len;		/* Bar resource length */
	uint32_t	b_offset;	/* Offset from start of map */
	uint8_t		b_index;	/* Bar Index */
} qdpc_bar_t;

#define QDPC_BAR_VADDR(bar, off) ((bar).b_vaddr +(off))

extern unsigned int (*qdpc_pci_readl)(void *addr);
extern void (*qdpc_pci_writel)(unsigned int val, void *addr);

/* Driver private housekeeping data structures. */
struct qdpc_priv {
	struct pci_dev			*pdev;             /* Points Pci device */
	struct net_device		*ndev;             /* points net device */
	struct net_device_stats		stats;             /* Network statistics */
	int				irq;               /* Interupt line  */
	struct tasklet_struct		rx_tasklet;        /* Tasklet scheduled in interrupt handler */
	struct tasklet_struct		txd_tasklet;        /* Transmit Done Tasklet scheduled in interrupt handler */
	struct task_struct		*init_thread;      /* Initialization thread */
	struct timer_list		txq_enable_timer;	  /* timer for enable tx */
	spinlock_t			lock;              /* Private structure lock */
	enum   qdpc_drv_state		init_done;         /* State of driver */
	int				msi_enabled;       /* Supports msi or not */
	qdpc_pktring_t			pktq;
	qdpc_dmadata_t			*dsdata;			  /* Downstream data */
	qdpc_dmadata_t			*usdata;			  /* Upstream data */

	struct sock			*netlink_socket;
	uint32_t			netlink_pid;
	qdpc_bar_t			sysctl_bar;
	qdpc_bar_t			epmem_bar;
	qdpc_epshmem_hdr_t		epmem;

	/* io memory pointers */
	__iomem qdpc_pcie_bda_t		*bda;
	__iomem uint32_t		*ep2host_irqstatus;	  /* IRQ Cause from EP */
	__iomem uint32_t		*host2ep_irqstatus;	  /* IRQ Cause from HOST */
	__iomem int32_t			*host_ep2h_txd_budget;
	__iomem int32_t			*host_h2ep_txd_budget;
};

/*
 * End-point(EP) is little-Endian.
 * These two macros are used for host side outbound window memory access.
 * Outbound here is host side view-point. So memory accessed by these two macros
 * should be on EP side.
 * NOTE: On some platforms, outbound hardware swap(byte order swap) should be
 * enabled for outbound memory access correctly. If enabled, Endian translation
 * will be done by hardware, and software Endian translation should be disabled.
 * */
#ifdef OUTBOUND_HW_SWAP
	#define le32_readl(x)           readl(x)
	#define le32_writel(x, addr)    writel(x, addr)
#else
	#define le32_readl(x)           le32_to_cpu(readl((x)))
	#define le32_writel(x, addr)    writel(cpu_to_le32((x)), addr)
#endif

static inline unsigned int qdpc_readl(void *addr)
{
	return readl(addr);
}
static inline void qdpc_writel(unsigned int val, void *addr)
{
	writel(val, addr);
}
static inline unsigned int qdpc_le32_readl(void *addr)
{
	return le32_to_cpu(readl((addr)));
}
static inline void qdpc_le32_writel(unsigned int val, void *addr)
{
	writel(cpu_to_le32((val)), addr);
}

static inline void qdpc_pcie_posted_write(uint32_t val, __iomem void *basereg)
{
	qdpc_pci_writel(val,basereg);
	/* flush posted write */
	qdpc_pci_readl(basereg);
}

static inline int qdpc_isbootstate(struct qdpc_priv *p, uint32_t state) {
	__iomem uint32_t *status = &p->bda->bda_bootstate;
	uint32_t s = qdpc_pci_readl(status);
	return (s == state);
}
static inline int qdpc_booterror(struct qdpc_priv *p) {
	__iomem uint32_t *status = &p->bda->bda_flags;
	uint32_t s = qdpc_pci_readl(status);
	return (s & PCIE_BDA_ERROR_MASK);
}
static inline void qdpc_setbootstate(struct qdpc_priv *p, uint32_t state) {
	__iomem qdpc_pcie_bda_t *bda = p->bda;

	qdpc_pcie_posted_write(state, &bda->bda_bootstate);
}

/* Function prototypes */
int qdpc_pcie_init_intr_and_mem(struct net_device *ndev);
int qdpc_dma_setup(struct qdpc_priv *priv);
void qdpc_interrupt_target(struct qdpc_priv *priv, uint32_t intr);
void qdpc_disable_irqs(struct qdpc_priv *priv);
void qdpc_enable_irqs(struct qdpc_priv *priv);
void qdpc_free_interrupt(struct pci_dev *pdev);
void qdpc_pcie_free_mem(struct pci_dev *pdev);
void qdpc_datapath_init(struct qdpc_priv *priv);
void qdpc_init_target_buffers(void *data);
void qdpc_veth_rx(struct net_device *ndev);
void qdpc_map_image_loc(struct qdpc_priv *priv);
uint32_t qdpc_host_to_ep_address(struct qdpc_priv *priv, void *addr);
void qdpc_veth_txdone(struct net_device *ndev);
int qdpc_send_packet(struct sk_buff *skb, struct net_device *ndev);
void *qdpc_map_pciemem(unsigned long busaddr, size_t len);
void qdpc_unmap_pciemem(unsigned long busaddr, void *vaddr, size_t len);
int qdpc_dmainit_rxq(struct qdpc_priv *priv);
int qdpc_dmainit_txq(struct qdpc_priv *priv);
int qdpc_unmap_iomem(struct qdpc_priv *priv);
int32_t qdpc_set_dma_mask(struct qdpc_priv *priv);
struct sk_buff *qdpc_get_skb(struct qdpc_priv *priv, size_t len);
void qdpc_datapath_uninit(struct qdpc_priv *priv);

#endif
