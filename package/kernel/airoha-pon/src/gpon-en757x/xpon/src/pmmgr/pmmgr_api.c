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
	omci.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	andy.Yi		2012/7/5	Create
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmmgr_api.h"
#include "pmmgr.h"
#include "pmmgr_timer.h"


/*
***********************************************************************************************************
Global performance monitors control function
***********************************************************************************************************
*/
uint8 timerPthreadExist = TIMER_PTHREAD_NO_EXIST;
extern pthread_mutex_t pmmgrGponLock;
extern int ponRestarting;

int pmmgrInit(uint8 mode)
{

	int ret = 0;
	
	if (timerPthreadExist == TIMER_PTHREAD_NO_EXIST)
	{
		ret = timerInit();
	
		if(ret != 0)
		{
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"\r\nInit timer fail!");
			return -1;
		}

		ret = pmmgrCmdInit();
		if (ret != 0){
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"\r\ncmd queue fail!");
			return -1;
		}
		
		timerPthreadExist = TIMER_PTHREAD_EXIST;
	}
	
	ret = databaseInit(mode);

	if(ret != 0)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"\r\nInit databaseInit fail!");
		return -1;
	}

	if (mode == GPON_MODE)
	{ 
		ret = gponPmEnable();
		if (ret != 0)
		{
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"\r\nInit gponPmEnable fail!");
			return -1;
		}
	}

	return 0;
}

int pmmgrDestroy(void)
{
	timerDestroy();
	databaseDestroy();
	return 0;
}

/*
***********************************************************************************************************
Gpon performance monitors function
***********************************************************************************************************
*/




/*______________________________________________________________________________
**	synchronizeTime
**
**	descriptions:
**		OLT synchronize time
**	parameters:
**		
**	return:
**		0: success
**		-1:fail
**______________________________________________________________________________
*/
int pmmgrSynchronizeTime(void)
{

	int meIndex;
	pmmgrPmInst_t *inst_p = NULL;
	pmmgrPmMe_t *me_p = NULL;
	int pmMeTableSize = pmmgrGetPmMeTableSize();

	pthread_mutex_lock(&pmmgrGponLock);
	if(ponRestarting)
	{
		pthread_mutex_unlock(&pmmgrGponLock);
		return 0;
	}
	
	gpPmmgrGponDbAddr->times = 0;
	for (meIndex = 0, me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
	{
		pthread_mutex_lock(me_p->pmmgrMutex);
		for (inst_p = me_p->pmmgrInstList; inst_p != NULL; inst_p = inst_p->next)
		{
			if( inst_p->pmStartData != NULL )
			{
				memset(inst_p->pmStartData, 0, 4 * me_p->attriTotalSize);
			}
		}
		pthread_mutex_unlock(me_p->pmmgrMutex);
	}
	
	pthread_mutex_unlock(&pmmgrGponLock);

	return 0;
}

/** pmmgrGetStaticticsTimes 
get the statictics times
**/
OUT uint16  pmmgrGetStaticticsTimes(void)
{
	if(gpPmmgrGponDbAddr != NULL)
	{
		return gpPmmgrGponDbAddr->times;
	}
	return 0;
}

OUT int  pmmgrSetStaticticsTimes(uint16 time)
{
	return gponSetStaticticsTimes(time);
}

/** pmmgrCreatePmMeInstance 
create the instance of PM me
**/
int pmmgrCreatePmMeInstance(uint16 classId, uint16 portId)
{
	pmmgrPmMe_ptr me_p = NULL;
	pmmgrPmInst_ptr inst_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrCreatePmMeInstance: me_p == NULL\n");
		return -1;
	}
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p != NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN,"pmmgrCreatePmMeInstance: the portId instance exist !\n");		

		pthread_mutex_lock(me_p->pmmgrMutex);
		inst_p->enable = ENABLE;
		inst_p->tcaSendMask = 0;
		pthread_mutex_unlock(me_p->pmmgrMutex);
		return 0;
	}
	
	pthread_mutex_lock(me_p->pmmgrMutex);
	inst_p = calloc(1, sizeof(pmmgrPmInst_t));
	if (inst_p == NULL)
	{
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrCreatePmMeInstance: inst_p == NULL\n");
		return -1;
	}

	inst_p->pmStartData = calloc(1, 4* me_p->attriTotalSize);
	if (inst_p->pmStartData == NULL)
	{
		free(inst_p);
		inst_p = NULL;
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrCreatePmMeInstance: inst_p->pmStartData == NULL\n");
		return -1;
	}
	inst_p->pmCurrentData = inst_p->pmStartData + me_p->attriTotalSize;
	inst_p->pmHistoryData = inst_p->pmStartData + 2*me_p->attriTotalSize;
	inst_p->pmThreshold = inst_p->pmStartData + 3*me_p->attriTotalSize;
	inst_p->enable = ENABLE;
	inst_p->portId = portId;
	inst_p->tcaSendMask = 0;

	inst_p->next = me_p->pmmgrInstList;
	me_p->pmmgrInstList = inst_p;

	pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;		
}

/** pmmgrDelPmMeInstance 
delete the instance of PM me
**/
int pmmgrDelPmMeInstance(uint16 classId, uint16 portId)
{
	pmmgrPmMe_ptr me_p = NULL;
	pmmgrPmInst_ptr inst_p = NULL;
	pmmgrPmInst_ptr preInst_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrDelPmMeInstance: me_p == NULL\n");
		return -1;
	}

	if(me_p->pmmgrInstList == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrDelPmMeInstance: me_p->pmmgrInstList == NULL\n");
		return -1;
	}
	pthread_mutex_lock(&pmmgrGponLock);
	pthread_mutex_lock(me_p->pmmgrMutex);	
	preInst_p = inst_p = me_p->pmmgrInstList;
	
	if(inst_p->portId == portId)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN,"pmmgrDelPmMeInstance portId=%x\n", inst_p->portId);
		me_p->pmmgrInstList = inst_p->next;
		pmmgrFreePmMeInstance(inst_p);
	}
	else
	{
		inst_p = inst_p->next;
		while(inst_p != NULL){
			pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrDelPmMeInstance portId=%x\n", inst_p->portId);
			if(inst_p->portId == portId)
			{
				preInst_p->next = inst_p->next;
				pmmgrFreePmMeInstance(inst_p);
				break;
			}
			preInst_p = inst_p;
			inst_p = inst_p->next;
		}
	}

	pthread_mutex_unlock(me_p->pmmgrMutex);
	pthread_mutex_unlock(&pmmgrGponLock);
	return 0;	
}

int pmmgrSetAlarmConfig(uint32 mask, gponAlarmConfig_t alarmConfig)
{
	if(gpPmmgrGponDbAddr != NULL)
	{
		gponAlarmConfig_ptr pmmgrAlarmCfg = &gpPmmgrGponDbAddr->gponAlarmCfg;
		
		if (mask & MASK_sfThreshold )
		{
			pmmgrAlarmCfg->sfThreshold = alarmConfig.sfThreshold;
		}

		if (mask & MASK_sdThreshold )
		{
			pmmgrAlarmCfg->sdThreshold = alarmConfig.sdThreshold;
		}
		
		if (mask & MASK_lowerOpticalThreshold )
		{
			pmmgrAlarmCfg->lowerOpticalThreshold = alarmConfig.lowerOpticalThreshold;
		}
		
		if (mask & MASK_upperOpticalThreshold )
		{
			pmmgrAlarmCfg->upperOpticalThreshold = alarmConfig.upperOpticalThreshold;
		}
		
		if (mask & MASK_lowerTxPowerThreshold )
		{
			pmmgrAlarmCfg->lowerTxPowerThreshold = alarmConfig.lowerTxPowerThreshold;
		}
		
		if (mask & MASK_upperTxPowerThreshold )
		{
			pmmgrAlarmCfg->upperTxPowerThreshold = alarmConfig.upperTxPowerThreshold;
		}
		
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig sfThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.sfThreshold);	
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig sdThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.sdThreshold);		
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig lowerOpticalThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.lowerOpticalThreshold);	
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig upperOpticalThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.upperOpticalThreshold);	
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig lowerTxPowerThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.lowerTxPowerThreshold);	
		pmmgrPrintf(PMMGR_DEBUG_TRACE ,"pmmgrSetAlarmConfig upperTxPowerThreshold=%d\n", gpPmmgrGponDbAddr->gponAlarmCfg.upperTxPowerThreshold);	
	}
	return 0;
}
/** pmmgrGetPmCurrentData 
get current data of  the instance of PM me
**/
int pmmgrGetPmCurrentData(uint16 classId, uint16 portId, void *pm)
{
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if(me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmCurrentData: me_p == NULL\n");
		return -1;
	}
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmCurrentData: inst_p == NULL\n");
		return -1;
	}

	if (pm == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmCurrentData: pm == NULL\n");
		return -1;
	}
	pthread_mutex_lock(me_p->pmmgrMutex);
	memcpy(pm, inst_p->pmCurrentData, me_p->attriTotalSize);
    pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;
}

/** pmmgrSetPmCurrentData 
set current data of  the instance of PM me to 0
**/
int pmmgrSetPmCurrentData(uint16 classId, uint16 portId, void *pm)
{
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if( me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmCurrentData: me_p == NULL\n");
		return -1;
	}
	if (pm == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmCurrentData: pm == NULL\n");
		return -1;
	}
	pthread_mutex_lock(me_p->pmmgrMutex);
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p == NULL)
	{
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmCurrentData: inst_p == NULL\n");
		return -1;
	}
	memset(inst_p->pmCurrentData, 0, me_p->attriTotalSize);
	pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;
}
/** pmmgrGetPmHistoryData 
get history data of  the instance of PM me
**/
int pmmgrGetPmHistoryData(uint16 classId, uint16 portId, void *pm)
{
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if( me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmHistoryData: inst_p == NULL\n");
		return -1;
	}
	if (pm == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmHistoryData: pm == NULL\n");
		return -1;
	}
	pthread_mutex_lock(me_p->pmmgrMutex);
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p == NULL)
	{
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmHistoryData: inst_p == NULL\n");
		return -1;
	}

	memcpy(pm, inst_p->pmHistoryData, me_p->attriTotalSize);
	pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;
}

/** pmmgrGetPmThreshold 
get threshold value of  the instance of PM me
**/
int pmmgrGetPmThreshold(uint16 classId, uint16 portId, void *threshold)
{
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;


	me_p = pmmgrGetPmMeByClassId(classId);
	if( me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmThreshold: inst_p == NULL\n");
		return -1;
	}
	if (threshold == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmThreshold: pm == NULL\n");
		return -1;
	}
	pthread_mutex_lock(me_p->pmmgrMutex);
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p == NULL)
	{
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPmThreshold: inst_p == NULL\n");
		return -1;
	}
	memcpy(threshold, inst_p->pmThreshold, me_p->attriTotalSize);
	pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;
}

/** pmmgrSetPmThreshold 
set threshold value of  the instance of PM me
**/
int pmmgrSetPmThreshold(uint16 classId, uint16 portId, void *threshold, uint32 mask)
{
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;
	pmmgrPmAttriDescript_t *attri_p = NULL;
	uint8 attriIndex = 0;

	me_p = pmmgrGetPmMeByClassId(classId);
	if( me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmThreshold: inst_p == NULL\n");
		return -1;
	}
	if (threshold == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmThreshold: pm == NULL\n");
		return -1;
	}
	pthread_mutex_lock(me_p->pmmgrMutex);
	inst_p = pmmgrGetPMInstanceByPortId(me_p, portId);
	if (inst_p == NULL)
	{
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrSetPmThreshold: inst_p == NULL\n");
		return -1;
	}
	for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p++)
	{
		if(mask & (1<<attriIndex))
		{
			if (inst_p->pmThreshold != NULL && threshold != NULL)
			{
				pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrSetPmThreshold: attriIndex = %x, attri_p->name=%s\n", attriIndex, attri_p->name);
				memcpy(inst_p->pmThreshold + attri_p->offset, ((uint8*)threshold+ attri_p->offset),  attri_p->length);
			}
			else
				pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrSetPmThreshold: inst_p->pmThreshold != NULL && threshold != NULL\n");
		}
	}
	pthread_mutex_unlock(me_p->pmmgrMutex);
	return 0;
}

/** pmmgrGetPmMeAttriNumByClassId 
return attribute NUM of ME by class Id: 
**/
uint8 pmmgrGetPmMeAttriNumByClassId(uint16 classId)
{
	pmmgrPmMe_t * me_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if (me_p == NULL)
		return 0xFF;

	return me_p->attriNum;
}

/** pmmgrGetPmMeAttriOffsetByClassId 
return attribute offset of ME by class Id: 
**/
uint8 pmmgrGetPmMeAttriOffsetByClassId(uint16 classId, uint8 attrIndex)
{
	pmmgrPmMe_t * me_p = NULL;
	pmmgrPmAttriDescript_t *attri_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if (me_p == NULL)
		return 0xFF;

	if (attrIndex > me_p->attriNum)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN, "pmmgrGetPmMeAttriLengthByClassId: attrIndex > me_p->attriNum !\n");
		return 0xFF;
	}

	if (me_p->pmmgrAttriDescriptList == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "pmmgrGetPmMeAttriLengthByClassId: me_p->pmmgrAttriDescriptList == NULL !\n");
		return 0xFF;
	}	
	attri_p = &me_p->pmmgrAttriDescriptList[attrIndex -1];
	return attri_p->offset;
}

/** pmmgrGetPmMeAttriLengthByClassId 
return attribute length of ME by class Id: 
**/
uint8 pmmgrGetPmMeAttriLengthByClassId(uint16 classId, uint8 attrIndex)
{
	pmmgrPmMe_t * me_p = NULL;
	pmmgrPmAttriDescript_t *attri_p = NULL;

	me_p = pmmgrGetPmMeByClassId(classId);
	if (me_p == NULL)
		return 0xFF;

	if (attrIndex > me_p->attriNum)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN, "pmmgrGetPmMeAttriLengthByClassId: attrIndex > me_p->attriNum !\n");
		return 0xFF;
	}

	if (me_p->pmmgrAttriDescriptList == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR, "pmmgrGetPmMeAttriLengthByClassId: me_p->pmmgrAttriDescriptList == NULL !\n");
		return 0xFF;
	}	
	attri_p = &me_p->pmmgrAttriDescriptList[attrIndex -1];
	return attri_p->length;
}

/** pmmgrGetDebugLevel 
return Debug Level: 
**/
uint8 pmmgrGetDebugLevel(void)
{
	return pmmgrDbgFlag;
}

/** pmmgrSetDebugLevel 
return Debug Level: 
**/
uint8 pmmgrSetDebugLevel(uint8 dbgFlag)
{
	pmmgrDbgFlag = dbgFlag;
	return 0;
}

/*
***********************************************************************************************************
Epon performance monitors function
***********************************************************************************************************
*/
int getEponPMCycleTime(uint8 portId)
{

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return FAIL;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return FAIL;
		
	return gpPmmgrEponDbAddr->PMEponControl[portId].cycleTime;
}

int setEponPMCycleTime(uint8 portId, uint16 time)
{
	PMEponControl_ptr PMControl;

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	
	PMControl = &gpPmmgrEponDbAddr->PMEponControl[portId];

	PMControl->cycleTime = time;

	return 0;
}

int getEponPMEnable(uint8 portId)
{
	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return FAIL;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return FAIL;
	
	return gpPmmgrEponDbAddr->PMEponControl[portId].enable;
}

uint8 timerParam[MAX_PORT_NUM] = {0}; //save the timer param for port id
int setEponPMEnable(uint8 portId, uint8 enable)
{
	PMEponControl_ptr PMControl;

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return -1;

	PMControl = &gpPmmgrEponDbAddr->PMEponControl[portId];

	pmmgrPrintf(PMMGR_DEBUG_WARN, "Set Epon PM Enable %d = %d\n", portId, enable);
	if ((enable == ENABLE) && (PMControl->enable == DISABLE))
	{	
		timerParam[portId] = portId;
		PMControl->enable = ENABLE;
		PMControl->times = 0;
		if (PMControl->cycleTime == 0)
			PMControl->cycleTime = DEFAULT_CYCLE_TIME;
		if (PMControl->timerId != 0)
			timerPeriodicStop(PMControl->timerId);
		PMControl->timerId = timerPeriodicStartS(1000, eponPMCurrentTmout, &timerParam[portId]);
		if (PMControl->timerId == -1)
		{	
			pmmgrPrintf(PMMGR_DEBUG_ERROR, "setEponPMEnable: PMControl->timerId == TIMER_ID_NULL\n");
			return -1;
		}
	}
	else if ((enable == DISABLE) && (PMControl->enable == ENABLE))
	{
		PMControl->enable = DISABLE;
		if (PMControl->timerId != 0)
			timerPeriodicStop(PMControl->timerId);
		PMControl->timerId = 0;
	}

	return 0;
}

int getEponPMCurrentData(uint8 portId, void *pm)
{
	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}

	if (pm == NULL)
		return -1;

	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	
	memcpy(pm, &gpPmmgrEponDbAddr->PMCurrentData[portId], sizeof(PMEponCounters_t));
	return 0;
}
int getEponPMHistoryData(uint8 portId, void *pm)
{
	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}

	if (pm == NULL)
		return -1;

	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	
	memcpy(pm, &gpPmmgrEponDbAddr->PMHistoryData[portId], sizeof(PMEponCounters_t));
	return 0;
}
int setEponPMCurrentData(uint8 portId, void *pm)
{
	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}
	
	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	
	memset(&gpPmmgrEponDbAddr->PMCurrentData[portId], 0 , sizeof(PMEponCounters_t));
	return 0;
}


int getEponPortAlarmThreshold(uint8 portId, uint16 alarmId, uint32 *set, uint32 *clear)
{

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}
	
	if (gpPmmgrEponDbAddr == NULL)
		return -1;

	switch(alarmId)
	{
		case ALARM_ID_PONIF_HIGH_RXPOWER:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highRxPower;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highRxPower;	
			break;
		case ALARM_ID_PONIF_LOW_RXPOWER:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowRxPower;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowRxPower;	
			break;
		case ALARM_ID_PONIF_HIGH_TXPOWER:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTxPower;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTxPower;	
			break;
		case ALARM_ID_PONIF_LOW_TXPOWER:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTxPower;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTxPower;	
			break;
		case ALARM_ID_PONIF_HIGH_TXCURRENT	:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTxCurrent;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTxCurrent;	
			break;
		case ALARM_ID_PONIF_LOW_TXCURRENT:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTxCurrent;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTxCurrent;	
			break;
		case ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE	:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highSupplyVoltage;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highSupplyVoltage;	
			break;
		case ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowSupplyVoltage;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowSupplyVoltage;	
			break;
		case ALARM_ID_PONIF_HIGH_TEMPRATURE:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTemprature;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTemprature;	
			break;
		case ALARM_ID_PONIF_LOW_TEMPRATURE:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTemprature;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTemprature;	
			break;
		case ALARM_ID_PONIF_DS_DROPEVENTS: 
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamDropEvents;
			break;
		case ALARM_ID_PONIF_US_DROPEVENTS:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamDropEvents;
			break;		
		case ALARM_ID_PONIF_DS_CRCERRORFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamCrcErroredFrames;
			break;	
		case ALARM_ID_PONIF_US_CRCERRORFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamCrcErroredFrames;
			break;	
		case ALARM_ID_PONIF_DS_UNDERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamUndersizeFrames;
			break;	
		case ALARM_ID_PONIF_US_UNDERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamUndersizeFrames;
			break;	
		case ALARM_ID_PONIF_DS_OVERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamOversizeFrames;
			break;	
		case ALARM_ID_PONIF_US_OVERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamOversizeFrames;
			break;	
		case ALARM_ID_PONIF_DS_FRAGMENTS :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamFragments;
			break;	
		case ALARM_ID_PONIF_US_FRAGMENTS:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamFragments;
			break;	
		case ALARM_ID_PONIF_DS_JABBERS :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamJabbers;
			break;	
		case ALARM_ID_PONIF_US_JABBERS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamJabbers;
			break;	
		case ALARM_ID_PONIF_DS_DISCARDS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamDiscards;
			break;	
		case ALARM_ID_PONIF_US_DISCARDS :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamDiscards;
			break;	
		case ALARM_ID_PONIF_DS_ERRORS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamErrors;
			break;	
		case ALARM_ID_PONIF_US_ERRORS   :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamErrors;
			break;	

		case WARNING_ID_PONIF_HIGH_RXPOWER:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highRxPower;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highRxPower;
			break;	
		case WARNING_ID_PONIF_LOW_RXPOWER:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowRxPower;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowRxPower;
			break;	
		case WARNING_ID_PONIF_HIGH_TXPOWER:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTxPower;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTxPower;
			break;	
		case WARNING_ID_PONIF_LOW_TXPOWER	:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTxPower;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTxPower;
			break;	
		case WARNING_ID_PONIF_HIGH_TXCURRENT:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTxCurrent;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTxCurrent;
			break;	
		case WARNING_ID_PONIF_LOW_TXCURRENT:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTxCurrent;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTxCurrent;
			break;	
		case WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highSupplyVoltage;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highSupplyVoltage;
			break;	
		case WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowSupplyVoltage;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowSupplyVoltage;
			break;	
		case WARNING_ID_PONIF_HIGH_TEMPRATURE:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTemprature;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTemprature;
			break;	
		case WARNING_ID_PONIF_LOW_TEMPRATURE:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTemprature;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTemprature;
			break;	
		case WARNING_ID_PONIF_DS_DROPEVENTS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamDropEvents;
			break;	
		case WARNING_ID_PONIF_US_DROPEVENTS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamDropEvents;
			break;	
		case WARNING_ID_PONIF_DS_CRCERRORFRAMES: 
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamCrcErroredFrames;
			break;	
		case WARNING_ID_PONIF_US_CRCERRORFRAMES:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamCrcErroredFrames;
			break;	
		case WARNING_ID_PONIF_DS_UNDERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamUndersizeFrames;
			break;	
		case WARNING_ID_PONIF_US_UNDERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamUndersizeFrames;
			break;	
		case WARNING_ID_PONIF_DS_OVERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamOversizeFrames;
			break;	
		case  WARNING_ID_PONIF_US_OVERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamOversizeFrames;
			break;	
		case WARNING_ID_PONIF_DS_FRAGMENTS :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamFragments;
			break;	
		case WARNING_ID_PONIF_US_FRAGMENTS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamFragments;
			break;	
		case WARNING_ID_PONIF_DS_JABBERS :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamJabbers;
			break;	
		case WARNING_ID_PONIF_US_JABBERS  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamJabbers;
			break;	
		case WARNING_ID_PONIF_DS_DISCARDS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamDiscards;
			break;	
		case WARNING_ID_PONIF_US_DISCARDS :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamDiscards;
			break;	
		case WARNING_ID_PONIF_DS_ERRORS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamErrors;
			break;	
		case WARNING_ID_PONIF_US_ERRORS:
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamErrors;
			break;	
			
		case ALARM_ID_PORT_DS_DROPEVENTS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamDropEvents;
			break;	
		case ALARM_ID_PORT_US_DROPEVENTS:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamDropEvents;
			break;	
		case ALARM_ID_PORT_DS_CRCERRORFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamCrcErroredFrames;
			break;	
		case ALARM_ID_PORT_US_CRCERRORFRAMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamCrcErroredFrames;
			break;	
		case ALARM_ID_PORT_DS_UNDERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamUndersizeFrames;
			break;	
		case ALARM_ID_PORT_US_UNDERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamUndersizeFrames;
			break;	
		case ALARM_ID_PORT_DS_OVERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamOversizeFrames;
			break;	
		case ALARM_ID_PORT_US_OVERSIZEFRAMES:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamOversizeFrames;
			break;	
		case ALARM_ID_PORT_DS_FRAGMENTS:
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamFragments;
			break;	
		case ALARM_ID_PORT_US_FRAGMENTS :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamFragments;
			break;	
		case ALARM_ID_PORT_DS_JABBERS :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamJabbers;
			break;	
		case ALARM_ID_PORT_US_JABBERS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamJabbers;
			break;	
		case ALARM_ID_PORT_DS_DISCARDS   :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamDiscards;
			break;	
		case ALARM_ID_PORT_US_DISCARDS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamDiscards;
			break;	
		case  ALARM_ID_PORT_DS_ERRORS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamErrors;
			break;	
		case  ALARM_ID_PORT_US_ERRORS  :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamErrors;
			break;	
		case ALARM_ID_PORT_STATUSCHANGETIMES :
			*set = gpPmmgrEponDbAddr->PMAlarmThreshold[portId].statusChangeTimes;
			*clear = gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].statusChangeTimes;
			break;	
			
		case WARNING_ID_PORT_DS_DROPEVENTS :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamDropEvents;
			break;	
		case WARNING_ID_PORT_US_DROPEVENTS  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamDropEvents;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamDropEvents;
			break;	
		case WARNING_ID_PORT_DS_CRCERRORFRAMES  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamCrcErroredFrames;
			break;	
		case WARNING_ID_PORT_US_CRCERRORFRAMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamCrcErroredFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamCrcErroredFrames;
			break;	
		case WARNING_ID_PORT_DS_UNDERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamUndersizeFrames;
			break;	
		case WARNING_ID_PORT_US_UNDERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamUndersizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamUndersizeFrames;
			break;	
		case WARNING_ID_PORT_DS_OVERSIZEFRAMES  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamOversizeFrames;
			break;	
		case  WARNING_ID_PORT_US_OVERSIZEFRAMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamOversizeFrames;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamOversizeFrames;
			break;	
		case WARNING_ID_PORT_DS_FRAGMENTS   :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamFragments;
			break;	
		case WARNING_ID_PORT_US_FRAGMENTS :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamFragments;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamFragments;
			break;	
		case WARNING_ID_PORT_DS_JABBERS  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamJabbers;
			break;	
		case WARNING_ID_PORT_US_JABBERS   :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamJabbers;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamJabbers;
			break;	
		case WARNING_ID_PORT_DS_DISCARDS  :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamDiscards;
			break;	
		case WARNING_ID_PORT_US_DISCARDS   :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamDiscards;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamDiscards;
			break;	
		case WARNING_ID_PORT_DS_ERRORS    :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamErrors;
			break;	
		case WARNING_ID_PORT_US_ERRORS   :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamErrors;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamErrors;
			break;	
		case WARNING_ID_PORT_STATUSCHANGETIMES :
			*set = gpPmmgrEponDbAddr->PMWarningThreshold[portId].statusChangeTimes;
			*clear = gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].statusChangeTimes;
			break;	
		default:
			return -1;
	}


	#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"getEponPortAlarmThreshold: portId = %x\n", portId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"getEponPortAlarmThreshold: alarmId = %x\n", alarmId);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"getEponPortAlarmThreshold: set = %x\n", *set);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"getEponPortAlarmThreshold: clear = %x\n", *clear);	
	#endif
	return 0;

}

int setEponPortAlarmThreshold(uint8 portId, uint16 alarmId,  uint32 set, uint32 clear)
{

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}
	if (gpPmmgrEponDbAddr == NULL)
		return -1;	

	#ifdef PMMGR_DEBUG
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"setEponPortAlarmThreshold: portId = %x\n", portId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"setEponPortAlarmThreshold: alarmId = %x\n", alarmId);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"setEponPortAlarmThreshold: set = %x\n", set);	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"setEponPortAlarmThreshold: clear = %x\n", clear);	
	#endif
	switch (alarmId)
	{
		case ALARM_ID_PONIF_HIGH_RXPOWER:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highRxPower = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highRxPower = clear;
			break;
		case ALARM_ID_PONIF_LOW_RXPOWER:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowRxPower = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowRxPower = clear;
			break;
		case ALARM_ID_PONIF_HIGH_TXPOWER:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTxPower = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTxPower = clear;
			break;
		case ALARM_ID_PONIF_LOW_TXPOWER:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTxPower = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTxPower = clear;
			break;
		case ALARM_ID_PONIF_HIGH_TXCURRENT	:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTxCurrent = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTxCurrent = clear;
			break;
		case ALARM_ID_PONIF_LOW_TXCURRENT:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTxCurrent = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTxCurrent = clear;
			break;
		case ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE	:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highSupplyVoltage = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highSupplyVoltage = clear;
			break;
		case ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowSupplyVoltage = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowSupplyVoltage = clear;
			break;
		case ALARM_ID_PONIF_HIGH_TEMPRATURE:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].highTemprature = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].highTemprature = clear;
			break;
		case ALARM_ID_PONIF_LOW_TEMPRATURE:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].lowTemprature = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].lowTemprature = clear;
			break;
		case ALARM_ID_PONIF_DS_DROPEVENTS: 
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamDropEvents = clear;
			break;
		case ALARM_ID_PONIF_US_DROPEVENTS:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamDropEvents = clear;
			break;		
		case ALARM_ID_PONIF_DS_CRCERRORFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamCrcErroredFrames = clear;
			break;	
		case ALARM_ID_PONIF_US_CRCERRORFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamCrcErroredFrames = clear;
			break;	
		case ALARM_ID_PONIF_DS_UNDERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamUndersizeFrames = clear;
			break;	
		case ALARM_ID_PONIF_US_UNDERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamUndersizeFrames = clear;
			break;	
		case ALARM_ID_PONIF_DS_OVERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamOversizeFrames = clear;
			break;	
		case ALARM_ID_PONIF_US_OVERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamOversizeFrames = clear;
			break;	
		case ALARM_ID_PONIF_DS_FRAGMENTS :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamFragments = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamFragments = clear;
			break;	
		case ALARM_ID_PONIF_US_FRAGMENTS:
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamFragments = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamFragments = clear;
			break;	
		case ALARM_ID_PONIF_DS_JABBERS :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamJabbers = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamJabbers = clear;
			break;	
		case ALARM_ID_PONIF_US_JABBERS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamJabbers = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamJabbers = clear;
			break;	
		case ALARM_ID_PONIF_DS_DISCARDS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamDiscards = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamDiscards = clear;
			break;	
		case ALARM_ID_PONIF_US_DISCARDS :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamDiscards = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamDiscards = clear;
			break;	
		case ALARM_ID_PONIF_DS_ERRORS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].downstreamErrors = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].downstreamErrors = clear;
			break;	
		case ALARM_ID_PONIF_US_ERRORS   :
			gpPmmgrEponDbAddr->PMAlarmThreshold[PON_IF_PORT].upstreamErrors = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[PON_IF_PORT].upstreamErrors = clear;
			break;	
			
		case WARNING_ID_PONIF_HIGH_RXPOWER:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highRxPower = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highRxPower = clear;
			break;	
		case WARNING_ID_PONIF_LOW_RXPOWER:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowRxPower = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowRxPower = clear;
			break;	
		case WARNING_ID_PONIF_HIGH_TXPOWER:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTxPower = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTxPower = clear;
			break;	
		case WARNING_ID_PONIF_LOW_TXPOWER	:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTxPower = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTxPower = clear;
			break;	
		case WARNING_ID_PONIF_HIGH_TXCURRENT:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTxCurrent = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTxCurrent = clear;
			break;	
		case WARNING_ID_PONIF_LOW_TXCURRENT:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTxCurrent = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTxCurrent = clear;
			break;	
		case WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highSupplyVoltage = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highSupplyVoltage = clear;
			break;	
		case WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowSupplyVoltage = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowSupplyVoltage = clear;
			break;	
		case WARNING_ID_PONIF_HIGH_TEMPRATURE:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].highTemprature = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].highTemprature = clear;
			break;	
		case WARNING_ID_PONIF_LOW_TEMPRATURE:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].lowTemprature = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].lowTemprature = clear;
			break;			
		case WARNING_ID_PONIF_DS_DROPEVENTS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamDropEvents = clear;
			break;	
		case WARNING_ID_PONIF_US_DROPEVENTS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamDropEvents = clear;
			break;	
		case WARNING_ID_PONIF_DS_CRCERRORFRAMES: 
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamCrcErroredFrames = clear;
			break;	
		case WARNING_ID_PONIF_US_CRCERRORFRAMES:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamCrcErroredFrames = clear;
			break;	
		case WARNING_ID_PONIF_DS_UNDERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamUndersizeFrames = clear;
			break;	
		case WARNING_ID_PONIF_US_UNDERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamUndersizeFrames = clear;
			break;	
		case WARNING_ID_PONIF_DS_OVERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamOversizeFrames = clear;
			break;	
		case  WARNING_ID_PONIF_US_OVERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamOversizeFrames = clear;
			break;	
		case WARNING_ID_PONIF_DS_FRAGMENTS :
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamFragments = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamFragments = clear;
			break;	
		case WARNING_ID_PONIF_US_FRAGMENTS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamFragments = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamFragments = clear;
			break;	
		case WARNING_ID_PONIF_DS_JABBERS :
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamJabbers = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamJabbers = clear;
			break;	
		case WARNING_ID_PONIF_US_JABBERS  :
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamJabbers = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamJabbers = clear;
			break;	
		case WARNING_ID_PONIF_DS_DISCARDS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamDiscards = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamDiscards = clear;
			break;	
		case WARNING_ID_PONIF_US_DISCARDS :
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamDiscards = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamDiscards = clear;
			break;	
		case WARNING_ID_PONIF_DS_ERRORS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].downstreamErrors = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].downstreamErrors = clear;
			break;	
		case WARNING_ID_PONIF_US_ERRORS:
			gpPmmgrEponDbAddr->PMWarningThreshold[PON_IF_PORT].upstreamErrors = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[PON_IF_PORT].upstreamErrors = clear;
			break;	
			
		case ALARM_ID_PORT_DS_DROPEVENTS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamDropEvents = clear;
			break;	
		case ALARM_ID_PORT_US_DROPEVENTS:
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamDropEvents = clear;
			break;	
		case ALARM_ID_PORT_DS_CRCERRORFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamCrcErroredFrames = clear;
			break;	
		case ALARM_ID_PORT_US_CRCERRORFRAMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamCrcErroredFrames = clear;
			break;	
		case ALARM_ID_PORT_DS_UNDERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamUndersizeFrames = clear;
			break;	
		case ALARM_ID_PORT_US_UNDERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamUndersizeFrames = clear;
			break;	
		case ALARM_ID_PORT_DS_OVERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamOversizeFrames = clear;
			break;	
		case ALARM_ID_PORT_US_OVERSIZEFRAMES:
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamOversizeFrames = clear;
			break;	
		case ALARM_ID_PORT_DS_FRAGMENTS:
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamFragments = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamFragments = clear;
			break;	
		case ALARM_ID_PORT_US_FRAGMENTS :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamFragments = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamFragments = clear;
			break;	
		case ALARM_ID_PORT_DS_JABBERS :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamJabbers = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamJabbers = clear;
			break;	
		case ALARM_ID_PORT_US_JABBERS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamJabbers = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamJabbers = clear;
			break;	
		case ALARM_ID_PORT_DS_DISCARDS   :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamDiscards = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamDiscards = clear;
			break;	
		case ALARM_ID_PORT_US_DISCARDS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamDiscards = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamDiscards = clear;
			break;	
		case  ALARM_ID_PORT_DS_ERRORS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].downstreamErrors = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].downstreamErrors = clear;
			break;	
		case  ALARM_ID_PORT_US_ERRORS  :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].upstreamErrors = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].upstreamErrors = clear;
			break;	
		case ALARM_ID_PORT_STATUSCHANGETIMES :
			gpPmmgrEponDbAddr->PMAlarmThreshold[portId].statusChangeTimes = set;
			gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId].statusChangeTimes = clear;
			break;	
			
		case WARNING_ID_PORT_DS_DROPEVENTS :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamDropEvents = clear;
			break;	
		case WARNING_ID_PORT_US_DROPEVENTS  :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamDropEvents = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamDropEvents = clear;
			break;	
		case WARNING_ID_PORT_DS_CRCERRORFRAMES  :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamCrcErroredFrames = clear;
			break;	
		case WARNING_ID_PORT_US_CRCERRORFRAMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamCrcErroredFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamCrcErroredFrames = clear;
			break;	
		case WARNING_ID_PORT_DS_UNDERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamUndersizeFrames = clear;
			break;	
		case WARNING_ID_PORT_US_UNDERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamUndersizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamUndersizeFrames = clear;
			break;	
		case WARNING_ID_PORT_DS_OVERSIZEFRAMES  :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamOversizeFrames = clear;
			break;	
		case  WARNING_ID_PORT_US_OVERSIZEFRAMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamOversizeFrames = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamOversizeFrames = clear;
			break;	
		case WARNING_ID_PORT_DS_FRAGMENTS   :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamFragments = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamFragments = clear;
			break;	
		case WARNING_ID_PORT_US_FRAGMENTS :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamFragments = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamFragments = clear;
			break;	
		case WARNING_ID_PORT_DS_JABBERS  :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamJabbers = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamJabbers = clear;
			break;	
		case WARNING_ID_PORT_US_JABBERS   :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamJabbers = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamJabbers = clear;
			break;	
		case WARNING_ID_PORT_DS_DISCARDS  :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamDiscards = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamDiscards = clear;
			break;	
		case WARNING_ID_PORT_US_DISCARDS   :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamDiscards = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamDiscards = clear;
			break;	
		case WARNING_ID_PORT_DS_ERRORS    :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].downstreamErrors = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].downstreamErrors = clear;
			break;	
		case WARNING_ID_PORT_US_ERRORS   :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].upstreamErrors = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].upstreamErrors = clear;
			break;	
		case WARNING_ID_PORT_STATUSCHANGETIMES :
			gpPmmgrEponDbAddr->PMWarningThreshold[portId].statusChangeTimes = set;
			gpPmmgrEponDbAddr->PMWarningClearThreshold[portId].statusChangeTimes = clear;
			break;	
		default:
			return -1;
	}	

	return 0;
}


int getEponPortAlarmStatus(uint8 portId, uint16 alarmId)
{
	uint16 alarmldIndex = 0;

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	alarmldIndex = pmmgrGetPmAlarmIndexByAlarmId(alarmId);

	if ((alarmldIndex & 0xFF00) == 0)
		return (gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].AlarmEnableMask & (1<< (alarmldIndex & 0x00FF)))? ENABLE: DISABLE;
	else if((alarmldIndex & 0xFF00) == 0x100)
		return (gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].WarningEnableMask & (1<< (alarmldIndex & 0x00FF)))? ENABLE: DISABLE;
	else if((alarmldIndex & 0xFF00) == 0x200)
		return (gpPmmgrEponDbAddr->PMEponControl[portId].AlarmEnableMask & (1<< (alarmldIndex & 0x00FF)))? ENABLE: DISABLE;
	else if((alarmldIndex & 0xFF00) == 0x300)
		return (gpPmmgrEponDbAddr->PMEponControl[portId].WarningEnableMask & (1<< (alarmldIndex & 0x00FF)))? ENABLE: DISABLE;	

	return -1;
}

int setEponPortAlarmStatus(uint8 portId, uint16 alarmId, uint8 enable)
{
	uint16 alarmldIndex = 0;

	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown portId\n");
		return -1;
	}
	if (gpPmmgrEponDbAddr == NULL)
		return -1;

	if (enable == ENABLE){		
		PMEponControl_ptr PMControl;
		PMControl = &gpPmmgrEponDbAddr->PMEponControl[portId];
		if (PMControl->enable == DISABLE){
			setEponPMEnable(portId, ENABLE);
		}
	}
	
	alarmldIndex = pmmgrGetPmAlarmIndexByAlarmId(alarmId);

	
	if ((alarmldIndex & 0xFF00) == 0)
	{
		if (enable == ENABLE)
		 	gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].AlarmEnableMask |= (1<< (alarmldIndex & 0x00FF));
		else
			gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].AlarmEnableMask &= ~(1<< (alarmldIndex & 0x00FF));
	}
	else if((alarmldIndex & 0xFF00) == 0x100)
	{
		if (enable == ENABLE)
		 	gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].WarningEnableMask |= (1<< (alarmldIndex & 0x00FF));
		else
			gpPmmgrEponDbAddr->PMEponControl[PON_IF_PORT].WarningEnableMask &= ~(1<< (alarmldIndex & 0x00FF));
	}
	else if((alarmldIndex & 0xFF00) == 0x200)
	{
		if (enable == ENABLE)
		 	gpPmmgrEponDbAddr->PMEponControl[portId].AlarmEnableMask |= (1<< (alarmldIndex & 0x00FF));
		else
			gpPmmgrEponDbAddr->PMEponControl[portId].AlarmEnableMask &= ~(1<< (alarmldIndex & 0x00FF));
	}
	else if((alarmldIndex & 0xFF00) == 0x300)
	{
		if (enable == ENABLE)
		 	gpPmmgrEponDbAddr->PMEponControl[portId].AlarmEnableMask |= (1<< (alarmldIndex & 0x00FF));
		else
			gpPmmgrEponDbAddr->PMEponControl[portId].AlarmEnableMask &= ~(1<< (alarmldIndex & 0x00FF));
	}

	return 0;
}


