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


#ifndef __gc_hal_kernel_device_h_
#define __gc_hal_kernel_device_h_

#include "gc_hal_kernel_debugfs.h"

#if gcdENABLE_TRUST_APPLICATION
#include "gc_hal_ta.h"

extern gcTA             globalTA[gcvCORE_COUNT];
#endif

/*******************************************************************************
 ************************* gckGALDEVICE Structure ******************************
 *******************************************************************************/

typedef struct _gckGALDEVICE {
    /* Objects. */
    gckOS               os;

    gcsPLATFORM         *platform;

    /* Attributes. */
    gctPHYS_ADDR_T      internalBase;
    gctSIZE_T           internalSize;
    gctPHYS_ADDR        internalPhysical;
    gctUINT32           internalPhysName;
    gctPOINTER          internalLogical;
    gckVIDMEM           internalVidMem;

    gctPHYS_ADDR_T      externalBase[gcdDEVICE_COUNT];
    gctSIZE_T           externalSize[gcdDEVICE_COUNT];
    gctPHYS_ADDR        externalPhysical[gcdDEVICE_COUNT];
    gctUINT32           externalPhysName[gcdDEVICE_COUNT];
    gckVIDMEM           externalVidMem[gcdDEVICE_COUNT];

    /* Shared external SRAMs. */
    gctPHYS_ADDR_T      extSRAMBases[gcvSRAM_EXT_COUNT];
    gctSIZE_T           extSRAMSizes[gcvSRAM_EXT_COUNT];
    gctPHYS_ADDR        extSRAMPhysicals[gcvSRAM_EXT_COUNT];
    gckVIDMEM           extSRAMVidMems[gcvSRAM_EXT_COUNT];

    gctPHYS_ADDR_T      contiguousBases[gcdSYSTEM_RESERVE_COUNT];
    gctSIZE_T           contiguousSizes[gcdSYSTEM_RESERVE_COUNT];
    gctPHYS_ADDR        contiguousPhysicals[gcdSYSTEM_RESERVE_COUNT];
    gctUINT32           contiguousPhysNames[gcdSYSTEM_RESERVE_COUNT];
    gctPOINTER          contiguousLogicals[gcdSYSTEM_RESERVE_COUNT];
    gckVIDMEM           contiguousVidMems[gcdSYSTEM_RESERVE_COUNT];

    gctPHYS_ADDR_T      requestedContiguousBases[gcdSYSTEM_RESERVE_COUNT];
    gctSIZE_T           requestedContiguousSizes[gcdSYSTEM_RESERVE_COUNT];


    gctPHYS_ADDR_T      lowContiguousBase;
    gctSIZE_T           lowContiguousSize;
    gctPHYS_ADDR        lowContiguousPhysical;
    gctUINT32           lowContiguousPhysName;
    gckVIDMEM           lowContiguousVidMem;

    gctPHYS_ADDR_T      exclusiveBase[gcdDEVICE_COUNT];
    gctSIZE_T           exclusiveSize[gcdDEVICE_COUNT];
    gctPHYS_ADDR        exclusivePhysical[gcdDEVICE_COUNT];
    gctUINT32           exclusivePhysName[gcdDEVICE_COUNT];
    gctPOINTER          exclusiveLogical[gcdDEVICE_COUNT];
    gckVIDMEM           exclusiveVidMem[gcdDEVICE_COUNT];

    gcsDEBUGFS_DIR      debugfsDir;

    gckDEVICE           devices[gcdDEVICE_COUNT];
    gctINT32            devIDs[gcdDEVICE_COUNT];
    gctUINT             devCount;

    gcsMODULE_PARAMETERS args;


    /* If we are going to shut down. */
    gctBOOL             gotoShutdown;
	
#if gcdENABLE_TRUST_APPLICATION
	/* gctsOs object for trust application. */
	gctaOS				taos;
#endif

#if gcdENABLE_DRM
    void                *drm;
#endif
} *gckGALDEVICE;

typedef struct _gcsHAL_PRIVATE_DATA {
    gckGALDEVICE        device;
    /*
     * 'fput' schedules actual work in '__fput' in a different thread.
     * So the process opens the device may not be the same as the one that
     * closes it.
     */
    gctUINT32           pidOpen;
    gctBOOL             isLocked;
    gctUINT32           devIndex;
} gcsHAL_PRIVATE_DATA, *gcsHAL_PRIVATE_DATA_PTR;

gceSTATUS
gckGALDEVICE_Start(gckGALDEVICE Device);

gceSTATUS
gckGALDEVICE_Stop(gckGALDEVICE Device);

gceSTATUS
gckGALDEVICE_Construct(gcsPLATFORM *Platform, const gcsMODULE_PARAMETERS *Args, gckGALDEVICE *Device);

gceSTATUS
gckGALDEVICE_Destroy(gckGALDEVICE Device);

static gcmkINLINE gckKERNEL
_GetValidKernel(gckGALDEVICE Device)
{
    gckDEVICE device = Device->devices[0];

    if (device->kernels[gcvCORE_MAJOR]) {
        return device->kernels[gcvCORE_MAJOR];
   } else {
        gcmkASSERT(gcvFALSE);
        return gcvNULL;
    }
}

#endif /* __gc_hal_kernel_device_h_ */
