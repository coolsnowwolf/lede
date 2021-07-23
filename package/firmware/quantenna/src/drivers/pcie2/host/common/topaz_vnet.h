/**
 * Copyright (c) 2012-2012 Quantenna Communications, Inc.
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

#ifndef __DRIVERS_NET_TOPAZ_VNET_H
#define __DRIVERS_NET_TOPAZ_VNET_H	1

#define ETH_TX_TIMEOUT (100*HZ)
#define MULTICAST_FILTER_LIMIT 64

#include <linux/slab.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>

#include <qdpc_config.h>
#include <topaz_netcom.h>

#define PROC_NAME_SIZE		(32)
#define VMAC_BD_EMPTY		((uint32_t)0x00000001)
#define VMAC_BD_WRAP		((uint32_t)0x00000002)
#define VMAC_BD_MASK_LEN	((uint32_t)0xFFFF0000)
#define VMAC_BD_MASK_OFFSET ((uint32_t)0x0000FF00)

#define VMAC_GET_LEN(x)		(((x) >> 16) & 0xFFFF)
#define VMAC_GET_OFFSET(x)	(((x) >> 8) & 0xFF)
#define VMAC_SET_LEN(len)	(((len) & 0xFFFF) << 16)
#define VMAC_SET_OFFSET(of)	(((of) & 0xFF) << 8)

#define VMAC_INDX_MINUS(x, y, m) (((x) + (m) - (y)) % (m))
#define VMAC_INDX_INC(index, m) do {	\
	if (++(index) >= (m))	\
		(index) = 0;	\
	} while(0)

/*
 * Helper macros handling memory mapped area access
 */
#define VMAC_REG_TST(reg, val) ( *((volatile unsigned int *)(reg)) & (val) )
#define VMAC_REG_SET(reg, val) { volatile unsigned int *r = (unsigned int *)(reg); *r = (*r | (val)); }
#define VMAC_REG_CLR(reg, val) { volatile unsigned int *r = (unsigned int *)(reg); *r = (*r & ~(val)); }
#define VMAC_REG_WRITE(reg, val) { *(volatile unsigned int *)reg = (val); }
#define VMAC_REG_READ(reg) {*(volatile unsigned int *)(reg); }

#define QTN_RC_TX_BUDGET		(16)
#define QTN_RC_TX_TASKLET_BUDGET	(32)

#define QTN_RX_SKB_FREELIST_FILL_SIZE	(1024)
#define QTN_RX_SKB_FREELIST_MAX_SIZE	(8192)
#define QTN_RX_BUF_MIN_SIZE		(1536)

#define VMAC_NL_BUF_SIZE		USHRT_MAX

typedef struct qdpc_bar {
        void *b_vaddr; /* PCIe bar virtual address */
        dma_addr_t b_busaddr; /* PCIe bar physical address */
        size_t b_len; /* Bar resource length */
        uint32_t b_offset; /* Offset from start of map */
        uint8_t b_index; /* Bar Index */
} qdpc_bar_t;

#define QDPC_BAR_VADDR(bar, off) ((bar).b_vaddr +(off))

struct vmac_cfg {
	uint16_t rx_bd_num;
	uint16_t tx_bd_num;
	char ifname[PROC_NAME_SIZE];
	struct net_device *dev;
};

#if defined(QTN_RC_ENABLE_HDP)
enum pkt_type {
        PKT_SKB = 0,
        PKT_TQE
};
#endif

struct vmac_tx_buf {
        uint32_t handle;
        uint16_t len;
#if defined(QTN_RC_ENABLE_HDP)
        uint8_t type; /* 1 payload only, 0 skb */
        uint8_t rsv;
#else
	uint16_t rsv;
#endif
};

struct vmac_priv {
	struct sk_buff **tx_skb;/* skb having post to PCIe DMA */
	volatile struct vmac_bd *tx_bd_base; /* Tx buffer descriptor */
	volatile uint32_t *ep_next_rx_pkt;
	uint16_t tx_bd_index;
	uint16_t tx_reclaim_start;
	uint16_t tx_bd_num;
	uint8_t txqueue_stopped;
	volatile uint32_t *txqueue_wake; /* shared variable with EP */
	spinlock_t txqueue_op_lock;
	unsigned long ep_ipc_reg;
	uint32_t tx_bd_busy_cnt; /* tx BD unavailable */
	uint32_t tx_stop_queue_cnt;
#ifdef RC_TXDONE_TIMER
	struct timer_list tx_timer;
	spinlock_t tx_lock;
#endif
	uint32_t vmac_tx_queue_len;

	struct sk_buff **rx_skb;
	volatile struct vmac_bd *rx_bd_base; /* Rx buffer descriptor  */
	uint16_t rx_bd_index;
	uint16_t rx_bd_num;

	uint32_t rx_skb_alloc_failures;
	uint32_t intr_cnt; /* msi/legacy interrupt counter */
	uint32_t vmac_xmit_cnt;
	uint32_t vmac_skb_free;

	struct sock *nl_socket;
	uint32_t str_call_nl_pid;
	uint32_t lib_call_nl_pid;
	struct napi_struct napi;

	uint32_t dbg_flg;

	struct net_device *ndev;
	struct pci_dev	*pdev;

	int mac_id;

	uint32_t dma_msi_imwr;
	uint32_t dma_msi_data;
	uint32_t dma_msi_dummy;
	uint32_t ep_pciecfg0_val; /* used to deassert Legacy irq from RC */

	/* The following members aren't related to datapath */
	struct vmac_cfg *pcfg;
	uint8_t show_item;

	uint32_t addr_uncache;
	uint32_t uncache_len;

	struct task_struct *init_thread; /* Initialization thread */
	uint8_t msi_enabled; /* PCIe MSI: 1 - Enabled, 0 - Disabled */

	qdpc_bar_t sysctl_bar;
	qdpc_bar_t epmem_bar;
	qdpc_bar_t dmareg_bar;

	uint32_t dma_imwr;

	/* io memory pointers */
	__iomem qdpc_pcie_bda_t *bda;
	uint32_t ep_ready;

#ifdef QCA_NSS_PLATFORM
	void *nssctx;              /* QCA NSS platform specific handler*/
	uint32_t nss_enable;
	struct ctl_table nss_sysctls[14]; /* Some system control for control and statistic */
	struct ctl_table_header *nss_sysctl_header;
#endif
#ifdef DNI_EXTRA_FUNCTIONS
#ifdef DNI_5G_LED
	uint32_t led_enable;
	struct ctl_table dni_sysctls[14];
	struct ctl_table_header *dni_sysctl_header;
#endif
#endif

#ifdef QTN_TX_SKBQ_SUPPORT
	struct sk_buff_head tx_skb_queue;
	spinlock_t tx_skbq_lock;
	struct tasklet_struct tx_skbq_tasklet;
	uint32_t tx_skbq_budget;
	uint32_t tx_skbq_tasklet_budget;
	uint32_t tx_skbq_max_size;
#endif

#ifdef QTN_SKB_RECYCLE_SUPPORT
	struct sk_buff_head rx_skb_freelist;
	spinlock_t rx_skb_freelist_lock;
	uint32_t rx_skb_freelist_fill_level;
	uint32_t skb_recycle_cnt;
	uint32_t skb_recycle_failures;
#endif

	volatile uint32_t *ep_pmstate;
	uint8_t *nl_buf;
	size_t nl_len;
};

#define QTN_DISABLE_SOFTIRQ		(0xABCD)

static inline void qtn_spin_lock_bh_save(spinlock_t *lock, unsigned long *flag)
{
	if (likely(irqs_disabled() || in_softirq())) {
		spin_lock(lock);
		*flag = 0;
	} else {
		spin_lock_bh(lock);
		*flag = QTN_DISABLE_SOFTIRQ;
        }
}

static inline void qtn_spin_unlock_bh_restore(spinlock_t *lock, unsigned long *flag)
{
	if (unlikely(*flag == QTN_DISABLE_SOFTIRQ)) {
		*flag = 0;
		spin_unlock_bh(lock);
	} else {
		spin_unlock(lock);
	}
}

extern struct net_device *vmac_alloc_ndev(void);
extern int vmac_net_init(struct pci_dev *pdev);
extern void vmac_clean(struct net_device *ndev);
extern int vmac_tx(void *pkt_handle, struct net_device *ndev);

#define PCIE_REG_CFG_BASE		0x0
#define PCIE_LOGIC_PORT_CFG_BASE	(PCIE_REG_CFG_BASE + 0x700)
#define PCIE_DMA_WR_INTR_MASK		0x2c4

void vmac_pcie_edma_enable(struct vmac_priv *priv);
void qdpc_deassert_intx(struct vmac_priv *priv);
void qdpc_pcie_edma_enable(struct vmac_priv *priv);
int qdpc_pcie_suspend(struct pci_dev *pdev, pm_message_t state);
int qdpc_pcie_resume(struct pci_dev *pdev);
#endif
