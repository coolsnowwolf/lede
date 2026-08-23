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
	oam_sys_api.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	lightman		2012/8/24	Create
*/
#include <unistd.h>
#include <sys/types.h>
#include <memory.h>
#include "ctc/oam_sys_api.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_ctc_dspch.h"

extern OamCtcDB_t gCtcDB;

inline int getPortNum()
{
    return gCtcDB.dev_info.lan_num;
}


/* -------------------Switch Port Functions list------------------- */
#ifndef TCSUPPORT_MT7530_SWITCH_API 

int macMT7530GetPortPolicingEnable(u_char portId)
{	
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return 0\r\n", __FUNCTION__, portId);
    return DISACTIVE;
}
int macMT7530SetPortPolicingEnable(u_char portId, u_char state)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, state = %u), return 0\r\n", __FUNCTION__, portId, state);
    return SUCCESS;
}
int macMT7530GetPortRatelimitingEnable(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return 0\r\n", __FUNCTION__, portId);
    return ACTIVE;
}
int macMT7530SetPortRatelimitingEnable(u_char portId, u_char state)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, state = %u), return 0\r\n", __FUNCTION__, portId, state);
    return SUCCESS;
}
int macMT7530PortAutoNegRestartAutoConfig(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return 0\r\n", __FUNCTION__, portId);
    return SUCCESS;
}
int macMT7530PortAutoNegActive(u_char portId, u_char enable)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, enable = %u), return 0\r\n", __FUNCTION__, portId, enable);
    return SUCCESS;
}
int macMT7530SetPortActive(u_char portId, u_char enable)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, enable = %u), return 0\r\n", __FUNCTION__, portId, enable);
    return SUCCESS;
}
int macMT7530SetPortLoopDetect(u_char portId, u_char enable)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, enable = %u), return 0\r\n", __FUNCTION__, portId, enable);
    return SUCCESS;
}

int macMT7530GetPortActive(u_char portId)
{    
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return 1\r\n", __FUNCTION__, portId);
    return DISABLED;
}

int macMT7530SetPortDisableLooped(u_char portId, u_char enabled)
{
    char node[10] = {0}, buf[10] = {0};

    if (enabled == 1){
        strcpy(buf, "Yes");
    }else{
        strcpy(buf, "No");
    }
    sprintf(node, "SwitchPara_Entry%d", portId);

    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, enabled = %u), return ?\r\n", __FUNCTION__, portId, enabled);
    if (0 == tcapi_set(node, "DisableLooped", buf)){
        return SUCCESS;
    }
    return SUCCESS;
}

int macMT7530GetPortPause(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return %d\r\n", __FUNCTION__, portId, DISACTIVE);
    return DISACTIVE;
}
int macMT7530SetPortPause(u_char portId, u_char state)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, state = %u), return 0\r\n", __FUNCTION__, portId, state);
    return SUCCESS;
}

int macMT7530GetPortPolicing(u_char portId, u_int *cir, u_int *cbs, u_int *ebs)
{
	*cir = 2000 + portId*50;
	*cbs = 3000 + portId*50;
	*ebs = 1024 + portId*50;
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, cir = %u, cbs = %u, ebs = %u), return SUCCESS\r\n", __FUNCTION__, portId, *cir, *cbs, *ebs);
    return SUCCESS;
}
int macMT7530SetPortPolicing(u_char portId, u_int cir, u_int cbs, u_int ebs)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, cir = %u, cbs = %u, ebs = %u), return SUCCESS\r\n", __FUNCTION__, portId, cir, cbs, ebs);
    return SUCCESS;
}
int macMT7530GetPortRatelimiting( u_char portId, u_int *cir, u_int *pir )
{
	*cir = 2000 + portId*50;
	*pir = 3000 + portId*50;
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, cir = %u, pir = %u), return SUCCESS\r\n", __FUNCTION__, portId, *cir, *pir);
    return SUCCESS;
}
int macMT7530SetPortRatelimiting( u_char portId, u_int cir, u_int pir )
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, cir = %u, pir = %u), return SUCCESS\r\n", __FUNCTION__, portId, cir, pir);
    return SUCCESS;
}

int macMT7530GetPortAutoNegState(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return 1\r\n", __FUNCTION__, portId);
    return DISABLED;
}

#endif

int macMT7530GetPortEthLinkState(u_char portId)
{
    char result[20] = {0};
    int  port[4] = {0};
    int  portNum;
    FILE *fp = NULL;

    //& if (access(ETH_PORT_STATUS, F_OK) != 0)
    //&     return FAIL;

    fp = fopen(ETH_PORT_STATUS, "r+");
    if (!fp)
        return FAIL;

    fgets(result, sizeof(result), fp);
    fclose(fp);

    portNum = sscanf(result, "%d %d %d %d", port, port+1, port+2, port+3);
    if (portId > portNum -1){
        eponOamExtDbg(DBG_OAM_L1, "Port Id bigger than real port num!\r\n");
        return FAIL;
    }
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u), return %d\r\n", __FUNCTION__, portId, port[portId]);

    return port[portId]; // 0=down/1=up
}

/* -------------------ONU Functions list------------------- */


//@TODO: need update with the running system
inline u_short ONUGetActiveIfPortNum(u_char ifType)
{
    u_short num = 0;
	char buf[10] = {0};
	
    switch(ifType){
        case IF_TYPE_GE:
		num = gCtcDB.dev_info.ge_num;
        break;
        case IF_TYPE_FE:
		num = gCtcDB.dev_info.fe_num;
        break;
        case IF_TYPE_WLAN:
		num = gCtcDB.dev_info.wlan_num;
        break;
        case IF_TYPE_ADSL2:
		num = gCtcDB.dev_info.adsl_num;
        break;
        case IF_TYPE_VDSL2:
		num = gCtcDB.dev_info.vdsl_num;
        break;
        case IF_TYPE_VOIP:
		num = gCtcDB.dev_info.voip_num;
        break;
        case IF_TYPE_USB:
		num = gCtcDB.dev_info.usb_num;
        break;
        case IF_TYPE_TDM:
		num = gCtcDB.dev_info.tdm_num;
        break;
   }

    return num;
}
inline u_char ONUGetActiveIfTypeNum()
{
    /* support GE/FE/VOIP/WLAN/USB */
    u_char ifNum = 0;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_GE))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_FE))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_VOIP))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_USB))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_WLAN))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_ADSL2))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_VDSL2))
        ifNum++;
    if (0 < ONUGetActiveIfPortNum(IF_TYPE_TDM))
        ifNum++;

    eponOamExtDbg(DBG_OAM_L3, "%s return %d\r\n", __FUNCTION__, ifNum);
    return ifNum;
}




/* -----------------Multicast Functions------------------- */
#ifndef TCSUPPORT_XPON_IGMP
int igmp_get_portvlan_cnt(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u) return 0x06\r\n", __FUNCTION__, portId);
    return 6;
}

int igmp_get_portvlan_id(u_char portid, int index, int *vid1, int *vid2)
{
	(*vid1) = (index+20);
	*vid2 = (*vid1)*2;
	eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, index = %d, vid1 = %d vid2=%d) return SUCCESS\r\n", __FUNCTION__, portid, index, (*vid1), *vid2);
	return SUCCESS;
}

 int igmp_add_portvlan(u_char portId, u_short mcVid)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, mcVid = %u) return SUCCESS\r\n", __FUNCTION__, portId, mcVid);
    return SUCCESS;
}
 int igmp_del_portvlan(u_char portId, u_short mcVid)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, mcVid = %u) return SUCCESS\r\n", __FUNCTION__, portId, mcVid);
    return SUCCESS;
}

int igmp_set_fwdmode(u_char mcType)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (mcType = %u) return SUCCESS\r\n", __FUNCTION__, mcType);
    return SUCCESS;    
}
int igmp_get_fwdmode(void)
{
    eponOamExtDbg(DBG_OAM_L3, "%s return 0x01\r\n", __FUNCTION__);
    return 0x01;   
}
int igmp_get_tagstrip(u_char port)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u) return SUCCESS\r\n", __FUNCTION__, port);
	return 2;
}
int igmp_set_tagstrip(u_char portId, u_char tagType)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, tagType = %u) return SUCCESS\r\n", __FUNCTION__, portId, tagType);
	return SUCCESS;
}
int igmp_get_fwdentry_cnt()
{
	eponOamExtDbg(DBG_OAM_L3, "%s return 4\r\n", __FUNCTION__);
	return 4;
}
int igmp_get_fwdentry(int idx,int* mcType,int* port,int* vid, unsigned char* grp_addr,unsigned char* src_ip,unsigned char* client_ip,int* join_time_sec)
//int igmp_get_fwdentry(int idx, u_char *mcType, u_char *port, u_short *vid, u_char *mac, u_char *ip)
{
	*mcType = 0; 
	*port = idx % 4;
	*vid = (idx+30);
	eponOamExtDbg(DBG_OAM_L3, "%s return 2\r\n", __FUNCTION__);
	return -1;
}
int igmp_clear_fwdentry()
{
	eponOamExtDbg(DBG_OAM_L3, "%s return SUCCESS\r\n", __FUNCTION__);
	return SUCCESS;
}
int igmp_add_fwdentry(int type, u_char portId,u_short mcVid,u_char *gdaMac, u_char* ip)
{
	return SUCCESS;
}
int igmp_del_fwdentry(int type, u_char portId,u_short mcVid,u_char *gdaMac,  u_char* ip)
{
	return SUCCESS;
}
int igmp_get_max_playgroup(u_char portId)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u) return 8\r\n", __FUNCTION__, portId);
    return 8;
}
int igmp_set_max_playgroup(u_char portId, u_char num)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, num = %u) return SUCCESS\r\n", __FUNCTION__, portId, num);
    return SUCCESS;    
}
int igmp_get_fastleave_ctc()
{
    u_char mode = 0;
    //& mode = igmpONUGetFastLeave()+1;
    eponOamExtDbg(DBG_OAM_L3, "%s return 0x%x\r\n", __FUNCTION__, mode);
    return mode;// DISABLED
}
 int igmp_set_fastleave_ctc(int state)
{
    //& igmpONUSetFastLeave(state);
    eponOamExtDbg(DBG_OAM_L3, "%s (state = %u) return SUCCESS\r\n", __FUNCTION__, state);
    return SUCCESS;
}

 int igmp_set_portvlan_switchid(u_char portId, u_short mcVid, u_short iptvVid)
{
	eponOamExtDbg(DBG_OAM_L3, "%s (portId = %u, mcVid = %u, iptvVid = %u) return SUCCESS\r\n", __FUNCTION__, portId, mcVid, iptvVid);
	return SUCCESS;    
}
 
int igmp_get_portvlan_switchcnt(int port)
{
	return 0;
}
int igmp_get_portvlan_switchid(int port,int idx,int* pvid,int* pnewid)
{
	return -1;
}
int igmp_clear_portvlan(int port)
{
	return -1;
}

#endif


inline int ONUIadOperate(u_char iadOperation)
{
    eponOamExtDbg(DBG_OAM_L3, "%s (iadOperation = %d) ", __FUNCTION__, iadOperation);
    switch(iadOperation){
        case 0x00: // reregister
        break;
        case 0x01: // logout
        break;
        case 0x02: // reset
        break;
        default:
        eponOamExtDbg(DBG_OAM_L3, "Error!\n");
		break;
    }
    return SUCCESS;
}


/* -------------------Performance API------------------- */
#ifdef TCSUPPORT_PMMGR
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include "../../../inc/pmmgr/pmmgr_api.h"
extern int gPmmgMq;
int pmmgr_msg_api(Pmmgr_msg_t *msg, int flag)
{
	int len = 0, rlen = sizeof(Pmmgr_msg_t)-sizeof(long);
	msg->msg_type = PMMGR_MSG_SND;
	if (0 != msgsnd(gPmmgMq, msg, rlen, 0)){
		return -1;
	}
    
	if (flag != 0){
		memset(msg, 0, sizeof(Pmmgr_msg_t));
		if ((len = msgrcv(gPmmgMq, msg, rlen, PMMGR_MSG_RCV, 0)) < rlen){
            tcdbg_printf("%s recv msg from msg queue %d failed!\n", __FUNCTION__, gPmmgMq);
			return -1;
		}
	}
	
	return 0;
}


int getPMDataCurrent(u_char portId, PMCounters_t *pm)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_PMDATA;
	msg.port_id = portId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	memcpy(pm, &msg.pm, sizeof(PMCounters_t));

	return SUCCESS;
}
int setPMDataCurrent(u_char portId, PMCounters_t *pm)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_SET_PMDATA;
	msg.port_id = portId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return SUCCESS;

}
int getPMDataHistory(u_char portId, PMCounters_t *pm)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_PMHSTYDATA;
	msg.port_id = portId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	memcpy(pm, &msg.pm, sizeof(PMCounters_t));

	return SUCCESS;
}

int getPMCycleTime(u_char portId)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_CYCLE_TIME;
	msg.port_id = portId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return msg.cycle_time;
}
int setPMCycleTime(u_char portId, u_int time)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_SET_CYCLE_TIME;
	msg.port_id = portId;
	msg.cycle_time = time;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return SUCCESS;
}

int getPMEnable(u_char portId)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_ENABLE;
	msg.port_id = portId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return msg.port_state;
}
int setPMEnable(u_char portId, u_char enable)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_SET_ENABLE;
	msg.port_id = portId;
	msg.port_state = enable;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;
	
    	return SUCCESS;
}

int reinitEponPmmgr(){
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_INIT_EPON;
	
	if (0 != pmmgr_msg_api(&msg, 0))
		return FAIL;

    return SUCCESS;
}

/* The portId: 0~3: ether port; 4~5: pon if */

// @return: -1:fail;  0: disable;  1:enable
int getPortAlarmStatus(u_char portId, u_int alarmId)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_STATE;
	msg.port_id = portId;
	msg.alarm_id = alarmId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return msg.alarm_state;
}

// @return: -1:fail; 0: success
// # enable: 0: disable; 1: enable
int  setPortAlarmStatus(u_char portId, u_int alarmId, u_char enable)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_SET_STATE;
	msg.port_id = portId;
	msg.alarm_id = alarmId;
	msg.alarm_state = enable;
    
	if (0 != pmmgr_msg_api(&msg, 1)){
		return FAIL;
	}
	return SUCCESS;
}

// @return: -1: fail;  0: success
// # set: threshold cause alarm; clear: threshold cause clear
int getPortAlarmThreshold(u_char portId, u_int alarmId, u_int *set, u_int *clear)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_GET_THRSHLD;
	msg.port_id = portId;
	msg.alarm_id = alarmId;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	*set = msg.alarm_set_threshold;
	*clear = msg.alarm_clear_threshold;
	return SUCCESS;
}
int setPortAlarmThreshold(u_char portId, u_int alarmId, u_int set, u_int clear)
{
	Pmmgr_msg_t msg = {0};

	msg.cmd_type = PMMGR_CT_SET_THRSHLD;
	msg.port_id = portId;
	msg.alarm_id = alarmId;
	msg.alarm_set_threshold = set;
	msg.alarm_clear_threshold = clear;

	if (0 != pmmgr_msg_api(&msg, 1))
		return FAIL;

	return SUCCESS;
}
#endif





