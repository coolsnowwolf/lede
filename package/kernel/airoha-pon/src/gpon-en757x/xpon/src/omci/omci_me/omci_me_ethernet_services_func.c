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
	omci_me_ethernet_services_func.c

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
#include "omci_me_ethernet_services.h"
#include "omci_me_general_purpose.h"
#ifdef TCSUPPORT_MT7530_SWITCH_API
#include "../ethcmd/libmii.h"
#include "../lib/mtkswitch_api.h"
#endif
#include "libcompileoption.h"

/*******************************************************************************************************************************
						F U N C T I O N   D E C L A R A T I O N S
										
********************************************************************************************************************************/

int omciCreatActionForPPTPEthernetUNI(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciInternalCreatActionForPPTPEthernetUNI(uint16 classId, uint16 instanceId);
int omciCreatActionForVirtualEthernetInterfacePoint(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc);
int omciInternalCreatActionForVirtualEthernetInterfacePoint(uint16 classId, uint16 instanceId);

extern int sendOmciNotifyMsg(notify_msg_t *buf);
extern int chmod(const char *path, mode_t mode);
extern int unlink(const char *pathname);

/*******************************************************************************************************************************
							9.5.1 PPTP Ethernet UNI

********************************************************************************************************************************/

int omciMeInitForPPTPEthernetUNI(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForPPTPEthernetUNI \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciInternalCreatActionForPPTPEthernetUNI(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[10] = {0};
	uint16 attriMask = 0;
	int ret = 0;
	uint8 mode = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	tmpomciMeInst_p->deviceId = ((instanceId & 0xff) - 1);//for receive message
	tmpomciMeInst_p->otherInfo = osMalloc(1);
	if(tmpomciMeInst_p->otherInfo == NULL)
	{
		goto fail;
	}

	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*1---Expected type*/	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x2f;
	else
		goto fail;

	/*2---Sensed type*/	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		if(TCSUPPORT_CT_PON_CZ_GD_VAL && (instanceId > ((BASET101001000_SLOT<<8) | 1) ) )
        {
		    *attributeValuePtr = 0x18;
		}
        else
		{
		    *attributeValuePtr = 0x2f;
		}
	}
	else
		goto fail;
	
	/*Auto detection configuration*/	
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Ethernet loopback configuration*/	
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Administrative state*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

#ifdef TCSUPPORT_MT7530_SWITCH_API	
	ret = macMT7530GetConfigurationStatus(tmpomciMeInst_p->deviceId);
	if(ret != -1){
		mode = (uint8)ret;
	}else{
		mode = 0;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNICfgIndValue->macMT7530GetConfigurationStatus fail[%d]",__LINE__);		
	}	
#endif
	/*Operational state*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
	
#if !defined(TCSUPPORT_CUC)
		if(mode == 0)
			*attributeValuePtr = 1;
		else
#endif
		*attributeValuePtr = 0;
	}
	else
		goto fail;

	/*Configuration ind*/
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = mode;
	else
		goto fail;

	/*Max frame size*/
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr , 0x05EE);
	else
		goto fail;

	/*DTE or DCE ind*/
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;//PHY default value: auto
	else
		goto fail;

	/*Pause time*/
	attriMask = 1<<6;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, buffer, 2);
#ifdef TCSUPPORT_MT7530_SWITCH_API		
		ret = macMT7530SetPauseTime(0, tmpomciMeInst_p->deviceId);
		if(ret == -1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIPauseTime->macMT7530SetPauseTime fail[%d]",__LINE__); 
		}
#endif
	}
	else
		goto fail;

	/*Bridged or IP ind*/
	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;  /* reference from dasan ONU */
	else
		goto fail;

	/*ARC*/
	attriMask = 1<<4;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*ARC interval*/
	attriMask = 1<<3;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x0;
	else
		goto fail;

	/*PPPoE filter*/
	attriMask = 1<<2;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Power control*/
	attriMask = 1<<1;
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

#ifdef TCSUPPORT_OMCI_ALCATEL
int omciInternalCreatActionForVlanTagDownstreamSupplemental1(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	omciTableAttriValue_t *tmpTableValue_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[10] = {0};
	uint16 attriMask = 0;
	int ret = 0;
	uint8 mode = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	
	tmpomciMeInst_p->deviceId = ((instanceId & 0xff) - 1);//for receive message
	tmpomciMeInst_p->otherInfo = osMalloc(1);
	if(tmpomciMeInst_p->otherInfo == NULL)
	{
		goto fail;
	}
	
	tmpTableValue_p = osMalloc(sizeof(omciTableAttriValue_t));
	if(tmpTableValue_p == NULL){
		return NULL;
 	}
	tmpTableValue_p->attriIndex = 2;
	tmpTableValue_p->tableSize = 0;
	tmpTableValue_p->tableValue_p = NULL;
	tmpTableValue_p->next = NULL;
	
 	tmpomciMeInst_p->tableValueList = tmpTableValue_p;

	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*1---Vlan Tx Default Tag*/	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0x2;
	else
		goto fail;
	
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);	
	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;		
}
#ifdef TCSUPPORT_UPSTREAM_VLAN_POLICER
int omciInternalCreatActionForVlanTagUpstreamPolicer(uint16 instanceId)
{
        omciManageEntity_t *pME = NULL;
        omciMeInst_t *pMEInst = NULL;
        omciTableAttriValue_t *pTblAttr = NULL;
        uint8 * attributeValuePtr = NULL;
        uint16 length = 0;
        uint8 buffer[10] = {0};
        uint16 attriMask = 0;
        int ret = 0;
        uint8 mode = 0;

        pME = omciGetMeByClassId(OMCI_CLASS_ID_VLAN_TAG_UPSTREAM_POLICER);
        if(pME == NULL)
                return -1;
        
        if (NULL == (pMEInst = omciAllocInstance(pME) ) ){
                return -1;
        }
        
        pMEInst->deviceId = ((instanceId & 0xff) - 1);//for receive message
        if (NULL == (pMEInst->otherInfo = osMalloc(1) ) ){
                goto fail;
        }
        
        if (NULL == (pTblAttr = osMalloc(sizeof(omciTableAttriValue_t) ) ) ){
                goto fail;
        }        
        
        pTblAttr->attriIndex = 1;
        pTblAttr->tableSize = 0;
        pTblAttr->tableValue_p = NULL;
        pTblAttr->next = NULL;
        
        pMEInst->tableValueList = pTblAttr;

        /*meId*/
        attributeValuePtr = pMEInst->attributeVlaue_ptr;
        if (attributeValuePtr != NULL){
                put16(attributeValuePtr, instanceId);
        }

        omciAddInstanceToTail(pME, pMEInst);    
        return 0;
        
fail:
        omciFreeInstance(pMEInst);
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
        return -1;              
}
#endif
int omciMeInitForUNISupplemental1V2(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForUNISupplemental1V2 \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciInternalCreatActionForUNISupplemental1V2(uint16 classId, uint16 instanceId)
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

	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*Downstream Tag Handling Mode*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 2;
	else
		goto fail;

	/*IGMP Access Control Method*/
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Multicast VID Value*/
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, 4096);
	else
		goto fail;

	/*Multicast P-Bit Value*/
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 8;
	else
		goto fail;

	/*IGMPChannelBridgePortNumber*/
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Upstream Default Priority*/
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;

	/*Maximum Number of Hosts per UNI*/
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 8;
	else
		goto fail;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	return 0;
	
fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;		
}

int32 getDownStreamTagHandlingModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setDownStreamTagHandlingModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 portId = 0;
	FILE *fp = NULL;
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 op = 0;
	uint16 length = 0;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	op = *((uint8*)value);

	fp = fopen("/proc/tc3162/eth_down_vlan", "w");
	if (!fp)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setDownStreamTagHandlingModeValue: fopen fail\n");
		return -1;
	}
	else
	{
		if ((op == 1) || (op == 2)) // 1: STRIP TAG Mode; 2: TAG THRU Mode.
		{
			fprintf(fp, "set_defop %d %d\n", portId, op);
			fclose(fp);
		}
		else
		{
			fclose(fp);
			return -1;
		}
	}
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = op;
	
	return 0;
}

int32 getIgmpAccessControlMethodValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setIgmpAccessControlMethodValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 getMulticastVIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setMulticastVIDValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 getMulticastPBitValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setMulticastPBitValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 getIgmpChannelBridgePortNumberValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setIgmpChannelBridgePortNumberValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 getUpstreamDefaultPriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 setUpstreamDefaultPriorityValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

int32 getMaximumNumberOfHostsPerUNIValue(char *value, uint8 *meInstantPtr, omciAttriDescript_t * omciAttribute, uint8 flag)
{
	return 0;
}

int32 setMaximumNumberOfHostsPerUNIValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;	
}
#endif

int omciInitInstForPPTPEthernetUNI(void)
{
	int ret = 0;
	uint16 i = 0;
	uint16 insID = 0;
	/*Physical path termination point Ethernet UNI*/
	for(i = 0x1; i < (pSystemCap->geNum + pSystemCap->feNum + 1); i++)
	{
		if(isFiberhome_551601){
			insID = i;
		}
		else{
			insID = ((BASET101001000_SLOT<<8) | i);
		}
		omciInitInstForUniG(insID);
		ret = omciInternalCreatActionForPPTPEthernetUNI(OMCI_CLASS_ID_PPTP_ETHERNET_UNI, insID);   //internal create func	
		if (ret == -1)		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForPPTPEthernetUNI create OMCI_CLASS_ID_PPTP_ETHERNET_UNI fail\n");

#ifdef TCSUPPORT_OMCI_ALCATEL
		ret = omciInternalCreatActionForVlanTagDownstreamSupplemental1(OMCI_CLASS_ID_VLAN_TAG_DOWNSTREAM_EGRESS_BEHAVIOR_SUPPLEMENTAL1, insID);   //internal create func	
		if (ret == -1)		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciMeInitForVlanTagDownstreamSupplemental1 create OMCI_CLASS_ID_VLAN_TAG_DOWNSTREAM_EGRESS_BEHAVIOR_SUPPLEMENTAL1 fail\n");
        #ifdef TCSUPPORT_UPSTREAM_VLAN_POLICER
		ret = omciInternalCreatActionForVlanTagUpstreamPolicer(insID);	
		if (ret == -1)		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciMeInitForVlanTagUpstreamPolicer create OMCI_CLASS_ID_VLAN_TAG_UPSTREAM_POLICER fail\n");
	#endif

		ret = omciInternalCreatActionForUNISupplemental1V2(OMCI_CLASS_ID_UNI_SUPPLEMENTAL_1_V2,  insID);  //internal create func	
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_UNI_SUPPLEMENTAL_1_V2 fail\n");
#endif
	}
#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
        InitUpStrmPolicerTrtcm();
#endif
	return ret;
}

int32 setPPTPEthernetUNIExpectedType(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getPPTPEthernetUNIAutoDetectConfig(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;

	//api
	#ifdef TCSUPPORT_MT7530_SWITCH_API
	ret = macMT7530GetAutoDetection(&mode, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNIAutoDetectConfig->macMT7530GetAutoDetection fail[%d]",__LINE__);		
	}
	#endif
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = mode;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 setPPTPEthernetUNIAutoDetectConfig(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint8 mode = 0;
	uint8 oldMode = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	mode = *((uint8*)value);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	oldMode = *attributeValuePtr;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setPPTPEthernetUNIPauseTime->oldMode = %d, mode = %d, [%d]",oldMode, mode, __LINE__);	
	if (oldMode != mode)
	{
		*attributeValuePtr = mode;
	//api
	#ifdef TCSUPPORT_MT7530_SWITCH_API
	ret = macMT7530SetAutoDetection(mode, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIAutoDetectConfig->macMT7530SetAutoDetection fail[%d]",__LINE__);		
	}	
	#endif
	}
	return 0;
}

int32 getPPTPEthernetUNIEthernetLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;

	//api
	#ifdef TCSUPPORT_MT7530_SWITCH_API
	ret = macMT7530GetLoopbackConf(&mode, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNIEthernetLoopbackCfg->macMT7530GetLoopbackConf fail[%d]",__LINE__);		
	}	
	#endif

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = mode;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 setPPTPEthernetUNIEthernetLoopbackCfg(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 portId = 0;
	int ret = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	
	tmpValue = *((uint8*)value);
	
	if((tmpValue != 0x0)
		&& (tmpValue != 0x3)){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setPPTPEthernetUNIEthernetLoopbackCfg para error, tmpValue=0x%02x",tmpValue);
		return -1;
	}
	//api
#ifdef TCSUPPORT_MT7530_SWITCH_API	
	if(tmpValue == 0x0){
		ret = macMT7530SetLoopbackConf(0, portId);
	}else if(tmpValue == 0x3){
		ret = macMT7530SetLoopbackConf(1, portId);
	}
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIEthernetLoopbackCfg->macMT7530SetLoopbackConf fail[%d]",__LINE__);		
	}	
#endif
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = tmpValue;
	
	return 0;
}

int32 getPPTPEthernetUNIAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attrMask = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;

#if 0
	/*Operational state*/
	attrMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_ptr , attrMask, &length);
	if(attributeValuePtr == NULL)
		return -1;
	
	*value = *attributeValuePtr;
#endif
	if (tmpomciMeInst_ptr->otherInfo != NULL)
 		*value = *tmpomciMeInst_ptr->otherInfo;
	
	return 0;
}

int32 setPPTPEthernetUNIAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 portId = 0;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint8 oldValue = 0;
	uint8 tmpValue = 0;
	char cmdbuf[66] = {0};
	notify_msg_t msg;
	uint8 ret = 0;
	uint16 attrMask = 0;
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;	
	tmpValue = *((uint8*)value);
#if 0
	/*Operational state*/
	attrMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_ptr , attrMask, &length);
	if(attributeValuePtr == NULL)
		return -1;
#endif
	if (tmpomciMeInst_ptr->otherInfo != NULL)
 		oldValue = *tmpomciMeInst_ptr->otherInfo;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setPPTPEthernetUNIAdminStateValue->oldValue = %d, tmpValue = %d, [%d]",oldValue, tmpValue, __LINE__);	
	/*deal with op state AVC, if admin state changes, we should send op state AVC*/
	if(tmpValue != oldValue){		
#ifdef TCSUPPORT_MT7530_SWITCH_API	
		ret = macMT7530SetPortAdmin(tmpValue, portId);
		if(ret == -1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIAdminStateValue->macMT7530SetPortAdmin fail[%d]",__LINE__);	
	}
#endif
	if (tmpomciMeInst_ptr->otherInfo != NULL)
		*tmpomciMeInst_ptr->otherInfo = tmpValue;
	#if 0
		memset(&msg, 0, sizeof(msg));
		msg.notifyType = OMCI_NOTIFY_TYPE_AVC;
		msg.avcId = OMCI_AVC_ID_PPTPEthernetUNI_OPERATIONAL_STATE;
		msg.deviceId = portId;

		if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\navcMsgHandle: sendOmciNotifyMsg fail!\r\n");
	#endif
	}
	
	return 0;
}

int32 getPPTPEthernetUNICfgIndValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;

	//api
#ifdef TCSUPPORT_MT7530_SWITCH_API	
	ret = macMT7530GetConfigurationStatus(portId);
	if(ret != -1){
		mode = (uint8)ret;
	}else{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNICfgIndValue->macMT7530GetConfigurationStatus fail[%d]",__LINE__);		
	}	

#endif

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = mode;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 getPPTPEthernetUNIMaxFrameSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 maxFrameSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	
	//api
#ifdef TCSUPPORT_MT7530_SWITCH_API	
	ret = macMT7530GetMaxFrameSize(&maxFrameSize, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNIMaxFrameSize->macMT7530GetMaxFrameSize fail[%d]",__LINE__); 	
	}
#endif
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	put16(attributeValuePtr , maxFrameSize);
	
	return getTheValue(value, (char *)attributeValuePtr, 2, omciAttribute);
}

int32 setPPTPEthernetUNIMaxFrameSize(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 maxFrameSize = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	maxFrameSize = get16((uint8 *)value);
#if !defined(TCSUPPORT_CHS) 
#ifdef TCSUPPORT_MT7530_SWITCH_API		
	ret = macMT7530SetMaxFrameSize(maxFrameSize, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIMaxFrameSize->macMT7530SetMaxFrameSize fail[%d]",__LINE__); 	
	}

#endif	
#endif

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	put16(attributeValuePtr , maxFrameSize);
	
	return 0;
}

int32 getPPTPEthernetUNIDTEOrDCEInd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint8 ind = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	//api 
#ifdef TCSUPPORT_MT7530_SWITCH_API		
	ret = macMT7530GetDTEDCEInd(&ind, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNIDTEOrDCEInd->macMT7530GetDTEDCEInd fail[%d]",__LINE__);	
	}
#endif
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = ind;
	
	return getTheValue(value, (char *)attributeValuePtr, 1, omciAttribute);
}

int32 setPPTPEthernetUNIDTEOrDCEInd(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint8 ind = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	ind = *((uint8*)value);

	//api 
#ifdef TCSUPPORT_MT7530_SWITCH_API
	if(ind != OMCI_ETH_UNI_DTE_DCE_AUTO){//0,1 is test mode, and this mode can't work, so only support auto mode(value=2)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIDTEOrDCEInd->ind = %d (only support auto),  fail[%d]",ind,__LINE__);
		return 0;  /* workround for dasan lab test , because we only support auto mode(value=2)*/
	}
	ret = macMT7530SetDTEDCEInd(ind, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIDTEOrDCEInd->macMT7530SetDTEDCEInd fail[%d]",__LINE__);	
	}
#endif
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	*attributeValuePtr = ind;
	
	return 0;
}

int32 getPPTPEthernetUNIPauseTime(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 pauseTime = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;

	//api
#ifdef TCSUPPORT_MT7530_SWITCH_API	
	ret = macMT7530GetPauseTime(&pauseTime, portId);
	if(ret == -1){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n getPPTPEthernetUNIPauseTime->macMT7530GetPauseTime fail[%d]",__LINE__);	
	}
#endif
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	put16(attributeValuePtr, pauseTime);
	
	return getTheValue(value, (char *)attributeValuePtr, 2, omciAttribute);
}

int32 setPPTPEthernetUNIPauseTime(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	int ret = 0;
	uint16 pauseTime = 0;
	uint16 oldPauseTime = 0;
	char tempBuffer[32] = {0};
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	pauseTime = get16((uint8 *)value);
	//api
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	oldPauseTime = get16(attributeValuePtr);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setPPTPEthernetUNIPauseTime->oldPauseTime = %d, pauseTime = %d, [%d]",oldPauseTime, pauseTime, __LINE__);	

	if(oldPauseTime != pauseTime)
	{
		put16(attributeValuePtr, pauseTime);	
		
#ifdef TCSUPPORT_MT7530_SWITCH_API		
		ret = macMT7530SetPauseTime(pauseTime, portId);
		if(ret == -1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIPauseTime->macMT7530SetPauseTime fail[%d]",__LINE__); 
		}
                
                /*need restart auto neg to let flow control function take effect*/
		ret = macMT7530PortAutoNegRestartAutoConfig(portId);
		if(ret == -1){
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\n setPPTPEthernetUNIPauseTime->macMT7530PortAutoNegRestartAutoConfig fail[%d]",__LINE__); 
		}
#endif

		/*for green drop function handle, if pauseTime=0,will restore the green drop function, else will disable green drop function*/
		sprintf(tempBuffer, "%x", pauseTime?1:0);
		tcapi_set("GPON_Queue","DropMode",tempBuffer);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_NO_MSG,"%s:pause=[%d],DropMode=%s!\n",__FUNCTION__, pauseTime, tempBuffer); 
		tcapi_commit("GPON_Queue");
	}
	
	return 0;
}

int32 setPPTPEthernetUNIARCValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_PPTP_ETHERNET_UNI);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[13];

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

int32 setPPTPEthernetUNIARCIntervalValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 setPPTPEthernetUNIPPPoEFilter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
		*attributeValuePtr = tmpValue;
	}
	else
	{
		return -1;
	}
	
	return 0;
}

/*******************************************************************************************************************************
					9.5.2 Ethernet performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForEthernetPMHistoryData(omciManageEntity_t *omciManageEntity_p)
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

int32 setEthernetPMHistoryDataThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	uint16 length = 0;
	uint8 *attributeValuePtr = NULL;
	int i = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);
	
	//get Threshold data instance
	thresholdMeId = get16((uint8 *)value);

	if((thresholdMeId == 0) || (thresholdMeId == 0xffff))
		return 0;

	for(i=1; i<6; i++)
	{
		//get TCA value from Threshold ME
		ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, i, &thresholdData);
		if (ret != 0)
			return -1;
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%x", thresholdData);
		//set TCA to PMGR PM attribute
		ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, i, tempBuffer);
		if (ret != 0)
			return -1;
	}
	
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 7, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 6, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA2,thresholdMeId, 8, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 7, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA2,thresholdMeId, 10, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 8, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA2,thresholdMeId, 13, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 9, tempBuffer);
	if (ret != 0)
		return -1;
	
	return 0;	
}

int32 getEthernetPMHistoryDataFCSErrors(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 1, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryDataExcessiveCollisionCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 2, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryDataLateCollisionCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryDataFramesTooLong(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryDataBufOverflowOnRecv(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*always return 0*/
int32 getEthernetPMHistoryDataBufOverflowOnTransmit(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tempBuffer[8] = {0};

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int32 getEthernetPMHistoryDataSingleCollisionFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 6, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryDataMultiCollisionFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 7, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*always return 0*/
int32 getEthernetPMHistoryDataSqeCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tempBuffer[8] = {0};

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int32 getEthernetPMHistoryDataDeferredTransCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 8, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*always return 0*/
int32 getEthernetPMHistoryDataInternalMACTransErrorCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tempBuffer[8] = {0};

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

/*always return 0*/
int32 getEthernetPMHistoryDataCarrierSenseErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tempBuffer[8] = {0};

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}

int32 getEthernetPMHistoryDataAlignErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 9, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*always return 0*/
int32 getEthernetPMHistoryDataInternalMACRecvErrCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tempBuffer[8] = {0};

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	memcpy(attributeValuePtr, tempBuffer, length);
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);
}


/*******************************************************************************************************************************
				9.5.3 Ethernet performance monitoring history data 2

********************************************************************************************************************************/
int omciMeInitForEthernetPMHistoryData2(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForEthernetPMHistoryData 2\r\n");

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

int32 setEthernetPMHistoryData2ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	
	if((thresholdMeId == 0) || (thresholdMeId == 0xffff))
		return 0;
	
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 1, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 1, tempBuffer);
	if (ret != 0)
		return -1;

	return 0;	
}

int32 getEthernetPMHistoryData2PPPoEFilteredFrameCnt(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
			9.5.4 Ethernet performance monitoring history data 3

********************************************************************************************************************************/
int omciMeInitForEthernetPMHistoryData3(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForEthernetPMHistoryData3 \r\n");

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

int32 setEthernetPMHistoryData3ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	
	if((thresholdMeId == 0) || (thresholdMeId == 0xffff))
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
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 6, tempBuffer);
	if (ret != 0)
		return -1;
	
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 3, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 7, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 4, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 8, tempBuffer);
	if (ret != 0)
		return -1;
	
	return 0;	
}

int32 getEthernetPMHistoryData3DropEvents(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3Packets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3BroadcastPackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3MulticastPackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3UndersizePackets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3Fragments(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 7, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Jabbers(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetPMHistoryData3Packets64Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 9, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Packets65To127Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 10, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Packets128To255Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 11, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Packets256To511Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 12, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Packets512To1023Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 13, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

int32 getEthernetPMHistoryData3Packets1024To1518Octets(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint32 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 14, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%x", &pmData);
	put32((uint8 *)value, pmData);

	return 0;
}

/*******************************************************************************************************************************
					9.5.5 Virtual Ethernet interface point

********************************************************************************************************************************/
int omciMeInitForVirtualEthernetInterfacePoint(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForVirtualEthernetInterfacePoint \r\n");

	if(omciManageEntity_p == NULL)
		return -1;
	
	tmpOmciManageEntity_p = omciManageEntity_p;
	/* init some param for this ME */
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	
	return 0;
}

int omciInternalCreatActionForVirtualEthernetInterfacePoint(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint8 buffer[32] = {0};
	uint16 attriMask = 0;

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	if(tmpOmciManageEntity_p == NULL)
		return -1;
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	if(tmpomciMeInst_p == NULL)
		return -1;
	tmpomciMeInst_p->deviceId = instanceId;//for receive message
	//omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n deviceID %d for instanceId %d \r\n",tmpomciMeInst_p->deviceId, instanceId);

	/*meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else 
		goto fail;
	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else 
		goto fail;

	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 25);
	else 
		goto fail;	

	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	memset(buffer, 0, sizeof(buffer));
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, buffer, 2);
	else 
		goto fail;	

	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr , 0xFFFF);
	else 
		goto fail;

	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;	

}

int omciInitInstForVirtualEthernetInterfacePoint(void)
{
	int ret = 0;
	uint16 insID = 0;

	insID = ((VEIP_SLOT<<8) | VEIP_INST_ID);
	omciInitInstForUniG(insID);
	ret = omciInternalCreatActionForVirtualEthernetInterfacePoint(OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT,  insID);  //internal create func	
	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT fail\n");

	return ret;
}

int32 setVirtualEthernetInterfacePointAdminStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 portId = 0;
	uint16 length = 0;
	notify_msg_t msg;
	uint8 oldValue = 0;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr)-1;
	
	tmpValue = *((uint8*)value);
	if(tmpValue == 0x0){//enable function
		//add api
	}
	else if(tmpValue == 0x1){//disable function
		//add api
	}
	else
	{
		return -1;
	}

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	oldValue = (*attributeValuePtr);
	*attributeValuePtr = tmpValue;
	
	if(tmpValue != oldValue){
		/*deal with op state AVC, if admin state changes, we should send op state AVC*/
		memset(&msg, 0, sizeof(msg));
		msg.notifyType = OMCI_NOTIFY_TYPE_AVC;
		msg.avcId = OMCI_AVC_ID_VEIP_OPERATIONAL_STATE;
		msg.deviceId = portId+1;
		if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\navcMsgHandle: sendOmciNotifyMsg fail!\r\n");

		if(tmpValue == 0x1){//disable function
			/*deal with connection function fail ALARM*/
			memset(&msg, 0, sizeof(msg));
			msg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
			msg.alarmId = OMCI_ALARM_ID_VEIP_CONNECTING_FUNCTION_FAIL;
			msg.alarmFlag = OMCI_ALARM_ERR;
			msg.deviceId = portId+1;
			if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\navcMsgHandle: sendOmciNotifyMsg fail!\r\n");
		}
		else//enable function
		{
			/*deal with connection function fail ALARM*/
			memset(&msg, 0, sizeof(msg));
			msg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
			msg.alarmId = OMCI_ALARM_ID_VEIP_CONNECTING_FUNCTION_FAIL;
			msg.alarmFlag = OMCI_ALARM_RECOVERY;
			msg.deviceId = portId+1;
			if (sendOmciNotifyMsg((notify_msg_t *)&msg) != 0)
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\navcMsgHandle: sendOmciNotifyMsg fail!\r\n");
		}
	}
	
	return 0;
}

int32 getVirtualEthernetInterfacePointOpStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 instanceId = 0;
	uint16 length = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciAttriDescript_ptr currentAttribute = NULL;
	uint8 tmpValue = 0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	instanceId = get16(tmpomciMeInst_ptr->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_VIRTUAL_ETHERNET_INTERFACE_POINT);
	if(currentME == NULL)
		return -1;
	meInstantCurPtr = omciGetInstanceByMeId(currentME , instanceId);
	if(meInstantCurPtr == NULL)
		return -1;
	currentAttribute = &currentME->omciAttriDescriptList[1];
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , currentAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	tmpValue = *attributeValuePtr;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	if(tmpValue == 0)
	{
		*attributeValuePtr = 0x0;
	}
	else if(tmpValue == 1)
	{
		*attributeValuePtr = 0x1;
	}
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);	
}

int32 getVirtualEthernetInterfacePointIANAAssignedPortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	uint16 length = 0;
	char tmpValue[8] = {0};
	uint16 tmpPort = 0x0;

	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;	
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_ptr , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	if(tcapi_get("Cwmp_Entry", "conReqPort", tmpValue) != 0)
		put16(attributeValuePtr , 0xFFFF);
	else
	{
		tmpPort = (uint16)atoi(tmpValue);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n tmpPort:%X \r\n", tmpPort);
		put16(attributeValuePtr , tmpPort);
	}
	
	return getTheValue(value, (char *)attributeValuePtr, length, omciAttribute);	
}

#ifdef TCSUPPORT_OMCI_ALCATEL
int omciMEInitForClockDataSet(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMEInitForClockDataSet \r\n");

    if(omciManageEntity_p == NULL)
        return -1;
    
    tmpOmciManageEntity_p = omciManageEntity_p;
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT]   = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET]     = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET]     = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE]  = omciDeleteAction;    
    return 0;
}

int32 getclockDataSetDomainValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 setclockDataSetDomainValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 getclockDataSetClockClassValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 setclockDataSetClockClassValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 getclockDataSetClockAccuracyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 setclockDataSetClockAccuracyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 getclockDataSetClockPrior1Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 setclockDataSetClockPrior1Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 getclockDataSetClockPrior2Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}
int32 setclockDataSetClockPrior2Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag) {
    return 0;
}

int omciMEInitForPTPMasterConfigDataProfile(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMEInitForPTPMasterConfigDataProfile \r\n");

    if(omciManageEntity_p == NULL)
        return -1;
    
    tmpOmciManageEntity_p = omciManageEntity_p;
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT]   = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET]     = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET]     = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE]  = omciDeleteAction;    
    return 0;
}

int omciMEInitForPTPPort(omciManageEntity_t *omciManageEntity_p){
    omciManageEntity_t *tmpOmciManageEntity_p = NULL;
    
    omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\nenter omciMEInitForPTPPort\n");

    if(omciManageEntity_p == NULL)
        return -1;
    
    tmpOmciManageEntity_p = omciManageEntity_p;
    /* init some param for this ME */
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT]   = omciCreateAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET]     = omciGetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET]     = omciSetAction;
    tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE]  = omciDeleteAction;    
    return 0;
}

#endif

