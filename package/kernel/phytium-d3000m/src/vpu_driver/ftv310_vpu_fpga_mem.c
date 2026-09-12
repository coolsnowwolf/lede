// SPDX-License-Identifier: GPL-2.0
/*
 * ftv310 driver on pcie fpga.
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

#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/io.h>
#include <linux/list.h>
#include "ftv310_vpu_fpga_mem.h"

#ifndef HLINA_START_ADDRESS
#define HLINA_START_ADDRESS 0x02000000
#endif

#ifndef HLINA_SIZE
#define HLINA_SIZE 96
#endif

#ifndef HLINA_TRANSL_OFFSET
#define HLINA_TRANSL_OFFSET 0x0
#endif

#ifdef HAS_MMU
extern unsigned long get_total_mmu_pgtbl_buf_size(u32 slice_cnt);
#endif

#ifdef HAS_VCMD
extern unsigned long get_total_vcmd_pool_size(void);
extern unsigned long get_vcmd_pool_size(void);
#endif

static unsigned long get_ftv310_vpu_fpgamem_offset(void);

static struct list_head vram_list;
static struct mutex vram_mutex;
unsigned long vram_used_size = 0;

extern long mem_start;

/* the size of chunk in MEMALLOC_DYNAMIC */
#define CHUNK_SIZE (PAGE_SIZE * 4)

struct ftv310_vpu_fpga_t g_fpga = {
	.alloc_size = HLINA_SIZE,
	.alloc_base = HLINA_START_ADDRESS,
	.addr_transl = 0,
};

int ftv310_vpu_fpga_memrelease(void)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int i = 0;

	for (i = 0; i < fpga->chunks; i++) {
		spin_lock(&fpga->mem_lock);
		if (fpga->hlina_chunks[i].filp != 0) {
			pr_warn("memalloc: Found unfreed memory at release time!\n");

			fpga->hlina_chunks[i].filp = 0;
			fpga->hlina_chunks[i].chunks_reserved = 0;
		}
		spin_unlock(&fpga->mem_lock);
	}
	vfree(fpga->hlina_chunks);
	pr_info("dev closed\n");
	return 0;
}

/* Cycle through the buffers we have, give the first free one */
int ftv310_vpu_fpga_memalloc(struct ftv310_vpu_mem_handle *phandle, unsigned int size)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int i = 0;
	int j = 0;
	unsigned int skip_chunks = 0;
	unsigned long busaddr;

	/* calculate how many chunks we need; round up to chunk boundary */
	unsigned int alloc_chunks = (size + CHUNK_SIZE - 1) / CHUNK_SIZE;

	busaddr = 0;

	/* run through the chunk table */
	for (i = 0; i < fpga->chunks;) {
		skip_chunks = 0;
		/* if this chunk is available */
		if (!fpga->hlina_chunks[i].chunks_reserved) {
			/* check that there is enough memory left */
			if (i + alloc_chunks > fpga->chunks)
				break;

			/* check that there is enough consecutive chunks */
			for (j = i; j < i + alloc_chunks; j++) {
				if (fpga->hlina_chunks[j].chunks_reserved) {
					skip_chunks = 1;
					/* skip the used chunks */
					i = j + fpga->hlina_chunks[j]
							.chunks_reserved;
					break;
				}
			}

			/* if enough free memory found */
			if (!skip_chunks) {
				busaddr = fpga->hlina_chunks[i].bus_address;
				fpga->hlina_chunks[i].filp = 0x55aa;
				fpga->hlina_chunks[i].chunks_reserved =
					alloc_chunks;
				break;
			}
		} else {
			/* skip the used chunks */
			i += fpga->hlina_chunks[i].chunks_reserved;
		}
	}

	if (busaddr == 0)
		return -EFAULT;

	phandle->sliceidx = 0; /* no slice in fpga now */
	phandle->size = alloc_chunks * (CHUNK_SIZE);
	phandle->paddr = busaddr;
	phandle->mem_base = fpga->ddr_base;

	/* FIXME: */
	phandle->vaddr = fpga->virt_addr + busaddr - fpga->alloc_base;
	return 0;
}

/**
 * Free a buffer based on bus address
 */
int ftv310_vpu_fpga_memfree(struct ftv310_vpu_mem_handle *phandle)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int i = 0;

	for (i = 0; i < fpga->chunks; i++) {
		/* user space SW has stored the translated bus address, add
		 * addr_transl to translate back to our address space.
		 */
		if (fpga->hlina_chunks[i].bus_address == phandle->paddr) {
			fpga->hlina_chunks[i].filp = 0;
			fpga->hlina_chunks[i].chunks_reserved = 0;
			break;
		}
	}
	if (i == fpga->chunks) {
		pr_warn("memalloc: Owner mismatch while freeing memory!\n");
		return -1;
	}

	return 0;
}

/**
 * Reset "used" status
 */
static int ftv310_vpu_fpga_memreset(void)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int i = 0;
	int result;
	unsigned long ba = fpga->alloc_base;

	pr_info("memalloc: Linear Memory Allocator\n");
	pr_info("memalloc: Linear memory base = %p\n",
		(void *)fpga->alloc_base);

	fpga->chunks = (fpga->alloc_size * 1024 * 1024) / CHUNK_SIZE;
	pr_info("memalloc: Total size %lu MB; %d chunks of size %lu\n",
		fpga->alloc_size, (int)fpga->chunks, CHUNK_SIZE);

	fpga->hlina_chunks = vmalloc(fpga->chunks * sizeof(struct hlinc));

	for (i = 0; i < fpga->chunks; i++) {
		fpga->hlina_chunks[i].bus_address = ba;
		fpga->hlina_chunks[i].filp = 0;
		fpga->hlina_chunks[i].chunks_reserved = 0;
		ba += CHUNK_SIZE;
	}
	return 0;

	if (fpga->hlina_chunks)
		vfree(fpga->hlina_chunks);

	return result;
}

/**
 * init ftv310_vpu fpga pcie board
 *
 * \return -1 fail;
 * \return  0 success;
 */
int ftv310_vpu_fpga_meminit(unsigned long ddr_base)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int ret;
	unsigned long offset;

	offset = get_ftv310_vpu_fpgamem_offset();
	fpga->ddr_base = ddr_base;
	fpga->alloc_base = fpga->ddr_base + offset;
	fpga->alloc_size = DDR_SIZE - offset / (0x100000);
	fpga->addr_transl = fpga->ddr_base;

	ret = ftv310_vpu_fpga_memreset();
	if (ret != 0)
		goto out;

	return 0;

out:
	return -1;
}

void *ftv310_vpu_phytium_map_physical(unsigned long alloc_base, unsigned int alloc_size)
{
	unsigned long pfn = PHYS_PFN(alloc_base);
	uint32_t numPages = 0, i = 0;
	struct page ** pages = NULL;
	struct page *page;
	pgprot_t pgprot;
	void * virt;

	if (pfn_valid(pfn)) {
		numPages = ((alloc_size + (alloc_base & ~PAGE_MASK)) + PAGE_SIZE - 1) >> PAGE_SHIFT;
		pages = kmalloc_array(numPages, sizeof(struct page *), GFP_KERNEL);
		if (pages == NULL)
			goto error;

		page = pfn_to_page(pfn);
		for (i = 0; i < numPages; i++)
			pages[i] = page + i;

		pgprot = pgprot_writecombine(PAGE_KERNEL);
		//pgprot = PAGE_KERNEL;
		virt = vmap(pages, numPages, 0, pgprot);
		if(virt == NULL)
			goto error;

		kfree(pages);
	} else {
		virt = ioremap_wc(alloc_base, alloc_size);
		if(virt == NULL)
			goto error;
	}

	return virt;
error:
	return NULL;
}

int ftv310_vpu_phytium_meminit(struct platform_device *pdev, unsigned long ddr_base, unsigned long size)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int ret;
	unsigned long offset;

	offset = get_ftv310_vpu_fpgamem_offset();

	if (size >= offset) {
#ifdef HAS_MMU
		fpga->mmu_base = ddr_base;
		fpga->vcmd_base[0] = fpga->mmu_base + get_total_mmu_pgtbl_buf_size(1);
#else
		fpga->mmu_base = 0;
		fpga->vcmd_base[0] = ddr_base;
#endif
		fpga->vcmd_base[1] = fpga->vcmd_base[0] + get_vcmd_pool_size();
		fpga->virt_pool_addr = ioremap_wc(ddr_base, offset);

		pr_info("ddr_base:0x%lx, mmu_base:0x%lx, vcmd_base:[0x%lx, 0x%lx], virt_pool:0x%lx, offset:0x%lx\n",
			ddr_base, fpga->mmu_base, fpga->vcmd_base[0], fpga->vcmd_base[1], (unsigned long)fpga->virt_pool_addr, offset);
		
		vram_used_size += offset;
	}

	INIT_LIST_HEAD(&vram_list);
	mutex_init(&vram_mutex);

	fpga->ddr_base = ddr_base;
	fpga->alloc_base = fpga->ddr_base + offset;
	fpga->alloc_size = (size - offset) / (0x100000);
	fpga->addr_transl = fpga->ddr_base;

	fpga->virt_addr = ioremap_wc(fpga->alloc_base, size - offset);

	ret = ftv310_vpu_fpga_memreset();
	if (ret != 0)
		goto out;

	return 0;

out:
	return -1;
}

void ftv310_vpu_phytium_mem_release(void)
{
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;
	int i;
	
	for (i = 0; i < fpga->chunks; i++) {
		spin_lock(&fpga->mem_lock);
		if (fpga->hlina_chunks[i].filp != 0) {
			pr_warn("memalloc: Found unfreed memory at release time!\n");
			fpga->hlina_chunks[i].filp = 0;
			fpga->hlina_chunks[i].chunks_reserved = 0;
		}
		spin_unlock(&fpga->mem_lock);
	}
	
	if (fpga->virt_pool_addr)
		iounmap(fpga->virt_pool_addr);
	
	if (fpga->virt_addr) {
		unsigned long pfn= PHYS_PFN(fpga->alloc_base);
		if (pfn_valid(pfn))
			vunmap(fpga->virt_addr);
		else
			iounmap(fpga->virt_addr);
	}
	
	if (fpga->hlina_chunks)
		vfree(fpga->hlina_chunks);

	if (vram_used_size) {
		if (g_fpga.virt_pool_addr) {
			if (vram_used_size != get_ftv310_vpu_fpgamem_offset())
				pr_err("vram leak!!!, left size:%ld\n", vram_used_size);
		} else {
			pr_err("vram leak!!!, left size:%ld\n", vram_used_size);
		}
	}
	mutex_destroy(&vram_mutex);
}

//Reserve memory region for MMU and VCMD buffer pool
static unsigned long get_ftv310_vpu_fpgamem_offset(void)
{
	unsigned long offset = 0;
#ifdef HAS_MMU
	offset += get_total_mmu_pgtbl_buf_size(1);
#endif

#ifdef HAS_VCMD
	offset += get_total_vcmd_pool_size();
#endif
	return offset;
}

void record_vram_node(struct vram_record *r, void *vaddr, unsigned long size) {
	mutex_lock(&vram_mutex);
	list_add_tail(&r->node, &vram_list);
	vram_used_size += size;
	mutex_unlock(&vram_mutex);

	r->vaddr = vaddr;
	r->size = size;
}

void remove_vram_node(struct vram_record *r) {
	mutex_lock(&vram_mutex);
	list_del(&r->node);
	vram_used_size -= r->size;
	mutex_unlock(&vram_mutex);
}

static void *vram_data = NULL;
void ftv310_vpu_vram_backup(void) {
	struct list_head *itr;
	unsigned long offset = 0;
	struct vram_record *r = NULL;
	unsigned long ddr_offset = get_ftv310_vpu_fpgamem_offset();

	pr_err("ftv310_vpu: enter vpu backup\n");

	if (!mem_start)
		return;

	if (!vram_used_size) {
		pr_info("ftv310_vpu: no need backup\n");
		return;
	}

	if (vram_data) {
		vfree(vram_data);
		vram_data = NULL;
	}

	if (!vram_data) {
		vram_data = vmalloc(vram_used_size);
		if (!vram_data) {
			pr_err("ftv310_vpu: backup failed, no mem\n");
			return;
		}
	}

	if (g_fpga.virt_pool_addr) {
		memcpy(vram_data, g_fpga.virt_pool_addr, ddr_offset);
		offset += ddr_offset;
	}

	list_for_each(itr, &vram_list) {
		r = list_entry(itr, struct vram_record, node);
		memcpy(vram_data + offset, r->vaddr, r->size);
		offset += r->size;
	}

	if (offset != vram_used_size)
		pr_err("ftv310_vpu: backup size mismatch:%ld -> %ld\n", vram_used_size, offset);
	else
		pr_err("ftv310_vpu: backup success, vram size:%ld\n", vram_used_size);
}

void ftv310_vpu_vram_restore(void) {
	struct vram_record *r = NULL;
	unsigned long ddr_offset = get_ftv310_vpu_fpgamem_offset();
	unsigned long offset = 0;
	struct list_head *itr;

	pr_err("ftv310_vpu: enter vpu restore\n");
	if (!mem_start || !vram_used_size) {
		return;
	}

	if (!vram_data) {
		pr_err("ftv310_vpu: backup data not found\n");
		return;
	}

	if (g_fpga.virt_pool_addr) {
		memcpy(g_fpga.virt_pool_addr, vram_data, ddr_offset);
		offset += ddr_offset;
	}

	list_for_each(itr, &vram_list) {
		r = list_entry(itr, struct vram_record, node);
		memcpy(r->vaddr, vram_data + offset, r->size);
		offset += r->size;
	}

	vfree(vram_data);
	vram_data = NULL;

	if (offset != vram_used_size) {
		pr_err("ftv310_vpu: restore size mismatch:%ld -> %ld\n", vram_used_size, offset);
	} else {
		pr_err("ftv310_vpu: restore success\n");
	}
}

