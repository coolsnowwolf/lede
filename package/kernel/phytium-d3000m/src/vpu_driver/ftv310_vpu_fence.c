// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver DMA_BUF fence operation.
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
#include "ftv310_vpu_priv.h"

/*fence related code*/
/*for fence's seqno, maybe each domain shoul hold one*/
static unsigned long seqno;
DEFINE_IDR(fence_idr);
/*a mutex for fence*/
struct mutex fence_mutex;

static const char *ftv310_vpu_fence_get_driver_name(ftv310_vpu_fence_t *fence)
{
	return "ftv310_vpu";
}

static const char *ftv310_vpu_fence_get_timeline_name(ftv310_vpu_fence_t *fence)
{
	return " "; /*it should correspond to six domains later*/
}

static bool ftv310_vpu_fence_enable_signaling(ftv310_vpu_fence_t *fence)
{
	if (test_bit(FTV310_VPU_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->flags))
		return true;
	else
		return false;
}

static bool ftv310_vpu_fence_signaled(ftv310_vpu_fence_t *fobj)
{
	unsigned long irqflags;
	bool ret;

	spin_lock_irqsave(fobj->lock, irqflags);
	ret = (test_bit(FTV310_VPU_FENCE_FLAG_SIGNAL_BIT, &fobj->flags) != 0);
	spin_unlock_irqrestore(fobj->lock, irqflags);
	return ret;
}

static void ftv310_vpu_fence_free(ftv310_vpu_fence_t *fence)
{
	kfree(fence->lock);
	fence->lock = NULL;
#if KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE
	fence_free(fence);
#else
	dma_fence_free(fence);
#endif
}

const static ftv310_vpu_fence_op_t ftv310_vpu_fenceops = {
	.get_driver_name = ftv310_vpu_fence_get_driver_name,
	.get_timeline_name = ftv310_vpu_fence_get_timeline_name,
	.enable_signaling = ftv310_vpu_fence_enable_signaling,
	.signaled = ftv310_vpu_fence_signaled,
	.wait = ftv310_vpu_fence_default_wait,
	.release = ftv310_vpu_fence_free,
};

static ftv310_vpu_fence_t *alloc_fence(unsigned int ctxno, struct file *filp)
{
	phytium_vpu_fence_t *fobj;
	/*spinlock*/
	spinlock_t *lock;

	fobj = kzalloc(sizeof(*fobj), GFP_KERNEL);
	if (!fobj)
		return NULL;
	lock = kzalloc(sizeof(*lock), GFP_KERNEL);
	if (!lock) {
		kfree(fobj);
		return NULL;
	}
	fobj->filp = filp;

	spin_lock_init(lock);
	ftv310_vpu_fence_init((ftv310_vpu_fence_t *)fobj, &ftv310_vpu_fenceops, lock, ctxno, seqno++);
	clear_bit(FTV310_VPU_FENCE_FLAG_SIGNAL_BIT, &fobj->base.flags);
	set_bit(FTV310_VPU_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fobj->base.flags);
	return &fobj->base;
}

static int isFTV310fence(ftv310_vpu_fence_t *fence)
{
	return (fence->ops == &ftv310_vpu_fenceops);
}

#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
int init_ftv310_vpu_resv(struct dma_resv *presv,
		     struct drm_gem_ftv310_vpu_object *cma_obj)
#else
int init_ftv310_vpu_resv(struct reservation_object *presv,
                     struct drm_gem_ftv310_vpu_object *cma_obj)
#endif
{
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	dma_resv_init(presv);
#else
	reservation_object_init(presv);
#endif
	cma_obj->ctxno = ftv310_vpu_fence_context_alloc(1);

	/*real fence insertion will be done when buffer is really attached */
	//reservation_object_add_excl_fence(presv, (ftv310_vpu_fence_t *)fobj);
	return 0;
}

int ftv310_vpu_waitfence(ftv310_vpu_fence_t *pfence)
{
	if (test_bit(FTV310_VPU_FENCE_FLAG_SIGNAL_BIT, &pfence->flags))
		return 0;

	if (isFTV310fence(pfence))
		/*need self check*/
		return 0;
	else
		return ftv310_vpu_fence_wait_timeout(pfence, true, 30 * HZ);
}

/*it's obselete, left here for compiling compatible*/
int ftv310_vpu_setdomain(struct drm_device *dev, void *data,
		     struct drm_file *file_priv)
{
	return 0;
}

void initFenceData(void)
{
	seqno = 0;
	mutex_init(&fence_mutex);
	idr_init(&fence_idr);
}

void releaseFenceData(void)
{
	int id = 0;
	phytium_vpu_fence_t *fence = NULL;

	mutex_lock(&fence_mutex);

	fence = idr_get_next(&fence_idr, &id);
	while (fence) {
		ftv310_vpu_fence_put(&fence->base);
		idr_remove(&fence_idr, id);
		id++;
		fence = idr_get_next(&fence_idr, &id);
	}

	mutex_unlock(&fence_mutex);
}

int ftv310_vpu_acquirebuf(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct ftv310_vpu_acquirebuf *arg = data;
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	struct dma_resv *resv;
#else
	struct reservation_object *resv;
#endif
	struct drm_gem_object *obj;
	ftv310_vpu_fence_t *fence = NULL;
	unsigned long timeout = arg->timeout;
	unsigned long fenceid = -1;
	int ret = 0;

	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, arg->handle);
	if (!obj)
		return -ENOENT;

	if (!obj->dma_buf) {
		if (ftv310_vpu_dev.drm_dev == obj->dev) {
			struct drm_gem_ftv310_vpu_object *hobj =
				to_drm_gem_ftv310_vpu_obj(obj);

			resv = &hobj->kresv;
		} else {
			ret = -ENOENT;
			goto err;
		}
	} else
		resv = obj->dma_buf->resv;

		/* Check for a stalled fence */
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
	ret = reservation_object_wait_timeout_rcu(resv, arg->flags & FTV310_VPU_FENCE_WRITE, 1,
				timeout);
#elif (KERNEL_VERSION(5, 13, 0) >= LINUX_VERSION_CODE) && (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
	ret = dma_resv_wait_timeout_rcu(resv, arg->flags & FTV310_VPU_FENCE_WRITE, 1,
				timeout);
#elif (KERNEL_VERSION(6, 1, 0) > LINUX_VERSION_CODE) && (KERNEL_VERSION(5, 13, 0) < LINUX_VERSION_CODE)
	ret = dma_resv_wait_timeout(resv, arg->flags & FTV310_VPU_FENCE_WRITE, 1,
				timeout);
#else
	ret = dma_resv_wait_timeout(resv, dma_resv_usage_rw(arg->flags & FTV310_VPU_FENCE_WRITE), 1,
				       timeout);
#endif
	if (ret == -ERESTARTSYS) {
		goto err;
	} else if (!ret) {
		ret = -EBUSY;
		goto err;
	}

	/* Expose the fence via the dma-buf */
	ret = -ENOMEM;
	fence = alloc_fence(ftv310_vpu_fence_context_alloc(1), file_priv->filp);
	if (!fence)
		goto err;

	mutex_lock(&fence_mutex);
	ret = idr_alloc(&fence_idr, fence, 1, 0, GFP_KERNEL);
	mutex_unlock(&fence_mutex);
	if (ret >= 0)
		fenceid = ret;
	else
		goto err;
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	dma_resv_lock(resv, NULL);
#else
	ww_mutex_lock(&resv->lock, NULL);
#endif
	ret = 0;
	if (arg->flags & FTV310_VPU_FENCE_WRITE) {
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
		reservation_object_add_excl_fence(resv, fence);
#elif (KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE) && (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
		dma_resv_add_excl_fence(resv, fence);
#else
		ret = ftv310_vpu_reserve_obj_shared(resv, 1);
		if (!ret)
			dma_resv_add_fence(resv, fence, DMA_RESV_USAGE_WRITE);
#endif
	} else {
		/*I'm not sure if 1 fence is enough, pass compilation first*/
		ret = ftv310_vpu_reserve_obj_shared(resv, 1);
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
		if (ret == 0)
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
			reservation_object_add_shared_fence(resv, fence);
#elif (KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE) && (KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE)
			dma_resv_add_shared_fence(resv, fence);
#endif
#endif
	}
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	dma_resv_unlock(resv);
#else
	ww_mutex_unlock(&resv->lock);
#endif

	/* Record the fence in our idr for later signaling */
	if (ret == 0) {
		arg->fence_handle = fenceid;
		goto out;
	}
err:
	if (fenceid >= 0) {
		mutex_lock(&fence_mutex);
		idr_remove(&fence_idr, fenceid);
		mutex_unlock(&fence_mutex);
	}
	if (fence) {
		ftv310_vpu_fence_signal(fence);
		ftv310_vpu_fence_put(fence);
	}
out:
	ftv310_vpu_unref_drmobj(obj);
	return ret;
}

int ftv310_vpu_testbufvalid(struct drm_device *dev, void *data,
			struct drm_file *file_priv)
{
	struct ftv310_vpu_fencecheck *arg = data;
#if KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	struct dma_resv *resv;
#else
	struct reservation_object *resv;
#endif
	struct drm_gem_object *obj;

	arg->ready = 0;
	obj = ftv310_vpu_gem_object_lookup(dev, file_priv, arg->handle);
	if (!obj)
		return -ENOENT;

	if (!obj->dma_buf) {
		if (ftv310_vpu_dev.drm_dev == obj->dev) {
			struct drm_gem_ftv310_vpu_object *hobj =
				to_drm_gem_ftv310_vpu_obj(obj);

			resv = &hobj->kresv;
		} else {
			ftv310_vpu_unref_drmobj(obj);
			return -ENOENT;
		}
	} else
		resv = obj->dma_buf->resv;

	/* Check for a stalled fence */
#if KERNEL_VERSION(5, 4, 0) > LINUX_VERSION_CODE
	if (reservation_object_wait_timeout_rcu(resv, 1, 1, 0) <= 0)
#elif (KERNEL_VERSION(5, 13, 0) >= LINUX_VERSION_CODE) && KERNEL_VERSION(5, 4, 0) <= LINUX_VERSION_CODE
	if (dma_resv_wait_timeout_rcu(resv, 1, 1, 0) <= 0)
#elif (KERNEL_VERSION(6, 1, 0) > LINUX_VERSION_CODE) && \
	(KERNEL_VERSION(5, 13, 0) < LINUX_VERSION_CODE)
	if (dma_resv_wait_timeout(resv, 1, 1, 0) <= 0)
#else
	if (dma_resv_wait_timeout(resv, dma_resv_usage_rw(1), 1, 0) <= 0)
#endif	
		arg->ready = 0;
	else
		arg->ready = 1;
	ftv310_vpu_unref_drmobj(obj);
	return 0;
}

int ftv310_vpu_releasebuf(struct drm_device *dev, void *data,
		      struct drm_file *file_priv)
{
	struct ftv310_vpu_releasebuf *arg = data;
	ftv310_vpu_fence_t *fence;
	int ret = 0;

	mutex_lock(&fence_mutex);
	fence = idr_replace(&fence_idr, NULL, arg->fence_handle);
	mutex_unlock(&fence_mutex);

	if (!fence || IS_ERR(fence))
		return -ENOENT;

	if (ftv310_vpu_fence_is_signaled(fence))
		ret = -ETIMEDOUT;

	ftv310_vpu_fence_signal(fence);
	ftv310_vpu_fence_put(fence);
	mutex_lock(&fence_mutex);
	idr_remove(&fence_idr, arg->fence_handle);
	mutex_unlock(&fence_mutex);
	return ret;
}

void ftv310_vpu_clear_fence(struct file *filp)
{
	int id = 0;
	phytium_vpu_fence_t *fence = NULL;

	mutex_lock(&fence_mutex);

	fence = idr_get_next(&fence_idr, &id);
	while (fence) {
		if (fence && (fence->filp == filp)) {
			ftv310_vpu_fence_put(&fence->base);
			idr_remove(&fence_idr, id);
		}
		id++;
		fence = idr_get_next(&fence_idr, &id);
	}
	mutex_unlock(&fence_mutex);

	return;
}
