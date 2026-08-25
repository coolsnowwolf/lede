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
	oam_ctc_node.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Shengyi		2012/09/26		Create
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
#include "ctc/oam_cuc_init.h"

#ifdef TCSUPPORT_EPON_MAPPING
#include "libeponmap.h"
#endif

#ifdef TCSUPPORT_MT7530_SWITCH_API
#define u8  unsigned char 
#define u16 unsigned short
#define u32 unsigned int
#include "mtkswitch_api.h"
#endif

#ifdef TCSUPPORT_XPON_IGMP
#include "lib_xpon_igmp.h"
#endif

#ifdef TCSUPPORT_PON_VLAN
#include "lib_pon_vlan.h"
#endif

/*
 * CUC Attributes init table.
 */
OamParamInit_t cucOamInitTable[]=
{
	{"aFECmode",	oamInitFecMode,	NULL},
	
	{"PowerSavingConfig",	oamInitPowerSavingConfig,	NULL},
	{"ProtectionParameters",	oamInitProtectionParameters,	NULL},

	{"ServiceSLA",	oamInitServiceSLA,	NULL},
	{"HoldoverConfig",	oamInitHoldoverConfig,	NULL},
	{"ActivePON_IFAdminstate",	oamInitActivePONIFAdminstate,	NULL},

	{"EthPortPause",	oamInitEthPortPause,	NULL},
	{"EthPortUSPolicing",	oamInitEthPortUSPolicing,	NULL},
	{"VoIPPort", oamInitVoIPPort,	NULL},
	{"E1Port",	oamInitE1Port,	NULL},
	{"EthPortDSRateLimiting",	oamInitEthPortDSRateLimiting,	NULL},
	{"PortLoopDetect",	oamInitPortLoopDetect,	NULL},
	{"PortDisableLooped",	oamInitPortDisableLooped,	NULL},
	{"PortMACAgingTime",	oamInitPortMacAgingTime,	NULL},
	{"OnuMacLimit",	oamInitPortMacLimit,	NULL},
	{"OnuPortModeCfg",	oamInitPortModeCfg,	NULL},

	{"VLAN",	oamInitVlan,	NULL},
	{"ClassificationMarking",	oamInitClassificationMarking,	NULL},
	{"AddDelMulticastVLAN",	oamInitMulticastVlan,	NULL},
	{"MulticastTagOper",	oamInitMulticastTagOper,	NULL},
	{"MulticastSwitch",	oamInitMulticastSwitch,	NULL},
	{"MulticastControl",	oamInitMulticastControl,	NULL},

	{"GroupNumMax",	oamInitGroupNumMax,	NULL},

	{"LLIDQueueConfig",	oamInitLlidQueueConfig,	NULL},
	{"MxUMngGlobalParameter", oamInitMxUMngGlobalParameter, NULL},
	{"MxUMngSNMPParameter", oamSetMxUMngSNMPParameter,	NULL},
	{"ONUTxPowerSupplyControl", oamInitONUTxPowerSupplyControl,	NULL},

	{"GlobalParameterConfig",	oamInitGlobalParameterConfig,	NULL},
	{"SIPParameterConfig",	oamInitSIPParameterConfig,	NULL},
	{"SIPUserParameterConfig",	oamInitSIPUserParameterConfig,	NULL},
	{"FaxModemConfig",	oamInitFaxModemConfig,	NULL},

	
	{"AlarmAdminState", oamInitAlarmAdminState,	NULL},
	{"AlarmThreshold",	oamInitAlarmThresholds,	NULL},
#ifdef TCSUPPORT_PMMGR
	{"PerformanceMonitorStatus",	oamInitPerformanceStatus,	NULL},
#endif	

	{"acPhyAdminControl",	oamInitPhyAdminControl,	NULL},
	{"acAutoNegAdminControl",	oamInitAutoNegAdminControl,	NULL},
	
	{"SleepControl",	oamInitSleepControl,	NULL},
	{"acMultiLLIDAdminControl",	oamInitMultiLlidAdminControl,	NULL},
	{"acFastLeaveAdminControl",	oamInitFastLeaveAdminControl,	NULL},

	{"SIPDigitMap", oamInitSIPDigitMap,	NULL},
	{"", NULL, NULL}
};

int oamInitFecMode(void){

	int i;
	char node[64] = {0};
	char fecMode[8] = {0};
	
	for(i=0; i<LLID_NUM_MAX; i++){
		sprintf(node, OAM_CUC_LLID, i);

		if (TCAPI_SUCCESS != tcapi_get(OAM_CUC, "FECMode", fecMode))
			continue;

		if(strcmp(fecMode, "0") == 0) //disable fec
			eponApiSetLlidFec(i, 0);		
		else if(strcmp(fecMode, "1") == 0) //enable fec
			eponApiSetLlidFec(i, 1);
	}

	return SUCCESS;
}

int oamInitPowerSavingConfig(void){
	
	return SUCCESS;
}

int oamInitProtectionParameters(void){

	if (TCAPI_SUCCESS != tcapi_commit(EPON_PON)){
		return FAIL;
	}
	
	return SUCCESS;
}

int oamInitServiceSLA(void){

	char buf[20] = {0};
	char attr[40] = {0};
	
#ifdef TCSUPPORT_EPON_MAPPING
	QueueWeight_t queueWeights[LLID_QUEUE_NUM_MAX];
	u_char i;
	u_char servicesNum;
	uint bestEffortSchedulingScheme;
	u_char highPriorityBoundary;	
#endif /*TCSUPPORT_EPON_MAPPING */

	if (TCAPI_SUCCESS != tcapi_commit(EPON_SERVICESLA))
		return FAIL;

#ifdef TCSUPPORT_EPON_MAPPING
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, "Enable", buf))
		return FAIL;

	if (strcmp(buf, "No") == 0)
		return SUCCESS;

	
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, "Num", buf))
		return FAIL;

	servicesNum = atoi(buf);
	if(servicesNum <= 0)
		return SUCCESS;

	for (i=0; i<servicesNum; ++i){
		// priority queue index
		sprintf(attr, "PriQ%d", i);
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, attr, buf))
			return FAIL;
			
		queueWeights[i].priQueue = atoi(buf);

		// queue wrr
		sprintf(attr, "WRR%d", i);
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, attr, buf))
			return FAIL;

		queueWeights[i].weight = atoi(buf);

		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, "BestScheduleScheme", buf))
			return FAIL;
		bestEffortSchedulingScheme = atoi(buf);

		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(EPON_SERVICESLA, "HighPriBoundary", buf))
			return FAIL;
		highPriorityBoundary = atoi(buf);

		if (bestEffortSchedulingScheme == 2 && highPriorityBoundary <= queueWeights[i].weight)
			queueWeights[i].weight = 0; // sp+wrr mode: weight exceed high boundary
		
	}

	if (FAIL == eponmapSetLlidQueueMapping(0, servicesNum, queueWeights))
		return FAIL;

	system("echo pwan qos 1 > /proc/gpon/debug");
	
#endif /*TCSUPPORT_EPON_MAPPING */

	return SUCCESS;

}

int oamInitHoldoverConfig(void){

	// Has done in oamInitProtectionParameters
#if 0
	if (TCAPI_SUCCESS != tcapi_commit(EPON_PON)){
		return FAIL;
	}
#endif

	return SUCCESS;
}

int oamInitActivePONIFAdminstate(void){

	//nothing need to do
	return SUCCESS;
}

int oamInitEthPortPause(void){

	u_char portId;
	u_char state;
	
	char node[32] = {0};
	char buf[8] = {0};

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "FlowControl", buf))
			continue;

		state = atoi(buf);
		switch(state){
			case DISACTIVE:
			case ACTIVE:
				macMT7530SetPortPause(portId, state);
				break;
			default:
				break;
		}
	}

	return SUCCESS;
}

int oamInitEthPortUSPolicing(void){

	char node[32] = {0};
	char attr[32] = {0};
	char buf[32] = {0};
	u_char portId;
	u_char USPolicingEnable = 0;	
	u_int cir=0, cbs=0, ebs=0;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);

		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(node, "USPolicingEnable", buf))
			continue;

		USPolicingEnable = atoi(buf);

		if (USPolicingEnable == DISACTIVE){
			macMT7530SetPortPolicingEnable(portId, DISACTIVE);
		}else if (USPolicingEnable == ACTIVE){
			macMT7530SetPortPolicingEnable(portId, ACTIVE);
			sprintf(attr, "%u", cir);
			if (TCAPI_SUCCESS != tcapi_get(node, "USPolicingCIR", buf))
				continue;
			cir = atoi(buf);
			
			sprintf(attr, "%u", cbs);
			if (TCAPI_SUCCESS != tcapi_get(node, "USPolicingCBS", buf))
				continue;
			cbs = atoi(buf);
			
			sprintf(attr, "%u", ebs);
			if (TCAPI_SUCCESS != tcapi_get(node, "USPolicingEBS", buf))
				continue;
			ebs = atoi(buf);

			macMT7530SetPortPolicing(portId, cir, cbs, ebs);
		}

	}

	return SUCCESS;
}

int oamInitVoIPPort(void){
	return SUCCESS;
}

int oamInitE1Port(void){
	
	return SUCCESS;
}

int oamInitEthPortDSRateLimiting(void){

	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_char DSRateLimitEnable = 0;
	u_int cir, pir;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "DSRateLimitEnable", buf))
			continue;

		DSRateLimitEnable = atoi(buf);

		if(DSRateLimitEnable == DISACTIVE){
			macMT7530SetPortRatelimitingEnable(portId, DISACTIVE);
		}else if(DSRateLimitEnable == ACTIVE){
			macMT7530SetPortRatelimitingEnable(portId, ACTIVE);

			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, "DSRateLimitCIR", buf)){
				return VAR_NO_RESOURCE;
			}	
			cir = atoi(buf);

			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, "DSRateLimitPIR", buf)){
				return VAR_NO_RESOURCE;
			}
			pir = atoi(buf);
			
			macMT7530SetPortRatelimiting(portId, cir, pir);
		}
		
	}
	
	return SUCCESS;
}

int oamInitPortLoopDetect(void)
{
	//TODO: not only ether port
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_char enable = 0;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "LoopDetect", buf))
			continue;

		enable = atoi(buf);

		if(enable == DISACTIVE){
			macMT7530SetPortLoopDetect(portId, DISACTIVE);
		}else if(enable == ACTIVE){
			macMT7530SetPortLoopDetect(portId, ACTIVE);
		}
	}
	
	return SUCCESS;
}

int oamInitPortDisableLooped(void)
{
	//TODO: not only ether port
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_char active = 0;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "DisableLooped", buf))
			continue;

		active = atoi(buf);

		if(active == DISACTIVE){
			macMT7530SetPortDisableLooped(portId, DISACTIVE);
		}else if(active == ACTIVE){		
			macMT7530SetPortDisableLooped(portId, ACTIVE);
		}
	}
	
	return SUCCESS;
}

int oamInitPortMacAgingTime(void)
{
	char buf[32] = {0};
	u_int agingTime;

	if (TCAPI_SUCCESS != tcapi_get(OAM_CUC, "MacAgingTime", buf))
		return FAIL;

	agingTime = atoi(buf);
	if (FAIL == macMT7530SetAgeTime(agingTime))
		return FAIL;
	
	return SUCCESS;
}

int oamInitPortMacLimit(void)
{
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_int limitCnt= 0;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "MacLimitCnt", buf))
			continue;

		limitCnt = atoi(buf);
		macMT7530SetPortMacLimit(portId, limitCnt);
	}

	return SUCCESS;
}

int oamInitPortModeCfg(void)
{
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_char mode= 0;
		
	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "PortMode", buf))
			continue;
		
		mode = atoi(buf);
		macMT7530SetPortSpeedMode(portId, mode);
	}

	return SUCCESS;
}

int oamInitVlan(void)
{
#ifdef TCSUPPORT_PON_VLAN
	u_char portId;
	char node[32] = {0};
	char buf[256] = {0};
	u_char VLanMode;
	uint defTag;
	uint newTag;	
	uint oldTag;
	char* ch_p = NULL;
	
	if (eponOamCfg.isHgu)
		return SUCCESS;

	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(node, "VlanMode", buf))
			continue;
		VLanMode = atoi(buf);
		
		if (FAIL == eponClearVlanRules(portId))
			  continue;

		switch(VLanMode){
			case VLAN_MODE_TRANSPARENT:
				if (FAIL == eponSetVlanTransparent(portId))
					return FAIL;
				break;
				
			case VLAN_MODE_TAG:				
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, "DefTag", buf))
					return FAIL;

				sscanf(buf, "%x", &defTag);
		            	if (FAIL == eponSetDefVlanTag(portId, defTag))
		                	return FAIL;

		            	eponBlockAllVlanTag(portId);
						break;
				
			case VLAN_MODE_TRANSLATION:
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, "DefTag", buf))
					return FAIL;
				sscanf(buf, "%x", &defTag);

		            	if (FAIL == eponSetDefVlanTag(portId, defTag))
	               		return FAIL;

				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, "VlanTranslate", buf))
					return FAIL;

				ch_p = buf;
				while(ch_p != NULL && *ch_p != '\0'){
					if(2 != sscanf(ch_p, "%x:%x", &oldTag, &newTag))
						break;

					if (FAIL == eponAddTranslateVlanTag(portId, oldTag, newTag))
                    			return FAIL;
					
					ch_p = strstr(ch_p, ";");
					if (ch_p != NULL) ch_p += 1;
				}

				break;
				
			case VLAN_MODE_TRUNK:
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, "DefTag", buf))
					return FAIL;
				sscanf(buf, "%x", &defTag);

		            	if (FAIL == eponSetDefVlanTag(portId, defTag))
	               		return FAIL;
						
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, "VlanTrunk", buf))
					return FAIL;
				
				ch_p = buf;
				while(ch_p != NULL && *ch_p != '\0'){
					if(1 != sscanf(ch_p, "%x", &oldTag))
						break;
						
					if (FAIL == eponAddTrunkVlanTag(portId, oldTag))
						return FAIL;

					ch_p = strstr(ch_p, ";");
					if (ch_p != NULL) ch_p += 1;
				}

				break;
			case VLAN_MODE_N1AGGREGATION: //not support
			default:
				break;
			
		}
	}
#endif

	return SUCCESS;

}


int oamInitClassificationMarking(void)
{
#ifdef TCSUPPORT_EPON_MAPPING
	// TODO: test
	u_char portId;
	u_char ruleNum, ruleIndex;
	u_char matchNum, matchIndex;
	char node[30] = {0}, attr[30] = {0}, buf[64] = {0};

	QosResult_t result;	
	QosMatchRule_t matchs[MAX_FIELD_NUM];
	QosMatchRule_Ptr pMatch = NULL;
	
	u_char *ptmp;
	u_int tmp;
	char* pch;
	int i;
	
	// nothing to do
	if (eponOamCfg.isHgu)
		return SUCCESS;

	for(portId=0; portId<4; portId++){

		sprintf(node, OAM_CUC_PORT, portId);
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS != tcapi_get(node, "RulesNum", buf))
			continue;
		ruleNum = atoi(buf);
		
		for(ruleIndex=0; ruleIndex<ruleNum; ruleIndex++){			
			memset(&result, 0, sizeof(result));
			memset(matchs, 0, sizeof(matchs));
			
			sprintf(attr, "Rule%d_Precedence", ruleIndex);
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
				break;
			result.precedence = atoi(buf);
			
			sprintf(attr, "Rule%d_EtherPriorityMark", ruleIndex);
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
				break;
			result.priority= atoi(buf);
						
			sprintf(attr, "Rule%d_QueueMapped", ruleIndex);
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
				break;	
			result.queueMapped = atoi(buf);
			
			sprintf(attr, "Rule%d_MatchNum", ruleIndex);
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))				
				break;	
			matchNum = atoi(buf);
			pMatch = &(matchs[0]);
			
			for(matchIndex=0; matchIndex<matchNum; matchIndex++, pMatch++){

								
				sprintf(attr, "Rule%d_Match%d_Field", ruleIndex, matchIndex);
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
					break;
	
				sscanf(buf, "%2x", &tmp);
				pMatch->field = tmp & 0xff;

				sprintf(attr, "Rule%d_Match%d_Content", ruleIndex, matchIndex); 
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
					break;	

				if (pMatch->field < FIELD_IPVER){
			
					switch(pMatch->field){
						case FIELD_SMAC:
						case FIELD_DMAC:
							pch = buf;
							for(i=0; i<6; i++){
								sscanf(pch, "%2x", &tmp);
								pMatch->mac[i] = tmp & 0x00ff;
								pch += 2;
							}
							break;
						case FIELD_PBIT:
						case FIELD_IPPROTO4:
						case FIELD_IPDSCP4:
						case FIELD_IPDSCP6:
							sscanf(buf, "%2x", &tmp);
							pMatch->v8 = tmp & 0x00ff;
							break;						
						case FIELD_VLANID:
						case FIELD_ETHTYPE:
						case FIELD_SPORT:
						case FIELD_DPORT:	
							sscanf(buf, "%4x", &tmp);
							pMatch->v16 = tmp & 0xffff;
							break;						
						case FIELD_SIP4:
						case FIELD_DIP4:
							sscanf(buf, "%8x", &(pMatch->ip4));
							break;
						default:
							break;
					}
				}else{
					switch(matchs[matchIndex].field){
						case FIELD_SIP6:
						case FIELD_DIP6:
						case FIELD_SIP6PREX:
						case FIELD_DIP6PREX:
							
							pch = buf;
							ptmp = (u_char*)(&(pMatch->ip6));
							for(i=0; i<16; i++, ptmp++){
								sscanf(pch, "%2x", &tmp);
								*ptmp = tmp & 0x00ff;
								pch += 2;
							}

							break;
						case FIELD_IPVER:
						case FIELD_IPPROTO6:
							sscanf(buf, "%4x", &tmp);
							pMatch->v16 = tmp & 0xffff;
							break;
						case FIELD_FLOWLABEL6:
							sscanf(buf, "%8x", &(pMatch->ip4));
							break;
						default:
							break;
					}
				}

				sprintf(attr, "Rule%d_Match%d_Operator", ruleIndex, matchIndex);
				memset(buf, 0, sizeof(buf));
				if (TCAPI_SUCCESS != tcapi_get(node, attr, buf))
					break;
				
				sscanf(buf, "%2x", &tmp);
				pMatch->op = tmp & 0x00ff;
			}
			
			if(matchIndex != matchNum)
				return FAIL;
			
			if(FAIL == eponmapInsertClsfyRule(portId, &result, matchNum, matchs))
				return FAIL;
			
		}

		if(ruleIndex != ruleNum)
			return FAIL;
	}
#endif

	return SUCCESS;
}

int oamInitMulticastVlan(void)
{
	
#ifdef TCSUPPORT_XPON_IGMP
	// TODO: save and restore
#endif

	return SUCCESS;
}

int oamInitMulticastTagOper(void)
{
#ifdef TCSUPPORT_XPON_IGMP
	// TODO: save and restore
#endif

	return SUCCESS;
}

int oamInitMulticastSwitch(void)
{
#ifdef TCSUPPORT_XPON_IGMP
	// TODO: save and restore
#endif

	return SUCCESS;
}

int oamInitMulticastControl(void)
{
#ifdef TCSUPPORT_XPON_IGMP
	// TODO: save and restore
#endif
	
	return SUCCESS;
}

int oamInitGroupNumMax(voip)
{
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	u_char maxGroupNum= 0;
		
	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "MCastGrpMaxNum", buf))
			continue;
		
		maxGroupNum = atoi(buf);

		if (FAIL == igmp_set_max_playgroup(portId+1, maxGroupNum))
			return FAIL;			
	}
	
	return SUCCESS;
}

int oamInitLlidQueueConfig(void)
{
#ifdef TCSUPPORT_XPON_IGMP
	// TODO: save and restore
#endif
	
	return SUCCESS;
}

int oamInitMxUMngGlobalParameter(void)
{
	// not support yet	
	return SUCCESS;
}

int oamInitMxUMngSNMPParameter(void)
{
	// not support yet	
	return SUCCESS;
}

int oamInitONUTxPowerSupplyControl(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitGlobalParameterConfig(void)
{
	// TODO: voip param, not really support yet
	return SUCCESS;	
}


int oamInitSIPParameterConfig(void)
{
	// TODO: voip param, not really support yet
	return SUCCESS;	
}

int oamInitSIPUserParameterConfig(void)
{
	// TODO: voip param, not really support yet
	return SUCCESS;		
}

int oamInitFaxModemConfig(void)
{
	// TODO: voip param, not really support yet
	return SUCCESS;		
}

int oamInitAlarmAdminState(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitAlarmThresholds(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitPerformanceStatus(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitPhyAdminControl(void)
{
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	uint portState= 0;
			
	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "PhyAdminControl", buf))
			continue;
			
		portState = atoi(buf) + 1;
		if (portState == ENABLED){
			macMT7530SetPortActive(portId, ACTIVE);
		}else if (portState == DISABLED){
			macMT7530SetPortActive(portId, DISACTIVE);
		}
	}
	
	return SUCCESS;
}


int oamInitAutoNegAdminControl(void)
{
	u_char portId;
	char node[32] = {0};
	char buf[32] = {0};
	uint active= 0;
			
	for(portId=0; portId<4; portId++){
		sprintf(node, OAM_CUC_PORT, portId);
		if (TCAPI_SUCCESS != tcapi_get(node, "AutoNegAdminControl", buf))
			continue;
			
		active = atoi(buf);
		if (active == DISABLED){
			macMT7530PortAutoNegActive(portId, DISACTIVE);
		}else if (active == ENABLED){
			macMT7530PortAutoNegActive(portId, ACTIVE);
		}
	}

	return SUCCESS;
}

int oamInitSleepControl(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitMultiLlidAdminControl(void)
{
	// TODO: save and restore	
	return SUCCESS;
}

int oamInitFastLeaveAdminControl(void)
{
	// TODO: need or not ??
	return SUCCESS;
}

int oamInitSIPDigitMap(void)
{
	// TODO: voip param, not really support yet
	return SUCCESS;
}

/********************************************************
  *
  * eponOamInitParam2
  * restore oam configuration from romfile
  *
  *******************************************************/
int eponOamInitParam2(void){

	OamParamInit_Ptr init_p = cucOamInitTable;
	int num = sizeof(cucOamInitTable) / sizeof(cucOamInitTable[0]);
	int i;
	char buf[8] = {0};

	if(TCAPI_SUCCESS != tcapi_get(OAM_CUC, "CfgCnt", buf))
		return -1;

	eponOamExtDbg(DBG_OAM_L4, "CUC oam init to reset param to system with cfgcnt=%s.\n",buf);

	if(0 == strcmp(buf, "0"))
		return 0;
	
	oamResetSystemParam(0); // need to reset param to default first.
	
	for (i = 0; i < num; ++i, ++init_p){
		if (init_p->oamInitFunc != NULL)
				init_p->oamInitFunc();
	}

	return 0;
}


/********************************************************
  *
  * oamClearRomfileConf
  * clean configuration in romfile
  *
  *******************************************************/
int oamClearRomfileConf(void){

	OamParamInit_Ptr init_p = cucOamInitTable;
	int num = sizeof(cucOamInitTable) / sizeof(cucOamInitTable[0]);
	int i;

	for (i = 0; i < num; ++i, ++init_p){
		if (init_p->oamCleanFunc != NULL)
			init_p->oamCleanFunc();
	}

	return 0;
}

