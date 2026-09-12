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


#ifndef __gc_hal_types_shared_h_
#define __gc_hal_types_shared_h_

#if !defined(GC_KMD)
#if defined(__KERNEL__)
#  include "linux/version.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
typedef unsigned long uintptr_t;
#  endif
#  include "linux/types.h"
# elif defined(UNDER_CE)
#  include <crtdefs.h>
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;
# elif defined(_MSC_VER) && (_MSC_VER <= 1500)
#  include <crtdefs.h>
#  include "vadefs.h"
# elif defined(__QNXNTO__)
#  define _QNX_SOURCE
#  include <stdint.h>
#  include <stddef.h>
# else
#  include <stdlib.h>
#  include <stddef.h>
#  include <stdint.h>
# endif
#endif

#ifdef _WIN32
# pragma warning(disable : 4127) /* Conditional expression is constant (do { } while(0)). */
# pragma warning(disable : 4100) /* Unreferenced formal parameter. */
# pragma warning(disable : 4204) /* Non-constant aggregate initializer (C99). */
# pragma warning(disable : 4131) /* Uses old-style declarator. */
# pragma warning(disable : 4206) /* Translation unit is empty. */
# pragma warning(disable : 4214) /* Nonstandard extension used :
                                  * bit field types other than int.
                                  */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************
 *********************************** Keyword **********************************
 ******************************************************************************/

#if defined(ANDROID) && defined(__BIONIC_FORTIFY)
#if defined(__clang__)
#if (__clang_major__ >= 10)
#   define gcmINLINE           __inline__ __attribute__ ((always_inline))
#  else
#   define gcmINLINE           __inline__ __attribute__ ((always_inline)) __attribute__ ((gnu_inline))
#  endif
# else
#  define gcmINLINE            __inline__ __attribute__ ((always_inline)) __attribute__ ((gnu_inline)) __attribute__ ((artificial))
# endif
#elif ((defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 199901L)) || defined(__APPLE__))
# define gcmINLINE            inline      /* C99 keyword. */
#elif defined(__GNUC__)
# define gcmINLINE            __inline__  /* GNU keyword. */
#elif defined(_MSC_VER) || defined(UNDER_CE)
# define gcmINLINE            __inline    /* Internal keyword. */
#else
# error "gcmINLINE: Platform could not be determined"
#endif

/* kernel layer keyword. */
#define gcmkINLINE            inline

/* kernel fall-through keyword. */
#if defined __has_attribute
#if __has_attribute(__fallthrough__)
#  define gcmkFALLTHRU                   __attribute__((__fallthrough__))
# else
#  define gcmkFALLTHRU                   do {} while (0)  /* fallthrough */
# endif
#else
# define gcmkFALLTHRU                    do {} while (0)  /* fallthrough */
#endif

/* Possible debug flags. */
#define gcdDEBUG_NONE           0
#define gcdDEBUG_ALL            (1 << 0)
#define gcdDEBUG_FATAL          (1 << 1)
#define gcdDEBUG_TRACE          (1 << 2)
#define gcdDEBUG_BREAK          (1 << 3)
#define gcdDEBUG_ASSERT         (1 << 4)
#define gcdDEBUG_CODE           (1 << 5)
#define gcdDEBUG_STACK          (1 << 6)

#define gcmIS_DEBUG(flag)       (gcdDEBUG & ((flag) | gcdDEBUG_ALL))

#ifndef gcdDEBUG
#if (defined(DBG) && DBG) || defined(DEBUG) || defined(_DEBUG)
#  define gcdDEBUG        gcdDEBUG_ALL
# else
#  define gcdDEBUG        gcdDEBUG_NONE
# endif
#endif

#ifdef _USRDLL
#ifdef _MSC_VER
#ifdef HAL_EXPORTS
#   define HALAPI      __declspec(dllexport)
#  else
#   define HALAPI      __declspec(dllimport)
#  endif
#  define HALDECL      __cdecl
# else
#ifdef HAL_EXPORTS
#   define HALAPI
#  else
#   define HALAPI      extern
#  endif
# endif
#else
# define HALAPI
# define HALDECL
#endif

/******************************************************************************
 ********************************* Common Types *******************************
 ******************************************************************************/

#define gcvFALSE                0
#define gcvTRUE                 1
#define gcvIGNORE               ((gctUINT32)~0U)

#define gcvINFINITE             ((gctUINT32)~0U)

#define gcvINVALID_HANDLE       ((gctHANDLE)gcmINT2PTR(~0U))

typedef int                     gctBOOL;
typedef gctBOOL                 *gctBOOL_PTR;

typedef int                     gctINT;
typedef signed char             gctINT8;
typedef signed short            gctINT16;
typedef signed int              gctINT32;
typedef signed long long        gctINT64;

typedef gctINT                  *gctINT_PTR;
typedef gctINT8                 *gctINT8_PTR;
typedef gctINT16                *gctINT16_PTR;
typedef gctINT32                *gctINT32_PTR;
typedef gctINT64                *gctINT64_PTR;

typedef unsigned int            gctUINT;
typedef unsigned char           gctUINT8;
typedef unsigned short          gctUINT16;
typedef unsigned int            gctUINT17;
typedef unsigned int            gctUINT32;
typedef unsigned long long      gctUINT64;
typedef uintptr_t               gctUINTPTR_T;
typedef ptrdiff_t               gctPTRDIFF_T;

typedef gctUINT                 *gctUINT_PTR;
typedef gctUINT8                *gctUINT8_PTR;
typedef gctUINT16               *gctUINT16_PTR;
typedef gctUINT32               *gctUINT32_PTR;
typedef gctUINT64               *gctUINT64_PTR;

typedef size_t                  gctSIZE_T;
typedef gctSIZE_T               *gctSIZE_T_PTR;
typedef gctUINT32               gctTRACE;

#ifdef __cplusplus
# define gcvNULL                0
#else
# define gcvNULL                ((void *)0)
#endif

#define gcvMAXINT8              0x7f
#define gcvMININT8              0x80
#define gcvMAXINT16             0x7fff
#define gcvMININT16             0x8000
#define gcvMAXINT32             0x7fffffff
#define gcvMININT32             0x80000000
#define gcvMAXINT64             0x7fffffffffffffff
#define gcvMININT64             0x8000000000000000
#define gcvMAXUINT8             0xff
#define gcvMINUINT8             0x0
#define gcvMAXUINT16            0xffff
#define gcvMINUINT16            0x0
#define gcvMAXUINT32            0xffffffff
#define gcvMINUINT32            0x0
#define gcvMAXUINT64            0xffffffffffffffff
#define gcvMINUINT64            0x0
#define gcvMAXUINTPTR_T         (~(gctUINTPTR_T)0)
#define gcvMAXSIZE_T            ((gctSIZE_T)(-1))

typedef float                   gctFLOAT;
typedef double                  gctDOUBLE;
typedef signed int              gctFIXED_POINT;
typedef float                   *gctFLOAT_PTR;
typedef double                  *gctDOUBLE_PTR;

typedef void                    *gctPHYS_ADDR;
typedef void                    *gctHANDLE;
typedef void                    *gctFILE;
typedef void                    *gctSIGNAL;
typedef void                    *gctWINDOW;
typedef void                    *gctIMAGE;
typedef void                    *gctSHBUF;

typedef void                    *gctSEMAPHORE;

typedef void                    *gctPOINTER;
typedef const void              *gctCONST_POINTER;

typedef char                    gctCHAR;
typedef signed char             gctSIGNED_CHAR;
typedef unsigned char           gctUNSIGNED_CHAR;
typedef char                    *gctSTRING;
typedef const char              *gctCONST_STRING;

typedef gctUINT64               gctPHYS_ADDR_T;
typedef gctUINT64               gctADDRESS;

typedef struct _gcsCOUNT_STRING {
    gctSIZE_T                   Length;
    gctCONST_STRING             String;
} gcsCOUNT_STRING;

typedef union _gcuFLOAT_UINT32 {
    gctFLOAT    f;
    gctUINT32   u;
} gcuFLOAT_UINT32;

/* Fixed point constants. */
#define gcvZERO_X               ((gctFIXED_POINT)0x00000000)
#define gcvHALF_X               ((gctFIXED_POINT)0x00008000)
#define gcvONE_X                ((gctFIXED_POINT)0x00010000)
#define gcvNEGONE_X             ((gctFIXED_POINT)0xFFFF0000)
#define gcvTWO_X                ((gctFIXED_POINT)0x00020000)

/* No special needs. */
#define gcvALLOC_FLAG_NONE                  0x00000000

/* Physical contiguous. */
#define gcvALLOC_FLAG_CONTIGUOUS            0x00000001
/* Physical non contiguous. */
#define gcvALLOC_FLAG_NON_CONTIGUOUS        0x00000002

/* Should not swap out. */
#define gcvALLOC_FLAG_NON_PAGED             0x00000004

/* CPU access explicitly needed. */
#define gcvALLOC_FLAG_CPU_ACCESS            0x00000008
/* Can be remapped as cacheable. */
#define gcvALLOC_FLAG_CACHEABLE             0x00000010

/* Need 32bit address. */
#define gcvALLOC_FLAG_4GB_ADDR              0x00000020

/* Secure buffer. */
#define gcvALLOC_FLAG_SECURITY              0x00000040
/* Can be exported as dmabuf-fd */
#define gcvALLOC_FLAG_DMABUF_EXPORTABLE     0x00000080
/* Do not try slow pools (gcvPOOL_VIRTUAL) */
#define gcvALLOC_FLAG_FAST_POOLS            0x00000100

/* Only accessed by GPU */
#define gcvALLOC_FLAG_NON_CPU_ACCESS        0x00000200
/* Do not be moved */
#define gcvALLOC_FLAG_NO_EVICT              0x00000400

/* Allocate from user space. */
#define gcvALLOC_FLAG_FROM_USER             0x00000800

/* Import DMABUF. */
#define gcvALLOC_FLAG_DMABUF                0x00001000
/* Import USERMEMORY. */
#define gcvALLOC_FLAG_USERMEMORY            0x00002000
/* Import an External Buffer, this flag is no longer used. */
#define gcvALLOC_FLAG_EXTERNAL_MEMORY       0x00004000
/* Import linux reserved memory. */
#define gcvALLOC_FLAG_LINUX_RESERVED_MEM    0x00008000

/* 1M pages unit allocation. */
#define gcvALLOC_FLAG_1M_PAGES              0x00010000

/* Non 1M pages unit allocation. */
#define gcvALLOC_FLAG_4K_PAGES              0x00020000

/* Lower 4G VA range. */
#define gcvALLOC_FLAG_32BIT_VA              0x00040000
#define gcvALLOC_FLAG_PRIOR_32BIT_VA        0x00080000

/* Dynamically allocate local memory pool memory. */
#define gcvALLOC_FLAG_DYNAMIC_ALLOC_LOCAL   0x00100000

/* GPU read only. */
#define gcvALLOC_FLAG_GPU_READ_ONLY         0x00200000

/* Real allocation happens when GPU page fault. */
#define gcvALLOC_FLAG_ALLOC_ON_FAULT        0x01000000
/* Alloc with memory limit. */
#define gcvALLOC_FLAG_MEMLIMIT              0x02000000
/* Alloc memory with mirror */
#define gcvALLOC_FLAG_WITH_MIRROR           0x04000000

#define gcmFIXEDCLAMP_NEG1_TO_1(_x)                                 \
    (((_x) < gcvNEGONE_X) ? gcvNEGONE_X :                           \
                            (((_x) > gcvONE_X) ? gcvONE_X : (_x)))

#define gcmFLOATCLAMP_NEG1_TO_1(_f)                                 \
    (((_f) < -1.0f) ? -1.0f : (((_f) > 1.0f) ? 1.0f : (_f)))

#define gcmFIXEDCLAMP_0_TO_1(_x)                                    \
    (((_x) < 0) ? 0 : (((_x) > gcvONE_X) ? gcvONE_X : (_x)))

#define gcmFLOATCLAMP_0_TO_1(_f)                                    \
    (((_f) < 0.0f) ? 0.0f : (((_f) > 1.0f) ? 1.0f : (_f)))

/******************************************************************************
 ****************************** Multicast Values ******************************
 ******************************************************************************/

/* Value unions. */
typedef union _gcuVALUE {
    gctUINT                     uintValue;
    gctFIXED_POINT              fixedValue;
    gctFLOAT                    floatValue;
    gctINT                      intValue;
} gcuVALUE;

/* Stringizing macro. */
#define gcmSTRING(Value)        #Value

/******************************************************************************
 ****************************** Fixed Point Math ******************************
 ******************************************************************************/

#define gcmXMultiply(x1, x2)            gcoMATH_MultiplyFixed(x1, x2)
#define gcmXDivide(x1, x2)              gcoMATH_DivideFixed(x1, x2)
#define gcmXMultiplyDivide(x1, x2, x3)  gcoMATH_MultiplyDivideFixed(x1, x2, x3)

/* 2D Engine profile. */
typedef struct _gcs2D_PROFILE {
    /* Cycle count.
     * 32bit counter incremented every 2D clock cycle.
     * Wraps back to 0 when the counter overflows.
     */
    gctUINT32       cycleCount;

    /* Pixels rendered by the 2D engine.
     * Resets to 0 every time it is read.
     */
    gctUINT32       pixelsRendered;
} gcs2D_PROFILE;

#define gcmPRINTABLE(c) ((((c) >= ' ') && ((c) <= '}')) ? ((c) != '%' ? (c) : ' ') : ' ')

#define gcmCC_PRINT(cc) \
    gcmPRINTABLE((char) ( (cc)        & 0xFF)), \
    gcmPRINTABLE((char) (((cc) >>  8) & 0xFF)), \
    gcmPRINTABLE((char) (((cc) >> 16) & 0xFF)), \
    gcmPRINTABLE((char) (((cc) >> 24) & 0xFF))

/******************************************************************************
 ***************************** Function Parameters ****************************
 ******************************************************************************/

#define IN
#define OUT
#define INOUT
#define OPTIONAL

/******************************************************************************
 ******************************** Status Macros *******************************
 ******************************************************************************/

#define gcmIS_ERROR(status)         ((status) < 0)
#define gcmNO_ERROR(status)         ((status) >= 0)
#define gcmIS_SUCCESS(status)       ((status) == gcvSTATUS_OK)
#define gcmERROR2PTR(err)           ((gctPOINTER)(err))
#define gcmPTR2ERROR(ptr)           ((gctINT64)(ptr))
/******************************************************************************
 ******************************** Field Macros ********************************
 ******************************************************************************/

#define __gcmSTART(reg_field)                        \
    (0 ? reg_field)

#define __gcmEND(reg_field)                          \
    (1 ? reg_field)

#define __gcmGETSIZE(reg_field)                      \
    (__gcmEND(reg_field) - __gcmSTART(reg_field) + 1)

#define __gcmALIGN(data, reg_field)                  \
    (((gctUINT32)(data)) << __gcmSTART(reg_field))

#define __gcmMASK(reg_field)                         \
    ((gctUINT32) ((__gcmGETSIZE(reg_field) == 32)    \
        ?  ~0U \
        : (~(~0U << __gcmGETSIZE(reg_field)))))


/*******************************************************************************
 **
 **  gcmFIELDMASK
 **
 **      Get aligned field mask.
 **
 **  ARGUMENTS:
 **
 **      reg     Name of register.
 **      field   Name of field within register.
 */
#define gcmFIELDMASK(reg, field)                     \
    (__gcmALIGN(__gcmMASK(reg##_##field), reg##_##field))

/*******************************************************************************
 **
 **  gcmGETFIELD
 **
 **      Extract the value of a field from specified data.
 **
 **  ARGUMENTS:
 **
 **      data    Data value.
 **      reg     Name of register.
 **      field   Name of field within register.
 */
#define gcmGETFIELD(data, reg, field)                       \
(                                                           \
    ((((gctUINT32)(data)) >> __gcmSTART(reg##_##field))     \
        & __gcmMASK(reg##_##field))                         \
)

/*******************************************************************************
 **
 **  gcmSETFIELD
 **
 **      Set the value of a field within specified data.
 **
 **  ARGUMENTS:
 **
 **      data    Data value.
 **      reg     Name of register.
 **      field   Name of field within register.
 **      value   Value for field.
 */
#define gcmSETFIELD(data, reg, field, value)                    \
(                                                               \
    (((gctUINT32)(data))                                       \
        & ~__gcmALIGN(__gcmMASK(reg##_##field), reg##_##field)) \
        |  __gcmALIGN((gctUINT32)(value)                       \
            & __gcmMASK(reg##_##field), reg##_##field)          \
)

/*******************************************************************************
 **
 **  gcmSETFIELDVALUE
 **
 **      Set the value of a field within specified data with a
 **      predefined value.
 **
 **  ARGUMENTS:
 **
 **      data    Data value.
 **      reg     Name of register.
 **      field   Name of field within register.
 **      value   Name of the value within the field.
 */
#define gcmSETFIELDVALUE(data, reg, field, value)               \
(                                                               \
    (((gctUINT32)(data))                                       \
        & ~__gcmALIGN(__gcmMASK(reg##_##field), reg##_##field)) \
        |  __gcmALIGN(reg##_##field##_##value                   \
            & __gcmMASK(reg##_##field), reg##_##field)          \
)

/*******************************************************************************
 **
 **  gcmGETMASKEDFIELDMASK
 **
 **      Determine field mask of a masked field.
 **
 **  ARGUMENTS:
 **
 **      reg     Name of register.
 **      field   Name of field within register.
 */
#define gcmGETMASKEDFIELDMASK(reg, field) \
( \
    gcmSETFIELD(0, reg,          field, ~0U) | \
    gcmSETFIELD(0, reg, MASK_ ## field, ~0U)   \
)

/*******************************************************************************
 **
 **  gcmSETMASKEDFIELD
 **
 **      Set the value of a masked field with specified data.
 **
 **  ARGUMENTS:
 **
 **      reg     Name of register.
 **      field   Name of field within register.
 **      value   Value for field.
 */
#define gcmSETMASKEDFIELD(reg, field, value) \
( \
    gcmSETFIELD     (~0U, reg,          field, value) & \
    gcmSETFIELDVALUE(~0U, reg, MASK_ ## field, ENABLED) \
)

/*******************************************************************************
 **
 **  gcmSETMASKEDFIELDVALUE
 **
 **      Set the value of a masked field with specified data.
 **
 **  ARGUMENTS:
 **
 **      reg     Name of register.
 **      field   Name of field within register.
 **      value   Value for field.
 */
#define gcmSETMASKEDFIELDVALUE(reg, field, value) \
( \
    gcmSETFIELDVALUE(~0U, reg,          field, value) & \
    gcmSETFIELDVALUE(~0U, reg, MASK_ ## field, ENABLED) \
)

/*******************************************************************************
 **
 **  gcmVERIFYFIELDVALUE
 **
 **      Verify if the value of a field within specified data equals a
 **      predefined value.
 **
 **  ARGUMENTS:
 **
 **      data    Data value.
 **      reg     Name of register.
 **      field   Name of field within register.
 **      value   Name of the value within the field.
 */
#define gcmVERIFYFIELDVALUE(data, reg, field, value)     \
(                                                        \
    (((gctUINT32)(data)) >> __gcmSTART(reg##_##field) &  \
                            __gcmMASK(reg##_##field))    \
        ==                                               \
    (reg##_##field##_##value & __gcmMASK(reg##_##field)) \
)

/*******************************************************************************
 **  Bit field macros.
 */

#define __gcmSTARTBIT(Field)                   \
    (1 ? Field)

#define __gcmBITSIZE(Field)                                                                        \
    (0 ? Field)

#define __gcmBITMASK(Field)                     \
(                                               \
    (1 << __gcmBITSIZE(Field)) - 1              \
)

#define gcmGETBITS(Value, Type, Field)          \
(                                               \
    (((Type)(Value)) >> __gcmSTARTBIT(Field))   \
    &                                           \
    __gcmBITMASK(Field)                         \
)

#define gcmSETBITS(Value, Type, Field, NewValue)                            \
(                                                                           \
    (((Type)(Value)) & ~(__gcmBITMASK(Field) << __gcmSTARTBIT(Field))) |    \
    ((((Type)(NewValue)) & __gcmBITMASK(Field)) << __gcmSTARTBIT(Field))    \
)

/*******************************************************************************
 **
 **  gcmISINREGRANGE
 **
 **      Verify whether the specified address is in the register range.
 **
 **  ARGUMENTS:
 **
 **      Address  the Address to be verified.
 **      Name     Name of a register.
 */

#define gcmISINREGRANGE(Address, Name)                          \
(                                                               \
    ((Address & (~0U << Name##_LSB)) == (Name##_Address >> 2))  \
)

/******************************************************************************
 ******************************* Ceiling Macro ********************************
 ******************************************************************************/
#define gcmCEIL(x)                                                      \
(                                                                       \
    ((x) - (gctUINT32)(x)) == 0 ? (gctUINT32)(x) : (gctUINT32)(x) + 1   \
)

/******************************************************************************
 ******************************* Min/Max Macros *******************************
 ******************************************************************************/

#define gcmMIN(x, y)            (((x) <= (y)) ? (x) : (y))
#define gcmMAX(x, y)            (((x) >= (y)) ? (x) : (y))
#define gcmCLAMP(x, min, max)   (((x) < (min)) ? (min) : ((x) > (max)) ? (max) : (x))
#define gcmABS(x)               (((x) < 0) ? -(x) : (x))
#define gcmNEG(x)               (((x) < 0) ? (x) : -(x))

/******************************************************************************
 ******************************* Bit Macro ************************************
 ******************************************************************************/
#define gcmBITSET(x, bit)       ((x) | (1 << (bit)))
#define gcmBITCLEAR(x, bit)     ((x) & ~(1 << (bit)))
#define gcmBITTEST(x, bit)      ((x) & (1 << (bit)))

/*******************************************************************************
 **
 **  gcmPTR2SIZE
 **
 **      Convert a pointer to an integer value.
 **
 **  ARGUMENTS:
 **
 **      p       Pointer value.
 */
#define gcmPTR2SIZE(p)      ((gctUINTPTR_T)(p))

#define gcmPTR2INT32(p)     ((gctUINT32)(gctUINTPTR_T)(p))

/*******************************************************************************
 **
 **  gcmINT2PTR
 **
 **      Convert an integer value into a pointer.
 **
 **  ARGUMENTS:
 **
 **      v       Integer value.
 */

#define gcmINT2PTR(i)       ((gctPOINTER)(gctUINTPTR_T)(i))

/*******************************************************************************
 **
 **  gcmOFFSETOF
 **
 **      Compute the byte offset of a field inside a structure.
 **
 **  ARGUMENTS:
 **
 **      s       Structure name.
 **      field   Field name.
 */
#define gcmOFFSETOF(s, field)       (gcmPTR2INT32(&(((struct s *)0)->field)))

#define __gcmOFFSETOF(type, field)  (gcmPTR2INT32(&(((type *)0)->field)))

/*******************************************************************************
 **
 **  gcmCONTAINEROF
 **
 **      Get containing structure of a member.
 **
 **  ARGUMENTS:
 **
 **      Pointer the Pointer of member.
 **      Type    Structure name.
 **      Name    Field name.
 */
#define gcmCONTAINEROF(Pointer, Type, Member)                         \
(                                                                     \
    (Type *)((gctUINTPTR_T)(Pointer) - __gcmOFFSETOF(Type, Member))   \
)

/*******************************************************************************
 **
 ** gcmBSWAP16/32/64
 **
 **      Return a value with all bytes in the 16/32/64 bit argument swapped.
 */
#if !defined(__KERNEL__) && defined(__GNUC__) &&                                \
    (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__ >= 40300) && \
    !defined(__VXWORKS__)
# define gcmBSWAP16(x)       __builtin_bswap16(x)
# define gcmBSWAP32(x)       __builtin_bswap32(x)
# define gcmBSWAP64(x)       __builtin_bswap64(x)
#else
# define gcmBSWAP16(x) ((gctUINT16)(                      \
    (((gctUINT16)(x) & (gctUINT16)0x00FF) << 8)  |        \
    (((gctUINT16)(x) & (gctUINT16)0xFF00) >> 8)))

# define gcmBSWAP32(x) ((gctUINT32)(                      \
    (((gctUINT32)(x) & (gctUINT32)0x000000FFU) << 24) |   \
    (((gctUINT32)(x) & (gctUINT32)0x0000FF00U) << 8)  |   \
    (((gctUINT32)(x) & (gctUINT32)0x00FF0000U) >> 8)  |   \
    (((gctUINT32)(x) & (gctUINT32)0xFF000000U) >> 24)))

# define gcmBSWAP64(x) ((gctUINT64)(                                \
    (((gctUINT64)(x) & (gctUINT64)0x00000000000000FFULL) << 56) |   \
    (((gctUINT64)(x) & (gctUINT64)0x000000000000FF00ULL) << 40) |   \
    (((gctUINT64)(x) & (gctUINT64)0x0000000000FF0000ULL) << 24) |   \
    (((gctUINT64)(x) & (gctUINT64)0x00000000FF000000ULL) << 8)  |   \
    (((gctUINT64)(x) & (gctUINT64)0x000000FF00000000ULL) >> 8)  |   \
    (((gctUINT64)(x) & (gctUINT64)0x0000FF0000000000ULL) >> 24) |   \
    (((gctUINT64)(x) & (gctUINT64)0x00FF000000000000ULL) >> 40) |   \
    (((gctUINT64)(x) & (gctUINT64)0xFF00000000000000ULL) >> 56)))
#endif

/*******************************************************************************
 **
 ** gcmBSWAP16IN32
 **
 **      Return a value with every 16 bit swapped of a 32 bit data type.
 */
#define gcmBSWAP16IN32(x) ((gctUINT32)(                 \
    (((gctUINT32)(x) & (gctUINT32)0x000000FFU) << 8)  | \
    (((gctUINT32)(x) & (gctUINT32)0x0000FF00U) >> 8)  | \
    (((gctUINT32)(x) & (gctUINT32)0x00FF0000U) << 8)  | \
    (((gctUINT32)(x) & (gctUINT32)0xFF000000U) >> 8)))

/*******************************************************************************
 **
 ** gcmBSWAP16IN32EX
 **
 **      Return a value with whole 16 bit swapped of a 32 bit data type.
 */
#define gcmBSWAP16IN32EX(x) ((gctUINT32)(                \
    (((gctUINT32)(x) & (gctUINT32)0x0000FFFFU) << 16)  | \
    (((gctUINT32)(x) & (gctUINT32)0xFFFF0000U) >> 16)))

/*******************************************************************************
 **
 ** gcmBSWAP32IN64
 **
 **      Return a value with whole 32 bit swapped of a 64 bit data type.
 */
#define gcmBSWAP32IN64(x) ((gctUINT64)(                           \
    (((gctUINT64)(x) & (gctUINT64)0x00000000FFFFFFFFULL) << 32) | \
    (((gctUINT64)(x) & (gctUINT64)0xFFFFFFFF00000000ULL) >> 32)))

/******************************************************************************
 **** Database ****************************************************************/

typedef struct _gcsDATABASE_COUNTERS {
    /* Number of currently allocated bytes. */
    gctUINT64                   bytes;

    /* Maximum number of bytes allocated (memory footprint). */
    gctUINT64                   maxBytes;

    /* Total number of bytes allocated. */
    gctUINT64                   totalBytes;

    /* The numbers of times video memory was allocated. */
    gctUINT32                   allocCount;

    /* The numbers of times video memory was freed. */
    gctUINT32                   freeCount;
} gcsDATABASE_COUNTERS;

typedef struct _gcuDATABASE_INFO {
    /* Counters. */
    gcsDATABASE_COUNTERS        counters;

    /* Time value. */
    gctUINT64                   time;
} gcuDATABASE_INFO;

/******************************************************************************
 **** Frame database **********************************************************/

/* gcsHAL_FRAME_INFO */
typedef struct _gcsHAL_FRAME_INFO {
    /* Current timer tick. */
    OUT gctUINT64               ticks;

    /* Bandwidth counters. */
    OUT gctUINT                 readBytes8[8];
    OUT gctUINT                 writeBytes8[8];

    /* Counters. */
    OUT gctUINT                 cycles[8];
    OUT gctUINT                 idleCycles[8];
    OUT gctUINT                 mcCycles[8];
    OUT gctUINT                 readRequests[8];
    OUT gctUINT                 writeRequests[8];

    /* 3D counters. */
    OUT gctUINT                 vertexCount;
    OUT gctUINT                 primitiveCount;
    OUT gctUINT                 rejectedPrimitives;
    OUT gctUINT                 culledPrimitives;
    OUT gctUINT                 clippedPrimitives;
    OUT gctUINT                 outPrimitives;
    OUT gctUINT                 inPrimitives;
    OUT gctUINT                 culledQuadCount;
    OUT gctUINT                 totalQuadCount;
    OUT gctUINT                 quadCount;
    OUT gctUINT                 totalPixelCount;

    /* PE counters. */
    OUT gctUINT                 colorKilled[8];
    OUT gctUINT                 colorDrawn[8];
    OUT gctUINT                 depthKilled[8];
    OUT gctUINT                 depthDrawn[8];

    /* Shader counters. */
    OUT gctUINT                 shaderCycles;
    OUT gctUINT                 vsInstructionCount;
    OUT gctUINT                 vsTextureCount;
    OUT gctUINT                 psInstructionCount;
    OUT gctUINT                 psTextureCount;

    /* Texture counters. */
    OUT gctUINT                 bilinearRequests;
    OUT gctUINT                 trilinearRequests;
    OUT gctUINT                 txBytes8;
    OUT gctUINT                 txHitCount;
    OUT gctUINT                 txMissCount;
} gcsHAL_FRAME_INFO;

/*
 * 'Patch' here means a mechanism to let kernel side modify user space reserved
 * command buffer location, or something the like, during the command buffer
 * commit.
 *
 * Reasons of using 'patch':
 * 1. Some resources/states are managed globally only in kernel side, such as
 *    MCFE semaphore, etc.
 * 2. For the sake of security or optimization, like video memory address.
 *
 * Patches are arranged in arrays, each array has the same type. The 'patchArray'
 * in 'gcsHAL_PATCH_LIST' pointers the concrete patch item array.
 *
 * NOTICE:
 * Be aware of the order and values! Tables in gc_hal_user_buffer.c and
 * gc_hal_kernel_command.c depend on this.
 */
/* The patch array. */
typedef struct _gcsHAL_PATCH_LIST {
    /* Patch type. */
    gctUINT32           type;

    /* Patch item count. */
    gctUINT32           count;

    /*
     * Pointer to the patch items.
     *
     * gcsHAL_PATCH_VIDMEM_ADDRESS * patchArray;
     * gcsHAL_PATCH_MCFE_SEMAPHORE * patchArray;
     * gcsHAL_PATCH_VIDMEM_TIMESTAMP * patchArray;
     * ...
     */
    gctUINT64           patchArray;

    /* struct _gcsHAL_PATCH_LIST * next; */
    gctUINT64           next;
} gcsHAL_PATCH_LIST;

/*
 * Patch a GPU address in the place (gcvHAL_PATCH_VIDMEM_ADDRESS).
 * Size of a GPU address is always 32 bits.
 */
typedef struct _gcsHAL_PATCH_VIDMEM_ADDRESS {
    /* Patch location in the command buffer. */
    gctUINT32           location;

    /* Handle of the video memory node. */
    gctUINT32           node;

    /* Address offset in the video memory node. */
    gctUINT32           offset;
} gcsHAL_PATCH_VIDMEM_ADDRESS;

/*
 * Patch a MCFE semaphore command in the place (gcvHAL_PATCH_MCFE_SEMAPHORE).
 * Size of the semaphore command is fixed at _64_ bits!
 */
typedef struct _gcsHAL_PATCH_MCFE_SEMAPHORE {
    /* Patch location in the command buffer. */
    gctUINT32           location;

    /* semaphore direction: 1 = Send, 0 = Wait. */
    gctUINT32           sendSema;

    /* Handle of the semaphore. */
    gctUINT32           semaHandle;
} gcsHAL_PATCH_MCFE_SEMAPHORE;

/*
 * Patch timestamp of given video memory node (gcvHAL_PATCH_VIDMEM_TIMESTAMP).
 * Pure software-wise, not command relevant.
 */
typedef struct _gcsHAL_PATCH_VIDMEM_TIMESTAMP {
    /* Handle of a video memory node. */
    gctUINT32           handle;

    gctUINT32           flag;
} gcsHAL_PATCH_VIDMEM_TIMESTAMP;

/* Put together patch list handling variables. */
typedef struct _gcsPATCH_LIST_VARIABLE {
    /* gcvHAL_PATCH_VIDMEM_TIMESTAMP. */
    gctUINT64           maxAsyncTimestamp;

    /* gcvHAL_PATCH_MCFE_SEMAPHORE. */
    gctBOOL             semaUsed;
} gcsPATCH_LIST_VARIABLE;

/*
 *  gcvFEATURE_DATABASE_DATE_MASK
 *
 *  Mask used to control which bits of chip date will be used to
 *  query feature database, ignore release date for fpga and emulator.
 */
#if (gcdFPGA_BUILD || defined(EMULATOR))
# define gcvFEATURE_DATABASE_DATE_MASK   (0U)
#else
# define gcvFEATURE_DATABASE_DATE_MASK   (~0U)
#endif

#if defined(__GNUC__)
#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__) && \
     (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define gcdENDIAN_BIG   1
# else
#  define gcdENDIAN_BIG   0
# endif
#else
# define gcdENDIAN_BIG    0
#endif

#ifdef __cplusplus
}
#endif

#endif /* __gc_hal_types_shared_h_ */
