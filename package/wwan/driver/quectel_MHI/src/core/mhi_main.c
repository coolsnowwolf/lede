// SPDX-License-Identifier: GPL-2.0-only
/* Copyright (c) 2018-2019, The Linux Foundation. All rights reserved. */

#include <linux/debugfs.h>
#include <linux/device.h>
#include <linux/dma-direction.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/of.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/io.h>
#include "mhi.h"
#include "mhi_internal.h"

static void __mhi_unprepare_channel(struct mhi_controller *mhi_cntrl,
				    struct mhi_chan *mhi_chan);

int __must_check mhi_read_reg(struct mhi_controller *mhi_cntrl,
			      void __iomem *base,
			      u32 offset,
			      u32 *out)
{
	u32 tmp = readl_relaxed(base + offset);

	/* unexpected value, query the link status */
	if (PCI_INVALID_READ(tmp) &&
	    mhi_cntrl->link_status(mhi_cntrl, mhi_cntrl->priv_data))
		return -EIO;

	*out = tmp;

	return 0;
}

int __must_check mhi_read_reg_field(struct mhi_controller *mhi_cntrl,
				    void __iomem *base,
				    u32 offset,
				    u32 mask,
				    u32 shift,
				    u32 *out)
{
	u32 tmp;
	int ret;

	ret = mhi_read_reg(mhi_cntrl, base, offset, &tmp);
	if (ret)
		return ret;

	*out = (tmp & mask) >> shift;

	return 0;
}

int mhi_get_capability_offset(struct mhi_controller *mhi_cntrl,
			      u32 capability,
			      u32 *offset)
{
	u32 cur_cap, next_offset;
	int ret;

	/* get the 1st supported capability offset */
	ret = mhi_read_reg_field(mhi_cntrl, mhi_cntrl->regs, MISC_OFFSET,
				 MISC_CAP_MASK, MISC_CAP_SHIFT, offset);
	if (ret)
		return ret;
	if (*offset >= 0x1000)
		return -ENXIO;
	do {
		ret = mhi_read_reg_field(mhi_cntrl, mhi_cntrl->regs, *offset,
					 CAP_CAPID_MASK, CAP_CAPID_SHIFT,
					 &cur_cap);
		if (ret)
			return ret;

		if (cur_cap == capability)
			return 0;

		ret = mhi_read_reg_field(mhi_cntrl, mhi_cntrl->regs, *offset,
					 CAP_NEXT_CAP_MASK, CAP_NEXT_CAP_SHIFT,
					 &next_offset);
		if (ret)
			return ret;

		*offset += next_offset;
	} while (next_offset);

	return -ENXIO;
}

void mhi_write_reg(struct mhi_controller *mhi_cntrl,
		   void __iomem *base,
		   u32 offset,
		   u32 val)
{
	writel_relaxed(val, base + offset);
}

void mhi_write_reg_field(struct mhi_controller *mhi_cntrl,
			 void __iomem *base,
			 u32 offset,
			 u32 mask,
			 u32 shift,
			 u32 val)
{
	int ret;
	u32 tmp;

	ret = mhi_read_reg(mhi_cntrl, base, offset, &tmp);
	if (ret)
		return;

	tmp &= ~mask;
	tmp |= (val << shift);
	mhi_write_reg(mhi_cntrl, base, offset, tmp);
}

void mhi_write_db(struct mhi_controller *mhi_cntrl,
		  void __iomem *db_addr,
		  dma_addr_t wp)
{
	mhi_write_reg(mhi_cntrl, db_addr, 4, upper_32_bits(wp));
	mhi_write_reg(mhi_cntrl, db_addr, 0, lower_32_bits(wp));
#if 0 //carl.yin 20190527  for debug
	if ((lower_32_bits(db_addr)&0xFFF) != 0x620)
	{
		u32 out = 0;
		int ret = mhi_read_reg(mhi_cntrl, db_addr, 0, &out);
		if (out != lower_32_bits(wp))
			MHI_ERR("%s db=%x, wp=w:%x - r:%x, ret=%d\n", __func__, lower_32_bits(db_addr), lower_32_bits(wp), out, ret);
	}
#endif
}

void mhi_db_brstmode(struct mhi_controller *mhi_cntrl,
		     struct db_cfg *db_cfg,
		     void __iomem *db_addr,
		     dma_addr_t wp)
{
	if (db_cfg->db_mode) {
		db_cfg->db_val = wp;
		mhi_write_db(mhi_cntrl, db_addr, wp);
		db_cfg->db_mode = false;
	}
}

void mhi_db_brstmode_disable(struct mhi_controller *mhi_cntrl,
			     struct db_cfg *db_cfg,
			     void __iomem *db_addr,
			     dma_addr_t wp)
{
	db_cfg->db_val = wp;
	mhi_write_db(mhi_cntrl, db_addr, wp);
}

void mhi_ring_er_db(struct mhi_event *mhi_event)
{
	struct mhi_ring *ring = &mhi_event->ring;

	mhi_event->db_cfg.process_db(mhi_event->mhi_cntrl, &mhi_event->db_cfg,
				     ring->db_addr, *ring->ctxt_wp);
}

void mhi_ring_cmd_db(struct mhi_controller *mhi_cntrl, struct mhi_cmd *mhi_cmd)
{
	dma_addr_t db;
	struct mhi_ring *ring = &mhi_cmd->ring;

	db = ring->iommu_base + (ring->wp - ring->base);
	*ring->ctxt_wp = db;
	mhi_write_db(mhi_cntrl, ring->db_addr, db);
}

//#define DEBUG_CHAN100_DB
#ifdef DEBUG_CHAN100_DB
static atomic_t chan100_seq = ATOMIC_INIT(0);
#define CHAN100_SIZE 0x1000
static unsigned int chan100_t[CHAN100_SIZE];
#endif

void mhi_ring_chan_db(struct mhi_controller *mhi_cntrl,
		      struct mhi_chan *mhi_chan)
{
	struct mhi_ring *ring = &mhi_chan->tre_ring;
	dma_addr_t db;

	db = ring->iommu_base + (ring->wp - ring->base);
	/*
	 * Writes to the new ring element must be visible to the hardware
	 * before letting h/w know there is new element to fetch.
	 */
	dma_wmb();
	*ring->ctxt_wp = db;
	mhi_chan->db_cfg.process_db(mhi_cntrl, &mhi_chan->db_cfg, ring->db_addr,
				    db);
}

enum mhi_ee mhi_get_exec_env(struct mhi_controller *mhi_cntrl)
{
	u32 exec;
	int ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->bhi, BHI_EXECENV, &exec);

	return (ret) ? MHI_EE_MAX : exec;
}

enum mhi_dev_state mhi_get_mhi_state(struct mhi_controller *mhi_cntrl)
{
	u32 state;
	int ret = mhi_read_reg_field(mhi_cntrl, mhi_cntrl->regs, MHISTATUS,
				     MHISTATUS_MHISTATE_MASK,
				     MHISTATUS_MHISTATE_SHIFT, &state);
	return ret ? MHI_STATE_MAX : state;
}

int mhi_queue_sclist(struct mhi_device *mhi_dev,
		     struct mhi_chan *mhi_chan,
		     void *buf,
		     size_t len,
		     enum MHI_FLAGS mflags)
{
	return -EINVAL;
}

int mhi_queue_nop(struct mhi_device *mhi_dev,
		  struct mhi_chan *mhi_chan,
		  void *buf,
		  size_t len,
		  enum MHI_FLAGS mflags)
{
	return -EINVAL;
}

static void mhi_add_ring_element(struct mhi_controller *mhi_cntrl,
				 struct mhi_ring *ring)
{
	void *wp = ring->wp;
	wp += ring->el_size;
	if (wp >= (ring->base + ring->len))
		wp = ring->base;
	ring->wp = wp;
	/* smp update */
	smp_wmb();
}

static void mhi_del_ring_element(struct mhi_controller *mhi_cntrl,
				 struct mhi_ring *ring)
{
	void *rp = ring->rp;
	rp += ring->el_size;
	if (rp >= (ring->base + ring->len))
		rp = ring->base;
	ring->rp = rp;
	/* smp update */
	smp_wmb();
}

static int get_nr_avail_ring_elements(struct mhi_controller *mhi_cntrl,
				      struct mhi_ring *ring)
{
	int nr_el;

	if (ring->wp < ring->rp)
		nr_el = ((ring->rp - ring->wp) / ring->el_size) - 1;
	else {
		nr_el = (ring->rp - ring->base) / ring->el_size;
		nr_el += ((ring->base + ring->len - ring->wp) /
			  ring->el_size) - 1;
	}
	return nr_el;
}

static u32 get_used_ring_elements(void *rp, void *wp, u32 el_num)
{
	u32 nr_el;

	if (wp >= rp)
		nr_el = (wp - rp)/sizeof(struct mhi_tre);
	else {
		nr_el = (rp - wp)/sizeof(struct mhi_tre);
		nr_el = el_num - nr_el;
	}
	return nr_el;
}

static void *mhi_to_virtual(struct mhi_ring *ring, dma_addr_t addr)
{
	return (addr - ring->iommu_base) + ring->base;
}

dma_addr_t mhi_to_physical(struct mhi_ring *ring, void *addr)
{
	return (addr - ring->base) + ring->iommu_base;
}

static void mhi_recycle_ev_ring_element(struct mhi_controller *mhi_cntrl,
					struct mhi_ring *ring)
{
	void *rp, *wp;

	/* update the WP */
	wp = ring->wp;
	wp += ring->el_size;
	if (wp >= (ring->base + ring->len)) {
		wp = ring->base;
	}
	ring->wp = wp;

	*ring->ctxt_wp = ring->iommu_base + (ring->wp - ring->base);

	/* update the RP */
	rp = ring->rp;
	rp += ring->el_size;
	if (rp >= (ring->base + ring->len))
		rp = ring->base;
	ring->rp = rp;

	/* visible to other cores */
	smp_wmb();
}

static bool mhi_is_ring_full(struct mhi_controller *mhi_cntrl,
			     struct mhi_ring *ring)
{
	void *tmp = ring->wp + ring->el_size;

	if (tmp >= (ring->base + ring->len))
		tmp = ring->base;

	return (tmp == ring->rp);
}

int mhi_map_single_no_bb(struct mhi_controller *mhi_cntrl,
			 struct mhi_buf_info *buf_info)
{
	buf_info->p_addr = dma_map_single(mhi_cntrl->dev, buf_info->v_addr,
					  buf_info->len, buf_info->dir);
	if (dma_mapping_error(mhi_cntrl->dev, buf_info->p_addr))
		return -ENOMEM;

	return 0;
}

int mhi_map_single_use_bb(struct mhi_controller *mhi_cntrl,
			  struct mhi_buf_info *buf_info)
{
	void *buf = mhi_alloc_coherent(mhi_cntrl, buf_info->len,
				       &buf_info->p_addr, GFP_ATOMIC);

	if (!buf)
		return -ENOMEM;

	if (buf_info->dir == DMA_TO_DEVICE)
		memcpy(buf, buf_info->v_addr, buf_info->len);

	buf_info->bb_addr = buf;

	return 0;
}

void mhi_unmap_single_no_bb(struct mhi_controller *mhi_cntrl,
			    struct mhi_buf_info *buf_info)
{
	dma_unmap_single(mhi_cntrl->dev, buf_info->p_addr, buf_info->len,
			 buf_info->dir);
}

void mhi_unmap_single_use_bb(struct mhi_controller *mhi_cntrl,
			    struct mhi_buf_info *buf_info)
{
	if (buf_info->dir == DMA_FROM_DEVICE)
		memcpy(buf_info->v_addr, buf_info->bb_addr, buf_info->len);

	mhi_free_coherent(mhi_cntrl, buf_info->len, buf_info->bb_addr,
			  buf_info->p_addr);
}

#ifdef ENABLE_MHI_MON
static void mon_bus_submit(struct mhi_controller *mbus, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len)
{
	unsigned long flags;
	struct list_head *pos;
	struct mon_reader *r;

	spin_lock_irqsave(&mbus->lock, flags);
	mbus->cnt_events++;
	list_for_each (pos, &mbus->r_list) {
		r = list_entry(pos, struct mon_reader, r_link);
		r->rnf_submit(r->r_data, chan, wp, mhi_tre, buf, len);
	}
	spin_unlock_irqrestore(&mbus->lock, flags);
}

static void mon_bus_receive(struct mhi_controller *mbus, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre, void *buf, size_t len)
{
	unsigned long flags;
	struct list_head *pos;
	struct mon_reader *r;

	spin_lock_irqsave(&mbus->lock, flags);
	mbus->cnt_events++;
	list_for_each (pos, &mbus->r_list) {
		r = list_entry(pos, struct mon_reader, r_link);
		r->rnf_receive(r->r_data, chan, wp, mhi_tre, buf, len);
	}
	spin_unlock_irqrestore(&mbus->lock, flags);
}

static void mon_bus_complete(struct mhi_controller *mbus, u32 chan, dma_addr_t wp, struct mhi_tre *mhi_tre)
{
	unsigned long flags;
	struct list_head *pos;
	struct mon_reader *r;

	spin_lock_irqsave(&mbus->lock, flags);
	mbus->cnt_events++;
	list_for_each (pos, &mbus->r_list) {
		r = list_entry(pos, struct mon_reader, r_link);
		r->rnf_complete(r->r_data, chan, wp, mhi_tre);
	}
	spin_unlock_irqrestore(&mbus->lock, flags);
}
#endif

int mhi_queue_skb(struct mhi_device *mhi_dev,
		  struct mhi_chan *mhi_chan,
		  void *buf,
		  size_t len,
		  enum MHI_FLAGS mflags)
{
	struct sk_buff *skb = buf;
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_ring *tre_ring = &mhi_chan->tre_ring;
	struct mhi_ring *buf_ring = &mhi_chan->buf_ring;
	struct mhi_buf_info *buf_info;
	struct mhi_tre *mhi_tre;
	bool assert_wake = false;
	int ret;

	if (mhi_is_ring_full(mhi_cntrl, tre_ring))
		return -ENOMEM;

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (unlikely(MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state))) {
		MHI_VERB("MHI is not in activate state, pm_state:%s\n",
			 to_mhi_pm_state_str(mhi_cntrl->pm_state));
		read_unlock_bh(&mhi_cntrl->pm_lock);

		return -EIO;
	}

	/* we're in M3 or transitioning to M3 */
	if (MHI_PM_IN_SUSPEND_STATE(mhi_cntrl->pm_state)) {
		mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
		mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
	}

	/*
	 * For UL channels always assert WAKE until work is done,
	 * For DL channels only assert if MHI is in a LPM
	 */
	if (mhi_chan->dir == DMA_TO_DEVICE ||
	    (mhi_chan->dir == DMA_FROM_DEVICE &&
	     mhi_cntrl->pm_state != MHI_PM_M0)) {
		assert_wake = true;
		mhi_cntrl->wake_get(mhi_cntrl, false);
	}

	/* generate the tre */
	buf_info = buf_ring->wp;
	buf_info->v_addr = skb->data;
	buf_info->cb_buf = skb;
	buf_info->wp = tre_ring->wp;
	buf_info->dir = mhi_chan->dir;
	buf_info->len = len;
	ret = mhi_cntrl->map_single(mhi_cntrl, buf_info);
	if (ret)
		goto map_error;

	mhi_tre = tre_ring->wp;

	mhi_tre->ptr = MHI_TRE_DATA_PTR(buf_info->p_addr);
	mhi_tre->dword[0] = MHI_TRE_DATA_DWORD0(buf_info->len);
	mhi_tre->dword[1] = MHI_TRE_DATA_DWORD1(1, 1, 0, 0);

#ifdef ENABLE_MHI_MON
	if (mhi_cntrl->nreaders) {
		mon_bus_submit(mhi_cntrl, mhi_chan->chan,
			mhi_to_physical(tre_ring, mhi_tre), mhi_tre, buf_info->v_addr, mhi_chan->chan&0x1 ? 0 : buf_info->len);
	}
#endif

	MHI_VERB("chan:%d WP:0x%llx TRE:0x%llx 0x%08x 0x%08x\n", mhi_chan->chan,
		 (u64)mhi_to_physical(tre_ring, mhi_tre), mhi_tre->ptr,
		 mhi_tre->dword[0], mhi_tre->dword[1]);

	if (mhi_chan->dir == DMA_TO_DEVICE) {
		if (atomic_inc_return(&mhi_cntrl->pending_pkts) == 1)
			mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
	}

	read_lock_bh(&mhi_chan->lock);
	/* increment WP */
	mhi_add_ring_element(mhi_cntrl, tre_ring);
	mhi_add_ring_element(mhi_cntrl, buf_ring);

#ifdef DEBUG_CHAN100_DB
	if (mhi_chan->chan == 100) {
		chan100_t[atomic_inc_return(&chan100_seq)&(CHAN100_SIZE-1)] = (((unsigned long)tre_ring->wp)&0xffff) | (mhi_chan->db_cfg.db_mode<<31) | (0<<30);
	}
#endif
	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state))) {
		mhi_ring_chan_db(mhi_cntrl, mhi_chan);
	}
		read_unlock_bh(&mhi_chan->lock);

	if (mhi_chan->dir == DMA_FROM_DEVICE && assert_wake)
		mhi_cntrl->wake_put(mhi_cntrl, true);

	read_unlock_bh(&mhi_cntrl->pm_lock);

	return 0;

map_error:
	if (assert_wake)
		mhi_cntrl->wake_put(mhi_cntrl, false);

	read_unlock_bh(&mhi_cntrl->pm_lock);

	return ret;
}

int mhi_queue_dma(struct mhi_device *mhi_dev,
		  struct mhi_chan *mhi_chan,
		  void *buf,
		  size_t len,
		  enum MHI_FLAGS mflags)
{
	struct mhi_buf *mhi_buf = buf;
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_ring *tre_ring = &mhi_chan->tre_ring;
	struct mhi_ring *buf_ring = &mhi_chan->buf_ring;
	struct mhi_buf_info *buf_info;
	struct mhi_tre *mhi_tre;
	bool assert_wake = false;

	if (mhi_is_ring_full(mhi_cntrl, tre_ring))
		return -ENOMEM;

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (unlikely(MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state))) {
		MHI_VERB("MHI is not in activate state, pm_state:%s\n",
			 to_mhi_pm_state_str(mhi_cntrl->pm_state));
		read_unlock_bh(&mhi_cntrl->pm_lock);

		return -EIO;
	}

	/* we're in M3 or transitioning to M3 */
	if (MHI_PM_IN_SUSPEND_STATE(mhi_cntrl->pm_state)) {
		mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
		mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
	}

	/*
	 * For UL channels always assert WAKE until work is done,
	 * For DL channels only assert if MHI is in a LPM
	 */
	if (mhi_chan->dir == DMA_TO_DEVICE ||
	    (mhi_chan->dir == DMA_FROM_DEVICE &&
	     mhi_cntrl->pm_state != MHI_PM_M0)) {
		assert_wake = true;
		mhi_cntrl->wake_get(mhi_cntrl, false);
	}

	/* generate the tre */
	buf_info = buf_ring->wp;
	MHI_ASSERT(buf_info->used, "TRE Not Freed\n");
	buf_info->p_addr = mhi_buf->dma_addr;
	buf_info->pre_mapped = true;
	buf_info->cb_buf = mhi_buf;
	buf_info->wp = tre_ring->wp;
	buf_info->dir = mhi_chan->dir;
	buf_info->len = len;

	mhi_tre = tre_ring->wp;

	if (mhi_chan->xfer_type == MHI_XFER_RSC_DMA) {
		buf_info->used = true;
		mhi_tre->ptr =
			MHI_RSCTRE_DATA_PTR(buf_info->p_addr, buf_info->len);
		mhi_tre->dword[0] =
			MHI_RSCTRE_DATA_DWORD0(buf_ring->wp - buf_ring->base);
		mhi_tre->dword[1] = MHI_RSCTRE_DATA_DWORD1;
	} else {
		mhi_tre->ptr = MHI_TRE_DATA_PTR(buf_info->p_addr);
		mhi_tre->dword[0] = MHI_TRE_DATA_DWORD0(buf_info->len);
		mhi_tre->dword[1] = MHI_TRE_DATA_DWORD1(1, 1, 0, 0);
	}

#ifdef ENABLE_MHI_MON
	if (mhi_cntrl->nreaders) {
		mon_bus_submit(mhi_cntrl, mhi_chan->chan,
			mhi_to_physical(tre_ring, mhi_tre), mhi_tre, buf_info->v_addr, mhi_chan->chan&0x1 ? 0: buf_info->len);
	}
#endif

	MHI_VERB("chan:%d WP:0x%llx TRE:0x%llx 0x%08x 0x%08x\n", mhi_chan->chan,
		 (u64)mhi_to_physical(tre_ring, mhi_tre), mhi_tre->ptr,
		 mhi_tre->dword[0], mhi_tre->dword[1]);

	if (mhi_chan->dir == DMA_TO_DEVICE) {
		if (atomic_inc_return(&mhi_cntrl->pending_pkts) == 1)
			mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
	}

	read_lock_bh(&mhi_chan->lock);
	/* increment WP */
	mhi_add_ring_element(mhi_cntrl, tre_ring);
	mhi_add_ring_element(mhi_cntrl, buf_ring);

	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state))) {
		mhi_ring_chan_db(mhi_cntrl, mhi_chan);
	}
		read_unlock_bh(&mhi_chan->lock);

	if (mhi_chan->dir == DMA_FROM_DEVICE && assert_wake)
		mhi_cntrl->wake_put(mhi_cntrl, true);

	read_unlock_bh(&mhi_cntrl->pm_lock);

	return 0;
}

int mhi_gen_tre(struct mhi_controller *mhi_cntrl,
		struct mhi_chan *mhi_chan,
		void *buf,
		void *cb,
		size_t buf_len,
		enum MHI_FLAGS flags)
{
	struct mhi_ring *buf_ring, *tre_ring;
	struct mhi_tre *mhi_tre;
	struct mhi_buf_info *buf_info;
	int eot, eob, chain, bei;
	int ret;

	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;

	buf_info = buf_ring->wp;
	buf_info->v_addr = buf;
	buf_info->cb_buf = cb;
	buf_info->wp = tre_ring->wp;
	buf_info->dir = mhi_chan->dir;
	buf_info->len = buf_len;

	ret = mhi_cntrl->map_single(mhi_cntrl, buf_info);
	if (ret)
		return ret;

	eob = !!(flags & MHI_EOB);
	eot = !!(flags & MHI_EOT);
	chain = !!(flags & MHI_CHAIN);
	bei = !!(mhi_chan->intmod);

	mhi_tre = tre_ring->wp;
	mhi_tre->ptr = MHI_TRE_DATA_PTR(buf_info->p_addr);
	mhi_tre->dword[0] = MHI_TRE_DATA_DWORD0(buf_len);
	mhi_tre->dword[1] = MHI_TRE_DATA_DWORD1(bei, eot, eob, chain);

#ifdef ENABLE_MHI_MON
	if (mhi_cntrl->nreaders) {
		mon_bus_submit(mhi_cntrl, mhi_chan->chan,
			mhi_to_physical(tre_ring, mhi_tre), mhi_tre, buf_info->v_addr, mhi_chan->chan&0x1 ? 0 : buf_info->len);
	}
#endif
	MHI_VERB("chan:%d WP:0x%llx TRE:0x%llx 0x%08x 0x%08x\n", mhi_chan->chan,
		 (u64)mhi_to_physical(tre_ring, mhi_tre), mhi_tre->ptr,
		 mhi_tre->dword[0], mhi_tre->dword[1]);

	/* increment WP */
	mhi_add_ring_element(mhi_cntrl, tre_ring);
	mhi_add_ring_element(mhi_cntrl, buf_ring);

	return 0;
}

int mhi_queue_buf(struct mhi_device *mhi_dev,
		  struct mhi_chan *mhi_chan,
		  void *buf,
		  size_t len,
		  enum MHI_FLAGS mflags)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_ring *tre_ring;
	unsigned long flags;
	bool assert_wake = false;
	int ret;

	/*
	 * this check here only as a guard, it's always
	 * possible mhi can enter error while executing rest of function,
	 * which is not fatal so we do not need to hold pm_lock
	 */
	if (unlikely(MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state))) {
		MHI_VERB("MHI is not in active state, pm_state:%s\n",
			 to_mhi_pm_state_str(mhi_cntrl->pm_state));

		return -EIO;
	}

	tre_ring = &mhi_chan->tre_ring;
	if (mhi_is_ring_full(mhi_cntrl, tre_ring))
		return -ENOMEM;

	ret = mhi_chan->gen_tre(mhi_cntrl, mhi_chan, buf, buf, len, mflags);
	if (unlikely(ret))
		return ret;

	read_lock_irqsave(&mhi_cntrl->pm_lock, flags);

	/* we're in M3 or transitioning to M3 */
	if (MHI_PM_IN_SUSPEND_STATE(mhi_cntrl->pm_state)) {
		mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
		mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
	}

	/*
	 * For UL channels always assert WAKE until work is done,
	 * For DL channels only assert if MHI is in a LPM
	 */
	if (mhi_chan->dir == DMA_TO_DEVICE ||
	    (mhi_chan->dir == DMA_FROM_DEVICE &&
	     mhi_cntrl->pm_state != MHI_PM_M0)) {
		assert_wake = true;
		mhi_cntrl->wake_get(mhi_cntrl, false);
	}

	if (mhi_chan->dir == DMA_TO_DEVICE) {
		if (atomic_inc_return(&mhi_cntrl->pending_pkts) == 1)
			mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
	}

	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state))) {
		unsigned long flags;

		read_lock_irqsave(&mhi_chan->lock, flags);
		mhi_ring_chan_db(mhi_cntrl, mhi_chan);
		read_unlock_irqrestore(&mhi_chan->lock, flags);
	}

	if (mhi_chan->dir == DMA_FROM_DEVICE && assert_wake)
		mhi_cntrl->wake_put(mhi_cntrl, true);

	read_unlock_irqrestore(&mhi_cntrl->pm_lock, flags);

	return 0;
}

static ssize_t ul_chan_id_show(struct device *dev, struct device_attribute *attr, char *buf) {
	struct mhi_device *mhi_dev = to_mhi_device(dev);

	return snprintf(buf, PAGE_SIZE, "%d\n",  mhi_dev->ul_chan_id);
}

static DEVICE_ATTR_RO(ul_chan_id);

static struct attribute *mhi_dev_attrs[] = {
	&dev_attr_ul_chan_id.attr,
	NULL,
};

static struct attribute_group mhi_dev_attr_group = {
	.attrs = mhi_dev_attrs,
};

/* destroy specific device */
int mhi_destroy_device(struct device *dev, void *data)
{
	struct mhi_device *mhi_dev;
	struct mhi_controller *mhi_cntrl;

	if (dev->bus != &mhi_bus_type)
		return 0;

	mhi_dev = to_mhi_device(dev);
	mhi_cntrl = mhi_dev->mhi_cntrl;

	/* only destroying virtual devices thats attached to bus */
	if (mhi_dev->dev_type ==  MHI_CONTROLLER_TYPE)
		return 0;

	MHI_LOG("destroy device for chan:%s\n", mhi_dev->chan_name);

	sysfs_remove_group(&mhi_dev->dev.kobj, &mhi_dev_attr_group);
	/* notify the client and remove the device from mhi bus */
	device_del(dev);
	put_device(dev);

	return 0;
}

void mhi_notify(struct mhi_device *mhi_dev, enum MHI_CB cb_reason)
{
	struct mhi_driver *mhi_drv;

	if (!mhi_dev->dev.driver)
		return;

	mhi_drv = to_mhi_driver(mhi_dev->dev.driver);

	if (mhi_drv->status_cb)
		mhi_drv->status_cb(mhi_dev, cb_reason);
}

static void mhi_assign_of_node(struct mhi_controller *mhi_cntrl,
			       struct mhi_device *mhi_dev)
{
	struct device_node *controller, *node;
	const char *dt_name;
	int ret;

	controller = of_find_node_by_name(mhi_cntrl->of_node, "mhi_devices");
	if (!controller)
		return;

	for_each_available_child_of_node(controller, node) {
		ret = of_property_read_string(node, "mhi,chan", &dt_name);
		if (ret)
			continue;
		if (!strcmp(mhi_dev->chan_name, dt_name)) {
			mhi_dev->dev.of_node = node;
			break;
		}
	}
}

static ssize_t time_show(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	u64 t_host, t_device;
	int ret;

	ret = mhi_get_remote_time_sync(mhi_dev, &t_host, &t_device);
	if (ret) {
		MHI_ERR("Failed to obtain time, ret:%d\n", ret);
		return ret;
	}

	return scnprintf(buf, PAGE_SIZE, "local: %llu remote: %llu (ticks)\n",
			 t_host, t_device);
}
static DEVICE_ATTR_RO(time);

static ssize_t time_us_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	u64 t_host, t_device;
	int ret;

	ret = mhi_get_remote_time_sync(mhi_dev, &t_host, &t_device);
	if (ret) {
		MHI_ERR("Failed to obtain time, ret:%d\n", ret);
		return ret;
	}

	return scnprintf(buf, PAGE_SIZE, "local: %llu remote: %llu (us)\n",
			 TIME_TICKS_TO_US(t_host), TIME_TICKS_TO_US(t_device));
}
static DEVICE_ATTR_RO(time_us);

static struct attribute *mhi_tsync_attrs[] = {
	&dev_attr_time.attr,
	&dev_attr_time_us.attr,
	NULL,
};

static const struct attribute_group mhi_tsync_group = {
	.attrs = mhi_tsync_attrs,
};

void mhi_destroy_timesync(struct mhi_controller *mhi_cntrl)
{
	if (mhi_cntrl->mhi_tsync) {
		sysfs_remove_group(&mhi_cntrl->mhi_dev->dev.kobj,
				   &mhi_tsync_group);
		kfree(mhi_cntrl->mhi_tsync);
		mhi_cntrl->mhi_tsync = NULL;
	}
}

int mhi_create_timesync_sysfs(struct mhi_controller *mhi_cntrl)
{
	return sysfs_create_group(&mhi_cntrl->mhi_dev->dev.kobj,
				  &mhi_tsync_group);
}

static void mhi_create_time_sync_dev(struct mhi_controller *mhi_cntrl)
{
	struct mhi_device *mhi_dev;
	int ret;

	if (!MHI_IN_MISSION_MODE(mhi_cntrl->ee))
		return;

	mhi_dev = mhi_alloc_device(mhi_cntrl);
	if (!mhi_dev)
		return;

	mhi_dev->dev_type = MHI_TIMESYNC_TYPE;
	mhi_dev->chan_name = "TIME_SYNC";
	dev_set_name(&mhi_dev->dev, "%04x_%02u.%02u.%02u_%s", mhi_dev->dev_id,
		     mhi_dev->domain, mhi_dev->bus, mhi_dev->slot,
		     mhi_dev->chan_name);

	/* add if there is a matching DT node */
	mhi_assign_of_node(mhi_cntrl, mhi_dev);

	ret = device_add(&mhi_dev->dev);
	if (ret) {
		MHI_ERR("Failed to register dev for  chan:%s\n",
			mhi_dev->chan_name);
		mhi_dealloc_device(mhi_cntrl, mhi_dev);
		return;
	}

	mhi_cntrl->tsync_dev = mhi_dev;
}

/* bind mhi channels into mhi devices */
void mhi_create_devices(struct mhi_controller *mhi_cntrl)
{
	int i;
	struct mhi_chan *mhi_chan;
	struct mhi_device *mhi_dev;
	int ret;

	/*
	 * we need to create time sync device before creating other
	 * devices, because client may try to capture time during
	 * clint probe.
	 */
	mhi_create_time_sync_dev(mhi_cntrl);

	mhi_chan = mhi_cntrl->mhi_chan;
	for (i = 0; i < mhi_cntrl->max_chan; i++, mhi_chan++) {
		if (!mhi_chan->configured || mhi_chan->mhi_dev ||
		    !(mhi_chan->ee_mask & BIT(mhi_cntrl->ee)))
			continue;
		mhi_dev = mhi_alloc_device(mhi_cntrl);
		if (!mhi_dev)
			return;

		mhi_dev->dev_type = MHI_XFER_TYPE;
		switch (mhi_chan->dir) {
		case DMA_TO_DEVICE:
			mhi_dev->ul_chan = mhi_chan;
			mhi_dev->ul_chan_id = mhi_chan->chan;
			mhi_dev->ul_xfer = mhi_chan->queue_xfer;
			mhi_dev->ul_event_id = mhi_chan->er_index;
			break;
		case DMA_NONE:
		case DMA_BIDIRECTIONAL:
			mhi_dev->ul_chan_id = mhi_chan->chan;
			mhi_dev->ul_event_id = mhi_chan->er_index;
			mhi_dev->dl_chan = mhi_chan;
			mhi_dev->dl_chan_id = mhi_chan->chan;
			mhi_dev->dl_xfer = mhi_chan->queue_xfer;
			mhi_dev->dl_event_id = mhi_chan->er_index;
			break;
		case DMA_FROM_DEVICE:
			/* we use dl_chan for offload channels */
			mhi_dev->dl_chan = mhi_chan;
			mhi_dev->dl_chan_id = mhi_chan->chan;
			mhi_dev->dl_xfer = mhi_chan->queue_xfer;
			mhi_dev->dl_event_id = mhi_chan->er_index;
			break;
		}

		mhi_chan->mhi_dev = mhi_dev;

		/* check next channel if it matches */
		if ((i + 1) < mhi_cntrl->max_chan && mhi_chan[1].configured) {
			if (!strcmp(mhi_chan[1].name, mhi_chan->name)) {
				i++;
				mhi_chan++;
				if (mhi_chan->dir == DMA_TO_DEVICE) {
					mhi_dev->ul_chan = mhi_chan;
					mhi_dev->ul_chan_id = mhi_chan->chan;
					mhi_dev->ul_xfer = mhi_chan->queue_xfer;
					mhi_dev->ul_event_id =
						mhi_chan->er_index;
				} else {
					mhi_dev->dl_chan = mhi_chan;
					mhi_dev->dl_chan_id = mhi_chan->chan;
					mhi_dev->dl_xfer = mhi_chan->queue_xfer;
					mhi_dev->dl_event_id =
						mhi_chan->er_index;
				}
				mhi_chan->mhi_dev = mhi_dev;
			}
		}

		mhi_dev->chan_name = mhi_chan->name;
		dev_set_name(&mhi_dev->dev, "%04x_%02u.%02u.%02u_%s",
			     mhi_dev->dev_id, mhi_dev->domain, mhi_dev->bus,
			     mhi_dev->slot, mhi_dev->chan_name);

		/* add if there is a matching DT node */
		mhi_assign_of_node(mhi_cntrl, mhi_dev);

		/* init wake source */
		if (mhi_dev->dl_chan && mhi_dev->dl_chan->wake_capable)
			device_init_wakeup(&mhi_dev->dev, true);

		ret = device_add(&mhi_dev->dev);
		if (ret) {
			MHI_ERR("Failed to register dev for  chan:%s\n",
				mhi_dev->chan_name);
			mhi_dealloc_device(mhi_cntrl, mhi_dev);
		}
		ret = sysfs_create_group(&mhi_dev->dev.kobj, &mhi_dev_attr_group);
	}
}

static void mhi_dump_tre(struct mhi_controller *mhi_cntrl, struct mhi_tre *_ev) {
	union mhi_dev_ring_element_type *ev = (union mhi_dev_ring_element_type *)_ev;

	switch (ev->generic.type) {
		case MHI_DEV_RING_EL_INVALID: {
			MHI_ERR("carl_ev cmd_invalid, ptr=%llx, %x, %x\n", _ev->ptr, _ev->dword[0], _ev->dword[1]);
		}
		break;
		case MHI_DEV_RING_EL_NOOP: {
			MHI_LOG("carl_ev cmd_no_op chan=%u\n", ev->cmd_no_op.chid);
		}
		break;
		case MHI_DEV_RING_EL_TRANSFER: {
			MHI_LOG("carl_ev cmd_transfer data=%llx, len=%u, chan=%u\n",
				ev->cmd_transfer.data_buf_ptr, ev->cmd_transfer.len, ev->cmd_transfer.chain);
		}
		break;
		case MHI_DEV_RING_EL_RESET: {
			MHI_LOG("carl_ev cmd_reset chan=%u\n", ev->cmd_reset.chid);
		}
		break;
		case MHI_DEV_RING_EL_STOP: {
			MHI_LOG("carl_ev cmd_stop chan=%u\n", ev->cmd_stop.chid);
		}
		break;
		case MHI_DEV_RING_EL_START: {
			MHI_LOG("carl_ev cmd_start chan=%u\n", ev->cmd_start.chid);
		}
		break;
		case MHI_DEV_RING_EL_MHI_STATE_CHG: {
			MHI_LOG("carl_ev evt_state_change mhistate=%u\n", ev->evt_state_change.mhistate);
		}
		break;
		case MHI_DEV_RING_EL_CMD_COMPLETION_EVT:{
			MHI_LOG("carl_ev evt_cmd_comp code=%u, type=%u\n", ev->evt_cmd_comp.code, ev->evt_cmd_comp.type);
		}
		break;
		case MHI_DEV_RING_EL_TRANSFER_COMPLETION_EVENT:{
			MHI_VERB("carl_ev evt_tr_comp ptr=%llx, len=%u, code=%u, chan=%u\n",
				ev->evt_tr_comp.ptr, ev->evt_tr_comp.len, ev->evt_tr_comp.code,  ev->evt_tr_comp.chid);
		}
		break;
		case MHI_DEV_RING_EL_EE_STATE_CHANGE_NOTIFY:{
			MHI_LOG("carl_ev evt_ee_state execenv=%u\n", ev->evt_ee_state.execenv);
		}
		break;
		case MHI_DEV_RING_EL_UNDEF:
		default: {
			MHI_ERR("carl_ev el_undef type=%d\n", ev->generic.type);
		};
		break;
	}
}

static int parse_xfer_event(struct mhi_controller *mhi_cntrl,
			    struct mhi_tre *event,
			    struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring, *tre_ring;
	u32 ev_code;
	struct mhi_result result;
	unsigned long flags = 0;

	ev_code = MHI_TRE_GET_EV_CODE(event);
	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;

	result.transaction_status = (ev_code == MHI_EV_CC_OVERFLOW) ?
		-EOVERFLOW : 0;

	/*
	 * if it's a DB Event then we need to grab the lock
	 * with preemption disable and as a write because we
	 * have to update db register and another thread could
	 * be doing same.
	 */
	if (ev_code >= MHI_EV_CC_OOB)
		write_lock_irqsave(&mhi_chan->lock, flags);
	else
		read_lock_bh(&mhi_chan->lock);

	if (mhi_chan->ch_state != MHI_CH_STATE_ENABLED)
		goto end_process_tx_event;

	switch (ev_code) {
	case MHI_EV_CC_OVERFLOW:
	case MHI_EV_CC_EOB:
	case MHI_EV_CC_EOT:
	{
		dma_addr_t ptr = MHI_TRE_GET_EV_PTR(event);
		struct mhi_tre *local_rp, *ev_tre;
		void *dev_rp;
		struct mhi_buf_info *buf_info;
		u16 xfer_len;

		/* Get the TRB this event points to */
		ev_tre = mhi_to_virtual(tre_ring, ptr);

		/* device rp after servicing the TREs */
		dev_rp = ev_tre + 1;
		if (dev_rp >= (tre_ring->base + tre_ring->len))
			dev_rp = tre_ring->base;

		mhi_chan->used_events[ev_code]++;

		result.dir = mhi_chan->dir;

		/* local rp */
		local_rp = tre_ring->rp;
		while (local_rp != dev_rp) {
			buf_info = buf_ring->rp;
			/* Always get the get len from the event */
			xfer_len = MHI_TRE_GET_EV_LEN(event);

			/* unmap if it's not premapped by client */
			if (likely(!buf_info->pre_mapped))
				mhi_cntrl->unmap_single(mhi_cntrl, buf_info);

			result.buf_addr = buf_info->cb_buf;
			result.bytes_xferd = xfer_len;
#ifdef ENABLE_MHI_MON
			if (mhi_cntrl->nreaders) {
				void *buf = NULL;
				size_t len = 0;

				if (mhi_chan->queue_xfer == mhi_queue_skb) {
					struct sk_buff *skb = result.buf_addr;
					buf = skb->data;
					len = result.bytes_xferd;
				}
				else if (CHAN_INBOUND(mhi_chan->chan)) {
					buf = result.buf_addr;
					len = result.bytes_xferd;
				}
				mon_bus_receive(mhi_cntrl, mhi_chan->chan,
					mhi_to_physical(tre_ring, local_rp), local_rp, buf, len);
			}
#endif
			mhi_del_ring_element(mhi_cntrl, buf_ring);
			mhi_del_ring_element(mhi_cntrl, tre_ring);
			local_rp = tre_ring->rp;

			/* notify client */
			mhi_chan->xfer_cb(mhi_chan->mhi_dev, &result);

			if (mhi_chan->dir == DMA_TO_DEVICE) {
				if (atomic_dec_return(&mhi_cntrl->pending_pkts) == 0) 
					mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
			}

			if (mhi_chan->dir == DMA_TO_DEVICE) {
				read_lock_bh(&mhi_cntrl->pm_lock);
				mhi_cntrl->wake_put(mhi_cntrl, false);
				read_unlock_bh(&mhi_cntrl->pm_lock);
			}

			/*
			 * recycle the buffer if buffer is pre-allocated,
			 * if there is error, not much we can do apart from
			 * dropping the packet
			 */
			if (mhi_chan->pre_alloc) {
				if (mhi_queue_buf(mhi_chan->mhi_dev, mhi_chan,
						  buf_info->cb_buf,
						  buf_info->len, MHI_EOT)) {
					MHI_ERR(
						"Error recycling buffer for chan:%d\n",
						mhi_chan->chan);
					kfree(buf_info->cb_buf);
				}
			}
		}
		break;
	} /* CC_EOT */
	case MHI_EV_CC_OOB:
	case MHI_EV_CC_DB_MODE:
	{
		unsigned long flags;

		mhi_chan->used_events[ev_code]++;
		MHI_VERB("DB_MODE/OOB Detected chan %d.\n", mhi_chan->chan);
		mhi_chan->db_cfg.db_mode = true;
		read_lock_irqsave(&mhi_cntrl->pm_lock, flags);
#ifdef DEBUG_CHAN100_DB
		if (mhi_chan->chan == 100) {
			chan100_t[atomic_inc_return(&chan100_seq)&(CHAN100_SIZE-1)] = (((unsigned long)tre_ring->rp)&0xffff) | (0xf0000);
			chan100_t[atomic_inc_return(&chan100_seq)&(CHAN100_SIZE-1)] = (((unsigned long)tre_ring->wp)&0xffff) | (mhi_chan->db_cfg.db_mode<<31) | (1<<30);
		}
#endif
		if (tre_ring->wp != tre_ring->rp &&
		    MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)) {
			mhi_ring_chan_db(mhi_cntrl, mhi_chan);
		}
		read_unlock_irqrestore(&mhi_cntrl->pm_lock, flags);
		break;
	}
	case MHI_EV_CC_BAD_TRE:
		MHI_ASSERT(1, "Received BAD TRE event for ring");
		break;
	default:
		MHI_CRITICAL("Unknown TX completion.\n");

		break;
	} /* switch(MHI_EV_READ_CODE(EV_TRB_CODE,event)) */

end_process_tx_event:
	if (ev_code >= MHI_EV_CC_OOB)
		write_unlock_irqrestore(&mhi_chan->lock, flags);
	else
		read_unlock_bh(&mhi_chan->lock);

	return 0;
}

static int parse_rsc_event(struct mhi_controller *mhi_cntrl,
			   struct mhi_tre *event,
			   struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring, *tre_ring;
	struct mhi_buf_info *buf_info;
	struct mhi_result result;
	int ev_code;
	u32 cookie; /* offset to local descriptor */
	u16 xfer_len;

	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;

	ev_code = MHI_TRE_GET_EV_CODE(event);
	cookie = MHI_TRE_GET_EV_COOKIE(event);
	xfer_len = MHI_TRE_GET_EV_LEN(event);

	/* received out of bound cookie */
	MHI_ASSERT(cookie >= buf_ring->len, "Invalid Cookie\n");

	buf_info = buf_ring->base + cookie;

	result.transaction_status = (ev_code == MHI_EV_CC_OVERFLOW) ?
		-EOVERFLOW : 0;
	result.bytes_xferd = xfer_len;
	result.buf_addr = buf_info->cb_buf;
	result.dir = mhi_chan->dir;

	read_lock_bh(&mhi_chan->lock);

	if (mhi_chan->ch_state != MHI_CH_STATE_ENABLED)
		goto end_process_rsc_event;

	MHI_ASSERT(!buf_info->used, "TRE already Freed\n");

	/* notify the client */
	mhi_chan->xfer_cb(mhi_chan->mhi_dev, &result);

	/*
	 * Note: We're arbitrarily incrementing RP even though, completion
	 * packet we processed might not be the same one, reason we can do this
	 * is because device guaranteed to cache descriptors in order it
	 * receive, so even though completion event is different we can re-use
	 * all descriptors in between.
	 * Example:
	 * Transfer Ring has descriptors: A, B, C, D
	 * Last descriptor host queue is D (WP) and first descriptor
	 * host queue is A (RP).
	 * The completion event we just serviced is descriptor C.
	 * Then we can safely queue descriptors to replace A, B, and C
	 * even though host did not receive any completions.
	 */
	mhi_del_ring_element(mhi_cntrl, tre_ring);
	buf_info->used = false;

end_process_rsc_event:
	read_unlock_bh(&mhi_chan->lock);

	return 0;
}

static void mhi_process_cmd_completion(struct mhi_controller *mhi_cntrl,
				       struct mhi_tre *tre)
{
	dma_addr_t ptr = MHI_TRE_GET_EV_PTR(tre);
	struct mhi_cmd *cmd_ring = &mhi_cntrl->mhi_cmd[PRIMARY_CMD_RING];
	struct mhi_ring *mhi_ring = &cmd_ring->ring;
	struct mhi_tre *cmd_pkt;
	struct mhi_chan *mhi_chan;
	struct mhi_timesync *mhi_tsync;
	enum mhi_cmd_type type;
	u32 chan;

	cmd_pkt = mhi_to_virtual(mhi_ring, ptr);

	/* out of order completion received */
	MHI_ASSERT(cmd_pkt != mhi_ring->rp, "Out of order cmd completion");

	type = MHI_TRE_GET_CMD_TYPE(cmd_pkt);

	if (type == MHI_CMD_TYPE_TSYNC) {
		mhi_tsync = mhi_cntrl->mhi_tsync;
		mhi_tsync->ccs = MHI_TRE_GET_EV_CODE(tre);
		complete(&mhi_tsync->completion);
	} else {
		chan = MHI_TRE_GET_CMD_CHID(cmd_pkt);
		mhi_chan = &mhi_cntrl->mhi_chan[chan];
		write_lock_bh(&mhi_chan->lock);
		mhi_chan->ccs = MHI_TRE_GET_EV_CODE(tre);
		complete(&mhi_chan->completion);
		write_unlock_bh(&mhi_chan->lock);
	}

	mhi_del_ring_element(mhi_cntrl, mhi_ring);
}

int mhi_process_ctrl_ev_ring(struct mhi_controller *mhi_cntrl,
			     struct mhi_event *mhi_event,
			     u32 event_quota)
{
	struct mhi_tre *dev_rp, *local_rp;
	struct mhi_ring *ev_ring = &mhi_event->ring;
	struct mhi_event_ctxt *er_ctxt =
		&mhi_cntrl->mhi_ctxt->er_ctxt[mhi_event->er_index];
	int count = 0;

	/*
	 * this is a quick check to avoid unnecessary event processing
	 * in case we already in error state, but it's still possible
	 * to transition to error state while processing events
	 */
	if (unlikely(MHI_EVENT_ACCESS_INVALID(mhi_cntrl->pm_state))) {
		MHI_ERR("No EV access, PM_STATE:%s\n",
			to_mhi_pm_state_str(mhi_cntrl->pm_state));
		return -EIO;
	}

	dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
	local_rp = ev_ring->rp;

	while (dev_rp != local_rp) {
		enum MHI_PKT_TYPE type = MHI_TRE_GET_EV_TYPE(local_rp);

//#define QL_READ_EVENT_WA //from Quectel Windows driver
#ifdef QL_READ_EVENT_WA
		if (mhi_event->er_index == 0) {
			if (local_rp->ptr == 0 && local_rp->dword[0] == 0 && local_rp->dword[1] == 0) {
				// event content no sync to memory, just break and wait next event.
				MHI_ERR("Zero Event!\n");
				break;
			}
		}
#endif

		mhi_dump_tre(mhi_cntrl, local_rp);
		MHI_VERB("Processing Event:0x%llx 0x%08x 0x%08x\n",
			local_rp->ptr, local_rp->dword[0], local_rp->dword[1]);
#ifdef ENABLE_MHI_MON
		if (mhi_cntrl->nreaders) {
			mon_bus_complete(mhi_cntrl, mhi_event->er_index, mhi_to_physical(ev_ring, local_rp), local_rp);
		}
#endif

		switch (type) {
		case MHI_PKT_TYPE_STATE_CHANGE_EVENT:
		{
			enum mhi_dev_state new_state;

			new_state = MHI_TRE_GET_EV_STATE(local_rp);

			MHI_LOG("MHI state change event to state:%s\n",
				TO_MHI_STATE_STR(new_state));

			switch (new_state) {
			case MHI_STATE_M0:
				mhi_pm_m0_transition(mhi_cntrl);
				break;
			case MHI_STATE_M1:
				mhi_pm_m1_transition(mhi_cntrl);
				break;
			case MHI_STATE_M3:
				mhi_pm_m3_transition(mhi_cntrl);
				break;
			case MHI_STATE_SYS_ERR:
			{
				enum MHI_PM_STATE new_state;

				MHI_ERR("MHI system error detected\n");
				write_lock_irq(&mhi_cntrl->pm_lock);
				new_state = mhi_tryset_pm_state(mhi_cntrl,
							MHI_PM_SYS_ERR_DETECT);
				write_unlock_irq(&mhi_cntrl->pm_lock);
				if (new_state == MHI_PM_SYS_ERR_DETECT)
					schedule_work(
						&mhi_cntrl->syserr_worker);
				break;
			}
			default:
				MHI_ERR("Unsupported STE:%s\n",
					TO_MHI_STATE_STR(new_state));
			}

			break;
		}
		case MHI_PKT_TYPE_CMD_COMPLETION_EVENT:
			mhi_process_cmd_completion(mhi_cntrl, local_rp);
			break;
		case MHI_PKT_TYPE_EE_EVENT:
		{
			enum MHI_ST_TRANSITION st = MHI_ST_TRANSITION_MAX;
			enum mhi_ee event = MHI_TRE_GET_EV_EXECENV(local_rp);

			MHI_LOG("MHI EE received event:%s\n",
				TO_MHI_EXEC_STR(event));
			switch (event) {
			case MHI_EE_SBL:
				st = MHI_ST_TRANSITION_SBL;
				break;
			case MHI_EE_FP:
				st = MHI_ST_TRANSITION_FP;
				break;
			case MHI_EE_WFW:
			case MHI_EE_AMSS:
				st = MHI_ST_TRANSITION_MISSION_MODE;
				break;
			case MHI_EE_RDDM:
				mhi_cntrl->status_cb(mhi_cntrl,
						     mhi_cntrl->priv_data,
						     MHI_CB_EE_RDDM);
				write_lock_irq(&mhi_cntrl->pm_lock);
				mhi_cntrl->ee = event;
				write_unlock_irq(&mhi_cntrl->pm_lock);
				wake_up_all(&mhi_cntrl->state_event);
				break;
			default:
				MHI_ERR("Unhandled EE event:%s\n",
					TO_MHI_EXEC_STR(event));
			}
			if (st != MHI_ST_TRANSITION_MAX)
				mhi_queue_state_transition(mhi_cntrl, st);
			break;
		}
#if 1 //Add by Quectel
		case MHI_PKT_TYPE_TX_EVENT:
		case MHI_PKT_TYPE_RSC_TX_EVENT:
		{
			u32 chan = MHI_TRE_GET_EV_CHID(local_rp);
			struct mhi_chan *mhi_chan = &mhi_cntrl->mhi_chan[chan];

			if (likely(type == MHI_PKT_TYPE_TX_EVENT)) {
				parse_xfer_event(mhi_cntrl, local_rp, mhi_chan);
			} else if (type == MHI_PKT_TYPE_RSC_TX_EVENT) {
				parse_rsc_event(mhi_cntrl, local_rp, mhi_chan);
			}
			break;
		}
#endif
		default:
			MHI_ASSERT(1, "Unsupported ev type");
			break;
		}

#ifdef QL_READ_EVENT_WA
		if (mhi_event->er_index == 0) {
			local_rp->ptr = 0;
			local_rp->dword[0] = local_rp->dword[1] = 0;
		}
#endif

		mhi_recycle_ev_ring_element(mhi_cntrl, ev_ring);
		local_rp = ev_ring->rp;
		dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
		count++;
	}

	if (count > mhi_event->used_elements) {
		mhi_event->used_elements = count;
	}

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)))
		mhi_ring_er_db(mhi_event);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	MHI_VERB("exit er_index:%u\n", mhi_event->er_index);

	return count;
}

int mhi_process_data_event_ring(struct mhi_controller *mhi_cntrl,
				struct mhi_event *mhi_event,
				u32 event_quota)
{
	struct mhi_tre *dev_rp, *local_rp;
	struct mhi_ring *ev_ring = &mhi_event->ring;
	struct mhi_event_ctxt *er_ctxt =
		&mhi_cntrl->mhi_ctxt->er_ctxt[mhi_event->er_index];
	int count = 0;
	u32 chan;
	struct mhi_chan *mhi_chan = NULL;
	u32 chan_count = 0;
	void *chan_local_rp = NULL;

	if (unlikely(MHI_EVENT_ACCESS_INVALID(mhi_cntrl->pm_state))) {
		MHI_ERR("No EV access, PM_STATE:%s\n",
			to_mhi_pm_state_str(mhi_cntrl->pm_state));
		return -EIO;
	}

	dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
	local_rp = ev_ring->rp;

	while (dev_rp != local_rp && event_quota > 0) {
		enum MHI_PKT_TYPE type = MHI_TRE_GET_EV_TYPE(local_rp);

		mhi_dump_tre(mhi_cntrl, local_rp);
		MHI_VERB("Processing Event:0x%llx 0x%08x 0x%08x\n",
			local_rp->ptr, local_rp->dword[0], local_rp->dword[1]);

		chan = MHI_TRE_GET_EV_CHID(local_rp);
		mhi_chan = &mhi_cntrl->mhi_chan[chan];
		chan_local_rp = mhi_chan->tre_ring.rp;

#ifdef ENABLE_MHI_MON
		if (mhi_cntrl->nreaders) {
			mon_bus_complete(mhi_cntrl, mhi_event->er_index, mhi_to_physical(ev_ring, local_rp), local_rp);
		}
#endif
		if (likely(type == MHI_PKT_TYPE_TX_EVENT)) {
			parse_xfer_event(mhi_cntrl, local_rp, mhi_chan);
			event_quota--;
		} else if (type == MHI_PKT_TYPE_RSC_TX_EVENT) {
			parse_rsc_event(mhi_cntrl, local_rp, mhi_chan);
			event_quota--;
		}

		chan_count += get_used_ring_elements(chan_local_rp, mhi_chan->tre_ring.rp, mhi_chan->tre_ring.elements);
		mhi_recycle_ev_ring_element(mhi_cntrl, ev_ring);
		local_rp = ev_ring->rp;
		if (local_rp == dev_rp || event_quota == 0) {
			if (chan_count > mhi_chan->used_elements)
				mhi_chan->used_elements = chan_count;
			chan_count = 0;
			dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
		}
		count++;
	}

	if (count > mhi_event->used_elements) {
		mhi_event->used_elements = count;
	}

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)))
		mhi_ring_er_db(mhi_event);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	MHI_VERB("exit er_index:%u\n", mhi_event->er_index);

	return count;
}

int mhi_process_tsync_event_ring(struct mhi_controller *mhi_cntrl,
				 struct mhi_event *mhi_event,
				 u32 event_quota)
{
	struct mhi_tre *dev_rp, *local_rp;
	struct mhi_ring *ev_ring = &mhi_event->ring;
	struct mhi_event_ctxt *er_ctxt =
		&mhi_cntrl->mhi_ctxt->er_ctxt[mhi_event->er_index];
	struct mhi_timesync *mhi_tsync = mhi_cntrl->mhi_tsync;
	int count = 0;
	u32 sequence;
	u64 remote_time;

	if (unlikely(MHI_EVENT_ACCESS_INVALID(mhi_cntrl->pm_state))) {
		MHI_ERR("No EV access, PM_STATE:%s\n",
			to_mhi_pm_state_str(mhi_cntrl->pm_state));
		read_unlock_bh(&mhi_cntrl->pm_lock);
		return -EIO;
	}

	dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
	local_rp = ev_ring->rp;

	while (dev_rp != local_rp) {
		enum MHI_PKT_TYPE type = MHI_TRE_GET_EV_TYPE(local_rp);
		struct tsync_node *tsync_node;

		MHI_VERB("Processing Event:0x%llx 0x%08x 0x%08x\n",
			local_rp->ptr, local_rp->dword[0], local_rp->dword[1]);

		MHI_ASSERT(type != MHI_PKT_TYPE_TSYNC_EVENT, "!TSYNC event");

		sequence = MHI_TRE_GET_EV_SEQ(local_rp);
		remote_time = MHI_TRE_GET_EV_TIME(local_rp);

		do {
			spin_lock_irq(&mhi_tsync->lock);
			tsync_node = list_first_entry_or_null(&mhi_tsync->head,
						      struct tsync_node, node);
			MHI_ASSERT(!tsync_node, "Unexpected Event");

			if (unlikely(!tsync_node))
				break;

			list_del(&tsync_node->node);
			spin_unlock_irq(&mhi_tsync->lock);

			/*
			 * device may not able to process all time sync commands
			 * host issue and only process last command it receive
			 */
			if (tsync_node->sequence == sequence) {
				tsync_node->cb_func(tsync_node->mhi_dev,
						    sequence,
						    tsync_node->local_time,
						    remote_time);
				kfree(tsync_node);
			} else {
				kfree(tsync_node);
			}
		} while (true);

		mhi_recycle_ev_ring_element(mhi_cntrl, ev_ring);
		local_rp = ev_ring->rp;
		dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
		count++;
	}

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)))
		mhi_ring_er_db(mhi_event);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	MHI_VERB("exit er_index:%u\n", mhi_event->er_index);

	return count;
}

void mhi_ev_task(unsigned long data)
{
	struct mhi_event *mhi_event = (struct mhi_event *)data;
	struct mhi_controller *mhi_cntrl = mhi_event->mhi_cntrl;

	MHI_VERB("Enter for ev_index:%d\n", mhi_event->er_index);

	mhi_cntrl->runtime_mark_last_busy(mhi_cntrl, mhi_cntrl->priv_data);
	/* process all pending events */
	spin_lock_bh(&mhi_event->lock);
	mhi_event->process_event(mhi_cntrl, mhi_event, U32_MAX);
	spin_unlock_bh(&mhi_event->lock);
}

void mhi_ctrl_ev_task(unsigned long data)
{
	struct mhi_event *mhi_event = (struct mhi_event *)data;
	struct mhi_controller *mhi_cntrl = mhi_event->mhi_cntrl;
	enum mhi_dev_state state;
	enum MHI_PM_STATE pm_state = 0;
	int ret;

	MHI_VERB("Enter for ev_index:%d\n", mhi_event->er_index);

	/*
	 * we can check pm_state w/o a lock here because there is no way
	 * pm_state can change from reg access valid to no access while this
	 * therad being executed.
	 */
	if (!MHI_REG_ACCESS_VALID(mhi_cntrl->pm_state))
		return;

	mhi_cntrl->runtime_mark_last_busy(mhi_cntrl, mhi_cntrl->priv_data);
	/* process ctrl events events */
	ret = mhi_event->process_event(mhi_cntrl, mhi_event, U32_MAX);

	/*
	 * we received a MSI but no events to process maybe device went to
	 * SYS_ERR state, check the state
	 */
	if (!ret) {
		write_lock_irq(&mhi_cntrl->pm_lock);
		state = mhi_get_mhi_state(mhi_cntrl);
		if (state == MHI_STATE_SYS_ERR) {
			MHI_ERR("MHI system error detected\n");
			pm_state = mhi_tryset_pm_state(mhi_cntrl,
						       MHI_PM_SYS_ERR_DETECT);
		}
		write_unlock_irq(&mhi_cntrl->pm_lock);
		if (pm_state == MHI_PM_SYS_ERR_DETECT)
			schedule_work(&mhi_cntrl->syserr_worker);
	}
}

irqreturn_t mhi_msi_handlr(int irq_number, void *dev)
{
	struct mhi_event *mhi_event = dev;
	struct mhi_controller *mhi_cntrl = mhi_event->mhi_cntrl;
	struct mhi_event_ctxt *er_ctxt =
		&mhi_cntrl->mhi_ctxt->er_ctxt[mhi_event->er_index];
	struct mhi_ring *ev_ring = &mhi_event->ring;
	void *dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);

	/* confirm ER has pending events to process before scheduling work */
	if (ev_ring->rp == dev_rp)
		return IRQ_HANDLED;

	/* client managed event ring, notify pending data */
	if (mhi_event->cl_manage) {
		struct mhi_chan *mhi_chan = mhi_event->mhi_chan;
		struct mhi_device *mhi_dev = mhi_chan->mhi_dev;

		if (mhi_dev)
			mhi_dev->status_cb(mhi_dev, MHI_CB_PENDING_DATA);
	} else
		tasklet_schedule(&mhi_event->task);

	return IRQ_HANDLED;
}

/* this is the threaded fn */
irqreturn_t mhi_intvec_threaded_handlr(int irq_number, void *dev)
{
	struct mhi_controller *mhi_cntrl = dev;
	enum mhi_dev_state state = MHI_STATE_MAX;
	enum MHI_PM_STATE pm_state = 0;
	enum mhi_ee ee = MHI_EE_MAX;
	unsigned long flags;

	MHI_VERB("Enter\n");

	write_lock_irqsave(&mhi_cntrl->pm_lock, flags);
	if (MHI_REG_ACCESS_VALID(mhi_cntrl->pm_state)) {
		state = mhi_get_mhi_state(mhi_cntrl);
		ee = mhi_get_exec_env(mhi_cntrl);
	 	if (mhi_cntrl->msi_allocated >= 5 ||(mhi_cntrl->msi_allocated == 1 && (mhi_cntrl->dev_state != state || mhi_cntrl->ee != ee)))
			MHI_LOG("device ee:%s dev_state:%s, pm_state:%s\n", TO_MHI_EXEC_STR(ee),
				TO_MHI_STATE_STR(state), to_mhi_pm_state_str(mhi_cntrl->pm_state));
	}

	if (state == MHI_STATE_SYS_ERR) {
		MHI_ERR("MHI system error detected\n");
	if (mhi_cntrl->pm_state != MHI_PM_SYS_ERR_DETECT)
		pm_state = mhi_tryset_pm_state(mhi_cntrl,
					       MHI_PM_SYS_ERR_DETECT);
	}
	write_unlock_irqrestore(&mhi_cntrl->pm_lock, flags);
	if (pm_state == MHI_PM_SYS_ERR_DETECT) {
		wake_up_all(&mhi_cntrl->state_event);

		if (mhi_cntrl->ee != ee) {
			MHI_LOG("device ee:%s -> %s\n", TO_MHI_EXEC_STR(mhi_cntrl->ee), TO_MHI_EXEC_STR(ee));
			schedule_work(&mhi_cntrl->syserr_worker);
		}
		/* for fatal errors, we let controller decide next step */
		else if (MHI_IN_PBL(ee))
			mhi_cntrl->status_cb(mhi_cntrl, mhi_cntrl->priv_data,
					     MHI_CB_FATAL_ERROR);
		else
			schedule_work(&mhi_cntrl->syserr_worker);
	}
	if (mhi_cntrl->msi_allocated >= 5||(mhi_cntrl->msi_allocated == 1 && (mhi_cntrl->dev_state != state || mhi_cntrl->ee != ee)))
		MHI_LOG("device ee:%s dev_state:%s, %s\n", TO_MHI_EXEC_STR(ee),
				TO_MHI_STATE_STR(state), TO_MHI_EXEC_STR(mhi_cntrl->ee));

	if (pm_state == MHI_PM_POR) {
		wake_up_all(&mhi_cntrl->state_event);
	}

	MHI_VERB("Exit\n");

	return IRQ_HANDLED;
}

irqreturn_t mhi_intvec_handlr(int irq_number, void *dev)
{

	struct mhi_controller *mhi_cntrl = dev;

	/* wake up any events waiting for state change */
	MHI_VERB("Enter\n");
	wake_up_all(&mhi_cntrl->state_event);
	MHI_VERB("Exit\n");

	return IRQ_WAKE_THREAD;
}

irqreturn_t mhi_one_msi_handlr(int irq_number, void *dev)
{
	struct mhi_controller *mhi_cntrl = dev;
	struct mhi_event_ctxt *er_ctxt = mhi_cntrl->mhi_ctxt->er_ctxt;
	struct mhi_event *mhi_event = mhi_cntrl->mhi_event;
	u32 i;
	u32 handle = 0;

	for (i = 0; i < NUM_MHI_EVT_RINGS; i++, er_ctxt++, mhi_event++) {
		struct mhi_ring *ev_ring = &mhi_event->ring;
		void *dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);
		if (ev_ring->rp != dev_rp) {
			handle++;
			mhi_msi_handlr(irq_number, mhi_event);
		}
	}

	 if (handle ==0) {
		mhi_intvec_threaded_handlr(irq_number, dev);
	}

	return IRQ_HANDLED;
}

int mhi_send_cmd(struct mhi_controller *mhi_cntrl,
		 struct mhi_chan *mhi_chan,
		 enum MHI_CMD cmd)
{
	struct mhi_tre *cmd_tre = NULL;
	struct mhi_cmd *mhi_cmd = &mhi_cntrl->mhi_cmd[PRIMARY_CMD_RING];
	struct mhi_ring *ring = &mhi_cmd->ring;
	int chan = 0;

	MHI_VERB("Entered, MHI pm_state:%s dev_state:%s ee:%s\n",
		 to_mhi_pm_state_str(mhi_cntrl->pm_state),
		 TO_MHI_STATE_STR(mhi_cntrl->dev_state),
		 TO_MHI_EXEC_STR(mhi_cntrl->ee));

	if (mhi_chan)
		chan = mhi_chan->chan;

	spin_lock_bh(&mhi_cmd->lock);
	if (!get_nr_avail_ring_elements(mhi_cntrl, ring)) {
		spin_unlock_bh(&mhi_cmd->lock);
		return -ENOMEM;
	}

	/* prepare the cmd tre */
	cmd_tre = ring->wp;
	switch (cmd) {
	case MHI_CMD_RESET_CHAN:
		cmd_tre->ptr = MHI_TRE_CMD_RESET_PTR;
		cmd_tre->dword[0] = MHI_TRE_CMD_RESET_DWORD0;
		cmd_tre->dword[1] = MHI_TRE_CMD_RESET_DWORD1(chan);
		break;
	case MHI_CMD_START_CHAN:
		cmd_tre->ptr = MHI_TRE_CMD_START_PTR;
		cmd_tre->dword[0] = MHI_TRE_CMD_START_DWORD0;
		cmd_tre->dword[1] = MHI_TRE_CMD_START_DWORD1(chan);
		break;
	case MHI_CMD_TIMSYNC_CFG:
		cmd_tre->ptr = MHI_TRE_CMD_TSYNC_CFG_PTR;
		cmd_tre->dword[0] = MHI_TRE_CMD_TSYNC_CFG_DWORD0;
		cmd_tre->dword[1] = MHI_TRE_CMD_TSYNC_CFG_DWORD1
			(mhi_cntrl->mhi_tsync->er_index);
		break;
	}

#ifdef ENABLE_MHI_MON
	if (mhi_cntrl->nreaders) {
		mon_bus_submit(mhi_cntrl, 128, mhi_to_physical(ring, cmd_tre), cmd_tre, NULL, 0);
	}
#endif	
	MHI_VERB("WP:0x%llx TRE: 0x%llx 0x%08x 0x%08x\n",
		 (u64)mhi_to_physical(ring, cmd_tre), cmd_tre->ptr,
		 cmd_tre->dword[0], cmd_tre->dword[1]);

	/* queue to hardware */
	mhi_add_ring_element(mhi_cntrl, ring);
	read_lock_bh(&mhi_cntrl->pm_lock);
	if (likely(MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)))
		mhi_ring_cmd_db(mhi_cntrl, mhi_cmd);
	read_unlock_bh(&mhi_cntrl->pm_lock);
	spin_unlock_bh(&mhi_cmd->lock);

	return 0;
}

static int __mhi_prepare_channel(struct mhi_controller *mhi_cntrl,
				 struct mhi_chan *mhi_chan)
{
	int ret = 0;

	MHI_LOG("Entered: preparing channel:%d\n", mhi_chan->chan);

	if (!(BIT(mhi_cntrl->ee) & mhi_chan->ee_mask)) {
		MHI_ERR("Current EE:%s Required EE Mask:0x%x for chan:%s\n",
			TO_MHI_EXEC_STR(mhi_cntrl->ee), mhi_chan->ee_mask,
			mhi_chan->name);
		return -ENOTCONN;
	}

	mutex_lock(&mhi_chan->mutex);

	/* if channel is not disable state do not allow to start */
	if (mhi_chan->ch_state != MHI_CH_STATE_DISABLED) {
		ret = -EIO;
		MHI_LOG("channel:%d is not in disabled state, ch_state%d\n",
			mhi_chan->chan, mhi_chan->ch_state);
		goto error_init_chan;
	}

	/* client manages channel context for offload channels */
	if (!mhi_chan->offload_ch) {
		ret = mhi_init_chan_ctxt(mhi_cntrl, mhi_chan);
		if (ret) {
			MHI_ERR("Error with init chan\n");
			goto error_init_chan;
		}
	}

	reinit_completion(&mhi_chan->completion);
	read_lock_bh(&mhi_cntrl->pm_lock);
	if (MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state)) {
		MHI_ERR("MHI host is not in active state\n");
		read_unlock_bh(&mhi_cntrl->pm_lock);
		ret = -EIO;
		goto error_pm_state;
	}

	mhi_cntrl->wake_get(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);

	ret = mhi_send_cmd(mhi_cntrl, mhi_chan, MHI_CMD_START_CHAN);
	if (ret) {
		MHI_ERR("Failed to send start chan cmd\n");
		goto error_send_cmd;
	}

	ret = wait_for_completion_timeout(&mhi_chan->completion,
				msecs_to_jiffies(mhi_cntrl->timeout_ms));
	if (!ret || mhi_chan->ccs != MHI_EV_CC_SUCCESS) {
		MHI_ERR("Failed to receive cmd completion for chan:%d\n",
			mhi_chan->chan);
		ret = -EIO;
		goto error_send_cmd;
	}

	mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);

	write_lock_irq(&mhi_chan->lock);
	mhi_chan->ch_state = MHI_CH_STATE_ENABLED;
	write_unlock_irq(&mhi_chan->lock);

	/* pre allocate buffer for xfer ring */
	if (mhi_chan->pre_alloc) {
		int nr_el = get_nr_avail_ring_elements(mhi_cntrl,
						       &mhi_chan->tre_ring);
		size_t len = mhi_cntrl->buffer_len;

		while (nr_el--) {
			void *buf;

			buf = kmalloc(len, GFP_KERNEL);
			if (!buf) {
				ret = -ENOMEM;
				goto error_pre_alloc;
			}

			/* prepare transfer descriptors */
			ret = mhi_chan->gen_tre(mhi_cntrl, mhi_chan, buf, buf,
						len, MHI_EOT);
			if (ret) {
				MHI_ERR("Chan:%d error prepare buffer\n",
					mhi_chan->chan);
				kfree(buf);
				goto error_pre_alloc;
			}
		}

		read_lock_bh(&mhi_cntrl->pm_lock);
		if (MHI_DB_ACCESS_VALID(mhi_cntrl->pm_state)) {
			read_lock_irq(&mhi_chan->lock);
			mhi_ring_chan_db(mhi_cntrl, mhi_chan);
			read_unlock_irq(&mhi_chan->lock);
		}
		read_unlock_bh(&mhi_cntrl->pm_lock);
	}

	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	mutex_unlock(&mhi_chan->mutex);

	MHI_LOG("Chan:%d successfully moved to start state\n", mhi_chan->chan);

	return 0;

error_send_cmd:
	mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

error_pm_state:
	if (!mhi_chan->offload_ch)
		mhi_deinit_chan_ctxt(mhi_cntrl, mhi_chan);

error_init_chan:
	mutex_unlock(&mhi_chan->mutex);

	return ret;

error_pre_alloc:

	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	mutex_unlock(&mhi_chan->mutex);
	__mhi_unprepare_channel(mhi_cntrl, mhi_chan);

	return ret;
}

static void mhi_mark_stale_events(struct mhi_controller *mhi_cntrl,
				  struct mhi_event *mhi_event,
				  struct mhi_event_ctxt *er_ctxt,
				  int chan)
{
	struct mhi_tre *dev_rp, *local_rp;
	struct mhi_ring *ev_ring;
	unsigned long flags;

	MHI_LOG("Marking all events for chan:%d as stale\n", chan);

	ev_ring = &mhi_event->ring;

	/* mark all stale events related to channel as STALE event */
	spin_lock_irqsave(&mhi_event->lock, flags);
	dev_rp = mhi_to_virtual(ev_ring, er_ctxt->rp);

	local_rp = ev_ring->rp;
	while (dev_rp != local_rp) {
		if (MHI_TRE_GET_EV_TYPE(local_rp) ==
		    MHI_PKT_TYPE_TX_EVENT &&
		    chan == MHI_TRE_GET_EV_CHID(local_rp))
			local_rp->dword[1] = MHI_TRE_EV_DWORD1(chan,
					MHI_PKT_TYPE_STALE_EVENT);
		local_rp++;
		if (local_rp == (ev_ring->base + ev_ring->len))
			local_rp = ev_ring->base;
	}


	MHI_LOG("Finished marking events as stale events\n");
	spin_unlock_irqrestore(&mhi_event->lock, flags);
}

static void mhi_reset_data_chan(struct mhi_controller *mhi_cntrl,
				struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring, *tre_ring;
	struct mhi_result result;

	/* reset any pending buffers */
	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;
	result.transaction_status = -ENOTCONN;
	result.bytes_xferd = 0;
	while (tre_ring->rp != tre_ring->wp) {
		struct mhi_buf_info *buf_info = buf_ring->rp;

		if (mhi_chan->dir == DMA_TO_DEVICE) {
			if (atomic_dec_return(&mhi_cntrl->pending_pkts) == 0) 
				mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
		}

		if (mhi_chan->dir == DMA_TO_DEVICE)
			mhi_cntrl->wake_put(mhi_cntrl, false);
		if (!buf_info->pre_mapped)
			mhi_cntrl->unmap_single(mhi_cntrl, buf_info);
		mhi_del_ring_element(mhi_cntrl, buf_ring);
		mhi_del_ring_element(mhi_cntrl, tre_ring);

		if (mhi_chan->pre_alloc) {
			kfree(buf_info->cb_buf);
		} else {
			result.buf_addr = buf_info->cb_buf;
			mhi_chan->xfer_cb(mhi_chan->mhi_dev, &result);
		}
	}
}

static void mhi_reset_rsc_chan(struct mhi_controller *mhi_cntrl,
			       struct mhi_chan *mhi_chan)
{
	struct mhi_ring *buf_ring, *tre_ring;
	struct mhi_result result;
	struct mhi_buf_info *buf_info;

	/* reset any pending buffers */
	buf_ring = &mhi_chan->buf_ring;
	tre_ring = &mhi_chan->tre_ring;
	result.transaction_status = -ENOTCONN;
	result.bytes_xferd = 0;

	buf_info = buf_ring->base;
	for (; (void *)buf_info < buf_ring->base + buf_ring->len; buf_info++) {
		if (!buf_info->used)
			continue;

		result.buf_addr = buf_info->cb_buf;
		mhi_chan->xfer_cb(mhi_chan->mhi_dev, &result);
		buf_info->used = false;
	}
}

void mhi_reset_chan(struct mhi_controller *mhi_cntrl, struct mhi_chan *mhi_chan)
{

	struct mhi_event *mhi_event;
	struct mhi_event_ctxt *er_ctxt;
	int chan = mhi_chan->chan;

	/* nothing to reset, client don't queue buffers */
	if (mhi_chan->offload_ch)
		return;

	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_event = &mhi_cntrl->mhi_event[mhi_chan->er_index];
	er_ctxt = &mhi_cntrl->mhi_ctxt->er_ctxt[mhi_chan->er_index];

	mhi_mark_stale_events(mhi_cntrl, mhi_event, er_ctxt, chan);

	if (mhi_chan->xfer_type == MHI_XFER_RSC_DMA)
		mhi_reset_rsc_chan(mhi_cntrl, mhi_chan);
	else
		mhi_reset_data_chan(mhi_cntrl, mhi_chan);

	read_unlock_bh(&mhi_cntrl->pm_lock);
	MHI_LOG("Reset complete.\n");
}

static void __mhi_unprepare_channel(struct mhi_controller *mhi_cntrl,
				    struct mhi_chan *mhi_chan)
{
	int ret;

	MHI_LOG("Entered: unprepare channel:%d\n", mhi_chan->chan);

	/* no more processing events for this channel */
	mutex_lock(&mhi_chan->mutex);
	write_lock_irq(&mhi_chan->lock);
	if (mhi_chan->ch_state != MHI_CH_STATE_ENABLED) {
		MHI_LOG("chan:%d is already disabled\n", mhi_chan->chan);
		write_unlock_irq(&mhi_chan->lock);
		mutex_unlock(&mhi_chan->mutex);
		return;
	}

	mhi_chan->ch_state = MHI_CH_STATE_DISABLED;
	write_unlock_irq(&mhi_chan->lock);

	reinit_completion(&mhi_chan->completion);
	read_lock_bh(&mhi_cntrl->pm_lock);
	if (MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state)) {
		read_unlock_bh(&mhi_cntrl->pm_lock);
		goto error_invalid_state;
	}

	mhi_cntrl->wake_get(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	mhi_cntrl->runtime_get(mhi_cntrl, mhi_cntrl->priv_data);
	ret = mhi_send_cmd(mhi_cntrl, mhi_chan, MHI_CMD_RESET_CHAN);
	if (ret) {
		MHI_ERR("Failed to send reset chan cmd\n");
		goto error_completion;
	}

	/* even if it fails we will still reset */
	ret = wait_for_completion_timeout(&mhi_chan->completion,
				msecs_to_jiffies(mhi_cntrl->timeout_ms));
	if (!ret || mhi_chan->ccs != MHI_EV_CC_SUCCESS)
		MHI_ERR("Failed to receive cmd completion, still resetting\n");

error_completion:
	mhi_cntrl->runtime_put(mhi_cntrl, mhi_cntrl->priv_data);
	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

error_invalid_state:
	if (!mhi_chan->offload_ch) {
		mhi_reset_chan(mhi_cntrl, mhi_chan);
		mhi_deinit_chan_ctxt(mhi_cntrl, mhi_chan);
	}
	MHI_LOG("chan:%d successfully resetted\n", mhi_chan->chan);
	mutex_unlock(&mhi_chan->mutex);
}

int mhi_debugfs_mhi_states_show(struct seq_file *m, void *d)
{
	struct mhi_controller *mhi_cntrl = m->private;
	int reg = 0;
	int ret;
	u32 val[4];

	seq_printf(m,
		   "pm_state:%s dev_state:%s EE:%s M0:%u M2:%u M3:%u wake:%d dev_wake:%u alloc_size:%u\n",
		   to_mhi_pm_state_str(mhi_cntrl->pm_state),
		   TO_MHI_STATE_STR(mhi_cntrl->dev_state),
		   TO_MHI_EXEC_STR(mhi_cntrl->ee),
		   mhi_cntrl->M0, mhi_cntrl->M2, mhi_cntrl->M3,
		   mhi_cntrl->wake_set,
		   atomic_read(&mhi_cntrl->dev_wake),
		   atomic_read(&mhi_cntrl->alloc_size));

	seq_printf(m,
		   "mhi_state:%s exec_env:%s\n",
		   TO_MHI_STATE_STR(mhi_get_mhi_state(mhi_cntrl)), TO_MHI_EXEC_STR(mhi_get_exec_env(mhi_cntrl)));

	seq_printf(m, "dump mhi reg addr:%p\n", mhi_cntrl->regs);
	for (reg = 0; reg < 0x100; reg+=16) {
		val[0] = val[1] = val[2] = val[3] = 0xFFFFFFFF;
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs, reg+0, &val[0]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs, reg+4, &val[1]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs, reg+8, &val[2]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->regs, reg+12, &val[3]);
		seq_printf(m, "%02x: %08x %08x %08x %08x\n", reg, val[0], val[1], val[2], val[3]);
	}

	seq_printf(m, "dump bhi reg addr:%p\n", mhi_cntrl->bhi);
	for (reg = 0; reg < 0x100; reg+=16) {
		val[0] = val[1] = val[2] = val[3] = 0xFFFFFFFF;
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->bhi, reg+0, &val[0]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->bhi, reg+4, &val[1]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->bhi, reg+8, &val[2]);
		ret = mhi_read_reg(mhi_cntrl, mhi_cntrl->bhi, reg+12, &val[3]);
		seq_printf(m, "%02x: %08x %08x %08x %08x\n", reg, val[0], val[1], val[2], val[3]);
	}

	return 0;
}

int mhi_debugfs_mhi_event_show(struct seq_file *m, void *d)
{
	struct mhi_controller *mhi_cntrl = m->private;
	struct mhi_event *mhi_event;
	struct mhi_event_ctxt *er_ctxt;

	int i;

	er_ctxt = mhi_cntrl->mhi_ctxt->er_ctxt;
	mhi_event = mhi_cntrl->mhi_event;
	for (i = 0; i < mhi_cntrl->total_ev_rings; i++, er_ctxt++,
		     mhi_event++) {
		struct mhi_ring *ring = &mhi_event->ring;

		if (mhi_event->offload_ev) {
			seq_printf(m, "Index:%d offload event ring\n", i);
		} else {
			seq_printf(m,
				   "Index:%d modc:%d modt:%d base:0x%0llx len:0x%llx",
				   i, er_ctxt->intmodc, er_ctxt->intmodt,
				   er_ctxt->rbase, er_ctxt->rlen);
			seq_printf(m,
				   " rp:0x%llx wp:0x%llx local_rp:0x%llx local_wp:0x%llx db:0x%llx\n",
				   er_ctxt->rp, er_ctxt->wp,
				   (unsigned long long)mhi_to_physical(ring, ring->rp),
				   (unsigned long long)mhi_to_physical(ring, ring->wp),
				   (unsigned long long)mhi_event->db_cfg.db_val);
			seq_printf(m, "used:%u\n", mhi_event->used_elements);

#ifdef DEBUG_CHAN100_DB
			if (mhi_event->mhi_chan && mhi_event->chan == 100) {
				struct mhi_tre *tre = (struct mhi_tre *)ring->base;
				size_t j;

				for (j = 0; j < ring->elements; j++, tre++) {
					seq_printf(m,
						"%08x: %llx, %08x, %08x\n",
						(unsigned int)(j*sizeof(struct mhi_tre)),
					tre->ptr, tre->dword[0], tre->dword[1]);
				}
			}
#endif
		}
	}

	return 0;
}

int mhi_debugfs_mhi_chan_show(struct seq_file *m, void *d)
{
	struct mhi_controller *mhi_cntrl = m->private;
	struct mhi_chan *mhi_chan;
	struct mhi_chan_ctxt *chan_ctxt;
	int i;

	mhi_chan = mhi_cntrl->mhi_chan;
	chan_ctxt = mhi_cntrl->mhi_ctxt->chan_ctxt;
	for (i = 0; i < mhi_cntrl->max_chan; i++, chan_ctxt++, mhi_chan++) {
		struct mhi_ring *ring = &mhi_chan->tre_ring;

		if (mhi_chan->ch_state == MHI_CH_STATE_DISABLED)
			continue;

		if (mhi_chan->offload_ch) {
			seq_printf(m, "%s(%u) offload channel\n",
				   mhi_chan->name, mhi_chan->chan);
		} else if (mhi_chan->mhi_dev) {
			seq_printf(m,
				   "%s(%u) state:0x%x brstmode:0x%x pllcfg:0x%x type:0x%x erindex:%u",
				   mhi_chan->name, mhi_chan->chan,
				   chan_ctxt->chstate, chan_ctxt->brstmode,
				   chan_ctxt->pollcfg, chan_ctxt->chtype,
				   chan_ctxt->erindex);
			seq_printf(m,
				   " base:0x%llx len:0x%llx rp:%llx wp:0x%llx local_rp:0x%llx local_wp:0x%llx db:0x%llx\n",
				   chan_ctxt->rbase, chan_ctxt->rlen,
				   chan_ctxt->rp, chan_ctxt->wp,
				   (unsigned long long)mhi_to_physical(ring, ring->rp),
				   (unsigned long long)mhi_to_physical(ring, ring->wp),
				   (unsigned long long)mhi_chan->db_cfg.db_val);
			seq_printf(m, "used:%u, EOB:%u, EOT:%u, OOB:%u, DB_MODE:%u\n", mhi_chan->used_elements,
				mhi_chan->used_events[MHI_EV_CC_EOB], mhi_chan->used_events[MHI_EV_CC_EOT],
				mhi_chan->used_events[MHI_EV_CC_OOB],mhi_chan->used_events[MHI_EV_CC_DB_MODE]);

#ifdef DEBUG_CHAN100_DB
			if (mhi_chan->chan == 100) {
				unsigned int n = 0;
				seq_printf(m, "chan100_seq = %04x\n", atomic_read(&chan100_seq)%CHAN100_SIZE);
				for (n = 0; n < CHAN100_SIZE; n++) {
					seq_printf(m, "%04x: %08x\n", n, chan100_t[n]);
				}
			}
#endif

#if 0
			if (ring->base && /*(i&1) &&*/ (i < MHI_CLIENT_IP_HW_0_OUT)) {
				struct mhi_tre *tre = (struct mhi_tre *)ring->base;
				size_t e;

				for (e = 0; e < ring->elements; e++, tre++) {
					seq_printf(m, "[%03d] %llx, %08x, %08x\n", i, tre->ptr, tre->dword[0], tre->dword[1]);
				}
			}
#endif
		}
	}

	return 0;
}

/* move channel to start state */
int mhi_prepare_for_transfer(struct mhi_device *mhi_dev)
{
	int ret, dir;
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan;

	for (dir = 0; dir < 2; dir++) {
		mhi_chan = dir ? mhi_dev->dl_chan : mhi_dev->ul_chan;

		if (!mhi_chan)
			continue;

		ret = __mhi_prepare_channel(mhi_cntrl, mhi_chan);
		if (ret) {
			MHI_ERR("Error moving chan %s,%d to START state\n",
				mhi_chan->name, mhi_chan->chan);
			goto error_open_chan;
		}

		if (mhi_dev->dl_chan == mhi_dev->ul_chan) {
			break;
		}
	}

	return 0;

error_open_chan:
	for (--dir; dir >= 0; dir--) {
		mhi_chan = dir ? mhi_dev->dl_chan : mhi_dev->ul_chan;

		if (!mhi_chan)
			continue;

		__mhi_unprepare_channel(mhi_cntrl, mhi_chan);
	}

	return ret;
}
EXPORT_SYMBOL(mhi_prepare_for_transfer);

void mhi_unprepare_from_transfer(struct mhi_device *mhi_dev)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan;
	int dir;

	for (dir = 0; dir < 2; dir++) {
		mhi_chan = dir ? mhi_dev->ul_chan : mhi_dev->dl_chan;

		if (!mhi_chan)
			continue;

		__mhi_unprepare_channel(mhi_cntrl, mhi_chan);

		if (mhi_dev->dl_chan == mhi_dev->ul_chan) {
			break;
		}
	}
}
EXPORT_SYMBOL(mhi_unprepare_from_transfer);

int mhi_get_no_free_descriptors(struct mhi_device *mhi_dev,
				enum dma_data_direction dir)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan = (dir == DMA_TO_DEVICE) ?
		mhi_dev->ul_chan : mhi_dev->dl_chan;
	struct mhi_ring *tre_ring = &mhi_chan->tre_ring;

	if (mhi_chan->offload_ch)
		return 0;
	
	return get_nr_avail_ring_elements(mhi_cntrl, tre_ring);
}
EXPORT_SYMBOL(mhi_get_no_free_descriptors);

#if (LINUX_VERSION_CODE < KERNEL_VERSION( 5,3,0 ))
static int __mhi_bdf_to_controller(struct device *dev, void *tmp)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	struct mhi_device *match = tmp;

	/* return any none-zero value if match */
	if (mhi_dev->dev_type == MHI_CONTROLLER_TYPE &&
	    mhi_dev->domain == match->domain && mhi_dev->bus == match->bus &&
	    mhi_dev->slot == match->slot && mhi_dev->dev_id == match->dev_id)
		return 1;

	return 0;
}
#else
static int __mhi_bdf_to_controller(struct device *dev, const void *tmp)
{
	struct mhi_device *mhi_dev = to_mhi_device(dev);
	const struct mhi_device *match = tmp;

	/* return any none-zero value if match */
	if (mhi_dev->dev_type == MHI_CONTROLLER_TYPE &&
	    mhi_dev->domain == match->domain && mhi_dev->bus == match->bus &&
	    mhi_dev->slot == match->slot && mhi_dev->dev_id == match->dev_id)
		return 1;

	return 0;
}
#endif

struct mhi_controller *mhi_bdf_to_controller(u32 domain,
					     u32 bus,
					     u32 slot,
					     u32 dev_id)
{
	struct mhi_device tmp, *mhi_dev;
	struct device *dev;

	tmp.domain = domain;
	tmp.bus = bus;
	tmp.slot = slot;
	tmp.dev_id = dev_id;

	dev = bus_find_device(&mhi_bus_type, NULL, &tmp,
			      __mhi_bdf_to_controller);
	if (!dev)
		return NULL;

	mhi_dev = to_mhi_device(dev);

	return mhi_dev->mhi_cntrl;
}
EXPORT_SYMBOL(mhi_bdf_to_controller);

int mhi_poll(struct mhi_device *mhi_dev,
	     u32 budget)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_chan *mhi_chan = mhi_dev->dl_chan;
	struct mhi_event *mhi_event = &mhi_cntrl->mhi_event[mhi_chan->er_index];
	int ret;

	spin_lock_bh(&mhi_event->lock);
	ret = mhi_event->process_event(mhi_cntrl, mhi_event, budget);
	spin_unlock_bh(&mhi_event->lock);

	return ret;
}
EXPORT_SYMBOL(mhi_poll);

int mhi_get_remote_time_sync(struct mhi_device *mhi_dev,
			     u64 *t_host,
			     u64 *t_dev)
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_timesync *mhi_tsync = mhi_cntrl->mhi_tsync;
	int ret;

	/* not all devices support time feature */
	if (!mhi_tsync)
		return -EIO;

	/* bring to M0 state */
	ret = __mhi_device_get_sync(mhi_cntrl);
	if (ret)
		return ret;

	mutex_lock(&mhi_tsync->lpm_mutex);

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (unlikely(MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state))) {
		MHI_ERR("MHI is not in active state, pm_state:%s\n",
			to_mhi_pm_state_str(mhi_cntrl->pm_state));
		ret = -EIO;
		goto error_invalid_state;
	}

	/* disable link level low power modes */
	ret = mhi_cntrl->lpm_disable(mhi_cntrl, mhi_cntrl->priv_data);
	if (ret)
		goto error_invalid_state;

	/*
	 * time critical code to fetch device times,
	 * delay between these two steps should be
	 * deterministic as possible.
	 */
	preempt_disable();
	local_irq_disable();

	*t_host = mhi_cntrl->time_get(mhi_cntrl, mhi_cntrl->priv_data);
	*t_dev = readq_relaxed_no_log(mhi_tsync->time_reg);

	local_irq_enable();
	preempt_enable();

	mhi_cntrl->lpm_enable(mhi_cntrl, mhi_cntrl->priv_data);

error_invalid_state:
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);
	mutex_unlock(&mhi_tsync->lpm_mutex);

	return ret;
}
EXPORT_SYMBOL(mhi_get_remote_time_sync);

/**
 * mhi_get_remote_time - Get external modem time relative to host time
 * Trigger event to capture modem time, also capture host time so client
 * can do a relative drift comparision.
 * Recommended only tsync device calls this method and do not call this
 * from atomic context
 * @mhi_dev: Device associated with the channels
 * @sequence:unique sequence id track event
 * @cb_func: callback function to call back
 */
int mhi_get_remote_time(struct mhi_device *mhi_dev,
			u32 sequence,
			void (*cb_func)(struct mhi_device *mhi_dev,
					u32 sequence,
					u64 local_time,
					u64 remote_time))
{
	struct mhi_controller *mhi_cntrl = mhi_dev->mhi_cntrl;
	struct mhi_timesync *mhi_tsync = mhi_cntrl->mhi_tsync;
	struct tsync_node *tsync_node;
	int ret;

	/* not all devices support time feature */
	if (!mhi_tsync)
		return -EIO;

	/* tsync db can only be rung in M0 state */
	ret = __mhi_device_get_sync(mhi_cntrl);
	if (ret)
		return ret;

	/*
	 * technically we can use GFP_KERNEL, but wants to avoid
	 * # of times scheduling out
	 */
	tsync_node = kzalloc(sizeof(*tsync_node), GFP_ATOMIC);
	if (!tsync_node) {
		ret = -ENOMEM;
		goto error_no_mem;
	}

	tsync_node->sequence = sequence;
	tsync_node->cb_func = cb_func;
	tsync_node->mhi_dev = mhi_dev;

	/* disable link level low power modes */
	mhi_cntrl->lpm_disable(mhi_cntrl, mhi_cntrl->priv_data);

	read_lock_bh(&mhi_cntrl->pm_lock);
	if (unlikely(MHI_PM_IN_ERROR_STATE(mhi_cntrl->pm_state))) {
		MHI_ERR("MHI is not in active state, pm_state:%s\n",
			to_mhi_pm_state_str(mhi_cntrl->pm_state));
		ret = -EIO;
		goto error_invalid_state;
	}

	spin_lock_irq(&mhi_tsync->lock);
	list_add_tail(&tsync_node->node, &mhi_tsync->head);
	spin_unlock_irq(&mhi_tsync->lock);

	/*
	 * time critical code, delay between these two steps should be
	 * deterministic as possible.
	 */
	preempt_disable();
	local_irq_disable();

	tsync_node->local_time =
		mhi_cntrl->time_get(mhi_cntrl, mhi_cntrl->priv_data);
	writel_relaxed_no_log(tsync_node->sequence, mhi_tsync->db);
	/* write must go thru immediately */
	wmb();

	local_irq_enable();
	preempt_enable();

	ret = 0;

error_invalid_state:
	if (ret)
		kfree(tsync_node);
	read_unlock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->lpm_enable(mhi_cntrl, mhi_cntrl->priv_data);

error_no_mem:
	read_lock_bh(&mhi_cntrl->pm_lock);
	mhi_cntrl->wake_put(mhi_cntrl, false);
	read_unlock_bh(&mhi_cntrl->pm_lock);

	return ret;
}
EXPORT_SYMBOL(mhi_get_remote_time);

void mhi_debug_reg_dump(struct mhi_controller *mhi_cntrl)
{
	enum mhi_dev_state state;
	enum mhi_ee ee;
	int i, ret;
	u32 val = 0;
	void __iomem *mhi_base = mhi_cntrl->regs;
	void __iomem *bhi_base = mhi_cntrl->bhi;
	void __iomem *bhie_base = mhi_cntrl->bhie;
	void __iomem *wake_db = mhi_cntrl->wake_db;
	struct {
		const char *name;
		int offset;
		void *base;
	} debug_reg[] = {
		{ "MHI_CNTRL", MHICTRL, mhi_base},
		{ "MHI_STATUS", MHISTATUS, mhi_base},
		{ "MHI_WAKE_DB", 0, wake_db},
		{ "BHI_EXECENV", BHI_EXECENV, bhi_base},
		{ "BHI_STATUS", BHI_STATUS, bhi_base},
		{ "BHI_ERRCODE", BHI_ERRCODE, bhi_base},
		{ "BHI_ERRDBG1", BHI_ERRDBG1, bhi_base},
		{ "BHI_ERRDBG2", BHI_ERRDBG2, bhi_base},
		{ "BHI_ERRDBG3", BHI_ERRDBG3, bhi_base},
		{ "BHIE_TXVEC_DB", BHIE_TXVECDB_OFFS, bhie_base},
		{ "BHIE_TXVEC_STATUS", BHIE_TXVECSTATUS_OFFS, bhie_base},
		{ "BHIE_RXVEC_DB", BHIE_RXVECDB_OFFS, bhie_base},
		{ "BHIE_RXVEC_STATUS", BHIE_RXVECSTATUS_OFFS, bhie_base},
		{ NULL },
	};

	MHI_LOG("host pm_state:%s dev_state:%s ee:%s\n",
		to_mhi_pm_state_str(mhi_cntrl->pm_state),
		TO_MHI_STATE_STR(mhi_cntrl->dev_state),
		TO_MHI_EXEC_STR(mhi_cntrl->ee));

	state = mhi_get_mhi_state(mhi_cntrl);
	ee = mhi_get_exec_env(mhi_cntrl);

	MHI_LOG("device ee:%s dev_state:%s\n", TO_MHI_EXEC_STR(ee),
		TO_MHI_STATE_STR(state));

	for (i = 0; debug_reg[i].name; i++) {
		ret = mhi_read_reg(mhi_cntrl, debug_reg[i].base,
				   debug_reg[i].offset, &val);
		MHI_LOG("reg:%s val:0x%x, ret:%d\n", debug_reg[i].name, val,
			ret);
	}
}
EXPORT_SYMBOL(mhi_debug_reg_dump);
