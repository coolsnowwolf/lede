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


#include <linux/device.h>
#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>

#include "gc_hal_kernel_linux.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel_parameter.h"

#include <linux/platform_device.h>

/* Zone used for header/footer. */
#define _GC_OBJ_ZONE gcvZONE_DRIVER

MODULE_DESCRIPTION("Phytium Graphics Driver");
MODULE_VERSION(gcvVERSION_STRING);
MODULE_LICENSE("Dual MIT/GPL");
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
MODULE_IMPORT_NS(VFS_internal_I_am_really_a_filesystem_and_am_NOT_a_driver);
#endif

static struct class *gpu_class = gcvNULL;

static gcsPLATFORM *platform;

static gckGALDEVICE galDevice;

static int gpu3DMinClock = 1;
static int contiguousRequested;
static ulong bankSize;

static struct miscdevice *gal_misc_device[gcdDEVICE_COUNT];

static uint32_t
viv_get_dev_index(uint32_t minor)
{
    int dev_index = 0;

    for (dev_index = 0; dev_index < galDevice->args.devCount; dev_index++) {
        if (galDevice->devIDs[dev_index] == minor)
            return dev_index;
    }

    /* If not found the correct device index. */
    return gcdDEVICE_COUNT;
}

static void
_InitModuleParam(gcsMODULE_PARAMETERS *ModuleParam)
{
    gctUINT i, j;
    gcsMODULE_PARAMETERS *p = ModuleParam;

    for (i = 0; i < gcdDEVICE_COUNT; i++)
        p->devices[i] = gcvNULL;

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        p->irqs[i] = irqs[i];
        if (irqs[i] != -1) {
            p->registerBases[i] = registerBases[i];
            p->registerSizes[i] = registerSizes[i];
        }
#if USE_LINUX_PCIE
        p->bars[i] = bars[i];
        p->regOffsets[i] = regOffsets[i];
#endif
        p->registerBasesMapped[i] = gcvNULL;

        for (j = 0; j < gcvSRAM_INTER_COUNT; j++) {
            p->sRAMBases[i][j] = sRAMBases[i * gcvSRAM_INTER_COUNT + j];
            p->sRAMSizes[i][j] = sRAMSizes[i * gcvSRAM_INTER_COUNT + j];
        }
    }

    for (i = 0; i < gcvCORE_COUNT; i++)
        p->chipIDs[i] = chipIDs[i];

    if (irqLine != -1) {
        p->irqs[gcvCORE_MAJOR] = irqLine;
        p->registerBases[gcvCORE_MAJOR] = registerMemBase;
        p->registerSizes[gcvCORE_MAJOR] = registerMemSize;
        /* Check legacy style. */
#if USE_LINUX_PCIE
        if (bar != -1) {
            if (p->bars[gcvCORE_MAJOR] == -1)
                p->bars[gcvCORE_MAJOR] = bar;
        }
#endif
    }

#if gcdDEC_ENABLE_AHB
    if (registerMemBaseDEC300 && registerMemSizeDEC300) {
        p->registerBases[gcvCORE_DEC] = registerMemBaseDEC300;
        p->registerSizes[gcvCORE_DEC] = registerMemSizeDEC300;
    }
#endif

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        p->contiguousBases[i] = contiguousBases[i];
        p->contiguousSizes[i] = contiguousSizes[i];
    }

    p->contiguousBase = contiguousBase;
    p->contiguousSize = contiguousSize;

    if (!p->contiguousSizes[0] && contiguousSize)
        p->contiguousSizes[0] = contiguousSize;

    p->contiguousRequested = contiguousRequested; /* not a module param. */

    p->devCount = 0;

    for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
        p->externalBase[i]  = externalBase[i];
        p->externalSize[i]  = externalSize[i];
        p->exclusiveBase[i] = exclusiveBase[i];
        p->exclusiveSize[i] = exclusiveSize[i];
    }

    for (i = 0; i < gcdPLATFORM_COUNT; i++)
        p->hwDevCounts[i] = hwDevCounts[i];

    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        p->devMemIDs[i] = devMemIDs[i];
        p->devSysMemIDs[i] = devSysMemIDs[i];
        p->devSRAMIDs[i] = devSRAMIDs[i];
        p->devCoreCounts[i] = devCoreCounts[i];
    }

    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        p->extSRAMBases[i] = extSRAMBases[i];
        p->extSRAMSizes[i] = extSRAMSizes[i];
#if USE_LINUX_PCIE
        p->sRAMBars[i] = sRAMBars[i];
        p->sRAMOffsets[i] = sRAMOffsets[i];
#endif
    }

    for (i = 0; i < gcdMAX_MAJOR_CORE_COUNT; i++)
        p->userClusterMasks[i] = userClusterMasks[i];

    p->sRAMRequested = sRAMRequested;
    p->sRAMLoopMode  = sRAMLoopMode;

    p->bankSize    = bankSize; /* not a module param. */

    p->recovery = recovery;
    p->powerManagement = powerManagement;

    p->softReset = softReset;

    p->enableMmu = mmu;
    p->fastClear = fastClear;

    p->compression = (compression == -1) ?
                     gcvCOMPRESSION_OPTION_DEFAULT : (gceCOMPRESSION_OPTION)compression;
    p->gpu3DMinClock = gpu3DMinClock; /* not a module param. */
    p->enableNN      = enableNN;
    p->registerAPB   = registerAPB;
    p->smallBatch    = smallBatch;

    p->stuckDump  = stuckDump;

    p->deviceType = type;
    p->showArgs   = showArgs;

    p->mmuPageTablePool = mmuPageTablePool;
    p->mmuCmdPool       = mmuCmdPool;
    p->sharedPageTable  = sharedPageTable;
    p->processPageTable = processPageTable;
    p->flatMapping      = flatMapping;

    p->mmuDynamicMap = mmuDynamicMap;
    p->allMapInOne   = allMapInOne;

    p->vGPUType = vGPUType;
    p->vGPUId   = vGPUId;
    p->lockLargeVA = lockLargeVA;

    p->isrPoll = isrPoll;

    p->cmdQueueSizeByPage = cmdQueueSizeByPage;
}

static void
_SyncModuleParam(gcsMODULE_PARAMETERS *ModuleParam)
{
    gctUINT i, j;
    gcsMODULE_PARAMETERS *p = ModuleParam;

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        irqs[i]          = p->irqs[i];
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
        registerBases[i] = p->registerBases[i];
#else
        registerBases[i] = (ulong)p->registerBases[i];
#endif
        registerSizes[i] = (ulong)p->registerSizes[i];
#if USE_LINUX_PCIE
        bars[i]          = p->bars[i];
        regOffsets[i]    = p->regOffsets[i];
#endif
        for (j = 0; j < gcvSRAM_INTER_COUNT; j++) {
            sRAMBases[i * gcvSRAM_INTER_COUNT + j] = p->sRAMBases[i][j];
            sRAMSizes[i * gcvSRAM_INTER_COUNT + j] = p->sRAMSizes[i][j];
        }
    }

    for (i = 0; i < gcvCORE_COUNT; i++)
        chipIDs[i] = p->chipIDs[i];

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
        contiguousBases[i] = p->contiguousBases[i];
#else
        contiguousBases[i] = (ulong)p->contiguousBases[i];
#endif

        contiguousSizes[i] = (ulong)p->contiguousSizes[i];
    }

    if (p->contiguousSize)
        p->contiguousSizes[0] = contiguousSizes[0] = p->contiguousSize;

    if (p->contiguousBase)
        p->contiguousBases[0] = contiguousBases[0] = p->contiguousBase;

    contiguousRequested = p->contiguousRequested;   /* not a module param. */

    for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
        externalBase[i]  = p->externalBase[i];
        externalSize[i]  = p->externalSize[i];
        exclusiveBase[i] = p->exclusiveBase[i];
        exclusiveSize[i] = p->exclusiveSize[i];
    }

    for (i = 0; i < gcdPLATFORM_COUNT; i++)
        hwDevCounts[i] = p->hwDevCounts[i];

    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        platformIDs[i] = p->platformIDs[i];
        devMemIDs[i] = p->devMemIDs[i];
        devSysMemIDs[i] = p->devSysMemIDs[i];
        devSRAMIDs[i] = p->devSRAMIDs[i];
        devCoreCounts[i] = p->devCoreCounts[i];

        if (p->devCoreCounts[i])
            p->devCount++;
    }

    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        extSRAMBases[i] = p->extSRAMBases[i];
        extSRAMSizes[i] = p->extSRAMSizes[i];

#if USE_LINUX_PCIE
        sRAMBars[i] = p->sRAMBars[i];
        sRAMOffsets[i] = p->sRAMOffsets[i];
#endif
    }

    for (i = 0; i < gcdMAX_MAJOR_CORE_COUNT; i++)
        userClusterMasks[i] = p->userClusterMasks[i];

    sRAMRequested = p->sRAMRequested;
    sRAMLoopMode  = p->sRAMLoopMode;

    bankSize    = p->bankSize; /* not a module param. */

    recovery = p->recovery;
    powerManagement = p->powerManagement;

    mmu           = p->enableMmu;
    fastClear     = p->fastClear;
    compression   = p->compression;
    gpu3DMinClock = p->gpu3DMinClock; /* not a module param. */
    enableNN      = p->enableNN;
    registerAPB   = p->registerAPB;
    smallBatch    = p->smallBatch;

    stuckDump = p->stuckDump;
    type      = p->deviceType;
    showArgs  = p->showArgs;

    mmuPageTablePool = p->mmuPageTablePool;
    mmuCmdPool       = p->mmuCmdPool;
    sharedPageTable  = p->sharedPageTable;
    processPageTable = p->processPageTable;
    flatMapping      = p->flatMapping;
    mmuDynamicMap    = p->mmuDynamicMap;
    allMapInOne      = p->allMapInOne;
    isrPoll          = p->isrPoll;

    vGPUType         = p->vGPUType;
    vGPUId           = p->vGPUId;
    lockLargeVA      = p->lockLargeVA;
}

void
gckOS_DumpParam(void)
{
    gctINT i;
    gcsMODULE_PARAMETERS *p = &platform->params;

    pr_warn("ftg340 options:\n");

#if gcdDEC_ENABLE_AHB
    pr_warn("  registerMemBaseDEC300 = 0x%08lX\n", registerMemBaseDEC300);
    pr_warn("  registerMemSizeDEC300 = 0x%08lX\n", registerMemSizeDEC300);
#endif

    pr_warn("  bankSize          = 0x%08lX\n", bankSize);
    pr_warn("  fastClear         = %d\n",      fastClear);
    pr_warn("  compression       = %d\n",      compression);
    pr_warn("  powerManagement   = %d\n",      powerManagement);
    pr_warn("  recovery          = %d\n",      recovery);
    pr_warn("  stuckDump         = %d\n",      stuckDump);
    pr_warn("  GPU smallBatch    = %d\n",      smallBatch);
    pr_warn("  allMapInOne       = %d\n",      allMapInOne);
    pr_warn("  enableNN          = 0x%x\n",    enableNN);

    pr_warn("  userClusterMasks  = ");
    for (i = 0; i < gcdMAX_MAJOR_CORE_COUNT; i++)
        pr_warn("%x, ", userClusterMasks[i]);
    pr_warn("\n");

    pr_warn("  irqs              = ");
    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        if (irqs[i] != -1)
            pr_warn("%d, ", irqs[i]);
    }

    pr_warn("\n");

#if USE_LINUX_PCIE
    pr_warn("Bars configuration: \n");

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        if (bars[i] != -1) {
            pr_warn("  bars[%d] = %d, regOffsets[%d] = %x, ",
                    i, bars[i], i, regOffsets[i]);
            pr_warn("\n");
        }
    }
#endif

    pr_warn("System reserve memory configuration: \n");

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (contiguousSizes[i]) {
            pr_warn("  contiguousSizes[%d] = 0x%lx\n", i, contiguousSizes[i]);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
            pr_warn("  contiguousBases[%d] = 0x%llX\n", i, contiguousBases[i]);
#else
            pr_warn("  contiguousBases[%d] = 0x%lX\n", i, contiguousBases[i]);
#endif
        }
    }

    pr_warn("Registers configuration: \n");

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        if (registerSizes[i]) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 17, 0)
            pr_warn("  bases[%d] = 0x%llx, sizes[%d] = 0x%lx, ",
                    i, registerBases[i], i, registerSizes[i]);
#else
            pr_warn("  bases[%d] = 0x%lx, sizes[%d] = 0x%lx, ",
                    i, registerBases[i], i, registerSizes[i]);
#endif
            pr_warn("\n");
        }
    }

    pr_warn("  External sRAMBases = ");
    for (i = 0; i < gcvSRAM_EXT_COUNT; i++)
        pr_warn("0x%llx, ", extSRAMBases[i]);
    pr_warn("\n");

    pr_warn("  mmu               = %d\n", mmu);
    pr_warn("  mmuPageTablePool  = %d\n", mmuPageTablePool);
    pr_warn("  mmuCmdPool        = %d\n", mmuCmdPool);
    pr_warn("  mmuDynamicMap     = %d\n", mmuDynamicMap);
    pr_warn("  isrPoll           = 0x%08X\n", isrPoll);
    pr_warn("  cmdQueueSizeByPage = %d\n", p->cmdQueueSizeByPage);

    pr_warn("Build options:\n");
    pr_warn("  gcdGPU_TIMEOUT    = %d\n", gcdGPU_TIMEOUT);
    pr_warn("  gcdINTERRUPT_STATISTIC = %d\n", gcdINTERRUPT_STATISTIC);
}

static int drv_open(struct inode *inode, struct file *filp)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_PRIVATE_DATA_PTR data = gcvNULL;
    gctUINT i, dev_index;
    gctUINT attached = 0;
    gckDEVICE device = gcvNULL;

    gcmkHEADER_ARG("inode=%p filp=%p", inode, filp);

    dev_index = iminor(file_inode(filp));

    if (type == 1)
        dev_index = viv_get_dev_index(dev_index);

    if (dev_index >= gcdDEVICE_COUNT)
        return -ENODEV;

    data = kmalloc(sizeof(gcsHAL_PRIVATE_DATA), GFP_KERNEL | __GFP_NOWARN);

    if (data == gcvNULL) {
        gcmkFOOTER_ARG("status=%d", gcvSTATUS_OUT_OF_MEMORY);
        return -ENOMEM;
    }

    data->isLocked = gcvFALSE;
    data->device   = galDevice;
    data->pidOpen  = _GetProcessID();

    device = galDevice->devices[dev_index];

    /* Attached the process. */
    for (i = 0; i < gcvCORE_COUNT; i++) {
        if (device->kernels[i]) {
            status = gckKERNEL_AttachProcess(device->kernels[i], gcvTRUE);

            if (gcmIS_ERROR(status))
                goto OnError;

            attached = i;
        }
    }

    filp->private_data = data;

    /* Success. */
    gcmkFOOTER_NO();
    return 0;

OnError:
    for (i = 0; i < attached; i++) {
        if (device && device->kernels[i])
            gcmkVERIFY_OK(gckKERNEL_AttachProcess(device->kernels[i], gcvFALSE));
    }

    kfree(data);

    gcmkFOOTER_ARG("status=%d", status);
    return -ENOTTY;
}

static int drv_release(struct inode *inode, struct file *filp)
{
    int ret = -ENOTTY;
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_PRIVATE_DATA_PTR data;
    gckGALDEVICE gal_device = gcvNULL;
    gckDEVICE device = gcvNULL;
    gctUINT i, dev_index;

    gcmkHEADER_ARG("inode=%p filp=%p", inode, filp);

    dev_index = iminor(file_inode(filp));

    if (type == 1)
        dev_index = viv_get_dev_index(dev_index);

    if (dev_index >= gcdDEVICE_COUNT)
        return -ENODEV;

    data = filp->private_data;

    if (data == gcvNULL) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): private_data is NULL\n",
                       __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gal_device = data->device;

    if (gal_device == gcvNULL) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): device is NULL\n",
                       __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (data->isLocked) {
        device = gal_device->devices[data->devIndex];

        /* Release the mutex. */
        gcmkONERROR(gckOS_ReleaseMutex(device->os, device->commitMutex));
        data->isLocked = gcvFALSE;
    }

    /* A process gets detached. */
    device = gal_device->devices[dev_index];

    /* A process gets detached. */
    for (i = 0; i < gcvCORE_COUNT; i++) {
        if (device && device->kernels[i]) {
            gcmkVERIFY_OK(gckKERNEL_AttachProcessEx(device->kernels[i],
                                                    gcvFALSE, data->pidOpen));
        }
    }

    kfree(data);
    filp->private_data = NULL;

    /* Success. */
    ret = 0;

OnError:
    gcmkFOOTER();
    return ret;
}

static long drv_ioctl(struct file *filp, unsigned int ioctlCode, unsigned long arg)
{
    long ret = -ENOTTY;
    gceSTATUS status = gcvSTATUS_OK;
    gcsHAL_INTERFACE iface = {0};
    gctUINT32 copyLen;
    DRIVER_ARGS drvArgs;
    gckGALDEVICE gal_device;
    gckDEVICE device;
    gcsHAL_PRIVATE_DATA_PTR data;
    gctUINT dev_index;
    gctUINT32 copy_size;
#if PHYTIUM_PROFILER
    static gcsHAL_PROFILER_INTERFACE iface_profiler = {0};
#endif

    gcmkHEADER_ARG("filp=%p ioctlCode=%u arg=%lu", filp, ioctlCode, arg);

    dev_index = iminor(file_inode(filp));

    if (type == 1)
        dev_index = viv_get_dev_index(dev_index);

    if (dev_index >= gcdDEVICE_COUNT)
        return -ENODEV;

    data = filp->private_data;

    if (data == gcvNULL) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): private_data is NULL\n",
                       __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    gal_device = data->device;

    if (gal_device->gotoShutdown)
	    return -ENODEV;

    if (gal_device == gcvNULL) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): device is NULL\n",
                       __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    switch (ioctlCode) {
    case IOCTL_GCHAL_INTERFACE:
        /* Get the drvArgs. */
        copyLen = copy_from_user(&drvArgs, (void *)arg, sizeof(DRIVER_ARGS));

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of the input arguments.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

#if !gcdIGNORE_DRIVER_VERSIONS_MISMATCH
        /* Now bring in the gcsHAL_INTERFACE structure. */
        if (drvArgs.InputBufferSize != sizeof(gcsHAL_INTERFACE) ||
            drvArgs.OutputBufferSize != sizeof(gcsHAL_INTERFACE)) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): input or/and output structures are invalid.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        copy_size = sizeof(gcsHAL_INTERFACE);
#else
        copy_size = gcmMIN(drvArgs.InputBufferSize, sizeof(gcsHAL_INTERFACE));
#endif /* !gcdIGNORE_DRIVER_VERSIONS_MISMATCH */

        copyLen = copy_from_user(&iface,
                                 gcmUINT64_TO_PTR(drvArgs.InputBuffer),
                                 copy_size);

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of input HAL interface.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (iface.command == gcvHAL_DEVICE_MUTEX) {
            if (iface.u.DeviceMutex.isMutexLocked == gcvTRUE)
                data->isLocked = gcvTRUE;
            else
                data->isLocked = gcvFALSE;
        }

        /* Record the last device id. */
#if gcdENABLE_MULTI_DEVICE_MANAGEMENT
        if (!dev_index && dev_index != iface.devIndex)
            dev_index = iface.devIndex;
#endif

        data->devIndex = dev_index;

        if (iface.command == gcvHAL_CHIP_INFO) {
            gctUINT i, count = 0;

            for (i = 0; i < gal_device->args.devCount; i++) {
                device = gal_device->devices[i];

                gcmkONERROR(gckDEVICE_ChipInfo(device, &iface, &count));
            }
        } else
        {
#if gcdENABLE_MULTI_DEVICE_MANAGEMENT
            if (iface.devIndex >= gcdDEVICE_COUNT)
                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
#endif

            device = gal_device->devices[dev_index];

            status = gckDEVICE_Dispatch(device, &iface);

            /* Redo system call after pending signal is handled. */
            if (status == gcvSTATUS_INTERRUPTED) {
                ret = -ERESTARTSYS;
                goto OnError;
            }
        }

        /* Copy data back to the user. */
        copyLen = copy_to_user(gcmUINT64_TO_PTR(drvArgs.OutputBuffer),
                               &iface,
                               copy_size);

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of output HAL interface.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        break;

    case IOCTL_GCHAL_PROFILER_INTERFACE:
#if PHYTIUM_PROFILER
        /* Get the drvArgs. */
        copyLen = copy_from_user(&drvArgs, (void *)arg, sizeof(DRIVER_ARGS));

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of the input arguments.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

#if !gcdIGNORE_DRIVER_VERSIONS_MISMATCH
        /* Now bring in the gcsHAL_INTERFACE structure. */
        if (drvArgs.InputBufferSize != sizeof(gcsHAL_PROFILER_INTERFACE) ||
            drvArgs.OutputBufferSize != sizeof(gcsHAL_PROFILER_INTERFACE)) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): input or/and output structures are invalid.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        copy_size = sizeof(gcsHAL_PROFILER_INTERFACE);
#else
        copy_size = gcmMIN(drvArgs.InputBufferSize, sizeof(gcsHAL_PROFILER_INTERFACE));
#endif /* !gcdIGNORE_DRIVER_VERSIONS_MISMATCH */

        copyLen = copy_from_user(&iface_profiler,
                                 gcmUINT64_TO_PTR(drvArgs.InputBuffer),
                                 copy_size);

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of input HAL interface.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        device = gal_device->devices[dev_index];

        status = gckDEVICE_Profiler_Dispatch(device, &iface_profiler);

        /* Redo system call after pending signal is handled. */
        if (status == gcvSTATUS_INTERRUPTED) {
            ret = -ERESTARTSYS;
            goto OnError;
        }

        /* Copy data back to the user. */
        copyLen = copy_to_user(gcmUINT64_TO_PTR(drvArgs.OutputBuffer),
                               &iface_profiler,
                               copy_size);

        if (copyLen != 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): error copying of output HAL interface.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
#endif
        break;

    default:
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): unknown command %d\n",
                       __func__, __LINE__, ioctlCode);

        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Success. */
    ret = 0;

OnError:
    gcmkFOOTER();
    return ret;
}

static const struct file_operations driver_fops = {
    .owner = THIS_MODULE,
    .open = drv_open,
    .release = drv_release,
    .unlocked_ioctl = drv_ioctl,

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 8, 18)

#ifdef HAVE_COMPAT_IOCTL
    .compat_ioctl = drv_ioctl,
# endif

#else
    .compat_ioctl = drv_ioctl,
#endif
};

gceSTATUS viv_misc_device_node_create(uint32_t dev_index)
{
    gceSTATUS status = gcvSTATUS_OK;
    struct miscdevice *dev = kzalloc(sizeof(struct miscdevice), GFP_KERNEL);
    gctINT32 ret;

    if (!dev) {
        ret = -ENOMEM;

        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): misc_register fails.\n",
                        __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    dev->minor = MISC_DYNAMIC_MINOR;
    dev->fops = &driver_fops;
    dev->mode = 0666;
    if (dev_index > 0)
        dev->name = kasprintf(GFP_KERNEL, "ftg340%d", dev_index);
    else
        dev->name = kasprintf(GFP_KERNEL, "ftg340");

    if (!dev->name) {
        ret = -ENOMEM;

        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): misc_register fails.\n",
                        __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    ret = misc_register(dev);
    if (ret != 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): misc_register fails.\n",
                        __func__, __LINE__);

        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
    }

    gal_misc_device[dev_index] = dev;
    galDevice->devIDs[dev_index] = dev->minor;

    return gcvSTATUS_OK;

OnError:
    /* If one node loads failed, try to release all the node resource. */
    for (; dev_index >= 0; dev_index--) {
        if (gal_misc_device[dev_index]) {
            misc_deregister(gal_misc_device[dev_index]);
            kfree(gal_misc_device[dev_index]->name);
            gal_misc_device[dev_index]->name = gcvNULL;
            kfree(gal_misc_device[dev_index]);
            gal_misc_device[dev_index] = gcvNULL;
        }
    }

    return status;
}

void viv_misc_device_node_destroy(uint32_t dev_index)
{
    gcmkASSERT(gal_misc_device[dev_index]);
    misc_deregister(gal_misc_device[dev_index]);
    kfree(gal_misc_device[dev_index]->name);
    gal_misc_device[dev_index]->name = gcvNULL;
    kfree(gal_misc_device[dev_index]);
    gal_misc_device[dev_index] = gcvNULL;
}

void viv_char_device_node_destroy(uint32_t dev_index)
{
    gcmkASSERT(gpu_class != gcvNULL);
    device_destroy(gpu_class, MKDEV(major, dev_index));
}

gceSTATUS viv_char_device_node_create(uint32_t dev_index)
{
    gceSTATUS status = gcvSTATUS_OK;
    int result = -EINVAL;

    gcmkHEADER();

    if (!gpu_class) {
        /* Register the character device. */
        result = register_chrdev(major, DEVICE_NAME, &driver_fops);

        if (result < 0) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): Could not allocate major number for mmap.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);
        }

        if (major == 0)
            major = result;

        /* Create the device class. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 4, 0)
        gpu_class = class_create(CLASS_NAME);
#else
        gpu_class = class_create(THIS_MODULE, CLASS_NAME);
#endif

        if (IS_ERR(gpu_class)) {
            gpu_class = gcvNULL;
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): Failed to create the class.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }
    }

    if (dev_index > 0) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        device_create(gpu_class, NULL, MKDEV(major, dev_index), NULL, "ftg340%d", dev_index);
#else
        device_create(gpu_class, NULL, MKDEV(major, dev_index), "ftg340%d", dev_index);
#endif
    } else { /* Compatible with old style. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 27)
        device_create(gpu_class, NULL, MKDEV(major, dev_index), NULL, DEVICE_NAME);
#else
        device_create(gpu_class, NULL, MKDEV(major, dev_index), DEVICE_NAME);
#endif
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (gpu_class)
        class_destroy(gpu_class);

    if (result < 0)
        unregister_chrdev(result, DEVICE_NAME);

    gcmkFOOTER();
    return status;
}

static int drv_init(void)
{
    gceSTATUS status;
    gckGALDEVICE device = gcvNULL;
    gctUINT dev_index = 0;

    gcmkHEADER();

    pr_info("ftg340 version %s\n", gcvVERSION_STRING);

    if (showArgs)
        gckOS_DumpParam();

    /* Create the GAL device. */
    status = gckGALDEVICE_Construct(platform, &platform->params, &device);

    if (gcmIS_ERROR(status)) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): Failed to create the GAL device: status=%d\n",
                       __func__, __LINE__, status);

        goto OnError;
    }

    /* Start the GAL device. */
    gcmkONERROR(gckGALDEVICE_Start(device));

    /* Set global galDevice pointer. */
    galDevice = device;

    for (dev_index = 0; dev_index < device->args.devCount; dev_index++) {
        if (type == 1)
            gcmkONERROR(viv_misc_device_node_create(dev_index));
        else
            gcmkONERROR(viv_char_device_node_create(dev_index));
    }
    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    if (device) {
        gcmkVERIFY_OK(gckGALDEVICE_Stop(device));
        gcmkVERIFY_OK(gckGALDEVICE_Destroy(device));
    }

    gcmkFOOTER();
    return status;
}

static void drv_exit(void)
{
    gctUINT dev_index;

    gcmkHEADER();

    if (type == 1) {
        for (dev_index = 0; dev_index < galDevice->args.devCount; dev_index++)
            viv_misc_device_node_destroy(dev_index);
    } else {
        gcmkASSERT(gpu_class != gcvNULL);

        for (dev_index = 0; dev_index < galDevice->args.devCount; dev_index++)
            viv_char_device_node_destroy(dev_index);

        class_destroy(gpu_class);
        gpu_class = gcvNULL;

        unregister_chrdev(major, DEVICE_NAME);
    }

    gcmkVERIFY_OK(gckGALDEVICE_Stop(galDevice));
    gcmkVERIFY_OK(gckGALDEVICE_Destroy(galDevice));

    gcmkFOOTER_NO();
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static int viv_dev_probe(struct platform_device *pdev)
#else
static int __devinit viv_dev_probe(struct platform_device *pdev)
#endif
{
    int  ret = -ENODEV;
    bool getPowerFlag = gcvFALSE;
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
    static u64 dma_mask = DMA_BIT_MASK(40);
#else
    static u64 dma_mask = DMA_40BIT_MASK;
#endif

#if gcdCAPTURE_ONLY_MODE
    gctPHYS_ADDR_T contiguousBaseCap = 0;
    gctSIZE_T contiguousSizeCap = 0;
    gctPHYS_ADDR_T sRAMBaseCap[gcdGLOBAL_CORE_COUNT][gcvSRAM_INTER_COUNT];
    gctUINT32 sRAMSizeCap[gcdGLOBAL_CORE_COUNT][gcvSRAM_INTER_COUNT];
    gctPHYS_ADDR_T extSRAMBaseCap[gcvSRAM_EXT_COUNT];
    gctUINT32 extSRAMSizeCap[gcvSRAM_EXT_COUNT];
    gctUINT i = 0, j = 0;
#endif

    gcmkHEADER();

    platform->device = pdev;

    if (!mmu) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
        dma_mask = DMA_BIT_MASK(32);
#else
        dma_mask = DMA_32BIT_MASK;
#endif
    }

    if (platform->ops->getPower) {
        if (gcmIS_ERROR(platform->ops->getPower(platform))) {
            gcmkFOOTER_NO();
            return ret;
        }
        getPowerFlag = gcvTRUE;
    }

    /* Gather module parameters. */
    _InitModuleParam(&platform->params);

    platform->params.devices[0] = &pdev->dev;

    platform->params.devices[0]->coherent_dma_mask = dma_mask;

    if (platform->params.devices[0]->dma_mask)
        *platform->params.devices[0]->dma_mask = dma_mask;
    else
        platform->params.devices[0]->dma_mask = &platform->params.devices[0]->coherent_dma_mask;

    if (platform->ops->dmaInit) {
        if (gcmIS_ERROR(platform->ops->dmaInit(platform))) {
            gcmkFOOTER_NO();
            return ret;
        }
    }

#if gcdCAPTURE_ONLY_MODE
    contiguousBaseCap = platform->params.contiguousBases[0];
    contiguousSizeCap = platform->params.contiguousSizes[0];

    gcmkPRINT("Capture only mode is enabled in Hal Kernel.");

    if ((contiguousBaseCap + contiguousSizeCap) > 0x80000000)
        gcmkPRINT("Capture only mode: contiguousBase + contiguousSize > 2G, there is error in CModel and old MMU version RTL simulation.");

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        for (j = 0; j < gcvSRAM_INTER_COUNT; j++) {
            sRAMBaseCap[i][j] = platform->params.sRAMBases[i][j];
            sRAMSizeCap[i][j] = platform->params.sRAMSizes[i][j];
        }
    }

    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        extSRAMBaseCap[i] = platform->params.extSRAMBases[i];
        extSRAMSizeCap[i] = platform->params.extSRAMSizes[i];
    }
#endif

    if (platform->ops->adjustParam)
        /* Override default module param. */
        platform->ops->adjustParam(platform, &platform->params);

#if gcdCAPTURE_ONLY_MODE
    platform->params.contiguousBases[0] = contiguousBaseCap;
    platform->params.contiguousSizes[0] = contiguousSizeCap;

    for (i = 0; i < gcdGLOBAL_CORE_COUNT; i++) {
        for (j = 0; j < gcvSRAM_INTER_COUNT; j++) {
            platform->params.sRAMBases[i][j] = sRAMBaseCap[i][j];
            platform->params.sRAMSizes[i][j] = sRAMSizeCap[i][j];
        }
    }

    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        platform->params.extSRAMBases[i] = extSRAMBaseCap[i];
        platform->params.extSRAMSizes[i] = extSRAMSizeCap[i];
    }
#endif

    /* Update dma_mask if GPU only support 4G address */
    if (platform->flagBits & gcvPLATFORM_FLAG_LIMIT_4G_ADDRESS) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 24)
        dma_mask = DMA_BIT_MASK(32);
#else
        dma_mask = DMA_32BIT_MASK;
#endif
        pdev->dev.coherent_dma_mask = dma_mask;
        *pdev->dev.dma_mask = dma_mask;
    }

    /* Update module param because drv_init() uses them directly. */
    _SyncModuleParam(&platform->params);

    if (powerManagement == 0)
        gcmkPRINT("[ftg340 warning]: power saving is disabled.");

    if (debugLevel)
        gckOS_SetDebugLevelZone(gcvLEVEL_VERBOSE, gcdZONE_ALL);

    ret = drv_init();

    if (!ret) {
        platform_set_drvdata(pdev, galDevice);

#if gcdENABLE_DRM
        ret = viv_drm_probe(&pdev->dev);
#endif
    }

    if (ret < 0) {
        int i;
        if (platform->ops->putPower) {
            if (getPowerFlag == gcvTRUE)
                platform->ops->putPower(platform);
        }

        for (i = 0; i < gcdDEVICE_COUNT; i++) {
            if (platform->params.devices[i])
                platform->params.devices[i] = NULL;
        }

        gcmkFOOTER_ARG(KERN_INFO "Failed to register gpu driver: %d\n", ret);
    } else {
        gcmkFOOTER_ARG(KERN_INFO "Success ret=%d", ret);
    }

    return ret;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
static int viv_dev_remove(struct platform_device *pdev)
#else
static int __devexit viv_dev_remove(struct platform_device *pdev)
#endif
{
    gctUINT i;

    gcmkHEADER();

#if gcdENABLE_DRM
    viv_drm_remove(&pdev->dev);
#endif

    drv_exit();

    if (platform->ops->putPower)
        platform->ops->putPower(platform);

    if (platform->ops->dmaExit)
        platform->ops->dmaExit(platform);

    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (platform->params.devices[i])
            platform->params.devices[i] = NULL;
    }

    gcmkFOOTER_NO();
    return 0;
}

static void viv_dev_shutdown(struct platform_device *pdev)
{
    galDevice->gotoShutdown = gcvTRUE;

    viv_dev_remove(pdev);
}

static int viv_dev_suspend(struct platform_device *dev, pm_message_t state)
{
    gceSTATUS status;
    gckGALDEVICE gal_device;
    gckDEVICE device;
    gctUINT i, dev_index;
    gckKERNEL kernel = gcvNULL;
    gctSIZE_T reservedBytes = 0;
    gceCHIPPOWERSTATE statesStored;

    gal_device = platform_get_drvdata(dev);

    if (!gal_device)
        return -1;

    for (dev_index = 0; dev_index < gal_device->args.devCount; dev_index++) {
        device = gal_device->devices[dev_index];

        for (i = 0; i < gcvCORE_COUNT; i++) {
            if (device->kernels[i] != gcvNULL) {
		kernel = device->kernels[i];
		gal_device->platform->params.userClusterMasks[i] = kernel->hardware->options.userClusterMasks[kernel->hardware->core];

                /* Store states. */
                status = gckHARDWARE_QueryPowerState(device->kernels[i]->hardware, &device->statesStored[i]);
                if (gcmIS_ERROR(status))
                    return -1;

                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, gcvPOWER_OFF);

                if (gcmIS_ERROR(status))
                    return -1;
            }
        }

        /* Calculate reserved memory bytes. */
        if (galDevice->contiguousSizes[dev_index])
            reservedBytes += (galDevice->contiguousVidMems[dev_index]->bytes -
                                galDevice->contiguousVidMems[dev_index]->freeBytes);

        if (galDevice->externalSize[dev_index])
            reservedBytes += (galDevice->externalVidMem[dev_index]->bytes -
                                galDevice->externalVidMem[dev_index]->freeBytes);

        if (galDevice->exclusiveSize[dev_index])
            reservedBytes += (galDevice->exclusiveVidMem[dev_index]->bytes -
                                galDevice->exclusiveVidMem[dev_index]->freeBytes);
    }


#if gcdSUSPEND_RESUME_FROM_DISK
    status = gckVIDMEM_AllocateResumeMemory(gal_device->devices[0]->kernels[0], reservedBytes);

    if (gcmIS_ERROR(status))
        goto error;

    status = gckVIDMEM_CopyMemoryforResume(gal_device->devices[0]->kernels[0]);

    if (gcmIS_ERROR(status))
        goto error;
#endif

    return 0;

error:
#if gcdSUSPEND_RESUME_FROM_DISK
    for (dev_index = 0; dev_index < gal_device->args.devCount; dev_index++) {
        device = gal_device->devices[dev_index];

        for (i = 0; i < gcvCORE_COUNT; i++) {
            if (device->kernels[i] != gcvNULL) {

                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, gcvPOWER_ON);

                if (gcmIS_ERROR(status))
                    return -1;

                /* Convert global state to crossponding internal state. */
                switch (device->statesStored[i]) {
                case gcvPOWER_ON:
                    statesStored = gcvPOWER_ON_AUTO;
                    break;
                case gcvPOWER_IDLE:
                    statesStored = gcvPOWER_IDLE_BROADCAST;
                    break;
                case gcvPOWER_SUSPEND:
                    statesStored = gcvPOWER_SUSPEND_BROADCAST;
                    break;
                case gcvPOWER_OFF:
                    statesStored = gcvPOWER_OFF_BROADCAST;
                    break;
                default:
                    statesStored = device->statesStored[i];
                    break;
                }

                /* Restore states. */
                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, statesStored);

                if (gcmIS_ERROR(status))
                    return -1;
            	}
            }
	}
	return 0;
#endif
}

static int viv_dev_resume(struct platform_device *dev)
{
    gceSTATUS status;
    gckGALDEVICE gal_device;
    gckDEVICE device;
    gctUINT i, dev_index;
    gceCHIPPOWERSTATE statesStored;
    struct device *device_acpi = &(dev->dev);

    gal_device = platform_get_drvdata(dev);

    if (!gal_device)
        return -1;
#if gcdSUSPEND_RESUME_FROM_DISK
    status = gckVIDMEM_DestroyResumeMemory(gal_device->devices[0]->kernels[0]);

    if (gcmIS_ERROR(status))
	    return -1;
#endif
    for (dev_index = 0; dev_index < gal_device->args.devCount; dev_index++) {
        device = gal_device->devices[dev_index];

        for (i = 0; i < gcvCORE_COUNT; i++) {
            if (device->kernels[i] != gcvNULL) {

                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, gcvPOWER_ON);

                if (gcmIS_ERROR(status))
                    return -1;

                /* Convert global state to crossponding internal state. */
                switch (device->statesStored[i]) {
                case gcvPOWER_ON:
                    statesStored = gcvPOWER_ON_AUTO;
                    break;
                case gcvPOWER_IDLE:
                    statesStored = gcvPOWER_IDLE_BROADCAST;
                    break;
                case gcvPOWER_SUSPEND:
                    statesStored = gcvPOWER_SUSPEND_BROADCAST;
                    break;
                case gcvPOWER_OFF:
                    statesStored = gcvPOWER_OFF_BROADCAST;
                    break;
                default:
                    statesStored = device->statesStored[i];
                    break;
                }

                /* Restore states. */
                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, statesStored);

                if (gcmIS_ERROR(status))
                    return -1;
            }
        }
    }

#if gcdENABLE_PHYTIUM_DEVFREQ
#include <acpi/actypes.h>

    acpi_handle handle = ACPI_HANDLE(device_acpi);
    union acpi_object args[4];
    
    struct acpi_object_list arg_list = {
	    .pointer = args,
	    .count = ARRAY_SIZE(args),
    };
    acpi_status acpi_status;
    unsigned long long data;

    args[0].type = ACPI_TYPE_INTEGER;
    args[0].integer.value = FT_DEVICE_TYPE;
    args[1].type = ACPI_TYPE_INTEGER;
    args[1].integer.value = device->kernels[0]->tmp_freq;
    args[2].type = ACPI_TYPE_INTEGER;
    args[2].integer.value = 1;
    args[3].type = ACPI_TYPE_INTEGER;
    args[3].integer.value = 1;

    acpi_status = acpi_evaluate_integer(handle, "PSCF", &arg_list, &data);
    if (ACPI_FAILURE(acpi_status)) {
	    pr_err("PSCF Method for shader failed %s\n", __func__);
	    return -ENODEV;
    }
#endif

    return 0;
}

#if gcdSUSPEND_RESUME_FROM_DISK
static int viv_dev_restore(struct platform_device *dev)
{
    gceSTATUS status;
    gckGALDEVICE gal_device;
    gckDEVICE device;
    gctUINT i, dev_index;
    gceCHIPPOWERSTATE statesStored;

    gal_device = platform_get_drvdata(dev);

    if (!gal_device)
        return -1;

    status = gckVIDMEM_RestoreMemoryforResume(gal_device->devices[0]->kernels[0]);

    if (gcmIS_ERROR(status))
        return -1;
    /* Destroy memory for resume. */
    status = gckVIDMEM_DestroyResumeMemory(gal_device->devices[0]->kernels[0]);

    if (gcmIS_ERROR(status))
        return -1;

    gal_device->devices[0]->kernels[0]->db->resumeNodeObject = gcvNULL;

    for (dev_index = 0; dev_index < gal_device->args.devCount; dev_index++) {
        device = gal_device->devices[dev_index];

        for (i = 0; i < gcvCORE_COUNT; i++) {
            if (device->kernels[i] != gcvNULL) {

                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, gcvPOWER_ON);

                if (gcmIS_ERROR(status))
                    return -1;

                /* Convert global state to crossponding internal state. */
                switch (device->statesStored[i]) {
                case gcvPOWER_ON:
                    statesStored = gcvPOWER_ON_AUTO;
                    break;
                case gcvPOWER_IDLE:
                    statesStored = gcvPOWER_IDLE_BROADCAST;
                    break;
                case gcvPOWER_SUSPEND:
                    statesStored = gcvPOWER_SUSPEND_BROADCAST;
                    break;
                case gcvPOWER_OFF:
                     statesStored = gcvPOWER_OFF_BROADCAST;
                    break;
                default:
                    statesStored = device->statesStored[i];
                    break;
                }

                /* Restore states. */
                status = gckHARDWARE_SetPowerState(device->kernels[i]->hardware, statesStored);

                if (gcmIS_ERROR(status))
                    return -1;
            }
        }
    }

    return 0;
}
#endif

#if defined(CONFIG_PM) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
#ifdef CONFIG_PM_SLEEP
static int viv_dev_system_suspend(struct device *dev)
{
    pm_message_t state = { 0 };

    return viv_dev_suspend(to_platform_device(dev), state);
}

static int viv_dev_system_resume(struct device *dev)
{
    return viv_dev_resume(to_platform_device(dev));
}

#  if gcdSUSPEND_RESUME_FROM_DISK
static int viv_dev_system_restore(struct device *dev)
{
    return viv_dev_restore(to_platform_device(dev));
}
#  endif /* gcdSUSPEND_RESUME_FROM_DISK */
# endif /* CONFIG_PM_SLEEP */

static const struct dev_pm_ops viv_dev_pm_ops = {
#if gcdSUSPEND_RESUME_FROM_DISK
    SET_SYSTEM_SLEEP_RESTORE_PM_OPS(viv_dev_system_suspend, viv_dev_system_resume,
                                    viv_dev_system_restore)
#else
    SET_SYSTEM_SLEEP_PM_OPS(viv_dev_system_suspend, viv_dev_system_resume)
#endif
};
#endif

static struct platform_driver viv_dev_driver = {
    .probe = viv_dev_probe,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 8, 0)
    .remove = viv_dev_remove,
#else
    .remove = __devexit_p(viv_dev_remove),
#endif

    .suspend = viv_dev_suspend,
    .resume = viv_dev_resume,
    .shutdown = viv_dev_shutdown,

    .driver = {
        .owner = THIS_MODULE,
        .name = DEVICE_NAME,
#if defined(CONFIG_PM) && LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 30)
        .pm = &viv_dev_pm_ops,
#endif
    }
};

static int __init viv_dev_init(void)
{
    int ret = 0;

    ret = gckPLATFORM_Init(&viv_dev_driver, &platform);

    if (ret || !platform) {
        pr_err("ftg340: Soc platform init failed.\n");
        return -ENODEV;
    }

    ret = platform_driver_register(&viv_dev_driver);

    if (ret) {
        pr_err("ftg340: gpu_init() failed to register driver!\n");
        gckPLATFORM_Terminate(platform);
        platform = NULL;
        return -ENODEV;
    }

    platform->driver = &viv_dev_driver;
    return 0;
}

static void __exit viv_dev_exit(void)
{
    platform_driver_unregister(&viv_dev_driver);

    gckPLATFORM_Terminate(platform);
    platform = NULL;
}

module_init(viv_dev_init);

module_exit(viv_dev_exit);
