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
#include "gc_hal_kernel_context.h"

#include "gc_feature_database.h"
#include <gc_hal_kernel_debug.h>

#define _GC_OBJ_ZONE gcvZONE_HARDWARE

typedef struct _gcsiDEBUG_REGISTERS *gcsiDEBUG_REGISTERS_PTR;
typedef struct _gcsiDEBUG_REGISTERS {
    gctSTRING module;
    gctUINT   index;
    gctUINT   shift;
    gctUINT   data;
    gctUINT   count;
    gctUINT32 pipeMask;
    gctUINT32 selectStart;
    gctBOOL   avail;
    gctBOOL   inCluster;
} gcsiDEBUG_REGISTERS;

typedef struct _gcsFE_STACK {
    gctSTRING name;
    gctINT    count;
    gctUINT32 highSelect;
    gctUINT32 lowSelect;
    gctUINT32 linkSelect;
    gctUINT32 clear;
    gctUINT32 next;
} gcsFE_STACK;

/******************************************************************************
 ********************************* Support Code *******************************
 ******************************************************************************/
static gctBOOL
_IsHardwareMatch(gckHARDWARE Hardware, gctINT32 ChipModel, gctUINT32 ChipRevision)
{
    return ((Hardware->identity.chipModel == ChipModel) &&
            (Hardware->identity.chipRevision == ChipRevision));
}

static gctBOOL _IsHardwareMatchFlag = gcvFALSE;

static gceSTATUS _ResetGPU(gckHARDWARE Hardware, gctBOOL NeedWR);

static void
_GetEcoID(gckHARDWARE Hardware,
          gcsHAL_QUERY_CHIP_IDENTITY_PTR Identity)
{
    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       GC_ECO_ID_Address, &Identity->ecoID));
}

static gceSTATUS
_IdentifyHardwareByDatabase(gckHARDWARE Hardware, gckOS Os,
                            gckDEVICE Device, gcsHAL_QUERY_CHIP_IDENTITY_PTR Identity)
{
    gceSTATUS status;
    gctUINT32 chipIdentity;
    gctUINT32 debugControl0;
    gctUINT32 chipInfo;
    gcsFEATURE_DATABASE *database;
    gctUINT i = 0;

    gcmkHEADER_ARG("Os=%p", Os);

    /* Get chip date. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_DATE_Address, &Identity->chipDate));

    /***************************************************************************
     * Get chip ID and revision.
     */

    /* Read chip identity register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQ_IDENT_Address, &chipIdentity));

    /* Read chip identity register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_ID_Address,
                                     (gctUINT32_PTR)&Identity->chipModel));

    /* Read CHIP_REV register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_REV_Address, &Identity->chipRevision));

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_PRODUCT_ID_Address, &Identity->productID));

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                   "Identity: chipModel=%X", Identity->chipModel);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                   "Identity: chipRevision=%X", Identity->chipRevision);

    _GetEcoID(Hardware, Identity);

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_CUSTOMER_Address, &Identity->customerID));

    status = gckOS_QueryOption(Hardware->os, "userClusterMasks",
                               (gctUINT64 *)Hardware->options.userClusterMasks);

    if (gcmIS_SUCCESS(status)) {
        gctUINT32 userClusterMask = Hardware->options.userClusterMasks[Hardware->core];
        gctUINT index = 0;
        gctUINT32 control;

        while (!(userClusterMask & 0x1)) {
            userClusterMask >>= 1;
            index++;
        }

        Hardware->aliveCluster0Index = index;

        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel,
                                         AQ_HI_CLOCK_CONTROL_Address, &control));

        control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);

        /* Select the first enabled cluster. */
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address, control));
    }

#ifdef GC_CHIP_CONFIG_Address
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_CONFIG_Address, &Identity->chipConfig));
#endif
    /*get hw minor features*/
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_FEATURES_Address, &Identity->chipFeatures));

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES0_Address, &Identity->chipMinorFeatures));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES1_Address, &Identity->chipMinorFeatures1));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES2_Address, &Identity->chipMinorFeatures2));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES3_Address, &Identity->chipMinorFeatures3));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES4_Address, &Identity->chipMinorFeatures4));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES5_Address, &Identity->chipMinorFeatures5));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES6_Address, &Identity->chipMinorFeatures6));

    /***************************************************************************
     ** Get chip features.
     */

    database = gcQueryFeatureDB(Hardware->identity.chipModel,
                                Hardware->identity.chipRevision,
                                Hardware->identity.productID,
                                Hardware->identity.ecoID,
                                Hardware->identity.customerID);

    if (database == gcvNULL) {
        gcmkPRINT("[ftg340%d]: Feature database is not found, chipModel=0x%0x, chipRevision=0x%x, productID=0x%x, ecoID=0x%x, customerID=0x%x",
                  Hardware->kernel->core,
                  Hardware->identity.chipModel,
                  Hardware->identity.chipRevision,
                  Hardware->identity.productID,
                  Hardware->identity.ecoID,
                  Hardware->identity.customerID);
        gcmkONERROR(gcvSTATUS_NOT_FOUND);
    } else if (database->chipVersion != Hardware->identity.chipRevision) {
        gcmkPRINT("[ftg340]: Warning: chipRevision mismatch, database chipRevision=0x%x register read chipRevision=0x%x\n",
                  database->chipVersion, Hardware->identity.chipRevision);
    }

    Hardware->featureDatabase = database;

    if (Hardware->identity.chipConfig != 0) {
        database->ClusterAliveMask = (0xff >> (8 - (Hardware->identity.chipConfig & 0xff)));
        ((gcsFEATURE_DATABASE *)(Hardware->featureDatabase))->ClusterAliveMask = database->ClusterAliveMask;
    }

    if (database->Q_CHANNEL_SUPPORT) {
        /* Qchannel power on. */
        gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvTRUE, gcvTRUE));
    }

    Identity->pixelPipes = database->NumPixelPipes;
    Identity->resolvePipes = database->NumResolvePipes;
    Identity->instructionCount = database->InstructionCount;
    Identity->PSInstructionCount = database->PS_INSTRUCTION_COUNT;
    Identity->numConstants = database->NumberOfConstants;
    Identity->varyingsCount = database->VaryingCount;
    Identity->gpuCoreCount = database->CoreCount;
    Identity->streamCount = database->Streams;
    Identity->clusterAvailMask = database->ClusterAliveMask;

    for (i = Identity->clusterAvailMask; i & 1; i >>= 1)
       Hardware->clusterCount++;

    Identity->nnClusterNum = database->NN_CLUSTER_NUM_FOR_POWER_CONTROL;
    Identity->virtualAddressBits = database->VIRTUAL_ADDRESS_BITS;
    Identity->physicalAddressBits = database->PHYSICAL_ADDRESS_BITS;

    if (Identity->virtualAddressBits > 32) {
        /* If disable 40VA, use largeVAVersion as default value gcv32BIT_VA_40BIT_PA. */
#if gcdENABLE_40BIT_VA
        Hardware->largeVAVersion = database->MMU_40BIT_VA_FIX ?
                                   gcv40BIT_VA_40BIT_PA_1 : gcv40BIT_VA_40BIT_PA_0;

        if (Identity->physicalAddressBits == 48)
            Hardware->largeVAVersion = gcv40BIT_VA_48BIT_PA_0;
#endif
        Hardware->graphicsLargeVA = database->MMU_40BIT_VA_GRAPHICS ? gcvTRUE : gcvFALSE;
    }

    if (gcmIS_SUCCESS(gckOS_QueryOption(Hardware->os, "sRAMBases",
                                        Device->sRAMBases[0]))) {
        gckOS_MemCopy(Identity->sRAMBases, Device->sRAMBases[Hardware->core],
                      sizeof(gctUINT64) * gcvSRAM_INTER_COUNT);
    } else {
        for (i = 0; i < gcvSRAM_INTER_COUNT; i++)
            Identity->sRAMBases[i] = gcvINVALID_PHYSICAL_ADDRESS;
    }

    if (gcmIS_SUCCESS(gckOS_QueryOption(Hardware->os, "sRAMSizes",
                                        (gctUINT64 *)Device->sRAMSizes[0]))) {
        gckOS_MemCopy(Identity->sRAMSizes, Device->sRAMSizes[Hardware->core],
                      sizeof(gctUINT32) * gcvSRAM_INTER_COUNT);
    } else {
        for (i = gcvSRAM_INTERNAL0; i < gcvSRAM_INTER_COUNT; i++)
            Identity->sRAMSizes[i] = 0;
    }

    for (i = gcvSRAM_INTERNAL0; i < gcvSRAM_INTER_COUNT; i++) {
        if (Identity->sRAMSizes[i])
            break;
    }

    /* If module parameter doesn't set per-core SRAM sizes. */
    if (i == gcvSRAM_INTER_COUNT) {
        gctUINT j = 0;

        for (i = Hardware->core; i < database->CoreCount; i++) {
            for (j = gcvSRAM_INTERNAL0; j < gcvSRAM_INTER_COUNT; j++) {
                /* Try to get SRAM sizes from database. */
                if (database->VIP_SRAM_SIZE_ARRAY[0] > 0) {
                    Device->sRAMSizes[i][j] = database->VIP_SRAM_SIZE_ARRAY[i];
                    Identity->sRAMSizes[j] = database->VIP_SRAM_SIZE_ARRAY[i];
                } else {
                    Device->sRAMSizes[i][j] = database->VIP_SRAM_SIZE;
                    Identity->sRAMSizes[j] = database->VIP_SRAM_SIZE;
                }
            }
        }
    }

    if (!Device->extSRAMSizes[0]) {
        gckOS_QueryOption(Hardware->os, "extSRAMBases", Device->extSRAMBases);
        gckOS_QueryOption(Hardware->os, "extSRAMSizes", (gctUINT64 *)Device->extSRAMSizes);
    }

    /* If module parameter doesn't set external SRAM sizes. */
    for (i = 0; i < gcvSRAM_EXT_COUNT; i++) {
        if (Device->extSRAMBases[i] != gcvINVALID_PHYSICAL_ADDRESS && !Device->extSRAMSizes[i]) {
            /* Try to get SRAM sizes from database. */
            Device->extSRAMSizes[i] = database->AXI_SRAM_SIZE;
        }
    }

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_DEBUG_CONTROL0_Address, &debugControl0));

    if (debugControl0 & (1 << 16))
        Identity->chipFlags |= gcvCHIP_FLAG_MSAA_COHERENCEY_ECO_FIX;

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_INFO_Address, &chipInfo));

    if (gcmVERIFYFIELDVALUE(chipInfo, GC_CHIP_INFO, BUS128_BITS, AVAILABLE))
        Identity->chipFlags |= gcvCHIP_AXI_BUS128_BITS;

    gckOS_QueryOption(Os, "platformFlagBits", &Identity->platformFlagBits);

    gckOS_QueryOption(Os, "registerAPB", &Identity->registerAPB);

    gckOS_QueryPCIInfo(Os, Device->platformIndex, &Identity->pciDomain, &Identity->pciBus, &Identity->pciSlot, &Identity->pciFunction);

    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_GetHardwareSignature(gckHARDWARE Hardware, gckOS Os, gcsHARDWARE_SIGNATURE *Signature)
{
    gceSTATUS status;

    gctUINT32 chipIdentity;

    gcmkHEADER_ARG("Os=%p", Os);

    /***************************************************************************
     * Get chip ID and revision.
     */

    /* Read chip identity register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQ_IDENT_Address, &chipIdentity));

    /* Read chip identity register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_ID_Address,
                                     (gctUINT32_PTR)&Signature->chipModel));

    /* Read CHIP_REV register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_CHIP_REV_Address,
                                     &Signature->chipRevision));

    /***************************************************************************
     * Get chip features.
     */

    /* Read chip feature register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_FEATURES_Address, &Signature->chipFeatures));

    /* Read chip minor feature register #0. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES0_Address,
                                     &Signature->chipMinorFeatures));

    if (gcmVERIFYFIELDVALUE(Signature->chipMinorFeatures,
                            GC_MINOR_FEATURES0, DEFAULT_REG0, AVAILABLE)) {
        /* Read chip minor features register #1. */
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES1_Address,
                                         &Signature->chipMinorFeatures1));

        /* Read chip minor features register #2. */
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GC_MINOR_FEATURES2_Address,
                                         &Signature->chipMinorFeatures2));
    } else {
        /* Chip doesn't has minor features register #1 or 2 or 3 or 4 or 5. */
        Signature->chipMinorFeatures1 = 0;
        Signature->chipMinorFeatures2 = 0;
    }

    /* Success. */
    gcmkFOOTER();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/* Set to 1 to enable module clock gating debug function.
 * Following options take effect when it is set to 1.
 */
#define gcdDEBUG_MODULE_CLOCK_GATING          0
/* Set to 1 to disable module clock gating of all modules. */
#define gcdDISABLE_MODULE_CLOCK_GATING        0
/* Set to 1 to disable module clock gating of each module. */
#define gcdDISABLE_STARVE_MODULE_CLOCK_GATING 0
#define gcdDISABLE_FE_CLOCK_GATING            0
#define gcdDISABLE_PE_CLOCK_GATING            0
#define gcdDISABLE_PA_CLOCK_GATING            0
#define gcdDISABLE_SE_CLOCK_GATING            0
#define gcdDISABLE_RA_CLOCK_GATING            0
#define gcdDISABLE_RA_EZ_CLOCK_GATING         0
#define gcdDISABLE_RA_HZ_CLOCK_GATING         0
#define gcdDISABLE_RS_CLOCK_GATING            0
#define gcdDISABLE_TX_CLOCK_GATING            0
#define gcdDISABLE_TFB_CLOCK_GATING           0
#define gcdDISABLE_GPIPE_CLOCK_GATING         0
#define gcdDISABLE_BLT_CLOCK_GATING           0
#define gcdDISABLE_TPG_CLOCK_GATING           0
#define gcdDISABLE_VX_CLOCK_GATING            0
#define gcdDISABLE_MC_CLOCK_GATING            0

#define gcdDISABLE_SH_CLOCK_GATING            0
#define gcdDISABLE_SH_EU_CLOCK_GATING         0
#define gcdDISABLE_SH_IO_CLOCK_GATING         0
#define gcdDISABLE_SH_L1_LS_CLOCK_GATING      0

/* SH sub clock gating. */
/* 1. gcregAHBPowerControl0RegAddrs. */
/* bit 0 ~ 3, value 0x0 ~ 0xf. */
#define gcdDISABLE_SH_PSI_CLOCK_GATING        0
/* bit 4 ~ 9, value 0x0 ~ 0x3f.*/
#define gcdDISABLE_SH_PSO_CLOCK_GATING        0
/* bit 10 ~ 12, value 0x0 ~ 0x7. */
#define gcdDISABLE_SH_TXIO_CLOCK_GATING       0
/* bit 16 ~ 27, value 0x0 ~ 0xfff. */
#define gcdDISABLE_SH_GPIPEIO_CLOCK_GATING    0

/* 2. gcregAHBPowerControl1RegAddrs. */
/* bit 0, value 0x0 ~ 0x1. */
#define gcdDISABLE_SH_VPF_CLOCK_GATING        0
/* bit 1, value 0x0 ~ 0x1. */
#define gcdDISABLE_SH_ATTR_CLOCK_GATING       0

#if gcdDEBUG_MODULE_CLOCK_GATING
gceSTATUS
_ConfigureModuleLevelClockGating(gckHARDWARE Hardware)
{
    gctUINT32 data, sh_control0, sh_control1;

    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                       &data));

    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL0_Address,
                                       &sh_control0));

    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL1_Address,
                                       &sh_control1));

#if gcdDISABLE_FE_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_FE,
                       1);
# endif

#if gcdDISABLE_PE_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_PE,
                       1);
# endif

#if gcdDISABLE_SH_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_SH,
                       1);
# endif

#if gcdDISABLE_SH_EU_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_SH_EU,
                       1);
# endif

#if gcdDISABLE_SH_IO_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_SH_IO,
                       1);
# endif

#if gcdDISABLE_SH_L1_LS_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_L1_LS,
                       1);
# endif

#if gcdDISABLE_SH_PSI_CLOCK_GATING
    sh_control0 = gcmSETFIELD(sh_control0,
                              GCREG_AHB_POWER_CONTROL0,
                              DISABLE_MODULE_CLOCK_GATING_SHPSI,
                              gcdDISABLE_SH_PSI_CLOCK_GATING);
# endif

#if gcdDISABLE_SH_PSO_CLOCK_GATING
    sh_control0 = gcmSETFIELD(sh_control0,
                              GCREG_AHB_POWER_CONTROL0,
                              DISABLE_MODULE_CLOCK_GATING_SHPSO,
                              gcdDISABLE_SH_PSO_CLOCK_GATING);
# endif

#if gcdDISABLE_SH_TXIO_CLOCK_GATING
    sh_control0 = gcmSETFIELD(sh_control0,
                              GCREG_AHB_POWER_CONTROL0,
                              DISABLE_MODULE_CLOCK_GATING_SHTXIO,
                              gcdDISABLE_SH_TXIO_CLOCK_GATING);
# endif

#if gcdDISABLE_SH_GPIPEIO_CLOCK_GATING
    sh_control0 = gcmSETFIELD(sh_control0,
                              GCREG_AHB_POWER_CONTROL0,
                              DISABLE_MODULE_CLOCK_GATING_SHGPIPEIO,
                              gcdDISABLE_SH_GPIPEIO_CLOCK_GATING);
# endif

#if gcdDISABLE_SH_VPF_CLOCK_GATING
    sh_control1 = gcmSETFIELD(sh_control1,
                              GCREG_AHB_POWER_CONTROL1,
                              DISABLE_MODULE_CLOCK_GATING_SHVPF,
                              gcdDISABLE_SH_VPF_CLOCK_GATING);
# endif

#if gcdDISABLE_SH_ATTR_CLOCK_GATING
    sh_control1 = gcmSETFIELD(sh_control1,
                              GCREG_AHB_POWER_CONTROL1,
                              DISABLE_MODULE_CLOCK_GATING_SHATTR,
                              gcdDISABLE_SH_ATTR_CLOCK_GATING);
# endif

#if gcdDISABLE_PA_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_PA,
                       1);
# endif

#if gcdDISABLE_SE_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_SE,
                       1);
# endif

#if gcdDISABLE_RA_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_RA,
                       1);
# endif

#if gcdDISABLE_RS_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_RS,
                       1);
# endif

#if gcdDISABLE_TX_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_TX,
                       1);
# endif

#if gcdDISABLE_RA_EZ_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_RA_EZ,
                       1);
# endif

#if gcdDISABLE_RA_HZ_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_RA_HZ,
                       1);
# endif

#if gcdDISABLE_TFB_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_TFB,
                       1);
# endif

#if gcdDISABLE_GPIPE_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_GPIPE,
                       1);
# endif

#if gcdDISABLE_BLT_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_BLT,
                       1);
# endif

#if gcdDISABLE_TPG_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_TPG,
                       1);
# endif

#if gcdDISABLE_VX_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_VX,
                       1);
# endif

#if gcdDISABLE_MC_CLOCK_GATING
    data = gcmSETFIELD(data,
                       GC_MODULE_POWER_MODULE_CONTROL,
                       DISABLE_MODULE_CLOCK_GATING_MC,
                       1);
#    endif

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                        Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                        data));

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                        Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL0_Address,
                                        sh_control0));

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                        Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL1_Address,
                                        sh_control1));

#if gcdDISABLE_STARVE_MODULE_CLOCK_GATING
    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                       &data));

    data = gcmSETFIELD(data, GC_MODULE_POWER_CONTROLS, DISABLE_STARVE_MODULE_CLOCK_GATING, 1);

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                        Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                        data));

# endif

#if gcdDISABLE_MODULE_CLOCK_GATING
    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                       &data));

    data = gcmSETFIELD(data, GC_MODULE_POWER_CONTROLS, ENABLE_MODULE_CLOCK_GATING, 0);

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                        Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                        data));
# endif

    return gcvSTATUS_OK;
}
#endif

#if gcdPOWEROFF_TIMEOUT
static void
_PowerStateTimerFunc(gctPOINTER Data)
{
    gckHARDWARE hardware = (gckHARDWARE)Data;

    gcmkVERIFY_OK(gckHARDWARE_SetPowerState(hardware, hardware->nextPowerState));
}
#endif

static gceSTATUS
_VerifyDMA(gckOS Os,
           gckHARDWARE Hardware,
           gctADDRESS *Address1,
           gctADDRESS *Address2,
           gctUINT32_PTR State1,
           gctUINT32_PTR State2)
{
    gceSTATUS status;
    gctUINT32 i;
    gctUINT32 addressLow, addressHi = 0;

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_STATE_Address, State1));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

    *Address1 = addressLow;

    if (Hardware->graphicsLargeVA) {
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

        *Address1 = ((gctADDRESS)addressHi << 32) | addressLow;
    }

    for (i = 0; i < 500; i += 1) {
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_STATE_Address, State2));
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

        *Address2 = addressLow;

        if (Hardware->graphicsLargeVA) {
            gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

            *Address2 = ((gctADDRESS)addressHi << 32) | addressLow;
        }


        if (*Address1 != *Address2)
            break;

        if (*State1 != *State2)
            break;
    }

OnError:
    return status;
}

static gceSTATUS
_DumpDebugRegisters(gckOS Os, gckKERNEL Kernel, gcsiDEBUG_REGISTERS_PTR Descriptor)
{
/* If this value is changed, print formats need to be changed too. */
#define REG_PER_LINE 8
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 select;
    gctUINT i, j, pipe;
    gctUINT32 datas[REG_PER_LINE];
    gctUINT32 oldControl, control;
    gctUINT32 maxNumOfPipes = Kernel->hardware->identity.pixelPipes;
    gctUINT32 clusterMask;

    gcmkHEADER_ARG("Os=%p Descriptor=%p", Os, Descriptor);

    gcmkONERROR(gckHARDWARE_QueryClusterInfo(Kernel->hardware, &clusterMask));

    if (Kernel->hardware->clusterCount > maxNumOfPipes)
        maxNumOfPipes = Kernel->hardware->clusterCount;

    /* Record control. */
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, 0x0, &oldControl));

    for (pipe = 0; pipe < maxNumOfPipes; pipe++) {
        if (!((1 << pipe) & clusterMask))
            continue;

        if (!Descriptor->avail)
            continue;
        if (!(Descriptor->pipeMask & (1 << pipe)))
            continue;

        gcmkPRINT_N(8, "    %s[%d] debug registers:\n", Descriptor->module, pipe);

        /* Switch pipe. */
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, 0x0, &control));
        control &= ~(0xF << 20);
        control |= (pipe << 20);
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, 0x0, control));

        gcmkASSERT(!(Descriptor->count % REG_PER_LINE));

        for (i = 0; i < Descriptor->count; i += REG_PER_LINE) {
            /* Select of first one in the group. */
            select = i + Descriptor->selectStart;

            /* Read a group of registers. */
            for (j = 0; j < REG_PER_LINE; j++) {
                /* Shift select to right position. */
                gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index,
                                                  (select + j) << Descriptor->shift));
                gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &datas[j]));
            }

            gcmkPRINT_N(32, "    [%02X] %08X %08X %08X %08X %08X %08X %08X %08X\n",
                        select, datas[0], datas[1], datas[2], datas[3],
                        datas[4], datas[5], datas[6], datas[7]);
        }
    }

    /* Restore control. */
    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, 0x0, oldControl));

OnError:
    /* Return the error. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_DumpLinkStack(gckOS Os, gckKERNEL Kernel, gcsiDEBUG_REGISTERS_PTR Descriptor)
{
    /* Get wrptr */
    gctUINT32 shift = Descriptor->shift;
    gctUINT32 pointerSelect = 0xE << shift;
    gctUINT32 pointer, wrPtr, rdPtr, links[16];
    gctUINT32 stackSize = 16;
    gctUINT32 oldestPtr = 0;
    gctUINT32 i;

    gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, pointerSelect));
    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &pointer));

    wrPtr = (pointer & 0xF0) >> 4;
    rdPtr = pointer & 0xF;

    /* Move rdptr to the oldest one (next one to the latest one. ) */
    oldestPtr = (wrPtr + 1) % stackSize;

    while (rdPtr != oldestPtr) {
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, 0x0));
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, 0xF << shift));

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, pointerSelect));
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &pointer));

        rdPtr = pointer & 0xF;
    }

    gcmkPRINT("    Link stack:");

    /* Read from stack bottom*/
    for (i = 0; i < stackSize; i++) {
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, 0xD << shift));
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &links[i]));

        /* Advance rdPtr. */
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, 0x0));
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, 0xF << shift));
    }

    /* Print. */
    for (i = 0; i < stackSize; i += 4) {
        gcmkPRINT_N(32, "      [0x%02X] 0x%08X [0x%02X] 0x%08X [0x%02X] 0x%08X [0x%02X] 0x%08X\n",
                    i, links[i], i + 1, links[i + 1], i + 2, links[i + 2], i + 3, links[i + 3]);
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_DumpFEStack(gckOS Os, gckKERNEL Kernel, gcsiDEBUG_REGISTERS_PTR Descriptor)
{
    gctUINT i;
    gctINT j;
    gctUINT32 stack[32][2];
    gctUINT32 link[32];

    static gcsFE_STACK _feStacks[] = {
        { "PRE_STACK", 32, 0x1A, 0x9A, 0x00, 0x1B, 0x1E },
        { "CMD_STACK", 32, 0x1C, 0x9C, 0x1E, 0x1D, 0x1E },
    };

    for (i = 0; i < gcmCOUNTOF(_feStacks); i++) {
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, _feStacks[i].clear));

        for (j = 0; j < _feStacks[i].count; j++) {
            gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, _feStacks[i].highSelect));

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &stack[j][0]));

            gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, _feStacks[i].lowSelect));

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &stack[j][1]));

            gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, _feStacks[i].next));

            if (_feStacks[i].linkSelect) {
                gcmkVERIFY_OK(gckOS_WriteRegisterEx(Os, Kernel, Descriptor->index, _feStacks[i].linkSelect));

                gcmkVERIFY_OK(gckOS_ReadRegisterEx(Os, Kernel, Descriptor->data, &link[j]));
            }
        }

        gcmkPRINT("  %s:", _feStacks[i].name);

        for (j = 31; j >= 3; j -= 4) {
            gcmkPRINT("    %08X %08X %08X %08X %08X %08X %08X %08X",
                      stack[j][0],     stack[j][1],     stack[j - 1][0], stack[j - 1][1],
                      stack[j - 2][0], stack[j - 2][1], stack[j - 3][0], stack[j - 3][1]);
        }

        if (_feStacks[i].linkSelect) {
            gcmkPRINT("  LINK_STACK:");

            for (j = 31; j >= 3; j -= 4) {
                gcmkPRINT("    %08X %08X %08X %08X %08X %08X %08X %08X",
                          link[j],     link[j],     link[j - 1], link[j - 1],
                          link[j - 2], link[j - 2], link[j - 3], link[j - 3]);
            }
        }
    }

    return gcvSTATUS_OK;
}

static gceSTATUS
_IsGPUPresent(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gcsHARDWARE_SIGNATURE signature;
    gctUINT32 control;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &control));

    control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, CLK2D_DIS, 0);
    control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, CLK3D_DIS, 0);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address, control));

    gckOS_ZeroMemory((gctPOINTER)&signature, gcmSIZEOF(gcsHARDWARE_SIGNATURE));

    /* Identify the hardware. */
    gcmkONERROR(_GetHardwareSignature(Hardware, Hardware->os, &signature));

    /* Check if these are the same values as saved before. */
    if (Hardware->signature.chipModel != signature.chipModel ||
        Hardware->signature.chipRevision != signature.chipRevision ||
        Hardware->signature.chipFeatures != signature.chipFeatures ||
        Hardware->signature.chipMinorFeatures != signature.chipMinorFeatures ||
        Hardware->signature.chipMinorFeatures1 != signature.chipMinorFeatures1 ||
        Hardware->signature.chipMinorFeatures2 != signature.chipMinorFeatures2) {
        gcmkONERROR(gcvSTATUS_GPU_NOT_RESPONDING);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_FlushCache(gckHARDWARE Hardware, gckCOMMAND Command)
{
    gceSTATUS status;
    gctUINT32 bytes, requested;
    gctPOINTER buffer;

    /* Get the size of the flush command. */
    gcmkONERROR(gckHARDWARE_Flush(Hardware, gcvFLUSH_ALL, gcvNULL, &requested));

    /* Reserve space in the command queue. */
    gcmkONERROR(gckCOMMAND_Reserve(Command, requested, &buffer, &bytes));

    /* Append a flush. */
    gcmkONERROR(gckHARDWARE_Flush(Hardware, gcvFLUSH_ALL, buffer, &bytes));

    /* Execute the command queue. */
    gcmkONERROR(gckCOMMAND_Execute(Command, requested));

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gctBOOL
_IsHWIdle(gctUINT32 Idle, gckHARDWARE Hardware)
{
#if !gcdFPGA_BUILD
    Idle |= 1 << AQ_HI_IDLE_IDLE_MC_Start;
#endif

    return Idle == AQ_HI_IDLE_ResetValue;
}

static gctBOOL
_QueryFeatureDatabase(gckHARDWARE Hardware, gceFEATURE Feature)
{
    gctBOOL available;

    gcsFEATURE_DATABASE *database = Hardware->featureDatabase;

    gcmkHEADER_ARG("Hardware=%p Feature=%d", Hardware, Feature);

    /* Only features needed by common kernel logic added here. */
    switch (Feature) {
    case gcvFEATURE_END_EVENT:
        available = gcvFALSE;
        break;

    case gcvFEATURE_MC20:
        available = database->REG_MC20;
        break;

    case gcvFEATURE_EARLY_Z:
        available = database->REG_NoEZ == 0;
        break;

    case gcvFEATURE_HZ:
        available = database->REG_HierarchicalZ;
        break;

    case gcvFEATURE_NEW_HZ:
        available = database->REG_NewHZ;
        break;

    case gcvFEATURE_FAST_MSAA:
        available = database->REG_FastMSAA;
        break;

    case gcvFEATURE_SMALL_MSAA:
        available = database->REG_SmallMSAA;
        break;

    case gcvFEATURE_DYNAMIC_FREQUENCY_SCALING:
        /* This feature doesn't apply for 2D cores. */
        available = database->REG_DynamicFrequencyScaling && database->REG_Pipe3D;
        break;

    case gcvFEATURE_ACE:
        available = database->REG_ACE;
        break;

    case gcvFEATURE_HALTI2:
        available = database->REG_Halti2;
        break;

    case gcvFEATURE_PIPE_2D:
        available = database->REG_Pipe2D;
        break;

    case gcvFEATURE_PIPE_3D:
        available = database->REG_Pipe3D;
        break;

    case gcvFEATURE_FC_FLUSH_STALL:
        available = database->REG_FcFlushStall;
        break;

    case gcvFEATURE_BLT_ENGINE:
        available = database->REG_BltEngine;
        break;

    case gcvFEATURE_HALTI0:
        available = database->REG_Halti0;
        break;

    case gcvFEATURE_FE_ALLOW_STALL_PREFETCH_ENG:
        available = database->REG_FEAllowStallPrefetchEng;
        break;

    case gcvFEATURE_MMU:
#if gcdCAPTURE_ONLY_MODE
        available = gcvTRUE;
#else
        available = database->REG_MMU;
#endif

        break;

    case gcvFEATURE_FENCE_64BIT:
        if (Hardware->identity.customerID == 0x54)
            available = gcvFALSE;
        else
            available = database->FENCE_64BIT;

        break;

    case gcvFEATURE_TEX_BASELOD:
        available = database->REG_Halti2;
        break;

    case gcvFEATURE_TEX_CACHE_FLUSH_FIX:
        available = database->REG_Halti5;
        break;

    case gcvFEATURE_BUG_FIXES1:
        available = database->REG_BugFixes1;
        break;

    case gcvFEATURE_MULTI_SOURCE_BLT:
        available = database->REG_MultiSourceBlt;
        break;

    case gcvFEATURE_HALTI5:
        available = database->REG_Halti5;
        break;

    case gcvFEATURE_FAST_CLEAR:
        available = database->REG_FastClear;
        break;

    case gcvFEATURE_BUG_FIXES7:
        available = database->REG_BugFixes7;
        break;

    case gcvFEATURE_ZCOMPRESSION:
        available = database->REG_ZCompression;
        break;

    case gcvFEATURE_SHADER_HAS_INSTRUCTION_CACHE:
        available = database->REG_InstructionCache;
        break;

    case gcvFEATURE_YUV420_TILER:
        available = database->REG_YUV420Tiler;
        break;

    case gcvFEATURE_2DPE20:
        available = database->REG_2DPE20;
        break;

    case gcvFEATURE_DITHER_AND_FILTER_PLUS_ALPHA_2D:
        available = database->REG_DitherAndFilterPlusAlpha2D;
        break;

    case gcvFEATURE_ONE_PASS_2D_FILTER:
        available = database->REG_OnePass2DFilter;
        break;

    case gcvFEATURE_HALTI1:
        available = database->REG_Halti1;
        break;

    case gcvFEATURE_HALTI3:
        available = database->REG_Halti3;
        break;

    case gcvFEATURE_HALTI4:
        available = database->REG_Halti4;
        break;

    case gcvFEATURE_GEOMETRY_SHADER:
        available = database->REG_GeometryShader;
        break;

    case gcvFEATURE_TESSELLATION:
        available = database->REG_TessellationShaders;
        break;

    case gcvFEATURE_GENERIC_ATTRIB:
        available = database->REG_Generics;
        break;

    case gcvFEATURE_TEXTURE_LINEAR:
        available = database->REG_LinearTextureSupport;
        break;

    case gcvFEATURE_TX_FILTER:
        available = database->REG_TXFilter;
        break;

    case gcvFEATURE_TX_SUPPORT_DEC:
        available = database->REG_TXSupportDEC;
        break;

    case gcvFEATURE_TX_FRAC_PRECISION_6BIT:
        available = database->REG_TX6bitFrac;
        break;

    case gcvFEATURE_TEXTURE_ASTC:
        available = database->REG_TXEnhancements4 && !database->NO_ASTC;
        break;

    case gcvFEATURE_SHADER_ENHANCEMENTS2:
        available = database->REG_SHEnhancements2;
        break;

    case gcvFEATURE_BUG_FIXES18:
        available = database->REG_BugFixes18;
        break;

    case gcvFEATURE_64K_L2_CACHE:
        available = gcvFALSE;
        break;

    case gcvFEATURE_BUG_FIXES4:
        available = database->REG_BugFixes4;
        break;

    case gcvFEATURE_BUG_FIXES12:
        available = database->REG_BugFixes12;
        break;

    case gcvFEATURE_HW_TFB:
        available = database->HWTFB;
        break;

    case gcvFEATURE_HW_TFB_PERF_FIX:
        available = database->TFB_PERF_FIX;
        break;

    case gcvFEATURE_SNAPPAGE_CMD_FIX:
        available = database->SH_SNAP2PAGE_FIX;
        break;

    case gcvFEATURE_SECURITY:
        available = database->SECURITY;
        break;

    case gcvFEATURE_TX_DESCRIPTOR:
        available = database->REG_Halti5;
        break;

    case gcvFEATURE_TX_DESC_CACHE_CLOCKGATE_FIX:
        available = database->TX_DESC_CACHE_CLOCKGATE_FIX;
        break;

    case gcvFEATURE_ROBUSTNESS:
        available = database->ROBUSTNESS;
        break;

    case gcvFEATURE_SNAPPAGE_CMD:
        available = database->SNAPPAGE_CMD;
        break;

    case gcvFEATURE_HALF_FLOAT_PIPE:
        available = database->REG_HalfFloatPipe;
        break;

    case gcvFEATURE_SH_INSTRUCTION_PREFETCH:
        available = database->SH_ICACHE_PREFETCH;
        break;

    case gcvFEATURE_FE_NEED_DUMMYDRAW:
        available = database->FE_NEED_DUMMYDRAW;
        break;

    case gcvFEATURE_PROBE:
        available = database->REG_Probe;
        break;

    case gcvFEATURE_DEC300_COMPRESSION:
        available = database->REG_DEC;
        break;

    case gcvFEATURE_DEC400_COMPRESSION:
        available = database->G2D_DEC400;
        break;

    case gcvFEATURE_DEC400EX_COMPRESSION:
        available = database->G2D_DEC400EX;
        break;

    case gcvFEATURE_TPC_COMPRESSION:
        available = database->REG_ThirdPartyCompression;
        break;

    case gcvFEATURE_TPCV11_COMPRESSION:
        available = database->G2D_3rd_PARTY_COMPRESSION_1_1;
        break;

    case gcvFEATURE_USC_DEFER_FILL_FIX:
        available = database->USC_DEFER_FILL_FIX;
        break;

    case gcvFEATURE_USC:
        available = database->REG_Halti5;
        break;

    case gcvFEATURE_RA_CG_FIX:
        available = database->RA_CG_FIX;
        break;

    case gcvFEATURE_MULTI_CLUSTER:
        available = database->MULTI_CLUSTER;
        break;

    case gcvFEATURE_ZERO_ATTRIB_SUPPORT:
        available = database->REG_Halti4;
        break;

    case gcvFEATURE_SH_CLOCK_GATE_FIX:
        available = database->SH_CLOCK_GATE_FIX;
        break;

    case gcvFEATURE_GPIPE_CLOCK_GATE_FIX:
        available = gcvFALSE;
        break;

    case gcvFEATURE_NEW_GPIPE:
        available = database->NEW_GPIPE;
        break;

    case gcvFEATURE_MULTI_CORE_BLOCK_SET_CONFIG2:
        available = database->MULTI_CORE_BLOCK_SET_CONFIG2;
        break;

    case gcvFEATURE_SECURITY_AHB:
        available = database->SECURITY_AHB;
        break;

    case gcvFEATURE_SMALL_BATCH:
        available = database->SMALLBATCH;
#if gcdWINDOWS_GL
        available = gcvFALSE;
#endif
        break;

    case gcvFEATURE_ASYNC_BLIT:
        available = database->ASYNC_BLT;
        break;

    case gcvFEATURE_PSCS_THROTTLE:
        available = database->PSCS_THROTTLE;
        break;

    case gcvFEATURE_SEPARATE_LS:
        available = database->SEPARATE_LS;
        break;

    case gcvFEATURE_MCFE:
        available = database->MCFE;
        break;

    case gcvFEATURE_COMPUTE_ONLY:
        available = database->COMPUTE_ONLY;
        break;

    case gcvFEATURE_USC_FULLCACHE_FIX:
        available = database->USC_FULL_CACHE_FIX;
        break;

    case gcvFEATURE_PE_TILE_CACHE_FLUSH_FIX:
        available = database->PE_TILE_CACHE_FLUSH_FIX;
        break;

    case gcvFEATURE_TILE_STATUS_2BITS:
        available = database->REG_TileStatus2Bits;
        break;

    case gcvFEATURE_128BTILE:
        available = database->CACHE128B256BPERLINE;
        break;

    case gcvFEATURE_COMPRESSION_DEC400:
        available = database->DEC400;
        break;

    case gcvFEATURE_SUPPORT_GCREGTX:
        available = database->REG_Halti1;
        break;

    case gcvFEATURE_MSAA_FRAGMENT_OPERATION:
        available = database->MSAA_FRAGMENT_OPERATION;
        break;

    case gcvFEATURE_OCB_COUNTER:
        available = database->OCB_COUNTER;
        break;

    case gcvFEATURE_AI_GPU:
        available = database->AI_GPU;
        break;

    case gcvFEATURE_NN_ENGINE:
        available = database->NNCoreCount > 0;
        break;

    case gcvFEATURE_TP_ENGINE:
        available = database->TP_ENGINE;
        break;

    case gcvFEATURE_HI_REORDER_FIX:
        available = database->HI_REORDER_FIX;
        break;

    case gcvFEATURE_EVIS2_FLOP_RESET_FIX:
        available = database->EVIS2_FLOP_RESET_FIX;
        break;

    case gcvFEATURE_USC_ASYNC_CP_RTN_FLOP_RESET_FIX:
        available = database->USC_ASYNC_CP_RTN_FLOP_RESET_FIX;
        break;

    case gcvFEATURE_TS_FC_VULKAN_SUPPORT:
        available = database->TS_FC_VULKAN_SUPPORT;
        break;

    case gcvFEATURE_USC_EVICT_CTRL_FIFO_FLOP_RESET_FIX:
        available = database->USC_EVICT_CTRL_FIFO_FLOP_RESET_FIX;
        break;

    case gcvFEATURE_Q_CHANNEL_SUPPORT:
        available = database->Q_CHANNEL_SUPPORT;
        break;

    case gcvFEATURE_MMU_PAGE_DESCRIPTOR:
        available = database->MMU_PAGE_DESCRIPTOR;
        break;

    case gcvFEATURE_VIP_REMOVE_MMU:
        available = database->VIP_REMOVE_MMU;
        break;

    case gcvFEATURE_VIP_SCALER:
        available = database->SCALER;
        break;

    case gcvFEATURE_VIP_SCALER_4K:
        available = database->SCALER_4K;
        break;

    case gcvFEATURE_BIT_AXI_FE:
        available = database->AXIFE;
        break;

    case gcvFEATURE_2D_FRAME_DONE_INTR:
#ifdef EMULATOR
        available = gcvFALSE;
#else
        available = database->G2D_FRAME_DONE_INTR;
#endif
        break;

    case gcvFEATURE_SH_SUPPORT_HIGHPVEC_FORMAT:
        available = database->HIGHP_VEC2;
        break;

    case gcvFEATURE_SH_SUPPORT_AIGM:
        available = database->ATTR_IN_GLOBAL_MEMORY;
        if (database->AIGM_MAX_SIZE == 0 && available)
        {
            gcmkASSERT(gcvFALSE);
            available = gcvFALSE;
        }
        break;

    case gcvFEATURE_SH_SUPPORT_MULTIVIEWPORT:
        available = database->D3D11_SUPPORT;
        break;

    case gcvFEATURE_SH_HAS_VS_SEMANTIC_LOC:
        available = database->D3D11_SUPPORT;
        break;

    case gcvFEATURE_SH_PER_STAGE_LOCAL_STORAGE:
        available = database->PER_STAGE_LOCAL_STORAGE;
        break;

    case gcvFEATURE_SH_SUPPORT_SEPARATED_TEX:
        available = database->SEPARATED_TEXTURE_SAMPLER;
        break;

    case gcvFEATURE_SH_SUPPORT_CLIP_CULL_DISTANCE:
        available = database->CLIP_DISTANCE_SUPPORT;
        break;

    case gcvFEATURE_TC_SHADER_TRIGGER_NN:
        available = database->TC_SHADER_TRIGGER_NN;
        break;

    case gcvFEATURE_BIT_NN_JOB_CANCELATION:
        available = database->NN_JOB_CANCELATION;
        break;

    case gcvFEATURE_NN_SUPPORT_EFUSE:
        available = database->NN_SUPPORT_EFUSE;
        break;

    case gcvFEATURE_BIT_SRAM_PARITY:
        available = database->SRAM_PARITY;
        break;

    case gcvFEATURE_D3D11_SUPPORT:
        available = database->D3D11_SUPPORT;
        break;

    case gcvFEATURE_MMU_40BIT_VA_GRAPHICS:
        available = database->MMU_40BIT_VA_GRAPHICS;
        break;

    case gcvFEATURE_CHIPENABLE_LINK:
        available = database->ChipEnableLink;
        break;

    case gcvFEATURE_BIT_VGPU:
        available = database->VGPU;
        break;
        /*FALLTHRU*/
    default:
        gcmkFATAL("Invalid feature has been requested, %d.", Feature);
        available = gcvFALSE;
        /*FALLTHRU*/
    }

    gcmkFOOTER_ARG("%d", available ? gcvSTATUS_TRUE : gcvSTATUS_FALSE);
    return available;
}

static void
_ConfigurePolicyID(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT32 policyID;
    gctUINT32 auxBit = ~0U;
    gctUINT32 axiConfig;
    gckOS os = Hardware->os;
    gctUINT32 i;
    gctUINT32 offset;
    gctUINT32 shift;
    gctUINT32 currentAxiConfig;

    status = gckOS_GetPolicyID(os, gcvVIDMEM_TYPE_GENERIC, &policyID, &axiConfig);

    if (status == gcvSTATUS_NOT_SUPPORTED) {
        /* No customized policyID setting. */
        return;
    }

    for (i = 0; i < 16; i++) {
        /* Mapping 16 surface type.*/
        status = gckOS_GetPolicyID(os, (gceVIDMEM_TYPE)i, &policyID, &axiConfig);

        if (gcmIS_SUCCESS(status)) {
            if (auxBit == ~0U) {
                /* There is a customized policyID setting for this type. */
                auxBit = (policyID >> 4) & 0x1;
            } else {
                /* Check whether this bit changes. */
                if (auxBit != ((policyID >> 4) & 0x1)) {
                    gcmkPRINT("[galHardware->kernel]: AUX_BIT changes");
                    return;
                }
            }

            offset = policyID >> 1;

            shift = (policyID & 0x1) * 16;

            axiConfig &= 0xFFFF;

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(os, Hardware->kernel,
                                               (gcAxiConfigRegAddrs + offset) << 2,
                                               &currentAxiConfig));

            currentAxiConfig |= (axiConfig << shift);

            gcmkVERIFY_OK(gckOS_WriteRegisterEx(os, Hardware->kernel,
                                                (gcAxiConfigRegAddrs + offset) << 2,
                                                currentAxiConfig));
        }
    }

    if (auxBit != ~0U) {
        gcmkVERIFY_OK(gckOS_WriteRegisterEx(os, Hardware->kernel, GC_USER_MODE_CONFIG_Address,
                                            gcmSETFIELDVALUE(0, GC_USER_MODE_CONFIG, USER_MODE, POLICY_ID_MODE) |
                                                 gcmSETFIELD(0, GC_USER_MODE_CONFIG, POLICY_ID_MODE_SW_AUX_BIT, auxBit)));
    }
}

static gceSTATUS
_QueryNNClusters(gckHARDWARE Hardware)
{
    gctUINT64 enableNN = ~0UL;
    gctUINT32 value = 0;
    gceSTATUS status = gcvSTATUS_OK;

    if (gcmIS_SUCCESS(gckOS_QueryOption(Hardware->os, "enableNN", &enableNN))) {
        if (!enableNN) {
            value = 0x2;
        } else if (enableNN == 0xFF || (enableNN == Hardware->identity.nnClusterNum)) {
            value = 0;
        } else {
            /* We only support maximum 8 clusters by current. */
            if (enableNN > 0x7) {
                gcmkPRINT("[ftg340 warning]: Invalid enableNN value is configured.");

                gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
            }

            value = (gctUINT32)enableNN + 0x2;
        }
    }

    Hardware->options.enableNNClusters = (gctUINT32)enableNN;

    if (value && Hardware->identity.customerID != 0x85)
        gcmkPRINT("ftg340 warning: Don't set enableNN as this chip not support NN cluster power control!\n");

    Hardware->options.configNNPowerControl = value;

OnError:
    return status;
}

/*
 * Initialise hardware options
 */
static void
_SetHardwareOptions(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT64 data = 0;
    gcsHAL_QUERY_CHIP_OPTIONS *options = &Hardware->options;
    gcsFEATURE_DATABASE *database = Hardware->featureDatabase;

    gctBOOL featureUSC = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_USC) ?
                         gcvTRUE : gcvFALSE;
    gctBOOL featureSeparateLS = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SEPARATE_LS) ?
                                gcvTRUE : gcvFALSE;
    gctBOOL featureComputeOnly = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_COMPUTE_ONLY) ?
                                 gcvTRUE : gcvFALSE;
    gctBOOL featureTS = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_TESSELLATION) ?
                        gcvTRUE : gcvFALSE;
    gctUINT32 featureL1CacheSize = database->L1CacheSize;
    gctUINT32 featureUSCMaxPages = database->USC_MAX_PAGES;
    gckDEVICE device = Hardware->kernel->device;
    gctUINT32 i, value;

    status = gckOS_QueryOption(Hardware->os, "powerManagement", &data);
    options->powerManagement = (data != 0);

    if (status == gcvSTATUS_NOT_SUPPORTED) {
        /* Enable power management by default. */
        options->powerManagement = gcvTRUE;
    }

#ifndef EMULATOR
    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_VIP_REMOVE_MMU)) {
        options->enableMMU = gcvFALSE;
    } else {
#endif
        status = gckOS_QueryOption(Hardware->os, "mmu", &data);
        options->enableMMU = (data != 0);

        if (status == gcvSTATUS_NOT_SUPPORTED) {
            /* Disable MMU if we can't get result from OS layer query */
            options->enableMMU = gcvFALSE;
        }
#ifndef EMULATOR
    }
#endif

    if (options->enableMMU == gcvFALSE)
        gcmkPRINT("ftg340 warning: MMU is disabled!\n");

    /* Query enabled NN clusters. */
    _QueryNNClusters(Hardware);

    gcmCONFIGUSC2(gcmk, featureUSC, featureSeparateLS,
                  featureComputeOnly, featureTS,
                  featureL1CacheSize, featureUSCMaxPages,
                  Hardware->options.uscAttribCacheRatio,
                  Hardware->options.uscL1CacheRatio);

    status = gckOS_QueryOption(Hardware->os, "smallBatch", &data);
    options->smallBatch = (data != 0);

    if (status == gcvSTATUS_NOT_SUPPORTED)
        options->smallBatch = gcvTRUE;

    for (i = 0; i < gcdMAX_MAJOR_CORE_COUNT; i++)
        options->userClusterMasks[i] = Hardware->identity.clusterAvailMask;

    options->userClusterMask = Hardware->identity.clusterAvailMask;

    status = gckOS_QueryOption(Hardware->os, "userClusterMasks",
                               (gctUINT64 *)options->userClusterMasks);

    if (gcmIS_SUCCESS(status)) {
        for (i = 0; i < gcdMAX_MAJOR_CORE_COUNT; i++)
            options->userClusterMasks[i] &= Hardware->identity.clusterAvailMask;

        options->userClusterMask = options->userClusterMasks[Hardware->core];
    }

    options->secureMode = gcvSECURE_NONE;

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY)) {
        gcmkASSERT(gcvSTATUS_TRUE == gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB));

        options->secureMode = gcvSECURE_IN_NORMAL;

        status = gckOS_QueryOption(Hardware->os, "TA", &data);

        if (gcmIS_SUCCESS(status) && data)
            options->secureMode = gcvSECURE_IN_TA;
    }
    else if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB))
        options->secureMode = gcvSECURE_IN_NORMAL;

    options->hasShader = database->NumShaderCores;

    /* By default, there is no AXI SRAM. */
    options->extSRAMCount = 0;

    options->vidMemCount = 0;

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NN_SUPPORT_EFUSE)) {
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GCREG_HI_CHIP_SPECS4_Address, &value));
        options->activeNNCoreCount = gcmGETFIELD(value, GCREG_HI_CHIP_SPECS4, NN_CONV_CORE_COUNT);
        if (!options->activeNNCoreCount)
            options->activeNNCoreCount = database->NN_ACTIVE_CORE_COUNT;
    } else {
        options->activeNNCoreCount = database->NN_ACTIVE_CORE_COUNT;
    }

    for (i = 0; i < gcdSYSTEM_RESERVE_COUNT; i++) {
        if (device->contiguousSizes[i])
            options->vidMemCount++;
    }

    return;
}

/*
 * State timer helper must be called with powerMutex held.
 */
static void
gckSTATETIMER_Reset(gcsSTATETIMER *StateTimer, gctUINT64 Start)
{
    gctUINT64 now;

    if (Start)
        now = Start;
    else
        gckOS_GetProfileTick(&now);

    StateTimer->recent = now;
    StateTimer->start  = now;

    gckOS_ZeroMemory(StateTimer->elapse, gcmSIZEOF(StateTimer->elapse));
}

gceSTATUS
gckHARDWARE_StartTimerReset(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER();

    gckSTATETIMER_Reset(&Hardware->powerStateCounter, 0);

    gcmkFOOTER();
    return status;
}

static void
gckSTATETIMER_Accumulate(gcsSTATETIMER *StateTimer, gceCHIPPOWERSTATE OldState)
{
    gctUINT64 now;
    gctUINT64 elapse;

    gckOS_GetProfileTick(&now);

    elapse = now - StateTimer->recent;

    StateTimer->recent = now;

    StateTimer->elapse[OldState] += elapse;
}

static void
gckSTATETIMER_Query(gcsSTATETIMER *StateTimer, gceCHIPPOWERSTATE State,
                    gctUINT64_PTR On, gctUINT64_PTR Off,
                    gctUINT64_PTR Idle, gctUINT64_PTR Suspend)
{
    gckSTATETIMER_Accumulate(StateTimer, State);

    *On = StateTimer->elapse[gcvPOWER_ON];
    *Off = StateTimer->elapse[gcvPOWER_OFF];
    *Idle = StateTimer->elapse[gcvPOWER_IDLE];
    *Suspend = StateTimer->elapse[gcvPOWER_SUSPEND];
}

static gceSTATUS
_InitPageTableArray(gckHARDWARE Hardware)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
        gcePOOL pool = gcvPOOL_DEFAULT;
        gctUINT32 flags = gcvALLOC_FLAG_CONTIGUOUS;

#if gcdENABLE_CACHEABLE_COMMAND_BUFFER
        flags |= gcvALLOC_FLAG_CACHEABLE;
#endif

        if (Hardware->largeVAVersion && !Hardware->graphicsLargeVA)
            flags |= gcvALLOC_FLAG_32BIT_VA;

        if (pool == gcvPOOL_EXTERNAL_SRAM)
            Hardware->pagetableArray.size = 1024;
        else
            Hardware->pagetableArray.size = gcdMMU_DESC_SIZE;

        /* Allocate mmu table array within 32bit space */
        gcmkONERROR(gckKERNEL_AllocateVideoMemory(Hardware->kernel, 64,
                                                  gcvVIDMEM_TYPE_COMMAND, flags,
                                                  &Hardware->pagetableArray.size, &pool,
                                                  &Hardware->pagetableArray.videoMem));

        /* Lock for kernel side CPU access. */
        gcmkONERROR(gckVIDMEM_NODE_LockCPU(Hardware->kernel,
                                           Hardware->pagetableArray.videoMem,
                                           gcvFALSE, gcvFALSE,
                                           &Hardware->pagetableArray.logical));

        /* Get GPU physical address. */
        gcmkONERROR(gckVIDMEM_NODE_GetGPUPhysical(Hardware->kernel,
                                                  Hardware->pagetableArray.videoMem, 0,
                                                  &Hardware->pagetableArray.address));
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (Hardware->pagetableArray.videoMem) {
        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Hardware->kernel,
                                                 Hardware->pagetableArray.videoMem));
    }

    gcmkFOOTER();
    return status;
}

static gceSTATUS
_SetupSRAMVidMem(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT i;

    for (i = gcvSRAM_INTERNAL0; i < gcvSRAM_INTER_COUNT; i++) {
        if (Hardware->identity.sRAMSizes[i] &&
            Hardware->identity.sRAMBases[i] != gcvINVALID_PHYSICAL_ADDRESS) {
            /* If the internal SRAM usage is memory block. */
            status = gckVIDMEM_Construct(Hardware->os,
                                         Hardware->identity.sRAMBases[i],
                                         Hardware->identity.sRAMSizes[i],
                                         64, 0,
                                         &Hardware->sRAMVidMem[i]);

            if (gcmIS_ERROR(status)) {
                Hardware->identity.sRAMSizes[i] = 0;
                Hardware->sRAMVidMem[i] = gcvNULL;
            } else {
                char sRAMName[32];
                gctUINT64 data = 0;
                gctBOOL sRAMRequested;

                gcmkSPRINTF(sRAMName, gcmSIZEOF(sRAMName) - 1, "gcCore%dSRAM%d", Hardware->core, i);
                status = gckOS_QueryOption(Hardware->os, "sRAMRequested", (gctUINT64 *)&data);
                sRAMRequested = (status == gcvSTATUS_OK) ? (data != 0) : gcvFALSE;

                gcmkONERROR(gckOS_RequestReservedMemory(Hardware->os,
                                                        Hardware->identity.sRAMBases[i],
                                                        Hardware->identity.sRAMSizes[i],
                                                        sRAMName,
                                                        sRAMRequested,
                                                        &Hardware->sRAMPhysical[i]));

                Hardware->sRAMVidMem[i]->physical = Hardware->sRAMPhysical[i];
            }
        }
    }

OnError:
    return status;
}

/******************************************************************************
 ****************************** gckHARDWARE API code **************************
 ******************************************************************************/

/*******************************************************************************
 **
 **  gckHARDWARE_Construct
 **
 **  Construct a new gckHARDWARE object.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to an initialized gckOS object.
 **
 **      gckKERNEL Kernel
 **          Specified core.
 **
 **  OUTPUT:
 **
 **      gckHARDWARE *Hardware
 **          Pointer to a variable that will hold the pointer to the gckHARDWARE
 **          object.
 */
gceSTATUS
gckHARDWARE_Construct(gckOS Os, gckKERNEL Kernel, gckHARDWARE *Hardware)
{
    gceSTATUS status;
    gckHARDWARE hardware = gcvNULL;
    gctUINT16 data = 0xff00;
    gctPOINTER pointer = gcvNULL;
    gctUINT i;
    gctUINT64 enableSoftReset = 1;

    gcmkHEADER_ARG("Os=0x%x", Os);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Os, gcvOBJ_OS);
    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    /* Allocate the gckHARDWARE object. */
    gcmkONERROR(gckOS_Allocate(Os, gcmSIZEOF(struct _gckHARDWARE), &pointer));

    gckOS_ZeroMemory(pointer, gcmSIZEOF(struct _gckHARDWARE));

    hardware = (gckHARDWARE)pointer;

    /* Initialize the gckHARDWARE object. */
    hardware->object.type = gcvOBJ_HARDWARE;
    hardware->os = Os;
    hardware->core = Kernel->core;
    hardware->kernel = Kernel;

    /* Enable the GPU. */
    gcmkONERROR(gckOS_SetGPUPower(Os, Kernel, gcvTRUE, gcvTRUE));
    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel,
                                      AQ_HI_CLOCK_CONTROL_Address,
                                      AQ_HI_CLOCK_CONTROL_ResetValue));

    /* Power state timer reset. */
    gcmkONERROR(gckHARDWARE_StartTimerReset(hardware));

    gcmkONERROR(_GetHardwareSignature(hardware, Os, &hardware->signature));

    /* Identify the hardware. */
    gcmkONERROR(_IdentifyHardwareByDatabase(hardware, Os, Kernel->device, &hardware->identity));

    /* Determine the hardware type */
    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_PIPE_3D) &&
        gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_PIPE_2D)) {
        hardware->type = gcvHARDWARE_3D2D;
    }
    else if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_PIPE_2D)) {
        hardware->type = gcvHARDWARE_2D;
}
    else if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_AI_GPU)) {
        hardware->type = gcvHARDWARE_3D;
    }
    else if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_PIPE_3D) &&
        gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_TC_SHADER_TRIGGER_NN)) {
        hardware->type = gcvHARDWARE_3D;
    }
    else if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_NN_ENGINE) ||
        gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_TP_ENGINE)) {
        hardware->type = gcvHARDWARE_VIP;
    }
    else {
        hardware->type = gcvHARDWARE_3D;
    }

#if gcdMMU_VERSION_2
    if (hardware->identity.virtualAddressBits != gcdVA_BITS) {
        gcmkPRINT("ftg340 error: feature database doesn't match with gcdVA_BITS, feature bit VIRTUAL_ADDRESS_BITS=%x gcdVA_BITS=%x.\n",
                                  hardware->identity.virtualAddressBits, gcdVA_BITS);
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }
#elif gcdENABLE_40BIT_VA
    if (hardware->identity.virtualAddressBits != 40) {
        /* largeVAVersion shouldn't be 0, to trigger kernel driver add 32bit flag when allocate buffer. */
        if (hardware->type == gcvHARDWARE_VIP) {
            hardware->largeVAVersion = gcv40BIT_VA_32BIT_PA;
            hardware->graphicsLargeVA = gcvFALSE;
        } else {
            gcmkPRINT("ftg340 error: Enable 40bit virtual address on a wrong chip.\n");
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }
    }
#endif

    /* Setup SRAM memory heap. */
    gcmkONERROR(_SetupSRAMVidMem(hardware));

    _SetHardwareOptions(hardware);

    gcmkONERROR(gckOS_SetGPUPowerEx(Os, Kernel, gcvTRUE, gcvTRUE, hardware->options.userClusterMask));

    hardware->hasQchannel = gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_Q_CHANNEL_SUPPORT);

    /* Bypass Qchannel power management. */
    if (!hardware->options.powerManagement &&
        hardware->hasQchannel) {
        gcmkONERROR(gckHARDWARE_QchannelBypass(hardware, gcvTRUE));
    }

    status = gckOS_QueryOption(Os, "softReset", (gctUINT64 *)&enableSoftReset);
    if (enableSoftReset == 1) {
        /* _ResetGPU need powerBaseAddress. */
        status = _ResetGPU(hardware, gcvTRUE);

        if (status != gcvSTATUS_OK) {
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                           "_ResetGPU failed: status=%d\n", status);
        }
    }

#if gcdDEC_ENABLE_AHB
    gcmkONERROR(gckOS_WriteRegisterEx(Os, gcvCORE_DEC,
                                      GCREG_DEC_CONTROL_Address,
                                      gcmSETFIELDVALUE(0, GCREG_DEC_CONTROL,
                                                       CONFIGURE_MODE, LOAD_STATE)));
#endif

    hardware->hasL2Cache = gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_64K_L2_CACHE);

    if (!hardware->hasL2Cache) {
        gcmkONERROR(gckOS_WriteRegisterEx(Os, hardware->kernel, GCREG_CONTROL1_Address, 0x00FFFFFF));
    }

    /* Determine whether bug fixes #1 are present. */
    hardware->extraEventStates =
        (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_BUG_FIXES1) == gcvFALSE);

    /* Check if big endian */
    hardware->bigEndian = (*(gctUINT8 *)&data == 0xff);

    /* Initialize the fast clear. */
    gcmkONERROR(gckHARDWARE_SetFastClear(hardware, -1, -1));

#if !gcdENABLE_128B_MERGE && defined(GCREG_CONTROL0_Address)

    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_MULTI_SOURCE_BLT)) {
        /* 128B merge is turned on by default. Disable it. */
        gcmkONERROR(gckOS_WriteRegisterEx(Os, hardware->kernel, GCREG_CONTROL0_Address, 0));
    }

#endif

#if (gcdFPGA_BUILD && defined(GCREG_CONTROL0_Address))
    if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_TPCV11_COMPRESSION)) {
        gctUINT32 data;

        gcmkONERROR(gckOS_ReadRegisterEx(Os, hardware->kernel, GCREG_CONTROL0_Address, &data));
        data |= 0x1 << 27;
        gcmkONERROR(gckOS_WriteRegisterEx(Os, hardware->kernel, GCREG_CONTROL0_Address, data));
    }
#endif

    {
        gctUINT32 value;

        gcmkONERROR(gckOS_ReadRegisterEx(Os, hardware->kernel, GCREG_CONTROL2_Address, &value));
#if gcdDEC_ENABLE_AHB
        if (gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_DEC300_COMPRESSION))
            value |= ~0xFFFFFFBF;
        else
            value &= 0xFFFFFFBF;
# else
        value &= 0xFFFFFFBF;
# endif
        gcmkONERROR(gckOS_WriteRegisterEx(Os, hardware->kernel, GCREG_CONTROL2_Address, value));
    }

    /* Set power state to ON. */
    hardware->chipPowerState = gcvPOWER_ON;
    hardware->clockState = gcvTRUE;
    hardware->powerState = gcvTRUE;
    hardware->lastWaitLink = ~0U;
    hardware->lastEnd = ~0U;
    hardware->globalSemaphore = gcvNULL;
#if gcdENABLE_FSCALE_VAL_ADJUST
    hardware->powerOnFscaleVal = 64;
    hardware->powerOnShaderFscaleVal = 64;
#endif
#if gcdPOWEROFF_TIMEOUT
    hardware->powerOffTimeout = gcdPOWEROFF_TIMEOUT;
#endif

    gcmkONERROR(gckOS_CreateMutex(Os, &hardware->powerMutex));
    gcmkONERROR(gckOS_CreateSemaphore(Os, &hardware->globalSemaphore));

#if gcdPOWEROFF_TIMEOUT
    gcmkVERIFY_OK(gckOS_CreateTimer(Os, _PowerStateTimerFunc,
                                    (gctPOINTER)hardware,
                                    &hardware->powerStateTimer));
#endif

    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++) {
        gcmkONERROR(gckOS_AtomConstruct(Os,
                                        &hardware->pageTableDirty[i]));
    }

    gcmkONERROR(gckOS_AtomConstruct(Os, &hardware->pendingEvent));
    gcmkONERROR(gckOS_AtomConstruct(Os, &hardware->curClusterMask));
    gcmkONERROR(gckOS_AtomSet(Os, hardware->curClusterMask, hardware->options.userClusterMask));

#if defined(LINUX) || defined(__QNXNTO__) || defined(UNDER_CE)
    hardware->stallFEPrefetch =
        gckHARDWARE_IsFeatureAvailable(hardware, gcvFEATURE_FE_ALLOW_STALL_PREFETCH_ENG);
#else
    hardware->stallFEPrefetch = _QueryFeatureDatabase(hardware, gcvFEATURE_MCFE) == gcvFALSE;
#endif

    hardware->minFscaleValue = 1;
    hardware->waitCount = 200;

#if gcdLINK_QUEUE_SIZE
    gcmkONERROR(gckQUEUE_Allocate(hardware->os, &hardware->linkQueue, gcdLINK_QUEUE_SIZE));
#endif

    /* Initialize FEs, either MCFE or wait-link FE. */
    if (_QueryFeatureDatabase(hardware, gcvFEATURE_MCFE)) {
        hardware->mcfeChannels[0] = gcvMCFE_CHANNEL_SYSTEM;
        hardware->mcfeChannels[1] = gcvMCFE_CHANNEL_SHADER;
        hardware->mcfeChannels[2] = gcvMCFE_CHANNEL_NN;
        hardware->mcfeChannels[3] = gcvMCFE_CHANNEL_TP;

        hardware->mcfeChannelCount = 4;

        gcmkONERROR(gckMCFE_Construct(hardware, &hardware->mcFE));
    } else {
        gcmkONERROR(gckWLFE_Construct(hardware, &hardware->wlFE));
    }

    if (_QueryFeatureDatabase(hardware, gcvFEATURE_ASYNC_BLIT))
        gcmkONERROR(gckASYNC_FE_Construct(hardware, &hardware->asyncFE));

    /* Construct hardware function */
    gcmkONERROR(gckFUNCTION_Construct(hardware));

    /* Return pointer to the gckHARDWARE object. */
    *Hardware = hardware;

    /* Success. */
    gcmkFOOTER_ARG("*Hardware=%p", *Hardware);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (hardware != gcvNULL) {
        /* Turn off the power. */
        gcmkVERIFY_OK(gckOS_SetGPUPower(Os, hardware->kernel, gcvFALSE, gcvFALSE));

        if (hardware->globalSemaphore != gcvNULL) {
            /* Destroy the global semaphore. */
            gcmkVERIFY_OK(gckOS_DestroySemaphore(Os, hardware->globalSemaphore));
        }

        if (hardware->powerMutex != gcvNULL) {
            /* Destroy the power mutex. */
            gcmkVERIFY_OK(gckOS_DeleteMutex(Os, hardware->powerMutex));
        }

#if gcdPOWEROFF_TIMEOUT
        if (hardware->powerStateTimer != gcvNULL) {
            gcmkVERIFY_OK(gckOS_StopTimer(Os, hardware->powerStateTimer));
            gcmkVERIFY_OK(gckOS_DestroyTimer(Os, hardware->powerStateTimer));
        }
#endif

        for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++) {
            if (hardware->pageTableDirty[i] != gcvNULL)
                gcmkVERIFY_OK(gckOS_AtomDestroy(Os, hardware->pageTableDirty[i]));
        }

        if (hardware->pendingEvent != gcvNULL)
            gcmkVERIFY_OK(gckOS_AtomDestroy(Os, hardware->pendingEvent));

        if (hardware->curClusterMask != gcvNULL)
            gcmkONERROR(gckOS_AtomConstruct(Os, &hardware->curClusterMask));

        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Os, hardware));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_PostConstruct(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT i;

    /* Initialize MMU page table array. */
    gcmkONERROR(_InitPageTableArray(Hardware));

    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++) {
        gctBOOL funcValid = gcvFALSE;

        gckFUNCTION_Validate(&Hardware->functions[i], &funcValid);
        if (funcValid)
            gcmkONERROR(gckFUNCTION_Init(&Hardware->functions[i]));
    }

    return gcvSTATUS_OK;

OnError:
    for (i = 0; i < gcvFUNCTION_EXECUTION_NUM; i++)
        gckFUNCTION_Release(&Hardware->functions[i]);

    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_PreDestroy
 **
 **  Prepare destroying an gckHARDWARE object.
 **  This is to destroy resources relevant to other modules such as MMU.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object that needs to be destroyed.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_PreDestroy(gckHARDWARE Hardware)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    if (!Hardware) {
        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

    gcmkVERIFY_OK(gckFUNCTION_Destory(Hardware));

    if (Hardware->pagetableArray.videoMem) {
        gcmkVERIFY_OK(gckVIDMEM_NODE_UnlockCPU(Hardware->kernel,
                                               Hardware->pagetableArray.videoMem,
                                               0, gcvFALSE, gcvFALSE));

        gcmkVERIFY_OK(gckVIDMEM_NODE_Dereference(Hardware->kernel,
                                                 Hardware->pagetableArray.videoMem));

        Hardware->pagetableArray.videoMem = gcvNULL;
        Hardware->pagetableArray.logical = gcvNULL;
    }

    if (Hardware->wlFE) {
        gckWLFE_Destroy(Hardware, Hardware->wlFE);
        Hardware->wlFE = gcvNULL;
    }

    if (Hardware->asyncFE) {
        gckASYNC_FE_Destroy(Hardware, Hardware->asyncFE);
        Hardware->asyncFE = gcvNULL;
    }

    if (Hardware->mcFE) {
        gckMCFE_Destroy(Hardware, Hardware->mcFE);
        Hardware->mcFE = gcvNULL;
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_Destroy
 **
 **  Destroy an gckHARDWARE object.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object that needs to be destroyed.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_Destroy(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT i;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Destroy the power semaphore. */
    gcmkVERIFY_OK(gckOS_DestroySemaphore(Hardware->os, Hardware->globalSemaphore));
    Hardware->globalSemaphore = gcvNULL;

    /* Destroy the power mutex. */
    if (Hardware->powerMutex) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Hardware->os, Hardware->powerMutex));
        Hardware->powerMutex = gcvNULL;
    }

#if gcdPOWEROFF_TIMEOUT
    gcmkVERIFY_OK(gckOS_StopTimer(Hardware->os, Hardware->powerStateTimer));
    gcmkVERIFY_OK(gckOS_DestroyTimer(Hardware->os, Hardware->powerStateTimer));
    Hardware->powerStateTimer = gcvNULL;
#endif

    for (i = 0; i < gcvENGINE_GPU_ENGINE_COUNT; i++) {
        gcmkVERIFY_OK(gckOS_AtomDestroy(Hardware->os, Hardware->pageTableDirty[i]));
        Hardware->pageTableDirty[i] = gcvNULL;
    }

    gcmkVERIFY_OK(gckOS_AtomDestroy(Hardware->os, Hardware->pendingEvent));
    Hardware->pendingEvent = gcvNULL;

#if gcdLINK_QUEUE_SIZE
    gckQUEUE_Free(Hardware->os, &Hardware->linkQueue);
#endif

    /* Mark the object as unknown. */
    Hardware->object.type = gcvOBJ_UNKNOWN;

    /* Free the object. */
    gcmkONERROR(gcmkOS_SAFE_FREE(Hardware->os, Hardware));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_GetType
 **
 **  Get the hardware type.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gceHARDWARE_TYPE *Type
 **          Pointer to a variable that receives the type of hardware object.
 */
gceSTATUS
gckHARDWARE_GetType(gckHARDWARE Hardware, gceHARDWARE_TYPE *Type)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);
    gcmkVERIFY_ARGUMENT(Type != gcvNULL);

    *Type = Hardware->type;

    gcmkFOOTER_ARG("*Type=%d", *Type);
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_InitializeHardware
 **
 **  Initialize the hardware.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_InitializeHardware(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT32 control;
    gctUINT32 data;
    gctUINT32 regPMC = 0;
    gctUINT32 regShaderCtl0 = 0;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &control));

    /* Disable isolate GPU bit. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address,
                                      gcmSETFIELD(control,
                                                  AQ_HI_CLOCK_CONTROL, ISOLATE_GPU, 0)));

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &control));

    /* Enable debug register. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address,
                                      gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL,
                                                  DISABLE_DEBUG_REGISTERS, 0)));

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB) &&
        Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_HI_AHB_CONTROL_Address,
                                          gcmSETFIELDVALUE(0, GCREG_HI_AHB_CONTROL,
                                                           DEBUG_MODE, ENABLE)));
    }

    /* Reset memory counters. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GC_RESET_MEM_COUNTERS_Address, ~0U));

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GC_RESET_MEM_COUNTERS_Address, 0));

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GCREG_AXI_AHB_CONFIG_Address, &data));

    data |= gcmSETFIELD(0, GCREG_AXI_AHB_CONFIG, AWCACHE, 0x3);
    data |= gcmSETFIELD(0, GCREG_AXI_AHB_CONFIG, AXCACHE_OVERRIDE_SHARED, 0x3);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GCREG_AXI_AHB_CONFIG_Address, data));

    data = 0;

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                     &data));

    /* Enable clock gating. */
    data = gcmSETFIELD(data, GC_MODULE_POWER_CONTROLS, ENABLE_MODULE_CLOCK_GATING, 1);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                      data));

    /* Initialize FE. */
    if (Hardware->wlFE)
        gckWLFE_Initialize(Hardware, Hardware->wlFE);
    else if (Hardware->mcFE)
        gckMCFE_Initialize(Hardware, gcvFALSE, Hardware->mcFE);

    if (Hardware->asyncFE)
        gckASYNC_FE_Initialize(Hardware, Hardware->asyncFE);

    if ((gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI2) == gcvSTATUS_FALSE) ||
        Hardware->identity.chipRevision < 0x5422) {
        if (regPMC == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             Hardware->powerBaseAddress +
                                                GC_MODULE_POWER_MODULE_CONTROL_Address,
                                             &regPMC));
        }

        regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                             DISABLE_MODULE_CLOCK_GATING_SH_IO, 1);
    }

    /* AHBDEC400 */
    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_DEC400EX_COMPRESSION)) {
        data = GCREG_AHBDEC_CONTROL_ResetValue;
        data = gcmSETFIELDVALUE(data, GCREG_AHBDEC_CONTROL, DISABLE_COMPRESSION, ENABLE);
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_AHBDEC_CONTROL_Address, data));

        data = GCREG_AHBDEC_CONTROL_EX2_ResetValue;
        data = gcmSETFIELD(data, GCREG_AHBDEC_CONTROL_EX2, TILE_STATUS_READ_ID, 4);
        data = gcmSETFIELD(data, GCREG_AHBDEC_CONTROL_EX2, TILE_STATUS_WRITE_ID, 2);
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_AHBDEC_CONTROL_EX2_Address, data));
    }

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BIT_AXI_FE)) {
        gctUINT32 offset = 0;

        gcmkSAFECASTPHYSADDRT(offset, Hardware->identity.registerAPB);

        gcmkPRINT("Initailize APB1 registers, APB offset is 0x%x.\n", offset);

        /* APB FE ctrl. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          offset + 0x28, 0x2));

        /* APB FE cfg. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          offset + 0x2C, 0x2));
    }

#if !gcdCAPTURE_ONLY_MODE
    gcmkONERROR(gckHARDWARE_SetMMU(Hardware, Hardware->kernel->mmu));
#endif

    if (Hardware->mcFE) {
        /* Reinitialize MCFE, now MMU is enabled. */
        gckMCFE_Initialize(Hardware, gcvTRUE, Hardware->mcFE);
    }

    /* Limit 2D outstanding request. */
    if (Hardware->maxOutstandingReads) {
        gctUINT32 data;

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQ_MEMORY_DEBUG_Address, &data));

        data = gcmSETFIELD(data, AQ_MEMORY_DEBUG, MAX_OUTSTANDING_READS,
                           Hardware->maxOutstandingReads & 0xFF);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, AQ_MEMORY_DEBUG_Address, data));
    }

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NEW_GPIPE) &&
        !gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_GPIPE_CLOCK_GATE_FIX)) {
        if (regPMC == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                             &regPMC));
        }

        /* Disable GPIPE clock gating. */
        regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                             DISABLE_MODULE_CLOCK_GATING_GPIPE, 1);
    }

    if (regPMC == 0) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                         &regPMC));
    }

    /* Disable RA HZ clock gating. */
    regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                         DISABLE_MODULE_CLOCK_GATING_RA_HZ, 1);

    /* Disable RA EZ clock gating. */
    regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                         DISABLE_MODULE_CLOCK_GATING_RA_EZ, 1);

    if ((gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI5) &&
         !gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_RA_CG_FIX))) {
        if (regPMC == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                             &regPMC));
        }

        /* Disable RA clock gating. */
        regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                             DISABLE_MODULE_CLOCK_GATING_RA, 1);
    }


    if ((gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI5) &&
         !gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SH_CLOCK_GATE_FIX)) ||
         gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_D3D11_SUPPORT) ||
         _IsHardwareMatchFlag) {
        if (regPMC == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                             &regPMC));
        }

        /* Disable SH clock gating. */
        regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                             DISABLE_MODULE_CLOCK_GATING_SH, 1);

        _IsHardwareMatchFlag = gcvFALSE;
    }





    if (_IsHardwareMatchFlag) {
        if (regPMC == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                             &regPMC));
        }

        /* Disable SH_EU clock gating. */
        regPMC = gcmSETFIELD(regPMC, GC_MODULE_POWER_MODULE_CONTROL,
                             DISABLE_MODULE_CLOCK_GATING_SH_EU, 1);

        _IsHardwareMatchFlag = gcvFALSE;
    }

    if (regPMC != 0) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                          regPMC));
    }


    if (_IsHardwareMatch(Hardware, 0x8400, 0x7305))
        _IsHardwareMatchFlag = gcvTRUE;


    if (_IsHardwareMatchFlag) {
        if (regShaderCtl0 == 0) {
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os,
                                             Hardware->kernel,
                                             Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL0_Address,
                                             &regShaderCtl0));
        }

        /* Disable pixel input interface's clock only */
        regShaderCtl0 = gcmSETFIELD(regShaderCtl0,
                                    GCREG_AHB_POWER_CONTROL0,
                                    DISABLE_MODULE_CLOCK_GATING_SHPSI,
                                    1);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os,
                                          Hardware->kernel,
                                          Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL0_Address,
                                          regShaderCtl0));

        _IsHardwareMatchFlag = gcvFALSE;
    }

    if (Hardware->identity.chipRevision > 0x5420 &&
        gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PIPE_3D)) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_PULSE_EATER_Address, &data));

        /* Disable internal DFS. */
        data =
#if gcdDVFS
            gcmSETFIELD(data, GC_PULSE_EATER, EXTERNAL_CONTROL, 1) |
#endif
            gcmSETFIELD(data, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH, 0) |
            gcmSETFIELD(data, GC_PULSE_EATER, DISABLE_AUTO_PULSE,   1);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_PULSE_EATER_Address, data));
    }

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NN_ENGINE) &&
        (!gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HI_REORDER_FIX) ||
         Hardware->kernel->device->extSRAMSizes[0] == 0) &&
        (((gcsFEATURE_DATABASE *)Hardware->featureDatabase)->HI_DEFAULT_ENABLE_REORDER_FIX) 
    ) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GCREG_CONTROL2_Address, &data));

        data = gcmSETFIELD(data, GCREG_CONTROL2, DISABLE_AXI_READ_REORDER, 1);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_CONTROL2_Address, data));
    }

    _ConfigurePolicyID(Hardware);

    gcmkONERROR(gckHARDWARE_PowerControlClusters(Hardware,
                                                 Hardware->options.configNNPowerControl,
                                                 gcvTRUE));

#if gcdDEBUG_MODULE_CLOCK_GATING
    _ConfigureModuleLevelClockGating(Hardware);
#endif

    if (_IsHardwareMatch(Hardware, 0x8800, 0x7305) ||
        _IsHardwareMatch(Hardware, 0x8400, 0x7305)) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL2_Address, 0x2));
    }

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_INTR_ENBL_Address, 0xFFFFFFFF));

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BIT_SRAM_PARITY)) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_AHB_INTR_ENBL_Address, 0x1));
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the error. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryMemory
 **
 **  Query the amount of memory available on the hardware.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T *InternalSize
 **          Pointer to a variable that will hold the size of the internal video
 **          memory in bytes.  If 'InternalSize' is gcvNULL, no information of the
 **          internal memory will be returned.
 **
 **      gctADDRESS *InternalBaseAddress
 **          Pointer to a variable that will hold the hardware's base address for
 **          the internal video memory.  This pointer cannot be gcvNULL if
 **          'InternalSize' is also non-gcvNULL.
 **
 **      gctUINT32 *InternalAlignment
 **          Pointer to a variable that will hold the hardware's base address for
 **          the internal video memory.  This pointer cannot be gcvNULL if
 **          'InternalSize' is also non-gcvNULL.
 **
 **      gctSIZE_T *ExternalSize
 **          Pointer to a variable that will hold the size of the external video
 **          memory in bytes.  If 'ExternalSize' is gcvNULL, no information of the
 **          external memory will be returned.
 **
 **      gctADDRESS *ExternalBaseAddress
 **          Pointer to a variable that will hold the hardware's base address for
 **          the external video memory.  This pointer cannot be gcvNULL if
 **          'ExternalSize' is also non-gcvNULL.
 **
 **      gctUINT32 *ExternalAlignment
 **          Pointer to a variable that will hold the hardware's base address for
 **          the external video memory.  This pointer cannot be gcvNULL if
 **          'ExternalSize' is also non-gcvNULL.
 **
 **      gctUINT32 *HorizontalTileSize
 **          Number of horizontal pixels per tile.  If 'HorizontalTileSize' is
 **          gcvNULL, no horizontal pixel per tile will be returned.
 **
 **      gctUINT32 *VerticalTileSize
 **          Number of vertical pixels per tile.  If 'VerticalTileSize' is
 **          gcvNULL, no vertical pixel per tile will be returned.
 */
gceSTATUS
gckHARDWARE_QueryMemory(gckHARDWARE Hardware,
                        gctSIZE_T *InternalSize,
                        gctADDRESS *InternalBaseAddress,
                        gctUINT32 *InternalAlignment,
                        gctSIZE_T *ExternalSize,
                        gctADDRESS *ExternalBaseAddress,
                        gctUINT32 *ExternalAlignment,
                        gctUINT32 *HorizontalTileSize,
                        gctUINT32 *VerticalTileSize)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (InternalSize != gcvNULL) {
        /* No internal memory. */
        *InternalSize = 0;
    }

    if (ExternalSize != gcvNULL) {
        /* No external memory. */
        *ExternalSize = 0;
    }

    if (HorizontalTileSize != gcvNULL) {
        /* 4x4 tiles. */
        *HorizontalTileSize = 4;
    }

    if (VerticalTileSize != gcvNULL) {
        /* 4x4 tiles. */
        *VerticalTileSize = 4;
    }

    /* Success. */
    gcmkFOOTER_ARG("*InternalSize=%lu *InternalBaseAddress=0x%llx *InternalAlignment=0x%08x *ExternalSize=%lu *ExternalBaseAddress=0x%llx *ExtenalAlignment=0x%08x *HorizontalTileSize=%u *VerticalTileSize=%u",
                   gcmOPT_VALUE(InternalSize), gcmOPT_VALUE(InternalBaseAddress),
                   gcmOPT_VALUE(InternalAlignment), gcmOPT_VALUE(ExternalSize),
                   gcmOPT_VALUE(ExternalBaseAddress), gcmOPT_VALUE(ExternalAlignment),
                   gcmOPT_VALUE(HorizontalTileSize), gcmOPT_VALUE(VerticalTileSize));
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryChipIdentity
 **
 **  Query the identity of the hardware.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gcsHAL_QUERY_CHIP_IDENTITY_PTR Identity
 **          Pointer to the identity structure.
 **
 */
gceSTATUS
gckHARDWARE_QueryChipIdentity(gckHARDWARE Hardware, gcsHAL_QUERY_CHIP_IDENTITY_PTR Identity)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Identity != gcvNULL);

    *Identity = Hardware->identity;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryChipOptions
 **
 **  Query the options of the hardware.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gcsHAL_QUERY_CHIP_OPTIONS_PTR Options
 **          Pointer to the identity structure.
 **
 */
gceSTATUS
gckHARDWARE_QueryChipOptions(gckHARDWARE Hardware, gcsHAL_QUERY_CHIP_OPTIONS_PTR Options)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Options != gcvNULL);

    *Options = Hardware->options;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_SplitMemory
 **
 **  Split a hardware specific memory address into a pool and offset.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to the gckHARDWARE object.
 **
 **      gctUINT32 Address
 **          Address in hardware specific format.
 **
 **  OUTPUT:
 **
 **      gcePOOL *Pool
 **          Pointer to a variable that will hold the pool type for the address.
 **
 **      gctUINT32 *Offset
 **          Pointer to a variable that will hold the offset for the address.
 */
gceSTATUS
gckHARDWARE_SplitMemory(gckHARDWARE Hardware, gctUINT32 Address,
                        gcePOOL *Pool, gctUINT32 *Offset)
{
    gcmkHEADER_ARG("Hardware=%p Address=0x%08x", Hardware, Address);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Pool != gcvNULL);
    gcmkVERIFY_ARGUMENT(Offset != gcvNULL);

    *Pool = gcvPOOL_SYSTEM;
    *Offset = Address;

    /* Success. */
    gcmkFOOTER_ARG("*Pool=%d *Offset=0x%08x", *Pool, *Offset);
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_PipeSelect
 **
 **  Append a PIPESELECT command at the specified user logical memory.
 **
 **  WARNING: Only for writing to USERSPACE!
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gctPOINTER Logical
 **          Pointer to the current location inside the command queue to append
 **          the PIPESELECT command at or gcvNULL just to query the size of the
 **          PIPESELECT command.
 **
 **      gcePIPE_SELECT Pipe
 **          Pipe value to select.
 **
 **      gctSIZE_T *Bytes
 **          Pointer to the number of bytes available for the PIPESELECT command.
 **          If 'Logical' is gcvNULL, this argument will be ignored.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T *Bytes
 **          Pointer to a variable that will receive the number of bytes required
 **          for the PIPESELECT command.  If 'Bytes' is gcvNULL, nothing will be
 **          returned.
 */
gceSTATUS
gckHARDWARE_PipeSelect(gckHARDWARE Hardware, gctPOINTER Logical,
                       gcePIPE_SELECT Pipe, gctUINT32 *Bytes)
{
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x Pipe=%d *Bytes=0x%x",
                   Hardware, Logical, Pipe, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    /* Append a PipeSelect. */
    if (Logical != gcvNULL) {
        gctUINT32 flush, stall;

        if (*Bytes < 32) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        flush = (Pipe == gcvPIPE_2D) ?
                gcmSETFIELDVALUE(0, AQ_FLUSH, CCACHE, ENABLE) |
                    gcmSETFIELDVALUE(0, AQ_FLUSH, ZCACHE, ENABLE) :
                gcmSETFIELDVALUE(0, AQ_FLUSH, PE2D_CACHE, ENABLE);

        stall = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);

        /* LoadState(AQFlush, 1), flush. */
        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical,
                                      gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQFlushRegAddrs) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1)));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 1, flush));

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "%p: FLUSH 0x%x", logical, flush);

        /* LoadState(AQSempahore, 1), stall. */
        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 2,
                                      gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs)));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 3, stall));

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "%p: SEMAPHORE 0x%x", logical + 2, stall);

        /* Stall, stall. */
        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 4,
                                      gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL)));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 5, stall));

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "0x%x: STALL 0x%x", logical + 4, stall);

        /* LoadState(AQPipeSelect, 1), pipe. */
        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 6,
                                      gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQPipeSelectRegAddrs) |
                                           gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1)));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 7,
                                      (Pipe == gcvPIPE_2D) ?
                                      AQ_PIPE_SELECT_PIPE_PIPE2D : AQ_PIPE_SELECT_PIPE_PIPE3D));

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "0x%x: PIPE %d", logical + 6, Pipe);
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the PIPESELECT command. */
        *Bytes = 32;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_FenceRender(gckHARDWARE Hardware, gctPOINTER Logical,
             gctADDRESS FenceAddress, gctUINT64 FenceData, gctUINT32 *Bytes)
{
    gckOS os = Hardware->os;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;

    gctUINT32 dataLow = (gctUINT32)FenceData;
    gctUINT32 dataHigh = (gctUINT32)(FenceData >> 32);

    if (logical) {
        gctUINT32 fenceAddress;

        gcmkSAFECASTVA(fenceAddress, FenceAddress);

        if (Hardware->graphicsLargeVA) {
            gcmkWRITE_MEMORY(logical,
                             gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFenceAddressHiRegAddrs) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

            gcmkWRITE_MEMORY(logical, (gctUINT32)(FenceAddress >> 32));
        }


        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFenceAddressRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, fenceAddress);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFenceDataHighRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, dataHigh);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFenceDataRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, dataLow);
    }

    if (Bytes)
        *Bytes = (Hardware->graphicsLargeVA) ? gcdRENDER_FENCE_LENGTH + 8 : gcdRENDER_FENCE_LENGTH;

    return gcvSTATUS_OK;
}

static gceSTATUS
_FenceBlt(gckHARDWARE Hardware, gctPOINTER Logical,
          gctADDRESS FenceAddress, gctUINT64 FenceData, gctUINT32 *Bytes)
{
    gckOS os = Hardware->os;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;

    gctUINT32 dataLow = (gctUINT32)FenceData;
    gctUINT32 dataHigh = (gctUINT32)(FenceData >> 32);

    if (logical) {
        gctUINT32 fenceAddress;

        gcmkSAFECASTVA(fenceAddress, FenceAddress);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK));


        if (Hardware->graphicsLargeVA) {
            gcmkWRITE_MEMORY(logical,
                             gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltFenceAddressHiRegAddrs) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

            gcmkWRITE_MEMORY(logical, (gctUINT32)(FenceAddress >> 32));
        }

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltFenceAddressRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, fenceAddress);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltFenceDataHighRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, dataHigh);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltFenceDataRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical, dataLow);

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1));

        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK));
    }

    if (Bytes)
        *Bytes = (Hardware->graphicsLargeVA) ? gcdBLT_FENCE_LENGTH + 8 : gcdBLT_FENCE_LENGTH;

    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_Fence
 **
 **  Append a HW FENCE states at the specified user logical memory.
 **
 **  WARNING: Only for writing to USERSPACE!
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gceENGINE Engine
 **          Engine type, render or 3d-blit currently.
 **
 **
 **      gctPOINTER Logical
 **          Pointer to the current location inside the command queue to append
 **          the PIPESELECT command at or gcvNULL just to query the size of the
 **          PIPESELECT command.
 **
 **      gctADDRESS FenceAddress
 **          GPU address to write out the data.
 **
 **      gctUINT64 FenceData
 **          The 64bit data to write out.
 **
 **      gctSIZE_T *Bytes
 **          Pointer to the number of bytes available for the PIPESELECT command.
 **          If 'Logical' is gcvNULL, this argument will be ignored.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T *Bytes
 **          Pointer to a variable that will receive the number of bytes required
 **          for the PIPESELECT command.  If 'Bytes' is gcvNULL, nothing will be
 **          returned.
 */
gceSTATUS
gckHARDWARE_Fence(gckHARDWARE Hardware, gceENGINE Engine, gctPOINTER Logical,
                  gctADDRESS FenceAddress, gctUINT64 FenceData, gctUINT32 *Bytes)
{
    if (Engine == gcvENGINE_RENDER)
        return _FenceRender(Hardware, Logical, FenceAddress, FenceData, Bytes);
    else
        return _FenceBlt(Hardware, Logical, FenceAddress, FenceData, Bytes);
}

/*******************************************************************************
 **
 **  gckHARDWARE_UpdateQueueTail
 **
 **  Update the tail of the command queue.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gctPOINTER Logical
 **          Logical address of the start of the command queue.
 **
 **      gctUINT32 Offset
 **          Offset into the command queue of the tail (last command).
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_UpdateQueueTail(gckHARDWARE Hardware, gctPOINTER Logical, gctUINT32 Offset)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=%p Logical=%p Offset=0x%08x",
                   Hardware, Logical, Offset);

    /* Verify the hardware. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Force a barrier. */
    gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, Logical));

    /* Notify gckKERNEL object of change. */
    gcmkONERROR(gckKERNEL_Notify(Hardware->kernel, gcvNOTIFY_COMMAND_QUEUE));

    if (status == gcvSTATUS_CHIP_NOT_READY)
        gcmkONERROR(gcvSTATUS_DEVICE);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static void
_ResumeWaitLinkFE(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctADDRESS address;
    gctUINT32 addressLow, addressHi = 0;
    gctUINT32 bytes;
    gctUINT32 idle;

    /* Make sure FE is idle. */
    do {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         AQ_HI_IDLE_Address, &idle));
    } while (idle != 0x7FFFFFFF);

    gcmkDUMP(Hardware->os,
             "@[register.wait 0x%05X 0x%08X 0x%08X]",
             AQ_HI_IDLE_Address,
             gcmSETFIELD(0, AQ_HI_IDLE, IDLE_FE, ~0U),
             idle);

    /* Read the current FE address. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));
    address = addressLow;

    if (Hardware->graphicsLargeVA) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

        address = ((gctADDRESS)addressHi << 32) | addressLow;
    }

    bytes = Hardware->hasL2Cache ? 24 : 16;

    /* Start Command Parser. */
    gckWLFE_Execute(Hardware, address, bytes);

OnError:
    return;
}

gceSTATUS
gckHARDWARE_ResumeWLFE(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;

    _ResumeWaitLinkFE(Hardware);
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_Interrupt
 **
 **  Process an interrupt. This function will read the interrupt acknowledge
 **  register, stores the data, and return whether the interrupt is from us.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_Interrupt(gckHARDWARE Hardware)
{
    gctUINT32 data = 0;
    gctUINT32 dataEx = 0;
    gceSTATUS status;
    gceSTATUS statusEx;

    /*
     * Notice:
     * In isr here.
     * We should return success when either FE or AsyncFE reports correct
     * interrupts, so that isr can wake up threadRoutine for either FE.
     * That means, only need return ERROR when both FEs reports ERROR.
     */
    /* Read AQIntrAcknowledge register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_INTR_ACKNOWLEDGE_Address, &data));

    if (data == 0) {
        /* Not our interrupt. */
        status = gcvSTATUS_NOT_OUR_INTERRUPT;
    } else {
#if gcdINTERRUPT_STATISTIC
        gckOS_AtomClearMask(Hardware->pendingEvent, data);
#endif

        /* Inform gckEVENT of the interrupt. */
        status = gckEVENT_Interrupt(Hardware->kernel->eventObj, data);
    }

    if (Hardware->kernel->parityEvent) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GCREG_AHB_INTR_STATUS_Address, &data));

        if (data)
           status = gckEVENT_Interrupt(Hardware->kernel->parityEvent, data);
    }

    if (!Hardware->asyncFE) {
        /* Done. */
        goto OnError;
    }

    /* Read BLT interrupt. */
    statusEx = gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                    AQ_INTR_ACKNOWLEDGE_EX_Address, &dataEx);

    if (gcmIS_ERROR(statusEx)) {
        /*
         * Do not overwrite status here, so that former status from
         * AQIntrAck is returned.
         */
        goto OnError;
    }

    /*
     * This bit looks useless now, we can use this check if this interrupt is
     * from FE.
     */
    dataEx &= ~0x80000000;

    /*
     * Descriptor fetched, update counter.
     * We can't do this at dataEx != 0 only, because read HW acknowledge
     * register will overwrite GCREG_FE_ASYNC_STATUS_Address. If one
     * interrupt we don't read it, we will miss it for ever.
     */
    gckASYNC_FE_UpdateAvaiable(Hardware);

    /* Do not need report NOT_OUT_INTERRUPT error if dataEx is 0. */
    if (dataEx) {
        statusEx = gckEVENT_Interrupt(Hardware->kernel->asyncEvent, dataEx);

        if (gcmIS_SUCCESS(statusEx)) {
            /* At least AsyncFE is success, treat all as success. */
            status = gcvSTATUS_OK;
        }
    }

OnError:
    /* Return the status. */
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_Notify
 **
 **  This functions will handle the event notifications.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_Notify(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gceEVENT_FAULT fault;
    gctUINT32 pending;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gckOS_AtomGet(Hardware->os,
                  Hardware->kernel->eventObj->pending,
                  (gctINT32_PTR)&pending);

    gckOS_AtomSetMask(Hardware->kernel->eventObj->pending, pending);

    /* Handle events. */
    gcmkONERROR(gckEVENT_Notify(Hardware->kernel->eventObj, 0, &fault));

    if (Hardware->kernel->parityEvent)
        gcmkONERROR(gckEVENT_ParityNotify(Hardware->kernel->parityEvent));

    if (Hardware->asyncFE)
        gcmkONERROR(gckEVENT_Notify(Hardware->kernel->asyncEvent, 0, &fault));

    if (fault & gcvEVENT_BUS_ERROR_FAULT)
        status = gckKERNEL_Recovery(Hardware->kernel);

OnError:
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryCommandBuffer
 **
 **  Query the command buffer alignment and number of reserved bytes.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T *Alignment
 **          Pointer to a variable receiving the alignment for each command.
 **
 **      gctSIZE_T *ReservedHead
 **          Pointer to a variable receiving the number of reserved bytes at the
 **          head of each command buffer.
 **
 **      gctSIZE_T *ReservedTail
 **          Pointer to a variable receiving the number of bytes reserved at the
 **          tail of each command buffer.
 */
gceSTATUS
gckHARDWARE_QueryCommandBuffer(gckHARDWARE Hardware, gceENGINE Engine,
                               gctUINT32 *Alignment, gctUINT32 *ReservedHead, gctUINT32 *ReservedTail)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (Alignment != gcvNULL) {
        /* Align every 8 bytes. */
        if(Hardware->graphicsLargeVA)
            *Alignment = 16;
        else
            *Alignment = 8;
    }

    if (ReservedHead != gcvNULL) {
        /* Reserve space for SelectPipe(). */
        *ReservedHead = 32;
    }

    if (ReservedTail != gcvNULL) {
        if (Engine == gcvENGINE_RENDER) {
            gcmkFOOTER_NO();
            return gcvSTATUS_NOT_SUPPORTED;
        } else {
            *ReservedTail = gcdBLT_FENCE_LENGTH;
        }
    }

    /* Success. */
    gcmkFOOTER_ARG("*Alignment=0x%x *ReservedHead=0x%x *ReservedTail=0x%x",
                   gcmOPT_VALUE(Alignment), gcmOPT_VALUE(ReservedHead), gcmOPT_VALUE(ReservedTail));
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QuerySystemMemory
 **
 **  Query the command buffer alignment and number of reserved bytes.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gctSIZE_T *SystemSize
 **          Pointer to a variable that receives the maximum size of the system
 **          memory.
 **
 **      gctUINT32 *SystemBaseAddress
 **          Poinetr to a variable that receives the base address for system
 **          memory.
 */
gceSTATUS
gckHARDWARE_QuerySystemMemory(gckHARDWARE Hardware, gctSIZE_T *SystemSize, gctUINT32 *SystemBaseAddress)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    if (SystemSize != gcvNULL) {
        /* Maximum system memory can be 2GB. */
        *SystemSize = 1U << 31;
    }

    if (SystemBaseAddress != gcvNULL) {
        /* Set system memory base address. */
        *SystemBaseAddress = gcmSETFIELDVALUE(0, AQ_MEMORY_ADDRESS, TYPE, SYSTEM);
    }

    /* Success. */
    gcmkFOOTER_ARG("*SystemSize=0x%zx *SystemBaseAddress=0x%x",
                   gcmOPT_VALUE(SystemSize), gcmOPT_VALUE(SystemBaseAddress));
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_SetMMU
 **
 **  Set the page table base address.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **      gckMMU Mmu
 **          Pointer to mmu object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_SetMMU(gckHARDWARE Hardware, gckMMU Mmu)
{
    gceSTATUS status;

    gctBOOL mmuValid = gcvTRUE;

    gcmkHEADER_ARG("Hardware=%p Mmu=%p", Hardware, Mmu);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(
        gckFUNCTION_Validate(&Hardware->functions[gcvFUNCTION_EXECUTION_MMU], &mmuValid));

    if (mmuValid) {
        gctBOOL hwMmuDisabled = gcvTRUE;

        /* Force Disable MMU to guarantee setup command be read from physical addr */
        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gctUINT32 regMmuCtrl = 0;

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             GCREG_MMUAHB_CONTROL_Address, &regMmuCtrl));

            hwMmuDisabled = (gcmGETFIELD(regMmuCtrl, GCREG_MMUAHB_CONTROL, MMU) ==
                                GCREG_MMUAHB_CONTROL_MMU_ENABLE) ?
                            gcvFALSE : gcvTRUE;
        } else {
            gctUINT32 regMmuCtrl = 0;

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             GCREG_MMU_CONTROL_Address, &regMmuCtrl));

            hwMmuDisabled = (gcmGETFIELD(regMmuCtrl, GCREG_MMU_CONTROL, ENABLE) ==
                                GCREG_MMU_CONTROL_ENABLE_ENABLE) ?
                            gcvFALSE : gcvTRUE;
        }

        if (hwMmuDisabled) {
            gcmkONERROR(gckFUNCTION_Execute(&Hardware->functions[gcvFUNCTION_EXECUTION_MMU]));

            /* Enable MMU. */
            if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
                gctUINT32 config;

                if (Mmu->initMode == gcvMMU_INIT_FROM_REG) {
                    config = gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTROL, MMU,        ENABLE) |
                             gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTROL, SET_UP_MMU, FROM_REG);
                } else {
                    config = gcmSETFIELDVALUE(0, GCREG_MMUAHB_CONTROL, MMU, ENABLE);
                }

                gcmkONERROR(gckOS_WriteRegisterEx_NoDump(Hardware->os, Hardware->kernel,
                                                         GCREG_MMUAHB_CONTROL_Address, config));
            } else {
                gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                                  GCREG_MMU_CONTROL_Address,
                                                  gcmSETFIELD(0, GCREG_MMU_CONTROL, ENABLE, gcvTRUE)));
            }
        }
    }

    /* Return the status. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_FlushMMU
 **
 **  Flush the page table.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_FlushMMU(gckHARDWARE Hardware,
                     gctPOINTER Logical, gctADDRESS Address,
                     gctUINT32 SubsequentBytes, gctUINT32 *Bytes)
{
    gceSTATUS status;
    gctUINT32_PTR buffer;
    gctUINT32 flushSize;
    gctBOOL bltEngine = gcvFALSE;
    gctBOOL multiCluster = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    flushSize = 10 * 4;

    bltEngine = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE);
    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    if (bltEngine) {
        flushSize += 4 * 4;

        if (multiCluster)
            flushSize += 2 * 4;
    }

    if (Hardware->graphicsLargeVA)
        flushSize += 4 * 4;

    if (Logical) {
        /* Flush the memory controller. */
        gctUINT32 count;
        gctUINT32 offset = 2;
        gctUINT32 prefetchCount = 4;
        gctUINT32 semaphore, stall;
        gctUINT32 address = 0;
#if gcdMMU_VERSION_2
        gckMMU mmu;
#endif

        if (*Bytes < flushSize)
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);

        if (bltEngine) {
            prefetchCount += 2;
            if (multiCluster)
                prefetchCount++;
        }

        buffer = (gctUINT32_PTR)Logical;

        count = SubsequentBytes >> 3;

        address = (gctUINT32)Address;

        if (Hardware->graphicsLargeVA) {
            gctUINT64_PTR opLogical = (gctUINT64_PTR)Logical;

            offset = 4;

            /* LINK to next slot to flush FE FIFO. */
            *opLogical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,   LINK64) |
                             gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND,       PREFETCH, prefetchCount + 1);

            *opLogical++ = Address + offset * gcmSIZEOF(gctUINT32);

            buffer = (gctUINT32_PTR)opLogical;
        } else {
            /* LINK to next slot to flush FE FIFO. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,   LINK) |
                         gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND,       PREFETCH, prefetchCount);

            *buffer++ = address + offset * gcmSIZEOF(gctUINT32);
        }

#if gcdMMU_VERSION_2
        gcmkONERROR(gckKERNEL_GetCurrentMMU(Hardware->kernel, gcvTRUE, 0, &mmu));

        /* Flush MMU VMID. */
        *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) |
            gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUPTFlushRegAddrs) |
            gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, 1);

        *buffer++ = gcmSETFIELD(0, GCREG_MMUPT_FLUSH, FLUSH_CMD, 1) |
                    gcmSETFIELD(0, GCREG_MMUPT_FLUSH, FLUSH_VMID, 0);
#else
        /* Flush MMU cache. */
        *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, 1);

        *buffer++ = gcmSETMASKEDFIELDVALUE(GCREG_MMU_CONFIGURATION, FLUSH, FLUSH);
#endif

        if (bltEngine) {
            /* Blt lock. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *buffer++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

            if (multiCluster) {
                gctUINT32 clusterMask;

                gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltClusterControlRegAddrs);

                *buffer++ = gcmSETFIELD(0, GCREG_BLT_CLUSTER_CONTROL, CLUSTER_ENABLE,
                                        clusterMask);
            }
        }

        /* Arm the PE-FE Semaphore. */
        *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

        semaphore = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END);

        if (Hardware->stallFEPrefetch)
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, FRONT_END, PREFETCH);

        if (bltEngine)
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, BLT_ENGINE);
        else
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);

        *buffer++ = semaphore;

        /* STALL FE until PE is done flushing. */
        *buffer++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

        stall = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END);

        if (Hardware->stallFEPrefetch)
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, FRONT_END, PREFETCH);

        if (bltEngine)
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, BLT_ENGINE);
        else
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE);

        *buffer++ = stall;

        if (bltEngine) {
            /* Blt unlock. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *buffer++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
        }

        if (Hardware->graphicsLargeVA) {
            gctUINT64_PTR opLogical = (gctUINT64_PTR)buffer;

            offset = 4;

            /* LINK to next slot to flush FE FIFO. */
            *opLogical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,   LINK64) |
                             gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND,       PREFETCH, count);

            *opLogical = Address + flushSize;
        } else {
            /* LINK to next slot to flush FE FIFO. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,   LINK) |
                         gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND,       PREFETCH, count);

            *buffer++ = address + flushSize;
        }
    }

    if (Bytes)
        *Bytes = flushSize;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_FlushAsyncMMU(gckHARDWARE Hardware, gctPOINTER Logical, gctUINT32 *Bytes)
{
    gctUINT32 semaphore, stall;
    gctUINT32_PTR buffer;
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x *Bytes=0x%x",
                   Hardware, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    if (Logical != gcvNULL) {
        buffer = (gctUINT32_PTR)Logical;

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer,
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1)));

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer + 1,
            gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK)));

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer + 2,
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1)));

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer + 3,
            gcmSETMASKEDFIELDVALUE(GCREG_MMU_CONFIGURATION, FLUSH, FLUSH)));

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer + 4,
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs)));

        semaphore = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                    gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, BLT_ENGINE);

        if (Hardware->stallFEPrefetch)
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, FRONT_END, PREFETCH);

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, buffer + 5, semaphore));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, buffer + 6,
                                      gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL)));

        stall = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
                gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, BLT_ENGINE);

        if (Hardware->stallFEPrefetch)
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, FRONT_END, PREFETCH);

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, buffer + 7, stall));

        gcmkONERROR(gckOS_WriteMemory(
            Hardware->os, buffer + 8,
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1)));

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, buffer + 9,
                                      gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK)));
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the PIPESELECT command. */
        *Bytes = 40;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return status;
}

gceSTATUS
gckHARDWARE_FlushMcfeMMU(gckHARDWARE Hardware, gctPOINTER Logical, gctUINT32 *Bytes)
{
#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
    gceSTATUS status;
    gctUINT32_PTR buffer;
    gctUINT32 flushSize;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    flushSize = 4 * 6;

    if (Logical) {
        if (*Bytes < flushSize)
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);

        buffer = (gctUINT32_PTR)Logical;

        /* Flush MMU cache. */
        *buffer++ =
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigurationRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *buffer++ = gcmSETMASKEDFIELDVALUE(GCREG_MMU_CONFIGURATION, FLUSH, FLUSH);

        /*
         * System channel can only take one command at a time, Trigger and
         * SubmitJob are not required.
         */

        /* AQHiIdle to trigger. */
        *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQHiIdleRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *buffer++ = 0;

        /* SubmitJob. */
        *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, SUB_COMMAND) |
                    gcmSETFIELDVALUE(0, MCFE_COMMAND, SUB_OPCODE, SUBMIT_JOB);

        *buffer++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
    }

    if (Bytes)
        *Bytes = flushSize;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:

    /* Return the status. */
    gcmkFOOTER();
    return status;
#else
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

/*******************************************************************************
 **
 **  gckHARDWARE_BuildVirtualAddress
 **
 **  Build a virtual address.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **      gctUINT32 Index
 **          Index into page table.
 **
 **      gctUINT32 Offset
 **          Offset into page.
 **
 **  OUTPUT:
 **
 **      gctUINT32 *Address
 **          Pointer to a variable receiving te hardware address.
 */
gceSTATUS
gckHARDWARE_BuildVirtualAddress(gckHARDWARE Hardware, gctUINT32 Index,
                                gctUINT32 Offset, gctUINT32 *Address)
{
    gcmkHEADER_ARG("Hardware=%p Index=%u Offset=%u",
                   Hardware, Index, Offset);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Address != gcvNULL);

    /* Build virtual address. */
    *Address = gcmSETFIELDVALUE(0, AQ_MEMORY_ADDRESS, TYPE, VIRTUAL_SYSTEM) |
                    gcmSETFIELD(0, AQ_MEMORY_ADDRESS, ADDRESS, Offset | (Index << 12));

    /* Success. */
    gcmkFOOTER_ARG("*Address=0x%08x", *Address);
    return gcvSTATUS_OK;
}

gceSTATUS
gckHARDWARE_GetIdle(gckHARDWARE Hardware, gctBOOL Wait, gctUINT32 *Data)
{
    gceSTATUS status;
    gctUINT32 idle = 0;
    gctINT retry, poll, pollCount;
    gctADDRESS address;
    gctUINT32 addressLow, addressHi = 0;


    gcmkHEADER_ARG("Hardware=%p Wait=%d", Hardware, Wait);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Data != gcvNULL);

    /* If we have to wait, try 100 polls per millisecond. */
    pollCount = Wait ? 100 : 1;

    /* At most, try for 1 second. */
    for (retry = 0; retry < 1000; ++retry) {
        /* If we have to wait, try 100 polls per millisecond. */
        for (poll = pollCount; poll > 0; --poll) {
            /* Read register. */
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             AQ_HI_IDLE_Address, &idle));

            /* Read the current FE address. */
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

            address = addressLow;

            if (Hardware->graphicsLargeVA) {
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

                address = ((gctADDRESS)addressHi << 32) | addressLow;
            }

            /* See if we have to wait for FE idle. */
            if (_IsHWIdle(idle, Hardware) && (address == Hardware->lastEnd + 8)) {
                /* FE is idle. */
                break;
            }
        }

        /* Check if we need to wait for FE and FE is busy. */
        if (Wait && !_IsHWIdle(idle, Hardware)) {
            /* Wait a little. */
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                           "%s: Waiting for idle: 0x%08X",
                           __FUNCTION__, idle);

            gcmkVERIFY_OK(gckOS_Delay(Hardware->os, 1));
        } else {
            break;
        }
    }

    /* Return idle to caller. */
    *Data = idle;

#if defined(EMULATOR)
    /*
     * Wait a little while until CModel FE gets END.
     * END is supposed to be appended by caller.
     */
    gckOS_Delay(Hardware->os, 100);
#endif

    /* Success. */
    gcmkFOOTER_ARG("*Data=0x%08x", *Data);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/* Flush the caches. */
gceSTATUS
gckHARDWARE_Flush(gckHARDWARE Hardware, gceKERNEL_FLUSH Flush,
                  gctPOINTER Logical, gctUINT32 *Bytes)
{
    gctUINT32 pipe;
    gctUINT32 flush = 0;
    gctUINT32 flushVST = 0;
    gctBOOL flushTileStatus;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gceSTATUS status;
    gctBOOL halti5;
    gctBOOL flushICache;
    gctBOOL flushTXDescCache;
    gctBOOL flushTFB;
    gctBOOL hwTFB;
    gctBOOL blt;
    gctBOOL peTSFlush;
    gctBOOL multiCluster;
    gctBOOL computeOnly;

    gcmkHEADER_ARG("Hardware=0x%x Flush=0x%x Logical=0x%x *Bytes=0x%x",
                   Hardware, Flush, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Get current pipe. */
    pipe = Hardware->kernel->command->pipeSelect;

    halti5 = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI5);

    hwTFB = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HW_TFB);

    blt = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE);
    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    peTSFlush = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_PE_TILE_CACHE_FLUSH_FIX);

    computeOnly = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_COMPUTE_ONLY);

    /* Flush tile status cache. */
    flushTileStatus = Flush & gcvFLUSH_TILE_STATUS;

    /* Flush Icache for halti5 hardware as we dont do it when program or context switches*/
    flushICache = (Flush & gcvFLUSH_ICACHE) && halti5;

    /* Flush texture descriptor cache */
    flushTXDescCache = Flush & gcvFLUSH_TXDESC;

    /* Flush USC cache for TFB client */
    flushTFB = (Flush & gcvFLUSH_TFBHEADER) && hwTFB;

    /* Flush TFB for vertex buffer */
    if (Flush & gcvFLUSH_VERTEX) {
        if (hwTFB)
            flushTFB = gcvTRUE;

        if (multiCluster)
            flush |= gcmSETFIELDVALUE(0, AQ_FLUSH, VERTEX_DATA_CACHE, ENABLE);
    }

    /* Flush 3D color cache. */
    if ((Flush & gcvFLUSH_COLOR) && pipe == AQ_PIPE_SELECT_PIPE_PIPE3D)
        flush |= gcmSETFIELDVALUE(0, AQ_FLUSH, CCACHE, ENABLE);

    /* Flush 3D depth cache. */
    if ((Flush & gcvFLUSH_DEPTH) && pipe == AQ_PIPE_SELECT_PIPE_PIPE3D)
        flush |= gcmSETFIELDVALUE(0, AQ_FLUSH, ZCACHE, ENABLE);

    /* Flush 3D texture cache. */
    if ((Flush & gcvFLUSH_TEXTURE) && pipe == AQ_PIPE_SELECT_PIPE_PIPE3D) {
        flush |= multiCluster ? 0 : gcmSETFIELDVALUE(0, AQ_FLUSH, TCACHE, ENABLE);
        flushVST = gcmSETFIELDVALUE(0, AQ_FLUSH, VST_CACHE, ENABLE);
    }

    /* Flush L2 cache. */
    if ((Flush & gcvFLUSH_L2) && pipe == AQ_PIPE_SELECT_PIPE_PIPE3D)
        flush |= gcmSETFIELDVALUE(0, AQ_FLUSH, L2_CACHE, ENABLE);

    /* Vertex buffer and texture could be touched by SHL1 for SSBO and image load/store */
    if ((Flush & (gcvFLUSH_VERTEX | gcvFLUSH_TEXTURE)) && pipe == AQ_PIPE_SELECT_PIPE_PIPE3D) {
        flush |= gcmSETFIELDVALUE(0, AQ_FLUSH, SHL1_CACHE, ENABLE) |
                 gcmSETFIELDVALUE(0, AQ_FLUSH, VSSHL1_CACHE, ENABLE) |
                 gcmSETFIELDVALUE(0, AQ_FLUSH, PSSHL1_CACHE, ENABLE);
    }

    /* See if there is a valid flush. */
    if (flush == 0 &&
        flushTileStatus == gcvFALSE &&
        flushICache == gcvFALSE &&
        flushTXDescCache == gcvFALSE &&
        flushTFB == gcvFALSE) {
        if (Bytes != gcvNULL) {
            /* No bytes required. */
            *Bytes = 0;
        }
    } else {
        gctBOOL appendNop = gcvFALSE;
        gctUINT32 reserveBytes = 0;
        gctBOOL txCacheFix =
            gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_TEX_CACHE_FLUSH_FIX) ?
            gcvTRUE : gcvFALSE;

        /* Determine reserve bytes. */
        if (!txCacheFix || flushICache || flushTXDescCache) {
            /* Semaphore/Stall */
            reserveBytes += blt ? (8 * gcmSIZEOF(gctUINT32)) : (4 * gcmSIZEOF(gctUINT32));
        }

        if (flush)
            reserveBytes += 2 * gcmSIZEOF(gctUINT32);

        if (flushVST)
            reserveBytes += 2 * gcmSIZEOF(gctUINT32);

        if (flushTileStatus && !computeOnly) {
            reserveBytes += (!peTSFlush && blt) ?
                            6 * gcmSIZEOF(gctUINT32) : 2 * gcmSIZEOF(gctUINT32);
        }

        if (flushICache)
            reserveBytes += 2 * gcmSIZEOF(gctUINT32);

        if (flushTXDescCache)
            reserveBytes += 2 * gcmSIZEOF(gctUINT32);

        if (flushTFB)
            reserveBytes += 2 * gcmSIZEOF(gctUINT32);

        /* Semaphore/Stall */
        reserveBytes += blt ? (8 * gcmSIZEOF(gctUINT32)) : (4 * gcmSIZEOF(gctUINT32));

#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
        if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MCFE) && (reserveBytes & 8)) {
            appendNop = gcvTRUE;
            reserveBytes += 8;
        }
#endif

        /* Copy to command queue. */
        if (Logical != gcvNULL) {
            if (*Bytes < reserveBytes) {
                /* Command queue too small. */
                gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
            }

            if (!txCacheFix || flushICache || flushTXDescCache) {
                if (blt) {
                    /* Semaphore from FE to BLT. */
                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                    *logical++ =
                        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

                    *logical++ = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                                 gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, BLT_ENGINE);

                    /* Stall from FE to BLT. */
                    *logical++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

                    *logical++ = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
                                 gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, BLT_ENGINE);

                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                    *logical++ =
                        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
                } else {
                    /* Semaphore. */
                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

                    *logical++ = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                                 gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);

                    /* Stall. */
                    *logical++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

                    *logical++ = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
                                 gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE);
                }
            }

            if (flush) {
                /* Append LOAD_STATE to AQFlush. */
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQFlushRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *logical++ = flush;

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "%p: FLUSH 0x%x", logical - 1, flush);
            }

            if (flushVST) {
                /* Append LOAD_STATE to AQFlush. */
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQFlushRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *logical++ = flushVST;

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "0x%x: FLUSH 0x%x", logical - 1, flush);
            }

            if (flushTileStatus && !computeOnly) {
                if (!peTSFlush && blt) {
                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                    *logical++ =
                        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltCacheFlushRegAddrs) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                    *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_CACHE_FLUSH, TILE_CACHE, ENABLE);

                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                    *logical++ =
                        gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
                } else {
                    *logical++ =
                        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcTileCacheFlushRegAddrs) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                    *logical++ = gcmSETFIELDVALUE(0, GC_TILE_CACHE_FLUSH, FLUSH, ENABLE);
                }

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "0x%x: FLUSH TILE STATUS 0x%x",
                               logical - 1, logical[-1]);
            }

            if (flushICache) {
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregSHIcacheInvalidateRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, 1);

                *logical++ = gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, VS, 1) |
                             gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, TCS, 1) |
                             gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, TES, 1) |
                             gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, GS, 1) |
                             gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, PS, 1);

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "0x%x: FLUSH Icache 0x%x",
                               logical - 1, logical[-1]);
            }

            if (flushTXDescCache) {
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTXGpipeCommandRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *logical++ = gcmSETFIELDVALUE(0, GCREG_TX_GPIPE_COMMAND, COMMAND, INVALIDATE_ALL);

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "%p: FLUSH Icache 0x%x",
                               logical - 1, logical[-1]);
            }

            if (flushTFB) {
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTFBInvalidateRegAddrs) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *logical++ = GCREG_TFB_INVALIDATE_INVALIDATE_TRUE;

                gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                               "0x%x: FLUSH TFB cache 0x%x",
                               logical - 1, logical[-1]);
            }

            if (blt) {
                /* Semaphore from FE to BLT. */
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

                *logical++ = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                             gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, BLT_ENGINE);

                /* Stall from FE to BLT. */
                *logical++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

                *logical++ = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
                             gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, BLT_ENGINE);

                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

                *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
            } else {
                /* Semaphore. */
                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

                *logical++ = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
                             gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);

                /* Stall. */
                *logical++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

                *logical++ = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
                             gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE);
            }
            if (appendNop) {
#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
                *logical++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);

                *logical++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
#endif
            }
        }

        if (Bytes != gcvNULL) {
            /* bytes required. */
            *Bytes = reserveBytes;
        }
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_SetFastClear(gckHARDWARE Hardware, gctINT Enable, gctINT Compression)
{
    gctUINT32 debug;
    gceSTATUS status;
    gceCOMPRESSION_OPTION compression = (Compression == -1) ?
                                        gcvCOMPRESSION_OPTION_DEFAULT :
                                        (gceCOMPRESSION_OPTION)Compression;

    gcmkHEADER_ARG("Hardware=%p Enable=%d Compression=%d",
                   Hardware, Enable, Compression);

    /* Only process if fast clear is available. */
    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_FAST_CLEAR)) {
        if (Enable == -1) {
            /* Determine automatic value for fast clear. */
            Enable = 1;
        }

        if (compression == gcvCOMPRESSION_OPTION_DEFAULT) {
            /* Determine automatic value for compression. */
            if (Enable) {
                if (gcvSTATUS_FALSE ==
                    gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_ZCOMPRESSION)) {
                    compression &= ~gcvCOMPRESSION_OPTION_DEPTH;
                }
            } else {
                compression = gcvCOMPRESSION_OPTION_NONE;
            }
        }

        /* Read AQMemoryDebug register. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         AQ_MEMORY_DEBUG_Address, &debug));

        /* Set fast clear bypass. */
        debug = gcmSETFIELD(debug, AQ_MEMORY_DEBUG, DISABLE_FAST_CLEAR, Enable == 0);

        /* Set compression bypass. */
        debug = gcmSETFIELD(debug, AQ_MEMORY_DEBUG, DISABLE_ZCOMPRESSION,
                            (gcvCOMPRESSION_OPTION_NONE == compression) ? 1 : 0);

        /* Write back AQMemoryDebug register. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_MEMORY_DEBUG_Address, debug));

        /* Store fast clear and comprersison flags. */
        Hardware->options.allowFastClear = Enable;
        Hardware->options.allowCompression = compression;

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "FastClear=%d Compression=%d",
                       Enable, Compression);
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();

    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_PowerControlClusters
 **
 **  Power control clusters of one core.
 **  Currently only support NN clusters.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **      gctUINT32 PowerControlVaule
 **          The value programmed to power control register.
 **
 **      gctBOOL PowerState
 **          Power State to switch.
 **
 */

gceSTATUS
gckHARDWARE_PowerControlClusters(gckHARDWARE Hardware,
                                 gctUINT32 PowerControlValue,
                                 gctBOOL PowerState)
{
    gceSTATUS status = gcvSTATUS_OK;
    gckCOMMAND command = gcvNULL;
    gctPOINTER buffer = gcvNULL;
    gctUINT32_PTR logical = gcvNULL;
    gctUINT32 reqBytes = 16;
    gctUINT32 bytes;
    gctUINT32 idle, timer = 0;

    gcmkHEADER_ARG("Hardware=%p PowerControlValue=%x",
                   Hardware, PowerControlValue);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    command = Hardware->kernel->command;

#if !gcdFPGA_BUILD
    if (Hardware->identity.customerID != 0x85 || !Hardware->mcFE ||
        Hardware->options.enableNNClusters == (gctUINT32)~0UL ||
        (!PowerState && !Hardware->powerState))
#endif
    {
        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    }

    /* Program cluster power control command, only with MCFE by current. */
    {
        /* Start the command parser. */
        gcmkONERROR(gckCOMMAND_Start(command));

        /* Reserve space in the command queue. */
        gcmkONERROR(gckCOMMAND_Reserve(command, reqBytes, &buffer, &bytes));

        logical = (gctUINT32_PTR)buffer;

        *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregConfigNNRegAddrs) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *logical++ = gcmSETFIELD(0, GCREG_CONFIG_NN, POWER_CONTROL, PowerControlValue);

        *logical++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);
        *logical++ = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP);

        gcmkONERROR(gckCOMMAND_ExecuteMultiChannel(command, gcvFALSE, 2, reqBytes));

        do {
            gckOS_Udelay(Hardware->os, 10);

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             AQ_HI_IDLE_Address, &idle));

            timer += 1;

#if gcdGPU_TIMEOUT
            if (timer >= Hardware->kernel->timeOut) {
                gcmkPRINT("%s %d ftg340 timeout...\n", __FUNCTION__, __LINE__);

                gcmkONERROR(gcvSTATUS_DEVICE);
            }
#endif
        } while (!_IsHWIdle(idle, Hardware));
        gcmkDUMP(Hardware->os,
                 "@[register.wait 0x%05X 0x%08X 0x%08X]",
                 AQ_HI_IDLE_Address,
                 gcmSETFIELD(0, AQ_HI_IDLE, IDLE_FE, ~0U),
                 idle);

        /* Stop the command parser. */
        gcmkONERROR(gckCOMMAND_Stop(command));
    }

OnError:
    gcmkFOOTER();
    return status;
}

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
static gctCONST_STRING
_PowerEnum(gceCHIPPOWERSTATE State)
{
    const gctCONST_STRING baseStates[] = {
        "ON", "IDLE", "SUSPEND", "OFF", "OFF(exit)", "ON[auto]",
    };

    const gctCONST_STRING broadcastStates[] = {
        "",
        "IDLE[broadcast]",
        "SUSPEND[broadcast]",
        "OFF[broadcast]",
    };

    const gctCONST_STRING timeoutStates[] = {
        "",
        "IDLE[timeout]",
        "SUSPEND[timeout]",
        "OFF[timeout]",
    };

    gcmSTATIC_ASSERT(gcvPOWER_ON == 0 && gcvPOWER_IDLE == 1 &&
                     gcvPOWER_SUSPEND == 2 && gcvPOWER_OFF == 3 && gcvPOWER_OFF_EXIT ==4 && gcvPOWER_ON_AUTO == 5,
                     "array subscript does not match");

    if (State & gcvPOWER_FLAG_BROADCAST)
        return broadcastStates[State & ~gcvPOWER_FLAG_BROADCAST];
    else if (State & gcvPOWER_FLAG_TIMEOUT)
        return timeoutStates[State & ~gcvPOWER_FLAG_TIMEOUT];
    else if ((State >= gcvPOWER_ON) && (State <= gcvPOWER_ON_AUTO))
        return baseStates[State - gcvPOWER_ON];

    return "unknown";
}
#endif

static gceSTATUS
_PmClockOn(gckHARDWARE Hardware, gctBOOL *RequireInit)
{
    gceSTATUS status;

    /* Turn on the power. */
    gcmkONERROR(gckOS_SetGPUPower(Hardware->os, Hardware->kernel, gcvTRUE, gcvTRUE));

    Hardware->clockState = gcvTRUE;
    Hardware->powerState = gcvTRUE;

    /* Check if GPU is present and awake. */
    while (_IsGPUPresent(Hardware) == gcvSTATUS_GPU_NOT_RESPONDING) {
        /* Turn off the power and clock. */
        gcmkONERROR(gckOS_SetGPUPower(Hardware->os, Hardware->kernel, gcvFALSE, gcvFALSE));

        Hardware->clockState = gcvFALSE;
        Hardware->powerState = gcvFALSE;

        /* Wait a little. */
        gckOS_Delay(Hardware->os, 1);

        /* Turn on the power and clock. */
        gcmkONERROR(gckOS_SetGPUPower(Hardware->os, Hardware->kernel, gcvTRUE, gcvTRUE));

        Hardware->clockState = gcvTRUE;
        Hardware->powerState = gcvTRUE;

        if (RequireInit)
            *RequireInit = gcvTRUE;
    }

OnError:
    return status;
}

static gceSTATUS
_PmClockOff(gckHARDWARE Hardware, gctBOOL PowerState)
{
    gceSTATUS status;

    gcmkONERROR(gckOS_SetGPUPower(Hardware->os, Hardware->kernel, gcvFALSE, PowerState));

    Hardware->clockState = gcvFALSE;
    Hardware->powerState = PowerState;

OnError:
    return status;
}

static gceSTATUS
_PmClockControl(gckHARDWARE Hardware, gceCHIPPOWERSTATE State)
{
    gceSTATUS status;
    gctUINT32 clock;
    gctUINT32 shaderClock;
    gctBOOL needUpdateShaderClock = gcvFALSE;

    static const gctUINT clocks[4] = {
        /* gcvPOWER_ON */
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK3D_DIS,       0) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK2D_DIS,       0) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_VAL,     64) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1),

        /* gcvPOWER_IDLE */
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK3D_DIS,       0) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK2D_DIS,       0) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_VAL,      1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1),

        /* gcvPOWER_SUSPEND */
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK3D_DIS,       1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK2D_DIS,       1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_VAL,      1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1),

        /* gcvPOWER_OFF */
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK3D_DIS,       1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK2D_DIS,       1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_VAL,      1) |
        gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1),

    };

    clock = clocks[State];
    clock = gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);

    shaderClock =   gcmSETFIELD(0, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH, 0)
                  | gcmSETFIELD(0, GC_PULSE_EATER, DISABLE_AUTO_PULSE,   1)
                  | gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_VAL_SH,        64)
                  | gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH,   1);

    if (Hardware->identity.customerID == 0xc6 ||
        Hardware->identity.customerID == 0x10000001) {
        return gcvSTATUS_OK;
    }

#if gcdFPGA_BUILD
    return gcvSTATUS_OK;
#endif

#if gcdENABLE_FSCALE_VAL_ADJUST
    if (State == gcvPOWER_ON) {
        clock = gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, FSCALE_VAL, Hardware->powerOnFscaleVal);

        if (Hardware->powerOnShaderFscaleVal != ~0U &&
            Hardware->powerOnShaderFscaleVal > 0 &&
            Hardware->powerOnShaderFscaleVal < 64) {
            needUpdateShaderClock = gcvTRUE;
            shaderClock =   gcmSETFIELD(0, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH, 0)
                          | gcmSETFIELD(0, GC_PULSE_EATER, DISABLE_AUTO_PULSE,   1)
                          | gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_VAL_SH,        Hardware->powerOnShaderFscaleVal)
                          | gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH,   1);
        }
    }
#endif

    if (Hardware->clockState && Hardware->powerState
#if gcdDVFS
        /* Don't touch clock control if dynamic frequency scaling is available. */
        && !gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_DYNAMIC_FREQUENCY_SCALING)
#endif
    ) {
        /* Write the clock control register. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address, clock));

        clock = gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 0);

        /* Done loading the frequency scaler. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address, clock));
        if (needUpdateShaderClock) {
            gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                              GC_PULSE_EATER_Address, shaderClock));

            /* Done loading the frequency scaler. */
            gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                              GC_PULSE_EATER_Address,
                                              gcmSETFIELD(shaderClock, GC_PULSE_EATER,
                                                          FSCALE_CMD_LOAD_SH, 0)));
        }
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

static gceSTATUS
_PmInitializeGPU(gckHARDWARE Hardware, gckCOMMAND Command)
{
    gceSTATUS status;

    /* Initialize hardware. */
    gcmkONERROR(gckHARDWARE_InitializeHardware(Hardware));

    gcmkONERROR(gckHARDWARE_SetFastClear(Hardware,
                                         Hardware->options.allowFastClear,
                                         Hardware->options.allowCompression));

    /* Force the command queue to reload the next context. */
    Command->currContext = gcvNULL;

OnError:
    return status;
}

/*
 * Notice: this function may return gcvSTATUS_NOT_READY, which is not an error,
 * but either not success.
 */
static gceSTATUS
_PmStallCommand(gckHARDWARE Hardware,
                gckCOMMAND Command,
                gctBOOL Broadcast,
                gctBOOL Exit)
{
    gceSTATUS status;
    gctBOOL idle;
    gctUINT32 timer = 0, delay = 1;

    if (Broadcast) {
        /* Check for idle. */
        gcmkONERROR(gckHARDWARE_QueryIdle(Hardware, &idle));

        if (!idle) {
            status = gcvSTATUS_CHIP_NOT_READY;
            goto OnError;
        }
    } else {
        /* Wait to finish all commands. */
        status = gckCOMMAND_Stall(Command, gcvTRUE);

        if (!gcmIS_SUCCESS(status) && !Exit)
            goto OnError;

        for (;;) {
            gcmkONERROR(gckHARDWARE_QueryIdle(Hardware, &idle));

            if (idle)
                break;

            gcmkVERIFY_OK(gckOS_Delay(Hardware->os, delay));

            timer += delay;
            delay *= 2;

#if gcdGPU_TIMEOUT
            if (timer >= Hardware->kernel->timeOut) {
                gcmkPRINT("[ftg340]: GPU timeout...\n");

                /* Make sure hardware is in safe state during exiting procedure. */
                if (Exit) {
                    gcmkPRINT("[ftg340]: Executing recovery.\n");
                    gcmkONERROR(gckKERNEL_Recovery(Hardware->kernel));
                    gcmkPRINT("[ftg340]: Recovery done.\n");
                } else {
                    gcmkONERROR(gcvSTATUS_DEVICE);
                }
            }
#endif
        }
    }

OnError:
    return status;
}

static gceSTATUS
_PmFlushCache(gckHARDWARE Hardware, gckCOMMAND Command)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL funcValid = gcvFALSE;

    if (Hardware->clockState == gcvFALSE) {
        /* Turn on the GPU power. */
        gcmkONERROR(gckOS_SetGPUPower(Hardware->os, Hardware->kernel, gcvTRUE, gcvTRUE));

        Hardware->clockState = gcvTRUE;

        /* Clock control, to ON state. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_ON));
    }

    gckFUNCTION_Validate(&Hardware->functions[gcvFUNCTION_EXECUTION_FLUSH], &funcValid);

    if (funcValid)
        gcmkONERROR(gckFUNCTION_Execute(&Hardware->functions[gcvFUNCTION_EXECUTION_FLUSH]));

OnError:
    return status;
}

/*
 * Put power to on direction:
 * Off -> Suspend
 * Off            -> Idle
 * Off                    -> On
 *        Suspend -> Idle
 *        Suspend         -> On
 *                   Idle -> On
 */
static gceSTATUS
_PmSetPowerOnDirection(gckHARDWARE Hardware, gceCHIPPOWERSTATE State, gctBOOL Global)
{
    gceSTATUS status;
    gckCOMMAND command = Hardware->kernel->command;
    gctBOOL clockOn = gcvFALSE;
    gctBOOL requireInit = gcvFALSE;

    switch (Hardware->chipPowerState) {
    case gcvPOWER_OFF:
        if (State == gcvPOWER_SUSPEND) {
            gcmkONERROR(_PmClockOn(Hardware, gcvNULL));
            clockOn = gcvTRUE;

            /* Clock control, put to suspend. */
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_SUSPEND));

            /* Initialize GPU. */
            gcmkONERROR(_PmInitializeGPU(Hardware, command));

            /* Suspend: clock off, power on. */
            gcmkONERROR(_PmClockOff(Hardware, gcvTRUE));
            break;
        }

        requireInit = gcvTRUE;
        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_SUSPEND:
        /* Clock on. */
        gcmkONERROR(_PmClockOn(Hardware, &requireInit));
        clockOn = gcvTRUE;

        /* Clock control, put to target state (On or idle). */
        gcmkONERROR(_PmClockControl(Hardware, State));

        /* Delay. */
        gcmkONERROR(gckOS_Delay(Hardware->os, gcdPOWER_CONTROL_DELAY));

        if (requireInit) {
#if gcdSUSPEND_RESUME_FROM_DISK
            if (Global)
                gcmkONERROR(_ResetGPU(Hardware, gcvTRUE));
#endif
            /* Initialize. */
            gcmkONERROR(_PmInitializeGPU(Hardware, command));
        }

        /* Start. */
        gcmkONERROR(gckCOMMAND_Start(command));
        break;

    case gcvPOWER_IDLE:
        /* Clock control, put to ON state. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_ON));
        break;

    default:
        break;
    }

    return gcvSTATUS_OK;

OnError:
    if (clockOn) {
        gctBOOL powerState = (Hardware->chipPowerState == gcvPOWER_SUSPEND);

        _PmClockOff(Hardware, powerState);
    }

    return status;
}

/*
 * Put power to off direction:
 * On -> Idle
 * On         -> Suspend
 * On                    -> Off
 *       Idle -> Suspend
 *       Idle            -> Off
 *               Suspend -> Off
 */
static gceSTATUS
_PmSetPowerOffDirection(gckHARDWARE Hardware, gceCHIPPOWERSTATE State, gctBOOL Broadcast)
{
    gceSTATUS status;
    gckCOMMAND command = Hardware->kernel->command;
    gctBOOL exit = gcvFALSE;

    if (State == gcvPOWER_OFF_EXIT) {
        exit = gcvTRUE;
        State = gcvPOWER_OFF;
    }

    switch (Hardware->chipPowerState) {
    case gcvPOWER_ON:
        if (Hardware->kernel->threadInitialized == gcvTRUE) {
            /* Stall. */
            status = _PmStallCommand(Hardware, command, Broadcast, exit);

            if (!gcmIS_SUCCESS(status)) {
                /* abort for error and NOT READY. */
                goto OnError;
            }
        }

        if (State == gcvPOWER_IDLE) {
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_IDLE));
            break;
        }
        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_IDLE:
        /* Stop. */
        gcmkONERROR(gckCOMMAND_Stop(command));
        if (Hardware->kernel->asyncCommand) {
            /* Stop the async command queue. */
            gcmkONERROR(gckCOMMAND_Stop(Hardware->kernel->asyncCommand));
        }

        if (State == gcvPOWER_SUSPEND) {
            /* Clock control, put to suspend state. */
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_SUSPEND));

            /* Power on, clock off. */
            gcmkONERROR(_PmClockOff(Hardware, gcvTRUE));
            break;
        }

        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_SUSPEND:
        if (Hardware->kernel->threadInitialized == gcvTRUE) {
            /* Flush. */
            gcmkONERROR(_PmFlushCache(Hardware, command));
        }

        gcmkONERROR(gckHARDWARE_PowerControlClusters(Hardware, 0x2, gcvFALSE));

        /* Clock control. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_OFF));

        /* Power off, clock off. */
        gcmkONERROR(_PmClockOff(Hardware, gcvFALSE));

        break;

    default:
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

/******************************************************************************
 ****************************** Qchannel Power Management *********************
 ******************************************************************************/

gceSTATUS
gckHARDWARE_QchannelPowerControl(gckHARDWARE Hardware, gctBOOL ClockState, gctBOOL PowerState)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 reg = 0, delay = 1;
    gctBOOL powerChange = gcvFALSE;
    gctBOOL clockChange = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    powerChange = (PowerState != Hardware->powerState);
    clockChange = (ClockState != Hardware->clockState);

    if (clockChange) {
        gcmkVERIFY_OK(gckOS_SetClockState(Hardware->os, Hardware->kernel, ClockState));
    }

    if (powerChange && PowerState == gcvTRUE) {
        gctBOOL state;

        gcmkVERIFY_OK(gckOS_GetClockState(Hardware->os, Hardware->kernel, &state));
        gcmkVERIFY_OK(gckOS_SetClockState(Hardware->os, Hardware->kernel, gcvTRUE));

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_QCHANNEL_POWER_CONTROL_Address,
                                          gcmSETFIELD(AQ_QCHANNEL_POWER_CONTROL_ResetValue,
                                                      AQ_QCHANNEL_POWER_CONTROL,
                                                      START_UP_QCHANNEL, 1)));

        do {
            gckOS_Delay(Hardware->os, delay);

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               AQ_QCHANNEL_POWER_STATUS_Address, &reg));
            delay *= 2;

        } while (!(reg & 0x1));

        gcmkVERIFY_OK(gckOS_SetClockState(Hardware->os, Hardware->kernel, state));
    }

    if (powerChange && PowerState == gcvFALSE) {
        gctBOOL state;

        gcmkVERIFY_OK(gckOS_GetClockState(Hardware->os, Hardware->kernel, &state));
        gcmkVERIFY_OK(gckOS_SetClockState(Hardware->os, Hardware->kernel, gcvTRUE));

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_QCHANNEL_POWER_CONTROL_Address,
                                          gcmSETFIELD(0, AQ_QCHANNEL_POWER_CONTROL,
                                                      POWER_OFF_QCHANNEL, 1)));

        gcmkVERIFY_OK(gckOS_SetClockState(Hardware->os, Hardware->kernel, state));
    }

    Hardware->clockState = ClockState;
    Hardware->powerState = PowerState;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_QchannelBypass(gckHARDWARE Hardware, gctBOOL Enable)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_QCHANNEL_POWER_CONTROL_Address,
                                      gcmSETFIELD(0, AQ_QCHANNEL_POWER_CONTROL,
                                                  DISABLE_QCHANNEL, Enable)));

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_QchannelFlushCache(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL funcValid = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    if (Hardware->clockState == gcvFALSE) {
        /* Turn on the GPU power. */
        gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvTRUE, gcvTRUE));

        /* Clock control, to ON state. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_ON));
    }

    gckFUNCTION_Validate(&Hardware->functions[gcvFUNCTION_EXECUTION_FLUSH], &funcValid);

    if (funcValid)
        gcmkONERROR(gckFUNCTION_Execute(&Hardware->functions[gcvFUNCTION_EXECUTION_FLUSH]));

OnError:
    gcmkFOOTER();

    return status;
}

static gceSTATUS
_QchannelPowerOnDirection(gckHARDWARE Hardware, gceCHIPPOWERSTATE State, gctBOOL Global)
{
    gceSTATUS status;
    gckCOMMAND command = Hardware->kernel->command;
    gctBOOL clockOn = gcvFALSE;
    gctBOOL requireInit = gcvTRUE;

    switch (Hardware->chipPowerState) {
    case gcvPOWER_OFF:
        if (State == gcvPOWER_SUSPEND) {
            gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvTRUE, gcvTRUE));
            clockOn = gcvTRUE;

            /* Clock control, put to suspend. */
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_SUSPEND));

            /* Initialize GPU. */
            gcmkONERROR(_PmInitializeGPU(Hardware, command));

            /* Suspend: clock off, power on. */
            gcmkONERROR(_PmClockOff(Hardware, gcvTRUE));
            break;
        }

        requireInit = gcvTRUE;
        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_SUSPEND:
        /* Power on, clock on. */
        gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvTRUE, gcvTRUE));

        clockOn = gcvTRUE;

        /* Clock control, put to target state (On or idle). */
        gcmkONERROR(_PmClockControl(Hardware, State));

        /* Delay. */
        gcmkONERROR(gckOS_Delay(Hardware->os, gcdPOWER_CONTROL_DELAY));

        if (requireInit) {
#if gcdSUSPEND_RESUME_FROM_DISK
            if (Global)
                gcmkONERROR(_ResetGPU(Hardware, gcvTRUE));
#endif
            /* Initialize. */
            gcmkONERROR(_PmInitializeGPU(Hardware, command));
        }

        /* Start. */
        gcmkONERROR(gckCOMMAND_Start(command));
        if (Hardware->kernel->asyncCommand) {
            /* Start the async command queue. */
            gcmkONERROR(gckCOMMAND_Start(Hardware->kernel->asyncCommand));
        }
        break;

    case gcvPOWER_IDLE:
        /* Clock control, put to ON state. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_ON));

        break;

    default:
        break;
    }

    return gcvSTATUS_OK;

OnError:
    if (clockOn) {
        gctBOOL powerState = (Hardware->chipPowerState == gcvPOWER_SUSPEND);

        gcmkVERIFY_OK(gckHARDWARE_QchannelPowerControl(Hardware, gcvFALSE, powerState));
    }

    return status;
}

static gceSTATUS
_QchannelPowerOffDirection(gckHARDWARE Hardware, gceCHIPPOWERSTATE State, gctBOOL Broadcast)
{
    gceSTATUS status;
    gckCOMMAND command = Hardware->kernel->command;
    gctBOOL exit = gcvFALSE;

    if (State == gcvPOWER_OFF_EXIT) {
        exit = gcvTRUE;
        State = gcvPOWER_OFF;
    }

    switch (Hardware->chipPowerState) {
    case gcvPOWER_ON:
        if (Hardware->kernel->threadInitialized == gcvTRUE) {
            /* Stall. */
            status = _PmStallCommand(Hardware, command, Broadcast, exit);

            if (!gcmIS_SUCCESS(status)) {
                /* abort for error and NOT READY. */
                goto OnError;
            }
        }

        if (State == gcvPOWER_IDLE) {
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_IDLE));
            break;
        }
        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_IDLE:
        /* Stop. */
        gcmkONERROR(gckCOMMAND_Stop(command));

        if (State == gcvPOWER_SUSPEND) {
            /* Clock control, put to suspend state. */
            gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_SUSPEND));

            /* Power on, clock off. */
            gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvFALSE, gcvTRUE));
            break;
        }

        /* FALLTHRU */
        gcmkFALLTHRU;

    case gcvPOWER_SUSPEND:
        if (Hardware->kernel->threadInitialized == gcvTRUE) {
            /* Flush. */
            gcmkONERROR(gckHARDWARE_QchannelFlushCache(Hardware));
        }

        /* Clock control, put to off state. */
        gcmkONERROR(_PmClockControl(Hardware, gcvPOWER_OFF));

        /* Power off, clock off. */
        gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvFALSE, gcvFALSE));

        break;

    default:
        break;
    }

    return gcvSTATUS_OK;

OnError:
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_SetPowerState
 **
 **  Set GPU to a specified power state.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gceCHIPPOWERSTATE State
 **          Power State.
 **
 */
gceSTATUS
gckHARDWARE_SetPowerState(gckHARDWARE Hardware, gceCHIPPOWERSTATE State)
{
    gceSTATUS status;
    gckCOMMAND command = gcvNULL;
    gckOS os;
    gctBOOL powerAcquired = gcvFALSE;
    gctBOOL mutexAcquired = gcvFALSE;
    gctBOOL broadcast = gcvFALSE;
    gctBOOL timeout = gcvFALSE;
    gceCHIPPOWERSTATE state = gcvPOWER_INVALID;

    /*
     * Acquire globalSempahore when set to global OFF, IDLE, SUSPEND, then
     * can not switch to non-global state unless global ON comes and release
     * the globalSemaphore.
     */
    gctBOOL global = gcvFALSE;
    gctBOOL globalAcquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p State=%d", Hardware, State);

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                   "Switching to power state %d(%s)",
                   State, _PowerEnum(State));
#endif

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    os = Hardware->os;
    command = Hardware->kernel->command;

    /* Convert power state. */
    switch (State) {
    case gcvPOWER_ON:
    case gcvPOWER_OFF:
    case gcvPOWER_IDLE:
    case gcvPOWER_SUSPEND:
    case gcvPOWER_OFF_EXIT:
        global = gcvTRUE;
        state = State;
        break;

    case gcvPOWER_ON_AUTO:
        state = gcvPOWER_ON;
        break;

    case gcvPOWER_OFF_TIMEOUT:
    case gcvPOWER_IDLE_TIMEOUT:
    case gcvPOWER_SUSPEND_TIMEOUT:
        timeout = gcvTRUE;
        /* FALLTHRU */
        gcmkFALLTHRU;
    case gcvPOWER_OFF_BROADCAST:
    case gcvPOWER_IDLE_BROADCAST:
    case gcvPOWER_SUSPEND_BROADCAST:
        broadcast = gcvTRUE;
        state = State & ~(gcvPOWER_FLAG_BROADCAST | gcvPOWER_FLAG_TIMEOUT);
        break;

    case gcvPOWER_INVALID:
        /* Canceled, nothing to do. */
        status = gcvSTATUS_OK;
        goto OnError;

    default:
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    if (broadcast) {
        /* Try to acquire the power mutex. */
        status = gckOS_AcquireMutex(os, Hardware->powerMutex, 0);

        if (status == gcvSTATUS_TIMEOUT) {
            /* Pm in progress, abort. */
            status = gcvSTATUS_OK;
            goto OnError;
        }
    } else {
        /* Acquire the power mutex. */
        status = gckOS_AcquireMutex(os, Hardware->powerMutex, gcvINFINITE);
    }

    gcmkONERROR(status);
    mutexAcquired = gcvTRUE;

    if (Hardware->chipPowerState == state) {
        /* No state change. */
        status = gcvSTATUS_OK;
        goto OnError;
    }

    if (global == gcvFALSE &&
        Hardware->options.powerManagement == gcvFALSE &&
        (Hardware->chipPowerState == gcvPOWER_ON || state != gcvPOWER_ON)) {
        status = gcvSTATUS_OK;
        goto OnError;
    }

    if (broadcast &&
        state == gcvPOWER_SUSPEND &&
        Hardware->chipPowerState == gcvPOWER_OFF) {
        /* Do nothing, do not change chipPowerState. */
        status = gcvSTATUS_OK;
        goto OnError;
    }

#if gcdPOWEROFF_TIMEOUT
    if (timeout && Hardware->nextPowerState == gcvPOWER_INVALID) {
        /* Delayed power state change is canceled. */
        status = gcvSTATUS_OK;
        goto OnError;
    }
#endif

    if (global) {
        if (state != gcvPOWER_ON) {
            /*
             * Switch to global non-ON (OFF, IDLE or SUSPEND), try acquire the
             * global semaphore if it has not been acquired.
             */
            status = gckOS_TryAcquireSemaphore(os, Hardware->globalSemaphore);

            if (status == gcvSTATUS_OK) {
                globalAcquired = gcvTRUE;
            } else if (status == gcvSTATUS_TIMEOUT) {
                /*
                 * Ignore and leave globalSemaphore not acquired in this try.
                 * In this condition, power state is changing between global
                 * OFF, SUSPEND and IDLE, which is allowed.
                 */
                gcmkASSERT(Hardware->chipPowerState != gcvPOWER_ON);
            } else {
                /* Other errors. */
                gcmkONERROR(status);
            }
        }
    } else {
        /* Try to acquire the global semaphore. */
        status = gckOS_TryAcquireSemaphore(os, Hardware->globalSemaphore);

        if (status == gcvSTATUS_TIMEOUT) {
            /* In global SUSPEND, IDLE, or OFF state. */
            gcmkONERROR(gckOS_ReleaseMutex(os, Hardware->powerMutex));
            mutexAcquired = gcvFALSE;

            if (broadcast) {
                /* Abort power state change. */
                status = gcvSTATUS_OK;
                goto OnError;
            }

            /*
             * Only ON_AUTO can run here.
             * ON_AUTO state can not be skipped, either can not change a global
             * state.
             * So we need to wait until global ON state here.
             */
            gcmkONERROR(gckOS_AcquireSemaphore(os, Hardware->globalSemaphore));
            globalAcquired = gcvTRUE;

            /* Acquire the power mutex. */
            gcmkONERROR(gckOS_AcquireMutex(os, Hardware->powerMutex, gcvINFINITE));
            mutexAcquired = gcvTRUE;

            if (Hardware->chipPowerState == state) {
                /* Done. */
                status = gcvSTATUS_OK;
                goto OnError;
            }
        } else {
            /* Check error. */
            gcmkONERROR(status);
        }

        /* We just check if in non-global state, but need not to acquire it. */
        gcmkONERROR(gckOS_ReleaseSemaphore(os, Hardware->globalSemaphore));
        globalAcquired = gcvFALSE;
    }

    if (Hardware->chipPowerState == gcvPOWER_ON) {
        /* Switch from power ON to other non-runnable states. */
        if (broadcast) {
            gctINT32 atomValue;

            /* Try to acquire the semaphore to block/sync with commit. */
            status = gckOS_TryAcquireSemaphore(os, command->powerSemaphore);

            if (gcmIS_ERROR(status)) {
                status = gcvSTATUS_CHIP_NOT_READY;
                goto OnError;
            }

            powerAcquired = gcvTRUE;

            /* Check commit atom, abort when commit is in progress. */
            gcmkONERROR(gckOS_AtomGet(Hardware->os, command->atomCommit, &atomValue));

            if (atomValue > 0) {
                status = gcvSTATUS_CHIP_NOT_READY;
                goto OnError;
            }
        } else {
            /* Acquire/wait the semaphore to block/sync with command commit. */
            gcmkONERROR(gckOS_AcquireSemaphore(os, command->powerSemaphore));
            powerAcquired = gcvTRUE;
        }
    }

    /* Do hardware power state change. */
    if (Hardware->chipPowerState < state) {
        /* On to off direction. */
        if (Hardware->hasQchannel)
            gcmkONERROR(_QchannelPowerOffDirection(Hardware, state, broadcast));
        else
            gcmkONERROR(_PmSetPowerOffDirection(Hardware, state, broadcast));
    } else {
        /* Off to on direction. */
        if (Hardware->hasQchannel)
            gcmkONERROR(_QchannelPowerOnDirection(Hardware, state, global));
        else
            gcmkONERROR(_PmSetPowerOnDirection(Hardware, state, global));
    }

    if (status == gcvSTATUS_CHIP_NOT_READY) {
        /* CHIP_NOT_READY is not an error, either not success. */
        goto OnError;
    }

    if (state == gcvPOWER_ON) {
        /* Switched to power ON from other non-runnable states. */
        gcmkONERROR(gckOS_ReleaseSemaphore(os, command->powerSemaphore));
        powerAcquired = gcvFALSE;

        if (global) {
            /*
             * Global semaphore should be acquired already before, when
             * global OFF, IDLE or SUSPEND.
             */
            status = gckOS_TryAcquireSemaphore(os, Hardware->globalSemaphore);
            if (status != gcvSTATUS_TIMEOUT && Hardware->isLastPowerGlobal)
                gcmkPRINT("%s: global state error", __FUNCTION__);

            /* Switched to global ON, now release the global semaphore. */
            gcmkONERROR(gckOS_ReleaseSemaphore(os, Hardware->globalSemaphore));
            globalAcquired = gcvFALSE;
        }
    }

    gckSTATETIMER_Accumulate(&Hardware->powerStateCounter, Hardware->chipPowerState);

    /* Save the new power state. */
    Hardware->chipPowerState = state;
    Hardware->isLastPowerGlobal = global;

#if gcdDVFS
    if (state == gcvPOWER_ON && Hardware->kernel->dvfs)
        gckDVFS_Start(Hardware->kernel->dvfs);
#endif

#if gcdPOWEROFF_TIMEOUT
    if (!broadcast) {
        /*
         * Cancel delayed power state change.
         * Stop timer is not as good as set as no state change. Timer may run
         * into this function already when try to stop the timer.
         */
        Hardware->nextPowerState = gcvPOWER_INVALID;
    }

    if (Hardware->powerOffTimeout &&
        (state == gcvPOWER_IDLE || state == gcvPOWER_SUSPEND)) {
        /* Delayed power off. */
        Hardware->nextPowerState = gcvPOWER_OFF_TIMEOUT;

        /* Start a timer to power off GPU when GPU enters IDLE or SUSPEND. */
        gcmkVERIFY_OK(gckOS_StartTimer(os, Hardware->powerStateTimer,
                                       Hardware->powerOffTimeout));
    }
#endif

    /* Release the power mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(os, Hardware->powerMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (powerAcquired) {
        /* Release powerSemaphore. */
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os,
                                             command->powerSemaphore));
    }

    if (globalAcquired) {
        /* Release globalSemaphore. */
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os,
                                             Hardware->globalSemaphore));
    }

    if (mutexAcquired) {
        /* Release powerMutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os,
                                         Hardware->powerMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryPowerStateUnlocked
 **
 **  Get GPU power state without locking the powerMutex. Will be used in context
 **  where the powerMutex is already taken.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gceCHIPPOWERSTATE *State
 **          Power State.
 **
 */
gceSTATUS
gckHARDWARE_QueryPowerStateUnlocked(gckHARDWARE Hardware, gceCHIPPOWERSTATE *State)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(State != gcvNULL);

    /* Return the state. */
    *State = Hardware->chipPowerState;

    /* Success. */
    gcmkFOOTER_ARG("*State=%d", *State);
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryPowerState
 **
 **  Get GPU power state.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gceCHIPPOWERSTATE *State
 **          Power State.
 **
 */
gceSTATUS
gckHARDWARE_QueryPowerState(gckHARDWARE Hardware, gceCHIPPOWERSTATE *State)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(State != gcvNULL);

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    status = gckHARDWARE_QueryPowerStateUnlocked(Hardware, State);

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    /* Return the status. */
    gcmkFOOTER_ARG("*State=%d", *State);
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryPowerManagement
 **
 **  Query GPU power management function.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      gctBOOL *Enable
 **          Power Management Enabling State.
 **
 */
gceSTATUS
gckHARDWARE_QueryPowerManagement(gckHARDWARE Hardware, gctBOOL *Enable)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    *Enable = Hardware->options.powerManagement;

    gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));
    acquired = gcvFALSE;

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_EnablePowerManagement
 **
 **  Configure GPU power management function.
 **  Only used in driver initialization stage.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **      gctBOOL Enable
 **          Power Management Enabling State.
 **
 */
gceSTATUS
gckHARDWARE_EnablePowerManagement(gckHARDWARE Hardware, gctBOOL Enable)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    Hardware->options.powerManagement = Enable;

    /* Success. */
    status = gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_SetGpuProfiler
 **
 **  Configure GPU profiler function.
 **  Only used in driver initialization stage.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **      gctBOOL GpuProfiler
 **          GOU Profiler State.
 **
 */
gceSTATUS
gckHARDWARE_SetGpuProfiler(gckHARDWARE Hardware, gctBOOL GpuProfiler)
{
    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

#ifdef GC_MODULE_POWER_CONTROLS_Address
    if (GpuProfiler == gcvTRUE) {
        gctUINT32 data = 0;

        /* Disable auto pulse eater for SH. If 0, SH will be pulse eaten same as the core.*/
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_PULSE_EATER_Address,
                                            &data));

        data = gcmSETFIELD(data, GC_PULSE_EATER,
                           ENABLE_AUTO_PULSE_SH, 0);

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_PULSE_EATER_Address,
                                            data));

        /* Need to disable clock gating when doing profiling. */
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                           Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                           &data));

        data = gcmSETFIELD(data, GC_MODULE_POWER_CONTROLS,
                           ENABLE_MODULE_CLOCK_GATING, 0);

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                            data));
    } else {
        gctUINT32 data = 0;

        /* Enable auto pulse eater for SH when disable profile.*/
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_PULSE_EATER_Address,
                                            &data));

        data = gcmSETFIELD(data, GC_PULSE_EATER,
                           ENABLE_AUTO_PULSE_SH, 1);

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_PULSE_EATER_Address,
                                            data));

        /* enable clock gating when disable profile. */
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                           Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                           &data));

        data = gcmSETFIELD(data, GC_MODULE_POWER_CONTROLS,
                           ENABLE_MODULE_CLOCK_GATING, 1);

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                            data));
    }
#endif

    if (GpuProfiler == gcvTRUE)
        Hardware->waitCount = 200 * 100;
    else
        Hardware->waitCount = 200;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

#if gcdENABLE_FSCALE_VAL_ADJUST
gceSTATUS
gckHARDWARE_SetFscaleValue(gckHARDWARE Hardware, gctUINT32 FscaleValue, gctUINT32 ShaderFscaleValue)
{
    gceSTATUS status;
    gctUINT32 clock;
    gctUINT64 powerManagement = 0;
    gctBOOL acquired = gcvFALSE;
    gctBOOL commitMutexAcquired = gcvFALSE;
    gceCHIPPOWERSTATE statesStored, state;

    gcmkHEADER_ARG("Hardware=0x%x FscaleValue=%d",
                   Hardware, FscaleValue);

    gcmkVERIFY_ARGUMENT(FscaleValue > 0 && FscaleValue <= 64);

    gcmkONERROR(gckOS_AcquireMutex(Hardware->kernel->os,
                                   Hardware->kernel->device->commitMutex,
                                   gcvINFINITE));

    commitMutexAcquired = gcvTRUE;

    gcmkONERROR(gckCOMMAND_Stall(Hardware->kernel->command, gcvFALSE));

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    Hardware->powerOnFscaleVal = FscaleValue;

    if (ShaderFscaleValue != ~0U &&
        ShaderFscaleValue > 0 &&
        ShaderFscaleValue <= 64) {
        Hardware->powerOnShaderFscaleVal = ShaderFscaleValue;
    }
    if (Hardware->chipPowerState == gcvPOWER_ON) {
        gctUINT32 data;

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         Hardware->powerBaseAddress +
                                             GC_MODULE_POWER_MODULE_CONTROL_Address,
                                         &data));

        /* Disable all clock gating. */
        gcmkONERROR(gckOS_WriteRegisterEx(
            Hardware->os, Hardware->kernel,
            Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
            gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_FE, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_DE, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_PE, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_SH, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_PA, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_SE, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_RA, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_TX, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_VG, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_IM, 1) |
                gcmSETFIELD(0, GC_MODULE_POWER_MODULE_CONTROL, DISABLE_MODULE_CLOCK_GATING_TS, 1)));

        /* Scale the core clock. */
        clock = gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK3D_DIS,            0) |
                gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, CLK2D_DIS,            0) |
                gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_VAL, FscaleValue) |
                gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD,      1);

        clock = gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address, clock));

        /* Done loading the frequency scaler. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL,
                                                      FSCALE_CMD_LOAD, 0)));

        /* A option to support shader clock scaling. */
        if (ShaderFscaleValue != ~0U && ShaderFscaleValue > 0 && ShaderFscaleValue <= 64) {
            /* Scale the shader clock. */
            clock = gcmSETFIELD(0, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH,          0) |
                    gcmSETFIELD(0, GC_PULSE_EATER, DISABLE_AUTO_PULSE,            1) |
                    gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_VAL_SH, ShaderFscaleValue) |
                    gcmSETFIELD(0, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH,            1);

            gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                              GC_PULSE_EATER_Address, clock));

            /* Done loading the frequency scaler. */
            gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                              GC_PULSE_EATER_Address,
                                              gcmSETFIELD(clock, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH, 0)));
        }

        /* Restore all clock gating. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          Hardware->powerBaseAddress + GC_MODULE_POWER_MODULE_CONTROL_Address,
                                          data));
    }

    gcmkVERIFY(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));
    acquired = gcvFALSE;

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

    gcmkONERROR(gckOS_ReleaseMutex(Hardware->kernel->os, Hardware->kernel->device->commitMutex));
    commitMutexAcquired = gcvFALSE;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    if (commitMutexAcquired) {
        gcmkONERROR(gckOS_ReleaseMutex(Hardware->kernel->os,
                                       Hardware->kernel->device->commitMutex));
    }
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_GetFscaleValue(gckHARDWARE Hardware, gctUINT *FscaleValue,
                           gctUINT *MinFscaleValue, gctUINT *MaxFscaleValue)
{
    *FscaleValue = Hardware->powerOnFscaleVal;
    *MinFscaleValue = Hardware->minFscaleValue;
    *MaxFscaleValue = 64;

    return gcvSTATUS_OK;
}

gceSTATUS
gckHARDWARE_SetMinFscaleValue(gckHARDWARE Hardware, gctUINT MinFscaleValue)
{
    if (MinFscaleValue >= 1 && MinFscaleValue <= 64)
        Hardware->minFscaleValue = MinFscaleValue;

    return gcvSTATUS_OK;
}
#endif

gceSTATUS
gckHARDWARE_QueryIdle(gckHARDWARE Hardware, gctBOOL_PTR IsIdle)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 idle;
    gctBOOL isIdle = gcvFALSE;

#if gcdINTERRUPT_STATISTIC
    gckEVENT eventObj = Hardware->kernel->eventObj;
    gctINT32 pendingInterrupt;
#endif

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(IsIdle != gcvNULL);

#if gcdCAPTURE_ONLY_MODE
    *IsIdle = gcvTRUE;
    gcmkONERROR(status);
#endif

    do {
        /* We are idle when the power is not ON. */
        if (Hardware->chipPowerState != gcvPOWER_ON) {
            isIdle = gcvTRUE;
            break;
        }

        if (Hardware->mcFE) {
            gctBOOL isIdle;

            gcmkONERROR(gckMCFE_HardwareIdle(Hardware, &isIdle));

            if (!isIdle)
                break;
        } else {
#if !gcdSECURITY
            gctADDRESS address;
            gctUINT32 addressLow, addressHi = 0;
#endif
            gckCOMMAND command = Hardware->kernel->command;
            gctUINT64 wlSize = 16;

            /* Read idle register. */
            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             AQ_HI_IDLE_Address, &idle));

            if (command->feType == gcvHW_FE_END) {
                if ((idle | (1 << AQ_HI_IDLE_IDLE_MC_Start)) != 0x7fffffff) {
                    /* Something is busy. */
                    break;
                }
            } else {
                /* Pipe must be idle. */
                if ((idle | (1 << AQ_HI_IDLE_IDLE_MC_Start)) != 0x7ffffffe) {
                    /* Something is busy. */
                    break;
                }

#if gcdSECURITY
                isIdle = gcvTRUE;
                break;
#else
                /* Read the current FE address. */
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));

                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 AQFE_DEBUG_CUR_CMD_ADR_Address, &addressLow));
                address = addressLow;

                if (Hardware->graphicsLargeVA) {
                    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

                    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQFE_DEBUG_CUR_CMD_ADDR_HI_Address, &addressHi));

                    address = ((gctADDRESS)addressHi << 32) | addressLow;

                    wlSize = 32;
                }

                /* Test if address is inside the last WAIT/LINK sequence. */
                if (address < Hardware->lastWaitLink ||
                    (address > (gctUINT64)Hardware->lastWaitLink + wlSize)) {
                    /* FE is not in WAIT/LINK yet. */
                    break;
                }
            }
#endif
        } /* end of else */

#if gcdINTERRUPT_STATISTIC
        gcmkONERROR(gckOS_AtomGet(Hardware->os, eventObj->interruptCount, &pendingInterrupt));

        if (pendingInterrupt) {
            /* Pending interrupts, not idle. */
            break;
        }

        if (Hardware->asyncFE) {
            gckEVENT asyncEvent = Hardware->kernel->asyncEvent;

            gcmkONERROR(gckOS_AtomGet(Hardware->os, asyncEvent->interruptCount, &pendingInterrupt));

            if (pendingInterrupt) {
                /* Pending async FE interrupts, not idle. */
                break;
            }
        }
#endif

        /* Is really idle. */
        isIdle = gcvTRUE;
    } while (gcvFALSE);

    *IsIdle = isIdle;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

#if PHYTIUM_PROFILER
/*******************************************************************************
 * Handy macros that will help in reading those debug registers.
 */
#define gcmkREAD_DEBUG_REGISTER_PART1(control, block, index, data)                                     \
    do {                                                                                               \
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,                              \
                                          GC_DEBUG_CONTROL##control##_Address,                         \
                                          gcmSETFIELD(0, GC_DEBUG_CONTROL##control, block, index)));   \
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,                               \
                                         GC_DEBUG_SIGNALS_##block##_Address,                           \
                                         &profiler_part1->data));                                      \
    } while (0)

#define gcmkREAD_DEBUG_REGISTER_PART2(control, block, index, data)                                     \
    do {                                                                                               \
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,                              \
                                          GC_DEBUG_CONTROL##control##_Address,                         \
                                          gcmSETFIELD(0, GC_DEBUG_CONTROL##control, block, index)));   \
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,                               \
                                         GC_DEBUG_SIGNALS_##block##_Address,                           \
                                         &profiler_part2->data));                                      \
    } while (0)

#define gcmkREAD_DEBUG_REGISTER_N(control, block, index, data)                                         \
    do {                                                                                               \
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,                              \
                                          GC_DEBUG_CONTROL##control##_Address,                         \
                                          gcmSETFIELD(0, GC_DEBUG_CONTROL##control, block, index)));   \
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,                               \
                                         GC_DEBUG_SIGNALS_##block##_Address,                           \
                                         &data));                                                      \
    } while (0)

#define gcmkRESET_DEBUG_REGISTER(control, block, value)                                                \
    do {                                                                                               \
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,                              \
                                          GC_DEBUG_CONTROL##control##_Address,                         \
                                          gcmSETFIELD(0, GC_DEBUG_CONTROL##control, block, value)));   \
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,                              \
                                          GC_DEBUG_CONTROL##control##_Address,                         \
                                          gcmSETFIELD(0, GC_DEBUG_CONTROL##control, block, 0)));       \
    } while (0)

#define gcmkUPDATE_PROFILE_DATA(data) \
    profilerHistroy->data += profiler->data

gceSTATUS
gckHARDWARE_QueryContextProfile(gckHARDWARE Hardware, gctBOOL Reset,
                                gcsPROFILER_COUNTERS*Counters)
{
    gceSTATUS status;
    gckCOMMAND command = Hardware->kernel->command;
    gcsPROFILER_COUNTERS *profiler = Counters;

    gcmkHEADER_ARG("Hardware=%p Counters_part1=%p, counters=%p",
                   Hardware, Counters, Counters);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Acquire the context sequnence mutex. */
    gcmkONERROR(gckOS_AcquireMutex(command->os, command->mutexContextSeq, gcvINFINITE));

    /* Read the counters. */
    gcmkVERIFY_OK(gckOS_MemCopy(profiler,
                                &Hardware->kernel->profiler.histroyProfiler,
                                gcmSIZEOF(gcsPROFILER_COUNTERS)));

    /* Reset counters. */
    if (Reset) {
        gcmkVERIFY_OK(gckOS_ZeroMemory(&Hardware->kernel->profiler.histroyProfiler,
                                       gcmSIZEOF(gcsPROFILER_COUNTERS)));
    }

    gcmkVERIFY_OK(gckOS_ReleaseMutex(command->os, command->mutexContextSeq));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_UpdateContextProfile(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gcsPROFILER_COUNTERS *profiler = &Hardware->kernel->profiler.latestProfiler;
    gcsPROFILER_COUNTERS *profilerHistroy = &Hardware->kernel->profiler.histroyProfiler;
    gctUINT32 i;
    gctUINT32 clock;
    gctUINT32 totalRead, totalWrite;
    gckCOMMAND command = Hardware->kernel->command;
    gctBOOL mutexAcquired = gcvFALSE;
    gctUINT32 loopcount = 0;

    gcmkHEADER_ARG("Hardware=0x%x", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    /* Acquire the context sequnence mutex. */
    gcmkONERROR(gckOS_AcquireMutex(command->os, command->mutexContextSeq, gcvINFINITE));
    mutexAcquired = gcvTRUE;

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GC_TOTAL_IDLE_CYCLES_Address,
                                     &profiler->hi_total_idle_cycle_count));

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GC_TOTAL_CYCLES_Address,
                                     &profiler->hi_total_cycle_count));

    gcmkUPDATE_PROFILE_DATA(hi_total_cycle_count);
    gcmkUPDATE_PROFILE_DATA(hi_total_idle_cycle_count);

    /* Read clock control register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &clock));

    profiler->hi_total_read_8B_count = 0;
    profiler->hi_total_write_8B_count = 0;
    profiler->hi_total_readOCB_16B_count = 0;
    profiler->hi_total_writeOCB_16B_count = 0;

    loopcount = Hardware->clusterCount;


    if (Hardware->type == gcvHARDWARE_VIP)
        loopcount = 1;

    for (i = 0; i < loopcount; ++i) {
        gctUINT32 clusterMask;

        /* check vip to avoid cluster mask is 0. */
        if (Hardware->type != gcvHARDWARE_VIP)
            gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));
        else
            clusterMask = 0x1;

        if (!((1 << i) & clusterMask))
            continue;

        /* Select proper pipe. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, i)));

        /* BW */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_READS_Address, &totalRead));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_WRITES_Address, &totalWrite));

        profiler->hi_total_read_8B_count += totalRead;
        profiler->hi_total_write_8B_count += totalWrite;

        /* OCB-only BW */
        if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_OCB_COUNTER)) {
            if (Hardware->identity.customerID == 0x7e ||
                Hardware->identity.customerID == 0x7d) {
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 GC_TOTAL_READS_OCB_Address, &totalRead));
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 GC_TOTAL_WRITES_OCB_Address, &totalWrite));
            } else {
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 GC_AHB_TOTAL_READS_OCB_Address, &totalRead));
                gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                                 GC_AHB_TOTAL_WRITES_OCB_Address, &totalWrite));
            }
        } else {
            totalRead  = 0;
            totalWrite = 0;
        }

        profiler->hi_total_readOCB_16B_count += totalRead;
        profiler->hi_total_writeOCB_16B_count += totalWrite;
    }

    gcmkUPDATE_PROFILE_DATA(hi_total_read_8B_count);
    gcmkUPDATE_PROFILE_DATA(hi_total_write_8B_count);
    gcmkUPDATE_PROFILE_DATA(hi_total_readOCB_16B_count);
    gcmkUPDATE_PROFILE_DATA(hi_total_writeOCB_16B_count);

    /* Reset clock control register. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address, clock));

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_IDLE_CYCLES_Address, 0));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_DBG_CYCLE_COUNTER_Address, 0));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_CYCLES_Address, 0));

    /* Reset bandwidth counters. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_RESET_MEM_COUNTERS_Address, 1));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_RESET_MEM_COUNTERS_Address, 0));

    gcmkVERIFY_OK(gckOS_ReleaseMutex(command->os, command->mutexContextSeq));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (mutexAcquired)
        gckOS_ReleaseMutex(command->os, command->mutexContextSeq);

    /* Return the status. */
    gcmkFOOTER();
    return status;
}
#endif

gceSTATUS
gckHARDWARE_InitProfiler(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT32 control;

    gcmkHEADER_ARG("Hardware=%p", Hardware);
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &control));
    /* Enable debug register. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address,
                                      gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL,
                                                  DISABLE_DEBUG_REGISTERS, 0)));

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_ResetGPU(gckHARDWARE Hardware, gctBOOL NeedWR)
{
#if defined AQ_HI_CLOCK_CONTROL_SOFT_RESET
    gctUINT32 control, idle;
    gceSTATUS status;
    gctUINT32 count = 0;
    gctUINT32 wrCount = 2;
    gctUINT32 mmuEnabled;
    gckOS Os = Hardware->os;
#if !defined(EMULATOR) && !defined(LINUXEMULATOR)
    gctUINT32 reset_done;
#endif

    wrCount = NeedWR ? 2 : 1;

    while (count < wrCount) {
        /* Disable clock gating. */
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel,
                                          Hardware->powerBaseAddress +
                                              GC_MODULE_POWER_MODULE_CONTROL_Address,
                                          GC_MODULE_POWER_MODULE_CONTROL_ResetValue));

        control = gcmSETFIELD(GC_PULSE_EATER_ResetValue, GC_PULSE_EATER, DISABLE_AUTO_PULSE, 1);

        /* Disable pulse-eater. */
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, GC_PULSE_EATER_Address, control));

        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, GC_PULSE_EATER_Address,
                                          gcmSETFIELD(control, GC_PULSE_EATER,
                                                      FSCALE_CMD_LOAD_SH, 1)));

        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, GC_PULSE_EATER_Address, control));

        control = gcmSETFIELD(AQ_HI_CLOCK_CONTROL_ResetValue,
                              AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1);

        control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);


        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, control));

        control = gcmSETFIELD(AQ_HI_CLOCK_CONTROL_ResetValue,
                              AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);

        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address,
                                          control));

        /* Wait for clock being stable. */
        gcmkONERROR(gckOS_Delay(Os, 1));

        /* Isolate the GPU. */
        control = gcmSETFIELD(AQ_HI_CLOCK_CONTROL_ResetValue,
                              AQ_HI_CLOCK_CONTROL, ISOLATE_GPU, 1);

        control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, Hardware->aliveCluster0Index);

        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, control));

        /* Set soft reset. */
        if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB) &&
            Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, GCREG_HI_AHB_CONTROL_Address,
                                              gcmSETFIELDVALUE(0, GCREG_HI_AHB_CONTROL, SOFT_RESET, ENABLE)));
        } else {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address,
                                              gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, SOFT_RESET, 1)));
        }

#if !defined(EMULATOR) && !defined(LINUXEMULATOR)
        if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BIT_VGPU)) {
            do {
#if gcdFPGA_BUILD
                gckOS_Delay(Os, 10);
#else
                gckOS_Udelay(Os, 10);
#endif
                gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, GCREG_HI_AHB_CONTROL_Address, &reset_done));
            } while (!gcmGETFIELD(reset_done, GCREG_HI_AHB_CONTROL, SOFT_RESET_DONE));
        }
#endif

        if (Hardware->hasQchannel) {
            /* Reset Qchannel. */
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, AQ_QCHANNEL_POWER_CONTROL_Address,
                                              gcmSETFIELD(0, AQ_QCHANNEL_POWER_CONTROL, SOFT_RESET_QCHANNEL, 1)));
        }

#if gcdFPGA_BUILD
        /* Wait more time on FPGA for reset as lower frequency */
        gcmkONERROR(gckOS_Delay(Os, 10));
# endif

        /* Release soft reset. */
        if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB) &&
            Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel,
                                              GCREG_HI_AHB_CONTROL_Address,
                                              gcmSETFIELDVALUE(0, GCREG_HI_AHB_CONTROL, SOFT_RESET, DISABLE)));
        } else {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel,
                                              AQ_HI_CLOCK_CONTROL_Address,
                                              gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, SOFT_RESET, 0)));
        }

#if gcdFPGA_BUILD
        /* Wait more time on FPGA for reset as lower frequency */
        gcmkONERROR(gckOS_Delay(Os, 10));
# else
        /* Wait for reset. */
        gcmkONERROR(gckOS_Delay(Os, 1));
# endif

        if (Hardware->hasQchannel) {
            Hardware->powerState = gcvFALSE;

            gcmkONERROR(gckHARDWARE_QchannelPowerControl(Hardware, gcvTRUE, gcvTRUE));

            /* Bypass Qchannel power management after reset. */
            if (!Hardware->options.powerManagement)
                gcmkONERROR(gckHARDWARE_QchannelBypass(Hardware, gcvTRUE));
        }

        /* Reset GPU isolation. */
        control = gcmSETFIELD(control, AQ_HI_CLOCK_CONTROL, ISOLATE_GPU, 0);

        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address, control));

        /* Read idle register. */
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQ_HI_IDLE_Address, &idle));

        if (gcmGETFIELD(idle, AQ_HI_IDLE, IDLE_FE) == 0)
            continue;

        gcmkDUMP(Os, "@[register.wait 0x%05X 0x%08X 0x%08X]",
                 AQ_HI_IDLE_Address,
                 gcmSETFIELD(0, AQ_HI_IDLE, IDLE_FE, ~0U), idle);

        /* Read reset register. */
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, &control));
        if ((gcmGETFIELD(control, AQ_HI_CLOCK_CONTROL, IDLE3_D) == 0) ||
            (gcmGETFIELD(control, AQ_HI_CLOCK_CONTROL, IDLE2_D) == 0))
            continue;

        gcmkDUMP(Os, "@[register.wait 0x%05X 0x%08X 0x%08X]",
                 AQ_HI_CLOCK_CONTROL_Address,
                 gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, IDLE3_D, ~0U) |
                     gcmSETFIELD(0, AQ_HI_CLOCK_CONTROL, IDLE2_D, ~0U),
                 control);

        /* Force Disable MMU to guarantee setup command be read from physical addr */
        if (Hardware->options.secureMode == gcvSECURE_IN_NORMAL) {
            gctUINT32 regMmuCtrl = 0;

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             GCREG_MMUAHB_CONTROL_Address, &regMmuCtrl));

            mmuEnabled = gcmGETFIELD(regMmuCtrl, GCREG_MMUAHB_CONTROL, MMU);
        } else {
            gctUINT32 regMmuCtrl = 0;

            gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                             GCREG_MMU_CONTROL_Address, &regMmuCtrl));

            mmuEnabled = gcmGETFIELD(regMmuCtrl, GCREG_MMU_CONTROL, ENABLE);
        }

        if (mmuEnabled) {
            /* Not reset properly, reset again. */
            continue;
        }

        count++;
    }

    /* Success. */
    return gcvSTATUS_OK;

OnError:

    /* Return the error. */
    return status;
#else
    /* Not supported. */
    return gcvSTATUS_NOT_SUPPORTED;
#endif
}

gceSTATUS
gckHARDWARE_Reset(gckHARDWARE Hardware, gctBOOL ForceSoftReset)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL powerManagement = gcvFALSE;
    gctBOOL globalAcquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_OBJECT(Hardware->kernel, gcvOBJ_KERNEL);

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    /* Grab the global semaphore. */
    gcmkONERROR(gckOS_AcquireSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvTRUE;

    /* Record context ID in debug register before reset. */
    gcmkONERROR(gckHARDWARE_UpdateContextID(Hardware));

#if gcdENABLE_AHBXTTD
    if (Hardware->core == gcvCORE_MAJOR)
        gcmkONERROR(gckHARDWARE_ResetAHBXTTD(gcvNULL, Hardware->kernel));
#endif

    /* Hardware reset. */
    if (ForceSoftReset) {
        gcmkONERROR(_ResetGPU(Hardware, gcvFALSE));
    } else {
        /* Hardware reset. */
        status = gckOS_ResetGPU(Hardware->os, Hardware->kernel);

        if (gcmIS_ERROR(status)) {
            if (Hardware->identity.chipRevision < 0x4600) {
                /* Not supported - we need the isolation bit. */
                gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
            }

            /* Soft reset. */
            gcmkONERROR(_ResetGPU(Hardware, gcvTRUE));
        }
    }

    /* Initialize hardware. */
    gcmkONERROR(gckHARDWARE_InitializeHardware(Hardware));

    /* Force the command queue to reload the next context. */
    Hardware->kernel->command->currContext = gcvNULL;

    Hardware->kernel->command->running = gcvFALSE;

    gcmkONERROR(gckCOMMAND_Start(Hardware->kernel->command));

    /* Release the global semaphore. */
    gcmkONERROR(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvFALSE;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvTRUE));

    if (!ForceSoftReset)
        gcmkPRINT("[ftg340]: recovery done");

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkPRINT("[ftg340]: Hardware not reset successfully, give up");

    if (globalAcquired) {
        /* Release the global semaphore. */
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));
    }

    /* Return the error. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_IsFeatureAvailable
 **
 **  Verifies whether the specified feature is available in hardware.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 **      gceFEATURE Feature
 **          Feature to be verified.
 */
gceSTATUS
gckHARDWARE_IsFeatureAvailable(gckHARDWARE Hardware, gceFEATURE Feature)
{
    gctBOOL available;

    gcmkHEADER_ARG("Hardware=%p Feature=%d", Hardware, Feature);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    available = _QueryFeatureDatabase(Hardware, Feature);

    /* Return result. */
    gcmkFOOTER_ARG("%d", available ? gcvSTATUS_TRUE : gcvSTATUS_FALSE);
    return available ? gcvSTATUS_TRUE : gcvSTATUS_FALSE;
}

gceSTATUS
gckHARDWARE_QueryMcfe(gckHARDWARE Hardware,
                      const gceMCFE_CHANNEL_TYPE * Channels[],
                      gctUINT32 *Count)
{
    if (!_QueryFeatureDatabase(Hardware, gcvFEATURE_MCFE)) {
        /* No MCFE feature. */
        return gcvSTATUS_NOT_SUPPORTED;
    }

    if (Channels)
        *Channels = Hardware->mcfeChannels;

    if (Count)
        *Count = Hardware->mcfeChannelCount;

    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckHARDWARE_DumpMMUException
 **
 **  Dump the MMU debug info on an MMU exception.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_DumpMMUException(gckHARDWARE Hardware)
{
    gctUINT32 mmu = 0;
    gctUINT32 mmuStatus = 0;
    gctUINT32 addressLow = 0, addressHigh = 0;
    gctADDRESS address = 0;
    gctUINT32 i = 0, pipeCount = 0;
    gctUINT32 mmuExceptionHighAddress = GCREG_MMUAHB_EXCEPTION_HIGH_Address;
    gctUINT32 mmuStatusRegAddress;
    gctUINT32 mmuExceptionAddress;
    gctUINT32 clusterMask = 0;
    gceAREA_TYPE areaType = gcvAREA_TYPE_UNKNOWN;
#if gcdMMU_VERSION_1
    gctUINT32 mtlb = 0;
    gctUINT32 stlb = 0;
    gctUINT32 offset = 0;
    gctUINT32 stlbShift;
    gctUINT32 stlbMask;
    gctUINT32 pgoffMask;
#endif

    gcmkHEADER_ARG("Hardware=%p", Hardware);

#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->options.secureMode == gcvSECURE_IN_TA) {
        gcmkVERIFY_OK(gckKERNEL_SecurityDumpMMUException(Hardware->kernel));

        gckMMU_DumpRecentFreedAddress(Hardware->kernel->mmu);

        gcmkFOOTER_NO();
        return gcvSTATUS_OK;
    } else if (Hardware->options.secureMode == gcvSECURE_NONE) {
        mmuStatusRegAddress = GCREG_MMU_STATUS_Address;
        mmuExceptionAddress = GCREG_MMU_EXCEPTION_Address;
    } else {
        mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
        mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;
    }
#else
    if (Hardware->options.secureMode == gcvSECURE_NONE) {
        mmuStatusRegAddress = GCREG_MMU_STATUS_Address;
        mmuExceptionAddress = GCREG_MMU_EXCEPTION_Address;
    } else {
        mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
        mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;
    }
#endif

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkPRINT("GPU[%d] core%d (ChipModel=0x%x ChipRevision=0x%x):\n",
              Hardware->devID,
              Hardware->core,
              Hardware->identity.chipModel,
              Hardware->identity.chipRevision);

    gcmkPRINT("**************************\n");
    gcmkPRINT("***   MMU STATUS DUMP   ***\n");
    gcmkPRINT("**************************\n");

    gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                       mmuStatusRegAddress, &mmuStatus));

    gcmkPRINT("  MMU status = 0x%08X\n", mmuStatus);

    gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask);

    for (i = clusterMask; i & 1; i >>= 1)
       pipeCount++;

    if (!pipeCount)
        pipeCount = 8;

    for (i = 0; i < pipeCount; i += 1) {
        if (!((1 << i) & clusterMask))
            continue;

        mmu = mmuStatus & 0xF;
        mmuStatus >>= 4;

        /* Due to mmu status always 0 in vGPU. */
        if (mmu == 0 && Hardware->kernel->vGPUType == gcvVGPU_NONE)
            continue;

        switch (mmu) {
        case 0x1:
#if gcdMMU_VERSION_2
            gcmkPRINT("  MMU%d: PT not present\n", i);
#else
            gcmkPRINT("  MMU%d: slave not present\n", i);
#endif
            break;

        case 0x2:
#if gcdMMU_VERSION_2
            gcmkPRINT("  MMU%d: PDE0 not present\n", i);
#else
            gcmkPRINT("  MMU%d: page not present\n", i);
#endif
            break;

        case 0x3:
            gcmkPRINT("  MMU%d: write violation\n", i);
            break;

        case 0x4:
            gcmkPRINT("  MMU%d: out of bound", i);
            break;

        case 0x5:
            gcmkPRINT("  MMU%d: read security violation", i);
            break;

        case 0x6:
            gcmkPRINT("  MMU%d: write security violation", i);
            break;

        case 0x7:
            gcmkPRINT("  MMU%d: PDE1 not present", i);
            break;

        case 0x8:
            gcmkPRINT("  MMU%d: PDE2 not present", i);
            break;

        case 0x9:
            gcmkPRINT("  MMU%d: PT page fault", i);
            break;

        case 0xA:
            gcmkPRINT("  MMU%d: PDE0 page fault", i);
            break;

        default:
            if (Hardware->kernel->vGPUType == gcvVGPU_NONE)
                gcmkPRINT("  MMU%d: unknown state\n", i);
            else
                gcmkPRINT("  MMU%d: Always dump the value in exception register for vGPU\n", i);
        }

        if (Hardware->options.secureMode == gcvSECURE_NONE) {
            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               mmuExceptionAddress + i * 4, &addressLow));
        } else {
            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               mmuExceptionAddress, &addressLow));

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               mmuExceptionHighAddress, &addressHigh));
        }

        address = ((gctADDRESS)addressHigh << 32) | addressLow;

        gckMMU_GetAreaType(Hardware->kernel->mmu, address, &areaType);

        if (areaType == gcvAREA_TYPE_UNKNOWN) {
            gcmkPRINT("  MMU%d: exception address = 0x%llx, it is not mapped.\n", i, address);
            gcmkFOOTER_NO();
            return gcvSTATUS_OK;
        }

        gcmkPRINT("  MMU%d: exception address = 0x%llx\n", i, address);

#if gcdMMU_VERSION_1
        pgoffMask = (areaType == gcvAREA_TYPE_4K) ? gcdMMU_PAGE_4K_MASK : gcdMMU_PAGE_1M_MASK;
        stlbShift = (areaType == gcvAREA_TYPE_4K) ? gcdMMU_STLB_4K_SHIFT : gcdMMU_STLB_1M_SHIFT;
        stlbMask = (areaType == gcvAREA_TYPE_4K) ? gcdMMU_STLB_4K_MASK : gcdMMU_STLB_1M_MASK;

        mtlb = (address & gcdMMU_MTLB_MASK) >> gcdMMU_MTLB_SHIFT;
        stlb = (address & stlbMask) >> stlbShift;
        offset = address & pgoffMask;

        gcmkPRINT("    MTLB entry = %d\n", mtlb);

        gcmkPRINT("    STLB entry = %d\n", stlb);

        gcmkPRINT("    Offset = 0x%08X (%d)\n", offset, offset);
#endif

        gckMMU_DumpPageTableEntry(Hardware->kernel->mmu, areaType, address);

        gckMMU_DumpRecentFreedAddress(Hardware->kernel->mmu);
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

gceSTATUS
gckHARDWARE_HandleFault(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_NOT_SUPPORTED;
    gctUINT32 mmu, mmuStatus, i = 0, pipeCount = 0;
    gctUINT32 addressLow = 0, addressHigh = 0;
    gctADDRESS address;
    gctUINT32 clusterMask;
    gctUINT32 mmuStatusRegAddress;
    gctUINT32 mmuExceptionAddress;
    gctUINT32 mmuExceptionHighAddress = GCREG_MMUAHB_EXCEPTION_HIGH_Address;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    if (Hardware->options.secureMode == gcvSECURE_NONE) {
        mmuStatusRegAddress = GCREG_MMU_STATUS_Address;
        mmuExceptionAddress = GCREG_MMU_EXCEPTION_Address;
    } else {
        mmuStatusRegAddress = GCREG_MMUAHB_STATUS_Address;
        mmuExceptionAddress = GCREG_MMUAHB_EXCEPTION_Address;
    }

    gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

    /* Get MMU exception address. */
#if gcdENABLE_TRUST_APPLICATION
    if (Hardware->options.secureMode == gcvSECURE_IN_TA) {
        gckKERNEL_ReadMMUException(Hardware->kernel, &mmuStatus, &addressLow);
    } else {
#endif
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                           mmuStatusRegAddress, &mmuStatus));

        gcmkPRINT("  MMU status = 0x%08X\n", mmuStatus);

        for (i = clusterMask; i & 1; i >>= 1)
           pipeCount++;

        if (!pipeCount)
            pipeCount = 8;

        for (i = 0; i < pipeCount; i++) {
            if (!((1 << i) & clusterMask))
                continue;

            mmu = mmuStatus & 0xF;
            mmuStatus >>= 4;

            if (mmu == 0)
                continue;

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               mmuExceptionAddress, &addressLow));

            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               mmuExceptionHighAddress, &addressHigh));

            break;
        }
#if gcdENABLE_TRUST_APPLICATION
    }
#endif

    address = ((gctADDRESS)addressHigh << 32) | addressLow;

    if (address) {
        gckVIDMEM_NODE nodeObject = gcvNULL;
        gctSIZE_T offset = 0;
        gctPHYS_ADDR_T physicalAddress = 0;
        gceAREA_TYPE areaType;
        gctUINT32 pageMask;
        gcePAGE_TYPE pageType;
#if gcdMMU_VERSION_2
        gctUINT64_PTR  entry;
#else
        gctUINT32_PTR  entry;
#endif
        gckMMU_GetAreaType(Hardware->kernel->mmu, address, &areaType);

#if gcdMMU_VERSION_2
        pageMask = (areaType == gcvAREA_TYPE_4K) ? gcdMMU_PAGE_4K_MASK : gcdMMU_PAGE_2M_MASK;
        pageType = (areaType == gcvAREA_TYPE_4K) ? gcvPAGE_TYPE_4K : gcvPAGE_TYPE_2M;
#else
        pageMask = (areaType == gcvAREA_TYPE_4K) ? gcdMMU_PAGE_4K_MASK : gcdMMU_PAGE_1M_MASK;
        pageType = (areaType == gcvAREA_TYPE_4K) ? gcvPAGE_TYPE_4K : gcvPAGE_TYPE_1M;
#endif

#if gcdENABLE_TRUST_APPLICATION
        address &= ~gcdMMU_PAGE_4K_MASK;
#else
        address &= ~pageMask;
#endif

        /* Try to allocate memory and setup map for exception address. */
        gcmkONERROR(gckVIDMEM_NODE_Find(Hardware->kernel, address,
                                        &nodeObject, &offset));

        gcmkONERROR(gckVIDMEM_NODE_GetCPUPhysical(Hardware->kernel, nodeObject,
                                                  offset, &physicalAddress));

#if gcdENABLE_TRUST_APPLICATION
        if (Hardware->options.secureMode == gcvSECURE_IN_TA) {
            gckKERNEL_HandleMMUException(Hardware->kernel, mmuStatus,
                                         physicalAddress, address);
        } else {
#endif
            /* Setup page table. */
            gcmkONERROR(gckMMU_GetPageEntry(Hardware->kernel->mmu,
                                            pageType,
                                            (address < gcd4G_SIZE),
                                            address, (gctPOINTER *)&entry));

            gckMMU_SetPage(Hardware->kernel->mmu,
                           physicalAddress, pageType,
                           (address < gcd4G_SIZE),
                           gcvTRUE, entry);

            /* Resume hardware execution. */
            gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                                mmuExceptionAddress + i * 4, *entry));
#if gcdENABLE_TRUST_APPLICATION
        }
#endif
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_DumpMCFEState(gckOS Os, gckHARDWARE Hardware)
{
    gctUINT32 i, j, data = 0, array[8] = { 0 };
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER();

    gcmkPRINT("**************************\n");
    gcmkPRINT("*****   MCFE STATE   *****\n");
    gcmkPRINT("**************************\n");

    /* Fetch address of channels. */
    gcmkPRINT("Channel fetch addresses:\n");
    gcmkPRINT("     [00]        [01]        [02]        [03]\n");

    for (i = 0; i < 4; i++)
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x3 + 2 * i));

    for (i = 0; i < 16; i++) {
        for (j = 0; j < 4; j++) {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x2 + 2 * j));
            gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &array[j]));
        }

        gcmkPRINT("  0x%08X  0x%08X  0x%08X  0x%08X\n", array[0], array[1], array[2], array[3]);
    }

    /* Command data of channels. */
    gcmkPRINT_N(0, "Channel command data:\n");
    gcmkPRINT_N(0,
                "           [00]                    [01]                    [02]                    [03]\n");
    gcmkPRINT_N(0,
                "     [Low        High]       [Low        High]       [Low        High]       [Low        High]\n");

    for (i = 0; i < 4; i++) {
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x11 + 4 * i));
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x11 + 4 * i + 2));
    }

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 4; j++) {
            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x10 + 4 * j));
            gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &array[j * 2]));

            gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x10 + 4 * j + 2));
            gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &array[j * 2 + 1]));
        }

        gcmkPRINT_N(0, "  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X  0x%08X\n",
                    array[0], array[1], array[2], array[3], array[4], array[5], array[6], array[7]);
    }

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x00));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "0x00: 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x01));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "0x01: 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0A));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "WaitSemaphore: 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0B));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "WaitEventID(channel 0 and 1): 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0C));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "WaitEventID(channel 2 and 3): 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0D));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "DecodeState: 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0E));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "DebugSelect(0x0E): 0x%08X\n", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x0F));
    gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
    gcmkPRINT_N(0, "DebugSelect(0x0F): 0x%08X\n", data);

    for (i = 0; i < 9; i++) {
        gcmkONERROR(gckOS_WriteRegisterEx(Os, Hardware->kernel, 0x470, 0x20 + i));
        gcmkONERROR(gckOS_ReadRegisterEx(Os, Hardware->kernel, 0x450, &data));
        gcmkPRINT_N(0, "DebugSelect(0x%02X): 0x%08X\n", 0x20 + i, data);
    }

OnError:
    gcmkFOOTER();

    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_DumpGPUState
 **
 **  Dump the GPU debug registers.
 **
 **  INPUT:
 **
 **      gckHARDWARE Harwdare
 **          Pointer to an gckHARDWARE object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckHARDWARE_DumpGPUState(gckHARDWARE Hardware)
{
    static gctCONST_STRING _cmdState[] = {
        "PAR_IDLE_ST",    "PAR_DEC_ST",       "PAR_ADR0_ST",       "PAR_LOAD0_ST",
        "PAR_ADR1_ST",    "PAR_LOAD1_ST",     "PAR_3DADR_ST",      "PAR_3DCMD_ST",
        "PAR_3DCNTL_ST",  "PAR_3DIDXCNTL_ST", "PAR_INITREQDMA_ST", "PAR_DRAWIDX_ST",
        "PAR_DRAW_ST",    "PAR_2DRECT0_ST",   "PAR_2DRECT1_ST",    "PAR_2DDATA0_ST",
        "PAR_2DDATA1_ST", "PAR_WAITFIFO_ST",  "PAR_WAIT_ST",       "PAR_LINK_ST",
        "PAR_END_ST",     "PAR_STALL_ST",     "INVALID_PAR_ST",    "INVALID_PAR_ST",
        "INVALID_PAR_ST", "INVALID_PAR_ST",   "INVALID_PAR_ST",    "INVALID_PAR_ST",
        "INVALID_PAR_ST", "INVALID_PAR_ST",   "INVALID_PAR_ST",    "INVALID_PAR_ST"
    };

    static gctCONST_STRING _cmdDmaState[] = {
        "CMD_IDLE_ST", "CMD_START_ST", "CMD_REQ_ST", "CMD_END_ST" };

    static gctCONST_STRING _cmdFetState[] = {
        "FET_IDLE_ST", "FET_RAMVALID_ST", "FET_VALID_ST", "INVALID_FET_ST" };

    static gctCONST_STRING _reqDmaState[] = {
        "REQ_IDLE_ST", "REQ_WAITIDX_ST", "REQ_CAL_ST", "INVALID_REQ_ST" };

    static gctCONST_STRING _calState[] = {
        "CAL_IDLE_ST", "CAL_LDADR_ST", "CAL_IDXCALC_ST", "INVALID_CAL_ST" };

    static gctCONST_STRING _veReqState[] = {
        "VER_IDLE_ST", "VER_CKCACHE_ST", "VER_MISS_ST", "INVALID_VER_ST" };

    enum {
        RA_INDEX = 0,
        TX_INDEX = 1,
        FE_INDEX = 2,
        PE_INDEX = 3,
        DE_INDEX = 4,
        SH_INDEX = 5,
        PA_INDEX = 6,
        SE_INDEX = 7,
        MC_INDEX = 8,
        HI_INDEX = 9,
        TPG_INDEX = 10,
        TFB_INDEX = 11,
        USC_INDEX = 12,
        L2_INDEX  = 13,
        BLT_INDEX = 14,
        WD_INDEX  = 15,
        VTXDATA_INDEX = 16,
        DIR_INDEX = 17,
        PPA_INDEX = 18,
        NN_INDEX  = 19,
        QC_INDEX  = 20,
        MODULE_MAX_INDEX,
    };

    /* must keep order correctly for _dbgRegs, we need adjust some value base on the index */
    static gcsiDEBUG_REGISTERS _dbgRegs[MODULE_MAX_INDEX] = {
        { "RA",      0x474, 16, 0x448, 256, 0x1, 0x00, gcvTRUE,  gcvTRUE  },
        { "TX",      0x474, 24, 0x44C, 128, 0x1, 0x00, gcvTRUE,  gcvTRUE  },
        { "FE",      0x470,  0, 0x450, 256, 0x1, 0x00, gcvTRUE,  gcvFALSE },
        { "PE",      0x470, 16, 0x454, 256, 0x3, 0x00, gcvTRUE,  gcvTRUE  },
        { "DE",      0x470,  8, 0x458, 256, 0x1, 0x00, gcvTRUE,  gcvFALSE },
        { "SH",      0x470, 24, 0x45C, 256, 0x1, 0x00, gcvTRUE,  gcvTRUE  },
        { "PA",      0x474,  0, 0x460, 256, 0x1, 0x00, gcvTRUE,  gcvTRUE  },
        { "SE",      0x474,  8, 0x464, 256, 0x1, 0x00, gcvTRUE,  gcvTRUE  },
        { "MC",      0x478,  0, 0x468, 256, 0x3, 0x00, gcvTRUE,  gcvTRUE  },
        { "HI",      0x478,  8, 0x46C, 256, 0x1, 0x00, gcvTRUE,  gcvFALSE },
        { "TPG",     0x474, 24, 0x44C,  32, 0x1, 0x80, gcvFALSE, gcvTRUE  },
        { "TFB",     0x474, 24, 0x44C,  32, 0x1, 0xA0, gcvFALSE, gcvTRUE  },
        { "USC",     0x474, 24, 0x44C,  64, 0x1, 0xC0, gcvFALSE, gcvTRUE  },
        { "L2",      0x478,  0, 0x564, 256, 0x1, 0x00, gcvTRUE,  gcvFALSE },
        { "BLT",     0x478, 24, 0x1A4, 256, 0x1, 0x00, gcvFALSE, gcvTRUE  },
        { "WD",      0xF0,  16, 0xF4,  256, 0x1, 0x00, gcvFALSE, gcvFALSE },
        { "VTXDATA", 0x474, 24, 0x44C, 64,  0x1, 0x40, gcvFALSE, gcvTRUE  },
        { "DIR",     0xF0,  24, 0xF8,  256, 0x1, 0x00, gcvFALSE, gcvTRUE  },
        { "PPA",     0x474,  0, 0x598, 256, 0x1, 0x00, gcvFALSE, gcvTRUE  },
        { "NN",      0x474, 24, 0x44C, 256, 0x1, 0x00, gcvFALSE, gcvTRUE  },
        { "QC",      0x5E8,  4, 0x59C, 256, 0x1, 0x00, gcvFALSE, gcvFALSE },

    };

    static gctUINT32 _otherRegs[] = {
        0x040, 0x044, 0x04C, 0x050, 0x054, 0x058, 0x05C, 0x060,
        0x43c, 0x440, 0x444, 0x414, 0x100
    };

    gceSTATUS status;
    gctUINT32 idle = 0, axi = 0, hiControl = 0;
    gctADDRESS dmaAddress1 = 0, dmaAddress2 = 0;
    gctUINT32 dmaState1 = 0, dmaState2 = 0;
    gctUINT32 dmaLow = 0, dmaHigh = 0;
    gctUINT32 cmdState = 0, cmdDmaState = 0, cmdFetState = 0;
    gctUINT32 dmaReqState = 0, calState = 0, veReqState = 0;
    gctUINT i;
    gctUINT pipe = 0, pipeMask = 0x1;
    static const gctUINT maxNumOfPipes = 8;
    gctUINT32 control = 0, oldControl = 0;
    gckOS os = Hardware->os;
    gceSTATUS hwTFB = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HW_TFB);
    gceSTATUS usc = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_USC);
    gceSTATUS multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);
    gceSTATUS bltEngine = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE);
    gceSTATUS gsShader = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_GEOMETRY_SHADER);
    gceSTATUS nnEngine = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_NN_ENGINE);

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkPRINT_N(12,
                "GPU[%d] core%d (ChipModel=0x%x ChipRevision=0x%x):\n",
                Hardware->devID,
                Hardware->core,
                Hardware->identity.chipModel,
                Hardware->identity.chipRevision);

    switch (Hardware->identity.pixelPipes) {
    case 2:
        pipeMask = 0x3;
        break;
    case 1:
        pipeMask = 0x1;
        break;
    default:
        gcmkASSERT(0);
    }

    if (!Hardware->mcFE) {
        /* Verify whether DMA is running. */
        gcmkONERROR(_VerifyDMA(os, Hardware, &dmaAddress1, &dmaAddress2, &dmaState1, &dmaState2));

        cmdState = dmaState2 & 0x1F;
        cmdDmaState = (dmaState2 >> 8) & 0x03;
        cmdFetState = (dmaState2 >> 10) & 0x03;
        dmaReqState = (dmaState2 >> 12) & 0x03;
        calState = (dmaState2 >> 14) & 0x03;
        veReqState = (dmaState2 >> 16) & 0x03;

        gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQFE_DEBUG_CMD_LOW_REG_Address, &dmaLow));
        gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQFE_DEBUG_CMD_LOW_REG_Address, &dmaLow));
        gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQFE_DEBUG_CMD_HI_REG_Address, &dmaHigh));
        gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQFE_DEBUG_CMD_HI_REG_Address, &dmaHigh));
    }

    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQ_HI_IDLE_Address, &idle));
    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, &hiControl));
    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, AQ_AXI_STATUS_Address, &axi));
#ifdef __QNXNTO__
    SLOG_CRITICAL("[ftg340] Dumping GPU State to: %s", gpuLog);
#endif

    gcmkPRINT_N(0, "**************************\n");
    gcmkPRINT_N(0, "***   GPU STATE DUMP   ***\n");
    gcmkPRINT_N(0, "**************************\n");

    gcmkPRINT_N(4, "  axi      = 0x%08X\n", axi);

    gcmkPRINT_N(4, "  idle     = 0x%08X\n", idle);
    if ((idle & 0x00000001) == 0)
        gcmkPRINT_N(0, "    FE not idle\n");
    if ((idle & 0x00000002) == 0)
        gcmkPRINT_N(0, "    DE not idle\n");
    if ((idle & 0x00000004) == 0)
        gcmkPRINT_N(0, "    PE not idle\n");
    if ((idle & 0x00000008) == 0)
        gcmkPRINT_N(0, "    SH not idle\n");
    if ((idle & 0x00000010) == 0)
        gcmkPRINT_N(0, "    PA not idle\n");
    if ((idle & 0x00000020) == 0)
        gcmkPRINT_N(0, "    SE not idle\n");
    if ((idle & 0x00000040) == 0)
        gcmkPRINT_N(0, "    RA not idle\n");
    if ((idle & 0x00000080) == 0)
        gcmkPRINT_N(0, "    TX not idle\n");
    if ((idle & 0x00000100) == 0)
        gcmkPRINT_N(0, "    VG not idle\n");
    if ((idle & 0x00000200) == 0)
        gcmkPRINT_N(0, "    IM not idle\n");
    if ((idle & 0x00000400) == 0)
        gcmkPRINT_N(0, "    FP not idle\n");
    if ((idle & 0x00000800) == 0)
        gcmkPRINT_N(0, "    TS not idle\n");
    if ((idle & 0x00001000) == 0)
        gcmkPRINT_N(0, "    BL not idle\n");
    if ((idle & 0x00002000) == 0)
        gcmkPRINT_N(0, "    ASYNCFE not idle\n");
    if ((idle & 0x00004000) == 0)
        gcmkPRINT_N(0, "    MC not idle\n");
    if ((idle & 0x00008000) == 0)
        gcmkPRINT_N(0, "    PPA not idle\n");
    if ((idle & 0x00010000) == 0)
        gcmkPRINT_N(0, "    DC not idle\n");
    if ((idle & 0x00020000) == 0)
        gcmkPRINT_N(0, "    WD not idle\n");
    if ((idle & 0x00040000) == 0)
        gcmkPRINT_N(0, "    NN not idle\n");
    if ((idle & 0x00080000) == 0)
        gcmkPRINT_N(0, "    TP not idle\n");
    if ((idle & 0x80000000) != 0)
        gcmkPRINT_N(0, "    AXI low power mode\n");

    gcmkPRINT_N(4, "  AQ_HI_CLOCK_CONTROL  = 0x%08X\n", hiControl);

    if (!Hardware->mcFE) {
        if (dmaAddress1 == dmaAddress2 &&
            dmaState1 == dmaState2) {
            gcmkPRINT_N(0, "  DMA appears to be stuck at this address:\n");
            gcmkPRINT_N(4, "    0x%llx\n", dmaAddress1);
        } else {
            if (dmaAddress1 == dmaAddress2) {
                gcmkPRINT_N(0, "  DMA address is constant, but state is changing:\n");
                gcmkPRINT_N(4, "    0x%08X\n", dmaState1);
                gcmkPRINT_N(4, "    0x%08X\n", dmaState2);
            } else {
                gcmkPRINT_N(0, "  DMA is running; known addresses are:\n");
                gcmkPRINT_N(4, "    0x%llx\n", dmaAddress1);
                gcmkPRINT_N(4, "    0x%llx\n", dmaAddress2);
            }
        }

        gcmkPRINT_N(4, "  dmaLow   = 0x%08X\n", dmaLow);
        gcmkPRINT_N(4, "  dmaHigh  = 0x%08X\n", dmaHigh);
        gcmkPRINT_N(4, "  dmaState = 0x%08X\n", dmaState2);
        gcmkPRINT_N(8, "    command state       = %d (%s)\n", cmdState, _cmdState[cmdState]);
        gcmkPRINT_N(8, "    command DMA state   = %d (%s)\n", cmdDmaState, _cmdDmaState[cmdDmaState]);
        gcmkPRINT_N(8, "    command fetch state = %d (%s)\n", cmdFetState, _cmdFetState[cmdFetState]);
        gcmkPRINT_N(8, "    DMA request state   = %d (%s)\n", dmaReqState, _reqDmaState[dmaReqState]);
        gcmkPRINT_N(8, "    cal state           = %d (%s)\n", calState, _calState[calState]);
        gcmkPRINT_N(8, "    VE request state    = %d (%s)\n", veReqState, _veReqState[veReqState]);
    }

    gcmkPRINT_N(0, "  Debug registers:\n");

    if (bltEngine)
        _dbgRegs[BLT_INDEX].avail = gcvTRUE;
    if (hwTFB)
        _dbgRegs[TFB_INDEX].avail = gcvTRUE;
    if (usc)
        _dbgRegs[USC_INDEX].avail = gcvTRUE;
    if (gsShader)
        _dbgRegs[TPG_INDEX].avail = gcvTRUE;

    if (Hardware->hasQchannel)
        _dbgRegs[QC_INDEX].avail = gcvTRUE;

    if (multiCluster) {
        gctUINT32 clusterMask;

        gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

        _dbgRegs[WD_INDEX].avail = gcvTRUE;
        _dbgRegs[DIR_INDEX].avail = gcvTRUE;
        _dbgRegs[VTXDATA_INDEX].avail = gcvTRUE;
        _dbgRegs[PPA_INDEX].avail = gcvTRUE;
        _dbgRegs[FE_INDEX].index = 0xF0;
        _dbgRegs[HI_INDEX].index = 0xF0;
        /*spare 64 DWORDS debug values from TX for VTXDATA prefetch in USC */
        _dbgRegs[TX_INDEX].count = 64;

        for (i = 0; i < gcmCOUNTOF(_dbgRegs); i++) {
            if (_dbgRegs[i].inCluster) {
                _dbgRegs[i].pipeMask = clusterMask;
            }
        }
        pipeMask = clusterMask;
    }

    if (nnEngine) {
        _dbgRegs[NN_INDEX].avail = gcvTRUE;

        /* Disable clock gating when dump NN debug register. */
        gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os,
                                           Hardware->kernel,
                                           Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                           &oldControl));

        oldControl = gcmSETFIELD(oldControl,
                                 GC_MODULE_POWER_CONTROLS,
                                 ENABLE_MODULE_CLOCK_GATING,
                                 0);

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os,
                                            Hardware->kernel,
                                            Hardware->powerBaseAddress + GC_MODULE_POWER_CONTROLS_Address,
                                            oldControl));

        gcmkVERIFY_OK(gckOS_WriteRegisterEx(Hardware->os,
                                            Hardware->kernel,
                                            Hardware->powerBaseAddress + GCREG_AHB_POWER_CONTROL9_Address,
                                            (1 << 16) - 1));
    }

    for (i = 0; i < gcmCOUNTOF(_dbgRegs); i += 1)
        gcmkONERROR(_DumpDebugRegisters(os, Hardware->kernel, &_dbgRegs[i]));

    /* Record control. */
    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, 0x0, &oldControl));

    for (pipe = 0; pipe < maxNumOfPipes; pipe++) {
        if (((1 << pipe) & pipeMask) == 0)
            continue;

        gcmkPRINT_N(4, "    Other Registers[%d]:\n", pipe);

        /* Switch pipe. */
        gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, 0x0, &control));
        control &= ~(0xF << 20);
        control |= (pipe << 20);
        gcmkONERROR(gckOS_WriteRegisterEx(os, Hardware->kernel, 0x0, control));

        for (i = 0; i < gcmCOUNTOF(_otherRegs); i += 1) {
            gctUINT32 read;

            gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, _otherRegs[i], &read));
            gcmkPRINT_N(12, "      [0x%04X] 0x%08X\n", _otherRegs[i], read);
        }

        gcmkPRINT("    MMU status from MC[%d]:", pipe);
        gckHARDWARE_DumpMMUException(Hardware);
    }

    /* MCFE state. */
    if (Hardware->mcFE)
        gcmkVERIFY_OK(_DumpMCFEState(os, Hardware));

    /* Restore control. */
    gcmkONERROR(gckOS_WriteRegisterEx(os, Hardware->kernel, 0x0, oldControl));

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_HALTI0) &&
        !Hardware->mcFE) {
        /* FE debug register. */
        gcmkVERIFY_OK(_DumpLinkStack(os, Hardware->kernel, &_dbgRegs[2]));
    }

    _DumpFEStack(os, Hardware->kernel, &_dbgRegs[2]);

    gcmkPRINT_N(0, "**************************\n");
    gcmkPRINT_N(0, "*****   SW COUNTERS  *****\n");
    gcmkPRINT_N(0, "**************************\n");
    gcmkPRINT_N(4, "    Execute Count = 0x%08X\n", Hardware->executeCount);
    gcmkPRINT_N(4, "    Execute Addr  = 0x%llx\n", Hardware->lastExecuteAddress);
    gcmkPRINT_N(4, "    End     Addr  = 0x%llx\n", Hardware->lastEnd);

    /* dump stack. */
    gckOS_DumpCallStack(os);

OnError:

    /* Return the error. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_ReadAxiStatusRegister(IN gckHARDWARE Hardware, OUT gctUINT32_PTR Value)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Read the register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQ_AXI_STATUS_Address, Value));

    /* Success. */
    gcmkFOOTER_ARG("*Value=0x%x", *Value);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
gckHARDWARE_ReadPerformanceRegister(gckHARDWARE Hardware, gctUINT PerformanceAddress,
                                    gctUINT IndexAddress, gctUINT IndexShift, gctUINT Index, gctUINT32_PTR Value)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=%p PerformanceAddress=0x%x IndexAddress=0x%x IndexShift=%u Index=%u",
                   Hardware, PerformanceAddress, IndexAddress, IndexShift, Index);

    /* Write the index. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, IndexAddress, Index << IndexShift));

    /* Read the register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, PerformanceAddress, Value));

    /* Test for reset. */
    if (Index == 15) {
        /* Index another register to get out of reset. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, IndexAddress, 0));
    }

    /* Success. */
    gcmkFOOTER_ARG("*Value=0x%x", *Value);
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_GetFrameInfo(gckHARDWARE Hardware, gcsHAL_FRAME_INFO *FrameInfo)
{
    gceSTATUS status;
    gctUINT i, clock;
    gcsHAL_FRAME_INFO info;
    gctUINT32 clusterMask;
#if gcdFRAME_DB_RESET
    gctUINT reset;
#endif

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Get profile tick. */
    gcmkONERROR(gckOS_GetProfileTick(&info.ticks));

    /* Read SH counters and reset them. */
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    4, &info.shaderCycles));
/*1.24*/
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    9, &info.vsInstructionCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    12, &info.vsTextureCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    7, &info.psInstructionCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    14, &info.psTextureCount));
#if gcdFRAME_DB_RESET
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_SH_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_SH_Start,
                                                    15, &reset));
#endif

    /* Read PA counters and reset them. */
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    3, &info.vertexCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    4, &info.primitiveCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    7, &info.rejectedPrimitives));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    8, &info.culledPrimitives));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    6, &info.clippedPrimitives));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    5, &info.outPrimitives));
#if gcdFRAME_DB_RESET
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_PA_Start,
                                                    15, &reset));
#endif

    /* Read RA counters and reset them. */
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    3, &info.inPrimitives));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    11, &info.culledQuadCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    1, &info.totalQuadCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    2, &info.quadCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    0, &info.totalPixelCount));
#if gcdFRAME_DB_RESET
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_RA_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_RA_Start,
                                                    15, &reset));
#endif

    /* Read TX counters and reset them. */
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    0, &info.bilinearRequests));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    1, &info.trilinearRequests));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    8, &info.txHitCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    9, &info.txMissCount));
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    6, &info.txBytes8));
#if gcdFRAME_DB_RESET
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_TX_Address,
                                                    GC_DEBUG_CONTROL1_Address,
                                                    GC_DEBUG_CONTROL1_TX_Start,
                                                    15, &reset));
#endif

    /* Read clock control register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &clock));

    gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

    /* Walk through all available pixel pipes. */
    for (i = 0; i < Hardware->clusterCount; ++i) {
        if (!((1 << i) & clusterMask))
            continue;

        /* Select proper pipe. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL,
                                                      MULTI_PIPE_REG_SELECT, i)));

        /* Read cycle registers. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_IDLE_CYCLES_Address,
                                         &info.idleCycles[i]));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_CYCLES_Address,
                                         &info.cycles[i]));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_DBG_CYCLE_COUNTER_Address,
                                         &info.mcCycles[i]));

        /* Read bandwidth registers. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_READ_REQS_Address,
                                         &info.readRequests[i]));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_READS_Address,
                                         &info.readBytes8[i]));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_WRITE_REQS_Address,
                                         &info.writeRequests[i]));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_TOTAL_WRITES_Address,
                                         &info.writeBytes8[i]));

        /* Read PE counters. */
        gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                        GC_DEBUG_SIGNALS_PE_Address,
                                                        GC_DEBUG_CONTROL0_Address,
                                                        GC_DEBUG_CONTROL0_PE_Start,
                                                        0, &info.colorKilled[i]));
        gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                        GC_DEBUG_SIGNALS_PE_Address,
                                                        GC_DEBUG_CONTROL0_Address,
                                                        GC_DEBUG_CONTROL0_PE_Start,
                                                        2, &info.colorDrawn[i]));
        gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                        GC_DEBUG_SIGNALS_PE_Address,
                                                        GC_DEBUG_CONTROL0_Address,
                                                        GC_DEBUG_CONTROL0_PE_Start,
                                                        1, &info.depthKilled[i]));
        gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                        GC_DEBUG_SIGNALS_PE_Address,
                                                        GC_DEBUG_CONTROL0_Address,
                                                        GC_DEBUG_CONTROL0_PE_Start,
                                                        3, &info.depthDrawn[i]));
    }

    /* Zero out remaning reserved counters. */
    for (; i < 8; ++i) {
        info.readBytes8[i] = 0;
        info.writeBytes8[i] = 0;
        info.cycles[i] = 0;
        info.idleCycles[i] = 0;
        info.mcCycles[i] = 0;
        info.readRequests[i] = 0;
        info.writeRequests[i] = 0;
        info.colorKilled[i] = 0;
        info.colorDrawn[i] = 0;
        info.depthKilled[i] = 0;
        info.depthDrawn[i] = 0;
    }

    /* Reset clock control register. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_HI_CLOCK_CONTROL_Address, clock));

    /* Reset cycle and bandwidth counters. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GC_RESET_MEM_COUNTERS_Address, 1));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GC_RESET_MEM_COUNTERS_Address, 0));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GC_TOTAL_CYCLES_Address, 0));

#if gcdFRAME_DB_RESET
    /* Reset PE counters. */
    gcmkONERROR(gckHARDWARE_ReadPerformanceRegister(Hardware,
                                                    GC_DEBUG_SIGNALS_PE_Address,
                                                    GC_DEBUG_CONTROL0_Address,
                                                    GC_DEBUG_CONTROL0_PE_Start,
                                                    15, &reset));
#endif

    /* Copy to user. */
    gcmkONERROR(gckOS_CopyToUserData(Hardware->os, &info, FrameInfo, gcmSIZEOF(info)));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_DumpGpuProfile(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT   clock, i;
    gctUINT32 totalRead, totalWrite, read, write;
    gctUINT32 clusterMask;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Read clock control register. */
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     AQ_HI_CLOCK_CONTROL_Address, &clock));

    totalRead = 0;
    totalWrite = 0;

    gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

    /* Walk through all available pixel pipes. */
    for (i = 0; i < Hardware->clusterCount; ++i) {
        if (!((1 << i) & clusterMask))
            continue;

        /* Select proper pipe. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(clock, AQ_HI_CLOCK_CONTROL, MULTI_PIPE_REG_SELECT, i)));

        /* BW */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_READS_Address, &read));
        totalRead += read;

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_WRITES_Address, &write));
        totalWrite += write;
    }

    gcmkPRINT("==============GPU Profile: read request : %d\n", totalRead);
    gcmkPRINT("==============GPU Profile: write request: %d\n", totalWrite);

    /* Reset clock control register. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, clock));
    /* Reset counters. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_RESET_MEM_COUNTERS_Address, 1));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_RESET_MEM_COUNTERS_Address, 0));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

#if gcdDVFS
# define READ_FROM_EATER1 0

gceSTATUS
gckHARDWARE_QueryLoad(gckHARDWARE Hardware, gctUINT32 *Load)
{
    gctUINT32 debug1;
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Load != gcvNULL);

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    if (Hardware->chipPowerState == gcvPOWER_ON) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_DEBUG0_Address, Load));
#if READ_FROM_EATER1
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER1_DEBUG0_Address, Load));
# endif

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_DEBUG1_Address, &debug1));

        /* Patch result of 0x110 with result of 0x114. */
        if ((debug1 & 0xFF) == 1) {
            *Load &= ~0xFF;
            *Load |= 1;
        }

        if (((debug1 & 0xFF00) >> 8) == 1) {
            *Load &= ~(0xFF << 8);
            *Load |= 1 << 8;
        }

        if (((debug1 & 0xFF0000) >> 16) == 1) {
            *Load &= ~(0xFF << 16);
            *Load |= 1 << 16;
        }

        if (((debug1 & 0xFF000000) >> 24) == 1) {
            *Load &= ~(0xFF << 24);
            *Load |= 1 << 24;
        }
    } else {
        status = gcvSTATUS_INVALID_REQUEST;
    }

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_SetDVFSPeroid(gckHARDWARE Hardware, gctUINT32 Frequency)
{
    gceSTATUS status;
    gctUINT32 period;
    gctUINT32 eater;
    gctBOOL acquired = gcvFALSE;

#if READ_FROM_EATER1
    gctUINT32 period1;
    gctUINT32 eater1;
# endif

    gcmkHEADER_ARG("Hardware=%p Frequency=%d", Hardware, Frequency);

    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    period = 0;

    while ((64 << period) < (gcdDVFS_ANAYLSE_WINDOW * Frequency * 1000))
        period++;

#if READ_FROM_EATER1
    /*
     *  Period = F * 1000 * 1000 / (60 * 16 * 1024);
     */
    period1 = Frequency * 6250 / 6114;
# endif

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    if (Hardware->chipPowerState == gcvPOWER_ON) {
        /* Get current configure. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, &eater));

        /* Change period. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_PULSE_EATER_Address,
                                          gcmSETFIELD(eater, GC_PULSE_EATER, PERIOD, period)));

#if READ_FROM_EATER1
        /* Config eater1. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER1_Address, &eater1));

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_PULSE_EATER1_Address,
                                          gcmSETFIELD(eater1, GC_PULSE_EATER1, PERIOD_PER_FRAME, period1)));
# endif
    } else {
        status = gcvSTATUS_INVALID_REQUEST;
    }

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_InitDVFS(gckHARDWARE Hardware)
{
    gceSTATUS status;
    gctUINT32 data;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, &data));

    data = gcmSETFIELD(data, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH, 1);
    data = gcmSETFIELD(data, GC_PULSE_EATER, EXTERNAL_CONTROL,     1);
    data = gcmSETFIELD(data, GC_PULSE_EATER, MASK_PE_STALL,        0);
    data = gcmSETFIELD(data, GC_PULSE_EATER, UPPER_BOUND_ENABLE,   1);
    data = gcmSETFIELD(data, GC_PULSE_EATER, ONLY_IDLE,            1);
    data = gcmSETFIELD(data, GC_PULSE_EATER, ENABLE_FAST_START,    0);

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "DVFS Configure=0x%X", data);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, data));

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}
#endif

gceSTATUS
gckHARDWARE_ExecuteFunctions(gcsFUNCTION_EXECUTION_PTR Execution)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 idle;
    gctUINT32 i, timer = 0, delay = 10;
    gctADDRESS address;
    gckHARDWARE hardware = (gckHARDWARE)Execution->hardware;

#if gcdCAPTURE_ONLY_MODE
    gcmkONERROR(status);
#endif

#if gcdDUMP_IN_KERNEL
    gcmkDUMP(hardware->os, "#[function: %s]", Execution->funcName);
#endif

    for (i = 0; i < Execution->funcCmdCount; i++) {
        address = Execution->funcCmd[i].address;

#if gcdDUMP_IN_KERNEL
        gcmkDUMP_BUFFER(hardware->os,
                        gcvDUMP_BUFFER_KERNEL_COMMAND,
                        Execution->funcCmd[i].logical,
                        Execution->funcCmd[i].address,
                        Execution->funcCmd[i].bytes);
#endif

        /* Execute prepared command sequence. */
        if (hardware->mcFE) {
            gcmkONERROR(gckMCFE_Execute(hardware, gcvFALSE,
                                        Execution->funcCmd[i].channelId,
                                        address,
                                        Execution->funcCmd[i].bytes));
        } else {
            gcmkONERROR(gckWLFE_Execute(hardware, address,
                                        Execution->funcCmd[i].bytes));
        }

#if gcdLINK_QUEUE_SIZE
        {
            gcuQUEUEDATA data;

            gcmkVERIFY_OK(gckOS_GetProcessID(&data.linkData.pid));

            data.linkData.start = address;
            data.linkData.end = address + Execution->funcCmd[i].bytes;
            data.linkData.linkLow = 0;
            data.linkData.linkHigh = 0;

            gckQUEUE_Enqueue(&hardware->linkQueue, &data);
        }
#endif

        /* Wait until GPU idle. */
        do {
            gckOS_Udelay(hardware->os, delay);

            gcmkONERROR(gckOS_ReadRegisterEx(hardware->os, hardware->kernel, AQ_HI_IDLE_Address, &idle));

            timer += delay;
            delay *= 2;

#if gcdGPU_TIMEOUT
            if (timer >= hardware->kernel->timeOut) {
                gckHARDWARE_DumpGPUState(hardware);

                if (hardware->kernel->command)
                    gckCOMMAND_DumpExecutingBuffer(hardware->kernel->command);

                /* Even if hardware is not reset correctly, let software
                 * continue to avoid software stuck. Software will timeout again
                 * and try to recover GPU in next timeout.
                 */
                gcmkONERROR(gcvSTATUS_DEVICE);
            }
#endif
        } while (!_IsHWIdle(idle, hardware));
        gcmkDUMP(hardware->os, "@[register.wait 0x%05X 0x%08X 0x%08X]",
                 AQ_HI_IDLE_Address, gcmSETFIELD(0, AQ_HI_IDLE, IDLE_FE, ~0U), idle);
    }
    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
gckHARDWARE_QueryStateTimer(gckHARDWARE Hardware, gctUINT64_PTR On,
                            gctUINT64_PTR Off, gctUINT64_PTR Idle, gctUINT64_PTR Suspend)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->powerMutex, gcvINFINITE));
    acquired = gcvTRUE;

    gckSTATETIMER_Query(&Hardware->powerStateCounter, Hardware->chipPowerState, On, Off, Idle, Suspend);

    status = gcvSTATUS_OK;

OnError:
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->powerMutex));

    return status;
}

gceSTATUS
gckHARDWARE_WaitFence(gckHARDWARE Hardware, gctPOINTER Logical,
                      gctUINT64 FenceData, gctADDRESS FenceAddress, gctUINT32 *Bytes)
{
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;

    gctUINT32 dataLow  = (gctUINT32)FenceData;
    gctUINT32 dataHigh = (gctUINT32)(FenceData >> 32);

    if (logical) {
        *logical++ =
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFEFenceDataHighRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *logical++ = dataHigh;

        *logical++ =
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregFEFenceDataRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *logical++ = dataLow;

        if (Hardware->graphicsLargeVA) {
            gctUINT64_PTR opLogical = (gctUINT64_PTR)logical;

            *opLogical++ =
                gcmSETFIELDVALUE(0, GCCMD_FENCE_COMMAND, OPCODE, FENCE64) |
                     gcmSETFIELD(0, GCCMD_FENCE_COMMAND, DELAY,  Hardware->waitCount) |
                gcmSETFIELDVALUE(0, GCCMD_FENCE_COMMAND, MODE,   GREATER_EQUAL);

            *opLogical++ = FenceAddress;
        } else {
            gctUINT32 fenceAddress;
            gcmkSAFECASTVA(fenceAddress, FenceAddress);

            *logical++ =
                gcmSETFIELDVALUE(0, GCCMD_FENCE_COMMAND, OPCODE, FENCE) |
                     gcmSETFIELD(0, GCCMD_FENCE_COMMAND, DELAY,  Hardware->waitCount) |
                gcmSETFIELDVALUE(0, GCCMD_FENCE_COMMAND, MODE,   GREATER_EQUAL);

            *logical++ = fenceAddress;
        }
    } else {
        *Bytes = (Hardware->graphicsLargeVA) ? 32 : 24;
    }

    return gcvSTATUS_OK;
}

gceSTATUS
gckHARDWARE_UpdateContextID(gckHARDWARE Hardware)
{
    static gcsiDEBUG_REGISTERS fe = { "FE", 0x470, 0, 0x450, 256, 0x1, 0x00, gcvTRUE, gcvFALSE };
    gckOS os = Hardware->os;
    gctUINT32 contextIDLow, contextIDHigh;
    gceSTATUS status;

    gcmkONERROR(gckOS_WriteRegisterEx(os, Hardware->kernel, fe.index, 0x53 << fe.shift));
    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, fe.data, &contextIDLow));

    gcmkONERROR(gckOS_WriteRegisterEx(os, Hardware->kernel, fe.index, 0x54 << fe.shift));
    gcmkONERROR(gckOS_ReadRegisterEx(os, Hardware->kernel, fe.data, &contextIDHigh));

    Hardware->contextID = ((gctUINT64)contextIDHigh << 32) + contextIDLow;

    return gcvSTATUS_OK;

OnError:
    return status;
}

gceSTATUS
gckHARDWARE_DummyDraw(gckHARDWARE Hardware, gctPOINTER Logical, gctADDRESS Address,
                      gceDUMMY_DRAW_TYPE DummyDrawType, gctUINT32 *Bytes)
{
    gctUINT32 dummyDraw_v60[] = {
        /* Semaphore from FE to PE. */
        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs),

        gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
            gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE),

        /* Stall from FE to PE. */
        gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL),

        gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
            gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregSHCacheControlRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0, GCREG_SH_CACHE_CONTROL, MODE, GCREG_SH_CACHE_CONTROL_MODE_STATES),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMultiSampleConfigRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0, GCREG_MULTI_SAMPLE_CONFIG, MODE, GCREG_MULTI_SAMPLE_CONFIG_MODE_OFF) |
            gcmSETFIELDVALUE(0, GCREG_MULTI_SAMPLE_CONFIG, DEPTH, DISABLED) |
            gcmSETFIELDVALUE(0, GCREG_MULTI_SAMPLE_CONFIG, VAA, OFF) |
            gcmSETFIELD(0, GCREG_MULTI_SAMPLE_CONFIG, ENABLE, 0),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQPixelShaderColorOutRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   6),
        0x0,
        0x2,
        0x0,
        0x0,
        0x0,
        0x0,
        (gctUINT32)~0x0,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQVertexShaderLoadBalancingRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        0xffffffff,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregVaryingsRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        2,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregVaryingPackingRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        2,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregPSUnpackRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0, GCREG_PS_UNPACK, VARYING0, 2),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregPSRelativeEndRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        1,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQPixelShaderTemporaryRegisterControlRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        3,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregPSInstructionRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        0,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregPSShaderCodeRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1 << 2),
        0x07801033,
        0x3fc00900,
        0x00000040,
        0x00390008,
        (gctUINT32)~0,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregPSStartPCRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        0x0,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTWConfigRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELDVALUE(0, GCREG_TW_CONFIG, WORK_DIM, ONE) |
            gcmSETFIELDVALUE(0, GCREG_TW_CONFIG, TRAVERSE_ORDER, XYZ) |
            gcmSETFIELDVALUE(0, GCREG_TW_CONFIG, SWATH_ENABLE_X, DISABLE) |
            gcmSETFIELDVALUE(0, GCREG_TW_CONFIG, VALUE_ORDER, WGL),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTWInfoXRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0, GCREG_TW_INFO_X, GLOBAL_SIZE, 31) |
            gcmSETFIELD(0, GCREG_TW_INFO_X, GLOBAL_OFFSET, 0),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTWWorkGroupXRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0, GCREG_TW_WORK_GROUP_X, SIZE, 31) |
            gcmSETFIELD(0, GCREG_TW_WORK_GROUP_X, COUNT, 0),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTWShaderInfoRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),

        (32 + (4 * (((gcsFEATURE_DATABASE *)Hardware->featureDatabase)->NumShaderCores) - 1)) /
            (4 * (((gcsFEATURE_DATABASE *)Hardware->featureDatabase)->NumShaderCores)),

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregTWTriggerRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        1,

        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQFlushRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELDVALUE(0, AQ_FLUSH, SHL1_CACHE, ENABLE),

        /* Semaphore from FE to PE. */
        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs),

        gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END) |
            gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE),

        /* Stall from FE to PE. */
        gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL),

        gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END) |
            gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE),

        /* Invalidate I cache.*/
        gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregSHIcacheInvalidateRegAddrs) |
             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1),
        gcmSETFIELD(0,     GCREG_SH_ICACHE_INVALIDATE, VS,  1) |
            gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, TCS, 1) |
            gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, TES, 1) |
            gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, GS,  1) |
            gcmSETFIELD(0, GCREG_SH_ICACHE_INVALIDATE, PS,  1),

#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
        /* SubmitJob. */
        gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP),
        gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP),
        gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP),
        gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, NOP),
#endif
    };

    gctUINT32 bytes = 0;
    gctUINT32_PTR dummyDraw = gcvNULL;
    gctUINT32 address;

    gcmkSAFECASTVA(address, Address);

    switch (DummyDrawType) {
    case gcvDUMMY_DRAW_V60:
        dummyDraw = dummyDraw_v60;
        bytes = gcmSIZEOF(dummyDraw_v60);
#ifdef GCREG_MCFE_STD_DESC_RING_BUF_START_ADDR_Address
        if (_QueryFeatureDatabase(Hardware, gcvFEATURE_MCFE)) {
            gctUINT32 submitJob;

            submitJob = gcmSETFIELDVALUE(0, MCFE_COMMAND, OPCODE, SUB_COMMAND) |
                        gcmSETFIELDVALUE(0, MCFE_COMMAND, SUB_OPCODE, SUBMIT_JOB);

            if (bytes & 8) {
                /* To keep 16 byte alignment. */
                bytes -= 8;
            }

            dummyDraw[(bytes >> 2) - 2] = submitJob;
        }
#endif
        break;
    default:
        /* other chip no need dummy draw.*/
        gcmkASSERT(0);
        break;
    };

    if (Logical != gcvNULL)
        gckOS_MemCopy(Logical, dummyDraw, bytes);

    *Bytes = bytes;

    return gcvSTATUS_OK;
}

gceSTATUS
gckHARDWARE_EnterQueryClock(gckHARDWARE Hardware, gctUINT64 *McStart, gctUINT64 *ShStart)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64 mcStart, shStart;

    gcmkONERROR(gckOS_GetTime(&mcStart));
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_DBG_CYCLE_COUNTER_Address, 0));

    *McStart = mcStart;

    if (Hardware->core <= gcvCORE_3D_MAX) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_DEBUG_CONTROL0_Address, 0xFFU << 24));

        gcmkONERROR(gckOS_GetTime(&shStart));

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_DEBUG_CONTROL0_Address, 0x4U << 24));

        *ShStart = shStart;
    }

OnError:
    return status;
}

gceSTATUS
gckHARDWARE_ExitQueryClock(gckHARDWARE Hardware, gctUINT64 McStart,
                           gctUINT64 ShStart, gctUINT64 *McClk, gctUINT64 *ShClk)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT64 mcEnd, shEnd;
    gctUINT32 mcCycle, shCycle;
    gctUINT64 mcFreq, shFreq = 0;

    gcmkONERROR(gckOS_GetTime(&mcEnd));
    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GC_DBG_CYCLE_COUNTER_Address, &mcCycle));

    if (mcCycle == 0)
        gcmkONERROR(gcvSTATUS_GENERIC_IO);

    /* cycle = (gctUINT64)cycle * 1000000 / (end - start); */
    mcFreq = ((gctUINT64)(mcCycle) * ((1000000U << 12) / (gctUINT32)(mcEnd - McStart))) >> 12;

    *McClk = mcFreq;

    if (Hardware->core <= gcvCORE_3D_MAX) {
        gcmkONERROR(gckOS_GetTime(&shEnd));
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GC_DEBUG_SIGNALS_SH_Address, &shCycle));

        if (!shCycle || Hardware->identity.chipModel >= gcv8000) {
            *ShClk = *McClk;
            return gcvSTATUS_OK;
        }

        if (!ShStart)
            gcmkONERROR(gcvSTATUS_GENERIC_IO);

        shFreq = ((gctUINT64)(shCycle) * ((1000000U << 12) / (gctUINT32)(shEnd - ShStart))) >> 12;
    }

    *ShClk = shFreq;

OnError:
    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryFrequency
 **
 **  Query current hardware frequency.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 */
gceSTATUS
gckHARDWARE_QueryFrequency(gckHARDWARE Hardware)
{
    gctUINT64 mcStart, shStart;
    gctUINT64 mcClk, shClk;
    gceSTATUS status;
    gctUINT64 powerManagement = 0;
    gctBOOL globalAcquired = gcvFALSE;
    gceCHIPPOWERSTATE statesStored, state;

    gcmkHEADER_ARG("Hardware=0x%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    mcStart = 0;
    shStart = 0;
    mcClk = 0;
    shClk = 0;

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    /* Grab the global semaphore. */
    gcmkONERROR(gckOS_AcquireSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvTRUE;

    gckHARDWARE_EnterQueryClock(Hardware, &mcStart, &shStart);

    gcmkONERROR(gckOS_Delay(Hardware->os, 50));

    if (mcStart) {
        gckHARDWARE_ExitQueryClock(Hardware, mcStart, shStart, &mcClk, &shClk);

        Hardware->mcClk = mcClk;
        Hardware->shClk = shClk;
    }

    /* Release the global semaphore. */
    gcmkONERROR(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvFALSE;

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

    gcmkFOOTER_NO();

    return gcvSTATUS_OK;

OnError:
    if (globalAcquired) {
        /* Release the global semaphore. */
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));
    }

    gcmkFOOTER();

    return status;
}

/*******************************************************************************
 **
 **  gckHARDWARE_QueryFScale
 **
 **  Query current hardware frequency scale.
 **
 **  INPUT:
 **
 **      gckHARDWARE Hardware
 **          Pointer to an gckHARDWARE object.
 **
 */
gceSTATUS
gckHARDWARE_QueryFScale(gckHARDWARE Hardware, gctUINT8 *Scale)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 regValue = 0;

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, &regValue));

    *Scale = (regValue & 0x1FC) >> 2;

    gcmkFOOTER_NO();
    return status;

OnError:
    gcmkFOOTER_NO();
    return status;
}

/*******************************************************************************
 **
 ** Set MC and SH clock
 **
 ** mcScale: MC clock scale
 ** shScale: SH clock scale
 */
gceSTATUS
gckHARDWARE_SetClock(gckHARDWARE Hardware, gctUINT32 MCScale, gctUINT32 SHScale)
{
    gceSTATUS status;
    gctUINT64 powerManagement = 0;
    gctBOOL globalAcquired = gcvFALSE;
    gctUINT32 org;
    gctUINT32 mcScale = MCScale;
    gctUINT32 shScale = SHScale;
    gceCHIPPOWERSTATE statesStored, state;

    gcmkHEADER();

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    /* Grab the global semaphore. */
    gcmkONERROR(gckOS_AcquireSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvTRUE;

    if (mcScale > 0 && mcScale <= 64) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, AQ_HI_CLOCK_CONTROL_Address, &org));

        org = gcmSETFIELD(org, AQ_HI_CLOCK_CONTROL, FSCALE_VAL, mcScale);

        /* Write the clock control register. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(org, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 1)));

        /* Done loading the frequency scaler. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_HI_CLOCK_CONTROL_Address,
                                          gcmSETFIELD(org, AQ_HI_CLOCK_CONTROL, FSCALE_CMD_LOAD, 0)));

        /* Need to change GC_PULSE_EATER_Address when it is introduced. */
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, &org));

        /* Never impact shader clk. */
        org = 0x01020800 | (org & 0xFF);

        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, org));

        Hardware->powerOnFscaleVal = mcScale;
    }

    /* set SH clock */
    if (shScale > 0 && shScale <= 64) {
        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel, GC_PULSE_EATER_Address, &org));

        org = gcmSETFIELD(org, GC_PULSE_EATER, FSCALE_VAL_SH, shScale);
        org = gcmSETFIELD(org, GC_PULSE_EATER, ENABLE_AUTO_PULSE_SH, 0);
        org = gcmSETFIELD(org, GC_PULSE_EATER, DISABLE_AUTO_PULSE, 1);

        /* Write the clock control register. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_PULSE_EATER_Address,
                                          gcmSETFIELD(org, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH, 1)));

        /* Done loading the frequency scaler. */
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GC_PULSE_EATER_Address,
                                          gcmSETFIELD(org, GC_PULSE_EATER, FSCALE_CMD_LOAD_SH, 0)));

        Hardware->powerOnShaderFscaleVal = shScale;
    }

    /* Release the global semaphore. */
    gcmkONERROR(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvFALSE;

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

    gcmkFOOTER_NO();

    return gcvSTATUS_OK;

OnError:
    if (globalAcquired) {
        /* Release the global semaphore. */
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));
    }

    gcmkFOOTER_NO();

    return status;
}

gceSTATUS
gckHARDWARE_QueryCycleCount(gckHARDWARE Hardware,
                            gctUINT32 *hi_total_cycle_count,
                            gctUINT32 *hi_total_idle_cycle_count)
{
    gceSTATUS status = gcvSTATUS_OK;
    gceCHIPMODEL chipModel;

    gcmkHEADER_ARG("Hardware=0x%p hi_total_cycle_count=0x%p hi_total_idle_cycle_count=0x%p",
                   Hardware, hi_total_cycle_count, hi_total_idle_cycle_count);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    chipModel = Hardware->identity.chipModel;

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GC_TOTAL_IDLE_CYCLES_Address,
                                     hi_total_idle_cycle_count));

    gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                     GC_TOTAL_CYCLES_Address,
                                     hi_total_cycle_count));

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_CleanCycleCount(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Hardware=0x%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_IDLE_CYCLES_Address, 0));

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_DBG_CYCLE_COUNTER_Address, 0));

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel, GC_TOTAL_CYCLES_Address, 0));

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_QueryCoreLoad(gckHARDWARE Hardware, gctUINT32 Mdelay, gctUINT32 *Load)
{
    gceSTATUS status = gcvSTATUS_OK;
    gceCHIPPOWERSTATE statesStored, state;
    gctBOOL powerManagement = gcvFALSE;
    static gctBOOL profilerEnable = gcvFALSE;
    gctUINT32 hi_total_cycle_count = 0, hi_total_idle_cycle_count = 0;

    gcmkHEADER_ARG("Hardware=0x%p Load=0x%p", Hardware, Load);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_QueryPowerState(Hardware, &statesStored));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    if (!profilerEnable) {
        gcmkONERROR(gckHARDWARE_SetGpuProfiler(Hardware, gcvTRUE));

        gcmkONERROR(gckHARDWARE_InitProfiler(Hardware));

        profilerEnable = gcvTRUE;
    }

    Hardware->waitCount = 200 * 100;

    gcmkONERROR(gckHARDWARE_CleanCycleCount(Hardware));

    gcmkONERROR(gckOS_Delay(Hardware->os, Mdelay));

    gcmkONERROR(gckHARDWARE_QueryCycleCount(Hardware, &hi_total_cycle_count, &hi_total_idle_cycle_count));

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

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvTRUE));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, state));

    *Load = (hi_total_cycle_count - hi_total_idle_cycle_count) * 100 / hi_total_cycle_count;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_CancelJob(gckHARDWARE Hardware)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL powerManagement = gcvFALSE;
    gctBOOL globalAcquired = gcvFALSE;
    gctBOOL mutexAcquired = gcvFALSE;
    gctUINT32 data;
    gckEVENT eventObj;
    gctUINT32 mask = 0;
    gctUINT32 i = 0, count = 0;
#if gcdINTERRUPT_STATISTIC
    gctINT32 oldValue;
#endif

    gcmkHEADER_ARG("Hardware=0x%p", Hardware);

    gcmkVERIFY_ARGUMENT(Hardware != gcvNULL);

    eventObj = Hardware->kernel->eventObj;

    if (!gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BIT_NN_JOB_CANCELATION) ||
        !gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_SECURITY_AHB) ||
        Hardware->options.secureMode != gcvSECURE_IN_NORMAL) {
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);
    }

    status = gckOS_AcquireMutex(Hardware->os,
                       Hardware->kernel->device->commitMutex,
                       gcdRECOVERY_FORCE_TIMEOUT);
    if (status == gcvSTATUS_OK)
        mutexAcquired = gcvTRUE;

    powerManagement = Hardware->options.powerManagement;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvFALSE));

    gcmkONERROR(gckHARDWARE_SetPowerState(Hardware, gcvPOWER_ON_AUTO));

    /* Grab the global semaphore. */
    gcmkONERROR(gckOS_AcquireSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvTRUE;

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GCREG_HI_AHB_CONTROL_Address,
                                      gcmSETFIELDVALUE(0, GCREG_HI_AHB_CONTROL, VIP_JOB_CANCEL, ENABLE)));

    do {
        gckOS_Udelay(Hardware->os, 10);

        gcmkONERROR(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                         GCREG_HI_AHB_CONTROL_Address, &data));
    } while (!(data & (0x1 << 4)));

    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      GCREG_HI_AHB_CONTROL_Address,
                                      gcmSETFIELDVALUE(0, GCREG_HI_AHB_CONTROL, VIP_JOB_CANCEL, DISABLE)));

    /* Force the command queue to reload the next context. */
    Hardware->kernel->command->currContext = gcvNULL;

    Hardware->kernel->command->running = gcvFALSE;

    gcmkONERROR(gckCOMMAND_Start(Hardware->kernel->command));

    gcmkVERIFY_OK(gckOS_AtomGet(Hardware->os, Hardware->pendingEvent, (gctINT32 *)&mask));

    if (mask) {
        /* Handle all outstanding events now. */
        gcmkONERROR(gckOS_AtomSetMask(eventObj->pending, mask));
    }

    for (i = 0; i < 32; i++) {
        if (mask & (1 << i))
            count++;
    }

#if gcdINTERRUPT_STATISTIC
    while (count--)
        gcmkONERROR(gckOS_AtomDecrement(Hardware->os, eventObj->interruptCount, &oldValue));

    gckOS_AtomClearMask(Hardware->pendingEvent, mask);
#endif

    gcmkONERROR(gckEVENT_Notify(eventObj, 2, gcvNULL));

    /* Release the global semaphore. */
    gcmkONERROR(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));

    globalAcquired = gcvFALSE;

    if (powerManagement)
        gcmkONERROR(gckHARDWARE_EnablePowerManagement(Hardware, gcvTRUE));

OnError:
    if (globalAcquired)
        gcmkVERIFY_OK(gckOS_ReleaseSemaphore(Hardware->os, Hardware->globalSemaphore));

    if (mutexAcquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Hardware->os, Hardware->kernel->device->commitMutex));

    gcmkFOOTER();
    return status;
}

#if gcdENABLE_AHBXTTD
#define SYSTEM_REGISTER_OFFSET 0x60000

gceSTATUS
gckHARDWARE_ResetAHBXTTD(gckHARDWARE Hardware, gckKERNEL Kernel)
{
    gctUINT32 value = 0;
    gceSTATUS status = gcvSTATUS_OK;
    gckOS Os = Kernel->os;

    gcmkHEADER_ARG("Hardware=%p Kernel=%p", Hardware, Kernel);
    gcmkVERIFY_ARGUMENT(Kernel != gcvNULL);

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, &value));

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET, 1);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, value));

#if gcdFPGA_BUILD
    /* Wait more time on FPGA for reset as lower frequency */
    gcmkONERROR(gckOS_Delay(Os, 10));
#else
    /* Wait for reset. */
    gcmkONERROR(gckOS_Udelay(Os, 100));
#endif

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET, 1);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, value));

#if gcdFPGA_BUILD
    /* Wait more time on FPGA for reset as lower frequency */
    gcmkONERROR(gckOS_Delay(Os, 10));
#else
    /* Wait for reset. */
    gcmkONERROR(gckOS_Udelay(Os, 100));
#endif

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, &value));

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP0, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP0, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP1, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP1, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP2, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP2, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP3, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP3, 1);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, value));

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_APPENDIX_Address + SYSTEM_REGISTER_OFFSET, &value));

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_VIP4, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKVIP4, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_VIP5, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKVIP5, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKXIODMA, 1)
              | gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLK_SEMAPHORE, 1);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_APPENDIX_Address + SYSTEM_REGISTER_OFFSET, value));

#if gcdFPGA_BUILD
    /* Wait more time on FPGA for reset as lower frequency */
    gcmkONERROR(gckOS_Delay(Os, 10));
#else
    /* Wait for reset. */
    gcmkONERROR(gckOS_Udelay(Os, 100));
#endif

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, &value));

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP0, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP0, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP1, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP1, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP2, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP2, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_VIP3, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC, SOFT_RESET_ACLKVIP3, 0);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_Address + SYSTEM_REGISTER_OFFSET, value));

    gcmkONERROR(gckOS_ReadRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_APPENDIX_Address + SYSTEM_REGISTER_OFFSET, &value));

    value = gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_VIP4, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKVIP4, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_VIP5, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKVIP5, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLKXIODMA, 0)
              & gcmSETFIELD(value, GCREG_AHBXTTD_MISC_APPENDIX, SOFT_RESET_ACLK_SEMAPHORE, 0);

    gcmkONERROR(gckOS_WriteRegisterEx(Os, Kernel, GCREG_AHBXTTD_MISC_APPENDIX_Address + SYSTEM_REGISTER_OFFSET, value));

#if gcdFPGA_BUILD
    /* Wait more time on FPGA for reset as lower frequency */
    gcmkONERROR(gckOS_Delay(Os, 10));
#else
    /* Wait for reset. */
    gcmkONERROR(gckOS_Udelay(Os, 100));
#endif

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    gcmkFOOTER();
    return status;
}
#endif

gceSTATUS
gckHARDWARE_FillMMUDescriptor(gckHARDWARE Hardware,
                              gctUINT32 Index, gctPHYS_ADDR_T MtlbPhysical)
{
    gceSTATUS status = gcvSTATUS_OK;
    gcsMMU_TABLE_ARRAY_ENTRY *entry;
    gctUINT32 config, extMtlb;
    gceMMU_MODE mode;

    gcmkHEADER_ARG("Hardware=%p Index=%x MtlbPhysical=%llx",
                    Hardware, Index, MtlbPhysical);

    config  = (gctUINT32)(MtlbPhysical & 0xFFFFFFFF);
    extMtlb = (gctUINT32)(MtlbPhysical >> 32);

    /* more than 40bit physical address */
    if (extMtlb & 0xFFFFFF00)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

#if gcdENABLE_MMU_1KMODE
    mode = gcvMMU_MODE_1K;
#else
    mode = gcvMMU_MODE_4K;
#endif

    if (Hardware->largeVAVersion == gcv40BIT_VA_40BIT_PA_0)
        mode = gcvMMU_MODE_4K;

    if (mode == gcvMMU_MODE_1K) {
        if (config & 0x3FF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE1_K);
    } else {
        if (config & 0xFFF)
            gcmkONERROR(gcvSTATUS_NOT_ALIGNED);

        config |= gcmSETFIELDVALUE(0, GCREG_MMU_CONFIGURATION, MODE, MODE4_K);
    }

    entry = (gcsMMU_TABLE_ARRAY_ENTRY *)Hardware->pagetableArray.logical;
    entry += Index;

    /* Setup page table array entry. */
    if (Hardware->bigEndian) {
        entry->low = gcmBSWAP32(config);
        entry->high = gcmBSWAP32(config);
    } else {
        entry->low = config;
        entry->high = extMtlb;
    }

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_SwitchPageTableId(gckHARDWARE Hardware, gckMMU Mmu,
                              gckCOMMAND Command)
{
    gctUINT32_PTR buffer;
    gctUINT32 mmuBytes = 24;
    gctUINT32 bufferSize;
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL multiCluster = gcvFALSE;
    gctBOOL bltEngine = gcvFALSE;
    gctUINT32 semaphore, stall;
#if gcdMMU_VERSION_2
    gctUINT32 flushVMID[gcdMMU_VMID_NUM] = { 0 };
    gctUINT32 flushVMIDCount = 0;
    gctUINT32 i;
#endif

    gcmkHEADER();

    if (Hardware->options.secureMode != gcvSECURE_IN_NORMAL)
        gcmkONERROR(gcvSTATUS_NOT_SUPPORTED);

    bltEngine = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE);
    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    if (bltEngine && multiCluster)
        mmuBytes += 32;
    else if (bltEngine)
        mmuBytes += 16;
    else if (multiCluster)
        mmuBytes += 8;

#if gcdMMU_VERSION_2
    /* Clean the unused map cache before switch to new page table. */
    gcmkONERROR(gckOS_AcquireMutex(Hardware->os, Hardware->kernel->mmuDescMutex,
                                   gcvINFINITE));

    for (i = 0; i < gcdMMU_VMID_NUM; i++) {
        if (!Hardware->kernel->vmidWaitingFlush[i])
            continue;

        flushVMID[flushVMIDCount++] = i;
        mmuBytes += 8;
        Hardware->kernel->vmidWaitingFlush[i] = 0;
    }

    gcmkONERROR(gckOS_ReleaseMutex(Hardware->os, Hardware->kernel->mmuDescMutex));
#endif

    gcmkONERROR(gckCOMMAND_Reserve(Command, mmuBytes,
                                   (gctPOINTER *)&buffer, &bufferSize));

    if (buffer) {
        gctUINT32 clusterMask;

        if (multiCluster) {
            gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

            *buffer++ =
                gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                     gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregClusterControlRegAddrs);

            *buffer++ = gcmSETFIELD(0, GCREG_CLUSTER_CONTROL, CLUSTER_ALIVE_ENABLE,
                                    clusterMask);
        }

#if gcdMMU_VERSION_2
        /* CLean useless map cache. */
        if (flushVMIDCount) {
            for (i = 0; i < flushVMIDCount; i++) {
                /* Flush MMU VMID. */
                *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) |
                    gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUPTFlushRegAddrs) |
                    gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, 1);

                *buffer++ = gcmSETFIELD(0, GCREG_MMUPT_FLUSH, FLUSH_CMD, 0) |
                            gcmSETFIELD(0, GCREG_MMUPT_FLUSH, FLUSH_VMID, flushVMID[i]);
            }
        }
#endif

        /* Setup command buffer to load index of page table array. */
        *buffer++ =
            gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregMMUConfigRegAddrs) |
                 gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *buffer++ = gcmSETMASKEDFIELD(GCREG_MMU_CONFIG, PAGE_TABLE_ID, Mmu->descIndex);

        if (bltEngine) {
            /* Blt lock. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *buffer++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

            if (multiCluster) {
                *buffer++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltClusterControlRegAddrs);

                *buffer++ = gcmSETFIELD(0, GCREG_BLT_CLUSTER_CONTROL, CLUSTER_ENABLE,
                                        clusterMask);
            }
        }

        /* Arm the PE-FE Semaphore. */
        *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQSemaphoreRegAddrs);

        semaphore = gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END);

        if (Hardware->stallFEPrefetch)
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, FRONT_END, PREFETCH);

        if (bltEngine)
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, BLT_ENGINE);
        else
            semaphore |= gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE);

        *buffer++ = semaphore;

        /* STALL FE until PE is done flushing. */
        *buffer++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);

        stall = gcmSETFIELDVALUE(0, STALL_STALL, SOURCE, FRONT_END);

        if (Hardware->stallFEPrefetch)
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, FRONT_END, PREFETCH);

        if (bltEngine)
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, BLT_ENGINE);
        else
            stall |= gcmSETFIELDVALUE(0, STALL_STALL, DESTINATION, PIXEL_ENGINE);

        *buffer++ = stall;

        if (bltEngine) {
            /* Blt unlock. */
            *buffer++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                             gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *buffer++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
        }
    } else
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    if (Command->feType == gcvHW_FE_WAIT_LINK)
        gcmkONERROR(gckCOMMAND_Execute(Command, mmuBytes));
    else
        gcmkONERROR(gckCOMMAND_ExecuteEnd(Command, mmuBytes));

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_QueryClusterInfo(gckHARDWARE Hardware, gctUINT32 *ClusterMask)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 value;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckOS_AtomGet(Hardware->os, Hardware->curClusterMask, (gctINT32_PTR)&value));

    if (ClusterMask)
        *ClusterMask = value;

OnError:
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckHARDWARE_ConfigCluster(gckHARDWARE Hardware, gctUINT32 ClusterMask)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Hardware=%p", Hardware);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkONERROR(gckCOMMAND_Stall(Hardware->kernel->command, gcvFALSE));
    gcmkONERROR(gckCOMMAND_Stop(Hardware->kernel->command));

    /* Trigger power module to config cluster power. */
    gcmkONERROR(gckOS_SetGPUPowerEx(Hardware->os, Hardware->kernel, gcvTRUE, gcvTRUE, ClusterMask));

    /* [TODO]: Read the cluster power setting status. */

    gcmkONERROR(gckOS_AtomSet(Hardware->os, Hardware->curClusterMask, ClusterMask));

    gcmkONERROR(gckHARDWARE_Reset(Hardware, gcvTRUE));

    Hardware->options.userClusterMasks[Hardware->core] = ClusterMask;

OnError:
    gcmkFOOTER();
    return status;
}

/* TODO: The cluster config context should be updated by user cmd delta. */
gceSTATUS
gckHARDWARE_UpdateClusterConfigContext(gckHARDWARE Hardware, gctUINT32 *Buffer, gctUINT32 BufferSize)
{
    gctBOOL multiCluster = gcvFALSE;
    gctUINT32 *buffer = Buffer;
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT32 index = 0;
    gctBOOL hasD3D11;

    hasD3D11 = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_D3D11_SUPPORT);
    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    if (!hasD3D11) {
        /* try to skip possible pipeline select command and find the command about cluster control */
        while (buffer && (index * gcmSIZEOF(gctUINT32)) < BufferSize) {
            if (*buffer == 0x08040E48) {
                break;
            } else {
                buffer++;
                index++;
            }
        }
    } else {
        /* try to skip possible pipeline select command and find the command about cluster control */
        while (buffer && (index * gcmSIZEOF(gctUINT32)) < BufferSize) {
            if (*buffer == 0x08040E44) {
                break;
            } else {
                buffer++;
                index++;
            }
        }
    }

    if ((index * gcmSIZEOF(gctUINT32)) >= BufferSize)
        gcmkONERROR(gcvSTATUS_NOT_FOUND);

    if (multiCluster) {
        gctUINT32 clusterMask, i;

        gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

        /* try to change from next command */
        buffer++;

        if (!hasD3D11) {
            for (i = 0; i < GCREG_POWER_CLUSTER_CONTROL_Count; i++) {
                *buffer++ = gcmSETFIELD(0, GCREG_POWER_CLUSTER_CONTROL, POWER_CLUSTER_ALIVE, clusterMask);
            }

            /* skip DEADDEAD and 0x08040E44*/
            buffer += 2;
        }

        for (i = 0; i < GCREG_CLUSTER_CONTROL_Count; i++) {
            *buffer++ = gcmSETFIELD(0, GCREG_CLUSTER_CONTROL, CLUSTER_ALIVE_ENABLE, clusterMask);
        }
    }

OnError:
    return status;
}

