// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 axife controller hardware driver.
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
#include <linux/version.h>
#include <linux/vmalloc.h>

#include "ftv310_vpu_axife.h"
#include "ftv310_vpu_device.h"
#include "ipoffset/axife_offset.h"

static int axifeprobed;
long AxifeReadRegs(struct axife_t *dev, struct core_desc *core)
{
	u32 i;
	long ret = 0;

	i = core->reg_id;
	/* user has to know exactly what they are asking for */
	//if (core->size != (FTV310_VPU_FTV310D_REGS * 4))
	//	return -EFAULT;

	/* read specific registers from hardware */
	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
		dev->dec_regs[i] = ioread32((void *)dev->hwregs + i * 4);

	/* put registers to user space*/
	ret = copy_to_user(core->regs, dev->dec_regs + core->reg_id,
			   core->size);
	if (ret) {
		pr_debug("copy_to_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	return 0;
}

long AxifeWriteRegs(struct axife_t *dev, struct core_desc *core)
{
	u32 i;
	long ret = 0;

	i = core->reg_id;
	ret = copy_from_user(dev->dec_regs + core->reg_id,
			     core->regs, core->size);
	if (ret) {
		pr_debug("copy_from_user failed, returned %li\n", ret);
		return -EFAULT;
	}
	for (i = core->reg_id; i < core->reg_id + core->size / 4; i++)
		iowrite32(dev->dec_regs[i], (void *)dev->hwregs + i * 4);

	return 0;
}

static void AXIFEEnable(volatile unsigned char *hwregs)
{
	if (!hwregs)
		return;

	//AXI FE pass through
	iowrite32(0x0, (void *)(hwregs + 0x2C));
	pr_info("AXI FE: 0x2C = 0x%x\n",
		ioread32((void *)(hwregs + 0x2C)));
	iowrite32(0x2, (void *)(hwregs + 0x28));
	pr_info("AXI FE: 0x28 = 0x%x\n",
		ioread32((void *)(hwregs + 0x28)));
}

int ftv310_vpu_axife_probe(dtbnode *pnode, int loop, struct axife_t *axifecore)
{
	struct axife_t *paxife;

	if (loop == 0) {
#ifndef USE_DTB_PROBE
		int i;

		for (i = 0; i < ARRAY_SIZE(axifecores); i++) {
			paxife = vmalloc(sizeof(*paxife));
			if (!paxife)
				break;
			paxife->core_cfg = axifecores[i];
			add_axifenode(axifecores[i].sliceidx, paxife);
		}
#else //ndef USE_DTB_PROBE
		{
			paxife = vmalloc(sizeof(*paxife));
			if (!paxife)
				return -ENOMEM;
			paxife->core_cfg.axifecorebase = pnode->ioaddr;
			paxife->core_cfg.iosize = pnode->iosize;
			paxife->core_cfg.sliceidx = pnode->sliceidx;
			paxife->core_cfg.parentaddr = pnode->parentaddr;

			add_axifenode(pnode->sliceidx, paxife);
		}
#endif
	} else {
		if (!request_mem_region(axifecore->core_cfg.axifecorebase,
					axifecore->core_cfg.iosize,
					"ftv310_vpu_axife")) {
			pr_err("axife: HW regs busy\n");
			return -ENODEV;
		}
		axifecore->hwregs =
			(u8 *)ioremap(axifecore->core_cfg.axifecorebase,
				      axifecore->core_cfg.iosize);
		if (!axifecore->hwregs) {
			release_mem_region(axifecore->core_cfg.axifecorebase,
					   axifecore->core_cfg.iosize);
			pr_err("axife: failed to map HW regs\n");
			return -ENODEV;
		}
		axifecore->dec_regs =
			vmalloc(axifecore->core_cfg.iosize);
		if (!axifecore->dec_regs)
			return -ENOMEM;

		AXIFEEnable(axifecore->hwregs);
	}

	return 0;
}

void __exit ftv310_vpu_axife_cleanup(void)
{
	int i, slicen = get_slicenumber();
	struct axife_t *dev, *pp;

	for (i = 0; i < slicen; i++) {
		dev = get_axifenodes(i, 0);
		while (dev) {
			if (dev->hwregs)
				release_mem_region(dev->core_cfg.axifecorebase,
						   dev->core_cfg.iosize);
			if (dev->dec_regs)
				vfree(dev->dec_regs);
			pp = dev->next;
			vfree(dev);
			dev = pp;
		}
	}
	axifeprobed = 0;
}

int __init ftv310_vpu_axife_init(void)
{
	axifeprobed = 0;
	return 0;
}
