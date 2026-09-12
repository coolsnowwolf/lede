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


#ifndef __gc_hal_kernel_hardware_h_
#define __gc_hal_kernel_hardware_h_

#include "gc_hal_kernel_hardware_func.h"

#ifdef __cplusplus
extern "C" {
#endif

#define EVENT_ID_INVALIDATE_PIPE 29

#define gcmSEMAPHORESTALL(buffer)                                                                       \
    do {                                                                                                \
        /* Arm the PE-FE Semaphore. */                                                                  \
        *(buffer)++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |         \
                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |                  \
                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs); \
                                                                                                        \
        *(buffer)++ = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE,      FRONT_END) |                       \
                      gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);                     \
                                                                                                        \
        /* STALL FE until PE is done flushing. */                                                       \
        *(buffer)++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);                                \
                                                                                                        \
        *(buffer)++ = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE,      FRONT_END) |                        \
                      gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE);                      \
    } while (0)

typedef enum {
    gcvHARDWARE_FUNCTION_MMU,
    gcvHARDWARE_FUNCTION_FLUSH,
    gcvHARDWARE_FUNCTION_NUM,
} gceHARDWARE_FUNCTION;

typedef struct _gckASYNC_FE *gckASYNC_FE;
typedef struct _gckWLFE     *gckWLFE;
typedef struct _gckMCFE     *gckMCFE;

typedef struct _gcsSTATETIMER {
    gctUINT64                   start;
    gctUINT64                   recent;

    /* Elapse of each power state. */
    gctUINT64                   elapse[4];
} gcsSTATETIMER;

typedef struct _gcsHARDWARE_SIGNATURE {
    /* Chip model. */
    gceCHIPMODEL                chipModel;

    /* Revision value.*/
    gctUINT32                   chipRevision;

    /* Supported feature fields. */
    gctUINT32                   chipFeatures;

    /* Supported minor feature fields. */
    gctUINT32                   chipMinorFeatures;

    /* Supported minor feature 1 fields. */
    gctUINT32                   chipMinorFeatures1;

    /* Supported minor feature 2 fields. */
    gctUINT32                   chipMinorFeatures2;
} gcsHARDWARE_SIGNATURE;

typedef struct _gcsMMU_TABLE_ARRAY_ENTRY {
    gctUINT32                   low;
    gctUINT32                   high;
} gcsMMU_TABLE_ARRAY_ENTRY;

typedef struct _gcsHARDWARE_PAGETABLE_ARRAY {
    /* Number of entries in page table array. */
    gctUINT                     num;

    /* Video memory node. */
    gckVIDMEM_NODE              videoMem;

    /* Size in bytes of array. */
    gctSIZE_T                   size;

    /* Physical address of array. */
    gctPHYS_ADDR_T              address;

    /* Logical address of array. */
    gctPOINTER                  logical;
} gcsHARDWARE_PAGETABLE_ARRAY;

/* gckHARDWARE object. */
struct _gckHARDWARE {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gctKERNEL object. */
    gckKERNEL                   kernel;

    /* Pointer to gctOS object. */
    gckOS                       os;

    /* Core */
    gceCORE                     core;

    /* Type */
    gceHARDWARE_TYPE            type;

    /* Chip characteristics. */
    gcsHAL_QUERY_CHIP_IDENTITY  identity;
    gcsHAL_QUERY_CHIP_OPTIONS   options;
    gctUINT32                   powerBaseAddress;
    gctBOOL                     extraEventStates;

    /* Big endian */
    gctBOOL                     bigEndian;

    /* Base address. */
    gctUINT32                   baseAddress;

    /* FE modules. */
    gckWLFE                     wlFE;
    gckASYNC_FE                 asyncFE;
    gckMCFE                     mcFE;

    /* Chip status */
    gctPOINTER                  powerMutex;
    gceCHIPPOWERSTATE           chipPowerState;
    gctBOOL                     clockState;
    gctBOOL                     powerState;
    gctPOINTER                  globalSemaphore;
    gctBOOL                     isLastPowerGlobal;

    /* Wait Link FE only. */
    gctADDRESS                  lastWaitLink;
    gctADDRESS                  lastEnd;

#if gcdPOWEROFF_TIMEOUT
    gceCHIPPOWERSTATE           nextPowerState;
    gctPOINTER                  powerStateTimer;
#endif

#if gcdENABLE_FSCALE_VAL_ADJUST
    gctUINT32                   powerOnFscaleVal;
    gctUINT32                   powerOnShaderFscaleVal;
#endif
    gctPOINTER                  pageTableDirty[gcvENGINE_GPU_ENGINE_COUNT];

#if gcdLINK_QUEUE_SIZE
    struct _gckQUEUE            linkQueue;
#endif
    gctBOOL                     stallFEPrefetch;

    gctUINT32                   minFscaleValue;
    gctUINT                     waitCount;

    gctUINT64                   mcClk;
    gctUINT64                   shClk;

    gctPOINTER                  pendingEvent;
    gctPOINTER                  curClusterMask;

    gcsFUNCTION_EXECUTION_PTR   functions;

    gcsSTATETIMER               powerStateCounter;
    gctUINT32                   executeCount;
    gctADDRESS                  lastExecuteAddress;

    /* Head for hardware list in gckMMU. */
    gcsLISTHEAD                 mmuHead;

    /* Internal SRAMs info. */
    gckVIDMEM                   sRAMVidMem[gcvSRAM_INTER_COUNT];
    gctPHYS_ADDR                sRAMPhysical[gcvSRAM_INTER_COUNT];

    gctPOINTER                  featureDatabase;
    gctBOOL                     hasL2Cache;

    /* MCFE channel bindings, temporary. */
    gceMCFE_CHANNEL_TYPE        mcfeChannels[64];
    gctUINT32                   mcfeChannelCount;

    gcsHARDWARE_SIGNATURE       signature;

    gctUINT32                   maxOutstandingReads;

    gcsHARDWARE_PAGETABLE_ARRAY pagetableArray;

    gctUINT64                   contextID;

    gctBOOL                     hasQchannel;

    gctUINT32                   powerOffTimeout;

    gctUINT32                   devID;

    gceLARGE_VA_VERSION         largeVAVersion;

    gctBOOL                     graphicsLargeVA;

    gctUINT                     aliveCluster0Index;

    gctUINT                     clusterCount;
};

gceSTATUS
gckHARDWARE_GetFrameInfo(gckHARDWARE Hardware, gcsHAL_FRAME_INFO *FrameInfo);

gceSTATUS
gckHARDWARE_DumpGpuProfile(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_HandleFault(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_ExecuteFunctions(gcsFUNCTION_EXECUTION_PTR Execution);

gceSTATUS
gckHARDWARE_DummyDraw(gckHARDWARE Hardware,
                      gctPOINTER Logical,
                      gctADDRESS Address,
                      gceDUMMY_DRAW_TYPE DummyDrawType,
                      gctUINT32 *Bytes);

gceSTATUS
gckHARDWARE_EnterQueryClock(gckHARDWARE Hardware, gctUINT64 *McStart, gctUINT64 *ShStart);

gceSTATUS
gckHARDWARE_ExitQueryClock(gckHARDWARE Hardware,
                           gctUINT64 McStart,
                           gctUINT64 ShStart,
                           gctUINT64 *McClk,
                           gctUINT64 *ShClk);

gceSTATUS
gckHARDWARE_QueryFrequency(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_QueryFScale(gckHARDWARE Hardware, gctUINT8 *Scale);

gceSTATUS
gckHARDWARE_SetClock(gckHARDWARE Hardware, gctUINT32 MCScale, gctUINT32 SHScale);

gceSTATUS
gckHARDWARE_PowerControlClusters(gckHARDWARE Hardware,
                                 gctUINT32 PowerControlValue,
                                 gctBOOL PowerState);

gceSTATUS
gckHARDWARE_QueryCycleCount(gckHARDWARE Hardware,
                            gctUINT32 *hi_total_cycle_count,
                            gctUINT32 *hi_total_idle_cycle_count);

gceSTATUS
gckHARDWARE_CleanCycleCount(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_QueryCoreLoad(gckHARDWARE Hardware,
                          gctUINT32 Mdelay,
                          gctUINT32 *Load);

gceSTATUS
gckHARDWARE_QueryClusterInfo(gckHARDWARE Hardware, gctUINT32 *ClusterMask);

gceSTATUS
gckHARDWARE_ConfigCluster(gckHARDWARE Hardware, gctUINT32 ClusterMask);

gceSTATUS
gckHARDWARE_UpdateClusterConfigContext(gckHARDWARE Hardware, gctUINT32 *Buffer, gctUINT32 BufferSize);

gceSTATUS
gckHARDWARE_CancelJob(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_FlushCache(gckHARDWARE Hardware, gckCOMMAND Command);

#if gcdENABLE_AHBXTTD
gceSTATUS
gckHARDWARE_ResetAHBXTTD(gckHARDWARE Hardware, gckKERNEL Kernel);
#endif

gceSTATUS
gckHARDWARE_FillMMUDescriptor(gckHARDWARE Hardware,
                              gctUINT32 Index,
                              gctPHYS_ADDR_T MtlbPhysical);

gceSTATUS
gckHARDWARE_SwitchPageTableId(gckHARDWARE Hardware, gckMMU Mmu,
                              gckCOMMAND Command);

gceSTATUS
gckHARDWARE_ResumeWLFE(gckHARDWARE Hardware);

#define gcmkWRITE_MEMORY(logical, data)                                       \
    do {                                                                      \
        gcmkVERIFY_OK(gckOS_WriteMemory(os, logical, data));                  \
        logical++;                                                            \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_hardware_h_ */
