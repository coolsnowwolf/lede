/*
 ***************************************************************************
 * MediaTeK Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2012, MTK.
 *
 * All rights reserved. MediaTeK's source   code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTeK Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTeK, Inc. is obtained.
 ***************************************************************************

    Module Name:
    oam_sys_api.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    lightman        2012/8/24   Create
*/
#ifndef SYS_API_H_
#define SYS_API_H_
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "epon_oam_types.h"
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ctc_node.h"
#include "libtcapi.h"



#define EPON_ONU         "EPON_ONU"
#define EPON_PON         "EPON_PON"
#define EPON_POWERSAVING "EPON_PowerSaving"
#define EPON_LOIDAUTH    "EPON_LOIDAuth"

#ifdef TCSUPPORT_PMMGR
/* -------------Perforcemance API from Andy------------- */
#define PM_EPON         "PM_EPON"
#define PMMGR_PORT_ID   "portId"
#define PMMGR_DATA_FLAG "dataFlag"
#define PMMGR_EPON_NONE             0   // data flag for get/set none
#define PMMGR_EPON_CYCLE_TIME           PMMGR_EPON_NONE+1           // data flag for get/set cycle time
#define PMMGR_EPON_ENABLE               PMMGR_EPON_CYCLE_TIME+1     // data flag for get/set all pm enable or disable
#define PMMGR_EPON_CURRENT_DATA     PMMGR_EPON_ENABLE+1         // data flag for get/set current data
#define PMMGR_EPON_HISTORY_DATA     PMMGR_EPON_CURRENT_DATA+1   // data flag for get history data
#define PMMGR_EPON_THRESHOLD            PMMGR_EPON_HISTORY_DATA+1   // data flag for get/set alarm's threshold
#define PMMGR_EPON_STATUS               PMMGR_EPON_THRESHOLD+1      // data flag for get/set alarm's status
#define PMMGR_EPON_INIT             (PMMGR_EPON_STATUS+1)           /* data flag for reinit pmmgr */

#define PMMGR_ALARM_ID          "alarmId"
#define PMMGR_ATTR_CYCLE        "cycleTime" // cycle
#define PMMGR_ATTR_STATUS       "status"  // enable/disable
#define PMMGR_ATTR_SET          "set"       // the alarm threshold set
#define PMMGR_ATTR_CLEAR        "clear" // the alarm threshold clear

// portId 
int getPMDataCurrent(u_char portId, PMCounters_t *pm);
int setPMDataCurrent(u_char portId, PMCounters_t *pm);
int getPMDataHistory(u_char portId, PMCounters_t *pm);

int getPMCycleTime(u_char portId);
int setPMCycleTime(u_char portId, u_int time);

int getPMEnable(u_char portId);
int setPMEnable(u_char portId, u_char enable);

/* The portId: 0~3: ether port; 4~5: pon if */
int reinitEponPmmgr();

// @return: -1:fail;  0: disable;  1:enable
int getPortAlarmStatus(u_char portId, u_int alarmId);

// @return: -1:fail; 0: success
// # enable: 0: disable; 1: enable
int  setPortAlarmStatus(u_char portId, u_int alarmId, u_char enable);

// @return: -1: fail;  0: success
// # set: threshold cause alarm; clear: threshold cause clear
int getPortAlarmThreshold(u_char portId, u_int alarmId, u_int *set, u_int *clear);
int setPortAlarmThreshold(u_char portId, u_int alarmId, u_int set, u_int clear);


#endif


#define EPON_SERVICESLA "EPON_ServiceSLA"

#ifdef TCSUPPORT_VOIP
    #define VOIPBASIC               "VoIPBasic"
    #define VOIPBASIC_COMMON        "VoIPBasic_Common"
    #define VOIPADVANCED            "VoIPAdvanced"
    #define VOIPADVANCED_COMMON     "VoIPAdvanced_Common"
    #define VOIPADVANCED_ENTRY      "VoIPAdvanced_Entry%d"
    #define VOIPBASIC_ENTRY         "VoIPBasic_Entry%d"
    #define VOIPMEDIA_COMMON        "VoIPMedia_Common"
    #define VOIPH248_COMMON         "VoIPH248_Common"
    #define VOIP_DIGIT_MAP_ENTRY    "VoIPDigitMap_Entry"
    #define VOIP_INFO_ENTRY         "InfoVoIP_Entry%d"
#endif

#define WANINFO_COMMON      "WanInfo_Common"
#define WANINFO_WANIF       "WanInfo_WanIF"
#define WANINFO_WANPVC      "WanInfo_WanPVC%d"

#define ETH_PORT_STATUS "/proc/tc3162/eth_port_status"

#define DEFAULT_PON_VENDORID  1206
#define DEFAULT_PON_MODEL     0614
#define DEFAULT_PON_REVISION  02


int getPortNum();



/*
 * Switch APIs
 */
#ifndef TCSUPPORT_MT7530_SWITCH_API 
int macMT7530GetPortActive(u_char portId);
int macMT7530GetPortPause(u_char portId);
int macMT7530SetPortPause(u_char portId, u_char state);

int macMT7530GetPortPolicingEnable(u_char portId);
int macMT7530SetPortPolicingEnable(u_char portId, u_char state);
int macMT7530GetPortPolicing(u_char portId, u_int *cir, u_int *cbs, u_int *ebs);
int macMT7530SetPortPolicing(u_char portId, u_int cir, u_int cbs, u_int ebs);

int macMT7530GetPortRatelimitingEnable(u_char portId);
int macMT7530SetPortRatelimitingEnable(u_char portId, u_char state);
int macMT7530GetPortRatelimiting(u_char portId, u_int *cir, u_int *pir);
int macMT7530SetPortRatelimiting(u_char portId, u_int cir, u_int pir);

int macMT7530SetPortDisableLooped(u_char portId, u_char enabled);
int macMT7530PortAutoNegRestartAutoConfig(u_char portId);
int macMT7530PortAutoNegActive(u_char portId, u_char enable);


int macMT7530GetEthPortDSRateLimiting(u_char portId, u_char *enable, u_int *cir, u_int *pir);
int macMT7530SetEthPortDSRateLimiting(u_char portId, u_char enable, u_int cir, u_int pir);

// open or close the ether port
int macMT7530SetPortActive(u_char portId, u_char enable);
int macMT7530SetPortLoopDetect(u_char portId, u_char enable);
int macMT7530GetAutoNegAdminState(u_char portId);

#endif
int macMT7530GetPortEthLinkState(u_char portId);





u_char ONUGetActiveIfTypeNum();
u_short ONUGetActiveIfPortNum(u_char ifType);

/*---------- VOIP FUNCTIONS ----------*/

int ONUIadOperate(u_char iadOperation);





/*-------- MultiCast FUNCTIONS --------*/
#ifndef TCSUPPORT_XPON_IGMP
int igmp_get_portvlan_cnt(u_char portId);
int igmp_get_portvlan_id(u_char portid, int index, int *vid1, int *vid2);


int igmp_add_portvlan(u_char portId, u_short mcVid);
int igmp_del_portvlan(u_char portId, u_short mcVid);

int igmp_get_tagstrip(u_char port);
int igmp_set_tagstrip(u_char portId, u_char tagType);


int igmp_get_fwdmode(void);
int igmp_set_fwdmode(u_char mcType);
int igmp_get_fwdentry_cnt();
//int igmp_get_fwdentry(int idx, u_char *mcType, u_char *port, u_short *vid, u_char *mac, u_char *ip);
int igmp_get_fwdentry(int idx,int* type,int* port,int* vid, unsigned char* grp_addr,unsigned char* src_ip,unsigned char* client_ip,int* join_time_sec);

int igmp_clear_fwdentry();
int igmp_add_fwdentry(int type, u_char portId,u_short mcVid,u_char *gdaMac,  u_char* ip);
int igmp_del_fwdentry(int type, u_char portId,u_short mcVid,u_char *gdaMac, u_char* ip);


int igmp_get_max_playgroup(u_char portId);
int igmp_set_max_playgroup(u_char portId, u_char num);
int igmp_get_fastleave_ctc();
int igmp_set_fastleave_ctc(int stat);
int igmp_set_portvlan_switchid(u_char portId, u_short mcVid, u_short iptvVid);
int igmp_get_portvlan_switchcnt(int port);
int igmp_get_portvlan_switchid(int port,int idx,int* pvid,int* pnewid);
int igmp_clear_portvlan(int port);

#endif

#endif
