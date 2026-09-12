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
#if defined(CONFIG_X86) && (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0))
# include <asm/set_memory.h>
#endif
#include "gc_hal_kernel_platform.h"

#define _GC_OBJ_ZONE gcvZONE_OS

#define gcdDISCRETE_PAGES 0

struct gfp_alloc {
    atomic_t low;
    atomic_t high;
};

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
struct sg_table {
    struct scatterlist       *sgl;
    unsigned int              nents;
    unsigned int              orig_nents;
};
#endif

struct gfp_mdl_priv {
    int contiguous;

    union {
        /* Pointer to a array of pages. */
        struct {
            struct page      *contiguousPages;
            dma_addr_t        dma_addr;
            int               exact;
        };

        struct {
            /* Pointer to a array of pointers to page. */
            struct page     **nonContiguousPages;

            struct page     **Pages1M;
            int               numPages1M;
            int              *isExact;
            struct sg_table   sgt;
        };
    };

    gcsPLATFORM *platform;
};

/*******************************************************************************
 ************************* GFP Allocator Debugfs *******************************
 *******************************************************************************/

static int gc_usage_show(struct seq_file *m, void *data)
{
    gcsINFO_NODE *node = m->private;
    gckALLOCATOR Allocator = node->device;
    struct gfp_alloc *priv = Allocator->privateData;
    long long low = (long long)atomic_read(&priv->low);
    long long high = (long long)atomic_read(&priv->high);

    seq_puts(m, "type        n pages        bytes\n");
    seq_printf(m, "normal   %10llu %12llu\n", low, low * PAGE_SIZE);
    seq_printf(m, "HighMem  %10llu %12llu\n", high, high * PAGE_SIZE);

    return 0;
}

static gcsINFO InfoList[] = {
    { "usage", gc_usage_show },
};

static void
_GFPAllocatorDebugfsInit(gckALLOCATOR Allocator, gckDEBUGFS_DIR Root)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_Init(&Allocator->debugfsDir, Root->root, "gfp"));

    gcmkVERIFY_OK(gckDEBUGFS_DIR_CreateFiles(&Allocator->debugfsDir, InfoList,
                                             gcmCOUNTOF(InfoList), Allocator));
}

static void
_GFPAllocatorDebugfsCleanup(gckALLOCATOR Allocator)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(&Allocator->debugfsDir,
                                             InfoList, gcmCOUNTOF(InfoList)));

    gckDEBUGFS_DIR_Deinit(&Allocator->debugfsDir);
}

static void
_NonContiguousFree(struct page **Pages, gctSIZE_T NumPages)
{
    gctSIZE_T i;

    gcmkHEADER_ARG("Pages=%p, NumPages=%zx", Pages, NumPages);

    gcmkASSERT(Pages != gcvNULL);

    for (i = 0; i < NumPages; i++)
        __free_page(Pages[i]);

    if (is_vmalloc_addr(Pages))
        vfree(Pages);
    else
        kfree(Pages);

    gcmkFOOTER_NO();
}

static gceSTATUS
_NonContiguousAlloc(struct gfp_mdl_priv *MdlPriv, gctSIZE_T NumPages, gctUINT32 Gfp)
{
    struct page **pages;
    struct page *p;
    gctSIZE_T i, size;

    gcmkHEADER_ARG("NumPages=%zx", NumPages);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (NumPages > totalram_pages()) {
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
    if (NumPages > totalram_pages) {
#else
    if (NumPages > num_physpages) {
#endif
        gcmkFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    size = NumPages * sizeof(struct page *);

    pages = kmalloc(size, GFP_KERNEL | gcdNOWARN);

    if (!pages) {
        pages = vmalloc(size);

        if (!pages) {
            gcmkFOOTER_NO();
            return gcvSTATUS_OUT_OF_MEMORY;
        }
    }

    for (i = 0; i < NumPages; i++) {
        p = alloc_page(Gfp);

        if (!p) {
            _NonContiguousFree(pages, i);
            gcmkFOOTER_NO();
            return gcvSTATUS_OUT_OF_MEMORY;
        }

#if gcdDISCRETE_PAGES
        if (i != 0) {
            if (page_to_pfn(pages[i - 1]) == page_to_pfn(p) - 1) {
                /* Replaced page. */
                struct page *l = p;

                /* Allocate a page which is not contiguous to previous one. */
                p = alloc_page(Gfp);

                /* Give replaced page back. */
                __free_page(l);

                if (!p) {
                    _NonContiguousFree(pages, i);
                    gcmkFOOTER_NO();
                    return gcvSTATUS_OUT_OF_MEMORY;
                }
            }
        }
#endif

        pages[i] = p;
    }

    MdlPriv->nonContiguousPages = pages;

    gcmkFOOTER_ARG("pages=%p", pages);
    return gcvSTATUS_OK;
}

static void
_NonContiguous1MPagesFree(struct gfp_mdl_priv *MdlPriv, gctUINT32 NumPages1M)
{
#if gcdENABLE_GPU_1M_PAGE
    gctINT i;

    if (MdlPriv->Pages1M && MdlPriv->isExact) {
        for (i = 0; i < NumPages1M && MdlPriv->Pages1M[i]; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
            if (MdlPriv->isExact[i] == gcvTRUE)
                free_pages_exact(page_address(MdlPriv->Pages1M[i]), gcd1M_PAGE_SIZE);
            else
#endif
                __free_pages(MdlPriv->Pages1M[i], get_order(gcd1M_PAGE_SIZE));
        }
    }

    if (MdlPriv->Pages1M) {
        if (is_vmalloc_addr(MdlPriv->Pages1M))
            vfree(MdlPriv->Pages1M);
        else
            kfree(MdlPriv->Pages1M);
        MdlPriv->Pages1M = gcvNULL;
    }

    if (MdlPriv->isExact) {
        if (is_vmalloc_addr(MdlPriv->isExact))
            vfree(MdlPriv->isExact);
        else
            kfree(MdlPriv->isExact);
    }

    if (MdlPriv->nonContiguousPages) {
        if (is_vmalloc_addr(MdlPriv->nonContiguousPages))
            vfree(MdlPriv->nonContiguousPages);
        else
            kfree(MdlPriv->nonContiguousPages);
        MdlPriv->nonContiguousPages = gcvNULL;
    }
#else
    return;
#endif
}

static gceSTATUS
_NonContiguous1MPagesAlloc(struct gfp_mdl_priv *MdlPriv, gctSIZE_T *NumPages, gctUINT32 Gfp)
{
#if gcdENABLE_GPU_1M_PAGE
    gceSTATUS status;
    size_t numPages1M, num, size;
    struct page **pages;
    struct page *page;
    void *addr = NULL;
    gctINT i, j;

    MdlPriv->numPages1M = 0;

    numPages1M = ((*NumPages << PAGE_SHIFT) + (gcd1M_PAGE_SIZE - 1)) >> gcd1M_PAGE_SHIFT;

    *NumPages = (numPages1M << gcd1M_PAGE_SHIFT) >> PAGE_SHIFT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 0, 0)
    if (*NumPages > totalram_pages())
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 32)
    if (*NumPages > totalram_pages)
#else
    if (*NumPages > num_physpages)
#endif
        return gcvSTATUS_INVALID_ARGUMENT;

    num = gcd1M_PAGE_SIZE / PAGE_SIZE;

    size = numPages1M * sizeof(struct page *);
    MdlPriv->Pages1M = kmalloc(size, GFP_KERNEL | gcdNOWARN);
    if (!MdlPriv->Pages1M) {
        MdlPriv->Pages1M = vmalloc(size);

        if (!MdlPriv->Pages1M)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    size = numPages1M * sizeof(int);
    MdlPriv->isExact = kmalloc(size, GFP_KERNEL | gcdNOWARN);
    if (!MdlPriv->isExact) {
        MdlPriv->isExact = vmalloc(size);
        if (!MdlPriv->isExact)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
    memset(MdlPriv->isExact, 0, size);

    size = *NumPages * sizeof(struct page *);
    pages = kmalloc(size, GFP_KERNEL | gcdNOWARN);
    if (!pages) {
        pages = vmalloc(size);
        if (!pages)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
    MdlPriv->nonContiguousPages = pages;

    for (i = 0; i < numPages1M; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        addr = alloc_pages_exact(gcd1M_PAGE_SIZE, (Gfp & ~__GFP_HIGHMEM) | __GFP_NORETRY);

        MdlPriv->Pages1M[i] = addr ? virt_to_page(addr) : gcvNULL;
        if (MdlPriv->Pages1M[i])
            MdlPriv->isExact[i] = gcvTRUE;
#endif

        if (MdlPriv->Pages1M[i] == gcvNULL) {
            int order = get_order(gcd1M_PAGE_SIZE);

#ifdef MAX_PAGE_ORDER
            if (order >= MAX_PAGE_ORDER)
#else
            if (order >= MAX_ORDER)
#endif
                gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

            MdlPriv->Pages1M[i] = alloc_pages(Gfp, order);
        }

        if (MdlPriv->Pages1M[i] == gcvNULL)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        MdlPriv->numPages1M += 1;

        for (j = 0; j < num; j++) {
            page = nth_page(MdlPriv->Pages1M[i], j);
            pages[i * num + j] = page;
        }
    }

    return gcvSTATUS_OK;
OnError:
    _NonContiguous1MPagesFree(MdlPriv, MdlPriv->numPages1M);

    return status;
#else
    return gcvSTATUS_OK;
#endif
}

/****************************************************************************
 *********************** GFP Allocator **************************************
 ****************************************************************************/

static gceSTATUS
_GFPAlloc(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T NumPages, gctUINT32 Flags)
{
    gceSTATUS status;
    gctSIZE_T i = 0;
    gctBOOL contiguous = Flags & gcvALLOC_FLAG_CONTIGUOUS;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 4, 0) && LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
    u32 normal_gfp = __GFP_HIGH | __GFP_ATOMIC | __GFP_NORETRY | gcdNOWARN;
#else
    u32 normal_gfp = __GFP_HIGH | __GFP_NORETRY | gcdNOWARN;
#endif
    u32 gfp = (contiguous ? normal_gfp : GFP_KERNEL) | __GFP_HIGHMEM | gcdNOWARN;

    struct gfp_alloc *priv = (struct gfp_alloc *)Allocator->privateData;
    struct device *dev = (struct device *)Mdl->device;
    struct gfp_mdl_priv *mdlPriv = gcvNULL;
    int result;
    int low  = 0;
    int high = 0;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p NumPages=%zu Flags=0x%x",
                   Allocator, Mdl, NumPages, Flags);

#ifdef gcdSYS_FREE_MEMORY_LIMIT
    if ((Flags & gcvALLOC_FLAG_MEMLIMIT) && !contiguous) {
        long freeram = 0;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 14, 0)
        freeram = global_zone_page_state(NR_FREE_PAGES);
#ifdef CONFIG_CMA
        freeram -= global_zone_page_state(NR_FREE_CMA_PAGES);
#  endif
# else
        struct sysinfo temsysinfo;

        si_meminfo(&temsysinfo);
        freeram = temsysinfo.freeram;
# endif
        if (freeram < NumPages || ((freeram - NumPages) < gcdSYS_FREE_MEMORY_LIMIT))
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }
#endif

    mdlPriv = kzalloc(sizeof(*mdlPriv), GFP_KERNEL | __GFP_NORETRY);

    if (!mdlPriv)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

#if defined(CONFIG_ZONE_DMA32) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    if ((Flags & gcvALLOC_FLAG_4GB_ADDR) ||
        ((Allocator->os->device->platform->flagBits & gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS) &&
        !(Flags & gcvALLOC_FLAG_WITH_MIRROR))) {
        /* remove __GFP_HIGHMEM bit, add __GFP_DMA32 bit */
        gfp &= ~__GFP_HIGHMEM;
        gfp |= __GFP_DMA32;
    }
#else
    if (Flags & gcvALLOC_FLAG_4GB_ADDR ||
        ((Allocator->os->device->platform->flagBits & gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS) &&
        !(Flags & gcvALLOC_FLAG_WITH_MIRROR))) {
        /* remove __GFP_HIGHMEM bit, add __GFP_DMA bit */
        gfp &= ~__GFP_HIGHMEM;
        gfp |= __GFP_DMA;
    }

#endif

    if ((Flags & gcvALLOC_FLAG_NON_CONTIGUOUS) && (Flags & gcvALLOC_FLAG_1M_PAGES))
        Mdl->pageUnit1M = gcvTRUE;
    else
        Mdl->pageUnit1M = gcvFALSE;

    if (contiguous) {
        size_t bytes = NumPages << PAGE_SHIFT;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        void *addr = NULL;

        addr = alloc_pages_exact(bytes, (gfp & ~__GFP_HIGHMEM) | __GFP_NORETRY);

        mdlPriv->contiguousPages = addr ? virt_to_page(addr) : gcvNULL;

        if (mdlPriv->contiguousPages)
            mdlPriv->exact = gcvTRUE;
#endif

        if (mdlPriv->contiguousPages == gcvNULL) {
            int order = get_order(bytes);

#ifdef MAX_PAGE_ORDER
            if (order >= MAX_PAGE_ORDER) {
#else
            if (order >= MAX_ORDER) {
#endif
                status = gcvSTATUS_OUT_OF_MEMORY;
                goto OnError;
            }

            mdlPriv->contiguousPages = alloc_pages(gfp, order);
        }

        if (mdlPriv->contiguousPages == gcvNULL)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        mdlPriv->dma_addr = dma_map_page(dev, mdlPriv->contiguousPages,
                                         0, NumPages * PAGE_SIZE, DMA_BIDIRECTIONAL);

        if (dma_mapping_error(dev, mdlPriv->dma_addr)) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
            if (mdlPriv->exact)
                free_pages_exact(page_address(mdlPriv->contiguousPages), bytes);
            else
#endif
                __free_pages(mdlPriv->contiguousPages, get_order(bytes));

            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

#if defined(CONFIG_X86)
        if (!PageHighMem(mdlPriv->contiguousPages)) {
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if gcdENABLE_NONCACHE_COMMANDBUF
            if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
                if (set_memory_uc((unsigned long)page_address(mdlPriv->contiguousPages), NumPages) != 0)
                    pr_warn("%s(%d): failed to set_memory_uc\n", __func__, __LINE__);
            else
#  endif
                if (set_memory_wc((unsigned long)page_address(mdlPriv->contiguousPages), NumPages) != 0)
                    pr_warn("%s(%d): failed to set_memory_wc\n", __func__, __LINE__);
# else
            if (set_memory_uc((unsigned long)page_address(mdlPriv->contiguousPages), NumPages) != 0)
                pr_warn("%s(%d): failed to set_memory_uc\n", __func__, __LINE__);
# endif
        }
#endif
    } else {
        if (Mdl->pageUnit1M) {
            gcmkONERROR(_NonContiguous1MPagesAlloc(mdlPriv, &NumPages, gfp));
        } else {
            if ((Flags & gcvALLOC_FLAG_4GB_ADDR) ||
                ((Allocator->os->device->platform->flagBits & gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS) &&
                !(Flags & gcvALLOC_FLAG_WITH_MIRROR))) {

                gcmkONERROR(_NonContiguousAlloc(mdlPriv, NumPages, gfp));
            } else {
                status = _NonContiguousAlloc(mdlPriv, NumPages, normal_gfp);

                if (gcmIS_ERROR(status))
                    gcmkONERROR(_NonContiguousAlloc(mdlPriv, NumPages, gfp));
            }
        }

#if gcdUSE_LINUX_SG_TABLE_API
        result = sg_alloc_table_from_pages(&mdlPriv->sgt, mdlPriv->nonContiguousPages,
                                           NumPages, 0, NumPages << PAGE_SHIFT, normal_gfp);

#else
        result = alloc_sg_list_from_pages(&mdlPriv->sgt.sgl, mdlPriv->nonContiguousPages,
                                          NumPages, 0, NumPages << PAGE_SHIFT, &mdlPriv->sgt.nents);

        mdlPriv->sgt.orig_nents = mdlPriv->sgt.nents;
#endif
        if (result < 0) {
            if (Mdl->pageUnit1M)
                _NonContiguous1MPagesFree(mdlPriv, mdlPriv->numPages1M);
            else
                _NonContiguousFree(mdlPriv->nonContiguousPages, NumPages);

            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        result = dma_map_sg(dev, mdlPriv->sgt.sgl, mdlPriv->sgt.nents, DMA_BIDIRECTIONAL);

        if (result == 0) {
            if (Mdl->pageUnit1M)
                _NonContiguous1MPagesFree(mdlPriv, mdlPriv->numPages1M);
            else
                _NonContiguousFree(mdlPriv->nonContiguousPages, NumPages);

#if gcdUSE_LINUX_SG_TABLE_API
            sg_free_table(&mdlPriv->sgt);
#else
            kfree(mdlPriv->sgt.sgl);
#endif
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

#if defined(CONFIG_X86)
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if gcdENABLE_NONCACHE_COMMANDBUF
        if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
            if (set_pages_array_uc(mdlPriv->nonContiguousPages, NumPages))
                pr_warn("%s(%d): failed to set_pages_array_uc\n", __func__, __LINE__);
        else
#  endif
            if (set_pages_array_wc(mdlPriv->nonContiguousPages, NumPages))
                pr_warn("%s(%d): failed to set_pages_array_wc\n", __func__, __LINE__);
# else
        if (set_pages_array_uc(mdlPriv->nonContiguousPages, NumPages))
            pr_warn("%s(%d): failed to set_pages_array_uc\n", __func__, __LINE__);
# endif
#endif
    }

    for (i = 0; i < NumPages; i++) {
        struct page *page;

        if (contiguous)
            page = nth_page(mdlPriv->contiguousPages, i);
        else
            page = mdlPriv->nonContiguousPages[i];

        SetPageReserved(page);

        if (PageHighMem(page))
            high++;
        else
            low++;
    }

    mdlPriv->platform = Allocator->os->device->platform;
    mdlPriv->contiguous = contiguous;
    atomic_add(low, &priv->low);
    atomic_add(high, &priv->high);

    Mdl->priv = mdlPriv;
    Mdl->numPages = NumPages;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    kfree(mdlPriv);

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_GFPGetSGT(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *SGT)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
    struct page **pages = gcvNULL;
    struct sg_table *sgt = NULL;
    struct gfp_mdl_priv *mdlPriv = (struct gfp_mdl_priv *)Mdl->priv;

    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T offset = Offset & ~PAGE_MASK;  /* Offset to the first page */
    gctSIZE_T skipPages = Offset >> PAGE_SHIFT; /* skipped pages */
    gctSIZE_T numPages = (PAGE_ALIGN(Offset + Bytes) >> PAGE_SHIFT) - skipPages;
    gctSIZE_T i;

    gcmkASSERT(Offset + Bytes <= Mdl->numPages << PAGE_SHIFT);

    if (mdlPriv->contiguous) {
        pages = kmalloc_array(numPages, sizeof(struct page *), GFP_KERNEL | gcdNOWARN);
        if (!pages)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        for (i = 0; i < numPages; ++i)
            pages[i] = nth_page(mdlPriv->contiguousPages, i + skipPages);
    } else {
        pages = &mdlPriv->nonContiguousPages[skipPages];
    }

    sgt = kmalloc(sizeof(*sgt), GFP_KERNEL | gcdNOWARN);
    if (!sgt)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    if (sg_alloc_table_from_pages(sgt, pages, numPages, offset, Bytes, GFP_KERNEL) < 0)
        gcmkONERROR(gcvSTATUS_GENERIC_IO);

    *SGT = (gctPOINTER)sgt;

OnError:
    if (mdlPriv->contiguous && pages)
        kfree(pages);

    if (gcmIS_ERROR(status) && sgt)
        kfree(sgt);

    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

static void
_GFPFree(gckALLOCATOR Allocator, PLINUX_MDL Mdl)
{
    gctSIZE_T i;
    struct page *page;
    struct gfp_alloc *priv = (struct gfp_alloc *)Allocator->privateData;
    struct gfp_mdl_priv *mdlPriv = Mdl->priv;
    struct device *dev = (struct device *)Mdl->device;
    int low = 0;
    int high = 0;

    if (mdlPriv->contiguous) {
        dma_unmap_page(dev, mdlPriv->dma_addr,
                       Mdl->numPages << PAGE_SHIFT, DMA_FROM_DEVICE);
    } else {
        dma_unmap_sg(dev, mdlPriv->sgt.sgl,
                     mdlPriv->sgt.nents, DMA_FROM_DEVICE);

#if gcdUSE_LINUX_SG_TABLE_API
        sg_free_table(&mdlPriv->sgt);
#else
        kfree(mdlPriv->sgt.sgl);
#endif
    }

    for (i = 0; i < Mdl->numPages; i++) {
        if (mdlPriv->contiguous)
            page = nth_page(mdlPriv->contiguousPages, i);
        else
            page = mdlPriv->nonContiguousPages[i];

        ClearPageReserved(page);

        if (PageHighMem(page))
            high++;
        else
            low++;
    }

    atomic_sub(low, &priv->low);
    atomic_sub(high, &priv->high);

    if (mdlPriv->contiguous) {
#if defined(CONFIG_X86)
        if (!PageHighMem(mdlPriv->contiguousPages))
            set_memory_wb((unsigned long)page_address(mdlPriv->contiguousPages), Mdl->numPages);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        if (mdlPriv->exact == gcvTRUE)
            free_pages_exact(page_address(mdlPriv->contiguousPages), Mdl->numPages * PAGE_SIZE);
        else
#endif
            __free_pages(mdlPriv->contiguousPages, get_order(Mdl->numPages * PAGE_SIZE));
    } else {
#if defined(CONFIG_X86)
        set_pages_array_wb(mdlPriv->nonContiguousPages, Mdl->numPages);
#endif

        if (Mdl->pageUnit1M)
            _NonContiguous1MPagesFree(mdlPriv, mdlPriv->numPages1M);
        else
            _NonContiguousFree(mdlPriv->nonContiguousPages, Mdl->numPages);
    }

    kfree(Mdl->priv);
}

static gceSTATUS
_GFPMmap(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctBOOL Cacheable,
         gctSIZE_T skipPages, gctSIZE_T numPages, struct vm_area_struct *vma)
{
    struct gfp_mdl_priv *mdlPriv = (struct gfp_mdl_priv *)Mdl->priv;
    gcsPLATFORM *platform = mdlPriv->platform;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p vma=%p", Allocator, Mdl, vma);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(6, 3, 0)) || \
    ((LINUX_VERSION_CODE >= KERNEL_VERSION(6, 1, 26)) && defined(CONFIG_ANDROID))
    vm_flags_set(vma, gcdVM_FLAGS);
#else
    vma->vm_flags |= gcdVM_FLAGS;
#endif

    if (Cacheable == gcvFALSE) {
        /* Make this mapping non-cached. */
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if gcdENABLE_NONCACHE_COMMANDBUF
        if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
            vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
        else
# endif
            vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#else
        vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif
    }
#if defined(CONFIG_X86)
    else {
        if (mdlPriv->contiguous)
            set_memory_wb((unsigned long)page_address(mdlPriv->contiguousPages) + (skipPages << PAGE_SHIFT), numPages);
        else
            set_pages_array_wb(mdlPriv->nonContiguousPages + skipPages, numPages);
    }
#endif

    if (platform && platform->ops->adjustProt)
        platform->ops->adjustProt(vma);

    gcmkASSERT(skipPages + numPages <= Mdl->numPages);

    /* Now map all the vmalloc pages to this user address. */
    if (mdlPriv->contiguous) {
        /* map kernel memory to user space.. */
        if (remap_pfn_range(vma, vma->vm_start,
                            page_to_pfn(mdlPriv->contiguousPages) + skipPages,
                            numPages << PAGE_SHIFT,
                            vma->vm_page_prot) < 0) {
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_OS,
                           "%s(%d): remap_pfn_range error.",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }
    } else {
        gctSIZE_T i;
        unsigned long start = vma->vm_start;

        for (i = 0; i < numPages; ++i) {
            unsigned long pfn = page_to_pfn(mdlPriv->nonContiguousPages[i + skipPages]);

            if (remap_pfn_range(vma, start, pfn, PAGE_SIZE, vma->vm_page_prot) < 0) {
                gcmkTRACE(gcvLEVEL_ERROR,
                          "%s(%d): remap_pfn_range error.",
                          __func__, __LINE__);

                gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }

            start += PAGE_SIZE;
        }
    }

OnError:
    gcmkFOOTER();
    return status;
}

static void
_GFPUnmapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctUINT32 Size)
{
    MdlMap->cacheable = gcvFALSE;

    if (unlikely(current->mm == gcvNULL))
        /* Do nothing if process is exiting. */
        return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 5, 0)
    if (vm_munmap((unsigned long)MdlMap->vmaAddr, Size) < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
                       "%s(%d): vm_munmap failed",
                       __func__, __LINE__);
    }
#else
    down_write(&current_mm_mmap_sem);
    if (do_munmap(current->mm, (unsigned long)MdlMap->vmaAddr, Size) < 0) {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
                       "%s(%d): do_munmap failed",
                       __func__, __LINE__);
    }
    up_write(&current_mm_mmap_sem);
#endif

    MdlMap->vma = NULL;
    MdlMap->vmaAddr = NULL;
}

static gceSTATUS
_GFPMapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctBOOL Cacheable)
{
    gctPOINTER userLogical = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p Cacheable=%d", Allocator, Mdl, Cacheable);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
#if gcdANON_FILE_FOR_ALLOCATOR
    userLogical = (gctPOINTER)vm_mmap(Allocator->anon_file,
# else
    userLogical = (gctPOINTER)vm_mmap(NULL,
# endif
                                      0L,
                                      Mdl->numPages * PAGE_SIZE, PROT_READ | PROT_WRITE,
                                      MAP_SHARED | MAP_NORESERVE,
                                      0);
#else
    down_write(&current_mm_mmap_sem);
    userLogical = (gctPOINTER)do_mmap_pgoff(NULL, 0L,
                                            Mdl->numPages * PAGE_SIZE,
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

        gcmkERR_BREAK(_GFPMmap(Allocator, Mdl, Cacheable, 0, Mdl->numPages, vma));
        MdlMap->vma = vma;
    } while (gcvFALSE);

    up_write(&current_mm_mmap_sem);

    if (gcmIS_SUCCESS(status)) {
        MdlMap->vmaAddr = userLogical;
        MdlMap->cacheable = Cacheable;
    }

OnError:
    if (gcmIS_ERROR(status) && userLogical) {
        MdlMap->vmaAddr = userLogical;
        _GFPUnmapUser(Allocator, Mdl, MdlMap, Mdl->numPages * PAGE_SIZE);
    }
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_GFPMapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
              gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *Logical)
{
    void *addr = 0;
    gctSIZE_T numPages = Mdl->numPages;
    struct gfp_mdl_priv *mdlPriv = Mdl->priv;
    unsigned long pgoff = (Offset >> PAGE_SHIFT);
    struct page **pages;
    gctBOOL free = gcvFALSE;
    pgprot_t pgprot;

    if (Offset + Bytes > (numPages << PAGE_SHIFT))
        return gcvSTATUS_INVALID_ARGUMENT;

    numPages = ((Offset & ~PAGE_MASK) + Bytes + (PAGE_SIZE - 1)) >> PAGE_SHIFT;

    if (mdlPriv->contiguous) {
        gctSIZE_T i;

        pages = kmalloc_array(numPages, sizeof(struct page *), GFP_KERNEL | gcdNOWARN);

        if (!pages)
            return gcvSTATUS_OUT_OF_MEMORY;

        for (i = 0; i < numPages; i++)
            pages[i] = nth_page(mdlPriv->contiguousPages, i + pgoff);

        free = gcvTRUE;
    } else {
        pages = &mdlPriv->nonContiguousPages[pgoff];
    }

    /* ioremap() can't work on system memory since 2.6.38. */
    if (Mdl->cacheable) {
        pgprot = PAGE_KERNEL;
    } else {
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if gcdENABLE_NONCACHE_COMMANDBUF
        if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
            pgprot = pgprot_noncached(PAGE_KERNEL);
        else
# endif
            pgprot = pgprot_writecombine(PAGE_KERNEL);
#else
        pgprot = pgprot_noncached(PAGE_KERNEL);
#endif
    }

    addr = vmap(pages, numPages, 0, pgprot);

    if (free)
        kfree(pages);

    if (addr) {
        /* Append offset in page. */
        *Logical = (uint8_t *)addr + (Offset & ~PAGE_MASK);
        return gcvSTATUS_OK;
    } else {
        return gcvSTATUS_OUT_OF_MEMORY;
    }
}

static gceSTATUS
_GFPUnmapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctPOINTER Logical)
{
    vunmap((void *)((uintptr_t)Logical & PAGE_MASK));

    return gcvSTATUS_OK;
}

static gceSTATUS
_GFPCache(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset,
          gctPOINTER Logical, gctSIZE_T Bytes, gceCACHEOPERATION Operation)
{
    struct gfp_mdl_priv *mdlPriv = Mdl->priv;
    struct device *dev = (struct device *)Mdl->device;
    enum dma_data_direction dir;
    dma_addr_t dma_addr = (mdlPriv->dma_addr + Offset) & PAGE_MASK;
    gctSIZE_T bytes = (mdlPriv->dma_addr + Offset + Bytes) - dma_addr;
    gctINT numPages = GetPageCount(bytes, 0);

    switch (Operation) {
    case gcvCACHE_CLEAN:
        dir = DMA_TO_DEVICE;

        if (mdlPriv->contiguous) {
            dma_sync_single_for_device(dev, dma_addr,
                                       numPages << PAGE_SHIFT, dir);
        } else {
            dma_sync_sg_for_device(dev, mdlPriv->sgt.sgl,
                                   mdlPriv->sgt.nents, dir);
        }

        break;
    case gcvCACHE_FLUSH:
        dir = DMA_TO_DEVICE;

        if (mdlPriv->contiguous) {
            dma_sync_single_for_device(dev, dma_addr,
                                       numPages << PAGE_SHIFT, dir);
        } else {
            dma_sync_sg_for_device(dev, mdlPriv->sgt.sgl,
                                   mdlPriv->sgt.nents, dir);
        }

        dir = DMA_FROM_DEVICE;

        if (mdlPriv->contiguous) {
            dma_sync_single_for_cpu(dev, dma_addr,
                                    numPages << PAGE_SHIFT, dir);
        } else {
            dma_sync_sg_for_cpu(dev, mdlPriv->sgt.sgl,
                                mdlPriv->sgt.nents, dir);
        }

        break;
    case gcvCACHE_INVALIDATE:
        dir = DMA_FROM_DEVICE;

        if (mdlPriv->contiguous) {
            dma_sync_single_for_cpu(dev, dma_addr,
                                    numPages << PAGE_SHIFT, dir);
        } else {
            dma_sync_sg_for_cpu(dev, mdlPriv->sgt.sgl,
                                mdlPriv->sgt.nents, dir);
        }

        break;
    default:
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_GFPPhysical(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
             unsigned long Offset, gctPHYS_ADDR_T *Physical)
{
    struct gfp_mdl_priv *mdlPriv = Mdl->priv;
    gctUINT32 offsetInPage = Offset & ~PAGE_MASK;
    gctUINT32 index = (gctUINT32)(Offset / PAGE_SIZE);

    if (mdlPriv->contiguous)
        *Physical = page_to_phys(nth_page(mdlPriv->contiguousPages, index));
    else
        *Physical = page_to_phys(mdlPriv->nonContiguousPages[index]);

    *Physical += offsetInPage;

    return gcvSTATUS_OK;
}

static void
_GFPAllocatorDestructor(gcsALLOCATOR *Allocator)
{
    _GFPAllocatorDebugfsCleanup(Allocator);

    kfree(Allocator->privateData);

    kfree(Allocator);
}

/* GFP allocator operations. */
static gcsALLOCATOR_OPERATIONS GFPAllocatorOperations = {
    .Alloc       = _GFPAlloc,
    .Free        = _GFPFree,
    .Mmap        = _GFPMmap,
    .MapUser     = _GFPMapUser,
    .UnmapUser   = _GFPUnmapUser,
    .MapKernel   = _GFPMapKernel,
    .UnmapKernel = _GFPUnmapKernel,
    .Cache       = _GFPCache,
    .Physical    = _GFPPhysical,
    .GetSGT      = _GFPGetSGT,
};

/* GFP allocator entry. */
gceSTATUS
_GFPAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator)
{
    gceSTATUS status;
    gckALLOCATOR allocator = gcvNULL;
    struct gfp_alloc *priv = gcvNULL;

    if (Os->iommu)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    gcmkONERROR(gckALLOCATOR_Construct(Os, &GFPAllocatorOperations, &allocator));

    priv = kzalloc(sizeof(*priv), GFP_KERNEL | gcdNOWARN);

    if (!priv)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    atomic_set(&priv->low, 0);
    atomic_set(&priv->high, 0);

    /* Register private data. */
    allocator->privateData = priv;
    allocator->destructor = _GFPAllocatorDestructor;

    _GFPAllocatorDebugfsInit(allocator, Parent);

    allocator->capability = gcvALLOC_FLAG_CONTIGUOUS
                          | gcvALLOC_FLAG_NON_CONTIGUOUS
                          | gcvALLOC_FLAG_CACHEABLE
                          | gcvALLOC_FLAG_MEMLIMIT
                          | gcvALLOC_FLAG_ALLOC_ON_FAULT
                          | gcvALLOC_FLAG_DMABUF_EXPORTABLE
#if (defined(CONFIG_ZONE_DMA32) || defined(CONFIG_ZONE_DMA)) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
                          | gcvALLOC_FLAG_4GB_ADDR
#endif
                          | gcvALLOC_FLAG_1M_PAGES
                          | gcvALLOC_FLAG_CPU_ACCESS
                          | gcvALLOC_FLAG_FROM_USER
#if gcdENABLE_VIDEO_MEMORY_MIRROR
                          | gcvALLOC_FLAG_WITH_MIRROR
#endif
                          | gcvALLOC_FLAG_32BIT_VA
                          | gcvALLOC_FLAG_PRIOR_32BIT_VA
                          ;

#if defined(gcdEMULATE_SECURE_ALLOCATOR)
    allocator->capability |= gcvALLOC_FLAG_SECURITY;
#endif

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    kfree(allocator);
    return status;
}
