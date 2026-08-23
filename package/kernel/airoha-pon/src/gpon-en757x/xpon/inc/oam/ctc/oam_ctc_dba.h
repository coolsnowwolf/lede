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
	oam_ctc_dba.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_DBA_INCLUDE_H
#define OAM_CTC_DBA_INCLUDE_H
#include "ctc/oam_ext_cmn.h"

#define OAM_CTC_GET_DBA_REQUEST  0x00
#define OAM_CTC_GET_DBA_RESPONSE 0x01
#define OAM_CTC_SET_DBA_REQUEST  0x02
#define OAM_CTC_SET_DBA_RESPONSE 0x03

#define OAM_CTC_DBA_NACK   0
#define OAM_CTC_DBA_ACK     1


int procExtDBA(Buff_Ptr bfp, u_char *pData, int length);
int buildGetDBAResponse(Buff_Ptr bfp);
int buildSetDBAResponse(Buff_Ptr bfp, u_char *pData, int length);


#endif

