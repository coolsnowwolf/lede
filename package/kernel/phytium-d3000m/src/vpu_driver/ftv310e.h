/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 encoder hardware driver header file.
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

#ifndef _FTV310E_H_
#define _FTV310E_H_

#include "ftv310_vpu_priv.h"
#include "ftv310_vpu.h"
/*
 * Macros to help debugging
 */

#undef PDEBUG /* undef it, just in case */
#ifdef FTV310E_DEBUG
#ifdef __KERNEL__
/* This one if debugging is on, and kernel space */
#define PDEBUG(fmt, args...) pr_info("hmp4e: " fmt, ##args)
#else
/* This one for user space */
#define PDEBUG(fmt, args...) printf(__FILE__ ":%d: " fmt, __LINE__, ##args)
#endif
#else
#define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#define IRQF_DISABLED 0x00000020

#define ENC_HW_ID1 0x48320100
#define ENC_HW_ID2 0x80006000
#define ENC_HW_ID3 0x90009100
#define CORE_INFO_MODE_OFFSET 31
#define CORE_INFO_AMOUNT_OFFSET 28

#define ASIC_STATUS_SEGMENT_READY 0x1000
#define ASIC_STATUS_FUSE_ERROR 0x200
#define ASIC_STATUS_SLICE_READY 0x100
#define ASIC_STATUS_LINE_BUFFER_DONE 0x080 /* low latency */
#define ASIC_STATUS_HW_TIMEOUT 0x040
#define ASIC_STATUS_BUFF_FULL 0x020
#define ASIC_STATUS_HW_RESET 0x010
#define ASIC_STATUS_ERROR 0x008
#define ASIC_STATUS_FRAME_READY 0x004
#define ASIC_IRQ_LINE 0x001

#define ASIC_STATUS_ALL                                                        \
	(ASIC_STATUS_SEGMENT_READY | ASIC_STATUS_FUSE_ERROR |                  \
	 ASIC_STATUS_SLICE_READY | ASIC_STATUS_LINE_BUFFER_DONE |              \
	 ASIC_STATUS_HW_TIMEOUT | ASIC_STATUS_BUFF_FULL |                      \
	 ASIC_STATUS_HW_RESET | ASIC_STATUS_ERROR | ASIC_STATUS_FRAME_READY)

long ftv310_vpu_enc_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);
int ftv310_vpu_enc_init(void);
int ftv310_vpu_enc_probe(dtbnode *pnode, int useirq, int loop,
		    struct ftv310_vpu_enc_t *penccore);
void ftv310_vpu_enc_cleanup(void);
u32 *ftv310_vpu_enc_getRegAddr(u32 coreid, u32 regid);
u32 ftv310_vpu_enc_readbandwidth(int sliceidx, int isreadBW);
int ftv310_vpu_enc_release(void);

#endif /* !_FTV310E_H_ */
