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


#ifndef __gc_hal_kernel_allocator_h_
#define __gc_hal_kernel_allocator_h_

#include "gc_hal_kernel_linux.h"
#include <linux/slab.h>
#include <linux/mm_types.h>

typedef struct _gcsALLOCATOR  *gckALLOCATOR;
typedef union _gcsATTACH_DESC *gcsATTACH_DESC_PTR;

typedef struct _gcsALLOCATOR_OPERATIONS {
    /**************************************************************************
     **
     ** Alloc
     **
     ** Allocate memory, request size is page aligned.
     **
     ** INPUT:
     **
     **    gckALLOCATOR Allocator
     **        Pointer to an gckALLOCATOER object.
     **
     **    PLINUX_Mdl
     **        Pointer to Mdl whichs stores information
     **        about allocated memory.
     **
     **    gctSIZE_T NumPages
     **        Number of pages need to allocate.
     **
     **    gctUINT32 Flag
     **        Allocation option.
     **
     ** OUTPUT:
     **
     **      Nothing.
     **
     */
    gceSTATUS
    (*Alloc)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T NumPages, gctUINT32 Flag);

    /**************************************************************************
     **
     ** Free
     **
     ** Free memory.
     **
     ** INPUT:
     **
     **     gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **     PLINUX_MDL Mdl
     **          Mdl which stores information.
     **
     ** OUTPUT:
     **
     **      Nothing.
     **
     */
    void (*Free)(gckALLOCATOR Allocator, PLINUX_MDL Mdl);

    /**************************************************************************
     **
     ** Mmap
     **
     ** Map a page range of the memory to user space.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl.
     **
     **      gctSIZE_T skipPages
     **          Number of page to be skipped from beginning of this memory.
     **
     **      gctSIZE_T numPages
     **          Number of pages to be mapped from skipPages.
     **
     ** INOUT:
     **
     **      struct vm_area_struct *vma
     **          Pointer to VMM memory area.
     **
     */
    gceSTATUS
    (*Mmap)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctBOOL Cacheable,
            gctSIZE_T skipPages, gctSIZE_T numPages, struct vm_area_struct *vma);

    /**************************************************************************
     **
     ** MapUser
     **
     ** Map memory to user space.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl.
     **
     **      gctBOOL Cacheable
     **          Whether this mapping is cacheable.
     **
     ** OUTPUT:
     **
     **      gctPOINTER *UserLogical
     **          Pointer to user logical address.
     **
     **      Nothing.
     **
     */
    gceSTATUS
    (*MapUser)(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
               PLINUX_MDL_MAP MdlMap, gctBOOL Cacheable);

    /**************************************************************************
     **
     ** UnmapUser
     **
     ** Unmap address from user address space.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      gctPOINTER Logical
     **          Address to be unmap
     **
     **      gctUINT32 Size
     **          Size of address space
     **
     ** OUTPUT:
     **
     **      Nothing.
     **
     */
    void
    (*UnmapUser)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, PLINUX_MDL_MAP MdlMap, gctUINT32 Size);

    /**************************************************************************
     **
     ** MapKernel
     **
     ** Map memory to kernel space.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl object.
     **
     ** OUTPUT:
     **      gctPOINTER *Logical
     **          Mapped kernel address.
     */
    gceSTATUS
    (*MapKernel)(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
                 gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER * Logical);

    /**************************************************************************
     **
     ** UnmapKernel
     **
     ** Unmap memory from kernel space.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl object.
     **
     **      gctPOINTER Logical
     **          Mapped kernel address.
     **
     ** OUTPUT:
     **
     **      Nothing.
     **
     */
    gceSTATUS
    (*UnmapKernel)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctPOINTER Logical);

    /**************************************************************************
     **
     ** Cache
     **
     ** Maintain cache coherency.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl object.
     **
     **      gctPOINTER Logical
     **          Logical address, could be user address or kernel address
     **
     **      gctSIZE_T Offset
     **          Physical address.
     **
     **      gctUINT32 Bytes
     **          Size of memory region.
     **
     **      gceCACHEOPERATION Opertaion
     **          Cache operation.
     **
     ** OUTPUT:
     **
     **      Nothing.
     **
     */
    gceSTATUS
    (*Cache)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, gctSIZE_T Offset,
             gctPOINTER Logical, gctSIZE_T Bytes, gceCACHEOPERATION Operation);

    /**************************************************************************
     **
     ** Physical
     **
     ** Get physical address from a offset in memory region.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      PLINUX_MDL Mdl
     **          Pointer to a Mdl object.
     **
     **      unsigned long Offset
     **          Offset in this memory region.
     **
     ** OUTPUT:
     **      gctUINT32_PTR Physical
     **          Physical address.
     **
     */
    gceSTATUS
    (*Physical)(gckALLOCATOR Allocator, PLINUX_MDL Mdl, unsigned long Offset, gctPHYS_ADDR_T * Physical);

    /**************************************************************************
     **
     ** Attach
     **
     ** Import memory allocated by an external allocator.
     **
     ** INPUT:
     **      gckALLOCATOR Allocator
     **          Pointer to an gckALLOCATOER object.
     **
     **      gctUINT32 Handle
     **          Handle of the memory.
     **
     ** OUTPUT:
     **      None.
     **
     */
    gceSTATUS
    (*Attach)(gckALLOCATOR Allocator, gcsATTACH_DESC_PTR Desc, PLINUX_MDL Mdl);

    /**************************************************************************
     *
     * GetSGT
     *
     * Get scatter-gather table from a range of the memory.
     *
     * INPUT:
     *      gckALLOCATOR Allocator
     *          Pointer to an gckALLOCATOER object.
     *
     *      gctUINT32 Handle
     *          The Handle of the memory.
     *
     *      gctSIZE_T Offset
     *          The offset to the beginning of this mdl.
     *
     *      gctSIZE_T Bytes
     *          Total bytes form Offset.
     *
     * OUTPUT:
     *      gctPOINTER *SGT
     *          scatter-gather table
     *
     */
    gceSTATUS
    (*GetSGT)(gckALLOCATOR Allocator, PLINUX_MDL Mdl,
              gctSIZE_T Offset, gctSIZE_T Bytes, gctPOINTER * SGT);
} gcsALLOCATOR_OPERATIONS;

typedef struct _gcsALLOCATOR {
    /* Pointer to gckOS Object. */
    gckOS                   os;

    /* Name. */
    gctSTRING               name;

    /* Operations. */
    gcsALLOCATOR_OPERATIONS *ops;

    /* Capability of this allocator. */
    gctUINT32               capability;

    /* Debugfs entry of this allocator. */
    gcsDEBUGFS_DIR          debugfsDir;

    /* Private data used by customer allocator. */
    void                    *privateData;

    /* Allocator destructor. */
    void                    (*destructor)(struct _gcsALLOCATOR *Allocator);

    struct list_head        link;
#if gcdANON_FILE_FOR_ALLOCATOR
    /*Anonymous file for map to user. */
    struct file             *anon_file;
#endif
} gcsALLOCATOR;

typedef struct _gcsALLOCATOR_DESC {
    /* Name of a allocator. */
    char    *name;

    /* Entry function to construct a allocator. */
    gceSTATUS
    (*construct)(gckOS, gcsDEBUGFS_DIR *, gckALLOCATOR *);
} gcsALLOCATOR_DESC;

typedef union _gcsATTACH_DESC {
    /* gcvALLOC_FLAG_DMABUF */
    struct {
        gctPOINTER          dmabuf;
    } dmaBuf;

    /* gcvALLOC_FLAG_USERMEMORY */
    struct {
        gctPOINTER          memory;
        gctPHYS_ADDR_T      physical;
        gctSIZE_T           size;
    } userMem;

    /* Reserved memory. */
    struct {
        unsigned long       start;
        unsigned long       size;
        const char          *name;
        int                 requested;
        gctBOOL             root;
    } reservedMem;
} gcsATTACH_DESC;

/*
 * Helpers
 */

/* Fill a gcsALLOCATOR_DESC structure. */
#define gcmkDEFINE_ALLOCATOR_DESC(Name, Construct)  \
    {                                               \
        .name = Name,                               \
        .construct = Construct,                     \
    }

/* Construct a allocator. */
static inline gceSTATUS
gckALLOCATOR_Construct(gckOS Os,
                       gcsALLOCATOR_OPERATIONS *Operations,
                       gckALLOCATOR *Allocator)
{
    gceSTATUS status;
    gckALLOCATOR allocator;

    gcmkASSERT(Allocator != gcvNULL);
    gcmkASSERT(Operations &&
               (Operations->Alloc || Operations->Attach) &&
               (Operations->Free) &&
               Operations->MapUser &&
               Operations->UnmapUser &&
               Operations->MapKernel &&
               Operations->UnmapKernel &&
               Operations->Cache &&
               Operations->Physical);

    allocator = kzalloc(sizeof(gcsALLOCATOR), GFP_KERNEL | gcdNOWARN);
    if (unlikely(!allocator))
        gcmkONERROR(gcvSTATUS_OUT_OF_MEMORY);

    /* Record os. */
    allocator->os = Os;

    /* Set operations. */
    allocator->ops = Operations;

    *Allocator = allocator;

    return gcvSTATUS_OK;

OnError:
    return status;
}

#if !gcdUSE_LINUX_SG_TABLE_API
int
alloc_sg_list_from_pages(struct scatterlist **sgl, struct page **pages, unsigned int n_pages,
                         unsigned long offset, unsigned long size, unsigned int *nents);
#endif

gceSTATUS
_DmaAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator);

gceSTATUS
_UserMemoryAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator);

gceSTATUS
_GFPAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator);

gceSTATUS
_ReservedMemoryAllocatorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator);

gceSTATUS
_DmabufAlloctorInit(gckOS Os, gcsDEBUGFS_DIR *Parent, gckALLOCATOR *Allocator);

/*
 *   How to implement customer allocator
 *
 *   Build in customer alloctor
 *
 *       It is recommended that customer allocator is implmented in independent
 *       source file(s) which is specified by CUSOMTER_ALLOCATOR_OBJS in Kbuld.
 *
 *   Register gcsALLOCATOR
 *
 *       For each customer specified allocator, a desciption entry must be added
 *       to allocatorArray defined in gc_hal_kernel_allocator_array.h.
 *
 *       An entry in allocatorArray is a gcsALLOCATOR_DESC structure which describes
 *       name and constructor of a gckALLOCATOR object.
 *
 *   Implement gcsALLOCATOR_DESC.init()
 *
 *       In gcsALLOCATOR_DESC.init(), gckALLOCATOR_Construct should be called
 *       to create a gckALLOCATOR object, customer specified private data can
 *       be put in gcsALLOCATOR.privateData.
 *
 *   Implement gcsALLOCATOR_OPERATIONS
 *
 *       When call gckALLOCATOR_Construct to create a gckALLOCATOR object, a
 *       gcsALLOCATOR_OPERATIONS structure must be provided whose all members
 *       implemented.
 *
 */
#endif /* __gc_hal_kernel_allocator_h_ */
