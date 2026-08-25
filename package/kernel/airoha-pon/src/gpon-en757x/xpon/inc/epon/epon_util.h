/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved.	MediaTeK's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

	Module Name:
	epon_util.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	jq.zhu		2012/9/26		Create
*/

#ifndef __EPON_UTIL
#define __EPON_UTIL


//#define EPONDBG_PRINT(LEVEL, )
void eponDbgPrint(__u32 debugLevel, char *fmt,...);
 int strToMacNum(__u8 *macstr, __u8 *macnum);
  __u16 get16(__u8	*cp);
  __u32 get32 (__u8	*cp);
   __u8 *put32(	__u8	*cp,	__u32	x);
   __u8 *put16(	__u8	*cp,	__u16	x);
#endif //__EPON_UTIL

