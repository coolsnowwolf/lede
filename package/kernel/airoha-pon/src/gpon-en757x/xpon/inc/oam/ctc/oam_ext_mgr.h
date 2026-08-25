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
	oam_ext_mgr.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lightman		2012/8/24	Create
*/
#ifndef OAM_EXT_MGR_H
#define OAM_EXT_MGR_H

#include "libtcapi.h"
#include "epon_oam.h"
#include "ctc/oam_ext_cmn.h"

/* DiscoveryState */
#define DS_PASSIVE_WAIT  0
#define DS_SEND_EXT_INFO 1
#define DS_SEND_ANY      2

/* The organization Specific the OAM PDU Handle structure
 * Use the struct to handle different Organization Spec Data
 * */
typedef struct extHandler_s
{
	u_char   oui[3];
	u_char   dscvryState[EPON_LLID_MAX_NUM];
	u_char   usedVerion[EPON_LLID_MAX_NUM];
	int     (*procInfoData)( // need to hold the machine state
					u_char llid,
					u_char *pOutData, // output data buff
					int   *outLen,   // output data buff length
					u_char *pInData,  // org spec TLV
					int   inLen      // org spec TLV length
					);			// Code = 0x00
	int     (*buildEventData)(
                    Buff_Ptr bfp,
					u_char *pData,
					int   length
					);			// Code = 0x01
	int     (*procExtData)(
					OUT Buff_Ptr bfp,
					IN  Buff_Ptr obfp
					);			// Code = 0xFE
}ExtHandler_t, *ExtHandler_Ptr;

typedef struct extOrgMgr_s
{
	ExtHandler_Ptr pHandler;
	
	int (*initExtHandler)(ExtHandler_Ptr pExtHdlr);
}ExtOrgMgr_t, *ExtOrgMgr_Ptr;

void initExtOrgMgr();
void resetExtOrgDscvyState(u_char llid);
ExtHandler_Ptr findHandlerByOui(u_char *oui);

void eponOamSetCtcDiscoveryState(unsigned char llid, unsigned char state);

#if 0
typedef struct oamAlarmMsg_s
{
	long msg_type;

	u_short trapIndex;			// Alarm ID
	u_short devIndex_1;			// port ID
	u_short devIndex_2;			// LLID
	u_short devIndex_3;			// PON IF
	u_char  trapOtherInfo[100];	// Alarm State & Info
}OamAlarmMsg_t, *OamAlarmMsg_Ptr;
void eponOamProcessAlarm(OamAlarmMsg_Ptr pAlarm);
#endif
void eponOamProcessAlarm(OamAlarmMsg_Ptr pAlarm);

#endif
