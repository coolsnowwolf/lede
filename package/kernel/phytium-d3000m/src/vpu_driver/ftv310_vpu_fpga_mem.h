/* SPDX-License-Identifier: GPL-2.0 */
/*
 * header file for pcie ftv310_vpu fpga.
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License version 2 for more details.
 *
 * You may obtain a copy of the GNU General Public License
 * Version 2 at the following locations:
 * https://opensource.org/licenses/gpl-2.0.php
 */

#ifndef __FTV310_VPU_FPGA_MEM__
#define __FTV310_VPU_FPGA_MEM__

#include "ftv310_vpu_priv.h"

/* Total DDR memory size */
#define DDR_SIZE    800
/* ftv310_vpu linear memory chunk */
struct hlinc {
	unsigned long bus_address;
	u16 chunks_reserved;
	//const struct file *filp; /* Client that allocated this chunk */
	u16 filp; /* as a flag only */
};

struct ftv310_vpu_fpga_t {
	/* PCI base register address of HW cores (Hardware address) */
	unsigned long reg_base;
	u32 reg_len; /* core registers address Length */

	/* PCI base register address of on board DDR (Hardware address) */
	unsigned long ddr_base;
	u32 ddr_len; /* on board DDR address Length */

	unsigned long alloc_size;
	unsigned long alloc_base;

	/* reserved mmu and vcmd pool buffer */
	unsigned long mmu_base;
	unsigned long vcmd_base[2];
	void *virt_pool_addr;

	/* user space SW will subtract HLINA_TRANSL_OFFSET from the bus address
	 * and decoder HW will use the result as the address translated base
	 * address. The SW needs the original host memory bus address for memory
	 * mapping to virtual address.
	 */
	unsigned long addr_transl;
	void * virt_addr;

	/* memory part */
	spinlock_t mem_lock;
	struct hlinc *hlina_chunks;
	size_t chunks;
};

/* API */

#define ftv310_vpu_fpga_memalloc ftv310_vpu_dev_memalloc
#define ftv310_vpu_fpga_memfree ftv310_vpu_dev_memfree

int ftv310_vpu_fpga_memalloc(struct ftv310_vpu_mem_handle *phandle, unsigned int size);
int ftv310_vpu_fpga_memfree(struct ftv310_vpu_mem_handle *phandle);
int ftv310_vpu_fpga_meminit(unsigned long ddr_base);
int ftv310_vpu_phytium_meminit(struct platform_device *pdev, unsigned long ddr_base, unsigned long size);
void ftv310_vpu_phytium_mem_release(void);
void *ftv310_vpu_phytium_map_physical(unsigned long alloc_base, unsigned int alloc_size);
int ftv310_vpu_fpga_memrelease(void);
#endif /* __FTV310_VPU_FPGA_MEM__ */
