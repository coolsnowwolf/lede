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


#define gcdRECOVERY_FORCE_TIMEOUT     100

#ifndef gcdENABLE_VIRTUAL_POOL
#define gcdENABLE_VIRTUAL_POOL        1
#endif

#ifndef gcdDYNAMIC_ALLOC_LOCAL_MEMORY
#define gcdDYNAMIC_ALLOC_LOCAL_MEMORY 0
#endif

#ifndef gcdSUSPEND_RESUME_FROM_DISK
#define gcdSUSPEND_RESUME_FROM_DISK   1
#endif

/*******************************************************************************
 ***** Stuck Dump Level ********************************************************/

/*
 * Stuck Dump Level
 *
 * Level  1 ~  5 : NORMAL model, when recovery is enabled, stuck dump is invalid
 * Level 11 ~ 15 : FORCE model, whether recovery is enabled or not, driver will
 *                 dump as level set
 *
 * NONE : Dump nothing when stuck happens.
 *
 * NEARBY_MEMORY : Dump GPU state and memory near stuck point.
 * USER_COMMAND  : Beside NEARBY_MEMORY, dump context buffer and user command
 *                 buffer.
 * STALL_COMMAND : Beside USER_COMMAND, commit will be stall to make sure command
 *                 causing stuck isn't missed.
 * ALL_COMMAND   : Beside USER_COMMAND, dump kernel command buffer.
 * ALL_CORE      : Dump all the cores with ALL_COMMAND level.
 */
typedef enum _gceSTUCK_DUMP_LEVEL {
    gcvSTUCK_DUMP_NONE = 0,

    gcvSTUCK_DUMP_NEARBY_MEMORY = 1,
    gcvSTUCK_DUMP_USER_COMMAND,
    gcvSTUCK_DUMP_STALL_COMMAND,
    gcvSTUCK_DUMP_ALL_COMMAND,
    gcvSTUCK_DUMP_ALL_CORE,

    gcvSTUCK_FORCE_DUMP_NEARBY_MEMORY = 11,
    gcvSTUCK_FORCE_DUMP_USER_COMMAND,
    gcvSTUCK_FORCE_DUMP_STALL_COMMAND,
    gcvSTUCK_FORCE_DUMP_ALL_COMMAND,
    gcvSTUCK_FORCE_DUMP_ALL_CORE,
} gceSTUCK_DUMP_LEVEL;

/******************************************************************************
 ***** Page table *************************************************************/

#define gcvPAGE_TABLE_DIRTY_BIT_OTHER (1 << 0)
#define gcvPAGE_TABLE_DIRTY_BIT_FE    (1 << 1)

/******************************************************************************
 ***** GPU Virtualization ****************************************************/

typedef enum _gceVGPU_TYPE {
    gcvVGPU_NONE = 0,
    gcvVGPU_MDEV,
    gcvVGPU_SRIOV,
} gceVGPU_TYPE;

/******************************************************************************
 ***** Process Database Management ********************************************/

typedef enum _gceDATABASE_TYPE {
    gcvDB_VIDEO_MEMORY = 1,    /* Video memory created. */
    gcvDB_COMMAND_BUFFER,      /* Command Buffer. */
    gcvDB_NON_PAGED,           /* Non paged memory. */
    gcvDB_CONTIGUOUS,          /* Contiguous memory. */
    gcvDB_SIGNAL,              /* Signal. */
    gcvDB_VIDEO_MEMORY_LOCKED, /* Video memory locked. */
    gcvDB_CONTEXT,             /* Context */
    gcvDB_IDLE,                /* GPU idle. */
    gcvDB_MAP_MEMORY,          /* Map memory */
    gcvDB_SHBUF,               /* Shared buffer. */
#if gcdENABLE_SW_PREEMPTION
    gcvDB_PRIORITY,
#endif
#if gcdENABLE_CLEAR_FENCE
    gcvDB_USER_FENCE,
#endif

    gcvDB_NUM_TYPES,
} gceDATABASE_TYPE;

typedef enum _gceLARGE_VA_VERSION {
    gcv32BIT_VA_40BIT_PA,
    /* 8bit or 10bit master table index. */
    gcv40BIT_VA_40BIT_PA_0,
    /* 16bit or 18bit master table index. */
    gcv40BIT_VA_40BIT_PA_1,
    /* 48bit PA. */
    gcv40BIT_VA_48BIT_PA_0,
    /* Used for 32bit hw co-work with 40bit hw. */
    gcv40BIT_VA_32BIT_PA,
} gceLARGE_VA_VERSION;

#define gcdDATABASE_TYPE_MASK           0x000000FF
#define gcdDB_VIDEO_MEMORY_TYPE_MASK    0x0000FF00
#define gcdDB_VIDEO_MEMORY_TYPE_SHIFT   8

#define gcdDB_VIDEO_MEMORY_POOL_MASK    0x00FF0000
#define gcdDB_VIDEO_MEMORY_POOL_SHIFT   16

#define gcdDB_VIDEO_MEMORY_DBTYPE_MASK  0xFF000000
#define gcdDB_VIDEO_MEMORY_DBTYPE_SHIFT 24

typedef struct _gcsDATABASE_RECORD *gcsDATABASE_RECORD_PTR;
typedef struct _gcsDATABASE_RECORD {
    /* Pointer to kernel. */
    gckKERNEL                           kernel;

    /* Pointer to next database record. */
    gcsDATABASE_RECORD_PTR              next;

    /* Type of record. */
    gceDATABASE_TYPE                    type;

    /* Data for record. */
    gctPOINTER                          data;
    gctPHYS_ADDR                        physical;
    gctSIZE_T                           bytes;
} gcsDATABASE_RECORD;

#if gcdENABLE_PERF_DISPATCH
typedef struct _gcsDISPATCH_PERF_RECORD {
    gctUINT64 count;
    gctUINT64 failed;

    gctUINT64 cost;
    gctUINT64 maximum;

    gctPOINTER mutex;
} gcsDISPATCH_PERF_RECORD;
#endif

typedef struct _gcsDATABASE            *gcsDATABASE_PTR;
typedef struct _gcsDATABASE {
    /* Pointer to next entry is hash list. */
    gcsDATABASE_PTR                     next;
    gctSIZE_T                           slot;

    /* Process ID. */
    gctUINT32                           processID;

    /* Open-Close ref count */
    gctPOINTER                          refs;

    /* Already mark for delete and cannot reenter */
    gctBOOL                             deleted;

    /* Sizes to query. */
    gcsDATABASE_COUNTERS                vidMem;
    gcsDATABASE_COUNTERS                nonPaged;
    gcsDATABASE_COUNTERS                contiguous;
    gcsDATABASE_COUNTERS                mapMemory;

    gcsDATABASE_COUNTERS                vidMemType[gcvVIDMEM_TYPE_COUNT];
    /* Counter for each video memory pool. */
    gcsDATABASE_COUNTERS                vidMemPool[gcvPOOL_NUMBER_OF_POOLS];
    gctPOINTER                          counterMutex;

    /* Idle time management. */
    gctUINT64                           lastIdle;
    gctUINT64                           idle;

    /* Pointer to database. */
    gcsDATABASE_RECORD_PTR              list[48];

    gctPOINTER                          handleDatabase;
    gctPOINTER                          handleDatabaseMutex;

    /* Per process mmu. */
    gckMMU                              mmu;

#if gcdENABLE_PERF_DISPATCH
    gcsDISPATCH_PERF_RECORD     dispatchPerfRecords[gcvHAL_NUM_COMMAND_CODES];
#endif
} gcsDATABASE;

typedef struct _gckLINKDATA *gckLINKDATA;
struct _gckLINKDATA {
    gctADDRESS                  start;
    gctADDRESS                  end;
    gctUINT32                   pid;
    gctUINT64                   linkLow;
    gctUINT64                   linkHigh;
};

typedef struct _gckADDRESSDATA *gckADDRESSDATA;
struct _gckADDRESSDATA {
    gctADDRESS                  start;
    gctADDRESS                  end;
};

typedef union _gcuQUEUEDATA {
    struct _gckLINKDATA         linkData;

    struct _gckADDRESSDATA      addressData;
} gcuQUEUEDATA;

typedef struct _gckQUEUE *gckQUEUE;
struct _gckQUEUE {
    gcuQUEUEDATA                *datas;
    gctUINT32                   rear;
    gctUINT32                   front;
    gctUINT32                   count;
    gctUINT32                   size;
};

typedef struct _gcsLISTHEAD *gcsLISTHEAD_PTR;
typedef struct _gcsLISTHEAD {
    gcsLISTHEAD_PTR             prev;
    gcsLISTHEAD_PTR             next;
} gcsLISTHEAD;

typedef struct _gcsFDPRIVATE *gcsFDPRIVATE_PTR;
typedef struct _gcsFDPRIVATE {
    gctINT (*release)(gcsFDPRIVATE_PTR Private);
} gcsFDPRIVATE;

typedef struct _gcsRECORDER *gckRECORDER;

typedef struct _gcsPARSER *gckPARSER;

typedef struct _gcsPARSER_HANDLER *gckPARSER_HANDLER;

typedef enum _gceEVENT_FAULT {
    gcvEVENT_NO_FAULT,
    gcvEVENT_BUS_ERROR_FAULT,
} gceEVENT_FAULT;

/* Reserved for kernel option. */
/*
 *   When enabled, use 1K mode for MMU version 1.0. otherwise use 4K mode.
 */
#ifndef gcdENABLE_MMU_1KMODE
#define gcdENABLE_MMU_1KMODE                    1
#endif

#ifndef gcdMMU_VERSION_2
#if defined(EMULATOR)
#include "gcDefines.h"
#if defined(gcdPHYSICAL_ADDRESS_WIDTH) && (gcdPHYSICAL_ADDRESS_WIDTH == 48)
#define gcdMMU_VERSION_2                        1
#endif /* gcdPHYSICAL_ADDRESS_WIDTH == 48 */
#endif /* defined(EMULATOR) */
#endif /* gcdMMU_VERSION_2 */

#ifndef gcdMMU_VERSION_2
#define gcdMMU_VERSION_2                        0
#endif

#ifndef gcdMMU_VERSION_1
#if gcdMMU_VERSION_2
#  define gcdMMU_VERSION_1                      0
# else
#  define gcdMMU_VERSION_1                      1
# endif
#endif

 /*
  *   gcdENABLE_GPU_1M_PAGE
  *       When non-zero, GPU page size will be 1M until the pool is out of memory
  *       and low-level to 4K pages. When zero, it uses 4k GPU pages.
  */
#if gcdMMU_VERSION_2
#define gcdENABLE_GPU_1M_PAGE                   0
#endif

#ifndef gcdENABLE_GPU_1M_PAGE
#if !gcdSECURITY && defined(LINUX)
#ifdef EMULATOR
#   define gcdENABLE_GPU_1M_PAGE                0
#  else
#   define gcdENABLE_GPU_1M_PAGE                1
#  endif
# else
#  define gcdENABLE_GPU_1M_PAGE                 0
# endif
#endif

/*
 *  gcdENABLE_FORCE_RESERVE_32BIT
 *      When non-zero, kernel will force reserve 32bit memory, that means if user
 *      set lowVA flag as 1 when try to lock buffer, kernel will generate a
 *      32-bit-width address.
 *
 *      This macro might be set back to 0 in gc_hal_kernel_mmu.h, because we should
 *      check the macro gcdENABLE_40BIT_VA which defined in that header file in cmodel.
 */
#ifndef gcdENABLE_FORCE_RESERVE_32BIT
# define gcdENABLE_FORCE_RESERVE_32BIT          1
#endif
