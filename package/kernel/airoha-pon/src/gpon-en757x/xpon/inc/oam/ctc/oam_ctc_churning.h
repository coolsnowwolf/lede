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
	oam_ctc_churning.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_CHURNING_INCLUDE_H
#define OAM_CTC_CHURNING_INCLUDE_H
#include "ctc/oam_ext_cmn.h"

#define EPON_MODE_1G   0x00
#define EPON_MODE_10G  0x01

#define OAM_CTC_CHURNING_NEW_KEY_REQUEST 0x00
#define OAM_CTC_CHURNING_NEW_CHURING_KEY 0x01

#pragma pack(push, 1)

typedef struct oamCtcChurning_s
{
	u_char code;
	u_char keyIndex;
}OamCtcChurning_t, *OamCtcChurning_Ptr;

#pragma pack(pop)

int procExtChurning(Buff_Ptr bfp, u_char *pData, int length);

int buildCtcChurningKeyResponse(Buff_Ptr bfp, u_char keyIdx);

u_char getEponMode();
void  getChurningKey(u_char llid, u_char index, u_char key[3]);

#endif
