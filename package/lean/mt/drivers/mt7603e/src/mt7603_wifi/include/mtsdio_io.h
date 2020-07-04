/*
 ***************************************************************************
 * MediaTek Inc. 
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mtsdio_io.h
*/

#ifndef __MTSDIO_IO_H__
#define __MTSDIO_IO_H__

INT32 MTSDIORead32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value);
INT32 MTSDIOWrite32(struct _RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value);
INT32 MTSDIOMultiRead(struct _RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length);
INT32 MTSDIOMultiWrite(struct _RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length);

#endif

