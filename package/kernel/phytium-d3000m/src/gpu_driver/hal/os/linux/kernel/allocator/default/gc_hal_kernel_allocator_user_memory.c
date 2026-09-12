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
#include <linux/scatterlist.h>

#include <linux/slab.h>
#include <linux/pagemap.h>
#include <linux/cache.h>

#define _GC_OBJ_ZONE        gcvZONE_ALLOCATOR

#define gcdUSING_PFN_FOLLOW 0

enum um_desc_type {
    UM_PHYSICAL_MAP,
    UM_PAGE_MAP,
    UM_PFN_MAP,
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
struct sg_table {
    struct scatterlist *sgl;
    unsigned int        nents;
    unsigned int        orig_nents;
};
#endif

/* Descriptor of a user memory imported. */
struct um_desc {
    int                     type;

    union {
        /* UM_PHYSICAL_MAP. */
        unsigned long       physical;

        /* UM_PAGE_MAP. */
        struct {
            struct page   **pages;
        };

        /* UM_PFN_MAP. */
        struct {
            unsigned long  *pfns;
            int            *refs;
        };
    };

    struct sg_table         sgt;

    /* contiguous chunks, does not include padding pages. */
    int                     chunk_count;

    unsigned long           vm_flags;
    unsigned long           user_vaddr;
    size_t                  size;
    unsigned long           offset;
    dma_addr_t              dmaHandle;

    size_t                  pageCount;
    size_t                  extraPage;
    unsigned int            alloc_from_res;
};

static void *alloc_memory(size_t bytes)
{
    if (bytes > PAGE_SIZE)
        return vzalloc(bytes);
    else
        return kzalloc(bytes, GFP_KERNEL | gcdNOWARN);
}

static void free_memory(void *ptr)
{
    if (is_vmalloc_addr(ptr))
        vfree(ptr);
    else
        kfree(ptr);
}

static int import_physical_map(gckOS Os, struct device *dev,
                               struct um_desc *um, unsigned long phys)
{
    um->type = UM_PHYSICAL_MAP;
    um->physical = phys & PAGE_MASK;
    um->chunk_count = 1;

    if (Os->iommu) {
        dma_addr_t dmaHandle;
        size_t size = um->size + (phys & (~PAGE_MASK));
        unsigned long pfn = phys >> PAGE_SHIFT;

        if (pfn_valid(pfn))
            dmaHandle = dma_map_page(dev, pfn_to_page(pfn),
                                     0, size, DMA_BIDIRECTIONAL);
        else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
            dmaHandle = dma_map_resource(dev, pfn << PAGE_SHIFT,
                                         size, DMA_BIDIRECTIONAL, 0);
#else
            dmaHandle = dma_map_page(dev, pfn_to_page(pfn),
                                     0, size, DMA_BIDIRECTIONAL);
#endif

        if (dma_mapping_error(dev, dmaHandle))
            return gcvSTATUS_OUT_OF_MEMORY;

        um->dmaHandle = dmaHandle;
    }

    return 0;
}

static gceSTATUS
phy_is_from_reserved(gckOS Os, gctPHYS_ADDR_T Physical)
{
    gceSTATUS status = gcvSTATUS_NOT_FOUND;
    gckGALDEVICE device = Os->device;
    gctUINT32 i = 0;

    if (device->lowContiguousSize) {
        if ((Physical >= device->lowContiguousBase) &&
            (Physical < device->lowContiguousBase + device->lowContiguousSize)) {
            status = gcvSTATUS_OK;
            return status;
        }
    }
    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (device->contiguousSizes[i]) {
            if ((Physical >= device->contiguousBases[i]) &&
                (Physical < device->contiguousBases[i] + device->contiguousSizes[i])) {
                status = gcvSTATUS_OK;
                return status;
            }
        }
    }
    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (device->externalSize[i]) {
            if ((Physical >= device->externalBase[i]) &&
                (Physical < device->externalBase[i] + device->externalSize[i])) {
                status = gcvSTATUS_OK;
                return status;
            }
        }
    }
    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (device->exclusiveSize[i]) {
            if ((Physical >= device->exclusiveBase[i]) &&
                (Physical < device->exclusiveBase[i] + device->exclusiveSize[i])) {
                status = gcvSTATUS_OK;
                return status;
            }
        }
    }
    return status;
}

static int
import_page_map(gckOS Os, struct device *dev, struct um_desc *um,
                unsigned long addr, size_t page_count, size_t size, unsigned long flags)
{
    int i;
    int result;
    struct page **pages;

    if ((addr & (cache_line_size() - 1)) || (size & (cache_line_size() - 1)))
        /* Not cpu cacheline size aligned, can not support. */
        return -EINVAL;

    pages = alloc_memory(page_count * sizeof(void *));
    if (!pages)
        return -ENOMEM;

    down_read(&current_mm_mmap_sem);

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 6, 0)
    result = pin_user_pages(addr & PAGE_MASK, page_count,
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4, 6, 0)
    result = get_user_pages(current, current->mm, addr & PAGE_MASK, page_count,
#else
    result = get_user_pages(addr & PAGE_MASK, page_count,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0) || defined(CONFIG_PPC)
                            (flags & VM_WRITE) ? FOLL_WRITE : 0,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 5, 1)
                            (flags & VM_WRITE) ? 1 : 0, 0,
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 168)
                            (flags & VM_WRITE) ? FOLL_WRITE : 0,
#else
                            (flags & VM_WRITE) ? 1 : 0, 0,
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0)
                            pages);
#else
                            pages, NULL);
#endif

    up_read(&current_mm_mmap_sem);

    if (result < page_count) {
        for (i = 0; i < result; i++) {
            if (pages[i])
#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 6, 0)
                unpin_user_page(pages[i]);
#else
                put_page(pages[i]);
#endif
        }

        free_memory(pages);
        return -ENODEV;
    }

    um->chunk_count = 1;
    for (i = 1; i < page_count; i++) {
        if (page_to_pfn(pages[i]) != page_to_pfn(pages[i - 1]) + 1)
            ++um->chunk_count;
    }

    if (gcmIS_SUCCESS(phy_is_from_reserved(Os, (gctPHYS_ADDR_T)page_to_pfn(pages[0]) << PAGE_SHIFT)))
        um->alloc_from_res = 1;

    if (!um->alloc_from_res) {
#if gcdUSE_LINUX_SG_TABLE_API
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        size_t max_segment = SIZE_MAX;

        max_segment = dma_max_mapping_size(dev);
# endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
        result = sg_alloc_table_from_pages_segment(&um->sgt, pages, page_count, addr & ~PAGE_MASK,
                                                   size, max_segment, GFP_KERNEL | gcdNOWARN);
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        result = PTR_ERR_OR_ZERO(__sg_alloc_table_from_pages(&um->sgt, pages, page_count,
                                   addr & ~PAGE_MASK, size, max_segment, gcvNULL, 0, GFP_KERNEL | gcdNOWARN));
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        max_segment = SCATTERLIST_MAX_SEGMENT;
        result = __sg_alloc_table_from_pages(&um->sgt, pages, page_count, addr & ~PAGE_MASK,
                                             size, max_segment, GFP_KERNEL | gcdNOWARN);
# else
        result = sg_alloc_table_from_pages(&um->sgt, pages, page_count,
                                           addr & ~PAGE_MASK, size, GFP_KERNEL | gcdNOWARN);
# endif
#else /* gcdUSE_LINUX_SG_TABLE_API */
        result = alloc_sg_list_from_pages(&um->sgt.sgl, pages, page_count,
                                          addr & ~PAGE_MASK, size, &um->sgt.nents);

        um->sgt.orig_nents = um->sgt.nents;
#endif
        if (unlikely(result < 0)) {
            pr_warn("[ftg340]: %s: sg_alloc_table_from_pages_segment failed\n", __func__);
            goto error;
        }

        result = dma_map_sg(dev, um->sgt.sgl, um->sgt.nents, DMA_TO_DEVICE);
        if (unlikely(result == 0)) {
            pr_warn("[ftg340]: %s: dma_map_sg failed\n", __func__);
            goto error;
        }

        if (Os->iommu)
            um->dmaHandle = sg_dma_address(um->sgt.sgl);

        dma_sync_sg_for_cpu(dev, um->sgt.sgl, um->sgt.nents, DMA_FROM_DEVICE);
    }

    um->type = UM_PAGE_MAP;
    um->pages = pages;

    return 0;

error:
#if gcdUSE_LINUX_SG_TABLE_API
    sg_free_table(&um->sgt);
#else
    kfree(um->sgt.sgl);
#endif

    free_memory(pages);

    return result;
}

static int
import_pfn_map(gckOS Os, struct device *dev, struct um_desc *um,
               unsigned long addr, size_t pfn_count)
{
    int i;
    struct vm_area_struct *vma;
    unsigned long *pfns;
    int *refs;
    struct page **pages = gcvNULL;
    int result = 0;
    size_t pageCount = 0;

    if (!current->mm)
        return -ENOTTY;

    down_read(&current_mm_mmap_sem);
    vma = find_vma(current->mm, addr);
#if !gcdUSING_PFN_FOLLOW && (LINUX_VERSION_CODE < KERNEL_VERSION(6, 5, 0))
    up_read(&current_mm_mmap_sem);
#endif

    if (!vma)
        return -ENOTTY;

    pfns = (unsigned long *)alloc_memory(pfn_count * sizeof(unsigned long));

    if (!pfns)
        return -ENOMEM;

    refs = (int *)alloc_memory(pfn_count * sizeof(int));

    if (!refs) {
        free_memory(pfns);
        return -ENOMEM;
    }

    pages = alloc_memory(pfn_count * sizeof(void *));
    if (!pages) {
        free_memory(pfns);
        free_memory(refs);
        return -ENOMEM;
    }

    for (i = 0; i < pfn_count; i++) {
#if gcdUSING_PFN_FOLLOW || (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0))
        int ret = 0;
        ret = follow_pfn(vma, addr, &pfns[i]);
        if (ret < 0) {
            up_read(&current_mm_mmap_sem);
            goto err;
        }
#else
        /* protect pfns[i] */
        spinlock_t  *ptl;
        pgd_t *pgd;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
        p4d_t *p4d;
# endif
        pud_t *pud;
        pmd_t *pmd;
        pte_t *pte;

        pgd = pgd_offset(current->mm, addr);
        if (pgd_none(*pgd) || pgd_bad(*pgd))
            goto err;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0)
        p4d = p4d_offset(pgd, addr);
        if (p4d_none(READ_ONCE(*p4d)))
            goto err;

        pud = pud_offset(p4d, addr);
# elif (defined(CONFIG_X86)) && LINUX_VERSION_CODE >= KERNEL_VERSION (4, 12, 0)
        pud = pud_offset((p4d_t *)pgd, addr);
# elif (defined(CONFIG_CPU_CSKYV2)) && LINUX_VERSION_CODE >= KERNEL_VERSION (4, 11, 0)
        pud = pud_offset((p4d_t *)pgd, addr);
# else
        pud = pud_offset(pgd, addr);
# endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(5, 9, 0) */
        if (pud_none(*pud) || pud_bad(*pud))
            goto err;

        pmd = pmd_offset(pud, addr);
        if (pmd_none(*pmd) || pmd_bad(*pmd))
            goto err;

        pte = pte_offset_map_lock(current->mm, pmd, addr, &ptl);

        if (!pte_present(*pte)) {
            pte_unmap_unlock(pte, ptl);
            goto err;
        }

        pfns[i] = pte_pfn(*pte);
        pte_unmap_unlock(pte, ptl);
#endif
        /* Advance to next. */
        addr += PAGE_SIZE;
    }
#if gcdUSING_PFN_FOLLOW || (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 5, 0))
    up_read(&current_mm_mmap_sem);
#endif

    for (i = 0; i < pfn_count; i++) {
        if (pfn_valid(pfns[i])) {
            struct page *page = pfn_to_page(pfns[i]);

            refs[i] = get_page_unless_zero(page);
            pages[i] = page;
            pageCount++;
        }
    }

    um->chunk_count = 1;
    for (i = 1; i < pfn_count; i++) {
        if (pfns[i] != pfns[i - 1] + 1)
            ++um->chunk_count;
    }

    if (gcmIS_SUCCESS(phy_is_from_reserved(Os, (gctPHYS_ADDR_T)pfns[0] << PAGE_SHIFT)))
        um->alloc_from_res = 1;

    if (pageCount == pfn_count && !um->alloc_from_res) {
#if gcdUSE_LINUX_SG_TABLE_API
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        size_t max_segment = SIZE_MAX;

        max_segment = dma_max_mapping_size(dev);
# endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 15, 0)
        result = sg_alloc_table_from_pages_segment(&um->sgt, pages, pfn_count, addr & ~PAGE_MASK,
                                                   pfn_count * PAGE_SIZE, max_segment, GFP_KERNEL | gcdNOWARN);
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 10, 0)
        result = PTR_ERR_OR_ZERO(__sg_alloc_table_from_pages(&um->sgt, pages, pfn_count, addr & ~PAGE_MASK,
                                       pfn_count * PAGE_SIZE, max_segment, gcvNULL, 0, GFP_KERNEL | gcdNOWARN));
# elif LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
        max_segment = SCATTERLIST_MAX_SEGMENT;
        result = __sg_alloc_table_from_pages(&um->sgt, pages, pfn_count, addr & ~PAGE_MASK,
                                             pfn_count * PAGE_SIZE, max_segment, GFP_KERNEL | gcdNOWARN);
# else
        result = sg_alloc_table_from_pages(&um->sgt, pages, pfn_count, addr & ~PAGE_MASK,
                                           pfn_count * PAGE_SIZE, GFP_KERNEL | gcdNOWARN);
# endif
#else /* gcdUSE_LINUX_SG_TABLE_API */
        result = alloc_sg_list_from_pages(&um->sgt.sgl, pages, pfn_count, addr & ~PAGE_MASK,
                                          pfn_count * PAGE_SIZE, &um->sgt.nents);

        um->sgt.orig_nents = um->sgt.nents;
#endif
        if (unlikely(result < 0)) {
            pr_warn("[ftg340]: %s: sg_alloc_table_from_pages_segment failed\n", __func__);
            goto err;
        }

        result = dma_map_sg(dev, um->sgt.sgl, um->sgt.nents, DMA_TO_DEVICE);

        if (unlikely(result == 0)) {
#if gcdUSE_LINUX_SG_TABLE_API
            sg_free_table(&um->sgt);
#else
            kfree(um->sgt.sgl);
#endif
            pr_warn("[ftg340]: %s: dma_map_sg failed\n", __func__);
            goto err;
        }
        if (Os->iommu)
            um->dmaHandle = sg_dma_address(um->sgt.sgl);
    }

    free_memory(pages);
    pages = gcvNULL;

    um->type = UM_PFN_MAP;
    um->pfns = pfns;
    um->refs = refs;
    return 0;

err:
    free_memory(pfns);
    free_memory(refs);
    free_memory(pages);

    return -ENOTTY;
}

static void
release_physical_map(gckOS Os, struct device *dev, struct um_desc *um)
{
    if (Os->iommu) {
        unsigned long pfn = um->physical >> PAGE_SHIFT;
        size_t size = um->size + um->offset;

        if (pfn_valid(pfn))
            dma_unmap_page(dev, um->dmaHandle, size, DMA_BIDIRECTIONAL);
        else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 9, 0)
            dma_unmap_resource(dev, um->dmaHandle, size, DMA_BIDIRECTIONAL, 0);
#else
            dma_unmap_page(dev, um->dmaHandle, size, DMA_BIDIRECTIONAL);
#endif

        um->dmaHandle = 0;
    }
}

static void
release_page_map(gckOS Os, struct device *dev, struct um_desc *um)
{
    int i;

    if (um->sgt.nents > 0) {
        dma_sync_sg_for_device(dev, um->sgt.sgl, um->sgt.nents, DMA_TO_DEVICE);

        dma_sync_sg_for_cpu(dev, um->sgt.sgl, um->sgt.nents, DMA_FROM_DEVICE);

        dma_unmap_sg(dev, um->sgt.sgl, um->sgt.nents, DMA_FROM_DEVICE);

        um->dmaHandle = 0;

#if gcdUSE_LINUX_SG_TABLE_API
        sg_free_table(&um->sgt);
#else
        kfree(um->sgt.sgl);
#endif
    }

    for (i = 0; i < um->pageCount; i++) {
        if (!PageReserved(um->pages[i]))
            SetPageDirty(um->pages[i]);

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 6, 0)
        unpin_user_page(um->pages[i]);
#else
        put_page(um->pages[i]);
#endif
    }

    free_memory(um->pages);
}

static void
release_pfn_map(gckOS Os, struct device *dev, struct um_desc *um)
{
    int i;

    if (um->sgt.nents > 0) {
        dma_unmap_sg(dev, um->sgt.sgl, um->sgt.nents, DMA_FROM_DEVICE);

#if gcdUSE_LINUX_SG_TABLE_API
        sg_free_table(&um->sgt);
#else
        kfree(um->sgt.sgl);
#endif
    }

    um->dmaHandle = 0;

    for (i = 0; i < um->pageCount; i++) {
        if (pfn_valid(um->pfns[i])) {
            struct page *page = pfn_to_page(um->pfns[i]);

            if (!PageReserved(page))
                SetPageDirty(page);

            if (um->refs[i])
                put_page(page);
        }
    }

    free_memory(um->pfns);
    free_memory(um->refs);
}

static gceSTATUS
_Import(gckOS Os, PLINUX_MDL Mdl, gctPOINTER Memory,
        gctPHYS_ADDR_T Physical, gctSIZE_T Size, struct um_desc *UserMemory)
{
    gceSTATUS status = gcvSTATUS_OK;
    unsigned long vm_flags = 0;
    struct vm_area_struct *vma = NULL;
    struct device *dev = (struct device *)Mdl->device;
    gctSIZE_T start, end, memory;
    int result = 0;
    gctBOOL mapped = gcvFALSE;

    gctSIZE_T extraPage;
    gctSIZE_T pageCount, i;

    gcmkHEADER_ARG("Os=%p Memory=%p Physical=0x%llx Size=%lu",
                   Os, Memory, Physical, Size);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Memory != gcvNULL || Physical != ~0ULL);
    gcmkVERIFY_ARGUMENT(Size > 0);

    memory = (Physical != gcvINVALID_PHYSICAL_ADDRESS) ?
             Physical : untagged_addr((gctSIZE_T)Memory);

    /* Get the number of required pages. */
    end = (memory + Size + PAGE_SIZE - 1) >> PAGE_SHIFT;
    start = memory >> PAGE_SHIFT;
    pageCount = end - start;

    /* Allocate extra page to avoid cache overflow */
    extraPage = (((memory + gcmALIGN(Size + 64, 64) + PAGE_SIZE - 1) >> PAGE_SHIFT) > end) ?
                1 : 0;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, _GC_OBJ_ZONE,
                   "%s(%d): pageCount: %d. extraPage: %d",
                   __func__, __LINE__, pageCount, extraPage);

    /* Overflow. */
    if ((memory + Size) < memory) {
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT);
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    memory = untagged_addr((gctSIZE_T)Memory);

    if (memory) {
        gctSIZE_T vaddr = memory;

        down_read(&current_mm_mmap_sem);
        vma = find_vma(current->mm, memory);
        up_read(&current_mm_mmap_sem);

        if (!vma)
            /* No such memory, or across vmas. */
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

#ifdef CONFIG_ARM
        /* coherent cache in case vivt or vipt-aliasing cache. */
        __cpuc_flush_user_range(memory, memory + Size, vma->vm_flags);
#endif

        vm_flags = vma->vm_flags;
        vaddr = vma->vm_end;

        down_read(&current_mm_mmap_sem);
        while (vaddr < memory + Size) {
            vma = find_vma(current->mm, vaddr);

            if (!vma) {
                /* No such memory. */
                up_read(&current_mm_mmap_sem);
                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            if ((vma->vm_flags & VM_PFNMAP) != (vm_flags & VM_PFNMAP)) {
                /*
                 * Can not support different map type:
                 * both PFN and PAGE detected.
                 */
                up_read(&current_mm_mmap_sem);
                gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            vaddr = vma->vm_end;
        }
        up_read(&current_mm_mmap_sem);
    }

    if (Physical != gcvINVALID_PHYSICAL_ADDRESS) {
        result = import_physical_map(Os, dev, UserMemory, Physical);
    } else {
        if (vm_flags & VM_PFNMAP)
            result = import_pfn_map(Os, dev, UserMemory, memory, pageCount);
        else
            result = import_page_map(Os, dev, UserMemory, memory, pageCount, Size, vm_flags);
    }

    if (result == -EINVAL)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    else if (result == -ENOMEM)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    else if (result < 0)
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);

    mapped = gcvTRUE;

    if (Os->device->platform->flagBits & gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS) {
        gctPHYS_ADDR_T addr;

        if (Physical != gcvINVALID_PHYSICAL_ADDRESS) {
            if (Physical > 0xFFFFFFFFu || Physical + Size > 0xFFFFFFFFu)
                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        } else if (vm_flags & VM_PFNMAP) {
            for (i = 0; i < pageCount; i++) {
                addr = UserMemory->pfns[i] << PAGE_SHIFT;
                if (addr > 0xFFFFFFFFu) {
                    free_memory(UserMemory->pfns);
                    UserMemory->pfns = gcvNULL;
                    free_memory(UserMemory->refs);
                    UserMemory->refs = gcvNULL;
                    gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        } else {
            for (i = 0; i < pageCount; i++) {
                addr = page_to_phys(UserMemory->pages[i]);
                if (addr > 0xFFFFFFFFu) {
                    free_memory(UserMemory->pages);
                    UserMemory->pages = gcvNULL;
                    gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
                }
            }
        }
    }

    UserMemory->vm_flags = vm_flags;
    UserMemory->user_vaddr = (unsigned long)Memory;
    UserMemory->size = Size;
    UserMemory->offset = (Physical != gcvINVALID_PHYSICAL_ADDRESS) ?
                         (Physical & ~PAGE_MASK) : (memory & ~PAGE_MASK);

    UserMemory->pageCount = pageCount;
    UserMemory->extraPage = extraPage;

    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (UserMemory && mapped) {
        switch (UserMemory->type) {
        case UM_PHYSICAL_MAP:
            release_physical_map(Os, dev, UserMemory);
            break;
        case UM_PAGE_MAP:
            release_page_map(Os, dev, UserMemory);
            break;
        case UM_PFN_MAP:
            release_pfn_map(Os, dev, UserMemory);
            break;
        }
    }

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_UserMemoryAttach(gckALLOCATOR Allocator, gcsATTACH_DESC_PTR Desc, PLINUX_MDL Mdl)
{
    gceSTATUS status;
    struct um_desc *userMemory = gcvNULL;

    gckOS os = Allocator->os;

    gcmkHEADER();

    /* Handle is meangless for this importer. */
    gcmkVERIFY_ARGUMENT(Desc != gcvNULL);

    gcmkONERROR(gckOS_Allocate(os, gcmSIZEOF(struct um_desc), (gctPOINTER *)&userMemory));

    gckOS_ZeroMemory(userMemory, gcmSIZEOF(struct um_desc));

    gcmkONERROR(_Import(os, Mdl, Desc->userMem.memory,
                        Desc->userMem.physical, Desc->userMem.size, userMemory));

    Mdl->priv = userMemory;
    Mdl->numPages = userMemory->pageCount + userMemory->extraPage;
    Mdl->contiguous = (userMemory->chunk_count == 1);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (userMemory != gcvNULL)
        gckOS_Free(os, (gctPOINTER)userMemory);

    gcmkFOOTER();
    return status;
}

static void
_UserMemoryFree(gckALLOCATOR Allocator, PLINUX_MDL Mdl)
{
    gckOS os = Allocator->os;
    struct um_desc *userMemory = Mdl->priv;
    struct device *dev = (struct device *)Mdl->device;

    gcmkHEADER();

    if (userMemory) {
        switch (userMemory->type) {
        case UM_PHYSICAL_MAP:
            release_physical_map(os, dev, userMemory);
            break;
        case UM_PAGE_MAP:
            release_page_map(os, dev, userMemory);
            break;
        case UM_PFN_MAP:
            release_pfn_map(os, dev, userMemory);
            break;
        }

        gcmkOS_SAFE_FREE(os, userMemory);
    }

    gcmkFOOTER_NO();
}

static gceSTATUS
_UserMemoryMapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctBOOL Cacheable)
{
    struct um_desc *userMemory = Mdl->priv;

    MdlMap->vmaAddr = (gctPOINTER)userMemory->user_vaddr;
    MdlMap->cacheable = gcvTRUE;

    return gcvSTATUS_OK;
}

static void
_UserMemoryUnmapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctUINT32 Size)
{
}

static gceSTATUS
_UserMemoryMapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                     gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *Logical)
{
    /* Kernel doesn't access video memory. */
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
_UserMemoryUnmapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctPOINTER Logical)
{
    /* Kernel doesn't access video memory. */
    return gcvSTATUS_NOT_SUPPORTED;
}

static gceSTATUS
_UserMemoryCache(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset,
                 gctPOINTER Logical, gctSIZE_T Bytes, gceCACHEOPERATION Operation)
{
    struct um_desc *um  = Mdl->priv;
    enum dma_data_direction dir;
    struct device *dev = (struct device *)Mdl->device;

    if (um->type == UM_PHYSICAL_MAP || um->alloc_from_res) {
        _MemoryBarrier();
        return gcvSTATUS_OK;
    }

    if (um->type == UM_PFN_MAP && um->sgt.nents <= 0) {
        _MemoryBarrier();
        return gcvSTATUS_OK;
    }

#ifdef CONFIG_ARM
    /* coherent cache in case vivt or vipt-aliasing cache. */
    __cpuc_flush_user_range(um->user_vaddr, um->user_vaddr + um->size, um->vm_flags);
#endif

    switch (Operation) {
    case gcvCACHE_CLEAN:
        dir = DMA_TO_DEVICE;
        dma_sync_sg_for_device(dev, um->sgt.sgl, um->sgt.nents, dir);
        break;
    case gcvCACHE_FLUSH:
        dir = DMA_TO_DEVICE;
        dma_sync_sg_for_device(dev, um->sgt.sgl, um->sgt.nents, dir);
        dir = DMA_FROM_DEVICE;
        dma_sync_sg_for_cpu(dev, um->sgt.sgl, um->sgt.nents, dir);
        break;
    case gcvCACHE_INVALIDATE:
        dir = DMA_FROM_DEVICE;
        dma_sync_sg_for_cpu(dev, um->sgt.sgl, um->sgt.nents, dir);
        break;
    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_UserMemoryPhysical(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                    unsigned long Offset, gctPHYS_ADDR_T *Physical)
{
    gckOS os = Allocator->os;
    struct um_desc *userMemory = Mdl->priv;
    unsigned long offset = Offset + userMemory->offset;
    gctUINT32 offsetInPage = offset & ~PAGE_MASK;
    gctUINT32 index = (gctUINT32)(offset / PAGE_SIZE);

    if (index >= userMemory->pageCount) {
        if (index < userMemory->pageCount + userMemory->extraPage) {
            if (os->iommu)
                *Physical = os->iommu->paddingPageDmaHandle;
            else
                *Physical = page_to_phys(os->paddingPage);
        } else {
            return gcvSTATUS_INVALID_ARGUMENT;
        }
    } else {
        if (os->iommu) {
            *Physical = userMemory->dmaHandle + (gctPHYS_ADDR_T)index * PAGE_SIZE;
        } else {
            switch (userMemory->type) {
            case UM_PHYSICAL_MAP:
                *Physical = userMemory->physical + (gctPHYS_ADDR_T)index * PAGE_SIZE;
                break;
            case UM_PAGE_MAP:
                *Physical = page_to_phys(userMemory->pages[index]);
                break;
            case UM_PFN_MAP:
                *Physical = (gctPHYS_ADDR_T)userMemory->pfns[index] << PAGE_SHIFT;
                break;
            }
        }
    }

    *Physical += offsetInPage;

    return gcvSTATUS_OK;
}

static void
_UserMemoryAllocatorDestructor(gcsALLOCATOR *Allocator)
{
    kfree(Allocator->privateData);

    kfree(Allocator);
}

/* User memory allocator (importer) operations. */
static gcsALLOCATOR_OPERATIONS UserMemoryAllocatorOperations = {
    .Attach      = _UserMemoryAttach,
    .Free        = _UserMemoryFree,
    .MapUser     = _UserMemoryMapUser,
    .UnmapUser   = _UserMemoryUnmapUser,
    .MapKernel   = _UserMemoryMapKernel,
    .UnmapKernel = _UserMemoryUnmapKernel,
    .Cache       = _UserMemoryCache,
    .Physical    = _UserMemoryPhysical,
};

/* Default allocator entry. */
gceSTATUS
_UserMemoryAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator)
{
    gceSTATUS status;
    gckALLOCATOR allocator;

    gcmkONERROR(gckALLOCATOR_Construct(Os, &UserMemoryAllocatorOperations, &allocator));

    allocator->destructor = _UserMemoryAllocatorDestructor;

    allocator->capability = gcvALLOC_FLAG_USERMEMORY
                          | gcvALLOC_FLAG_32BIT_VA
                          | gcvALLOC_FLAG_PRIOR_32BIT_VA;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    return status;
}
