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
	omci_me_voip_func.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2013/9/26	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_layer3_data_services.h"
#include "omci_me_voip.h"
#include "errno.h"
#include "libcompileoption.h"


/*******************************************************************************************************************************
globle variable

********************************************************************************************************************************/
    
#define LARGE_STRING_ME_MAX_LEN (25*15)
#define SIP_USER_DATA_USER_PART_AOR_LEN   2

/*******************************************************************************************************************************
extern 

********************************************************************************************************************************/

/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
// set the ptr value, and retrieve the string to some buff for later parsing
static int
VOIP_General_LargeStringPtr_Set(IN    char  * value, 
                                           IN_OUT uint8 * meInstantPtr , 
                                           IN    omciAttriDescript_t * omciAttribute, 
                                           IN    uint8   flag, 
                                           OUT   uint8 * buff /*Careful! Crash if NULL*/)
{
    OMCI_FUNC_TRACE_ENTER;
    omciMeInst_ptr inst_ptr = (omciMeInst_t *) meInstantPtr;
    
    int32 ret = setGeneralValue(value, inst_ptr , omciAttribute, flag);
    if(0 != ret)
        return OMCI_VOIP_FAIL;

    int16  largeStringME_ID  = get16(value);
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Get LargeString ME Instance NO.:%d\n", largeStringME_ID);
    
    uint16 len = 0;
    if ( (0 != getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, largeStringME_ID ,buff, &len)) || 
         (0 == len) )
    {
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "%s: Can't get Large String ME, len: %d\n", __FUNCTION__, len);
        return OMCI_VOIP_FAIL;
    }

    return OMCI_VOIP_SUCC;
}

/*******************************************************************************************
**function name
	getEntryIdBySipUserData
**description:
	get sip entry id
 **retrun :
	-1: failure
	0:	success
**parameter:
	instId :	9.9.2 instance id
	entryId:	return entry id
********************************************************************************************/
int getEntryIdBySipUserData(uint16 instId, uint16 *entryId){
	int ret = OMCI_VOIP_FAIL;
	uint8 *attrValue_ptr = NULL;
	uint16 tmpU16 = 0;	

	if(entryId == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->entryId == NULL, fail",__FUNCTION__);
		goto end;
	}

	/*calloc memory*/
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getEntryIdBySipUserData-> val calloc error ");
		ret = -1;
		goto end;
	}


	/*get pptp pointer from current instance*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_SIP_USER_DATA, instId, 10, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId pptp pointer, fail",__FUNCTION__);
		goto end;
	}

	tmpU16 = get16(attrValue_ptr);
	*entryId = (tmpU16&0x00FF)-1;
	
	ret = OMCI_VOIP_SUCC;

end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
	return ret;

}
/*******************************************************************************************
**function name
	setSipUserPartAOR
**description:
	set user part AOR by tcapi
 **retrun :
	-1: failure
	0:	success
**parameter:
	val :	value
	entryId:	entry id
********************************************************************************************/
int setSipUserPartAOR(char * val, uint16 entryId){
	int ret = OMCI_VOIP_FAIL;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};

	if(val == NULL){
		goto end;
	}

	snprintf(nodeName, MAX_BUFFER_SIZE-1 ,"%s%d", OMCI_VOIP_BASIC_ENTRY,entryId);	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	strncpy(tempBuffer,val,MAX_BUFFER_SIZE-1);
	tcapi_set(nodeName, OMCI_SIP_USER_PART_AOR, tempBuffer);

    OMCI_TRACE("[%s][%d] set  %s %s %s\n",__FUNCTION__,__LINE__,nodeName,OMCI_SIP_USER_PART_AOR,tempBuffer);

	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	return ret;

}

/*******************************************************************************************
**function name
	setSipDispName
**description:
	set display name by tcapi
 **retrun :
	-1: failure
	0:	success
**parameter:
	val :	value
	entryId:	entry id
********************************************************************************************/
int setSipDispName(char * val, uint16 entryId){
	int ret = OMCI_VOIP_FAIL;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};

	if(val == NULL){
		goto end;
	}

	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY,entryId);	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	strcpy(tempBuffer,val);
	tcapi_set(nodeName, OMCI_SIP_DISP_NAME, tempBuffer);

	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	return ret;

}

/*******************************************************************************************
**function name
	setSipUserPass
**description:
	set sip username and password by tcapi
 **retrun :
	-1: failure
	0:	success
**parameter:
	instId :	9.9.2 instance id
	entryId:	entry id
********************************************************************************************/
int setSipUserPass(uint16 instId, uint16 entryId){
	int ret = OMCI_VOIP_FAIL;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	uint8 *attrValue_ptr = NULL;
	uint16 authInstId = 0;

	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY,entryId);	

	/*calloc memory*/
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setSipUserPass-> val calloc error ");
		ret = -1;
		goto end;
	}

	/*get 9.9.2 username/pass attribute*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_SIP_USER_DATA, instId, 4, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId username/pass attribute, fail",__FUNCTION__);
		goto end;
	}
	authInstId = get16(attrValue_ptr);

	/*get user name form 9.12.4 authentication security method ME*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_AUTHENTICATION_METHOD, authInstId, 2, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId username, fail",__FUNCTION__);
		goto end;
	}
	
	/*set sip auth user name*/
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	memcpy(tempBuffer,attrValue_ptr,OMCI_ME_MAX_STR_ATTR_LEN);
	
	tcapi_set(nodeName, OMCI_SIP_AUTH_USER_NAME, tempBuffer);
	
	/*get password form 9.12.4 authentication security method ME*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_AUTHENTICATION_METHOD, authInstId, 3, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId password, fail",__FUNCTION__);
		goto end;
	}	

	/*set sip auth password*/	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	memcpy(tempBuffer,attrValue_ptr,OMCI_ME_MAX_STR_ATTR_LEN);
	tcapi_set(nodeName, OMCI_SIP_PASSWORD, tempBuffer);

	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
	
end:	
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
	return ret;

}

/*******************************************************************************************
**function name
	setSipURI
**description:
	set sip URI by tcapi
 **retrun :
	-1: failure
	0:	success
**parameter:
	instId :	9.9.2 instance id
	entryId:	entry id
********************************************************************************************/
int setSipURI(uint16 instId, uint16 entryId){
	int ret = OMCI_VOIP_FAIL;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	uint8 *attrValue_ptr = NULL;
	uint16 networkInstId = 0;
	uint16 largeStrInstId = 0;
	uint8 num = 0;
	uint8 * buf_ptr = NULL;
	uint8 * tmp_ptr = NULL;
	int index = 0;
	int endIndex = 0;

	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY,entryId);

	/*calloc memory*/
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setSipURI-> val calloc error ");
		ret = -1;
		goto end;
	}

	/*calloc memory*/
	buf_ptr =  (uint8 *)calloc(OMCI_ME_MAX_LARGE_STR_PART_NUM , OMCI_ME_MAX_STR_ATTR_LEN);
	if(buf_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setSipURI-> val calloc 2 error ");
		ret = -1;
		goto end;
	}

	/*get 9.9.2 sip URI pointer*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_SIP_USER_DATA, instId, 5, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId sip URI pointer*, fail",__FUNCTION__);
		goto end;
	}
	networkInstId = get16(attrValue_ptr);

	/*get address pointer  form 9.12.3 network address ME*/
	memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);	
	if(getAttributeValueByInstId(OMCI_CLASS_ID_NETWORK_ADDRESS, networkInstId, 2, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId instance id, fail",__FUNCTION__);
		goto end;
	}
	largeStrInstId = get16(attrValue_ptr);

	/*get number of parts form 9.12.5 large string*/
	memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(getAttributeValueByInstId(OMCI_CLASS_ID_LARGE_STRING, largeStrInstId, 1, attrValue_ptr, 1) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId number of parts, fail",__FUNCTION__);
		goto end;
	}
	num = *attrValue_ptr;

	if(num > OMCI_ME_MAX_LARGE_STR_PART_NUM){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->number of part %d >15, fail",__FUNCTION__, num);
		goto end;
	}

	endIndex = 2+num;
	tmp_ptr = buf_ptr;
	for(index = 2; index < endIndex; index++){		
		/*get parts val form 9.12.5 large string*/
		memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);
		if(getAttributeValueByInstId(OMCI_CLASS_ID_LARGE_STRING, largeStrInstId, index, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId part %d, fail",__FUNCTION__,index);
			goto end;
		}
		memcpy(tmp_ptr, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN);
		tmp_ptr +=OMCI_ME_MAX_STR_ATTR_LEN;
	}

	tmp_ptr = buf_ptr + (OMCI_ME_MAX_LARGE_STR_PART_NUM*OMCI_ME_MAX_STR_ATTR_LEN -1);
	*tmp_ptr = 0;
	
	/*set sip URI*/
	tcapi_set(nodeName, OMCI_SIP_URI, (char *)buf_ptr);
	
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
	
end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
	if(buf_ptr != NULL){
		free(buf_ptr);
	}
	
	return ret;

}

/*******************************************************************************************
**function name
	updateSipInfoByPPTP
**description:
	update 9.9.2 sip user date info
 **retrun :
	-1: failure
	0:	success
**parameter:
	instId :	9.9.2 instance id
	entryId:	entry id
********************************************************************************************/
int updateSipInfoByPPTP(uint16 instId,uint16 entryId){
	int ret = OMCI_VOIP_FAIL;
	uint8 *attrValue_ptr = NULL;
    omciMeInst_ptr meInstantPtr = NULL;
    omciAttriDescript_t * omciAttribute = NULL;
    uint16 len = 0;
    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};
    int16  largeStringME_ID  = 0;

	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n updateSipInfoByPPTP-> attrValue_ptr == NULL calloc error ");
		goto end;
	}

    OMCI_TRACE("=========== [%s][%d] instId = %d ===========\n",__FUNCTION__,__LINE__,instId);
    /*set user part AOR*/
    if(getAttributeValueByInstId(OMCI_CLASS_ID_SIP_USER_DATA, instId, 2, attrValue_ptr, SIP_USER_DATA_USER_PART_AOR_LEN) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId AOR, fail",__FUNCTION__);
		goto end;
	}
    OMCI_TRACE("[%s][%d] get large string instange ID = [%s]\n",__FUNCTION__,__LINE__,attrValue_ptr);
    
    largeStringME_ID = get16(attrValue_ptr);
    OMCI_TRACE("[%s][%d] get large string instange ID = [%d]\n",__FUNCTION__,__LINE__,largeStringME_ID);
    
    ret = getOneStringFromInstance(OMCI_CLASS_ID_LARGE_STRING, largeStringME_ID ,buff, &len);
    if (( OMCI_VOIP_SUCC != ret ) || (0 == len) )
        return OMCI_VOIP_FAIL;

    OMCI_TRACE("[%s][%d] get large string [%s]\n",__FUNCTION__,__LINE__,buff);
    if(setSipUserPartAOR(buff, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setSipDispName fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}
        
	/*get sip display name from current instance*/
	if(getAttributeValueByInstId(OMCI_CLASS_ID_SIP_USER_DATA, instId, 3, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId display name, fail",__FUNCTION__);
		goto end;
	}

	if(setSipDispName((char *)attrValue_ptr, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->setSipDispName, fail",__FUNCTION__);
	}
	if(setSipUserPass(instId, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->setSipUserPass, fail",__FUNCTION__);
	}
	if(setSipURI(instId, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->setSipURI, fail",__FUNCTION__);
	}
	
	ret = OMCI_VOIP_SUCC;

end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}

	return ret;

}

/*******************************************************************************************
**function name
	setMgcInfo
**description:
	set primary or secondary MGC infor by  9.9.16 MGC config data ME primary MGC or Secondary MGC attribute
 **retrun :
	-1: failure
	0:	success
**parameter:
	instId :	9.12.3 network address instance id
	flag :	0:primary, 1:secondary
********************************************************************************************/
int setMgcInfo(uint16 networkinstId, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	char attrStr[MAX_BUFFER_SIZE] = {0};	
	char attrStr2[MAX_BUFFER_SIZE] = {0};
	char tmpBuff[MAX_BUFFER_SIZE] = {0};
	uint8 *attrValue_ptr = NULL;
	uint16 largeStrInstId = 0;
	uint8 num = 0;
	uint8 * buf_ptr = NULL;
	uint8 * tmp_ptr = NULL;
	char * tmpStr_ptr = NULL;
	int index = 0;
	int endIndex = 0;
	uint8 findFlag = OMCI_VOIP_NO_FIND;
	uint32 mgcPort = 0;

	switch(flag){
		case OMCI_H248_PRIMARY_ATTR_INDEX:
		case OMCI_H248_SECNDARY_ATTR_INDEX:
			break;
		default:
			goto end;
	}
	/*calloc memory*/
	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMgcInfo-> val calloc error ");
		ret = -1;
		goto end;
	}

	/*calloc memory*/
	buf_ptr =  (uint8 *)calloc(OMCI_ME_MAX_LARGE_STR_PART_NUM , OMCI_ME_MAX_STR_ATTR_LEN);
	if(buf_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMgcInfo-> val calloc 2 error ");
		ret = -1;
		goto end;
	}

	/*get address pointer  form 9.12.3 network address ME*/	
	memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);	
	if(getAttributeValueByInstId(OMCI_CLASS_ID_NETWORK_ADDRESS, networkinstId, 2, attrValue_ptr, 2) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId large string ME instId, fail",__FUNCTION__);
		goto end;
	}
	largeStrInstId = get16(attrValue_ptr);

	/*get number of parts form 9.12.5 large string*/
	memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(getAttributeValueByInstId(OMCI_CLASS_ID_LARGE_STRING, largeStrInstId, 1, attrValue_ptr, 1) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId number of parts attribute, fail",__FUNCTION__);
		goto end;
	}
	num = *attrValue_ptr;

	if(num > OMCI_ME_MAX_LARGE_STR_PART_NUM){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->number of part %d >15, fail",__FUNCTION__, num);
		goto end;
	}

	endIndex = 2+num;
	tmp_ptr = buf_ptr;
	for(index = 2; index < endIndex; index++){		
		/*get parts val form 9.12.5 large string*/
		memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);
		if(getAttributeValueByInstId(OMCI_CLASS_ID_LARGE_STRING, largeStrInstId, index, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getAttributeValueByInstId username, fail",__FUNCTION__);
			goto end;
		}

		memcpy(tmp_ptr, attrValue_ptr, OMCI_ME_MAX_STR_ATTR_LEN);
		tmp_ptr +=OMCI_ME_MAX_STR_ATTR_LEN;
	}

	tmp_ptr = buf_ptr + (OMCI_ME_MAX_LARGE_STR_PART_NUM*OMCI_ME_MAX_STR_ATTR_LEN -1);
	*tmp_ptr = 0;

	tmp_ptr = buf_ptr;

	if(flag == OMCI_H248_PRIMARY_ATTR_INDEX){
		strcpy(attrStr,OMCI_H248_PRIMARY_MGC_IP_NAME);
		strcpy(attrStr2,OMCI_H248_PRIMARY_MGC_PORT_NAME);		
	}else if(flag == OMCI_H248_SECNDARY_ATTR_INDEX){
		strcpy(attrStr,OMCI_H248_SECONDARY_MGC_IP_NAME);
		strcpy(attrStr2,OMCI_H248_SECONDARY_MGC_PORT_NAME);
	}else{
		//nothing
	}
	
	if(strstr((char *)tmp_ptr, ":") == NULL){
		/*set primary or secondary mgc ip*/
		tcapi_set(OMCI_VOIP_H248_COMMON, attrStr, (char *)tmp_ptr);
		/*set primary or secondary mgc port*/
		tcapi_set(OMCI_VOIP_H248_COMMON, attrStr2, "");
	}else{
		tmpStr_ptr = strtok((char *)tmp_ptr, ":");
		/*set primary or secondary mgc ip*/
		tcapi_set(OMCI_VOIP_H248_COMMON, attrStr, tmpStr_ptr);

		/*set primary or secondary mgc port*/
		tmpStr_ptr = strtok(NULL, ":");
		if(tmpStr_ptr != NULL){
			mgcPort = atoi(tmpStr_ptr);
			memset(tmpBuff, 0, sizeof(tmpBuff));
			if(mgcPort != 0){
				sprintf(tmpBuff, "%d", mgcPort);
			}			
			tcapi_set(OMCI_VOIP_H248_COMMON, attrStr2, tmpBuff);
		}else{
			tcapi_set(OMCI_VOIP_H248_COMMON, attrStr2, "");
		}
		
	}
	
	if(tcapi_commit(OMCI_VOIP_H248_COMMON) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
	
end:
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}
	if(buf_ptr != NULL){
		free(buf_ptr);
	}
	
	return ret;

}


/*******************************************************************************************************************************
9.9.2 sip user data

********************************************************************************************************************************/
int omciMeInitForSipUserData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

int setSipUserDataUserPartAOR(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instId = 0;
	uint16 entryId = 0;
    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};

	if(value == NULL){
		OMCI_ERROR("\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		OMCI_ERROR("\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}
    
    ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
    if(0 != ret)
        return OMCI_VOIP_FAIL;
    
	if(flag == OMCI_CREATE_ACTION_SET_VAL){
        OMCI_TRACE("[%s][%d] set on create just return \n",__FUNCTION__,__LINE__);
		ret = OMCI_VOIP_SUCC;
		goto end;
	}
    ret = VOIP_General_LargeStringPtr_Set(value, meInstantPtr, omciAttribute, flag, buff);

    OMCI_TRACE("[%s][%d] get large string [%s]\n",__FUNCTION__,__LINE__,buff);
    
    if ( OMCI_VOIP_SUCC != ret )
        return ret;

	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
    OMCI_TRACE("[%s][%d] instId [%d]\n",__FUNCTION__,__LINE__,instId);

	/*get entry id*/
	if(getEntryIdBySipUserData(instId, &entryId) != OMCI_VOIP_SUCC){
		OMCI_ERROR("\r\n [%s]->getEntryIdBySipUserData, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}

    OMCI_TRACE("[%s][%d] entryId [%d]\n",__FUNCTION__,__LINE__,entryId);
    
	if(setSipUserPartAOR(value, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setSipDispName fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:	
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int32 getSipUserDataDispName(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint16 entryId = 0;
	uint16 instId = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	
	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*get entry id*/
	if(getEntryIdBySipUserData(instId, &entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getEntryIdBySipUserData, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}

	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY,entryId);	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_SIP_DISP_NAME, tempBuffer);
	if(strlen(tempBuffer) > OMCI_MAX_SIP_DISP_NAME_LEN){
		goto end;
	}
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	
	memcpy(attributeValuePtr, tempBuffer, OMCI_SIP_DISP_NAME_ATTR_LEN);
	
	ret = OMCI_VOIP_SUCC;	
end:
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;
}

int setSipUserDataDispName(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instId = 0;
	uint16 entryId = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		ret = OMCI_VOIP_SUCC;
		goto end;
	}

	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*get entry id*/
	if(getEntryIdBySipUserData(instId, &entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getEntryIdBySipUserData, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}

	if(setSipDispName(value, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setSipDispName fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:	
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int setSipUserDataUserPass(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instId = 0;
	uint16 entryId = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		ret = OMCI_VOIP_SUCC;
		goto end;
	}

	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*get entry id*/
	if(getEntryIdBySipUserData(instId, &entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getEntryIdBySipUserData, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}

	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->setGeneralValue, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}
	
	if(setSipUserPass(instId, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setSipUserPass fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	#if 1 // LTM
	{
		char node[32] = {0};
		sprintf(node, "%s%d", OMCI_VOIP_BASIC_ENTRY, entryId);
		tcapi_set(node, "SIPEnable", "Yes");
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "LTM: set %s SIPEnable to Yes", node);
	}
	#endif

	ret = OMCI_VOIP_SUCC;
end:
	
	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}
	return ret;

}

int setSipUserDataSipURI(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 instId = 0;
	uint16 entryId = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		ret = OMCI_VOIP_SUCC;
		goto end;
	}

	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*get entry id*/
	if(getEntryIdBySipUserData(instId, &entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->getEntryIdBySipUserData, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}	

	if(setGeneralValue(value, meInstantPtr , omciAttribute, flag) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->setGeneralValue, fail[%d]",__FUNCTION__,__LINE__);
		goto end;
	}
	
	if(setSipURI(instId, entryId) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setSipURI fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}
	return ret;

}

int32 getSipUserDataExpTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint32 expTime = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(OMCI_VOIP_ADVANCED_COMMON, OMCI_SIP_VOICEMAIL_EXPIRE, tempBuffer);

	expTime = atoi(tempBuffer);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	put32(attributeValuePtr, expTime);
	
	
	ret = OMCI_VOIP_SUCC;	
end:
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;
}


int setSipUserDataExpTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};

	if(value == NULL){
		ret = OMCI_VOIP_FAIL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		ret = OMCI_VOIP_FAIL;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer,"%d",get32((uint8 *)value));
	tcapi_set(OMCI_VOIP_ADVANCED_COMMON, OMCI_SIP_VOICEMAIL_EXPIRE, tempBuffer);
	if(tcapi_commit(OMCI_VOIP_ADVANCED_COMMON) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int setSipUserDataPPTPPointer(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;	
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint16 oldPPTPMeId = 0;
	uint16 newPPTPMeId = 0;
	uint16 instId = 0;
	uint16 entryId = 0;
	uint8 changeEntryFlag = OMCI_VOIP_NO_CHANGE_ENTRY;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	newPPTPMeId = get16((uint8 *)value);

	/*get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
		
	if(flag == OMCI_CREATE_ACTION_SET_VAL){
		changeEntryFlag = OMCI_VOIP_CHANGE_ENTRY;
	}else{		
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
		if (attributeValuePtr == NULL){
			goto end;
		}
		oldPPTPMeId = get16(attributeValuePtr);
		if(newPPTPMeId != oldPPTPMeId){
			changeEntryFlag = OMCI_VOIP_CHANGE_ENTRY;
		}
	}

	/*When create instance or create PPTP instace id, update attribute value to system*/
	if(changeEntryFlag == OMCI_VOIP_CHANGE_ENTRY){
		entryId = (newPPTPMeId & 0x00FF)-1;
		
		if(updateSipInfoByPPTP(instId, entryId) != OMCI_VOIP_SUCC){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->updateSipInfoByPPTP, fail[%d]",__FUNCTION__,__LINE__);
			goto end;
		}
	}	

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}
/*******************************************************************************************************************************
9.9.3 SIP agent config data

********************************************************************************************************************************/

// init the action function array
int omciMeInitForSIPAgentConfigData(omciManageEntity_t *omciManageEntity_p)
{
    int (**actionFunc)(uint16 , omciMeInst_t *, omciPayload_t * , uint8);
    actionFunc = omciManageEntity_p->omciActionFunc;

	actionFunc[MT_OMCI_MSG_TYPE_GET]    = omciGetAction;
	actionFunc[MT_OMCI_MSG_TYPE_SET]    = omciSetAction;
	actionFunc[MT_OMCI_MSG_TYPE_CREAT]  = omciCreateAction;
	actionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

// parse the URL string and get the addr and port
static void
Parse_URL(IN_OUT uint8 * buff /*Careful! Crash if NULL*/, 
          OUT uint8 **  addr, OUT uint8 ** port)
{
    OMCI_FUNC_TRACE_ENTER;

    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "String to Parse: %s\n", buff);
    
    *addr = buff;
    *port = NULL;
    
    char * delimter_pos = strchr(buff, ':');

    if ( NULL != delimter_pos )
    {
        * delimter_pos = '\0';
        * port = ++ delimter_pos;
    }
    OMCI_FUNC_TRACE_LEAVE;
}


/*
    The following functions set the corresponding cfg manager node's value
*/

static int
Set_SipAgentCfgData_ProxyAddr_CfgManagerNode( IN_OUT uint8 * buff /*Careful! Crash if NULL*/ )
{
    char * addr = NULL, * port = NULL;
    Parse_URL(buff, &addr, &port);

    if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPProxyAddr", addr) )
        return OMCI_VOIP_FAIL;

    if (NULL != port){
        if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPProxyPort", port) ){
            return OMCI_VOIP_FAIL;
        }
    }

    if( 0 != tcapi_commit(OMCI_VOIP_BASIC) )
        return OMCI_VOIP_FAIL;
    
    return OMCI_VOIP_SUCC;
}

static int
Set_SipAgentCfgData_OutBoundProxyAddr_CfgManagerNode( IN_OUT uint8 * buff )
{
    char * addr = NULL, * port = NULL;
    Parse_URL(buff, &addr, &port);

    if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPOutboundProxyAddr", addr) )
        return OMCI_VOIP_FAIL;

    if (NULL != port){
        if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPOutboundProxyPort", port) ){
            return OMCI_VOIP_FAIL;
        }
    }
    
    if( 0 != tcapi_commit(OMCI_VOIP_BASIC) )
        return OMCI_VOIP_FAIL;

    return OMCI_VOIP_SUCC;
}

static int
Set_SipAgentCfgData_SIPRegistrar_CfgManagerNode( IN_OUT uint8 * buff )
{
    char * addr = NULL, * port = NULL;
    Parse_URL(buff, &addr, &port);

    if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "RegistrarServer", addr) )
        return OMCI_VOIP_FAIL;

    if (NULL != port){
        if ( 0 != tcapi_set(OMCI_VOIP_BASIC_COMMON, "RegistrarServerPort", port) ){
            return OMCI_VOIP_FAIL;
        }
    }

    if( 0 != tcapi_commit(OMCI_VOIP_BASIC) )
        return OMCI_VOIP_FAIL;

    return OMCI_VOIP_SUCC;
}

/*
    Attributes' setting functions
*/

int 
Set_SipAgentCfgData_ProxyAddr(char *value, uint8 *meInstantPtr , 
                              omciAttriDescript_t * omciAttribute, uint8 flag)
{
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    if (NULL == value )
        return OMCI_VOIP_FAIL;

    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};
    
	#if 1 // LTM 
	tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPProxyEnable", "Yes");
	#endif
    
    int ret = VOIP_General_LargeStringPtr_Set(value, meInstantPtr, omciAttribute, flag, buff);
    
    if ( OMCI_VOIP_SUCC != ret )
        return ret;

    return Set_SipAgentCfgData_ProxyAddr_CfgManagerNode(buff);
}

int
Set_SipAgentCfgData_OutBoundProxyAddr(char *value, uint8 *meInstantPtr , 
                                      omciAttriDescript_t  * omciAttribute, uint8 flag)
{
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    if (NULL == value )
        return OMCI_VOIP_FAIL;

    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};

    /* If outbound SIP proxy is not used, should set to null */
    if (0 == get16(value))
        return OMCI_VOIP_SUCC;
    
    int ret = VOIP_General_LargeStringPtr_Set(value, meInstantPtr, omciAttribute, flag, buff);
    
    if ( OMCI_VOIP_SUCC != ret )
        return ret;

    return Set_SipAgentCfgData_OutBoundProxyAddr_CfgManagerNode(buff);
}

int
Get_SipAgentCfgData_SipRegExpTime(char *value, uint8 *meInstantPtr, 
                                  omciAttriDescript_t * omciAttribute, uint8 flag)
{
    if ( NULL == value )
        return OMCI_VOIP_FAIL;

    char buf[20];
    if (0 != tcapi_get(OMCI_VOIP_ADVANCED_COMMON, "RegistrationExpire", buf))
        return OMCI_VOIP_FAIL;

    uint32 regExpireTime;

    regExpireTime = strtol(buf, NULL, 10);

    if ( 0 == regExpireTime && ( EINVAL == errno || ERANGE == errno) )
        return OMCI_VOIP_FAIL;
    
    uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL){
        put32(attributeValuePtr, regExpireTime);
		memcpy(value, attributeValuePtr, 4);
		return OMCI_VOIP_SUCC;
	}
	else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"Get_SipAgentCfgData_SIPStatus: attributeValuePtr == NULL\n");
		return OMCI_VOIP_FAIL;
	}
}

int
Set_SipAgentCfgData_SipRegExpTime(char *value, uint8 *meInstantPtr , 
                                  omciAttriDescript_t * omciAttribute, uint8 flag)
{
    if (NULL == value )
        return OMCI_VOIP_FAIL;

    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    omciMeInst_ptr inst_ptr = (omciMeInst_t *) meInstantPtr;
    
    int32 ret = setGeneralValue(value, inst_ptr , omciAttribute, flag);
    
    if ( OMCI_VOIP_SUCC != ret )
        return ret;
  
    unsigned int expTime = get32(value);
    char buf[20];
    snprintf(buf, 20, "%u", expTime);
    if ( 0 != tcapi_set(OMCI_VOIP_ADVANCED_COMMON, "RegistrationExpire", buf) )
        return OMCI_VOIP_FAIL;

    if( 0 != tcapi_commit(OMCI_VOIP_ADVANCED) )
        return OMCI_VOIP_FAIL;

    return OMCI_VOIP_SUCC;
}

int
Get_SipAgentCfgData_SIPReregTime(char *value, uint8 *meInstantPtr,
                                 omciAttriDescript_t * omciAttribute, uint8 flag)
{
    if ( NULL == value )
        return OMCI_VOIP_FAIL;

    char buf[20];
    if (0 != tcapi_get(OMCI_VOIP_ADVANCED_COMMON, "RegisterRetryInterval", buf))
        return OMCI_VOIP_FAIL;

    uint32 retryInterval;

    retryInterval = strtol(buf, NULL, 10);

    if ( 0 == retryInterval && ( EINVAL == errno || ERANGE == errno) )
        return OMCI_VOIP_FAIL;
    
    uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL){
        put32(attributeValuePtr, retryInterval);
		memcpy(value, attributeValuePtr, 4);
		return OMCI_VOIP_SUCC;
	}
	else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"Get_SipAgentCfgData_SIPStatus: attributeValuePtr == NULL\n");
		return OMCI_VOIP_FAIL;
	}
}


int
Set_SipAgentCfgData_SIPReregTime(char *value, uint8 *meInstantPtr , 
                                 omciAttriDescript_t * omciAttribute, uint8 flag)
{
    if (NULL == value )
        return OMCI_VOIP_FAIL;

    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    omciMeInst_ptr inst_ptr = (omciMeInst_t *) meInstantPtr;
    
    int32 ret = setGeneralValue(value, inst_ptr , omciAttribute, flag);
    
    if ( OMCI_VOIP_SUCC != ret )
        return ret;
   
    unsigned int reregTime = get32(value);
    char buf[20];
    snprintf(buf, 20, "%u", reregTime);
    if ( 0 != tcapi_set(OMCI_VOIP_ADVANCED_COMMON, "RegisterRetryInterval", buf) )
        return OMCI_VOIP_FAIL;

    if( 0 != tcapi_commit(OMCI_VOIP_ADVANCED) )
        return OMCI_VOIP_FAIL;

    return OMCI_VOIP_SUCC;
}

int
Get_SipAgentCfgData_SIPStatus(char *value, uint8 *meInstantPtr, 
                              omciAttriDescript_t * omciAttribute, uint8 flag)
{
    if (NULL == value )
        return OMCI_VOIP_FAIL;

    char buff[30];
    int  entryIdx = 0; // todo, how to get the current voip line?
    char nodename[20];
    snprintf(nodename, 20, OMCI_VOIP_ADVANCED_ENTRY"d", entryIdx );
    
    if ( 0 != tcapi_get(nodename, "status", buff) )
        return OMCI_VOIP_FAIL;

    char status = 0;

    if ( 0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_OK_INITIAL)   || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_UP)       || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_INITING)  || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_REGING)   || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_UNREGING) || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_TESTING)  || 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_QUIESCENT)|| 
         0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_DISABELED) )
    {
         status = VOIP_SIP_STATUTS_OK_INITIAL;
    }else if (0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_CONNECTED)){
        status = VOIP_SIP_STATUTS_CONNECTED;
    }else if (0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_FAIL_ICMP_ERROR) || 
              0 == strcmp(buff, VOIP_SIP_TR69_SZ_STATUS_ERROR)){
        status = VOIP_SIP_STATUTS_FAIL_ICMP_ERROR;
    }else if (0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_FAIL_MALFORMED_RES)){
        status = VOIP_SIP_STATUTS_FAIL_MALFORMED_RES;
    }else if (0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_FAIL_INADEQUATE_INFO_RES)){
        status = VOIP_SIP_STATUTS_FAIL_INADEQUATE_INFO_RES;
    }else if (0 == strcmp(buff, VOIP_SIP_SZ_STATUTS_FAIL_TIMEOUT)){
        status = VOIP_SIP_STATUTS_FAIL_TIMEOUT;
    }else{
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "In Get_SipAgentCfgData_SIPStatus can't recognize status: %s\n", buff);
        return OMCI_VOIP_FAIL;
    }
    
    uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL){
        *value = *attributeValuePtr = status;
		return OMCI_VOIP_SUCC;
	}
	else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"Get_SipAgentCfgData_SIPStatus: attributeValuePtr == NULL\n");
		return OMCI_VOIP_FAIL;
	}
}

int Set_SipAgentCfgData_SIPRegistrar(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};
	uint8 *attrValPtr = NULL;
	uint16 instId = 0, largeStrInstId = 0;
	uint8 *attrValue_ptr = NULL;
	int ret = OMCI_VOIP_FAIL;

	if (!meInstantPtr)
		return OMCI_VOIP_NO_FIND;

	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setSipURI-> val calloc error ");
		ret = -1;
		goto end;
	}

	/* get 9.12.3 network address ME pointer */
	instId = get16(value); // networkAddress instance

	/* get large string ME from network address ME */
	memset(attrValue_ptr,0, OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(getAttributeValueByInstId(OMCI_CLASS_ID_NETWORK_ADDRESS, instId, 2, attrValue_ptr, 2) != 0){
		goto end;
	}
    
    ret = VOIP_General_LargeStringPtr_Set(attrValue_ptr, meInstantPtr, omciAttribute, flag, buff);
    if ( OMCI_VOIP_SUCC != ret )
        goto end;
	
	ret = Set_SipAgentCfgData_SIPRegistrar_CfgManagerNode(buff);

end:	
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}

    return ret;
}

int Set_SipAgentCfgData_TcpUdp(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "Set SIP agent config data attri:%s\n", omciAttribute->attriName);

    uint8 buff[LARGE_STRING_ME_MAX_LEN + 1] = {0};
	uint8 *attrValPtr = NULL;
	uint16 instId = 0, portId = 0;
	uint8 *attrValue_ptr = NULL;
    int ipHostIstId = 0;
	int ret = OMCI_VOIP_FAIL;

	if (!meInstantPtr)
		return OMCI_VOIP_NO_FIND;
    
    if(setGeneralValue(value, meInstantPtr, omciAttribute, flag) != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nSipAgentCfgData_TCPUDP: setGeneralValue fail\n ");
        goto end;
    }

	attrValue_ptr =  (uint8 *)calloc(1 , OMCI_ME_DEFAULT_STR_ATTR_LEN);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\nSipAgentCfgData_TCPUDP: Create attrVal fail\n ");
		ret = -1;
		goto end;
	}

	/* get 9.4.3 TCP/UDP ME pointer */
	instId = get16(value); // TCP/UDP instance

	/* get PortID from TCP/UDP ME */
	if(getAttributeValueByInstId(OMCI_CLASS_ID_TCP_UDP_CFGDATA, instId, 1, attrValue_ptr, 2) != 0){
		goto end;
	}

	portId = get16(attrValue_ptr);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "\r\nSipAgentCfgData_TCPUDP: get port = %d\n ", portId);

	sprintf(buff, "%d", portId);
	if (0!=tcapi_set(OMCI_VOIP_BASIC_COMMON, "SIPProxyPort", buff))
		goto end;
	/* get IP host pointer from TCP/UDP instance*/
    if(getAttributeValueByInstId(OMCI_CLASS_ID_TCP_UDP_CFGDATA, instId, 4, attrValue_ptr, 2) != 0){
		goto end;
	}
    ipHostIstId = get16(attrValue_ptr);

#if defined(TCSUPPORT_PON_IP_HOST)
    checkIPHostWanInterfaceTimer(WAN_SERVICELIST_VOICE, ipHostIstId);
#endif

	ret = OMCI_VOIP_SUCC;

end:	
	if(attrValue_ptr != NULL){
		free(attrValue_ptr);
	}

    return ret;
}

/*******************************************************************************************************************************
9.9.4  VoIP voice CTP

********************************************************************************************************************************/
 int omciMeInitForVoipVoiceCTP(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
} 

/*
    Attributes' setting functions
*/


/*******************************************************************************************************************************
9.9.5  VoIP media profile

********************************************************************************************************************************/
int omciMeInitForVoipMediaPro(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

int32 getVOIPMediaProFaxMode(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint8 faxMode= 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(OMCI_VOIP_MEDIA_COMMON, OMCI_FAX_CODEC_NAME, tempBuffer);

	if(strcmp(tempBuffer,OMCI_VOIP_ATTR_ENABLE_NAME)== 0){
		faxMode = OMCI_FAX_MODE_T38;
	}else{
		faxMode = OMCI_FAX_MODE_PASSTHRU;
	}

 	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = faxMode; 
	
	
	ret = OMCI_VOIP_SUCC;	
end:
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;
}
int setVOIPMediaProFaxMode(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 faxMode= 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	faxMode = *(uint8 *)value;
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	if(faxMode == OMCI_FAX_MODE_PASSTHRU){
		strcpy(tempBuffer,OMCI_VOIP_ATTR_DISABLE_NAME);
	}else if(faxMode == OMCI_FAX_MODE_T38){
		strcpy(tempBuffer,OMCI_VOIP_ATTR_ENABLE_NAME);
	}else{
		goto end;		
	}	
	
	tcapi_set(OMCI_VOIP_MEDIA_COMMON, OMCI_FAX_CODEC_NAME, tempBuffer);
	if(tcapi_commit(OMCI_VOIP_MEDIA_COMMON) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int32 getVOIPMediaProCodecSele(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index){
	int ret = OMCI_VOIP_FAIL;
	uint8 codecType = 0;
	uint8 entryIndex = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	char attrName[MAX_BUFFER_SIZE] = {0};
	uint8 index_pri = 0;
	char index_priBuff[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	
#if defined(TCSUPPORT_CT)
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, index);
			break;
		default:
			goto end;
	}
#else
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY, index);
			break;
		default:
			goto end;
	}
#endif
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_MEDIA_CODEC_NAME, tempBuffer);
	
	if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMA) == 0){
		codecType = OMCI_MEDIA_CODEC_PCMA;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMU) == 0){
		codecType = OMCI_MEDIA_CODEC_PCMU;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G722) == 0){
		codecType = OMCI_MEDIA_CODEC_G722;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G723) == 0){
		codecType = OMCI_MEDIA_CODEC_G723;	
	}else if(strstr(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G726) != NULL){
		codecType = OMCI_MEDIA_CODEC_G726;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G729) == 0){
		codecType = OMCI_MEDIA_CODEC_G729;
	}else{
		goto end;
	}
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = codecType; 
	
	
	ret = OMCI_VOIP_SUCC;
end:
	return ret;
}
int setVOIPMediaProCodecSele(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index){
	static int initial = 0;
	int ret = OMCI_VOIP_FAIL;
	int i = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 codecType= 0;
	uint8 entryIndex = 0;
	uint8 index_pri = 0;
	char index_priBuff[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char attrName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

#if defined(TCSUPPORT_CT)	
	memset(index_priBuff, 0, MAX_BUFFER_SIZE);
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:		
		case OMCI_MEDIA_3RD:		
		case OMCI_MEDIA_4TH:
			index_pri = index + 1;
			sprintf(index_priBuff, "%d", index_pri);
			break;	
		default:
			goto end;
	}

	codecType = *(uint8 *)value;
	if(initial == 0){
		for(i=0; i<OMCI_MEDIA_NUM; i++){
			memset(nodeName, 0, MAX_BUFFER_SIZE);
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, i);
			if(TCAPI_SUCCESS != tcapi_set(nodeName, OMCI_MEDIA_PRIORITY, "0"))
				OMCI_ERROR("initial omci media priority, fail\n");	
		}
		memset(nodeName, 0, MAX_BUFFER_SIZE);
		initial = 1;
	}
	switch(codecType){
		case OMCI_MEDIA_CODEC_G722:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, 0);
			tcapi_set(nodeName, OMCI_MEDIA_PRIORITY, index_priBuff);			
			break;
		case OMCI_MEDIA_CODEC_PCMA:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, 1);
			tcapi_set(nodeName, OMCI_MEDIA_PRIORITY, index_priBuff);
			break;
		case OMCI_MEDIA_CODEC_PCMU:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, 2);
			tcapi_set(nodeName, OMCI_MEDIA_PRIORITY, index_priBuff);			
			break;
		case OMCI_MEDIA_CODEC_G729:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, 3);
			tcapi_set(nodeName, OMCI_MEDIA_PRIORITY, index_priBuff);	
			break;		
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:codecType do not support !\n",__FUNCTION__);
			goto end;
	}
#else
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY,index);
			break;
		default:
			goto end;
	}

	codecType = *(uint8 *)value;
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	switch(codecType){
		case OMCI_MEDIA_CODEC_PCMA:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMA);
			break;
		case OMCI_MEDIA_CODEC_PCMU:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMU);
			break;
		case OMCI_MEDIA_CODEC_G722:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_G722);
			break;
		case OMCI_MEDIA_CODEC_G723:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_G723);
			break;
		case OMCI_MEDIA_CODEC_G726:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_G726);
			break;
		case OMCI_MEDIA_CODEC_G729:
			strcpy(tempBuffer, OMCI_MEDIA_SIPSUPPORTED_CODECS_G729);
			break;
		default:
			goto end;
	}
	
	tcapi_set(nodeName, OMCI_MEDIA_CODEC_NAME, tempBuffer);
#endif

	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	return ret;

}

int32 getVOIPMediaProPktPeriodSele(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index){
	int ret = OMCI_VOIP_FAIL;
	uint8 periodMs = 0;
	uint8 entryIndex = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

#if defined(TCSUPPORT_CT)
	sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC, 0);
#else
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY,index);
			break;
		default:
			goto end;

	}
#endif
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_MEDIA_SIP_PACKETIZATION_TIME_NAME, tempBuffer);
	
	periodMs = atoi(tempBuffer);

	if((periodMs < MIN_OMCI_MEDIA_PKT_PERIOD_VAL) || (periodMs > MAX_OMCI_MEDIA_PKT_PERIOD_VAL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s] periodMs=%d(10-30),error[%d]",__FUNCTION__,periodMs, __LINE__);
		goto end;
	}
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = periodMs;
	
	
	ret = OMCI_VOIP_SUCC;
end:
	return ret;
}
int setVOIPMediaProPktPeriodSele(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag, uint8 index){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 periodMs = 0;
	uint8 entryIndex = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char tempBuffer2[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	periodMs = *(uint8 *)value;
	if((periodMs < MIN_OMCI_MEDIA_PKT_PERIOD_VAL) || (periodMs > MAX_OMCI_MEDIA_PKT_PERIOD_VAL)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s] periodMs=%d(10-30),error[%d]",__FUNCTION__,periodMs, __LINE__);
		goto end;
	}

#if defined(TCSUPPORT_CT)
	sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC, 0);
#else
	switch(index){
		case OMCI_MEDIA_1ST:			
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY,index);
			break;
		default:
			goto end;

	}
#endif
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", periodMs);	
	tcapi_set(nodeName, OMCI_MEDIA_SIP_PACKETIZATION_TIME_NAME, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	return ret;

}

int32 getVOIPMediaProSlience(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag,uint8 index){
	int ret = OMCI_VOIP_FAIL;
	uint8 silenceFlag = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

#if defined(TCSUPPORT_CT)
	sprintf(nodeName, "%s%d", OMCI_VOIP_MEDIA_ENTRY, 0);
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_MEDIA_SILENCE_COMPRESSIONENABLE, tempBuffer);

	if(strcmp(tempBuffer,OMCI_MEDIA_SILENCE_ENABLE) == 0){
		silenceFlag = OMCI_MEDIA_SILENCE_ON;
	}else{
		silenceFlag = OMCI_MEDIA_SILENCE_OFF;
	}
#else
	switch(index){
		case OMCI_MEDIA_1ST:
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY,index);
			break;
		default:
			goto end;

	}
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_MEDIA_SILENCE_SUPPRESSION_NAME, tempBuffer);
	
	if(strcmp(tempBuffer,OMCI_MEDIA_VAD_ON_VAL) == 0){
		silenceFlag = OMCI_MEDIA_SILENCE_ON;
	}else{
		silenceFlag = OMCI_MEDIA_SILENCE_OFF;
	}
#endif	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = silenceFlag;
	
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute); 
	else
		return ret;

}
int setVOIPMediaProSlience(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag,uint8 index){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 silenceFlag = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

#if defined(TCSUPPORT_CT)
	sprintf(nodeName, "%s%d", OMCI_VOIP_MEDIA_ENTRY, 0);
#else
	switch(index){
		case OMCI_MEDIA_1ST:			
		case OMCI_MEDIA_2ND:
		case OMCI_MEDIA_3RD:
		case OMCI_MEDIA_4TH:
			sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_ENTRY,index);
			break;
		default:
			goto end;

	}
#endif
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	silenceFlag = *(uint8 *)value;
	
#if defined(TCSUPPORT_CT)
	switch(silenceFlag){
		case OMCI_MEDIA_SILENCE_OFF:
			strcpy(tempBuffer,OMCI_MEDIA_SILENCE_DISABLE);
			break;		
		case OMCI_MEDIA_SILENCE_ON:
			strcpy(tempBuffer,OMCI_MEDIA_SILENCE_ENABLE);
			break;
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s] silenceFlag=%d(0 or 1),error[%d]",__FUNCTION__,silenceFlag, __LINE__);
			goto end;
			
	}
	tcapi_set(nodeName, OMCI_MEDIA_SILENCE_COMPRESSIONENABLE, tempBuffer);
#else
	switch(silenceFlag){
		case OMCI_MEDIA_SILENCE_OFF:
			strcpy(tempBuffer,OMCI_MEDIA_VAD_OFF_VAL);
			break;
		case OMCI_MEDIA_SILENCE_ON:
			strcpy(tempBuffer,OMCI_MEDIA_VAD_ON_VAL);
			break;
		default:
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s] silenceFlag=%d(0 or 1),error[%d]",__FUNCTION__,silenceFlag, __LINE__);
		goto end;
			
	}
	tcapi_set(nodeName, OMCI_MEDIA_SILENCE_SUPPRESSION_NAME, tempBuffer);
#endif

	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	return ret;

}

int32 getVOIPMediaProCodecSele1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProCodecSele1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}


int32 getVOIPMediaProPktPeriodSele1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProPktPeriodSele1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProSlience1st(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;		
}
int setVOIPMediaProSlience1st(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_1ST);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}


int32 getVOIPMediaProCodecSele2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProCodecSele2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProPktPeriodSele2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProPktPeriodSele2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProSlience2nd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;		
}
int setVOIPMediaProSlience2nd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_2ND);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}


int32 getVOIPMediaProCodecSele3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProCodecSele3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProPktPeriodSele3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProPktPeriodSele3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProSlience3rd(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;		
}
int setVOIPMediaProSlience3rd(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_3RD);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}


int32 getVOIPMediaProCodecSele4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProCodecSele4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProCodecSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProPktPeriodSele4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;
		
}
int setVOIPMediaProPktPeriodSele4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProPktPeriodSele(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}

int32 getVOIPMediaProSlience4th(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = getVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return getGeneralValue(value, meInstantPtr, omciAttribute, flag);
	else
		return ret;		
}
int setVOIPMediaProSlience4th(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;

	ret = setVOIPMediaProSlience(value,meInstantPtr,omciAttribute,flag,OMCI_MEDIA_4TH);

	if(ret == OMCI_VOIP_SUCC)
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	else
		return ret;
}


int32 getVOIPMediaProOOBDtmf(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint8 dtmfFlag = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	char tempBuffer2[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	strcpy(nodeName,OMCI_VOIP_ADVANCED_COMMON);
	strcpy(tempBuffer2,OMCI_MEDIA_DTMF_MODE_NAME);

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, tempBuffer2, tempBuffer);

	if(strcmp(tempBuffer,OMCI_MEDIA_DTMF_MODE_RFC2833) == 0){
		dtmfFlag = OMCI_VOIP_PARA_ENABLE;
	}else{
		dtmfFlag = OMCI_VOIP_PARA_DISABLE;
	}
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = dtmfFlag;
	
	
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;
	
}
int setVOIPMediaProOOBDtmf(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 dtmfFlag = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char tempBuffer2[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}
	strcpy(nodeName,OMCI_VOIP_ADVANCED_COMMON);
	strcpy(tempBuffer2,OMCI_MEDIA_DTMF_MODE_NAME);

	dtmfFlag = *(uint8 *)value;
	if(dtmfFlag == OMCI_VOIP_PARA_ENABLE){
		strcpy(tempBuffer,OMCI_MEDIA_DTMF_MODE_RFC2833);
	}else{
		strcpy(tempBuffer,OMCI_MEDIA_DTMF_MODE_INBAND);
	}
	
	tcapi_set(nodeName, tempBuffer2, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}
	

}

/*******************************************************************************************************************************
9.9.13: Rtp performance monitoring history data

********************************************************************************************************************************/

int omciMeInitForRtpPMHistoryData(omciManageEntity_t * omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForRtpPerformanceMonitoringHistoryData \r\n");

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



int32 setRTPPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);
	
	if(thresholdMeId == 0 || thresholdMeId == 0xFFFF)
		return 0;
	
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 1, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 1, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 2, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 2, tempBuffer);
	if (ret != 0)
		return -1;
	
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 3, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 3, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 4, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 4, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 6, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 5, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 7, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 6, tempBuffer);
	if (ret != 0)
		return -1;
	
	return 0;	
}

int32 getRTPPMHistoryDataRtpErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getRTPPMHistoryDataPacketLoss(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
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

int32 getRTPPMHistoryDataMaxJitter(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
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

int32 getRTPPMHistoryDataMaxTimeBetRTCP(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag){
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

int32 getRTPPMHistoryDataBufferUnderflows(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getRTPPMHistoryDataBufferOverflows(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 6, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*******************************************************************************************************************************
9.9.14: SIP agent performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForSipAgentPMHistoryData(omciManageEntity_t * omciManageEntity_p)
{
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    OMCI_TRACE("enter omciMeInitForSipAgentPMHistoryData \n");

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

int32 setSipAgentPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    char tempBuffer[MAX_BUFFER_SIZE] = {0};
    omciMeInst_t *tmpomciMeInst_p = NULL;
    int ret = 0;
    uint32 thresholdData = 0;
    uint16 thresholdMeId = 0;
    uint16 length = 0;
    uint8 *attributeValuePtr = NULL;
    int i  = 0;
    ThrholdPmMap_t thrholdMap[6] = 
    {
    	{1, 2},
        {2, 3},
        {3, 4},
        {4, 5},
        {5, 6},
        {6, 7},
    };// ref to Threshold crossing alert table

    if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
        return -1;

    tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
    if(attributeValuePtr == NULL)
        return -1;
    memcpy(attributeValuePtr, value, length);
    
    //get Threshold data instance
    thresholdMeId = get16((uint8 *)value);
    
    if(thresholdMeId == 0 || thresholdMeId == 0xFFFF)
        return 0;

    for ( i = 0 ; i < 6 ; i++)
    {
        ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, thrholdMap[i].threhold_idx, &thresholdData);
        if (ret != 0)
            return -1;
        memset(tempBuffer, 0, sizeof(tempBuffer));
        snprintf(tempBuffer, MAX_BUFFER_SIZE-1, "%x", thresholdData);
        pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, thrholdMap[i].pm_idx, tempBuffer);
    }
    
    return 0;   
}


int32 getRTPPMHistoryDataRxInviteReqs(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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


int32 getRTPPMHistoryDataTxInviteReqs(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    char tempBuffer[MAX_BUFFER_SIZE] = {0};
    omciMeInst_t *tmpomciMeInst_p = NULL;
    int ret = 0;
    uint32 pmData = 0;

    if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
        return -1;

    tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 8, tempBuffer);
    if (ret != 0)
        return -1;
    
    sscanf(tempBuffer, "%x", &pmData);
    put32((uint8 *)value, pmData);

    return 0;
}

/*******************************************************************************************************************************
9.9.15: SIP call initation performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForSipCallInitPMHistoryData(omciManageEntity_t * omciManageEntity_p)
{
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForSipCallInitPMHistoryData \r\n");

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

int32 setSipCallInitPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    char tempBuffer[MAX_BUFFER_SIZE] = {0};
    omciMeInst_t *tmpomciMeInst_p = NULL;
    int ret = 0;
    uint32 thresholdData = 0;
    uint16 thresholdMeId = 0;
    uint16 length = 0;
    uint8 *attributeValuePtr = NULL;
    ThrholdPmMap_t thrholdMap[5] = 
    {
    	{1, 1},
        {2, 2},
        {3, 3},
        {4, 4},
        {5, 5},
    };// ref to Threshold crossing alert table
    int i = 0;

    if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
        return -1;

    tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
    if(attributeValuePtr == NULL)
        return -1;
    memcpy(attributeValuePtr, value, length);
    
    //get Threshold data instance
    thresholdMeId = get16((uint8 *)value);
    
    if(thresholdMeId == 0 || thresholdMeId == 0xFFFF)
        return 0;
    
    for (i =0 ;i < 5 ;i++)
    {
        ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, thrholdMap[i].threhold_idx, &thresholdData);
        if (ret != 0)
            return -1;
        memset(tempBuffer, 0, sizeof(tempBuffer));
        snprintf(tempBuffer, MAX_BUFFER_SIZE-1,"%x", thresholdData);
        pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, thrholdMap[i].pm_idx, tempBuffer);
    }
    
    return 0;   
}

int32 getVoIPPMDataCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
    char tempBuffer[MAX_BUFFER_SIZE] = {0};
    omciMeInst_t *tmpomciMeInst_p = NULL;
    int ret = 0;
    uint32 pmData = 0;
    int arrtIdx = 0;

    if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
        return -1;
    
    arrtIdx = omciAttribute->attriIndex - 2;

    tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, arrtIdx, tempBuffer);
    if (ret != 0)
        return -1;
    
    sscanf(tempBuffer, "%x", &pmData);
    put32((uint8 *)value, pmData);

    return 0;
}

/*******************************************************************************************************************************
9.9.16 MGC config data

********************************************************************************************************************************/
int omciMeInitForMgcCfgData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

int setMGCCfgDataPriMGC(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 networkinstId = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	/*get network address ME instance id*/
	networkinstId = get16((uint8 *)value);

	if(setMgcInfo(networkinstId, OMCI_H248_PRIMARY_ATTR_INDEX) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setMgcInfo fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:	
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}
int setMGCCfgDataSecMGC(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 networkinstId = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	/*get network address ME instance id*/
	networkinstId = get16((uint8 *)value);

	if(setMgcInfo(networkinstId, OMCI_H248_SECNDARY_ATTR_INDEX) != OMCI_VOIP_SUCC){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:setMgcInfo fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:	
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

#if 0
int32 getMGCCfgDataMsgFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint8 msgType = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	char attrName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	strcpy(nodeName, OMCI_VOIP_H248_ENTRY);	
	strcpy(attrName,OMCI_H248_MSG_FORMAT_NAME);
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);	
	tcapi_get(nodeName, attrName, tempBuffer);
	
	if(strcmp(tempBuffer,OMCI_H248_MSG_FORMAT_TEXT_LONG_VAL) == 0){
		msgType = OMCI_H248_MSG_FORMAT_TEXT_LONG;
	}else if(strcmp(tempBuffer,OMCI_H248_MSG_FORMAT_TEXT_SHORT_VAL) == 0){
		msgType = OMCI_H248_MSG_FORMAT_TEXT_SHORT;
	}else{
		goto end;
	}
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = msgType; 
	
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}
int setMGCCfgDataMsgFormat(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 msgType= 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char attrName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	strcpy(nodeName, OMCI_VOIP_H248_ENTRY);
	strcpy(attrName,OMCI_H248_MSG_FORMAT_NAME);
	
	msgType = *(uint8 *)value;
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	switch(msgType){
		case OMCI_H248_MSG_FORMAT_TEXT_LONG:
			strcpy(tempBuffer, OMCI_H248_MSG_FORMAT_TEXT_LONG_VAL);
			break;
		case OMCI_H248_MSG_FORMAT_TEXT_SHORT:
			strcpy(tempBuffer, OMCI_H248_MSG_FORMAT_TEXT_SHORT_VAL);
			break;
		case OMCI_H248_MSG_FORMAT_BINARY:
		default:
			goto end;
	}
	
	tcapi_set(nodeName, attrName, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}
int32 getMGCCfgDataMaxRetryTime(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint16 maxRetryTime = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	char attrName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	strcpy(nodeName, OMCI_VOIP_H248_ENTRY);	
	strcpy(attrName,OMCI_H248_MAX_RETRANS_TIME_NAME);
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);	
	tcapi_get(nodeName, attrName, tempBuffer);

	maxRetryTime = atoi(tempBuffer);
	/*tcapi node (ms), OMCI node(s)*/
	maxRetryTime = maxRetryTime/1000;
	if(maxRetryTime <= 0){
		maxRetryTime = OMCI_H248_DEFAULT_MAX_RETRANS_TIME;
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	put16(attributeValuePtr,maxRetryTime);
	
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}
int setMGCCfgDataMaxRetryTime(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint16 maxRetryTime = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char attrName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	strcpy(nodeName, OMCI_VOIP_H248_ENTRY);	
	strcpy(attrName,OMCI_H248_MAX_RETRANS_TIME_NAME);
	
	maxRetryTime = get16((uint8 *)value);
	/*tcapi node (ms), OMCI node(s)*/
	maxRetryTime = maxRetryTime*1000;
	sprintf(tempBuffer, "%d", maxRetryTime);

	tcapi_set(nodeName, attrName, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}
#endif

int32 getMGCCfgDataDevId(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	char attrName[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	strcpy(nodeName, OMCI_VOIP_H248_COMMON);	
	strcpy(attrName,OMCI_H248_DEVICE_ID_NAME);
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);	
	tcapi_get(nodeName, attrName, tempBuffer);	
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	memcpy(attributeValuePtr, tempBuffer, length);	
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}
int setMGCCfgDataDevId(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	tcapi_set(OMCI_VOIP_H248_COMMON, OMCI_H248_DEVICE_ID_NAME, value);
	if(tcapi_commit(OMCI_VOIP_H248_COMMON) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

/*******************************************************************************************************************************
9.9.6 Voice  Service profile

********************************************************************************************************************************/
int omciMeInitForVoiceServiceProfile(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

/*******************************************************************************************************************************
9.9.7 RTP profile data

********************************************************************************************************************************/
int omciMeInitForRtpProfileData(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}

/*******************************************************************************************************************************
9.9.8 VoIP application service profile

********************************************************************************************************************************/
int omciMeInitForVoipAppSvcProfile(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return OMCI_VOIP_SUCC;
}


/*******************************************************************************************************************************
9.9.18 VoIP config data

********************************************************************************************************************************/
int omciMeInitForVoipCfgData(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return OMCI_VOIP_SUCC;
}

int omciInternalCreateForVoipCfgData(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	char buffer[25] = {0};

	if(instanceId != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "voip cfg data only one instance, instaceid=%d,error!\n",instanceId);
		return OMCI_VOIP_FAIL;
	}

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

/*0---meId = 0*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForVoipCfgData: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

/*1---Available signalling protocols*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{		
		*attributeValuePtr = 0x3; //support SIP && H.248 - ONT default
	}
	else
		goto fail;

/*3---Available VoIP configuration methods*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{		
		*(attributeValuePtr+3) = 0x5; //do not configure - ONT default
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForVoipCfgData: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);
	

/*4---voip configuration method used*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 0; //do not configure - ONT default
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForVoipCfgData: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);

/*5---voip configuration address pointer=0xFFFF*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 0xFF;
		*(attributeValuePtr+1) = 0xFF;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForVoipCfgData: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, buffer);

/*6---voip configuration state*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0; //0: inactive -configuration retrieval has not been attempted.
	else
		goto fail;		
	
/*7---retrieve profile*/
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 0;
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForVoipCfgData: tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName = %s, length = %x, buffer = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName, length, buffer);


	return OMCI_VOIP_SUCC;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL \n");
	return OMCI_VOIP_FAIL;
}

int omciInitInstForVoipCfgData(void)
{
	int ret = OMCI_VOIP_FAIL;

	ret = omciInternalCreateForVoipCfgData(OMCI_CLASS_ID_VOIP_CFG_DATA, 0);/*internal create func*/
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_VOIP_CFG_DATA fail\n");

	return ret;
}

int32 getVOIPCfgDataAvaiProto(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint8 protocol = OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_NONE;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char tempBuffer[256] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);	
	tcapi_get(OMCI_VOIP_ADVANCED_COMMON, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_NAME, tempBuffer);
	if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP_VAL) != NULL){
		protocol |= OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP;
	}
	if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248_VAL) != NULL){
		protocol |= OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248;
	}
	if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP_VAL) != NULL){
		protocol |= OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP;
	}

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = protocol;
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}

int32 getVOIPCfgDataProtoUsed(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	uint8 *attributeValuePtr = NULL;	
	uint16 length;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 protocol = OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NONE;
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	char attrName[MAX_BUFFER_SIZE] = {0};	

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	strcpy(nodeName,OMCI_VOIP_DIGIT_MAP_ENTRY);
	strcpy(attrName,OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NAME);	
	tcapi_get(nodeName, attrName, tempBuffer);

	if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP_VAL) != NULL){
		protocol = OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_SIP;
	}else if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248_VAL) != NULL){
		protocol = OMCI_VOIP_CFG_DATA_PTOCOLS_USED_H248;
	}else if(strstr(tempBuffer, OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP_VAL) != NULL){
		protocol = OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_MGCP;
	}else{
		protocol = OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NONE;
	}

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	
	if(*attributeValuePtr != OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NO_OMCI){
		*attributeValuePtr = protocol;
	}

	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}
int setVOIPCfgDataProtoUsed(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 protocol = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}
	
	protocol = *(uint8 *)value;
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	switch(protocol){
		case OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_SIP:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_SIP_VAL);
			break;
		case OMCI_VOIP_CFG_DATA_PTOCOLS_USED_H248:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_H248_VAL);
			break;
		case OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_MGCP:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_SIGNALING_PROTOCOLS_MGCP_VAL);
			break;
		case OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NONE:			
			break;
		case OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NO_OMCI:
			ret = OMCI_VOIP_SUCC;
			goto end;
			break;
		default:
			goto end;
	}

	strcpy(nodeName,OMCI_VOIP_DIGIT_MAP_ENTRY);
	tcapi_set(nodeName, OMCI_VOIP_CFG_DATA_PROTOCOLS_USED_NAME, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}


int32 getVOIPCfgDataAvaVoipCfgMethods(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	uint32 methods = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	
	methods = (OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_OMCI | OMCI_VOIP_CFG_DATA_VOIP_CFG_METHOD_TR69);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	put32(attributeValuePtr,methods);
	
	
	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}

int setVOIPCfgDataMethodsUsed(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint8 cfgMethod = 0;
	char nodeName[MAX_BUFFER_SIZE] = {0};	

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}
	
	cfgMethod = *(uint8 *)value;
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	switch(cfgMethod){
		case OMCI_VOIP_CFG_DATA_CFG_METHOD_NOT_USED:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_CFG_METHOD_NOTCFG_VAL);
			break;
		case OMCI_VOIP_CFG_DATA_CFG_METHOD_USED_OMCI:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_CFG_METHOD_OMCI_VAL);
			break;
		case OMCI_VOIP_CFG_DATA_CFG_METHOD_USED_TR69:
			strcpy(tempBuffer,OMCI_VOIP_CFG_DATA_CFG_METHOD_TR69_VAL);
			break;
		default:
			goto end;
	}
	strncpy(nodeName,OMCI_VOIP_BASIC_COMMON,MAX_BUFFER_SIZE-1);
	tcapi_set(nodeName, OMCI_VOIP_CFG_DATA_CFG_METHOD_NAME, tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int32 getVOIPCfgDatProVersion(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	uint8 *attributeValuePtr = NULL;	
	uint16 length;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};	

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	strcpy(nodeName,OMCI_VOIP_DIGIT_MAP_ENTRY);	
	tcapi_get(nodeName, OMCI_VOIP_CFG_DATA_PROFILE_VERSION_NAME, tempBuffer);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	memcpy(	attributeValuePtr,tempBuffer,length);

	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}

/*******************************************************************************************************************************
9.9.10 network dial plan table

********************************************************************************************************************************/
int omciMeInitForNetDialPlanTbl(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p =omciManageEntity_p;

	if(tmpOmciManageEntity_p == NULL){
		return OMCI_VOIP_FAIL;
	}
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;		
}

int32 getNetDialPlanTblDialNumber(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	uint8 *attributeValuePtr = NULL;	
	uint16 length = 0;
	uint16 maxNum = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	omciTableAttriValue_t *tmp_ptr = NULL;
	omciManageEntity_t *me_p = NULL;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	/*get dial number location in instance*/
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}

	/*get dial plan table size and get max number in table*/
	me_p = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_DIAL_PLAN_TBL);
	if(me_p ==	NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getNetDialPlanTblDialNumber->omciGetMeByClassId fail");
		goto end;
	}

	tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex);
	if(tmp_ptr == NULL){
		maxNum = 0;
	}else{		
		maxNum = tmp_ptr->tableSize/OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
	}
	put16(attributeValuePtr,maxNum);

	ret = OMCI_VOIP_SUCC;
end:
	
	if(ret == OMCI_VOIP_SUCC)
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
	else
		return ret;

}

int setNetDialPlanTblDialFormat(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 format = 0;

	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;

	}

	format = *(uint8 *)value;
	switch(format){
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_NO_DEFINED:
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_H248:
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_NSC:
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_FORMAT_VENDOR:
			break;
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->format=%d, fail",__FUNCTION__,format);
			goto end;
	}


	ret = OMCI_VOIP_SUCC;
end:	
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		return ret; 
	}

}

int32 getNetDialPlanTblDialTbl(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	omciTableAttriValue_t *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;	
	char *tableBuffer = NULL;
	uint32 tableSize = 0;
	uint16 instanceId = 0;
	int ret = OMCI_VOIP_FAIL;
	omciManageEntity_t *me_p = NULL;

	me_p = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_DIAL_PLAN_TBL);
	if(me_p ==  NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getNetDialPlanTblDialTbl->omciGetMeByClassId fail");
		goto end;
	}
	
	tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, me_p->omciAttriDescriptList[6].attriIndex);
	if(tmp_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getNetDialPlanTblDialTbl->omciGetTableValueByIndex fail");
		tableSize = 0;
		goto end;	
	}
	
	/*1.calculate the table size*/
	tableSize = tmp_ptr->tableSize;
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getExtVlanTagOpTblValue: tableSize = %d",tableSize);

	
	/*2.get table content*/
	tableBuffer = (char *)tmp_ptr->tableValue_p;
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getNetDialPlanTblDialTbl->tableBuffer is NULL");
		goto end;
	}

	/*3. add the table to omci archtecture*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	if (omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getNetDialPlanTblDialTbl->omciAddTableContent fail");
		goto end;
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(ret != OMCI_VOIP_SUCC){
		tableSize = 0;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getExtVlanTagOpTblValue->tableSize = %d",tableSize);
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

int setNetDialPlanTblDialTbl(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = OMCI_VOIP_FAIL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;	
	omciManageEntity_t *me_p = NULL;
	uint8 *temp_ptr = NULL;
	uint8 *temp2_ptr = NULL;
	uint8 *entryValue_ptr = NULL;
	uint8 *attrValue_ptr = NULL;
	uint16 instId = 0;
	omciTableAttriValue_t *tableConent_ptr = NULL; //table content pointer
	uint32 tableSize= 0;
	uint32 length = 0;
	uint32 tempU32 = 0;
	uint8 findFlag = OMCI_VOIP_NO_FIND;
	uint8 action = OMCI_VOIP_NET_DIAL_PLAN_TBL_ADD_ACTION;
	uint16 maxNum =0, currNum = 0;

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl-> parameter is NULL, fail");
		goto end;
	}

	/*1.1 get ME pointer*/
	me_p = omciGetMeByClassId(OMCI_CLASS_ID_NETWORK_DIAL_PLAN_TBL);
	if(me_p ==  NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl-> omciGetMeByClassId , fail");
		goto end;
	}

	/*1.2 get instance id*/
	instId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	/*1.3 get action type*/
	temp_ptr = (uint8 *)value;
	action = *(temp_ptr+1);
	switch(action){
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_ADD_ACTION:
		case OMCI_VOIP_NET_DIAL_PLAN_TBL_DEL_ACTION:
			break;
		default:
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl-> action=%d , fail",action);
			goto end;
	}
	
	
	/*1.4 get current table content*/
	tableConent_ptr = omciGetTableValueByIndex(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex);
	if(tableConent_ptr == NULL){		
		tableSize = 0;
		temp2_ptr = NULL;
	}else{
		tableSize = tableConent_ptr->tableSize;
		temp2_ptr = tableConent_ptr->tableValue_p;
	}

	/*1.5 search record*/
	findFlag = OMCI_VOIP_NO_FIND;
	length = OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;	
	while(temp2_ptr  != NULL){
		currNum++; //calculate current record number in table
		if(*temp2_ptr == *temp_ptr){
			findFlag = OMCI_VOIP_FIND;//find
		}
		length += OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
		if(length > tableConent_ptr->tableSize){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setNetDialPlanTblDialTbl->length > tableConent_ptr->tableSize");
			break;
		}
		temp2_ptr += OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
	}		
	
	/*2.1 
	**find and add  rule then return success,
	**no find and delete rule then return success.
	*/
	if(findFlag == OMCI_VOIP_FIND){//find
		if(action == OMCI_VOIP_NET_DIAL_PLAN_TBL_ADD_ACTION){// add entry
			ret = OMCI_VOIP_SUCC;
			goto end;
		}
	}else{ //no find
		if(action == OMCI_VOIP_NET_DIAL_PLAN_TBL_DEL_ACTION){ //del entry
			ret = OMCI_VOIP_SUCC;
			goto end;
		}
	}

	/*2.2 get max number in attribute*/
	attrValue_ptr = (uint8 *)omciGetInstAttriByName(tmpomciMeInst_p , me_p->omciAttriDescriptList[2].attriName, &length);
	if(attrValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->omciGetInstAttriByName get max size attribute, fail!");
		goto end;
	}
	maxNum = get16(attrValue_ptr);

	/*3. prepare value for table*/
	/*3.1 malloc new space*/
	entryValue_ptr = calloc(1 , tableSize+OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN);			
	if(entryValue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->entryValue_ptr calloc failure");
		goto end;		
	}
	memset(entryValue_ptr, 0, tableSize+OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN);

	/*3.2 fill new space to table*/
	if(action == OMCI_VOIP_NET_DIAL_PLAN_TBL_DEL_ACTION){//del entry
		/*handle table in OMCI*/
		if(tableConent_ptr != NULL){
			temp2_ptr = tableConent_ptr->tableValue_p;
			temp_ptr = (uint8 *)value;
			/*search record*/
			length = OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
			while(temp2_ptr  != NULL){		
				if(*temp2_ptr == *temp_ptr){ //find and delete entry in  table	
					/*copy first part*/
					tempU32 = length - OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;					
					temp_ptr = entryValue_ptr;
					memcpy(temp_ptr, tableConent_ptr->tableValue_p, tempU32);
					temp_ptr += tempU32;
					/*copy second part*/
					temp2_ptr = tableConent_ptr->tableValue_p+length;
					tempU32 = tableConent_ptr->tableSize - length;// copy length
					if(tempU32 > 0){
						memcpy(temp_ptr, temp2_ptr, tempU32);
					}
					
					tableSize -= OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
					break;
				}
				length += OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
				if(length > tableConent_ptr->tableSize) {//no find and no happen
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->del entry and no find entry, fail");
					break;
				}
				temp2_ptr += OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;
			}
		}		
	}else {// add entry		
		/*overflow in this table*/
		if(currNum >= maxNum){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->currNum(%d)>=maxNum(%d), fail!",currNum, maxNum);
			goto end;
		}
		
		/*handle table in OMCI*/
		if(tableSize > 0){
			memcpy(entryValue_ptr, tableConent_ptr->tableValue_p, tableSize);
		}
		memcpy(entryValue_ptr+tableSize, value, OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN);
		tableSize += OMCI_VOIP_NET_DIAL_PLAN_TBL_RECORD_LEN;

	}
	
	/*3. update table content in instance table list*/
	if(tableConent_ptr != NULL){//del table content  in instance table list
		if(omciDelTableValueOfInst(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->omciDelTableValueOfInst, failure");
			goto end;
		}
	}
		
	if(tableSize == 0){//current table is empty
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setNetDialPlanTblDialTbl->current table is empty");
		ret = OMCI_VOIP_SUCC;
		goto end;
	}
#if 0 //LHS DBG
	omciDumpTable(entryValue_ptr, tableSize);
#endif
	tableConent_ptr = omciAddTableValueToInst(tmpomciMeInst_p, me_p->omciAttriDescriptList[6].attriIndex, tableSize, entryValue_ptr);
	if(tableConent_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setNetDialPlanTblDialTbl->omciAddTableValueToInst failure");
		goto end;	
	}

	ret = OMCI_VOIP_SUCC;
end:
	if(entryValue_ptr != NULL){
		free(entryValue_ptr);
	}
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setNetDialPlanTblDialTbl->tableSize = %d",tableSize);	
	return ret;
}

/*******************************************************************************************************************************
9.9.1 Physical path termination point POTS UNI

********************************************************************************************************************************/
	
int32 getPptpPotsUNIAdminStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = OMCI_VOIP_FAIL;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;
	uint8 state = OMCI_PPTP_UNI_STATE_LOCK;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	memset(nodeName, 0, MAX_BUFFER_SIZE);
	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY, 0);
	tcapi_get(nodeName, "Enable", tempBuffer);
	if(strcmp(tempBuffer,"Yes") == 0){
		state = OMCI_PPTP_UNI_STATE_UNLOCK;
	}else if(strcmp(tempBuffer,"No") == 0){
		state = OMCI_PPTP_UNI_STATE_LOCK;
	}else{
		goto end;
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = state;
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute); 
	}
	else{
		tcdbg_printf("[%s][%s][%d]setVOIPMediaProCodecSele ret == %d,error!!!\n",__FILE__,__FUNCTION__,__LINE__,ret);
		return ret;
	}
}

int32 setPptpPotsUNIAdminStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = OMCI_VOIP_FAIL;
	uint8 state = OMCI_PPTP_UNI_STATE_LOCK;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};	
	
	if(value == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->value == NULL, fail",__FUNCTION__);
		goto end;
	}
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n [%s]->tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL, fail",__FUNCTION__);
		goto end;
	}

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	memset(nodeName, 0, MAX_BUFFER_SIZE);
	sprintf(nodeName, "%s%d", OMCI_VOIP_BASIC_ENTRY, 0);
	
	state = *(uint8 *)value;
	if(state == OMCI_PPTP_UNI_STATE_UNLOCK){
		strcpy(tempBuffer, "Yes");
	}else if(state == OMCI_PPTP_UNI_STATE_LOCK){
		strcpy(tempBuffer, "No");
	}else{
		goto end;
	}
	
	tcapi_set(nodeName, "Enable", tempBuffer);
	if(tcapi_commit(nodeName) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n[%s]:tcapi_commit fail ![%d]\n",__FUNCTION__,__LINE__);
		goto end;
	}
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){		
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		tcdbg_printf("[%s][%s][%d]setVOIPMediaProCodecSele ret == %d,error!!!\n",__FILE__,__FUNCTION__,__LINE__,ret);
		return ret; 
	}
}
int32 setPptpPotsUNIARCValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 arcValue = 0;
	uint8 arcInterval = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint16 instanceId = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_ME_CLASS_ID_PPTP_POTS_UNI);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[4];

	attributeValuePtr = omciGetInstAttriByName(meInstantCurPtr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	arcInterval = (*attributeValuePtr);

	arcValue = *((uint8*)value);
	if(arcValue == 1)
		omciArcEnable(tmpomciMeInst_ptr, arcInterval);
	else if(arcValue == 0)
		omciArcDisable(tmpomciMeInst_ptr);
	else
		return -1;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = arcValue;
	
	return 0;
}

int32 setPptpPotsUNIARCIntervalValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpValue = *((uint8*)value);
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = tmpValue;
	
	return 0;
}
int32 getPptpPotsUNIRxGain(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 instanceId = 0;
	uint16 instance = 0;
	char buf[10] = {0};
	char entryBuf[24] = {0};
	uint8 tempValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);

	if(isFiberhome_551601){
		instance = instanceId - 1;
	}
	else{
		instance =instanceId & 0xFF -1;
	}

	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getPptpPotsUNIRxGain is [%d] instance=%d!\r\n", instanceId, instance);
	sprintf(entryBuf, "VoIPAdvanced_Entry%d", instance);
	if(0 != tcapi_get(entryBuf,"VoiceVolumeListen", buf)){
		return -1;
	}
	tempValue = atoi(buf);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = tempValue;

	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int32 setPptpPotsUNIRxGain(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 instanceId = 0;
	uint16 instance = 0;
	char entryBuf[24] = {0};
	char valueBuf[2] = {0};
	uint8 tempValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	if(isFiberhome_551601){
		instance = instanceId - 1;
	}
	else{
		instance =instanceId & 0xFF -1;
	}


	sprintf(valueBuf, "%d", *value);
	sprintf(entryBuf, "VoIPAdvanced_Entry%d", instance);
	if(0 != tcapi_set(entryBuf,"VoiceVolumeListen", valueBuf)){
		return -1;
	}
	tcapi_commit(entryBuf);
	
	tempValue = atoi(value);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = *((uint8 *)value);
	
	return 0;
}

int32 getPptpPotsUNITxGain(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint8 tempValue = 0;
	uint16 length = 0;
	uint16 instanceId = 0;
	uint16 instance = 0;
	char buf[10] = {0};
	char entryBuf[24] = {0};

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	if(isFiberhome_551601){
		instance = instanceId - 1;
	}
	else{
		instance =instanceId & 0xFF -1;
	}


	sprintf(entryBuf, "VoIPAdvanced_Entry%d", instance);
	if(0 != tcapi_get(entryBuf,"VoiceVolumeSpeak", buf)){
		return -1;
	}

	tempValue = atoi(buf);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = tempValue;

	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}
int32 setPptpPotsUNITxGain(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 instanceId = 0;
	uint16 instance = 0;
	char entryBuf[24] = {0};
	char valueBuf[2] = {0};
	uint8 tempValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	if(isFiberhome_551601){
		instance = instanceId - 1;
	}
	else{
		instance =instanceId & 0xFF -1;
	}


	sprintf(valueBuf, "%d", *value);
	sprintf(entryBuf, "VoIPAdvanced_Entry%d", instance);
	if(0 != tcapi_set(entryBuf,"VoiceVolumeSpeak", valueBuf)){
		return -1;
	}
	if(0 != tcapi_commit(entryBuf))
		return -1;
	
	tempValue = atoi(value);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = *((uint8 *)value);
	
	return 0;
}


int32 getPptpPotsUNIHookState(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 instanceId = 0;
	uint16 instance = 0;
	char buf[10] = {0};
	char entryBuf[20] = {0};

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	if(isFiberhome_551601){
		instance = instanceId - 1;
	}
	else{
		instance =instanceId & 0xF -1;
	}


	sprintf(entryBuf, "VoIPSysParam_Entry%d", instance);
	if(0 != tcapi_get(entryBuf, "SC_LINE_HOOK_STATE", buf)){
		strncpy(buf, "0", 2);
	}else{
        if(NULL != strstr(buf,"Off"))
    		strncpy(buf, "1", 2);
    	else
    		strncpy(buf, "0", 2);
    }

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = (uint8)atoi(buf);

	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int omciMeInitForPptpPotsUNI(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForPptpPotsUNI \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_TEST] = omciTestAction;
	
	return 0;
}
int omciInternalCreatActionForPPTPPotsUNI(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	int ret = -1;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	tmpomciMeInst_p->deviceId = instanceId;

	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*Administrative state*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Interworking TP pointer*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 0);
	else
		goto fail;

	/*ARC*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*ARC interval*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*Impedance*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Transmission path*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*Rx gain*/
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*Tx gain*/
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*Operation state*/
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*Hook state*/
	attriMask = 1<<6;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*POTS holdover time*/
	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;		
}

int omciInitInstForPPTPPotsUNI(void)
{
	int ret = 0;
	uint16 i = 0;
	uint16 insID = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_ME_CLASS_ID_PPTP_POTS_UNI\n");
	for(i=0x1; i<=pSystemCap->potsNum; i++){
		if(isFiberhome_551601)
			insID = i;
		else
			insID = ((POTS_SLOT<<8) | i);	
		omciInitInstForUniG(insID);
		ret = omciInternalCreatActionForPPTPPotsUNI(OMCI_ME_CLASS_ID_PPTP_POTS_UNI,  insID);  //internal create func	
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_ME_CLASS_ID_PPTP_POTS_UNI fail\n");
		
		ret = omciInternalCreateActionForVoIPLineStatus(OMCI_CLASS_ID_VOIP_LINE_STATUS, insID);
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_VOIP_LINE_STATUS fail\n");
	}

	return ret;

}

/*******************************************************************************************************************************
9.9.11: VoIP line status

********************************************************************************************************************************/
int32 getVoIPStatusDataCodecsUsed(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = OMCI_VOIP_FAIL;
	uint16 codecType = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char nodeName[MAX_BUFFER_SIZE] = {0};
	uint8 index = 0; 
	uint8 index_pri = 0;
	char index_priBuff[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	for(index=0; index<OMCI_MEDIA_NUM; index++){
		memset(nodeName, 0, MAX_BUFFER_SIZE);
		sprintf(nodeName, "%s%d", OMCI_VOIP_CODECS_PVC_ENTRY, index);
		memset(index_priBuff, 0, MAX_BUFFER_SIZE);
		tcapi_get(nodeName, OMCI_MEDIA_PRIORITY, index_priBuff);
		index_pri = atoi(index_priBuff);
		if(index_pri == OMCI_VOIP_LINE_USED_CODEC)
			break;
	}
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get(nodeName, OMCI_MEDIA_CODEC_NAME, tempBuffer);
	if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMA) == 0){
		codecType = OMCI_MEDIA_CODEC_PCMA;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_PCMU) == 0){
		codecType = OMCI_MEDIA_CODEC_PCMU;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G722) == 0){
		codecType = OMCI_MEDIA_CODEC_G722;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G723) == 0){
		codecType = OMCI_MEDIA_CODEC_G723;	
	}else if(strstr(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G726) != NULL){
		codecType = OMCI_MEDIA_CODEC_G726;
	}else if(strcmp(tempBuffer,OMCI_MEDIA_SIPSUPPORTED_CODECS_G729) == 0){
		codecType = OMCI_MEDIA_CODEC_G729;
	}else{
		goto end;
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	put16(attributeValuePtr , codecType);
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute); 
	}
	return ret;
}

int32 getVoIPLineStatusVoiceServerStatus(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = OMCI_VOIP_FAIL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 status = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get("InfoVoIP_Entry0", "Status", tempBuffer);
	if(strcmp(tempBuffer,"Up") == 0){
		status = OMCI_VOIP_SERVER_STATUS_REGISTERED;
	}else if(strcmp(tempBuffer,"Error") == 0){
		status = OMCI_VOIP_SERVER_STATUS_FAILED;
	}else if(strcmp(tempBuffer,"Initializing") == 0){
		status = OMCI_VOIP_SERVER_STATUS_INITIAL;	
	}else{
		status = OMCI_VOIP_SERVER_STATUS_NOTCONFIG;
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = status;
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute); 
	}
	return ret;
}

int32 getVoIPLineStatusPortSessionType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	int ret = OMCI_VOIP_FAIL;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 status = OMCI_VOIP_PORT_SESSION_TYPE_IDLE;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	if(tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL){
		goto end;
	}
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tcapi_get("VoIPSysParam_Entry0", "SC_LINE_INFO_STATUS", tempBuffer);
	if(strcmp(tempBuffer,"Connect") == 0){
		status = OMCI_VOIP_PORT_SESSION_TYPE_2WAY;
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr == NULL){
		goto end;
	}
	*attributeValuePtr = status;
	ret = OMCI_VOIP_SUCC;
end:
	if(ret == OMCI_VOIP_SUCC){
		return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute); 
	}
	return ret;
}

static inline int32 selectVoIPLineStatusCallPacketPeriod(char* buf,int index)
{
	int ret = OMCI_VOIP_FAIL;
#if defined(TCSUPPORT_CT)
	if(index == OMCI_VOIP_CALL_PACKET_PERIOD_ONE)
		ret = tcapi_get("VoIPCodecs_PVC0","SIPPacketizationTime", buf);
	else if(index == OMCI_VOIP_CALL_PACKET_PERIOD_TWO)
		ret = tcapi_get("VoIPCodecs_PVC1","SIPPacketizationTime", buf);
#else
	if(index == OMCI_VOIP_CALL_PACKET_PERIOD_ONE)
		ret = tcapi_get("VoIPAdvanced_Entry0","SIPPacketizationTime", buf);
	else if(index == OMCI_VOIP_CALL_PACKET_PERIOD_TWO)
		ret = tcapi_get("VoIPAdvanced_Entry1","SIPPacketizationTime", buf);
#endif
	return ret;
}

int32 getVoIPLineStatusCall1PacketPeriod(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 tempValue = 0;
	uint16 length = 0;
	char buf[10] = {0};
	int index = OMCI_VOIP_CALL_PACKET_PERIOD_ONE;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	if(0 != selectVoIPLineStatusCallPacketPeriod(buf, index)){
		return -1;
	}
	tempValue = atoi(buf);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	put16(attributeValuePtr , tempValue);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n  isattributeValuePtr [%s] instance=%s!\r\n", attributeValuePtr, buf);

	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int32 getVoIPLineStatusCall2PacketPeriod(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 *attributeValuePtr = NULL;
	uint16 tempValue = 0;
	uint16 length = 0;
	char buf[10] = {0};
	int index = OMCI_VOIP_CALL_PACKET_PERIOD_TWO;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	if(0 != selectVoIPLineStatusCallPacketPeriod(buf, index)){
		return -1;
	}
	tempValue = atoi(buf);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;

	put16(attributeValuePtr , tempValue);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n  isattributeValuePtr [%s] instance=%s!\r\n", attributeValuePtr, buf);

	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int omciInternalCreateActionForVoIPLineStatus(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = -1;

	/*create instace*/
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInternalCreateActionForVoIPLineStatus omciGetMeByClassId->failure");
		goto end;
	}

	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n omciInternalCreateActionForVoIPLineStatus omciAllocInstance->failure");
		goto end;
	}
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	/*set instance id  to new instace*/
	put16(tmpomciMeInst_p->attributeVlaue_ptr, instanceId);

	ret = 0;	
end: 
	
	if(ret != 0){
		if(tmpomciMeInst_p != NULL){
			omciDelInstance(tmpOmciManageEntity_p, tmpomciMeInst_p);
		}
	}
	return ret;
}

int omciMeInitForVoIPLineStatus(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForVoIPLineStatus \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	
	return 0;
}

/*******************************************************************************************************************************
9.9.12: Call Control performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForCallCtrlPMHistory(omciManageEntity_t * omciManageEntity_p)
{
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    OMCI_TRACE("enter omciMeInitForCallCtrlPMHistory \n");
    
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

ThrholdPmMap_t gCallCtrlMap[CALL_CTRL_MAX_ATTRI_NUM] = 
{
	{THRHOLD_SETUP_FAILURE,         PM_SETUP_FAILURE},
    {THRHOLD_SETUP_TIMER,           PM_SETUP_TIMER},
    {THRHOLD_TERMINATE_FAILURE,     PM_TERMINATE_FAILURE},
    {THRHOLD_PORT_RELEASE,          PM_PORT_RELEASE},
    {THRHOLD_PORT_OFFHOOK_FAILURE,  PM_PORT_OFFHOOK_FAILURE},
};
    
int32 setCallCtrlPMThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;
    uint8 index = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);
	
	if(thresholdMeId == 0 || thresholdMeId == 0xFFFF)
		return 0;

    for(index = 0;index < CALL_CTRL_MAX_ATTRI_NUM; index++)
    {
		ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, gCallCtrlMap[index].threhold_idx, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		snprintf(tempBuffer,MAX_BUFFER_SIZE, "%x", thresholdData);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"%s pmmgr set attri %d \n",__FUNCTION__,gCallCtrlMap[index].threhold_idx);
		ret = pmmgrTcapiSet(tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, gCallCtrlMap[index].pm_idx, tempBuffer);
		if (ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"ERROR %s pmmgr set attri %d fail \n",__FUNCTION__,gCallCtrlMap[index].threhold_idx);
			return -1;
		}
    }
    
	return 0;
}

int32 getPMCommonFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;
	uint8	pmIndex = gCallCtrlMap[omciAttribute->attriIndex - 3].pm_idx;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"[Err ]%s para is NULL \n",__FUNCTION__);
		return -1;
	}
	tcdbg_printf("%s pmmgr get attri %d name %s\n",__FUNCTION__,pmIndex,omciAttribute->attriName);
	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, pmIndex, tempBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"[Err ]%s pmmgr get attri %d \n",__FUNCTION__,pmIndex);
		return -1;
	}
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*******************************************************************************************************************************
VoIP Call statistics
********************************************************************************************************************************/
#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_VOIP)
int omciMEInitForVoIPCallStatistics(omciManageEntity_t *omciManageEntity_p)
{
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForPptpPotsUNI \r\n");

    if(omciManageEntity_p == NULL)
        return -1;
    
    tmpOmciManageEntity_p = omciManageEntity_p;
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT]   = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET]     = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET]     = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT]= omciGetNextAction;
    
    return 0;
}

int32 getCallHistoryTable(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
    omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
    uint16 instanceId = 0;//instance id
    uint32 tableSize = 0;
    uint8  callNum=0,i;
    char *tableBuffer = NULL;
    callHistory_ptr callHistory_p;
    int ret = -1;

    omciTableAttriValue_t *tableConent_ptr = NULL;
    omciManageEntity_t *me_p=NULL;

    /*get instance id*/
    instanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
 	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getCallHistoryTable->instanceId = 0x%0x",instanceId);

    /*get history call num*/
    callNum=32;  // maxium is 32, set 32 because pots no exist! Jason
    
    /*1.calculate the table size*/
    tableSize = sizeof(callHistory_t) * callNum; // 120* n bytes
    
    /*2.malloc memory for table*/
    tableBuffer = (char *)calloc(1,tableSize);
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getCallHistoryTable->calloc failure");
		goto end;
	}
    /*3.get call history and fill table*/
    callHistory_p = (callHistory_ptr)tableBuffer;
  
    for(i=0;i<callNum;i++) {
        memset(callHistory_p[i].date,0,8);
        memset(callHistory_p[i].time,0,8);
        callHistory_p[i].callDuration=-1;
        memset(callHistory_p[i].calledNumber,0,8);
        memset(callHistory_p[i].callingNumber,0,8);
        callHistory_p[i].RTPTxPackets      =-1;
        callHistory_p[i].RTPRxPackets      =-1;
        callHistory_p[i].RTPRxPacketsLost  =-1;
        callHistory_p[i].RTPPacketsDiscards=-1;
        callHistory_p[i].RTPOverruns       =-1;
        callHistory_p[i].RTPUnderruns      =-1;
        callHistory_p[i].AverageJitter     =-1;
        callHistory_p[i].RTCPParticipation =0xFF;
        callHistory_p[i].PeakJitter        =-1;
        callHistory_p[i].AverageJitterBufferDepth =-1;
        callHistory_p[i].RTCP_XRParticipation  =0xFF;
        callHistory_p[i].AverageMOS        =-1;
        callHistory_p[i].PeakRoundTripDelay=-1;
        callHistory_p[i].AverageRoundTripDelay =-1;
    }
    ret = omciAddTableContent(tmpomciMeInst_p->classId,instanceId,tableBuffer,tableSize,omciAttribute->attriName);
    if(ret != 0){
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getCallHistoryTable->tableConent_ptr is NULL, fail");
        ret = -1;
        goto end;
    }
    if (tableBuffer != NULL) {
        free(tableBuffer);
    }
    return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
end:
    tableSize = 0;
    if (tableBuffer != NULL){
        free(tableBuffer);
    }
    return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}
#endif

#define classMask   0x30
#define modeMask    0x80
#define potsIdMask  0xff
uint8 pptp_pots_uni_test_support(pthreadArgType *arg)
{
#ifdef TCSUPPORT_VOIP    
    uint8 ret = OMCI_CMD_SUCCESS;
    uint8 potsId = 0;
    char buf[32] = {0};
    char node[32] = {0};
    uint8 testType = 0;
    
    testType = arg->msgContent[0];
    
    if(0 == (testType & classMask)>>4)   /*Test class 0*/
    {
        if(0 == (testType & modeMask)>>4)   /*normal mode*/
        {
            if(0!=tcapi_get(OMCI_VOIP_BASIC_COMMON, "SIPProtocol", buf)){
                return OMCI_CMD_ERROR;
            }
            
            potsId = ((arg->meInstId)&(potsIdMask)-1);
            if(!strcmp(buf, "H.248"))   /*H.248 protocal*/
            {
                sprintf(node, OMCI_VOIP_INFO_H248_ENTRY, potsId);
                if(0!=tcapi_get(node,"lineInfoStatus", buf)){
                    return OMCI_CMD_ERROR;
                }
                
                if(strcmp(buf, "Idle")){
                    return OMCI_DEVICE_BUSY;
                }
            }
            else    /*SIP protocal*/
            {
                sprintf(node, OMCI_VOIP_SYS_PARAM_ENTRY, potsId);
                if(0!=tcapi_get(node, "SC_LINE_INFO_STATUS", buf)){
                    return OMCI_CMD_ERROR;
                }
                
                if(strcmp(buf, "Idle")){
                    return OMCI_DEVICE_BUSY;
                }
            }
        }
    }
    else if(1 == (testType & classMask)>>4)  /*Test class 1*/
    {
        /*reserve for class 1 operation*/
    }
    else{
        ret = OMCI_CMD_NOT_SUPPORT;
    }
    return ret;
#else
    return OMCI_CMD_NOT_SUPPORT;
#endif    
}

/*POTS test reslut offset defination*/
#define OFS_MLT_RESULT              0
#define OFS_SELF_VEND_TEST          1
#define OFS_MAKE_BREAK_FLAGS        2
#define OFS_POWER_FLAGS             3
#define OFS_LOOP_DC_FLAGS           4
#define OFS_LOOP_AC_FLAGS           5
#define OFS_LOOP_R_FLAGS_1          6
#define OFS_LOOP_R_FLAGS_2          7
#define OFS_TIME_DRAW               8
#define OFS_TIME_BREAK              9
#define OFS_DIAL_TONE_MEAS          10
#define OFS_CHANNEL_MEAS            11
#define OFS_TG_DC                   12
#define OFS_RG_DC                   14
#define OFS_TG_AC                   16
#define OFS_RG_AC                   17
#define OFS_TG_DC_R                 18
#define OFS_RG_DC_R                 20
#define OFS_TR_DC_R                 22
#define OFS_RINGER                  24
#define OFS_POINTER_PURPOSE         25
#define OFS_TR_V_FLAGS              27
#define OFS_TR_AC                   28
#define OFS_TR_DC                   29

/*Slic Verdor defination*/
#define Zarlink 1
#define Silicon 2

/*POTS test defination*/
#define ALL_MLT         0x00
#define HIGH_VOLTAGE    0x01
#define EMF             0x02
#define RESISTANCE      0x03
#define OFF_HOOK        0x04
#define RINGER          0x05
#define SELF_TEST       0x07
#define DIAL_TONE       0x08

#ifdef TCSUPPORT_VOIP
void PPTP_POTS_UNITestRst_HIGH_VOLTAGE(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{
    uint8 *tmp = NULL;
    int data[8] = {0};
    FILE * stream;

    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_DC_FLAGS;
    *tmp = (*tmp)|(0x3f);
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_AC_FLAGS;
    *tmp = (*tmp)|(0x3f);                       
    
    system("echo 0 1 >/proc/fxs/sliclinetest");
    stream = fopen("/proc/fxs/sliclinetest", "r");
    /*ACTip ACRing DCTip DCRing*/
    fscanf(stream, "%d %d %d %d", &data[0], &data[1], &data[2], &data[3]);
    fclose(stream);

    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_TG_DC;
    put16(tmp, ((data[2]/1000)==0)?1:(data[2]/1000));
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_RG_DC;
    put16(tmp, ((data[3]/1000)==0)?1:(data[3]/1000));

    if((data[2]>40000)){
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_DC_FLAGS;
        *tmp = (*tmp)&(0x1f);
    }
    if((data[3]>40000)){
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_DC_FLAGS;
        *tmp = (*tmp)&(0x3b);
    }
    
    if(Zarlink == slicVendor)
    {
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_TG_AC;
        *tmp = ((data[0]/1000)==0)?1:(data[0]/1000);
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_RG_AC;
        *tmp = ((data[1]/1000)==0)?1:(data[1]/1000);
        if(data[0]>50000){
            tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_AC_FLAGS;
            *tmp = (*tmp)&(0x1f);
        }
        if(data[1]>50000){
            tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_AC_FLAGS;
            *tmp = (*tmp)&(0x3b);
        }
    }
    else
    {
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_TG_AC;
        *tmp = ((sqrt(data[0])*100/1000)==0)?1:(sqrt(data[0])*100/1000);
        tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_RG_AC;
        *tmp = ((sqrt(data[1])*100/1000)==0)?1:(sqrt(data[1])*100/1000);
        if(sqrt(data[0])*100>50000){
            tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_AC_FLAGS;
            *tmp = (*tmp)&(0x1f);
        }
        if(sqrt(data[1])*100>50000){
            tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_AC_FLAGS;
            *tmp = (*tmp)&(0x3b);
        }
    }
}

void PPTP_POTS_UNITestRst_EMF(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{}

void PPTP_POTS_UNITestRst_RESISTANCE(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{
    uint8 *tmp = NULL;
    int data[8] = {0};
    FILE * stream;
    
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_R_FLAGS_1;
    *tmp = (*tmp)|(0x3f);
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_R_FLAGS_2;
    *tmp = (*tmp)|(0x38);

    if(Zarlink == slicVendor)
    {
       system("echo 0 4 >/proc/fxs/sliclinetest");
       stream = fopen("/proc/fxs/sliclinetest", "r");
    }
    else
    {
       system("echo 0 3 >/proc/fxs/sliclinetest");
       stream = fopen("/proc/fxs/sliclinetest", "r");
    }
    /*TIP_RESISTANCE RING_RESISTANCE TIP_RING_RESISTANCE*/
    fscanf(stream, "%d %d %d", &data[0], &data[1], &data[2]);
    fclose(stream);

    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_TG_DC_R;
    put16(tmp, (data[0]/10/1000)<0xffff?(data[0]/10/1000):0xffff);
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_RG_DC_R;
    put16(tmp, (data[0]/10/1000)<0xffff?(data[0]/10/1000):0xffff);
    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_TR_DC_R;
    put16(tmp, (data[0]/10/1000)<0xffff?(data[0]/10/1000):0xffff);

    if(data[0]/10<20000){
       tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_R_FLAGS_1;
       *tmp = (*tmp)|(0x1f);
    }
    if(data[1]/10<20000){
       tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_R_FLAGS_1;
       *tmp = (*tmp)|(0x3b);
    }
    if(data[2]/10<20000){
       tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_LOOP_R_FLAGS_2;
       *tmp = (*tmp)|(0x1f);
    }   
}

void PPTP_POTS_UNITestRst_OFF_HOOK(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{}

void PPTP_POTS_UNITestRst_RINGER(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{}

void PPTP_POTS_UNITestRst_SELF_TEST(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{
    uint8 *tmp = NULL;

    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_SELF_VEND_TEST;
    *tmp = 0x02;
}

void PPTP_POTS_UNITestRst_DIAL_TONE(omciBaselinePayload_t *tstPktBaseline, uint8 slicVendor)
{}

void omciPPTP_POTS_UNITestRstClass0(omciBaselinePayload_t *tstPktBaseline, pthreadArgType *pthreadArg)
{
    uint8 *tmp = NULL;
    int data[8] = {0};
    uint8 mask = 0;
    uint8 slicVendor = 0;
    FILE * stream;
    
    mask = (pthreadArg->msgContent[0]) & (0x0f);

    stream = fopen("/proc/fxs/slicType", "r");
    fscanf(stream, "%s", &data[0]);
    fclose(stream);

    if(('s'==data[0])&&('i'==data[1])){
        slicVendor = Silicon;
    }
    else{
        slicVendor = Zarlink;
    }

    tmp = (uint8 *)(tstPktBaseline->msgContent)+OFS_MLT_RESULT;
    *tmp = 0x3f;
    
    if((ALL_MLT == mask)||(HIGH_VOLTAGE == mask)){
        PPTP_POTS_UNITestRst_HIGH_VOLTAGE(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(EMF == mask)){           
        /*reserve for EMF test*/
        PPTP_POTS_UNITestRst_EMF(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(RESISTANCE == mask)){
       PPTP_POTS_UNITestRst_RESISTANCE(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(OFF_HOOK == mask)){        
        /*reserve for OFF_HOOK test*/
        PPTP_POTS_UNITestRst_OFF_HOOK(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(RINGER == mask)){           
        /*reserve for TINGER test*/
        PPTP_POTS_UNITestRst_RINGER(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(SELF_TEST == mask)){       
        /*reserve for SELF_TEST test*/
        PPTP_POTS_UNITestRst_SELF_TEST(tstPktBaseline, slicVendor);
    }
    
    if((ALL_MLT == mask)||(DIAL_TONE == mask)){
        /*reserve for DIAL_TONE test*/
        PPTP_POTS_UNITestRst_DIAL_TONE(tstPktBaseline, slicVendor);
    } 
}

void omciPPTP_POTS_UNITestRstClass1(omciBaselinePayload_t *tstPktBaseline, pthreadArgType *pthreadArg)
{
}
#endif

int omciPPTP_POTS_UNITestRstMsgHandleBaseline(pthreadArgType *pthreadArg)
{
#ifdef TCSUPPORT_VOIP
	uint8 *tmp = NULL;    
	omciPayload_t tstPkt;
	omciBaselinePayload_t *tstPktBaseline=NULL;
    
	if(!TCSUPPORT_VOIP_VAL)
		return 0;

	if (pthreadArg == NULL)
		return -1;
	memset(&tstPkt, 0, sizeof(omciBaselinePayload_t));
	tstPktBaseline = (omciBaselinePayload_t *)(&tstPkt);
	
	tstPktBaseline->tranCorrelatId = pthreadArg->tranCorrelatId;
	tmp = (uint8*)&tstPktBaseline->meId;
	put16(tmp, OMCI_ME_CLASS_ID_PPTP_POTS_UNI);
	put16(tmp+2, pthreadArg->meInstId);
	tstPktBaseline->msgType = MT_OMCI_MSG_TYPE_TEST_RST;
	tstPktBaseline->devId = OMCI_BASELINE_MSG;
    
    if(0==((0x10)&(pthreadArg->msgContent[0])))/*class 0*/
    {
        omciPPTP_POTS_UNITestRstClass0(tstPktBaseline, pthreadArg);
    }
    else if(1==((0x10)&(pthreadArg->msgContent[0])))/*class 1*/
    {
        /*reserve for class 1 operation*/
        omciPPTP_POTS_UNITestRstClass1(tstPktBaseline, pthreadArg);
    }
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"testType error!");
		return -1;
	}

	return	omciInternalNotificationPktSend(&tstPkt);
#else
    return 0;
#endif
}

#ifdef TCSUPPORT_OMCI_EXTENDED_MSG
int omciPPTP_POTS_UNITestRstMsgHandleExtended(pthreadArgType *pthreadArg)
{
#ifdef TCSUPPORT_VOIP
	omciPayload_t tstPkt;
	omciExtendedPayload_t *tstPktExtended=NULL;	
	uint8 *tmp = NULL;

	if(!TCSUPPORT_VOIP_VAL)
		return 0;

	memset(&tstPkt, 0, sizeof(tstPkt));
	tstPktExtended = (omciExtendedPayload_t *)(&tstPkt);
	tstPktExtended->tranCorrelatId = (uint16)pthreadArg->tranCorrelatId;
	tmp = (uint8*)(&(tstPktExtended->meId));
	put16(tmp, OMCI_ME_CLASS_ID_PPTP_POTS_UNI);
	put16(tmp+2, pthreadArg->meInstId);
	tstPktExtended->msgType = MT_OMCI_MSG_TYPE_TEST_RST;
	tstPktExtended->devId = OMCI_EXTENED_MSG;	
	
	if((pthreadArg->msgContent[0] == 0x07) ||((pthreadArg->msgContent[0] & 0x08) != 0) )/* test*/
	{
/*send test result packets*/
		/*PPTP_POTS UNI  test MLT drop test result  14*/
		tmp = (uint8 *)(tstPktExtended->msgContent);
		*tmp = 1;
		
		/*PPTP_POTS UNI  test MLT result of test    15*/
		tmp = tmp + 1;
		*tmp = 1;
		/*PPTP_POTS UNI  test dial tone make-break flags  16*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test dial tone power flags  17*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test loop test DC voltage flags  18*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test loop test AC voltage flags  19*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test loop test resistance flags 1  20*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test loop test resistance flags 2  21*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test Time to draw dial tone 22*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test Time to break dial tone 23*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test total dial tone power pwasurement 24*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test quiet channel power measurement 25*/
		tmp = tmp + 1;
		*tmp = 0;
		
		/*PPTP_POTS UNI  test Tip-ground DC voltage 26-27*/
		tmp = tmp + 1;
		put16(tmp, 0);

		/*PPTP_POTS UNI  test Ring-ground DC volatage 28-29*/
		tmp = tmp + 2;
		put16(tmp, 0);

		/*PPTP_POTS UNI  test Tip-ground AC voltage 30*/
		tmp = tmp + 2;
		*tmp = 0;

		/*PPTP_POTS UNI  test Ring-ground DC volatage 31*/
		tmp = tmp + 1;
		*tmp = 0;

		/*PPTP_POTS UNI  test Tip-ground DC resistance 32-33*/
		tmp = tmp + 1;
		put16(tmp, 0);

		/*PPTP_POTS UNI  test Ring-ground DC resistance 34-35*/
		tmp = tmp + 2;
		put16(tmp, 0);

		/*PPTP_POTS UNI  test Tip-ring  DC resistance 36-37*/
		tmp = tmp + 2;
		put16(tmp, 0);

		/*PPTP_POTS UNI  test Ringer equivalence 38*/
		tmp = tmp + 2;
		*tmp = 0;

		/*PPTP_POTS UNI  test Pointer to a general purpose buffer 39-40*/
		tmp = tmp + 1;
		put16(tmp, 0);
		
	}		
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"testType error!");
		return -1;
	}
	
	omciPktSend(&tstPkt, TRUE);
#endif
	return 0;
}
#endif

