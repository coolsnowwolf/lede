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
	oam_ctc_alarm.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_sys_api.h"


OamAlarmNode_t	oamAlarmTable[]	=
{
	{"EquipmentAlarm",        	ALARM_ID_EQUIPMENT,            	0x04, oamStatusEquipment,           	NULL},
	{"PowerAlarm",            	ALARM_ID_POWER,                	0x04, oamStatusPower,               	NULL},
	{"BatteryMissing",        	ALARM_ID_BATTERYMISSING,       	0x00, oamStatusBatteryMissing,      	NULL},
	{"BatteryFailure",        	ALARM_ID_BATTERYFAILURE,       	0x00, oamStatusBatteryFailure,      	NULL},
	{"BatteryVoltLow",        	ALARM_ID_BATTERYVOLTLOW,       	0x04, oamStatusBatteryVoltLow,      	oamThresholdBatteryVoltLow},
	{"PhysicalIntrusionAlarm",	ALARM_ID_PHYSICALINTRUSION,    	0x00, oamStatusPhysicalIntrusion,   	NULL},
	{"ONUSelfTestFailure",    	ALARM_ID_ONUSELFTESTFAILURE,   	0x04, oamStatusONUSelfTestFailure,  	NULL},
	{"ONUTempHighAlarm",      	ALARM_ID_ONUTEMPHIGH,          	0x04, oamStatusONUTempHigh,         	oamThresholdONUTempHigh},
	{"ONUTempLowAlarm",       	ALARM_ID_ONUTEMPLOW,           	0x04, oamStatusONUTempLow,          	oamThresholdONUTempLow},
	{"IADConnectionFailure",  	ALARM_ID_IADCONNECTIONFAILURE, 	0x04, oamStatusIADConnectionFailure,	NULL},
	{"PON_IFSwitch",          	ALARM_ID_PON_IFSWITCH,         	0x04, oamStatusPON_IFSwitch,        	NULL},

	{"SleepStatusUpdate",	ALARM_ID_SLEEPSTATUSUPDATE,	0x04, oamStatusSleepStatusUpdate,	NULL},
	
	/* PON IF alarms & warnings */
	{"RXPowerHighAlarm",      	ALARM_ID_RXPOWERHIGH,          0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"RXPowerLowAlarm",       	ALARM_ID_RXPOWERLOW,          0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TXPowerHighAlarm",      	ALARM_ID_TXPOWERHIGH,          0x04, oamStatusPMCounter,   oamThresholdPMCounter},
	{"TXPowerLowAlarm",       	ALARM_ID_TXPOWERLOW,          0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TXBiasHighAlarm",       	ALARM_ID_TXBIASHIGH,           	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TXBiasLowAlarm",        	ALARM_ID_TXBIASLOW,            	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"VccHighAlarm",          	ALARM_ID_VCCHIGH,              	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"VccLowAlarm",           	ALARM_ID_VCCLOW,               	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TempHighAlarm",         	ALARM_ID_TEMPHIGH,             	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TempLowAlarm",          	ALARM_ID_TEMPLOW,              	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"RXPowerHighWarning",    	WARING_ID_RXPOWERHIGH,        0x04, oamStatusPMCounter,  	oamThresholdPMCounter},
	{"RXPowerLowWarning",     	WARING_ID_RXPOWERLOW,        	0x04, oamStatusPMCounter,   	oamThresholdPMCounter},
	{"TXPowerHighWarning",    	WARING_ID_TXPOWERHIGH,        0x04, oamStatusPMCounter,  	oamThresholdPMCounter},
	{"TXPowerLowWarning",     	WARING_ID_TXPOWERLOW,        	0x04, oamStatusPMCounter,   	oamThresholdPMCounter},
	{"TXBiasHighWarning",     	WARING_ID_TXBIASHIGH,          	0x04, oamStatusPMCounter,   	oamThresholdPMCounter},
	{"TXBiasLowWarning",      	WARING_ID_TXBIASLOW,           	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"VccHighWarning",        	WARING_ID_VCCHIGH,             	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"VccLowWarning",         	WARING_ID_VCCLOW,              	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TempHighWarning",       	WARING_ID_TEMPHIGH,            	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
	{"TempLowWarning",        	WARING_ID_TEMPLOW,             	0x04, oamStatusPMCounter,    oamThresholdPMCounter},
#ifdef TCSUPPORT_PMMGR
	{"DS_DropeventsAlarm",      ALARM_ID_PONIF_DS_DROPEVENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_DropeventsAlarm",      ALARM_ID_PONIF_US_DROPEVENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_CrcerrorframesAlarm",  ALARM_ID_PONIF_DS_CRCERRORFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_CrcerrorframesAlarm",  ALARM_ID_PONIF_US_CRCERRORFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_UndersizeframesAlarm", ALARM_ID_PONIF_DS_UNDERSIZEFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_UndersizeframesAlarm", ALARM_ID_PONIF_US_UNDERSIZEFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_OversizeframesAlarm",  ALARM_ID_PONIF_DS_OVERSIZEFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_OversizeframesAlarm",  ALARM_ID_PONIF_US_OVERSIZEFRAMES,	0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_FragmentsAlarm",       ALARM_ID_PONIF_DS_FRAGMENTS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_FragmentsAlarm",       ALARM_ID_PONIF_US_FRAGMENTS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_JabbersAlarm",        	ALARM_ID_PONIF_DS_JABBERS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_JabbersAlarm",        	ALARM_ID_PONIF_US_JABBERS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_DiscardsAlarm",        ALARM_ID_PONIF_DS_DISCARDS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_DiscardsAlarm",        ALARM_ID_PONIF_US_DISCARDS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_ErrorsAlarm",        	ALARM_ID_PONIF_DS_ERRORS,           0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_ErrorsAlarm",        	ALARM_ID_PONIF_US_ERRORS,           0x08, oamStatusPMCounter,      	oamThresholdPMCounter},

	{"DS_DropeventsWarning",      WARNING_ID_PONIF_DS_DROPEVENTS,      0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_DropeventsWarning",      WARNING_ID_PONIF_US_DROPEVENTS,      0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_CrcerrorframesWarning",  WARNING_ID_PONIF_DS_CRCERRORFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_CrcerrorframesWarning",  WARNING_ID_PONIF_US_CRCERRORFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_UndersizeframesWarning", WARNING_ID_PONIF_DS_UNDERSIZEFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_UndersizeframesWarning", WARNING_ID_PONIF_US_UNDERSIZEFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_OversizeframesWarning",  WARNING_ID_PONIF_DS_OVERSIZEFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_OversizeframesWarning",  WARNING_ID_PONIF_US_OVERSIZEFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_FragmentsWarning",       WARNING_ID_PONIF_DS_FRAGMENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_FragmentsWarning",       WARNING_ID_PONIF_US_FRAGMENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_JabbersWarning",         WARNING_ID_PONIF_DS_JABBERS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_JabbersWarning",         WARNING_ID_PONIF_US_JABBERS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_DiscardsWarning",        WARNING_ID_PONIF_DS_DISCARDS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_DiscardsWarning",        WARNING_ID_PONIF_US_DISCARDS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"DS_ErrorsWarning",          WARNING_ID_PONIF_DS_ERRORS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"US_ErrorsWarning",          WARNING_ID_PONIF_US_ERRORS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
#endif
	/* Ethernet Port Alarms & Warnings */
	{"EthPortAutoNegFailure", 	ALARM_ID_ETHPORT_AUTONEGFAILURE,	0x00, oamStatusPMCounter,		NULL},
	{"EthPortLOS",            	ALARM_ID_ETHPORT_LOS,           	0x00, oamStatusPMCounter,    	NULL},
	{"EthPortFailure",        	ALARM_ID_ETHPORT_FAILURE,       	0x00, oamStatusPMCounter,      	NULL},
#ifdef TCSUPPORT_CUC
	{"EthPortLoopback",       	ALARM_ID_ETHPORT_LOOPBACK,      	0x04, oamStatusEthPortLoopback,     NULL},
#else
	{"EthPortLoopback",       	ALARM_ID_ETHPORT_LOOPBACK,      	0x00, oamStatusEthPortLoopback, NULL},
#endif
	{"EthPortCongestion",     	ALARM_ID_ETHPORT_CONGESTION,    	0x00, oamStatusPMCounter,   	NULL},
#ifdef TCSUPPORT_PMMGR
	{"EthPort_DS_DropeventsAlarm",         ALARM_ID_PORT_DS_DROPEVENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_DropeventsAlarm",         ALARM_ID_PORT_US_DROPEVENTS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_CrcerrorframesAlarm",     ALARM_ID_PORT_DS_CRCERRORFRAMES,   0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_CrcerrorframesAlarm",     ALARM_ID_PORT_US_CRCERRORFRAMES,   0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_UndersizeframesAlarm",    ALARM_ID_PORT_DS_UNDERSIZEFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_UndersizeframesAlarm",    ALARM_ID_PORT_US_UNDERSIZEFRAMES,  0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_OversizeframesAlarm",     ALARM_ID_PORT_DS_OVERSIZEFRAMES,   0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_OversizeframesAlarm", 	   ALARM_ID_PORT_US_OVERSIZEFRAMES,   0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_FragmentsAlarm",          ALARM_ID_PORT_DS_FRAGMENTS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_FragmentsAlarm",          ALARM_ID_PORT_US_FRAGMENTS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_JabbersAlarm",        	   ALARM_ID_PORT_DS_JABBERS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_JabbersAlarm",        	   ALARM_ID_PORT_US_JABBERS,          0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_DiscardsAlarm",           ALARM_ID_PORT_DS_DISCARDS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_DiscardsAlarm",           ALARM_ID_PORT_US_DISCARDS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_ErrorsAlarm",        	   ALARM_ID_PORT_DS_ERRORS,           0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_ErrorsAlarm",        	   ALARM_ID_PORT_US_ERRORS,           0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_StatusChangesAlarm",		   ALARM_ID_PORT_STATUSCHANGETIMES,   0x08, oamStatusPMCounter,      	oamThresholdPMCounter},

	{"EthPort_DS_DropeventsWarning",       WARNING_ID_PORT_DS_DROPEVENTS,     0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_DropeventsWarning",       WARNING_ID_PORT_US_DROPEVENTS,     0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_CrcerrorframesWarning",   WARNING_ID_PORT_DS_CRCERRORFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_CrcerrorframesWarning",   WARNING_ID_PORT_US_CRCERRORFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_UndersizeframesWarning",  WARNING_ID_PORT_DS_UNDERSIZEFRAMES,0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_UndersizeframesWarning",  WARNING_ID_PORT_US_UNDERSIZEFRAMES,0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_OversizeframesWarning",   WARNING_ID_PORT_DS_OVERSIZEFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_OversizeframesWarning",   WARNING_ID_PORT_US_OVERSIZEFRAMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_FragmentsWarning",        WARNING_ID_PORT_DS_FRAGMENTS,      0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_FragmentsWarning",        WARNING_ID_PORT_US_FRAGMENTS,      0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_JabbersWarning",          WARNING_ID_PORT_DS_JABBERS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_JabbersWarning",          WARNING_ID_PORT_US_JABBERS,        0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_DiscardsWarning",         WARNING_ID_PORT_DS_DISCARDS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_DiscardsWarning",         WARNING_ID_PORT_US_DISCARDS,       0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_DS_ErrorsWarning",           WARNING_ID_PORT_DS_ERRORS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_US_ErrorsWarning",           WARNING_ID_PORT_US_ERRORS,         0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
	{"EthPort_StatusChangesWarning",	   WARNING_ID_PORT_STATUSCHANGETIMES, 0x08, oamStatusPMCounter,      	oamThresholdPMCounter},
#endif

	{"POTSPortFailure",       	ALARM_ID_POTSPORTFAILURE,      	0x04, oamStatusPOTSPortFailure,     	NULL},
	{"E1PortFailure",         	ALARM_ID_E1PORTFAILURE,        	0x04, oamStatusE1PortFailure,       	NULL},
	{"E1TimingUnlock",        	ALARM_ID_E1TIMINGUNLOCK,       	0x00, oamStatusE1TimingUnlock,      	NULL},
	{"E1LOS",                 	ALARM_ID_E1LOS,                	0x00, oamStatusE1LOS,               	NULL},
	
};
void oamDisableAllAlarm(){
	OamAlarmNode_Ptr anp = oamAlarmTable;
	int num = sizeof(oamAlarmTable) / sizeof(oamAlarmTable[0]);
	int i = 0, j;
	OamEventStatusEntry_t ese = {0};

	for (i = 0; i < num; i++){
		if (!anp || !anp->oamStatus)
			continue;
		ese.alarmID = anp->alarmId;
		ese.status = ALARM_DISABLE;
		if (anp->alarmId < ALARM_ID_ONU_USED_MAX){
			ese.instanceNum = 0;
			ese.objectType = OBJ_LF_ONU;
			anp->oamStatus(&ese, REQ_SET);
		}else if (anp->alarmId >=ALARM_ID_RXPOWERHIGH && anp->alarmId <=WARING_ID_TEMPLOW){
			ese.instanceNum = 0;
			ese.objectType = OBJ_LF_PONIF;
			anp->oamStatus(&ese, REQ_SET);
		}else if (anp->alarmId >=ALARM_ID_ETHPORT_AUTONEGFAILURE &&  anp->alarmId <=ALARM_ID_ETHPORT_CONGESTION){
			ese.objectType = OBJ_LF_PORT;
			for (j = 0; j < PORT_ETHER_NUM; j++){
				ese.instanceNum = j;
				anp->oamStatus(&ese, REQ_SET);
			}
		}
		
		anp ++;
	}
}

OamAlarmNode_Ptr findAlarmNodeById(u_short alarmId){
	OamAlarmNode_Ptr anp = oamAlarmTable;
	int num = sizeof(oamAlarmTable) / sizeof(oamAlarmTable[0]);
	int i = 0;

	for (; i < num; i++){
		if (alarmId == anp->alarmId)
			return anp;
		anp ++;
	}
	return NULL;
}

#ifdef TCSUPPORT_PMMGR
int oamStatusPMCounter(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	u_char portId  = esep->instanceNum;
	u_int  alarmId = esep->alarmID;
	int   ret;

	if (esep->objectType == OBJ_LF_PONIF){
		portId += PORT_ETHER_NUM;
	}else if (esep->objectType != OBJ_LF_PORT){
		esep->status = EVENT_NOT_FOUND;
		return SUCCESS;
	}

	if (req_type == REQ_GET){
		ret = getPortAlarmStatus(portId, alarmId);
		if (FAIL == ret){
			esep->status = EVENT_NOT_FOUND;
		}else{
			esep->status = ret;
		}
	}else{
		// the status = 0 / 1
		ret = setPortAlarmStatus(portId, alarmId, esep->status);

		if (FAIL == ret){
			esep->status = EVENT_NOT_FOUND;
		}
	}
	return SUCCESS;
}
int oamThresholdPMCounter(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	u_char portId  = ethep->instanceNum;
	u_int  alarmId = ethep->alarmID, set, clear;
	int   ret;

	if (ethep->objectType == OBJ_LF_PONIF){
		portId += PORT_ETHER_NUM;
	}else if (ethep->objectType != OBJ_LF_PORT){
		ethep->setThreshold   = EVENT_NOT_FOUND;
		ethep->clearThreshold = EVENT_NOT_FOUND;
		return SUCCESS;
	}

	if (req_type == REQ_GET){
		ret = getPortAlarmThreshold(portId, alarmId, &set, &clear);
		if (ret == FAIL){
			ethep->setThreshold   = EVENT_NOT_FOUND;
			ethep->clearThreshold = EVENT_NOT_FOUND;
		}else{
			ethep->setThreshold   = set;
			ethep->clearThreshold = clear;
		}
	}else{
		set   = ethep->setThreshold;
		clear = ethep->clearThreshold;
		ret   = setPortAlarmThreshold(portId, alarmId, set, clear);
		if (ret == FAIL){
			ethep->setThreshold   = EVENT_NOT_FOUND;
			ethep->clearThreshold = EVENT_NOT_FOUND;
		}
	}
	return SUCCESS;
}
#endif

int oamStatusEquipment(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	if (esep->objectType != OBJ_LF_ONU)
		esep->status = EVENT_NOT_FOUND;
	else{
		if (req_type == REQ_GET){

		}else{

		}
	}
	return SUCCESS;
}


int oamStatusONUSelfTestFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusSleepStatusUpdate(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}


int oamStatusONUTempHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdONUTempHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusONUTempLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdONUTempLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusPON_IFSwitch(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdPON_IFSwitch(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

/*
int oamStatusRXPowerHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdRXPowerHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusRXPowerLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdRXPowerLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXPowerHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXPowerHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXPowerLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXPowerLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXBiasHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXBiasHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}


int oamStatusTXBiasLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXBiasLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusVccHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdVccHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusVccLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdVccLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTempHigh(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTempHigh(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTempLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTempLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusRXPowerHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdRXPowerHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusRXPowerLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdRXPowerLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXPowerHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXPowerHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXPowerLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXPowerLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXBiasHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXBiasHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTXBiasLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTXBiasLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusVccHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdVccHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusVccLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdVccLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}

int oamStatusTempHighWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdTempHighWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}

int oamStatusTempLowWarning(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdTempLowWarning(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}

int oamStatusEthPortAutoNegFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}

int oamStatusEthPortLOS(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}

int oamStatusEthPortFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}
*/

int oamStatusEthPortLoopback(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    u_int    instanceNum = ntohl(esep->instanceNum);
   
	u_char   portId  = instanceNum & 0xFF;
	u_short  alarmId = ntohs(esep->alarmID);
	int      ret;   
    
	if (OBJ_LF_PORT != esep->objectType ){
		goto NOT_FOUND;
	}

	if (REQ_GET == req_type ){
		ret = getPortAlarmStatus(portId, alarmId);
		if (FAIL == ret){
			goto NOT_FOUND;
		}
		esep->status = ret;
	}else{
		ret = setPortAlarmStatus(portId, alarmId, esep->status);

		if (FAIL == ret){
			goto NOT_FOUND;
		}
	}
	
	return SUCCESS;

NOT_FOUND:
    esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}
/*
int oamStatusEthPortCongestion(OamEventStatusEntry_Ptr esep, u_char req_type)
{
    esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}
*/

// * ----------These alarms or warnings don't supported by our system.----------//
int oamStatusIADConnectionFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdIADConnectionFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusPower(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusBatteryMissing(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusBatteryFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusBatteryVoltLow(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdBatteryVoltLow(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusPhysicalIntrusion(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
	return SUCCESS;
}

int oamStatusPOTSPortFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}

int oamStatusE1PortFailure(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}

int oamStatusE1TimingUnlock(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}

int oamStatusE1LOS(OamEventStatusEntry_Ptr esep, u_char req_type)
{
	esep->status = EVENT_NOT_FOUND;
    return SUCCESS;
}
// * /

/* ----------------These alarms or warnings have no threshold.----------------//
int oamThresholdEquipment(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdPower(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdBatteryMissing(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdBatteryFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdPhysicalIntrusion(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
	return SUCCESS;
}
int oamThresholdONUSelfTestFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	return SUCCESS;
}
int oamThresholdEthPortAutoNegFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdEthPortLOS(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdEthPortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdEthPortLoopback(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdEthPortCongestion(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
    return SUCCESS;
}
int oamThresholdPOTSPortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
    return SUCCESS;
}
int oamThresholdE1PortFailure(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
    return SUCCESS;
}
int oamThresholdE1TimingUnlock(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
    return SUCCESS;
}
int oamThresholdE1LOS(OamEventThresholdEntry_Ptr ethep, u_char req_type)
{
	ethep->setThreshold   = EVENT_NOT_FOUND;
	ethep->clearThreshold = EVENT_NOT_FOUND;
    return SUCCESS;
}
*/

