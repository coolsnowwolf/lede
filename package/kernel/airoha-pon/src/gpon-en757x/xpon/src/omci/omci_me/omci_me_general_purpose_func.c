
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
	omci_me_general_purpose_func.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
        Andy.Yi                 2012/7/27       add the 9.12.1-11 me node
*/
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_general_purpose.h"
#include "omci_me_layer3_data_services.h"

int omciInternalCreateForUniG(uint16 classId, uint16 instanceId);
int omciInternalCreateForOltG(uint16 classId, uint16 instanceId);
int omciInternalCreateForOmci(uint16 classId, uint16 instanceId);

int omciInternalCreateForManagedEntity(uint16 classId, uint16 instanceId);
int omciInternalCreateForAttribute(uint16 classId, uint16 instanceId);
int omciInternalCreateForFileTransController(uint16 classId, uint16 instanceId);
uint16 getAttributeInstancesNum(uint16 classId, uint8 attributeId);
uint16 getClassIdFromAttributeInstancesNum(uint16 instancesNum);
uint8 getAttributeFromAttributeInstancesNum(uint16 instancesNum);


#ifdef TCSUPPORT_SNMP
int omciCreateActionForSnmpConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciDeleteActionForSnmpConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
#endif
#ifdef TCSUPPORT_CWMP
int omciInternalCreateForTr069ManageServer(uint16 classId, uint16 instanceId);
#endif


/*******************************************************************************************************************************
9.12.1 UNI-G 

********************************************************************************************************************************/
int omciMeInitForUniG(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciInternalCreateForUniG(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;	
	uint16 attriMask = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
/*0---me Id*/	
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForUniG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;

/*1---Deprecated */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForUniG: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);
	else
		goto fail;

/*3----Management capability*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForUniG: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = OMCI_AND_NOOMCI; //both OMCI and no-OMCI
	else
		goto fail;

/*4---Non-OMCI manageid*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForUniG: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);
	else
		goto fail;

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;
}

int omciInitInstForUniG(uint16 instanceId)
{
	int ret = 0;

	ret = omciInternalCreateForUniG(OMCI_CLASS_ID_UNI_G, instanceId);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_UNI_G fail\n");

	return ret;
}

/*******************************************************************************************************************************
9.12.2 OLT-G 

********************************************************************************************************************************/
int omciMeInitForOltG(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int omciInternalCreateForOltG(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;	

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"OLT-G only a instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);
	else
		goto fail;

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	return -1;
}

int omciInitInstForOltG(void)
{
	int ret = 0;

	ret = omciInternalCreateForOltG(OMCI_CLASS_ID_OLT_G,  0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_OLT_G fail\n");

	return ret;
}


int32 setOLTVendorIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint8 *attributeValuePtr;
	uint16 length = 0;
	int ret = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setOLTVendorIdValue:value = %s\n", value);
	ret = tcApiSetAttriubte("GPON_OLT", "VendorId", value);
	if (ret == 0)
	{
		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			memcpy(attributeValuePtr, value, length);
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTimeOfDayValue: attributeValuePtr == NULL\n");
			ret = -1;
		}
	}

	return ret;
}
int32 setOLTEquipmentIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint8 *attributeValuePtr;
	uint16 length = 0;
	int ret = 0;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setOLTEquipmentIdValue:value = %s\n", value);
	ret = tcApiSetAttriubte("GPON_OLT", "EquipmentId", value);
	if (ret == 0)
	{
		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			memcpy(attributeValuePtr, value, length);
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTimeOfDayValue: attributeValuePtr == NULL\n");
			ret = -1;
		}
	}

	return ret;
}
int32 setOLTVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint8 *attributeValuePtr;
	uint16 length = 0;
	int ret = 0;
	uint8  value2[16];
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setOLTVersionValue:value = %s\n", value);


//for ALCL OLT, version will be non ascii format
	memset(value2,0,16);
	memcpy(value2,value,14);
	attributeValuePtr = omciGetInstAttriByMask((omciMeInst_ptr)meInstantPtr, 1<<15, &length);
	length = 0;
	if (attributeValuePtr){
		if (strcmp(attributeValuePtr,"ALCL")==0){
			while(value2[length]){
				if (value2[length] < 0x20){
					value2[length] += 0x30;
				}
				length++;
			}
		}
	}

	ret = tcApiSetAttriubte("GPON_OLT", "Version", value2);
	
	if (ret == 0)
	{
		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			memcpy(attributeValuePtr, value, length);
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTimeOfDayValue: attributeValuePtr == NULL\n");
			ret = -1;
		}
	}

	return ret;
}
int32 getTimeOfDayValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char buffer[MAX_BUFFER_SIZE] = {0};
	uint32 second = 0;
	uint32 nanosecond = 0;

	if (value == NULL)
		return -1;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		tcapi_get("GPON_OLT", "ToD", buffer);
		sscanf(buffer, "%d,%d", &second, &nanosecond);
		put32(attributeValuePtr + 6, second);
		put32(attributeValuePtr + 10, nanosecond);
		memcpy(value, attributeValuePtr, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getGeneralValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 setTimeOfDayValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint8 *attributeValuePtr;
	uint16 length = 0;
	int ret = 0;
	uint32 SFCounter = 0;
	uint32 second = 0;
	uint32 nanoSecond = 0;
	char buffer[MAX_BUFFER_SIZE] = {0};

	if (value == NULL)
		return -1;

	SFCounter = get32((uint8*)value);
	second = get32((uint8*)value+6);
	nanoSecond = get32((uint8*)value+10);
	memset(buffer, 0, sizeof(buffer));
	sprintf(buffer, "%d,%d,%d", SFCounter, second, nanoSecond);
	ret = tcApiSetAttriubte("GPON_OLT", "ToD", buffer);
	if (ret != 0)
	        goto fail;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTimeOfDayValue: attributeValuePtr == NULL\n");
		ret = -1;
	}

fail:
	return ret;
}


/*******************************************************************************************************************************
9.12.3 Network address 

********************************************************************************************************************************/
int omciMeInitForNetworkAddress(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

/*******************************************************************************************************************************
9.12.4 Authentication method

********************************************************************************************************************************/
int omciMeInitForAuthenticationMethod(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;	
}

/*******************************************************************************************************************************
9.12.5 Large string 

********************************************************************************************************************************/
int omciMeInitForLargeString(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;		
}

/*******************************************************************************************************************************
9.12.6 Threshold data 1 

********************************************************************************************************************************/
int omciMeInitForThresholdData1(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */
	
	return 0;	
}

/*******************************************************************************************************************************
9.12.7 Threshold data 2

********************************************************************************************************************************/
int omciMeInitForThresholdData2(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;	
	
}

/*******************************************************************************************************************************
9.12.8 OMCI

********************************************************************************************************************************/
int omciMeInitForOmci(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;	
	
}

int omciInternalCreateForOmci(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;	

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "OMCI only one instance\n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);


/*meId = 0*/
//	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, &length);
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOmci: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	return -1;
}

int omciInitInstForOmci(void)
{
	int ret = 0;

	ret = omciInternalCreateForOmci(OMCI_CLASS_ID_OMCI,  0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_OMCI fail\n");

	return ret;
}
int32 getMessageTypeTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = 0;
	uint32 tableSize = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 msgType[] = {
		MT_OMCI_MSG_TYPE_CREAT,
		MT_OMCI_MSG_TYPE_DELETE ,
		MT_OMCI_MSG_TYPE_SET,
		MT_OMCI_MSG_TYPE_GET   , 				
		MT_OMCI_MSG_TYPE_GET_ALL_ALARMS    	,	
		MT_OMCI_MSG_TYPE_GET_ALL_ALARMS_NEXT ,
		MT_OMCI_MSG_TYPE_MIB_UPLOAD		,	
		MT_OMCI_MSG_TYPE_MIB_UPLOAD_NEXT	,
		MT_OMCI_MSG_TYPE_MIB_RESET	,		
		MT_OMCI_MSG_TYPE_ALARM		,		
		MT_OMCI_MSG_TYPE_AVC		,			
		MT_OMCI_MSG_TYPE_TEST				,
		MT_OMCI_MSG_TYPE_START_SW_DOWNLOAD,
		MT_OMCI_MSG_TYPE_DOWNLOAD_SEC	,		
		MT_OMCI_MSG_TYPE_END_SW_DOWNLOAD	,	
		MT_OMCI_MSG_TYPE_ACTIVE_SW		,	
		MT_OMCI_MSG_TYPE_COMMIT_SW		,		
		MT_OMCI_MSG_TYPE_SYNC_TIME	,		
		MT_OMCI_MSG_TYPE_REBOOT	,			
		MT_OMCI_MSG_TYPE_GET_NEXT		,		//26
		MT_OMCI_MSG_TYPE_TEST_RST		,		//27
		MT_OMCI_MSG_TYPE_GET_CURR_DATA	,	//	28
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
		MT_OMCI_MSG_TYPE_SET_TABLE				//29
#endif
	};/*have a external message type table*/

	
	tableSize = sizeof(msgType)/sizeof(uint8);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getMETypeTableValue:  tableSize = %x\n", tableSize);

	
	ret = omciAddTableContent(tmpomciMeInst_p->classId, 0, msgType, tableSize, omciAttribute->attriName);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}

	return getTheValue(value, (char *)&tableSize, 4, omciAttribute);	
}

int32 getMETypeTableValue(char *value, uint8 *meInstantPtr,  omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;
	uint32 tableSize = 0;
	int i;
	omciManageEntity_ptr currentME;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	

/*calculate the table size*/
	for (i = 0; i < omci.meTableSize; i++)
	{
		currentME = &omciMeTable[i];
		if (currentME->support == OMCI_ME_SUPPORT ||currentME->support == OMCI_ME_PARTIAL_SUPPORT)
		{
			tableSize += 2;
		}
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getMETypeTableValue:  tableSize = %x\n", tableSize);
	tableBuffer = osMalloc(tableSize);
	if (tableBuffer == NULL)
		return -1;

	pTableBuffer = tableBuffer;
	for (i = 0; i < omci.meTableSize; i++)
	{
		currentME = &omciMeTable[i];
		if (currentME->support == OMCI_ME_SUPPORT ||currentME->support == OMCI_ME_PARTIAL_SUPPORT)
		{
			pTableBuffer = put16(pTableBuffer, currentME->omciMeClassId);
		}
	}

	ret = omciAddTableContent(tmpomciMeInst_p->classId, 0, tableBuffer, tableSize, omciAttribute->attriName);
	if (tableBuffer != NULL)
		osFree(tableBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}
	
	return getTheValue(value, (char *)&tableSize, 4, omciAttribute);	
}



/*******************************************************************************************************************************
9.12.9 Managed entity

********************************************************************************************************************************/
int omciMeInitForManagedEntity(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	
	return 0;	
}

int omciInternalCreateForManagedEntity(uint16 classId, uint16 instanceId)
{
	int i;
	uint32 actionsMask = 0;
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;

	omciManageEntity_ptr currentME = NULL;

	

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);

	currentME = omciGetMeByClassId(instanceId);

/*0---meId */
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;

/*1---Name */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
	if (attributeValuePtr != NULL)
	{
		memset(attributeValuePtr, 0, 25);
		memcpy(attributeValuePtr, currentME->omciMeName, 25);
	}
	else
		goto fail;

/*3---Access */
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = currentME->access;
	}
	else
		goto fail;

/*6---Actions */
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		if ((currentME->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] != NULL) && (currentME->access != OMCI_ME_ACCESS_ONU))
			actionsMask = 1<<MT_OMCI_MSG_TYPE_CREAT;

		if ((currentME->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] != NULL) && (currentME->access != OMCI_ME_ACCESS_ONU))
			actionsMask = 1<<MT_OMCI_MSG_TYPE_DELETE;
		
		for (i = MT_OMCI_MSG_TYPE_DELETE+1; i < MT_OMCI_MSG_TYPE_MAX; i++)
		{
			
			if (currentME->omciActionFunc[i] != NULL)
				actionsMask |= (1<<i);
		}
		put32(attributeValuePtr, actionsMask);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attriName = %s, actionsMask = %x\n",  tmpOmciManageEntity_p->omciAttriDescriptList[6].attriName, actionsMask);

	}
	else
		goto fail;


/*8---Support */
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[8].attriName);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = currentME->support;
	}
	else
		goto fail;
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	return 0;
fail:
	omciFreeInstance(tmpomciMeInst_p);
	return -1;
}

uint16 getAttributeInstancesNum(uint16 classId, uint8 attributeId)
{
	uint16 instancesNum = 0;

	instancesNum = (uint16)attributeId;
	instancesNum = instancesNum<< 10;

	if ((classId & 0xFF00) == 0xFF00)
		classId = classId & 0x80FF;
	instancesNum = instancesNum | classId;

	return instancesNum;	
}

uint16 getClassIdFromAttributeInstancesNum(uint16 instancesNum)
{
	if ((instancesNum & 0x8000) == 0x8000)
		return instancesNum | 0xFF00;
	else
		return instancesNum & 0x83FF;
}

uint8 getAttributeFromAttributeInstancesNum(uint16 instancesNum)
{
	return (instancesNum & 0x7C00) >> 10;
}

typedef struct generalAttributeTable_s{
	char *name;
	uint16 attributeMeId;
}generalAttributeTable_t;

#define ARCATTRIBUTEINSTID 				(OMCI_CLASS_ID_ANI_G | (8 << 10))
#define ARCINTERVALATTRIBUTEINSTID 	(OMCI_CLASS_ID_ANI_G | (9 << 10))

generalAttributeTable_t generalAttributeTable[] = {
	{"ARC",   		ARCATTRIBUTEINSTID},
	{"ARC interval",  	ARCINTERVALATTRIBUTEINSTID }
};

int attriubteIsGeneral(char *name)
{
	int i;

	for(i = 0; i < sizeof(generalAttributeTable)/sizeof(generalAttributeTable_t); i++)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "attriubteIsGeneral: name= %s, generalAttributeTable[i].name = %s\n", name,generalAttributeTable[i].name);
		if(strcmp(name, generalAttributeTable[i].name) == 0)
			return generalAttributeTable[i].attributeMeId;
	}
	return 0;
}

int omciInitInstForManagedEntity(void)
{
	int i;
	uint16 classId;
	int ret = 0;
	
	for(i = 0; i < omci.meTableSize; i++)
	{
		classId = omciMeTable[i].omciMeClassId;
		ret = omciInternalCreateForManagedEntity(OMCI_CLASS_ID_MANAGED_ENTITY,  classId);
		if (ret != 0)
		{
			goto goToEnd;
		}
	}
		
goToEnd:
	return ret;
}

int createGeneralAttribute(omciManageEntity_ptr attributeME)
{
	int i;
	uint16 instanceId; 
	int ret = 0;

	for(i = 0; i < sizeof(generalAttributeTable)/sizeof(generalAttributeTable_t); i++)
	{
		instanceId = generalAttributeTable[i].attributeMeId;
		ret = omciInternalCreateForAttribute(OMCI_CLASS_ID_ATTRIBUTE, instanceId);
		if (ret != 0)
			goto goToEnd;
	}

goToEnd:
	return ret;
}

int omciInitInstForAttribute(void)
{
	int i;
	uint8 j;
	omciManageEntity_ptr currentME;
	omciManageEntity_ptr attributeME;
	omciAttriDescript_ptr currentAttribute;
	uint16 attributeInstancesNum = 0;
	int ret = 0;
	uint8 attributeNum = 0;

	attributeME = omciGetMeByClassId(OMCI_CLASS_ID_ATTRIBUTE);
	
	ret = createGeneralAttribute(attributeME);
	if (ret != 0)
		goto goToEnd;

	
	for(i = 0; i <omci.meTableSize; i++)
	{
		currentME = &omciMeTable[i];
		attributeNum = omciGetAttriNumOfMe(currentME);
		
		for(j = 1; j < attributeNum; j++)
		{
			currentAttribute = &currentME->omciAttriDescriptList[j];
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "createAttributeOfMe: classId = %x, attriIndex = %x, attriName = %s\n", currentME->omciMeClassId, currentAttribute->attriIndex, currentAttribute->attriName);
			if(attriubteIsGeneral((char *)currentAttribute->attriName))
			{
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "createAttributeOfMe: attriubteIsGeneral, currentAttribute->attriName = %s\n", currentAttribute->attriName);
				continue;
			}
			attributeInstancesNum = getAttributeInstancesNum(currentME->omciMeClassId, j); /*xxxx xxxx xxxx xxxx        0-9 ,15, ME class id, 10-14, attribute id*/		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "createAttributeOfMe: attributeInstancesNum = %x\n", attributeInstancesNum);
			ret = omciInternalCreateForAttribute(OMCI_CLASS_ID_ATTRIBUTE, attributeInstancesNum);
			if (ret != 0)
			{
				goto goToEnd;
			}
		}
	}
		
goToEnd:
	return ret;
}

int getAttributeTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;
	uint16 classId = 0;
	int i;
	uint16 attributeInstancesNum = 0;
	uint8 attributeNum = 0;

	int tableSize = 0;
	int ret = 0;

	omciManageEntity_ptr currentME;
	omciAttriDescript_ptr currentAttribute;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	classId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(classId);
	attributeNum = omciGetAttriNumOfMe(currentME);

	/*1.calculate the table size*/
	tableSize = 2 *(attributeNum-1);

	/*2.malloc memory for table*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getAttributeTableValue: tableSize = %x\n", tableSize);
	tableBuffer = osMalloc(tableSize);
	if (tableBuffer == NULL)
		return -1;
	pTableBuffer = tableBuffer;

	/*3.get  the table copy */
	for(i = 1; i < attributeNum; i++) /*me id for every me is not considered */
	{
		currentAttribute = &currentME->omciAttriDescriptList[i];
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getAttributeTableValue: currentAttribute->attriName = %s\n", currentAttribute->attriName);
		if( (attributeInstancesNum = attriubteIsGeneral((char*)currentAttribute->attriName)) == 0)
			attributeInstancesNum = getAttributeInstancesNum(classId, i);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getAttributeTableValue: attributeInstancesNum = %x\n", attributeInstancesNum);
		pTableBuffer = put16(pTableBuffer, attributeInstancesNum);
	}

	/*4. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, classId, tableBuffer, tableSize, omciAttribute->attriName);
	if (tableBuffer != NULL)
		osFree(tableBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}


	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
	
}



int32 getAlarmsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 classId;
	int ret = 0;
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;
	uint32 tableSize = 0;
	int i = 0;
	uint8 alarmId = 0;

	omciManageEntity_ptr currentME;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;


	classId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(classId);

	if (currentME->alarmIdMapTable != NULL)
	{
		/*1.calculate the table size*/
		for (i = 0; currentME->alarmIdMapTable[i].alarmId != 0; i++)
			;
		tableSize =  i;

		/*2.malloc memory for table*/
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getAlarmsTableValue: tableSize = %x\n", tableSize);
		tableBuffer = osMalloc(tableSize);
		if (tableBuffer == NULL)
			return -1;
		pTableBuffer = tableBuffer;

		/*3.get  the table copy */
		for (i = 0; currentME->alarmIdMapTable[i].alarmId != 0; i++)
		{
			alarmId = (uint8)currentME->alarmIdMapTable[i].alarmId;
			*pTableBuffer++ = alarmId;
		}

		/*4. add the table to omci archtecture*/
		ret = omciAddTableContent(tmpomciMeInst_p->classId, classId, tableBuffer, tableSize, omciAttribute->attriName);
		if (tableBuffer != NULL)
			osFree(tableBuffer);
		if (ret != 0)
		{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
			return -1;
		}
	}
	
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

int32 getAVCsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 classId;
	int ret = 0;
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;
	uint32 tableSize = 0;
	int i = 0;
	uint8 attributeNum = 0;

	omciManageEntity_ptr currentME = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	classId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(classId);
	attributeNum = omciGetAttriNumOfMe(currentME);

	/*1. calculate the table size*/
	for (i = 1; i < attributeNum; i++)
	{
		currentAttribute = &currentME->omciAttriDescriptList[i];
		if (currentAttribute->avcId != NO_AVC)
			tableSize++;
	}

	/*2. malloc memory for table*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getAVCsTableValue: tableSize = %x\n", tableSize);
	tableBuffer = osMalloc(tableSize);
	if (tableBuffer == NULL)
		return -1;
	pTableBuffer = tableBuffer;

	/*3.get  the table copy */
	for (i = 1; i < attributeNum; i++)
	{
		currentAttribute = &currentME->omciAttriDescriptList[i];
		if (currentAttribute->avcId != NO_AVC)
			*pTableBuffer++ = (uint8)currentAttribute->avcId;
	}

	/*4.add table to omci architecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, classId, tableBuffer, tableSize, omciAttribute->attriName);
	if (tableBuffer != NULL)
		osFree(tableBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}
		
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}


int32 getInstancesTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;

	int ret = 0;
	uint16 classId;
	uint16 instancesMeId = 0;
	int tableSize = 0;
	omciManageEntity_ptr currentME;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	classId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(classId);

	/*1. calculate the table size*/
	for(meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next) /*for every me is not considered */
		tableSize += 2;

	/*2. malloc memory for table*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getInstancesTableValue: tableSize = %x\n", tableSize);
	tableBuffer = osMalloc(tableSize);
	if (tableBuffer == NULL)
		return -1;

	/*3.get  the table copy */	
	pTableBuffer = tableBuffer;
	for(meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next) /*for every me is not considered */
	{
		instancesMeId = get16(meInstantCurPtr->attributeVlaue_ptr);
		pTableBuffer = put16(pTableBuffer, instancesMeId);
	}

	/*4.add table to omci architecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, classId, tableBuffer, tableSize, omciAttribute->attriName);
	if (tableBuffer != NULL)
		osFree(tableBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}
	
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

/*******************************************************************************************************************************
9.12.10 Attribute

********************************************************************************************************************************/
int omciMeInitForAttribute(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;

	return 0;	
}

int omciInternalCreateForAttribute(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	uint16 attributeClassId = 0;
	uint8 attributeIndex = 0;
	omciManageEntity_ptr currentME = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);


	attributeClassId = getClassIdFromAttributeInstancesNum(instanceId);
	attributeIndex = getAttributeFromAttributeInstancesNum(instanceId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attributeClassId= %x, attributeIndex = %x\n", attributeClassId, attributeIndex);
	if (attributeIndex > 17)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciCreateActionFor_Attribute: attributeIndex is error\n");
		goto fail;
	}
	currentME = omciGetMeByClassId(attributeClassId);
	currentAttribute = &currentME->omciAttriDescriptList[attributeIndex];

/*0---meId */
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;

	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;
/*1---Name */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);

	if (attributeValuePtr != NULL)
	{
		memset(attributeValuePtr, 0, 25);
		memcpy(attributeValuePtr, currentAttribute->attriName, 25);
	}
	else
		goto fail;

/*2---Size */
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put32(attributeValuePtr, currentAttribute->attriLen);
	}
	else
		goto fail;

/*3---Access */
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);	
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = currentAttribute->attriAccess;
	}
	else
		goto fail;

/*4---Format */
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = currentAttribute->attriAccess;
	}
	else
		goto fail;

/*5---Lower limit */
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put32(attributeValuePtr, currentAttribute->lowerLimit);
	}
	else
		goto fail;

/*6---Upper limit */
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put32(attributeValuePtr, currentAttribute->uppererLimit);
	}
	else
		goto fail;

/*7---Bit Field */
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put32(attributeValuePtr, currentAttribute->bitField);
	}
	else
		goto fail;
	

/*9---Support */
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = currentAttribute->support;
	}
	else
		goto fail;


	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	return 0;
fail:
	omciFreeInstance(tmpomciMeInst_p);
	return -1;
}

int32 getCodePointsTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint16 classId = 0;
	uint16 instancesMeId = 0;
	uint16 attributeIndex = 0;
	int ret = 0;
	uint8 *tableBuffer = NULL;
	uint8 *pTableBuffer = NULL;
	uint32 tableSize = 0;
	int i = 0;

	omciManageEntity_ptr currentME = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;


	instancesMeId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	classId =  getClassIdFromAttributeInstancesNum(instancesMeId);
	attributeIndex = getAttributeFromAttributeInstancesNum(instancesMeId);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreateActionFor_Attribute: attributeClassId= %x, attributeIndex = %x\n", classId, attributeIndex);
	if (attributeIndex > 17)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciCreateActionFor_Attribute: attributeIndex is error\n");
		return -1;
	}
	currentME = omciGetMeByClassId(classId);
	currentAttribute = &currentME->omciAttriDescriptList[attributeIndex];

	if (currentAttribute->codePointTbl != NULL)
	{
		/*1. calculate the table size*/
		for (i = 0; currentAttribute->codePointTbl[i] != CODE_POINT_END; i++)
			;
		tableSize = 2 * i;

		/*2. malloc memory for table*/
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getCodePointsTableValue: tableSize = %x\n", tableSize);
		tableBuffer = osMalloc(tableSize);
		if (tableBuffer == NULL)
			return -1;
		pTableBuffer = tableBuffer;
		
		/*3.get  the table copy */	
		for (i = 0; currentAttribute->codePointTbl[i] != CODE_POINT_END; i++)
			pTableBuffer = put16(pTableBuffer, currentAttribute->codePointTbl[i]);

		/*4. add the table to omci archtecture*/
		ret = omciAddTableContent(tmpomciMeInst_p->classId, instancesMeId, tableBuffer, tableSize, omciAttribute->attriName);
		if (tableBuffer != NULL)
			osFree(tableBuffer);
		if (ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciAddTableContent fail\n");
			return -1;
		}
	}
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}
/*******************************************************************************************************************************
end
9.12.10 Attribute

********************************************************************************************************************************/

/*******************************************************************************************************************************
9.12.11 Octet string 

********************************************************************************************************************************/
int omciMeInitForOctetString(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;		
}
/*******************************************************************************************************************************
9.12.12 General purpose buffer 

********************************************************************************************************************************/
int omciMeInitForGeneralBuffer(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;		
}

int omciMeCreateHandleForGeneralBuffer(omciManageEntity_t *me_p, omciMeInst_t *meInstant_ptr, uint8 *contentPtr)
{
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId =0;
	int result = 0; 
	int ret = 0;
	uint32 maxSize = 0;

	if (me_p == NULL || meInstant_ptr == NULL || contentPtr == NULL)
	{
		result = OMCI_CMD_ERROR;
		goto fail;
	}
	
	instanceId = get16(meInstant_ptr->attributeVlaue_ptr);
	currentAttribute = &me_p->omciAttriDescriptList[1];
	if(currentAttribute->support == OMCI_ME_UNSUPPORT)
	{
		ret = addOneBuffer(meInstant_ptr, MAX_GERNEL_BUFFER_SIZE);
		if (ret == -1)
		{
			result = OMCI_CMD_ERROR;
			goto fail;
		}
		
	}
	else
	{
		maxSize = get32(contentPtr);
		if (maxSize == 0)
		{
			ret = addOneBuffer(meInstant_ptr, MAX_GERNEL_BUFFER_SIZE);
			if (ret == -1)
			{
				result = OMCI_CMD_ERROR;
				goto fail;
			}
		}
		else
		{
			if (maxSize > MAX_GERNEL_BUFFER_SIZE)
			{
				result = OMCI_PARAM_ERROR;
				goto fail;
			}
			ret = addOneBuffer(meInstant_ptr, maxSize);
			if (ret == -1)
			{
				result = OMCI_PARAM_ERROR;
				goto fail;
			}
			put32(meInstant_ptr->attributeVlaue_ptr + 2, maxSize);
		}
	}

	return 0;
fail:
	omciDelInstance(me_p, meInstant_ptr);/*If the ONU cannot allocate enough memory to accommodate this size, it should deny the ME create action. look 9.12.12 Maxmium Size*/
	return result;			
}
int omciMeDeleteHandleForGeneralBuffer(omciMeInst_t *meInstant_ptr)
{
	return delOneBuffer(meInstant_ptr);
}

/*______________________________________________________________________________
**	addOneBuffer
**
**	descriptions:
**		add one buffer to bufferMng
**	parameters:
**		instaceId: general purpose buffer instance id
**		bufferSize: the new buffer size
**	return:
**		0: success
**		-1:fail
**______________________________________________________________________________
*/
int addOneBuffer(omciMeInst_t *meInstant_ptr, uint32 bufferSize)
{
	bufferMngType * pBufferMng = NULL;

	if (meInstant_ptr == NULL)
		return -1;

	meInstant_ptr->otherInfo = osMalloc(sizeof(bufferMngType));
	if (meInstant_ptr->otherInfo == NULL)
			return -1;
	pBufferMng = (bufferMngType *)meInstant_ptr->otherInfo;

		pBufferMng->bufferPoint = osMalloc(bufferSize);
		if (pBufferMng->bufferPoint == NULL)
		{
			osFree(pBufferMng);
			return -1;
		}
	return 0;
}
/*______________________________________________________________________________
**	delOneBuffer
**
**	descriptions:
**		delete one buffer from bufferMng
**	parameters:
**		instaceId: general purpose buffer instance id
**	return:
**		0: success
**		-1:fail
**______________________________________________________________________________
*/
int delOneBuffer(omciMeInst_t *meInstant_ptr)
{
	bufferMngType * pBufferMng = NULL;

	if (meInstant_ptr == NULL)
		return -1;

	pBufferMng = (bufferMngType * )meInstant_ptr->otherInfo;
	if (pBufferMng == NULL)
		return -1;

	osFree(pBufferMng->bufferPoint);
	osFree(pBufferMng);
	return 0;
}

int32 setMaximumSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint32 maxSize = 0;
	uint16 instanceId = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	bufferMngType * pBufferMng = NULL;


	instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	pBufferMng = (bufferMngType *)tmpomciMeInst_p->otherInfo;;
	if (pBufferMng != NULL)
		osFree(pBufferMng->bufferPoint);
	maxSize = get32((uint8*)value);

	if (maxSize == 0)
	{
		pBufferMng->bufferPoint = osMalloc(MAX_GERNEL_BUFFER_SIZE);
		if (pBufferMng->bufferPoint == NULL)
			return -1;
		pBufferMng->bufferSize = MAX_GERNEL_BUFFER_SIZE;
	}
	else
	{
		if (maxSize > MAX_GERNEL_BUFFER_SIZE)
			return OMCI_PARAM_ERROR;
		pBufferMng->bufferPoint = osMalloc(maxSize);
		if (pBufferMng->bufferPoint == NULL)
			return -1;
		pBufferMng->bufferSize = maxSize;
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
		put32(attributeValuePtr, maxSize);
	}
	
	return 0;	
}

int32 getGeneralBufferTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instanceId = 0;
	bufferMngType * pBufferMng = NULL;
	uint32 tableSize = 0;
	uint8 *pTableBuffer = NULL;
	int ret = 0;

	instanceId= get16(tmpomciMeInst_p->attributeVlaue_ptr);
	pBufferMng = (bufferMngType *)tmpomciMeInst_p->otherInfo;

	if (pBufferMng == NULL)
		return -1;
	if (pBufferMng->bufferPoint == NULL)
		return -1;

	/*1.calculate the table size*/
	tableSize =  pBufferMng->actualSize;

	/*2.malloc memory for table*/
	pTableBuffer = pBufferMng->bufferPoint;

	/*3.get  the table copy */


	/*4. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, instanceId, pTableBuffer, tableSize, omciAttribute->attriName);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciAddTableContent fail\n");
		return -1;
	}
	
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

/*******************************************************************************************************************************
9.12.13 File transfer controller 

********************************************************************************************************************************/
int omciMeInitForFileTransController(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;

	return 0;		
}

typedef struct __fileTransferType
{
	uint8 *localFileName;
	uint8 fileType;
	uint8 *networkAddress;
}fileTransferType;

fileTransferType fileTransferMng;

int omciInternalCreateForFileTransController(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;	

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER only a instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	fileTransferMng.localFileName = NULL;
	fileTransferMng.networkAddress = NULL;
	fileTransferMng.fileType = 0;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);
	else
		goto fail;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 2;
	else
		goto fail;
	
	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForFileTransController: attributeValuePtr == NULL\n");
	return -1;
}

int omciInitInstForFileTransController(void)
{
	int ret = 0;

	ret = omciInternalCreateForFileTransController(OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER,  0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_FILE_TRANSFER_CONTROLLER fail\n");

	return ret;
}


int32 setFileTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 fileType = 0;

	fileType = get16((uint8*)value);

	if (fileType == 256)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "software image download\n");
		fileTransferMng.fileType = 1;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "unknown file type\n");
		return -1;
	}
	
	return setGeneralValue(value, meInstantPtr, omciAttribute,flag);
}
int32 setFileInstanceValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 fileInstance = 0;

	fileInstance = get16((uint8*)value);

	if (fileInstance == 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "software image download\n");
		fileTransferMng.fileType = 1;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "unknown file type\n");
		return -1;
	}
	
	return setGeneralValue(value, meInstantPtr, omciAttribute,flag);
}
int32 setLocalFileNameValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 largeStringMeId = 0;
	uint16 length = 0;
	char buffer[MAX_STRING_SIZE] = {0};
	
	largeStringMeId = get16((uint8*)value);

	if (largeStringMeId == 0 || largeStringMeId == 0xFFFF)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "the local File Name is NULL\n");
	}
	else
	{
		if (getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, largeStringMeId, buffer, &length) == 0)
		{
			osFree(fileTransferMng.localFileName);
			fileTransferMng.localFileName = osMalloc(length);
			memcpy(fileTransferMng.localFileName, buffer, length);
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "fileTransferMng.localFileName = %s, length = %x\n", fileTransferMng.localFileName, length);
		}
		else
		{
			return -1;
		}
	}
	
	return setGeneralValue(value, meInstantPtr, omciAttribute,flag);
}

int32 setNetworkAddressValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 networkAddressMeId = 0;
	omciManageEntity_ptr currentME;
	omciMeInst_t *currentInstantPtr;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 numOfParts = 0;
	uint16 largeStringMeId = 0;
	char buffer[MAX_STRING_SIZE] = {0};

	networkAddressMeId = get16((uint8*)value);

	if ((networkAddressMeId == 0) || (networkAddressMeId == 0xFFFF))
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "the local File Name is NULL\n");
	}
	else
	{
		currentME = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_ADDRESS);
		currentInstantPtr = omciGetInstanceByMeId(currentME, networkAddressMeId);
		attributeValuePtr = omciGetInstAttriByMask(currentInstantPtr, 1<<14, &length);
		if (attributeValuePtr != NULL)
			largeStringMeId = get16(attributeValuePtr);
		else 
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setNetworkAddressValue: attributeValuePtr == NULL\n");
			return -1;
		}
		
		if (getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, largeStringMeId, buffer, &length) == 0)
		{
			osFree(fileTransferMng.networkAddress);
			fileTransferMng.networkAddress = osMalloc(length + 1);
			memcpy(fileTransferMng.networkAddress, buffer, length);
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "fileTransferMng.networkAddress = %s, length = %x\n", fileTransferMng.networkAddress, length);
		}
		else
		{
			return -1;
		}
	}
	
	return setGeneralValue(value, meInstantPtr, omciAttribute,flag);
}


int32 setFileTransferTriggerValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 fileTransferTrigger = 0;
	char cmdBuffer[128] = {0};

	fileTransferTrigger = (uint8)*value;

	switch (fileTransferTrigger)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "fileTransferMng.localFileName = %s, fileTransferMng.networkAddress = %s\n", fileTransferMng.localFileName, fileTransferMng.networkAddress);
		case 1:
			if (fileTransferMng.networkAddress != NULL)
			{
				tcapi_set("OMCI_Entry", "TransferFlag", "1");
				if ( fileTransferMng.localFileName != NULL)
					sprintf(cmdBuffer, "tftp -g -r tclinux.bin -l /tmp/omci/%s %s &", fileTransferMng.localFileName, fileTransferMng.networkAddress);
				else
					sprintf(cmdBuffer, "tftp -g -r tclinux.bin -l /tmp/omci/tclinux.bin %s &",  fileTransferMng.networkAddress);
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "cmdBuffer = %s\n", cmdBuffer);
				system(cmdBuffer);
			}
			else
			{
				return -1;
			}
			break;
		case 2:
			if (fileTransferMng.localFileName != NULL)
			{
				tcapi_set("OMCI_Entry", "TransferFlag", "1");
				system("cd /tmp/omci");
				sprintf(cmdBuffer, "tftp -g  -l %s %s &", fileTransferMng.localFileName, fileTransferMng.networkAddress);
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "cmdBuffer = %s\n", cmdBuffer);
				system(cmdBuffer);
			}
			else
			{
				return -1;
			}
			break;
		case 3:
			system("killall -9 tftp");
			break;
		case 4:
			system("rm -rf /tmp/omci/tclinux.bin");
			break;
		case 5:
			break;
		default:
			break;
	}
	
	return setGeneralValue(value, meInstantPtr, omciAttribute,flag);
}

/*******************************************************************************************************************************
9.12.14 Generic status portal 

********************************************************************************************************************************/
int omciMeInitForGenericStatus(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;


	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	/* if need ONU auto create a ME instance, create here */
	return 0;		
}

int32 getStatusDocumentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	FILE *fp;
	uint32 fileSize = 0;
	uint32 tableSize = 0;
	uint8* tableBuffer = NULL;
	int ret = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instanceId = 0;

	instanceId= get16(tmpomciMeInst_p->attributeVlaue_ptr);	
	

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getStatusDocumentValue: 11");
	fp = fopen("/tmp/omci/cfgDocument.xml", "r");
//	fp = fopen("filename.xml", "r");
	if (fp == NULL)
		return -1;
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);

	/*1.calculate the table size*/
	tableSize = fileSize;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getStatusDocumentValue: tableSize = %x", tableSize);
	/*2.malloc memory for table*/
	tableBuffer = osMalloc(tableSize);

	/*3.get  the table copy */
	fseek(fp, 0L, SEEK_SET);
	fileSize = fread(tableBuffer, 1, tableSize, fp);

	fclose(fp);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getStatusDocumentValue: tableBuffer = %s", tableBuffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getStatusDocumentValue: tableSize = %x, fileSize = %x\n", tableSize, fileSize);
	if (fileSize != tableSize)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "fread fail\n");
		return -1;
	}

	/*4. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, instanceId, tableBuffer, tableSize, omciAttribute->attriName);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}
int32 getConfugurationdocumentValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	FILE *fp;
	uint32 fileSize = 0;
	uint32 tableSize = 0;
	uint8* tableBuffer = NULL;
	int ret = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instanceId = 0;

	instanceId= get16(tmpomciMeInst_p->attributeVlaue_ptr);	
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getStatusDocumentValue: 11");
	fp = fopen("/tmp/omci/cfgDocument.xml", "r");
	if (fp == NULL)
		return -1;
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	
	/*1.calculate the table size*/
	tableSize = fileSize;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "getConfugurationdocumentValue: tableSize = %x", tableSize);
	/*2.malloc memory for table*/
	tableBuffer = osMalloc(tableSize);

	/*3.get  the table copy */
	fseek(fp, 0L, SEEK_SET);
	fileSize = fread(tableBuffer, 1, tableSize, fp);
	fclose(fp);
	
	if (fileSize != tableSize)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "fread fail\n");
		return -1;
	}

	/*4. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_p->classId, instanceId, tableBuffer, tableSize, omciAttribute->attriName);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciAddTableContent fail\n");
		return -1;
	}
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}
/* omciAVCEnableTmOut
 meId = class ID<<16 | inst ID
 
 */

int omciAVCEnableTmOut(uint32 meId){
	omciManageEntity_t *me_p;
	uint16 classId = get16((uint8*)&meId);
	uint16 instId = get16((uint8*)&meId+2);
	omciMeInst_t *meInst;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nEnter omciAVCEnableTmOut meId = %x", meId);
	//find the instance
	me_p = omciGetMeByClassId(classId);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nomciAVCEnableTmOut can not find the me class!");
		return -1;
	}
	
	pthread_mutex_lock(&(me_p->omciInst.omciInstLock));
	//change the arc value of me
	meInst = omciGetInstanceByMeId(me_p , instId);
	if(meInst == NULL){
		pthread_mutex_unlock(&(me_p->omciInst.omciInstLock));	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nomciArcTmOut can not find the me inst!");
		return -1;
	}
	
	
	meInst->arcState = OMCI_ALM;
	pthread_mutex_unlock(&(me_p->omciInst.omciInstLock));
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nEnd omciAVCEnableTmOut ");
	return 0;
}
int32 setAVCReportRateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 AVCReportRate = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	
	AVCReportRate = *value;
	if (AVCReportRate == 0)
	{
		tmpomciMeInst_p->arcState = OMCI_NALM;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setAVCReportRateValue: AVCReportRate = 0\n");
		timerPeriodicStop(tmpomciMeInst_p->arcTm);
	}
	else if (AVCReportRate == 1)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setAVCReportRateValue: AVCReportRate = 1\n");
		tmpomciMeInst_p->interval = 60;
		tmpomciMeInst_p->arcState = OMCI_NALM;
		timerPeriodicStop(tmpomciMeInst_p->arcTm);
		tmpomciMeInst_p->arcTm = timerPeriodicStartS(60000*10, omciAVCEnableTmOut, ((tmpomciMeInst_p->classId)<<16)|(get16(tmpomciMeInst_p->attributeVlaue_ptr)));
		if (tmpomciMeInst_p->arcTm == -1)
			return -1;
	}
	else if (AVCReportRate == 2)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setAVCReportRateValue: AVCReportRate = 2\n");
		tmpomciMeInst_p->interval = 1;
		tmpomciMeInst_p->arcState = OMCI_NALM;
		timerPeriodicStop(tmpomciMeInst_p->arcTm);
		tmpomciMeInst_p->arcTm = timerPeriodicStartS(1000, omciAVCEnableTmOut, ((tmpomciMeInst_p->classId)<<16)|(get16(tmpomciMeInst_p->attributeVlaue_ptr)));
		if (tmpomciMeInst_p->arcTm == -1)
			return -1;
	}
	else if (AVCReportRate == 3)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setAVCReportRateValue: AVCReportRate = 3\n");
		/* stop pre timer */
		tmpomciMeInst_p->interval = 255;
		tmpomciMeInst_p->arcState = OMCI_ALM;
		timerPeriodicStop(tmpomciMeInst_p->arcTm);	
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "unknown value\n");
		return -1;
	}
	return 0;
}


#ifdef TCSUPPORT_SNMP
/*******************************************************************************************************************************
9.12.15 SNMP configuration data 

********************************************************************************************************************************/
int32 setSnmpVersionValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/

	
	return 0;
}

int32 setSnmpAgentIpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;



	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/
	/*Agent IP: not used now*/

	
	return 0;
}

int32 setSnmpServerIpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;



	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/

	
	return 0;
}

int32 setSnmpServerPortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;



	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/
	/*Server Port: fixed now(SNMP_PORT 161)*/

	
	return 0;
}	

int32 setSnmpReadCommunityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	char EntryName[32]={0};
	char stringBuf[256]={0};
	uint16 instanceId=0;
	int ret=0;
	uint16 stringLength = 0;

	/*get string from large string instance*/
	instanceId = get16((uint8 *)value);
//	ret = getStringFromInstance(instanceId, stringBuf);
	ret = getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, instanceId, (uint8 *)stringBuf, &stringLength);	
	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nsetSNMPReadCommunityValue:---get string from instance fail!!!");
		return -1;
	}
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);	
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/
	memset(EntryName,0, sizeof(EntryName));
	sprintf(EntryName, "Snmpd_Entry");
	tcapi_set(EntryName, "rocommunity", stringBuf);
	tcapi_commit(EntryName);
	tcapi_save();
	
	return 0;
}

int32 setSnmpWriteCommunityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	char EntryName[32]={0};
	char stringBuf[256]={0};
	uint16 instanceId=0;
	int ret=0;
	uint16 stringLength = 0;


	/*get string from large string instance*/
	instanceId = get16((uint8 *)value);
//	ret = getStringFromInstance(instanceId, stringBuf);
	ret = getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, instanceId, (uint8 *)stringBuf, &stringLength);	
	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nsetSNMPWriteCommunityValue:---get string from instance fail!!!");
		return -1;
	}
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);	
	memcpy(attributeValuePtr, value, length);
	
	/*set the value to SNMP app*/
	memset(EntryName,0, sizeof(EntryName));
	sprintf(EntryName, "Snmpd_Entry");
	tcapi_set(EntryName, "rwcommunity", stringBuf);
	tcapi_commit(EntryName);
	tcapi_save();	
	
	return 0;
}

int32 setSnmpSysNameValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	char EntryName[32]={0};
	char stringBuf[256]={0};
	uint16 instanceId=0;
	int ret=0;
	uint16 stringLength = 0;	


	/*get string from large string instance*/
	instanceId = get16((uint8 *)value);
//	ret = getStringFromInstance(instanceId, stringBuf);
	ret = getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, instanceId, (uint8 *)stringBuf, &stringLength);	
	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nsetSNMPSysNameValue:---get string from instance fail!!!");
		return -1;
	}
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);	
	memcpy(attributeValuePtr, value, length);

	/*set the value to SNMP app*/
	memset(EntryName,0, sizeof(EntryName));
	sprintf(EntryName, "Snmpd_Entry");
	tcapi_set(EntryName, "sysName", stringBuf);
	tcapi_commit(EntryName);
	tcapi_save();

	return 0;
}

int omciCreateActionForSnmpConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0;
	char EntryName[32]={0};
	
	ret = omciCreateAction(meClassId , meInstant_ptr, omciPayLoad, msgSrc);

	if(ret == 0)
	{
		/*active SNMP function*/
		memset(EntryName,0, sizeof(EntryName));
		sprintf(EntryName, "Snmpd_Entry");
		tcapi_set(EntryName, "active", "Yes");
		tcapi_commit(EntryName);
		tcapi_save();		
	}

	return ret;
}

int omciDeleteActionForSnmpConfigData(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0;
	char EntryName[32]={0};	

	ret = omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);
	if(ret == 0)
	{
		/*active SNMP function*/
		memset(EntryName,0, sizeof(EntryName));
		sprintf(EntryName, "Snmpd_Entry");
		tcapi_set(EntryName, "active", "No");
		tcapi_commit(EntryName);
		tcapi_save();		
	}

	return ret;
}

int omciMeInitForSnmpConfigData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;


	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateActionForSnmpConfigData;//omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteActionForSnmpConfigData;//omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;		
}
#endif

#ifdef TCSUPPORT_CUC
int omciInitInstForCucOnuCapability()
{
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    omciMeInst_t *tmpomciMeInst_p = NULL;
    uint8 * attributeValuePtr = NULL;
    uint16 attriMask = 0;
    uint16 length = 0;
    int tmp;
    char buf[16];

    tmpOmciManageEntity_p = omciGetMeByClassId(OMCI_CLASS_ID_ONU_CAPABILITY);
    if ( NULL == tmpOmciManageEntity_p ){
        return -1;
    }
    
    tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
    if ( NULL == tmpomciMeInst_p ){
        return -1;
    }
    
    omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
    
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);

    /* Operator ID  */
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL){
	    strcpy(attributeValuePtr, "CUC");
	}else{
	    goto fail;
	}
	
    /* CUC Spec Ver */
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL){
	    if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "CucOmciVer", buf)){
            buf[7] = 0;
            if(0 == strcmp(buf,"3.0")){
                *attributeValuePtr = 0;
            }else if(0 == strcmp(buf,"4.0")){
                *attributeValuePtr = 1;
            }else{
                *attributeValuePtr = 0;
            }
        }else{
            *attributeValuePtr = 0;
        }
    }else{
		goto fail;
	}

    /* ONU Type */
    attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL){
        if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "OnuType", buf)){
            buf[7] = 0;
            tmp = atoi(buf);
            if(tmp <= 0 || tmp > 2){
                tmp = 1;
            }
        }else{
            tmp = 1;
        }
        *attributeValuePtr = tmp - 1;	
	}else{
		goto fail;
	}

    /* Tx Power Supply */
    attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL){
        if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "TransceiverPowCtrl", buf)){
            buf[7] = 0;
            tmp = atoi(buf);
            if(tmp < 0 || tmp > 2){
                tmp = 1;
            }
        }else{
            tmp = 1;
        }
    	*attributeValuePtr = tmp;
	}else{
		goto fail;
	}

    return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL \n");
	return -1;
}

int omciMeInitForCucOnuCapability(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	
	return 0;		
}

int32 getCucOnuCapOperatorID(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
		
    memcpy(value, "CUC", 4);
    return 0;
}

int32 getCucOnuCapSpecVer(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

    char buf[8];
    memset(buf, 0, 8);
    
    if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "CucOmciVer", buf)){
        buf[7] = 0;
        if(0 == strcmp(buf,"3.0")){
            *value = 0;
        }else 
        if(0 == strcmp(buf,"4.0")){
            *value = 1;
        }else{
            return OMCI_CMD_ERROR;
        }
    }else{
        return OMCI_CMD_ERROR;
    }
	
    return 0;
}

int32 getCucOnuCapOnuType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

    char buf[8];
    memset(buf, 0, 8);
    int8 tmp;
    
    if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "OnuType", buf)){
        buf[7] = 0;
        tmp = atoi(buf);
        
        if(tmp <= 0 || tmp > 2){
            return OMCI_CMD_ERROR;
        }
    }else{
        return OMCI_CMD_ERROR;
    }

    *value = tmp - 1;
    return 0;
}

int32 getCucOnuCapOnuTxPowerCtrl(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

    char buf[8];
    memset(buf, 0, 8);
    int8 tmp;
    
    if(TCAPI_SUCCESS == tcapi_get("GPON_ONU", "TransceiverPowCtrl", buf)){
        buf[7] = 0;
        tmp = atoi(buf);
        
        if(tmp < 0 || tmp > 2){
            return OMCI_CMD_ERROR;
        }
    }else{
        return OMCI_CMD_ERROR;
    }

	*value = tmp;
    return 0;
}
#endif


#ifdef TCSUPPORT_CWMP
/*******************************************************************************************************************************
9.12.16 TR069 management server 

********************************************************************************************************************************/
int omciInitInstForTr069ManageServer(void)
{
	int ret = 0;
		
	ret = omciInternalCreateForTr069ManageServer(OMCI_CLASS_ID_TR069_MANAGE_SERVER, ((VEIP_SLOT<<8) | VEIP_INST_ID));

	return ret;
}

int omciInternalCreateForTr069ManageServer(uint16 classId, uint16 instanceId)
{	
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;	
	int ret = 0;
	
	omciPayload_t cmdMsg;
	uint8 *tmp = NULL;
	uint8 buffer[256] = {0};
	omciManageEntity_t *omciLargeStringEntity_p = NULL;
	uint16 largeStrId=0;	
	omciManageEntity_t *omciNetworkAddrEntity_p = NULL;
	uint16 networkAddrId=0;	
	omciManageEntity_t *omciAuthMethodEntity_p = NULL;
	uint16 authMethodId = 0;
	uint8 nameLen = 0;
	
	/*create and init LARGE STRING ME instance*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----create and init LARGE STRING ME instance");
	memset(&cmdMsg, 0, sizeof(cmdMsg));
	cmdMsg.devId = OMCI_BASELINE_MSG;
	tmp = (uint8 *)(&cmdMsg.meId);
	put16(tmp, OMCI_CLASS_ID_LARGE_STRING);  /*me class id*/
	omciLargeStringEntity_p = omciGetMeByClassId(OMCI_CLASS_ID_LARGE_STRING);
	if(omciLargeStringEntity_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----omciLargeStringEntity_p is NULL!\n");
		return -1;
	}
	largeStrId = omciGetMeOfInstanceId(omciLargeStringEntity_p,  0xfffe, 0x8000, -1);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInitInstForTr069ManageServer: ---OMCI_CLASS_ID_LARGE_STRING largeStrId = %d \n",largeStrId);
	put16(tmp+2, largeStrId);					 /*me instance id*/

	ret = omciLargeStringEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT](OMCI_CLASS_ID_LARGE_STRING,  NULL,  &cmdMsg , INTERNAL_MSG);/*internal create func*/
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciInitInstForTr069ManageServer: ---create OMCI_CLASS_ID_LARGE_STRING  fail!\n");	
		return ret;
	}
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("Cwmp_Entry", "acsUrl", (char *)buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----buffer = %s\n",buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----strlen(buffer) = %d\n",strlen((char *)buffer));

	ret = addOneStringToInstance(OMCI_CLASS_ID_LARGE_STRING, largeStrId, buffer, strlen((char *)buffer));
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nomciInitInstForTr069ManageServer:----setStringByInsatance fail!!!");
		return -1;
	}
	
// creat and init 	OMCI_CLASS_ID_AUTHENTICATION_METHOD instance
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----create and init OMCI_CLASS_ID_AUTHENTICATION_METHOD instance");
	memset(&cmdMsg, 0, sizeof(cmdMsg));
	cmdMsg.devId = OMCI_BASELINE_MSG;	
	tmp = (uint8 *)(&cmdMsg.meId);
	put16(tmp, OMCI_CLASS_ID_AUTHENTICATION_METHOD);  /*me class id*/	
	omciAuthMethodEntity_p = omciGetMeByClassId(OMCI_CLASS_ID_AUTHENTICATION_METHOD);
	if(omciAuthMethodEntity_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nomciInitInstForTr069ManageServer:---omciAuthMethodEntity_p == NULL");
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:---find ME success");
	}

	authMethodId = omciGetMeOfInstanceId(omciAuthMethodEntity_p, 1, 0xfffe, 1);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInitInstForTr069ManageServer: ---OMCI_CLASS_ID_AUTHENTICATION_METHOD authMethodId = %d \n",authMethodId);
	put16(tmp+2, authMethodId);					 /*me instance id*/	
	
	ret = omciAuthMethodEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT](OMCI_CLASS_ID_AUTHENTICATION_METHOD,  NULL,  &cmdMsg , INTERNAL_MSG);/*internal create func*/
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciInitInstForTr069ManageServer: ---create OMCI_CLASS_ID_AUTHENTICATION_METHOD fail!\n");	
		return ret;
	}
	
	tmpomciMeInst_p = omciGetInstanceByMeId(omciAuthMethodEntity_p, authMethodId);
	if(tmpomciMeInst_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstanceByMeId fail!\n");
		return -1;		
	}

	//init validation scheme
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<15), &length);
	if(attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstAttriByName fail!\n");
		return -1;		
	}
	memset(buffer, 0, sizeof(buffer));	
	tcapi_get("Cwmp_Entry", "authType", (char *)buffer);
	*attributeValuePtr = atoi((char *)buffer);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:---authType = %d\n", *attributeValuePtr);	

	//init validation username1 and username2
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<14), &length);
	if(attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstAttriByName fail!\n");
		return -1;		
	}
	memset(buffer, 0, sizeof(buffer));	
	tcapi_get("Cwmp_Entry", "acsUserName", (char *)buffer);
	memcpy(attributeValuePtr, buffer, length);
	nameLen = strlen((char *)buffer);
	if(nameLen > length)
	{
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<11), &length);
		if(attributeValuePtr == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstAttriByName fail!\n");
			return -1;		
		}
		memcpy(attributeValuePtr, (buffer+length), (nameLen-length));
	}

	//init  password
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<13), &length);
	if(attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstAttriByName fail!\n");
		return -1;		
	}
	memset(buffer, 0, sizeof(buffer));	
	tcapi_get("Cwmp_Entry", "acsPassword", (char *)buffer);	
	memcpy(attributeValuePtr, buffer, length);

	//init  realm
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<12), &length);
	if(attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstAttriByName fail!\n");
		return -1;		
	}
	memset(buffer, 0, sizeof(buffer));	
	tcapi_get("Cwmp_Entry", "realm", (char *)buffer);	
	memcpy(attributeValuePtr, buffer, length);	
	

	/*create and init network address ME instance*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----create and init network address ME instance");
	memset(&cmdMsg, 0, sizeof(cmdMsg));
	cmdMsg.devId = OMCI_BASELINE_MSG;	
	tmp = (uint8 *)(&cmdMsg.meId);
	put16(tmp, OMCI_CLASS_ID_NETWORK_ADDRESS);  /*me class id*/	
	omciNetworkAddrEntity_p = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_ADDRESS);
	if(omciNetworkAddrEntity_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nomciInitInstForTr069ManageServer:---omciNetworkAddrEntity_p == NULL");
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:---find ME success");
	}

//	networkAddrId = 100;//getMeNextInstanceId(omciNetworkAddrEntity_p);
	networkAddrId = omciGetMeOfInstanceId(omciNetworkAddrEntity_p, 0, 0x7fff, 1);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInitInstForTr069ManageServer: ---OMCI_CLASS_ID_NETWORK_ADDRESS networkAddrId = %d \n",networkAddrId);
	put16(tmp+2, networkAddrId);					 /*me instance id*/	
	
	ret = omciNetworkAddrEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT](OMCI_CLASS_ID_NETWORK_ADDRESS,  NULL,  &cmdMsg , INTERNAL_MSG);/*internal create func*/
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciInitInstForTr069ManageServer: ---create OMCI_CLASS_ID_NETWORK_ADDRESS fail!\n");	
		return ret;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----create OMCI_CLASS_ID_NETWORK_ADDRESS success");	
	
	tmpomciMeInst_p = omciGetInstanceByMeId(omciNetworkAddrEntity_p, networkAddrId);
	if(tmpomciMeInst_p == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForTr069ManageServer: omciGetInstanceByMeId fail!\n");
		return -1;		
	}

	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<15), &length);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, authMethodId);	
	
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<14), &length);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, largeStrId);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\nomciInitInstForTr069ManageServer:----init success");
	
	
	/* create  and init tr069 manage server ME instance*/
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciCreateActionFor_Software_image: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);

	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<15), &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInitInstForTr069ManageServer: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
	memset(buffer, 0, sizeof(buffer));
	tcapi_get("Cwmp_Entry", "Active", (char *)buffer);
	if(strcmp((char *)buffer, "Yes") == 0)
	{
		*attributeValuePtr =  0;
	}
	else
	{
		*attributeValuePtr =  1;
	}
	
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<14), &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInitInstForTr069ManageServer: tmpOmciManageEntity_p->currentME[2].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, networkAddrId);

	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, (1<<13), &length);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, 0xffff);

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	return 0;
}

int32 getTr069ManageAdmin(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char buffer[8] = {0};
	uint8 temp=0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTr069ManageAdmin: come into getTR069ManageAdmin\n");
	/*read from instance or from romfile*/	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);	
	if (attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTr069ManageAdmin: attributeValuePtr == NULL\n");
		return -1;
	}

	if(tcapi_get("Cwmp_Entry", "Active", buffer) != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTr069ManageAdmin: tcapi_get fail!!!\n");
		return -1;
	}

	if(strcmp(buffer, "Yes") == 0)
	{
		temp = 0;
	}
	else
	{
		temp = 1;
	}
	return getTheValue(value, (char *)(&temp), length, omciAttribute);	
}


int32 setTr069ManageAdmin(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	char EntryName[32]={0};

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	
#if !defined(TCSUPPORT_CT_PON)
	sprintf(EntryName,"Cwmp_Entry");
	if(*value == 0)
	{
		tcapi_set(EntryName, "Active", "Yes");
	}
	else
	{
		tcapi_set(EntryName, "Active", "No");
	}
	tcapi_commit(EntryName);
	tcapi_save();
#endif
	
	return 0;
}

int32 setTr069AcsAddr(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	uint8 buffer[32] = {0};
	uint16 instanceId=0;
	omciManageEntity_t *currentME=NULL;
	omciMeInst_t *currentInst_p=NULL;
	char EntryName[32] = {0};
	int ret=0;
	uint8 strBuf[256] = {0};
	uint8 userName1[32] = {0};
	uint8 userName2[32] = {0};
	uint8 userName[64] = {0};
	uint8 passWord[32] = {0};
	uint8 realm[32] = {0};
	uint8 authType = 0;//0:disable 1:md5 3:basic

	sprintf(EntryName,"Cwmp_Entry");
	if(tcapi_get(EntryName, "Active", (char *)buffer) != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: tcapi_get fail!!!\n");
		return -1;
	}

	if(strcmp((char *)buffer, "Yes") != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: ---fail--- tr069 is not active!!!\n");
		return -1;
	}



	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	
	/*get acs addr from network address instance*/
	instanceId =  get16(attributeValuePtr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_ADDRESS);
	currentInst_p = omciGetInstanceByMeId(currentME, instanceId);

	if (currentInst_p == NULL) {
		tcdbg_printf("\r\nsetTr069AcsAddr(): Get Network Address ME fail, currentInst_p is NULL.");
		return -1;
	}

	attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[2].attriName, &length);
	instanceId = get16(attributeValuePtr);

	//wait for andy's api
	ret = getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, instanceId, strBuf, &length);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: getOneStringFromInstance fail!\n");
		return -1;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---strBuf = %s\n", strBuf);
	
	
	//get auth information from OMCI_CLASS_ID_AUTHENTICATION_METHOD
	attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[1].attriName, &length);
	if(attributeValuePtr == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
		return -1;		
	}		
	instanceId = get16(attributeValuePtr);

	if((instanceId != 0) && (instanceId != 0xFFFF))
	{
		currentME = omciGetMeByClassId(OMCI_CLASS_ID_AUTHENTICATION_METHOD);
		if(currentME == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: could not find OMCI_CLASS_ID_AUTHENTICATION_METHOD!\n");
			return -1;
		}
		
		currentInst_p = omciGetInstanceByMeId(currentME, instanceId);
		if(currentInst_p == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstanceByMeId fail!\n");
			return -1;		
		}

		attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[1].attriName, &length);
		if(attributeValuePtr == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
			return -1;		
		}
		
		memcpy(&authType, attributeValuePtr, length);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---authType = %d\n", authType);	



		attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[2].attriName, &length);
		if(attributeValuePtr == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
			return -1;		
		}
		
		memcpy(userName1, attributeValuePtr, length);
		userName1[length] = '\0';
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---userName1 = %s\n", userName1);


		attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[5].attriName, &length);
		if(attributeValuePtr == NULL)
	{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
		return -1;
	}
		memcpy(userName2, attributeValuePtr, length);
		userName2[length] = '\0';	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---userName2 = %s\n", userName2);

		if((strlen((char *)userName1) == 25) && (strlen((char *)userName2) != 0))
	{
			sprintf((char *)userName, "%s%s", (char *)userName1, (char *)userName2);
	}
	else
	{
			sprintf((char *)userName, "%s", (char *)userName1);
		}
		
		attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[3].attriName, &length);
		if(attributeValuePtr == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
			return -1;		
		}	
		memcpy(passWord, attributeValuePtr, length);
		passWord[length] = '\0';	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---passWord = %s\n", passWord);

		attributeValuePtr = omciGetInstAttriByName(currentInst_p, currentME->omciAttriDescriptList[4].attriName, &length);
		if(attributeValuePtr == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setTr069AcsAddr: omciGetInstAttriByName fail!\n");
		return -1;
	}
		memcpy(realm, attributeValuePtr, length);
		realm[length] = '\0';			
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setTr069AcsAddr: ---realm = %s\n", realm);		

		tcapi_set(EntryName, "acsUserName", (char *)userName);
		tcapi_set(EntryName, "acsPassword", (char *)passWord);
		tcapi_set(EntryName, "authType", (char *)userName);
		tcapi_set(EntryName, "realm", (char *)passWord);
	}
	

	tcapi_set(EntryName, "acsUrl", (char *)strBuf);	
	delayOmciCommitNode(EntryName);
	
	return 0;
}

/*******************************************************************************************************************************
9.12.5 Large string 

********************************************************************************************************************************/    
int32 setLargeStringValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    uint8 *attributeValuePtr = NULL;
    uint16 length = 0;
    uint8 totalParts = 0;
    uint8 currPart = 0;
    omciMeInst_t * inMeInst_p = meInstantPtr;
    omciMeInst_t * networkAddressInstList[OMCI_GET_INSTANC_BY_ATTR_MAX_NUM] = {0};
    omciMeInst_t * TR069InstList[OMCI_GET_INSTANC_BY_ATTR_MAX_NUM] = {0};
    omciMeInst_t * meInst_p = NULL;
    uint16 instanceId = get16(inMeInst_p->attributeVlaue_ptr);
    omciManageEntity_ptr currentME = NULL;
    omciAttriDescript_t * currentAttribute = NULL;
    uint8 isLargeString = 1;
    uint8 i = 0;
    uint8 j = 0;
    
	if (NULL == value)
		return -1;

    setGeneralValue(value, meInstantPtr, omciAttribute, flag);

    attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr,"Number of parts", &length);
    if(NULL == attributeValuePtr)
        return 0;

    /* config TR-069 address */
    /* get the instance ID of ME:Network address*/
    omciGetInstByAttriValue(OMCI_CLASS_ID_NETWORK_ADDRESS , "Address pointer",  &instanceId, networkAddressInstList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
    for (i = 0; i <= OMCI_GET_INSTANC_BY_ATTR_MAX_NUM; i++)
    {
        meInst_p = networkAddressInstList[i];
        if (NULL == meInst_p)
            break;
        
        /*get the instance ID of ME:TR-069 managerment server*/
        instanceId = get16(meInst_p->attributeVlaue_ptr);
        omciGetInstByAttriValue(OMCI_CLASS_ID_TR069_MANAGE_SERVER , "ACS network address",  &instanceId, TR069InstList, OMCI_GET_INSTANC_BY_ATTR_MAX_NUM);
        for (j = 0; j <= OMCI_GET_INSTANC_BY_ATTR_MAX_NUM; j++)
        {
           meInst_p = TR069InstList[j];
            if (NULL == meInst_p)
                break;
        
            /* get the related ME:TR-069 managerment server, set new ACS address*/
            currentME = omciGetMeByClassId(OMCI_CLASS_ID_TR069_MANAGE_SERVER);
            currentAttribute = & currentME->omciAttriDescriptList[2];
            setTr069AcsAddr(&instanceId, meInst_p, currentAttribute, flag);
        }
    }

    return 0;
}

extern char tr69wan[];
extern int setWanInfo(int type ,uint16 IPHostIstId, char* buf);

int32 setTr069AssociateTag(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length=0;
	char EntryName[32]={0};
    omciMeInst_t * MeInst_p = meInstantPtr;
    uint16 instanceId = get16(MeInst_p->attributeVlaue_ptr);
    uint16 ipHostIstId = 0; /* default ip host config data instance id is 0*/
    uint16 TCP_UDPIstId = 0;

	char value1[10] = {0}, value2[10] = {0};
	uint16 vlanid_prio = 0, vlanid = 0, prio = 0;
    omciManageEntity_ptr currentME = NULL;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	
	vlanid_prio = get16(attributeValuePtr);
	vlanid = (vlanid_prio & 0xfff);
	prio = ((vlanid_prio & 0xe000)>>13);

	sprintf(value1, "%d", vlanid);
	sprintf(value2, "%d", prio);

    /* TR069 management server --> veip -> TCP/UDP config data -> IPHOST*/
    
    /* get VEIP by TR069 instance id */
    currentME = omciGetMeByClassId(OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT);
    MeInst_p = omciGetInstanceByMeId(currentME , instanceId);
    if(NULL != MeInst_p)
    {
        /* get TCP/UDP config data */
        attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)MeInst_p , "tcp/udp pointer", &length);
        TCP_UDPIstId =  get16(attributeValuePtr);
        currentME = omciGetMeByClassId(OMCI_CLASS_ID_TCP_UDP_CFGDATA);
        MeInst_p = omciGetInstanceByMeId(currentME , TCP_UDPIstId);

        if(NULL != MeInst_p)
        {
            /* get iphost instance*/
            attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)MeInst_p , "ip host pointer", &length);
            ipHostIstId =  get16(attributeValuePtr);
        }
    }
#if defined(TCSUPPORT_PON_IP_HOST)
	/* update vlanid and dot1p to tr69 wan connection */
	setWanInfo(WAN_IF_TYPE_VLANID, ipHostIstId,value1); /* set vlan id */
	setWanInfo(WAN_IF_TYPE_DOT1P, ipHostIstId,value2); /* set dot1p */
#endif

	if(delayOmciCommitNode(tr69wan) !=0){
		OMCI_ERROR("addOmciCommitNodeList and start the Timer fail\n");
		return -1;	
	}

	return 0;
}


int omciMeInitForTr069ManageServer(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;		
}
#endif

#ifdef TCSUPPORT_VNPTT
int omciInitInstForRsv347ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 347;
	uint16 instanceId = 2;
	uint16 attriMask = 0;
	uint16 length = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

    attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	
    uint8 tmp_1[1] = {0x0};
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, tmp_1, length);
	}

    attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	
    uint8 tmp_2[6] = {0xa0,0x65,0x18,0x13,0xea,0x1b};
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, tmp_2, length);
	}

	return 0;
}
#endif

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME
int omciInitInstForRsv350ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 350;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv352ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 352;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv353ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 353;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv367ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 367;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv373ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 373;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

#ifdef TCSUPPORT_HUAWEI_OLT_VENDOR_SPECIFIC_ME_FOR_INA
int omciInitInstForRsv370ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 370;
	uint16 instanceId = 257;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv65408ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 65408;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv65414ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 65414;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}

int omciInitInstForRsv65425ME()
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 classId = 65425;
	uint16 instanceId = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForOnuG: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	return 0;
}
#endif
#endif

