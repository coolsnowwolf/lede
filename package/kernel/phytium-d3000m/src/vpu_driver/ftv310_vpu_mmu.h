/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 mmu driver header file.
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

#ifndef _FTV310_VPUMMU_H_
#define _FTV310_VPUMMU_H_

#include "ftv310_vpu_priv.h"

#define REGION_PRIVATE_START 0x1000u
#define REGION_PRIVATE_END 0x20000000u

#define REGION_PRIVATE_MMU_START 0x1000u
#define REGION_PRIVATE_MMU_END 0x20000000u

enum MMUStatus {
	MMU_STATUS_OK = 0,

	MMU_STATUS_FALSE = -1,
	MMU_STATUS_INVALID_ARGUMENT = -2,
	MMU_STATUS_INVALID_OBJECT = -3,
	MMU_STATUS_OUT_OF_MEMORY = -4,
	MMU_STATUS_NOT_FOUND = -19,
};
enum MMUProbeStage {
	PGTBL_RESOURCE = 0,
	HWREG_RESOURCE = 1,
	ALL_RESOURCE = 2, /* probe page table and reserve hw reg both */
};
struct addr_desc {
	void *virtual_address; /* buffer virtual address */
	unsigned long bus_address; /* buffer physical address */
	unsigned long mmu_bus_address;
	unsigned long size; /* physical size */
	unsigned int id; /*SLICE, TYPE*/
	int handle;
};

struct mmu_addr_desc {
    /* same with addr_desc */
	void *virtual_address; /* buffer virtual address */
	unsigned long bus_address; /* buffer physical address */
	unsigned long mmu_bus_address;
	unsigned long size; /* physical size */
	unsigned int id; /*SLICE, TYPE*/
	int handle;

    /* extend for imported buffer */
	struct page **pages;
};

/* Init MMU, should be called in driver init function. */
int ftv310_vpu_MMUprobe(dtbnode *pnode, enum MMUProbeStage loop, struct mmu_t *pmmu,
		   struct platform_device *pdev, unsigned long ddr_base, struct mmu_core_cfg *cfg);

/* Clean up all data in MMU */
void ftv310_vpu_MMUCleanup(void);
/* The function should be called in driver realease function */
void ftv310_vpu_MMURelease(void *filp);
/* Memmap the address to ftv310_vpu mmu */
void ftv310_vpu_mmu_map(struct mmu_addr_desc *addr, int valid_bus_addr);
long ftv310_vpu_MMUIoctl(unsigned int cmd, void *filp, unsigned long arg);
int mmu_pm_resume(u32 sliceindex, int type);

enum MMURegion {
	MMU_REGION_PRIVATE,
	MMU_REGION_PUB,

	MMU_REGION_COUNT
};

struct MMUNode {
	void *buf_virtual_address;
	unsigned int buf_bus_address; /* used in kernel map mode */
	int mtlb_start;
	int stlb_start;
	int mtlb_end;
	int stlb_end;
	unsigned int page_count;
	int process_id;
	struct file *filp;

	struct MMUNode *next;
	struct MMUNode *prev;
};

struct MMUDDRRegion {
	unsigned long long physical_address;
	unsigned long long virtual_address;
	unsigned int page_count;

	void *node_mutex;
	struct MMUNode *simple_map_head;
	struct MMUNode *simple_map_tail;
	struct MMUNode *free_map_head;
	struct MMUNode *map_head;
	struct MMUNode *free_map_tail;
	struct MMUNode *map_tail;
};

struct MMU {
	struct MMUDDRRegion region[MMU_REGION_COUNT];
};
long ftv310_vpu_MMUClear(struct mmu_addr_desc *addr, struct MMUDDRRegion *region, struct mmu_tbl_info *pg_tbl);

#endif //#ifndef _FTV310_VPUMMU_H_
