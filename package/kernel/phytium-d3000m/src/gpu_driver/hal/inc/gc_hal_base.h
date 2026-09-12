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


#ifndef __gc_hal_base_h_
#define __gc_hal_base_h_

#include "gc_hal_enum.h"
#include "gc_hal_types.h"
#include "gc_hal_debug_zones.h"
#include "shared/gc_hal_base_shared.h"

#ifdef VSIMULATOR_DEBUG
#include "../../vsimulator/common/inc/vSimComm.hpp"
#endif

#ifdef __QNXNTO__
# define CHECK_PRINTF_FORMAT(string_index, first_to_check) \
        __attribute__((__format__(__printf__, (string_index), (first_to_check))))
#else
# define CHECK_PRINTF_FORMAT(string_index, first_to_check)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 ****************************** Object Declarations ***************************
 ******************************************************************************/

typedef struct _gckOS                  *gckOS;
typedef struct _gcoHAL                 *gcoHAL;
typedef struct _gcoOS                  *gcoOS;
typedef struct _gco2D                  *gco2D;
typedef struct gcsATOM                 *gcsATOM_PTR;

typedef struct _gco3D                  *gco3D;
typedef struct _gcoCL                  *gcoCL;
typedef struct _gcoVX                  *gcoVX;
typedef struct _gcsFAST_FLUSH          *gcsFAST_FLUSH_PTR;

typedef struct _gcoSURF                *gcoSURF;
typedef struct _gcsSURF_NODE           *gcsSURF_NODE_PTR;
typedef struct _gcsSURF_FORMAT_INFO    *gcsSURF_FORMAT_INFO_PTR;
typedef struct _gcsPOINT               *gcsPOINT_PTR;
typedef struct _gcsSIZE                *gcsSIZE_PTR;
typedef struct _gcsRECT                *gcsRECT_PTR;
typedef struct _gcsBOUNDARY            *gcsBOUNDARY_PTR;
typedef struct _gcoHARDWARE            *gcoHARDWARE;
typedef struct _gcoDEVICE              *gcoDEVICE;
#if gcdENABLE_MULTI_DEVICE_MANAGEMENT
typedef struct _gcsHARDWARE_PROFILE     gcsHARDWARE_PROFILE;
#endif
typedef union  _gcuVIDMEM_NODE         *gcuVIDMEM_NODE_PTR;
typedef struct _gcsVIDMEM_NODE         *gckVIDMEM_NODE;
typedef struct _gcsVIDMEM_BLOCK        *gckVIDMEM_BLOCK;

typedef void                           *gcoVG;

typedef struct _gcoFENCE               *gcoFENCE;
typedef struct _gcsSYNC_CONTEXT        *gcsSYNC_CONTEXT_PTR;

typedef struct _gcsUSER_MEMORY_DESC    *gcsUSER_MEMORY_DESC_PTR;

#if gcdENABLE_CLEAR_FENCE
typedef struct _gcsUSER_FENCE_INFO *gcsUSER_FENCE_INFO_PTR;
#endif

/* Immuatable features from database */
typedef struct _gcsNN_FIXED_FEATURE {
    gctUINT  hwChipVersion;
    gctUINT  vipCoreCount;
    gctUINT  vipRingCount;
    gctUINT  nnMadPerCore;
    gctUINT  nnInputBufferDepth;
    gctUINT  nnAccumBufferDepth;
    gctUINT  nnFCNonPrunAccel;
    gctUINT  nnInImageOffsetBits;
    gctUINT  tpCoreCount; /* full-function core count */
    gctUINT  tpPwlLUTCount;
    gctUINT  tpPwlLUTSize;
    gctUINT  vip7Version;
    gctUINT  vipBrickMode;
    gctUINT  tpReorderInImageSize;
    gctUINT  tpliteCoreCount; /* fc-only core count */
    gctUINT  nnFP16XYDPX;
    gctUINT  nnFP16XYDPY;
    gctUINT  nnFP16ZDP;
    gctUINT  zrlBits;
    gctUINT  uscCacheControllers;
    gctUINT  uscBanks;
    gctUINT  nnLanesPerOutCycle;
    gctUINT  maxOTNumber;
    gctUINT  physicalVipSramWidthInByte;
    gctUINT  equivalentVipsramWidthInByte;
    gctUINT  shaderCoreCount;
    gctUINT  axiBusWidth;
    gctUINT  nnMaxKXSize;
    gctUINT  nnMaxKYSize;
    gctUINT  nnMaxKZSize;
    gctUINT  nnClusterNumForPowerControl;
    gctUINT  vipMinAxiBurstSize;
    gctUINT  nnInLinesPerCycle;
    gctUINT  nnPreprocessorMaxSegmentPerCycle;

    /* stream processor info */
    gctUINT  streamProcessorExecCount;
    gctUINT  streamProcessorVectorSize;

    /* add related information for check in/out size */
    gctUINT  outImageXStrideBits;
    gctUINT  outImageYStrideBits;
    gctUINT  inImageXStrideBits;
    gctUINT  inImageYStrideBits;
    gctUINT  outImageXSizeBits;
    gctUINT  outImageYSizeBits;
    gctUINT  inImageXSizeBits;
    gctUINT  inImageYSizeBits;
    gctUINT  smallAccumBits;
    gctUINT  coefDecompressPerfX;
    gctUINT  decompVzGroupBits;
    gctUINT  nnCommandSize;
} gcsNN_FIXED_FEATURE;

/* Features can be customized from outside */
typedef struct _gcsNN_CUSTOMIZED_FEATURE {
    gctUINT  nnActiveCoreCount;
    gctUINT  nnCoreCount;         /* total nn core count */
    gctUINT  nnCoreCountInt8;     /* total nn core count supporting int8 */
    gctUINT  nnCoreCountInt16;    /* total nn core count supporting int16 */
    gctUINT  nnCoreCountUint16;    /* total nn core count supporting uint16 */
    gctUINT  nnCoreCountFloat16;  /* total nn core count supporting float16 */
    gctUINT  nnCoreCountBFloat16; /* total nn core count supporting Bfloat16 */
    gctUINT  vipSRAMSize;
    gctUINT  axiSRAMSize;
    gctFLOAT ddrReadBWLimit;
    gctFLOAT ddrWriteBWLimit;
    gctFLOAT ddrTotalBWLimit;
    gctFLOAT axiSramReadBWLimit;
    gctFLOAT axiSramWriteBWLimit;
    gctFLOAT axiSramTotalBWLimit;
    gctFLOAT axiBusReadBWLimit;
    gctFLOAT axiBusWriteBWLimit;
    gctFLOAT axiBusTotalBWLimit;
    gctUINT  vipSWTiling;
    gctFLOAT ddrLatency;
    gctUINT  freqInMHZ;
    gctUINT  axiClockFreqInMHZ;
    gctUINT  maxSocOTNumber; /*max SOC outstanding transfer number*/
    gctUINT  nnWriteWithoutUSC;
    gctUINT  depthWiseSupport;
    gctUINT  vipVectorPrune;
    gctUINT  ddrKernelBurstSize;
    gctFLOAT  axiSRAMLatency;
    gctUINT  latencyHidingAtFullAxiBw;
} gcsNN_CUSTOMIZED_FEATURE;

/* Features are unified (hardcoded) for hardwares */
typedef struct _gcsNN_UNIFIED_FEATURE {
    gctUINT  nnUSCCacheSize;
    gctUINT  nnCmdSizeInBytes;
    gctUINT  tpCmdSizeInBytes;
    gctUINT  vipCoefDecodePerf;
    gctUINT  vipCachedReadFromSram;
    gctUINT  vipImagePartialCache;
    gctUINT  lanesPerConv;
    gctUINT  maxTileSize;
    gctUINT  fullCacheKernelHeadFix : 1;
    gctUINT  conv1x1HalfPerformance : 1;
    gctUINT  per3DTileBubbleFix : 1;
    gctUINT  cacheLineModeDisabled : 1;
    gctUINT  tpReOrderFix : 1;
    gctUINT  zdp3NoCompressFix : 1;
    gctUINT  asyncCopyPerfFix : 1;
    gctUINT  accurateTileBW : 1;
    gctUINT  zxdp3KernelReadConflictFix : 1;
    gctUINT  axiSramSlowedDownByAddr : 1;
    gctUINT  slowNNReqArbitrationFix : 1;
    gctUINT  singlePortAccBuffer : 1;
    gctUINT  convOutFifoDepthFix : 1;
    gctUINT  smallBatchEnable : 1;
    gctUINT  axiSramOnlySWTiling : 1;
    gctUINT  imageNotPackedInSram : 1;
    gctUINT  coefDeltaCordOverFlowZRL8BitFix : 1;
    gctUINT  lowEfficiencyOfIDWriteImgBufFix : 1;
    gctUINT  xyOffsetLimitationFix : 1;
    gctUINT  kernelPerCoreLTOneThirdCoefFix : 1;
    gctUINT  diffConditionForCachelineModePreFix : 1;
} gcsNN_UNIFIED_FEATURE;

/* Features are derived from above ones */
typedef struct _gcsNN_DERIVIED_FEATURE {
    gctUINT  nnDPAmount;
    gctUINT  nnXYDPX;
    gctUINT  nnXYDPY;
    gctUINT  nnZDP;
    gctFLOAT totalLatency;
    gctFLOAT internalLatency;
    gctFLOAT ddrReadBWInBytePerCycle;
    gctFLOAT ddrWriteBWInBytePerCycle;
    gctFLOAT totalAxiSRAMLatency;
} gcsNN_DERIVED_FEATURE;

/******************************************************************************
 ********************* Share obj lock/unlock macros. **************************
 ******************************************************************************/
#define gcmLOCK_SHARE_OBJ(Obj)                                             \
    {                                                                      \
        if (Obj->sharedLock != gcvNULL) {                                  \
            (gcoOS_AcquireMutex(gcvNULL, Obj->sharedLock, gcvINFINITE));   \
        }                                                                  \
    }

#define gcmUNLOCK_SHARE_OBJ(Obj)                                           \
    {                                                                      \
        if (Obj->sharedLock != gcvNULL) {                                  \
            (gcoOS_ReleaseMutex(gcvNULL, Obj->sharedLock));                \
        }                                                                  \
    }

typedef struct _gcsSystemInfo {
    /* memory latency number for SH data fetch, in SH cycle*/
    gctUINT32 memoryLatencySH;
} gcsSystemInfo;

#define gcPLS_INITIALIZER                                    \
{                                                            \
    gcvNULL,          /* gcoOS object.      */               \
    gcvNULL,          /* gcoHAL object.     */               \
    0,                /* internalSize       */               \
    0,                /* internalPhysName   */               \
    gcvNULL,          /* internalLogical    */               \
    0,                /* externalSize       */               \
    0,                /* externalPhysName   */               \
    gcvNULL,          /* externalLogical    */               \
    0,                /* contiguousSize     */               \
    0,                /* contiguousPhysName */               \
    gcvNULL,          /* contiguousLogical  */               \
    gcvNULL,          /* eglDisplayInfo     */               \
    gcvNULL,          /* eglSurfaceInfo     */               \
    gcvSURF_A8R8G8B8, /* eglConfigFormat    */               \
    gcvNULL,          /* reference          */               \
    0,                /* processID          */               \
    0,                /* threadID           */               \
    gcvFALSE,         /* exiting            */               \
    gcvFALSE,         /* Special flag for NP2 texture. */    \
    gcvFALSE,         /* device open.       */               \
    {gcvNULL},        /* destructor         */               \
    gcvNULL,          /* accessLock         */               \
    gcvNULL,          /* GL FE compiler lock*/               \
    gcvNULL,          /* CL FE compiler lock*/               \
    gcvNULL,          /* VX context lock    */               \
    gcvPATCH_NOTINIT, /* global patchID     */               \
    gcvNULL,          /* global fenceID*/                    \
    gcvNULL,          /* mainThreadHandle */                 \
    gcvFALSE,         /* memory profile flag */              \
    gcvNULL,          /* profileLock;        */              \
    0,                /* allocCount;         */              \
    0,                /* allocSize;          */              \
    0,                /* maxAllocSize;       */              \
    0,                /* freeCount;          */              \
    0,                /* freeSize;           */              \
    0,                /* currentSize;        */              \
    0,                /* video_allocCount;   */              \
    0,                /* video_allocSize;    */              \
    0,                /* video_maxAllocSize; */              \
    0,                /* video_freeCount;    */              \
    0,                /* video_freeSize;     */              \
    0,                /* video_currentSize;  */              \
}

/******************************************************************************
 ******************************* Thread local storage *************************
 ******************************************************************************/

typedef struct _gcsDRIVER_TLS *gcsDRIVER_TLS_PTR;

typedef struct _gcsDRIVER_TLS {
    void (*destructor)(gcsDRIVER_TLS_PTR Tls);
} gcsDRIVER_TLS;

typedef struct _gcsTLS *gcsTLS_PTR;

typedef struct _gcsTLS {
    gceHARDWARE_TYPE            currentType;
    gceHARDWARE_TYPE            targetType;

    /* To which core device control is called,
     * it is index in a hardware type.
     */
    gctUINT32                   currentCoreIndex;

    /* Current device index of this thread. */
    gctUINT32                   currentDevIndex;

    /* Current 3D hardwre of this thread */
    gcoHARDWARE                 currentHardware;

    /* Default 3D hardware of this thread */
    gcoHARDWARE                 defaultHardware;

    /* Only for separated 3D and 2D */
    gcoHARDWARE                 hardware2D;

#if gcdENABLE_3D
    gco3D                       engine3D;
#endif
    gcoVX                       engineVX;

    gctBOOL                     copied;

    /* libGAL.so handle */
    gctHANDLE                   handle;

    gctHANDLE                   graph;

    /* If true, do not releas 2d engine and hardware in hal layer */
    gctBOOL                     release2DUpper;

    /* Driver tls. */
    gcsDRIVER_TLS_PTR           driverTLS[gcvTLS_KEY_COUNT];

    gctINT32                    cmd_buf_size;

#if gcdENABLE_SW_PREEMPTION
    /* PriorityID. */
    gctUINT                     priorityID;
#endif
} gcsTLS;

typedef struct _gcsSURF_VIEW {
    gcoSURF surf;
    gctUINT firstSlice;
    gctUINT numSlices;
} gcsSURF_VIEW;

/* gcsHAL_Limits*/
typedef struct _gcsHAL_LIMITS {
    /* chip info */
    gceCHIPMODEL    chipModel;
    gctUINT32       chipRevision;
    gctUINT32       featureCount;
    gctUINT32      *chipFeatures;

    /* target caps */
    gctUINT32       maxWidth;
    gctUINT32       maxHeight;
    gctUINT32       multiTargetCount;
    gctUINT32       maxSamples;

} gcsHAL_LIMITS;

typedef struct _gcsHAL_CHIPIDENTITY {
    gceCHIPMODEL    chipModel;
    gctUINT32       chipRevision;
    gctUINT32       productID;
    gctUINT32       customerID;
    gctUINT32       ecoID;
    gceCHIP_FLAG    chipFlags;
    gctUINT64       platformFlagBits;
} gcsHAL_CHIPIDENTITY;

/******************************************************************************
 ******************************** gcoHAL Object *******************************
 ******************************************************************************/

/* Construct a new gcoHAL object. */
gceSTATUS
gcoHAL_ConstructEx(IN gctPOINTER Context, IN gcoOS Os, OUT gcoHAL *Hal);

/* Destroy an gcoHAL object. */
gceSTATUS
gcoHAL_DestroyEx(IN gcoHAL Hal);

/* Empty function for compatibility. */
gceSTATUS
gcoHAL_Construct(IN gctPOINTER Context, IN gcoOS Os, OUT gcoHAL *Hal);

/* Empty function for compatibility. */
gceSTATUS
gcoHAL_Destroy(IN gcoHAL Hal);

/* Get HAL options */
gceSTATUS
gcoHAL_GetOption(IN gcoHAL Hal, IN gceOPTION Option);

gceSTATUS
gcoHAL_FrameInfoOps(IN gcoHAL Hal,
                    IN gceFRAMEINFO FrameInfo,
                    IN gceFRAMEINFO_OP Op,
                    IN OUT gctUINT *Val);

/* Set HAL options */
gceSTATUS
gcoHAL_SetOption(IN gcoHAL Hal, IN gceOPTION Option, IN gctBOOL Value);

gceSTATUS
gcoHAL_GetHardware(IN gcoHAL Hal, OUT gcoHARDWARE *Hw);

#if gcdENABLE_3D
gceSTATUS
gcoHAL_GetSpecialHintData(IN gcoHAL Hal, OUT gctINT *Hint);
/*
 ** Deprecated(Don't use it), keep it here for external library(libgcu.so)
 */
gceSTATUS
gcoHAL_Get3DEngine(IN gcoHAL Hal, OUT gco3D *Engine);
#endif /* gcdENABLE_3D */

gceSTATUS
gcoHAL_GetProductName(IN gcoHAL Hal,
                      OUT gctSTRING *ProductName,
                      OUT gctUINT *PID );
gceSTATUS
gcoHAL_GetProductNameWithHardware(IN gcoHARDWARE Hardware,
                                  OUT gctSTRING *ProductName,
                                  OUT gctUINT *PID);

gceSTATUS
gcoHAL_SetFscaleValue(IN gcoHAL Hal,
                      IN gctUINT CoreIndex,
                      IN gctUINT FscaleValue,
                      IN gctUINT ShaderFscaleValue);

gceSTATUS
gcoHAL_CancelJob(gcoHAL Hal);

gceSTATUS
gcoHAL_GetFscaleValue(OUT gctUINT *FscaleValue,
                      OUT gctUINT *MinFscaleValue,
                      OUT gctUINT *MaxFscaleValue);

gceSTATUS
gcoHAL_SetBltNP2Texture(gctBOOL enable);

gceSTATUS
gcoHAL_ExportVideoMemory(IN gctUINT32 Handle,
                         IN gctUINT32 Flags,
                         OUT gctINT32 *FD);

gceSTATUS
gcoHAL_NameVideoMemory(IN gctUINT32 Handle, OUT gctUINT32 *Name);

gceSTATUS
gcoHAL_ImportVideoMemory(IN gctUINT32 Name, OUT gctUINT32 *Handle);

gceSTATUS
gcoHAL_GetVideoMemoryFd(IN gctUINT32 Handle, OUT gctINT *Fd);

gceSTATUS
gcoHAL_GetExportedVideoMemoryFd(IN gctUINT32 Handle, OUT gctINT *Fd);

/* Verify whether the specified feature is available in hardware. */
gceSTATUS
gcoHAL_IsFeatureAvailable(IN gcoHAL Hal, IN gceFEATURE Feature);

gceSTATUS
gcoHAL_IsFeatureAvailableWithHardware(IN gcoHARDWARE Hardware, IN gceFEATURE Feature);

gceSTATUS
gcoHAL_IsFeatureAvailable1(IN gcoHAL Hal, IN gceFEATURE Feature);

/* Query the identity of the hardware. */
gceSTATUS
gcoHAL_QueryChipIdentity(IN gcoHAL Hal,
                         OUT gceCHIPMODEL *ChipModel,
                         OUT gctUINT32 *ChipRevision,
                         OUT gctUINT32 *ChipFeatures,
                         OUT gctUINT32 *ChipMinorFeatures);

gceSTATUS
gcoHAL_QueryChipIdentityWithHardware(IN gcoHARDWARE Hardware,
                                     OUT gceCHIPMODEL *ChipModel,
                                     OUT gctUINT32 *ChipRevision);

gceSTATUS
gcoHAL_QueryChipIdentityEx(IN gcoHAL Hal,
                           IN gctUINT32 SizeOfParam,
                           OUT gcsHAL_CHIPIDENTITY *ChipIdentity);

gceSTATUS
gcoHAL_QuerySuperTileMode(OUT gctUINT32_PTR SuperTileMode);

gceSTATUS
gcoHAL_QueryChipAxiBusWidth(OUT gctBOOL *AXI128Bits);

gceSTATUS
gcoHAL_QueryMultiGPUAffinityConfig(IN gceHARDWARE_TYPE Type,
                                   OUT gceMULTI_PROCESSOR_MODE *Mode,
                                   OUT gctUINT32_PTR CoreIndex);

gceSTATUS
gcoHAL_QueryHwDeviceIdByEnv(IN gcoHAL Hal,
                            OUT gctUINT32 *DeviceID,
                            OUT gctBOOL *HasEnv);

gceSTATUS
gcoHAL_QuerySRAM(IN gcoHAL Hal,
                 IN gcePOOL Type,
                 OUT gctUINT32 *Size,
                 OUT gctADDRESS *GPUVirtAddr,
                 OUT gctPHYS_ADDR_T *GPUPhysAddr,
                 OUT gctUINT32 *GPUPhysName,
                 OUT gctPHYS_ADDR_T *CPUPhysAddr);

#ifdef LINUX
gctINT32
gcoOS_EndRecordAllocation(void);
void
gcoOS_RecordAllocation(void);
void
gcoOS_AddRecordAllocation(gctSIZE_T Size);
#endif

/* Query the amount of video memory. */
gceSTATUS
gcoHAL_QueryVideoMemory(IN gcoHAL Hal,
                        OUT gctUINT32 *InternalPhysName,
                        OUT gctSIZE_T *InternalSize,
                        OUT gctUINT32 *ExternalPhysName,
                        OUT gctSIZE_T *ExternalSize,
                        OUT gctUINT32 *ContiguousPhysName,
                        OUT gctSIZE_T *ContiguousSize);

/* Map video memory. */
gceSTATUS
gcoHAL_MapMemory(IN gcoHAL Hal, IN gctUINT32 PhysName,
                 IN gctSIZE_T NumberOfBytes, OUT gctPOINTER *Logical);

/* Unmap video memory. */
gceSTATUS
gcoHAL_UnmapMemory(IN gcoHAL Hal, IN gctUINT32 PhysName,
                   IN gctSIZE_T NumberOfBytes, IN gctPOINTER Logical);

/* Schedule an unmap of a buffer mapped through its physical address. */
gceSTATUS
gcoHAL_ScheduleUnmapMemory(IN gcoHAL Hal, IN gctUINT32 PhysName,
                           IN gctSIZE_T NumberOfBytes, IN gctPOINTER Logical);

/* Allocate video memory. */
gceSTATUS
gcoOS_AllocateVideoMemory(IN gcoOS Os,
                          IN gctBOOL InUserSpace,
                          IN gctBOOL InCacheable,
                          IN OUT gctSIZE_T *Bytes,
                          OUT gctUINT32 *Address,
                          OUT gctPOINTER *Logical,
                          OUT gctPOINTER *Handle);

/* Free video memory. */
gceSTATUS
gcoOS_FreeVideoMemory(IN gcoOS Os, IN gctPOINTER Handle);

/* Lock video memory. */
gceSTATUS
gcoOS_LockVideoMemory(IN gcoOS Os,
                      IN gctPOINTER Handle,
                      IN gctBOOL InUserSpace,
                      IN gctBOOL InCacheable,
                      OUT gctUINT32 *Address,
                      OUT gctPOINTER *Logical);

/* Commit the current command buffer. */
gceSTATUS
gcoHAL_Commit(IN gcoHAL Hal, IN gctBOOL Stall);

/* Do SW reset by kernel. */
gceSTATUS
gcoHAL_Reset(IN gcoHAL Hal);

#if gcdENABLE_3D
/* Sencd fence command. */
gceSTATUS
gcoHAL_SendFence(IN gcoHAL Hal);

/* Send fence command for GL_TIME_ELAPSED. */
gceSTATUS
gcoHAL_TimeQuery_SendFence(IN gcoHAL Hal, IN gctADDRESS physical);
/* Wait fence result for GL_TIME_ELAPSED. */
gceSTATUS
gcoHAL_TimeQuery_WaitFence(IN gcoHAL Hal,
                           IN gcsSURF_NODE_PTR node,
                           IN gctPOINTER nodeHeaderLocked,
                           IN gctPOINTER logical);
#endif /* gcdENABLE_3D */

/* Query the tile capabilities. */
gceSTATUS
gcoHAL_QueryTiled(IN gcoHAL Hal,
                  OUT gctINT32 *TileWidth2D,
                  OUT gctINT32 *TileHeight2D,
                  OUT gctINT32 *TileWidth3D,
                  OUT gctINT32 *TileHeight3D);

gceSTATUS
gcoHAL_Compact(IN gcoHAL Hal);

#if PHYTIUM_PROFILER_SYSTEM_MEMORY 
gceSTATUS
gcoHAL_ProfileStart(IN gcoHAL Hal);

gceSTATUS
gcoHAL_ProfileEnd(IN gcoHAL Hal, IN gctCONST_STRING Title);
#endif

/* Power Management */
gceSTATUS
gcoHAL_SetPowerManagementState(IN gcoHAL Hal,
                               IN gceCHIPPOWERSTATE State);

gceSTATUS
gcoHAL_QueryPowerManagementState(IN gcoHAL Hal,
                                 OUT gceCHIPPOWERSTATE *State);

/* Set the filter type for filter blit. */
gceSTATUS
gcoHAL_SetFilterType(IN gcoHAL Hal,
                     IN gceFILTER_TYPE FilterType);

/* Call the kernel HAL layer. */
gceSTATUS
gcoHAL_Call(IN gcoHAL Hal,
            IN OUT gcsHAL_INTERFACE_PTR Interface);

/* Schedule an event. */
gceSTATUS
gcoHAL_ScheduleEvent(IN gcoHAL Hal,
                     IN OUT gcsHAL_INTERFACE_PTR Interface);

/* Request a start/stop timestamp. */
gceSTATUS
gcoHAL_SetTimer(IN gcoHAL Hal, IN gctUINT32 Index, IN gctBOOL Start);

/* Get Time delta from a Timer in microseconds. */
gceSTATUS
gcoHAL_GetTimerTime(IN gcoHAL Hal, IN gctUINT32 Timer,
                    OUT gctINT32_PTR TimeDelta);

/* set timeout value. */
gceSTATUS
gcoHAL_SetTimeOut(IN gcoHAL Hal, IN gctUINT32 timeOut);

gceSTATUS
gcoHAL_SetHardwareType(IN gcoHAL Hal,
                       IN gceHARDWARE_TYPE HardwardType);

gceSTATUS
gcoHAL_GetHardwareType(IN gcoHAL Hal,
                       OUT gceHARDWARE_TYPE *HardwardType);

gceSTATUS
gcoHAL_QueryChipCount(IN gcoHAL Hal, OUT gctINT32 *Count);

gceSTATUS
gcoHAL_Query3DCoreCount(IN gcoHAL Hal, OUT gctUINT32 *Count);

gceSTATUS
gcoHAL_Query2DCoreCount(IN gcoHAL Hal, OUT gctUINT32 *Count);

gceSTATUS
gcoHAL_QueryCluster(IN gcoHAL Hal,
                    OUT gctINT32 *ClusterMinID,
                    OUT gctINT32 *ClusterMaxID,
                    OUT gctUINT32 *ClusterCount,
                    OUT gctUINT32 *ClusterIDWidth);

gceSTATUS
gcoHAL_QueryUscAttribCacheRatio(IN gcoHAL Hal,
                                OUT gctUINT32 *UscAttribCacheRatio);

gceSTATUS
gcoHAL_QuerySmallBatch(IN gcoHAL Hal, OUT gctBOOL *SmallBatch);

gceSTATUS
gcoHAL_QueryResetMsaaProduct(IN gcoHAL Hal, IN gctBOOL bReset, OUT gctUINT32* msaaProduct);

gceSTATUS
gcoHAL_QueryCoreCount(IN gcoHAL Hal,
                      IN gceHARDWARE_TYPE Type,
                      OUT gctUINT *Count,
                      OUT gctUINT_PTR ChipIDs);

gceSTATUS
gcoHAL_QuerySeparated2D(IN gcoHAL Hal);

gceSTATUS
gcoHAL_QueryHybrid2D(IN gcoHAL Hal);

gceSTATUS
gcoHAL_Is3DAvailable(IN gcoHAL Hal);

/* Get pointer to gcoVG object. */
gceSTATUS
gcoHAL_GetVGEngine(IN gcoHAL Hal, OUT gcoVG *Engine);

gceSTATUS
gcoHAL_QueryChipLimits(IN gcoHAL Hal,
                       IN gctINT32 Chip,
                       OUT gcsHAL_LIMITS *Limits);

gceSTATUS
gcoHAL_QueryChipFeature(IN gcoHAL Hal, IN gctINT32 Chip, IN gceFEATURE Feature);

gceSTATUS
gcoHAL_SetDeviceIndex(IN gcoHAL Hal, IN gctUINT32 DeviceIndex);

gceSTATUS
gcoHAL_GetCurrentDeviceIndex(IN gcoHAL Hal, OUT gctUINT32 *DeviceIndex);

gceSTATUS
gcoHAL_SetCoreIndex(IN gcoHAL Hal, IN gctUINT32 Core);

gceSTATUS
gcoHAL_GetCurrentCoreIndex(IN gcoHAL Hal, OUT gctUINT32 *Core);

gceSTATUS
gcoHAL_InitCoreIndexByType(IN gcoHAL Hal,
                           IN gceHARDWARE_TYPE Type,
                           IN gctBOOL Init,
                           OUT gctUINT32 *CoreIndex);

gceSTATUS
gcoHAL_ConvertCoreIndexGlobal(IN gcoHAL Hal,
                              IN gceHARDWARE_TYPE Type,
                              IN gctUINT32 CoreCount,
                              IN gctUINT32 *LocalCoreIndexs,
                              OUT gctUINT32 *GlobalCoreIndexs);

gceSTATUS
gcoHAL_ConvertCoreIndexLocal(IN gcoHAL Hal,
                             IN gceHARDWARE_TYPE Type,
                             IN gctUINT32 CoreCount,
                             IN gctUINT32 *GlobalCoreIndexs,
                             OUT gctUINT32 *LocalCoreIndexs);

gceSTATUS
gcoHAL_SelectChannel(IN gcoHAL Hal, IN gctBOOL Priority, IN gctUINT32 ChannelId);

gceSTATUS
gcoHAL_MCFESemaphore(IN gctUINT32 SemaHandle, IN gctBOOL SendSema);

gceSTATUS
gcoHAL_AllocateMCFESemaphore(OUT gctUINT32 *SemaHandle);

gceSTATUS
gcoHAL_FreeMCFESemaphore(IN gctUINT32 SemaHandle);

/*----------------------------------------------------------------------------*/
/*----- Shared Buffer --------------------------------------------------------*/

/* Create shared buffer. */
gceSTATUS
gcoHAL_CreateShBuffer(IN gctUINT32 Size, OUT gctSHBUF *ShBuf);

/* Destroy shared buffer. */
gceSTATUS
gcoHAL_DestroyShBuffer(IN gctSHBUF ShBuf);

/* Map shared buffer to current process. */
gceSTATUS
gcoHAL_MapShBuffer(IN gctSHBUF ShBuf);

/* Write user data to shared buffer. */
gceSTATUS
gcoHAL_WriteShBuffer(IN gctSHBUF ShBuf, IN gctCONST_POINTER Data, IN gctUINT32 ByteCount);

/* Read user data from shared buffer. */
gceSTATUS
gcoHAL_ReadShBuffer(IN gctSHBUF ShBuf,
                    IN gctPOINTER Data,
                    IN gctUINT32 BytesCount,
                    OUT gctUINT32 *BytesRead);

/* Config power management to be enabled or disabled. */
gceSTATUS
gcoHAL_ConfigPowerManagement(IN gctBOOL Enable, OUT gctBOOL *OldValue);

gceSTATUS
gcoHAL_QueryAndConfigCluster(IN gctUINT32 ClusterMask, OUT gctUINT32 *CurValue);

gceSTATUS
gcoHAL_AllocateVideoMemory(IN gctUINT Alignment,
                           IN gceVIDMEM_TYPE Type,
                           IN gctUINT32 Flag,
                           IN OUT gcePOOL *Pool,
                           IN OUT gctSIZE_T *Bytes,
                           OUT gctUINT32_PTR Node);

gceSTATUS
gcoHAL_LockVideoMemory(IN gctUINT32 Node,
                       IN gctBOOL Cacheable,
                       IN gceENGINE engine,
                       OUT gctADDRESS *Address,
                       OUT gctPOINTER *Logical);

gceSTATUS
gcoHAL_LockVideoMemoryEx(IN gctUINT32 Node,
                         IN gctBOOL Cacheable,
                         IN gceENGINE engine,
                         IN gceLOCK_VIDEO_MEMORY_OP Op,
                         OUT gctADDRESS *Address,
                         OUT gctPOINTER *Logical);

gceSTATUS
gcoHAL_UnlockVideoMemory(IN gctUINT32 Node, IN gceVIDMEM_TYPE Type, IN gceENGINE engine);

gceSTATUS
gcoHAL_UnlockVideoMemoryEX(IN gctUINT32 Node,
                           IN gceVIDMEM_TYPE Type,
                           IN gceENGINE Engine,
                           IN gctBOOL Sync,
                           IN gceLOCK_VIDEO_MEMORY_OP Op);

gceSTATUS
gcoHAL_ReleaseVideoMemory(IN gctUINT32 Node);

#if gcdENABLE_3D
/* Query the target capabilities. */
gceSTATUS
gcoHAL_QueryTargetCaps(IN gcoHAL Hal,
                       OUT gctUINT *MaxWidth,
                       OUT gctUINT *MaxHeight,
                       OUT gctUINT *MultiTargetCount,
                       OUT gctUINT *MaxSamples);
#endif

gceSTATUS
gcoHAL_PrepareVideoMemory(IN gctUINT32 Node);

gceSTATUS
gcoHAL_FinishVideoMemory(IN gctUINT32 Node);

gceSTATUS
gcoHAL_WrapUserMemory(IN gcsUSER_MEMORY_DESC_PTR UserMemoryDesc,
                      IN gceVIDMEM_TYPE Type,
                      OUT gctUINT32_PTR Node);

gceSTATUS
gcoHAL_WrapUserMemoryEx(IN gcsUSER_MEMORY_DESC_PTR UserMemoryDesc,
                        IN gceVIDMEM_TYPE Type,
                        OUT gctUINT32_PTR Node,
                        OUT gctUINT64_PTR Size);

gceSTATUS
gcoHAL_QueryResetTimeStamp(OUT gctUINT64_PTR ResetTimeStamp,
                           OUT gctUINT64_PTR ContextID);

gceSTATUS
gcoHAL_WaitFence(IN gctUINT32 Handle, IN gctUINT32 TimeOut);

gceSTATUS
gcoHAL_ScheduleSignal(IN gctSIGNAL Signal,
                      IN gctSIGNAL AuxSignal,
                      IN gctINT ProcessID,
                      IN gceKERNEL_WHERE FromWhere);

gceSTATUS
gcoHAL_GetGraphicBufferFd(IN gctUINT32 Node[3],
                          IN gctSHBUF ShBuf,
                          IN gctSIGNAL Signal,
                          OUT gctINT32 *Fd);

gceSTATUS
gcoHAL_AlignToTile(IN OUT gctUINT32 *Width,
                   IN OUT gctUINT32 *Height,
                   IN gceSURF_TYPE Type,
                   IN gceSURF_FORMAT Format);

gceSTATUS
gcoHAL_GetLastCommitStatus(IN gcoHAL Hal, OUT gctBOOL *Pending);

gceSTATUS
gcoHAL_SetLastCommitStatus(IN gcoHAL Hal, IN gctBOOL Pending);

gceSTATUS
gcoHAL_CommitDone(IN gcoHAL Hal);

gceSTATUS
gcoHAL_IsFlatMapped(IN gctPHYS_ADDR_T PhysicalAddress,
                    OUT gctADDRESS *Address);

gceSTATUS
gcoHAL_QueryMCFESemaphoreCapacity(IN gcoHAL Hal,
                                  OUT gctUINT32 *Capacity);

#if gcdENABLE_MP_SWITCH
gceSTATUS
gcoHAL_SwitchMpMode(gcoHAL Hal);
#endif

gceSTATUS
gcoHAL_CommandBufferAutoCommit(gcoHAL Hal, gctBOOL AutoCommit);

gceSTATUS
gcoHAL_CommandBufferAutoSync(gcoHAL Hal, gctBOOL AutoSync);


#if gcdENABLE_MULTI_DEVICE_MANAGEMENT
gceSTATUS
gcoHAL_SwitchContext(IN gcoHAL Hal,
                     IN gcoHARDWARE Hardware,
                     OUT gcoHARDWARE *SavedHardware,
                     OUT gceHARDWARE_TYPE *SavedType,
                     OUT gctUINT32 *SavedHwDeviceIndex,
                     OUT gctUINT32 *SavedCoreIndex);

gceSTATUS
gcoHAL_RestoreContext(IN gcoHAL Hal,
                      IN gcoHARDWARE Hardware,
                      IN gceHARDWARE_TYPE Type,
                      IN gctUINT32 HwDeviceIndex,
                      IN gctUINT32 CoreIndex);

gceSTATUS
gcoHAL_ShowDeviceInfo(gcoHAL Hal);
#endif

gceSTATUS
gcoHAL_GetHwContext(
    IN gcoHARDWARE Hardware,
    OUT gctUINT32* Context);

/******************************************************************************
 ********************************** gcoOS Object ******************************
 ******************************************************************************/
/* Lock PLS access */
gceSTATUS
gcoOS_LockPLS(void);

/* Unlock PLS access */
gceSTATUS
gcoOS_UnLockPLS(void);

/* Get PLS value for given key */
gctPOINTER
gcoOS_GetPLSValue(IN gcePLS_VALUE key);

/* Set PLS value of a given key */
void
gcoOS_SetPLSValue(IN gcePLS_VALUE key, OUT gctPOINTER value);

/* Lock GL FE compiler access */
gceSTATUS
gcoOS_LockGLFECompiler(void);

/* Unlock GL FE compiler access */
gceSTATUS
gcoOS_UnLockGLFECompiler(void);

/* Lock CL FE compiler access */
gceSTATUS
gcoOS_LockCLFECompiler(void);

/* Unlock CL FE compiler access */
gceSTATUS
gcoOS_UnLockCLFECompiler(void);

gceSTATUS
gcoOS_GetTLS(OUT gcsTLS_PTR *TLS);

/* Copy the TLS from a source thread. */
gceSTATUS
gcoOS_CopyTLS(IN gcsTLS_PTR Source);

/* Query the thread local storage. */
gceSTATUS
gcoOS_QueryTLS(OUT gcsTLS_PTR *TLS);

/* Get access to driver tls. */
gceSTATUS
gcoOS_GetDriverTLS(IN gceTLS_KEY Key,
                   OUT gcsDRIVER_TLS_PTR *TLS);

/*
 * Set driver tls.
 * May cause memory leak if 'destructor' not set.
 */
gceSTATUS
gcoOS_SetDriverTLS(IN gceTLS_KEY Key, IN gcsDRIVER_TLS *TLS);

/* Destroy the objects associated with the current thread. */
void
gcoOS_FreeThreadData(void);

/* Empty function for compatibility. */
gceSTATUS
gcoOS_Construct(IN gctPOINTER Context, OUT gcoOS *Os);

/* Empty function for compatibility. */
gceSTATUS
gcoOS_Destroy(IN gcoOS Os);

/* Deprecated API: please use gcoHAL_GetBaseAddr() instead.
 **                This API was kept only for legacy BSP usage.
 **
 ** Get the base address for the physical memory.
 */
gceSTATUS
gcoOS_GetBaseAddress(IN gcoOS Os, OUT gctUINT32_PTR BaseAddress);

/* Allocate memory from the heap. */
gceSTATUS
gcoOS_Allocate(IN gcoOS Os, IN gctSIZE_T Bytes,
               OUT gctPOINTER *Memory);

gceSTATUS
gcoOS_Realloc(IN gcoOS Os,
              IN gctSIZE_T Bytes,
              IN gctSIZE_T OrgBytes,
              OUT gctPOINTER *Memory);

/* Get allocated memory size. */
gceSTATUS
gcoOS_GetMemorySize(IN gcoOS Os, IN gctPOINTER Memory,
                    OUT gctSIZE_T_PTR MemorySize);

/* Free allocated memory. */
gceSTATUS
gcoOS_Free(IN gcoOS Os, IN gctPOINTER Memory);

/* Allocate memory. */
gceSTATUS
gcoOS_AllocateSharedMemory(IN gcoOS Os, IN gctSIZE_T Bytes,
                           OUT gctPOINTER *Memory);

/* Free memory. */
gceSTATUS
gcoOS_FreeSharedMemory(IN gcoOS Os, IN gctPOINTER Memory);

/* Allocate memory. */
gceSTATUS
gcoOS_AllocateMemory(IN gcoOS Os, IN gctSIZE_T Bytes,
                     OUT gctPOINTER *Memory);

/* Realloc memory. */
gceSTATUS
gcoOS_ReallocMemory(IN gcoOS Os, IN gctSIZE_T Bytes,
                    IN gctSIZE_T OrgBytes, OUT gctPOINTER *Memory);

/* Free memory. */
gceSTATUS
gcoOS_FreeMemory(IN gcoOS Os, IN gctPOINTER Memory);

/* Device I/O Control call to the kernel HAL layer. */
gceSTATUS
gcoOS_DeviceControl(IN gcoOS Os,
                    IN gctUINT32 IoControlCode,
                    IN gctPOINTER InputBuffer,
                    IN gctSIZE_T InputBufferSize,
                    IN gctPOINTER OutputBuffer,
                    IN gctSIZE_T OutputBufferSize);

#define gcmOS_SAFE_FREE(os, mem) \
    gcoOS_Free(os, mem); \
    mem = gcvNULL

#define gcmOS_SAFE_FREE_SHARED_MEMORY(os, mem) \
    gcoOS_FreeSharedMemory(os, mem); \
    mem = gcvNULL

#define gcmkOS_SAFE_FREE(os, mem) \
    gckOS_Free(os, mem); \
    mem = gcvNULL

#define gcdMAX_PATH 512

#define gcdMAX_ARGUMENT_SIZE 1024
#define gcdMAX_ARGUMENT_COUNT 64

/* Open a file. */
gceSTATUS
gcoOS_Open(IN gcoOS Os,
           IN gctCONST_STRING FileName,
           IN gceFILE_MODE Mode,
           OUT gctFILE *File);

/* Close a file. */
gceSTATUS
gcoOS_Close(IN gcoOS Os, IN gctFILE File);

/* Remove a file. */
gceSTATUS
gcoOS_Remove(IN gcoOS Os, IN gctCONST_STRING FileName);

/* Read data from a file. */
gceSTATUS
gcoOS_Read(IN gcoOS Os,
           IN gctFILE File,
           IN gctSIZE_T ByteCount,
           IN gctPOINTER Data,
           OUT gctSIZE_T *ByteRead);

/* Write data to a file. */
gceSTATUS
gcoOS_Write(IN gcoOS Os,
            IN gctFILE File,
            IN gctSIZE_T ByteCount,
            IN gctCONST_POINTER Data);

/* Flush data to a file. */
gceSTATUS
gcoOS_Flush(IN gcoOS Os, IN gctFILE File);

/* Close a file descriptor. */
gceSTATUS
gcoOS_CloseFD(IN gcoOS Os, IN gctINT FD);

/* Scan a file. */
gceSTATUS
gcoOS_FscanfI(IN gcoOS Os,
              IN gctFILE File,
              IN gctCONST_STRING Format,
              OUT gctUINT *result);

/* Dup file descriptor to another. */
gceSTATUS
gcoOS_DupFD(IN gcoOS Os,
            IN gctINT FD,
            OUT gctINT *FD2);

/* Lock a file. */
gceSTATUS
gcoOS_LockFile(IN gcoOS Os,
               IN gctFILE File,
               IN gctBOOL Shared,
               IN gctBOOL Block);

/* Unlock a file. */
gceSTATUS
gcoOS_UnlockFile(IN gcoOS Os, IN gctFILE File);

/* Create an endpoint for communication. */
gceSTATUS
gcoOS_Socket(IN gcoOS Os,
             IN gctINT Domain,
             IN gctINT Type,
             IN gctINT Protocol,
             OUT gctINT *SockFd);

/* Close a socket. */
gceSTATUS
gcoOS_CloseSocket(IN gcoOS Os, IN gctINT SockFd);

/* Initiate a connection on a socket. */
gceSTATUS
gcoOS_Connect(IN gcoOS Os,
              IN gctINT SockFd,
              IN gctCONST_POINTER HostName,
              IN gctUINT Port);

/* Shut down part of connection on a socket. */
gceSTATUS
gcoOS_Shutdown(IN gcoOS Os, IN gctINT SockFd, IN gctINT How);

/* Send a message on a socket. */
gceSTATUS
gcoOS_Send(IN gcoOS Os,
           IN gctINT SockFd,
           IN gctSIZE_T ByteCount,
           IN gctCONST_POINTER Data,
           IN gctINT Flags);

/* Initiate a connection on a socket. */
gceSTATUS
gcoOS_WaitForSend(IN gcoOS Os, IN gctINT SockFd,
                  IN gctINT Seconds, IN gctINT MicroSeconds);

/* Get environment variable value. */
gceSTATUS
gcoOS_GetEnv(IN gcoOS Os, IN gctCONST_STRING VarName, OUT gctSTRING *Value);

/* Set environment variable value. */
gceSTATUS
gcoOS_SetEnv(IN gcoOS Os, IN gctCONST_STRING VarName, IN gctSTRING Value);

/* Get current working directory. */
gceSTATUS
gcoOS_GetCwd(IN gcoOS Os, IN gctINT SizeInBytes, OUT gctSTRING Buffer);

/* Get file status info. */
gceSTATUS
gcoOS_Stat(IN gcoOS Os, IN gctCONST_STRING FileName, OUT gctPOINTER Buffer);

/* Set the current position of a file. */
gceSTATUS
gcoOS_Seek(IN gcoOS Os, IN gctFILE File, IN gctUINT32 Offset, IN gceFILE_WHENCE Whence);

/* Set the current position of a file. */
gceSTATUS
gcoOS_SetPos(IN gcoOS Os, IN gctFILE File, IN gctUINT32 Position);

/* Get the current position of a file. */
gceSTATUS
gcoOS_GetPos(IN gcoOS Os, IN gctFILE File, OUT gctUINT32 *Position);

/* Same as strstr. */
gceSTATUS
gcoOS_StrStr(IN gctCONST_STRING String,
             IN gctCONST_STRING SubString,
             OUT gctSTRING *Output);

/* Find the last occurrence of a character inside a string. */
gceSTATUS
gcoOS_StrFindReverse(IN gctCONST_STRING String,
                     IN gctINT8 Character,
                     OUT gctSTRING *Output);

gceSTATUS
gcoOS_StrDup(IN gcoOS Os, IN gctCONST_STRING String, OUT gctSTRING *Target);

/* Copy a string. */
gceSTATUS
gcoOS_StrCopySafe(IN gctSTRING Destination,
                  IN gctSIZE_T DestinationSize,
                  IN gctCONST_STRING Source);

/* Append a string. */
gceSTATUS
gcoOS_StrCatSafe(IN gctSTRING Destination,
                 IN gctSIZE_T DestinationSize,
                 IN gctCONST_STRING Source);

/* Compare two strings. */
gceSTATUS
gcoOS_StrCmp(IN gctCONST_STRING String1, IN gctCONST_STRING String2);

/* Compare characters of two strings. */
gceSTATUS
gcoOS_StrNCmp(IN gctCONST_STRING String1,
              IN gctCONST_STRING String2,
              IN gctSIZE_T Count);

/* Convert string to float. */
gceSTATUS
gcoOS_StrToFloat(IN gctCONST_STRING String, OUT gctFLOAT *Float);

/* Convert string to double. */
gceSTATUS
gcoOS_StrToDouble(IN gctCONST_STRING String, OUT gctDOUBLE* Double);

/* Convert hex string to integer. */
gceSTATUS
gcoOS_HexStrToInt(IN gctCONST_STRING String, OUT gctINT *Int);

/* Convert hex string to float. */
gceSTATUS
gcoOS_HexStrToFloat(IN gctCONST_STRING String, OUT gctFLOAT *Float);

/* Convert string to integer. */
gceSTATUS
gcoOS_StrToInt(IN gctCONST_STRING String, OUT gctINT *Int);

gceSTATUS
gcoOS_MemCmp(IN gctCONST_POINTER Memory1,
             IN gctCONST_POINTER Memory2,
             IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_PrintStrSafe(OUT gctSTRING String,
                   IN gctSIZE_T StringSize,
                   IN OUT gctUINT *Offset,
                   IN gctCONST_STRING Format,
                   ...)
CHECK_PRINTF_FORMAT(4, 5);

gceSTATUS
gcoOS_LoadLibrary(IN gcoOS Os, IN gctCONST_STRING Library, OUT gctHANDLE *Handle);

gceSTATUS
gcoOS_FreeLibrary(IN gcoOS Os, IN gctHANDLE Handle);

gceSTATUS
gcoOS_GetProcAddress(IN gcoOS Os,
                     IN gctHANDLE Handle,
                     IN gctCONST_STRING Name,
                     OUT gctPOINTER *Function);

gceSTATUS
gcoOS_Compact(IN gcoOS Os);

gceSTATUS
gcoOS_AddSignalHandler(IN gceSignalHandlerType SignalHandlerType);

#if PHYTIUM_PROFILER_SYSTEM_MEMORY 
gceSTATUS
gcoOS_ProfileStart(IN gcoOS Os);

gceSTATUS
gcoOS_ProfileEnd(IN gcoOS Os, IN gctCONST_STRING Title);

gceSTATUS
gcoOS_SetProfileSetting(IN gcoOS Os,
                        IN gctBOOL Enable,
                        IN gceProfilerMode ProfileMode,
                        IN gctCONST_STRING FileName);
#endif

/* Get the amount of physical system memory */
gceSTATUS
gcoOS_GetPhysicalSystemMemorySize(OUT gctUINT64 *PhysicalSystemMemorySize);

/* Query the video memory. */
gceSTATUS
gcoOS_QueryVideoMemory(IN gcoOS Os,
                       OUT gctUINT32 *InternalPhysName,
                       OUT gctSIZE_T *InternalSize,
                       OUT gctUINT32 *ExternalPhysName,
                       OUT gctSIZE_T *ExternalSize,
                       OUT gctUINT32 *ContiguousPhysName,
                       OUT gctSIZE_T *ContiguousSize);

gceSTATUS
gcoOS_QueryCurrentProcessName(OUT gctSTRING Name, IN gctSIZE_T Size);

gceSTATUS
gcoOS_QueryCurrentProcessArguments(OUT gctCHAR Argv[gcdMAX_ARGUMENT_COUNT][gcdMAX_ARGUMENT_SIZE],
                                   OUT gctUINT32 *Argc,
                                   IN  gctUINT32 MaxArgc,
                                   IN  gctUINT32 MaxSizePerArg);

/*----------------------------------------------------------------------------*/
/*----- Atoms ----------------------------------------------------------------*/

/* Construct an atom. */
gceSTATUS
gcoOS_AtomConstruct(IN gcoOS Os, OUT gcsATOM_PTR *Atom);

/* Destroy an atom. */
gceSTATUS
gcoOS_AtomDestroy(IN gcoOS Os, IN gcsATOM_PTR Atom);

/* Get the 32-bit value protected by an atom. */
gceSTATUS
gcoOS_AtomGet(IN gcoOS Os, IN gcsATOM_PTR Atom, OUT gctINT32_PTR Value);

/* Set the 32-bit value protected by an atom. */
gceSTATUS
gcoOS_AtomSet(IN gcoOS Os, IN gcsATOM_PTR Atom, IN gctINT32 Value);

/* Increment an atom. */
gceSTATUS
gcoOS_AtomIncrement(IN gcoOS Os, IN gcsATOM_PTR Atom, OUT gctINT32_PTR OldValue);

/* Decrement an atom. */
gceSTATUS
gcoOS_AtomDecrement(IN gcoOS Os, IN gcsATOM_PTR Atom, OUT gctINT32_PTR OldValue);

gctHANDLE
gcoOS_GetCurrentProcessID(void);

gctHANDLE
gcoOS_GetCurrentThreadID(void);

/*----------------------------------------------------------------------------*/
/*----- Time -----------------------------------------------------------------*/

/* Get the number of milliseconds since the system started. */
gctUINT32
gcoOS_GetTicks(void);

/* Get time in microseconds. */
gceSTATUS
gcoOS_GetTime(gctUINT64_PTR Time);

/* Get CPU usage in microseconds. */
gceSTATUS
gcoOS_GetCPUTime(gctUINT64_PTR CPUTime);

/* Get memory usage. */
gceSTATUS
gcoOS_GetMemoryUsage(gctUINT32_PTR MaxRSS,
                     gctUINT32_PTR IxRSS,
                     gctUINT32_PTR IdRSS,
                     gctUINT32_PTR IsRSS);

/* Delay a number of milliseconds. */
gceSTATUS
gcoOS_Delay(IN gcoOS Os, IN gctUINT32 Delay);


/* Delay a number of microseconds. */
gceSTATUS
gcoOS_DelayUs(IN gcoOS Os, IN gctUINT32 Delay);

/*----------------------------------------------------------------------------*/
/*----- Threads --------------------------------------------------------------*/

#ifdef _WIN32
/* Cannot include windows.h here because "near" and "far"
 * which are used in gcsDEPTH_INFO, are defined to nothing in WinDef.h.
 * So, use the real value of DWORD and WINAPI, instead.
 * DWORD is unsigned long, and WINAPI is __stdcall.
 * If these two are change in WinDef.h, the following two typdefs
 * need to be changed, too.
 */
typedef unsigned long gctTHREAD_RETURN;
typedef unsigned long(__stdcall *gcTHREAD_ROUTINE)(void *Argument);
#else
typedef void *gctTHREAD_RETURN;
typedef void *(*gcTHREAD_ROUTINE)(void *);
#endif

/* Create a new thread. */
gceSTATUS
gcoOS_CreateThread(IN gcoOS Os,
                   IN gcTHREAD_ROUTINE Worker,
                   IN gctPOINTER Argument,
                   OUT gctPOINTER *Thread);

/* Close a thread. */
gceSTATUS
gcoOS_CloseThread(IN gcoOS Os, IN gctPOINTER Thread);

/*----------------------------------------------------------------------------*/
/*----- Mutexes --------------------------------------------------------------*/

/* Create a new mutex. */
gceSTATUS
gcoOS_CreateMutex(IN gcoOS Os, OUT gctPOINTER *Mutex);

/* Delete a mutex. */
gceSTATUS
gcoOS_DeleteMutex(IN gcoOS Os, IN gctPOINTER Mutex);

/* Acquire a mutex. */
gceSTATUS
gcoOS_AcquireMutex(IN gcoOS Os, IN gctPOINTER Mutex, IN gctUINT32 Timeout);

/* Release a mutex. */
gceSTATUS
gcoOS_ReleaseMutex(IN gcoOS Os, IN gctPOINTER Mutex);

/*----------------------------------------------------------------------------*/
/*----- Signals --------------------------------------------------------------*/

/* Create a signal. */
gceSTATUS
gcoOS_CreateSignal(IN gcoOS Os, IN gctBOOL ManualReset, OUT gctSIGNAL *Signal);

/* Destroy a signal. */
gceSTATUS
gcoOS_DestroySignal(IN gcoOS Os, IN gctSIGNAL Signal);

/* Signal a signal. */
gceSTATUS
gcoOS_Signal(IN gcoOS Os, IN gctSIGNAL Signal, IN gctBOOL State);

/* Wait for a signal. */
gceSTATUS
gcoOS_WaitSignal(IN gcoOS Os, IN gctSIGNAL Signal, IN gctUINT32 Wait);

/* Map a signal from another process */
gceSTATUS
gcoOS_MapSignal(IN gctSIGNAL RemoteSignal, OUT gctSIGNAL *LocalSignal);

/* Unmap a signal mapped from another process */
gceSTATUS
gcoOS_UnmapSignal(IN gctSIGNAL Signal);

/*----------------------------------------------------------------------------*/
/*----- Android Native Fence -------------------------------------------------*/

/* Create native fence. */
gceSTATUS
gcoOS_CreateNativeFence(IN gcoOS Os, IN gctSIGNAL Signal, OUT gctINT *FenceFD);

/* (CPU) Wait on native fence. */
gceSTATUS
gcoOS_ClientWaitNativeFence(IN gcoOS Os, IN gctINT FenceFD, IN gctUINT32 Timeout);

/* (GPU) Wait on native fence. */
gceSTATUS
gcoOS_WaitNativeFence(IN gcoOS Os, IN gctINT FenceFD, IN gctUINT32 Timeout);

/*----------------------------------------------------------------------------*/
/*----- Memory Access and Cache ----------------------------------------------*/

/* Write a register. */
gceSTATUS
gcoOS_WriteRegister(IN gcoOS Os, IN gctUINT32 Address, IN gctUINT32 Data);

/* Read a register. */
gceSTATUS
gcoOS_ReadRegister(IN gcoOS Os, IN gctUINT32 Address, OUT gctUINT32 *Data);

gceSTATUS
gcoOS_CacheClean(IN gcoOS Os, IN gctUINT32 Node,
                 IN gctPOINTER Logical, IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_CacheFlush(IN gcoOS Os, IN gctUINT32 Node,
                 IN gctPOINTER Logical, IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_CacheInvalidate(IN gcoOS Os, IN gctUINT32 Node,
                      IN gctPOINTER Logical, IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_CacheCleanEx(IN gcoOS Os, IN gctUINT32 Node,
                   IN gctPOINTER Logical, IN gctSIZE_T Offset, IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_CacheFlushEx(IN gcoOS Os, IN gctUINT32 Node,
                   IN gctPOINTER Logical, IN gctSIZE_T Offset, IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_CacheInvalidateEx(IN gcoOS Os, IN gctUINT32 Node,
                        IN gctPOINTER Logical, IN gctSIZE_T Offset, IN gctSIZE_T Bytes);


gceSTATUS
gcoOS_MemoryBarrier(IN gcoOS Os, IN gctPOINTER Logical);

gceSTATUS
gcoOS_CPUPhysicalToGPUPhysical(IN gctPHYS_ADDR_T CPUPhysical,
                               OUT gctPHYS_ADDR_T *GPUPhysical);

gceSTATUS
gcoHAL_QueryCPUFrequency(IN gctUINT32 CPUId, OUT gctUINT32_PTR CPUFrequency);

gceSTATUS
gcoOS_QuerySystemInfo(IN gcoOS Os, OUT gcsSystemInfo *Info);

#ifdef VSIMULATOR_DEBUG
gceSTATUS
gcoOS_UpdateSimulatorCallback(INOUT VSIMULATOR_CALLBACK *Callback, IN gctBOOL update);

gceSTATUS
gcoOS_SetFrameworkType(IN gctUINT32 frameworkType, IN gctINT32 cmSize);

void
gcoOS_SetVSimulatorTarget(IN gctUINT32 hwConfigID, IN gctUINT32 skippedExecution);
#endif

/*----------------------------------------------------------------------------*/
/*----- Profile --------------------------------------------------------------*/

gceSTATUS
gckOS_GetProfileTick(OUT gctUINT64_PTR Tick);

gceSTATUS
gckOS_QueryProfileTickRate(OUT gctUINT64_PTR TickRate);

gctUINT32
gckOS_ProfileToMS(IN gctUINT64 Ticks);

gceSTATUS
gcoOS_GetProfileTick(OUT gctUINT64_PTR Tick);

gceSTATUS
gcoOS_QueryProfileTickRate(OUT gctUINT64_PTR TickRate);

#if gcdSTATIC_LINK
void gcoOS_ModuleConstructor(void);

void gcoOS_ModuleDestructor(void);
#endif

#define _gcmPROFILE_INIT(prefix, freq, start)                               \
    do {                                                                    \
        prefix##OS_QueryProfileTickRate(&(freq));                           \
        prefix##OS_GetProfileTick(&(start));                                \
    } while (gcvFALSE)

#define _gcmPROFILE_QUERY(prefix, start, ticks)                             \
    do {                                                                    \
        prefix##OS_GetProfileTick(&(ticks));                                \
        (ticks) = ((ticks) > (start)) ? ((ticks) - (start))                 \
                                      : (~0ull - (start) + (ticks) + 1);    \
    } while (gcvFALSE)

#if gcdENABLE_PROFILING
# define gcmkPROFILE_INIT(freq, start)    _gcmPROFILE_INIT(gck, freq, start)
# define gcmkPROFILE_QUERY(start, ticks)  _gcmPROFILE_QUERY(gck, start, ticks)
# define gcmPROFILE_INIT(freq, start)     _gcmPROFILE_INIT(gco, freq, start)
# define gcmPROFILE_QUERY(start, ticks)   _gcmPROFILE_QUERY(gco, start, ticks)
# define gcmPROFILE_ONLY(x)               x
# define gcmPROFILE_ELSE(x)               do { } while (gcvFALSE)
# define gcmPROFILE_DECLARE_ONLY(x)       x
#else
# define gcmkPROFILE_INIT(start, freq)    do { } while (gcvFALSE)
# define gcmkPROFILE_QUERY(start, ticks)  do { } while (gcvFALSE)
# define gcmPROFILE_INIT(start, freq)     do { } while (gcvFALSE)
# define gcmPROFILE_QUERY(start, ticks)   do { } while (gcvFALSE)
# define gcmPROFILE_ONLY(x)               do { } while (gcvFALSE)
# define gcmPROFILE_ELSE(x)               x
# define gcmPROFILE_DECLARE_ONLY(x)       do { } while (gcvFALSE)
#endif

/*******************************************************************************
 **  gcoMATH object
 */

#define gcdPI       3.14159265358979323846f

/* User. */
gctUINT32
gcoMATH_Log2in5dot5(IN gctINT X);

gctFLOAT
gcoMATH_UIntAsFloat(IN gctUINT32 X);

gctUINT32
gcoMATH_FloatAsUInt(IN gctFLOAT X);

gctBOOL
gcoMATH_CompareEqualF(IN gctFLOAT X, IN gctFLOAT Y);

gctUINT16
gcoMATH_UInt8AsFloat16(IN gctUINT8 X);

gctUINT32
gcoMATH_Float16ToFloat(IN gctUINT16 In);

gctUINT16
gcoMATH_FloatToFloat16(IN gctUINT32 In);

gctUINT17
gcoMATH_FloatToFloat17(IN gctUINT32 In);

gctUINT32
gcoMATH_Float11ToFloat(IN gctUINT32 In);

gctUINT16
gcoMATH_FloatToFloat11(IN gctUINT32 In);

gctUINT32
gcoMATH_Float10ToFloat(IN gctUINT32 In);

gctUINT16
gcoMATH_FloatToFloat10(IN gctUINT32 In);

gctUINT32
gcoMATH_Float14ToFloat(IN gctUINT16 In);

/******************************************************************************
 **************************** Coordinate Structures ***************************
 ******************************************************************************/

typedef struct _gcsPOINT {
    gctINT32        x;
    gctINT32        y;
} gcsPOINT;

typedef struct _gcsSIZE {
    gctINT32        width;
    gctINT32        height;
} gcsSIZE;

typedef struct _gcsRECT {
    gctINT32        left;
    gctINT32        top;
    gctINT32        right;
    gctINT32        bottom;
} gcsRECT;

typedef struct _gcsRECTFU {
    gcuFLOAT_UINT32 left;
    gcuFLOAT_UINT32 top;
    gcuFLOAT_UINT32 right;
    gcuFLOAT_UINT32 bottom;
} gcsRECTFU;

typedef struct _gcs2D_RGBU32
{
    gctUINT32       R;
    gctUINT32       G;
    gctUINT32       B;
} gcs2D_RGBU32;

typedef struct _gcsPIXEL {
    union {
        struct {
            gctFLOAT r, g, b, a;
        } f;
        struct {
            gctINT32 r, g, b, a;
        } i;
        struct {
            gctUINT32 r, g, b, a;
        } ui;
    } color;

    gctFLOAT  d;
    gctUINT32 s;

} gcsPIXEL;

/******************************************************************************
 ******************************** gcoSURF Object ******************************
 ******************************************************************************/

/*----------------------------------------------------------------------------*/
/*------------------------------- gcoSURF Common ------------------------------*/

/* Color format component parameters. */
typedef struct _gcsFORMAT_COMPONENT {
    gctUINT8                    start;
    gctUINT8                    width;
} gcsFORMAT_COMPONENT;

/* RGBA color format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_RGBA {
    gcsFORMAT_COMPONENT         alpha;
    gcsFORMAT_COMPONENT         red;
    gcsFORMAT_COMPONENT         green;
    gcsFORMAT_COMPONENT         blue;
} gcsFORMAT_CLASS_TYPE_RGBA;

/* YUV color format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_YUV {
    gcsFORMAT_COMPONENT         y;
    gcsFORMAT_COMPONENT         u;
    gcsFORMAT_COMPONENT         v;
} gcsFORMAT_CLASS_TYPE_YUV;

/* Index color format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_INDEX {
    gcsFORMAT_COMPONENT         value;
} gcsFORMAT_CLASS_TYPE_INDEX;

/* Luminance color format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_LUMINANCE {
    gcsFORMAT_COMPONENT         alpha;
    gcsFORMAT_COMPONENT         value;
} gcsFORMAT_CLASS_TYPE_LUMINANCE;

/* Bump map color format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_BUMP {
    gcsFORMAT_COMPONENT         alpha;
    gcsFORMAT_COMPONENT         l;
    gcsFORMAT_COMPONENT         v;
    gcsFORMAT_COMPONENT         u;
    gcsFORMAT_COMPONENT         q;
    gcsFORMAT_COMPONENT         w;
} gcsFORMAT_CLASS_TYPE_BUMP;

/* Depth and stencil format class. */
typedef struct _gcsFORMAT_CLASS_TYPE_DEPTH {
    gcsFORMAT_COMPONENT         depth;
    gcsFORMAT_COMPONENT         stencil;
} gcsFORMAT_CLASS_TYPE_DEPTH;

/* Intensity format class. */
typedef struct _gcsFORMAT_CLASs_TYPE_INTENSITY {
    gcsFORMAT_COMPONENT         value;
} gcsFORMAT_CLASs_TYPE_INTENSITY;

typedef union _gcuPIXEL_FORMAT_CLASS {
    gcsFORMAT_CLASS_TYPE_BUMP       bump;
    gcsFORMAT_CLASS_TYPE_RGBA       rgba;
    gcsFORMAT_CLASS_TYPE_YUV        yuv;
    gcsFORMAT_CLASS_TYPE_LUMINANCE  lum;
    gcsFORMAT_CLASS_TYPE_INDEX      index;
    gcsFORMAT_CLASS_TYPE_DEPTH      depth;
    gcsFORMAT_CLASs_TYPE_INTENSITY  intensity;
} gcuPIXEL_FORMAT_CLASS;

/* Format parameters. */
typedef struct _gcsSURF_FORMAT_INFO {
    /* Name of the format */
    gctCONST_STRING             formatName;

    /* Format code and class. */
    gceSURF_FORMAT              format;
    gceFORMAT_CLASS             fmtClass;

    /* Format data type */
    gceFORMAT_DATATYPE          fmtDataType;

    /* The size of one pixel in bits. */
    gctUINT8                    bitsPerPixel;

    /* Pixel block dimensions. */
    gctUINT                     blockWidth;
    gctUINT                     blockHeight;

    /* Pixel block size in bits. */
    gctUINT                     blockSize;

    /* Some formats are larger than what the GPU can support.      */
    /* These formats are read in the number of layers specified.   */
    gctUINT8                    layers;

    /* The format is faked and software will interpret it differently
     * with HW. Most of them can't be blendable(PE) or filterable(TX).
     */
    gctBOOL                     fakedFormat;

    /* Some formats have two neighbour pixels interleaved together. */
    /* To describe such format, set the flag to 1 and add another   */
    /* like this one describing the odd pixel format.               */
    gctBOOL                     interleaved;

    /* sRGB format. */
    gctBOOL                     sRGB;

    /* How GPU read from big-endian host memory */
    gceENDIAN_HINT              endian;

    /* Format components. */
    gcuPIXEL_FORMAT_CLASS       u;

    /* Format components. */
    gcuPIXEL_FORMAT_CLASS       uOdd;

    /* Render format. */
    gceSURF_FORMAT              closestRenderFormat;
    gctUINT                     renderFormat;
    const gceTEXTURE_SWIZZLE   *pixelSwizzle;

    /* Texture format. */
    gceSURF_FORMAT              closestTXFormat;
    gctUINT                     txFormat;
    const gceTEXTURE_SWIZZLE   *txSwizzle;
    gctBOOL                     txIntFilter;

    /* 3DBlit format. */
    gctUINT                     blitFormat;
    const gceTEXTURE_SWIZZLE   *blitSwizzle;
} gcsSURF_FORMAT_INFO;

/* Frame buffer information. */
typedef struct _gcsSURF_FRAMEBUFFER {
    gctPOINTER                  logical;
    gctUINT                     width, height;
    gctINT                      stride;
    gceSURF_FORMAT              format;
} gcsSURF_FRAMEBUFFER;

typedef union _gcu2D_STATE_VALUE
{
    gcs2D_RGBU32                    minValue;
    gcs2D_RGBU32                    maxMinReciprocal;
    gcs2D_RGBU32                    stdReciprocal;
    gcs2D_RGBU32                    meanValue;
    gctBOOL                         enable;
    gctUINT32                       stepReciprocal;
    gce2D_NORMALIZATION_MODE        normalizationMode;
    gctBOOL                         byPassQuantization;
    gce2D_U8ToU10_CONVERSION_MODE   u8Tu10_Mode;
    gce2D_MULTICORE_MODE            multicoreMode;
} gcu2D_STATE_VALUE;

/* Set 2D state */
typedef struct _gcs2D_STATE_CONFIG
{
    gce2D_STATE_KEY             state;
    gcu2D_STATE_VALUE           value;
} gcs2D_STATE_CONFIG;

/* Generic pixel component descriptors. */
extern gcsFORMAT_COMPONENT gcvPIXEL_COMP_XXX8;
extern gcsFORMAT_COMPONENT gcvPIXEL_COMP_XX8X;
extern gcsFORMAT_COMPONENT gcvPIXEL_COMP_X8XX;
extern gcsFORMAT_COMPONENT gcvPIXEL_COMP_8XXX;

/* Construct a new gcoSURF object. */
gceSTATUS
gcoSURF_Construct(IN gcoHAL Hal,
                  IN gctUINT Width,
                  IN gctUINT Height,
                  IN gctUINT Depth,
                  IN gceSURF_TYPE Type,
                  IN gceSURF_FORMAT Format,
                  IN gcePOOL Pool,
                  OUT gcoSURF *Surface);

gceSTATUS
gcoSURF_ConstructWithUserPool(IN gcoHAL Hal,
                              IN gctUINT Width,
                              IN gctUINT Height,
                              IN gctUINT Depth,
                              IN gceSURF_TYPE Type,
                              IN gceSURF_FORMAT Format,
                              IN gctPOINTER TileStatusLogical,
                              IN gctPHYS_ADDR_T TileStatusPhysical,
                              IN gctPOINTER Logical,
                              IN gctPHYS_ADDR_T Physical,
                              OUT gcoSURF *Surface);

/* Destroy an gcoSURF object. */
gceSTATUS
gcoSURF_Destroy(IN gcoSURF Surface);

gceSTATUS
gcoSURF_DestroyForAllHWType(IN gcoSURF Surface);

/* Map user-allocated surface. */
gceSTATUS
gcoSURF_MapUserSurface(IN gcoSURF Surface,
                       IN gctUINT Alignment,
                       IN gctPOINTER Logical,
                       IN gctPHYS_ADDR_T Physical);

/* Wrapp surface with known logical/GPU address */
gceSTATUS
gcoSURF_WrapSurface(IN gcoSURF Surface,
                    IN gctUINT Alignment,
                    IN gctPOINTER Logical,
                    IN gctADDRESS Address);

/* Query vid mem node info. */
gceSTATUS
gcoSURF_QueryVidMemNode(IN gcoSURF Surface,
                        OUT gctUINT32 *Node,
                        OUT gcePOOL *Pool,
                        OUT gctSIZE_T_PTR Bytes,
                        OUT gctUINT32 *TsNode,
                        OUT gcePOOL *TsPool,
                        OUT gctSIZE_T_PTR TsBytes);

/* Query vid mem Multi node info. */
gceSTATUS
gcoSURF_QueryVidMemMultiNode(IN gcoSURF Surface,
                             OUT gctUINT32 *Node,
                             OUT gcePOOL *Pool,
                             OUT gctSIZE_T_PTR Bytes,
                             OUT gctUINT32 *Node2,
                             OUT gcePOOL *Pool2,
                             OUT gctSIZE_T_PTR Bytes2,
                             OUT gctUINT32 *Node3,
                             OUT gcePOOL *Pool3,
                             OUT gctSIZE_T_PTR Bytes3);

/* Set the color type of the surface. */
gceSTATUS
gcoSURF_SetColorType(IN gcoSURF Surface, IN gceSURF_COLOR_TYPE ColorType);

/* Get the color type of the surface. */
gceSTATUS
gcoSURF_GetColorType(IN gcoSURF Surface, OUT gceSURF_COLOR_TYPE *ColorType);

/* Set the color space of the surface. */
gceSTATUS
gcoSURF_SetColorSpace(IN gcoSURF Surface, IN gceSURF_COLOR_SPACE ColorSpace);

/* Get the color space of the surface. */
gceSTATUS
gcoSURF_GetColorSpace(IN gcoSURF Surface, OUT gceSURF_COLOR_SPACE *ColorSpace);

/* Set the surface ration angle. */
gceSTATUS
gcoSURF_SetRotation(IN gcoSURF Surface, IN gceSURF_ROTATION Rotation);

gceSTATUS
gcoSURF_IsValid(IN gcoSURF Surface);

#if gcdENABLE_3D
/* Verify and return the state of the tile status mechanism. */
gceSTATUS
gcoSURF_IsTileStatusSupported(IN gcoSURF Surface);

/* Verify if surface has tile status enabled. */
gceSTATUS
gcoSURF_IsTileStatusEnabled(IN gcsSURF_VIEW *SurfView);

/* Verify if surface is compressed. */
gceSTATUS
gcoSURF_IsCompressed(IN gcsSURF_VIEW *SurfView);

/* Enable tile status for the specified surface on zero slot. */
gceSTATUS
gcoSURF_EnableTileStatus(IN gcsSURF_VIEW *Surface);

/* Enable tile status for the specified surface on specified slot. */
gceSTATUS
gcoSURF_EnableTileStatusEx(IN gcsSURF_VIEW *surfView, IN gctUINT RtIndex);

/* Disable tile status for the specified surface. */
gceSTATUS
gcoSURF_DisableTileStatus(IN gcsSURF_VIEW *SurfView, IN gctBOOL Decompress);

/* Flush tile status cache for the specified surface. */
gceSTATUS
gcoSURF_FlushTileStatus(IN gcsSURF_VIEW *SurfView, IN gctBOOL Decompress);
#endif /* gcdENABLE_3D */

/* Get surface size. */
gceSTATUS
gcoSURF_GetSize(IN gcoSURF Surface,
                OUT gctUINT *Width,
                OUT gctUINT *Height,
                OUT gctUINT *Depth);

/* Get surface information */
gceSTATUS
gcoSURF_GetInfo(IN gcoSURF Surface,
                IN gceSURF_INFO_TYPE InfoType,
                IN OUT gctINT32 *Value);

/* Get surface aligned sizes. */
gceSTATUS
gcoSURF_GetAlignedSize(IN gcoSURF Surface,
                       OUT gctUINT *Width,
                       OUT gctUINT *Height,
                       OUT gctINT *Stride);

/* Get alignments. */
gceSTATUS
gcoSURF_GetAlignment(IN gceSURF_TYPE Type,
                     IN gceSURF_FORMAT Format,
                     OUT gctUINT *AddressAlignment,
                     OUT gctUINT *XAlignment,
                     OUT gctUINT *YAlignment);

gceSTATUS
gcoSURF_AlignResolveRect(IN gcoSURF Surf,
                         IN gcsPOINT_PTR RectOrigin,
                         IN gcsPOINT_PTR RectSize,
                         OUT gcsPOINT_PTR AlignedOrigin,
                         OUT gcsPOINT_PTR AlignedSize);

/* Get surface type and format. */
gceSTATUS
gcoSURF_GetFormat(IN gcoSURF Surface,
                  OUT OPTIONAL gceSURF_TYPE *Type,
                  OUT OPTIONAL gceSURF_FORMAT *Format);

/* Get surface information */
gceSTATUS
gcoSURF_GetFormatInfo(IN gcoSURF Surface,
                      OUT gcsSURF_FORMAT_INFO_PTR *formatInfo);

/* Get Surface pack format */
gceSTATUS
gcoSURF_GetPackedFormat(IN gcoSURF Surface,
                        OUT gceSURF_FORMAT *Format);

/* Get surface tiling. */
gceSTATUS
gcoSURF_GetTiling(IN gcoSURF Surface, OUT gceTILING *Tiling);

/* Get bottom buffer offset bytes. */
gceSTATUS
gcoSURF_GetBottomBufferOffset(IN gcoSURF Surface,
                              OUT gctUINT_PTR BottomBufferOffset);

/* Lock the surface. */
gceSTATUS
gcoSURF_Lock(IN gcoSURF Surface,
             IN OUT gctADDRESS *Address,
             IN OUT gctPOINTER *Memory);

/* Unlock the surface. */
gceSTATUS
gcoSURF_Unlock(IN gcoSURF Surface, IN gctPOINTER Memory);

/*. Query surface flags.*/
gceSTATUS
gcoSURF_QueryFlags(IN gcoSURF Surface, IN gceSURF_FLAG Flag);

gceSTATUS
gcoSURF_QueryHints(IN gcoSURF Surface, IN gceSURF_TYPE Hints);

/* Return pixel format parameters; Info is required to be a pointer to an
 * array of at least two items because some formats have up to two records
 * of description.
 */
gceSTATUS
gcoSURF_QueryFormat(IN gceSURF_FORMAT Format,
                    OUT gcsSURF_FORMAT_INFO_PTR *Info);

/* Compute the color pixel mask. */
gceSTATUS
gcoSURF_ComputeColorMask(IN gcsSURF_FORMAT_INFO_PTR Format,
                         OUT gctUINT32_PTR ColorMask);

/* Flush the surface. */
gceSTATUS
gcoSURF_Flush(IN gcoSURF Surface);

gceSTATUS
gcoSURF_3DBlitClearTileStatus(IN gcsSURF_VIEW *SurfView,
                              IN gctBOOL ClearAsDirty);

/* Fill surface from it's tile status buffer. */
gceSTATUS
gcoSURF_FillFromTile(IN gcsSURF_VIEW *SurView);

/* Fill surface with a value. */
gceSTATUS
gcoSURF_Fill(IN gcoSURF Surface,
             IN gcsPOINT_PTR Origin,
             IN gcsSIZE_PTR Size,
             IN gctUINT32 Value,
             IN gctUINT32 Mask);

/* Alpha blend two surfaces together. */
gceSTATUS
gcoSURF_Blend(IN gcoSURF SrcSurf,
              IN gcoSURF DstSurf,
              IN gcsPOINT_PTR SrcOrigin,
              IN gcsPOINT_PTR DstOrigin,
              IN gcsSIZE_PTR Size,
              IN gceSURF_BLEND_MODE Mode);

/* Create a new gcoSURF wrapper object. */
gceSTATUS
gcoSURF_ConstructWrapper(IN gcoHAL Hal, OUT gcoSURF *Surface);

/* Set surface flags.*/
gceSTATUS
gcoSURF_SetFlags(IN gcoSURF Surface, IN gceSURF_FLAG Flag, IN gctBOOL Value);

/* Set the underlying buffer for the surface wrapper. */
gceSTATUS
gcoSURF_SetBuffer(IN gcoSURF Surface,
                  IN gceSURF_TYPE Type,
                  IN gceSURF_FORMAT Format,
                  IN gctUINT Stride,
                  IN gctPOINTER Logical,
                  IN gctUINT64 Physical);

/* Set the size of the surface in pixels and map the underlying buffer. */
gceSTATUS
gcoSURF_SetWindow(IN gcoSURF Surface,
                  IN gctUINT X,
                  IN gctUINT Y,
                  IN gctUINT Width,
                  IN gctUINT Height);

/* Set the size of the surface in pixels and map the underlying buffer. */
gceSTATUS
gcoSURF_SetImage(IN gcoSURF Surface,
                 IN gctUINT X,
                 IN gctUINT Y,
                 IN gctUINT Width,
                 IN gctUINT Height,
                 IN gctUINT Depth);

/* Set width/height alignment of the surface directly and calculate stride/size.
 * This is only for dri backend now. Please be careful before use.
 */
gceSTATUS
gcoSURF_SetAlignment(IN gcoSURF Surface, IN gctUINT Width, IN gctUINT Height);

/* Increase reference count of the surface. */
gceSTATUS
gcoSURF_ReferenceSurface(IN gcoSURF Surface);

/* Get surface reference count. */
gceSTATUS
gcoSURF_QueryReferenceCount(IN gcoSURF Surface, OUT gctINT32 *ReferenceCount);

/* Set surface orientation. */
gceSTATUS
gcoSURF_SetOrientation(IN gcoSURF Surface, IN gceORIENTATION Orientation);

/* Query surface orientation. */
gceSTATUS
gcoSURF_QueryOrientation(IN gcoSURF Surface, OUT gceORIENTATION *Orientation);

gceSTATUS
gcoSURF_NODE_Cache(IN gcsSURF_NODE_PTR Node,
                   IN gctPOINTER Logical,
                   IN gctSIZE_T Bytes,
                   IN gceCACHEOPERATION Operation);

gceSTATUS
gcsSURF_NODE_SetHardwareAddress(IN gcsSURF_NODE_PTR Node, IN gctADDRESS Address);

gceSTATUS
gcsSURF_NODE_GetHardwareAddress(IN gcsSURF_NODE_PTR Node,
                                OUT gctADDRESS *Physical,
                                OUT gctADDRESS *Physical2,
                                OUT gctADDRESS *Physical3,
                                OUT gctADDRESS *PhysicalBottom);

gctADDRESS
gcsSURF_NODE_GetHWAddress(IN gcsSURF_NODE_PTR Node);

/* Lock and unlock surface node */
gceSTATUS
gcoSURF_LockNode(IN gcsSURF_NODE_PTR Node,
                 OUT gctADDRESS *Address,
                 OUT gctPOINTER *Memory);

gceSTATUS
gcoSURF_UnLockNode(IN gcsSURF_NODE_PTR Node, IN gceSURF_TYPE Type);

/* Perform CPU cache operation on surface node */
gceSTATUS
gcoSURF_NODE_CPUCacheOperation(IN gcsSURF_NODE_PTR Node,
                               IN gceSURF_TYPE Type,
                               IN gctSIZE_T Offset,
                               IN gctSIZE_T Length,
                               IN gceCACHEOPERATION Operation);

/* Perform CPU cache operation on surface */
gceSTATUS
gcoSURF_CPUCacheOperation(IN gcoSURF Surface,
                          IN gceCACHEOPERATION Operation);

gceSTATUS
gcoSURF_Swap(IN gcoSURF Surface1, IN gcoSURF Surface2);

gceSTATUS
gcoSURF_ResetSurWH(IN gcoSURF Surface,
                   IN gctUINT oriw,
                   IN gctUINT orih,
                   IN gctUINT alignw,
                   IN gctUINT alignh,
                   IN gceSURF_FORMAT fmt);

/* Update surface timestamp. */
gceSTATUS
gcoSURF_UpdateTimeStamp(IN gcoSURF Surface);

/* Query surface current timestamp. */
gceSTATUS
gcoSURF_QueryTimeStamp(IN gcoSURF Surface, OUT gctUINT64 *TimeStamp);

/*
 * Allocate shared buffer for this surface, so that
 * surface states can be shared across processes.
 */
gceSTATUS
gcoSURF_AllocShBuffer(IN gcoSURF Surface, OUT gctSHBUF *ShBuf);

/* Bind shared buffer to this surface */
gceSTATUS
gcoSURF_BindShBuffer(IN gcoSURF Surface, IN gctSHBUF ShBuf);

/* Push surface shared states to shared buffer. */
gceSTATUS
gcoSURF_PushSharedInfo(IN gcoSURF Surface);

/* Pop shared states from shared buffer. */
gceSTATUS
gcoSURF_PopSharedInfo(IN gcoSURF Surface);

#if (gcdENABLE_3D)
/* Copy surface. */
gceSTATUS
gcoSURF_Copy(IN gcoSURF Surface, IN gcoSURF Source);

/* Set number of samples for a gcoSURF object. */
gceSTATUS
gcoSURF_SetSamples(IN gcoSURF Surface, IN gctUINT Samples);

/* Get the number of samples per pixel. */
gceSTATUS
gcoSURF_GetSamples(IN gcoSURF Surface, OUT gctUINT_PTR Samples);

/* Append tile status buffer to user pool surface. */
gceSTATUS
gcoSURF_AppendTileStatus(IN gcoSURF Surface);
#endif

gceSTATUS
gcoSURF_WrapUserMemory(IN gcoHAL Hal,
                       IN gctUINT Width,
                       IN gctUINT Height,
                       IN gctUINT Stride,
                       IN gctUINT Depth,
                       IN gceSURF_TYPE Type,
                       IN gceSURF_FORMAT Format,
                       IN gctUINT32 Handle,
                       IN gctUINT32 Flag,
                       OUT gcoSURF *Surface);

#ifdef EMULATOR
/* Wrap user memory by external address. */
gceSTATUS
gcoSURF_WrapUserMemory_2(    IN gcoHAL Hal,
                       IN gctUINT Width,
                       IN gctUINT Height,
                       IN gctUINT Stride,
                       IN gctUINT Depth,
                       IN gceSURF_TYPE Type,
                       IN gceSURF_FORMAT Format,
                       IN gctUINT32_PTR Address,
                       IN gctUINT32 Flag,
                       OUT gcoSURF * Surface);
#endif

gceSTATUS
gcoSURF_WrapUserMultiBuffer(IN gcoHAL Hal,
                            IN gctUINT Width,
                            IN gctUINT Height,
                            IN gceSURF_TYPE Type,
                            IN gceSURF_FORMAT Format,
                            IN gctUINT Stride[3],
                            IN gctUINT32 Handle[3],
                            IN gctUINT BufferOffset[3],
                            IN gctUINT32 Flag,
                            OUT gcoSURF *Surface,
                            OUT gctINT Ts_fd[3]);

gceSTATUS
gcoSURF_UpdateMetadata(IN gcoSURF Surface, IN gctINT TsFD);

#define MAX_SURF_MIX_SRC_NUM 64
gceSTATUS
gcoSURF_MixSurfacesCPU(IN gcoSURF TargetSurface,
                       IN gctUINT TargetSliceIndex,
                       IN gcoSURF *SourceSurface,
                       IN gctUINT *SourceSliceIndices,
                       IN gctFLOAT *Weights,
                       IN gctINT Count);

/******************************************************************************
 ****************************** Hash Structure ********************************
 ******************************************************************************/

typedef struct _gcsHASH_MD5CTX {
    gctBOOL   bigEndian;
    gctSIZE_T bytes; /* Number of bytes processed */
    gctUINT32 states[4];
    gctUINT8  buffer[64];
} gcsHASH_MD5CTX;

void
gcsHASH_MD5Init(gcsHASH_MD5CTX *ctx);
void
gcsHASH_MD5Update(gcsHASH_MD5CTX *ctx, const void *data, gctSIZE_T bytes);
void
gcsHASH_MD5Final(gcsHASH_MD5CTX *ctx, gctUINT8 digest[16]);

/******************************************************************************
 ******************************* gcsRECT Structure ****************************
 ******************************************************************************/

/* Initialize rectangle structure. */
gceSTATUS
gcsRECT_Set(OUT gcsRECT_PTR Rect,
            IN gctINT32 Left,
            IN gctINT32 Top,
            IN gctINT32 Right,
            IN gctINT32 Bottom);

/* Return the width of the rectangle. */
gceSTATUS
gcsRECT_Width(IN gcsRECT_PTR Rect, OUT gctINT32 *Width);

/* Return the height of the rectangle. */
gceSTATUS
gcsRECT_Height(IN gcsRECT_PTR Rect, OUT gctINT32 *Height);

/* Ensure that top left corner is to the left and above the right bottom. */
gceSTATUS
gcsRECT_Normalize(IN OUT gcsRECT_PTR Rect);

/* Compare two rectangles. */
gceSTATUS
gcsRECT_IsEqual(IN gcsRECT_PTR Rect1, IN gcsRECT_PTR Rect2, OUT gctBOOL *Equal);

/* Compare the sizes of two rectangles. */
gceSTATUS
gcsRECT_IsOfEqualSize(IN gcsRECT_PTR Rect1, IN gcsRECT_PTR Rect2, OUT gctBOOL *EqualSize);

gceSTATUS
gcsRECT_RelativeRotation(IN gceSURF_ROTATION Orientation,
                         IN OUT gceSURF_ROTATION *Relation);

gceSTATUS
gcsRECT_Rotate(IN OUT gcsRECT_PTR Rect,
               IN gceSURF_ROTATION Rotation,
               IN gceSURF_ROTATION toRotation,
               IN gctINT32 SurfaceWidth,
               IN gctINT32 SurfaceHeight);

/******************************************************************************
 **************************** gcsBOUNDARY Structure ***************************
 ******************************************************************************/

typedef struct _gcsBOUNDARY {
    gctINT x;
    gctINT y;
    gctINT width;
    gctINT height;
} gcsBOUNDARY;

/******************************************************************************
 ********************************* gcoHEAP Object *****************************
 ******************************************************************************/

typedef struct _gcoHEAP *gcoHEAP;

/* Construct a new gcoHEAP object. */
gceSTATUS
gcoHEAP_Construct(IN gcoOS Os, IN gctSIZE_T AllocationSize, OUT gcoHEAP *Heap);

/* Destroy an gcoHEAP object. */
gceSTATUS
gcoHEAP_Destroy(IN gcoHEAP Heap);

/* Allocate memory. */
gceSTATUS
gcoHEAP_Allocate(IN gcoHEAP Heap, IN gctSIZE_T Bytes, OUT gctPOINTER *Node);

gceSTATUS
gcoHEAP_GetMemorySize(IN gcoHEAP Heap, IN gctPOINTER Memory, OUT gctSIZE_T_PTR MemorySize);

/* Free memory. */
gceSTATUS
gcoHEAP_Free(IN gcoHEAP Heap, IN gctPOINTER Node);

#if (PHYTIUM_PROFILER_SYSTEM_MEMORY  || gcdDEBUG)
/* Profile the heap. */
gceSTATUS
gcoHEAP_ProfileStart(IN gcoHEAP Heap);

gceSTATUS
gcoHEAP_ProfileEnd(IN gcoHEAP Heap, IN gctCONST_STRING Title);
#endif


/******************************************************************************
 ******************************* Debugging Macros *****************************
 ******************************************************************************/

void
gcoOS_SetDebugLevel(IN gctUINT32 Level);

void
gcoOS_GetDebugLevel(OUT gctUINT32_PTR DebugLevel);

void
gcoOS_GetDebugZone(IN gctUINT32 Zone, OUT gctUINT32_PTR DebugZone);

void
gcoOS_SetDebugZone(IN gctUINT32 Zone);

void
gcoOS_SetDebugFile(IN gctCONST_STRING FileName);

void
gcoOS_EnableDebugDump(IN gctBOOL Enable);

gctFILE
gcoOS_ReplaceDebugFile(IN gctFILE fp);

/*******************************************************************************
 **
 **  gcmFATAL
 **
 **      Print a message to the debugger and execute a break point.
 **
 **  ARGUMENTS:
 **
 **      message .
 **      ...     Optional arguments.
 */

void
gckOS_DebugFatal(IN gctCONST_STRING Message, ...);

void
gcoOS_DebugFatal(IN gctCONST_STRING Message, ...);

#if gcmIS_DEBUG(gcdDEBUG_FATAL)
# define gcmFATAL        gcoOS_DebugFatal
# define gcmkFATAL       gckOS_DebugFatal
#else
# define gcmFATAL(...)
# define gcmkFATAL(...)
#endif

/*******************************************************************************
 **
 **  gcmTRACE
 **
 **      Print a message to the debugfer if the correct level has been set.  In
 **      retail mode this macro does nothing.
 **
 **  ARGUMENTS:
 **
 **      level   Level of message.
 **      message .
 **      ...     Optional arguments.
 */
#define gcvLEVEL_NONE       -1
#define gcvLEVEL_ERROR      0
#define gcvLEVEL_WARNING    1
#define gcvLEVEL_INFO       2
#define gcvLEVEL_VERBOSE    3

void
gckOS_DebugTrace(IN gctUINT32 Level, IN gctCONST_STRING Message, ...)
CHECK_PRINTF_FORMAT(2, 3);

void
gcoOS_DebugTrace(IN gctUINT32 Level, IN gctCONST_STRING Message, ...)
CHECK_PRINTF_FORMAT(2, 3);

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
# define gcmTRACE            gcoOS_DebugTrace
# define gcmkTRACE           gckOS_DebugTrace
# define gcmkTRACE_N(Level, ArgumentSize, ...) \
        gckOS_DebugTrace(Level, __VA_ARGS__)
#else
# define gcmTRACE(...)
# define gcmkTRACE(...)
# define gcmkTRACE_N(...)
#endif

/*******************************************************************************
 **
 **  gcmTRACE_ZONE
 **
 **      Print a message to the debugger if the correct level and zone has been
 **      set.  In retail mode this macro does nothing.
 **
 **  ARGUMENTS:
 **
 **      Level   Level of message.
 **      Zone    Zone of message.
 **      Message debuger message.
 **      ...     Optional arguments.
 */

void
gckOS_DebugTraceZone(IN gctUINT32 Level, IN gctUINT32 Zone, IN gctCONST_STRING Message, ...);

void
gcoOS_DebugTraceZone(IN gctUINT32 Level, IN gctUINT32 Zone, IN gctCONST_STRING Message, ...);

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
# define gcmTRACE_ZONE      gcoOS_DebugTraceZone
# define gcmkTRACE_ZONE     gckOS_DebugTraceZone
# define gcmkTRACE_ZONE_N(Level, Zone, ArgumentSize, ...) \
        gckOS_DebugTraceZone(Level, Zone, __VA_ARGS__)
#else
# define gcmTRACE_ZONE(...)
# define gcmkTRACE_ZONE(...)
# define gcmkTRACE_ZONE_N(...)
#endif

/*******************************************************************************
 **
 **  gcmDEBUG_ONLY
 **
 **      Execute a statement or function only in DEBUG mode.
 **
 **  ARGUMENTS:
 **
 **      f       Statement or function to execute.
 */
#if gcmIS_DEBUG(gcdDEBUG_CODE)
# define gcmDEBUG_ONLY(f)        f
#else
# define gcmDEBUG_ONLY(f)
#endif

/*******************************************************************************
 **
 **  gcmSTACK_PUSH
 **  gcmSTACK_POP
 **  gcmSTACK_DUMP
 **  gcmSTACK_REMOVE
 **
 **      Push or pop a function with entry arguments on the trace stack.
 **
 **  ARGUMENTS:
 **
 **      Function    Name of function.
 **      Line        Line number.
 **      Text        Optional text.
 **      ...         Optional arguments for text.
 **
 **      Thread      Thread id.
 */
void
gcoOS_StackPush(IN gctINT8_PTR Identity,
                IN gctCONST_STRING Function,
                IN gctINT Line,
                IN gctCONST_STRING Text,
                ...);

void
gcoOS_StackPop(IN gctINT8_PTR Identity, IN gctCONST_STRING Function);

void
gcoOS_StackDump(void);

void
gcoOS_StackRemove(IN gctHANDLE Thread);

#if gcmIS_DEBUG(gcdDEBUG_STACK)
# define gcmSTACK_PUSH           gcoOS_StackPush
# define gcmSTACK_POP            gcoOS_StackPop
# define gcmSTACK_DUMP           gcoOS_StackDump
# define gcmSTACK_REMOVE         gcoOS_StackRemove
#else
# define gcmSTACK_PUSH(...)
# define gcmSTACK_POP(...)
# define gcmSTACK_DUMP()
# define gcmSTACK_REMOVE(...)
#endif

/*******************************************************************************
 **
 **  gcmBINARY_TRACE
 **
 **      Push or pop a function with entry arguments on the trace stack.
 **
 **  ARGUMENTS:
 **
 **      Function    Name of function
 **      Line        Line number
 **      Text        Optional text
 **      ...         Optional arguments for text.
 */
void
gcoOS_BinaryTrace(IN gctCONST_STRING Function,
                  IN gctINT Line,
                  IN gctCONST_STRING Text OPTIONAL,
                  ...);

void
gckOS_BinaryTrace(IN gctCONST_STRING Function,
                  IN gctINT Line,
                  IN gctCONST_STRING Text OPTIONAL,
                  ...);

#if gcdBINARY_TRACE
# define gcmBINARY_TRACE         gcoOS_BinaryTrace
# define gcmkBINARY_TRACE        gckOS_BinaryTrace
#else
# define gcmBINARY_TRACE(Function, Line, Text, ...)
# define gcmkBINARY_TRACE(Function, Line, Text, ...)
#endif

/*******************************************************************************
 **
 **  gcmSYSTRACE_BEGIN
 **  gcmSYSTRACE_END
 **
 **      Systrace is a performance tuning tool on linux.
 **
 **  ARGUMENTS:
 **
 **      FuncName Function name
 **      Zone     Systrace zone. Only specified zones are traced.
 */

void
gcoOS_SysTraceBegin(IN gctUINT32 Zone, IN gctCONST_STRING FuncName);

void
gcoOS_SysTraceEnd(IN gctUINT32 Zone);

#if defined(LINUX) && gcdSYSTRACE
# define gcmSYSTRACE_BEGIN       gcoOS_SysTraceBegin
# define gcmSYSTRACE_END         gcoOS_SysTraceEnd
#else
# define gcmSYSTRACE_BEGIN(...)
# define gcmSYSTRACE_END(...)
#endif

/******************************************************************************
 ******************************** Logging Macros ******************************
 ******************************************************************************/

#define gcdHEADER_LEVEL gcvLEVEL_VERBOSE

/* Always enable header/footer when systrace build is on */
#if defined(LINUX) && gcdSYSTRACE
# undef gcdEMPTY_HEADER_FOOTER
#endif

#ifndef gcdEMPTY_HEADER_FOOTER
# define gcdEMPTY_HEADER_FOOTER 0
#endif

#if gcdENABLE_PROFILING
void
gcoOS_ProfileDB(IN gctCONST_STRING Function, IN OUT gctBOOL_PTR Initialized);

#define gcmHEADER() \
    gctINT8 __user__ = 1; \
    static gctBOOL __profile__initialized__ = gcvFALSE; \
    gcmSTACK_PUSH(&__user__, __FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcoOS_ProfileDB(__FUNCTION__, &__profile__initialized__)

#define gcmHEADER_ARG(...) \
    gctINT8 __user__ = 1; \
    static gctBOOL __profile__initialized__ = gcvFALSE; \
    gcmSTACK_PUSH(&__user__, __FUNCTION__, __LINE__, Text, __VA_ARGS__); \
    gcoOS_ProfileDB(__FUNCTION__, &__profile__initialized__)

#define gcmFOOTER() \
    gcmSTACK_POP(&__user__, __FUNCTION__); \
    gcoOS_ProfileDB(__FUNCTION__, gcvNULL)

#define gcmFOOTER_NO() \
    gcmSTACK_POP(&__user__, __FUNCTION__); \
    gcoOS_ProfileDB(__FUNCTION__, gcvNULL)

#define gcmFOOTER_ARG(...) \
    gcmSTACK_POP(&__user__, __FUNCTION__); \
    gcoOS_ProfileDB(__FUNCTION__, gcvNULL)

#define gcmFOOTER_KILL() \
    gcmSTACK_POP(&__user__, __FUNCTION__); \
    gcoOS_ProfileDB(gcvNULL, gcvNULL)

#else /* !gcdENABLE_PROFILING */

#if gcdEMPTY_HEADER_FOOTER
#   define gcmHEADER()
#else
#define gcmHEADER() \
    gctINT8 __user__ = 1; \
    gctINT8_PTR __user_ptr__ = &__user__; \
    gcmSTACK_PUSH(__user_ptr__, __FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcmSYSTRACE_BEGIN(_GC_OBJ_ZONE, __FUNCTION__); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "++%s(%d)", __FUNCTION__, __LINE__)
#endif

#if gcdEMPTY_HEADER_FOOTER
# define gcmHEADER_ARG(Text, ...)
#else
# define gcmHEADER_ARG(Text, ...) \
    gctINT8 __user__ = 1; \
    gctINT8_PTR __user_ptr__ = &__user__; \
    gcmSTACK_PUSH(__user_ptr__, __FUNCTION__, __LINE__, Text, __VA_ARGS__); \
    gcmSYSTRACE_BEGIN(_GC_OBJ_ZONE, __FUNCTION__); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, Text, __VA_ARGS__); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "++%s(%d): " Text, __FUNCTION__, __LINE__, __VA_ARGS__)
#endif

#if gcdEMPTY_HEADER_FOOTER
# define gcmFOOTER()
#else
# define gcmFOOTER() \
    gcmSTACK_POP(__user_ptr__, __FUNCTION__); \
    gcmSYSTRACE_END(_GC_OBJ_ZONE); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "--%s(%d): status=%d(%s)", \
                  __FUNCTION__, __LINE__, \
                  status, gcoOS_DebugStatus2Name(status)); \
    *__user_ptr__ -= 1
#endif

#if gcdEMPTY_HEADER_FOOTER
# define gcmFOOTER_NO()
#else
#define gcmFOOTER_NO() \
    gcmSTACK_POP(__user_ptr__, __FUNCTION__); \
    gcmSYSTRACE_END(_GC_OBJ_ZONE); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "--%s(%d)", __FUNCTION__, __LINE__); \
    *__user_ptr__ -= 1
#endif

#if gcdEMPTY_HEADER_FOOTER
# define gcmFOOTER_KILL()
#else
#define gcmFOOTER_KILL() \
    gcmSTACK_POP(__user_ptr__, __FUNCTION__); \
    gcmSYSTRACE_END(_GC_OBJ_ZONE); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "--%s(%d)", __FUNCTION__, __LINE__); \
    *__user_ptr__ -= 1
#endif

#if gcdEMPTY_HEADER_FOOTER
# define gcmFOOTER_ARG(Text, ...)
#else
# define gcmFOOTER_ARG(Text, ...) \
    gcmSTACK_POP(__user_ptr__, __FUNCTION__); \
    gcmSYSTRACE_END(_GC_OBJ_ZONE); \
    gcmBINARY_TRACE(__FUNCTION__, __LINE__, Text, __VA_ARGS__); \
    gcmTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE, \
                  "--%s(%d): " Text, __FUNCTION__, __LINE__, __VA_ARGS__); \
    *__user_ptr__ -= 1
#endif

#endif /* gcdENABLE_PROFILING */

# define gcmkHEADER()                                               \
    do {                                                            \
        gcmkBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
        gcmkTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE,               \
                       "++%s(%d)", __FUNCTION__, __LINE__);         \
    } while (0)

# define gcmkHEADER_ARG(Text, ...)                                   \
    do {                                                             \
        gcmkBINARY_TRACE(__FUNCTION__, __LINE__, Text, __VA_ARGS__); \
        gcmkTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE,                \
                       "++%s(%d): " Text,                            \
                       __FUNCTION__, __LINE__, __VA_ARGS__);         \
    } while (0)

# define gcmkFOOTER()                                              \
    do {                                                           \
        gcmkBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, status); \
        gcmkTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE,              \
                       "--%s(%d): status=%d(%s)",                  \
                       __FUNCTION__, __LINE__, status,             \
                       gckOS_DebugStatus2Name(status));            \
    } while (0)

# define gcmkFOOTER_NO()                                            \
    do {                                                            \
        gcmkBINARY_TRACE(__FUNCTION__, __LINE__, gcvNULL, gcvNULL); \
        gcmkTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE,               \
                       "--%s(%d)", __FUNCTION__, __LINE__);         \
    } while (0)

# define gcmkFOOTER_ARG(Text, ...)                                   \
    do {                                                             \
        gcmkBINARY_TRACE(__FUNCTION__, __LINE__, Text, __VA_ARGS__); \
        gcmkTRACE_ZONE(gcdHEADER_LEVEL, _GC_OBJ_ZONE,                \
                       "--%s(%d): " Text,                            \
                       __FUNCTION__, __LINE__, __VA_ARGS__);         \
    } while (0)

#define gcmOPT_VALUE(ptr)              (((ptr) == gcvNULL) ? 0 : *(ptr))
#define gcmOPT_VALUE_INDEX(ptr, index) (((ptr) == gcvNULL) ? 0 : ptr[index])
#define gcmOPT_POINTER(ptr)            (((ptr) == gcvNULL) ? gcvNULL : *(ptr))
#define gcmOPT_STRING(ptr)             (((ptr) == gcvNULL) ? "(nil)" : (ptr))

void
gckOS_Print(IN gctCONST_STRING Message, ...) CHECK_PRINTF_FORMAT(1, 2);

void
gcoOS_Print(IN gctCONST_STRING Message, ...) CHECK_PRINTF_FORMAT(1, 2);

#define gcmPRINT                       gcoOS_Print
#define gcmkPRINT                      gckOS_Print
#define gcmkPRINT_N(ArgumentSize, ...) gckOS_Print(__VA_ARGS__)

# define gcmPRINT_ONCE(Text, ...)          \
    {                                      \
        static gctBOOL _once = gcvFALSE;   \
        if (!_once) {                      \
            gcmPRINT(Text, __VA_ARGS__);   \
            _once = gcvTRUE;               \
        }                                  \
    }


#if gcdFEATURE_SANITYCHECK
# define gcmFEATURE_CHECKED      gcmPRINT_ONCE
#else
# define gcmFEATURE_CHECKED(Text, ...)
#endif

#if gcdPRINT_VERSION
# define gcmPRINT_VERSION()          \
    do {                             \
        _gcmPRINT_VERSION(gcm);      \
        gcmSTACK_DUMP();             \
    } while (0)
# define gcmkPRINT_VERSION()     _gcmPRINT_VERSION(gcmk)
# define _gcmPRINT_VERSION(prefix)    \
        prefix##TRACE(gcvLEVEL_ERROR, "Phytium HAL version %s", gcvVERSION_STRING)
#else
# define gcmPRINT_VERSION()          \
    do {                             \
        gcmSTACK_DUMP();             \
    } while (gcvFALSE)
# define gcmkPRINT_VERSION()         \
    do {                             \
    } while (gcvFALSE)
#endif

void
gckOS_Dump(IN gckOS Os, IN gctCONST_STRING Format, ...);

void
gckOS_DumpBuffer(IN gckOS Os,
                 IN gceDUMP_BUFFER_TYPE Type,
                 IN gctPOINTER Buffer,
                 IN gctUINT64 Address,
                 IN gctSIZE_T Size);

#if gcdDUMP_IN_KERNEL
# define gcmkDUMP            gckOS_Dump

# define gcmkDUMP_BUFFER     gckOS_DumpBuffer
#else
# define gcmkDUMP(...)               \
    do {                             \
    } while (0)
# define gcmkDUMP_BUFFER(...)        \
    do {                             \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmDUMP_FRAMERATE
 **
 **      Print average frame rate
 **
 */
gceSTATUS
gcoOS_DumpFrameRate(void);
#define gcmDUMP_FRAMERATE       gcoOS_DumpFrameRate

/*******************************************************************************
 **
 **  gcoOS_SetDumpFlag
 **
 **      Dump print switch.
 **
 **  ARGUMENTS:
 **
 **      DumpState
 **          True to enable dump prints.
 */

gceSTATUS
gcoOS_SetDumpFlag(IN gctBOOL DumpState);

/*******************************************************************************
 **
 **  gcmDUMP
 **
 **      Print a dump message.
 **
 **  ARGUMENTS:
 **
 **      gctSTRING   Message.
 **
 **      ...         Optional arguments.
 */

#if gcdDUMP
gceSTATUS
gcoOS_Dump(IN gcoOS Os, IN gctCONST_STRING String, ...);
# define gcmDUMP     gcoOS_Dump
#else
# define gcmDUMP(...)              \
    do {                           \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmDUMP_BUFFER
 **
 **      Print a buffer to the dump.
 **
 **  ARGUMENTS:
 **
 **      gctSTRING Tag
 **          Tag for dump.
 **
 **      gctADDRESS Address
 **          GPU address of buffer.
 **
 **      gctPOINTER Logical
 **          Logical address of buffer.
 **
 **      gctUINT32 Offset
 **          Offset into buffer.
 **
 **      gctSIZE_T Bytes
 **          Number of bytes.
 */

#if gcdDUMP
gceSTATUS
gcoOS_DumpBuffer(IN gcoOS Os,
                 IN gceDUMP_BUFFER_TYPE Type,
                 IN gctADDRESS Address,
                 IN gctPOINTER Logical,
                 IN gctSIZE_T Offset,
                 IN gctSIZE_T Bytes);

gceSTATUS
gcoOS_DumpZeroBuffer(IN gcoOS Os,
    IN gceDUMP_BUFFER_TYPE Type,
    IN gctADDRESS Address,
    IN gctSIZE_T Offset,
    IN gctSIZE_T Bytes);

# define gcmDUMP_BUFFER      gcoOS_DumpBuffer
# define gcmDUMP_ZERO_BUFFER gcoOS_DumpZeroBuffer
#else
# define gcmDUMP_BUFFER(...)     \
    do {                         \
    } while (0)

# define gcmDUMP_ZERO_BUFFER(...) \
    do {                          \
    } while (0)
#endif

#if gcdDUMP
void
gcoOS_DumpLock(void);
# define gcmDUMP_LOCK        gcoOS_DumpLock
#else
# define gcmDUMP_LOCK(...)          \
    do {                            \
    } while (0)
#endif

#if gcdDUMP
void
gcoOS_DumpUnlock(void);
# define gcmDUMP_UNLOCK      gcoOS_DumpUnlock
#else
# define gcmDUMP_UNLOCK(...)        \
    do {                            \
    } while (0)
#endif

#if gcdDUMP_DX
void
gcoOS_FlushDumpFile(void);
#endif

/*******************************************************************************
 **
 **  gcmDUMP_API
 **
 **      Print a dump message for a high level API prefixed by the function name.
 **
 **  ARGUMENTS:
 **
 **      gctSTRING   Message.
 **
 **      ...         Optional arguments.
 */
gceSTATUS
gcoOS_DumpApi(IN gctCONST_STRING String, ...);
#if gcdDUMP_API
# define gcmDUMP_API         gcoOS_DumpApi
#else
# define gcmDUMP_API(...)           \
    do {                            \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmDUMP_API_ARRAY
 **
 **      Print an array of data.
 **
 **  ARGUMENTS:
 **
 **      gctUINT32_PTR   Pointer to array.
 **      gctUINT32       Size.
 */
gceSTATUS
gcoOS_DumpArray(IN gctCONST_POINTER Data, IN gctUINT32 Size);
#if gcdDUMP_API
# define gcmDUMP_API_ARRAY       gcoOS_DumpArray
#else
# define gcmDUMP_API_ARRAY(...)     \
    do {                            \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmDUMP_API_ARRAY_TOKEN
 **
 **      Print an array of data terminated by a token.
 **
 **  ARGUMENTS:
 **
 **      gctUINT32_PTR   Pointer to array.
 **      gctUINT32       Termination.
 */
gceSTATUS
gcoOS_DumpArrayToken(IN gctCONST_POINTER Data, IN gctUINT32 Termination);
#if gcdDUMP_API
# define gcmDUMP_API_ARRAY_TOKEN         gcoOS_DumpArrayToken
#else
# define gcmDUMP_API_ARRAY_TOKEN(...) \
    do {                              \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmDUMP_API_DATA
 **
 **      Print an array of bytes.
 **
 **  ARGUMENTS:
 **
 **      gctCONST_POINTER    Pointer to array.
 **      gctSIZE_T           Size.
 */
gceSTATUS
gcoOS_DumpApiData(IN gctCONST_POINTER Data, IN gctSIZE_T Size);
#if gcdDUMP_API
# define gcmDUMP_API_DATA        gcoOS_DumpApiData
#else
# define gcmDUMP_API_DATA(...)       \
    do {                             \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmTRACE_RELEASE
 **
 **      Print a message to the shader debugger.
 **
 **  ARGUMENTS:
 **
 **      gctCONST_STRING  Message.
 **      ...              Optional arguments.
 */

#define gcmTRACE_RELEASE        gcoOS_DebugShaderTrace

void
gcoOS_DebugShaderTrace(IN gctCONST_STRING Message, ...);

void
gcoOS_SetDebugShaderFiles(IN gctCONST_STRING VSFileName, IN gctCONST_STRING FSFileName);

void
gcoOS_SetDebugShaderFileType(IN gctUINT32 ShaderType);

void
gcoOS_EnableDebugBuffer(IN gctBOOL Enable);

/*******************************************************************************
 **
 **  gcmBREAK
 **
 **      Break into the debugger.  In retail mode this macro does nothing.
 **
 **  ARGUMENTS:
 **
 **      None.
 */

void
gcoOS_DebugBreak(void);

void
gckOS_DebugBreak(void);

#if gcmIS_DEBUG(gcdDEBUG_BREAK)
# define gcmBREAK        gcoOS_DebugBreak
# define gcmkBREAK       gckOS_DebugBreak
#else
# define gcmBREAK()
# define gcmkBREAK()
#endif

/*******************************************************************************
 **
 **  gcmASSERT
 **
 **      Evaluate an expression and break into the debugger if the expression
 **      evaluates to false.  In retail mode this macro does nothing.
 **
 **  ARGUMENTS:
 **
 **      exp     Expression to evaluate.
 */
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
# define _gcmASSERT(prefix, exp) \
    do \
    { \
        if (!(exp)) \
        { \
            prefix##TRACE(gcvLEVEL_ERROR, \
                          #prefix "ASSERT at %s(%d)", \
                          __FUNCTION__, __LINE__); \
            prefix##TRACE(gcvLEVEL_ERROR, \
                          "(%s)", #exp); \
            prefix##BREAK(); \
        } \
    } \
    while (gcvFALSE)
# define gcmASSERT(exp)          _gcmASSERT(gcm, exp)
# define gcmkASSERT(exp)         _gcmASSERT(gcmk, exp)
#else
# define gcmASSERT(exp)
# define gcmkASSERT(exp)
#endif

/*******************************************************************************
 **
 **  gcmSTATIC_ASSERT
 **
 **      Tests a software assertion at compile time. If the specific constant
 **      expression is false, the compiler displays the specified message and
 **      the compilation fails with an error, otherwise the declaration has
 **      no effect.
 **      Static assert is suitable for both user and kernel space.
 **
 **  ARGUMENTS:
 **
 **      exp     Constant expression.
 **      message Error message displayed on assertion failure.
 */
#if defined(__GNUC__) && ((__GNUC__ == 4 && __GNUC_MINOR__ >= 6) || (__GNUC__ > 4))
# define gcmSTATIC_ASSERT(constExp, message) \
    do {                                     \
        _Static_assert((constExp), message); \
    }                                        \
    while (0)

#elif defined(_MSC_VER) && (_MSC_VER >= 1600)
# define gcmSTATIC_ASSERT(constExp, message) \
        static_assert((constExp), message)

#else
# define gcmSTATIC_ASSERT(constExp, message) \
    do {                                     \
    } while (0)
#endif

/*******************************************************************************
 **
 **  gcmVERIFY
 **
 **      Verify if an expression returns true.  If the expression does not
 **      evaluates to true, an assertion will happen in debug mode.
 **
 **  ARGUMENTS:
 **
 **      exp     Expression to evaluate.
 */
#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
# define gcmVERIFY(exp)          gcmASSERT(exp)
# define gcmkVERIFY(exp)         gcmkASSERT(exp)
#else
# define gcmVERIFY(exp)          ((void)exp)
# define gcmkVERIFY(exp)         ((void)exp)
#endif

/*******************************************************************************
 **
 **  gcmVERIFY_OK
 **
 **      Verify a function returns gcvSTATUS_OK.  If the function does not return
 **      gcvSTATUS_OK, an assertion will happen in debug mode.
 **
 **  ARGUMENTS:
 **
 **      func    Function to evaluate.
 */

void
gcoOS_Verify(IN gceSTATUS status);

void
gckOS_Verify(IN gceSTATUS status);

#if gcmIS_DEBUG(gcdDEBUG_ASSERT)
# define gcmVERIFY_OK(func)                                     \
    do {                                                        \
        gceSTATUS verifyStatus = func;                          \
        gcoOS_Verify(verifyStatus);                             \
        if (verifyStatus != gcvSTATUS_OK) {                     \
            gcmTRACE(gcvLEVEL_ERROR,                            \
                     "gcmVERIFY_OK(%d): function returned %d",  \
                     __LINE__, verifyStatus);                   \
        }                                                       \
        gcmASSERT(verifyStatus == gcvSTATUS_OK);                \
    } while (gcvFALSE)

# define gcmkVERIFY_OK(func)                                    \
    do {                                                        \
        gceSTATUS verifyStatus = func;                          \
        if (verifyStatus != gcvSTATUS_OK) {                     \
            gcmkTRACE(gcvLEVEL_ERROR,                           \
                      "gcmkVERIFY_OK(%d): function returned %d",\
                      __LINE__, verifyStatus);                  \
        }                                                       \
        gckOS_Verify(verifyStatus);                             \
        gcmkASSERT(verifyStatus == gcvSTATUS_OK);               \
    } while (gcvFALSE)
#else
# define gcmVERIFY_OK(func)          func
# define gcmkVERIFY_OK(func)         func
#endif

gctCONST_STRING
gcoOS_DebugStatus2Name(gceSTATUS status);

gctCONST_STRING
gckOS_DebugStatus2Name(gceSTATUS status);

/*******************************************************************************
 **
 **  gcmERR_BREAK
 **
 **      Executes a break statement on error.
 **
 **  ASSUMPTIONS:
 **
 **      'status' variable of gceSTATUS type must be defined.
 **
 **  ARGUMENTS:
 **
 **      func    Function to evaluate.
 */
#define _gcmERR_BREAK(prefix, func) {                              \
    status = func;                                                 \
    if (gcmIS_ERROR(status)) {                                     \
        prefix##PRINT_VERSION();                                   \
        prefix##TRACE(gcvLEVEL_ERROR,                              \
                      #prefix "ERR_BREAK: status=%d(%s) @ %s(%d)", \
                      status, gcoOS_DebugStatus2Name(status),      \
                      __FUNCTION__, __LINE__);                     \
        break;                                                     \
    }                                                              \
    do { } while (gcvFALSE);                                       \
}

#define _gcmkERR_BREAK(prefix, func) {                             \
    status = func;                                                 \
    if (gcmIS_ERROR(status)) {                                     \
        prefix##PRINT_VERSION();                                   \
        prefix##TRACE(gcvLEVEL_ERROR,                              \
                      #prefix "ERR_BREAK: status=%d(%s) @ %s(%d)", \
                      status, gckOS_DebugStatus2Name(status),      \
                      __FUNCTION__, __LINE__);                     \
        break;                                                     \
    }                                                              \
    do { } while (gcvFALSE);                                       \
}

#define gcmERR_BREAK(func)          _gcmERR_BREAK(gcm, func)
#define gcmkERR_BREAK(func)         _gcmkERR_BREAK(gcmk, func)

/*******************************************************************************
 **
 **  gcmERR_RETURN
 **
 **      Executes a return on error.
 **
 **  ASSUMPTIONS:
 **
 **      'status' variable of gceSTATUS type must be defined.
 **
 **  ARGUMENTS:
 **
 **      func    Function to evaluate.
 */
#define _gcmERR_RETURN(prefix, func)                                    \
    do {                                                                \
        status = func;                                                  \
        if (gcmIS_ERROR(status)) {                                      \
            prefix##PRINT_VERSION();                                    \
            prefix##TRACE(gcvLEVEL_ERROR,                               \
                          #prefix "ERR_RETURN: status=%d(%s) @ %s(%d)", \
                          status, gcoOS_DebugStatus2Name(status),       \
                          __FUNCTION__, __LINE__);                      \
            prefix##FOOTER();                                           \
            return status;                                              \
        }                                                               \
    } while (gcvFALSE)
#define _gcmkERR_RETURN(prefix, func)                                   \
    do {                                                                \
        status = func;                                                  \
        if (gcmIS_ERROR(status)) {                                      \
            prefix##PRINT_VERSION();                                    \
            prefix##TRACE(gcvLEVEL_ERROR,                               \
                          #prefix "ERR_RETURN: status=%d(%s) @ %s(%d)", \
                          status, gckOS_DebugStatus2Name(status),       \
                          __FUNCTION__, __LINE__);                      \
            prefix##FOOTER();                                           \
            return status;                                              \
        }                                                               \
    } while (gcvFALSE)
#define gcmERR_RETURN(func)         _gcmERR_RETURN(gcm, func)
#define gcmkERR_RETURN(func)        _gcmkERR_RETURN(gcmk, func)

/*******************************************************************************
 **
 **  gcmONERROR
 **
 **      Jump to the error handler in case there is an error.
 **
 **  ASSUMPTIONS:
 **
 **      'status' variable of gceSTATUS type must be defined.
 **
 **  ARGUMENTS:
 **
 **      func    Function to evaluate.
 */
#define _gcmONERROR(prefix, func)                                        \
    do {                                                                 \
        status = func;                                                   \
        if (gcmIS_ERROR(status)) {                                       \
            prefix##PRINT_VERSION();                                     \
            prefix##TRACE(gcvLEVEL_ERROR,                                \
                          #prefix "ONERROR: status=%d(%s) @ %s(%d)",     \
                          status, gcoOS_DebugStatus2Name(status),        \
                          __FUNCTION__, __LINE__);                       \
            goto OnError;                                                \
        }                                                                \
    } while (gcvFALSE)
#define _gcmkONERROR(prefix, func)                                       \
    do {                                                                 \
        status = func;                                                   \
        if (gcmIS_ERROR(status)) {                                       \
            prefix##PRINT_VERSION();                                     \
            prefix##TRACE(gcvLEVEL_ERROR,                                \
                          #prefix "ONERROR: status=%d(%s) @ %s(%d)",     \
                          status, gckOS_DebugStatus2Name(status),        \
                          __FUNCTION__, __LINE__);                       \
            goto OnError;                                                \
        }                                                                \
    } while (gcvFALSE)

/* Ignore the debug info when the specific error occurs. */
#define _gcmkONERROR_EX(prefix, func, error)                             \
    do {                                                                 \
        status = func;                                                   \
        if (gcmIS_ERROR(status)) {                                       \
            if (status != (error)) {                                     \
                prefix##PRINT_VERSION();                                 \
                prefix##TRACE(gcvLEVEL_ERROR,                            \
                              #prefix "ONERROR: status=%d(%s) @ %s(%d)", \
                              status, gckOS_DebugStatus2Name(status),    \
                              __FUNCTION__, __LINE__);                   \
            }                                                            \
            goto OnError;                                                \
        }                                                                \
    } while (gcvFALSE)

#define gcmONERROR(func)                    _gcmONERROR(gcm, func)
#define gcmkONERROR(func)                   _gcmkONERROR(gcmk, func)
#define gcmkONERROR_EX(func, error)         _gcmkONERROR_EX(gcmk, func, error)

#define gcmGET_INDEX_SIZE(type, size) \
    switch (type) \
    { \
    case gcvINDEX_8: \
        size = 1; \
        break; \
    case gcvINDEX_16: \
        size = 2; \
        break; \
    case gcvINDEX_32: \
        size = 4; \
        break; \
    default: \
        gcmONERROR(gcvSTATUS_INVALID_ARGUMENT); \
    } \

/*******************************************************************************
 **
 **  gcmkSAFECASTSIZET
 **
 **      Check whether value of a gctSIZE_T variable beyond the capability
 **      of 32bits GPU hardware.
 **
 **  ASSUMPTIONS:
 **
 **
 **
 **  ARGUMENTS:
 **
 **      x   A gctUINT32 variable
 **      y   A gctSIZE_T variable
 */
#define gcmkSAFECASTSIZET(x, y)                                   \
    do {                                                          \
        gctSIZE_T tmp = y;                                        \
        if (gcmSIZEOF(gctSIZE_T) > gcmSIZEOF(gctUINT32)) {        \
            gcmkASSERT(tmp <= gcvMAXUINT32);                      \
        }                                                         \
        (x) = (gctUINT32)tmp;                                     \
    } while (gcvFALSE)

#define gcmSAFECASTSIZET(x, y)                                    \
    do {                                                          \
        gctSIZE_T tmp = y;                                        \
        if (gcmSIZEOF(gctSIZE_T) > gcmSIZEOF(gctUINT32)) {        \
            gcmASSERT(tmp <= gcvMAXUINT32);                       \
        }                                                         \
        (x) = (gctUINT32)tmp;                                     \
    } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmkSAFECASTPHYSADDRT
 **
 **      Check whether value of a gctPHYS_ADDR_T variable beyond the capability
 **      of 32bits GPU hardware.
 **
 **  ASSUMPTIONS:
 **
 **
 **
 **  ARGUMENTS:
 **
 **      x   A gctUINT32 variable
 **      y   A gctPHYS_ADDR_T variable
 */
#define gcmkSAFECASTPHYSADDRT(x, y)                               \
    do {                                                          \
        gctPHYS_ADDR_T tmp = y;                                   \
        if (gcmSIZEOF(gctPHYS_ADDR_T) > gcmSIZEOF(gctUINT32)) {   \
            gcmkASSERT(tmp <= gcvMAXUINT32);                      \
        }                                                         \
        (x) = (gctUINT32)tmp;                                     \
    } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmSAFECASTPHYSADDRT
 **
 **      Check whether value of a gctPHYS_ADDR_T variable beyond the capability
 **      of 32bits GPU hardware.
 **
 **  ASSUMPTIONS:
 **
 **
 **
 **  ARGUMENTS:
 **
 **      x   A gctUINT32 variable
 **      y   A gctPHYS_ADDR_T variable
 */
#define gcmSAFECASTPHYSADDRT(x, y)                                \
    do {                                                          \
        gctPHYS_ADDR_T tmp = y;                                   \
        if (gcmSIZEOF(gctPHYS_ADDR_T) > gcmSIZEOF(gctUINT32)) {   \
            gcmASSERT(tmp <= gcvMAXUINT32);                       \
        }                                                         \
        (x) = (gctUINT32)tmp;                                     \
    } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmkSAFECASTVA
 **
 **      Check whether value of a gctADDRESS variable beyond the capability
 **      of 32bits GPU hardware.
 **
 **  ASSUMPTIONS:
 **
 **
 **
 **  ARGUMENTS:
 **
 **      x   A gctUINT32 variable
 **      y   A gctADDRESS variable
 */
#define gcmkSAFECASTVA(x, y)                                     \
    do {                                                         \
        gctADDRESS tmp = y;                                      \
        if (gcmSIZEOF(gctADDRESS) > gcmSIZEOF(gctUINT32)) {      \
            gcmkASSERT(tmp <= gcvMAXUINT32);                     \
        }                                                        \
        (x) = (gctUINT32)tmp;                                    \
    } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmSAFECASTVA
 **
 **      Check whether value of a gctADDRESS variable beyond the capability
 **      of 32bits GPU hardware.
 **
 **  ASSUMPTIONS:
 **
 **
 **
 **  ARGUMENTS:
 **
 **      x   A gctUINT32 variable
 **      y   A gctADDRESS variable
 */
#define gcmSAFECASTVA(x, y)                                      \
    do {                                                         \
        gctADDRESS tmp = y;                                      \
        if (gcmSIZEOF(gctADDRESS) > gcmSIZEOF(gctUINT32)) {      \
            gcmASSERT(tmp <= gcvMAXUINT32);                      \
        }                                                        \
        (x) = (gctUINT32)tmp;                                    \
    } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmVERIFY_LOCK
 **
 **      Verifies whether the surface is locked.
 **
 **  ARGUMENTS:
 **
 **      surfaceInfo Pointer to the surface iniformational structure.
 */
#define gcmVERIFY_LOCK(surfaceInfo) \
    if (!surfaceInfo->node.valid) \
    { \
        gcmONERROR(gcvSTATUS_MEMORY_UNLOCKED); \
    } \

/*******************************************************************************
 **
 **  gcmVERIFY_NODE_LOCK
 **
 **      Verifies whether the surface node is locked.
 **
 **  ARGUMENTS:
 **
 **      surfaceInfo Pointer to the surface iniformational structure.
 */
#define gcmVERIFY_NODE_LOCK(surfaceNode) \
    if (!(surfaceNode)->valid) \
    { \
        status = gcvSTATUS_MEMORY_UNLOCKED; \
        break; \
    } \
    do { } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmBADOBJECT_BREAK
 **
 **      Executes a break statement on bad object.
 **
 **  ARGUMENTS:
 **
 **      obj     Object to test.
 **      t       Expected type of the object.
 */
#define gcmBADOBJECT_BREAK(obj, t) \
    if ((obj == gcvNULL) \
    ||  (((gcsOBJECT *)(obj))->type != t) \
    ) \
    { \
        status = gcvSTATUS_INVALID_OBJECT; \
        break; \
    } \
    do { } while (gcvFALSE)

/*******************************************************************************
 **
 **  gcmCHECK_STATUS
 **
 **      Executes a break statement on error.
 **
 **  ASSUMPTIONS:
 **
 **      'status' variable of gceSTATUS type must be defined.
 **
 **  ARGUMENTS:
 **
 **      func    Function to evaluate.
 */
#define _gcmCHECK_STATUS(prefix, func)                                    \
    do {                                                                  \
        last = func;                                                      \
        if (gcmIS_ERROR(last)) {                                          \
            prefix##TRACE(gcvLEVEL_ERROR,                                 \
                          #prefix "CHECK_STATUS: status=%d(%s) @ %s(%d)", \
                          last, gcoOS_DebugStatus2Name(last),             \
                          __FUNCTION__, __LINE__);                        \
            status = last;                                                \
        }                                                                 \
    } while (gcvFALSE)
#define _gcmkCHECK_STATUS(prefix, func)                                   \
    do {                                                                  \
        last = func;                                                      \
        if (gcmIS_ERROR(last)) {                                          \
            prefix##TRACE(gcvLEVEL_ERROR,                                 \
                          #prefix "CHECK_STATUS: status=%d(%s) @ %s(%d)", \
                          last, gckOS_DebugStatus2Name(last),             \
                          __FUNCTION__, __LINE__);                        \
            status = last;                                                \
        }                                                                 \
    } while (gcvFALSE)
#define gcmCHECK_STATUS(func)       _gcmCHECK_STATUS(gcm, func)
#define gcmkCHECK_STATUS(func)      _gcmkCHECK_STATUS(gcmk, func)

/*******************************************************************************
 **
 **  gcmVERIFY_ARGUMENT
 **
 **      Assert if an argument does not apply to the specified expression.  If
 **      the argument evaluates to false, gcvSTATUS_INVALID_ARGUMENT will be
 **      returned from the current function.  In retail mode this macro does
 **      nothing.
 **
 **  ARGUMENTS:
 **
 **      arg     Argument to evaluate.
 */
#define _gcmVERIFY_ARGUMENT(prefix, arg) \
       do \
       { \
           if (!(arg)) \
           { \
               prefix##TRACE(gcvLEVEL_ERROR, #prefix "VERIFY_ARGUMENT failed:"); \
               prefix##ASSERT(arg); \
               prefix##FOOTER_ARG("status=%d", gcvSTATUS_INVALID_ARGUMENT); \
               return gcvSTATUS_INVALID_ARGUMENT; \
           } \
       } \
       while (gcvFALSE)
#define gcmVERIFY_ARGUMENT(arg)             _gcmVERIFY_ARGUMENT(gcm, arg)
#define gcmkVERIFY_ARGUMENT(arg)            _gcmVERIFY_ARGUMENT(gcmk, arg)

/*******************************************************************************
 **
 **  gcmDEBUG_VERIFY_ARGUMENT
 **
 **      Works just like gcmVERIFY_ARGUMENT, but is only valid in debug mode.
 **      Use this to verify arguments inside non-public API functions.
 */
#if gcdDEBUG
# define gcmDEBUG_VERIFY_ARGUMENT(arg)   _gcmVERIFY_ARGUMENT(gcm, arg)
# define gcmkDEBUG_VERIFY_ARGUMENT(arg)  _gcmkVERIFY_ARGUMENT(gcm, arg)
#else
# define gcmDEBUG_VERIFY_ARGUMENT(arg)
# define gcmkDEBUG_VERIFY_ARGUMENT(arg)
#endif

/*******************************************************************************
 **
 **  _gcmCHECK_ADD_OVERFLOW
 */
#define _gcmCHECK_ADD_OVERFLOW(x, y)                                                            \
(                                                                                               \
    ((x) > 0 && (y) > 0 && gcvMAXSIZE_T - (x) < (y)) ? gcvSTATUS_RESLUT_OVERFLOW : gcvSTATUS_OK \
)

#define gcmCHECK_ADD_OVERFLOW(x, y)                 _gcmCHECK_ADD_OVERFLOW(x, y)
#define gcmkCHECK_ADD_OVERFLOW(x, y)                _gcmCHECK_ADD_OVERFLOW(x, y)

#define MAX_LOOP_COUNT 0x7FFFFFFF

/******************************************************************************
 ****************************** User Debug Option *****************************
 ******************************************************************************/

typedef struct _gcsUSER_DEBUG_OPTION {
    gceDEBUG_MSG debugMsg;
} gcsUSER_DEBUG_OPTION;

gcsUSER_DEBUG_OPTION *
gcoHAL_GetUserDebugOption(void);

# define gcmUSER_DEBUG_MSG(level, ...) \
    do \
    { \
        if (level <= gcoHAL_GetUserDebugOption()->debugMsg) \
        { \
            gcoOS_Print(__VA_ARGS__); \
        } \
    } while (gcvFALSE)

# define gcmUSER_DEBUG_ERROR_MSG(...)   gcmUSER_DEBUG_MSG(gcvDEBUG_MSG_ERROR, "Error: " __VA_ARGS__)
# define gcmUSER_DEBUG_WARNING_MSG(...) gcmUSER_DEBUG_MSG(gcvDEBUG_MSG_WARNING, "Warring: " __VA_ARGS__)

/*******************************************************************************
 **
 **  A set of macros to aid state loading.
 **
 **  ARGUMENTS:
 **
 **      CommandBuffer   Pointer to a gcoCMDBUF object.
 **      StateDelta      Pointer to a gcsSTATE_DELTA state delta structure.
 **      Memory          Destination memory pointer of gctUINT32_PTR type.
 **      PartOfContext   Whether or not the state is a part of the context.
 **      FixedPoint      Whether or not the state is of the fixed point format.
 **      Count           Number of consecutive states to be loaded.
 **      Address         State address.
 **      Data            Data to be set to the state.
 */

/*----------------------------------------------------------------------------*/

#if gcmIS_DEBUG(gcdDEBUG_CODE)

# define gcmSTORELOADSTATE(CommandBuffer, Memory, Address, Count) \
        CommandBuffer->lastLoadStatePtr = gcmPTR_TO_UINT64(Memory); \
        CommandBuffer->lastLoadStateAddress = Address; \
        CommandBuffer->lastLoadStateCount = Count

# define gcmVERIFYLOADSTATE(CommandBuffer, Memory, Address) \
        gcmASSERT( \
            (gctUINT) (Memory  - gcmUINT64_TO_TYPE(CommandBuffer->lastLoadStatePtr, gctUINT32_PTR) - 1) \
            == \
            (gctUINT) (Address - CommandBuffer->lastLoadStateAddress) \
            ); \
        \
        gcmASSERT(CommandBuffer->lastLoadStateCount > 0); \
        \
        CommandBuffer->lastLoadStateCount -= 1

# define gcmVERIFYLOADSTATEDONE(CommandBuffer) \
        gcmASSERT(CommandBuffer->lastLoadStateCount == 0);

# define gcmDEFINELOADSTATEBASE() \
        gctUINT32_PTR LoadStateBase;

# define gcmSETLOADSTATEBASE(CommandBuffer, OutSide) \
        if (OutSide) \
        {\
            LoadStateBase = (gctUINT32_PTR)*OutSide; \
        }\
        else\
        {\
            LoadStateBase = (gctUINT_PTR)CommandBuffer->buffer;\
        }


# define gcmVERIFYLOADSTATEALIGNED(CommandBuffer, Memory) \
        gcmASSERT(((Memory - LoadStateBase) & 1) == 0);

# define gcmUNSETLOADSTATEBASE() \
        LoadStateBase = LoadStateBase;

#else

# define gcmSTORELOADSTATE(CommandBuffer, Memory, Address, Count)
# define gcmVERIFYLOADSTATE(CommandBuffer, Memory, Address)
# define gcmVERIFYLOADSTATEDONE(CommandBuffer)

# define gcmDEFINELOADSTATEBASE()
# define gcmSETLOADSTATEBASE(CommandBuffer, OutSide)
# define gcmVERIFYLOADSTATEALIGNED(CommandBuffer, Memory)
# define gcmUNSETLOADSTATEBASE()

#endif

/*----------------------------------------------------------------------------*/

#define gcmDUMPSTATEDATA(StateDelta, FixedPoint, Address, Data)

#define gcmDEFINESTATEBUFFER(CommandBuffer, StateDelta, Memory, ReserveSize) \
    gctSIZE_T ReserveSize;                                                   \
    gcoCMDBUF CommandBuffer;                                                 \
    gctUINT32_PTR Memory;                                                    \
    gcsSTATE_DELTA_PTR StateDelta;                                           \
    gceENGINE CurrentEngine = gcvENGINE_RENDER

#define gcmBEGINSTATEBUFFER(Hardware, CommandBuffer, StateDelta, Memory, ReserveSize) \
    {                                                                                 \
        gcmONERROR(gcoBUFFER_Reserve(Hardware->engine[CurrentEngine].buffer,          \
                                     ReserveSize, gcvTRUE,                            \
                                     gcvCOMMAND_3D, &CommandBuffer));                 \
                                                                                      \
        Memory = (gctUINT32_PTR)gcmUINT64_TO_PTR(CommandBuffer->lastReserve);         \
                                                                                      \
        StateDelta = Hardware->delta;                                                 \
    }

#define gcmENDSTATEBUFFER(Hardware, CommandBuffer, Memory, ReserveSize)                     \
    {                                                                                       \
        gcmASSERT(gcmUINT64_TO_TYPE(CommandBuffer->lastReserve, gctUINT8_PTR) + ReserveSize \
                  == (gctUINT8_PTR)Memory);                                                 \
    }

/*----------------------------------------------------------------------------*/

#define gcmBEGINSTATEBATCH(CommandBuffer, Memory, FixedPoint, Address, Count)                   \
    {                                                                                           \
        gcmASSERT(((Memory - gcmUINT64_TO_TYPE(CommandBuffer->lastReserve, gctUINT32_PTR)) & 1) \
                  == 0);                                                                        \
        gcmASSERT((gctUINT32)Count <= 1024);                                                    \
                                                                                                \
        gcmVERIFYLOADSTATEDONE(CommandBuffer);                                                  \
                                                                                                \
        gcmSTORELOADSTATE(CommandBuffer, Memory, Address, Count);                               \
                                                                                                \
        *Memory++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE, LOAD_STATE) |    \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, FLOAT, FixedPoint) |          \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT, Count) |               \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, Address);            \
    }

#define gcmENDSTATEBATCH(CommandBuffer, Memory)                                                 \
    {                                                                                           \
        gcmVERIFYLOADSTATEDONE(CommandBuffer);                                                  \
                                                                                                \
        gcmASSERT(((Memory - gcmUINT64_TO_TYPE(CommandBuffer->lastReserve, gctUINT32_PTR)) & 1) \
                  == 0);                                                                        \
    }

/*----------------------------------------------------------------------------*/

#define gcmSETSTATEDATA(StateDelta, CommandBuffer, Memory,     \
                        FixedPoint, Address, Data)             \
    {                                                          \
        gctUINT32 __temp_data32__;                             \
                                                               \
        gcmVERIFYLOADSTATE(CommandBuffer, Memory, Address);    \
                                                               \
        gcmSAFECASTSIZET(__temp_data32__, Data);               \
                                                               \
        *Memory++ = __temp_data32__;                           \
                                                               \
        gcoHARDWARE_UpdateDelta(StateDelta, Address,           \
                                0, __temp_data32__);           \
                                                               \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint,               \
                         Address, __temp_data32__);            \
    }

#define gcmSETSTATEDATAWITHMASK(StateDelta, CommandBuffer, Memory,  \
                                FixedPoint, Address, Mask, Data)    \
    {                                                               \
        gctUINT32 __temp_data32__;                                  \
                                                                    \
        gcmVERIFYLOADSTATE(CommandBuffer, Memory, Address);         \
                                                                    \
        gcmSAFECASTSIZET(__temp_data32__, Data);                    \
                                                                    \
        *Memory++ = __temp_data32__;                                \
                                                                    \
        gcoHARDWARE_UpdateDelta(StateDelta, Address,                \
                                Mask, __temp_data32__);             \
                                                                    \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint,                    \
                         Address, __temp_data32__);                 \
    }

#define gcmSETCTRLSTATE(StateDelta, CommandBuffer,                  \
                        Memory, Address, Data)                      \
    {                                                               \
        gctUINT32 __temp_data32__;                                  \
                                                                    \
        gcmVERIFYLOADSTATE(CommandBuffer, Memory, Address);         \
                                                                    \
        gcmSAFECASTSIZET(__temp_data32__, Data);                    \
                                                                    \
        *Memory++ = __temp_data32__;                                \
                                                                    \
        gcmDUMPSTATEDATA(StateDelta, gcvFALSE,                      \
                         Address, __temp_data32__);                 \
    }

#define gcmSETFILLER(CommandBuffer, Memory)                         \
    {                                                               \
        gcmVERIFYLOADSTATEDONE(CommandBuffer);                      \
                                                                    \
        *(gctUINT32_PTR)Memory = 0x18000000;                        \
        Memory += 1;                                                \
    }

/*----------------------------------------------------------------------------*/

#define gcmSETSINGLESTATE(StateDelta, CommandBuffer,                \
                          Memory, FixedPoint, Address, Data)        \
    {                                                               \
        gcmBEGINSTATEBATCH(CommandBuffer, Memory,                   \
                           FixedPoint, Address, 1);                 \
        gcmSETSTATEDATA(StateDelta, CommandBuffer, Memory,          \
                        FixedPoint, Address, Data);                 \
        gcmENDSTATEBATCH(CommandBuffer, Memory);                    \
    }

#define gcmSETSINGLESTATEWITHMASK(StateDelta, CommandBuffer, Memory,\
                                  FixedPoint, Address, Mask, Data)  \
                                                                    \
    {                                                               \
        gcmBEGINSTATEBATCH(CommandBuffer, Memory,                   \
                           FixedPoint, Address, 1);                 \
        gcmSETSTATEDATAWITHMASK(StateDelta, CommandBuffer, Memory,  \
                                FixedPoint, Address, Mask, Data);   \
                                                                    \
        gcmENDSTATEBATCH(CommandBuffer, Memory);                    \
    }

#define gcmSETSINGLECTRLSTATE(StateDelta, CommandBuffer,            \
                              Memory, FixedPoint, Address, Data)    \
    {                                                               \
        gcmBEGINSTATEBATCH(CommandBuffer, Memory, FixedPoint, Address, 1); \
        gcmSETCTRLSTATE(StateDelta, CommandBuffer, Memory, Address, Data); \
        gcmENDSTATEBATCH(CommandBuffer, Memory);                    \
    }

#define gcmSETSEMASTALLPIPE(StateDelta, CommandBuffer, Memory, Data)\
    {                                                               \
        gcmSETSINGLECTRLSTATE(StateDelta, CommandBuffer, Memory,    \
                              gcvFALSE, AQSemaphoreRegAddrs, Data); \
                                                                    \
        *Memory++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL); \
                                                                    \
        *Memory++ = Data;                                           \
                                                                    \
        gcmDUMP(gcvNULL, "#[stall 0x%08X 0x%08X]",                  \
                gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END), \
                gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE)); \
    }

/*******************************************************************************
 **
 **  gcmSETSTARTDECOMMAND
 **
 **      Form a START_DE command.
 **
 **  ARGUMENTS:
 **
 **      Memory          Destination memory pointer of gctUINT32_PTR type.
 **      Count           Number of the rectangles.
 */

#define gcmSETSTARTDECOMMAND(Memory, Count)                                              \
    {                                                                                    \
        *Memory++ = gcmSETFIELDVALUE(0, AQ_COMMAND_START_DE_COMMAND, OPCODE, START_DE) | \
                         gcmSETFIELD(0, AQ_COMMAND_START_DE_COMMAND, COUNT,  Count) |    \
                         gcmSETFIELD(0, AQ_COMMAND_START_DE_COMMAND, DATA_COUNT, 0);     \
                                                                                         \
        *Memory++ = 0xDEADDEED;                                                          \
    }

/*****************************************
 ** Temp command buffer macro
 */
#define gcmDEFINESTATEBUFFER_NEW(CommandBuffer, StateDelta, Memory) \
    gcmDEFINELOADSTATEBASE()                                        \
    gcsTEMPCMDBUF CommandBuffer = gcvNULL;                          \
    gctUINT32_PTR        Memory;                                    \
    gcsSTATE_DELTA_PTR   StateDelta;                                \
    gceENGINE            CurrentEngine = gcvENGINE_RENDER

#define gcmBEGINSTATEBUFFER_NEW(Hardware, CommandBuffer,            \
                                StateDelta, Memory, OutSide)        \
    {                                                               \
        if (OutSide) {                                              \
            Memory = (gctUINT32_PTR)*OutSide;                       \
        } else {                                                    \
            gcmONERROR(gcoBUFFER_StartTEMPCMDBUF(Hardware->engine[CurrentEngine].buffer, \
                                                 Hardware->engine[CurrentEngine].queue,  \
                                                 &CommandBuffer));                       \
                                                                                         \
            Memory = (gctUINT32_PTR)(CommandBuffer->buffer); \
        }                                                    \
        StateDelta = Hardware->tempDelta;                    \
                                                             \
        gcmSETLOADSTATEBASE(CommandBuffer, OutSide);         \
    }

#define gcmENDSTATEBUFFER_NEW(Hardware, CommandBuffer, Memory, OutSide)                  \
    {                                                                                    \
        if (OutSide) {                                                                   \
            *OutSide = Memory;                                                           \
        } else {                                                                         \
            CommandBuffer->currentByteSize =                                             \
                (gctUINT32)((gctUINT8_PTR)Memory - (gctUINT8_PTR)CommandBuffer->buffer); \
                                                                                         \
            gcmONERROR(gcoBUFFER_EndTEMPCMDBUF(                                          \
                Hardware->engine[CurrentEngine].buffer, gcvFALSE));                      \
            if (Hardware->constructType != gcvHARDWARE_2D) {                             \
                gcoHARDWARE_UpdateTempDelta(Hardware);                                   \
            }                                                                            \
        }                                                                                \
        gcmUNSETLOADSTATEBASE()                                                          \
    }

#define gcmDEFINECTRLSTATEBUFFER(CommandBuffer, Memory)                         \
    gcmDEFINELOADSTATEBASE()                                                    \
    gcsTEMPCMDBUF CommandBuffer = gcvNULL;                                      \
    gctUINT32_PTR Memory;                                                       \
    gceENGINE CurrentEngine = gcvENGINE_RENDER

#define gcmBEGINCTRLSTATEBUFFER(Hardware, CommandBuffer, Memory, OutSide)       \
{                                                                               \
    if (OutSide)                                                                \
    {                                                                           \
        Memory = (gctUINT32_PTR)*OutSide;                                       \
    }                                                                           \
    else                                                                        \
    {                                                                           \
        gcmONERROR(gcoBUFFER_StartTEMPCMDBUF(                                   \
            Hardware->engine[CurrentEngine].buffer,                             \
            Hardware->engine[CurrentEngine].queue, &CommandBuffer               \
            ));                                                                 \
                                                                                \
        Memory = (gctUINT32_PTR)(CommandBuffer->buffer);                        \
    }                                                                           \
    gcmSETLOADSTATEBASE(CommandBuffer,OutSide);                                 \
}

/*----------------------------------------------------------------------------*/

#define gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                     \
                               FixedPoint, Address, Count)                \
    {                                                                     \
        gcmVERIFYLOADSTATEALIGNED(CommandBuffer, Memory);                 \
        gcmASSERT((gctUINT32)Count <= 1024);                              \
                                                                          \
        *Memory++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |  \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, FLOAT,   FixedPoint) |  \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   Count) |       \
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, Address);      \
    }

#define gcmENDSTATEBATCH_NEW(CommandBuffer, Memory)  \
    gcmVERIFYLOADSTATEALIGNED(CommandBuffer, Memory)

/*----------------------------------------------------------------------------*/

#define gcmSETSTATEDATA_NEW(StateDelta, CommandBuffer, Memory,              \
                            FixedPoint, Address, Data)                      \
    {                                                                       \
        gctUINT32 __temp_data32__;                                          \
                                                                            \
        gcmSAFECASTSIZET(__temp_data32__, Data);                            \
                                                                            \
        *Memory++ = __temp_data32__;                                        \
                                                                            \
        gcoHARDWARE_UpdateDelta(StateDelta, Address, 0, __temp_data32__);   \
                                                                            \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint, Address, __temp_data32__); \
    }

#define gcmSETSTATEDATAWITHMASK_NEW(StateDelta, CommandBuffer, Memory,      \
                                    FixedPoint, Address, Mask, Data)        \
    {                                                                       \
        gctUINT32 __temp_data32__;                                          \
                                                                            \
        gcmSAFECASTSIZET(__temp_data32__, Data);                            \
                                                                            \
        *Memory++ = __temp_data32__;                                        \
                                                                            \
        gcoHARDWARE_UpdateDelta(StateDelta, Address, Mask, __temp_data32__);\
                                                                            \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint, Address, __temp_data32__); \
    }

#define gcmSETCTRLSTATE_NEW(StateDelta, CommandBuffer, Memory, Address, Data) \
    {                                                                         \
        gctUINT32 __temp_data32__;                                            \
                                                                              \
        gcmSAFECASTSIZET(__temp_data32__, Data);                              \
                                                                              \
        *Memory++ = __temp_data32__;                                          \
                                                                              \
        gcmDUMPSTATEDATA(StateDelta, gcvFALSE, Address, __temp_data32__);     \
    }

#define gcmSETFILLER_NEW(CommandBuffer, Memory)      \
    {                                                \
        *(gctUINT32_PTR)Memory = 0x18000000;         \
        Memory += 1;                                 \
    }

/*----------------------------------------------------------------------------*/

#define gcmSETSINGLESTATE_NEW(StateDelta, CommandBuffer, Memory,    \
                              FixedPoint, Address, Data)            \
    {                                                               \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,               \
                               FixedPoint, Address, 1);             \
        gcmSETSTATEDATA_NEW(StateDelta, CommandBuffer, Memory,      \
                            FixedPoint, Address, Data);             \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                \
    }

#define gcmSETSINGLESTATEWITHMASK_NEW(StateDelta, CommandBuffer, Memory,    \
                                      FixedPoint, Address, Mask, Data)      \
    {                                                                       \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                       \
                               FixedPoint, Address, 1);                     \
        gcmSETSTATEDATAWITHMASK_NEW(StateDelta, CommandBuffer, Memory,      \
                                    FixedPoint, Address, Mask, Data);       \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                        \
    }

#define gcmSETSINGLECTRLSTATE_NEW(StateDelta, CommandBuffer, Memory,    \
                                  FixedPoint, Address, Data)            \
    {                                                                   \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                   \
                               FixedPoint, Address, 1);                 \
        gcmSETCTRLSTATE_NEW(StateDelta, CommandBuffer,                  \
                            Memory, Address, Data);                     \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                    \
    }

#define gcmSETBLOCKCTRLSTATE_NEW(StateDelta, CommandBuffer, Memory,     \
                                 FixedPoint, Address, Data, Count)      \
    {                                                                   \
        gctUINT32 c;                                                    \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                   \
                               FixedPoint, Address, Count);             \
        for (c = 0; c < Count; c++) {                                   \
            gcmSETCTRLSTATE_NEW(StateDelta, CommandBuffer,              \
                                Memory, Address, Data);                 \
        }                                                               \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                    \
    }

#define gcmSETCTRLSTATES_NEW(StateDelta, CommandBuffer, Memory,         \
                             FixedPoint, Address, Data, Count)          \
    {                                                                   \
        gctUINT32 c;                                                    \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                   \
                               FixedPoint, Address, Count);             \
        for (c = 0; c < Count; c++) {                                   \
            gcmSETCTRLSTATE_NEW(StateDelta, CommandBuffer,              \
                                Memory, Address, Data[c]);              \
        }                                                               \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                    \
    }

#define gcmSETSEMASTALLPIPE_NEW(StateDelta, CommandBuffer, Memory, Data)\
    {                                                                   \
        gcmSETSINGLECTRLSTATE_NEW(StateDelta, CommandBuffer, Memory,    \
                                  gcvFALSE, AQSemaphoreRegAddrs, Data); \
                                                                        \
        *Memory++ = gcmSETFIELDVALUE(0, STALL_COMMAND, OPCODE, STALL);  \
                                                                        \
        *Memory++ = Data;                                               \
                                                                        \
        gcmDUMP(gcvNULL, "#[stall 0x%08X 0x%08X]",                      \
                gcmSETFIELDVALUE(0, AQ_SEMAPHORE, SOURCE, FRONT_END),   \
                gcmSETFIELDVALUE(0, AQ_SEMAPHORE, DESTINATION, PIXEL_ENGINE)); \
    }

#define gcmSETSTATEDATA_NEW_FAST(StateDelta, CommandBuffer, Memory,         \
                                 FixedPoint, Address, Data)                 \
    {                                                                       \
        gctUINT32 __temp_data32__;                                          \
                                                                            \
        gcmSAFECASTSIZET(__temp_data32__, Data);                            \
                                                                            \
        *Memory++ = __temp_data32__;                                        \
                                                                            \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint, Address, __temp_data32__); \
    }

#define gcmSETSTATEDATAWITHMASK_NEW_FAST(StateDelta, CommandBuffer, Memory, \
                                         FixedPoint, Address, Mask, Data)   \
    {                                                                       \
        gctUINT32 __temp_data32__;                                          \
                                                                            \
        __temp_data32__ = Data;                                             \
                                                                            \
        *Memory++ = __temp_data32__;                                        \
                                                                            \
        gcmDUMPSTATEDATA(StateDelta, FixedPoint, Address, __temp_data32__); \
    }

#define gcmSETSINGLESTATE_NEW_FAST(StateDelta, CommandBuffer, Memory,       \
                                   FixedPoint, Address, Data)               \
    {                                                                       \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory,                       \
                               FixedPoint, Address, 1);                     \
        gcmSETSTATEDATA_NEW_FAST(StateDelta, CommandBuffer, Memory,         \
                                 FixedPoint, Address, Data);                \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                        \
    }

#define gcmSETSINGLESTATEWITHMASK_NEW_FAST(StateDelta, CommandBuffer, Memory,  \
                                           FixedPoint, Address, Mask, Data)    \
    {                                                                          \
        gcmBEGINSTATEBATCH_NEW(CommandBuffer, Memory, FixedPoint, Address, 1); \
        gcmSETSTATEDATAWITHMASK_NEW_FAST(StateDelta, CommandBuffer, Memory,    \
                                         FixedPoint, Address, Mask, Data);     \
        gcmENDSTATEBATCH_NEW(CommandBuffer, Memory);                           \
    }

#define gcmDEFINESTATEBUFFER_NEW_FAST(CommandBuffer, Memory)                \
    gcmDEFINELOADSTATEBASE()                                                \
    gcsTEMPCMDBUF CommandBuffer = gcvNULL;                                  \
    gctUINT32_PTR Memory;

#define gcmBEGINSTATEBUFFER_NEW_FAST(Hardware, CommandBuffer, Memory, OutSide)      \
{                                                                                   \
    if (OutSide) {                                                                  \
        Memory = (gctUINT32_PTR)*OutSide;                                           \
    } else {                                                                        \
        gcmONERROR(gcoBUFFER_StartTEMPCMDBUF(Hardware->engine[gcvENGINE_RENDER].buffer, \
                                             Hardware->engine[gcvENGINE_RENDER].queue,  \
                                             &CommandBuffer));                      \
                                                                                    \
        Memory = (gctUINT32_PTR)(CommandBuffer->buffer);                            \
                                                                                    \
    }                                                                               \
                                                                                    \
    gcmSETLOADSTATEBASE(CommandBuffer,OutSide);                                     \
}

#define gcmENDSTATEBUFFER_NEW_FAST(Hardware, CommandBuffer, Memory, OutSide)             \
    {                                                                                    \
        if (OutSide) {                                                                   \
            *OutSide = Memory;                                                           \
        } else {                                                                         \
            CommandBuffer->currentByteSize =                                             \
                (gctUINT32)((gctUINT8_PTR)Memory - (gctUINT8_PTR)CommandBuffer->buffer); \
                                                                                         \
            gcmONERROR(gcoBUFFER_EndTEMPCMDBUF(                                          \
                Hardware->engine[gcvENGINE_RENDER].buffer, gcvFALSE));                   \
        }                                                                                \
        gcmUNSETLOADSTATEBASE()                                                          \
    }

/*******************************************************************************
 **
 **  gcmCONFIGUREUNIFORMS
 **
 **      Configure uniforms according to chip and numConstants.
 */
#if !gcdENABLE_UNIFIED_CONSTANT
# define gcmCONFIGUREUNIFORMS(ChipModel, ChipRevision, NumConstants,         \
                              UnifiedConst, VsConstBase, PsConstBase,        \
                              VsConstMax, PsConstMax, ConstMax)              \
        {                                                                    \
            if (ChipModel == gcv2000 &&                                      \
                (ChipRevision == 0x5118 || ChipRevision == 0x5140)) {        \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 256;                                            \
                PsConstMax = 64;                                             \
                ConstMax = 320;                                              \
            } else if (NumConstants == 320) {                                \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 256;                                            \
                PsConstMax = 64;                                             \
                ConstMax = 320;                                              \
            } /* All GC1000 series chips can only support 64 uniforms for ps on non-unified const mode. */ \
            else if (NumConstants > 256 && ChipModel == gcv1000) {           \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 256;                                            \
                PsConstMax = 64;                                             \
                ConstMax = 320;                                              \
            } else if (NumConstants > 256) {                                 \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 256;                                            \
                PsConstMax = 256;                                            \
                ConstMax = 512;                                              \
            } else if (NumConstants == 256) {                                \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 256;                                            \
                PsConstMax = 256;                                            \
                ConstMax = 512;                                              \
            } else {                                                         \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 168;                                            \
                PsConstMax = 64;                                             \
                ConstMax = 232;                                              \
            }                                                                \
        }
#else
# define gcmCONFIGUREUNIFORMS(ChipModel, ChipRevision, Halti5Avail,          \
                              SmallBatch, ComputeOnly, NumConstants,         \
                              UnifiedConst, VsConstBase, PsConstBase,        \
                              VsConstMax, PsConstMax, ConstMax)              \
        {                                                                    \
            if (NumConstants > 256) {                                        \
                UnifiedConst = gcvTRUE;                                      \
                if (SmallBatch) {                                            \
                    VsConstBase = gcregGpipeUniformsRegAddrs;                \
                    PsConstBase = gcregGpipeUniformsRegAddrs;                \
                } else if (Halti5Avail) {                                    \
                    VsConstBase = gcregGpipeUniformsRegAddrs;                \
                    PsConstBase = gcregPixelUniformsRegAddrs;                \
                } else {                                                     \
                    VsConstBase = gcregSHUniformsRegAddrs;                   \
                    PsConstBase = gcregSHUniformsRegAddrs;                   \
                }                                                            \
                if ((ChipModel == gcv880) &&                                 \
                    ((ChipRevision & 0xfff0) == 0x5120)) {                   \
                    VsConstMax = 512;                                        \
                    PsConstMax = 64;                                         \
                    ConstMax = 576;                                          \
                } else {                                                     \
                    VsConstMax = gcmMIN(512, NumConstants - 64);             \
                    PsConstMax = gcmMIN(512, NumConstants - 64);             \
                    ConstMax = NumConstants;                                 \
                }                                                            \
            } else if (NumConstants == 256) {                                \
                if (ChipModel == gcv2000 &&                                  \
                    (ChipRevision == 0x5118 || ChipRevision == 0x5140)) {    \
                    UnifiedConst = gcvFALSE;                                 \
                    VsConstBase = AQVertexShaderConstRegAddrs;               \
                    PsConstBase = AQPixelShaderConstRegAddrs;                \
                    VsConstMax = 256;                                        \
                    PsConstMax = 64;                                         \
                    ConstMax = 320;                                          \
                } else {                                                     \
                    UnifiedConst = gcvFALSE;                                 \
                    VsConstBase = AQVertexShaderConstRegAddrs;               \
                    PsConstBase = AQPixelShaderConstRegAddrs;                \
                    VsConstMax = 256;                                        \
                    PsConstMax = 256;                                        \
                    ConstMax = 512;                                          \
                }                                                            \
            } else if (NumConstants == 160 && ComputeOnly) {                 \
                UnifiedConst = gcvTRUE;                                      \
                VsConstBase = gcregGpipeUniformsRegAddrs;                    \
                PsConstBase = gcregPixelUniformsRegAddrs;                    \
                VsConstMax = 0;                                              \
                PsConstMax = 160;                                            \
                ConstMax = 160;                                              \
            } else {                                                         \
                UnifiedConst = gcvFALSE;                                     \
                VsConstBase = AQVertexShaderConstRegAddrs;                   \
                PsConstBase = AQPixelShaderConstRegAddrs;                    \
                VsConstMax = 168;                                            \
                PsConstMax = 64;                                             \
                ConstMax = 232;                                              \
            }                                                                \
        }
#endif

/*******************************************************************************
 **
 **  gcmCONFIGUREUNIFORMS2
 **  only fix clang build error
 **
 **      Configure uniforms according to chip and numConstants.
 */
#if !gcdENABLE_UNIFIED_CONSTANT
#define gcmCONFIGUREUNIFORMS2(ChipModel, ChipRevision, NumConstants, \
             UnifiedConst, VsConstMax, PsConstMax) \
{ \
    if (ChipModel == gcv2000 && (ChipRevision == 0x5118 || ChipRevision == 0x5140)) \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 256; \
        PsConstMax   = 64; \
    } \
    else if (NumConstants == 320) \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 256; \
        PsConstMax   = 64; \
    } \
    /* All GC1000 series chips can only support 64 uniforms for ps on non-unified const mode. */ \
    else if (NumConstants > 256 && ChipModel == gcv1000) \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 256; \
        PsConstMax   = 64; \
    } \
    else if (NumConstants > 256) \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 256; \
        PsConstMax   = 256; \
    } \
    else if (NumConstants == 256) \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 256; \
        PsConstMax   = 256; \
    } \
    else \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 168; \
        PsConstMax   = 64; \
    } \
}
#else
#define gcmCONFIGUREUNIFORMS2(ChipModel, ChipRevision, Halti5Avail, SmallBatch, ComputeOnly, NumConstants, \
             UnifiedConst, VsConstMax, PsConstMax) \
{ \
    if (NumConstants > 256) \
    { \
        UnifiedConst = gcvTRUE; \
        if ((ChipModel == gcv880) && ((ChipRevision & 0xfff0) == 0x5120)) \
        { \
            VsConstMax   = 512; \
            PsConstMax   = 64; \
        } \
        else \
        { \
            VsConstMax   = gcmMIN(512, NumConstants - 64); \
            PsConstMax   = gcmMIN(512, NumConstants - 64); \
        } \
    } \
    else if (NumConstants == 256) \
    { \
        if (ChipModel == gcv2000 && (ChipRevision == 0x5118 || ChipRevision == 0x5140)) \
        { \
            UnifiedConst = gcvFALSE; \
            VsConstMax   = 256; \
            PsConstMax   = 64; \
        } \
        else \
        { \
            UnifiedConst = gcvFALSE; \
            VsConstMax   = 256; \
            PsConstMax   = 256; \
        } \
    } \
    else if (NumConstants == 160 && ComputeOnly) \
    { \
        UnifiedConst = gcvTRUE; \
        VsConstMax   = 0; \
        PsConstMax   = 160; \
    } \
    else \
    { \
        UnifiedConst = gcvFALSE; \
        VsConstMax   = 168; \
        PsConstMax   = 64; \
    } \
}
#endif

#define gcmAnyTileStatusEnableForFullMultiSlice(SurfView, anyTsEnableForMultiSlice) \
    {                                                                               \
        gctUINT i = 0;                                                              \
        for (; i < (SurfView->surf->requestD); i++) {                               \
            if ((SurfView->surf->tileStatusNode.pool != gcvPOOL_UNKNOWN) &&         \
                (SurfView->surf->tileStatusDisabled[i] == gcvFALSE)) {              \
                *anyTsEnableForMultiSlice = gcvTRUE;                                \
                break;                                                              \
            }                                                                       \
        }                                                                           \
    }

#define gcmAnyTileStatusEnableForMultiSlice(SurfView, anyTsEnableForMultiSlice) \
    {                                                                           \
        gctUINT i = SurfView->firstSlice;                                       \
        for (; i < (SurfView->firstSlice + SurfView->numSlices); i++) {         \
            if ((SurfView->surf->tileStatusNode.pool != gcvPOOL_UNKNOWN) &&     \
                (SurfView->surf->tileStatusDisabled[i] == gcvFALSE)) {          \
                *anyTsEnableForMultiSlice = gcvTRUE;                            \
                break;                                                          \
            }                                                                   \
        }                                                                       \
    }

#define gcmCanTileStatusEnabledForMultiSlice(SurfView, canTsEnabled)                               \
    {                                                                                              \
        if (SurfView->numSlices > 1) {                                                             \
            if (SurfView->surf->tileStatusNode.pool != gcvPOOL_UNKNOWN) {                          \
                gctUINT i = 0;                                                                     \
                for (; i < SurfView->numSlices; i++) {                                             \
                    if (SurfView->surf->tileStatusDisabled[i] == gcvTRUE) {                        \
                        *canTsEnabled = gcvFALSE;                                                  \
                        break;                                                                     \
                    }                                                                              \
                    if (SurfView->surf->fcValue[i] != SurfView->surf->fcValue[0]) {                \
                        *canTsEnabled = gcvFALSE;                                                  \
                        break;                                                                     \
                    }                                                                              \
                                                                                                   \
                    if (SurfView->surf->fcValueUpper[i] != SurfView->surf->fcValueUpper[0]) {      \
                        *canTsEnabled = gcvFALSE;                                                  \
                        break;                                                                     \
                    }                                                                              \
                }                                                                                  \
            } else {                                                                               \
                *canTsEnabled = gcvFALSE;                                                          \
            }                                                                                      \
        } else {                                                                                   \
            if ((SurfView->surf->tileStatusNode.pool == gcvPOOL_UNKNOWN) ||                        \
                (SurfView->surf->tileStatusDisabled[SurfView->firstSlice] == gcvTRUE)) {           \
                *canTsEnabled = gcvFALSE;                                                          \
            }                                                                                      \
        }                                                                                          \
    }

#define gcmCONFIGUSC(prefix, featureUSC, featureSeparateLS,                     \
                     featureComputeOnly, featureTS, featureGS,                  \
                     featureUSCFullCacheFix, featureL1CacheSize,                \
                     featureUSCMaxPages, attribCacheRatio, L1CacheRatio)        \
    {                                                                           \
        attribCacheRatio = GCREG_USC_CONTROL_ATTRIB_CACHE_PAGES_QUARTER;        \
                                                                                \
        if (featureUSC) {                                                       \
            if (featureSeparateLS) {                                            \
                L1CacheRatio = GCREG_USC_CONTROL_CACHE_PAGES_FULL;              \
            } else {                                                            \
                gctUINT L1cacheSize;                                            \
                                                                                \
                if (featureComputeOnly) {                                       \
                    L1cacheSize = featureL1CacheSize;                           \
                } else {                                                        \
                    gctUINT attribBufSizeInKB;                                  \
                    if (featureTS) {                                            \
                        /* GS/TS must be bundled. */                            \
                        prefix##ASSERT(featureGS);                              \
                        featureGS         = featureGS;                          \
                        attribBufSizeInKB = 42;                                 \
                    } else {                                                    \
                        prefix##ASSERT(!featureGS);                             \
                        attribBufSizeInKB = 8;                                  \
                    }                                                           \
                    if (attribBufSizeInKB < featureUSCMaxPages) {               \
                        L1cacheSize = featureUSCMaxPages - attribBufSizeInKB;   \
                    } else {                                                    \
                        attribBufSizeInKB -= 2;                                 \
                        L1cacheSize = 2;                                        \
                    }                                                           \
                }                                                               \
                prefix##ASSERT(L1cacheSize);                                    \
                if (L1cacheSize >= featureL1CacheSize) {                        \
                    L1CacheRatio = GCREG_USC_CONTROL_CACHE_PAGES_FULL;          \
                    prefix##ASSERT(featureUSCFullCacheFix);                     \
                    featureUSCFullCacheFix = featureUSCFullCacheFix;            \
                } else {                                                        \
                    static const gctINT s_uscCacheRatio[] = {                   \
                        100000, /* 1.0f */                                      \
                        50000,  /* 0.5f */                                      \
                        25000,  /* 0.25f */                                     \
                        12500,  /* 0.125f */                                    \
                        62500,  /* 0.0625f */                                   \
                        3125,   /* 0.03125f */                                  \
                        75000,  /* 0.75f */                                     \
                        0,      /* 0.0f */                                      \
                    };                                                          \
                    gctINT maxL1cacheSize = L1cacheSize * 100000;               \
                    gctINT delta = 2147483647; /* start with very big delta */  \
                    gctINT i = 0;                                               \
                    gctINT curIndex = -1;                                       \
                    for (; i < gcmCOUNTOF(s_uscCacheRatio); ++i) {              \
                        gctINT curL1cacheSize = featureL1CacheSize * s_uscCacheRatio[i]; \
                                                                                \
                        if ((maxL1cacheSize >= curL1cacheSize) &&               \
                            ((maxL1cacheSize - curL1cacheSize) < delta)) {      \
                            curIndex = i;                                       \
                            delta = maxL1cacheSize - curL1cacheSize;            \
                        }                                                       \
                    }                                                           \
                    prefix##ASSERT(-1 != curIndex);                             \
                    L1CacheRatio = curIndex;                                    \
                }                                                               \
            }                                                                   \
        }                                                                       \
    }

#define gcmCONFIGUSC2(prefix, featureUSC, featureSeparateLS, featureComputeOnly, \
    featureTS, featureL1CacheSize, featureUSCMaxPages, \
    attribCacheRatio, L1CacheRatio) \
{ \
    attribCacheRatio = GCREG_USC_CONTROL_ATTRIB_CACHE_PAGES_QUARTER; \
    \
    if (featureUSC) \
    { \
        if (featureSeparateLS) \
        { \
            L1CacheRatio = GCREG_USC_CONTROL_CACHE_PAGES_FULL; \
        } \
        else \
        { \
            gctUINT L1cacheSize; \
            \
            if (featureComputeOnly) \
            { \
                L1cacheSize = featureL1CacheSize; \
                attribCacheRatio = GCREG_USC_CONTROL_ATTRIB_CACHE_PAGES_NONE; \
            } \
            else \
            { \
                gctUINT attribBufSizeInKB; \
                if (featureTS) \
                { \
                    /* GS/TS must be bundled. */ \
                    attribBufSizeInKB = 42; \
                } \
                else \
                { \
                    attribBufSizeInKB = 8; \
                } \
                if (attribBufSizeInKB < featureUSCMaxPages) \
                { \
                    L1cacheSize = featureUSCMaxPages - attribBufSizeInKB; \
                } \
                else \
                { \
                    attribBufSizeInKB -= 2; \
                    L1cacheSize = 2; \
                } \
            } \
            prefix##ASSERT(L1cacheSize); \
            if (L1cacheSize >= featureL1CacheSize) \
            { \
                L1CacheRatio = GCREG_USC_CONTROL_CACHE_PAGES_FULL; \
            } \
            else \
            { \
                static const gctINT s_uscCacheRatio[] = \
                { \
                    100000,/* 1.0f */     \
                    50000, /* 0.5f */     \
                    25000, /* 0.25f */    \
                    12500, /* 0.125f */   \
                    62500, /* 0.0625f */  \
                    3125,  /* 0.03125f */ \
                    75000, /* 0.75f */    \
                    0,     /*0.0f */      \
                }; \
                gctINT maxL1cacheSize = L1cacheSize * 100000; \
                gctINT delta = 2147483647; /* start with very big delta */ \
                gctINT i = 0; \
                gctINT curIndex = -1; \
                for (; i < gcmCOUNTOF(s_uscCacheRatio); ++i) \
                { \
                    gctINT curL1cacheSize = featureL1CacheSize * s_uscCacheRatio[i]; \
                  \
                    if ((maxL1cacheSize >= curL1cacheSize) && \
                        ((maxL1cacheSize - curL1cacheSize) < delta)) \
                    { \
                        curIndex = i; \
                        delta = maxL1cacheSize - curL1cacheSize; \
                    } \
                } \
                prefix##ASSERT(-1 != curIndex); \
                L1CacheRatio = curIndex; \
            } \
        } \
    } \
} \

#if PHYTIUM_PROFILER_SYSTEM_MEMORY
typedef struct _memory_profile_info {
    struct {
        gctUINT64 currentSize;
        gctUINT64 peakSize;
        gctUINT64 total_allocate;
        gctUINT64 total_free;
        gctUINT32 total_allocateCount;
        gctUINT32 total_freeCount;
    } system_memory, gpu_memory;
} memory_profile_info;

gceSTATUS
gcoOS_GetMemoryProfileInfo(size_t size, struct _memory_profile_info *info);

gceSTATUS
gcoOS_DumpMemoryProfile(void);
gceSTATUS
gcoOS_InitMemoryProfile(void);
gceSTATUS
gcoOS_DeInitMemoryProfile(void);
#endif

void
gcd_2D_printf(
    const char *format, ...
    );

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_base_h_ */
