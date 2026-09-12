/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2023, Phytium Corporation.
 */
#ifndef __PHYTIUM_NPU_DMA_BUF_HEAP_H__
#define __PHYTIUM_NPU_DMA_BUF_HEAP_H__
#include <linux/scatterlist.h>
#include <linux/list.h>

struct npu_unified_heap_buffer {
	struct dma_heap *heap;
	struct list_head attachments;
	struct mutex lock;
	struct sg_table sgt;
	unsigned long req_len;
	unsigned int page_size;
	int vmap_cnt;
	int buffer_status; /* buffer is ready for inference */
	void *vaddr;
};

#endif
