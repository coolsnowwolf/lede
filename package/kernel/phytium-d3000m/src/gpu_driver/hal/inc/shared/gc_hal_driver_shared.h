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


#ifndef __gc_hal_driver_shared_h_
#define __gc_hal_driver_shared_h_

#include "gc_hal_enum_shared.h"
#include "gc_hal_types_shared.h"

#if defined(__QNXNTO__)
#include <sys/siginfo.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* the DC instruction will cause segfault when disable writebufer and cache.*/
#ifndef gcdSKIP_ARM_DC_INSTRUCTION
#define gcdSKIP_ARM_DC_INSTRUCTION 0
#endif

/* The number of context buffers per user. */
#if gcdCAPTURE_ONLY_MODE
#define gcdCONTEXT_BUFFER_COUNT 1
#else
#define gcdCONTEXT_BUFFER_COUNT 2
#endif

#define gcdRENDER_FENCE_LENGTH              (6 * gcmSIZEOF(gctUINT32))
#define gcdBLT_FENCE_LENGTH                 (10 * gcmSIZEOF(gctUINT32))

/******************************************************************************
 ****************************** I/O Control Codes *****************************
 ******************************************************************************/

#define gcvHAL_CLASS                        "ftg340"
#define IOCTL_GCHAL_INTERFACE               30000
#define IOCTL_GCHAL_PROFILER_INTERFACE      30001
#define IOCTL_GCHAL_TERMINATE               30002

/******************************************************************************
 ***************************** Interface Structure ****************************
 ******************************************************************************/

#define gcdMAX_PROFILE_FILE_NAME            128
#define gcdMAX_FLAT_MAPPING_COUNT           8

/* gcvHAL_CHIP_INFO */
typedef struct _gcsHAL_CHIP_INFO {
    /* Chip count. */
    OUT gctUINT16           count;

    /* Chip types. */
    OUT gctUINT8            types[gcdGLOBAL_CORE_COUNT];

    /* Chip IDs. */
    OUT gctUINT8            ids[gcdGLOBAL_CORE_COUNT];

    OUT gctUINT16           coreIndexs[gcdGLOBAL_CORE_COUNT];

    /* Hardware device ID. */
    OUT gctUINT8            hwDevIDs[gcdGLOBAL_CORE_COUNT];
} gcsHAL_CHIP_INFO;

/* gcvHAL_VERSION */
typedef struct _gcsHAL_VERSION {
    /* version: <major>.<minor>.<patch>. */
    OUT gctINT32            major;
    OUT gctINT32            minor;
    OUT gctINT32            patch;

    /* Build version. */
    OUT gctUINT32           build;
} gcsHAL_VERSION;

/* gcvHAL_SET_TIMEOUT. */
typedef struct _gcsHAL_SET_TIMEOUT {
    gctUINT32               timeOut;
} gcsHAL_SET_TIMEOUT;

/* gcvHAL_QUERY_VIDEO_MEMORY */
typedef struct _gcsHAL_QUERY_VIDEO_MEMORY {
    /* Physical memory address of internal memory. Just a name. */
    OUT gctUINT32           internalPhysName;
    /* Size in bytes of internal memory. */
    OUT gctUINT64           internalSize;

    /* Physical memory address of external memory. Just a name. */
    OUT gctUINT32           externalPhysName;
    /* Size in bytes of external memory.*/
    OUT gctUINT64           externalSize;

    /* Physical memory address of contiguous memory. Just a name. */
    OUT gctUINT32           contiguousPhysName;
    /* Size in bytes of contiguous memory.*/
    OUT gctUINT64           contiguousSize;

    /* Physical memory address of exclusive memory. Just a name. */
    OUT gctUINT32           exclusivePhysName;
    /* Size in bytes of exclusive memory.*/
    OUT gctUINT64           exclusiveSize;

    /* If the virtual pool can be an available video memory pool. */
    OUT gctBOOL             virtualPoolEnabled;
} gcsHAL_QUERY_VIDEO_MEMORY;

/* gcvHAL_QUERY_CHIP_IDENTITY */
typedef struct _gcsHAL_QUERY_CHIP_IDENTITY *gcsHAL_QUERY_CHIP_IDENTITY_PTR;
typedef struct _gcsHAL_QUERY_CHIP_IDENTITY {
    /* Chip model. */
    gceCHIPMODEL                chipModel;

    /* Revision value.*/
    gctUINT32                   chipRevision;

    /* Chip date. */
    gctUINT32                   chipDate;

    /* Supported feature fields. */
    gctUINT32                   chipFeatures;

    /* Supported minor feature fields. */
    gctUINT32                   chipMinorFeatures;

    /* Supported minor feature 1 fields. */
    gctUINT32                   chipMinorFeatures1;

    /* Supported minor feature 2 fields. */
    gctUINT32                   chipMinorFeatures2;

    /* Supported minor feature 3 fields. */
    gctUINT32                   chipMinorFeatures3;

    /* Supported minor feature 4 fields. */
    gctUINT32                   chipMinorFeatures4;

    /* Supported minor feature 5 fields. */
    gctUINT32                   chipMinorFeatures5;

    /* Supported minor feature 6 fields. */
    gctUINT32                   chipMinorFeatures6;

    /* Number of streams supported. */
    gctUINT32                   streamCount;

    /* Number of pixel pipes. */
    gctUINT32                   pixelPipes;

    /* Number of resolve pipes. */
    gctUINT32                   resolvePipes;

    /* Number of instructions. */
    gctUINT32                   instructionCount;

    /* Number of PS instructions. */
    gctUINT32                   PSInstructionCount;

    /* Number of constants. */
    gctUINT32                   numConstants;

    /* Number of varyings */
    gctUINT32                   varyingsCount;

    /* Number of 3D GPUs */
    gctUINT32                   gpuCoreCount;

    /* Physical mask of all AVAILABLE clusters in core.*/
    gctUINT32                   clusterAvailMask;

    /* Product ID */
    gctUINT32                   productID;

    /* Special chip flag bits */
    gceCHIP_FLAG                chipFlags;

    /* ECO ID. */
    gctUINT32                   ecoID;

    /* Customer ID. */
    gctUINT32                   customerID;

    gctUINT32                   chipConfig;

    /* CPU view physical address and size of SRAMs. */
    gctUINT64                   sRAMBases[gcvSRAM_INTER_COUNT];
    gctUINT32                   sRAMSizes[gcvSRAM_INTER_COUNT];

    gctUINT64                   platformFlagBits;

    /* APB register offset. */
    gctUINT64                   registerAPB;

    /* Enabled NN clusters number. */
    gctUINT32                   nnClusterNum;

    /* Virtual address bits. */
    gctUINT32                   virtualAddressBits;
    /* Physical address bits. */
    gctUINT32                   physicalAddressBits;

    /* PCI device information. */
    gctUINT32                   pciDomain;
    gctUINT32                   pciBus;
    gctUINT32                   pciSlot;
    gctUINT32                   pciFunction;
} gcsHAL_QUERY_CHIP_IDENTITY;

/* gcvHAL_QUERY_CHIP_OPTION. */
typedef struct _gcsHAL_QUERY_CHIP_OPTIONS *gcsHAL_QUERY_CHIP_OPTIONS_PTR;
typedef struct _gcsHAL_QUERY_CHIP_OPTIONS {
    gctBOOL                     gpuProfiler;
    gctBOOL                     allowFastClear;
    gctBOOL                     powerManagement;
    /*
     * Whether use new MMU. It is meaningless
     * for old MMU since old MMU is always enabled.
     */
    gctBOOL                     enableMMU;
    gceCOMPRESSION_OPTION       allowCompression;
    gctBOOL                     smallBatch;
    gctUINT32                   uscL1CacheRatio;
    gctUINT32                   uscAttribCacheRatio;
    gctUINT32                   userClusterMask;
    gctUINT32                   userClusterMasks[gcdMAX_MAJOR_CORE_COUNT];

    /* Internal SRAM. */
    gctADDRESS                  sRAMGPUVirtAddrs[gcvSRAM_INTER_COUNT];
    gctUINT32                   sRAMSizes[gcvSRAM_INTER_COUNT];
    gctUINT32                   sRAMCount;

    /* External SRAM. */
    gctPHYS_ADDR_T              extSRAMCPUPhysAddrs[gcvSRAM_EXT_COUNT];
    gctPHYS_ADDR_T              extSRAMGPUPhysAddrs[gcvSRAM_EXT_COUNT];
    gctADDRESS                  extSRAMGPUVirtAddrs[gcvSRAM_EXT_COUNT];
    gctUINT32                   extSRAMGPUPhysNames[gcvSRAM_EXT_COUNT];
    gctUINT32                   extSRAMSizes[gcvSRAM_EXT_COUNT];
    gctUINT32                   extSRAMCount;

    /* Only represents system reserved memory pool currently. */
    gctUINT32                   vidMemCount;

    gceSECURE_MODE              secureMode;

    gctBOOL                     hasShader;

    /* NN clusters power control. */
    gctUINT32                   enableNNClusters;
    gctUINT32                   configNNPowerControl;
    /* Active NN core count. */
    gctUINT32                   activeNNCoreCount;

    gctBOOL                     processPageTable;
    gctBOOL                     sharedPageTable;
} gcsHAL_QUERY_CHIP_OPTIONS;

/* gcvHAL_QUERY_CHIP_FREQUENCY. */
typedef struct _gcsHAL_QUERY_CHIP_FREQUENCY *gcsHAL_QUERY_CHIP_FREQUENCY_PTR;
typedef struct _gcsHAL_QUERY_CHIP_FREQUENCY {
    OUT gctUINT64               mcClk;
    OUT gctUINT64               shClk;
} gcsHAL_QUERY_CHIP_FREQUENCY;

/* Obsolete for userpace. */
/* gcvHAL_ALLOCATE_NON_PAGED_MEMORY */
typedef struct _gcsHAL_ALLOCATE_NON_PAGED_MEMORY {
    /* Allocation flags. */
    IN gctUINT32                flags;

    /* Number of bytes to allocate. */
    IN OUT gctUINT64            bytes;

    /* Physical address of allocation. Just a name. */
    OUT gctUINT32               physName;

    /* Logical address of allocation. */
    OUT gctUINT64               logical;
} gcsHAL_ALLOCATE_NON_PAGED_MEMORY;

/* Obsolete for userpace. */
/* gcvHAL_FREE_NON_PAGED_MEMORY */
typedef struct _gcsHAL_FREE_NON_PAGED_MEMORY {
    /* Number of bytes allocated. */
    IN gctUINT64                bytes;

    /* Physical address of allocation. Just a name. */
    IN gctUINT32                physName;

    /* Logical address of allocation. */
    IN gctUINT64                logical;
} gcsHAL_FREE_NON_PAGED_MEMORY;

/* Video memory allocation. */
/* gcvHAL_ALLOCATE_LINEAR_VIDEO_MEMORY */
typedef struct _gcsHAL_ALLOCATE_LINEAR_VIDEO_MEMORY {
    /* Number of bytes to allocate. */
    IN OUT gctUINT64            bytes;

    /* Buffer alignment. */
    IN gctUINT32                alignment;

    /* Type of allocation, see gceVIDMEM_TYPE. */
    IN gctUINT32                type;

    /* Flag of allocation. */
    IN gctUINT32                flag;

    /* Memory pool to allocate from. */
    IN OUT gctUINT32            pool;

    /* Internal SRAM index. */
    IN gctINT32                 sRAMIndex;

    /* External SRAM index. */
    IN gctINT32                 extSRAMIndex;

    /* Video memory index, only represents system reserved memroy pool currently. */
    IN gctINT32                 vidMemIndex;

    /* Allocated video memory. */
    OUT gctUINT32               node;
} gcsHAL_ALLOCATE_LINEAR_VIDEO_MEMORY;

typedef struct _gcsUSER_MEMORY_DESC {
    /* Import flag. */
    gctUINT32                   flag;

    /* gcvALLOC_FLAG_DMABUF */
    gctUINT32                   handle;
    gctUINT64                   dmabuf;

    /* gcvALLOC_FLAG_USERMEMORY */
    gctUINT64                   logical;
    gctUINT64                   physical;
    gctUINT64                   size;
} gcsUSER_MEMORY_DESC;

/* gcvHAL_WRAP_USER_MEMORY. */
typedef struct _gcsHAL_WRAP_USER_MEMORY {
    /* Description of user memory. */
    IN gcsUSER_MEMORY_DESC      desc;

    /* Video memory allocation type. */
    IN gctUINT32                type;

    /* Output video mmory node. */
    OUT gctUINT32               node;

    /* size of the node in bytes */
    OUT gctUINT64               bytes;
} gcsHAL_WRAP_USER_MEMORY;

/* gcvHAL_RELEASE_VIDEO_MEMORY */
typedef struct _gcsHAL_RELEASE_VIDEO_MEMORY {
    /* Allocated video memory. */
    IN gctUINT32                node;

#ifdef __QNXNTO__
    /* Mapped logical address to unmap in user space. */
    OUT gctUINT64               memory;

    /* Number of bytes to allocated. */
    OUT gctUINT64               bytes;
#endif
} gcsHAL_RELEASE_VIDEO_MEMORY;

/* gcvHAL_LOCK_VIDEO_MEMORY */
typedef struct _gcsHAL_LOCK_VIDEO_MEMORY {
    /* Allocated video memory. */
    IN gctUINT32                node;

    /* Cache configuration. */
    /* Only gcvPOOL_VIRTUAL can be configured */
    IN gctBOOL                  cacheable;

    /* Hardware specific address. */
    OUT gctADDRESS              address;

    /* Mapped logical address. */
    OUT gctUINT64               memory;

    /* Customer priviate handle*/
    OUT gctUINT32               gid;

    /* Bus address of a contiguous video node. */
    OUT gctUINT64               physicalAddress;

#if gcdCAPTURE_ONLY_MODE
    IN gctBOOL                  queryCapSize;
    IN gctPOINTER               captureLogical;
    OUT gctUINT64               captureSize;
#endif

    IN gceLOCK_VIDEO_MEMORY_OP  op;
} gcsHAL_LOCK_VIDEO_MEMORY;

/* gcvHAL_UNLOCK_VIDEO_MEMORY */
typedef struct _gcsHAL_UNLOCK_VIDEO_MEMORY {
    /* Allocated video memory. */
    IN gctUINT64                node;

    /* Video memory allocation type. */
    IN gctUINT32                type;

    /* Pool of the unlock node */
    OUT gctUINT32               pool;

    /* Bytes of the unlock node */
    OUT gctUINT64               bytes;

    /* Flag to unlock surface asynchroneously. */
    IN OUT gctBOOL              asynchroneous;

#if gcdCAPTURE_ONLY_MODE
    OUT gctPOINTER              captureLogical;
#endif

    IN gceLOCK_VIDEO_MEMORY_OP  op;

    IN gctUINT64                mmu;
} gcsHAL_UNLOCK_VIDEO_MEMORY;

/* gcvHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY: */
typedef struct _gcsHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY {
    /* Allocated video memory. */
    IN gctUINT32                node;

    /* Video memory allocation type. */
    IN gctUINT32                type;
} gcsHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY;

/* gcvHAL_EXPORT_VIDEO_MEMORY. */
typedef struct _gcsHAL_EXPORT_VIDEO_MEMORY {
    /* Allocated video memory. */
    IN gctUINT32                node;

    /* Export flags */
    IN gctUINT32                flags;

    /* Exported dma_buf fd */
    OUT gctINT32                fd;
} gcsHAL_EXPORT_VIDEO_MEMORY;

/* gcvHAL_NAME_VIDEO_MEMORY. */
typedef struct _gcsHAL_NAME_VIDEO_MEMORY {
    IN gctUINT32                handle;
    OUT gctUINT32               name;
} gcsHAL_NAME_VIDEO_MEMORY;

/* gcvHAL_IMPORT_VIDEO_MEMORY. */
typedef struct _gcsHAL_IMPORT_VIDEO_MEMORY {
    IN gctUINT32                name;
    OUT gctUINT32               handle;
} gcsHAL_IMPORT_VIDEO_MEMORY;

/* gcvHAL_MAP_MEMORY */
typedef struct _gcsHAL_MAP_MEMORY {
    /* Physical memory address to map. Just a name on Linux/Qnx. */
    IN gctUINT32                physName;

    /* Number of bytes in physical memory to map. */
    IN gctUINT64                bytes;

    /* Address of mapped memory. */
    OUT gctUINT64               logical;
} gcsHAL_MAP_MEMORY;

/* gcvHAL_UNMAP_MEMORY */
typedef struct _gcsHAL_UNMAP_MEMORY {
    /* Physical memory address to unmap. Just a name on Linux/Qnx. */
    IN gctUINT32                physName;

    /* Number of bytes in physical memory to unmap. */
    IN gctUINT64                bytes;

    /* Address of mapped memory to unmap. */
    IN gctUINT64                logical;
} gcsHAL_UNMAP_MEMORY;

/* gcvHAL_CACHE */
typedef struct _gcsHAL_CACHE {
    IN gceCACHEOPERATION        operation;
    IN gctUINT64                process;
    IN gctUINT64                logical;
    IN gctUINT64                bytes;
    IN gctUINT64                offset;
    IN gctUINT32                node;
} gcsHAL_CACHE;

/* gcvHAL_ATTACH */
typedef struct _gcsHAL_ATTACH {
    /* Handle of context buffer object. */
    OUT gctUINT32               context;

    /* Maximum state in the buffer. */
    OUT gctUINT64               maxState;

    /* Number of states in the buffer. */
    OUT gctUINT32               numStates;

    /* Map context buffer to user or not. */
    IN gctBOOL                  map;

    /* Physical of context buffer. */
    OUT gctUINT64               logicals[2];

    /* Bytes of context buffer. */
    OUT gctUINT32               bytes;

    /* The context is for multi-core or not. */
    IN gctBOOL                  shared;

    /* If want to force disable small batch for some process, pls set to true. */
    IN gctBOOL                  disableSmallBatch;

#if gcdCAPTURE_ONLY_MODE
    IN gctBOOL                  queryCapSize;
    IN gctPOINTER               contextLogical[gcdCONTEXT_BUFFER_COUNT];
    OUT gctUINT64               captureSize;
#endif
} gcsHAL_ATTACH;

/* gcvHAL_DETACH */
typedef struct _gcsHAL_DETACH {
    /* Context buffer object gckCONTEXT. Just a name. */
    IN gctUINT32                context;
} gcsHAL_DETACH;

/* gcvHAL_EVENT_COMMIT. */
typedef struct _gcsHAL_EVENT_COMMIT {
    /* Event queue in gcsQUEUE. */
    IN gctUINT64                queue;

    /* Brother cores in user device of current commit process. */
    IN gctUINT32                broCoreMask;

    IN gctBOOL                  shared;
#if gcdENABLE_SW_PREEMPTION
    /* Priority ID. */
    IN gctUINT32                priorityID;

    /* Does it require top priority? */
    IN gctBOOL                  topPriority;
#endif
} gcsHAL_EVENT_COMMIT;

typedef struct _gcsHAL_COMMAND_LOCATION {
    gctUINT32                   priority;
    gctUINT32                   channelId;

    gctUINT32                   videoMemNode;

    gctADDRESS                  address;
    gctUINT64                   logical;
    gctUINT32                   startOffset;
    /* size includes reservedHead and reservedTail. */
    gctUINT32                   size;

    gctUINT32                   reservedHead;
    gctUINT32                   reservedTail;

    /* Pointer to patch list. */
    gctUINT64                   patchHead;

    /*
     * Location index of exit commands, ie where to put the chipEnable/link back
     * commands in the reservedTail area.
     * It's used in fully shared command buffer for multiple cores.
     */
    gctUINT32                   exitIndex;
    gctUINT32                   entryPipe;
    gctUINT32                   exitPipe;

    /* struct _gcsHAL_COMMAND_LOCATION * next; */
    gctUINT64                   next;
#if gcdCAPTURE_ONLY_MODE
    gctPOINTER                  contextLogical[gcdCONTEXT_BUFFER_COUNT];
#endif

    /* Save Gc_PrivateDmaData + context deltas */
    gctUINT64 privateDriverData;
    gctUINT64 privateDriverDataSize;
    gctUINT64 contextDeltaPos;
} gcsHAL_COMMAND_LOCATION;

typedef struct _gcsHAL_SUBCOMMIT {
    gctUINT32                   coreId;

    /* user gcsSTATE_DELTA_PTR. */
    gctUINT64                   delta;

    /* Kernel gckCONTEXT. */
    gctUINT64                   context;

    /* Event queue in user gcsQUEUE *. */
    gctUINT64                   queue;

    /* Locate the commands. */
    gcsHAL_COMMAND_LOCATION     commandBuffer;

    /* struct _gcsHAL_SUBCOMMIT * next; */
    gctUINT64                   next;

#if gcdENABLE_SW_PREEMPTION
    /* Process ID. */
    gctUINT32                   pid;

    /* Engine type. */
    gceENGINE                   engine;

    /* Is it multi-core and shared mode?. */
    gctBOOL                     shared;

    /* Priority ID. */
    gctUINT32                   priorityID;

    /* Does it require top priority. */
    gctBOOL                     topPriority;
#endif
} gcsHAL_SUBCOMMIT, *gcsHAL_SUBCOMMIT_PTR;

/* gcvHAL_COMMIT */
typedef struct _gcsHAL_COMMIT {
    gcsHAL_SUBCOMMIT            subCommit;

    gctBOOL                     shared;

    gctBOOL                     contextSwitched;

    /* Commit stamp of this commit. */
    OUT gctUINT64               commitStamp;

    /* Brother cores in user device of current commit process. */
    gctUINT32                   broCoreMask;

    gctUINT32                   MsaaProduct;

#if gcdENABLE_MP_SWITCH
    /* Multi-processor mode. */
    gctUINT32                   mpMode;

    /* Switch multi-processor mode. */
    gctUINT32                   switchMpMode;
#endif

#if gcdENABLE_SW_PREEMPTION
    /* If user need to merge the delta. */
    gctBOOL                     needMerge;

    /* If this commit is pending. */
    gctBOOL                     pending;
#endif
} gcsHAL_COMMIT;

typedef struct _gcsHAL_COMMIT_DONE {
    IN gctUINT64                context;

#if gcdENABLE_SW_PREEMPTION
    /* Priority ID. */
    IN gctUINT32                priorityID;
#endif
} gcsHAL_COMMIT_DONE;

/* gcvHAL_USER_SIGNAL  */
typedef struct _gcsHAL_USER_SIGNAL {
    /* Command. */
    gceUSER_SIGNAL_COMMAND_CODES command;

    /* Signal ID. */
    IN OUT gctINT32             id;

    /* Reset mode. */
    IN gctBOOL                  manualReset;

    /* Wait timedout. */
    IN gctUINT32                wait;

    /* State. */
    IN gctBOOL                  state;

    /* Return status */
    IN gceSIGNAL_STATUS         status;
} gcsHAL_USER_SIGNAL;

/* gcvHAL_SIGNAL. */
typedef struct _gcsHAL_SIGNAL {
    /* Signal handle to signal gctSIGNAL. */
    IN gctUINT64                signal;

    /* Reserved gctSIGNAL. */
    IN gctUINT64                auxSignal;

    /* Process owning the signal gctHANDLE. */
    IN gctUINT64                process;

#if defined(__QNXNTO__)
    /* Client pulse event. */
    IN struct sigevent          event;

    /* Set by server. */
    IN gctINT32                 rcvid;
#endif
    /* Event generated from where of pipeline */
    IN gceKERNEL_WHERE          fromWhere;

#if gcdENABLE_SW_PREEMPTION
    /* If it is a fence signal. */
    IN gctBOOL                  fenceSignal;
#endif
} gcsHAL_SIGNAL;

/* gcvHAL_WRITE_DATA. */
typedef struct _gcsHAL_WRITE_DATA {
    /* Address to write data to. */
    IN gctUINT32                address;

    /* Data to write. */
    IN gctUINT32                data;
} gcsHAL_WRITE_DATA;

/* gcvHAL_READ_REGISTER */
typedef struct _gcsHAL_READ_REGISTER {
    /* Logical address of memory to write data to. */
    IN gctUINT32                address;

    /* Data read. */
    OUT gctUINT32               data;
} gcsHAL_READ_REGISTER;

/* gcvHAL_WRITE_REGISTER */
typedef struct _gcsHAL_WRITE_REGISTER {
    /* Logical address of memory to write data to. */
    IN gctUINT32                address;

    /* Data read. */
    IN gctUINT32                data;
} gcsHAL_WRITE_REGISTER;

/* gcvHAL_READ_REGISTER_EX */
typedef struct _gcsHAL_READ_REGISTER_EX {
    /* Logical address of memory to write data to. */
    IN gctUINT32                address;

    IN gctUINT32                coreSelect;

    /* Data read. */
    OUT gctUINT32               data[4];
} gcsHAL_READ_REGISTER_EX;

/* gcvHAL_WRITE_REGISTER_EX */
typedef struct _gcsHAL_WRITE_REGISTER_EX {
    /* Logical address of memory to write data to. */
    IN gctUINT32                address;

    IN gctUINT32                coreSelect;

    /* Data read. */
    IN gctUINT32                data[4];
} gcsHAL_WRITE_REGISTER_EX;

/* gcvHAL_APB_AXIFEE_ACCESS */
typedef struct _gcsHAL_APB_AXIFE_ACCESS {
    /* Logical address of memory to write data to. */
    IN gctUINT32                address;

    IN gctUINT32                coreSelect;

    IN gctBOOL                  isRead;

    /* Data read. */
    IN gctUINT32                data;
} gcsHAL_APB_AXIFE_ACCESS;

#if PHYTIUM_PROFILER
/* gcvHAL_GET_PROFILE_SETTING */
typedef struct _gcsHAL_GET_PROFILE_SETTING {
    /* Enable profiling */
    OUT gctBOOL                 enable;
    /* Profile mode */
    OUT gceProfilerMode         profileMode;
    /* Probe mode */
    OUT gceProbeMode            probeMode;
} gcsHAL_GET_PROFILE_SETTING;

/* gcvHAL_SET_PROFILE_SETTING */
typedef struct _gcsHAL_SET_PROFILE_SETTING {
    /* Enable profiling */
    IN gctBOOL                  enable;
    /* Profile mode */
    IN gceProfilerMode          profileMode;
    /* Probe mode */
    IN gceProbeMode             probeMode;
} gcsHAL_SET_PROFILE_SETTING;

/* gcvHAL_READ_PROFILER_REGISTER_SETTING */
typedef struct _gcsHAL_READ_PROFILER_REGISTER_SETTING {
    /*Should Clear Register*/
    IN gctBOOL                  bclear;
} gcsHAL_READ_PROFILER_REGISTER_SETTING;

typedef struct _gcsHAL_READ_ALL_PROFILE_REGISTERS {
    /* Data read. */
    OUT gcsPROFILER_COUNTERS  Counters;
} gcsHAL_READ_ALL_PROFILE_REGISTERS;
#endif

/* gcvHAL_SET_POWER_MANAGEMENT_STATE */
typedef struct _gcsHAL_SET_POWER_MANAGEMENT {
    /* Data read. */
    IN gceCHIPPOWERSTATE        state;
} gcsHAL_SET_POWER_MANAGEMENT;

/* gcvHAL_QUERY_POWER_MANAGEMENT_STATE */
typedef struct _gcsHAL_QUERY_POWER_MANAGEMENT {
    /* Data read. */
    OUT gceCHIPPOWERSTATE       state;

    /* Idle query. */
    OUT gctBOOL                 isIdle;
} gcsHAL_QUERY_POWER_MANAGEMENT;

/* gcvHAL_CONFIG_POWER_MANAGEMENT. */
typedef struct _gcsHAL_CONFIG_POWER_MANAGEMENT {
    IN gctBOOL                  enable;
    OUT gctBOOL                 oldValue;
} gcsHAL_CONFIG_POWER_MANAGEMENT;

typedef struct _gcsFLAT_MAPPING_RANGE {
    gctUINT64                   start;
    gctUINT64                   end;
    gctUINT64                   size;
    gceFLATMAP_FLAG             flag;

    /* Corresponding virtual start. */
    gctUINT64                   vStart;
} gcsFLAT_MAPPING_RANGE;

/* gcvHAL_GET_BASE_ADDRESS */
typedef struct _gcsHAL_GET_BASE_ADDRESS {
    /* Physical memory address of internal memory. */
    OUT gctUINT32               baseAddress;

    OUT gctUINT32               flatMappingRangeCount;

    OUT gcsFLAT_MAPPING_RANGE   flatMappingRanges[gcdMAX_FLAT_MAPPING_COUNT];
} gcsHAL_GET_BASE_ADDRESS;

typedef struct _gcsHAL_SET_DEBUG_LEVEL_ZONE {
    IN gctUINT32                level;
    IN gctUINT32                zones;
    IN gctBOOL                  enable;
} gcsHAL_SET_DEBUG_LEVEL_ZONE;

typedef struct _gcsHAL_QUERY_CPU_FREQUENCY
{
    IN  gctUINT32               CPUId;
    OUT gctUINT32               CPUFrequency;
} gcsHAL_QUERY_CPU_FREQUENCY;

/* gcvHAL_DEBUG_DUMP. */
typedef struct _gcsHAL_DEBUG_DUMP {
    /* gceDUMP_BUFFER_TYPE      type. */
    IN gctUINT32                type;

    IN gctUINT64                ptr;
    IN gctADDRESS               address;
    IN gctUINT32                size;
} gcsHAL_DEBUG_DUMP;

/* gcvHAL_TIMESTAMP */
typedef struct _gcsHAL_TIMESTAMP {
    /* Timer select. */
    IN gctUINT32                timer;

    /* Timer request type (0-stop, 1-start, 2-send delta). */
    IN gctUINT32                request;

    /* Result of delta time in microseconds. */
    OUT gctINT32                timeDelta;
} gcsHAL_TIMESTAMP;

/* gcvHAL_DATABASE */
typedef struct _gcsHAL_DATABASE {
    /*
     * Set to gcvTRUE if you want to query a particular process ID.
     * Set to gcvFALSE to query the last detached process.
     */
    IN gctBOOL                  validProcessID;

    /* Process ID to query. */
    IN gctUINT32                processID;

    /* Information. */
    OUT gcuDATABASE_INFO        vidMem;
    OUT gcuDATABASE_INFO        nonPaged;
    OUT gcuDATABASE_INFO        contiguous;
    OUT gcuDATABASE_INFO        gpuIdle;

    /* Detail information about video memory. */
    OUT gcuDATABASE_INFO        vidMemPool[3];
} gcsHAL_DATABASE;

/* gcvHAL_GET_FRAME_INFO. */
typedef struct _gcsHAL_GET_FRAME_INFO {
    /* gcsHAL_FRAME_INFO* */
    OUT gctUINT64               frameInfo;
} gcsHAL_GET_FRAME_INFO;

typedef struct _gcsHAL_SET_FSCALE_VALUE {
    IN gctUINT32                value;
    IN gctUINT32                shValue;
} gcsHAL_SET_FSCALE_VALUE;

typedef struct _gcsHAL_GET_FSCALE_VALUE {
    OUT gctUINT32               value;
    OUT gctUINT32               minValue;
    OUT gctUINT32               maxValue;
} gcsHAL_GET_FSCALE_VALUE;

/* gcvHAL_QUERY_RESET_TIME_STAMP. */
typedef struct _gcsHAL_QUERY_RESET_TIME_STAMP {
    OUT gctUINT64               timeStamp;
    OUT gctUINT64               contextID;
} gcsHAL_QUERY_RESET_TIME_STAMP;

/* gcvHAL_CREATE_NATIVE_FENCE. */
typedef struct _gcsHAL_CREATE_NATIVE_FENCE {
    /* Signal id. */
    IN gctUINT64                signal;

    /* Native fence file descriptor. */
    OUT gctINT32                fenceFD;

} gcsHAL_CREATE_NATIVE_FENCE;

/* gcvHAL_WAIT_NATIVE_FENCE. */
typedef struct _gcsHAL_WAIT_NATIVE_FENCE {
    /* Native fence file descriptor. */
    IN gctINT32                 fenceFD;

    /* Wait timeout. */
    IN gctUINT32                timeout;
} gcsHAL_WAIT_NATIVE_FENCE;

/* gcvHAL_SHBUF. */
typedef struct _gcsHAL_SHBUF {
    gceSHBUF_COMMAND_CODES      command;

    /* Shared buffer. */
    IN OUT gctUINT64            id;

    /* User data to be shared. */
    IN gctUINT64                data;

    /* Data size. */
    IN OUT gctUINT32            bytes;
} gcsHAL_SHBUF;

/* gcvHAL_GET_GRAPHIC_BUFFER_FD. */
/*
 * Fd representation of android graphic buffer contents.
 * Currently, it is only to reference video nodes, signal, etc to avoid being
 * destroyed when trasfering across processes.
 */
typedef struct _gcsHAL_GET_GRAPHIC_BUFFER_FD {
    /* Max 3 video nodes, node handle here. */
    IN gctUINT32                node[3];

    /* A shBuf. */
    IN gctUINT64                shBuf;

    /* A signal. */
    IN gctUINT64                signal;

    OUT gctINT32                fd;
} gcsHAL_GET_GRAPHIC_BUFFER_FD;

typedef struct _gcsHAL_VIDEO_MEMORY_METADATA {
    /* Allocated video memory. */
    IN gctUINT32                node;

    IN gctUINT32                readback;

    INOUT gctINT32              ts_fd;
    INOUT gctUINT32             ts_offset;
    INOUT gctUINT32             tile_mode;
    INOUT gctUINT32             dmabuf_size;
    INOUT gctUINT32             image_format;
    INOUT gctUINT32             offset;
    INOUT gctUINT32             stride;
    INOUT gctUINT32             width;
    INOUT gctUINT32             height;
    INOUT gctUINT32             fc_enabled;
    INOUT gctUINT32             fc_value;
    INOUT gctUINT32             fc_value_upper;

    INOUT gctUINT32             compressed;
    INOUT gctUINT32             compress_format;

    INOUT gctUINT32             header_size;
} gcsHAL_VIDEO_MEMORY_METADATA;

/* gcvHAL_GET_VIDEO_MEMORY_FD. */
typedef struct _gcsHAL_GET_VIDEO_MEMORY_FD {
    IN gctUINT32                handle;
    IN gctBOOL                  exported;
    OUT gctINT32                fd;
} gcsHAL_GET_VIDEO_MEMORY_FD;

/* gcvHAL_DESTROY_MMU. */
typedef struct _gcsHAL_DESTROY_MMU {
    /* Mmu object. */
    IN gctUINT64                mmu;
    IN gctUINT64                database;
    IN gctUINT32                pid;
} gcsHAL_DESTROY_MMU;

/* gcvHAL_WAIT_FENCE. */
typedef struct _gcsHAL_WAIT_FENCE {
    IN gctUINT32                handle;
    IN gctUINT32                timeOut;
} gcsHAL_WAIT_FENCE;

/* gcvHAL_DEVICE_MUTEX: */
typedef struct _gcsHAL_DEVICE_MUTEX {
    /* Lock or Release device mutex. */
    gctBOOL                     isMutexLocked;
} gcsHAL_DEVICE_MUTEX;


#if gcdDEC_ENABLE_AHB
/* gcvHAL_DEC300_READ. */
typedef struct _gcsHAL_DEC300_READ {
    gctUINT32                   enable;
    gctUINT32                   readId;
    gctUINT32                   format;
    gctUINT32                   strides[3];
    gctUINT32                   is3D;
    gctUINT32                   isMSAA;
    gctUINT32                   clearValue;
    gctUINT32                   isTPC;
    gctUINT32                   isTPCCompressed;
    gctUINT32                   surfAddrs[3];
    gctUINT32                   tileAddrs[3];
} DEC300Read;

/* gcvHAL_DEC300_WRITE. */
typedef struct _gcsHAL_DEC300_WRITE {
    gctUINT32                   enable;
    gctUINT32                   readId;
    gctUINT32                   writeId;
    gctUINT32                   format;
    gctUINT32                   surfAddr;
    gctUINT32                   tileAddr;
} DEC300Write;

/* gcvHAL_DEC300_FLUSH. */
typedef struct _gcsHAL_DEC300_FLUSH {
    IN gctUINT8                 useless;
} DEC300Flush;

/* gcvHAL_DEC300_FLUSH_WAIT. */
typedef struct _gcsHAL_DEC300_FLUSH_WAIT {
    IN gctUINT32                done;
} DEC300FlushWait;
#endif

#if gcdENABLE_VIDEO_MEMORY_MIRROR
typedef struct _gcsHAL_SYNC_VIDEO_MEMORY {
    IN gctUINT64                node;
    IN gceSYNC_MEMORY_DIRECTION dir;
} gcsHAL_SYNC_VIDEO_MEMORY;
#endif

#if gcdENABLE_CLEAR_FENCE
typedef struct _gcsHAL_STORE_CLEAR_FENCE {
    gctADDRESS                  address;
    gctUINT64                   fenceValue;
    gctUINT64                   recordId;
    gctBOOL                     use64BitFence;
    gctBOOL                     isClear;
} gcsHAL_STORE_CLEAR_FENCE;
#endif

typedef struct _gcsHAL_CONFIG_CLUSTER {
    IN gctUINT32                clusterMask;
    OUT gctUINT32               curValue;
} gcsHAL_CONFIG_CLUSTER;

typedef struct _gcsHAL_INTERFACE {
    /* Command code. */
    gceHAL_COMMAND_CODES        command;

    /* Hardware type. */
    gceHARDWARE_TYPE            hardwareType;

    /* Device index. */
    gctUINT32                   devIndex;

    /* Core index for current hardware type. */
    gctUINT32                   coreIndex;

    /* Status value. */
    gceSTATUS                   status;

    /* Engine */
    gceENGINE                   engine;

    /* Ignore information from TSL when doing IO control */
    gctBOOL                     ignoreTLS;

    /* The mutext already acquired */
    IN gctBOOL                  commitMutex;

    /* Kernel gckCONTEXT. */
    IN gctUINT64                context;

    /* Union of command structures. */
    union _u {
        gcsHAL_CHIP_INFO                    ChipInfo;
        gcsHAL_VERSION                      Version;
        gcsHAL_SET_TIMEOUT                  SetTimeOut;

        gcsHAL_QUERY_VIDEO_MEMORY           QueryVideoMemory;
        gcsHAL_QUERY_CHIP_IDENTITY          QueryChipIdentity;
        gcsHAL_QUERY_CHIP_OPTIONS           QueryChipOptions;
        gcsHAL_QUERY_CHIP_FREQUENCY         QueryChipFrequency;

        gcsHAL_ALLOCATE_NON_PAGED_MEMORY    AllocateNonPagedMemory;
        gcsHAL_FREE_NON_PAGED_MEMORY        FreeNonPagedMemory;

        gcsHAL_ALLOCATE_LINEAR_VIDEO_MEMORY AllocateLinearVideoMemory;
        gcsHAL_WRAP_USER_MEMORY             WrapUserMemory;
        gcsHAL_RELEASE_VIDEO_MEMORY         ReleaseVideoMemory;

        gcsHAL_LOCK_VIDEO_MEMORY               LockVideoMemory;
        gcsHAL_UNLOCK_VIDEO_MEMORY             UnlockVideoMemory;
        gcsHAL_BOTTOM_HALF_UNLOCK_VIDEO_MEMORY BottomHalfUnlockVideoMemory;

        gcsHAL_EXPORT_VIDEO_MEMORY          ExportVideoMemory;
        gcsHAL_NAME_VIDEO_MEMORY            NameVideoMemory;
        gcsHAL_IMPORT_VIDEO_MEMORY          ImportVideoMemory;

        gcsHAL_MAP_MEMORY                   MapMemory;
        gcsHAL_UNMAP_MEMORY                 UnmapMemory;

        gcsHAL_CACHE                        Cache;

        gcsHAL_ATTACH                       Attach;
        gcsHAL_DETACH                       Detach;

        gcsHAL_EVENT_COMMIT                 Event;
        gcsHAL_COMMIT                       Commit;

        gcsHAL_COMMIT_DONE                  CommitDone;

        gcsHAL_USER_SIGNAL                  UserSignal;
        gcsHAL_SIGNAL                       Signal;

        gcsHAL_WRITE_DATA                   WriteData;
        gcsHAL_READ_REGISTER                ReadRegisterData;
        gcsHAL_WRITE_REGISTER               WriteRegisterData;
        gcsHAL_APB_AXIFE_ACCESS             APBAXIFEAccess;
        gcsHAL_READ_REGISTER_EX             ReadRegisterDataEx;
        gcsHAL_WRITE_REGISTER_EX            WriteRegisterDataEx;
        gcsHAL_SET_POWER_MANAGEMENT         SetPowerManagement;
        gcsHAL_QUERY_POWER_MANAGEMENT       QueryPowerManagement;
        gcsHAL_CONFIG_POWER_MANAGEMENT      ConfigPowerManagement;

        gcsHAL_GET_BASE_ADDRESS             GetBaseAddress;

        gcsHAL_SET_DEBUG_LEVEL_ZONE         DebugLevelZone;

        gcsHAL_QUERY_CPU_FREQUENCY          QueryCPUFrequency;

        gcsHAL_DEBUG_DUMP                   DebugDump;

        gcsHAL_TIMESTAMP                    TimeStamp;
        gcsHAL_DATABASE                     Database;

        gcsHAL_GET_FRAME_INFO               GetFrameInfo;

        /* gcsHAL_DUMP_GPU_STATE */
        /* gcsHAL_DUMP_EVENT */

        gcsHAL_SET_FSCALE_VALUE             SetFscaleValue;
        gcsHAL_GET_FSCALE_VALUE             GetFscaleValue;

        gcsHAL_QUERY_RESET_TIME_STAMP       QueryResetTimeStamp;

        gcsHAL_CREATE_NATIVE_FENCE          CreateNativeFence;
        gcsHAL_WAIT_NATIVE_FENCE            WaitNativeFence;
        gcsHAL_SHBUF                        ShBuf;
        gcsHAL_GET_GRAPHIC_BUFFER_FD        GetGraphicBufferFd;
        gcsHAL_VIDEO_MEMORY_METADATA        SetVidMemMetadata;
        gcsHAL_GET_VIDEO_MEMORY_FD          GetVideoMemoryFd;

        gcsHAL_DESTROY_MMU                  DestroyMmu;

        gcsHAL_WAIT_FENCE                   WaitFence;

        /* gcvHAL_DEVICE_MUTEX: */
        gcsHAL_DEVICE_MUTEX                 DeviceMutex;


#if gcdDEC_ENABLE_AHB
        gcsHAL_DEC300_READ                  DEC300Read;
        gcsHAL_DEC300_WRITE                 DEC300Write;
        gcsHAL_DEC300_FLUSH                 DEC300Flush;
        gcsHAL_DEC300_FLUSH_WAIT            DEC300FlushWait;
#endif
#if gcdENABLE_VIDEO_MEMORY_MIRROR
        gcsHAL_SYNC_VIDEO_MEMORY            SyncVideoMemory;
#endif

        gcsHAL_CONFIG_CLUSTER               ConfigCluster;

#if gcdENABLE_CLEAR_FENCE
        gcsHAL_STORE_CLEAR_FENCE            UserFence;
#endif
    } u;

    /*
     *  The above structure needs to remain unchanged for certain vendors so all new elements should be added after
     *  this point.
     */

    /* O/S specific device context. -- Needed for Windows WDDM device callbacks and kernel mode thunks. */
    gctUINT64                   devCtxt;
    /* API type. -- Needed for Windows WDDM device kernel mode thunks to set ClientHint when a context is created. */
    gceAPI                      api;

} gcsHAL_INTERFACE;

#if PHYTIUM_PROFILER
typedef struct _gcsHAL_PROFILER_INTERFACE {
    /* Command code. */
    gceHAL_COMMAND_CODES        command;

    /* Hardware type. */
    gceHARDWARE_TYPE            hardwareType;

    /* Device index. */
    gctUINT32                   devIndex;

    /* Core index for current hardware type. */
    gctUINT32                   coreIndex;

    /* Status value. */
    gceSTATUS                   status;

    /* Ignore information from TSL when doing IO control */
    gctBOOL                     ignoreTLS;

    /* Union of command structures. */
    union profiler_u {
        gcsHAL_GET_PROFILE_SETTING              GetProfileSetting;
        gcsHAL_SET_PROFILE_SETTING              SetProfileSetting;
        gcsHAL_READ_PROFILER_REGISTER_SETTING   SetProfilerRegisterClear;
        gcsHAL_READ_ALL_PROFILE_REGISTERS       RegisterProfileData;
    } u;
#if gcdENABLE_MULTI_DEVICE_MANAGEMENT
    /* Device index. */
    gctUINT32                   devIndex;
#endif

} gcsHAL_PROFILER_INTERFACE;
#endif

/* State delta record. */
typedef struct _gcsSTATE_DELTA_RECORD *gcsSTATE_DELTA_RECORD_PTR;
typedef struct _gcsSTATE_DELTA_RECORD {
    /* State address. */
    gctUINT                     address;

    /* State mask. */
    gctUINT32                   mask;

    /* State data. */
    gctUINT32                   data;
} gcsSTATE_DELTA_RECORD;

/* State delta. */
typedef struct _gcsSTATE_DELTA {
    /* For debugging: the number of delta in the order of creation. */
    gctUINT                     num;

    /*
     * Main state delta ID. Every time state delta structure gets reinitialized,
     * main ID is incremented. If main state ID overflows, all map entry IDs get
     * reinitialized to make sure there is no potential erroneous match after
     * the overflow.
     */
    gctUINT                     id;

    /* The number of contexts pending modification by the delta. */
    gctINT                      refCount;

    /* Vertex element count for the delta buffer. */
    gctUINT                     elementCount;

    /* Number of states currently stored in the record array. */
    gctUINT                     recordCount;

    /* Record array; holds all modified states in gcsSTATE_DELTA_RECORD. */
    gctUINT64                   recordArray;

    /*
     * Map entry ID is used for map entry validation. If map entry ID does not
     * match the main state delta ID, the entry and the corresponding state are
     * considered not in use.
     */
    gctUINT64                   mapEntryID;
    gctUINT                     mapEntryIDSize;

    /*
     * If the map entry ID matches the main state delta ID, index points to
     * the state record in the record array.
     */
    gctUINT64                   mapEntryIndex;

    /* Previous and next state deltas in gcsSTATE_DELTA. */
    gctUINT64                   prev;
    gctUINT64                   next;
} gcsSTATE_DELTA;

typedef struct _gcsQUEUE {
    /* Pointer to next gcsQUEUE structure in gcsQUEUE. */
    gctUINT64                   next;

    /* Event information. */
    gcsHAL_INTERFACE            iface;
} gcsQUEUE;

/* A record chunk include multiple records to save allocation. */
typedef struct _gcsQUEUE_CHUNK {
    struct _gcsQUEUE_CHUNK      *next;

    gcsQUEUE                    record[16];
} gcsQUEUE_CHUNK;

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_driver_shared_h_ */

