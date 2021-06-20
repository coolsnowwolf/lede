/* 
LzmaTypes.h 

Types for LZMA Decoder

This file written and distributed to public domain by Igor Pavlov.
This file is part of LZMA SDK 4.40 (2006-05-01)
*/

#ifndef __LZMATYPES_H
#define __LZMATYPES_H

#ifndef _7ZIP_BYTE_DEFINED
#define _7ZIP_BYTE_DEFINED
typedef unsigned char Byte;
#endif 

#ifndef _7ZIP_UINT16_DEFINED
#define _7ZIP_UINT16_DEFINED
typedef unsigned short UInt16;
#endif 

#ifndef _7ZIP_UINT32_DEFINED
#define _7ZIP_UINT32_DEFINED
#ifdef _LZMA_UINT32_IS_ULONG
typedef unsigned long UInt32;
#else
typedef unsigned int UInt32;
#endif
#endif 

/* #define _LZMA_NO_SYSTEM_SIZE_T */
/* You can use it, if you don't want <stddef.h> */

#ifndef _7ZIP_SIZET_DEFINED
#define _7ZIP_SIZET_DEFINED
#ifdef _LZMA_NO_SYSTEM_SIZE_T
typedef UInt32 SizeT;
#else
#include <stddef.h>
typedef size_t SizeT;
#endif
#endif

#endif
