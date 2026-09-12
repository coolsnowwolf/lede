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


#include "gc_hal_types.h"
#include "gc_hal_base.h"
#include "gc_hal_security_interface.h"
#include "gc_hal_ta.h"
#include "gc_hal_ta_hardware.h"
#include "gc_hal.h"
#include "gc_feature_database.h"

#include "AQ.h"

#define _GC_OBJ_ZONE     1
#define SRC_MAX          8
#define RECT_ADDR_OFFSET 3

#define INVALID_ADDRESS  ~0U

/******************************************************************************\
********************************* Support Code *********************************
\******************************************************************************/
#ifdef VSIMULATOR_DEBUG
gctBOOL isVSimulatorRunInNBGMode();
#endif
static gceSTATUS
_IdentifyHardwareByDatabase(
    IN gcTA_HARDWARE Hardware
    )
{
    gceSTATUS status;
    gctUINT32 chipIdentity = 0;
    gcsFEATURE_DATABASE *database;
    gctaOS os = Hardware->os;

    gcmkHEADER();

    /***************************************************************************
    ** Get chip ID and revision.
    */

    /* Read chip identity register. */
    gcmkONERROR(gctaOS_ReadRegister(os, Hardware->ta->core, AQ_IDENT_Address, &chipIdentity));

    /* Special case for older graphic cores. */
    if (gcmVERIFYFIELDVALUE(chipIdentity, AQ_IDENT, FAMILY, GC500))
    {
        Hardware->chipModel    = gcv500;
        Hardware->chipRevision = gcmGETFIELD(chipIdentity, AQ_IDENT, REVISION);
    }

    else
    {
        /* Read chip identity register. */
        gcmkONERROR(
            gctaOS_ReadRegister(os, Hardware->ta->core,
                                 GC_CHIP_ID_Address,
                                 (gctUINT32_PTR) &Hardware->chipModel));

        if (((Hardware->chipModel & 0xFF00) == 0x0400)
          && (Hardware->chipModel != 0x0420)
          && (Hardware->chipModel != 0x0428))
        {
            Hardware->chipModel = (gceCHIPMODEL) (Hardware->chipModel & 0x0400);
        }

        /* Read CHIP_REV register. */
        gcmkONERROR(
            gctaOS_ReadRegister(os, Hardware->ta->core,
                                 GC_CHIP_REV_Address,
                                 &Hardware->chipRevision));

        if ((Hardware->chipModel    == gcv300)
        &&  (Hardware->chipRevision == 0x2201)
        )
        {
            gctUINT32 chipDate = 0;
            gctUINT32 chipTime = 0;

            /* Read date and time registers. */
            gcmkONERROR(
                gctaOS_ReadRegister(os, Hardware->ta->core,
                                     GC_CHIP_DATE_Address,
                                     &chipDate));

            gcmkONERROR(
                gctaOS_ReadRegister(os, Hardware->ta->core,
                                     GC_CHIP_TIME_Address,
                                     &chipTime));

            if ((chipDate == 0x20080814) && (chipTime == 0x12051100))
            {
                /* This IP has an ECO; put the correct revision in it. */
                Hardware->chipRevision = 0x1051;
            }
        }

        gcmkONERROR(
            gctaOS_ReadRegister(os, Hardware->ta->core,
                                 GC_PRODUCT_ID_Address,
                                 &Hardware->productID));
    }

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        os, Hardware->ta->core,
        GC_ECO_ID_Address
        ,
        &Hardware->ecoID
        ));

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        os, Hardware->ta->core,
        GC_CHIP_CUSTOMER_Address
        ,
        &Hardware->customerID
        ));

    /***************************************************************************
    ** Get chip features.
    */

    database =
    Hardware->featureDatabase =
    gcQueryFeatureDB(
        Hardware->chipModel,
        Hardware->chipRevision,
        Hardware->productID,
        Hardware->ecoID,
        Hardware->customerID
        );

    if (database == gcvNULL)
    {
        gcmkPRINT("[ftg340]: Feature database is not found,"
                  "chipModel=0x%0x, chipRevision=0x%x, productID=0x%x, ecoID=0x%x, customerID=0x%x",
                  Hardware->chipModel,
                  Hardware->chipRevision,
                  Hardware->productID,
                  Hardware->ecoID,
                  Hardware->customerID);
        gcmkONERROR(gcvSTATUS_NOT_FOUND);
    }
#ifdef VSIMULATOR_DEBUG  
    /* vsimulator don't support SECURITY feature */
    database->SECURITY = 0;
    database->SECURITY_AHB = 0;
    database->TP_LRN = 0;
    database->NN_ZDP3 = 0;
    database->NN_ZDP6 = 0;
    database->NN_INTERLEVE8 = 0;
    database->TP_REORDER = 0;
    database->TP_ROI_POOLING = 0;
    database->TP_MAX_POOLING_STRIDE1 = 0;
    database->MCFE = (isVSimulatorRunInNBGMode() && Hardware->chipRevision == 0x7200 && Hardware->customerID == 0x85) ? 1 : 0;
    database->NN_XYDP9 = 0;
    database->NN_XYDP6 = 0;
#endif
    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}


gceSTATUS
gctaHARDWARE_SetMMUStates(
    IN gcTA_HARDWARE Hardware,
    IN gctPOINTER MtlbAddress,
    IN gceMMU_MODE Mode,
    IN gctPOINTER SafeAddress,
    IN gctPOINTER Logical,
    IN OUT gctUINT32 * Bytes
    )
{
    gceSTATUS status;
    gctUINT32 config;
    gctUINT32 extMtlb;
    gctPHYS_ADDR_T physical;
    gctUINT32_PTR buffer;
    gctUINT32 reserveBytes = 2 * 4;
    gcsMMU_TABLE_ARRAY_ENTRY * entry;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    entry = (gcsMMU_TABLE_ARRAY_ENTRY *) Hardware->pagetableArray.logical;

    /* Convert logical address into physical address. */
    gcmkONERROR(
        gctaOS_GetPhysicalAddress(Hardware->os, MtlbAddress, &physical));

    config  = (gctUINT32)(physical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(physical >> 32);
    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
    {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    switch (Mode)
    {
    case gcvMMU_MODE_1K:
        if (config & 0x3FF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE1_K);

        break;

    case gcvMMU_MODE_4K:
        if (config & 0xFFF)
        {
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);
        }

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE4_K);

        break;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (Logical != gcvNULL)
    {
        buffer = Logical;

        /* Setup page table array entry. */
        entry->low = config;
        entry->high = extMtlb;

        /* Setup command buffer to load index 0 of page table array. */
        *buffer++
            = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE)
            | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigRegAddrs)
            | gcmSETFIELD     (0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *buffer++
            = gcmSETMASKEDFIELD(GCREG_MMU_CONFIG, PAGE_TABLE_ID, 0);
    }

    if (Bytes != gcvNULL)
    {
        *Bytes = reserveBytes;
    }

    /* Return the status. */
    gcmkFOOTER_NO();
    return status;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gctaHARDWARE_End(
    IN gcTA_HARDWARE Hardware,
    IN gctPOINTER Logical,
    IN OUT gctUINT32 * Bytes
    )
{
    gctUINT32_PTR logical = (gctUINT32_PTR) Logical;
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=%p Logical=%p *Bytes=%lu",
        Hardware, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    if (Logical != gcvNULL)
    {
        if (*Bytes < 8)
        {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        /* Append END. */
        logical[0] =
            gcmSETFIELDVALUE(0, AQ_COMMAND_END_COMMAND, OPCODE, END);

        /* Record the count of execution which is finised by this END. */
        logical[1] =
            0;

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "%p: END", Logical);
    }

    if (Bytes != gcvNULL)
    {
        /* Return number of bytes required by the END command. */
        *Bytes = 8;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=%lu", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}


gceSTATUS
gctaHARDWARE_Construct(
    IN gcTA TA,
    OUT gcTA_HARDWARE * Hardware
    )
{
    gceSTATUS status;
    gcTA_HARDWARE hardware = gcvNULL;

    gctaOS os = TA->os;

    gcmkONERROR(gctaOS_Allocate(
        gcmSIZEOF(gcsTA_HARDWARE),
        (gctPOINTER *)&hardware
        ));

    gctaOS_ZeroMemory((gctUINT8_PTR)hardware, gcmSIZEOF(gcsTA_HARDWARE));

    hardware->ta = TA;
    hardware->os = os;

    hardware->pagetableArray.size = 4096;

    hardware->functionBytes = 4096;

    /* Power on GPU. */
    gctaOS_SetGPUPower(os, TA->core, gcvTRUE, gcvTRUE);

    /*************************************/
    /********  Get chip information ******/
    /*************************************/
    gctaOS_WriteRegister(
        hardware->ta->os, hardware->ta->core,
        AQ_HI_CLOCK_CONTROL_Address,
        AQ_HI_CLOCK_CONTROL_ResetValue
        );

    gcmkONERROR(_IdentifyHardwareByDatabase(hardware));

    *Hardware = hardware;

    return gcvSTATUS_OK;

OnError:
    if (hardware)
    {
        gctaOS_Free(hardware);
    }

    return status;
}

gceSTATUS
gctaHARDWARE_Destroy(
    IN gcTA_HARDWARE Hardware
    )
{
    if (Hardware->pagetableArray.logical)
    {
        gctaOS_FreeSecurityMemory(
            Hardware->ta->os,
            Hardware->pagetableArray.size,
            Hardware->pagetableArray.logical,
            (gctUINT32_PTR)Hardware->pagetableArray.physical
            );
    }

    if (Hardware->functionLogical)
    {
        gctaOS_FreeSecurityMemory(
            Hardware->ta->os,
            Hardware->functionBytes,
            Hardware->functionLogical,
            (gctUINT32_PTR)Hardware->functionPhysical
            );
    }

    gctaOS_Free(Hardware);

    return gcvSTATUS_OK;
}

gceSTATUS
gctaHARDWARE_Execute(
    IN gcTA TA,
    IN gctUINT32 Address,
    IN gctUINT32 Bytes
    )
{
    gceSTATUS status;
    gctUINT32 address = Address, control;

    gcmkHEADER_ARG("Address=0x%x Bytes=%lu",
        Address, Bytes);

    /* Enable all events. */
    gcmkONERROR(
        gctaOS_WriteRegister(TA->os, TA->core, AQ_INTR_ENBL_Address, ~0U));

    /* Write address register. */
    gcmkONERROR(
        gctaOS_WriteRegister(TA->os, TA->core, AQ_CMD_BUFFER_ADDR_Address, address));

    /* Build control register. */
    control = gcmSETFIELDVALUE(0,
        AQ_CMD_BUFFER_CTRL,
        ENABLE,
        ENABLE)
        | gcmSETFIELD(0,
        AQ_CMD_BUFFER_CTRL,
        PREFETCH,
        (Bytes + 7) >> 3);

    /* Write control register. */
    gcmkONERROR(
        gctaOS_WriteRegister(TA->os, TA->core, GCREG_CMD_BUFFER_AHB_CTRL_Address, control));

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
        "Started command buffer @ 0x%08x",
        address);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gctaHARDWARE_MmuEnable(
    IN gcTA_HARDWARE Hardware
    )
{
    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMU_CONTROL_Address,
        gcmSETFIELD(0, GCREG_MMU_CONTROL, ENABLE, 1
        ));

    return gcvSTATUS_OK;
}

/*
* In trust zone, we prepare page table array table and configure base address of
* it to hardware.
*/
gceSTATUS
gctaHARDWARE_SetMMU(
    IN gcTA_HARDWARE Hardware,
    IN gctPOINTER Logical
    )
{
    gcsMMU_TABLE_ARRAY_ENTRY *entry;
    gcsHARDWARE_FUNCTION *function = &Hardware->functions[0];
    gctUINT32 delay = 1;
    gctUINT32 timer = 0;
    gctUINT32 idle = 0;
    gctPHYS_ADDR_T mtlbPhysical;
    gctPHYS_ADDR_T secureSafeAddress;
    gctPHYS_ADDR_T nonSecureSafeAddress;

    gctaOS_GetPhysicalAddress(Hardware->ta->os, Logical, &mtlbPhysical);

    gctaOS_GetPhysicalAddress(Hardware->ta->os, Hardware->ta->mmu->safePageLogical, &secureSafeAddress);

    gctaOS_GetPhysicalAddress(Hardware->ta->os, Hardware->ta->mmu->nonSecureSafePageLogical, &nonSecureSafeAddress);

    /* not support more than 40bit physical address */
    if ((secureSafeAddress & 0xFFFFFF0000000000ULL) ||
        (nonSecureSafeAddress & 0xFFFFFF0000000000ULL))
    {
        return (gcvSTATUS_NOT_SUPPORTED);
    }

    /* Fill entry 0 of page table array. */
    entry = (gcsMMU_TABLE_ARRAY_ENTRY *)Hardware->pagetableArray.logical;

    entry->low  = (gctUINT32)(mtlbPhysical & 0xFFFFFFFF);

    entry->high = (gctUINT32)(mtlbPhysical >> 32)
                | gcmSETFIELD(0, GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_HIGH, MASTER_TLB_SECURE, 1)
                ;

    /* Set page table base. */
    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_LOW_Address,
        (gctUINT32)(Hardware->pagetableArray.address & 0xFFFFFFFF)
        );

    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_TABLE_ARRAY_BASE_ADDRESS_HIGH_Address,
        (gctUINT32)((Hardware->pagetableArray.address >> 32) & 0xFFFFFFFF)
        );

    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_TABLE_ARRAY_SIZE_Address
        ,
        1
        );

    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_SAFE_SECURE_ADDRESS_Address,
        (gctUINT32)(secureSafeAddress & 0xFFFFFFFF)
        );

    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_SAFE_NON_SECURE_ADDRESS_Address,
        (gctUINT32)(nonSecureSafeAddress & 0xFFFFFFFF)
        );

    gctaOS_WriteRegister(
        Hardware->ta->os, Hardware->ta->core,
        GCREG_MMUAHB_SAFE_ADDRESS_EXT_Address,
        gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_SECURE, (gctUINT32)((secureSafeAddress >> 32) & 0xFFFFFFFF))
      | gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_SECURE, ENABLED)
      | gcmSETFIELD(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, SAFE_ADDRESS_NON_SECURE, (gctUINT32)((secureSafeAddress >> 32) & 0xFFFFFFFF))
      | gcmSETFIELDVALUE(0, GCREG_MMUAHB_SAFE_ADDRESS_EXT, MASK_SAFE_ADDRESS_NON_SECURE, ENABLED)
        );

    /* Execute prepared command sequence. */
    gctaHARDWARE_Execute(
        Hardware->ta,
        function->address,
        function->bytes
        );

    /* Wait until MMU configure finishes. */
    do
    {
        gctaOS_Delay(Hardware->os, delay);

        gctaOS_ReadRegister(
            Hardware->ta->os, Hardware->ta->core,
            AQ_HI_IDLE_Address,
            &idle);

        timer += delay;
        delay *= 2;
    }
    while (!gcmGETFIELD(idle, AQ_HI_IDLE, IDLE_FE));

    /* Enable MMU. */
    gctaOS_WriteRegister(
        Hardware->os, Hardware->ta->core,
        GCREG_MMUAHB_CONTROL_Address,
        gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTROL, MMU, ENABLE)
        );

    return gcvSTATUS_OK;
}

gceSTATUS
gctaHARDWARE_PrepareFunctions(
    IN gcTA_HARDWARE Hardware
    )
{
    gceSTATUS status;
    gcsHARDWARE_FUNCTION * function;
    gctUINT32 mmuBytes;
    gctUINT32 endBytes = 8;
    gctUINT8_PTR logical;
    gctPHYS_ADDR_T physical;

    gcmkHEADER();

    /* Allocate page table array. */
    gcmkONERROR(gctaOS_AllocateSecurityMemory(
        Hardware->ta->os,
        &Hardware->pagetableArray.size,
        &Hardware->pagetableArray.logical,
        &Hardware->pagetableArray.physical
        ));

    gcmkONERROR(gctaOS_GetPhysicalAddress(
        Hardware->ta->os,
        Hardware->pagetableArray.logical,
        &Hardware->pagetableArray.address
        ));

    /* Allocate GPU functions. */
    gcmkONERROR(gctaOS_AllocateSecurityMemory(
        Hardware->ta->os,
        &Hardware->functionBytes,
        &Hardware->functionLogical,
        &Hardware->functionPhysical
        ));

    gcmkONERROR(gctaOS_GetPhysicalAddress(
        Hardware->ta->os,
        Hardware->functionLogical,
        &physical
        ));

    gcmkSAFECASTPHYSADDRT(Hardware->functionAddress, physical);

    function = &Hardware->functions[0];

    function->logical = Hardware->functionLogical;

    function->address = Hardware->functionAddress;

    logical = function->logical;

    gcmkONERROR(gctaHARDWARE_SetMMUStates(
        Hardware,
        Hardware->ta->mmu->mtlbLogical,
        gcvMMU_MODE_4K,
        Hardware->ta->mmu->safePageLogical,
        logical,
        &mmuBytes
        ));

    logical += 8;

    gcmkONERROR(gctaHARDWARE_End(
        Hardware,
        logical,
        &endBytes
        ));

    function->bytes = mmuBytes + endBytes;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gctaHARDWARE_IsFeatureAvailable(
    IN gcTA_HARDWARE Hardware,
    IN gceFEATURE Feature
    )
{
    gctBOOL available;
    gcsFEATURE_DATABASE *database = Hardware->featureDatabase;

    switch (Feature)
    {
    case gcvFEATURE_SECURITY:
        available = database->SECURITY;
        break;
    default:
        gcmkFATAL("Invalid feature has been requested.");
        available = gcvFALSE;
    }

    return available;
}

gceSTATUS
gctaHARDWARE_DumpMMUException(
    IN gcTA_HARDWARE Hardware
    )
{
    gctUINT32 mmu       = 0;
    gctUINT32 mmuStatus = 0;
    gctUINT32 address   = 0;
    gctUINT32 i         = 0;

    gctUINT32 mmuStatusRegAddress;
    gctUINT32 mmuExceptionAddress;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);

    mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
    mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkPRINT("ChipModel=0x%x ChipRevision=0x%x:\n",
              Hardware->chipModel,
              Hardware->chipRevision);

    gcmkPRINT("**************************\n");
    gcmkPRINT("***   MMU ERROR DUMP   ***\n");
    gcmkPRINT("**************************\n");

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        Hardware->os, Hardware->ta->core,
        mmuStatusRegAddress
        ,
        &mmuStatus
        ));

    gcmkPRINT("  MMU status = 0x%08X\n", mmuStatus);

    for (i = 0; i < 4; i += 1)
    {
        mmu = mmuStatus & 0xF;
        mmuStatus >>= 4;

        if (mmu == 0)
        {
            continue;
        }

        switch (mmu)
        {
        case 1:
              gcmkPRINT("  MMU%d: slave not present\n", i);
              break;

        case 2:
              gcmkPRINT("  MMU%d: page not present\n", i);
              break;

        case 3:
              gcmkPRINT("  MMU%d: write violation\n", i);
              break;

        case 4:
              gcmkPRINT("  MMU%d: out of bound", i);
              break;

        case 5:
              gcmkPRINT("  MMU%d: read security violation", i);
              break;

        case 6:
              gcmkPRINT("  MMU%d: write security violation", i);
              break;

        default:
              gcmkPRINT("  MMU%d: unknown state\n", i);
        }

        gcmkVERIFY_OK(gctaOS_ReadRegister(
            Hardware->os, Hardware->ta->core,
            mmuExceptionAddress + i * 4
            ,
            &address
            ));

        gcmkPRINT("  MMU%d: exception address = 0x%08X\n", i, address);

        gctaMMU_DumpPagetableEntry(Hardware->ta->mmu, address);
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gctaHARDWARE_ReadMMUException(
    IN gcTA_HARDWARE Hardware,
    OUT gctUINT32_PTR MMUStatus,
    OUT gctUINT32_PTR MMUException
    )
{
    gctUINT32 mmuStatusRegAddress;
    gctUINT32 mmuExceptionAddress;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
    mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        Hardware->os, Hardware->ta->core,
        mmuStatusRegAddress
        ,
        MMUStatus
        ));

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        Hardware->os, Hardware->ta->core,
        mmuExceptionAddress
        ,
        MMUException
        ));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gctaHARDWARE_HandleMMUException(
    IN gcTA_HARDWARE Hardware,
    IN gctUINT32 MMUStatus,
    IN gctPHYS_ADDR_T Physical,
    IN gctUINT32 GPUAddress
    )
{
    gctUINT32 mmu       = 0;
    gctUINT32 mmuStatus = 0;
    gctUINT32 mtlbEntry = 0;
    gctUINT32_PTR stlbEntry;
    gctBOOL secure;

    gctUINT32 mmuStatusRegAddress;
    gctUINT32 mmuExceptionAddress;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
    mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;

    gcmkVERIFY_OK(gctaOS_ReadRegister(
        Hardware->os, Hardware->ta->core,
        mmuStatusRegAddress
        ,
        &mmuStatus
        ));

    mmu = mmuStatus & 0xF;

    /* Setup page table. */

    gctaMMU_GetPageEntry(
        Hardware->ta->mmu,
        GPUAddress,
        &mtlbEntry,
        &stlbEntry,
        &secure
        );

    gctaMMU_SetPage(
        Hardware->ta->mmu,
        (gctUINT32)Physical,
        stlbEntry
        );

    switch (mmu)
    {
    case 1:
        gcmkASSERT(mtlbEntry != 0);
        gctaOS_WriteRegister(
            Hardware->os, Hardware->ta->core,
            mmuExceptionAddress
            ,
            mtlbEntry
            );

        break;

    case 2:
         gctaOS_WriteRegister(
            Hardware->os, Hardware->ta->core,
            mmuExceptionAddress
            ,
            *stlbEntry
            );
        break;

    default:
        gcmkASSERT(0);
    }


    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}
