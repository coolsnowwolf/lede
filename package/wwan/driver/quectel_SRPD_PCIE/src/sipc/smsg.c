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

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <linux/syscore_ops.h>
#include <linux/sched.h>

#ifdef CONFIG_SPRD_MAILBOX
#include <linux/sprd_mailbox.h>
#endif

#include "../include/sipc.h"
#include "sipc_priv.h"
#include "../include/sprd_pcie_resource.h"

#if defined(CONFIG_DEBUG_FS)
#include "sipc_debugfs.h"
#endif

#define SMSG_TXBUF_ADDR		(0)
#define SMSG_TXBUF_SIZE		(SZ_1K)
#define SMSG_RXBUF_ADDR		(SMSG_TXBUF_SIZE)
#define SMSG_RXBUF_SIZE		(SZ_1K)

#define SMSG_RINGHDR		(SMSG_TXBUF_SIZE + SMSG_RXBUF_SIZE)
#define SMSG_TXBUF_RDPTR	(SMSG_RINGHDR + 0)
#define SMSG_TXBUF_WRPTR	(SMSG_RINGHDR + 4)
#define SMSG_RXBUF_RDPTR	(SMSG_RINGHDR + 8)
#define SMSG_RXBUF_WRPTR	(SMSG_RINGHDR + 12)

#define SMSG_RESERVE_BASE	(SMSG_RINGHDR + SZ_1K)
#define SMSG_PCIE_WRPTR		(SMSG_RESERVE_BASE + 0)
#define SMSG_PCIE_IRQPTR	(SMSG_RESERVE_BASE + 4)

#define SIPC_READL(addr)      readl((__force void __iomem *)(addr))
#define SIPC_WRITEL(b, addr)  writel(b, (__force void __iomem *)(addr))

static u8 g_wakeup_flag;

struct smsg_ipc *smsg_ipcs[SIPC_ID_NR];
EXPORT_SYMBOL_GPL(smsg_ipcs);

static ushort debug_enable;

module_param_named(debug_enable, debug_enable, ushort, 0644);
static u8 channel2index[SMSG_CH_NR + 1];

static int smsg_ipc_smem_init(struct smsg_ipc *ipc);

void smsg_init_channel2index(void)
{
	u16 i, j;

	for (i = 0; i < ARRAY_SIZE(channel2index); i++) {
		for (j = 0; j < SMSG_VALID_CH_NR; j++) {
			/* find the index of channel i */
			if (sipc_cfg[j].channel == i)
				break;
		}

		/* if not find, init with INVALID_CHANEL_INDEX,
		 * else init whith j
		 */
		if (j == SMSG_VALID_CH_NR)
			channel2index[i] = INVALID_CHANEL_INDEX;
		else
			channel2index[i] = j;
	}
}

static void get_channel_status(u8 dst, char *status, int size)
{
	int i, len;
	struct smsg_channel *ch;

	len = strlen(status);
	for (i = 0;  i < SMSG_VALID_CH_NR && len < size; i++) {
		ch = smsg_ipcs[dst]->channels[i];
		if (!ch)
			continue;
		if (SIPC_READL(ch->rdptr) < SIPC_READL(ch->wrptr))
			snprintf(
				status + len,
				size - len,
				"dst-%d-ch-%d: rd = %u, wr = %u.\n",
				dst,
				i,
				SIPC_READL(ch->rdptr),
				SIPC_READL(ch->wrptr)
				);
	}
}

static void smsg_wakeup_print(struct smsg_ipc *ipc, struct smsg *msg)
{
	/* if the first msg come after the irq wake up by sipc,
	 * use prin_fo to output log
	 */
	if (g_wakeup_flag) {
		g_wakeup_flag = 0;
		pr_info("irq read smsg: dst=%d, channel=%d,type=%d, flag=0x%04x, value=0x%08x\n",
			ipc->dst,
			msg->channel,
			msg->type,
			msg->flag,
			msg->value);
	} else {
		pr_debug("irq read smsg: dst=%d, channel=%d,type=%d, flag=0x%04x, value=0x%08x\n",
			 ipc->dst,
			 msg->channel,
			 msg->type,
			 msg->flag,
			 msg->value);
	}
}

static void smsg_die_process(struct smsg_ipc *ipc, struct smsg *msg)
{
	if (msg->type == SMSG_TYPE_DIE) {
		if (debug_enable) {
			char sipc_status[100] = {0};

			get_channel_status(ipc->dst,
					   sipc_status,
					   sizeof(sipc_status));
			sbuf_get_status(ipc->dst,
					sipc_status,
					sizeof(sipc_status));
			panic("cpcrash: %s", sipc_status);
			while (1)
				;
		}
	}
}

static void smsg_msg_process(struct smsg_ipc *ipc,
			     struct smsg *msg, bool wake_lock)
{
	struct smsg_channel *ch = NULL;
	u32 wr;
	u8 ch_index;

	smsg_wakeup_print(ipc, msg);
	smsg_die_process(ipc, msg);

	ch_index = channel2index[msg->channel];
	atomic_inc(&ipc->busy[ch_index]);

	pr_debug("smsg:get dst=%d msg channel=%d, type=%d, flag=0x%04x, value=0x%08x\n",
		 ipc->dst, msg->channel,
		 msg->type, msg->flag,
		 msg->value);

	if (msg->type >= SMSG_TYPE_NR) {
		/* invalid msg */
		pr_err("invalid smsg: channel=%d, type=%d, flag=0x%04x, value=0x%08x\n",
			msg->channel, msg->type, msg->flag, msg->value);
		goto exit_msg_proc;
	}

	ch = ipc->channels[ch_index];
	if (!ch) {
		if (ipc->states[ch_index] == CHAN_STATE_UNUSED &&
			msg->type == SMSG_TYPE_OPEN &&
			msg->flag == SMSG_OPEN_MAGIC)
			ipc->states[ch_index] = CHAN_STATE_CLIENT_OPENED;
		else
			/* drop this bad msg since channel
			 * is not opened
			 */
			pr_info("smsg channel %d not opened! drop smsg: type=%d, flag=0x%04x, value=0x%08x\n",
				msg->channel, msg->type,
				msg->flag, msg->value);

		goto exit_msg_proc;
	}

	if ((int)(SIPC_READL(ch->wrptr) - SIPC_READL(ch->rdptr)) >=
		SMSG_CACHE_NR) {
		/* msg cache is full, drop this msg */
		pr_info("smsg channel %d recv cache is full! drop smsg: type=%d, flag=0x%04x, value=0x%08x\n",
			msg->channel, msg->type, msg->flag, msg->value);
	} else {
		/* write smsg to cache */
		wr = SIPC_READL(ch->wrptr) & (SMSG_CACHE_NR - 1);
		memcpy(&ch->caches[wr], msg, sizeof(struct smsg));
		SIPC_WRITEL(SIPC_READL(ch->wrptr) + 1, ch->wrptr);
	}

	wake_up_interruptible_all(&ch->rxwait);

	if (wake_lock)
		sprd_pms_request_wakelock_period(ch->rx_pms, 500);

exit_msg_proc:
	atomic_dec(&ipc->busy[ch_index]);
}

#ifdef CONFIG_SPRD_MAILBOX
static irqreturn_t smsg_mbox_irq_handler(void *ptr, void *private)
{
	struct smsg_ipc *ipc = (struct smsg_ipc *)private;
	struct smsg *msg;

	msg = ptr;
	smsg_msg_process(ipc, msg, true);

	return IRQ_HANDLED;
}

static irqreturn_t smsg_mbox_sensor_irq_handler(void *ptr, void *private)
{
	struct smsg_ipc *ipc = (struct smsg_ipc *)private;
	struct smsg *msg;

	msg = ptr;
	smsg_msg_process(ipc, msg, false);

	return IRQ_HANDLED;
}

#endif

static int sipc_process_all_msg(struct smsg_ipc *ipc)
{
	struct smsg *msg;
	struct smsg msg_recv;
	uintptr_t rxpos;

	/* msg coming, means resource ok, don't wait */
	sipc_smem_request_resource(ipc->sipc_pms, ipc->dst, 0);

	while (BL_READL(ipc->rxbuf_wrptr) != BL_READL(ipc->rxbuf_rdptr)) {
		rxpos = (BL_READL(ipc->rxbuf_rdptr) & (ipc->rxbuf_size - 1)) *
			sizeof(struct smsg) + ipc->rxbuf_addr;
		msg = (struct smsg *)rxpos;

		/* share memory smsg to ipc msg */
		msg_recv.channel = msg->channel;
		msg_recv.type = msg->type;
		msg_recv.flag = BL_GETW(msg->flag);
		msg_recv.value= BL_GETL(msg->value);
		smsg_msg_process(ipc, &msg_recv, true);
		/* update smsg rdptr */
		BL_WRITEL(BL_READL(ipc->rxbuf_rdptr) + 1, ipc->rxbuf_rdptr);
	}

	sipc_smem_release_resource(ipc->sipc_pms, ipc->dst);

	return 0;
}


static irqreturn_t smsg_irq_handler(int irq, void *private)
{
	struct smsg_ipc *ipc = (struct smsg_ipc *)private;

	if (ipc->rxirq_status(ipc->dst))
		ipc->rxirq_clear(ipc->dst);

	sipc_process_all_msg(ipc);

	return IRQ_HANDLED;
}

static void smsg_ipc_init_smsg_irq_callback(struct smsg_ipc *ipc)
{
#ifdef CONFIG_SPRD_MAILBOX
	if (ipc->type == SIPC_BASE_MBOX) {
		mbox_register_irq_handle(ipc->core_id,
			smsg_mbox_irq_handler, ipc);

		if ((ipc->dst == SIPC_ID_PM_SYS) &&
		    (ipc->core_sensor_id != MBOX_INVALID_CORE))
			mbox_register_irq_handle(ipc->core_sensor_id,
						 smsg_mbox_sensor_irq_handler,
						 ipc);
		return;
	}
#endif

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
	if (ipc->type == SIPC_BASE_PCIE) {
		sprd_ep_dev_register_irq_handler(ipc->ep_dev,
			PCIE_MSI_SIPC_IRQ,
			smsg_irq_handler, ipc);
		sprd_ep_dev_set_irq_addr(ipc->ep_dev, ipc->write_addr + 4);
		return;
	}
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
	if (ipc->type == SIPC_BASE_PCIE) {
		sprd_pci_epf_register_irq_handler(ipc->ep_fun,
				   PCIE_DBELL_SIPC_IRQ,
				   smsg_irq_handler,
				   ipc);
		sprd_pci_epf_set_write_addr(ipc->ep_fun, ipc->write_addr);
		return;
	}
#endif

	if (ipc->type == SIPC_BASE_IPI) {
		int ret;

		/* explicitly call irq handler in case of missing irq on boot */
		smsg_irq_handler(ipc->irq, ipc);

		/* register IPI irq */
		ret = request_irq(ipc->irq,
				   smsg_irq_handler,
				   IRQF_NO_SUSPEND,
				   ipc->name,
				   ipc);
		if (ret)
			pr_info("%s: request irq err = %d!\n", ipc->name, ret);
	}
}

static int smsg_ipc_smem_init(struct smsg_ipc *ipc)
{
	void __iomem *base, *p;
	phys_addr_t offset = 0;
	int ret;

	pr_debug("%s: %s, smem_type = %d!\n",
		__func__, ipc->name, ipc->smem_type);

	ret = smem_init(ipc->smem_base, ipc->smem_size,
			ipc->dst, ipc->smem_type);
	if (ret) {
		pr_err("%s: %s err = %d!\n", __func__, ipc->name, ret);
		return ret;
	}

	if (ipc->type != SIPC_BASE_MBOX) {
		ipc->ring_base = smem_alloc(ipc->dst, SZ_4K);
		ipc->ring_size = SZ_4K;
		pr_info("%s: ring_base = 0x%x, ring_size = 0x%x\n",
				__func__,
				ipc->ring_base,
				ipc->ring_size);
	}

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	offset = ipc->high_offset;
	offset = offset << 32;
#endif

	if (ipc->ring_base) {
		base = (void __iomem *)shmem_ram_vmap_nocache(ipc->dst,
					ipc->ring_base + offset,
					ipc->ring_size);
		if (!base) {
			pr_err("%s: ioremap failed!\n", __func__);
			smem_free(ipc->dst, ipc->ring_base, SZ_4K);
			ipc->ring_base = 0;
			return -ENOMEM;
		}

		/* assume client is boot later than host */
		if (!ipc->client) {
			/**
			 * memset(base, 0, ipc->ring_size);
			 * the instruction dc avz
			 * will abort for nocache memory
			 */
			for (p = base; p < base + ipc->ring_size;) {
#ifdef CONFIG_64BIT
				*(uint64_t *)p = 0x0;
				p += sizeof(uint64_t);
#else
				*(u32 *)p = 0x0;
				p += sizeof(u32);
#endif
			}
		}

		if (ipc->client) {
			/* clent mode, tx is host rx , rx is host tx*/
			ipc->smem_vbase = (void *)base;
			ipc->txbuf_size = SMSG_RXBUF_SIZE /
				sizeof(struct smsg);
			ipc->txbuf_addr = (uintptr_t)base +
				SMSG_RXBUF_ADDR;
			ipc->txbuf_rdptr = (uintptr_t)base +
				SMSG_RXBUF_RDPTR;
			ipc->txbuf_wrptr = (uintptr_t)base +
				SMSG_RXBUF_WRPTR;
			ipc->rxbuf_size = SMSG_TXBUF_SIZE /
				sizeof(struct smsg);
			ipc->rxbuf_addr = (uintptr_t)base +
				SMSG_TXBUF_ADDR;
			ipc->rxbuf_rdptr = (uintptr_t)base +
				SMSG_TXBUF_RDPTR;
			ipc->rxbuf_wrptr = (uintptr_t)base +
				SMSG_TXBUF_WRPTR;
		} else {
			ipc->smem_vbase = (void *)base;
			ipc->txbuf_size = SMSG_TXBUF_SIZE /
				sizeof(struct smsg);
			ipc->txbuf_addr = (uintptr_t)base +
				SMSG_TXBUF_ADDR;
			ipc->txbuf_rdptr = (uintptr_t)base +
				SMSG_TXBUF_RDPTR;
			ipc->txbuf_wrptr = (uintptr_t)base +
				SMSG_TXBUF_WRPTR;
			ipc->rxbuf_size = SMSG_RXBUF_SIZE /
				sizeof(struct smsg);
			ipc->rxbuf_addr = (uintptr_t)base +
				SMSG_RXBUF_ADDR;
			ipc->rxbuf_rdptr = (uintptr_t)base +
				SMSG_RXBUF_RDPTR;
			ipc->rxbuf_wrptr = (uintptr_t)base +
				SMSG_RXBUF_WRPTR;
		}
		ipc->write_addr = base + SMSG_PCIE_WRPTR;
	}

	/* after smem_init complete, regist msg irq */
	smsg_ipc_init_smsg_irq_callback(ipc);

	return 0;
}

#ifdef CONFIG_PCIE_EPF_SPRD
static void smsg_pcie_first_ready(void *data)
{
	struct smsg_ipc *ipc = (struct smsg_ipc *)data;

	if (ipc->smem_type == SMEM_PCIE)
		smsg_ipc_smem_init(ipc);
	else
		pr_err("%s: pcie first ready, smem_type =%d!\n",
			ipc->name, ipc->smem_type);
}
#endif

static void smsg_ipc_mpm_init(struct smsg_ipc *ipc)
{
	/* create modem power manger instance for this sipc */
	sprd_mpm_create(ipc->dst, ipc->name, ipc->latency);

	/* init a power manager source */
	ipc->sipc_pms = sprd_pms_create(ipc->dst, ipc->name, true);
	if (!ipc->sipc_pms)
		pr_warn("create pms %s failed!\n", ipc->name);

	if (ipc->type == SIPC_BASE_PCIE) {
		/* int mpm resource ops */
		sprd_mpm_init_resource_ops(ipc->dst,
			sprd_pcie_wait_resource,
			sprd_pcie_request_resource,
			sprd_pcie_release_resource);

#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
		/* in pcie host side, init pcie host resource */
		sprd_pcie_resource_host_init(ipc->dst,
					     ipc->ep_dev, ipc->pcie_dev);
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
		/* in pcie ep side, init pcie client resource */
		sprd_pcie_resource_client_init(ipc->dst, ipc->ep_fun);
#endif
	}
}

void smsg_ipc_create(struct smsg_ipc *ipc)
{
	pr_info("%s: %s\n", __func__, ipc->name);

	smsg_ipcs[ipc->dst] = ipc;

	smsg_ipc_mpm_init(ipc);


	if (ipc->type == SIPC_BASE_PCIE) {
#ifdef CONFIG_PCIE_EPF_SPRD
		/* set epf door bell irq number */
		sprd_pci_epf_set_irq_number(ipc->ep_fun, ipc->irq);

		/* register first pcie ready notify */
		sprd_register_pcie_resource_first_ready(ipc->dst,
							smsg_pcie_first_ready,
							ipc);
#endif
	}

	/* if SMEM_PCIE, must init after pcie ready */
	if (ipc->smem_type != SMEM_PCIE)
		smsg_ipc_smem_init(ipc);
}

void smsg_ipc_destroy(struct smsg_ipc *ipc)
{
	shmem_ram_unmap(ipc->dst, ipc->smem_vbase);
	smem_free(ipc->dst, ipc->ring_base, SZ_4K);

#ifdef CONFIG_SPRD_MAILBOX
	if (ipc->type == SIPC_BASE_MBOX) {
		mbox_unregister_irq_handle(ipc->core_id);

	if ((ipc->dst == SIPC_ID_PM_SYS) &&
	    (ipc->core_sensor_id != MBOX_INVALID_CORE))
		mbox_unregister_irq_handle(ipc->core_sensor_id);
	}
#endif

	if (ipc->type == SIPC_BASE_PCIE) {
#ifdef CONFIG_SPRD_PCIE_EP_DEVICE
		sprd_ep_dev_unregister_irq_handler(ipc->ep_dev, ipc->irq);
#endif

#ifdef CONFIG_PCIE_EPF_SPRD
		sprd_pci_epf_unregister_irq_handler(ipc->ep_fun, ipc->irq);
#endif
		sprd_pcie_resource_trash(ipc->dst);
	} else {
		free_irq(ipc->irq, ipc);
	}

	smsg_ipcs[ipc->dst] = NULL;
}

int sipc_get_wakeup_flag(void)
{
	return (int)g_wakeup_flag;
}
EXPORT_SYMBOL_GPL(sipc_get_wakeup_flag);

void sipc_set_wakeup_flag(void)
{
	g_wakeup_flag = 1;
}
EXPORT_SYMBOL_GPL(sipc_set_wakeup_flag);

void sipc_clear_wakeup_flag(void)
{
	g_wakeup_flag = 0;
}
EXPORT_SYMBOL_GPL(sipc_clear_wakeup_flag);

int smsg_ch_wake_unlock(u8 dst, u8 channel)
{
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	struct smsg_channel *ch;
	u8 ch_index;

	ch_index = channel2index[channel];
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}

	if (!ipc)
		return -ENODEV;

	ch = ipc->channels[ch_index];
	if (!ch)
		return -ENODEV;

	sprd_pms_release_wakelock(ch->rx_pms);
	return 0;
}
EXPORT_SYMBOL_GPL(smsg_ch_wake_unlock);

int smsg_ch_open(u8 dst, u8 channel, int timeout)
{
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	struct smsg_channel *ch;
	struct smsg mopen;
	struct smsg mrecv;
	int rval = 0;
	u8 ch_index;

	ch_index = channel2index[channel];
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}

	if (!ipc)
		return -ENODEV;

	ch = kzalloc(sizeof(*ch), GFP_KERNEL);
	if (!ch)
		return -ENOMEM;

	sprintf(ch->tx_name, "smsg-%d-%d-tx", dst, channel);
	ch->tx_pms = sprd_pms_create(dst, ch->tx_name, true);
	if (!ch->tx_pms)
		pr_warn("create pms %s failed!\n", ch->tx_name);

	sprintf(ch->rx_name, "smsg-%d-%d-rx", dst, channel);
	ch->rx_pms = sprd_pms_create(dst, ch->rx_name, true);
	if (!ch->rx_pms)
		pr_warn("create pms %s failed!\n", ch->rx_name);

	atomic_set(&ipc->busy[ch_index], 1);
	init_waitqueue_head(&ch->rxwait);
	mutex_init(&ch->rxlock);
	ipc->channels[ch_index] = ch;

	pr_info("%s: channel %d-%d send open msg!\n",
		__func__, dst, channel);

	smsg_set(&mopen, channel, SMSG_TYPE_OPEN, SMSG_OPEN_MAGIC, 0);
	rval = smsg_send(dst, &mopen, timeout);
	if (rval != 0) {
		pr_err("%s: channel %d-%d send open msg error = %d!\n",
		       __func__, dst, channel, rval);
		ipc->states[ch_index] = CHAN_STATE_UNUSED;
		ipc->channels[ch_index] = NULL;
		atomic_dec(&ipc->busy[ch_index]);
		/* guarantee that channel resource isn't used in irq handler  */
		while (atomic_read(&ipc->busy[ch_index]))
			;

		kfree(ch);

		return rval;
	}

	/* open msg might be got before */
	if (ipc->states[ch_index] == CHAN_STATE_CLIENT_OPENED)
		goto open_done;

	ipc->states[ch_index] = CHAN_STATE_HOST_OPENED;

	do {
		smsg_set(&mrecv, channel, 0, 0, 0);
		rval = smsg_recv(dst, &mrecv, timeout);
		if (rval != 0) {
			pr_err("%s: channel %d-%d smsg receive error = %d!\n",
			       __func__, dst, channel, rval);
			ipc->states[ch_index] = CHAN_STATE_UNUSED;
			ipc->channels[ch_index] = NULL;
			atomic_dec(&ipc->busy[ch_index]);
			/* guarantee that channel resource isn't used
			 * in irq handler
			 */
			while (atomic_read(&ipc->busy[ch_index]))
				;

			kfree(ch);
			return rval;
		}
	} while (mrecv.type != SMSG_TYPE_OPEN || mrecv.flag != SMSG_OPEN_MAGIC);

	pr_info("%s: channel %d-%d receive open msg!\n",
		__func__, dst, channel);

open_done:
	pr_info("%s: channel %d-%d success\n", __func__, dst, channel);
	ipc->states[ch_index] = CHAN_STATE_OPENED;
	atomic_dec(&ipc->busy[ch_index]);

	return 0;
}
EXPORT_SYMBOL_GPL(smsg_ch_open);

int smsg_ch_close(u8 dst, u8 channel,  int timeout)
{
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	struct smsg_channel *ch;
	struct smsg mclose;
	u8 ch_index;

	ch_index = channel2index[channel];
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}

	ch = ipc->channels[ch_index];
	if (!ch)
		return 0;

	smsg_set(&mclose, channel, SMSG_TYPE_CLOSE, SMSG_CLOSE_MAGIC, 0);
	smsg_send(dst, &mclose, timeout);

	ipc->states[ch_index] = CHAN_STATE_FREE;
	wake_up_interruptible_all(&ch->rxwait);

	/* wait for the channel being unused */
	while (atomic_read(&ipc->busy[ch_index]))
		;

	/* maybe channel has been free for smsg_ch_open failed */
	if (ipc->channels[ch_index]) {
		ipc->channels[ch_index] = NULL;
		/* guarantee that channel resource isn't used in irq handler */
		while (atomic_read(&ipc->busy[ch_index]))
			;
        sprd_pms_destroy(ch->rx_pms);
		sprd_pms_destroy(ch->tx_pms);
		kfree(ch);
	}

	/* finally, update the channel state*/
	ipc->states[ch_index] = CHAN_STATE_UNUSED;

	return 0;
}
EXPORT_SYMBOL_GPL(smsg_ch_close);

static void smsg_bl_cpoy_msg(struct smsg *dst, struct smsg *src)
{
	dst->channel = src->channel;
	dst->type = src->type;
	BL_SETW(dst->flag, src->flag);
	BL_SETL(dst->value, src->value);
}

int smsg_senddie(u8 dst)
{
	struct smsg msg;
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	uintptr_t txpos;
	int rval = 0;

	if (!ipc)
		return -ENODEV;

	msg.channel = SMSG_CH_CTRL;
	msg.type = SMSG_TYPE_DIE;
	msg.flag = 0;
	msg.value = 0;

#ifdef CONFIG_SPRD_MAILBOX
	if (ipc->type == SIPC_BASE_MBOX) {
		mbox_just_sent(ipc->core_id, *((u64 *)&msg));
		return 0;
	}
#endif

	if (ipc->ring_base) {
		/* must wait resource before read or write share memory */
		rval = sprd_pms_request_resource(ipc->sipc_pms, 0);
		if (rval < 0)
			return rval;

		if (((int)(BL_READL(ipc->txbuf_wrptr) -
				BL_READL(ipc->txbuf_rdptr)) >=
				ipc->txbuf_size)) {
			pr_info("%s: smsg txbuf is full!\n", __func__);
			rval = -EBUSY;
		} else {
			/* calc txpos and write smsg */
			txpos = (BL_READL(ipc->txbuf_wrptr) &
				(ipc->txbuf_size - 1)) *
				sizeof(struct smsg) + ipc->txbuf_addr;
			smsg_bl_cpoy_msg((void *)txpos, &msg);

			/* update wrptr */
			BL_WRITEL(BL_READL(ipc->txbuf_wrptr) + 1,
				    ipc->txbuf_wrptr);
		}
		ipc->txirq_trigger(ipc->dst, *((u64 *)&msg));
		sprd_pms_release_resource(ipc->sipc_pms);
	}

	return rval;
}
EXPORT_SYMBOL_GPL(smsg_senddie);

int smsg_send(u8 dst, struct smsg *msg, int timeout)
{
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	struct smsg_channel *ch;
	uintptr_t txpos;
	int rval = 0;
	unsigned long flags;
	u8 ch_index;

	ch_index = channel2index[msg->channel];
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, msg->channel);
		return -EINVAL;
	}

	if (!ipc)
		return -ENODEV;

	if (!ipc->channels[ch_index]) {
		pr_err("%s: channel %d not inited!\n", __func__, msg->channel);
		return -ENODEV;
	}

	if (ipc->states[ch_index] != CHAN_STATE_OPENED &&
	    msg->type != SMSG_TYPE_OPEN &&
	    msg->type != SMSG_TYPE_CLOSE) {
		pr_err("%s: channel %d not opened!\n", __func__, msg->channel);
		return -EINVAL;
	}

	ch = ipc->channels[ch_index];

	pr_debug("send smsg: channel=%d, type=%d, flag=0x%04x, value=0x%08x\n",
		 msg->channel, msg->type, msg->flag, msg->value);

	/*
	 * Must wait resource before read or write share memory,
	 * and must wait resource before trigger irq,
	 * And it must before if (ipc->ring_base),
	 * because it be inited as the same time as resource ready.
	 */
	rval = sprd_pms_request_resource(ch->tx_pms, timeout);
	if (rval < 0)
		return rval;

	if (ipc->ring_base) {
		spin_lock_irqsave(&ipc->txpinlock, flags);
		if (((int)(BL_READL(ipc->txbuf_wrptr) -
				BL_READL(ipc->txbuf_rdptr)) >=
				ipc->txbuf_size)) {
			pr_err("write smsg: txbuf full, wrptr=0x%x, rdptr=0x%x\n",
				 BL_READL(ipc->txbuf_wrptr),
				 BL_READL(ipc->txbuf_rdptr));
			rval = -EBUSY;
		} else {
			/* calc txpos and write smsg */
			txpos = (BL_READL(ipc->txbuf_wrptr) &
				 (ipc->txbuf_size - 1)) *
				sizeof(struct smsg) + ipc->txbuf_addr;
			smsg_bl_cpoy_msg((void *)txpos, msg);

			/* update wrptr */
			BL_WRITEL(BL_READL(ipc->txbuf_wrptr) + 1,
				    ipc->txbuf_wrptr);
		}
		spin_unlock_irqrestore(&ipc->txpinlock, flags);
	} else if (ipc->type != SIPC_BASE_MBOX) {
		pr_err("send smsg:ring_base is NULL");
		sprd_pms_release_resource(ch->tx_pms);
		return -EINVAL;
	}

	ipc->txirq_trigger(ipc->dst, *(u64 *)msg);
	sprd_pms_release_resource(ch->tx_pms);

	return rval;
}
EXPORT_SYMBOL_GPL(smsg_send);

int smsg_recv(u8 dst, struct smsg *msg, int timeout)
{
	struct smsg_ipc *ipc = smsg_ipcs[dst];
	struct smsg_channel *ch;
	u32 rd;
	int rval = 0;
	u8 ch_index;

	ch_index = channel2index[msg->channel];
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, msg->channel);
		return -EINVAL;
	}

	if (!ipc)
		return -ENODEV;

	atomic_inc(&ipc->busy[ch_index]);

	ch = ipc->channels[ch_index];

	if (!ch) {
		pr_err("%s: channel %d not opened!\n", __func__, msg->channel);
		atomic_dec(&ipc->busy[ch_index]);
		return -ENODEV;
	}

	pr_debug("%s: dst=%d, channel=%d, timeout=%d, ch_index = %d\n",
		 __func__, dst, msg->channel, timeout, ch_index);

	if (timeout == 0) {
		if (!mutex_trylock(&ch->rxlock)) {
			pr_err("dst=%d, channel=%d recv smsg busy!\n",
			       dst, msg->channel);
			atomic_dec(&ipc->busy[ch_index]);

			return -EBUSY;
		}

		/* no wait */
		if (SIPC_READL(ch->wrptr) == SIPC_READL(ch->rdptr)) {
			pr_info("dst=%d, channel=%d smsg rx cache is empty!\n",
				dst, msg->channel);

			rval = -ENODATA;

			goto recv_failed;
		}
	} else if (timeout < 0) {
	    mutex_lock_interruptible(&ch->rxlock);
		/* wait forever */
		rval = wait_event_interruptible(
				ch->rxwait,
				(SIPC_READL(ch->wrptr) !=
				 SIPC_READL(ch->rdptr)) ||
				(ipc->states[ch_index] == CHAN_STATE_FREE));
		if (rval < 0) {
			pr_debug("%s: dst=%d, channel=%d wait interrupted!\n",
				 __func__, dst, msg->channel);

			goto recv_failed;
		}

		if (ipc->states[ch_index] == CHAN_STATE_FREE) {
			pr_info("%s: dst=%d, channel=%d channel is free!\n",
				__func__, dst, msg->channel);

			rval = -EIO;

			goto recv_failed;
		}
	} else {
	    mutex_lock_interruptible(&ch->rxlock);
		/* wait timeout */
		rval = wait_event_interruptible_timeout(
			ch->rxwait,
			(SIPC_READL(ch->wrptr) != SIPC_READL(ch->rdptr)) ||
			(ipc->states[ch_index] == CHAN_STATE_FREE),
			timeout);
		if (rval < 0) {
			pr_debug("%s: dst=%d, channel=%d wait interrupted!\n",
				 __func__, dst, msg->channel);

			goto recv_failed;
		} else if (rval == 0) {
			pr_debug("%s: dst=%d, channel=%d wait timeout!\n",
				 __func__, dst, msg->channel);

			rval = -ETIME;

			goto recv_failed;
		}

		if (ipc->states[ch_index] == CHAN_STATE_FREE) {
			pr_info("%s: dst=%d, channel=%d channel is free!\n",
				__func__, dst, msg->channel);

			rval = -EIO;

			goto recv_failed;
		}
	}

	/* read smsg from cache */
	rd = SIPC_READL(ch->rdptr) & (SMSG_CACHE_NR - 1);
	memcpy(msg, &ch->caches[rd], sizeof(struct smsg));
	SIPC_WRITEL(SIPC_READL(ch->rdptr) + 1, ch->rdptr);

	if (ipc->ring_base)
		pr_debug("read smsg: dst=%d, channel=%d, wrptr=%d, rdptr=%d, rd=%d\n",
			 dst,
			 msg->channel,
			 SIPC_READL(ch->wrptr),
			 SIPC_READL(ch->rdptr),
			 rd);

	pr_debug("recv smsg: dst=%d, channel=%d, type=%d, flag=0x%04x, value=0x%08x, rval = %d\n",
		 dst, msg->channel, msg->type, msg->flag, msg->value, rval);

recv_failed:
	mutex_unlock(&ch->rxlock);
	atomic_dec(&ipc->busy[ch_index]);
	return rval;
}
EXPORT_SYMBOL_GPL(smsg_recv);

u8 sipc_channel2index(u8 channel)
{
	return channel2index[channel];
}
EXPORT_SYMBOL_GPL(sipc_channel2index);

#if defined(CONFIG_DEBUG_FS)
static int smsg_debug_show(struct seq_file *m, void *private)
{
	struct smsg_ipc *ipc = NULL;
	struct smsg_channel *ch;

	int i, j, cnt;

	for (i = 0; i < SIPC_ID_NR; i++) {
		ipc = smsg_ipcs[i];
		if (!ipc)
			continue;

		sipc_debug_putline(m, '*', 120);
		seq_printf(m, "sipc: %s:\n", ipc->name);
		seq_printf(m, "dst: 0x%0x, irq: 0x%0x\n",
			   ipc->dst, ipc->irq);
		if (ipc->ring_base) {
			/*
			 * must wait resource before
			 * read or write share memory.
			 */
			if (sipc_smem_request_resource(ipc->sipc_pms,
						       ipc->dst, 1000) < 0)
				continue;

			seq_printf(m, "txbufAddr: 0x%p, txbufsize: 0x%x, txbufrdptr: [0x%p]=%d, txbufwrptr: [0x%p]=%d\n",
				   (void *)ipc->txbuf_addr,
				   ipc->txbuf_size,
				   (void *)ipc->txbuf_rdptr,
				   BL_READL(ipc->txbuf_rdptr),
				   (void *)ipc->txbuf_wrptr,
				   BL_READL(ipc->txbuf_wrptr));
			seq_printf(m, "rxbufAddr: 0x%p, rxbufsize: 0x%x, rxbufrdptr: [0x%p]=%d, rxbufwrptr: [0x%p]=%d\n",
				   (void *)ipc->rxbuf_addr,
				   ipc->rxbuf_size,
				   (void *)ipc->rxbuf_rdptr,
				   BL_READL(ipc->rxbuf_rdptr),
				   (void *)ipc->rxbuf_wrptr,
				   BL_READL(ipc->rxbuf_wrptr));

			/* release resource */
			sipc_smem_release_resource(ipc->sipc_pms, ipc->dst);
		}
		sipc_debug_putline(m, '-', 80);
		seq_puts(m, "1. all channel state list:\n");

		for (j = 0; j < SMSG_VALID_CH_NR; j++)
			seq_printf(m,
				   "%2d. channel[%3d] states: %d, name: %s\n",
				   j,
				   sipc_cfg[j].channel,
				   ipc->states[j],
				   sipc_cfg[j].name);

		sipc_debug_putline(m, '-', 80);
		seq_puts(m, "2. channel rdpt < wrpt list:\n");

		cnt = 1;
		for (j = 0;  j < SMSG_VALID_CH_NR; j++) {
			ch = ipc->channels[j];
			if (!ch)
				continue;

			if (SIPC_READL(ch->rdptr) < SIPC_READL(ch->wrptr))
				seq_printf(m, "%2d. channel[%3d] rd: %d, wt: %d, name: %s\n",
					   cnt++,
					   sipc_cfg[j].channel,
					   SIPC_READL(ch->rdptr),
					   SIPC_READL(ch->wrptr),
					   sipc_cfg[j].name);
		}
	}
	return 0;
}

static int smsg_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, smsg_debug_show, inode->i_private);
}

static const struct file_operations smsg_debug_fops = {
	.open = smsg_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int smsg_init_debugfs(void *root)
{
	if (!root)
		return -ENXIO;
	debugfs_create_file("smsg", 0444,
			    (struct dentry *)root,
			    NULL,
			    &smsg_debug_fops);
	return 0;
}
EXPORT_SYMBOL_GPL(smsg_init_debugfs);

#endif /* CONFIG_DEBUG_FS */


MODULE_AUTHOR("Chen Gaopeng");
MODULE_DESCRIPTION("SIPC/SMSG driver");
MODULE_LICENSE("GPL v2");
