// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver DMA_BUF fence operation.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 or later at the following locations:
 *    http://www.opensource.org/licenses/gpl-license.html
 *    http://www.gnu.org/copyleft/gpl.html
 */

#include <drm/drm_cache.h>
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu_dec.h"
#include "ftv310e.h"
#include "ftv310_vpu_cache.h"
#include "ftv310_vpu_dec400.h"
#include "ftv310_vpu_vcmd.h"
#include "ftv310_vpu_mmu.h"
#include "ftv310_vpu.h"
#ifdef __amd64__
#include <asm/set_memory.h>
#endif


#include "ftv310_vpu_fpga_mem.h"
#include "ftv310_vpu_debugfs.h"
#include "ftv310_vpu_trace.h"

#ifdef PCIE_EN
#include "ftv310_vpu_pcie.h"
#endif

#define DRIVER_DESC "ftv310_vpu DRM"
#define DRIVER_DATE "20200228"
#define DRIVER_MAJOR 1
#define DRIVER_MINOR 1

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE
static const struct drm_gem_object_funcs ftv310_vpu_drm_gem_cma_funcs;
#endif
static int AllocateMem(struct drm_gem_ftv310_vpu_object *cma_obj, struct drm_mode_create_dumb *args);
static void FreeMem(struct drm_gem_ftv310_vpu_object *cma_obj);
static int ftv310_vpu_prime_pin(struct drm_gem_object *obj);
static void ftv310_vpu_unpin_pages(struct drm_gem_ftv310_vpu_object *bo);
extern long mem_start;

void record_vram_node(struct vram_record *r, void *vaddr, unsigned long size);
void remove_vram_node(struct vram_record *r);

static void ftv310_vpu_drm_fb_destroy(struct drm_framebuffer *fb)
{
	struct ftv310_vpu_drm_fb *PHY_fb = (struct ftv310_vpu_drm_fb *)fb;
	int i;

	for (i = 0; i < 4; i++)
		ftv310_vpu_unref_drmobj(PHY_fb->obj[i]);

	drm_framebuffer_cleanup(fb);
	kfree(PHY_fb);
}

static int ftv310_vpu_drm_fb_create_handle(struct drm_framebuffer *fb,
				       struct drm_file *file_priv,
				       unsigned int *handle)
{
	struct ftv310_vpu_drm_fb *PHY_fb = (struct ftv310_vpu_drm_fb *)fb;

	return drm_gem_handle_create(file_priv, PHY_fb->obj[0], handle);
}

static int ftv310_vpu_drm_fb_dirty(struct drm_framebuffer *fb,
			       struct drm_file *file, unsigned int flags,
			       unsigned int color, struct drm_clip_rect *clips,
			       unsigned int num_clips)
{
	/*nothing to do now*/
	return 0;
}

static const struct drm_framebuffer_funcs ftv310_vpu_drm_fb_funcs = {
	.destroy = ftv310_vpu_drm_fb_destroy,
	.create_handle = ftv310_vpu_drm_fb_create_handle,
	.dirty = ftv310_vpu_drm_fb_dirty,
};

static int ftv310_vpu_gem_dumb_create_internal(struct drm_file *file_priv,
					   struct drm_device *dev,
					   struct drm_mode_create_dumb *args)
{
	int ret = 0;
	int in_size, out_size;
	struct drm_gem_ftv310_vpu_object *cma_obj;
	struct drm_gem_object *obj;

	int min_pitch = DIV_ROUND_UP(args->width * args->bpp, 8);
	unsigned int sliceidx = args->handle;
	unsigned int config;

	struct slice_info *pslice = getslicenode(sliceidx);

	if (!pslice)
		return -EINVAL;

	config = pslice->config;

	args->handle = 0;
	if (mutex_lock_interruptible(&dev->struct_mutex))
		return -EBUSY;

	cma_obj = kzalloc(sizeof(*cma_obj), GFP_KERNEL);
	if (!cma_obj) {
		ret = -ENOMEM;
		goto out;
	}
	obj = &cma_obj->base;
	cma_obj->dmapriv.self = cma_obj;
	in_size = sizeof(*args);
	out_size = in_size;
	args->pitch = ALIGN(min_pitch, 64);
	args->size = (__u64)args->pitch * (__u64)args->height;
	args->size = PAGE_ALIGN(args->size);

	cma_obj->num_pages = args->size >> PAGE_SHIFT;
	cma_obj->flag = 0;
	cma_obj->pageaddr = NULL;
	cma_obj->vaddr = NULL;
	cma_obj->sliceidx = sliceidx;
	mutex_init(&cma_obj->pages_lock);

	ret = AllocateMem(cma_obj, args);
	if (ret != 0) {
		kfree(cma_obj);
		ret = -ENOMEM;
		goto out;
	}

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE
	if (!obj->funcs)
		obj->funcs = &ftv310_vpu_drm_gem_cma_funcs;
#endif
	drm_gem_object_init(dev, obj, args->size);
	ret = drm_gem_handle_create(file_priv, obj, &args->handle);
	ftv310_vpu_unref_drmobj(obj);

	if (ret) {
		FreeMem(cma_obj);
		kfree(cma_obj);
	}
	init_ftv310_vpu_resv(&cma_obj->kresv, cma_obj);
	cma_obj->handle = args->handle;

	trace_vcmd_buf_alloc(current->pid, args->size, args->handle, cma_obj->paddr);

out:
	mutex_unlock(&dev->struct_mutex);
	return ret;
}

static int ftv310_vpu_gem_dumb_create(struct drm_device *dev, void *data,
				  struct drm_file *file_priv)
{
	return ftv310_vpu_gem_dumb_create_internal(
		file_priv, dev, (struct drm_mode_create_dumb *)data);
}

static int ftv310_vpu_gem_dumb_map_offset(struct drm_file *file_priv,
				      struct drm_device *dev, uint32_t handle,
				      uint64_t *offset)
{
	struct drm_gem_object *obj;
	int ret;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, handle);
	if (!obj)
		return -EINVAL;

	ret = drm_gem_create_mmap_offset(obj);
	if (ret == 0) {
		*offset = (drm_vma_node_offset_addr(&obj->vma_node) + PHY_MMAP_ADDRES_CEIL);
		ftv310_vpu_mmaplog("create normal map offset %llx", *offset);
	}

	ftv310_vpu_unref_drmobj(obj);
	return ret;
}

#if KERNEL_VERSION(6, 4, 0) > LINUX_VERSION_CODE
static int ftv310_vpu_gem_dumb_destroy(struct drm_file *file,
				   struct drm_device *dev, u32 handle)
{
	return drm_gem_handle_delete(file, handle);
}
#endif

static int ftv310_vpu_destroy_dumb(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	struct drm_mode_destroy_dumb *args = data;
	struct drm_gem_object *obj;
	struct drm_gem_ftv310_vpu_object *cma_obj;

	if (mutex_lock_interruptible(&dev->struct_mutex))
		return -EBUSY;
	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, args->handle);
	if (!obj) {
		mutex_unlock(&dev->struct_mutex);
		return -EINVAL;
	}
	ftv310_vpu_unref_drmobj(obj);

	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	drm_gem_handle_delete(file_priv, args->handle);
	mutex_unlock(&dev->struct_mutex);
	return 0;
}

static struct sg_table *
ftv310_vpu_gem_prime_get_sg_table(struct drm_gem_object *obj)
{
	struct drm_gem_ftv310_vpu_object *cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	struct slice_info *pslice = getslicenode(cma_obj->sliceidx);
	struct sg_table *sgt = NULL;
	struct page *page = NULL;
	int ret = 0;

	if (!pslice)
		return NULL;
	if (cma_obj->flag & FTV310_VPU_GEM_FLAG_USEVMALLOC) {
		if (!cma_obj->pages)
			ftv310_vpu_prime_pin(obj);
		if (cma_obj->pages)
#if KERNEL_VERSION(5, 10, 0) > LINUX_VERSION_CODE
			sgt = drm_prime_pages_to_sg(cma_obj->pages, cma_obj->num_pages);
#else
			sgt = drm_prime_pages_to_sg(obj->dev, cma_obj->pages, cma_obj->num_pages);
#endif
	} else {
		sgt = kzalloc(sizeof(*sgt), GFP_KERNEL);
		if (!sgt)
			return NULL;

		ret = sg_alloc_table(sgt, 1, GFP_KERNEL);
		if (ret < 0) {
			DRM_ERROR("falied to allocate sg\n");
			kfree(sgt);
			sgt = NULL;
		}

		page = phys_to_page(cma_obj->paddr);
		sg_set_page(sgt->sgl, page, PAGE_ALIGN(obj->size), 0);
	}
	return sgt;
}

static struct drm_gem_object *
ftv310_vpu_gem_prime_import_sg_table(struct drm_device *dev,
				 struct dma_buf_attachment *attach,
				 struct sg_table *sgt)
{
	struct drm_gem_ftv310_vpu_object *cma_obj;
	struct drm_gem_object *obj;

	cma_obj = kzalloc(sizeof(*cma_obj), GFP_KERNEL);
	if (!cma_obj)
		return ERR_PTR(-ENOMEM);

	if (sgt->nents > 1) {
		int npages;

		npages = PAGE_ALIGN(attach->dmabuf->size) / PAGE_SIZE;

		/* check if the entries in the sg_table are contiguous */
		cma_obj->flag |= FTV310_VPU_GEM_FLAG_IMPORT; //make it use MMU
		if (!cma_obj->pages)
			cma_obj->pages = kvmalloc_array(npages, sizeof(struct page *), GFP_KERNEL);

		cma_obj->num_pages = attach->dmabuf->size >> PAGE_SHIFT;
		cma_obj->pages_pin_count++; /* perma-pinned */
#if KERNEL_VERSION(5, 12, 0) <= LINUX_VERSION_CODE
		drm_prime_sg_to_page_array(sgt, cma_obj->pages, npages);
#else
		drm_prime_sg_to_page_addr_arrays(sgt, cma_obj->pages, NULL, npages);
#endif
	}

	obj = &cma_obj->base;

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE
	if (!obj->funcs)
		obj->funcs = &ftv310_vpu_drm_gem_cma_funcs;
#endif

	if (drm_gem_object_init(dev, obj, attach->dmabuf->size) != 0) {
		kfree(cma_obj);
		return ERR_PTR(-ENOMEM);
	}
	cma_obj->paddr = sg_dma_address(sgt->sgl);
	cma_obj->sgt = sgt;
	cma_obj->num_pages = attach->dmabuf->size >> PAGE_SHIFT;
	cma_obj->dmapriv.meta_data =
		*((struct viv_vidmem_metadata *)attach->dmabuf->priv);
	cma_obj->dmapriv.self = cma_obj;
	cma_obj->dmapriv.meta_data.magic = FTV310_VPU_IMAGE_VIV_META_DATA_MAGIC;
	cma_obj->flag |= FTV310_VPU_GEM_FLAG_IMPORT;
	if ((GET_ENC_CFG_BITS(ftv310_vpu_dev.config) & CONFIG_FTV310_VPUMMU) ||
		(GET_DEC_CFG_BITS(ftv310_vpu_dev.config) & CONFIG_FTV310_VPUMMU))
		cma_obj->flag |= FTV310_VPU_GEM_FLAG_IMPORT; //make it use MMU

	return obj;
}

#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
static void *ftv310_vpu_gem_prime_vmap(struct drm_gem_object *obj)
{
	struct drm_gem_ftv310_vpu_object *cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	return cma_obj->vaddr;
}

static void ftv310_vpu_gem_prime_vunmap(struct drm_gem_object *obj, void *vaddr)
{
	struct drm_gem_ftv310_vpu_object *cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	ftv310_vpu_unpin_pages(cma_obj);
}

#else
static int ftv310_vpu_gem_prime_vmap(struct drm_gem_object *obj,
#if KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE
				 struct dma_buf_map *map)
#else
				 struct iosys_map *map)
#endif
{
	struct drm_gem_ftv310_vpu_object *cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

#if KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE
	dma_buf_map_set_vaddr(map, cma_obj->vaddr);
#else
	iosys_map_set_vaddr(map, cma_obj->vaddr);
#endif

	return 0;
}

static void ftv310_vpu_gem_prime_vunmap(struct drm_gem_object *obj,
#if KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE
				   struct dma_buf_map *map)
#else
				   struct iosys_map *map)
#endif
{
	struct drm_gem_ftv310_vpu_object *cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	ftv310_vpu_unpin_pages(cma_obj);
}
#endif

#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
static int ftv310_vpu_gem_prime_mmap(struct drm_gem_object *obj,
				 struct vm_area_struct *vma)
{
	struct drm_gem_ftv310_vpu_object *cma_obj;
	unsigned long page_num = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	int ret = 0;

	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	if (page_num > cma_obj->num_pages)
		return -EINVAL;

	if ((unsigned long)cma_obj->vaddr == 0)
		return -EINVAL;

	if(obj->size < vma->vm_end - vma->vm_start)
		return -EINVAL;
	
	drm_gem_object_get(obj);
	
	vma->vm_private_data = obj;
	vma->vm_ops = obj->funcs->vm_ops;

	if(!vma->vm_ops){
		ret = -EINVAL;
		drm_gem_object_put(obj);
		return ret;
	}

	vm_flags_set(vma, VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP);
	vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(vma->vm_flags));
	vma->vm_page_prot = pgprot_decrypted(vma->vm_page_prot);

#if KERNEL_VERSION(6, 3, 0) > LINUX_VERSION_CODE
	vma->vm_flags &= ~VM_PFNMAP;
#else
	vm_flags_clear(vma, VM_PFNMAP);
#endif
	vma->vm_pgoff = 0;
	if (mutex_lock_interruptible(&ftv310_vpu_dev.drm_dev->struct_mutex))
		return -EBUSY;
	if (dma_mmap_coherent(obj->dev->dev, vma, cma_obj->vaddr,
			      cma_obj->paddr, vma->vm_end - vma->vm_start)) {
		drm_gem_vm_close(vma);
		mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
		return -EAGAIN;
	}
	mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
	vma->vm_private_data = cma_obj;
	return ret;
}
#else
/* omitted in kernel version > 5.4.0
 *static struct reservation_object *ftv310_vpu_gem_prime_res_obj(
 *	struct drm_gem_object *obj)
 *{
 *	struct drm_gem_ftv310_vpu_object *hobj = to_drm_gem_ftv310_vpu_obj(obj);
 *
 *	return &hobj->kresv;
 *}
 */
static int ftv310_vpu_gem_prime_mmap(struct drm_gem_object *obj,
				 struct vm_area_struct *vma)
{
	struct drm_gem_ftv310_vpu_object *cma_obj;
	unsigned long page_num = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	int ret = 0;

	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	if (page_num > cma_obj->num_pages)
		return -EINVAL;

	if ((unsigned long)cma_obj->vaddr == 0)
		return -EINVAL;

	ret = drm_gem_mmap_obj(obj, obj->size, vma);
	if (ret < 0)
		return ret;

	vma->vm_flags &= ~VM_PFNMAP;
	vma->vm_pgoff = 0;
	if (mutex_lock_interruptible(&ftv310_vpu_dev.drm_dev->struct_mutex))
		return -EBUSY;
	if (dma_mmap_coherent(obj->dev->dev, vma, cma_obj->vaddr,
			      cma_obj->paddr, vma->vm_end - vma->vm_start)) {
		drm_gem_vm_close(vma);
		mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
		return -EAGAIN;
	}
	mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
	vma->vm_private_data = cma_obj;
	return ret;
}
#endif

static void ftv310_vpu_gem_free_object(struct drm_gem_object *gem_obj)
{
	struct drm_gem_ftv310_vpu_object *cma_obj;
	/* dma buf imported from others,
	 * release data structures allocated by ourselves
	 */

	cma_obj = to_drm_gem_ftv310_vpu_obj(gem_obj);

	if (gem_obj->import_attach) {
		if (cma_obj->mmu) {
        	struct mmu_addr_desc addr = {0};
            addr.size = cma_obj->num_pages << PAGE_SHIFT;
            if (cma_obj->pages)
            	addr.pages = cma_obj->pages;
            else
            	addr.bus_address = cma_obj->paddr;
            addr.virtual_address = cma_obj;
            ftv310_vpu_MMUClear(&addr, &cma_obj->mmu->g_mmu->region[0], &cma_obj->mmu->pg_tbl);
        }
		drm_prime_gem_destroy(gem_obj, cma_obj->sgt);
	} else if (cma_obj->vaddr) {
		FreeMem(cma_obj);
	}
	if (cma_obj->pages)
		kvfree(cma_obj->pages);
	mutex_destroy(&cma_obj->pages_lock);
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	dma_resv_fini(&cma_obj->kresv);
#else
	reservation_object_fini(&cma_obj->kresv);
#endif
	drm_gem_object_release(gem_obj);
	kfree(cma_obj);
}

static int ftv310_vpu_gem_close(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct drm_gem_close *args = data;
	int ret = 0;
	struct drm_gem_object *obj =
		ftv310_vpu_gem_object_lookup(dev, file_priv, args->handle);

	if (!obj)
		return -EINVAL;

	ret = drm_gem_handle_delete(file_priv, args->handle);
	ftv310_vpu_unref_drmobj(obj);
	return ret;
}

static int ftv310_vpu_gem_open(struct drm_device *dev, void *data,
			   struct drm_file *file_priv)
{
	int ret;
	u32 handle;
	struct drm_gem_open *openarg;
	struct drm_gem_object *obj = NULL;

	openarg = (struct drm_gem_open *)data;

	obj = idr_find(&dev->object_name_idr, (int)openarg->name);
	if (obj)
		ftv310_vpu_ref_drmobj(obj);
	else
		return -ENOENT;

	ret = drm_gem_handle_create(file_priv, obj, &handle);
	ftv310_vpu_unref_drmobj(obj);
	if (ret)
		return ret;

	openarg->handle = handle;
	openarg->size = obj->size;

	return ret;
}

static int ftv310_vpu_map_vaddr(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct ftv310_vpu_addrmap *pamap = data;
	struct drm_gem_object *obj;
	struct drm_gem_ftv310_vpu_object *cma_obj;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, pamap->handle);
	if (!obj)
		return -EINVAL;

	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	pamap->vm_addr = (unsigned long long)cma_obj->vaddr;
	pamap->phy_addr = cma_obj->paddr;
	pamap->mem_base = cma_obj->mem_base;

	ftv310_vpu_unref_drmobj(obj);
	return 0;
}

static int ftv310_vpu_get_hwcfg(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	return ftv310_vpu_dev.config;
}

static int ftv310_vpu_get_slicenum(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	int vcmd_en;
	ftv310_vpu_ioctl_id ioctl_id_par;

	ioctl_id_par.ID_PAR.node_idx = 0;
	vcmd_en = ftv310_vpu_get_vcmdsup(NULL, &ioctl_id_par.data, NULL);

#ifdef HAS_VCMD
		return get_vcmd_slice_num();
#endif
		return get_slicenumber();
}

/*reference linux 4.11. ubuntu 16.04 have issues in its drm_gem_flink_ioctl().
 * MODIFICATION:
 * drm_gem_object_lookup(file_priv, args->handle);
 * => drm_gem_object_lookup(dev, file_priv, args->handle);
 */
static int ftv310_vpu_gem_flink(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct drm_gem_flink *args = data;
	struct drm_gem_object *obj;
	int ret;

	if (!drm_core_check_feature(dev, DRIVER_GEM))
		return -ENODEV;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, args->handle);
	if (!obj)
		return -ENOENT;

	mutex_lock(&dev->object_name_lock);
	/* prevent races with concurrent gem_close. */
	if (obj->handle_count == 0) {
		ret = -ENOENT;
		goto err;
	}

	if (!obj->name) {
		ret = idr_alloc(&dev->object_name_idr, obj, 1, 0, GFP_KERNEL);
		if (ret < 0)
			goto err;

		obj->name = ret;
	}

	args->name = (uint64_t)obj->name;
	ret = 0;

err:
	mutex_unlock(&dev->object_name_lock);
	ftv310_vpu_unref_drmobj(obj);
	return ret;
}

static int ftv310_vpu_map_dumb(struct drm_device *dev, void *data,
			   struct drm_file *file_priv)
{
	int ret;
	struct drm_mode_map_dumb *temparg = (struct drm_mode_map_dumb *)data;

	ret = ftv310_vpu_gem_dumb_map_offset(file_priv, dev, temparg->handle,
					 &temparg->offset);

	return ret;
}

static int ftv310_vpu_drm_open(struct drm_device *dev, struct drm_file *file)
{
	struct idr *ptr;

	ptr = kzalloc(sizeof(*ptr), GFP_KERNEL);
	if (!ptr)
		return -ENOMEM;
	idr_init(ptr);
	file->driver_priv = ptr;
	return 0;
}

static void ftv310_vpu_drm_close(struct drm_device *dev, struct drm_file *file)
{
	struct idr *ptr = file->driver_priv;

	kfree(ptr);
	file->driver_priv = NULL;
}

static struct drm_gem_object *
ftv310_vpu_drm_gem_prime_import(struct drm_device *dev, struct dma_buf *dma_buf)
{
	struct device *attach_dev = dev->dev;
	struct dma_buf_attachment *attach;
	struct sg_table *sgt;
	struct drm_gem_object *obj;
	int ret;

	if (dma_buf->ops == &ftv310_vpu_dmabuf_ops) {
		obj = ftv310_vpu_get_gem_from_dmabuf(dma_buf);
		if (obj && obj->dev == dev) {
			drm_gem_object_get(obj);
			return obj;
		}
	}

	if (!dev->driver->gem_prime_import_sg_table)
		return ERR_PTR(-EINVAL);

	attach = dma_buf_attach(dma_buf, attach_dev);
	if (IS_ERR(attach))
		return ERR_CAST(attach);

	get_dma_buf(dma_buf);

	sgt = dma_buf_map_attachment(attach, DMA_BIDIRECTIONAL);
	if (IS_ERR(sgt)) {
		ret = PTR_ERR(sgt);
		goto fail_detach;
	}

	obj = dev->driver->gem_prime_import_sg_table(dev, attach, sgt);
	if (IS_ERR(obj)) {
		ret = PTR_ERR(obj);
		goto fail_unmap;
	}

	obj->import_attach = attach;
#if KERNEL_VERSION(5, 2, 0) <= LINUX_VERSION_CODE
	obj->resv = dma_buf->resv;
#endif
	return obj;

fail_unmap:
	dma_buf_unmap_attachment(attach, sgt, DMA_BIDIRECTIONAL);
fail_detach:
	dma_buf_detach(dma_buf, attach);
	dma_buf_put(dma_buf);

	return ERR_PTR(ret);
}

#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
static struct dma_buf *ftv310_vpu_prime_export(struct drm_gem_object *obj,
					   int flags)
#else
static struct dma_buf *ftv310_vpu_prime_export(struct drm_device *dev,
							struct drm_gem_object *obj, int flags)
#endif
{
	struct dma_buf *dma_buf;
	struct drm_gem_ftv310_vpu_object *cma_obj;
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	struct drm_device *dev = obj->dev;
#endif
	struct dma_buf_export_info exp_info = {
		.exp_name = KBUILD_MODNAME,
		.owner = dev->driver->fops->owner,
		.ops = &ftv310_vpu_dmabuf_ops,
		.flags = flags,
	};

	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	exp_info.resv = &cma_obj->kresv;
	exp_info.size = cma_obj->num_pages << PAGE_SHIFT;
	exp_info.priv = &cma_obj->dmapriv.meta_data;

	dma_buf = dma_buf_export(&exp_info);
	if (IS_ERR(dma_buf))
		return dma_buf;

	drm_dev_get(dev);
	drm_gem_object_get(&cma_obj->base);

	return dma_buf;
}

static int ftv310_vpu_handle_to_fd(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	int ret;
	struct drm_prime_handle *primeargs = (struct drm_prime_handle *)data;
	struct drm_gem_object *obj;
	struct drm_gem_ftv310_vpu_object *cma_obj;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, primeargs->handle);
	if (!obj)
		return -ENOENT;

	ret = drm_gem_prime_handle_to_fd(dev, file_priv, primeargs->handle,
					 primeargs->flags, &primeargs->fd);

	if (ret == 0) {
		cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
		cma_obj->flag |= FTV310_VPU_GEM_FLAG_EXPORT;
	}
	ftv310_vpu_unref_drmobj(obj);
	return ret;
}

static int ftv310_vpu_fd_to_handle(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	struct drm_prime_handle *primeargs = (struct drm_prime_handle *)data;
	s32 ret = 0;
	struct dma_buf *dma_buf;
	struct drm_gem_object *obj;
	struct viv_vidmem_metadata *meta_data;
	struct drm_gem_ftv310_vpu_object *cma_obj;

	primeargs->flags = 0;
	ret = drm_gem_prime_fd_to_handle(dev, file_priv, primeargs->fd,
					 &primeargs->handle);

	/*Just for debugging to get object*/
	if (0) {
		dma_buf = dma_buf_get(primeargs->fd);
		if (IS_ERR(dma_buf))
			return PTR_ERR(dma_buf);

		if (dev->driver->gem_prime_import)
			obj = dev->driver->gem_prime_import(dev, dma_buf);
		else
			obj = drm_gem_prime_import(dev, dma_buf);

		cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
		meta_data = dma_buf->priv;
		dma_buf_put(dma_buf);
		pr_debug(
			"client[%p]import  gem name[%u] gem size[%zu] handle_cnt[%d] refcnt[%d] flag[0x%x]\n",
			file_priv, obj->name, obj->size, obj->handle_count,
			kref_read(&obj->refcount), cma_obj->flag);
	}

	return ret;
}

static int ftv310_vpu_fb_create2(struct drm_device *dev, void *data,
			     struct drm_file *file_priv)
{
	struct drm_mode_fb_cmd2 *mode_cmd = (struct drm_mode_fb_cmd2 *)data;
	struct ftv310_vpu_drm_fb *PHYfb;
	struct drm_gem_object *objs[4];
	struct drm_gem_object *obj;
#if KERNEL_VERSION(4, 16, 0) <= LINUX_VERSION_CODE
	const struct drm_format_info *info = drm_get_format_info(dev, mode_cmd);
#endif
	unsigned int hsub;
	unsigned int vsub;
	int num_planes;
	int ret;
	int i;

#if KERNEL_VERSION(4, 16, 0) <= LINUX_VERSION_CODE
	hsub = info->hsub;
	vsub = info->vsub;
	num_planes = min_t(int, info->num_planes, 4);
#else
	hsub = drm_format_horz_chroma_subsampling(mode_cmd->pixel_format);
	vsub = drm_format_vert_chroma_subsampling(mode_cmd->pixel_format);
	num_planes = min(drm_format_num_planes(mode_cmd->pixel_format), 4);
#endif
	for (i = 0; i < num_planes; i++) {
		unsigned int width = mode_cmd->width / (i ? hsub : 1);
		unsigned int height = mode_cmd->height / (i ? vsub : 1);
		unsigned int min_size;

		obj = ftv310_vpu_gem_object_lookup(dev, file_priv,
					       mode_cmd->handles[i]);
		if (!obj) {
			ret = -ENXIO;
			goto err_gem_object_unreference;
		}
		ftv310_vpu_unref_drmobj(obj);
		min_size = (height - 1) * mode_cmd->pitches[i] +
			   mode_cmd->offsets[i] +
#if KERNEL_VERSION(4, 16, 0) <= LINUX_VERSION_CODE
			   width * info->cpp[i];
#else
			   width * drm_format_plane_cpp(mode_cmd->pixel_format,
							i);
#endif
		if (obj->size < min_size) {
			ret = -EINVAL;
			goto err_gem_object_unreference;
		}
		objs[i] = obj;
	}
	PHYfb = kzalloc(sizeof(*PHYfb), GFP_KERNEL);
	if (!PHYfb)
		return -ENOMEM;
	drm_helper_mode_fill_fb_struct(dev, &PHYfb->fb, mode_cmd);
	for (i = 0; i < num_planes; i++)
		PHYfb->obj[i] = objs[i];
	ret = drm_framebuffer_init(dev, &PHYfb->fb, &ftv310_vpu_drm_fb_funcs);
	if (ret)
		kfree(PHYfb);
	return ret;

err_gem_object_unreference:
	for (i--; i >= 0; i--)
		; //ftv310_vpu_unref_drmobj(objs[i]);

	return ret;
}

static int ftv310_vpu_fb_create(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct drm_mode_fb_cmd *cmd_or = data;
	struct drm_mode_fb_cmd2 r = {};
	int ret;

	/* convert to new format and call new ioctl */
	r.fb_id = cmd_or->fb_id;
	r.width = cmd_or->width;
	r.height = cmd_or->height;
	r.pitches[0] = cmd_or->pitch;
	r.pixel_format = drm_mode_legacy_fb_format(cmd_or->bpp, cmd_or->depth);
	r.handles[0] = cmd_or->handle;

	ret = ftv310_vpu_fb_create2(dev, &r, file_priv);
	if (ret)
		return ret;

	cmd_or->fb_id = r.fb_id;

	return 0;
}

static int ftv310_vpu_get_version(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	struct drm_version *pversion;
	char *sname = DRIVER_NAME;
	char *sdesc = DRIVER_DESC;
	char *sdate = DRIVER_DATE;

	pversion = (struct drm_version *)data;
	pversion->version_major = dev->driver->major;
	pversion->version_minor = dev->driver->minor;
	pversion->version_patchlevel = 0;
	pversion->name_len = strlen(DRIVER_NAME);
	pversion->desc_len = strlen(DRIVER_DESC);
	pversion->date_len = strlen(DRIVER_DATE);
	if (pversion->name)
		if (copy_to_user(pversion->name, sname, pversion->name_len))
			return -EFAULT;
	if (pversion->date)
		if (copy_to_user(pversion->date, sdate, pversion->date_len))
			return -EFAULT;
	if (pversion->desc)
		if (copy_to_user(pversion->desc, sdesc, pversion->desc_len))
			return -EFAULT;
	return 0;
}

static int ftv310_vpu_get_cap(struct drm_device *dev, void *data,
			  struct drm_file *file_priv)
{
	struct drm_get_cap *req = (struct drm_get_cap *)data;

	req->value = 0;
	/*some values should be reset*/
	switch (req->capability) {
	case DRM_CAP_PRIME:
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
		req->value |= dev->driver->prime_fd_to_handle ?
				      DRM_PRIME_CAP_IMPORT :
				      0;
		req->value |= dev->driver->prime_handle_to_fd ?
				      DRM_PRIME_CAP_EXPORT :
				      0;
#else
		req->value = DRM_PRIME_CAP_IMPORT | DRM_PRIME_CAP_EXPORT;
#endif
		return 0;
	case DRM_CAP_DUMB_BUFFER:
		req->value = 1;
		break;
	case DRM_CAP_VBLANK_HIGH_CRTC:
		req->value = 1;
		break;
	case DRM_CAP_DUMB_PREFERRED_DEPTH:
		req->value = dev->mode_config.preferred_depth;
		break;
	case DRM_CAP_DUMB_PREFER_SHADOW:
		req->value = dev->mode_config.prefer_shadow;
		break;
	case DRM_CAP_ASYNC_PAGE_FLIP:
		req->value = dev->mode_config.async_page_flip;
		break;
	case DRM_CAP_CURSOR_WIDTH:
		if (dev->mode_config.cursor_width)
			req->value = dev->mode_config.cursor_width;
		else
			req->value = 64;
		break;
	case DRM_CAP_CURSOR_HEIGHT:
		if (dev->mode_config.cursor_height)
			req->value = dev->mode_config.cursor_height;
		else
			req->value = 64;
		break;
	case DRM_CAP_ADDFB2_MODIFIERS:
#if KERNEL_VERSION(5, 18, 0) > LINUX_VERSION_CODE
		req->value = dev->mode_config.allow_fb_modifiers;
#else
		// for kernel 6.1 compile 
		req->value = dev->mode_config.fb_modifiers_not_supported;
#endif
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

/*just a test API for any purpose*/
static int ftv310_vpu_test(struct drm_device *dev, void *data,
		       struct drm_file *file_priv)
{
	unsigned int *input = data;
	int handle = *input;
	struct drm_gem_object *obj;
	ftv310_vpu_fence_t *pfence;
	int ret = 10 * HZ; /*timeout*/

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, handle);
	if (!obj)
		return -EINVAL;

#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
	pfence = reservation_object_get_excl(obj->dma_buf->resv);
#elif (KERNEL_VERSION(5, 14, 0) > LINUX_VERSION_CODE) && (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	pfence = dma_resv_get_excl(obj->dma_buf->resv);
#elif KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE && (KERNEL_VERSION(5, 14, 0) <= LINUX_VERSION_CODE)
	pfence = dma_resv_excl_fence(obj->dma_buf->resv);
#else
	// for kernel 6.1 compile
	 ftv310_vpu_fence_t **fence;

	dma_resv_get_fences(obj->dma_buf->resv, 0, NULL, &fence);
	pfence = *fence;	
#endif
	while (ret > 0)
		ret = schedule_timeout(ret);

	ftv310_vpu_fence_signal(pfence);
	ftv310_vpu_unref_drmobj(obj);
	return 0;
}

static int ftv310_vpu_getprimeaddr(struct drm_device *dev, void *data,
			       struct drm_file *file_priv)
{
	unsigned long *input = data;
	int fd = *input;
	struct drm_gem_ftv310_vpu_object *cma_obj;
	struct drm_gem_object *obj;
	struct dma_buf *dma_buf;

	dma_buf = dma_buf_get(fd);
	if (IS_ERR(dma_buf))
		return PTR_ERR(dma_buf);

	if (dev->driver->gem_prime_import)
		obj = dev->driver->gem_prime_import(dev, dma_buf);
	else
		obj = drm_gem_prime_import(dev, dma_buf);
	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	*input = cma_obj->paddr;
	dma_buf_put(dma_buf);
	return 0;
}

static int ftv310_vpu_ptr_to_phys(struct drm_device *dev, void *data,
			      struct drm_file *file_priv)
{
	unsigned long *arg = data;
	struct vm_area_struct *vma;
	struct drm_gem_ftv310_vpu_object *cma_obj;
	unsigned long vaddr = *arg;

	vma = find_vma(current->mm, vaddr);
	if (!vma)
		return -EFAULT;

	cma_obj = (struct drm_gem_ftv310_vpu_object *)vma->vm_private_data;
	if (!cma_obj)
		return -EFAULT;

	if (cma_obj->base.dev != dev)
		return -EFAULT;

	if (vaddr < vma->vm_start ||
	    vaddr >= vma->vm_start + (cma_obj->num_pages << PAGE_SHIFT))
		return -EFAULT;

	*arg = (phys_addr_t)(vaddr - vma->vm_start) + cma_obj->paddr;
	return 0;
}

static int ftv310_vpu_query_metadata(struct drm_device *dev, void *data,
				 struct drm_file *file_priv)
{
	struct ftv310_vpu_metadata_params *metadata_p =
		(struct ftv310_vpu_metadata_params *)data;
	struct drm_gem_object *obj = NULL;
	struct drm_gem_ftv310_vpu_object *cma_obj = NULL;
	u32 i = 0;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, metadata_p->handle);
	if (!obj)
		return -ENOENT;
	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);
	for (i = 0; i < 3; i++) {
	//ftv310_vpu_query_metadata and ftv310_vpu_update_metadata must in pair when having ts_fd
		if (cma_obj->dmapriv.meta_data.plane[i].ts_fd &&
			cma_obj->dmapriv.meta_data.plane[i].ts_dma_buf) {
			cma_obj->dmapriv.meta_data.plane[i].ts_fd =
				dma_buf_fd(cma_obj->dmapriv.meta_data.plane[i].ts_dma_buf,
					DRM_CLOEXEC | DRM_RDWR);
		}
	}

	memcpy(&metadata_p->meta_data, &cma_obj->dmapriv.meta_data,
	       sizeof(struct viv_vidmem_metadata));

	ftv310_vpu_unref_drmobj(obj);

	return 0;
}

static int ftv310_vpu_update_metadata(struct drm_device *dev, void *data,
				  struct drm_file *file_priv)
{
	struct ftv310_vpu_metadata_params *metadata_p =
		(struct ftv310_vpu_metadata_params *)data;
	struct drm_gem_object *obj = NULL;
	struct drm_gem_ftv310_vpu_object *cma_obj = NULL;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, metadata_p->handle);
	if (!obj)
		return -ENOENT;
	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	memcpy(&cma_obj->dmapriv.meta_data, &metadata_p->meta_data,
	       sizeof(struct viv_vidmem_metadata));

	if (metadata_p->meta_data.plane[0].ts_fd >= 0)
		cma_obj->dmapriv.meta_data.plane[0].ts_dma_buf =
			dma_buf_get(metadata_p->meta_data.plane[0].ts_fd);
	if (metadata_p->meta_data.plane[1].ts_fd >= 0)
		cma_obj->dmapriv.meta_data.plane[1].ts_dma_buf =
			dma_buf_get(metadata_p->meta_data.plane[1].ts_fd);
	if (metadata_p->meta_data.plane[2].ts_fd >= 0)
		cma_obj->dmapriv.meta_data.plane[2].ts_dma_buf =
			dma_buf_get(metadata_p->meta_data.plane[2].ts_fd);

	ftv310_vpu_unref_drmobj(obj);

	return 0;
}

static int ftv310_vpu_getmagic(struct drm_device *dev, void *data,
			   struct drm_file *file_priv)
{
	struct drm_auth *auth = data;
	int ret = 0;

	mutex_lock(&dev->struct_mutex);
	if (!file_priv->magic) {
		ret = idr_alloc(&file_priv->master->magic_map, file_priv, 1, 0,
				GFP_KERNEL);
		if (ret >= 0)
			file_priv->magic = ret;
	}
	auth->magic = file_priv->magic;
	DBG("kmagic %d\n", auth->magic);
	mutex_unlock(&dev->struct_mutex);

	return ret < 0 ? ret : 0;
}

static int ftv310_vpu_authmagic(struct drm_device *dev, void *data,
			    struct drm_file *file_priv)
{
	struct drm_auth *auth = data;
	struct drm_file *file;

	mutex_lock(&dev->struct_mutex);
	file = idr_find(&file_priv->master->magic_map, auth->magic);
	DBG("get kmagic %d\n", auth->magic);
	if (file) {
		file->authenticated = 1;
		idr_replace(&file_priv->master->magic_map, NULL, auth->magic);
	}
	mutex_unlock(&dev->struct_mutex);

	return file ? 0 : -EINVAL;
}

#define DRM_IOCTL_DEF(ioctl, _func, _flags)                                    \
	[DRM_IOCTL_NR(ioctl)] = {                                              \
		.cmd = ioctl, .func = _func, .flags = _flags, .name = #ioctl   \
	}

/*after kernel 4.16 this definition is removed*/
#ifndef DRM_CONTROL_ALLOW
#define DRM_CONTROL_ALLOW 0
#endif
/* Ioctl table */
static const struct drm_ioctl_desc ftv310_vpu_ioctls[] = {
	DRM_IOCTL_DEF(DRM_IOCTL_VERSION, ftv310_vpu_get_version,
		      DRM_UNLOCKED | DRM_RENDER_ALLOW | DRM_CONTROL_ALLOW),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_UNIQUE, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_MAGIC, ftv310_vpu_getmagic, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_IRQ_BUSID, drm_invalid_op,
		      DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_MAP, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_CLIENT, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_STATS, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_CAP, ftv310_vpu_get_cap,
		      DRM_UNLOCKED | DRM_RENDER_ALLOW),
		  
	DRM_IOCTL_DEF(DRM_IOCTL_SET_CLIENT_CAP, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_SET_VERSION, drm_invalid_op,
		      DRM_UNLOCKED | DRM_MASTER),

	DRM_IOCTL_DEF(DRM_IOCTL_SET_UNIQUE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_BLOCK, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_UNBLOCK, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AUTH_MAGIC, ftv310_vpu_authmagic,
		      DRM_AUTH | DRM_UNLOCKED | DRM_MASTER),

	DRM_IOCTL_DEF(DRM_IOCTL_ADD_MAP, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_RM_MAP, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_SET_SAREA_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_SAREA_CTX, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_SET_MASTER, drm_invalid_op,
		      DRM_UNLOCKED | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_DROP_MASTER, drm_invalid_op,
		      DRM_UNLOCKED | DRM_ROOT_ONLY),

	DRM_IOCTL_DEF(DRM_IOCTL_ADD_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_RM_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_MOD_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_GET_CTX, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_SWITCH_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_NEW_CTX, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_RES_CTX, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_ADD_DRAW, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_RM_DRAW, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),

	DRM_IOCTL_DEF(DRM_IOCTL_LOCK, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_UNLOCK, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_FINISH, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_ADD_BUFS, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_MARK_BUFS, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_INFO_BUFS, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_MAP_BUFS, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_FREE_BUFS, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_DMA, drm_invalid_op, DRM_AUTH),

	DRM_IOCTL_DEF(DRM_IOCTL_CONTROL, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),

#if IS_ENABLED(CONFIG_AGP)
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_ACQUIRE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_RELEASE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_ENABLE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_INFO, drm_invalid_op, DRM_AUTH),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_ALLOC, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_FREE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_BIND, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_AGP_UNBIND, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
#endif

	DRM_IOCTL_DEF(DRM_IOCTL_SG_ALLOC, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),
	DRM_IOCTL_DEF(DRM_IOCTL_SG_FREE, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),

	DRM_IOCTL_DEF(DRM_IOCTL_WAIT_VBLANK, drm_invalid_op, DRM_UNLOCKED),

	DRM_IOCTL_DEF(DRM_IOCTL_MODESET_CTL, drm_invalid_op, 0),

	DRM_IOCTL_DEF(DRM_IOCTL_UPDATE_DRAW, drm_invalid_op,
		      DRM_AUTH | DRM_MASTER | DRM_ROOT_ONLY),

	DRM_IOCTL_DEF(DRM_IOCTL_GEM_CLOSE, ftv310_vpu_gem_close,
		      DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF(DRM_IOCTL_GEM_FLINK, ftv310_vpu_gem_flink,
		      DRM_AUTH | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_GEM_OPEN, ftv310_vpu_gem_open,
		      DRM_AUTH | DRM_UNLOCKED),

	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETRESOURCES, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),

	DRM_IOCTL_DEF(DRM_IOCTL_PRIME_HANDLE_TO_FD, ftv310_vpu_handle_to_fd,
		      DRM_AUTH | DRM_UNLOCKED | DRM_RENDER_ALLOW),
	DRM_IOCTL_DEF(DRM_IOCTL_PRIME_FD_TO_HANDLE, ftv310_vpu_fd_to_handle,
		      DRM_AUTH | DRM_UNLOCKED | DRM_RENDER_ALLOW),

	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETPLANERESOURCES, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETCRTC, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_SETCRTC, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETPLANE, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_SETPLANE, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_CURSOR, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETGAMMA, drm_invalid_op, DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_SETGAMMA, drm_invalid_op,
		      DRM_MASTER | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETENCODER, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETCONNECTOR, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_ATTACHMODE, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_DETACHMODE, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETPROPERTY, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_SETPROPERTY, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETPROPBLOB, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_GETFB, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_ADDFB, ftv310_vpu_fb_create,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_ADDFB2, ftv310_vpu_fb_create2,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_RMFB, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_PAGE_FLIP, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_DIRTYFB, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_CREATE_DUMB, ftv310_vpu_gem_dumb_create,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_MAP_DUMB, ftv310_vpu_map_dumb,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_DESTROY_DUMB, ftv310_vpu_destroy_dumb,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_OBJ_GETPROPERTIES, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_OBJ_SETPROPERTY, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_CURSOR2, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_ATOMIC, drm_invalid_op,
		      DRM_MASTER | DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_CREATEPROPBLOB, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_MODE_DESTROYPROPBLOB, drm_invalid_op,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),

	/*ftv310_vpu specific ioctls*/
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_TESTCMD, ftv310_vpu_test,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GETPADDR, ftv310_vpu_map_vaddr,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_HWCFG, ftv310_vpu_get_hwcfg,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_TESTREADY, ftv310_vpu_testbufvalid,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_SETDOMAIN, ftv310_vpu_setdomain,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_ACQUIREBUF, ftv310_vpu_acquirebuf,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_RELEASEBUF, ftv310_vpu_releasebuf,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GETPRIMEADDR, ftv310_vpu_getprimeaddr,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_PTR_PHYADDR, ftv310_vpu_ptr_to_phys,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_QUERY_METADATA, ftv310_vpu_query_metadata,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_UPDATE_METADATA, ftv310_vpu_update_metadata,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),

	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GET_SLICENUM, ftv310_vpu_get_slicenum,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GET_VCMDSUP, ftv310_vpu_get_vcmdsup,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GET_IRQINFO, ftv310_vpu_get_irqinfo,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
	DRM_IOCTL_DEF(DRM_IOCTL_FTV310_VPU_GET_PMSUP, ftv310_vpu_get_pmsup,
		      DRM_CONTROL_ALLOW | DRM_UNLOCKED),
};

#if DRM_CONTROL_ALLOW == 0
#undef DRM_CONTROL_ALLOW
#endif

#define FTV310_VPU_IOCTL_COUNT ARRAY_SIZE(ftv310_vpu_ioctls)
static long ftv310_vpu_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct drm_file *file_priv = filp->private_data;
	struct drm_device *dev = ftv310_vpu_dev.drm_dev;
	const struct drm_ioctl_desc *ioctl = NULL;
	drm_ioctl_t *func;
	unsigned int nr = _IOC_NR(cmd);
	int retcode = 0;
	char stack_kdata[256];
	char *kdata = stack_kdata;
	unsigned int in_size, out_size;

#if KERNEL_VERSION(4, 15, 0) <= LINUX_VERSION_CODE
	if (drm_dev_is_unplugged(dev))
		return -ENODEV;
#else
	if (drm_device_is_unplugged(dev))
		return -ENODEV;
#endif

	in_size = _IOC_SIZE(cmd);
	out_size = in_size;
	pr_debug("ioctl cmd %d:%d\n", _IOC_TYPE(cmd), nr);

	if (in_size > 0) {
		if (_IOC_DIR(cmd) & _IOC_READ)
			retcode = !ftv310_vpu_access_ok(VERIFY_WRITE, (void *)arg,
						    in_size);
		else if (_IOC_DIR(cmd) & _IOC_WRITE)
			retcode = !ftv310_vpu_access_ok(VERIFY_READ, (void *)arg,
						    in_size);
		if (retcode)
			return -EFAULT;
	}
	if (_IOC_TYPE(cmd) == FTV310_VPU_IOC_MAGIC) {
		if (nr >= _IOC_NR(FTV310_VPUENC_IOC_START) &&
		    nr <= _IOC_NR(FTV310_VPUENC_IOC_END)) {
#ifdef HAS_VCE
			return ftv310_vpu_enc_ioctl(filp, cmd, arg);
#else
			if (cmd == FTV310_VPUENC_IOCG_CORE_NUM) {
				int corenum = 0;

				__put_user(corenum, (unsigned int *)arg);
			} else {
				return -EFAULT;
			}
#endif
		}
		if (nr >= _IOC_NR(FTV310_VPUDEC_IOC_START) &&
		    nr <= _IOC_NR(FTV310_VPUDEC_IOC_END)) {
#ifdef HAS_VCD
			return ftv310_vpu_dec_ioctl(filp, cmd, arg);
#else
			return -EFAULT;
#endif
		}

		if (nr >= _IOC_NR(FTV310_VPUCACHE_IOC_START) &&
		    nr <= _IOC_NR(FTV310_VPUCACHE_IOC_END)) {
#ifdef HAS_CACHECORE
			return ftv310_vpu_cache_ioctl(filp, cmd, arg);
#else
			return -EFAULT;
#endif
		}

		if (nr >= _IOC_NR(FTV310_VPUDEC400_IOC_START) &&
		    nr <= _IOC_NR(FTV310_VPUDEC400_IOC_END)) {
#ifdef HAS_DEC400
			return ftv310_vpu_dec400_ioctl(filp, cmd, arg);
#else
			return -EFAULT;
#endif
		}

		if (nr >= _IOC_NR(FTV310_VPUMMU_IOC_START) &&
		    nr <= _IOC_NR(FTV310_VPUMMU_IOC_END)) {
#ifdef HAS_MMU
			if ((GET_ENC_CFG_BITS(ftv310_vpu_dev.config) & CONFIG_FTV310_VPUMMU)
				|| (GET_DEC_CFG_BITS(ftv310_vpu_dev.config) & CONFIG_FTV310_VPUMMU))
				return ftv310_vpu_MMUIoctl(cmd, filp, arg);
			else
				return -EFAULT;
#else
			return -EFAULT;
#endif
		}
#ifdef HAS_VCMD
		if (nr >= _IOC_NR(VCMD_IOC_START) &&
		    nr <= _IOC_NR(VCMD_IOC_END)) {
			return vcmd_ioctl(filp, cmd, arg);
		}
#endif
	} else if (_IOC_TYPE(cmd) == DRM_IOCTL_BASE) {
		if (nr >= FTV310_VPU_IOCTL_COUNT)
			return -EINVAL;
		ioctl = &ftv310_vpu_ioctls[nr];

		if (cmd == DRM_IOCTL_FTV310_VPU_UPDATE_METADATA ||
		    cmd == DRM_IOCTL_FTV310_VPU_QUERY_METADATA) {
			if (sizeof(stack_kdata) <
			    sizeof(struct ftv310_vpu_metadata_params)) {
				pr_err("%s arg size is too large,sizeof(stack_kdata) %ld,in_size %ld\n",
				       __func__, sizeof(stack_kdata),
				       sizeof(struct ftv310_vpu_metadata_params));
				return -EFAULT;
			}
			if (copy_from_user(
				    kdata, (void __user *)arg,
				    sizeof(struct ftv310_vpu_metadata_params)) != 0)
				return -EFAULT;
		} else {
			if (sizeof(stack_kdata) < in_size) {
				pr_err("%s arg size is too large,sizeof(stack_kdata) %ld,in_size %d\n",
				       __func__, sizeof(stack_kdata), in_size);
				return -EFAULT;
			}
			if (copy_from_user(kdata, (void __user *)arg,
					   in_size) != 0)
				return -EFAULT;
		}

		if (cmd == DRM_IOCTL_MODE_SETCRTC ||
		    cmd == DRM_IOCTL_MODE_GETRESOURCES ||
		    cmd == DRM_IOCTL_SET_CLIENT_CAP ||
		    cmd == DRM_IOCTL_MODE_GETCRTC ||
		    cmd == DRM_IOCTL_MODE_GETENCODER ||
		    cmd == DRM_IOCTL_MODE_GETCONNECTOR ||
		    cmd == DRM_IOCTL_MODE_GETFB) {
			retcode = drm_ioctl(filp, cmd, arg);
			return retcode;
		}
		func = ioctl->func;
		if (!func)
			return -EINVAL;
		retcode = func(dev, kdata, file_priv);

		if (cmd == DRM_IOCTL_FTV310_VPU_UPDATE_METADATA ||
		    cmd == DRM_IOCTL_FTV310_VPU_QUERY_METADATA) {
			if (copy_to_user(
				    (void __user *)arg, kdata,
				    sizeof(struct ftv310_vpu_metadata_params)) !=
			    0) {
				retcode = -EFAULT;
			}
		} else {
			if (copy_to_user((void __user *)arg, kdata, out_size) !=
			    0)
				retcode = -EFAULT;
		}
	} else {
		retcode = -EINVAL;
	}

	return retcode;
}

static int ftv310_vpu_device_open(struct inode *inode, struct file *filp)
{
	int ret = 0;
	int vcmd_en;
	ftv310_vpu_ioctl_id ioctl_id_par;
	
	if (iminor(inode) < 128)
		return -EACCES;

	ret = drm_open(inode, filp);

	ioctl_id_par.ID_PAR.node_idx = 0;
	vcmd_en = ftv310_vpu_get_vcmdsup(NULL, &ioctl_id_par.data, NULL);

	if (vcmd_en == 0) {
#ifdef HAS_VCD
		ftv310_vpu_dec_open(inode, filp);
#endif
#ifdef HAS_CACHECORE
		cache_open(inode, filp);
#endif
	}
#ifdef HAS_VCMD
	else
		ftv310_vpu_vcmd_open(inode, filp);
#endif
#ifdef PHY_CONFIG_PM
	ftv310_vpu_pm_runtime_get(ftv310_vpu_dev.drm_dev->dev);
#endif
	return ret;
}

static int ftv310_vpu_device_release(struct inode *inode, struct file *filp)
{
	int vcmd_en;

	ftv310_vpu_ioctl_id ioctl_id_par;

	ioctl_id_par.ID_PAR.node_idx = 0;
	vcmd_en = ftv310_vpu_get_vcmdsup(NULL, &ioctl_id_par.data, NULL);
	if (vcmd_en == 0) {
#ifdef HAS_CACHECORE
		cache_release(filp);
#endif
#ifdef HAS_VCD
		ftv310_vpu_dec_release(filp);
#endif
#ifdef HAS_VCE
		ftv310_vpu_enc_release();
#endif
#ifdef HAS_MMU
		ftv310_vpu_MMURelease(filp);
#endif
	}
#ifdef HAS_VCMD
	else {
		ftv310_vpu_vcmd_release(inode, filp);
#ifdef HAS_MMU
		ftv310_vpu_MMURelease(filp);
#endif
	}
#endif
#ifdef PHY_CONFIG_PM
	ftv310_vpu_pm_runtime_put(ftv310_vpu_dev.drm_dev->dev);
#endif
	return drm_release(inode, filp);
}

static int ftv310_vpu_mmap(struct file *filp, struct vm_area_struct *vma)
{
	int ret = 0;
	struct drm_gem_object *obj = NULL;
	struct drm_gem_ftv310_vpu_object *cma_obj;
	struct drm_vma_offset_node *node;
	unsigned long page_num = (vma->vm_end - vma->vm_start) >> PAGE_SHIFT;
	int sgtidx = 0;
	struct scatterlist *pscatter = NULL;
	struct slice_info *pslice;
	struct device *dev;

	ftv310_vpu_mmaplog("%s :%lx", __func__, vma->vm_pgoff);
	if (vma->vm_pgoff < PHY_MMAP_ADDRES_CEIL_MMAP)
		return ftv310_vpu_map_internal_address(filp, vma);
	if (mutex_lock_interruptible(&ftv310_vpu_dev.drm_dev->struct_mutex))
		return -EBUSY;
	vma->vm_pgoff -= PHY_MMAP_ADDRES_CEIL_MMAP;
	drm_vma_offset_lock_lookup(ftv310_vpu_dev.drm_dev->vma_offset_manager);
	node = drm_vma_offset_exact_lookup_locked(
		ftv310_vpu_dev.drm_dev->vma_offset_manager, vma->vm_pgoff,
		vma_pages(vma));

	if (likely(node)) {
		obj = container_of(node, struct drm_gem_object, vma_node);
		if (!kref_get_unless_zero(&obj->refcount))
			obj = NULL;
	}
	drm_vma_offset_unlock_lookup(ftv310_vpu_dev.drm_dev->vma_offset_manager);

	if (!obj) {
		mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
		return -EINVAL;
	}
	ftv310_vpu_unref_drmobj(obj);
	cma_obj = to_drm_gem_ftv310_vpu_obj(obj);

	if (page_num > cma_obj->num_pages) {
		mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
		return -EINVAL;
	}
	if (!(cma_obj->flag & FTV310_VPU_GEM_FLAG_IMPORT)) {
		pslice = getslicenode(cma_obj->sliceidx);
		if (!pslice) {
			mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
			return -EINVAL;
		}
		dev = pslice->dev;
	} else {
		dev = obj->dev->dev;
	}
	if ((cma_obj->flag & FTV310_VPU_GEM_FLAG_IMPORT) == 0) {
		if (cma_obj->vaddr == 0) {
			mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
			return -EINVAL;
		}
#if KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
		ret = drm_gem_mmap_obj(
			obj, drm_vma_node_size(node) << PAGE_SHIFT, vma);

		if (ret) {
			mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
			return ret;
		}
#else
		drm_gem_object_get(obj);
	
		vma->vm_private_data = obj;
		vma->vm_ops = obj->funcs->vm_ops;

		if (!vma->vm_ops) {
			ret = -EINVAL;
			drm_gem_object_put(obj);
			return ret;
		}

		vm_flags_set(vma, VM_IO | VM_PFNMAP | VM_DONTEXPAND | VM_DONTDUMP);
		vma->vm_page_prot = pgprot_writecombine(vm_get_page_prot(vma->vm_flags));
		vma->vm_page_prot = pgprot_decrypted(vma->vm_page_prot);
#endif
	} else {
		pscatter = &cma_obj->sgt->sgl[sgtidx];
#ifdef __amd64__
		set_memory_uc((unsigned long)cma_obj->vaddr, (int)page_num);
#endif
		vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
		/*else mmap report uncached error for some importer, e.g. i915*/
	}

	if (cma_obj->flag & FTV310_VPU_GEM_FLAG_USEVMALLOC) {
		int i, j;
		unsigned long uaddr = vma->vm_start;
		void *address = cma_obj->vaddr;
		struct page *pages = NULL;

#if KERNEL_VERSION(6, 3, 0) > LINUX_VERSION_CODE
		vma->vm_flags |= VM_LOCKED;
		vma->vm_flags &= ~VM_PFNMAP;
#else
		vm_flags_set(vma, VM_LOCKED);
		vm_flags_clear(vma, VM_PFNMAP);
#endif

		for (i = 0; i < page_num; i++) {
			pages = (vmalloc_to_page(address));
			if (IS_ERR(pages) || !page_count(pages) ||
			    vm_insert_page(vma, uaddr, pages)) {
				pr_err("alloc page %d fail = %lx:%lx\n", i,
				       (unsigned long)address,
				       (unsigned long)pages);
				address = cma_obj->vaddr;
				for (j = 0; j < i; j++) {
					pages = (vmalloc_to_page(address));
					unref_page(pages);
					address += PAGE_SIZE;
				}
				mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
				return -ENOMEM;
			}
			ref_page(pages);
			uaddr += PAGE_SIZE;
			address += PAGE_SIZE;
		}
	} else {
#ifdef PHY_FPGA_MEM
		vma->vm_pgoff = 0;
		if (dma_mmap_coherent(dev, vma, cma_obj->vaddr, cma_obj->paddr,
				      page_num << PAGE_SHIFT)) {
			mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
			return -EAGAIN;
		}
#else
		remap_pfn_range(vma, vma->vm_start,
				cma_obj->paddr >> PAGE_SHIFT,
				(vma->vm_end - vma->vm_start),  pgprot_writecombine(vm_get_page_prot(vma->vm_flags)));
#endif
	}
	vma->vm_private_data = cma_obj;
	mutex_unlock(&ftv310_vpu_dev.drm_dev->struct_mutex);
	return ret;
}

/* VFS methods */
static const struct file_operations ftv310_vpu_fops = {
	.owner = THIS_MODULE,
	.open = ftv310_vpu_device_open,
	.mmap = ftv310_vpu_mmap,
	.release = ftv310_vpu_device_release,
	.poll = drm_poll,
	.read = drm_read,
	.unlocked_ioctl = ftv310_vpu_ioctl, //drm_ioctl,
	.compat_ioctl = drm_compat_ioctl,
};

static void ftv310_vpu_gem_vm_close(struct vm_area_struct *vma)
{
	drm_gem_vm_close(vma);
}

#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
static int ftv310_vpu_unload(struct drm_device *dev)
{
	return 0;
}
#else
static void ftv310_vpu_release(struct drm_device *dev)
{
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	drm_dev_unregister(ftv310_vpu_dev.drm_dev);
#elif KERNEL_VERSION(5, 8, 0) > LINUX_VERSION_CODE
	drm_dev_fini(ftv310_vpu_dev.drm_dev);
#endif
}
#endif

#if	KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
static int ftv310_vpu_gem_prime_handle_to_fd(struct drm_device *dev,
					 struct drm_file *filp, uint32_t handle,
					 u32 flags, int *prime_fd)
{
	return drm_gem_prime_handle_to_fd(dev, filp, handle, flags, prime_fd);
}
#endif

#if KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE
/*we shall not support page fault. */
static int ftv310_vpu_vm_fault(struct vm_area_struct *vma, struct vm_fault *vmf)
{
	return -EPERM;
}
#elif KERNEL_VERSION(5, 0, 0) > LINUX_VERSION_CODE
static int ftv310_vpu_vm_fault(struct vm_fault *vmf)
{
	return -EPERM;
}
#else
static vm_fault_t ftv310_vpu_vm_fault(struct vm_fault *vmf)
{
	return -EPERM;
}
#endif

static const struct vm_operations_struct ftv310_vpu_drm_gem_cma_vm_ops = {
	.open = drm_gem_vm_open,
	.close = ftv310_vpu_gem_vm_close,
	.fault = ftv310_vpu_vm_fault,
};

#ifdef CONFIG_DRM_LEGACY
/*temp no usage now*/
static u32 ftv310_vpu_vblank_no_hw_counter(struct drm_device *dev,
				       unsigned int pipe)
{
	return 0;
}
#endif

#ifdef HAS_MMU
// Demo. Customer maybe need change it accrording to own environment
static void FreeMemWithMMU(struct drm_gem_ftv310_vpu_object *cma_obj)
{
	int i, map_count;
	int num_pages = cma_obj->num_pages;
	void *address = cma_obj->vaddr;
	struct page *pages = NULL;

	if (cma_obj->pages)
		ftv310_vpu_unpin_pages(cma_obj);

	for (i = 0; i < num_pages; i++) {
		pages = (vmalloc_to_page(address));
		map_count = page_mapcount(pages);
		if (map_count)
			unref_page(pages);
		address += PAGE_SIZE;
	}

	vfree(cma_obj->vaddr);
}

// Demo. Customer maybe need change it accrording to own environment
static int AllocMemWithMMU(struct drm_gem_ftv310_vpu_object *cma_obj,  struct drm_mode_create_dumb *args)
{
	cma_obj->vaddr = vmalloc(args->size);
	if (!cma_obj->vaddr)
		return -ENOMEM;
	cma_obj->paddr = page_to_phys(vmalloc_to_page(cma_obj->vaddr));
	cma_obj->flag |= FTV310_VPU_GEM_FLAG_USEVMALLOC;
	return 0;
}
#else
#ifdef USE_CMA
static void FreeCMAMem(struct drm_gem_ftv310_vpu_object *cma_obj)
{
	struct slice_info *pslice = getslicenode(cma_obj->sliceidx);

	if (!pslice)
		return;
	dma_release_from_contiguous(pslice->dev, cma_obj->pageaddr,
					cma_obj->num_pages);
}

/*For X86 environment, CMA maybe need disabled for these reasons:
 *1. to bring up CMA, CONFIG_DMA_CMA should be enabled in kernel building
 *2. dma_alloc/release_from_contiguous should be exported in kernel code
 *3. a boot parameter like "cma=268435456@134217728" should be added
 *4. CMA's memory management is not stable.
 */

static int AllocateCMAMem(struct drm_gem_ftv310_vpu_object *cma_obj, struct drm_mode_create_dumb *args)
{
	struct slice_info *pslice = getslicenode(args->handle);
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	cma_obj->pageaddr = dma_alloc_from_contiguous(
		pslice->dev, args->size >> PAGE_SHIFT, 1);
#else
	cma_obj->pageaddr = dma_alloc_from_contiguous(
		pslice->dev, args->size >> PAGE_SHIFT, 1, GFP_KERNEL);
#endif
	if (!cma_obj->pageaddr)
		return -ENOMEM;
	cma_obj->vaddr = page_to_virt(cma_obj->pageaddr);
	cma_obj->paddr = virt_to_phys(cma_obj->vaddr);
	return 0;
}
#else
static void FreeDMAMem(struct drm_gem_ftv310_vpu_object *cma_obj)
{
	struct slice_info *pslice = getslicenode(cma_obj->sliceidx);

	if (!pslice)
		return;
	dma_free_coherent(pslice->dev, cma_obj->base.size,
				cma_obj->vaddr, cma_obj->paddr);
}

static int AllocateDMAMem(struct drm_gem_ftv310_vpu_object *cma_obj, struct drm_mode_create_dumb *args)
{
	struct slice_info *pslice = getslicenode(args->handle);

	cma_obj->vaddr = dma_alloc_coherent(pslice->dev, args->size,
						&cma_obj->paddr,
						GFP_KERNEL | GFP_DMA);
	if (!cma_obj->vaddr)
		return -ENOMEM;
	return 0;
}
#endif
#endif


static void FreeFTV310FpgaMem(struct drm_gem_ftv310_vpu_object *cma_obj)
{
	struct ftv310_vpu_mem_handle fpga_phandle;

	remove_vram_node(&cma_obj->vram_node);

	fpga_phandle.vaddr = cma_obj->vaddr;
	fpga_phandle.paddr = cma_obj->paddr;
	ftv310_vpu_fpga_memfree(&fpga_phandle);
	cma_obj->flag &= ~FTV310_VPU_GEM_FLAG_RESERVED;
}

static int AllocFTV310FpgaMem(struct drm_gem_ftv310_vpu_object *cma_obj, struct drm_mode_create_dumb *args)
{
	struct ftv310_vpu_mem_handle fpga_phandle = {0};
	int ret;

	ret = ftv310_vpu_fpga_memalloc(&fpga_phandle, args->size);
	if (ret != 0)
		return ret;
		
	cma_obj->vaddr = fpga_phandle.vaddr;
	cma_obj->paddr = fpga_phandle.paddr;
	cma_obj->mem_base = fpga_phandle.mem_base;
	cma_obj->flag |= FTV310_VPU_GEM_FLAG_RESERVED;

	record_vram_node(&cma_obj->vram_node, cma_obj->vaddr, args->size);

	return 0;
}

static void FreeMem(struct drm_gem_ftv310_vpu_object *cma_obj)
{
	if (cma_obj->mmu) {
		struct mmu_addr_desc addr = {0};

		addr.handle = cma_obj->handle;
		addr.bus_address = cma_obj->paddr;
		addr.virtual_address = cma_obj->uptr;

		ftv310_vpu_MMUClear(&addr, &cma_obj->mmu->g_mmu->region[0], &cma_obj->mmu->pg_tbl);
		cma_obj->mmu = NULL;
	}
	trace_vcmd_buf_destroy(current->pid, cma_obj->num_pages * PAGE_SIZE, cma_obj->handle, cma_obj->paddr);

	if (cma_obj->flag & FTV310_VPU_GEM_FLAG_RESERVED)
	{
		FreeFTV310FpgaMem(cma_obj);
		return;
	}
//Customer changeing following implement based on own environment
#ifdef HAS_MMU
	if (cma_obj->flag & FTV310_VPU_GEM_FLAG_USEVMALLOC) {
		FreeMemWithMMU(cma_obj);
		return;
	}
#else

#ifdef USE_CMA
	FreeCMAMem(cma_obj);
#else
	FreeDMAMem(cma_obj);
#endif

#endif//HAS_MMU end
}

static int AllocateMem(struct drm_gem_ftv310_vpu_object *cma_obj, struct drm_mode_create_dumb *args)
{
	int ret;

	if (mem_start) {
		ret = AllocFTV310FpgaMem(cma_obj, args);
#ifdef HAS_MMU
		if (ret) {
			ret = AllocMemWithMMU(cma_obj, args);
		}
#endif
	} else {
//Customer changeing following implement based on own environment
#ifdef HAS_MMU
		ret = AllocMemWithMMU(cma_obj, args);
#else

#ifdef USE_CMA
		ret = AllocateCMAMem(cma_obj, args);
#else
		ret = AllocateDMAMem(cma_obj, args);
#endif

#endif
	}

	return ret;
}

#if KERNEL_VERSION(5, 11, 0) <= LINUX_VERSION_CODE
static const struct drm_gem_object_funcs ftv310_vpu_drm_gem_cma_funcs = {
	.export = ftv310_vpu_prime_export,
	.get_sg_table = ftv310_vpu_gem_prime_get_sg_table,
	.vmap = ftv310_vpu_gem_prime_vmap,
	.vunmap = ftv310_vpu_gem_prime_vunmap,
	.free = ftv310_vpu_gem_free_object,
	.vm_ops = &ftv310_vpu_drm_gem_cma_vm_ops,
#if KERNEL_VERSION(6, 6, 0) <= LINUX_VERSION_CODE
	.mmap = ftv310_vpu_gem_prime_mmap,
#endif
};
#endif

struct drm_driver ftv310_vpu_drm_driver = {
	//these two are related with controlD and renderD
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	.driver_features = DRIVER_GEM | DRIVER_RENDER,
#else
	.driver_features = DRIVER_GEM | DRIVER_RENDER | DRIVER_PRIME,
#endif
#ifdef CONFIG_DRM_LEGACY
	.get_vblank_counter = ftv310_vpu_vblank_no_hw_counter,
#endif
	.open = ftv310_vpu_drm_open,
	.postclose = ftv310_vpu_drm_close,
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	.unload = ftv310_vpu_unload,
#else
	.release = ftv310_vpu_release,
#endif
	.dumb_create = ftv310_vpu_gem_dumb_create_internal,
	.dumb_map_offset = ftv310_vpu_gem_dumb_map_offset,
#if KERNEL_VERSION(5, 11, 0) > LINUX_VERSION_CODE
	.dumb_destroy = drm_gem_dumb_destroy,
	.gem_prime_export = ftv310_vpu_prime_export,
	.gem_prime_get_sg_table = ftv310_vpu_gem_prime_get_sg_table,
	.gem_prime_vmap = ftv310_vpu_gem_prime_vmap,
	.gem_prime_vunmap = ftv310_vpu_gem_prime_vunmap,
	.gem_free_object_unlocked = ftv310_vpu_gem_free_object,
	.gem_vm_ops = &ftv310_vpu_drm_gem_cma_vm_ops,
#endif
#if KERNEL_VERSION(6, 4, 0) > LINUX_VERSION_CODE
	.dumb_destroy = ftv310_vpu_gem_dumb_destroy,
#endif
	.gem_prime_import = ftv310_vpu_drm_gem_prime_import,
#if	KERNEL_VERSION(6, 6, 0) > LINUX_VERSION_CODE
	.prime_handle_to_fd = ftv310_vpu_gem_prime_handle_to_fd,
	.prime_fd_to_handle = drm_gem_prime_fd_to_handle,
	.gem_prime_mmap = ftv310_vpu_gem_prime_mmap,
#endif
	.gem_prime_import_sg_table = ftv310_vpu_gem_prime_import_sg_table,
#if defined(CONFIG_DEBUG_FS)
	.debugfs_init = ftv310_vpu_debugfs_init,
#endif
	.fops = &ftv310_vpu_fops,
	.name = DRIVER_NAME,
	.desc = DRIVER_DESC,
	.date = DRIVER_DATE,
	.major = DRIVER_MAJOR,
	.minor = DRIVER_MINOR,
};

static struct page **ftv310_vpu_pin_pages(struct drm_gem_ftv310_vpu_object *bo)
{
	mutex_lock(&bo->pages_lock);
	if (bo->pages_pin_count++ == 0) {
		struct page **pages;

		pages = kvmalloc_array(bo->num_pages, sizeof(struct page *), GFP_KERNEL);
		if (IS_ERR(pages)) {
			bo->pages_pin_count--;
			mutex_unlock(&bo->pages_lock);
			return pages;
		}
		bo->pages = pages;
	}
	mutex_unlock(&bo->pages_lock);
	return bo->pages;
}

static void ftv310_vpu_unpin_pages(struct drm_gem_ftv310_vpu_object *bo)
{
	mutex_lock(&bo->pages_lock);
	if (--bo->pages_pin_count == 0) {
		kvfree(bo->pages);
		bo->pages = NULL;
	}
	mutex_unlock(&bo->pages_lock);
}

static int ftv310_vpu_prime_pin(struct drm_gem_object *obj)
{
	struct drm_gem_ftv310_vpu_object *bo = to_drm_gem_ftv310_vpu_obj(obj);
	long n_pages = obj->size >> PAGE_SHIFT;
	struct page **pages;
	int i;
	void *address;

	pages = ftv310_vpu_pin_pages(bo);
	if (IS_ERR(pages))
		return PTR_ERR(pages);

	address = bo->vaddr;
	for (i = 0; i < n_pages; i++) {
		pages[i] = (vmalloc_to_page(address));
		address += PAGE_SIZE;
	}
	drm_clflush_pages(pages, n_pages);
	return 0;
}
