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


#include "gc_hal.h"
#include "gc_hal_kernel.h"
#include "AQ.h"
#include "gc_hal_kernel_hardware.h"

#define _GC_OBJ_ZONE    gcvZONE_HARDWARE

static gceSTATUS
_FuncExecute(gcsFUNCTION_EXECUTION_PTR Execution)
{
    return gckHARDWARE_ExecuteFunctions(Execution);
}

static gceSTATUS
_FuncValidate_MMU(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (hardware->options.enableMMU &&
        hardware->options.secureMode != gcvSECURE_IN_TA) {
        Execution->valid = gcvTRUE;
    } else {
        Execution->valid = gcvFALSE;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncRelease_MMU(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gctUINT8    i;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (Execution->funcCmd) {
        for (i = 0; i < Execution->funcCmdCount; i++) {
            if (Execution->funcCmd[i].funcVidMem) {
                if (Execution->funcCmd[i].logical) {
                    gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(hardware->kernel,
                                                           Execution->funcCmd[i].funcVidMem,
                                                           0, gcvFALSE, gcvFALSE));
                    Execution->funcCmd[0].logical = gcvNULL;
                }

                gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(hardware->kernel,
                                                         Execution->funcCmd[i].funcVidMem));
                Execution->funcCmd[i].funcVidMem = gcvNULL;
            }
        }

        gcmkVERIFY_OK(gckOS_Free(hardware->os, (gctPOINTER)Execution->funcCmd));
        Execution->funcCmd = gcvNULL;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_ProgramMMUStates(gckHARDWARE Hardware, gckMMU Mmu,
                  gceMMU_MODE Mode, gctPOINTER Logical, gctUINT32 *Bytes)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 config, address;
    gctUINT32 extMtlb, extSafeAddress, configEx = 0;
    gctPHYS_ADDR_T physical;
    gctUINT32_PTR buffer;
    gctBOOL ace;
    gctUINT32 reserveBytes = 0;
    gctBOOL config2D;
    gctUINT i;
    gctUINT probeSelectCount = 4;
    gctBOOL multiCluster = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    ace = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_ACE);

    switch (Hardware->options.secureMode) {
    case gcvSECURE_IN_NORMAL:
        reserveBytes = 8 + 4 * 4;
        break;
    case gcvSECURE_NONE:
        reserveBytes = 16 + 4 * 4;
        if (ace)
            reserveBytes += 8;
        break;
    case gcvSECURE_IN_TA:
    default:
        gcmkASSERT(gcvFALSE);
        gcmkPRINT("%s(%d): secureMode is wrong", __FUNCTION__, __LINE__);
        break;
    }

    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    config2D = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PIPE_3D) &&
               gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PIPE_2D);

    if (config2D) {
        reserveBytes +=
            /* Pipe Select. */
            4 * 4
            /* Configure MMU States. */
            + 4 * 4
            /* Semaphore stall */
            + 4 * 8;

        if (ace)
            reserveBytes += 8;
    }

    reserveBytes += 8 + 8 * probeSelectCount;

    if (multiCluster)
        reserveBytes += 8;

    physical = Mmu->mtlbPhysical;

    config = (gctUINT32)(physical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(physical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    physical = Mmu->safePagePhysical;

    address = (gctUINT32)(physical & 0xFFFFFFFF);
    extSafeAddress = (gctUINT32)(physical >> 32);

    if (address & 0x3F)
        gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

    /* more than 40bit physical address */
    if (extSafeAddress & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    if (ace) {
        configEx = gcmSETFIELD(0, GCREG_MMU_CONFIGURATION_EXT, SAFE_ADDRESS, extSafeAddress) |
                   gcmSETFIELD(0, GCREG_MMU_CONFIGURATION_EXT, MASTER_TLB, extMtlb);
    }

    switch (Mode) {
    case gcvMMU_MODE_1K:
        if (config & 0x3FF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE1_K);

        break;

    case gcvMMU_MODE_4K:
        if (config & 0xFFF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE4_K);

        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Logical != gcvNULL) {
        buffer = (gctUINT32_PTR)Logical;

        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gcsMMU_TABLE_ARRAY_ENTRY *entry;

            entry = (gcsMMU_TABLE_ARRAY_ENTRY *)Hardware->pagetableArray.logical;

            /* Setup page table array entry. */
            if (Hardware->bigEndian) {
                entry->low = gcmBSWAP32(config);
                entry->high = gcmBSWAP32(extMtlb);
            } else {
                entry->low = config;
                entry->high = extMtlb;
            }

            gcmkDUMP(Mmu->os, "#[mmu: page table array]");

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address, entry->low, 4);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address + 4, entry->high, 4);

            gcmkONERROR(gckVIDMEM_NODE_CleanCache(Hardware->kernel,
                                                  Hardware->pagetableArray.videoMem,
                                                  0, entry, 8));

            if (multiCluster) {
                gctUINT32 clusterMask;

                gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregClusterControlRegAddrs);

                *buffer++ = gcmSETFIELD(0, GCREG_CLUSTER_CONTROL, CLUSTER_ALIVE_ENABLE, clusterMask);
            }

            /* Setup command buffer to load index 0 of page table array. */
            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = gcmSETMASKEDFIELD(GCREG_MMU_CONFIG, PAGE_TABLE_ID, 0);
        } else {
            gcmkASSERT(Hardware->options.secureMode == gcvSECURE_NONE);

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = config;

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUSafeAddressRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = address;

            if (ace) {
                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationExtRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *buffer++ = configEx;
            }
        }

        *buffer++ =
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregStoreCounterRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *buffer++ = gcmSETFIELD(0, GCREG_STORE_COUNTER, SOURCE_SH, 1) |
                    gcmSETFIELD(0, GCREG_STORE_COUNTER, SOURCE_HI, 1) |
                    gcmSETFIELD(0, GCREG_STORE_COUNTER, SOURCE_PA, 1) |
                    gcmSETFIELD(0, GCREG_STORE_COUNTER, SOURCE_TX, 1);

        for (i = 0; i < probeSelectCount; i++)
        {
            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregProbeSelectRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);


            *buffer++ = gcmSETFIELD(0, GCREG_PROBE_SELECT, COUNTER, i) |
                   gcmSETFIELDVALUE(0, GCREG_PROBE_SELECT, MODULE,  HOST_INTERFACE0) |
                   gcmSETFIELDVALUE(0, GCREG_PROBE_SELECT, OP_CODE, RESET);
        }

        gcmSEMAPHORESTALL(buffer);

        if (config2D) {
            /* LoadState(AQPipeSelect, 1), pipe. */
            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQPipeSelectRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = AQ_PIPE_SELECT_PIPE_PIPE2D;

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = config;

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUSafeAddressRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = address;

            if (ace) {
                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationExtRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *buffer++ = configEx;
            }

            gcmSEMAPHORESTALL(buffer);

            /* LoadState(AQPipeSelect, 1), pipe. */
            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQPipeSelectRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = AQ_PIPE_SELECT_PIPE_PIPE3D;

            gcmSEMAPHORESTALL(buffer);
        }
    }

    if (Bytes != gcvNULL)
        *Bytes = reserveBytes;

    /* Return the status. */
    gcmkFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_ProgramMMUStatesMCFE(gckHARDWARE Hardware, gckMMU Mmu,
                      gceMMU_MODE Mode, gctPOINTER Logical, gctUINT32 *Bytes)
{
#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 config, address;
    gctUINT32 extMtlb, extSafeAddress, configEx = 0;
    gctPHYS_ADDR_T physical;
    gctUINT32_PTR buffer;
    gctBOOL ace;
    gctUINT32 reserveBytes = 0;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    ace = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_ACE);

    switch (Hardware->options.secureMode) {
    case gcvSECURE_IN_NORMAL:
        reserveBytes = 8;
        reserveBytes += 8;
        break;
    case gcvSECURE_NONE:
        reserveBytes = 16;
        ;
        if (ace) {
            reserveBytes += 8;
            reserveBytes += 8;
        }
        break;
    case gcvSECURE_IN_TA:
    default:
        gcmkASSERT(gcvFALSE);
        gcmkPRINT("%s(%d): secureMode is wrong", __FUNCTION__, __LINE__);
        break;
    }

    physical = Mmu->mtlbPhysical;

    config = (gctUINT32)(physical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(physical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    physical = Mmu->safePagePhysical;

    address = (gctUINT32)(physical & 0xFFFFFFFF);
    extSafeAddress = (gctUINT32)(physical >> 32);

    if (address & 0x3F)
        gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

    /* more than 40bit physical address */
    if (extSafeAddress & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    if (ace) {
        configEx = gcmSETFIELD(0, GCREG_MMU_CONFIGURATION_EXT, SAFE_ADDRESS, extSafeAddress) |
                   gcmSETFIELD(0, GCREG_MMU_CONFIGURATION_EXT, MASTER_TLB, extMtlb);
    }

    switch (Mode) {
    case gcvMMU_MODE_1K:
        if (config & 0x3FF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE1_K);

        break;

    case gcvMMU_MODE_4K:
        if (config & 0xFFF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE4_K);

        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Logical != gcvNULL) {
        buffer = (gctUINT32_PTR)Logical;

        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gcsMMU_TABLE_ARRAY_ENTRY *entry;

            entry = (gcsMMU_TABLE_ARRAY_ENTRY *)Hardware->pagetableArray.logical;

            /* Setup page table array entry. */
            if (Hardware->bigEndian) {
                entry->low = gcmBSWAP32(config);
                entry->high = gcmBSWAP32(extMtlb);
            } else {
                entry->low = config;
                entry->high = extMtlb;
            }

            gcmkDUMP(Mmu->os, "#[mmu: page table array]");

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address, entry->low, 4);

            gcmkDUMP(Mmu->os, "@[physical.fill 0x%010llX 0x%08X 0x%08X]",
                     (unsigned long long)Hardware->pagetableArray.address + 4, entry->high, 4);

            gcmkONERROR(gckVIDMEM_NODE_CleanCache(Hardware->kernel,
                                                  Hardware->pagetableArray.videoMem, 0, entry, 8));

            /* Setup command buffer to load index 0 of page table array. */
            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = gcmSETMASKEDFIELD(GCREG_MMU_CONFIG, PAGE_TABLE_ID, 0);

            *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
            *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
        } else {
            gcmkASSERT(Hardware->options.secureMode == gcvSECURE_NONE);

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = config;

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUSafeAddressRegAddrs) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

            *buffer++ = address;

            if (ace) {
                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationExtRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *buffer++ = configEx;

                *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
                *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
            }
        }
    }

    if (Bytes != gcvNULL)
        *Bytes = reserveBytes;

    /* Return the status. */
    gcmkFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

static gceSTATUS
_FuncInit_MMU(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status;
    gctUINT32 mmuBytes = 0;
    gctUINT32 tailBytes;
    gctUINT32 flags = gcvALLOC_FLAG_CONTIGUOUS;
    gceMMU_MODE mode;
    gcePOOL pool;
    gctPHYS_ADDR_T physical;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gckMMU mmu = hardware->kernel->mmu;
    gctPOINTER pointer = gcvNULL;
    gctUINT64 data = 0;

    if (mmu->initMode == gcvMMU_INIT_FROM_REG)
        return gcvSTATUS_OK;

#if gcdENABLE_MMU_1KMODE
    mode = gcvMMU_MODE_1K;
#else
    mode = gcvMMU_MODE_4K;
#endif

    if (hardware->largeVAVersion == gcv40BIT_VA_40BIT_PA_0)
        mode = gcvMMU_MODE_4K;

    /* If not support PD mode and graphicLargeVA. */
    if (!gckHARDWARE_IsFeatureAvailable(hardware,
                                        gcvFEATURE_MMU_PAGE_DESCRIPTOR) &&
        !hardware->graphicsLargeVA) {
#ifdef __linux__
#if defined(CONFIG_ZONE_DMA32) || defined(CONFIG_ZONE_DMA)
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 37)
    flags |= gcvALLOC_FLAG_4GB_ADDR | gcvALLOC_FLAG_4K_PAGES;
#endif
#endif
#else
    flags |= gcvALLOC_FLAG_4GB_ADDR;
#endif
    }

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    flags |= gcvALLOC_FLAG_CACHEABLE;
#endif

    if (hardware->largeVAVersion && !hardware->graphicsLargeVA)
        flags |= gcvALLOC_FLAG_32BIT_VA;

    status = gckOS_QueryOption(hardware->os, "mmuCmdPool", &data);
    if (gcmIS_SUCCESS(status))
        pool = (gcePOOL)data;
    else
        pool = gcvPOOL_DEFAULT;

#if gcdCAPTURE_ONLY_MODE
    pool = gcvPOOL_VIRTUAL;
#endif

    Execution->funcCmdCount = 1;

    /* Allocate the gcsFUNCTION_COMMAND buffer */
    gcmkONERROR(gckOS_Allocate(hardware->os,
                               gcmSIZEOF(gcsFUNCTION_COMMAND) * Execution->funcCmdCount,
                               (gctPOINTER *)&pointer));

    gckOS_ZeroMemory((gctPOINTER)pointer,
                     gcmSIZEOF(gcsFUNCTION_COMMAND) * Execution->funcCmdCount);

    Execution->funcCmd = (gcsFUNCTION_COMMAND_PTR)pointer;

    Execution->funcCmd[0].funcVidMemBytes = 1024;
    /* Allocate mmu command buffer within 32bit space */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(hardware->kernel, 64,
                                              gcvVIDMEM_TYPE_COMMAND, flags,
                                              &Execution->funcCmd[0].funcVidMemBytes, &pool,
                                              &Execution->funcCmd[0].funcVidMem));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(hardware->kernel,
                                       Execution->funcCmd[0].funcVidMem,
                                       gcvFALSE, gcvFALSE,
                                       &Execution->funcCmd[0].logical));

    /* Get GPU physical address. */
    gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(hardware->kernel,
                                              Execution->funcCmd[0].funcVidMem,
                                              0, &physical));

    if (!gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_MMU_PAGE_DESCRIPTOR) &&
        !hardware->graphicsLargeVA &&
        (physical & 0xFFFFFFFF00000000ULL)) {
        gcmkFATAL("%s(%d): Command buffer physical address (0x%llx) for MMU setup exceeds 32bits, please rebuild kernel with CONFIG_ZONE_DMA32=y or CONFIG_ZONE_DMA=y or both.",
                  __FUNCTION__, __LINE__, physical);

        gcmkFATAL("Some Archs, for ARM64, the setting is special:\n"
                    "kernel version   ZONE_DMA   ZONE_DMA32\n"
                    "3.7  - 3.14        no          yes\n"
                    "3.15 - 4.15        yes         no\n"
                    "4.16 - 5.4.5       no          yes\n"
                    "5.5.rc1 -          yes         yes\n");

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    Execution->funcCmd[0].physical = physical;

    if (hardware->mcFE) {
        gcmkONERROR(_ProgramMMUStatesMCFE(hardware,
                                          hardware->kernel->mmu, mode,
                                          Execution->funcCmd[0].logical, &mmuBytes));
    } else {
        gcmkONERROR(_ProgramMMUStates(hardware,
                                      hardware->kernel->mmu, mode,
                                      Execution->funcCmd[0].logical, &mmuBytes));
    }

    Execution->funcCmd[0].endPhysical = Execution->funcCmd[0].physical + mmuBytes;
    Execution->funcCmd[0].endLogical  = (gctUINT8_PTR)Execution->funcCmd[0].logical + mmuBytes;

    Execution->funcCmd[0].address = Execution->funcCmd[0].physical;
    Execution->funcCmd[0].endAddress = Execution->funcCmd[0].endPhysical;

    if (hardware->wlFE) {
        tailBytes = (gctUINT32)(Execution->funcCmd[0].funcVidMemBytes - mmuBytes);
        gcmkONERROR(gckWLFE_End(hardware,
                                Execution->funcCmd[0].endLogical,
                                Execution->funcCmd[0].endAddress,
                                &tailBytes));
    } else {
        tailBytes = 0;
    }

    Execution->funcCmd[0].bytes = mmuBytes + tailBytes;

    gcmkONERROR(gckVIDMEM_NODE_CleanCache(hardware->kernel,
                                          Execution->funcCmd[0].funcVidMem, 0,
                                          Execution->funcCmd[0].logical,
                                          Execution->funcCmd[0].bytes));

    return gcvSTATUS_OK;

OnError:
    _FuncRelease_MMU(Execution);

    return status;
}

static gceSTATUS
_FuncExecute_MMU_REG(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gckMMU mmu = hardware->kernel->mmu;
    gctUINT32 address = 0;
    gctUINT32 extSafeAddress = 0;
    gctUINT32 mtlb, extMtlb;

    mtlb = (gctUINT32)(mmu->mtlbPhysical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(mmu->mtlbPhysical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    gcmkONERROR(gckOS_WriteRegisterEx(
        hardware->os, hardware->kernel,
        GCREG_MMUAHB_CONTEXT_PD_ENTRY0_Address,
        gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, MASTER_TLB_MODE, ONE_K_MODE) |
            gcmSETFIELD(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, PD_ADDRESS, (extMtlb << 20) | (mtlb >> 12))));

    address = (gctUINT32)(mmu->safePagePhysical & 0xFFFFFFFF);
    extSafeAddress = (gctUINT32)(mmu->safePagePhysical >> 32);

    if (address & 0x3F)
        gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

    /* more than 40bit physical address */
    if (extSafeAddress & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                      GCREG_MMUAHB_TABLE_ARRAY_SIZE_Address, 1));

    gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                      GCREG_MMUAHB_SAFE_SECURE_ADDRESS_Address, address));

    gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                      GCREG_MMUAHB_SAFE_NON_SECURE_ADDRESS_Address, address));

    gcmkONERROR(gckOS_WriteRegisterEx(
        hardware->os, hardware->kernel,
        GCREG_MMUAHB_SAFE_ADDRESS_EXT_Address,
        gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_SECURE, (gctUINT32)extSafeAddress) |
            gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_SECURE, ENABLED) |
            gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_NON_SECURE, (gctUINT32)extSafeAddress) |
            gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_NON_SECURE, ENABLED)));

OnError:
    return status;
}

static gceSTATUS
_FuncExecute_MMU_CMD(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 address = 0;
    gctUINT32 idle;
    gctUINT32 timer = 0, delay = 10;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gckMMU mmu = hardware->kernel->mmu;
    gctUINT32_PTR endLogical = (gctUINT32_PTR)Execution->funcCmd[0].endLogical;
    gctBOOL multiCluster = gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_MULTI_CLUSTER);
  
    /*
     * Prepared command sequence contains an END,
     * so update lastEnd and store executeCount to END command.
     */

    hardware->lastEnd = Execution->funcCmd[0].endAddress;

    if (hardware->wlFE) {
        /* Append a executeCount in End command, MCFE does not support such End command. */
        *(endLogical + 1) = hardware->executeCount + 1;
    }

    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_MMU_PAGE_DESCRIPTOR)) {
        gctUINT32 extAddress = (gctUINT32)(Execution->funcCmd[0].physical >> 32);

        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os, hardware->kernel,
            GCREG_MMUAHB_CONTEXT_PD_ENTRY0_Address,
            gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, MASTER_TLB_MODE, BY_PASS_MODE) |
                 gcmSETFIELD(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, PD_ADDRESS, (extAddress << 20))));
    }

    if (hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
        gctUINT32 extSafeAddress;
        gctUINT32 size = 1;
        gctUINT32 extAddress = (gctUINT32)((hardware->pagetableArray.address >> 32) & 0xFFFFFFFF);

        if (hardware->kernel->processPageTable)
            size = gcdMMU_DESC_SIZE;

        /* Set up base address of page table array. */
        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_LOW_Address,
                                          (gctUINT32)(hardware->pagetableArray.address & 0xFFFFFFFF)));

        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
            GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_HIGH_Address,
            gcmSETFIELD(0, GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_HIGH, MASTER_TLB, extAddress & 0xFF) |
            gcmSETFIELD(0, GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_HIGH, MASTER_TLB_HI, (extAddress >> 8) & 0xFF)));

        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_TABLE_ARRAY_SIZE_Address, size));

        address = (gctUINT32)(mmu->safePagePhysical & 0xFFFFFFFF);
        extSafeAddress = (gctUINT32)(mmu->safePagePhysical >> 32);

        if (address & 0x3F)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        /* more than 40bit physical address */
        if (extSafeAddress & 0xFFFFFF00)
            gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_SAFE_SECURE_ADDRESS_Address, address));

        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_SAFE_NON_SECURE_ADDRESS_Address, address));

#if gcdMMU_VERSION_1
        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os, hardware->kernel,
            GCREG_MMUAHB_SAFE_ADDRESS_EXT_Address,
            gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_SECURE, (gctUINT32)extSafeAddress) |
                gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_SECURE, ENABLED) |
                gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_NON_SECURE, (gctUINT32)extSafeAddress) |
                gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_NON_SECURE, ENABLED)));
#endif

#if gcdMMU_VERSION_2
        /* Upper Safe Address for 48PA mmu. */
        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_SAFE_ADDRESS_Address, extSafeAddress));
        /* Upper Secure Safe Address for 48PA mmu. */
        gcmkONERROR(gckOS_WriteRegisterEx(hardware->os, hardware->kernel,
                                          GCREG_MMUAHB_SECURE_SAFE_ADDRESS_Address, extSafeAddress));
#endif
    }

    gckFUNCTION_Dump(Execution);

    if (multiCluster) {
        gctUINT32 clusterMask, state, offset = 0, index = 0;
        gctUINT32 *buffer = Execution->funcCmd[0].logical;

        state = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregClusterControlRegAddrs);

        gcmkONERROR(gckHARDWARE_QueryClusterInfo(hardware, &clusterMask));

        do {
            if (*buffer++ == state)
                break;

            offset = ++index * gcmSIZEOF(gctUINT32);
        } while (offset < Execution->funcCmd[0].bytes);

        if (offset >= Execution->funcCmd[0].bytes)
            gcmkONERROR(gcvSTATUS_NOT_FOUND);

        *buffer = gcmSETFIELD(0, GCREG_CLUSTER_CONTROL, CLUSTER_ALIVE_ENABLE, clusterMask);
    }

    /* Execute prepared command sequence. */
    if (hardware->mcFE) {
        gcmkONERROR(gckMCFE_Execute(hardware, gcvFALSE, 0,
                                    Execution->funcCmd[0].address,
                                    Execution->funcCmd[0].bytes));
    } else {
        gcmkONERROR(gckWLFE_Execute(hardware,
                                    Execution->funcCmd[0].address,
                                    Execution->funcCmd[0].bytes));
    }

#if gcdLINK_QUEUE_SIZE
    {
        gcuQUEUEDATA data;

        gcmkVERIFY_OK(gckOS_GetProcessID(&data.linkData.pid));

        data.linkData.start = Execution->funcCmd[0].address;
        data.linkData.end = Execution->funcCmd[0].address + Execution->funcCmd[0].bytes;
        data.linkData.linkLow = 0;
        data.linkData.linkHigh = 0;

        gckQUEUE_Enqueue(&hardware->linkQueue, &data);
    }
#endif

    /* Wait until MMU configure finishes. */
    do {
#if gcdFPGA_BUILD
        gckOS_Delay(hardware->os, delay);
#else
        gckOS_Udelay(hardware->os, delay);
#endif

        gcmkONERROR(gckOS_ReadRegisterEx(hardware->os, hardware->kernel, AQ_HI_IDLE_Address, &idle));

        timer += delay;
        delay *= 2;

#if gcdGPU_TIMEOUT
        if (timer >= hardware->kernel->timeOut) {
            gckHARDWARE_DumpGPUState(hardware);

            if (hardware->kernel->command)
                gckCOMMAND_DumpExecutingBuffer(hardware->kernel->command);

            /*
             * Even if hardware is not reset correctly, let software
             * continue to avoid software stuck. Software will timeout again
             * and try to recover GPU in next timeout.
             */
            gcmkONERROR(gcvSTATUS_DEVICE);
        }
#endif
    } while (!gcmGETFIELD(idle, AQ_HI_IDLE, IDLE_FE));

    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_MMU_PAGE_DESCRIPTOR)) {
        gcmkONERROR(gckOS_WriteRegisterEx(
            hardware->os, hardware->kernel,
            GCREG_MMUAHB_CONTEXT_PD_ENTRY0_Address,
            gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, MASTER_TLB_MODE, ONE_K_MODE) |
                 gcmSETFIELD(0, GCREG_MMUAHB_CONTEXT_PD_ENTRY0, PD_ADDRESS, 0)));
    }

    gcmkDUMP(hardware->os, "@[register.wait 0x%05X 0x%08X 0x%08X]",
             AQ_HI_IDLE_Address, gcmSETFIELD(0, AQ_HI_IDLE, IDLE_FE, ~0U), idle);

OnError:
    return status;
}

static gceSTATUS
_FuncExecute_MMU(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gckMMU mmu = hardware->kernel->mmu;
    gceSTATUS status = gcvSTATUS_OK;

    if (mmu->initMode == gcvMMU_INIT_FROM_REG)
        gcmkONERROR(_FuncExecute_MMU_REG(Execution));
    else
        gcmkONERROR(_FuncExecute_MMU_CMD(Execution));

OnError:
    return status;
}

static gceSTATUS
_FuncValidate_Flush(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_BLT_ENGINE) &&
        gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_COMPUTE_ONLY)) {
        Execution->valid = gcvFALSE;
    } else {
        Execution->valid = gcvTRUE;
    }

#if gcdDUMP_IN_KERNEL
    if (hardware->mcFE)
        Execution->valid = gcvFALSE;
#endif

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncRelease_Flush(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gctUINT8 i;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (Execution->funcCmd) {
        for (i = 0; i < Execution->funcCmdCount; i++) {
            if (Execution->funcCmd[i].funcVidMem) {
                if (Execution->funcCmd[i].address) {
                    /* Synchroneous unlock. */
                    gcmkVERIFY_OK(gckVIDMEM_NODE_Unlock(hardware->kernel,
                                                        Execution->funcCmd[i].funcVidMem,
                                                        hardware->kernel->mmu, gcvNULL));
                    Execution->funcCmd[i].address = 0;
                }

                if (Execution->funcCmd[i].logical) {
                    gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(hardware->kernel,
                                                           Execution->funcCmd[i].funcVidMem,
                                                           0, gcvFALSE, gcvFALSE));
                    Execution->funcCmd[i].logical = gcvNULL;
                }

                gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(hardware->kernel,
                                                         Execution->funcCmd[i].funcVidMem));

                Execution->funcCmd[i].funcVidMem = gcvNULL;
            }
        }

        gcmkVERIFY_OK(gckOS_Free(hardware->os, (gctPOINTER)Execution->funcCmd));
        Execution->funcCmd = gcvNULL;
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_FuncInit_Flush(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 flushBytes = 0;
    gctUINT32 offset = 0;
    gctUINT32 endBytes = 0;
    gctADDRESS address;
    gctUINT32 allocFlag = 0;
    gcePOOL pool;
    gctUINT8_PTR logical;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;
    gctPOINTER pointer = gcvNULL;

#if !gcdCAPTURE_ONLY_MODE
    pool = gcvPOOL_DEFAULT;
#else
    pool = gcvPOOL_VIRTUAL;
#endif

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
    allocFlag = gcvALLOC_FLAG_CACHEABLE;
#endif

    if (hardware->largeVAVersion && !hardware->graphicsLargeVA)
        allocFlag |= gcvALLOC_FLAG_32BIT_VA;

    Execution->funcCmdCount = 1;

    /* Allocate the gcsFUNCTION_COMMAND buffer */
    gcmkONERROR(gckOS_Allocate(hardware->os,
                               gcmSIZEOF(gcsFUNCTION_COMMAND) * Execution->funcCmdCount,
                               (gctPOINTER *)&pointer));

    gckOS_ZeroMemory((gctPOINTER)pointer,
                     gcmSIZEOF(gcsFUNCTION_COMMAND) * Execution->funcCmdCount);

    Execution->funcCmd = (gcsFUNCTION_COMMAND_PTR)pointer;

    Execution->funcCmd[0].funcVidMemBytes = 1024;
    Execution->funcCmd[0].channelId = 0;
    /* Allocate video memory node for aux functions. */
    gcmkONERROR(gckKERNEL_AllocateVideoMemory(hardware->kernel, 64,
                                              gcvVIDMEM_TYPE_COMMAND,
                                              allocFlag, &Execution->funcCmd[0].funcVidMemBytes,
                                              &pool, &Execution->funcCmd[0].funcVidMem));

    /* Lock for GPU access. */
    gcmkONERROR(gckVIDMEM_NODE_Lock(hardware->kernel,
                                    Execution->funcCmd[0].funcVidMem,
                                    &Execution->funcCmd[0].address));

    /* Lock for kernel side CPU access. */
    gcmkONERROR(gckVIDMEM_NODE_LockCPU(hardware->kernel,
                                       Execution->funcCmd[0].funcVidMem,
                                       gcvFALSE, gcvFALSE,
                                       &Execution->funcCmd[0].logical));
    /*
     * All cache flush command sequence.
     */
    logical = (gctUINT8_PTR)Execution->funcCmd[0].logical;
    address = Execution->funcCmd[0].address;

    /* Get the size of the flush command. */
    gcmkONERROR(gckHARDWARE_Flush(hardware, gcvFLUSH_ALL, gcvNULL, &flushBytes));

    /* Append a flush. */
    gcmkONERROR(gckHARDWARE_Flush(hardware, gcvFLUSH_ALL, logical, &flushBytes));

    offset += flushBytes;
    logical += offset;
    address += offset;

    if (hardware->wlFE) {
        gcmkONERROR(gckWLFE_End(hardware, gcvNULL, ~0U, &endBytes));
        gcmkONERROR(gckWLFE_End(hardware, logical, address, &endBytes));
    }

    Execution->funcCmd[0].bytes = flushBytes + endBytes;
    Execution->funcCmd[0].endAddress = Execution->funcCmd[0].address + flushBytes;
    Execution->funcCmd[0].endLogical = (gctUINT8_PTR)Execution->funcCmd[0].logical + flushBytes;

    return gcvSTATUS_OK;
OnError:
    _FuncRelease_Flush(Execution);

    return status;
}

/*******************************************************************************
 **
 **  gckFUNCTION_Construct
 **
 **  Generate command buffer snippets which will be used by gckHARDWARE, by which
 **  gckHARDWARE can manipulate GPU by FE command without using gckCOMMAND to avoid
 **  race condition and deadlock.
 **
 **  Notice:
 **  1. Each snippet can only be executed when GPU is idle.
 **  2. Execution is triggered by AHB (0x658)
 **  3. Each snippet followed by END so software can sync with GPU by checking GPU
 **     idle
 **  4. It is transparent to gckCOMMAND command buffer.
 **
 **  Existing Snippets:
 **  1. MMU Configure
 **     For new MMU, after GPU is reset, FE execute this command sequence to enable MMU.
 */
gceSTATUS
gckFUNCTION_Construct(gctPOINTER Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckHARDWARE hardware = (gckHARDWARE)Hardware;
    gctPOINTER pointer = gcvNULL;
    gctUINT i;

    gcmkHEADER_ARG("Hardware=%p", Hardware);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    /* Allocate the gcsFUNCTION_EXECUTION object. */
    gcmkONERROR(gckOS_Allocate(hardware->os,
                               gcmSIZEOF(gcsFUNCTION_EXECUTION) * gcvFUNCTION_EXECUTION_NUM,
                               &pointer));

    gckOS_ZeroMemory(pointer, gcmSIZEOF(gcsFUNCTION_EXECUTION) * gcvFUNCTION_EXECUTION_NUM);

    hardware->functions = (gcsFUNCTION_EXECUTION_PTR)pointer;

    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++) {
        gcsFUNCTION_EXECUTION_PTR func = &hardware->functions[i];

        func->hardware = hardware;
        func->funcId = (gceFUNCTION_EXECUTION)i;
        func->funcCmd = gcvNULL;

        /* Init functions API pointer */
        switch (i) {
        case gcvFUNCTION_EXECUTION_MMU:
            gckOS_MemCopy(func->funcName, "set mmu", 8);
            func->funcExecution.init = _FuncInit_MMU;
            func->funcExecution.validate = _FuncValidate_MMU;
            func->funcExecution.execute = _FuncExecute_MMU;
            func->funcExecution.release = _FuncRelease_MMU;
            break;

        case gcvFUNCTION_EXECUTION_FLUSH:
            gckOS_MemCopy(func->funcName, "flush", 6);
            func->funcExecution.init = _FuncInit_Flush;
            func->funcExecution.validate = _FuncValidate_Flush;
            func->funcExecution.execute = _FuncExecute;
            func->funcExecution.release = _FuncRelease_Flush;
            break;

        default:
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }

    return gcvSTATUS_OK;
OnError:
    if (hardware->functions)
        gcmkVERIFY_OK(gckOS_Free(hardware->os, hardware->functions));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckFUNCTION_Destory(gctPOINTER Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckHARDWARE hardware = (gckHARDWARE)Hardware;
    gctUINT i;

    gcmkHEADER_ARG("Hardware=%p", Hardware);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++)
        gckFUNCTION_Release(&hardware->functions[i]);

    gcmkVERIFY_OK(gckOS_Free(hardware->os, hardware->functions));
    hardware->functions = gcvNULL;

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckFUNCTION_Validate(gcsFUNCTION_EXECUTION_PTR Execution, gctBOOL_PTR Valid)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=%p", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);
    gcmkVERIFY_ARGUMENT(Valid != gcvNULL);

    if (Execution->funcExecution.validate) {
        status = Execution->funcExecution.validate(Execution);
        *Valid = Execution->valid;
    }

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckFUNCTION_Init(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=%p", Execution);
    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    Execution->inited = gcvTRUE;
    if (Execution->funcExecution.init)
        status = Execution->funcExecution.init(Execution);

    if (status != gcvSTATUS_OK)
        Execution->inited = gcvFALSE;

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckFUNCTION_Execute(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Execution=0x%x", Execution);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    if (Execution->inited && Execution->funcExecution.execute)
        gcmkONERROR(Execution->funcExecution.execute(Execution));
    else
        status = gcvSTATUS_NOT_SUPPORTED;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckFUNCTION_Release(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;

    gcmkHEADER_ARG("Execution=0x%x", Execution);

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Execution != gcvNULL);

    if (Execution->inited && Execution->funcExecution.release)
        status = Execution->funcExecution.release(Execution);

    Execution->inited = gcvFALSE;

    gcmkFOOTER();
    return status;
}

void
gckFUNCTION_Dump(gcsFUNCTION_EXECUTION_PTR Execution)
{
#if gcdDUMP_IN_KERNEL
    gctUINT32 i;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

    if (!Execution || !Execution->funcCmd)
        return;

    gcmkDUMP(hardware->os, "#[function: %s]", Execution->funcName);
    for (i = 0; i < Execution->funcCmdCount; i++) {
        gcmkDUMP_BUFFER(hardware->os,
                        gcvDUMP_BUFFER_KERNEL_COMMAND,
                        Execution->funcCmd[i].logical,
                        Execution->funcCmd[i].address,
                        Execution->funcCmd[i].bytes);
    }
#endif
}
