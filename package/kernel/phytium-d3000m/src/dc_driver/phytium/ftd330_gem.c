// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2025, Phytium Technology Co., Ltd.
 */

#include <linux/dma-buf.h>

#include <asm/set_memory.h>
#include <linux/genalloc.h>
#include <drm/drm_atomic.h>
#include <drm/drm_file.h>
#include "drm/ftd330_drm.h"
#include "ftd330_drv.h"
#include "ftd330_gem.h"

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct drm_gem_object_funcs ftd330_gem_default_funcs;
#endif

#define VRAM_POOL_ALLOC_ORDER 12

static void nonseq_free(struct page **pages, unsigned int nr_page)
{
	u32 i;

	if (!pages)
		return;

	for (i = 0; i < nr_page; i++)
		__free_page(pages[i]);
}

static void put_pages(unsigned int nr_page, struct ftd330_gem_object *ftd330_obj)
{
	u32 i;

	for (i = 0; i < nr_page; i++)
		ClearPageReserved(ftd330_obj->pages[i]);

#ifdef CONFIG_X86
	set_pages_array_wb(ftd330_obj->pages, nr_page);
#endif

	nonseq_free(ftd330_obj->pages, nr_page);

}

#if !defined(CONFIG_PHYTIUM_NCC) && defined(CONFIG_PHYTIUM_MMU)
static int get_pages(unsigned int nr_page, struct ftd330_gem_object *ftd330_obj)
{
	struct page *pages;
	u32 i, num_page, page_count = 0;
	int order = 0;
	gfp_t gfp = GFP_KERNEL;

	if (!ftd330_obj->pages)
		return -EINVAL;

	gfp &= ~__GFP_HIGHMEM;
	gfp |= __GFP_DMA32;

	num_page = nr_page;

	do {
		pages = NULL;
		order = get_order(num_page * PAGE_SIZE);
		num_page = 1 << order;

		if ((num_page + page_count > nr_page) || (order >= MAX_ORDER)) {
			num_page = num_page >> 1;
			continue;
		}

		pages = alloc_pages(gfp, order);
		if (!pages) {
			if (num_page == 1) {
				nonseq_free(ftd330_obj->pages, page_count);
				return -ENOMEM;
			}

			num_page = num_page >> 1;
		} else {
			for (i = 0; i < num_page; i++) {
				ftd330_obj->pages[page_count + i] = &pages[i];
				SetPageReserved(ftd330_obj->pages[page_count + i]);
			}

			page_count += num_page;
			num_page = nr_page - page_count;
		}
	} while (page_count < nr_page);

#ifdef CONFIG_X86
	if (set_pages_array_uc(ftd330_obj->pages, nr_page))
		DRM_DEV_ERROR(ftd330_obj->base.dev->dev, "failed to set_pages_array_uc.\n");
#endif

	ftd330_obj->get_pages = true;

	return 0;
}
#endif

#ifndef CONFIG_PHYTIUM_NCC
static int ftd330_gem_alloc_buf(struct ftd330_gem_object *ftd330_obj)
{
	struct drm_device *dev = ftd330_obj->base.dev;
	unsigned int nr_pages;
	struct sg_table sgt;
	int ret = -ENOMEM;
#ifdef CONFIG_PHYTIUM_MMU
	struct ftd330_drm_private *priv = dev->dev_private;
	u32 mmu_addr = 0;
#endif

	if (ftd330_obj->dma_addr) {
		DRM_DEV_DEBUG_KMS(dev->dev, "already allocated.\n");
		return 0;
	}

#ifdef CONFIG_X86
	ftd330_obj->dma_attrs = 0;
#else
	ftd330_obj->dma_attrs = DMA_ATTR_WRITE_COMBINE | DMA_ATTR_NO_KERNEL_MAPPING;
#endif

	if (!is_iommu_enabled(dev))
		ftd330_obj->dma_attrs |= DMA_ATTR_FORCE_CONTIGUOUS;

	nr_pages = ftd330_obj->size >> PAGE_SHIFT;

	ftd330_obj->pages = kvmalloc_array(nr_pages, sizeof(struct page *), GFP_KERNEL | __GFP_ZERO);
	if (!ftd330_obj->pages) {
		DRM_DEV_ERROR(dev->dev, "failed to allocate pages.\n");
		return -ENOMEM;
	}

	ftd330_obj->cookie = dma_alloc_attrs(to_dma_dev(dev), ftd330_obj->size, &ftd330_obj->dma_addr,
					 GFP_KERNEL, ftd330_obj->dma_attrs);
	if (!ftd330_obj->cookie) {
#ifdef CONFIG_PHYTIUM_MMU
		ret = get_pages(nr_pages, ftd330_obj);
		if (ret) {
			DRM_DEV_ERROR(dev->dev, "fail to allocate buffer.\n");
			goto err_free;
		}
#else
		DRM_DEV_ERROR(dev->dev, "failed to allocate buffer.\n");
		goto err_free;
#endif
	}
#ifdef CONFIG_X86
	else {
		ret = set_memory_uc((unsigned long)(ftd330_obj->cookie), nr_pages);
		if (ret) {
			DRM_DEV_ERROR(dev->dev, "failed to set_memory_uc.\n");
			goto err_mem_free;
		}
	}
#endif

#ifdef CONFIG_PHYTIUM_MMU
	/* MMU map*/
	if (!priv->mmu) {
		DRM_DEV_ERROR(dev->dev, "invalid mmu.\n");
		ret = -EINVAL;
		goto err_mem_free;
	}

	/* mmu for ree driver */
	if (!ftd330_obj->get_pages)
		ret = dc_mmu_map_memory_and_flush(dev, priv->mmu, (u64)phy_obj->dma_addr, nr_pages,
				&mmu_addr, true, false);
	else
		ret = dc_mmu_map_memory_and_flush(dev, priv->mmu, (u64)vs_obj->pages, nr_pages,
				&mmu_addr, false, false);

	if (ret) {
		DRM_DEV_ERROR(dev->dev, "failed to do mmu map.\n");
		goto err_mem_free;
	}

	ftd330_obj->iova = (u64)mmu_addr;
#else
	ftd330_obj->iova = (u64)ftd330_obj->dma_addr;
#endif

	if (!ftd330_obj->get_pages) {
		ret = dma_get_sgtable_attrs(to_dma_dev(dev), &sgt, ftd330_obj->cookie, ftd330_obj->dma_addr,
					    ftd330_obj->size, ftd330_obj->dma_attrs);
		if (ret < 0) {
			DRM_DEV_ERROR(dev->dev, "failed to get sgtable.\n");
			goto err_mem_free;
		}

#if KERNEL_VERSION(5, 12, 0) > LINUX_VERSION_CODE
		if (drm_prime_sg_to_page_addr_arrays(&sgt, ftd330_obj->pages, NULL, nr_pages)) {
			DRM_DEV_ERROR(dev->dev, "invalid sgtable.\n");
			ret = -EINVAL;
			goto err_sgt_free;
		}
#else
		if (drm_prime_sg_to_page_array(&sgt, ftd330_obj->pages, nr_pages)) {
			DRM_DEV_ERROR(dev->dev, "invalid sgtable.\n");
			ret = -EINVAL;
			goto err_sgt_free;
		}
#endif
		sg_free_table(&sgt);
	}

	return 0;

err_sgt_free:
	sg_free_table(&sgt);
err_mem_free:
	if (!ftd330_obj->get_pages)
		dma_free_attrs(to_dma_dev(dev), ftd330_obj->size, ftd330_obj->cookie, ftd330_obj->dma_addr,
			       ftd330_obj->dma_attrs);
	else
		put_pages(nr_pages, ftd330_obj);
err_free:
	if (ftd330_obj->pages) {
		kvfree(ftd330_obj->pages);
		ftd330_obj->pages = NULL;
	}

	return ret;
}

static void ftd330_gem_free_buf(struct ftd330_gem_object *ftd330_obj)
{
	struct drm_device *dev = ftd330_obj->base.dev;
#ifdef CONFIG_PHYTIUM_MMU
	struct ftd330_drm_private *priv = dev->dev_private;
	unsigned int nr_pages;
#endif

	if ((!ftd330_obj->get_pages) && (!ftd330_obj->dma_addr)) {
		DRM_DEV_DEBUG_KMS(dev->dev, "dma_addr is invalid.\n");
		return;
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (!priv->mmu) {
		DRM_DEV_ERROR(dev->dev, "invalid mmu.\n");
		return;
	}

	nr_pages = ftd330_obj->size >> PAGE_SHIFT;
	dc_mmu_unmap_memory_and_flush(dev, priv->mmu, (u32)vs_obj->iova, nr_pages);
#endif

	if (!ftd330_obj->get_pages) {
#ifdef CONFIG_X86
		set_memory_wb((unsigned long)(ftd330_obj->cookie), ftd330_obj->size >> PAGE_SHIFT);
#endif
		dma_free_attrs(to_dma_dev(dev), ftd330_obj->size, ftd330_obj->cookie,
			       (dma_addr_t)ftd330_obj->dma_addr, ftd330_obj->dma_attrs);
	} else {
		put_pages(ftd330_obj->size >> PAGE_SHIFT, ftd330_obj);
	}
}
#endif

#ifdef CONFIG_PHYTIUM_MMU
static void _ftd330_mmu_free_buf(struct ftd330_gem_object *ftd330_obj)
{
	struct drm_device *dev = ftd330_obj->base.dev;
	struct ftd330_drm_private *priv = dev->dev_private;
	unsigned int nr_pages;

	if (!priv->mmu) {
		DRM_DEV_ERROR(dev->dev, "invalid mmu.\n");
		return;
	}
	nr_pages = ftd330_obj->size >> PAGE_SHIFT;
	dc_mmu_unmap_memory_and_flush(dev, priv->mmu, (u32)ftd330_obj->iova, nr_pages);
}
#endif


#ifdef CONFIG_PHYTIUM_NCC
static int phytium_gem_alloc_buf(struct ftd330_gem_object *ftd330_obj)
{
	struct drm_device *dev = ftd330_obj->base.dev;
	unsigned int nr_pages;
	struct sg_table sgt;
	int ret = -ENOMEM;
	struct ftd330_drm_private *priv = dev->dev_private;
#ifdef CONFIG_PHYTIUM_MMU
	u32 mmu_addr = 0;
#endif
	unsigned long vaddr;

	if (ftd330_obj->dma_addr) {
		DRM_DEV_DEBUG_KMS(dev->dev, "already allocated.\n");
		return 0;
	}

#ifdef CONFIG_X86
	ftd330_obj->dma_attrs = 0;
#else
	ftd330_obj->dma_attrs = DMA_ATTR_WRITE_COMBINE | DMA_ATTR_NO_KERNEL_MAPPING;
#endif
	if (!is_iommu_enabled(dev))
		ftd330_obj->dma_attrs |= DMA_ATTR_FORCE_CONTIGUOUS;

	nr_pages = ftd330_obj->size >> PAGE_SHIFT;

	ftd330_obj->pages = kvmalloc_array(nr_pages, sizeof(struct page *), GFP_KERNEL | __GFP_ZERO);
	if (!ftd330_obj->pages) {
		DRM_DEV_ERROR(dev->dev, "mem_pool failed to allocate pages.\n");
		return -ENOMEM;
	}

	vaddr = gen_pool_alloc(priv->mem_pool, ftd330_obj->size);
	ftd330_obj->dma_addr = gen_pool_virt_to_phys(priv->mem_pool, vaddr);
	ftd330_obj->cookie = (void *)vaddr;

	if (!ftd330_obj->dma_addr) {
		DRM_DEV_ERROR(dev->dev, "Error:mem_pool alloc buf failed\n");
		goto err_free;
	}

#ifdef CONFIG_PHYTIUM_MMU
	/* MMU map*/
	if (!priv->mmu) {
		DRM_DEV_ERROR(dev->dev, "invalid mmu.\n");
		ret = -EINVAL;
		goto err_mem_free;
	}

	/* mmu for ree driver */
	if (!ftd330_obj->get_pages)
		ret = dc_mmu_map_memory_and_flush(dev, priv->mmu, (u64)ftd330_obj->dma_addr, nr_pages,
				&mmu_addr, true, false);
	else
		ret = dc_mmu_map_memory_and_flush(dev, priv->mmu, (u64)ftd330_obj->pages, nr_pages,
				 &mmu_addr, false, false);

	if (ret) {
		DRM_DEV_ERROR(dev->dev, "failed to do mmu map.\n");
		goto err_mem_free;
	}

	ftd330_obj->iova = (u64)mmu_addr;
#else
	ftd330_obj->iova = (u64)ftd330_obj->dma_addr;
#endif
	
	if (!ftd330_obj->get_pages) {
		ret = dma_get_sgtable_attrs(to_dma_dev(dev), &sgt, ftd330_obj->cookie, ftd330_obj->dma_addr,
					    ftd330_obj->size, ftd330_obj->dma_attrs);
		if (ret < 0) {
			DRM_DEV_ERROR(dev->dev, "failed to get sgtable.\n");
			goto err_mem_free;
		}

#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 12, 0)
		if (drm_prime_sg_to_page_addr_arrays(&sgt, ftd330_obj->pages, NULL, nr_pages)) {
			DRM_DEV_ERROR(dev->dev, "invalid sgtable.\n");
			ret = -EINVAL;
			goto err_sgt_free;
		}
#else
		if (drm_prime_sg_to_page_array(&sgt, ftd330_obj->pages, nr_pages)) {
			DRM_DEV_ERROR(dev->dev, "invalid sgtable.\n");
			ret = -EINVAL;
			goto err_sgt_free;
		}
#endif
		sg_free_table(&sgt);
	}
	return 0;

err_sgt_free:
	sg_free_table(&sgt);
err_mem_free:
	if (!ftd330_obj->get_pages)
		gen_pool_free(priv->mem_pool, (unsigned long)ftd330_obj->cookie, ftd330_obj->size);
	else
		put_pages(nr_pages, ftd330_obj);
err_free:
	if (ftd330_obj->pages) {
		kvfree(ftd330_obj->pages);
		ftd330_obj->pages = NULL;
	}

	return ret;
}

static void phytium_gem_free_buf(struct ftd330_gem_object *ftd330_obj)
{
	struct drm_device *dev = ftd330_obj->base.dev;
	struct ftd330_drm_private *priv = dev->dev_private;
#ifdef CONFIG_PHYTIUM_MMU
	unsigned int nr_pages;
#endif

	if ((!ftd330_obj->get_pages) && (!ftd330_obj->dma_addr)) {
		DRM_DEV_DEBUG_KMS(dev->dev, "dma_addr is invalid.\n");
		return;
	}

#ifdef CONFIG_PHYTIUM_MMU
	if (!priv->mmu) {
		DRM_DEV_ERROR(dev->dev, "invalid mmu.\n");
		return;
	}

	nr_pages = ftd330_obj->size >> PAGE_SHIFT;
	dc_mmu_unmap_memory_and_flush(dev, priv->mmu, (u32)ftd330_obj->iova, nr_pages);
#endif
	if (!ftd330_obj->get_pages)
		gen_pool_free(priv->mem_pool, (unsigned long)ftd330_obj->cookie, ftd330_obj->size);
	else
		put_pages(ftd330_obj->size >> PAGE_SHIFT, ftd330_obj);

}


int phytium_mem_pool_init(struct drm_device *dev)
{
	int ret = 0;
	struct ftd330_drm_private *priv = dev->dev_private;

	priv->mem_pool = gen_pool_create(VRAM_POOL_ALLOC_ORDER, -1);
	if (priv->mem_pool == NULL) {
		DRM_ERROR("fail to create memory pool\n");
		ret = -1;
		goto err_create_pool;
	}

	ret = gen_pool_add_virt(priv->mem_pool, (unsigned long)priv->mem_pool_start_address_virt,
								priv->mem_pool_start_address_phy, priv->mem_pool_size, -1);

	if (ret) {
		DRM_DEV_ERROR(dev->dev, "Error: add pool failed\n");
		ret = -1;
		goto err_add_poll;
	}

	memset(priv->mem_pool_start_address_virt, 0 ,3840*2200*4);
	return 0;

err_add_poll:
	if (priv->mem_pool)
		gen_pool_destroy(priv->mem_pool);

err_create_pool:
	return ret;

}

void phytium_mem_pool_deinit(struct drm_device *dev)
{
	struct ftd330_drm_private *priv = dev->dev_private;

	if (priv->mem_pool)
		gen_pool_destroy(priv->mem_pool);

	iounmap(priv->mem_pool_start_address_virt);
}
#endif

void ftd330_gem_free_object(struct drm_gem_object *obj)
{
	struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);
	struct ftd330_gem_object *sub_gem = NULL;

	if (obj->import_attach) {
#ifdef CONFIG_PHYTIUM_MMU
		_ftd330_mmu_free_buf(ftd330_obj);
#endif
		drm_prime_gem_destroy(obj, ftd330_obj->sgt);
	} else {
		if (!list_empty(&ftd330_obj->list))
			list_del(&ftd330_obj->list);
#ifdef CONFIG_PHYTIUM_NCC
		phytium_gem_free_buf(ftd330_obj);
#else
		ftd330_gem_free_buf(ftd330_obj);
#endif
	}

	if (ftd330_obj->pages) {
		kvfree(ftd330_obj->pages);
		ftd330_obj->pages = NULL;
	}

	if (ftd330_obj->sub_gem) {
		sub_gem = (struct ftd330_gem_object *)ftd330_obj->sub_gem;
		ftd330_gem_free_object(&sub_gem->base);
		ftd330_obj->sub_gem = NULL;
	}


	drm_gem_object_release(obj);

	if (ftd330_obj)
		kfree(ftd330_obj);
}

static struct ftd330_gem_object *ftd330_gem_alloc_object(struct drm_device *dev, size_t size)
{
	struct ftd330_gem_object *ftd330_obj;
	struct drm_gem_object *obj;
	int ret;

	ftd330_obj = kzalloc(sizeof(*ftd330_obj), GFP_KERNEL);
	if (!ftd330_obj)
		return ERR_PTR(-ENOMEM);

	ftd330_obj->size = size;
	obj = &ftd330_obj->base;

	ret = drm_gem_object_init(dev, obj, size);
	if (ret)
		goto err_free;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
	ftd330_obj->base.funcs = &ftd330_gem_default_funcs;
#endif
	ret = drm_gem_create_mmap_offset(obj);
	if (ret) {
		drm_gem_object_release(obj);
		goto err_free;
	}

	ftd330_obj->sub_gem = NULL;

	return ftd330_obj;

err_free:
	if (ftd330_obj)
		kfree(ftd330_obj);
	return ERR_PTR(ret);
}

struct ftd330_gem_object *ftd330_gem_create_object(struct drm_device *dev, size_t size)
{
	struct ftd330_gem_object *ftd330_obj;
	struct ftd330_drm_private *priv = dev->dev_private;
	int ret;

	size = PAGE_ALIGN(size);

	ftd330_obj = ftd330_gem_alloc_object(dev, size);
	if (IS_ERR(ftd330_obj))
		return ftd330_obj;

#ifdef CONFIG_PHYTIUM_NCC
	ret = phytium_gem_alloc_buf(ftd330_obj);
#else
	ret = ftd330_gem_alloc_buf(ftd330_obj);
#endif

	if (ret) {
		drm_gem_object_release(&ftd330_obj->base);
		if (ftd330_obj)
			kfree(ftd330_obj);
		return ERR_PTR(ret);
	}
	if (!ftd330_obj->base.import_attach) {
		list_add_tail(&ftd330_obj->list, &priv->gem_list_head);
	}

	FTD330_LOG("FTD330_LOG %s size is :%ld,physical address = 0x%llx,mmu address = 0x%llx, cookie = 0x%p\n",
						__func__, size, ftd330_obj->dma_addr,
						ftd330_obj->iova, ftd330_obj->cookie);
	return ftd330_obj;
}

static struct ftd330_gem_object *ftd330_gem_create_with_handle(struct drm_device *dev,
						       struct drm_file *file, size_t size,
						       unsigned int *handle)
{
	struct ftd330_gem_object *ftd330_obj;
	struct drm_gem_object *obj;
	int ret;

	ftd330_obj = ftd330_gem_create_object(dev, size);
	if (IS_ERR(ftd330_obj))
		return ftd330_obj;

	obj = &ftd330_obj->base;

	ret = drm_gem_handle_create(file, obj, handle);

#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	drm_gem_object_put(obj);
#else
	drm_gem_object_put_unlocked(obj);
#endif

	if (ret)
		return ERR_PTR(ret);

	return ftd330_obj;
}

int ftd330_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);
#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
	struct drm_device *drm_dev = ftd330_obj->base.dev;
#endif
	unsigned long vm_size;
	int ret = 0;
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	unsigned long pfn;
#endif

	vm_size = vma->vm_end - vma->vm_start;
	if (vm_size > ftd330_obj->size)
		return -EINVAL;

	vma->vm_pgoff = 0;

	if (!ftd330_obj->get_pages) {
#if KERNEL_VERSION(6, 3, 0) > LINUX_VERSION_CODE
		vma->vm_flags &= ~VM_PFNMAP;
#else
		vm_flags_clear(vma, VM_PFNMAP);
#endif

#ifdef CONFIG_X86
		vma->vm_page_prot =
			__pgprot(pgprot_val(vma->vm_page_prot) & (~(1 << _PAGE_BIT_PWT)));

		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
		pfn = PHYS_PFN(ftd330_obj->dma_addr);
		ret = remap_pfn_range(vma, vma->vm_start, pfn, vma->vm_end-vma->vm_start, vma->vm_page_prot);
#else
		ret = dma_mmap_attrs(to_dma_dev(drm_dev), vma, ftd330_obj->cookie, ftd330_obj->dma_addr,
				     ftd330_obj->size, ftd330_obj->dma_attrs);
#endif
	} else {
		u32 i, nr_pages, pfn = 0U;
		unsigned long start;

#ifdef CONFIG_X86
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#else
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#endif

#if KERNEL_VERSION(6, 3, 0) > LINUX_VERSION_CODE
		vma->vm_flags |= VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP;
#else
		vm_flags_set(vma, VM_IO | VM_DONTCOPY | VM_DONTEXPAND | VM_DONTDUMP);
#endif
		start = vma->vm_start;
		vm_size = PAGE_ALIGN(vm_size);
		nr_pages = vm_size >> PAGE_SHIFT;

		for (i = 0; i < nr_pages; i++) {
			pfn = page_to_pfn(ftd330_obj->pages[i]);

			ret = remap_pfn_range(vma, start, pfn, PAGE_SIZE, vma->vm_page_prot);
			if (ret < 0)
				break;

			start += PAGE_SIZE;
		}
	}

	if (ret)
		drm_gem_vm_close(vma);

	return ret;
}

struct sg_table *ftd330_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);

#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE
	return drm_prime_pages_to_sg(obj->dev, ftd330_obj->pages, ftd330_obj->size >> PAGE_SHIFT);
#else
	return drm_prime_pages_to_sg(ftd330_obj->pages, ftd330_obj->size >> PAGE_SHIFT);
#endif
}

#if KERNEL_VERSION(5, 18, 0) <= LINUX_VERSION_CODE
static int ftd330_gem_prime_vmap(struct drm_gem_object *obj, struct iosys_map *map)
{
	struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);

	iosys_map_set_vaddr_iomem(map, ftd330_obj->cookie);
	return 0;
}
#elif KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
void *ftd330_gem_prime_vmap(struct drm_gem_object *obj)
{
        struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);
	return ftd330_obj->cookie;
}
#else
static int ftd330_gem_prime_vmap(struct drm_gem_object *obj, struct dma_buf_map *dma_buf_map)
{
        struct ftd330_gem_object *ftd330_obj = to_ftd330_gem_object(obj);

	iosys_map_set_vaddr_iomem(map, ftd330_obj->cookie);
        return 0;

}
#endif

#if KERNEL_VERSION(5, 18, 0) <= LINUX_VERSION_CODE
static void ftd330_gem_prime_vunmap(struct drm_gem_object *obj, struct iosys_map *map)
#elif KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
void ftd330_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
#else
static void ftd330_gem_prime_vunmap(struct drm_gem_object *obj, struct dma_buf_map *dma_buf_map)
#endif
{
	/* Nothing to do */
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
static const struct vm_operations_struct ftd330_vm_ops = {
	.open = drm_gem_vm_open,
	.close = drm_gem_vm_close,
};

static const struct drm_gem_object_funcs ftd330_gem_default_funcs = {
	.free = ftd330_gem_free_object,
	.get_sg_table = ftd330_gem_prime_get_sg_table,
	.vmap = ftd330_gem_prime_vmap,
	.vunmap = ftd330_gem_prime_vunmap,
	.vm_ops = &ftd330_vm_ops,
};
#endif

int ftd330_gem_dumb_create(struct drm_file *file, struct drm_device *dev,
		       struct drm_mode_create_dumb *args)
{
	struct ftd330_drm_private *priv = dev->dev_private;
	struct ftd330_gem_object *ftd330_obj;
	unsigned int pitch = DIV_ROUND_UP(args->width * args->bpp, 8);

	if (args->bpp % 10)
		args->pitch = ALIGN(pitch, priv->pitch_alignment);
	else
		/* for costum 10bit format with no bit gaps */
		args->pitch = pitch;
	args->size = PAGE_ALIGN(args->pitch * args->height);

	ftd330_obj = ftd330_gem_create_with_handle(dev, file, args->size, &args->handle);
	return PTR_ERR_OR_ZERO(ftd330_obj);
}

struct drm_gem_object *ftd330_gem_prime_import(struct drm_device *dev, struct dma_buf *dma_buf)
{
	return drm_gem_prime_import_dev(dev, dma_buf, to_dma_dev(dev));
}

struct drm_gem_object *ftd330_gem_prime_import_sg_table(struct drm_device *dev,
						    struct dma_buf_attachment *attach,
						    struct sg_table *sgt)
{
	struct ftd330_gem_object *ftd330_obj;
	int npages;
	int ret;
	size_t size = attach->dmabuf->size;
#ifndef CONFIG_PHYTIUM_MMU
	dma_addr_t expected;
	struct scatterlist *s;
	u32 i = 0;
#else
	u32 iova = 0;
	struct ftd330_drm_private *priv = dev->dev_private;

	if (!priv->mmu) {
		DRM_ERROR("invalid mmu.\n");
		ret = -EINVAL;
		return ERR_PTR(ret);
	}
#endif

	size = PAGE_ALIGN(size);

	ftd330_obj = ftd330_gem_alloc_object(dev, size);
	if (IS_ERR(ftd330_obj))
		return ERR_CAST(ftd330_obj);

#ifndef CONFIG_PHYTIUM_MMU
	expected = sg_dma_address(sgt->sgl);
	for_each_sg(sgt->sgl, s, sgt->nents, i) {
		if (sg_dma_address(s) != expected) {
			DRM_ERROR("sg_table is not contiguous");
			ret = -EINVAL;
			goto err;
		}

		if (sg_dma_len(s) & (PAGE_SIZE - 1)) {
			ret = -EINVAL;
			goto err;
		}


		if (i == 0)
			ftd330_obj->iova = (u64)sg_dma_address(s);

		expected = sg_dma_address(s) + sg_dma_len(s);
	}
#endif

	npages = ftd330_obj->size >> PAGE_SHIFT;
	ftd330_obj->pages = kvmalloc_array(npages, sizeof(struct page *), GFP_KERNEL);
	if (!ftd330_obj->pages) {
		ret = -ENOMEM;
		goto err;
	}

#if KERNEL_VERSION(5, 12, 0) > LINUX_VERSION_CODE
	ret = drm_prime_sg_to_page_addr_arrays(sgt, ftd330_obj->pages, NULL, npages);
#else
	ret = drm_prime_sg_to_page_array(sgt, ftd330_obj->pages, npages);
#endif

	if (ret)
		goto err_free_page;

#ifdef CONFIG_PHYTIUM_MMU
	ret = dc_mmu_map_memory_and_flush(dev, priv->mmu, (u64)ftd330_obj->pages, npages, &iova, false,
				false);
	if (ret) {
		DRM_ERROR("failed to do mmu map.\n");
		goto err;
	}
	ftd330_obj->iova = (u64)iova;
#endif
	ftd330_obj->dma_addr = sg_dma_address(sgt->sgl);
	ftd330_obj->sgt = sgt;

	FTD330_LOG("import sg_table dma_addr:0x%llx,mmu addr:0x%llx\n", ftd330_obj->dma_addr, ftd330_obj->iova);

	return &ftd330_obj->base;

err_free_page:
	if (ftd330_obj->pages) {
		kvfree(ftd330_obj->pages);
		ftd330_obj->pages = NULL;
	}
err:
	ftd330_gem_free_object(&ftd330_obj->base);

	return ERR_PTR(ret);
}

int ftd330_gem_prime_mmap(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	int ret = 0;

	ret = drm_gem_mmap_obj(obj, obj->size, vma);
	if (ret < 0)
		return ret;

	return ftd330_gem_mmap_obj(obj, vma);
}

int ftd330_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	struct drm_gem_object *obj;
	int ret;

	ret = drm_gem_mmap(filp, vma);
	if (ret)
		return ret;

	obj = vma->vm_private_data;

	if (obj->import_attach)
		return dma_buf_mmap(obj->dma_buf, vma, 0);

	return ftd330_gem_mmap_obj(obj, vma);
}

static int query_handle(struct drm_device *dev, struct drm_ftd330_gem_query_info *info,
			struct drm_file *file)
{
	struct drm_gem_object *obj;
	struct ftd330_gem_object *ftd330_obj;

	obj = drm_gem_object_lookup(file, info->handle);
	if (!obj) {
		dev_err(dev->dev, "Failed to GEM object with handle %#x.\n", info->handle);
		return -ENXIO;
	}
	ftd330_obj = to_ftd330_gem_object(obj);
	info->data = ftd330_obj->iova;
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	drm_gem_object_put(obj);
#else
	drm_gem_object_put_unlocked(obj);
#endif
	return 0;
}

int ftd330_gem_query_ioctl(struct drm_device *dev, void *data, struct drm_file *file)
{
	struct drm_ftd330_gem_query_info *info = data;

	switch (info->type) {
	case FTD330_GEM_QUERY_HANDLE:
		return query_handle(dev, info, file);
	default:
		dev_err(dev->dev, "Unknown type %#x.\n", info->type);
		break;
	}
	return -EINVAL;
}
struct ftd330_gem_object *ftd330_gem_object_lookup(u32 fd, u32 handle)
{
	struct drm_gem_object *bo;
	struct ftd330_gem_object *ftd330_bo = NULL;
	struct file *flip = fget(fd);
	struct drm_file *file_priv = flip->private_data;

	bo = drm_gem_object_lookup(file_priv, handle);
	if (bo)
		ftd330_bo = to_ftd330_gem_object(bo);
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	drm_gem_object_put(bo);
#else
	drm_gem_object_put_unlocked(bo);
#endif
	fput(flip);
	return ftd330_bo;
}
