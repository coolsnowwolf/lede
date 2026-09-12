/****************************************************************************
*
*    The MIT License (MIT)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
*
*    Permission is hereby granted, free of charge, to any person obtaining a
*    copy of this software and associated documentation files (the "Software"),
*    to deal in the Software without restriction, including without limitation
*    the rights to use, copy, modify, merge, publish, distribute, sublicense,
*    and/or sell copies of the Software, and to permit persons to whom the
*    Software is furnished to do so, subject to the following conditions:
*
*    The above copyright notice and this permission notice shall be included in
*    all copies or substantial portions of the Software.
*
*    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
*    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
*    DEALINGS IN THE SOFTWARE.
*
*****************************************************************************
*
*    The GPL License (GPL)
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.
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
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software Foundation,
*    Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*****************************************************************************
*
*    Note: This software is released under dual MIT and GPL licenses. A
*    recipient may use this file under the terms of either the MIT license or
*    GPL License. If you wish to use only one license not the other, you can
*    indicate your decision by deleting one of the above license notices in your
*    version of this file.
*
*****************************************************************************/


#include <gc_hal.h>
#include <gc_hal_base.h>

#if gcdLINUX_SYNC_FILE

#include <linux/kernel.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#include "gc_hal_kernel_sync.h"
#include "gc_hal_kernel_linux.h"

#ifndef CONFIG_SYNC_FILE

static struct sync_pt *viv_sync_pt_dup(struct sync_pt *sync_pt)
{
    gceSTATUS status;
    struct viv_sync_pt *pt;
    struct viv_sync_pt *src;
    struct viv_sync_timeline *obj;

    src = (struct viv_sync_pt *)sync_pt;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
    obj = (struct viv_sync_timeline *)sync_pt_parent(sync_pt);
# else
    obj = (struct viv_sync_timeline *)sync_pt->parent;
# endif

    /* Create the new sync_pt. */
    pt = (struct viv_sync_pt *)sync_pt_create(&obj->obj, sizeof(struct viv_sync_pt));

    pt->stamp = src->stamp;

    /* Reference signal. */
    status = gckOS_MapSignal(obj->os,
                             src->signal,
                             gcvNULL /* (gctHANDLE) _GetProcessID() */,
                             &pt->signal);

    if (gcmIS_ERROR(status)) {
        sync_pt_free((struct sync_pt *)pt);
        return NULL;
    }

    return (struct sync_pt *)pt;
}

static int viv_sync_pt_has_signaled(struct sync_pt *sync_pt)
{
    gceSTATUS status;
    struct viv_sync_pt *pt;
    struct viv_sync_timeline *obj;

    pt  = (struct viv_sync_pt *)sync_pt;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
    obj = (struct viv_sync_timeline *)sync_pt_parent(sync_pt);
# else
    obj = (struct viv_sync_timeline *)sync_pt->parent;
# endif

    status = _QuerySignal(obj->os, pt->signal);

    if (gcmIS_ERROR(status)) {
        /* Error. */
        return -1;
    }

    return (int)status;
}

static int viv_sync_pt_compare(struct sync_pt *a, struct sync_pt *b)
{
    int ret;
    struct viv_sync_pt *pt1 = (struct viv_sync_pt *)a;
    struct viv_sync_pt *pt2 = (struct viv_sync_pt *)b;

    ret = (pt1->stamp < pt2->stamp) ?
          -1 : (pt1->stamp == pt2->stamp) ? 0 : 1;

    return ret;
}

static void viv_sync_pt_free(struct sync_pt *sync_pt)
{
    struct viv_sync_pt *pt;
    struct viv_sync_timeline *obj;

    pt = (struct viv_sync_pt *)sync_pt;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
    obj = (struct viv_sync_timeline *)sync_pt_parent(sync_pt);
# else
    obj = (struct viv_sync_timeline *)sync_pt->parent;
# endif

    gckOS_DestroySignal(obj->os, pt->signal);
}

static void viv_timeline_value_str(struct sync_timeline *timeline, char *str, int size)
{
    struct viv_sync_timeline *obj;

    obj = (struct viv_sync_timeline *)timeline;
    snprintf(str, size, "stamp_%llu", obj->stamp);
}

static void viv_pt_value_str(struct sync_pt *sync_pt, char *str, int size)
{
    struct viv_sync_pt *pt;

    pt = (struct viv_sync_pt *)sync_pt;
    snprintf(str, size, "signal_%lu@stamp_%llu", (unsigned long)pt->signal, pt->stamp);
}

static struct sync_timeline_ops viv_timeline_ops = {
    .driver_name        = "viv_gpu_sync",
    .dup                = viv_sync_pt_dup,
    .has_signaled       = viv_sync_pt_has_signaled,
    .compare            = viv_sync_pt_compare,
    .free_pt            = viv_sync_pt_free,
    .timeline_value_str = viv_timeline_value_str,
    .pt_value_str       = viv_pt_value_str,
};

struct viv_sync_timeline *viv_sync_timeline_create(const char *name, gckOS os)
{
    struct viv_sync_timeline *obj;

    obj = (struct viv_sync_timeline *)sync_timeline_create(&viv_timeline_ops,
                                                           sizeof(struct viv_sync_timeline),
                                                           name);

    obj->os = os;
    obj->stamp = 0;

    return obj;
}

struct sync_pt *viv_sync_pt_create(struct viv_sync_timeline *obj, gctSIGNAL Signal)
{
    gceSTATUS status;
    struct viv_sync_pt *pt;

    pt = (struct viv_sync_pt *)sync_pt_create(&obj->obj, sizeof(struct viv_sync_pt));

    pt->stamp = obj->stamp++;

    /* Dup signal. */
    status = gckOS_MapSignal(obj->os, Signal,
                             gcvNULL /* (gctHANDLE) _GetProcessID() */,
                             &pt->signal);

    if (gcmIS_ERROR(status)) {
        sync_pt_free((struct sync_pt *)pt);
        return NULL;
    }

    return (struct sync_pt *)pt;
}

# else

struct viv_sync_timeline *viv_sync_timeline_create(const char *name, gckOS Os)
{
    struct viv_sync_timeline *timeline;

    timeline = kmalloc(sizeof(*timeline), gcdNOWARN | GFP_KERNEL);

    if (!timeline)
        return NULL;

    strncpy(timeline->name, name, sizeof(timeline->name) - 1);
    timeline->context = dma_fence_context_alloc(1);
    atomic64_set(&timeline->seqno, 0);
    timeline->os = Os;

    return timeline;
}

void viv_sync_timeline_destroy(struct viv_sync_timeline *timeline)
{
    kfree(timeline);
}

static const char *viv_fence_get_driver_name(struct dma_fence *fence)
{
    return "viv_gpu_sync";
}

static const char *viv_fence_get_timeline_name(struct dma_fence *fence)
{
    struct viv_fence *f = (struct viv_fence *)fence;

    return f->parent->name;
}

/* Same as fence_signaled. */
static inline bool __viv_fence_signaled(struct dma_fence *fence)
{
    struct viv_fence *f = (struct viv_fence *)fence;
    struct viv_sync_timeline *timeline = f->parent;
    gceSTATUS status;

    status = _QuerySignal(timeline->os, f->signal);

    return (status == gcvSTATUS_TRUE) ? true : false;
}

static bool viv_fence_enable_signaling(struct dma_fence *fence)
{
    /* fence is locked already. */
    return !__viv_fence_signaled(fence);
}

static bool viv_fence_signaled(struct dma_fence *fence)
{
    /* fence could be locked, could be not. */
    return __viv_fence_signaled(fence);
}

static void viv_fence_release(struct dma_fence *fence)
{
    struct viv_fence *f = (struct viv_fence *)fence;
    struct viv_sync_timeline *timeline = f->parent;

    if (f->signal)
        gckOS_DestroySignal(timeline->os, f->signal);

    kfree(fence);
}

static struct dma_fence_ops viv_fence_ops = {
    .get_driver_name   = viv_fence_get_driver_name,
    .get_timeline_name = viv_fence_get_timeline_name,
    .enable_signaling  = viv_fence_enable_signaling,
    .signaled          = viv_fence_signaled,
    .wait              = dma_fence_default_wait,
    .release           = viv_fence_release,
};

struct dma_fence *viv_fence_create(struct viv_sync_timeline *timeline, gcsSIGNAL *signal)
{
    gceSTATUS status;
    struct viv_fence *fence;
    struct dma_fence *old_fence = NULL;
    unsigned int seqno;

    fence = kzalloc(sizeof(*fence), gcdNOWARN | GFP_KERNEL);

    if (!fence)
        return NULL;

    /* Reference signal in fence. */
    status = gckOS_MapSignal(timeline->os,
                             (gctSIGNAL)(uintptr_t)signal->id,
                             NULL,
                             &fence->signal);

    if (gcmIS_ERROR(status)) {
        kfree(fence);
        return NULL;
    }

    spin_lock_init(&fence->lock);

    fence->parent = timeline;

    seqno = (unsigned int)atomic64_inc_return(&timeline->seqno);

    dma_fence_init((struct dma_fence *)fence, &viv_fence_ops,
                   &fence->lock, timeline->context, seqno);

    /*
     * Reference fence in signal.
     * Be aware of recursive reference!!
     */
    spin_lock(&signal->lock);

    if (signal->fence) {
        old_fence = signal->fence;
        signal->fence = NULL;
    }

    if (!signal->done) {
        signal->fence = (struct dma_fence *)fence;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 9, 68)
        dma_fence_get((struct dma_fence *)fence);
#  endif
    }

    spin_unlock(&signal->lock);

    if (old_fence)
        dma_fence_put(old_fence);

    if (!signal->fence) {
        /* Fence already signaled. */
        gckOS_DestroySignal(timeline->os, fence->signal);
        fence->signal = NULL;

        dma_fence_signal_locked((struct dma_fence *)fence);
    }

    return (struct dma_fence *)fence;
}

# endif /* CONFIG_SYNC_FILE */

#endif /* gcdLINUX_SYNC_FILE */
