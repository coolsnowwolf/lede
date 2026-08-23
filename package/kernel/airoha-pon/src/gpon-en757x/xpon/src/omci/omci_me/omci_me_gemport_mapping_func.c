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
	omci_me_gemport_mapping_func.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	wayne.lee	2012/7/28	Create
*/

#include "omci_types.h"
#include "omci_general_func.h"
#include "omci_me_gemport_mapping.h"
#ifndef TCSUPPORT_GPON_MAPPING
/*******************************************************************************************************************************
gem port mapping API Start

********************************************************************************************************************************/
int addGemPortMappingRule(gemPortMappingIoctl_t *tableEntry){
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addGemPortMappingRule API");
	return 0;
}
int delGemPortMappingRule(gemPortMappingIoctl_t *tableEntry){
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delGemPortMappingRule API");
	return 0;

}
int displayAllGemPortMappingRule(void){
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n displayAllGemPortMappingRule API");
	return 0;

}
/*******************************************************************************************************************************
gem port mapping API End

********************************************************************************************************************************/
#endif

/*******************************************************************************************************************************
globle variable

********************************************************************************************************************************/
omciGemPortMapping_ptr gemPortMapping_ptr = NULL;

/*******************************************************************************************************************************
general function

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	convertGemPortRule
**description:
	convert omci gem port format to gem port API format
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	srcRule_ptr: source rule
  	destRule_ptr:	dest rule
********************************************************************************************/
int convertGemPortRule(IN omciGemPortMapping_ptr srcRule_ptr, OUT gemPortMappingIoctl_ptr destRule_ptr ){
	int ret = -1;
	uint16 tempU16 = 0;

	if(srcRule_ptr == NULL || destRule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n convertGemPortRule parameter is error, fail");
		goto end;
	}
	tempU16 = (~OMCI_GEMPORT_MAPPING_CLASSID_INSTID);
	destRule_ptr->tagCtl = (srcRule_ptr->tagctl & tempU16);
	destRule_ptr->tagFlag = srcRule_ptr->tagFlag;
	destRule_ptr->userPort = srcRule_ptr->userPort;
	destRule_ptr->aniPort = srcRule_ptr->aniPort;
	destRule_ptr->vid = srcRule_ptr->vid;
	destRule_ptr->dscp = srcRule_ptr->dscp;
	destRule_ptr->pbit = srcRule_ptr->pbit;
	destRule_ptr->gemPort = srcRule_ptr->gemPort;
	
	ret = 0;
end:
	return ret;
}

/*******************************************************************************************
**function name
	addConvertedGemPortRule
**description:
	call gem port API, add this omci gem port rule
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	rule_ptr: omci gem port rule
********************************************************************************************/
int addConvertedGemPortRule(IN omciGemPortMapping_ptr  rule_ptr){
	int ret = -1;
	gemPortMappingIoctl_ptr destRule_ptr  = NULL;
	int result = 0;

	if(rule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addConvertedGemPortRule->rule_ptr == NULL");
		goto end;
	}
	
	destRule_ptr = (gemPortMappingIoctl_ptr)calloc(1 , sizeof(gemPortMappingIoctl_t));
	if(destRule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addConvertedGemPortRule->calloc, fail");
		goto end;
	}

	/*1.convert omci rule format to gem port API format*/
	if(convertGemPortRule(rule_ptr, destRule_ptr) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addConvertedGemPortRule->convertGemPortRule,rule_ptr->index = %d,  fail", rule_ptr->index);
		goto end;
	}
	/*2. call gem port API to add gem port rule*/
	result = addGemPortMappingRule(destRule_ptr);
	if(result == GPONMAP_SUCCESS || result == GPONMAP_ENTRY_EXIST){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addConvertedGemPortRule->addGemPortMappingRule, rule_ptr->index = %d,  success", rule_ptr->index);
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addConvertedGemPortRule->addGemPortMappingRule, rule_ptr->index = %d,  fail", rule_ptr->index);
		goto end;
	}

	ret = 0;
end:
	if(destRule_ptr != NULL){
		free(destRule_ptr);
	}
	return ret;
}

/*******************************************************************************************
**function name
	delConvertedGemPortRule
**description:
	call gem port API, del this omci gem port rule
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	rule_ptr: omci gem port rule
********************************************************************************************/
int delConvertedGemPortRule(IN omciGemPortMapping_ptr  rule_ptr){
	int ret = -1;
	gemPortMappingIoctl_ptr destRule_ptr  = NULL;
	int result = 0;

	if(rule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delConvertedGemPortRule->rule_ptr == NULL");
		goto end;
	}
	
	destRule_ptr = (gemPortMappingIoctl_ptr)calloc(1 , sizeof(gemPortMappingIoctl_t));
	if(destRule_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delConvertedGemPortRule->calloc, fail");
		goto end;
	}

	/*1.convert omci rule format to gem port API format*/
	if(convertGemPortRule(rule_ptr, destRule_ptr) != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delConvertedGemPortRule->convertGemPortRule,rule_ptr->index = %d,  fail", rule_ptr->index);
		goto end;
	}
	/*2. call gem port API to delete gem port rule*/
	result = delGemPortMappingRule(destRule_ptr);
	if(result == GPONMAP_SUCCESS || result == GPONMAP_ENTRY_NOT_FOUND){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delConvertedGemPortRule->delGemPortMappingRule, rule_ptr->index = %d,  success", rule_ptr->index);
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delConvertedGemPortRule->delGemPortMappingRule, rule_ptr->index = %d,  fail", rule_ptr->index);
		goto end;
	}

	ret = 0;
end:
	if(destRule_ptr != NULL){
		free(destRule_ptr);
	}
	return ret;
}

/*******************************************************************************************
**function name
	findOmciGemPortMappingRule
**description:
	find gem port mapping rule
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	rule_ptr: source rule
  	index:	find rule index
********************************************************************************************/
int findOmciGemPortMappingRule(omciGemPortMapping_ptr rule_ptr, uint16 * index){
	uint8  findFlag = 0;
	omciGemPortMapping_ptr mapping_ptr = NULL;
	/*ctlFlag: if the bit value is 1, then this bit function is found .
		bit		description
		0		tag flag, 0:untagged, 1:tagged
		1		user port
		2		ani port
		3		vid
		4		dscp
		5		pbit
		6		gem port
		7		ME id (class id+instance id)
		8~15	reserved	*/	
	uint16 ctlFlag = 0;

	if((gemPortMapping_ptr == NULL) || (rule_ptr == NULL))
		goto end;
	
	mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr != NULL){
		if(rule_ptr->tagctl == mapping_ptr->tagctl){
			ctlFlag = 0;
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_TAGFLAG)
				&& (mapping_ptr->tagFlag== rule_ptr->tagFlag)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_TAGFLAG;
			}
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_USERPORT)
				&& (mapping_ptr->userPort == rule_ptr->userPort)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_USERPORT;
			}
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_ANI_PORT)
				&& (mapping_ptr->aniPort == rule_ptr->aniPort)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_ANI_PORT;
			}			
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_VID)
				&& (mapping_ptr->vid == rule_ptr->vid)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_VID;
			}
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_DSCP)
				&& (mapping_ptr->dscp == rule_ptr->dscp)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_DSCP;
			}
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_PBIT)
				&& (mapping_ptr->pbit == rule_ptr->pbit)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_PBIT;
			}
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_GEMPORT)
				&& (mapping_ptr->gemPort== rule_ptr->gemPort)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_GEMPORT;
			}			
			if((rule_ptr->tagctl & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
				&& (mapping_ptr->classIdInstId == rule_ptr->classIdInstId)){
					ctlFlag |= OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
			}		
			if(ctlFlag == rule_ptr->tagctl){
				findFlag = 1;
				break;
			}
		}
		mapping_ptr = mapping_ptr->next;
	}

end:
	
#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif
	
	if(findFlag == 1){
		*index = mapping_ptr->index;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findOmciGemPortMappingRule->find");
		return 0;
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n findOmciGemPortMappingRule->no find");
		return -1;
	}
}

/*******************************************************************************************
**function name
	modifyOmciGemPortMappingRuleByUnmarkOpt(dscp->default pbit, default pbit->dscp)
**description:
	modify gem port mapping rule by unmark option
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	flag: 0:set dscp for  gem port mapping in untagged frame, 1:set default pbit for gem port mapping in untagged frame. 
  	length: val length  	
  	val: 64 entry[dscp,pbit,gemport]  or 1 entry [pbit, gemport]
  	classIdInstId : class id + instance id
********************************************************************************************/
int modifyOmciGemPortMappingRuleByUnmarkOpt(uint8 flag, uint16 length, uint8 *val, uint32 classIdInstId){
	int ret = -1;
	omciGemPortMapping_ptr mapping_ptr = gemPortMapping_ptr;
	omciGemPortMapping_ptr temp_mapping_ptr = NULL;
	uint8 * temp = NULL;
	uint16 ctlFlag = 0;
	int i = 0;
	uint8 maxIndex = 0;
	uint16 delMatchFlag = 0;

//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->start");
	if(val == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->val==NULL");
		goto end;
	}

	if(length == 0){//don't need modify rule
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->don't need modify rule");
		goto end;
	}

	temp = val;
	if((flag == 0) && (length <= MAX_OMCI_DSCP_PBIT_GPORT_LEN)){
		//nothing, dscp pbit to gemport , 64 entry
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->dscp");

	}else if((flag == 1) && (length <= MAX_OMCI_PBIT_GPORT_LEN)){
		//nothing, default pbit to gemport, 1 entry
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->default pbit");
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->flag = %d, length=%d,  fail",flag,length);
		goto end;
	}

	temp_mapping_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
	if(temp_mapping_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->calloc failure");
		goto end;
	}
	
	while(mapping_ptr != NULL){
		/*find rule and delete these rule*/
		if( ((mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_TAGFLAG)
				&& (mapping_ptr->tagFlag ==  OMCI_GEMPORT_MAPPING_UNTAGGED_FLAG))
			&& ((mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
				&& (mapping_ptr->classIdInstId == classIdInstId))){
			if(flag == 0){//set untaged frame to using DSCP
				ctlFlag = OMCI_GEMPORT_MAPPING_DSCP; // untag rule is checked only by OMCI_GEMPORT_MAPPING_DSCP flag(0 or 1)
				if((mapping_ptr->tagctl & ctlFlag) == 0) {  //default to DSCP
					/* modify this rule*/			
					/*1. call gem port API to delete old gem port rule*/
					if(delConvertedGemPortRule(mapping_ptr) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->delConvertedGemPortRule, mapping_ptr->index = %d,  fail", mapping_ptr->index);
						continue;
					}
					/*2. update omci gem port rule*/
					temp = val;
					mapping_ptr->tagctl = (mapping_ptr->tagctl | OMCI_GEMPORT_MAPPING_DSCP);
					mapping_ptr->dscp = *temp++;
					mapping_ptr->pbit  = *temp++;
					mapping_ptr->gemPort = get16(temp);
					temp += 2;
					/*3. call gem port API to add new gem port rule*/
					if(addConvertedGemPortRule(mapping_ptr) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->addConvertedGemPortRule, mapping_ptr->index = %d,  fail", mapping_ptr->index);
						continue;
					}
					/*add dscp rule*/
					memcpy(temp_mapping_ptr, mapping_ptr, sizeof(omciGemPortMapping_t));
					maxIndex = (length/MAX_OMCI_DSCP_PBIT_GPORT_ITEM_LEN)-1;
					for(i = 0; i<maxIndex; i++){
						temp_mapping_ptr->dscp = *temp++;
						temp_mapping_ptr->pbit  = *temp++;
						temp_mapping_ptr->gemPort = get16(temp);
						temp += 2;
						ret = addOmciGemPortMappingRule(temp_mapping_ptr);
					}
				}
			}
				
			if(flag == 1){//set untaged frame to using default pbit
				if(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_DSCP){  //DSCP to default pbit
					/*prepare for del gem port mapping rule*/
					delMatchFlag = (OMCI_GEMPORT_MAPPING_TAGCTL
									|OMCI_GEMPORT_MAPPING_CLASSID_INSTID);					
					temp_mapping_ptr->tagctl = mapping_ptr->tagctl;
					temp_mapping_ptr->classIdInstId = mapping_ptr->classIdInstId;					
//					omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->temp_mapping_ptr->tagct=0x%02x, temp_mapping_ptr->classIdInstId=0x%02x",temp_mapping_ptr->tagctl,temp_mapping_ptr->classIdInstId);
					/* modify this rule*/
					/*1. call gem port API to delete old gem port rule*/
					if(delConvertedGemPortRule(mapping_ptr) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->delConvertedGemPortRule 2, mapping_ptr->index = %d,  fail", mapping_ptr->index);
						continue;
					}
					/*2. update omci gem port rule*/
					temp = val;
					mapping_ptr->tagctl = (mapping_ptr->tagctl & (~OMCI_GEMPORT_MAPPING_DSCP)); //OMCI_GEMPORT_MAPPING_PBIT)
					mapping_ptr->dscp = 0;
					mapping_ptr->pbit  = *temp++;
					mapping_ptr->gemPort = get16(temp);
					temp += 2;
					/*3. call gem port API to add new gem port rule*/
					if(addConvertedGemPortRule(mapping_ptr) != 0){
						omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRuleByUnmarkOpt->addConvertedGemPortRule 2, mapping_ptr->index = %d,  fail", mapping_ptr->index);
						continue;
					}
					/*del gem port mapping rule*/					
					ret = delOmciGemPortMappingRuleByMatchFlag(delMatchFlag, temp_mapping_ptr);	
					
				}
			}
			
		}
		mapping_ptr = mapping_ptr->next;
	}

	ret = 0;
end:

#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif
	if(temp_mapping_ptr != NULL){
		free(temp_mapping_ptr);
	}
	if(ret != 0){
		ret = -1;
	}
	return ret;
}
/*******************************************************************************************
**function name
	modifyOmciGemPortMappingRule
**description:
	modify gem port mapping rule
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	modifyFlag: modify flag
	  			bit		description
			  	0		tagFlag
				1		user port
				2		ani port
				3		VID
				4		DSCP
				5		P-bit
				6		GEM port			
				7		classIdInstId
	  			8		tagctl				
				9~15	reserved

	oldRule_ptr:	old rule information
	newRule_ptr:  new rule information
********************************************************************************************/
int modifyOmciGemPortMappingRule(uint16 modifyFlag, omciGemPortMapping_ptr oldRule_ptr, omciGemPortMapping_ptr newRule_ptr){
	int ret = -1;
	uint16 ctlFlag = 0;
	omciGemPortMapping_ptr mapping_ptr = gemPortMapping_ptr;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRule->modifyFlag=0x%02x",modifyFlag);
	if((oldRule_ptr == NULL) || (newRule_ptr == NULL))
		goto end;

	while(mapping_ptr != NULL){
		/*find mdify rule*/
		ctlFlag = 0;		
		if((modifyFlag & OMCI_GEMPORT_MAPPING_TAGCTL)
			&& (mapping_ptr->tagctl== oldRule_ptr->tagctl)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_TAGCTL;
		}
		if((modifyFlag & OMCI_GEMPORT_MAPPING_TAGFLAG)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_TAGFLAG)
			&& (mapping_ptr->tagFlag == oldRule_ptr->tagFlag)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_TAGFLAG;
		}
		if((modifyFlag & OMCI_GEMPORT_MAPPING_USERPORT)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_USERPORT)
			&& (mapping_ptr->userPort== oldRule_ptr->userPort)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_USERPORT;
		}
		if((modifyFlag & OMCI_GEMPORT_MAPPING_ANI_PORT)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_ANI_PORT)
			&& (mapping_ptr->aniPort == oldRule_ptr->aniPort)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_ANI_PORT;
		}		
		
		if((modifyFlag & OMCI_GEMPORT_MAPPING_VID)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_VID)
			&& (mapping_ptr->vid== oldRule_ptr->vid)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_VID;
		}
		if((modifyFlag & OMCI_GEMPORT_MAPPING_DSCP)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_DSCP)
			&& (mapping_ptr->dscp== oldRule_ptr->dscp)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_DSCP;
		}
		if((modifyFlag & OMCI_GEMPORT_MAPPING_PBIT)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_PBIT)
			&& (mapping_ptr->pbit== oldRule_ptr->pbit)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_PBIT;
		}

		if((modifyFlag & OMCI_GEMPORT_MAPPING_GEMPORT)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_GEMPORT)
			&& (mapping_ptr->gemPort== oldRule_ptr->gemPort)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_GEMPORT;
		}
		
		if((modifyFlag & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
			&&(mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
			&& (mapping_ptr->classIdInstId== oldRule_ptr->classIdInstId)){
				ctlFlag |= OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
		}
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRule->ctlFlag=0x%02x",ctlFlag);
		/*modify rule*/
		if(modifyFlag == ctlFlag){
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRule->modify ");
			/*1. call gem port API to del old gem port rule*/
			if(delConvertedGemPortRule(mapping_ptr) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRule->delConvertedGemPortRule, mapping_ptr->index=%d, fail", mapping_ptr->index);
				goto end;
			}
			/*2. update omci gem port mapping rule*/			
			if(modifyFlag & OMCI_GEMPORT_MAPPING_TAGCTL){
				mapping_ptr->tagctl= newRule_ptr->tagctl;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_TAGFLAG){
				mapping_ptr->tagFlag = newRule_ptr->tagFlag;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_USERPORT){
				mapping_ptr->userPort= newRule_ptr->userPort;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_ANI_PORT){
				mapping_ptr->aniPort = newRule_ptr->aniPort;
			}			
			if(modifyFlag & OMCI_GEMPORT_MAPPING_VID){
				mapping_ptr->vid= newRule_ptr->vid;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_DSCP){
				mapping_ptr->dscp= newRule_ptr->dscp;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_PBIT){
				mapping_ptr->pbit= newRule_ptr->pbit;
			}
			if(modifyFlag & OMCI_GEMPORT_MAPPING_GEMPORT){
				mapping_ptr->gemPort= newRule_ptr->gemPort;
			}
			
			if(modifyFlag & OMCI_GEMPORT_MAPPING_CLASSID_INSTID){
				mapping_ptr->classIdInstId = newRule_ptr->classIdInstId;
			}
			/*3. call gem port mapping API to add new rule*/
			if(addConvertedGemPortRule(mapping_ptr) != 0){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n modifyOmciGemPortMappingRule->addConvertedGemPortRule, mapping_ptr->index=%d, fail", mapping_ptr->index);
				goto end;
			}
		}
		mapping_ptr = mapping_ptr->next;
	}
	ret = 0;
end:
#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif
	if(ret != 0){
		ret = -1;
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRule->failure");
	}else{
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n modifyOmciGemPortMappingRule->success");
	}
	return ret;

}
/*******************************************************************************************
**function name
	getOmciGemPortMappingRuleMaxIndex
**description:
	get max rule index in list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	maxIndex: return max index result
********************************************************************************************/
int getOmciGemPortMappingRuleMaxIndex(uint16 * maxIndex){
	uint16 index = 0;
	int ret = -1;
	omciGemPortMapping_ptr mapping_ptr = NULL;
	
	if(maxIndex == NULL){
		goto end;
	}
	mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr != NULL){
		if(mapping_ptr->index > index){
			index = mapping_ptr->index;
		}
		mapping_ptr = mapping_ptr->next;
	}
	*maxIndex = index;
//	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getOmciGemPortMappingRuleMaxIndex->max index=%d",*maxIndex);
	ret = 0;
end:
	return ret;
	
}

/*******************************************************************************************
**function name
	addOmciGemPortMappingRule
**description:
	add rule in list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	rule_ptr:	source rule
********************************************************************************************/
int addOmciGemPortMappingRule(omciGemPortMapping_ptr rule_ptr){
	int ret = -1;
	omciGemPortMapping_ptr mapping_ptr = NULL;
	omciGemPortMapping_ptr temp_ptr = NULL;
	omciGemPortMapping_ptr  new_mapping_ptr = NULL;
	uint16 index = 0;
	uint16 maxIndex = 0;
	uint8 freeFlag = OMCI_GEMPORT_RULE_FREE_FLAG1;

	if(rule_ptr == NULL)
		goto end;

	/*set new mapping rule*/
	new_mapping_ptr = (omciGemPortMapping_ptr)calloc(1 , sizeof(omciGemPortMapping_t));
	if(new_mapping_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRule->calloc failure");
		goto end;
	}
	memcpy(new_mapping_ptr, rule_ptr, sizeof(omciGemPortMapping_t));

	/*find rule*/
	ret = findOmciGemPortMappingRule(new_mapping_ptr, &index);
	if(ret == 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRule->find success");
		ret = 0;
		goto end;
	}
	/*add rule*/
	ret = getOmciGemPortMappingRuleMaxIndex(&maxIndex);
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRule->getOmciGemPortMappingRuleMaxIndex, fail");
		ret = -1;
		goto end;
	}
	/*whether this table is overflow or not*/
	if(maxIndex >= (MAX_GEM_PORT_MAPPING_RULE_NUM-1)){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRule->overflow fail");
		goto end;
	}
	new_mapping_ptr->index = maxIndex+1;
	/*1. call gem port API to add gem port rule*/
	if(addConvertedGemPortRule(new_mapping_ptr) != 0){
		ret = -1;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRule->addConvertedGemPortRule, fail");
		goto end;
	}
	/*2. add omci gem port rule*/
	freeFlag = OMCI_GEMPORT_RULE_FREE_FLAG0;
	/*add in  header*/
	new_mapping_ptr->next = NULL;
	if(gemPortMapping_ptr == NULL){
		gemPortMapping_ptr = new_mapping_ptr;		
		ret = 0;
		goto end;
	}
	/*add in list tail*/
	mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr->next != NULL){
		mapping_ptr = mapping_ptr->next;
	}
	mapping_ptr->next = new_mapping_ptr;
	ret = 0;
end:

#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif

	if((freeFlag == OMCI_GEMPORT_RULE_FREE_FLAG1) && (new_mapping_ptr != NULL)){
		free(new_mapping_ptr);
	}
	if(ret != 0){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRule->fail");
		ret = -1;	
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRule->success");

	}
	return ret;
	
}

/*******************************************************************************************
**function name
	addOmciGemPortMappingRuleByUni
**description:
	add rule in list , according the user port 
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	userPort:	9.3.4 MEUNI port
  	classIdInstId: 9.3.4 classid+instance id
  	meIdArr: all classid+instance id in same mac bridge service profile
  	validNum: valid number in meIdArr array
********************************************************************************************/

int addOmciGemPortMappingRuleByUni(uint8 userPort, uint32 classIdInstId, uint32 *meIdArr, uint8 validNum){
	int ret = -1;
	omciGemPortMapping_ptr mapping_ptr = NULL;
	omciGemPortMapping_ptr mappingTail_ptr = NULL;
	omciGemPortMapping_ptr  new_mapping_ptr = NULL;	
	int i = 0;
	uint8 ruleLen = 0;
	uint16 maxIndex = 0;
	uint16 oldMaxIndex = 0;
	uint16 validMaxIndex = 0;	

	if((meIdArr == NULL) || (validNum > MAX_GEMPORT_VALID_MEID_NUM)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRuleByMeId->parameter error");
		return -1;
	}

	ruleLen = sizeof(omciGemPortMapping_t);
	validMaxIndex = MAX_GEM_PORT_MAPPING_RULE_NUM-1;

	/*get valid location for insert rule, get max index in current gemport list*/
	mappingTail_ptr = gemPortMapping_ptr;
	if(mappingTail_ptr == NULL){
		ret = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n addOmciGemPortMappingRuleByMeId->gemPortMapping_ptr==NULL, fail");
		goto end;
	}
	while(mappingTail_ptr->next != NULL){
		if(mappingTail_ptr->index > maxIndex){
			maxIndex = mappingTail_ptr->index;
		}
		mappingTail_ptr = mappingTail_ptr->next;
	}
	if(mappingTail_ptr->index > maxIndex){
		maxIndex = mappingTail_ptr->index;
	}
	
	oldMaxIndex = maxIndex;
	mapping_ptr = gemPortMapping_ptr;
	while((mapping_ptr != NULL) && (mapping_ptr->index <= oldMaxIndex)){
		if((mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_CLASSID_INSTID) == 0){
			mapping_ptr = mapping_ptr->next;
			continue;
		}
		if((mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_USERPORT) == OMCI_GEMPORT_MAPPING_USERPORT){
			mapping_ptr = mapping_ptr->next;
			continue;
		}
		for(i=0; i<validNum; i++){			
			if(mapping_ptr->classIdInstId != meIdArr[i]){
				continue;
			}
			/*add a rule to omci gemport list*/
			/*1.malloc space to save rule*/
			new_mapping_ptr = (omciGemPortMapping_ptr)calloc(1 , ruleLen);
			if(new_mapping_ptr == NULL){
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRuleByMeId->calloc failure");
				ret = -1;
				goto end;
			}
			memcpy(new_mapping_ptr, mapping_ptr, ruleLen);
			new_mapping_ptr->next = NULL;
			new_mapping_ptr->tagctl |= OMCI_GEMPORT_MAPPING_USERPORT;
			new_mapping_ptr->userPort = userPort;
			new_mapping_ptr->classIdInstId = classIdInstId;
			
			/*whether this table is overflow or not*/
			if(maxIndex >= validMaxIndex){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRuleByMeId->overflow fail");
				goto end;
			}
			new_mapping_ptr->index = maxIndex + 1;
			
			/*1. call gem port API to add gem port rule*/
			if(addConvertedGemPortRule(new_mapping_ptr) != 0){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n addOmciGemPortMappingRuleByMeId->addConvertedGemPortRule, fail");
				goto end;
			}
			/*2. add omci gem port rule*/
			/*add in  header*/
			if(mappingTail_ptr != NULL){
				mappingTail_ptr->next = new_mapping_ptr;//link new rule to list
				mappingTail_ptr = new_mapping_ptr; //update tail pointer
			}
			/*3 update some value for next loop*/
			maxIndex++;
			
		}
		mapping_ptr = mapping_ptr->next;
	}

	ret = 0;
end:

#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif

	if((ret != 0) && (new_mapping_ptr != NULL)){
		free(new_mapping_ptr);
	}
	return 0;
	
}


/*******************************************************************************************
**function name
	delOmciGemPortMappingRuleByMatchFlag
**description:
	del match  rule in list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	delMatchFlag: match rule  flag
	  			bit		description
			  	0		tagFlag
				1		user port
				2		ani port
				3		VID
				4		DSCP
				5		P-bit
				6		GEM port			
				7		classIdInstId
	  			8		tagctl				
				9~15	reserved
	currMappingRule: match rule value
********************************************************************************************/
int delOmciGemPortMappingRuleByMatchFlag(uint16 delMatchFlag, omciGemPortMapping_ptr currMappingRule){
	omciGemPortMapping_ptr mapping_ptr = NULL;
	int ret = -1;
	uint16 matchFlag = 0;
	omciGemPortMapping_ptr pre_mapping_ptr = NULL;
	omciGemPortMapping_ptr temp_mapping_ptr = NULL;
	uint16 index = 0;

	if(gemPortMapping_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delOmciGemPortMappingRuleByMatchFlag->gemPortMapping_ptr == NULL, ok");
		ret = 0;
		goto end;
	}
	
	if(currMappingRule == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delOmciGemPortMappingRuleByMatchFlag->currMappingRule == NULL, fail");
		goto end;
	}

	mapping_ptr = gemPortMapping_ptr;
	pre_mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr != NULL){
		matchFlag = 0;
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_TAGCTL)
			&& (mapping_ptr->tagctl == currMappingRule->tagctl)){
				matchFlag |= OMCI_GEMPORT_MAPPING_TAGCTL;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_TAGFLAG)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_TAGFLAG)
			&& (mapping_ptr->tagFlag== currMappingRule->tagFlag)){
				matchFlag |= OMCI_GEMPORT_MAPPING_TAGFLAG;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_GEMPORT)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_GEMPORT)
			&& (mapping_ptr->gemPort== currMappingRule->gemPort)){
				matchFlag |= OMCI_GEMPORT_MAPPING_GEMPORT;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_USERPORT)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_USERPORT)
			&& (mapping_ptr->userPort == currMappingRule->userPort)){
				matchFlag |= OMCI_GEMPORT_MAPPING_USERPORT;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_ANI_PORT)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_ANI_PORT)
			&& (mapping_ptr->aniPort == currMappingRule->aniPort)){
				matchFlag |= OMCI_GEMPORT_MAPPING_ANI_PORT;
		}		
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_VID)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_VID)
			&& (mapping_ptr->vid == currMappingRule->vid)){
				matchFlag |= OMCI_GEMPORT_MAPPING_VID;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_DSCP)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_DSCP)
			&& (mapping_ptr->dscp == currMappingRule->dscp)){
				matchFlag |= OMCI_GEMPORT_MAPPING_DSCP;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_PBIT)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_PBIT)
			&& (mapping_ptr->pbit == currMappingRule->pbit)){
				matchFlag |= OMCI_GEMPORT_MAPPING_PBIT;
		}
		if((delMatchFlag & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
			&& (mapping_ptr->tagctl & OMCI_GEMPORT_MAPPING_CLASSID_INSTID)
			&& (mapping_ptr->classIdInstId == currMappingRule->classIdInstId)){
				matchFlag |= OMCI_GEMPORT_MAPPING_CLASSID_INSTID;
		}
//		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delOmciGemPortMappingRuleByMatchFlag->delMatchFlag=0x%02x, matchFlag=0x%02x",delMatchFlag,matchFlag);
		if(delMatchFlag == matchFlag){//delete this entry
			/*1.call gem port mapping API to delete this rule*/
			if(delConvertedGemPortRule(mapping_ptr) != 0){
				ret = -1;
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n delOmciGemPortMappingRuleByMatchFlag->delConvertedGemPortRule, fail");
				goto end;
			}
			/*2.delete omci gem port rule*/
			if(mapping_ptr->index == gemPortMapping_ptr->index){//del header pointer
				gemPortMapping_ptr = mapping_ptr->next;
				temp_mapping_ptr = gemPortMapping_ptr;
				pre_mapping_ptr = gemPortMapping_ptr;
			}else{
				pre_mapping_ptr->next = mapping_ptr->next;
				temp_mapping_ptr = pre_mapping_ptr->next;
			}
//			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delOmciGemPortMappingRuleByMatchFlag->success mapping_ptr->index = %d", mapping_ptr->index);
			/*free omci gem port mapping rule*/
			free(mapping_ptr);
			mapping_ptr = temp_mapping_ptr;
			continue;
		}
		pre_mapping_ptr = mapping_ptr;
		mapping_ptr = mapping_ptr->next;
	}

	/*del rule success and update rule index*/
	mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr != NULL){
		mapping_ptr->index = ++index;
		mapping_ptr = mapping_ptr->next;
	}
	
	ret = 0;
end:

#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif

	if(ret != 0){
		ret = -1;
	}
	return ret;

}
#if 0
/*******************************************************************************************
**function name
	delGemPortMappingRule
**description:
	del rule in list
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	index :	deleted rule index
********************************************************************************************/
int delGemPortMappingRule(uint16 index){
	int ret = -1;
	omciGemPortMapping_ptr mapping_ptr = NULL;
	omciGemPortMapping_ptr pre_mapping_ptr = NULL;
	omciGemPortMapping_ptr temp_mapping_ptr = NULL;
	uint8 updateIndexFlag = 0;

	if(gemPortMapping_ptr == NULL){
		goto end;
	}
	mapping_ptr = gemPortMapping_ptr;	
	pre_mapping_ptr = gemPortMapping_ptr;
	while(mapping_ptr != NULL){
		if(mapping_ptr->index == index){
			if(mapping_ptr->index == gemPortMapping_ptr->index){//del header pointer
				gemPortMapping_ptr = mapping_ptr->next;
				temp_mapping_ptr = gemPortMapping_ptr;
			}else{
				pre_mapping_ptr->next = mapping_ptr->next;
				temp_mapping_ptr = pre_mapping_ptr->next;
			}
			free(mapping_ptr);
			updateIndexFlag = 1;
			break;
		}
		pre_mapping_ptr = mapping_ptr;
		mapping_ptr = mapping_ptr->next;
	}
	/*del rule success and update rule index*/
	if(updateIndexFlag == 1){
		while(temp_mapping_ptr != NULL){
			temp_mapping_ptr->index--;
			temp_mapping_ptr = temp_mapping_ptr->next;
		}
		ret = 0;
	}
	
end:
	if(ret != 0){
		ret = -1;	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delGemPortMappingRule->fail");
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n delGemPortMappingRule->success");
	}
	return ret;

}
#endif
#if 0
/*******************************************************************************************
**function name
	clearGemPortMappingRuleByInstId
**description:
	clear some gem port mapping rule by class id and instance id 
 **retrun :
 	-1:	failure
 	0:	success
**parameter:
  	classIdInstId : class id +instance id
********************************************************************************************/
int clearGemPortMappingRuleByInstId(uint32 classIdInstId){
	omciGemPortMapping_ptr mapping_ptr = gemPortMapping_ptr;
	omciGemPortMapping_ptr temp_mapping_ptr = NULL;
	int ret = -1;

	while(mapping_ptr != NULL){
		if(mapping_ptr->classIdInstId == classIdInstId){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n clearGemPortMappingRuleByInstId->mapping_ptr->index=0x%02x",mapping_ptr->index);
			temp_mapping_ptr = mapping_ptr->next;
			delGemPortMappingRule(mapping_ptr->index);
			mapping_ptr = temp_mapping_ptr;
			continue;
		}
		mapping_ptr = mapping_ptr->next;
	}

	ret = 0;
	return ret;
}
#endif
/*******************************************************************************************
**function name
	displayOmciCurrGemPortMappingRule
**description:
	display current  gem port mapping rule.
 **retrun :

**parameter:
  	
********************************************************************************************/
void displayOmciCurrGemPortMappingRule(){
	omciGemPortMapping_ptr mapping_ptr = gemPortMapping_ptr;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n displayOmciCurrGemPortMappingRule OMCI gemPortmapping-->");
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n index	tagctl	tagFlag	userPort	ani port	vid");
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n dscp	pbit	gemPort	classIdInstId");	
	while(mapping_ptr != NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,  "\r\n 0x%02x	0x%02x	0x%02x	0x%02x	0x%02x	0x%02x	0x%02x	0x%02x	0x%02x	0x%02x",
			mapping_ptr->index, mapping_ptr->tagctl,mapping_ptr->tagFlag, mapping_ptr->userPort, mapping_ptr->aniPort, mapping_ptr->vid, mapping_ptr->dscp,
			mapping_ptr->pbit, mapping_ptr->gemPort, mapping_ptr->classIdInstId);
		mapping_ptr = mapping_ptr->next;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n");

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n displayOmciCurrGemPortMappingRule-->displayAllGemPortMappingRule   API-->");
	displayAllGemPortMappingRule();
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n");
	return;
}

/*******************************************************************************************
**function name
	clearOmciGemportRule
**description:
	clear gem port mapping rule in omci.
 **retrun :

**parameter:
  	
********************************************************************************************/
void clearOmciGemportRule(void){
	omciGemPortMapping_ptr mapping_ptr = gemPortMapping_ptr;
	omciGemPortMapping_ptr tempMapping_ptr = NULL;

	while(mapping_ptr != NULL){
		tempMapping_ptr = mapping_ptr->next;
		free(mapping_ptr);
		mapping_ptr = tempMapping_ptr;
	}
	gemPortMapping_ptr = NULL;	

	if(gemPortMapping_ptr == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"clearOmciGemportRule gemPortMapping_ptr == NULL(finished) \r\n");
	}
	
#if 0//def TCSUPPORT_CT_PON
	if (gemPortMapping_ptr == NULL)
		tcapi_set("XPON_Common", "hasMap", "0");
	else
		tcapi_set("XPON_Common", "hasMap", "1");
#endif

}


