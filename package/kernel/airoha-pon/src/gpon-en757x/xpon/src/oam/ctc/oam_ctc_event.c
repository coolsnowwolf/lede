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
	oam_ctc_event.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include "ctc/oam_ctc_event.h"
#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ctc_node.h"


inline void ntohEventEntry(OamEventEntry_Ptr eep)
{
	eep->alarmID     = ntohs(eep->alarmID);
	eep->objectType  = ntohs(eep->objectType);
	eep->instanceNum = ntohl(eep->instanceNum);
}

inline void htonEventStatus(OamEventStatusEntry_Ptr esep)
{
	esep->alarmID     = htons(esep->alarmID);
	esep->objectType  = htons(esep->objectType);
	esep->instanceNum = htonl(esep->instanceNum);
	esep->status      = htonl(esep->status);
}

inline void ntohEventStatus(OamEventStatusEntry_Ptr esep)
{
	esep->alarmID     = ntohs(esep->alarmID);
	esep->objectType  = ntohs(esep->objectType);
	esep->instanceNum = ntohl(esep->instanceNum);
	esep->status      = ntohl(esep->status);
}

inline void htonEventThreshold(OamEventThresholdEntry_Ptr etep)
{
	etep->alarmID        = htons(etep->alarmID);
	etep->objectType     = htons(etep->objectType);
	etep->instanceNum    = htonl(etep->instanceNum);
	etep->setThreshold   = htonl(etep->setThreshold);
	etep->clearThreshold = htonl(etep->clearThreshold);
}

inline void ntohEventThreshold(OamEventThresholdEntry_Ptr etep)
{
	etep->alarmID        = ntohs(etep->alarmID);
	etep->objectType     = ntohs(etep->objectType);
	etep->instanceNum    = ntohl(etep->instanceNum);
	etep->setThreshold   = ntohl(etep->setThreshold);
	etep->clearThreshold = ntohl(etep->clearThreshold);
}


/* 
 * Test the object is the alarm's object.
 * And test the object inst is exist or not.
 * And translate object inst to system used.
 */
int sysAlarmAviable(OamEventEntry_Ptr oeep)
{
	u_char alarmObjType = (oeep->alarmID) >> 8;
	u_char portType = (oeep->instanceNum) >> 24;
	u_char lidx = (oeep->instanceNum & 0x000000FF);
	int   ret = FALSE;

	if (oeep->objectType == OBJ_LF_PORT){
		switch(portType){
			case PORT_ETHER:
				if (alarmObjType == ALARM_OBJ_ETHER_PORT){
					if (lidx >= OBJ_LF_ETH_START_V2 && lidx < OBJ_LF_ETH_START_V2+PORT_ETHER_NUM){						
						lidx -= OBJ_LF_ETH_START_V2;
						ret = TRUE;
					}
				}
			break;
			case PORT_VOIP:
				if (alarmObjType == ALARM_OBJ_VOIP_PORT){
					if (lidx >= OBJ_LF_VOIP_START_V2 && lidx < OBJ_LF_VOIP_START_V2+PORT_VOIP_NUM){
						lidx -= OBJ_LF_VOIP_START_V2;
						ret = TRUE;
					}
				}
			break;			
			case PORT_E1:
				if (alarmObjType == ALARM_OBJ_E1_PORT){
					ret = FALSE;
				}
			break;
		}
	}else if (oeep->objectType == OBJ_LF_PONIF){
		if (alarmObjType == ALARM_OBJ_PONIF && lidx < PONIF_NUM)
			ret = TRUE;
	}else if (oeep->objectType == OBJ_LF_ONU){
		if (alarmObjType == ALARM_OBJ_ONU)
			ret = TRUE;
	}

	if (ret == TRUE){
		oeep->instanceNum = lidx; // translate port code to the system used.
	}
	return ret;
}


/* ------------------------------------Begin Ext Event--------------------------------------*/
/* ----------------------------   <-- evt_data
 * | 1         Subtype        |
 * ----------------------------
 * | 2       entryCount       |
 * ----------------------------
 * | X          data          |   length = X + 3
 * ----------------------------
 * */
int procExtEvent(Buff_Ptr bfp, u_char *evt_data_ptr, int length)
{
	u_char	*dp = evt_data_ptr;
	OamEventHdr_Ptr ehp = (OamEventHdr_Ptr)dp;

	if (length < 3)
		return FAIL;

    ehp->entryCount = ntohs(ehp->entryCount);

	PTR_MOVE(dp, sizeof(OamEventHdr_t));
	length -= sizeof(OamEventHdr_t);

	eponOamExtDbg(DBG_OAM_L2, ">> Event.subtype = 0x%02X EntryCount = %d\n", ehp->subtype, ehp->entryCount);

	switch(ehp->subtype){
	case EXT_EVENT_STATUS_REQUEST:
		putBuffU8(bfp, EXT_EVENT_STATUS_RESPONSE);
		procEventStatusRequest(bfp, ehp->entryCount, dp, length);
		break;
	case EXT_EVENT_STATUS_SET:
		putBuffU8(bfp, EXT_EVENT_STATUS_RESPONSE);
		procEventStatusSet(bfp, ehp->entryCount, dp, length);
		break;
	case EXT_EVENT_THRESHOLD_REQUEST:
		putBuffU8(bfp, EXT_EVENT_THRESHOLD_RESPONSE);
		procEventThresholdRequest(bfp, ehp->entryCount, dp, length);
		break;
	case EXT_EVENT_THRESHOLD_SET:
		putBuffU8(bfp, EXT_EVENT_THRESHOLD_RESPONSE);
		procEventThresholdSet(bfp, ehp->entryCount, dp, length);
		break;
	default:
		break;
	}
	return SUCCESS;
}

/* ---------------------------
 * | 2       entryCount      |
 * ---------------------------  <-- evt_entry_ptr
 * | 2       objectType      |
 * ---------------------------
 * | 4      instanceNum      |
 * ---------------------------
 * | 2        alarmID        |
 * ---------------------------
 * |          ......         |
 * ---------------------------  length = (2+4+2) * entryCount
 * */
int procEventStatusRequest( Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length )
{
	int ret;
	u_short i = 0;
	OamEventEntry_Ptr evtEntryPtr = (OamEventEntry_Ptr)evt_entry_ptr;
	u_short real_count = MIN(entry_count , (length / sizeof(OamEventEntry_t)));

	// TODO: we can use real_count < 1 instead??
	if (length < sizeof(OamEventEntry_t))
		return FAIL;

	/* if real_count > 124(=(1500-4-3-3)/12) ,
	 * the buf will be overflow. And
	 * the real_count's max value = 186(=1490/8)
	 * */
	putBuffU16(bfp, MIN(entry_count, 124));

	// step 2: get status enabled, and build response PDU
	for (; i < real_count;	i++)
	{
		ntohEventEntry(evtEntryPtr);

		ret = buildOamEventStatusResponse(bfp, evtEntryPtr);
		if (ret == ERROR_BUFF_NOTENOUGH){
			handleBuffNotEnough(bfp, /*sizeof(OamHdr_t)+*/sizeof(OamOrgHdr_t)+1);
			putBuffU16(bfp, real_count - 124);
			buildOamEventStatusResponse(bfp, evtEntryPtr);
		}
		PTR_MOVE(evtEntryPtr, 1);
	}

	// step 3: send packet
	return SUCCESS;
}

/* Get the EventStatus by objectType & instanceNum & alarmID
 * */
inline void oamGetAlarmStatus(OamEventStatusEntry_Ptr status_ptr)
{
	OamAlarmNode_Ptr anp = findAlarmNodeById(status_ptr->alarmID);

	eponOamExtDbg(DBG_OAM_L2, ">> Event Status Get:\n   "
		"ObjectType=0x%04X InstanceNum=0x%08X AlarmID=0x%04X\n",\
		status_ptr->objectType, status_ptr->instanceNum, status_ptr->alarmID);

	if(anp /*&& anp->oamStatus*/){
		eponOamExtDbg(DBG_OAM_L2, "\t Name = %s\r\n", anp->name);
		anp->oamStatus(status_ptr, REQ_GET);
	}/*else{
		status_ptr->status = EVENT_NOT_FOUND;
	}*/
}

/* --------------------------
 * | 2       objectType     |
 * --------------------------
 * | 4      instanceNum     |
 * --------------------------
 * | 2        alarmID       |
 * --------------------------
 * | 4      eventStatus     |
 * --------------------------
 * */
int buildOamEventStatusResponse(Buff_Ptr bfp, OamEventEntry_Ptr oeep){
	OamEventStatusEntry_t status;

	if (bfp->usedLen + sizeof(OamEventStatusEntry_t) > BUFF_MAX_LEN)
		return ERROR_BUFF_NOTENOUGH;

	status.objectType  = oeep->objectType;
	status.instanceNum = oeep->instanceNum;
	status.alarmID     = oeep->alarmID;
	status.status      = EVENT_NOT_FOUND;

	if (TRUE == sysAlarmAviable(oeep))
		oamGetAlarmStatus(&status);
	
	eponOamExtDbg(DBG_OAM_L3, "<< Build EventStatusRequestResponse:\n   "
		"ObjectType:0x%04X InstanceNum:0x%08X AlarmID:0x%04X EventStatus:0x%08X\n\n",\
		status.objectType, status.instanceNum, status.alarmID, status.status);
	
	htonEventStatus(&status);
	putBuffData(bfp, (u_char*)&status, sizeof(status));
	addBuffFlag(bfp, BF_NEED_SEND);

	return SUCCESS;
}

/* --------------------------
 * | 2      entryCount      |
 * --------------------------  <-- evt_entry_ptr
 * | 2       objectType     |
 * --------------------------
 * | 4      instanceNum     |
 * --------------------------
 * | 2       alarmID        |
 * --------------------------
 * | 4      eventStatus     |
 * --------------------------
 * |          ......        |
 * -------------------------- length = 12 * entryCount
 * */
int procEventStatusSet( Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length )
{
	int ret;
	u_short i = 0;
	OamEventStatusEntry_Ptr evtStatusEntryPtr = (OamEventStatusEntry_Ptr)evt_entry_ptr;
	u_short real_count = MIN(entry_count , (length / sizeof(OamEventStatusEntry_t)));

	putBuffU16(bfp, real_count);

	for (; i < real_count; i++)
	{
		ntohEventStatus(evtStatusEntryPtr);

		ret = buildOamEventStatusSetResponse(bfp, evtStatusEntryPtr);
		PTR_MOVE(evtStatusEntryPtr, 1);
	}

	//step 3: send packet
	return SUCCESS;
}

/* Set the EventStatus by objectType & instanceNum & alarmID & status
 * */
inline void oamSetAlarmStatus(OamEventStatusEntry_Ptr status_ptr)
{
	OamAlarmNode_Ptr anp = findAlarmNodeById(status_ptr->alarmID);

	eponOamExtDbg(DBG_OAM_L2, ">> Event Status Set: \n   "
		"ObjectType=0x%04X InstanceNum=0x%08X AlarmID=0x%04X Status=0x%08X\n",\
		status_ptr->objectType, status_ptr->instanceNum, status_ptr->alarmID, status_ptr->status);

	if(anp /*&& anp->oamStatus*/){
		eponOamExtDbg(DBG_OAM_L2, "   Name = %s\n", anp->name);
		if (status_ptr->status > 1){ // need to be skipped
			anp->oamStatus(status_ptr, REQ_GET);
		}else{
			anp->oamStatus(status_ptr, REQ_SET);
		}
	}else{
		status_ptr->status = EVENT_NOT_FOUND;
	}
}

// How to response the set ??
/* --------------------------
 * | 2       objectType     |
 * --------------------------
 * | 4      instanceNum     |
 * --------------------------
 * | 2        alarmID       |
 * --------------------------
 * | 4      eventStatus     |
 * --------------------------
 * */
int buildOamEventStatusSetResponse(Buff_Ptr bfp, OamEventStatusEntry_Ptr oesep){
	OamEventStatusEntry_t status;
	// if (bfp->usedLen + sizeof(OamEventStatusEntry_t) > BUFF_MAX_LEN)
	// 	return ERROR_BUFF_NOTENOUGH;

	if (oesep->status != EVENT_DISABLED && oesep->status != EVENT_ENABLED){
		eponOamExtDbg(DBG_OAM_L2, ">> Event Status Set Skiped:\n   "
		"ObjectType=0x%04X InstanceNum=0x%08X AlarmID=0x%04X EventStatus=0x%08X\n\n",\
		oesep->objectType, oesep->instanceNum, oesep->alarmID, oesep->status);
		return SUCCESS;
	}

	status = *oesep;

	if (TRUE == sysAlarmAviable((OamEventEntry_Ptr)oesep))
		oamSetAlarmStatus(&status);
	else{
		status.status = EVENT_NOT_FOUND;
	}
	
	eponOamExtDbg(DBG_OAM_L3, "<< Build EventStatusSetResponse:\n   "
		"ObjectType:0x%04X InstanceNum:0x%08X AlarmID:0x%04X EventStatus:0x%08X\n\n",\
		status.objectType, status.instanceNum, status.alarmID, status.status);
	
	htonEventStatus(&status);
	putBuffData(bfp, (u_char*)&status, sizeof(status));
    addBuffFlag(bfp, BF_NEED_SEND);

	return SUCCESS;
}

/* ---------------------------
 * | 2      entryCount       |
 * ---------------------------  <-- evt_entry_ptr
 * | 2       objectType      |
 * ---------------------------
 * | 4      instanceNum      |
 * ---------------------------
 * | 2       alarmID         |
 * ---------------------------
 * |          ......         |
 * --------------------------- length = 8 * entryCount
 * */
//TODO: if the reply packet's length bigger than 1496, we need split to two packet to send.
int procEventThresholdRequest( Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length )
{
	int ret;
	u_short i = 0;
	OamEventEntry_Ptr evtEntryPtr = (OamEventEntry_Ptr)evt_entry_ptr;
	u_short real_count = MIN(entry_count , (length / sizeof(OamEventEntry_t)));

	/* if real_count > 93(=(1500-4-3-3)/16) ,
	 * the buf will be overflow. And
	 * the real_count's max value = 186(=1490/8)
	 * */
	putBuffU16(bfp, MIN(real_count, 93));

	for (; i < real_count;	i++)
	{
		ntohEventEntry(evtEntryPtr);

		ret = buildOamEventThresholdResponse(bfp, evtEntryPtr);
		if (ret == ERROR_BUFF_NOTENOUGH){
			handleBuffNotEnough(bfp, /*sizeof(OamHdr_t)+*/sizeof(OamOrgHdr_t)+1);
			putBuffU16(bfp, real_count - 93);
			buildOamEventThresholdResponse(bfp, evtEntryPtr);
		}
		PTR_MOVE(evtEntryPtr, 1);
	}

	return SUCCESS;
}

inline void oamGetAlarmThreshold(OamEventThresholdEntry_Ptr threshold_ptr)
{
	OamAlarmNode_Ptr anp = findAlarmNodeById(threshold_ptr->alarmID);

	eponOamExtDbg(DBG_OAM_L2, ">> Event Threshold Get:\n   "
		"ObjectType=0x%04X InstanceNum=0x%08X AlarmID=0x%04X\n",\
		threshold_ptr->objectType, threshold_ptr->instanceNum, threshold_ptr->alarmID);

	if(anp && anp->oamThreshold){
		eponOamExtDbg(DBG_OAM_L2, "   Name = %s\n", anp->name);
		anp->oamThreshold(threshold_ptr, REQ_GET);
	}/*else{
		threshold_ptr->setThreshold   = EVENT_NOT_FOUND;
		threshold_ptr->clearThreshold = EVENT_NOT_FOUND;
	}*/
}

/* ----------------------------
 * | 2       objectType       |
 * ----------------------------
 * | 4      instanceNum       |
 * ----------------------------
 * | 2        alarmID         |
 * ----------------------------
 * | 4      setThreshold      |
 * ----------------------------
 * | 4     clearThreshold     |
 * ----------------------------
 * */
int buildOamEventThresholdResponse(Buff_Ptr bfp, OamEventEntry_Ptr oeep){
	OamEventThresholdEntry_t threshold;
	if (bfp->usedLen + sizeof(OamEventThresholdEntry_t) > BUFF_MAX_LEN)
		return ERROR_BUFF_NOTENOUGH;

	threshold.objectType  = oeep->objectType;
	threshold.instanceNum = oeep->instanceNum;
	threshold.alarmID     = oeep->alarmID;
	threshold.setThreshold = EVENT_NOT_FOUND;
	threshold.clearThreshold = EVENT_NOT_FOUND;

	if (TRUE == sysAlarmAviable(oeep))
		oamGetAlarmThreshold(&threshold);
	
	eponOamExtDbg(DBG_OAM_L3, "<< Build EventThresholdRequestResponse:\n   "
		"ObjectType:0x%04X InstanceNum:0x%08X AlarmID:0x%04X\n   "
		"SetThreshold:0x%08X ClearThreshold:0x%08X\n\n",\
		threshold.objectType, threshold.instanceNum, threshold.alarmID, \
		threshold.setThreshold, threshold.clearThreshold);
	
	htonEventThreshold(&threshold);
	putBuffData(bfp, (u_char*)&threshold, sizeof(threshold));
	addBuffFlag(bfp, BF_NEED_SEND);

	return SUCCESS;
}


/* -----------------------------------
 * | 2       entry_count             |
 * -----------------------------------     <---- evt_entry_ptr
 * | 2       objectType              |
 * -----------------------------------
 * | 4      instanceNum              |
 * -----------------------------------
 * | 2          alarmID              |
 * -----------------------------------
 * | 4        setThreshold           |
 * -----------------------------------
 * | 4       clearThreshold          |
 * -----------------------------------
 * |           .......               |
 * -----------------------------------      length = 16 * entry_count
 * */
int procEventThresholdSet( Buff_Ptr bfp, u_short entry_count,u_char *evt_entry_ptr, int length )
{
	u_short i = 0;
	u_short real_count = MIN(entry_count , (length / sizeof(OamEventThresholdEntry_Ptr)));
	OamEventThresholdEntry_Ptr evtThresholdEntryPtr = (OamEventThresholdEntry_Ptr)evt_entry_ptr;

	putBuffU16(bfp, real_count);

	for (; i < real_count;	i++)
	{
		ntohEventThreshold(evtThresholdEntryPtr);

		buildOamEventThresholdSetResponse(bfp, evtThresholdEntryPtr);
		PTR_MOVE(evtThresholdEntryPtr, 1);
	}

	return SUCCESS;
}

inline void oamSetAlarmThreshold(OamEventThresholdEntry_Ptr threshold_ptr)
{
	OamAlarmNode_Ptr anp = findAlarmNodeById(threshold_ptr->alarmID);

	eponOamExtDbg(DBG_OAM_L2, ">> Event Threshold Set:\n   "
		"ObjectType=0x%04X InstanceNum=0x%08X AlarmID=0x%04X\n   "
		"SetThreshold=0x%08X ClearThreshold=0x%08X\n", \
		threshold_ptr->objectType, threshold_ptr->instanceNum,\
		threshold_ptr->alarmID, threshold_ptr->setThreshold, \
		threshold_ptr->clearThreshold);

	if(anp && anp->oamThreshold){
		eponOamExtDbg(DBG_OAM_L2, "   Name = %s\n", anp->name);
		anp->oamThreshold(threshold_ptr, REQ_SET);
	}else{
		threshold_ptr->setThreshold   = EVENT_NOT_FOUND;
		threshold_ptr->clearThreshold = EVENT_NOT_FOUND;
	}
}

/* -----------------------------------
 * | 2       objectType              |
 * -----------------------------------
 * | 4      instanceNum              |
 * -----------------------------------
 * | 2          alarmID              |
 * -----------------------------------
 * | 4        setThreshold           |
 * -----------------------------------
 * | 4       clearThreshold          |
 * -----------------------------------
 * */
int buildOamEventThresholdSetResponse(Buff_Ptr bfp, OamEventThresholdEntry_Ptr oethep){
	OamEventThresholdEntry_t threshold;
	// if (bfp->usedLen + sizeof(OamEventThresholdEntry_t) > BUFF_MAX_LEN)
	// 	return ERROR_BUFF_NOTENOUGH;

	threshold = *oethep;

	if (TRUE == sysAlarmAviable((OamEventEntry_Ptr)oethep))
		oamSetAlarmThreshold(&threshold);
	else{
		threshold.setThreshold   = EVENT_NOT_FOUND;
		threshold.clearThreshold = EVENT_NOT_FOUND;
	}
	
	eponOamExtDbg(DBG_OAM_L3, "<< Build EventThresholdSetResponse:\n   "
		"ObjectType:0x%04X InstanceNum:0x%08X AlarmID:0x%04X\n   "
		"SetThreshold:0x%08X ClearThreshold:0x%08X\n\n",\
		threshold.objectType, threshold.instanceNum, threshold.alarmID,\
		threshold.setThreshold, threshold.clearThreshold);
	
	htonEventThreshold(&threshold);
	putBuffData(bfp, (u_char*)&threshold, sizeof(threshold));
	addBuffFlag(bfp, BF_NEED_SEND);

	return SUCCESS;
}

/* ------------------------------------End Ext Event--------------------------------------*/
