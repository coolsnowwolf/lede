/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 decoder hardware driver header file.
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

#ifndef _FTV310_VPUDEC_H_
#define _FTV310_VPUDEC_H_
#include <linux/ioctl.h>
#include <linux/types.h>
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu.h"

#undef PDEBUG
#ifdef FTV310_VPUDEC_DEBUG
#ifdef __KERNEL__
#define PDEBUG(fmt, args...) pr_info("ftv310_vpu_dec: " fmt, ##args)
#else
#define PDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define PDEBUG(fmt, args...)
#endif

int ftv310_vpu_dec_release(struct file *filp);
int ftv310_vpu_dec_init(void);
int ftv310_vpu_dec_probe(dtbnode *pnode, int useirq, int loop,
		    struct ftv310_vpu_dec_t *pdeccore);
void ftv310_vpu_dec_cleanup(void);
long ftv310_vpu_dec_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
u32 *ftv310_vpu_dec_getRegAddr(u32 coreid, u32 regid);
int ftv310_vpu_dec_open(struct inode *inode, struct file *filp);
u32 ftv310_vpu_dec_readbandwidth(int sliceidx, int isreadBW);
int dec_pm_suspend(void *_dev);
int dec_pm_resume(void *_dev);

#endif /* !_FTV310_VPUDEC_H_ */
