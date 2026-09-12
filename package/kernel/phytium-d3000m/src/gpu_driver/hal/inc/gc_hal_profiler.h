/****************************************************************************
*
*    Copyright (c) 2025, Phytium Technology Co., Ltd.  All rights reserved.
*
*    The material in this file is confidential and contains trade secrets
*    of Phytium Corporation. This is proprietary information owned by
*    Phytium Corporation. No part of this work may be disclosed,
*    reproduced, copied, transmitted, or used in any way for any purpose,
*    without the express written permission of Phytium Corporation.
*
*****************************************************************************/


#ifndef __gc_hal_profiler_h_
#define __gc_hal_profiler_h_

#include "shared/gc_hal_profiler_shared.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GLVERTEX_OBJECT         10
#define GLVERTEX_OBJECT_BYTES   11

#define GLINDEX_OBJECT          20
#define GLINDEX_OBJECT_BYTES    21

#define GLTEXTURE_OBJECT        30
#define GLTEXTURE_OBJECT_BYTES  31

#define GLBUFOBJ_OBJECT         40
#define GLBUFOBJ_OBJECT_BYTES   41

#define    ES11_CALLS              151
#define    ES11_DRAWCALLS          (ES11_CALLS             + 1)
#define    ES11_STATECHANGECALLS   (ES11_DRAWCALLS         + 1)
#define    ES11_POINTCOUNT         (ES11_STATECHANGECALLS  + 1)
#define    ES11_LINECOUNT          (ES11_POINTCOUNT        + 1)
#define    ES11_TRIANGLECOUNT      (ES11_LINECOUNT         + 1)

#define    ES30_CALLS              159
#define    ES30_DRAWCALLS          (ES30_CALLS             + 1)
#define    ES30_STATECHANGECALLS   (ES30_DRAWCALLS         + 1)
#define    ES30_POINTCOUNT         (ES30_STATECHANGECALLS  + 1)
#define    ES30_LINECOUNT          (ES30_POINTCOUNT        + 1)
#define    ES30_TRIANGLECOUNT      (ES30_LINECOUNT         + 1)

#define    VG11_CALLS              88
#define    VG11_DRAWCALLS          (VG11_CALLS              + 1)
#define    VG11_STATECHANGECALLS   (VG11_DRAWCALLS          + 1)
#define    VG11_FILLCOUNT          (VG11_STATECHANGECALLS   + 1)
#define    VG11_STROKECOUNT        (VG11_FILLCOUNT          + 1)
/* End of Driver API ID Definitions. */

/* HAL & MISC IDs. */
#define HAL_VERTBUFNEWBYTEALLOC    1
#define HAL_VERTBUFTOTALBYTEALLOC  (HAL_VERTBUFNEWBYTEALLOC     + 1)
#define HAL_VERTBUFNEWOBJALLOC     (HAL_VERTBUFTOTALBYTEALLOC   + 1)
#define HAL_VERTBUFTOTALOBJALLOC   (HAL_VERTBUFNEWOBJALLOC      + 1)
#define HAL_INDBUFNEWBYTEALLOC     (HAL_VERTBUFTOTALOBJALLOC    + 1)
#define HAL_INDBUFTOTALBYTEALLOC   (HAL_INDBUFNEWBYTEALLOC      + 1)
#define HAL_INDBUFNEWOBJALLOC      (HAL_INDBUFTOTALBYTEALLOC    + 1)
#define HAL_INDBUFTOTALOBJALLOC    (HAL_INDBUFNEWOBJALLOC       + 1)
#define HAL_TEXBUFNEWBYTEALLOC     (HAL_INDBUFTOTALOBJALLOC     + 1)
#define HAL_TEXBUFTOTALBYTEALLOC   (HAL_TEXBUFNEWBYTEALLOC      + 1)
#define HAL_TEXBUFNEWOBJALLOC      (HAL_TEXBUFTOTALBYTEALLOC    + 1)
#define HAL_TEXBUFTOTALOBJALLOC    (HAL_TEXBUFNEWOBJALLOC       + 1)

#define GPU_CYCLES            1
#define GPU_READ64BYTE        (GPU_CYCLES            + 1)
#define GPU_WRITE64BYTE       (GPU_READ64BYTE        + 1)
#define GPU_TOTALCYCLES       (GPU_WRITE64BYTE       + 1)
#define GPU_IDLECYCLES        (GPU_TOTALCYCLES       + 1)

#define VS_INSTCOUNT          1
#define VS_BRANCHINSTCOUNT    (VS_INSTCOUNT          + 1)
#define VS_TEXLDINSTCOUNT     (VS_BRANCHINSTCOUNT    + 1)
#define VS_RENDEREDVERTCOUNT  (VS_TEXLDINSTCOUNT     + 1)
#define VS_SOURCE             (VS_RENDEREDVERTCOUNT  + 1)
#define VS_NONIDLESTARVECOUNT (VS_SOURCE             + 1)
#define VS_STARVELCOUNT       (VS_NONIDLESTARVECOUNT + 1)
#define VS_STALLCOUNT         (VS_STARVELCOUNT       + 1)
#define VS_PROCESSCOUNT       (VS_STALLCOUNT         + 1)

#define PS_INSTCOUNT          1
#define PS_BRANCHINSTCOUNT    (PS_INSTCOUNT          + 1)
#define PS_TEXLDINSTCOUNT     (PS_BRANCHINSTCOUNT    + 1)
#define PS_RENDEREDPIXCOUNT   (PS_TEXLDINSTCOUNT     + 1)
#define PS_SOURCE             (PS_RENDEREDPIXCOUNT   + 1)
#define PS_NONIDLESTARVECOUNT (PS_SOURCE             + 1)
#define PS_STARVELCOUNT       (PS_NONIDLESTARVECOUNT + 1)
#define PS_STALLCOUNT         (PS_STARVELCOUNT       + 1)
#define PS_PROCESSCOUNT       (PS_STALLCOUNT         + 1)
#define PS_SHADERCYCLECOUNT   (PS_PROCESSCOUNT       + 1)

#define PA_INVERTCOUNT        1
#define PA_INPRIMCOUNT        (PA_INVERTCOUNT      + 1)
#define PA_OUTPRIMCOUNT       (PA_INPRIMCOUNT      + 1)
#define PA_DEPTHCLIPCOUNT     (PA_OUTPRIMCOUNT     + 1)
#define PA_TRIVIALREJCOUNT    (PA_DEPTHCLIPCOUNT   + 1)
#define PA_CULLCOUNT          (PA_TRIVIALREJCOUNT  + 1)
#define PA_NONIDLESTARVECOUNT (PA_CULLCOUNT        + 1)
#define PA_STARVELCOUNT       (PA_NONIDLESTARVECOUNT + 1)
#define PA_STALLCOUNT         (PA_STARVELCOUNT     + 1)
#define PA_PROCESSCOUNT       (PA_STALLCOUNT       + 1)

#define SE_TRIANGLECOUNT        1
#define SE_LINECOUNT            (SE_TRIANGLECOUNT        + 1)
#define SE_STARVECOUNT          (SE_LINECOUNT            + 1)
#define SE_STALLCOUNT           (SE_STARVECOUNT          + 1)
#define SE_RECEIVETRIANGLECOUNT (SE_STALLCOUNT           + 1)
#define SE_SENDTRIANGLECOUNT    (SE_RECEIVETRIANGLECOUNT + 1)
#define SE_RECEIVELINESCOUNT    (SE_SENDTRIANGLECOUNT    + 1)
#define SE_SENDLINESCOUNT       (SE_RECEIVELINESCOUNT    + 1)
#define SE_NONIDLESTARVECOUNT   (SE_SENDLINESCOUNT       + 1)
#define SE_PROCESSCOUNT         (SE_NONIDLESTARVECOUNT   + 1)

#define RA_VALIDPIXCOUNT       1
#define RA_TOTALQUADCOUNT      (RA_VALIDPIXCOUNT      + 1)
#define RA_VALIDQUADCOUNTEZ    (RA_TOTALQUADCOUNT     + 1)
#define RA_TOTALPRIMCOUNT      (RA_VALIDQUADCOUNTEZ   + 1)
#define RA_PIPECACHEMISSCOUNT  (RA_TOTALPRIMCOUNT     + 1)
#define RA_PREFCACHEMISSCOUNT  (RA_PIPECACHEMISSCOUNT + 1)
#define RA_EEZCULLCOUNT        (RA_PREFCACHEMISSCOUNT + 1)
#define RA_NONIDLESTARVECOUNT  (RA_EEZCULLCOUNT       + 1)
#define RA_STARVELCOUNT        (RA_NONIDLESTARVECOUNT + 1)
#define RA_STALLCOUNT          (RA_STARVELCOUNT       + 1)
#define RA_PROCESSCOUNT        (RA_STALLCOUNT         + 1)

#define TX_TOTBILINEARREQ      1
#define TX_TOTTRILINEARREQ     (TX_TOTBILINEARREQ      + 1)
#define TX_TOTDISCARDTEXREQ    (TX_TOTTRILINEARREQ     + 1)
#define TX_TOTTEXREQ           (TX_TOTDISCARDTEXREQ    + 1)
#define TX_MEMREADCOUNT        (TX_TOTTEXREQ           + 1)
#define TX_MEMREADIN8BCOUNT    (TX_MEMREADCOUNT        + 1)
#define TX_CACHEMISSCOUNT      (TX_MEMREADIN8BCOUNT    + 1)
#define TX_CACHEHITTEXELCOUNT  (TX_CACHEMISSCOUNT      + 1)
#define TX_CACHEMISSTEXELCOUNT (TX_CACHEHITTEXELCOUNT  + 1)
#define TX_NONIDLESTARVECOUNT  (TX_CACHEMISSTEXELCOUNT + 1)
#define TX_STARVELCOUNT        (TX_NONIDLESTARVECOUNT  + 1)
#define TX_STALLCOUNT          (TX_STARVELCOUNT        + 1)
#define TX_PROCESSCOUNT        (TX_STALLCOUNT          + 1)

#define PE_KILLEDBYCOLOR      1
#define PE_KILLEDBYDEPTH      (PE_KILLEDBYCOLOR     + 1)
#define PE_DRAWNBYCOLOR       (PE_KILLEDBYDEPTH     + 1)
#define PE_DRAWNBYDEPTH       (PE_DRAWNBYCOLOR      + 1)

#define MC_READREQ8BPIPE      1
#define MC_READREQ8BIP        (MC_READREQ8BPIPE     + 1)
#define MC_WRITEREQ8BPIPE     (MC_READREQ8BIP       + 1)
#define MC_AXIMINLATENCY      (MC_WRITEREQ8BPIPE    + 1)
#define MC_AXIMAXLATENCY      (MC_AXIMINLATENCY     + 1)
#define MC_AXITOTALLATENCY    (MC_AXIMAXLATENCY     + 1)
#define MC_AXISAMPLECOUNT     (MC_AXITOTALLATENCY   + 1)

#define AXI_READREQSTALLED    1
#define AXI_WRITEREQSTALLED   (AXI_READREQSTALLED   + 1)
#define AXI_WRITEDATASTALLED  (AXI_WRITEREQSTALLED  + 1)

#define FE_DRAWCOUNT          1
#define FE_OUTVERTEXCOUNT     (FE_DRAWCOUNT         + 1)
#define FE_STALLCOUNT         (FE_OUTVERTEXCOUNT    + 1)
#define FE_STARVECOUNT        (FE_STALLCOUNT        + 1)

#define PVS_INSTRCOUNT        1
#define PVS_ALUINSTRCOUNT     (PVS_INSTRCOUNT       + 1)
#define PVS_TEXINSTRCOUNT     (PVS_ALUINSTRCOUNT    + 1)
#define PVS_ATTRIBCOUNT       (PVS_TEXINSTRCOUNT    + 1)
#define PVS_UNIFORMCOUNT      (PVS_ATTRIBCOUNT      + 1)
#define PVS_FUNCTIONCOUNT     (PVS_UNIFORMCOUNT     + 1)
#define PVS_SOURCE            (PVS_FUNCTIONCOUNT    + 1)

#define PPS_INSTRCOUNT        1
#define PPS_ALUINSTRCOUNT     (PPS_INSTRCOUNT       + 1)
#define PPS_TEXINSTRCOUNT     (PPS_ALUINSTRCOUNT    + 1)
#define PPS_ATTRIBCOUNT       (PPS_TEXINSTRCOUNT    + 1)
#define PPS_UNIFORMCOUNT      (PPS_ATTRIBCOUNT      + 1)
#define PPS_FUNCTIONCOUNT     (PPS_UNIFORMCOUNT     + 1)
#define PPS_SOURCE            (PPS_FUNCTIONCOUNT    + 1)
/* End of MISC Counter IDs. */

#define VPG(x) (gcoPROFILER_getVPGConst(x))

enum gceVPG {
    VPHEADER,
    INFO,
    FRAME,
    VPTIME,
    ES11,
    VG11,
    HW,
    MULTI_GPU,
    PROG,
    ES11DRAW,
    MEM,
    PVS,
    PPS,
    ES11_TIME,
    ES30,
    ES30_DRAW,
    ES30_TIME,
    FINISH,
    END,
    COMPUTE30,
    BLT,
    CLUSTER,
    PVIP,
};

/* Category Constants. */
#define VPG_HAL              0x080000
#define VPG_GPU              0x0a0000
#define VPG_VS               0x0b0000
#define VPG_PS               0x0c0000
#define VPG_PA               0x0d0000
#define VPG_SETUP            0x0e0000
#define VPG_RA               0x0f0000
#define VPG_TX               0x100000
#define VPG_PE               0x110000
#define VPG_MC               0x120000
#define VPG_AXI              0x130000
#define VPG_VG11_TIME        0x220000
#define VPG_FE               0x230000
#define VPNG_FE              0x250000
#define VPNG_VS              0x260000
#define VPNG_PS              0x270000
#define VPNG_PA              0x280000
#define VPNG_SETUP           0x290000
#define VPNG_RA              0x2a0000
#define VPNG_TX              0x2b0000
#define VPNG_PE              0x2c0000
#define VPNG_MCC             0x2d0000
#define VPNG_MCZ             0x2e0000
#define VPNG_HI              0x2f0000
#define VPNG_L2              0x300000
#define VPNG_NN              0x310000
#define VPNG_TP              0x320000
#define VPG_CLUSTER          0x68000000

/* Info. */
#define VPC_INFOCOMPANY         (VPG(INFO) + 1)
#define VPC_INFOVERSION         (VPC_INFOCOMPANY + 1)
#define VPC_INFORENDERER        (VPC_INFOVERSION + 1)
#define VPC_INFOREVISION        (VPC_INFORENDERER + 1)
#define VPC_INFODRIVER          (VPC_INFOREVISION + 1)
#define VPC_INFODRIVERMODE      (VPC_INFODRIVER + 1)
#define VPC_INFOSCREENSIZE      (VPC_INFODRIVERMODE + 1)
#define VPC_INFOASICMODE        (VPC_INFOSCREENSIZE + 1)

/* Counter Constants. */
#define VPC_ELAPSETIME          (VPG(VPTIME) + 1)
#define VPC_CPUTIME             (VPC_ELAPSETIME + 1)

#define VPC_MEMMAXRES           (VPG(MEM) + 1)
#define VPC_MEMSHARED           (VPC_MEMMAXRES + 1)
#define VPC_MEMUNSHAREDDATA     (VPC_MEMSHARED + 1)
#define VPC_MEMUNSHAREDSTACK    (VPC_MEMUNSHAREDDATA + 1)

/* OpenGL ES11 Statics Counter IDs. */
#define    VPC_ES11CALLS            (VPG(ES11) +    ES11_CALLS)
#define    VPC_ES11DRAWCALLS        (VPG(ES11) +    ES11_DRAWCALLS)
#define    VPC_ES11STATECHANGECALLS (VPG(ES11) +    ES11_STATECHANGECALLS)
#define    VPC_ES11POINTCOUNT       (VPG(ES11) +    ES11_POINTCOUNT)
#define    VPC_ES11LINECOUNT        (VPG(ES11) +    ES11_LINECOUNT)
#define    VPC_ES11TRIANGLECOUNT    (VPG(ES11) +    ES11_TRIANGLECOUNT)

/* OpenGL ES30 Statistics Counter IDs. */
#define    VPC_ES30CALLS            (VPG(ES30) +    ES30_CALLS)
#define    VPC_ES30DRAWCALLS        (VPG(ES30) +    ES30_DRAWCALLS)
#define    VPC_ES30STATECHANGECALLS (VPG(ES30) +    ES30_STATECHANGECALLS)
#define    VPC_ES30POINTCOUNT       (VPG(ES30) +    ES30_POINTCOUNT)
#define    VPC_ES30LINECOUNT        (VPG(ES30) +    ES30_LINECOUNT)
#define    VPC_ES30TRIANGLECOUNT    (VPG(ES30) +    ES30_TRIANGLECOUNT)

/* OpenVG Statistics Counter IDs. */
#define    VPC_VG11CALLS            (VPG(VG11) +    VG11_CALLS)
#define    VPC_VG11DRAWCALLS        (VPG(VG11) +    VG11_DRAWCALLS)
#define    VPC_VG11STATECHANGECALLS (VPG(VG11) +    VG11_STATECHANGECALLS)
#define    VPC_VG11FILLCOUNT        (VPG(VG11) +    VG11_FILLCOUNT)
#define    VPC_VG11STROKECOUNT      (VPG(VG11) +    VG11_STROKECOUNT)

/* HAL Counters. */
#define VPC_HALVERTBUFNEWBYTEALLOC      (VPG_HAL + HAL_VERTBUFNEWBYTEALLOC)
#define VPC_HALVERTBUFTOTALBYTEALLOC    (VPG_HAL + HAL_VERTBUFTOTALBYTEALLOC)
#define VPC_HALVERTBUFNEWOBJALLOC       (VPG_HAL + HAL_VERTBUFNEWOBJALLOC)
#define VPC_HALVERTBUFTOTALOBJALLOC     (VPG_HAL + HAL_VERTBUFTOTALOBJALLOC)
#define VPC_HALINDBUFNEWBYTEALLOC       (VPG_HAL + HAL_INDBUFNEWBYTEALLOC)
#define VPC_HALINDBUFTOTALBYTEALLOC     (VPG_HAL + HAL_INDBUFTOTALBYTEALLOC)
#define VPC_HALINDBUFNEWOBJALLOC        (VPG_HAL + HAL_INDBUFNEWOBJALLOC)
#define VPC_HALINDBUFTOTALOBJALLOC      (VPG_HAL + HAL_INDBUFTOTALOBJALLOC)
#define VPC_HALTEXBUFNEWBYTEALLOC       (VPG_HAL + HAL_TEXBUFNEWBYTEALLOC)
#define VPC_HALTEXBUFTOTALBYTEALLOC     (VPG_HAL + HAL_TEXBUFTOTALBYTEALLOC)
#define VPC_HALTEXBUFNEWOBJALLOC        (VPG_HAL + HAL_TEXBUFNEWOBJALLOC)
#define VPC_HALTEXBUFTOTALOBJALLOC      (VPG_HAL + HAL_TEXBUFTOTALOBJALLOC)

/* HW: GPU Counters. */
#define VPC_GPUCYCLES                   (VPG_GPU + GPU_CYCLES)
#define VPC_GPUREAD64BYTE               (VPG_GPU + GPU_READ64BYTE)
#define VPC_GPUWRITE64BYTE              (VPG_GPU + GPU_WRITE64BYTE)
#define VPC_GPUTOTALCYCLES              (VPG_GPU + GPU_TOTALCYCLES)
#define VPC_GPUIDLECYCLES               (VPG_GPU + GPU_IDLECYCLES)

#define TOTAL_MODULE_NUMBER              12

/* PROGRAM: Shader program counters. */
#define VPC_PVSINSTRCOUNT           (VPG(PVS) + PVS_INSTRCOUNT)
#define VPC_PVSALUINSTRCOUNT        (VPG(PVS) + PVS_ALUINSTRCOUNT)
#define VPC_PVSTEXINSTRCOUNT        (VPG(PVS) + PVS_TEXINSTRCOUNT)
#define VPC_PVSATTRIBCOUNT          (VPG(PVS) + PVS_ATTRIBCOUNT)
#define VPC_PVSUNIFORMCOUNT         (VPG(PVS) + PVS_UNIFORMCOUNT)
#define VPC_PVSFUNCTIONCOUNT        (VPG(PVS) + PVS_FUNCTIONCOUNT)
#define VPC_PVSSOURCE               (VPG(PVS) + PVS_SOURCE)

#define VPC_PPSINSTRCOUNT           (VPG(PPS) + PPS_INSTRCOUNT)
#define VPC_PPSALUINSTRCOUNT        (VPG(PPS) + PPS_ALUINSTRCOUNT)
#define VPC_PPSTEXINSTRCOUNT        (VPG(PPS) + PPS_TEXINSTRCOUNT)
#define VPC_PPSATTRIBCOUNT          (VPG(PPS) + PPS_ATTRIBCOUNT)
#define VPC_PPSUNIFORMCOUNT         (VPG(PPS) + PPS_UNIFORMCOUNT)
#define VPC_PPSFUNCTIONCOUNT        (VPG(PPS) + PPS_FUNCTIONCOUNT)
#define VPC_PPSSOURCE               (VPG(PPS) + PPS_SOURCE)

#define VPC_PROGRAMHANDLE           (VPG(PROG) + 1)

#define VPC_ES30_DRAW_NO            (VPG(ES30_DRAW) + 1)
#define VPC_ES11_DRAW_NO            (VPG_ES11_DRAW + 1)
#define VPC_ES30_GPU_NO             (VPG(MULTI_GPU) + 1)

#define   MODULE_HOST_INTERFACE0_COUNTER_NUM              0x9
#define   MODULE_HOST_INTERFACE1_COUNTER_NUM              0x7
#define   TOTAL_PROBE_NUMBER                                                    \
            (MODULE_HOST_INTERFACE0_COUNTER_NUM + MODULE_HOST_INTERFACE1_COUNTER_NUM)

/* VIP operation info. */
#define VPC_PVIP_LAYERID            (VPG(PVIP) + 1)
#define VPC_PVIP_OPID               (VPG(PVIP) + 2)
#define VPC_PVIP_CORESYNCMASKID     (VPG(PVIP) + 3)
#define VPC_PVIP_ABSOPID            (VPG(PVIP) + 4)
#define VPC_PVIP_COREID             (VPG(PVIP) + 5)
#define VPC_PVIP_LAYERNAME          (VPG(PVIP) + 6)
#define VPC_PVIP_OPUID              (VPG(PVIP) + 7)

#define VPNC_NN_LAYER_ID                        (VPNG_NN + 1)
#define VPNC_NN_LAYER_ID_OVFL                   (VPNG_NN + 2)
#define VPNC_NN_INSTR_INFO                      (VPNG_NN + 3)
#define VPNC_NN_TOTAL_BUSY_CYCLE                (VPNG_NN + 4)
#define VPNC_NN_TOTAL_BUSY_CYCLE_OVFL           (VPNG_NN + 5)
#define VPNC_NN_TOTAL_READ_CYCLE_DDR            (VPNG_NN + 6)
#define VPNC_NN_TOTAL_READ_CYCLE_DDR_OVFL       (VPNG_NN + 7)
#define VPNC_NN_TOTAL_READ_VALID_BW_DDR         (VPNG_NN + 8)
#define VPNC_NN_TOTAL_READ_VALID_BW_DDR_OVFL    (VPNG_NN + 9)
#define VPNC_NN_TOTAL_WRITE_CYCLE_DDR           (VPNG_NN + 10)
#define VPNC_NN_TOTAL_WRITE_CYCLE_DDR_OVFL      (VPNG_NN + 11)
#define VPNC_NN_TOTAL_WRITE_VALID_BW_DDR        (VPNG_NN + 12)
#define VPNC_NN_TOTAL_WRITE_VALID_BW_DDR_OVFL   (VPNG_NN + 13)
#define VPNC_NN_TOTAL_READ_CYCLE_SRAM           (VPNG_NN + 14)
#define VPNC_NN_TOTAL_READ_CYCLE_SRAM_OVFL      (VPNG_NN + 15)
#define VPNC_NN_TOTAL_WRITE_CYCLE_SRAM          (VPNG_NN + 16)
#define VPNC_NN_TOTAL_WRITE_CYCLE_SRAM_OVFL     (VPNG_NN + 17)
#define VPNC_NN_TOTAL_MAC_CYCLE                 (VPNG_NN + 18)
#define VPNC_NN_TOTAL_MAC_CYCLE_OVFL            (VPNG_NN + 19)
#define VPNC_NN_TOTAL_MAC_COUNT                 (VPNG_NN + 20)
#define VPNC_NN_TOTAL_MAC_COUNT_OVFL            (VPNG_NN + 21)
#define VPNC_NN_ZERO_COEF_SKIP_COUNT            (VPNG_NN + 22)
#define VPNC_NN_ZERO_COEF_SKIP_COUNT_OVFL       (VPNG_NN + 23)
#define VPNC_NN_NON_ZERO_COEF_COUNT             (VPNG_NN + 24)
#define VPNC_NN_NON_ZERO_COEF_COUNT_OVFL        (VPNG_NN + 25)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE_OVFL      (VPNG_NN + 26)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE0          (VPNG_NN + 27)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE1          (VPNG_NN + 28)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE2          (VPNG_NN + 29)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE3          (VPNG_NN + 30)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE4          (VPNG_NN + 31)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE5          (VPNG_NN + 32)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE6          (VPNG_NN + 33)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE7          (VPNG_NN + 34)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE8          (VPNG_NN + 35)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE9          (VPNG_NN + 36)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE10         (VPNG_NN + 37)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE11         (VPNG_NN + 38)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE12         (VPNG_NN + 39)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE13         (VPNG_NN + 40)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE14         (VPNG_NN + 41)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE15         (VPNG_NN + 42)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE16         (VPNG_NN + 43)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE17         (VPNG_NN + 44)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE18         (VPNG_NN + 45)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE19         (VPNG_NN + 46)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE20         (VPNG_NN + 47)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE21         (VPNG_NN + 48)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE22         (VPNG_NN + 49)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE23         (VPNG_NN + 50)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE24         (VPNG_NN + 51)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE25         (VPNG_NN + 52)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE26         (VPNG_NN + 53)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE27         (VPNG_NN + 54)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE28         (VPNG_NN + 55)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE29         (VPNG_NN + 56)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE30         (VPNG_NN + 57)
#define VPNC_NN_TOTAL_IDLE_CYCLE_CORE31         (VPNG_NN + 58)
#define VPNC_NN_COUNT                           (VPNC_NN_TOTAL_IDLE_CYCLE_CORE31 - VPNG_NN)

/* HW: TP Probe Counters. */
#define VPNC_TP_LAYER_ID                      (VPNG_TP + 1)
#define VPNC_TP_LAYER_ID_OVFL                 (VPNG_TP + 2)
#define VPNC_TP_TOTAL_BUSY_CYCLE              (VPNG_TP + 3)
#define VPNC_TP_TOTAL_BUSY_CYCLE_OVFL         (VPNG_TP + 4)
#define VPNC_TP_TOTAL_READ_BW_CACHE           (VPNG_TP + 5)
#define VPNC_TP_TOTAL_READ_BW_CACHE_OVFL      (VPNG_TP + 6)
#define VPNC_TP_TOTAL_WRITE_BW_CACHE          (VPNG_TP + 7)
#define VPNC_TP_TOTAL_WRITE_BW_CACHE_OVFL     (VPNG_TP + 8)
#define VPNC_TP_TOTAL_READ_BW_SRAM            (VPNG_TP + 9)
#define VPNC_TP_TOTAL_READ_BW_SRAM_OVFL       (VPNG_TP + 10)
#define VPNC_TP_TOTAL_WRITE_BW_SRAM           (VPNG_TP + 11)
#define VPNC_TP_TOTAL_WRITE_BW_SRAM_OVFL      (VPNG_TP + 12)
#define VPNC_TP_TOTAL_READ_BW_OCB             (VPNG_TP + 13)
#define VPNC_TP_TOTAL_READ_BW_OCB_OVFL        (VPNG_TP + 14)
#define VPNC_TP_TOTAL_WRITE_BW_OCB            (VPNG_TP + 15)
#define VPNC_TP_TOTAL_WRITE_BW_OCB_OVFL       (VPNG_TP + 16)
#define VPNC_TP_FC_PIX_CNT                    (VPNG_TP + 17)
#define VPNC_TP_FC_ZERO_SKIP_CNT              (VPNG_TP + 18)
#define VPNC_TP_FC_PIX_CNT_OVFL               (VPNG_TP + 19)
#define VPNC_TP_FC_ZERO_SKIP_CNT_OVFL         (VPNG_TP + 20)
#define VPNC_TP_FC_COEF_CNT                   (VPNG_TP + 21)
#define VPNC_TP_FC_COEF_ZERO_CNT              (VPNG_TP + 22)
#define VPNC_TP_FC_COEF_CNT_OVFL              (VPNG_TP + 23)
#define VPNC_TP_FC_COEF_ZERO_CNT_OVFL         (VPNG_TP + 24)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE0        (VPNG_TP + 25)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE0_OVFL   (VPNG_TP + 26)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE1        (VPNG_TP + 27)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE1_OVFL   (VPNG_TP + 28)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE2        (VPNG_TP + 29)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE2_OVFL   (VPNG_TP + 30)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE3        (VPNG_TP + 31)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE3_OVFL   (VPNG_TP + 32)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE4        (VPNG_TP + 33)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE4_OVFL   (VPNG_TP + 34)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE5        (VPNG_TP + 35)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE5_OVFL   (VPNG_TP + 36)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE6        (VPNG_TP + 37)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE6_OVFL   (VPNG_TP + 38)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE7        (VPNG_TP + 39)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE7_OVFL   (VPNG_TP + 40)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE8        (VPNG_TP + 41)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE8_OVFL   (VPNG_TP + 42)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE9        (VPNG_TP + 43)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE9_OVFL   (VPNG_TP + 44)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE10       (VPNG_TP + 45)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE10_OVFL  (VPNG_TP + 46)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE11       (VPNG_TP + 47)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE11_OVFL  (VPNG_TP + 48)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE12       (VPNG_TP + 49)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE12_OVFL  (VPNG_TP + 50)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE13       (VPNG_TP + 51)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE13_OVFL  (VPNG_TP + 52)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE14       (VPNG_TP + 53)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE14_OVFL  (VPNG_TP + 54)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE15       (VPNG_TP + 55)
#define VPNC_TP_TOTAL_IDLE_CYCLE_CORE15_OVFL  (VPNG_TP + 56)
#define VPNC_TP_COUNT                         (VPNC_TP_TOTAL_IDLE_CYCLE_CORE15_OVFL - VPNG_TP)

#define   MODULE_NN_BASE_COUNTER_NUM          0x6
/*#define   MODULE_NN_RESERVED_COUNTER_NUM           0x9*/
#define   MODULE_NN_IDLE_COUNTER_NUM          0x9
#define   MODULE_NN_COUNTER_NUM               (MODULE_NN_BASE_COUNTER_NUM + MODULE_NN_RESERVED_COUNTER_NUM + MODULE_NN_IDLE_COUNTER_NUM)

#define  TOTAL_VIP_COUNTER_NUMBER       (VPNC_TP_COUNT + VPNC_NN_COUNT)
#define   TOTAL_VIP_MODULE_NUMBER       2
#define   MODULE_TP_COUNTER_NUM         0xE
#define   TOTAL_VIP_PROBE_NUMBER        (MODULE_NN_COUNTER_NUM + MODULE_TP_COUNTER_NUM)

#define MDUID_NN                        0x00190000

#ifdef ANDROID
# define DEFAULT_PROFILE_FILE_NAME   "/sdcard/vprofiler.vpd"
#else
# define DEFAULT_PROFILE_FILE_NAME   "vprofiler.vpd"
#endif

#define VPHEADER_VERSION                "VP22"

#define VPFILETYPE_GL                   "10"
#define VPFILETYPE_CL                   "00"
#define VPFILETYPE_VX                   "20"

#if gcdENDIAN_BIG
# define BIG_ENDIAN_TRANS_INT(x)                                          \
    ((gctUINT32)((((gctUINT32)(x) & (gctUINT32)0x000000FFUL) << 24) |     \
                 (((gctUINT32)(x) & (gctUINT32)0x0000FF00UL) << 8)  |     \
                 (((gctUINT32)(x) & (gctUINT32)0x00FF0000UL) >> 8)  |     \
                 (((gctUINT32)(x) & (gctUINT32)0xFF000000UL) >> 24)))
#else
# define BIG_ENDIAN_TRANS_INT(x) x
#endif

/* Write a data value. */
#define gcmWRITE_VALUE(IntData)                                               \
    do {                                                                      \
        gceSTATUS status;                                                     \
        gctINT32  value = IntData;                                            \
        value           = BIG_ENDIAN_TRANS_INT(value);                        \
        gcmERR_BREAK(gcoPROFILER_Write(Profiler, gcmSIZEOF(value), &value));  \
    } while (gcvFALSE)

#define gcmWRITE_CONST(Const)                                                 \
    do {                                                                      \
        gceSTATUS status;                                                     \
        gctINT32  data = Const;                                               \
        data           = BIG_ENDIAN_TRANS_INT(data);                          \
        gcmERR_BREAK(gcoPROFILER_Write(Profiler, gcmSIZEOF(data), &data));    \
    } while (gcvFALSE)

#define gcmWRITE_COUNTER(Counter, Value)                                      \
    do {                                                                      \
        gcmWRITE_CONST(Counter);                                              \
        gcmWRITE_VALUE(Value);                                                \
    } while (gcvFALSE)

/* Write a data value. */
#define gcmRECORD_VALUE(IntData)                                              \
    do {                                                                      \
        gctINT32 value              = IntData;                                \
        value                       = BIG_ENDIAN_TRANS_INT(value);            \
        counterData[counterIndex++] = value;                                  \
    } while (gcvFALSE)

#define gcmRECORD_CONST(Const)                                                \
    do {                                                                      \
        gctINT32 data               = Const;                                  \
        data                        = BIG_ENDIAN_TRANS_INT(data);             \
        counterData[counterIndex++] = data;                                   \
    } while (gcvFALSE)

#define gcmRECORD_COUNTER(Counter, Value)                                     \
    do {                                                                      \
        gcmRECORD_CONST(Counter);                                             \
        gcmRECORD_VALUE(Value);                                               \
    } while (gcvFALSE)

/* Write a string value (char*). */
#define gcmWRITE_STRING(String)                                                          \
    do {                                                                                 \
        gceSTATUS status;                                                                \
        gctINT32  length;                                                                \
        length = (gctINT32)gcoOS_StrLen((gctSTRING)String, gcvNULL);                     \
        length = BIG_ENDIAN_TRANS_INT(length);                                           \
        gcmERR_BREAK(gcoPROFILER_Write(Profiler, gcmSIZEOF(length), &length));           \
        gcmERR_BREAK(gcoPROFILER_Write(Profiler, length, String));                       \
    } while (gcvFALSE)

#define gcmWRITE_BUFFER(Size, Buffer)                                                    \
    do {                                                                                 \
        gceSTATUS status;                                                                \
        gcmERR_BREAK(gcoPROFILER_Write(Profiler, Size, Buffer));                         \
    } while (gcvFALSE)

#define gcmGET_COUNTER(counter, counterId)                                               \
    do {                                                                                 \
        if (*(memory + (counterId + offset) * (1 << clusterIDWidth)) == 0xdeaddead) {    \
            counter = 0xdeaddead;                                                        \
        } else {                                                                         \
            gctUINT32     i;                                                             \
            gctUINT32_PTR Memory             = memory;                                   \
            gctUINT32     total_probe_number = 0;                                        \
            counter                          = 0;                                        \
            gcmONERROR(gcoPROFILER_GetProbeNumber(Profiler, &total_probe_number));       \
            Memory = memory + total_probe_number * CoreId * (1 << clusterIDWidth);       \
            for (i = 0; i < (gctUINT32)(1 << clusterIDWidth); i++) {                     \
                counter += *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i); \
            }                                                                            \
        }                                                                                \
    } while (gcvFALSE)

#define gcmGET_MAXCOUNTER(counter, counterId)                                                   \
    do {                                                                                        \
        if (*(memory + (counterId + offset) * (1 << clusterIDWidth)) == 0xdeaddead) {           \
            counter = 0xdeaddead;                                                               \
        } else {                                                                                \
            gctUINT32 i;                                                                        \
            gctUINT32_PTR Memory = memory;                                                      \
            gctUINT32 total_probe_number = 0;                                                   \
            gctUINT32 max_counter = 0;                                                          \
                                                                                                \
            counter = 0;                                                                        \
            gcmONERROR(gcoPROFILER_GetProbeNumber(Profiler, &total_probe_number));              \
            Memory = memory + total_probe_number * CoreId * (1 << clusterIDWidth);              \
            for (i = 0; i < (gctUINT32)(1 << clusterIDWidth); i++) {                            \
                if (max_counter < *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i)) \
                    max_counter = *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i); \
            }                                                                                   \
            counter = max_counter;                                                              \
        }                                                                                       \
    } while (gcvFALSE)

#define gcmGET_MINCOUNTER(counter, counterId)                                                   \
    do {                                                                                        \
        if (*(memory + (counterId + offset) * (1 << clusterIDWidth)) == 0xdeaddead) {           \
            counter = 0xdeaddead;                                                               \
        } else {                                                                                \
            gctUINT32 i;                                                                        \
            gctUINT32_PTR Memory = memory;                                                      \
            gctUINT32 total_probe_number = 0;                                                   \
            gctUINT32 min_counter = 0;                                                          \
            counter = 0;                                                                        \
            gcmONERROR(gcoPROFILER_GetProbeNumber(Profiler, &total_probe_number));              \
            Memory = memory + total_probe_number * CoreId * (1 << clusterIDWidth);              \
            min_counter = *(Memory + (counterId + offset) * (1 << clusterIDWidth));             \
            for (i = 0; i < (gctUINT32)(1 << clusterIDWidth); i++) {                            \
            if (Profiler->isDebugCounter) \
            { \
                gcmPRINT("%s %d hi_total_idle_cycle_count CoreId=%d clusterId=%d offset=%u cpuaddress=%p value=%u", __FUNCTION__, __LINE__, CoreId, i, offset, (Memory + (counterId + offset) * (1 << clusterIDWidth) + i), *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i)); \
            } \
                if (min_counter > *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i)) \
                    min_counter = *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i); \
            }                                                                                   \
            counter = min_counter;                                                              \
        }                                                                                       \
    } while (gcvFALSE)

#define gcmGET_VIPCOUNTER(counter, counterId)                                            \
    do {                                                                                 \
        if (*(memory + (counterId + offset) * (1 << clusterIDWidth)) == 0xdeaddead) {    \
            counter = 0xdeaddead;                                                        \
        } else {                                                                         \
            gctUINT32     i;                                                             \
            gctUINT32_PTR Memory = memory;                                               \
            counter = 0;                                                                 \
            Memory  = memory + TOTAL_VIP_PROBE_NUMBER * CoreId * (1 << clusterIDWidth);  \
            for (i = 0; i < (gctUINT32)(1 << clusterIDWidth); i++) {                     \
                counter += *(Memory + (counterId + offset) * (1 << clusterIDWidth) + i); \
            }                                                                            \
        }                                                                                \
    } while (gcvFALSE)

#define gcmGET_LATENCY_COUNTER(minLatency, maxLatency, counterId)                                      \
    do {                                                                                               \
        if (*(memory + (counterId + offset) * (1 << clusterIDWidth)) == 0xdeaddead) {                  \
            minLatency = 0xdeaddead;                                                                   \
            maxLatency = 0xdeaddead;                                                                   \
        } else {                                                                                       \
            gctUINT32     i;                                                                           \
            gctUINT32_PTR Memory             = memory;                                                 \
            gctUINT32     total_probe_number = 0;                                                      \
            gcmONERROR(gcoPROFILER_GetProbeNumber(Profiler, &total_probe_number));                     \
            Memory = memory + total_probe_number * CoreId * (1 << clusterIDWidth);                     \
            for (i = 0; i < (gctUINT32)(1 << clusterIDWidth); i++) {                                   \
                maxLatency +=                                                                          \
                    ((*(Memory + (counterId + offset) * (1 << clusterIDWidth) + i) & 0xfff000) >> 12); \
                minLatency +=                                                                          \
                    (*(Memory + (counterId + offset) * (1 << clusterIDWidth) + i) & 0x000fff);         \
                if (minLatency == 4095)                                                                \
                    minLatency = 0;                                                                    \
            }                                                                                          \
        }                                                                                              \
    } while (gcvFALSE)

#define NumOfPerFrameBuf        16
#define NumOfPerDrawBuf         128

struct _gcsAppInfoCounter {
    gctUINT32                   count[7];
};

typedef struct _gcsAppInfoCounter gcsAppInfoCounter;

typedef struct gcsCounterBuffer   *gcsCounterBuffer_PTR;

struct gcsCounterBuffer {
    gctPOINTER                      counters;
    gcsPROFILER_VIP_PROBE_COUNTERS *vipCounters;
    gctHANDLE                       couterBufobj;
    gctADDRESS                      probeAddress;
    gctPOINTER                      logicalAddress;
    gceCOUNTER_OPTYPE               opType;
    gctUINT32                       opID;
    gcsAppInfoCounter               opCount;
    gctUINT32                       currentShaderId[6];
    gctUINT32                       startPos;
    gctUINT32                       endPos;
    gctUINT32                       dataSize;
    gctBOOL                         available;
    gctBOOL                         needDump;
    gcsCounterBuffer_PTR            next;
    gcsCounterBuffer_PTR            prev;
};

typedef struct _gcoPROBE            gcoPROBE;
struct _gcoPROBE {
    gctUINT32                   address;
    gctUINT32                   offset;
};

typedef struct _gcoMODULE           gcoMODULE;
struct _gcoMODULE {
    gctUINT32                   name;
    gctUINT32                   address;
    gctUINT32                   numProbe;
    gcoPROBE                    probe[256];
};

typedef struct _gcoPROFILER         *gcoPROFILER;

struct _gcoPROFILER {
    gctBOOL                     enable;
    gctBOOL                     enablePrint;
    gctBOOL                     disableProbe;

    gctBOOL                     vipProbe;

    gctFILE                     file;
    gctCHAR                    *fileName;
    gceProfilerMode             profilerMode;
    gceProbeMode                probeMode;

    gcsCounterBuffer_PTR        counterBuf;
    gcsAppInfoCounter           currentOpCount;
    gctUINT32                   bufferCount;

    gctBOOL                     perDrawMode;
    gctBOOL                     needDump;
    gctBOOL                     counterEnable;

    gceProfilerClient           profilerClient;

    gctBOOL                     needBltDump;
    gctBOOL                     isDummyDraw;
    gctBOOL                     isDebugCounter;
    gctUINT32                   rdByte;
    gctUINT32                   wrByte;
    gctUINT32                   busyCycle;
    gctUINT64                   totalCycle;
    gctUINT64_PTR               globaltimerStart;
    gctUINT64_PTR               globaltimerEnd;

    /*query some features from hw*/
    gctUINT32                   coreCount;
    gctUINT32                   shaderCoreCount;
    gctBOOL                     bHalti4;
    gctBOOL                     psRenderPixelFix;
    gctBOOL                     axiBus128bits;
    gctBOOL                     bZDP3;
};

typedef struct _gcsPROBESTATES {
    gceProbeStatus              status;
    gctADDRESS                  probeAddress;
} gcsPROBESTATES;

typedef struct _gckPROFILER {
    /* Enable profiling */
    gctBOOL                     profileEnable;
    /* Profile mode */
    gceProfilerMode             profileMode;
    /* Probe mode */
    gceProbeMode                probeMode;
    /* Clear profile register or not*/
    gctBOOL                     profileCleanRegister;
    /* Profile counter */
    gcsPROFILER_COUNTERS  latestProfiler;
    gcsPROFILER_COUNTERS  histroyProfiler;
    gcsPROFILER_COUNTERS  preProfiler;
} gckPROFILER;

/* Construct a Profiler object per context. */
gceSTATUS
gcoPROFILER_Construct(OUT gcoPROFILER *Profiler);

gceSTATUS
gcoPROFILER_Destroy(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_Initialize(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_Enable(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_Disable(void);

gceSTATUS
gcoPROFILER_EnableCounters(IN gcoPROFILER Profiler,
                           IN gceCOUNTER_OPTYPE operationType);

gceSTATUS
gcoPROFILER_Start(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_End(IN gcoPROFILER Profiler,
                IN gceCOUNTER_OPTYPE operationType,
                IN gctUINT32 OpID);

gceSTATUS
gcoPROFILER_Write(IN gcoPROFILER Profiler,
                  IN gctSIZE_T ByteCount,
                  IN gctCONST_POINTER Data);

gceSTATUS
gcoPROFILER_Flush(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_GetProbeNumber(IN gcoPROFILER Profiler,
                           OUT gctUINT32 *TotalProbeNumber);

gctUINT32
gcoPROFILER_getMuduleNum(IN gcoPROFILER Profiler);

gctUINT32
gcoPROFILER_getMuduleProbeNum(IN gcoPROFILER Profiler, IN gctUINT32 index);

gctUINT32
gcoPROFILER_getModuleAddress(IN gcoPROFILER Profiler, IN gctUINT32 ModuleIndex);

gctUINT32
gcoPROFILER_getProbeAddress(IN gcoPROFILER Profiler,
                            IN gctUINT32 ModuleIndex,
                            IN gctUINT32 ProbeIndex);

gctUINT32
gcoPROFILER_getHIIndex(IN gcoPROFILER Profiler);

gctUINT32
gcoPROFILER_getCounterBufferSize(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_WriteChipInfo(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_WriteClusterInfo(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_Reset(IN gcoPROFILER Profiler);

gceSTATUS
gcoPROFILER_WriteCounters(IN gcoPROFILER Profiler);

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_profiler_h_ */

