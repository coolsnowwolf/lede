/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */
#ifndef __PHYTIUM_NPU_MMU_H__
#define __PHYTIUM_NPU_MMU_H__

#include <linux/types.h>
#include <linux/io.h>
#include "linux/dma-fence.h"
#include "phytium_npu_uapi.h"

#define PHYT_MMU_PC_MASK	0xFFC0000000
#define PHYT_MMU_PC_SHIFT	30
#define PHYT_MMU_PD_MASK	0x3FE00000
#define PHYT_MMU_PD_SHIFT	21
#define PHYT_MMU_PT_MASK	0x1FF000
#define PHYT_MMU_PT_SHIFT	12
#define PC_SHIFT 8
#define PHYT_MMU_ENTRY_VALID	0x1
#define PHYT_MMU_ENTRY_INVALID	0x0
#define PHYT_MMU_PC_ENTRY_MASK 0xFFFFFFF0

#define PHYT_MMU_PTE_AXCACHE_MASK	0x3C00000000000000UL
#define PHYT_MMU_PTE_AXCACHE_SHIFT	58
#define PHYT_MMU_PTE_PARITY_SHIFT	62

#define PHYT_MMU_AXICACHE_ATTR	0x1C00000000000000
/* Page catalogue address shift */
#define PHYT_MMU_PC_ADDR_SHIFT 12
#define PHYT_MMU_DDR_VIRT_BASE 0x40000000
#define PHYT_MMU_ALLOC_PAGE_SIZE 0x1000

#define PHYT_MMU_PD_PT_RECORD 512
#define MAX_INFERENCE_CTX 4
#define NUM_FENCES_PER_BUF 10
#define MAX_NUM_DEVS 8

/* MMUv3 PTE entry flags */
enum mmu_map_flag {
	PHYT_MMU_PTE_FLAG_NONE = 0x0,
	PHYT_MMU_PTE_FLAG_VALID = 0x1,
	PHYT_MMU_PTE_FLAG_READ_ONLY = 0x2,
	PHYT_MMU_PTE_FLAG_CACHE_COHERENCY = 0x4,
};

struct npu_mmu_config_global {
	u8	mode_bypass;
	u8 width;
	u8 is_use_mmu_pte;
	u8 default_ctx_id;
	u32 page_size;
};

struct npu_mmu_config {
	u32 addr_width; /* physical */
	bool bypass_hw; /* MMU bypass mode */
	bool use_pte_parity; /* enables parity calculation for PTEs */
	enum mem_attr new_attr;
	int page_size;
};

struct mmu_page_desc {
	u64 phys;
	u64 cpu_addr;
	u64 npu_addr_base;
};

struct pd_ext_info {
	u32 valid_pd;
	struct mmu_page_desc *pd_desc[512];
};

struct npu_mmu_pt {
	struct npu_mmu_pd *mpd;
	struct mmu_page_desc desc;
	u32 valid;
};

struct npu_mmu_pd {
	struct npu_mmu_catalogue *mpc;
	struct mmu_page_desc desc;
	struct npu_mmu_pt *pt;
	u64 pt_page_addr[512];
	u32 ref_count;
};

struct npu_mmu_catalogue {
	struct npu_mmu_ctx *mctx;
	struct npu_mmu_pd **pd;
	struct mmu_page_desc desc;
	struct pd_ext_info pd_ext;
	u32 ref_count;
};

struct npu_mctx_map {
	struct npu_mmu_ctx *mctx;
	struct list_head mctx_map_entry; //add to npu_mmu_ctx maplist
	struct dma_buf *dma_buf;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	int dma_buf_fd;
	u32 map_type;
	u64 virt_addr;			//map mmu virtual address from user
};

struct npu_mmu_ctx {
	struct phytium_npu_mmu_context *pnmctx;
	struct npu_mmu_catalogue *mmu_pc;
	struct list_head maplist;
};

struct phytium_npu_mmu_context {
	struct device *dev;
	u32 npu_id;
	u32 map_type;
	u32 context_id;
	u32 pc_base_phys_addr;
	u64 virt_base;
	u64 curr_virt_base;
	struct npu_mmu_ctx *mctx;
};

static inline u64 phytium_npu_read64(void *addr)
{
	return (u64)readl_relaxed((void __iomem *)addr) |
			((u64)readl_relaxed((void __iomem *)addr + 4) << 32);
}

#define REGWRITE64(N, K, V) \
	writeq_relaxed(V, (void __iomem *)((N)->reg_base + (K)))
#define REGWRITE32(N, K, V) \
	writel_relaxed(V, (void __iomem *)((N)->reg_base + (K)))

#define REGREAD32(N, K) \
	readl_relaxed((void __iomem *)((N)->reg_base + (K)))

#define REGREAD64(N, K)	\
	readq_relaxed((void __iomem *)((N)->reg_base + (K)))

/* Virtual memory space for dma buf in the kernel - OCM & device debug buffers */
#define NPU_VA_BASE_ADDR	0x2000000ULL
#define NPU_VA_SIZE			0xc0000000ULL //SIZE 2G, 2M-3G

#endif
