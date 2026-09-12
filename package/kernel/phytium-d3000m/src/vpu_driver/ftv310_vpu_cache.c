// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver main entrance.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License, version 2, as
 *    published by the Free Software Foundation.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License version 2 for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 at the following locations:
 *    https://opensource.org/licenses/gpl-2.0.php
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/moduleparam.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/spinlock.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <asm/irq.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/of.h>

#include "ftv310_vpu_cache.h"
#include "ipoffset/cache_offset.h"
/********variables declaration related with race condition**********/

//struct semaphore enc_core_sem;

/*------------------PORTING LAYER------------------------------------*/
/* Cache types */
#define CCLIENT_TYPE_FTV310E "_FTV310E"
#define CCLIENT_TYPE_FTV310D0 "_FTV310D_0"
#define CCLIENT_TYPE_FTV310D1 "_FTV310D_1"
#define CCLIENT_TYPE_DECG10 "_DECODER_G1_0"
#define CCLIENT_TYPE_DECG11 "_DECODER_G1_1"
#define CCLIENT_TYPE_DECG20 "_DECODER_G2_0"
#define CCLIENT_TYPE_DECG21 "_DECODER_G2_1"
/* Cache directions */
#define CC_DIR_READ "_DIRRD"
#define CC_DIR_WRITE "_DIRWR"
#define CC_DIR_BIDIR "_DIRBI"

static int bcacheprobed;

/*------------------------------END-------------------------------------*/

/***************************TYPE AND FUNCTION DECLARATION****************/

/* here's all the must remember stuff */
static int ReserveIO(struct cache_dev_t *);
static void ReleaseIO(struct cache_dev_t *);
#ifndef PHY_FPGA
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t cache_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t cache_isr(int irq, void *dev_id);
#endif
#endif
/*********************local variable declaration*****************/

/******************************************************************************/
long CacheReadRegs(struct cache_dev_t *dev, struct core_desc *core)
{
	u32 i;
	long ret = 0;

	i = core->reg_id;
	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
		dev->dec_regs[i] = ioread32((void *)dev->hwregs + i * 4);
	ret = copy_to_user(core->regs, dev->dec_regs + core->reg_id,
			   core->size);
	if (ret) {
		PDEBUG("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	return 0;
}

long CacheWriteRegs(struct cache_dev_t *dev, struct core_desc *core)
{
	u32 i;
	long ret = 0;

	i = core->reg_id;
	ret = copy_from_user(dev->dec_regs + core->reg_id, core->regs,
			     core->size);

	if (ret) {
		PDEBUG("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
		iowrite32(dev->dec_regs[i], (void *)dev->hwregs + i * 4);
	return 0;
}

static int CheckCacheIrq(struct cache_dev_t *dev)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);
	unsigned long flags;
	int rdy = 0;

	spin_lock_irqsave(&parentslice->cache_owner_lock, flags);
	if (dev->irq_received) {
		/* reset the wait condition(s) */
		//		dev->irq_received = 0;
		rdy = 1;
	}
	spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);

	return rdy;
}

static unsigned int WaitCacheReady(struct cache_dev_t *dev)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);

	if (wait_event_interruptible(parentslice->cache_wait_queue,
				     CheckCacheIrq(dev))) {
		PDEBUG("Cache wait_event_interruptible interrupted\n");
		return -ERESTARTSYS;
	}

	return 0;
}

static int CheckCoreOccupation(struct cache_dev_t *dev, struct file *filp)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);
	int ret = 0;
	unsigned long flags;

	spin_lock_irqsave(&parentslice->cache_owner_lock, flags);
	if (!dev->is_reserved) {
		dev->is_reserved = 1;
		dev->cacheowner = filp;
		ret = 1;
	}

	spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);

	return ret;
}

static int GetWorkableCore(struct cache_dev_t *dev, struct file *filp)
{
	return CheckCoreOccupation(dev, filp);
}

static long ReserveCore(struct cache_dev_t *dev, struct file *filp)
{
	struct slice_info *parentslice;

	parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);

	/* lock a core that has specified core id*/
	if (wait_event_interruptible(parentslice->cache_hw_queue,
				     GetWorkableCore(dev, filp) != 0))
		return -ERESTARTSYS;

	return 0;
}

static void ReleaseCore(struct cache_dev_t *dev)
{
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);

	/* release specified core id */
	spin_lock_irqsave(&parentslice->cache_owner_lock, flags);
	if (dev->is_reserved) {
		dev->cacheowner = NULL;
		dev->is_reserved = 0;
	}

	dev->irq_received = 0;
	dev->irq_status = 0;
	spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);

	wake_up_interruptible_all(&parentslice->cache_hw_queue);
}

long ftv310_vpu_cache_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	u32 tmp, id, slice, node, type;
	u32 core_id;
	unsigned long long tmp64;
	struct cache_dev_t *pccore;

	switch (cmd) {
	case CACHE_IOCGHWOFFSET:
		__get_user(id, (int *)arg);
		slice = SLICE(id);
		node = KCORE(id);
		pccore = get_cachenodes(slice, node);
		if (!pccore)
			return -EFAULT;
		__put_user(pccore->com_base_addr, (unsigned long long *)arg);
		break;
	case CACHE_IOCGHWIOSIZE:
		id = (u32)arg;
		slice = SLICE(id);
		node = KCORE(id);
		pccore = get_cachenodes(slice, node);
		if (!pccore)
			return -EFAULT;
		else
			return pccore->core_cfg.iosize;
		break;
	case CACHE_IOCG_CORE_NUM:
		id = arg;
		id = get_slicecorenum(id, FTV310_VPU_CORE_CACHE);
		return id;
	case CACHE_IOCH_HW_RESERVE: {
		driver_cache_dir dir;
		cache_client_type client;
		/*
		 *  it's a little danger here since here's
		 *  no protection of the chain
		 */
		__get_user(tmp64, (unsigned long long *)arg);
		id = tmp64 >> 32;
		slice = SLICE(id);
		type = NODETYPE(id);
		node = KCORE(id);
		core_id = (u32)tmp64; //get client and direction info
		dir = core_id & 0x01;
		client = (core_id & 0x06) >> 1;
		pccore = get_cachenodes(slice, 0);

		while (pccore) {
			/* a valid core supports such client and dir*/
			if (pccore->core_cfg.client == client &&
			    pccore->core_cfg.dir == dir &&
			    pccore->parentid == node && pccore->is_valid &&
			    ((type == NODE_TYPE_DEC &&
			      pccore->parenttype == FTV310_VPU_CORE_DEC) ||
			     (type == NODE_TYPE_ENC &&
			      pccore->parenttype == FTV310_VPU_CORE_ENC)))
				break;
			pccore = pccore->next;
		}
		if (!pccore)
			return -EFAULT;

		ret = ReserveCore(pccore, filp);
		if (ret == 0)
			return pccore->core_id;
		return ret;
	}
	case CACHE_IOCH_HW_RELEASE:
		core_id = (u32)arg;
		slice = SLICE(core_id);
		node = KCORE(core_id);
		pccore = get_cachenodes(slice, node);
		if (!pccore)
			return -EFAULT;

		ReleaseCore(pccore);

		break;
	case CACHE_IOCG_ABORT_WAIT:
		core_id = (u32)arg;
		slice = SLICE(core_id);
		node = KCORE(core_id);
		pccore = get_cachenodes(slice, node);
		if (!pccore)
			return -EFAULT;
		tmp = WaitCacheReady(pccore);
		if (tmp == 0)
			return pccore->irq_status;
		break;
	}
	return 0;
}

int cache_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int cache_release(struct file *filp)
{
	int i, slicen = get_slicenumber();
	struct cache_dev_t *dev;

	for (i = 0; i < slicen; i++) {
		dev = get_cachenodes(i, 0);
		while (dev) {
			if (dev->cacheowner == filp && dev->is_reserved)
				ReleaseCore(dev);
			dev = dev->next;
		}
	}
	return 0;
}

int __init cache_init(void)
{
	int result = 0;

	bcacheprobed = 0;
	return result;
}

#ifdef USE_DTB_PROBE
static void cache_getcachetype(const char *name, int *client, int *dir)
{
	if (strstr(name, CCLIENT_TYPE_FTV310E))
		*client = VCE;
	else if (strstr(name, CCLIENT_TYPE_FTV310D0))
		*client = VCD_0;
	else if (strstr(name, CCLIENT_TYPE_FTV310D1))
		*client = VCD_1;
	else if (strstr(name, CCLIENT_TYPE_DECG10))
		*client = DECODER_G1_0;
	else if (strstr(name, CCLIENT_TYPE_DECG11))
		*client = DECODER_G1_1;
	else if (strstr(name, CCLIENT_TYPE_DECG20))
		*client = DECODER_G2_0;
	else if (strstr(name, CCLIENT_TYPE_DECG21))
		*client = DECODER_G2_1;
	else
		*client = -1;

	if (strstr(name, CC_DIR_READ))
		*dir = DIR_RD;
	else if (strstr(name, CC_DIR_WRITE))
		*dir = DIR_WR;
	else if (strstr(name, CC_DIR_BIDIR))
		*dir = DIR_RD;
	else
		*dir = -1;
}
#endif

int cache_probe(dtbnode *pnode, int useirq, int loop,
		struct cache_dev_t *cachecore)
{
	int result;
	int i, k;
	struct cache_dev_t *pccore;

	if (loop == 0) {
#ifndef USE_DTB_PROBE /*simulate and compatible with old code*/
		for (i = 0; i < ARRAY_SIZE(cache_core_array); i++) {
			pccore = vmalloc(sizeof(*pccore));
			if (!pccore)
				return -ENOMEM;

			memset(pccore, 0, sizeof(struct cache_dev_t));

			pccore->core_cfg.base_addr =
				cache_core_array[i].base_addr;
			pccore->com_base_addr = pccore->core_cfg.base_addr;
			pccore->core_cfg.iosize = cache_core_array[i].iosize;
			pccore->core_cfg.client = cache_core_array[i].client;
			pccore->core_cfg.dir = cache_core_array[i].dir;
			pccore->core_cfg.sliceidx =
				cache_core_array[i].sliceidx;
			pccore->core_cfg.parentaddr =
				cache_core_array[i].parentaddr;
			for (k = 0; k < 4; k++)
				pccore->irqlist[k] = -1;
			pccore->irqlist[0] = cache_core_array[i].irq;
			pccore->is_valid = 1;

			add_cachenode(pccore->core_cfg.sliceidx, pccore);
		}

#else /*USE_DTB_PROBE*/
		int type, dir;

		cache_getcachetype(pnode->ofnode->name, &type, &dir);
		if (type == -1 || dir == -1)
			return -EINVAL;
		pccore = vmalloc(sizeof(*pccore));
		if (!pccore)
			return -ENOMEM;

		memset(pccore, 0, sizeof(struct cache_dev_t));
		pccore->core_cfg.base_addr = pnode->ioaddr;
		pccore->com_base_addr = pccore->core_cfg.base_addr;
		pccore->core_cfg.iosize = pnode->iosize;
		pccore->core_cfg.client = type;
		pccore->core_cfg.dir = dir;
		for (i = 0; i < 4; i++)
			pccore->irqlist[i] = -1;
		pccore->irqlist[0] = pnode->irq[0];
		pccore->core_cfg.sliceidx = pnode->sliceidx;
		pccore->core_cfg.parentaddr = pnode->parentaddr;
		add_cachenode(pnode->sliceidx, pccore);
#endif /*USE_DTB_PROBE*/
	} else {
		result = ReserveIO(cachecore);
		if (result < 0) {
			pr_err("cachecore: reserve reg 0x%llx-0x%x fail\n",
			       cachecore->com_base_addr,
			       cachecore->core_cfg.iosize);
			return -ENODEV;
		}
	}

	return 0;
}

void __exit cache_cleanup(void)
{
	int i, k, slicen = get_slicenumber();
	struct cache_dev_t *pccore, *pnext;

	for (i = 0; i < slicen; i++) {
		pccore = get_cachenodes(i, 0);
		while (pccore) {
			pnext = pccore->next;
			writel(0, pccore->hwregs + 0x04); /* disable HW */
			writel(0xF, pccore->hwregs + 0x14); /* clear IRQ */

			/* free the encoder IRQ */
			for (k = 0; k < 4; k++)
				if (pccore->irqlist[k] > 0)
					free_irq(pccore->irqlist[k],
						 (void *)pccore);
			ReleaseIO(pccore);

			vfree(pccore);
			pccore = pnext;
		}
	}
	bcacheprobed = 0;
}

static int cache_get_hwid(unsigned long base_addr, int *hwid)
{
	u8 *hwregs = NULL;

	if (!request_mem_region(base_addr, 4, "ftv310_vpu_cache")) {
		PDEBUG(KERN_INFO
		       "ftv310_cache: failed to reserve HW regs,base_addr:%p\n",
		       (void *)base_addr);
		return -1;
	}

	hwregs = (u8 *)ioremap(base_addr, 4);
	if (!hwregs) {
		PDEBUG(KERN_INFO "ftv310_cache: failed to ioremap HW regs\n");
		release_mem_region(base_addr, 4);
		return -1;
	}

	*hwid = readl(hwregs + 0x00);
	PDEBUG(KERN_INFO "ftv310_vpu_cache: hwid = %x, base_addr= %p\n", (int)*hwid,
	       (void *)base_addr);

	if (hwregs)
		iounmap((void *)hwregs);
	release_mem_region(base_addr, 4);

	return 0;
}

static int ReserveIO(struct cache_dev_t *pccore)
{
	int hwid;

	if (!request_mem_region(pccore->core_cfg.base_addr,
				pccore->core_cfg.iosize, "ftv310_vpu_cache")) {
		PDEBUG(KERN_INFO
		       "ftv310_cache: failed to reserve HW regs,base_addr:%p\n",
		       (void *)pccore->core_cfg.base_addr);
		return -1;
	}

	pccore->hwregs = (u8 *)ioremap(pccore->core_cfg.base_addr,
				       pccore->core_cfg.iosize);
	if (!pccore->hwregs) {
		PDEBUG(KERN_INFO "ftv310_cache: failed to ioremap HW regs\n");
		release_mem_region(pccore->core_cfg.base_addr,
				   pccore->core_cfg.iosize);
		return -1;
	}

	hwid = readl(pccore->hwregs + 0x00);
	pr_info("ftv310_vpu_cache: hwid = %x, base_addr= %p\n", hwid,
		(void *)pccore->core_cfg.base_addr);

	pccore->dec_regs = vmalloc(pccore->core_cfg.iosize);
	if (!pccore->dec_regs)
		return -ENOMEM;

	if (pccore->core_cfg.dir == DIR_RD)
		PDEBUG("cache  reg[0x10]=%08x\n", readl(pccore->hwregs + 0x10));
	else
		PDEBUG("shaper reg[0x08]=%08x\n", readl(pccore->hwregs + 0x08));

	return 0;
}

static void ReleaseIO(struct cache_dev_t *pccore)
{
	if (pccore->is_valid == 0)
		return;
	if (pccore->dec_regs)
		vfree(pccore->dec_regs);
	if (pccore->hwregs)
		iounmap((void *)pccore->hwregs);
	release_mem_region(pccore->core_cfg.base_addr, pccore->core_cfg.iosize);
}

#ifndef PHY_FPGA
#if (KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE)
static irqreturn_t cache_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t cache_isr(int irq, void *dev_id)
#endif
{
	unsigned int handled = 0;
	struct cache_dev_t *dev = (struct cache_dev_t *)dev_id;
	u32 irq_status;
	unsigned long flags;
	u32 irq_triggered = 0;
	struct slice_info *parentslice;

	parentslice = getparentslice(dev, FTV310_VPU_CORE_CACHE);
	/* If core is not reserved by any user,
	 * but irq is received, just ignore it
	 */
	spin_lock_irqsave(&parentslice->cache_owner_lock, flags);
	if (!dev->is_reserved) {
		spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);
		return IRQ_HANDLED;
	}
	spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);

	if (dev->core_cfg.dir == DIR_RD) {
		irq_status = readl(dev->hwregs + 0x04);
		if (irq_status & 0x28) {
			irq_triggered = 1;
			writel(irq_status, dev->hwregs + 0x04); //clear irq
		}
	} else {
		irq_status = readl(dev->hwregs + 0x0C);
		if (irq_status) {
			irq_triggered = 1;
			writel(irq_status, dev->hwregs + 0x0C); //clear irq
		}
	}
	if (irq_triggered == 1) {
		/* clear all IRQ bits. IRQ is cleared by writing 1 */
		spin_lock_irqsave(&parentslice->cache_owner_lock, flags);
		dev->irq_received = 1;
		dev->irq_status = irq_status;
		spin_unlock_irqrestore(&parentslice->cache_owner_lock, flags);

		wake_up_interruptible_all(&parentslice->cache_wait_queue);
		handled++;
	}
	if (!handled)
		PDEBUG("IRQ received, but not cache's!\n");
	return IRQ_HANDLED;
}
#endif
