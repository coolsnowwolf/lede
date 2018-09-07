/******************************************************************************
**
** FILE NAME    : LzmaWrapper.c
** PROJECT      : bootloader
** MODULES      : U-boot
**
** DATE         : 2 Nov 2006
** AUTHOR       : Lin Mars
** DESCRIPTION  : LZMA decoder support for U-boot 1.1.5
** COPYRIGHT    :       Copyright (c) 2006
**                      Infineon Technologies AG
**                      Am Campeon 1-12, 85579 Neubiberg, Germany
**
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
** HISTORY
** $Date        $Author         $Comment
** 2 Nov 2006   Lin Mars        init version which derived from LzmaTest.c from
**                              LZMA v4.43 SDK
** 24 May 2007	Lin Mars	Fix issue for multiple lzma_inflate involved
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "LzmaDecode.h"
#include "LzmaWrapper.h"

#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
static const char *kCantReadMessage = "Can not read from source buffer";
static const char *kCantAllocateMessage = "Not enough buffer for decompression";
#endif

static size_t rpos=0, dpos=0;

static int MyReadFileAndCheck(unsigned char *src, void *dest, size_t size)
{
  if (size == 0)
    return 0;
  memcpy(dest, src + rpos, size);
  rpos += size;
  return 1;
}

int lzma_inflate(unsigned char *source, int s_len, unsigned char *dest, int *d_len)
{
  /* We use two 32-bit integers to construct 64-bit integer for file size.
     You can remove outSizeHigh, if you don't need >= 4GB supporting,
     or you can use UInt64 outSize, if your compiler supports 64-bit integers*/
  UInt32 outSize = 0;
  UInt32 outSizeHigh = 0;
  SizeT outSizeFull;
  unsigned char *outStream;
  
  int waitEOS = 1; 
  /* waitEOS = 1, if there is no uncompressed size in headers, 
   so decoder will wait EOS (End of Stream Marker) in compressed stream */

  SizeT compressedSize;
  unsigned char *inStream;

  CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
  unsigned char properties[LZMA_PROPERTIES_SIZE];

  int res;

  rpos=0; dpos=0;

  if (sizeof(UInt32) < 4)
  {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
    printf("LZMA decoder needs correct UInt32\n");
#endif
    return LZMA_RESULT_DATA_ERROR;
  }

  {
    long length=s_len;
    if ((long)(SizeT)length != length)
    {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
      printf("Too big compressed stream\n");
#endif
      return LZMA_RESULT_DATA_ERROR;
    }
    compressedSize = (SizeT)(length - (LZMA_PROPERTIES_SIZE + 8));
  }

  /* Read LZMA properties for compressed stream */

  if (!MyReadFileAndCheck(source, properties, sizeof(properties)))
  {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
    printf("%s\n", kCantReadMessage);
#endif
    return LZMA_RESULT_DATA_ERROR;
  }

  /* Read uncompressed size */
  {
    int i;
    for (i = 0; i < 8; i++)
    {
      unsigned char b;
      if (!MyReadFileAndCheck(source, &b, 1))
      {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
        printf("%s\n", kCantReadMessage);
#endif
        return LZMA_RESULT_DATA_ERROR;
      }
      if (b != 0xFF)
        waitEOS = 0;
      if (i < 4)
        outSize += (UInt32)(b) << (i * 8);
      else
        outSizeHigh += (UInt32)(b) << ((i - 4) * 8);
    }
    
    if (waitEOS)
    {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
      printf("Stream with EOS marker is not supported");
#endif
      return LZMA_RESULT_DATA_ERROR;
    }
    outSizeFull = (SizeT)outSize;
    if (sizeof(SizeT) >= 8)
      outSizeFull |= (((SizeT)outSizeHigh << 16) << 16);
    else if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize)
    {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
      printf("Too big uncompressed stream");
#endif
      return LZMA_RESULT_DATA_ERROR;
    }
  }

  /* Decode LZMA properties and allocate memory */
  if (LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE) != LZMA_RESULT_OK)
  {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
    printf("Incorrect stream properties");
#endif
    return LZMA_RESULT_DATA_ERROR;
  }
  state.Probs = (CProb *)malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));

  if (outSizeFull == 0)
    outStream = 0;
  else
  {
    if (outSizeFull > d_len)
      outStream = 0;
    else
      outStream = dest;
  }

  if (compressedSize == 0)
    inStream = 0;
  else
  {
    if ((compressedSize+rpos) > s_len )
      inStream = 0;
    else
      inStream = source + rpos;
  }

  if (state.Probs == 0 
    || (outStream == 0 && outSizeFull != 0)
    || (inStream == 0 && compressedSize != 0)
    )
  {
    free(state.Probs);
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
    printf("%s\n", kCantAllocateMessage);
#endif
    return LZMA_RESULT_DATA_ERROR;
  }

  /* Decompress */
  {
    SizeT inProcessed;
    SizeT outProcessed;
    res = LzmaDecode(&state,
      inStream, compressedSize, &inProcessed,
      outStream, outSizeFull, &outProcessed);
    if (res != 0)
    {
#if defined(DEBUG_ENABLE_BOOTSTRAP_PRINTF) || !defined(CFG_BOOTSTRAP_CODE)
      printf("\nDecoding error = %d\n", res);
#endif
      res = 1;
    }
    else
    {
      *d_len = outProcessed;
    }
  }

  free(state.Probs);
  return res;
}
