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
	oam_ctc_auth.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_AUTH_H
#define OAM_CTC_AUTH_H
#include "ctc/oam_ext_cmn.h"

/* OAM EXT CTC Authentication DEFINE */
#define AUTH_CODE_REQUEST          0x01
#define AUTH_CODE_RESPONSE         0x02
#define AUTH_CODE_SUCESS           0x03
#define AUTH_CODE_FAILURE          0x04

#define AUTH_TYPE_LOID_PWD         0x01
#define AUTH_TYPE_NAK              0x02

#define AUTH_FAILURE_LOID_NOEXIST  0x01
#define AUTH_FAILURE_PWD_WRONG     0x02
#define AUTH_FAILURE_LOID_CONFLICT 0x03

#pragma pack(push, 1)

/* ONU Authentication Ext PDU */
typedef struct oamAuth_s
{
	u_char	authCode;
	u_short	length;
}OamAuth_t, *OamAuth_Ptr;

#pragma pack(pop)


int procExtAuth(OUT Buff_Ptr bfp, IN Buff_Ptr obfp);

int buildAuthResponse(OUT Buff_Ptr bfp, IN u_char authType);

void getLoidPassword(IN u_char llidIdx, OUT u_char loid[24], OUT u_char password[12]);

void procAuthFailure(IN u_char llid, IN u_char failureType);
void procAuthSuccess(IN u_char llid);
#endif
