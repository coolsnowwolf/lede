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
#include "gc_hal_kernel_device.h"

#include <linux/iommu.h>
#include <linux/platform_device.h>

#define _GC_OBJ_ZONE gcvZONE_OS

void
gckIOMMU_Destory(IN gckOS Os, IN gckIOMMU Iommu)
{
    gcmkHEADER_ARG("Os=%p Iommu=%p", Os, Iommu);

    if (Iommu) {
        if (Iommu->paddingPageDmaHandle) {
            dma_unmap_page(Iommu->device, Iommu->paddingPageDmaHandle,
                           PAGE_SIZE, DMA_FROM_DEVICE);
        }

        gcmkOS_SAFE_FREE(Os, Iommu);
    }

    gcmkFOOTER_NO();
}

gceSTATUS
gckIOMMU_Construct(IN gckOS Os, OUT gckIOMMU *Iommu)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckIOMMU iommu = gcvNULL;
    struct device *dev;
    dma_addr_t dmaHandle;
    gctUINT64 phys;
    struct iommu_domain *domain;
    gctUINT32 gfp = GFP_KERNEL;

    gcmkHEADER_ARG("Os=%p", Os);

    dev = &Os->device->platform->device->dev;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 2, 0)
    domain = iommu_get_domain_for_dev(dev);
#else
    domain = gcvNULL;
#endif

    if (domain) {
        struct page *page;

        page = alloc_page(gfp);
        if (!page)
            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

        phys = page_to_phys(page);

        dmaHandle = dma_map_page(dev, page, 0, PAGE_SIZE, DMA_TO_DEVICE);

        if (dmaHandle)
            dma_unmap_page(dev, dmaHandle, PAGE_SIZE, DMA_FROM_DEVICE);

        __free_page(page);

        /* Iommu bypass */
        if (phys == dmaHandle) {
            *Iommu = gcvNULL;

            gcmkFOOTER();
            return status;
        }
    } else {
        /* Don't enable iommu */
        *Iommu = gcvNULL;

        gcmkFOOTER();
        return status;
    }

    gcmkONERROR(gckOS_Allocate(Os, gcmSIZEOF(gcsIOMMU), (gctPOINTER *)&iommu));

    gckOS_ZeroMemory(iommu, gcmSIZEOF(gcsIOMMU));

    if (Os->paddingPage)
        iommu->paddingPageDmaHandle = dma_map_page(dev, Os->paddingPage, 0, PAGE_SIZE, DMA_TO_DEVICE);

    iommu->domain = domain;
    iommu->device = dev;

    *Iommu = iommu;

    gcmkPRINT("[ftg340]: Enable IOMMU\n");
OnError:
    if (gcmIS_ERROR(status))
        gckIOMMU_Destory(Os, iommu);

    gcmkFOOTER();
    return status;
}
