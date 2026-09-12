/*
 * Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 *
 */

#ifndef _VGEM_DRV_H_
#define _VGEM_DRV_H_

#include <drm/drm_gem.h>
#include <drm/drm_cache.h>
#include <drm/drm_drv.h>
#include <drm/drm_file.h>
#include <drm/drm_ioctl.h>
#include <drm/drm_prime.h>
#include <uapi/drm/vgem_drm.h>

#define to_vgem_bo(x) container_of(x, struct drm_vgem_gem_object, base)
#define DRM_VGEM_TESTREADY	0x3
#define DRM_IOCTL_VGEM_FTV310_VPU_TESTREADY	DRM_IOWR(DRM_COMMAND_BASE + DRM_VGEM_TESTREADY, struct ftv310_vpu_fencecheck)



struct vgem_file {
	struct idr fence_idr;
	struct mutex fence_mutex;
};

struct vgem_device {
	struct drm_device drm;
	struct platform_device *platform;
};

struct drm_vgem_gem_object {
	struct drm_gem_object base;

	struct page **pages;
	unsigned int pages_pin_count;
	struct mutex pages_lock;

	struct sg_table *table;

	struct dma_resv kresv;
};

struct ftv310_vpu_fencecheck {
	unsigned int handle;
	int ready;
};

int vgem_fence_open(struct vgem_file *file);
int vgem_fence_attach_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file);
int vgem_fence_signal_ioctl(struct drm_device *dev,
			    void *data,
			    struct drm_file *file);

int ftv310_vpu_testbufvalid(struct drm_device *dev, void *data,
			struct drm_file *file);

void vgem_fence_close(struct vgem_file *file);

#endif
