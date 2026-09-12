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


#ifndef __gc_hal_kernel_context_h_
#define __gc_hal_kernel_context_h_

#ifdef __cplusplus
extern "C" {
#endif

/* Maps state locations within the context buffer. */
typedef struct _gcsSTATE_MAP *gcsSTATE_MAP_PTR;
typedef struct _gcsSTATE_MAP {
    /* Index of the state in the context buffer. */
    gctUINT                     index;

    /* State mask. */
    gctUINT32                   mask;
} gcsSTATE_MAP;

/* Context buffer. */
typedef struct _gcsCONTEXT *gcsCONTEXT_PTR;
typedef struct _gcsCONTEXT {
    /* For debugging: the number of context buffer in the order of creation. */
    gctUINT                     num;

    /* Pointer to gckEVENT object. */
    gckEVENT                    eventObj;

    /* Context busy signal. */
    gctSIGNAL                   signal;

    /* Video memory of the context buffer. */
    gckVIDMEM_NODE              videoMem;

#if gcdCAPTURE_ONLY_MODE
    gctUINT32                   handle;
#endif

    /* Logical address of the context buffer. */
    gctUINT32_PTR               logical;

    /* Hardware address of the context buffer. */
    gctADDRESS                  address;

    /* Pointer to the LINK commands. */
    gctPOINTER                  link2D;
    gctPOINTER                  link3D;

    /* The number of pending state deltas. */
    gctUINT                     deltaCount;

    /* Pointer to the first delta to be applied. */
    gcsSTATE_DELTA_PTR          delta;

#if gcdENABLE_SW_PREEMPTION
    /* Kernel delta. */
    gcsSTATE_DELTA_PTR          kDelta;
    gctUINT                     kDeltaCount;
#endif

    /* Next context buffer. */
    gcsCONTEXT_PTR              next;
} gcsCONTEXT;

#if gcdHARDWARE_CONTEXT_SWITCH
typedef enum _gceCONTEXT_SWITCH_OPERATION {
    gcvCONTEXT_INIT = 0,
    gcvCONTEXT_RESTORE,
    gcvCONTEXT_SAVE,
    gcvCONTEXT_SWITCH_OPERATION_COUNT,
} gceCONTEXT_SWITCH_OPERATION;
#endif

typedef struct _gcsRECORD_ARRAY_MAP *gcsRECORD_ARRAY_MAP_PTR;
struct  _gcsRECORD_ARRAY_MAP {
    /* User pointer key. */
    gctUINT64                   key;

    /* Kernel memory buffer. */
    gcsSTATE_DELTA_RECORD_PTR   kData;

    /* Next map. */
    gcsRECORD_ARRAY_MAP_PTR     next;

};

#define USE_SW_RESET 1

/* gckCONTEXT structure that hold the current context. */
struct _gckCONTEXT {
    /* Object. */
    gcsOBJECT                   object;

    /* Pointer to gckOS object. */
    gckOS                       os;

    /* Pointer to gckHARDWARE object. */
    gckHARDWARE                 hardware;

    /* Command buffer alignment. */
    gctUINT32                   alignment;
    gctUINT32                   reservedHead;

    /* Context buffer metrics. */
    gctSIZE_T                   maxState;
    gctUINT32                   numStates;
    gctUINT32                   totalSize;
    gctUINT32                   bufferSize;
    gctUINT32                   linkIndex2D;
    gctUINT32                   linkIndex3D;
    gctUINT32                   linkIndexXD;
    gctUINT32                   entryOffset3D;
    gctUINT32                   entryOffsetXDFrom2D;
    gctUINT32                   entryOffsetXDFrom3D;

    /* State mapping. */
    gcsSTATE_MAP_PTR            map;

    /* List of context buffers. */
    gcsCONTEXT_PTR              buffer;

    /* Requested pipe select for context. */
    gcePIPE_SELECT              entryPipe;
    gcePIPE_SELECT              exitPipe;

    /* Variables used for building state buffer. */
    gctADDRESS                  lastAddress;
    gctSIZE_T                   lastSize;
    gctUINT32                   lastIndex;
    gctBOOL                     lastFixed;

#if gcdHARDWARE_CONTEXT_SWITCH
    gctBOOL                     needInitialize;
    gctBOOL                     probeEnabled;
    gctUINT64                   lastStamp;
    gcsCONTEXT_PTR              probeBuffer;
#endif

    gctUINT32                   pipeSelectBytes;

#if gcdENABLE_SW_PREEMPTION
    /* Kernel delta. */
    gcsSTATE_DELTA_PTR            delta;
    gcsSTATE_DELTA_PTR            deltaHead;

    gcsSTATE_DELTA                prevDelta;
    gcsSTATE_DELTA_PTR            prevDeltaPtr;
    gcsSTATE_DELTA_RECORD_PTR     prevRecordArray;
    gctUINT32                    *prevMapEntryID;
    gctUINT32                    *prevMapEntryIndex;
#endif
    gctBOOL                       smallBatch;
};

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_kernel_context_h_ */
