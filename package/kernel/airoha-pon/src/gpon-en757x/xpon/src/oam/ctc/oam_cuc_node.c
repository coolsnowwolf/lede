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
	oam_cuc_node.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2013/3/18	Create
*/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/if_ether.h>

#include "libepon.h"
#include "epon_oam.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ext_mgr.h"
#include "ctc/oam_sys_api.h"
#include "ctc/oam_ctc_dspch.h"

#ifdef TCSUPPORT_EPON_MAPPING
#include "libeponmap.h"
#endif

#ifdef TCSUPPORT_MT7530_SWITCH_API
#define u8  unsigned char 
#define u16 unsigned short
#define u32 unsigned int
#include "mtkswitch_api.h"
#endif

#include "../../../../tcapi_lib/tcapi.h"

extern OamCtcDB_t gCtcDB;

/*
 * CUC Attributes and actions table.
 */
OamParamNode_t cucOamNodeTable[]=
{
	//	{"Name", objectType, branch, leaf, GET, SET}
	{"aPhyAdminState",              	OBJ_BIT_PORT, 	OAM_STD_ATTR,	OAM_LF_PHYADMINSTATE,				0x04,	oamGetPhyAdminState,			NULL},
	{"aAutoNegAdminState",            OBJ_BIT_PORT, 	OAM_STD_ATTR,	OAM_LF_AUTONEGADMINSTATE,			0x04,	oamGetAutoNegAdminState,		NULL},
	{"aAutoNegLocalTechAbility",     	OBJ_BIT_PORT, 	OAM_STD_ATTR,	OAM_LF_AUTONEGLOCALTECHABILITY,	0xFF,	oamGetAutoNegLocalTechAbility,	NULL},
	{"aAutoNegAdvTechAbility",		OBJ_BIT_PORT, 	OAM_STD_ATTR,	OAM_LF_AUTONEGADVTECHABILITY,		0xFF,	oamGetAutoNegAdvTechAbility,	NULL},
	{"aFECAbility",                        	OBJ_BIT_ONU,  	OAM_STD_ATTR,	OAM_LF_FECABILITY,					0x04,	oamGetFecAbility,				NULL},
	{"aFECmode",               		OBJ_BIT_ONU,  	OAM_STD_ATTR,	OAM_LF_FECMODE,					0x04,	oamGetFecMode,					oamSetFecMode},
	
	{"ONUSN",                       		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_ONU_SN,						0x26,	oamGetOnuSN,						NULL},
	{"FirmwareVer",                 		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_FIRMWAREVER,				0xFF,	oamGetFirmwareVer,					NULL},
	{"ChipsetID",                   		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_CHIPSET_ID,					0x08,	oamGetChipsetID,					NULL},
	{"ONUCapabilities1",            	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_ONU_CAPABILITIES_1,			0x1A,	oamGetOnuCapabilities1,			NULL},
	{"ONUCapabilities2",            	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_ONU_CAPABILITIES_2,			0xFF,	oamGetOnuCapabilities2,			NULL},
	{"ONUCapabilities3",            	OBJ_BIT_ONU,  	OAM_EXT_ATTR, 	OAM_LF_ONU_CAPABILITIES_3,			0x02,	oamGetOnuCapabilities3,			NULL},
	{"PowerSavingCapabilities",     	OBJ_BIT_ONU,  	OAM_EXT_ATTR, 	OAM_LF_POWERSAVINGCAPABILITIES,		0x02,	oamGetPowerSavingCapabilities,		NULL},
	{"PowerSavingConfig", 			OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_CU_POWERSAVINGCONFIG,		0x07,	oamGetPowerSavingConfig, 			oamSetPowerSavingConfig},
	{"ProtectionParameters",		OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_CU_PROTECTIONPARAMETERS,	0x04,	oamGetProtectionParameters,		oamSetProtectionParameters},
	{"OpticalTransceiverDiagnosis", 	OBJ_BIT_PONIF,  	OAM_EXT_ATTR,	OAM_LF_OPTICALTRANSCEIVERDIAGNOSIS,0x0A,	oamGetOpticalTransceiverDiagnosis,	NULL},
	{"ServiceSLA",                  		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_SERVICE_SLA,					0xFF,	oamGetServiceSLA,					oamSetServiceSLA},
	{"HoldoverConfig",              		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_HOLDOVERCONFIG,				0x08,	oamGetHoldoverConfig,				oamSetHoldoverConfig},
	{"ActivePON_IFAdminstate",      	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_ACTIVE_PON_IFADMINSTATE,	0x01,	oamGetActivePONIFAdminstate,		oamSetActivePONIFAdminstate},
	{"EthLinkState",                		OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_ETHLINKSTATE,				0x01,	oamGetEthLinkState,				NULL},
	{"EthPortPause",                		OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_ETHPORT_PAUSE,				0x01,	oamGetEthPortPause,				oamSetEthPortPause},
	{"EthPortUSPolicing",           	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_ETHPORTUS_POLICING,			0xFF,	oamGetEthPortUSPolicing,			oamSetEthPortUSPolicing},
	{"E1Port",                      		OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_E1_PORT,						0x01,	oamGetE1Port,						oamSetE1Port},
	{"EthPortDSRateLimiting",       	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_ETHPORTDS_RATELIMITING,		0xFF,	oamGetEthPortDSRateLimiting,		oamSetEthPortDSRateLimiting},
	{"PortLoopDetect",              	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_PORTLOOPDETECT,				0x04,	oamGetCucPortLoopDetect,            oamSetCucPortLoopDetect},
	{"PortDisableLooped",           	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_PORTDISABLELOOPED,			0x04,	NULL,								oamCucSetPortDisableLooped},
	{"PortMACAgingTime",            	OBJ_BIT_ONU, 	OAM_EXT_ATTR,	OAM_LF_CU_ONUMACAGINGTIME,			0x04,	oamGetPortMacAgingTime,			oamSetPortMacAgingTime},
	{"OnuMacLimit", 				OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_CU_ONUMACLIMIT, 				0x02,	oamGetPortMacLimit, 					oamSetPortMacLimit},
	{"OnuPortModeCfg",			OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_CU_ONUPORTMODECONFIG,		0x01,	NULL, 								oamSetPortModeCfg},
	{"OnuPortModeStatus", 		OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_CU_ONUPORTMODESTATUS,		0x01,	oamGetPortModeStatus, 				NULL},
	{"ConfigCount",				OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_CU_CFGCNT,					0x04,	oamGetCfgCnt, 						oamSetCfgCnt},

	{"VLAN",                        		OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_VLAN,						0xFF,	oamGetVlan,						oamSetVlan},
	{"ClassificationMarking",       	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_CLASSIFICATION_MARKING,		0xFF,	oamGetClassificationMarking,		oamSetClassificationMarking},
	{"AddDelMulticastVLAN",         	OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_ADD_DEL_MULTICAST_VLAN,		0xFF,	oamGetAddDelMulticastVlan,			oamSetAddDelMulticastVlan},
	{"MulticastTagOper",            	OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_MULTICASTTAGOPER,			0xFF,	oamGetMulticastTagOper,			oamSetMulticastTagOper},
	{"MulticastSwitch",             	OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_MULTICASTSWITCH,			0x01,	oamGetMulticastSwitch,				oamSetMulticastSwitch},
	{"MulticastControl",            		OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_MULTICASTCONTROL,			0xFF,	oamGetMulticastControl,			oamSetMulticastControl},
	{"GroupNumMax",                 	OBJ_BIT_PORT,	OAM_EXT_ATTR,	OAM_LF_GROUP_NUM_MAX,				0x01,	oamGetGroupNumMax,					oamSetGroupNumMax},
	{"aFastLeaveAbility",           	OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_FASTLEAVEABILITY,			0xFF,	oamGetFastLeaveAbility,			NULL},
	{"aFastLeaveAdminState",        	OBJ_BIT_ONU,		OAM_EXT_ATTR,	OAM_LF_FASTLEAVEADMINSTATE,		0x04,	oamGetFastLeaveAdminState,			NULL},

	{"LLIDQueueConfig",             	OBJ_BIT_LLID,		OAM_EXT_ATTR,	OAM_LF_LLIDQUEUECONFIG_V3,			0xFF,	oamGetLlidQueueConfig,				oamSetLlidQueueConfig},
	{"MxUMngGlobalParameter", 		OBJ_BIT_ONU, 	OAM_EXT_ATTR, 	OAM_LF_MXUMNGGLOBALPARAMETER, 	0xFF,	oamGetMxUMngGlobalParameter, 		oamSetMxUMngGlobalParameter},
	{"MxUMngSNMPParameter", 		OBJ_BIT_ONU, 	OAM_EXT_ATTR, 	OAM_LF_MXUMNGSNMPPARAMETER, 		0xFF,	oamGetMxUMngSNMPParameter, 		oamSetMxUMngSNMPParameter},
	{"ONUTxPowerSupplyControl",     OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_ONUTXPOWERSUPPLYCONTROL,	0xFF,	NULL,								oamSetONUTxPowerSupplyControl},
#ifdef TCSUPPORT_VOIP
	{"IADInfo",                     		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_IADINFO,						0x48,	oamGetIADInfo,						NULL},
	{"VoIPPort",                             OBJ_BIT_PORT,     OAM_EXT_ATTR,   OAM_LF_VOIP_PORT,                   0x01,   oamGetVoIPPort,                 oamSetVoIPPort},
	{"GlobalParameterConfig",       	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_GLOBALPARAMETERCONFIG,		0x54,	oamGetGlobalParameterConfig,		oamSetGlobalParameterConfig},
	{"SIPParameterConfig",          	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_SIPPARAMETERCONFIG,			0x2D,	oamGetSIPParameterConfig,			oamSetSIPParameterConfig},
	{"SIPUserParameterConfig",     	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_SIPUSERPARAMETERCONFIG,		0x40,	oamGetSIPUserParameterConfig,		oamSetSIPUserParameterConfig},
	{"FaxModemConfig",              	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_FAX_MODEM_CONFIG,			0x02,	oamGetFaxModemConfig,				oamSetFaxModemConfig},
	{"H.248ParameterConfig",		OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_H248_PARAMETERCONFIG,		0x54,	oamGetH248ParameterConfig,			oamSetH248ParameterConfig},
	{"H.248UserTIDInfo",			OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_H248_USERTIDINFO,			0x20,	oamGetH248UserTIDInfo,				oamSetH248UserTIDInfo},
	{"H.248RTPTIDConfig",               OBJ_BIT_ONU,      OAM_EXT_ATTR,   OAM_LF_H248_RTP_TIDCONFIG,          0x1B,   NULL,                               oamSetH248RTPTIDConfig},
	{"POTSStatus",                  	OBJ_BIT_PORT, 	OAM_EXT_ATTR,	OAM_LF_POTSSTATUS,					0x0C,	oamGetPOTSStatus,					NULL},
#endif
	
	{"AlarmAdminState",             	OBJ_BIT_ALL, 		OAM_EXT_ATTR,	OAM_LF_ALARMADMINSTATE,			0x06,	oamGetAlarmAdminState,				oamSetAlarmAdminState},
	{"AlarmThreshold",               	OBJ_BIT_ALL, 		OAM_EXT_ATTR,	OAM_LF_ALARMTHRESHOLD,				0x0A,	oamGetAlarmThresholds,				oamSetAlarmThresholds},
#ifdef TCSUPPORT_VOIP
	{"PingTest",               	OBJ_BIT_ONU, 		OAM_EXT_ATTR,	OAM_LF_CU_PINGTEST,				0x0B,	NULL,				oamSetPingTest},
	{"PingTestResult",               	OBJ_BIT_ONU, 		OAM_EXT_ATTR,	OAM_LF_CU_PINGTEST_RESULT,				0x0E,	oamGetPingTestResult,				NULL},
#endif
#ifdef TCSUPPORT_PMMGR
	{"PerformanceMonitorStatus",     		OBJ_BIT_PORT|OBJ_BIT_PONIF, 	OAM_EXT_ATTR,	OAM_LF_PFMCSTATUS,		0x06,	oamGetPerformanceStatus,			oamSetPerformanceStatus},
	{"PerformanceMonitorCurrentData",	OBJ_BIT_PORT|OBJ_BIT_PONIF, 	OAM_EXT_ATTR,	OAM_LF_PFMCDATACURR,	0x80,	oamGetPerformanceDataCurrent,		oamSetPerformanceDataCurrent},
	{"PerformanceMonitorHistoryData",	OBJ_BIT_PORT|OBJ_BIT_PONIF, 	OAM_EXT_ATTR,	OAM_LF_PFMCDATAHSTRY,	0x80,	oamGetPerformanceDataHistory,		NULL},
#endif
/*
	{"H.248RTPTIDInfo",			OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_H248_RTP_TIDINFO,			0x21,	oamGetH248RTPTIDInfo,				NULL},
	{"H.248IADOperationStatus",	OBJ_BIT_ONU,  	OAM_EXT_ATTR,	OAM_LF_H248IADOPERATIONSTATUS,		0x04,	oamGetH248IADOperationStatus,		NULL},
*/	
	{"acPhyAdminControl",            	OBJ_BIT_PORT,	OAM_STD_ACT,	OAM_LF_PHYADMINCONTROL,			0x04,	NULL,	oamSetPhyAdminControl},
	{"acAutoNegRestartAutoConfig",  OBJ_BIT_PORT,	OAM_STD_ACT,	OAM_LF_AUTONEGRESTARTAUTOCONFIG,	0x00,	NULL,	oamSetAutoNegRestartAutoConfig},
	{"acAutoNegAdminControl",        	OBJ_BIT_PORT,	OAM_STD_ACT,	OAM_LF_AUTONEGADMINCONTROL,		0x04,	NULL,	oamSetAutoNegAdminControl},
	
	{"ResetOnu",                     		OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_RESETONU,				0x00,	NULL,	oamSetResetOnu},
	{"ConfigParamMngmt",       		OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_CU_CFGPRMMNGMT,		0x01,	NULL,	oamSetCfgParaMngmt},
	{"SleepControl",                 		OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_CU_SLEEPCONTROL,		0x0E,	NULL,	oamSetSleepControl},
	{"acMultiLLIDAdminControl",      	OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_MULTILLIDADMINCONTROL,	0x04,	NULL,	oamSetMultiLlidAdminControl},
	{"acFastLeaveAdminControl",      OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_FASTLEAVEADMINCONTROL,	0x04,	NULL,	oamSetFastLeaveAdminControl},
#ifdef TCSUPPORT_VOIP
	{"IADOperation",                 	OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_IADOPERATION,			0x04,	NULL,	oamSetIADOperation},
	{"SIPDigitMap",                  		OBJ_BIT_ONU,  	OAM_EXT_ACT,	OAM_LF_SIPDIGITMAP,				0xFF,	NULL,	oamSetSIPDigitMap},
	{"TimerConfig", 				 OBJ_BIT_ONU,  OAM_EXT_ATTR, OAM_LF_CUC_TIMER_CONFIG,		0x02,	oamGetTimerCfg, oamSetTimerCfg},
#endif
	{"RMSGlobalParameterConfig1",     OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_CU_RMSGLBPRMCFG1,  0x54,   oamGetRMSGlobalParameterConfig1,            oamSetRMSGlobalParameterConfig1},
	{"RMSGlobalParameterConfig2",     OBJ_BIT_ONU,    OAM_EXT_ATTR,   OAM_LF_CU_RMSGLBPRMCFG2,  0xFF,   oamGetRMSGlobalParameterConfig2,            oamSetRMSGlobalParameterConfig2},
	{"", 0, 0, 0, 0, NULL, NULL}
};


/*----------------------------------------------------------------------------*/

OamParamNode_Ptr findCucOamNodeByBranchLeaf(u_char branch, u_short leaf){
	int i;
	OamParamNode_Ptr opnp = cucOamNodeTable;
	int num = sizeof(cucOamNodeTable) / sizeof(cucOamNodeTable[0]);

	for (i = 0; i < num; ++i, ++opnp){
		if ( (leaf == opnp->leaf) &&(branch == opnp->branch))
			return opnp;
	}
	return NULL;
}

/*----------------------------------------------------------------------------*/
/*	GET/SET function of every node	*/

u_char oamGetPortMacLimit(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	int   limitCnt;

	if (portId == PORT_ERROR)
		return VAR_BAD_PARAMETERS;

	limitCnt = macMT7530GetPortMacLimit(portId);
	if (limitCnt < 0)
		return VAR_NO_RESOURCE;

	setU16(pValue, limitCnt);
	*pLength = 2;
	return SUCCESS;
}
u_char oamSetPortMacLimit(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	u_short limitCnt = GetU16(pValue);
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	if (portId == PORT_ERROR)
		return VAR_BAD_PARAMETERS;

	if (SUCCESS != macMT7530SetPortMacLimit(portId, (u_int)limitCnt)){
		return VAR_NO_RESOURCE;
	}

	gCtcDB.port[portId].mac_limit_cnt = limitCnt;
	return SET_OK;
}


typedef struct{
	char * speed;
	char * duplex;
}eth_mode_t;

eth_mode_t g_oam_eth_modes[] = {
		{"10", "Half"} ,
		{"10", "Full"} ,
		{"100", "Half"},
		{"100", "Full"},
};

u_char oamGetPortModeStatus(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	u_char state = 1,mode = 1;
	u_char speed[5]={0},duplex[5]={0};
	eth_mode_t * modePtr = NULL;
	
	if (portId == PORT_ERROR)
		return VAR_BAD_PARAMETERS;
	if (SUCCESS != macMT7530GetMaxBitRate(portId, speed))
		return VAR_NO_RESOURCE;
	if (SUCCESS != macMT7530GetDuplexMode(portId, duplex))
		return VAR_NO_RESOURCE;
	
	for(mode = 1;mode <= ARRAY_SIZE(g_oam_eth_modes); mode++){
		modePtr = &g_oam_eth_modes[mode-1];
		if (!strcmp(speed, modePtr->speed) && !strcmp(duplex, modePtr->duplex)){
			state = mode;
			break;
		}
	}
	setU8(pValue, state);
	*pLength = 1;
	return SUCCESS;
}

u_char oamSetPortModeCfg(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	u_char mode = GetU8(pValue);
	eth_mode_t * modePtr = NULL;
	
	if (portId == PORT_ERROR)
		return VAR_BAD_PARAMETERS;

	if(ARRAY_SIZE(g_oam_eth_modes) < mode || mode <= 0 )
		return VAR_BAD_PARAMETERS;

	modePtr = &g_oam_eth_modes[mode-1];
	if (SUCCESS != macMT7530SetMaxBitRate(portId, modePtr->speed))
		return VAR_NO_RESOURCE;
	if (SUCCESS != macMT7530SetDuplexMode(portId, modePtr->duplex))
		return VAR_NO_RESOURCE;

	gCtcDB.port[portId].port_mode = mode;
	return SET_OK;
}

u_char oamGetCfgCnt(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	u_int cfgCnt = 0;

	cfgCnt = gCtcDB.config_counter;
	
	setU32(pValue, cfgCnt);
	*pLength = 4;
	
	return SUCCESS;	
}
u_char oamSetCfgCnt(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	u_int cfgCnt = ntohl(*(u_int*)pValue);
	char buf[20];

	gCtcDB.config_counter = cfgCnt;
	gCtcDB.db_sync_sflag |= DB_SYNC_SAVE_CONFIG;

	return SET_OK;
}


u_char oamSetCfgParaMngmt(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	u_char action = *pValue;

	switch(action){
	case 01: // save oam cfg
		gCtcDB.db_sync_sflag |= DB_SYNC_SAVE_CONFIG;
	break;
	case 02: // clear oam cfg
		oamClearSelfConfig();
		oamResetSystemParam(llidIdx);
	break;
	case 03: // reset to factory
		system("mtd write -r /userfs/romfile.cfg romfile");
	break;
	default:
		eponOamExtDbg(DBG_OAM_L3, "Recv action: %d, skip!\n", action);
	}
	return SET_OK;
}

#ifdef TCSUPPORT_VOIP
OamPingTest_t gCucPingTest = {0};
int doOamSetPingTest(){
	char buf[32] = {0};
	char *tmp = NULL;
	
	tmp = inet_ntoa(*((struct in_addr *) &(ntohl(gCucPingTest.destIP))));
	if(tmp){
		if(tcapi_set("CucPing_Entry", "DestIP", tmp) != TCAPI_SUCCESS){
			return VAR_NO_RESOURCE;
		}
	}
	else{
		return VAR_BAD_PARAMETERS;
	}
	sprintf(buf, "%d", gCucPingTest.pingAction);
	if(tcapi_set("CucPing_Entry","PingAction",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;
	
	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(gCucPingTest.pingTimes));
	if(tcapi_set("CucPing_Entry","PingTimes",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(gCucPingTest.pingSize));
	if(tcapi_set("CucPing_Entry","PingSize",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(gCucPingTest.timeout));
	if(tcapi_set("CucPing_Entry","PingTimeout",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	if(tcapi_commit("CucPing") != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;
	
	return 0;
}


u_char oamSetPingTest(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	OamPingTest_Ptr pPing = (OamPingTest_Ptr)pValue;
	char buf[32] = {0};
	char *tmp = NULL;
	
#if 1
	gCucPingTest.destIP	= pPing->destIP;
	gCucPingTest.pingAction = pPing->pingAction;	
	gCucPingTest.pingTimes = pPing->pingTimes;
	gCucPingTest.pingSize = pPing->pingSize;
	gCucPingTest.timeout = pPing->timeout;
	
	postDBSetSyncMsg(DB_SYNC_CUC_PINT_TEST_CONFIG);
#else	
	tmp = inet_ntoa(*((struct in_addr *) &(ntohl(pPing->destIP))));
	if(tmp){
		if(tcapi_set("CucPing_Entry", "DestIP", tmp) != TCAPI_SUCCESS){
			return VAR_NO_RESOURCE;
		}
	}
	else{
		return VAR_BAD_PARAMETERS;
	}
	sprintf(buf, "%d", pPing->pingAction);
	if(tcapi_set("CucPing_Entry","PingAction",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;
	
	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(pPing->pingTimes));
	if(tcapi_set("CucPing_Entry","PingTimes",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(pPing->pingSize));
	if(tcapi_set("CucPing_Entry","PingSize",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	memset(buf,0,sizeof(buf));
	sprintf(buf, "%d", ntohs(pPing->timeout));
	if(tcapi_set("CucPing_Entry","PingTimeout",buf) != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;

	if(tcapi_commit("CucPing") != TCAPI_SUCCESS)
		return VAR_NO_RESOURCE;
#endif
	
	return SET_OK;
}

u_char oamGetPingTestResult(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	char buf[18] = {0};
	OamPingTestResult_Ptr pPingResult = (OamPingTestResult_Ptr)pValue;
	*pLength = sizeof(OamPingTestResult_t);

   	 if (TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "Result", buf)){
    		return VAR_NO_RESOURCE;
    	}
	pPingResult->testResult = atoi(buf);
		
	memset(buf, 0, sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "TXPkt", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->txPkt = htons(atoi(buf));
	
	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "RxPkt", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->rxPkt = htons(atoi(buf));

	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "LostPkt", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->lostPkt = htons(atoi(buf));

	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "LostPktRatio", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->lostPktRatio = atoi(buf);

	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "MinDelay", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->minDelay = htons(atoi(buf));

	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "MaxDelay", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->maxDelay = htons(atoi(buf));

	memset(buf,0,sizeof(buf));
	if(TCAPI_SUCCESS != tcapi_get("CucPing_Entry", "AvgDelay", buf)){
		return VAR_NO_RESOURCE;
	}
	pPingResult->avgDelay = htons(atoi(buf));

	return SUCCESS;
}

#endif // TCSUPPORT_VOIP

#define PVC_NUM 	8
#define MAX_SMUX_NUM	8
#define WAN_ENCAP_DYN	0
#define WAN_ENCAP_STATIC	1
#define WAN_ENCAP_PPP	2
#define WAN_ENCAP_BRIDGE	3

u_char oamGetRMSGlobalParameterConfig1(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue){
	char  node[32] = {0};
	int pvc_index = 0;
	int entry_index = 0;
	char buf[64] = {0};
	int find_tr69 = 0;
	oamRMSGlbParaCfg1_Ptr rmsCfg1_ptr = (oamRMSGlbParaCfg1_Ptr)pValue;

	*pLength = sizeof(oamRMSGlbParaCfg1_t);
	for(pvc_index = 0; pvc_index < PVC_NUM; pvc_index++){
		for(entry_index = 0; entry_index < MAX_SMUX_NUM; entry_index++){
			memset(node, 0, sizeof(node));
			memset(buf, 0, sizeof(buf));			
			sprintf(node, "Wan_PVC%d_Entry%d", pvc_index, entry_index);
			if(tcapi_get(node,"ServiceList", buf) == TCAPI_SUCCESS ){
				if(strstr(buf, "TR069") != NULL){
					find_tr69 = 1;
					tcdbg_printf("\noamGetRMSGlobalParameterConfig1, find tr069, pvc = %d, entry = %d", pvc_index, entry_index);
					goto out_of_loop;
				}
			}
		}
	}

out_of_loop:
	if(find_tr69 == 0){
		tcdbg_printf("\noamGetRMSGlobalParameterConfig1, NOT find tr069!!!");
		return VAR_NO_RESOURCE;
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(node, "ISP", buf)){
		if(atoi(buf) == WAN_ENCAP_STATIC)
	    		rmsCfg1_ptr->RMSIpMode= 0x00;
		else if(atoi(buf) == WAN_ENCAP_PPP)
			rmsCfg1_ptr->RMSIpMode= 0x02;
		else
			rmsCfg1_ptr->RMSIpMode= 0x01;
	}else{
	        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
	    	return VAR_NO_RESOURCE;
	}
	if(rmsCfg1_ptr->RMSIpMode != 0x00){
		rmsCfg1_ptr->RMSIpAddr = 0;
		rmsCfg1_ptr->RMSNetMask = 0;
		rmsCfg1_ptr->RMSDefaultGW = 0;
	}
	else{	//Static IP
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "IPADDR", buf)){	//ip addr
			inet_aton(buf, (struct in_addr *)&(rmsCfg1_ptr->RMSIpAddr));
		}
		else{
		        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}

		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "NETMASK", buf)){	//net mask
			inet_aton(buf, (struct in_addr *)&(rmsCfg1_ptr->RMSNetMask));
		}
		else{
		        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}

		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "GATEWAY", buf)){	//default gw
			inet_aton(buf, (struct in_addr *)&(rmsCfg1_ptr->RMSDefaultGW));
		}
		else{
		        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}
	}
	
	if(rmsCfg1_ptr->RMSIpMode == 0x02){	//PPPoE
		rmsCfg1_ptr->pppoeMode = 0x00;
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "USERNAME", buf)){
		    memset(rmsCfg1_ptr->pppoeUsername, 0, sizeof(rmsCfg1_ptr->pppoeUsername));
			strncpy(rmsCfg1_ptr->pppoeUsername, buf, 32);
		}else{
		    tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}
		
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "PASSWORD", buf)){
		    memset(rmsCfg1_ptr->pppoePassword, 0, sizeof(rmsCfg1_ptr->pppoePassword));
			strncpy(rmsCfg1_ptr->pppoePassword, buf, 32);
		}
		else{
		        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}	
	}
	else{
		rmsCfg1_ptr->pppoeMode = 0xFF;
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(node, "VLANMode", buf)){	//tag flag
		if(strcmp(buf, "TRANSPARENT") == 0)
			rmsCfg1_ptr->taggedFlag = 0x00;
		else
			rmsCfg1_ptr->taggedFlag = 0x01;	//tag
	}
	else{
	        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
		return VAR_NO_RESOURCE;
	}
	
	if(rmsCfg1_ptr->taggedFlag == 0x01){	//tag mode
	        memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "VLANID", buf)){	//vlan id
			rmsCfg1_ptr->CVlan= htons(atoi(buf));
		}
		else{
		        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
			return VAR_NO_RESOURCE;
		}
	        memset(buf, 0, sizeof(buf));
        	if (TCAPI_SUCCESS == tcapi_get(node, "dot1pData", buf)){	//priority
        		rmsCfg1_ptr->priority= atoi(buf);
        	}
        	else{
        	        tcdbg_printf("[%s] failed here %d",__FUNCTION__, __LINE__);
        		return VAR_NO_RESOURCE;
        	}
	}
	else{
		rmsCfg1_ptr->CVlan= 0;
		rmsCfg1_ptr->priority = 0;
	}
	rmsCfg1_ptr->SVLAN = 0;
	

	return SUCCESS;	
}

enum{ TOTAL_REMOTE_DEVICE_NODE = 8 };
enum{ TOTAL_REMOTE_ENTRY_NODE = 8 };

static int findUnusedPVCIndex(char * wan_pvc, size_t size)
{
    char buf[16] = {0};
    int i = 0;

    for( i = 0; i < TOTAL_REMOTE_DEVICE_NODE; ++ i){
        snprintf(wan_pvc, size -1,"Wan_PVC%d", i);
        wan_pvc[size -1] = 0;
        if( TCAPI_SUCCESS != tcapi_get(wan_pvc, "VLANMode", buf) ){
            return i;
        }
    }
    
    return -1;
}

static int findUnUsedEntryIndex(int pvcIndex, char * wan_pvc_entry, size_t size){
	char tmpValue[32] = {0};
	int i = 0;

	for (i = 0; i < TOTAL_REMOTE_ENTRY_NODE; i++) {
		snprintf(wan_pvc_entry, size - 1, "Wan_PVC%d_Entry%d", pvcIndex, i);
		wan_pvc_entry[size -1] = 0;
		if(TCAPI_SUCCESS != tcapi_get(wan_pvc_entry, "Active", tmpValue)){
			return i;
		}
	}

	return -1;
}

#define TRY_TCAPI_SET(Node, attr, value, retCode)           \
    do{                                                     \
        if(TCAPI_SUCCESS != tcapi_set(Node, attr, value)){  \
            return retCode;                                 \
        }                                                   \
    }while(0)

static int getNewIFID()
{
    int pvc_idx = 0;
    int entry_idx = 0;
    char nodeName[32] = {0};
    char ifaceID[4] = {0};
    char IFIDs[100];
    u_char IFCount = 0;
    int i = 0;
    u_short newIFID = 0;
    
    memset(IFIDs, -1, 100);
    
	for ( pvc_idx = 0; pvc_idx < TOTAL_REMOTE_DEVICE_NODE; pvc_idx ++ ){
		for ( entry_idx = 0; entry_idx < TOTAL_REMOTE_ENTRY_NODE; entry_idx ++ ){
			sprintf(nodeName, "Wan_PVC%d_Entry%d", pvc_idx, entry_idx);
			if ( tcapi_get(nodeName, "IFIdx", ifaceID) < 0 )
				continue;

			IFIDs[IFCount]= atoi(ifaceID);
			++ IFCount;
		}
	}

    for( i = 0 ; i < IFCount && IFIDs[i] >= 0; ++ i ){
        if ( newIFID < IFIDs[i] ){
            newIFID = IFIDs[i];
        }
    }
    
    return ++ newIFID;
}

static int findOrCreateTR69Wan(int * pvc_index, int * entry_index, const char * vlan_str )
{
    char wan_pvc[32] = {0};
    char wan_pvc_entry[32] = {0};
	char buf[64] = {0};
	int  pvc_match_idx = -1;
	int  entry_match_idx = -1;
	int  newIFID = -1;
	int  i = 0;
	int  pvc_tmp_idx = 0;
	int  entry_tmp_idx = 0;

	/*
	    if there is already a TR69 wan, find it
	*/
	for(pvc_tmp_idx = 0; pvc_tmp_idx < PVC_NUM; pvc_tmp_idx++){
	    if(-1 == pvc_match_idx){
    	    snprintf(wan_pvc, sizeof(wan_pvc) -1,"Wan_PVC%d", pvc_tmp_idx);
    	    wan_pvc[sizeof(wan_pvc) -1] = 0;
    		if(tcapi_get(wan_pvc,"VLANID", buf) == TCAPI_SUCCESS ){
    			if(strcmp(buf, vlan_str) == 0){
    				pvc_match_idx = pvc_tmp_idx;
    			}
    		}
		}
		
		for(entry_tmp_idx = 0; entry_tmp_idx < MAX_SMUX_NUM; entry_tmp_idx++){
			memset(wan_pvc_entry, 0, sizeof(wan_pvc_entry));
			memset(buf, 0, sizeof(buf));			
			snprintf(wan_pvc_entry, sizeof(wan_pvc_entry) -1,"Wan_PVC%d_Entry%d", pvc_tmp_idx, entry_tmp_idx);
			wan_pvc_entry[sizeof(wan_pvc_entry) -1] = 0;
			if(tcapi_get(wan_pvc_entry,"ServiceList", buf) == TCAPI_SUCCESS ){
				if(strstr(buf, "TR069") != NULL){
				    *pvc_index = pvc_tmp_idx;
				    *entry_index = entry_tmp_idx;
					return 0;
				}
			}
		}
	}

    /*
        TR69 wan not found, should create one
    */
    if(-1 == pvc_match_idx){ // pvc has not been created yet, should create it
        pvc_match_idx = findUnusedPVCIndex(wan_pvc, sizeof(wan_pvc));

        if (pvc_match_idx < 0){
            eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "There is no free pvc for adding TR69 Wan!\n");
            return -1;
        }
        
        TRY_TCAPI_SET(wan_pvc, "VLANID", "0", -1);
    	TRY_TCAPI_SET(wan_pvc, "DOT1P", "0",  -1);
    	TRY_TCAPI_SET(wan_pvc, "VLANMode", "UNTAG", -1); //default UNTAG mode
    	TRY_TCAPI_SET(wan_pvc, "EPONEnable", "Yes", -1);
    	TRY_TCAPI_SET(wan_pvc, "GPONEnable", "Yes", -1);
    	TRY_TCAPI_SET(wan_pvc, "ENCAP", "1483 Bridged IP LLC", -1);
    	TRY_TCAPI_SET(wan_pvc, "Action","Add", -1);	
    }

    entry_match_idx = findUnUsedEntryIndex(pvc_match_idx, wan_pvc_entry, sizeof(wan_pvc_entry));

    if(entry_match_idx < 0){
        eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "There is no free entry for adding TR69 Wan!\n");
        return -1;
    }
    
    TRY_TCAPI_SET(wan_pvc_entry, "MTU","1492", -1);
    TRY_TCAPI_SET(wan_pvc_entry, "ServiceList","TR069", -1);
	TRY_TCAPI_SET(wan_pvc_entry, "BandActive","N/A", -1);
	TRY_TCAPI_SET(wan_pvc_entry, "WanMode","Route", -1);
	TRY_TCAPI_SET(wan_pvc_entry, "Active", "Yes", -1);
	TRY_TCAPI_SET(wan_pvc_entry, "PPPGETIP","Dynamic", -1);
	TRY_TCAPI_SET(wan_pvc_entry, "IPVERSION","IPv4", -1);

    newIFID = getNewIFID();
    if ( newIFID < 1 ){
        eponOamDbg(EPON_DEBUG_LEVEL_DEBUG, "%s:%n,can't get new ifid!!!\n",__FUNCTION__,__LINE__);
        return -1;
    }
    sprintf(buf, "%d", newIFID);
    TRY_TCAPI_SET(wan_pvc_entry, "IFIdx", buf, -1);
    TRY_TCAPI_SET("Wan_Common", "LatestIFIdx", buf, -1);

    for( i = 0; i < gCtcDB.dev_info.lan_num; ++ i){
        sprintf(buf, "Lan%d", i+1);
        TRY_TCAPI_SET(wan_pvc_entry, buf, "No", -1);
    }

    *pvc_index = pvc_match_idx;
    *entry_index = entry_match_idx;
	
    return 0;
}

char gTR069URL[256] = {0};
oamRMSGlbParaCfg1_t gTR069WanNodeInfo = {0};
int setTR069WanInfo(){	
	int  pvc_index;
	int  entry_index;
	char wan_pvc[32]={0};
	char wan_pvc_entry[32] = {0};
	char buf[64] = {0};
	char vlan_str[5] = {0};
	char *tmp = NULL;

	memset(vlan_str,0,sizeof(vlan_str));
	snprintf(vlan_str, sizeof(vlan_str) - 1, "%d", ntohs(gTR069WanNodeInfo.CVlan));
	vlan_str[sizeof(vlan_str) - 1] = 0;
    
	if(findOrCreateTR69Wan(&pvc_index, &entry_index, vlan_str) != 0){
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR,"\%s:%d, NOT find tr069!!!\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

    snprintf(wan_pvc,sizeof(wan_pvc)-1,"Wan_PVC%d",pvc_index);
    wan_pvc[sizeof(wan_pvc)-1] = 0;

    snprintf(wan_pvc_entry,sizeof(wan_pvc_entry)-1,"Wan_PVC%d_Entry%d",pvc_index,entry_index);
    wan_pvc_entry[sizeof(wan_pvc_entry)-1] = 0;
	
	if(gTR069WanNodeInfo.RMSIpMode == 0x00){//Static
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkIP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "1", VAR_NO_RESOURCE);
		
		tmp =  inet_ntoa(*((struct in_addr *) &(gTR069WanNodeInfo.RMSIpAddr)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "IPADDR", tmp,VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}
		
		tmp = NULL;
		tmp =  inet_ntoa(*((struct in_addr *) &(gTR069WanNodeInfo.RMSNetMask)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "NETMASK", tmp,VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}

		tmp = NULL;
		tmp =  inet_ntoa(*((struct in_addr *) &(gTR069WanNodeInfo.RMSDefaultGW)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "GATEWAY", tmp, VAR_NO_RESOURCE);
			// if DNS is not set, TR069 will not work
			TRY_TCAPI_SET(wan_pvc_entry, "DNS", tmp, VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}
	}else if(gTR069WanNodeInfo.RMSIpMode == 0x02){	//pppoe
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkPPP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "2",VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "USERNAME", gTR069WanNodeInfo.pppoeUsername,VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "PASSWORD", gTR069WanNodeInfo.pppoePassword, VAR_NO_RESOURCE);
	}else{	//dynamic
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkIP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "0", VAR_NO_RESOURCE);
	}
	//tagged flag
	if(gTR069WanNodeInfo.taggedFlag == 0x00){
		TRY_TCAPI_SET(wan_pvc, "VLANMode", "TRANSPARENT", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "VLANMode", "TRANSPARENT", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1q", "No", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1p", "No", VAR_NO_RESOURCE);
	}else{
		TRY_TCAPI_SET(wan_pvc, "VLANMode", "TAG", VAR_NO_RESOURCE);
        TRY_TCAPI_SET(wan_pvc_entry, "VLANMode", "TAG", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1q", "Yes", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1p", "Yes", VAR_NO_RESOURCE);
    	//CVLan
    	TRY_TCAPI_SET(wan_pvc, "VLANID", vlan_str, VAR_NO_RESOURCE);
    	TRY_TCAPI_SET(wan_pvc_entry, "VLANID", vlan_str, VAR_NO_RESOURCE);
    	//Priority
    	memset(buf,0,sizeof(buf));
    	sprintf(buf, "%d", gTR069WanNodeInfo.priority);
    	TRY_TCAPI_SET(wan_pvc, "DOT1P", buf, VAR_NO_RESOURCE);
    	TRY_TCAPI_SET(wan_pvc_entry, "dot1pData", buf, VAR_NO_RESOURCE);
	}

	tcapi_commit(wan_pvc);
	return 0;
}


u_char oamSetRMSGlobalParameterConfig1(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue){
	int  pvc_index;
	int  entry_index;
	char wan_pvc[32]={0};
	char wan_pvc_entry[32] = {0};
	char buf[64] = {0};
	char vlan_str[5] = {0};
	oamRMSGlbParaCfg1_Ptr rmsCfg1_ptr = (oamRMSGlbParaCfg1_Ptr)pValue;
	char *tmp = NULL;

#if 1
	gTR069WanNodeInfo.RMSIpMode = rmsCfg1_ptr->RMSIpMode;
	gTR069WanNodeInfo.RMSIpAddr	= rmsCfg1_ptr->RMSIpAddr;
	gTR069WanNodeInfo.RMSNetMask= rmsCfg1_ptr->RMSNetMask;
	gTR069WanNodeInfo.RMSDefaultGW = rmsCfg1_ptr->RMSDefaultGW;
	gTR069WanNodeInfo.pppoeMode = rmsCfg1_ptr->pppoeMode;
	strcpy(gTR069WanNodeInfo.pppoeUsername, rmsCfg1_ptr->pppoeUsername);
	strcpy(gTR069WanNodeInfo.pppoePassword, rmsCfg1_ptr->pppoePassword);
	gTR069WanNodeInfo.taggedFlag = rmsCfg1_ptr->taggedFlag;
	gTR069WanNodeInfo.CVlan = rmsCfg1_ptr->CVlan;
	gTR069WanNodeInfo.SVLAN= rmsCfg1_ptr->SVLAN;
	gTR069WanNodeInfo.priority= rmsCfg1_ptr->priority;
#else
	memset(vlan_str,0,sizeof(vlan_str));
	snprintf(vlan_str, sizeof(vlan_str) - 1, "%d", ntohs(rmsCfg1_ptr->CVlan));
	vlan_str[sizeof(vlan_str) - 1] = 0;
    
	if(findOrCreateTR69Wan(&pvc_index, &entry_index, vlan_str) != 0){
		eponOamDbg(EPON_DEBUG_LEVEL_ERROR,"\%s:%d, NOT find tr069!!!\n",__FUNCTION__,__LINE__);
		return VAR_NO_RESOURCE;
	}

    snprintf(wan_pvc,sizeof(wan_pvc)-1,"Wan_PVC%d",pvc_index);
    wan_pvc[sizeof(wan_pvc)-1] = 0;

    snprintf(wan_pvc_entry,sizeof(wan_pvc_entry)-1,"Wan_PVC%d_Entry%d",pvc_index,entry_index);
    wan_pvc_entry[sizeof(wan_pvc_entry)-1] = 0;
	
	if(rmsCfg1_ptr->RMSIpMode == 0x00){//Static
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkIP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "1", VAR_NO_RESOURCE);
		
		tmp =  inet_ntoa(*((struct in_addr *) &(rmsCfg1_ptr->RMSIpAddr)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "IPADDR", tmp,VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}
		
		tmp = NULL;
		tmp =  inet_ntoa(*((struct in_addr *) &(rmsCfg1_ptr->RMSNetMask)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "NETMASK", tmp,VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}

		tmp = NULL;
		tmp =  inet_ntoa(*((struct in_addr *) &(rmsCfg1_ptr->RMSDefaultGW)));
		if(tmp){
			TRY_TCAPI_SET(wan_pvc_entry, "GATEWAY", tmp, VAR_NO_RESOURCE);
			// if DNS is not set, TR069 will not work
			TRY_TCAPI_SET(wan_pvc_entry, "DNS", tmp, VAR_NO_RESOURCE);
		}else{
			return VAR_BAD_PARAMETERS;
		}
	}else if(rmsCfg1_ptr->RMSIpMode == 0x02){	//pppoe
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkPPP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "2",VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "USERNAME", rmsCfg1_ptr->pppoeUsername,VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "PASSWORD", rmsCfg1_ptr->pppoePassword, VAR_NO_RESOURCE);
	}else{	//dynamic
	    TRY_TCAPI_SET(wan_pvc_entry, "LinkMode", "linkIP", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "ISP", "0", VAR_NO_RESOURCE);
	}
	//tagged flag
	if(rmsCfg1_ptr->taggedFlag == 0x00){
		TRY_TCAPI_SET(wan_pvc, "VLANMode", "TRANSPARENT", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "VLANMode", "TRANSPARENT", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1q", "No", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1p", "No", VAR_NO_RESOURCE);
	}else{
		TRY_TCAPI_SET(wan_pvc, "VLANMode", "TAG", VAR_NO_RESOURCE);
        TRY_TCAPI_SET(wan_pvc_entry, "VLANMode", "TAG", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1q", "Yes", VAR_NO_RESOURCE);
		TRY_TCAPI_SET(wan_pvc_entry, "dot1p", "Yes", VAR_NO_RESOURCE);
    	//CVLan
    	TRY_TCAPI_SET(wan_pvc, "VLANID", vlan_str, VAR_NO_RESOURCE);
    	TRY_TCAPI_SET(wan_pvc_entry, "VLANID", vlan_str, VAR_NO_RESOURCE);
    	//Priority
    	memset(buf,0,sizeof(buf));
    	sprintf(buf, "%d", rmsCfg1_ptr->priority);
    	TRY_TCAPI_SET(wan_pvc, "DOT1P", buf, VAR_NO_RESOURCE);
    	TRY_TCAPI_SET(wan_pvc_entry, "dot1pData", buf, VAR_NO_RESOURCE);
	}
#endif	
	postDBSetSyncMsg(DB_SYNC_RMS_CONFIG1);

	return SET_OK;
}

u_char oamGetRMSGlobalParameterConfig2(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue){
	char buf[256] = {0};
	oamRMSGlbParaCfg2_Ptr rmsCfg2_ptr = (oamRMSGlbParaCfg2_Ptr)pValue;
	int len = 0;

	if (TCAPI_SUCCESS != tcapi_get("Cwmp_Entry", "acsUrl", buf)){
    		return VAR_NO_RESOURCE;
    }
	rmsCfg2_ptr->numOfURLBlocks = 1;
	rmsCfg2_ptr->seqOfURLBlock = 0;
	len = strlen(buf);
	*pLength = (len <= 125 ? len + 2 : 127);
	if(len <= 125)
		strncpy(rmsCfg2_ptr->RMS_URL, buf, len);
	else
		strncpy(rmsCfg2_ptr->RMS_URL, buf, 125);

	return SUCCESS;
}

typedef struct rmsCfg2Data_s
{
    struct rmsCfg2Data_s *next;
    u_char  blockNo;
    u_char  length;
    u_int   data[125];
}rmsCfg2Data_t, *rmsCfg2Data_Ptr;

typedef struct rmsCfg2DataMgr_s
{
    u_char  state;// NULL=0/BEGIN=1
    u_char  num;     // all block num
    u_char  curNum; // current recivie num
    u_int   length; // the recivie blocks length sum
    rmsCfg2Data_Ptr end;
    rmsCfg2Data_Ptr list;
}rmsCfg2DataMgr_t, *rmsCfg2DataMgr_Ptr;

rmsCfg2DataMgr_t grmsCfg2DataMgr = {0};

void freermsCfg2DataMgr(rmsCfg2DataMgr_Ptr prmsCfg2Mgr)
{
    if (prmsCfg2Mgr){
        // free the list
        rmsCfg2Data_Ptr pData = prmsCfg2Mgr->list;
        while(pData){
            prmsCfg2Mgr->list = pData->next;
            free(pData);
            pData = prmsCfg2Mgr->list;
        }
        // zero the Mgr object
        memset(prmsCfg2Mgr, 0, sizeof(rmsCfg2DataMgr_t));
    }
}

u_char oamSetRMSGlobalParameterConfig2(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue){
    oamRMSGlbParaCfg2_Ptr rmsCfg2_ptr = (oamRMSGlbParaCfg2_Ptr)pValue;
    rmsCfg2Data_Ptr pCfgData = NULL;

    if (length <= 2){
            return VAR_BAD_PARAMETERS;
    }
     
    if (rmsCfg2_ptr->seqOfURLBlock!= grmsCfg2DataMgr.curNum){
        if(0 == rmsCfg2_ptr->seqOfURLBlock ){
            freermsCfg2DataMgr(&grmsCfg2DataMgr);
        }else{
            goto fail;
        }
    }

    // alloc memory for new block and copy data
    pCfgData = (rmsCfg2Data_Ptr)malloc(sizeof(rmsCfg2Data_t));
    if(NULL == pCfgData){
        goto fail;
    }
    pCfgData->next = NULL;
    pCfgData->blockNo = rmsCfg2_ptr->seqOfURLBlock;
    pCfgData->length = length - 2;
    memcpy(pCfgData->data, pValue+2, pCfgData->length); 

    if (grmsCfg2DataMgr.state == 0){
        grmsCfg2DataMgr.state  = 1;
        grmsCfg2DataMgr.curNum = 1;
        grmsCfg2DataMgr.num    = rmsCfg2_ptr->numOfURLBlocks;
        grmsCfg2DataMgr.length = pCfgData->length;
        grmsCfg2DataMgr.list = grmsCfg2DataMgr.end = pCfgData;
    }else if (grmsCfg2DataMgr.state == 1){
        grmsCfg2DataMgr.length += pCfgData->length;
        if( grmsCfg2DataMgr.length >= MAXLEN_TCAPI_MSG){
            goto fail;
        }
        ++ grmsCfg2DataMgr.curNum;
        grmsCfg2DataMgr.end->next = pCfgData;
        grmsCfg2DataMgr.end       = pCfgData;
    }else{
        goto fail;
    }

    if (grmsCfg2DataMgr.curNum == grmsCfg2DataMgr.num){ // all blocks received
        char *block = (u_char*)malloc(grmsCfg2DataMgr.length);
        if(NULL == block){
            goto fail;
        }
        char *dp = block;
        rmsCfg2Data_Ptr pCfgData = grmsCfg2DataMgr.list;

        while(pCfgData){
            memcpy(dp, pCfgData->data, pCfgData->length);
            PTR_MOVE(dp, pCfgData->length);
            pCfgData = pCfgData->next;
        }

        *dp=0;
#if 0
        if (0 != tcapi_set("Cwmp_Entry", "acsUrl", block)){
            goto fail;
        }
#else
		strcpy(gTR069URL,block);
#endif
        freermsCfg2DataMgr(&grmsCfg2DataMgr);

	postDBSetSyncMsg(DB_SYNC_RMS_ACSURL_CONFIG);
    }
    
    return SET_OK;
    
fail:
    freermsCfg2DataMgr(&grmsCfg2DataMgr);
    return VAR_NO_RESOURCE;
}

u_char gLoopDetectPortId = 0;
u_int  gLoopDetectActive = 0;
int doOamSetCucPortLoopDetect(){
	char buf[16];
	u_int LoopDetectMask=0;
	
	if (TCAPI_SUCCESS != tcapi_get("LoopDetect_Entry","LoopDetectMask", buf) ){
	  return VAR_NO_RESOURCE;
	}
	buf[sizeof(buf)-1] = 0;
	sscanf(buf, "%x",&LoopDetectMask);
	LoopDetectMask &= 0x0F;

	switch(gLoopDetectActive)
	{
	case DISABLED:
	  LoopDetectMask &= ~(1 << gLoopDetectPortId);
	  break;
	case ENABLED:
	  LoopDetectMask |= (1 << gLoopDetectPortId);
	  break;
	default:
	  return VAR_BAD_PARAMETERS;
	}

	snprintf(buf,sizeof(buf)-1,"0x%02X",LoopDetectMask);
	buf[sizeof(buf)-1]=0;

	if ( TCAPI_SUCCESS != tcapi_set("LoopDetect_Entry","LoopDetectMask",buf) ){
	  return VAR_NO_RESOURCE;
	}

	tcapi_commit("LoopDetect_Entry");
    return 0;
}

u_char oamSetCucPortLoopDetect(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);      
    u_int  active = GetU32(pValue);

    if (portId > 3)
        return VAR_BAD_PARAMETERS;

#if 1
	gLoopDetectPortId = portId;
	gLoopDetectActive = active;
#else
    char buf[16];
    u_int LoopDetectMask=0;
    if (TCAPI_SUCCESS != tcapi_get("LoopDetect_Entry","LoopDetectMask", buf) ){
        return VAR_NO_RESOURCE;
    }
    buf[sizeof(buf)-1] = 0;
    sscanf(buf, "%x",&LoopDetectMask);
    LoopDetectMask &= 0x0F;
    
    switch(active)
    {
    case DISABLED:
        LoopDetectMask &= ~(1 << portId);
        break;
    case ENABLED:
        LoopDetectMask |= (1 << portId);
        break;
    default:
        return VAR_BAD_PARAMETERS;
    }
    
    snprintf(buf,sizeof(buf)-1,"0x%02X",LoopDetectMask);
    buf[sizeof(buf)-1]=0;
    
    if ( TCAPI_SUCCESS != tcapi_set("LoopDetect_Entry","LoopDetectMask",buf) ){
        return VAR_NO_RESOURCE;
    }
#endif
    postDBSetSyncMsg(DB_SYNC_LOOP_DETECT);
    
    return SET_OK;
}

u_char oamGetCucPortLoopDetect(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);
    char buf[8];
    u_int LoopDetectMask = 0;
    memset(buf,0,sizeof(buf));

    if (portId >3)
        return VAR_BAD_PARAMETERS;
   
    if(TCAPI_SUCCESS != tcapi_get("LoopDetect_Entry", "LoopDetectMask", buf)){
        return VAR_BAD_PARAMETERS;
    }
    buf[sizeof(buf)-1] = 0;
    sscanf(buf, "%x",&LoopDetectMask);
    LoopDetectMask &= 0x0F;

    if(LoopDetectMask & (1 << portId)){
        setU32(pValue, ENABLED);
    }else{
        setU32(pValue, DISABLED);
    }

    *pLength = 4;
    return SUCCESS;
}

u_char gDisableLoopedPortId = 0;
u_int  gDisableLoopedState = 0;
int doOamCucSetPortDisableLooped(){	
    char buf[16];
    u_int LoopAutoShutMask;
    if (TCAPI_SUCCESS != tcapi_get("LoopDetect_Entry","LoopAutoShutMask", buf) ){
        return VAR_NO_RESOURCE;
    }
    sscanf(buf, "%x",&LoopAutoShutMask);
    LoopAutoShutMask &= 0x0F;
    
    switch(gDisableLoopedState)
    {
    case DISACTIVE:
        LoopAutoShutMask &= ~(1 << gDisableLoopedPortId);
        break;
    case ACTIVE:
        LoopAutoShutMask |= (1 << gDisableLoopedPortId);
        break;
    default:
        return VAR_BAD_PARAMETERS;
    }
    
    snprintf(buf, sizeof(buf)-1,"0x%02X",LoopAutoShutMask);
    buf[sizeof(buf)-1]=0;
    
    if ( TCAPI_SUCCESS != tcapi_set("LoopDetect_Entry","LoopAutoShutMask",buf)||
         TCAPI_SUCCESS != tcapi_commit("LoopDetect_Entry")){
        return VAR_NO_RESOURCE;
    }
}


u_char oamCucSetPortDisableLooped(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
    u_char portId = sysObjIdx(pObj, PORT_ETHER);      
    u_int  state = GetU32(pValue);

    if (portId > 3)
        return VAR_BAD_PARAMETERS;

#if 1
	gDisableLoopedPortId = portId;
	gDisableLoopedState = state;
	postDBSetSyncMsg(DB_SYNC_DISABLE_LOOPED_CONFIG);
#else
    char buf[16];
    u_int LoopAutoShutMask;
    if (TCAPI_SUCCESS != tcapi_get("LoopDetect_Entry","LoopAutoShutMask", buf) ){
        return VAR_NO_RESOURCE;
    }
    sscanf(buf, "%x",&LoopAutoShutMask);
    LoopAutoShutMask &= 0x0F;
    
    switch(state)
    {
    case DISACTIVE:
        LoopAutoShutMask &= ~(1 << portId);
        break;
    case ACTIVE:
        LoopAutoShutMask |= (1 << portId);
        break;
    default:
        return VAR_BAD_PARAMETERS;
    }
    
    snprintf(buf, sizeof(buf)-1,"0x%02X",LoopAutoShutMask);
    buf[sizeof(buf)-1]=0;
    
    if ( TCAPI_SUCCESS != tcapi_set("LoopDetect_Entry","LoopAutoShutMask",buf)||
         TCAPI_SUCCESS != tcapi_commit("LoopDetect_Entry")){
        return VAR_NO_RESOURCE;
    }
#endif
	
    return SET_OK;
}

/* --------------------------ONU attributions and actions--------------------------- */


/* --------------------------ONU attributions and actions end----------------------- */

/* --------------------------Qos attributions and actions---------------------------- */
/*
#ifndef TCSUPPORT_EPON_MAPPING
u_char oamGetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	return VAR_NO_RESOURCE;
}
u_char oamSetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	return VAR_NO_RESOURCE;
}
#else
//@TODO
u_char oamGetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, OUT int *pLength, OUT u_char *pValue)
{
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	u_char *dp = pValue;
	u_char matchNum, i, j;
	OamCM_Ptr pClsfy = (OamCM_Ptr)pValue;
	QosResult_t result;
	QosMatchRule_t matchs[MAX_FIELD_NUM];
	OamQosRule_Ptr pRule;
	OamQosRuleEntry4_Ptr pEntry4;
	OamQosRuleEntry6_Ptr pEntry6;
	int portRulesNum = -1;

	if (portId == PORT_ERROR)
		return VAR_BAD_PARAMETERS;
	
	portRulesNum = eponmapGetClsfyRulesNum(portId);
	if (portRulesNum == FAIL)
		return VAR_NO_RESOURCE;
	eponOamExtDbg(DBG_OAM_L4, "portRulesNum: %d\n", portRulesNum);

	pClsfy->action = ACTION_LIST;
	pClsfy->rulesNum = portRulesNum;
	dp += sizeof(OamCM_t);
	
	for (i = 0; i < portRulesNum; ++i){
		pRule = (OamQosRule_Ptr)dp;
		matchNum = MAX_FIELD_NUM;
		if (0 == eponmapGetClsfyRule(portId, i, &result, &matchNum, matchs)){
			pRule->precedence = result.precedence;
			pRule->queueMapped = result.queueMapped;
			pRule->etherPriorityMark = result.priority;
			pRule->entriesNum = matchNum;
			pRule->length = 3;

			dp += sizeof(OamQosRule_t);
			for (j = 0; j < matchNum; ++j){
				if (matchs[j].field < FIELD_IPVER){
					pRule->length += sizeof(OamQosRuleEntry4_t);
					pEntry4 = (OamQosRuleEntry4_Ptr)dp;
					pEntry4->field = matchs[j].field;
					pEntry4->validOperator = matchs[j].op;
					
					memset(pEntry4->matchs, 0, ETH_ALEN);
					switch(pEntry4->field){
						case FIELD_SMAC:
						case FIELD_DMAC:
							memcpy(pEntry4->matchs, matchs[j].mac, ETH_ALEN);
						break;
						case FIELD_PBIT:
						case FIELD_IPPROTO4:
							pEntry4->matchs[ETH_ALEN-1] = matchs[j].v8;
						break;						
						case FIELD_VLANID:
						case FIELD_ETHTYPE:
						case FIELD_SPORT:
						case FIELD_DPORT:
							pEntry4->matchs[ETH_ALEN-2] = matchs[j].v16 >> 8;
							pEntry4->matchs[ETH_ALEN-1] = matchs[j].v16 & 0x00FF;
						break;						
						case FIELD_SIP4:
						case FIELD_DIP4:
							pEntry4->matchs[ETH_ALEN-4] = matchs[j].ip4 >> 24;
							pEntry4->matchs[ETH_ALEN-3] = matchs[j].ip4 >> 16;
							pEntry4->matchs[ETH_ALEN-2] = matchs[j].ip4 >> 8;
							pEntry4->matchs[ETH_ALEN-1] = matchs[j].ip4;
						break;
					}
					dp += sizeof(OamQosRuleEntry4_t);
				}else{
					pRule->length += sizeof(OamQosRuleEntry6_t);
					pEntry6 = (OamQosRuleEntry6_Ptr)dp;
					pEntry6->field = matchs[j].field;
					pEntry6->validOperator = matchs[j].op;
					
					switch(pEntry6->field){
						case FIELD_SIP6:
						case FIELD_DIP6:
							memcpy(pEntry6->matchs, matchs[j].ip6.s6_addr, 16);
						break;
						case FIELD_IPVER:
						case FIELD_IPPROTO6:
							memset(pEntry6->matchs, 0, 16);
							pEntry6->matchs[15] = matchs[j].v8;
						break;
					}
					dp += sizeof(OamQosRuleEntry6_t);
				}
			}
		}else
			return VAR_NO_RESOURCE;
	}

	*pLength = dp-pValue;
	return SUCCESS;
}
//@TODO
u_char oamSetClassificationMarking(IN u_char llidIdx, IN OamObject_Ptr pObj, IN int length, IN u_char *pValue)
{
	u_char portId = sysObjIdx(pObj, PORT_ETHER);
	u_char *dp = pValue;
	u_char rulesNum, matchsNum, field, i, j;
	OamCM_Ptr pClsfy = (OamCM_Ptr)pValue;
	QosResult_t result;
	QosMatchRule_t matchs[MAX_FIELD_NUM];

	OamQosRule_Ptr pRule;
	OamQosRuleEntry4_Ptr pEntry4;
	OamQosRuleEntry6_Ptr pEntry6;
	int ret = 0;

	if (PORT_ERROR == portId)
		return VAR_BAD_PARAMETERS;

	if (*pValue == ACTION_CLEAR){
		if (0 == eponmapClearClsfyRules(portId))
			return SET_OK;

		return VAR_NO_RESOURCE;
	}

	length -= sizeof(OamCM_t);
	if (length < 0)
		return VAR_BAD_PARAMETERS;

	rulesNum = pClsfy->rulesNum;
	dp += sizeof(OamCM_t);

	// TODO: need process the length < the real need
	for (i = 0; i < rulesNum; ++i){
		pRule = (OamQosRule_Ptr)dp;

		length -= pRule->length + 1;
		if (length < 0)
			return VAR_BAD_PARAMETERS;

		dp += sizeof(OamQosRule_t);

		result.precedence  = pRule->precedence;
		result.priority    = pRule->etherPriorityMark;
		result.queueMapped = pRule->queueMapped;

		matchsNum = pRule->entriesNum;

		for (j = 0; j < matchsNum; ++j)
		{
			field = *dp;
			matchs[j].field = field;
			if (field < FIELD_IPVER){
				pEntry4 = (OamQosRuleEntry4_Ptr)dp;
				dp += sizeof(OamQosRuleEntry4_t);

				eponOamExtDbg(DBG_OAM_L4, "QosRule: matchs: %.2x%.2x%.2x%.2x%.2x%.2x\n", pEntry4->matchs[0],\
					pEntry4->matchs[1],pEntry4->matchs[2],pEntry4->matchs[3],pEntry4->matchs[4],pEntry4->matchs[5]);
				switch(pEntry4->field){
					case FIELD_SMAC:
					case FIELD_DMAC:
						memcpy(matchs[j].mac, pEntry4->matchs, 6);
					break;
					case FIELD_PBIT:
					case FIELD_IPPROTO4:
					case FIELD_IPDSCP4:
					case FIELD_IPDSCP6:
						matchs[j].v8 = pEntry4->matchs[5];
					break;						
					case FIELD_VLANID:
					case FIELD_ETHTYPE:
					case FIELD_SPORT:
					case FIELD_DPORT:						
						matchs[j].v16 = ((pEntry4->matchs[4] << 8) |
										pEntry4->matchs[5]);
					break;						
					case FIELD_SIP4:
					case FIELD_DIP4:
						matchs[j].ip4 = (   (pEntry4->matchs[2] << 24)|
										(pEntry4->matchs[3] << 16)|
										(pEntry4->matchs[4] << 8)|
										(pEntry4->matchs[5]) );
					break;
				}
				// memcpy(rules[j].match.value1, pEntry4->matchs, 6);
				matchs[j].op = pEntry4->validOperator;
			}else{
				pEntry6 = (OamQosRuleEntry6_Ptr)dp;
				dp += sizeof(OamQosRuleEntry6_t);

				switch(pEntry6->field){
					case FIELD_SIP6:
					case FIELD_DIP6:
					case FIELD_SIP6PREX:
					case FIELD_DIP6PREX:
						memcpy(matchs[j].ip6.s6_addr, pEntry6->matchs, 16);
					break;
					case FIELD_IPVER:
					case FIELD_IPPROTO6:
						memset(pEntry6->matchs, 0, 16);
						matchs[j].v8 = pEntry6->matchs[15];
					break;
					case FIELD_FLOWLABEL6:
						matchs[j].ip4 = (   (pEntry6->matchs[13] << 16)|
										(pEntry6->matchs[14] << 8)|
										(pEntry6->matchs[15]) );
					break;
					default:
						continue;
				}
				matchs[j].op = pEntry6->validOperator;
			}
		}		

		switch(pClsfy->action){
			case ACTION_ADD:
			ret = eponmapInsertClsfyRule(portId, &result, matchsNum, matchs);
			break;
			case ACTION_DELETE:
			ret = eponmapDeleteClsfyRule(portId, &result, matchsNum, matchs);
			break;
		}
		if (ret == FAIL){
			return VAR_NO_RESOURCE;
		}
	}

	return SET_OK;
}
#endif
*/
/* --------------------------Qos attributions and actions end------------------------ */

