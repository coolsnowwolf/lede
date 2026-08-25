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
	pmgr.c

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
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <pthread.h>
#include "pmmgr_struct.h"
#include "pmmgr_define.h"
#include "pmmgr.h"
#include "pmmgr_timer.h"
#include "pmmgr_pm.h"
#include <stdarg.h>
#include "libtcapi.h"
#ifdef TCSUPPORT_PONMGR
#include "api/mgr_api.h"
#endif
#ifdef TCSUPPORT_MT7530_SWITCH_API
#define u8  unsigned char 
#define u16 unsigned short
#define u32 unsigned int
#include "mtkswitch_api.h"
#endif
#ifdef TCSUPPORT_WAN_EPON
#include "libepon.h"
#endif
#if !defined(TCSUPPORT_CT) 
#include "../cfg_manager/cfg_manager.h"
#endif

extern mxml_node_t **gTreePtr;
extern int ponRestarting;

gponAlarmStatus_t gpPmmgrGponAlarmStatus;

pmmgrGponDatabase_t *gpPmmgrGponDbAddr = NULL;
pmmgrEponDatabase_t *gpPmmgrEponDbAddr = NULL;
pthread_mutex_t pmmgrGponLock;

pmmgrPmMe_t pmmgrPmMeTable[] = {
{OMCI_CLASS_ID_FEC_PM_DATA, 					"FEC PM", 				pmmgrPmInitForFecPM, 					NULL, NULL, 0, 0, pmmgrPmAttriDescriptFecPM, 				getAllDataFromApiForFecPM},
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PMHD,			"MAC Bridge PM", 			pmmgrPmInitForMACBridgePM, 			NULL, NULL, 0, 0, pmmgrPmAttriDescriptMACBridgePM,		getAllDataFromApiForMACBridgePM},
{OMCI_ME_CLASS_ID_MAC_BRIDGE_PORT_PMHD, 		"MAC Bridge Port PM", 		pmmgrPmInitForMACBridgePortPM, 		NULL, NULL, 0, 0, pmmgrPmAttriDescriptMACBridgePortPM, 	getAllDataFromApiForMACBridgePortPM},
{OMCI_ME_CLASS_ID_ETHERNET_FRAME_UP_PMHD,		"Ethernet frame UP PM", 	pmmgrPmInitForEthernetFrameStreamPM, NULL, NULL, 0, 0, pmmgrPmAttriDescriptEthernetFrameUpPM, 	getAllDataFromApiForEthernetFrameUpPM},
{OMCI_ME_CLASS_ID_ETHERNET_FRAME_DOWN_PMHD,	"Ethernet frame Down PM",	pmmgrPmInitForEthernetFrameStreamPM, NULL, NULL, 0, 0, pmmgrPmAttriDescriptEthernetFrameDownPM,getAllDataFromApiForEthernetFrameDownPM},
{OMCI_CLASS_ID_GEM_PORT_NETWORK_CTP_PM, 		"GEM Port network CTP PM", pmmgrPmInitForGemPortCtpPM, 			NULL, NULL, 0, 0, pmmgrPmAttriDescriptGemPortCtpPM, 		getAllDataFromApiForGemPortCtpPM},
{OMCI_CLASS_ID_ENERGY_CONSUMPTION_PM, 		"Energy Consumption PM", 	pmmgrPmInitForEnergyConsumPM, 		NULL, NULL, 0, 0, pmmgrPmAttriDescriptEnergyConsumPM, 	getAllDataFromApiForEnergyConsumPM},
{OMCI_CLASS_ID_GEM_PORT_PM, 				"GEM port PM",		pmmgrPmInitForGemPortPM,			NULL, NULL, 0, 0, pmmgrPmAttriDescriptGemPortPM,		getAllDataFromApiForGemPortPM},
{OMCI_CLASS_ID_GAL_ETHERNET_PM, 				"GAL ethernet PM", 		pmmgrPmInitForGalEthernetPM, 			NULL, NULL, 0, 0, pmmgrPmAttriDescriptGalEthernetPM, 		getAllDataFromApiForGalEthernetPM},
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA,	 	"Ethernet frame HD PM", 	pmmgrPmInitForPptpEthernetUniPM,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptPptpEthernetUniPM,	getAllDataFromApiForPptpEthernetUniPM},
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_2, 	"Ethernet frame HD2 PM", 	pmmgrPmInitForPptpEthernetUni2PM,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptPptpEthernetUni2PM,	getAllDataFromApiForPptpEthernetUni2PM},
{OMCI_CLASS_ID_ETHERNET_PM_HISTORY_DATA_3, 	"Ethernet frame HD3 PM",	pmmgrPmInitForPptpEthernetUni3PM,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptPptpEthernetUni3PM,	getAllDataFromApiForPptpEthernetUni3PM},
{OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM,	"Ethernet frame Extended PM",	pmmgrPmInitForEthernetFrameExtendedPM,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptEthernetFrameExtendedPM,	getAllDataFromApiForEthernetFrameExtendedPM},
{OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT,	"Ethernet frame extended pm 64-bit",	pmmgrPmInitForEthernetFrameExtendedPM64bit,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptEthernetFrameExtendedPM64bit,	getAllDataFromApiForEthernetFrameExtendedPM64bit},
{OMCI_CLASS_ID_IP_HOST_PM,	 					"IP host cfg data PM", 		pmmgrPmInitForIpHostCfgDataPM, 		NULL, NULL, 0, 0, pmmgrPmAttriDescriptIpHostCfgDataPM, 	getAllDataFromApiForIpHostCfgDataPM},
#ifdef TCSUPPORT_VOIP
{CID_CALL_CTRL_PM_HISTORY,             "Call ctrl history data PM",      pmmgrPmInitForCallCtrlPM,      NULL, NULL, 0, 0, pmmgrPmAttriDescriptCallCtrlPM,  getAllDataFromApiForCallCtrlPM},
{OMCI_CLASS_ID_RTP_PM_HISTORY_DATA,             "Rtp history data PM",      pmmgrPmInitForRTPDataPM,      NULL, NULL, 0, 0, pmmgrPmAttriDescriptRtpDataPM,  getAllDataFromApiForRTPDataPM},
{OMCI_CLASS_SIP_AGENT_PM_HISTORY,     "SIP agent performance monitoring history data",      pmmgrPmInitForSipAgentPM,      NULL, NULL, 0, 0, pmmgrPmAttriDescriptSipAgentPM,  getAllDataFromApiForSipAgentPM},
{OMCI_CLASS_SIP_CALL_INIT_PM_HISTORY, "SIP call initiation performance monitoring history data",      pmmgrPmInitForSipCallInitPM,      NULL, NULL, 0, 0, pmmgrPmAttriDescriptSipCallInitPM,  getAllDataFromApiForSipCallInitPM},
#endif
{OMCI_CLASS_ID_TCP_UDP_PM, 					"tcp udp cfg data PM", 		pmmgrPmInitForTcpUdpCfgDataPM, 		NULL, NULL, 0, 0, pmmgrPmAttriDescriptTcpUdpCfgDataPM, 	getAllDataFromApiForTcpUdpCfgDataPM},
#ifdef TCSUPPORT_OMCI_ALCATEL
{OMCI_CLASS_ID_TOTAL_GEM_PORT_PM, 			"Total GEM port PM",		pmmgrPmInitForTotalGemPortPM,		NULL, NULL, 0, 0, pmmgrPmAttriDescriptTotalGemPortPM,	getAllDataFromApiForTotalGemPortPM},
{OMCI_CLASS_ID_ETHERNET_TRAFFIC_PM,			"Ethernet Traffic PM",		pmmgrPmInitForEthernetTrafficPM,	NULL, NULL, 0, 0, pmmgrPmAttriDescriptEthernetTrafficPM,	getAllDataFromApiForEthernetTrafficPM},
{OMCI_CLASS_ID_GEMPORT_PROTOCOL_MONITORING_HISTORY_DATA_PART_2,	"GEMPort Protocol Monitoring HD Part2 PM",			pmmgrPmInitForGEMPortProtocolMonitoringHDPart2PM,			NULL, NULL, 0, 0, pmmgrPmAttriDescriptGEMPortProtocolMonitoringHDPart2PM,	getAllDataFromApiForGEMPortProtocolMonitoringHDPart2PM}
#endif
};

uint8 pmmgrDbgFlag = PMMGR_DEBUG_ERROR;
/*_____________________________________________________________________________
**      function name: pmmgrPrintf
**      descriptions:
**            Show debug message in three level.
**
**      parameters:
**           N/A
**
**      global:
**           N/A
**
**      return:
**	     	 N/A
**      call:
**      	 N/A
**      revision:
**       Andy.yi
**____________________________________________________________________________
*/
void pmmgrPrintf(uint8 debugLevel, char *fmt,...)
{

	FILE *proc_file;
	char msg[256];
	va_list args;

	if(debugLevel > pmmgrDbgFlag )
		return;
	
	va_start(args, fmt);

	vsnprintf(msg, 256, fmt, args);	

    	proc_file = fopen("/proc/tc3162/dbg_msg", "w");
	if (!proc_file) {
		printf("open /proc/tc3162/dbg_msg fail\n");
		return;
	}

//	tcdbg_printf("%s", msg);
	fprintf(proc_file, "%s", msg);
	fclose(proc_file);
	va_end(args);	
}


 uint16
get16(
	uint8	*cp
)
{
	register uint16 x;

	x = *cp++;
	x <<= 8;
	x |= *cp;
	return x;
} /* get16 */

 uint32
get32 (
	uint8	*cp
)
{
	uint32 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
} /* get32 */
 
 uint64
get64 (
	uint8	*cp
)
{
	uint64 rval;

	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval = *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp++;
	rval <<= 8;
	rval |= *cp;

	return rval;
} /* get642 */
 uint8 *
put64(
	uint8	*cp,
	uint64	x
)
{
	*cp++ = x >> 56;
	*cp++ = x >> 48;
	*cp++ = x >> 40;
	*cp++ = x >> 32;
	*cp++ = x >> 24;
	*cp++ = x >> 16;
	*cp++ = x >> 8;
	*cp++ = x;
	return cp;
} /* put64 */

 uint8 *
put32(
	uint8	*cp,
	uint32	x
)
{
	*cp++ = x >> 24;
	*cp++ = x >> 16;
	*cp++ = x >> 8;
	*cp++ = x;
	return cp;
} /* put32 */

uint8 *
put16(
	uint8	*cp,
	uint16	x
)
{
	*cp++ = x >> 8;
	*cp++ = x;

	return cp;
} /* put16 */


/*send tca msg to omci*/
int sendTca2Omci(int tcaId, uint16 portId, uint8 clearFlag)
{

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendTca2Omci: tcaId = %x\n", tcaId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendTca2Omci: portId = %x\n", portId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendTca2Omci: clearFlag = %x\n", clearFlag);

#ifdef TCSUPPORT_OMCI
	notify_msg_t msg;
	memset(&msg, 0, sizeof(msg));

	msg.notifyType = OMCI_NOTIFY_TYPE_TCA;
	msg.tcaId = tcaId;
	msg.deviceId = portId;
	msg.alarmFlag = clearFlag;
	
	return sendOmciNotifyMsg(&msg);
#else
	return 0;
#endif
}

/*send alarm msg to omci*/
int sendAlarm2Omci(int alarmId, uint16 portId, uint8 clearFlag)
{

	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendAlarm2Omci: alarmId = %x\n", alarmId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendAlarm2Omci: portId = %x\n", portId);
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter sendAlarm2Omci: clearFlag = %x\n", clearFlag);

#ifdef TCSUPPORT_OMCI
	notify_msg_t msg;
	memset(&msg, 0, sizeof(msg));

	msg.notifyType = OMCI_NOTIFY_TYPE_ALARM;
	msg.alarmId = alarmId;
	msg.deviceId = portId;
	msg.alarmFlag = clearFlag;
	
	return sendOmciNotifyMsg(&msg);
#else
	return 0;
#endif
}

#ifdef TCSUPPORT_OMCI_ALCATEL
int sendPeriodAvc2Omci(void)
{
#ifdef TCSUPPORT_OMCI
	notify_msg_t msg;
	memset(&msg, 0, sizeof(msg));

	msg.notifyType = OMCI_NOTIFY_TYPE_PERIOD_AVC;
	
	return sendOmciNotifyMsg(&msg);
#else
	return 0;
#endif
}
#endif

/*send alarm/warning msg to oam*/
int sendAlarm2OAM(int alarmId, uint8 portId, uint8 alarmState, long long alarmInfo)
{
#ifdef TCSUPPORT_EPON_OAM
	OamAlarmMsg_t amp;
	memset(&amp, 0, sizeof(OamAlarmMsg_t));
	
	pmmgrPrintf(PMMGR_DEBUG_WARN,"Enter sendAlarm2OAM: alarmId = %x, portId=%x, alarmState=%x,alarmInfoL=%u\n", 
		alarmId, portId, alarmState, (int)alarmInfo);

	amp.trapIndex = alarmId;
	*(amp.trapOtherInfo) = alarmState;
	if (portId == PON_IF_PORT){
		amp.devIndex_3 = 0;
	}else{
		amp.devIndex_1 = portId+1; // lan port start at 0x01
	}

	amp.trapOtherInfo[1] = alarmInfo>>56;
	amp.trapOtherInfo[2] = alarmInfo>>48;
	amp.trapOtherInfo[3] = alarmInfo>>40;
	amp.trapOtherInfo[4] = alarmInfo>>32;
	amp.trapOtherInfo[5] = alarmInfo>>24;
	amp.trapOtherInfo[6] = alarmInfo>>16;
	amp.trapOtherInfo[7] = alarmInfo>>8;
	amp.trapOtherInfo[8] = alarmInfo>>0;

	//tcdbg_printf("<P2> Alarm Info: %02x%02x%02x%02x%02x%02x%02x%02x\n", amp.trapOtherInfo[1], amp.trapOtherInfo[2],
	//	amp.trapOtherInfo[3], amp.trapOtherInfo[4], amp.trapOtherInfo[5], amp.trapOtherInfo[6], amp.trapOtherInfo[7], amp.trapOtherInfo[8]);

	//return sendOamAlarmMsg(&amp, 0);
	sendAlarm2EponOam(&amp);
#endif
	return 0;
}

/**
set gpon statictics times under mutex lock
**/
int gponSetStaticticsTimes(uint16 time)
{
	pthread_mutex_lock(&pmmgrGponLock);
	if(gpPmmgrGponDbAddr != NULL)
		gpPmmgrGponDbAddr->times = time;
	pthread_mutex_unlock(&pmmgrGponLock);

	return 0;	
}

/** pmmgrGetAttriNumOfMe 
return attribute NUM of ME: 
**/
uint8 pmmgrGetAttriNumOfMe(pmmgrPmMe_t *me_p)
{
	uint8 attriNum = 0;
	pmmgrPmAttriDescript_t *attriDesp;

	if(me_p == NULL || me_p->pmmgrAttriDescriptList== NULL){
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"r\n pmmgrGetAttriNumOfMe param error\n");
		return 0;
	}
	
	
	attriDesp = me_p->pmmgrAttriDescriptList;
	while(strlen(attriDesp->name) != 0){
		attriNum++;
		attriDesp++;
	}
	return attriNum;	
}



/** pmmgrGetPmMeTableSize 
return PmMeTable Size: 
**/
int pmmgrGetPmMeTableSize(void)
{
	return sizeof(pmmgrPmMeTable)/sizeof(pmmgrPmMe_t);
}

/** pmmgrGetPMInstanceByPortId 
return the PM instance by port id search
**/
pmmgrPmInst_t *pmmgrGetPMInstanceByPortId(pmmgrPmMe_t *me_p , uint16 portId)
{
	pmmgrPmInst_t *inst_p = NULL;	

	if (me_p == NULL)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrGetPMInstanceByPortId: me_p == NULL\n");
		return NULL;
	}

	inst_p = me_p->pmmgrInstList;
	while(inst_p != NULL){
		pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrGetPMInstanceByPortId portId=%x\n", inst_p->portId);

		if(inst_p->portId == portId)
			break;
		inst_p = inst_p->next;
	}

	return inst_p;
	
}

/** pmmgrGetPmMeByClassId 
return the PM me by class id search
**/
pmmgrPmMe_t *pmmgrGetPmMeByClassId(uint16 classId){
	int meIndex;
	static uint16 cacheClassId = 0;
	static pmmgrPmMe_t * cacheMe_p = NULL;
	pmmgrPmMe_t *me_p = NULL;
	int pmMeTableSize = pmmgrGetPmMeTableSize();

	if (cacheClassId == classId)
	{
		pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrGetPmMeByClassId() find the cache ME meClass=%d\n", classId);
		return cacheMe_p;
	}
	/*find the pm ME class*/
	for(meIndex = 0, me_p = pmmgrPmMeTable; meIndex <  pmMeTableSize; meIndex++, me_p++){
		if(me_p->classId == classId ){
			pmmgrPrintf(PMMGR_DEBUG_TRACE,"pmmgrGetPmMeByClassId() find the ME meClass=%d\n", classId);
			cacheClassId = classId;
			cacheMe_p = me_p;
			return me_p;//find the ME
		}
	}
	
	return NULL;
}

/** pmmgrFreePmMeInstance 
free the pm me instance
**/
int pmmgrFreePmMeInstance(pmmgrPmInst_t * inst_p)
{
	if(inst_p == NULL){
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"r\n pmmgrFreePmMeInstance: inst_p == NULL");
		return 0;
	}

	if(inst_p->pmStartData != NULL){
		free(inst_p->pmStartData);
		inst_p->pmStartData = NULL;
	}

	free(inst_p);
	inst_p = NULL;
	return 0;
}

/** pmmgrMeTableDestroy 
destory the pm me table, free the pm memory
**/
int pmmgrMeTableDestroy(void)
{
	int meIndex = 0;
	
	pmmgrPmInst_t * inst_p = NULL;
	pmmgrPmMe_t *me_p = NULL;
	int pmMeTableSize = pmmgrGetPmMeTableSize();

	for (meIndex = 0, me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
	{
		pthread_mutex_lock(me_p->pmmgrMutex);
		inst_p = me_p->pmmgrInstList;
		while (inst_p != NULL){
			me_p->pmmgrInstList = inst_p->next;
			pmmgrFreePmMeInstance(inst_p);
			inst_p = me_p->pmmgrInstList;
		}
		pthread_mutex_unlock(me_p->pmmgrMutex);
		pthread_mutex_destroy(me_p->pmmgrMutex);
		free(me_p->pmmgrMutex);
		me_p->pmmgrMutex = NULL;
	}

	return 0;
}

/** pmmgrMeTableInit 
init the pm me table
**/
int pmmgrMeTableInit(void)
{
	int meIndex = 0;
	int ret = 0;
	pmmgrPmMe_t *me_p = NULL;
	int pmMeTableSize = pmmgrGetPmMeTableSize();

	for (meIndex = 0, me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
	{
		if (me_p->pmmgrPmInitFunc != NULL)
		{
			me_p->pmmgrInstList = NULL;
			me_p->pmmgrPmInitFunc(me_p);
		}
		//to do mutex initize
		me_p->pmmgrMutex = malloc(sizeof(pthread_mutex_t));
		if(!me_p->pmmgrMutex)  
		{
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"[%s]---malloc faile for  me_p->pmmgrMutex  \n", __FUNCTION__);
			return -1;
		}
		ret = pthread_mutex_init(me_p->pmmgrMutex, NULL);
		if(ret != 0)
		{
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"pmmgrMeTableInit: mutex_init failed!\n");

			free(me_p->pmmgrMutex);
			me_p->pmmgrMutex = NULL;
			return -1;
		}
	}

	return 0;
}


/** databaseInit 
init global database
**/
#define MAX_UNSIGN_THRESHOLD		0xFFFFFFFF;
#define MIN_UNSIGN_THRESHOLD		0
#define MAX_SIGN_THRESHOLD			0xEFFFFFFF;
#define MIN_SIGN_THRESHOLD			0xFFFFFFFF;

int eponDataBaseInit()
{
	int i = 0;
	if(gpPmmgrEponDbAddr  == NULL) 
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"Create database memory failed, errno: \n") ;
		return -1 ;
	}

	//pmmgrPrintf(PMMGR_DEBUG_TRACE,"====>init threshold 111!!!\n") ; 
	memset(gpPmmgrEponDbAddr, 0, sizeof(pmmgrEponDatabase_t));

	for( i = 0; i < MAX_PORT_NUM; i++){
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamDropEvents= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamOctets= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamFrames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamBroadcastFrames= MAX_UNSIGN_THRESHOLD;			

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamMulticastFrames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamCrcErroredFrames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamUndersizeFrames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamOversizeFrames= MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamFragments= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamJabbers= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream64Frames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream127Frames= MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream255Frames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream511Frames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream1023Frames= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstream1518Frames= MAX_UNSIGN_THRESHOLD;
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamDiscards= MAX_UNSIGN_THRESHOLD;	
		
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].upstreamErrors= MAX_UNSIGN_THRESHOLD;	
				
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].highRxPower = MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].highTxPower = MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].highTxCurrent = MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].highSupplyVoltage = MAX_UNSIGN_THRESHOLD;

		gpPmmgrEponDbAddr->PMAlarmThreshold[i].highTemprature = MAX_SIGN_THRESHOLD;
		gpPmmgrEponDbAddr->PMAlarmThreshold[i].lowTemprature= MIN_SIGN_THRESHOLD;
	}

	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"====>init threshold success!!!\n") ;	
	return 0;
}


int databaseInit(uint8 mode)
{
	int ret = 0;
	//alloc the Database memory
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Create database memory.\n") ;
	if (mode == GPON_MODE)
	{
		if(!ponRestarting)
		{
			ret = pthread_mutex_init(&pmmgrGponLock, NULL);
			if (ret != 0)
			{
				pmmgrPrintf(PMMGR_DEBUG_ERROR,"Init pmmgrGponLock failed, errno: \n") ;
				return -1;
			}
		}
		gpPmmgrGponDbAddr = calloc(1, sizeof(pmmgrGponDatabase_t));
		if(gpPmmgrGponDbAddr  == NULL) 
		{
			if(!ponRestarting)
			{
				pthread_mutex_destroy(&pmmgrGponLock);
			}
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"Create database memory failed, errno: \n") ;
			return -1 ;
		}
		gpPmmgrGponDbAddr->gponAlarmCfg.sfThreshold = 0x5;
		gpPmmgrGponDbAddr->gponAlarmCfg.sfThreshold = 0x9;
		gpPmmgrGponDbAddr->gponAlarmCfg.lowerOpticalThreshold = 0xFF;
		gpPmmgrGponDbAddr->gponAlarmCfg.upperOpticalThreshold = 0xFF;
		gpPmmgrGponDbAddr->gponAlarmCfg.lowerTxPowerThreshold= 0x81;
		gpPmmgrGponDbAddr->gponAlarmCfg.upperTxPowerThreshold= 0x81;
		gpPmmgrGponDbAddr->pmMeTable = (pmmgrPmMe_t *)pmmgrPmMeTable;
		pmmgrMeTableInit();

		memset(&gpPmmgrGponAlarmStatus, 0, sizeof(gponAlarmStatus_t));
		pmmgrPrintf(PMMGR_DEBUG_TRACE,"The total share database size is %d\n", sizeof(pmmgrGponDatabase_t));
	}
	else if (mode == EPON_MODE)
	{
		gpPmmgrEponDbAddr = calloc(1, sizeof(pmmgrEponDatabase_t));
		if(gpPmmgrEponDbAddr  == NULL) 
		{
			pmmgrPrintf(PMMGR_DEBUG_ERROR,"Create database memory failed, errno: \n") ;
			return -1 ;
		}
		pmmgrPrintf(PMMGR_DEBUG_TRACE,"The total share database size is %d.\n", sizeof(pmmgrEponDatabase_t)) ;
		eponDataBaseInit();
	}
	else
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"unknown mode\n");
		return -1;
	}
	
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Create the share database memory successful.\n") ;	
	
	return 0 ;
}

/** gponPmEnable 
start gpom pm monitor
**/
int gponPmEnable(void)
{

	if(gpPmmgrGponDbAddr != NULL)
	{
		
		gpPmmgrGponDbAddr->enable = ENABLE;
		gpPmmgrGponDbAddr->alarmEnable = ENABLE;
		gpPmmgrGponDbAddr->timerId = timerPeriodicStartS(1000, gponPMCurrentTmout, NULL);
		if (gpPmmgrGponDbAddr->timerId == -1)
		{	
			pmmgrPrintf(PMMGR_DEBUG_ERROR, "gponPmEnable: timerId == TIMER_ID_NULL\n");
			return -1;
		}
	}
	return 0;
}

/** timerDestroy 
stop the timer
**/
int timerDestroy(void)
{
	int i = 0;

	if (gpPmmgrGponDbAddr != NULL)
	{
		if (gpPmmgrGponDbAddr->timerId != 0)
		{
			pmmgrPrintf(PMMGR_DEBUG_TRACE,"timerDestroy:timerPeriodicStop timerId = %x\n", gpPmmgrGponDbAddr->timerId);
			timerPeriodicStop(gpPmmgrGponDbAddr->timerId);
			gpPmmgrGponDbAddr->timerId = 0;
		}

	}
	
	if (gpPmmgrEponDbAddr != NULL)
	{
		for (i = 0; i < MAX_PORT_NUM; i++)
		{
			if (gpPmmgrEponDbAddr->PMEponControl[i].timerId != 0)
			{
				pmmgrPrintf(PMMGR_DEBUG_TRACE,"timerDestroy:timerPeriodicStop timerId = %x\n", gpPmmgrEponDbAddr->PMEponControl[i].timerId);
				timerPeriodicStop(gpPmmgrEponDbAddr->PMEponControl[i].timerId);
				gpPmmgrEponDbAddr->PMEponControl[i].timerId = 0;
			}
		}
	}

	return 0;
}

/** databaseDestroy 
destory the database, free the memory
**/
int databaseDestroy(void)
{

	if (gpPmmgrGponDbAddr != NULL)
	{
		pmmgrMeTableDestroy();
		free(gpPmmgrGponDbAddr);
		gpPmmgrGponDbAddr = NULL;
		if(!ponRestarting)
		{
			pthread_mutex_destroy(&pmmgrGponLock);
		}
	}
	
	if (gpPmmgrEponDbAddr != NULL)
	{
		free(gpPmmgrEponDbAddr);
		gpPmmgrEponDbAddr = NULL;
	}

	return 0;
}

float logFunction(float x, int base, int decimalplaces)
{
	int integer = 0;
	float return_value = 0;
	float decimalplaces_count = 0.1;
	if (x <= 0.0)
		return -40;  //assume it is x = 1 when x is 0.
	
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

	return return_value;
}
		
float transferOpticalPower(float power_mw)
{
	float log_value = 0;
	log_value = 10*logFunction(power_mw, 10, 4);
	pmmgrPrintf(PMMGR_DEBUG_TRACE, "power_mw : %f, dB_value :%f\n", power_mw, log_value);
	return log_value;
}

int lan_port_alarm_func(void)
{
    uint8 port = 0;
    uint8 avcValue = 0;
    unsigned char curStatus[ETH_LAN_PORT_NUM] = {0};
    char nodeName[32]={0};
    char tmp[4] = {0};
    
    enum{ETH_LINK_UP, ETH_LINK_DOWN, ETH_LINK_UNKNOW};
    static unsigned char preStatus[ETH_LAN_PORT_NUM] = {ETH_LINK_UNKNOW, ETH_LINK_UNKNOW, ETH_LINK_UNKNOW, ETH_LINK_UNKNOW};

	strcpy(nodeName, "XPON_Common");
	tcapi_nget(nodeName, "trafficStatus", tmp,sizeof(tmp));
    
    if(0 != strncmp(tmp, "up", sizeof(tmp))){
        return -1;
    }
    
    for (port = 0; port < ETH_LAN_PORT_NUM; port++)
    {
        curStatus[port] = macMT7530GetEthPortLos(port);     

        if (curStatus[port] == preStatus[port]){
            continue;
        }
        
        if (ETH_LINK_DOWN == curStatus[port])
        {
            avcValue = ETH_LINK_DOWN;
            sendAlarm2Omci(OMCI_ALARM_ID_PPTPEthernetUNI_LAN_LOS_ALARM, port, OMCI_ALARM_ERR);
            sendAvc2Omci(OMCI_AVC_ID_PPTPEthernetUNI_OPERATIONAL_STATE, port, &avcValue, 1);
        }
        else
        {   
            avcValue = ETH_LINK_UP;
            sendAlarm2Omci(OMCI_ALARM_ID_PPTPEthernetUNI_LAN_LOS_ALARM, port, OMCI_ALARM_RECOVERY);     
            sendAvc2Omci(OMCI_AVC_ID_PPTPEthernetUNI_OPERATIONAL_STATE, port, &avcValue, 1);
        }
        preStatus[port] = curStatus[port];
    }

    return 0;
}

/** gponAlarmHandle 
gpom alarm monitor main function
**/
int gponAlarmHandle(void)
{
	
/* ani port */
	MACRO_CMD_STRUCT(PhyTransParameters) phyTransParams;
	gponAlarmConfig_t alarmCfg = gpPmmgrGponDbAddr->gponAlarmCfg;
	float powerMw = 0.0;
	float rxPower = 0.0;
	float txPower = 0.0;
	float rxPowerLowerTh;
	float rxPowerUpperTh;
	float txPowerLowerTh;
	float txPowerUpperTh;
    int cur_rxpower_alarm = 0;
    int cur_txpower_alarm = 0;

	if(PonApi_PhyTransParameters_Get(&phyTransParams) != EXEC_OK) {
		pmmgrPrintf(PMMGR_DEBUG_NO_MSG,"Get the phyTransParams command failed\n\n") ;
		return -1 ;
	}
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Temperature:%x, Voltage:%x, TxCurrent:%x, TxPower:%x, RxPower:%x\n",
		phyTransParams.Temperature, phyTransParams.Voltage, phyTransParams.TxCurrent, phyTransParams.TxPower, phyTransParams.RxPower);

	/*SF */

	/*SD */
	powerMw = phyTransParams.RxPower * 0.0001;
	if (powerMw == 0)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN, "RxPower: 0\n");
		return -1;
	}
	rxPower = transferOpticalPower(powerMw);

	/* Low received optical power*/
	if (alarmCfg.lowerOpticalThreshold != 0xFF)
	{
		rxPowerLowerTh = alarmCfg.lowerOpticalThreshold * (-0.5);	
		if (rxPower < rxPowerLowerTh)
		    cur_rxpower_alarm = OMCI_ALARM_ID_LOW_RX_OPTICAL;
	}
	/*High received optical power */
	if (alarmCfg.upperOpticalThreshold != 0xFF)
	{
		rxPowerUpperTh = alarmCfg.upperOpticalThreshold * (-0.5);
		if (rxPower > rxPowerUpperTh)
		    cur_rxpower_alarm = OMCI_ALARM_ID_HIGH_RX_OPTICAL;
	}
	
    pmmgrPrintf(PMMGR_DEBUG_TRACE,"rxPower:%f, rxPowerLowerTh:%f, rxPowerUpperTh:%f\n",rxPower, rxPowerLowerTh, rxPowerUpperTh);
    if ((0 == cur_rxpower_alarm) && (0 != gpPmmgrGponAlarmStatus.AlarmID_RXPower))
    {
        /* abnormal -> normal */
        sendAlarm2Omci(gpPmmgrGponAlarmStatus.AlarmID_RXPower, 0, OMCI_ALARM_RECOVERY);
    }
    else if (0 != cur_rxpower_alarm)
    {
        if (0 == gpPmmgrGponAlarmStatus.AlarmID_RXPower)
        {
            /* normal -> abnormal */
            sendAlarm2Omci(cur_rxpower_alarm, 0, OMCI_ALARM_ERR);
        }
        else if (cur_rxpower_alarm != gpPmmgrGponAlarmStatus.AlarmID_RXPower)
        {
            /* abnormal -> abnormal */
            sendAlarm2Omci(gpPmmgrGponAlarmStatus.AlarmID_RXPower, 0, OMCI_ALARM_RECOVERY);
            sendAlarm2Omci(cur_rxpower_alarm, 0, OMCI_ALARM_ERR);
        }
    }  
    gpPmmgrGponAlarmStatus.AlarmID_RXPower = cur_rxpower_alarm;
    
	powerMw = phyTransParams.TxPower * 0.0001;
	if (powerMw == 0)
	{
		pmmgrPrintf(PMMGR_DEBUG_WARN, "TxPower: 0\n");
		return -1;
	}
	txPower = transferOpticalPower(powerMw);

	/* Low transmit optical power */
	if (alarmCfg.lowerTxPowerThreshold != 0x81)
	{
		txPowerLowerTh = (int8)alarmCfg.lowerTxPowerThreshold * (0.5);
		if (txPower < txPowerLowerTh)
			cur_txpower_alarm = OMCI_ALARM_ID_LOW_TX_OPTICAL;
	}
	
	/*High transmit optical power */
	if (alarmCfg.upperTxPowerThreshold != 0x81)
	{
		txPowerUpperTh = (int8)alarmCfg.upperTxPowerThreshold * (0.5);
		if (txPower > txPowerUpperTh)
			cur_txpower_alarm = OMCI_ALARM_ID_HIGH_TX_OPTICAL;
	}

    pmmgrPrintf(PMMGR_DEBUG_TRACE,"txPower:%f, txPowerLowerTh:%f, txPowerUpperTh:%f\n",txPower, txPowerLowerTh, txPowerUpperTh);
    if ((0 == cur_txpower_alarm) && (0 != gpPmmgrGponAlarmStatus.AlarmID_TXPower))
    {
        /* abnormal -> normal */
        sendAlarm2Omci(gpPmmgrGponAlarmStatus.AlarmID_TXPower, 0, OMCI_ALARM_RECOVERY);
    }
    else if (0 != cur_txpower_alarm)
    {
        if (0 == gpPmmgrGponAlarmStatus.AlarmID_TXPower)
        {
            /* normal -> abnormal */
            sendAlarm2Omci(cur_txpower_alarm, 0, OMCI_ALARM_ERR);
        }
        else if (cur_txpower_alarm != gpPmmgrGponAlarmStatus.AlarmID_TXPower)
        {
            /* abnormal -> abnormal */
            sendAlarm2Omci(gpPmmgrGponAlarmStatus.AlarmID_TXPower, 0, OMCI_ALARM_RECOVERY);
            sendAlarm2Omci(cur_txpower_alarm, 0, OMCI_ALARM_ERR);
        }
    }  
    gpPmmgrGponAlarmStatus.AlarmID_TXPower = cur_txpower_alarm;
    
    /* lan port */
    if(-1 == lan_port_alarm_func()){
        pmmgrPrintf(PMMGR_DEBUG_TRACE, "trafficStatus is down\n");
    }
    
	return 0;
}

int isEthernetFrameExtendedPMMe(pmmgrPmMe_t *me_p)
{
	int ClassId = 0;
	ClassId = me_p->classId;
	if( (ClassId == OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM ) || (ClassId == OMCI_CLASS_ID_ETHERNET_FRAME_EXTENDED_PM_64_BIT ) )
		return 1;
	return 0;
}
void updataHistoryData(pmmgrPmMe_t *me_p) {
	pmmgrPmInst_t *inst_p = NULL;
	pmmgrPmAttriDescript_t *attri_p = NULL;
	uint32 tmp4BytesData = 0;
	uint64 tmp8BytesData = 0;
	uint8 attriIndex = 0;
	uint8 * pDataSwap = NULL;
	for(inst_p = me_p->pmmgrInstList; inst_p != NULL; inst_p = inst_p->next)
	{
		if((isEthernetFrameExtendedPMMe(me_p)))
		{
			for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p ++)
			{
				if(attri_p->length == 4)
				{
					tmp4BytesData = get32(inst_p->pmCurrentData+attri_p->offset);
					tmp4BytesData += get32(inst_p->pmHistoryData+attri_p->offset);
					put32(inst_p->pmHistoryData+attri_p->offset, tmp4BytesData);
				}
				else if(attri_p->length == 8)
				{
					tmp8BytesData = get64(inst_p->pmCurrentData+attri_p->offset);
					tmp8BytesData += get64(inst_p->pmHistoryData+attri_p->offset);
					put64(inst_p->pmHistoryData+attri_p->offset, tmp8BytesData);
				}
			}
		}
		else 
		{
			pDataSwap = inst_p->pmHistoryData;
			inst_p->pmHistoryData = inst_p->pmCurrentData;
			inst_p->pmCurrentData = pDataSwap;
		}
		memset(inst_p->pmCurrentData, 0, me_p->attriTotalSize);
		if (inst_p->tcaSendMask != 0)
		{
			for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p ++)
			{
				if (attri_p->tcaId != NO_TCA)
				{
					sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_CLR);
					inst_p->tcaSendMask = 0;
					break;
				}
			}
		}
	}
}


/** gponPMCurrentTmout 
gpom pm monitor main function
**/
int gponPMCurrentTmout(void* unused)
{
	int meIndex = 0;
	int ret = -1;
	int pmMeTableSize = 0;
	pmmgrPmMe_t *me_p = NULL;
	pmmgrPmInst_t *inst_p = NULL;
	pmmgrPmAttriDescript_t *attri_p = NULL;
	uint8 attriIndex = 0;
	uint8 tmp1BytesData = 0;
	uint16 tmp2BytesData = 0;
	uint32 tmp4BytesData = 0;
	uint64 tmp8BytesData = 0;
	uint8 * pDataSwap = NULL;
#ifdef TCSUPPORT_OMCI_ALCATEL
	static uint32 oneSecPeriodicRound = 0;
	char periodEnable[10] = {0};
	char periodRound[10] = {0};
	uint8 round = 0;
#endif
#if 0
	uint8 old1BytesData = 0;
	uint16 old2BytesData = 0;
	uint32 old4BytesData = 0;
	uint64 old8BytesData = 0;
#endif
#ifdef PMGR_DEBUG
	FECPmcounters_t *pm;
#endif
	char nodeName[32]={0};
	pmMeTableSize = pmmgrGetPmMeTableSize();

//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout \n");
	ret = pthread_mutex_trylock(&pmmgrGponLock);
    if(ret)
		return -1;
	if (ponRestarting)
	{
		pthread_mutex_unlock(&pmmgrGponLock);
		return -1;
	}

	gponAlarmHandle();
#ifdef TCSUPPORT_OMCI_ALCATEL
	strcpy(nodeName, "GPON_ONU");
	if(tcapi_nget(nodeName, "PeriodEnable", periodEnable,sizeof(periodEnable)) != TCAPI_PROCESS_OK){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n %s:get PeriodEnable,  fail \n",__FUNCTION__);
	}
	if(tcapi_nget(nodeName, "PeriodRound", periodRound,sizeof(periodRound)) != TCAPI_PROCESS_OK){
		pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n %s:get periodRound,  fail \n",__FUNCTION__);
	}
	round = atoi(periodRound);
	if (!strcmp(periodEnable, "1"))
	{
		oneSecPeriodicRound++;
		if (oneSecPeriodicRound%round == 0)
		{
			sendPeriodAvc2Omci();
			oneSecPeriodicRound = 0;
		}
	}
#endif
	
	if (gpPmmgrGponDbAddr->times == 0)
	{//cycle time start
		 if (gpPmmgrGponDbAddr->enable  == ENABLE)
		 {
			for (meIndex = 0, 	me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
			{
//				pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout me_p->name = %s\n", me_p->name);
				pthread_mutex_lock(me_p->pmmgrMutex);
				inst_p = me_p->pmmgrInstList;
				pthread_mutex_unlock(me_p->pmmgrMutex);
				while(NULL != inst_p)
				{
					pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout  inst_p->portId = %x\n", inst_p->portId);
					if (me_p->getAllDataFromApi != NULL)
					{
						me_p->getAllDataFromApi(inst_p->pmStartData, inst_p->portId);
					}
					else
					{
						for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p ++)
						{
							attri_p->getDataFromApi((inst_p->pmStartData + attri_p->offset), inst_p->portId);
						}
					}
					pthread_mutex_lock(me_p->pmmgrMutex);
					inst_p = inst_p->next;
					pthread_mutex_unlock(me_p->pmmgrMutex);
				}
			}
		 }
	}
	else
	{//in cycle time 
		 if  (gpPmmgrGponDbAddr->enable  == ENABLE)
		 {
			for (meIndex = 0,	me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
			{
//				pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout me_p->name = %s\n", me_p->name);
				pthread_mutex_lock(me_p->pmmgrMutex);
				inst_p = me_p->pmmgrInstList;
				pthread_mutex_unlock(me_p->pmmgrMutex);

				while(NULL != inst_p)
				{
					pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout inst_p->portId = %x\n", inst_p->portId);
					if (me_p->getAllDataFromApi != NULL)
					{//me instance get all data from one api

						me_p->getAllDataFromApi(inst_p->pmCurrentData, inst_p->portId);

						#ifdef PMGR_DEBUG
						pm = (FECPmcounters_t*)(inst_p->pmCurrentData);
						pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout pm->correctedBytes = %x\n", pm->correctedBytes);
						pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout pm->correctedCodeWords = %x\n", pm->correctedCodeWords);
						pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout pm->uncorrectableCodeWords = %x\n", pm->uncorrectableCodeWords);
						pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout pm->totalCodeWords = %x\n", pm->totalCodeWords);	
						pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout pm->FECSeconds = %x\n", pm->FECSeconds);
						#endif
						for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p ++)
						{
							if (attri_p->length == 1)
							{
								tmp1BytesData = *(inst_p->pmCurrentData+attri_p->offset);
								if (tmp1BytesData <= *(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp1BytesData, *(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp1BytesData -= *(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x,  tmp1BytesData = %x\n", attri_p->name,  attri_p->offset, tmp1BytesData);
								*(inst_p->pmCurrentData+attri_p->offset) = tmp1BytesData;
								
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if ((*(inst_p->pmThreshold + attri_p->offset) != 0) && (tmp1BytesData >= *(inst_p->pmThreshold + attri_p->offset)))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}
							else if(attri_p->length == 2)
							{
								tmp2BytesData = get16(inst_p->pmCurrentData+attri_p->offset);
								if (tmp2BytesData <= get16(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp2BytesData, get16(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp2BytesData -= get16(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x,  tmp2BytesData = %x\n", attri_p->name,  attri_p->offset, tmp2BytesData);
								put16(inst_p->pmCurrentData+attri_p->offset, tmp2BytesData);
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if ((get16(inst_p->pmThreshold + attri_p->offset) != 0) && (tmp2BytesData >= get16(inst_p->pmThreshold + attri_p->offset)))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
			
							}
							else if(attri_p->length == 4)
							{
								tmp4BytesData = get32(inst_p->pmCurrentData+attri_p->offset);
								if (tmp4BytesData <= get32(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp4BytesData, get32(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp4BytesData -= get32(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x,  tmp4BytesData = %x\n", attri_p->name,  attri_p->offset, tmp4BytesData);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"inst_p->tcaSendMask = %x, attri_p->tcaId = %x, get32(inst_p->pmThreshold + attri_p->offset) = %x\n",inst_p->tcaSendMask, attri_p->tcaId, get32(inst_p->pmThreshold + attri_p->offset));
								put32(inst_p->pmCurrentData+attri_p->offset, tmp4BytesData);
								if((inst_p->tcaSendMask & (1<< attriIndex)) == 0) 
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if ((get32(inst_p->pmThreshold + attri_p->offset) != 0) && (tmp4BytesData >= get32(inst_p->pmThreshold + attri_p->offset)))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}
							else if(attri_p->length == 8)
							{
								tmp8BytesData = get64(inst_p->pmCurrentData+attri_p->offset);
								if (tmp8BytesData <= get64(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x\n", tmp8BytesData);
									continue;
								}
								tmp8BytesData -= get64(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x\n", attri_p->name,  attri_p->offset);
								put64(inst_p->pmCurrentData+attri_p->offset, tmp8BytesData);
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if ((get64(inst_p->pmThreshold + attri_p->offset) != 0) && (tmp8BytesData >= get64(inst_p->pmThreshold + attri_p->offset)))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}	
						}
					}
					else
					{//me instance attribute get data from api
						for(attri_p = me_p->pmmgrAttriDescriptList, attriIndex = 0; attriIndex < me_p->attriNum; attriIndex++, attri_p ++)
						{
							pmmgrPrintf(PMMGR_DEBUG_TRACE,"Enter gponPMCurrentTmout me_p->name = %s\n", me_p->name);
							attri_p->getDataFromApi((inst_p->pmCurrentData + attri_p->offset), inst_p->portId);
							if (attri_p->length == 1)
							{
								tmp1BytesData = *(inst_p->pmCurrentData+attri_p->offset);
								if (tmp1BytesData <= *(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp1BytesData, *(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp1BytesData -= *(inst_p->pmStartData + attri_p->offset);
								*(inst_p->pmCurrentData+attri_p->offset) = tmp1BytesData;
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if (tmp1BytesData >= *(inst_p->pmThreshold + attri_p->offset))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}
							else if(attri_p->length == 2)
							{
								tmp2BytesData = get16(inst_p->pmCurrentData+attri_p->offset);
								if (tmp2BytesData <= get16(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp2BytesData, get16(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp2BytesData -= get16(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x,  tmp2BytesData = %x\n", attri_p->name,  attri_p->offset, tmp2BytesData);
								put16(inst_p->pmCurrentData+attri_p->offset, tmp2BytesData);
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if (tmp2BytesData >= get16(inst_p->pmThreshold + attri_p->offset))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
			
							}
							else if(attri_p->length == 4)
							{
								tmp4BytesData = get32(inst_p->pmCurrentData+attri_p->offset);
								if (tmp4BytesData <= get32(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x, StartData = %x\n", tmp4BytesData, get32(inst_p->pmStartData + attri_p->offset));
									continue;
								}
								tmp4BytesData -= get32(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, tmp2BytesData = %x\n", attri_p->name, tmp4BytesData);

								put32(inst_p->pmCurrentData+attri_p->offset, tmp4BytesData);
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if (tmp2BytesData >= get32(inst_p->pmThreshold + attri_p->offset))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}
							else if(attri_p->length == 8)
							{
								tmp8BytesData = get64(inst_p->pmCurrentData+attri_p->offset);
								if (tmp8BytesData <= get64(inst_p->pmStartData + attri_p->offset)) //the value is overflow or current data is 0
								{
									pmmgrPrintf(PMMGR_DEBUG_WARN, "the value is overflow or current data is 0 RealtimeData = %x\n", tmp8BytesData);
									continue;
								}
								tmp8BytesData -= get64(inst_p->pmStartData + attri_p->offset);
								pmmgrPrintf(PMMGR_DEBUG_TRACE,"attri_p->name = %s, attri_p->offset= %x\n", attri_p->name,  attri_p->offset);
								put64(inst_p->pmCurrentData+attri_p->offset, tmp8BytesData);
								if ((inst_p->tcaSendMask & (1<< attriIndex)) == 0)
								{
									if (attri_p->tcaId != NO_TCA)
									{
										if (tmp2BytesData >= get64(inst_p->pmThreshold + attri_p->offset))
										{
											sendTca2Omci(attri_p->tcaId, inst_p->portId, ALARM_SET);
											inst_p->tcaSendMask |=  (1<< attriIndex);
										}
									}
								}
							}	
						}					
					}
					pthread_mutex_lock(me_p->pmmgrMutex);
					inst_p = inst_p->next;
					pthread_mutex_unlock(me_p->pmmgrMutex);
				}
			}
		 }
	}

	gpPmmgrGponDbAddr->times ++;

	if (gpPmmgrGponDbAddr->times == DEFAULT_CYCLE_TIME)
	{//cycle time end
		 if (gpPmmgrGponDbAddr->enable  == ENABLE)
		 {
		 	for (meIndex = 0, 	me_p = pmmgrPmMeTable; meIndex < pmMeTableSize; meIndex++, me_p++)
			{
				pthread_mutex_lock(me_p->pmmgrMutex);
				updataHistoryData(me_p);
				pthread_mutex_unlock(me_p->pmmgrMutex);
			}
		 }
		 gpPmmgrGponDbAddr->times = 0;
	}
	pthread_mutex_unlock(&pmmgrGponLock);
	return 0;
}
/*
***********************************************************************************************************
Epon performance monitors function
***********************************************************************************************************
*/

/** getPmDataFromApi 
get real time data from other modules api
**/
int getPmDataFromApi(PMEponCounters_ptr PMTmpData, uint16 portId)
{
//	PMTmpData->downstreamDropEvents = getDataFromApi(NULL, portId);
    char nodeName[32] = {0};

	if (portId == PON_IF_PORT){
#ifdef TCSUPPORT_PONMGR
		MACRO_CMD_STRUCT(PhyTransParameters) phyTransParams;
		MACRO_CMD_STRUCT(PWanCntStats) wanCntStats;
		
		if(PonApi_PhyTransParameters_Get(&phyTransParams) != EXEC_OK) {
			pmmgrPrintf(PMMGR_DEBUG_ERROR, "Get the phyTransParams command failed\n") ;
		}
		//pmmgrPrintf(PMMGR_DEBUG_WARN, "Get the phyTransParams command Success\n") ;
		PMTmpData->highRxPower = phyTransParams.RxPower;
		PMTmpData->highTxPower = phyTransParams.TxPower;
		PMTmpData->highTxCurrent = phyTransParams.TxCurrent;		
		PMTmpData->highSupplyVoltage = phyTransParams.Voltage;
		PMTmpData->highTemprature = phyTransParams.Temperature;

		if(PonApi_PWanCntStats_Get(&wanCntStats) != EXEC_OK) {
			pmmgrPrintf(PMMGR_DEBUG_ERROR, "PonApi_PWanCntStats_Get failed\n") ;
			return -1 ;
		}
		
		PMTmpData->downstreamDropEvents = wanCntStats.RxDropCnt;
		PMTmpData->upstreamDropEvents = wanCntStats.TxDropCnt;
		PMTmpData->downstreamOctets = wanCntStats.RxFrameLen;
		
		PMTmpData->upstreamOctets = wanCntStats.TxFrameLen;
		PMTmpData->downstreamFrames = wanCntStats.RxFrameCnt;
		PMTmpData->upstreamFrames = wanCntStats.TxFrameCnt;
		PMTmpData->downstreamBroadcastFrames = wanCntStats.RxBroadcastCnt;
		PMTmpData->upstreamBroadcastFrames = wanCntStats.TxBroadcastCnt;
		PMTmpData->downstreamMulticastFrames = wanCntStats.RxMulticastCnt;
		PMTmpData->upstreamMulticastFrames = wanCntStats.TxMulticastCnt;
		PMTmpData->downstreamCrcErroredFrames = wanCntStats.RxCrcCnt;
		PMTmpData->upstreamCrcErroredFrames = 0;
		PMTmpData->downstreamUndersizeFrames = wanCntStats.RxLess64Cnt;
		PMTmpData->upstreamUndersizeFrames = wanCntStats.TxLess64Cnt;
		PMTmpData->downstreamOversizeFrames = wanCntStats.RxMore1518Cnt;
		PMTmpData->upstreamOversizeFrames = wanCntStats.TxMore1518Cnt;
		PMTmpData->downstreamFragments = wanCntStats.RxFragFameCnt;
		PMTmpData->upstreamFragments = 0;
		PMTmpData->downstreamJabbers = wanCntStats.RxJabberFameCnt;
		PMTmpData->upstreamJabbers = 0;
		PMTmpData->downstream64Frames = wanCntStats.Rx64Cnt;
		PMTmpData->upstream64Frames = wanCntStats.Tx64Cnt;
		PMTmpData->downstream127Frames = wanCntStats.Rx65To127Cnt;
		PMTmpData->upstream127Frames = wanCntStats.Tx128To255Cnt;
		PMTmpData->downstream255Frames = wanCntStats.Rx128To255Cnt;
		PMTmpData->upstream255Frames = wanCntStats.Tx128To255Cnt;
		PMTmpData->downstream511Frames = wanCntStats.Rx256To511Cnt;
		PMTmpData->upstream511Frames = wanCntStats.Tx256To511Cnt;
		PMTmpData->downstream1023Frames = wanCntStats.Rx512To1023Cnt;
		PMTmpData->upstream1023Frames = wanCntStats.Tx512To1023Cnt;
		PMTmpData->downstream1518Frames = wanCntStats.Rx1024To1518Cnt;
		PMTmpData->upstream1518Frames = wanCntStats.Tx1024To1518Cnt;
		PMTmpData->downstreamDiscards = wanCntStats.RxDropCnt;
		PMTmpData->upstreamDiscards = wanCntStats.TxDropCnt;
		PMTmpData->downstreamErrors = wanCntStats.RxCrcCnt;
		PMTmpData->upstreamErrors = 0;	
#endif
		PMTmpData->statusChangeTimes = 0;
	}else{
		char node[20]={0}, buf[10]={0};
#ifdef TCSUPPORT_MT7530_SWITCH_API
		unsigned int rxCnt, txCnt;
		macMT7530GetPortRxDropFramesCnt(portId, &rxCnt);
		macMT7530GetPortTxDropFramesCnt(portId, &txCnt);
		PMTmpData->upstreamErrors = rxCnt;
		PMTmpData->downstreamErrors = txCnt;
		PMTmpData->upstreamDropEvents = rxCnt;
		PMTmpData->downstreamDropEvents = txCnt;

		macMT7530GetPortRxOctetsCnt(portId, &rxCnt);
		macMT7530GetPortTxOctetsCnt(portId, &txCnt);
		PMTmpData->upstreamOctets = rxCnt;
		PMTmpData->downstreamOctets = txCnt;
			
		macMT7530GetPortRxPktsCnt(portId, &rxCnt);
		macMT7530GetPortTxPktsCnt(portId, &txCnt);
		PMTmpData->upstreamFrames = rxCnt;
		PMTmpData->downstreamFrames = txCnt;
		
		macMT7530GetPortRxBroadPktsCnt(portId, &rxCnt);
		macMT7530GetPortTxBroadPktsCnt(portId, &txCnt);
		PMTmpData->upstreamBroadcastFrames = rxCnt;
		PMTmpData->downstreamBroadcastFrames = txCnt;
		
		macMT7530GetPortRxMultiPktsCnt(portId, &rxCnt);
		macMT7530GetPortTxMultiPktsCnt(portId, &txCnt);
		PMTmpData->upstreamMulticastFrames = rxCnt;
		PMTmpData->downstreamMulticastFrames = txCnt;
		
		macMT7530GetPortRxCRCPktsCnt(portId, &rxCnt);
		macMT7530GetPortTxCRCPktsCnt(portId, &txCnt);
		PMTmpData->upstreamCrcErroredFrames = rxCnt;
		PMTmpData->downstreamCrcErroredFrames = txCnt;
		
		macMT7530GetPortRxUnderSizePktsCnt(portId, &rxCnt);
		macMT7530GetPortTxUnderSizePktsCnt(portId, &txCnt);
		PMTmpData->upstreamUndersizeFrames = rxCnt;
		PMTmpData->downstreamUndersizeFrames = txCnt;

		macMT7530GetPortRxOverSizePktsCnt(portId, &rxCnt);
		macMT7530GetPortTxOverSizePktsCnt(portId, &txCnt);
		PMTmpData->upstreamOversizeFrames = rxCnt;
		PMTmpData->downstreamOversizeFrames = txCnt;
		
		macMT7530GetFragments(portId, &rxCnt);
		PMTmpData->upstreamFragments = rxCnt;
		PMTmpData->downstreamFragments = 0;

		macMT7530GetJabbers(portId, &rxCnt);
		PMTmpData->upstreamJabbers = rxCnt;
		PMTmpData->downstreamJabbers = 0;
		
		macMT7530GetPortRx64PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx64PktsCnt(portId, &txCnt);
		PMTmpData->upstream64Frames = rxCnt;
		PMTmpData->downstream64Frames = txCnt;
		
		macMT7530GetPortRx127PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx127PktsCnt(portId, &txCnt);
		PMTmpData->upstream127Frames = rxCnt;
		PMTmpData->downstream127Frames = txCnt;	
		
		macMT7530GetPortRx255PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx255PktsCnt(portId, &txCnt);
		PMTmpData->upstream255Frames = rxCnt;
		PMTmpData->downstream255Frames = txCnt;		

		macMT7530GetPortRx511PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx511PktsCnt(portId, &txCnt);
		PMTmpData->upstream511Frames = rxCnt;
		PMTmpData->downstream511Frames = txCnt;	
		
		macMT7530GetPortRx1023PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx1023PktsCnt(portId, &txCnt);
		PMTmpData->upstream1023Frames = rxCnt;
		PMTmpData->downstream1023Frames = txCnt;

		macMT7530GetPortRx1518PktsCnt(portId, &rxCnt);
		macMT7530GetPortTx1518PktsCnt(portId, &txCnt);
		PMTmpData->upstream1518Frames = rxCnt;
		PMTmpData->downstream1518Frames = txCnt;

		macMT7530GetPortRxPacketsDiscardsCnt(portId, &rxCnt);
		macMT7530GetPortTxPacketsDiscardsCnt(portId, &txCnt);
		PMTmpData->upstreamDiscards = rxCnt;
		PMTmpData->downstreamDiscards = txCnt;
#endif
#if !defined(TCSUPPORT_CT) 
		strcpy(nodeName, "Info_Oam");
		sprintf(node, "PortChangeTime%d", portId);
		if(tcapi_nget(nodeName, node, buf,sizeof(buf)) != TCAPI_PROCESS_OK){
			pmmgrPrintf(PMMGR_DEBUG_TRACE, "\r\n %s:get %s ,  fail \n",__FUNCTION__,node);
		}
		PMTmpData->statusChangeTimes = atoi(buf);
#endif
	}

	return 0;
}

/** getPmCurrentData 
calculate the current data
**/
int getPmCurrentData(PMEponCounters_ptr PMCurrentData, PMEponCounters_ptr PMTmpData, PMEponCounters_ptr PMStartData)
{
/* 0-9*/
	PMCurrentData->downstreamDropEvents = PMTmpData->downstreamDropEvents - PMStartData->downstreamDropEvents;
	PMCurrentData->upstreamDropEvents = PMTmpData->upstreamDropEvents - PMStartData->upstreamDropEvents;
	PMCurrentData->downstreamOctets = PMTmpData->downstreamOctets - PMStartData->downstreamOctets;
	PMCurrentData->upstreamOctets = PMTmpData->upstreamOctets - PMStartData->upstreamOctets;
	PMCurrentData->downstreamFrames = PMTmpData->downstreamFrames - PMStartData->downstreamFrames;
	PMCurrentData->upstreamFrames = PMTmpData->upstreamFrames - PMStartData->upstreamFrames;
	PMCurrentData->downstreamBroadcastFrames = PMTmpData->downstreamBroadcastFrames - PMStartData->downstreamBroadcastFrames;
	PMCurrentData->upstreamBroadcastFrames = PMTmpData->upstreamBroadcastFrames - PMStartData->upstreamBroadcastFrames;
	PMCurrentData->downstreamMulticastFrames = PMTmpData->downstreamMulticastFrames - PMStartData->downstreamMulticastFrames;	
	PMCurrentData->upstreamMulticastFrames = PMTmpData->upstreamMulticastFrames - PMStartData->upstreamMulticastFrames;
/* 10-19*/	
	PMCurrentData->downstreamCrcErroredFrames = PMTmpData->downstreamCrcErroredFrames - PMStartData->downstreamCrcErroredFrames;
	PMCurrentData->upstreamCrcErroredFrames = PMTmpData->upstreamCrcErroredFrames - PMStartData->upstreamCrcErroredFrames;
	PMCurrentData->downstreamUndersizeFrames = PMTmpData->downstreamUndersizeFrames - PMStartData->downstreamUndersizeFrames;
	PMCurrentData->upstreamUndersizeFrames = PMTmpData->upstreamUndersizeFrames - PMStartData->upstreamUndersizeFrames;
	PMCurrentData->downstreamOversizeFrames = PMTmpData->downstreamOversizeFrames - PMStartData->downstreamOversizeFrames;
	PMCurrentData->upstreamOversizeFrames = PMTmpData->upstreamOversizeFrames - PMStartData->upstreamOversizeFrames;
	PMCurrentData->downstreamFragments = PMTmpData->downstreamFragments - PMStartData->downstreamFragments;
	PMCurrentData->upstreamFragments = PMTmpData->upstreamFragments - PMStartData->upstreamFragments;	
	PMCurrentData->downstreamJabbers = PMTmpData->downstreamJabbers - PMStartData->downstreamJabbers;
	PMCurrentData->upstreamJabbers = PMTmpData->upstreamJabbers - PMStartData->upstreamJabbers;
/*20-29*/	
	PMCurrentData->downstream64Frames = PMTmpData->downstream64Frames - PMStartData->downstream64Frames;
	PMCurrentData->upstream64Frames = PMTmpData->upstream64Frames - PMStartData->upstream64Frames;
	PMCurrentData->downstream127Frames = PMTmpData->downstream127Frames - PMStartData->downstream127Frames;	
	PMCurrentData->upstream127Frames = PMTmpData->upstream127Frames - PMStartData->upstream127Frames;
	PMCurrentData->downstream255Frames = PMTmpData->downstream255Frames - PMStartData->downstream255Frames;
	PMCurrentData->upstream255Frames = PMTmpData->upstream255Frames - PMStartData->upstream255Frames;
	PMCurrentData->downstream511Frames = PMTmpData->downstream511Frames - PMStartData->downstream511Frames;
	PMCurrentData->upstream511Frames = PMTmpData->upstream511Frames - PMStartData->upstream511Frames;
	PMCurrentData->downstream1023Frames = PMTmpData->downstream1023Frames - PMStartData->downstream1023Frames;      
	PMCurrentData->upstream1023Frames = PMTmpData->upstream1023Frames - PMStartData->upstream1023Frames;
/* 30-36*/	
	PMCurrentData->downstream1518Frames = PMTmpData->downstream1518Frames - PMStartData->downstream1518Frames;
	PMCurrentData->upstream1518Frames = PMTmpData->upstream1518Frames - PMStartData->upstream1518Frames;
	PMCurrentData->downstreamDiscards = PMTmpData->downstreamDiscards - PMStartData->downstreamDiscards;
	PMCurrentData->upstreamDiscards = PMTmpData->upstreamDiscards - PMStartData->upstreamDiscards;
	PMCurrentData->downstreamErrors = PMTmpData->downstreamErrors - PMStartData->downstreamErrors;                
	PMCurrentData->upstreamErrors = PMTmpData->upstreamErrors - PMStartData->upstreamErrors;
	PMCurrentData->statusChangeTimes = PMTmpData->statusChangeTimes - PMStartData->statusChangeTimes;            

/*37-41*/
	PMCurrentData->highRxPower = PMTmpData->highRxPower;
	PMCurrentData->highTxPower = PMTmpData->highTxPower;
	PMCurrentData->highTxCurrent	= PMTmpData->highTxCurrent;
	PMCurrentData->highSupplyVoltage	= PMTmpData->highSupplyVoltage;
	PMCurrentData->highTemprature	= PMTmpData->highTemprature;

	return 0;
}

/** pmWarningHandle 
performance monitor waring threshold handle
**/
int pmWarningHandle(PMEponCounters_ptr PMCurrentData, PMEponCounters_ptr PMWarningThreshold, PMEponControl_ptr PMControl, uint8 portId)
{
	if (portId == PON_IF_PORT)
	{
		if ((!(PMControl->WarningSendMask & MASK_highRxPower))&&(PMControl->WarningEnableMask & MASK_highRxPower) && (PMCurrentData->highRxPower >= PMWarningThreshold->highRxPower))
		{//high rxPower
			sendAlarm2OAM(WARNING_ID_PONIF_HIGH_RXPOWER, portId, ALARM_SET, PMCurrentData->highRxPower);
			PMControl->WarningSendMask |= MASK_highRxPower;
		}
		else if ((!(PMControl->WarningSendMask & MASK_lowRxPower))&&(PMControl->WarningEnableMask & MASK_lowRxPower) && (PMCurrentData->highRxPower <= PMWarningThreshold->lowRxPower))
		{//low rxPower
			sendAlarm2OAM(WARNING_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highRxPower);
			PMControl->WarningSendMask |= MASK_lowRxPower;
		}

		if ((!(PMControl->WarningSendMask & MASK_highTxPower))&&(PMControl->WarningEnableMask & MASK_highTxPower) && (PMCurrentData->highTxPower >= PMWarningThreshold->highTxPower))
		{//high txPower
			sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TXPOWER, portId, ALARM_SET, PMCurrentData->highTxPower);
			PMControl->WarningSendMask |= MASK_highTxPower;
		}
		else if ((!(PMControl->WarningSendMask & MASK_lowTxPower))&&(PMControl->WarningEnableMask & MASK_lowTxPower) && (PMCurrentData->highTxPower <= PMWarningThreshold->lowTxPower))
		{//low txPower
			sendAlarm2OAM(WARNING_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highTxPower);
			PMControl->WarningSendMask |= MASK_lowTxPower;
		}		

		if ((!(PMControl->WarningSendMask & MASK_highTxCurrent))&&(PMControl->WarningEnableMask & MASK_highTxCurrent) && (PMCurrentData->highTxCurrent >= PMWarningThreshold->highTxCurrent))
		{//high txCurrent
			sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TXCURRENT, portId, ALARM_SET, PMCurrentData->highTxCurrent);
			PMControl->WarningSendMask |= MASK_highTxCurrent;
		}
		else if ((!(PMControl->WarningSendMask & MASK_lowTxCurrent))&&(PMControl->WarningEnableMask & MASK_lowTxCurrent) && (PMCurrentData->highTxCurrent <= PMWarningThreshold->lowTxCurrent))
		{//low txCurrent
			sendAlarm2OAM(WARNING_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highTxCurrent);
			PMControl->WarningSendMask |= MASK_lowTxCurrent;
		}

		if ((!(PMControl->WarningSendMask & MASK_highSupplyVoltage))&&(PMControl->WarningEnableMask & MASK_highSupplyVoltage) && (PMCurrentData->highSupplyVoltage >= PMWarningThreshold->highSupplyVoltage))
		{//high supplyVoltage
			sendAlarm2OAM(WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE, portId, ALARM_SET, PMCurrentData->highSupplyVoltage);
			PMControl->WarningSendMask |= MASK_highSupplyVoltage;
		}
		else if ((!(PMControl->WarningSendMask & MASK_lowSupplyVoltage))&&(PMControl->WarningEnableMask & MASK_lowSupplyVoltage) && (PMCurrentData->highSupplyVoltage <= PMWarningThreshold->lowSupplyVoltage))
		{//low supplyVoltage
			sendAlarm2OAM(WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE, portId, ALARM_SET, PMCurrentData->highSupplyVoltage);
			PMControl->WarningSendMask |= MASK_lowSupplyVoltage;
		}			

		if ((!(PMControl->WarningSendMask & MASK_highTemprature))&&(PMControl->WarningEnableMask & MASK_highTemprature) 
			&& (PMWarningThreshold->highTemprature!=0xEFFFFFFF) && ((short)PMCurrentData->highTemprature >= PMWarningThreshold->highTemprature))
		{//high temprature
			sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TEMPRATURE, portId, ALARM_SET, PMCurrentData->highTemprature);
			PMControl->WarningSendMask |= MASK_highTemprature;
		}
		else if ((!(PMControl->WarningSendMask & MASK_lowTemprature))&&(PMControl->WarningEnableMask & MASK_lowTemprature) 
			&& (PMWarningThreshold->lowTemprature!=0xFFFFFFFF) && ((short)PMCurrentData->highTemprature <= PMWarningThreshold->lowTemprature))
		{//low temprature
			sendAlarm2OAM(WARNING_ID_PONIF_LOW_TEMPRATURE, portId, ALARM_SET, PMCurrentData->highTemprature);
			PMControl->WarningSendMask |= MASK_lowTemprature;
		}		
	}

	if((PMControl->WarningEnableMask & MASK_downstreamDropEvents) )
	{//downstreamDropEvents
		if (PMCurrentData->downstreamDropEvents >= PMWarningThreshold->downstreamDropEvents)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamDropEvents) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_DROPEVENTS, portId, ALARM_SET, PMCurrentData->downstreamDropEvents);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_DROPEVENTS, portId, ALARM_SET, PMCurrentData->downstreamDropEvents);
				PMControl->WarningSendMask |= MASK_downstreamDropEvents;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamDropEvents) )
	{//upstreamDropEvents
		if (PMCurrentData->upstreamDropEvents >= PMWarningThreshold->upstreamDropEvents)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamDropEvents) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_DROPEVENTS, portId, ALARM_SET, PMCurrentData->upstreamDropEvents);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_DROPEVENTS, portId, ALARM_SET, PMCurrentData->upstreamDropEvents);
				PMControl->WarningSendMask |= MASK_upstreamDropEvents;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamCrcErroredFrames) )
	{//downstreamCrcErroredFrames
		if (PMCurrentData->downstreamCrcErroredFrames >= PMWarningThreshold->downstreamCrcErroredFrames)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamCrcErroredFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->downstreamCrcErroredFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->downstreamCrcErroredFrames);
				PMControl->WarningSendMask |= MASK_downstreamCrcErroredFrames;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamCrcErroredFrames) )
	{//upstreamCrcErroredFrames
		if (PMCurrentData->upstreamCrcErroredFrames >= PMWarningThreshold->upstreamCrcErroredFrames)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamCrcErroredFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->upstreamCrcErroredFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->upstreamCrcErroredFrames);
				PMControl->WarningSendMask |= MASK_upstreamCrcErroredFrames;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamUndersizeFrames) )
	{//downstreamUndersizeFrames
		if (PMCurrentData->downstreamUndersizeFrames >= PMWarningThreshold->downstreamUndersizeFrames)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamUndersizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamUndersizeFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamUndersizeFrames);
				PMControl->WarningSendMask |= MASK_downstreamUndersizeFrames;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamUndersizeFrames) )
	{//upstreamUndersizeFrames
		if (PMCurrentData->upstreamUndersizeFrames >= PMWarningThreshold->upstreamUndersizeFrames)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamUndersizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamUndersizeFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamUndersizeFrames);
				PMControl->WarningSendMask |= MASK_upstreamUndersizeFrames;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamOversizeFrames) )
	{//downstreamOversizeFrames
		if (PMCurrentData->downstreamOversizeFrames >= PMWarningThreshold->downstreamOversizeFrames)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamOversizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamOversizeFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamOversizeFrames);
				PMControl->WarningSendMask |= MASK_downstreamOversizeFrames;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamOversizeFrames) )
	{//upstreamOversizeFrames
		if (PMCurrentData->upstreamOversizeFrames >= PMWarningThreshold->upstreamOversizeFrames)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamOversizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamOversizeFrames);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamOversizeFrames);
				PMControl->WarningSendMask |= MASK_upstreamOversizeFrames;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamFragments) )
	{//downstreamFragments
		if (PMCurrentData->downstreamFragments >= PMWarningThreshold->downstreamFragments)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamFragments) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_FRAGMENTS, portId, ALARM_SET, PMCurrentData->downstreamFragments );
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_FRAGMENTS, portId, ALARM_SET, PMCurrentData->downstreamFragments );
				PMControl->WarningSendMask |= MASK_downstreamFragments;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamFragments) )
	{//upstreamFragments
		if (PMCurrentData->upstreamFragments >= PMWarningThreshold->upstreamFragments)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamFragments) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_FRAGMENTS, portId, ALARM_SET, PMCurrentData->upstreamFragments);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_FRAGMENTS, portId, ALARM_SET, PMCurrentData->upstreamFragments);
				PMControl->WarningSendMask |= MASK_upstreamFragments;
			}
		}
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamJabbers) )
	{//downstreamJabbers
		if (PMCurrentData->downstreamJabbers >= PMWarningThreshold->downstreamJabbers)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamJabbers) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_JABBERS, portId, ALARM_SET, PMCurrentData->downstreamJabbers);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_JABBERS, portId, ALARM_SET, PMCurrentData->downstreamJabbers);
				PMControl->WarningSendMask |= MASK_downstreamJabbers;
			}
		}
	}	

	
	if((PMControl->WarningEnableMask & MASK_upstreamJabbers) )
	{//upstreamJabbers
		if (PMCurrentData->upstreamJabbers >= PMWarningThreshold->upstreamJabbers)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamJabbers) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_JABBERS, portId, ALARM_SET, PMCurrentData->upstreamJabbers);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_JABBERS, portId, ALARM_SET, PMCurrentData->upstreamJabbers);
				PMControl->WarningSendMask |= MASK_upstreamJabbers;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamDiscards) )
	{//downstreamDiscards
		if (PMCurrentData->downstreamDiscards >= PMWarningThreshold->downstreamDiscards)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamDiscards) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_DISCARDS, portId, ALARM_SET, PMCurrentData->downstreamDiscards);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_DISCARDS, portId, ALARM_SET, PMCurrentData->downstreamDiscards);
				PMControl->WarningSendMask |= MASK_downstreamDiscards;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamDiscards) )
	{//upstreamDiscards
		if (PMCurrentData->upstreamDiscards >= PMWarningThreshold->upstreamDiscards)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamDiscards) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_DISCARDS, portId, ALARM_SET, PMCurrentData->upstreamDiscards);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_DISCARDS, portId, ALARM_SET, PMCurrentData->upstreamDiscards);
				PMControl->WarningSendMask |= MASK_upstreamDiscards;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamErrors) )
	{//downstreamErrors
		if (PMCurrentData->downstreamErrors >= PMWarningThreshold->downstreamErrors)
		{
			if ((PMControl->WarningSendMask & MASK_downstreamErrors) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				PMControl->WarningSendMask |= MASK_downstreamErrors;
			}
		}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamErrors) )
	{//downstreamErrors
		if (PMCurrentData->downstreamErrors >= PMWarningThreshold->downstreamErrors)
		{
			if ((PMControl->WarningSendMask & MASK_upstreamErrors) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				PMControl->WarningSendMask |= MASK_upstreamErrors;
			}
		}
	}


	if((PMControl->WarningEnableMask & MASK_statusChangeTimes) )
	{//statusChangeTimes
		if (PMCurrentData->statusChangeTimes >= PMWarningThreshold->statusChangeTimes)
		{
			if ((PMControl->WarningSendMask & MASK_statusChangeTimes) == 0)
			{	
				sendAlarm2OAM(WARNING_ID_PORT_STATUSCHANGETIMES, portId, ALARM_SET, PMCurrentData->statusChangeTimes);
				PMControl->WarningSendMask |= MASK_statusChangeTimes;
			}
		}
	}
	return 0;
}

/** pmAlarmHandle 
performance monitor alarm threshold handle
**/
int pmAlarmHandle(PMEponCounters_ptr PMCurrentData, PMEponCounters_ptr PMAlarmThreshold, PMEponControl_ptr PMControl, uint8 portId)
{
	if (portId == PON_IF_PORT)
	{
		if ((!(PMControl->AlarmSendMask & MASK_highRxPower))&&(PMControl->AlarmEnableMask & MASK_highRxPower) && (PMCurrentData->highRxPower >= PMAlarmThreshold->highRxPower))
		{//high rxPower
			sendAlarm2OAM(ALARM_ID_PONIF_HIGH_RXPOWER, portId, ALARM_SET, PMCurrentData->highRxPower);
			PMControl->AlarmSendMask |= MASK_highRxPower;
		}
		else if((!(PMControl->AlarmSendMask & MASK_lowRxPower))&&(PMControl->AlarmEnableMask & MASK_lowRxPower) && (PMCurrentData->highRxPower <= PMAlarmThreshold->lowRxPower))
		{//low rxPower
			sendAlarm2OAM(ALARM_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highRxPower);
			PMControl->AlarmSendMask |= MASK_lowRxPower;
		}

		if ((!(PMControl->AlarmSendMask & MASK_highTxPower))&&(PMControl->AlarmEnableMask & MASK_highTxPower) && (PMCurrentData->highTxPower >= PMAlarmThreshold->highTxPower))
		{//high txPower
			sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TXPOWER, portId, ALARM_SET, PMCurrentData->highTxPower);
			PMControl->AlarmSendMask |= MASK_highTxPower;
		}
		else if ((!(PMControl->AlarmSendMask & MASK_lowTxPower))&&(PMControl->AlarmEnableMask & MASK_lowTxPower) && (PMCurrentData->highTxPower <= PMAlarmThreshold->lowTxPower))
		{//low txPower
			sendAlarm2OAM(ALARM_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highTxPower);
			PMControl->AlarmSendMask |= MASK_lowTxPower;
		}		

		if ((!(PMControl->AlarmSendMask & MASK_highTxCurrent))&&(PMControl->AlarmEnableMask & MASK_highTxCurrent) && (PMCurrentData->highTxCurrent >= PMAlarmThreshold->highTxCurrent))
		{//high txCurrent
			sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TXCURRENT, portId, ALARM_SET, PMCurrentData->highTxCurrent);
			PMControl->AlarmSendMask |= MASK_highTxCurrent;
		}
		else if ((!(PMControl->AlarmSendMask & MASK_lowTxCurrent))&&(PMControl->AlarmEnableMask & MASK_lowTxCurrent) && (PMCurrentData->highTxCurrent <= PMAlarmThreshold->lowTxCurrent))
		{//low txCurrent
			sendAlarm2OAM(ALARM_ID_PONIF_LOW_RXPOWER, portId, ALARM_SET, PMCurrentData->highTxCurrent);
			PMControl->AlarmSendMask |= MASK_lowTxCurrent;
		}

		if ((!(PMControl->AlarmSendMask & MASK_highSupplyVoltage))&&(PMControl->AlarmEnableMask & MASK_highSupplyVoltage) && (PMCurrentData->highSupplyVoltage >= PMAlarmThreshold->highSupplyVoltage))
		{//high supplyVoltage
			sendAlarm2OAM(ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE, portId, ALARM_SET, PMCurrentData->highSupplyVoltage);
			PMControl->AlarmSendMask |= MASK_highSupplyVoltage;
		}
		else if ((!(PMControl->AlarmSendMask & MASK_lowSupplyVoltage))&&(PMControl->AlarmEnableMask & MASK_lowSupplyVoltage) && (PMCurrentData->highSupplyVoltage <= PMAlarmThreshold->lowSupplyVoltage))
		{//low supplyVoltage
			sendAlarm2OAM(ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE, portId, ALARM_SET, PMCurrentData->highSupplyVoltage);
			PMControl->AlarmSendMask |= MASK_lowSupplyVoltage;
		}			

		if ((!(PMControl->AlarmSendMask & MASK_highTemprature))&&(PMControl->AlarmEnableMask & MASK_highTemprature) 
			&& (PMAlarmThreshold->highTemprature!=0xEFFFFFFF) && ((short)PMCurrentData->highTemprature >= PMAlarmThreshold->highTemprature))
		{//high temprature
			sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TEMPRATURE, portId, ALARM_SET, PMCurrentData->highTemprature);
			PMControl->AlarmSendMask |= MASK_highTemprature;
		}
		else if ((!(PMControl->AlarmSendMask & MASK_lowTemprature))&&(PMControl->AlarmEnableMask & MASK_lowTemprature) 
			&& (PMAlarmThreshold->lowTemprature!=0xFFFFFFFF) && ((short)PMCurrentData->highTemprature <= PMAlarmThreshold->lowTemprature))
		{//low temprature
			sendAlarm2OAM(ALARM_ID_PONIF_LOW_TEMPRATURE, portId, ALARM_SET, PMCurrentData->highTemprature);
			PMControl->AlarmSendMask |= MASK_lowTemprature;
		}				
	}

//	pmmgrPrintf(PMMGR_DEBUG_TRACE,"PMControl->AlarmEnableMask = %x\n", PMControl->AlarmEnableMask);
	if((PMControl->AlarmEnableMask & MASK_downstreamDropEvents) )
	{
		if (PMCurrentData->downstreamDropEvents >= PMAlarmThreshold->downstreamDropEvents)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamDropEvents) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_DROPEVENTS, portId, ALARM_SET, PMCurrentData->downstreamDropEvents);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_DROPEVENTS, portId, ALARM_SET, PMCurrentData->downstreamDropEvents);
				PMControl->AlarmSendMask|= MASK_downstreamDropEvents;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamDropEvents))
	{//upstreamDropEvents
		if (PMCurrentData->upstreamDropEvents >= PMAlarmThreshold->upstreamDropEvents)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamDropEvents) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_DROPEVENTS, portId, ALARM_SET, PMCurrentData->upstreamDropEvents);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_DROPEVENTS, portId, ALARM_SET, PMCurrentData->upstreamDropEvents);
				PMControl->AlarmSendMask |= MASK_upstreamDropEvents;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamCrcErroredFrames) )
	{//downstreamCrcErroredFrames
		if (PMCurrentData->downstreamCrcErroredFrames >= PMAlarmThreshold->downstreamCrcErroredFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamCrcErroredFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->downstreamCrcErroredFrames);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->downstreamCrcErroredFrames);
				PMControl->AlarmSendMask |= MASK_downstreamCrcErroredFrames;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamCrcErroredFrames) )
	{//upstreamCrcErroredFrames
		if (PMCurrentData->upstreamCrcErroredFrames >= PMAlarmThreshold->upstreamCrcErroredFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamCrcErroredFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->upstreamCrcErroredFrames);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_CRCERRORFRAMES, portId, ALARM_SET, PMCurrentData->upstreamCrcErroredFrames);
				PMControl->AlarmSendMask |= MASK_upstreamCrcErroredFrames;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamUndersizeFrames) )
	{//downstreamUndersizeFrames
		if (PMCurrentData->downstreamUndersizeFrames >= PMAlarmThreshold->downstreamUndersizeFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamUndersizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamUndersizeFrames);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamUndersizeFrames);
				PMControl->AlarmSendMask |= MASK_downstreamUndersizeFrames;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamUndersizeFrames) )
	{//upstreamUndersizeFrames
		if (PMCurrentData->upstreamUndersizeFrames >= PMAlarmThreshold->upstreamUndersizeFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamUndersizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamUndersizeFrames );
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_UNDERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamUndersizeFrames );
				PMControl->AlarmSendMask |= MASK_upstreamUndersizeFrames;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamOversizeFrames) )
	{//downstreamOversizeFrames
		if (PMCurrentData->downstreamOversizeFrames >= PMAlarmThreshold->downstreamOversizeFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamOversizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamOversizeFrames);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->downstreamOversizeFrames);
				PMControl->AlarmSendMask |= MASK_downstreamOversizeFrames;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamOversizeFrames) )
	{//upstreamOversizeFrames
		if (PMCurrentData->upstreamOversizeFrames >= PMAlarmThreshold->upstreamOversizeFrames)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamOversizeFrames) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamOversizeFrames);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_OVERSIZEFRAMES, portId, ALARM_SET, PMCurrentData->upstreamOversizeFrames);
				PMControl->AlarmSendMask |= MASK_upstreamOversizeFrames;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamFragments) )
	{//downstreamFragments
		if (PMCurrentData->downstreamFragments >= PMAlarmThreshold->downstreamFragments)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamFragments) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_FRAGMENTS, portId, ALARM_SET, PMCurrentData->downstreamFragments);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_FRAGMENTS, portId, ALARM_SET, PMCurrentData->downstreamFragments);
				PMControl->AlarmSendMask |= MASK_downstreamFragments;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamFragments) )
	{//upstreamFragments
		if (PMCurrentData->upstreamFragments >= PMAlarmThreshold->upstreamFragments)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamFragments) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_FRAGMENTS, portId, ALARM_SET, PMCurrentData->upstreamFragments);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_FRAGMENTS, portId, ALARM_SET, PMCurrentData->upstreamFragments);
				PMControl->AlarmSendMask |= MASK_upstreamFragments;
			}
		}
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamJabbers) )
	{//downstreamJabbers
		if (PMCurrentData->downstreamJabbers >= PMAlarmThreshold->downstreamJabbers)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamJabbers) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_JABBERS, portId, ALARM_SET, PMCurrentData->downstreamJabbers);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_JABBERS, portId, ALARM_SET, PMCurrentData->downstreamJabbers);
				PMControl->AlarmSendMask |= MASK_downstreamJabbers;
			}
		}
	}	

	
	if((PMControl->AlarmEnableMask & MASK_upstreamJabbers) )
	{//upstreamJabbers
		if (PMCurrentData->upstreamJabbers >= PMAlarmThreshold->upstreamJabbers)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamJabbers) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_JABBERS, portId, ALARM_SET, PMCurrentData->upstreamJabbers); 
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_JABBERS, portId, ALARM_SET, PMCurrentData->upstreamJabbers);
				PMControl->AlarmSendMask |= MASK_upstreamJabbers;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamDiscards) )
	{//downstreamDiscards
		if (PMCurrentData->downstreamDiscards >= PMAlarmThreshold->downstreamDiscards)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamDiscards) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_DISCARDS, portId, ALARM_SET, PMCurrentData->downstreamDiscards);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_DISCARDS, portId, ALARM_SET, PMCurrentData->downstreamDiscards);
				PMControl->AlarmSendMask |= MASK_downstreamDiscards;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamDiscards) )
	{//upstreamDiscards
		if (PMCurrentData->upstreamDiscards >= PMAlarmThreshold->upstreamDiscards)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamDiscards) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_DISCARDS, portId, ALARM_SET, PMCurrentData->upstreamDiscards);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_DISCARDS, portId, ALARM_SET, PMCurrentData->upstreamDiscards);
				PMControl->AlarmSendMask |= MASK_upstreamDiscards;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamErrors) )
	{//downstreamErrors
		if (PMCurrentData->downstreamErrors >= PMAlarmThreshold->downstreamErrors)
		{
			if ((PMControl->AlarmSendMask & MASK_downstreamErrors) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				PMControl->AlarmSendMask |= MASK_downstreamErrors;
			}
		}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamErrors) )
	{//downstreamErrors
		if (PMCurrentData->downstreamErrors >= PMAlarmThreshold->downstreamErrors)
		{
			if ((PMControl->AlarmSendMask & MASK_upstreamErrors) == 0)
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_ERRORS, portId, ALARM_SET, PMCurrentData->downstreamErrors);
				PMControl->AlarmSendMask |= MASK_upstreamErrors;
			}
		}
	}


	if((PMControl->AlarmEnableMask & MASK_statusChangeTimes) )
	{//statusChangeTimes
		if (PMCurrentData->statusChangeTimes >= PMAlarmThreshold->statusChangeTimes)
		{
			if ((PMControl->AlarmSendMask & MASK_statusChangeTimes) == 0)
			{	
				sendAlarm2OAM(ALARM_ID_PORT_STATUSCHANGETIMES, portId, ALARM_SET, PMCurrentData->statusChangeTimes);
				PMControl->AlarmSendMask |= MASK_statusChangeTimes;
			}
		}
	}
	return 0;
}

/** pmClearWarningHandle 
pon if performance monitor clear warning threshold handle
**/
int pmClearWarningHandle(PMEponCounters_ptr PMCurrentData, PMEponCounters_ptr PMClearWarningThreshold, PMEponControl_ptr PMControl, uint8 portId)
{
	if (portId == PON_IF_PORT)
	{
		if (PMControl->WarningEnableMask & MASK_highRxPower)
		{//clear high rxPower
			if ((PMControl->WarningSendMask & MASK_highRxPower) && (PMCurrentData->highRxPower < PMClearWarningThreshold->highRxPower))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_HIGH_RXPOWER, portId, ALARM_CLR, PMCurrentData->highRxPower);				
				PMControl->WarningSendMask &= ~MASK_highRxPower;
			}				
		}

		if (PMControl->WarningEnableMask & MASK_lowRxPower)
		{//clear low rxPower
			if ((PMControl->WarningSendMask & MASK_lowRxPower) && (PMCurrentData->highRxPower > PMClearWarningThreshold->lowRxPower))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_LOW_RXPOWER, portId, ALARM_CLR, PMCurrentData->highRxPower);				
				PMControl->WarningSendMask &= ~MASK_lowRxPower;
			}
		}

		if (PMControl->WarningEnableMask & MASK_highTxPower)
		{//clear high txPower
			if ((PMControl->WarningSendMask & MASK_highTxPower) && (PMCurrentData->highTxPower < PMClearWarningThreshold->highTxPower))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TXPOWER, portId, ALARM_CLR, PMCurrentData->highTxPower);				
				PMControl->WarningSendMask &= ~MASK_highTxPower;
			}
		}

		if (PMControl->WarningEnableMask & MASK_lowTxPower)
		{//clear low txPower
			if ((PMControl->WarningSendMask & MASK_lowTxPower) && (PMCurrentData->highTxPower > PMClearWarningThreshold->lowTxPower))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_LOW_TXPOWER, portId, ALARM_CLR, PMCurrentData->highTxPower);				
				PMControl->WarningSendMask &= ~MASK_lowTxPower;
			}
		}

		if (PMControl->WarningEnableMask & MASK_highTxCurrent)
		{//clear high txCurrent
			if ((PMControl->WarningSendMask & MASK_highTxCurrent) && (PMCurrentData->highTxCurrent < PMClearWarningThreshold->highTxCurrent))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TXCURRENT, portId, ALARM_CLR, PMCurrentData->highTxCurrent);				
				PMControl->WarningSendMask &= ~MASK_highTxCurrent;
			}
		}

		if (PMControl->WarningEnableMask & MASK_lowTxCurrent)
		{//clear low txCurrent
			if ((PMControl->WarningSendMask & MASK_lowTxCurrent) && (PMCurrentData->highTxCurrent > PMClearWarningThreshold->lowTxCurrent))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_LOW_TXCURRENT, portId, ALARM_CLR, PMCurrentData->highTxCurrent);				
				PMControl->WarningSendMask &= ~MASK_lowTxCurrent;
			}
		}		

		if (PMControl->WarningEnableMask & MASK_highSupplyVoltage)
		{//clear high supplyVoltage
			if ((PMControl->WarningSendMask & MASK_highSupplyVoltage) && (PMCurrentData->highSupplyVoltage < PMClearWarningThreshold->highSupplyVoltage))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE, portId, ALARM_CLR, PMCurrentData->highSupplyVoltage);		
				PMControl->WarningSendMask &= ~MASK_highSupplyVoltage;
			}
		}

		if (PMControl->WarningEnableMask & MASK_lowSupplyVoltage)
		{//clear low supplyVoltage
			if ((PMControl->WarningSendMask & MASK_lowSupplyVoltage) && (PMCurrentData->highSupplyVoltage > PMClearWarningThreshold->lowSupplyVoltage))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE, portId, ALARM_CLR, PMCurrentData->highSupplyVoltage);		
				PMControl->WarningSendMask &= ~MASK_lowSupplyVoltage;
			}
		}

		if (PMControl->WarningEnableMask & MASK_highTemprature)
		{//clear high temprature
			if ((PMClearWarningThreshold->highTemprature!=0xEFFFFFFF)&&(PMControl->WarningSendMask & MASK_highTemprature) && ((short)PMCurrentData->highTemprature < PMClearWarningThreshold->highTemprature))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_HIGH_TEMPRATURE, portId, ALARM_CLR, PMCurrentData->highTemprature);		
				PMControl->WarningSendMask &= ~MASK_highTemprature;
			}
		}
		
		if (PMControl->WarningEnableMask & MASK_lowTemprature)
		{//clear low temprature
			if ((PMClearWarningThreshold->lowTemprature!=0xFFFFFFFF)&&(PMControl->WarningSendMask & MASK_lowTemprature) && ((short)PMCurrentData->highTemprature > PMClearWarningThreshold->lowTemprature))
			{
				sendAlarm2OAM(WARNING_ID_PONIF_LOW_TEMPRATURE, portId, ALARM_CLR, PMCurrentData->highTemprature); 
				PMControl->WarningSendMask &= ~MASK_lowTemprature;
			}
		}


	}
	return 0;
}

/** pmClearAlarmHandle 
pon if performance monitor clear alarm threshold handle
**/
int pmClearAlarmHandle(PMEponCounters_ptr PMCurrentData, PMEponCounters_ptr PMClearAlarmThreshold, PMEponControl_ptr PMControl, uint8 portId)
{
	if (portId == PON_IF_PORT)
	{
		if (PMControl->AlarmEnableMask & MASK_highRxPower)
		{//clear high rxPower
			if ((PMControl->AlarmSendMask & MASK_highRxPower) && (PMCurrentData->highRxPower < PMClearAlarmThreshold->highRxPower))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_HIGH_RXPOWER, portId, ALARM_CLR, PMCurrentData->highRxPower);				
				PMControl->AlarmSendMask &= ~MASK_highRxPower;
			}				
		}

		if (PMControl->AlarmEnableMask & MASK_lowRxPower)
		{//clear low rxPower
			if ((PMControl->AlarmSendMask & MASK_lowRxPower) && (PMCurrentData->highRxPower > PMClearAlarmThreshold->lowRxPower))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_LOW_RXPOWER, portId, ALARM_CLR, PMCurrentData->highRxPower);				
				PMControl->AlarmSendMask &= ~MASK_lowRxPower;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_highTxPower)
		{//clear high txPower
			if ((PMControl->AlarmSendMask & MASK_highTxPower) && (PMCurrentData->highTxPower < PMClearAlarmThreshold->highTxPower))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TXPOWER, portId, ALARM_CLR, PMCurrentData->highTxPower);				
				PMControl->AlarmSendMask &= ~MASK_highTxPower;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_lowTxPower)
		{//clear low txPower
			if ((PMControl->AlarmSendMask & MASK_lowTxPower) && (PMCurrentData->highTxPower > PMClearAlarmThreshold->lowTxPower))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_LOW_TXPOWER, portId, ALARM_CLR, PMCurrentData->highTxPower);				
				PMControl->AlarmSendMask &= ~MASK_lowTxPower;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_highTxCurrent)
		{//clear high txCurrent
			if ((PMControl->AlarmSendMask & MASK_highTxCurrent) && (PMCurrentData->highTxCurrent < PMClearAlarmThreshold->highTxCurrent))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TXCURRENT, portId, ALARM_CLR, PMCurrentData->highTxCurrent);				
				PMControl->AlarmSendMask &= ~MASK_highTxCurrent;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_lowTxCurrent)
		{//clear low txCurrent
			if ((PMControl->AlarmSendMask & MASK_lowTxCurrent) && (PMCurrentData->highTxCurrent > PMClearAlarmThreshold->lowTxCurrent))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_LOW_TXCURRENT, portId, ALARM_CLR, PMCurrentData->highTxCurrent);				
				PMControl->AlarmSendMask &= ~MASK_lowTxCurrent;
			}
		}		

		if (PMControl->AlarmEnableMask & MASK_highSupplyVoltage)
		{//clear high supplyVoltage
			if ((PMControl->AlarmSendMask & MASK_highSupplyVoltage) && (PMCurrentData->highSupplyVoltage < PMClearAlarmThreshold->highSupplyVoltage))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE, portId, ALARM_CLR, PMCurrentData->highSupplyVoltage);		
				PMControl->AlarmSendMask &= ~MASK_highSupplyVoltage;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_lowSupplyVoltage)
		{//clear low supplyVoltage
			if ((PMControl->AlarmSendMask & MASK_lowSupplyVoltage) && (PMCurrentData->highSupplyVoltage > PMClearAlarmThreshold->lowSupplyVoltage))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE, portId, ALARM_CLR, PMCurrentData->highSupplyVoltage);		
				PMControl->AlarmSendMask &= ~MASK_lowSupplyVoltage;
			}
		}

		if (PMControl->AlarmEnableMask & MASK_highTemprature)
		{//clear high temprature
			if ((PMClearAlarmThreshold->highTemprature!=0xEFFFFFFF) && (PMControl->AlarmSendMask & MASK_highTemprature) && ((short)PMCurrentData->highTemprature < PMClearAlarmThreshold->highTemprature))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_HIGH_TEMPRATURE, portId, ALARM_CLR, PMCurrentData->highTemprature);		
				PMControl->AlarmSendMask &= ~MASK_highTemprature;
			}
		}
		
		if (PMControl->AlarmEnableMask & MASK_lowTemprature)
		{//clear low temprature
			if ((PMClearAlarmThreshold->lowTemprature!=0xFFFFFFFF) && (PMControl->AlarmSendMask & MASK_lowTemprature) && ((short)PMCurrentData->highTemprature > PMClearAlarmThreshold->lowTemprature))
			{
				sendAlarm2OAM(ALARM_ID_PONIF_LOW_TEMPRATURE, portId, ALARM_CLR, PMCurrentData->highTemprature); 
				PMControl->AlarmSendMask &= ~MASK_lowTemprature;
			}
		}
	}
	return 0;	
}
int pmWarningClearHandle(PMEponControl_ptr PMControl, uint8 portId)
{
	if((PMControl->WarningEnableMask & MASK_downstreamDropEvents) )
	{//downstreamDropEvents
		if ((PMControl->WarningSendMask & MASK_downstreamDropEvents) )
		{	
			if (portId == PON_IF_PORT)
				sendAlarm2OAM(WARNING_ID_PONIF_DS_DROPEVENTS, portId, ALARM_CLR, 0);
			else
				sendAlarm2OAM(WARNING_ID_PORT_DS_DROPEVENTS, portId, ALARM_CLR, 0);
		}
	}


	if((PMControl->WarningEnableMask & MASK_upstreamDropEvents) )
	{//upstreamDropEvents
			if ((PMControl->WarningSendMask & MASK_upstreamDropEvents) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_DROPEVENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_DROPEVENTS, portId, ALARM_CLR, 0);
			}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamCrcErroredFrames) )
	{//downstreamCrcErroredFrames
			if ((PMControl->WarningSendMask & MASK_downstreamCrcErroredFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamCrcErroredFrames;
			}
		
	}

	if((PMControl->WarningEnableMask & MASK_upstreamCrcErroredFrames) )
	{//upstreamCrcErroredFrames
			if ((PMControl->WarningSendMask & MASK_upstreamCrcErroredFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamCrcErroredFrames;
			}
		
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamUndersizeFrames) )
	{//downstreamUndersizeFrames
			if ((PMControl->WarningSendMask & MASK_downstreamUndersizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamUndersizeFrames;
			}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamUndersizeFrames) )
	{//upstreamUndersizeFrames
			if ((PMControl->WarningSendMask & MASK_upstreamUndersizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamUndersizeFrames;
			}
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamOversizeFrames) )
	{//downstreamOversizeFrames
			if ((PMControl->WarningSendMask & MASK_downstreamOversizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamOversizeFrames;
			}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamOversizeFrames) )
	{//upstreamOversizeFrames
			if ((PMControl->WarningSendMask & MASK_upstreamOversizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamOversizeFrames;
			}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamFragments) )
	{//downstreamFragments
			if ((PMControl->WarningSendMask & MASK_downstreamFragments) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_FRAGMENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_FRAGMENTS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamFragments;
			}
	}	

	if((PMControl->WarningEnableMask & MASK_upstreamFragments) )
	{//upstreamFragments
			if ((PMControl->WarningSendMask & MASK_upstreamFragments) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_FRAGMENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_FRAGMENTS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamFragments;
			}
	}	

	if((PMControl->WarningEnableMask & MASK_downstreamJabbers) )
	{//downstreamJabbers
			if ((PMControl->WarningSendMask & MASK_downstreamJabbers) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_JABBERS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_JABBERS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamJabbers;
			}
	}	

	
	if((PMControl->WarningEnableMask & MASK_upstreamJabbers) )
	{//upstreamJabbers
			if ((PMControl->WarningSendMask & MASK_upstreamJabbers) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_JABBERS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_JABBERS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamJabbers;
			}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamDiscards) )
	{//downstreamDiscards
			if ((PMControl->WarningSendMask & MASK_downstreamDiscards) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_DISCARDS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_DISCARDS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamDiscards;
			}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamDiscards) )
	{//upstreamDiscards
			if ((PMControl->WarningSendMask & MASK_upstreamDiscards) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_DISCARDS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_DISCARDS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamDiscards;
			}
	}

	if((PMControl->WarningEnableMask & MASK_downstreamErrors) )
	{//downstreamErrors
			if ((PMControl->WarningSendMask & MASK_downstreamErrors) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_DS_ERRORS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_DS_ERRORS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_downstreamErrors;
			}
	}

	if((PMControl->WarningEnableMask & MASK_upstreamErrors) )
	{//upstreamErrors
			if ((PMControl->WarningSendMask & MASK_upstreamErrors) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(WARNING_ID_PONIF_US_ERRORS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(WARNING_ID_PORT_US_ERRORS, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_upstreamErrors;
			}
	}


	if((PMControl->WarningEnableMask & MASK_statusChangeTimes) )
	{//statusChangeTimes
			if ((PMControl->WarningSendMask & MASK_statusChangeTimes) )
			{	
				sendAlarm2OAM(WARNING_ID_PORT_STATUSCHANGETIMES, portId, ALARM_CLR, 0);
				PMControl->WarningSendMask |= MASK_statusChangeTimes;
			}
	}
		 
	PMControl->AlarmSendMask= 0;

	return 0;
}

int pmAlarmClearHandle(PMEponControl_ptr PMControl, uint8 portId)
{

	if((PMControl->AlarmEnableMask & MASK_downstreamDropEvents) )
	{//downstreamDropEvents
		if ((PMControl->AlarmSendMask & MASK_downstreamDropEvents) )
		{	
			if (portId == PON_IF_PORT)
				sendAlarm2OAM(ALARM_ID_PONIF_DS_DROPEVENTS, portId, ALARM_CLR, 0);
			else
				sendAlarm2OAM(ALARM_ID_PORT_DS_DROPEVENTS, portId, ALARM_CLR, 0);
		}
	}


	if((PMControl->AlarmEnableMask & MASK_upstreamDropEvents) )
	{//upstreamDropEvents
			if ((PMControl->AlarmSendMask & MASK_upstreamDropEvents) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_DROPEVENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_DROPEVENTS, portId, ALARM_CLR, 0);
			}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamCrcErroredFrames) )
	{//downstreamCrcErroredFrames

			if ((PMControl->AlarmSendMask & MASK_downstreamCrcErroredFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamCrcErroredFrames;
			}
		
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamCrcErroredFrames) )
	{//upstreamCrcErroredFrames
			if ((PMControl->AlarmSendMask & MASK_upstreamCrcErroredFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_CRCERRORFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamCrcErroredFrames;
			}
		
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamUndersizeFrames) )
	{//downstreamUndersizeFrames
			if ((PMControl->AlarmSendMask & MASK_downstreamUndersizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamUndersizeFrames;
			}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamUndersizeFrames) )
	{//upstreamUndersizeFrames
			if ((PMControl->AlarmSendMask & MASK_upstreamUndersizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_UNDERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamUndersizeFrames;
			}
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamOversizeFrames) )
	{//downstreamOversizeFrames
			if ((PMControl->AlarmSendMask & MASK_downstreamOversizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamOversizeFrames;
			}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamOversizeFrames) )
	{//upstreamOversizeFrames
			if ((PMControl->AlarmSendMask & MASK_upstreamOversizeFrames) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_OVERSIZEFRAMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamOversizeFrames;
			}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamFragments) )
	{//downstreamFragments
			if ((PMControl->AlarmSendMask & MASK_downstreamFragments) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_FRAGMENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_FRAGMENTS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamFragments;
			}
	}	

	if((PMControl->AlarmEnableMask & MASK_upstreamFragments) )
	{//upstreamFragments
			if ((PMControl->AlarmSendMask & MASK_upstreamFragments) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_FRAGMENTS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_FRAGMENTS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamFragments;
			}
	}	

	if((PMControl->AlarmEnableMask & MASK_downstreamJabbers) )
	{//downstreamJabbers
			if ((PMControl->AlarmSendMask & MASK_downstreamJabbers) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_JABBERS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_JABBERS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamJabbers;
			}
	}	

	
	if((PMControl->AlarmEnableMask & MASK_upstreamJabbers) )
	{//upstreamJabbers
			if ((PMControl->AlarmSendMask & MASK_upstreamJabbers) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_JABBERS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_JABBERS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamJabbers;
			}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamDiscards) )
	{//downstreamDiscards
			if ((PMControl->AlarmSendMask & MASK_downstreamDiscards) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_DISCARDS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_DISCARDS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamDiscards;
			}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamDiscards) )
	{//upstreamDiscards
			if ((PMControl->AlarmSendMask & MASK_upstreamDiscards) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_DISCARDS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_DISCARDS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamDiscards;
			}
	}

	if((PMControl->AlarmEnableMask & MASK_downstreamErrors) )
	{//downstreamErrors
			if ((PMControl->AlarmSendMask & MASK_downstreamErrors) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_DS_ERRORS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_DS_ERRORS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_downstreamErrors;
			}
	}

	if((PMControl->AlarmEnableMask & MASK_upstreamErrors) )
	{//upstreamErrors
			if ((PMControl->AlarmSendMask & MASK_upstreamErrors) )
			{	
				if (portId == PON_IF_PORT)
					sendAlarm2OAM(ALARM_ID_PONIF_US_ERRORS, portId, ALARM_CLR, 0);
				else
					sendAlarm2OAM(ALARM_ID_PORT_US_ERRORS, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_upstreamErrors;
			}
	}


	if((PMControl->AlarmEnableMask & MASK_statusChangeTimes) )
	{//statusChangeTimes
			if ((PMControl->AlarmSendMask & MASK_statusChangeTimes) )
			{	
				sendAlarm2OAM(ALARM_ID_PORT_STATUSCHANGETIMES, portId, ALARM_CLR, 0);
				PMControl->AlarmSendMask |= MASK_statusChangeTimes;
			}
	}
		 
	PMControl->AlarmSendMask= 0;

	return 0;
}

#ifdef TCSUPPORT_CT_LOOPDETECT
static int getLoopStatus(int port)
{
    if( port < 0 || port > 3 ){
        return -1;
    }
    
    FILE * fp = fopen("/proc/tc3162/loop_detect_status2", "r");
    if (NULL == fp){
        return -1;
    }
    
    char buf[16];
    fscanf(fp, "%s", buf);
    fclose(fp);
    
    if( '0' != buf[port] && '1' != buf[port] && '2' != buf[port]){
        return -1;
    }

    return buf[port] - '0';
}
#endif

int pmEthPortAlarmHandle(int port, PMEponControl_ptr pmControl){
	int status = 0;

#ifdef TCSUPPORT_MT7530_SWITCH_API
	if (pmControl->AlarmEnableMask & MASK_ETH_LOS){
	status = macMT7530GetEthPortLos(port);
	if (status>=0){
			if ((1 == status) && !(pmControl->AlarmSendMask & MASK_ETH_LOS)){
				sendAlarm2OAM(ALARM_ID_PORT_LOS, port, ALARM_SET, 0);
				pmControl->AlarmSendMask |= MASK_ETH_LOS;
			}else if ((0 == status) && (pmControl->AlarmSendMask & MASK_ETH_LOS)){
				sendAlarm2OAM(ALARM_ID_PORT_LOS, port, ALARM_CLR, 0);
				pmControl->AlarmSendMask &= ~MASK_ETH_LOS;
			}
		}
	}

	if (pmControl->AlarmEnableMask & MASK_ETH_AutoNegFailure){
	status = macMT7530GetEthPortAutoNegFailure(port); 	
	if (status>=0){
			if ((1 == status) && !(pmControl->AlarmSendMask & MASK_ETH_AutoNegFailure)){
				sendAlarm2OAM(ALARM_ID_PORT_AUTONEGFAILURE, port, ALARM_SET, 0);
				pmControl->AlarmSendMask |= MASK_ETH_AutoNegFailure;
			}else if ((0 == status) && (pmControl->AlarmSendMask & MASK_ETH_AutoNegFailure)){
				sendAlarm2OAM(ALARM_ID_PORT_AUTONEGFAILURE, port, ALARM_CLR, 0);
				pmControl->AlarmSendMask &= ~MASK_ETH_AutoNegFailure;
			}
		}
	}

	if (pmControl->AlarmEnableMask & MASK_ETH_Failure){
	status = macMT7530GetEthPortFailure(port); 	
	if (status>=0){
			if ((1 == status) && !(pmControl->AlarmSendMask & MASK_ETH_Failure)){
				sendAlarm2OAM(ALARM_ID_PORT_FAILURE, port, ALARM_SET, 0);
				pmControl->AlarmSendMask |= MASK_ETH_Failure;
			}else if ((0 == status) && (pmControl->AlarmSendMask & MASK_ETH_Failure)){
				sendAlarm2OAM(ALARM_ID_PORT_FAILURE, port, ALARM_CLR, 0);
				pmControl->AlarmSendMask &= ~MASK_ETH_Failure;
			}
		}
	}

	if (pmControl->AlarmEnableMask & MASK_ETH_Congestion){
	status = macMT7530GetEthPortCongestion(port); 	
	if (status>=0){
			if ((1 == status) && !(pmControl->AlarmSendMask & MASK_ETH_Congestion)){
				pmmgrPrintf(PMMGR_DEBUG_ERROR, "Port%d: Congestion=%d\n", port, status);
				sendAlarm2OAM(ALARM_ID_PORT_CONGESTION, port, ALARM_SET, 0);
				pmControl->AlarmSendMask |= MASK_ETH_Congestion;
			}else if ((0 == status) && (pmControl->AlarmSendMask & MASK_ETH_Congestion)){
				sendAlarm2OAM(ALARM_ID_PORT_CONGESTION, port, ALARM_CLR, 0);
				pmControl->AlarmSendMask &= ~MASK_ETH_Congestion;
			}
		}
	}
#endif // #ifdef TCSUPPORT_MT7530_SWITCH_API

#ifdef TCSUPPORT_CT_LOOPDETECT
	if (pmControl->AlarmEnableMask & MASK_ETH_Loopback){
        status = getLoopStatus(port);
        if(status>=0){
            if((status > 0) && !(pmControl->AlarmSendMask & MASK_ETH_Loopback)){
    			pmmgrPrintf(PMMGR_DEBUG_ERROR, "Port%d: loopBackStatus=%d\n", port, status);
    			sendAlarm2OAM(ALARM_ID_PORT_LOOPBACK, port, ALARM_SET, status);
    			pmControl->AlarmSendMask |= MASK_ETH_Loopback;
            }else if((0 == status) && (pmControl->AlarmSendMask & MASK_ETH_Loopback)){
    			pmmgrPrintf(PMMGR_DEBUG_ERROR, "Port%d: loopBackStatus=%d\n", port, status);
    			sendAlarm2OAM(ALARM_ID_PORT_LOOPBACK, port, ALARM_CLR, 0);
    			pmControl->AlarmSendMask &= ~MASK_ETH_Loopback;
            }
        }
	}
#endif //#ifdef TCSUPPORT_CT_LOOPDETECT

	return 0;	
}
int eponPMCurrentTmout(uint8 *param)
{
	uint8 portId = 0;
	PMEponCounters_t PMTmpData_t;
	
	PMEponControl_ptr PMControl = NULL;
	PMEponCounters_ptr PMTmpData = &PMTmpData_t;
	PMEponCounters_ptr PMStartData = NULL;
	PMEponCounters_ptr PMCurrentData = NULL;
	PMEponCounters_ptr PMHistoryData = NULL;
	PMEponCounters_ptr PMAlarmThreshold = NULL;
	PMEponCounters_ptr PMWarningThreshold = NULL;
	PMEponCounters_ptr PMAlarmClearThreshold = NULL;
	PMEponCounters_ptr PMWarningClearThreshold = NULL;

	portId = *param;
	pmmgrPrintf(PMMGR_DEBUG_TRACE,"eponPMCurrentTmout: portId = %x\n", portId);
	if (portId > MAX_PORT_NUM -1)
	{
		pmmgrPrintf(PMMGR_DEBUG_ERROR,"eponPMCurrentTmout:unkown port\n");
		return -1;
	}

	if (gpPmmgrEponDbAddr == NULL)
		return -1;
	
	PMControl = &gpPmmgrEponDbAddr->PMEponControl[portId];
	PMStartData = &gpPmmgrEponDbAddr->PMStartData[portId];
	PMCurrentData =&gpPmmgrEponDbAddr->PMCurrentData[portId];
	PMHistoryData = &gpPmmgrEponDbAddr->PMHistoryData[portId];
	PMAlarmThreshold = &gpPmmgrEponDbAddr->PMAlarmThreshold[portId];
	PMWarningThreshold = &gpPmmgrEponDbAddr->PMWarningThreshold[portId];
	PMAlarmClearThreshold = &gpPmmgrEponDbAddr->PMAlarmClearThreshold[portId];
	PMWarningClearThreshold = &gpPmmgrEponDbAddr->PMWarningClearThreshold[portId];

	if (PMControl->enable == ENABLE && portId != PON_IF_PORT){
		pmEthPortAlarmHandle(portId, PMControl);
	}

	if (PMControl->times == 0)
	{//cycle time start 
		 if (PMControl->enable  == ENABLE)
		 {
		 	getPmDataFromApi(PMStartData, portId);
		 }
	}
	else
	{
		 if  (PMControl->enable == ENABLE)
		 {
		 	getPmDataFromApi(PMTmpData, portId);
			/*
			get Current data
			*/
			getPmCurrentData(PMCurrentData, PMTmpData, PMStartData);
			pmWarningHandle(PMCurrentData, PMWarningThreshold, PMControl, portId);
			pmAlarmHandle(PMCurrentData, PMAlarmThreshold, PMControl, portId);	
			pmClearWarningHandle(PMCurrentData, PMWarningClearThreshold, PMControl, portId);
			pmClearAlarmHandle(PMCurrentData, PMAlarmClearThreshold, PMControl, portId);
		 }
	}


	PMControl->times ++;
	if (PMControl->times == PMControl->cycleTime)
	{//cycle time end
		 if (PMControl->enable == ENABLE)
		 {
			memcpy(PMHistoryData, PMCurrentData, sizeof(PMEponCounters_t));
			memset(PMCurrentData, 0, sizeof(PMEponCounters_t));
		 }
		pmWarningClearHandle(PMControl, portId);
		pmAlarmClearHandle(PMControl, portId);
		 
		PMControl->times  = 0;
	}
	return 0;
}


alarmIdIndex_t alarmIdTable[] = {
{0x0, ALARM_ID_PONIF_DS_DROPEVENTS},//ALARM_ID_PONIF, index = ((0x00 << 8) | MASK_XXX)
{0x1, ALARM_ID_PONIF_US_DROPEVENTS},
{0x02, ALARM_ID_PONIF_DS_CRCERRORFRAMES},
{0x3, ALARM_ID_PONIF_US_CRCERRORFRAMES},
{0x4, ALARM_ID_PONIF_DS_UNDERSIZEFRAMES},
{0x5, ALARM_ID_PONIF_US_UNDERSIZEFRAMES },
{0x6, ALARM_ID_PONIF_DS_OVERSIZEFRAMES },
{0x7, ALARM_ID_PONIF_US_OVERSIZEFRAMES },
{0x8, ALARM_ID_PONIF_DS_FRAGMENTS },
{0x9, ALARM_ID_PONIF_US_FRAGMENTS},
{0xa, ALARM_ID_PONIF_DS_JABBERS },
{0xb,  ALARM_ID_PONIF_US_JABBERS},
{0xc, ALARM_ID_PONIF_DS_DISCARDS  },
{0xd, ALARM_ID_PONIF_US_DISCARDS  },
{0xe, ALARM_ID_PONIF_DS_ERRORS},
{0xf, ALARM_ID_PONIF_US_ERRORS },
{0x11, ALARM_ID_PONIF_HIGH_RXPOWER},
{0x12, ALARM_ID_PONIF_LOW_RXPOWER},
{0x13, ALARM_ID_PONIF_HIGH_TXPOWER},
{0x14, ALARM_ID_PONIF_LOW_TXPOWER},
{0x15, ALARM_ID_PONIF_HIGH_TXCURRENT},
{0x16, ALARM_ID_PONIF_LOW_TXCURRENT},
{0x17, ALARM_ID_PONIF_HIGH_SUPPLYVOLTAGE},
{0x18, ALARM_ID_PONIF_LOW_SUPPLYVOLTAGE},
{0x19, ALARM_ID_PONIF_HIGH_TEMPRATURE},
{0x1a, ALARM_ID_PONIF_LOW_TEMPRATURE},
{0x100, WARNING_ID_PONIF_DS_DROPEVENTS },//WARNING_ID_PONIF, index = ((0x01 << 8) | MASK_XXX)
{0x101, WARNING_ID_PONIF_US_DROPEVENTS },
{0x102, WARNING_ID_PONIF_DS_CRCERRORFRAMES },
{0x103, WARNING_ID_PONIF_US_CRCERRORFRAMES  },
{0x104, WARNING_ID_PONIF_DS_UNDERSIZEFRAMES   },
{0x105, WARNING_ID_PONIF_US_UNDERSIZEFRAMES    },
{0x106, WARNING_ID_PONIF_DS_OVERSIZEFRAMES },
{0x107, WARNING_ID_PONIF_US_OVERSIZEFRAMES },
{0x108, WARNING_ID_PONIF_DS_FRAGMENTS   },
{0x109, WARNING_ID_PONIF_US_FRAGMENTS  },
{0x10a, WARNING_ID_PONIF_DS_JABBERS  },
{0x10b, WARNING_ID_PONIF_US_JABBERS  },
{0x10c, WARNING_ID_PONIF_DS_DISCARDS  },
{0x10d, WARNING_ID_PONIF_US_DISCARDS },
{0x10e, WARNING_ID_PONIF_DS_ERRORS   },
{0x10f, WARNING_ID_PONIF_US_ERRORS   },
{0x111, WARNING_ID_PONIF_HIGH_RXPOWER},
{0x112, WARNING_ID_PONIF_LOW_RXPOWER},
{0x113, WARNING_ID_PONIF_HIGH_TXPOWER},
{0x114, WARNING_ID_PONIF_LOW_TXPOWER},
{0x115, WARNING_ID_PONIF_HIGH_TXCURRENT},
{0x116, WARNING_ID_PONIF_LOW_TXCURRENT},
{0x117, WARNING_ID_PONIF_HIGH_SUPPLYVOLTAGE},
{0x118, WARNING_ID_PONIF_LOW_SUPPLYVOLTAGE},
{0x119, WARNING_ID_PONIF_HIGH_TEMPRATURE},
{0x11a, WARNING_ID_PONIF_LOW_TEMPRATURE},
{0x200, ALARM_ID_PORT_DS_DROPEVENTS  },//ALARM_ID_PORT, index = ((0x02 << 8) | MASK_XXX)
{0x201, ALARM_ID_PORT_US_DROPEVENTS },
{0x202, ALARM_ID_PORT_DS_CRCERRORFRAMES},
{0x203, ALARM_ID_PORT_US_CRCERRORFRAMES},
{0x204, ALARM_ID_PORT_DS_UNDERSIZEFRAMES  },
{0x205, ALARM_ID_PORT_US_UNDERSIZEFRAMES},
{0x206, ALARM_ID_PORT_DS_OVERSIZEFRAMES },
{0x207, ALARM_ID_PORT_US_OVERSIZEFRAMES },
{0x208, ALARM_ID_PORT_DS_FRAGMENTS    },
{0x209, ALARM_ID_PORT_US_FRAGMENTS },
{0x20a, ALARM_ID_PORT_DS_JABBERS   },
{0x20b, ALARM_ID_PORT_US_JABBERS   },
{0x20c, ALARM_ID_PORT_DS_DISCARDS   },
{0x20d, ALARM_ID_PORT_US_DISCARDS  },
{0x20e, ALARM_ID_PORT_DS_ERRORS    },
{0x20f, ALARM_ID_PORT_US_ERRORS },
{0x210, ALARM_ID_PORT_STATUSCHANGETIMES},
{0x21b, ALARM_ID_PORT_AUTONEGFAILURE},
{0x21c, ALARM_ID_PORT_LOS},
{0x21d, ALARM_ID_PORT_FAILURE},
{0x21e, ALARM_ID_PORT_LOOPBACK},
{0x21f, ALARM_ID_PORT_CONGESTION},
{0x300, WARNING_ID_PORT_DS_DROPEVENTS  }, //WARNING_ID_PORT, index = ((0x03 << 8) | MASK_XXX)
{0x301, WARNING_ID_PORT_US_DROPEVENTS  },
{0x302, WARNING_ID_PORT_DS_CRCERRORFRAMES },
{0x303, WARNING_ID_PORT_US_CRCERRORFRAMES },
{0x304, WARNING_ID_PORT_DS_UNDERSIZEFRAMES },
{0x305, WARNING_ID_PORT_US_UNDERSIZEFRAMES},
{0x306, WARNING_ID_PORT_DS_OVERSIZEFRAMES},
{0x307, WARNING_ID_PORT_US_OVERSIZEFRAMES },
{0x308, WARNING_ID_PORT_DS_FRAGMENTS},
{0x309, WARNING_ID_PORT_US_FRAGMENTS},
{0x30a, WARNING_ID_PORT_DS_JABBERS  },
{0x30b, WARNING_ID_PORT_US_JABBERS  },
{0x30c, WARNING_ID_PORT_DS_DISCARDS    },
{0x30d, WARNING_ID_PORT_US_DISCARDS },
{0x30e, WARNING_ID_PORT_DS_ERRORS  },
{0x30f,  WARNING_ID_PORT_US_ERRORS},
{0x310, WARNING_ID_PORT_STATUSCHANGETIMES}
};

uint16 pmmgrGetPmAlarmIndexByAlarmId(uint16 alarmId)
{
	int i = 0;
	alarmIdIndex_ptr alarmId_p = NULL;
	int alarmIdTableSize = sizeof(alarmIdTable)/sizeof(alarmIdIndex_t);
	alarmId_p = alarmIdTable;
	for (i = 0; i < alarmIdTableSize; i++, alarmId_p++)
		if (alarmId_p->alarmId == alarmId)
			return alarmId_p->index;

	return 0xFFFF;
}

/**********************************************************
	EPON MSG API
***********************************************************
*/
void *pmmgrMqTask(void);

int pmmgrMq = -1;
pthread_t pmmgrThread = 0;

int pmmgrCmdInit(void){
	int ret = 0;
	int oflag = 0;
	key_t mqkey;

	oflag = 0666|IPC_CREAT;

	ret = open(PMMGR_MQ_PATH, O_CREAT|O_WRONLY);
	if (ret < 0){
		return -1;
	}
	close(ret);

	mqkey = ftok(PMMGR_MQ_PATH, PMMGR_PROJID);
	
	pmmgrMq = msgget(mqkey, oflag);

	if (pmmgrMq < 0)
		return -1;

	/* create a thread to process API */
	ret = pthread_create(&pmmgrThread, NULL, (void*)pmmgrMqTask, NULL);
	if (ret != 0)
		return -1;

	return 0;
}


void *pmmgrMqTask(void){
	int msg_len = 0;
	int need_send = 0;
	Pmmgr_msg_t msg, snd_msg;
	
	while (1){
		need_send = 0;
		memset((void*)&msg, 0, sizeof(Pmmgr_msg_t));
		memset((void*)&snd_msg, 0, sizeof(Pmmgr_msg_t));

		msg_len = msgrcv(pmmgrMq, &msg, sizeof(Pmmgr_msg_t)-sizeof(long), PMMGR_MSG_SND, 0);
		if (msg_len < (sizeof(Pmmgr_msg_t)-sizeof(long))){
			continue;
		}

		snd_msg.msg_type = msg.cmd_type;
		snd_msg.port_id = msg.port_id;
		snd_msg.alarm_id = msg.alarm_id;
		need_send = 1;
		switch (msg.cmd_type){
			case PMMGR_CT_GET_ENABLE:
				snd_msg.port_state = getEponPMEnable(msg.port_id);
				snd_msg.cmd_ret = 0;
			break;
			case PMMGR_CT_SET_ENABLE:
				snd_msg.cmd_ret = setEponPMEnable(msg.port_id, msg.port_state);
			break;
			case PMMGR_CT_GET_STATE:
				snd_msg.alarm_state = getEponPortAlarmStatus(msg.port_id, msg.alarm_id);
				snd_msg.cmd_ret = 0;
			break;
			case PMMGR_CT_SET_STATE:
				snd_msg.cmd_ret = setEponPortAlarmStatus(msg.port_id, msg.alarm_id, msg.alarm_state);
			break;
			case PMMGR_CT_GET_THRSHLD:
				snd_msg.cmd_ret = getEponPortAlarmThreshold(msg.port_id, msg.alarm_id, &snd_msg.alarm_set_threshold, &snd_msg.alarm_clear_threshold);
			break;
			case PMMGR_CT_SET_THRSHLD:
				snd_msg.cmd_ret = setEponPortAlarmThreshold(msg.port_id, msg.alarm_id, msg.alarm_set_threshold, msg.alarm_clear_threshold);
			break;
			case PMMGR_CT_GET_CYCLE_TIME:
				snd_msg.cycle_time = getEponPMCycleTime(msg.port_id);
				snd_msg.cmd_ret = 0;
			break;
			case PMMGR_CT_SET_CYCLE_TIME:
				snd_msg.cmd_ret = setEponPMCycleTime(msg.port_id, msg.cycle_time);
			break;
			case PMMGR_CT_GET_PMDATA:
				snd_msg.cmd_ret = getEponPMCurrentData(msg.port_id, &snd_msg.pm);
			break;
			case PMMGR_CT_SET_PMDATA:
				snd_msg.cmd_ret = setEponPMCurrentData(msg.port_id, &msg.pm);
			break;
			case PMMGR_CT_GET_PMHSTYDATA:
				snd_msg.cmd_ret = getEponPMHistoryData(msg.port_id, &snd_msg.pm);
			break;
			case PMMGR_CT_INIT_EPON:
				pmmgrPrintf(PMMGR_DEBUG_WARN, "EPM: init EPON pmmgr!\n");
				//databaseDestroy();
				pmmgrDestroy();
				pmmgrInit(EPON_MODE);
				need_send = 0;
			break;
			default:
				/* do nothing */
				need_send = 0;
		}

		if (need_send){
			snd_msg.msg_type = PMMGR_MSG_RCV;
			msgsnd(pmmgrMq, &snd_msg, sizeof(Pmmgr_msg_t)-sizeof(long), 0);			
		}
	}

}

