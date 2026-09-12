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
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/swiotlb.h>

#define _GC_OBJ_ZONE gcvZONE_OS

/*
 * reserved_mem is for contiguous pool, internal pool and external pool, etc.
 */

/* mdl private. */
struct reserved_mem {
    unsigned long   start;
    unsigned long   size;
    unsigned int    offset_in_page;
    char            name[32];
    int             release;

    /* Link together. */
    struct          list_head link;
    /* the mdl is root or not */
    gctBOOL         root;
};

/* allocator info. */
struct reserved_mem_alloc {
    /* Record allocated reserved memory regions. */
    struct list_head region;
    /* protect reserved memory */
    struct mutex     lock;
};

static int reserved_mem_show(struct seq_file *m, void *data)
{
    struct list_head *pos;
    gcsINFO_NODE *node = m->private;
    gckALLOCATOR Allocator = node->device;
    struct reserved_mem_alloc *alloc = Allocator->privateData;

    list_for_each(pos, &alloc->region) {
        struct reserved_mem *res = list_entry(pos, struct reserved_mem, link);

        seq_printf(m, "0x%08lx-0x%08lx : %s\n",
                   res->start, res->start + res->size - 1, res->name);
    }

    return 0;
}

static gcsINFO info_list[] = {
    { "reserved-mem", reserved_mem_show },
};

static void
reserved_mem_debugfs_init(gckALLOCATOR Allocator, gckDEBUGFS_DIR Root)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_Init(&Allocator->debugfsDir,
                                      Root->root, "reserved-mem"));

    gcmkVERIFY_OK(gckDEBUGFS_DIR_CreateFiles(&Allocator->debugfsDir, info_list,
                                             gcmCOUNTOF(info_list), Allocator));
}

static void
reserved_mem_debugfs_cleanup(gckALLOCATOR Allocator)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(&Allocator->debugfsDir,
                                             info_list, gcmCOUNTOF(info_list)));

    gckDEBUGFS_DIR_Deinit(&Allocator->debugfsDir);
}

static gceSTATUS
reserved_mem_alloc(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T NumPages, gctUINT32 Flags)
{
    return gcvSTATUS_OUT_OF_MEMORY;
}

static gceSTATUS
reserved_mem_attach(gckALLOCATOR Allocator, gcsATTACH_DESC_PTR Desc, PLINUX_MDL Mdl)
{
    struct reserved_mem_alloc *alloc = Allocator->privateData;
    struct reserved_mem *res;
    struct resource *region  = NULL;
    gctPHYS_ADDR_T gpu_end = 0;
    gctBOOL acquiredMutex = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;

    if (Desc == gcvNULL)
        return gcvSTATUS_INVALID_ARGUMENT;

    res = kzalloc(sizeof(*res), GFP_KERNEL | gcdNOWARN);

    if (!res)
        return gcvSTATUS_OUT_OF_MEMORY;

    res->start = Desc->reservedMem.start;
    res->size = Desc->reservedMem.size;
    res->offset_in_page = Desc->reservedMem.start & (PAGE_SIZE - 1);
    strncpy(res->name, Desc->reservedMem.name, sizeof(res->name) - 1);
    res->release = 0;
    res->root = Desc->reservedMem.root;

    /* the region requierd is handed by root MDL */
    if (Desc->reservedMem.root) {
        if (!Desc->reservedMem.requested) {
            region = request_mem_region(res->start, res->size, res->name);

            if (!region) {
                pr_warn("request mem %s(0x%lx - 0x%lx) failed\n",
                        res->name, res->start, res->start + res->size - 1);

                kfree(res);
                return gcvSTATUS_OUT_OF_RESOURCES;
            }

            res->release = 1;
        }

        gcmkONERROR(gckOS_AcquireMutex(Allocator->os, &alloc->lock, gcvINFINITE));
        acquiredMutex = gcvTRUE;

        list_add(&res->link, &alloc->region);

        gcmkONERROR(gckOS_ReleaseMutex(Allocator->os, &alloc->lock));
        acquiredMutex = gcvFALSE;

    }

    Mdl->priv = res;

    gckOS_CPUPhysicalToGPUPhysical(Allocator->os,
                                   (res->start + res->size - 1),
                                   &gpu_end);
    if (gpu_end <= 0xFFFFFFFF)
        Allocator->capability |= gcvALLOC_FLAG_4GB_ADDR;

    return gcvSTATUS_OK;

OnError:
    if (acquiredMutex)
        gckOS_ReleaseMutex(Allocator->os, &alloc->lock);

    return status;
}

static void
reserved_mem_detach(gckALLOCATOR Allocator, PLINUX_MDL Mdl)
{
    struct reserved_mem_alloc *alloc = Allocator->privateData;
    struct reserved_mem *res = Mdl->priv;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL acquiredMutex = gcvFALSE;

    if (res->root) {
        /* unlink from region list. */
        gcmkONERROR(gckOS_AcquireMutex(Allocator->os, &alloc->lock, gcvINFINITE));
        acquiredMutex = gcvTRUE;

        list_del_init(&res->link);

        gcmkONERROR(gckOS_ReleaseMutex(Allocator->os, &alloc->lock));
        acquiredMutex = gcvFALSE;

        if (res->release)
            release_mem_region(res->start, res->size);
    }

    kfree(res);

OnError:
    if (acquiredMutex)
        gckOS_ReleaseMutex(Allocator->os, &alloc->lock);

}

static gceSTATUS
reserved_mem_mmap(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctBOOL Cacheable,
                  gctSIZE_T skipPages, gctSIZE_T numPages, struct vm_area_struct *vma)
{
    struct reserved_mem *res = (struct reserved_mem *)Mdl->priv;
    unsigned long pfn;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p vma=%p", Allocator, Mdl, vma);

    gcmkASSERT(skipPages + numPages <= Mdl->numPages);

    pfn = (res->start >> PAGE_SHIFT) + skipPages;

    /* Make this mapping non-cached. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)) || \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 26)) && defined(CONFIG_ANDROID))
    vm_flags_set(vma, gcdVM_FLAGS);
#else
    vma->vm_flags |= gcdVM_FLAGS;
#endif

#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#else
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

    if (remap_pfn_range(vma, vma->vm_start, pfn,
                        numPages << PAGE_SHIFT, vma->vm_page_prot) < 0) {
        gcmkTRACE(gcvLEVEL_ERROR,
                  "%s(%d): remap_pfn_range error.",
                  __func__, __LINE__);

        status = gcvSTATUS_OUT_OF_MEMORY;
    }

    gcmkFOOTER();
    return status;
}

static void
reserved_mem_unmap_user(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                        PLINUX_MDL_MAP MdlMap, gctUINT32 Size)
{
    struct reserved_mem *res = (struct reserved_mem *)Mdl->priv;

    if (unlikely(!current->mm))
        return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
    if (vm_munmap((unsigned long)MdlMap->vmaAddr - res->offset_in_page, res->size) < 0)
        pr_warn("%s: vm_munmap failed\n", __func__);
#else
    down_write(&current_mm_mmap_sem);
    if (do_munmap(current->mm,
                  (unsigned long)MdlMap->vmaAddr - res->offset_in_page,
                  res->size) < 0)
        pr_warn("%s: do_munmap failed\n", __func__);

    up_write(&current_mm_mmap_sem);
#endif

    MdlMap->vma = NULL;
    MdlMap->vmaAddr = NULL;
}

static gceSTATUS
reserved_mem_map_user(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                      PLINUX_MDL_MAP MdlMap, gctBOOL Cacheable)
{
    struct reserved_mem *res = (struct reserved_mem *)Mdl->priv;
    gctPOINTER userLogical = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p Cacheable=%d", Allocator, Mdl, Cacheable);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
#if gcdANON_FILE_FOR_ALLOCATOR
    userLogical = (gctPOINTER)vm_mmap(Allocator->anon_file, 0L, res->size,
# else
    userLogical = (gctPOINTER)vm_mmap(NULL, 0L, res->size,
# endif
                                      PROT_READ | PROT_WRITE, MAP_SHARED | MAP_NORESERVE, 0);
#else
    down_write(&current_mm_mmap_sem);
    userLogical = (gctPOINTER)do_mmap_pgoff(NULL, 0L, res->size,
                                            PROT_READ | PROT_WRITE, MAP_SHARED, 0);
    up_write(&current_mm_mmap_sem);
#endif

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                   "%s(%d): vmaAddr->%p for phys_addr->%p",
                   __func__, __LINE__, userLogical, Mdl);

    if (IS_ERR(userLogical)) {
        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                       "%s(%d): do_mmap_pgoff error",
                       __func__, __LINE__);

        userLogical = gcvNULL;

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    down_write(&current_mm_mmap_sem);

    do {
        struct vm_area_struct *vma = find_vma(current->mm, (unsigned long)userLogical);

        if (vma == gcvNULL) {
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                           "%s(%d): find_vma error",
                           __func__, __LINE__);

            gcmkERR_BREAK(gcvSTATUS_OUT_OF_RESOURCES);
        }

        gcmkERR_BREAK(reserved_mem_mmap(Allocator, Mdl, gcvFALSE, 0, Mdl->numPages, vma));

        MdlMap->vmaAddr = userLogical + res->offset_in_page;
        MdlMap->cacheable = gcvFALSE;
        MdlMap->vma = vma;
    } while (gcvFALSE);

    up_write(&current_mm_mmap_sem);

OnError:
    if (gcmIS_ERROR(status) && userLogical) {
        MdlMap->vmaAddr = userLogical + res->offset_in_page;
        reserved_mem_unmap_user(Allocator, Mdl, MdlMap, res->size);
    }

    gcmkFOOTER();
    return status;
}

static gceSTATUS
reserved_mem_map_kernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                        gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *Logical)
{
    struct reserved_mem *res = Mdl->priv;
    void *vaddr;

    if (Offset + Bytes > res->size)
        return gcvSTATUS_INVALID_ARGUMENT;

#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
    vaddr = ioremap_wc(res->start + Offset, Bytes);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
    vaddr = ioremap(res->start + Offset, Bytes);
# else
    vaddr = ioremap_nocache(res->start + Offset, Bytes);
# endif
#endif

    if (!vaddr)
        return gcvSTATUS_OUT_OF_MEMORY;

    *Logical = vaddr;
    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_unmap_kernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctPOINTER Logical)
{
    iounmap((void *)Logical);

    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_cache_op(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset,
                      gctPOINTER Logical, gctSIZE_T Bytes, gceCACHEOPERATION Operation)
{
    /* Always WC or UC, safe to use mb. */
    switch (Operation) {
    case gcvCACHE_CLEAN:
    case gcvCACHE_FLUSH:
        _MemoryBarrier();
        break;
    case gcvCACHE_INVALIDATE:
        break;
    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_get_physical(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                          unsigned long Offset, gctPHYS_ADDR_T *Physical)
{
    struct reserved_mem *res = Mdl->priv;
    *Physical = res->start + Offset;

    return gcvSTATUS_OK;
}

static gceSTATUS
reserved_mem_GetSGT(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                    gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *SGT)
{
    struct page *page = gcvNULL;
    struct sg_table *sgt = NULL;
    struct reserved_mem *res = Mdl->priv;
    gceSTATUS status = gcvSTATUS_OK;
    struct scatterlist *sg = NULL;
    gctUINT32 offset_in_page = 0;
    unsigned long mem_base = res->start + Offset;
    gctSIZE_T sz = 0;
    gctSIZE_T left = Bytes;
    gctSIZE_T chunk_size = IO_TLB_SEGSIZE * (1 << IO_TLB_SHIFT);

    gcmkASSERT(Offset + Bytes <= Mdl->numPages << PAGE_SHIFT);

    sgt = kmalloc(sizeof(*sgt), GFP_KERNEL);
    if (!sgt)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    if (sg_alloc_table(sgt, (Bytes + chunk_size - 1) / chunk_size, GFP_KERNEL))
        gcmkONERROR(gcvSTATUS_GENERIC_IO);

    sg = sgt->sgl;
    while (left) {
        if (left > chunk_size)
            sz = chunk_size;
        else
            sz = left;

        left -= sz;

        page = pfn_to_page(mem_base >> PAGE_SHIFT);
        offset_in_page = (gctUINT32)mem_base & (PAGE_SIZE - 1);

        sg_set_page(sg, page, sz, offset_in_page);

        mem_base += sz;
        sg = sg_next(sg);
        if (!sg)
            break;
    }

    if (left)
        pr_warn("Bytes is mismatch with sgt size, start: 0x%llx, left: 0x%llx, Offset: 0x%llx, Bytes: 0x%llx\n",
                (gctUINT64)res->start,
                (gctUINT64)left,
                (gctUINT64)Offset,
                (gctUINT64)Bytes);

    *SGT = (gctPOINTER)sgt;

OnError:
    if (gcmIS_ERROR(status) && sgt)
        kfree(sgt);

    return status;
}

static void
reserved_mem_dtor(gcsALLOCATOR *Allocator)
{
    reserved_mem_debugfs_cleanup(Allocator);

    gckOS_ZeroMemory(Allocator->privateData, sizeof(struct reserved_mem_alloc));
    kfree(Allocator->privateData);

    kfree(Allocator);
}

/* GFP allocator operations. */
static gcsALLOCATOR_OPERATIONS reserved_mem_ops = {
    .Alloc       = reserved_mem_alloc,
    .Attach      = reserved_mem_attach,
    .Free        = reserved_mem_detach,
    .Mmap        = reserved_mem_mmap,
    .MapUser     = reserved_mem_map_user,
    .UnmapUser   = reserved_mem_unmap_user,
    .MapKernel   = reserved_mem_map_kernel,
    .UnmapKernel = reserved_mem_unmap_kernel,
    .Cache       = reserved_mem_cache_op,
    .Physical    = reserved_mem_get_physical,
    .GetSGT      = reserved_mem_GetSGT,
};

/* GFP allocator entry. */
gceSTATUS
_ReservedMemoryAllocatorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator)
{
    gceSTATUS status;
    gckALLOCATOR allocator = gcvNULL;
    struct reserved_mem_alloc *alloc = NULL;

    gcmkONERROR(gckALLOCATOR_Construct(Os, &reserved_mem_ops, &allocator));

    alloc = kzalloc(sizeof(*alloc), GFP_KERNEL | gcdNOWARN);

    if (!alloc)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    INIT_LIST_HEAD(&alloc->region);
    mutex_init(&alloc->lock);

    /* Register private data. */
    allocator->privateData = alloc;
    allocator->destructor = reserved_mem_dtor;

    reserved_mem_debugfs_init(allocator, Parent);

    allocator->capability = gcvALLOC_FLAG_LINUX_RESERVED_MEM
                          | gcvALLOC_FLAG_CONTIGUOUS
                          | gcvALLOC_FLAG_MEMLIMIT
                          | gcvALLOC_FLAG_CPU_ACCESS
                          | gcvALLOC_FLAG_DMABUF_EXPORTABLE
                          | gcvALLOC_FLAG_NON_CPU_ACCESS
                          | gcvALLOC_FLAG_FROM_USER
#if gcdENABLE_VIDEO_MEMORY_MIRROR
                          | gcvALLOC_FLAG_WITH_MIRROR
#endif
                          | gcvALLOC_FLAG_32BIT_VA
                          | gcvALLOC_FLAG_PRIOR_32BIT_VA
                          ;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    kfree(allocator);
    return status;
}
