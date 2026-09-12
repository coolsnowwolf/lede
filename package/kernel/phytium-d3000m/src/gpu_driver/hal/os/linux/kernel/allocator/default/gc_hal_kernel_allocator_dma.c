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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 19, 0)
# include <linux/dma-direct.h>
#endif
#include <linux/slab.h>
#include <linux/platform_device.h>
#if defined(CONFIG_X86)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
#  include <asm/set_memory.h>
# endif
#endif

#define _GC_OBJ_ZONE gcvZONE_OS

typedef struct _gcsDMA_PRIV *gcsDMA_PRIV_PTR;
typedef struct _gcsDMA_PRIV {
    atomic_t    usage;
} gcsDMA_PRIV;

struct mdl_dma_priv {
    gctPOINTER  kvaddr;
    dma_addr_t  dmaHandle;
};

/*
 * Debugfs support.
 */
static int gc_dma_usage_show(struct seq_file *m, void *data)
{
    gcsINFO_NODE *node = m->private;
    gckALLOCATOR Allocator = node->device;
    gcsDMA_PRIV_PTR priv = Allocator->privateData;
    long long usage = (long long)atomic_read(&priv->usage);

    seq_puts(m, "type        n pages        bytes\n");
    seq_printf(m, "normal   %10llu %12llu\n", usage, usage * PAGE_SIZE);

    return 0;
}

static gcsINFO InfoList[] = {
    { "dmausage", gc_dma_usage_show },
};

static void
_DebugfsInit(gckALLOCATOR Allocator, gckDEBUGFS_DIR Root)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_Init(&Allocator->debugfsDir, Root->root, "dma"));

    gcmkVERIFY_OK(gckDEBUGFS_DIR_CreateFiles(&Allocator->debugfsDir, InfoList,
                                             gcmCOUNTOF(InfoList), Allocator));
}

static void
_DebugfsCleanup(gckALLOCATOR Allocator)
{
    gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(&Allocator->debugfsDir,
                                             InfoList, gcmCOUNTOF(InfoList)));

    gckDEBUGFS_DIR_Deinit(&Allocator->debugfsDir);
}

static gceSTATUS
_DmaAlloc(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T NumPages, gctUINT32 Flags)
{
    gceSTATUS status;
    u32 gfp = GFP_KERNEL | gcdNOWARN;
    gcsDMA_PRIV_PTR allocatorPriv = (gcsDMA_PRIV_PTR)Allocator->privateData;
    struct mdl_dma_priv *mdlPriv = gcvNULL;
    struct device *dev = (struct device *)Mdl->device;
    gckOS os = Allocator->os;

    gcmkHEADER_ARG("Mdl=%p NumPages=0x%zx Flags=0x%x", Mdl, NumPages, Flags);

    gcmkONERROR(gckOS_Allocate(os, sizeof(struct mdl_dma_priv), (gctPOINTER *)&mdlPriv));
    mdlPriv->kvaddr = gcvNULL;

#if defined(CONFIG_ZONE_DMA32) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    if (Flags & gcvALLOC_FLAG_4GB_ADDR)
        gfp |= __GFP_DMA32;
#else
    if (Flags & gcvALLOC_FLAG_4GB_ADDR)
        gfp |= __GFP_DMA;
#endif

#if gcdENABLE_NONCACHE_COMMANDBUF
    if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
        mdlPriv->kvaddr = dma_alloc_coherent(dev, NumPages * PAGE_SIZE, &mdlPriv->dmaHandle, gfp);
    else
#endif
        mdlPriv->kvaddr =
#if defined CONFIG_MIPS || defined CONFIG_CPU_CSKYV2 || defined CONFIG_PPC ||                      \
    defined CONFIG_ARM64 || !gcdENABLE_BUFFERABLE_VIDEO_MEMORY
            dma_alloc_coherent(dev, NumPages * PAGE_SIZE, &mdlPriv->dmaHandle, gfp);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
            dma_alloc_wc(dev, NumPages * PAGE_SIZE, &mdlPriv->dmaHandle, gfp);
# else
            dma_alloc_writecombine(dev, NumPages * PAGE_SIZE, &mdlPriv->dmaHandle, gfp);
# endif
#endif

    if (mdlPriv->kvaddr == gcvNULL)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

#if defined(CONFIG_X86)
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if gcdENABLE_NONCACHE_COMMANDBUF
    if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
        if (set_memory_uc((unsigned long)(mdlPriv->kvaddr), NumPages) != 0)
            pr_warn("%s(%d): failed to set_memory_uc\n", __func__, __LINE__);
    else
#  endif
        if (set_memory_wc((unsigned long)mdlPriv->kvaddr, NumPages) != 0)
            pr_warn("%s(%d): failed to set_memory_wc\n", __func__, __LINE__);
# else
    if (set_memory_uc((unsigned long)mdlPriv->kvaddr, NumPages) != 0)
        pr_warn("%s(%d): failed to set_memory_uc\n", __func__, __LINE__);
# endif
#endif
    Mdl->priv = mdlPriv;

    Mdl->dmaHandle = mdlPriv->dmaHandle;

    /* Statistic. */
    atomic_add(NumPages, &allocatorPriv->usage);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (mdlPriv)
        gckOS_Free(os, mdlPriv);

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_DmaGetSGT(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *SGT)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 6, 0)
    struct page **pages = gcvNULL;
    struct page *page = gcvNULL;
    struct sg_table *sgt = NULL;
    struct mdl_dma_priv *mdlPriv = (struct mdl_dma_priv *)Mdl->priv;

    gceSTATUS status = gcvSTATUS_OK;
    gctSIZE_T offset = Offset & ~PAGE_MASK;  /* Offset to the first page */
    gctSIZE_T skipPages = Offset >> PAGE_SHIFT; /* skipped pages */
    gctSIZE_T numPages = (PAGE_ALIGN(Offset + Bytes) >> PAGE_SHIFT) - skipPages;
    gctSIZE_T i;

    gcmkASSERT(Offset + Bytes <= Mdl->numPages << PAGE_SHIFT);

    sgt = kmalloc(sizeof(*sgt), GFP_KERNEL | gcdNOWARN);
    if (!sgt)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    pages = kmalloc_array(numPages, sizeof(struct page *), GFP_KERNEL | gcdNOWARN);
    if (!pages)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    if (Allocator->os->iommu) {
        phys_addr_t phys;

        for (i = 0; i < numPages; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 1, 0)
            phys = iommu_iova_to_phys(Allocator->os->iommu->domain,
                                      mdlPriv->dmaHandle + (i + skipPages) * PAGE_SIZE);
# else
            gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
# endif
            pages[i] = pfn_to_page(phys >> PAGE_SHIFT);
        }
    } else {
#if !defined(phys_to_page)
        page = virt_to_page(mdlPriv->kvaddr);
# elif LINUX_VERSION_CODE < KERNEL_VERSION(3, 13, 0)
        page = phys_to_page(mdlPriv->dmaHandle);
# else
        page = phys_to_page(dma_to_phys(&Allocator->os->device->platform->device->dev,
                                        mdlPriv->dmaHandle));
# endif

        for (i = 0; i < numPages; ++i)
            pages[i] = nth_page(page, i + skipPages);
    }

    if (sg_alloc_table_from_pages(sgt, pages, numPages, offset, Bytes, GFP_KERNEL) < 0)
        gcmkONERROR(gcvSTATUS_GENERIC_IO);

    *SGT = (gctPOINTER)sgt;

OnError:
    kfree(pages);

    if (gcmIS_ERROR(status) && sgt)
        kfree(sgt);

    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

static void
_DmaFree(gckALLOCATOR Allocator, PLINUX_MDL Mdl)
{
    gckOS os = Allocator->os;
    struct mdl_dma_priv *mdlPriv = (struct mdl_dma_priv *)Mdl->priv;
    gcsDMA_PRIV_PTR allocatorPriv = (gcsDMA_PRIV_PTR)Allocator->privateData;
    struct device *dev = (struct device *)Mdl->device;

#if gcdENABLE_NONCACHE_COMMANDBUF
    if (Mdl->type == gcvVIDMEM_TYPE_COMMAND)
        dma_free_coherent(dev, Mdl->numPages * PAGE_SIZE, mdlPriv->kvaddr, mdlPriv->dmaHandle);
    else
#endif
#if defined CONFIG_MIPS || defined CONFIG_CPU_CSKYV2 ||     \
    defined CONFIG_PPC || defined CONFIG_ARM64 || !gcdENABLE_BUFFERABLE_VIDEO_MEMORY
        dma_free_coherent(dev, Mdl->numPages * PAGE_SIZE, mdlPriv->kvaddr, mdlPriv->dmaHandle);
#else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
        dma_free_wc(dev, Mdl->numPages * PAGE_SIZE, mdlPriv->kvaddr, mdlPriv->dmaHandle);
# else
        dma_free_writecombine(dev, Mdl->numPages * PAGE_SIZE, mdlPriv->kvaddr, mdlPriv->dmaHandle);
# endif
#endif

    gckOS_Free(os, mdlPriv);

    /* Statistic. */
    atomic_sub(Mdl->numPages, &allocatorPriv->usage);
}

static gceSTATUS
_DmaMmap(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctBOOL Cacheable,
         gctSIZE_T skipPages, gctSIZE_T numPages, struct vm_area_struct *vma)
{
    struct mdl_dma_priv *mdlPriv = (struct mdl_dma_priv *)Mdl->priv;
#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
#if !defined CONFIG_MIPS && !defined CONFIG_CPU_CSKYV2 && !defined CONFIG_PPC
    struct device *dev = (struct device *)Mdl->device;
# endif
#else
    struct device *dev = (struct device *)Mdl->device;
#endif
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p vma=%p", Allocator, Mdl, vma);

    gcmkASSERT(skipPages + numPages <= Mdl->numPages);

#if gcdENABLE_NONCACHE_COMMANDBUF
    if (Mdl->type == gcvVIDMEM_TYPE_COMMAND) {
    /* map kernel memory to user space.. */
        if (dma_mmap_coherent(dev, vma,
                              (gctINT8_PTR)mdlPriv->kvaddr + (skipPages << PAGE_SHIFT),
                              mdlPriv->dmaHandle + (skipPages << PAGE_SHIFT),
                              numPages << PAGE_SHIFT) < 0) {
            gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
                           "%s(%d): dma_mmap_attrs error",
                           __func__, __LINE__);

            gcmkFOOTER();
            return gcvSTATUS_OUT_OF_MEMORY;
        }
        gcmkFOOTER();
        return status;
    }
#endif

#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
    vma->vm_page_prot = pgprot_writecombine(vma->vm_page_prot);
#else
    vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);
#endif

#if gcdENABLE_BUFFERABLE_VIDEO_MEMORY
    /* map kernel memory to user space.. */
#if defined CONFIG_MIPS || defined CONFIG_CPU_CSKYV2 || defined CONFIG_PPC
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        (mdlPriv->dmaHandle >> PAGE_SHIFT) + skipPages,
                        numPages << PAGE_SHIFT,
                        pgprot_writecombine(vma->vm_page_prot)) < 0)
# else
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 6, 0)
    /* map kernel memory to user space.. */
    if (dma_mmap_wc(dev, vma,
                    (gctINT8_PTR)mdlPriv->kvaddr + (skipPages << PAGE_SHIFT),
                    mdlPriv->dmaHandle + (skipPages << PAGE_SHIFT),
                    numPages << PAGE_SHIFT) < 0)
#  else
    /* map kernel memory to user space.. */
    if (dma_mmap_writecombine(dev, vma,
                              (gctINT8_PTR)mdlPriv->kvaddr + (skipPages << PAGE_SHIFT),
                              mdlPriv->dmaHandle + (skipPages << PAGE_SHIFT),
                              numPages << PAGE_SHIFT) < 0)
#  endif
# endif
#else
    /* map kernel memory to user space.. */
    if (dma_mmap_coherent(dev, vma,
                          (gctINT8_PTR)mdlPriv->kvaddr + (skipPages << PAGE_SHIFT),
                          mdlPriv->dmaHandle + (skipPages << PAGE_SHIFT),
                          numPages << PAGE_SHIFT) < 0)
#endif
    {
        gcmkTRACE_ZONE(gcvLEVEL_WARNING, gcvZONE_OS,
                       "%s(%d): dma_mmap_attrs error",
                       __func__, __LINE__);

        status = gcvSTATUS_OUT_OF_MEMORY;
    }

    gcmkFOOTER();
    return status;
}

static void
_DmaUnmapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctUINT32 Size)
{
    if (unlikely(current->mm == gcvNULL))
        /* Do nothing if process is exiting. */
        return;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
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
_DmaMapUser(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctBOOL Cacheable)
{
    gctPOINTER userLogical = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Allocator=%p Mdl=%p Cacheable=%d", Allocator, Mdl, Cacheable);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 4, 0)
#if gcdANON_FILE_FOR_ALLOCATOR
    userLogical = (gctPOINTER)vm_mmap(Allocator->anon_file,
# else
    userLogical = (gctPOINTER)vm_mmap(gcvNULL,
# endif
                                      0L,
                                      Mdl->numPages * PAGE_SIZE,
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED | MAP_NORESERVE,
                                      0);
#else
    down_write(&current_mm_mmap_sem);
    userLogical = (gctPOINTER)do_mmap_pgoff(gcvNULL, 0L, Mdl->numPages * PAGE_SIZE,
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

        gcmkERR_BREAK(_DmaMmap(Allocator, Mdl, Cacheable, 0, Mdl->numPages, vma));

        MdlMap->vmaAddr = userLogical;
        MdlMap->cacheable = gcvFALSE;
        MdlMap->vma = vma;
    } while (gcvFALSE);

    up_write(&current_mm_mmap_sem);

OnError:
    if (gcmIS_ERROR(status) && userLogical) {
        MdlMap->vmaAddr = userLogical;
        _DmaUnmapUser(Allocator, Mdl, MdlMap, Mdl->numPages * PAGE_SIZE);
    }
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_DmaMapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
              gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER *Logical)
{
    struct mdl_dma_priv *mdlPriv = (struct mdl_dma_priv *)Mdl->priv;
    *Logical = (uint8_t *)mdlPriv->kvaddr + Offset;
    return gcvSTATUS_OK;
}

static gceSTATUS
_DmaUnmapKernel(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctPOINTER Logical)
{
    return gcvSTATUS_OK;
}

static gceSTATUS
_DmaCache(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset,
          gctPOINTER Logical, gctSIZE_T Bytes, gceCACHEOPERATION Operation)
{
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
_DmaPhysical(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
             unsigned long Offset, gctPHYS_ADDR_T *Physical)
{
    struct mdl_dma_priv *mdlPriv = (struct mdl_dma_priv *)Mdl->priv;

    *Physical = mdlPriv->dmaHandle + Offset;

    return gcvSTATUS_OK;
}

static void
_DmaAllocatorDestructor(gcsALLOCATOR *Allocator)
{
    _DebugfsCleanup(Allocator);

    kfree(Allocator->privateData);

    kfree(Allocator);
}

/* Default allocator operations. */
gcsALLOCATOR_OPERATIONS DmaAllocatorOperations = {
    .Alloc       = _DmaAlloc,
    .Free        = _DmaFree,
    .Mmap        = _DmaMmap,
    .MapUser     = _DmaMapUser,
    .UnmapUser   = _DmaUnmapUser,
    .MapKernel   = _DmaMapKernel,
    .UnmapKernel = _DmaUnmapKernel,
    .Cache       = _DmaCache,
    .Physical    = _DmaPhysical,
    .GetSGT      = _DmaGetSGT,
};

/* Default allocator entry. */
gceSTATUS
_DmaAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator)
{
    gceSTATUS status;
    gckALLOCATOR allocator = gcvNULL;
    gcsDMA_PRIV_PTR priv = gcvNULL;

    gcmkONERROR(gckALLOCATOR_Construct(Os, &DmaAllocatorOperations, &allocator));

    priv = kzalloc(gcmSIZEOF(gcsDMA_PRIV), GFP_KERNEL | gcdNOWARN);

    if (!priv)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    atomic_set(&priv->usage, 0);

    /* Register private data. */
    allocator->privateData = priv;
    allocator->destructor = _DmaAllocatorDestructor;

    _DebugfsInit(allocator, Parent);

    /* DMA allocator is only used for NonPaged memory. */
    allocator->capability = gcvALLOC_FLAG_CONTIGUOUS
                          | gcvALLOC_FLAG_DMABUF_EXPORTABLE
#if (defined(CONFIG_ZONE_DMA32) || defined(CONFIG_ZONE_DMA)) &&   \
    LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
                          | gcvALLOC_FLAG_4GB_ADDR
#endif
                          | gcvALLOC_FLAG_CPU_ACCESS
                          | gcvALLOC_FLAG_FROM_USER
                          | gcvALLOC_FLAG_32BIT_VA
                          | gcvALLOC_FLAG_PRIOR_32BIT_VA
#if gcdENABLE_VIDEO_MEMORY_MIRROR
                          | gcvALLOC_FLAG_WITH_MIRROR
#endif
                          ;

    if (Os->iommu) {
        /* Add some flags to ensure successful memory allocation */
        allocator->capability |= gcvALLOC_FLAG_NON_CONTIGUOUS;
        allocator->capability |= gcvALLOC_FLAG_1M_PAGES;
        allocator->capability |= gcvALLOC_FLAG_CACHEABLE;
        allocator->capability |= gcvALLOC_FLAG_MEMLIMIT;
    }

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    kfree(allocator);

    return status;
}
