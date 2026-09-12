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


#ifndef __gc_hal_priv_h_
#define __gc_hal_priv_h_

#include "gc_hal_base.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gcsSYMBOLSLIST     *gcsSYMBOLSLIST_PTR;
/******************************************************************************
 **
 ** Patch defines which should be moved to dedicate file later
 **
 ** !!! ALWAYS ADD new ID in the TAIL, otherwise will break exising TRACE FILE
 ******************************************************************************/
typedef enum _gcePATCH_ID {
    gcvPATCH_NOTINIT = -1,
    gcvPATCH_INVALID = 0,

    gcvPATCH_DEBUG,                          /* 1 */
    gcvPATCH_GTFES30,                        /* 2 */
    gcvPATCH_CTGL11,                         /* 3 */
    gcvPATCH_CTGL20,                         /* 4 */
    gcvPATCH_GLBM11,                         /* 5 */
    gcvPATCH_GLBM21,                         /* 6 */
    gcvPATCH_GLBM25,                         /* 7 */
    gcvPATCH_GLBM27,                         /* 8 */
    gcvPATCH_GLBMGUI,                        /* 9 */
    gcvPATCH_GFXBENCH,                       /* 10 */
    gcvPATCH_ANDROID_GPUTEST,                /* 11 */
    gcvPATCH_ANTUTU,                         /* 12 */ /* Antutu 3.x */
    gcvPATCH_ANTUTU4X,                       /* 13 */ /* Antutu 4.x */
    gcvPATCH_ANTUTU5X,                       /* 14 */ /* Antutu 5.x */
    gcvPATCH_ANTUTUGL3,                      /* 15 */ /* Antutu 3D Rating */
    gcvPATCH_QUADRANT,                       /* 16 */
    gcvPATCH_GPUBENCH,                       /* 17 */
    gcvPATCH_GLOFTSXHM,                      /* 18 */
    gcvPATCH_XRUNNER,                        /* 19 */
    gcvPATCH_BUSPARKING3D,                   /* 20 */
    gcvPATCH_SIEGECRAFT,                     /* 21 */
    gcvPATCH_PREMIUM,                        /* 22 */
    gcvPATCH_RACEILLEGAL,                    /* 23 */
    gcvPATCH_MEGARUN,                        /* 24 */
    gcvPATCH_BMGUI,                          /* 25 */
    gcvPATCH_NENAMARK,                       /* 26 */
    gcvPATCH_NENAMARK2,                      /* 27 */
    gcvPATCH_FISHNOODLE,                     /* 28 */
    gcvPATCH_MM06,                           /* 29 */
    gcvPATCH_MM07,                           /* 30 */
    gcvPATCH_BM21,                           /* 31 */
    gcvPATCH_SMARTBENCH,                     /* 32 */
    gcvPATCH_JPCT,                           /* 33 */
    gcvPATCH_NEOCORE,                        /* 34 */
    gcvPATCH_RTESTVA,                        /* 35 */
    gcvPATCH_NBA2013,                        /* 36 */
    gcvPATCH_BARDTALE,                       /* 37 */
    gcvPATCH_F18,                            /* 38 */
    gcvPATCH_CARPARK,                        /* 39 */
    gcvPATCH_CARCHALLENGE,                   /* 40 */
    gcvPATCH_HEROESCALL,                     /* 41 */
    gcvPATCH_GLOFTF3HM,                      /* 42 */
    gcvPATCH_CRAZYRACING,                    /* 43 */
    gcvPATCH_FIREFOX,                        /* 44 */
    gcvPATCH_CHROME,                         /* 45 */
    gcvPATCH_MONOPOLY,                       /* 46 */
    gcvPATCH_SNOWCOLD,                       /* 47 */
    gcvPATCH_BM3,                            /* 48 */
    gcvPATCH_BASEMARKX,                      /* 49 */
    gcvPATCH_DEQP,                           /* 50 */
    gcvPATCH_SF4,                            /* 51 */
    gcvPATCH_MGOHEAVEN2,                     /* 52 */
    gcvPATCH_SILIBILI,                       /* 53 */
    gcvPATCH_ELEMENTSDEF,                    /* 54 */
    gcvPATCH_GLOFTKRHM,                      /* 55 */
    gcvPATCH_OCLCTS,                         /* 56 */
    gcvPATCH_A8HP,                           /* 57 */
    gcvPATCH_A8CN,                           /* 58 */
    gcvPATCH_WISTONESG,                      /* 59 */
    gcvPATCH_SPEEDRACE,                      /* 60 */
    gcvPATCH_FSBHAWAIIF,                     /* 61 */
    gcvPATCH_AIRNAVY,                        /* 62 */
    gcvPATCH_F18NEW,                         /* 63 */
    gcvPATCH_CKZOMBIES2,                     /* 64 */
    gcvPATCH_EADGKEEPER,                     /* 65 */
    gcvPATCH_BASEMARK2V2,                    /* 66 */
    gcvPATCH_RIPTIDEGP2,                     /* 67 */
    gcvPATCH_OESCTS,                         /* 68 */
    gcvPATCH_GANGSTAR,                       /* 69 */
    gcvPATCH_TRIAL,                          /* 70 */
    gcvPATCH_WHRKYZIXOVAN,                   /* 71 */
    gcvPATCH_GMMY16MAPFB,                    /* 72 */
    gcvPATCH_UIMARK,                         /* 73 */
    gcvPATCH_NAMESGAS,                       /* 74 */
    gcvPATCH_AFTERBURNER,                    /* 75 */
    gcvPATCH_ANDROID_CTS_MEDIA,              /* 76 */
    gcvPATCH_FM_OES_PLAYER,                  /* 77 */
    gcvPATCH_SUMSUNG_BENCH,                  /* 78 */
    gcvPATCH_ROCKSTAR_MAXPAYNE,              /* 79 */
    gcvPATCH_TITANPACKING,                   /* 80 */
    gcvPATCH_OES20SFT,                       /* 81 */
    gcvPATCH_OES30SFT,                       /* 82 */
    gcvPATCH_BASEMARKOSIICN,                 /* 83 */
    gcvPATCH_ANDROID_WEBGL,                  /* 84 */
    gcvPATCH_ANDROID_COMPOSITOR,             /* 85 */
    gcvPATCH_CTS_TEXTUREVIEW,                /* 86 */
    gcePATCH_ANDROID_CTS_GRAPHICS_GLVERSION, /* 87 */
    gcvPATCH_WATER2_CHUKONG,                 /* 88 */
    gcvPATCH_GOOGLEEARTH,                    /* 89 */
    gcvPATCH_LEANBACK,                       /* 90 */
    gcvPATCH_YOUTUBE_TV,                     /* 91 */
    gcvPATCH_NETFLIX,                        /* 92 */
    gcvPATCH_ANGRYBIRDS,                     /* 93 */
    gcvPATCH_REALRACING,                     /* 94 */
    gcvPATCH_TEMPLERUN,                      /* 95 */
    gcvPATCH_SBROWSER,                       /* 96 */
    gcvPATCH_CLASHOFCLAN,                    /* 97 */
    gcvPATCH_YOUILABS_SHADERTEST,            /* 98 */
    gcvPATCH_AXX_SAMPLE,                     /* 99 */
    gcvPATCH_3DMARKSS,                       /* 100 */
    gcvPATCH_GFXBENCH4,                      /* 101 */
    gcvPATCH_BATCHCOUNT,                     /* 102 */
    gcvPATCH_SILICONSTUDIOGPUMARK,           /* 103 */
    gcvPATCH_LEANBACKSCROLLING,              /* 104 */
    gcvPATCH_ANTUTU6X,                       /* 105 */ /* Antutu 6.x */
    gcvPATCH_ANTUTU3DBench,                  /* 106 */
    gcvPATCH_ANTUTU_V8_REFINERY,             /* 107 */
    gcvPATCH_CAR_CHASE,                      /* 108 */
    gcvPATCH_ANDROID_BROWSER,                /* 109 */
    gcvPATCH_COMPUTBENCH_CL,                 /* 110 */ /* ComputBench 1.5 */
    gcvPATCH_ANDROID_CTS_UIRENDERING,        /* 111 */
    gcvPATCH_GLU3,                           /* 112 */ /* gfx3.0 glu.*/
    gcvPATCH_GLU4,                           /* 113 */ /* gfx4.0 glu.*/
    gcvPATCH_MRVELBM20,                      /* 114 */
    gcvPATCH_WESTON,                         /* 115 */
    gcvPATCH_SKIA_SKQP,                      /* 116 */
    gcvPATCH_SASCHAWILLEMS,                  /* 117 */
    gcvPATCH_NATIVEHARDWARE_CTS,             /* 118 */
    gcvPATCH_ANDROID_PHOTOS,                 /* 119 */
    gcvPATCH_MUSTANG,                        /* 120 */
    gcvPATCH_OVX_CTS,                        /* 121 */
    gcvPATCH_DEQP_VK,                        /* 122 */
    gcvPATCH_KANZI,                          /* 123 */
    gcvPATCH_OPENCV_TEST_PHOTO,              /* 124 */
    gcvPATCH_VKMARK,                         /* 125 */
    gcvPATCH_VK_T3DSTRESSTEST,               /* 126 */
    gcvPATCH_VK_HDR02_FBBASICTONEMAPPING,    /* 127 */
    gcvPATCH_VK_BLOOM,                       /* 128 */
    gcvPATCH_VK_GFXBENCH5,                   /* 129 */
    gcvPATCH_CLBLAST,                        /* 130 */
    gcvPATCH_GRAPHICS_BENCHMARKS,            /* 131 */
    gcvPATCH_INTEL_CL_BITONICSORT,           /* 132 */
    gcvPATCH_PIGLIT_CL,                      /* 133 */
    gcvPATCH_VK_UNITY,                       /* 134 */
    gcvPATCH_VK_UNREAL,                      /* 135 */
    gcvPATCH_VK_UNITY_SAMPLEASSETS,          /* 136 */
    gcvPATCH_GLDEMO_GEOMETRY,                /* 137 */
    gcvPATCH_HP_Nano_Tesla,                  /* 138 */
    gcvPATCH_3D_PERF_FLOAT,                  /* 139 */
    gcvPATCH_SOFTISP,                        /* 140 */
    gcvPATCH_OPENCV_ALL,                     /* 141 */
    gcvPATCH_GLDEMO_INFINITE_LOOP,           /* 142 */
    gcvPATCH_VIEWPERF,                       /* 143 */
    gcvPATCH_COMPIZ,                         /* 144 */
    gcvPATCH_XORG,                           /* 145 */
    gcvPATCH_OPENCV_PERF_VIDEO,              /* 146 */
    gcvPATCH_CL_API_CREATE_BUFFER,           /* 147 */
    gcvPATCH_CLPEAK,                         /* 148 */
    gcvPATCH_GEMM,                           /* 149 */
    gcvPATCH_XWAYLAND,                       /* 150 */
    gcvPATCH_UKUI_KWIN,                      /* 151 */
    gcvPATCH_VK_TEST,                        /* 152 */
    gcvPATCH_GLSLSANDBOXPLAYER,              /* 153 */
    gcvPATCH_GLMARK2,                        /* 154 */
    gcvPATCH_GCBS_GPU,                       /* 155 */
    gcvPATCH_DX_CLIENT,                      /* 156 */
    gcvPATCH_COUNT
} gcePATCH_ID;

#if gcdENABLE_3D
#define gcdPROC_IS_WEBGL(patchId)       ((patchId) == gcvPATCH_CHROME           || \
                                         (patchId) == gcvPATCH_FIREFOX          || \
                                         (patchId) == gcvPATCH_ANDROID_WEBGL    || \
                                         (patchId) == gcvPATCH_ANDROID_BROWSER)

enum {
    gcvHINT_BIT_0 = 1 << 0,
    gcvHINT_BIT_1 = 1 << 1,
    gcvHINT_BIT_2 = 1 << 2,
};

#endif /* gcdENABLE_3D */

/******************************************************************************
 ****************************** Process local storage *************************
 ******************************************************************************/

typedef struct _gcsPLS *gcsPLS_PTR;

typedef void (*gctPLS_DESTRUCTOR) (gcsPLS_PTR);

typedef struct _gcsPLS {
    /* Global objects. */
    gcoOS                       os;
    gcoHAL                      hal;

    /* Internal memory pool. */
    gctSIZE_T                   internalSize;
    gctUINT32                   internalPhysName;
    gctPOINTER                  internalLogical;

    /* External memory pool. */
    gctSIZE_T                   externalSize;
    gctUINT32                   externalPhysName;
    gctPOINTER                  externalLogical;

    /* Contiguous memory pool. */
    gctSIZE_T                   contiguousSize;
    gctUINT32                   contiguousPhysName;
    gctPOINTER                  contiguousLogical;

    /* EGL-specific process-wide objects. */
    gctPOINTER                  eglDisplayInfo;
    gctPOINTER                  eglSurfaceInfo;
    gceSURF_FORMAT              eglConfigFormat;

    /* PLS reference count */
    gcsATOM_PTR                 reference;

    /* PorcessID of the constrcutor process */
    gctUINT32                   processID;

    /* ThreadID of the constrcutor process. */
    gctSIZE_T                   threadID;
    /* Flag for calling module destructor. */
    gctBOOL                     exiting;

    gctBOOL                     bNeedSupportNP2Texture;
    gctBOOL                     bDeviceOpen;

    gctPLS_DESTRUCTOR           destructor[gcvAPI_COUNT];
    /* Mutex to guard PLS access. currently it's for EGL.
     *  We can use this mutex for every PLS access.
     */
    gctPOINTER                  accessLock;

    /* Mutex to gurad gl FE compiler access. */
    gctPOINTER                  glFECompilerAccessLock;

    /* Mutex to gurad CL FE compiler access. */
    gctPOINTER                  clFECompilerAccessLock;

    /* Mutex to gurad VX context access. */
    gctPOINTER                  vxContextGlobalLock;

    /* Global patchID to overwrite the detection */
    gcePATCH_ID                 patchID;

    /* Global fenceID to record each fence object */
    gcsATOM_PTR                 globalFenceID;

    /* flag for memory profile */
    gctHANDLE               mainThread;
    gctBOOL                 bMemoryProfile;
    gctPOINTER              profileLock;
    gctUINT32               allocCount;
    gctUINT64               allocSize;
    gctUINT64               maxAllocSize;
    gctUINT32               freeCount;
    gctUINT64               freeSize;
    gctUINT64               currentSize;

    gctUINT32               video_allocCount;
    gctUINT64               video_allocSize;
    gctUINT64               video_maxAllocSize;
    gctUINT32               video_freeCount;
    gctUINT64               video_freeSize;
    gctUINT64               video_currentSize;

    gctBOOL                 bIsNonD3DUMD;

    gctUINT64               Gc_PrivateDmaDataStructSize;
} gcsPLS;

extern gcsPLS gcPLS;

gceSTATUS
gcoHAL_GetBaseAddr(IN  gcoHAL Hal, OUT gctUINT32 *BaseAddr);

gceSTATUS
gcoHAL_SetPatchID(IN  gcoHAL Hal, IN  gcePATCH_ID PatchID);

/* Get Patch ID based on process name */
gceSTATUS
gcoHAL_GetPatchID(IN  gcoHAL Hal, OUT gcePATCH_ID *PatchID);

gceSTATUS
gcoHAL_SetGlobalPatchID(IN  gcoHAL Hal, IN  gcePATCH_ID PatchID);

/* Detect if the current process is the executable specified. */
gceSTATUS
gcoOS_DetectProcessByName(IN gctCONST_STRING Name);

gceSTATUS
gcoOS_DetectProcessByEncryptedName(IN gctCONST_STRING Name);

gceSTATUS
gcoOS_DetectProgrameByEncryptedSymbols(IN gcsSYMBOLSLIST_PTR Symbols);

/* Get access to the process local storage. */
gceSTATUS
gcoHAL_GetPLS(OUT gcsPLS_PTR *PLS);

gceSTATUS
gcoHAL_SetPriority(IN gcoHAL Hal, IN gctUINT32 PriorityID);

gceSTATUS
gcoHAL_GetPriority(IN gcoHAL Hal, OUT gctUINT32 *PriorityID);

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_priv_h_ */
