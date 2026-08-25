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
#ifndef _PMMGR_H_
#define _PMMGR_H_
#include "pmmgr_define.h"
#include "pmmgr_struct.h"
#ifdef TCSUPPORT_OMCI
#include "../omci/omci_external_msg.h"
#endif
extern pmmgrPmMe_t pmmgrPmMeTable[];

extern pmmgrGponDatabase_t *gpPmmgrGponDbAddr;

extern pmmgrEponDatabase_t *gpPmmgrEponDbAddr;
extern uint8 pmmgrDbgFlag;

void pmmgrPrintf(uint8 debugLevel, char *fmt,...);
int gponSetStaticticsTimes(uint16 time);
uint8 pmmgrGetAttriNumOfMe(pmmgrPmMe_t *me_p);
int pmmgrGetPmMeTableSize(void);
pmmgrPmInst_t *pmmgrGetPMInstanceByPortId(pmmgrPmMe_t *me_p , uint16 portId);
pmmgrPmMe_t *pmmgrGetPmMeByClassId(uint16 classId);
int pmmgrFreePmMeInstance(pmmgrPmInst_t * inst_p);
uint16 get16(uint8 *cp);
uint32 get32 (uint8 *cp);
uint64 get64 (uint8 *cp);
uint8 *put64(uint8 *cp, uint64 x);
uint8 *put32(uint8 *cp, uint32 x);
uint8 *put16(uint8 *cp, uint16 x);
int sendTca2Omci(int tcaId, uint16 portId, uint8 clearFlag);
int sendAlarm2OAM(int alarmId, uint8 portId, uint8 clearFlag, long long alarmInfo);
int pmmgrMeTableDestroy(void);
int pmmgrMeTableInit(void);
int databaseInit(uint8 mode);
int isEthernetFrameExtendedPMMe(pmmgrPmMe_t *me_p);
int gponPmEnable(void);
int timerDestroy(void);
int databaseDestroy(void);
int gponPMCurrentTmout(void* unused);

int eponPMCurrentTmout(uint8* param);
uint16 pmmgrGetPmAlarmIndexByAlarmId(uint16 alarmId);
int pmmgrCmdInit(void);
#endif
