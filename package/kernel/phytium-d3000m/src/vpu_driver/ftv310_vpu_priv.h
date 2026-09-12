/* SPDX-License-Identifier: GPL-2.0 */
/*
 *    ftv310 driver private header file.
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

#ifndef FTV310_VPU_PRIV_H
#define FTV310_VPU_PRIV_H
#include "ftv310_vpu.h"
#include "ftv310_vpu_device.h"

/* compile options */

#define FTV310_VPU_GEM_FLAG_IMPORT BIT(0)
#define FTV310_VPU_GEM_FLAG_EXPORT BIT(1)
#define FTV310_VPU_GEM_FLAG_EXPORTUSED BIT(2)
#define FTV310_VPU_GEM_FLAG_USEVMALLOC BIT(3)
#define FTV310_VPU_GEM_FLAG_USECMA     BIT(4)
#define FTV310_VPU_GEM_FLAG_RESERVED     BIT(5)

#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
#define ftv310_vpu_access_ok(a, b, c) access_ok(b, c)
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
#define ftv310_vpu_reserve_obj_shared(a, b) dma_resv_reserve_shared(a, b)
#else
#define ftv310_vpu_reserve_obj_shared(a, b) dma_resv_reserve_fences(a, b)
#endif
#else /*KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE*/
#define ftv310_vpu_access_ok(a, b, c) access_ok(a, b, c)
#define ftv310_vpu_reserve_obj_shared(a, b) reservation_object_reserve_shared(a)
#endif

#if KERNEL_VERSION(5, 10, 0) <= LINUX_VERSION_CODE
#define ftv310_vpu_ref_drmobj drm_gem_object_get
#define ftv310_vpu_unref_drmobj drm_gem_object_put
#else
#define ftv310_vpu_ref_drmobj drm_gem_object_get
#define ftv310_vpu_unref_drmobj drm_gem_object_put_unlocked
#endif

#define NODENAME_DECODER "decoder"
#define NODENAME_ENCODER "encoder"
#define NODENAME_CACHE "cache"
#define NODENAME_DEC400 "dec400"
#define NODENAME_AXIFE "axife"
#define NODENAME_MMU "ftv310_vpu_mmu"
#define NODENAME_VCMD "ftv310_vpu_vcmd"
#define NODENAME_IM "ftv310_vpu_IM"
#define NODENAME_DECJPG "decjpg"
#define NODENAME_ENCJPG "encjpg"

#define SET_DEC_CFG_BIT(a) (a)
#define SET_ENC_CFG_BIT(a) ((a << 8))
#define GET_DEC_CFG_BITS(a) (a & 0xff)
#define GET_ENC_CFG_BITS(a) ((a >> 8) & 0xff)

#ifdef PHY_CONFIG_PM
struct ftv310_vpu_dev_data {
	void *data;
	struct ftv310_vpu_dev_data *next;
};
#endif

typedef struct dtbnode {
	struct device_node *ofnode;
	int type;
	phys_addr_t ioaddr;
	phys_addr_t iosize;
	int irq[4];
	int parenttype;
	phys_addr_t parentaddr;
	int sliceidx;
	struct dtbnode *next;
} dtbnode;

struct ftv310_vpu_device_handle {
	struct platform_device *platformdev; /* parent device */
	struct drm_device *drm_dev;
	u32 config; /* Encoder subs IP info store in bit [9:16] */
#ifdef PHY_CONFIG_PM
	void *ftv310_vpu_data;
#endif
#ifdef PCIE_EN
	void *dev;
#endif
};

struct ftv310_vpu_mem_handle {
	unsigned int sliceidx;
	unsigned int size;
	dma_addr_t paddr;
	dma_addr_t mem_base;
	void *vaddr;
	struct list_head mem_node;
};

struct ftv310_vpu_base_addr {
	unsigned long reg_base;
	unsigned long ddr_base;
};

extern struct ftv310_vpu_device_handle ftv310_vpu_dev;
extern const struct dma_buf_ops ftv310_vpu_dmabuf_ops;
extern struct drm_driver ftv310_vpu_drm_driver;

extern int dbg_mmap;
#define ftv310_vpu_mmaplog(fmt, ...) {\
	if (dbg_mmap)	\
		pr_info(fmt, ##__VA_ARGS__);	\
}

#if KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE

#define FTV310_VPU_FENCE_FLAG_ENABLE_SIGNAL_BIT FENCE_FLAG_ENABLE_SIGNAL_BIT
#define FTV310_VPU_FENCE_FLAG_SIGNAL_BIT FENCE_FLAG_SIGNALED_BIT

typedef struct fence ftv310_vpu_fence_t;
typedef struct fence_ops ftv310_vpu_fence_op_t;

static inline signed long
ftv310_vpu_fence_default_wait(ftv310_vpu_fence_t *fence, bool intr, signed long timeout)
{
	return fence_default_wait(fence, intr, timeout);
}

static inline void ftv310_vpu_fence_init(ftv310_vpu_fence_t *fence,
				     const ftv310_vpu_fence_op_t *ops,
				     spinlock_t *lock, unsigned int context,
				     unsigned int seqno)
{
	return fence_init(fence, ops, lock, context, seqno);
}

static inline unsigned int ftv310_vpu_fence_context_alloc(unsigned int num)
{
	return fence_context_alloc(num);
}

static inline signed long
ftv310_vpu_fence_wait_timeout(ftv310_vpu_fence_t *fence, bool intr, signed long timeout)
{
	return fence_wait_timeout(fence, intr, timeout);
}

static inline struct drm_gem_object *
ftv310_vpu_gem_object_lookup(struct drm_device *dev, struct drm_file *filp,
			 u32 handle)
{
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	return drm_gem_object_lookup(filp, handle);
#else
	return drm_gem_object_lookup(dev, filp, handle);
#endif
}

static inline void ftv310_vpu_fence_put(ftv310_vpu_fence_t *fence)
{
	return fence_put(fence);
}

static inline int ftv310_vpu_fence_signal(ftv310_vpu_fence_t *fence)
{
	return fence_signal(fence);
}

static inline void ref_page(struct page *pp)
{
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	atomic_inc(&pp->_refcount);
#else
	atomic_inc(&pp->_count);
#endif
}

static inline void unref_page(struct page *pp)
{
#if KERNEL_VERSION(4, 10, 0) > LINUX_VERSION_CODE
	atomic_dec(&pp->_refcount);
#else
	atomic_dec(&pp->_count);
#endif
}

static inline bool ftv310_vpu_fence_is_signaled(ftv310_vpu_fence_t *fence)
{
	return fence_is_signaled(fence);
}

#else /*version higher */
#define FTV310_VPU_FENCE_FLAG_ENABLE_SIGNAL_BIT DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT
#define FTV310_VPU_FENCE_FLAG_SIGNAL_BIT DMA_FENCE_FLAG_SIGNALED_BIT

typedef struct dma_fence ftv310_vpu_fence_t;
typedef struct dma_fence_ops ftv310_vpu_fence_op_t;

static inline signed long
ftv310_vpu_fence_default_wait(ftv310_vpu_fence_t *fence, bool intr, signed long timeout)
{
	return dma_fence_default_wait(fence, intr, timeout);
}

static inline void ftv310_vpu_fence_init(ftv310_vpu_fence_t *fence,
				     const ftv310_vpu_fence_op_t *ops,
				     spinlock_t *lock, unsigned int context,
				     unsigned int seqno)
{
	return dma_fence_init(fence, ops, lock, context, seqno);
}

static inline unsigned int ftv310_vpu_fence_context_alloc(unsigned int num)
{
	return dma_fence_context_alloc(num);
}

static inline signed long
ftv310_vpu_fence_wait_timeout(ftv310_vpu_fence_t *fence, bool intr, signed long timeout)
{
	return dma_fence_wait_timeout(fence, intr, timeout);
}

static inline struct drm_gem_object *
ftv310_vpu_gem_object_lookup(struct drm_device *dev, struct drm_file *filp,
			 u32 handle)
{
	return drm_gem_object_lookup(filp, handle);
}

static inline void ftv310_vpu_fence_put(ftv310_vpu_fence_t *fence)
{
	return dma_fence_put(fence);
}

static inline int ftv310_vpu_fence_signal(ftv310_vpu_fence_t *fence)
{
	return dma_fence_signal(fence);
}

static inline void ref_page(struct page *pp)
{
	atomic_inc(&pp->_refcount);
	atomic_inc(&pp->_mapcount);
}

static inline void unref_page(struct page *pp)
{
	atomic_dec(&pp->_refcount);
	atomic_dec(&pp->_mapcount);
}

static inline bool ftv310_vpu_fence_is_signaled(ftv310_vpu_fence_t *fence)
{
	return dma_fence_is_signaled(fence);
}

#endif
typedef struct {
	ftv310_vpu_fence_t base;
	struct file *filp;
} phytium_vpu_fence_t;

static inline struct drm_gem_ftv310_vpu_object *
to_drm_gem_ftv310_vpu_obj(struct drm_gem_object *gem_obj)
{
	return container_of(gem_obj, struct drm_gem_ftv310_vpu_object, base);
}

struct drm_gem_object *
ftv310_vpu_get_gem_from_dmabuf(struct dma_buf *dma_buf);

int ftv310_vpu_setdomain(struct drm_device *dev, void *data,
		     struct drm_file *file_priv);
int ftv310_vpu_acquirebuf(struct drm_device *dev, void *data,
		      struct drm_file *file_priv);
int ftv310_vpu_testbufvalid(struct drm_device *dev, void *data,
			struct drm_file *file_priv);
int ftv310_vpu_releasebuf(struct drm_device *dev, void *data,
		      struct drm_file *file_priv);
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
int init_ftv310_vpu_resv(struct dma_resv *presv,
		     struct drm_gem_ftv310_vpu_object *cma_obj);
#else
int init_ftv310_vpu_resv(struct reservation_object *presv,
                     struct drm_gem_ftv310_vpu_object *cma_obj);
#endif
void initFenceData(void);
void releaseFenceData(void);

int ftv310_vpu_memalloc(struct ftv310_vpu_mem_handle *phandle);
int ftv310_vpu_memfree(struct ftv310_vpu_mem_handle *phandle);

int ftv310_vpu_createsysfsAPI(int sliceidx, struct device *dev);
int ftv310_vpu_unlinksysfsAPI(void);
int ftv310_vpu_get_vcmdsup(struct drm_device *dev, void *data,
		       struct drm_file *file_priv);
int ftv310_vpu_get_irqinfo(struct drm_device *dev, void *data,
		       struct drm_file *file_priv);
int ftv310_vpu_get_pmsup(struct drm_device *dev, void *data,
		       struct drm_file *file_priv);

int ftv310_vpu_map_internal_address(struct file *filp, struct vm_area_struct *vma);

void ftv310_vpu_pm_runtime_get(struct device *kdev);
void ftv310_vpu_pm_runtime_put(struct device *kdev);
void ftv310_vpu_clear_fence(struct file *filp);

/* debug */
#define ENABLE_DEBUG
#ifdef ENABLE_DEBUG
#define DBG(...) pr_info(__VA_ARGS__)
#else
#define DBG(...)
#endif

#endif /*FTV310_VPU_PRIV_H*/
