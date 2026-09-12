// SPDX-License-Identifier: GPL-2.0
/* DMABUF NPU system heap exporter
 *
 * Copyright (C) 2023 Phytium Technology Co., Ltd.
 */

#include <linux/dma-buf.h>
#include <linux/dma-mapping.h>
#include <linux/dma-heap.h>
#include <linux/err.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/scatterlist.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <uapi/linux/dma-heap.h>
#include "linux/phytium_npu_dma_buf_heap.h"

struct dma_heap_attachment {
	struct device *dev;
	struct sg_table *sgtable;
	struct list_head attach_list;
	bool mapped;
};

static struct dma_heap *npu_unified_heap;

static gfp_t gfp_type = GFP_KERNEL | __GFP_COMP | __GFP_NOWARN | ___GFP_ZERO;
/*
 * The orders used for allocation (1MB, 64K, 4K)
 */
static const unsigned int orders[] = {0, 4, 8};//use 4k for test

static struct sg_table *duplicate_sg_table(struct sg_table *table)
{
	struct sg_table *new_table;
	int ret, i;
	struct scatterlist *sg, *new_sg;

	new_table = kzalloc(sizeof(*new_table), GFP_KERNEL);
	if (!new_table)
		return ERR_PTR(-ENOMEM);

	ret = sg_alloc_table(new_table, table->orig_nents, GFP_KERNEL);
	if (ret) {
		kfree(new_table);
		return ERR_PTR(-ENOMEM);
	}

	new_sg = new_table->sgl;
	for_each_sgtable_sg(table, sg, i) {
		sg_set_page(new_sg, sg_page(sg), sg->length, sg->offset);
		new_sg = sg_next(new_sg);
	}

	return new_table;
}

static int npu_system_heap_attach(struct dma_buf *pdmabuf,
			      struct dma_buf_attachment *attachment)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct sg_table *sgtable;

	struct dma_heap_attachment *attach = kzalloc(sizeof(struct dma_heap_attachment),
											GFP_KERNEL);
	if (!attach)
		return -ENOMEM;

	sgtable = duplicate_sg_table(&buffer->sgt);
	if (IS_ERR(sgtable)) {
		kfree(attach);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&attach->attach_list);
	attach->sgtable = sgtable;
	attach->dev = attachment->dev;

	attachment->priv = attach;
	attach->mapped = false;
	mutex_lock(&buffer->lock);
	list_add(&attach->attach_list, &buffer->attachments);
	mutex_unlock(&buffer->lock);

	return 0;
}

static void npu_system_heap_detach(struct dma_buf *pdmabuf,
			       struct dma_buf_attachment *attachment)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct dma_heap_attachment *attach = attachment->priv;

	mutex_lock(&buffer->lock);
	list_del(&attach->attach_list);
	mutex_unlock(&buffer->lock);

	sg_free_table(attach->sgtable);
	kfree(attach->sgtable);
	kfree(attach);
}

static struct sg_table *npu_system_heap_map_dma_buf(struct dma_buf_attachment *attachment,
						enum dma_data_direction direction)
{
	struct dma_heap_attachment *attach = attachment->priv;
	struct sg_table *sgtable = attach->sgtable;
	int ret;

	ret = dma_map_sgtable(attachment->dev, sgtable, direction, 0);
	if (ret)
		return ERR_PTR(ret);

	attach->mapped = true;
	return sgtable;
}

static void npu_system_heap_unmap_dma_buf(struct dma_buf_attachment *attachment,
				      struct sg_table *table,
				      enum dma_data_direction direction)
{
	struct dma_heap_attachment *a = attachment->priv;

	a->mapped = false;
	dma_unmap_sgtable(attachment->dev, table, direction, 0);
}

static int npu_system_heap_dma_buf_begin_cpu_access(struct dma_buf *pdmabuf,
						enum dma_data_direction direction)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct dma_heap_attachment *attach;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		invalidate_kernel_vmap_range(buffer->vaddr, buffer->req_len);

	list_for_each_entry(attach, &buffer->attachments, attach_list) {
		if (!attach->mapped)
			continue;
		dma_sync_sgtable_for_cpu(attach->dev, attach->sgtable, direction);
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int npu_system_heap_dma_buf_end_cpu_access(struct dma_buf *pdmabuf,
					      enum dma_data_direction direction)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct dma_heap_attachment *attach;

	mutex_lock(&buffer->lock);

	if (buffer->vmap_cnt)
		flush_kernel_vmap_range(buffer->vaddr, buffer->req_len);

	list_for_each_entry(attach, &buffer->attachments, attach_list) {
		if (!attach->mapped)
			continue;
		dma_sync_sgtable_for_device(attach->dev, attach->sgtable, direction);
	}
	mutex_unlock(&buffer->lock);

	return 0;
}

static int npu_system_heap_mmap(struct dma_buf *pdmabuf, struct vm_area_struct *vma)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct sg_table *sgtable = &buffer->sgt;
	unsigned long new_addr = vma->vm_start;
	struct sg_page_iter piter;
	int ret;

	for_each_sgtable_page(sgtable, &piter, vma->vm_pgoff) {
		struct page *page = sg_page_iter_page(&piter);

		ret = remap_pfn_range(vma, new_addr, page_to_pfn(page), PAGE_SIZE,
				      vma->vm_page_prot);
		if (ret)
			return ret;
		new_addr += PAGE_SIZE;
		if (new_addr >= vma->vm_end)
			return 0;
	}
	return 0;
}

static void *npu_system_heap_do_vmap(struct npu_unified_heap_buffer *buffer)
{
	struct sg_table *sgtable = &buffer->sgt;
	int npages = PAGE_ALIGN(buffer->req_len) / PAGE_SIZE;
	struct page **pages = vmalloc(sizeof(struct page *) * npages);
	struct page **tmp = pages;
	struct sg_page_iter piter;
	void *vaddr;

	if (!pages)
		return ERR_PTR(-ENOMEM);

	for_each_sgtable_page(sgtable, &piter, 0) {
		WARN_ON(tmp - pages >= npages);
		*tmp++ = sg_page_iter_page(&piter);
	}

	vaddr = vmap(pages, npages, VM_MAP, PAGE_KERNEL);
	vfree(pages);

	if (!vaddr)
		return ERR_PTR(-ENOMEM);

	return vaddr;
}

static int npu_system_heap_vmap(struct dma_buf *pdmabuf, struct iosys_map *map)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	void *vaddr;
	int ret = 0;

	mutex_lock(&buffer->lock);
	if (buffer->vmap_cnt) {
		buffer->vmap_cnt++;
		iosys_map_set_vaddr(map, buffer->vaddr);
		goto unlock;
	}

	vaddr = npu_system_heap_do_vmap(buffer);
	if (IS_ERR(vaddr)) {
		ret = PTR_ERR(vaddr);
		goto unlock;
	}

	buffer->vaddr = vaddr;
	buffer->vmap_cnt++;
	iosys_map_set_vaddr(map, buffer->vaddr);
unlock:
	mutex_unlock(&buffer->lock);

	return ret;
}

static void npu_system_heap_vunmap(struct dma_buf *pdmabuf, struct iosys_map *map)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;

	mutex_lock(&buffer->lock);
	if (!--buffer->vmap_cnt) {
		vunmap(buffer->vaddr);
		buffer->vaddr = NULL;
	}
	mutex_unlock(&buffer->lock);
	iosys_map_clear(map);
}

static void npu_system_heap_dma_buf_release(struct dma_buf *pdmabuf)
{
	struct npu_unified_heap_buffer *buffer = pdmabuf->priv;
	struct sg_table *table;
	struct scatterlist *sg;
	int i;

	table = &buffer->sgt;
	for_each_sgtable_sg(table, sg, i) {
		struct page *page = sg_page(sg);

		__free_pages(page, compound_order(page));
	}
	sg_free_table(table);
	kfree(buffer);
}

static const struct dma_buf_ops npu_system_heap_buf_ops = {
	.attach = npu_system_heap_attach,
	.detach = npu_system_heap_detach,
	.map_dma_buf = npu_system_heap_map_dma_buf,
	.unmap_dma_buf = npu_system_heap_unmap_dma_buf,
	.begin_cpu_access = npu_system_heap_dma_buf_begin_cpu_access,
	.end_cpu_access = npu_system_heap_dma_buf_end_cpu_access,
	.mmap = npu_system_heap_mmap,
	.vmap = npu_system_heap_vmap,
	.vunmap = npu_system_heap_vunmap,
	.release = npu_system_heap_dma_buf_release,
};

static struct dma_buf *npu_system_heap_alloc(struct dma_heap *heap,
						    unsigned long len,
						    u32 fd_flags,
						    u64 heap_flags)
{
	struct npu_unified_heap_buffer *buffer;
	DEFINE_DMA_BUF_EXPORT_INFO(exp_info);
	struct dma_buf *pdmabuf;
	struct sg_table *table;
	struct scatterlist *sg;
	struct list_head pages;
	struct page *page, *tmp_page;
	int i, ret = -ENOMEM;
	unsigned int order = orders[0];
	unsigned long size_remaining = len;

	buffer = kzalloc(sizeof(*buffer), GFP_KERNEL);
	if (!buffer)
		return ERR_PTR(-ENOMEM);

	INIT_LIST_HEAD(&buffer->attachments);
	mutex_init(&buffer->lock);
	buffer->heap = heap;
	buffer->req_len = len;

	INIT_LIST_HEAD(&pages);
	i = 0;
	while (size_remaining > 0) {
		/*
		 * Avoid trying to allocate memory if the process
		 * has been killed by SIGKILL
		 */
		if (fatal_signal_pending(current)) {
			ret = -EINTR;
			goto free_buffer;
		}

		page = alloc_pages(gfp_type, order);
		if (!page)
			goto free_buffer;

		list_add_tail(&page->lru, &pages);
		size_remaining -= page_size(page);
		order = compound_order(page);
		i++;
	}

	table = &buffer->sgt;
	if (sg_alloc_table(table, i, GFP_KERNEL))
		goto free_buffer;

	sg = table->sgl;
	list_for_each_entry_safe(page, tmp_page, &pages, lru) {
		sg_set_page(sg, page, page_size(page), 0);
		sg = sg_next(sg);
		list_del(&page->lru);
	}

	/* create the pdmabuf */
	exp_info.exp_name = dma_heap_get_name(heap);
	exp_info.ops = &npu_system_heap_buf_ops;
	exp_info.size = buffer->req_len;
	exp_info.flags = fd_flags;
	exp_info.priv = buffer;
	pdmabuf = dma_buf_export(&exp_info);
	if (IS_ERR(pdmabuf)) {
		ret = PTR_ERR(pdmabuf);
		goto free_pages;
	}
	return pdmabuf;

free_pages:
	for_each_sgtable_sg(table, sg, i) {
		struct page *p = sg_page(sg);

		__free_pages(p, compound_order(p));
	}
	sg_free_table(table);
free_buffer:
	list_for_each_entry_safe(page, tmp_page, &pages, lru)
		__free_pages(page, compound_order(page));
	kfree(buffer);

	return ERR_PTR(ret);
}

static const struct dma_heap_ops npu_system_heap_ops = {
	.allocate = npu_system_heap_alloc,
};

static int npu_system_heap_create(void)
{
	struct dma_heap_export_info exp_info;

	exp_info.name = "npu_heap";
	exp_info.ops = &npu_system_heap_ops;
	exp_info.priv = NULL;

	npu_unified_heap = dma_heap_add(&exp_info);
	if (IS_ERR(npu_unified_heap))
		return PTR_ERR(npu_unified_heap);

	return 0;
}
module_init(npu_system_heap_create);

MODULE_LICENSE("GPL");
MODULE_IMPORT_NS("DMA_BUF");
MODULE_AUTHOR("Cheng Quan <chengquan@phytium.com.cn>");
MODULE_DESCRIPTION("Phytium NPU DMA BUF heap driver");
