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
	omci_me_multicast_func.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_multicast.h"
#include "omci_me_mac_bridge.h"
#include "libcompileoption.h"

#ifndef TCSUPPORT_XPON_IGMP

/*******************************************************************************************************************************
IGMP API

********************************************************************************************************************************/
int igmpONUGetVer(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetVer->port=%d",port);
	return 3;
}

int igmpONUSetVer(uint16 port,uint8 igmpVer){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetVer->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetVer->igmpVer=%d",igmpVer);
	return 0;
}

int igmpONUGetFunc(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetFunc->port=%d",port);
	return 3;
}

int igmpONUSetFunc(uint16 port,uint8 igmpFuncType){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetFunc->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetFunc->igmpFuncType=%d",igmpFuncType);	
	return 0;
}

int igmpONUGetFastLeave(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetFastLeave->port=%d",port);
	return 0;
}

int igmpONUSetFastLeave(uint16 port,uint8 fastLeaveEnabled){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetFastLeave->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetFastLeave->fastLeaveEnabled=%d",fastLeaveEnabled);
	return 0;
}

int igmpONUGetUpstreamTCI(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetUpstreamTCI->port=%d",port);
	return 0;
}

int igmpONUSetUpstreamTCI(uint16 port,uint16 tci){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetUpstreamTCI->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetUpstreamTCI->tci=%d",tci);
	return 0;
}

int igmpONUGetTagCtrl(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetTagCtrl->port=%d",port);
	return 0;
}

int igmpONUSetTagCtrl(uint16 port,uint8 type){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetTagCtrl->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetTagCtrl->type=%d",type);
	return 0;
}


int igmpONUGetMaxRate(uint16 port, uint32 * maxRate){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetMaxRate->port=%d",port);
	return 0;
}

int igmpONUSetMaxRate(uint16 port, uint32 maxRate){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetMaxRate->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetMaxRate->maxRate=%d",maxRate);
	return 0;
}

int igmpONUClearMaxRate(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUClearMaxRate->port=%d",port);
	return 0;
}

int igmpONUSetDyCtrList(uint16 port, omciIgmpRuleInfo_ptr ruleRecord){
	int i = 0;
#if 0	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->port=%d",port);
	if(ruleRecord!=NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->rowKey=0x%0x",ruleRecord->rowKey);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->ruleType=0x%0x",ruleRecord->ruleType);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->gemPortId=0x%0x",ruleRecord->gemPortId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->vid=0x%0x",ruleRecord->vid);

		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->srcIp[%d]=0x%0x",i,ruleRecord->srcIp[i]);
		}
		
		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->startDestIp[%d]=0x%0x",i,ruleRecord->startDestIp[i]);
		}
		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->endDestIp[%d]=0x%0x",i,ruleRecord->endDestIp[i]);
		}
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetDyCtrList->ruleRecord->imputedBandwidth=0x%0x",ruleRecord->imputedBandwidth);

	}
#endif
	return 0;
}

int igmpONUDelDyCtrList(uint16 port, uint16 ruleKey){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUDelDyCtrList->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUDelDyCtrList->ruleKey=%d",ruleKey);
	return 0;
}

int igmpONUClearDyCtrList(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUClearDyCtrList->port=%d",port);
	return 0;
}

int igmpONUSetStaticCtrList(uint16 port, omciIgmpRuleInfo_ptr ruleRecord){
#if 0	
	int i = 0;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->port=%d",port);

	if(ruleRecord!=NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->rowKey=0x%0x",ruleRecord->rowKey);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->ruleType=0x%0x",ruleRecord->ruleType);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->gemPortId=0x%0x",ruleRecord->gemPortId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->vid=0x%0x",ruleRecord->vid);
		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->srcIp[%d]=0x%0x",i,ruleRecord->srcIp[i]);
		}
		
		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->startDestIp[%d]=0x%0x",i,ruleRecord->startDestIp[i]);
		}
		for(i=0; i<16; i++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->endDestIp[%d]=0x%0x",i,ruleRecord->endDestIp[i]);
		}

		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUSetStaticCtrList->ruleRecord->imputedBandwidth=0x%0x",ruleRecord->imputedBandwidth);
		
	}
#endif
	return 0;
}

int igmpONUDelStaticCtrList(uint16 port, uint16 ruleKey){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUDelStaticCtrList->port=%d",port);
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUDelStaticCtrList->ruleKey=%d",ruleKey);
	
	return 0;
}

int igmpONUClearStaticCtrList(uint16 port){
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUClearStaticCtrList->port=%d",port);
	return 0;
}

int igmpONUGetActiveGroupList(IN uint16 portId, IN uint16 maxEntryNum, IN uint8 type, OUT uint8 *groupListNum, OUT omciIgmpActiveGroupInfo_ptr rule_ptr){
//	uint8 maxGroup = 0;
	omciIgmpActiveGroupInfo_ptr interRule_ptr = NULL;
	omciIgmpActiveGroupInfo_ptr temp = NULL;
	uint16 realLen = 0;
	int ret = -1;
//	int i = 0;
//	int j = 0;

	realLen = 2*OMCI_ACTIVE_MLTICAST_GROUP_RECORD_SIZE;
	if(maxEntryNum < 2){
		ret = -1;
		goto end;
	}

	 interRule_ptr = (omciIgmpActiveGroupInfo_ptr)calloc(2 , sizeof(omciIgmpActiveGroupInfo_t));
	 temp = interRule_ptr;
	/*vid*/
	temp->vid = 0x1234;
	/*src ip*/
	temp->srcIp[12] = 0x01;
	temp->srcIp[13] = 0x02;
	temp->srcIp[14] = 0x03;
	temp->srcIp[15] = 0x04;
	/*dest ip*/
	temp->multiDestIp[12] = 0x05;
	temp->multiDestIp[13] = 0x06;
	temp->multiDestIp[14] = 0x07;
	temp->multiDestIp[15] = 0x08;
	/*best bandwidth*/
	temp->bestBandWidth = 0x09;
	/*client ip*/
	temp->clientIp[12] = 0x0a;
	temp->clientIp[13] = 0x0b;
	temp->clientIp[14] = 0x0c;
	temp->clientIp[15] = 0x0d;
	/*time*/
	temp->time = 0x0e; 
#if 1
	temp = interRule_ptr+1;
	/*vid*/
	temp->vid = 0x5678;
	/*src ip*/
	temp->srcIp[12] = 0x11;
	temp->srcIp[13] = 0x22;
	temp->srcIp[14] = 0x13;
	temp->srcIp[15] = 0x14;
	/*dest ip*/
	temp->multiDestIp[12] = 0x15;
	temp->multiDestIp[13] = 0x16;
	temp->multiDestIp[14] = 0x17;
	temp->multiDestIp[15] = 0x18;
	/*best bandwidth*/
	temp->bestBandWidth = 0x19;
	/*client ip*/
	temp->clientIp[12] = 0x1a;
	temp->clientIp[13] = 0x1b;
	temp->clientIp[14] = 0x1c;
	temp->clientIp[15] = 0x1d;
	/*time*/
	temp->time = 0x1e; 
#endif	
	*groupListNum = 2;
	memcpy(rule_ptr , interRule_ptr , realLen);
#if 0
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetActiveGroupList->portId=%d",portId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetActiveGroupList->maxEntryNum=%d",maxEntryNum);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetActiveGroupList->*groupListNum=%d",*groupListNum);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n API igmpONUGetActiveGroupList->val");
	for(i = 0; i<*groupListNum; i++){
		temp = rule_ptr+i;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n *groupListNum=%d",*groupListNum);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->vid=0x%02x",temp->vid);
		for(j=0; j<16; j++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->srcIp[%d]=0x%02x",j,temp->srcIp[j]);
		}
		for(j=0; j<16; j++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->multiDestIp[%d]=0x%02x",j,temp->multiDestIp[j]);
		}
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->bestBandWidth=0x%02x",temp->bestBandWidth);
		for(j=0; j<16; j++){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->clientIp[%d]=0x%02x",j,temp->clientIp[j]);
		}
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->time=0x%02x",temp->time);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, " \r\n temp->reserved=0x%02x",temp->reserved);		
	}
#endif
	ret = 0;
end:
	if(interRule_ptr != NULL){
		free(interRule_ptr);
	}
	if(ret != 0){
		ret = -1;
	}
	return ret;
}
#endif
/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	getLanPortByMeType
**description:
	get lan port information according 9.3.28 or 9.3.29  MeType attribute.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instanceId :	9.3.28 multicast subscriber config info ME or 9.3.29 multicast subscriber monitor  instance id
  	meType:	9.3.28 or 9.3.29 meType value
  	portFlag: 1:get a lan port id, 2: need set all lan port.
  	portId: when port Flag is 1, return lan port id.
  	
********************************************************************************************/
int getLanPortByMeType(IN uint16 instanceId, IN uint8 meType, OUT uint8 *portFlag, OUT uint16 *portId){
	int ret = -1;
	int ifaceType = 0;
	uint8 tempPortId = 0;
	uint8 ifcType = 0;

	if((meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0)
		|| (meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_1)){
		//nothing
	}else{
		ret = -1;
		goto end;
	}

	if((portFlag == NULL) || (portId == NULL)){
		ret = -1;
		goto end;
	}

	if(meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0){//9.3.4 mac bridge port config data ME
		/*get mac bridge port interface type*/
		ifaceType = getMacBridgePortInstType(instanceId, &ifcType);
		if(ifaceType == -1){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> getMacBridgePortInstType fail");
			goto end;
		}
		/*get  mac bridge port  ANI/UNI port id*/
		if(ifaceType == OMCI_MAC_BRIDGE_PORT_LAN_IFC){//lan, UNI port
			ret = getMacBridgeLanPortId(instanceId, &tempPortId);
			if(ret != 0){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> getMacBridgeLanPortId fail");
				goto end;
			}
			*portId = (uint16)tempPortId;
			*portFlag = OMCI_MULTICAST_ONE_PORT_FLAG;
		}else if(ifaceType == OMCI_MAC_BRIDGE_PORT_WAN_IFC){//wan , ANI port
			*portId = 0;
			*portFlag  = OMCI_MULTICAST_ALL_PORT_FLAG;
		}else{
			//no happen
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> getMacBridgePortInstType  fail, no happen");
			goto end;
		}
				
	}else if(meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_1 ){//9.3.10 802.1p
		*portId = 0;
		*portFlag  = OMCI_MULTICAST_ALL_PORT_FLAG;
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> me type attribute value error, fail");
		goto end;
	}
	
	ret = 0;
end:
#if 0
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> ret=0x%02x",ret);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> instanceId =0x%02x",instanceId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> meType =0x%02x",meType);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> *portId=0x%02x",*portId);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getLanPortByMeType-> *portFlag=0x%02x",*portFlag);
#endif
	if(ret != 0){
		ret = -1;
	}
	return ret;	
}
/*******************************************************************************************
**function name
	setMOPMulticastByPort
**description:
	set multicast parameter according to 9.3.27 instance.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instId :	9.3.27 multicast operations profile instance id
  	port: 	lan port id
  	type: 
  		bit	description
  		0	version
  		1	function 
  		2	fast leave
  		3	upstream tci
  		4	tag ctrl
  		5	max rate
  		6	dynamic acl list
  		7 	static acl list
  		9	Robustness
  		10	Querier IP address
  		11 	Query interval
  		12	Query max response time
  		13	Last member query interval
  		14	Unauthorized join request
  		15	Downstream IGMP and TCI

********************************************************************************************/
int setMOPMulticastByPort(uint16 instanceId, uint16 port, uint16 type){
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	int ret = -1;
	omciManageEntity_t *me_p = NULL;
	omciMeInst_t *meInst_p = NULL;
	omciIgmpRuleInfo_t ruleRecordInfo;
	omciIgmpRuleInfo_ptr ruleRecord = &ruleRecordInfo;
	omciTableAttriValue_t *tableConent_ptr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	uint16 msciInstanceId = 0;
	uint16 msciAssMopInstanceID = 0;
	uint16 mbpcdInstanceId = 0;
	uint16 mbpcdEqualMsciInstanceId = 0;
	uint8 mbpcdTpType = 0;
	uint8 *tmp_ptr = NULL;
	uint16 	ctrlField_tmp = 0;
	uint16	rowPart = 0;
	uint16	rowID = 0;
	uint8 tempValU8 = 0;
	uint16 tempValU16 = 0;
	uint32 tempValU32 = 0;
	uint16 attriMask = 0;
	uint16 ruleType = 0;

	/*we should first find all 9.3.28 instances meId by 9.3.27 meId*/
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instanceId =%x, port=%x, clearType=%x\n",__LINE__, __func__, instanceId, port, type);

	if((type&OMCI_MULTICAST_VER_TYPE)
		|| (type&OMCI_MULTICAST_FUNC_TYPE)
		|| (type&OMCI_MULTICAST_FAST_LEAVE_TYPE)
		|| (type&OMCI_MULTICAST_UP_TCI_TYPE)
		|| (type&OMCI_MULTICAST_TAG_CTRL_TYPE)		
		|| (type&OMCI_MULTICAST_MAX_RATE_TYPE)
		|| (type & OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_STATIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		|| (type&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		|| (type&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE)
	){
		//nothing		
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]type =%x\n",__LINE__, __func__, type);
		ret = -1;
		goto end;
	}
	
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_OPERATE_PROFILE);
	if(me_p ==  NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciIGMPCofigInfoEntity_p == NULL\n",__LINE__, __func__);
		goto end;
	}
	meInst_p = omciGetInstanceByMeId(me_p , instanceId);
	if(meInst_p == NULL)
	{
#ifdef TCSUPPORT_OMCI_CTC
		me_p = omciGetMeByClassId(OMCI_CLASS_ID_CTC_EXTENDED_MUTICAST_PROFILE);
		if(me_p ==  NULL){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciIGMPCofigInfoEntity_p == NULL\n",__LINE__, __func__);
			goto end;
		}
		meInst_p = omciGetInstanceByMeId(me_p , instanceId);
		if(meInst_p == NULL)
		{
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]meInst_p == NULL\n",__LINE__, __func__);
			goto end;
		}
#else
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]meInst_p == NULL\n",__LINE__, __func__);
		goto end;
#endif
	}
	if((attributeValuePtr=meInst_p->attributeVlaue_ptr) == NULL)
	{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]attributeValuePtr == NULL\n",__LINE__, __func__);
		goto end;
	}

	if(TCSUPPORT_XPON_IGMP_CHT_VAL)
	{
		currentME = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO);
		for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
		{
			msciInstanceId = get16(meInstantCurPtr->attributeVlaue_ptr);

			ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO,msciInstanceId,2,&msciAssMopInstanceID,2);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMOPMulticastByPort -> getAttributeValueByInstId fail ");
				ret = -1;
				goto end;
			}

			if(msciAssMopInstanceID == instanceId)
			{
				mbpcdEqualMsciInstanceId = msciInstanceId;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setMOPMulticastByPort--break---");
				break;
			}
		}

		currentME = omciGetMeByClassId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA);
		for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
		{
			mbpcdInstanceId = get16(meInstantCurPtr->attributeVlaue_ptr);

			if(mbpcdEqualMsciInstanceId == mbpcdInstanceId)
			{
				ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_CON_DATA,mbpcdInstanceId,3,&mbpcdTpType,1);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setMOPMulticastByPort -> getAttributeValueByInstId fail ");
					ret = -1;
					goto end;
				}

				break;
			}
		}

		if(OMCI_MULTICAST_MBPCD_TP_TYPE_VEIP == mbpcdTpType)
			/*fix veip to xponigmp port index 1*/
			port = 1;
		else
			/*update port+1 for igmp API(1~4)*/
			port +=1;
	}
	else
		port +=1;
	
	/*set igmp version by port*/
	if(type&OMCI_MULTICAST_VER_TYPE){
		/*get multicast version from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_VER_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);			
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
		
		/*set version by port*/
#if 0   /*olt don't really want to configure port version,only ask onu for whelther support this version */
		if(igmpONUSetVer(port, tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetVer fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}

	/*set igmp function by port*/
	if(type&OMCI_MULTICAST_FUNC_TYPE){
		/*get multicast function from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_FUNC_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);			
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);

		/*set multicast function by port*/
		if(igmpONUSetFunc(port, tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFunc fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}

	/*set igmp fast leave by port*/
	if(type&OMCI_MULTICAST_FAST_LEAVE_TYPE){
		/*get multicast fast leave from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_FAST_LEAVE_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);			
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
		
		/*set multicast fast leave by port*/
		if(igmpONUSetFastLeave(port, tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFunc fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}

	/*set igmp upstream tci  by port*/
	if(type&OMCI_MULTICAST_UP_TCI_TYPE){
		/*get multicast upstream tci from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_UP_TCI_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);			
		tempValU16 = get16(attributeValuePtr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU16 == %x\n",__LINE__, __func__, tempValU16);
		
		/*set multicast upstream tci by port*/
		if(igmpONUSetUpstreamTCI(port, tempValU16) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetUpstreamTCI fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}

	/*set igmp tag ctrl type  by port*/
	if(type&OMCI_MULTICAST_TAG_CTRL_TYPE){
		/*get multicast tag ctrl  from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_TAG_CTRL_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);			
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
		
		/*set tag ctrl  by port*/
		if(igmpONUSetTagCtrl(port, tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetUpstreamTCI fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}
	
	/*set max rate by port*/
	if(type&OMCI_MULTICAST_MAX_RATE_TYPE){
		/*get maxRate from instance*/
		attriMask = 1<<(16-OMCI_MULTICAST_UP_RATE_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p , attriMask, &length);			
		tempValU32 = get32(attributeValuePtr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]maxRate == 0x%04x\n",__LINE__, __func__, tempValU32);

		/*set maxRate by port*/
		if(igmpONUSetMaxRate(port, tempValU32) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetMaxRate fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}

	/*Set dynamic ACL ruleRecord from instance || Set static ACL ruleRecord from instance*/	
	if((type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE) ||(type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE)){
		if(type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE){
			/*get dynamic ACL ruleRecord from instance*/
			tableConent_ptr = omciGetTableValueByIndex(meInst_p, OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX);
			ruleType = OMCI_MULTICAST_IGMP_RULE_TYPE_DYN;
		} else {
			/*get static ACL ruleRecord from instance*/
			tableConent_ptr = omciGetTableValueByIndex(meInst_p, OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX);
			ruleType = OMCI_MULTICAST_IGMP_RULE_TYPE_STATIC;
		}
		if(tableConent_ptr != NULL){		
			/*Set dynamic ACL ruleRecord from instance*/	
			length = OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
			for(tmp_ptr = tableConent_ptr->tableValue_p; tmp_ptr != NULL; 	tmp_ptr += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH)
			{
				ctrlField_tmp = get16(tmp_ptr);
				
				rowPart = ((ctrlField_tmp&OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_MASK)>>OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
				if(rowPart == OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART0)
				{
					rowID = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_AMD1_MASK);
				}
				else
				{
					rowID = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_MASK);
				}
				
				memset(ruleRecord, 0, sizeof(omciIgmpRuleInfo_t));
				ruleRecord->rowKey = rowID;
				ruleRecord->ruleType = ruleType;
				ruleRecord->gemPortId = get16(tmp_ptr+2);
				ruleRecord->vid = get16(tmp_ptr+4);					
				memcpy(ruleRecord->srcIp,tmp_ptr+6, OMCI_MULTICAST_IPV4_LEN);	
				memcpy(ruleRecord->startDestIp,tmp_ptr+10, OMCI_MULTICAST_IPV4_LEN);
				memcpy(ruleRecord->endDestIp,tmp_ptr+14, OMCI_MULTICAST_IPV4_LEN);
				ruleRecord->imputedBandwidth = get32(tmp_ptr+18);

				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]ruleRecord rowKey:%x,ruleType:%x,gemPortId:%x,vid:%x,srcIp:%x,startDestIp:%x,endDestIp:%x\n",
						__LINE__, __func__, ruleRecord->rowKey,ruleRecord->ruleType,ruleRecord->gemPortId,ruleRecord->vid, 
						get32(ruleRecord->srcIp), get32(ruleRecord->startDestIp), get32(ruleRecord->endDestIp));
				
				/*set acl rule by port*/
				ret = igmpONUSetDyCtrList(port, ruleRecord);
				if(ret != 0){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetDyCtrList fail\n",__LINE__, __func__);
				}
				
				length += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
				if(length > tableConent_ptr->tableSize){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]length:%d > tableConent_ptr->tableSize:%d",__LINE__, __func__,length, tableConent_ptr->tableSize);
					break;
				}
			}
		}
	}

	/*set igmp robustness type by port*/
	if(type&OMCI_MULTICAST_ROBUSTNESS_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_ROBUSTNESS_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);

#ifdef TCSUPPORT_XPON_IGMP
		if(igmpONUSetRobustness(port,tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetRobustness fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}

	/*set igmp query ip type by port*/
	if(type&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE){
		char queryip[16] = {0};
		memset(queryip,0,16);
		attriMask = 1<<(16-OMCI_MULTICAST_QUERIER_IP_ADDRESS_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		memcpy(queryip, attributeValuePtr, 4);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %02x%02x%02x%02x\n",__LINE__, __func__, *attributeValuePtr,*(attributeValuePtr+1),*(attributeValuePtr+2),*(attributeValuePtr+3));

#ifdef TCSUPPORT_XPON_IGMP	
		if(igmpONUSetQueryIp(port,queryip) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetRobustness fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif		
	}
	
	/*set igmp query interval type by port*/
	if(type&OMCI_MULTICAST_QUERY_INTERVAL_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_QUERY_INTERVAL_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU32 = get32(attributeValuePtr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
#ifdef TCSUPPORT_XPON_IGMP	
		if(igmpONUSetQueryInterval(port,tempValU32) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryInterval fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif

	}
	
	/*set max response time type by port*/		
	if(type&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU32 = get32(attributeValuePtr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
#ifdef TCSUPPORT_XPON_IGMP		
		if(igmpONUSetQueryMaxRespTime(port,tempValU32) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryMaxRespTime fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}
	
	/*set last query interval type by port*/
	if(type&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_LAST_QUERY_INTERVAL_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU32 = get32(attributeValuePtr);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
#ifdef TCSUPPORT_XPON_IGMP	
		if(igmpONUSetLastQueryInterval(port,tempValU32) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetLastQueryInterval fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}
	
	/*set Unauthorized join request type by port*/ 
	if(type&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_UNAUTHORIZED_JOIN_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU8 = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
		
#ifdef TCSUPPORT_XPON_IGMP	
		if(igmpONUSetUnauthorized(port,tempValU8) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetLastQueryInterval fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}
	
	/*set Downstream IGMP and TCI type by port*/
	if(type&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE){
		attriMask = 1<<(16-OMCI_MULTICAST_DOWN_IGMP_AND_TCI_ATTR_INDEX);
		attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);		
		tempValU8 = *attributeValuePtr;
		tempValU16 = get16(attributeValuePtr+1);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x, tempValU16 == %x\n",__LINE__, __func__, tempValU8, tempValU16);
#ifdef TCSUPPORT_XPON_IGMP		
		if(igmpONUSetDownTCIType(port,tempValU8, tempValU16) != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetDownTCIType fail\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
#endif
	}

	ret = 0;
end:
	if(ret != 0){
		ret = -1;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret =%x\n",__LINE__, __func__, ret);
	return ret;	
}

/*******************************************************************************************
**function name
	setRealMulticastRuleByMeType
**description:
	set multicast rule according to 9.3.28 me type
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instId :	9.3.28 multicast subscriber config info ME or 9.3.4 mac bridge port ME  instance id
  	meType: 	
  		0:	9.3.28 me type attribute, pointer to 9.3.4 mac bridge port config data ME
  		1:	9.3.28 me type attribute, pointer to 9.3.10 802.1p ME
  	type: 
  		bit	description
  		0	version
  		1	function 
  		2	fast leave
  		3	upstream tci
  		4	tag ctrl
  		5	max rate
  		6	dynamic acl list
  		7 	static acl list
  		9	Robustness
  		10	Querier IP address
  		11 	Query interval
  		12	Query max response time
  		13	Last member query interval
  		14	Unauthorized join request
  		15	Downstream IGMP and TCI

********************************************************************************************/
int setRealMulticastRuleByMeType(uint16 instanceId, uint8 meType, uint16 type){
	int ret = -1;
	uint8 portFlag = 0;
	uint16 portId = 0;	
	int i = 0;
	uint16 multicastProfileInstId = 0;//9.3.27
	uint8 *attributeValuePtr = NULL;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instanceId =%d, meType=%d, clearType=%d\n",__LINE__, __func__, instanceId, meType, type);
	if((meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0)
		|| (meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_1)){
		//nothing
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]meType fail, portFlag =%d\n",__LINE__, __func__);
		goto end;
	}

	if((type & OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_STATIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_MAX_RATE_TYPE)
		|| (type&OMCI_MULTICAST_VER_TYPE)
		|| (type&OMCI_MULTICAST_FUNC_TYPE)
		|| (type&OMCI_MULTICAST_FAST_LEAVE_TYPE)
		|| (type&OMCI_MULTICAST_UP_TCI_TYPE)
		|| (type&OMCI_MULTICAST_TAG_CTRL_TYPE)		
		|| (type&OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		|| (type&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		|| (type&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE)
	){
		//nothing
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]type fail, \n",__LINE__, __func__);
		goto end;
	}

	/*get 9.3.28  multicast subscriber config info ME multicast operations profile pointer attribute*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO, instanceId, 2);
	if(attributeValuePtr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]attributeValuePtr NULL, \n",__LINE__, __func__);
		ret = -1;
		goto end;
	}
	/*get multicast operations profile Me instance id*/ 
	multicastProfileInstId = get16(attributeValuePtr);
		
	/*get lan  port information*/
	ret = getLanPortByMeType(instanceId, meType, &portFlag, &portId);
	if(ret != 0){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]getLanPortByMeType fail \n",__LINE__, __func__);
		goto end;
	}

	/*real set multicast rule*/
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG){
		ret = setMOPMulticastByPort(multicastProfileInstId, portId, type);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastByPort fail portId = %d\n",__LINE__, __func__, portId);
		}		
	}else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
		for(i = 0; i<OMCI_LAN_PORT_NUM; i++){
			portId = i;
			ret = setMOPMulticastByPort(multicastProfileInstId, portId, type);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastByPort fail portId = %d\n",__LINE__, __func__, portId);
			}
		}
			
	}else{
		//nothing
	}

	ret = 0;
end:
	if(ret != 0){
		ret = -1;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret =%d\n",__LINE__, __func__, ret);
	return ret;
}

/*******************************************************************************************
**function name
	clearRealMulticastRuleByMeType
**description:
	clear real multicast  rule according 9.3.28 MeType attribute.
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	instanceId :	9.3.28 multicast subscriber config info ME or 9.3.4 mac bridge port config data ME  instance id
  	meType:	9.3.28 meType value
  	clearType: 
  		bit	description
  		0	version
  		1	function 
  		2	fast leave
  		3	upstream tci
  		4	tag ctrl
  		5	max rate
  		6	dynamic acl list
  		7 	static acl list
  		9	Robustness
  		10	Querier IP address
  		11 	Query interval
  		12	Query max response time
  		13	Last member query interval
  		14	Unauthorized join request
  		15	Downstream IGMP and TCI

 ********************************************************************************************/

int clearRealMulticastRuleByMeType(uint16 instanceId, uint8 meType, uint16 clearType){
	int ret = -1;
	uint8 portFlag = 0;
	uint16 portId = 0;	
	int i = 0;
	uint8 tempPortNum = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instanceId =%d, meType=%d, clearType=%d\n",__LINE__, __func__, instanceId, meType, clearType);
	if((meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0)
		|| (meType == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_1)){
		//nothing
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]meType fail, portFlag =%d\n",__LINE__, __func__);
		goto end;
	}

	if((clearType&OMCI_MULTICAST_VER_TYPE)
		|| (clearType&OMCI_MULTICAST_FUNC_TYPE)
		|| (clearType&OMCI_MULTICAST_FAST_LEAVE_TYPE)
		|| (clearType&OMCI_MULTICAST_UP_TCI_TYPE)
		|| (clearType&OMCI_MULTICAST_TAG_CTRL_TYPE)
		|| (clearType&OMCI_MULTICAST_MAX_RATE_TYPE)	
		|| (clearType&OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (clearType&OMCI_MULTICAST_STATIC_ACL_TYPE)
		|| (clearType&OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		|| (clearType&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		|| (clearType&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		|| (clearType&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE)
	){
		//nothing
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]clearType fail\n",__LINE__, __func__);
		goto end;
	}

	/*get lan  port information*/
	if(getLanPortByMeType(instanceId, meType, &portFlag, &portId) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]getLanPortByMeType fail\n",__LINE__, __func__);
		ret = 0;
		goto end;
	}
	
	/*real clear acl rule*/
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG){
		tempPortNum = 1;		
	}else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
		tempPortNum = OMCI_LAN_PORT_NUM;
	}else{
		//nothing
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]portFlag fail, portFlag =%d\n",__LINE__, __func__, portFlag);
		ret = -1;
		goto end;
	}
	/*call clean multicast API*/
	for(i = 0; i<tempPortNum; i++){
		if(tempPortNum == OMCI_LAN_PORT_NUM){
			portId = i+1; //igmp valid value (1~4)
		}else{
			portId += 1; //igmp valid value (1~4)
		}

#ifdef TCSUPPORT_XPON_IGMP
		/*set igmp version to igmp version 2*/
		if(clearType&OMCI_MULTICAST_VER_TYPE){
			if(igmpONUSetVer(portId,OMCI_MULTICAST_VERSION_MLD_V2) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetVer fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp function to snooping*/
		if(clearType&OMCI_MULTICAST_FUNC_TYPE){
			if(igmpONUSetFunc(portId,OMCI_MULTICAST_DEFAULT_MODE) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFunc fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp disable fast leave*/
		if(clearType&OMCI_MULTICAST_FAST_LEAVE_TYPE){
			if(igmpONUSetFastLeave(portId,OMCI_MULTICAST_DEFAULT_FAST_LEAVE) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFastLeave fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp upstream TCI to 0*/
		if(clearType&OMCI_MULTICAST_UP_TCI_TYPE){
			if(igmpONUSetUpstreamTCI(portId,OMCI_MULTICAST_DEFAULT_UP_TCI) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetUpstreamTCI fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp upstream tag ctrl to transparent*/
		if(clearType&OMCI_MULTICAST_TAG_CTRL_TYPE){
			if(igmpONUSetTagCtrl(portId,OMCI_MULTICAST_TAG_CTRL_TRANSPARENT) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetTagCtrl fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*clear max rate limit according to the port id*/
		if(clearType&OMCI_MULTICAST_MAX_RATE_TYPE){
			if(igmpONUClearMaxRate(portId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUClearMaxRate fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*clean dynamic acl */
		if(clearType&OMCI_MULTICAST_DYNAMIC_ACL_TYPE){
			if(igmpONUClearDyCtrList(portId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUClearDyCtrList fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}						
		}
		/*clean static  acl */
		if(clearType&OMCI_MULTICAST_STATIC_ACL_TYPE){
			if(igmpONUClearStaticCtrList(portId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUClearStaticCtrList fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}					
		/*set igmp robustness to default 0*/
		if(clearType&OMCI_MULTICAST_ROBUSTNESS_TYPE){
			if(igmpONUSetRobustness(portId,OMCI_MULTICAST_DEFAULT_ROBUSTNESS) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetRobustness fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp query ip to default 0.0.0.0*/
		if(clearType&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE){
			char queryip[16] = {0};
			if(igmpONUSetQueryIp(portId,queryip) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryIp fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set igmp query interval to default 125s*/
		if(clearType&OMCI_MULTICAST_QUERY_INTERVAL_TYPE){
			if(igmpONUSetQueryInterval(portId,OMCI_MULTICAST_DEFAULT_QUERY_INTERVAL) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryInterval fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set max response time to default 100s*/		
		if(clearType&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE){
			if(igmpONUSetQueryMaxRespTime(portId,OMCI_MULTICAST_DEFAULT_QUERY_MAX_RESPONSE) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryMaxRespTime fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set last query interval to default 10s*/
		if(clearType&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE){
			if(igmpONUSetLastQueryInterval(portId,OMCI_MULTICAST_DEFAULT_LAST_QUERY_INTERVAL) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetLastQueryInterval fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set Unauthorized join request to default disable*/ 
		if(clearType&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE){
			if(igmpONUSetUnauthorized(portId,OMCI_MULTICAST_UNAUTHORIZED_JOIN_DISABLE) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetUnauthorized fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
		/*set Downstream IGMP and TCI to default transparent*/
		if(clearType&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE){
			if(igmpONUSetDownTCIType(portId,OMCI_MULTICAST_DOWN_TAG_CTRL_TRANSPARENT, 0) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetDownTCIType fail, portId=%d\n",__LINE__, __func__, portId);
				ret = -1;
				goto end;
			}
		}
#endif
	}	

	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret =%d\n",__LINE__, __func__, ret);
	if(ret != 0){
		ret = -1;
	}
	return ret;
}
/*******************************************************************************************
**function name
	clearRealMulticastRuleByProfileType
**description:
	clear multicast  ACL list according to multicast profile and type value
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	currInstId :	9.3.27 multicast operations profile instance id
  	clearType:   		
  		bit	description
  		0	version
  		1	function 
  		2	fast leave
  		3	upstream tci
  		4	tag ctrl
  		5	max rate
  		6	dynamic acl list
  		7 	static acl list
  		9	Robustness
  		10	Querier IP address
  		11 	Query interval
  		12	Query max response time
  		13	Last member query interval
  		14	Unauthorized join request
  		15	Downstream IGMP and TCI
********************************************************************************************/

int clearRealMulticastRuleByProfileType(IN uint16 currInstId, IN uint16 clearType){
	int ret = -1;
	omciManageEntity_ptr me_p = NULL;
	omciMeInst_t *inst_p = NULL;
	uint16 tempInstId = 0;
	uint16 tempMulticastProfileInstId = 0;
	uint8 tempMeType = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, currInstId =%d, type=%d\n",__LINE__, __func__, currInstId, clearType);

	if((clearType&OMCI_MULTICAST_VER_TYPE)
		|| (clearType&OMCI_MULTICAST_FUNC_TYPE)
		|| (clearType&OMCI_MULTICAST_FAST_LEAVE_TYPE)
		|| (clearType&OMCI_MULTICAST_UP_TCI_TYPE)
		|| (clearType&OMCI_MULTICAST_TAG_CTRL_TYPE)
		|| (clearType&OMCI_MULTICAST_MAX_RATE_TYPE)	
		|| (clearType&OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (clearType&OMCI_MULTICAST_STATIC_ACL_TYPE)
		|| (clearType&OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		|| (clearType&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		|| (clearType&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		|| (clearType&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		|| (clearType&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE)
	){
		//nothing		
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]type fail, type=%d\n",__LINE__, __func__, clearType);
		ret = -1;
		goto end;
	}
		
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]me_p fail\n",__LINE__, __func__);
		ret = -1;
		goto end;
	}
	for (inst_p = me_p->omciInst.omciMeInstList; inst_p != NULL; inst_p = inst_p->next)
	{
		tempInstId = get16(inst_p->attributeVlaue_ptr); //9.3.28 inst id
		tempMeType = *(inst_p->attributeVlaue_ptr+2); //9.3.28 ME type attribute
		tempMulticastProfileInstId = get16(inst_p->attributeVlaue_ptr+3); //9.3.28 multicast operations profile pointer attribute

		if(tempMulticastProfileInstId == currInstId){			
			ret = clearRealMulticastRuleByMeType(tempInstId, tempMeType, clearType);			
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]clearRealMulticastRuleByMeType fail\n",__LINE__, __func__);
			}			
		}
	}
	
	ret = 0;
end:
	if(ret != 0){
		ret = -1;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret =%d\n",__LINE__, __func__, ret);
	return ret;
}


/*******************************************************************************************
**function name
	setRealMulticastRuleByProfile
**description:
	set multicast value by calling multicast API
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	currInstId :	9.3.27 multicast operations profile instance id
  	type: 
  		bit	description
  		0	version
  		1	function 
  		2	fast leave
  		3	upstream tci
  		4	tag ctrl
  		5	max rate
  		6	dynamic acl list
  		7 	static acl list
  		9	Robustness
  		10	Querier IP address
  		11 	Query interval
  		12	Query max response time
  		13	Last member query interval
  		14	Unauthorized join request
  		15	Downstream IGMP and TCI


  	aclRuleAction: acl rule action type, 1:add a rule, 2:delete a rule according to the ruleRecord->rowkey
  	val_ptr: max rate or version or function type, fast leave type, upstream tci type, tag ctrl type value
  	ruleRecord: igmp rule value
********************************************************************************************/
int setRealMulticastRuleByProfile(IN uint16 currInstId, IN uint16 type, IN uint8 aclRuleAction, IN char *val_ptr, IN omciIgmpRuleInfo_t *ruleRecord){
	int ret = -1;
	omciManageEntity_ptr me_p = NULL;
	omciMeInst_t *inst_p = NULL;
	uint16 tempInstId = 0;
	uint16 tempMulticastProfileInstId = 0;
	uint8 tempMeType = 0;
	int i = 0;
	uint16 portId = 0;
	uint8 opFlag = 0;
	uint8 tempValU8 = 0;
	uint16 tempValU16 = 0;
	uint32 tempValU32 = 0;
	uint8 maxPortNum = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, currInstId =%d, type=%x\n",__LINE__, __func__, currInstId, type);

	if((type & OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_STATIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_MAX_RATE_TYPE)
		|| (type&OMCI_MULTICAST_VER_TYPE)
		|| (type&OMCI_MULTICAST_FUNC_TYPE)
		|| (type&OMCI_MULTICAST_FAST_LEAVE_TYPE)
		|| (type&OMCI_MULTICAST_UP_TCI_TYPE)
		|| (type&OMCI_MULTICAST_TAG_CTRL_TYPE)		
		|| (type&OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		|| (type&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		|| (type&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		|| (type&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE)
	){
		//nothing		
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]type fail type = %x\n",__LINE__, __func__, type);
		ret = -1;
		goto end;
	}

	if((type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		|| (type&OMCI_MULTICAST_STATIC_ACL_TYPE)){
		if(ruleRecord == NULL){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]ruleRecord= NULL\n",__LINE__, __func__);
			goto end;
		}

		if((aclRuleAction == OMCI_IGMP_ACL_TABLE_ENTRY_WRITE_ACTION)
			|| (aclRuleAction == OMCI_IGMP_ACL_TABLE_ENTRY_DEL_ACTION)){
			//nothing
		}else{
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]aclRuleAction = %x\n",__LINE__, __func__, aclRuleAction);
			goto end;
		}
	}else{
		if(val_ptr == NULL){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]val_ptr = NULL%x\n",__LINE__, __func__);
			ret = -1;
			goto end;
		}
	}
	
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]me_p fail\n",__LINE__, __func__);
		ret = -1;
		goto end;
	}
	for (inst_p = me_p->omciInst.omciMeInstList; inst_p != NULL; inst_p = inst_p->next)
	{
		opFlag = 0;
		tempInstId = get16(inst_p->attributeVlaue_ptr); //9.3.28 inst id
		tempMeType = *(inst_p->attributeVlaue_ptr+2); //9.3.28 ME type attribute
		tempMulticastProfileInstId = get16(inst_p->attributeVlaue_ptr+3); //9.3.28 multicast operations profile pointer attribute
		
		if(tempMulticastProfileInstId == currInstId){
			/*get lan  port information*/
			ret = getLanPortByMeType(tempInstId, tempMeType, &opFlag, &portId);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]getLanPortByMeType fail\n",__LINE__, __func__);
				continue;
			} 

			/*CALL multicast API*/
			if(opFlag == OMCI_MULTICAST_ONE_PORT_FLAG){
				maxPortNum = 1;				
			}else if(opFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
				maxPortNum = OMCI_LAN_PORT_NUM;
			}else{
				//nothing
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]opFlag = %x\n",__LINE__, __func__, opFlag);
				continue;
			}
			for(i = 0; i<maxPortNum; i++){
				if(maxPortNum == OMCI_LAN_PORT_NUM){
					portId = i+1; //igmp valid value 1~4
				}else{
					portId += 1; //igmp valid value 1~4
				}
		
				if(type&OMCI_MULTICAST_VER_TYPE){
					tempValU8= *(uint8 *)val_ptr;
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
#if 0  /*olt don't really want to configure port version,only ask onu for whelther support this version */
					ret = igmpONUSetVer(portId, tempValU8);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetVer fail\n",__LINE__, __func__);
					}
#endif
				}
				if(type&OMCI_MULTICAST_FUNC_TYPE){
					tempValU8= *(uint8 *)val_ptr;

					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
					ret = igmpONUSetFunc(portId, tempValU8);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFunc fail\n",__LINE__, __func__);
					}
				}
				if(type&OMCI_MULTICAST_FAST_LEAVE_TYPE){
					tempValU8= *(uint8 *)val_ptr;
					
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
					ret = igmpONUSetFastLeave(portId, tempValU8);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetFastLeave fail\n",__LINE__, __func__);
					}
				}
				if(type&OMCI_MULTICAST_UP_TCI_TYPE){
					tempValU16= get16((uint8 *)val_ptr);
					
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU16 == %x\n",__LINE__, __func__, tempValU16);
					ret = igmpONUSetUpstreamTCI(portId, tempValU16);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetUpstreamTCI fail\n",__LINE__, __func__);
					}
				}
				if(type&OMCI_MULTICAST_TAG_CTRL_TYPE){
					tempValU8= *(uint8 *)val_ptr;
					
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU16 == %x\n",__LINE__, __func__, tempValU8);
					ret = igmpONUSetTagCtrl(portId, tempValU8);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetTagCtrl fail\n",__LINE__, __func__);
					}
				}
				if(type&OMCI_MULTICAST_MAX_RATE_TYPE){
					tempValU32 = get32((uint8 *)val_ptr);
					
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
					ret = igmpONUSetMaxRate(portId, tempValU32);
					if(ret != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetMaxRate fail\n",__LINE__, __func__);
					}
				}
				if((type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE) || (type&OMCI_MULTICAST_STATIC_ACL_TYPE)){		
					switch(aclRuleAction){
						case OMCI_IGMP_ACL_TABLE_ENTRY_WRITE_ACTION: //add acl rule
							if(type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE){
								ret = igmpONUSetDyCtrList(portId, ruleRecord);
								if(ret != 0){
										omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetDyCtrList  fail,tempInstId=0x%02x,portId=%d\n",__LINE__, __func__, tempInstId, portId);
								}
							}
							if(type&OMCI_MULTICAST_STATIC_ACL_TYPE){
								ret = igmpONUSetStaticCtrList(portId, ruleRecord);
								if(ret != 0){
										omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetStaticCtrList  fail,tempInstId=0x%02x,portId=%d\n",__LINE__, __func__, tempInstId, portId);
								}
							}
							break;

						case OMCI_IGMP_ACL_TABLE_ENTRY_DEL_ACTION://del acl rule
							if(type&OMCI_MULTICAST_DYNAMIC_ACL_TYPE){
								ret = igmpONUDelDyCtrList(portId, ruleRecord->rowKey);
								if(ret != 0){
										omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]->igmpONUDelDyCtrList  fail,tempInstId=0x%02x,portId=%d",__LINE__, __func__,tempInstId, portId);
								}
							}
							if(type&OMCI_MULTICAST_STATIC_ACL_TYPE){
								ret = igmpONUDelStaticCtrList(portId, ruleRecord->rowKey);
								if(ret != 0){
										omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]->igmpONUDelStaticCtrList  fail,tempInstId=0x%02x,portId=%d",__LINE__, __func__,tempInstId, portId);
								}
							}
							break;
							
						default:
							break;
					}
				}
#ifdef TCSUPPORT_XPON_IGMP					
				/*set igmp robustness type by port*/
				if(type&OMCI_MULTICAST_ROBUSTNESS_TYPE){
					tempValU8 = *(uint8 *)val_ptr;
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
				
					if(igmpONUSetRobustness(portId,tempValU8) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetRobustness fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set igmp query ip type by port*/
				if(type&OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE){
					tempValU8 = *(uint8 *)val_ptr;
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %02x%02x%02x%02x\n",__LINE__, __func__, *val_ptr,*(val_ptr+1),*(val_ptr+2),*(val_ptr+3));
				
					if(igmpONUSetQueryIp(portId,val_ptr) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetRobustness fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set igmp query interval type by port*/
				if(type&OMCI_MULTICAST_QUERY_INTERVAL_TYPE){
					tempValU32 = get32((uint8*)val_ptr);
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
				
					if(igmpONUSetQueryInterval(portId,tempValU32) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryInterval fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set max response time type by port*/		
				if(type&OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE){
					tempValU32 = get32((uint8*)val_ptr);
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
				
					if(igmpONUSetQueryMaxRespTime(portId,tempValU32) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetQueryMaxRespTime fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set last query interval type by port*/
				if(type&OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE){
					tempValU32 = get32((uint8*)val_ptr);
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU32 == %x\n",__LINE__, __func__, tempValU32);
				
					if(igmpONUSetLastQueryInterval(portId,tempValU32) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetLastQueryInterval fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set Unauthorized join request type by port*/ 
				if(type&OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE){
					tempValU8 = *(uint8 *)val_ptr;
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x\n",__LINE__, __func__, tempValU8);
				
					if(igmpONUSetUnauthorized(portId,tempValU8) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetLastQueryInterval fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
				
				/*set Downstream IGMP and TCI type by port*/
				if(type&OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE){
					tempValU8 = *val_ptr;
					tempValU16 = get16((uint8*)val_ptr+1);
					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]tempValU8 == %x, tempValU16 == %x\n",__LINE__, __func__, tempValU8, tempValU16);
				
					if(igmpONUSetDownTCIType(portId,tempValU8, tempValU16) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetDownTCIType fail\n",__LINE__, __func__);
						ret = -1;
						goto end;
					}
				}
#endif					
			}	
		}
	}
	
	ret = 0;	
end:
	if(ret != 0){
		ret = -1;
	}

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setRealMulticastRuleByProfile->ret =0x%0x",ret);
	return ret;
	
}

/*******************************************************************************************
**function name
	findMulticastInstByMacBrPortInst
**description:
	find 9.3.28 instance by 9.3.4 mac bridge port ME instance id
 **retrun :
 	-1:	not find
 	0:	find
**parameter:
  	macBrPortinstId :	9.3.4 mac bridge port config data ME instance id
********************************************************************************************/
int findMulticastInstByMacBrPortInst(uint16 macBrPortinstId){
	omciManageEntity_ptr me_p = NULL;
	omciInst_t *omciInst_p = NULL;
	omciMeInst_t *inst_p = NULL;
	int ret = -1;	
	uint8 *attributeValuePtr = NULL;
	
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMulticastInstByMacBrPortInst->start");

	/*get 9.3.28 multicast subscriber config info ME*/
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findMulticastInstByMacBrPortInst->omciGetMeByClassId fail");
		ret = -1;
		goto end;
	}

	/*find 9.3.28 instance by mac bridge port ME instance id*/
	omciInst_p = &(me_p->omciInst);
	if(omciInst_p->omciMeInstList == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n findMulticastInstByMacBrPortInst-> omciMeInstList == NULL");
		ret = -1;
		goto end;
	}
	inst_p = omciInst_p->omciMeInstList;
	while(inst_p != NULL){
		attributeValuePtr = inst_p->attributeVlaue_ptr;
		if((attributeValuePtr != NULL)
			&& (get16(attributeValuePtr) == macBrPortinstId)
			&& (*(attributeValuePtr+2) == OMCI_MULTICAST_CONFIG_INFO_ME_TYPE_0)){

//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMulticastInstByMacBrPortInst-> Find, macBrPortinstId=0x%02x",macBrPortinstId);	
			ret = 0; //find;
			break;
		}
		inst_p = inst_p->next;
	}

end:
	if(ret != 0){
		ret = -1;
	}
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findMulticastInstByMacBrPortInst-> ret = 0x%02x",ret);
	return ret;
}

/*******************************************************************************************
**function name
	getMulticastPortList
**description:
	according the multicast profile ME instance id , get lan port list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	currInstId :	multicast profile ME instance id
  	portList: stored the port
  	validPortNum: max valid port num
********************************************************************************************/
int getMulticastPortList(IN uint16 currInstId, OUT uint16 *portList, OUT uint8 *validPortNum){
	int ret = -1;
	omciManageEntity_ptr me_p = NULL;
	omciInst_t *omciInst_p = NULL;
	omciMeInst_t *inst_p = NULL;
	uint16 tempInstId = 0;
	uint16 tempMulticastProfileInstId = 0;
	uint8 tempMeType = 0;
	uint16 portAttr[OMCI_LAN_PORT_NUM] = {0};
	uint8 portNum = 0;
	uint8 portFlag = 0;
	uint16 portId = 0;	
	int i = 0;
	uint8 findFlag = 0;

	if(portList == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMulticastPortList->portList is NULL");
		goto end;
	}
	me_p = omciGetMeByClassId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO);
	if(me_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n clearRealMulticastRuleByProfileType->omciGetMeByClassId fail");
		ret = -1;
		goto end;
	}

	omciInst_p = &(me_p->omciInst);
	if(omciInst_p->omciMeInstList == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n clearRealMulticastRuleByProfileType->omciInst_p->omciMeInstList == NULL, ok, ret =0");
		ret = 0;
		goto end;
	}
	inst_p = omciInst_p->omciMeInstList;
	while(inst_p != NULL){
		tempInstId = get16(inst_p->attributeVlaue_ptr); //9.3.28 inst id
		tempMeType = *(inst_p->attributeVlaue_ptr+2); //9.3.28 ME type attribute
		tempMulticastProfileInstId = get16(inst_p->attributeVlaue_ptr+3); //9.3.28 multicast operations profile pointer attribute
		if(tempMulticastProfileInstId == currInstId){
			/*get lan  port information*/
			if(getLanPortByMeType(tempInstId, tempMeType, &portFlag, &portId) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMulticastPortList-> getLanPortByMeType  fail, 9.3.28 instanceid = 0x%02x",tempInstId);
			}else{
				if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG){
					if(portNum == OMCI_LAN_PORT_NUM){
						break;
					}
					findFlag = 0;
					for(i=0; i<portNum; i++){
						if(portAttr[i] == portId){
							findFlag = 1;
							break;
						}
					}
					if(findFlag == 0){
						portAttr[portNum] = portId;
						portNum++;
					}
				}else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
					for(i= 0; i<OMCI_LAN_PORT_NUM; i++){
						portAttr[i] = i;
					}
					portNum = OMCI_LAN_PORT_NUM;
					break;
				}else{
					//nothing
				}
			}
		}
		inst_p = inst_p->next;
	}
	

	ret = 0;
	
end:
	for(i=0; i<portNum; i++){
		*(portList+i) = portAttr[i];
	}
	*validPortNum = portNum;
	
	if(ret != 0){
		ret = -1;
	}

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMulticastPortList->ret =0x%02x",ret);
	return ret;
	
}

/*******************************************************************************************************************************
start
9.3.27 multicast operations profile

********************************************************************************************************************************/

int omciMeInitFor_MulticastProfile(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMulticastProfileDeleteAction;

	/* if need ONU auto create a ME instance, create here */
	return 0;
}

int omciMulticastProfileDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
	int ret = -1;
	uint16 currInstId = 0;
	uint16 type = 0;

	currInstId = get16((uint8*)&(omciPayLoad->meId)+2);	

	type = ((OMCI_MULTICAST_VER_TYPE)
		| (OMCI_MULTICAST_FUNC_TYPE)
		| (OMCI_MULTICAST_FAST_LEAVE_TYPE)
		| (OMCI_MULTICAST_UP_TCI_TYPE)
		| (OMCI_MULTICAST_TAG_CTRL_TYPE)		
		| (OMCI_MULTICAST_MAX_RATE_TYPE)
		| (OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		| (OMCI_MULTICAST_STATIC_ACL_TYPE)
		| (OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		| (OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		| (OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		| (OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		| (OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE));

	ret = clearRealMulticastRuleByProfileType(currInstId, type);

	if(ret == 0){	
		return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);	
	}else{
		ret = -1;
		return OMCI_CMD_ERROR;
	}
}

int setMOPMulticastAttrValue(uint16 instId, uint16 type, char *val_ptr){
	int ret = -1;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, type=%02x\n",__LINE__, __func__, instId, type);
	if(val_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]val_ptr == NULL\n",__LINE__, __func__);
		goto end;
	}
	/*clear all configuration in lan port according to the this profile*/
	if(clearRealMulticastRuleByProfileType(instId, type) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]clearRealMulticastRuleByProfileType fail\n",__LINE__, __func__);
		goto end;
	}
	
	/*set multicast rule */
	if(setRealMulticastRuleByProfile(instId, type, 0, val_ptr, NULL) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByProfile fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	return ret;
}


int setMOPMulticastVersionValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 igmpVersion = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_VER_TYPE;

	igmpVersion = *(uint8 *)value;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, igmpVersion=%02x\n",__LINE__, __func__, instanceId, igmpVersion);
	
	switch(igmpVersion){
		case OMCI_MULTICAST_VERSION_V2:
		case OMCI_MULTICAST_VERSION_V3:
		case OMCI_MULTICAST_VERSION_MLD_V1:
		case OMCI_MULTICAST_VERSION_MLD_V2:			
			break;
		default:
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpVersion reserved\n",__LINE__, __func__);
			goto end;
	}
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}

	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}

}

int setMOPMulticastFuncValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 igmpFuncType = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_FUNC_TYPE;

	igmpFuncType = *(uint8 *)value;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, igmpFuncType=%02x\n",__LINE__, __func__, instanceId, igmpFuncType);
	
	switch(igmpFuncType){
		case OMCI_MULTICAST_FUNCTION_SNOOPING:
		case OMCI_MULTICAST_FUNCTION_SPR:
		case OMCI_MULTICAST_FUNCTION_PROXY:		
			break;
		default:
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpFuncType not support\n",__LINE__, __func__);
			goto end;
	}

	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}

	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}

int setMOPMulticastFastLeaveValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 igmpFastLeave = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_FAST_LEAVE_TYPE;

	igmpFastLeave = *(uint8 *)value;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, igmpFastLeave=%02x\n",__LINE__, __func__, instanceId, igmpFastLeave);
	
	switch(igmpFastLeave){
		case OMCI_MULTICAST_FAST_LEAVE_DISABLE:
		case OMCI_MULTICAST_FAST_LEAVE_ENABLE:	
			break;
		default:
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpFastLeave not support\n",__LINE__, __func__);
			goto end;
	}

	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}

	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}

}

int setMOPMulticastUpstreamTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint16 upSteamTCI = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_UP_TCI_TYPE;

	upSteamTCI = get16((uint8 *)value);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, upSteamTCI=%02x\n",__LINE__, __func__, instanceId, upSteamTCI);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}

	ret = 0;
	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}


int setMOPMulticastTagCtrlValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 tagCtrlVal = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_TAG_CTRL_TYPE;

	tagCtrlVal = *(uint8 *)value;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, tagCtrlVal=%02x\n",__LINE__, __func__, instanceId, tagCtrlVal);
	
	switch(tagCtrlVal){
		case OMCI_MULTICAST_TAG_CTRL_TRANSPARENT:
		case OMCI_MULTICAST_TAG_CTRL_ADD_TCI:
		case OMCI_MULTICAST_TAG_CTRL_REPLACE_TCI:
		case OMCI_MULTICAST_TAG_CTRL_REPLACE_VID:
			break;
		default: //only update instance value
			ret = 0;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpFuncType not support\n",__LINE__, __func__);
			goto end;
	}

	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}	

	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}

int setMOPMulticastUpstreamRateValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint32 maxRate = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	maxRate = get32((uint8 *)value);
	type = OMCI_MULTICAST_MAX_RATE_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x, maxRate=%x\n",__LINE__, __func__, instanceId, maxRate);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}

/*******************************************************************************************
**function name
	getMOPMulticastAclTableValueByType
**description:
	get ACL Value for OLT request according to the attribute index.
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
int getMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, IN uint16 attrIndex)
{
	omciTableAttriValue_t *tmp_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;	
	char *tableBuffer = NULL;
	uint32 tableSize = 0;
	uint16 instanceId = 0;
	int ret = -1;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, attrIndex =%02x\n",__LINE__, __func__, attrIndex);

	/*only support dynamic acl attribute or static acl attribute*/
	if((attrIndex == OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX)
		|| (attrIndex == OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX) ){
		//nothing
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]attrIndex fail\n",__LINE__, __func__);
		goto end;
	}
	
	tmp_ptr = omciGetTableValueByIndex(tmpomciMeInst_ptr, attrIndex);
	if(tmp_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciGetTableValueByIndex fail\n",__LINE__, __func__);
		ret = -1;
		goto end;	
	}
	
	/*1.calculate the table size*/
	tableSize = tmp_ptr->tableSize;
	
	/*2.get table content*/
	tableBuffer = (char *)tmp_ptr->tableValue_p;
	if(tableBuffer == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tableBuffer NULL\n",__LINE__, __func__);
		ret = -1;
		goto end;		
	}

	/*3. add the table to omci archtecture*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	ret = omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, (uint8 *)tableBuffer, tableSize ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]omciAddTableContent fail\n",__LINE__, __func__);
		ret = -1;
		goto end;
	}

	ret = 0;
end:
	if(ret != 0){
		ret = -1;
		tableSize = 0;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, tableSize=%02x\n",__LINE__, __func__, tableSize);
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);

}

/*******************************************************************************************
**function name
	setMOPMulticastAclTableValueByType
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
int setMOPMulticastAclTableValueByType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, IN uint16 attrIndex){
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
	uint8 G988Flag = 0;
	
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
	setCtrl = ((ctrlField&OMCI_MOP_ACL_TABLE_ENTRY_SET_CTRL_MASK)>> OMCI_MOP_ACL_TABLE_ENTRY_SET_CTRL_FILED);
	rowPart = ((ctrlField&OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_MASK)>> OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
	rowID = (ctrlField & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_AMD1_MASK);	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]setCtrl=0x%2x, rowPart=0x%2x, rowID=%d\n",__LINE__, __func__, setCtrl,rowPart,rowID);

	/*only support row part0*/
	if(rowPart != OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART0){
		/* we need hanle this table entry to G.988 format(not raw port&&test) */
		G988Flag = 1;
		rowID = (ctrlField & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_MASK);
	}

	/*set rule from packet value */
	memset(ruleRecord, 0, sizeof(omciIgmpRuleInfo_t));
	ruleRecord->rowKey = rowID;
	ruleRecord->ruleType = ruleType;
	ruleRecord->gemPortId = get16(tmp_ptr+2);
	ruleRecord->vid = get16(tmp_ptr+4);
	memcpy(ruleRecord->srcIp,tmp_ptr+6, OMCI_MULTICAST_IPV4_LEN);
	memcpy(ruleRecord->startDestIp,tmp_ptr+10, OMCI_MULTICAST_IPV4_LEN);
	memcpy(ruleRecord->endDestIp,tmp_ptr+14, OMCI_MULTICAST_IPV4_LEN);
	ruleRecord->imputedBandwidth = get32(tmp_ptr+18);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]ruleRecord rowKey:%x,ruleType:%x,gemPortId:%x,vid:%x,srcIp:%x,startDestIp:%x,endDestIp:%x\n",
			__LINE__, __func__, ruleRecord->rowKey,ruleRecord->ruleType,ruleRecord->gemPortId,ruleRecord->vid, 
		get32(ruleRecord->srcIp), get32(ruleRecord->startDestIp), get32(ruleRecord->endDestIp));
	
	/*don't support extend format, so test bit in table contrl will be set to 1.*/
//	ctrlField |= OMCI_MOP_ACL_TABLE_ENTRY_CLEAR_TEST_BIT;
//	put16(tmp_ptr, ctrlField);

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
		length = OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
		for(tmp_ptr = tableConent_ptr->tableValue_p; tmp_ptr != NULL; tmp_ptr += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH){
			ctrlField_tmp = get16(tmp_ptr);
			rowPart_tmp = ((ctrlField_tmp&OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_MASK)>> OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
			if (G988Flag == 1)	{
				rowID_tmp = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_MASK);
				if (rowID == rowID_tmp)
				{
					findFlag = 1; //find
					break;
				}
			} else {
				rowID_tmp = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_AMD1_MASK);
				if((rowID == rowID_tmp) &&(rowPart == rowPart_tmp)){
					findFlag = 1;//find
					break;
				}
			}
			
			length += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
			if(length > tableConent_ptr->tableSize){ 
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]length:%d > tableConent_ptr->tableSize:%d\n",__LINE__, __func__, length, tableConent_ptr->tableSize);
				findFlag = 0;//not find
				break;
			}
		}
		
		/*get real length*/
		length -= OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
		/*get valid content in table*/
		if(findFlag == 1){
			/*delete ACL rule via calling multicast API*/
			ret = setRealMulticastRuleByProfile(instanceId, type, action, NULL, ruleRecord);
			if(ret != 0){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByProfile fail\n",__LINE__, __func__);
				goto end;
			}			
			
			/*delete a entry from table list*/			
			/*malloc new space*/
			if(tableSize-OMCI_MOP_ACL_TABLE_ENTRY_LENGTH > 0)
			{
				tableSize -= OMCI_MOP_ACL_TABLE_ENTRY_LENGTH; //update current table size
				tmpValue_ptr = calloc(1 , tableSize);
				if(tmpValue_ptr == NULL){
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
					ret = -1;
					goto end;
				}	
				/*if find, then delete this record in table*/			
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, length);
				memcpy(tmpValue_ptr+length, tableConent_ptr->tableValue_p+length+OMCI_MOP_ACL_TABLE_ENTRY_LENGTH, tableSize-length);
			}
			else
			{
				tableSize = 0;
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
		for(length = OMCI_MOP_ACL_TABLE_ENTRY_LENGTH; tmp_ptr != NULL; tmp_ptr += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH){		
			ctrlField_tmp = get16(tmp_ptr);
			rowPart_tmp = ((ctrlField_tmp&OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_MASK)>> OMCI_MOP_ACL_TABLE_ENTRY_ROW_PART_FILED);
			if (G988Flag == 1)	{
				rowID_tmp = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_MASK);
				if (rowID == rowID_tmp)
				{
					findFlag = 1; //find
					break;
				}
			} else {
				rowID_tmp = (ctrlField_tmp & OMCI_MOP_ACL_TABLE_ENTRY_ROW_ID_988_AMD1_MASK);
				if((rowID == rowID_tmp) &&(rowPart == rowPart_tmp)){
					findFlag = 1;//find
					break;
				}
			}

			length += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
			if(length > tableConent_ptr->tableSize){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]length:%d > tableConent_ptr->tableSize:%d\n",__LINE__, __func__, length, tableConent_ptr->tableSize);
				findFlag = 0;//not find
				break;
			}
		}
		/*get real length*/
		length -= OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;

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
			memcpy(tmpValue_ptr+length, value, OMCI_MOP_ACL_TABLE_ENTRY_LENGTH);	
		}else{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]not find, add this record\n", __LINE__, __func__);	
			/*if not find, add this record*/
			/*malloc new space*/
			tmpValue_ptr = calloc(1 , tableSize+OMCI_MOP_ACL_TABLE_ENTRY_LENGTH);
			if(tmpValue_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]tmpValue_ptr fail\n",__LINE__, __func__);
				ret = -1;
				goto end;		
			}		

			/*fill new space*/
			if(tableSize > 0)
				memcpy(tmpValue_ptr,tableConent_ptr->tableValue_p, tableSize);
			memcpy(tmpValue_ptr+tableSize, value, OMCI_MOP_ACL_TABLE_ENTRY_LENGTH);
			tableSize += OMCI_MOP_ACL_TABLE_ENTRY_LENGTH;
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

int32 getMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX;

	return getMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);

}

int setMOPMulticastDynamicAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_DYNAMIC_ACL_ATTR_INDEX;
	
	return setMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);
}

int32 getMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX;

	return getMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);

}

int setMOPMulticastStaticAclTableValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	uint16 attrIndex = OMCI_MULTICAST_STATIC_ACL_ATTR_INDEX;
	
	return setMOPMulticastAclTableValueByType(value, meInstantPtr, omciAttribute, attrIndex);
}

int setMOPMulticastRobustnessValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_ROBUSTNESS_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}

	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMOPMulticastQuerierIPAddressValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	char queryip[16] = {0};

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	memcpy(queryip, value, 4);
	if(setMOPMulticastAttrValue(instanceId, type, queryip) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMOPMulticastQueryIntervalValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_QUERY_INTERVAL_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMOPMulticastQueryMaxResponseTimeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMOPMulticastLastMemberQueryIntervalValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMOPMulticastUnauthorizedJoinRequestValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, instId =%02x\n",__LINE__, __func__, instanceId);
	
	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setMOPMulticastAttrValue fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;	
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret=%02x\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}

int setMOPMulticastDownstreamIGMPAndTCIValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag)
{
	int ret = -1;
	uint8 tagCtrlVal = 0;
	uint16 downStreamTCI = 0;
	uint16 instanceId=0;
	uint16 type = 0;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL)){
		goto end;
	}
	
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	type = OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE;

	tagCtrlVal = *(uint8 *)value;
	downStreamTCI = get16((uint8 *)value+1);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]:tagCtrlVal = %x, downStreamTCI=%x\n", __LINE__, __func__, tagCtrlVal, downStreamTCI);
	
	switch(tagCtrlVal){
		case OMCI_MULTICAST_DOWN_TAG_CTRL_TRANSPARENT:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_STRIP:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_ADD_TCI:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_TCI:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_VID:			
			break;
		case OMCI_MULTICAST_DOWN_TAG_CTRL_ADD_TCI_USE_UNI:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_TCI_USE_UNI:
		case OMCI_MULTICAST_DOWN_TAG_CTRL_REPLACE_VID_USE_UNI:
			break;
		default: //only update instance value
			if(flag == OMCI_CREATE_ACTION_SET_VAL){
				ret = 0;
			}
			goto end;
	}

	if(setMOPMulticastAttrValue(instanceId, type, value) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]:setMOPMulticastAttrValue, fail", __LINE__, __func__);
		goto end;
	}	

	ret = 0;
end:
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}

/*******************************************************************************************************************************
start
9.3.28 multicast subscriber config info

********************************************************************************************************************************/
int omciMeInitForMulticasSubscribertCfgInfo(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciMulticasSubscribertCfgInfoDeleteAction;

	/* if need ONU auto create a ME instance, create here */
	return 0;
}


int omciMulticasSubscribertCfgInfoDeleteAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
	int ret = -1;
	uint16 instId = 0;
	uint8 meType = 0;
	uint16 clearType = 0;	

	instId = get16((uint8*)&(omciPayLoad->meId)+2);

	/*get ME type attribute value*/
	ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_CFG_INFO, instId, 1, &meType, 1);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]getAttributeValueByInstId fail, ret =%d\n",__LINE__, __func__, ret);
		goto end;
	}

	/*set clear type*/
	clearType = ((OMCI_MULTICAST_VER_TYPE)
		| (OMCI_MULTICAST_FUNC_TYPE)
		| (OMCI_MULTICAST_FAST_LEAVE_TYPE)
		| (OMCI_MULTICAST_UP_TCI_TYPE)
		| (OMCI_MULTICAST_TAG_CTRL_TYPE)		
		| (OMCI_MULTICAST_MAX_RATE_TYPE)
		| (OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		| (OMCI_MULTICAST_STATIC_ACL_TYPE)
		| (OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		| (OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		| (OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		| (OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		| (OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE));

	/*clear real multicast rule by 9.3.28 me type attribute.*/
	ret = clearRealMulticastRuleByMeType(instId, meType, clearType);			
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]clearRealMulticastRuleByMeType fail, ret =%d\n",__LINE__, __func__, ret);
		goto end;
	}
	
	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret =%d\n",__LINE__, __func__, ret);
	if(ret == 0){	
		return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);	
	}else{
		ret = -1;
		return OMCI_CMD_ERROR;
	}
}

int setMSCmeTypeValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 currInstId = 0;
	uint8 meType = 0;
	uint16 clearType = 0;
	uint16 multicastProfileInstId = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start\n",__LINE__, __func__);

	if(value == NULL || (tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL)){
		ret = -1;
		goto end;
	}

	/*get 9.3.28 ME  instance id*/
	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	/*get old  me type value*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr + 2;
	meType = *attributeValuePtr;

	multicastProfileInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr+3);
	if ((multicastProfileInstId == 0x0) && (multicastProfileInstId == 0xffff))
	{
		ret = 0;	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]multicastProfileInstId is NULL\n",__LINE__, __func__);
		goto end;
	}

	/*set operation  type*/
	clearType = (( OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		| (OMCI_MULTICAST_STATIC_ACL_TYPE)
		| (OMCI_MULTICAST_MAX_RATE_TYPE)
		| (OMCI_MULTICAST_VER_TYPE)
		| (OMCI_MULTICAST_FUNC_TYPE)
		| (OMCI_MULTICAST_FAST_LEAVE_TYPE)
		| (OMCI_MULTICAST_UP_TCI_TYPE)
		| (OMCI_MULTICAST_TAG_CTRL_TYPE)		
		| (OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		| (OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		| (OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		| (OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		| (OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE));

	/*clear old  real multicast rule by 9.3.28 me type attribute.*/
	ret = clearRealMulticastRuleByMeType(currInstId, meType, clearType);			
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]clearRealMulticastRuleByMeType fail\n",__LINE__, __func__);
		goto end;
	}

	/*get new me type value*/
	meType = *(uint8 *)value;

	/*set new rule by 9.3.28 me type*/
	ret = setRealMulticastRuleByMeType(currInstId, meType, clearType);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByMeType fail\n",__LINE__, __func__);
		goto end;
	}
	
	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end, ret = %d\n",__LINE__, __func__, ret);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}	
}

int setMSCOperationProfileValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 currInstId = 0;
	uint8 meType = 0;
	uint16 clearType = 0;
	uint16 multicastProfileInstId = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start\n",__LINE__, __func__);

	if(value == NULL || (tmpomciMeInst_p == NULL || tmpomciMeInst_p->attributeVlaue_ptr == NULL)){
		ret = -1;
		goto end;
	}

	/*get 9.3.28 ME  instance id*/
	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	/*get me type value*/
	meType = *(tmpomciMeInst_p->attributeVlaue_ptr+2);
	
	/*set operation  type*/
	clearType = (( OMCI_MULTICAST_DYNAMIC_ACL_TYPE)
		| (OMCI_MULTICAST_STATIC_ACL_TYPE)
		| (OMCI_MULTICAST_MAX_RATE_TYPE)
		| (OMCI_MULTICAST_VER_TYPE)
		| (OMCI_MULTICAST_FUNC_TYPE)
		| (OMCI_MULTICAST_FAST_LEAVE_TYPE)
		| (OMCI_MULTICAST_UP_TCI_TYPE)
		| (OMCI_MULTICAST_TAG_CTRL_TYPE)		
		| (OMCI_MULTICAST_ROBUSTNESS_TYPE)	
		| (OMCI_MULTICAST_QUERIER_IP_ADDRESS_TYPE)	
		| (OMCI_MULTICAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_QUERY_MAX_RESPONSE_TIME_TYPE)	
		| (OMCI_MULTICAST_LAST_QUERY_INTERVAL_TYPE)	
		| (OMCI_MULTICAST_UNAUTHORIZED_JOIN_TYPE)	
		| (OMCI_MULTICAST_DOWN_IGMP_AND_TCI_TYPE));
	
	/*clear old  real multicast rule by 9.3.28 me type attribute.*/
	ret = clearRealMulticastRuleByMeType(currInstId, meType, clearType);			
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]clearRealMulticastRuleByMeType fail\n",__LINE__, __func__);
		goto end;
	}
	
	/*update multicast operations profile pointer attribute value*/
	ret = setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	if(ret != 0){
		ret = -1;
		goto end;
	}

	multicastProfileInstId = get16((uint8*)value);
	if ((multicastProfileInstId == 0x0) && (multicastProfileInstId == 0xffff))
	{
		ret = 0;	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]multicastProfileInstId is NULL\n",__LINE__, __func__);
		goto end;
	}
	

	/*set new rule by 9.3.28 me type*/
	ret = setRealMulticastRuleByMeType(currInstId, meType, clearType);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]setRealMulticastRuleByMeType is NULL\n",__LINE__, __func__);
	}
	
	ret = 0;
end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end\n",__LINE__, __func__);
	if(ret != 0){
		ret = -1;
	}	
	return ret;	
}
int setMSCMaxSimultaneousGroupValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint16 val = 0;
	omciMeInst_t *tmpomciMeInst_p; 
	uint16 currInstId = 0;
	uint8 meType = 0;
	uint8 portFlag = 0;
	uint16 portId = 0;	
	int i;
	
	if((value == NULL)
		||(meInstantPtr == NULL)
		|| (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	meType = *(tmpomciMeInst_p->attributeVlaue_ptr+2);
	
	val = get16((uint8 *)value);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]start, currInstId=%d, meType=%d, val=%d\n", __LINE__, __func__,currInstId, meType, val);

	ret = getLanPortByMeType(currInstId, meType, &portFlag, &portId);
	if(ret != 0){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]getLanPortByMeType fail\n",__LINE__, __func__);
		goto end;
	}
	
#ifdef	TCSUPPORT_XPON_IGMP
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG){
		ret = igmpONUSetMaxPlayNum(portId+1,val);
		if(ret != 0){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetMaxPlayNum fail\n",__LINE__, __func__);
		}		
	}else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
		for(i = 0; i<OMCI_LAN_PORT_NUM; i++){
			ret = igmpONUSetMaxPlayNum(i+1,val);
			if(ret != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]igmpONUSetMaxPlayNum fail\n",__LINE__, __func__);
			}
		}	
	}
#endif

	ret = 0;

end:
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]end\n",__LINE__, __func__);
	if(ret == 0){
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}else{
		ret = -1;
		return ret;
	}
}
int setMSCMaxBandwidthValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1, i = 0;
	uint32 val = 0;
	uint16 currInstId = 0, portId = 0;
	uint8 meType = 0, portFlag = 0;
	omciMeInst_t *tmpomciMeInst_p; 
    
	if((value == NULL)
		||(meInstantPtr == NULL)
		|| (omciAttribute == NULL))
	{
		ret = -1;
		goto end;
	}

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;	

	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);	
	meType = *(tmpomciMeInst_p->attributeVlaue_ptr+2);	
	val = get32((uint8 *)value);

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%s][LN%d]start, currInstId=%d, meType=%d, val=%d\n",__FUNCTION__,__LINE__,currInstId, meType, val);

	ret = getLanPortByMeType(currInstId, meType, &portFlag, &portId);
	if(ret != 0)
	{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]getLanPortByMeType fail\n",__FUNCTION__,__LINE__);
		goto end;
	}
	
#ifdef	TCSUPPORT_XPON_IGMP
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG)
	{
		ret = igmpONUSetMaxBW(portId+1,val);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]igmpONUSetMaxBW fail\n",__FUNCTION__,__LINE__);
		}		
	}
	else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG)
	{
		for(i = 0; i<OMCI_LAN_PORT_NUM; i++)
		{
			ret = igmpONUSetMaxBW(i+1,val);
			if(ret != 0)
			{
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]igmpONUSetMaxBW fail\n",__FUNCTION__,__LINE__);
			}
		}	
	}
#endif

	ret = 0;
end:

	if(ret == 0)
	{       
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}
	else
	{
		ret = -1;
		return ret;
	}
}

int setMSCBWEnforcementValue(char *value, uint8 *meInstantPtr , omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1, i = 0;
	uint8 val = 0;
	uint16 currInstId = 0, portId = 0;
	uint8 meType = 0, portFlag = 0;
	omciMeInst_t *tmpomciMeInst_p; 
    
	if((value == NULL)
		||(meInstantPtr == NULL)
		|| (omciAttribute == NULL))
	{
		ret = -1;
		goto end;
	}

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;	

	currInstId = get16(tmpomciMeInst_p->attributeVlaue_ptr);	
	meType = *(tmpomciMeInst_p->attributeVlaue_ptr+2);	
	val = *(uint8 *)value;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%s][LN%d]start, currInstId=%d, meType=%d, val=%d\n",__FUNCTION__,__LINE__,currInstId, meType, val);

	ret = getLanPortByMeType(currInstId, meType, &portFlag, &portId);
	if(ret != 0)
	{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]getLanPortByMeType fail\n",__FUNCTION__,__LINE__);
		goto end;
	}
	
#ifdef TCSUPPORT_XPON_IGMP    
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG)
	{
		ret = igmpONUSetMaxEnforcement(portId+1,val);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]igmpONUSetMaxBW fail\n",__FUNCTION__,__LINE__);
		}		
	}
	else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG)
	{
		for(i = 0; i<OMCI_LAN_PORT_NUM; i++)
		{
			ret = igmpONUSetMaxEnforcement(i+1,val);
			if(ret != 0)
			{
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%s][LN%d]igmpONUSetMaxBW fail\n",__FUNCTION__,__LINE__);
			}
		}	
	}
#endif

	ret = 0;
end:

	if(ret == 0)
	{       
		return setGeneralValue(value, meInstantPtr , omciAttribute, flag);
	}
	else
	{
		ret = -1;
		return ret;
	}
}

/*******************************************************************************************************************************
start
9.3.29 multicast subscriber monitor

********************************************************************************************************************************/

int omciMeInitForMulticastSubscriberMonitor(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;

	if(tmpOmciManageEntity_p == NULL)
		return -1;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_NEXT] = omciGetNextAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;

	return 0;
}

int32 getMSMIpv4ActiveGroupTblValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag){
	int ret = -1;
	uint32 tableSize = 0;
	uint16 maxBytes = 0;
	uint8 groupListNum = 0;
//	uint8 ipv4ListNum = 0;
	uint16 maxIpv4ListBytes = 0;
	uint8 maxIpv4ListNum = 0;	
	uint8 * tableBuffer_ptr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	uint16 instanceId = 0;
	uint8 meType = 0;
	uint8 portFlag =0;
	uint16 portId = 0;
	int i = 0;
	int j = 0;
//	uint16 len = 0;
	uint8 type = OMCI_MULTICAST_DYNAMIC_ACL_TYPE;
//	uint8 * temp_ptr = NULL;
	uint8 * temp1_ptr = NULL;
//	uint8 ipv4Prefix[OMCI_MAX_IPV4_PREFIX] =  {0};
	uint8 time = 0;

	omciIgmpActiveGroupInfo_ptr rule_ptr = NULL;
	omciIgmpActiveGroupInfo_ptr tempRule_ptr = NULL;

	if(value == NULL || meInstantPtr == NULL || omciAttribute == NULL ){
		ret = -1; 
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue -->parameter is NULL, fail");
		goto end;
	}

	if(tmpomciMeInst_ptr == NULL || tmpomciMeInst_ptr->attributeVlaue_ptr == NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue -->tmpomciMeInst_ptr or attributeVlaue_ptr  is NULL, fail");
		goto end;
	}
		
	/*malloc space for get a port active group list value(60N)*/
	maxBytes = OMCI_MAX_ACTIVE_MLTICAST_GROUP_NUM * OMCI_ACTIVE_MLTICAST_GROUP_RECORD_SIZE;
	rule_ptr = (omciIgmpActiveGroupInfo_ptr)calloc(OMCI_MAX_ACTIVE_MLTICAST_GROUP_NUM, sizeof(omciIgmpActiveGroupInfo_t));
	if(rule_ptr == NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue -->rule_ptr is NULL, calloc fail");
		goto end;
	}	
	
	/*malloc space for all ipv4 active dynamic group list*/
	maxIpv4ListNum = OMCI_MAX_ACTIVE_MLTICAST_GROUP_NUM * OMCI_LAN_PORT_NUM;
	maxIpv4ListBytes = maxIpv4ListNum*OMCI_IPV4_ACTIVE_MLTICAST_GROUP_RECORD_SIZE;
	tableBuffer_ptr = (uint8 *)calloc(1,maxIpv4ListBytes);
	if(tableBuffer_ptr == NULL){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue -->tableBuffer_ptr is NULL, calloc fail");
		goto end;
	}	

	/*get instance id*/
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	
	/*get 9.3.29  multicast subscriber monitorME me type attribute*/
	ret = getAttributeValueByInstId(OMCI_ME_CLASS_ID_MULTICAST_SUBSCRIBER_MONITOR, instanceId, 1, &meType, 1);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue-> getAttributeValueByInstId  me type fail ");
		ret = -1;
		goto end;
	}

	/*get lan  port information*/
	ret = getLanPortByMeType(instanceId, meType, &portFlag, &portId);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue-> getLanPortByMeType  fail");
		goto end;
	}
	
	if(portFlag == OMCI_MULTICAST_ONE_PORT_FLAG){		
		time = 1;
	}else if(portFlag == OMCI_MULTICAST_ALL_PORT_FLAG){
		time = OMCI_LAN_PORT_NUM;			
	}else{
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue-> portFlag  fail");
		goto end;
	}

//	ipv4ListNum = 0;
	tableSize = 0;
	/*get ipv4 dynamic active group list*/
	for(i = 0; i<time; i++){
		if(time == OMCI_LAN_PORT_NUM){
			portId = i+1; //igmp valid value 1~4
		}else{
			portId += 1; //igmp valid value 1~4
		}
		/*call Multicast dynamic access list table API*/
		groupListNum = 0;
//		len = 0;
		memset(rule_ptr, 0, maxBytes);
		ret = igmpONUGetActiveGroupList(portId, OMCI_MAX_ACTIVE_MLTICAST_GROUP_NUM,type, &groupListNum, rule_ptr);
		if(ret != 0){
			ret = -1;
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue-> igmpONUGetActiveGroupList  fail");
			continue;
		}
		
		/*get ipv4 dynamic active access list*/
		for(j = 0; j<groupListNum; j ++){
			tempRule_ptr = rule_ptr+j;
			temp1_ptr = tableBuffer_ptr + tableSize;
			/*vid*/
			put16(temp1_ptr, tempRule_ptr->vid);
			temp1_ptr += 2;
			
			/*source ip*/
			memcpy(temp1_ptr, tempRule_ptr->srcIp+OMCI_MAX_IPV4_PREFIX, 4);
			temp1_ptr +=4;
			
			/*multicast dest ip*/
			memcpy(temp1_ptr, tempRule_ptr->multiDestIp+OMCI_MAX_IPV4_PREFIX, 4);
			temp1_ptr +=4;

			/*best bandwidth*/
			put32(temp1_ptr, tempRule_ptr->bestBandWidth);
			temp1_ptr +=4;
			
			/*client ip*/
			memcpy(temp1_ptr, tempRule_ptr->clientIp+OMCI_MAX_IPV4_PREFIX, 4);
			temp1_ptr +=4;
			
			/*time(4bytes)*/
			put32(temp1_ptr, tempRule_ptr->time);
			temp1_ptr +=4;

			/*reserved(2bytes)*/
			put16(temp1_ptr, 0);
			temp1_ptr += 2;

			tableSize += OMCI_IPV4_ACTIVE_MLTICAST_GROUP_RECORD_SIZE;
			if(tableSize > maxIpv4ListBytes){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMSMIpv4ActiveGroupTblValue-> tableSize > maxIpv4ListBytes , tableSize=%d, maxIpv4ListBytes=%d",tableSize, maxIpv4ListBytes);
				tableSize -= OMCI_IPV4_ACTIVE_MLTICAST_GROUP_RECORD_SIZE;
				break;
			}			
		}		
	}

	/*handle table*/
	/*1.handle table size*/
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMSMIpv4ActiveGroupTblValue: tableSize = %d",tableSize);
	if(tableSize <= 0){
		tableSize = 0;
		ret = 0;
		goto end;
	}
	/*2. add the table to omci archtecture*/
	ret = omciAddTableContent(tmpomciMeInst_ptr->classId, instanceId, tableBuffer_ptr, tableSize ,omciAttribute->attriName);
	if (ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMSMIpv4ActiveGroupTblValue->omciAddTableContent fail");
		ret = -1;
		goto end;
	}
	
	ret = 0;
end:	
	if(rule_ptr != NULL){
		free(rule_ptr);
	}
	if(tableBuffer_ptr != NULL){
		free(tableBuffer_ptr);
	}

	if(ret != 0){
		tableSize = 0;
	}
	return getTheValue(value, (char*)&tableSize, 4, omciAttribute);
}

void hook_igmpONUSetGponMode(void){
	
#ifdef	TCSUPPORT_XPON_IGMP
	igmpONUSetGponMode();
#endif

}
