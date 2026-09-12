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


#ifndef __gc_hal_h_
#define __gc_hal_h_

#include "gc_hal_types.h"
#include "gc_hal_enum.h"
#include "gc_hal_base.h"
#include "gc_hal_profiler.h"
#include "gc_hal_driver.h"
#if gcdENABLE_3D
#include "gc_hal_statistics.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _gckVIDMEM          *gckVIDMEM;
typedef struct _gckKERNEL          *gckKERNEL;
typedef struct _gckCOMMAND         *gckCOMMAND;
typedef struct _gckEVENT           *gckEVENT;
typedef struct _gckDB              *gckDB;
typedef struct _gckDVFS            *gckDVFS;
typedef struct _gckMMU             *gckMMU;
typedef struct _gcsDEVICE          *gckDEVICE;

/******************************************************************************
 ****************************** Alignment Macros ******************************
 ******************************************************************************/

/* Alignment with a non-power of two value. */
#define gcmALIGN_NP2(n, align) (((n) + (align) - 1) - (((n) + (align) - 1) % (align)))

#define gcmALIGN_NP2_SAFE(n, align)                                         \
(                                                                           \
    (gcmALIGN_NP2((n) & ~0ULL, (align) & ~0ULL) ^ gcmALIGN_NP2(n, align)) ? \
        (n) : gcmALIGN_NP2(n, align)                                        \
)

/* Alignment with a power of two value. */
#define gcmALIGN(n, align) (((n) + ((align) - 1)) & ~((align) - 1))

#define gcmALIGN_SAFE(n, align)                                        \
(                                                                      \
    (gcmALIGN((n) & ~0ULL, (align) & ~0ULL) ^ gcmALIGN(n, align)) ?    \
         (n) : gcmALIGN(n, align)                                      \
)

#define gcmALIGN_CHECK_OVERFLOW(n, align)                              \
(                                                                      \
    (gcmALIGN((n) & ~0ULL, (align) & ~0ULL) ^ gcmALIGN(n, align)) ?    \
         gcvSTATUS_RESLUT_OVERFLOW : gcvSTATUS_OK                      \
)

#define gcmALIGN_BASE(n, align) \
( \
    ((n) & ~((align) - 1)) \
)

/******************************************************************************
 **************************** Element Count Macro *****************************
 ******************************************************************************/

#define gcmSIZEOF(a)    ((gctSIZE_T)(sizeof(a)))

#define gcmCOUNTOF(a)   (sizeof(a) / sizeof(a[0]))

/******************************************************************************
 ******************************** Cast Macro **********************************
 ******************************************************************************/
#define gcmNAME_TO_PTR(na) \
    gckKERNEL_QueryPointerFromName(kernel, gcmALL_TO_UINT32(na))

#define gcmPTR_TO_NAME(ptr) \
    gckKERNEL_AllocateNameFromPointer(kernel, ptr)

#define gcmRELEASE_NAME(na) \
    gckKERNEL_DeleteName(kernel, gcmALL_TO_UINT32(na))

#define gcmALL_TO_UINT32(t) ((gctUINT32)(gctUINTPTR_T)(t))

#define gcmPTR_TO_UINT64(p) ((gctUINT64)(gctUINTPTR_T)(p))

#define gcmUINT64_TO_PTR(u) ((gctPOINTER)(gctUINTPTR_T)(u))

#define gcmUINT64_TO_TYPE(u, t) ((t)(gctUINTPTR_T)(u))

/******************************************************************************
 ******************************* Useful Macro *********************************
 ******************************************************************************/

#define gcvINVALID_ADDRESS              ~0ULL
#define gcvINVALID_VALUE                0xCCCCCCCC

#define gcvINVALID_PHYSICAL_ADDRESS     ~0ULL

#define gcmGET_PRE_ROTATION(rotate) \
    ((rotate) & (~(gcvSURF_POST_FLIP_X | gcvSURF_POST_FLIP_Y)))

#define gcmGET_POST_ROTATION(rotate) \
    ((rotate) & (gcvSURF_POST_FLIP_X | gcvSURF_POST_FLIP_Y))

typedef struct _gckHARDWARE         *gckHARDWARE;

#define gcdMAX_GPU_COUNT            gcvCORE_COUNT

#define gcdMAX_SURF_LAYERS          4

#define gcdMAX_DRAW_BUFFERS         16

#define gcdMAX_3DGPU_COUNT          8

#define gcdMAX_VERTEX_STREAM_COUNT  4
/*******************************************************************************
 **
 **  gcmVERIFY_OBJECT
 **
 **      Assert if an object is invalid or is not of the specified type.  If the
 **      object is invalid or not of the specified type, gcvSTATUS_INVALID_OBJECT
 **      will be returned from the current function.  In retail mode this macro
 **      does nothing.
 **
 **  ARGUMENTS:
 **
 **      obj     Object to test.
 **      t       Expected type of the object.
 */
#if gcmIS_DEBUG(gcdDEBUG_TRACE)
#define _gcmVERIFY_OBJECT(prefix, obj, t) \
    if ((obj) == gcvNULL) \
    { \
        prefix##TRACE(gcvLEVEL_ERROR, \
                      #prefix "VERIFY_OBJECT failed: NULL"); \
        prefix##TRACE(gcvLEVEL_ERROR, "  expected: %c%c%c%c", \
                      gcmCC_PRINT(t)); \
        prefix##ASSERT((obj) != gcvNULL); \
        prefix##FOOTER_ARG("status=%d", gcvSTATUS_INVALID_OBJECT); \
        return gcvSTATUS_INVALID_OBJECT; \
    } \
    else if (((gcsOBJECT*) (obj))->type != t) \
    { \
        prefix##TRACE(gcvLEVEL_ERROR, \
                      #prefix "VERIFY_OBJECT failed: %c%c%c%c", \
                      gcmCC_PRINT(((gcsOBJECT*) (obj))->type)); \
        prefix##TRACE(gcvLEVEL_ERROR, "  expected: %c%c%c%c", \
                      gcmCC_PRINT(t)); \
        prefix##ASSERT(((gcsOBJECT*)(obj))->type == t); \
        prefix##FOOTER_ARG("status=%d", gcvSTATUS_INVALID_OBJECT); \
        return gcvSTATUS_INVALID_OBJECT; \
    }

#define gcmVERIFY_OBJECT(obj, t)        _gcmVERIFY_OBJECT(gcm, obj, t)
#define gcmkVERIFY_OBJECT(obj, t)       _gcmVERIFY_OBJECT(gcmk, obj, t)
#else
#define gcmVERIFY_OBJECT(obj, t)        do {} while (gcvFALSE)
#define gcmkVERIFY_OBJECT(obj, t)       do {} while (gcvFALSE)
#endif

/******************************************************************************/
/*VERIFY_OBJECT if special return expected*/
/******************************************************************************/
#ifndef EGL_API_ANDROID
# define _gcmVERIFY_OBJECT_RETURN(prefix, obj, t, retVal)                    \
    do {                                                                     \
        if ((obj) == gcvNULL) {                                              \
            prefix##PRINT_VERSION();                                         \
            prefix##TRACE(gcvLEVEL_ERROR,                                    \
                          #prefix "VERIFY_OBJECT_RETURN failed: NULL");      \
            prefix##TRACE(gcvLEVEL_ERROR, "  expected: %c%c%c%c",            \
                          gcmCC_PRINT(t));                                   \
            prefix##ASSERT((obj) != gcvNULL);                                \
            prefix##FOOTER_ARG("retVal=%d", retVal);                         \
            return retVal;                                                   \
        } else if (((gcsOBJECT *)(obj))->type != t) {                        \
            prefix##PRINT_VERSION();                                         \
            prefix##TRACE(gcvLEVEL_ERROR,                                    \
                          #prefix "VERIFY_OBJECT_RETURN failed: %c%c%c%c",   \
                          gcmCC_PRINT(((gcsOBJECT *)(obj))->type));          \
            prefix##TRACE(gcvLEVEL_ERROR, "  expected: %c%c%c%c",            \
                          gcmCC_PRINT(t));                                   \
            prefix##ASSERT(((gcsOBJECT *)(obj))->type == t);                 \
            prefix##FOOTER_ARG("retVal=%d", retVal);                         \
            return retVal;                                                   \
        }                                                                    \
    } while (gcvFALSE)
# define gcmVERIFY_OBJECT_RETURN(obj, t, retVal)  \
    _gcmVERIFY_OBJECT_RETURN(gcm, obj, t, retVal)
# define gcmkVERIFY_OBJECT_RETURN(obj, t, retVal) \
    _gcmVERIFY_OBJECT_RETURN(gcmk, obj, t, retVal)
#else
# define gcmVERIFY_OBJECT_RETURN(obj, t)      do {} while (gcvFALSE)
# define gcmkVERIFY_OBJECT_RETURN(obj, t)     do {} while (gcvFALSE)
#endif

/******************************************************************************
 ********************************* gckOS Object *******************************
 ******************************************************************************/

/* Construct a new gckOS object. */
gceSTATUS
gckOS_Construct(IN gctPOINTER Context, OUT gckOS *Os);

/* Destroy an gckOS object. */
gceSTATUS
gckOS_Destroy(IN gckOS Os);

/* Query the video memory. */
gceSTATUS
gckOS_QueryVideoMemory(IN gckOS Os,
                       OUT gctPHYS_ADDR *InternalAddress,
                       OUT gctSIZE_T *InternalSize,
                       OUT gctPHYS_ADDR *ExternalAddress,
                       OUT gctSIZE_T *ExternalSize,
                       OUT gctPHYS_ADDR *ContiguousAddress,
                       OUT gctSIZE_T *ContiguousSize);

/* Allocate memory from the heap. */
gceSTATUS
gckOS_Allocate(IN gckOS Os, IN gctSIZE_T Bytes, OUT gctPOINTER *Memory);

/* Free allocated memory. */
gceSTATUS
gckOS_Free(IN gckOS Os, IN gctPOINTER Memory);

/* Wrapper for allocation memory.. */
gceSTATUS
gckOS_AllocateMemory(IN gckOS Os, IN gctSIZE_T Bytes, OUT gctPOINTER *Memory);

/* Wrapper for freeing memory. */
gceSTATUS
gckOS_FreeMemory(IN gckOS Os, IN gctPOINTER Memory);

/* Allocate paged memory. */
gceSTATUS
gckOS_AllocatePagedMemory(IN gckOS Os,
                          IN gckKERNEL Kernel,
                          IN gctUINT32 Flag,
                          IN gceVIDMEM_TYPE Type,
                          IN OUT gctSIZE_T *Bytes,
                          OUT gctUINT32 *Gid,
                          OUT gctPHYS_ADDR *Physical);

/* Lock pages. */
gceSTATUS
gckOS_LockPages(IN gckOS Os,
                IN gctPHYS_ADDR Physical,
                IN gctSIZE_T Bytes,
                IN gctBOOL Cacheable,
                OUT gctPOINTER *Logical);

/* Map pages. */
gceSTATUS
gckOS_MapPagesEx(IN gckOS Os,
                 IN gckKERNEL Kernel,
                 IN gckMMU Mmu,
                 IN gctPHYS_ADDR Physical,
                 IN gctSIZE_T Offset,
                 IN gctSIZE_T PageCount,
                 IN gctADDRESS Address,
                 IN gctPOINTER PageTable,
                 IN gctBOOL Writable,
                 IN gceVIDMEM_TYPE Type);

/* Map 1M pages. */
gceSTATUS
gckOS_Map1MPages(IN gckOS Os,
                 IN gckKERNEL Kernel,
                 IN gckMMU Mmu,
                 IN gctPHYS_ADDR Physical,
                 IN gctSIZE_T PageCount,
                 IN gctADDRESS Address,
                 IN gctPOINTER PageTable,
                 IN gctBOOL Writable,
                 IN gceVIDMEM_TYPE Type);

gceSTATUS
gckOS_UnmapPages(IN gckOS Os, IN gctSIZE_T PageCount, IN gctADDRESS Address);

/* Unlock pages. */
gceSTATUS
gckOS_UnlockPages(IN gckOS Os, IN gctPHYS_ADDR Physical,
                  IN gctSIZE_T Bytes, IN gctPOINTER Logical);

/* Free paged memory. */
gceSTATUS
gckOS_FreePagedMemory(IN gckOS Os, IN gctPHYS_ADDR Physical, IN gctSIZE_T Bytes);

/* Allocate non-paged memory. */
gceSTATUS
gckOS_AllocateNonPagedMemory(IN gckOS Os,
                             IN gckKERNEL Kernel,
                             IN gctBOOL InUserSpace,
                             IN gctUINT32 Flag,
                             IN OUT gctSIZE_T *Bytes,
                             OUT gctPHYS_ADDR *Physical,
                             OUT gctPOINTER *Logical);

/* Free non-paged memory. */
gceSTATUS
gckOS_FreeNonPagedMemory(IN gckOS Os,
                         IN gctPHYS_ADDR Physical,
                         IN gctPOINTER Logical,
                         IN gctSIZE_T Bytes);

/* Reserved memory. */
gceSTATUS
gckOS_RequestReservedMemory(gckOS Os,
                            gctPHYS_ADDR_T Start,
                            gctSIZE_T Size,
                            const char *Name,
                            gctBOOL Requested,
                            gctPOINTER *MemoryHandle);

void
gckOS_ReleaseReservedMemory(gckOS Os, gctPOINTER MemoryHandle);

/* Reserved memory sub area */
gceSTATUS
gckOS_RequestReservedMemoryArea(IN gckOS Os,
                                IN gctPOINTER MemoryHandle,
                                IN gctSIZE_T Offset,
                                IN gctSIZE_T Size,
                                OUT gctPOINTER *MemoryAreaHandle);

void
gckOS_ReleaseReservedMemoryArea(gctPOINTER MemoryAreaHandle);

/* Get the number fo bytes per page. */
gceSTATUS
gckOS_GetPageSize(IN gckOS Os, OUT gctSIZE_T *PageSize);

/* Get the physical address of a corresponding logical address. */
gceSTATUS
gckOS_GetPhysicalAddress(IN gckOS Os, IN gctPOINTER Logical,
                         OUT gctPHYS_ADDR_T *Address);

/* Get real physical address from handle. */
gceSTATUS
gckOS_GetPhysicalFromHandle(IN gckOS Os,
                            IN gctPHYS_ADDR Physical,
                            IN gctSIZE_T Offset,
                            OUT gctPHYS_ADDR_T *PhysicalAddress);

/* Get the physical address of a corresponding user logical address. */
gceSTATUS
gckOS_UserLogicalToPhysical(IN gckOS Os, IN gctPOINTER Logical,
                            OUT gctPHYS_ADDR_T *Address);

/* Map physical memory. */
gceSTATUS
gckOS_MapPhysical(IN gckOS Os,
                  IN gctPHYS_ADDR_T Physical,
                  IN gctSIZE_T Bytes,
                  OUT gctPOINTER *Logical);

/* Unmap previously mapped physical memory. */
gceSTATUS
gckOS_UnmapPhysical(IN gckOS Os, IN gctPOINTER Logical, IN gctSIZE_T Bytes);

/* Read data from a hardware register. */
gceSTATUS
gckOS_ReadRegister(IN gckOS Os, IN gctUINT32 Address, OUT gctUINT32 *Data);

/* Read data from a hardware register. */
gceSTATUS
gckOS_ReadRegisterEx(IN gckOS Os,
                     IN gckKERNEL Kernel,
                     IN gctUINT32 Address,
                     OUT gctUINT32 *Data);

/* Write data to a hardware register. */
gceSTATUS
gckOS_WriteRegister(IN gckOS Os, IN gctUINT32 Address, IN gctUINT32 Data);

/* Write data to a hardware register. */
gceSTATUS
gckOS_WriteRegisterEx(IN gckOS Os,
                      IN gckKERNEL Kernel,
                      IN gctUINT32 Address,
                      IN gctUINT32 Data);

/* Write data to a hardware register without dump. */
gceSTATUS
gckOS_WriteRegisterEx_NoDump(IN gckOS Os,
                             IN gckKERNEL Kernel,
                             IN gctUINT32 Address,
                             IN gctUINT32 Data);

#ifdef __QNXNTO__
static gcmINLINE gceSTATUS
gckOS_WriteMemory(IN gckOS Os, IN gctPOINTER Address, IN gctUINT32 Data)
{
    /* Write memory. */
    *(gctUINT32 *)Address = Data;
    return gcvSTATUS_OK;
}

#else
/* Write data to a 32-bit memory location. */
gceSTATUS
gckOS_WriteMemory(IN gckOS Os, IN gctPOINTER Address, IN gctUINT32 Data);
#endif

/* Map physical memory into the process space. */
gceSTATUS
gckOS_MapMemory(IN gckOS Os, IN gctPHYS_ADDR Physical,
                IN gctSIZE_T Bytes, OUT gctPOINTER *Logical);

/* Unmap physical memory from the specified process space. */
gceSTATUS
gckOS_UnmapMemoryEx(IN gckOS Os,
                    IN gctPHYS_ADDR Physical,
                    IN gctSIZE_T Bytes,
                    IN gctPOINTER Logical,
                    IN gctUINT32 PID);

/* Unmap physical memory from the process space. */
gceSTATUS
gckOS_UnmapMemory(IN gckOS Os, IN gctPHYS_ADDR Physical,
                  IN gctSIZE_T Bytes, IN gctPOINTER Logical);

/* Delete a mutex. */
gceSTATUS
gckOS_DeleteMutex(IN gckOS Os, IN gctPOINTER Mutex);

/* Acquire a mutex. */
gceSTATUS
gckOS_AcquireMutex(IN gckOS Os, IN gctPOINTER Mutex, IN gctUINT32 Timeout);

/* Release a mutex. */
gceSTATUS
gckOS_ReleaseMutex(IN gckOS Os, IN gctPOINTER Mutex);

/* Atomically exchange a pair of 32-bit values. */
gceSTATUS
gckOS_AtomicExchange(IN gckOS Os,
                     IN OUT gctUINT32_PTR Target,
                     IN gctUINT32 NewValue,
                     OUT gctUINT32_PTR OldValue);

/* Atomically exchange a pair of pointers. */
gceSTATUS
gckOS_AtomicExchangePtr(IN gckOS Os,
                        IN OUT gctPOINTER *Target,
                        IN gctPOINTER NewValue,
                        OUT gctPOINTER *OldValue);

gceSTATUS
gckOS_AtomSetMask(IN gctPOINTER Atom, IN gctUINT32 Mask);

gceSTATUS
gckOS_AtomClearMask(IN gctPOINTER Atom, IN gctUINT32 Mask);

gceSTATUS
gckOS_DumpCallStack(IN gckOS Os);

gceSTATUS
gckOS_GetProcessNameByPid(IN gctINT Pid, IN gctSIZE_T Length, OUT gctUINT8_PTR String);

gceSTATUS
gckOS_QueryCPUFrequency(IN gckOS Os, IN gctUINT32 CPUId, OUT gctUINT32 *Frequency);

gceSTATUS
gckOS_TraceGpuMemory(IN gckOS Os, IN gctINT32 ProcessID, IN gctINT64 Delta);

void
gckOS_NodeIdAssign(gckOS Os, gcuVIDMEM_NODE_PTR Node);

#if gcdENABLE_CLEAR_FENCE
void
gckOS_ClearAllFence(gckDEVICE Device);

gctUINT64
gckOS_AllocFenceRecordId(IN gckDEVICE Device, IN gcsUSER_FENCE_INFO_PTR fence_info);

gcsUSER_FENCE_INFO_PTR
gckOS_ReleaseFenceRecordId(IN gckDEVICE Device, IN gctUINT64 recordId);

void
gckOS_PreLoadFenceRecId(IN gckDEVICE Device);

void
gckOS_PreLoadEndFenceRecId(IN gckDEVICE Device);

#endif
/*******************************************************************************
 **
 **  gckOS_AtomConstruct
 **
 **  Create an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **  OUTPUT:
 **
 **      gctPOINTER *Atom
 **          Pointer to a variable receiving the constructed atom.
 */
gceSTATUS
gckOS_AtomConstruct(IN gckOS Os, OUT gctPOINTER *Atom);

/*******************************************************************************
 **
 **  gckOS_AtomDestroy
 **
 **  Destroy an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gctPOINTER Atom
 **          Pointer to the atom to destroy.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckOS_AtomDestroy(IN gckOS Os, OUT gctPOINTER Atom);

/*******************************************************************************
 **
 **  gckOS_AtomGet
 **
 **  Get the 32-bit value protected by an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gctPOINTER Atom
 **          Pointer to the atom.
 **
 **  OUTPUT:
 **
 **      gctINT32_PTR Value
 **          Pointer to a variable the receives the value of the atom.
 */
gceSTATUS
gckOS_AtomGet(IN gckOS Os, IN gctPOINTER Atom, OUT gctINT32_PTR Value);

/*******************************************************************************
 **
 **  gckOS_AtomSet
 **
 **  Set the 32-bit value protected by an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gctPOINTER Atom
 **          Pointer to the atom.
 **
 **      gctINT32 Value
 **          The value of the atom.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckOS_AtomSet(IN gckOS Os, IN gctPOINTER Atom, IN gctINT32 Value);

/*******************************************************************************
 **
 **  gckOS_AtomIncrement
 **
 **  Atomically increment the 32-bit integer value inside an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gctPOINTER Atom
 **          Pointer to the atom.
 **
 **  OUTPUT:
 **
 **      gctINT32_PTR Value
 **          Pointer to a variable the receives the original value of the atom.
 */
gceSTATUS
gckOS_AtomIncrement(IN gckOS Os, IN gctPOINTER Atom, OUT gctINT32_PTR Value);

/*******************************************************************************
 **
 **  gckOS_AtomDecrement
 **
 **  Atomically decrement the 32-bit integer value inside an atom.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gctPOINTER Atom
 **          Pointer to the atom.
 **
 **  OUTPUT:
 **
 **      gctINT32_PTR Value
 **          Pointer to a variable the receives the original value of the atom.
 */
gceSTATUS
gckOS_AtomDecrement(IN gckOS Os, IN gctPOINTER Atom, OUT gctINT32_PTR Value);

/* Delay a number of milliseconds. */
gceSTATUS
gckOS_Delay(IN gckOS Os, IN gctUINT32 Delay);

/* Delay a number of milliseconds. */
gceSTATUS
gckOS_Udelay(IN gckOS Os, IN gctUINT32 Delay);

/* Get time in milliseconds. */
gceSTATUS
gckOS_GetTicks(OUT gctUINT32_PTR Time);

/* Compare time value. */
gceSTATUS
gckOS_TicksAfter(IN gctUINT32 Time1, IN gctUINT32 Time2, OUT gctBOOL_PTR IsAfter);

/* Get time in microseconds. */
gceSTATUS
gckOS_GetTime(OUT gctUINT64_PTR Time);

/* Memory barrier. */
gceSTATUS
gckOS_MemoryBarrier(IN gckOS Os, IN gctPOINTER Address);

/* Map user pointer. */
gceSTATUS
gckOS_MapUserPointer(IN gckOS Os,
                     IN gctPOINTER Pointer,
                     IN gctSIZE_T Size,
                     OUT gctPOINTER *KernelPointer);

/* Unmap user pointer. */
gceSTATUS
gckOS_UnmapUserPointer(IN gckOS Os,
                       IN gctPOINTER Pointer,
                       IN gctSIZE_T Size,
                       IN gctPOINTER KernelPointer);

/*******************************************************************************
 **
 **  gckOS_QueryNeedCopy
 **
 **  Query whether the memory can be accessed or mapped directly or it has to be
 **  copied.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to an gckOS object.
 **
 **      gctUINT32 ProcessID
 **          Process ID of the current process.
 **
 **  OUTPUT:
 **
 **      gctBOOL_PTR NeedCopy
 **          Pointer to a boolean receiving gcvTRUE if the memory needs a copy or
 **          gcvFALSE if the memory can be accessed or mapped dircetly.
 */
gceSTATUS
gckOS_QueryNeedCopy(IN gckOS Os, IN gctUINT32 ProcessID, OUT gctBOOL_PTR NeedCopy);

/*******************************************************************************
 **
 **  gckOS_CopyFromUserData
 **
 **  Copy data from user to kernel memory.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to an gckOS object.
 **
 **      gctPOINTER KernelPointer
 **          Pointer to kernel memory.
 **
 **      gctPOINTER Pointer
 **          Pointer to user memory.
 **
 **      gctSIZE_T Size
 **          Number of bytes to copy.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckOS_CopyFromUserData(IN gckOS Os,
                       IN gctPOINTER KernelPointer,
                       IN gctPOINTER Pointer,
                       IN gctSIZE_T Size);

/*******************************************************************************
 **
 **  gckOS_CopyToUserData
 **
 **  Copy data from kernel to user memory.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to an gckOS object.
 **
 **      gctPOINTER KernelPointer
 **          Pointer to kernel memory.
 **
 **      gctPOINTER Pointer
 **          Pointer to user memory.
 **
 **      gctSIZE_T Size
 **          Number of bytes to copy.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckOS_CopyToUserData(IN gckOS Os,
                     IN gctPOINTER KernelPointer,
                     IN gctPOINTER Pointer,
                     IN gctSIZE_T Size);

gceSTATUS
gckOS_SuspendInterrupt(IN gckOS Os);

gceSTATUS
gckOS_SuspendInterruptEx(IN gckOS Os, IN gceCORE Core);

gceSTATUS
gckOS_ResumeInterrupt(IN gckOS Os);

gceSTATUS
gckOS_ResumeInterruptEx(IN gckOS Os, IN gceCORE Core);

/* Perform a memory copy. */
gceSTATUS
gckOS_MemCopy(IN gctPOINTER Destination,
              IN gctCONST_POINTER Source,
              IN gctSIZE_T Bytes);

/* Zero memory. */
gceSTATUS
gckOS_ZeroMemory(IN gctPOINTER Memory, IN gctSIZE_T Bytes);

/*******************************************************************************
 **
 **  gckOS_GetProcessID
 **
 **  Get current process ID.
 **
 **  INPUT:
 **
 **      Nothing.
 **
 **  OUTPUT:
 **
 **      gctUINT32_PTR ProcessID
 **          Pointer to the variable that receives the process ID.
 */
gceSTATUS
gckOS_GetProcessID(OUT gctUINT32_PTR ProcessID);

gceSTATUS
gckOS_GetCurrentProcessID(OUT gctUINT32_PTR ProcessID);

/*******************************************************************************
 **
 **  gckOS_GetThreadID
 **
 **  Get current thread ID.
 **
 **  INPUT:
 **
 **      Nothing.
 **
 **  OUTPUT:
 **
 **      gctUINT32_PTR ThreadID
 **          Pointer to the variable that receives the thread ID.
 */
gceSTATUS
gckOS_GetThreadID(OUT gctUINT32_PTR ThreadID);

/******************************************************************************
 ********************************* Signal Object ******************************
 ******************************************************************************/

/* Create a signal. */
gceSTATUS
gckOS_CreateSignal(IN gckOS Os, IN gctBOOL ManualReset, OUT gctSIGNAL *Signal);

/* Destroy a signal. */
gceSTATUS
gckOS_DestroySignal(IN gckOS Os, IN gctSIGNAL Signal);

/* Signal a signal. */
gceSTATUS
gckOS_Signal(IN gckOS Os, IN gctSIGNAL Signal, IN gctBOOL State);

/* Wait for a signal. */
gceSTATUS
gckOS_WaitSignal(IN gckOS Os, IN gctSIGNAL Signal,
                 IN gctBOOL Interruptable, IN gctUINT32 Wait);

#ifdef __QNXNTO__
gceSTATUS
gckOS_SignalPulse(IN gckOS Os, IN gctSIGNAL Signal);

gceSTATUS
gckOS_SignalPending(IN gckOS Os, IN gctSIGNAL Signal);
#endif

/* Map a user signal to the kernel space. */
gceSTATUS
gckOS_MapSignal(IN gckOS Os, IN gctSIGNAL Signal,
                IN gctHANDLE Process, OUT gctSIGNAL *MappedSignal);

/* Unmap a user signal */
gceSTATUS
gckOS_UnmapSignal(IN gckOS Os, IN gctSIGNAL Signal);

/* Get scatter-gather table from memory. */
gceSTATUS
gckOS_MemoryGetSGT(IN gckOS Os,
                   IN gctPHYS_ADDR Physical,
                   IN gctSIZE_T Offset,
                   IN gctSIZE_T Bytes,
                   OUT gctPOINTER *SGT);

/* Map a page range of memory to user space. */
gceSTATUS
gckOS_MemoryMmap(IN gckOS Os,
                 IN gctPHYS_ADDR Physical,
                 IN gctSIZE_T skipPages,
                 IN gctSIZE_T numPages,
                 INOUT gctPOINTER Vma);

/* Wrap a user memory to gctPHYS_ADDR. */
gceSTATUS
gckOS_WrapMemory(IN gckOS Os,
                 IN gckKERNEL Kernel,
                 IN gcsUSER_MEMORY_DESC_PTR Desc,
                 OUT gctSIZE_T *Bytes,
                 OUT gctPHYS_ADDR *Physical,
                 OUT gctBOOL *Contiguous,
                 OUT gctSIZE_T *PageCountCpu);

gceSTATUS
gckOS_GetPolicyID(IN gckOS Os,
                  IN gceVIDMEM_TYPE Type,
                  OUT gctUINT32_PTR PolicyID,
                  OUT gctUINT32_PTR AXIConfig);

#if gcdENABLE_MP_SWITCH
gceSTATUS
gckOS_SwitchCoreCount(IN gckOS Os, OUT gctUINT32 *Count);
#endif

#if defined(LINUX) && !defined(EMULATOR)
gceSTATUS
gckOS_FillMdlMetadata(gckOS Os, gckVIDMEM_NODE nodeObj, gctPHYS_ADDR Physical);

gceSTATUS
gckOS_SetMetadata(gckOS Os, gckVIDMEM_NODE nodeObj, gctPHYS_ADDR Physical);
#endif

/******************************************************************************
 ************************* Android Native Fence Sync **************************
 ******************************************************************************/
gceSTATUS
gckOS_CreateSyncTimeline(IN gckOS Os, IN gceCORE Core, OUT gctHANDLE *Timeline);

gceSTATUS
gckOS_DestroySyncTimeline(IN gckOS Os, IN gctHANDLE Timeline);

gceSTATUS
gckOS_CreateNativeFence(IN gckOS Os,
                        IN gctHANDLE Timeline,
                        IN gctSIGNAL Signal,
                        OUT gctINT *FenceFD);

gceSTATUS
gckOS_WaitNativeFence(IN gckOS Os, IN gctHANDLE Timeline,
                      IN gctINT FenceFD, IN gctUINT32 Timeout);

#if !USE_NEW_LINUX_SIGNAL
/* Create signal to be used in the user space. */
gceSTATUS
gckOS_CreateUserSignal(IN gckOS Os, IN gctBOOL ManualReset, OUT gctINT *SignalID);

/* Destroy signal used in the user space. */
gceSTATUS
gckOS_DestroyUserSignal(IN gckOS Os, IN gctINT SignalID);

/* Wait for signal used in the user space. */
gceSTATUS
gckOS_WaitUserSignal(IN gckOS Os,
                     IN gctINT SignalID,
                     IN gctUINT32 Wait,
                     OUT gceSIGNAL_STATUS *SignalStatus);

/* Signal a signal used in the user space. */
gceSTATUS
gckOS_SignalUserSignal(IN gckOS Os, IN gctINT SignalID, IN gctBOOL State);
#endif /* USE_NEW_LINUX_SIGNAL */

/* Set a signal owned by a process. */
#if defined(__QNXNTO__)
gceSTATUS
gckOS_UserSignal(IN gckOS Os, IN gctSIGNAL Signal,
                 IN gctINT Rcvid, IN const struct sigevent *Event);
#else
gceSTATUS
gckOS_UserSignal(IN gckOS Os, IN gctSIGNAL Signal, IN gctHANDLE Handle);
#endif

/******************************************************************************\
 ** Cache Support
 */

gceSTATUS
gckOS_CacheClean(gckOS Os,
                 gctUINT32 ProcessID,
                 gctPHYS_ADDR Handle,
                 gctSIZE_T Offset,
                 gctPOINTER Logical,
                 gctSIZE_T Bytes);

gceSTATUS
gckOS_CacheFlush(gckOS Os,
                 gctUINT32 ProcessID,
                 gctPHYS_ADDR Handle,
                 gctSIZE_T Offset,
                 gctPOINTER Logical,
                 gctSIZE_T Bytes);

gceSTATUS
gckOS_CacheInvalidate(gckOS Os,
                      gctUINT32 ProcessID,
                      gctPHYS_ADDR Handle,
                      gctSIZE_T Offset,
                      gctPOINTER Logical,
                      gctSIZE_T Bytes);

gceSTATUS
gckOS_CPUPhysicalToGPUPhysical(IN gckOS Os,
                               IN gctPHYS_ADDR_T CPUPhysical,
                               IN gctPHYS_ADDR_T *GPUPhysical);

gceSTATUS
gckOS_GPUPhysicalToCPUPhysical(IN gckOS Os,
                               IN gctPHYS_ADDR_T GPUPhysical,
                               IN gctPHYS_ADDR_T *CPUPhysical);

gceSTATUS
gckOS_QueryPCIInfo(IN gckOS Os,
                   IN gctUINT32 PlatformIndex,
                   OUT gctUINT32 *Domain,
                   OUT gctUINT32 *Bus,
                   OUT gctUINT32 *Slot,
                   OUT gctUINT32 *Function);

gceSTATUS
gckOS_QueryOption(IN gckOS Os, IN gctCONST_STRING Option, OUT gctUINT64 *Value);

/******************************************************************************\
 ** Debug Support
 */

void
gckOS_SetDebugLevel(IN gctUINT32 Level);

void
gckOS_SetDebugZone(IN gctUINT32 Zone);

void
gckOS_SetDebugLevelZone(IN gctUINT32 Level, IN gctUINT32 Zone);

void
gckOS_SetDebugZones(IN gctUINT32 Zones, IN gctBOOL Enable);

void
gckOS_SetDebugFile(IN gctCONST_STRING FileName);

gceSTATUS
gckOS_Broadcast(IN gckOS Os, IN gckHARDWARE Hardware, IN gceBROADCAST Reason);

gceSTATUS
gckOS_BroadcastHurry(IN gckOS Os, IN gckHARDWARE Hardware, IN gctUINT Urgency);

gceSTATUS
gckOS_BroadcastCalibrateSpeed(IN gckOS Os,
                              IN gckHARDWARE Hardware,
                              IN gctUINT Idle,
                              IN gctUINT Time);

/*******************************************************************************
 **
 **  gckOS_SetGPUPower
 **
 **  Set the power of the GPU on or off.
 **
 **  INPUT:
 **
 **      gckOS Os
 **          Pointer to a gckOS object.
 **
 **      gckKERNEL Kernel
 **          Core whose power is set.
 **
 **      gctBOOL Clock
 **          gcvTRUE to turn on the clock, or gcvFALSE to turn off the clock.
 **
 **      gctBOOL Power
 **          gcvTRUE to turn on the power, or gcvFALSE to turn off the power.
 **
 **      gctUINT32 ClusterMask
 **          ClusterMask to power on
 **
 **  OUTPUT:
 **
 **      Nothing.
 */

gceSTATUS
gckOS_SetGPUPowerEx(IN gckOS Os,
                    IN gckKERNEL Kernel,
                    IN gctBOOL Clock,
                    IN gctBOOL Power,
                    IN gctUINT32 ClusterMask);

gceSTATUS
gckOS_SetGPUPower(IN gckOS Os,
                  IN gckKERNEL Kernel,
                  IN gctBOOL Clock,
                  IN gctBOOL Power);

gceSTATUS
gckOS_SetClockState(IN gckOS Os, IN gckKERNEL Kernel, IN gctBOOL Clock);

gceSTATUS
gckOS_GetClockState(IN gckOS Os, IN gckKERNEL Kernel, IN gctBOOL *Clock);

gceSTATUS
gckOS_ResetGPU(IN gckOS Os, IN gckKERNEL Kernel);

gceSTATUS
gckOS_PrepareGPUFrequency(IN gckOS Os, IN gceCORE Core);

gceSTATUS
gckOS_FinishGPUFrequency(IN gckOS Os, IN gceCORE Core);

gceSTATUS
gckOS_QueryGPUFrequency(IN gckOS Os,
                        IN gckHARDWARE Hardware,
                        OUT gctUINT32 *Frequency,
                        OUT gctUINT8 *Scale);

gceSTATUS
gckOS_SetGPUFrequency(IN gckOS Os, IN gckHARDWARE Hardware, IN gctUINT8 Scale);

/*******************************************************************************
 ** Semaphores.
 */

/* Create a new semaphore. */
gceSTATUS
gckOS_CreateSemaphore(IN gckOS Os, OUT gctPOINTER *Semaphore);

gceSTATUS
gckOS_CreateSemaphoreEx(IN gckOS Os, OUT gctPOINTER *Semaphore);

/* Delete a semaphore. */
gceSTATUS
gckOS_DestroySemaphore(IN gckOS Os, IN gctPOINTER Semaphore);

/* Acquire a semaphore. */
gceSTATUS
gckOS_AcquireSemaphore(IN gckOS Os, IN gctPOINTER Semaphore);

/* Try to acquire a semaphore. */
gceSTATUS
gckOS_TryAcquireSemaphore(IN gckOS Os, IN gctPOINTER Semaphore);

/* Release a semaphore. */
gceSTATUS
gckOS_ReleaseSemaphore(IN gckOS Os, IN gctPOINTER Semaphore);

/* Release a semaphore. */
gceSTATUS
gckOS_ReleaseSemaphoreEx(IN gckOS Os, IN gctPOINTER Semaphore);

/*******************************************************************************
 ** Timer API.
 */

typedef void (*gctTIMERFUNCTION)(gctPOINTER);

/* Create a timer. */
gceSTATUS
gckOS_CreateTimer(IN gckOS Os,
                  IN gctTIMERFUNCTION Function,
                  IN gctPOINTER Data,
                  OUT gctPOINTER *Timer);

/* Destroy a timer. */
gceSTATUS
gckOS_DestroyTimer(IN gckOS Os, IN gctPOINTER Timer);

/* Start a timer. */
gceSTATUS
gckOS_StartTimer(IN gckOS Os, IN gctPOINTER Timer, IN gctUINT32 Delay);

/* Stop a timer. */
gceSTATUS
gckOS_StopTimer(IN gckOS Os, IN gctPOINTER Timer);

/******************************************************************************
 ******************************** gckHEAP Object ******************************
 ******************************************************************************/

typedef struct _gckHEAP *gckHEAP;

/* Construct a new gckHEAP object. */
gceSTATUS
gckHEAP_Construct(IN gckOS Os, IN gctSIZE_T AllocationSize, OUT gckHEAP *Heap);

/* Destroy an gckHEAP object. */
gceSTATUS
gckHEAP_Destroy(IN gckHEAP Heap);

/* Allocate memory. */
gceSTATUS
gckHEAP_Allocate(IN gckHEAP Heap, IN gctSIZE_T Bytes, OUT gctPOINTER *Node);

/* Free memory. */
gceSTATUS
gckHEAP_Free(IN gckHEAP Heap, IN gctPOINTER Node);

/* Profile the heap. */
gceSTATUS
gckHEAP_ProfileStart(IN gckHEAP Heap);

gceSTATUS
gckHEAP_ProfileEnd(IN gckHEAP Heap, IN gctCONST_STRING Title);

/******************************************************************************
 ******************************* gckKERNEL Object *****************************
 ******************************************************************************/

struct _gcsHAL_INTERFACE;

/* Construct a new gckKERNEL object. */
gceSTATUS
gckKERNEL_Construct(IN gckOS Os,
                    IN gceCORE Core,
                    IN gctUINT ChipID,
                    IN gctPOINTER Context,
                    IN gckDEVICE Device,
                    IN gckDB SharedDB,
                    OUT gckKERNEL *Kernel);

/* Destroy an gckKERNEL object. */
gceSTATUS
gckKERNEL_Destroy(IN gckKERNEL Kernel);

/* Dispatch a user-level command. */
gceSTATUS
gckKERNEL_Dispatch(IN gckKERNEL Kernel,
                   IN gckDEVICE Device,
                   IN OUT struct _gcsHAL_INTERFACE *Interface);

/* Query Database requirements. */
gceSTATUS
gckKERNEL_QueryDatabase(IN gckKERNEL Kernel,
                        IN gctUINT32 ProcessID,
                        IN OUT gcsHAL_INTERFACE *Interface);

/* Query the video memory. */
gceSTATUS
gckKERNEL_QueryVideoMemory(IN gckKERNEL Kernel,
                           OUT struct _gcsHAL_INTERFACE *Interface);

/* Lookup the gckVIDMEM object for a pool. */
gceSTATUS
gckKERNEL_GetVideoMemoryPool(IN gckKERNEL Kernel, IN OUT gcePOOL *Pool,
                             OUT gckVIDMEM *VideoMemory);

/* Map dedicated video memory node. */
gceSTATUS
gckKERNEL_MapVideoMemory(IN gckKERNEL Kernel,
                         IN gctBOOL InUserSpace,
                         IN gcePOOL Pool,
                         IN gctPHYS_ADDR Physical,
                         IN gctSIZE_T Offset,
                         IN gctSIZE_T Bytes,
                         OUT gctPOINTER *Logical);

/* Unmap dedicated video memory. */
gceSTATUS
gckKERNEL_UnmapVideoMemory(IN gckKERNEL Kernel,
                           IN gcePOOL Pool,
                           IN gctPHYS_ADDR Physical,
                           IN gctPOINTER Logical,
                           IN gctUINT32 Pid,
                           IN gctSIZE_T Bytes);

/* Map memory. */
gceSTATUS
gckKERNEL_MapMemory(IN gckKERNEL Kernel,
                    IN gctPHYS_ADDR Physical,
                    IN gctSIZE_T Bytes,
                    OUT gctPOINTER *Logical);

/* Unmap memory. */
gceSTATUS
gckKERNEL_UnmapMemory(IN gckKERNEL Kernel,
                      IN gctPHYS_ADDR Physical,
                      IN gctSIZE_T Bytes,
                      IN gctPOINTER Logical,
                      IN gctUINT32 ProcessID);
/* Destroy reserved mem when destroy process*/
gceSTATUS
gckKERNEL_DestroyProcessReservedUserMap(IN gckKERNEL Kernel, IN gctUINT32 Pid);

/* Notification of events. */
gceSTATUS
gckKERNEL_Notify(IN gckKERNEL Kernel, IN gceNOTIFY Notifcation);

#if gcdENABLE_VIDEO_MEMORY_MIRROR
gceSTATUS
gckKERNEL_SyncVideoMemoryMirror(gckKERNEL Kernel,
                                gckVIDMEM_NODE Node,
                                gctSIZE_T Offset,
                                gctSIZE_T Bytes,
                                gctUINT32 Reason);
#endif
/*******************************************************************************
 **
 **  gckKERNEL_Recovery
 **
 **  Try to recover the GPU from a fatal error.
 **
 **  INPUT:
 **
 **      gckKERNEL Kernel
 **          Pointer to an gckKERNEL object.
 **
 **  OUTPUT:
 **
 **      Nothing.
 */
gceSTATUS
gckKERNEL_Recovery(IN gckKERNEL Kernel);

/* Get access to the user data. */
gceSTATUS
gckKERNEL_OpenUserData(IN gckKERNEL Kernel,
                       IN gctBOOL NeedCopy,
                       IN gctPOINTER StaticStorage,
                       IN gctPOINTER UserPointer,
                       IN gctSIZE_T Size,
                       OUT gctPOINTER *KernelPointer);

/* Release resources associated with the user data connection. */
gceSTATUS
gckKERNEL_CloseUserData(IN gckKERNEL Kernel,
                        IN gctBOOL NeedCopy,
                        IN gctBOOL FlushData,
                        IN gctPOINTER UserPointer,
                        IN gctSIZE_T Size,
                        OUT gctPOINTER *KernelPointer);

/* Query kernel by core index */
gceSTATUS
gckOS_QueryKernel(IN gckKERNEL Kernel, IN gctINT index, OUT gckKERNEL *KernelOut);

gceSTATUS
gckDVFS_Construct(IN gckHARDWARE Hardware, OUT gckDVFS *Frequency);

gceSTATUS
gckDVFS_Destroy(IN gckDVFS Dvfs);

gceSTATUS
gckDVFS_Start(IN gckDVFS Dvfs);

gceSTATUS
gckDVFS_Stop(IN gckDVFS Dvfs);

/******************************************************************************
 ******************************* gckHARDWARE Object ***************************
 ******************************************************************************/

/* Construct a new gckHARDWARE object. */
gceSTATUS
gckHARDWARE_Construct(IN gckOS Os,
                      IN gckKERNEL Kernel,
                      OUT gckHARDWARE *Hardware);

/* Post hardware resource allocation after gckHARDWARE object constructed. */
gceSTATUS
gckHARDWARE_PostConstruct(IN gckHARDWARE Hardware);

/* Pre-destroy hardwre resource before destroying an gckHARDWARE object. */
gceSTATUS
gckHARDWARE_PreDestroy(IN gckHARDWARE Hardware);

/* Destroy an gckHARDWARE object. */
gceSTATUS
gckHARDWARE_Destroy(IN gckHARDWARE Hardware);

/* Get hardware type. */
gceSTATUS
gckHARDWARE_GetType(IN gckHARDWARE Hardware, OUT gceHARDWARE_TYPE *Type);

/* Query system memory requirements. */
gceSTATUS
gckHARDWARE_QuerySystemMemory(IN gckHARDWARE Hardware,
                              OUT gctSIZE_T *SystemSize,
                              OUT gctUINT32 *SystemBaseAddress);

/* Build virtual address. */
gceSTATUS
gckHARDWARE_BuildVirtualAddress(IN gckHARDWARE Hardware,
                                IN gctUINT32 Index,
                                IN gctUINT32 Offset,
                                OUT gctUINT32 *Address);

/* Query command buffer requirements. */
gceSTATUS
gckHARDWARE_QueryCommandBuffer(IN gckHARDWARE Hardware,
                               IN gceENGINE Engine,
                               OUT gctUINT32 *Alignment,
                               OUT gctUINT32 *ReservedHead,
                               OUT gctUINT32 *ReservedTail);

/* Add a PIPESELECT command in the command queue. */
gceSTATUS
gckHARDWARE_PipeSelect(IN gckHARDWARE Hardware,
                       IN gctPOINTER Logical,
                       IN gcePIPE_SELECT Pipe,
                       IN OUT gctUINT32  *Bytes);

/* Query the available memory. */
gceSTATUS
gckHARDWARE_QueryMemory(IN gckHARDWARE Hardware,
                        OUT gctSIZE_T *InternalSize,
                        OUT gctADDRESS *InternalBaseAddress,
                        OUT gctUINT32 *InternalAlignment,
                        OUT gctSIZE_T *ExternalSize,
                        OUT gctADDRESS *ExternalBaseAddress,
                        OUT gctUINT32 *ExternalAlignment,
                        OUT gctUINT32 *HorizontalTileSize,
                        OUT gctUINT32 *VerticalTileSize);

/* Query the identity of the hardware. */
gceSTATUS
gckHARDWARE_QueryChipIdentity(IN gckHARDWARE Hardware,
                              OUT gcsHAL_QUERY_CHIP_IDENTITY_PTR Identity);

gceSTATUS
gckHARDWARE_QueryChipOptions(IN gckHARDWARE Hardware,
                             OUT gcsHAL_QUERY_CHIP_OPTIONS_PTR Options);

/* Split a harwdare specific address into API stuff. */
gceSTATUS
gckHARDWARE_SplitMemory(IN gckHARDWARE Hardware,
                        IN gctUINT32 Address,
                        OUT gcePOOL *Pool,
                        OUT gctUINT32 *Offset);

/* Update command queue tail pointer. */
gceSTATUS
gckHARDWARE_UpdateQueueTail(IN gckHARDWARE Hardware,
                            IN gctPOINTER Logical,
                            IN gctUINT32 Offset);

/* Interrupt manager. */
gceSTATUS
gckHARDWARE_Interrupt(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_Notify(IN gckHARDWARE Hardware);

/* Program MMU. */
gceSTATUS
gckHARDWARE_SetMMU(IN gckHARDWARE Hardware, IN gckMMU Mmu);

/* Flush the MMU. */
gceSTATUS
gckHARDWARE_FlushMMU(IN gckHARDWARE Hardware,
                     IN gctPOINTER Logical,
                     IN gctADDRESS Address,
                     IN gctUINT32 SubsequentBytes,
                     IN OUT gctUINT32 *Bytes);

gceSTATUS
gckHARDWARE_FlushAsyncMMU(IN gckHARDWARE Hardware,
                          IN gctPOINTER Logical,
                          IN OUT gctUINT32 *Bytes);

gceSTATUS
gckHARDWARE_FlushMcfeMMU(IN gckHARDWARE Hardware,
                         IN gctPOINTER Logical,
                         IN OUT gctUINT32 *Bytes);

/* Get idle register. */
gceSTATUS
gckHARDWARE_GetIdle(IN gckHARDWARE Hardware, IN gctBOOL Wait, OUT gctUINT32 *Data);

/* Flush the caches. */
gceSTATUS
gckHARDWARE_Flush(IN gckHARDWARE Hardware,
                  IN gceKERNEL_FLUSH Flush,
                  IN gctPOINTER Logical,
                  IN OUT gctUINT32 *Bytes);

/* Enable/disable fast clear. */
gceSTATUS
gckHARDWARE_SetFastClear(IN gckHARDWARE Hardware,
                         IN gctINT Enable,
                         IN gctINT Compression);

gceSTATUS
gckHARDWARE_ReadInterrupt(IN gckHARDWARE Hardware, OUT gctUINT32_PTR IDs);

/*
 * State timer helper.
 */
gceSTATUS
gckHARDWARE_StartTimerReset(IN gckHARDWARE Hardware);

/* Power management. */
gceSTATUS
gckHARDWARE_SetPowerState(IN gckHARDWARE Hardware, IN gceCHIPPOWERSTATE State);

gceSTATUS
gckHARDWARE_QueryPowerStateUnlocked(IN gckHARDWARE Hardware, OUT gceCHIPPOWERSTATE *State);

gceSTATUS
gckHARDWARE_QueryPowerState(IN gckHARDWARE Hardware, OUT gceCHIPPOWERSTATE *State);

gceSTATUS
gckHARDWARE_EnablePowerManagement(IN gckHARDWARE Hardware, IN gctBOOL Enable);

gceSTATUS
gckHARDWARE_QueryPowerManagement(IN gckHARDWARE Hardware, OUT gctBOOL *Enable);

gceSTATUS
gckHARDWARE_SetGpuProfiler(IN gckHARDWARE Hardware, IN gctBOOL GpuProfiler);

#if gcdENABLE_FSCALE_VAL_ADJUST
gceSTATUS
gckHARDWARE_SetFscaleValue(IN gckHARDWARE Hardware,
                           IN gctUINT32 FscaleValue,
                           IN gctUINT32 ShaderFscaleValue);

gceSTATUS
gckHARDWARE_GetFscaleValue(IN gckHARDWARE Hardware,
                           IN gctUINT *FscaleValue,
                           IN gctUINT *MinFscaleValue,
                           IN gctUINT *MaxFscaleValue);

gceSTATUS
gckHARDWARE_SetMinFscaleValue(IN gckHARDWARE Hardware, IN gctUINT MinFscaleValue);
#endif

gceSTATUS
gckHARDWARE_InitializeHardware(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_Reset(IN gckHARDWARE Hardware, IN gctBOOL ForceSoftReset);

/* Check for Hardware features. */
gceSTATUS
gckHARDWARE_IsFeatureAvailable(IN gckHARDWARE Hardware, IN gceFEATURE Feature);

gceSTATUS
gckHARDWARE_DumpMMUException(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_DumpGPUState(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_ReadAxiStatusRegister(IN gckHARDWARE Hardware, OUT gctUINT32_PTR Value);

gceSTATUS
gckHARDWARE_InitDVFS(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_QueryLoad(IN gckHARDWARE Hardware, OUT gctUINT32 *Load);

gceSTATUS
gckHARDWARE_SetDVFSPeroid(IN gckHARDWARE Hardware, IN gctUINT32 Frequency);

gceSTATUS
gckHARDWARE_QueryStateTimer(IN gckHARDWARE Hardware,
                            OUT gctUINT64_PTR On,
                            OUT gctUINT64_PTR Off,
                            OUT gctUINT64_PTR Idle,
                            OUT gctUINT64_PTR Suspend);

gceSTATUS
gckHARDWARE_Fence(IN gckHARDWARE Hardware,
                  IN gceENGINE Engine,
                  IN gctPOINTER Logical,
                  IN gctADDRESS FenceAddress,
                  IN gctUINT64 FenceData,
                  IN OUT gctUINT32 *Bytes);

/******************************************************************************
 ******************************** gckMMU Object *******************************
 ******************************************************************************/

/* Construct a new gckMMU object. */
gceSTATUS
gckMMU_Construct(IN gckKERNEL Kernel, OUT gckMMU *Mmu);

/* Destroy an gckMMU object. */
gceSTATUS
gckMMU_Destroy(IN gckMMU Mmu);

/* Allocate pages inside the MMU. */
gceSTATUS
gckMMU_AllocatePages(IN gckMMU Mmu,
                     IN gctSIZE_T PageCount,
                     IN gcePAGE_TYPE PageType,
                     OUT gctPOINTER *PageTable,
                     OUT gctADDRESS *Address);

gceSTATUS
gckMMU_AllocatePagesEx(IN gckMMU Mmu,
                       IN gctSIZE_T PageCount,
                       IN gceVIDMEM_TYPE Type,
                       IN gcePAGE_TYPE PageType,
                       IN gctBOOL LowVA,
                       IN gctBOOL Secure,
                       OUT gctPOINTER *PageTable,
                       OUT gctADDRESS *Address);

/* Remove a page table from the MMU. */
gceSTATUS
gckMMU_FreePages(IN gckMMU Mmu,
                 IN gctBOOL Secure,
                 IN gcePAGE_TYPE PageType,
                 IN gctBOOL LowVA,
                 IN gctADDRESS Address,
                 IN gctPOINTER PageTable,
                 IN gctSIZE_T PageCount);

/* Set the MMU page with info. */
gceSTATUS
gckMMU_SetPage(IN gckMMU Mmu,
               IN gctPHYS_ADDR_T PageAddress,
               IN gcePAGE_TYPE PageType,
               IN gctBOOL LowVA,
               IN gctBOOL Writable,
               IN gctPOINTER PageEntry);

gceSTATUS
gckMMU_Flush(IN gckMMU Mmu, IN gceVIDMEM_TYPE Type);

gceSTATUS
gckMMU_DumpPageTableEntry(IN gckMMU Mmu, IN gceAREA_TYPE AreaType, IN gctADDRESS Address);

gceSTATUS
gckMMU_FillFlatMapping(IN gckMMU Mmu,
                       IN gctUINT64 PhysBase,
                       IN gctSIZE_T Size,
                       IN gctBOOL Reserved,
                       IN gctBOOL AbleToShift,
                       OUT gctADDRESS *GpuBaseAddress);

gceSTATUS
gckMMU_IsFlatMapped(IN gckMMU Mmu,
                    IN gctUINT64 Physical,
                    IN gctSIZE_T Bytes,
                    OUT gctBOOL *In,
                    INOUT gctADDRESS *Address);

gceSTATUS
gckMMU_GetAreaType(IN gckMMU Mmu, IN gctADDRESS GpuAddress, OUT gceAREA_TYPE *AreaType);

gceSTATUS
gckHARDWARE_QueryContextProfile(IN gckHARDWARE Hardware,
                                IN gctBOOL Reset,
                                OUT gcsPROFILER_COUNTERS *Counters);

gceSTATUS
gckHARDWARE_UpdateContextProfile(IN gckHARDWARE Hardware);

gceSTATUS
gckHARDWARE_InitProfiler(IN gckHARDWARE Hardware);

gceSTATUS
gckOS_DetectProcessByName(IN gctCONST_POINTER Name);

void
gckOS_DumpParam(void);

gceSTATUS
gc_mmuinfo_show(void);

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_h_ */
