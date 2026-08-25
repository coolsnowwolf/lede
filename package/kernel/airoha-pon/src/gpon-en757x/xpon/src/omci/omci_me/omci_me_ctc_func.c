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
	ctc_ME_func.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/7/16	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_ctc.h"

uint8 AuthStatusFlag = 0;
/*******************************************************************************************************************************
CTC ONU Capability

********************************************************************************************************************************/
int omciInternalCreateForOnuCapability(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[25] = {0};
	char entryName[32]={0};
	uint16 attriMask = 0;
	

	if(instanceId > 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"only one instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;

	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	sprintf(entryName, "GPON_Capability");	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nCreate ME: %s",entryName);

	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_ONU_capability: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "OperatorID", (char *)buffer);
		memcpy(attributeValuePtr, buffer, length);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nOperatorID = %s", buffer);	
	}
	else
		goto fail;


	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_ONU_capability: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "SpecVersion", (char *)buffer);
		*attributeValuePtr = atoi((char *)buffer);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nSpecVersion = %d",*attributeValuePtr);	
	}
	else
		goto fail;
	
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{	
		*attributeValuePtr = pSystemCap->onuType - 1;	
	}
	else
		goto fail;

	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_ONU_capability: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "ONU_PowerCtr", (char *)buffer);
		*attributeValuePtr = atoi((char *)buffer);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nONU_PowerCtr = %d",*attributeValuePtr);
	}
	else
		goto fail;
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;	
}

int omciInitInstForOnuCapability(void)
{
	int ret = 0;
		
	ret = omciInternalCreateForOnuCapability(OMCI_CLASS_ID_CTC_ONU_CAPABILITY, 0);
	return ret;
}

int omciMeInitForOnuCapability(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	
	return 0;		
}

/*******************************************************************************************************************************
CTC LOID authentication

********************************************************************************************************************************/
int omciMeInitForLoidAuthen(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;		
}

int omciInternalCreateForLoidAuthen(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[25] = {0};
	char entryName[32]={0};
	uint16 attriMask = 0;
	
	

	if(instanceId > 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"only one instance \n");
		return -1;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, &length);

	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	sprintf(entryName, "GPON_LOIDAuth");	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nCreate ME: %s",entryName);

	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_LOID_authen: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "OperatorID", (char *)buffer);
		memcpy(attributeValuePtr, buffer, length);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nOperatorID = %s", buffer);	
	}
	else
		goto fail;


	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_LOID_authen: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "LOID", (char *)buffer);
		memcpy(attributeValuePtr, buffer, length);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nLOID = %s", buffer);	
	}
	else
		goto fail;
	
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_LOID_authen: tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName);
		memset(buffer, 0, sizeof(buffer));
		tcapi_get(entryName, "Password", (char *)buffer);
		memcpy(attributeValuePtr, buffer, length);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nPassword = %s", buffer);	
	}
	else
		goto fail;


	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if(attributeValuePtr != NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\nomciInternalCreateFor_LOID_authen: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName);
		if(AuthStatusFlag == 0)
		{
			*attributeValuePtr = 0;	
			tcapi_set(entryName, "AuthStatus", "0");
		}
		else
		{
			memset(buffer, 0, sizeof(buffer));
			if(TCAPI_SUCCESS == tcapi_get(entryName,"AuthStatus",(char *)buffer))
				*attributeValuePtr = atoi(buffer);
			else
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\ntcapi_get AuthStatus fail\n");
		}
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\nAuthStatus = %d",*attributeValuePtr);
	}
	else
		goto fail;	
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;	
}

int omciInitInstForLoidAuthen(void)
{
	int ret = 0;
	uint16 instanceNum=0;
	
	ret = omciInternalCreateForLoidAuthen(OMCI_CLASS_ID_CTC_LOID_AUTHEN, instanceNum);;/*internal create func*/
	if (ret == -1)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForLoidAuthen create OMCI_CLASS_ID_CTC_LOID_AUTHEN fail\n");
		return ret;
	}
	return ret;
}

int32 getOperatorIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[64] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_LOIDAuth", "OperatorID", buffer);
		memcpy(attributeValuePtr,  buffer, 4);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOperatorIDValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 4, omciAttribute);	
}
int32 getCtcLOIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[64] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_LOIDAuth", "LOID", buffer);
		memcpy(attributeValuePtr,  buffer, 24);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOperatorIDValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 24, omciAttribute);	
}
int32 getCtcPasswordValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[64] = {'\0'};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memset(buffer, 0, sizeof(buffer));
		tcapi_get("GPON_LOIDAuth", "Password", buffer);
		memcpy(attributeValuePtr,  buffer, 12);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "getOperatorIDValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, 12, omciAttribute);	
}

int setAuthStatusValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{

	uint8 *attributeValuePtr;
	uint16 length = 0;
	int ret = 0;
	char entryValue[2]={0};

	/*set AuthStatus to instance and romfile*/

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setAuthStatusValue:value = %d", *value);
	
	sprintf(entryValue, "%d", *value);
	if(strcmp(entryValue, "0") != 0)
		AuthStatusFlag = 1;

	ret = tcapi_set("GPON_LOIDAuth", "AuthStatus", entryValue);
	if (ret == 0)
	{
		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			memcpy(attributeValuePtr, value, length);
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTimeOfDayValue: attributeValuePtr == NULL\n");
			ret = -1;
		}
	}

	return ret;
}


/*******************************************************************************************************************************
CTC Extended multicast operations files

********************************************************************************************************************************/
int omciMeInitForExtendMulticastProfile(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMulticastProfileDeleteAction;
#ifdef TCSUPPORT_OMCI_EXTENDED_MSG	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET_TABLE] = omciSetTableAction; //do not support now
#endif	
	
	return 0;		
}

/*******************************************************************************************
**function name
	setEMOPMulticastAclTableValueByType
**description:
	Set ACL Value for OLT request according to the attribute index.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	value :	return value
  	meInstantPtr: 9.3.27 instance pointer
  	omciAttribute: 9.3.27 attribute pointer
  	attrIndex: valid value
  		7: dynamic acl attribute
  		8: static acl attribute
********************************************************************************************/
int setEMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, IN uint16 attrIndex){
	omciTableAttriValue_t *tableConent_ptr = NULL; //table content pointer
	uint8 *tmpValue_ptr = NULL;
	uint8 *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	uint8 action = 0;//0:remove this entry, 1:add this entry, 2: clear table
	uint16 instanceId = 0;//instance id
	uint32 tableSize = 0;
	uint32 length = 0;
	uint8 findFlag = 0;//0:not find, 1:find
	int ret = -1;
	uint16 ctrlField = 0;
	uint8 setCtrl = 0;
	uint8 rowPart = 0;
	uint16 rowID = 0;
	uint16 ctrlField_tmp = 0;
	uint8 rowPart_tmp = 0;
	uint16 rowID_tmp = 0;	
	omciIgmpRuleInfo_t ruleRecordInfo;
	omciIgmpRuleInfo_ptr ruleRecord = &ruleRecordInfo;
	uint16 type = 0;
	uint16 ruleType = 0;
	uint8 tempAclTableRowEntry[OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH] = {0};
	uint8 zero[OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH] = {0};
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, attrIndex =%02x\n",__LINE__, __func__, attrIndex);

	if((tmpomciMeInst_ptr == NULL) || (value == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	/*only support dynamic acl attribute or static acl attribute*/
	if(attrIndex == OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX){
		ruleType = OMCI_MULTICAST_IGMP_RULE_TYPE_DYN;
		type = OMCI_MULTICAST_DYNAMIC_ACL_TYPE;
	}else if(attrIndex == OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX){
		ruleType = OMCI_MULTICAST_IGMP_RULE_TYPE_STATIC;
		type = OMCI_MULTICAST_STATIC_ACL_TYPE;		
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]attrIndex fail\n",__LINE__, __func__);
		goto end;
	}

	/*parse value*/
	tmp_ptr = (uint8 *)value;
	ctrlField = get16(tmp_ptr);
	setCtrl = ((ctrlField & OMCI_EMOP_ACL_TABLE_ENTRY_SET_CTRL_MASK) >> OMCI_EMOP_ACL_TABLE_ENTRY_SET_CTRL_FILED);
	rowPart = ((ctrlField & OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_MASK) >> OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
	rowID = (ctrlField & OMCI_EMOP_ACL_TABLE_ENTRY_ROW_ID_MASK);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]setCtrl=0x%2x, rowPart=0x%2x, rowID=%d\n",__LINE__, __func__, setCtrl,rowPart,rowID);

	/*set rule from packet value */
	memset(ruleRecord, 0, sizeof(omciIgmpRuleInfo_t));
	ruleRecord->rowKey = rowID;
	ruleRecord->ruleType = ruleType;

	
	/*get instance id*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);


	/*get action(add/del/clear)*/
	action = setCtrl;	
	if(action == OMCI_IGMP_ACL_TABLE_ENTRY_DEL_ACTION){// delete table
		/*get current table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, attrIndex);
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tableConent_ptr fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}

		tableSize = tableConent_ptr->tableSize;		
		/*search record*/
		length = OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
		for(tmp_ptr = tableConent_ptr->tableValue_p; tmp_ptr != NULL; tmp_ptr += OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH){
			ctrlField_tmp = get16(tmp_ptr+OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH*rowPart);
			rowPart_tmp = ((ctrlField_tmp & OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_MASK)>> OMCI_EMOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
			rowID_tmp = (ctrlField_tmp & OMCI_EMOP_ACL_TABLE_ENTRY_ROW_ID_MASK);	
			if((rowID == rowID_tmp) &&(rowPart == rowPart_tmp)){
				memset(tmp_ptr + OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH*rowPart, 0, OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH);				
				memcpy(tempAclTableRowEntry, tmp_ptr, OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH);
				findFlag = 1;//find
				break;
			}

			length += OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
			if(length > tableConent_ptr->tableSize){ 
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]length:%d > tableConent_ptr->tableSize:%d\n",__LINE__, __func__, length, tableConent_ptr->tableSize);
				findFlag = 0;//not find
				break;
			}
		}
		
		/*get real length*/
		length -= OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
		/*get valid content in table*/
		if(findFlag == 1){
			if(memcmp(tempAclTableRowEntry, zero, OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH) == 0){
			/*delete ACL rule via calling multicast API*/
			ret = setRealMulticastRuleByProfile(instanceId, type, action, NULL, ruleRecord);
			if(ret != 0){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByProfile fail\n",__LINE__, __func__);
				goto end;
			}			
			
			/*delete a entry from table list*/			
			/*malloc new space*/
			if(tableSize-OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH > 0)
			{
				tableSize -= OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH; //update current table size
				tmpValue_ptr = calloc(1 , tableSize);
				if(tmpValue_ptr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
					ret = -1;
					goto end;
				}	
				/*if find, then delete this record in table*/			
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, length);
				memcpy(tmpValue_ptr+length, tableConent_ptr->tableValue_p+length+OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH, tableSize-length);
			}
				else
				{
					tableSize = 0;
				}				
			}
			else{
				/*set rule  info */
				/*row part 0 */
				tmp_ptr = tempAclTableRowEntry;
				ruleRecord->gemPortId = get16(tmp_ptr+2);
				ruleRecord->vid = get16(tmp_ptr+4);	
				ruleRecord->imputedBandwidth = get32(tmp_ptr+6);
				/*row part 1 */	
				tmp_ptr = tempAclTableRowEntry+OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH;
				if (memcmp(tmp_ptr+2, zero, OMCI_MULTICAST_IPV4_OFFSET) == 0) /* ipv4 address*/		
					memcpy(ruleRecord->startDestIp,tmp_ptr+2+OMCI_MULTICAST_IPV4_OFFSET, OMCI_MULTICAST_IPV4_LEN);
				else
					memcpy(ruleRecord->startDestIp,tmp_ptr+2, OMCI_MULTICAST_IP_LEN);
				/*row part 2 */	
				tmp_ptr = tempAclTableRowEntry+2*OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH;
				if (memcmp(tmp_ptr+2, zero, OMCI_MULTICAST_IPV4_OFFSET) == 0) /* ipv4 address*/		
					memcpy(ruleRecord->endDestIp,tmp_ptr+2+OMCI_MULTICAST_IPV4_OFFSET, OMCI_MULTICAST_IPV4_LEN);
				else
					memcpy(ruleRecord->endDestIp,tmp_ptr+2, OMCI_MULTICAST_IP_LEN);
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]ruleRecord rowKey:%x,ruleType:%x,gemPortId:%x,vid:%x,srcIp:%x,startDestIp:%x,endDestIp:%x\n",
						__LINE__, __func__, ruleRecord->rowKey,ruleRecord->ruleType,ruleRecord->gemPortId,ruleRecord->vid, 
					get32(ruleRecord->srcIp), get32(ruleRecord->startDestIp), get32(ruleRecord->endDestIp));

				
				/*set ACL rule via calling multicast API*/
				ret = setRealMulticastRuleByProfile(instanceId, type, OMCI_IGMP_ACL_TABLE_ENTRY_WRITE_ACTION, NULL, ruleRecord);
				if(ret != 0){
					ret = -1;
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByProfile fail\n",__LINE__, __func__);
					goto end;
				}			

				
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s] not last entry, modify this record\n", __LINE__, __func__);			
				/*if find, then modify this record in table*/	
				/*malloc new space*/
				tmpValue_ptr = calloc(1 , tableSize);
				if(tmpValue_ptr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
					ret = -1;
					goto end;
				}
				
				/* re fill this record in table*/			
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, tableSize);
			}
		}else{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s] not find \n", __LINE__, __func__);			
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
				ret = -1;
				goto end;
			}
			
			/*fill new space*/	
			memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, tableSize);
		}
	}else if(action == OMCI_IGMP_ACL_TABLE_ENTRY_WRITE_ACTION)// add or modify table
	{
		/*get current table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, attrIndex);
		if(tableConent_ptr == NULL){
			findFlag = 0;
			tableSize = 0;
			tmp_ptr = NULL;
		}else{
			tableSize = tableConent_ptr->tableSize;
			tmp_ptr = tableConent_ptr->tableValue_p;
		}
						
		/*search record*/
		for(length = OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH; tmp_ptr != NULL; tmp_ptr += OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH){		
			ctrlField_tmp = get16(tmp_ptr);
			rowID_tmp = (ctrlField_tmp & OMCI_EMOP_ACL_TABLE_ENTRY_ROW_ID_MASK);	
			if(rowID == rowID_tmp){
				memcpy(tempAclTableRowEntry, tmp_ptr, OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH);
				findFlag = 1;//find
				break;
			}
			length += OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
			if(length > tableConent_ptr->tableSize){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]length:%d > tableConent_ptr->tableSize:%d\n",__LINE__, __func__, length, tableConent_ptr->tableSize);
				findFlag = 0;//not find
				break;
			}
		}
		/*get real length*/
		length -= OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
		memcpy(tempAclTableRowEntry + OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH*rowPart, value, OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH);

		/*set rule other info */
		/*row part 0 */
		tmp_ptr = tempAclTableRowEntry;
		ruleRecord->gemPortId = get16(tmp_ptr+2);
		ruleRecord->vid = get16(tmp_ptr+4);	
		ruleRecord->imputedBandwidth = get32(tmp_ptr+6);
		/*row part 1 */	
		tmp_ptr = tempAclTableRowEntry+OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH;
		if (memcmp(tmp_ptr+2, zero, OMCI_MULTICAST_IPV4_OFFSET) == 0) /* ipv4 address*/		
			memcpy(ruleRecord->startDestIp,tmp_ptr+2+OMCI_MULTICAST_IPV4_OFFSET, OMCI_MULTICAST_IPV4_LEN);
		else
			memcpy(ruleRecord->startDestIp,tmp_ptr+2, OMCI_MULTICAST_IP_LEN);
		/*row part 2 */	
		tmp_ptr = tempAclTableRowEntry+2*OMCI_EMOP_ACL_TABLE_ENTRY_LENGTH;
		if (memcmp(tmp_ptr+2, zero, OMCI_MULTICAST_IPV4_OFFSET) == 0) /* ipv4 address*/		
			memcpy(ruleRecord->endDestIp,tmp_ptr+2+OMCI_MULTICAST_IPV4_OFFSET, OMCI_MULTICAST_IPV4_LEN);
		else
			memcpy(ruleRecord->endDestIp,tmp_ptr+2, OMCI_MULTICAST_IP_LEN);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]ruleRecord rowKey:%x,ruleType:%x,gemPortId:%x,vid:%x,srcIp:%x,startDestIp:%x,endDestIp:%x\n",
				__LINE__, __func__, ruleRecord->rowKey,ruleRecord->ruleType,ruleRecord->gemPortId,ruleRecord->vid, 
			get32(ruleRecord->srcIp), get32(ruleRecord->startDestIp), get32(ruleRecord->endDestIp));

		
		/*set ACL rule via calling multicast API*/
		ret = setRealMulticastRuleByProfile(instanceId, type, action, NULL, ruleRecord);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByProfile fail\n",__LINE__, __func__);
			goto end;
		}
			
		/*get valid content in table*/
		if(findFlag == 1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s] find, modify this record\n", __LINE__, __func__);			
			/*if find, then modify this record in table*/	
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
				ret = -1;
				goto end;
			}
			
			/*fill new space*/	
			memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, tableSize);
			memcpy(tmpValue_ptr+length, tempAclTableRowEntry, OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH);	
		}else{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]not find, add this record\n", __LINE__, __func__);	
			/*if not find, add this record*/
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize+OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
				ret = -1;
				goto end;		
			}		

			/*fill new space*/
			if(tableSize > 0)
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, tableSize);
			memcpy(tmpValue_ptr+length, tempAclTableRowEntry, OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH);	
			tableSize += OMCI_EMOP_ACL_TABLE_ROW_ENTRY_LENGTH;
		}
	
	}	else if(action == OMCI_IGMP_ACL_TABLE_ENTRY_CLEAR_ACTION){//clear this table
		/*get table content*/
		tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, attrIndex);
		if(tableConent_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tableConent_ptr NULL\n",__LINE__, __func__);
			ret = 0;
			goto end;	
		}
		/*real clean record*/
		ret = clearRealMulticastRuleByProfileType(instanceId, type);		
		tableSize = 0;	
	}

	/*update table content in instance table list*/
	if(tableConent_ptr != NULL){//del table content  in instance table list
		ret = omciDelTableValueOfInst(tmpomciMeInst_ptr, attrIndex);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciDelTableValueOfInst fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}
		
	if(tableSize == 0){//current table is empty
		ret = 0;
		goto end;	
	}
#if 1
	omciDumpTable(tmpValue_ptr, tableSize);
#endif
	tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_ptr, attrIndex, tableSize, tmpValue_ptr);
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciAddTableValueToInst fail\n",__LINE__, __func__);
		ret = -1;
		goto end;	
	}	

	ret = 0;
end:
	if(tmpValue_ptr != NULL){
		free(tmpValue_ptr);
		tmpValue_ptr = NULL;
	}

	if(ret != 0){
		ret = -1;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, tableSize=%d\n",__LINE__, __func__, tableSize);
	return ret;
}



int setEMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX;
	
	return setEMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);
}


int setEMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX;
	
	return setEMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);
}

