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

#ifdef __QNXNTO__
#include "gc_hal_kernel_qnx.h"
#endif

#define _GC_OBJ_ZONE gcvZONE_EVENT

#define gcdEVENT_ALLOCATION_COUNT (4096 / gcmSIZEOF(gcsHAL_INTERFACE))
#define gcdEVENT_MIN_THRESHOLD    4

/******************************************************************************
 ********************************* Support Code *******************************
 ******************************************************************************/

static gcmINLINE gceSTATUS
gckEVENT_AllocateQueue(gckEVENT Event, gcsEVENT_QUEUE_PTR *Queue)
{
    gceSTATUS status;

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Queue != gcvNULL);

    /* Do we have free queues? */
    if (Event->freeList == gcvNULL)
        gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);

    /* Move one free queue from the free list. */
    *Queue = Event->freeList;
    Event->freeList = Event->freeList->next;

    /* Success. */
    gcmkFOOTER_ARG("*Queue=0x%x", gcmOPT_POINTER(Queue));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
gckEVENT_FreeQueue(gckEVENT Event, gcsEVENT_QUEUE_PTR Queue)
{
    gceSTATUS status = gcvSTATUS_OK;

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Queue != gcvNULL);

    /* Move one free queue from the free list. */
    Queue->next = Event->freeList;
    Event->freeList = Queue;

    /* Success. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
gckEVENT_FreeRecord(gckEVENT Event, gcsEVENT_PTR Record)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Event=0x%x Record=0x%x", Event, Record);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Record != gcvNULL);

    /* Acquire the mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->freeEventMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Push the record on the free list. */
    Record->next = Event->freeEventList;
    Event->freeEventList = Record;
    Event->freeEventCount += 1;

    /* Release the mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->freeEventMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->freeEventMutex));

    /* Return the status. */
    gcmkFOOTER();
    return gcvSTATUS_OK;
}

static gceSTATUS
gckEVENT_IsEmpty(gckEVENT Event, gctBOOL_PTR IsEmpty)
{
    gceSTATUS status;
    gctINT i;

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(IsEmpty != gcvNULL);

    /* Assume the event queue is empty. */
    *IsEmpty = gcvTRUE;

    /* Walk the event queue. */
    for (i = 0; i < Event->totalQueueCount; ++i) {
        /* Check whether this event is in use. */
        if (Event->queues[i].head != gcvNULL) {
            /* The event is in use, hence the queue is not empty. */
            *IsEmpty = gcvFALSE;
            break;
        }
    }

    /* Try acquiring the mutex. */
    status = gckOS_AcquireMutex(Event->os, Event->eventQueueMutex, 0);
    if (status == gcvSTATUS_TIMEOUT) {
        /* Timeout - queue is no longer empty. */
        *IsEmpty = gcvFALSE;
    } else {
        /* Bail out on error. */
        gcmkONERROR(status);

        /* Release the mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
    }

    /* Success. */
    gcmkFOOTER_ARG("*IsEmpty=%d", gcmOPT_VALUE(IsEmpty));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

static gceSTATUS
_TryToIdleGPU(gckEVENT Event)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctBOOL empty = gcvFALSE;
    gctBOOL idle = gcvFALSE;
    gctBOOL powerLocked = gcvFALSE;
    gckHARDWARE hardware = Event->kernel->hardware;
#if gcdENABLE_PER_DEVICE_PM
    gctBOOL devicePowerLocked = gcvFALSE;
    gckDEVICE device = Event->kernel->device;
#endif

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);

    gcmkVERIFY_OBJECT(hardware, gcvOBJ_HARDWARE);

    if (!hardware->options.powerManagement) {
        gcmkFOOTER();
        return status;
    }

    /* Check whether the event queue is empty. */
    gcmkONERROR(gckEVENT_IsEmpty(Event, &empty));

    if (empty) {
#if gcdENABLE_PER_DEVICE_PM
        if (hardware->type == gcvHARDWARE_3D ||
            hardware->type == gcvHARDWARE_3D2D ||
            hardware->type == gcvHARDWARE_VIP) {
            status = gckOS_AcquireMutex(device->os, device->powerMutex, 0);
            if (status == gcvSTATUS_TIMEOUT) {
                gcmkFOOTER();
                return gcvSTATUS_OK;
            }

            devicePowerLocked = gcvTRUE;

            status = gckOS_AcquireMutex(hardware->os, hardware->powerMutex, 0);
            if (status == gcvSTATUS_TIMEOUT) {
                gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os, device->powerMutex));
                gcmkFOOTER();
                return gcvSTATUS_OK;
            }

            powerLocked = gcvTRUE;

            /* Query whether the hardware is idle. */
            gcmkONERROR(gckHARDWARE_QueryIdle(hardware, &idle));

            gcmkONERROR(gckOS_ReleaseMutex(hardware->os, hardware->powerMutex));

            powerLocked = gcvFALSE;

            if (idle) {
                gctUINT32 broCoreMask;
                gckKERNEL kernel;
                gctUINT i;

                gcmkVERIFY_OK(gckOS_AtomGet(hardware->os, Event->kernel->atomBroCoreMask,
                                            (gctINT32_PTR)&broCoreMask));

                /* I am along. */
                if ((gceCORE)broCoreMask == hardware->core) {
                    /* Inform the system of idle GPU. */
                    gcmkONERROR(gckOS_Broadcast(hardware->os, hardware, gcvBROADCAST_GPU_IDLE));

                    gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os, device->powerMutex));
                    gcmkFOOTER();
                    return gcvSTATUS_OK;
                }

                /* Check all the brother cores. */
                for (i = 0; i < device->coreNum; i++) {
                    kernel = device->coreInfoArray[i].kernel;
                    hardware = kernel->hardware;

                    if (!hardware || ((gceCORE)i == hardware->core))
                        continue;

                    if ((1 << i) & broCoreMask) {
                        status = gckOS_AcquireMutex(hardware->os, hardware->powerMutex, 0);
                        if (status == gcvSTATUS_TIMEOUT) {
                            gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os, device->powerMutex));
                            gcmkFOOTER();
                            return gcvSTATUS_OK;
                        }

                        powerLocked = gcvTRUE;

                        /* Query whether the hardware is idle. */
                        gcmkONERROR(gckHARDWARE_QueryIdle(hardware, &idle));

                        gcmkONERROR(gckOS_ReleaseMutex(hardware->os, hardware->powerMutex));
                        powerLocked = gcvFALSE;

                        if (!idle) {
                            /* A brother is not idle, quit. */
                            gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os, device->powerMutex));
                            gcmkFOOTER();
                            return gcvSTATUS_OK;
                        }
                    }
                }

                /* All the brothers are idle. */
                for (i = 0; i < device->coreNum; i++) {
                    if ((1 << i) & broCoreMask) {
                        kernel = device->coreInfoArray[i].kernel;
                        hardware = kernel->hardware;

                        /* Inform the system of idle GPU. */
                        gcmkONERROR(gckOS_Broadcast(hardware->os, hardware, gcvBROADCAST_GPU_IDLE));
                    }
                }
            }

            gcmkONERROR(gckOS_ReleaseMutex(device->os, device->powerMutex));
        } else {
#endif
            status = gckOS_AcquireMutex(hardware->os, hardware->powerMutex, 0);
            if (status == gcvSTATUS_TIMEOUT) {
                gcmkFOOTER();
                return gcvSTATUS_OK;
            }

            powerLocked = gcvTRUE;

            /* Query whether the hardware is idle. */
            gcmkONERROR(gckHARDWARE_QueryIdle(Event->kernel->hardware, &idle));

            gcmkONERROR(gckOS_ReleaseMutex(hardware->os, hardware->powerMutex));

            powerLocked = gcvFALSE;

            if (idle) {
                /* Inform the system of idle GPU. */
                gcmkONERROR(gckOS_Broadcast(Event->os,
                                            Event->kernel->hardware,
                                            gcvBROADCAST_GPU_IDLE));
            }
#if gcdENABLE_PER_DEVICE_PM
        }
#endif
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
#if gcdENABLE_PER_DEVICE_PM
    if (devicePowerLocked)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(device->os, device->powerMutex));
#endif

    if (powerLocked)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(hardware->os, hardware->powerMutex));

    gcmkFOOTER();
    return status;
}

static gceSTATUS
__RemoveRecordFromProcessDB(gckEVENT Event, gcsEVENT_PTR Record)
{
    gcmkHEADER_ARG("Event=0x%x Record=0x%x", Event, Record);
    gcmkVERIFY_ARGUMENT(Record != gcvNULL);

    switch (Record->info.command) {
    case gcvHAL_UNLOCK_VIDEO_MEMORY:
        gcmkVERIFY_OK(gckKERNEL_RemoveProcessDB(Event->kernel,
                                                Record->processID,
                                                gcvDB_VIDEO_MEMORY_LOCKED,
                                                gcmUINT64_TO_PTR(Record->info.u.UnlockVideoMemory.node)));
        break;

    default:
        break;
    }

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

static gceSTATUS
_ReleaseVideoMemoryHandle(gckKERNEL Kernel, gcsEVENT_PTR Record, gcsHAL_INTERFACE *Interface)
{
    gceSTATUS status;
    gckVIDMEM_NODE nodeObject;
    gctUINT32 handle;

    switch (Interface->command) {
    case gcvHAL_UNLOCK_VIDEO_MEMORY:
        handle = (gctUINT32)Interface->u.UnlockVideoMemory.node;

        gcmkONERROR(gckVIDMEM_HANDLE_Lookup(Kernel, Record->processID, handle, &nodeObject));

        Record->info.u.UnlockVideoMemory.node = gcmPTR_TO_UINT64(nodeObject);

        gcmkONERROR(gckVIDMEM_HANDLE_Dereference(Kernel, Record->processID, handle));
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
 **  _QueryFlush
 **
 **  Check the type of surfaces which will be released by current event and
 **  determine the cache needed to flush.
 **
 */
static gceSTATUS
_QueryFlush(gckEVENT Event, gcsEVENT_PTR Record, gceKERNEL_FLUSH *Flush)
{
    gceKERNEL_FLUSH flush = 0;
    gckVIDMEM_NODE nodeObject;

    gcmkHEADER_ARG("Event=0x%x Record=0x%x", Event, Record);
    gcmkVERIFY_ARGUMENT(Record != gcvNULL);

    while (Record != gcvNULL) {
        switch (Record->info.command) {
        case gcvHAL_UNLOCK_VIDEO_MEMORY:
            nodeObject = gcmUINT64_TO_PTR(Record->info.u.UnlockVideoMemory.node);

            switch (nodeObject->type) {
            case gcvVIDMEM_TYPE_TILE_STATUS:
                flush |= gcvFLUSH_TILE_STATUS;
                break;
            case gcvVIDMEM_TYPE_COLOR_BUFFER:
                flush |= gcvFLUSH_COLOR;
                break;
            case gcvVIDMEM_TYPE_DEPTH_BUFFER:
                flush |= gcvFLUSH_DEPTH;
                break;
            case gcvVIDMEM_TYPE_TEXTURE:
                flush |= gcvFLUSH_TEXTURE;
                break;
            case gcvVIDMEM_TYPE_ICACHE:
                flush |= gcvFLUSH_ICACHE;
                break;
            case gcvVIDMEM_TYPE_TXDESC:
                flush |= gcvFLUSH_TXDESC;
                break;
            case gcvVIDMEM_TYPE_FENCE:
                flush |= gcvFLUSH_FENCE;
                break;
            case gcvVIDMEM_TYPE_VERTEX_BUFFER:
                flush |= gcvFLUSH_VERTEX;
                break;
            case gcvVIDMEM_TYPE_TFBHEADER:
                flush |= gcvFLUSH_TFBHEADER;
                break;
            case gcvVIDMEM_TYPE_GENERIC:
                flush = gcvFLUSH_ALL;
                goto Out;
            default:
                break;
            }
            break;
        default:
            break;
        }

        Record = Record->next;
    }

Out:
    *Flush = flush;

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

static void
_SubmitTimerFunction(gctPOINTER Data)
{
    gckEVENT event = (gckEVENT)Data;
    gcsEVENT_ATTR eventAttr;

    eventAttr.wait = gcvTRUE;
    eventAttr.shared = gcvFALSE;
    eventAttr.fromPower = gcvFALSE;
    eventAttr.broadcast = gcvTRUE;

    gckOS_AcquireMutex(event->os, event->kernel->device->commitMutex, gcvINFINITE);

    gcmkVERIFY_OK(gckEVENT_Submit(event, &eventAttr));

    gcmkVERIFY_OK(gckOS_ReleaseMutex(event->os, event->kernel->device->commitMutex));
}

/******************************************************************************
 ******************************* gckEVENT API Code ****************************
 ******************************************************************************/

/*******************************************************************************
 **
 **  gckEVENT_Construct
 **
 **  Construct a new gckEVENT object.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to an gckKERNEL object.
 **
 **  OUTPUT:
 **
 **      gckEVENT * Event
 **          Pointer to a variable that receives the gckEVENT object pointer.
 */
gceSTATUS
gckEVENT_Construct(gckKERNEL Kernel, gckCOMMAND Command, gckEVENT *Event)
{
    gckOS os;
    gceSTATUS status;
    gckEVENT eventObj = gcvNULL;
    int i;
    gcsEVENT_PTR record;
    gctPOINTER pointer = gcvNULL;

    gcmkHEADER_ARG("Kernel=0x%x", Kernel);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Kernel, gcvOBJ_KERNEL);
    gcmkVERIFY_ARGUMENT(Event != gcvNULL);

    /* Extract the pointer to the gckOS object. */
    os = Kernel->os;
    gcmkVERIFY_OBJECT(os, gcvOBJ_OS);

    /* Allocate the gckEVENT object. */
    gcmkONERROR(gckOS_Allocate(os, gcmSIZEOF(struct _gckEVENT), &pointer));

    eventObj = pointer;

    /* Reset the object. */
    gcmkVERIFY_OK(gckOS_ZeroMemory(eventObj, gcmSIZEOF(struct _gckEVENT)));

    /* Initialize the gckEVENT object. */
    eventObj->object.type = gcvOBJ_EVENT;
    eventObj->kernel = Kernel;
    eventObj->os = os;
    eventObj->command = Command;

    /* Create the mutexes. */
    gcmkONERROR(gckOS_CreateMutex(os, &eventObj->eventQueueMutex));
    gcmkONERROR(gckOS_CreateMutex(os, &eventObj->freeEventMutex));
    gcmkONERROR(gckOS_CreateMutex(os, &eventObj->eventListMutex));

    /* Create a bunch of event reccords. */
    for (i = 0; i < gcdEVENT_ALLOCATION_COUNT; i += 1) {
        /* Allocate an event record. */
        gcmkONERROR(gckOS_Allocate(os, gcmSIZEOF(gcsEVENT), &pointer));

        record = pointer;

        /* Push it on the free list. */
        record->next = eventObj->freeEventList;
        eventObj->freeEventList = record;
        eventObj->freeEventCount += 1;
    }

    /* Initialize the free list of event queues. */
    for (i = 0; i < gcdREPO_LIST_COUNT; i += 1) {
        eventObj->repoList[i].next = eventObj->freeList;
        eventObj->freeList = &eventObj->repoList[i];
    }

    eventObj->totalQueueCount = (Command->feType == gcvHW_FE_END) ?
                                gcdEVENT_QUEUE_COUNT - 1 :
                                gcdEVENT_QUEUE_COUNT;

    eventObj->freeQueueCount = eventObj->totalQueueCount;

    gcmkONERROR(gckOS_AtomConstruct(os, &eventObj->pending));

    gcmkVERIFY_OK(gckOS_CreateTimer(os, _SubmitTimerFunction,
                                    (gctPOINTER)eventObj,
                                    &eventObj->submitTimer));

#if gcdINTERRUPT_STATISTIC
    gcmkONERROR(gckOS_AtomConstruct(os, &eventObj->interruptCount));
    gcmkONERROR(gckOS_AtomSet(os, eventObj->interruptCount, 0));
#endif

    eventObj->notifyState = -1;

    /* Return pointer to the gckEVENT object. */
    *Event = eventObj;

    /* Success. */
    gcmkFOOTER_ARG("*Event=0x%x", *Event);
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (eventObj != gcvNULL) {
        if (eventObj->eventQueueMutex != gcvNULL)
            gcmkVERIFY_OK(gckOS_DeleteMutex(os, eventObj->eventQueueMutex));

        if (eventObj->freeEventMutex != gcvNULL)
            gcmkVERIFY_OK(gckOS_DeleteMutex(os, eventObj->freeEventMutex));

        if (eventObj->eventListMutex != gcvNULL)
            gcmkVERIFY_OK(gckOS_DeleteMutex(os, eventObj->eventListMutex));

        while (eventObj->freeEventList != gcvNULL) {
            record = eventObj->freeEventList;
            eventObj->freeEventList = record->next;

            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(os, record));
        }

        if (eventObj->pending != gcvNULL)
            gcmkVERIFY_OK(gckOS_AtomDestroy(os, eventObj->pending));

#if gcdINTERRUPT_STATISTIC
        if (eventObj->interruptCount)
            gcmkVERIFY_OK(gckOS_AtomDestroy(os, eventObj->interruptCount));
#endif
        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(os, eventObj));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckEVENT_Destroy
 **
 **  Destroy an gckEVENT object.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Destroy(gckEVENT Event)
{
    gcsEVENT_PTR record;
    gcsEVENT_QUEUE_PTR queue;

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);

    if (Event->submitTimer != gcvNULL) {
        gcmkVERIFY_OK(gckOS_StopTimer(Event->os, Event->submitTimer));
        gcmkVERIFY_OK(gckOS_DestroyTimer(Event->os, Event->submitTimer));
        Event->submitTimer = gcvNULL;
    }

    /* Delete the queue mutex. */
    if (Event->eventQueueMutex) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Event->os, Event->eventQueueMutex));
        Event->eventQueueMutex = gcvNULL;
    }

    /* Free all free events. */
    while (Event->freeEventList != gcvNULL) {
        record = Event->freeEventList;
        Event->freeEventList = record->next;

        gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Event->os, record));
    }

    /* Delete the free mutex. */
    if (Event->freeEventMutex) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Event->os, Event->freeEventMutex));
        Event->freeEventMutex = gcvNULL;
    }

    /* Free all pending queues. */
    while (Event->queueHead != gcvNULL) {
        /* Get the current queue. */
        queue = Event->queueHead;

        /* Free all pending events. */
        while (queue->head != gcvNULL) {
            record = queue->head;
            queue->head = record->next;

            gcmkTRACE_ZONE_N(gcvLEVEL_WARNING, gcvZONE_EVENT,
                             gcmSIZEOF(record) + gcmSIZEOF(queue->source),
                             "Event record 0x%x is still pending for %d.",
                             record, queue->source);

            gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Event->os, record));
        }

        /* Remove the top queue from the list. */
        if (Event->queueHead == Event->queueTail) {
            Event->queueHead = gcvNULL;
            Event->queueTail = gcvNULL;
        } else {
            Event->queueHead = Event->queueHead->next;
        }

        /* Free the queue. */
        gcmkVERIFY_OK(gckEVENT_FreeQueue(Event, queue));
    }

    /* Delete the list mutex. */
    if (Event->eventListMutex) {
        gcmkVERIFY_OK(gckOS_DeleteMutex(Event->os, Event->eventListMutex));
        Event->eventListMutex = gcvNULL;
    }

    gcmkVERIFY_OK(gckOS_AtomDestroy(Event->os, Event->pending));
    Event->pending = gcvNULL;

#if gcdINTERRUPT_STATISTIC
    gcmkVERIFY_OK(gckOS_AtomDestroy(Event->os, Event->interruptCount));
    Event->interruptCount = gcvNULL;
#endif

    /* Mark the gckEVENT object as unknown. */
    Event->object.type = gcvOBJ_UNKNOWN;

    /* Free the gckEVENT object. */
    gcmkVERIFY_OK(gcmkOS_SAFE_FREE(Event->os, Event));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckEVENT_GetEvent
 **
 **  Reserve the next available hardware event.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gctBOOL Wait
 **          Set to gcvTRUE to force the function to wait if no events are
 **          immediately available.
 **
 **      gceKERNEL_WHERE Source
 **          Source of the event.
 **
 **  OUTPUT:
 **
 **      gctUINT8 *EventID
 **          Reserved event ID.
 */
#define gcdINVALID_EVENT_PTR ((gcsEVENT_PTR)gcvMAXUINTPTR_T)

gceSTATUS
gckEVENT_GetEvent(gckEVENT Event, gctBOOL Wait,
                  gctUINT8 *EventID, gceKERNEL_WHERE Source)
{
    gctINT i, id;
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;

    gcmkHEADER_ARG("Event=0x%x Source=%d", Event, Source);

    while (gcvTRUE) {
        /* Grab the queue mutex. */
        gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->eventQueueMutex, gcvINFINITE));
        acquired = gcvTRUE;

        /* Walk through all events. */
        id = Event->lastID;
        for (i = 0; i < Event->totalQueueCount; ++i) {
            gctINT nextID = id + 1;

            if (nextID == Event->totalQueueCount)
                nextID = 0;

            if (Event->queues[id].head == gcvNULL) {
                *EventID = (gctUINT8)id;

                Event->lastID = (gctUINT8)nextID;

                /* Save time stamp of event. */
                Event->queues[id].head   = gcdINVALID_EVENT_PTR;
                Event->queues[id].stamp  = ++(Event->stamp);
                Event->queues[id].source = Source;

                /* Decrease the number of free events. */
                --Event->freeQueueCount;

#if gcdDYNAMIC_SPEED
                if (Event->freeQueueCount <= gcdDYNAMIC_EVENT_THRESHOLD) {
                    gcmkONERROR(gckOS_BroadcastHurry(Event->os,
                                                     Event->kernel->hardware,
                                                     gcdDYNAMIC_EVENT_THRESHOLD - Event->freeQueueCount));
                }
#endif

                /* Release the queue mutex. */
                gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));

                /* Success. */
                gcmkTRACE_ZONE_N(gcvLEVEL_INFO, gcvZONE_EVENT, gcmSIZEOF(id),
                                 "Using id=%d", id);

                gcmkFOOTER_ARG("*EventID=%u", *EventID);
                return gcvSTATUS_OK;
            }

            id = nextID;
        }

#if gcdDYNAMIC_SPEED
        /* No free events, speed up the GPU right now! */
        gcmkONERROR(gckOS_BroadcastHurry(Event->os, Event->kernel->hardware,
                                         gcdDYNAMIC_EVENT_THRESHOLD));
#endif

        /* Release the queue mutex. */
        gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
        acquired = gcvFALSE;

        /* Fail if wait is not requested. */
        if (!Wait) {
            /* Out of resources. */
            gcmkONERROR(gcvSTATUS_OUT_OF_RESOURCES);
        }

        /* Delay a while. */
        gcmkONERROR(gckOS_Delay(Event->os, 1));
    }

OnError:
    if (acquired) {
        /* Release the queue mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckEVENT_AllocateRecord
 **
 **  Allocate a record for the new event.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gctBOOL AllocateAllowed
 **          State for allocation if out of free events.
 **
 **  OUTPUT:
 **
 **      gcsEVENT_PTR *Record
 **          Allocated event record.
 */
static gcmINLINE gceSTATUS
gckEVENT_AllocateRecord(gckEVENT Event, gctBOOL AllocateAllowed, gcsEVENT_PTR *Record)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gctINT i;
    gcsEVENT_PTR record;
    gctPOINTER pointer = gcvNULL;

    gcmkHEADER_ARG("Event=0x%x AllocateAllowed=%d", Event, AllocateAllowed);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Record != gcvNULL);

    /* Acquire the mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->freeEventMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Test if we are below the allocation threshold. */
    if ((AllocateAllowed && Event->freeEventCount < gcdEVENT_MIN_THRESHOLD) ||
        Event->freeEventCount == 0) {
        /* Allocate a bunch of records. */
        for (i = 0; i < gcdEVENT_ALLOCATION_COUNT; i += 1) {
            /* Allocate an event record. */
            gcmkONERROR(gckOS_Allocate(Event->os, gcmSIZEOF(gcsEVENT), &pointer));

            record = pointer;

            /* Push it on the free list. */
            record->next = Event->freeEventList;
            Event->freeEventList = record;
            Event->freeEventCount += 1;
        }
    }

    *Record = Event->freeEventList;
    Event->freeEventList = Event->freeEventList->next;
    Event->freeEventCount -= 1;

    /* Release the mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->freeEventMutex));

    /* Success. */
    gcmkFOOTER_ARG("*Record=0x%x", gcmOPT_POINTER(Record));
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->freeEventMutex));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckEVENT_AddList
 **
 **  Add a new event to the list of events.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gcsHAL_INTERFACE_PTR Interface
 **          Pointer to the interface for the event to be added.
 **
 **      gceKERNEL_WHERE FromWhere
 **          Place in the pipe where the event needs to be generated.
 **
 **      gctBOOL AllocateAllowed
 **          State for allocation if out of free events.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_AddListEx(gckEVENT Event, gcsHAL_INTERFACE_PTR Interface,
                   gceKERNEL_WHERE FromWhere, gctBOOL AllocateAllowed,
                   gctBOOL FromKernel, gctUINT32 ProcessID)
{
    gceSTATUS status;
    gctBOOL acquired = gcvFALSE;
    gcsEVENT_PTR record = gcvNULL;
    gcsEVENT_QUEUE_PTR queue;

    gcmkHEADER_ARG("Event=0x%x Interface=0x%x", Event, Interface);

    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, _GC_OBJ_ZONE,
                   "FromWhere=%d AllocateAllowed=%d",
                   FromWhere, AllocateAllowed);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Interface != gcvNULL);

    /* Verify the event command. */
    gcmkASSERT((Interface->command == gcvHAL_WRITE_DATA) ||
               (Interface->command == gcvHAL_UNLOCK_VIDEO_MEMORY) ||
               (Interface->command == gcvHAL_SIGNAL) ||
               (Interface->command == gcvHAL_TIMESTAMP) ||
               (Interface->command == gcvHAL_COMMIT_DONE) ||
               (Interface->command == gcvHAL_DESTROY_MMU));

    /* Validate the source. */
    if (FromWhere != gcvKERNEL_COMMAND &&
        FromWhere != gcvKERNEL_PIXEL) {
        /* Invalid argument. */
        gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
    }

    /* Allocate a free record. */
    gcmkONERROR(gckEVENT_AllocateRecord(Event, AllocateAllowed, &record));

    /* Termninate the record. */
    record->next = gcvNULL;

    /* Record the committer. */
    record->fromKernel = FromKernel;

    /* Copy the event interface into the record. */
    gckOS_MemCopy(&record->info, Interface, gcmSIZEOF(record->info));

    /* Get process ID. */
    if (ProcessID)
        record->processID = ProcessID;
    else
        gcmkONERROR(gckOS_GetProcessID(&record->processID));

    if (Interface->command == gcvHAL_UNLOCK_VIDEO_MEMORY && Interface->u.UnlockVideoMemory.mmu)
        record->mmu = (gckMMU)gcmUINT64_TO_PTR(Interface->u.UnlockVideoMemory.mmu);
    else
        gcmkONERROR(gckKERNEL_GetCurrentMMU(Event->kernel, !FromKernel, record->processID, &record->mmu));

    if (FromKernel == gcvFALSE) {
        gcmkONERROR(__RemoveRecordFromProcessDB(Event, record));

        /* Handle is belonged to current process, it must be released now. */
        status = _ReleaseVideoMemoryHandle(Event->kernel, record, Interface);

        if (gcmIS_ERROR(status)) {
            /* Ignore error because there are other events in the queue. */
            status = gcvSTATUS_OK;
            goto OnError;
        }
    }

#ifdef __QNXNTO__
    record->kernel = Event->kernel;
#endif

    /* Acquire the mutex. */
    gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->eventListMutex, gcvINFINITE));
    acquired = gcvTRUE;

    /* Do we need to allocate a new queue? */
    if (Event->queueTail == gcvNULL ||
        Event->queueTail->source < FromWhere) {
        /* Allocate a new queue. */
        gcmkONERROR(gckEVENT_AllocateQueue(Event, &queue));

        /* Initialize the queue. */
        queue->source = FromWhere;
        queue->head = gcvNULL;
        queue->next = gcvNULL;

        /* Attach it to the list of allocated queues. */
        if (Event->queueTail == gcvNULL) {
            Event->queueHead = queue;
            Event->queueTail = queue;
        } else {
            Event->queueTail->next = queue;
            Event->queueTail = queue;
        }
    } else {
        queue = Event->queueTail;
    }

    /* Attach the record to the queue. */
    if (queue->head == gcvNULL) {
        queue->head = record;
        queue->tail = record;
    } else {
        queue->tail->next = record;
        queue->tail = record;
    }

    /* Release the mutex. */
    gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Roll back. */
    if (acquired)
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));

    if (record != gcvNULL)
        gcmkVERIFY_OK(gckEVENT_FreeRecord(Event, record));

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckEVENT_AddList(gckEVENT Event, gcsHAL_INTERFACE_PTR Interface,
                 gceKERNEL_WHERE FromWhere, gctBOOL AllocateAllowed, gctBOOL FromKernel)
{
    return gckEVENT_AddListEx(Event, Interface, FromWhere, AllocateAllowed, FromKernel, 0);
}

/*******************************************************************************
 **
 **  gckEVENT_Unlock
 **
 **  Schedule an event to unlock virtual memory.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gceKERNEL_WHERE FromWhere
 **          Place in the pipe where the event needs to be generated.
 **
 **      gcuVIDMEM_NODE_PTR Node
 **          Pointer to a gcuVIDMEM_NODE union that specifies the virtual memory
 **          to unlock.
 **
 **      gceVIDMEM_TYPE Type
 **          Video memory allocation type to unlock.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Unlock(gckEVENT Event, gceKERNEL_WHERE FromWhere,
                gckMMU Mmu, gctPOINTER Node)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface = {0};

    gcmkHEADER_ARG("Event=0x%x FromWhere=%d Node=0x%x",
                   Event, FromWhere, Node);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Node != gcvNULL);

    /* Mark the event as an unlock. */
    iface.command = gcvHAL_UNLOCK_VIDEO_MEMORY;
    iface.u.UnlockVideoMemory.node = gcmPTR_TO_UINT64(Node);
    iface.u.UnlockVideoMemory.asynchroneous = 0;
    iface.u.UnlockVideoMemory.mmu = gcmPTR_TO_UINT64(Mmu);

    /* Append it to the queue. */
    gcmkONERROR(gckEVENT_AddList(Event, &iface, FromWhere, gcvFALSE, gcvTRUE));

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
 **  gckEVENT_Signal
 **
 **  Schedule an event to trigger a signal.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gctSIGNAL Signal
 **          Pointer to the signal to trigger.
 **
 **      gceKERNEL_WHERE FromWhere
 **          Place in the pipe where the event needs to be generated.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Signal(gckEVENT Event, gctSIGNAL Signal, gceKERNEL_WHERE FromWhere)
{
    gceSTATUS status;
    gcsHAL_INTERFACE iface;

    gcmkHEADER_ARG("Event=0x%x Signal=0x%x FromWhere=%d",
                   Event, Signal, FromWhere);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(Signal != gcvNULL);

    /* Mark the event as a signal. */
    iface.command = gcvHAL_SIGNAL;
    iface.u.Signal.signal = gcmPTR_TO_UINT64(Signal);
    iface.u.Signal.auxSignal = 0;
    iface.u.Signal.process = 0;

#ifdef __QNXNTO__
    iface.u.Signal.rcvid = 0;
    SIGEV_NONE_INIT(&iface.u.Signal.event);

    gcmkONERROR(gckOS_SignalPending(Event->os, Signal));
#endif

    /* Append it to the queue. */
    gcmkONERROR(gckEVENT_AddList(Event, &iface, FromWhere, gcvFALSE, gcvTRUE));

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
 **  gckEVENT_Submit
 **
 **  Submit the current event queue to the GPU.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to a gckEVENT object.
 **
 **      gckEVENT_ATTR EventAttr
 **          Pointer to a gckEVENT_ATTR object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Submit(gckEVENT Event, gckEVENT_ATTR EventAttr)
{
    gceSTATUS status;
    gctUINT8 id = 0xFF;
    gcsEVENT_QUEUE_PTR queue;
    gctBOOL acquired = gcvFALSE;
    gckCOMMAND command = gcvNULL;
    gctBOOL commitEntered = gcvFALSE;
    gctUINT32 bytes;
    gctPOINTER buffer;
    gctUINT32 executeBytes;
    gctUINT32 flushBytes;
    gckHARDWARE hardware = gcvNULL;
    gceKERNEL_FLUSH flush = gcvFALSE;
    gctBOOL wait, fromPower, broadcast;
    gctUINT64 commitStamp;
#if gcdSHARED_COMMAND_BUFFER
    gctSIZE_T chipEnableBytes = 8;
#endif
#if gcdINTERRUPT_STATISTIC
    gctINT32 oldValue;
#endif
#if gcdSECURITY
    gctPOINTER reservedBuffer;
#endif

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Get gckCOMMAND object. */
    command = Event->command;
#if gcdSHARED_COMMAND_BUFFER
    if (EventAttr->shared)
        command = Event->kernel->sharedCommand;
#endif

    hardware = Event->kernel->hardware;

    gcmkVERIFY_OBJECT(hardware, gcvOBJ_HARDWARE);

    wait = EventAttr->wait;
    fromPower = EventAttr->fromPower;
    broadcast = EventAttr->broadcast;

    gckOS_GetTicks(&Event->lastCommitStamp);

    gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->eventListMutex, gcvINFINITE));
    queue = Event->queueHead;
    gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));

    /* Are there event queues? */
    if (queue) {
        if (broadcast) {
            /* Acquire the command queue. */
            gcmkONERROR(gckCOMMAND_EnterCommit(command, fromPower));
            commitEntered = gcvTRUE;
        }

        /* Get current commit stamp. */
        commitStamp = command->commitStamp;

        if (commitStamp)
            commitStamp -= 1;

        /* Process all queues. */
        while (gcvTRUE) {
            /* Acquire the list mutex. */
            gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->eventListMutex, gcvINFINITE));
            acquired = gcvTRUE;

            if (!Event->queueHead) {
                gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));
                acquired = gcvFALSE;
                break;
            }
            /* Get the current queue. */
            queue = Event->queueHead;

            /* Allocate an event ID. */
            gcmkONERROR(gckEVENT_GetEvent(Event, wait, &id, queue->source));

            /* Copy event list to event ID queue. */
            Event->queues[id].head = queue->head;

            /* Update current commit stamp. */
            Event->queues[id].commitStamp = commitStamp;

            /* Remove the top queue from the list. */
            if (Event->queueHead == Event->queueTail) {
                Event->queueHead = gcvNULL;
                Event->queueTail = gcvNULL;
            } else {
                Event->queueHead = Event->queueHead->next;
            }

            /* Free the queue. */
            gcmkONERROR(gckEVENT_FreeQueue(Event, queue));

            /* Release the list mutex. */
            gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));
            acquired = gcvFALSE;

            if (command->feType == gcvHW_FE_WAIT_LINK || command->feType == gcvHW_FE_END) {
                /* Determine cache needed to flush. */
                gcmkVERIFY_OK(_QueryFlush(Event, Event->queues[id].head, &flush));

                /* Get the size of the hardware event. */
                gcmkONERROR(gckWLFE_Event(hardware, gcvNULL, id,
                                          Event->queues[id].source, &bytes));

                /* Get the size of flush command. */
                gcmkONERROR(gckHARDWARE_Flush(hardware, flush, gcvNULL, &flushBytes));

                bytes += flushBytes;
#if gcdSHARED_COMMAND_BUFFER
                bytes += chipEnableBytes * 2;
#endif
            } else if (command->feType == gcvHW_FE_ASYNC) {
                /* Get the size of the hardware event. */
                gcmkONERROR(gckASYNC_FE_Event(hardware, gcvNULL, id,
                                              Event->queues[id].source, &bytes));
            } else {
                /* Get the size of the hardware event. */
                gcmkONERROR(gckMCFE_Event(hardware, gcvNULL, id,
                                          Event->queues[id].source, &bytes));
            }

            /* Total bytes need to execute. */
            executeBytes = bytes;

            /* Reserve space in the command queue. */
            gcmkONERROR(gckCOMMAND_Reserve(command, bytes, &buffer, &bytes));
#if gcdSECURITY
            reservedBuffer = buffer;
#endif

#if gcdINTERRUPT_STATISTIC
            gcmkVERIFY_OK(gckOS_AtomIncrement(Event->os, Event->interruptCount, &oldValue));
#endif

            if (command->feType == gcvHW_FE_WAIT_LINK || command->feType == gcvHW_FE_END) {
#if gcdSHARED_COMMAND_BUFFER
                gceCORE_3D_MASK mask = (gceCORE_3D_MASK)(1 << hardware->kernel->chipID);

                gcmkONERROR(gckWLFE_ChipEnable(hardware, buffer, mask, &chipEnableBytes));

                buffer = (gctUINT8_PTR)buffer + chipEnableBytes;
#endif

                /* Set the flush in the command queue. */
                gcmkONERROR(gckHARDWARE_Flush(hardware, flush, buffer, &flushBytes));

                /* Advance to next command. */
                buffer = (gctUINT8_PTR)buffer + flushBytes;

                /* Set the hardware event in the command queue. */
                gcmkONERROR(gckWLFE_Event(hardware, buffer, id,
                                          Event->queues[id].source, &bytes));

#if gcdSHARED_COMMAND_BUFFER
                buffer = (gctUINT8_PTR)buffer + bytes;
                mask = gcvCORE_3D_ALL_MASK;

                gcmkONERROR(gckWLFE_ChipEnable(hardware, buffer, mask, &chipEnableBytes));
#endif

#if gcdSECURITY
                gckKERNEL_SecurityExecute(Event->kernel, reservedBuffer, executeBytes);
#else
                /* Execute the hardware event. */
                if (command->feType == gcvHW_FE_WAIT_LINK)
                    gcmkONERROR(gckCOMMAND_Execute(command, executeBytes));
                else
                    gcmkONERROR(gckCOMMAND_ExecuteEnd(command, executeBytes));
#endif
            } else if (command->feType == gcvHW_FE_ASYNC) {
                /* Set the hardware event in the command queue. */
                gcmkONERROR(gckASYNC_FE_Event(hardware, buffer, id,
                                              Event->queues[id].source, &bytes));

                /* Execute the hardware event. */
                gcmkONERROR(gckCOMMAND_ExecuteAsync(command, executeBytes));
            } else {
                /* Set the hardware event in the command queue. */
                gcmkONERROR(gckMCFE_Event(hardware, buffer, id, Event->queues[id].source, &bytes));

                /* Execute the hardware event. */
                gcmkONERROR(gckCOMMAND_ExecuteMultiChannel(command, 0, 0, executeBytes));
            }

#if gcdNULL_DRIVER || gcdCAPTURE_ONLY_MODE
            /* Notify immediately on infinite hardware. */
            gcmkONERROR(gckEVENT_Interrupt(Event, 1 << id));

            gcmkONERROR(gckEVENT_Notify(Event, 0, gcvNULL));
#endif
        }

        if (broadcast) {
            /* Release the command queue. */
            gcmkONERROR(gckCOMMAND_ExitCommit(command, fromPower));
        }

#if !gcdNULL_DRIVER
        if (!fromPower)
            gcmkVERIFY_OK(_TryToIdleGPU(Event));
#endif
    }

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Need to unroll the mutex acquire. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->eventListMutex));
    }

    if (commitEntered) {
        /* Release the command queue mutex. */
        gcmkVERIFY_OK(gckCOMMAND_ExitCommit(command, fromPower));
    }

    if (id != 0xFF) {
        /* Need to unroll the event allocation. */
        Event->queues[id].head = gcvNULL;
    }

    if (status == gcvSTATUS_GPU_NOT_RESPONDING) {
        /* Broadcast GPU stuck. */
        status = gckOS_Broadcast(Event->os, Event->kernel->hardware, gcvBROADCAST_GPU_STUCK);
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckEVENT_Commit
 **
 **  Commit an event queue from the user.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gckPREEMPT_COMMIT PreemptCommit
 **          The preempt commit.
 **
 **      gctBOOL Forced
 **          Force fire a event. There won't be interrupt if there's no events
 **          queued. Force a event by append a dummy one if this parameter is on.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
#if gcdENABLE_SW_PREEMPTION
gceSTATUS
gckEVENT_PreemptCommit(gckEVENT Event, gckPREEMPT_COMMIT PreemptCommit, gctBOOL Forced)
{
    gceSTATUS status;
    gcsQUEUE_PTR record = gcvNULL;
    gcsEVENT_ATTR eventAttr;

    gcmkHEADER_ARG("Event=0x%x PreemptCommit=0x%x", Event, PreemptCommit);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);
    gcmkVERIFY_ARGUMENT(PreemptCommit != gcvNULL);

    record = PreemptCommit->eventQueue;

    /* Loop while there are records in the queue. */
    while (record != gcvNULL) {
        /* Append event record to event queue. */
        gcmkONERROR(gckEVENT_AddListEx(Event, &record->iface, gcvKERNEL_PIXEL,
                                       gcvTRUE, gcvFALSE, PreemptCommit->pid));

        /* Next record in the queue. */
        record = gcmUINT64_TO_PTR(record->next);
    }

    eventAttr.wait = gcvTRUE;
    eventAttr.shared = gcvFALSE;
    eventAttr.fromPower = gcvFALSE;
    eventAttr.broadcast = gcvTRUE;

    /* Submit the event list. */
    gcmkONERROR(gckEVENT_Submit(Event, &eventAttr));

    /* Success */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}
#endif

/*******************************************************************************
 **
 **  gckEVENT_Commit
 **
 **  Commit an event queue from the user.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gcsQUEUE_PTR Queue
 **          User event queue.
 **
 **      gctBOOL Forced
 **          Force fire a event. There won't be interrupt if there's no events
 **          queued. Force a event by append a dummy one if this parameter is on.
 **      gctBOOL Submit
 **          Submit event or not.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Commit(gckEVENT Event, gcsQUEUE_PTR Queue,
                gctBOOL Forced, gctBOOL Submit, gctBOOL Shared)
{
    gceSTATUS status;
    gcsQUEUE_PTR record = gcvNULL, next;
    gctUINT32 processID;
    gctBOOL needCopy = gcvFALSE;
    gctPOINTER pointer = gcvNULL;

    gcmkHEADER_ARG("Event=0x%x Queue=0x%x", Event, Queue);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);

    /* Get the current process ID. */
    gcmkONERROR(gckOS_GetProcessID(&processID));

    /* Query if we need to copy the client data. */
    gcmkONERROR(gckOS_QueryNeedCopy(Event->os, processID, &needCopy));

    /* Loop while there are records in the queue. */
    while (Queue != gcvNULL) {
        gcsQUEUE queue;

        if (needCopy) {
            /* Point to stack record. */
            record = &queue;

            /* Copy the data from the client. */
            gcmkONERROR(gckOS_CopyFromUserData(Event->os, record, Queue,
                                               gcmSIZEOF(gcsQUEUE)));
        } else {
            /* Map record into kernel memory. */
            gcmkONERROR(gckOS_MapUserPointer(Event->os, Queue,
                                             gcmSIZEOF(gcsQUEUE), &pointer));

            record = pointer;
        }

        /* Append event record to event queue. */
        gcmkONERROR(gckEVENT_AddList(Event, &record->iface,
                                     gcvKERNEL_PIXEL, gcvTRUE, gcvFALSE));

        /* Next record in the queue. */
        next = gcmUINT64_TO_PTR(record->next);

        if (!needCopy) {
            /* Unmap record from kernel memory. */
            gcmkONERROR(gckOS_UnmapUserPointer(Event->os, Queue,
                                               gcmSIZEOF(gcsQUEUE), (gctPOINTER *)record));
            record = gcvNULL;
        }

        Queue = next;
    }

    if (Forced && Event->queueHead == gcvNULL) {
        gcsHAL_INTERFACE iface;

        iface.command = gcvHAL_COMMIT_DONE;

        gcmkONERROR(gckEVENT_AddList(Event, &iface, gcvKERNEL_PIXEL,
                                     gcvFALSE, gcvTRUE));
    }

    if (Submit) {
        gcsEVENT_ATTR eventAttr;

        eventAttr.wait = gcvTRUE;
        eventAttr.shared = Shared;
        eventAttr.fromPower = gcvFALSE;
        eventAttr.broadcast = gcvTRUE;

        /* Submit the event list. */
        gcmkONERROR(gckEVENT_Submit(Event, &eventAttr));
    }

    /* Success */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (pointer) {
        /* Roll back. */
        gcmkVERIFY_OK(gckOS_UnmapUserPointer(Event->os, Queue,
                                             gcmSIZEOF(gcsQUEUE),
                                             (gctPOINTER *)pointer));
    }

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/*******************************************************************************
 **
 **  gckEVENT_Interrupt
 **
 **  Called by the interrupt service routine to store the triggered interrupt
 **  mask to be later processed by gckEVENT_Notify.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **      gctUINT32 Data
 **          Mask for the 32 interrupts.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Interrupt(gckEVENT Event, gctUINT32 Data)
{
    /* Combine current interrupt status with pending flags. */
    gckOS_AtomSetMask(Event->pending, Data);

#if gcdINTERRUPT_STATISTIC
    {
        gctINT   j = 0;
        gctINT32 oldValue;

        for (j = 0; j < Event->totalQueueCount; j++) {
            if ((Data & (1 << j)))
                gckOS_AtomDecrement(Event->os, Event->interruptCount, &oldValue);
        }
    }
#endif

    /* Success. */
    return gcvSTATUS_OK;
}

/*******************************************************************************
 **
 **  gckEVENT_Notify
 **
 **  Process all triggered interrupts.
 **
 **  INPUT:
 **
 **      gckEVENT Event
 **          Pointer to an gckEVENT object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckEVENT_Notify(gckEVENT Event, gctUINT32 IDs, gceEVENT_FAULT *Fault)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctINT i;
    gcsEVENT_QUEUE *queue;
    gctUINT mask = 0;
    gctBOOL acquired = gcvFALSE;
    gctSIGNAL signal;
    gctUINT pending  = 0;
    gceEVENT_FAULT fault = gcvEVENT_NO_FAULT;
    gckMMU mmu = gcvNULL;

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
    gctINT eventNumber = 0;
#endif
    gckVIDMEM_NODE nodeObject;

    gcmkHEADER_ARG("Event=0x%x IDs=0x%x", Event, IDs);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);

/* gcmDEBUG_ONLY(f) */
#if gcmIS_DEBUG(gcdDEBUG_CODE)
    if (IDs != 0) {
        for (i = 0; i < Event->totalQueueCount; ++i) {
            if (Event->queues[i].head != gcvNULL) {
                gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT,
                               "Queue(%d): stamp=%llu source=%d",
                               i, Event->queues[i].stamp, Event->queues[i].source);
            }
        }
    }
#endif

    /* Begin of event handling. */
    Event->notifyState = 0;

    for (;;) {
        gcsEVENT_PTR record;

        /* Grab the mutex queue. */
        gcmkONERROR(gckOS_AcquireMutex(Event->os, Event->eventQueueMutex, gcvINFINITE));
        acquired = gcvTRUE;

        gckOS_AtomGet(Event->os, Event->pending, (gctINT32_PTR)&pending);

        if (pending == 0) {
            /* Release the mutex queue. */
            gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
            acquired = gcvFALSE;

            /* No more pending interrupts - done. */
            break;
        }

        if (pending & (1 << 29)) {
            /* Event ID 29 is not a normal event, but for invalidating pipe. */
            gckHARDWARE_ResumeWLFE(Event->kernel->hardware);
            pending &= ~(1 << 29);
            gckOS_AtomClearMask(Event->pending, (1 << 29));
        }

        if (pending & 0x80000000) {
            gctUINT32 axi = 0;

            gcmkPRINT("AXI BUS ERROR");
            gckHARDWARE_ReadAxiStatusRegister(Event->kernel->hardware, &axi);
            gcmkPRINT("axi status = 0x%08X\n", axi);
            pending &= 0x7FFFFFFF;

            fault |= gcvEVENT_BUS_ERROR_FAULT;
        }

        if (pending & 0x40000000) {
#if gcdUSE_MMU_EXCEPTION

#if defined(EMULATOR) || defined(LINUXEMULATOR)
            gcmkPRINT("MMU exception is detected.\n");
# endif

#if gcdALLOC_ON_FAULT
            status = gckHARDWARE_HandleFault(Event->kernel->hardware);
# endif
            if (gcmIS_ERROR(status)) {
                /* Dump error is fault can't be handle. */
                gckHARDWARE_DumpMMUException(Event->kernel->hardware);

                gckHARDWARE_DumpGPUState(Event->kernel->hardware);
            }
#endif

            pending &= 0xBFFFFFFF;
        }

        gcmkTRACE_ZONE_N(gcvLEVEL_INFO, gcvZONE_EVENT, gcmSIZEOF(pending),
                         "Pending interrupts 0x%x", pending);

        queue = gcvNULL;

#if gcmIS_DEBUG(gcdDEBUG_CODE)
        if (IDs == 0) {
            for (i = 0; i < Event->totalQueueCount; ++i) {
                if (Event->queues[i].head != gcvNULL) {
                    gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT,
                                   "Queue(%d): stamp=%llu source=%d",
                                   i, Event->queues[i].stamp,
                                   Event->queues[i].source);
                }
            }
        }
#endif

        /* Find the oldest pending interrupt. */
        for (i = 0; i < Event->totalQueueCount; ++i) {
            if (Event->queues[i].head != gcvNULL && (pending & (1 << i))) {
                if (queue == gcvNULL || Event->queues[i].stamp < queue->stamp) {
                    queue = &Event->queues[i];
                    mask = 1 << i;
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
                    eventNumber = i;
#endif
                }
            }
        }

        if (queue == gcvNULL) {
            gcmkTRACE_ZONE_N(gcvLEVEL_ERROR, gcvZONE_EVENT, gcmSIZEOF(pending),
                             "Interrupts 0x%x are not pending.", pending);

            /* Clear the BUS ERROR event. */
            if (fault & gcvEVENT_BUS_ERROR_FAULT)
                pending |= (1 << 31);

            gckOS_AtomClearMask(Event->pending, pending);

            /* Release the mutex queue. */
            gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
            acquired = gcvFALSE;
            break;
        }

        /* Check whether there is a missed interrupt. */
        for (i = 0; i < Event->totalQueueCount; ++i) {
            if (Event->queues[i].head != gcvNULL &&
                Event->queues[i].stamp < queue->stamp &&
                Event->queues[i].source <= queue->source) {
                gcmkTRACE_N(gcvLEVEL_ERROR,
                            gcmSIZEOF(i) + gcmSIZEOF(Event->queues[i].stamp),
                            "Event %d lost (stamp %llu)",
                            i, Event->queues[i].stamp);

                /* Use this event instead. */
                queue = &Event->queues[i];
                mask = 0;
            }
        }

        if (mask != 0) {
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
            gcmkTRACE_ZONE_N(gcvLEVEL_INFO, gcvZONE_EVENT, gcmSIZEOF(eventNumber),
                             "Processing interrupt %d", eventNumber);
#endif
        }

        gckOS_AtomClearMask(Event->pending, mask);

        if (!gckHARDWARE_IsFeatureAvailable(Event->kernel->hardware, gcvFEATURE_FENCE_64BIT)) {
            /* Write out commit stamp.*/
            *(gctUINT64 *)(Event->kernel->command->fence->logical) = queue->commitStamp;
        }

        /* Signal clients waiting for fence. */
        gcmkVERIFY_OK(gckFENCE_Signal(Event->os, Event->kernel->command->fence));

        /* Grab the event head. */
        record = queue->head;

        /* Now quickly clear its event list. */
        queue->head = gcvNULL;

        /* Increase the number of free events. */
        Event->freeQueueCount++;

        /* Release the mutex queue. */
        gcmkONERROR(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
        acquired = gcvFALSE;

        /* Walk all events for this interrupt. */
        while (record != gcvNULL) {
            gcsEVENT_PTR recordNext;
#ifndef __QNXNTO__
            gctPOINTER logical;
#endif
            /* Grab next record. */
            recordNext = record->next;

#ifdef __QNXNTO__
            /*
             * Assign record->processID as the pid for this ftg340 thread.
             * Used in the OS calls which do not take a pid.
             */
            drv_thread_specific_key_assign(record->processID, 0);
#endif
            gcmkTRACE_ZONE_N(gcvLEVEL_INFO, gcvZONE_EVENT,
                             gcmSIZEOF(record->info.command),
                             "Processing event type: %d",
                             record->info.command);

            switch (record->info.command) {
            case gcvHAL_WRITE_DATA:
#ifndef __QNXNTO__
                /* Convert physical into logical address. */
                gcmkERR_BREAK(gckOS_MapPhysical(Event->os,
                                                record->info.u.WriteData.address,
                                                gcmSIZEOF(gctUINT32), &logical));

                /* Write data. */
                gcmkERR_BREAK(gckOS_WriteMemory(Event->os, logical,
                                                record->info.u.WriteData.data));

                /* Unmap the physical memory. */
                gcmkERR_BREAK(gckOS_UnmapPhysical(Event->os, logical, gcmSIZEOF(gctUINT32)));
#else
                /* Write data. */
                gcmkERR_BREAK(gckOS_WriteMemory(Event->os,
                                                gcmUINT64_TO_PTR(record->info.u.WriteData.address),
                                                record->info.u.WriteData.data));
#endif
                break;

            case gcvHAL_UNLOCK_VIDEO_MEMORY:
                gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT,
                               "gcvHAL_UNLOCK_VIDEO_MEMORY: 0x%x",
                               record->info.u.UnlockVideoMemory.node);

                nodeObject = gcmUINT64_TO_PTR(record->info.u.UnlockVideoMemory.node);

                /* Unlock, sync'ed. */
                gcmkERR_BREAK(gckVIDMEM_NODE_Unlock(Event->kernel, nodeObject,
                                                    record->mmu, gcvNULL));

                /* Deref node. */
                gcmkERR_BREAK(gckVIDMEM_NODE_DereferenceEx(Event->kernel, nodeObject, record->processID));

                break;

            case gcvHAL_SIGNAL:
                signal = gcmUINT64_TO_PTR(record->info.u.Signal.signal);
                gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT, "gcvHAL_SIGNAL: 0x%x", signal);

#ifdef __QNXNTO__
                if (record->info.u.Signal.event.sigev_notify == SIGEV_NONE &&
                    record->info.u.Signal.rcvid == 0) {
                    /* Kernel signal. */
                    gcmkERR_BREAK(gckOS_SignalPulse(Event->os, signal));
                } else {
                    /* User signal. */
                    gcmkERR_BREAK(gckOS_UserSignal(Event->os, signal,
                                                   record->info.u.Signal.rcvid,
                                                   &record->info.u.Signal.event));
                }
#else
                /* Set signal. */
                if (gcmUINT64_TO_PTR(record->info.u.Signal.process) == gcvNULL) {
                    /* Kernel signal. */
                    status = gckOS_Signal(Event->os, signal, gcvTRUE);
                    /* Skip not found print for kernel signal. */
                    if (gcmIS_ERROR(status))
                        break;
                } else {
                    gctHANDLE handle = gcvNULL;

                    if (IDs == 0)
                        handle = gcmUINT64_TO_PTR(record->info.u.Signal.process);
                    else if (IDs == 2)
                        handle = gcvINVALID_HANDLE;

                    /* User signal. */
                    gcmkERR_BREAK(gckOS_UserSignal(Event->os, signal, handle));
                }

                gcmkASSERT(record->info.u.Signal.auxSignal == 0);
#endif
                break;

            case gcvHAL_TIMESTAMP:
                gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT,
                               "gcvHAL_TIMESTAMP: %d %d",
                               record->info.u.TimeStamp.timer,
                               record->info.u.TimeStamp.request);

                /* Process the timestamp. */
                switch (record->info.u.TimeStamp.request) {
                case 0:
                    status = gckOS_GetTime(&Event->kernel->timers[record->info.u.TimeStamp.timer].stopTime);
                    break;

                case 1:
                    status = gckOS_GetTime(&Event->kernel->timers[record->info.u.TimeStamp.timer].startTime);
                    break;

                default:
                    gcmkTRACE_ZONE_N(gcvLEVEL_ERROR, gcvZONE_EVENT,
                                     gcmSIZEOF(record->info.u.TimeStamp.request),
                                     "Invalid timestamp request: %d",
                                     record->info.u.TimeStamp.request);

                    status = gcvSTATUS_INVALID_ARGUMENT;
                    break;
                }
                break;

            case gcvHAL_COMMIT_DONE:
                break;

            case gcvHAL_DESTROY_MMU:
                mmu = gcmUINT64_TO_PTR(record->info.u.DestroyMmu.mmu);
                if (mmu)
                    gcmkERR_BREAK(gckMMU_DestroyProcessMMU(mmu));

                break;

            default:
                /* Invalid argument. */
                gcmkTRACE_ZONE_N(gcvLEVEL_ERROR, gcvZONE_EVENT, gcmSIZEOF(record->info.command),
                                 "Unknown event type: %d", record->info.command);

                status = gcvSTATUS_INVALID_ARGUMENT;
                break;
            }

            /* Make sure there are no errors generated. */
            if (gcmIS_ERROR(status)) {
                gcmkTRACE_ZONE_N(gcvLEVEL_WARNING, gcvZONE_EVENT, gcmSIZEOF(status),
                                 "Event produced status: %d(%s)",
                                 status, gckOS_DebugStatus2Name(status));
            }

            /* Free the event. */
            gcmkVERIFY_OK(gckEVENT_FreeRecord(Event, record));

            /* Advance to next record. */
            record = recordNext;
        }

        gcmkTRACE_ZONE(gcvLEVEL_VERBOSE, gcvZONE_EVENT, "Handled interrupt 0x%x", mask);
    }

    if (IDs == 0)
        gcmkONERROR(_TryToIdleGPU(Event));

    /* End of event handling. */
    Event->notifyState = -1;

    if (Fault != gcvNULL)
        *Fault = fault;

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    if (acquired) {
        /* Release mutex. */
        gcmkVERIFY_OK(gckOS_ReleaseMutex(Event->os, Event->eventQueueMutex));
    }

    /* End of event handling. */
    Event->notifyState = -1;

    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckEVENT_ParityNotify(gckEVENT Event)
{
    gceSTATUS status = gcvSTATUS_OK;
    gctUINT pending = 0;

    gcmkHEADER_ARG("Event=0x%x", Event);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Event, gcvOBJ_EVENT);

    gckOS_AtomGet(Event->os, Event->pending, (gctINT32_PTR)&pending);

    if (pending & 0x1)
        gcmkPRINT("[ftg340]: Parity check error!\n");

    gckOS_AtomClearMask(Event->pending, 0);

    return status;
}

static void
_PrintRecord(gcsEVENT_PTR record)
{
    switch (record->info.command) {
    case gcvHAL_WRITE_DATA:
        gcmkPRINT("      gcvHAL_WRITE_DATA");
        break;

    case gcvHAL_UNLOCK_VIDEO_MEMORY:
        gcmkPRINT("      gcvHAL_UNLOCK_VIDEO_MEMORY");
        break;

    case gcvHAL_SIGNAL:
        gcmkPRINT("      gcvHAL_SIGNAL process=%lld signal=0x%llx",
                  record->info.u.Signal.process, record->info.u.Signal.signal);
        break;

    case gcvHAL_TIMESTAMP:
        gcmkPRINT("      gcvHAL_TIMESTAMP");
        break;

    case gcvHAL_COMMIT_DONE:
        gcmkPRINT("      gcvHAL_COMMIT_DONE");
        break;

    case gcvHAL_DESTROY_MMU:
        gcmkPRINT("      gcvHAL_DESTORY_MMU mmu=%p",
                  gcmUINT64_TO_PTR(record->info.u.DestroyMmu.mmu));

        break;
    default:
        gcmkPRINT("      Illegal Event %d", record->info.command);
        break;
    }
}

/*******************************************************************************
 ** gckEVENT_Dump
 **
 ** Dump record in event queue when stuck happens.
 ** No protection for the event queue.
 **/
gceSTATUS
gckEVENT_Dump(gckEVENT Event)
{
    gcsEVENT_QUEUE_PTR queueHead = Event->queueHead;
    gcsEVENT_QUEUE_PTR queue;
    gcsEVENT_PTR record = gcvNULL;
    gctINT i;
#if gcdINTERRUPT_STATISTIC
    gctINT32 pendingInterrupt;
    gctUINT32 intrAcknowledge;
#endif
    gctINT32 pending;

    gcmkHEADER_ARG("Event=0x%x", Event);

    gcmkPRINT("**************************\n");
    gcmkPRINT("***  EVENT STATE DUMP  ***\n");
    gcmkPRINT("**************************\n");

    gcmkPRINT("  Unsumbitted Event:");
    while (queueHead) {
        queue  = queueHead;
        record = queueHead->head;

        gcmkPRINT("    [%p]:", queue);
        while (record) {
            _PrintRecord(record);
            record = record->next;
        }

        if (queueHead == Event->queueTail)
            queueHead = gcvNULL;
        else
            queueHead = queueHead->next;
    }

    gcmkPRINT("  Untriggered Event:");
    for (i = 0; i < Event->totalQueueCount; i++) {
        queue  = &Event->queues[i];
        record = queue->head;

        gcmkPRINT("    [%d]:", i);
        while (record) {
            _PrintRecord(record);
            record = record->next;
        }
    }

#if gcdINTERRUPT_STATISTIC
    gckOS_AtomGet(Event->os, Event->interruptCount, &pendingInterrupt);
    gcmkPRINT("  Number of Pending Interrupt: %d", pendingInterrupt);

    if (Event->kernel->recovery == 0) {
        gceSTATUS status;

        status = gckOS_ReadRegisterEx(Event->os, Event->kernel, 0x10, &intrAcknowledge);
        if (gcmIS_ERROR(status))
            gcmkPRINT("  READ INTR_ACKNOWLEDGE ERROR!");
        else
            gcmkPRINT("  INTR_ACKNOWLEDGE=0x%x", intrAcknowledge);
    }
#endif

    gcmkPRINT("  Notify State=%d", Event->notifyState);

    gckOS_AtomGet(Event->os, Event->pending, &pending);

    gcmkPRINT("  Pending=0x%x", pending);

    gcmkFOOTER_NO();
    return gcvSTATUS_OK;
}
