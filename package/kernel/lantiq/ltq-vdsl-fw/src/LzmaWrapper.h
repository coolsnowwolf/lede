/******************************************************************************
**
** FILE NAME    : LzmaWrapper.h
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
*******************************************************************************/
#ifndef  __LZMA_WRAPPER_H__
#define  __LZMA_WRAPPER_H__

#ifndef LZMA_RESULT_OK
#define LZMA_RESULT_OK 0
#endif
#ifndef LZMA_RESULT_DATA_ERROR
#define LZMA_RESULT_DATA_ERROR 1
#endif

extern int lzma_inflate(unsigned char *source, int s_len, unsigned char *dest, int *d_len);

#endif /*__LZMA_WRAPPER_H__*/
