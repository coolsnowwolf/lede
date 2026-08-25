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
	oam_ctc_event.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_EVENT_H
#define OAM_CTC_EVENT_H
#include "../epon_oam_types.h"
#include "ctc/oam_ext_cmn.h"

#define ERROR_BUFF_NOTENOUGH        -1

/* OAM CTC EVENT PARAM GET/SET DEFINE */
#define EXT_EVENT_STATUS_REQUEST    	0x01
#define EXT_EVENT_STATUS_SET        		0x02
#define EXT_EVENT_STATUS_RESPONSE   	0x03
#define EXT_EVENT_THRESHOLD_REQUEST 	0x04
#define EXT_EVENT_THRESHOLD_SET     	0x05
#define EXT_EVENT_THRESHOLD_RESPONSE	0x06

/* OAM CTC EVENT STATUS VALUE */
#define EVENT_DISABLED  	0x00000000
#define EVENT_ENABLED   		0x00000001
#define EVENT_NOT_FOUND 	0xFFFFFFFF
#define ERROR_BUFF_NOTENOUGH        -1

#pragma pack(push, 1)

typedef struct oamEventHdr_s
{
    u_char   subtype;
    u_short  entryCount;
}OamEventHdr_t, *OamEventHdr_Ptr;

typedef struct oamEventEntry_s
{
	u_short	objectType;
	u_int	instanceNum;
	u_short	alarmID;
}OamEventEntry_t, *OamEventEntry_Ptr;

typedef struct oamEventStatusEntry_s
{
	u_short	objectType;
	u_int	instanceNum;
	u_short	alarmID;
	u_int	status;
}OamEventStatusEntry_t, *OamEventStatusEntry_Ptr;

typedef struct oamEventThresholdEntry_s
{
	u_short	objectType;
	u_int	instanceNum;
	u_short	alarmID;
	u_int	setThreshold;
	u_int	clearThreshold;
}OamEventThresholdEntry_t, *OamEventThresholdEntry_Ptr;

// for store the current config of every alarm node
typedef struct oamAlarmEntry_s
{
	u_short	objectType;
	u_int	instanceNum;
	u_short	alarmID;
	u_int	status;
	u_int	setThreshold;
	u_int	clearThreshold;
	u_char	currStatus;
}OamAlarmEntry_t;

#pragma pack(pop)

#define ALARM_OBJ_ONU        0x00 // ONU alarm id high byte
#define ALARM_OBJ_PONIF      0x01 // PON IF alarm id high byte
#define ALARM_OBJ_ETHER_PORT 0x03 // Ether Port alarm id high byte
#define ALARM_OBJ_VOIP_PORT  0x04 // VOIP Port alarm id high byte
#define ALARM_OBJ_E1_PORT    0x05 // E1 Port alarm id high byte
int sysAlarmAviable(OamEventEntry_Ptr oeep);

void ntohEventEntry(INOUT OamEventEntry_Ptr eep);
void htonEventStatus(INOUT OamEventStatusEntry_Ptr esep);
void ntohEventStatus(INOUT OamEventStatusEntry_Ptr esep);
void htonEventThreshold(INOUT OamEventThresholdEntry_Ptr etep);
void ntohEventThreshold(INOUT OamEventThresholdEntry_Ptr etep);

int procExtEvent(Buff_Ptr bfp, u_char *evt_data, int length);


int procEventStatusRequest(Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length);
int buildOamEventStatusResponse(Buff_Ptr bfp, OamEventEntry_Ptr evtEntryPtr);
void oamGetAlarmStatus(OamEventStatusEntry_Ptr status_ptr);

int procEventStatusSet(Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length);
int buildOamEventStatusSetResponse(Buff_Ptr bfp, OamEventStatusEntry_Ptr evtStatusEntryPtr);
void oamSetAlarmStatus(OamEventStatusEntry_Ptr status_ptr);

int procEventThresholdRequest(Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length);
int buildOamEventThresholdResponse(Buff_Ptr bfp, OamEventEntry_Ptr evtEntryPtr);
void oamGetAlarmThreshold(OamEventThresholdEntry_Ptr threshold_ptr);

int procEventThresholdSet(Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length);
int buildOamEventThresholdSetResponse(Buff_Ptr bfp, OamEventThresholdEntry_Ptr evtThresholdEntryPtr);
void oamSetAlarmThreshold(OamEventThresholdEntry_Ptr threshold_ptr);

#endif
