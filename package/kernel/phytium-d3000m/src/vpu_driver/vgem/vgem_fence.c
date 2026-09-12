/*
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software")
 * to deal in the software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * them Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTIBILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES, OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT, OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <linux/dma-buf.h>
#include <linux/dma-resv.h>
#include <linux/version.h>
#include "vgem_drv.h"

#define VGEM_FENCE_TIMEOUT (10*HZ)
#define ftv310_vpu_unref_drmobj drm_gem_object_put
#if KERNEL_VERSION(4, 13, 0) > LINUX_VERSION_CODE
#define FTV310_VPU_FENCE_FLAG_ENABLE_SIGNAL_BIT FENCE_FLAG_ENABLE_SIGNAL_BIT
#define FTV310_VPU_FENCE_FLAG_SIGNAL_BIT FENCE_FLAG_SIGNALED_BIT
#else /*version higher */
#define VGEM_FENCE_FLAG_ENABLE_SIGNAL_BIT DMA_FENCE_FLAG_ENABLE_SIGNAL_BIT
#define VGEM_FENCE_FLAG_SIGNAL_BIT DMA_FENCE_FLAG_SIGNALED_BIT
#endif

#define USE_FTV310_VPU_FENCE

extern struct vgem_device *vgem_device;

struct vgem_fence {
	struct dma_fence base;
	struct spinlock lock;
	struct timer_list timer;
};
static struct dma_fence *alloc_fence(unsigned int ctxno);
static const char *vgem_fence_get_driver_name(struct dma_fence *fence)
{
	return "vgem";
}

static const char *vgem_fence_get_timeline_name(struct dma_fence *fence)
{
	return "unbound";
}

static void vgem_fence_release(struct dma_fence *base)
{
	struct vgem_fence *fence = container_of(base, typeof(*fence), base);

	del_timer_sync(&fence->timer);
	dma_fence_free(&fence->base);
}

static void vgem_fence_value_str(struct dma_fence *fence, char *str, int size)
{
	snprintf(str, size, "%llu", fence->seqno);
}

static void vgem_fence_timeline_value_str(struct dma_fence *fence, char *str,
					  int size)
{
	snprintf(str, size, "%llu",
		 dma_fence_is_signaled(fence) ? fence->seqno : 0);
}

static const struct dma_fence_ops vgem_fence_ops = {
	.get_driver_name = vgem_fence_get_driver_name,
	.get_timeline_name = vgem_fence_get_timeline_name,
	.release = vgem_fence_release,

	.fence_value_str = vgem_fence_value_str,
	.timeline_value_str = vgem_fence_timeline_value_str,
};

static void vgem_fence_timeout(struct timer_list *t)
{
	struct vgem_fence *fence = from_timer(fence, t, timer);
	printk("Warning: vgem_fence_timeout fence 0x%p\n", fence);
//	dma_fence_signal(&fence->base);
}

#ifdef USE_FTV310_VPU_FENCE
/*
 * vgem_fence_attach_ioctl (DRM_IOCTL_VGEM_FENCE_ATTACH):
 *
 * Create and attach a fence to the vGEM handle. This fence is then exposed
 * via the dma-buf reservation object and visible to consumers of the exported
 * dma-buf. If the flags contain VGEM_FENCE_WRITE, the fence indicates the
 * vGEM buffer is being written to by the client and is exposed as an exclusive
 * fence, otherwise the fence indicates the client is current reading from the
 * buffer and all future writes should wait for the client to signal its
 * completion. Note that if a conflicting fence is already on the dma-buf (i.e.
 * an exclusive fence when adding a read, or any fence when adding a write),
 * -EBUSY is reported. Serialisation between operations should be handled
 * by waiting upon the dma-buf.
 *
 * This returns the handle for the new fence that must be signaled within 10
 * seconds (or otherwise it will automatically expire). See
 * vgem_fence_signal_ioctl (DRM_IOCTL_VGEM_FENCE_SIGNAL).
 *
 * If the vGEM handle does not exist, vgem_fence_attach_ioctl returns -ENOENT.
 */
int vgem_fence_attach_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file)
{
	struct drm_vgem_fence_attach *arg = data;
	struct vgem_file *vfile = file->driver_priv;
	struct dma_resv *resv;
	struct drm_gem_object *obj;
	struct dma_fence *fence = NULL;
	unsigned long timeout = 50000000;//arg->pad;
	unsigned long fenceid = -1;
	int ret = 0;
	obj = drm_gem_object_lookup(file, arg->handle);
	if (!obj)
		return -ENOENT;

	if (!obj->dma_buf) {
		if (&vgem_device->drm == obj->dev) {
			struct drm_vgem_gem_object *hobj =
				to_vgem_bo(obj);

			resv = &hobj->kresv;
		} else {
			ret = -ENOENT;
			goto err;
		}
	} else
		resv = obj->dma_buf->resv;

	/* Check for a stalled fence */
#if KERNEL_VERSION(5, 13, 0) >= LINUX_VERSION_CODE
	if (!dma_resv_wait_timeout_rcu(resv, arg->flags & VGEM_FENCE_WRITE, 1,
				       timeout)) {
#else
	if (!dma_resv_wait_timeout(resv, arg->flags & VGEM_FENCE_WRITE, 1,
				       timeout)) {
#endif
		ret = -EBUSY;
		goto err;
	}

	/* Expose the fence via the dma-buf */
	ret = -ENOMEM;
	fence = alloc_fence(dma_fence_context_alloc(1));
	printk("vgem_fence_attach_ioctl fence %p \r\n", fence);
	if (!fence)
		goto err;

	mutex_lock(&vfile->fence_mutex);
	ret = idr_alloc(&vfile->fence_idr, fence, 1, 0, GFP_KERNEL);
	mutex_unlock(&vfile->fence_mutex);
	if (ret >= 0)
		fenceid = ret;
	else
		goto err;

	dma_resv_lock(resv, NULL);
	ret = 0;
	if (arg->flags & VGEM_FENCE_WRITE)
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
		dma_resv_add_excl_fence(resv, fence);
#else
		dma_resv_add_fence(resv, fence, DMA_RESV_USAGE_WRITE);
#endif
	else {
		/*I'm not sure if 1 fence is enough, pass compilation first*/
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
		ret = dma_resv_reserve_shared(resv, 1);
#else
		ret = dma_resv_reserve_fences(resv, 1);
#endif
#else
		ret = reservation_object_reserve_shared(resv);
#endif
#if KERNEL_VERSION(5, 19, 0) > LINUX_VERSION_CODE
		if (ret == 0)
			dma_resv_add_shared_fence(resv, fence);
#endif
	}
	dma_resv_unlock(resv);

	/* Record the fence in our idr for later signaling */
	if (ret == 0) {
		arg->out_fence = fenceid;
		goto out;
	}
err:
	if (fenceid >= 0) {
		mutex_lock(&vfile->fence_mutex);
		idr_remove(&vfile->fence_idr, fenceid);
		mutex_unlock(&vfile->fence_mutex);
	}
	if (fence) {
		dma_fence_signal(fence);
		dma_fence_put(fence);
	}
out:
#if KERNEL_VERSION(5, 0, 0) <= LINUX_VERSION_CODE
	drm_gem_object_put(obj);
#else
	drm_gem_object_unreference_unlocked(obj);
#endif
	return ret;
}

/*
 * vgem_fence_signal_ioctl (DRM_IOCTL_VGEM_FENCE_SIGNAL):
 *
 * Signal and consume a fence ealier attached to a vGEM handle using
 * vgem_fence_attach_ioctl (DRM_IOCTL_VGEM_FENCE_ATTACH).
 *
 * All fences must be signaled within 10s of attachment or otherwise they
 * will automatically expire (and a vgem_fence_signal_ioctl returns -ETIMEDOUT).
 *
 * Signaling a fence indicates to all consumers of the dma-buf that the
 * client has completed the operation associated with the fence, and that the
 * buffer is then ready for consumption.
 *
 * If the fence does not exist (or has already been signaled by the client),
 * vgem_fence_signal_ioctl returns -ENOENT.
 */
int vgem_fence_signal_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file)
{
	struct vgem_file *vfile = file->driver_priv;
	struct drm_vgem_fence_signal *arg = data;
	struct dma_fence *fence;
	int ret = 0;
	mutex_lock(&vfile->fence_mutex);
	fence = idr_replace(&vfile->fence_idr, NULL, arg->fence);
	mutex_unlock(&vfile->fence_mutex);
	if (!fence || IS_ERR(fence))
		return -ENOENT;


	if (dma_fence_is_signaled(fence))
		ret = -ETIMEDOUT;

	dma_fence_signal(fence);
	dma_fence_put(fence);
	mutex_lock(&vfile->fence_mutex);
	idr_remove(&vfile->fence_idr, arg->fence);
	mutex_unlock(&vfile->fence_mutex);
	return ret;
}

int ftv310_vpu_testbufvalid(struct drm_device *dev, void *data,
			struct drm_file *file)
{
	struct ftv310_vpu_fencecheck *arg = data;
	struct dma_resv *resv;
	struct drm_gem_object *obj;
	int ready = 0;
	arg->ready = 0;

	obj = drm_gem_object_lookup(file, arg->handle);
	if (!obj)
		return -ENOENT;

	if (!obj->dma_buf) {
		if (&vgem_device->drm == obj->dev) {
			struct drm_vgem_gem_object *hobj =
				to_vgem_bo(obj);

			resv = &hobj->kresv;
		} else {
			ftv310_vpu_unref_drmobj(obj);
			return -ENOENT;
		}
	} else
		resv = obj->dma_buf->resv;
	/* Check for a conflicting fence */
#if KERNEL_VERSION(5, 13, 0) >= LINUX_VERSION_CODE
	if (!dma_resv_test_signaled_rcu(resv, 1))
#else
	if(!dma_resv_test_signaled(resv, 1))
#endif
		arg->ready = 0;
	else
		arg->ready = 1;

	ready = arg->ready;

	ftv310_vpu_unref_drmobj(obj);
	return ready;
}
#else

static struct dma_fence *vgem_fence_create(struct vgem_file *vfile,
					   unsigned int flags)
{
	struct vgem_fence *fence;

	fence = kzalloc(sizeof(*fence), GFP_KERNEL);
	if (!fence)
		return NULL;

	spin_lock_init(&fence->lock);
	dma_fence_init(&fence->base, &vgem_fence_ops, &fence->lock,
		       dma_fence_context_alloc(1), 1);

	timer_setup(&fence->timer, vgem_fence_timeout, 0);

	/* We force the fence to expire within 10s to prevent driver hangs */
	mod_timer(&fence->timer, jiffies + VGEM_FENCE_TIMEOUT);

	return &fence->base;
}

/*
 * vgem_fence_attach_ioctl (DRM_IOCTL_VGEM_FENCE_ATTACH):
 *
 * Create and attach a fence to the vGEM handle. This fence is then exposed
 * via the dma-buf reservation object and visible to consumers of the exported
 * dma-buf. If the flags contain VGEM_FENCE_WRITE, the fence indicates the
 * vGEM buffer is being written to by the client and is exposed as an exclusive
 * fence, otherwise the fence indicates the client is current reading from the
 * buffer and all future writes should wait for the client to signal its
 * completion. Note that if a conflicting fence is already on the dma-buf (i.e.
 * an exclusive fence when adding a read, or any fence when adding a write),
 * -EBUSY is reported. Serialisation between operations should be handled
 * by waiting upon the dma-buf.
 *
 * This returns the handle for the new fence that must be signaled within 10
 * seconds (or otherwise it will automatically expire). See
 * vgem_fence_signal_ioctl (DRM_IOCTL_VGEM_FENCE_SIGNAL).
 *
 * If the vGEM handle does not exist, vgem_fence_attach_ioctl returns -ENOENT.
 */
int vgem_fence_attach_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file)
{
	struct drm_vgem_fence_attach *arg = data;
	struct vgem_file *vfile = file->driver_priv;
	struct dma_resv *resv;
	struct drm_gem_object *obj;
	struct dma_fence *fence = NULL;
	int ret;

	if (arg->flags & ~VGEM_FENCE_WRITE)
		return -EINVAL;

	if (arg->pad)
		return -EINVAL;

	obj = drm_gem_object_lookup(file, arg->handle);
	if (!obj)
		return -ENOENT;

	fence = vgem_fence_create(vfile, arg->flags);
	if (!fence) {
		ret = -ENOMEM;
		goto err;
	}

	/* Check for a conflicting fence */
	resv = obj->resv;
	if (!dma_resv_test_signaled_rcu(resv,
						  arg->flags & VGEM_FENCE_WRITE)) {
		ret = -EBUSY;
		goto err_fence;
	}

	/* Expose the fence via the dma-buf */
	ret = 0;
	dma_resv_lock(resv, NULL);
	if (arg->flags & VGEM_FENCE_WRITE)
		dma_resv_add_excl_fence(resv, fence);
	else if ((ret = dma_resv_reserve_shared(resv, 1)) == 0)
		dma_resv_add_shared_fence(resv, fence);
	dma_resv_unlock(resv);

	/* Record the fence in our idr for later signaling */
	if (ret == 0) {
		mutex_lock(&vfile->fence_mutex);
		ret = idr_alloc(&vfile->fence_idr, fence, 1, 0, GFP_KERNEL);
		mutex_unlock(&vfile->fence_mutex);
		if (ret > 0) {
			arg->out_fence = ret;
			ret = 0;
		}
	}
err_fence:
	if (ret) {
		dma_fence_signal(fence);
		dma_fence_put(fence);
	}
err:
#if KERNEL_VERSION(5, 9, 0) <= LINUX_VERSION_CODE
	drm_gem_object_put(obj);
#else
	drm_gem_object_put_unlocked(obj);
#endif
	return ret;
}

/*
 * vgem_fence_signal_ioctl (DRM_IOCTL_VGEM_FENCE_SIGNAL):
 *
 * Signal and consume a fence ealier attached to a vGEM handle using
 * vgem_fence_attach_ioctl (DRM_IOCTL_VGEM_FENCE_ATTACH).
 *
 * All fences must be signaled within 10s of attachment or otherwise they
 * will automatically expire (and a vgem_fence_signal_ioctl returns -ETIMEDOUT).
 *
 * Signaling a fence indicates to all consumers of the dma-buf that the
 * client has completed the operation associated with the fence, and that the
 * buffer is then ready for consumption.
 *
 * If the fence does not exist (or has already been signaled by the client),
 * vgem_fence_signal_ioctl returns -ENOENT.
 */
int vgem_fence_signal_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file)
{
	struct vgem_file *vfile = file->driver_priv;
	struct drm_vgem_fence_signal *arg = data;
	struct dma_fence *fence;
	int ret = 0;

	if (arg->flags)
		return -EINVAL;

	mutex_lock(&vfile->fence_mutex);
	fence = idr_replace(&vfile->fence_idr, NULL, arg->fence);
	mutex_unlock(&vfile->fence_mutex);
	if (!fence)
		return -ENOENT;
	if (IS_ERR(fence))
		return PTR_ERR(fence);

	if (dma_fence_is_signaled(fence))
		ret = -ETIMEDOUT;

	dma_fence_signal(fence);
	dma_fence_put(fence);
	return ret;
}

int ftv310_vpu_testbufvalid(struct drm_device *dev, void *data,
			struct drm_file *file)
{
	struct ftv310_vpu_fencecheck *arg = data;
	struct dma_resv *resv;
	struct drm_gem_object *obj;
	int ready = 0;
	arg->ready = 0;

	obj = drm_gem_object_lookup(file, arg->handle);
	if (!obj)
		return -ENOENT;

	/* Check for a conflicting fence */
	resv = obj->resv;
	if (!dma_resv_test_signaled_rcu(resv, 1))
		arg->ready = 0;
	else
		arg->ready = 1;

	ready = arg->ready;

	ftv310_vpu_unref_drmobj(obj);
	return ready;
}
#endif
static struct dma_fence *alloc_fence(unsigned int ctxno)
{
	struct vgem_fence *fence;
    fence = kzalloc(sizeof(*fence), GFP_KERNEL);
	if (!fence)
		return NULL;

	spin_lock_init(&fence->lock);
	dma_fence_init(&fence->base, &vgem_fence_ops, &fence->lock,
		           ctxno, 1);

	timer_setup(&fence->timer, vgem_fence_timeout, 0);

	/* We force the fence to expire within 10s to prevent driver hangs */
	mod_timer(&fence->timer, jiffies + VGEM_FENCE_TIMEOUT);
    clear_bit(VGEM_FENCE_FLAG_SIGNAL_BIT, &fence->base.flags);
	/* TODO: din't set VGEM_FENCE_FLAG_ENABLE_SIGNAL_BIT same as ftv310_vpu_fence,
		if set, dma_fence_is_signaled() in vgem_fence_signal_ioctl will be true,
		then return -ETIMEDOUT.
	*/
	//set_bit(VGEM_FENCE_FLAG_ENABLE_SIGNAL_BIT, &fence->base.flags);
    printk("create vgem_fence fence\n");
    return &fence->base;
}


int vgem_fence_open(struct vgem_file *vfile)
{
	mutex_init(&vfile->fence_mutex);
#if KERNEL_VERSION(5, 10, 0) < LINUX_VERSION_CODE
	idr_init_base(&vfile->fence_idr, 1);
#else
	idr_init(&vfile->fence_idr);
#endif
	return 0;
}

static int __vgem_fence_idr_fini(int id, void *p, void *data)
{
	dma_fence_signal(p);
	dma_fence_put(p);
	return 0;
}

void vgem_fence_close(struct vgem_file *vfile)
{
	idr_for_each(&vfile->fence_idr, __vgem_fence_idr_fini, vfile);
	idr_destroy(&vfile->fence_idr);
}
