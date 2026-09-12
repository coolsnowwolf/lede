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


#ifndef __gc_hal_kernel_preemption_h_
#define __gc_hal_kernel_preemption_h_

#if gcdENABLE_SW_PREEMPTION
# define gcdMAX_PRIORITY_QUEUE_NUM 3
# define gcdMAX_EVENT_QUEUE_NUM    29

/*
 * Fully preemptible mode:
 * Once high priority render thread commits, the low priority thread will be blocked,
 * until the high priority render thread finish and exit, the low priority can continue to commit to hardware.
 *
 * Non-fully preemptible mode:
 * The low priority render thread can commit, as long as there is no pending commit in any higher priority queue.
 *
 */
typedef enum _gcePREEMPTION_MODE {
    gcvFULLY_PREEMPTIBLE_MODE,
    gcvNON_FULLY_PREEMPTIBLE_MODE,
} gcePREEMPTION_MODE;

typedef struct _gcsPATCH_ARRAY {
    gctUINT64 kArray[32];

    struct _gcsPATCH_ARRAY *next;
} gcsPATCH_ARRAY;

/* Preempt commit. */
typedef struct _gcsPREEMPT_COMMIT *gckPREEMPT_COMMIT;
typedef struct _gcsPREEMPT_COMMIT {
    /* Priority ID. */
    gctUINT32                 priorityID;

    /* Engine type. */
    gceENGINE                 engine;

    /* Process ID. */
    gctUINT32                 pid;

    /* If it is multi-core and shared commit. */
    gctBOOL                   shared;

    /* Command location from user commit. */
    gcsHAL_COMMAND_LOCATION  *cmdLoc;

    /* Static data to store the state delta. */
    gcsSTATE_DELTA            sDelta;

    /* State delta from user commit. */
    gcsSTATE_DELTA_PTR        delta;

    /* Record array. */
    gcsSTATE_DELTA_RECORD_PTR recordArray;

    /* Event queue. */
    gcsQUEUE_PTR              eventQueue;

    /* If it is an event only commit. */
    gctBOOL                   eventOnly;

    /* Context object. */
    gckCONTEXT                context;

    /* Record array; holds all modified states in gcsSTATE_DELTA_RECORD. */
    gctUINT32                 dirtyRecordArraySize;

    /*
     * Map entry ID is used for map entry validation. If map entry ID does not
     * match the main state delta ID, the entry and the corresponding state are
     * considered not in use.
     */
    gctUINT32                *mapEntryID;

    /*
     * If the map entry ID matches the main state delta ID, index points to
     * the state record in the record array.
     */
    gctUINT32                *mapEntryIndex;

    /* If Commit done. */
    gctBOOL                   isEnd;

    /* If it is a NOP commit. */
    gctBOOL                   isNop;

    gckPREEMPT_COMMIT         next;
} gcsPREEMPT_COMMIT;

/* Priority queue. */
typedef struct _gcsPRIORITY_QUEUE *gcsPRIORITY_QUEUE_PTR;
typedef struct _gcsPRIORITY_QUEUE {
    /* Queue id. */
    gctUINT32             id;

    /* Priority queue head. */
    gckPREEMPT_COMMIT     head;

    /* Priority queue tail. */
    gckPREEMPT_COMMIT     tail;

    gcsPRIORITY_QUEUE_PTR next;
} gcsPRIORITY_QUEUE;

/* Construct the preempt commit.*/
gceSTATUS
gckKERNEL_ConstructPreemptCommit(gckKERNEL Kernel,
                                 gcsHAL_SUBCOMMIT_PTR SubCommit,
                                 gceENGINE Engine, gctUINT32 ProcessID,
                                 gctBOOL Shared, gckPREEMPT_COMMIT *PreemptCommit);

/* Prepare the preempt event.*/
gceSTATUS
gckKERNEL_PreparePreemptEvent(gckKERNEL Kernel, gcsQUEUE_PTR Queue,
                              gctUINT32 PriorityID, gctUINT32 ProcessID,
                              gckPREEMPT_COMMIT *PreemptCommit);

/* Destroy the priority queue. */
gceSTATUS
gckKERNEL_PriorityQueueDestroy(gckKERNEL Kernel, gcsPRIORITY_QUEUE_PTR Queue);

/* Append preemptCommit to priority queue. */
gceSTATUS
gckKERNEL_PriorityQueueAppend(gckKERNEL Kernel, gctUINT PriorityID,
                              gckPREEMPT_COMMIT PreemptCommit);

/* Preemption thread. */
gceSTATUS
gckKERNEL_PreemptionThread(gckKERNEL Kernel);

/* Commit command with preemption. */
gceSTATUS
gckKERNEL_CommandCommitPreemption(gckKERNEL Kernel, gceENGINE Engine,
                                  gctUINT32 ProcessId, gckCOMMAND Command,
                                  gckEVENT EventObj, gcsHAL_SUBCOMMIT *SubCommit,
                                  gcsHAL_COMMIT *Commit);

/* Commit event with preemption. */
gceSTATUS
gckKERNEL_EventCommitPreemption(gckKERNEL Kernel, gceENGINE Engine,
                                gctUINT32 ProcessID, gcsQUEUE_PTR Queue,
                                gctUINT32 PriorityID, gctBOOL TopPriority, gctBOOL Shared);

/* Commit command in preemption mode. */
gceSTATUS
gckCOMMAND_PreemptCommit(gckCOMMAND Command, gckPREEMPT_COMMIT PreemptCommit);

/* Update delta in kernel driver. */
gceSTATUS
gckCONTEXT_UpdateDelta(gckCONTEXT Context, gcsSTATE_DELTA_PTR Delta);

/* Destroy previous context switch delta. */
gceSTATUS
gckCONTEXT_DestroyPrevDelta(gckCONTEXT Context);

/* Construct and store previous context switch delta. */
gceSTATUS
gckCONTEXT_ConstructPrevDelta(gckCONTEXT Context, gctUINT32 ProcessID, gcsSTATE_DELTA_PTR StateDelta);

/* Context update in preemption mode.*/
gceSTATUS
gckCONTEXT_PreemptUpdate(gckCONTEXT Context,
                         gckPREEMPT_COMMIT PreemptCommit);

/* Event commit in preemption mode. */
gceSTATUS
gckEVENT_PreemptCommit(gckEVENT Event,
                       gckPREEMPT_COMMIT PreemptCommit,
                       gctBOOL Forced);

/* Construct a NOP preemptCommit which means it is the end commit. */
gceSTATUS
gckKERNEL_PreemptCommitDone(gckKERNEL Kernel, gctUINT32 PriorityID,
                            gckPREEMPT_COMMIT *PreemptCommit);
#endif

#endif /* __gc_hal_kernel_preemption_h_ */
