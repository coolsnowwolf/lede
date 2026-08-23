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
	omci_me_traffic_management.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/

#include <stdio.h>
#include <unistd.h>
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_traffic_management.h"

#define MULTICAST_IWTP_TABLE_ENTRY_LEN 12

/*******************************************************************************************************************************
9.2.7: GAL Ethernet profile

********************************************************************************************************************************/

int omciMeInitForGALEthernetProfile(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForGALEthernetProfile \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int32 getGALEthernetProfileMaxGemPayloadSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 maxSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr)-1;

	//api
	//ret = macMT7530GetMaxSize(&maxSize, portId);
	if(ret == -1)
		return ret;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	put16(attributeValuePtr, maxSize);
	
	return getTheValue(value, (char *)attributeValuePtr, 2, omciAttribute);
}

int32 setGALEthernetProfileMaxGemPayloadSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 maxSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr)-1;
	maxSize = get16((uint8 *)value);
	//api
	//ret = macMT7530SetMaxSize(maxSize, portId);
	if(ret == -1)
		return ret;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	put16(attributeValuePtr, maxSize);
	
	return 0;
}


/*******************************************************************************************************************************
9.2.8: GAL Ethernet performance monitoring history data

********************************************************************************************************************************/

int omciMeInitForGALEthernetPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForGALEthernetPM \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	
	return 0;
}

int32 setGALEthernetPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
#if 0
	uint16 thresholdMeId = 0;
	int ret = 0;
	uint32 thresholdData = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
#endif

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

#if 0
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 1, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 1, tempBuffer);
	if (ret != 0)
		return -1;
#endif

	return 0;
}

int32 getGALEthernetPMDiscardedFrames(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 1, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*******************************************************************************************************************************
9.2.9: FEC performance monitoring history data

********************************************************************************************************************************/

int omciMeInitForFecPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForFecPM \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	
	return 0;
}

int32 setFecPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
#if 0
	uint16 thresholdMeId = 0;
	int ret = 0;
	int i = 0;
	uint32 thresholdData = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
#endif

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
#if 0
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);

	for(i=1; i<4; i++)
	{
		ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, i, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%x", thresholdData);
		ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, i, tempBuffer);
		if (ret != 0)
			return -1;
	}

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 4, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 5, tempBuffer);
	if (ret != 0)
		return -1;
#endif
	return 0;
}

int32 getFecPMCorrectedBytes(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 1, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getFecPMCorrectedCodeWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 2, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getFecPMUnCorrectedCodeWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getFecPMTotalCorrectedWords(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getFecPMFecSeconds(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 tmpPmData = 0;
	uint16 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &tmpPmData);
	if(tmpPmData <= 65535)
		pmData = tmpPmData;
	else
		return -1;
	put16((uint8 *)value, pmData);

	return 0;
}


/*******************************************************************************************************************************
9.2.4: GEM interworking termination point

********************************************************************************************************************************/

int omciMeInitForGemIWTP(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForGemIWTP \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int32 getGemIWTPGalLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint8 mode = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr)-1;

	//api
	//ret = macMT7530GetIWTPLoopbackCfg(&mode, portId);
	if(ret == -1)
		return ret;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = mode;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 setGemIWTPGalLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint8 mode = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr)-1;
	mode = *((uint8*)value);

	//api
	//ret = macMT7530SetIWTPLoopbackCfg(mode, portId);
	if(ret == -1)
		return ret;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = mode;
	
	return 0;
}

int32 getGemIWTPPPTPCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = 0;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 getGemIWTPOperState(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = 0;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

/*******************************************************************************************************************************
9.2.5: Multicast GEM interworking termination point

********************************************************************************************************************************/

int omciMeInitForMulticastGemIWTP(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForMulticastGemIWTP \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	
	return 0;
}

int32 setMulticastGemIWTPNotUsed1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tmpValue = *((uint8*)value);
	if(tmpValue == 0){
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		memcpy(attributeValuePtr, value, length);
	}
	else
	{
		return -1;
	}

	return 0;
}

int32 getMulticastGemIWTPPPTPCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = 0;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 getMulticastGemIWTPOperState(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{	
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = 0;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}


int32 setMulticastGemIWTPNotUsed2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tmpValue = *((uint8*)value);
	if(tmpValue == 0){
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
		if(attributeValuePtr == NULL)
			return -1;
		memcpy(attributeValuePtr, value, length);
	}
	else
	{
		return -1;
	}

	return 0;
}

int32 getMulticastGemIWTPIPv4MultiAddrTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciTableAttriValue_t *tmp_ptr = NULL;	
	char *tableBuffer = NULL;	
	uint32 tableSize = 0;
	int ret = 0;	
	uint16 instanceId = 0;	
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex);	
	if(tmp_ptr == NULL){		
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getTB->tmp_ptr is NULL \r\n");
		return 0;		
	}
	
	/*1.calculate the table size*/	
	tableSize = tmp_ptr->tableSize;		
	
	/*2.get table content*/
	if(tmp_ptr->tableValue_p == NULL)
		return 0;
	tableBuffer = (char *)tmp_ptr->tableValue_p;	
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getTB->tableBuffer is NULL \r\n");
		return 0;			
	}
	
	/*3. add the table to omci archtecture*/	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	ret = omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getTB: omciAddTableContent fail \r\n");		
		return 0;
	}
	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMulticastGemIWTPIPv4MultiAddrTB->tableSize = %d",tableSize);

	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);	
}

int32 setMulticastGemIWTPIPv4MultiAddrTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciTableAttriValue_t *tableContent_ptr = NULL; 
	uint8 *tableValue_ptr = NULL;
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 tableSize = 0;
	int ret = 0;
	uint8 content[16] = {0};
	uint8 zero[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
	uint16 length = 0;
	uint8 findFlag = 0;//0:not find, 1:find

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	/*get this table list*/
	tableContent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex);
	if(tableContent_ptr == NULL){
		tableSize = 0;
		tableValue_ptr = NULL;
	}else{
		tableSize = tableContent_ptr->tableSize;
		tableValue_ptr = tableContent_ptr->tableValue_p;
	}
	tmp_ptr = tableValue_ptr;

	memcpy(content, value, 4);
	if(tmp_ptr != NULL){
		/*search current entry*/
		length = MULTICAST_IWTP_TABLE_ENTRY_LEN;
		while(tmp_ptr != NULL){
			if(memcmp(content, tmp_ptr, 4) == 0){
				findFlag = 1;
				break;			
			}
			length += MULTICAST_IWTP_TABLE_ENTRY_LEN;
			if(length > tableContent_ptr->tableSize){
				//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMulticastGemIWTPIPv4MultiAddrTB->length > tableContent_ptr->tableSize");
				findFlag = 0;
				break;
			}
			tmp_ptr += MULTICAST_IWTP_TABLE_ENTRY_LEN;
		}
	}
	
	if(findFlag == 1){
		memset(content, 0, sizeof(content));
		memcpy(content, value+4, 8);
		
		if(memcmp(content, zero, 8) != 0)
		{
			/*if last 8 bytes aren't all zeros, replace this entry*/
			memcpy(tmp_ptr, value, MULTICAST_IWTP_TABLE_ENTRY_LEN);
		}
		else
		{
			/*if last 8 bytes are all zeros, delete this entry*/
			if(tableSize == MULTICAST_IWTP_TABLE_ENTRY_LEN)//only this entry in list
			{
				if(tableContent_ptr != NULL){
					/*del table content in instance table list*/
					ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB: omciDelTableValueOfInst failure");
						ret = -1;
						goto end;
					}
				}
			}
			else
			{
				/*malloc new space*/
				tmpValue_ptr = calloc(1 , tableSize-MULTICAST_IWTP_TABLE_ENTRY_LEN);
				if(tmpValue_ptr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB:calloc failure");
					ret = -1;
					goto end;	
				}
				/*delete this entry*/
				if(length == MULTICAST_IWTP_TABLE_ENTRY_LEN){
					memcpy(tmpValue_ptr, tableValue_ptr+MULTICAST_IWTP_TABLE_ENTRY_LEN, tableSize-MULTICAST_IWTP_TABLE_ENTRY_LEN);
					tableSize -= MULTICAST_IWTP_TABLE_ENTRY_LEN;
				}
				else
				{
					memcpy(tmpValue_ptr, tableValue_ptr, length-MULTICAST_IWTP_TABLE_ENTRY_LEN);
					if((tableSize-length) > 0)
						memcpy(tmpValue_ptr+length-MULTICAST_IWTP_TABLE_ENTRY_LEN, tableValue_ptr+length, tableSize-length);
					tableSize -= MULTICAST_IWTP_TABLE_ENTRY_LEN;
				}

				/*we need to update instance table list, because of new alloc*/
				if(tableContent_ptr != NULL){
					/*del table content in instance table list*/
					ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB: omciDelTableValueOfInst failure");
						ret = -1;
						goto end;
					}
				}

				tableContent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex, tableSize, tmpValue_ptr);
				if(tableContent_ptr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB: omciAddTableValueToInst failure");
					ret = -1;
					goto end;	
				}
			}
		}
	}
	else
	{
		/*if not find, add this entry*/
		/*malloc new space*/
		tmpValue_ptr = calloc(1 , tableSize+MULTICAST_IWTP_TABLE_ENTRY_LEN);
		if(tmpValue_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB:calloc failure");
			ret = -1;
			goto end;		
		}
		/*add this entry*/
		if(tableValue_ptr != NULL)
			memcpy(tmpValue_ptr, tableValue_ptr, tableSize);
		memcpy(tmpValue_ptr+tableSize, value, MULTICAST_IWTP_TABLE_ENTRY_LEN);		
		tableSize += MULTICAST_IWTP_TABLE_ENTRY_LEN;	

		/*we need to update instance table list, because of new alloc*/
		if(tableContent_ptr != NULL){
			/*del table content in instance table list*/
			ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB: omciDelTableValueOfInst failure");
				ret = -1;
				goto end;
			}
		}

		tableContent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, omciAttriDescriptListMulticastGemIWTP[9].attriIndex, tableSize, tmpValue_ptr);
		if(tableContent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMulticastGemIWTPIPv4MultiAddrTB: omciAddTableValueToInst failure");
			ret = -1;
			goto end;	
		}
	}
	
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}
	return ret;
}


