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


#ifndef __gc_hal_kernel_sync_h_
#define __gc_hal_kernel_sync_h_

#include <linux/types.h>

#ifndef CONFIG_SYNC_FILE

/* sync.h is in drivers/staging/android/ for now. */
#include <sync.h>

#include <gc_hal.h>
#include <gc_hal_base.h>

struct viv_sync_timeline {
    /* Parent object. */
    struct sync_timeline obj;

    /* Timestamp when sync_pt is created. */
    gctUINT64            stamp;

    /* Pointer to os struct. */
    gckOS                os;
};

struct viv_sync_pt {
    /* Parent object. */
    struct sync_pt       pt;

    /* Reference signal. */
    gctSIGNAL            signal;

    /* Timestamp when sync_pt is created. */
    gctUINT64            stamp;
};

/* Create viv_sync_timeline object. */
struct viv_sync_timeline *viv_sync_timeline_create(const char *name, gckOS Os);

/* Create viv_sync_pt object. */
struct sync_pt *viv_sync_pt_create(struct viv_sync_timeline *obj, gctSIGNAL signal);

#else

#include <linux/sync_file.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 10, 0)
#include <linux/fence.h>
#include <linux/fence-array.h>
# else
#include <linux/dma-fence.h>
#include <linux/dma-fence-array.h>
# endif

#include <gc_hal.h>
#include <gc_hal_base.h>
#include "gc_hal_kernel_linux.h"

struct viv_sync_timeline {
    char        name[64];

    /* Parent object. */
    u64         context;

    /* Timestamp when sync_pt is created. */
    atomic64_t  seqno;

    /* Pointer to os struct. */
    gckOS       os;
};

struct viv_fence {
    /* must be the first. */
    struct dma_fence          base;
    /* spinlock_t */
    spinlock_t                lock;

    struct viv_sync_timeline *parent;

    /* link with signal. */
    gctSIGNAL                 signal;
};

struct viv_sync_timeline *viv_sync_timeline_create(const char *name, gckOS Os);

void viv_sync_timeline_destroy(struct viv_sync_timeline *timeline);

struct dma_fence *viv_fence_create(struct viv_sync_timeline *timeline, gcsSIGNAL *signal);

#endif /* CONFIG_SYNC_FILE */

#endif /* __gc_hal_kernel_sync_h_ */
