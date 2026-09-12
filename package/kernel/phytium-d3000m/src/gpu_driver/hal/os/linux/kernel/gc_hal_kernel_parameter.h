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


#ifndef _GC_HAL_KERNEL_PARAMETER_H_
#define _GC_HAL_KERNEL_PARAMETER_H_

/******************************************************************************
 *                       register memory related                              *
 ******************************************************************************/

static ulong registerMemBase = 0x80000000;
module_param(registerMemBase, ulong, 0644);
MODULE_PARM_DESC(registerMemBase, "Base of bus address of GC core AHB register");

static ulong registerMemSize = 2 << 16;
module_param(registerMemSize, ulong, 0644);
MODULE_PARM_DESC(registerMemSize, "Size of bus address range of GC core AHB register");

#if gcdDEC_ENABLE_AHB
static ulong registerMemBaseDEC300 = 0x00000000;
module_param(registerMemBaseDEC300, ulong, 0644);

static ulong registerMemSizeDEC300 = 2 << 10;
module_param(registerMemSizeDEC300, ulong, 0644);
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static gctPHYS_ADDR_T registerBases[gcdGLOBAL_CORE_COUNT];
module_param_array(registerBases, ullong, NULL, 0644);
#else
static ulong registerBases[gcdGLOBAL_CORE_COUNT];
module_param_array(registerBases, ulong, NULL, 0644);
#endif
MODULE_PARM_DESC(registerBases, "Array of bases of bus address of register of multi-core");

static ulong registerSizes[gcdGLOBAL_CORE_COUNT] = {
    [0 ... gcdGLOBAL_CORE_COUNT - 1] = 2 << 16
};
module_param_array(registerSizes, ulong, NULL, 0644);
MODULE_PARM_DESC(registerSizes, "Array of sizes of bus address range of register of multi-core");

static uint registerAPB = 0x300000;
module_param(registerAPB, uint, 0644);
MODULE_PARM_DESC(registerAPB, "The offset of APB register to the register base address.");

/******************************************************************************
 *                         memory pool related                                *
 ******************************************************************************/

static ulong contiguousSize = 0;
module_param(contiguousSize, ulong, 0644);
MODULE_PARM_DESC(contiguousSize, "Size of reserved system memory");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static gctPHYS_ADDR_T contiguousBase = 0;
module_param(contiguousBase, ullong, 0644);
#else
static ulong contiguousBase = 0;
module_param(contiguousBase, ulong, 0644);
#endif
MODULE_PARM_DESC(contiguousBase, "Base address of reserved system memory");

static ulong contiguousSizes[gcdSYSTEM_RESERVE_COUNT] = {
    [0 ... gcdSYSTEM_RESERVE_COUNT - 1] = 0
};
module_param_array(contiguousSizes, ulong, NULL, 0644);
MODULE_PARM_DESC(contiguousSizes, "Sizes of reserved system memory array");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static gctPHYS_ADDR_T contiguousBases[gcdSYSTEM_RESERVE_COUNT] = {
    [0 ... gcdSYSTEM_RESERVE_COUNT - 1] = 0
};
module_param_array(contiguousBases, ullong, NULL, 0644);
#else
static ulong contiguousBases[gcdSYSTEM_RESERVE_COUNT] = {
    [0 ... gcdSYSTEM_RESERVE_COUNT - 1] = 0
};
module_param_array(contiguousBases, ulong, NULL, 0644);
#endif
MODULE_PARM_DESC(contiguousBases, "Base addresses of reserved system memory array");

static ulong externalSize[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(externalSize, ulong, NULL, 0644);
MODULE_PARM_DESC(externalSize, "Size of external local memory, if it is 0, means there is no external pool");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static gctPHYS_ADDR_T externalBase[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(externalBase, ullong, NULL, 0644);
#else
static ulong externalBase[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(externalBase, ulong, NULL, 0644);
#endif
MODULE_PARM_DESC(externalBase, "Base address of external memory");

static ulong exclusiveSize[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(exclusiveSize, ulong, NULL, 0644);
MODULE_PARM_DESC(exclusiveSize, "Size of exclusive local memory, if it is 0, means there is no exclusive pool");

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
static gctPHYS_ADDR_T exclusiveBase[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(exclusiveBase, ullong, NULL, 0644);
#else
static ulong exclusiveBase[gcdLOCAL_MEMORY_COUNT] = {
    [0 ... gcdLOCAL_MEMORY_COUNT - 1] = 0
};
module_param_array(exclusiveBase, ulong, NULL, 0644);
#endif
MODULE_PARM_DESC(exclusiveBase, "Base address of exclusive memory(GPU access only)");

static uint lockLargeVA = 1;
module_param(lockLargeVA, uint, 0644);
MODULE_PARM_DESC(lockLargeVA, "Default 1. 0: not lowVA buf could map lowVA addr. 1: not lowVA buf must map largeVA.");

/******************************************************************************
 *                          interrupt related                                 *
 ******************************************************************************/

static int irqLine = -1;
module_param(irqLine, int, 0644);
MODULE_PARM_DESC(irqLine, "IRQ number of GC core");

static int irqs[gcdGLOBAL_CORE_COUNT] = {[0 ... gcdGLOBAL_CORE_COUNT - 1] = -1};
module_param_array(irqs, int, NULL, 0644);
MODULE_PARM_DESC(irqs, "Array of IRQ numbers of multi-core");

static uint isrPoll = 0;
module_param(isrPoll, uint, 0644);
MODULE_PARM_DESC(isrPoll, "Bits isr polling for per-core, default 0'1b means disable, 1'1b means auto enable isr polling mode");

/******************************************************************************
 *                        identification related                              *
 ******************************************************************************/

static uint platformIDs[gcdDEVICE_COUNT] = {[0 ... gcdDEVICE_COUNT - 1] = 0};
module_param_array(platformIDs, uint, NULL, 0644);
MODULE_PARM_DESC(platformIDs, "Array of platform id of each hardware device");

static uint hwDevCounts[gcdPLATFORM_COUNT] = {[0 ... gcdPLATFORM_COUNT - 1] = 1};
module_param_array(hwDevCounts, uint, NULL, 0644);
MODULE_PARM_DESC(hwDevCounts, "Array of hw device count of each platform");

static uint devCoreCounts[gcdDEVICE_COUNT] = {gcdMAX_MAJOR_CORE_COUNT, 0};
module_param_array(devCoreCounts, uint, NULL, 0644);
MODULE_PARM_DESC(devCoreCounts, "Array of core count of each hardware device");

static uint devMemIDs[gcdDEVICE_COUNT] = {[0 ... gcdDEVICE_COUNT - 1] = 0};
module_param_array(devMemIDs, uint, NULL, 0644);
MODULE_PARM_DESC(devMemIDs, "Array of local memory index of each hardware device");

static uint devSysMemIDs[gcdDEVICE_COUNT] = {[0 ... gcdDEVICE_COUNT - 1] = 1};
module_param_array(devSysMemIDs, uint, NULL, 0644);
MODULE_PARM_DESC(devSysMemIDs, "Array of system reserved memory index of each hardware device, each bit represents memory id");

static uint devSRAMIDs[gcdDEVICE_COUNT] = {[0 ... gcdDEVICE_COUNT - 1] = 1};
module_param_array(devSRAMIDs, uint, NULL, 0644);
MODULE_PARM_DESC(devSRAMIDs, "Array of SRAM index of each hardware device, each bit represents memory id");

static uint chipIDs[gcvCORE_COUNT] = {
    [0 ... gcvCORE_COUNT - 1] = gcvCHIP_ID_DEFAULT
};
module_param_array(chipIDs, uint, NULL, 0644);
MODULE_PARM_DESC(chipIDs, "Array of chipIDs of multi-chips");

/******************************************************************************
 *                           driver function                                  *
 ******************************************************************************/

static int fastClear = -1;
module_param(fastClear, int, 0644);
MODULE_PARM_DESC(fastClear, "Disable fast clear if set it to 0, enabled by default");

static int compression = -1;
module_param(compression, int, 0644);
MODULE_PARM_DESC(compression, "Disable compression if set it to 0, enabled by default");

static int powerManagement = 0;
module_param(powerManagement, int, 0644);
MODULE_PARM_DESC(powerManagement, "Disable auto power saving if set it to 0, enabled by default");

static uint recovery = 0;
module_param(recovery, uint, 0644);
MODULE_PARM_DESC(recovery, "Recover GPU from stuck (1: Enable, 0: Disable)");

/*
 * Level of stuck dump content, 0~5 and 11~15.
 * 0: Disable. 1: Dump nearby memory. 2: Dump user command.
 * 3: Commit stall besides level2. 4: Dump kernel command buffer besides level3.
 * 5: Dump all the cores with level4.
 *
 * Level 1~5 + 10 means force dump, whether the recovery is enabled or not.
 */
static uint stuckDump = 0;
module_param(stuckDump, uint, 0644);
MODULE_PARM_DESC(stuckDump, "Level of stuck dump content.");

/*
 * Level of debug zone.
 * 0: Disable.  1: debug level verbose and zone all.
 */
static uint debugLevel = 0;
module_param(debugLevel, uint, 0644);
MODULE_PARM_DESC(debugLevel, "Level of debug.");

static int showArgs = 1;
module_param(showArgs, int, 0644);
MODULE_PARM_DESC(showArgs, "Display parameters value when driver loaded");

static uint userClusterMasks[gcdMAX_MAJOR_CORE_COUNT] = {
    [0 ... gcdMAX_MAJOR_CORE_COUNT - 1] = 0xf
};
module_param_array(userClusterMasks, uint, NULL, 0644);
MODULE_PARM_DESC(userClusterMasks, "Array of user defined per-core cluster enable mask");

static uint enableNN = 0xFF;
module_param(enableNN, uint, 0644);
MODULE_PARM_DESC(enableNN, "How many NN cores will be enabled in one VIP, 0xFF means all enabled, 0 means all disabled, 1 means enable 1 NN core...");

/* GPU small batch feature. */
static int smallBatch = 1;
module_param(smallBatch, int, 0644);
MODULE_PARM_DESC(smallBatch, "Enable/disable GPU small batch feature, enable by default");

static int allMapInOne = 1;
module_param(allMapInOne, int, 0644);
MODULE_PARM_DESC(allMapInOne, "Mapping kernel video memory to user, 0 means mapping every time, otherwise only mapping one time");

static uint softReset = 1;
module_param(softReset, uint, 0644);
MODULE_PARM_DESC(softReset, "Disable soft reset when insert the driver if set it to 0, enabled by default.");

/******************************************************************************
 *                            SRAM related                                    *
 ******************************************************************************/

static uint sRAMLoopMode = 0;
module_param(sRAMLoopMode, uint, 0644);
MODULE_PARM_DESC(sRAMLoopMode, "Default 0 means SRAM pool must be specified when allocating SRAM memory, 1 means SRAM memory will be looped as default pool.");

/*
 * Per-core SRAM physical address base, the order of configuration is according
 * to access speed, gcvINVALID_PHYSICAL_ADDRESS means no bus address.
 */
static gctPHYS_ADDR_T sRAMBases[gcvSRAM_INTER_COUNT * gcdGLOBAL_CORE_COUNT] = {
    [0 ... gcvSRAM_INTER_COUNT * gcdGLOBAL_CORE_COUNT - 1] = gcvINVALID_PHYSICAL_ADDRESS
};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
module_param_array(sRAMBases, ullong, NULL, 0644);
MODULE_PARM_DESC(sRAMBases, "Array of base of bus address of SRAM,INTERNAL, EXTERNAL0, EXTERNAL1..., gcvINVALID_PHYSICAL_ADDRESS means no bus address");
#endif

/* Per-core SRAM size. */
static uint sRAMSizes[gcvSRAM_INTER_COUNT * gcdGLOBAL_CORE_COUNT] = {
    [0 ... gcvSRAM_INTER_COUNT * gcdGLOBAL_CORE_COUNT - 1] = 0
};
module_param_array(sRAMSizes, uint, NULL, 0644);
MODULE_PARM_DESC(sRAMSizes, "Array of size of per-core SRAMs, 0 means no SRAM");

/* Shared SRAM physical address bases. */
static gctPHYS_ADDR_T extSRAMBases[gcvSRAM_EXT_COUNT] = {
    [0 ... gcvSRAM_EXT_COUNT - 1] = gcvINVALID_PHYSICAL_ADDRESS
};
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
module_param_array(extSRAMBases, ullong, NULL, 0644);
MODULE_PARM_DESC(extSRAMBases, "Shared SRAM physical address bases.");
#endif

/* Shared SRAM sizes. */
static uint extSRAMSizes[gcvSRAM_EXT_COUNT] = {[0 ... gcvSRAM_EXT_COUNT - 1] = 0};
module_param_array(extSRAMSizes, uint, NULL, 0644);
MODULE_PARM_DESC(extSRAMSizes, "Shared SRAM sizes.");

static uint sRAMRequested = 1;
module_param(sRAMRequested, uint, 0644);
MODULE_PARM_DESC(sRAMRequested, "Default 1 means AXI-SRAM is already reserved for GPU, 0 means GPU driver need request the memory region.");

/******************************************************************************
 *                             mmu related                                    *
 ******************************************************************************/

static int mmu = 1;
module_param(mmu, int, 0644);
MODULE_PARM_DESC(mmu, "Disable MMU if set it to 0, enabled by default");

static uint flatMapping = 1;
module_param(flatMapping, int, 0644);
MODULE_PARM_DESC(flatMapping, "Flat mapping for reserve memory, enabled by default");

static uint mmuPageTablePool = 0;
module_param(mmuPageTablePool, uint, 0644);
MODULE_PARM_DESC(mmuPageTablePool, "Default 1 means alloc mmu page table in virtual memory(external if PCIE), 0 means auto select memory pool.");

static uint mmuCmdPool = 1;
module_param(mmuCmdPool, uint, 0644);
MODULE_PARM_DESC(mmuCmdPool, "Default 1 means auto select memory pool to allocate mmu initial command, specific the pool type with gcvPOOL_XXXX");

static uint mmuDynamicMap = 1;
module_param(mmuDynamicMap, uint, 0644);
MODULE_PARM_DESC(mmuDynamicMap, "Default 1 means enable mmu dynamic mapping in virsual memory, 0 means disable dynnamic mapping.");

static uint sharedPageTable = 1;
module_param(sharedPageTable, uint, 0644);
MODULE_PARM_DESC(sharedPageTable, "Default 1 means all the cores in one HW device share one MMU page table, 0 means per core page table.");

/*
 * If hardware do not support switch mmu, driver will set back processPageTable to 0.
 * So if want to enable processPageTable with switch mmu by software, need to modify code in kernel construct.
 */
static uint processPageTable = 1;
module_param(processPageTable, uint, 0644);
MODULE_PARM_DESC(processPageTable, "Default 0 means the page table is shared by all the cores in one device or per-core. 1 means the page table is per-process.");

/******************************************************************************
 *                            PCIE related                                    *
 ******************************************************************************/

#if USE_LINUX_PCIE
static int bar = 1;
module_param(bar, int, 0644);
MODULE_PARM_DESC(bar, "PCIE Bar index of GC core");

static int bars[gcdGLOBAL_CORE_COUNT] = {[0 ... gcdGLOBAL_CORE_COUNT - 1] = -1};
module_param_array(bars, int, NULL, 0644);
MODULE_PARM_DESC(bars, "Array of bar index of PCIE platform for multi-GPU");

static uint regOffsets[gcdGLOBAL_CORE_COUNT] = {
    [0 ... gcdGLOBAL_CORE_COUNT - 1] = 0
};
module_param_array(regOffsets, uint, NULL, 0644);
MODULE_PARM_DESC(regOffsets, "Array of register offsets in corresponding BAR space");

static int sRAMBars[gcvSRAM_EXT_COUNT] = {[0 ... gcvSRAM_EXT_COUNT - 1] = -1};
module_param_array(sRAMBars, int, NULL, 0644);
MODULE_PARM_DESC(sRAMBars, "Array of SRAM bar index of shared external SRAMs.");

static int sRAMOffsets[gcvSRAM_EXT_COUNT] = {[0 ... gcvSRAM_EXT_COUNT - 1] = -1};
module_param_array(sRAMOffsets, int, NULL, 0644);
MODULE_PARM_DESC(sRAMOffsets, "Array of SRAM offset inside bar of shared external SRAMs.");

#endif

/******************************************************************************
 *                               others                                       *
 ******************************************************************************/

static uint major = 199;
module_param(major, uint, 0644);
MODULE_PARM_DESC(major, "major device number for GC device");

static uint type = 1;
module_param(type, uint, 0664);
MODULE_PARM_DESC(type, "0 - Char Driver (Default), 1 - Misc Driver");

static uint vGPUType;
module_param(vGPUType, uint, 0664);
MODULE_PARM_DESC(type, "0 - vGPU is disabled(Default), 1 - MDEV vGPU, 2 - SRIOV vGPU");

static uint vGPUId;
module_param(vGPUId, uint, 0664);
MODULE_PARM_DESC(type, "The vGPU id will be used set for debug register when the vGPU is enabled");

static uint cmdQueueSizeByPage = 1;
module_param(cmdQueueSizeByPage, uint, 0664);
MODULE_PARM_DESC(cmdQueueSizeByPage, "The each kernel command queue page number");

#endif /* _GC_HAL_KERNEL_PARAMETER_H_ */
