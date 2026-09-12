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


#include "gc_hal_kernel_precomp.h"

#if gcdMMU_VERSION_2
#define _GC_OBJ_ZONE gcvZONE_MMU

static gceSTATUS
_FillPageTable(gctUINT64_PTR PageTable, gctUINT32 PageCount, gctUINT64 EntryValue)
{
    gctUINT i;

    for (i = 0; i < PageCount; i++)
        _WritePageEntry(PageTable + i, EntryValue);

    return gcvSTATUS_OK;
}

static gceSTATUS
_FillMap(gctUINT64_PTR Map, gctUINT32 PageCount, gctUINT32 EntryValue)
{
    gctUINT i;

    for (i = 0; i < PageCount; i++)
        Map[i] = EntryValue;

    return gcvSTATUS_OK;
}

static gceSTATUS
_Link(gcsADDRESS_AREA_PTR Area, gctUINT32 Index, gctUINT32 Next)
{
    if (Index >= Area->pteEntries) {
        /* Just move heap pointer. */
        Area->heapList = Next;
    } else {
        /* Address page table. */
        gctUINT64_PTR map = Area->mapLogical;

        /* Dispatch on node type. */
        switch (gcmENTRY_TYPE(map[Index])) {
        case gcvMMU_SINGLE:
            /* Set single index. */
            map[Index] = ((gctUINT64)Next << 8) | gcvMMU_SINGLE;
            break;

        case gcvMMU_FREE:
            /* Set index. */
            map[Index + 1] = (gctUINT64)Next;
            break;

        default:
            gcmkFATAL("MMU table correcupted at index %u!", Index);
            return gcvSTATUS_HEAP_CORRUPTED;
        }
    }

    /* Success. */
    return gcvSTATUS_OK;
}

static gceSTATUS
_AddFree(gcsADDRESS_AREA_PTR Area, gctUINT32 Index, gctUINT32 Node, gctUINT32 Count)
{
    gctUINT64_PTR map = Area->mapLogical;

    if (Count == 1) {
        /* Initialize a single page node. */
        map[Node] = (~((1ULL << 8) - 1)) | gcvMMU_SINGLE;
    } else {
        /* Initialize the node. */
        map[Node + 0] = ((gctUINT64)Count << 8) | gcvMMU_FREE;
        map[Node + 1] = ~0ULL;
    }

    /* Append the node. */
    return _Link(Area, Index, Node);
}

static gceSTATUS
_Collect(gcsADDRESS_AREA_PTR Area)
{
    gctUINT64_PTR map = Area->mapLogical;
    gceSTATUS status;
    gctUINT32 i, previous, start = 0, count = 0;

    previous = ~0U;
    Area->heapList = ~0U;
    Area->freeNodes = gcvFALSE;

    /* Walk the entire page table. */
    for (i = 0; i < Area->pteEntries; ++i) {
        /* Dispatch based on type of page. */
        switch (gcmENTRY_TYPE(map[i])) {
        case gcvMMU_USED:
            /* Used page, so close any open node. */
            if (count > 0) {
                /* Add the node. */
                gcmkONERROR(_AddFree(Area, previous, start, count));

                /* Reset the node. */
                previous = start;
                count    = 0;
            }
            break;

        case gcvMMU_SINGLE:
            /* Single free node. */
            if (count++ == 0) {
                /* Start a new node. */
                start = i;
            }
            break;

        case gcvMMU_FREE:
            /* A free node. */
            if (count == 0) {
                /* Start a new node. */
                start = i;
            }

            /* Advance the count. */
            count += (gctUINT32)(map[i] >> 8);

            /* Advance the index into the page table. */
            i += (gctUINT32)(map[i] >> 8) - 1;
            break;

        default:
            gcmkFATAL("MMU page table correcupted at index %u!", i);
            return gcvSTATUS_HEAP_CORRUPTED;
        }
    }

    /* See if we have an open node left. */
    if (count > 0) {
        /* Add the node to the list. */
        gcmkONERROR(_AddFree(Area, previous, start, count));
    }

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_MMU, "Performed a garbage collection of the MMU heap.");

    /* Success. */
    return gcvSTATUS_OK;

OnError:
    /* Return the staus. */
    return status;
}

static gctUINT64
_SetPage(gctUINT64 PageAddress,    gctUINT32 PageSizeBits, gctBOOL Reserved, gctBOOL Writable)
{
    gctUINT64 entry = PageAddress
                      /* 2M Page Size */
                      | PageSizeBits
                      /* Ignore exception */
                      | (0 << 1);

    if (Reserved)
        entry |= (0 << 0);
    else
        entry |= (1 << 0);

    if (Writable) {
        /* writable */
        entry |= (1 << 4);
#if gcdUSE_MMU_EXCEPTION
    } else {
        /* If this page is read only, set exception bit to make exception happens
         * when writing to it.
         */
        entry |= gcdMMU_BIT_EXCEPTION;
    }
#else
    }
#endif
    return entry;
}

static gctBOOL
_IsRangeInsected(gctUINT64 baseAddress1, gctSIZE_T size1,
                 gctUINT64 baseAddress2, gctSIZE_T size2)
{
    gctUINT64 endAddress1 = baseAddress1 + size1 - 1;
    gctUINT64 endAddress2 = baseAddress2 + size2 - 1;

    if (!size1 || !size2)
        return gcvFALSE;

    return (((baseAddress2 <= endAddress1) && (endAddress2 >= baseAddress1)) ||
            ((baseAddress1 <= endAddress2) && (endAddress1 >= baseAddress2)));
}

static gceSTATUS
_CollectFreeSpace(gckMMU Mmu, gctUINT64 *Logical, gcsFreeSpaceNode_PTR *Array, gctINT *Size)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPOINTER pointer = gcvNULL;
    gcsFreeSpaceNode_PTR array = gcvNULL;
    gcsFreeSpaceNode_PTR node = gcvNULL;
    gctINT size = 0;
    gctINT i = 0;

    for (i = 0; i < gcdMMU_PUD_ENTRY_NUM * gcdMMU_PGD_ENTRY_NUM; i++) {
        if (!Logical[i]) {
            if (!node) {
                /* This is the first entry of the free space. */
                node += 1;
                size++;
            }
        } else if (node) {
            /* Reset the start. */
            node = gcvNULL;
        }
    }

    /* Allocate memory for the array. */
    gcmkONERROR(gckOS_Allocate(Mmu->os, gcmSIZEOF(*array) * size, &pointer));

    array = (gcsFreeSpaceNode_PTR)pointer;
    node = gcvNULL;

    for (i = 0, size = 0; i < gcdMMU_PUD_ENTRY_NUM * gcdMMU_PGD_ENTRY_NUM; i++) {
        if (!Logical[i]) {
            if (!node) {
                /* This is the first entry of the free space. */
                node = &array[size++];

                node->start = i;
                node->entries = 0;
            }

            node->entries++;
        } else if (node) {
            /* Reset the start. */
            node = gcvNULL;
        }
    }

    *Array = array;
    *Size = size;

    return gcvSTATUS_OK;

OnError:
    if (pointer != gcvNULL)
        gckOS_Free(Mmu->os, pointer);

    return status;
}

gceSTATUS
_GetPudFreeSpace(gckMMU Mmu, gctUINT32 NumEntries,
                 gctUINT32 *PudStart, gctUINT32 *PudEnd)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsFreeSpaceNode_PTR nodeArray = gcvNULL;
    gctINT i, nodeArraySize = 0;
    gctUINT numEntries = gcdMMU_PUD_ENTRY_NUM * gcdMMU_PGD_ENTRY_NUM;
    gctINT32 pudGlobalStart = -1;
    gctINT32 pudGlobalEnd = -1;

    gcmkONERROR(_CollectFreeSpace(Mmu, Mmu->pudLogical, &nodeArray, &nodeArraySize));

    /* Find the smallest space for NumEntries */
    for (i = 0; i < nodeArraySize; i++) {
        if (nodeArray[i].entries <= numEntries && NumEntries <= nodeArray[i].entries) {
            numEntries = nodeArray[i].entries;
#if defined(gcdENABLE_40BIT_VA) && gcdENABLE_40BIT_VA
            /* Try to get avaliable entry from end. */
            pudGlobalEnd = nodeArray[i].start + nodeArray[i].entries - 1;
            pudGlobalStart = pudGlobalEnd - NumEntries + 1;
#else
            /* Due to one pud entry represents 1G VA space. So if we need to limit all VA within
               32bit, we will limit pud entry number under 4 for new mmu driver and try to get
               avaliable entry from start. */
            pudGlobalStart = nodeArray[i].start;
            pudGlobalEnd = nodeArray[i].start + NumEntries - 1;
            if (pudGlobalStart > 3 || pudGlobalEnd > 3) {
                gcmkPRINT("Maybe you has disabled gcdENABLE_40BIT_VA, so please reduce the reserved memory size to below 2GB");
                gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
            }
#endif
        }
    }

    if (pudGlobalStart == -1 && pudGlobalEnd == -1)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    *PudStart = (gctUINT32)pudGlobalStart;
    *PudEnd = (gctUINT32)pudGlobalEnd;

OnError:
    if (nodeArray)
        gckOS_Free(Mmu->os, (gctPOINTER)nodeArray);

    return status;
}

static gcePOOL
_GetPageTablePool(gckOS Os)
{
    gcePOOL pool = gcvPOOL_DEFAULT;
    gctUINT64 data = 0;
    gceSTATUS status;

    status = gckOS_QueryOption(Os, "mmuPageTablePool", &data);

    if (status == gcvSTATUS_OK && data == 1) {
#if USE_LINUX_PCIE
        gctUINT64 externalSize = 0;

        status = gckOS_QueryOption(Os, "externalSize", &externalSize);
        if (status == gcvSTATUS_OK && externalSize != 0)
            pool = gcvPOOL_LOCAL_EXTERNAL;
        else
            pool = gcvPOOL_VIRTUAL;
#else
        pool = gcvPOOL_VIRTUAL;
#endif
    }

    return pool;
}

static gceSTATUS
_GetCurPteChunk(gckMMU Mmu, gctUINT32 Index, gcsMMU_PTE_CHUNK_PTR *PteChunk)
{
    gcsMMU_PTE_CHUNK_PTR pteCurChunk = (gcsMMU_PTE_CHUNK_PTR)Mmu->staticPTE;

    while (pteCurChunk) {
        if (Index >= pteCurChunk->prevPTIndex &&
            (Index < (pteCurChunk->prevPTIndex + pteCurChunk->prevPTEntryNum))) {
            break;
        }
        pteCurChunk = pteCurChunk->next;
    }

    *PteChunk = pteCurChunk;

    return gcvSTATUS_OK;
}

static gceSTATUS
_SearchAvailableMapArea(gcsADDRESS_AREA_PTR Area, gctSIZE_T PageCount, gctUINT32 *Index)
{
    gcsADDRESS_AREA_PTR area = Area;
    gctUINT32 index = 0, previous = ~0U;
    gctUINT64_PTR map;
    gctBOOL gotIt;
    gceSTATUS status = gcvSTATUS_OK;

    /* Search for available area. */
    for (map = area->mapLogical, gotIt = gcvFALSE; !gotIt;) {
        index = area->heapList;

        /* Walk the heap list. */
        for (; !gotIt && (index < area->pteEntries);) {
            /* Check the node type. */
            switch (gcmENTRY_TYPE(map[index])) {
            case gcvMMU_SINGLE:
                /* Single odes are valid if we only need 1 page. */
                if (PageCount == 1) {
                    gotIt = gcvTRUE;
                } else {
                    /* Move to next node. */
                    previous = index;
                    index = (gctUINT32)map[index] >> 8;
                }
                break;

            case gcvMMU_FREE:
                /* Test if the node has enough space. */
                if (PageCount <= (map[index] >> 8)) {
                    gotIt = gcvTRUE;
                } else {
                    /* Move to next node. */
                    previous = index;
                    index = (gctUINT32)map[index + 1];
                }
                break;

            default:
                return gcvSTATUS_OUT_OF_RESOURCES;
            }
        }

        /* Test if we are out of memory. */
        if (index >= area->pteEntries) {
            if (area->freeNodes) {
                /* Time to move out the trash! */
                status = _Collect(area);
                if (gcmIS_ERROR(status))
                    return status;

                /* We are going to search from start, so reset previous to start. */
                previous = ~0U;
            } else {
                /* Out of resources. */
                return gcvSTATUS_OUT_OF_RESOURCES;
            }
        }
    }

    *Index = index;

    return status;
}

static gceSTATUS
gckMMU_FillFlatMappingWithPage2M(gckMMU Mmu, gctUINT64 PhysBase, gctSIZE_T flatSize,
                                 gctBOOL Reserved, gctBOOL needShiftMapping,
                                 gctBOOL specificFlatMapping, gctADDRESS reqVirtualBase,
                                 gctADDRESS *GpuBaseAddress)
{
    gceSTATUS status;
    gckKERNEL kernel = Mmu->hardware->kernel;
    gctBOOL mutex = gcvFALSE;
    gctUINT64 start = PhysBase & ~gcdMMU_PAGE_2M_MASK;
    gctUINT64 end = (PhysBase + flatSize - 1) & ~gcdMMU_PAGE_2M_MASK;
    gctUINT32 pgdStart = (gctUINT32)(start >> gcdMMU_PGD_SHIFT);
    gctUINT32 pudStart = (gctUINT32)((start & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);
    gctUINT32 pudEnd = (gctUINT32)((end & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);
    gctUINT32 pteStart = (gctUINT32)((start & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT);
    gctUINT32 pteEnd = (gctUINT32)((end & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT);
    gctPHYS_ADDR_T physical;
    gcsMMU_PTE_CHUNK_PTR pteNewChunk = gcvNULL, pteCurChunk = gcvNULL;
    gctUINT32 pteIndex = 0, pteNewTotal = 0;
    gctUINT32 pudCurEntry;
    gctINT32 pudFirstEntry = -1;
    gctADDRESS flatVirtualBase = 0;
    enum {
        COLOR_NONE = 0,
        COLOR_RED = 1, /* occupied entry */
        COLOR_BLUE = 2, /* empty entry */
        COLOR_MAX = COLOR_BLUE,
    } lastColor = COLOR_NONE;
    gctUINT32 colorNumber = 0;

    /* Grab the mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Mmu->os, Mmu->pageTableMutex, gcvINFINITE));
    mutex = gcvTRUE;

    if (needShiftMapping) {
        gctUINT32 pudEntries;
        gctUINT32 pteEntries;

        pudEntries = (gctUINT32)((flatSize + (1ULL << gcdMMU_PUD_SHIFT) - 1) /
                                (1ULL << gcdMMU_PUD_SHIFT));
        pteStart = 0;

        gcmkONERROR(_GetPudFreeSpace(Mmu, pudEntries, &pudStart, &pudEnd));

        pteEntries = (gctUINT32)((flatSize + gcdMMU_PAGE_2M_SIZE - 1) / gcdMMU_PAGE_2M_SIZE);
        pteEnd = (pteStart + pteEntries - 1) % gcdMMU_PTE_2M_ENTRY_NUM;
    }

    if (specificFlatMapping) {
        gctUINT64 reqStart = reqVirtualBase & ~gcdMMU_PAGE_2M_MASK;
        gctUINT64 reqEnd   = (reqVirtualBase + flatSize - 1) & ~gcdMMU_PAGE_2M_MASK;

        pudStart = (gctUINT32)((reqStart & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);
        pudEnd = (gctUINT32)((reqEnd & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);
        pteStart = (gctUINT32)((reqStart & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT);
        pteEnd = (gctUINT32)((reqEnd & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT);
    }

    /* No matter direct mapping or shift mapping or specific mapping, store gpu virtual ranges */
    flatVirtualBase = ((gctADDRESS)pgdStart << gcdMMU_PGD_SHIFT) |
                      ((gctADDRESS)pudStart << gcdMMU_PUD_SHIFT) |
                      (pteStart << gcdMMU_PTE_2M_SHIFT) |
                      (PhysBase & gcdMMU_PAGE_2M_MASK);

    /* Return GPU virtual base address if necessary */
    if (GpuBaseAddress)
        *GpuBaseAddress = flatVirtualBase;

    pudCurEntry = pudStart;

    /* find all new pud, part of new flat mapping range may already have pud.*/
    while (pudCurEntry <= pudEnd) {
        if (*(Mmu->pudLogical + pudCurEntry) == 0) {
            if (lastColor != COLOR_BLUE) {
                if (colorNumber < COLOR_MAX) {
                    lastColor = COLOR_BLUE;
                    colorNumber++;
                } else {
                    gcmkPRINT("There is a hole in new flat mapping range, which is not correct");
                }
            }

            pteNewTotal++;
            if (-1 == pudFirstEntry)
                pudFirstEntry = pudCurEntry;
        } else {
            if (lastColor != COLOR_RED) {
                if (colorNumber < COLOR_MAX) {
                    lastColor = COLOR_RED;
                    colorNumber++;
                } else {
                    gcmkPRINT("There is a hole in new flat mapping range, which is not correct");
                }
            }
        }
        pudCurEntry++;
    }

    /* Need allocate a new chunk of pte. */
    if (pteNewTotal) {
        gcePOOL pool = Mmu->pool;
        gctUINT32 allocFlag = gcvALLOC_FLAG_CONTIGUOUS;

        gcmkONERROR(gckOS_Allocate(Mmu->os, sizeof(struct _gcsMMU_PTE_CHUNK),
                                   (gctPOINTER *)&pteNewChunk));

        pteNewChunk->prevPTEntryNum = pteNewTotal;
        pteNewChunk->next = gcvNULL;
        pteNewChunk->videoMem = gcvNULL;
        pteNewChunk->logical = gcvNULL;
        pteNewChunk->size = gcdMMU_PTE_2M_SIZE * pteNewChunk->prevPTEntryNum;
        pteNewChunk->pageCount = 0;
        pteNewChunk->prevPTIndex = pudFirstEntry;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
        allocFlag |= gcvALLOC_FLAG_CACHEABLE;
#endif

        gcmkONERROR(gckKERNEL_AllocateVideoMemory(kernel, 64,
                                                  gcvVIDMEM_TYPE_COMMAND,
                                                  allocFlag | gcvALLOC_FLAG_4K_PAGES,
                                                  &pteNewChunk->size, &pool,
                                                  &pteNewChunk->videoMem));

        /* Lock for kernel side CPU access. */
        gcmkONERROR(gckVIDMEM_NODE_LockCPU(kernel, pteNewChunk->videoMem,
                                           gcvFALSE, gcvFALSE,
                                           (gctPOINTER *)&pteNewChunk->logical));

        gcmkONERROR(gckOS_ZeroMemory(pteNewChunk->logical, pteNewChunk->size));

        /* Get GPU physical address. */
        gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(kernel, pteNewChunk->videoMem, 0, &physical));

        pteNewChunk->physBase = physical;
    }

    while (pudStart <= pudEnd) {
        gctUINT32 last = (pudStart == pudEnd) ? pteEnd : (gcdMMU_PTE_2M_ENTRY_NUM - 1);
        gctPHYS_ADDR_T ptePhysBase;
        gctUINT64_PTR pteLogical;

        gcmkASSERT(pudStart < gcdMMU_PUD_ENTRY_NUM);

        if (*(Mmu->pudLogical + pudStart) == 0) {
            if (!pteNewChunk)
                gcmkONERROR(gcvSTATUS_INVALID_OBJECT);

            pteCurChunk = pteNewChunk;
            ptePhysBase = pteCurChunk->physBase + (pteIndex * gcdMMU_PTE_2M_SIZE);
            pteLogical = (gctUINT64_PTR)((gctUINT8_PTR)pteCurChunk->logical +
                                          (pteIndex * gcdMMU_PTE_2M_SIZE));

            physical = ptePhysBase
                       /* 2MB page size */
                       | (1 << 3)
                       /* Ignore exception */
                       | (0 << 1)
                       /* Present */
                       | (1 << 0);

            _WritePageEntry(Mmu->pudLogical + pudStart, physical);

#if gcdMMU_TABLE_DUMP
            gckOS_Print("%s(%d): insert pud[%d]: %llx\n",
                        __FUNCTION__, __LINE__,
                        pudStart, _ReadPageEntry(Mmu->pudLogical + pudStart));

            gckOS_Print("%s(%d): pte: logical:%llx -> physical:%llx\n",
                         __FUNCTION__, __LINE__,
                        pteLogical, ptePhysBase);
#endif

            gcmkDUMP(Mmu->os, "#[mmu-pud: flat-mapping, slot: %d]", pudStart);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%llx 0x%llx]",
                     (unsigned long long)Mmu->pudPhysical + pudStart * 4,
                     Mmu->pudLogical[pudStart], 4);

            ++pteIndex;
        } else {
            gctUINT64 pudEntryValue = _ReadPageEntry(Mmu->pudLogical + pudStart);
            gctUINT pteOffset;

            _GetCurPteChunk(Mmu, pudStart, &pteCurChunk);

            if (!pteCurChunk)
                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

            pteOffset = pudStart - pteCurChunk->prevPTIndex;

            ptePhysBase = pteCurChunk->physBase + (pteOffset * gcdMMU_PTE_2M_SIZE);
            pteLogical = (gctUINT64_PTR)((gctUINT8_PTR)pteCurChunk->logical +
                                          (pteOffset * gcdMMU_PTE_2M_SIZE));

            if (ptePhysBase != (pudEntryValue & gcdMMU_ENTRY_ATTR_BITS_MASK))
                gcmkASSERT(0);
        }

#if gcdDUMP_IN_KERNEL
        gcmkDUMP(Mmu->os, "#[mmu-pte: flat-mapping: 0x%llx - 0x%llx]",
                 start, start + (last - pteStart) * gcdMMU_PAGE_2M_SIZE - 1);
#endif

        while (pteStart <= last) {
            gcmkASSERT(!(start & gcdMMU_PAGE_2M_MASK));

            _WritePageEntry(pteLogical + pteStart, _SetPage(start, (1 << 3), Reserved, gcvTRUE));

#if gcdMMU_TABLE_DUMP
            gckOS_Print("%s(%d): insert pte[%d]: %llx\n",
                        __FUNCTION__, __LINE__,
                        pteStart, _ReadPageEntry(pteLogical + pteStart));
#endif
            /* next page. */
            start += gcdMMU_PAGE_2M_SIZE;
            pteStart++;
            pteCurChunk->pageCount++;
        }

#if gcdDUMP_IN_KERNEL
        {
            gctUINT32 i = pteStart;
            gctUINT32 data = pteLogical[i] & ~0xF;
            gctUINT32 step = (last > i) ? (pteLogical[i + 1] - pteLogical[i]) : 0;
            gctUINT32 mask = pteLogical[i] & 0xF;

            gcmkDUMP(Mmu->os, "@[physical.step 0x%010llX 0x%llx 0x%llx 0x%llx 0x%llx]",
                     (unsigned long long)ptePhysBase + i * 4, data, (last - i) * 4, step, mask);
        }
#endif

        gcmkONERROR(gckVIDMEM_NODE_CleanCache(kernel, pteCurChunk->videoMem, 0,
                                              pteCurChunk->logical, pteCurChunk->size));

        pteStart = 0;
        ++pudStart;
    }

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(kernel, Mmu->pudVideoMem, 0,
                                          Mmu->pudLogical, Mmu->pudSize));

    gcmkASSERT(pteNewTotal == pteIndex);

    if (pteNewChunk) {
        /* Insert the PTE chunk into staticPTE. */
        if (Mmu->staticPTE == gcvNULL) {
            Mmu->staticPTE = pteNewChunk;
        } else {
            gcmkASSERT(pteNewChunk != gcvNULL);
            gcmkASSERT(pteNewChunk->next == gcvNULL);
            pteNewChunk->next = Mmu->staticPTE;
            Mmu->staticPTE = pteNewChunk;
        }
    }

    /* Release the mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));

    return gcvSTATUS_OK;
OnError:
    if (pteNewChunk) {
        if (pteNewChunk->videoMem)
            gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(kernel, pteNewChunk->videoMem));

        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, pteNewChunk));
    }
    if (mutex) {
        /* Release the mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));
    }
    return status;
}

static gceSTATUS
_ConstructDynamicPmdPte(gckMMU Mmu, gcsADDRESS_AREA_PTR Area)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckKERNEL kernel = Mmu->hardware->kernel;
    gctUINT32 allocFlag = gcvALLOC_FLAG_CONTIGUOUS;
    gcePOOL pool = Mmu->pool;
    gctUINT64 address;
    gctUINT64 pudEntry, pmdEntry;
    gctUINT32 i;
    gctSIZE_T offset = 0;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    allocFlag |= gcvALLOC_FLAG_CACHEABLE;
#endif

    /* 2M dynamic area do not have PMD. */
    if (Area->pmdSize) {
        /* Construct PMD Table. */
        gcmkONERROR(gckKERNEL_AllocateVideoMemory(kernel, 0x1000, gcvVIDMEM_TYPE_COMMAND,
                                                  allocFlag | gcvALLOC_FLAG_4K_PAGES,
                                                  &Area->pmdSize,
                                                  &pool, &Area->pmdVideoMem));

        /* Lock for kernel side CPU access. */
        gcmkONERROR(gckVIDMEM_NODE_LockCPU(kernel, Area->pmdVideoMem,
                                           gcvFALSE, gcvFALSE,
                                           (gctPOINTER *)&Area->pmdLogical));

#if gcdUSE_MMU_EXCEPTION
        gcmkONERROR(_FillPageTable(Area->pmdLogical, Area->pmdEntries,
                                   /* Enable exception */
                                   1 << 1));
#else
        /* Invalidate all entries. */
        gcmkONERROR(gckOS_ZeroMemory(Area->pmdLogical, Area->pmdSize));
#endif
    }

    /* Construct PTE Table. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(kernel, 0x1000, gcvVIDMEM_TYPE_COMMAND,
                                              allocFlag | gcvALLOC_FLAG_4K_PAGES,
                                              &Area->pteSize,
                                              &pool, &Area->pteVideoMem));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(kernel, Area->pteVideoMem,
                                       gcvFALSE, gcvFALSE,
                                       (gctPOINTER *)&Area->pteLogical));

#if gcdUSE_MMU_EXCEPTION
    gcmkONERROR(_FillPageTable(Area->pteLogical, Area->pteEntries,
                               /* Enable exception */
                               1 << 1));
#else
    /* Invalidate all entries. */
    gcmkONERROR(gckOS_ZeroMemory(Area->pteLogical, Area->pteSize));
#endif

    offset = 0;
    /* Write PUD table with PMD physical (or PTE physical for 2M dynamic area). */
    for (i = Area->pudStart; i < Area->pudStart + Area->pudEntries; i++) {

        if (Area->pmdSize) {
            /* Get PMD table physical. */
            gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(kernel, Area->pmdVideoMem,
                        offset, &address));
        } else {
            /* Get PTE table physical. */
            gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(kernel, Area->pteVideoMem,
                        offset, &address));
        }

        pudEntry = address
                   /*Ignore exception */
                   | (0 << 1)
                   /* Present */
                   | (1 << 0);

        if (Area->pmdSize)
            offset += gcdMMU_PMD_SIZE;
        else
            offset += gcdMMU_PTE_2M_SIZE;

        _WritePageEntry(Mmu->pudLogical + i, pudEntry);

#if gcdMMU_TABLE_DUMP
        gckOS_Print("%s(%d): insert PUD[%d]: %08x\n",
                    __FUNCTION__, __LINE__,
                    i, _ReadPageEntry(Mmu->pudLogical + i));
#endif
    }

    gcmkDUMP(Mmu->os, "#[mmu-pud: slot: %d - %d]",
             Area->pudStart, Area->pudEnd - 1);

    if (Area->pmdSize) {
        /* Set page size if it's 4K(low4K) or 64K.
           2M page size flag should write to PTE entry, due to it doesn't have PMD. */
        gctUINT32 pageSize = Area->areaType == gcvAREA_TYPE_64K ? 1 : 0;

        offset = 0;
        /* Write PMD table with PTE physical. */
        for (i = 0; i < Area->pmdEntries; i++) {

            if ((Area->areaType == gcvAREA_TYPE_LOW_4K) &&
                (i < (gcdVA_RESERVED_SIZE >> gcdMMU_PMD_SHIFT))) {
                /* Reserve 0 ~ 16M, not present. */
                pmdEntry = (1 << 2);
            } else {
                /* Get PTE table physical. */
                gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(kernel, Area->pteVideoMem,
                            offset, &address));

                pmdEntry = address
                           /* set page size */
                           | (pageSize << 2)
                           /* Ignore exception */
                           | (0 << 1)
                           /* Present */
                           | (1 << 0);
            }

            if (Area->areaType == gcvAREA_TYPE_64K)
                offset += gcdMMU_PTE_64K_SIZE;
            else
                offset += gcdMMU_PTE_4K_SIZE;

            _WritePageEntry(Area->pmdLogical + i, pmdEntry);

#if gcdMMU_TABLE_DUMP
            gckOS_Print("%s(%d): insert PMD[%d]: %08x\n",
                        __FUNCTION__, __LINE__,
                        i, _ReadPageEntry(Area->pmdLogical + i));
#endif
        }
    }

OnError:
    return status;
}

static gceSTATUS
_ConstructAddressLowArea(gckMMU Mmu, gcsADDRESS_AREA_PTR Area)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64_PTR map;
    gctUINT32 num = gcdVA_RESERVED_SIZE / gcdMMU_PAGE_4K_SIZE;

    Area->pgdStart = 0;
    Area->pgdEnd = 0;
    Area->pudStart = 0;
    Area->pudEnd = 0;
    Area->pudEntries = Area->pudEnd - Area->pudStart + 1;
    Area->pmdEntries = Area->pudEntries * gcdMMU_PMD_ENTRY_NUM;
    Area->pmdSize = Area->pmdEntries << 3;
    Area->pteEntries = Area->pmdEntries * gcdMMU_PTE_4K_ENTRY_NUM;
    Area->pteSize = Area->pteEntries << 3;
    Area->mapSize = 2 * Area->pteSize;
    Area->vaSize = Area->pudEntries * (1 << gcdMMU_PUD_SHIFT);
    Area->baseAddress = ((gctADDRESS)Area->pgdStart << gcdMMU_PGD_SHIFT) |
                        ((Area->pudStart % gcdMMU_PUD_ENTRY_NUM) << gcdMMU_PUD_SHIFT);

    Area->areaType = gcvAREA_TYPE_LOW_4K;

    gcmkONERROR(gckOS_Allocate(Mmu->os, Area->mapSize, (gctPOINTER *)&Area->mapLogical));

    gcmkONERROR(_ConstructDynamicPmdPte(Mmu, Area));

    map = Area->mapLogical;
    /* 0~16M is reserved. */
    _FillMap(&map[0], num, gcvMMU_USED);
    map[num] = (((gctUINT64)Area->pteEntries - num) << 8) | gcvMMU_FREE;
    map[num + 1] = ~0ULL;
    Area->heapList = num;

OnError:
    return status;
}

/*******************************************************************************
 **
 **  _ConstructAddressArea
 **
 **  Construct a dynamic area for map.
 **
 **  INPUT:
 **
 **      gctSIZE_T PageCount
 **          Total page count for this dynamic, generally, we will construct
 **          1G area once, to prevent frequently construct dynamic area.
 **
 **      gceAREA_TYPE Type
 **          The area type of this dynamic area, we support 4k / 64k /
 **          2M area type to dynamic map.
 **
 **  OUTPUT:
 **
 **      gcsADDRESS_AREA_PTR *Area
 **          The final area.
 **
 **      gctUINT32 *Index
 **          An avaliable start map index of this dynamic area.
 */
static gceSTATUS
_ConstructAddressArea(gckMMU Mmu,  gctSIZE_T PageCount, gceAREA_TYPE Type,
                      gcsADDRESS_AREA_PTR *Area, gctUINT32 *Index)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64_PTR map;
    gcsADDRESS_AREA_PTR area = gcvNULL;
    gctUINT32 pudEntries, pudStart, pudEnd;
    gctSIZE_T totalSize = 0;

    gcmkHEADER();

    totalSize = PageCount << gcmkMMU_GET_PTE_SHIFT(Type);
    totalSize = gcmALIGN(totalSize, 0x40000000);

    pudEntries = (gctUINT32)(totalSize >> gcdMMU_PUD_SHIFT);

    gcmkONERROR(_GetPudFreeSpace(Mmu, pudEntries, &pudStart, &pudEnd));

    gcmkONERROR(gckOS_Allocate(Mmu->os, gcmSIZEOF(gcsADDRESS_AREA), (gctPOINTER *)&area));
    gcmkONERROR(gckOS_ZeroMemory(area, gcmSIZEOF(gcsADDRESS_AREA)));

    area->pgdStart = pudStart >> gcdMMU_PUD_BITS;
    area->pgdEnd = pudEnd >> gcdMMU_PUD_BITS;
    area->pudStart = pudStart;
    area->pudEnd = pudEnd;
    area->pudEntries = pudEntries;
    if (Type != gcvAREA_TYPE_2M) {
        area->pmdEntries = area->pudEntries * gcdMMU_PMD_ENTRY_NUM;
        area->pmdSize = area->pmdEntries << 3;
        area->pteEntries = area->pmdEntries * gcmkMMU_GET_ENTRY_NUM(Type);
        area->pteSize = area->pteEntries << 3;
    } else {
        /* if gcvAREA_TYPE_2M type, we treat PMD as PTE */
        area->pmdEntries = 0;
        area->pmdSize = 0;
        area->pteEntries = area->pudEntries * gcmkMMU_GET_ENTRY_NUM(Type);
        area->pteSize = area->pteEntries << 3;
    }
    area->mapSize = 2 * area->pteSize;
    area->baseAddress = ((gctADDRESS)area->pgdStart << gcdMMU_PGD_SHIFT) |
                        ((gctSIZE_T)(area->pudStart % gcdMMU_PUD_ENTRY_NUM) << gcdMMU_PUD_SHIFT);

    area->vaSize = pudEntries * (1 << gcdMMU_PUD_SHIFT);
    area->areaType = Type;

    gcmkONERROR(gckOS_Allocate(Mmu->os, area->mapSize, (gctPOINTER *)&area->mapLogical));

    gcmkONERROR(_ConstructDynamicPmdPte(Mmu, area));

    /* Initialization. */
    map = area->mapLogical;
    map[0] = ((gctUINT64)area->pteEntries << 8) | gcvMMU_FREE;
    map[1] = ~0ULL;
    area->heapList = 0;
    area->freeNodes = gcvFALSE;

    gcmkONERROR(_SearchAvailableMapArea(area, PageCount, Index));

    if (Area)
        *Area = area;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (area) {
        gcmkVERIFY_OK(gckOS_Free(Mmu->os, area));
        area = gcvNULL;
    }

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_FreeAddressArea(gckMMU Mmu, gcsADDRESS_AREA *Area)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckKERNEL Kernel = Mmu->hardware->kernel;

    /* clean the pud entries usage. */
    _FillPageTable(Mmu->pudLogical + Area->pudStart, Area->pudEntries, 0);

    if (!Area)
        return status;

    if (Area->mapLogical != gcvNULL) {
        gcmkVERIFY_OK(gckOS_Free(Kernel->os, (gctPOINTER)Area->mapLogical));
        Area->mapLogical = gcvNULL;
    }

    if (Area->pmdLogical != gcvNULL) {
        /* Free page table. */
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Kernel, Area->pmdVideoMem));
    }

    if (Area->pteLogical != gcvNULL) {
        /* Free page table. */
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Kernel, Area->pteVideoMem));
    }

    if (Area->areaType != gcvAREA_TYPE_LOW_4K) {
        gcsADDRESS_AREA *nextArea = Area->next;
        gcsADDRESS_AREA *prevArea = Mmu->dynamicAreaList;

        /* Try to find the previous area of destroied area. */
        while (gcvTRUE) {
            if (!prevArea)
                break;
            if (prevArea == Area) {
                Mmu->dynamicAreaList = prevArea->next;
                gcmkVERIFY_OK(gckOS_Free(Kernel->os, Area));
                Area = gcvNULL;
                return status;
            }
            if (prevArea->next == Area)
                break;
            else
                prevArea = prevArea->next;
        }
        if (!prevArea)
            return gcvSTATUS_NOT_FOUND;

        /* Remove the area from area list. */
        prevArea->next = nextArea;
    }

    if (Area->areaType != gcvAREA_TYPE_LOW_4K) {
        /* lowArea is not pointer, will release with mmu struct. */
        gcmkVERIFY_OK(gckOS_Free(Kernel->os, Area));
        Area = gcvNULL;
    }

    return status;
}

gctUINT32
_GetPageCountOfUsedNode(gctUINT64_PTR Node)
{
    gctUINT32 count;

    count = (gctUINT32)gcmENTRY_COUNT(*Node);

    if ((count << 8) == (~((1U << 8) - 1)))
        count = 1;

    return count;
}

static gceSTATUS
_FillPgdTable(gckMMU Mmu)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctPHYS_ADDR_T phys, offset;
    gctUINT64 entry;
    int i;

    for (i = 0; i < gcdMMU_PGD_ENTRY_NUM; i++) {
        offset = i * gcdMMU_PUD_SIZE;
        if (offset >= Mmu->pudSize)
            return gcvSTATUS_INVALID_ARGUMENT;

        gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(Mmu->hardware->kernel, Mmu->pudVideoMem, offset, &phys));
        entry = (0 << 1) | (1 << 0) | phys;

        _WritePageEntry(Mmu->pgdLogical + i, entry);
    }

OnError:
    return status;
}

/*******************************************************************************
 **
 **  gckMMU_Construct
 **
 **  Construct a new gckMMU object.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to an gckKERNEL object.
 **
 **      gctSIZE_T MmuSize
 **          Number of bytes for the page table.
 **
 **  OUTPUT:
 **
 **      gckMMU *Mmu
 **          Pointer to a variable that receives the gckMMU object pointer.
 */
gceSTATUS
gckMMU_Construct(gckKERNEL Kernel, gckMMU *Mmu)
{
    gckOS os;
    gckHARDWARE hardware;
    gceSTATUS status;
    gckMMU mmu = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gctPHYS_ADDR_T contiguousBase;
    gctADDRESS contiguousBaseAddress = 0;
    gctSIZE_T contiguousSize = 0;
    gcePOOL pool;
    gctUINT64 data;
    gctUINT32 allocFlag = gcvALLOC_FLAG_CONTIGUOUS;
    gctUINT64 mmuEnabled;
    gckDEVICE device;
    gctPHYS_ADDR_T gpuContiguousBase = gcvINVALID_PHYSICAL_ADDRESS;
    gctPHYS_ADDR_T gpuExternalBase = gcvINVALID_PHYSICAL_ADDRESS;
    int i = 0;
    gctADDRESS flatMapAt = 0;

    gcmkHEADER_ARG("Kernel=%p", Kernel);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);
    gcmkVERIFY_ARGUMENT(Mmu != gcvNULL);

    /* Extract the gckOS object pointer. */
    os = Kernel->os;
    gcmkVERIFY_OBJECT(os, gcvOBJ_OS);

    /* Extract the gckHARDWARE object pointer. */
    hardware = Kernel->hardware;
    gcmkVERIFY_OBJECT(hardware, gcvOBJ_HARDWARE);

    device = Kernel->device;

    if (device->showMemInfo) {
        gcmkPRINT("[ftg340]: mmu v2 processPageTable=%d largeVAVersion=%d graphicsLargeVA=%d reserve32bitVA=%d",
                   Kernel->processPageTable, hardware->largeVAVersion, hardware->graphicsLargeVA, Kernel->reserve32bitVA);
    }

    /* Allocate memory for the gckMMU object. */
    gcmkONERROR(gckOS_Allocate(os, sizeof(struct _gckMMU), &pointer));

    gckOS_ZeroMemory(pointer, sizeof(struct _gckMMU));

    mmu = pointer;

    /* Initialize the gckMMU object. */
    mmu->object.type = gcvOBJ_MMU;
    mmu->os = os;
    mmu->hardware = hardware;
    mmu->enabled = gcvFALSE;
    mmu->initMode = gcvMMU_INIT_FROM_CMD;
    mmu->pool = _GetPageTablePool(mmu->os);
    gcsLIST_Init(&mmu->hardwareList);

    /* Create the page table mutex. */
    gcmkONERROR(gckOS_CreateMutex(os, &mmu->pageTableMutex));

    gcmkONERROR(gckOS_QueryOption(os, "mmu", &mmuEnabled));

    mmu->flatMapping = Kernel->flatMapping;

    mmu->flatMappingMode = gcdFLAT_MAPPING_MODE;

    mmu->pgdSize = gcdMMU_PGD_SIZE;
    mmu->pudSize = gcdMMU_PUD_SIZE * gcdMMU_PGD_ENTRY_NUM;

    if (device->externalSize) {
        gcmkONERROR(gckOS_CPUPhysicalToGPUPhysical(mmu->os,
                                                   device->externalBase,
                                                   &gpuExternalBase));
    }

    contiguousBase = device->contiguousBases[0];
    contiguousSize = device->contiguousSizes[0];

    if (!contiguousSize) {
        status = gckOS_QueryOption(mmu->os, "contiguousBase", &contiguousBase);

        if (gcmIS_SUCCESS(status)) {
            status = gckOS_QueryOption(mmu->os, "contiguousSize", &data);
            contiguousSize = (gctSIZE_T)data;
        }
    }

    if (!device->lowContiguousSize) {
        data = 0;

        gckOS_QueryOption(mmu->os, "lowContiguousSize", &data);
        if (data) {
            contiguousSize += (gctSIZE_T)data;

            status = gckOS_QueryOption(mmu->os, "lowContiguousBase", &data);
            if (gcmIS_SUCCESS(status))
                contiguousBase = (gctADDRESS)data;
        }
    } else {
        contiguousSize += device->lowContiguousSize;
        contiguousBase = device->lowContiguousBase;
    }

    if (contiguousSize) {
        gcmkONERROR(gckOS_CPUPhysicalToGPUPhysical(mmu->os, contiguousBase,
                                                   &gpuContiguousBase));
    }

    pool = mmu->pool;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    allocFlag |= gcvALLOC_FLAG_CACHEABLE;
#endif
    /* Allocate pgd table. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(Kernel, 0x1000, gcvVIDMEM_TYPE_COMMAND,
                                              allocFlag | gcvALLOC_FLAG_4K_PAGES,
                                              &mmu->pgdSize, &pool, &mmu->pgdVideoMem));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(Kernel, mmu->pgdVideoMem,
                                       gcvFALSE, gcvFALSE, (gctPOINTER *)&mmu->pgdLogical));

    /* Get pgd table physical. */
    gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(Kernel, mmu->pgdVideoMem,
                                              0, &mmu->pgdPhysical));

    mmu->mtlbPhysical = mmu->pgdPhysical;

    /* Invalid all the entries. */
    gcmkONERROR(gckOS_ZeroMemory(mmu->pgdLogical, mmu->pgdSize));

    /* Allocate pud table. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(Kernel, 0x1000, gcvVIDMEM_TYPE_COMMAND,
                                              allocFlag | gcvALLOC_FLAG_4K_PAGES,
                                              &mmu->pudSize, &pool, &mmu->pudVideoMem));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(Kernel, mmu->pudVideoMem,
                                       gcvFALSE, gcvFALSE, (gctPOINTER *)&mmu->pudLogical));

    gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(Kernel, mmu->pudVideoMem,
                                              0, &mmu->pudPhysical));

    /* Invalid all the entries. */
    gcmkONERROR(gckOS_ZeroMemory(mmu->pudLogical, mmu->pudSize));

    /* Write all the pgd entries. */
    gcmkONERROR(_FillPgdTable(mmu));

#if !gcdCAPTURE_ONLY_MODE
    /* Reserve the space in page table for reserve usage. */
    if (!_ReadPageEntry(mmu->pudLogical)) {
        if (!Kernel->reserve32bitVA) {
            gcmkONERROR(gckMMU_FillFlatMappingWithPage2M(mmu, 0, gcdVA_RESERVED_SIZE, gcvTRUE, gcvFALSE,
                                                         gcvFALSE, 0, gcvNULL));

            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].start = 0;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].end = gcdVA_RESERVED_SIZE - 1;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].size = gcdVA_RESERVED_SIZE;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].flag = gcvFLATMAP_DIRECT;
            mmu->gpuAddressRangeCount++;
            mmu->reserveRangeSize = gcdVA_RESERVED_SIZE;
        } else {
            gcmkONERROR(_ConstructAddressLowArea(mmu, &mmu->dynamicLowArea4K));
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].start = 0;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].end = gcdLOW4K_RESERVE_SIZE - 1;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].size = gcdLOW4K_RESERVE_SIZE;
            mmu->gpuAddressRanges[mmu->gpuAddressRangeCount].flag = gcvFLATMAP_DIRECT;
            mmu->gpuAddressRangeCount++;
            mmu->reserveRangeSize = gcdLOW4K_RESERVE_SIZE;
            flatMapAt = gcdLOW4K_RESERVE_SIZE;
        }
    }
#endif

    gcmkONERROR(gckMMU_SetupSRAM(mmu, mmu->hardware, Kernel->device));

    if (Kernel->device->lockLargeVA) {
        flatMapAt = gcd4G_SIZE;
    } else {
        /* flatmap from 3G. */
        if (flatMapAt < gcd4G_SIZE - gcd4G_VA_FM_SIZE)
            flatMapAt = gcd4G_SIZE - gcd4G_VA_FM_SIZE;
    }

    if (mmu->flatMapping && Kernel->device->externalSize &&
        gpuExternalBase != gcvINVALID_PHYSICAL_ADDRESS) {
        gctADDRESS externalBaseAddress = 0;

        if (Kernel->reserve32bitVA) {
            /* If has, external pool will always flatmap from 3G. */
            externalBaseAddress = flatMapAt;
        }

        /* Setup flat mapping for external memory. */
        gcmkONERROR(gckMMU_FillFlatMapping(mmu, gpuExternalBase,
                                           Kernel->device->externalSize,
                                           gcvFALSE, gcvTRUE, &externalBaseAddress));

        mmu->externalBaseAddress = externalBaseAddress;

        /* update the available position to flatmap. */
        flatMapAt = externalBaseAddress + Kernel->device->externalSize;
        /* flatmap from next pud. */
        flatMapAt = gcmALIGN(flatMapAt, 0x40000000);

        if (device->showMemInfo)
            gcmkPRINT("[ftg340]: device%d external pool CPU physical=0x%llx GPU physical=0x%llx virtual=0x%llx size=0x%zx",
                       device->id, device->externalBase, gpuExternalBase, externalBaseAddress, device->externalSize);
    }

    if (mmu->flatMapping) {
        for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
            if (i) {
                contiguousSize = device->contiguousSizes[i];

                if (contiguousSize)
                    gcmkONERROR(gckOS_CPUPhysicalToGPUPhysical(mmu->os,
                                                               device->contiguousBases[i],
                                                               &gpuContiguousBase));
            } else if (Kernel->reserve32bitVA) {
                gcmkASSERT(gpuContiguousBase >= mmu->reserveRangeSize + device->externalSize);

                contiguousBaseAddress = flatMapAt;
            }

            if (contiguousSize && gpuContiguousBase != gcvINVALID_PHYSICAL_ADDRESS) {
                /* Setup flat mapping for Reserved memory (VIDMEM). */
                gcmkONERROR(gckMMU_FillFlatMapping(mmu, gpuContiguousBase, contiguousSize,
                                                   gcvFALSE, gcvTRUE, &contiguousBaseAddress));
                flatMapAt = contiguousBaseAddress + contiguousSize;
                flatMapAt = gcmALIGN(flatMapAt, 0x40000000);
                if (mmuEnabled)
                    mmu->contiguousBaseAddresses[i] = contiguousBaseAddress;
                else
                    gcmkSAFECASTPHYSADDRT(mmu->contiguousBaseAddresses[i], gpuContiguousBase);

                if (device->showMemInfo)
                    gcmkPRINT("[ftg340]: system Reserved pool%d CPU physical=0x%llx GPU physical=0x%llx virtual=0x%llx size=0x%llx",
                              i, contiguousBase, gpuContiguousBase, mmu->contiguousBaseAddresses[i], (gctUINT64)contiguousSize);
            }
        }
    }

    if (mmu->flatMapping && Kernel->device->exclusiveSize) {
        gctUINT64 gpuExclusiveBase;
        gctADDRESS exclusiveBaseAddress = flatMapAt;

        gcmkONERROR(gckOS_CPUPhysicalToGPUPhysical(mmu->os,
            Kernel->device->exclusiveBase,
            &gpuExclusiveBase));

        /* Setup flat mapping for external memory. */
        gcmkONERROR(gckMMU_FillFlatMapping(mmu, gpuExclusiveBase,
            Kernel->device->exclusiveSize,
            gcvFALSE, gcvTRUE, &exclusiveBaseAddress));

        mmu->exclusiveBaseAddress = exclusiveBaseAddress;

        if (device->showMemInfo)
            gcmkPRINT("[ftg340]: device%d exclusive pool CPU physical=0x%llx GPU physical=0x%llx virtual=0x%llx size=0x%zx",
                device->id, device->exclusiveBase, gpuExclusiveBase, exclusiveBaseAddress, device->exclusiveSize);
    }

    device->showMemInfo = gcvFALSE;

    /* A 64 byte for safe address, we use 256 here. */
    mmu->safePageSize = 256;

    pool = mmu->pool;

    allocFlag = gcvALLOC_FLAG_CONTIGUOUS | gcvALLOC_FLAG_4K_PAGES;

    /* Allocate safe page from video memory. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(Kernel, 256, gcvVIDMEM_TYPE_COMMAND, allocFlag,
                                              &mmu->safePageSize, &pool, &mmu->safePageVideoMem));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(Kernel, mmu->safePageVideoMem,
                                       gcvFALSE, gcvFALSE,
                                       &mmu->safePageLogical));

    /* Get GPU physical address. */
    gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(Kernel, mmu->safePageVideoMem,
                                              0, &mmu->safePagePhysical));

    mmu->safeAddress = mmu->safePagePhysical;

    gckOS_ZeroMemory(mmu->safePageLogical, mmu->safePageSize);

    gcmkDUMP(mmu->os, "#[safe page]");
    gcmkDUMP(mmu->os, "@[physical.fill 0x%010llX 0x%llx 0x%08lX]",
             (unsigned long long)mmu->safePagePhysical, 0, (unsigned long)mmu->safePageSize);

    gcmkDUMP_BUFFER(mmu->os, gcvDUMP_BUFFER_KERNEL_COMMAND,
                    mmu->safePageLogical, mmu->safeAddress, mmu->safePageSize);

    gcmkONERROR(gckQUEUE_Allocate(os, &mmu->recentFreedAddresses, 32));

    if (Kernel->processPageTable) {
        gcsLIST_Init(&mmu->nodeList);

        gcmkONERROR(gckOS_CreateMutex(mmu->os, &mmu->nodeListMutex));
    }

    mmu->sRAMMapped = gcvFALSE;

    /* Return the gckMMU object pointer. */
    *Mmu = mmu;

    /* Success. */
    gcmkFOOTER_ARG("*Mmu=%p", *Mmu);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (mmu != gcvNULL) {
        if (mmu->pgdLogical)
            gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Kernel, mmu->pgdVideoMem));

        if (mmu->pudLogical)
            gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Kernel, mmu->pudVideoMem));

        if (mmu->pageTableMutex) {
            /* Delete the mutex. */
            gcmkVERIFY_OK(gckOS_DeleteMutex(os, mmu->pageTableMutex));
        }

        gcmkVERIFY_OK(gckQUEUE_Free(os, &mmu->recentFreedAddresses));

        /* Mark the gckMMU object as unknown. */
        mmu->object.type = gcvOBJ_UNKNOWN;

        /* Free the allocates memory. */
        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(os, mmu));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_Destroy(gckMMU Mmu)
{
    gckKERNEL kernel = Mmu->hardware->kernel;
    gcsADDRESS_AREA_PTR area;

    gcmkHEADER_ARG("Mmu=%p", Mmu);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);

    while (Mmu->staticPTE != gcvNULL) {
        gcsMMU_PTE_CHUNK_PTR pre = Mmu->staticPTE;

        Mmu->staticPTE = pre->next;

        if (pre->videoMem)
            gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(kernel, pre->videoMem));

        if (pre->prevPTEntryNum) {
            gctUINT i;

            for (i = 0; i < pre->prevPTEntryNum; ++i) {
                /* Clean pud entires in flat mapping area. */
                _WritePageEntry(Mmu->pudLogical + pre->prevPTIndex + i, 0);
            }

            gcmkDUMP(Mmu->os, "#[mmu-pud: clean up slot: %d - %d]", pre->prevPTIndex,
                     pre->prevPTIndex + pre->prevPTEntryNum - 1);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%llx 0x%08lX]",
                     (unsigned long long)(Mmu->pudPhysical + pre->prevPTIndex * 4),
                     Mmu->pudLogical[pre->prevPTIndex],
                     (unsigned long)(pre->prevPTEntryNum * 4));
        }

        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, pre));
    }

    /* Free address area. */
    area = Mmu->dynamicAreaList;
    while (area) {
        _FreeAddressArea(Mmu, area);
        area = Mmu->dynamicAreaList;
    }

    Mmu->dynamicAreaList = gcvNULL;

    _FreeAddressArea(Mmu, &Mmu->dynamicLowArea4K);

    if (Mmu->pudLogical)
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(kernel, Mmu->pudVideoMem));

    if (Mmu->pgdLogical)
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(kernel, Mmu->pgdVideoMem));

    /* Delete the page table mutex. */
    if (Mmu->pageTableMutex) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Mmu->os, Mmu->pageTableMutex));
        Mmu->pageTableMutex = gcvNULL;
    }

    if (Mmu->nodeListMutex && kernel->processPageTable) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Mmu->os, Mmu->nodeListMutex));
        Mmu->nodeListMutex = gcvNULL;
    }

    if (Mmu->safePageLogical)
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(kernel, Mmu->safePageVideoMem));

    gcmkVERIFY_OK(gckQUEUE_Free(Mmu->os, &Mmu->recentFreedAddresses));

    /* Mark the gckMMU object as unknown. */
    Mmu->object.type = gcvOBJ_UNKNOWN;

    /* Free the gckMMU object. */
    gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, Mmu));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckMMU_AllocatePagesEx(gckMMU Mmu, gctSIZE_T PageCount, gceVIDMEM_TYPE Type,
               gcePAGE_TYPE PageType, gctBOOL LowVA, gctBOOL Secure,
               gctPOINTER *PageTable, gctADDRESS *Address)
{
    gceSTATUS status;
    gctBOOL mutex = gcvFALSE;
    gctUINT32 index = 0, previous = ~0U, left;
    gctUINT64_PTR map;
    gctADDRESS address;
    gctUINT32 pageCount;
    gcsADDRESS_AREA_PTR area = Mmu->dynamicAreaList;
    gcsADDRESS_AREA_PTR newArea = gcvNULL;
    gcsADDRESS_AREA_PTR gotArea;
    gctBOOL gotIt;
    gctUINT64 pgd = 0, pud = 0, pmd = 0, pte = 0;
    gceAREA_TYPE areaType = gcmkMMU_GET_AERA_TYPE(PageType);

    gcmkHEADER_ARG("Mmu=%p PageCount=%lu", Mmu, PageCount);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);
    gcmkVERIFY_ARGUMENT(PageCount > 0);
    gcmkVERIFY_ARGUMENT(PageTable != gcvNULL);

    gcmkSAFECASTSIZET(pageCount, PageCount);

#if gcdBOUNDARY_CHECK
    /* Extra pages as bounary. */
    pageCount += gcdBOUNDARY_CHECK * 2;
#endif

    /* Grab the mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Mmu->os, Mmu->pageTableMutex, gcvINFINITE));
    mutex = gcvTRUE;

    if (LowVA && Mmu->hardware->kernel->reserve32bitVA) {
        area = &Mmu->dynamicLowArea4K;
    } else {
        while (area) {
            if (area->areaType == areaType) {
                status = _SearchAvailableMapArea(area, pageCount, &index);
                if (status == gcvSTATUS_OK)
                    break;
            }

            area = area->next;
        }
    }

    if (!area) {
        if (LowVA && Mmu->hardware->kernel->reserve32bitVA)
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

        gcmkONERROR(_ConstructAddressArea(Mmu, PageCount, areaType, &newArea, &index));

        if (Mmu->dynamicAreaList == gcvNULL) {
            Mmu->dynamicAreaList = newArea;
        } else {
            newArea->next = Mmu->dynamicAreaList;
            Mmu->dynamicAreaList = newArea;
        }

        gotArea = newArea;
    } else {
        gotArea = area;
    }

    /* Cast pointer to page table. */
    for (map = gotArea->mapLogical, gotIt = gcvFALSE; !gotIt;) {
        index = gotArea->heapList;

        /* Walk the heap list. */
        for (; !gotIt && (index < gotArea->pteEntries);) {
            /* Check the node type. */
            switch (gcmENTRY_TYPE(map[index])) {
            case gcvMMU_SINGLE:
                /* Single odes are valid if we only need 1 page. */
                if (pageCount == 1) {
                    gotIt = gcvTRUE;
                } else {
                    /* Move to next node. */
                    previous = index;
                    index = (gctUINT32)map[index] >> 8;
                }
                break;

            case gcvMMU_FREE:
                /* Test if the node has enough space. */
                if (pageCount <= (map[index] >> 8)) {
                    gotIt = gcvTRUE;
                } else {
                    /* Move to next node. */
                    previous = index;
                    index = (gctUINT32)map[index + 1];
                }
                break;

            default:
                gcmkFATAL("MMU table correcupted at index %u!", index);
                gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
            }
        }

        /* Test if we are out of memory. */
        if (index >= gotArea->pteEntries) {
            if (gotArea->freeNodes) {
                /* Time to move out the trash! */
                gcmkONERROR(_Collect(gotArea));

                /* We are going to search from start, so reset previous to start. */
                previous = ~0U;
            } else {
                /* Out of resources. */
                gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
            }
        }
    }

    switch (gcmENTRY_TYPE(map[index])) {
    case gcvMMU_SINGLE:
        /* Unlink single node from free list. */
        gcmkONERROR(_Link(gotArea, previous, (gctUINT32)(map[index] >> 8)));
        break;

    case gcvMMU_FREE:
        /* Check how many pages will be left. */
        left = (gctUINT32)((map[index] >> 8) - pageCount);
        switch (left) {
        case 0:
            /* The entire node is consumed, just unlink it. */
            gcmkONERROR(_Link(gotArea, previous, (gctUINT32)map[index + 1]));
            break;

        case 1:
            /*
             * One page will remain.  Convert the node to a single node and
             * advance the index.
             */
            map[index] = (map[index + 1] << 8) | gcvMMU_SINGLE;
            index++;
            break;

        default:
            /*
             * Enough pages remain for a new node.  However, we will just adjust
             * the size of the current node and advance the index.
             */
            map[index] = ((gctUINT64)left << 8) | gcvMMU_FREE;
            index += left;
            break;
        }
        break;
    }

    /* Mark node as used. */
    gcmkONERROR(_FillMap(&map[index], pageCount, gcvMMU_USED));

#if gcdBOUNDARY_CHECK
    index += gcdBOUNDARY_CHECK;
#endif

    /* Record pageCount of allocated node at the beginning of node. */
    if (pageCount == 1)
        map[index] = (~((1ULL << 8) - 1)) | gcvMMU_USED;
    else
        map[index] = ((gctUINT64)pageCount << 8) | gcvMMU_USED;

    if (gotArea->pteLogical != gcvNULL) {
        *PageTable = &gotArea->pteLogical[index];
    } else {
        *PageTable = gcvNULL;
    }

    pgd = gotArea->pgdStart;
    pud = gotArea->pudStart % gcdMMU_PUD_ENTRY_NUM;
    if (areaType != gcvAREA_TYPE_2M)
        pmd = index / gcmkMMU_GET_ENTRY_NUM(areaType);
    pte = index % gcmkMMU_GET_ENTRY_NUM(areaType);
    /* Build virtual address. */
    address = (pgd << gcdMMU_PGD_SHIFT) |
              (pud << gcdMMU_PUD_SHIFT) |
              (pmd << gcdMMU_PMD_SHIFT) | /* +mapping start?*/
              (pte << gcmkMMU_GET_PTE_SHIFT(areaType));

    if (Address != gcvNULL)
        *Address = address;

    /* Release the mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));

    /* Success. */
    gcmkFOOTER_ARG("*PageTable=%p *Address=%08x", *PageTable, gcmOPT_VALUE(Address));
    return gcvSTATUS_OK;

OnError:

    if (mutex) {
        /* Release the mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));
    }

    if (newArea)
        gcmkVERIFY_OK(_FreeAddressArea(Mmu, newArea));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_FreePages(gckMMU Mmu, gctBOOL Secure, gcePAGE_TYPE PageType, gctBOOL LowVA,
           gctADDRESS Address, gctPOINTER PageTable, gctSIZE_T PageCount)
{
    gctUINT32 index;
    gctUINT64_PTR node;
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gctUINT32 pageCount;
    gcuQUEUEDATA data;
    gcsADDRESS_AREA_PTR area = Mmu->dynamicAreaList;
    gctUINT32 pageSize = (PageType == gcvPAGE_TYPE_2M) ?
                         gcdMMU_PAGE_2M_SIZE : gcdMMU_PAGE_4K_SIZE;
    gceAREA_TYPE areaType = (PageType == gcvPAGE_TYPE_4K) ?
                            gcvAREA_TYPE_4K : gcvAREA_TYPE_2M;

    gcmkHEADER_ARG("Mmu=%p PageTable=%p PageCount=%lu",
                   Mmu, PageTable, PageCount);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);
    gcmkVERIFY_ARGUMENT(PageCount > 0);

    gcmkSAFECASTSIZET(pageCount, PageCount);

#if gcdBOUNDARY_CHECK
    pageCount += gcdBOUNDARY_CHECK * 2;
#endif

    gcmkONERROR(gckOS_AcquireMutex(Mmu->os, Mmu->pageTableMutex, gcvINFINITE));
    acquired = gcvTRUE;

    if (LowVA && Mmu->hardware->kernel->reserve32bitVA) {
        area = &Mmu->dynamicLowArea4K;
    } else {
        while (area) {
            if (area->areaType == areaType &&
                (Address >= area->baseAddress && Address < area->baseAddress + area->vaSize))
                break;

            area = area->next;
        }
    }

    if (!area)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    /* Get the node by index. */
    index = (gctUINT32)((gctUINT64_PTR)PageTable - area->pteLogical);

    node = area->mapLogical + index;

    if (pageCount != _GetPageCountOfUsedNode(node))
        gcmkONERROR(gcvSTATUS_INVALID_REQUEST);

#if gcdBOUNDARY_CHECK
    node -= gcdBOUNDARY_CHECK;
#endif

    if (pageCount == 1) {
        /* Single page node. */
        node[0] = (~((1ULL << 8) - 1)) | gcvMMU_SINGLE;

        if (PageTable != gcvNULL) {
#if gcdUSE_MMU_EXCEPTION
            /* Enable exception */
            _WritePageEntry(PageTable, (1 << 1));
#else
            _WritePageEntry(PageTable, 0);
#endif
        }
    } else {
        /* Mark the node as free. */
        node[0] = ((gctUINT64)pageCount << 8) | gcvMMU_FREE;
        node[1] = ~0ULL;

        if (PageTable != gcvNULL) {
#if gcdUSE_MMU_EXCEPTION
            /* Enable exception */
            gcmkVERIFY_OK(_FillPageTable(PageTable, (gctUINT32)PageCount, 1 << 1));
#else
            gcmkVERIFY_OK(_FillPageTable(PageTable, (gctUINT32)PageCount, 0));
#endif
        }
    }

    /* We have free nodes. */
    area->freeNodes = gcvTRUE;

    /* Record freed address range. */
    data.addressData.start = Address;
    data.addressData.end = Address + (gctUINT32)PageCount * pageSize;
    gckQUEUE_Enqueue(&Mmu->recentFreedAddresses, &data);

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));
    acquired = gcvFALSE;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->pageTableMutex));

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_AllocatePages(gckMMU Mmu, gctSIZE_T PageCount, gcePAGE_TYPE PageType,
                     gctPOINTER *PageTable, gctADDRESS *Address)
{
    return gckMMU_AllocatePagesEx(Mmu, PageCount, gcvVIDMEM_TYPE_GENERIC,
                                  PageType, gcvFALSE, gcvFALSE, PageTable, Address);
}

gceSTATUS
gckMMU_SetPage(gckMMU Mmu, gctPHYS_ADDR_T PageAddress, gcePAGE_TYPE PageType,
               gctBOOL LowVA, gctBOOL Writable, gctPOINTER PageEntry)
{
    gctUINT64 entry;

    gcmkHEADER_ARG("Mmu=%p", Mmu);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);
    gcmkVERIFY_ARGUMENT(PageEntry != gcvNULL);

    if (PageType == gcvPAGE_TYPE_2M)
        gcmkVERIFY_ARGUMENT(!(PageAddress & 0x1FFFFF));
    else
        gcmkVERIFY_ARGUMENT(!(PageAddress & 0xFFF));

    entry = PageAddress | (1 << 0);

    if (Writable) {
        /* writable */
        entry |= (1 << 4);
    } else {
        entry |= (0 << 4);
#if gcdUSE_MMU_EXCEPTION
        /* If this page is read only, set exception bit to make exception happens
         * when writing to it.
         */
        entry |= gcdMMU_BIT_EXCEPTION;
#endif
    }

    if (PageType == gcvPAGE_TYPE_2M)
        entry |= (2 << 2);
    else if (PageType == gcvPAGE_TYPE_64K)
        entry |= (1 << 2);
    else
        entry |= (0 << 2);

    _WritePageEntry(PageEntry, entry);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckMMU_Flush(gckMMU Mmu, gceVIDMEM_TYPE Type)
{
    gckHARDWARE hardware;
    gckMMU mmu;
    gctUINT32 mask;
    gctINT i;
    gcsLISTHEAD_PTR hardwareHead;

    mask = gcvPAGE_TABLE_DIRTY_BIT_OTHER;

    hardware = Mmu->hardware;

    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
        gcmkVERIFY_OK(gckOS_AtomSetMask(hardware->pageTableDirty[i], mask));


    if (hardware->kernel->processPageTable)
        mmu = hardware->kernel->mmu;
    else
        mmu = Mmu;

    gcmkLIST_FOR_EACH(hardwareHead, &mmu->hardwareList) {
        hardware = gcmCONTAINEROF(hardwareHead, struct _gckHARDWARE, mmuHead);

        if (hardware != mmu->hardware) {
            for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++)
                gcmkVERIFY_OK(gckOS_AtomSetMask(hardware->pageTableDirty[i], mask));
        }
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gckMMU_DumpPageTableEntry(gckMMU Mmu, gceAREA_TYPE AreaType, gctADDRESS Address)
{
    gctUINT64_PTR pageTable;
    gctUINT32 index;
    gctUINT32 pud, pmd, pte;
    gcsADDRESS_AREA_PTR area = Mmu->dynamicAreaList;

    gcmkHEADER_ARG("Mmu=0x%llx Address=0x%llx", Mmu, Address);
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);

    if (AreaType == gcvAREA_TYPE_LOW_4K) {
        area = &Mmu->dynamicLowArea4K;
    } else {
        while (area) {
            if (area->areaType == AreaType &&
                (Address >= area->baseAddress && Address < area->baseAddress + area->vaSize))
                break;

            area = area->next;
        }
    }

    if (!area && AreaType != gcvAREA_TYPE_FLATMAP) {
        gcmkFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    pud = (gctUINT32)((Address & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);

    if (AreaType != gcvAREA_TYPE_FLATMAP) {
        pmd = (gctUINT32)((Address & gcdMMU_PMD_MASK) >> gcdMMU_PMD_SHIFT);
        pte = (gctUINT32)((Address & gcdMMU_PTE_4K_MASK) >> gcdMMU_PTE_4K_SHIFT);

        pageTable = area->pteLogical;

        index = (pud - area->pudStart % gcdMMU_PUD_ENTRY_NUM) * gcdMMU_PMD_ENTRY_NUM * gcdMMU_PTE_4K_ENTRY_NUM +
                 pmd * gcdMMU_PTE_4K_ENTRY_NUM + pte;

        gcmkPRINT("    Page table entry = 0x%llx",
                  _ReadPageEntry(pageTable + index));
    } else {
        gcsMMU_PTE_CHUNK_PTR pteChunkObj = Mmu->staticPTE;
        gctUINT64 entry = Mmu->pudLogical[pud];

        pte = (Address & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT;

        entry &= 0xFFFFFFF0;

        while (pteChunkObj) {
            gctUINT i;
            gctBOOL found = gcvFALSE;

            for (i = 0; i < pteChunkObj->prevPTEntryNum; ++i) {
                gctPHYS_ADDR_T ptePhysBase = pteChunkObj->physBase + (i * gcdMMU_PTE_2M_SIZE);
                gctUINT64_PTR pteLogical = (gctUINT64_PTR)((gctUINT8_PTR)pteChunkObj->logical +
                                                              (i * gcdMMU_PTE_2M_SIZE));
                if (entry == ptePhysBase) {
                    gcmkPRINT("    Page table entry = 0x%llx", pteLogical[pte]);
                    found = gcvTRUE;
                    break;
                }
            }
            if (found)
                break;

            pteChunkObj = pteChunkObj->next;
        }
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

void
gckMMU_DumpRecentFreedAddress(gckMMU Mmu)
{
    gckQUEUE queue = &Mmu->recentFreedAddresses;
    gctUINT32 i;
    gcuQUEUEDATA *data;

    if (queue->count) {
        gcmkPRINT("    Recent %d freed GPU address ranges:", queue->count);

        for (i = 0; i < queue->count; i++) {
            gckQUEUE_GetData(queue, i, &data);

            gcmkPRINT("      [%llx - %llx]",
                      data->addressData.start, data->addressData.end);
        }
    }
}

gceSTATUS
gckMMU_FillFlatMapping(gckMMU Mmu, gctUINT64 PhysBase,
                       gctSIZE_T Size, gctBOOL Reserved,
                       gctBOOL AbleToShift, gctADDRESS *GpuBaseAddress)
{
    gceSTATUS status;
    gctBOOL physicalRangeOverlapped = gcvFALSE;
    gctBOOL virtualRangeOverlapped = gcvFALSE;
    gctBOOL specificFlatMapping = gcvFALSE;
    gctBOOL needShiftMapping = gcvFALSE;
    gctUINT64 flatBase = PhysBase;
    gctSIZE_T flatSize = Size;
    gctUINT64 base = flatBase;
    gctUINT64 end = base + flatSize;
    gctADDRESS reqVirtualBase = 0;
    gctADDRESS flatVirtualBase = 0;
    gctADDRESS baseAddress = 0;
    gceFLATMAP_FLAG mapFlag = gcvFLATMAP_DIRECT;
    gctUINT32 i;

    /************************ Get flat mapping type and range. ************************/
    for (i = 0; i < Mmu->gpuPhysicalRangeCount; i++) {
        if (base < Mmu->gpuPhysicalRanges[i].start) {
            if (end > Mmu->gpuPhysicalRanges[i].start) {
                physicalRangeOverlapped = gcvTRUE;
                if (Mmu->gpuPhysicalRanges[i].flag == gcvFLATMAP_DIRECT) {
                    /* Overlapped part is direct mapping, continue direct mapping */
                    end = Mmu->gpuPhysicalRanges[i].start;
                } else {
                    /* Overlapped part is shift mapping, do entire shift mapping */
                    needShiftMapping = gcvTRUE;
                }
            }

            flatSize = (gctSIZE_T)(end - base);
        } else if (end > Mmu->gpuPhysicalRanges[i].end) {
            if (base < Mmu->gpuPhysicalRanges[i].end) {
                physicalRangeOverlapped = gcvTRUE;
                if (Mmu->gpuPhysicalRanges[i].flag == gcvFLATMAP_DIRECT) {
                    /* Overlapped part is direct mapping, continue direct mapping */
                    base = Mmu->gpuPhysicalRanges[i].end + 1;
                } else {
                    /* Overlapped part is shift mapping, do entire shift mapping */
                    needShiftMapping = gcvTRUE;
                }
            }

            flatBase = base;
            flatSize = (gctSIZE_T)(end - base);
        } else {
            /* it is already inside existing flat mapping ranges. */
            flatSize = 0;
        }

        if (!flatSize) {
            if (GpuBaseAddress)
                *GpuBaseAddress = (gctADDRESS)(Mmu->gpuAddressRanges[i].start + base - Mmu->gpuPhysicalRanges[i].start);

            return gcvSTATUS_OK;
        }
    }


    /* overwrite the original parameters */
    PhysBase = flatBase;

    if (GpuBaseAddress)
        reqVirtualBase = *GpuBaseAddress;

    /*
     * if no partcial physical range overlap to request entire shift mapping,
     * it is specific shift mapping or directly mapping by default.
     */
    if (!needShiftMapping) {
        flatVirtualBase = reqVirtualBase ?
                          reqVirtualBase : (gctADDRESS)flatBase;
    }

    for (i = 0; i < Mmu->gpuAddressRangeCount; i++) {
        if (_IsRangeInsected(flatVirtualBase, flatSize,
                             Mmu->gpuAddressRanges[i].start,
                             (gctSIZE_T)Mmu->gpuAddressRanges[i].size)) {
            virtualRangeOverlapped = gcvTRUE;
        }
    }

    /* If gpu virtual range overlapped or gpu physical over 4G, still need entire shift mapping */
    if ((!physicalRangeOverlapped && virtualRangeOverlapped) ||
        !Mmu->hardware->kernel->reserve32bitVA ||
        PhysBase + flatSize - 1 > ((1ULL << gcdMMU_VA_BITS) - 1)) {
        needShiftMapping = gcvTRUE;
    }

    if (needShiftMapping && !AbleToShift) {
        /*
         * Return without mapping any address.
         * By now, only physBase physSize could run here.
         */
        return gcvSTATUS_OK;
    }

    if (needShiftMapping || specificFlatMapping)
        mapFlag = gcvFLATMAP_SHIFT;

    specificFlatMapping = (reqVirtualBase && !virtualRangeOverlapped && !physicalRangeOverlapped);

    /************************ Setup flat mapping in non dynamic range. **************/
    gcmkONERROR(gckMMU_FillFlatMappingWithPage2M(Mmu, PhysBase, flatSize, Reserved,
                                                 needShiftMapping, specificFlatMapping,
                                                 reqVirtualBase, &baseAddress));

    if (GpuBaseAddress)
        *GpuBaseAddress = baseAddress;

    /* Store the gpu virtual ranges */
    Mmu->gpuAddressRanges[Mmu->gpuAddressRangeCount].start = baseAddress;
    Mmu->gpuAddressRanges[Mmu->gpuAddressRangeCount].end = baseAddress + flatSize - 1;
    Mmu->gpuAddressRanges[Mmu->gpuAddressRangeCount].size = flatSize;
    Mmu->gpuAddressRanges[Mmu->gpuAddressRangeCount].flag = mapFlag;
    Mmu->gpuAddressRangeCount++;

    gcmkASSERT(Mmu->gpuAddressRangeCount <= gcdMAX_FLAT_MAPPING_COUNT);

    /* Store the gpu physical ranges */
    Mmu->gpuPhysicalRanges[Mmu->gpuPhysicalRangeCount].start  = flatBase;
    Mmu->gpuPhysicalRanges[Mmu->gpuPhysicalRangeCount].end = flatBase + flatSize - 1;
    Mmu->gpuPhysicalRanges[Mmu->gpuPhysicalRangeCount].size = flatSize;
    Mmu->gpuPhysicalRanges[Mmu->gpuPhysicalRangeCount].flag = mapFlag;
    Mmu->gpuPhysicalRanges[Mmu->gpuPhysicalRangeCount].vStart = baseAddress;
    Mmu->gpuPhysicalRangeCount++;

    gcmkASSERT(Mmu->gpuPhysicalRangeCount <= gcdMAX_FLAT_MAPPING_COUNT);

    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
gckMMU_IsFlatMapped(gckMMU Mmu, gctUINT64 Physical,
                    gctSIZE_T Bytes, gctBOOL *In, gctADDRESS *Address)
{
    gceSTATUS status;
    gctUINT32 i;
    gctBOOL inFlatmapping = gcvFALSE;

    gcmkHEADER();

    gcmkVERIFY_ARGUMENT(In != gcvNULL);

    if (gckHARDWARE_IsFeatureAvailable(Mmu->hardware, gcvFEATURE_MMU) == gcvFALSE) {
        /* gcvSTATUS_NOT_SUPPORTED */
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    if (Address && *Address != gcvINVALID_ADDRESS) {
        if (*Address >= Mmu->dynamicLowArea4K.baseAddress && *Address < Mmu->dynamicLowArea4K.baseAddress + Mmu->dynamicLowArea4K.vaSize)
            goto Found;

        for (i = 0; i < Mmu->gpuAddressRangeCount; i++) {
            if ((*Address >= Mmu->gpuAddressRanges[i].start) &&
                (*Address + Bytes - 1 <= Mmu->gpuAddressRanges[i].end)) {
                inFlatmapping = gcvTRUE;
                goto Found;
            }
        }
    }

    if (Physical != gcvINVALID_PHYSICAL_ADDRESS) {
        for (i = 0; i < Mmu->gpuPhysicalRangeCount; i++) {
            if (Physical >= Mmu->gpuPhysicalRanges[i].start &&
                (Physical + Bytes - 1 <= Mmu->gpuPhysicalRanges[i].end)) {
                inFlatmapping = gcvTRUE;

                if (Address) {
                    *Address = Mmu->gpuPhysicalRanges[i].vStart +
                               (gctADDRESS)(Physical - Mmu->gpuPhysicalRanges[i].start);
                }

                break;
            }
        }
    }

Found:
    *In = inFlatmapping;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_SetupSRAM(gckMMU Mmu, gckHARDWARE Hardware, gckDEVICE Device)
{
    gctPHYS_ADDR_T reservedBase = gcvINVALID_PHYSICAL_ADDRESS;
    gctBOOL needMapInternalSRAM = gcvFALSE;
    gctUINT32 reservedSize = 0;
    gctINT i = 0;
    gctINT j = 0;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Mmu=%p Hardware=%p", Mmu, Hardware);

    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (!Mmu->sRAMMapped) {
        gctADDRESS address = gcvINVALID_ADDRESS;
        gctUINT32 size = 0;
        gctINT32 cursor = -1;

        /* Map all the SRAMs in MMU table. */
        for (i = 0; i < gcdCORE_3D_COUNT; i++) {
            for (j = gcvSRAM_INTERNAL0; j < gcvSRAM_INTER_COUNT; j++) {
                reservedBase = Device->sRAMBases[i][j];
                reservedSize = Device->sRAMSizes[i][j];

                Device->sRAMBaseAddresses[i][j] = 0;

                needMapInternalSRAM = reservedSize && (reservedBase != gcvINVALID_PHYSICAL_ADDRESS);

                /* Map the internal SRAM. */
                if (needMapInternalSRAM) {
                    if (Device->showSRAMMapInfo) {
                        gcmkPRINT("ftg340 Info: MMU mapped core%d SRAM base=0x%llx size=0x%x",
                            i, reservedBase, reservedSize);
                    }

                    /*
                     * Default gpu virtual base = 0.
                     * It can be specified if not conflict with existing mapping.
                     */
                    gcmkONERROR(gckMMU_FillFlatMapping(Mmu, reservedBase, reservedSize, gcvTRUE,
                        gcvTRUE, &Device->sRAMBaseAddresses[i][j]));

                } else if (reservedSize && reservedBase == gcvINVALID_PHYSICAL_ADDRESS) {
                    /*
                     * Reserve the internal SRAM range in first MMU pgd and
                     * set base to gcdRESERVE_START. If internal SRAM range is
                     * not specified, which means it is reserve usage.
                     */
                    if (cursor == -1)
                        cursor = i;

                    Device->sRAMBaseAddresses[i][j] = (i > cursor) ? Device->sRAMBaseAddresses[cursor][j] :
                                                      (address == gcvINVALID_ADDRESS) ? gcdRESERVE_START :
                                                      address + gcmALIGN(size, gcdRESERVE_ALIGN);

                    address = Device->sRAMBaseAddresses[i][j];

                    size = Device->sRAMSizes[i][j];

                    Device->sRAMPhysFaked[i][j] = gcvFALSE;
                } else {
                    gcmkASSERT(gcvSTATUS_NOT_SUPPORTED);
                }

#if gcdCAPTURE_ONLY_MODE
                Device->sRAMPhysFaked[i][j] = gcvTRUE;
#endif
            }
        }

        address = Mmu->reserveRangeSize;

        /* Map all the external SRAMs in MMU table. */
        for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
            if (Device->extSRAMSizes[i] &&
               (Device->extSRAMBases[i] != gcvINVALID_PHYSICAL_ADDRESS)) {
                gcmkONERROR(gckOS_CPUPhysicalToGPUPhysical(Mmu->os,
                                                           Device->extSRAMBases[i],
                                                           &Device->extSRAMGPUBases[i]));

                gcmkONERROR(gckMMU_FillFlatMapping(Mmu,
                                                   Device->extSRAMGPUBases[i],
                                                   Device->extSRAMSizes[i],
                                                   gcvFALSE,
                                                   gcvTRUE,
                                                   &address));

                Device->extSRAMBaseAddresses[i] = address;

                address += Device->extSRAMSizes[i];

                Device->extSRAMGPUPhysNames[i] =
                    gckKERNEL_AllocateNameFromPointer(Hardware->kernel, Device->extSRAMPhysicals[i]);
            }
        }

        /* The value of non-reserve range start address is equal with reserve range size. */
        Mmu->reserveRangeSize = (gctSIZE_T)address;
        Mmu->sRAMMapped = gcvTRUE;
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_GetPageEntry(gckMMU Mmu, gcePAGE_TYPE PageType, gctBOOL LowVA,
                    gctADDRESS Address, gctPOINTER *PageTable)
{
    gctUINT64_PTR pageTable;
    gctUINT32 index;
    gctUINT32 pgd, pud, pmd, pte;
    gcsADDRESS_AREA_PTR area = Mmu->dynamicAreaList;

    gcmkHEADER_ARG("Mmu=0x%llx Address=0x%llx", Mmu, Address);
    gcmkVERIFY_OBJECT(Mmu, gcvOBJ_MMU);

    if (LowVA && Mmu->hardware->kernel->reserve32bitVA) {
        area = &Mmu->dynamicLowArea4K;
    } else {
        while (area) {
            if (Address >= area->baseAddress && Address < area->baseAddress + area->vaSize)
                break;

            area = area->next;
        }
    }

    if (!area) {
        gcmkFOOTER_NO();
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    pgd = (gctUINT32)((Address & gcdMMU_PGD_MASK) >> gcdMMU_PGD_SHIFT);
    pud = (gctUINT32)((Address & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT);

    pmd = (gctUINT32)((Address & gcdMMU_PMD_MASK) >> gcdMMU_PMD_SHIFT);
    pte = (gctUINT32)((Address & gcdMMU_PTE_4K_MASK) >> gcdMMU_PTE_4K_SHIFT);

    pageTable = area->pteLogical;

    index = (((pgd - area->pgdStart) * gcdMMU_PUD_ENTRY_NUM + pud)
              * gcdMMU_PMD_ENTRY_NUM + pmd) * gcdMMU_PTE_4K_ENTRY_NUM + pte;

    *PageTable = pageTable + index;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckMMU_GetAreaType(gckMMU Mmu, gctADDRESS Address, gceAREA_TYPE *AreaType)
{
    gctBOOL flatMapped;
    gceSTATUS status = gcvSTATUS_OK;
    gcsADDRESS_AREA_PTR area = Mmu->dynamicAreaList;

    gcmkONERROR(gckMMU_IsFlatMapped(Mmu, gcvINVALID_PHYSICAL_ADDRESS,
                                    1, &flatMapped, &Address));

    if (flatMapped) {
        /* FlatMapping */
        *AreaType = gcvAREA_TYPE_FLATMAP;
    } else if (Address < gcdLOW4K_RESERVE_SIZE) {
        *AreaType = gcvAREA_TYPE_LOW_4K;
    } else {
        while (area) {
            if (Address >= area->baseAddress && Address < area->baseAddress + area->vaSize)
                break;

            area = area->next;
        }

        if (!area)
            *AreaType = gcvAREA_TYPE_UNKNOWN;
        else
            *AreaType = area->areaType;
    }
OnError:
    return status;
}

gceSTATUS
gckMMU_SwitchPgd(gckMMU dstMMU, gckMMU srcMMU)
{
    gceSTATUS status = gcvSTATUS_OK;
#if gcdDEBUG_MMU_SWITCH
    gctUINT32 i;
#endif

    gcmkHEADER();

#if gcdDEBUG_MMU_SWITCH
    for (i = 0; i < dstMMU->pgdSize / 4; i++) {
        gcmkPRINT("[ftg340}: dstMMU=%p pgd%d=0x%x before switch.\n",
                  dstMMU, i, *((gctUINT64 *)(dstMMU->pgdLogical + i)));
        gcmkPRINT("[ftg340]: srcMMU=%p pgd%d=0x%x before switch.\n",
                  srcMMU, i, *((gctUINT64 *)(srcMMU->pgdLogical + i)));
    }
#endif

    gcmkONERROR(gckOS_MemCopy(dstMMU->pgdLogical, srcMMU->pgdLogical, srcMMU->pgdSize));

    gcmkONERROR(gckOS_MemoryBarrier(dstMMU->os, dstMMU->pgdLogical));

    gcmkVERIFY_OK(gckMMU_Flush(dstMMU, gcvVIDMEM_TYPE_COMMAND));

#if gcdDEBUG_MMU_SWITCH
    for (i = 0; i < gcdMMU_PGD_ENTRY_NUM; i++) {
        gcmkPRINT("[ftg340]: dstMMU=%p pgd%d=0x%x after switch.\n",
                  dstMMU, i, *((gctUINT64 *)(dstMMU->pgdLogical + i)));
        gcmkPRINT("[ftg340]: srcMMU=%p pgd%d=0x%x after switch.\n",
                  srcMMU, i, *((gctUINT64 *)(srcMMU->pgdLogical + i)));
    }
#endif

OnError:
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_CopyUsedDynamicArea(gckOS Os, gcsADDRESS_AREA_PTR Area, gcsADDRESS_AREA_PTR AreaCopy)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 index;
    gctUINT32 total = Area->pteEntries;

    gcmkONERROR(gckOS_Allocate(Os, Area->pteSize, (gctPOINTER *)&AreaCopy->pteLogical));
    gcmkONERROR(gckOS_Allocate(Os, Area->mapSize, (gctPOINTER *)&AreaCopy->mapLogical));

    for (index = total - 1; index; index--) {
#if gcdUSE_MMU_EXCEPTION
        if (Area->pteLogical[index] == 0x2) {
#else
        if (!Area->pteLogical[index]) {
#endif
            Area->usedIndex = index + 1;
            break;
        }
    }

    if (!Area->usedIndex || Area->usedIndex >= total)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    gcmkONERROR(gckOS_MemCopy(AreaCopy->pteLogical + Area->usedIndex,
                              Area->pteLogical + Area->usedIndex,
                              (Area->pteSize - Area->usedIndex * 8)));

    gcmkONERROR(gckOS_MemCopy(AreaCopy->mapLogical, Area->mapLogical, Area->mapSize));

OnError:
    return status;
}

gceSTATUS
gckMMU_DestroyMmuCopy(gckMMU Mmu)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsADDRESS_AREA_PTR area;

    if (!Mmu)
        return gcvSTATUS_INVALID_ARGUMENT;

    if (Mmu->hardware->kernel->reserve32bitVA) {
        if (Mmu->dynamicLowArea4K.pteLogical)
            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, Mmu->dynamicLowArea4K.pteLogical));

        if (Mmu->dynamicLowArea4K.mapLogical)
            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, Mmu->dynamicLowArea4K.mapLogical));
    }

    area = Mmu->dynamicAreaList;

    while (area) {
        if (area->pteLogical)
            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, area->pteLogical));

        if (area->mapLogical)
            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, area->mapLogical));

        area = area->next;
    }

    gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, Mmu));

    return status;
}

gceSTATUS
gckMMU_ConstructMmuCopy(gckKERNEL Kernel, gckMMU *MmuCopy)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckMMU mmu = Kernel->mmu;
    gckMMU mmuCopy = gcvNULL;
    gcsADDRESS_AREA_PTR area, areaCopy;
    gctPOINTER pointer = gcvNULL;

    if (!mmu)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    /* Allocate memory for the gckMMU object. */
    gcmkONERROR(gckOS_Allocate(Kernel->os, sizeof(struct _gckMMU), &pointer));

    gckOS_ZeroMemory(pointer, sizeof(struct _gckMMU));

    mmuCopy = (gckMMU)pointer;
    mmuCopy->os = Kernel->os;

    if (Kernel->reserve32bitVA)
        gcmkONERROR(_CopyUsedDynamicArea(Kernel->os, &mmu->dynamicLowArea4K, &mmuCopy->dynamicLowArea4K));

    area = mmu->dynamicAreaList;

    while (area) {
        gcmkONERROR(gckOS_Allocate(Kernel->os, sizeof(gcsADDRESS_AREA), (gctPOINTER *)&areaCopy));
        gcmkONERROR(gckOS_MemCopy(areaCopy, area, gcmSIZEOF(gcsADDRESS_AREA)));

        gcmkONERROR(_CopyUsedDynamicArea(Kernel->os, area, areaCopy));

        if (mmuCopy->dynamicAreaList == gcvNULL)
            mmuCopy->dynamicAreaList = areaCopy;
        else {
            areaCopy->next = mmuCopy->dynamicAreaList;
            mmuCopy->dynamicAreaList = areaCopy;
        }

        area = area->next;
    }

    if (MmuCopy)
        *MmuCopy = mmuCopy;

OnError:
    return status;
}

gceSTATUS
gckMMU_CopyDynamicAreas(gckKERNEL Kernel, gckMMU dstMMU)
{
    gckMMU mmuCopy = Kernel->mmuCopy;
    gcsADDRESS_AREA_PTR dstArea, areaCopy;
    gceSTATUS status = gcvSTATUS_OK;
    gcsADDRESS_AREA_PTR lowArea = &dstMMU->dynamicLowArea4K;
    gcsADDRESS_AREA_PTR lowAreaCopy = &mmuCopy->dynamicLowArea4K;

    if (!mmuCopy || !dstMMU)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    if (Kernel->reserve32bitVA && lowArea && lowAreaCopy) {
        gcmkASSERT(lowArea->pteSize - lowAreaCopy->usedIndex * 8 > 0);

        gcmkONERROR(gckOS_MemCopy(lowArea->mapLogical,
                                  lowAreaCopy->mapLogical,
                                  lowArea->mapSize));

        gcmkONERROR(gckOS_MemCopy(lowArea->pteLogical + lowAreaCopy->usedIndex,
                                  lowAreaCopy->pteLogical + lowAreaCopy->usedIndex,
                                 (lowArea->pteSize - lowAreaCopy->usedIndex * 8)));
    }

    areaCopy = mmuCopy->dynamicAreaList;

    while (areaCopy) {
        gcmkONERROR(gckOS_Allocate(Kernel->os, gcmSIZEOF(gcsADDRESS_AREA), (gctPOINTER *)&dstArea));

        gcmkONERROR(gckOS_MemCopy(dstArea, areaCopy, gcmSIZEOF(gcsADDRESS_AREA)));

        gcmkONERROR(gckOS_Allocate(Kernel->os, dstArea->mapSize, (gctPOINTER *)&dstArea->mapLogical));

        gcmkONERROR(_ConstructDynamicPmdPte(dstMMU, dstArea));

        gcmkASSERT(dstArea->pteSize - areaCopy->usedIndex * 8 > 0);

        gcmkONERROR(gckOS_MemCopy(dstArea->mapLogical,
                                  areaCopy->mapLogical,
                                  dstArea->mapSize));

        gcmkONERROR(gckOS_MemCopy(dstArea->pteLogical + areaCopy->usedIndex,
                                  areaCopy->pteLogical + areaCopy->usedIndex,
                                  (dstArea->pteSize - areaCopy->usedIndex * 8)));

        if (dstMMU->dynamicAreaList == gcvNULL)
            dstMMU->dynamicAreaList = dstArea;
        else {
            dstArea->next = dstMMU->dynamicAreaList;
            dstMMU->dynamicAreaList = dstArea;
        }

        areaCopy = areaCopy->next;
    }

#if gcdDEBUG_MMU_SWITCH
    /* TODO: */
#endif

OnError:
    return status;
}

static gceSTATUS
_GetNextDescId(gckKERNEL Kernel, gctUINT32 *DescId)
{
    gctUINT32 id = Kernel->nextMmuDescId;
    gckKERNEL kernel = gcvNULL;
    gctUINT i;
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkVERIFY_ARGUMENT(Kernel != gcvNULL);

    gcmkONERROR(gckOS_AcquireMutex(Kernel->os, Kernel->mmuDescMutex, gcvINFINITE));
    acquired = gcvTRUE;

    do {
        /* Highly likely that 0~1 has already been used by the kernel. */
        if (++Kernel->nextMmuDescId == Kernel->totalMmuDescNum)
            Kernel->nextMmuDescId = 2;

        if (id == Kernel->nextMmuDescId)
            return gcvSTATUS_NOT_FOUND;

    } while (Kernel->mmuDescMap[Kernel->nextMmuDescId]);

    Kernel->mmuDescMap[Kernel->nextMmuDescId] = 1;

    /* Keep the descID of all cores the same.
     * And could change to per-core if need. */
    for (i = 0; i < Kernel->device->coreNum; i++) {
        kernel = Kernel->device->kernels[i];
        if (kernel)
            kernel->mmuDescMap[Kernel->nextMmuDescId] = 1;
    }

    *DescId = Kernel->nextMmuDescId;

    status = gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Kernel->os, Kernel->mmuDescMutex));

    return status;
}

gceSTATUS
gckMMU_DestroyProcessMMU(gckMMU Mmu)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Mmu=%p", Mmu);

    gcmkVERIFY_ARGUMENT(Mmu != gcvNULL);

    if (Mmu->descIndex) {
        gckKERNEL kernel = Mmu->hardware->kernel;
        gckDEVICE device = kernel->device;
        gctUINT i;

        gcmkONERROR(gckOS_AcquireMutex(kernel->os, kernel->mmuDescMutex, gcvINFINITE));

        for (i = 0; i < device->coreNum; i++) {
            if (device->kernels[i]) {
                device->kernels[i]->mmuDescMap[Mmu->descIndex] = 0;

                /* Will flush cache line when next switch page table. */
                device->kernels[i]->vmidWaitingFlush[Mmu->vmid] = 1;
            }
        }

        gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->mmuDescMutex));
    }

    gcmkVERIFY_OK(gckMMU_Destroy(Mmu));

    status = gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_ConstructProcessMMU(gckKERNEL Kernel, gctUINT32 ProcessID, gckMMU *Mmu)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckMMU mmu = gcvNULL;
    gckDEVICE device = Kernel->device;
    gctUINT i;

    gcmkHEADER_ARG("ProcessID=%d", ProcessID);

    /* Construct the gckMMU object. */
    gcmkONERROR(gckMMU_Construct(Kernel, &mmu));

    /* Flush PGD table. */
    gcmkONERROR(gckVIDMEM_NODE_CleanCache(Kernel,
                                          mmu->pgdVideoMem,
                                          0,
                                          mmu->pgdLogical,
                                          mmu->pgdSize));

    if (Kernel->command->pool == gcvPOOL_VIRTUAL || !Kernel->flatMapping)
        gcmkONERROR(gckMMU_CopyDynamicAreas(Kernel, mmu));

    if (Kernel->hardware->options.secureMode == gcvSECURE_IN_NORMAL && ProcessID) {
        gcmkONERROR(_GetNextDescId(Kernel, &mmu->descIndex));

        /* Specify the lower 4 bits as vmid. */
        mmu->vmid = mmu->descIndex & 0xF;

        for (i = 0; i < device->coreNum; i++) {
            if (device->kernels[i])
                gcmkONERROR(gckHARDWARE_FillMMUDescriptor(device->kernels[i]->hardware,
                                                          mmu->descIndex, mmu->pgdPhysical));
        }
    }

    *Mmu = mmu;

    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (mmu)
        gcmkVERIFY_OK(gckMMU_DestroyProcessMMU(mmu));

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_AttachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node)
{
    gcuVIDMEM_NODE_PTR node = gcvNULL;
    gctPOINTER pointer = gcvNULL;
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gctUINT index;

    gcmkHEADER_ARG("Mmu=%p Node=%p", Mmu, Node);

    if (!Mmu)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gcmkONERROR(gckOS_Allocate(Mmu->os, gcmSIZEOF(gcuVIDMEM_NODE), &pointer));

    gcmkVERIFY_OK(gckOS_ZeroMemory(pointer, gcmSIZEOF(gcuVIDMEM_NODE)));

    node = pointer;

    node->VidMem.id = Node->VidMem.id;
    index = 0;
    node->VidMem.lockeds[index]++;
    node->VidMem.addresses[index] = Node->VidMem.addresses[index];
    node->VidMem.pageTables[index] = Node->VidMem.pageTables[index];

    gcmkONERROR(gckOS_AcquireMutex(Mmu->os,
                                     Mmu->nodeListMutex,
                                     gcvINFINITE));
    acquired = gcvTRUE;

    gcsLIST_Add(&node->VidMem.lockLink, &Mmu->nodeList);

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->nodeListMutex));
    acquired = gcvFALSE;

    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->nodeListMutex));

    if (pointer) {
        /* Relese unuesd memory */
        gcmkOS_SAFE_FREE(Mmu->os, pointer);
    }

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckMMU_DetachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Mmu=%p Node=%p", Mmu, Node);

    if (!Mmu || !Node)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    gcmkONERROR(gckOS_AcquireMutex(Mmu->os,
                                   Mmu->nodeListMutex,
                                   gcvINFINITE));
    acquired = gcvTRUE;

    gcsLIST_Del(&Node->VidMem.lockLink);

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->nodeListMutex));
    acquired = gcvFALSE;

    gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Mmu->os, Node));

    status = gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Mmu->os, Mmu->nodeListMutex));

    gcmkFOOTER();
    return status;
}
#endif

