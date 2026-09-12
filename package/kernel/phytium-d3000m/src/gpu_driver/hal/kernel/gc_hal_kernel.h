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


#ifndef __gc_hal_kernel_h_
#define __gc_hal_kernel_h_

#include "gc_hal.h"
#include "gc_hal_kernel_types.h"
#include "gc_hal_kernel_hardware.h"
#include "gc_hal_kernel_hardware_fe.h"
#include "gc_hal_driver.h"
#include "gc_hal_kernel_mutex.h"
#include "gc_hal_metadata.h"
#if gcdMMU_VERSION_1
#include "gc_hal_kernel_mmu.h"
#endif
#if gcdMMU_VERSION_2
#include "gc_hal_kernel_mmu_v2.h"
#endif

#if gcdHARDWARE_CONTEXT_SWITCH
#include "gc_hal_kernel_context.h"
#endif

#if gcdENABLE_SW_PREEMPTION
#include "gc_hal_kernel_preemption.h"
#endif

#if gcdSECURITY || gcdENABLE_TRUST_APPLICATION
#include "gc_hal_security_interface.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Create a process database that will contain all its allocations. */
gceSTATUS
gckKERNEL_CreateProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID);

/* Add a record to the process database. */
gceSTATUS
gckKERNEL_AddProcessDB(gckKERNEL Kernel,
                       gctUINT32 ProcessID,
                       gceDATABASE_TYPE Type,
                       gctPOINTER Pointer,
                       gctPHYS_ADDR Physical,
                       gctSIZE_T Size);

/* Remove a record to the process database. */
gceSTATUS
gckKERNEL_RemoveProcessDB(gckKERNEL Kernel,
                          gctUINT32 ProcessID,
                          gceDATABASE_TYPE Type,
                          gctPOINTER Pointer);

gceSTATUS
gckKERNEL_RemoveDatabaseFromList(gckKERNEL Kernel, gcsDATABASE_PTR Database, gctUINT32 ProcessID);

/* Destroy the process database. */
gceSTATUS
gckKERNEL_DestroyProcessDB(gckKERNEL Kernel, gctUINT32 ProcessID);

/* Find a record to the process database. */
gceSTATUS
gckKERNEL_FindProcessDB(gckKERNEL Kernel,
                        gctUINT32 ProcessID,
                        gctUINT32 ThreadID,
                        gceDATABASE_TYPE Type,
                        gctPOINTER Pointer,
                        gcsDATABASE_RECORD_PTR Record);

/* Query the process database. */
gceSTATUS
gckKERNEL_QueryProcessDB(gckKERNEL Kernel,
                         gctUINT32 ProcessID,
                         gctBOOL LastProcessID,
                         gceDATABASE_TYPE Type,
                         gcuDATABASE_INFO *Info);

/* Dump the process database. */
gceSTATUS
gckKERNEL_DumpProcessDB(gckKERNEL Kernel);

/* Dump the video memory usage for process specified. */
gceSTATUS
gckKERNEL_DumpVidMemUsage(gckKERNEL Kernel, gctINT32 ProcessID);

/* Dump GPU state. */
void
gckKERNEL_DumpState(gckKERNEL Kernel);

gceSTATUS
gckKERNEL_FindDatabase(gckKERNEL Kernel,
                       gctUINT32 ProcessID,
                       gctBOOL LastProcessID,
                       gcsDATABASE_PTR *Database);

gceSTATUS
gckKERNEL_FindHandleDatabase(gckKERNEL Kernel,
                             gctUINT32 ProcessID,
                             gctPOINTER *HandleDatabase,
                             gctPOINTER *HandleDatabaseMutex);

gceSTATUS
gckKERNEL_CreateIntegerDatabase(gckKERNEL Kernel,
                                gctUINT32 Capacity,
                                gctPOINTER *Database);

gceSTATUS
gckKERNEL_DestroyIntegerDatabase(gckKERNEL Kernel, gctPOINTER Database);

gceSTATUS
gckKERNEL_AllocateIntegerId(gctPOINTER Database, gctPOINTER Pointer, gctUINT32 *Id);

gceSTATUS
gckKERNEL_FreeIntegerId(gctPOINTER Database, gctUINT32 Id);

gceSTATUS
gckKERNEL_QueryIntegerId(gctPOINTER Database, gctUINT32 Id, gctPOINTER *Pointer);

/* Pointer rename  */
gctUINT32
gckKERNEL_AllocateNameFromPointer(gckKERNEL Kernel, gctPOINTER Pointer);

gctPOINTER
gckKERNEL_QueryPointerFromName(gckKERNEL Kernel, gctUINT32 Name);

gceSTATUS
gckKERNEL_DeleteName(gckKERNEL Kernel, gctUINT32 Name);

gceSTATUS
gckKERNEL_ConfigCluster(gckKERNEL Kernel, gcsHAL_INTERFACE *Interface);

/*******************************************************************************
 ********* Timer Management ****************************************************/
typedef struct _gcsTIMER *gcsTIMER_PTR;
typedef struct _gcsTIMER {
    /* Start and Stop time holders. */
    gctUINT64       startTime;
    gctUINT64       stopTime;
}
gcsTIMER;

/******************************************************************************
 ********************************** Structures ********************************
 ******************************************************************************/

/* gckDB object. */
struct _gckDB {
    /* Database management. */
    gcsDATABASE_PTR             db[16];
    gctPOINTER                  dbMutex;
    gcsDATABASE_PTR             freeDatabase;
    gcsDATABASE_RECORD_PTR      freeRecord;
    gcsDATABASE_PTR             lastDatabase;
    gctUINT32                   lastProcessID;
    gctUINT64                   lastIdle;
    gctUINT64                   idleTime;
    gctUINT64                   lastSlowdown;
    gctUINT64                   lastSlowdownIdle;

    gctPOINTER                  nameDatabase;
    gctPOINTER                  nameDatabaseMutex;

    gctPOINTER                  pointerDatabase;

    gcsLISTHEAD                 videoMemList;
    gctPOINTER                  videoMemListMutex;

    gctPOINTER                  refcnt;

#if gcdSUSPEND_RESUME_FROM_DISK
    gckVIDMEM_NODE              resumeNodeObject;
#endif
};

/* gckKERNEL object. */
struct _gckKERNEL {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Pointer to gckDEVICE object. */
    gckDEVICE                   device;

    /* Pointer to gckHARDWARE object. */
    gckHARDWARE                 hardware;

    /* Hardware type. */
    gceHARDWARE_TYPE            type;

    /* Core */
    gceCORE                     core;
    gctUINT                     chipID;

    /* Brothers */
    gctPOINTER                  atomBroCoreMask;

    /* Main command module, event and context. */
    gckCOMMAND                  command;
    gckCOMMAND                  sharedCommand;
    gckEVENT                    eventObj;
    gctPOINTER                  context;

    /* Async FE command and event modules. */
    gckCOMMAND                  asyncCommand;
    gckEVENT                    asyncEvent;

    /* Parity event. */
    gckEVENT                    parityEvent;

    /* Pointer to gckMMU object. */
    gckMMU                      mmu;
    gckMMU                      mmuCopy;

    /* If this core use shared MMU page table. */
    gctBOOL                     sharedPageTable;

    /* If this page table is per-process. */
    gctBOOL                     processPageTable;

    /* If use SW copy for per-process page table. */
    gctBOOL                     switchMmuByCopy;

    /* If this page table enables flat mapping. */
    gctBOOL                     flatMapping;

    /* Arom holding number of clients. */
    gctPOINTER                  atomClients;

    /* Database management. */
    gckDB                       db;
    gctBOOL                     dbCreated;

    gctUINT64                   resetTimeStamp;

    /* Pointer to gckEVENT object. */
    gcsTIMER                    timers[8];
    gctUINT32                   timeOut;

    /* Enable recovery. */
    gctBOOL                     recovery;

    /* Level of dump information after stuck. */
    gctUINT                     stuckDump;

    /* Timer to monitor GPU stuck. */
    gctPOINTER                  monitorTimer;

    /* Flag to quit monitor timer. */
    gctBOOL                     monitorTimerStop;

    /* Monitor states. */
    gctBOOL                     monitoring;
    gctUINT32                   lastCommitStamp;
    gctUINT32                   timer;
    gctADDRESS                  restoreAddress;
    gctINT32                    restoreMask;

    gckVIDMEM_BLOCK             vidMemBlock;
    gctPOINTER                  vidMemBlockMutex;

    gctADDRESS                  contiguousBaseAddresses[gcdSYSTEM_RESERVE_COUNT];
    gctADDRESS                  lowContiguousBaseAddress;
    gctADDRESS                  externalBaseAddress;
    gctADDRESS                  internalBaseAddress;
    gctADDRESS                  exclusiveBaseAddress;

    /* External shared SRAM. */
    gctADDRESS                  extSRAMBaseAddresses[gcvSRAM_EXT_COUNT];

    /* Per core SRAM description. */
    gctUINT32                   sRAMIndex;
    gckVIDMEM                   sRAMVidMem[gcvSRAM_INTER_COUNT];
    gctPHYS_ADDR                sRAMPhysical[gcvSRAM_INTER_COUNT];
    gctADDRESS                  sRAMBaseAddresses[gcvSRAM_INTER_COUNT];
    gctUINT32                   sRAMSizes[gcvSRAM_INTER_COUNT];
    gctBOOL                     sRAMPhysFaked[gcvSRAM_INTER_COUNT];
    gctUINT64                   sRAMLoopMode;

    gctUINT32                   timeoutPID;
    gctBOOL                     threadInitialized;
    gctBOOL                     killThread;
    gctPOINTER                  resetStatus;
    gctSEMAPHORE                sema;

    /* Device ID. */
    gctUINT32                   devID;

    /* In multi-core context or not. */
    gctBOOL                     inShared;

    gctUINT32                   totalMmuDescNum;
    gctUINT32                   nextMmuDescId;
    gctUINT32                   *mmuDescMap;
    gctPOINTER                  mmuDescMutex;

#if gcdMMU_VERSION_2
    /* Record the vmid if the process has been destroyed.
       Avoid dead vmid occupied the cache line. */
    gctUINT32                   *vmidWaitingFlush;
#endif

    /* vGPU type and id. */
    gceVGPU_TYPE                vGPUType;
    gctUINT32                   vGPUId;

    gctBOOL                     reserve32bitVA;

#if PHYTIUM_PROFILER
    gckPROFILER                 profiler;
#endif

#ifdef QNX_SINGLE_THREADED_DEBUGGING
    gctPOINTER                  debugMutex;
#endif

#if gcdDVFS
    gckDVFS                     dvfs;
#endif

#if gcdLINUX_SYNC_FILE
    gctHANDLE                   timeline;
#endif

#if gcdSECURITY || gcdENABLE_TRUST_APPLICATION
    gctUINT32                   securityChannel;
#endif

#if gcdENABLE_SW_PREEMPTION
    gctPOINTER                  priorityQueueMutex[gcdMAX_PRIORITY_QUEUE_NUM];
    gcsPRIORITY_QUEUE_PTR       priorityQueues[gcdMAX_PRIORITY_QUEUE_NUM];
    gctBOOL                     priorityDBCreated[gcdMAX_PRIORITY_QUEUE_NUM];
    gctSEMAPHORE                preemptSema;
    gcePREEMPTION_MODE          preemptionMode;
    gctBOOL                     killPreemptThread;
#endif
#if gcdENABLE_PHYTIUM_DEVFREQ
    unsigned long 		tmp_freq;
#endif
    gctPOINTER                  lastMsaaProduct;
};

struct _FrequencyHistory {
    gctUINT32 frequency;
    gctUINT32 count;
};

/* gckDVFS object. */
struct _gckDVFS {
    gckOS                    os;
    gckHARDWARE              hardware;
    gctPOINTER               timer;
    gctUINT32                pollingTime;
    gctBOOL                  stop;
    gctUINT32                totalConfig;
    gctUINT32                loads[8];
    gctUINT8                 currentScale;
    struct _FrequencyHistory frequencyHistory[16];
};

typedef struct _gcsFENCE *gckFENCE;
typedef struct _gcsFENCE {
    /* Pointer to required object. */
    gckKERNEL                   kernel;

    /* Fence location. */
    gckVIDMEM_NODE              videoMem;
    gctPOINTER                  logical;
    gctADDRESS                  address;

    gcsLISTHEAD                 waitingList;
    gctPOINTER                  mutex;
} gcsFENCE;

/* A sync point attached to fence. */
typedef struct _gcsFENCE_SYNC *gckFENCE_SYNC;
typedef struct _gcsFENCE_SYNC {
    /* Stamp of commit access this node. */
    gctUINT64                   commitStamp;

    /* Attach to waiting list. */
    gcsLISTHEAD                 head;

    gctPOINTER                  signal;

    gctBOOL                     inList;
} gcsFENCE_SYNC;

typedef struct _gcsCOMMAND_QUEUE {
    gctSIGNAL      signal;
    gckVIDMEM_NODE videoMem;
    gctPOINTER     logical;
    gctADDRESS     address;
    gcePOOL        pool;
} gcsCOMMAND_QUEUE;

/* gckCOMMAND object. */
struct _gckCOMMAND {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to required object. */
    gckKERNEL                   kernel;
    gckOS                       os;

    gceHW_FE_TYPE               feType;

    /* Number of bytes. */
    gctUINT32                   size;

    /* Current pipe select. */
    gcePIPE_SELECT              pipeSelect;

    /* Command queue running flag. */
    gctBOOL                     running;

    /* Idle flag and commit stamp. */
    gctBOOL                     idle;
    gctUINT64                   commitStamp;

    /* Command queue mutex. */
    gctPOINTER                  mutexQueue;

    /* Context switching mutex. */
    gctPOINTER                  mutexContext;

    /* Context sequence mutex. */
    gctPOINTER                  mutexContextSeq;

    /* Command queue power semaphore. */
    gctPOINTER                  powerSemaphore;

    /* Command queues. */
    gcsCOMMAND_QUEUE            queues[gcdCOMMAND_QUEUES];

    /* Current queue. */
    gckVIDMEM_NODE              videoMem;
    gctPOINTER                  logical;
    gctADDRESS                  address;
    gcePOOL                     pool;

    gctUINT32                   offset;
    gctINT                      index;

    /* The command queue is new. */
    gctBOOL                     newQueue;

    /* Context management. */
    gckCONTEXT                  currContext;

    /* state map is used to reord the stats register location in context buffer,
     * so we only need to know the largest address of ctx buffer used. But the
     * largest state address of ctx is different for smallbatch on or off, so
     * we need two buffer to record. */
    gctPOINTER                  stateMapSmallBatchOff;
    gctPOINTER                  stateMapSmallBatchOn;

    /* Pointer to last WAIT command. */
    /* Wait-Link FE only. */
    struct {
        gckVIDMEM_NODE          videoMem;
        gctUINT32               offset;
        gctPOINTER              logical;
        gctADDRESS              address;
        gctUINT32               size;
    }
    waitPos;

    /* MCFE. */
    gctUINT32                   totalSemaId;

    /*
     * freeSemaId   ------>  [pendingSema]
     * ^                    freePos -->+ +
     * |                    ^          | |
     * |                    |          | |
     * |                    nextPos ---+ |
     * |                                 |
     * |                                 |
     * |                                 v
     * nextSemaId <----------------------+
     *
     * Terminology:
     * 'freeSemaId': the top (final) free sema id can be used, signaled already.
     * 'nextSemaId': the next sema id to be used.
     * 'pendingSema': the used semaphores which are tracked in the ring.
     *
     * 3 Id sections:
     * Id(s) between 'nextSemaId' and 'freeSemaId':
     * Available for immediate use.
     *
     * Id(s) between 'freeSemaId' (exclusive) and 'pendingSema':
     * To be signed, will be available to use later.
     *
     * Id(s) between 'pendingSema' and 'nextSemaId':
     * The semaphores just used, not tracking in pendingSema ring.
     *
     * Conditions:
     * 'nextSemaId' = 'freeSemaId':
     * Using the final free semaphore, means the ring is full of used ones.
     *
     * 'freeSemaId' + 1 = 'nextSemaId':
     * Can use 'freeSema' + 1 to 'nextSema'(loop back), means empty ring,
     * ie, no used one.
     */

    /* MCFE semaphore id tracking ring. */
    gctUINT32                   nextSemaId;
    gctUINT32                   freeSemaId;

    gctUINT32                   semaMinThreshhold;

    /* pending semaphore id tracking ring. */
    struct {
        gctUINT32               semaId;
        gctSIGNAL               signal;
    } pendingSema[8];

    gctUINT32                   nextPendingPos;
    gctUINT32                   freePendingPos;

    /* semaphore id (array index) to handle value map. */
    gctUINT32                  *semaHandleMap;

    /*
     * Dirty channels, ie channels ever submitted commands.
     * Need sync to (send semaphore to) system channel.
     */
    gctUINT64                   dirtyChannel[2];

    /*
     * Sync channels, need sync from (wait semaphore from) the system
     * channel before its own jobs.
     */
    gctUINT64                   syncChannel[2];

    /* Command buffer alignment. */
    gctUINT32                   alignment;

    /* Commit counter. */
    gctPOINTER                  atomCommit;

    /* Kernel process ID. */
    gctUINT32                   kernelProcessID;

    gctUINT32                   kernelProcessAttached;

    gckFENCE                    fence;
#if gcdHARDWARE_CONTEXT_SWITCH
    gckFENCE                    contextFence;
    gctUINT64                   ctxSwitchStamp;
#endif

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    gctUINT                     wrapCount;
#endif

#if gcdRECORD_COMMAND
    gckRECORDER                 recorder;
#endif

    gctBOOL                     dummyDraw;
};

typedef struct _gcsEVENT       *gcsEVENT_PTR;

/* Structure holding one event to be processed. */
typedef struct _gcsEVENT {
    /* Pointer to next event in queue. */
    gcsEVENT_PTR                next;

    /* Event information. */
    gcsHAL_INTERFACE            info;

    /* Process ID owning the event. */
    gctUINT32                   processID;

#ifdef __QNXNTO__
    /* Kernel. */
    gckKERNEL                   kernel;
#endif

    gctBOOL                     fromKernel;

    gckMMU                      mmu;
} gcsEVENT;

/* Structure holding a list of events to be processed by an interrupt. */
typedef struct _gcsEVENT_QUEUE *gcsEVENT_QUEUE_PTR;
typedef struct _gcsEVENT_QUEUE {
    /* Time stamp. */
    gctUINT64                   stamp;

    /* Source of the event. */
    gceKERNEL_WHERE             source;

    /* Pointer to head of event queue. */
    gcsEVENT_PTR                head;

    /* Pointer to tail of event queue. */
    gcsEVENT_PTR                tail;

    /* Next list of events. */
    gcsEVENT_QUEUE_PTR          next;

    /* Current commit stamp. */
    gctUINT64                   commitStamp;
} gcsEVENT_QUEUE;

/*
 *   gcdREPO_LIST_COUNT defines the maximum number of event queues with different
 *   hardware module sources that may coexist at the same time. Only two sources
 *   are supported - gcvKERNEL_COMMAND and gcvKERNEL_PIXEL. gcvKERNEL_COMMAND
 *   source is used only for managing the kernel command queue and is only issued
 *   when the current command queue gets full. Since we commit event queues every
 *   time we commit command buffers, in the worst case we can have up to three
 *   pending event queues:
 *       - gcvKERNEL_PIXEL
 *       - gcvKERNEL_COMMAND (queue overflow)
 *       - gcvKERNEL_PIXEL
 */
#define gcdREPO_LIST_COUNT      3

#define gcdEVENT_QUEUE_COUNT    29

/* gckEVENT object. */
struct _gckEVENT {
    /* The object. */
    gcsOBJECT                   object;

    /* Pointer to required objects. */
    gckOS                       os;
    gckKERNEL                   kernel;

    /* Pointer to COMMAND object, either one of the 3. */
    gckCOMMAND                  command;

    /* Submit function pointer, different for different command module. */
    gceSTATUS                   (*submitEvent)(gckEVENT, gctBOOL, gctBOOL);

    /* Time stamp. */
    gctUINT64                   stamp;
    gctUINT32                   lastCommitStamp;

    /* Queue mutex. */
    gctPOINTER                  eventQueueMutex;

    /* Array of event queues. */
    gcsEVENT_QUEUE              queues[gcdEVENT_QUEUE_COUNT];
    gctINT32                    totalQueueCount;
    gctINT32                    freeQueueCount;
    gctUINT8                    lastID;

    /* Pending events. */
    gctPOINTER                  pending;

    /* List of free event structures and its mutex. */
    gcsEVENT_PTR                freeEventList;
    gctSIZE_T                   freeEventCount;
    gctPOINTER                  freeEventMutex;

    /* Event queues. */
    gcsEVENT_QUEUE_PTR          queueHead;
    gcsEVENT_QUEUE_PTR          queueTail;
    gcsEVENT_QUEUE_PTR          freeList;
    gcsEVENT_QUEUE              repoList[gcdREPO_LIST_COUNT];
    gctPOINTER                  eventListMutex;

    gctPOINTER                  submitTimer;

    gctINT                      notifyState;

#if gcdINTERRUPT_STATISTIC
    gctPOINTER                  interruptCount;
#endif
};

/* Event Attribute. */
typedef struct _gckEVENT_ATTR *gckEVENT_ATTR;
typedef struct _gckEVENT_ATTR {
    /* Submit requires one vacant event or not. */
    gctBOOL wait;
    /* It is in multi-core programming or not. */
    gctBOOL shared;
    /* It is called by power management or not.*/
    gctBOOL fromPower;
    /* It is broadcast the new commit arrives or not.*/
    gctBOOL broadcast;
} gcsEVENT_ATTR;

/* Construct a new gckEVENT object. */
gceSTATUS
gckEVENT_Construct(gckKERNEL Kernel, gckCOMMAND Command, gckEVENT *Event);

/* Destroy an gckEVENT object. */
gceSTATUS
gckEVENT_Destroy(gckEVENT Event);

/* Reserve the next available hardware event. */
gceSTATUS
gckEVENT_GetEvent(gckEVENT Event,
                  gctBOOL Wait,
                  gctUINT8 *EventID,
                  gceKERNEL_WHERE Source);

/* Add a new event to the list of events. */
gceSTATUS
gckEVENT_AddListEx(gckEVENT Event,
                   gcsHAL_INTERFACE_PTR Interface,
                   gceKERNEL_WHERE FromWhere,
                   gctBOOL AllocateAllowed,
                   gctBOOL FromKernel,
                   gctUINT32 ProcessID);

/* Add a new event to the list of events. */
gceSTATUS
gckEVENT_AddList(gckEVENT Event,
                 gcsHAL_INTERFACE_PTR Interface,
                 gceKERNEL_WHERE FromWhere,
                 gctBOOL AllocateAllowed,
                 gctBOOL FromKernel);

/* Schedule a FreeVideoMemory event. */
gceSTATUS
gckEVENT_FreeVideoMemory(gckEVENT Event,
                         gcuVIDMEM_NODE_PTR VideoMemory,
                         gceKERNEL_WHERE FromWhere);

/* Schedule a signal event. */
gceSTATUS
gckEVENT_Signal(gckEVENT Event, gctSIGNAL Signal, gceKERNEL_WHERE FromWhere);

/* Schedule an Unlock event. */
gceSTATUS
gckEVENT_Unlock(gckEVENT Event, gceKERNEL_WHERE FromWhere, gckMMU Mmu, gctPOINTER Node);

gceSTATUS
gckEVENT_CommitDone(gckEVENT Event,
                    gceKERNEL_WHERE FromWhere,
                    gckCONTEXT Context);

gceSTATUS
gckEVENT_Submit(gckEVENT Event, gckEVENT_ATTR EventAttr);

gceSTATUS
gckEVENT_Commit(gckEVENT Event,
                gcsQUEUE_PTR Queue,
                gctBOOL Forced,
                gctBOOL Submit,
                gctBOOL Shared);

/* Event callback routine. */
gceSTATUS
gckEVENT_Notify(gckEVENT Event, gctUINT32 IDs, gceEVENT_FAULT *Fault);

/* Event callback routine. */
gceSTATUS
gckEVENT_Interrupt(gckEVENT Event, gctUINT32 IDs);

gceSTATUS
gckEVENT_ParityNotify(gckEVENT Event);

gceSTATUS
gckEVENT_Dump(gckEVENT Event);

/* Free all events belonging to a process. */
gceSTATUS
gckEVENT_FreeProcess(gckEVENT Event, gctUINT32 ProcessID);

#if gcdENABLE_VIDEO_MEMORY_MIRROR
typedef struct _gcsVIDMEM_NODE_MIRROR {
    gceMIRROR_TYPE  type;
    gckVIDMEM_NODE  mirrorNode;
#if !gcdSTATIC_VIDEO_MEMORY_MIRROR
    gctUINT32       refCount;
# endif
} gcsVIDMEM_NODE_MIRROR;
#endif

/* gcuVIDMEM_NODE structure. */
typedef union _gcuVIDMEM_NODE {
    /* Allocated from gckVIDMEM. */
    struct _gcsVIDMEM_NODE_VIDMEM {
        /* Owner of this node. */
        gckVIDMEM               parent;

        /* Dual-linked list of nodes. */
        gcuVIDMEM_NODE_PTR      next;
        gcuVIDMEM_NODE_PTR      prev;

        /* Dual linked list of free nodes. */
        gcuVIDMEM_NODE_PTR      nextFree;
        gcuVIDMEM_NODE_PTR      prevFree;

        /* Information for this node. */
        gctSIZE_T               offset;

        gctADDRESS              address;
        gctSIZE_T               bytes;
        gctUINT32               alignment;

        /* Client virtual address. */
        gctPOINTER              logical;

        /* Process ID owning this memory. */
        gctUINT32               processID;

        /* Locked counter. */
        gctINT32                locked;

        /* Memory pool. */
        gcePOOL                 pool;

        /* Kernel virtual address. */
        gctPOINTER              kvaddr;

        /* mdl record pointer. */
        gctPHYS_ADDR            physical;

        /* Pointer to gckKERNEL object. */
        gckKERNEL               kernel;

        /* Information for this node. */
        /* Contiguously allocated? */
        gctBOOL                 contiguous;

        /* Customer private handle */
        gctUINT32               gid;

        /* Page table information. */
        gctSIZE_T               pageCount;

        gctPOINTER              pageTables[gcvCORE_COUNT];

        /* Virtual address. */
        gctADDRESS              addresses[gcvCORE_COUNT];

        /* Locked counter. */
        gctINT32                lockeds[gcvCORE_COUNT];

        /* If the virtual address needs to be within 32 bit. */
        gctBOOL                 lowVA;

        /* Video memory allocation type. */
        gceVIDMEM_TYPE          type;

        /* Secure GPU virtual address. */
        gctBOOL                 secure;

        gctBOOL                 onFault;

        /* If the node is allocated for user. */
        gctBOOL                 fromUser;

        /* Identifier. */
        gctINT                  id;

        /* Link in _gckMMU::nodeList. */
        gcsLISTHEAD             lockLink;

        /* This node is GPU read only. */
        gctBOOL                 gpuRO;
    } VidMem;

    struct _gcsVIDMEM_NODE_VIRTUAL_CHUNK {
        /* Owner of this chunk */
        gckVIDMEM_BLOCK         parent;

        /* Pointer to gckKERNEL object. */
        gckKERNEL               kernel;

        /* Dual-linked list of chunk. */
        gcuVIDMEM_NODE_PTR      next;
        gcuVIDMEM_NODE_PTR      prev;

        /* Dual linked list of free chunk. */
        gcuVIDMEM_NODE_PTR      nextFree;
        gcuVIDMEM_NODE_PTR      prevFree;

        /* Information for this chunk. */
        gctSIZE_T               offset;
        gctADDRESS              addresses[gcvCORE_COUNT];
        gctINT32                lockeds[gcvCORE_COUNT];

        gctSIZE_T               bytes;

        /* Mapped user logical */
        gctPOINTER              logical;

        /* Kernel virtual address. */
        gctPOINTER              kvaddr;

        gctUINT32               processID;

        /* If the node is allocated for user. */
        gctBOOL                 fromUser;

        gctINT                  id;

        /* Link in _gckMMU::nodeList. */
        gcsLISTHEAD             lockLink;
    } VirtualChunk;

} gcuVIDMEM_NODE;

/* gckVIDMEM object. */
struct _gckVIDMEM {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* mdl record pointer... a kmalloc address. Process agnostic. */
    gctPHYS_ADDR                physical;

    /* Information for this video memory heap. */
    gctPHYS_ADDR_T              physicalBase;
    gctSIZE_T                   bytes;
    gctSIZE_T                   freeBytes;
    gctSIZE_T                   minFreeBytes;

    /* caps inherit from its allocator, ~0u if allocator was not applicable. */
    gctUINT32                   capability;

    /* Mapping for each type of surface. */
    gctINT                      mapping[gcvVIDMEM_TYPE_COUNT];

    /* Sentinel nodes for up to 8 banks. */
    gcuVIDMEM_NODE              sentinel[8];

    /* Allocation threshold. */
    gctSIZE_T                   threshold;

    /* The heap mutex. */
    gctPOINTER                  mutex;
};

/* gckVIDMEM_BLOCK object. */
typedef struct _gcsVIDMEM_BLOCK {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* linked list of nodes. */
    gckVIDMEM_BLOCK             next;

    /* Contiguously allocated? */
    gctBOOL                     contiguous;

    /* Customer private handle */
    gctUINT32                   gid;

    /* mdl record pointer... a kmalloc address. Process agnostic. */
    gctPHYS_ADDR                physical;

    /* Information for this video memory virtual block. */
    gctSIZE_T                   bytes;
    gctSIZE_T                   freeBytes;

    /* 1M page count. */
    gctUINT32                   pageCount;
    gctUINT32                   fixedPageCount;

    /* Gpu virtual base of this video memory heap. */
    gctADDRESS                  addresses[gcvCORE_COUNT];
    gctPOINTER                  pageTables[gcvCORE_COUNT];

    gceVIDMEM_TYPE              type;

    /* Virtual chunk. */
    gcuVIDMEM_NODE              node;

    gctPOINTER                  mutex;

    gctBOOL                     secure;
    gctBOOL                     onFault;

    /* If the virtual address needs to be within 32 bit. */
    gctBOOL                     lowVA;

    /* If the video memory block is allocated for user space. */
    /* Only used when pre-process page table is enabled. */
    gctBOOL                     fromUser;

    /* Only used when pre-process page table is enabled. */
    gctUINT32                   processID;

    gctBOOL                     cacheable;
} gcsVIDMEM_BLOCK;

typedef struct _gcsVIDMEM_PIDINFO {
    gctUINT32 pid;
    gctPOINTER ref;
    struct _gcsVIDMEM_PIDINFO *next;
} gcsVIDMEM_PIDINFO;

typedef struct _gcsVIDMEM_PIDINFO *gckVIDMEM_PIDINFO;

typedef struct _gcsVIDMEM_NODE {
    _VIV_VIDMEM_METADATA metadata;

    /* Pointer to gcuVIDMEM_NODE. */
    gcuVIDMEM_NODE_PTR          node;

    /* Pointer to gckKERNEL object. */
    gckKERNEL                   kernel;

    /* Mutex to protect node. */
    gctPOINTER                  mutex;

    /* Reference count. */
    gctPOINTER                  reference;

    /* Name for client to import. */
    gctUINT32                   name;

    /* Link in _gckDB::videoMemList. */
    gcsLISTHEAD                 link;

    /* dma_buf */
    gctPOINTER                  dmabuf;

    /* fd */
    gctINT                      fd;

    /* Video memory allocation type. */
    gceVIDMEM_TYPE              type;

    /* Pool from which node is allocated. */
    gcePOOL                     pool;

    gcsFENCE_SYNC               sync[gcvENGINE_GPU_ENGINE_COUNT];

    /* For DRM usage */
    gctUINT64                   timeStamp;
    gckVIDMEM_NODE              tsNode;
    gctUINT32                   tilingMode;
    gctUINT32                   tsMode;
    gctUINT32                   tsCacheMode;
    gctUINT64                   clearValue;

    /* Allocation flag */
    gctUINT32                   flag;

    gckVIDMEM_PIDINFO           pidInfo;

#if gcdCAPTURE_ONLY_MODE
    gctSIZE_T                   captureSize;
    gctPOINTER                  captureLogical;
#endif

#if gcdENABLE_VIDEO_MEMORY_MIRROR
    gcsVIDMEM_NODE_MIRROR       mirror;
#endif

#if gcdSUSPEND_RESUME_FROM_DISK
    gctSIZE_T                   resumeOffset;
#endif
} gcsVIDMEM_NODE;

typedef struct _gcsVIDMEM_HANDLE *gckVIDMEM_HANDLE;
typedef struct _gcsVIDMEM_HANDLE {
    /* Pointer to gckVIDMEM_NODE. */
    gckVIDMEM_NODE              node;

    /* Handle for current process. */
    gctUINT32                   handle;

    /* Reference count for this handle. */
    gctPOINTER                  reference;
} gcsVIDMEM_HANDLE;

typedef struct _gcsSHBUF *gcsSHBUF_PTR;
typedef struct _gcsSHBUF {
    /* ID. */
    gctUINT32                   id;

    /* Reference count. */
    gctPOINTER                  reference;

    /* Data size. */
    gctUINT32                   size;

    /* Data. */
    gctPOINTER                  data;
} gcsSHBUF;

typedef struct _gcsCORE_INFO {
    gceHARDWARE_TYPE type;
    gceCORE          core;
    gckKERNEL        kernel;
    gctUINT32        chipID;
} gcsCORE_INFO;

typedef struct _gcsCORE_LIST {
    gckKERNEL   kernels[gcvCORE_COUNT];
    gctUINT32   num;
} gcsCORE_LIST;

#if gcdENABLE_CLEAR_FENCE
typedef struct _gcsUSER_FENCE_INFO {
    gctADDRESS addr;
    gctUINT64  fenceValue;
    gctUINT64  recordId;

    /* Link in _gcsDEVICE::fenceList */
    gcsLISTHEAD fenceLink;
    gctINT32   tid;
    gctBOOL    use64BitFence;
} gcsUSER_FENCE_INFO;
#endif

/* A gckDEVICE is a group of cores (gckKERNEL in software). */
typedef struct _gcsDEVICE {
    /* Global device id in all the platforms. */
    gctUINT                     id;
    /* Platform id that the device locate. */
    gctUINT                     platformIndex;

    gcsCORE_INFO                coreInfoArray[gcvCORE_COUNT];
    gctUINT32                   coreNum;
    gcsCORE_LIST                map[gcvHARDWARE_NUM_TYPES];
    gceHARDWARE_TYPE            defaultHwType;

    gckOS                       os;

    /* Process resource database. */
    gckDB                       database;

    /* Same hardware type of one platform device shares one MMU. */
    gckMMU                      mmus[gcvHARDWARE_NUM_TYPES];

    gckCOMMAND                  commands[gcvHARDWARE_NUM_TYPES];

    gckKERNEL                   kernels[gcvCORE_COUNT];

    gctUINT32                   extSRAMIndex;

    /* Physical address of internal SRAMs. */
    gctPHYS_ADDR_T              sRAMBases[gcvCORE_COUNT][gcvSRAM_INTER_COUNT];
    /* Internal SRAMs' size. */
    gctUINT32                   sRAMSizes[gcvCORE_COUNT][gcvSRAM_INTER_COUNT];
    /* GPU/VIP virtual address of internal SRAMs. */
    gctADDRESS                  sRAMBaseAddresses[gcvCORE_COUNT][gcvSRAM_INTER_COUNT];
    gctBOOL                     sRAMPhysFaked[gcvCORE_COUNT][gcvSRAM_INTER_COUNT];

    /* CPU physical address of external SRAMs. */
    gctPHYS_ADDR_T              extSRAMBases[gcvSRAM_EXT_COUNT];
    /* GPU physical address of external SRAMs. */
    gctPHYS_ADDR_T              extSRAMGPUBases[gcvSRAM_EXT_COUNT];
    /* External SRAMs' size. */
    gctUINT32                   extSRAMSizes[gcvSRAM_EXT_COUNT];
    /* GPU/VIP virtual address of external SRAMs. */
    gctADDRESS                  extSRAMBaseAddresses[gcvSRAM_EXT_COUNT];
    /* MDL. */
    gctPHYS_ADDR                extSRAMPhysicals[gcvSRAM_EXT_COUNT];
    /* IntegerId. */
    gctUINT32                   extSRAMGPUPhysNames[gcvSRAM_EXT_COUNT];
    /* Vidmem. */
    gckVIDMEM                   extSRAMVidMems[gcvSRAM_EXT_COUNT];

    /* Show SRAM mapping info or not. */
    gctUINT                     showSRAMMapInfo;

    /* Mutex to make sure stuck dump for multiple cores doesn't interleave. */
    gctPOINTER                  stuckDumpMutex;

    /* Mutex for multi-core combine mode command submission */
    gctPOINTER                  commitMutex;

    /* Mutex for per-device power management. */
    gctPOINTER                  powerMutex;

    /* Mutex for recovery all core */
    gctPOINTER                  recoveryMutex;

    /* Which memory this core uses. */
    gctUINT32                   memIndex;

    gctPHYS_ADDR_T              contiguousBases[gcdSYSTEM_RESERVE_COUNT];
    gctSIZE_T                   contiguousSizes[gcdSYSTEM_RESERVE_COUNT];
    gctPHYS_ADDR                contiguousPhysicals[gcdSYSTEM_RESERVE_COUNT];
    gctUINT32                   contiguousPhysNames[gcdSYSTEM_RESERVE_COUNT];
    gckVIDMEM                   contiguousVidMems[gcdSYSTEM_RESERVE_COUNT];

    gctPHYS_ADDR_T              lowContiguousBase;
    gctSIZE_T                   lowContiguousSize;

    gctPHYS_ADDR_T              exclusiveBase;
    gctSIZE_T                   exclusiveSize;
    gctPHYS_ADDR                exclusivePhysical;
    gctUINT32                   exclusivePhysName;
    gctPOINTER                  exclusiveLogical;
    gckVIDMEM                   exclusiveVidMem;

    gctPHYS_ADDR_T              externalBase;
    gctSIZE_T                   externalSize;
    gctPHYS_ADDR                externalPhysical;
    gctUINT32                   externalPhysName;
    gctPOINTER                  externalLogical;
    gckVIDMEM                   externalVidMem;

    /* IRQ management. */
    gctINT                      irqLines[gcvCORE_COUNT];
    gctBOOL                     isrInitializeds[gcvCORE_COUNT];
    gctPOINTER                  isrThread[gcvCORE_COUNT];
    gctBOOL                     killIsrThread;

    /* Register memory. */
    gctPOINTER                  registerBases[gcvCORE_COUNT];
    gctSIZE_T                   registerSizes[gcvCORE_COUNT];
    gctPOINTER                  registerBasesMapped[gcvCORE_COUNT];

    /* By request_mem_region. */
    gctUINT64                   requestedRegisterMemBases[gcvCORE_COUNT];
    gctSIZE_T                   requestedRegisterMemSizes[gcvCORE_COUNT];

    /* Thread management. */
    gctPOINTER                  threadCtxts[gcvCORE_COUNT];
    gctBOOL                     threadInitializeds[gcvCORE_COUNT];

    /* States before suspend. */
    gceCHIPPOWERSTATE           statesStored[gcvCORE_COUNT];

    /* Which video memory is for device. */
    gctUINT32                   vidMemIndex;

    /* Which SRAM is for device. */
    gctUINT32                   sRAMIndex;

    /* Core count configuration. */
    gctUINT32                   configCoreCount;

    /* Platform device structure. */
    gctPOINTER                  dev;

    /* Print the memory info or not. */
    gctBOOL                     showMemInfo;

    /* Process page table enable or not. */
    gctBOOL                     processPageTable;

    gctBOOL                     reserve32bitVA;

    gctBOOL                     lockLargeVA;

#if gcdENABLE_SW_PREEMPTION
    gctPOINTER                  atomPriorityID;
    gctPOINTER                  preemptThread[gcvCORE_COUNT];
    gctBOOL                     preemptThreadInits[gcvCORE_COUNT];
#endif

#if gcdENABLE_CLEAR_FENCE
    gcsLISTHEAD                 fenceList;
    gctPOINTER                  fenceListMutex;
    gctPOINTER                  fenceIdr;
    gctPOINTER                  fenceIdrLock;
#endif
} gcsDEVICE;

/* video memory pool functions. */
/* Construct a new gckVIDMEM object. */
gceSTATUS
gckVIDMEM_Construct(gckOS Os,
                    gctPHYS_ADDR_T PhysicalBase,
                    gctSIZE_T Bytes,
                    gctSIZE_T Threshold,
                    gctSIZE_T Banking,
                    gckVIDMEM *Memory);

/* Destroy an gckVDIMEM object. */
gceSTATUS
gckVIDMEM_Destroy(gckVIDMEM Memory);

gceSTATUS
gckVIDMEM_HANDLE_Allocate(gckKERNEL Kernel, gckVIDMEM_NODE Node, gctUINT32 *Handle);

gceSTATUS
gckVIDMEM_HANDLE_Reference(gckKERNEL Kernel, gctUINT32 ProcessID, gctUINT32 Handle);

gceSTATUS
gckVIDMEM_HANDLE_Dereference(gckKERNEL Kernel, gctUINT32 ProcessID, gctUINT32 Handle);

gceSTATUS
gckVIDMEM_HANDLE_Lookup(gckKERNEL Kernel,
                        gctUINT32 ProcessID,
                        gctUINT32 Handle,
                        gckVIDMEM_NODE *Node);

gceSTATUS
gckVIDMEM_HANDLE_Lookup2(gckKERNEL Kernel,
                         gcsDATABASE_PTR Database,
                         gctUINT32 Handle,
                         gckVIDMEM_NODE *Node);

/* video memory node functions. */
gceSTATUS
gckVIDMEM_NODE_AllocateLinear(gckKERNEL Kernel,
                              gckVIDMEM VideoMemory,
                              gcePOOL Pool,
                              gceVIDMEM_TYPE Type,
                              gctUINT32 Flag,
                              gctUINT32 Alignment,
                              gctBOOL Specified,
                              gctSIZE_T *Bytes,
                              gckVIDMEM_NODE *NodeObject);

gceSTATUS
gckVIDMEM_NODE_AllocateVirtual(gckKERNEL Kernel,
                               gcePOOL Pool,
                               gceVIDMEM_TYPE Type,
                               gctUINT32 Flag,
                               gctSIZE_T *Bytes,
                               gckVIDMEM_NODE *NodeObject);

gceSTATUS
gckVIDMEM_NODE_AllocateVirtualChunk(gckKERNEL Kernel,
                                    gcePOOL Pool,
                                    gceVIDMEM_TYPE Type,
                                    gctUINT32 Flag,
                                    gctSIZE_T *Bytes,
                                    gckVIDMEM_NODE *NodeObject);

gceSTATUS
gckVIDMEM_NODE_Reference(gckKERNEL Kernel, gckVIDMEM_NODE Node);

gceSTATUS
gckVIDMEM_NODE_Dereference(gckKERNEL Kernel, gckVIDMEM_NODE Node);

gceSTATUS
gckVIDMEM_NODE_DereferenceEx(gckKERNEL Kernel, gckVIDMEM_NODE Node, gctUINT32 ProcessID);

gceSTATUS
gckVIDMEM_NODE_GetReference(gckKERNEL Kernel,
                            gckVIDMEM_NODE NodeObject,
                            gctINT32 *ReferenceCount);

gceSTATUS
gckVIDMEM_NODE_Lock(gckKERNEL Kernel,
                    gckVIDMEM_NODE NodeObject,
                    gctADDRESS *Address);

gceSTATUS
gckVIDMEM_NODE_Unlock(gckKERNEL Kernel,
                      gckVIDMEM_NODE NodeObject,
                      gckMMU Mmu,
                      gctBOOL *Asynchroneous);

gceSTATUS
gckVIDMEM_NODE_CleanCache(gckKERNEL Kernel,
                          gckVIDMEM_NODE NodeObject,
                          gctSIZE_T Offset,
                          gctPOINTER Logical,
                          gctSIZE_T Bytes);

gceSTATUS
gckVIDMEM_NODE_InvalidateCache(gckKERNEL Kernel,
                               gckVIDMEM_NODE NodeObject,
                               gctSIZE_T Offset,
                               gctPOINTER Logical,
                               gctSIZE_T Bytes);

gceSTATUS
gckVIDMEM_NODE_GetLockCount(gckKERNEL Kernel,
                            gckVIDMEM_NODE NodeObject,
                            gctINT32 *LockCount);

gceSTATUS
gckVIDMEM_NODE_LockCPU(gckKERNEL Kernel,
                       gckVIDMEM_NODE NodeObject,
                       gctBOOL Cacheable,
                       gctBOOL FromUser,
                       gctPOINTER *Logical);

gceSTATUS
gckVIDMEM_NODE_UnlockCPU(gckKERNEL Kernel,
                         gckVIDMEM_NODE NodeObject,
                         gctUINT32 ProcessID,
                         gctBOOL FromUser,
                         gctBOOL Defer);

gceSTATUS
gckVIDMEM_NODE_GetCPUPhysical(gckKERNEL Kernel,
                              gckVIDMEM_NODE NodeObject,
                              gctSIZE_T Offset,
                              gctPHYS_ADDR_T *PhysicalAddress);

gceSTATUS
gckVIDMEM_NODE_GetGPUPhysical(gckKERNEL Kernel,
                              gckVIDMEM_NODE NodeObject,
                              gctSIZE_T Offset,
                              gctPHYS_ADDR_T *PhysicalAddress);

gceSTATUS
gckVIDMEM_NODE_GetGid(gckKERNEL Kernel,
                      gckVIDMEM_NODE NodeObject,
                      gctUINT32 *Gid);

gceSTATUS
gckVIDMEM_NODE_GetSize(gckKERNEL Kernel,
                       gckVIDMEM_NODE NodeObject,
                       gctSIZE_T *Size);

gceSTATUS
gckVIDMEM_NODE_GetType(gckKERNEL Kernel,
                       gckVIDMEM_NODE NodeObject,
                       gceVIDMEM_TYPE *Type,
                       gcePOOL *Pool);

gceSTATUS
gckVIDMEM_NODE_Export(gckKERNEL Kernel,
                      gckVIDMEM_NODE NodeObject,
                      gctINT32 Flags,
                      gctPOINTER *DmaBuf,
                      gctINT32 *FD);

gceSTATUS
gckVIDMEM_NODE_Name(gckKERNEL Kernel,
                    gckVIDMEM_NODE NodeObject,
                    gctUINT32 *Name);

gceSTATUS
gckVIDMEM_NODE_Import(gckKERNEL Kernel,
                      gctUINT32 Name,
                      gckVIDMEM_NODE *NodeObject);

gceSTATUS
gckVIDMEM_NODE_GetFd(gckKERNEL Kernel,
                     gckVIDMEM_NODE NodeObject,
                     gctBOOL Exported,
                     gctINT *Fd);

gceSTATUS
gckVIDMEM_NODE_WrapUserMemory(gckKERNEL Kernel,
                              gcsUSER_MEMORY_DESC_PTR Desc,
                              gceVIDMEM_TYPE Type,
                              gckVIDMEM_NODE *NodeObject,
                              gctUINT64 *Bytes);

gceSTATUS
gckVIDMEM_NODE_SetCommitStamp(gckKERNEL Kernel,
                              gceENGINE Engine,
                              gckVIDMEM_NODE NodeObject,
                              gctUINT64 CommitStamp);

gceSTATUS
gckVIDMEM_NODE_GetCommitStamp(gckKERNEL Kernel,
                              gceENGINE Engine,
                              gckVIDMEM_NODE NodeObject,
                              gctUINT64_PTR CommitStamp);

gceSTATUS
gckVIDMEM_NODE_Find(gckKERNEL Kernel,
                    gctADDRESS Address,
                    gckVIDMEM_NODE *NodeObject,
                    gctSIZE_T *Offset);

gceSTATUS
gckVIDMEM_NODE_IsContiguous(gckKERNEL Kernel,
                            gckVIDMEM_NODE NodeObject,
                            gctBOOL *Contiguous);

gceSTATUS
gckVIDMEM_NODE_GetMemoryHandle(gckKERNEL Kernel,
                               gckVIDMEM_NODE NodeObject,
                               gctPOINTER *MemoryHandle);

gceSTATUS
gckVIDMEM_NODE_GetMapKernel(gckKERNEL Kernel,
                            gckVIDMEM_NODE NodeObject,
                            gctPOINTER *KernelMap);

gceSTATUS
gckVIDMEM_NODE_GetOffset(gckKERNEL Kernel,
                         gckVIDMEM_NODE NodeObject,
                         gctSIZE_T *Offset);

#if gcdSUSPEND_RESUME_FROM_DISK
gceSTATUS
gckVIDMEM_AllocateResumeMemory(gckKERNEL Kernel,
                               gctSIZE_T Bytes);

gceSTATUS
gckVIDMEM_DestroyResumeMemory(gckKERNEL Kernel);

gceSTATUS
gckVIDMEM_CopyMemoryforResume(gckKERNEL Kernel);

gceSTATUS
gckVIDMEM_RestoreMemoryforResume(gckKERNEL Kernel);
#endif

gceSTATUS
gckOS_CreateKernelMapping(gckOS Os,
                          gctPHYS_ADDR Physical,
                          gctSIZE_T Offset,
                          gctSIZE_T Bytes,
                          gctPOINTER *Logical);

gceSTATUS
gckOS_DestroyKernelMapping(gckOS Os, gctPHYS_ADDR Physical, gctPOINTER Logical);

gceSTATUS
gckOS_GetFd(gctSTRING Name, gcsFDPRIVATE_PTR Private, gctINT *Fd);

/*******************************************************************************
 **
 **  gckOS_ReadMappedPointer
 **
 **  Read pointer mapped from user pointer which returned by gckOS_MapUserPointer.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to an gckOS object.
 **
 **      gctPOINTER Address
 **          Pointer returned by gckOS_MapUserPointer.
 **
 **      gctUINT32_PTR Data
 **          Pointer to hold 32 bits data.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckOS_ReadMappedPointer(gckOS Os, gctPOINTER Address, gctUINT32_PTR Data);

gceSTATUS
gckKERNEL_AttachProcess(gckKERNEL Kernel, gctBOOL Attach);

gceSTATUS
gckKERNEL_AttachProcessEx(gckKERNEL Kernel, gctBOOL Attach, gctUINT32 PID);

gceSTATUS
gckKERNEL_AllocateVideoMemory(gckKERNEL Kernel,
                              gctUINT32 Alignment,
                              gceVIDMEM_TYPE Type,
                              gctUINT32 Flag,
                              gctSIZE_T *Bytes,
                              gcePOOL *Pool,
                              gckVIDMEM_NODE *NodeObject);

/* Config power management form dispatch */
gceSTATUS
gckKERNEL_ConfigPowerManagement(gckKERNEL Kernel, gcsHAL_INTERFACE *Interface);

gceSTATUS
gckHARDWARE_QchannelPowerControl(gckHARDWARE Hardware,
                                 gctBOOL ClockState,
                                 gctBOOL PowerState);

gceSTATUS
gckHARDWARE_QchannelBypass(gckHARDWARE Hardware, gctBOOL Enable);

gceSTATUS
gckHARDWARE_QueryIdle(gckHARDWARE Hardware, gctBOOL_PTR IsIdle);

gceSTATUS
gckHARDWARE_WaitFence(gckHARDWARE Hardware,
                      gctPOINTER Logical,
                      gctUINT64 FenceData,
                      gctADDRESS FenceAddress,
                      gctUINT32 *Bytes);

gceSTATUS
gckHARDWARE_UpdateContextID(gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_QueryMcfe(gckHARDWARE Hardware,
                      const gceMCFE_CHANNEL_TYPE * Channels[],
                      gctUINT32 *Count);

gceSTATUS
gckHARDWARE_QchannelFlushCache(gckHARDWARE Hardware);

#if gcdSECURITY || gcdENABLE_TRUST_APPLICATION
gceSTATUS
gckKERNEL_SecurityAllocateSecurityMemory(IN gckKERNEL  Kernel,
                                         IN gctUINT32  Bytes,
                                         OUT gctUINT32 *Handle);
#endif

#if gcdSECURITY
gceSTATUS
gckKERNEL_SecurityOpen(gckKERNEL Kernel, gctUINT32 GPU, gctUINT32 *Channel);

/*
 ** Close a security service channel
 */
gceSTATUS
gckKERNEL_SecurityClose(gctUINT32 Channel);

/*
 ** Security service interface.
 */
gceSTATUS
gckKERNEL_SecurityCallService(gctUINT32 Channel, gcsTA_INTERFACE *Interface);

gceSTATUS
gckKERNEL_SecurityStartCommand(gckKERNEL Kernel);

gceSTATUS
gckKERNEL_SecurityExecute(gckKERNEL Kernel, gctPOINTER Buffer, gctUINT32 Bytes);

gceSTATUS
gckKERNEL_SecurityMapMemory(gckKERNEL Kernel,
                            gctUINT32 *PhysicalArray,
                            gctUINT32 PageCount,
                            gctADDRESS *GPUAddress);

gceSTATUS
gckKERNEL_SecurityUnmapMemory(gckKERNEL Kernel,
                              gctADDRESS GPUAddress,
                              gctUINT32 PageCount);

#endif

#if gcdENABLE_TRUST_APPLICATION
gceSTATUS
gckKERNEL_SecurityOpen(gckKERNEL Kernel, gctUINT32 GPU, gctUINT32 *Channel);

/*
 ** Close a security service channel
 */
gceSTATUS
gckKERNEL_SecurityClose(gctUINT32 Channel);

/*
 ** Security service interface.
 */
gceSTATUS
gckKERNEL_SecurityCallService(gctUINT32 Channel,
                              gcsTA_INTERFACE *Interface);

gceSTATUS
gckKERNEL_SecurityStartCommand(gckKERNEL Kernel,
                               gctADDRESS Address,
                               gctUINT32 Bytes);

gceSTATUS
gckKERNEL_SecurityMapMemory(gckKERNEL Kernel,
                            gctUINT32 *PhysicalArray,
                            gctPHYS_ADDR_T Physical,
                            gctUINT32 PageCount,
                            gctADDRESS *GPUAddress);

gceSTATUS
gckKERNEL_SecurityUnmapMemory(gckKERNEL Kernel,
                              gctADDRESS GPUAddress,
                              gctUINT32 PageCount);

gceSTATUS
gckKERNEL_SecurityDumpMMUException(gckKERNEL Kernel);

gceSTATUS
gckKERNEL_ReadMMUException(gckKERNEL Kernel,
                           gctUINT32_PTR MMUStatus,
                           gctUINT32_PTR MMUException);

gceSTATUS
gckKERNEL_HandleMMUException(gckKERNEL Kernel,
                             gctUINT32 MMUStatus,
                             gctPHYS_ADDR_T Physical,
                             gctADDRESS GPUAddres);
#endif

gceSTATUS
gckKERNEL_CreateShBuffer(gckKERNEL Kernel,
                         gctUINT32 Size,
                         gctSHBUF *ShBuf);

gceSTATUS
gckKERNEL_DestroyShBuffer(gckKERNEL Kernel, gctSHBUF ShBuf);

gceSTATUS
gckKERNEL_MapShBuffer(gckKERNEL Kernel, gctSHBUF ShBuf);

gceSTATUS
gckKERNEL_WriteShBuffer(gckKERNEL Kernel,
                        gctSHBUF ShBuf,
                        gctPOINTER UserData,
                        gctUINT32 ByteCount);

gceSTATUS
gckKERNEL_ReadShBuffer(gckKERNEL Kernel,
                       gctSHBUF ShBuf,
                       gctPOINTER UserData,
                       gctUINT32 ByteCount,
                       gctUINT32 *BytesRead);

gceSTATUS
gckKERNEL_GetHardwareType(gckKERNEL Kernel, gceHARDWARE_TYPE *Type);

#if gcdENABLE_MP_SWITCH
gceSTATUS
gckKERNEL_DetectMpModeSwitch(gckKERNEL Kernel,
                             gceMULTI_PROCESSOR_MODE Mode,
                             gctUINT32 *SwitchMpMode);
#endif

gceSTATUS
gckKERNEL_CancelJob(gckKERNEL Kernel);

gceSTATUS
gckKERNEL_FindProcessMMU(gckKERNEL Kernel, gctUINT32 ProcessID, gckMMU *Mmu);

gceSTATUS
gckKERNEL_GetCurrentMMU(gckKERNEL Kernel, gctBOOL FromUser,
                        gctUINT32 ProcessID, gckMMU *Mmu);

gceSTATUS
gckKERNEL_SwitchMMU(gckKERNEL Kernel, gctBOOL Shared, gckMMU Mmu);

/*******************************************************************************
 ******************************* gckCONTEXT Object *****************************
 ******************************************************************************/

gceSTATUS
gckCONTEXT_Construct(gckOS Os,
                     gckHARDWARE Hardware,
                     gctUINT32 ProcessID,
                     gctBOOL Shared,
                     gctBOOL SmallBatch,
                     gckCONTEXT *Context);

gceSTATUS
gckCONTEXT_Destroy(gckCONTEXT Context);

gceSTATUS
gckCONTEXT_Update(gckCONTEXT Context,
                  gctUINT32 ProcessID,
                  gcsSTATE_DELTA_PTR StateDelta);

gceSTATUS
gckCONTEXT_MapBuffer(gckCONTEXT Context, gctUINT64 *Logicals, gctUINT32 *Bytes);

#if gcdHARDWARE_CONTEXT_SWITCH
gceSTATUS
gckCONTEXT_HWSwitchWriteCommand(gckHARDWARE Hardware,
    gckCONTEXT Context,
    gctPOINTER Logical,
    gctBOOL SwitchMMU,
    gctBOOL SwitchProbe,
    gceCONTEXT_SWITCH_OPERATION Operation,
    gctUINT32 *Bytes);

gceSTATUS
gckCONTEXT_HWSwitchAddFence(gckHARDWARE Hardware,
    gctPOINTER Logical,
    gctADDRESS FenceAddress,
    gctUINT64 FenceData,
    gctUINT32 *Bytes);

gceSTATUS
gckCONTEXT_HWSwitchCheckFence(gckCONTEXT Context, gctUINT32 ProcessID);

gceSTATUS
gckCOMMAND_HWSwitchContext(gckCOMMAND Command,
    gckHARDWARE Hardware,
    gckCONTEXT OldContext,
    gckCONTEXT NewContext);
#endif

void
gckQUEUE_Enqueue(gckQUEUE LinkQueue, gcuQUEUEDATA *Data);

void
gckQUEUE_GetData(gckQUEUE LinkQueue, gctUINT32 Index, gcuQUEUEDATA **Data);

gceSTATUS
gckQUEUE_Allocate(gckOS Os, gckQUEUE Queue, gctUINT32 Size);

gceSTATUS
gckQUEUE_Free(gckOS Os, gckQUEUE Queue);

/*******************************************************************************
 ****************************** gckRECORDER Object *****************************
 ******************************************************************************/
gceSTATUS
gckRECORDER_Construct(gckOS Os,
                      gckHARDWARE Hardware,
                      gckRECORDER *Recorder);

gceSTATUS
gckRECORDER_Destory(gckOS Os, gckRECORDER Recorder);

void
gckRECORDER_AdvanceIndex(gckRECORDER Recorder, gctUINT64 CommitStamp);

void
gckRECORDER_Record(gckRECORDER Recorder,
                   gctUINT8_PTR CommandBuffer,
                   gctUINT32 CommandBytes,
                   gctUINT8_PTR ContextBuffer,
                   gctUINT32 ContextBytes);

void
gckRECORDER_Dump(gckRECORDER Recorder);

gceSTATUS
gckRECORDER_UpdateMirror(gckRECORDER Recorder, gctUINT32 State, gctUINT32 Data);

/*******************************************************************************
 ****************************** gckPARSER Object *****************************
 ******************************************************************************/
gceSTATUS
gckPARSER_Parse(gckPARSER Parser, gctUINT8_PTR Buffer, gctUINT32 Bytes);

gceSTATUS
gckPARSER_RegisterCommandHandler(gckPARSER Parser, gckPARSER_HANDLER Handler);

gceSTATUS
gckPARSER_Construct(gckOS Os, gckPARSER_HANDLER Handler, gckPARSER *Parser);

void
gckPARSER_Destroy(gckOS Os, gckPARSER Parser);

/******************************************************************************
 ****************************** gckCOMMAND Object *****************************
 ******************************************************************************/

/* Construct a new gckCOMMAND object. */
gceSTATUS
gckCOMMAND_Construct(gckKERNEL Kernel,
                     gceHW_FE_TYPE FeType,
                     gckCOMMAND *Command);

/* Destroy an gckCOMMAND object. */
gceSTATUS
gckCOMMAND_Destroy(gckCOMMAND Command);

/* Acquire command queue synchronization objects. */
gceSTATUS
gckCOMMAND_EnterCommit(gckCOMMAND Command, gctBOOL FromPower);

/* Release command queue synchronization objects. */
gceSTATUS
gckCOMMAND_ExitCommit(gckCOMMAND Command, gctBOOL FromPower);

/* Start the command queue. */
gceSTATUS
gckCOMMAND_Start(gckCOMMAND Command);

/* Stop the command queue. */
gceSTATUS
gckCOMMAND_Stop(gckCOMMAND Command);

/* Commit command buffers. */
gceSTATUS
gckCOMMAND_Commit(gckCOMMAND Command,
                  gcsHAL_SUBCOMMIT *SubCommit,
                  gctUINT32 ProcessId,
                  gctBOOL Shared,
                  gctUINT64_PTR CommitStamp,
                  gctBOOL *contextSwitched);

/* Reserve space in the command buffer. */
gceSTATUS
gckCOMMAND_Reserve(gckCOMMAND Command,
                   gctUINT32 RequestedBytes,
                   gctPOINTER *Buffer,
                   gctUINT32 *BufferSize);

/*
 * Execute reserved space in the command buffer.
 * Wait link FE version.
 */
gceSTATUS
gckCOMMAND_Execute(gckCOMMAND Command, gctUINT32 RequstedBytes);

/*
 * Execute reserved space in the command buffer.
 * End command version.
 */
gceSTATUS
gckCOMMAND_ExecuteEnd(gckCOMMAND Command, gctUINT32 RequstedBytes);

/*
 * Execute reserved space in the command buffer.
 * Async FE version.
 */
gceSTATUS
gckCOMMAND_ExecuteAsync(gckCOMMAND Command, gctUINT32 RequestedBytes);

/*
 * Execute reserved space in the command buffer.
 * MC-FE version.
 */
gceSTATUS
gckCOMMAND_ExecuteMultiChannel(gckCOMMAND Command,
                               gctBOOL Priority,
                               gctUINT32 ChannelId,
                               gctUINT32 RequstedBytes);

/* Stall the command queue. */
gceSTATUS
gckCOMMAND_Stall(gckCOMMAND Command, gctBOOL FromPower);

/* Attach user process. */
gceSTATUS
gckCOMMAND_Attach(gckCOMMAND Command,
                  gckCONTEXT *Context,
                  gctSIZE_T *MaxState,
                  gctUINT32 *NumStates,
                  gctUINT32 ProcessID,
                  gctBOOL Shared,
                  gctBOOL SmallBatch);

/* Dump command buffer being executed by GPU. */
gceSTATUS
gckCOMMAND_DumpExecutingBuffer(gckCOMMAND Command);

/* Detach user process. */
gceSTATUS
gckCOMMAND_Detach(gckCOMMAND Command, gckCONTEXT Context);

gceSTATUS
gckCOMMAND_CheckFlushMMU(gckCOMMAND Command, gckHARDWARE Hardware);

/* Switch to security first, then switch to non-security mode. */
gceSTATUS
gckCOMMAND_SwitchSecurityMode(gckCOMMAND Command, gckHARDWARE Hardware);

void
gcsLIST_Init(gcsLISTHEAD_PTR Node);

void
gcsLIST_Add(gcsLISTHEAD_PTR New, gcsLISTHEAD_PTR Head);

void
gcsLIST_AddTail(gcsLISTHEAD_PTR New, gcsLISTHEAD_PTR Head);

void
gcsLIST_Del(gcsLISTHEAD_PTR Node);

gctBOOL
gcsLIST_Empty(gcsLISTHEAD_PTR Head);

#define gcmkLIST_FOR_EACH(pos, head) for (pos = (head)->next; pos != (head); pos = pos->next)

#define gcmkLIST_FOR_EACH_SAFE(pos, n, head)                                                       \
    for (pos = (head)->next, n = pos->next; pos != (head); pos = n, n = pos->next)

gceSTATUS
gckFENCE_Create(gckOS Os, gckKERNEL Kernel, gckFENCE *Fence);

gceSTATUS
gckFENCE_Destory(gckOS Os, gckFENCE Fence);

gceSTATUS
gckFENCE_Signal(gckOS Os, gckFENCE Fence);

gceSTATUS
gckDEVICE_Construct(gckOS Os, gckDEVICE *Device);

gceSTATUS
gckDEVICE_DestroyCores(gckDEVICE Device);

gceSTATUS
gckDEVICE_AddCore(gckDEVICE Device,
                  gceCORE Core,
                  gctUINT chipID,
                  gctPOINTER Context,
                  gckKERNEL *Kernel);

gceSTATUS
gckDEVICE_Destroy(gckOS Os, gckDEVICE Device);

gceSTATUS
gckDEVICE_Dispatch(gckDEVICE Device, gcsHAL_INTERFACE_PTR Interface);

gceSTATUS
gckDEVICE_ChipInfo(gckDEVICE Device,
                   gcsHAL_INTERFACE_PTR Interface,
                   gctUINT32 *CoreCount);

#if PHYTIUM_PROFILER
gceSTATUS
gckDEVICE_Profiler_Dispatch(gckDEVICE Device,
                            gcsHAL_PROFILER_INTERFACE_PTR Interface);
#endif

gceSTATUS
gckDEVICE_GetMMU(gckDEVICE Device,
                 gceHARDWARE_TYPE Type,
                 gckMMU *Mmu);

gceSTATUS
gckDEVICE_SetMMU(gckDEVICE Device,
                 gceHARDWARE_TYPE Type,
                 gckMMU Mmu);

gceSTATUS
gckDEVICE_GetCommandQueue(gckDEVICE Device,
                          gceHARDWARE_TYPE Type,
                          gckCOMMAND *Command);

gceSTATUS
gckDEVICE_SetCommandQueue(gckDEVICE Device,
                          gceHARDWARE_TYPE Type,
                          gckCOMMAND Command);

gceSTATUS
gckDEVICE_Version(gckDEVICE Device,
                  gcsHAL_INTERFACE_PTR Interface);

#if gcdENABLE_TRUST_APPLICATION
gceSTATUS
gckKERNEL_MapInTrustApplicaiton(gckKERNEL Kernel,
                                gctPOINTER Logical,
                                gctPHYS_ADDR Physical,
                                gctADDRESS GPUAddress,
                                gctSIZE_T PageCount);
#endif

#if gcdSECURITY || gcdENABLE_TRUST_APPLICATION
gceSTATUS
gckOS_OpenSecurityChannel(gckOS Os, gceCORE Core, gctUINT32 *Channel);

gceSTATUS
gckOS_CloseSecurityChannel(gctUINT32 Channel);

gceSTATUS
gckOS_CallSecurityService(gctUINT32 Channel, gcsTA_INTERFACE *Interface);

gceSTATUS
gckOS_InitSecurityChannel(gctUINT32 Channel);

gceSTATUS
gckOS_AllocatePageArray(gckOS Os,
                        gctPHYS_ADDR Physical,
                        gctSIZE_T PageCount,
                        gctPOINTER *PageArrayLogical,
                        gctPHYS_ADDR *PageArrayPhysical);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_h_ */
