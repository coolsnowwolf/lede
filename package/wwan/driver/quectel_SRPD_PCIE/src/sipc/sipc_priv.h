/*
 * Copyright (C) 2019 Spreadtrum Communications Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __SIPC_PRIV_H
#define __SIPC_PRIV_H
#include <linux/ktime.h>
#include <linux/device.h>
#include <linux/interrupt.h>

#include "../include/sprd_mpm.h"
#ifdef CONFIG_SPRD_MAILBOX
#include <linux/sprd_mailbox.h>
#endif

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
#include "../include/sprd_pcie_ep_device.h"
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
#include <linux/pcie-epf-sprd.h>
#endif

#ifdef CONFIG_PCI
#include "../include/sprd_pcie_resource.h"
#endif

#include "../include/sipc_big_to_little.h"

#ifndef SZ_1K
#define SZ_1K 0x00000400
#define SZ_4K 0x00001000
#endif

enum {
	SIPC_BASE_MBOX = 0,
	SIPC_BASE_PCIE,
	SIPC_BASE_IPI,
	SIPC_BASE_NR
};

enum smem_type {
	SMEM_LOCAL = 0,
	SMEM_PCIE
};

extern struct smsg_ipc *smsg_ipcs[];
#define SMSG_CACHE_NR		256

struct smsg_channel {
	/* wait queue for recv-buffer */
	wait_queue_head_t	rxwait;
	struct mutex		rxlock;
	struct sprd_pms	*tx_pms;
	struct sprd_pms	*rx_pms;
	char		tx_name[16];
	char		rx_name[16];

	/* cached msgs for recv */
	uintptr_t		wrptr[1];
	uintptr_t		rdptr[1];
	struct smsg		caches[SMSG_CACHE_NR];
};

/* smsg ring-buffer between AP/CP ipc */
struct smsg_ipc {
	const char	*name;
	struct sprd_pms	*sipc_pms;

	u8	dst;
	u8	client;	/* sipc is  client mode */
	/* target core_id over mailbox */
	u8	core_id;
	u8	core_sensor_id;
	u32	type; /* sipc type, mbox, ipi, pcie */

	void __iomem	*write_addr;

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	u32	ep_dev;
	struct platform_device	*pcie_dev;
#endif
#ifdef CONFIG_PCIE_EPF_SPRD
	u32	ep_fun;
#endif
	u32	latency;

	/* send-buffer info */
	uintptr_t	txbuf_addr;
	u32		txbuf_size;	/* must be 2^n */
	uintptr_t	txbuf_rdptr;
	uintptr_t	txbuf_wrptr;

	/* recv-buffer info */
	uintptr_t	rxbuf_addr;
	u32		rxbuf_size;	/* must be 2^n */
	uintptr_t	rxbuf_rdptr;
	uintptr_t	rxbuf_wrptr;

	/* sipc irq related */
	int	irq;
	u32	(*rxirq_status)(u8 id);
	void	(*rxirq_clear)(u8 id);
	void	(*txirq_trigger)(u8 id, u64 msg);

	u32	ring_base;
	u32	ring_size;
	void	*smem_vbase;
	u32	smem_base;
	u32	smem_size;
	enum smem_type	smem_type;
	u32	dst_smem_base;
#ifdef CONFIG_PHYS_ADDR_T_64BIT
	u32	high_offset;
#endif
	/* lock for send-buffer */
	spinlock_t		txpinlock;
	/* all fixed channels receivers */
	struct smsg_channel	*channels[SMSG_VALID_CH_NR];
	/* record the runtime status of smsg channel */
	atomic_t		busy[SMSG_VALID_CH_NR];
	/* all channel states: 0 unused, 1 be opened by other core, 2 opend */
	u8			states[SMSG_VALID_CH_NR];
};

#define CHAN_STATE_UNUSED		0
#define CHAN_STATE_CLIENT_OPENED	1
#define CHAN_STATE_HOST_OPENED		2
#define CHAN_STATE_OPENED		3
#define CHAN_STATE_FREE			4

void smsg_init_channel2index(void);
void smsg_ipc_create(struct smsg_ipc *ipc);
void smsg_ipc_destroy(struct smsg_ipc *ipc);

/*smem alloc size align*/
#define SMEM_ALIGN_POOLSZ 0x40000	/*256KB*/

#ifdef CONFIG_64BIT
#define SMEM_ALIGN_BYTES	8
#define SMEM_MIN_ORDER		3
#else
#define SMEM_ALIGN_BYTES	4
#define SMEM_MIN_ORDER		2
#endif

/* initialize smem pool for AP/CP */
int smem_init(u32 addr, u32 size, u32 dst, u32 mem_type);
void sbuf_get_status(u8 dst, char *status_info, int size);

#if defined(CONFIG_DEBUG_FS)
void sipc_debug_putline(struct seq_file *m, char c, int n);
#endif

#ifdef CONFIG_SPRD_MAILBOX
#define MBOX_INVALID_CORE  0xff
#endif

/* sipc_smem_request_resource
 * local smem no need request resource, just return 0.
 */
static inline int sipc_smem_request_resource(struct sprd_pms *pms,
					     u8 dst, int timeout)
{
	if (smsg_ipcs[dst]->smem_type == SMEM_LOCAL)
		return 0;

	return sprd_pms_request_resource(pms, timeout);
}

/* sipc_smem_release_resource
 * local smem no need release resource, do nothing.
 */
static inline void sipc_smem_release_resource(struct sprd_pms *pms, u8 dst)
{
	if (smsg_ipcs[dst]->smem_type != SMEM_LOCAL)
		sprd_pms_release_resource(pms);
}
#endif
