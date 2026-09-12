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


#ifndef _gc_hal_kenrel_mmu_v2_h_
#define _gc_hal_kenrel_mmu_v2_h_

/*
 * mmu v2 for 48 physical address width device like gc8800 and gc8100DX.
 */

#if defined(EMULATOR)
# define gcdENABLE_40BIT_VA               1
#endif /* defined(EMULATOR) */

#ifndef gcdENABLE_40BIT_VA
# define gcdENABLE_40BIT_VA               0
#endif

#if !gcdENABLE_40BIT_VA
#undef gcdENABLE_FORCE_RESERVE_32BIT
#define gcdENABLE_FORCE_RESERVE_32BIT     0
#endif

#define gcd4G_VA_FM_SIZE        0x40000000

#ifndef gcdDEBUG_MMU_SWITCH
#define gcdDEBUG_MMU_SWITCH     0
#endif

#ifndef gcdMMU_DESC_SIZE
#define gcdMMU_DESC_SIZE        (1 << 16)
#endif

#ifndef gcdMMU_VMID_NUM
#define gcdMMU_VMID_NUM         (1 << 4)
#endif

#ifndef gcdDDR_SIZE_MAX
#define gcdDDR_SIZE_MAX         0x800000000
#endif

#ifndef gcdVA_BITS
#define gcdVA_BITS              40
#endif

#define gcdMMU_PTE_4K_SHIFT     12
#define gcdMMU_PTE_64K_SHIFT    16
#define gcdMMU_PTE_2M_SHIFT     21
#define gcdMMU_PMD_SHIFT        21
#define gcdMMU_PUD_SHIFT        30
#define gcdMMU_PGD_SHIFT        39

#define gcdMMU_PMD_BITS         9
#define gcdMMU_PUD_BITS         9

#define gcdMMU_VA_BITS          gcdVA_BITS

#define gcdMMU_PGD_BITS         (gcdMMU_VA_BITS - gcdMMU_PGD_SHIFT)

#define gcdMMU_PTE_PAGE_BITS    (gcdMMU_VA_BITS - gcdMMU_PGD_BITS - gcdMMU_PUD_BITS - gcdMMU_PMD_BITS)

#define gcdMMU_PAGE_4K_BITS     gcdMMU_PTE_4K_SHIFT
#define gcdMMU_PTE_4K_BITS      (gcdMMU_PTE_PAGE_BITS - gcdMMU_PAGE_4K_BITS)
#define gcdMMU_PAGE_64K_BITS    gcdMMU_PTE_64K_SHIFT
#define gcdMMU_PTE_64K_BITS     (gcdMMU_PTE_PAGE_BITS - gcdMMU_PAGE_64K_BITS)
#define gcdMMU_PAGE_2M_BITS     gcdMMU_PTE_2M_SHIFT
#define gcdMMU_PTE_2M_BITS      gcdMMU_PMD_BITS

#define gcdMMU_PGD_ENTRY_NUM    (1 << gcdMMU_PGD_BITS)
#define gcdMMU_PGD_SIZE         (gcdMMU_PGD_ENTRY_NUM << 3)
#define gcdMMU_PUD_ENTRY_NUM    (1 << gcdMMU_PUD_BITS)
#define gcdMMU_PUD_SIZE         (gcdMMU_PUD_ENTRY_NUM << 3)
#define gcdMMU_PMD_ENTRY_NUM    (1 << gcdMMU_PMD_BITS)
#define gcdMMU_PMD_SIZE         (gcdMMU_PMD_ENTRY_NUM << 3)

#define gcdMMU_PTE_4K_ENTRY_NUM    (1 << gcdMMU_PTE_4K_BITS)
#define gcdMMU_PTE_4K_SIZE         (gcdMMU_PTE_4K_ENTRY_NUM << 3)
#define gcdMMU_PAGE_4K_SIZE        (1 << gcdMMU_PTE_4K_SHIFT)
#define gcdMMU_PTE_64K_ENTRY_NUM   (1 << gcdMMU_PTE_64K_BITS)
#define gcdMMU_PTE_64K_SIZE        (gcdMMU_PTE_64K_ENTRY_NUM << 3)
#define gcdMMU_PAGE_64K_SIZE       (1 << gcdMMU_PTE_64K_SHIFT)
#define gcdMMU_PTE_2M_ENTRY_NUM    (1 << gcdMMU_PTE_2M_BITS)
#define gcdMMU_PTE_2M_SIZE         (gcdMMU_PTE_2M_ENTRY_NUM << 3)
#define gcdMMU_PAGE_2M_SIZE        (1 << gcdMMU_PTE_2M_SHIFT)

#define gcdMMU_VA_MASK             ((1ULL << gcdMMU_VA_BITS) - 1)
#define gcdMMU_PGD_MASK            (~0ULL << gcdMMU_PGD_SHIFT)
#define gcdMMU_PUD_MASK            (((~0ULL << gcdMMU_PUD_SHIFT) ^ gcdMMU_PGD_MASK) & gcdMMU_VA_MASK)
#define gcdMMU_PMD_MASK            \
    (((~0ULL << gcdMMU_PMD_SHIFT) ^ (~0ULL << gcdMMU_PUD_SHIFT)) & gcdMMU_VA_MASK)

#define gcdMMU_PTE_4K_MASK                                                                    \
    (((~0ULL << gcdMMU_PTE_4K_SHIFT) ^ (~0ULL << gcdMMU_PMD_SHIFT)) & gcdMMU_VA_MASK)
#define gcdMMU_PTE_64K_MASK                                                                   \
    (((~0ULL << gcdMMU_PTE_64K_SHIFT) ^ (~0ULL << gcdMMU_PMD_SHIFT)) & gcdMMU_VA_MASK)
#define gcdMMU_PTE_2M_MASK          gcdMMU_PMD_MASK

#define gcdMMU_PAGE_4K_MASK         (gcdMMU_PAGE_4K_SIZE - 1)
#define gcdMMU_PAGE_64K_MASK        (gcdMMU_PAGE_64K_SIZE - 1)
#define gcdMMU_PAGE_2M_MASK         (gcdMMU_PAGE_2M_SIZE - 1)

#define gcdMMU_ENTRY_ATTR_BITS 6
#define gcdMMU_ENTRY_ATTR_BITS_MASK (~((1ULL << gcdMMU_ENTRY_ATTR_BITS) - 1))

#define gcdMMU_BIT_PRESENT          0x00000001
#define gcdMMU_BIT_EXCEPTION        0x00000002
#define gcdMMU_BIT_4K_PAGE          (0 << 2)
#define gcdMMU_BIT_64K_PAGE         (1 << 2)
#define gcdMMU_BIT_2M_PAGE          (2 << 2)
#define gcdMMU_BIT_WRITEABLE        0x00000010

#define gcd2M_PAGE_SIZE             (1 << 21)
#define gcd2M_PAGE_SHIFT            21
#define gcd4K_PAGE_SIZE             (1 << 12)

#define gcdTRUNK_PAGE_SIZE          gcd2M_PAGE_SIZE
#define gcdTRUNK_PAGE_BIT           gcdMMU_PAGE_2M_BITS
#define gcdFLAT_MAPPING_MODE        gcvPAGE_TYPE_2M 

#define gcmkMMU_GET_ENTRY_NUM(type) (((type) == gcvAREA_TYPE_4K) ? \
                                    gcdMMU_PTE_4K_ENTRY_NUM :      \
                                    ((type) == gcvAREA_TYPE_64K) ? \
                                    gcdMMU_PTE_64K_ENTRY_NUM :     \
                                    gcdMMU_PTE_2M_ENTRY_NUM)

#define gcmkMMU_GET_PTE_SHIFT(type) (((type) == gcvAREA_TYPE_4K) ? \
                                    gcdMMU_PTE_4K_SHIFT :          \
                                    ((type) == gcvAREA_TYPE_64K) ? \
                                    gcdMMU_PTE_64K_SHIFT :         \
                                    gcdMMU_PTE_2M_SHIFT)

#define gcmkMMU_GET_AERA_TYPE(type) (((type) == gcvPAGE_TYPE_4K) ? \
                                    gcvAREA_TYPE_4K :              \
                                    ((type) == gcvPAGE_TYPE_64K) ? \
                                    gcvAREA_TYPE_64K :             \
                                    gcvAREA_TYPE_2M)

#define gcmkMMU_GET_PGD(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PGD_MASK) >> gcdMMU_PGD_SHIFT)
#define gcmkMMU_GET_PUD(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PUD_MASK) >> gcdMMU_PUD_SHIFT)
#define gcmkMMU_GET_PMD(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PMD_MASK) >> gcdMMU_PMD_SHIFT)
#define gcmkMMU_GET_PTE_4K(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PTE_4K_MASK) >> gcdMMU_PTE_4K_SHIFT)
#define gcmkMMU_GET_PTE_64K(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PTE_64K_MASK) >> gcdMMU_PTE_64K_SHIFT)
#define gcmkMMU_GET_PTE_2M(va) \
    (gctUINT32)((gctSIZE_T)((va) & gcdMMU_PTE_2M_MASK) >> gcdMMU_PTE_2M_SHIFT)

#define gcd4G_SIZE                  0x100000000
#define gcdVA_RESERVED_SIZE         (16 << 20)
#define gcdLOW4K_RESERVE_SIZE       (1 << 30)

/* VIP SRAM start virtual address. */
#define gcdRESERVE_START            (4 << 20)

#define gcdRESERVE_ALIGN            (4 << 10)

#define gcmENTRY_TYPE(x)            ((x) & 0xF0)

#define gcmENTRY_COUNT(x)           (((x) & 0xFFFFFFFF00) >> 8)

#define gcdMMU_TABLE_DUMP           0

#define gcdMMU_CONSTRUCT_1G_AREA    0
typedef enum _gceMMU_TYPE {
    gcvMMU_USED   = (0 << 4),
    gcvMMU_SINGLE = (1 << 4),
    gcvMMU_FREE   = (2 << 4),
} gceMMU_TYPE;

typedef struct _gcsMMU_PTE_CHUNK *gcsMMU_PTE_CHUNK_PTR;

typedef struct _gcsMMU_PTE_CHUNK {
    gckVIDMEM_NODE        videoMem;
    gctUINT32_PTR         logical;
    gctSIZE_T             size;
    gctPHYS_ADDR_T        physBase;
    gctSIZE_T             pageCount;
    gctUINT32             prevPTIndex;
    gctUINT32             prevPTEntryNum;
    gcsMMU_PTE_CHUNK_PTR  next;
} gcsMMU_PTE_CHUNK;

typedef struct _gcsFreeSpaceNode *gcsFreeSpaceNode_PTR;
typedef struct _gcsFreeSpaceNode {
    gctUINT32 start;
    gctUINT32 entries;
} gcsFreeSpaceNode;

#if gcdENDIAN_BIG

# define _WritePageEntry(pageEntry, entryValue) \
        (*(gctUINT64_PTR)(pageEntry) = gcmBSWAP64((gctUINT64)(entryValue)))

# define _ReadPageEntry(pageEntry) \
        gcmBSWAP64(*(gctUINT64_PTR)(pageEntry))

#else

# define _WritePageEntry(pageEntry, entryValue) \
        (*(gctUINT64_PTR)(pageEntry) = (gctUINT64)(entryValue))

# define _ReadPageEntry(pageEntry) \
        (*(gctUINT64_PTR)(pageEntry))

#endif

typedef enum _gceMMU_INIT_MODE {
    gcvMMU_INIT_FROM_REG,
    gcvMMU_INIT_FROM_CMD,
} gceMMU_INIT_MODE;

typedef struct _gcsADDRESS_AREA *gcsADDRESS_AREA_PTR;
typedef struct _gcsADDRESS_AREA {
    /* PGD table information. */
    gctUINT32                   pgdStart;
    gctUINT32                   pgdEnd;

    /* PUD table information. */
    gctUINT32                   pudStart;
    gctUINT32                   pudEnd;
    gctUINT32                   pudEntries;

    /* PMD table information. */
    gctSIZE_T                   pmdSize;
    gckVIDMEM_NODE              pmdVideoMem;
    gctUINT64_PTR               pmdLogical;
    gctUINT32                   pmdEntries;
    gctPHYS_ADDR_T              pmdPhysical;

    /* PTE table information. */
    gctSIZE_T                   pteSize;
    gckVIDMEM_NODE              pteVideoMem;
    gctUINT64_PTR               pteLogical;
    gctUINT32                   pteEntries;
    gctPHYS_ADDR_T              ptePhysical;

    gctADDRESS                  baseAddress;
    gctSIZE_T                   vaSize;

    /* Free entries. */
    gctUINT32                   heapList;
    gctBOOL                     freeNodes;

    gceAREA_TYPE                areaType;

    gctSIZE_T                   mapSize;
    gctUINT64_PTR               mapLogical;

    gctUINT32                   usedIndex;

    /* The processID which owned this dynamicArea. */
    gctUINT32                   processID;
    gcsADDRESS_AREA_PTR         next;
} gcsADDRESS_AREA;

/* gckMMU object. */
struct _gckMMU {
    /* The object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Pointer to gckHARDWARE object. */
    gckHARDWARE                 hardware;

    /* The page table mutex. */
    gctPOINTER                  pageTableMutex;

    /* PGD information. */
    gctSIZE_T                   pgdSize;
    gckVIDMEM_NODE              pgdVideoMem;
    gctUINT64_PTR               pgdLogical;
    gctPHYS_ADDR_T              pgdPhysical;
    gctPHYS_ADDR_T              mtlbPhysical;

    /* PUD information. */
    gctSIZE_T                   pudSize;
    gckVIDMEM_NODE              pudVideoMem;
    gctUINT64_PTR               pudLogical;
    gctUINT32                   pudEntries;
    gctPHYS_ADDR_T              pudPhysical;

    /* memory pool used for page table */
    gcePOOL                     pool;

    gctPOINTER                  staticPTE;
    gctBOOL                     enabled;

    gctSIZE_T                   safePageSize;
    gckVIDMEM_NODE              safePageVideoMem;
    gctPOINTER                  safePageLogical;
    gctADDRESS                  safeAddress;
    /* Safe page physical address. */
    gctPHYS_ADDR_T              safePagePhysical;

    /* GPU physical address flat mapping area. */
    gctUINT32                   gpuPhysicalRangeCount;
    gcsFLAT_MAPPING_RANGE       gpuPhysicalRanges[gcdMAX_FLAT_MAPPING_COUNT];

    /* GPU virtual address flat mapping area*/
    gctUINT32                   gpuAddressRangeCount;
    gcsFLAT_MAPPING_RANGE       gpuAddressRanges[gcdMAX_FLAT_MAPPING_COUNT];

    /* List of hardware which uses this MMU. */
    gcsLISTHEAD                 hardwareList;

    struct _gckQUEUE            recentFreedAddresses;

    gctBOOL                     sRAMMapped;

    gctADDRESS                  contiguousBaseAddresses[gcdSYSTEM_RESERVE_COUNT];
    gctADDRESS                  externalBaseAddress;
    gctADDRESS                  internalBaseAddress;
    gctADDRESS                  exclusiveBaseAddress;

    gceMMU_INIT_MODE            initMode;

    /* Flat mapping for reserve memory. */
    gctBOOL                     flatMapping;

    gcePAGE_TYPE                flatMappingMode;

    /* The reserve size in page table. */
    gctSIZE_T                   reserveRangeSize;

    gcsLISTHEAD                 nodeList;
    gctPOINTER                  nodeListMutex;

    gctUINT32                   descIndex;
    gctUINT32                   vmid;

    gcsADDRESS_AREA             dynamicLowArea4K;
    gctPOINTER                  dynamicAreaList;
};

gceSTATUS
gckMMU_GetPageEntry(gckMMU Mmu, gcePAGE_TYPE PageType, gctBOOL LowVA,
                    gctADDRESS Address, gctPOINTER *PageTable);

gceSTATUS
gckMMU_SetupSRAM(gckMMU Mmu, gckHARDWARE Hardware, gckDEVICE Device);

void
gckMMU_DumpRecentFreedAddress(gckMMU Mmu);

gceSTATUS
gckMMU_DestroyProcessMMU(gckMMU Mmu);

gceSTATUS
gckMMU_ConstructProcessMMU(gckKERNEL Kernel, gctUINT32 ProcessID, gckMMU *Mmu);

gceSTATUS
gckMMU_CopyDynamicAreas(gckKERNEL Kernel, gckMMU dstMMU);

gceSTATUS
gckMMU_SwitchPgd(gckMMU dstMMU, gckMMU srcMMU);

gceSTATUS
gckMMU_AttachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node);

gceSTATUS
gckMMU_DetachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node);

gceSTATUS
gckMMU_DestroyMmuCopy(gckMMU Mmu);

gceSTATUS
gckMMU_ConstructMmuCopy(gckKERNEL Kernel, gckMMU *MmuCopy);

#endif /* _gc_hal_kernel_mmu_v2_h */
