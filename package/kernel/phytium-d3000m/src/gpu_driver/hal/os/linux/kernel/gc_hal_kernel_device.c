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
#include "gc_hal_kernel.h"
#include "gc_feature_database.h"
#include "AQ.h"
#include <linux/pagemap.h>
#include <linux/seq_file.h>
#include <linux/mman.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/io.h>

#define _GC_OBJ_ZONE    gcvZONE_DEVICE

static gckGALDEVICE     galDevice;

#ifdef CONFIG_DEBUG_FS
#define ENABLE_VIV_DEBUG_FS 1
#endif

#if ENABLE_VIV_DEBUG_FS
#if defined(CONFIG_CPU_CSKYV2) && LINUX_VERSION_CODE <= KERNEL_VERSION(3, 0, 8)
static void
seq_vprintf(struct seq_file *m, const char *f, va_list args)
{
    int len;

    if (m->count < m->size) {
        len = vsnprintf(m->buf + m->count, m->size - m->count, f, args);
        if (m->count + len < m->size) {
            m->count += len;
            return;
        }
    }
    m->count = m->size;
}
# endif

static int
debugfs_printf(void *obj, const char *fmt, ...)
{
    va_list args;

    va_start(args, fmt);
    seq_vprintf((struct seq_file *)obj, fmt, args);
    va_end(args);

    return 0;
}
#else
static int
sys_printf(void *obj, const char *fmt, ...)
{
    int len = 0;
    va_list args;

    va_start(args, fmt);
    len = vsprintf((char *)obj, fmt, args);
    va_end(args);

    return len;
}
#endif

#ifdef ENABLE_VIV_DEBUG_FS
# define fs_printf debugfs_printf
#else
# define fs_printf sys_printf
#endif

/*******************************************************************************
 ******************************* Debugfs Support *******************************
 *******************************************************************************/

/*******************************************************************************
 **************************** DEBUG SHOW FUNCTIONS *****************************
 *******************************************************************************/

static int
gc_info_show(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device = gcvNULL;
    int i = 0;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif
    gceCHIPMODEL chipModel = 0;
    gctUINT32 chipRevision = 0;
    gctUINT32 productID = 0;
    gctUINT32 ecoID = 0;

    if (!gal_device)
        return -ENXIO;

    device = gal_device->devices[0];

    for (i = 0; i < gcdMAX_GPU_COUNT; i++) {
        if (device->kernels[i]) {

            chipModel = device->kernels[i]->hardware->identity.chipModel;
            chipRevision = device->kernels[i]->hardware->identity.chipRevision;
            productID = device->kernels[i]->hardware->identity.productID;
            ecoID = device->kernels[i]->hardware->identity.ecoID;

            len = fs_printf(ptr, "gpu      : %s\n", "FTG340");
#if 0
            len += fs_printf(ptr + len, "model    : %4x\n", chipModel);
            len += fs_printf(ptr + len, "revision : %4x\n", chipRevision);
            len += fs_printf(ptr + len, "product  : %4x\n", productID);
            len += fs_printf(ptr + len, "eco      : %4x\n", ecoID);
            len += fs_printf(ptr + len, "\n");
#endif
        }
    }
    return len;
}

static int
gc_clients_show(void *m, void *data)
{
    gckGALDEVICE device = galDevice;

    gckKERNEL kernel = _GetValidKernel(device);

    gcsDATABASE_PTR database;
    gceSTATUS status;
    gctINT i, pid;
    char name[24];
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!kernel)
        return -ENXIO;

    len = fs_printf(ptr, "%-8s%s\n", "PID", "NAME");
    len += fs_printf(ptr + len, "------------------------\n");

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i) {
        for (database = kernel->db->db[i];
             database != gcvNULL;
             database = database->next) {
            pid = database->processID;

            gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

            len += fs_printf(ptr + len, "%-8d%s\n", pid, name);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    /* Success. */
    return len;

OnError:
    if (status == gcvSTATUS_INVALID_ARGUMENT)
        return -EINVAL;
    return status;
}

static int
gc_meminfo_show(void *m, void *data)
{
    gckGALDEVICE device = galDevice;
    gckKERNEL kernel = _GetValidKernel(device);
    gckVIDMEM memory;
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gctUINT32 i;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    gctUINT64 free = 0, used = 0, total = 0, minFree = 0, maxUsed = 0;

    gcsDATABASE_COUNTERS virtualCounter  = { 0, 0, 0 };
    gcsDATABASE_COUNTERS nonPagedCounter = { 0, 0, 0 };

    if (!kernel)
        return -ENXIO;

    len = fs_printf(ptr, "VIDEO MEMORY:\n");

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++)
    {
        gcePOOL pool = gcvPOOL_SYSTEM;

        if (!kernel->device->contiguousSizes[i])
            break;

        kernel->device->memIndex = i;

        status = gckKERNEL_GetVideoMemoryPool(kernel, &pool, &memory);

        if (gcmIS_SUCCESS(status)) {
            gcmkONERROR(gckOS_AcquireMutex(memory->os, memory->mutex, gcvINFINITE));

            free = memory->freeBytes;
            minFree = memory->minFreeBytes;
            used = memory->bytes - memory->freeBytes;
            maxUsed = memory->bytes - memory->minFreeBytes;
            total = memory->bytes;

            gcmkVERIFY_OK(gckOS_ReleaseMutex(memory->os, memory->mutex));
        }

        pool = gcvPOOL_SYSTEM_32BIT_VA;
        status = gckKERNEL_GetVideoMemoryPool(kernel, &pool, &memory);

        if (!i && gcmIS_SUCCESS(status)) {
            gcmkONERROR(gckOS_AcquireMutex(memory->os, memory->mutex, gcvINFINITE));

            free += memory->freeBytes;
            minFree += memory->minFreeBytes;
            used += memory->bytes - memory->freeBytes;
            maxUsed += memory->bytes - memory->minFreeBytes;
            total += memory->bytes;

            gcmkVERIFY_OK(gckOS_ReleaseMutex(memory->os, memory->mutex));
        }

        len += fs_printf(ptr + len, "  POOL SYSTEM%d:\n", i);
        len += fs_printf(ptr + len, "    Free :    %13llu B\n", free);
        len += fs_printf(ptr + len, "    Used :    %13llu B\n", used);
        len += fs_printf(ptr + len, "    MinFree : %13llu B\n", minFree);
        len += fs_printf(ptr + len, "    MaxUsed : %13llu B\n", maxUsed);
        len += fs_printf(ptr + len, "    Total :   %13llu B\n", total);
    }

    kernel->device->memIndex = 0;

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    /* Walk the databases. */
    for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i) {
        for (database = kernel->db->db[i];
             database != gcvNULL;
             database = database->next) {
            gcsDATABASE_COUNTERS *counter;

            counter = &database->vidMemPool[gcvPOOL_VIRTUAL];
            virtualCounter.bytes += counter->bytes;
            virtualCounter.maxBytes += counter->maxBytes;

            counter = &database->nonPaged;
            nonPagedCounter.bytes += counter->bytes;
            nonPagedCounter.bytes += counter->maxBytes;
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    len += fs_printf(ptr + len, "  POOL VIRTUAL:\n");
    len += fs_printf(ptr + len, "    Used :    %10llu B\n", virtualCounter.bytes);
    len += fs_printf(ptr + len, "    MaxUsed : %10llu B\n", virtualCounter.bytes);

    return len;

OnError:
    if (status == gcvSTATUS_INVALID_ARGUMENT)
        return -EINVAL;
    return status;
}

gceSTATUS
gc_mmuinfo_show(void)
{
#if gcdMMU_VERSION_1
    gckGALDEVICE device = galDevice;
    gckKERNEL kernel = _GetValidKernel(device);
    gckMMU mmu = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 mStart = 0;
    gctUINT32 mEnd = gcdMMU_MTLB_ENTRY_NUM;
    gctUINT32 sStart = 0;
    gctUINT32 sEnd = 0;

    gctUINT32 mtlbEntry = 0;
    gctUINT32 stlbEntry = 0;

    gctUINT32 stlbSize = 0;
    gctUINT32 stlbEntryNum = 0;

    gctUINT32_PTR stlbLogical = gcvNULL;
    gctPHYS_ADDR_T stlbPhysical = 0;
    gctUINT32 extAddr = 0;

    gctUINT32 i;

    if (!kernel)
        gcmkONERROR(gcvSTATUS_INVALID_OBJECT);

    mmu = kernel->mmu;
    if (!mmu)
        gcmkONERROR(gcvSTATUS_INVALID_OBJECT);

    gcmkPRINT("MMU PAGE TABLE:\n");

    gcmkPRINT("  BASE ADDRESS:\n");
    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++)
        gcmkPRINT("    contiguousBaseAddresses[%d]: 0x%010X\n", i, mmu->contiguousBaseAddresses[i]);

    gcmkPRINT("    externalBaseAddress:        0x%010X\n", mmu->externalBaseAddress);
    gcmkPRINT("    internalBaseAddress:        0x%010X\n", mmu->internalBaseAddress);
    gcmkPRINT("    exclusiveBaseAddress:       0x%010X\n", mmu->exclusiveBaseAddress);

    /* get extAddress */
    extAddr = (gctUINT32)(mmu->mtlbPhysical >> 32);
    /* more than 40bit physical address */
    if (extAddr & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    gcmkPRINT("  Extensional 8 bit for 40bit address: 0x%02X\n", extAddr);

    gcmkPRINT("  MTLB ENTRY:\n");

    /* first mtlb entry is reserved for SRAM */
    gcmkPRINT("    MTLB entry[0] is reserved for SRAM.\n");
    ++mStart;

    while (mStart < mEnd) {
        mtlbEntry = *(mmu->mtlbLogical + mStart);
        if (mtlbEntry) {
            gctUINT64 dataMtlb = mtlbEntry & ~0X1F;
            gctUINT8 maskMtlb = mtlbEntry & 0X1F;

            gcmkASSERT(dataMtlb);

            /* stlb physical address */
            stlbPhysical = ((gctUINT64)extAddr << 32) | (gctUINT64)dataMtlb;

            /* get stlb entry info */
            switch ((mtlbEntry & 0X0C) >> 2) {
            case 0:
                sEnd = gcdMMU_STLB_4K_ENTRY_NUM;
                stlbSize = gcdMMU_STLB_4K_SIZE;
                gcmkPRINT("    MTLB entry[%d]:   STLB physical = 0x%010lX, PAGE_SIZE = 4K, MASK = 0x%02X\n", mStart, stlbPhysical, maskMtlb);
                break;
            case 1:
                sEnd = gcdMMU_STLB_64K_ENTRY_NUM;
                stlbSize = gcdMMU_STLB_64K_SIZE;
                gcmkPRINT("    MTLB entry[%d]:   STLB physical = 0x%010lX, PAGE_SIZE = 64K, MASK = 0x%02X\n", mStart, stlbPhysical, maskMtlb);
                break;
            case 2:
                sEnd = gcdMMU_STLB_1M_ENTRY_NUM;
                stlbSize = gcdMMU_STLB_1M_SIZE;
                gcmkPRINT("    MTLB entry[%d]:   STLB physical = 0x%010lX, PAGE_SIZE = 1M, MASK = 0x%02X\n", mStart, stlbPhysical, maskMtlb);
                break;
            case 3:
                sEnd = gcdMMU_STLB_16M_ENTRY_NUM;
                stlbSize = gcdMMU_STLB_16M_SIZE;
                gcmkPRINT("    MTLB entry[%d]:   STLB physical = 0x%010lX, PAGE_SIZE = 16M, MASK = 0x%02X\n", mStart, stlbPhysical, maskMtlb);
                break;
            default:
                pr_warn("\nERROR: unknown PAGE SIZE...\n");
                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
                break;
            }

            /* Convert GPU physical to CPU physical. */
            gckOS_GPUPhysicalToCPUPhysical(kernel->os, stlbPhysical, &stlbPhysical);

            gcmkONERROR(gckOS_MapPhysical(kernel->os, stlbPhysical, stlbSize, (gctPOINTER *)&stlbLogical));

            /* print stlb */
            gcmkPRINT("      STLB entry : \n");
            stlbEntryNum = stlbSize >> 2;
            for (sStart = 0; sStart < stlbEntryNum; ++sStart) {
                stlbEntry = _ReadPageEntry(stlbLogical + sStart);
                if (stlbEntry && stlbEntry & ~0XF) {
                    gctUINT64 pagePhysical = ((gctUINT64)stlbEntry & 0XFF0) << 28 | ((gctUINT64)stlbEntry & ~0XFFF);
                    gctUINT32 maskStlb = stlbEntry & 0XF;

                    gcmkPRINT("        stlb entry[%04d]:      Page Physical = 0x%010lX  mask = 0x%01X\n", sStart, pagePhysical, maskStlb);
                }
            }

            gcmkONERROR(gckOS_UnmapPhysical(kernel->os, stlbLogical, stlbSize));
        } else {
            gcmkPRINT("      STLB is empty\n");
        }
        mStart++;
    }

    return status;
OnError:
    gcmkPRINT("ERROR...\n");
    return status;
#else
    return gcvSTATUS_OK;
#endif
}

static const char *vidmemTypeStr[gcvVIDMEM_TYPE_COUNT] = {
    "Generic",
    "Index",
    "Vertex",
    "Texture",
    "RenderTarget",
    "Depth",
    "Bitmap",
    "TileStatus",
    "Image",
    "Mask",
    "Scissor",
    "HZ",
    "ICache",
    "TxDesc",
    "Fence",
    "TFBHeader",
    "Command",
};

static const char *poolStr[gcvPOOL_NUMBER_OF_POOLS] = {
    "Unknown",
    "Default",
    "Local",
    "Internal",
    "External",
    "Unified",
    "System",
    "Virtual",
    "User",
    "Insram",
    "Exsram",
    "Exclusive",
    "System_32bit_va",
};

static int
_ShowCounters(void *File, gcsDATABASE_PTR Database)
{
    gctUINT i = 0;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = File;
#else
    char *ptr = (char *)File;
#endif

    static const char * const otherCounterNames[] = {
        "AllocNonPaged",
        "AllocContiguous",
        "MapMemory",
    };

    gcsDATABASE_COUNTERS *otherCounters[] = {
        &Database->nonPaged,
        &Database->contiguous,
        &Database->mapMemory,
    };

    len = fs_printf(ptr, "%-16s %16s %16s %16s\n", "", "Current", "Maximum", "Total");

    /* Print surface type counters. */
    len += fs_printf(ptr + len,
                     "%-16s %16lld %16lld %16lld\n",
                     "All-Types",
                     Database->vidMem.bytes,
                     Database->vidMem.maxBytes,
                     Database->vidMem.totalBytes);

    for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++) {
        len += fs_printf(ptr + len,
                         "%-16s %16lld %16lld %16lld\n",
                         vidmemTypeStr[i],
                         Database->vidMemType[i].bytes,
                         Database->vidMemType[i].maxBytes,
                         Database->vidMemType[i].totalBytes);
    }
    /*seq_puts(File, "\n");*/
    len += fs_printf(ptr + len, "\n");

    /* Print surface pool counters. */
    len += fs_printf(ptr + len,
                     "%-16s %16lld %16lld %16lld\n",
                     "All-Pools",
                     Database->vidMem.bytes,
                     Database->vidMem.maxBytes,
                     Database->vidMem.totalBytes);

    for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++) {
        len += fs_printf(ptr + len,
                         "%-16s %16lld %16lld %16lld\n",
                         poolStr[i],
                         Database->vidMemPool[i].bytes,
                         Database->vidMemPool[i].maxBytes,
                         Database->vidMemPool[i].totalBytes);
    }
    /*seq_puts(File, "\n");*/
    len += fs_printf(ptr + len, "\n");

    /* Print other counters. */
    for (i = 0; i < gcmCOUNTOF(otherCounterNames); i++) {
        len += fs_printf(ptr + len,
                         "%-16s %16lld %16lld %16lld\n",
                         otherCounterNames[i],
                         otherCounters[i]->bytes,
                         otherCounters[i]->maxBytes,
                         otherCounters[i]->totalBytes);
    }
    /*seq_puts(File, "\n");*/
    len += fs_printf(ptr + len, "\n");
    return len;
}

static int
_ShowRecord(void *File, gcsDATABASE_PTR Database, gcsDATABASE_RECORD_PTR Record)
{
    gctUINT32 handle;
    gckVIDMEM_NODE nodeObject;
    gctPHYS_ADDR_T physical;
    gceSTATUS status = gcvSTATUS_OK;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = File;
#else
    char *ptr = (char *)File;
#endif

    static const char *recordTypes[gcvDB_NUM_TYPES] = {
        "Unknown",
        "VideoMemory",
        "CommandBuffer",
        "NonPaged",
        "Contiguous",
        "Signal",
        "VidMemLock",
        "Context",
        "Idel",
        "MapMemory",
        "ShBuf",
    };

    handle = gcmPTR2INT32(Record->data);

    if (Record->type == gcvDB_VIDEO_MEMORY ||
        Record->type == gcvDB_VIDEO_MEMORY_LOCKED) {
        status = gckVIDMEM_HANDLE_Lookup2(Record->kernel, Database, handle, &nodeObject);

        if (gcmIS_ERROR(status)) {
            len += fs_printf(ptr + len, "%6u Invalid Node\n", handle);
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
        gcmkONERROR(gckVIDMEM_NODE_GetCPUPhysical(Record->kernel, nodeObject, 0, &physical));
    } else {
        physical = (gctUINT64)(gctUINTPTR_T)Record->physical;
    }

    len += fs_printf(ptr + len,
                     "%-14s %3d %16x %16zx %16zu\n",
                     recordTypes[Record->type],
                     Record->kernel->core,
                     gcmPTR2INT32(Record->data),
                     (size_t)physical,
                     Record->bytes);

OnError:
    return len;
}

static int
_ShowDataBaseOldFormat(void *File, gcsDATABASE_PTR Database)
{
    gctINT  pid;
    gctUINT i;
    char name[24];
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = File;
#else
    char *ptr = (char *)File;
#endif

    /* Process ID and name */
    pid = Database->processID;
    gcmkVERIFY_OK(gckOS_GetProcessNameByPid(pid, gcmSIZEOF(name), name));

    len = fs_printf(ptr, "--------------------------------------------------------------------------------\n");
    len += fs_printf(ptr + len, "Process: %-8d %s\n", pid, name);

    len += fs_printf(ptr + len, "Records:\n");

    len += fs_printf(ptr + len,
                     "%14s %3s %16s %16s %16s\n",
                     "Type", "GPU", "Data/Node", "Physical/Node", "Bytes");

    for (i = 0; i < gcmCOUNTOF(Database->list); i++) {
        gcsDATABASE_RECORD_PTR record = Database->list[i];

        while (record) {
            len += _ShowRecord(ptr + len, Database, record);
            record = record->next;
        }
    }

    len += fs_printf(ptr + len, "Counters:\n");

    len += _ShowCounters(ptr + len, Database);
    return len;
}

static int
gc_db_old_show(void *m, void *data, gctBOOL all)
{
    gcsDATABASE_PTR  database;
    gctINT i;

    static gctUINT64 idleTime;
    gceSTATUS status;
    gckGALDEVICE device = galDevice;
    gckKERNEL kernel = _GetValidKernel(device);
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!kernel)
        return -ENXIO;

    /* Acquire the database mutex. */
    gcmkONERROR(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

    if (kernel->db->idleTime) {
        /* Record idle time if DB upated. */
        idleTime = kernel->db->idleTime;
        kernel->db->idleTime = 0;
    }

    /* Idle time since last call */
    len = fs_printf(ptr, "GPU Idle: %llu ns\n", idleTime);

    if (all) {
        /* Walk the databases. */
        for (i = 0; i < gcmCOUNTOF(kernel->db->db); ++i) {
            for (database = kernel->db->db[i];
                 database != gcvNULL;
                 database = database->next)
                len += _ShowDataBaseOldFormat(ptr + len, database);
        }
    }

    /* Release the database mutex. */
    gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));

    return len;

OnError:
    if (status == gcvSTATUS_INVALID_ARGUMENT)
        return -EINVAL;
    return status;
}

static int
gc_db_show(void *m, void *data, gctBOOL all)
{
    return 0;
}

static int
gc_version_show(void *m, void *data)
{
    gckGALDEVICE device = galDevice;
    gcsPLATFORM *platform = gcvNULL;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!device)
        return -ENXIO;

    platform = device->platform;
    if (!platform)
        return -ENXIO;

#ifdef ENABLE_VIV_DEBUG_FS
    len = fs_printf(ptr, "%s built at %s\n", gcvVERSION_STRING, HOST);

    if (platform->name)
        len += fs_printf(ptr + len, "Platform path: %s\n", platform->name);
    else
        len += fs_printf(ptr + len, "Code path: %s\n", __FILE__);
#else
    len = fs_printf(ptr, "%s\n", gcvVERSION_STRING);
#endif

    return len;
}

static void
print_ull(char dest[32], unsigned long long u)
{
    unsigned int t[7];
    int i;

    if (u < 1000) {
        sprintf(dest, "%27llu", u);
        return;
    }

    for (i = 0; i < 7 && u; i++)
        t[i] = do_div(u, 1000);

    dest += sprintf(dest, "%*s", (7 - i) * 4, "");
    dest += sprintf(dest, "%3u", t[--i]);

    for (i--; i >= 0; i--)
        dest += sprintf(dest, ",%03u", t[i]);
}

/*******************************************************************************
 **
 ** Show PM state timer.
 **
 ** Entry is called as 'idle' for compatible reason, it shows more information
 ** than idle actually.
 **
 **  Start: Start time of this counting period.
 **  End: End time of this counting period.
 **  On: Time GPU stays in gcvPOWER_0N.
 **  Off: Time GPU stays in gcvPOWER_0FF.
 **  Idle: Time GPU stays in gcvPOWER_IDLE.
 **  Suspend: Time GPU stays in gcvPOWER_SUSPEND.
 */
static int
gc_idle_show(void *m, void *data)
{
    gckGALDEVICE device = galDevice;
    gckKERNEL kernel = _GetValidKernel(device);
    char str[32];

    gctUINT64 on;
    gctUINT64 off;
    gctUINT64 idle;
    gctUINT64 suspend;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!kernel)
        return -ENXIO;

    gckHARDWARE_QueryStateTimer(kernel->hardware, &on, &off, &idle, &suspend);

    /* Idle time since last call */
    print_ull(str, on);
    len = fs_printf(ptr, "On:      %s ns\n", str);
    print_ull(str, off);
    len += fs_printf(ptr + len, "Off:     %s ns\n", str);
    print_ull(str, idle);
    len += fs_printf(ptr + len, "Idle:    %s ns\n", str);
    print_ull(str, suspend);
    len += fs_printf(ptr + len, "Suspend: %s ns\n", str);

    return len;
}

#define LOAD_FSHIFT 100
static gctUINT dumpDevice = 0;
static gctUINT dumpCore   = 0;
static gctUINT delayMs    = 100;

static int
gc_load_show(void *m, void *data)
{
    int len = 0;
    gctUINT32 i, j;
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device = gcvNULL;
    gceCHIPPOWERSTATE statesStored = gcvPOWER_INVALID;
    gceCHIPPOWERSTATE state = gcvPOWER_INVALID;
    gctUINT32 load_fixed[gcdDEVICE_COUNT][gcvCORE_3D_MAX + 1] = { {0}, {0} };
    gctUINT32 hi_total_cycle_count[gcdDEVICE_COUNT][gcvCORE_3D_MAX + 1] = { {0}, {0} };
    gctUINT32 hi_total_idle_cycle_count[gcdDEVICE_COUNT][gcvCORE_3D_MAX + 1] = { {0}, {0} };

    gctBOOL powerManagement[gcdDEVICE_COUNT][gcvCORE_3D_MAX + 1] = { {gcvFALSE}, {gcvFALSE} };

#ifdef CONFIG_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!gal_device)
        return -ENXIO;

    for (j = 0; j < gcdDEVICE_COUNT; j++) {
        if (gal_device->devices[j])
            device = gal_device->devices[j];
        else
            continue;

        for (i = 0; i <= gcvCORE_3D_MAX; i++) {
            if (device->kernels[i]) {
                if (device->kernels[i]->hardware) {
                    gckHARDWARE Hardware = device->kernels[i]->hardware;

                    powerManagement[j][i] = Hardware->options.powerManagement;

                    if (powerManagement[j][i])
                        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

                    gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

                    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

                    gcmkONERROR(gckHARDWARE_SetGpuProfiler(Hardware, gcvTRUE));

                    Hardware->waitCount = 200 * 100;
                }
            }
        }
    }

    for (j = 0; j < gcdDEVICE_COUNT; j++) {
        if (gal_device->devices[j])
            device = gal_device->devices[j];
        else
            continue;

        for (i = 0; i <= gcvCORE_3D_MAX; i++) {
            if (device->kernels[i]) {
                if (device->kernels[i]->hardware)
                    gcmkONERROR(gckHARDWARE_CleanCycleCount(device->kernels[i]->hardware));
            }
        }
    }

    gckOS_Delay(gcvNULL, delayMs);

    for (j = 0; j < gcdDEVICE_COUNT; j++) {
        if (gal_device->devices[j])
            device = gal_device->devices[j];
        else
            continue;

        for (i = 0; i <= gcvCORE_3D_MAX; i++) {
            if (device->kernels[i]) {
                if (device->kernels[i]->hardware)
                    gcmkONERROR(gckHARDWARE_QueryCycleCount(device->kernels[i]->hardware,
                                                            &hi_total_cycle_count[j][i],
                                                            &hi_total_idle_cycle_count[j][i]));
            }
        }
    }

    len += fs_printf(ptr + len, "The previous %ums HW load\n", delayMs);

    for (j = 0; j < gcdDEVICE_COUNT; j++) {
        if (gal_device->devices[j]) {
            len += fs_printf(ptr + len, "device        : %d\n", j);
            device = gal_device->devices[j];
        } else
            continue;

        for (i = 0; i <= gcvCORE_3D_MAX; i++) {
            if (device->kernels[i]) {
                if (device->kernels[i]->hardware) {
                    gckHARDWARE Hardware = device->kernels[i]->hardware;

                    switch (statesStored) {
                    case gcvPOWER_OFF:
                        state = gcvPOWER_OFF_BROADCAST;
                        break;
                    case gcvPOWER_IDLE:
                        state = gcvPOWER_IDLE_BROADCAST;
                        break;
                    case gcvPOWER_SUSPEND:
                        state = gcvPOWER_SUSPEND_BROADCAST;
                        break;
                    case gcvPOWER_ON:
                        state = gcvPOWER_ON_AUTO;
                        break;
                    default:
                        state = statesStored;
                        break;
                    }

                    Hardware->waitCount = 200;

                    gcmkONERROR(gckHARDWARE_SetGpuProfiler(Hardware, gcvFALSE));

                    if (powerManagement[j][i])
                        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvTRUE));

                    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, state));

                    if (hi_total_cycle_count[j][i] == 0) {
                        len += fs_printf(ptr, "The current HW doesn't support use AHB register to read cycle counter.\n");
                        goto OnError;
                    } else {
                        load_fixed[j][i] = div_u64((gctUINT64)(hi_total_cycle_count[j][i] - hi_total_idle_cycle_count[j][i]) * 100 * LOAD_FSHIFT,
                                          hi_total_cycle_count[j][i]);

                    }

                    len += fs_printf(ptr + len, "    core      : %d\n", i);
                    len += fs_printf(ptr + len, "    load      : %d.%d%%\n", load_fixed[j][i] / LOAD_FSHIFT, load_fixed[j][i] % LOAD_FSHIFT);
                    len += fs_printf(ptr + len, "\n");
                }
            }
        }
    }

OnError:
    return len;
}

static int
gc_dump_trigger_show(void *m, void *data)
{
    int len = 0;

#ifdef CONFIG_DEBUG_FS
    void *ptr = m;
# else
    char *ptr = (char *)m;
# endif

    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device = gcvNULL;
    gckKERNEL kernel = gcvNULL;
    gckHARDWARE Hardware = gcvNULL;
    gctBOOL powerManagement = gcvFALSE;
    gceSTATUS status = gcvSTATUS_OK;
    gceCHIPPOWERSTATE statesStored, state;

    gctINT i = 0;

    len += fs_printf(ptr + len, "Dump one device: For example, dump cores on device 0: echo 0 > /sys/kernel/debug/gc/dump_trigger; cat /sys/kernel/debug/gc/dump_trigger\n");
    len += fs_printf(ptr + len, "The dump will be in [dmesg].\n");

    device = gal_device->devices[dumpDevice];

    for (i = 0; i < gcvCORE_COUNT; ++i) {
        if (!device->kernels[i])
            continue;

        kernel = device->kernels[i];
        Hardware = kernel->hardware;
        powerManagement = Hardware->options.powerManagement;

        if (powerManagement)
            gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

        gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

        gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

        gckKERNEL_DumpState(kernel);

        switch (statesStored) {
        case gcvPOWER_OFF:
            state = gcvPOWER_OFF_BROADCAST;
            break;
        case gcvPOWER_IDLE:
            state = gcvPOWER_IDLE_BROADCAST;
            break;
        case gcvPOWER_SUSPEND:
            state = gcvPOWER_SUSPEND_BROADCAST;
            break;
        case gcvPOWER_ON:
            state = gcvPOWER_ON_AUTO;
            break;
        default:
            state = statesStored;
            break;
        }

        if (powerManagement)
            gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvTRUE));

        gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, state));
    }

OnError:
    return len;
}

static int dumpProcess;

#ifdef CONFIG_DEBUG_FS
static int
_ShowVideoMemoryOldFormat(void *File, gcsDATABASE_PTR Database, gctBOOL All)
{
    gctUINT i = 0;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = File;
#else
    char *ptr = (char *)File;
#endif

    static const char * const otherCounterNames[] = {
        "AllocNonPaged",
        "AllocContiguous",
        "MapMemory",
    };

    gcsDATABASE_COUNTERS *otherCounters[] = {
        &Database->nonPaged,
        &Database->contiguous,
        &Database->mapMemory,
    };

    len = fs_printf(ptr, "%-16s %16s %16s %16s\n", "", "Current", "Maximum", "Total");

    /* Print surface type counters. */
    len += fs_printf(ptr + len,
                     "%-16s %16llu %16llu %16llu\n",
                     "All-Types",
                     Database->vidMem.bytes,
                     Database->vidMem.maxBytes,
                     Database->vidMem.totalBytes);

    if (All) {
        for (i = 1; i < gcvVIDMEM_TYPE_COUNT; i++) {
            len += fs_printf(ptr + len,
                             "%-16s %16llu %16llu %16llu\n",
                             vidmemTypeStr[i],
                             Database->vidMemType[i].bytes,
                             Database->vidMemType[i].maxBytes,
                             Database->vidMemType[i].totalBytes);
        }
        /*seq_puts(File, "\n");*/
        len += fs_printf(ptr + len, "\n");

        /* Print surface pool counters. */
        len += fs_printf(ptr + len,
                         "%-16s %16llu %16llu %16llu\n",
                         "All-Pools",
                         Database->vidMem.bytes,
                         Database->vidMem.maxBytes,
                         Database->vidMem.totalBytes);

        for (i = 1; i < gcvPOOL_NUMBER_OF_POOLS; i++) {
            len += fs_printf(ptr + len,
                             "%-16s %16llu %16llu %16llu\n",
                             poolStr[i],
                             Database->vidMemPool[i].bytes,
                             Database->vidMemPool[i].maxBytes,
                             Database->vidMemPool[i].totalBytes);
        }
        /*seq_puts(File, "\n");*/
        len += fs_printf(ptr + len, "\n");

        /* Print other counters. */
        for (i = 0; i < gcmCOUNTOF(otherCounterNames); i++) {
            len += fs_printf(ptr + len,
                             "%-16s %16llu %16llu %16llu\n",
                             otherCounterNames[i],
                             otherCounters[i]->bytes,
                             otherCounters[i]->maxBytes,
                             otherCounters[i]->totalBytes);
        }
        /*seq_puts(File, "\n");*/
        len += fs_printf(ptr + len, "\n");
    }

    return len;
}

static int
gc_vidmem_old_show(void *m, void *unused, gctBOOL all)
{
    gceSTATUS status;
    gcsDATABASE_PTR database;
    gckGALDEVICE gal_device = galDevice;
    gckKERNEL kernel = gcvNULL;
    char name[64];
    int i;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    kernel = _GetValidKernel(gal_device);

    if (!kernel)
        return -ENXIO;

    if (dumpProcess == 0) {
        /* Acquire the database mutex. */
        gcmkONERROR(gckOS_AcquireMutex(kernel->os, kernel->db->dbMutex, gcvINFINITE));

        for (i = 0; i < gcmCOUNTOF(kernel->db->db); i++) {
            for (database = kernel->db->db[i];
                 database != gcvNULL;
                 database = database->next) {
                gckOS_GetProcessNameByPid(database->processID, gcmSIZEOF(name), name);
                len += fs_printf(ptr + len,
                                 "VidMem Usage (Process %u: %s):\n",
                                 database->processID, name);
                len += _ShowVideoMemoryOldFormat(ptr + len, database, all);
                /*seq_puts(m, "\n");*/
                len += fs_printf(ptr + len, "\n");
            }
        }

        /* Release the database mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(kernel->os, kernel->db->dbMutex));
    } else {
        /* Find the database. */
        status = gckKERNEL_FindDatabase(kernel, dumpProcess, gcvFALSE, &database);

        if (gcmIS_ERROR(status)) {
            len += fs_printf(ptr + len, "ERROR: process %d not found\n", dumpProcess);
            return len;
        }

        gckOS_GetProcessNameByPid(dumpProcess, gcmSIZEOF(name), name);
        len += fs_printf(ptr + len, "VidMem Usage (Process %d: %s):\n", dumpProcess, name);
        len += _ShowVideoMemoryOldFormat(ptr + len, database, all);
    }

    return len;

OnError:
    if (status == gcvSTATUS_INVALID_ARGUMENT)
        return -EINVAL;
    return status;
}
#endif

static int
gc_vidmem_show(void *m, void *unused, gctBOOL all)
{
    return 0;
}

static int
gc_reserved_mem_usage_show(void *m)
{
    int len = 0;
    gckGALDEVICE gal_device = galDevice;
    gckVIDMEM vidMem;
    int usage;
    int i = 0;
    size_t t_freeBytes = 0;
    size_t t_bytes = 0;
    int t_usage = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    len = fs_printf(ptr, "%-25s %16s %16s %16s\n", "", "FreeBytes", "TotalBytes", "Usage(%)");


    /* Internal POOL */
    if (gal_device->internalVidMem) {
        vidMem = gal_device->internalVidMem;
        usage = (vidMem->bytes - vidMem->freeBytes) * 100 / vidMem->bytes;
        len += fs_printf(ptr + len, "%-25s", "Local Internel Pool:");
        len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", "", vidMem->freeBytes, vidMem->bytes, usage);
    }

    /* LOCAL EXTERNAL POOL */
    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (gal_device->externalVidMem[i]) {
            vidMem = gal_device->externalVidMem[i];
            t_freeBytes += vidMem->freeBytes;
            t_bytes += vidMem->bytes;
        } else {
            break;
        }
    }

    if (t_bytes > 0) {
        t_usage = (t_bytes - t_freeBytes) * 100 / t_bytes;
        len += fs_printf(ptr + len, "%-25s", "Local External Pool:");
        len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", t_freeBytes, t_bytes, t_usage);
    }

    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (gal_device->externalVidMem[i]) {
            vidMem = gal_device->externalVidMem[i];
            usage = (vidMem->bytes - vidMem->freeBytes) * 100 / vidMem->bytes;
            len += fs_printf(ptr + len, "%4s%s %d %s:%9s", "", "Card", i, "Pool", "");
            len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", vidMem->freeBytes, vidMem->bytes, usage);
        } else {
            break;
        }
    }

    t_freeBytes = 0;
    t_bytes = 0;
    t_usage = 0;

    /* LOCAL EXCLUSIVE POOL */
    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (gal_device->exclusiveVidMem[i]) {
            vidMem = gal_device->exclusiveVidMem[i];
            t_freeBytes += vidMem->freeBytes;
            t_bytes += vidMem->bytes;
        } else {
            break;
        }
    }

    if (t_bytes > 0) {
        t_usage = (t_bytes - t_freeBytes) * 100 / t_bytes;
        len += fs_printf(ptr + len, "%-25s", "Local Exclusive Pool:");
        len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", t_freeBytes, t_bytes, t_usage);
    }

    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        if (gal_device->exclusiveVidMem[i]) {
            vidMem = gal_device->exclusiveVidMem[i];
            usage = (vidMem->bytes - vidMem->freeBytes) * 100 / vidMem->bytes;
            len += fs_printf(ptr + len, "%4s%s %d %s:%9s", "", "Card", i, "Pool", "");
            len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", vidMem->freeBytes, vidMem->bytes, usage);
        } else {
            break;
        }
    }

    t_freeBytes = 0;
    t_bytes = 0;
    t_usage = 0;

    /* RESERVED POOL */
    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (gal_device->contiguousVidMems[i]) {
            vidMem = gal_device->contiguousVidMems[i];
            t_freeBytes += vidMem->freeBytes;
            t_bytes += vidMem->bytes;
        } else {
            break;
        }
    }

    if (t_bytes > 0) {
        t_usage = (t_bytes - t_freeBytes) * 100 / t_bytes;
        len += fs_printf(ptr + len, "%-25s", "System Reserved Pool:");
        len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", t_freeBytes, t_bytes, t_usage);
    }

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (gal_device->contiguousVidMems[i]) {
            vidMem = gal_device->contiguousVidMems[i];
            usage = (vidMem->bytes - vidMem->freeBytes) * 100 / vidMem->bytes;
            len += fs_printf(ptr + len, "%4s%s %d:%14s", "", "Pool", i, "");
            len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", vidMem->freeBytes, vidMem->bytes, usage);
        } else {
            break;
        }
    }

    t_freeBytes = 0;
    t_bytes = 0;
    t_usage = 0;

    /* SRAM POOL */
    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        if (gal_device->extSRAMVidMems[i]) {
            vidMem = gal_device->extSRAMVidMems[i];
            t_freeBytes += vidMem->freeBytes;
            t_bytes += vidMem->bytes;
        } else {
            break;
        }
    }

    if (t_bytes > 0) {
        t_usage = (t_bytes - t_freeBytes) * 100 / t_bytes;
        len += fs_printf(ptr + len, "%-25s", "SRAM Pool:");
        len += fs_printf(ptr + len, "%16lld %16lld %16d%%\n", t_freeBytes, t_bytes, t_usage);
    }

    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        if (gal_device->extSRAMVidMems[i]) {
            vidMem = gal_device->extSRAMVidMems[i];
            usage = (vidMem->bytes - vidMem->freeBytes) * 100 / vidMem->bytes;
            len += fs_printf(ptr + len, "SRAM Pool %d: ", i);
            len += fs_printf(ptr + len, "%-16s %16lld %16lld %16d%%\n", "", vidMem->freeBytes, vidMem->bytes, usage);
        } else {
            break;
        }
        len += fs_printf(ptr + len, "\n");
    }
    return 0;
}

#ifdef ENABLE_VIV_DEBUG_FS
static inline int
strtoint_from_user(const char __user *s, size_t count, int *res)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3, 0, 0)
    int ret = kstrtoint_from_user(s, count, 10, res);

    return ret < 0 ? ret : count;
# else
    /* sign, base 2 representation, newline, terminator */
    char buf[1 + sizeof(long) * 8 + 1 + 1];

    size_t len = min(count, sizeof(buf) - 1);

    if (copy_from_user(buf, s, len))
        return -EFAULT;
    buf[len] = '\0';

    if (kstrtol(buf, 0, res))
        return gcvSTATUS_INVALID_DATA;

    return count;
# endif
}

static int
gc_vidmem_write(const char __user *buf, size_t count, void *data)
{
    return strtoint_from_user(buf, count, &dumpProcess);
}

static int
gc_dump_trigger_write(const char __user *buf, size_t count, void *data)
{
    return strtoint_from_user(buf, count, &dumpDevice);
}

#if gcdENABLE_MP_SWITCH
static int
gc_switch_core_count(void *m, void *data)
{
    return 0;
}

static int
gc_switch_core_count_write(const char __user *buf, size_t count, void *data)
{
    gckGALDEVICE device = galDevice;
    int coreCount = 0;
    int ret;

    ret = strtoint_from_user(buf, count, &coreCount);

    if (ret && coreCount)
        device->platform->coreCount = coreCount;

    return ret;
}
# endif

static int
gc_load_write(const char __user *buf, size_t count, void *data)
{
    return strtoint_from_user(buf, count, &delayMs);
}
#endif

static int
gc_clk_show(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device = gcvNULL;
    gctUINT i;
    gceSTATUS status;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!gal_device)
        return -ENXIO;

    device = gal_device->devices[0];

    for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++) {
        if (device->kernels[i]) {
            gckHARDWARE hardware = device->kernels[i]->hardware;

            status = gckHARDWARE_QueryFrequency(hardware);
            if (gcmIS_ERROR(status)) {
                len += fs_printf(ptr + len, "query gpu%d clock fail.\n", i);
                continue;
            }

            if (hardware->mcClk)
                len += fs_printf(ptr + len, "gpu%d mc clock: %llu HZ.\n", i, hardware->mcClk);

            if (hardware->shClk)
                len += fs_printf(ptr + len, "gpu%d sh clock: %llu HZ.\n", i, hardware->shClk);
        }
    }

    return len;
}

static gctINT clkScale[2] = { 0, 0 };

static int
set_clk(const char *buf)
{
    gckHARDWARE hardware;
    gctINT n, j, k;
    gctBOOL isSpace = gcvFALSE;
    char data[20];
    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device = gcvNULL;
    gctUINT32 devIndex = 0;

    if (!gal_device)
        return -ENXIO;

    memset(data, 0, 20);
    n = 0; j = 0; k = 0;

    while (gcvTRUE) {
        if ((buf[k] >= '0') && (buf[k] <= '9')) {
            if (isSpace) {
                data[n++] = ' ';
                isSpace = gcvFALSE;
            }
            data[n++] = buf[k];
        } else if (buf[k] == ' ') {
            if (n > 0)
                isSpace = gcvTRUE;
        } else if (buf[k] == '\n') {
            break;
        } else {
            pr_err("Error: command format must be this: echo \"0 32 32\" > /sys/kernel/debug/gc/clk\n");
            return 0;
        }

        k++;

        if (k >= 20)
            break;
    }

    if (4 == sscanf(data, "%d %d %d %d", &devIndex, &dumpCore, &clkScale[0], &clkScale[1])) {
        pr_warn("Change device:%d core:%d MC scale:%d SH scale:%d\n",
                devIndex, dumpCore, clkScale[0], clkScale[1]);
    } else {
        pr_warn("usage: echo \"0 0 32 32\" > clk\n");
        return 0;
    }

    /* Currently only support to scale one device. */
    device = gal_device->devices[devIndex];

    if (dumpCore < gcvCORE_COUNT && device->kernels[dumpCore]) {
        hardware = device->kernels[dumpCore]->hardware;

        gckHARDWARE_SetClock(hardware, clkScale[0], clkScale[1]);
    } else {
        pr_err("Error: invalid core\n");
    }

    return 0;
}

static int
gc_poweroff_timeout_show(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    gckHARDWARE hardware;
    int len = 0;
#ifdef ENABLE_VIV_DEBUG_FS
    void *ptr = m;
#else
    char *ptr = (char *)m;
#endif

    if (!gal_device)
        return -ENXIO;

    hardware = gal_device->devices[0]->kernels[0]->hardware;

#ifdef ENABLE_VIV_DEBUG_FS
    len += fs_printf(ptr + len, "power off timeout: %d ms.\n", hardware->powerOffTimeout);
#else
    len += sprintf(ptr + len, "power off timeout: %d ms.\n", hardware->powerOffTimeout);
#endif

    return len;
}

static int
poweroff_timeout_set(const char *buf)
{
    gckGALDEVICE gal_device = galDevice;
    gctINT i, ret;
    gckDEVICE device = gcvNULL;
    gctUINT32 devIndex;

    if (!gal_device)
        return -ENXIO;

    for (devIndex = 0; devIndex < gal_device->args.devCount; devIndex++) {
        device = gal_device->devices[devIndex];

        for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++) {
            if (device->kernels[i]) {
                gckHARDWARE hardware = device->kernels[i]->hardware;

                ret = kstrtouint(buf, 0, &hardware->powerOffTimeout);
                if (ret < 0)
                    return ret;
            }
        }
    }

    return 0;
}

static int
gc_config_cluster_show(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    gckDEVICE device;
    int i, len = 0;
    char *ptr = (char *)m;

    if (!gal_device)
        return -ENXIO;

    device = gal_device->devices[0];

    for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++) {
        if (device->kernels[i]) {
            gckHARDWARE hardware = device->kernels[i]->hardware;
            gctUINT32 clusterMask;

            gckHARDWARE_QueryClusterInfo(hardware, &clusterMask);
            len += fs_printf(ptr + len, "device%d core%d cluster mask:0x%x\n", 0, i, clusterMask);
        }
    }

    return len;
}

static int
config_cluster(const char *buf)
{
    unsigned int clusterMask;
    gckGALDEVICE gal_device = galDevice;
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device;
    int i = 0;

#if !gcdENABLE_DYNAMIC_CLUSTERS
    gcmkPRINT("can not config cluster due to not enabled.");
    gcmkONERROR(gcvSTATUS_INVALID_DATA);
#endif

    if (kstrtoint(buf, 0, &clusterMask))
        return gcvSTATUS_INVALID_DATA;

    if (!gal_device)
        return -ENXIO;

    device = gal_device->devices[0];

    gcmkONERROR(gckOS_AcquireMutex(device->os, device->commitMutex, gcvINFINITE));
    for (i = gcvCORE_MAJOR; i < gcvCORE_COUNT; i++) {
        if (device->kernels[i]) {
            gckHARDWARE hardware = device->kernels[i]->hardware;

            if (!(~hardware->identity.clusterAvailMask & clusterMask))
                gckHARDWARE_ConfigCluster(hardware, clusterMask);
            else
                gcmkPRINT("Invalid cluster config 0x%x", clusterMask);
        }
    }

    gcmkONERROR(gckOS_ReleaseMutex(device->os, device->commitMutex));

OnError:
    return status;
}

#ifdef ENABLE_VIV_DEBUG_FS
static int
debugfs_copy_from_user(char *k_buf, const char __user *buf, size_t count)
{
    int ret;

    ret = copy_from_user(k_buf, buf, count);
    if (ret != 0) {
        pr_err("Error: lost data: %d\n", (int)ret);
        return -1;
    }

    k_buf[count] = 0;

    return count;
}

static int
gc_clk_write(const char __user *buf, size_t count, void *data)
{
    size_t ret, _count;
    char k_buf[30];

    _count = min_t(size_t, count, (sizeof(k_buf) - 1));

    ret = debugfs_copy_from_user(k_buf, buf, _count);
    if (ret == -1)
        return ret;

    set_clk(k_buf);

    return ret;
}

static int
gc_poweroff_timeout_write(const char __user *buf, size_t count, void *data)
{
    size_t ret;
    char k_buf[30];

    ret = debugfs_copy_from_user(k_buf, buf, count);
    if (ret == -1)
        return ret;

    poweroff_timeout_set(k_buf);

    return ret;
}

static int
gc_config_cluster_write(const char __user *buf, size_t count, void *data)
{
    size_t ret;
    char k_buf[30];

    ret = debugfs_copy_from_user(k_buf, buf, count);
    if (ret == -1)
        return ret;

    config_cluster(k_buf);

    return ret;
}

static int
gc_info_show_debugfs(struct seq_file *m, void *data)
{
    return gc_info_show((void *)m, data);
}

static int
gc_clients_show_debugfs(struct seq_file *m, void *data)
{
    return gc_clients_show((void *)m, data);
}

static int
gc_meminfo_show_debugfs(struct seq_file *m, void *data)
{
    return gc_meminfo_show((void *)m, data);
}

static int
gc_mmuinfo_show_debugfs(struct seq_file *m, void *data)
{
    return gc_mmuinfo_show();
}

static int
gc_idle_show_debugfs(struct seq_file *m, void *data)
{
    return gc_idle_show((void *)m, data);
}

static int
gc_db_old_show_debugfs(struct seq_file *m, void *data)
{
    return gc_db_old_show((void *)m, data, gcvTRUE);
}

static int
gc_db_show_debugfs(struct seq_file *m, void *data)
{
    return gc_db_show((void *)m, data, gcvTRUE);
}

static int
gc_version_show_debugfs(struct seq_file *m, void *data)
{
    return gc_version_show((void *)m, data);
}

static int
gc_vidmem_old_show_debugfs(struct seq_file *m, void *data)
{
    return gc_vidmem_old_show((void *)m, data, gcvTRUE);
}

static int
gc_vidmem_show_debugfs(struct seq_file *m, void *data)
{
    return gc_vidmem_show((void *)m, data, gcvTRUE);
}

static int
gc_reserved_mem_usage_show_debugfs(struct seq_file *m, void *data)
{
    return gc_reserved_mem_usage_show((void *)m);
}

static int
gc_dump_trigger_show_debugfs(struct seq_file *m, void *data)
{
    return gc_dump_trigger_show((void *)m, data);
}

#if gcdSUPPORT_DEVICE_ACPI
static int
gc_suspend_trigger(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    struct device *dev = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    if (!gal_device)
        return -ENXIO;

    dev = (struct device *)galDevice->devices[0]->dev;
    if (!dev)
        return -ENXIO;

    gcmkONERROR(dev->driver->pm->suspend(dev));

OnError:
    return status;
}

static int
gc_resume_trigger(void *m, void *data)
{
    gckGALDEVICE gal_device = galDevice;
    struct device *dev = gcvNULL;
    gceSTATUS status = gcvSTATUS_OK;

    if (!gal_device)
        return -ENXIO;

    dev = (struct device *)galDevice->devices[0]->dev;
    if (!dev)
        return -ENXIO;

    gcmkONERROR(dev->driver->pm->resume(dev));

OnError:
    return status;
}

static int
ft_gettemp(gcsPLATFORM *Platform)
{
       struct device *dev = &Platform->device->dev;
       acpi_handle handle = ACPI_HANDLE(dev);
       acpi_status status;
       unsigned long long data;
       unsigned int tmp;

       status = acpi_evaluate_integer(handle, "PTMP", NULL, &data);
        if (ACPI_FAILURE(status)) {
                pr_err("GCF Method failed %s\n", __func__);
                return -ENODEV;
        }

        if (data < 0) {
               pr_err("%s Failed to set the freq to %llx\n", __func__, data);
       }
        tmp = data;

       return tmp;
}

static int
ft_getvolt(gcsPLATFORM *Platform)
{
       unsigned long volt;
       gceSTATUS status = gcvSTATUS_OK;
       struct device *dev = &Platform->device->dev;

       status = gpu_get_data_from_acpi(dev, "PGDV", FT_3D, FT_Shader, &volt);

       return volt;
}

static int
gc_temp_trigger(void *m, void *data)
{
       gckGALDEVICE gal_device = galDevice;
       gctUINT32 temp = 0;
       gcsPLATFORM *platform;
       int len = 0;
#ifdef ENABLE_VIV_DEBUGS_FS
       void *ptr = m;
#else
       char *ptr = (char *)m;
#endif

       if (!gal_device)
               return -ENXIO;

       platform = gal_device->platform;
       temp = ft_gettemp(platform);
       len = fs_printf(ptr, "gpu temp :%d\n", temp);

       return len;
}

static int
gc_volt_trigger(void *m, void *data)
{
       gckGALDEVICE gal_device = galDevice;
       gctUINT32 temp = 0;
       gcsPLATFORM *platform;
       int len = 0;
#ifdef ENABLE_VIV_DEBUGS_FS
       void *ptr = m;
#else
       char *ptr = (char *)m;
#endif

       if (!gal_device)
               return -ENXIO;

       platform = gal_device->platform;
       temp = ft_getvolt(platform);
       len = fs_printf(ptr, "gpu volt :%d\n", temp);

       return len;
}

static int
gc_total_mem_trigger(void *m, void *data)
{
	gcsPLATFORM *platform;
	gckGALDEVICE gal_device = galDevice;
	int total_mem = 0;
	int len = 0;
#ifdef ENABLE_VIV_DEBUGS_FS
       void *ptr = m;
#else
       char *ptr = (char *)m;
#endif
	
       if (!gal_device)
               return -ENXIO;
		
	platform = gal_device->platform;
	total_mem = ft_gettotalmem(platform);
	if (total_mem < 0)
		len = fs_printf(ptr, "firmware is not support!\n");
	else
		len = fs_printf(ptr, "gpu vpu dc total mem :%d (MB)\n", total_mem/1024);

	return len;
}

static int
gc_suspend_trigger_debugfs(struct seq_file *m, void *data)
{
    return gc_suspend_trigger((void *)m, data);
}
static int
gc_resume_trigger_debugfs(struct seq_file *m, void *data)
{
    return gc_resume_trigger((void *)m, data);
}

static int
gc_temp_trigger_debugfs(struct seq_file *m, void *data)
{
       return gc_temp_trigger((void *)m, data);
}

static int
gc_volt_trigger_debugfs(struct seq_file *m, void *data)
{
       return gc_volt_trigger((void *)m, data);
}

static int
gc_total_mem_debugfs(struct seq_file *m, void *data)
{
       return gc_total_mem_trigger((void *)m, data);
}
#endif


static int
gc_clk_show_debugfs(struct seq_file *m, void *data)
{
    return gc_clk_show((void *)m, data);
}

static int
gc_poweroff_timeout_show_debugfs(struct seq_file *m, void *data)
{
    return gc_poweroff_timeout_show((void *)m, data);
}

static int
gc_config_cluster_show_debugfs(struct seq_file *m, void *data)
{
    return gc_config_cluster_show((void *)m, data);
}

#if gcdENABLE_MP_SWITCH
static int
gc_switch_core_count_debugfs(struct seq_file *m, void *data)
{
    return gc_switch_core_count((void *)m, data);
}
# endif

static int
gc_load_show_debugfs(struct seq_file *m, void *data)
{
    return gc_load_show((void *)m, data);
}

static gcsINFO InfoList[] = {
    { "info", gc_info_show_debugfs },
    { "clients", gc_clients_show_debugfs },
    { "meminfo", gc_meminfo_show_debugfs },
    { "mmu", gc_mmuinfo_show_debugfs },
    { "idle", gc_idle_show_debugfs },
    { "database", gc_db_old_show_debugfs },
    { "database64x", gc_db_show_debugfs },
    { "version", gc_version_show_debugfs },
    { "vidmem", gc_vidmem_old_show_debugfs, gc_vidmem_write },
    { "vidmem64x", gc_vidmem_show_debugfs, gc_vidmem_write },
    { "reserved_mem_usage", gc_reserved_mem_usage_show_debugfs, gc_vidmem_write },
    { "dump_trigger", gc_dump_trigger_show_debugfs, gc_dump_trigger_write },
    { "clk", gc_clk_show_debugfs, gc_clk_write },
    { "poweroff_timeout", gc_poweroff_timeout_show_debugfs, gc_poweroff_timeout_write },
    { "config_cluster", gc_config_cluster_show_debugfs, gc_config_cluster_write },
#if gcdENABLE_MP_SWITCH
    { "core_count", gc_switch_core_count_debugfs, gc_switch_core_count_write },
# endif
    { "load", gc_load_show_debugfs, gc_load_write },
#if gcdSUPPORT_DEVICE_ACPI
    { "suspend_trigger", gc_suspend_trigger_debugfs},
    { "resume_trigger", gc_resume_trigger_debugfs},
    {"temp", gc_temp_trigger_debugfs},
    {"voltage", gc_volt_trigger_debugfs},
    {"total_mem", gc_total_mem_debugfs},
#endif
};
#else
static ssize_t
info_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_info_show((void *)buf, NULL);
}
DEVICE_ATTR_RO(info);

static ssize_t
clients_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_clients_show((void *)buf, NULL);
}
DEVICE_ATTR_RO(clients);

static ssize_t
meminfo_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_meminfo_show((void *)buf, NULL);
}
DEVICE_ATTR_RO(meminfo);

static ssize_t
mmu_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_mmuinfo_show();
}
DEVICE_ATTR_RO(mmu);

static ssize_t
idle_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_idle_show((void *)buf, NULL);
}
DEVICE_ATTR_RO(idle);

static ssize_t
database_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_db_old_show((void *)buf, NULL, gcvFALSE);
}
DEVICE_ATTR_RO(database);

static ssize_t
database64x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_db_show((void *)buf, NULL, gcvFALSE);
}
DEVICE_ATTR_RO(database64x);

static ssize_t
version_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_version_show((void *)buf, NULL);
}
DEVICE_ATTR_RO(version);

static ssize_t
load_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_load_show((void *)buf, NULL);
}

static ssize_t
reserved_mem_usage_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_reserved_mem_usage_show((void *)buf);
}
DEVICE_ATTR_RO(reserved_mem_usage);

static ssize_t
vidmem_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_vidmem_show((void *)buf, NULL, gcvFALSE);
}

static ssize_t
vidmem_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (kstrtoint(buf, 0, &dumpProcess))
        return gcvSTATUS_INVALID_DATA;
    return count;
}
DEVICE_ATTR_RW(vidmem);

static ssize_t
vidmem64x_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_vidmem_show((void *)buf, NULL, gcvFALSE);
}

static ssize_t
vidmem64x_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (kstrtoint(buf, 0, &dumpProcess))
        return gcvSTATUS_INVALID_DATA;
    return count;
}
DEVICE_ATTR_RW(vidmem64x);

static ssize_t
dump_trigger_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_dump_trigger_show((void *)buf, NULL);
}

static ssize_t
dump_trigger_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (kstrtoint(buf, 0, &dumpDevice))
        return gcvSTATUS_INVALID_DATA;
    return count;
}
DEVICE_ATTR_RW(dump_trigger);

static ssize_t
clk_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_clk_show((void *)buf, NULL);
}

static ssize_t
clk_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    set_clk(buf);
    return count;
}
DEVICE_ATTR_RW(clk);

static ssize_t
poweroff_timeout_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_poweroff_timeout_show((void *)buf, NULL);
}

static ssize_t
poweroff_timeout_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    poweroff_timeout_set(buf);
    return count;
}
DEVICE_ATTR_RW(poweroff_timeout);

static ssize_t
config_cluster_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_config_cluster_show((void *)buf, NULL);
}

static ssize_t
config_cluster_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    config_cluster(buf);
    return count;
}
DEVICE_ATTR_RW(config_cluster);

static ssize_t
load_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (kstrtoint(buf, 0, &delayMs))
        return gcvSTATUS_INVALID_DATA;
    return count;
}
DEVICE_ATTR_RW(load);

static ssize_t
gc_suspend_trigger_debugfs(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_suspend_trigger((void *)buf, NULL);
}

static ssize_t
gc_resume_trigger_debugfs(struct device *dev, struct device_attribute *attr, char *buf)
{
    return gc_resume_trigger((void *)buf, NULL);
}

static struct attribute *ftg340_sys_attrs[] = {
    &dev_attr_info.attr,
    &dev_attr_clients.attr,
    &dev_attr_meminfo.attr,
    &dev_attr_mmu.attr,
    &dev_attr_idle.attr,
    &dev_attr_database.attr,
    &dev_attr_database64x.attr,
    &dev_attr_version.attr,
    &dev_attr_load.attr,
    &dev_attr_reserved_mem_usage.attr,
    &dev_attr_vidmem.attr,
    &dev_attr_vidmem64x.attr,
    &dev_attr_dump_trigger.attr,
    &dev_attr_clk.attr,
    &dev_attr_poweroff_timeout.attr,
    &dev_attr_config_cluster.attr,
    &dev_attr_load.attr,
    NULL,
};
ATTRIBUTE_GROUPS(ftg340_sys);
#endif

static gceSTATUS
_DebugfsInit(gckGALDEVICE Device)
{
    gceSTATUS status = gcvSTATUS_OK;

#ifdef ENABLE_VIV_DEBUG_FS
    gckDEBUGFS_DIR dir = &Device->debugfsDir;

    gcmkONERROR(gckDEBUGFS_DIR_Init(dir, gcvNULL, "gc"));
    gcmkONERROR(gckDEBUGFS_DIR_CreateFiles(dir, InfoList, gcmCOUNTOF(InfoList), Device));
#else
    int ret;
    /* TODO. */
    struct device *dev = (struct device *)Device->devices[0]->dev;

    ret = sysfs_create_groups(&dev->kobj, ftg340_sys_groups);
    if (ret < 0)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
#endif
    galDevice = Device;

OnError:
    return status;
}

static void
_DebugfsCleanup(gckGALDEVICE Device)
{
#ifdef ENABLE_VIV_DEBUG_FS
    gckDEBUGFS_DIR dir = &Device->debugfsDir;

    if (Device->debugfsDir.root) {
        gcmkVERIFY_OK(gckDEBUGFS_DIR_RemoveFiles(dir, InfoList, gcmCOUNTOF(InfoList)));

        gckDEBUGFS_DIR_Deinit(dir);
    }
#else
    /* TODO. */
    struct device *dev = (struct device *)Device->devices[0]->dev;

    sysfs_remove_groups(&dev->kobj, ftg340_sys_groups);
#endif
}

/*******************************************************************************
 ************************** Memory Allocation Wrappers *************************
 *******************************************************************************/

static gceSTATUS
_FreeMemory(gckGALDEVICE Device, gctPOINTER Logical, gctPHYS_ADDR Physical)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Device=%p Logical=%p Physical=%p",
                   Device, Logical, Physical);

    gcmkVERIFY_ARGUMENT(Device);

    status = gckOS_FreeNonPagedMemory(Device->os, Physical, Logical,
                                      ((PLINUX_MDL)Physical)->numPages * PAGE_SIZE);

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_SetupContiguousVidMem(gckGALDEVICE Device, gctUINT Index, const gcsMODULE_PARAMETERS *Args)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckGALDEVICE device = Device;
    gctUINT i = Index;

    gcmkHEADER_ARG("Device=%p Args=%p", Device, Args);

    if (i >= gcdSYSTEM_RESERVE_COUNT)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    /* set up the contiguous memory */
    device->contiguousBases[i] = Args->contiguousBases[i];
    device->contiguousSizes[i] = Args->contiguousSizes[i];
    device->requestedContiguousBases[i] = 0;
    device->requestedContiguousSizes[i] = 0;

    if (!device->contiguousSizes[i]) {
        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

    if (device->os->iommu) {
        /* Disable contiguous memory pool. */
        device->contiguousVidMems[i] = gcvNULL;
        device->contiguousSizes[i] = 0;
    } else {
        gctPHYS_ADDR_T contiguousBases = device->contiguousBases[i];
        gctSIZE_T contiguousSizes = device->contiguousSizes[i];
        gckDEVICE dev = device->devices[0];

        gcmkASSERT(dev != gcvNULL);

        if (dev->reserve32bitVA && !device->lowContiguousSize && !Args->lockLargeVA) {
            gctPHYS_ADDR_T gpuBase;
            gctPHYS_ADDR_T lowContiguousBase = contiguousBases;
            gctSIZE_T lowContiguousSize = contiguousSizes;

            gckOS_CPUPhysicalToGPUPhysical(device->os, contiguousBases, &gpuBase);

            if (gpuBase < gcd4G_SIZE) {
                if (gpuBase + contiguousSizes > gcd4G_SIZE) {
                    gckOS_GPUPhysicalToCPUPhysical(device->os, gcd4G_SIZE, &contiguousBases);

                    /* Adjust low VA range. */
                    if (gpuBase < gcdVA_RESERVED_SIZE)
                        contiguousBases -= gcdVA_RESERVED_SIZE;
                    else if (gpuBase > gcd4G_SIZE - gcd4G_VA_FM_SIZE)
                        contiguousBases += gpuBase - (gcd4G_SIZE - gcd4G_VA_FM_SIZE);

                    lowContiguousSize = contiguousBases - lowContiguousBase;
                    contiguousSizes = (lowContiguousSize >= gcd4G_VA_FM_SIZE) ? (contiguousSizes - lowContiguousSize) : 0;
                } else {
                    contiguousSizes = 0;
                }
            } else {
                if (contiguousSizes > gcd4G_VA_FM_SIZE) {
                    /* At least reserve gcd4G_VA_FM_SIZE. */
                    lowContiguousSize = gcd4G_VA_FM_SIZE;
                    contiguousBases += gcd4G_VA_FM_SIZE;
                    contiguousSizes -= gcd4G_VA_FM_SIZE;
                } else {
                    contiguousSizes = 0;
                }
            }

            /* Construct reserve memory pool below 4G. */
            {
                /* Create the contiguous memory heap. */
                status = gckVIDMEM_Construct(device->os,
                                             lowContiguousBase,
                                             lowContiguousSize,
                                             64,
                                             Args->bankSize,
                                             &device->lowContiguousVidMem);

                if (gcmIS_ERROR(status)) {
                    /* Error, disable contiguous memory pool. */
                    device->lowContiguousVidMem = gcvNULL;
                    device->lowContiguousSize   = 0;
                } else {
                    gckALLOCATOR allocator;
                    gctBOOL contiguousRequested = Args->contiguousRequested;

#if gcdCAPTURE_ONLY_MODE
                    contiguousRequested = gcvTRUE;
#endif

                    gcmkONERROR(gckOS_RequestReservedMemory(device->os,
                                                            lowContiguousBase,
                                                            lowContiguousSize,
                                                            "gcLowContMem",
                                                            contiguousRequested,
                                                            &device->lowContiguousPhysical));

                    allocator = ((PLINUX_MDL)device->lowContiguousPhysical)->allocator;

                    device->lowContiguousVidMem->capability = allocator->capability | gcvALLOC_FLAG_MEMLIMIT;
                    device->lowContiguousVidMem->physical = device->lowContiguousPhysical;
                    device->lowContiguousBase = lowContiguousBase;
                    device->lowContiguousSize = lowContiguousSize;
                }
            }

            if (contiguousSizes == 0) {
                device->contiguousVidMems[i] = gcvNULL;
                device->contiguousSizes[i] = 0;

                return gcvSTATUS_OK;
            }
        }

        /* Create the contiguous memory heap. */
        status = gckVIDMEM_Construct(device->os,
                                     contiguousBases,
                                     contiguousSizes,
                                     64,
                                     Args->bankSize,
                                     &device->contiguousVidMems[i]);

        if (gcmIS_ERROR(status)) {
            /* Error, disable contiguous memory pool. */
            device->contiguousVidMems[i] = gcvNULL;
            device->contiguousSizes[i] = 0;
        } else {
            gckALLOCATOR allocator;
            gctBOOL contiguousRequested = Args->contiguousRequested;

#if gcdCAPTURE_ONLY_MODE
            contiguousRequested = gcvTRUE;
#endif

            gcmkONERROR(gckOS_RequestReservedMemory(device->os,
                                                    contiguousBases,
                                                    contiguousSizes,
                                                    "gcContMem",
                                                    contiguousRequested,
                                                    &device->contiguousPhysicals[i]));

            allocator = ((PLINUX_MDL)device->contiguousPhysicals[i])->allocator;

            device->contiguousVidMems[i]->capability = allocator->capability | gcvALLOC_FLAG_MEMLIMIT;
            device->contiguousVidMems[i]->physical = device->contiguousPhysicals[i];
            device->requestedContiguousBases[i] = contiguousBases;
            device->requestedContiguousSizes[i] = contiguousSizes;
            device->contiguousBases[i] = contiguousBases;
            device->contiguousSizes[i] = contiguousSizes;
        }
    }

    if (Args->showArgs) {
        gcmkPRINT("ftg340 Info: ContiguousBase=0x%llx ContiguousSize=0x%zx\n",
                  device->contiguousBases[i], device->contiguousSizes[i]);
    }

OnError:
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_SetupExternalSRAMVidMem(gckGALDEVICE galDevice, const gcsMODULE_PARAMETERS *Args)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device;
    gctUINT32 i, j, index, devIndex;

    gcmkHEADER_ARG("Device=%p", galDevice);

    /* Setup external SRAM memory region. */
    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        if (!galDevice->extSRAMSizes[i] && galDevice->devices[0]->extSRAMSizes[i])
            galDevice->extSRAMSizes[i] = galDevice->devices[0]->extSRAMSizes[i];

        if (galDevice->extSRAMSizes[i] > 0) {
            /* create the external SRAM memory heap */
            status = gckVIDMEM_Construct(galDevice->os,
                                         galDevice->extSRAMBases[i],
                                         galDevice->extSRAMSizes[i],
                                         64,
                                         0,
                                         &galDevice->extSRAMVidMems[i]);

            if (gcmIS_ERROR(status)) {
                /* Error, disable external SRAM heap. */
                galDevice->extSRAMSizes[i] = 0;
            } else {
                char sRAMName[40];
                snprintf(sRAMName, gcmSIZEOF(sRAMName) - 1, "ftg340 external sram%d", i);

#if gcdCAPTURE_ONLY_MODE
                galDevice->args.sRAMRequested = gcvTRUE;
#endif
                /* Map external SRAM memory. */
                gcmkONERROR(gckOS_RequestReservedMemory(galDevice->os,
                                                        galDevice->extSRAMBases[i],
                                                        galDevice->extSRAMSizes[i],
                                                        sRAMName,
                                                        galDevice->args.sRAMRequested,
                                                        &galDevice->extSRAMPhysicals[i]));

                galDevice->extSRAMVidMems[i]->physical = galDevice->extSRAMPhysicals[i];
            }
        }
    }

    for (devIndex = 0; devIndex < galDevice->args.devCount; devIndex++) {
        device = galDevice->devices[devIndex];
        index = 0;

        for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
            if (Args->devSRAMIDs[devIndex] & (1 << i)) {
                device->extSRAMVidMems[index] = galDevice->extSRAMVidMems[i];
                device->extSRAMPhysicals[index] = galDevice->extSRAMPhysicals[i];

                for (j = 0; j < gcvCORE_COUNT; j++) {
                    if (device->irqLines[j] != -1 &&
                        device->kernels[j] &&
                        device->extSRAMPhysicals[index]) {
                        device->kernels[j]->hardware->options.extSRAMGPUPhysNames[index]
                            = gckKERNEL_AllocateNameFromPointer(device->kernels[j], device->extSRAMPhysicals[index]);
                    }
                }

                index++;
            }
        }
    }

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 ****************************** Interrupt Handler ******************************
 *******************************************************************************/

static irqreturn_t
isrRoutine(int irq, void *ctxt)
{
    gceSTATUS status;
    gckKERNEL kernel = (gckKERNEL)ctxt;

    /* Call kernel interrupt notification. */
    status = gckHARDWARE_Interrupt(kernel->hardware);

    if (gcmIS_SUCCESS(status)) {
        up(kernel->sema);
        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}

static const char * const isrNames[] = {
    "ftg340:0",
    "ftg340:3d-1",
    "ftg340:3d-2",
    "ftg340:3d-3",
    "ftg340:3d-4",
    "ftg340:3d-5",
    "ftg340:3d-6",
    "ftg340:3d-7",
    "ftg340:3d-8",
    "ftg340:3d-9",
    "ftg340:3d-10",
    "ftg340:3d-11",
    "ftg340:3d-12",
    "ftg340:3d-13",
    "ftg340:3d-14",
    "ftg340:3d-15",
#if gcdDEC_ENABLE_AHB
    "ftg340:dec"
#endif
};

static int
isrRoutinePoll(void *ctxt)
{
    gckKERNEL kernel = (gckKERNEL)ctxt;

    while (1) {
        if (unlikely(kernel->killThread)) {
            /* The daemon exits. */
            while (!kthread_should_stop())
                gckOS_Delay(kernel->os, 1);

            return 0;
        }

        isrRoutine(-1, kernel);

        gckOS_Delay(kernel->os, 10);
    }

    return 0;
}

static gceSTATUS
_SetupIsr(gckDEVICE Device, gceCORE Core)
{
    gctINT ret = 0;
    gceSTATUS status = gcvSTATUS_OK;
    irq_handler_t handler;
    gctUINT64 isrPolling = 0;

    gcmkHEADER_ARG("Device=%p Core=%d", Device, Core);

    gcmkVERIFY_ARGUMENT(Device);

    gcmSTATIC_ASSERT(gcvCORE_COUNT == gcmCOUNTOF(isrNames),
                     "isrNames array does not match core types");

    gckOS_QueryOption(Device->os, "isrPoll", &isrPolling);
    if (isrPolling) {
        if (Device->isrThread[Core])
            return status;

        /* use kthread to poll int stat */
        if (gcmBITTEST(isrPolling, Core) != 0) {
            struct task_struct *task;

            Device->killIsrThread = gcvFALSE;

            task = kthread_run(isrRoutinePoll,
                               (gctPOINTER)Device->kernels[Core],
                               "%s_poll", isrNames[Core]);

            if (IS_ERR(task)) {
                gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                               "%s(%d): Could not start the intr poll thread.\n",
                               __func__, __LINE__);

                gcmkONERROR(gcvSTATUS_GENERIC_IO);
            }

            gcmkPRINT("ftg340: polling core%d int state\n", Core);

            Device->isrThread[Core] = task;
            Device->isrInitializeds[Core] = gcvTRUE;

            return status;
        }
        /* it should not run to here */
        return gcvSTATUS_INVALID_ARGUMENT;
    }

    handler = isrRoutine;

    /*
     * Hook up the isr based on the irq line.
     * For shared irq, device-id can not be 0, but CORE_MAJOR value is.
     * Add by 1 here and subtract by 1 in isr to fix the issue.
     */
    ret = request_irq(Device->irqLines[Core], handler, gcdIRQF_FLAG,
                      isrNames[Core], (void *)Device->kernels[Core]);

    if (ret != 0) {
        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                       "%s(%d): Could not register irq line %d (error=%d)\n",
                       __func__, __LINE__, Device->irqLines[Core], ret);

        gcmkONERROR(gcvSTATUS_GENERIC_IO);
    }

    /* Mark ISR as initialized. */
    Device->isrInitializeds[Core] = gcvTRUE;

OnError:
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_ReleaseIsr(gckDEVICE Device, gceCORE Core)
{
    gcmkHEADER_ARG("Device=%p Core=%d", Device, Core);

    gcmkVERIFY_ARGUMENT(Device);

    /* release the irq */
    if (Device->isrInitializeds[Core]) {
        if (Device->isrThread[Core]) {
            Device->killIsrThread = gcvTRUE;
            kthread_stop(Device->isrThread[Core]);
            Device->isrThread[Core] = gcvNULL;
        } else {
            free_irq(Device->irqLines[Core], (void *)(Device->kernels[Core]));
        }

        Device->isrInitializeds[Core] = gcvFALSE;
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

static int
threadRoutine(void *ctxt)
{
    gckKERNEL kernel = (gckKERNEL)ctxt;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_DRIVER,
                   "Starting isr Thread with extension=%p", kernel);

    for (;;) {
        int down;

        down = down_interruptible(kernel->sema);
        if (down && down != -EINTR)
            return down;

        if (unlikely(kernel->killThread)) {
            /* The daemon exits. */
            while (!kthread_should_stop())
                gckOS_Delay(kernel->os, 1);

            return 0;
        }

        gckKERNEL_Notify(kernel, gcvNOTIFY_INTERRUPT);
    }
}

static gceSTATUS
_StartThread(gckDEVICE Device, gceCORE Core)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device = Device;
    struct task_struct *task;

    if (device->kernels[Core] != gcvNULL) {
        /* Start the kernel thread. */
        task = kthread_run(threadRoutine, (void *)device->kernels[Core],
                           "ftg340_deamon/%d-%d", Device->id, Core);

        if (IS_ERR(task)) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): Could not start the kernel thread.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        device->threadCtxts[Core] = task;
        device->threadInitializeds[Core] = gcvTRUE;
        device->kernels[Core]->threadInitialized = gcvTRUE;

        set_user_nice(task, -20);
    } else {
        device->threadInitializeds[Core] = gcvFALSE;
    }

OnError:
    return status;
}

static void
_StopThread(gckDEVICE Device, gceCORE Core)
{
    if (Device->threadInitializeds[Core]) {
        Device->kernels[Core]->killThread = gcvTRUE;
        up(Device->kernels[Core]->sema);

        kthread_stop(Device->threadCtxts[Core]);
        Device->threadCtxts[Core] = gcvNULL;
        Device->threadInitializeds[Core] = gcvFALSE;
    }
}

#if gcdENABLE_SW_PREEMPTION

static int
_ThreadPreempt(void *ctxt)
{
    gckKERNEL kernel = (gckKERNEL)ctxt;

    for (;;) {
        int down;

        down = down_interruptible(kernel->preemptSema);
        if (down && down != -EINTR)
            return down;

        if (unlikely(kernel->killPreemptThread)) {
            while (!kthread_should_stop())
                gckOS_Delay(kernel->os, 1);

            return 0;
        }

        gckKERNEL_PreemptionThread(kernel);
    }
}

static gceSTATUS
_StartPreemptThread(gckDEVICE Device, gceCORE Core)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device = Device;
    struct task_struct *task;

    if (device->kernels[Core] != gcvNULL) {
        /* Start the kernel thread. */
        task = kthread_run(_ThreadPreempt, (void *)device->kernels[Core],
                           "ftg340_preempt/%d-%d", device->id, Core);

        if (IS_ERR(task)) {
            gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                           "%s(%d): Could not start the kernel preempt thread.\n",
                           __func__, __LINE__);

            gcmkONERROR(gcvSTATUS_GENERIC_IO);
        }

        device->preemptThread[Core] = task;
        device->preemptThreadInits[Core] = gcvTRUE;
    } else {
        device->preemptThreadInits[Core] = gcvFALSE;
    }

OnError:
    return status;
}

static void
_StopPreemptThread(gckDEVICE Device, gceCORE Core)
{
    if (Device->preemptThreadInits[Core]) {
        Device->kernels[Core]->killPreemptThread = gcvTRUE;
        up(Device->kernels[Core]->preemptSema);

        kthread_stop((struct task_struct *)Device->preemptThread[Core]);
        Device->preemptThread[Core] = gcvNULL;
        Device->preemptThreadInits[Core] = gcvFALSE;
    }
}
#endif

static gceSTATUS
_ReadRegister(gckDEVICE Device, gctUINT Core, gctUINT32 Address, gctUINT32 *Data)
{
    if (Core >= gcdCORE_3D_COUNT || Address > Device->registerSizes[Core] - 1)
        return gcvSTATUS_INVALID_ARGUMENT;

    *Data = readl((gctUINT8 *)Device->registerBases[Core] + Address);

    return gcvSTATUS_OK;
}

static gceSTATUS
_PowerOnCore(gckDEVICE Device, gctUINT32 coreID)
{
    gcsPLATFORM *platform;
    gckOS os;

    gcmkHEADER();

    os = Device->os;
    platform = os->device->platform;

    gcmkASSERT(Device->id < gcdDEVICE_COUNT);

    /* Try to power on GPU. */
    if (!os->powerStates[Device->id][coreID]) {
        if (platform && platform->ops->setPower)
            gcmkVERIFY_OK(platform->ops->setPower(platform, Device->id, coreID, gcvTRUE));
        os->powerStates[Device->id][coreID] = gcvTRUE;
    }

    if (!os->clockStates[Device->id][coreID]) {
        unsigned long flags;

        if (platform && platform->ops->setClock)
            gcmkVERIFY_OK(platform->ops->setClock(platform, Device->id, coreID, gcvTRUE));

        spin_lock_irqsave(&os->registerAccessLock, flags);
        os->clockStates[Device->id][coreID] = gcvTRUE;
        spin_unlock_irqrestore(&os->registerAccessLock, flags);
    }

#if gcdENABLE_AHBXTTD
    return gcvSTATUS_NOT_SUPPORTED;
#endif

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/* Only used for query memory feature for 3D core. */
static gceSTATUS
_QueryDeviceFeaturesFor3D(gckDEVICE Device, gckGALDEVICE GalDevice)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsFEATURE_DATABASE *database;
    gctUINT32 chipModel, chipRevision, productID, ecoID, customerID;

    /* Before access register, we should power on GPU. */
    _PowerOnCore(Device, 0);

    /* Read chip identity register. */
    gcmkONERROR(_ReadRegister(Device, 0, GC_CHIP_ID_Address, &chipModel));

    /* Read CHIP_REV register. */
    gcmkONERROR(_ReadRegister(Device, 0, GC_CHIP_REV_Address, &chipRevision));

    gcmkONERROR(_ReadRegister(Device, 0, GC_PRODUCT_ID_Address, &productID));

    gcmkONERROR(_ReadRegister(Device, 0, GC_ECO_ID_Address, &ecoID));

    gcmkONERROR(_ReadRegister(Device, 0, GC_CHIP_CUSTOMER_Address, &customerID));

    database = gcQueryFeatureDB(chipModel, chipRevision, productID,
                                ecoID, customerID);

    if (database == gcvNULL) {
        gcmkPRINT("[ftg340]: Feature database is not found, chipModel=0x%0x, chipRevision=0x%x, productID=0x%x, ecoID=0x%x, customerID=0x%x",
                   chipModel, chipRevision, productID, ecoID, customerID);

        gcmkONERROR(gcvSTATUS_NOT_FOUND);
    } else if (database->chipVersion != chipRevision) {
        gcmkPRINT("[ftg340]: Warning: chipRevision mismatch, database chipRevision=0x%x register read chipRevision=0x%x\n",
                  database->chipVersion, chipRevision);
    }

#if gcdENABLE_FORCE_RESERVE_32BIT
    Device->reserve32bitVA = gcvTRUE;
#else
    Device->reserve32bitVA = (database->VIRTUAL_ADDRESS_BITS > 32 && !database->MMU_40BIT_VA_GRAPHICS);
#endif

OnError:
    return status;
}

/*******************************************************************************
 *
 *  gckGALDEVICE_Construct
 *
 *  Constructor.
 *
 *  INPUT:
 *
 *  OUTPUT:
 *
 *      gckGALDEVICE *Device
 *          Pointer to a variable receiving the gckGALDEVICE object pointer on
 *          success.
 */
gceSTATUS
gckGALDEVICE_Construct(gcsPLATFORM *Platform,
                       const gcsMODULE_PARAMETERS *Args,
                       gckGALDEVICE *Device)
{
    gckKERNEL kernel = gcvNULL;
    gckGALDEVICE gal_device;
    gckDEVICE device;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64 isrPolling = -1;
    gctINT32 i = 0;
    gctUINT32 devIndex, globalIndex = 0;
    gctUINT32 index = 0;
    gctUINT32 sysReserveBits;

    gcmkHEADER_ARG("Platform=%p Args=%p", Platform, Args);

    /* Allocate device structure. */
    gal_device = kmalloc(sizeof(struct _gckGALDEVICE), GFP_KERNEL | __GFP_NOWARN);

    if (!gal_device)
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    memset(gal_device, 0, sizeof(struct _gckGALDEVICE));

    gal_device->platform = Platform;
    gal_device->platform->dev = gcvNULL;

    gal_device->args = *Args;

    if (gal_device->args.devCount > gcdDEVICE_COUNT)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    isrPolling = Args->isrPoll;

    /* Construct the gckOS object. */
    gcmkONERROR(gckOS_Construct(gal_device, &gal_device->os));

    /* Loop all the hardware devices. */
    for (devIndex = 0; devIndex < gal_device->args.devCount; devIndex++) {
        /* Construct the gckDEVICE object for os independent core management. */
        gcmkONERROR(gckDEVICE_Construct(gal_device->os, &gal_device->devices[devIndex]));

        device = gal_device->devices[devIndex];

        device->id = devIndex;
        device->vidMemIndex = Args->devMemIDs[devIndex];
        device->configCoreCount = Args->devCoreCounts[devIndex];
        device->showSRAMMapInfo = Args->showArgs;
        device->showMemInfo = Args->showArgs;
        device->platformIndex = Args->platformIDs[devIndex];
        device->dev = Args->devices[device->platformIndex];
        device->processPageTable = (gal_device->args.devCount > 1) ? gcvFALSE : gcvTRUE;
        device->lockLargeVA = (gcdENABLE_40BIT_VA == 1) ? Args->lockLargeVA : gcvFALSE;

        /* Loop all the cores in one deivce. */
        for (i = 0; i < device->configCoreCount; i++) {
            /* Get arguments from global core index. */
            device->irqLines[i] = Args->irqs[globalIndex];
            device->requestedRegisterMemBases[i] = Args->registerBases[globalIndex];
            device->requestedRegisterMemSizes[i] = Args->registerSizes[globalIndex];
            device->registerBasesMapped[i] = Args->registerBasesMapped[globalIndex];

            globalIndex++;
        }

        for (i = 0; i < gcvCORE_COUNT; i++) {
            unsigned long physical = (unsigned long)device->requestedRegisterMemBases[i];

            /* Set up register memory region. */
            if (physical) {
                if (device->registerBasesMapped[i]) {
                    device->registerBases[i] = device->registerBasesMapped[i];
                    device->registerSizes[i] = device->requestedRegisterMemSizes[i];
                    device->requestedRegisterMemBases[i] = 0;
                } else {
#if USE_LINUX_PCIE
                    gcmkPRINT("register should be mapped in platform layer");
#endif
                    if (!request_mem_region(physical,
                                            device->requestedRegisterMemSizes[i],
                                            "ftg340 register region")) {
                        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                                       "%s(%d): Failed to claim %lu bytes @ 0x%llx\n",
                                       __func__, __LINE__,
                                       device->requestedRegisterMemSizes[i], physical);

                        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
                    }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
                    device->registerBases[i] =
                        (gctPOINTER)ioremap(physical, device->requestedRegisterMemSizes[i]);
#else
                    device->registerBases[i] =
                        (gctPOINTER)ioremap_nocache(physical, device->requestedRegisterMemSizes[i]);
#endif
                    device->registerSizes[i] = device->requestedRegisterMemSizes[i];

                    if (!device->registerBases[i]) {
                        gcmkTRACE_ZONE(gcvLEVEL_ERROR, gcvZONE_DRIVER,
                                       "%s(%d): Unable to map %ld bytes @ 0x%zx\n",
                                       __func__, __LINE__,
                                       physical, device->requestedRegisterMemSizes[i]);

                        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
                    }
                }
            }
        }

        gcmkONERROR(_QueryDeviceFeaturesFor3D(device, gal_device));
    }

#if gcdENABLE_TRUST_APPLICATION
    if (gal_device->devices[0]->irqLines[gcvCORE_MAJOR] != -1 ||
        gcmBITTEST(isrPolling, gcvCORE_MAJOR)!= 0) {
        gcmkONERROR(gctaOS_ConstructOS(gal_device->os, &gal_device->taos));
    }
#endif
    for (i = 0; i < gcdDEVICE_COUNT; i++) {
        /* set -1 as default value, to avoid the misc device random minor is 0 */
        gal_device->devIDs[i] = -1;
    }

    gal_device->platform->dev = gal_device->devices[0];

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        /* Setup contiguous video memory pool. */
        gcmkONERROR(_SetupContiguousVidMem(gal_device, i, Args));
    }

    /* Set the external SRAM base address. */
    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        gal_device->extSRAMBases[i] = Args->extSRAMBases[i];
        gal_device->extSRAMSizes[i] = Args->extSRAMSizes[i];
    }

    /* Setup all the local memory for GAL device. */
    for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
        char name[20];

        gal_device->externalBase[i] = Args->externalBase[i];
        gal_device->externalSize[i] = Args->externalSize[i];
        gal_device->exclusiveBase[i] = Args->exclusiveBase[i];
        gal_device->exclusiveSize[i] = Args->exclusiveSize[i];

        if (gal_device->externalSize[i] > 0) {
            /* create the external memory heap */
            status = gckVIDMEM_Construct(gal_device->os,
                                         gal_device->externalBase[i],
                                         gal_device->externalSize[i],
                                         64,
                                         0,
                                         &gal_device->externalVidMem[i]);

            if (gcmIS_ERROR(status)) {
                /* Error, disable external heap. */
                gal_device->externalSize[i] = 0;
            } else {
                sprintf(name, "gcExtMem%d", i);
                /* Map external memory. */
                gcmkONERROR(gckOS_RequestReservedMemory(gal_device->os,
                                                        gal_device->externalBase[i],
                                                        gal_device->externalSize[i],
                                                        name,
                                                        gcvTRUE,
                                                        &gal_device->externalPhysical[i]));

                gal_device->externalVidMem[i]->physical = gal_device->externalPhysical[i];
            }
        }

        if (gal_device->exclusiveSize[i] > 0) {
            /* create the exclusive memory heap */
            status = gckVIDMEM_Construct(gal_device->os,
                                         gal_device->exclusiveBase[i],
                                         gal_device->exclusiveSize[i],
                                         64,
                                         0,
                                         &gal_device->exclusiveVidMem[i]);

            if (gcmIS_ERROR(status)) {
                /* Error, disable exclusive heap. */
                gal_device->exclusiveSize[i] = 0;
            } else {
                gckALLOCATOR allocator;

                sprintf(name, "gcExcMem%d", i);

                /* Map exclusive memory. */
                gcmkONERROR(gckOS_RequestReservedMemory(gal_device->os,
                                                        gal_device->exclusiveBase[i],
                                                        gal_device->exclusiveSize[i],
                                                        name,
                                                        gcvTRUE,
                                                        &gal_device->exclusivePhysical[i]));

                allocator = ((PLINUX_MDL)gal_device->exclusivePhysical[i])->allocator;
                gal_device->exclusiveVidMem[i]->physical = gal_device->exclusivePhysical[i];
                gal_device->exclusiveVidMem[i]->capability |= allocator->capability;
            }
        }
    }

#if gcdEXTERNAL_SRAM_USAGE
    /* Setup external SRAM video memory pool. */
    gcmkONERROR(_SetupExternalSRAMVidMem(gal_device, Args));
#endif

    for (devIndex = 0; devIndex < gal_device->args.devCount; devIndex++) {
        device = gal_device->devices[devIndex];

        if (devIndex)
            device->database = gal_device->devices[0]->database;

        /* Query device memory configuration. */
        device->externalBase = gal_device->externalBase[device->vidMemIndex];
        device->externalSize = gal_device->externalSize[device->vidMemIndex];
        device->externalPhysical = gal_device->externalPhysical[device->vidMemIndex];
        device->externalVidMem = gal_device->externalVidMem[device->vidMemIndex];

        device->exclusiveBase = gal_device->exclusiveBase[device->vidMemIndex];
        device->exclusiveSize = gal_device->exclusiveSize[device->vidMemIndex];
        device->exclusivePhysical = gal_device->exclusivePhysical[device->vidMemIndex];
        device->exclusiveVidMem = gal_device->exclusiveVidMem[device->vidMemIndex];

        sysReserveBits = Args->devSysMemIDs[devIndex];
        index = 0;

        for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
            if (sysReserveBits & (1 << i)) {
                device->contiguousBases[index] = gal_device->contiguousBases[i];
                device->contiguousSizes[index] = gal_device->contiguousSizes[i];
                device->contiguousPhysicals[index] = gal_device->contiguousPhysicals[i];
                device->contiguousVidMems[index] = gal_device->contiguousVidMems[i];
                index++;
            }
        }

        device->lowContiguousBase = gal_device->lowContiguousBase;
        device->lowContiguousSize = gal_device->lowContiguousSize;

        index = 0;

        for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
            if (Args->devSRAMIDs[devIndex] & (1 << i)) {
                device->extSRAMBases[index] = gal_device->extSRAMBases[i];
                device->extSRAMSizes[index] = gal_device->extSRAMSizes[i];
                index++;
            }
        }

        /* Add core for all available major cores. */
        for (i = gcvCORE_MAJOR; i <= gcvCORE_3D_MAX; i++) {
            if (device->irqLines[i] != -1 || gcmBITTEST(isrPolling, i) != 0) {
                gcmkONERROR(gckDEVICE_AddCore(device,
                                              (gceCORE)i,
                                              Args->chipIDs[i],
                                              gal_device,
                                              &device->kernels[i]));

                if (!kernel)
                    kernel = device->kernels[i];

#if gcdENABLE_TRUST_APPLICATION
                if (device->kernels[i]->hardware->options.secureMode == gcvSECURE_IN_TA &&
                    devIndex == 0) {
                    gcmkONERROR(gcTA_Construct(gal_device->taos, (gceCORE)i, &globalTA[i]));
                }
#endif

                gcmkONERROR(gckHARDWARE_SetFastClear(device->kernels[i]->hardware,
                                                     Args->fastClear,
                                                     Args->compression));

                gcmkONERROR(gckHARDWARE_EnablePowerManagement(device->kernels[i]->hardware,
                                                              Args->powerManagement));

#if gcdENABLE_FSCALE_VAL_ADJUST
                gcmkONERROR(gckHARDWARE_SetMinFscaleValue(device->kernels[i]->hardware,
                                                          Args->gpu3DMinClock));

#endif
            } else {
                device->kernels[i] = gcvNULL;
            }
        }
    }

#if !gcdEXTERNAL_SRAM_USAGE
    /* Setup external SRAM video memory pool. */
    gcmkONERROR(_SetupExternalSRAMVidMem(gal_device, Args));
#endif

    if (!kernel)
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);

    if (gal_device->internalPhysical)
        gal_device->internalPhysName = gcmPTR_TO_NAME(gal_device->internalPhysical);

    if (gal_device->lowContiguousPhysical)
        gal_device->lowContiguousPhysName = gcmPTR_TO_NAME(gal_device->lowContiguousPhysical);

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (gal_device->contiguousPhysicals[i])
            gal_device->contiguousPhysNames[i] = gcmPTR_TO_NAME(gal_device->contiguousPhysicals[i]);
    }

    for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
        if (gal_device->externalPhysical[i])
            gal_device->externalPhysName[i] = gcmPTR_TO_NAME(gal_device->externalPhysical[i]);

        if (gal_device->exclusivePhysical[i])
            gal_device->exclusivePhysName[i] = gcmPTR_TO_NAME(gal_device->exclusivePhysical[i]);
    }

    for (devIndex = 0; devIndex < gal_device->args.devCount; devIndex++) {
        device = gal_device->devices[devIndex];

        /* Query device memory configuration. */
        device->externalPhysName = gal_device->externalPhysName[device->vidMemIndex];
        device->exclusivePhysName = gal_device->exclusivePhysName[device->vidMemIndex];

        sysReserveBits = Args->devSysMemIDs[devIndex];

        for (i = 0, index = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
            if (sysReserveBits & (1 << i)) {
                device->contiguousPhysNames[index] = gal_device->contiguousPhysNames[i];
                index++;
            }
        }
    }

    gcmkONERROR(_DebugfsInit(gal_device));

    /* Return pointer to the device. */
    galDevice = gal_device;
    *Device = gal_device;

OnError:
    if (gcmIS_ERROR(status)) {
        /* Roll back. */
        gcmkVERIFY_OK(gckGALDEVICE_Destroy(gal_device));
    }

    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 *
 *  gckGALDEVICE_Destroy
 *
 *  Class destructor.
 *
 *  INPUT:
 *
 *      Nothing.
 *
 *  OUTPUT:
 *
 *      Nothing.
 *
 *  RETURNS:
 *
 *      Nothing.
 */
gceSTATUS
gckGALDEVICE_Destroy(gckGALDEVICE gal_device)
{
    gctINT i, j = 0;
    gckKERNEL kernel = gcvNULL;
    gctINT32 devIndex;
    gckDEVICE device;

    gcmkHEADER_ARG("gal_device=%p", gal_device);

    if (gal_device) {
        if (!gal_device->devices[0])
            goto free_gal_device;

        kernel = gal_device->devices[0]->kernels[gcvCORE_MAJOR];

        /* Free all the local memories. */
        for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
            if (gal_device->externalPhysName[i]) {
                gcmRELEASE_NAME(gal_device->externalPhysName[i]);
                gal_device->externalPhysName[i] = 0;
            }

            if (gal_device->exclusivePhysName[i]) {
                gcmRELEASE_NAME(gal_device->exclusivePhysName[i]);
                gal_device->exclusivePhysName[i] = 0;
            }
        }

        for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
            if (gal_device->contiguousPhysNames[i]) {
                gcmRELEASE_NAME(gal_device->contiguousPhysNames[i]);
                gal_device->contiguousPhysNames[i] = 0;
            }
        }

        if (gal_device->lowContiguousPhysName) {
            gcmRELEASE_NAME(gal_device->lowContiguousPhysName);
            gal_device->lowContiguousPhysName = 0;
        }

        _DebugfsCleanup(gal_device);

        for (devIndex = gal_device->args.devCount - 1; devIndex >= 0; devIndex--) {
            device = gal_device->devices[devIndex];

            if (!device)
                continue;

            for (i = 0; i < gcvCORE_COUNT; i++) {
                if (device->kernels[i]) {
                    kernel = device->kernels[i];

#if gcdENABLE_TRUST_APPLICATION
                    if (kernel->hardware->options.secureMode == gcvSECURE_IN_TA &&
                        globalTA[i] && !devIndex) {
                        gcTA_Destroy(globalTA[i]);
                        globalTA[i] = gcvNULL;
                    }
#endif

                    if (gal_device->gotoShutdown)
                        kernel->dbCreated = gcvFALSE;

                    for (j = gcvSRAM_INTERNAL0; j < gcvSRAM_INTER_COUNT; j++) {
                        if (kernel->sRAMPhysical[j] != gcvNULL) {
                            /* Release reserved SRAM memory. */
                            gckOS_ReleaseReservedMemory(device->os,
                                                        kernel->sRAMPhysical[j]);

                            kernel->sRAMPhysical[j] = gcvNULL;
                        }

                        if (kernel->sRAMVidMem[j] != gcvNULL) {
                            /* Destroy the SRAM contiguous heap. */
                            gcmkVERIFY_OK(gckVIDMEM_Destroy(kernel->sRAMVidMem[j]));
                            kernel->sRAMVidMem[j] = gcvNULL;
                        }
                    }
                }
            }

            gcmkVERIFY_OK(gckDEVICE_DestroyCores(device));

            for (i = 0; i < gcvCORE_COUNT; i++) {
                if (device->registerBases[i]) {
                    /* Unmap register memory. */
                    if (device->requestedRegisterMemBases[i] != 0) {
                        iounmap(device->registerBases[i]);

                        release_mem_region(device->requestedRegisterMemBases[i],
                                           device->requestedRegisterMemSizes[i]);
                    }

                    device->registerBases[i] = gcvNULL;
                    device->requestedRegisterMemBases[i] = 0;
                    device->requestedRegisterMemSizes[i] = 0;
                }
            }

            gcmkVERIFY_OK(gckDEVICE_Destroy(device->os, device));

            device = gcvNULL;
        }

        for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
            if (gal_device->extSRAMPhysicals[i]) {
                gckOS_ReleaseReservedMemory(gal_device->os,
                                            gal_device->extSRAMPhysicals[i]);

                gal_device->extSRAMPhysicals[i] = gcvNULL;
            }

            if (gal_device->extSRAMVidMems[i]) {
                gcmkVERIFY_OK(gckVIDMEM_Destroy(gal_device->extSRAMVidMems[i]));
                gal_device->extSRAMVidMems[i] = gcvNULL;
            }
        }

        /* Free all the local memories. */
        for (i = 0; i < gcdLOCAL_MEMORY_COUNT; i++) {
            if (gal_device->externalPhysical[i]) {
                gckOS_ReleaseReservedMemory(gal_device->os,
                                            gal_device->externalPhysical[i]);

                gal_device->externalPhysical[i] = gcvNULL;
            }

            if (gal_device->externalVidMem[i]) {
                /* destroy the external heap */
                gcmkVERIFY_OK(gckVIDMEM_Destroy(gal_device->externalVidMem[i]));
                gal_device->externalVidMem[i] = gcvNULL;
            }

            if (gal_device->exclusivePhysical[i]) {
                gckOS_ReleaseReservedMemory(gal_device->os,
                                            gal_device->exclusivePhysical[i]);

                gal_device->exclusivePhysical[i] = gcvNULL;
            }

            if (gal_device->exclusiveLogical[i])
                gal_device->exclusiveLogical[i] = gcvNULL;

            if (gal_device->exclusiveVidMem[i]) {
                /* destroy the external heap */
                gcmkVERIFY_OK(gckVIDMEM_Destroy(gal_device->exclusiveVidMem[i]));
                gal_device->exclusiveVidMem[i] = gcvNULL;
            }
        }

         /*
          * Destroy contiguous memory pool after gckDEVICE destroyed. gckDEVICE
          * may allocates GPU memory types from SYSTEM pool.
          */
        for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
            if (gal_device->contiguousPhysicals[i]) {
                if (!gal_device->requestedContiguousBases[i]) {
                    gcmkVERIFY_OK(_FreeMemory(gal_device,
                                              gal_device->contiguousLogicals[i],
                                              gal_device->contiguousPhysicals[i]));
                } else {
                    gckOS_ReleaseReservedMemory(gal_device->os,
                                                gal_device->contiguousPhysicals[i]);

                    gal_device->contiguousPhysicals[i] = gcvNULL;
                    gal_device->requestedContiguousBases[i] = 0;
                    gal_device->requestedContiguousSizes[i] = 0;
                    gal_device->contiguousLogicals[i] = gcvNULL;
                    gal_device->contiguousPhysicals[i] = gcvNULL;
                }
            }

            if (gal_device->contiguousVidMems[i]) {
                /* Destroy the contiguous heap. */
                gcmkVERIFY_OK(gckVIDMEM_Destroy(gal_device->contiguousVidMems[i]));
                gal_device->contiguousVidMems[i] = gcvNULL;
            }
        }

        if (gal_device->lowContiguousPhysical != gcvNULL) {
            gckOS_ReleaseReservedMemory(gal_device->os,
                                        gal_device->lowContiguousPhysical);

            gal_device->lowContiguousPhysical = gcvNULL;
        }

        if (gal_device->lowContiguousVidMem != gcvNULL) {
            /* Destroy the contiguous heap. */
            gcmkVERIFY_OK(gckVIDMEM_Destroy(gal_device->lowContiguousVidMem));
            gal_device->lowContiguousVidMem = gcvNULL;
        }

#if gcdENABLE_TRUST_APPLICATION
        if (gal_device->taos) {
            gcmkVERIFY_OK(gctaOS_DestroyOS(gal_device->taos));
            gal_device->taos = gcvNULL;
        }
#endif

        /* Destroy the gckOS object. */
        if (gal_device->os != gcvNULL) {
            gcmkVERIFY_OK(gckOS_Destroy(gal_device->os));
            gal_device->os = gcvNULL;
        }

        /* Free the device. */
        kfree(gal_device);
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
free_gal_device:
    kfree(gal_device);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

#if gcdENABLE_PHYTIUM_DEVFREQ
#include <linux/devfreq.h>
#include <acpi/actypes.h>
struct ftg_devfreq {
        struct devfreq_dev_profile profile;
        struct devfreq_simple_ondemand_data ondemand_data;
};

unsigned long frequency = 0;
unsigned int freq_count = 0;
unsigned long freq_table[6] = {0};
unsigned int status_count = 0;

static int ftg_set_freq(struct device *dev, unsigned long freq)
{
	gckGALDEVICE gal_device = galDevice;
	gckKERNEL kernel = gal_device->devices[0]->kernels[0];
	gctUINT ClusterMask = 0;

       acpi_handle handle = ACPI_HANDLE(dev);
        union acpi_object args[4];
        struct acpi_object_list arg_list = {
                .pointer = args,
                .count = ARRAY_SIZE(args),
        };
        acpi_status status;
        unsigned long long data;

        args[0].type = ACPI_TYPE_INTEGER;
        args[0].integer.value = FT_DEVICE_TYPE;
        args[1].type = ACPI_TYPE_INTEGER;
        args[1].integer.value = freq;
        args[2].type = ACPI_TYPE_INTEGER;
        args[2].integer.value = 1;
        args[3].type = ACPI_TYPE_INTEGER;
        args[3].integer.value = 1;

	gckOS_AcquireMutex(kernel->os, kernel->device->commitMutex, gcvINFINITE);
	gckCOMMAND_Stall(kernel->command, gcvFALSE);
	gckCOMMAND_Stop(kernel->command);

        status = acpi_evaluate_integer(handle, "PSCF", &arg_list, &data);
        if (ACPI_FAILURE(status)) {
                pr_err("PSCF Method for shader failed %s\n", __func__);
                return -ENODEV;
        }

        if (data) {
                pr_err("%s Failed to set the freq to %lu\n", __func__, freq);
        }

        args[3].integer.value = 0;
        status = acpi_evaluate_integer(handle, "PSCF", &arg_list, &data);
        if (ACPI_FAILURE(status)) {
                pr_err("PSCF Method for Core AXI failed %s\n", __func__);
                return -ENODEV;
        }

        if (data) {
                pr_err("%s Failed to set the freq to %lu\n", __func__, freq);
        }

	if (freq == 200000) {
		ClusterMask = 0x1;
	} else if (freq == 400000) {
		ClusterMask = 0x3;
	} else if (freq == 600000) {
		ClusterMask = 0x7;
	} else {
		ClusterMask = 0xf;
	}

	gckOS_SetGPUPowerEx(kernel->hardware->os, kernel, gcvTRUE, gcvTRUE, ClusterMask);
	gckOS_AtomSet(kernel->hardware->os, kernel->hardware->curClusterMask, ClusterMask);

	gckHARDWARE_Reset(kernel->hardware, gcvTRUE);
	kernel->hardware->options.userClusterMasks[kernel->hardware->core] = ClusterMask;
	gckOS_ReleaseMutex(kernel->os, kernel->device->commitMutex);

	kernel->tmp_freq = freq;
        return 0;

}

static int ftg_get_cur_freq(struct device *dev, unsigned long *freq)
{
       gceSTATUS status = gcvSTATUS_OK;

       status = gpu_get_data_from_acpi(dev, "PGCF", FT_3D, FT_Shader, freq);

        return status;
}

static int ftg_dvf_status(struct device *dev, struct devfreq_dev_status *stat)
{

       gctUINT32 load = 0;
       gckHARDWARE Hardware = gcvNULL;
       gckDEVICE device = galDevice->devices[0];
       gceSTATUS status = gcvSTATUS_OK;
       gctUINT32 hi_total_cycle_count = 0, hi_total_idle_cycle_count = 0;

       gcmkHEADER();

       /*VIV: Query Core0 load to set all core clock. */
       Hardware = device->kernels[0]->hardware;

       gcmkONERROR(gckHARDWARE_QueryCycleCount(Hardware, &hi_total_cycle_count, &hi_total_idle_cycle_count));

       load = div_u64((gctUINT64)(hi_total_cycle_count - hi_total_idle_cycle_count) * 100, hi_total_cycle_count);
       stat->current_frequency = (unsigned long)frequency;
       stat->busy_time = (unsigned long)load;
       stat->total_time = (unsigned long)100;

       gcmkONERROR(gckHARDWARE_CleanCycleCount(Hardware));

OnError:
       gcmkFOOTER_NO();
       return status;
}

static int ftg_dvf_target(struct device *dev, unsigned long *freq, u32 flags)
{
        unsigned long old_freq = frequency;
        unsigned long target_freq;
        struct dev_pm_opp *opp;
        int ret;

        opp = devfreq_recommended_opp(dev, freq, flags);
        if (IS_ERR(opp)) {
                pr_err("%s %d failed\n",__func__, __LINE__);
                return PTR_ERR(opp);
        }

        target_freq = dev_pm_opp_get_freq(opp);
        dev_pm_opp_put(opp);

        if (target_freq == old_freq)
                return 0;

        ret = ftg_set_freq(dev, target_freq);
        if (ret) {
                pr_err("failed to set ftg frequency:%d\n", ret);
                *freq = old_freq;
        }

        *freq = target_freq;
        frequency = target_freq;
        return ret;
}
static int ftg_get_opp_table(struct device *dev)
{
        struct acpi_buffer buffer = {ACPI_ALLOCATE_BUFFER, NULL};
        union acpi_object args[3], *package, *element;
        struct acpi_object_list arg_list = {
                .pointer = args,
                .count = ARRAY_SIZE(args),
        };
        acpi_handle handle = ACPI_HANDLE(dev);
        acpi_status status;
        int i = 0;

        args[0].type = ACPI_TYPE_INTEGER;
        args[0].integer.value = FT_DEVICE_TYPE;
        args[1].type = ACPI_TYPE_INTEGER;
        args[1].integer.value = FT_3D;
        args[2].type = ACPI_TYPE_INTEGER;
        args[2].integer.value = FT_Shader;

        status = acpi_evaluate_object(handle, "PGCL", &arg_list, &buffer);
        if (ACPI_FAILURE(status)) {
                pr_err("No GCL Method\n");
                return -1;
        }

        package = buffer.pointer;

        element = &package->package.elements[1];
        freq_count = element->integer.value;

        for (i = 0; i < freq_count; i++) {
                element = &package->package.elements[i+2];
                freq_table[i] = element->integer.value;
        }

        return 0;
}

struct devfreq *devf;

gceSTATUS
_EnableDevfreq(gckGALDEVICE Device)
{
    struct ftg_devfreq *private;
    gcsPLATFORM* platform = galDevice->platform;
    struct device *ftg340_device= &(platform->device->dev);
    struct platform_device *pdev = platform->device;
    gceSTATUS status = gcvSTATUS_OK;
    int res, i;

    private = devm_kzalloc(ftg340_device, sizeof(*private), GFP_KERNEL);
    if (!private) {
        pr_err("private alloc failed\n");
       return -ENOMEM;
    }

    platform_set_drvdata(pdev, private);

    res = ftg_get_opp_table(ftg340_device);
    if(res) {
        pr_err("Failed to construct gpu opp table\n");
        return gcvSTATUS_NOT_SUPPORTED;
    }
    private->profile.initial_freq = freq_table[0];
    private->ondemand_data.upthreshold = 50;
    private->ondemand_data.downdifferential = 5;

    private->profile.polling_ms = 200;
    private->profile.target = ftg_dvf_target;
    private->profile.get_dev_status = ftg_dvf_status;
    private->profile.get_cur_freq = ftg_get_cur_freq;

    for (i = 0; i < freq_count; i++) {
        res = dev_pm_opp_add(ftg340_device, freq_table[i], 0);
        if (res < 0) {
                pr_err("%s failed to get OPP table\n", __func__);
                goto err;
        }
    }

    devf = devm_devfreq_add_device(ftg340_device, &private->profile, DEVFREQ_GOV_SIMPLE_ONDEMAND, &private->ondemand_data);
    if (IS_ERR(devf)) {
        gcmkPRINT("Error: init devfreq %lx\n", (unsigned long)ftg340_device);
        status = gcvSTATUS_NOT_SUPPORTED;
        pr_err("devfreq constrct failed\n");
    }

    pr_err("devfreq constrct successful\n");

    return status;

err:
    dev_pm_opp_of_remove_table(ftg340_device);
    return res;
}
#endif

#if gcdENABLE_DEVFREQ
#include <../drivers/devfreq/governor.h>
static gctUINT32 cur_freq = 64;
static int gc_df_target(struct device *dev, unsigned long *freq, u32 flags)
{
    gctUINT32 i = 0;
    gctUINT32 _freq = 1;
    gckHARDWARE hardware;
    gckDEVICE device = galDevice->devices[0];

    if (device->os->clockStates[0][0] == gcvFALSE)
        return 0;

    _freq = *freq;

    for (i = 0; i < gcvCORE_3D_MAX; i++) {
        if (device->kernels[i]) {
            hardware = device->kernels[i]->hardware;
            gckHARDWARE_SetClock(hardware, _freq, _freq);
        }
    }

    cur_freq = _freq;

    return 0;
}

static gceSTATUS gc_df_status(struct device *dev, struct devfreq_dev_status *stat)
{
    gckHARDWARE Hardware = gcvNULL;
    gctUINT32 load = 0;
    gckDEVICE device = galDevice->devices[0];
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 hi_total_cycle_count = 0, hi_total_idle_cycle_count = 0;

    gcmkHEADER();

    if (device->os->clockStates[0][0] == gcvFALSE)
        return gcvSTATUS_OK;

    Hardware = device->kernels[0]->hardware;

    gcmkONERROR(gckHARDWARE_QueryCycleCount(Hardware, &hi_total_cycle_count, &hi_total_idle_cycle_count));

    load = div_u64((gctUINT64)(hi_total_cycle_count - hi_total_idle_cycle_count) * 100, hi_total_cycle_count);

    stat->current_frequency = (unsigned long)cur_freq;
    stat->busy_time = (unsigned long)load;
    stat->total_time = (unsigned long)100;

    gcmkONERROR(gckHARDWARE_CleanCycleCount(Hardware));

OnError:
    gcmkFOOTER_NO();
    return status;
}

static int gc_df_get_cur_freq(struct device *dev, unsigned long *freq)
{
    *freq = cur_freq;
    return 0;
}

#define POLLING_MS        500
#define UP_THRESHOLD       90
#define DOWN_DIFFERENCTIAL 5

static struct devfreq *df;

static struct devfreq_simple_ondemand_data ftg340_gov_data;

static struct devfreq_dev_profile gc_df_profile = {
    .polling_ms = POLLING_MS,
    .target = gc_df_target,
    .get_dev_status = gc_df_status,
    .get_cur_freq = gc_df_get_cur_freq,
};

static gceSTATUS gc_df_governor_target(struct devfreq *df, unsigned long *freq)
{
    struct devfreq_dev_status *stat;
    unsigned long long a, b;

    gctUINT32 up_threshold = UP_THRESHOLD;
    gctUINT32 down_differential = DOWN_DIFFERENCTIAL;
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER();

    if (galDevice->os->clockStates[0][0] == gcvFALSE)
        return gcvSTATUS_OK;

    gcmkONERROR(devfreq_update_stats(df));

    stat = &df->last_status;

    /* Set MAX if it's busy enough */
    if (stat->busy_time * 100 >
        stat->total_time * up_threshold) {
        *freq = 64;
        goto OnError;
    }

    /* Set MAX if we do not know the initial frequency */
    if (stat->current_frequency == 0) {
        *freq = 64;
        goto OnError;
    }

    /* Keep the current frequency */
    if (stat->busy_time * 100 >
        stat->total_time * (up_threshold - down_differential)) {
        *freq = stat->current_frequency;
        goto OnError;
    }

    /* Set the desired frequency based on the load */
    a = stat->busy_time;
    a *= stat->current_frequency;
    b = div_u64(a, stat->total_time);
    b *= 100;
    b = div_u64(b, (up_threshold - down_differential / 2));

    if (b <= 1)
        *freq = 1;
    else if (b > 1 && b <= 2)
        *freq = 2;
    else if (b > 2 && b <= 4)
        *freq = 4;
    else if (b > 4 && b <= 8)
        *freq = 8;
    else if (b > 8 && b <= 16)
        *freq = 16;
    else if (b > 16 && b <= 32)
        *freq = 32;
    else if (b > 32 && b <= 48)
        *freq = 48;
    else
        *freq = 64;

OnError:
    gcmkFOOTER_NO();
    return status;
}

static int gc_df_governor_handler(struct devfreq *df, unsigned int event, void *data)
{
    switch (event) {
    case DEVFREQ_GOV_START:
        devfreq_monitor_start(df);
        break;

    case DEVFREQ_GOV_STOP:
        devfreq_monitor_stop(df);
        break;

    default:
        break;
    }

    return 0;
}

static struct devfreq_governor gc_df_governor = {
    .name = "ftg340_gov",
    .get_target_freq = gc_df_governor_target,
    .event_handler = gc_df_governor_handler,
};

static gceSTATUS
gc_df_init(gckGALDEVICE galDevice)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 err = 0;
    gctUINT32 i = 0;
    gckHARDWARE Hardware;
    struct device *ftg340_device = galDevice->devices[0]->dev;
    gckDEVICE device = galDevice->devices[0];

    gcmkHEADER();

    dev_pm_opp_add(ftg340_device, 1, 0);
    dev_pm_opp_add(ftg340_device, 2, 0);
    dev_pm_opp_add(ftg340_device, 4, 0);
    dev_pm_opp_add(ftg340_device, 8, 0);
    dev_pm_opp_add(ftg340_device, 16, 0);
    dev_pm_opp_add(ftg340_device, 32, 0);
    dev_pm_opp_add(ftg340_device, 48, 0);
    dev_pm_opp_add(ftg340_device, 64, 0);

    err = devfreq_add_governor(&gc_df_governor);

    if (err) {
        gcmkPRINT("Failed to add governor: %d", err);
        return gcvSTATUS_NOT_SUPPORTED;
    }

    df = devm_devfreq_add_device(ftg340_device, &gc_df_profile,
                                 "ftg340_gov", &ftg340_gov_data);
    if (IS_ERR(df)) {
        gcmkPRINT("Error: init devfreq %lx", (unsigned long)ftg340_device);
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    for (i = 0; i < gcvCORE_3D_MAX; i++) {
        if (device->kernels[i]) {
            Hardware = device->kernels[i]->hardware;

            if (Hardware->options.powerManagement) {
                gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

                gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

                Hardware->options.powerManagement = gcvFALSE;
            }

            gcmkONERROR(gckHARDWARE_SetGpuProfiler(Hardware, gcvTRUE));

            gcmkONERROR(gckHARDWARE_InitProfiler(Hardware));

            Hardware->waitCount = 200 * 100;

            gcmkONERROR(gckHARDWARE_CleanCycleCount(Hardware));
        }
    }

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
gc_df_exit(gckGALDEVICE Device)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT32 err = 0;
    struct device *ftg340_device = Device->devices[0]->dev;

    devm_devfreq_remove_device(ftg340_device, df);

    err = devfreq_remove_governor(&gc_df_governor);

    if (err) {
        gcmkPRINT("%s %d: failed remove governor %d", __FUNCTION__, __LINE__,  err);
        status = gcvSTATUS_NOT_SUPPORTED;
    }

    return status;
}
#endif

/*******************************************************************************
 *
 *  gckGALDEVICE_Start
 *
 *  Start the gal device, including the following actions: setup the isr routine
 *  and start the daemoni thread.
 *
 *  INPUT:
 *
 *      gckGALDEVICE Device
 *          Pointer to an gckGALDEVICE object.
 *
 *  OUTPUT:
 *
 *      Nothing.
 *
 *  RETURNS:
 *
 *      gcvSTATUS_OK
 *          Start successfully.
 */
gceSTATUS
gckGALDEVICE_Start(gckGALDEVICE Device)
{
    gctUINT i, devIndex;
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device = gcvNULL;
    gckKERNEL kernel = gcvNULL;

    gcmkHEADER_ARG("Device=%p", Device);

    for (devIndex = 0; devIndex < Device->args.devCount; devIndex++) {
        device = Device->devices[devIndex];

        /* Start the kernel threads. */
        for (i = 0; i < gcvCORE_COUNT; ++i) {

            gcmkONERROR(_StartThread(device, i));

#if gcdENABLE_SW_PREEMPTION
            gcmkONERROR(_StartPreemptThread(device, i));
#endif
        }

        /* Power off needs to be after all the command start. */
        for (i = 0; i < gcvCORE_COUNT; i++) {
            kernel = device->kernels[i];

            if (!kernel)
                continue;

            if (!i && kernel->processPageTable &&
                (kernel->command->pool == gcvPOOL_VIRTUAL ||
                 !kernel->flatMapping))
                gcmkONERROR(gckMMU_ConstructMmuCopy(kernel, &kernel->mmuCopy));

            /* Setup the ISR routine. */
            gcmkONERROR(_SetupIsr(device, i));

            /* Switch to SUSPEND power state. */
            gcmkONERROR(gckHARDWARE_SetPowerState(kernel->hardware,
                                                      gcvPOWER_OFF_BROADCAST));
        }
    }

#if gcdENABLE_PHYTIUM_DEVFREQ
    gcmkONERROR(_EnableDevfreq(Device));
#endif

#if gcdENABLE_DEVFREQ
    gcmkONERROR(gc_df_init(Device));
#endif

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 *
 *  gckGALDEVICE_Stop
 *
 *  Stop the gal device, including the following actions: stop the daemon
 *  thread, release the irq.
 *
 *  INPUT:
 *
 *      gckGALDEVICE Device
 *          Pointer to an gckGALDEVICE object.
 *
 *  OUTPUT:
 *
 *      Nothing.
 *
 *  RETURNS:
 *
 *      Nothing.
 */
gceSTATUS
gckGALDEVICE_Stop(gckGALDEVICE Device)
{
    gctUINT i, devIndex;
    gceSTATUS status = gcvSTATUS_OK;
    gckDEVICE device = gcvNULL;
    gckKERNEL kernel = gcvNULL;
#if gcdENABLE_PHYTIUM_DEVFREQ
    struct device *ftg340_device = Device->devices[0]->dev;
#endif

    gcmkHEADER_ARG("Device=%p", Device);

    gcmkVERIFY_ARGUMENT(Device != NULL);
    
    #if gcdENABLE_PHYTIUM_DEVFREQ
        devm_devfreq_remove_device(ftg340_device, devf);
    #endif

    for (devIndex = 0; devIndex < Device->args.devCount; devIndex++) {
        device = Device->devices[devIndex];

        for (i = 0; i < gcvCORE_COUNT; i++) {
            kernel = device->kernels[i];
            if (!kernel)
                continue;
	    Device->platform->params.userClusterMasks[i] = kernel->hardware->options.userClusterMasks[kernel->hardware->core];
	    if (Device->platform->params.userClusterMasks[i]) {
            	if (!Device->gotoShutdown) {
                	gcmkONERROR(gckHARDWARE_EnablePowerManagement(kernel->hardware,
                                                              	gcvTRUE));

                	/* Switch to OFF power state. */
                	gcmkONERROR(gckHARDWARE_SetPowerState(kernel->hardware,
                                                          	gcvPOWER_OFF_EXIT));
            	}
	    }

            if (!i && kernel->processPageTable &&
                (kernel->command->pool == gcvPOOL_VIRTUAL ||
                 !kernel->flatMapping))
                gcmkONERROR(gckMMU_DestroyMmuCopy(kernel->mmuCopy));

            /* Stop the ISR routine. */
            gcmkONERROR(_ReleaseIsr(device, i));

            _StopThread(device, i);
#if gcdENABLE_SW_PREEMPTION
            _StopPreemptThread(device, i);
#endif
        }
    }

#if gcdENABLE_DEVFREQ
    gcmkONERROR(gc_df_exit(Device));
#endif

OnError:
    gcmkFOOTER();
    return status;
}
