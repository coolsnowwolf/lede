// SPDX-License-Identifier: GPL-2.0
/*
 * ftv310 Decoder device driver (kernel module)
 *
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,USA.
 *
 *---------------------------------------------------------------------------
 */
#include <linux/mm.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/debugfs.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/pagemap.h>
#include <linux/sched.h>
#include <linux/stddef.h>
#include <linux/platform_device.h>
#include <linux/mod_devicetable.h>
#include <linux/dma-buf.h>
#include "ftv310_vpu_mmu.h"
#include "ipoffset/mmu_offset.h"
#include "ftv310_vpu_fpga_mem.h"
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu_trace.h"

MODULE_DESCRIPTION("Phytium VPU Driver");
MODULE_LICENSE("GPL");

#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void *)0)
#endif
#endif
#define PD_MODE
#define MMU_REG_OFFSET              0
#define MMU_REG_HW_ID          (MMU_REG_OFFSET + 6 * 4)
#define MMU_REG_FLUSH          (MMU_REG_OFFSET + 97 * 4)
#define MMU_REG_PAGE_TABLE_ID  (MMU_REG_OFFSET + 107 * 4)
#define MMU_REG_CONTROL        (MMU_REG_OFFSET + 226 * 4)
#define MMU_REG_ADDRESS        (MMU_REG_OFFSET + 227 * 4)
#define MMU_REG_ADDRESS_MSB    (MMU_REG_OFFSET + 228 * 4)
#define MMU_REG_PDENTRY0       (MMU_REG_OFFSET + 237 * 4)

/***** New MMU Defination ********************************************/
#define MMU_MTLB_SHIFT 22
#define MMU_STLB_4K_SHIFT 12
#define MMU_STLB_64K_SHIFT 16

#define MMU_MTLB_BITS (32 - MMU_MTLB_SHIFT)
#define MMU_PAGE_4K_BITS MMU_STLB_4K_SHIFT
#define MMU_STLB_4K_BITS (32 - MMU_MTLB_BITS - MMU_PAGE_4K_BITS)
#define MMU_PAGE_64K_BITS MMU_STLB_64K_SHIFT
#define MMU_STLB_64K_BITS (32 - MMU_MTLB_BITS - MMU_PAGE_64K_BITS)

#define MMU_MTLB_ENTRY_NUM BIT(MMU_MTLB_BITS)
#define MMU_MTLB_SIZE (MMU_MTLB_ENTRY_NUM << 2)
#define MMU_STLB_4K_ENTRY_NUM BIT(MMU_STLB_4K_BITS)
#define MMU_STLB_4K_SIZE (MMU_STLB_4K_ENTRY_NUM << 2)
#define MMU_PAGE_4K_SIZE BIT(MMU_STLB_4K_SHIFT)
#define MMU_STLB_64K_ENTRY_NUM BIT(MMU_STLB_64K_BITS)
#define MMU_STLB_64K_SIZE (MMU_STLB_64K_ENTRY_NUM << 2)
#define MMU_PAGE_64K_SIZE BIT(MMU_STLB_64K_SHIFT)

#define MMU_MTLB_MASK (~((1U << MMU_MTLB_SHIFT) - 1))
#define MMU_STLB_4K_MASK ((~0U << MMU_STLB_4K_SHIFT) ^ MMU_MTLB_MASK)
#define MMU_PAGE_4K_MASK (MMU_PAGE_4K_SIZE - 1)
#define MMU_STLB_64K_MASK ((~((1U << MMU_STLB_64K_SHIFT) - 1)) ^ MMU_MTLB_MASK)
#define MMU_PAGE_64K_MASK (MMU_PAGE_64K_SIZE - 1)

/* Page offset definitions. */
#define MMU_OFFSET_4K_BITS (32 - MMU_MTLB_BITS - MMU_STLB_4K_BITS)
#define MMU_OFFSET_4K_MASK ((1U << MMU_OFFSET_4K_BITS) - 1)
#define MMU_OFFSET_16K_BITS (32 - MMU_MTLB_BITS - MMU_STLB_16K_BITS)
#define MMU_OFFSET_16K_MASK ((1U << MMU_OFFSET_16K_BITS) - 1)

#define MMU_MTLB_ENTRY_HINTS_BITS 6
#define MMU_MTLB_ENTRY_STLB_MASK (~((1U << MMU_MTLB_ENTRY_HINTS_BITS) - 1))

#define MMU_MTLB_PRESENT 0x00000001
#define MMU_MTLB_EXCEPTION 0x00000002
#define MMU_MTLB_4K_PAGE 0x00000000

#define MMU_STLB_PRESENT 0x00000001
#define MMU_STLB_EXCEPTION 0x00000002
#define MMU_STLB_4K_PAGE 0x00000000

#define MMU_FALSE 0
#define MMU_TRUE 1

#define MMU_ERR_OS_FAIL (0xffff)
#define MMU_EFAULT MMU_ERR_OS_FAIL
#define MMU_ENOTTY MMU_ERR_OS_FAIL

#define MMU_INFINITE ((u32)~0U)

#define MAX_NOPAGED_SIZE 0x20000
#define MMU_SUPPRESS_OOM_MESSAGE 1

#define MTLB_PCIE_START_ADDRESS 0x00100000
#define PAGE_PCIE_START_ADDRESS 0x00200000 /* page_table_entry start address */
#define STLB_PCIE_START_ADDRESS 0x00300000
#define PAGE_TABLE_ENTRY_SIZE 64

#if MMU_SUPPRESS_OOM_MESSAGE
#define MMU_NOWARN __GFP_NOWARN
#else
#define MMU_NOWARN 0
#endif

#define MMU_IS_ERROR(status) ((status) < 0)
#define MMU_NO_ERROR(status) ((status) >= 0)
#define MMU_IS_SUCCESS(status) ((status) == MMU_STATUS_OK)

#undef MMUDEBUG
//#define FTV310_VPUMMU_DEBUG
#ifdef FTV310_VPUMMU_DEBUG
#ifdef __KERNEL__
#define MMUDEBUG(fmt, args...) pr_info("ftv310_vpu_mmu: " fmt, ##args)
#else
#define MMUDEBUG(fmt, args...) fprintf(stderr, fmt, ##args)
#endif
#else
#define MMUDEBUG(fmt, args...)
#endif

#define MMU_ON_ERROR(func)                                                     \
	do {                                                                   \
		status = func;                                                 \
		if (MMU_IS_ERROR(status)) {                                    \
			goto onerror;                                          \
		}                                                              \
	} while (MMU_FALSE)

#define WritePageEntry(page_entry, entry_value)                                \
	(*(unsigned int *)(page_entry) = (unsigned int)(entry_value))

/* Reserve 8MB for memeory allocator table by default. */
#define MEM_ALLOC_TABLE_SIZE 0x800000

/* simple map mode: generate mmu address which is same as input bus address*/
static unsigned int simple_map;
/* this shift should be an integral multiple of mmu page size(4096).
 *It can generate a mmu address shift in simple map mode
 */
static unsigned int map_shift;

extern struct ftv310_vpu_fpga_t g_fpga;

/* module_param(name, type, perm) */
module_param(simple_map, uint, 0);
module_param(map_shift, uint, 0);

#define MMU_TYPE_ENC "_ENC"
#define MMU_TYPE_DEC "_DEC"

unsigned int mmu_enable = MMU_FALSE;
static unsigned int pcie;
static unsigned long gBaseDDRHw = -1; /* PCI base register address (memalloc) */

static unsigned int region_private_mmu_start = REGION_PRIVATE_MMU_START;
static unsigned int region_private_mmu_end = REGION_PRIVATE_MMU_END;

static enum MMUStatus MMUEnableStep0(struct device *dev,
				     unsigned long BaseDDRAddr,
				     struct mmu_t *mmucore);
static void MMUEnableStep1(struct mmu_t *mmucore);
static enum MMUStatus MMURelease(void *filp, struct mmu_t *mmucore);
static enum MMUStatus MMUCleanup(struct mmu_t *mmucore);

static void MMUPageTableClean(struct device *dev, struct mmu_tbl_info *tmp);
static enum MMUStatus MMUPageTableInit(struct device *dev,
				     unsigned long BaseDDRAddr,
				     struct mmu_tbl_info *tmp);

static enum MMUStatus ZeroMemory(void *memory, unsigned int bytes)
{
	memset(memory, 0, bytes);

	return MMU_STATUS_OK;
}

static enum MMUStatus AllocateMemory(unsigned int bytes, void **memory)
{
	void *pointer;
	enum MMUStatus status;

	if (bytes > MAX_NOPAGED_SIZE) {
		pointer = vmalloc(bytes);
		MMUDEBUG(" *****VMALLOC size*****%d\n", bytes);
	} else {
		pointer = kmalloc(bytes, GFP_KERNEL | MMU_NOWARN);
		MMUDEBUG(" *****KMALLOC size*****%d\n", bytes);
	}

	if (!pointer) {
		/* Out of memory. */
		status = MMU_STATUS_OUT_OF_MEMORY;
		goto onerror;
	}

	/* Return pointer to the memory allocation. */
	*memory = pointer;

	return MMU_STATUS_OK;

onerror:
	/* Return the status. */
	return status;
}

static enum MMUStatus FreeMemory(void *memory)
{
	/* Free the memory from the OS pool. */
	if (is_vmalloc_addr(memory)) {
		MMUDEBUG(" *****VFREE*****%p\n", memory);
		vfree(memory);
	} else {
		MMUDEBUG(" *****KFREE*****%p\n", memory);
		kfree(memory);
	}
	return MMU_STATUS_OK;
}

static enum MMUStatus SMDeleteNode(struct MMUNode **pp)
{
	(*pp)->prev->next = (*pp)->next;
	(*pp)->next->prev = (*pp)->prev;

	MMUDEBUG(" *****DeleteNode size*****%d\n", (*pp)->page_count);
	FreeMemory(*pp);

	return MMU_STATUS_OK;
}

static enum MMUStatus DeleteNode(struct MMUNode **pp)
{
	(*pp)->prev->next = (*pp)->next;
	(*pp)->next->prev = (*pp)->prev;

	MMUDEBUG(" *****%s size*****%d\n", __func__, (*pp)->page_count);
	FreeMemory(*pp);

	return MMU_STATUS_OK;
}

static enum MMUStatus MergeNode(struct MMUNode *h, struct MMUNode **pp)
{
	struct MMUNode *tmp0 = h->next;
	struct MMUNode *tmp1 = h->next;

	while (tmp0) {
		/* 1th step: find front contiguous memory node */
		if (tmp0->mtlb_end == (*pp)->mtlb_start &&
		    tmp0->stlb_end == (*pp)->stlb_start) {
			tmp0->mtlb_end = (*pp)->mtlb_end;
			tmp0->stlb_end = (*pp)->stlb_end;
			tmp0->page_count += (*pp)->page_count;
			DeleteNode(pp);
			MMUDEBUG(
				" *****first merge to front. node size*****%d\n",
				tmp0->page_count);
			/* after merge to front contiguous memory node,
			 *find if there is behind contiguous memory node.
			 */
			while (tmp1) {
				/* merge */
				if (tmp1->mtlb_start == tmp0->mtlb_end &&
				    tmp1->stlb_start == tmp0->stlb_end) {
					tmp1->mtlb_start = tmp0->mtlb_start;
					tmp1->stlb_start = tmp0->stlb_start;
					tmp1->page_count += tmp0->page_count;
					MMUDEBUG(
						" *****second merge to behind. node size*****%d\n",
						tmp1->page_count);
					DeleteNode(&tmp0);
					return MMU_STATUS_OK;
				}
				tmp1 = tmp1->next;
			}
			return MMU_STATUS_OK;
			/* 1th step: find behind contiguous memory node */
		} else if (tmp0->mtlb_start == (*pp)->mtlb_end &&
			   tmp0->stlb_start == (*pp)->stlb_end) {
			tmp0->mtlb_start = (*pp)->mtlb_start;
			tmp0->stlb_start = (*pp)->stlb_start;
			tmp0->page_count += (*pp)->page_count;
			DeleteNode(pp);
			MMUDEBUG(
				" *****first merge to behind. node size*****%d\n",
				tmp0->page_count);
			/* after merge to behind contiguous memory node,
			 *find if there is front contiguous memory node
			 */
			while (tmp1) {
				/* merge */
				if (tmp1->mtlb_end == tmp0->mtlb_start &&
				    tmp1->stlb_end == tmp0->stlb_start) {
					tmp1->mtlb_end = tmp0->mtlb_end;
					tmp1->stlb_end = tmp0->stlb_end;
					tmp1->page_count += tmp0->page_count;
					MMUDEBUG(
						" *****second merge to front. node size*****%d\n",
						tmp1->page_count);
					DeleteNode(&tmp0);
					return MMU_STATUS_OK;
				}
				tmp1 = tmp1->next;
			}
			return MMU_STATUS_OK;
		}
		tmp0 = tmp0->next;
	}
	return MMU_STATUS_FALSE;
}

/* Insert a node to map list */
static enum MMUStatus SMInsertNode(struct MMUDDRRegion *region, struct MMUNode **pp)
{
	struct MMUNode *h;
	h = region->simple_map_head;

	h->next->prev = *pp;
	(*pp)->next = h->next;
	(*pp)->prev = h;
	h->next = *pp;
	MMUDEBUG(" *****insert bm node*****%d\n", (*pp)->page_count);

	return MMU_STATUS_OK;
}

static enum MMUStatus InsertNode(struct MMUDDRRegion *region, struct MMUNode **pp,
				 unsigned int free)
{
	enum MMUStatus status;
	struct MMUNode *h, *b;

	if (free) {
		h = region->free_map_head;
		b = region->map_head;
		status = MergeNode(h, pp);
		MMUDEBUG(" *****insert free*****%d\n", (*pp)->page_count);
		if (MMU_IS_ERROR(status)) {
			/* remove from map*/
			if ((*pp)->prev && (*pp)->next) {
				(*pp)->prev->next = (*pp)->next;
				(*pp)->next->prev = (*pp)->prev;
			}
			/* insert to free map */
			h->next->prev = *pp;
			(*pp)->next = h->next;
			(*pp)->prev = h;
			h->next = *pp;
		}
	} else {
		h = region->map_head;

		h->next->prev = *pp;
		(*pp)->next = h->next;
		(*pp)->prev = h;
		h->next = *pp;
		MMUDEBUG(" *****insert unfree*****%d\n", (*pp)->page_count);
	}

	return MMU_STATUS_OK;
}

/* Create a Node */
static enum MMUStatus SMCreateNode(struct MMUDDRRegion *region, struct MMUNode **node,
				   unsigned int page_count)
{
	struct MMUNode *p, **new;

	p = kmalloc(sizeof(struct MMUNode), GFP_KERNEL | MMU_NOWARN);
	new = &p;

	(*new)->mtlb_start = -1;
	(*new)->stlb_start = -1;
	(*new)->mtlb_end = -1;
	(*new)->stlb_end = -1;
	(*new)->process_id = 0;
	(*new)->filp = NULL;
	(*new)->page_count = 0;
	(*new)->prev = NULL;
	(*new)->next = NULL;
	/* Insert a uncomplete Node, it will be initialized later */
	SMInsertNode(region, new);

	/* return a new node for map buffer */
	*node = *new;
	return MMU_STATUS_OK;
}

/* Create initial Nodes */
static enum MMUStatus SMCreateNodes(struct MMUDDRRegion *region)
{
	struct MMUNode *simple_map_head;
	struct MMUNode *simple_map_tail;
	int i;
	/* Init each region map node */
	for (i = 0; i < MMU_REGION_COUNT; i++) {
		simple_map_head = kmalloc(sizeof(struct MMUNode),
					  GFP_KERNEL | MMU_NOWARN);
		simple_map_tail = kmalloc(sizeof(struct MMUNode),
					  GFP_KERNEL | MMU_NOWARN);

		simple_map_head->mtlb_start = -1;
		simple_map_head->stlb_start = -1;
		simple_map_head->mtlb_end = -1;
		simple_map_head->stlb_end = -1;
		simple_map_head->process_id = 0;
		simple_map_head->filp = NULL;
		simple_map_head->page_count = 0;
		simple_map_head->prev = NULL;
		simple_map_head->next = simple_map_tail;

		simple_map_tail->mtlb_start = -1;
		simple_map_tail->stlb_start = -1;
		simple_map_tail->mtlb_end = -1;
		simple_map_tail->stlb_end = -1;
		simple_map_tail->process_id = 0;
		simple_map_tail->filp = NULL;
		simple_map_tail->page_count = 0;
		simple_map_tail->prev = simple_map_head;
		simple_map_tail->next = NULL;

		region[i].simple_map_head = simple_map_head;
		region[i].simple_map_tail = simple_map_tail;
	}
	return MMU_STATUS_OK;
}

static enum MMUStatus CreateNode(struct MMUDDRRegion *region)
{
	struct MMUNode *free_map_head, *map_head, *p, **pp;
	struct MMUNode *free_map_tail, *map_tail;
	int i;
	unsigned int page_count;
	unsigned int prev_stlb = 0, prev_mtlb = 0;

	/* Init each region map node */
	for (i = 0; i < MMU_REGION_COUNT; i++) {
		free_map_head = kmalloc(sizeof(struct MMUNode),
					GFP_KERNEL | MMU_NOWARN);
		map_head = kmalloc(sizeof(struct MMUNode),
				   GFP_KERNEL | MMU_NOWARN);
		free_map_tail = kmalloc(sizeof(struct MMUNode),
					GFP_KERNEL | MMU_NOWARN);
		map_tail = kmalloc(sizeof(struct MMUNode),
				   GFP_KERNEL | MMU_NOWARN);

		free_map_head->mtlb_start = map_head->mtlb_start = -1;
		free_map_head->stlb_start = map_head->stlb_start = -1;
		free_map_head->mtlb_end = map_head->mtlb_end = -1;
		free_map_head->stlb_end = map_head->stlb_end = -1;
		free_map_head->process_id = map_head->process_id = 0;
		free_map_head->filp = map_head->filp = NULL;
		free_map_head->page_count = map_head->page_count = 0;
		free_map_head->prev = map_head->prev = NULL;
		free_map_head->next = free_map_tail;
		map_head->next = map_tail;

		free_map_tail->mtlb_start = map_tail->mtlb_start = -1;
		free_map_tail->stlb_start = map_tail->stlb_start = -1;
		free_map_tail->mtlb_end = map_tail->mtlb_end = -1;
		free_map_tail->stlb_end = map_tail->stlb_end = -1;
		free_map_tail->process_id = map_tail->process_id = 0;
		free_map_tail->filp = map_tail->filp = NULL;
		free_map_tail->page_count = map_tail->page_count = 0;
		free_map_tail->prev = free_map_head;
		map_tail->prev = map_head;
		free_map_tail->next = map_tail->next = NULL;

		region[i].free_map_head = free_map_head;
		region[i].map_head = map_head;
		region[i].free_map_tail = free_map_tail;
		region[i].map_tail = map_tail;

		p = kmalloc(sizeof(struct MMUNode), GFP_KERNEL | MMU_NOWARN);
		pp = &p;

		switch (i) {
		case MMU_REGION_PRIVATE:
			page_count = (REGION_PRIVATE_END -
				      REGION_PRIVATE_START + 1) /
				     PAGE_SIZE;
			p->stlb_start = region_private_mmu_start >> 12 & 0x3FF;
			p->mtlb_start = region_private_mmu_start >> 22;
			p->stlb_end = prev_stlb =
				region_private_mmu_end >> 12 & 0x3FF;
			p->mtlb_end = prev_mtlb = region_private_mmu_end >> 22;
			p->page_count = page_count;
			break;
		case MMU_REGION_PUB:
			p->stlb_start = prev_stlb;
			p->mtlb_start = prev_mtlb;
			p->stlb_end = prev_stlb = MMU_STLB_4K_ENTRY_NUM - 1;
			p->mtlb_end = prev_mtlb = MMU_MTLB_ENTRY_NUM - 1;
			p->page_count = (p->mtlb_end - p->mtlb_start) *
						MMU_STLB_4K_ENTRY_NUM +
					p->stlb_end - p->stlb_start + 1;
			break;
		default:
			pr_err(" *****MMU Region Error*****\n");
			break;
		}

		p->process_id = 0;
		p->filp = NULL;
		p->next = p->prev = NULL;

		InsertNode(&region[i], pp, 1);
	}

	return MMU_STATUS_OK;
}

/* A simpile function to check if the map buffer is existed.
 *it needs more complex version
 */
static enum MMUStatus SMCheckAddress(struct MMUDDRRegion *region, void *virtual_address)
{
	struct MMUNode *p;
	p = region->simple_map_head->next;
	while (p) {
		if (p->buf_virtual_address == virtual_address)
			return MMU_STATUS_FALSE;
		p = p->next;
	}
	return MMU_STATUS_OK;
}

static enum MMUStatus FindFreeNode(struct MMUDDRRegion *region, struct MMUNode **node,
					unsigned int page_count)
{
	struct MMUNode *p;
	p = region->free_map_head->next;
	while (p) {
		if (p->page_count >= page_count) {
			*node = p;
			return MMU_STATUS_OK;
		}
		p = p->next;
	}
	return MMU_STATUS_FALSE;
}

static enum MMUStatus SplitFreeNode(struct MMUDDRRegion *region, struct MMUNode **node,
				    unsigned int page_count)
{
	struct MMUNode *p, **new;

	p = kmalloc(sizeof(struct MMUNode), GFP_KERNEL | MMU_NOWARN);
	new = &p;

	**new = **node;

	(*new)->mtlb_start = (*node)->mtlb_start;
	(*new)->stlb_start = (*node)->stlb_start;
	(*new)->mtlb_end =
		(page_count + (*node)->stlb_start) / MMU_STLB_4K_ENTRY_NUM +
		(*node)->mtlb_start;
	(*new)->stlb_end =
		(page_count + (*node)->stlb_start) % MMU_STLB_4K_ENTRY_NUM;
	(*new)->process_id = (*node)->process_id;
	(*new)->page_count = page_count;

	MMUDEBUG(" *****new mtlb_start*****%d\n", (*new)->mtlb_start);
	MMUDEBUG(" *****new stlb_start*****%d\n", (*new)->stlb_start);
	MMUDEBUG(" *****new mtlb_end*****%d\n", (*new)->mtlb_end);
	MMUDEBUG(" *****new stlb_end*****%d\n", (*new)->stlb_end);
	/* Insert a new node in map */
	InsertNode(region, new, 0);

	/* Update free node in free map*/
	(*node)->page_count -= page_count;
	if ((*node)->page_count == 0) {
		DeleteNode(node);
		MMUDEBUG(" *****old node deleted*****\n");
	} else {
		(*node)->mtlb_start = (*new)->mtlb_end;
		(*node)->stlb_start = (*new)->stlb_end;

		MMUDEBUG(" *****old mtlb_start*****%d\n", (*node)->mtlb_start);
		MMUDEBUG(" *****old stlb_start*****%d\n", (*node)->stlb_start);
		MMUDEBUG(" *****old mtlb_end*****%d\n", (*node)->mtlb_end);
		MMUDEBUG(" *****old stlb_end*****%d\n", (*node)->stlb_end);
	}
	/* return a new node for map buffer */
	*node = *new;

	return MMU_STATUS_OK;
}

static enum MMUStatus SMRemoveNode(struct MMUDDRRegion *region,
				   void *buf_virtual_address,
				   unsigned int process_id)
{
	struct MMUNode *p, **pp;

	p = region->simple_map_head->next;
	pp = &p;
	while (*pp) {
		if ((*pp)->buf_virtual_address == buf_virtual_address &&
		    (*pp)->process_id == process_id) {
			SMDeleteNode(pp);
			break;
		}
		*pp = (*pp)->next;
	}

	return MMU_STATUS_OK;
}

static enum MMUStatus RemoveNode(struct MMUDDRRegion *region, void *buf_virtual_address,
				 unsigned int process_id)
{
	struct MMUNode *p, **pp;

	p = region->map_head->next;
	pp = &p;
	while (*pp) {
		if ((*pp)->buf_virtual_address == buf_virtual_address &&
		    (*pp)->process_id == process_id) {
			InsertNode(region, pp, 1);
			break;
		}
		*pp = (*pp)->next;
	}

	return MMU_STATUS_OK;
}

static enum MMUStatus Delay(unsigned int delay)
{
	if (delay > 0) {
		ktime_t dl = ktime_set((delay / MSEC_PER_SEC),
				       (delay % MSEC_PER_SEC) * NSEC_PER_MSEC);
		__set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_hrtimeout(&dl, HRTIMER_MODE_REL);
	}

	return MMU_STATUS_OK;
}

static enum MMUStatus CreateMutex(void **mtx)
{
	enum MMUStatus status;

	/* Allocate the mutex structure. */
	status = AllocateMemory(sizeof(struct mutex), mtx);
	if (MMU_IS_SUCCESS(status)) {
		/* Initialize the mutex. */
		mutex_init(*(struct mutex **)mtx);
	}

	return status;
}


static enum MMUStatus DeleteMutex(void *mtx)
{
	/* Destroy the mutex. */
	mutex_destroy((struct mutex *)mtx);

	/* Free the mutex structure. */
	FreeMemory(mtx);

	return MMU_STATUS_OK;
}

static enum MMUStatus AcquireMutex(void *mtx, unsigned int timeout)
{
	if (timeout == MMU_INFINITE) {
		/* Lock the mutex. */
		mutex_lock(mtx);

		/* Success. */
		return MMU_STATUS_OK;
	}

	for (;;) {
		/* Try to acquire the mutex. */
		if (mutex_trylock(mtx))
			return MMU_STATUS_OK;

		if (timeout-- == 0)
			break;

		/* Wait for 1 millisecond. */
		Delay(1);
	}

	return MMU_STATUS_OK;
}

static enum MMUStatus ReleaseMutex(void *mtx)
{
	/* Release the mutex. */
	mutex_unlock(mtx);
	return MMU_STATUS_OK;
}

static inline enum MMUStatus QueryProcessPageTable(void *logical,
						   unsigned long long *address)
{
	unsigned long lg = (unsigned long)logical;
	unsigned long offset = lg & ~PAGE_MASK;
	struct vm_area_struct *vma;
	spinlock_t *ptl;
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;

	if (is_vmalloc_addr(logical)) {
		/* vmalloc area. */
		*address = page_to_phys(vmalloc_to_page(logical)) | offset;
		return MMU_STATUS_OK;
	} else if (virt_addr_valid(logical)) {
		/* Kernel logical address. */
		*address = virt_to_phys(logical);
		return MMU_STATUS_OK;
	}

	return MMU_STATUS_INVALID_OBJECT;

	/* Try user VM area. */
	if (!current->mm)
		return MMU_STATUS_NOT_FOUND;

#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
	down_read(&current->mm->mmap_sem);
#else
	mmap_read_lock(current->mm);
#endif
	vma = find_vma(current->mm, lg);
#if KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
	up_read(&current->mm->mmap_sem);
#else
	mmap_read_unlock(current->mm);
#endif

	/* To check if mapped to user. */
	if (!vma)
		return MMU_STATUS_NOT_FOUND;

	pgd = pgd_offset(current->mm, lg);
	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return MMU_STATUS_NOT_FOUND;

#if (defined(CONFIG_CPU_CSKYV2) || defined(CONFIG_X86)) &&                     \
	KERNEL_VERSION(4, 12, 0) <= LINUX_VERSION_CODE
	pud = pud_offset((p4d_t *)pgd, lg);
#elif (defined(CONFIG_CPU_CSKYV2)) &&                                          \
	KERNEL_VERSION(4, 11, 0) <= LINUX_VERSION_CODE
	pud = pud_offset((p4d_t *)pgd, lg);
#else
	pud = pud_offset((p4d_t *)pgd, lg);
#endif
	if (pud_none(*pud) || pud_bad(*pud))
		return MMU_STATUS_NOT_FOUND;

	pmd = pmd_offset(pud, lg);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return MMU_STATUS_NOT_FOUND;

#if KERNEL_VERSION(6, 5, 0) > LINUX_VERSION_CODE
	pte = pte_offset_map_lock(current->mm, pmd, lg, &ptl);
#else
	ptl = pte_lockptr(current->mm, pmd);
	pte = pte_offset_kernel(pmd,lg);
	spin_lock(ptl);
#endif
	if (!pte) {
		spin_unlock(ptl);
		return MMU_STATUS_NOT_FOUND;
	}

	if (!pte_present(*pte)) {
		pte_unmap_unlock(pte, ptl);
		return MMU_STATUS_NOT_FOUND;
	}

	*address = (pte_pfn(*pte) << PAGE_SHIFT) | offset;
	pte_unmap_unlock(pte, ptl);

	*address -= gBaseDDRHw;

	return MMU_STATUS_OK;
}

static inline int GetProcessID(void)
{
	return current->tgid;
}

static enum MMUStatus GetPhysicalAddress(void *logical,
					 unsigned long long *address)
{
	enum MMUStatus status;

	status = QueryProcessPageTable(logical, address);

	return status;
}

static enum MMUStatus GetPageEntry(struct MMUNode *node,
				   unsigned int **page_table_entry,
				   void *stlb_virtual, unsigned int i)
{
	int num =
		node->mtlb_start * MMU_STLB_4K_ENTRY_NUM + node->stlb_start + i;
	*page_table_entry = (unsigned int *)stlb_virtual + num;
	return MMU_STATUS_OK;
}

static enum MMUStatus SetupDynamicSpace(struct MMUDDRRegion *region)
{
	/* Initial map info. */
	if (simple_map)
		SMCreateNodes(region);
	else
		CreateNode(region);

	return MMU_STATUS_OK;
}

//cfg only valid when using VCMD
int ftv310_vpu_MMUprobe(dtbnode *pnode, enum MMUProbeStage loop, struct mmu_t *p,
			struct platform_device *pdev, unsigned long ddr_base,
			struct mmu_core_cfg *cfg)
{
	enum MMUStatus status = 0;
	unsigned int i, k, id, mmu_core_num;
	void *pointer;
	struct mmu_t *pmmu = p;
	struct mmu_core_cfg *cfg_p = cfg;
	if (cfg_p == NULL) {
		cfg_p = mmu_core_array;
		mmu_core_num = ARRAY_SIZE(mmu_core_array);
	} else {
		mmu_core_num = 1;
	}

#ifdef PCIE_EN
	pcie = 1;
#else
	pcie = 0;
#endif
	if (loop == PGTBL_RESOURCE && gBaseDDRHw != -1) {
		pr_warn("Didn't call %s with loop = 0 once\n", __func__);
		return 0;
	}

	if (gBaseDDRHw == -1) {
		simple_map = 0;
		map_shift = 0;
	}
	MMUDEBUG("%s %d ddr = %lx", __func__, pcie, ddr_base);
	if (loop == PGTBL_RESOURCE || loop == ALL_RESOURCE) {
		struct mmu_t *mmucore;
		for (i = 0; i < mmu_core_num; i++) {
			mmucore = vmalloc(sizeof(struct mmu_t));
			if (!mmucore) {
				pr_err("vmalloc can't alloc mmu mmucore, no mem");
				return -1;
			}

			pointer =
				kmalloc(sizeof(struct MMU),
					GFP_KERNEL | MMU_NOWARN);
			if (!pointer) {
				pr_err("can't alloc mmu node, no mem");
				vfree(mmucore);
				return -1;
			}
			memset(pointer, 0, sizeof(struct MMU));
			mmucore->g_mmu = pointer;
			mmucore->mmu_enable = 0;
			mmucore->dev = &pdev->dev;
			mmucore->core_cfg = *cfg_p;
			cfg_p++;

			MMUDEBUG("mmu step 0: %llx:%d, loop = %d\n",
					mmucore->core_cfg.mmucorebase,
					mmucore->core_cfg.iosize, loop);

			MMU_ON_ERROR(
				CreateMutex(&mmucore->hw_mutex));

			for (k = 0; k < MMU_REGION_COUNT; k++)
				MMU_ON_ERROR(CreateMutex(
					&mmucore->g_mmu->region[k].node_mutex));

			MMUDEBUG("mmu probe at slice %d\n", mmucore->core_cfg.sliceidx);

			if (add_mmunode(mmucore->core_cfg.sliceidx, mmucore) < 0) {
				MMUDEBUG("!!! add_mmunode failed\n");
				return -ENODEV;
			}

			MMUDEBUG("add mmu core %p at slice %d\n",
				mmucore, mmucore->core_cfg.sliceidx);
			MMUEnableStep0(&pdev->dev, ddr_base, mmucore);
			if (loop == PGTBL_RESOURCE)
				return 0;
			pmmu = mmucore; //vcmd mode probe mmu one by one
		}
	}

	if (!pmmu)
		return -ENODEV;

	if (!request_mem_region(pmmu->core_cfg.mmucorebase,
				pmmu->core_cfg.iosize, "ftv310_vpu_mmu")) {
		kfree(pmmu->g_mmu);
		return -ENODEV;
	}
	pmmu->hwregs = (u8 *)ioremap(pmmu->core_cfg.mmucorebase,
				     pmmu->core_cfg.iosize);
	if (!pmmu->hwregs) {
		release_mem_region(pmmu->core_cfg.mmucorebase,
				   pmmu->core_cfg.iosize);
		kfree(pmmu->g_mmu);
		return -ENODEV;
	}
	MMUDEBUG("mmu reg %llx map to %p", pmmu->core_cfg.mmucorebase,
		 pmmu->hwregs);
	id = ioread32((void *)(pmmu->hwregs + MMU_REG_HW_ID)) >> 16;
	if (id != 0x4D4D) {
		iounmap((void *)pmmu->hwregs);
		release_mem_region(pmmu->core_cfg.mmucorebase,
				   pmmu->core_cfg.iosize);
		kfree(pmmu->g_mmu);
		pr_err("MMU_STATUS_NOT_FOUND, read id is 0x %x\n", id);
		return -ENODEV;
	}
	MMUEnableStep1(pmmu);
	return MMU_STATUS_OK;

onerror:
	pr_err(" *****MMU Init Error*****\n");
	return status;
}


void ftv310_vpu_MMURelease(void *filp)
{
	int i, slicen = get_slicenumber();
	struct mmu_t *mmucore;

	for (i = 0; i < slicen; i++) {
		mmucore = get_mmunode(i, 0);
		while (mmucore) {
			if (simple_map) {
				if (!mmucore->g_mmu->region[0].simple_map_head)
					continue;
				if (!mmucore->g_mmu->region[0].simple_map_head)
					continue;
			} else {
				if (!mmucore->g_mmu->region[0].map_head)
					continue;
			}

			MMURelease(filp, mmucore);
			mmucore = mmucore->next;
		}
	}
}

static enum MMUStatus MMURelease(void *filp, struct mmu_t *mmucore)
{
	int i = 0, j = 0;
	struct MMUNode *p, *tmp;
	unsigned long long address;
	unsigned int *page_table_entry;
	struct MMU *mmu = mmucore->g_mmu;
	/* if mmu or TLB not enabled, return */
	MMUDEBUG(" *****MMU Try to Release*****\n");

	AcquireMutex(mmucore->pg_tbl.page_table_mutex, MMU_INFINITE);
	if (simple_map)
		p = mmu->region[i].simple_map_head->next;
	else
		p = mmu->region[i].map_head->next;

	for (i = 0; i < MMU_REGION_COUNT; i++) {
		while (p) {
			tmp = p->next;
			if (p->filp == (struct file *)filp) {
				for (j = 0; j < p->page_count; j++) {
					GetPageEntry(p,
								&page_table_entry,
								mmucore->pg_tbl.stlb_virtual, j);
					address = 0;
					WritePageEntry(page_table_entry,
								address);
				}
				MMUDEBUG(" *****MMU Release Node *****\n");

				if (simple_map)
					SMRemoveNode(&mmu->region[i],
							p->buf_virtual_address,
							p->process_id);
				else
					RemoveNode(&mmu->region[i], p->buf_virtual_address,
						p->process_id);
			}
			p = tmp;
		}
	}

	ReleaseMutex(mmucore->pg_tbl.page_table_mutex);

	return MMU_STATUS_OK;
}

void ftv310_vpu_MMUCleanup(void)
{
	int i, slicen = get_slicenumber();
	struct mmu_t *mmucore, *pnext;
	struct slice_info *parentslice;

	for (i = 0; i < slicen; i++) {
		mmucore = get_mmunode(i, 0);
		if (mmucore) {
			parentslice = (struct slice_info *)(mmucore->parentslice);
			MMUPageTableClean(mmucore->dev, &parentslice->pg_tbl);
			DeleteMutex(parentslice->mmu_page_table_mutex);
			parentslice->mmu_page_table_mutex = NULL;
		}

		while (mmucore) {
			MMUCleanup(mmucore);

			iounmap((void *)mmucore->hwregs);
			release_mem_region(mmucore->core_cfg.mmucorebase,
					   mmucore->core_cfg.iosize);
			pnext = mmucore->next;
			vfree(mmucore);
			mmucore = pnext;
		}
	}
}

static enum MMUStatus MMUCleanup(struct mmu_t *mmucore)
{
	int i;
	struct MMUNode *p, *tmp;
	struct MMUNode *fp;
	struct MMU *mmu = mmucore->g_mmu;
	unsigned char *hwregs = mmucore->hwregs;
	MMUDEBUG(" *****MMU cleanup*****\n");
	DeleteMutex(mmucore->hw_mutex);

	for (i = 0; i < MMU_REGION_COUNT; i++) {
		DeleteMutex(mmu->region[i].node_mutex);
		if (simple_map) {
			p = mmu->region[i].simple_map_head;
			while (p) {
				tmp = p->next;
				FreeMemory(p);
				p   = tmp;
				MMUDEBUG(" *****clean node*****\n");
			}
		} else {
			fp = mmu->region[i].free_map_head;
			p  = mmu->region[i].map_head;
			while (fp) {
				tmp = fp->next;
				FreeMemory(fp);
				fp  = tmp;
				MMUDEBUG(" *****clean free node*****\n");
			}

			while (p) {
				tmp = p->next;
				FreeMemory(p);
				p   = tmp;
				MMUDEBUG(" *****clean node*****\n");
			}
		}
	}
	memset(&mmucore->pg_tbl, 0, sizeof(struct mmu_tbl_info));

	iowrite32(0, (void *) (hwregs + MMU_REG_CONTROL));
	return MMU_STATUS_OK;
}

/*------------------------------------------------------------------------------
 *Function name: MMUEnable
 *Description:
 *Create TLB, set registers and enable MMU

 *For pcie, TLB buffers come from FPGA memory and The distribution is as follows
 *MTLB:              start from: 0x00100000, size: 4K bits
 *page table array:              0x00200000        64 bits
 *STLB:                          0x00300000        4M bits
 *------------------------------------------------------------------------------
 */
static enum MMUStatus MMUEnableStep0(struct device *dev,
				     unsigned long BaseDDRAddr,
				     struct mmu_t *mmucore)
{
	enum MMUStatus status;
	struct slice_info *parentslice;

	MMUDEBUG(" *****MMU Enable...*****\n");

	parentslice = (struct slice_info *)(mmucore->parentslice);
	if (!parentslice) {
		MMUDEBUG("!!! erro parentslice, return");
		return -1;
	}

	if (!parentslice->mmu_page_table_mutex &&
		parentslice->is_share_pgtbl_in_slice) {
		MMU_ON_ERROR(
			CreateMutex(&parentslice->mmu_page_table_mutex));

		if (parentslice->mmucore_num > 0) {
			AcquireMutex(parentslice->mmu_page_table_mutex, MMU_INFINITE);
			parentslice->pg_tbl.page_table_mutex = parentslice->mmu_page_table_mutex;
			MMUPageTableInit(dev, BaseDDRAddr, &parentslice->pg_tbl);
			ReleaseMutex(parentslice->mmu_page_table_mutex);
		}
	}

	AcquireMutex(parentslice->mmu_page_table_mutex, MMU_INFINITE);
	if (parentslice->mmucore_num > 0)
		memcpy(&mmucore->pg_tbl, &parentslice->pg_tbl,
			sizeof(struct mmu_tbl_info));
	else
		memcpy(&mmucore->pg_tbl, &parentslice->mmuhdr->pg_tbl,
			sizeof(struct mmu_tbl_info));

	MMU_ON_ERROR(SetupDynamicSpace(&mmucore->g_mmu->region[0]));

	ReleaseMutex(parentslice->mmu_page_table_mutex);

	return MMU_STATUS_OK;
onerror:
	MMUDEBUG(" *****MMU Enable Error*****\n");
	return status;
}

static void MMUEnableStep1(struct mmu_t *mmucore)
{
unsigned char *hwregs = mmucore->hwregs;
struct mmu_tbl_info *pg_tbl = &mmucore->pg_tbl;

#ifndef PD_MODE
	/* set regs of all MMUs */
	unsigned int address;
	unsigned int address_ext = 0;
	if (pcie) {
		address = PAGE_PCIE_START_ADDRESS;
		address_ext = 0;
	} else {
		address = pg_tbl->page_table_array_physical;
		address_ext =
			((u32)(pg_tbl->page_table_array_physical >> 32)) & 0xff;
	}
	AcquireMutex(mmucore->hw_mutex, MMU_INFINITE);
	iowrite32(address, (void *)(hwregs + MMU_REG_ADDRESS));
	iowrite32(address_ext, (void *)(hwregs + MMU_REG_ADDRESS_MSB));
	iowrite32(0x10000, (void *)(hwregs + MMU_REG_PAGE_TABLE_ID));

	iowrite32(0x00000, (void *)(hwregs + MMU_REG_PAGE_TABLE_ID));
	iowrite32(0x1, (void *)(hwregs + MMU_REG_CONTROL));
#else
	AcquireMutex(mmucore->hw_mutex, MMU_INFINITE);
	iowrite32((pg_tbl->mtlb_physical >> 8) & 0xfffffff0,
		(void __iomem *)(hwregs + MMU_REG_PDENTRY0));
	iowrite32(0x21, (void __iomem *)(hwregs + MMU_REG_CONTROL));
#endif
	ReleaseMutex(mmucore->hw_mutex);

}

/*------------------------------------------------------------------------------
 * Function name: MMUFlush
 * Description:
 * Flush MMU reg to update cache in MMU.
 *------------------------------------------------------------------------------
 */
static enum MMUStatus MMUFlush(struct mmu_t *mmu_core)
{
	if (!mmu_core || !mmu_core->hwregs)
		return MMU_STATUS_FALSE;

	pr_debug("%s:%p", __func__, mmu_core->hwregs);

	AcquireMutex(mmu_core->hw_mutex, MMU_INFINITE);
	iowrite32(0x10, (void *)(mmu_core->hwregs + MMU_REG_FLUSH));
	iowrite32(0x00, (void *)(mmu_core->hwregs + MMU_REG_FLUSH));
	ReleaseMutex(mmu_core->hw_mutex);

	return MMU_STATUS_OK;
}

static enum MMUStatus MMUMemNodeMap(struct mmu_addr_desc *addr, struct file *filp,
				    struct mmu_t *mmucore, int use_phyaddr)
{
	enum MMUStatus status;
	unsigned int page_count = 0;
	unsigned int i = 0;
	struct MMUNode *p;
	unsigned long long address = 0x0;
	unsigned int *page_table_entry;
	enum MMURegion e;
	unsigned int mutex = MMU_FALSE;
	u32 ext_addr;
	u32 uint_value;
	struct MMU *mmu = mmucore->g_mmu;
	MMUDEBUG(" *****MMU Map*****\n");
	AcquireMutex(mmucore->pg_tbl.page_table_mutex, MMU_INFINITE);
	mutex = MMU_TRUE;

	page_count = (addr->size - 1) / PAGE_SIZE + 1;

	if (!use_phyaddr) {
		if (addr->pages)
			address = page_to_phys(addr->pages[0]);
		else
			GetPhysicalAddress(addr->virtual_address, &address);
		MMUDEBUG(" *****MMU begin to map phy address: 0x%x which get from virture 0x%x\n", address, addr->virtual_address);
	} else {
		address = addr->bus_address;
		MMUDEBUG(" *****MMU begin to map phy address: 0x%x\n", address);
	}
        if (address >= REGION_PRIVATE_START &&
		 address + addr->size < REGION_PRIVATE_END)
		e = MMU_REGION_PRIVATE;
	else
		e = MMU_REGION_PUB;

	if (simple_map) {
		MMU_ON_ERROR(SMCheckAddress(&mmu->region[e], addr->virtual_address));
		SMCreateNode(&mmu->region[e], &p, page_count);
		MMUDEBUG(" *****Node map size*****%d\n", page_count);

		p->buf_virtual_address = addr->virtual_address;
		p->process_id = GetProcessID();
		p->filp = filp;

		p->mtlb_start = ((address + map_shift) >> MMU_MTLB_SHIFT);
		p->stlb_start =
			((address + map_shift) >> MMU_STLB_4K_SHIFT) & 0x3FF;
		p->mtlb_end =
			(page_count + p->stlb_start) / MMU_STLB_4K_ENTRY_NUM +
			p->mtlb_start;
		p->stlb_end =
			(page_count + p->stlb_start) % MMU_STLB_4K_ENTRY_NUM;
		p->page_count = page_count;

		for (i = 0; i < page_count; i++) {
			if (!use_phyaddr) {
				if (addr->pages)
					address = page_to_phys(addr->pages[i]);
				else
					GetPhysicalAddress(addr->virtual_address +
							i * PAGE_SIZE,
							&address);
			} else {
				address = addr->bus_address + i * PAGE_SIZE;
			}
			ext_addr = ((u32)(address >> 32)) & 0xff;
			GetPageEntry(p, &page_table_entry, mmucore->pg_tbl.stlb_virtual, i);
			address = (address & 0xFFFFF000)
				  /* writable */
				  | (1 << 2)
				  /* Ignore exception */
				  | (0 << 1)
				  /* Present */
				  | (1 << 0);
			WritePageEntry(page_table_entry, address);
		}

		/* Purpose of Bare_metal mode: input bus address==mmu address*/
		addr->bus_address = p->mtlb_start << MMU_MTLB_SHIFT |
				    p->stlb_start << MMU_STLB_4K_SHIFT;
	} else {
		MMU_ON_ERROR(FindFreeNode(&mmu->region[e], &p, page_count));

		SplitFreeNode(&mmu->region[e], &p, page_count);
		MMUDEBUG(" *****Node map size*****%d\n", p->page_count);

		p->buf_virtual_address = addr->virtual_address;
		p->process_id = GetProcessID();
		p->filp = filp;

		for (i = 0; i < page_count; i++) {
			if (!use_phyaddr) {
				if (addr->pages)
					address = page_to_phys(addr->pages[i]);
				else
					GetPhysicalAddress(addr->virtual_address +
							i * PAGE_SIZE,
							&address);
			} else {
				address = addr->bus_address + i * PAGE_SIZE;
			}

			ext_addr = ((u32)(address >> 32)) & 0xff;
			GetPageEntry(p, &page_table_entry, mmucore->pg_tbl.stlb_virtual, i);
			uint_value =
				(address & 0xFFFFF000)
				/* ext address , physical address bits [39,32]*/
				| (ext_addr << 4)
				/* writable */
				| (1 << 2)
				/* Ignore exception */
				| (0 << 1)
				/* Present */
				| (1 << 0);
			WritePageEntry(page_table_entry, uint_value);
		}
		addr->bus_address = p->mtlb_start << MMU_MTLB_SHIFT |
				    p->stlb_start << MMU_STLB_4K_SHIFT;
	}

	addr->bus_address = addr->bus_address & 0xffffffff;

	MMUDEBUG(" MMU_MTLB_SHIFT %d MMU_STLB_4K_SHIFT %d\n", MMU_MTLB_SHIFT,
		 MMU_STLB_4K_SHIFT);
	MMUDEBUG(
		"%s map total %d pages in region %d\nMTLB/STLB starts %d/%d, MTLB/STLB ends %d/%d\n",
		__func__, page_count, (u32)e, p->mtlb_start, p->stlb_start,
		p->mtlb_end, p->stlb_end);
	MMUDEBUG(" *****MMU map finished, mapped addr: 0x%x\n", addr->bus_address);

	ReleaseMutex(mmucore->pg_tbl.page_table_mutex);
	return MMU_STATUS_OK;

onerror:
	if (mutex)
		ReleaseMutex(mmucore->pg_tbl.page_table_mutex);
	pr_err(" *****MMU Map Error*****\n");
	return status;
}

static enum MMUStatus MMUMemNodeUnmap(struct mmu_addr_desc *addr, struct MMUDDRRegion *region, struct mmu_tbl_info *pg_tbl)
{
	unsigned int i;
	unsigned long long address = 0x0;
	unsigned int *page_table_entry;
	int process_id = GetProcessID();
	enum MMURegion e = MMU_REGION_COUNT;
	enum MMUStatus status = MMU_STATUS_OUT_OF_MEMORY;
	struct MMUNode *p;
	unsigned int mutex = MMU_FALSE;

	MMUDEBUG(" *****MMU Unmap*****\n");
	AcquireMutex(pg_tbl->page_table_mutex, MMU_INFINITE);
	mutex = MMU_TRUE;

	if (!addr->bus_address) {
		if (addr->pages)
			address = page_to_phys(addr->pages[0]);
		else
			GetPhysicalAddress(addr->virtual_address, &address);
	} else {
		address = addr->bus_address;
	}

    if (address >= REGION_PRIVATE_START &&
		 address < REGION_PRIVATE_END)
		e = MMU_REGION_PRIVATE;
	else
		e = MMU_REGION_PUB;

	if (simple_map)
		p = region[e].simple_map_head->next;
	else
		p = region[e].map_head->next;
	/* Reset STLB of the node */
	while (p) {
		if (p->buf_virtual_address == addr->virtual_address &&
		    p->process_id == process_id) {
			for (i = 0; i < p->page_count; i++) {
				GetPageEntry(p, &page_table_entry, pg_tbl->stlb_virtual, i);
				address = 0;
				WritePageEntry(page_table_entry, address);
			}
			break;
		}
		p = p->next;
	}
	if (!p)
		goto onerror;

	if (simple_map)
		SMRemoveNode(&region[e], addr->virtual_address, process_id);
	else
		RemoveNode(&region[e], addr->virtual_address, process_id);

	ReleaseMutex(pg_tbl->page_table_mutex);
	return MMU_STATUS_OK;

onerror:
	if (mutex)
		ReleaseMutex(pg_tbl->page_table_mutex);
	MMUDEBUG(" *****MMU Unmap Error*****\n");
	return status;
}

static long MMUCtlFlush(struct mmu_t *mmuhdr)
{
	/*need to flush both MMU and MMU_WR here*/
	MMUFlush(mmuhdr);
	return 0;
}

int mmu_pm_resume(u32 sliceindex, int type)
{
	struct mmu_t *mmu  = NULL;

	mmu = get_mmunode(sliceindex, 0);
	if (!mmu) {
		pr_err("get mmut_t from sliceindex(%d) failed\n", sliceindex);
		return -1;
	}
	while (mmu) {
		if (mmu->core_cfg.parenttype == type)
			MMUEnableStep1(mmu);
		mmu = mmu->next;
	}

	return 0;
}

static long MMUCtlBufferMap(struct file *filp, struct mmu_addr_desc *addr,
			    struct mmu_t *mmucore)
{
	struct drm_gem_object *obj = NULL;
	unsigned long bus_address = 0;
	enum MMUStatus status = 0;

	if (addr->handle > 0) {
		struct drm_device *dev = ftv310_vpu_dev.drm_dev;
		struct drm_file *file_priv = filp->private_data;
		struct drm_gem_ftv310_vpu_object *cma_obj = NULL;

		obj = ftv310_vpu_gem_object_lookup(dev, file_priv, addr->handle);
		if (obj) {
			cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
			if (cma_obj->flag & FTV310_VPU_GEM_FLAG_RESERVED)
				addr->bus_address = cma_obj->paddr;
			else if (cma_obj->flag & FTV310_VPU_GEM_FLAG_USEVMALLOC)
				addr->virtual_address = cma_obj->vaddr;
			else if (cma_obj->flag & FTV310_VPU_GEM_FLAG_IMPORT) {
				addr->virtual_address = cma_obj; //fake address
				addr->pages = cma_obj->pages;
				if (!addr->pages)
					addr->bus_address = cma_obj->paddr;
				if (!addr->pages && !addr->bus_address) {
					pr_err("invalid addr desc\n");
					return -MMU_EFAULT;
				}
			}
			cma_obj->mmu = mmucore;
			cma_obj->uptr = addr->virtual_address;
		}
	}

	bus_address = addr->bus_address;

	status = MMUMemNodeMap(addr, filp, mmucore, addr->bus_address ? 1: 0);

	if (obj)
		ftv310_vpu_unref_drmobj(obj);

	trace_vcmd_mmu_map(current->pid, addr->size, addr->handle, bus_address, addr->bus_address);

	if (status != MMU_STATUS_OK) {
		return -MMU_EFAULT;
	}

	while (mmucore) {
		MMUCtlFlush(mmucore);
		mmucore = mmucore->next;
	}
	return 0;
}

long ftv310_vpu_MMUClear(struct mmu_addr_desc *addr, struct MMUDDRRegion *region, struct mmu_tbl_info *pg_tbl)
{
	trace_vcmd_mmu_unmap(current->pid, addr->handle, addr->bus_address);
	MMUMemNodeUnmap(addr, region, pg_tbl);

	return 0;
}

static long MMUCtlBufferUnmap(struct file *filp, struct mmu_addr_desc *addr, struct MMUDDRRegion *region, struct mmu_tbl_info *pg_tbl)
{
	struct drm_gem_object *obj = NULL;
	if (addr->handle > 0) {
		struct drm_device *dev = ftv310_vpu_dev.drm_dev;
		struct drm_file *file_priv = filp->private_data;
		struct drm_gem_ftv310_vpu_object *cma_obj = NULL;

		obj = ftv310_vpu_gem_object_lookup(dev, file_priv, addr->handle);
		if (obj) {
			cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
			if (cma_obj->flag & FTV310_VPU_GEM_FLAG_RESERVED)
				addr->bus_address = cma_obj->paddr;
			else if (cma_obj->flag & FTV310_VPU_GEM_FLAG_USEVMALLOC)
				addr->virtual_address = cma_obj->vaddr;
			else if (cma_obj->flag & FTV310_VPU_GEM_FLAG_IMPORT) {
				addr->virtual_address = cma_obj; //fake address
				addr->size = cma_obj->num_pages << PAGE_SHIFT;
				addr->pages = cma_obj->pages;
				if (!addr->pages) {
					addr->bus_address = cma_obj->paddr;
				}
			}
		}
	}

	trace_vcmd_mmu_unmap(current->pid, addr->handle, addr->bus_address);
	MMUMemNodeUnmap(addr, region, pg_tbl);

	if (obj)
		ftv310_vpu_unref_drmobj(obj);

	return 0;
}

static struct mmu_t *getMMUcore(struct mmu_addr_desc *addr)
{
	unsigned int slice, type, tid = 0;
	struct mmu_t *pmmu;

	slice = SLICE(addr->id);
	type = NODETYPE(addr->id);
	if (type & NODE_TYPE_DEC)
		tid |= FTV310_VPU_CORE_DEC;
	if (type & NODE_TYPE_ENC)
		tid |= FTV310_VPU_CORE_ENC;
	if (type & NODE_TYPE_VCMD)
		tid |= FTV310_VPU_CORE_VCMD;

	tid |= FTV310_VPU_CORE_VCMD;
	pmmu = get_mmunodebytype(slice, tid); // to do by id
	MMUDEBUG("%s  %p on slice %d, type %d\n", __func__, pmmu, slice, tid);
	return pmmu;
}

void ftv310_vpu_mmu_map(struct mmu_addr_desc *addr, int use_phyaddr)
{
	struct mmu_t *mmu;

	if (!addr)
		return;

	mmu = get_mmunode(SLICE(addr->id), 0);
	if (!mmu)
		return;
	MMUMemNodeMap(addr, NULL, mmu, use_phyaddr);
	while (mmu) {
		MMUCtlFlush(mmu);
		mmu = mmu->next;
	}
}

long ftv310_vpu_MMUIoctl(unsigned int cmd, void *filp, unsigned long arg)
{
	struct mmu_addr_desc addr;
	struct mmu_t *mmu;
	int ret = 0;

	if (copy_from_user(&addr, (void *)arg, sizeof(struct addr_desc)))
		return -EFAULT;

	addr.pages = NULL;

	switch (cmd) {
	case FTV310_VPU_IOCS_MMU_MEM_MAP:
		mmu = getMMUcore(&addr);
		if (!mmu)
			return -ENODEV;
		ret = MMUCtlBufferMap((struct file *)filp, &addr, mmu);
		break;
	case FTV310_VPU_IOCS_MMU_MEM_UNMAP:
		mmu = getMMUcore(&addr);
		if (!mmu)
			return -ENODEV;
		ret = MMUCtlBufferUnmap((struct file *)filp, &addr, &mmu->g_mmu->region[0], &mmu->pg_tbl);
		break;
	case FTV310_VPU_IOCS_MMU_FLUSH:
		mmu = get_mmunode(SLICE(addr.id), 0);
		if (!mmu)
			return -ENODEV;
		ret = MMUCtlFlush(mmu);
		break;
	default:
		return -EINVAL;
	}
	if (copy_to_user((void *)arg, &addr, sizeof(struct addr_desc))) {
		MMUDEBUG("copy_to_user failed\n");
		return -MMU_EFAULT;
	}

	if (ret)
		send_sig(SIGTERM, current, 0); //kill current process

	return ret;
}

unsigned long get_mmu_pgtbl_buf_size(void)
{
	return MEM_ALLOC_TABLE_SIZE;
}
//one slice share same mmu tbl buffer
unsigned long get_total_mmu_pgtbl_buf_size(u32 slice_cnt)
{
	return MEM_ALLOC_TABLE_SIZE * slice_cnt;
}


static void MMUPageTableClean(struct device *dev, struct mmu_tbl_info *tmp)
{
	if (pcie) {
		if (tmp->stlb_virtual)
			iounmap(tmp->stlb_virtual);
		if (tmp->mtlb_virtual)
			iounmap(tmp->mtlb_virtual);
#ifndef PD_MODE
		if (tmp->page_table_array)
			iounmap(tmp->page_table_array);
#endif
	} else {
		struct ftv310_vpu_fpga_t *fpga = &g_fpga;
		if (!fpga->mmu_base) {
			if (tmp->stlb_virtual)
				dma_free_coherent(dev, tmp->stlb_size,
					tmp->stlb_virtual, (dma_addr_t)tmp->stlb_physical);
			if (tmp->mtlb_virtual)
				dma_free_coherent(dev, tmp->mtlb_size,
					tmp->mtlb_virtual, (dma_addr_t)tmp->mtlb_physical);
		}
#ifndef PD_MODE
		if (tmp->page_table_array)
			dma_free_coherent(dev, tmp->page_table_array_size,
								tmp->page_table_array, (dma_addr_t)tmp->page_table_array_physical);
#endif
	}
	memset(tmp, 0, sizeof(struct mmu_tbl_info));
}

static enum MMUStatus MMUPageTableInit(struct device *dev,
				     unsigned long BaseDDRAddr,
				     struct mmu_tbl_info *tmp)
{
	enum MMUStatus status = MMU_STATUS_OK;
	dma_addr_t dma_handle;
	void *pointer;
	unsigned long long address;
	unsigned int stlb_entry, total_table_size, i;
	unsigned int *mtlb_virtual;
	struct ftv310_vpu_fpga_t *fpga = &g_fpga;

	if (pcie) {
		tmp->mtlb_size = MMU_MTLB_SIZE;
		tmp->mtlb_virtual =
			ioremap(BaseDDRAddr + MTLB_PCIE_START_ADDRESS,
				tmp->mtlb_size);
		tmp->mtlb_physical = MTLB_PCIE_START_ADDRESS;
#ifndef PD_MODE
		tmp->page_table_array =
			ioremap(BaseDDRAddr + PAGE_PCIE_START_ADDRESS,
				PAGE_TABLE_ENTRY_SIZE);
#endif
		ZeroMemory(tmp->mtlb_virtual, tmp->mtlb_size);

		pointer = ioremap(BaseDDRAddr + STLB_PCIE_START_ADDRESS,
				  MMU_MTLB_ENTRY_NUM * MMU_STLB_4K_SIZE);
		tmp->stlb_virtual = pointer;
		MMUDEBUG(" *****stlb_virtual = %p, size %d=%dx%d\n", pointer,
			 MMU_MTLB_ENTRY_NUM * MMU_STLB_4K_SIZE, MMU_MTLB_ENTRY_NUM, MMU_STLB_4K_SIZE);
		address = STLB_PCIE_START_ADDRESS;
		mtlb_virtual = (unsigned int *)tmp->mtlb_virtual;
		for (i = 0; i < MMU_MTLB_ENTRY_NUM; i++) {
			stlb_entry = address
				     /* 4KB page size */
				     | (0 << 2)
				     /* Ignore exception */
				     | (0 << 1)
				     /* Present */
				     | (1 << 0);
			WritePageEntry(mtlb_virtual++, stlb_entry);
			address += MMU_STLB_4K_SIZE;
		}
	} else {
		/* Allocate the 4K mode MTLB table. */
		if (!fpga->mmu_base) {
			total_table_size =
				MMU_MTLB_SIZE +
				MMU_MTLB_ENTRY_NUM * MMU_STLB_4K_SIZE;
		}
		tmp->mtlb_size = MMU_MTLB_SIZE;
		if (fpga->mmu_base) {
			tmp->mtlb_virtual = fpga->virt_pool_addr;
		} else {
			tmp->mtlb_virtual =
				dma_alloc_coherent(dev, total_table_size,
					&dma_handle,
					GFP_KERNEL | GFP_DMA);
		}

		MMUDEBUG(" *****g_mmu->mtlb_virtual = 0x%p\n",
				tmp->mtlb_virtual);

		if (fpga->mmu_base) {
			tmp->mtlb_physical = (unsigned long long)fpga->mmu_base;
		} else {
			tmp->mtlb_physical = (unsigned long long)dma_handle;
		}

		MMUDEBUG(" *****mtlb_physical = 0x%08x\n",
				(unsigned int)tmp->mtlb_physical);
		if (!tmp->mtlb_virtual) {
			pr_err("ftv310_vpu_dec alloc buffer fail\n");
			status = MMU_STATUS_FALSE;
			goto onerror;
		}
		ZeroMemory(tmp->mtlb_virtual, total_table_size);
#ifndef PD_MODE
		tmp->page_table_array_size = PAGE_TABLE_ENTRY_SIZE;
		tmp->page_table_array = dma_alloc_coherent(dev,
			tmp->page_table_array_size, &dma_handle,
			GFP_KERNEL | GFP_DMA);
		MMUDEBUG(" *****page_table_array = 0x%p\n",
				tmp->page_table_array);
		tmp->page_table_array_physical =
			(unsigned long long)dma_handle;
		pr_debug(" *****page_table_array_physical = 0x%llx\n",
				tmp->page_table_array_physical);
		if (!tmp->page_table_array) {
			pr_err("ftv310_vpu_dec alloc buffer fail\n");
			status = MMU_STATUS_FALSE;
			goto onerror;
		}
#endif

		tmp->stlb_virtual =
			(void *)((u64)(tmp->mtlb_virtual) + MMU_MTLB_SIZE);
		tmp->stlb_physical = address =
			tmp->mtlb_physical + MMU_MTLB_SIZE;
		tmp->stlb_size = MMU_MTLB_ENTRY_NUM * MMU_STLB_4K_SIZE;
		mtlb_virtual = (unsigned int *)tmp->mtlb_virtual;

		for (i = 0; i < MMU_MTLB_ENTRY_NUM; i++) {
			stlb_entry = (address & 0xffffffe0)
				     /* 4KB page size */
				     | (0 << 2)
				     /* Ignore exception */
				     | (0 << 1)
				     /* Present */
				     | (1 << 0);
			WritePageEntry(mtlb_virtual++, stlb_entry);
			address += MMU_STLB_4K_SIZE;
		}
	}
	tmp->enabled = 1;
	return status;
onerror:
	if (!fpga->mmu_base && tmp->mtlb_virtual) {
		dma_free_coherent(dev, total_table_size, tmp->mtlb_virtual, (dma_addr_t)tmp->mtlb_virtual);
	}
	pr_err(" %s Error\n", __func__);
	return status;
}
