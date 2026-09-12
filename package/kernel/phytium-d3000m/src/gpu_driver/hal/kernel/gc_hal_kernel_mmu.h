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


#ifndef _gc_hal_kenrel_mmu_h_
#define _gc_hal_kenrel_mmu_h_

/*
 * mmu v1 for 40 physical address width and previous device.
 */

#if defined(EMULATOR)
# include "gcDefines.h"
#if defined(gcdVIRTUAL_ADDRESS_WIDTH) && (gcdVIRTUAL_ADDRESS_WIDTH == 40)
#  define gcdENABLE_40BIT_VA        1
#if defined(gcdMMU_EXTRA_TO_L1) && (gcdMMU_EXTRA_TO_L1 == 1)
#   define gcdMMU_40VA_40PA         0
#  else
#   define gcdMMU_40VA_40PA         1
#  endif
# endif
#endif

#ifndef gcdENABLE_40BIT_VA
# define gcdENABLE_40BIT_VA         0
#endif

#if !gcdENABLE_40BIT_VA
#undef gcdENABLE_FORCE_RESERVE_32BIT
#define gcdENABLE_FORCE_RESERVE_32BIT     0
#endif

#ifndef gcdMMU_40VA_40PA
# define gcdMMU_40VA_40PA           0
#endif

#define gcd4G_VA_FM_SIZE            0x40000000

#ifndef gcdDEBUG_MMU_SWITCH
#define gcdDEBUG_MMU_SWITCH         0
#endif

#ifndef gcdMMU_DESC_SIZE
#define gcdMMU_DESC_SIZE            (1 << 16)
#endif

/*******************************************************************************
 ***** New MMU Defination ******************************************************/

#if gcdENABLE_MMU_1KMODE
/* 1k mode */
# define gcdMMU_MTLB_SHIFT       24
# define gcdMMU_VA_BITS          32

#else /* gcdENABLE_MMU_1KMODE */
/* 4K mode */
# define gcdMMU_MTLB_SHIFT       22
# define gcdMMU_VA_BITS          32
#endif

#if gcdENABLE_40BIT_VA
# undef gcdMMU_VA_BITS
# undef gcdMMU_MTLB_SHIFT

#if gcdMMU_40VA_40PA
#  define gcdMMU_MTLB_SHIFT      30
# else
#  define gcdMMU_MTLB_SHIFT      24
# endif

# define gcdMMU_VA_BITS          40

#ifndef gcdDDR_SIZE_MAX
# define gcdDDR_SIZE_MAX         0x800000000
# endif
#endif

#define gcdMMU_STLB_4K_SHIFT        12
#define gcdMMU_STLB_64K_SHIFT       16
#define gcdMMU_STLB_1M_SHIFT        20
#define gcdMMU_STLB_16M_SHIFT       24

#define gcdMMU_MTLB_BITS            (gcdMMU_VA_BITS - gcdMMU_MTLB_SHIFT)
#define gcdMMU_PAGE_4K_BITS         gcdMMU_STLB_4K_SHIFT
#define gcdMMU_STLB_4K_BITS         (gcdMMU_VA_BITS - gcdMMU_MTLB_BITS - gcdMMU_PAGE_4K_BITS)
#define gcdMMU_PAGE_64K_BITS        gcdMMU_STLB_64K_SHIFT
#define gcdMMU_STLB_64K_BITS        (gcdMMU_VA_BITS - gcdMMU_MTLB_BITS - gcdMMU_PAGE_64K_BITS)
#define gcdMMU_PAGE_1M_BITS         gcdMMU_STLB_1M_SHIFT
#define gcdMMU_STLB_1M_BITS         (gcdMMU_VA_BITS - gcdMMU_MTLB_BITS - gcdMMU_PAGE_1M_BITS)
#define gcdMMU_PAGE_16M_BITS        gcdMMU_STLB_16M_SHIFT

#if gcdENABLE_40BIT_VA && gcdMMU_40VA_40PA
# define gcdMMU_STLB_16M_BITS    (gcdMMU_VA_BITS - gcdMMU_MTLB_BITS - gcdMMU_PAGE_16M_BITS)
#else
# define gcdMMU_STLB_16M_BITS    4
#endif

#ifndef gcdMMU_MTLB_ENTRY_NUM

#if defined(EMULATOR) && gcdENABLE_40BIT_VA
#if !gcdMMU_40VA_40PA
# define gcdMMU_MTLB_ENTRY_NUM   (2*1024)
# else
# define gcdMMU_MTLB_ENTRY_NUM   40
# endif
#elif gcdENABLE_40BIT_VA
# define gcdMMU_MTLB_ENTRY_NUM   ((gcdDDR_SIZE_MAX * 2) >> gcdMMU_MTLB_SHIFT)
#else
# define gcdMMU_MTLB_ENTRY_NUM   (1 << gcdMMU_MTLB_BITS)
#endif

#endif

/* this macro only used for flatmapping 16M page. */
#define gcdMMU_STLB_ALLOCATED_MAX   gcdMMU_MTLB_ENTRY_NUM >> gcdMMU_STLB_16M_BITS

#define gcdMMU_MTLB_SIZE            (gcdMMU_MTLB_ENTRY_NUM << 2)
#define gcdMMU_STLB_4K_ENTRY_NUM    (1 << gcdMMU_STLB_4K_BITS)
#define gcdMMU_STLB_4K_SIZE         (gcdMMU_STLB_4K_ENTRY_NUM << 2)
#define gcdMMU_PAGE_4K_SIZE         (1 << gcdMMU_STLB_4K_SHIFT)
#define gcdMMU_STLB_64K_ENTRY_NUM   (1 << gcdMMU_STLB_64K_BITS)
#define gcdMMU_STLB_64K_SIZE        (gcdMMU_STLB_64K_ENTRY_NUM << 2)
#define gcdMMU_PAGE_64K_SIZE        (1 << gcdMMU_STLB_64K_SHIFT)
#define gcdMMU_STLB_1M_ENTRY_NUM    (1 << gcdMMU_STLB_1M_BITS)
#define gcdMMU_STLB_1M_SIZE         (gcdMMU_STLB_1M_ENTRY_NUM << 2)
#define gcdMMU_PAGE_1M_SIZE         (1 << gcdMMU_STLB_1M_SHIFT)
#define gcdMMU_STLB_16M_ENTRY_NUM   (1 << gcdMMU_STLB_16M_BITS)
#define gcdMMU_STLB_16M_SIZE        (gcdMMU_STLB_16M_ENTRY_NUM << 2)
#define gcdMMU_PAGE_16M_SIZE        (1 << gcdMMU_STLB_16M_SHIFT)

#if gcdENABLE_40BIT_VA
# define gcdMMU_VA_MASK          ((1ULL << gcdMMU_VA_BITS) - 1)
# define gcdMMU_MTLB_MASK        (~((1ULL << gcdMMU_MTLB_SHIFT) - 1) & gcdMMU_VA_MASK)

# define gcdMMU_STLB_4K_MASK                                                                    \
     (((~0ULL << gcdMMU_STLB_4K_SHIFT) ^ gcdMMU_MTLB_MASK) & gcdMMU_VA_MASK)
# define gcdMMU_STLB_64K_MASK                                                                   \
     (((~((1ULL << gcdMMU_STLB_64K_SHIFT) - 1)) ^ gcdMMU_MTLB_MASK) & gcdMMU_VA_MASK)
# define gcdMMU_STLB_1M_MASK                                                                    \
     (((~((1ULL << gcdMMU_STLB_1M_SHIFT) - 1)) ^ gcdMMU_MTLB_MASK) & gcdMMU_VA_MASK)
# define gcdMMU_STLB_16M_MASK                                                                   \
     (((~((1ULL << gcdMMU_STLB_16M_SHIFT) - 1)) ^ gcdMMU_MTLB_MASK) & gcdMMU_VA_MASK)
#else
# define gcdMMU_MTLB_MASK        (~((1U << gcdMMU_MTLB_SHIFT) - 1))
# define gcdMMU_STLB_4K_MASK     ((~0U << gcdMMU_STLB_4K_SHIFT) ^ gcdMMU_MTLB_MASK)
# define gcdMMU_STLB_64K_MASK    ((~((1U << gcdMMU_STLB_64K_SHIFT) - 1)) ^ gcdMMU_MTLB_MASK)
# define gcdMMU_STLB_1M_MASK     ((~((1U << gcdMMU_STLB_1M_SHIFT) - 1)) ^ gcdMMU_MTLB_MASK)
# define gcdMMU_STLB_16M_MASK    0x0F000000
#endif

#define gcdMMU_PAGE_4K_MASK         (gcdMMU_PAGE_4K_SIZE - 1)
#define gcdMMU_PAGE_64K_MASK        (gcdMMU_PAGE_64K_SIZE - 1)
#define gcdMMU_PAGE_1M_MASK         (gcdMMU_PAGE_1M_SIZE - 1)
#define gcdMMU_PAGE_16M_MASK        (gcdMMU_PAGE_16M_SIZE - 1)

#define gcdMMU_MTLB_ENTRY_HINTS_BITS 6
#define gcdMMU_MTLB_ENTRY_STLB_MASK  (~((1U << gcdMMU_MTLB_ENTRY_HINTS_BITS) - 1))

#define gcdMMU_STLB_EXCEPTION       0x00000002

#define gcd1M_PAGE_SIZE             (1 << 20)
#define gcd1M_PAGE_SHIFT            20
#define gcd4K_PAGE_SIZE             (1 << 12)
#define gcdTRUNK_PAGE_SIZE          gcd1M_PAGE_SIZE
#define gcdTRUNK_PAGE_BIT           gcdMMU_PAGE_1M_BITS

#define gcd4G_SIZE                  0x100000000
#define gcdVA_RESERVED_SIZE         (16 << 20)

/* VIP SRAM start virtual address. */
#define gcdRESERVE_START            (4 << 20)

#define gcdRESERVE_ALIGN            (4 << 10)

#define gcmENTRY_TYPE(x)            ((x) & 0xF0)

#define gcmENTRY_COUNT(x)           (((x) & 0xFFFFFFFF00) >> 8)

#define gcdMMU_TABLE_DUMP           0

#define gcdVERTEX_START             (128 << 10)

#define gcdFLAT_MAPPING_MODE        gcvPAGE_TYPE_16M

typedef enum _gceMMU_TYPE {
    gcvMMU_USED   = (0 << 4),
    gcvMMU_SINGLE = (1 << 4),
    gcvMMU_FREE   = (2 << 4),
} gceMMU_TYPE;

typedef struct _gcsMMU_STLB_CHUNK *gcsMMU_STLB_CHUNK_PTR;

typedef struct _gcsMMU_STLB_CHUNK {
    gckVIDMEM_NODE        videoMem;
    gctUINT32_PTR         logical;
    gctSIZE_T             size;
    gctPHYS_ADDR_T        physBase;
    gctSIZE_T             pageCount;
    gctUINT32             mtlbIndex;
    gctUINT32             mtlbEntryNum;
    gcsMMU_STLB_CHUNK_PTR next;
} gcsMMU_STLB_CHUNK;

typedef struct _gcsFreeSpaceNode *gcsFreeSpaceNode_PTR;
typedef struct _gcsFreeSpaceNode {
    gctUINT32 start;
    gctUINT32 entries;
} gcsFreeSpaceNode;

#if gcdENDIAN_BIG

# define _WritePageEntry(pageEntry, entryValue) \
        (*(gctUINT32_PTR)(pageEntry) = gcmBSWAP32((gctUINT32)(entryValue)))

# define _ReadPageEntry(pageEntry) \
        gcmBSWAP32(*(gctUINT32_PTR)(pageEntry))

#else

# define _WritePageEntry(pageEntry, entryValue) \
        (*(gctUINT32_PTR)(pageEntry) = (gctUINT32)(entryValue))

# define _ReadPageEntry(pageEntry) \
        (*(gctUINT32_PTR)(pageEntry))

#endif

typedef enum _gceMMU_INIT_MODE {
    gcvMMU_INIT_FROM_REG,
    gcvMMU_INIT_FROM_CMD,
} gceMMU_INIT_MODE;

typedef struct _gcsADDRESS_AREA *gcsADDRESS_AREA_PTR;
typedef struct _gcsADDRESS_AREA {
    /* Page table / STLB table information. */
    gctSIZE_T                   stlbSize;
    gckVIDMEM_NODE              stlbVideoMem;
    gctUINT32_PTR               stlbLogical;
    gctUINT32                   stlbEntries;
    /* stlb physical address. */
    gctPHYS_ADDR_T              stlbPhysical;

    /* Free entries. */
    gctUINT32                   heapList;
    gctBOOL                     freeNodes;

    gceAREA_TYPE                areaType;

    gctUINT32                   mappingStart;
    gctUINT32                   mappingEnd;

    gctUINT64_PTR               mapLogical;

    gctUINT32                   usedIndex;
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

    /* Master TLB information. */
    gctSIZE_T                   mtlbSize;
    gckVIDMEM_NODE              mtlbVideoMem;
    gctUINT32_PTR               mtlbLogical;
    gctUINT32                   mtlbEntries;
    /* mtlb physical address. */
    gctPHYS_ADDR_T              mtlbPhysical;

    /* memory pool used for page table */
    gcePOOL                     pool;

    gctPOINTER                  staticSTLB;
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

    gcsADDRESS_AREA             dynamicArea1M;
    gcsADDRESS_AREA             dynamicArea4K;
    gcsADDRESS_AREA             dynamicLowArea1M;
    gcsADDRESS_AREA             dynamicLowArea4K;
    gcsADDRESS_AREA             secureArea;

    gctBOOL                     dynamicAreaSetuped;

    gctBOOL                     sRAMMapped;

    gctADDRESS                  contiguousBaseAddresses[gcdSYSTEM_RESERVE_COUNT];
    gctADDRESS                  externalBaseAddress;
    gctADDRESS                  internalBaseAddress;
    gctADDRESS                  exclusiveBaseAddress;

    gceMMU_INIT_MODE            initMode;
    gctBOOL                     pageTableOver4G;

    /* Flat mapping for reserve memory. */
    gctBOOL                     flatMapping;

    gcePAGE_TYPE                flatMappingMode;

#if !gcdMMU_40VA_40PA
    /* If the stlb is allocated when page size is 16M . */
    gctBOOL                     stlbAllocated[gcdMMU_STLB_ALLOCATED_MAX];
#endif

    /* The reserve size in page table. */
    gctSIZE_T                   reserveRangeSize;

    gcsLISTHEAD                 nodeList;
    gctPOINTER                  nodeListMutex;

    gctUINT32                   descIndex;
};

gceSTATUS
gckMMU_GetPageEntry(gckMMU Mmu, gcePAGE_TYPE PageType, gctBOOL LowVA,
                    gctADDRESS Address, gctPOINTER *PageTable);

gceSTATUS
gckMMU_SetupSRAM(gckMMU Mmu, gckHARDWARE Hardware, gckDEVICE Device);

gceSTATUS
gckMMU_SetupDynamicSpace(gckMMU Mmu);

void
gckMMU_DumpRecentFreedAddress(gckMMU Mmu);

gceSTATUS
gckMMU_DestroyProcessMMU(gckMMU Mmu);

gceSTATUS
gckMMU_ConstructProcessMMU(gckKERNEL Kernel, gctUINT32 ProcessID, gckMMU *Mmu);

gceSTATUS
gckMMU_CopyDynamicAreas(gckKERNEL Kernel, gckMMU dstMMU);

gceSTATUS
gckMMU_SwitchMtlb(gckMMU dstMMU, gckMMU srcMMU);

gceSTATUS
gckMMU_AttachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node);

gceSTATUS
gckMMU_DetachNode(gckMMU Mmu, gcuVIDMEM_NODE_PTR Node);

gceSTATUS
gckMMU_DestroyMmuCopy(gckMMU Mmu);

gceSTATUS
gckMMU_ConstructMmuCopy(gckKERNEL Kernel, gckMMU *MmuCopy);

#endif /* _gc_hal_kernel_mmu_h */
