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
#include <unistd.h>
#include <string.h>
#include "pmmgr_api.h"

int main(void)
{
	uint8 preState, curState;
	uint8 times = 0;


	FECPmcounters_t pm;
	uint32 mask = 0;
	PMEponCounters_t eponPm;

	uint8 enable = 0;
	int ret = 0;
	uint16 cycleTime = 0;
	uint32 set = 0;
	
	preState = curState = 0;
#if 0
	while (1)
	{
		curState = getXponMode();
		if (curState == GPON_MODE && preState == EPON_MODE)
		{
			pmmgrDestroy();
			pmmgrInit(GPON_MODE);
		}
		if (curState == EPON_MODE && preState == GPON_MODE)
		{
			pmmgrDestroy();
			pmmgrInit(EPON_MODE);
		}
		
		preState = curState;
		sleep(1);		
	}
#endif
#if 0
	pmmgrInit(GPON_MODE);

	pmmgrSynchronizeTime();
	while (1)
	{
		if (times == 0)
			pmmgrCreatePmMeInstance(OMCI_CLASS_ID_FEC_PM, 0);
		else if(times == 2)
		{
			memset(&pm, 0, sizeof(FECPmcounters_t));
			pmmgrGetPmCurrentData(OMCI_CLASS_ID_FEC_PM, 0, &pm);
			printf("pmCurrentData.correctedBytes = %x\n", pm.correctedBytes);
			printf("pmCurrentData.correctedCodeWords = %x\n", pm.correctedCodeWords);
			printf("pmCurrentData.uncorrectableCodeWords = %x\n", pm.uncorrectableCodeWords);
			printf("pmCurrentData.totalCodeWords = %x\n", pm.totalCodeWords);
			printf("pmCurrentData.FECSeconds = %x\n", pm.FECSeconds);
		}
		else if(times == 1)
		{
			memset(&pm, 0, sizeof(FECPmcounters_t));
			pmmgrGetPmHistoryData(OMCI_CLASS_ID_FEC_PM, 0, &pm);
			printf("pmHistoryData.correctedBytes = %x\n", pm.correctedBytes);
			printf("pmHistoryData.correctedCodeWords = %x\n", pm.correctedCodeWords);
			printf("pmHistoryData.uncorrectableCodeWords = %x\n", pm.uncorrectableCodeWords);
			printf("pmHistoryData.totalCodeWords = %x\n", pm.totalCodeWords);
			printf("pmHistoryData.FECSeconds = %x\n", pm.FECSeconds);
		}
		else if(times == 3)
		{
			memset(&pm, 0, sizeof(FECPmcounters_t));	
			pm.correctedBytes = 0x10;
			pm.correctedCodeWords = 0x20;
			pm.uncorrectableCodeWords = 0x30;
			pm.FECSeconds = 0x40;

			mask = (1<<0);
			mask |= (1<<1);
			mask |= (1<<2);
			mask |= (1<<4);
			pmmgrSetPmThreshold(OMCI_CLASS_ID_FEC_PM, 0, &pm, mask);
		}
		else if(times == 4)
		{
			memset(&pm, 0, sizeof(FECPmcounters_t));
			pmmgrGetPmThreshold(OMCI_CLASS_ID_FEC_PM, 0, &pm);
			printf("pmThreshold.correctedBytes = %x\n", pm.correctedBytes);
			printf("pmThreshold.correctedCodeWords = %x\n", pm.correctedCodeWords);
			printf("pmThreshold.uncorrectableCodeWords = %x\n", pm.uncorrectableCodeWords);
			printf("pmThreshold.totalCodeWords = %x\n", pm.totalCodeWords);
			printf("pmThreshold.FECSeconds = %x\n", pm.FECSeconds);
		}
	/*	else  if(times == 5)
		{
			pmmgrDelPmMeInstance(OMCI_CLASS_ID_FEC_PM, 0);
		}*/
		else //if(times == 6)
		{
			printf("pm interval end time = %x\n",pmmgrGetStaticticsTimes());
			break;
		}

		times ++;
		sleep(10);		
	}
#endif

	pmmgrInit(EPON_MODE);
	times = 0;
	while(1)
	{
		if (times == 0)
		{
			enable = getEponPMEnable(ETHER_PORT_4);
			if (enable == 0)
			{
				ret = setEponPMEnable(ETHER_PORT_1, ENABLE);
				ret = setEponPMEnable(ETHER_PORT_2, ENABLE);	
				ret = setEponPMEnable(ETHER_PORT_3, ENABLE);			
				ret = setEponPMEnable(ETHER_PORT_4, ENABLE);
				ret = setEponPMEnable(PON_IF_PORT, ENABLE);
				if (ret == -1)
				{
					printf("setEponPMEnable fail\n");
					return -1;
				}
			}
			enable = getEponPMEnable(ETHER_PORT_4);
			printf("getEponPMEnable: enable = %x\n", enable);
		}
		else if (times == 1)
		{
			printf("enter getEponPMCycleTime: times = %x\n", times);
			cycleTime = getEponPMCycleTime(ETHER_PORT_1);
			printf("getEponPMCycleTime: cycleTime = %x\n", cycleTime);
			if (cycleTime != 800)
			{
				ret = setEponPMCycleTime(ETHER_PORT_1, 800);
				if (ret == -1)
				{
					printf("setEponPMEnable fail\n");
					return -1;
				}
			}
			cycleTime = getEponPMCycleTime(ETHER_PORT_1);
			printf("getEponPMCycleTime: cycleTime = %x\n", cycleTime);
		}
		else if (times == 2)
		{
			ret = getEponPMCurrentData(ETHER_PORT_1, &eponPm);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			printf("getEponPMCurrentData: eponPm.downstreamDropEvents = %x\n", eponPm.downstreamDropEvents);
			
		}
		else if (times == 3)
		{
			ret = getEponPMHistoryData(ETHER_PORT_1, &eponPm);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}

			printf("getEponPMHistoryData: eponPm.downstreamDropEvents = %x\n", eponPm.downstreamDropEvents);
			
		}	
		else if(times == 4)
		{
			ret = setEponPortAlarmThreshold(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS, 0x10);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			ret = getEponPortAlarmThreshold(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS, &set);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			printf(" getEponPortAlarmThreshold(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS, &set) = %x\n", set);
		}
		else if (times  == 5)
		{
			ret = setEponPortAlarmStatus(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS, ENABLE);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			ret = getEponPortAlarmStatus(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			printf("getEponPortAlarmStatus(ETHER_PORT_1,ALARM_ID_PORT_DS_DROPEVENTS) = %x\n", ret);
		}
		else
		{
			printf("here ?");
			ret = getEponPMCurrentData(ETHER_PORT_1, &eponPm);
			if (ret == -1)
			{
				printf("getEponPMCurrentData fail\n");
				return -1;
			}
			printf("getEponPMCurrentData: eponPm.downstreamDropEvents = %x\n", eponPm.downstreamDropEvents);
		}


		times ++;
		sleep(10);	
		printf("times = %x \n", times);	
	}
	
	return 0;
}
