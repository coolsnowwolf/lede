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
	omci_me_layer2_data_services_func.c

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
#include "omci_me_mac_bridge.h"
#include "omci_me_layer2_data_services.h"
#include "mgr_type.h"
#include "mgr_sdi.h"


/*******************************************************************************************************************************
									C O N S T A N T S
										
********************************************************************************************************************************/

#define UPDATE_RULE_FOR_GPON_ICMPV6		0
#define ADD_EBT_CHAIN_FOR_GPON_ICMPV6	1
#define DEL_EBT_CHAIN_FOR_GPON_ICMPV6	2
#define GPON_ICMPV6_SH_EBT				"/tmp/etc/gpon_icmpv6_ebt.sh"

#define LAN_INTERFACE 						0
#define WAN_INTERFACE 						1

#define ICMPV6_ERROR_MSG_ATTR				1
#define ICMPV6_INFO_MSG_ATTR				2
#define ICMPV6_RS_MSG_ATTR					3
#define ICMPV6_MLD_MSG_ATTR				8
#define ICMPV6_UNKNOWN_MSG_ATTR			9

int icmpv6Type[6] = {133, 134, 135, 136, 137, 130};


/*******************************************************************************************************************************
						F U N C T I O N   D E C L A R A T I O N S
										
********************************************************************************************************************************/

int omciCreateActionForDot1agMaintenanceDomain(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
 int omciDeleteActionForDot1agMaintenanceDomain(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int omciCreateActionForDot1agMaintenanceAssociation(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
 int omciDeleteActionForDot1agMaintenanceAssociation(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int omciCreatActionForMACBridgePortICMPv6Process(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciInternalCreatActionForMACBridgePortICMPv6Process(uint16 classId, uint16 instanceId);
int omciDeleteActionForMACBridgePortICMPv6Process(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);

int8 updateRulesForGPONICMPv6(uint16 instanceId, int action);
int icmpv6MsgProcessForEbtables(int ifType, uint16 instanceId, char * ifName, int action);

extern int chmod(const char *path, mode_t mode);
extern int unlink(const char *pathname);


/*******************************************************************************************************************************
							9.3.18 Dot1 rate limiter

********************************************************************************************************************************/

int omciMeInitForDot1RateLimiter(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForDot1RateLimiter \r\n");

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

int32 setDot1RateLimiterTPType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if((tmpValue != 1) && (tmpValue != 2)){// 1:MAC bridge service profile; 2:802.1p mapper service profile
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = tmpValue;

	return 0;
}

#ifdef TCSUPPORT_OMCI_DOT1AG
/*******************************************************************************************************************************
						9.3.19 Dot1ag maintenance domain

********************************************************************************************************************************/
int omciMeInitForDot1agMaintenanceDomain(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForDot1agMaintenanceDomain \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateActionForDot1agMaintenanceDomain;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteActionForDot1agMaintenanceDomain;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciCreateActionForDot1agMaintenanceDomain(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 

	//add MD info api
	
	ret = omciCreateAction(meClassId , meInstant_ptr, omciPayLoad, msgSrc);

	if(ret != 0)
	{ 
		//delete MD info api
	}

	return ret; 	
}

int omciDeleteActionForDot1agMaintenanceDomain(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 

	ret = omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);
	if(ret == 0)
	{
		//delete MD info api	
	}

	return ret;
}

int32 setDot1agMaintenanceDomainMDLevel(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if(tmpValue > 7){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MD info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceDomainMDNameFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	/*
	1:none
	2:DNS-like name
	3:mac addr and UINT
	4:character string
	32:ICC-based
	*/
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3) && (tmpValue != 4) && (tmpValue != 32)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MD info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceDomainMDName1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MD info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceDomainMDName2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MD info API
		
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceDomainMHFCreation(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3)){
		// 1:none, 2:default, 3:explicit
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MD info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceDomainSenderIDPermission(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3) && (tmpValue != 4)){
		// 1:none, 2:clasis, 3:manage,4:classisManage
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MD info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

/*******************************************************************************************************************************
						9.3.20 Dot1ag maintenance association

********************************************************************************************************************************/
int omciMeInitForDot1agMaintenanceAssociation(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForDot1agMaintenanceAssociation \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateActionForDot1agMaintenanceAssociation;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteActionForDot1agMaintenanceAssociation;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciCreateActionForDot1agMaintenanceAssociation(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 

	//add MA info api
	
	ret = omciCreateAction(meClassId , meInstant_ptr, omciPayLoad, msgSrc);

	if(ret != 0)
	{ 
		//delete MA info api
	}

	return ret; 	
}

int omciDeleteActionForDot1agMaintenanceAssociation(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 

	ret = omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);
	if(ret == 0)
	{
		//delete MA info api
	}

	return ret;
}

int32 setDot1agMaintenanceDomainMANameFormat(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	/*
	1:primary VID
	2:character string
	3:2-octets integer
	4:VPN ID
	*/
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3) && (tmpValue != 4)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationShortMAName1(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationShortMAName2(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationCCMInterval(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationAssociatedVlans(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationMHFCreation(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3) && (tmpValue != 4)){
		// 1:none, 2:default, 3:explicit, 4:defer
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}

int32 setDot1agMaintenanceAssociationSenderIDPermission(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpValue = *((uint8*)value);
	if((tmpValue != 1) && (tmpValue != 2) && (tmpValue != 3) && (tmpValue != 4) && (tmpValue != 5)){
		// 1:none, 2:clasis, 3:manage,4:classisManage,5:defer
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n Invalid value\r\n");
		return -1;
	}
	
	//set MA info API
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_t *)meInstantPtr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;
}
#endif



/*******************************************************************************************************************************
9.3.32 Ethernet frame extended PM 

********************************************************************************************************************************/

int createEthernetFrameExtendedPM(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	omciCreateAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);
	return 0;
}


int delEthernetFrameExtendedPM(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 

	omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);

	return ret;
}


int32 setEthnetFrameExtendedPMFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}



int32 getEthnetFrameExtendedPMcontrolblockFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}



int32 getEthnetFrameExtendedPMFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if ((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, (omciAttribute->attriIndex)-2, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}


int omciMeInitForEthernetFrameExtendedPM(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_t *tmpOmciManageEntity_p =omciManageEntity_p;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForEthernetFrameExtendedPM \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = createEthernetFrameExtendedPM;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = delEthernetFrameExtendedPM;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	
	return 0;
}


/*******************************************************************************************************************************
				9.3.33 MAC bridge port ICMPv6 process preassign table

********************************************************************************************************************************/

int omciMeInitForMACBridgePortICMPv6Process(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForMACBridgePortICMPv6Process \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreatActionForMACBridgePortICMPv6Process;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteActionForMACBridgePortICMPv6Process;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciCreatActionForMACBridgePortICMPv6Process(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint16 classId = 0;
	uint16 instanceId = 0;

	if(omciPayLoad == NULL)
		return -1;
	
	if(msgSrc == INTERNAL_MSG)
	{
		classId = get16((uint8*)&(omciPayLoad->meId));
		instanceId = get16((uint8*)&(omciPayLoad->meId)+2);
		omciInternalCreatActionForMACBridgePortICMPv6Process(classId, instanceId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciCreatActionForMACBridgePortICMPv6Process: create success\n");
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciCreatActionForMACBridgePortICMPv6Process: fail, can be only created by ONU!\n");
		return -1;
	}

	return 0;
}

int omciInternalCreatActionForMACBridgePortICMPv6Process(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	tmpomciMeInst_p->deviceId = instanceId;//for receive message

	/*meId*/
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[3].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x6;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x9;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[5].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[6].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[7].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x1;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[8].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x1;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[9].attriName, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x5;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	/*deal with ebtables*/
	updateRulesForGPONICMPv6(instanceId, ADD_EBT_CHAIN_FOR_GPON_ICMPV6);
	
	return 0;
}

int omciDeleteActionForMACBridgePortICMPv6Process(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	int ret=0; 
	uint16 instanceId = 0;

	if(omciPayLoad == NULL)
		return -1;
	
	instanceId = get16((uint8*)&(omciPayLoad->meId)+2);
 
	ret = omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);
	if(ret == 0)
	{	
		updateRulesForGPONICMPv6(instanceId, DEL_EBT_CHAIN_FOR_GPON_ICMPV6); 
	}

	return ret;
}

int omciInitInstForMACBridgePortICMPv6Process(void)
{
	omciManageEntity_ptr currentME = NULL;
	int ret = 0;
	omciPayload_t cmdMsg;
	uint8 *tmp = NULL;

	currentME = omciGetMeByClassId(OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6);
	if(currentME== NULL)
		return -1;
	memset(&cmdMsg, 0, sizeof(cmdMsg));	
	cmdMsg.devId = OMCI_BASELINE_MSG;
	tmp = (uint8 *)&cmdMsg.meId;	
	put16(tmp, OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6);	/*me class id*/
	put16(tmp+2, 0x1);  									 /*me instance id*/
	ret = currentME->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT](OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6,  NULL,  &cmdMsg , INTERNAL_MSG);   //internal create func	
	if (ret == -1)		
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6 fail\n");

	return ret;
}

int8 updateRulesForGPONICMPv6(uint16 instanceId, int action)
{
	int ifType = 0;//0: Lan interface; 1: Wan interface
	char ifName[16] = {0};
	uint8 ifcType = 0;

	int ret = 0;

	//get interface type, and ifName, from  MAC bridge port
	memset(ifName, 0, sizeof(ifName));
	ifType = getMacBridgePortInstType(instanceId,&ifcType);	//from Wayne
	if(ifType == 0){//LAN interface
		//get lan portid
		ret = getMacBridgePortIfcByInst(instanceId, ifName);
		if(ret != 0)
			return -1;
	}else if(ifType == 1){//WAN interface
		//get ANI portid	
		ret = getMacBridgePortIfcByInst(instanceId, ifName);//from Wayne
		if(ret != 0)
			return -1;
	}else{		
		return -1;	
	}	

	if(icmpv6MsgProcessForEbtables(ifType, instanceId, ifName, action) != 0)
		return -1;

	return 0;
}

int icmpv6MsgProcessForEbtables(int ifType, uint16 instanceId, char * ifName, int action)
{
	FILE *fp = NULL;
	char chainName[32] = {0};
	uint16 length = 0;
	uint8 attrIndex = 0;
	uint8 tmpValue = 0;
	uint8 *attributeValuePtr = NULL;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	int i = 0;
	int ret = 0;

	currentME = omciGetMeByClassId(OMCI_CLASS_ID_MAC_BRIDGE_PORT_ICMPV6);
	if(currentME== NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	
	sprintf(chainName, "GPON_ICMPV6_EBT%d%d", ifType, instanceId);
	fp = fopen(GPON_ICMPV6_SH_EBT, "w+");
	if(action == ADD_EBT_CHAIN_FOR_GPON_ICMPV6)
	{
		fprintf(fp, "ebtables -t filter -F %s 2>/dev/null\n", chainName);
		fprintf(fp, "ebtables -t filter -D FORWARD -p IPv6 --ip6-proto 58 -j %s 2>/dev/null\n", chainName);
		fprintf(fp, "ebtables -t filter -X %s 2>/dev/null\n", chainName);
		
		fprintf(fp, "ebtables -t filter -N %s\n", chainName);
		fprintf(fp, "ebtables -t filter -A FORWARD -p IPv6 --ip6-proto 58 -j %s\n", chainName);
		fprintf(fp,"ebtables -t filter -F %s 2>/dev/null\n", chainName);
		fclose(fp);
		chmod(GPON_ICMPV6_SH_EBT ,777);
		system(GPON_ICMPV6_SH_EBT);
		unlink(GPON_ICMPV6_SH_EBT);
		return 0;
	}
	else if(action == DEL_EBT_CHAIN_FOR_GPON_ICMPV6)
	{
		fprintf(fp, "ebtables -t filter -F %s 2>/dev/null\n", chainName);
		fprintf(fp, "ebtables -t filter -D FORWARD -p IPv6 --ip6-proto 58 -j %s\n", chainName);
		fprintf(fp, "ebtables -t filter -X %s 2>/dev/null\n", chainName);
		fclose(fp);
		chmod(GPON_ICMPV6_SH_EBT ,777);
		system(GPON_ICMPV6_SH_EBT);
		unlink(GPON_ICMPV6_SH_EBT);
		return 0;
	}
	else if(action ==	UPDATE_RULE_FOR_GPON_ICMPV6)
	{
		fprintf(fp,"ebtables -t filter -F %s 2>/dev/null\n", chainName);
		for(attrIndex = 1; attrIndex<10; attrIndex++)
		{
			/*icmpv6 types*/
			currentAttribute = &currentME->omciAttriDescriptList[attrIndex];
			attributeValuePtr = omciGetInstAttriByName(meInstantCurPtr , currentAttribute->attriName, &length);
			if(attributeValuePtr == NULL){
				ret = -1;
			}
			/*get setting for icmpv6 types*/
			tmpValue =  (*attributeValuePtr)&0x0F;
			/*
			* bit			name			setting
			* 1-2(LSB)		upstream		00:forward
											01:discard
											10:snoop
			* 3-4			downstream	00:forward
											01:discard
											10:snoop
			* 5-8			Reserved		0
			*/
			if(attrIndex == ICMPV6_ERROR_MSG_ATTR){
				//process upstream
				if((tmpValue&0x03) == 0){//upstream:forward
					if(ifType == LAN_INTERFACE){
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, i);
					}else{
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, i);
					}
				}else if((tmpValue&0x03) == 1){//upstream:discard
					if(ifType == LAN_INTERFACE){
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, i);
					}else{
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, i);
					}
				}else if((tmpValue&0x03) == 2){//upstream:snoop
					;
				}
				else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}

				//process downstream
				if((tmpValue&0x0c) == 0){//downstream:forward
					if(ifType == LAN_INTERFACE){
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, i);
					}else{
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, i);
					}
				}else if((tmpValue&0x0c) == 1){//downstream:discard
					if(ifType == LAN_INTERFACE){
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, i);
					}else{
						for(i=1; i<5; i++)
							fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, i);
					}
				}else if((tmpValue&0x0c) == 2){//downstream:snoop
					;
				}
				else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}
			}
			else if(attrIndex == ICMPV6_INFO_MSG_ATTR){
				//process upstream
				if((tmpValue&0x03) == 0){//upstream:forward
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j ACCEPT\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j ACCEPT\n", chainName, ifName);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j ACCEPT\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j ACCEPT\n", chainName, ifName);
					}
				}else if((tmpValue&0x03) == 1){//upstream:disard
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j DROP\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j DROP\n", chainName, ifName);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j DROP\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j DROP\n", chainName, ifName);
					}
				}else if((tmpValue&0x03) == 2){//upstream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}

				//process downstream
				if((tmpValue&0x0c) == 0){//downstream:forward
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j ACCEPT\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j ACCEPT\n", chainName, ifName);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j ACCEPT\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j ACCEPT\n", chainName, ifName);
					}
				}else if((tmpValue&0x0c) == 1){//downstream:disard
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j DROP\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j DROP\n", chainName, ifName);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 128 -j DROP\n", chainName, ifName);
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type 129 -j DROP\n", chainName, ifName);
					}
				}else if((tmpValue&0x0c) == 2){//downstream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}
			}
			else if((attrIndex >= ICMPV6_RS_MSG_ATTR) && (attrIndex <= ICMPV6_MLD_MSG_ATTR)){
				//process upstream
				if((tmpValue&0x03) == 0){//upstream:forward
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					}
				}else if((tmpValue&0x03) == 1){//upstream:discard
					if(ifType == LAN_INTERFACE){
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					}else{
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					}
				}else if((tmpValue&0x03) == 2){//upstream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}

				//process downstream
				if((tmpValue&0x0c) == 0){//downstream:forward
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					else
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j ACCEPT\n", chainName, ifName, icmpv6Type[attrIndex-3]);
				}else if((tmpValue&0x0c) == 1){//downstream:discard
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, icmpv6Type[attrIndex-3]);
					else
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 --ip6-icmpv6type %d -j DROP\n", chainName, ifName, icmpv6Type[attrIndex-3]);
				}else if((tmpValue&0x0c) == 2){//downstream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}
			}
			else{// if(attrIndex == ICMPV6_UNKNOWN_MSG_ATTR){
				//process upstream
				if((tmpValue&0x03) == 0){//upstream:forward
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 -j ACCEPT\n", chainName, ifName);
					else
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 -j ACCEPT\n", chainName, ifName);
				}else if((tmpValue&0x03) == 1){//upstream:discard
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 -j DROP\n", chainName, ifName);
					else
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 -j DROP\n", chainName, ifName);
				}else if((tmpValue&0x03) == 2){//upstream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}

				//process downsream
				if((tmpValue&0x0c) == 0){//downsream:forward
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 -j ACCEPT\n", chainName, ifName);
					else
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 -j ACCEPT\n", chainName, ifName);
				}else if((tmpValue&0x0c) == 1){//downsream:discard
					if(ifType == LAN_INTERFACE)
						fprintf(fp, "ebtables -t filter -A %s -o %s -p IPv6 --ip6-proto 58 -j DROP\n", chainName, ifName);
					else
						fprintf(fp, "ebtables -t filter -A %s -i %s -p IPv6 --ip6-proto 58 -j DROP\n", chainName, ifName);
				}else if((tmpValue&0x0c) == 2){//downsream:snoop
					;
				}else{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n invalid value \r\n");
					ret = -1;
				}
			}
		}
		fclose(fp);
		chmod(GPON_ICMPV6_SH_EBT ,777);
		system(GPON_ICMPV6_SH_EBT);
		unlink(GPON_ICMPV6_SH_EBT);
		return ret;
	}

	return 0;
}

int32 setMACBridgePortICMPv6ProcessMsg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	tmpValue = *((uint8*)value);
	/*
			* bit			name			setting
			* 1-2(LSB)		upstream		00:forward
											01:discard
											10:snoop
			* 3-4			downstream	00:forward
											01:discard
											10:snoop
			* 5-8			Reserved		0
	*/
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	/*update ebtables rules*/
	updateRulesForGPONICMPv6(instanceId, UPDATE_RULE_FOR_GPON_ICMPV6);
	
	return 0;
}



/*******************************************************************************************************************************
				9.3.34 Ethernet frame extended pm 64-bit

********************************************************************************************************************************/

int omciMeInitForEthernetFrameExtendedPM64bit(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForEthernetPMHistoryData \r\n");

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


int32 setEthnetFrameExtendedPM64BitFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}
int32 getEthnetFrameExtendedPM64BitcontrolblockFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

char *ctrlfield_str[]=
{
	"threshold3",
    "me_type",
    "me_instance",
    "accumulate_disable",
    "tca_disable",
    "ctrl_field",
    "tci",
    "resved",
};

int32 getEthFrameExtndPM64CtrlBlkDetail(char *value,EthExtndPM64CtrlBlk_t *ctrl_data)
{
	uint8 	i = 0;
    uint16 	*attr = ctrl_data;

    if(value == NULL || ctrl_data == NULL)
   	{
   		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthFrameExtndPM64CtrlBlkDetail input NULL \n");
        return -1;
   	}
    
    for(i = 0; i < PM_64BIT_CTRL_BLK_ATTRI_NUM;i++,attr++,value = value+2)
    {
    	*attr = *((uint16*)value);
        omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"Omci Get PM 64 attr %20s value is %x \n",ctrlfield_str[i],*attr);
    }
    
    return 0;
}

EthExtendPM64ThrholdPmMap_t gPM64ThresholdPMMap[] = 
{
	{THRHOLD_DROP_EVENT,         PM_DROP_EVENT},
    {THRHOLD_CRC_ERROR_FRAMES,   PM_CRC_ERROR_FRAMES},
    {THRHOLD_UNDERSIZE_FRAMES,   PM_UNDERSIZE_FRAMES},
    {THRHOLD_OVERSIZE_FRAMES,    PM_OVERSIZE_FRAMES},
};

int32 setEthnetFrameExtendedPM64BitThreshold(EthExtndPM64CtrlBlk_t *ctrl_data,omciMeInst_t *tmpomciMeInst_p)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;
	uint64 thresholdData = 0;
	uint16 length = 0;
    uint8 index = 0;
	
	if(ctrl_data->me_instance == 0 || ctrl_data->me_instance == 0xFFFF)
		return 0;

    for(index = 0;index < PM_64BIT_ALERT_ATTRI_NUM; index++)
    {
		ret = get64BitThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA_64_BIT, ctrl_data->threshold3, gPM64ThresholdPMMap[index].threhold_idx, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%llx", thresholdData);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"%s pmmgr set attri %d \n",__FUNCTION__,gPM64ThresholdPMMap[index].threhold_idx);
		ret = pmmgrTcapiSet(tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, gPM64ThresholdPMMap[index].pm_idx, tempBuffer);
		if (ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"ERROR %s pmmgr set attri %d fail \n",__FUNCTION__,gPM64ThresholdPMMap[index].threhold_idx);
			return -1;
		}
    }	
}


int32 setEthnetFrameExtendedPM64BitctlblkFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	EthExtndPM64CtrlBlk_t ctrl_data = {0};
    omciMeInst_t *tmpomciMeInst_p = NULL;
    uint16	portID = 0;
    int32	ret = 0;
    
    tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	getEthFrameExtndPM64CtrlBlkDetail(value,&ctrl_data);
    tcdbg_printf("Omci Get me %d instance %x direction %x\n",ctrl_data.me_type,ctrl_data.me_instance,ctrl_data.ctrl_field.direction);
    setEthnetFrameExtendedPM64BitThreshold(&ctrl_data,tmpomciMeInst_p);
    
	return 0;
}

int32 getEthnetFrameExtendedPM64BitFunc(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, (omciAttribute->attriIndex - 2), tempBuffer);
	if (ret != 0)
		return -1;
	sscanf(tempBuffer, "%llx", &pmData);
	put64((uint8 *)value, pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"OMCI get 64bit PM attr %s index %u val %s\n",
        omciAttribute->attriName,omciAttribute->attriIndex,tempBuffer);
	return 0;
}


