/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#ifndef __FTD330_GEM_H__
#define __FTD330_GEM_H__

#include <linux/dma-buf.h>

#include <drm/drm_gem.h>

#include "ftd330_drv.h"

#if KERNEL_VERSION(5, 5, 0) <= LINUX_VERSION_CODE
#include <drm/drm_prime.h>
#endif

/*
 *
 * @base: drm gem object.
 * @size: size requested from user
 * @cookie: cookie returned by dma_alloc_attrs
 *  - not kernel virtual address with DMA_ATTR_NO_KERNEL_MAPPING
 * @dma_addr: bus address(accessed by dma) to allocated memory region.
 *  - this address could be physical address without IOMMU and
 *  device address with IOMMU.
 * @dma_attrs: attribute for DMA API
 * @get_pages: flag for manually applying for non-contiguous memory.
 * @pages: Array of backing pages.
 * @sgt: Imported sg_table.
 *
 */
struct ftd330_gem_object {
	struct drm_gem_object base;
	size_t size;
	void *cookie;
	dma_addr_t dma_addr;
	u64 iova;
	unsigned long dma_attrs;
	bool get_pages;
	struct page **pages;
	struct sg_table *sgt;
	struct list_head list;
	void *vaddr_save;
	void *sub_gem;
};

static inline struct ftd330_gem_object *to_ftd330_gem_object(struct drm_gem_object *obj)
{
	return container_of(obj, struct ftd330_gem_object, base);
}

struct ftd330_gem_object *ftd330_gem_create_object(struct drm_device *dev, size_t size);

int ftd330_gem_prime_mmap(struct drm_gem_object *obj, struct vm_area_struct *vma);

int ftd330_gem_dumb_create(struct drm_file *file_priv, struct drm_device *drm,
		       struct drm_mode_create_dumb *args);
int phytium_mem_pool_init(struct drm_device *dev);
void phytium_mem_pool_deinit(struct drm_device *dev);
int ftd330_gem_mmap(struct file *filp, struct vm_area_struct *vma);
int ftd330_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma);

struct sg_table *ftd330_gem_prime_get_sg_table(struct drm_gem_object *obj);

struct drm_gem_object *ftd330_gem_prime_import(struct drm_device *dev, struct dma_buf *dma_buf);
struct drm_gem_object *ftd330_gem_prime_import_sg_table(struct drm_device *dev,
						    struct dma_buf_attachment *attach,
						    struct sg_table *sgt);
int ftd330_gem_query_ioctl(struct drm_device *dev, void *data, struct drm_file *file);
void ftd330_gem_free_object(struct drm_gem_object *obj);
struct ftd330_gem_object *ftd330_gem_object_lookup(u32 fd, u32 handle);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 10, 0)
void *ftd330_gem_prime_vmap(struct drm_gem_object *obj);
void ftd330_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr);
#endif
#endif /* __FTD330_GEM_H__ */
