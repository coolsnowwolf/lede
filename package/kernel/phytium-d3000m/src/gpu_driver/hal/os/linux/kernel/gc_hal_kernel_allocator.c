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


#include "gc_hal_kernel_linux.h"
#include "gc_hal_kernel_allocator.h"
#include <linux/pagemap.h>
#include <linux/seq_file.h>
#include <linux/mman.h>
#include <linux/atomic.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
# include <linux/anon_inodes.h>
#endif
#include <linux/file.h>

#include "gc_hal_kernel_allocator_array.h"
#include "gc_hal_kernel_platform.h"

#define _GC_OBJ_ZONE gcvZONE_OS

/*******************************************************************************
 ******************************* Debugfs Support *******************************
 *******************************************************************************/
static gceSTATUS
_AllocatorDebugfsInit(gckOS Os)
{
    gceSTATUS status;
    gckGALDEVICE device = Os->device;

    gckDEBUGFS_DIR dir = &Os->allocatorDebugfsDir;

    gcmkONERROR(gckDEBUGFS_DIR_Init(dir, device->debugfsDir.root, "allocators"));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static void
_AllocatorDebugfsCleanup(gckOS Os)
{
    gckDEBUGFS_DIR dir = &Os->allocatorDebugfsDir;

    gckDEBUGFS_DIR_Deinit(dir);
}

/****************************************************************************
 *********************** Allocator management *******************************
 ****************************************************************************/
#if gcdANON_FILE_FOR_ALLOCATOR
static int
tmp_mmap(struct file *fp, struct vm_area_struct *vma)
{
    return 0;
}

static const struct file_operations tmp_fops = {
    .mmap = tmp_mmap,
};

#endif

gceSTATUS
gckOS_ImportAllocators(gckOS Os)
{
    gceSTATUS status;
    gctUINT i;
    gckALLOCATOR allocator;

#if gcdANON_FILE_FOR_ALLOCATOR
    struct file *anon_file = gcvNULL;
    gctINT32 ufd = 0;

    ufd = anon_inode_getfd("[ftg340]", &tmp_fops, gcvNULL, O_RDWR);
    anon_file = fget(ufd);
#endif

    _AllocatorDebugfsInit(Os);

    INIT_LIST_HEAD(&Os->allocatorList);

    for (i = 0; i < gcmCOUNTOF(allocatorArray); i++) {
        if (allocatorArray[i].construct) {
            /* Construct allocator. */
            status = allocatorArray[i].construct(Os, &Os->allocatorDebugfsDir, &allocator);

            if (gcmIS_ERROR(status)) {
                gcmkPRINT("[" DEVICE_NAME "]: Can't construct allocator(%s)",
                          allocatorArray[i].name);

                continue;
            }

            allocator->name = allocatorArray[i].name;
#if gcdANON_FILE_FOR_ALLOCATOR
            allocator->anon_file = anon_file;
#endif
            list_add_tail(&allocator->link, &Os->allocatorList);
        }
    }

#if gcdDEBUG
    list_for_each_entry(allocator, &Os->allocatorList, link) {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
                       "%s(%d) Allocator: %s",
                       __func__, __LINE__, allocator->name);
    }
#endif

    return gcvSTATUS_OK;
}

gceSTATUS
gckOS_FreeAllocators(gckOS Os)
{
    gckALLOCATOR allocator;
    gckALLOCATOR temp;

    list_for_each_entry_safe(allocator, temp, &Os->allocatorList, link) {
        list_del(&allocator->link);

        /* Destroy allocator. */
        allocator->destructor(allocator);
    }

    _AllocatorDebugfsCleanup(Os);

    return gcvSTATUS_OK;
}

#if !gcdUSE_LINUX_SG_TABLE_API

#if LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 23)
static inline void
sg_set_page(struct scatterlist *sg, struct page *page,
            unsigned int len, unsigned int offset)
{
    sg->page = page;
    sg->offset = offset;
    sg->length = len;
}

static inline void
sg_mark_end(struct scatterlist *sg)
{
    (void)sg;
}
# endif

int
alloc_sg_list_from_pages(struct scatterlist **sgl, struct page **pages,
                         unsigned int n_pages, unsigned long offset,
                         unsigned long size, unsigned int *nents)
{
    unsigned int chunks;
    unsigned int i;
    unsigned int cur_page;
    struct scatterlist *s;

    chunks = 1;

    for (i = 1; i < n_pages; ++i) {
        if (page_to_pfn(pages[i]) != page_to_pfn(pages[i - 1]) + 1)
            ++chunks;
    }

    s = kzalloc(sizeof(*s) * chunks, GFP_KERNEL);
    if (unlikely(!s))
        return -ENOMEM;

    *sgl = s;
    *nents = chunks;

    cur_page = 0;

    for (i = 0; i < chunks; i++, s++) {
        unsigned long chunk_size;
        unsigned int j;

        for (j = cur_page + 1; j < n_pages; j++) {
            if (page_to_pfn(pages[j]) != page_to_pfn(pages[j - 1]) + 1)
                break;
        }

        chunk_size = ((j - cur_page) << PAGE_SHIFT) - offset;
        sg_set_page(s, pages[cur_page], min(size, chunk_size), offset);
        size -= chunk_size;
        offset = 0;
        cur_page = j;
    }

    sg_mark_end(s - 1);

    return 0;
}
#endif  /* #if !gcdUSE_LINUX_SG_TABLE_API */
