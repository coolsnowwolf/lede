/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _CACHE_H_
#define _CACHE_H_
#include <linux/ioctl.h>
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu.h"

#undef PDEBUG /* undef it, just in case */
#ifdef CACHE_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG printk
#else
/* This one for user space */
#define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

long ftv310_vpu_cache_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

long CacheReadRegs(struct cache_dev_t *dev, struct core_desc *core);
long CacheWriteRegs(struct cache_dev_t *dev, struct core_desc *core);

int cache_init(void);
int cache_probe(dtbnode *pnode, int useirq, int loop,
		struct cache_dev_t *cachecore);
void cache_cleanup(void);
int cache_open(struct inode *inode, struct file *filp);
int cache_release(struct file *filp);

#endif /* !_CACHE_H_ */
