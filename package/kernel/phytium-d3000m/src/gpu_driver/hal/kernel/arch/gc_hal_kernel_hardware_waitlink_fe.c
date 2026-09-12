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

#define _GC_OBJ_ZONE gcvZONE_HARDWARE

gceSTATUS
gckWLFE_Construct(gckHARDWARE Hardware, gckWLFE *FE)
{
    /* Just a non-null value. */
    *FE = (gckWLFE)(gctUINTPTR_T)1;
    return gcvSTATUS_OK;
}

void
gckWLFE_Destroy(gckHARDWARE Hardware, gckWLFE FE)
{
    gcmkASSERT(FE);
}

gceSTATUS
gckWLFE_Initialize(gckHARDWARE Hardware, gckWLFE FE)
{
    gcmkASSERT(FE);
    return gcvSTATUS_OK;
}

/*******************************************************************************
 *
 *  gckWLFE_WaitLink
 *
 *  Append a WAIT/LINK command sequence at the specified location in the command
 *  queue.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to an gckHARDWARE object.
 *
 *      gctPOINTER Logical
 *          Pointer to the current location inside the command queue to append
 *          WAIT/LINK command sequence at or gcvNULL just to query the size of
 *          the WAIT/LINK command sequence.
 *
 *      gctADDRESS Address
 *          GPU address of current location inside the command queue.
 *
 *      gctUINT32 Offset
 *          The Offset into command buffer required for alignment.
 *
 *      gctSIZE_T *Bytes
 *          Pointer to the number of bytes available for the WAIT/LINK command
 *          sequence.  If 'Logical' is gcvNULL, this argument will be ignored.
 *
 *  OUTPUT:
 *
 *      gctSIZE_T *Bytes
 *          Pointer to a variable that will receive the number of bytes
 *          required by the WAIT/LINK command sequence.  If 'Bytes' is gcvNULL,
 *          nothing will be returned.
 *
 *      gctUINT32 *WaitOffset
 *          Pointer to a variable that will receive the offset of the WAIT
 *          command from the specified logcial pointer. If 'WaitOffset' is
 *          gcvNULL nothing will be returned.
 *
 *      gctSIZE_T *WaitSize
 *          Pointer to a variable that will receive the number of bytes used by
 *          the WAIT command.  If 'LinkSize' is gcvNULL nothing will be
 *          returned.
 */
gceSTATUS
gckWLFE_WaitLink(gckHARDWARE Hardware,
                 gctPOINTER Logical,
                 gctADDRESS Address,
                 gctUINT32 Offset,
                 gctUINT32 *Bytes,
                 gctUINT32 *WaitOffset,
                 gctUINT32 *WaitSize)
{
    gceSTATUS status;
    gctUINT32_PTR logical;
    gctUINT32 bytes, waitSize;
    gctBOOL useL2;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x Offset=0x%08x *Bytes=0x%x",
                   Hardware, Logical, Offset, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    gcmkASSERT(Hardware->wlFE);
    useL2 = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_64K_L2_CACHE);

    /* Compute number of bytes required. */
    if (Hardware->graphicsLargeVA) {
        if (useL2) {
            bytes = gcmALIGN(Offset + 40, 16) - Offset;
            waitSize = 24;
        } else {
            bytes = gcmALIGN(Offset + 32, 16) - Offset;
            waitSize = 16;
        }
    } else {
        if (useL2) {
            bytes = gcmALIGN(Offset + 24, 8) - Offset;
            waitSize = 16;
        } else {
            bytes = gcmALIGN(Offset + 16, 8) - Offset;
            waitSize = 8;
        }
    }

    /* Cast the input pointer. */
    logical = (gctUINT32_PTR)Logical;

    if (logical != gcvNULL) {
        /* Not enough space? */
        if (*Bytes < bytes) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        gcmkASSERT(Address != gcvINVALID_ADDRESS);

        /* Store the WAIT/LINK address. */
        Hardware->lastWaitLink = Address;

        if (Hardware->graphicsLargeVA) {
            gctUINT64_PTR opLogical = (gctUINT64_PTR)Logical;

            if(Address & 0xF){
                gcmkPRINT("waitlink not aligned from:");
                gckOS_DumpCallStack(Hardware->os);
            }

            /* Append WAIT(count). */
            *opLogical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_WAIT_COMMAND, OPCODE, WAIT64) |
                              gcmSETFIELD(0, AQ_COMMAND_WAIT_COMMAND, DELAY,  Hardware->waitCount);

            opLogical++;

            if (useL2) {
                logical = (gctUINT32_PTR)opLogical;

                /* LoadState(AQFlush, 1), flush. */
                *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQFlushRegAddrs) |
                                  gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

                *logical++ = gcmSETFIELDVALUE(0, AQ_FLUSH, L2_CACHE, ENABLE);

                opLogical = (gctUINT64_PTR)logical;
            }

            *opLogical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LINK_COMMAND, OPCODE, LINK64) |
                              gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND, PREFETCH, bytes >> 3);


            *opLogical = Address;

            gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, opLogical));
        } else {
            gctUINT32 address;

            /* Append WAIT(count). */
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_WAIT_COMMAND, OPCODE, WAIT) |
                              gcmSETFIELD(0, AQ_COMMAND_WAIT_COMMAND, DELAY,  Hardware->waitCount);

            logical++;


            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LINK_COMMAND, OPCODE,   LINK) |
                              gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND, PREFETCH, bytes >> 3);

            gcmkSAFECASTVA(address, Address);

            *logical = address;
            gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, logical));
        }

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "0x%llx: WAIT %u",
                       Address, Hardware->waitCount);

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "0x%llx: LINK 0x%llx, #0x%x",
                       Address + waitSize, Address, bytes);

        if (WaitOffset != gcvNULL) {
            /* Return the offset pointer to WAIT command. */
            *WaitOffset = 0;
        }

        if (WaitSize != gcvNULL) {
            /* Return number of bytes used by the WAIT command. */
            *WaitSize = waitSize;
        }
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the WAIT/LINK command
         * sequence.
         */
        *Bytes = bytes;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x *WaitOffset=0x%x *WaitSize=0x%x",
                   gcmOPT_VALUE(Bytes), gcmOPT_VALUE(WaitOffset), gcmOPT_VALUE(WaitSize));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

gceSTATUS
gckWLFE_InvalidatePipe(gckHARDWARE Hardware,
                       gctPOINTER Logical,
                       gctADDRESS Address,
                       gctUINT32 *Bytes)
{
    gctUINT size;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gceSTATUS status;
    gctBOOL blt = gcvFALSE;
    gctBOOL multiCluster = gcvFALSE;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x *Bytes=0x%x",
                   Hardware, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    gcmkASSERT(Hardware->wlFE);

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE)) {
        /* Send all event from blt. */
        blt = gcvTRUE;
        multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);
    }

    /* Determine the size of the command. */
    size = Hardware->extraEventStates ?
           gcmALIGN(8 + (1 + 5) * 4, 8) /* EVENT + 5 STATES */
           :
           8;

    if (blt) {
        size += 16;
        if (multiCluster)
            size += 8;
    }

    /* END. */
    size += 8;

    if (Logical != gcvNULL) {
        if (*Bytes < size) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        if (blt) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

            if (multiCluster) {
                gctUINT32 clusterMask;

                gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltClusterControlRegAddrs);

                *logical++ = gcmSETFIELD(0, GCREG_BLT_CLUSTER_CONTROL, CLUSTER_ENABLE, clusterMask);
            }
        }

        /* Append EVENT(Event, PE_SRC). */
        *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQEventRegAddrs) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *logical++ = gcmSETFIELDVALUE(0, AQ_EVENT, PE_SRC,   ENABLE) |
                          gcmSETFIELD(0, AQ_EVENT, EVENT_ID, EVENT_ID_INVALIDATE_PIPE);

        if (blt) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
        }

#if gcmIS_DEBUG(gcdDEBUG_TRACE)
        {
            gctPHYS_ADDR_T phys;

            gckOS_GetPhysicalAddress(Hardware->os, Logical, &phys);
            gckOS_CPUPhysicalToGPUPhysical(Hardware->os, phys, &phys);
            gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                           "0x%08llx: EVENT %d", phys, EVENT_ID_INVALIDATE_PIPE);
        }
#endif

        /* Append the extra states. These are needed for the chips that do not
         * support back-to-back events due to the async interface. The extra
         * states add the necessary delay to ensure that event IDs do not
         * collide.
         */
        if (Hardware->extraEventStates) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQMemoryFePageTableRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   5);
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
        }

#if gcdINTERRUPT_STATISTIC
        if (Hardware->kernel->eventObj->totalQueueCount > EVENT_ID_INVALIDATE_PIPE) {
            gckOS_AtomSetMask(Hardware->pendingEvent,
                              1 << EVENT_ID_INVALIDATE_PIPE);
        }
#endif

        /* Append END. */
        *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_END_COMMAND, OPCODE, END);

        /* Record the count of execution which is finised by this END. */
        *logical++ = Hardware->executeCount;

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "%p: END", Logical);

        /* Make sure the CPU writes out the data to memory. */
        gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, Logical));

        Hardware->lastEnd = Address + size - 8;
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the EVENT command. */
        *Bytes = size;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/******************************************************************************
 *
 *  gckWLFE_Link
 *
 *  Append a LINK command at the specified location in the command queue.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to an gckHARDWARE object.
 *
 *      gctPOINTER Logical
 *          Pointer to the current location inside the command queue to append
 *          the LINK command at or gcvNULL just to query the size of the LINK
 *          command.
 *
 *      gctADDRESS FetchAddress
 *          Hardware address of destination of LINK.
 *
 *      gctSIZE_T FetchSize
 *          Number of bytes in destination of LINK.
 *
 *      gctSIZE_T *Bytes
 *          Pointer to the number of bytes available for the LINK command.  If
 *          'Logical' is gcvNULL, this argument will be ignored.
 *
 *  OUTPUT:
 *
 *      gctSIZE_T *Bytes
 *          Pointer to a variable that will receive the number of bytes required
 *          for the LINK command.  If 'Bytes' is gcvNULL, nothing will be
 *          returned.
 */
gceSTATUS
gckWLFE_Link(gckHARDWARE Hardware,
             gctPOINTER Logical,
             gctADDRESS FetchAddress,
             gctUINT32 FetchSize,
             gctUINT32 *Bytes,
             gctUINT64 *Low,
             gctUINT64 *High)
{
    gceSTATUS status;
    gctSIZE_T bytes, linkBytes;
    gctUINT32 link, address;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x FetchAddress=0x%x FetchSize=0x%x *Bytes=0x%x",
                   Hardware, Logical, FetchAddress, FetchSize, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    gcmkASSERT(Hardware->wlFE);

    linkBytes = Hardware->graphicsLargeVA ? 16 : 8;

    if (Logical != gcvNULL) {
        if (*Bytes < linkBytes) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        address = (gctUINT32)FetchAddress;

        if (High)
            *High = FetchAddress;

        /* Compute number of 64-byte aligned bytes to fetch. */
        bytes = (gctSIZE_T)(gcmALIGN(FetchAddress + FetchSize, 64) - FetchAddress);

        if (Hardware->graphicsLargeVA) {
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 1, 0));
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 2, address));
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 3, FetchAddress >> 32));
            gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, logical + 3));

            /* Append LINK(bytes / 8), FetchAddress. */
            link = gcmSETFIELDVALUE(0, AQ_COMMAND_LINK_COMMAND, OPCODE,   LINK64) |
                        gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND, PREFETCH, bytes >> 3);

        } else {
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 1, address));
            gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, logical + 1));

            /* Append LINK(bytes / 8), FetchAddress. */
            link = gcmSETFIELDVALUE(0, AQ_COMMAND_LINK_COMMAND, OPCODE,   LINK) |
                        gcmSETFIELD(0, AQ_COMMAND_LINK_COMMAND, PREFETCH, bytes >> 3);
        }

        gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical, link));

        if (Low)
            *Low = link;

        /* Memory barrier. */
        gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, logical));
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the LINK command. */
        *Bytes = linkBytes;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%x", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/******************************************************************************
 *
 *  gckWLFE_End
 *
 *  Append an END command at the specified location in the command queue.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to an gckHARDWARE object.
 *
 *      gctPOINTER Logical
 *          Pointer to the current location inside the command queue to append
 *          END command at or gcvNULL just to query the size of the END
 *          command.
 *
 *      gctADDRESS Address
 *          GPU address of current location inside the command queue.
 *
 *      gctUINT32 User
 *          If we write the user logical or not.
 *
 *      gctSIZE_T *Bytes
 *          Pointer to the number of bytes available for the END command.  If
 *          'Logical' is gcvNULL, this argument will be ignored.
 *
 *  OUTPUT:
 *
 *      gctSIZE_T *Bytes
 *          Pointer to a variable that will receive the number of bytes
 *          required for the END command.  If 'Bytes' is gcvNULL, nothing
 *          will be returned.
 */
gceSTATUS
gckWLFE_EndEx(gckHARDWARE Hardware,
              gctPOINTER Logical,
              gctADDRESS Address,
              gctBOOL User,
              gctUINT32 *Bytes)
{
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gctUINT32 end;
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x *Bytes=0x%x",
                   Hardware, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkASSERT(Hardware->wlFE);

    if (Logical != gcvNULL) {
        if (Bytes && *Bytes < 8) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        end = gcmSETFIELDVALUE(0, AQ_COMMAND_END_COMMAND, OPCODE, END);

        if (User) {
            /* Append END. */
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical, end));

            /* Record the count of execution which is finised by this END. */
            gcmkONERROR(gckOS_WriteMemory(Hardware->os, logical + 1, Hardware->executeCount));
        } else {
            /* Append END. */
            logical[0] = end;
            /* Record the count of execution which is finised by this END. */
            logical[1] = Hardware->executeCount;
        }

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "%p: END", Logical);

        /* Make sure the CPU writes out the data to memory. */
        gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, Logical));

        gcmkASSERT(Address != ~0U);

        Hardware->lastEnd = Address;
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the END command. */
        *Bytes = 8;
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
gckWLFE_End(gckHARDWARE Hardware,
            gctPOINTER Logical,
            gctADDRESS Address,
            gctUINT32 *Bytes)
{
    return gckWLFE_EndEx(Hardware, Logical, Address, gcvFALSE, Bytes);
}

/******************************************************************************
 *
 *  gckWLFE_Nop
 *
 *  Append a NOP command at the specified location in the command queue.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to an gckHARDWARE object.
 *
 *      gctPOINTER Logical
 *          Pointer to the current location inside the command queue to append
 *          NOP command at or gcvNULL just to query the size of the NOP
 *          command.
 *
 *      gctSIZE_T *Bytes
 *          Pointer to the number of bytes available for the NOP command.  If
 *          'Logical' is gcvNULL, this argument will be ignored.
 *
 *  OUTPUT:
 *
 *      gctSIZE_T *Bytes
 *          Pointer to a variable that will receive the number of bytes
 *          required for the NOP command.  If 'Bytes' is gcvNULL, nothing
 *          will be returned.
 */
gceSTATUS
gckWLFE_Nop(gckHARDWARE Hardware, gctPOINTER Logical, gctSIZE_T *Bytes)
{
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x *Bytes=0x%zx",
                   Hardware, Logical, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkASSERT(Hardware->wlFE);

    if (Logical != gcvNULL) {
        /* Append NOP. */
        logical[0] = gcmSETFIELDVALUE(0, AQ_COMMAND_NOP_COMMAND, OPCODE, NOP);

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE, "%p: NOP", Logical);
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the NOP command. */
        *Bytes = 4;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%zx", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

}

/******************************************************************************
 *
 *  gckWLFE_Event
 *
 *  Append an EVENT command at the specified location in the command queue.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to an gckHARDWARE object.
 *
 *      gctPOINTER Logical
 *          Pointer to the current location inside the command queue to append
 *          the EVENT command at or gcvNULL just to query the size of the EVENT
 *          command.
 *
 *      gctUINT8 Event
 *          The Event ID to program.
 *
 *      gceKERNEL_WHERE FromWhere
 *          Location of the pipe to send the event.
 *
 *      gctSIZE_T *Bytes
 *          Pointer to the number of bytes available for the EVENT command.  If
 *          'Logical' is gcvNULL, this argument will be ignored.
 *
 *  OUTPUT:
 *
 *      gctSIZE_T *Bytes
 *          Pointer to a variable that will receive the number of bytes
 *          required for the EVENT command.  If 'Bytes' is gcvNULL, nothing
 *          will be returned.
 */
gceSTATUS
gckWLFE_Event(gckHARDWARE Hardware,
              gctPOINTER Logical,
              gctUINT8 Event,
              gceKERNEL_WHERE FromWhere,
              gctUINT32 *Bytes)
{
    gctUINT size;
    gctUINT32 destination = 0;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gceSTATUS status;
    gctBOOL blt;
    gctBOOL extraEventStates;
    gctBOOL multiCluster;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x Event=%u FromWhere=%d *Bytes=0x%x",
                   Hardware, Logical, Event, FromWhere, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);
    gcmkVERIFY_ARGUMENT(Event < 32);

    gcmkASSERT(Hardware->wlFE);

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BLT_ENGINE)) {
        /* Send all event from blt. */
        if (FromWhere == gcvKERNEL_PIXEL)
            FromWhere = gcvKERNEL_BLT;
    }

    blt = FromWhere == gcvKERNEL_BLT ? gcvTRUE : gcvFALSE;

    multiCluster = gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_MULTI_CLUSTER);

    /* Determine the size of the command. */

    extraEventStates = Hardware->extraEventStates && (FromWhere == gcvKERNEL_PIXEL);

    size = extraEventStates ?
           gcmALIGN(8 + (1 + 5) * 4, 8) /* EVENT + 5 STATES */
           :
           8;

    if (blt) {
        size += 16;
        if (multiCluster)
            size += 8;
    }

    if (Logical != gcvNULL) {
        if (*Bytes < size) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        switch (FromWhere) {
        case gcvKERNEL_COMMAND:
            /* From command processor. */
            destination = gcmSETFIELDVALUE(0, AQ_EVENT, FE_SRC, ENABLE);
            break;

        case gcvKERNEL_PIXEL:
            /* From pixel engine. */
            destination = gcmSETFIELDVALUE(0, AQ_EVENT, PE_SRC, ENABLE);
            break;

        case gcvKERNEL_BLT:
            destination = gcmSETFIELDVALUE(0, AQ_EVENT, BLT_SRC, ENABLE);
            break;

        default:
            gcmkONERROR(gcvSTATUS_INVALID_ARGUMENT);
        }

        if (blt) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, LOCK);

            if (multiCluster) {
                gctUINT32 clusterMask;

                gcmkONERROR(gckHARDWARE_QueryClusterInfo(Hardware, &clusterMask));

                *logical++ =
                    gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                         gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltClusterControlRegAddrs);

                *logical++ = gcmSETFIELD(0, GCREG_BLT_CLUSTER_CONTROL, CLUSTER_ENABLE, clusterMask);
            }
        }

        /* Append EVENT(Event, destination). */
        *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQEventRegAddrs) |
                          gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1);

        *logical++ = gcmSETFIELD(destination, AQ_EVENT, EVENT_ID, Event);

        if (blt) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   1) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, gcregBltGeneralControlRegAddrs);

            *logical++ = gcmSETFIELDVALUE(0, GCREG_BLT_GENERAL_CONTROL, STREAM_CONTROL, UNLOCK);
        }

        /* Make sure the event ID gets written out before GPU can access it. */
        gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, logical + 1));

        /* Append the extra states. These are needed for the chips that do not
         * support back-to-back events due to the async interface. The extra
         * states add the necessary delay to ensure that event IDs do not
         * collide.
         */
        if (extraEventStates) {
            *logical++ = gcmSETFIELDVALUE(0, AQ_COMMAND_LOAD_STATE_COMMAND, OPCODE,  LOAD_STATE) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, ADDRESS, AQMemoryFePageTableRegAddrs) |
                              gcmSETFIELD(0, AQ_COMMAND_LOAD_STATE_COMMAND, COUNT,   5);
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
            *logical++ = 0;
        }

#if gcdINTERRUPT_STATISTIC
        if (Event < (gctUINT8)Hardware->kernel->eventObj->totalQueueCount)
            gckOS_AtomSetMask(Hardware->pendingEvent, 1 << Event);
#endif
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the EVENT command. */
        *Bytes = size;
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
gckWLFE_ChipEnable(gckHARDWARE Hardware,
                   gctPOINTER Logical,
                   gceCORE_3D_MASK ChipEnable,
                   gctSIZE_T *Bytes)
{
    gckOS os = Hardware->os;
    gctUINT32_PTR logical = (gctUINT32_PTR)Logical;
    gceSTATUS status;

    gcmkHEADER_ARG("Hardware=0x%x Logical=0x%x ChipEnable=0x%x *Bytes=0x%zx",
                   Hardware, Logical, ChipEnable, gcmOPT_VALUE(Bytes));

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);
    gcmkVERIFY_ARGUMENT(Bytes != gcvNULL);

    gcmkASSERT(Hardware->wlFE);

    if (Logical != gcvNULL) {
        if (*Bytes < 8) {
            /* Command queue too small. */
            gcmkONERROR(gcvSTATUS_BUFFER_TOO_SMALL);
        }

        /* Append CHIPENABLE. */
        gcmkWRITE_MEMORY(logical,
                         gcmSETFIELDVALUE(0, GCCMD_CHIP_ENABLE_COMMAND, OPCODE, CHIP_ENABLE) | ChipEnable);

        gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                       "%p: CHIPENABLE 0x%x", Logical, ChipEnable);
    }

    if (Bytes != gcvNULL) {
        /* Return number of bytes required by the CHIPENABLE command. */
        *Bytes = 8;
    }

    /* Success. */
    gcmkFOOTER_ARG("*Bytes=0x%zx", gcmOPT_VALUE(Bytes));
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}

/******************************************************************************
 *
 *  gckWLFE_Execute
 *
 *  Kickstart the hardware's command processor with an initialized command
 *  buffer.
 *
 *  INPUT:
 *
 *      gckHARDWARE Hardware
 *          Pointer to the gckHARDWARE object.
 *
 *      gctADDRESS Address
 *          Hardware address of command buffer.
 *
 *      gctUINT32 Bytes
 *          Number of bytes for the prefetch unit (until after the first LINK).
 *
 *  OUTPUT:
 *
 *      Nothing.
 */
gceSTATUS
gckWLFE_Execute(gckHARDWARE Hardware, gctADDRESS Address, gctUINT32 Bytes)
{
    gceSTATUS status;
    gctUINT32 control;
    gctUINT32 eventEnable = 0xFFFFFFFF;
    gckCOMMAND command = Hardware->kernel->command;
    gctUINT32 address;

    gcmkHEADER_ARG("Hardware=0x%x Address=0x%llx Bytes=0x%x", Hardware, Address, Bytes);

    /* Verify the arguments. */
    gcmkVERIFY_OBJECT(Hardware, gcvOBJ_HARDWARE);

    gcmkASSERT(Hardware->wlFE);
    gcmkASSERT(command);

    if (command->feType == gcvHW_FE_END) {
        gctUINT idle = 0;

        /* Make sure FE is idle. */
        do {
            gcmkVERIFY_OK(gckOS_ReadRegisterEx(Hardware->os, Hardware->kernel,
                                               AQ_HI_IDLE_Address, &idle));
        } while (idle != 0x7FFFFFFF);
    }

    /* Enable all events. */
    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_INTR_ENBL_Address, eventEnable));

    if (gckHARDWARE_IsFeatureAvailable(Hardware, gcvFEATURE_BIT_SRAM_PARITY)) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_AHB_INTR_ENBL_Address, 0x1));
    }

    address = (gctUINT32)Address;

    /* Write address register. */
    if (Hardware->graphicsLargeVA) {
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_CMD_BUFFER_ADDR_HI_Address, (gctUINT32)(Address >> 32)));
    }


    gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                      AQ_CMD_BUFFER_ADDR_Address, address));

    /* Build control register. */
    control = gcmSETFIELDVALUE(0, AQ_CMD_BUFFER_CTRL, ENABLE,   ENABLE) |
                   gcmSETFIELD(0, AQ_CMD_BUFFER_CTRL, PREFETCH, (Bytes + 7) >> 3);

    /* Set big endian */
    if (Hardware->bigEndian)
        control |= gcmSETFIELDVALUE(0, AQ_CMD_BUFFER_CTRL, ENDIAN_CONTROL, SWAP_DWORD);

    /* Make sure writing to command buffer and previous AHB register is done. */
    gcmkONERROR(gckOS_MemoryBarrier(Hardware->os, gcvNULL));

    /* Write control register. */
    switch (Hardware->options.secureMode) {
    case gcvSECURE_NONE:
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_CMD_BUFFER_CTRL_Address, control));
        break;
    case gcvSECURE_IN_NORMAL:
#if defined(__KERNEL__)
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          AQ_CMD_BUFFER_CTRL_Address, control));
#endif
        gcmkONERROR(gckOS_WriteRegisterEx(Hardware->os, Hardware->kernel,
                                          GCREG_CMD_BUFFER_AHB_CTRL_Address, control));
        break;
#if gcdENABLE_TRUST_APPLICATION
    case gcvSECURE_IN_TA:
        /* Send message to TA. */
        gcmkONERROR(gckKERNEL_SecurityStartCommand(Hardware->kernel, address, (gctUINT32)Bytes));
        break;
#endif
    default:
        break;
    }

    /* Increase execute count. */
    Hardware->executeCount++;

    /* Record last execute address. */
    Hardware->lastExecuteAddress = Address;

    gcmkTRACE_ZONE(gcvLEVEL_INFO, gcvZONE_HARDWARE,
                   "Started command buffer @ 0x%llx", Address);

    /* Success. */
    gcmkFOOTER_NO();
    return gcvSTATUS_OK;

OnError:
    /* Return the status. */
    gcmkFOOTER();
    return status;
}
