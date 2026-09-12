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


#ifndef __gc_hal_kernel_hardware_func_h_
#define __gc_hal_kernel_hardware_func_h_
#include "gc_hal.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gcsFUNCTION_EXECUTION *gcsFUNCTION_EXECUTION_PTR;

typedef enum {
    gcvFUNCTION_EXECUTION_MMU,
    gcvFUNCTION_EXECUTION_FLUSH,
    gcvFUNCTION_EXECUTION_NUM
} gceFUNCTION_EXECUTION;

typedef struct {
    /* Data Vidmem object */
    gckVIDMEM_NODE          bufVidMem;

    /* Total bytes of the data. */
    gctSIZE_T               bufVidMemBytes;

    /* Entry of the data. */
    gctADDRESS              address;

    /* CPU address of the data. */
    gctPOINTER              logical;

    /* Actually bytes of the data. */
    gctSIZE_T               bytes;
} gcsFUNCTION_EXECUTION_DATA, *gcsFUNCTION_EXECUTION_DATA_PTR;

typedef struct {
    gceSTATUS (*validate)(gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*init)(gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*execute)(gcsFUNCTION_EXECUTION_PTR Execution);
    gceSTATUS (*release)(gcsFUNCTION_EXECUTION_PTR Execution);
} gcsFUNCTION_API, *gcsFUNCTION_API_PTR;

typedef struct _gcsFUNCTION_COMMAND {
    /* Function Vidmem object */
    gckVIDMEM_NODE          funcVidMem;

    /* Total bytes of the function. */
    gctSIZE_T               funcVidMemBytes;

    /* Entry of the function. */
    gctADDRESS              address;

    /* CPU address of the function. */
    gctPOINTER              logical;

    /* Physical address of this command. */
    gctPHYS_ADDR_T          physical;

    /* Actually bytes of the function. */
    gctUINT32               bytes;

    /* Hardware address of END in this function. */
    gctADDRESS              endAddress;

    /* Logical of END in this function. */
    gctUINT8_PTR            endLogical;

    /* Physical of End in this function. */
    gctPHYS_ADDR_T          endPhysical;

    /* mcfe channel set. */
    gctUINT32               channelId;

    /* Function private data */
    gctUINT32               dataCount;
    gcsFUNCTION_EXECUTION_DATA_PTR      data;
} gcsFUNCTION_COMMAND, *gcsFUNCTION_COMMAND_PTR;

typedef struct _gcsFUNCTION_EXECUTION {
    gctPOINTER              hardware;

    /* Function name */
    gctCHAR                 funcName[16];

    /* Function ID */
    gceFUNCTION_EXECUTION   funcId;

    /* Function count */
    gctUINT8                funcCmdCount;

    /* Function array */
    gcsFUNCTION_COMMAND_PTR funcCmd;

    /* API of functions */
    gcsFUNCTION_API         funcExecution;

    /* Need this function or not */
    gctBOOL                 valid;

    /* Function has inited */
    gctBOOL                 inited;
} gcsFUNCTION_EXECUTION;

gceSTATUS
gckFUNCTION_Construct(gctPOINTER Hardware);

gceSTATUS
gckFUNCTION_Destory(gctPOINTER Hardware);

gceSTATUS
gckFUNCTION_Validate(gcsFUNCTION_EXECUTION_PTR Execution, gctBOOL_PTR Valid);

gceSTATUS
gckFUNCTION_Init(gcsFUNCTION_EXECUTION_PTR Execution);

gceSTATUS
gckFUNCTION_Execute(gcsFUNCTION_EXECUTION_PTR Execution);

gceSTATUS
gckFUNCTION_Release(gcsFUNCTION_EXECUTION_PTR Execution);

void
gckFUNCTION_Dump(gcsFUNCTION_EXECUTION_PTR Execution);
#ifdef __cplusplus
}
#endif
#endif
