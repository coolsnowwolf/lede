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
	enhanced_security_ctl_me_func.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lisa.xue	2012/7/28	Create
*/
#include <stdio.h>
#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_crypt_aes.h"
#include "omci_me_enhanced_security_ctl.h"

/*******************************************************************************************************************************
									C O N S T A N T S
										
********************************************************************************************************************************/

#define OLT_TABLE_ENTRY_LEN 17
#define ONU_TABLE_ENTRY_LEN 16
#define MASTER_SESSION_KEY_LEN 16
#define PRESHARED_KEY_LEN 16
#define PEER_IDENTITY_LEN 16
#define ONU_AUTHENTICATION_STATUS_S0 0
#define ONU_AUTHENTICATION_STATUS_S1 1
#define ONU_AUTHENTICATION_STATUS_S2 2
#define ONU_AUTHENTICATION_STATUS_S3 3
#define ONU_AUTHENTICATION_STATUS_S4 4
#define ONU_AUTHENTICATION_STATUS_S5 5

/*******************************************************************************************************************************
							F U N C T I O N   D E C L A R A T I O N S
										
********************************************************************************************************************************/

int omciCreatActionForEnhancedSecurityCtl(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciInternalCreatActionForEnhancedSecurityCtl(uint16 classId, uint16 instanceId);
uint32 getTableAttributeValue(omciMeInst_t *omciMeInst_ptr, uint8 attriIndex, omciAttriDescript_ptr omciAttribute);
void setONUAuthenticationState(omciMeInst_t *meInstantPtr, uint8 curState);
int updateONUAuthenticationState(omciMeInst_t *meInstantPtr);
int generateONURandomChallengeTable(omciMeInst_t *meInstantPtr);
int generateONUAuthenticationResultTable(omciMeInst_t *meInstantPtr);
int generateMasterSessionKey(omciMeInst_t *meInstantPtr, int flag);
int generateMasterSessionKeyName(omciMeInst_t *meInstantPtr, int flag);
int getAuthenticationResult(uint8 *meInstantPtr);

extern int sendOmciNotifyMsg(notify_msg_t *buf);

/*******************************************************************************************************************************
							9.13.11: Enhanced security control

********************************************************************************************************************************/

int omciMeInitForEnhancedSecurityCtl(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForEnhancedSecurityCtl \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreatActionForEnhancedSecurityCtl;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciCreatActionForEnhancedSecurityCtl(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint16 classId = 0;
	uint16 instanceId = 0;

	if(omciPayLoad == NULL)
		return -1;
	
	if(msgSrc == INTERNAL_MSG)
	{
		classId = get16((uint8*)&(omciPayLoad->meId));
		instanceId = get16((uint8*)&(omciPayLoad->meId)+2);
		omciInternalCreatActionForEnhancedSecurityCtl(classId, instanceId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n omciCreatActionForEnhancedSecurityCtl: create success \r\n");
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciCreatActionForEnhancedSecurityCtl: fail, can be only created by ONU!\r\n");
		return -1;
	}

	return 0;
}

int omciInternalCreatActionForEnhancedSecurityCtl(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint8 buffer[20] = {0};
	uint16 length = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	tmpomciMeInst_p->deviceId = instanceId;//for receive message

	/*meId, there is only one instance, meId=0*/
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 16);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x1;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[8].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[10].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 16);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[12].attriName, (uint16 *)&length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0x80);

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
}

int omciInitInstForEnhancedSecurityCtl(void)
{
	omciManageEntity_ptr currentME = NULL;
	int ret = 0;
	omciPayload_t cmdMsg;
	uint8 *tmp = NULL;

	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	if(currentME== NULL)
		return -1;
	memset(&cmdMsg, 0, sizeof(cmdMsg));	
	cmdMsg.devId = OMCI_BASELINE_MSG;
	tmp = (uint8 *)&cmdMsg.meId;
	put16(tmp, OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);    /*me class id*/
	put16(tmp+2, 0x0);    /*me instance id*/
	ret = currentME->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT](OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL,  NULL,  &cmdMsg , INTERNAL_MSG);   //internal create func	
	if (ret == -1)		
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInitInst create OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL fail \r\n");

	return ret;
}

uint32 getTableAttributeValue(omciMeInst_t *omciMeInst_ptr, uint8 attriIndex, omciAttriDescript_ptr omciAttribute)
{
	omciTableAttriValue_t *tmp_ptr = NULL;	
	char *tableBuffer = NULL;	
	uint32 tableSize = 0;
	int ret = 0;	
	uint16 instanceId = 0;	

	if((omciMeInst_ptr == NULL) || (omciAttribute == NULL))
		return 0;
	
	tmp_ptr = omciGetTableValueByIndex(omciMeInst_ptr, attriIndex);	
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
	instanceId = get16(omciMeInst_ptr->attributeVlaue_ptr);
	ret = omciAddTableContent(omciMeInst_ptr->classId, instanceId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getTB: omciAddTableContent fail \r\n");		
		return 0;	
	}

	return tableSize;
}

void setONUAuthenticationState(omciMeInst_t *meInstantPtr, uint8 newStatus)
{
	uint8 *attributeValuePtr = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	notify_msg_t msg;
	uint8 curStatus = 0;

	if(meInstantPtr == NULL)
		return;
	
	instanceId = get16(meInstantPtr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	attributeValuePtr = omciGetInstAttriByName(meInstantPtr , currentAttribute->attriName, (uint16 *)&length);
	curStatus = *attributeValuePtr;
	*attributeValuePtr = newStatus;

	if(curStatus != newStatus)
	{
		/*deal with ONU authentication status's AVC */
		memset(&msg, 0, sizeof(msg));
		msg.notifyType = OMCI_NOTIFY_TYPE_AVC;
		msg.avcId = OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_STATUS;
		
		if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n avcMsgHandle: sendOmciNotifyMsg fail!\r\n");

		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n avcMsgHandle: sendOmciNotifyMsg OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_STATUS!\r\n");
	
	}

	return;
}

int updateONUAuthenticationState(omciMeInst_t *meInstantPtr)
{
	uint8 *attributeValuePtr = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint8 curStatus = 0;

	if(meInstantPtr == NULL)
		return -1;
	
	instanceId = get16(meInstantPtr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	attributeValuePtr = omciGetInstAttriByName(meInstantPtr , currentAttribute->attriName, (uint16 *)&length);
	curStatus = *attributeValuePtr;

	if(curStatus == ONU_AUTHENTICATION_STATUS_S2){
		setONUAuthenticationState(meInstantPtr, ONU_AUTHENTICATION_STATUS_S5); 
		if(timerStartS(1000, (TimerCallback)updateONUAuthenticationState, meInstantPtr) == -1)
			return -1;
	}
	else if((curStatus == ONU_AUTHENTICATION_STATUS_S4) || (curStatus == ONU_AUTHENTICATION_STATUS_S5))
		setONUAuthenticationState(meInstantPtr, ONU_AUTHENTICATION_STATUS_S0);

	return 0;
}

int32 setEnhancedSecurityCtlOLTCryptoCapabilities(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *tmpValue_ptr = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	/*get ONU authentication status*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	if((*tmpValue_ptr) != ONU_AUTHENTICATION_STATUS_S0)
		return -1;
	tmpValue_ptr = NULL;

	/*set OLT challenge status to false*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[3];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	*tmpValue_ptr = 0;
	tmpValue_ptr = NULL;

	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, (uint16 *)&length);
	memcpy(tmpValue_ptr, value, length);

	return 0;
}

int32 getEnhancedSecurityCtlOLTRandomChallengeTb(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 tableSize = 0;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tableSize = getTableAttributeValue(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex, omciAttribute);

	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getOLTRandomChallengeTb->tableSize = %d",tableSize);

	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);	
}

int32 setEnhancedSecurityCtlOLTRandomChallengeTb(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciTableAttriValue_t *tableConent_ptr = NULL; 
	uint8 *tableValue_ptr = NULL;
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 index = 0;
	uint8 findFlag = 0;//0:not find, 1:find
	uint16 instanceId = 0;
	uint32 tableSize = 0;
	int ret = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	/*get ONU authentication status*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr, currentAttribute->attriName, (uint16 *)&length);
	if(*tmpValue_ptr != ONU_AUTHENTICATION_STATUS_S0)
		return -1;
	tmpValue_ptr = NULL;
	
	/*set OLT challenge status to false*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[3];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr, currentAttribute->attriName, (uint16 *)&length);
	*tmpValue_ptr = 0;
	tmpValue_ptr = NULL;
	
	index = *(uint8*)value;
	if(index != 0)/*add an entry*/
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setOLTRandomChallengeTb add an entry.");
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n OLTRandomChallengeTb is NULL and add first record");
			tableSize = 0;
			tableValue_ptr = NULL;
		}else{
			tableSize = tableConent_ptr->tableSize;
			tableValue_ptr = tableConent_ptr->tableValue_p;
		}
		tmp_ptr = tableValue_ptr;
		
		/*search current entry*/
		length = OLT_TABLE_ENTRY_LEN;
		while(tmp_ptr != NULL){
			if(index == *tmp_ptr){
				findFlag = 1;
				break;
			}
			length += OLT_TABLE_ENTRY_LEN;
			if(length > tableConent_ptr->tableSize){
				//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setOLTRandomChallengeTb->length > tableConent_ptr->tableSize");
				findFlag = 0;
				break;
			}
			tmp_ptr += OLT_TABLE_ENTRY_LEN;
		}

		if(findFlag == 1){
			/*if find, replace this entry*/
			memcpy(tmp_ptr, value, OLT_TABLE_ENTRY_LEN);
		}
		else
		{
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize+OLT_TABLE_ENTRY_LEN);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTRandomChallengeTb:calloc failure");
				ret = -1;
				goto end;		
			}
			memcpy(tmpValue_ptr, tableValue_ptr, tableSize);
			memcpy(tmpValue_ptr+tableSize, value, OLT_TABLE_ENTRY_LEN);		
			tableSize += OLT_TABLE_ENTRY_LEN;		
		}

		/*we need to update instance table list, because of new alloc*/
		if(findFlag == 0){
			if(tableConent_ptr != NULL){
				/*del table content  in instance table list*/
				ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTRandomChallengeTb: omciDelTableValueOfInst failure");
					ret = -1;
					goto end;
				}
			}

			tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex, tableSize, tmpValue_ptr);
			if(tableConent_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTRandomChallengeTb: omciAddTableValueToInst failure");
				ret = -1;
				goto end;	
			}
		}
	}
	else if(index == 0)/*clear this table*/
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setOLTRandomChallengeTb clear the table.");
		/*get table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
		if(tableConent_ptr == NULL){
			ret = 0;
			goto end;	
		}
		
		/*clean table in table list*/
		ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTRandomChallengeTb: omciDelTableValueOfInst failure");
			ret = -1;
			goto end;	
		}
		
	}
	ret = 0;
	
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}
	return ret;
}

int generateONURandomChallengeTable(omciMeInst_t *meInstantPtr)
{
	omciTableAttriValue_t *tableConent_ptr = NULL; 
	uint32 tableSize = 0;
	uint32 tmp_tableSize = 0;
	uint8 *tableValue_ptr = NULL; 
	uint8 *tmp_tableValue_ptr = NULL; 
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	uint8 index = 0;
	uint8 entryNumber = 0;
	uint16 length = 0;
	int ret = 0;

	if(meInstantPtr == NULL)
		return -1;
	
	/*if ONU random challenge already exist, we clean table in table list*/
	tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex);
	if(tableConent_ptr == NULL)
	{
		tableSize = 0;
		tableValue_ptr = NULL;
	}
	else
	{
		ret = omciDelTableValueOfInst(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONURandomChallengeTable: omciDelTableValueOfInst failure");
			ret = -1;
			goto end;	
		}
	}
	
	/*get OLT random challenge table*/
	tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
	if(tableConent_ptr == NULL)
	{
		tmp_tableSize = 0;
		tmp_tableValue_ptr = NULL;
		ret = -1;
		goto end;	
	}
	else
	{
		tmp_tableSize = tableConent_ptr->tableSize;
		tmp_tableValue_ptr = tableConent_ptr->tableValue_p;
	}
	tmp_ptr = tmp_tableValue_ptr;
	
	/*generate ONU random challenge table according to OLT random challenge table*/
	/*1. calculate ONU random challenge table size, and malloc new space*/
	entryNumber = tmp_tableSize/OLT_TABLE_ENTRY_LEN;
	tmpValue_ptr = calloc(1 , tableSize+ONU_TABLE_ENTRY_LEN*entryNumber);
	if(tmpValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONURandomChallengeTable: calloc failure");
		ret = -1;
		goto end;		
	}
	/*2.generate ONU random challenge table*/
	length = OLT_TABLE_ENTRY_LEN;
	while(index < entryNumber){
		memcpy(tmpValue_ptr+tableSize, tmp_ptr+1, ONU_TABLE_ENTRY_LEN);		
		tableSize += ONU_TABLE_ENTRY_LEN;
		length += OLT_TABLE_ENTRY_LEN;
		if(length > tmp_tableSize){
			//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n generateONURandomChallengeTable->length > tableConent_ptr->tableSize");
			break;
		}
		tmp_ptr += OLT_TABLE_ENTRY_LEN;
		index++;
	}
	
	/*we need to update instance table list, because of new alloc*/
	tableConent_ptr = omciAddTableValueToInst(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex, tableSize, tmpValue_ptr);
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONURandomChallengeTable: omciAddTableValueToInst failure");
		ret = -1;
		goto end;	
	}
	ret = 0;
	
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}
	return ret;
}

int generateONUAuthenticationResultTable(omciMeInst_t *meInstantPtr)
{
	uint8 index = 0;
	uint8 entryNumber = 0;
	uint16 instanceId = 0;
	uint8 *OltValue_ptr = NULL;
	uint8 *OnuValue_ptr = NULL;
	uint32 OltSize = 0;
	uint32 OnuSize = 0;
	uint8 *attributeValuePtr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint32 tableSize = 0;
	uint8 *tableValue_ptr = NULL; 
	omciTableAttriValue_t *tableConent_ptr = NULL; 
	uint8 *tmpValue_ptr = NULL;
	int ret = 0;
	uint8 MACText[MASTER_SESSION_KEY_LEN] = {0};
	uint32 MACTextLen = MASTER_SESSION_KEY_LEN;
	uint8 *plainText = NULL;
	uint32 plainTextLen = 0;
	uint8 identity1[PEER_IDENTITY_LEN] = {
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	uint8 psk[PRESHARED_KEY_LEN] = {0};
	int tmpPSK[PRESHARED_KEY_LEN] = {0};
	char buffer[52] =  {0};
	uint16 length = 0;

	if(meInstantPtr == NULL)
		return -1;
	
	/*get EnahncedPSK from romfile*/
	tcapi_get(GPON_ONU, ENHANCED_PSK, buffer);
	sscanf(buffer, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
		&tmpPSK[0], &tmpPSK[1], &tmpPSK[2], &tmpPSK[3], &tmpPSK[4], &tmpPSK[5], &tmpPSK[6], &tmpPSK[7], 
		&tmpPSK[8], &tmpPSK[9], &tmpPSK[10], &tmpPSK[11], &tmpPSK[12], &tmpPSK[13], &tmpPSK[14], &tmpPSK[15]);

	for(index=0; index<PRESHARED_KEY_LEN; index++){
		if((tmpPSK[index] >= 0) && (tmpPSK[index] <= 255))
			psk[index] = tmpPSK[index];
		else
			goto end;
	}
	
	/*if ONU authentication result table already exist, we clean table in table list*/
	tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[6].attriIndex);
	if(tableConent_ptr == NULL)
	{
		tableSize = 0;
		tableValue_ptr = NULL;
	}
	else
	{
		/*clean table in table list*/
		ret = omciDelTableValueOfInst(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[6].attriIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONUAuthenticationResultTable: omciDelTableValueOfInst failure");
			ret = -1;
			goto end;	
		}
	}

	/* get plain text for ONU authentication result table, '|' means concatenation
	* which is: ONU_selected_crypto_capabilities|OLT_random_challenge_table|ONU_random_challenge_tables|0x0000 0000 0000 0000
	*/
	/*1.get OLT random challenge table*/
	tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
	if(tableConent_ptr == NULL)
	{
		OltSize = 0;
		OltValue_ptr = NULL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n OLT random challenge table is NULL!\r\n");
		ret = -1;
		goto end;
	}
	else
	{
		OltSize = tableConent_ptr->tableSize;
		OltValue_ptr = tableConent_ptr->tableValue_p;
	}
	tableConent_ptr = NULL;
	/*2.get ONU random challenge table*/
	tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex);
	if(tableConent_ptr == NULL)
	{
		OnuSize = 0;
		OnuValue_ptr = NULL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n ONU random challenge table is NULL!\r\n");
		ret = -1;
		goto end;
	}
	else
	{
		OnuSize = tableConent_ptr->tableSize;
		OnuValue_ptr = tableConent_ptr->tableValue_p;
	}
	tableConent_ptr = NULL;
	entryNumber = OnuSize/ONU_TABLE_ENTRY_LEN;
	/*3.get ONU selected crypto*/
	instanceId = get16(meInstantPtr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[4];
	attributeValuePtr = omciGetInstAttriByName(meInstantPtr , currentAttribute->attriName, (uint16 *)&length);

	/*malloc space for ONU authentication result table*/
	tmpValue_ptr = calloc(1 , tableSize+MASTER_SESSION_KEY_LEN*entryNumber);
	if(tmpValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONUAuthenticationResultTable:calloc failure");
		ret = -1;
		goto end;		
	}

	/*generate palainText and ONU authentication result table */
	index = 0;
	plainTextLen = 1+(OLT_TABLE_ENTRY_LEN-1)+ONU_TABLE_ENTRY_LEN+PEER_IDENTITY_LEN;
	/*calloc space for palinText*/
	plainText = calloc(1, plainTextLen);
	if(plainText == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONUAuthenticationResultTable: calloc fail!\r\n");
		ret = -1;
		goto end;
	}
	while(index < entryNumber){
		/*generate plain text for ONU authentication result table*/
		*plainText = *attributeValuePtr;
		memcpy(plainText+1, OltValue_ptr+1, OLT_TABLE_ENTRY_LEN-1);
		memcpy(plainText+OLT_TABLE_ENTRY_LEN, OnuValue_ptr, ONU_TABLE_ENTRY_LEN);
		memcpy(plainText+OLT_TABLE_ENTRY_LEN+ONU_TABLE_ENTRY_LEN, identity1, PEER_IDENTITY_LEN);
		AES_CMAC(plainText, plainTextLen, psk, ONU_TABLE_ENTRY_LEN, MACText, (uint32 *)&MACTextLen);
		memcpy(tmpValue_ptr+tableSize, MACText, MASTER_SESSION_KEY_LEN);		
		tableSize += MASTER_SESSION_KEY_LEN;
		memset(MACText, 0, sizeof(MACText));
		OltValue_ptr += OLT_TABLE_ENTRY_LEN;
		OnuValue_ptr += ONU_TABLE_ENTRY_LEN;
		index++;
	}
	if(plainText != NULL){
		free(plainText);
		plainText = NULL;
	}
	
	/*update instance table list, because of new alloc*/
	tableConent_ptr = omciAddTableValueToInst(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[6].attriIndex, tableSize, tmpValue_ptr);
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n generateONUAuthenticationResultTable: omciAddTableValueToInst failure");
		ret = -1;
		goto end;
	}
	ret = 0;
	
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}
	if(plainText != NULL){
		free(plainText);
		plainText = NULL;
	}
	return ret;
}

int32 setEnhancedSecurityCtlOLTChallengeStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 origValue = 0;
	uint8 tmpValue = 0;
	notify_msg_t msg;
	int currflag = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 instanceId = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	/*get ONU authentication status*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	if(*attributeValuePtr != ONU_AUTHENTICATION_STATUS_S0)
		return -1;
	attributeValuePtr = NULL;
	
	/*set OLT Challenge Status*/
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, (uint16 *)&length);
	if(attributeValuePtr == NULL)
		return -1;
	origValue = *attributeValuePtr;
	memcpy(attributeValuePtr, value, length);
	
	tmpValue = *((uint8*)value);
	if((origValue == 0) && (tmpValue == 1))/*false to true*/
	{
		/*set ONU anthentication status to s1*/
		setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S1);

		/*deal with ONU random challenge table's AVC*/
		if(generateONURandomChallengeTable(tmpomciMeInst_ptr) == 0){
			memset(&msg, 0, sizeof(msg));
			msg.notifyType = OMCI_NOTIFY_TYPE_AVC;
			msg.avcId = OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_RANDOM_CHALLENGE_TB;
			
			if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n avcMsgHandle: sendOmciNotifyMsg fail!\r\n");

			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n avcMsgHandle: sendOmciNotifyMsg OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_RANDOM_CHALLENGE_TB!\r\n");
			
			/*deal with ONU authentication result table's AVC*/
			if(generateONUAuthenticationResultTable(tmpomciMeInst_ptr) == 0){	
				memset(&msg, 0, sizeof(msg));
				msg.notifyType = OMCI_NOTIFY_TYPE_AVC;
				msg.avcId = OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_RESULT_TB;
				
				if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n avcMsgHandle: sendOmciNotifyMsg fail!\r\n");

				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n avcMsgHandle: sendOmciNotifyMsg OMCI_AVC_ID_ENHANCED_SECURITY_CTL_ONU_AUTHENTICATION_RESULT_TB!\r\n");
			
				currflag = 1;
			}
		}

		if(currflag == 0)
		{
			/*set ONU anthentication status to s5*/
			setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S5);
			if(timerStartS(1000, (TimerCallback)updateONUAuthenticationState, tmpomciMeInst_ptr) == -1)
				return -1;
		}
		else
		{
			/*set ONU anthentication status to s2*/
			setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S2);
			if(timerStartS(3000, (TimerCallback)updateONUAuthenticationState, tmpomciMeInst_ptr) == -1)
				return -1;
		}
	}
	
	return 0;
}

int32 getEnhancedSecurityCtlONURandomChallengeTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 tableSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tableSize = getTableAttributeValue(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex, omciAttribute);

	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getONURandomChallengeTB->tableSize = %d",tableSize);

	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

int32 getEnhancedSecurityCtlONUAuthenticationResultTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 tableSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tableSize = getTableAttributeValue(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[6].attriIndex, omciAttribute);

	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getONUAuthenticationResultTB->tableSize = %d",tableSize);

	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

int32 setEnhancedSecurityCtlOLTAuthenticationResultTB(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciTableAttriValue_t *tableConent_ptr = NULL;
	uint8 *tableValue_ptr = NULL; 
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	uint8 index = 0;
	uint8 findFlag = 0;//0:not find, 1:find
	uint16 instanceId = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 tableSize = 0;
	int ret = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	/*get ONU authentication status*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	if(*tmpValue_ptr != ONU_AUTHENTICATION_STATUS_S2)
		return -1;
	tmpValue_ptr = NULL;
	
	/*set OLT result status to false*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[8];
	tmpValue_ptr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	*tmpValue_ptr = 0;
	tmpValue_ptr = NULL;

	index = *(uint8*)value;
	if(index != 0)/*add an entry*/
	{
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex);
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n OLTAuthenticationResultTb is NULL and add first record");
			tableSize = 0;
			tableValue_ptr = NULL;
		}else{
			tableSize = tableConent_ptr->tableSize;
			tableValue_ptr = tableConent_ptr->tableValue_p;
		}
		tmp_ptr = tableValue_ptr;
		
		/*search current entry*/
		length = OLT_TABLE_ENTRY_LEN;
		while(tmp_ptr != NULL){
			if(memcmp(&index,tmp_ptr,1) == 0){
				findFlag = 1;
				break;
			}
			length += OLT_TABLE_ENTRY_LEN;
			if(length > tableConent_ptr->tableSize){
				//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setOLTAuthenticationResultTb->length > tableConent_ptr->tableSize");
				findFlag = 0;
				break;
			}
			tmp_ptr += OLT_TABLE_ENTRY_LEN;
		}

		if(findFlag == 1){
			/*if find, replace this entry*/
			memcpy(tmp_ptr, value, OLT_TABLE_ENTRY_LEN);
		}
		else
		{
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize+OLT_TABLE_ENTRY_LEN);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTAuthenticationResultTb:calloc failure");
				ret = -1;
				goto end;		
			}
			memcpy(tmpValue_ptr, tableValue_ptr, tableSize);
			memcpy(tmpValue_ptr+tableSize, value, OLT_TABLE_ENTRY_LEN);		
			tableSize += OLT_TABLE_ENTRY_LEN;		
		}

		//we need to update instance table list, because of new alloc
		if(findFlag == 0){
			if(tableConent_ptr != NULL){//del table content  in instance table list
				ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTAuthenticationResultTb: omciDelTableValueOfInst failure");
					ret = -1;
					goto end;
				}
			}

			tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex, tableSize, tmpValue_ptr);
			if(tableConent_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getOLTAuthenticationResult: omciAddTableValueToInst failure");
				ret = -1;
				goto end;	
			}
		}
	}
	else if(index == 0)/*clear this table*/
	{
		/*get table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex);
		if(tableConent_ptr == NULL){
			ret = 0;
			goto end;	
		}
		
		/*clean table in table list*/
		ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setOLTAuthenticationResultTb: omciDelTableValueOfInst failure");
			ret = -1;
			goto end;	
		}
		
	}
	ret = 0;
	
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}
	return ret;
}

int generateMasterSessionKey(omciMeInst_t *meInstantPtr, int flag)
{
	uint8 *OltValue_ptr = NULL;
	uint8 *OnuValue_ptr = NULL;
	uint32 OltSize = 0;
	uint32 tmpOltSize = 0;
	uint32 OnuSize = 0;
	omciTableAttriValue_t *tableConent_ptr = NULL;
	uint8 MACText[MASTER_SESSION_KEY_LEN] = {0};
	uint32 MACTextLen = MASTER_SESSION_KEY_LEN;
	uint8 *plainText = NULL;
	uint32 plainTextLen = 0;
	uint16 index = 0;
	uint16 entryNumber = 0;
	int ret = 0;
	uint8 psk[PRESHARED_KEY_LEN] = {0};
	int tmpPSK[PRESHARED_KEY_LEN] = {0};
	char buffer[52] = {0};

	if(meInstantPtr == NULL)
		return -1;
	
	if(flag == 0){
		//here, we can do: termination of a master session key
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n termination of a master session key.");
	}
	else if(flag == 1)//generate master session key according to AES_CMAC_128
	{
		/*get EnahncedPSK from romfile*/
		tcapi_get(GPON_ONU, ENHANCED_PSK, buffer);
		sscanf(buffer, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
			&tmpPSK[0], &tmpPSK[1], &tmpPSK[2], &tmpPSK[3], &tmpPSK[4], &tmpPSK[5], &tmpPSK[6], &tmpPSK[7], 
			&tmpPSK[8], &tmpPSK[9], &tmpPSK[10], &tmpPSK[11], &tmpPSK[12], &tmpPSK[13], &tmpPSK[14], &tmpPSK[15]);

		for(index=0; index<PRESHARED_KEY_LEN; index++){
			if((tmpPSK[index] >= 0) && (tmpPSK[index] <= 255))
				psk[index] = tmpPSK[index];
			else
				goto end;
		}
		
		/*
		* generate plain text for: master session key, '|' means concatenation
		* ONU_random_challenge_tables|OLT_random_challenge_table
		*/
		/*1.get OLT random challenge table*/
		tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
		if(tableConent_ptr == NULL)
		{
			OltSize = 0;
			OltValue_ptr = NULL;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n OLT random challenge table is NULL!\r\n");
			ret = -1;
			goto end;
		}
		else
		{
			OltSize = tableConent_ptr->tableSize;
			OltValue_ptr = tableConent_ptr->tableValue_p;
		}
		tableConent_ptr = NULL;
		/*2.get ONU random challenge table*/
		tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex);
		if(tableConent_ptr == NULL)
		{
			OnuSize = 0;
			OnuValue_ptr = NULL;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n ONU random challenge table is NULL!\r\n");
			ret = -1;
			goto end;
		}
		else
		{
			OnuSize = tableConent_ptr->tableSize;
			OnuValue_ptr = tableConent_ptr->tableValue_p;
		}
		tableConent_ptr = NULL;
		entryNumber = OnuSize/ONU_TABLE_ENTRY_LEN;
		/*3.calloc space for palinText*/
		plainTextLen = (OLT_TABLE_ENTRY_LEN-1+ONU_TABLE_ENTRY_LEN)*entryNumber;
		plainText = calloc(1, plainTextLen);
		if(plainText == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n calloc fail!\r\n");
			ret = -1;
			goto end;
		}
		/*4.generate palinText*/
		while(index < entryNumber){
			memcpy(plainText+tmpOltSize, OltValue_ptr+1, OLT_TABLE_ENTRY_LEN-1);
			tmpOltSize += OLT_TABLE_ENTRY_LEN;
			tmpOltSize -= 1;
			OltValue_ptr += OLT_TABLE_ENTRY_LEN;
			index++;
		}
		memcpy(plainText+plainTextLen-OnuSize, OnuValue_ptr, OnuSize);
		
		/*generate master session key according to AES_CMAC_128*/
		AES_CMAC(plainText, plainTextLen, psk, PRESHARED_KEY_LEN, MACText, (uint32 *)&MACTextLen);
		if(plainText != NULL){
			free(plainText);
			plainText = NULL;
		}
		/*set master session key*/
		//add set command here, supply by WL
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n set master session key");
	}
	
end:
	if(plainText != NULL){
		free(plainText);
		plainText = NULL;
	}
	return ret;
}

int generateMasterSessionKeyName(omciMeInst_t *meInstantPtr, int flag)
{
	uint8 *OltValue_ptr = NULL;
	uint8 *OnuValue_ptr = NULL;
	uint32 OltSize = 0;
	uint32 tmpOltSize = 0;
	uint32 OnuSize = 0;
	omciTableAttriValue_t *tableConent_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint8 MACText[MASTER_SESSION_KEY_LEN]={0};
	uint32 MACTextLen = MASTER_SESSION_KEY_LEN;
	uint8 *plainText = NULL;
	uint32 plainTextLen = 0;
	uint8 index = 0;
	uint8 entryNumber = 0;
	uint16 instanceId = 0;
	int ret = 0;
	uint8 identityMSK[PEER_IDENTITY_LEN] = {
		0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93, 0x31, 0x41, 0x59, 0x26, 0x53, 0x58, 0x97, 0x93
	};
	uint8 psk[PRESHARED_KEY_LEN] = {0};
	int tmpPSK[PRESHARED_KEY_LEN] = {0};
	char buffer[52] = {0};
	uint16 length = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n generateMasterSessionKeyName begin\r\n");

	if(meInstantPtr == NULL)
		return -1;
	
	/*get master session key name*/
	instanceId = get16(meInstantPtr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[10];
	attributeValuePtr = omciGetInstAttriByName(meInstantPtr , currentAttribute->attriName, (uint16 *)&length);

	/*upon termination of a master session key, clear master session key name to all-zeros*/
	if(flag == 0){
		memset(MACText, 0, sizeof(MACText));
		memcpy(attributeValuePtr, MACText, length);
	}
	else if(flag == 1)
	{/*generate master session key name according to AES_CMAC_128*/

		/*get EnahncedPSK from romfile*/
		tcapi_get(GPON_ONU, ENHANCED_PSK, buffer);
		sscanf(buffer, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X", 
			&tmpPSK[0], &tmpPSK[1], &tmpPSK[2], &tmpPSK[3], &tmpPSK[4], &tmpPSK[5], &tmpPSK[6], &tmpPSK[7], 
			&tmpPSK[8], &tmpPSK[9], &tmpPSK[10], &tmpPSK[11], &tmpPSK[12], &tmpPSK[13], &tmpPSK[14], &tmpPSK[15]);

		for(index=0; index<PRESHARED_KEY_LEN; index++){
			if((tmpPSK[index] >= 0) && (tmpPSK[index] <= 255))
				psk[index] = tmpPSK[index];
			else
				goto end;
		}
		
		/* generate plain text for: master session key name, '|' means concatenation
		 * ONU_random_challenge_tables|OLT_random_challenge_table|0x3141 5926 5358 9793 3141 5926 5358 9793
		 */
		/*1.get OLT random challenge table*/
		tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[2].attriIndex);
		if(tableConent_ptr == NULL)
		{
			OltSize = 0;
			OltValue_ptr = NULL;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n OLT random challenge table is NULL!\r\n");
			ret = -1;
			goto end;
		}
		else
		{
			OltSize = tableConent_ptr->tableSize;
			OltValue_ptr = tableConent_ptr->tableValue_p;
		}
		tableConent_ptr = NULL;
		/*2.get ONU random challenge table*/
		tableConent_ptr = omciGetTableValueByIndex(meInstantPtr, omciAttriDescriptListEnhancedSecurityCtl[5].attriIndex);
		if(tableConent_ptr == NULL)
		{
			OnuSize = 0;
			OnuValue_ptr = NULL;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n ONU random challenge table is NULL!\r\n");
			ret = -1;
			goto end;
		}
		else
		{
			OnuSize = tableConent_ptr->tableSize;
			OnuValue_ptr = tableConent_ptr->tableValue_p;
		}
		tableConent_ptr = NULL;
		entryNumber = OnuSize/ONU_TABLE_ENTRY_LEN;
		/*3.calloc space for palinText*/
		plainTextLen = (OLT_TABLE_ENTRY_LEN-1+ONU_TABLE_ENTRY_LEN)*entryNumber+PEER_IDENTITY_LEN;
		plainText = calloc(1, plainTextLen);
		if(plainText == NULL)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n calloc fail!\r\n");
			ret = -1;
			goto end;
		}
		/*4.generate palinText*/
		memcpy(plainText, OnuValue_ptr, OnuSize);
		while(index < entryNumber){
			memcpy(plainText+OnuSize+tmpOltSize, OltValue_ptr+1, OLT_TABLE_ENTRY_LEN-1);
			tmpOltSize += OLT_TABLE_ENTRY_LEN;
			tmpOltSize -= 1;
			OltValue_ptr += OLT_TABLE_ENTRY_LEN;
			index++;
		}
		memcpy(plainText+plainTextLen-PEER_IDENTITY_LEN, identityMSK, PEER_IDENTITY_LEN);
		
		/*generate master session key name according to AES_CMAC_128*/
		AES_CMAC(plainText, plainTextLen, psk, PRESHARED_KEY_LEN, MACText, (uint32 *)&MACTextLen);
		if(plainText != NULL){
			free(plainText);
			plainText = NULL;
		}
		memcpy(attributeValuePtr, MACText, MASTER_SESSION_KEY_LEN);
	}
	
end:
	if(plainText != NULL){
		free(plainText);
		plainText = NULL;
	}
	return ret;
}

int getAuthenticationResult(uint8 *meInstantPtr)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint32 oltSize = 0;
	uint32 onuSize = 0;
	uint8 *OltValue_ptr = NULL;
	uint8 *OnuValue_ptr = NULL;
	omciTableAttriValue_t *tableConent_ptr = NULL; 
	int index = 0;
	int entryNumber = 0;
	int ret = 0;

	if(meInstantPtr == NULL)
		return -1;

	/*get OLT authentication result table*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[7].attriIndex);
	if(tableConent_ptr == NULL)
	{
		oltSize = 0;
		OltValue_ptr = NULL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n OLT anthentication result table is NULL \r\n");
		ret = -1;
		return ret;
	}
	else
	{
		oltSize = tableConent_ptr->tableSize;
		OltValue_ptr = tableConent_ptr->tableValue_p;
	}
	tableConent_ptr = NULL;

	/*get ONU authentication result table*/
	tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, omciAttriDescriptListEnhancedSecurityCtl[6].attriIndex);
	if(tableConent_ptr == NULL)
	{
		onuSize = 0;
		OnuValue_ptr = NULL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n ONU anthentication result table is NULL \r\n");
		ret = -1;
		return ret;
	}
	else
	{
		onuSize = tableConent_ptr->tableSize;
		OnuValue_ptr = tableConent_ptr->tableValue_p;
	}
	tableConent_ptr = NULL;
	entryNumber = onuSize/ONU_TABLE_ENTRY_LEN;

	/*compare OLT&ONU authentication result table*/
	while(index < entryNumber)
	{
		if(memcmp(OnuValue_ptr, OltValue_ptr+1, ONU_TABLE_ENTRY_LEN) == 0)
		{
			OnuValue_ptr += ONU_TABLE_ENTRY_LEN;
			OltValue_ptr += OLT_TABLE_ENTRY_LEN;
			index++;
		}
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n OLT anthentication result table differ with ONU's \r\n");
			ret = -1;
			return ret;
		}
	}
	
	if(index != entryNumber)
	{
		ret = -1;
		return ret;
	}
	
	return ret;
}

int32 setEnhancedSecurityCtlOLTResultStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint8 oldValue = 0;
	uint8 newValue = 0;
	uint16 instanceId = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	/*get ONU authentication status*/
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_ENHANCED_SECURITY_CONTROL);
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	currentAttribute = &currentME->omciAttriDescriptList[9];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, (uint16 *)&length);
	if(*attributeValuePtr != ONU_AUTHENTICATION_STATUS_S2)
		return -1;
	attributeValuePtr = NULL;
	
	newValue = *((uint8*)value);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, (uint16 *)&length);
	oldValue = *attributeValuePtr;
	memcpy(attributeValuePtr, value, length);
	if((oldValue == 0) && (newValue == 1))/*false to true*/
	{
		/*ONU deal with OLT authentication result table*/
		if(getAuthenticationResult(meInstantPtr) == 0)
		{
			/*3-steps authentication success*/
			if(generateMasterSessionKeyName(tmpomciMeInst_ptr, 1) == 0)
			{
				setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S3);
				//generate&supply master session key to TC layer
				generateMasterSessionKey(tmpomciMeInst_ptr, 1);
				setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S0);
			}
		}
		else
		{
			/*3-steps authentication fail*/
			setONUAuthenticationState(tmpomciMeInst_ptr, ONU_AUTHENTICATION_STATUS_S4);
			if(timerStartS(1000, (TimerCallback)updateONUAuthenticationState, tmpomciMeInst_ptr) == -1)
				return -1;
		}
	}
	
	return 0;
}


