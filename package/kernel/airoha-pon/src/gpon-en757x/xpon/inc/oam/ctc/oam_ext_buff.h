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
	oam_ext_buf.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#ifndef OAM_BUFF_H_
#define OAM_BUFF_H_
#include "epon_oam_types.h"
#include <netinet/in.h>
#include "epon_oam.h"

#define BUFF_MAX_LEN (EPON_MAX_OAMPDU_SIZE-21-2) //1536 - 21(oamPduHdr) - 2(llid)
#define OAM_MAX_BUFF 12

#define BF_NEED_SEND (1<<0)
#define BF_FROM_LAN  (1<<1)

typedef struct Buff
{
	u_char llidIdx;
	u_char flag;	// used for buff's attribute
	int   offset;     // for used pop buff data
	int   usedLen; // for used put buff data
	//u_char data[BUFF_MAX_LEN];
	u_char *data;
}Buff_t, *Buff_Ptr;


void putBuffU8(Buff_Ptr bfp, u_char data);
void putBuffU16(Buff_Ptr bfp, u_short data);
void putBuffU24(Buff_Ptr bfp, const u_char data[3]);
void putBuffUint(Buff_Ptr bfp, u_int data);
void putBuffData(Buff_Ptr bfp, u_char *data, int length);

void addBuffFlag(Buff_Ptr bfp, u_char flag);
void delBuffFlag(Buff_Ptr bfp, u_char flag);

#define isEnough(bfp, len) ((((bfp)->offset + (len)) > (bfp)->usedLen)? 0 : 1)
#define isNotEnough(bfp, len) ((((bfp)->offset + (len)) > (bfp)->usedLen)? 1 : 0)
#endif
