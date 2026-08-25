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
	oam_ext_buf.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#include <stdio.h>
#include <memory.h>
#include "ctc/oam_ext_buff.h"

inline void putBuffU8(Buff_Ptr bfp, u_char data)
{
	u_char *ptr = bfp->data;
	ptr += bfp->usedLen;
	*ptr = data;
	bfp->usedLen += 1;
}

inline void putBuffU16(Buff_Ptr bfp, u_short data)
{
	u_char *ptr = bfp->data;
	ptr += bfp->usedLen;
	*((u_short*)ptr) = htons(data);
	bfp->usedLen += 2;
}

inline void putBuffU24(Buff_Ptr bfp, const u_char data[3])
{
	u_char *ptr = bfp->data;
	ptr += bfp->usedLen;
	*ptr++ = data[0];
	*ptr++ = data[1];
	*ptr = data[2];
	bfp->usedLen += 3;
}

inline void putBuffUint(Buff_Ptr bfp, u_int data)
{
	u_char *ptr = bfp->data;
	ptr += bfp->usedLen;
	*((u_int*)ptr) = htonl(data);
	bfp->usedLen += 4;
}

inline void putBuffData(Buff_Ptr bfp, u_char *data, int length)
{
	u_char *ptr = bfp->data;
	ptr += bfp->usedLen;
	memcpy(ptr, data, length);
	bfp->usedLen += length;
}

inline void addBuffFlag(Buff_Ptr bfp, u_char flag)
{
	bfp->flag |= flag;
}

inline void delBuffFlag(Buff_Ptr bfp, u_char flag)
{
	bfp->flag &= ~flag;
}

