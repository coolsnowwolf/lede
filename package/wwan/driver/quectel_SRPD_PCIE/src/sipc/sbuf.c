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
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kthread.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <asm/pgtable.h>
#include <linux/sched.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,4,60 ))
#include <uapi/linux/sched/types.h>
#include <linux/sched/task.h>
#endif

#include "../include/sipc.h"
#include "sipc_priv.h"
#include "sbuf.h"

#if defined(CONFIG_DEBUG_FS)
#include "sipc_debugfs.h"
#endif

#define VOLA_SBUF_SMEM volatile struct sbuf_smem_header
#define VOLA_SBUF_RING volatile struct sbuf_ring_header

struct name_node {
	struct	list_head list;
	char	comm[TASK_COMM_LEN];
	pid_t	pid;
	u8	latest;
};

union sbuf_buf {
	void *buf;
	void __user *ubuf;
};

enum task_type {
	TASK_RXWAIT = 0,
	TASK_TXWAIT,
	TASK_SELECT
};

static struct sbuf_mgr *sbufs[SIPC_ID_NR][SMSG_VALID_CH_NR];

static bool sbuf_has_data(struct sbuf_ring *ring, u8 dst, bool tx)
{
	struct sbuf_ring_header_op *hd_op = &ring->header_op;
	bool has_data;
	unsigned long flags;

	/*
	 * if it is local share memory,
	 * check the read and write point directly.
	 */
	if (smsg_ipcs[dst]->smem_type == SMEM_LOCAL) {
		if (tx)
			return (int)(BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p)) <
				hd_op->tx_size;

		return BL_READL(hd_op->rx_wt_p) != BL_READL(hd_op->rx_rd_p);
	}

	/*
	 * if it is remote share memmory read the poll_mask,
	 * this situation requires that the poll_mask must be accurate enough.
	 */
	spin_lock_irqsave(&ring->poll_lock, flags);
	if (tx)
		has_data = ring->poll_mask & (POLLOUT | POLLWRNORM);
	else
		has_data = ring->poll_mask & (POLLIN | POLLRDNORM);
	spin_unlock_irqrestore(&ring->poll_lock, flags);

	return has_data;
}

static bool sbuf_is_task_pointer(const void *ptr)
{
	struct task_struct *task;
	struct thread_info *thread_info;

	task = (struct task_struct *)ptr;
	if (IS_ERR_OR_NULL(task) || !virt_addr_valid(task))
		return false;

#ifndef CONFIG_THREAD_INFO_IN_TASK
	/* in this case thread_info is in the same addres with stack thread_union*/
	if (IS_ERR_OR_NULL(task->stack) || !virt_addr_valid(task->stack))
		return false;
#endif

	thread_info = task_thread_info(task);

	if (IS_ERR_OR_NULL(thread_info) || !virt_addr_valid(thread_info))
		return false;

	return true;
}

static struct task_struct *sbuf_wait_get_task(
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
								wait_queue_t *pos,
#else
								wait_queue_entry_t *pos,
#endif
								u32 *b_select)
{
	struct task_struct *task;
	struct poll_wqueues *table;

	if (!pos->private)
		return NULL;

	/* if the private is put into wait list by sbuf_read, the struct of
	 * pos->private is struct task_struct
	 * if the private is put into list by sbuf_poll, the struct of
	 * pos->private is struct poll_wqueues
	 */

	/* firstly, try struct poll_wqueues */
	table = (struct poll_wqueues *)pos->private;
	task = table->polling_task;
	if (sbuf_is_task_pointer(task)) {
		*b_select = 1;
		return task;
	}

	/* firstly, try convert it with the struct task_struct */
	task = (struct task_struct *)pos->private;

	if (sbuf_is_task_pointer(task)) {
		*b_select = 0;
		return task;
	}

	return NULL;
}

#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
static void sbuf_record_rdwt_owner(struct sbuf_ring *ring, int b_rx)
{
	int b_add;
	int cnt = 0;
	struct name_node *pos = NULL;
	struct name_node *temp = NULL;
	struct list_head *owner_list;
	unsigned long flags;

	b_add = 1;
	owner_list = b_rx ? (&ring->rx_list) : (&ring->tx_list);

	spin_lock_irqsave(&ring->rxwait.lock, flags);
	list_for_each_entry(pos, owner_list, list) {
		cnt++;
		if (pos->pid == current->pid) {
			b_add = 0;
			pos->latest = 1;
			continue;
		}
		if (pos->latest)
			pos->latest = 0;
	}
	spin_unlock_irqrestore(&ring->rxwait.lock, flags);

	if (b_add) {
		/* delete head next */
		if (cnt == MAX_RECORD_CNT) {
			temp = list_first_entry(owner_list,
				struct name_node, list);
			list_del(&temp->list);
			kfree(temp);
		}

		pos = kzalloc(sizeof(*pos), GFP_KERNEL);
		if (pos) {
			memcpy(pos->comm, current->comm, TASK_COMM_LEN);
			pos->pid = current->pid;
			pos->latest = 1;
			spin_lock_irqsave(&ring->rxwait.lock, flags);
			list_add_tail(&pos->list, owner_list);
			spin_unlock_irqrestore(&ring->rxwait.lock, flags);
		}
	}
}

static void sbuf_destroy_rdwt_owner(struct sbuf_ring *ring)
{
	struct name_node *pos, *temp;
	unsigned long flags;

	spin_lock_irqsave(&ring->rxwait.lock, flags);
	/* free task node */
	list_for_each_entry_safe(pos,
				 temp,
				 &ring->rx_list,
				 list) {
		list_del(&pos->list);
		kfree(pos);
	}

	list_for_each_entry_safe(pos,
				 temp,
				 &ring->tx_list,
				 list) {
		list_del(&pos->list);
		kfree(pos);
	}
	spin_unlock_irqrestore(&ring->rxwait.lock, flags);
}
#endif

static void sbuf_skip_old_data(struct sbuf_mgr *sbuf)
{
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op = NULL;
	u32 i, v;
	unsigned long flags;

	ring = &sbuf->rings[0];
	/* must reques resource before read or write share memory */
	if (sipc_smem_request_resource(ring->rx_pms, sbuf->dst, -1) < 0)
		return;

	for (i = 0; i < sbuf->ringnr; i++) {
		ring = &sbuf->rings[i];
		hd_op = &ring->header_op;

		/* clean sbuf tx ring , sbuf tx ring no need to clear */
		/* *(hd_op->tx_wt_p) = *(hd_op->tx_rd_p); */
		/* clean sbuf rx ring */
		v = BL_READL(hd_op->rx_wt_p);
		BL_WRITEL(v, hd_op->rx_rd_p);
		/* restore write mask. */
		spin_lock_irqsave(&ring->poll_lock, flags);
		ring->poll_mask = POLLOUT | POLLWRNORM;
		spin_unlock_irqrestore(&ring->poll_lock, flags);
	}
	ring = &sbuf->rings[0];
	/* release resource */
	sipc_smem_release_resource(ring->rx_pms, sbuf->dst);
}

static void sbuf_pms_init(struct sbuf_ring *ring,
			  uint8_t dst, uint8_t ch, int index)
{
	ring->need_wake_lock = true;
	sprintf(ring->tx_pms_name, "sbuf-%d-%d-%d-tx", dst, ch, index);
	ring->tx_pms = sprd_pms_create(dst, ring->tx_pms_name, false);
	if (!ring->tx_pms)
		pr_warn("create pms %s failed!\n", ring->tx_pms_name);

	sprintf(ring->rx_pms_name, "sbuf-%d-%d-%d-rx", dst, ch, index);
	ring->rx_pms = sprd_pms_create(dst, ring->rx_pms_name, false);
	if (!ring->rx_pms)
		pr_warn("create pms %s failed!\n", ring->rx_pms_name);
}

static void sbuf_comm_init(struct sbuf_mgr *sbuf)
{
	u32 bufnum = sbuf->ringnr;
	int i;
	struct sbuf_ring *ring;

	for (i = 0; i < bufnum; i++) {
		ring = &sbuf->rings[i];
		init_waitqueue_head(&ring->txwait);
		init_waitqueue_head(&ring->rxwait);
#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
		INIT_LIST_HEAD(&ring->tx_list);
		INIT_LIST_HEAD(&ring->rx_list);
#endif
		mutex_init(&ring->txlock);
		mutex_init(&ring->rxlock);
		spin_lock_init(&ring->poll_lock);
		spin_lock_init(&ring->rxwait.lock);

		/* init, set write mask. */
		ring->poll_mask = POLLOUT | POLLWRNORM;
		sbuf_pms_init(ring, sbuf->dst, sbuf->channel, i);
	}
}

static int sbuf_host_init(struct smsg_ipc *sipc, struct sbuf_mgr *sbuf,
	u32 bufnum, u32 txbufsize, u32 rxbufsize)
{
	VOLA_SBUF_SMEM *smem;
	VOLA_SBUF_RING *ringhd;
	struct sbuf_ring_header_op *hd_op;
	int hsize, i, rval;
	phys_addr_t offset = 0;
	u8 dst = sbuf->dst;
	struct sbuf_ring *ring;
	u32 txbuf_addr;

	sbuf->ringnr = bufnum;

	/* allocate smem */
	hsize = sizeof(struct sbuf_smem_header) +
		sizeof(struct sbuf_ring_header) * bufnum;
	sbuf->smem_size = hsize + (txbufsize + rxbufsize) * bufnum;
	sbuf->smem_addr = smem_alloc(dst, sbuf->smem_size);
	if (!sbuf->smem_addr) {
		pr_err("%s: channel %d-%d, Failed to allocate smem for sbuf\n",
			__func__, sbuf->dst, sbuf->channel);
		return -ENOMEM;
	}
	sbuf->dst_smem_addr = sbuf->smem_addr - sipc->smem_base +
		sipc->dst_smem_base;

	pr_debug("%s: channel %d-%d, smem_addr=0x%x, smem_size=0x%x, dst_smem_addr=0x%x\n",
		 __func__,
		 sbuf->dst,
		 sbuf->channel,
		 sbuf->smem_addr,
		 sbuf->smem_size,
		 sbuf->dst_smem_addr);

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	offset = sipc->high_offset;
	offset = offset << 32;
#endif

	pr_info("%s: channel %d-%d, offset = 0x%lx!\n",
		__func__, sbuf->dst, sbuf->channel, (unsigned long)offset);
	sbuf->smem_virt = shmem_ram_vmap_nocache(dst,
						sbuf->smem_addr + offset,
						sbuf->smem_size);
	if (!sbuf->smem_virt) {
		pr_err("%s: channel %d-%d, Failed to map smem for sbuf\n",
			__func__, sbuf->dst, sbuf->channel);
		smem_free(dst, sbuf->smem_addr, sbuf->smem_size);
		return -EFAULT;
	}

	/* allocate rings description */
	sbuf->rings = kcalloc(bufnum, sizeof(struct sbuf_ring), GFP_KERNEL);
	if (!sbuf->rings) {
		smem_free(dst, sbuf->smem_addr, sbuf->smem_size);
		shmem_ram_unmap(dst, sbuf->smem_virt);
		return -ENOMEM;
	}

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(sipc->sipc_pms, sipc->dst, -1);
	if (rval < 0) {
		smem_free(dst, sbuf->smem_addr, sbuf->smem_size);
		shmem_ram_unmap(dst, sbuf->smem_virt);
		kfree(sbuf->rings);
		return rval;
	}

	/* initialize all ring bufs */
	smem = (VOLA_SBUF_SMEM *)sbuf->smem_virt;
	BL_SETL(smem->ringnr, bufnum);
	for (i = 0; i < bufnum; i++) {
		ringhd = (VOLA_SBUF_RING *)&smem->headers[i];
		txbuf_addr = sbuf->dst_smem_addr + hsize +
			(txbufsize + rxbufsize) * i;
		BL_SETL(ringhd->txbuf_addr, txbuf_addr);
		BL_SETL(ringhd->txbuf_size, txbufsize);
		BL_SETL(ringhd->txbuf_rdptr, 0);
		BL_SETL(ringhd->txbuf_wrptr, 0);
		BL_SETL(ringhd->rxbuf_addr, txbuf_addr + txbufsize);
		BL_SETL(ringhd->rxbuf_size, rxbufsize);
		BL_SETL(ringhd->rxbuf_rdptr, 0);
		BL_SETL(ringhd->rxbuf_wrptr, 0);

		ring = &sbuf->rings[i];
		ring->header = ringhd;
		ring->txbuf_virt = sbuf->smem_virt + hsize +
			(txbufsize + rxbufsize) * i;
		ring->rxbuf_virt = ring->txbuf_virt + txbufsize;
		/* init header op */
		hd_op = &ring->header_op;
		hd_op->rx_rd_p = &ringhd->rxbuf_rdptr;
		hd_op->rx_wt_p = &ringhd->rxbuf_wrptr;
		hd_op->rx_size = ringhd->rxbuf_size;
		hd_op->tx_rd_p = &ringhd->txbuf_rdptr;
		hd_op->tx_wt_p = &ringhd->txbuf_wrptr;
		hd_op->tx_size = ringhd->txbuf_size;
	}

	/* release resource */
	sipc_smem_release_resource(sipc->sipc_pms, sipc->dst);

	sbuf_comm_init(sbuf);

	return 0;
}

static int sbuf_client_init(struct smsg_ipc *sipc, struct sbuf_mgr *sbuf)
{
	VOLA_SBUF_SMEM *smem;
	VOLA_SBUF_RING *ringhd;
	struct sbuf_ring_header_op *hd_op;
	struct sbuf_ring *ring;
	int hsize, i, rval;
	u32 txbufsize, rxbufsize;
	phys_addr_t offset = 0;
	u32 bufnum;
	u8 dst = sbuf->dst;

#ifdef CONFIG_PHYS_ADDR_T_64BIT
	offset = sipc->high_offset;
	offset = offset << 32;
	pr_info("%s: channel %d-%d, offset = 0x%llx!\n",
		__func__, sbuf->dst, sbuf->channel, offset);
#endif

	/* get bufnum and bufsize */
	hsize = sizeof(struct sbuf_smem_header) +
		sizeof(struct sbuf_ring_header) * 1;
	sbuf->smem_virt = shmem_ram_vmap_nocache(dst,
						 sbuf->smem_addr + offset,
						 hsize);
	if (!sbuf->smem_virt) {
		pr_err("%s: channel %d-%d, Failed to map smem for sbuf head\n",
		       __func__, sbuf->dst, sbuf->channel);
		return -EFAULT;
	}
	smem = (VOLA_SBUF_SMEM *)sbuf->smem_virt;
	sbuf->ringnr = smem->ringnr;
	bufnum = sbuf->ringnr;
	ringhd = (VOLA_SBUF_RING *)&smem->headers[0];
	txbufsize = BL_GETL(ringhd->rxbuf_size);
	rxbufsize = BL_GETL(ringhd->txbuf_size);
	hsize = sizeof(struct sbuf_smem_header) +
	sizeof(struct sbuf_ring_header) * bufnum;
	sbuf->smem_size = hsize + (txbufsize + rxbufsize) * bufnum;
	pr_debug("%s: channel %d-%d, txbufsize = 0x%x, rxbufsize = 0x%x!\n",
		 __func__, sbuf->dst, sbuf->channel, txbufsize, rxbufsize);
	pr_debug("%s: channel %d-%d, smem_size = 0x%x, ringnr = %d!\n",
		 __func__, sbuf->dst, sbuf->channel, sbuf->smem_size, bufnum);
	shmem_ram_unmap(dst, sbuf->smem_virt);

	/* alloc debug smem */
	sbuf->smem_addr_debug = smem_alloc(dst, sbuf->smem_size);
	if (!sbuf->smem_addr_debug) {
		pr_err("%s: channel %d-%d,Failed to allocate debug smem for sbuf\n",
			__func__, sbuf->dst, sbuf->channel);
		return -ENOMEM;
	}

	/* get smem virtual address */
	sbuf->smem_virt = shmem_ram_vmap_nocache(dst,
						sbuf->smem_addr + offset,
						sbuf->smem_size);
	if (!sbuf->smem_virt) {
		pr_err("%s: channel %d-%d,Failed to map smem for sbuf\n",
			__func__, sbuf->dst, sbuf->channel);
		smem_free(dst, sbuf->smem_addr_debug, sbuf->smem_size);
		return -EFAULT;
	}

	/* allocate rings description */
	sbuf->rings = kcalloc(bufnum, sizeof(struct sbuf_ring), GFP_KERNEL);
	if (!sbuf->rings) {
		smem_free(dst, sbuf->smem_addr_debug, sbuf->smem_size);
		shmem_ram_unmap(dst, sbuf->smem_virt);
		return -ENOMEM;
	}
	pr_info("%s: channel %d-%d, ringns = 0x%p!\n",
		 __func__, sbuf->dst, sbuf->channel, sbuf->rings);

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(sipc->sipc_pms, sipc->dst, -1);
	if (rval < 0) {
		smem_free(dst, sbuf->smem_addr, sbuf->smem_size);
		shmem_ram_unmap(dst, sbuf->smem_virt);
		kfree(sbuf->rings);
		return rval;
	}

	/* initialize all ring bufs */
	smem = (VOLA_SBUF_SMEM *)sbuf->smem_virt;
	for (i = 0; i < bufnum; i++) {
		ringhd = (VOLA_SBUF_RING *)&smem->headers[i];
		ring = &sbuf->rings[i];
		ring->header = ringhd;
		/* host txbuf_addr */
		ring->rxbuf_virt = sbuf->smem_virt + hsize +
			(txbufsize + rxbufsize) * i;
		/* host rxbuf_addr */
		ring->txbuf_virt = ring->rxbuf_virt + rxbufsize;
		/* init header op , client mode, rx <==> tx */
		hd_op = &ring->header_op;
		hd_op->rx_rd_p = &ringhd->txbuf_rdptr;
		hd_op->rx_wt_p = &ringhd->txbuf_wrptr;
		hd_op->rx_size = ringhd->txbuf_size;
		hd_op->tx_rd_p = &ringhd->rxbuf_rdptr;
		hd_op->tx_wt_p = &ringhd->rxbuf_wrptr;
		hd_op->tx_size = ringhd->rxbuf_size;
	}

	/* release resource */
	sipc_smem_release_resource(sipc->sipc_pms, sipc->dst);

	sbuf_comm_init(sbuf);

	return 0;
}

static int sbuf_thread(void *data)
{
	struct sbuf_mgr *sbuf = data;
	struct sbuf_ring *ring;
	struct smsg mcmd, mrecv;
	int rval, bufid;
	struct smsg_ipc *sipc;
	unsigned long flags;

	/* since the channel open may hang, we call it in the sbuf thread */
	rval = smsg_ch_open(sbuf->dst, sbuf->channel, -1);
	if (rval != 0) {
		pr_err("Failed to open channel %d\n", sbuf->channel);
		/* assign NULL to thread poniter as failed to open channel */
		sbuf->thread = NULL;
		return rval;
	}

	/* if client, send SMSG_CMD_SBUF_INIT, wait sbuf SMSG_DONE_SBUF_INIT */
	sipc = smsg_ipcs[sbuf->dst];
	if (sipc->client) {
		smsg_set(&mcmd, sbuf->channel, SMSG_TYPE_CMD,
			 SMSG_CMD_SBUF_INIT, 0);
		smsg_send(sbuf->dst, &mcmd, -1);
		do {
			smsg_set(&mrecv, sbuf->channel, 0, 0, 0);
			rval = smsg_recv(sbuf->dst, &mrecv, -1);
			if (rval != 0) {
				sbuf->thread = NULL;
				return rval;
			}
		} while (mrecv.type != SMSG_TYPE_DONE ||
			mrecv.flag != SMSG_DONE_SBUF_INIT);
		sbuf->smem_addr = mrecv.value;
		pr_info("%s: channel %d-%d, done_sbuf_init, address = 0x%x!\n",
			__func__, sbuf->dst, sbuf->channel, sbuf->smem_addr);
		if (sbuf_client_init(sipc, sbuf)) {
			sbuf->thread = NULL;
			return 0;
		}
		sbuf->state = SBUF_STATE_READY;
	}

	/* sbuf init done, handle the ring rx events */
	while (!kthread_should_stop()) {
		/* monitor sbuf rdptr/wrptr update smsg */
		smsg_set(&mrecv, sbuf->channel, 0, 0, 0);
		rval = smsg_recv(sbuf->dst, &mrecv, -1);
		if (rval == -EIO) {
			/* channel state is free */
			msleep(20);
			continue;
		}

		pr_debug("sbuf thread recv msg: dst=%d, channel=%d, type=%d, flag=0x%04x, value=0x%08x\n",
			 sbuf->dst,
			 sbuf->channel,
			 mrecv.type,
			 mrecv.flag,
			 mrecv.value);

		switch (mrecv.type) {
		case SMSG_TYPE_OPEN:
			pr_info("%s: channel %d-%d, state=%d, recv open msg!\n",
				__func__, sbuf->dst,
				sbuf->channel, sbuf->state);
			if (sipc->client)
				break;

			/* if channel state is already reay, reopen it
			 * (such as modem reset), we must skip the old
			 * buf data , than give open ack and reset state
			 * to idle
			 */
			if (sbuf->state == SBUF_STATE_READY) {
				sbuf_skip_old_data(sbuf);
				sbuf->state = SBUF_STATE_IDLE;
			}
			/* handle channel open */
			smsg_open_ack(sbuf->dst, sbuf->channel);
			break;
		case SMSG_TYPE_CLOSE:
			/* handle channel close */
			sbuf_skip_old_data(sbuf);
			smsg_close_ack(sbuf->dst, sbuf->channel);
			sbuf->state = SBUF_STATE_IDLE;
			break;
		case SMSG_TYPE_CMD:
			pr_info("%s: channel %d-%d state = %d, recv cmd msg, flag = %d!\n",
				__func__, sbuf->dst, sbuf->channel,
				sbuf->state, mrecv.flag);
			if (sipc->client)
				break;

			/* respond cmd done for sbuf init only state is idle */
			if (sbuf->state == SBUF_STATE_IDLE &&
			    mrecv.flag == SMSG_CMD_SBUF_INIT) {
				smsg_set(&mcmd,
					 sbuf->channel,
					 SMSG_TYPE_DONE,
					 SMSG_DONE_SBUF_INIT,
					 sbuf->dst_smem_addr);
				smsg_send(sbuf->dst, &mcmd, -1);
				sbuf->state = SBUF_STATE_READY;
				for (bufid = 0; bufid < sbuf->ringnr; bufid++) {
					ring = &sbuf->rings[bufid];
					if (ring->handler)
						ring->handler(SBUF_NOTIFY_READY,
								ring->data);
				}
			}
			break;
		case SMSG_TYPE_EVENT:
			bufid = mrecv.value;
			WARN_ON(bufid >= sbuf->ringnr);
			ring = &sbuf->rings[bufid];
			switch (mrecv.flag) {
			case SMSG_EVENT_SBUF_RDPTR:
				if (ring->need_wake_lock)
					sprd_pms_request_wakelock_period(ring->tx_pms,
									 500);
				/* set write mask. */
				spin_lock_irqsave(&ring->poll_lock, flags);
				ring->poll_mask |= POLLOUT | POLLWRNORM;
				spin_unlock_irqrestore(&ring->poll_lock, flags);
				wake_up_interruptible_all(&ring->txwait);
				if (ring->handler)
					ring->handler(SBUF_NOTIFY_WRITE,
						      ring->data);
				break;
			case SMSG_EVENT_SBUF_WRPTR:
				/* set read mask. */
				spin_lock_irqsave(&ring->poll_lock, flags);
				ring->poll_mask |= POLLIN | POLLRDNORM;
				spin_unlock_irqrestore(&ring->poll_lock, flags);

				if (ring->need_wake_lock)
					sprd_pms_request_wakelock_period(ring->rx_pms,
									 500);
				wake_up_interruptible_all(&ring->rxwait);
				if (ring->handler)
					ring->handler(SBUF_NOTIFY_READ,
						      ring->data);
				break;
			default:
				rval = 1;
				break;
			}
			break;
		default:
			rval = 1;
			break;
		};

		if (rval) {
			pr_info("non-handled sbuf msg: %d-%d, %d, %d, %d\n",
				sbuf->dst,
				sbuf->channel,
				mrecv.type,
				mrecv.flag,
				mrecv.value);
			rval = 0;
		}
		/* unlock sipc channel wake lock */
		smsg_ch_wake_unlock(sbuf->dst, sbuf->channel);
	}

	return 0;
}


int sbuf_create(u8 dst, u8 channel, u32 bufnum, u32 txbufsize, u32 rxbufsize)
{
	struct sbuf_mgr *sbuf;
	u8 ch_index;
	int ret;
	struct smsg_ipc *sipc = NULL;
	struct sched_param param = {.sched_priority = 10};

	sipc = smsg_ipcs[dst];
	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}

	pr_debug("%s dst=%d, chanel=%d, bufnum=%d, txbufsize=0x%x, rxbufsize=0x%x\n",
		 __func__,
		 dst,
		 channel,
		 bufnum,
		 txbufsize,
		 rxbufsize);

	if (dst >= SIPC_ID_NR  || !sipc) {
		pr_err("%s: dst = %d is invalid\n", __func__, dst);
		return -EINVAL;
	}

	sbuf = kzalloc(sizeof(*sbuf), GFP_KERNEL);
	if (!sbuf)
		return -ENOMEM;

	sbuf->state = SBUF_STATE_IDLE;
	sbuf->dst = dst;
	sbuf->channel = channel;

	/* The dst smem type is pcie, need force send smsg in sbuf_write. */
	if (sipc->type == SIPC_BASE_PCIE && sipc->smem_type == SMEM_LOCAL)
		sbuf->force_send = true;

	if (!sipc->client) {
		ret = sbuf_host_init(sipc, sbuf, bufnum, txbufsize, rxbufsize);
		if (ret) {
			kfree(sbuf);
			return ret;
		}
	}

	sbuf->thread = kthread_create(sbuf_thread, sbuf,
			"sbuf-%d-%d", dst, channel);
	if (IS_ERR(sbuf->thread)) {
		pr_err("Failed to create kthread: sbuf-%d-%d\n", dst, channel);
		if (!sipc->client) {
			kfree(sbuf->rings);
			shmem_ram_unmap(dst, sbuf->smem_virt);
			smem_free(dst, sbuf->smem_addr, sbuf->smem_size);
		}
		ret = PTR_ERR(sbuf->thread);
		kfree(sbuf);
		return ret;
	}

	sbufs[dst][ch_index] = sbuf;

	/*set the thread as a real time thread, and its priority is 10*/
	sched_setscheduler(sbuf->thread, SCHED_FIFO, &param);
	wake_up_process(sbuf->thread);

	return 0;
}
EXPORT_SYMBOL_GPL(sbuf_create);

void sbuf_set_no_need_wake_lock(u8 dst, u8 channel, u32 bufnum)
{
	u8 ch_index;
	struct sbuf_mgr *sbuf;
	struct sbuf_ring *ring = NULL;

	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return;
	}

	sbuf = sbufs[dst][ch_index];
	if (!sbuf || sbuf->ringnr <= bufnum)
		return;

	ring = &sbuf->rings[bufnum];
	ring->need_wake_lock = false;
}
EXPORT_SYMBOL_GPL(sbuf_set_no_need_wake_lock);


void sbuf_down(u8 dst, u8 channel)
{
	struct sbuf_mgr *sbuf;
	u8 ch_index;
    int i;
	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return;
	}

	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return;
	sbuf->state = SBUF_STATE_IDLE;
	if (sbuf->rings) {
		for (i = 0; i < sbuf->ringnr; i++) {
			wake_up_interruptible_all(&sbuf->rings[i].txwait);
			wake_up_interruptible_all(&sbuf->rings[i].rxwait);
		}
	}
    pr_info("%s: channel=%d sbuf down success\n", __func__, channel);
}

void sbuf_destroy(u8 dst, u8 channel)
{
	struct sbuf_mgr *sbuf;
	int i;
	u8 ch_index;
	struct smsg_ipc *sipc;

	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return;
	}

	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return;

	sbuf->state = SBUF_STATE_IDLE;
	smsg_ch_close(dst, channel, -1);

	/* stop sbuf thread if it's created successfully and still alive */
	if (!IS_ERR_OR_NULL(sbuf->thread))
		kthread_stop(sbuf->thread);

	if (sbuf->rings) {
		for (i = 0; i < sbuf->ringnr; i++) {
			wake_up_interruptible_all(&sbuf->rings[i].txwait);
			wake_up_interruptible_all(&sbuf->rings[i].rxwait);
#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
			sbuf_destroy_rdwt_owner(&sbuf->rings[i]);
#endif
			sprd_pms_destroy(sbuf->rings[i].tx_pms);
			sprd_pms_destroy(sbuf->rings[i].rx_pms);
		}
		kfree(sbuf->rings);
	}

	if (sbuf->smem_virt)
		shmem_ram_unmap(dst, sbuf->smem_virt);

	sipc = smsg_ipcs[dst];
	if (sipc->client)
		smem_free(dst, sbuf->smem_addr_debug, sbuf->smem_size);
	else
		smem_free(dst, sbuf->smem_addr, sbuf->smem_size);

	kfree(sbuf);

	sbufs[dst][ch_index] = NULL;
}
EXPORT_SYMBOL_GPL(sbuf_destroy);

int sbuf_write(u8 dst, u8 channel, u32 bufid,
	       void *buf, u32 len, int timeout)
{
	struct sbuf_mgr *sbuf;
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op;
	struct smsg mevt;
	void *txpos;
	int rval, left, tail, txsize;
	u8 ch_index;
	union sbuf_buf u_buf;
	bool no_data;
	unsigned long flags;
	u32 v;

	u_buf.buf = buf;
	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}

	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return -ENODEV;
	ring = &sbuf->rings[bufid];
	hd_op = &ring->header_op;
	if (sbuf->state != SBUF_STATE_READY) {
		pr_info("sbuf-%d-%d not ready to write!\n",
			dst, channel);
		return -ENODEV;
	}

	pr_debug("%s: dst=%d, channel=%d, bufid=%d, len=%d, timeout=%d\n",
		 __func__,
		 dst,
		 channel,
		 bufid,
		 len,
		 timeout);

	rval = 0;
	left = len;

	if (timeout) {
	    mutex_lock_interruptible(&ring->txlock);
	} else {
		if (!mutex_trylock(&ring->txlock)) {
			pr_debug("sbuf_read busy, dst=%d, channel=%d, bufid=%d\n",
				 dst, channel, bufid);
			return -EBUSY;
		}
	}

#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
	sbuf_record_rdwt_owner(ring, 0);
#endif

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(ring->tx_pms, sbuf->dst, -1);
	if (rval < 0) {
		mutex_unlock(&ring->txlock);
		return rval;
	}

	pr_debug("%s: channel=%d, wrptr=%d, rdptr=%d\n",
		 __func__,
		 channel,
		 BL_READL(hd_op->tx_wt_p),
		 BL_READL(hd_op->tx_rd_p));
	no_data = ((int)(BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p)) >=
					hd_op->tx_size);

	/* update write mask */
	spin_lock_irqsave(&ring->poll_lock, flags);
	if (no_data)
		ring->poll_mask &= ~(POLLOUT | POLLWRNORM);
	else
		ring->poll_mask |= POLLOUT | POLLWRNORM;
	spin_unlock_irqrestore(&ring->poll_lock, flags);

	/* release resource */
	sipc_smem_release_resource(ring->tx_pms, sbuf->dst);

	if (no_data) {
		if (timeout == 0) {
			pr_info("%s: %d-%d ring %d txbuf is full!\n",
				__func__, dst, channel, bufid);
			rval = -EBUSY;
		} else if (timeout < 0) {
			/* wait forever */
			rval = wait_event_interruptible(
				ring->txwait,
				sbuf_has_data(ring, dst, true) ||
				sbuf->state == SBUF_STATE_IDLE);
			if (rval < 0)
				pr_debug("%s: wait interrupted!\n", __func__);

			if (sbuf->state == SBUF_STATE_IDLE) {
				pr_err("%s: sbuf state is idle!\n", __func__);
				rval = -EIO;
			}
		} else {
			/* wait timeout */
			rval = wait_event_interruptible_timeout(
				ring->txwait,
				sbuf_has_data(ring, dst, true) ||
				sbuf->state == SBUF_STATE_IDLE,
				timeout);
			if (rval < 0) {
				pr_debug("%s: wait interrupted!\n", __func__);
			} else if (rval == 0) {
				pr_info("%s: wait timeout!\n", __func__);
				rval = -ETIME;
			}

			if (sbuf->state == SBUF_STATE_IDLE) {
				pr_err("%s: sbuf state is idle!\n", __func__);
				rval = -EIO;
			}
		}
	}

	if (rval < 0) {
		mutex_unlock(&ring->txlock);
		return rval;
	}

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(ring->tx_pms, sbuf->dst, -1);
	if (rval < 0) {
		mutex_unlock(&ring->txlock);
		return rval;
	}

	while (left && (int)(BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p)) <
	       hd_op->tx_size && sbuf->state == SBUF_STATE_READY) {
		/* calc txpos & txsize */
		txpos = ring->txbuf_virt +
			BL_READL(hd_op->tx_wt_p) % hd_op->tx_size;
		txsize = hd_op->tx_size -
			(int)(BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p));
		txsize = min(txsize, left);

		tail = txpos + txsize - (ring->txbuf_virt + hd_op->tx_size);
		if (tail > 0) {
			/* ring buffer is rounded */
			if ((uintptr_t)u_buf.buf > TASK_SIZE) {
				unalign_memcpy(txpos, u_buf.buf, txsize - tail);
				unalign_memcpy(ring->txbuf_virt,
					       u_buf.buf + txsize - tail, tail);
			} else {
				if (unalign_copy_from_user(
					txpos,
					u_buf.ubuf,
					txsize - tail) ||
				   unalign_copy_from_user(
					ring->txbuf_virt,
					u_buf.ubuf + txsize - tail,
					tail)) {
					pr_err("%s:failed to copy from user!\n",
					       __func__);
					rval = -EFAULT;
					break;
				}
			}
		} else {
			if ((uintptr_t)u_buf.buf > TASK_SIZE) {
				unalign_memcpy(txpos, u_buf.buf, txsize);
			} else {
				/* handle the user space address */
				if (unalign_copy_from_user(
						txpos,
						u_buf.ubuf,
						txsize)) {
					pr_err("%s:failed to copy from user!\n",
					       __func__);
					rval = -EFAULT;
					break;
				}
			}
		}

		pr_debug("%s: channel=%d, txpos=%p, txsize=%d\n",
			 __func__, channel, txpos, txsize);

		/* update tx wrptr */
		v = BL_READL(hd_op->tx_wt_p) + txsize;
		BL_WRITEL(v, hd_op->tx_wt_p);
		/*
		 * force send be true or tx ringbuf is empty,
		 * need to notify peer side
		 */
		if (sbuf->force_send ||
		    BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p) == txsize) {
			smsg_set(&mevt, channel,
				 SMSG_TYPE_EVENT,
				 SMSG_EVENT_SBUF_WRPTR,
				 bufid);
			smsg_send(dst, &mevt, -1);
		}

		left -= txsize;
		u_buf.buf += txsize;
	}

	/* update write mask */
	spin_lock_irqsave(&ring->poll_lock, flags);
	if ((int)(BL_READL(hd_op->tx_wt_p) - BL_READL(hd_op->tx_rd_p)) >=
	    hd_op->tx_size)
		ring->poll_mask &= ~(POLLOUT | POLLWRNORM);
	else
		ring->poll_mask |= POLLOUT | POLLWRNORM;
	spin_unlock_irqrestore(&ring->poll_lock, flags);

	/* release resource */
	sipc_smem_release_resource(ring->tx_pms, sbuf->dst);
	if (ring->need_wake_lock)
		sprd_pms_release_wakelock_later(ring->tx_pms, 20);

	mutex_unlock(&ring->txlock);

	pr_debug("%s: done, channel=%d, len=%d\n",
		 __func__, channel, len - left);

	if (len == left)
		return rval;
	else
		return (len - left);
}
EXPORT_SYMBOL_GPL(sbuf_write);

int sbuf_read(u8 dst, u8 channel, u32 bufid,
	      void *buf, u32 len, int timeout)
{
	struct sbuf_mgr *sbuf;
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op;
	struct smsg mevt;
	void *rxpos;
	int rval, left, tail, rxsize;
	u8 ch_index;
	union sbuf_buf u_buf;
	bool no_data;
	unsigned long flags;
	u32 v;

	u_buf.buf = buf;
	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}
	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return -ENODEV;
	ring = &sbuf->rings[bufid];
	hd_op = &ring->header_op;

	if (sbuf->state != SBUF_STATE_READY) {
		pr_debug("sbuf-%d-%d not ready to read!\n", dst, channel);
		return -ENODEV;
	}

	pr_debug("%s:dst=%d, channel=%d, bufid=%d, len=%d, timeout=%d\n",
		 __func__, dst, channel, bufid, len, timeout);

	rval = 0;
	left = len;

	if (timeout) {
	    mutex_lock_interruptible(&ring->rxlock);
	} else {
		if (!mutex_trylock(&ring->rxlock)) {
			pr_debug("%s: busy!,dst=%d, channel=%d, bufid=%d\n",
				 __func__, dst, channel, bufid);
			return -EBUSY;
		}
	}

#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
	sbuf_record_rdwt_owner(ring, 1);
#endif

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(ring->rx_pms, sbuf->dst, -1);
	if (rval < 0) {
		mutex_unlock(&ring->rxlock);
		return rval;
	}

	pr_debug("%s: channel=%d, wrptr=%d, rdptr=%d\n",
		 __func__,
		 channel,
		 BL_READL(hd_op->rx_wt_p),
		 BL_READL(hd_op->rx_rd_p));
	no_data = (BL_READL(hd_op->rx_wt_p) == BL_READL(hd_op->rx_rd_p));
	/* update read mask */
	spin_lock_irqsave(&ring->poll_lock, flags);
	if (no_data)
		ring->poll_mask &= ~(POLLIN | POLLRDNORM);
	else
		ring->poll_mask |= POLLIN | POLLRDNORM;
	spin_unlock_irqrestore(&ring->poll_lock, flags);

	/* release resource */
	sipc_smem_release_resource(ring->rx_pms, sbuf->dst);

	if (no_data) {
		if (timeout == 0) {
			/* no wait */
			pr_debug("%s: %d-%d ring %d rxbuf is empty!\n",
				 __func__, dst, channel, bufid);
			rval = -ENODATA;
		} else if (timeout < 0) {
			/* wait forever */
			rval = wait_event_interruptible(
				ring->rxwait,
				sbuf_has_data(ring, dst, false) ||
				sbuf->state == SBUF_STATE_IDLE);
			if (rval < 0)
				pr_debug("%s: wait interrupted!\n", __func__);

			if (sbuf->state == SBUF_STATE_IDLE) {
				pr_err("%s: sbuf state is idle!\n", __func__);
				rval = -EIO;
			}
		} else {
			/* wait timeout */
			rval = wait_event_interruptible_timeout(
				ring->rxwait,
				sbuf_has_data(ring, dst, false) ||
				sbuf->state == SBUF_STATE_IDLE, timeout);
			if (rval < 0) {
				pr_debug("%s: wait interrupted!\n", __func__);
			} else if (rval == 0) {
				pr_info("%s: wait timeout!\n", __func__);
				rval = -ETIME;
			}

			if (sbuf->state == SBUF_STATE_IDLE) {
				pr_err("%s: state is idle!\n", __func__);
				rval = -EIO;
			}
		}
	}

	if (rval < 0) {
		mutex_unlock(&ring->rxlock);
		return rval;
	}

	/* must request resource before read or write share memory */
	rval = sipc_smem_request_resource(ring->rx_pms, sbuf->dst, -1);
	if (rval < 0) {
		mutex_unlock(&ring->rxlock);
		return rval;
	}

	while (left &&
	       (BL_READL(hd_op->rx_wt_p) != BL_READL(hd_op->rx_rd_p)) &&
	       sbuf->state == SBUF_STATE_READY) {
		/* calc rxpos & rxsize */
		rxpos = ring->rxbuf_virt +
			BL_READL(hd_op->rx_rd_p) % hd_op->rx_size;
		rxsize = (int)(BL_READL(hd_op->rx_wt_p) - BL_READL(hd_op->rx_rd_p));
		/* check overrun */
		if (rxsize > hd_op->rx_size)
			pr_err("%s: bufid = %d, channel= %d rxsize=0x%x, rdptr=%d, wrptr=%d",
			       __func__,
			       bufid,
			       channel,
			       rxsize,
			       BL_READL(hd_op->rx_wt_p),
			       BL_READL(hd_op->rx_rd_p));

		rxsize = min(rxsize, left);

		pr_debug("%s: channel=%d, buf=%p, rxpos=%p, rxsize=%d\n",
			 __func__, channel, u_buf.buf, rxpos, rxsize);

		tail = rxpos + rxsize - (ring->rxbuf_virt + hd_op->rx_size);

		if (tail > 0) {
			/* ring buffer is rounded */
			if ((uintptr_t)u_buf.buf > TASK_SIZE) {
				unalign_memcpy(u_buf.buf, rxpos, rxsize - tail);
				unalign_memcpy(u_buf.buf + rxsize - tail,
					       ring->rxbuf_virt, tail);
			} else {
				/* handle the user space address */
				if (unalign_copy_to_user(u_buf.ubuf,
							 rxpos,
							 rxsize - tail) ||
				    unalign_copy_to_user(u_buf.ubuf
							 + rxsize - tail,
							 ring->rxbuf_virt,
							 tail)) {
					pr_err("%s: failed to copy to user!\n",
						__func__);
					rval = -EFAULT;
					break;
				}
			}
		} else {
			if ((uintptr_t)u_buf.buf > TASK_SIZE) {
				unalign_memcpy(u_buf.buf, rxpos, rxsize);
			} else {
				/* handle the user space address */
				if (unalign_copy_to_user(u_buf.ubuf,
							 rxpos, rxsize)) {
					pr_err("%s: failed to copy to user!\n",
						__func__);
					rval = -EFAULT;
					break;
				}
			}
		}

		/* update rx rdptr */
		v = BL_READL(hd_op->rx_rd_p) + rxsize;
		BL_WRITEL(v, hd_op->rx_rd_p);
		/* rx ringbuf is full ,so need to notify peer side */
		if (BL_READL(hd_op->rx_wt_p) - BL_READL(hd_op->rx_rd_p) ==
		    hd_op->rx_size - rxsize) {
			smsg_set(&mevt, channel,
				 SMSG_TYPE_EVENT,
				 SMSG_EVENT_SBUF_RDPTR,
				 bufid);
			smsg_send(dst, &mevt, -1);
		}

		left -= rxsize;
		u_buf.buf += rxsize;
	}

	/* update read mask */
	spin_lock_irqsave(&ring->poll_lock, flags);
	if (BL_READL(hd_op->rx_wt_p) == BL_READL(hd_op->rx_rd_p))
		ring->poll_mask &= ~(POLLIN | POLLRDNORM);
	else
		ring->poll_mask |= POLLIN | POLLRDNORM;
	spin_unlock_irqrestore(&ring->poll_lock, flags);

	/* release resource */
	sipc_smem_release_resource(ring->rx_pms, sbuf->dst);
	if (ring->need_wake_lock)
		sprd_pms_release_wakelock_later(ring->rx_pms, 20);

	mutex_unlock(&ring->rxlock);

	pr_debug("%s: done, channel=%d, len=%d", __func__, channel, len - left);

	if (len == left)
		return rval;
	else
		return (len - left);
}
EXPORT_SYMBOL_GPL(sbuf_read);

int sbuf_poll_wait(u8 dst, u8 channel, u32 bufid,
		   struct file *filp, poll_table *wait)
{
	struct sbuf_mgr *sbuf;
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op;
	unsigned int mask = 0;
	u8 ch_index;

	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return mask;
	}
	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return mask;
	ring = &sbuf->rings[bufid];
	hd_op = &ring->header_op;
	if (sbuf->state != SBUF_STATE_READY) {
		pr_err("sbuf-%d-%d not ready to poll !\n", dst, channel);
		return mask;
	}

	poll_wait(filp, &ring->txwait, wait);
	poll_wait(filp, &ring->rxwait, wait);


	if (sbuf_has_data(ring, dst, true))
		mask |= POLLOUT | POLLWRNORM;

	if (sbuf_has_data(ring, dst, false))
		mask |= POLLIN | POLLRDNORM;

	return mask;
}
EXPORT_SYMBOL_GPL(sbuf_poll_wait);

int sbuf_status(u8 dst, u8 channel)
{
	struct sbuf_mgr *sbuf;
	u8 ch_index;

	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}
	sbuf = sbufs[dst][ch_index];

	if (!sbuf)
		return -ENODEV;
	if (sbuf->state != SBUF_STATE_READY)
		return -ENODEV;

	return 0;
}
EXPORT_SYMBOL_GPL(sbuf_status);

int sbuf_register_notifier(u8 dst, u8 channel, u32 bufid,
			   void (*handler)(int event, void *data), void *data)
{
	struct sbuf_mgr *sbuf;
	struct sbuf_ring *ring = NULL;
	u8 ch_index;

	ch_index = sipc_channel2index(channel);
	if (ch_index == INVALID_CHANEL_INDEX) {
		pr_err("%s:channel %d invalid!\n", __func__, channel);
		return -EINVAL;
	}
	sbuf = sbufs[dst][ch_index];
	if (!sbuf)
		return -ENODEV;
	ring = &sbuf->rings[bufid];
	ring->handler = handler;
	ring->data = data;

	if (sbuf->state == SBUF_STATE_READY)
		handler(SBUF_NOTIFY_READ, data);

	return 0;
}
EXPORT_SYMBOL_GPL(sbuf_register_notifier);

void sbuf_get_status(u8 dst, char *status_info, int size)
{
	struct sbuf_mgr *sbuf = NULL;
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op;
	
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
	wait_queue_t *pos;
#else
	wait_queue_entry_t *pos;
#endif
	struct task_struct *task;
	unsigned long flags;
	int i, n, len, cnt;
	u32 b_select;
	char *phead;
#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
	struct name_node *node = NULL;
#endif

	len = strlen(status_info);
	if (!status_info || size < 0 || dst >= SIPC_ID_NR)
		return;

	for (i = 0;  i < SMSG_VALID_CH_NR; i++) {
		sbuf = sbufs[dst][i];
		if (!sbuf)
			continue;

		ring = &sbuf->rings[0];
		/* must request resource before read or write share memory */
		if (sipc_smem_request_resource(ring->rx_pms, dst, 1000) < 0)
			continue;

		for (n = 0;  n < sbuf->ringnr && len < size; n++) {
			ring = &sbuf->rings[n];
			hd_op = &ring->header_op;

			if ((BL_READL(hd_op->rx_wt_p) - BL_READL(hd_op->rx_rd_p))
					< hd_op->rx_size)
				continue;

			snprintf(status_info + len,
				 size - len,
				 "ch-%d-ring-%d is full.\n",
				 sbuf->channel,
				 n);
			len = strlen(status_info);

			/*  show all rxwait task */
			spin_lock_irqsave(&ring->rxwait.lock, flags);
			cnt = 0;

			list_for_each_entry(pos,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
					    &ring->rxwait.task_list,
					    task_list
#else
					    &ring->rxwait.head,
					    entry
#endif				
                                                 ) {
				task = sbuf_wait_get_task(pos, &b_select);
				if (!task)
					continue;

				if (b_select)
					phead = "rxwait task";
				else
					phead = "select task";

				snprintf(
					 status_info + len,
					 size - len,
					 "%s %d: %s, state=0x%lx, pid=%d.\n",
					 phead,
					 cnt, task->comm,
					 task->state, task->pid);
				cnt++;
				len = strlen(status_info);
			}
			spin_unlock_irqrestore(&ring->rxwait.lock, flags);

			/* only show the latest ever read task */
#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
			spin_lock_irqsave(&ring->rxwait.lock, flags);
			list_for_each_entry(node, &ring->rx_list, list) {
				if (node->latest) {
					snprintf(
						 status_info + len,
						 size - len,
						 "read task: %s, pid = %d.\n",
						 node->comm,
						 node->pid);
					break;
				}
			}
			spin_unlock_irqrestore(&ring->rxwait.lock, flags);
#endif
		}
		ring = &sbuf->rings[0];
		/* release resource */
		sipc_smem_release_resource(ring->rx_pms, sbuf->dst);
	}
}
EXPORT_SYMBOL_GPL(sbuf_get_status);

#if defined(CONFIG_DEBUG_FS)
static void sbuf_debug_task_show(struct seq_file *m,
				 struct sbuf_mgr *sbuf, int task_type)
{
	int n, cnt;
	u32 b_select;
	unsigned long flags;
	struct sbuf_ring *ring = NULL;
	wait_queue_head_t *phead;
	char *buf;
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
	wait_queue_t *pos;
#else
	wait_queue_entry_t *pos;
#endif
	struct task_struct *task;

	for (n = 0;  n < sbuf->ringnr;	n++) {
		ring = &sbuf->rings[n];
		cnt = 0;

		if (task_type == TASK_RXWAIT) {
			phead = &ring->rxwait;
			buf = "rxwait task";
		} else if (task_type == TASK_TXWAIT) {
			phead = &ring->txwait;
			buf = "txwait task";
		} else {
			phead = &ring->rxwait;
			buf = "select task";
		}

		spin_lock_irqsave(&phead->lock, flags);

		list_for_each_entry(pos,
#if (LINUX_VERSION_CODE <= KERNEL_VERSION( 4,10,0 ))
                  	&phead->task_list, task_list
#else
					&phead->head, entry
#endif
                                                     ){
		task = sbuf_wait_get_task(pos, &b_select);
			if (!task)
				continue;

			if (b_select && (task_type != TASK_SELECT))
				continue;

			seq_printf(m, "  ring[%2d]: %s %d ",
				   n,
				   buf,
				   cnt);
			seq_printf(m, ": %s, state = 0x%lx, pid = %d\n",
				   task->comm,
				   task->state,
				   task->pid);
			cnt++;
		}
		spin_unlock_irqrestore(
			&phead->lock,
			flags);
	}
}

#if defined(SIPC_DEBUG_SBUF_RDWT_OWNER)
static void sbuf_debug_list_show(struct seq_file *m,
				 struct sbuf_mgr *sbuf, int b_rx)
{
	int n, cnt;
	struct sbuf_ring *ring = NULL;
	struct list_head *plist;
	char *buf;
	struct name_node *node = NULL;
	unsigned long flags;

	/* list all  sbuf task list */
	for (n = 0;  n < sbuf->ringnr;	n++) {
		ring = &sbuf->rings[n];
		cnt = 0;

		if (b_rx) {
			plist = &ring->rx_list;
			buf = "read task";
		} else {
			plist = &ring->tx_list;
			buf = "write task";
		}

		spin_lock_irqsave(&ring->rxwait.lock, flags);
		list_for_each_entry(node, plist, list) {
			seq_printf(m, "  ring[%2d]: %s %d : %s, pid = %d, latest = %d\n",
				   n,
				   buf,
				   cnt,
				   node->comm,
				   node->pid,
				   node->latest);
			cnt++;
		}
		spin_unlock_irqrestore(&ring->rxwait.lock, flags);
	}
}
#endif

static int sbuf_debug_show(struct seq_file *m, void *private)
{
	struct sbuf_mgr *sbuf = NULL;
	struct sbuf_ring *ring = NULL;
	struct sbuf_ring_header_op *hd_op;
	int i, j, n, cnt;
	struct smsg_ipc *sipc = NULL;

	for (i = 0; i < SIPC_ID_NR; i++) {
		sipc = smsg_ipcs[i];
		if (!sipc)
			continue;

		/* must request resource before read or write share memory */
		if (sipc_smem_request_resource(sipc->sipc_pms,
					       sipc->dst, 1000) < 0)
			continue;

		sipc_debug_putline(m, '*', 120);
		seq_printf(m, "dst: 0x%0x, sipc: %s:\n", i, sipc->name);
		sipc_debug_putline(m, '*', 120);

		for (j = 0;  j < SMSG_VALID_CH_NR; j++) {
			sbuf = sbufs[i][j];
			if (!sbuf)
				continue;
			/* list a sbuf channel */
			sipc_debug_putline(m, '-', 100);
			seq_printf(m, "sbuf_%d_%03d, state: %d, force: %d",
				   sbuf->dst,
				   sbuf->channel,
				   sbuf->state,
				   sbuf->force_send);
			seq_printf(m, "virt: 0x%lx, phy: 0x%0x, map: 0x%x",
				   (unsigned long)sbuf->smem_virt,
				   sbuf->smem_addr,
				   sbuf->dst_smem_addr);
			seq_printf(m, " size: 0x%0x, ringnr: %d\n",
				   sbuf->smem_size,
				   sbuf->ringnr);
			sipc_debug_putline(m, '-', 100);

			/* list all  sbuf ring info list in a chanel */
			sipc_debug_putline(m, '-', 80);
			seq_puts(m, "  1. all sbuf ring info list:\n");
			for (n = 0;  n < sbuf->ringnr;  n++) {
				ring = &sbuf->rings[n];
				hd_op = &ring->header_op;
				if (BL_READL(hd_op->tx_wt_p) == 0 &&
				    BL_READL(hd_op->rx_wt_p) == 0)
					continue;

				seq_printf(m, "  rx ring[%2d]: addr: 0x%0x, mask: 0x%x",
					   n, ring->header->rxbuf_addr,
					   ring->poll_mask);
				seq_printf(m, "rp: 0x%0x, wp: 0x%0x, size: 0x%0x\n",
					   BL_READL(hd_op->rx_rd_p),
					   BL_READL(hd_op->rx_wt_p),
					   hd_op->rx_size);

				seq_printf(m, "  tx ring[%2d]: addr: 0x%0x, ",
					   n, ring->header->txbuf_addr);
				seq_printf(m, "rp: 0x%0x, wp: 0x%0x, size: 0x%0x\n",
					   BL_READL(hd_op->tx_rd_p),
					   BL_READL(hd_op->tx_wt_p),
					   hd_op->tx_size);
			}

			/* list all sbuf rxwait/txwait in a chanel */;
			sipc_debug_putline(m, '-', 80);
			seq_puts(m, "  2. all waittask list:\n");
			sbuf_debug_task_show(m, sbuf, TASK_RXWAIT);
			sbuf_debug_task_show(m, sbuf, TASK_TXWAIT);
			sbuf_debug_task_show(m, sbuf, TASK_SELECT);

#ifdef SIPC_DEBUG_SBUF_RDWT_OWNER
			/* list all sbuf ever read task list in a chanel */;
			sipc_debug_putline(m, '-', 80);
			seq_puts(m, "  3. all ever rdwt list:\n");
			sbuf_debug_list_show(m, sbuf, 1);
			sbuf_debug_list_show(m, sbuf, 0);
#endif

			/* list all  rx full ring list in a chanel */
			cnt = 0;
			for (n = 0;  n < sbuf->ringnr;	n++) {
				ring = &sbuf->rings[n];
				hd_op = &ring->header_op;
				if ((BL_READL(hd_op->rx_wt_p) - BL_READL(hd_op->rx_rd_p))
						== hd_op->rx_size) {
					if (cnt == 0) {
						sipc_debug_putline(m, '-', 80);
						seq_puts(m, "  x. all rx full ring list:\n");
					}
					cnt++;
					seq_printf(m, "  ring[%2d]\n", n);
				}
			}
		}
		/* release resource */
		sipc_smem_release_resource(sipc->sipc_pms, sipc->dst);
	}

	return 0;
}

static int sbuf_debug_open(struct inode *inode, struct file *file)
{
	return single_open(file, sbuf_debug_show, inode->i_private);
}

static const struct file_operations sbuf_debug_fops = {
	.open = sbuf_debug_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int sbuf_init_debugfs(void *root)
{
	if (!root)
		return -ENXIO;
	debugfs_create_file("sbuf", 0444,
			    (struct dentry *)root,
			    NULL, &sbuf_debug_fops);
	return 0;
}
EXPORT_SYMBOL_GPL(sbuf_init_debugfs);

#endif /* CONFIG_DEBUG_FS */

MODULE_AUTHOR("Chen Gaopeng");
MODULE_DESCRIPTION("SIPC/SBUF driver");
MODULE_LICENSE("GPL v2");
