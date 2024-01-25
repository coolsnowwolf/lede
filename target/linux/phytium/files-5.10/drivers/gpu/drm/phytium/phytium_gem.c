// SPDX-License-Identifier: GPL-2.0
/* Phytium display drm driver
 *
 * Copyright (c) 2021-2023 Phytium Technology Co., Ltd.
 */

#include <linux/dma-buf.h>
#include <linux/vmalloc.h>
#include <linux/version.h>
#include <linux/mm_types.h>
#include <linux/mm.h>
#include <linux/dma-mapping.h>
#include <linux/genalloc.h>
#include <drm/drm_prime.h>
#include <linux/dmaengine.h>
#include <linux/completion.h>
#include "phytium_display_drv.h"
#include "phytium_gem.h"

#define VRAM_POOL_ALLOC_ORDER 12

int phytium_memory_pool_alloc(struct phytium_display_private *priv, void **pvaddr,
					phys_addr_t *phys_addr, uint64_t size)
{
	unsigned long vaddr;

	vaddr = gen_pool_alloc(priv->memory_pool, size);
	if (!vaddr)
		return -ENOMEM;

	*phys_addr = gen_pool_virt_to_phys(priv->memory_pool, vaddr);

	*pvaddr = (void *)vaddr;
	return 0;
}

void phytium_memory_pool_free(struct phytium_display_private *priv, void *vaddr, uint64_t size)
{
	gen_pool_free(priv->memory_pool, (unsigned long)vaddr, size);
}

int phytium_memory_pool_init(struct device *dev, struct phytium_display_private *priv)
{
	int ret = 0;

	priv->memory_pool = gen_pool_create(VRAM_POOL_ALLOC_ORDER, -1);
	if (priv->memory_pool == NULL) {
		DRM_ERROR("fail to create memory pool\n");
		ret = -1;
		goto failed_create_pool;
	}

	ret = gen_pool_add_virt(priv->memory_pool, (unsigned long)priv->pool_virt_addr,
				priv->pool_phys_addr, priv->pool_size, -1);
	if (ret) {
		DRM_ERROR("fail to add vram pool\n");
		ret = -1;
		goto failed_add_pool_virt;
	}

	return 0;

failed_add_pool_virt:
	gen_pool_destroy(priv->memory_pool);

failed_create_pool:
	return ret;
}

void phytium_memory_pool_fini(struct device *dev, struct phytium_display_private *priv)
{
	gen_pool_destroy(priv->memory_pool);
}

struct sg_table *
phytium_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct phytium_gem_object *phytium_gem_obj = to_phytium_gem_obj(obj);
	struct sg_table *sgt;
	struct drm_device *dev = obj->dev;
	int ret;
	struct page *page = NULL;

	sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
	if (!sgt) {
		DRM_DEBUG_KMS("malloc sgt fail\n");
		return ERR_PTR(-ENOMEM);
	}

	if ((phytium_gem_obj->memory_type == MEMORY_TYPE_VRAM) ||
	    (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_CARVEOUT)) {
		ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
		if (ret) {
			DRM_ERROR("failed to allocate sg\n");
			goto sgt_free;
		}
		page = phys_to_page(phytium_gem_obj->phys_addr);
		sg_set_page(sgt->sgl, page, PAGE_ALIGN(phytium_gem_obj->size), 0);
	} else if (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_UNIFIED) {
		ret = dma_get_sgtable_attrs(dev->dev, sgt, phytium_gem_obj->vaddr,
				    phytium_gem_obj->iova, phytium_gem_obj->size,
				    DMA_ATTR_WRITE_COMBINE);
		if (ret) {
			DRM_ERROR("failed to allocate sgt, %d\n", ret);
			goto sgt_free;
		}
	}

	return sgt;
sgt_free:
	kfree(sgt);
	return ERR_PTR(ret);
}

struct drm_gem_object *
phytium_gem_prime_import_sg_table(struct drm_device *dev,
					       struct dma_buf_attachment *attach,
					       struct sg_table *sgt)
{
	struct phytium_gem_object *phytium_gem_obj = NULL;
	struct scatterlist *s;
	dma_addr_t expected;
	int ret, i;

	phytium_gem_obj = kzalloc(sizeof(*phytium_gem_obj), GFP_KERNEL);
	if (!phytium_gem_obj) {
		DRM_ERROR("failed to allocate phytium_gem_obj\n");
		ret = -ENOMEM;
		goto failed_malloc;
	}

	ret = drm_gem_object_init(dev, &phytium_gem_obj->base, attach->dmabuf->size);
	if (ret) {
		DRM_ERROR("failed to initialize drm gem object: %d\n", ret);
		goto failed_object_init;
	}

	expected = sg_dma_address(sgt->sgl);
	for_each_sg(sgt->sgl, s, sgt->nents, i) {
		if (sg_dma_address(s) != expected) {
			DRM_ERROR("sg_table is not contiguous");
			ret = -EINVAL;
			goto failed_check_continue;
		}
		expected = sg_dma_address(s) + sg_dma_len(s);
	}

	phytium_gem_obj->iova = sg_dma_address(sgt->sgl);
	phytium_gem_obj->sgt = sgt;

	return &phytium_gem_obj->base;
failed_check_continue:
	drm_gem_object_release(&phytium_gem_obj->base);
failed_object_init:
	kfree(phytium_gem_obj);
failed_malloc:
	return ERR_PTR(ret);
}

void *phytium_gem_prime_vmap(struct drm_gem_object *obj)
{
	struct phytium_gem_object *phytium_obj = to_phytium_gem_obj(obj);

	return phytium_obj->vaddr;
}

void phytium_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	return;
}

int phytium_gem_prime_mmap(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	int ret = 0;

	ret = drm_gem_mmap_obj(obj, obj->size, vma);
	if (ret < 0)
		return ret;

	return phytium_gem_mmap_obj(obj, vma);
}

static void phytium_dma_callback(void *callback_param)
{
	struct completion *comp = callback_param;

	complete(comp);
}

int phytium_dma_transfer(struct drm_device *drm_dev, int dev_to_mem, void *addr,
			 dma_addr_t iova, uint64_t size)
{
	struct phytium_display_private *priv = drm_dev->dev_private;
	struct dma_chan *dma_chan = priv->dma_chan;
	struct sg_table st;
	struct scatterlist *sgl;
	int ret = 0, timeout;
	uint32_t nents, i;
	struct dma_slave_config cfg = {0};
	struct dma_async_tx_descriptor *desc;
	struct completion comp;
	enum dma_data_direction dir;
	size_t min = 0;

	nents = DIV_ROUND_UP(size, PAGE_SIZE);
	ret = sg_alloc_table(&st, nents, GFP_KERNEL);
	if (ret) {
		DRM_ERROR("failed to allocate sg_table\n");
		ret = -ENOMEM;
		goto failed_sg_alloc_table;
	}

	for_each_sg(st.sgl, sgl, st.nents, i) {
		min = min_t(size_t, size, PAGE_SIZE - offset_in_page(addr));
		sg_set_page(sgl, vmalloc_to_page(addr), min, offset_in_page(addr));
		addr += min;
		size -= min;
	}

	memset(&cfg, 0, sizeof(cfg));
	if (dev_to_mem) {
		cfg.direction = DMA_DEV_TO_MEM;
		cfg.src_addr = iova;
		cfg.dst_addr = 0;
		dir = DMA_FROM_DEVICE;
	} else {
		cfg.direction = DMA_MEM_TO_DEV;
		cfg.src_addr = 0;
		cfg.dst_addr = iova;
		dir = DMA_TO_DEVICE;
	}

	dmaengine_slave_config(dma_chan, &cfg);

	nents = dma_map_sg(dma_chan->device->dev, st.sgl, st.nents, dir);
	if (!nents) {
		DRM_DEV_ERROR(drm_dev->dev, "failed to dma_map_sg for dmaengine\n");
		ret = -EINVAL;
		goto failed_dma_map_sg;
	}
	st.nents = nents;
	dma_sync_sg_for_device(dma_chan->device->dev, st.sgl, st.nents, dir);

	sgl = st.sgl;
	desc = dmaengine_prep_slave_sg(dma_chan,
					st.sgl,
					st.nents,
					cfg.direction,
					DMA_PREP_INTERRUPT | DMA_CTRL_ACK);
	if (!desc) {
		DRM_DEV_ERROR(drm_dev->dev, "failed to dmaengine_prep_slave_sg\n");
		ret = -EINVAL;
		goto failed_prep_slave_sg;
	}
	init_completion(&comp);
	desc->callback = phytium_dma_callback;
	desc->callback_param = &comp;

	dmaengine_submit(desc);
	dma_async_issue_pending(dma_chan);

	timeout = wait_for_completion_timeout(&comp, 2 * HZ);
	if (timeout == 0) {
		DRM_DEV_ERROR(drm_dev->dev, "wait for dma callback timeout\n");
		ret = -EIO;
	}
	dma_sync_sg_for_cpu(dma_chan->device->dev, st.sgl, st.nents, dir);

failed_prep_slave_sg:
	dma_unmap_sg(dma_chan->device->dev, st.sgl, st.nents, dir);
failed_dma_map_sg:
	sg_free_table(&st);
failed_sg_alloc_table:
	return ret;
}

int phytium_gem_suspend(struct drm_device *drm_dev)
{
	struct phytium_display_private *priv = drm_dev->dev_private;
	struct phytium_gem_object *phytium_gem_obj = NULL;
	int ret = 0;

	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		if (phytium_gem_obj->memory_type != MEMORY_TYPE_VRAM)
			continue;

		phytium_gem_obj->vaddr_save = vmalloc(phytium_gem_obj->size);
		if (!phytium_gem_obj->vaddr_save)
			goto malloc_failed;

		if (priv->dma_inited)
			ret = phytium_dma_transfer(drm_dev, 1, phytium_gem_obj->vaddr_save,
						   phytium_gem_obj->iova, phytium_gem_obj->size);

		if ((!priv->dma_inited) || ret)
			memcpy(phytium_gem_obj->vaddr_save, phytium_gem_obj->vaddr,
			       phytium_gem_obj->size);
	}

	return 0;
malloc_failed:
	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		if (phytium_gem_obj->memory_type != MEMORY_TYPE_VRAM)
			continue;

		if (phytium_gem_obj->vaddr_save) {
			vfree(phytium_gem_obj->vaddr_save);
			phytium_gem_obj->vaddr_save = NULL;
		}
	}
	return -ENOMEM;
}

void phytium_gem_resume(struct drm_device *drm_dev)
{
	struct phytium_display_private *priv = drm_dev->dev_private;
	struct phytium_gem_object *phytium_gem_obj = NULL;

	list_for_each_entry(phytium_gem_obj, &priv->gem_list_head, list) {
		if (phytium_gem_obj->memory_type != MEMORY_TYPE_VRAM)
			continue;

		memcpy(phytium_gem_obj->vaddr, phytium_gem_obj->vaddr_save, phytium_gem_obj->size);
		vfree(phytium_gem_obj->vaddr_save);
		phytium_gem_obj->vaddr_save = NULL;
	}
}

void phytium_gem_free_object(struct drm_gem_object *obj)
{
	struct phytium_gem_object *phytium_gem_obj = to_phytium_gem_obj(obj);
	struct drm_device *dev = obj->dev;
	struct phytium_display_private *priv = dev->dev_private;
	uint64_t size = phytium_gem_obj->size;

	DRM_DEBUG_KMS("free phytium_gem_obj iova:0x%pa size:0x%lx\n",
		      &phytium_gem_obj->iova, phytium_gem_obj->size);
	if (phytium_gem_obj->vaddr) {
		if (phytium_gem_obj->memory_type == MEMORY_TYPE_VRAM) {
			phytium_memory_pool_free(priv, phytium_gem_obj->vaddr, size);
			priv->mem_state[PHYTIUM_MEM_VRAM_ALLOC] -= size;
		} else if (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_CARVEOUT) {
			dma_unmap_page(dev->dev, phytium_gem_obj->iova, size, DMA_TO_DEVICE);
			phytium_memory_pool_free(priv, phytium_gem_obj->vaddr, size);
			priv->mem_state[PHYTIUM_MEM_SYSTEM_CARVEOUT_ALLOC] -= size;
		} else if (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_UNIFIED) {
			dma_free_attrs(dev->dev, size, phytium_gem_obj->vaddr,
				       phytium_gem_obj->iova, 0);
			priv->mem_state[PHYTIUM_MEM_SYSTEM_UNIFIED_ALLOC] -= size;
		}
		list_del(&phytium_gem_obj->list);
	} else if (obj->import_attach)
		drm_prime_gem_destroy(obj, phytium_gem_obj->sgt);
	drm_gem_object_release(obj);
	kfree(phytium_gem_obj);
}

int phytium_gem_mmap_obj(struct drm_gem_object *obj, struct vm_area_struct *vma)
{
	int ret = 0;
	struct phytium_gem_object *phytium_gem_obj = to_phytium_gem_obj(obj);
	unsigned long pfn = PHYS_PFN(phytium_gem_obj->phys_addr);
	/*
	 * Clear the VM_PFNMAP flag that was set by drm_gem_mmap(), and set the
	 * vm_pgoff (used as a fake buffer offset by DRM) to 0 as we want to map
	 * the whole buffer.
	 */
	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_pgoff = 0;
	vma->vm_page_prot = vm_get_page_prot(vma->vm_flags);

	if (phytium_gem_obj->memory_type == MEMORY_TYPE_VRAM) {
		vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
		ret = remap_pfn_range(vma, vma->vm_start, pfn,
			       vma->vm_end - vma->vm_start, vma->vm_page_prot);
	} else if (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_CARVEOUT) {
		ret = remap_pfn_range(vma, vma->vm_start, pfn,
				      vma->vm_end - vma->vm_start, vma->vm_page_prot);
	} else if (phytium_gem_obj->memory_type == MEMORY_TYPE_SYSTEM_UNIFIED) {
		ret = dma_mmap_attrs(obj->dev->dev, vma, phytium_gem_obj->vaddr,
				     phytium_gem_obj->iova, vma->vm_end - vma->vm_start, 0);
	}
	if (ret)
		drm_gem_vm_close(vma);

	return ret;
}

int phytium_gem_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;

	ret = drm_gem_mmap(filp, vma);
	if (ret < 0)
		return ret;

	return phytium_gem_mmap_obj(vma->vm_private_data, vma);
}

int phytium_gem_dumb_destroy(struct drm_file *file, struct drm_device *dev, uint32_t handle)
{
	return drm_gem_dumb_destroy(file, dev, handle);
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
static const struct vm_operations_struct phytium_vm_ops = {
	.open   = drm_gem_vm_open,
	.close  = drm_gem_vm_close,
};

static const struct drm_gem_object_funcs phytium_drm_gem_object_funcs = {
	.free = phytium_gem_free_object,
	.get_sg_table = phytium_gem_prime_get_sg_table,
	.vmap = phytium_gem_prime_vmap,
	.vunmap = phytium_gem_prime_vunmap,
	.vm_ops = &phytium_vm_ops,
};
#endif

struct phytium_gem_object *phytium_gem_create_object(struct drm_device *dev, unsigned long size)
{
	struct phytium_gem_object *phytium_gem_obj = NULL;
	struct phytium_display_private *priv = dev->dev_private;
	struct page *page = NULL;
	int ret = 0;

	phytium_gem_obj = kzalloc(sizeof(*phytium_gem_obj), GFP_KERNEL);
	if (!phytium_gem_obj) {
		DRM_ERROR("failed to allocate phytium_gem_obj\n");
		ret = -ENOMEM;
		goto error;
	}

	ret = drm_gem_object_init(dev, &phytium_gem_obj->base, size);
	if (ret) {
		DRM_ERROR("failed to initialize drm gem object: %d\n", ret);
		goto failed_object_init;
	}

	if (priv->support_memory_type & MEMORY_TYPE_VRAM) {
		ret = phytium_memory_pool_alloc(priv, &phytium_gem_obj->vaddr,
						&phytium_gem_obj->phys_addr, size);
		if (ret) {
			DRM_ERROR("fail to allocate vram buffer with size %lx\n", size);
			goto failed_dma_alloc;
		}
		phytium_gem_obj->iova = phytium_gem_obj->phys_addr;
		phytium_gem_obj->memory_type = MEMORY_TYPE_VRAM;
		priv->mem_state[PHYTIUM_MEM_VRAM_ALLOC] += size;
	} else if (priv->support_memory_type & MEMORY_TYPE_SYSTEM_CARVEOUT) {
		ret = phytium_memory_pool_alloc(priv, &phytium_gem_obj->vaddr,
						&phytium_gem_obj->phys_addr, size);
		if (ret) {
			DRM_ERROR("fail to allocate carveout memory with size %lx\n", size);
			goto failed_dma_alloc;
		}
		page = phys_to_page(phytium_gem_obj->phys_addr);
		phytium_gem_obj->iova = dma_map_page(dev->dev, page, 0, size, DMA_TO_DEVICE);
		if (dma_mapping_error(dev->dev, phytium_gem_obj->iova)) {
			DRM_ERROR("fail to dma map carveout memory with size %lx\n", size);
			phytium_memory_pool_free(priv, phytium_gem_obj->vaddr, size);
			ret = -ENOMEM;
			goto failed_dma_alloc;
		}
		phytium_gem_obj->memory_type = MEMORY_TYPE_SYSTEM_CARVEOUT;
		priv->mem_state[PHYTIUM_MEM_SYSTEM_CARVEOUT_ALLOC] += size;
	} else if (priv->support_memory_type & MEMORY_TYPE_SYSTEM_UNIFIED) {
		phytium_gem_obj->vaddr = dma_alloc_attrs(dev->dev, size, &phytium_gem_obj->iova,
					 GFP_KERNEL, 0);
		if (!phytium_gem_obj->vaddr) {
			DRM_ERROR("fail to allocate unified buffer with size %lx\n", size);
			ret = -ENOMEM;
			goto failed_dma_alloc;
		}
		phytium_gem_obj->memory_type = MEMORY_TYPE_SYSTEM_UNIFIED;
		priv->mem_state[PHYTIUM_MEM_SYSTEM_UNIFIED_ALLOC] += size;
	} else {
		DRM_ERROR("fail to allocate buffer with size %lx\n", size);
		ret = -ENOMEM;
		goto failed_dma_alloc;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	phytium_gem_obj->base.funcs = &phytium_drm_gem_object_funcs;
#endif

	phytium_gem_obj->size = size;
	list_add_tail(&phytium_gem_obj->list, &priv->gem_list_head);
	DRM_DEBUG_KMS("phytium_gem_obj iova:0x%pa size:0x%lx\n",
		       &phytium_gem_obj->iova, phytium_gem_obj->size);
	return phytium_gem_obj;

failed_dma_alloc:
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	drm_gem_object_put(&phytium_gem_obj->base);
#else
	drm_gem_object_unreference_unlocked(&phytium_gem_obj->base);
#endif

	return ERR_PTR(ret);
failed_object_init:
	kfree(phytium_gem_obj);
error:
	return ERR_PTR(ret);
}

int phytium_gem_dumb_create(struct drm_file *file, struct drm_device *dev,
				     struct drm_mode_create_dumb *args)
{
	int size = 0;
	struct phytium_gem_object *phytium_gem_obj = NULL;
	int ret = 0;

	args->pitch = ALIGN(args->width*DIV_ROUND_UP(args->bpp, 8), 128);
	args->size = args->pitch * args->height;
	size = PAGE_ALIGN(args->size);
	phytium_gem_obj = phytium_gem_create_object(dev, size);
	if (IS_ERR(phytium_gem_obj))
		return PTR_ERR(phytium_gem_obj);
	ret = drm_gem_handle_create(file, &phytium_gem_obj->base, &args->handle);
	if (ret) {
		DRM_ERROR("failed to drm_gem_handle_create\n");
		goto failed_gem_handle;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0))
	drm_gem_object_put(&phytium_gem_obj->base);
#else
	drm_gem_object_unreference_unlocked(&phytium_gem_obj->base);
#endif

	return 0;
failed_gem_handle:
	phytium_gem_free_object(&phytium_gem_obj->base);
	return ret;
}
