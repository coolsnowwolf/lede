
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
	9_12_ME_func.c
	
	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	kenny.weng		2012/8/07	Create
	Andy.Yi		        2012/8/07	add 9.2.1,9.2.2
*/


#include "omci_types.h"
#include "omci_me.h"
#include "omci_general_func.h"
#include "omci_me_ani_management.h"

#ifdef TCSUPPORT_MT7530_SWITCH_API
#include "../ethcmd/libmii.h"
#include "../lib/mtkswitch_api.h"
#endif
#include "libcompileoption.h"
#include "../lib/libapi_lib_ppe.h"

enum {
    UPSTREAM_GEMPORT,
    DOWNSTREAM_GEMPORT,
};
#ifdef TCSUPPORT_CPU_EN7521
    #define UP_GEMPORT_TO_METER_START	    10
    #define UP_GEMPORT_TO_METER_NUM		    8
    #define DOWN_GEMPORT_TO_METER_START	    (UP_GEMPORT_TO_METER_START+UP_GEMPORT_TO_METER_NUM)
    #define DOWN_GEMPORT_TO_METER_NUM		8
    #if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
        #define TRTCM_CHAN_NUM_FOR_UP_STRM_POLICER 8
        #define TRTCM_CHAN_NUM_FOR_GEMPORT UP_GEMPORT_TO_METER_NUM
        #define CONST_SHAPING_NUMBER (UP_GEMPORT_TO_METER_NUM + TRTCM_CHAN_NUM_FOR_UP_STRM_POLICER)
        #define MAKE_VLAN_RX_POLICER_ID(instID, entryID) \
        ((0x08 & (instID) ) | ((entryID) << 8 ) )
        #define INVALID_POLICERID 0xFFFF
    #else
        #define TRTCM_CHAN_NUM_FOR_GEMPORT UP_GEMPORT_TO_METER_NUM
        #define CONST_SHAPING_NUMBER TRTCM_CHAN_NUM_FOR_GEMPORT
    #endif
#else
    #define CONST_SHAPING_NUMBER 32
    #if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
    #define TRTCM_CHAN_NUM_FOR_GEMPORT 24
    #define TRTCM_CHAN_NUM_FOR_UP_STRM_POLICER (CONST_SHAPING_NUMBER - TRTCM_CHAN_NUM_FOR_GEMPORT)
    #define MAKE_VLAN_RX_POLICER_ID(instID, entryID) \
        ((0x08 & (instID) ) | ((entryID) << 8 ) )
    #define INVALID_POLICERID 0xFFFF
    #else
    #define TRTCM_CHAN_NUM_FOR_GEMPORT CONST_SHAPING_NUMBER
    #endif
#endif

#ifdef TCSUPPORT_CPU_EN7521
#define CONST_DS_TRTCM_NUMBER DOWN_GEMPORT_TO_METER_NUM
#else
#define CONST_DS_TRTCM_NUMBER 32
#endif

static uint16 trtcmChannel[CONST_SHAPING_NUMBER];
uint16 dsTrtcmTable[CONST_DS_TRTCM_NUMBER] = {0};
static uint8 dsTrtcmEnableFlag = 0;


#ifdef TCSUPPORT_PON_ROSTELECOM
#define INVALID_TRTCM_CHAN   0xFF
#endif

#define INVALID_GEMPORTID 0xFFFF
#define GET_ETH_NO_FROM_PPTP_ETH_UNI_INST(inst) (isFiberhome_551601 ? (inst) : ( (inst) & 0xFF ))

static void
InitGponMappingTrtcm(void)
{
        memset(trtcmChannel, 0xff, sizeof(trtcmChannel[0]) * TRTCM_CHAN_NUM_FOR_GEMPORT);
}

static inline 
int8 getTrtcChan4Gemport(uint16 gemPortId, uint16 direction)
{
    uint8 channelId = 0;
    uint16 *trtcmTable = NULL;
    uint16 tableLen = 0;

    if(direction == UPSTREAM_GEMPORT){
        trtcmTable = trtcmChannel;
        tableLen = TRTCM_CHAN_NUM_FOR_GEMPORT;
    }
    else if(direction == DOWNSTREAM_GEMPORT){
        trtcmTable = dsTrtcmTable;
        tableLen = CONST_DS_TRTCM_NUMBER;
    }
    else
        return -1;

    /* search the id using channel id */
    for (channelId = 0; channelId < tableLen; channelId++)
    {
            if (gemPortId == trtcmTable[channelId])
                    return channelId;
    }

    /* if the id is not using channel, then select an avaible channel for Trtcm*/
    for (channelId = 0; channelId < tableLen; channelId++)
    {
            if (INVALID_GEMPORTID == trtcmTable[channelId])
                    break;
    }

    return channelId;

}


static int
setTrtcmUsedByGemportMapping(uint16 gemPortId, uint8 channel, uint16 direction)
{

    uint16 *trtcmTable = NULL;
    uint16 tableLen = 0;

    if(direction == UPSTREAM_GEMPORT){
        trtcmTable = trtcmChannel;
        tableLen = TRTCM_CHAN_NUM_FOR_GEMPORT;
    }
    else if(direction == DOWNSTREAM_GEMPORT){
        trtcmTable = dsTrtcmTable;
        tableLen = CONST_DS_TRTCM_NUMBER;
    }
    else
        return -1;

    if ((channel >= tableLen) || \
        (INVALID_GEMPORTID == gemPortId)){
            return -1;
    }
    
    trtcmTable[channel] = gemPortId;
    return 0;
}


static int
setTrtcmAva4GemportMapping(uint16 gemPortId)
{
        uint8 channelId = 0;

        if (INVALID_GEMPORTID == gemPortId)
                return -1;
        
        for (channelId = 0; channelId < TRTCM_CHAN_NUM_FOR_GEMPORT; channelId++)
                if (trtcmChannel[channelId] == gemPortId)
                        trtcmChannel[channelId] = INVALID_GEMPORTID;
        return 0;
}

#if defined (TCSUPPORT_OMCI_ALCATEL) && defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
void
InitUpStrmPolicerTrtcm(void)
{
        memset(trtcmChannel + TRTCM_CHAN_NUM_FOR_GEMPORT, 0xff, \
               sizeof(trtcmChannel[0]) * TRTCM_CHAN_NUM_FOR_UP_STRM_POLICER);   
}

static inline
uint8 getTrtcmChan4UpStrmPolicer(uint16 PolicerID)
{
        uint8 channelId = TRTCM_CHAN_NUM_FOR_GEMPORT;

        /* search the id using channel id */
        for (; channelId < CONST_SHAPING_NUMBER; channelId++)
        {
                if (PolicerID == trtcmChannel[channelId])
                        return channelId;
        }

        /* if the id is not using channel, then select an avaible channel for Trtcm*/
        for (channelId = TRTCM_CHAN_NUM_FOR_GEMPORT; channelId < CONST_SHAPING_NUMBER; channelId++)
        {
                if (INVALID_POLICERID == trtcmChannel[channelId])
                        break;
        }

        return channelId;
}


static int
setTrtcmChanUsedByUpStrmPolicer(uint16 policerID, uint8 chanID)
{
        if ( chanID <  TRTCM_CHAN_NUM_FOR_GEMPORT || \
             chanID >= CONST_SHAPING_NUMBER       || \
             INVALID_POLICERID == policerID){
                return -1;
        }

        trtcmChannel[chanID] = policerID;
        return 0;
}

static int
setTrtcmAva4UpStrmPolicer(uint16 policerID)
{
        uint8 channelId = TRTCM_CHAN_NUM_FOR_GEMPORT;

        if (INVALID_POLICERID== policerID){
                return -1;
        }
        
        for (; channelId < CONST_SHAPING_NUMBER; ++ channelId){
                if (trtcmChannel[channelId] == policerID){
                        trtcmChannel[channelId] = INVALID_POLICERID;
                        break;
                }
        }
        if ( CONST_SHAPING_NUMBER == channelId ){
                return -1;
        }
        
        return 0;
}

#endif

typedef struct TrtcmParam_s{
        uint32 CIR;
        uint32 PIR;
        uint32 CBS;
        uint32 PBS;
        uint8  chanID;
}TrtcmParam, * TrtcmParam_p;

static int setTrtcmParam(TrtcmParam_p pTrtcm)
{
        char buffer[MAX_BUFFER_SIZE] = {0};
        
/* local setting */
        if ((pTrtcm->CIR == 0) || (pTrtcm->CIR > 8000*16000)){
                pTrtcm->CIR = 8000*16000;
        }

        if ((pTrtcm->PIR == 0) || (pTrtcm->PIR > 8000*16000)){
                pTrtcm->PIR = 8000*16000;
        }

/* local setting */
        if ((pTrtcm->CBS == 0) ||(pTrtcm->CBS > 65535)){
                pTrtcm->CBS = 65535;
        }

/* local setting */
        if ((pTrtcm->PBS == 0) || (pTrtcm->PBS > 65535)){
                pTrtcm->PBS = 65535;
        }
        
        memset(buffer, 0,  sizeof(buffer));
        sprintf(buffer, "%d", pTrtcm->chanID);
        if(0 != tcapi_set("GPON_Trtcm", "channel", buffer))
                return -1;

        memset(buffer, 0,  sizeof(buffer));
        sprintf(buffer, "%d", pTrtcm->CIR);
        if(0 != tcapi_set("GPON_Trtcm", "cir", buffer))
                return -1;

        memset(buffer, 0,  sizeof(buffer));
        sprintf(buffer, "%d", pTrtcm->PIR);
        if(0 != tcapi_set("GPON_Trtcm", "pir", buffer))
                return -1;

        memset(buffer, 0,  sizeof(buffer));
        sprintf(buffer, "%d", pTrtcm->CBS);
        if(0 != tcapi_set("GPON_Trtcm", "cbs", buffer))
                return -1;

        memset(buffer, 0,  sizeof(buffer));
        sprintf(buffer, "%d", pTrtcm->PBS);
        if(0 != tcapi_set("GPON_Trtcm", "pbs", buffer))
                return -1;

        if(0 != tcapi_commit("GPON_Trtcm"))
                return -1;

        return 0;
}


int omciInternalCreateForAniG(uint16 classId, uint16 instanceId);
int omciInternalCreateForTcont(uint16 meClassId, uint16 instanceId);

/*******************************************************************************************************************************
9.2.1 ANI-G

********************************************************************************************************************************/
int omciMeInitForAniG(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_TEST] = omciTestAction;

	return 0;
}

int omciInternalCreateForAniG(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attrMask = 0;
	int tmp = 0;
	char buffer[25] = {0};	
	
	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	tmpomciMeInst_p->deviceId = 0;
/*0---meId*/
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;

/*1---SR Indication*/
	attrMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = pAniCap->srIndication;
	}
	else
		goto fail;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForAniG: tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName = %s, length = %x, tmp = %d\n",  tmpOmciManageEntity_p->omciAttriDescriptList[1].attriName, length, tmp);

/*2---Total TCONT Number*/
	attrMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		put16(attributeValuePtr, pAniCap->totalTcontNum);
	}
	else
		goto fail;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForAniG: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, tmp = %d\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, tmp);

/*3---GEM block length=48*/
	attrMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		put16(attributeValuePtr, 48);
	}
	else
		goto fail;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForAniG: tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName = %s, length = %x, tmp = %d\n",  tmpOmciManageEntity_p->omciAttriDescriptList[2].attriName, length, tmp);
	
/*4---Piggyback DBA*/
	attrMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = pAniCap->piggybackDBA;
	}
	else
		goto fail;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciInternalCreateForAniG: tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName = %s, length = %x, tmp = %d\n",  tmpOmciManageEntity_p->omciAttriDescriptList[4].attriName, length, tmp);


/*5---Deprecated=0*/
	attrMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 0;
	else
		goto fail;
/*6---SF threshold*/	
	attrMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 5;
	}
	else
		goto fail;
	
/*7---SD threshold*/	
	attrMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 9;
	}
	else
		goto fail;

/*10---Optical signal level*/
	memset(buffer, 0, sizeof(buffer));
	getOpticalSignalLevelValue(buffer, tmpomciMeInst_p, &tmpOmciManageEntity_p->omciAttriDescriptList[10], 0);

/*11---Lower optical threshold*/
	attrMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 0xFF;
	}
	else
		goto fail;	

/*12---Upper optical threshold*/
	attrMask = 1<<4;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 0xFF;
	}
	else
		goto fail;

/*13---ONU Response Time*/
	attrMask = 1<<3;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		put16(attributeValuePtr, pAniCap->onuResponseTime);
	}
	else
		goto fail;
	
/*14---Transmit optical level*/
	memset(buffer, 0, sizeof(buffer));
	getTransmitOpticalLevelValue(buffer, tmpomciMeInst_p, &tmpOmciManageEntity_p->omciAttriDescriptList[14], 0);
	
/*15---Lower transmit power threshold*/
	attrMask = 1<<1;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 0x81;
	}
	else
		goto fail;	

/*16---Upper transmit power threshold*/
	attrMask = 1<<0;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{	
		*attributeValuePtr = 0x81;
	}
	else
		goto fail;	
	
	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;
}

int omciInitInstForAniG(void)
{
	int ret = 0;

	if(isFiberhome_551601){
		ret = omciInternalCreateForAniG(OMCI_CLASS_ID_ANI_G,  0);   /*internal create func*/
	} 
	else{
		ret = omciInternalCreateForAniG(OMCI_CLASS_ID_ANI_G,  ((GPON_SLOT<<8) + 1));   /*internal create func*/
	}

	if (ret == -1)
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_ANI_G fail\n");

	return ret;
}

int omciPhyAlarmHandler(uint32 trapValue)
{
	int ret = 0;
	notify_msg_t notifyMsg;
	uint8 *attributeValuePtr = NULL;
	uint8 internelPolicyValue = 0;
	omciManageEntity_t * me_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint16 intanceId = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciPhyAlarmHandler: trapValue = 0x%x\n", trapValue);

	me_p = omciGetMeByClassId(OMCI_CLASS_ID_ANI_G);
	if (me_p == NULL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "me_p == NULL\n");
		return NULL;
	}

	tmpomciMeInst_p = me_p->omciInst.omciMeInstList;

	if (tmpomciMeInst_p == NULL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "tmpomciMeInst_p == NULL\n");
		return NULL;
	}	

	intanceId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	/*
		XMCS_PHY_ALARM_TX_POWER_HIGH	= (1<<0),
		XMCS_PHY_ALARM_TX_POWER_LOW		= (1<<1),
		XMCS_PHY_ALARM_TX_CURRENT_HIGH	= (1<<2),
		XMCS_PHY_ALARM_TX_CURRENT_LOW	= (1<<3),
		XMCS_PHY_ALARM_RX_POWER_HIGH	= (1<<4),
		XMCS_PHY_ALARM_RX_POWER_LOW		= (1<<5),
	*/
	if (trapValue & (1<<0)) 
	{	
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_ANI_G, intanceId, 16);
		if (attributeValuePtr != NULL)
		{
			internelPolicyValue = *attributeValuePtr;
			if (internelPolicyValue == 0x81)
			{
				ret = 1;				
				goto retHandle;
			}
		}
		else
		{
			ret = -1;				
			goto retHandle;
		}
		
		memset((uint8*)&notifyMsg, 0, sizeof(notify_msg_t));
		notifyMsg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
		notifyMsg.deviceId = 0;
		notifyMsg.alarmFlag = OMCI_ALARM_ERR;
		notifyMsg.alarmId = OMCI_ALARM_ID_HIGH_TX_OPTICAL;
		ret = omciAlarmMsgHandle(&notifyMsg);
			
		if(ret < 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciAlarmMsgHandle fail!");
		}
	}

	if (trapValue & (1<<1))		
	{
	
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_ANI_G, intanceId, 15);
		if (attributeValuePtr != NULL)
		{
			internelPolicyValue = *attributeValuePtr;
			if (internelPolicyValue == 0x81)
			{
				ret = 1;				
				goto retHandle;
			}
		}
		else
		{
			ret = -1;				
			goto retHandle;
		}
		memset((uint8*)&notifyMsg, 0, sizeof(notify_msg_t));
		notifyMsg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
		notifyMsg.deviceId = 0;
		notifyMsg.alarmFlag = OMCI_ALARM_ERR;
		notifyMsg.alarmId = OMCI_ALARM_ID_LOW_TX_OPTICAL;
		ret = omciAlarmMsgHandle(&notifyMsg);
		if(ret < 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciAlarmMsgHandle fail!");
		}
	}				

	if (trapValue & (1<<2))
	{
		notifyMsg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
		notifyMsg.deviceId = 0;
		notifyMsg.alarmFlag = OMCI_ALARM_ERR;
		notifyMsg.alarmId = OMCI_ALARM_ID_LASER_BIAS_CURRENT;
		ret = omciAlarmMsgHandle(&notifyMsg);
		if(ret < 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciAlarmMsgHandle fail!");
		}
	}			

	if (trapValue & (1<<4))
	{
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_ANI_G, intanceId, 12);
		if (attributeValuePtr != NULL)
		{
			internelPolicyValue = *attributeValuePtr;
			if (internelPolicyValue == 0xFF)
			{
				ret = 1;				
				goto retHandle;
			}
		}
		else
		{
			ret = -1;				
			goto retHandle;
		}
		memset((uint8*)&notifyMsg, 0, sizeof(notify_msg_t));
		notifyMsg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
		notifyMsg.deviceId = 0;
		notifyMsg.alarmFlag = OMCI_ALARM_ERR;
		notifyMsg.alarmId = OMCI_ALARM_ID_HIGH_RX_OPTICAL;
		ret = omciAlarmMsgHandle(&notifyMsg);
		if(ret < 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciAlarmMsgHandle fail!");
		}
	}	

	if (trapValue & (1<<5))
	{
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_ANI_G, intanceId, 11);
		if (attributeValuePtr != NULL)
		{
			internelPolicyValue = *attributeValuePtr;
			if (internelPolicyValue == 0xFF)
			{
				ret = 1;				
				goto retHandle;
			}
		}
		else
		{
			ret = -1;				
			goto retHandle;
		}
		memset((uint8*)&notifyMsg, 0, sizeof(notify_msg_t));
		notifyMsg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
		notifyMsg.deviceId = 0;
		notifyMsg.alarmFlag = OMCI_ALARM_ERR;
		notifyMsg.alarmId = OMCI_ALARM_ID_LOW_RX_OPTICAL;
		ret = omciAlarmMsgHandle(&notifyMsg);
		if(ret < 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"\r\nomciAlarmMsgHandle fail!");
		}
	}	

retHandle:
	return ret;
}

int32 setGEMBlockLengthValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", get16((uint8*)value));
	tcApiSetAttriubte("GPON_ANI", "GEMBlockLength", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}
int32 setSFThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", (uint8)*value);
	tcApiSetAttriubte("GPON_ANI", "SFThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}
int32 setSDThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", (uint8)*value);	
	tcApiSetAttriubte("GPON_ANI", "SDThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}
int32 setANIGARCValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint8 ARCEnable = 0;
	uint8 ARCInterval = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	ARCEnable = *value;
	if (ARCEnable == 0)
	{
		omciArcDisable(tmpomciMeInst_p);
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, (uint8*)omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			*attributeValuePtr = ARCEnable;
	}
	else if (ARCEnable == 1)
	{
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, (uint8*)"ARC interval", &length);
		if (attributeValuePtr != NULL)
			ARCInterval = *attributeValuePtr;
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "setANIGARCValue: ARCInterval = %d\n", ARCInterval);
		omciArcDisable(tmpomciMeInst_p);
		omciArcEnable(tmpomciMeInst_p, ARCInterval);
		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			*attributeValuePtr = ARCEnable;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setANIGARCValue: ARCEnable unknown value\n");
		return -1;
	}

	return 0;
}
int32 setANIGARCIntervalValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint16 length = 0;
	uint8 ARCEnable = 0;
	uint8 ARCInterval = 0;
	uint8 *attributeValuePtr = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	*attributeValuePtr = *value;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, (uint8*)"ARC", &length);
	if (attributeValuePtr != NULL)
		ARCEnable = *attributeValuePtr;
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setANIGARCIntervalValue: attributeValuePtr != NULL\n");
		return -1;
	}

	ARCInterval = *value; 
	if (ARCEnable == 0)
	{
		omciArcDisable(tmpomciMeInst_p);
	}
	else if (ARCEnable == 1)
	{
		omciArcDisable(tmpomciMeInst_p);
		omciArcEnable(tmpomciMeInst_p, ARCInterval);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setANIGARCIntervalValue: ARCEnable unknown value\n");
		return -1;
	}

	return 0;
}
#if 0
float logFunction(float x, int base, int decimalplaces)
{
	int integer = 0;
	float return_value = 0;
	float decimalplaces_count = 0.1;
	while(x < 1)
	{
		integer = integer -1;
		x = x*base;
	}
	while(x >= base)
	{
		integer = integer + 1;
		x = x/base;
	}
	return_value = integer;
	while(decimalplaces > 0)
	{
		x = x*x*x*x*x*x*x*x*x*x;
		int digit = 0;
		while (x >= base)
		{
			digit = digit + 1;
			x = x/base;
		}

		return_value = return_value + (digit * decimalplaces_count);
		decimalplaces = decimalplaces - 1;
		decimalplaces_count = decimalplaces_count * 0.1;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_NO_MSG, "log_value : %f\n", return_value);
	return return_value;
}
		
short transferOpticalPower(float power_mw)
{
	short return_value = 0;
	float log_value = 0;
	log_value = 10*logFunction(power_mw, 10, 4);
	if (log_value < 0)
	{
		return_value = return_value | 0x8000;
		log_value = log_value + 0x8000;
	}
	return_value = return_value | ((int) (log_value / 0.002));
	omcidbgPrintf(OMCI_DEBUG_LEVEL_NO_MSG, "power_mw : %f, dB_value :0x%.8x\n", power_mw, return_value);
	return return_value;
}
#endif

float logFunction2(float x, int base, int decimalplaces)
{
	int integer = 0;
	float return_value = 0;
	float decimalplaces_count = 0.1;
	if (x <= 0.0)
		return -40; //assume it is x = 1 when x is 0.
	
	while(x < 1)
	{
		integer = integer -1;
		x = x*base;
	}
	while(x >= base)
	{
		integer = integer + 1;
		x = x/base;
	}
	return_value = integer;
	while(decimalplaces > 0)
	{
		x = x*x*x*x*x*x*x*x*x*x;
		int digit = 0;
		while (x >= base)
		{
			digit = digit + 1;
			x = x/base;
		}

		return_value = return_value + (digit * decimalplaces_count);
		decimalplaces = decimalplaces - 1;
		decimalplaces_count = decimalplaces_count * 0.1;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "log_value : %f\n", return_value);
	return return_value;
}

int32 getOpticalSignalLevelValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};
	float power_mw = 0.0;
	float power_uw = 0.0;
	short rxPower = 0;
	float log_value = 0.0;
	

	
	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);

	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		power_uw = phyPara[4] * 0.1;
		log_value = 10*(logFunction2(power_uw, 10, 4) - 3);
		rxPower = (short)(log_value * 500);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "phyPara[4]: %x, power_uw : %f, log_value:%f, rxPower :0x%x\n",phyPara[4], power_uw, log_value, rxPower);
		put16(attributeValuePtr, rxPower);
	}
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}
int32 setLowerOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", *value);		
	tcApiSetAttriubte("GPON_ANI", "LowerRxOpticalThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}

int32 setUpperOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", *value);			
	tcApiSetAttriubte("GPON_ANI", "UpperRxOpticalThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}
int32 getTransmitOpticalLevelValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[125] = {0};
	int phyPara[5] = {0};
	float power_uw = 0.0;
	int16_t txPower = 0;
	float log_value = 0.0;

	tcapi_get("GPON_ANI", "PhyTransParameters", tempBuffer);
	sscanf(tempBuffer, "Temperature:%x,Voltage:%x,TxCurrent:%x,TxPower:%x,RxPower:%x", &phyPara[0], &phyPara[1], &phyPara[2], &phyPara[3], &phyPara[4]);

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL){
		power_uw = phyPara[3] * 0.1;
		log_value = 10*(logFunction2(power_uw, 10, 4) - 3);
		txPower = (uint8)(log_value * 500);
		put16(attributeValuePtr, txPower);
	}
	else
		return -1;
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	
}

int32 setLowerTransmitOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", *value);			
	tcApiSetAttriubte("GPON_ANI", "LowerTxOpticalThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	
	return 0;
}
int32 setUpperTransmitOpticalThresholdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char buffer[25] = {0};
	
	sprintf(buffer, "%d", *value);			
	tcApiSetAttriubte("GPON_ANI", "UpperTxOpticalThd", buffer);
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
		return -1;
	return 0;
}

/*******************************************************************************************************************************
9.2.2 T-CONT

********************************************************************************************************************************/
int omciMeInitForTcont(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;

	return 0;
}

int omciInternalCreateForTcont(uint16 meClassId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p =NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attrMask = 0;

	/*It may be represented as 0xSSBB, where SS indicates the slot id that contains this T-CONT (0 for the ONU as a whole)*/
	tmpOmciManageEntity_p = omciGetMeByClassId(meClassId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);
	
/*0---meId*/	
	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;		
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);
	else
		goto fail;	

/*1---alloc Id*/
	/*this attribute has an unambiguously unusable initial value, namely the value 0x00FF or 0xFFFF for G.984 systems*/
	attrMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
	{
        if((CONFIG_GPON_MAX_TCONT == pAniCap->totalTcontNum) && (0x8000 == instanceId)){
            put16(attributeValuePtr, omci.onuId);
        }else{
            put16(attributeValuePtr, 0x00FF);
        }

	}
	else
		goto fail;	
	
/*2---Deprecated=1*/	
	attrMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = 1;
	else
		goto fail;	
	
/*3---Policy*/	
	attrMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attrMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr = TCONT_POLICY_SP;
	else
		goto fail;		
	
	return 0;

fail:
	omciFreeInstance(tmpomciMeInst_p);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
	return -1;
}

int omciInitInstForTcont(void)
{
	int ret = 0;
	int instanceId = 0;
	char tempBuffer[64] = {0};    

	for (instanceId = ((GPON_SLOT<<8) + 0); instanceId < ((GPON_SLOT<<8) + pAniCap->totalTcontNum); instanceId++)
	{
		ret = omciInternalCreateForTcont(OMCI_CLASS_ID_T_CONT,  instanceId);/*internal create func*/
		if (ret == -1)
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInitInst create OMCI_CLASS_ID_UNI_G fail\n");
			strcat(tempBuffer, "X_");
	}

	tcapi_set(GPON_OMCI_ENTRY, PON_QOS_TYPE, tempBuffer);
	tcapi_commit(GPON_OMCI_ENTRY);

	return ret;
}

int32 setTCONTAllocIdValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 meId = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	uint16 tcontPointerId = 0;
	int tcontCounts = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int result = 0;

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	currentME = omciGetMeByClassId(OMCI_CLASS_ID_GEM_PORT_CTP);
	for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
	{
		/*tcont pointer id*/
		tcontPointerId = get16(meInstantCurPtr->attributeVlaue_ptr + 4);
		if (tcontPointerId == meId)
		{
			result = omciAddQueueMappingRule(OMCI_CLASS_ID_GEM_PORT_CTP, meInstantCurPtr);
			if (result != 0)
				goto fail;
		}
	}

	return setGeneralValue(value, meInstantPtr, omciAttribute, flag);
fail:
	return -1;
}


int32 setTCONTPolicyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint16 QosConfFlex = 0;
	uint16 meId = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret=0;
	uint8 tcontId=0;
	omciManageEntity_ptr currentME = omciGetMeByClassId(OMCI_CLASS_ID_ONU2_G);
	omciMeInst_ptr meInstantCurPtr = currentME->omciInst.omciMeInstList;
	omciAttriDescript_ptr currentAttribute = &currentME->omciAttriDescriptList[13];

	attributeValuePtr = omciGetInstAttriByName(meInstantCurPtr , currentAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		QosConfFlex=get16(attributeValuePtr);
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
		tcdbg_printf("NULL\n");
		return -1;
	}	
	 /*get ONU2-G QoS configuration flexibility*/
	if(QosConfFlex & 0x0010) /*t-cont me: policy attribute is read-write*/
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
		sprintf(tempBuffer, "%x", meId);
		tcapi_set("GPON_TCONT", "Number", tempBuffer);
		memset(tempBuffer, 0, sizeof(tempBuffer));
		sprintf(tempBuffer, "%x", *value);
		tcApiSetAttriubte("GPON_TCONT", "Policy", tempBuffer);

		attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
			memcpy(attributeValuePtr, value, length);
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
			return -1;
		}	
		//upstream priority queue QOS Polcy Set
		tcontId = (uint8)(meId & 0x00ff);	
		ret=setQOSPolicy(tcontId,*value,1);
		if(ret != 0)
		    omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTcontPolicyValue: faile\n");
	}
	else
	{
		return OMCI_PARAM_ERROR;
	}
	return 0;
}
/*******************************************************************************************************************************
9.2.6 GEM port performance monitoring history data

********************************************************************************************************************************/
int omciMeInitForGemPortPM(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
		/* if need ONU auto create a ME instance, create here */

	return 0;
}

int32 getGemPortPmLostPacketsValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getGemPortPmMisinsertedPacketsValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getGemPortPmReceivedPacketsValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmReceivedPacketsValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmReceivedPacketsValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	return 0;
}

int32 getGemPortPmReceivedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmReceivedBlocksValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmReceivedBlocksValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	
	return 0;
}

int32 getGemPortPmTransmittedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmTransmittedBlocksValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortPmTransmittedBlocksValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	
	return 0;
}

int32 getGemPortPmImpairedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 setGemPortPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	int i= 0;

	
	if((value == NULL) || (meInstantPtr == NULL))
		return -1;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setGemPortPmThrdDataValue...\n");

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
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 2, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 3, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 6, tempBuffer);
	if (ret != 0)
		return -1;
	return 0;	
}


#ifdef TCSUPPORT_OMCI_ALCATEL
/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:   Total GEM Port PM

********************************************************************************************************************************/
int omciMeInitForTotalGemPortPM(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int32 getTotalGemPortPmLostCntDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getTotalGemPortPmLostCntUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getTotalGemPortPmReceivedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedCntValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedCntValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	return 0;
}

int32 getTotalGemPortPmReceivedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedBlocksValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedBlocksValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	return 0;
}

int32 getTotalGemPortPmTransmittedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 5, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedBlocksValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedBlocksValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	return 0;
}

int32 getTotalGemPortPmImpairedBlocksValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getTotalGemPortPmTransmittedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 7, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedCntValue...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 32;
	*((uint8 *)value+1) = pmData >> 24;
	*((uint8 *)value+2) = pmData >> 16;
	*((uint8 *)value+3) = pmData >> 8;
	*((uint8 *)value+4) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedCntValue...%x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4));
	return 0;
}

int32 getTotalGemPortPmBadReceivedCntValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getTotalGemPortPmReceivedBlocks64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 9, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedBlocks64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmReceivedBlocks64Value...%x %x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 getTotalGemPortPmTransmittedBlocks64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 10, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedBlocks64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getTotalGemPortPmTransmittedBlocks64Value...%x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 setTotalGemPortPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	int i= 0;
	
	if((value == NULL) || (meInstantPtr == NULL))
		return -1;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setTotalGemPortPmThrdDataValue...\n");

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
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 2, tempBuffer);
	if (ret != 0)
		return -1;

	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 3, &thresholdData);
	if (ret != 0)
		return -1;
	memset(tempBuffer, 0, sizeof(tempBuffer));
	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 6, tempBuffer);
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


/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:   Ethernet Traffic PM

********************************************************************************************************************************/
int omciMeInitForEthernetTrafficPM(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int32 getEthernetTrafficPmFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmBytesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmBytesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmDropFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmDropBytesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmDropFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmMulticastFramesUpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmMulticastFramesDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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

int32 getEthernetTrafficPmBytesUp64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 10, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmBytesUp64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmBytesUp64Value...%x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 getEthernetTrafficPmBytesDown64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 11, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmBytesDown64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmBytesDown64Value...%x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 getEthernetTrafficPmFramesUp64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 12, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmFramesUp64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmFramesUp64Value...%x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 getEthernetTrafficPmFramesDown64Value(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = NULL;
	int ret = 0;
	uint64 pmData = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, flag, 13, tempBuffer);
	if (ret != 0)
		return -1;
	
	sscanf(tempBuffer, "%llx", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmFramesDown64Value...%llx\n", pmData);
	*((uint8 *)value) = pmData >> 56;
	*((uint8 *)value+1) = pmData >> 48;
	*((uint8 *)value+2) = pmData >> 40;
	*((uint8 *)value+3) = pmData >> 32;
	*((uint8 *)value+4) = pmData >> 24;
	*((uint8 *)value+5) = pmData >> 16;
	*((uint8 *)value+6) = pmData >> 8;
	*((uint8 *)value+7) = pmData;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getEthernetTrafficPmFramesDown64Value...%x %x %x %x %x %x %x\n", 
		*((uint8 *)value), *((uint8 *)value+1), *((uint8 *)value+2), *((uint8 *)value+3),*((uint8 *)value+4),*((uint8 *)value+5),*((uint8 *)value+6),*((uint8 *)value+7));
	return 0;
}

int32 setEthernetTrafficPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;
	int i= 0;
	
	if((value == NULL) || (meInstantPtr == NULL))
		return -1;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setEthernetTrafficPmThrdDataValue...\n");

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
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 5, tempBuffer);
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
		
	return 0;	
}


/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:   Vlan Tag Downstream Egress Behavior Supplemental 1

********************************************************************************************************************************/
int omciMeInitForVlanTagDownstreamSupplemental1(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int32 setVlanTxDefaultTagValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	FILE *fp = NULL;
	uint8 op = 0;
	uint8 cmd[64] = {0};
	uint16 length = 0;
	uint16 portId = 0;
	uint8 *attributeValuePtr=NULL;
	omciMeInst_t *tmpomciMeInst_ptr = NULL;
	
	if((value == NULL) || (meInstantPtr == NULL) || (omciAttribute == NULL))
		return -1;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setVlanTxDefaultTagValue: attributeValuePtr == NULL\n");
		return -1;
	}
	
	tmpomciMeInst_ptr = (omciMeInst_ptr)meInstantPtr;
	portId = (get16(tmpomciMeInst_ptr->attributeVlaue_ptr) & 0xFF)-1;
	op = *((uint8*)value);

	// set default vlan mode
	fp = fopen("/proc/tc3162/eth_down_vlan", "w");
	if (!fp)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setVlanTxDefaultTagValue: fopen fail\n");
		return -1;
	}
	else
	{
		if ((op >= 1) && (op <= 3)) // 1: Remove TAG Mode; 2: TAG THRU Mode; 3: Priority TAG Mode
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
	
	return 0;	
}

int32 setVlanTxBehaviorTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	FILE *fp = NULL;
	int i=0, ret=0, curIndex=0, resv=0, vlanId=0, portId;
	uint8 cmd[64] = {0};
	uint16 tableItemCnt=0;
	uint16 classId=0;
	uint16 instanceId=0;
	uint16 actionType=0, tagAction=0;
	uint16 vlanTxTableSize=0;
	uint8 *tmpBuffer=NULL;
	uint8 entryNumber=0;
	omciMeInst_t *tmpomciMeInst_p = meInstantPtr;
	omciTableAttriValue_t *tmpTableValueList_p = NULL;
	vlanPolicerTxTable_t *tmpTableValue_p, *curTableValue_p = (vlanPolicerTxTable_t *)value;

	if((tmpomciMeInst_p == NULL) || (value == NULL) || (omciAttribute == NULL)){
		ret = -1;
		goto end;
	}
	
	vlanTxTableSize = sizeof(vlanPolicerTxTable_t);
	
	portId = get16(tmpomciMeInst_p->attributeVlaue_ptr)&0xFF-1;
	entryNumber = curTableValue_p->EntryNumber+1;
	actionType = (curTableValue_p->VlanField >> 7) & 0x1;
	tagAction = curTableValue_p->VlanField & 0x3;
	vlanId = curTableValue_p->vlanID;
	resv = curTableValue_p->Reserved;

	tmpTableValueList_p = tmpomciMeInst_p->tableValueList;

	while(tmpTableValueList_p != NULL) {
		if(tmpTableValueList_p->attriIndex == omciAttribute->attriIndex)
			break;
		
		tmpTableValueList_p = tmpTableValueList_p->next;
	}
	if(tmpTableValueList_p == NULL) {
		ret = -1;
		goto end;		
	}
	
	tableItemCnt = tmpTableValueList_p->tableSize / vlanTxTableSize;
	if(actionType == 0) {	// remove Entry
		if(tmpTableValueList_p->tableSize < vlanTxTableSize) {
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setVlanTxBehaviorTableValue: table is empty, can't remove\n");
			ret = -1;
			goto end;
		}
		
		for(i=0; i<tableItemCnt; i++) {
			tmpTableValue_p = (vlanPolicerTxTable_t *)(tmpTableValueList_p->tableValue_p + i*vlanTxTableSize);
			if(tmpTableValue_p->vlanID == curTableValue_p->vlanID)
				break;
		}
		if(i == tableItemCnt) { /* fail: the entry need to remove is not exist in the list */
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setVlanTxBehaviorTableValue: the entry need to remove is not exist\n");
			ret = -1;
			goto end;
		}
		curIndex = i;

		tmpTableValueList_p->tableSize -= vlanTxTableSize;
		tableItemCnt--;
		if(tableItemCnt != 0) {
			tmpBuffer = calloc(1 , tmpTableValueList_p->tableSize);
			memcpy(tmpBuffer, tmpTableValueList_p->tableValue_p, curIndex*vlanTxTableSize);
			memcpy(tmpBuffer+(curIndex*vlanTxTableSize), tmpTableValueList_p->tableValue_p+((curIndex+1)*vlanTxTableSize), (tableItemCnt-curIndex)*vlanTxTableSize);
		} else {
			tmpBuffer = NULL;
		}
		
		free(tmpTableValueList_p->tableValue_p);
		tmpTableValueList_p->tableValue_p = tmpBuffer;

		// remove one TX vlan rule
		fp = fopen("/proc/tc3162/eth_down_vlan", "w");
		if (!fp)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setVlanTxBehaviorTableValue: fopen fail\n");
			ret = -1;
			goto end;
		}
		else
		{
			fprintf(fp, "del_entry %d %d %d %d %d\n", portId, entryNumber, tagAction, vlanId, resv);
			fclose(fp);
		}
	} else if(actionType == 1) {	// add Entry
		if(tableItemCnt >= 8) {
			ret = -1;
			goto end;
		}
		
		for(i=0; i<tableItemCnt; i++) {
			tmpTableValue_p = (vlanPolicerTxTable_t *)(tmpTableValueList_p->tableValue_p + i*vlanTxTableSize);
			if(tmpTableValue_p->vlanID == curTableValue_p->vlanID)
				break;
		}
		
		if(i != tableItemCnt) {	// modify one existed rule
			memcpy(tmpTableValueList_p->tableValue_p+(i*vlanTxTableSize), value, vlanTxTableSize);
		} else {	// add one new rule
			tmpTableValueList_p->tableSize += vlanTxTableSize;
			tmpBuffer = calloc(1 , tmpTableValueList_p->tableSize);
			if(tableItemCnt != 0)
				memcpy(tmpBuffer, tmpTableValueList_p->tableValue_p, tableItemCnt*vlanTxTableSize);
			memcpy(tmpBuffer+(tableItemCnt*vlanTxTableSize), value, vlanTxTableSize);
			if(tableItemCnt != 0)
				free(tmpTableValueList_p->tableValue_p);
			tmpTableValueList_p->tableValue_p = tmpBuffer;
		}
	
		// add one TX vlan rule
		fp = fopen("/proc/tc3162/eth_down_vlan", "w");
		if (!fp)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "setVlanTxBehaviorTableValue: fopen fail\n");
			ret = -1;
			goto end;
		}
		else
		{
			if ((tagAction >= 1) && (tagAction <= 3)) // 1: Remove TAG Mode; 2: TAG THRU Mode; 3: Priority TAG Mode
			{
				fprintf(fp, "add_entry %d %d %d %d %d\n", portId, entryNumber, tagAction, vlanId, resv);
				fclose(fp);
			}
			else
			{
				fclose(fp);
				ret = -1;
				goto end;
			}
		}	
	}else {
		ret = -1;
		goto end;
	}
		
	ret = 0;
	
end:
	if(ret != 0){
		ret = -1;
	}

	return ret;
}


/*******************************************************************************************************************************
ALCATEL Vendor Specific ME:  Vlan Tag Upstream Policer

********************************************************************************************************************************/
#if defined (TCSUPPORT_UPSTREAM_VLAN_POLICER)
int omciMeInitForVlanTagUpstreamPolicer(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;
	
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int32 setVlanRxPolicerTableValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
        if( (meInstantPtr == NULL) || (value == NULL) || (omciAttribute == NULL)){
                return -1;
        }
         
        uint16 instanceId= 0;
        uint16 policerID = 0;
        uint8  chanID = 0;
        omciMeInst_t *pMEInst = meInstantPtr;
        omciTableAttriValue_p pTbl=pMEInst->tableValueList;
        vlanPolicerRxTable_ptr pRecords = NULL;
        vlanPolicerRxTable_ptr pNewRecord = (vlanPolicerRxTable_ptr)value;
        gpon_upstream_vlan_policer_ioctl ruleData;
        memset(&ruleData, 0 , sizeof(ruleData));
        int i=0;

        if(pNewRecord->entryID >= VLAN_POLICER_RX_TBL_MAX_ENTRY){
                OMCI_ERROR( "Max table entry for vlan upstream policer is: %d,"
                            "however we got: %d\n", VLAN_POLICER_RX_TBL_MAX_ENTRY,
                            (int)pNewRecord->entryID);
                return -1;
        }
        
        for(; pTbl != NULL; pTbl = pTbl->next) {
                if(pTbl->attriIndex == omciAttribute->attriIndex)
                        break;
        }

        if(NULL == pTbl) {
                OMCI_ERROR("Can't find the table with attr index: %d\n", 
                           (int)omciAttribute->attriIndex);
                return -1;              
        }

        pRecords = (vlanPolicerRxTable_ptr)pTbl->tableValue_p;

        if(NULL == pRecords){
                pRecords =osMalloc(VLAN_POLICER_RX_TBL_MAX_SIZE);
                
                if(NULL == pRecords){
                        OMCI_ERROR("osMalloc failed at %s:%s\n", __FILE__, __FUNCTION__);
                        return -1;
                }

                for(i = 0; i < VLAN_POLICER_RX_TBL_MAX_ENTRY; ++ i){
                        pRecords[i].entryID = 0xFF;  // invalid entry
                }
        
                pTbl->tableValue_p = pRecords;
        }
                
        instanceId = get16(pMEInst->attributeVlaue_ptr);
        policerID  = MAKE_VLAN_RX_POLICER_ID(instanceId, pNewRecord->entryID);

        if(VLAN_POLICY_ADD == pNewRecord->actionType){
                chanID = getTrtcmChan4UpStrmPolicer(policerID);
                
                if( CONST_SHAPING_NUMBER == chanID ){
                        OMCI_ERROR("No more hw resource for \"upstrm vlan tag policer\"!\n");
                        return -1;
                }

                setTrtcmChanUsedByUpStrmPolicer(policerID, chanID);
        
                TrtcmParam trtcm = {
                        .CIR = pNewRecord->CIR,
                        .PIR = pNewRecord->CIR,
                        .CBS = pNewRecord->CBS,
                        .PBS = pNewRecord->CBS,
                        .chanID = chanID
                };

                if ( 0 != setTrtcmParam(&trtcm) ){
                        return -1;
                }
                
                ruleData.ethID   = GET_ETH_NO_FROM_PPTP_ETH_UNI_INST(instanceId);
                ruleData.entryID = pNewRecord->entryID;
                ruleData.vid     = pNewRecord->vlanID;
                ruleData.chanID  = chanID;

                memcpy(pRecords + pNewRecord->entryID, pNewRecord, sizeof(vlanPolicerRxTable_t) );
                addUpstreamVlanPolicer(&ruleData);
        }else{                
                if (0 != setTrtcmAva4UpStrmPolicer(policerID)){
                        OMCI_ERROR("Can't del policer rule which does not exit!\n");
                        return -1;
                }
                                
                ruleData.ethID   = GET_ETH_NO_FROM_PPTP_ETH_UNI_INST(instanceId);
                ruleData.entryID = pNewRecord->entryID;
                ruleData.vid     = pNewRecord->vlanID;
                ruleData.chanID  = chanID;
        
                delUpstreamVlanPolicer(&ruleData);
        }

        system("/userfs/bin/hw_nat -+0"); // clear hw_nat rules to make the policer effective
        
        return 0;
}
#endif

int omciMeInitForGEMPortProtocolMonitoringHistoryDataPart2(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n enter omciMeInitForGEMPortProtocolMonitoringHistoryDataPart2 \r\n");

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

int32 setGEMPortProtocolMonitoringHistoryDataPart2ThresholdData(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
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
	
	if(thresholdMeId == 0)
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

int32 getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter: enter!\n");

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 1, tempBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter: pmmgrTcapiGet fail!\n");
		return -1;
	}

	sscanf(tempBuffer, "%x", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGEMPortProtocolMonitoringHistoryDataPart2LostGEMFragmentCounter: pmData = %d\n",pmData);

	put32(value, pmData);

	return 0;
}

int32 getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter: enter!\n");

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 2, tempBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter: pmmgrTcapiGet fail!\n");
		return -1;
	}

	sscanf(tempBuffer, "%x", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGEMPortProtocolMonitoringHistoryDataPart2TransmittedGEMFragmentCounter: pmData = %d\n",pmData);

	put32(value, pmData);

	return 0;
}
#endif


/*******************************************************************************************************************************
9.2.3 GEM port network CTP 

********************************************************************************************************************************/
int omciMeInitForGemPortCtp(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

int omciGetTcontInstCounts(void)
{
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	uint16 tcontAllocId = 0;
	int tcontCounts = 0;
	
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_T_CONT);
	for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
	{
		/*Alloc-Id*/
		tcontAllocId = get16(meInstantCurPtr->attributeVlaue_ptr + 2);
		if (tcontAllocId != 0x00FF)
			tcontCounts ++;
	}

	return tcontCounts;
}
int omciGemPortReconfig(uint16 allocId)
{
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	uint8 attributeValuePtr = NULL;
	uint16 gemPortInstanceId = 0;
	uint16 gemPortId = 0;
	uint16 tcontInstanceId = 0;
	uint16 tcontAllocId = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};

	
	/*search which gemport  T-CONT pointer's allocid == assigned allocId*/

	/*1------Gem port Id*/
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_GEM_PORT_CTP);
	for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
	{
		gemPortInstanceId = get16(meInstantCurPtr->attributeVlaue_ptr);
		/*1------Gem port Id*/
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_GEM_PORT_CTP, gemPortInstanceId, 1);
		if (attributeValuePtr != NULL)
			gemPortId = get16(attributeValuePtr);
		else
			goto fail;	
		/*2------T-CONT pointer*/
			attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_GEM_PORT_CTP, gemPortInstanceId, 2);
			if (attributeValuePtr != NULL)
				tcontInstanceId = get16(attributeValuePtr);
			else
				goto fail;	
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);
		
			/*Alloc-Id*/
			if ((tcontInstanceId != 0) && (tcontInstanceId != 0xFFFF))
			{
				attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_T_CONT, tcontInstanceId, 1);
				if (attributeValuePtr != NULL)
					tcontAllocId = get16(attributeValuePtr);
				else
					goto fail;	
			}
			else
			{
				tcontAllocId = 0xFFFF;
				continue;
			}

			if (tcontAllocId == allocId)
			{	/*set gemport-->tcont mapping*/
				memset(tempBuffer, 0, sizeof(MAX_BUFFER_SIZE));
				sprintf(tempBuffer, "%d", gemPortId);
				tcapi_set("GPON_GEMPort", "PortId", tempBuffer);
				memset(tempBuffer, 0, sizeof(MAX_BUFFER_SIZE));
				sprintf(tempBuffer, "%d", tcontAllocId);				
				tcApiSetAttriubte("GPON_GEMPort", "AllocId",tempBuffer);
			}
	}

fail:
	return -1;
}

#ifdef TCSUPPORT_MT7530_SWITCH_API
int setDsTrtcmRateLimitBySwitch(TrtcmParam trtcm)
{
    mt7530_switch_api_trtcm_t trtcm_switch;
    int ret = 0;

    if ((trtcm.CIR == 0) || (trtcm.CIR > 65535 * 8000))
    {
        trtcm.CIR = 65535 * 8000 ;
    }

    if ((trtcm.PIR == 0) || (trtcm.PIR > 65535 * 8000))
    {
        trtcm.PIR = 65535 * 8000;
    }

    if (trtcm.CIR > trtcm.PIR)
        trtcm.CIR = trtcm.PIR;

/* local setting */
    if ((trtcm.CBS == 0) ||(trtcm.CBS > 0xffff))
    {
        trtcm.CBS = 0xffff;
    }

/* local setting */
    if ((trtcm.PBS == 0) || (trtcm.PBS > 0xffff))
    {
        trtcm.PBS = 0xffff;
    }

    if (trtcm.CBS > trtcm.PBS)
        trtcm.CBS = trtcm.PBS;

    if (dsTrtcmEnableFlag == 0)
	{
		macMT7530SetDsTrtcmEnable(1);
		dsTrtcmEnableFlag = 1;
	}
    
/* 7530 switch api*/	
    memset(&trtcm_switch, 0, sizeof(trtcm_switch));
    trtcm_switch.trtcmId = trtcm.chanID;
    trtcm_switch.CBS = trtcm.CBS;
    trtcm_switch.PBS = trtcm.PBS;
    trtcm_switch.CIR = trtcm.CIR/8000;
    trtcm_switch.PIR = trtcm.PIR/8000;
    ret = macMT7530SetDsTrtcm(&trtcm_switch);

    return ret;

}
#endif

int32 setQueueMappingTrtcmPara(uint16 gemPortId, uint16 tdPointerInstanceId , uint16 direction)
{
    uint8 *attrValPtr = NULL;
    TrtcmParam trtcm;
    char buffer[MAX_BUFFER_SIZE] = {0};
    int ret = 0;
    int trtcmTableLen = 0;
    uint8 chanID = 0;
#ifdef TCSUPPORT_CPU_EN7521
    int meterStart = 0;   
#endif

    
/*1------CIR*/
    attrValPtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerInstanceId, 1);
    if (attrValPtr != NULL)
            trtcm.CIR = get32(attrValPtr);
    else
            return -1;

/*2------PIR*/
    attrValPtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerInstanceId, 2);
    if (attrValPtr != NULL)
            trtcm.PIR = get32(attrValPtr);
    else
            return -1;

/*3------CBS*/
    attrValPtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerInstanceId, 3);
    if (attrValPtr != NULL)
            trtcm.CBS= get32(attrValPtr);
    else
            return -1;

/*4------PBS*/
    attrValPtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerInstanceId, 4);
    if (attrValPtr != NULL)
            trtcm.PBS= get32(attrValPtr);
    else
            return -1;

#ifdef TCSUPPORT_PON_ROSTELECOM
	if ((0 == trtcm.CIR) && (0 == trtcm.PIR) && (0 == trtcm.CBS) && (0 == trtcm.PBS))
	{
		/* the traffic descriptor is not really used */
		if(direction == UPSTREAM_GEMPORT){
    		return INVALID_TRTCM_CHAN;
        }else if (direction == DOWNSTREAM_GEMPORT){
            return CONST_DS_TRTCM_NUMBER;
        }
	}
    
#endif

    if(direction == UPSTREAM_GEMPORT){
        trtcmTableLen = TRTCM_CHAN_NUM_FOR_GEMPORT;
    }else if(direction == DOWNSTREAM_GEMPORT){
        trtcmTableLen = CONST_DS_TRTCM_NUMBER;
    }
    else
        return -1;
   
	
/*avaliable trtcm channel*/
    trtcm.chanID= getTrtcChan4Gemport(gemPortId,direction);
    if ( (trtcm.chanID == trtcmTableLen) || (trtcm.chanID == -1) )
    {
            OMCI_ERROR("no avaible channel for trtcm\n");   
            return -1;
    }

    chanID = (trtcm.chanID >= trtcmTableLen) ? 0 : trtcm.chanID;

#ifndef TCSUPPORT_CPU_EN7521
    if(direction == UPSTREAM_GEMPORT){
        if(0 != setTrtcmParam(&trtcm))
                return -1;
    }else if (direction == DOWNSTREAM_GEMPORT){
    #ifdef TCSUPPORT_MT7530_SWITCH_API
        if(0 != setDsTrtcmRateLimitBySwitch(trtcm))
            return -1;
    #endif
    }
    else
        return -1;
#else
    if(direction == UPSTREAM_GEMPORT){
        meterStart = UP_GEMPORT_TO_METER_START;
    }else if(direction == DOWNSTREAM_GEMPORT){
        meterStart = DOWN_GEMPORT_TO_METER_START;
    }
    else
        return -1;
    ppemgr_lib_set_meter_ratelimit(meterStart+chanID, (trtcm.PIR*8)/1000);
#endif
    ret = setTrtcmUsedByGemportMapping(gemPortId, chanID, direction);
    if (ret != 0)
          return -1;

    return chanID;

}

uint8 searchFreeDsTrtcmTable(uint16 gemPortId)
{
	uint8 trtcmId = 0;

	/* search the gemPortId using channel id */
	for (trtcmId = 1; trtcmId < CONST_DS_TRTCM_NUMBER; trtcmId++)
	{
		if (dsTrtcmTable[trtcmId] == gemPortId)
			return trtcmId;
	}

	/* if the gemport id is not using channel, then select an avaible channel for Trtcm*/
	for (trtcmId = 1; trtcmId < CONST_DS_TRTCM_NUMBER; trtcmId++)
	{
		if (dsTrtcmTable[trtcmId] == 0xffff)
			break;
	}

	return trtcmId;	
}


int setUseDsTrtcmTable(uint16 gemPortId, uint8 trtcmId)
{
	if ((trtcmId == 0) || (trtcmId >= CONST_DS_TRTCM_NUMBER) || (gemPortId == 0xffff))
		return -1;
	dsTrtcmTable[trtcmId] = gemPortId;

#ifdef TCSUPPORT_MT7530_SWITCH_API	
	if (dsTrtcmEnableFlag == 0)
	{
		macMT7530SetDsTrtcmEnable(1);
		dsTrtcmEnableFlag = 1;
	}
#endif		
	return 0;
}

int setAllAvaDsTrtcmTable(void)
{
	int i = 0;

	for(i = 0; i < CONST_DS_TRTCM_NUMBER; i++)
		dsTrtcmTable[i] = 0xffff;
	return 0;
}

int setAvaDsTrtcmTable(uint16 gemPortId)
{
	uint8 trtcmId = 0;

	if (gemPortId == 0xffff)
		return 0;
	
	for (trtcmId = 1; trtcmId < CONST_DS_TRTCM_NUMBER; trtcmId++)
		if (dsTrtcmTable[trtcmId] == gemPortId)
			dsTrtcmTable[trtcmId] = 0xffff;

	for (trtcmId = 1; trtcmId < CONST_DS_TRTCM_NUMBER; trtcmId++)
		if (dsTrtcmTable[trtcmId] != 0xffff)
			break;
			
#ifdef TCSUPPORT_MT7530_SWITCH_API	
	if (trtcmId == CONST_DS_TRTCM_NUMBER)	
	{
		macMT7530SetDsTrtcmEnable(0);
		dsTrtcmEnableFlag = 0;
	}
#endif

	return 0;
}

int32 setDsTrtcmPara(uint16 gemPortId, uint16 tdPointerDownInstanceId , omciMeInst_t *meInstant_ptr, uint8 *tsTrtcmlId)
{
	uint8 *attributeValuePtr = NULL;
	uint8 trtcmAvaChannel = 0;
	uint32 trtcmCIR = 0;
	uint32 trtcmPIR = 0;
	uint32 trtcmCBS = 0;
	uint32 trtcmPBS = 0;
	char buffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;
#ifdef TCSUPPORT_MT7530_SWITCH_API
	mt7530_switch_api_trtcm_t trtcm;
#endif
#ifdef TCSUPPORT_CDS
	uint8 channelId = 0;
#endif

	if (meInstant_ptr == NULL)
		goto fail;
	
/*1------CIR*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerDownInstanceId, 1);
	if (attributeValuePtr != NULL)
		trtcmCIR = get32(attributeValuePtr);
	else
		goto fail;	

/*2------PIR*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerDownInstanceId, 2);
	if (attributeValuePtr != NULL)
		trtcmPIR = get32(attributeValuePtr);
	else
		goto fail;	

/*3------CBS*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerDownInstanceId, 3);
	if (attributeValuePtr != NULL)
		trtcmCBS = get32(attributeValuePtr);
	else
		goto fail;	

/*4------PBS*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, tdPointerDownInstanceId, 4);
	if (attributeValuePtr != NULL)
		trtcmPBS = get32(attributeValuePtr);
	else
		goto fail;	

#ifdef TCSUPPORT_PON_ROSTELECOM	
	if ((0 == trtcmCIR) && (0 == trtcmPIR) && (0 == trtcmCBS) && (0 == trtcmPBS))
	{
		/* the traffic descriptor is not really used */
		*tsTrtcmlId = CONST_DS_TRTCM_NUMBER;
		return 0;
	}
#endif

/*avaliable trtcm channel*/
	if ((trtcmAvaChannel = searchFreeDsTrtcmTable(gemPortId)) == CONST_DS_TRTCM_NUMBER)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "no avaible channel for trtcm\n");	
		goto fail;
	}

#ifdef TCSUPPORT_CDS
	for (channelId = 0; channelId < TRTCM_CHAN_NUM_FOR_GEMPORT; channelId++)
	{
		if (gemPortId == trtcmChannel[channelId])
			break;
	}
	if(channelId >= 8)
		channelId = 0;
	ppemgr_lib_set_meter_ratelimit(DOWN_GEMPORT_TO_METER_START+channelId, (trtcmPIR*8)/1000);
#endif

	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "tsTrtcmlId = %d, CIR = %d, PIR = %d, CBS = %d, PBS = %d\n", trtcmAvaChannel, trtcmCIR, trtcmPIR, trtcmCBS, trtcmPBS);	

/* local setting */
	if ((trtcmCIR == 0) || (trtcmCIR > 65535 * 8000))
	{
		trtcmCIR = 65535 * 8000 ;
	}

	if ((trtcmPIR == 0) || (trtcmPIR > 65535 * 8000))
	{
		trtcmPIR = 65535 * 8000;
	}

	if (trtcmCIR > trtcmPIR)
		trtcmCIR = trtcmPIR;

/* local setting */
	if ((trtcmCBS == 0) ||(trtcmCBS > 0xffff))
	{
		trtcmCBS = 0xffff;
	}

/* local setting */
	if ((trtcmPBS == 0) || (trtcmPBS > 0xffff))
	{
		trtcmPBS = 0xffff;
	}

	if (trtcmCBS > trtcmPBS)
		trtcmCBS = trtcmPBS;

	ret = setUseDsTrtcmTable(gemPortId, trtcmAvaChannel);
	if (ret != 0)
		goto fail;

	*tsTrtcmlId = trtcmAvaChannel;
	
/* 7530 switch api*/
#ifdef TCSUPPORT_MT7530_SWITCH_API		
	memset(&trtcm, 0, sizeof(trtcm));
	trtcm.trtcmId = trtcmAvaChannel;
	trtcm.CBS = trtcmCBS;
	trtcm.PBS = trtcmPBS;
	trtcm.CIR = trtcmCIR/8000;
	trtcm.PIR = trtcmPIR/8000;
	ret = macMT7530SetDsTrtcm(&trtcm);
#endif

	return 0;
fail:
	return -1;
}


#ifdef TCSUPPORT_GPON_MAPPING
int omciAddQueueMappingRule(uint16 meClassId , omciMeInst_t *meInstant_ptr)
{
	int ret = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 gemPortId = 0;
	uint16 gemPortInstanceId = 0;
	uint16 tcontInstanceId = 0;
	uint8 direction = 0;
	uint8 gemPortType = 0;
	uint16 tcontAllocId = 0;
	uint8 TMOption = 0;
	uint16 TMPointerUpInstanceId = 0;
	uint16 tdPointerUpInstanceId = 0;
	uint16 tdPointerDownInstanceId = 0;
	uint16 PQPointerDownInstanceId = 0;
	uint8 tsChannelId = 0;
	uint8 tsTrtcmId = 0;
	uint8 tsDsTrtcmId = 0;
	uint8 queueId  = 0;
	uint16 queuePriority = 0;
	uint8 lanPortId = 0;
	uint8 downQueueId = 0;
	uint8 queueFlag = 0; /* multi queue, or specfic queue*/
	uint8 descriptorFlag = 0;  			/*traffic shaping enable/disable*/
	uint8 downDescriptorFlag = 0;
	uint8 queueTrtcmEnable = DISABLE;
	gponQueueMappingIoctl_t data;
	int isDsTrtcmSupport = 1;

	if (meInstant_ptr == NULL)
		goto fail;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);

	gemPortInstanceId = get16(meInstant_ptr->attributeVlaue_ptr);

/*3------Direction*/
	attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 3);
	if (attributeValuePtr != NULL)
	{
		direction = *attributeValuePtr;
		if (direction == DIRECTION_ANI_TO_UNI)
		{
			gemPortType = GEM_TYPE_MULTICAST;
		}
		else if (direction == DIRECTION_BIDIRECTIONAL)
		{
			gemPortType = GEM_TYPE_UNICAST;
		}
	}
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "FILE = %s, LINE = %d, direction = %d\n",__FILE__, __LINE__, direction);
	
/*1------Gem port Id*/
	attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 1);
	if (attributeValuePtr != NULL)
		gemPortId = get16(attributeValuePtr);
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);

/*2------T-CONT pointer*/
	attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 2);
	if (attributeValuePtr != NULL)
		tcontInstanceId = get16(attributeValuePtr);
	else
		goto fail;	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);

	/*Alloc-Id*/
	if ((direction != DIRECTION_ANI_TO_UNI) && ((tcontInstanceId != 0) && (tcontInstanceId != 0xFFFF)))
	{
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_T_CONT, tcontInstanceId, 1);
		if (attributeValuePtr != NULL)
			tcontAllocId = get16(attributeValuePtr);
		else
			goto fail;	
	}
	else
	{
		tcontAllocId = 0xFFFF;
	}

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);
/*4-------TMPointerUp*/
	attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_ONU_G, 0, 4);	
	if (attributeValuePtr != NULL)
		TMOption = *attributeValuePtr;
	else
		goto fail;			

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);
	if (TMOption == 1)
	{
		/*point to T-CONT, no need to handle*/
	}
	else if(TMOption == 0 || TMOption == 2)
	{
		attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 4);	
		if (attributeValuePtr != NULL)
			TMPointerUpInstanceId = get16(attributeValuePtr);
		else
			goto fail;		
		
		if ((direction != DIRECTION_ANI_TO_UNI) && (TMPointerUpInstanceId != 0) && (TMPointerUpInstanceId != 0xFFFF))
		{/*upstream priority queue is setted by OLT.*/
			/*priority queue id*/
			attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_PRIORITY_QUEUE, TMPointerUpInstanceId, 6);	
			if (attributeValuePtr != NULL)
			{
				queuePriority = get16(attributeValuePtr+2);
				queueId = (7 -((uint8)queuePriority & 0x07));
			}
			else
				goto fail;		

			queueFlag = TRAFFIC_SCHEDULER_SPECIFIC_QUEUE_MODE;
		}
		else
		{/*upstream priority queue is not setted by OLT.*/
			queueFlag = TRAFFIC_SCHEDULER_MULTI_QUEUE_MODE; 
		}
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "the Traffic management Option is wrong\n");
		goto fail;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);
	
/*5------TDPointerUp	*/
	if (TMOption == 0)
	{
		/*TDPointerUp is not used, no need to handle*/
	}
	else if(TMOption == 1 || TMOption == 2)
	{
		attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 5);	
		if (attributeValuePtr != NULL)
			tdPointerUpInstanceId = get16(attributeValuePtr);
		else
			goto fail;	
		
		if ((direction != DIRECTION_ANI_TO_UNI)
            && (tdPointerUpInstanceId != 0) && (tdPointerUpInstanceId != 0xFFFF) 
#ifndef TCSUPPORT_CDS            
            &&(HGU != pSystemCap->onuType)
#endif
            )
		{/*upstream traffic descriptor is setted by OLT.*/
			descriptorFlag = TRAFFIC_SHAPING_ENABLE;
			
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]\n",__LINE__, __func__);
			/*set trtcm parameter: channel, cir, pir, cbs, pbs*/
			tsChannelId = setQueueMappingTrtcmPara(gemPortId, tdPointerUpInstanceId, UPSTREAM_GEMPORT);
			if (tsChannelId == -1)
				goto fail;

#ifdef TCSUPPORT_PON_ROSTELECOM
			if (INVALID_TRTCM_CHAN == tsChannelId)
			{
				descriptorFlag = TRAFFIC_SHAPING_DISABLE;
				tsChannelId = 0;
			}
#endif
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "[%5d][%s]\n",__LINE__, __func__);
		}
		else
		{/*upstream traffic descriptor is not setted by OLT.*/
			descriptorFlag = TRAFFIC_SHAPING_DISABLE;
		}
		
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "the Traffic management Option is wrong\n");
		goto fail;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "FILE = %s, LINE = %d\n",__FILE__, __LINE__);

/*7------PQPointerDown	*/
	attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 7);	
	if (attributeValuePtr != NULL)
		PQPointerDownInstanceId = get16(attributeValuePtr);
	else
		goto fail;	
		
	/*priority queue id*/
	if (PQPointerDownInstanceId != 0xFFFF)
	{
		attributeValuePtr = omciGetAttriValueFromInst(OMCI_CLASS_ID_PRIORITY_QUEUE, PQPointerDownInstanceId, 6);	
		if (attributeValuePtr != NULL)
		{
			queuePriority = get16(attributeValuePtr);
			lanPortId = (uint8) queuePriority;
			queuePriority = get16(attributeValuePtr+2);
			downQueueId = (uint8) queuePriority;
		}
	}
#ifdef TCSUPPORT_CDS
        // downstream gemport mapping to queue by priority queue
        if(downQueueId <8 )
            downQueueId = 7 - downQueueId;
        // downstream boardcast gemport mapping to queue0
        if(gemPortId == 4095 )
            tsChannelId = 0;
        // downstream multicast gemport mapping to queue7
        if(gemPortId == 4094 )
            tsChannelId = 7;
        //  more than 8 gemport, other gemport is mapping to Queue 0
        if(tsChannelId >= 8)
            tsChannelId = 0;
#endif

/*9------TDPointerDown	*/
	if (TMOption == 0)
	{
		/*TDPointerDown is not used, no need to handle*/
	}
	else if(TMOption == 1 || TMOption == 2)
	{
		attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 9);	
		if (attributeValuePtr != NULL)
			tdPointerDownInstanceId = get16(attributeValuePtr);
		else
			goto fail;	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]\n",__LINE__, __func__);

		if(TCSUPPORT_CUC_VAL)
			isDsTrtcmSupport = 0;
		
		if (isDsTrtcmSupport && ((tdPointerDownInstanceId != 0) && (tdPointerDownInstanceId != 0xFFFF)))
		{/*downstream traffic descriptor is setted by OLT.*/
			downDescriptorFlag = TRAFFIC_SHAPING_ENABLE;
			
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]\n",__LINE__, __func__);
			/*set trtcm parameter: channel, cir, pir, cbs, pbs*/
			tsDsTrtcmId = setQueueMappingTrtcmPara(gemPortId, tdPointerDownInstanceId, DOWNSTREAM_GEMPORT);
			if (tsDsTrtcmId == -1)
#ifdef TCSUPPORT_CHS
				tsDsTrtcmId = setQueueMappingTrtcmPara(gemPortId, 0xFFFF, DOWNSTREAM_GEMPORT);
				if (tsDsTrtcmId == -1)
#endif
				goto fail;

#ifdef TCSUPPORT_PON_ROSTELECOM
			if (CONST_DS_TRTCM_NUMBER == tsDsTrtcmId)
			{
				tsDsTrtcmId = 0;
				downDescriptorFlag = TRAFFIC_SHAPING_DISABLE;
			}
#endif
		}
		else
		{/*downstream traffic descriptor is not setted by OLT.*/
#ifdef TCSUPPORT_CDS
		    tsDsTrtcmId = setQueueMappingTrtcmPara(gemPortId, tdPointerDownInstanceId, DOWNSTREAM_GEMPORT);
		    if (tsDsTrtcmId == -1)
		        goto fail;
#endif
			downDescriptorFlag = TRAFFIC_SHAPING_DISABLE;
		}
		
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "the Traffic management Option is wrong\n");
		goto fail;
	}
	omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s]\n",__LINE__, __func__);


	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciQueueMapping:gemPort = %x, gemPortType = %x, pqMode = %x, queue = %x, tcontAllocId = %x, tsEnable = %x, tsChannelId = %x\n", 
											gemPortId, gemPortType, queueFlag, queueId, tcontAllocId, descriptorFlag, tsChannelId);
/*gpon up queue map api*/
	memset(&data, 0, sizeof(gponQueueMappingIoctl_t));
	data.gemPort = gemPortId;
	data.gemType = gemPortType;
	data.pqMode = queueFlag;
	data.queue = queueId;
	data.allocId = tcontAllocId;
	data.tsEnable = descriptorFlag;
	data.tsChannelId = tsChannelId;
	
	ret = addQueueMappingRule(&data);
	if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciQueueMapping: add the rule success\n");
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciQueueMapping: add the rule fail\n");	
		goto fail;
	}	

#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "omciQueueMapping:gemPort = %x, lanPortId = eth0.%d, downQueueId = %x, downTrtcmId = %x\n", gemPortId, lanPortId, downQueueId, tsDsTrtcmId);
	if (lanPortId == 0)
		lanPortId = 1;
    
    /*gpon down queue map api*/
    uint8 if_mask = 0;
#if !defined(TCSUPPORT_CT_PON) && !defined(TCSUPPORT_CDS) 
    if_mask = 0x0F;
#endif


	gpon_downstream_mapping_ioctl ds_rule;
	memset(&ds_rule, 0, sizeof(ds_rule));
	ds_rule.gem_port_num = gemPortId;
	ds_rule.if_mask = if_mask;
	ds_rule.queue = downQueueId;
	ds_rule.trtcmId = tsDsTrtcmId;
    ds_rule.ds_pq_enable = (PQPointerDownInstanceId != 0) ? ENABLE : DISABLE;
    ds_rule.ds_trtcm_enable = downDescriptorFlag;
	ds_rule.option_flag = OPT_SET;

	ret = addGemDownstreamMappingRule(ds_rule);

	if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s] success!\n", __LINE__, __func__);
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "[%5d][%s] fail!\n", __LINE__, __func__);
		goto fail;
	}	
#endif
	return 0;
fail:
	return OMCI_CMD_ERROR;
}

int omciDelQueueMappingRule(uint16 meClassId , omciMeInst_t *meInstant_ptr)
{
	int ret = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 gemPortId = 0;
	uint16 gemPortInstanceId = 0;

	gponQueueMappingIoctl_t data;
#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)	
	gpon_downstream_mapping_ioctl dataDown;
#endif


	gemPortInstanceId = get16(meInstant_ptr->attributeVlaue_ptr);
/*Gem port Id*/
	attributeValuePtr = omciGetAttriValueFromInst(meClassId, gemPortInstanceId, 1);
	if (attributeValuePtr != NULL)
		gemPortId = get16(attributeValuePtr);
	else
		goto fail;	


/*gpon queue map api*/
	memset(&data, 0, sizeof(gponQueueMappingIoctl_t));
	data.gemPort = gemPortId;
	
	ret = delQueueMappingRule(&data);
	if (ret == GPONMAP_ENTRY_NOT_FOUND)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule:GPONMAP_ENTRY_NOT_FOUND\n");
	}
	else if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule: delete old queue mapping rule\n");		
	}
	else if (ret == GPONMAP_FAIL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule: delete queue mapping rule fail\n");		
		goto fail;
	}
	
#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
	/*gpon down queue map api*/
	memset(&dataDown, 0, sizeof(dataDown));

	dataDown.gem_port_num = gemPortId;

	dataDown.option_flag = OPT_DEL;
	ret = gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&dataDown, sizeof(dataDown));
	if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule: delete old down queue mapping rule\n");		
	}
#endif

	return 0;
fail:
	return OMCI_CMD_ERROR;
}
int omciReconfigQueueMappingRule(void)
{
	int ret = 0;

	gponQueueMappingIoctl_t data;

/*gpon queue map api*/
	memset(&data, 0, sizeof(gponQueueMappingIoctl_t));
	data.gemPort = RECONFIG_GPONMAP_RULE;
	
	ret = addQueueMappingRule(&data);

 	if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule: delete old queue mapping rule\n");		
	}
	else 
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciDelQueueMappingRule: delete queue mapping rule fail\n");		
		goto fail;
	}
	
	return 0;
fail:
	return OMCI_CMD_ERROR;
}

#ifdef TCSUPPORT_CDS
static void InitGemRatelimit(void)
{
    int chanID = 0;
    for(chanID=0;chanID<UP_GEMPORT_TO_METER_NUM;chanID++){
        ppemgr_lib_set_meter_ratelimit(DOWN_GEMPORT_TO_METER_START+chanID, 1024000);
        ppemgr_lib_set_meter_ratelimit(UP_GEMPORT_TO_METER_START+chanID, 1024000);
    }

	return;
}
#endif
int omciEmptyQueueMappingRule(void)
{
	int ret = 0;

	gponQueueMappingIoctl_t data;
#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)	
	gpon_downstream_mapping_ioctl dataDown;
#endif

/*gpon queue map api*/
	memset(&data, 0, sizeof(gponQueueMappingIoctl_t));
	data.gemPort = EMPTY_GPONMAP_RULE;
	
	ret = delQueueMappingRule(&data);
	if (ret == GPONMAP_ENTRY_NOT_FOUND)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciEmptyQueueMappingRule:GPONMAP_ENTRY_NOT_FOUND\n");
	}
	else if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciEmptyQueueMappingRule: delete old queue mapping rule\n");		
	}
	else if (ret == GPONMAP_FAIL)
	{	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciEmptyQueueMappingRule: delete queue mapping rule fail\n");		
		goto fail;
	}
	
	InitGponMappingTrtcm();
	setAllAvaDsTrtcmTable();

#if defined(TCSUPPORT_GPON_DOWNSTREAM_MAPPING)
	/*gpon down queue map api*/
	memset(&dataDown, 0, sizeof(dataDown));

	dataDown.option_flag = OPT_CLEAN;
	ret = gponmapIoctl(DOWNSTREAM_MAPPING_RULE_OPT, (void *)&dataDown, sizeof(dataDown));
	if (ret == GPONMAP_SUCCESS)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "omciEmptyQueueMappingRule: empty down queue mapping rule\n");		
	}
#endif

#ifdef TCSUPPORT_CDS
	InitGemRatelimit();
#endif

	return 0;
fail:
	return OMCI_CMD_ERROR;
}

#endif

int32 getUNIcounterValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint16 meId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
//	sprintf(name, "GEMPort%d", meId);
//	/*tcapi_set(name, "TMPointerUp", value);*/
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	memcpy(attributeValuePtr, value, length);
	
	return 0;
}

int32 setPQPointerDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	uint16 meId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	if(tmpomciMeInst_p->attributeVlaue_ptr != NULL)
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	else
	{
		printf("attributeValuePtr == NULL");
		return -1;
	}	
	tcapi_set("GPON_GEMPort", "Number", (char* )&meId);
	tcApiSetAttriubte("GPON_GEMPort", "PQPointerDown", value);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		memcpy(attributeValuePtr, value, length);
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
		return -1;
	}	
	
	return 0;
}

int32 getEncryptionStateValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	uint16 meId = 0;
	uint16 gemPortId = 0;
	uint32 encryState = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	
	if(tmpomciMeInst_p->attributeVlaue_ptr != NULL)
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	else
	{
		printf("attributeValuePtr == NULL");
		return -1;
	}	
	
/*1------Gem port Id*/
	attributeValuePtr = omciGetAttriValueFromInst(tmpomciMeInst_p->classId, meId, 1);
	if (attributeValuePtr != NULL)
		gemPortId = get16(attributeValuePtr);
	else
		goto fail;		
	memset(tempBuffer, 0, sizeof(MAX_BUFFER_SIZE));
	sprintf(tempBuffer, "%d", gemPortId);
	tcapi_set("GPON_GEMPort", "PortId", tempBuffer);

	memset(tempBuffer, 0, sizeof(MAX_BUFFER_SIZE));
	tcapi_get("GPON_GEMPort", "EncryptionState", tempBuffer);
	sscanf(tempBuffer, "%d", &encryState);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = (uint8)encryState;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "attributeValuePtr == NULL");
		return -1;
	}	
	
	return getTheValue(value, (char*)attributeValuePtr, length, omciAttribute);	

fail:
	return -1;
}
int32 setTDPointerDownValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	return 0;
}

/*******************************************************************************************************************************
9.2.3 Priority Queue

********************************************************************************************************************************/
int32 getMaxQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 maxQueueSize = 0;
	int ret = 0;
	uint16 meId = 0;
	uint8 queueId = 0;	

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/
		
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
#ifdef TCSUPPORT_MT7530_SWITCH_API		
		maxQueueSize = macMT7530GetMaximumQueueSize(queueId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getMaxQueueSizeValue: maxQueueSize = %ld\r\n",maxQueueSize);		
		put16(attributeValuePtr, maxQueueSize);
#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
			
	}
	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n getMaxQueueSizeValue: get maxQueueSize fail!\r\n");		
		return -1;
	}


	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 getAllocQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId = 0;	
	uint16 allocQueueSize = 0;
	uint8 queueId = 0;

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);				
#ifdef TCSUPPORT_MT7530_SWITCH_API		
		allocQueueSize = macMT7530GetAllocatedQueueSize(queueId);
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getAllocQueueSizeValue: allocQueueSize = %ld\r\n",allocQueueSize); 	
		put16(attributeValuePtr, allocQueueSize);

#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
			
	}
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setAllocQueueSizeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 allocQueueSize = 0;
	uint8 queueId = 0;
	int ret = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId = 0;	

	if(value == NULL)
	{
		return -1;
	}
	allocQueueSize = get16((uint8 *)value);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n setAllocQueueSizeValue: allocQueueSize = %ld\r\n",allocQueueSize);		
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	
	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);				
#ifdef TCSUPPORT_MT7530_SWITCH_API	
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setAllocQueueSizeValue: allocQueueSize=%d, queueId=%d!\n",allocQueueSize, queueId);
		ret = macMT7530SetAllocatedQueueSize(allocQueueSize, queueId);
#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
			
	}
	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "\r\n setAllocQueueSizeValue: set allocQueueSize fail!\r\n");		
		return -1;
	}
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr , omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, allocQueueSize);
	else
		return -1;
	
	return 0;
}

int32 getRelatePortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;

	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setRelatePortValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr;
	uint16 length = 0;
	omciManageEntity_t *currentMe = NULL;
	omciMeInst_t *currentMeInst_p = NULL;
	uint16 qosCfg = 0;
	uint16 port = 0;
	uint16 priority = 0;
	uint32 relatedPort = 0;
	uint16 queueId = 0;
	uint16 meId = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 trafficSchedPoint =  0;
	uint8 weight = 0;
	uint16 portOld = 0;
	uint32 valueTmp = 0;
	int ret =  0;
	

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, "Traffic scheduler pointer", &length);
	trafficSchedPoint = get16(attributeValuePtr);

	currentMe = omciGetMeByClassId(OMCI_CLASS_ID_ONU2_G);
	currentMeInst_p = omciGetInstanceByMeId(currentMe, 0);
	attributeValuePtr = omciGetInstAttriByName(currentMeInst_p , "QoS configuration", &length);
	
	qosCfg = get16(attributeValuePtr);

	relatedPort = get32((uint8 *)value);

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setRelatePortValue: qosCfg = 0x%x, relatedPort = 0x%x\n", qosCfg, relatedPort);
	
	//bit 1: priority queue ME's port field of related port attribute is read-write and can point to any T-CONT or UNI port in the same slot;
	//bit 2: priority queue ME's traffic scheduler pointer is permitted to refer to any other traffic scheduler in the same slot
	//bit 3: traffic scheduler ME's T-CONT pointer is read-write
	//bit 4: traffic scheduler ME's policy attribute is read-write
	//bit 5: T-CONT ME's policy attribute is read-write
	//bit 6: Priority queue ME's Priority field of related port attribute is read-write
	//others: reserved

	if(((qosCfg & 0x01) != 0) && ((qosCfg & 0x20) != 0))
	{
		if(trafficSchedPoint == 0)//traffic scheduler pointer is null
		{
			port = (uint16)((relatedPort >> 16) & 0x00ff);
			priority = (uint16)(relatedPort & 0x00ff);
			
			valueTmp =  get32(attributeValuePtr);
			portOld = (uint16)((valueTmp >> 16) & 0x00ff);
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setRelatePortValue: port = 0x%x, priority = 0x%x\n", port, priority);
			omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setRelatePortValue: portOld = 0x%x\n", portOld);

			if(meId < MIN_TONT_MEID)
			{
				//downstream priority queue
				/*wait for switch api*/		
				queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
#ifdef TCSUPPORT_MT7530_SWITCH_API					
				macMT7530SetQueuePriority(priority, queueId);	
				weight = macMT7530GetWeight(queueId, portOld);	
				if(portOld != port)
				{
	//				macMT7530CleanWeight(queueId, port);	//no such api now
					ret = macMT7530SetWeight(weight, queueId, port);
				}
#endif	
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setRelatePortValue: set relateport fail!\n");				
					return -1;
				}
			}
			else
			{
				//upstream priority queue
				//wait for pon mac api
					
			}			
		}
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setRelatePortValue: trafficSchedPoint is not null, relateport is igored!\n");					
			return OMCI_PARAM_ERROR;		
		}
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setRelatePortValue: qosCfg is 0x%x, can't not set relateport!\n",qosCfg);					
		return OMCI_PARAM_ERROR;
	}

	
	
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setRelatePortValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 setTrafficSchedPointValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meIdT_CONT = 0;
	uint32 relatePort = 0;
	uint16 trafficSchedId = 0;
	omciManageEntity_t *currentMe = NULL;
	omciMeInst_t *currentMeInst_p = NULL;
	uint16 meIdT_CONT_tmp = 0;
	uint16 qosCfg = 0;
	uint16 meId = 0;

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	if(meId >= MIN_TONT_MEID)
	{
		//upstream priority queue
		trafficSchedId=  get16((uint8 *)value);

		currentMe = omciGetMeByClassId(OMCI_CLASS_ID_ONU2_G);
		currentMeInst_p = omciGetInstanceByMeId(currentMe, 0);
		attributeValuePtr = omciGetInstAttriByName(currentMeInst_p , "QoS configuration", &length);
		
		qosCfg = get16(attributeValuePtr);
		if((qosCfg & 0x02) == 0)
		{
			if(trafficSchedId != 0)
			{
				currentMe = omciGetMeByClassId(OMCI_CLASS_ID_TRAFFIC_SCHEDULER);
				currentMeInst_p = omciGetInstanceByMeId(currentMe, trafficSchedId);
				if(currentMeInst_p == NULL)
					return -1;
				
				attributeValuePtr = omciGetInstAttriByName(currentMeInst_p , "T-CONT pointer", &length);
				meIdT_CONT_tmp = get16(attributeValuePtr);
				
				attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
				relatePort = get32(attributeValuePtr);
				meIdT_CONT = ((relatePort>>16)&0xffff);

				if(meIdT_CONT !=  meIdT_CONT_tmp)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficSchedPointValue: downstream priority queue, reject the operation\n");			
					return OMCI_PARAM_ERROR;
				}
			}
		}

		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
		if (attributeValuePtr != NULL)
		{
			memcpy(attributeValuePtr, value, length);
			return 0;
		}
		else
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficSchedPointValue: attributeValuePtr == NULL\n");
			return -1;
		}	
	}
	else
	{
		//downstream priority queue, reject the operation.
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficSchedPointValue: downstream priority queue, reject the operation\n");		
		return OMCI_PARAM_ERROR;	
	}
	
	return 0;
}


int32 getWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	uint8 weight = 1;
	uint32 relatePort = 0;
	uint8 port = 0;
	uint8 tcontId = 0;
	uint8 qosChannel = 0;
	uint8 qosType = 0;
	uint8 ponWeight[NUM_OF_QUEUE_PER_TCONT] = {0};
	int ret = 0;

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);	

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
		port = (uint8)((relatePort >> 16) & 0x00ff);
#ifdef TCSUPPORT_MT7530_SWITCH_API		
		weight = macMT7530GetWeight(queueId, port);		
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE, "\r\n getWeightValue: macMT7530GetWeight weight = %ld\r\n",weight); 	
		*attributeValuePtr=weight;
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);		
		tcontId = (uint8)((relatePort >> 16) & 0x00ff);	
#if 1
		ret = getPonMacQOSParam(tcontId, &qosChannel, &qosType, ponWeight);
		if(ret != 0)
		{		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getWeightValue: getPonMacQOSParam fail!\n");
			return ret;
		}
		weight = ponWeight[queueId];
		*attributeValuePtr=weight;	
#endif		
	}		
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

/**********************************************
pon mac api
***********************************************/
int setQOSPolicy(uint8 channel,uint8 qosType,uint8 cfg_commit)
{
	uint8 Type = 0;
	uint8 ponWeight[NUM_OF_QUEUE_PER_TCONT] = {0};
    uint8 qosChannel=0;
	int ret=0;

    ret = getPonMacQOSParam(channel, &qosChannel, &Type, ponWeight);
    if(ret != 0)
    {       
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: getPonMacQOSParam fail!\n");
        return -1;
    }


	memset(ponWeight,0,sizeof(ponWeight));
     switch(qosType) {
        case WRR_POLICY:
            Type = 0;//ENUM_QOS_TYPE_WRR;
            break;
        case SP_POLICY:
        default:
            Type = 1;//ENUM_QOS_TYPE_SP;
            break;    
    }
    ret = setPonMacQOSParam(qosChannel, Type, ponWeight, cfg_commit);
    if(ret != 0)
    {       
        omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: setPonMacQOSParam fail!\n");
        return -1;
    }
    return 0;
}

void updatePonQosType(uint8 qosChannel, uint8 qosType)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
	char *tempStr_p = NULL;
	int ret=0;
	int index = 0;


	ret = tcapi_get(GPON_OMCI_ENTRY, PON_QOS_TYPE, buffer);
	if(ret != 0)
	{
		return ret;
	}

	tempStr_p = buffer;
	while(*tempStr_p != '\0')
	{	
	    if (*tempStr_p != GPON_GEMPORT_LIST_SEP)
	    {
	        if (index == qosChannel)
	        {
	            if (0 == qosType)
	                *tempStr_p = '0';//ENUM_QOS_TYPE_WRR
	            else if (1 == qosType)
	                *tempStr_p = '1';//ENUM_QOS_TYPE_SP
	            else
	                *tempStr_p = PON_QOS_TYPE_DEFAULT_VALUE;
	        }
	        index++;
	    }
	    tempStr_p++;
	}
	
    tcapi_set(GPON_OMCI_ENTRY, PON_QOS_TYPE, buffer);

	return 0;
}

int getPonMacQOSParam(uint8 channel,uint8 *qosChannel, uint8 *qosType ,uint8 *ponWeight)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char buffer[MAX_BUFFER_SIZE] = {0};
	char *tempStr_p = NULL;
	uint8 paramNum = 0;
	uint8 paramList[MAX_BUFFER_SIZE] = {0};
	uint8 len = 0;
	int ret=0;
	int i = 0;
#if 1
	if(channel<15)   //map TCONT ME X to channel X+1 
		channel++;
#endif

	sprintf(tempBuffer, "%s%d", ATTR_QOS_VALUE, channel);
	ret = tcapi_get(GPON_OMCI_ENTRY, tempBuffer, buffer);
	if(ret != 0)
	{
		return ret;
	}
		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tempStr_p = buffer;
	while(*tempStr_p != '\0'){		
		if(*tempStr_p == GPON_GEMPORT_LIST_SEP){	
			tempBuffer[len] = '\0';
			paramList[paramNum] = atoi(tempBuffer);
			paramNum++;
			memset(tempBuffer,0,MAX_BUFFER_SIZE);
			len = 0;
			tempStr_p++;
			continue;			
		}
		tempBuffer[len++] = *tempStr_p;
		tempStr_p++;
	}
	
	paramNum = 0;
	*qosChannel = paramList[paramNum++];
	*qosType = paramList[paramNum++];
	for(i=0; i<8; i++)
	{
		ponWeight[i] = paramList[paramNum++];
	}

	return 0;

}
int setPonMacQOSParam(uint8 qosChannel, uint8 qosType, uint8 *ponWeight, uint8 cfg_commit)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", GPON_HANDLE_SET);
	tcapi_set(GPON_OMCI_ENTRY, GPON_QOS_CFG, tempBuffer);

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", MASK_QOS_CONFIG);
	tcapi_set(GPON_OMCI_ENTRY, PON_QOS_MASK, tempBuffer);
		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d_%d_%d_%d_%d_%d_%d_%d_%d_%d_", qosChannel, qosType, ponWeight[0], ponWeight[1], ponWeight[2],
		ponWeight[3], ponWeight[4], ponWeight[5],ponWeight[6], ponWeight[7]);
	tcapi_set(GPON_OMCI_ENTRY, PON_QOS, tempBuffer);

    if (cfg_commit)
    	ret = tcapi_commit(GPON_OMCI_ENTRY);
    else
        updatePonQosType(qosChannel, qosType);

	return ret;
}

int getPonMacCongestDropProbality(uint8 *geenDrop_p, uint8 *yellowDrop_p)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char buffer[MAX_BUFFER_SIZE] = {0};
	char *tempStr_p = NULL;
	uint8 paramNum = 0;
	uint8 paramList[MAX_BUFFER_SIZE] = {0};
	uint8 len = 0;
	int ret=0;
	int i = 0;

	ret = tcapi_get(GPON_OMCI_ENTRY, ATTR_CONGEST_DROP_P, buffer);
	if(ret != 0)
	{
		return ret;
	}
		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tempStr_p = buffer;
	while(*tempStr_p != '\0'){		
		if(*tempStr_p == GPON_GEMPORT_LIST_SEP){	
			tempBuffer[len] = '\0';
			paramList[paramNum] = atoi(tempBuffer);
			paramNum++;
			memset(tempBuffer,0,MAX_BUFFER_SIZE);
			len = 0;
			tempStr_p++;
			continue;			
		}
		tempBuffer[len++] = *tempStr_p;
		tempStr_p++;
	}
	
	paramNum = 0;
	*geenDrop_p = paramList[paramNum++];
	*yellowDrop_p = paramList[paramNum++];

	return 0;

}

int setCongestDropProbality(uint8 geenDrop_p, uint8 yellowDrop_p)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", GPON_HANDLE_SET);
	tcapi_set(GPON_OMCI_ENTRY, GPON_CONGEST_CFG, tempBuffer);

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", MASK_CONGEST_DROP_PROBILITY);
	tcapi_set(GPON_OMCI_ENTRY, PON_CONGEST_MASK, tempBuffer);

		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d_%d_", geenDrop_p, yellowDrop_p);
	tcapi_set(GPON_OMCI_ENTRY, PON_CONGEST, tempBuffer);

	ret = tcapi_commit(GPON_OMCI_ENTRY);

	return ret;

}

int getPonMacCongestThreshold(uint8 queueIdx, uint16 *congestQueueIdx, uint16 *greenMax, uint16 *greenMin, uint16 *yellowMax, uint16 *yellowMin)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	char buffer[MAX_BUFFER_SIZE] = {0};
	char *tempStr_p = NULL;
	uint8 paramNum = 0;
	uint16 paramList[MAX_BUFFER_SIZE] = {0};
	uint8 len = 0;
	int ret=0;
	int i = 0;

	ret = tcapi_get(GPON_OMCI_ENTRY, ATTR_CONGEST_THRED_VALUE, buffer);
	if(ret != 0)
	{
		return ret;
	}
		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	tempStr_p = buffer;
	while(*tempStr_p != '\0'){		
		if(*tempStr_p == GPON_GEMPORT_LIST_SEP){	
			tempBuffer[len] = '\0';
			paramList[paramNum] = atoi(tempBuffer);
			paramNum++;
			memset(tempBuffer,0,MAX_BUFFER_SIZE);
			len = 0;
			tempStr_p++;
			continue;			
		}
		tempBuffer[len++] = *tempStr_p;
		tempStr_p++;
	}
	
	paramNum = 0;
	*congestQueueIdx = paramList[paramNum++];
	*greenMax = paramList[paramNum++];
	*greenMin = paramList[paramNum++];
	*yellowMax = paramList[paramNum++];	
	*yellowMin = paramList[paramNum++];

	return 0;

}


int setCongestThredValue(uint8 congestQueueIdx, uint16 greenMax, uint16 greenMin, uint16 yellowMax, uint16 yellowMin)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", GPON_HANDLE_SET);
	tcapi_set(GPON_OMCI_ENTRY, GPON_CONGEST_CFG, tempBuffer);

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", MASK_CONGEST_DROP_PROBILITY);
	tcapi_set(GPON_OMCI_ENTRY, PON_CONGEST_MASK, tempBuffer);

		
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d_%d_%d_%d_%d_", congestQueueIdx, greenMax, greenMin, yellowMax, yellowMin);
	tcapi_set(GPON_OMCI_ENTRY, MASK_CONGEST_THRED, tempBuffer);

	ret = tcapi_commit(GPON_OMCI_ENTRY);

	return ret;

}


int setPonMacTrafficDescriptor(uint8 portId, trafficDescriptor_t *trafficDescriptor)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	int ret = 0;

	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", GPON_HANDLE_SET);
	tcapi_set(GPON_OMCI_ENTRY, GPON_TRTCM_CFG, tempBuffer);
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d", MASK_TRTCM_PARAMETERS);
	tcapi_set(GPON_OMCI_ENTRY, PON_TRTCM_PARAMS_MASK, tempBuffer);
	
	
	memset(tempBuffer, 0, MAX_BUFFER_SIZE);
	sprintf(tempBuffer, "%d_%u_%u_%u_%u_", portId, trafficDescriptor->CIR, trafficDescriptor->CBS, 
		trafficDescriptor->PIR, trafficDescriptor->PBS);
	tcapi_set(GPON_OMCI_ENTRY, PON_TRTCM_PARAMS, tempBuffer);
	
	ret = tcapi_commit(GPON_OMCI_ENTRY);

	return ret;

}

int32 setWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 weight = 0;
	uint32 relatePort = 0;
	uint8 port = 0;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	
	uint8 tcontId = 0;
	uint8 qosType = 0;
	uint8 qosChannel = 0;
	uint8 ponWeight[NUM_OF_QUEUE_PER_TCONT] = {0};
	

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);

	weight = *value;

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
		port = (uint8)((relatePort >> 16) & 0x00ff);		
#ifdef TCSUPPORT_MT7530_SWITCH_API
		omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"setWeightValue: weight=%d, queueId=%d, port=%d!\n",weight, queueId, port);
		ret = macMT7530SetWeight(weight, queueId, port);
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);
		tcontId = (uint8)((relatePort >> 16) & 0x00ff);	
#if 1
		ret = getPonMacQOSParam(tcontId, &qosChannel, &qosType, ponWeight);
		if(ret != 0)
		{		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: getPonMacQOSParam fail!\n");
			return ret;
		}
		ponWeight[queueId] = weight;

		ret = setPonMacQOSParam(qosChannel, qosType, ponWeight, 1);
		if(ret != 0)
		{		
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: setPonMacQOSParam fail!\n");
			return ret;
		}
#endif					
	}	
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: macMT7530SetWeight fail!\n");
		return ret;
	}
		

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setWeightValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getBackPressureOperatetValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 enableValue = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;
	
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);				
#ifdef TCSUPPORT_MT7530_SWITCH_API					
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOperatetValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}
		enableValue = backPressure.Enable ? 0 : 1;	
		put16(attributeValuePtr, enableValue);
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);				
		ret = ponMacGetBackPressure(&backPressure, queueId);	
	if(ret != 0)
	{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOperatetValue: ponMacGetBackPressure fail!\n");
		return ret;
	}
	enableValue = backPressure.Enable;
	put16(attributeValuePtr, enableValue);
#endif
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setBackPressureOperatetValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 enableValue = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;
	
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);						
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOperatetValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}

		enableValue = get16((uint8 *)value);
		backPressure.Enable = enableValue ? 0 : 1;
		ret = macMT7530SetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOperatetValue: macMT7530SetBackPressure fail!\n");
			return ret;
		}
#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);				
		ret = ponMacGetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOperatetValue: ponMacGetBackPressure fail!\n");
			return ret;
		}

		enableValue = get16(value);
		backPressure.Enable = enableValue;
		ret = ponMacSetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOperatetValue: ponMacSetBackPressure fail!\n");
			return ret;
		}
#endif					
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOperatetValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getBackPressureTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint32 time = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;	
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);								
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOperatetValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}
		time = backPressure.time;	
		put32(attributeValuePtr, time);	
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);						
		ret = ponMacGetBackPressure(&backPressure, queueId);
	if(ret != 0)
	{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOperatetValue: ponMacGetBackPressure fail!\n");
		return ret;
	}
		time = backPressure.time;	
		put32(attributeValuePtr, time);			
#endif
	}
	
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setBackPressureTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint32 time = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;		
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);										
#ifdef TCSUPPORT_MT7530_SWITCH_API					
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureTimeValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}

		time = get32((uint8 *)value);
		backPressure.time = time;
		ret = macMT7530SetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureTimeValue: macMT7530SetBackPressure fail!\n");
			return ret;
		}
#endif	
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);										
		ret = ponMacGetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureTimeValue: ponMacGetBackPressure fail!\n");
			return ret;
		}

		time = get32(value);
		backPressure.time = time;
		ret = ponMacSetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureTimeValue: ponMacSetBackPressure fail!\n");
			return ret;
		}				
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureTimeValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getBackPressureOccurThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 maxQueueThred = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;	
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);												
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOccurThredValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}	
		maxQueueThred = backPressure.MaxQueueThreshold;
		put16(attributeValuePtr, maxQueueThred);

#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);										
		ret = ponMacGetBackPressure(&backPressure, queueId);
	if(ret != 0)
	{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureOccurThredValue: ponMacGetBackPressure fail!\n");
		return ret;
	}
	maxQueueThred = backPressure.MaxQueueThreshold;
	put16(attributeValuePtr, maxQueueThred);
#endif		
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setBackPressureOccurThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 maxQueueThred = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;		
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
#ifdef TCSUPPORT_MT7530_SWITCH_API
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);										
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}
			
		maxQueueThred = get16((uint8 *)value);
		backPressure.MaxQueueThreshold= maxQueueThred;
		ret = macMT7530SetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);

		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: macMT7530SetBackPressure fail!\n");
			return ret;
		}
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);										
		ret = ponMacGetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: ponMacGetBackPressure fail!\n");
			return ret;
		}
			
		maxQueueThred = get16(value);
		backPressure.MaxQueueThreshold= maxQueueThred;
		ret = ponMacSetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: ponMacSetBackPressure fail!\n");
			return ret;
		}
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int32 getBackPressureClearThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 minQueueThred = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;			
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);												
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureClearThredValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}

		minQueueThred = backPressure.MinQueueThreshold;
		put16(attributeValuePtr, minQueueThred);

#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);										
		ret = ponMacGetBackPressure(&backPressure, queueId);
	if(ret != 0)
	{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getBackPressureClearThredValue: ponMacGetBackPressure fail!\n");
		return ret;
	}

	minQueueThred = backPressure.MinQueueThreshold;
	put16(attributeValuePtr, minQueueThred);
#endif					
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setBackPressureClearThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint16 minQueueThred = 0;
	backPressure_t backPressure;
	uint32 relatePort = 0;
	uint8 port = 0;			
	memset(&backPressure,0, sizeof(backPressure_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);														
#ifdef TCSUPPORT_MT7530_SWITCH_API	
		ret = macMT7530GetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}

		minQueueThred = get16((uint8 *)value);
		backPressure.MinQueueThreshold= minQueueThred;
		ret = macMT7530SetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, port);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: macMT7530SetBackPressure fail!\n");
			return ret;
		}
#endif			
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);
	//	ret = ponMacGetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: ponMacGetBackPressure fail!\n");
			return ret;
		}

		minQueueThred = get16(value);
		backPressure.MinQueueThreshold= minQueueThred;
	//	ret = ponMacSetBackPressure(&backPressure, queueId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: ponMacSetBackPressure fail!\n");
			return ret;
		}					
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getPacketDropThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;
	uint16 greenPacketDropQueueMaxThr = 0;
	uint16 greenPacketDropQueueMinThr = 0;
	uint16 yellowPacketDropQueueMaxThr = 0;
	uint16 yellowPacketDropQueueMinThr = 0;
	uint16 congestQueueIdx = 0;
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
#ifdef TCSUPPORT_MT7530_SWITCH_API	
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getPacketDropThredValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}
		greenPacketDropQueueMaxThr = dropPolicy.greenPacketDropQueueMaxThr;
		greenPacketDropQueueMinThr = dropPolicy.greenPacketDropQueueMinThr;
		yellowPacketDropQueueMaxThr = dropPolicy.yellowPacketDropQueueMaxThr;
		yellowPacketDropQueueMinThr = dropPolicy.yellowPacketDropQueueMinThr;
		
		put16(attributeValuePtr, greenPacketDropQueueMaxThr);
		put16(attributeValuePtr+2, greenPacketDropQueueMinThr);
		put16(attributeValuePtr+4, yellowPacketDropQueueMaxThr);
		put16(attributeValuePtr+6, yellowPacketDropQueueMinThr);
		
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);		
		ret = getPonMacCongestThreshold(queueId,  &congestQueueIdx, &greenPacketDropQueueMaxThr, 
			&greenPacketDropQueueMinThr, &yellowPacketDropQueueMaxThr, &yellowPacketDropQueueMinThr);
		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getPacketDropThredValue: getPonMacCongestThreshold fail!\n");
			return ret;
		}
		
		put16(attributeValuePtr, greenPacketDropQueueMaxThr);
		put16(attributeValuePtr+2, greenPacketDropQueueMinThr);
		put16(attributeValuePtr+4, yellowPacketDropQueueMaxThr);
		put16(attributeValuePtr+6, yellowPacketDropQueueMinThr);
#endif
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setPacketDropThredValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;
	uint16 greenPacketDropQueueMaxThr = 0;
	uint16 greenPacketDropQueueMinThr = 0;
	uint16 yellowPacketDropQueueMaxThr = 0;
	uint16 yellowPacketDropQueueMinThr = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);				
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropThredValue: macMT7530GetBackPressure fail!\n");
			return ret;
		}

		greenPacketDropQueueMaxThr = get16((uint8 *)value);
		greenPacketDropQueueMinThr = get16((uint8 *)(value+2));
		yellowPacketDropQueueMaxThr = get16((uint8 *)(value+4));
		yellowPacketDropQueueMinThr = get16((uint8 *)(value+6));		

		dropPolicy.greenPacketDropQueueMaxThr = greenPacketDropQueueMaxThr;
		dropPolicy.greenPacketDropQueueMinThr = greenPacketDropQueueMinThr;
		dropPolicy.yellowPacketDropQueueMaxThr = yellowPacketDropQueueMaxThr;
		dropPolicy.yellowPacketDropQueueMinThr = yellowPacketDropQueueMinThr;

		ret = macMT7530SetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropThredValue: macMT7530SetDropPolicy fail!\n");
			return ret;
		}
#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);		

		greenPacketDropQueueMaxThr = get16((uint8 *)value);
		greenPacketDropQueueMinThr = get16((uint8 *)(value+2));
		yellowPacketDropQueueMaxThr = get16((uint8 *)(value+4));
		yellowPacketDropQueueMinThr = get16((uint8 *)(value+6));		

		ret = setCongestThredValue(queueId,  greenPacketDropQueueMaxThr, greenPacketDropQueueMaxThr, 
			greenPacketDropQueueMaxThr, yellowPacketDropQueueMinThr);
		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropThredValue: setCongestThredValue fail!\n");
			return ret;
		}
#endif					
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropThredValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getPacketDropMaxpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 greenPacketDropMax_p = 0;
	uint8 yellowPacketDropMax_p = 0;
	
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));


	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);						
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getPacketDropMaxpValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		greenPacketDropMax_p = dropPolicy.greenPacketDropMax_p;
		yellowPacketDropMax_p = dropPolicy.yellowPacketDropMax_p;
		*attributeValuePtr = greenPacketDropMax_p;	
		*(attributeValuePtr+1) = greenPacketDropMax_p;	
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 1	
		ret = getPonMacCongestDropProbality(&greenPacketDropMax_p, &yellowPacketDropMax_p);	
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getPacketDropMaxpValue: getPonMacCongestDropProbality fail!\n");
			return ret;
		}

		*attributeValuePtr = greenPacketDropMax_p;	
		*(attributeValuePtr+1) = yellowPacketDropMax_p;		
#endif
	}
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setPacketDropMaxpValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 greenPacketDropMax_p = 0;
	uint8 yellowPacketDropMax_p = 0;
	
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);								
#ifdef TCSUPPORT_MT7530_SWITCH_API					
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropMaxpValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		greenPacketDropMax_p = *value;
		yellowPacketDropMax_p = *(value+1);
		dropPolicy.greenPacketDropMax_p = greenPacketDropMax_p;
		dropPolicy.yellowPacketDropMax_p = yellowPacketDropMax_p;
		ret = macMT7530SetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropMaxpValue: macMT7530SetDropPolicy fail!\n");
			return ret;
		}
#endif	
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 1
		greenPacketDropMax_p = *value;
		yellowPacketDropMax_p = *(value+1);

		ret = setCongestDropProbality(greenPacketDropMax_p, yellowPacketDropMax_p);				
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropMaxpValue: setCongestDropProbality fail!\n");
			return ret;
		}				
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setPacketDropMaxpValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getQueueDropWqValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 queueDropW_q = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));


	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);										
#ifdef TCSUPPORT_MT7530_SWITCH_API			
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getQueueDropWqValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		queueDropW_q = dropPolicy.QueueDropW_q;
		*attributeValuePtr = queueDropW_q;

#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);								
		ret =  ponMacGetDropPolicy(port, queueId, &dropPolicy);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getQueueDropWqValue: ponMacGetDropPolicy fail!\n");
			return ret;
		}
		
		queueDropW_q = dropPolicy.QueueDropW_q;
		*attributeValuePtr = queueDropW_q;
#endif		
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setQueueDropWqValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 queueDropW_q = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);												
#ifdef TCSUPPORT_MT7530_SWITCH_API
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setQueueDropWqValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		queueDropW_q = *value;
		dropPolicy.QueueDropW_q= queueDropW_q;
		ret = macMT7530SetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setQueueDropWqValue: macMT7530SetDropPolicy fail!\n");
			return ret;
		}
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);												

		ret =  ponMacGetDropPolicy(port, queueId, &dropPolicy);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setQueueDropWqValue: ponMacGetDropPolicy fail!\n");
			return ret;
		}

		queueDropW_q = *value;
		dropPolicy.QueueDropW_q= queueDropW_q;
		ret =  ponMacSetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setQueueDropWqValue: ponMacSetDropPolicy fail!\n");
			return ret;
		}
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureOccurThredValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int32 getDropPrecedenceColorMarkValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 dropPrecdenceColourMarking = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);														
#ifdef TCSUPPORT_MT7530_SWITCH_API
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getDropPrecedenceColorMarkValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		dropPrecdenceColourMarking = dropPolicy.DropPrecdenceColourMarking;
		*attributeValuePtr = dropPrecdenceColourMarking;
#endif

	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);												
		ret =  ponMacGetDropPolicy(port, queueId, &dropPolicy);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getDropPrecedenceColorMarkValue: ponMacGetDropPolicy fail!\n");
			return ret;
		}

		dropPrecdenceColourMarking = dropPolicy.dropPrecdenceColourMarking;
		*attributeValuePtr = dropPrecdenceColourMarking;
#endif					
	}
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setDropPrecedenceColorMarkValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint16 meId= 0;
	uint8 queueId = 0;
	int ret = 0;
	uint8 dropPrecdenceColourMarking = 0;
	dropPolicy_t dropPolicy;
	uint32 relatePort = 0;
	uint8 port = 0;	
	
	memset(&dropPolicy,0, sizeof(dropPolicy_t));


	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , "Related port", &length);
	relatePort =  get32(attributeValuePtr);
	port = (uint8)((relatePort >> 16) & 0x00ff);		

	if(meId < MIN_TONT_MEID)
	{
		//downstream priority queue
		/*wait for switch api*/	
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		
#ifdef TCSUPPORT_MT7530_SWITCH_API	
		ret =  macMT7530GetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setDropPrecedenceColorMarkValue: macMT7530GetDropPolicy fail!\n");
			return ret;
		}

		dropPrecdenceColourMarking = *value;
		dropPolicy.DropPrecdenceColourMarking= dropPrecdenceColourMarking;
		ret = macMT7530SetDropPolicy(port, queueId, (mt7530_switch_DropPolicy_t *)(&dropPolicy));		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setDropPrecedenceColorMarkValue: macMT7530SetDropPolicy fail!\n");
			return ret;
		}
#endif			
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
#if 0
		queueId = (uint8)((meId & 0x00ff)%NUM_OF_QUEUE_PER_TCONT);		
		ret = ponMacGetDropPolicy(port, queueId, &dropPolicy);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setDropPrecedenceColorMarkValue: ponMacGetDropPolicy fail!\n");
			return ret;
		}
		
		dropPrecdenceColourMarking = *value;
		dropPolicy.DropPrecdenceColourMarking= dropPrecdenceColourMarking;
		ret = ponMacSetDropPolicy(port, queueId, &dropPolicy);		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setDropPrecedenceColorMarkValue: ponMacSetDropPolicy fail!\n");
			return ret;
		}
#endif		
	}	

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setBackPressureClearThredValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int omciInternalCreateForPriorityQueue(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	uint8 buffer[25] = {0};
	uint8 portId = 0;
	int ret = 0;
	uint8 queueId = 0;
	backPressure_t backPressure;
	uint16 maxQueueSize = 0;
	uint16 allocQueueSize = 0;
	uint16 priority = 0;

	uint8 qosType = 0;
	uint8 qosChannel = 0;
	uint8 ponWeight[NUM_OF_QUEUE_PER_TCONT] = {0};	
	
	queueId = (uint8)((instanceId & 0x00ff)%NUM_OF_QUEUE_PER_UIN_PORT);		

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p, tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName, &length);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciInternalCreateForPriorityQueue: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*Queue configuration operation*/	
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		*attributeValuePtr=1; // 1:several queue share one max queue size; 0:each queue has an individual max queue size

	/*Max Queue size*/	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		maxQueueSize = 0xffff;
		if(instanceId < MIN_TONT_MEID)
		{
			//downstream priority queue
#ifdef TCSUPPORT_MT7530_SWITCH_API		
	//		ret = macMT7530SetMaximumQueueSize(maxQueueSize, queueId);//not support
#endif
		}
		else
		{
			//upstream priority queue
			//wait for pon mac api
//			ret = ponMacSetMaximumQueueSize(maxQueueSize, queueId);
			
		}
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: set maxQueueSize fail!\n");		
			return -1;	
		}
		put16(attributeValuePtr, maxQueueSize);
	}

	/*Allocated Queue size*/			
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		allocQueueSize = 4;
		if(instanceId < MIN_TONT_MEID)
		{
			//downstream priority queue
#ifdef TCSUPPORT_MT7530_SWITCH_API					
			ret = macMT7530SetAllocatedQueueSize(allocQueueSize, queueId);
#endif
		}
		else
		{
			//upstream priority queue
			//wait for pon mac api
//			ret =  ponMacSetAllocatedQueueSize(allocQueueSize, queueId);
			
		}		
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: set allocQueueSize fail!\n");		
			return -1;	
		}	
		put16(attributeValuePtr, allocQueueSize);
	}

	/*Queue priority*/			
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		if(instanceId < MIN_TONT_MEID)
		{
			//downstream priority queue
			portId = instanceId/NUM_OF_QUEUE_PER_UIN_PORT + 1;
			priority = instanceId%NUM_OF_QUEUE_PER_UIN_PORT;
			put16(attributeValuePtr, (0x100 + portId)); //set first two bytes, PPTP ** UNI meId
			put16(attributeValuePtr+2, priority); //set priority
#ifdef TCSUPPORT_MT7530_SWITCH_API					
			ret = macMT7530SetQueuePriority(priority, queueId);
#endif	
		}
		else
		{
			//upstream priority queue
			portId = (instanceId-MIN_TONT_MEID)/NUM_OF_QUEUE_PER_TCONT; // T-CONT portID 1~15	
			priority = (instanceId-MIN_TONT_MEID)%NUM_OF_QUEUE_PER_TCONT;
			put16(attributeValuePtr, (0x8000 + portId)); //set first two bytes, T-CONT meId
			put16(attributeValuePtr+2, (7-(priority&0x07))); //set priority			
			//wait for pon mac api
	//		ret = ponMacSetQueuePriority(priority, queueId);
			
		}			
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: set priority fail!\n");		
			return -1;	
		}	

	}

	/*traffic scheduler pointer*/			
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0); //set traffic scheduler pointer,default 0
	}	

	/*weight*/			
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr =  1; //set weight,default 1
		if(instanceId < MIN_TONT_MEID)
		{
			//downstream priority queue
#ifdef TCSUPPORT_MT7530_SWITCH_API		
			ret = macMT7530SetWeight(1, queueId, portId);
#endif
		}
		else
		{
			//upstream priority queue
			//wait for pon mac api
#if 0
			ret = getPonMacQOSParam(portId, &qosChannel, &qosType, ponWeight);
			if(ret != 0)
			{		
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: getPonMacQOSParam fail!\n");
				return -1;
			}
			ponWeight[queueId] = 1;
			
			ret = setPonMacQOSParam(qosChannel, qosType, ponWeight);
			if(ret != 0)
			{		
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: setPonMacQOSParam fail!\n");
					return -1;
			}
#endif				
		}
	}	

#if 1
	attriMask = 1<<7;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0 ); 
	}	

	attriMask = 1<<6;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put32(attributeValuePtr, 0 ); 
	}	
	
	attriMask = 1<<5;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr,  0xffff ); 
	}	
	
	attriMask = 1<<4;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0 ); 
	}	

	if(instanceId < MIN_TONT_MEID)
	{
		//downstream priority queue
		memset(&backPressure, 0, sizeof(backPressure_t));
		backPressure.Enable = 1;    //default value is enable;
		backPressure.time = 0;
		backPressure.MaxQueueThreshold = 0xffff;
		backPressure.MinQueueThreshold = 0;
#ifdef TCSUPPORT_MT7530_SWITCH_API				
		ret = macMT7530SetBackPressure((mt7530_switch_api_backPressure_t *)(&backPressure), queueId, portId);
#endif
	}
	else
	{
		//upstream priority queue
		//wait for pon mac api
		//downstream priority queue
		memset(&backPressure, 0, sizeof(backPressure_t));
		backPressure.Enable = 0;
		backPressure.time = 0;
		backPressure.MaxQueueThreshold = 0xffff;
		backPressure.MinQueueThreshold = 0;
		
//		ret = ponMacSetBackPressure(&backPressure, queueId);		
	}			
#if 0		
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"omciInternalCreateForPriorityQueue: macMT7530SetBackPressure fail!\n");
		return ret;
	}
#endif
#endif

//	attribute 13~16 not support now


	return 0;
}


int omciInitInstForPriorityQueue(void)
{
	int ret = 0;
	int i = 0;
	int j = 0;


	/*init upstream priority queue*/
	for(i=0; i<(MAX_TCONT_NUM); i++)
	{
		for(j=0; j<NUM_OF_QUEUE_PER_TCONT; j++)
		{
			if(isFiberhome_551601){
				ret = omciInternalCreateForPriorityQueue(OMCI_CLASS_ID_PRIORITY_QUEUE, (((i*NUM_OF_QUEUE_PER_TCONT+j)<<8)+MIN_TONT_MEID));/*internal create func*/
			} 
			else{
				ret = omciInternalCreateForPriorityQueue(OMCI_CLASS_ID_PRIORITY_QUEUE, (i*NUM_OF_QUEUE_PER_TCONT+j+MIN_TONT_MEID));/*internal create func*/
			}
	
			if (ret == -1)
			{
				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForPriorityQueue: create OMCI_CLASS_ID_PRIORITY_QUEUE fail\n");
				return ret;
			}
		}
	}
    
#if defined(TCSUPPORT_HGU_OMCI_MIB_UPLOAD_PPTP_ETH_UNI)
    if (HGU != pSystemCap->onuType) {
#endif
    	/*init downstream priority queue*/	
    	for(i=0; i<(pSystemCap->geNum + pSystemCap->feNum); i++)
    	{
    		for(j=0; j<NUM_OF_QUEUE_PER_UIN_PORT; j++)
    		{
    			ret = omciInternalCreateForPriorityQueue(OMCI_CLASS_ID_PRIORITY_QUEUE, (i*NUM_OF_QUEUE_PER_UIN_PORT+j));/*internal create func*/
    			if (ret == -1)
    			{
    				omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInstForPriorityQueue create OMCI_CLASS_ID_PRIORITY_QUEUE fail\n");
    				return ret;
    			}
    		}
    	}
#if defined(TCSUPPORT_HGU_OMCI_MIB_UPLOAD_PPTP_ETH_UNI)
    }
#endif
	return ret;
}

int omciMeInitForPriorityQueue(omciManageEntity_t *omciManageEntity_p){
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

//	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by other ME
//	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by other ME
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

/*******************************************************************************************************************************
9.2.11 Traffic scheduler

********************************************************************************************************************************/
int32 setTcontPointValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciManageEntity_t *currentMe = NULL;
	omciMeInst_t *currentMeInst_p = NULL;	
	uint16 qosCfg = 0;
    uint16 tconId = 0;
	currentMe = omciGetMeByClassId(OMCI_CLASS_ID_ONU2_G);
	currentMeInst_p = omciGetInstanceByMeId(currentMe, 0);
	attributeValuePtr = omciGetInstAttriByName(currentMeInst_p , "QoS configuration", &length);
	
	qosCfg = get16(attributeValuePtr);
	if((qosCfg & 0x0004) == 0)
	{
		return OMCI_PARAM_ERROR;
	}

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTcontPointValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 setTcontPolicyValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciManageEntity_t *currentMe = NULL;
	omciMeInst_t *currentMeInst_p = NULL;	
	uint16 qosCfg = 0;
	uint16 tcont_p = 0;
	uint8 tcontId = 0;

	int ret=0;
	currentMe = omciGetMeByClassId(OMCI_CLASS_ID_ONU2_G);
	currentMeInst_p = omciGetInstanceByMeId(currentMe, 0);
	attributeValuePtr = omciGetInstAttriByName(currentMeInst_p , "QoS configuration", &length);
	qosCfg = get16(attributeValuePtr);
	if((qosCfg & 0x0008) == 0)
	{
		return OMCI_PARAM_ERROR;
	}
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{	    
        /* Set Traffic Scheduler Attribute*/
        memcpy(attributeValuePtr, value, length);
        /* PON MAC QOS Policy set*/
		attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, "T-CONT pointer", &length);
		memcpy((char *)&tcont_p,attributeValuePtr,length);
		tcontId = (uint8)(tcont_p & 0x00ff);
		ret=setQOSPolicy(tcontId,*value,1);
        if(ret != 0)
            omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTcontPolicyValue: faile\n");

		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTcontPolicyValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 setTcontPriorityWeightValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTcontPriorityWeightValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int omciInternalCreateForTrafficScheduler(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	uint8 buffer[25] = {0};
	uint8 portId = 0;
	int ret = 0;
	backPressure_t backPressure;
	uint16 maxQueueSize = 0;
	uint16 allocQueueSize = 0;
	

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciInternalCreateForTrafficScheduler: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if(attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*T-CONT pointer*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		put16(attributeValuePtr, instanceId);

	/*Traffic scheduler pointer*/	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0);
	}
	
	/*policy*/		
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 1; /* default SP */
	}

	/*priority or weight*/		
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		*attributeValuePtr = 0;
	}

	return 0;
}

#ifdef TCSUPPORT_CHS
/*****************************************************************************
*function :
*		omciInternalCreateForTrafficDescriptor
*description : 
*		this function is used to create a traffic descriptor ME instance
*input :	
*		classId 	: ME class ID
*		instanceId	: ME instance ID
*output :
*		N/A
******************************************************************************/
int omciInternalCreateForTrafficDescriptor(uint16 classId, uint16 instanceId)
{
	omciManageEntity_t *tmpOmciManageEntity_p = NULL;
	omciMeInst_t *tmpomciMeInst_p = NULL;
	uint8 * attributeValuePtr = NULL;
	uint16 length = 0;
	uint16 attriMask = 0;
	int ret = 0;	

	tmpOmciManageEntity_p = omciGetMeByClassId(classId);
	tmpomciMeInst_p = omciAllocInstance(tmpOmciManageEntity_p);
	omciAddInstanceToTail(tmpOmciManageEntity_p, tmpomciMeInst_p);

	attributeValuePtr = tmpomciMeInst_p->attributeVlaue_ptr;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"omciInternalCreateForTrafficDescriptor: tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName = %s\n",  
		tmpOmciManageEntity_p->omciAttriDescriptList[0].attriName);
	if(attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, instanceId);
	}

	/*CIR*/
	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0);
	}

	/*PIR*/	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
	{
		put16(attributeValuePtr, 0);
	}

	return 0;
}

/*****************************************************************************
*function :
*		omciInitInstForTrafficDescriptor
*description : 
*		this function is used to init a traffic descriptor ME instance
*input :	
*		N/A
*output :
*		N/A
******************************************************************************/
int omciInitInstForTrafficDescriptor(void)
{
	int ret = 0;
	uint16 instranceId = 0xFFFF;
	
	ret = omciInternalCreateForTrafficDescriptor(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR, instranceId);/*internal create func*/
	if (ret == -1)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_TRAFFIC_SCHEDULER fail\n");
		return ret;
	}
}
#endif

int omciInitInstForTrafficScheduler(void)
{
	int ret = 0;
	int i = 0;
	
	/*create 15*8 traffic scheduler ME instance*/
	for(i=((GPON_SLOT<<8) + 0); i<((GPON_SLOT<<8) + MAX_TCONT_NUM); i++)
	{
		ret = omciInternalCreateForTrafficScheduler(OMCI_CLASS_ID_TRAFFIC_SCHEDULER, i);/*internal create func*/
		if (ret == -1)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciInitInst create OMCI_CLASS_ID_TRAFFIC_SCHEDULER fail\n");
			return ret;
		}
	}

	return ret;
}

int omciMeInitForTrafficScheduler(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by other ME
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by other ME
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}


/*******************************************************************************************************************************
9.2.12 Traffic descriptor

********************************************************************************************************************************/
#if 0  //remove now, may be used in future

int getTrafficDescriptorValue(trafficDescriptor_t *trafficDescriptor, uint16 meId)
{
	int ret = 0;
	int i=0;
	trafficDesPortList_t trafficDesPortList;

	memset(&trafficDesPortList, 0, sizeof(trafficDesPortList_t));	
	
	
	/*seach port from Mac Bridge ME*/
//	ret = getPortByTrafficDesInst(&trafficDesPortList, meId);
	if(ret == 0)
	{
		if(trafficDesPortList.portNum != 0)
		{
			if(trafficDesPortList.ifaceType[0] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[0]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API	
				//	ret = macMT7530GetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API								
				//	ret = macMT7530GetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: macMT7530GetTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[0]  == LAN_INTERFACE)
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: ponMacGetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		
		}

	}
	else
	{
		/*seach port from Gem Port CTP ME*/
	//	ret = getPortFromGemPortByInstId(&trafficDesPortList, meId);	
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getPortFromGemPortByInstId fail!\n");
			return ret;
		}

	//	call PON MAC API
		if(trafficDesPortList.portNum != 0)
		{
			if(trafficDesPortList.ifaceType[0] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[0]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API					
				//	ret = macMT7530GetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530GetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: macMT7530GetTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[0]  == TXDIRECTION)
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[0], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: ponMacGetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		}
	}

	return 0;
}

int setTrafficDescriptorValue(trafficDescriptor_t *trafficDescriptor, uint16 meId)
{
	int ret = 0;
	int i=0;
	trafficDesPortList_t trafficDesPortList;

	memset(&trafficDesPortList, 0, sizeof(trafficDesPortList_t));	

	
	/*seach port from Mac Bridge ME*/
//	ret = getPortByTrafficDesInst(&trafficDesPortList, meId);
	if(ret == 0)
	{
		for(i=0; i<trafficDesPortList.portNum; i++)
		{
			if(trafficDesPortList.ifaceType[i] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530SetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530SetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorValue: macMT7530SetTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[i]  == LAN_INTERFACE)
				{
				//	ret = ponMacSetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacSetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorValue: ponMacSetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		
		}

	}

		memset(&trafficDesPortList, 0, sizeof(trafficDesPortList_t));	

		/*seach port from Gem Port CTP ME*/
	//	ret = getPortFromGemPortByInstId(&trafficDesPortList, meId);	
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorValue: getPortFromGemPortByInstId fail!\n");
			return ret;
		}

	//	call PON MAC API
		for(i=0; i<trafficDesPortList.portNum; i++)
		{
			if(trafficDesPortList.ifaceType[i] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530SetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530SetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorValue: macMT7530SetTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacGetTrafficDescriptor(trafficDescriptor, trafficDesPortList.portId[i], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: ponMacGetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		}

	return 0;
}

int clearTrafficDescriptorValue(uint16 meId)
{
	int ret = 0;
	int i=0;
	trafficDesPortList_t trafficDesPortList;

	memset(&trafficDesPortList, 0, sizeof(trafficDesPortList_t));	

	
	/*seach port from Mac Bridge ME*/
//	ret = getPortByTrafficDesInst(&trafficDesPortList, meId);
	if(ret == 0)
	{
		for(i=0; i<trafficDesPortList.portNum; i++)
		{
			if(trafficDesPortList.ifaceType[i] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530CleanTrafficDescriptor(trafficDesPortList.portId[i], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530CleanTrafficDescriptor(trafficDesPortList.portId[i], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"clearTrafficDescriptorValue: macMT7530CleanTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[i]  == LAN_INTERFACE)
				{
				//	ret = ponMacCleanTrafficDescriptor(trafficDesPortList.portId[i], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacCleanTrafficDescriptor(trafficDesPortList.portId[i], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"clearTrafficDescriptorValue: ponMacSetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		
		}

	}

		memset(&trafficDesPortList, 0, sizeof(trafficDesPortList_t));	

		/*seach port from Gem Port CTP ME*/
	//	ret = getPortFromGemPortByInstId(&trafficDesPortList, meId);	
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"clearTrafficDescriptorValue: getPortFromGemPortByInstId fail!\n");
			return ret;
		}

	//	call PON MAC API
		for(i=0; i<trafficDesPortList.portNum; i++)
		{
			if(trafficDesPortList.ifaceType[i] == LAN_INTERFACE)
			{
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530CleanTrafficDescriptor(trafficDesPortList.portId[i], TXDIRECTION);
#endif				
				}
				else
				{
#ifdef TCSUPPORT_MT7530_SWITCH_API				
				//	ret = macMT7530CleanTrafficDescriptor(trafficDesPortList.portId[i], RXDIRECTION);
#endif				
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"clearTrafficDescriptorValue: macMT7530SetTrafficDescriptor fail!\n");
					return ret;
				}
			}
			else
			{
			//	call PON MAC API	
				if(trafficDesPortList.direction[i]  == TXDIRECTION)
				{
				//	ret = ponMacCleanTrafficDescriptor(trafficDesPortList.portId[i], TXDIRECTION);
				}
				else
				{
				//	ret = ponMacCleanTrafficDescriptor(trafficDesPortList.portId[i], RXDIRECTION);
				}
				
				if(ret != 0)
				{
					omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"clearTrafficDescriptorValue: ponMacGetTrafficDescriptor fail!\n");
					return ret;
				}			
			}		
		}

	return 0;
}



int32 getTrafficDescriptorCIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 CIR = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	CIR = trafficDescriptor.CIR;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	put32(attributeValuePtr, CIR);
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptorCIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 CIR = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		CIR = get32(value);
		trafficDescriptor.CIR = CIR;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCIRValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCIRValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int32 getTrafficDescriptorPIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 PIR = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	PIR = trafficDescriptor.PIR;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	put32(attributeValuePtr, PIR);
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptorPIRValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 PIR = 0;

	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));


	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPIRValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		PIR = get32(value);
		trafficDescriptor.PIR = PIR;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPIRValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPIRValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int32 getTrafficDescriptorCBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 CBS = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	CBS = trafficDescriptor.CBS;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	put32(attributeValuePtr, CBS);
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptorCBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 CBS = 0;
	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));


	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCBSValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		CBS = get32(value);
		trafficDescriptor.CBS = CBS;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCBSValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}

	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCBSValue: attributeValuePtr == NULL\n");
		return -1;
	}
}


int32 getTrafficDescriptorPBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 PBS = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	PBS = trafficDescriptor.PBS;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	put32(attributeValuePtr, PBS);
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptorPBSValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint32 PBS = 0;
	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPBSValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		PBS = get32(value);
		trafficDescriptor.PBS = PBS;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPBSValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}
	
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorPBSValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getTrafficDescriptorColourModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 colourMode = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	colourMode= trafficDescriptor.colourMode;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	*attributeValuePtr = colourMode;
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}


int32 setTrafficDescriptorColourModeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 colourMode = 0;

	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorColourModeValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		colourMode =  (uint8)(*value);
		trafficDescriptor.colourMode = colourMode;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorColourModeValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}		
	
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorColourModeValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getTrafficDescriptoraInMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 inColourMark = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}

	inColourMark= trafficDescriptor.ingressColourMarking;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	*attributeValuePtr = inColourMark;
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptoraInMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 inColourMark= 0;

	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraInMarkingValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		inColourMark =  (uint8)(*value);
		trafficDescriptor.ingressColourMarking = inColourMark;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraInMarkingValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}
	
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraInMarkingValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getTrafficDescriptoraEnMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 enColourMark = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}



	enColourMark= trafficDescriptor.engressColourMarking;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	*attributeValuePtr = enColourMark;
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}

int32 setTrafficDescriptoraEnMarkingValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 EnColourMark= 0;

	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraEnMarkingValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		EnColourMark =  (uint8)(*value);
		trafficDescriptor.engressColourMarking = EnColourMark;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraEnMarkingValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}
	
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptoraEnMarkingValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int32 getTrafficDescriptorMeterTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint16 meId = 0;
	uint8 meterType = 0;
	trafficDescriptor_t trafficDescriptor;
	
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	
	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

	ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
	if(ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getTrafficDescriptorCIRValue: getTrafficDescriptorValue fail!\n");
		return ret;	
	}



	meterType= trafficDescriptor.meterType;
	
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	*attributeValuePtr = meterType;
	
	return getTheValue(value, (char *)attributeValuePtr, (uint8)length, omciAttribute);	
}


int32 setTrafficDescriptorMeterTypeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint8 portId = 0;
	uint8 ifaceType = 0;
	uint16 meId = 0;
	uint8 meterType = 0;
	uint16 attriMask = 0;
	uint8 direction = 0;

	trafficDescriptor_t trafficDescriptor;
	memset(&trafficDescriptor,0, sizeof(trafficDescriptor_t));

	if(flag != OMCI_CREATE_ACTION_SET_VAL)
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		ret = getTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCBSValue: getTrafficDescriptorValue fail!\n");
			return ret;	
		}
		meterType =  (uint8)(*value);
		trafficDescriptor.meterType = meterType;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorCBSValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}		

	}
	else
	{
		meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);

		attriMask = 1<<15;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.CIR = get32(attributeValuePtr);

		attriMask = 1<<14;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.PIR = get32(attributeValuePtr);

		attriMask = 1<<13;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.CBS = get32(attributeValuePtr);	

		attriMask = 1<<12;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.PBS= get32(attributeValuePtr);

		attriMask = 1<<11;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.colourMode= *attributeValuePtr;

		attriMask = 1<<10;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.ingressColourMarking= *attributeValuePtr;

		attriMask = 1<<9;
		attributeValuePtr = omciGetInstAttriByMask(tmpomciMeInst_p, attriMask, &length);
		if (attributeValuePtr != NULL)
			trafficDescriptor.engressColourMarking= *attributeValuePtr;

		
		meterType = (uint8)(*value);
		trafficDescriptor.meterType = meterType;

		ret = setTrafficDescriptorValue(&trafficDescriptor, meId);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorMeterTypeValue: setTrafficDescriptorValue fail!\n");
			return ret;	
		}	
		
	}
	
	attributeValuePtr = omciGetInstAttriByName((omciMeInst_ptr)meInstantPtr, omciAttribute->attriName, &length);
	if (attributeValuePtr != NULL)
	{
		memcpy(attributeValuePtr, value, length);
		return 0;
	}
	else
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setTrafficDescriptorMeterTypeValue: attributeValuePtr == NULL\n");
		return -1;
	}
}

int omciDeleteTrafficDesAction(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc){
	int ret = 0;
	uint16 instanceId = 0;
	uint8 *tmp = NULL;

	ret = omciDeleteAction(meClassId, meInstant_ptr, omciPayLoad, msgSrc);

	if(ret == 0)
	{

		tmp = (uint8 *)&omciPayLoad->meId;
		instanceId = get16(tmp+2);
		
		ret = clearTrafficDescriptorValue(instanceId);
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR, "omciDeleteTrafficDesAction: clearTrafficDescriptorValue fail \n");		
		}
	}

	return ret;

}
#endif
/*******************************************************************************************************************************

9.2.12 traffic descriptor  API  (Kenny)

********************************************************************************************************************************/
/*******************************************************************************************
**function name
	setBoundTD
**description:
	set Bound traffic descriptor to limit the trafic
 **retrun :
 	-1:	failure
 	0: 	success
**parameter:
  	portId: lan port id or wan port id
  	ifcType: 0:lan interface, 1: wan interface
  	tdInstId: 9.2.12 traffic descriptor instance id
  	dir: 1:in bound  0:out bound
********************************************************************************************/
int setBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir){
	omciManageEntity_t *currentME = NULL;
	omciMeInst_t *meInst_p = NULL;
	int ret = 0;
	uint16 attriMask = 0;
	uint8 *attributeValuePtr = NULL;
	uint16 length = 0;
	trafficDescriptor_t trafficDescriptor;
	
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n setBoundTD API");
	memset(&trafficDescriptor, 0, sizeof(trafficDescriptor_t));
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR);
	if(currentME == NULL)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "setInBoundTD: the ME %x is not exist\n", OMCI_CLASS_ID_TRAFFIC_DESCRIPTOR);	
		return -1;	
	}
	
	meInst_p = omciGetInstanceByMeId(currentME , tdInstId);
	if(meInst_p == NULL){
		omcidbgPrintf(OMCI_DEBUG_LEVEL_WARN, "setInBoundTD: the instance %x is not exist\n", tdInstId);	
		return -1;
	}

	attriMask = 1<<15;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.CIR = get32(attributeValuePtr);
	
	attriMask = 1<<14;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.PIR = get32(attributeValuePtr);
	
	attriMask = 1<<13;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.CBS = get32(attributeValuePtr);	
	
	attriMask = 1<<12;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.PBS= get32(attributeValuePtr);
	
	attriMask = 1<<11;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.colourMode= *attributeValuePtr;
	
	attriMask = 1<<10;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.ingressColourMarking= *attributeValuePtr;
	
	attriMask = 1<<9;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.engressColourMarking= *attributeValuePtr;
	
	attriMask = 1<<8;
	attributeValuePtr = omciGetInstAttriByMask(meInst_p, attriMask, &length);
	if (attributeValuePtr != NULL)
		trafficDescriptor.meterType = *attributeValuePtr;
	
	trafficDescriptor.direction = dir;//1:RX  0:TX

	if(ifcType == 0)
	{
#ifdef TCSUPPORT_MT7530_SWITCH_API					
		ret = macMT7530SetTrafficDescriptor(portId, (mt7530_switch_api_trafficDescriptor_t *)(&trafficDescriptor));
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setInBoundTD: macMT7530SetTrafficDescriptor fail!\n");
			return ret;	
		}
#endif		
	}
	else
	{
		//wait for PON API	,still some problem
		ret = setPonMacTrafficDescriptor(portId, &trafficDescriptor);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"setInBoundTD: setPonMacTrafficDescriptor fail!\n");
			return ret;	
		}		
	}
	
	return 0;
}

int cleanBoundTD(IN uint8 portId, IN uint8 ifcType, IN uint16 tdInstId, IN uint8 dir){
	int ret = 0;
	
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"\r\n cleanBoundTD API");

	if(ifcType == 0)
	{
#ifdef TCSUPPORT_MT7530_SWITCH_API					
		ret = macMT7530CleanTrafficDescriptor(portId, dir);
		if(ret != 0)
		{
			omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"cleanBoundTD: macMT7530CleanTrafficDescriptor fail!\n");
			return ret;	
		}
#endif		
	}
	else
	{
		//wait for PON API
	}
	
	return 0;
}

int omciDeleteActionForTrafficDescriptor(uint16 meClassId , omciMeInst_t *meInstant_ptr, omciPayload_t * omciPayLoad, uint8 msgSrc)
{
	uint8 *attributeValuePtr = NULL;
	uint16 meId = 0;
	omciManageEntity_ptr currentME = NULL;
	omciMeInst_ptr meInstantCurPtr = NULL;
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstant_ptr;
	uint16 attrMask = 0;
	uint16 length = 0;
	uint16 tdPointerUpInstanceId = 0;
	uint16 tdPointerDownInstanceId = 0;
	uint16 gemPortId = 0;

	meId = get16(tmpomciMeInst_p->attributeVlaue_ptr);
	currentME = omciGetMeByClassId(OMCI_CLASS_ID_GEM_PORT_CTP);
	for (meInstantCurPtr = currentME->omciInst.omciMeInstList; meInstantCurPtr != NULL; meInstantCurPtr = meInstantCurPtr->next)
	{
		/*1------Gem port Id*/
		attrMask = 1<<15;
		attributeValuePtr = omciGetInstAttriByMask(meInstantCurPtr, attrMask, &length);
		if (attributeValuePtr != NULL)
			gemPortId = get16(attributeValuePtr);

		/*5------TDPointerUp	*/
		attrMask = 1<<11;
		attributeValuePtr = omciGetInstAttriByMask(meInstantCurPtr, attrMask, &length);
		if (attributeValuePtr != NULL)
		{	
			tdPointerUpInstanceId = get16(attributeValuePtr);
		}

		if (tdPointerUpInstanceId == meId)
			setTrtcmAva4GemportMapping(gemPortId);
		
		/*9------TDPointerDown	*/
		attrMask = 1<<7;
		attributeValuePtr = omciGetInstAttriByMask(meInstantCurPtr, attrMask, &length);
		if (attributeValuePtr != NULL)
		{	
			tdPointerDownInstanceId = get16(attributeValuePtr);
		}

		if (tdPointerDownInstanceId == meId)
			setAvaDsTrtcmTable(gemPortId);		
	}
	
	return omciDeleteAction(meClassId , (omciMeInst_t *)meInstant_ptr, omciPayLoad, msgSrc);
}

int omciMeInitForTrafficDescriptor(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by other ME
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteActionForTrafficDescriptor;//delete by other ME
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

/*******************************************************************************************************************************
9.2.13 GEM port network CTP  PM

********************************************************************************************************************************/

int32 setGemPortCtpPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
	uint32 thresholdData = 0;
	uint16 thresholdMeId = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

#if 0
	thresholdMeId = get16(attributeValuePtr);
	ret = getThresholdDataValue(OMCI_CLASS_ID_THRESHOLD_DATA1,thresholdMeId, 1, &thresholdData);
	if (ret != 0)
		return -1;

	sprintf(tempBuffer, "%x", thresholdData);
	ret = pmmgrTcapiSet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, PMMGR_THRESHOLD, 4, tempBuffer);
	if (ret != 0)
		return -1;	
#endif

	return 0;	
}


int32 getGemPortCtpPmTransmitFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmTransmitFramesValue: come into getGEMPortCTPPMTransmitFramesValue!\n");

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 1, tempBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getGemPortCtpPmTransmitFramesValue: pmmgrTcapiGet fail!\n");
		return -1;
	}

	sscanf(tempBuffer, "%x", &pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmTransmitFramesValue: pmData = %d\n",pmData);

	put32(value, pmData);

	return 0;
}

int32 getGemPortCtpPmReceiveFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmReceiveFramesValue: come into getGEMPortCTPPMReceiveFramesValue!\n");

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 2, tempBuffer);
	if (ret != 0)
	{
		omcidbgPrintf(OMCI_DEBUG_LEVEL_ERROR,"getGemPortCtpPmReceiveFramesValue: pmmgrTcapiGet fail!\n");
		return -1;
	}

	sscanf(tempBuffer, "%x", &pmData);
	put32(value, pmData);
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmReceiveFramesValue: pmData = %d\n",pmData);	

	return 0;
}

int32 getGemPortCtpPmReceiveBytesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint64 pmData = 0;

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 3, tempBuffer);
	if (ret != 0)
		return -1;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmReceiveBytesValue: tempBuffer = %s\n", tempBuffer);

	sscanf(tempBuffer, "%llx", &pmData);
	put64(value, pmData);

	return 0;
}


int32 getGemPortCtpPmTransmitBytesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint64 pmData = 0;

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 4, tempBuffer);
	if (ret != 0)
		return -1;
	omcidbgPrintf(OMCI_DEBUG_LEVEL_TRACE,"getGemPortCtpPmTransmitBytesValue: tempBuffer = %s\n", tempBuffer);

	sscanf(tempBuffer, "%llx", &pmData);
	put64(value, pmData);

	return 0;
}


int omciMeInitForGemportNetworkCtpPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;//create by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;//delete by OLT
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}

/*******************************************************************************************************************************
9.2.13 Energy consumption performance monitoring history data

********************************************************************************************************************************/
int32 setEnergyComsumPmThrdDataValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute,uint8 flag)
{
	uint8 *attributeValuePtr = NULL;
	uint16 length;
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	int ret = 0;
//	uint32 thresholdData = 0;
//	uint16 thresholdMeId = 0;

	if((value == NULL) || (meInstantPtr == NULL))
		return -1;

	tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	attributeValuePtr = omciGetInstAttriByName(tmpomciMeInst_p , omciAttribute->attriName, &length);
	if(attributeValuePtr == NULL)
		return -1;
	memcpy(attributeValuePtr, value, length);

	return 0;	
}


int32 getEnergyComsumPmTransmitFramesValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 1, tempBuffer);
	if (ret != 0)
		return -1;

	sscanf(tempBuffer, "%x", &pmData);
	put32(value, pmData);

	return 0;
}

int32 getEnergyComsumPmCyclicSleepTimeValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 2, tempBuffer);
	if (ret != 0)
		return -1;

	sscanf(tempBuffer, "%x", &pmData);
	put32(value, pmData);

	return 0;
}

int32 getEnergyComsumPmEnergyConsumedValue(char *value, uint8 *meInstantPtr, omciAttriDescript_ptr omciAttribute, uint8 flag)
{
	char tempBuffer[MAX_BUFFER_SIZE] = {0};
	omciMeInst_t *tmpomciMeInst_p = (omciMeInst_ptr)meInstantPtr;
	uint8 dataFlag = 0;
	int ret = 0;
	uint32 pmData = 0;

	if (value == NULL)
		return -1;
	
	if (flag == 0)
		dataFlag = 0;
	else 
		dataFlag = 1;
		
	ret = pmmgrTcapiGet( tmpomciMeInst_p->classId, tmpomciMeInst_p->deviceId, dataFlag, 3, tempBuffer);
	if (ret != 0)
		return -1;

	sscanf(tempBuffer, "%x", &pmData);
	put32(value, pmData);

	return 0;
}

int omciMeInitForEnergyConsumPM(omciManageEntity_t *omciManageEntity_p)
{
	omciManageEntity_ptr tmpOmciManageEntity_p = omciManageEntity_p;

	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_CREAT] = omciCreateAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_DELETE] = omciDeleteAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET] = omciGetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_SET] = omciSetAction;
	tmpOmciManageEntity_p->omciActionFunc[MT_OMCI_MSG_TYPE_GET_CURR_DATA] = omciGetCurrentDataAction;
	/* if need ONU auto create a ME instance, create here */

	return 0;
}




