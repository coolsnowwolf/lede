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

#define _GC_OBJ_ZONE gcvZONE_KERNEL

#if gcdENABLE_TRUST_APPLICATION

/*
 * Open a security service channel.
 */
gceSTATUS
gckKERNEL_SecurityOpen(gckKERNEL Kernel, gctUINT32 GPU, gctUINT32 *Channel)
{
    gceSTATUS status;

    gcmkONERROR(gckOS_OpenSecurityChannel(Kernel->os, Kernel->core, Channel));
    gcmkONERROR(gckOS_InitSecurityChannel(*Channel));

    return gcvSTATUS_OK;

OnError:
    return status;
}

/*
 * Close a security service channel
 */
gceSTATUS
gckKERNEL_SecurityClose(gctUINT32 Channel)
{
    return gcvSTATUS_OK;
}

/*
 * Security service interface.
 */
gceSTATUS
gckKERNEL_SecurityCallService(gctUINT32 Channel, gcsTA_INTERFACE *Interface)
{
    gceSTATUS status;

    gcmkHEADER();

    gcmkVERIFY_ARGUMENT(Interface != gcvNULL);

    gckOS_CallSecurityService(Channel, Interface);

    status = Interface->result;

    gcmkONERROR(status);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_SecurityStartCommand(gckKERNEL Kernel, gctADDRESS Address, gctUINT32 Bytes)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_START_COMMAND;
    iface.u.StartCommand.gpu = Kernel->core;
    iface.u.StartCommand.address = Address;
    iface.u.StartCommand.bytes = Bytes;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_SecurityAllocateSecurityMemory(gckKERNEL Kernel, gctUINT32 Bytes, gctUINT32 *Handle)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_ALLOCATE_SECRUE_MEMORY;
    iface.u.AllocateSecurityMemory.bytes = Bytes;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    *Handle = iface.u.AllocateSecurityMemory.memory_handle;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_SecurityMapMemory(gckKERNEL Kernel, gctUINT32 *PhysicalArray,
                            gctPHYS_ADDR_T Physical, gctUINT32 PageCount, gctADDRESS *GPUAddress)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_MAP_MEMORY;

    iface.u.MapMemory.physicals = PhysicalArray;
    iface.u.MapMemory.physical = Physical;
    iface.u.MapMemory.pageCount = PageCount;
    iface.u.MapMemory.gpuAddress = *GPUAddress;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_SecurityDumpMMUException(gckKERNEL Kernel)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_DUMP_MMU_EXCEPTION;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_SecurityUnmapMemory(gckKERNEL Kernel, gctADDRESS GPUAddress, gctUINT32 PageCount)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_UNMAP_MEMORY;

    iface.u.UnmapMemory.gpuAddress = GPUAddress;
    iface.u.UnmapMemory.pageCount = PageCount;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_ReadMMUException(gckKERNEL Kernel, gctUINT32_PTR MMUStatus, gctUINT32_PTR MMUException)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_READ_MMU_EXCEPTION;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    *MMUStatus = iface.u.ReadMMUException.mmuStatus;
    *MMUException = iface.u.ReadMMUException.mmuException;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckKERNEL_HandleMMUException(gckKERNEL Kernel, gctUINT32 MMUStatus,
                             gctPHYS_ADDR_T Physical, gctADDRESS GPUAddress)
{
    gceSTATUS status;
    gcsTA_INTERFACE iface;

    gcmkHEADER();

    iface.command = KERNEL_HANDLE_MMU_EXCEPTION;

    iface.u.HandleMMUException.mmuStatus = MMUStatus;
    iface.u.HandleMMUException.physical = Physical;
    iface.u.HandleMMUException.gpuAddress = GPUAddress;

    gcmkONERROR(gckKERNEL_SecurityCallService(Kernel->securityChannel, &iface));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

#endif
