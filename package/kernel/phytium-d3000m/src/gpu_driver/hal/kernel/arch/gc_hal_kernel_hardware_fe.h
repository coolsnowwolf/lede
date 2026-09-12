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


#ifndef __gc_hal_kernel_hardware_fe_h_
#define __gc_hal_kernel_hardware_fe_h_

#include "gc_hal.h"

/******************************************************************************/
/* Wait-Link FE commands. */

/* Construct Wait-Link FE. */
gceSTATUS
gckWLFE_Construct(gckHARDWARE Hardware, gckWLFE *FE);

void
gckWLFE_Destroy(gckHARDWARE Hardware, gckWLFE FE);

/* Initialize Wait-Link FE, when hardware reset. */
gceSTATUS
gckWLFE_Initialize(gckHARDWARE Hardware, gckWLFE FE);

/* Add a WAIT/LINK pair in the command queue. */
gceSTATUS
gckWLFE_WaitLink(gckHARDWARE Hardware,
                 gctPOINTER Logical,
                 gctADDRESS Address,
                 gctUINT32 Offset,
                 gctUINT32 *Bytes,
                 gctUINT32 *WaitOffset,
                 gctUINT32 *WaitBytes);

/* Add a LINK command in the command queue. */
gceSTATUS
gckWLFE_Link(gckHARDWARE Hardware,
             gctPOINTER Logical,
             gctADDRESS FetchAddress,
             gctUINT32 FetchSize,
             gctUINT32 *Bytes,
             gctUINT64 *Low,
             gctUINT64 *High);

/* Add an END command in the command queue. */
gceSTATUS
gckWLFE_EndEx(gckHARDWARE Hardware,
              gctPOINTER Logical,
              gctADDRESS Address,
              gctBOOL User,
              gctUINT32 *Bytes);

gceSTATUS
gckWLFE_End(gckHARDWARE Hardware,
            gctPOINTER Logical,
            gctADDRESS Address,
            gctUINT32 *Bytes);

/* Add a NOP command in the command queue. */
gceSTATUS
gckWLFE_Nop(gckHARDWARE Hardware,
            gctPOINTER Logical,
            gctSIZE_T *Bytes);

/* Add an EVENT command in the command queue. */
gceSTATUS
gckWLFE_Event(gckHARDWARE Hardware,
              gctPOINTER Logical,
              gctUINT8 Event,
              gceKERNEL_WHERE FromWhere,
              gctUINT32 *Bytes);

gceSTATUS
gckWLFE_ChipEnable(gckHARDWARE Hardware,
                   gctPOINTER Logical,
                   gceCORE_3D_MASK ChipEnable,
                   gctSIZE_T *Bytes);

/* Kickstart the command processor. */
gceSTATUS
gckWLFE_Execute(gckHARDWARE Hardware, gctADDRESS Address, gctUINT32 Bytes);

gceSTATUS
gckWLFE_InvalidatePipe(gckHARDWARE Hardware,
                       gctPOINTER Logical,
                       gctADDRESS Address,
                       gctUINT32 *Bytes);

void
gckWLFE_DoneInvalidatePipe(gckHARDWARE Hardware);

/******************************************************************************/
/* ASync FE commands. */

gceSTATUS
gckASYNC_FE_Construct(gckHARDWARE Hardware, gckASYNC_FE *FE);

void
gckASYNC_FE_Destroy(gckHARDWARE Hardware, gckASYNC_FE FE);

/* Initialize Async FE, when hardware reset. */
gceSTATUS
gckASYNC_FE_Initialize(gckHARDWARE Hardware, gckASYNC_FE FE);

/* Add a NOP command in the command queue. */
gceSTATUS
gckASYNC_FE_Nop(gckHARDWARE Hardware, gctPOINTER Logical, gctSIZE_T *Bytes);

/* Add an EVENT command in the command queue. */
gceSTATUS
gckASYNC_FE_Event(gckHARDWARE Hardware,
                  gctPOINTER Logical,
                  gctUINT8 Event,
                  gceKERNEL_WHERE FromWhere,
                  gctUINT32 *Bytes);

/* Kickstart the command processor. */
gceSTATUS
gckASYNC_FE_Execute(gckHARDWARE Hardware, gctADDRESS Address, gctUINT32 Bytes);

gceSTATUS
gckASYNC_FE_ReserveSlot(gckHARDWARE Hardware, gctBOOL *Available);

void
gckASYNC_FE_UpdateAvaiable(gckHARDWARE Hardware);

/******************************************************************************/
/* MC FE commands. */

/* One MCFE includes max 64 engine, each engine contains 2 channels. */
gceSTATUS
gckMCFE_Construct(gckHARDWARE Hardware, gckMCFE *FE);

void
gckMCFE_Destroy(gckHARDWARE Hardware, gckMCFE FE);

/* Initialize MC FE, when hardware reset. */
gceSTATUS
gckMCFE_Initialize(gckHARDWARE Hardware, gctBOOL MMUEnabled, gckMCFE FE);

/* Add a NOP command in the command queue. */
gceSTATUS
gckMCFE_Nop(gckHARDWARE Hardware, gctPOINTER Logical, gctSIZE_T *Bytes);

/* Add an EVENT command in the command queue. */
gceSTATUS
gckMCFE_Event(gckHARDWARE Hardware,
              gctPOINTER Logical,
              gctUINT8 Event,
              gceKERNEL_WHERE FromWhere,
              gctUINT32 *Bytes);

/* Add a SendSemaphore command in the command queue. */
gceSTATUS
gckMCFE_SendSemaphore(gckHARDWARE Hardware,
                      gctPOINTER Logical,
                      gctUINT32 SemaId,
                      gctUINT32 *Bytes);

/* Add a WaitSemaphore command in the command queue. */
gceSTATUS
gckMCFE_WaitSemaphore(gckHARDWARE Hardware,
                      gctPOINTER Logical,
                      gctUINT32 SemaId,
                      gctUINT32 *Bytes);

/* Kickstart the command processor. */
gceSTATUS
gckMCFE_Execute(gckHARDWARE Hardware,
                gctBOOL Priority,
                gctUINT32 ChannelId,
                gctADDRESS Address,
                gctUINT32 Bytes);

/* Query hardware module idle */
gceSTATUS
gckMCFE_HardwareIdle(gckHARDWARE Hardware, gctBOOL_PTR IsIdle);
#endif
