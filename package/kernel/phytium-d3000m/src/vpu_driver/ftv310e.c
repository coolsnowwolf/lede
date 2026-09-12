// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 encoder hardware driver.
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
#include <linux/io.h>
#include <linux/pci.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/vmalloc.h>
#include <linux/timer.h>
#include "ftv310e.h"
#include <linux/irq.h>
#include <linux/delay.h>
#include "ipoffset/vce_offset.h"

#ifdef PHY_FPGA_MEM
#include "ftv310_vpu_fpga_mem.h"
#endif

#ifdef PHY_FPGA_PCIE
#include "ftv310_vpu_pcie.h"
#endif

static u32 resource_shared;

#define FTV310_VPU_FTV310E_REG_BWREAD 215
#define FTV310_VPU_FTV310E_REG_BWWRITE 219
#define FTV310E_BURSTWIDTH 16

static int bencprobed;

/*------------------------------END-------------------------------------*/

/***************************TYPE AND FUNCTION DECLARATION****************/

/* here's all the must remember stuff */

static int ReserveIO(struct ftv310_vpu_enc_t *pcore);
static void ReleaseIO(struct ftv310_vpu_enc_t *pcore);
static void ResetAsic(struct ftv310_vpu_enc_t *dev);
static int CheckCoreOccupation(struct ftv310_vpu_enc_t *dev);
static void ReleaseEncoder(struct ftv310_vpu_enc_t *dev, u32 *core_info,
			   u32 nodenum);

/* IRQ handler */
#if KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE
static irqreturn_t ftv310_vpu_enc_isr(int irq, void *dev_id, struct pt_regs *regs);
#else
static irqreturn_t ftv310_vpu_enc_isr(int irq, void *dev_id);
#endif

/*********************local variable declaration*****************/
unsigned long long sram_base;
unsigned int sram_size;
/* and this is our MAJOR; use 0 for dynamic allocation (recommended)*/
static int ftv310_vpu_enc_major;

#ifdef PHY_CONFIG_PM
static long EncRestoreRegs(struct ftv310_vpu_enc_t *dev)
{
	long i;
	/* write all regs to hardware */
	u32 *reg_buf = dev->reg_buf;

	for (i = 0; i < ASIC_SWREG_AMOUNT * 4; i += 4)
		iowrite32(reg_buf[i/4], (void __iomem *)(dev->hwregs + i));

	return 0;
}

static long EncStoreRegs(struct ftv310_vpu_enc_t *dev)
{
	long i;
	/* read all registers from hardware */
	u32 *reg_buf = dev->reg_buf;

	for (i = 0; i < ASIC_SWREG_AMOUNT * 4; i += 4)
		reg_buf[i/4] = ioread32((void __iomem *)(dev->hwregs + i));

	return 0;
}

int enc_pm_suspend(void *_dev)
{
	struct ftv310_vpu_enc_t *dev = (struct ftv310_vpu_enc_t *)_dev;

	pr_info("%s start..\n", __func__);

	while (dev) {
		/*if HW is active, need to wait until frame ready interrupt*/
		if ((dev->is_reserved == 0) || (down_interruptible(&dev->core_suspend_sem)))
			continue;

		dev->reg_corrupt = 1;
		if (dev->irq_status & 0x04)
			EncStoreRegs(dev);

		up(&dev->core_suspend_sem);
		dev = dev->next;
	}

	pr_info("%s succeed!\n", __func__);
	return 0;
}

int enc_pm_resume(void *_dev)
{
	u32 *reg_buf;
	struct ftv310_vpu_enc_t *dev = (struct ftv310_vpu_enc_t *)_dev;

	pr_info("%s start..\n", __func__);

	while (dev) {
		if (dev->is_reserved == 0)
			continue;

		reg_buf = dev->reg_buf;

		if (dev->irq_status & 0x04) {
			EncRestoreRegs(dev);
			dev->reg_corrupt = 0;
		}
		dev = dev->next;
	}

	pr_info("%s succeed!\n", __func__);
	return 0;
}
#endif

/******************************************************************************/
static int CheckEncIrq(struct ftv310_vpu_enc_t *dev, u32 *core_info, u32 *irq_status,
		       u32 nodenum)
{
	unsigned long flags;
	int rdy = 0;
	u32 i = 0;
	u8 core_mapping = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	core_mapping = (u8)(*core_info & 0xFF);

	//pr_info("core_mapping = %d\n",core_mapping);
	while (core_mapping) {
		if (core_mapping & 0x1) {
			if (i >= nodenum)
				break;

			spin_lock_irqsave(&parentslice->enc_owner_lock, flags);

			if (dev->irq_received) {
				/* reset the wait condition(s) */
				PDEBUG("check %d irq ready\n", i);
				dev->irq_received = 0;
				rdy = 1;
				*core_info = i;
				*irq_status = dev->irq_status;
			}

			spin_unlock_irqrestore(&parentslice->enc_owner_lock,
					       flags);
			break;
		}
		core_mapping = core_mapping >> 1;
		i++;
		dev = dev->next;
	}
	//pr_info("rdy=%d\n",rdy);
	return rdy;
}

static int CheckEncIrqbyPolling(struct ftv310_vpu_enc_t *dev, u32 *irq_status)
{
	unsigned long flags;
	int rdy = 0;
	u32 irq, hwId, majorId, wClr;
	u32 loop = 30;
	u32 interval = 100;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	pr_info("%s,%d\n", __func__, __LINE__);

	do {
		spin_lock_irqsave(&parentslice->enc_owner_lock, flags);

		if (dev->irq_received) {
			pr_info("%s,%d\n", __func__, __LINE__);

			PDEBUG("check %d irq ready\n", i);
			dev->irq_received = 0;
			rdy = 1;
			*irq_status = dev->irq_status;
			goto end_1;
		}

		irq = (u32)ioread32((void *)(dev->hwregs + 0x04));
		pr_info("%s,%d,irq %x\n", __func__, __LINE__, irq);

		if (irq & ASIC_STATUS_ALL) {
			if (irq & 0x20)
				iowrite32(0, (void *)(dev->hwregs + 0x14));

			/* clear all IRQ bits. (hwId >= 0x80006100) means
			 * IRQ is cleared by writing 1
			 */
			hwId = ioread32((void *)dev->hwregs);
			majorId = (hwId & 0x0000FF00) >> 8;
			wClr = (majorId >= 0x61) ? irq : (irq & (~0x1FD));
			iowrite32(wClr, (void *)(dev->hwregs + 0x04));

			rdy = 1;
			*irq_status = irq;
			dev->irq_received = 0;
			dev->irq_status = irq;
#ifdef PHY_CONFIG_PM
		//if frame_rdy IRQ is received, then HW will not be used any more.
		if (*irq_status & ASIC_STATUS_FRAME_READY)
			up(&dev->core_suspend_sem);
#endif

			goto end_1;
		}

		spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);
		mdelay(interval);
	} while (loop--);
	goto end_2;
	pr_info("%s,%d\n", __func__, __LINE__);

end_1:
	spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);
end_2:
	return rdy;
}

static unsigned int WaitEncReady(struct ftv310_vpu_enc_t *dev, u32 *core_info,
				 u32 *irq_status, u32 nodenum)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	PDEBUG("%s\n", __func__);

	if (wait_event_interruptible(parentslice->enc_wait_queue,
				     CheckEncIrq(dev, core_info, irq_status,
						 nodenum))) {
		PDEBUG("ENC wait_event_interruptible interrupted\n");
		ReleaseEncoder(dev, core_info, nodenum);
		return -ERESTARTSYS;
	}

	return 0;
}

static int CheckEncAnyIrq(struct ftv310_vpu_enc_t *dev, CORE_WAIT_OUT *out)
{
	int rdy = 0;
	u32 i = 0;

	while ((dev) && (out->irq_num < CORE_MAX)) {
		if (1 ==
		    CheckEncIrqbyPolling(dev, &out->irq_status[out->irq_num])) {
			/* reset the wait condition(s) */
			PDEBUG("check %d irq ready\n", i);
			out->irq_status[out->irq_num] = dev->irq_status;
			out->job_id[out->irq_num] = dev->core_id;
			out->irq_num++;
			rdy = 1;
		}
		i++;
		dev = dev->next;
	}
	return rdy;
}

static unsigned int WaitEncAnyReady(struct ftv310_vpu_enc_t *dev, CORE_WAIT_OUT *out)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	if (wait_event_interruptible(parentslice->enc_wait_queue,
				     CheckEncAnyIrq(dev, out))) {
		PDEBUG("ENC wait_event_interruptible interrupted\n");
		return -ERESTARTSYS;
	}

	return 0;
}

u32 ftv310_vpu_enc_readbandwidth(int sliceidx, int isreadBW)
{
	int i, slicen = get_slicenumber();
	u32 bandwidth = 0;
	struct ftv310_vpu_enc_t *pcore;
	u8 *rregs;
	u8 *wregs;

	if (sliceidx < 0) {
		for (i = 0; i < slicen; i++) {
			pcore = get_encnodes(i, 0);
			while (pcore) {
				rregs = pcore->hwregs +
					FTV310_VPU_FTV310E_REG_BWREAD * 4;
				wregs = pcore->hwregs +
					FTV310_VPU_FTV310E_REG_BWWRITE * 4;
				if (isreadBW)
					bandwidth += ioread32((void *)rregs);
				else
					bandwidth += ioread32((void *)wregs);
				pcore = pcore->next;
			}
		}
	} else {
		pcore = get_encnodes(sliceidx, 0);
		while (pcore) {
			rregs = pcore->hwregs + FTV310_VPU_FTV310E_REG_BWREAD * 4;
			wregs = pcore->hwregs + FTV310_VPU_FTV310E_REG_BWWRITE * 4;
			if (isreadBW)
				bandwidth += ioread32((void *)rregs);
			else
				bandwidth += ioread32((void *)wregs);
			pcore = pcore->next;
		}
	}
	return bandwidth * FTV310E_BURSTWIDTH;
}

static int CheckCoreOccupation(struct ftv310_vpu_enc_t *dev)
{
	int ret = 0;
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	spin_lock_irqsave(&parentslice->enc_owner_lock, flags);
	if (!dev->is_reserved) {
		dev->is_reserved = 1;
		dev->pid = current->tgid;
		ret = 1;
		PDEBUG("%s pid=%d\n", __func__, dev->pid);
	}

	spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);

	return ret;
}

static int GetWorkableCore(struct ftv310_vpu_enc_t *dev, u32 *core_info,
			   u32 *core_info_tmp, u32 nodenum)
{
	int ret = 0;
	u32 i = 0;
	u32 cores;
	u8 core_type = 0;
	u32 required_num = 0;

	cores = *core_info;
	required_num = ((cores >> CORE_INFO_AMOUNT_OFFSET) & 0x7) + 1;
	core_type = (u8)(cores & 0xFF);

	if (*core_info_tmp == 0)
		*core_info_tmp = required_num << CORE_INFO_AMOUNT_OFFSET;
	else
		required_num = (*core_info_tmp >> CORE_INFO_AMOUNT_OFFSET);

	PDEBUG("%s:required_num=%d,core_info=%x\n", __func__, required_num,
	       *core_info);

	if (required_num) {
		/* a valid free Core with specified core type */
		for (i = 0; i < nodenum; i++) {
			if (CheckCoreOccupation(dev)) {
				*core_info_tmp = ((((*core_info_tmp >>
						     CORE_INFO_AMOUNT_OFFSET) -
						    1)
						   << CORE_INFO_AMOUNT_OFFSET) |
						  (*core_info_tmp & 0x0FF));
				*core_info_tmp = (*core_info_tmp | (1 << i));
				if ((*core_info_tmp >>
				     CORE_INFO_AMOUNT_OFFSET) == 0) {
					ret = 1;
					*core_info = (dev->core_id << 16) |
						     (*core_info_tmp & 0xFF);
					required_num = 0;
					break;
				}
			}
			dev = dev->next;
		}
	} else {
		ret = 1;
	}

	PDEBUG("*core_info = %x\n", *core_info);
	return ret;
}

static long ReserveEncoder(struct ftv310_vpu_enc_t *dev, u32 *core_info, u32 nodenum)
{
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);
	u32 core_info_tmp = 0;
	/*If HW resources are shared inter cores,
	 *just make sure only one is using the HW
	 */
	if (resource_shared) {
		if (down_interruptible(&parentslice->enc_core_sem))
			return -ERESTARTSYS;
	}

	/* lock a core that has specified core id*/
	if (wait_event_interruptible(parentslice->enc_hw_queue,
				     GetWorkableCore(dev, core_info,
						     &core_info_tmp,
						     nodenum) != 0))
		return -ERESTARTSYS;

	return 0;
}

static void ReleaseEncoder(struct ftv310_vpu_enc_t *dev, u32 *core_info, u32 nodenum)
{
	unsigned long flags;
	u32 core_num = 0;
	u32 i = 0, core_id;
	u8 core_mapping = 0;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	core_num = ((*core_info >> CORE_INFO_AMOUNT_OFFSET) & 0x7) + 1;

	core_mapping = (u8)(*core_info & 0xFF);

	PDEBUG("%s:core_num=%d,core_mapping=%x\n", __func__, core_num,
	       core_mapping);
	/* release specified core id */
	while (core_mapping) {
		if (core_mapping & 0x1) {
			if (i >= nodenum)
				break;
			core_id = i;
			spin_lock_irqsave(&parentslice->enc_owner_lock, flags);
			PDEBUG("dev[core_id].pid=%d,current->pid= %x app name %s\n",
			       dev->pid, current->pid, current->tgid, current->comm);
			if (dev->is_reserved) {
				dev->pid = -1;
				dev->is_reserved = 0;
				dev->irq_received = 0;
				dev->irq_status = 0;
#ifdef PHY_CONFIG_PM
				dev->reg_corrupt = 0;
#endif
			}
			spin_unlock_irqrestore(&parentslice->enc_owner_lock,
					       flags);

			//wake_up_interruptible_all(&enc_hw_queue);
		}
		core_mapping = core_mapping >> 1;
		i++;
		dev = dev->next;
	}

	wake_up_interruptible_all(&parentslice->enc_hw_queue);

	if (resource_shared)
		up(&parentslice->enc_core_sem);
}

long ftv310_vpu_enc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	unsigned int id, tmp;
	struct ftv310_vpu_enc_t *pcore;
	u32 core_info;
	ftv310_vpu_ioctl_id ioctl_id_par;
	int ret;

	switch (cmd) {
	case FTV310_VPUENC_IOCGHWOFFSET: {
		__get_user(id, (unsigned long long *)arg);

		ioctl_id_par.data = id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return -EFAULT;

		__put_user(pcore->core_cfg.base_addr,
			   (unsigned long long *)arg);
		break;
	}

	case FTV310_VPUENC_IOCGHWIOSIZE: {
		u32 io_size;

		__get_user(id, (unsigned long *)arg);

		ioctl_id_par.data = id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx,
				     ioctl_id_par.ID_PAR.codec_idx);
		if (!pcore)
			return -EFAULT;
		io_size = pcore->core_cfg.iosize;
		__put_user(io_size, (u32 *)arg);
		return 0;
	}
	case FTV310_VPUENC_IOCGSRAMOFFSET:
		__put_user(sram_base, (unsigned long long *)arg);
		break;
	case FTV310_VPUENC_IOCGSRAMEIOSIZE:
		__put_user(sram_size, (unsigned int *)arg);
		break;
	case FTV310_VPUENC_IOCG_CORE_NUM:
		tmp = arg;
		return get_slicecorenum(tmp, FTV310_VPU_CORE_ENC);
	case FTV310_VPUENC_IOCH_ENC_RESERVE: {
		struct nor32_parameter core_info;

		PDEBUG("Reserve ENC Cores\n");
		ret = copy_from_user(&core_info, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_info.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;
		tmp = get_slicecorenum(ioctl_id_par.ID_PAR.node_idx,
				       FTV310_VPU_CORE_ENC);
		ret = ReserveEncoder(pcore, (u32 *)&core_info.data, tmp);
		if (ret == 0) {
			ret = copy_to_user((void *)arg, &core_info,
					   sizeof(struct nor32_parameter));
		}
		return ret;
	}
	case FTV310_VPUENC_IOCH_ENC_RELEASE: {
		struct nor32_parameter core_cfg;

		ret = copy_from_user(&core_cfg, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_cfg.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;
		PDEBUG("Release ENC Core\n");
		tmp = get_slicecorenum(ioctl_id_par.ID_PAR.node_idx,
				       FTV310_VPU_CORE_ENC);
		ReleaseEncoder(pcore, (u32 *)&core_cfg.data, tmp);

		break;
	}

	case FTV310_VPU_IOCG_ENABLE_CORE:
	{
#ifdef PHY_CONFIG_PM
		struct nor32_parameter core_cfg;
		u32 reg_value;

		PDEBUG("Enable ENC Core\n");
		ret = copy_from_user(&core_cfg, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_cfg.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;

		if (pcore->is_reserved == 0)
			return -EPERM;

		if (pcore->reg_corrupt) {
			/*need to re-config HW if exception happen between reserve and enable*/
			pcore->reg_corrupt = 0;
			return -EAGAIN;
		}

		if (down_interruptible(&pcore->core_suspend_sem))
			return -ERESTARTSYS;

		reg_value = (u32)ioread32((void *)(pcore->hwregs + 0x14));
		reg_value |= 0x01;
		iowrite32(reg_value, (void *)(pcore->hwregs + 0x14));
#endif
		break;
	}

	case FTV310_VPUENC_IOCG_CORE_WAIT: {
		struct nor32_parameter core_cfg;

		ret = copy_from_user(&core_cfg, (void *)arg,
				     sizeof(struct nor32_parameter));
		if (ret)
			return ret;
		ioctl_id_par.data = core_cfg.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;
#ifdef PHY_CONFIG_PM
		if (pcore->is_reserved == 0)
			return -EPERM;
#endif
		tmp = get_slicecorenum(ioctl_id_par.ID_PAR.node_idx,
				       FTV310_VPU_CORE_ENC);

		core_info = core_cfg.data;
		tmp = WaitEncReady(pcore, &core_info, (u32 *)&core_cfg.data,
				   tmp);
		if (tmp == 0) {
			ret = copy_to_user((void *)arg, &core_cfg,
					   sizeof(struct nor32_parameter));
			return core_info; //return core_id
		}
		ret = copy_to_user((void *)arg, &core_cfg,
				   sizeof(struct nor32_parameter));
		return -1;

		break;
	}

	case FTV310_VPUENC_IOCG_CORE_INFO: {
		SUBSYS_CORE_INFO in_data;

		ret = copy_from_user(&in_data, (void *)arg,
				     sizeof(SUBSYS_CORE_INFO));
		if (ret)
			return ret;
		ioctl_id_par.data = in_data.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx, 0);
		if (!pcore)
			return -EFAULT;

		ret = copy_to_user((void *)arg, &pcore->core_info,
				   sizeof(SUBSYS_CORE_INFO));
		break;
	}
	case FTV310_VPUENC_IOCG_ANYCORE_WAIT: {
		CORE_WAIT_OUT out;

		ret = copy_from_user((void *)(&out), (void *)arg,
				     sizeof(CORE_WAIT_OUT));
		if (ret)
			return ret;
		ioctl_id_par.data = out.id;
		pcore = get_encnodes(ioctl_id_par.ID_PAR.node_idx,
				     0); /*from list header*/
		tmp = WaitEncAnyReady(pcore, &out);
		if (tmp == 0) {
			ret = copy_to_user((void *)arg, &out,
					   sizeof(CORE_WAIT_OUT));
			return ret;
		} else {
			return -1;
		}

		break;
	}
	}
	return 0;
}

int ftv310_vpu_enc_release(void)
{
	struct slice_info *parentslice;
	int i, slicen = get_slicenumber();
	struct ftv310_vpu_enc_t *dev;
	unsigned long flags;

	for (i = 0; i < slicen; i++) {
		dev = get_encnodes(i, 0);
		if (!dev)
			continue;
		parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);
		while (dev) {
			spin_lock_irqsave(&parentslice->enc_owner_lock, flags);
			if (dev->is_reserved == 1 && dev->pid == current->tgid) {
				dev->pid = -1;
				dev->is_reserved = 0;
				dev->irq_received = 0;
				dev->irq_status = 0;
				PDEBUG("release reserved core\n");
			}
			spin_unlock_irqrestore(&parentslice->enc_owner_lock,
					       flags);
			dev = dev->next;
		}
		wake_up_interruptible_all(&parentslice->enc_hw_queue);
		if (resource_shared)
			up(&parentslice->enc_core_sem);
	}
	return 0;
}

int ftv310_vpu_enc_init(void)
{
	sram_base = 0;
	sram_size = 0;
	ftv310_vpu_enc_major = 0;
	resource_shared = 0;
	bencprobed = 0;
	return 0;
}

int ftv310_vpu_enc_probe(dtbnode *pnode, int useirq, int loop,
		    struct ftv310_vpu_enc_t *penccore)
{
	int result = 0;
	struct ftv310_vpu_enc_t *pcore = NULL;
	int i, k;

	if (loop == 0) {
#ifndef USE_DTB_PROBE /*simulate and compatible with old code*/
		for (i = 0; i < ARRAY_SIZE(core_array); i++) {
			pcore = vmalloc(sizeof(*pcore));
			if (!pcore)
				break;

			memset(pcore, 0, sizeof(struct ftv310_vpu_enc_t));
			pcore->core_cfg.base_addr = core_array[i].base_addr;
			pcore->core_cfg.iosize = core_array[i].iosize;
			pcore->core_cfg.sliceidx = core_array[i].sliceidx;
			for (k = 0; k < 4; k++)
				pcore->irqlist[k] = -1;
			pcore->irqlist[0] = core_array[i].irq;

			pcore->core_info.type_info |= 1 << CORE_VCE;
			pcore->core_info.offset[CORE_VCE] = 0;
			pcore->core_info.regSize[CORE_VCE] =
				pcore->core_cfg.iosize;
			pcore->core_info.irq[CORE_VCE] = pcore->irqlist[0];

			add_encnode(pcore->core_cfg.sliceidx, pcore);
#ifdef PHY_CONFIG_PM
			sema_init(&pcore->core_suspend_sem, 1);
#endif
		}
#else /*USE_DTB_PROBE*/
		{
			pcore = vmalloc(sizeof(*pcore));
			if (!pcore)
				return -ENOMEM;

			memset(pcore, 0, sizeof(struct ftv310_vpu_enc_t));
			pcore->core_cfg.base_addr = pnode->ioaddr;
			pcore->core_cfg.iosize = pnode->iosize;
			pcore->core_cfg.sliceidx = pnode->sliceidx;
			for (i = 0; i < 4; i++)
				pcore->irqlist[i] = -1;
			pcore->irqlist[0] = pnode->irq[0];

			pcore->core_info.type_info |= 1 << CORE_VCE;
			pcore->core_info.offset[CORE_VCE] = 0;
			pcore->core_info.regSize[CORE_VCE] =
				pcore->core_cfg.iosize;
			pcore->core_info.irq[CORE_VCE] = pcore->irqlist[0];

			add_encnode(pnode->sliceidx, pcore);
		}
#endif /*USE_DTB_PROBE*/
	} else {
		result = ReserveIO(penccore);
		if (result < 0) {
			pr_err("ftv310e: reserve reg 0x%llx-0x%lx fail\n",
			       penccore->core_cfg.base_addr,
			       pcore->core_info.regSize[CORE_VCE]);
			return result;
		}

		ResetAsic(penccore); /* reset hardware */

		if (useirq && penccore->irqlist[0] > 0) {
			result = request_irq(penccore->irqlist[0],
					     ftv310_vpu_enc_isr, IRQF_SHARED,
					     "ftv310e", (void *)penccore);
			if (result < 0) {
				pr_err("ftv310e: request IRQ <%d> fail\n",
				       penccore->irqlist[0]);
				ReleaseIO(penccore);
				return -ENODEV;
			}
		}
	}
	pr_debug("ftv310e: module inserted. Major <%d>\n", ftv310_vpu_enc_major);

	return 0;
}

void ftv310_vpu_enc_cleanup(void)
{
	int i, k, slicen = get_slicenumber();
	struct ftv310_vpu_enc_t *pcore, *pnext;

	for (i = 0; i < slicen; i++) {
		pcore = get_encnodes(i, 0);
		while (pcore) {
			u32 hwId = pcore->hw_id;
			u32 majorId = (hwId & 0x0000FF00) >> 8;
			u32 wClr = (majorId >= 0x61) ? (0x1FD) : (0);

			pnext = pcore->next;
			iowrite32(0, (void *)(pcore->hwregs +
					      0x14)); /* disable HW */
			iowrite32(wClr, (void *)(pcore->hwregs +
						 0x04)); /* clear enc IRQ */

			/* free the encoder IRQ */
			for (k = 0; k < 4; k++)
				if (pcore->irqlist[k] > 0)
					free_irq(pcore->irqlist[k],
						 (void *)pcore);
			ReleaseIO(pcore);
			vfree(pcore);
			pcore = pnext;
		}
	}
	bencprobed = 0;
	pr_info("ftv310_vpu_enc: module removed\n");
}

static int ReserveIO(struct ftv310_vpu_enc_t *pcore)
{
	u32 hwid;

	if (!request_mem_region(pcore->core_cfg.base_addr,
				pcore->core_info.regSize[CORE_VCE],
				"ftv310e")) {
		pr_info("ftv310_vpu_enc: failed to reserve HW regs\n");
		return -1;
	}

	pcore->hwregs = (u8 *)ioremap(pcore->core_cfg.base_addr,
				      pcore->core_info.regSize[CORE_VCE]);
	if (!pcore->hwregs) {
		pr_info("ftv310_vpu_enc: failed to ioremap HW regs\n");
		release_mem_region(pcore->core_cfg.base_addr,
				   pcore->core_info.regSize[CORE_VCE]);
		return -1;
	}

	/*read hwid and check validness and store it*/
	hwid = (u32)ioread32((void *)pcore->hwregs);
	pr_info("hwid=0x%08x, reg size %ld\n", hwid, pcore->core_info.regSize[CORE_VCE]);

	/* check for encoder HW ID */
	if (((((hwid >> 16) & 0xFFFF) != ((ENC_HW_ID1 >> 16) & 0xFFFF))) &&
	    ((((hwid >> 16) & 0xFFFF) != ((ENC_HW_ID2 >> 16) & 0xFFFF))) &&
	    ((((hwid >> 16) & 0xFFFF) != ((ENC_HW_ID3 >> 16) & 0xFFFF)))) {
		pr_info("ftv310_vpu_enc: HW not found at %llx\n",
			pcore->core_cfg.base_addr);
		ReleaseIO(pcore);
		return -1;
	}
	pcore->hw_id = hwid;

	pr_info("ftv310_vpu_enc: HW at base <%llx> with ID <0x%08x>\n",
		pcore->core_cfg.base_addr, hwid);

	return 0;
}

static void ReleaseIO(struct ftv310_vpu_enc_t *pcore)
{
	if (pcore->hwregs)
		iounmap((void *)pcore->hwregs);
	release_mem_region(pcore->core_cfg.base_addr,
			   pcore->core_info.regSize[CORE_VCE]);
}

#if KERNEL_VERSION(2, 6, 18) > LINUX_VERSION_CODE
static irqreturn_t ftv310_vpu_enc_isr(int irq, void *dev_id, struct pt_regs *regs)
#else
static irqreturn_t ftv310_vpu_enc_isr(int irq, void *dev_id)
#endif
{
	unsigned int handled = 0;
	struct ftv310_vpu_enc_t *dev = (struct ftv310_vpu_enc_t *)dev_id;
	u32 irq_status;
	unsigned long flags;
	struct slice_info *parentslice = getparentslice(dev, FTV310_VPU_CORE_ENC);

	/* If core is not reserved by any user,
	 * but irq is received, just ignore it
	 */
	pr_info("ftv310_vpu_enc_isr:received IRQ!\n");
	spin_lock_irqsave(&parentslice->enc_owner_lock, flags);
	if (!dev->is_reserved) {
		pr_info("ftv310_vpu_enc_isr:received IRQ but core is not reserved!\n");
		irq_status = (u32)ioread32((void *)(dev->hwregs + 0x04));
		if (irq_status & 0x01) {
			/* clear all IRQ bits. (hwId >= 0x80006100) means IRQ is
			 *cleared by writing 1
			 */
			u32 hwId = ioread32((void *)dev->hwregs);
			u32 majorId = (hwId & 0x0000FF00) >> 8;
			u32 wClr = (majorId >= 0x61) ? irq_status :
						       (irq_status & (~0x1FD));

			/* Disable HW when buffer over-flow happen
			 * HW behavior changed in over-flow
			 * in-pass, HW cleanup HWIF_ENC_E auto
			 * new version:  ask SW cleanup HWIF_ENC_E
			 * when buffer over-flow
			 */
			if (irq_status & 0x20)
				iowrite32(0, (void *)(dev->hwregs + 0x14));
			iowrite32(wClr, (void *)(dev->hwregs + 0x04));
		}
		spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);
		return IRQ_HANDLED;
	}
	spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);

	irq_status = (u32)ioread32((void *)(dev->hwregs + 0x04));
	pr_info("irq_status of %d is:%x\n", dev->core_id, irq_status);
	if (irq_status & 0x01) {
		/* clear all IRQ bits. (hwId >= 0x80006100) means
		 * IRQ is cleared by writing 1
		 */
		u32 hwId = ioread32((void *)dev->hwregs);
		u32 majorId = (hwId & 0x0000FF00) >> 8;
		u32 wClr = (majorId >= 0x61) ? irq_status :
					       (irq_status & (~0x1FD));

		if (irq_status & 0x20)
			iowrite32(0, (void *)(dev->hwregs + 0x14));
		iowrite32(wClr, (void *)(dev->hwregs + 0x04));
		spin_lock_irqsave(&parentslice->enc_owner_lock, flags);
		dev->irq_received = 1;
		dev->irq_status = irq_status & (~0x01);
		spin_unlock_irqrestore(&parentslice->enc_owner_lock, flags);

#ifdef PHY_CONFIG_PM
		//if frame_rdy IRQ is received, then HW will not be used any more.
		if (irq_status & ASIC_STATUS_FRAME_READY)
			up(&dev->core_suspend_sem);
#endif
		wake_up_interruptible_all(&parentslice->enc_wait_queue);
		handled++;
	}
	if (!handled)
		pr_info("IRQ received, but not ftv310_vpu enc's!\n");

	return IRQ_HANDLED;
}

static void ResetAsic(struct ftv310_vpu_enc_t *dev)
{
#ifndef SIMICS_TEST
	int i;

	iowrite32(0, (void *)(dev->hwregs + 0x14));
	for (i = 4; i < dev->core_info.regSize[CORE_VCE]; i += 4)
		iowrite32(0, (void *)(dev->hwregs + i));
#endif
}
