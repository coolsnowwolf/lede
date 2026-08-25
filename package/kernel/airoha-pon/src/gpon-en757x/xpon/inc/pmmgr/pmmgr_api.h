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
#ifndef _PMMGR_API_H_
#define _PMMGR_API_H_

#include "pmmgr_struct.h"
#include "pmmgr_define.h"

/*
***********************************************************************************************************
Global performance monitors control function
***********************************************************************************************************
*/

OUT int pmmgrInit(IN uint8 mode);
OUT int pmmgrDestroy(void);
/*
***********************************************************************************************************
Gpon performance monitors function
***********************************************************************************************************
*/

OUT int pmmgrSynchronizeTime(void);
OUT uint16 pmmgrGetStaticticsTimes(void);
OUT int pmmgrCreatePmMeInstance(IN uint16 classId, IN uint16 portId);
OUT int pmmgrDelPmMeInstance(IN uint16 classId, IN uint16 portId);
OUT int pmmgrGetPmCurrentData(IN uint16 classId, IN uint16 portId, OUT void *pm);
OUT int pmmgrSetAlarmConfig(IN uint32 mask, IN gponAlarmConfig_t alarmConfig);
OUT int pmmgrSetPmCurrentData(IN uint16 classId, IN uint16 portId, OUT void *pm);
OUT int pmmgrGetPmHistoryData(IN uint16 classId, IN uint16 portId, OUT void *pm);
OUT int pmmgrGetPmThreshold(IN uint16 classId, IN uint16 portId, OUT void *threshold);
OUT int pmmgrSetPmThreshold(IN uint16 classId, IN uint16 portId, IN void *threshold, IN uint32 mask);

OUT uint8 pmmgrGetPmMeAttriNumByClassId(IN uint16 classId);
OUT uint8 pmmgrGetPmMeAttriOffsetByClassId(IN uint16 classId,IN  uint8 attrIndex);
OUT uint8 pmmgrGetPmMeAttriLengthByClassId(IN uint16 classId,IN  uint8 attrIndex);
OUT uint8 pmmgrGetDebugLevel(IN  void);
OUT uint8 pmmgrSetDebugLevel(IN  uint8 dbgFlag);
/*
***********************************************************************************************************
Epon performance monitors function
***********************************************************************************************************
*/
OUT int getEponPMCycleTime(IN uint8 portId);
OUT int setEponPMCycleTime(IN uint8 portId, IN uint16 time);
OUT int getEponPMEnable(IN uint8 portId);
OUT int setEponPMEnable(IN uint8 portId, IN uint8 enable);
OUT int getEponPMCurrentData(IN uint8 portId, OUT void *pm);
OUT int getEponPMHistoryData(IN uint8 portId, OUT void *pm);
OUT int setEponPMCurrentData(IN uint8 portId, IN void *pm);

OUT int getEponPortAlarmThreshold(IN uint8 portId, IN uint16 alarmId, OUT uint32 *set, OUT uint32 *clear);
OUT int setEponPortAlarmThreshold(IN uint8 portId, IN uint16 alarmId,  IN uint32 set, IN uint32 clear);
OUT int getEponPortAlarmStatus(IN uint8 portId, IN uint16 alarmId);
OUT int setEponPortAlarmStatus(IN uint8 portId, IN uint16 alarmId, IN uint8 enable);


#endif
