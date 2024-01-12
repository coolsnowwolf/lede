// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved. */

#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/wait.h>
#include <asm/uaccess.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE > KERNEL_VERSION( 4,11,0 ))
#include <linux/sched/signal.h>
#else
#include <linux/signal.h>
#endif
#include "mhi.h"
#include "mhi_internal.h"

struct mhi_controller_map {
	u32 dev_id;
	u32 domain;
	u32 bus;
	u32 slot;
};

#define MAX_MHI_CONTROLLER 16
struct mhi_controller_map mhi_controller_minors[MAX_MHI_CONTROLLER];

#define MHI_CNTRL_DRIVER_NAME "mhi_cntrl_q"
struct mhi_cntrl_drv {
	struct list_head head;
	struct mutex lock;
	struct class *class;
	int major;
};
static struct mhi_cntrl_drv mhi_cntrl_drv;

const char * const mhi_ee_str[MHI_EE_MAX] = {
	[MHI_EE_PBL] = "PBL",
	[MHI_EE_SBL] = "SBL",
	[MHI_EE_AMSS] = "AMSS",
	[MHI_EE_RDDM] = "RDDM",
	[MHI_EE_WFW] = "WFW",
	[MHI_EE_PTHRU] = "PASS THRU",
	[MHI_EE_EDL] = "EDL",
	[MHI_EE_FP] = "FlashProg",
	[MHI_EE_DISABLE_TRANSITION] = "DISABLE",
};

const char * const mhi_state_tran_str[MHI_ST_TRANSITION_MAX] = {
	[MHI_ST_TRANSITION_PBL] = "PBL",
	[MHI_ST_TRANSITION_READY] = "READY",
	[MHI_ST_TRANSITION_SBL] = "SBL",
	[MHI_ST_TRANSITION_MISSION_MODE] = "MISSION MODE",
	[MHI_ST_TRANSITION_FP] = "FlashProg",
};

const char * const mhi_state_str[MHI_STATE_MAX] = {
	[MHI_STATE_RESET] = "RESET",
	[MHI_STATE_READY] = "READY",
	[MHI_STATE_M0] = "M0",
	[MHI_STATE_M1] = "M1",
	[MHI_STATE_M2] = "M2",
	[MHI_STATE_M3] = "M3",
	[MHI_STATE_BHI] = "BHI",
	[MHI_STATE_SYS_ERR] = "SYS_ERR",
};

static const char * const mhi_pm_state_str[] = {
	[MHI_PM_BIT_DISABLE] = "DISABLE",
	[MHI_PM_BIT_POR] = "POR",
	[MHI_PM_BIT_M0] = "M0",
	[MHI_PM_BIT_M2] = "M2",
	[MHI_PM_BIT_M3_ENTER] = "M?->M3",
	[MHI_PM_BIT_M3] = "M3",
	[MHI_PM_BIT_M3_EXIT] = "M3->M0",
	[MHI_PM_BIT_FW_DL_ERR] = "FW DL Error",
	[MHI_PM_BIT_SYS_ERR_DETECT] = "SYS_ERR Detect",
	[MHI_PM_BIT_SYS_ERR_PROCESS] = "SYS_ERR Process",
	[MHI_PM_BIT_SHUTDOWN_PROCESS] = "SHUTDOWN Process",
	[MHI_PM_BIT_LD_ERR_FATAL_DETECT] = "LD or Error Fatal Detect",
};

struct mhi_bus mhi_bus;

const char *to_mhi_pm_state_str(enum MHI_PM_STATE state)
{
	int index = find_last_bit((unsigned long *)&state, 32);

	if (index >= ARRAY_SIZE(mhi_pm_state_str))
		return "Invalid State";

	return mhi_pm_state_str[index];
}

#if 0
/* MHI protocol require transfer ring to be aligned to ring length */
static int mhi_alloc_aligned_ring(struct mhi_controller *mhi_cntrl,
				  struct mhi_ring *ring,
				  u64 len)
{
	ring->alloc_size = len + (len - 1);
	ring->pre_aligned = mhi_alloc_coherent(mhi_cntrl, ring->alloc_size,
					       &ring->dma_handle, GFP_KERNEL);
	if (!ring->pre_aligned)
		return -ENOMEM;

	ring->iommu_base = (ring->dma_handle + (len - 1)) & ~(len - 1);
	ring->base = ring->pre_aligned + (ring->iommu_base - ring->dma_handle);
	return 0;
}
#endif

static void mhi_ring_aligned_check(struct mhi_controller *mhi_cntrl, u64 rbase, u64 rlen) {
	uint64_t ra;

	ra = rbase;
	do_div(ra, roundup_pow_of_two(rlen));

	if (rbase != ra * roundup_pow_of_two(rlen)) {
		MHI_ERR("bad params ring base not aligned 0x%llx align 0x%lx\n", rbase, roundup_pow_of_two(rlen));
	}
}

void mhi_deinit_free_irq(struct mhi_controller *mhi_cntrl)
{
	int i;
	struct mhi_event *mhi_event = mhi_cntrl->mhi_event;


	if (mhi_cntrl->msi_allocated == 1) {
		free_irq(mhi_cntrl->irq[mhi_cntrl->msi_irq_base], mhi_cntrl);
		return;
	}

	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++) {
		if (mhi_event->offload_ev)
			continue;

		free_irq(mhi_cntrl->irq[mhi_event->msi], mhi_event);
	}

	free_irq(mhi_cntrl->irq[mhi_cntrl->msi_irq_base], mhi_cntrl);
}

int mhi_init_irq_setup(struct mhi_controller *mhi_cntrl)
{
	int i;
	int ret;
	struct mhi_event *mhi_event = mhi_cntrl->mhi_event;

	if (mhi_cntrl->msi_allocated == 1) {
		for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++) {
			mhi_event->msi = 0;
		}

		ret = request_threaded_irq(mhi_cntrl->irq[0], NULL,
								mhi_one_msi_handlr, IRQF_ONESHOT, "mhi", mhi_cntrl);
		if (ret) {
			MHI_ERR("Error requesting irq:%d, ret=%d\n", mhi_cntrl->irq[0], ret);
		}
		return ret;
	}

	/* for BHI INTVEC msi */
	ret = request_threaded_irq(mhi_cntrl->irq[mhi_cntrl->msi_irq_base], mhi_intvec_handlr,
				   mhi_intvec_threaded_handlr, IRQF_ONESHOT,
				   "mhi", mhi_cntrl);
	if (ret)
		return ret;

	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++) {
		if (mhi_event->offload_ev)
			continue;

		ret = request_irq(mhi_cntrl->irq[mhi_event->msi],
				  mhi_msi_handlr, IRQF_SHARED, "mhi",
				  mhi_event);
		if (ret) {
			MHI_ERR("Error requesting irq:%d for ev:%d\n",
				mhi_cntrl->irq[mhi_event->msi], i);
			goto error_request;
		}
	}

	return 0;

error_request:
	for (--i, --mhi_event; i >= 0; i--, mhi_event--) {
		if (mhi_event->offload_ev)
			continue;

		free_irq(mhi_cntrl->irq[mhi_event->msi], mhi_event);
	}
	free_irq(mhi_cntrl->irq[0], mhi_cntrl);

	return ret;
}

void mhi_deinit_dev_ctxt(struct mhi_controller *mhi_cntrl)
{
	int i;
	struct mhi_ctxt *mhi_ctxt = mhi_cntrl->mhi_ctxt;
	struct mhi_cmd *mhi_cmd;
	struct mhi_event *mhi_event;
	struct mhi_ring *ring;

	mhi_cmd = mhi_cntrl->mhi_cmd;
	for (i = 0; i < NR_OF_CMD_RINGS; i++, mhi_cmd++) {
		ring = &mhi_cmd->ring;
#if 0
		mhi_free_coherent(mhi_cntrl, ring->alloc_size,
				  ring->pre_aligned, ring->dma_handle);
#endif
		ring->base = NULL;
		ring->iommu_base = 0;
	}

#if 0
	mhi_free_coherent(mhi_cntrl,
			  sizeof(*mhi_ctxt->cmd_ctxt) * NR_OF_CMD_RINGS,
			  mhi_ctxt->cmd_ctxt, mhi_ctxt->cmd_ctxt_addr);
#endif

	mhi_event = mhi_cntrl->mhi_event;
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++) {
		if (mhi_event->offload_ev)
			continue;

		ring = &mhi_event->ring;
#if 0
		mhi_free_coherent(mhi_cntrl, ring->alloc_size,
				  ring->pre_aligned, ring->dma_handle);
#endif
		ring->base = NULL;
		ring->iommu_base = 0;
	}

#if 0
	mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->er_ctxt) *
			  mhi_cntrl->total_ev_rings, mhi_ctxt->er_ctxt,
			  mhi_ctxt->er_ctxt_addr);

	mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->chan_ctxt) *
			  mhi_cntrl->max_chan, mhi_ctxt->chan_ctxt,
			  mhi_ctxt->chan_ctxt_addr);
#endif

	mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->ctrl_seg), mhi_ctxt->ctrl_seg, mhi_ctxt->ctrl_seg_addr);
	kfree(mhi_ctxt);
	mhi_cntrl->mhi_ctxt = NULL;
}

static int mhi_init_debugfs_mhi_states_open(struct inode *inode,
					    struct file *fp)
{
	return single_open(fp, mhi_debugfs_mhi_states_show, inode->i_private);
}

static int mhi_init_debugfs_mhi_event_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, mhi_debugfs_mhi_event_show, inode->i_private);
}

static int mhi_init_debugfs_mhi_chan_open(struct inode *inode, struct file *fp)
{
	return single_open(fp, mhi_debugfs_mhi_chan_show, inode->i_private);
}

static const struct file_operations debugfs_state_ops = {
	.open = mhi_init_debugfs_mhi_states_open,
	.release = single_release,
	.read = seq_read,
};

static const struct file_operations debugfs_ev_ops = {
	.open = mhi_init_debugfs_mhi_event_open,
	.release = single_release,
	.read = seq_read,
};

static const struct file_operations debugfs_chan_ops = {
	.open = mhi_init_debugfs_mhi_chan_open,
	.release = single_release,
	.read = seq_read,
};

DEFINE_SIMPLE_ATTRIBUTE(debugfs_trigger_reset_fops, NULL,
			 mhi_debugfs_trigger_reset, "%llu\n");

#ifdef ENABLE_MHI_MON
struct mon_event_text {
	struct list_head e_link;
	int type;		/* submit, complete, etc. */
	unsigned int tstamp;
	u32 chan;
	dma_addr_t wp;
	struct mhi_tre mhi_tre;
	u8 data[32];
	size_t len;
};

#define EVENT_MAX  (16*PAGE_SIZE / sizeof(struct mon_event_text))
#define PRINTF_DFL  250
#define SLAB_NAME_SZ  30

struct mon_reader_text {
	struct kmem_cache *e_slab;
	int nevents;
	struct list_head e_list;
	struct mon_reader r;	/* In C, parent class can be placed anywhere */

	wait_queue_head_t wait;
	int printf_size;
	char *printf_buf;
	int left_size;
	int left_pos;
	struct mutex printf_lock;

	char slab_name[SLAB_NAME_SZ];
};

struct mon_text_ptr {
	int cnt, limit;
	char *pbuf;
};

static DEFINE_MUTEX(mon_lock);

static inline unsigned int mon_get_timestamp(void)
{
	struct timespec64 now;
	unsigned int stamp;

	ktime_get_ts64(&now);
	stamp = now.tv_sec & 0xFFF;  /* 2^32 = 4294967296. Limit to 4096s. */
	stamp = stamp * USEC_PER_SEC + now.tv_nsec / NSEC_PER_USEC;
	return stamp;
}

static void mon_text_event(struct mon_reader_text *rp,
    u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len,
    char ev_type)
{
	struct mon_event_text *ep;	

	if (rp->nevents >= EVENT_MAX ||
	    (ep = kmem_cache_alloc(rp->e_slab, GFP_ATOMIC)) == NULL) {
		rp->r.m_bus->cnt_text_lost++;
		return;
	}

	ep->type = ev_type;
	ep->tstamp = mon_get_timestamp();
	ep->chan = chan;
	ep->wp = wp;
	ep->mhi_tre = *mhi_tre;
	if (len > sizeof(ep->data))
		len = sizeof(ep->data);
	memcpy(ep->data, buf, len);
	ep->len = len;
	rp->nevents++;
	list_add_tail(&ep->e_link, &rp->e_list);
	wake_up(&rp->wait);
}

static void mon_text_submit(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len)
{
	struct mon_reader_text *rp = data;
	mon_text_event(rp, chan, wp, mhi_tre, buf, len, 'W');
}

static void mon_text_receive(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len)
{
	struct mon_reader_text *rp = data;
	mon_text_event(rp, chan, wp, mhi_tre, buf, len, 'R');
}

static void mon_text_complete(void *data, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre)
{
	struct mon_reader_text *rp = data;
	mon_text_event(rp, chan, wp, mhi_tre, NULL, 0, 'E');
}

void mon_reader_add(struct mhi_controller *mbus, struct mon_reader *r)
{
	unsigned long flags;

	spin_lock_irqsave(&mbus->lock, flags);
	mbus->nreaders++;
	list_add_tail(&r->r_link, &mbus->r_list);
	spin_unlock_irqrestore(&mbus->lock, flags);

	kref_get(&mbus->ref);
}

static void mon_bus_drop(struct kref *r)
{
	struct mhi_controller *mbus = container_of(r, struct mhi_controller, ref);
	kfree(mbus);
}

static void mon_reader_del(struct mhi_controller *mbus, struct mon_reader *r)
{
	unsigned long flags;

	spin_lock_irqsave(&mbus->lock, flags);
	list_del(&r->r_link);
	--mbus->nreaders;
	spin_unlock_irqrestore(&mbus->lock, flags);

	kref_put(&mbus->ref, mon_bus_drop);
}

static void mon_text_ctor(void *mem)
{
	/*
	 * Nothing to initialize. No, really!
	 * So, we fill it with garbage to emulate a reused object.
	 */
	memset(mem, 0xe5, sizeof(struct mon_event_text));
}

static int mon_text_open(struct inode *inode, struct file *file)
{
	struct mhi_controller *mbus;
	struct mon_reader_text *rp;
	int rc;

	mutex_lock(&mon_lock);
	mbus = inode->i_private;

	rp = kzalloc(sizeof(struct mon_reader_text), GFP_KERNEL);
	if (rp == NULL) {
		rc = -ENOMEM;
		goto err_alloc;
	}
	INIT_LIST_HEAD(&rp->e_list);
	init_waitqueue_head(&rp->wait);
	mutex_init(&rp->printf_lock);

	rp->printf_size = PRINTF_DFL;
	rp->printf_buf = kmalloc(rp->printf_size, GFP_KERNEL);
	if (rp->printf_buf == NULL) {
		rc = -ENOMEM;
		goto err_alloc_pr;
	}

	rp->r.m_bus = mbus;
	rp->r.r_data = rp;
	rp->r.rnf_submit = mon_text_submit;
	rp->r.rnf_receive = mon_text_receive;
	rp->r.rnf_complete = mon_text_complete;

	snprintf(rp->slab_name, SLAB_NAME_SZ, "mon_text_%p", rp);
	rp->e_slab = kmem_cache_create(rp->slab_name,
	    sizeof(struct mon_event_text), sizeof(long), 0,
	    mon_text_ctor);
	if (rp->e_slab == NULL) {
		rc = -ENOMEM;
		goto err_slab;
	}

	mon_reader_add(mbus, &rp->r);

	file->private_data = rp;
	mutex_unlock(&mon_lock);
	return 0;

// err_busy:
//	kmem_cache_destroy(rp->e_slab);
err_slab:
	kfree(rp->printf_buf);
err_alloc_pr:
	kfree(rp);
err_alloc:
	mutex_unlock(&mon_lock);
	return rc;
}

static struct mon_event_text *mon_text_fetch(struct mon_reader_text *rp,
    struct mhi_controller *mbus)
{
	struct list_head *p;
	unsigned long flags;

	spin_lock_irqsave(&mbus->lock, flags);
	if (list_empty(&rp->e_list)) {
		spin_unlock_irqrestore(&mbus->lock, flags);
		return NULL;
	}
	p = rp->e_list.next;
	list_del(p);
	--rp->nevents;
	spin_unlock_irqrestore(&mbus->lock, flags);
	return list_entry(p, struct mon_event_text, e_link);
}

static struct mon_event_text *mon_text_read_wait(struct mon_reader_text *rp,
    struct file *file)
{
	struct mhi_controller *mbus = rp->r.m_bus;
	DECLARE_WAITQUEUE(waita, current);
	struct mon_event_text *ep;

	add_wait_queue(&rp->wait, &waita);
	set_current_state(TASK_INTERRUPTIBLE);
	while ((ep = mon_text_fetch(rp, mbus)) == NULL) {
		if (file->f_flags & O_NONBLOCK) {
			set_current_state(TASK_RUNNING);
			remove_wait_queue(&rp->wait, &waita);
			return ERR_PTR(-EWOULDBLOCK);
		}
		/*
		 * We do not count nwaiters, because ->release is supposed
		 * to be called when all openers are gone only.
		 */
		schedule();
		if (signal_pending(current)) {
			remove_wait_queue(&rp->wait, &waita);
			return ERR_PTR(-EINTR);
		}
		set_current_state(TASK_INTERRUPTIBLE);
	}
	set_current_state(TASK_RUNNING);
	remove_wait_queue(&rp->wait, &waita);
	return ep;
}

static ssize_t mon_text_read_u(struct file *file, char __user *buf,
				size_t nbytes, loff_t *ppos)
{
	struct mon_reader_text *rp = file->private_data;
	struct mon_event_text *ep;
	struct mon_text_ptr ptr;

	if (rp->left_size) {
		int cnt = rp->left_size;

		if (cnt > nbytes)
			cnt = nbytes;
		if (copy_to_user(buf, rp->printf_buf + rp->left_pos, cnt))
			return -EFAULT;
		rp->left_pos += cnt;
		rp->left_size -= cnt;
		return cnt;
	}

	if (IS_ERR(ep = mon_text_read_wait(rp, file)))
		return PTR_ERR(ep);
	mutex_lock(&rp->printf_lock);
	ptr.cnt = 0;
	ptr.pbuf = rp->printf_buf;
	ptr.limit = rp->printf_size;

	ptr.cnt += snprintf(ptr.pbuf + ptr.cnt, ptr.limit - ptr.cnt,
	    "%u %c %03d WP:%llx TRE: %llx %08x %08x",
	    ep->tstamp, ep->type, ep->chan, ep->wp,
	    ep->mhi_tre.ptr, ep->mhi_tre.dword[0], ep->mhi_tre.dword[1]);

	if (ep->len) {
		struct mon_text_ptr *p = &ptr;
		size_t i = 0;

		for (i = 0; i < ep->len; i++) {
			if (i % 4 == 0) {
				p->cnt += snprintf(p->pbuf + p->cnt,
				    p->limit - p->cnt,
				    " ");
			}
			p->cnt += snprintf(p->pbuf + p->cnt,
			    p->limit - p->cnt,
			    "%02x", ep->data[i]);
		}

	}

	ptr.cnt += snprintf(ptr.pbuf +ptr.cnt, ptr.limit - ptr.cnt, "\n");

	if (ptr.cnt > nbytes) {
		rp->left_pos = nbytes;
		rp->left_size = ptr.cnt - nbytes;
		ptr.cnt = nbytes;
	}

	if (copy_to_user(buf, rp->printf_buf, ptr.cnt))
		ptr.cnt = -EFAULT;
	mutex_unlock(&rp->printf_lock);
	kmem_cache_free(rp->e_slab, ep);
	return ptr.cnt;
}

static int mon_text_release(struct inode *inode, struct file *file)
{
	struct mon_reader_text *rp = file->private_data;
	struct mhi_controller *mbus;
	/* unsigned long flags; */
	struct list_head *p;
	struct mon_event_text *ep;

	mutex_lock(&mon_lock);
	mbus = inode->i_private;

	if (mbus->nreaders <= 0) {
		mutex_unlock(&mon_lock);
		return 0;
	}
	mon_reader_del(mbus, &rp->r);

	/*
	 * In theory, e_list is protected by mbus->lock. However,
	 * after mon_reader_del has finished, the following is the case:
	 *  - we are not on reader list anymore, so new events won't be added;
	 *  - whole mbus may be dropped if it was orphaned.
	 * So, we better not touch mbus.
	 */
	/* spin_lock_irqsave(&mbus->lock, flags); */
	while (!list_empty(&rp->e_list)) {
		p = rp->e_list.next;
		ep = list_entry(p, struct mon_event_text, e_link);
		list_del(p);
		--rp->nevents;
		kmem_cache_free(rp->e_slab, ep);
	}
	/* spin_unlock_irqrestore(&mbus->lock, flags); */

	kmem_cache_destroy(rp->e_slab);
	kfree(rp->printf_buf);
	kfree(rp);

	mutex_unlock(&mon_lock);
	return 0;
}


static const struct file_operations mon_fops_text_u = {
	.owner =	THIS_MODULE,
	.open =		mon_text_open,
	.llseek =	no_llseek,
	.read =		mon_text_read_u,
	.release =	mon_text_release,
};
#endif

void mhi_init_debugfs(struct mhi_controller *mhi_cntrl)
{
	struct dentry *dentry;
	char node[32];

#ifdef ENABLE_MHI_MON
	struct mhi_controller *mbus = mhi_cntrl;

	mbus->nreaders = 0;
	kref_init(&mbus->ref);
	spin_lock_init(&mbus->lock);
	INIT_LIST_HEAD(&mbus->r_list);
#endif

	if (!mhi_cntrl->parent)
	snprintf(node, sizeof(node), "mhi_%04x_%02u:%02u.%02u",
		 mhi_cntrl->dev_id, mhi_cntrl->domain, mhi_cntrl->bus,
		 mhi_cntrl->slot);
	else
	snprintf(node, sizeof(node), "%04x_%02u:%02u.%02u",
		 mhi_cntrl->dev_id, mhi_cntrl->domain, mhi_cntrl->bus,
		 mhi_cntrl->slot);

	dentry = debugfs_create_dir(node, mhi_cntrl->parent);
	if (IS_ERR_OR_NULL(dentry))
		return;

	debugfs_create_file("states", 0444, dentry, mhi_cntrl,
				   &debugfs_state_ops);
	debugfs_create_file("events", 0444, dentry, mhi_cntrl,
				   &debugfs_ev_ops);
	debugfs_create_file("chan", 0444, dentry, mhi_cntrl,
				   &debugfs_chan_ops);
	debugfs_create_file("reset", 0444, dentry, mhi_cntrl,
				   &debugfs_trigger_reset_fops);
#ifdef ENABLE_MHI_MON
	debugfs_create_file("mhimon", 0444, dentry, mhi_cntrl,
				   &mon_fops_text_u);
#endif
	mhi_cntrl->dentry = dentry;
}

void mhi_deinit_debugfs(struct mhi_controller *mhi_cntrl)
{
	debugfs_remove_recursive(mhi_cntrl->dentry);
	mhi_cntrl->dentry = NULL;
}

int mhi_init_dev_ctxt(struct mhi_controller *mhi_cntrl)
{
	struct mhi_ctxt *mhi_ctxt;
	struct mhi_chan_ctxt *chan_ctxt;
	struct mhi_event_ctxt *er_ctxt;
	struct mhi_cmd_ctxt *cmd_ctxt;
	struct mhi_chan *mhi_chan;
	struct mhi_event *mhi_event;
	struct mhi_cmd *mhi_cmd;
	int ret = -ENOMEM, i;

	atomic_set(&mhi_cntrl->dev_wake, 0);
	atomic_set(&mhi_cntrl->alloc_size, 0);
	atomic_set(&mhi_cntrl->pending_pkts, 0);

	mhi_ctxt = kzalloc(sizeof(*mhi_ctxt), GFP_KERNEL);
	if (!mhi_ctxt)
		return -ENOMEM;

	mhi_ctxt->ctrl_seg = mhi_alloc_coherent(mhi_cntrl, sizeof(*mhi_ctxt->ctrl_seg),
			&mhi_ctxt->ctrl_seg_addr, GFP_KERNEL);
	MHI_LOG("mhi_ctxt->ctrl_seg = %p\n", mhi_ctxt->ctrl_seg);
	if (!mhi_ctxt->ctrl_seg)
		goto error_alloc_chan_ctxt;

	if ((unsigned long)mhi_ctxt->ctrl_seg & (4096-1)) {
		mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->ctrl_seg), mhi_ctxt->ctrl_seg, mhi_ctxt->ctrl_seg_addr);
		goto error_alloc_chan_ctxt;
	}

	/* setup channel ctxt */
#if 1
	mhi_ctxt->chan_ctxt = mhi_ctxt->ctrl_seg->chan_ctxt;
	mhi_ctxt->chan_ctxt_addr = mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, chan_ctxt);
#else
	mhi_ctxt->chan_ctxt = mhi_alloc_coherent(mhi_cntrl,
			sizeof(*mhi_ctxt->chan_ctxt) * mhi_cntrl->max_chan,
			&mhi_ctxt->chan_ctxt_addr, GFP_KERNEL);
	if (!mhi_ctxt->chan_ctxt)
		goto error_alloc_chan_ctxt;
#endif

	mhi_chan = mhi_cntrl->mhi_chan;
	chan_ctxt = mhi_ctxt->chan_ctxt;
	for (i = 0; i < mhi_cntrl->max_chan; i++, chan_ctxt++, mhi_chan++) {
		/* If it's offload channel skip this step */
		if (mhi_chan->offload_ch)
			continue;

		chan_ctxt->chstate = MHI_CH_STATE_DISABLED;
		chan_ctxt->brstmode = mhi_chan->db_cfg.brstmode;
		chan_ctxt->pollcfg = mhi_chan->db_cfg.pollcfg;
		chan_ctxt->chtype = mhi_chan->type;
		chan_ctxt->erindex = mhi_chan->er_index;

		mhi_chan->ch_state = MHI_CH_STATE_DISABLED;
		mhi_chan->tre_ring.db_addr = &chan_ctxt->wp;
	}

	/* setup event context */
#if 1
	mhi_ctxt->er_ctxt = mhi_ctxt->ctrl_seg->er_ctxt;
	mhi_ctxt->er_ctxt_addr = mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, er_ctxt);
#else
	mhi_ctxt->er_ctxt = mhi_alloc_coherent(mhi_cntrl,
			sizeof(*mhi_ctxt->er_ctxt) * mhi_cntrl->total_ev_rings,
			&mhi_ctxt->er_ctxt_addr, GFP_KERNEL);
	if (!mhi_ctxt->er_ctxt)
		goto error_alloc_er_ctxt;
#endif

	er_ctxt = mhi_ctxt->er_ctxt;
	mhi_event = mhi_cntrl->mhi_event;
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, er_ctxt++,
		     mhi_event++) {
		struct mhi_ring *ring = &mhi_event->ring;

		/* it's a satellite ev, we do not touch it */
		if (mhi_event->offload_ev)
			continue;

		er_ctxt->intmodc = 0;
		er_ctxt->intmodt = mhi_event->intmod;
		er_ctxt->ertype = MHI_ER_TYPE_VALID;
		if (mhi_cntrl->msi_allocated == 1) {
			mhi_event->msi = 0;
		}
		er_ctxt->msivec = mhi_event->msi;
		mhi_event->db_cfg.db_mode = true;

		ring->el_size = sizeof(struct mhi_tre);
		ring->len = ring->el_size * ring->elements;
#if 1
		ring->alloc_size = ring->len;
		ring->pre_aligned = mhi_ctxt->ctrl_seg->event_ring[i];
		ring->dma_handle = mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, event_ring[i]);
		ring->iommu_base = ring->dma_handle;
		ring->base = ring->pre_aligned + (ring->iommu_base - ring->dma_handle);
#else
		ret = mhi_alloc_aligned_ring(mhi_cntrl, ring, ring->len);
		if (ret)
			goto error_alloc_er;
#endif

		ring->rp = ring->wp = ring->base;
		er_ctxt->rbase = ring->iommu_base;
		er_ctxt->rp = er_ctxt->wp = er_ctxt->rbase;
		er_ctxt->rlen = ring->len;
		ring->ctxt_wp = &er_ctxt->wp;

		mhi_ring_aligned_check(mhi_cntrl, er_ctxt->rbase, er_ctxt->rlen);
		memset(ring->base, 0xCC, ring->len);
	}

	/* setup cmd context */
#if 1
	mhi_ctxt->cmd_ctxt = mhi_ctxt->ctrl_seg->cmd_ctxt;
	mhi_ctxt->cmd_ctxt_addr = mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, cmd_ctxt);
#else
	mhi_ctxt->cmd_ctxt = mhi_alloc_coherent(mhi_cntrl,
				sizeof(*mhi_ctxt->cmd_ctxt) * NR_OF_CMD_RINGS,
				&mhi_ctxt->cmd_ctxt_addr, GFP_KERNEL);
	if (!mhi_ctxt->cmd_ctxt)
		goto error_alloc_er;
#endif

	mhi_cmd = mhi_cntrl->mhi_cmd;
	cmd_ctxt = mhi_ctxt->cmd_ctxt;
	for (i = 0; i < NR_OF_CMD_RINGS; i++, mhi_cmd++, cmd_ctxt++) {
		struct mhi_ring *ring = &mhi_cmd->ring;

		ring->el_size = sizeof(struct mhi_tre);
		ring->elements = CMD_EL_PER_RING;
		ring->len = ring->el_size * ring->elements;
#if 1
		ring->alloc_size = ring->len;
		ring->pre_aligned = mhi_ctxt->ctrl_seg->cmd_ring[i];
		ring->dma_handle = mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, cmd_ring[i]);
		ring->iommu_base = ring->dma_handle;
		ring->base = ring->pre_aligned + (ring->iommu_base - ring->dma_handle);
#else
		ret = mhi_alloc_aligned_ring(mhi_cntrl, ring, ring->len);
		if (ret)
			goto error_alloc_cmd;
#endif

		ring->rp = ring->wp = ring->base;
		cmd_ctxt->rbase = ring->iommu_base;
		cmd_ctxt->rp = cmd_ctxt->wp = cmd_ctxt->rbase;
		cmd_ctxt->rlen = ring->len;
		ring->ctxt_wp = &cmd_ctxt->wp;

		mhi_ring_aligned_check(mhi_cntrl, cmd_ctxt->rbase, cmd_ctxt->rlen);
	}

	mhi_cntrl->mhi_ctxt = mhi_ctxt;

	return 0;

#if 0
error_alloc_cmd:
	for (--i, --mhi_cmd; i >= 0; i--, mhi_cmd--) {
		struct mhi_ring *ring = &mhi_cmd->ring;

		mhi_free_coherent(mhi_cntrl, ring->alloc_size,
				  ring->pre_aligned, ring->dma_handle);
	}
	mhi_free_coherent(mhi_cntrl,
			  sizeof(*mhi_ctxt->cmd_ctxt) * NR_OF_CMD_RINGS,
			  mhi_ctxt->cmd_ctxt, mhi_ctxt->cmd_ctxt_addr);
	i = mhi_cntrl->total_ev_rings;
	mhi_event = mhi_cntrl->mhi_event + i;

error_alloc_er:
	for (--i, --mhi_event; i >= 0; i--, mhi_event--) {
		struct mhi_ring *ring = &mhi_event->ring;

		if (mhi_event->offload_ev)
			continue;

		mhi_free_coherent(mhi_cntrl, ring->alloc_size,
				  ring->pre_aligned, ring->dma_handle);
	}
	mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->er_ctxt) *
			  mhi_cntrl->total_ev_rings, mhi_ctxt->er_ctxt,
			  mhi_ctxt->er_ctxt_addr);

error_alloc_er_ctxt:
	mhi_free_coherent(mhi_cntrl, sizeof(*mhi_ctxt->chan_ctxt) *
			  mhi_cntrl->max_chan, mhi_ctxt->chan_ctxt,
			  mhi_ctxt->chan_ctxt_addr);
#endif

error_alloc_chan_ctxt:
	kfree(mhi_ctxt);

	return ret;
}

static int mhi_get_tsync_er_cfg(struct mhi_controller *mhi_cntrl)
{
	int i;
	struct mhi_event *mhi_event = mhi_cntrl->mhi_event;

	/* find event ring with timesync support */
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++)
		if (mhi_event->data_type == MHI_ER_TSYNC_ELEMENT_TYPE)
			return mhi_event->er_index;

	return -ENOENT;
}

int mhi_init_timesync(struct mhi_controller *mhi_cntrl)
{
	struct mhi_timesync *mhi_tsync;
	u32 time_offset, db_offset;
	int ret;

	read_lock_bh(&mhi_cntrl->pm_lock);

	if (!MHI_REG_ACCESS_VALID(mhi_cntrl->pm_state)) {
		ret = -EIO;
		goto exit_timesync;
	}

	ret = mhi_get_capability_offset(mhi_cntrl, TIMESYNC_CAP_ID,
					&time_offset);
	if (ret) {
		MHI_LOG("No timesync capability found\n");
		goto exit_timesync;
	}

	read_unlock_bh(&mhi_cntrl->pm_lock);

	if (!mhi_cntrl->time_get || !mhi_cntrl->lpm_disable ||
	     !mhi_cntrl->lpm_enable)
		return -EINVAL;

	/* register method supported */
	mhi_tsync = kzalloc(sizeof(*mhi_tsync), GFP_KERNEL);
	if (!mhi_tsync)
		return -ENOMEM;

	spin_lock_init(&mhi_tsync->lock);
	INIT_LIST_HEAD(&mhi_tsync->head);
	init_completion(&mhi_tsync->completion);

	/* save time_offset for obtaining time */
	MHI_LOG("TIME OFFS:0x%x\n", time_offset);
	mhi_tsync->time_reg = mhi_cntrl->regs + time_offset
			      + TIMESYNC_TIME_LOW_OFFSET;

	mhi_cntrl->mhi_tsync = mhi_tsync;

	ret = mhi_create_timesync_sysfs(mhi_cntrl);
	if (unlikely(ret)) {
		/* kernel method still work */
		MHI_ERR("Failed to create timesync sysfs nodes\n");
	}

	read_lock_bh(&mhi_cntrl->pm_lock);

	if (!MHI_REG_ACCESS_VALID(mhi_cntrl->pm_state)) {
		ret = -EIO;
		goto exit_timesync;
	}

	/* get DB offset if supported, else return */
	ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs,
			   time_offset + TIMESYNC_DB_OFFSET, &db_offset);
	if (ret || !db_offset) {
		ret = 0;
		goto exit_timesync;
	}

	MHI_LOG("TIMESYNC_DB OFFS:0x%x\n", db_offset);
	mhi_tsync->db = mhi_cntrl->regs + db_offset;

	read_unlock_bh(&mhi_cntrl->pm_lock);

	/* get time-sync event ring configuration */
	ret = mhi_get_tsync_er_cfg(mhi_cntrl);
	if (ret < 0) {
		MHI_LOG("Could not find timesync event ring\n");
		return ret;
	}

	mhi_tsync->er_index = ret;

	ret = mhi_send_cmd(mhi_cntrl, NULL, MHI_CMD_TIMSYNC_CFG);
	if (ret) {
		MHI_ERR("Failed to send time sync cfg cmd\n");
		return ret;
	}

	ret = wait_for_completion_timeout(&mhi_tsync->completion,
			msecs_to_jiffies(mhi_cntrl->timeout_ms));

	if (!ret || mhi_tsync->ccs != MHI_EV_CC_SUCCESS) {
		MHI_ERR("Failed to get time cfg cmd completion\n");
		return -EIO;
	}

	return 0;

exit_timesync:
	read_unlock_bh(&mhi_cntrl->pm_lock);

	return ret;
}

int mhi_init_mmio(struct mhi_controller *mhi_cntrl)
{
	u32 val;
	int i, ret;
	struct mhi_chan *mhi_chan;
	struct mhi_event *mhi_event;
	void __iomem *base = mhi_cntrl->regs;
	struct {
		u32 offset;
		u32 mask;
		u32 shift;
		u32 val;
	} reg_info[] = {
		{
			CCABAP_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->mhi_ctxt->chan_ctxt_addr),
		},
		{
			CCABAP_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->mhi_ctxt->chan_ctxt_addr),
		},
		{
			ECABAP_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->mhi_ctxt->er_ctxt_addr),
		},
		{
			ECABAP_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->mhi_ctxt->er_ctxt_addr),
		},
		{
			CRCBAP_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->mhi_ctxt->cmd_ctxt_addr),
		},
		{
			CRCBAP_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->mhi_ctxt->cmd_ctxt_addr),
		},
#if 0 //carl.yin 20190527 UDE-WIN-InitMmio
		{
			MHICFG, MHICFG_NER_MASK, MHICFG_NER_SHIFT,
			mhi_cntrl->total_ev_rings,
		},
		{
			MHICFG, MHICFG_NHWER_MASK, MHICFG_NHWER_SHIFT,
			mhi_cntrl->hw_ev_rings,
		},
#endif
		{
			MHICTRLBASE_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->mhi_ctxt->ctrl_seg_addr),
		},
		{
			MHICTRLBASE_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->mhi_ctxt->ctrl_seg_addr),
		},
		{
			MHIDATABASE_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->iova_start),
		},
		{
			MHIDATABASE_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->iova_start),
		},
		{
			MHICTRLLIMIT_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->mhi_ctxt->ctrl_seg_addr+sizeof(struct mhi_ctrl_seg)),
		},
		{
			MHICTRLLIMIT_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->mhi_ctxt->ctrl_seg_addr+sizeof(struct mhi_ctrl_seg)),
		},
		{
			MHIDATALIMIT_HIGHER, U32_MAX, 0,
			upper_32_bits(mhi_cntrl->iova_stop),
		},
		{
			MHIDATALIMIT_LOWER, U32_MAX, 0,
			lower_32_bits(mhi_cntrl->iova_stop),
		},
		{ 0, 0, 0 }
	};

	MHI_LOG("Initializing MMIO\n");

	/* set up DB register for all the chan rings */
	ret = mhi_read_reg_field(mhi_cntrl, base, CHDBOFF, CHDBOFF_CHDBOFF_MASK,
				 CHDBOFF_CHDBOFF_SHIFT, &val);
	if (ret)
		return -EIO;

	MHI_LOG("CHDBOFF:0x%x\n", val);

	/* setup wake db */
	mhi_cntrl->wake_db = base + val + (8 * MHI_DEV_WAKE_DB);
#if 0 //'EM120RGLAPR02A07M4G_11' will treate as chan 127's interrput, and report complete event over cmd ring, but cmd ring is not set by now
	mhi_write_reg(mhi_cntrl, mhi_cntrl->wake_db, 4, 0);
	mhi_write_reg(mhi_cntrl, mhi_cntrl->wake_db, 0, 0);
	mhi_cntrl->wake_set = false;
#endif

	/* setup channel db addresses */
	mhi_chan = mhi_cntrl->mhi_chan;
	for (i = 0; i < mhi_cntrl->max_chan; i++, val += 8, mhi_chan++)
		mhi_chan->tre_ring.db_addr = base + val;

	/* setup event ring db addresses */
	ret = mhi_read_reg_field(mhi_cntrl, base, ERDBOFF, ERDBOFF_ERDBOFF_MASK,
				 ERDBOFF_ERDBOFF_SHIFT, &val);
	if (ret)
		return -EIO;

	MHI_LOG("ERDBOFF:0x%x\n", val);

	mhi_event = mhi_cntrl->mhi_event;
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, val += 8, mhi_event++) {
		if (mhi_event->offload_ev)
			continue;

		mhi_event->ring.db_addr = base + val;
	}

	/* set up DB register for primary CMD rings */
	mhi_cntrl->mhi_cmd[PRIMARY_CMD_RING].ring.db_addr = base + CRDB_LOWER;

	MHI_LOG("Programming all MMIO values.\n");
	for (i = 0; reg_info[i].offset; i++)
		mhi_write_reg_field(mhi_cntrl, base, reg_info[i].offset,
				    reg_info[i].mask, reg_info[i].shift,
				    reg_info[i].val);

	return 0;
}

void mhi_deinit_chan_ctxt(struct mhi_controller *mhi_cntrl,
			  struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring;
	struct mhi_ring *tre_ring;
	struct mhi_chan_ctxt *chan_ctxt;

	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;
	chan_ctxt = &mhi_cntrl->mhi_ctxt->chan_ctxt[mhi_chan->chan];

#if 0
	mhi_free_coherent(mhi_cntrl, tre_ring->alloc_size,
			  tre_ring->pre_aligned, tre_ring->dma_handle);
#endif
	kfree(buf_ring->base);

	buf_ring->base = tre_ring->base = NULL;
	chan_ctxt->rbase = 0;
}

int mhi_init_chan_ctxt(struct mhi_controller *mhi_cntrl,
		       struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring;
	struct mhi_ring *tre_ring;
	struct mhi_chan_ctxt *chan_ctxt;
	int ret;

	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;
	tre_ring->el_size = sizeof(struct mhi_tre);
	tre_ring->len = tre_ring->el_size * tre_ring->elements;
	chan_ctxt = &mhi_cntrl->mhi_ctxt->chan_ctxt[mhi_chan->chan];
#if 1
	tre_ring->alloc_size = tre_ring->len;
	if (MHI_CLIENT_IP_HW_0_IN == mhi_chan->chan) {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->hw_in_chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, hw_in_chan_ring[mhi_chan->ring]);
	}
	else if (MHI_CLIENT_IP_HW_0_OUT == mhi_chan->chan) {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->hw_out_chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, hw_out_chan_ring[mhi_chan->ring]);
	}
#ifdef ENABLE_IP_SW0
	else if (MHI_CLIENT_IP_SW_0_IN == mhi_chan->chan) {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->sw_in_chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, sw_in_chan_ring[mhi_chan->ring]);
	}
	else if (MHI_CLIENT_IP_SW_0_OUT == mhi_chan->chan) {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->sw_out_chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, sw_out_chan_ring[mhi_chan->ring]);
	}
#endif
	else if (MHI_CLIENT_DIAG_IN == mhi_chan->chan) {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->diag_in_chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, diag_in_chan_ring[mhi_chan->ring]);
	}
	else {
		tre_ring->pre_aligned = &mhi_cntrl->mhi_ctxt->ctrl_seg->chan_ring[mhi_chan->ring];
		tre_ring->dma_handle = mhi_cntrl->mhi_ctxt->ctrl_seg_addr + offsetof(struct mhi_ctrl_seg, chan_ring[mhi_chan->ring]);
	}
	tre_ring->iommu_base = tre_ring->dma_handle;
	tre_ring->base = tre_ring->pre_aligned + (tre_ring->iommu_base - tre_ring->dma_handle);
	ret = 0;
#else
	ret = mhi_alloc_aligned_ring(mhi_cntrl, tre_ring, tre_ring->len);
#endif
	if (ret)
		return -ENOMEM;

	buf_ring->el_size = sizeof(struct mhi_buf_info);
	buf_ring->len = buf_ring->el_size * buf_ring->elements;
	buf_ring->base = kzalloc(buf_ring->len, GFP_KERNEL);

	if (!buf_ring->base) {
#if 0
		mhi_free_coherent(mhi_cntrl, tre_ring->alloc_size,
				  tre_ring->pre_aligned, tre_ring->dma_handle);
#endif
		return -ENOMEM;
	}

	chan_ctxt->chstate = MHI_CH_STATE_ENABLED;
	chan_ctxt->rbase = tre_ring->iommu_base;
	chan_ctxt->rp = chan_ctxt->wp = chan_ctxt->rbase;
	chan_ctxt->rlen = tre_ring->len;
	tre_ring->ctxt_wp = &chan_ctxt->wp;

	tre_ring->rp = tre_ring->wp = tre_ring->base;
	buf_ring->rp = buf_ring->wp = buf_ring->base;
	mhi_chan->db_cfg.db_mode = true;

	mhi_ring_aligned_check(mhi_cntrl, chan_ctxt->rbase, chan_ctxt->rlen);
	/* update to all cores */
	smp_wmb();

	return 0;
}

int mhi_device_configure(struct mhi_device *mhi_dev,
			 enum dma_data_direction dir,
			 struct mhi_buf *cfg_tbl,
			 int elements)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan;
	struct mhi_event_ctxt *er_ctxt;
	struct mhi_chan_ctxt *ch_ctxt;
	int er_index, chan;

	switch (dir) {
	case DMA_TO_DEVICE:
		mhi_chan = mhi_dev->ul_chan;
		break;
	case DMA_BIDIRECTIONAL:
	case DMA_FROM_DEVICE:
	case DMA_NONE:
		mhi_chan = mhi_dev->dl_chan;
		break;
	default:
		return -EINVAL;
	}

	er_index = mhi_chan->er_index;
	chan = mhi_chan->chan;

	for (; elements > 0; elements--, cfg_tbl++) {
		/* update event context array */
		if (!strcmp(cfg_tbl->name, "ECA")) {
			er_ctxt = &mhi_cntrl->mhi_ctxt->er_ctxt[er_index];
			if (sizeof(*er_ctxt) != cfg_tbl->len) {
				MHI_ERR(
					"Invalid ECA size, expected:%zu actual%zu\n",
					sizeof(*er_ctxt), cfg_tbl->len);
				return -EINVAL;
			}
			memcpy((void *)er_ctxt, cfg_tbl->buf, sizeof(*er_ctxt));
			continue;
		}

		/* update channel context array */
		if (!strcmp(cfg_tbl->name, "CCA")) {
			ch_ctxt = &mhi_cntrl->mhi_ctxt->chan_ctxt[chan];
			if (cfg_tbl->len != sizeof(*ch_ctxt)) {
				MHI_ERR(
					"Invalid CCA size, expected:%zu actual:%zu\n",
					sizeof(*ch_ctxt), cfg_tbl->len);
				return -EINVAL;
			}
			memcpy((void *)ch_ctxt, cfg_tbl->buf, sizeof(*ch_ctxt));
			continue;
		}

		return -EINVAL;
	}

	return 0;
}

#if 0
static int of_parse_ev_cfg(struct mhi_controller *mhi_cntrl,
			   struct device_node *of_node)
{
	int i, ret, num = 0;
	struct mhi_event *mhi_event;
	struct device_node *child;

	of_node = of_find_node_by_name(of_node, "mhi_events");
	if (!of_node)
		return -EINVAL;

	for_each_available_child_of_node(of_node, child) {
		if (!strcmp(child->name, "mhi_event"))
			num++;
	}

	if (!num)
		return -EINVAL;

	mhi_cntrl->total_ev_rings = num;
	mhi_cntrl->mhi_event = kcalloc(num, sizeof(*mhi_cntrl->mhi_event),
				       GFP_KERNEL);
	if (!mhi_cntrl->mhi_event)
		return -ENOMEM;

	/* populate ev ring */
	mhi_event = mhi_cntrl->mhi_event;
	i = 0;
	for_each_available_child_of_node(of_node, child) {
		if (strcmp(child->name, "mhi_event"))
			continue;

		mhi_event->er_index = i++;
		ret = of_property_read_u32(child, "mhi,num-elements",
					   (u32 *)&mhi_event->ring.elements);
		if (ret)
			goto error_ev_cfg;

		ret = of_property_read_u32(child, "mhi,intmod",
					   &mhi_event->intmod);
		if (ret)
			goto error_ev_cfg;

		ret = of_property_read_u32(child, "mhi,msi",
					   &mhi_event->msi);
		if (ret)
			goto error_ev_cfg;

		ret = of_property_read_u32(child, "mhi,chan",
					   &mhi_event->chan);
		if (!ret) {
			if (mhi_event->chan >= mhi_cntrl->max_chan)
				goto error_ev_cfg;
			/* this event ring has a dedicated channel */
			mhi_event->mhi_chan =
				&mhi_cntrl->mhi_chan[mhi_event->chan];
		}

		ret = of_property_read_u32(child, "mhi,priority",
					   &mhi_event->priority);
		if (ret)
			goto error_ev_cfg;

		ret = of_property_read_u32(child, "mhi,brstmode",
					   &mhi_event->db_cfg.brstmode);
		if (ret || MHI_INVALID_BRSTMODE(mhi_event->db_cfg.brstmode))
			goto error_ev_cfg;

		mhi_event->db_cfg.process_db =
			(mhi_event->db_cfg.brstmode == MHI_BRSTMODE_ENABLE) ?
			mhi_db_brstmode : mhi_db_brstmode_disable;

		ret = of_property_read_u32(child, "mhi,data-type",
					   &mhi_event->data_type);
		if (ret)
			mhi_event->data_type = MHI_ER_DATA_ELEMENT_TYPE;

		if (mhi_event->data_type > MHI_ER_DATA_TYPE_MAX)
			goto error_ev_cfg;

		switch (mhi_event->data_type) {
		case MHI_ER_DATA_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_data_event_ring;
			break;
		case MHI_ER_CTRL_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_ctrl_ev_ring;
			break;
		case MHI_ER_TSYNC_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_tsync_event_ring;
			break;
		}

		mhi_event->hw_ring = of_property_read_bool(child, "mhi,hw-ev");
		if (mhi_event->hw_ring)
			mhi_cntrl->hw_ev_rings++;
		else
			mhi_cntrl->sw_ev_rings++;
		mhi_event->cl_manage = of_property_read_bool(child,
							"mhi,client-manage");
		mhi_event->offload_ev = of_property_read_bool(child,
							      "mhi,offload");
		mhi_event++;
	}

	/* we need msi for each event ring + additional one for BHI */
	mhi_cntrl->msi_required = mhi_cntrl->total_ev_rings + 1;

	return 0;

error_ev_cfg:

	kfree(mhi_cntrl->mhi_event);
	return -EINVAL;
}
static int of_parse_ch_cfg(struct mhi_controller *mhi_cntrl,
			   struct device_node *of_node)
{
	int ret;
	struct device_node *child;
	u32 chan;

	ret = of_property_read_u32(of_node, "mhi,max-channels",
				   &mhi_cntrl->max_chan);
	if (ret)
		return ret;

	of_node = of_find_node_by_name(of_node, "mhi_channels");
	if (!of_node)
		return -EINVAL;

	mhi_cntrl->mhi_chan = kcalloc(mhi_cntrl->max_chan,
				      sizeof(*mhi_cntrl->mhi_chan), GFP_KERNEL);
	if (!mhi_cntrl->mhi_chan)
		return -ENOMEM;

	INIT_LIST_HEAD(&mhi_cntrl->lpm_chans);

	/* populate channel configurations */
	for_each_available_child_of_node(of_node, child) {
		struct mhi_chan *mhi_chan;

		if (strcmp(child->name, "mhi_chan"))
			continue;

		ret = of_property_read_u32(child, "reg", &chan);
		if (ret || chan >= mhi_cntrl->max_chan)
			goto error_chan_cfg;

		mhi_chan = &mhi_cntrl->mhi_chan[chan];

		ret = of_property_read_string(child, "label",
					      &mhi_chan->name);
		if (ret)
			goto error_chan_cfg;

		mhi_chan->chan = chan;

		ret = of_property_read_u32(child, "mhi,num-elements",
					   (u32 *)&mhi_chan->tre_ring.elements);
		if (!ret && !mhi_chan->tre_ring.elements)
			goto error_chan_cfg;

		/*
		 * For some channels, local ring len should be bigger than
		 * transfer ring len due to internal logical channels in device.
		 * So host can queue much more buffers than transfer ring len.
		 * Example, RSC channels should have a larger local channel
		 * than transfer ring length.
		 */
		ret = of_property_read_u32(child, "mhi,local-elements",
					   (u32 *)&mhi_chan->buf_ring.elements);
		if (ret)
			mhi_chan->buf_ring.elements =
				mhi_chan->tre_ring.elements;

		ret = of_property_read_u32(child, "mhi,event-ring",
					   &mhi_chan->er_index);
		if (ret)
			goto error_chan_cfg;

		ret = of_property_read_u32(child, "mhi,chan-dir",
					   &mhi_chan->dir);
		if (ret)
			goto error_chan_cfg;

		/*
		 * For most channels, chtype is identical to channel directions,
		 * if not defined, assign ch direction to chtype
		 */
		ret = of_property_read_u32(child, "mhi,chan-type",
					   &mhi_chan->type);
		if (ret)
			mhi_chan->type = (enum mhi_ch_type)mhi_chan->dir;

		ret = of_property_read_u32(child, "mhi,ee", &mhi_chan->ee_mask);
		if (ret)
			goto error_chan_cfg;

		of_property_read_u32(child, "mhi,pollcfg",
				     &mhi_chan->db_cfg.pollcfg);

		ret = of_property_read_u32(child, "mhi,data-type",
					   &mhi_chan->xfer_type);
		if (ret)
			goto error_chan_cfg;

		switch (mhi_chan->xfer_type) {
		case MHI_XFER_BUFFER:
			mhi_chan->gen_tre = mhi_gen_tre;
			mhi_chan->queue_xfer = mhi_queue_buf;
			break;
		case MHI_XFER_SKB:
			mhi_chan->queue_xfer = mhi_queue_skb;
			break;
		case MHI_XFER_SCLIST:
			mhi_chan->gen_tre = mhi_gen_tre;
			mhi_chan->queue_xfer = mhi_queue_sclist;
			break;
		case MHI_XFER_NOP:
			mhi_chan->queue_xfer = mhi_queue_nop;
			break;
		case MHI_XFER_DMA:
		case MHI_XFER_RSC_DMA:
			mhi_chan->queue_xfer = mhi_queue_dma;
			break;
		default:
			goto error_chan_cfg;
		}

		mhi_chan->lpm_notify = of_property_read_bool(child,
							     "mhi,lpm-notify");
		mhi_chan->offload_ch = of_property_read_bool(child,
							"mhi,offload-chan");
		mhi_chan->db_cfg.reset_req = of_property_read_bool(child,
							"mhi,db-mode-switch");
		mhi_chan->pre_alloc = of_property_read_bool(child,
							    "mhi,auto-queue");
		mhi_chan->auto_start = of_property_read_bool(child,
							     "mhi,auto-start");
		mhi_chan->wake_capable = of_property_read_bool(child,
							"mhi,wake-capable");

		if (mhi_chan->pre_alloc &&
		    (mhi_chan->dir != DMA_FROM_DEVICE ||
		     mhi_chan->xfer_type != MHI_XFER_BUFFER))
			goto error_chan_cfg;

		/* bi-dir and dirctionless channels must be a offload chan */
		if ((mhi_chan->dir == DMA_BIDIRECTIONAL ||
		     mhi_chan->dir == DMA_NONE) && !mhi_chan->offload_ch)
			goto error_chan_cfg;

		/* if mhi host allocate the buffers then client cannot queue */
		if (mhi_chan->pre_alloc)
			mhi_chan->queue_xfer = mhi_queue_nop;

		if (!mhi_chan->offload_ch) {
			ret = of_property_read_u32(child, "mhi,doorbell-mode",
						   &mhi_chan->db_cfg.brstmode);
			if (ret ||
			    MHI_INVALID_BRSTMODE(mhi_chan->db_cfg.brstmode))
				goto error_chan_cfg;

			mhi_chan->db_cfg.process_db =
				(mhi_chan->db_cfg.brstmode ==
				 MHI_BRSTMODE_ENABLE) ?
				mhi_db_brstmode : mhi_db_brstmode_disable;
		}

		mhi_chan->configured = true;

		if (mhi_chan->lpm_notify)
			list_add_tail(&mhi_chan->node, &mhi_cntrl->lpm_chans);
	}

	return 0;

error_chan_cfg:
	kfree(mhi_cntrl->mhi_chan);

	return -EINVAL;
}
#else
static int of_parse_ev_cfg(struct mhi_controller *mhi_cntrl,
			   struct device_node *of_node)
{
	int i, num = 0;
	struct mhi_event *mhi_event;

	num = NUM_MHI_EVT_RINGS;
	mhi_cntrl->total_ev_rings = num;
	mhi_cntrl->mhi_event = kcalloc(num, sizeof(*mhi_cntrl->mhi_event),
				       GFP_KERNEL);
	if (!mhi_cntrl->mhi_event)
		return -ENOMEM;

	mhi_cntrl->msi_irq_base = 0;
	/* populate ev ring */
	mhi_event = mhi_cntrl->mhi_event;
	i = 0;

	for (i = 0; i < mhi_cntrl->total_ev_rings; i++) {
		mhi_event->er_index = i;

		mhi_event->ring.elements = NUM_MHI_EVT_RING_ELEMENTS; //Event ring length in elements
		if (i == PRIMARY_EVENT_RING || i == ADPL_EVT_RING)
			mhi_event->ring.elements = 256; //256 is enough, and 1024 some times make driver fail to open channel (reason is x6x fail to malloc) 

		mhi_event->intmod = 1; //Interrupt moderation time in ms

		/* see mhi_netdev_status_cb(), when interrupt come, the napi_poll maybe scheduled, so can reduce interrupts
		root@OpenWrt:/# cat /proc/interrupts | grep mhi
		root@OpenWrt:/# cat /sys/kernel/debug/mhi_q/mhi_netdev/pcie_mhi_0306_00.01.00_0/rx_int 
		*/
		if (i == IPA_IN_EVENT_RING)
			mhi_event->intmod = 5;

#ifdef ENABLE_IP_SW0
		if (i == SW_0_IN_EVT_RING)
			mhi_event->intmod = 5;
#endif

		mhi_event->msi = 1 + i + mhi_cntrl->msi_irq_base;  //MSI associated with this event ring

		if (i == IPA_OUT_EVENT_RING)
			mhi_event->chan = MHI_CLIENT_IP_HW_0_OUT; //Dedicated channel number, if it's a dedicated event ring
		else if (i == IPA_IN_EVENT_RING)
			mhi_event->chan = MHI_CLIENT_IP_HW_0_IN; //Dedicated channel number, if it's a dedicated event ring
#ifdef ENABLE_IP_SW0
		else if (i == SW_0_OUT_EVT_RING)
			mhi_event->chan = MHI_CLIENT_IP_SW_0_OUT;
		else if (i == SW_0_IN_EVT_RING)
			mhi_event->chan = MHI_CLIENT_IP_SW_0_IN;
#endif
		else
			mhi_event->chan = 0;

		/* this event ring has a dedicated channel */
		mhi_event->mhi_chan =
			&mhi_cntrl->mhi_chan[mhi_event->chan];

		mhi_event->priority = 1; //Event ring priority, set to 1 for now

		if (mhi_event->chan && mhi_event->mhi_chan->db_cfg.brstmode == MHI_BRSTMODE_ENABLE)
			mhi_event->db_cfg.brstmode = MHI_BRSTMODE_ENABLE;
		else
			mhi_event->db_cfg.brstmode = MHI_BRSTMODE_DISABLE;

		mhi_event->db_cfg.process_db =
			(mhi_event->db_cfg.brstmode == MHI_BRSTMODE_ENABLE) ?
			mhi_db_brstmode : mhi_db_brstmode_disable;

		if (i == IPA_OUT_EVENT_RING || i == IPA_IN_EVENT_RING)
			mhi_event->data_type = MHI_ER_DATA_ELEMENT_TYPE;
#ifdef ENABLE_IP_SW0
		else if (i == SW_0_OUT_EVT_RING || i == SW_0_IN_EVT_RING)
			mhi_event->data_type = MHI_ER_DATA_ELEMENT_TYPE;
#endif
		else
			mhi_event->data_type = MHI_ER_CTRL_ELEMENT_TYPE;

		switch (mhi_event->data_type) {
		case MHI_ER_DATA_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_data_event_ring;
			break;
		case MHI_ER_CTRL_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_ctrl_ev_ring;
			break;
		case MHI_ER_TSYNC_ELEMENT_TYPE:
			mhi_event->process_event = mhi_process_tsync_event_ring;
			break;
		}

		if (i == IPA_OUT_EVENT_RING || i == IPA_IN_EVENT_RING)
			mhi_event->hw_ring = true;
		else
			mhi_event->hw_ring = false;

		if (mhi_event->hw_ring)
			mhi_cntrl->hw_ev_rings++;
		else
			mhi_cntrl->sw_ev_rings++;

		mhi_event->cl_manage = false;
		if (mhi_event->chan == MHI_CLIENT_IP_HW_0_IN || mhi_event->chan == MHI_CLIENT_IP_SW_0_IN)
			mhi_event->cl_manage = true;
		mhi_event->offload_ev = false;
		mhi_event++;
	}

	/* we need msi for each event ring + additional one for BHI */
	mhi_cntrl->msi_required = mhi_cntrl->total_ev_rings + 1 + mhi_cntrl->msi_irq_base;

	return 0;
}

struct chan_cfg_t {
	const char *chan_name;
	u32 chan_id;
	u32 elements;
};

static struct chan_cfg_t chan_cfg[] = {
//"Qualcomm PCIe Loopback"
	{"LOOPBACK", MHI_CLIENT_LOOPBACK_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"LOOPBACK", MHI_CLIENT_LOOPBACK_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe Sahara"
	{"SAHARA", MHI_CLIENT_SAHARA_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"SAHARA", MHI_CLIENT_SAHARA_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe Diagnostics"
	{"DIAG", MHI_CLIENT_DIAG_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"DIAG", MHI_CLIENT_DIAG_IN, NUM_MHI_DIAG_IN_RING_ELEMENTS},
//"Qualcomm PCIe QDSS Data"
	{"QDSS", MHI_CLIENT_QDSS_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"QDSS", MHI_CLIENT_QDSS_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe EFS"
	{"EFS", MHI_CLIENT_EFS_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"EFS", MHI_CLIENT_EFS_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe MBIM"
	{"MBIM", MHI_CLIENT_MBIM_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"MBIM", MHI_CLIENT_MBIM_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe QMI"
	{"QMI0", MHI_CLIENT_QMI_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"QMI0", MHI_CLIENT_QMI_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe QMI"
	//{"QMI1", MHI_CLIENT_QMI_2_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	//{"QMI1", MHI_CLIENT_QMI_2_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe IP CTRL"
	{"IP_CTRL", MHI_CLIENT_IP_CTRL_1_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"IP_CTRL", MHI_CLIENT_IP_CTRL_1_IN, NUM_MHI_CHAN_RING_ELEMENTS},
#if 0 //AG15
//"Qualcomm PCIe IPCR"
	{"IPCR", MHI_CLIENT_DIAG_CONS_IF_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"IPCR", MHI_CLIENT_DIAG_CONS_IF_IN, NUM_MHI_CHAN_RING_ELEMENTS},
#endif
//"Qualcomm PCIe Boot Logging"
	//{"BL", MHI_CLIENT_BOOT_LOG_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	//{"BL", MHI_CLIENT_BOOT_LOG_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm PCIe Modem"
	{"DUN", MHI_CLIENT_DUN_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"DUN", MHI_CLIENT_DUN_IN, NUM_MHI_CHAN_RING_ELEMENTS},
//"Qualcomm EDL "
	{"EDL", MHI_CLIENT_EDL_OUT, NUM_MHI_CHAN_RING_ELEMENTS},
	{"EDL", MHI_CLIENT_EDL_IN, NUM_MHI_CHAN_RING_ELEMENTS},
#ifdef ENABLE_IP_SW0
//"Qualcomm PCIe LOCAL Adapter"
	{"IP_SW0", MHI_CLIENT_IP_SW_0_OUT, NUM_MHI_SW_IP_RING_ELEMENTS},
	{"IP_SW0", MHI_CLIENT_IP_SW_0_IN, NUM_MHI_SW_IP_RING_ELEMENTS},
#endif
//"Qualcomm PCIe WWAN Adapter"
	{"IP_HW0", MHI_CLIENT_IP_HW_0_OUT, NUM_MHI_IPA_OUT_RING_ELEMENTS},
	{"IP_HW0", MHI_CLIENT_IP_HW_0_IN, NUM_MHI_IPA_IN_RING_ELEMENTS},
};

extern int mhi_netdev_mbin_enabled(void);
static int of_parse_ch_cfg(struct mhi_controller *mhi_cntrl,
			   struct device_node *of_node)
{
	u32 chan;
	u32 i, num;
	u32 ring = 0;

	mhi_cntrl->max_chan = MHI_MAX_CHANNELS;
	num = sizeof(chan_cfg)/sizeof(chan_cfg[0]);

	mhi_cntrl->mhi_chan = kcalloc(mhi_cntrl->max_chan,
				      sizeof(*mhi_cntrl->mhi_chan), GFP_KERNEL);
	if (!mhi_cntrl->mhi_chan)
		return -ENOMEM;

	INIT_LIST_HEAD(&mhi_cntrl->lpm_chans);

	/* populate channel configurations */
	for (i = 0; i < num; i++) {
		struct mhi_chan *mhi_chan;

		if (!strncmp( chan_cfg[i].chan_name, "MBIM", 4)) {
			if (!mhi_netdev_mbin_enabled())
				continue;
		}
		else if (!strncmp( chan_cfg[i].chan_name, "QMI", 3)) {
			if (mhi_netdev_mbin_enabled())
				continue;
		}

		chan = chan_cfg[i].chan_id;

		mhi_chan = &mhi_cntrl->mhi_chan[chan];

		mhi_chan->name = chan_cfg[i].chan_name;

		mhi_chan->chan = chan;

		mhi_chan->tre_ring.elements = chan_cfg[i].elements;

		/*
		 * For some channels, local ring len should be bigger than
		 * transfer ring len due to internal logical channels in device.
		 * So host can queue much more buffers than transfer ring len.
		 * Example, RSC channels should have a larger local channel
		 * than transfer ring length.
		 */
		mhi_chan->buf_ring.elements = mhi_chan->tre_ring.elements;

		if (chan == MHI_CLIENT_IP_HW_0_OUT || chan == MHI_CLIENT_IP_HW_0_IN || chan == MHI_CLIENT_DIAG_IN
			|| chan == MHI_CLIENT_IP_SW_0_OUT || chan == MHI_CLIENT_IP_SW_0_IN) {
			mhi_chan->ring = 0;
		}
		else {
			mhi_chan->ring = ring;
			ring += mhi_chan->buf_ring.elements;
		}

		if (chan == MHI_CLIENT_IP_HW_0_OUT)
			mhi_chan->er_index = IPA_OUT_EVENT_RING;
		else if (chan == MHI_CLIENT_IP_HW_0_IN)
			mhi_chan->er_index = IPA_IN_EVENT_RING;
#ifdef ENABLE_IP_SW0
		else if (chan == MHI_CLIENT_IP_SW_0_OUT)
			mhi_chan->er_index = SW_0_OUT_EVT_RING;
		else if (chan == MHI_CLIENT_IP_SW_0_IN)
			mhi_chan->er_index = SW_0_IN_EVT_RING;
#endif
		else
			mhi_chan->er_index = PRIMARY_EVENT_RING;

		mhi_chan->dir = CHAN_INBOUND(chan) ? DMA_FROM_DEVICE : DMA_TO_DEVICE;

		/*
		 * For most channels, chtype is identical to channel directions,
		 * if not defined, assign ch direction to chtype
		 */
		mhi_chan->type = (enum mhi_ch_type)mhi_chan->dir;

		mhi_chan->ee_mask = BIT(MHI_EE_AMSS);
		if (CHAN_SBL(chan))
			mhi_chan->ee_mask = BIT(MHI_EE_SBL);
		else if (CHAN_EDL(chan))
			mhi_chan->ee_mask = BIT(MHI_EE_FP);

		mhi_chan->db_cfg.pollcfg = 0;

  		if (chan == MHI_CLIENT_IP_HW_0_OUT || chan == MHI_CLIENT_IP_SW_0_OUT)
			mhi_chan->xfer_type = MHI_XFER_SKB;
		else if (chan == MHI_CLIENT_IP_HW_0_IN || chan == MHI_CLIENT_IP_SW_0_IN)
			mhi_chan->xfer_type = MHI_XFER_SKB; //MHI_XFER_DMA;
		else
			mhi_chan->xfer_type = MHI_XFER_BUFFER;

		if (chan_cfg[i].elements == 0) {
			mhi_chan->dir = DMA_BIDIRECTIONAL;
			mhi_chan->xfer_type = MHI_XFER_NOP;
		}

		switch (mhi_chan->xfer_type) {
		case MHI_XFER_BUFFER:
			mhi_chan->gen_tre = mhi_gen_tre;
			mhi_chan->queue_xfer = mhi_queue_buf;
			break;
		case MHI_XFER_SKB:
			mhi_chan->queue_xfer = mhi_queue_skb;
			break;
		case MHI_XFER_SCLIST:
			mhi_chan->gen_tre = mhi_gen_tre;
			mhi_chan->queue_xfer = mhi_queue_sclist;
			break;
		case MHI_XFER_NOP:
			mhi_chan->queue_xfer = mhi_queue_nop;
			break;
		case MHI_XFER_DMA:
		case MHI_XFER_RSC_DMA:
			mhi_chan->queue_xfer = mhi_queue_dma;
			break;
		default:
			goto error_chan_cfg;
		}

		mhi_chan->lpm_notify = false;
		mhi_chan->offload_ch = (chan_cfg[i].elements == 0);
		mhi_chan->db_cfg.reset_req = false;
		mhi_chan->pre_alloc = false;
		mhi_chan->auto_start = false;
		mhi_chan->wake_capable = false;

		if (mhi_chan->pre_alloc &&
		    (mhi_chan->dir != DMA_FROM_DEVICE ||
		     mhi_chan->xfer_type != MHI_XFER_BUFFER))
			goto error_chan_cfg;

		/* bi-dir and dirctionless channels must be a offload chan */
		if ((mhi_chan->dir == DMA_BIDIRECTIONAL ||
		     mhi_chan->dir == DMA_NONE) && !mhi_chan->offload_ch)
			goto error_chan_cfg;

		/* if mhi host allocate the buffers then client cannot queue */
		if (mhi_chan->pre_alloc)
			mhi_chan->queue_xfer = mhi_queue_nop;

		if (!mhi_chan->offload_ch) {
			mhi_chan->db_cfg.brstmode = MHI_BRSTMODE_DISABLE;
			if (chan == MHI_CLIENT_IP_HW_0_OUT || chan == MHI_CLIENT_IP_HW_0_IN)
				mhi_chan->db_cfg.brstmode = MHI_BRSTMODE_ENABLE;

			if (MHI_INVALID_BRSTMODE(mhi_chan->db_cfg.brstmode))
				goto error_chan_cfg;

			mhi_chan->db_cfg.process_db =
				(mhi_chan->db_cfg.brstmode ==
				 MHI_BRSTMODE_ENABLE) ?
				mhi_db_brstmode : mhi_db_brstmode_disable;
		}

		mhi_chan->configured = true;

		if (mhi_chan->lpm_notify)
			list_add_tail(&mhi_chan->node, &mhi_cntrl->lpm_chans);
	}

	return 0;

error_chan_cfg:
	kfree(mhi_cntrl->mhi_chan);

	return -EINVAL;
}
#endif

static int of_parse_dt(struct mhi_controller *mhi_cntrl,
		       struct device_node *of_node)
{
	int ret;

	/* parse MHI channel configuration */
	ret = of_parse_ch_cfg(mhi_cntrl, of_node);
	if (ret)
		return ret;

	/* parse MHI event configuration */
	ret = of_parse_ev_cfg(mhi_cntrl, of_node);
	if (ret)
		goto error_ev_cfg;

#if 0
	ret = of_property_read_u32(of_node, "mhi,timeout",
				   &mhi_cntrl->timeout_ms);
	if (ret)
		mhi_cntrl->timeout_ms = MHI_TIMEOUT_MS;

	mhi_cntrl->bounce_buf = of_property_read_bool(of_node, "mhi,use-bb");
	ret = of_property_read_u32(of_node, "mhi,buffer-len",
				   (u32 *)&mhi_cntrl->buffer_len);
	if (ret)
		mhi_cntrl->buffer_len = MHI_MAX_MTU;
#else
	mhi_cntrl->timeout_ms = MHI_TIMEOUT_MS;
	mhi_cntrl->bounce_buf = false;
	mhi_cntrl->buffer_len = MHI_MAX_MTU;
#endif

	return 0;

error_ev_cfg:
	kfree(mhi_cntrl->mhi_chan);

	return ret;
}

int of_register_mhi_controller(struct mhi_controller *mhi_cntrl)
{
	int ret;
	int i;
	struct mhi_event *mhi_event;
	struct mhi_chan *mhi_chan;
	struct mhi_cmd *mhi_cmd;
	struct mhi_device *mhi_dev;

	//if (!mhi_cntrl->of_node)
	//	return -EINVAL;

	for (i = 0; i < MAX_MHI_CONTROLLER; i++) {
		 if (mhi_controller_minors[i].dev_id == mhi_cntrl->dev_id
			&& mhi_controller_minors[i].domain == mhi_cntrl->domain
			&& mhi_controller_minors[i].bus == mhi_cntrl->bus
			&& mhi_controller_minors[i].slot == mhi_cntrl->slot) {
			mhi_cntrl->cntrl_idx = i;
			break;
		}
		else if (mhi_controller_minors[i].dev_id == 0
			&& mhi_controller_minors[i].domain == 0
			&& mhi_controller_minors[i].bus == 0
			&& mhi_controller_minors[i].slot == 0) {
			mhi_controller_minors[i].dev_id = mhi_cntrl->dev_id;
			mhi_controller_minors[i].domain = mhi_cntrl->domain;
			mhi_controller_minors[i].bus = mhi_cntrl->bus;
			mhi_controller_minors[i].slot = mhi_cntrl->slot;
			mhi_cntrl->cntrl_idx = i;
			break;
		}
	}

	if (i == MAX_MHI_CONTROLLER)
		return -EINVAL;

	if (!mhi_cntrl->runtime_get || !mhi_cntrl->runtime_put)
		return -EINVAL;

	if (!mhi_cntrl->status_cb || !mhi_cntrl->link_status)
		return -EINVAL;

	ret = of_parse_dt(mhi_cntrl, mhi_cntrl->of_node);
	if (ret)
		return -EINVAL;

	mhi_cntrl->mhi_cmd = kcalloc(NR_OF_CMD_RINGS,
				     sizeof(*mhi_cntrl->mhi_cmd), GFP_KERNEL);
	if (!mhi_cntrl->mhi_cmd) {
		ret = -ENOMEM;
		goto error_alloc_cmd;
	}

	INIT_LIST_HEAD(&mhi_cntrl->transition_list);
	mutex_init(&mhi_cntrl->pm_mutex);
	rwlock_init(&mhi_cntrl->pm_lock);
	spin_lock_init(&mhi_cntrl->transition_lock);
	spin_lock_init(&mhi_cntrl->wlock);
	INIT_WORK(&mhi_cntrl->st_worker, mhi_pm_st_worker);
	INIT_WORK(&mhi_cntrl->fw_worker, mhi_fw_load_worker);
	INIT_WORK(&mhi_cntrl->syserr_worker, mhi_pm_sys_err_worker);
	INIT_DELAYED_WORK(&mhi_cntrl->ready_worker, mhi_pm_ready_worker);
	init_waitqueue_head(&mhi_cntrl->state_event);

	mhi_cmd = mhi_cntrl->mhi_cmd;
	for (i = 0; i < NR_OF_CMD_RINGS; i++, mhi_cmd++)
		spin_lock_init(&mhi_cmd->lock);

	mhi_event = mhi_cntrl->mhi_event;
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, mhi_event++) {
		if (mhi_event->offload_ev)
			continue;

		mhi_event->mhi_cntrl = mhi_cntrl;
		spin_lock_init(&mhi_event->lock);
		if (mhi_event->data_type == MHI_ER_CTRL_ELEMENT_TYPE)
			tasklet_init(&mhi_event->task, mhi_ctrl_ev_task,
				     (ulong)mhi_event);
		else
			tasklet_init(&mhi_event->task, mhi_ev_task,
				     (ulong)mhi_event);
	}

	mhi_chan = mhi_cntrl->mhi_chan;
	for (i = 0; i < mhi_cntrl->max_chan; i++, mhi_chan++) {
		mutex_init(&mhi_chan->mutex);
		init_completion(&mhi_chan->completion);
		rwlock_init(&mhi_chan->lock);
	}

	if (mhi_cntrl->bounce_buf) {
		mhi_cntrl->map_single = mhi_map_single_use_bb;
		mhi_cntrl->unmap_single = mhi_unmap_single_use_bb;
	} else {
		mhi_cntrl->map_single = mhi_map_single_no_bb;
		mhi_cntrl->unmap_single = mhi_unmap_single_no_bb;
	}

	/* register controller with mhi_bus */
	mhi_dev = mhi_alloc_device(mhi_cntrl);
	if (!mhi_dev) {
		ret = -ENOMEM;
		goto error_alloc_dev;
	}

	mhi_dev->dev_type = MHI_CONTROLLER_TYPE;
	mhi_dev->mhi_cntrl = mhi_cntrl;
	dev_set_name(&mhi_dev->dev, "%04x_%02u.%02u.%02u", mhi_dev->dev_id,
		     mhi_dev->domain, mhi_dev->bus, mhi_dev->slot);

	ret = device_add(&mhi_dev->dev);
	if (ret)
		goto error_add_dev;

	if (mhi_cntrl->cntrl_idx)
		mhi_cntrl->cntrl_dev = device_create(mhi_cntrl_drv.class, mhi_cntrl->dev,
					      MKDEV(mhi_cntrl_drv.major, mhi_cntrl->cntrl_idx), NULL,
					      "mhi_BHI%d", mhi_cntrl->cntrl_idx);
	else
		mhi_cntrl->cntrl_dev = device_create(mhi_cntrl_drv.class, mhi_cntrl->dev,
					      MKDEV(mhi_cntrl_drv.major, mhi_cntrl->cntrl_idx), NULL,
					      "mhi_BHI");

	mhi_cntrl->mhi_dev = mhi_dev;

	mhi_cntrl->parent = mhi_bus.dentry;
	mhi_cntrl->klog_lvl = MHI_MSG_LVL_ERROR;

	/* adding it to this list only for debug purpose */
	mutex_lock(&mhi_bus.lock);
	list_add_tail(&mhi_cntrl->node, &mhi_bus.controller_list);
	mutex_unlock(&mhi_bus.lock);

	return 0;

error_add_dev:
	mhi_dealloc_device(mhi_cntrl, mhi_dev);

error_alloc_dev:
	kfree(mhi_cntrl->mhi_cmd);

error_alloc_cmd:
	kfree(mhi_cntrl->mhi_chan);
	kfree(mhi_cntrl->mhi_event);

	return ret;
};
EXPORT_SYMBOL(of_register_mhi_controller);

void mhi_unregister_mhi_controller(struct mhi_controller *mhi_cntrl)
{
	struct mhi_device *mhi_dev = mhi_cntrl->mhi_dev;

	kfree(mhi_cntrl->mhi_cmd);
	kfree(mhi_cntrl->mhi_event);
	kfree(mhi_cntrl->mhi_chan);
	kfree(mhi_cntrl->mhi_tsync);

	if (mhi_cntrl->cntrl_dev)
		device_destroy(mhi_cntrl_drv.class, MKDEV(mhi_cntrl_drv.major, mhi_cntrl->cntrl_idx));
	device_del(&mhi_dev->dev);
	put_device(&mhi_dev->dev);

	mutex_lock(&mhi_bus.lock);
	list_del(&mhi_cntrl->node);
	mutex_unlock(&mhi_bus.lock);
}

/* set ptr to control private data */
static inline void mhi_controller_set_devdata(struct mhi_controller *mhi_cntrl,
					 void *priv)
{
	mhi_cntrl->priv_data = priv;
}


/* allocate mhi controller to register */
struct mhi_controller *mhi_alloc_controller(size_t size)
{
	struct mhi_controller *mhi_cntrl;

	mhi_cntrl = kzalloc(size + sizeof(*mhi_cntrl), GFP_KERNEL);

	if (mhi_cntrl && size)
		mhi_controller_set_devdata(mhi_cntrl, mhi_cntrl + 1);

	return mhi_cntrl;
}
EXPORT_SYMBOL(mhi_alloc_controller);

int mhi_prepare_for_power_up(struct mhi_controller *mhi_cntrl)
{
	int ret;
	u32 bhie_off;

	mutex_lock(&mhi_cntrl->pm_mutex);

	ret = mhi_init_dev_ctxt(mhi_cntrl);
	if (ret) {
		MHI_ERR("Error with init dev_ctxt\n");
		goto error_dev_ctxt;
	}

	ret = mhi_init_irq_setup(mhi_cntrl);
	if (ret) {
		MHI_ERR("Error setting up irq\n");
		goto error_setup_irq;
	}

	/*
	 * allocate rddm table if specified, this table is for debug purpose
	 * so we'll ignore erros
	 */
	if (mhi_cntrl->rddm_size) {
		mhi_alloc_bhie_table(mhi_cntrl, &mhi_cntrl->rddm_image,
				     mhi_cntrl->rddm_size);

		/*
		 * This controller supports rddm, we need to manually clear
		 * BHIE RX registers since por values are undefined.
		 */
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs, BHIEOFF,
				   &bhie_off);
		if (ret) {
			MHI_ERR("Error getting bhie offset\n");
			goto bhie_error;
		}

		memset_io(mhi_cntrl->regs + bhie_off + BHIE_RXVECADDR_LOW_OFFS,
			  0, BHIE_RXVECSTATUS_OFFS - BHIE_RXVECADDR_LOW_OFFS +
			  4);
	}

	mhi_cntrl->pre_init = true;

	mutex_unlock(&mhi_cntrl->pm_mutex);

	return 0;

bhie_error:
	if (mhi_cntrl->rddm_image) {
		mhi_free_bhie_table(mhi_cntrl, mhi_cntrl->rddm_image);
		mhi_cntrl->rddm_image = NULL;
	}
	mhi_deinit_free_irq(mhi_cntrl);

error_setup_irq:
	mhi_deinit_dev_ctxt(mhi_cntrl);

error_dev_ctxt:
	mutex_unlock(&mhi_cntrl->pm_mutex);

	return ret;
}
EXPORT_SYMBOL(mhi_prepare_for_power_up);

void mhi_unprepare_after_power_down(struct mhi_controller *mhi_cntrl)
{
	if (mhi_cntrl->fbc_image) {
		mhi_free_bhie_table(mhi_cntrl, mhi_cntrl->fbc_image);
		mhi_cntrl->fbc_image = NULL;
	}

	if (mhi_cntrl->rddm_image) {
		mhi_free_bhie_table(mhi_cntrl, mhi_cntrl->rddm_image);
		mhi_cntrl->rddm_image = NULL;
	}

	mhi_deinit_free_irq(mhi_cntrl);
	mhi_deinit_dev_ctxt(mhi_cntrl);
	mhi_cntrl->pre_init = false;
}

/* match dev to drv */
static int mhi_match(struct device *dev, struct device_driver *drv)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_driver *mhi_drv = to_mhi_driver(drv);
	const struct mhi_device_id *id;

	/* if controller type there is no client driver associated with it */
	if (mhi_dev->dev_type == MHI_CONTROLLER_TYPE)
		return 0;

	for (id = mhi_drv->id_table; id->chan[0]; id++)
		if (!strcmp(mhi_dev->chan_name, id->chan)) {
			mhi_dev->id = id;
			return 1;
		}

	return 0;
};

static void mhi_release_device(struct device *dev)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);

	if (mhi_dev->ul_chan)
		mhi_dev->ul_chan->mhi_dev = NULL;

	if (mhi_dev->dl_chan)
		mhi_dev->dl_chan->mhi_dev = NULL;

	kfree(mhi_dev);
}

struct bus_type mhi_bus_type = {
	.name = "mhi_q",
	.dev_name = "mhi_q",
	.match = mhi_match,
};

static int mhi_driver_probe(struct device *dev)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct device_driver *drv = dev->driver;
	struct mhi_driver *mhi_drv = to_mhi_driver(drv);
	struct mhi_event *mhi_event;
	struct mhi_chan *ul_chan = mhi_dev->ul_chan;
	struct mhi_chan *dl_chan = mhi_dev->dl_chan;
	bool auto_start = false;
	int ret;

	/* bring device out of lpm */
	ret = mhi_device_get_sync(mhi_dev);
	if (ret)
		return ret;

	ret = -EINVAL;
	if (ul_chan) {
		/* lpm notification require status_cb */
		if (ul_chan->lpm_notify && !mhi_drv->status_cb)
			goto exit_probe;

		if (!ul_chan->offload_ch && !mhi_drv->ul_xfer_cb)
			goto exit_probe;

		ul_chan->xfer_cb = mhi_drv->ul_xfer_cb;
		mhi_dev->status_cb = mhi_drv->status_cb;
		auto_start = ul_chan->auto_start;
	}

	if (dl_chan) {
		if (dl_chan->lpm_notify && !mhi_drv->status_cb)
			goto exit_probe;

		if (!dl_chan->offload_ch && !mhi_drv->dl_xfer_cb)
			goto exit_probe;

		mhi_event = &mhi_cntrl->mhi_event[dl_chan->er_index];

		/*
		 * if this channal event ring manage by client, then
		 * status_cb must be defined so we can send the async
		 * cb whenever there are pending data
		 */
		if (mhi_event->cl_manage && !mhi_drv->status_cb)
			goto exit_probe;

		dl_chan->xfer_cb = mhi_drv->dl_xfer_cb;

		/* ul & dl uses same status cb */
		mhi_dev->status_cb = mhi_drv->status_cb;
		auto_start = (auto_start || dl_chan->auto_start);
	}

	ret = mhi_drv->probe(mhi_dev, mhi_dev->id);

	if (!ret && auto_start)
		mhi_prepare_for_transfer(mhi_dev);

exit_probe:
	mhi_device_put(mhi_dev);

	return ret;
}

static int mhi_driver_remove(struct device *dev)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_driver *mhi_drv = to_mhi_driver(dev->driver);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan;
	enum MHI_CH_STATE ch_state[] = {
		MHI_CH_STATE_DISABLED,
		MHI_CH_STATE_DISABLED
	};
	int dir;

	/* control device has no work to do */
	if (mhi_dev->dev_type == MHI_CONTROLLER_TYPE)
		return 0;

	MHI_LOG("Removing device for chan:%s\n", mhi_dev->chan_name);

	/* reset both channels */
	for (dir = 0; dir < 2; dir++) {
		mhi_chan = dir ? mhi_dev->ul_chan : mhi_dev->dl_chan;

		if (!mhi_chan)
			continue;

		/* wake all threads waiting for completion */
		write_lock_irq(&mhi_chan->lock);
		mhi_chan->ccs = MHI_EV_CC_INVALID;
		complete_all(&mhi_chan->completion);
		write_unlock_irq(&mhi_chan->lock);

		/* move channel state to disable, no more processing */
		mutex_lock(&mhi_chan->mutex);
		write_lock_irq(&mhi_chan->lock);
		ch_state[dir] = mhi_chan->ch_state;
		mhi_chan->ch_state = MHI_CH_STATE_SUSPENDED;
		write_unlock_irq(&mhi_chan->lock);

		/* reset the channel */
		if (!mhi_chan->offload_ch)
			mhi_reset_chan(mhi_cntrl, mhi_chan);

		mutex_unlock(&mhi_chan->mutex);
	}

	/* destroy the device */
	mhi_drv->remove(mhi_dev);

	/* de_init channel if it was enabled */
	for (dir = 0; dir < 2; dir++) {
		mhi_chan = dir ? mhi_dev->ul_chan : mhi_dev->dl_chan;

		if (!mhi_chan)
			continue;

		mutex_lock(&mhi_chan->mutex);

		if (ch_state[dir] == MHI_CH_STATE_ENABLED &&
		    !mhi_chan->offload_ch)
			mhi_deinit_chan_ctxt(mhi_cntrl, mhi_chan);

		mhi_chan->ch_state = MHI_CH_STATE_DISABLED;

		mutex_unlock(&mhi_chan->mutex);
	}


	if (mhi_cntrl->tsync_dev == mhi_dev)
		mhi_cntrl->tsync_dev = NULL;

	/* relinquish any pending votes */
	read_lock_bh(&mhi_cntrl->pm_lock);
	while (atomic_read(&mhi_dev->dev_wake))
		mhi_device_put(mhi_dev);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	return 0;
}

int mhi_driver_register(struct mhi_driver *mhi_drv)
{
	struct device_driver *driver = &mhi_drv->driver;

	if (!mhi_drv->probe || !mhi_drv->remove)
		return -EINVAL;

	driver->bus = &mhi_bus_type;
	driver->probe = mhi_driver_probe;
	driver->remove = mhi_driver_remove;
	return driver_register(driver);
}
EXPORT_SYMBOL(mhi_driver_register);

void mhi_driver_unregister(struct mhi_driver *mhi_drv)
{
	driver_unregister(&mhi_drv->driver);
}
EXPORT_SYMBOL(mhi_driver_unregister);

struct mhi_device *mhi_alloc_device(struct mhi_controller *mhi_cntrl)
{
	struct mhi_device *mhi_dev = kzalloc(sizeof(*mhi_dev), GFP_KERNEL);
	struct device *dev;

	if (!mhi_dev)
		return NULL;

	dev = &mhi_dev->dev;
	device_initialize(dev);
	dev->bus = &mhi_bus_type;
	dev->release = mhi_release_device;
	dev->parent = mhi_cntrl->dev;
	mhi_dev->mhi_cntrl = mhi_cntrl;
	mhi_dev->vendor = mhi_cntrl->vendor;
	mhi_dev->dev_id = mhi_cntrl->dev_id;
	mhi_dev->domain = mhi_cntrl->domain;
	mhi_dev->bus = mhi_cntrl->bus;
	mhi_dev->slot = mhi_cntrl->slot;
	mhi_dev->mtu = MHI_MAX_MTU;
	atomic_set(&mhi_dev->dev_wake, 0);

	return mhi_dev;
}

static int mhi_cntrl_open(struct inode *inode, struct file *f)
{
	int ret = -ENODEV;
	struct mhi_controller *mhi_cntrl;

	mutex_lock(&mhi_bus.lock);
	list_for_each_entry(mhi_cntrl, &mhi_bus.controller_list, node) {
		if (MINOR(inode->i_rdev) == mhi_cntrl->cntrl_idx) {
			ret = 0;
			f->private_data = mhi_cntrl;
			break;
		}
	}
	mutex_unlock(&mhi_bus.lock);

	return ret;
}

static int mhi_cntrl_release(struct inode *inode, struct file *f)
{
	f->private_data = NULL;
	return 0;
}

#define IOCTL_BHI_GETDEVINFO 0x8BE0 + 1
#define IOCTL_BHI_WRITEIMAGE 0x8BE0 + 2
long bhi_get_dev_info(struct mhi_controller *mhi_cntrl, void __user *to);
long bhi_write_image(struct mhi_controller *mhi_cntrl, void __user *from);

static long mhi_cntrl_ioctl(struct file *f, unsigned int cmd, unsigned long __arg)
{
	long ret = -ENODEV;
	struct mhi_controller *mhi_cntrl;

	mutex_lock(&mhi_bus.lock);
	list_for_each_entry(mhi_cntrl, &mhi_bus.controller_list, node) {
		if (mhi_cntrl == (struct mhi_controller *)f->private_data) {
			ret = 0;
			break;
		}
	}
	mutex_unlock(&mhi_bus.lock);

	if (ret)
		return ret;

	switch (cmd) {
		case IOCTL_BHI_GETDEVINFO:
			ret = bhi_get_dev_info(mhi_cntrl, (void __user *)__arg);
		break;

		case IOCTL_BHI_WRITEIMAGE:
			ret = bhi_write_image(mhi_cntrl, (void __user *)__arg);
		break;

		default:
			ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct file_operations mhi_cntrl_fops = {
	.unlocked_ioctl =	mhi_cntrl_ioctl,
	.open =			mhi_cntrl_open,
	.release =		mhi_cntrl_release,
};

static int __init mhi_cntrl_init(void)
{
	int ret;

	ret = register_chrdev(0, MHI_CNTRL_DRIVER_NAME, &mhi_cntrl_fops);
	if (ret < 0)
		return ret;

	mhi_cntrl_drv.major = ret;
	mhi_cntrl_drv.class = class_create(THIS_MODULE, MHI_CNTRL_DRIVER_NAME);
	if (IS_ERR(mhi_cntrl_drv.class)) {
		unregister_chrdev(mhi_cntrl_drv.major, MHI_CNTRL_DRIVER_NAME);
		return -ENODEV;
	   }

	mutex_init(&mhi_cntrl_drv.lock);
	INIT_LIST_HEAD(&mhi_cntrl_drv.head);

	return 0;
}

void mhi_cntrl_exit(void)
{
	class_destroy(mhi_cntrl_drv.class);
	unregister_chrdev(mhi_cntrl_drv.major, MHI_CNTRL_DRIVER_NAME);
}

extern int mhi_dtr_init(void);
extern void mhi_dtr_exit(void);
extern int mhi_device_netdev_init(struct dentry *parent);
extern void mhi_device_netdev_exit(void);
extern int mhi_device_uci_init(void);
extern void mhi_device_uci_exit(void);
extern int mhi_controller_qcom_init(void);
extern void mhi_controller_qcom_exit(void);

static char mhi_version[] = "Quectel_Linux_PCIE_MHI_Driver_"PCIE_MHI_DRIVER_VERSION;
module_param_string(mhi_version, mhi_version, sizeof(mhi_version), S_IRUGO);

static int __init mhi_init(void)
{
	int ret;

	pr_info("%s %s\n", __func__, mhi_version);

	mutex_init(&mhi_bus.lock);
	INIT_LIST_HEAD(&mhi_bus.controller_list);

	/* parent directory */
	mhi_bus.dentry = debugfs_create_dir(mhi_bus_type.name, NULL);

	ret = bus_register(&mhi_bus_type);
	if (ret) {
		pr_err("Error bus_register ret:%d\n", ret);
		return ret;
	}

	ret = mhi_dtr_init();
	if (ret) {
		pr_err("Error mhi_dtr_init ret:%d\n", ret);
		bus_unregister(&mhi_bus_type);
		return ret;
	}

	ret = mhi_device_netdev_init(mhi_bus.dentry);
	if (ret) {
		pr_err("Error mhi_device_netdev_init ret:%d\n", ret);
	}

	ret = mhi_device_uci_init();
	if (ret) {
		pr_err("Error mhi_device_uci_init ret:%d\n", ret);
	}

	ret = mhi_cntrl_init();
	if (ret) {
		pr_err("Error mhi_cntrl_init ret:%d\n", ret);
	}

	ret = mhi_controller_qcom_init();
	if (ret) {
		pr_err("Error mhi_controller_qcom_init ret:%d\n", ret);
	}

	return ret;
}

static void mhi_exit(void)
{
	mhi_controller_qcom_exit();
	mhi_cntrl_exit();
	mhi_device_uci_exit();
	mhi_device_netdev_exit();
	mhi_dtr_exit();
	bus_unregister(&mhi_bus_type);
	debugfs_remove_recursive(mhi_bus.dentry);
}

module_init(mhi_init);
module_exit(mhi_exit);

MODULE_LICENSE("GPL v2");
MODULE_ALIAS("MHI_CORE");
MODULE_DESCRIPTION("MHI Host Interface");
