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
	oam_ctc_dispatch.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/

#ifndef OAM_CTC_DISPATCH_H_
#define OAM_CTC_DISPATCH_H_
#include <semaphore.h>
#include "ctc/oam_ext_cmn.h"
#include "ctc/oam_ext_mgr.h"
#include "oam_ctc_node.h"


/* OAM CTC EXT SPEC OPCODE DEFINE */
#define EXT_OPCODE_RESERVED           0x00
#define EXT_OPCODE_GET_REQUEST        0x01
#define EXT_OPCODE_GET_RESPONSE       0x02
#define EXT_OPCODE_SET_REQUEST        0x03
#define EXT_OPCODE_SET_RESPONSE       0x04
#define EXT_OPCODE_ONU_AUTHENTICATION 0x05
#define EXT_OPCODE_SOFTWARE_DOWNLOAD  0x06
#define EXT_OPCODE_CHURNING           0x09
#define EXT_OPCODE_DBA                0x0A
#define EXT_OPCODE_SLEEP_CONTROL      0xFE
#define EXT_OPCODE_EXT_EVENT          0xFF


int initCtcHandler(IN ExtHandler_Ptr pExtHdlr);

int ctcProcInfoData(u_char llid, u_char *pOutData, int *outLen, u_char *pInData, int inLen);
int ctcBuildEventData(Buff_Ptr bfp, u_char *pData, int length);
int ctcProcExtData(OUT Buff_Ptr bfp, IN Buff_Ptr obfp);

int rcvOamExtVar(OUT Buff_Ptr bfp, IN u_char opCode, IN Buff_Ptr obfp);




/***************************************************
 * Remove depend on cfg_manager to improve reply speed.
 * Use DB to manage the info we need, and get info only at init.
 ***************************************************/
typedef struct{
	u_char desired_type;
	char   state; // 0:init; 1:success; -1:fail
	char loid[24];
	char passwd[12];
}OamCtcDBAuth_t;

typedef struct{
	u_char 	valid0:1,
			valid1:1,
			active:2,
			main:2;
	#if 0
	char version0[256];
	char version1[256];
	#endif
}OamCtcDBSoftwareUpgrad_t;

typedef struct {
	u_short qidx;
	u_short weight;
}OamCtcDBQW_t;

typedef struct{
	u_char num;
	OamCtcDBQW_t qcfg[8]; // max support 8
}OamCtcDBLQCfg_t;

typedef struct{
	u_char fec_en;
	
	OamCtcDBAuth_t auth;
	
	OamCtcDBLQCfg_t llid_queue_cfg;
}OamCtcDBLlid_t;

typedef struct{
	u_char queue_id;
	u_short fixed_pkt_size;
	u_short fixed_bw;
	u_short assured_bw;
	u_short best_bw;
	u_char  wrr_wight;
}OamCtcDBSlaQue_t;

typedef struct{
	u_char enable;
	u_char sch_scheme;
	u_char high_pri_bound;
	u_int  cycle_len;
	u_char srvs_num;
	OamCtcDBSlaQue_t srvs[8];// max entr 8
}OamCtcDBSla_t;

#define OAM_VLAN_RULE_MAX_NUM  8
typedef struct {
	u_char mode; // default is transparent
	u_char num;
	u_int   def_tag;
	u_int   old_tags[OAM_VLAN_RULE_MAX_NUM];
	u_int   new_tags[OAM_VLAN_RULE_MAX_NUM];
}OamCtcDBPortVlan_t;

typedef struct {
	u_char  tag_strip;
	u_char  max_grp_num;
	u_char  trans_num;
	u_char  trunk_num;
	
	u_short mcast_vid[OAM_VLAN_RULE_MAX_NUM];
	u_short user_vid[OAM_VLAN_RULE_MAX_NUM];

	u_short trunk_vid[OAM_VLAN_RULE_MAX_NUM];
}OamCtcDBPortMCast_t;

typedef struct {
	u_char  type;
	u_char  fast_leave;
}OamCtcDBMCast_t;

/* Manage LAN Port Config/Info */
typedef struct{	
	u_char phy_ctrl_en:1;
	u_char phy_linkup:1;
	u_char linkup:1;
	u_char auto_neg_en:1;
	u_char flow_ctrl_en:1;
	u_char loopdetect:1;
	u_char disable_looped:1;
	u_char us_policing_en:1;
	u_char ds_ratelimit_en:1;
	u_char port_mode;
	u_int  us_cir, us_cbs, us_ebs;
	u_int  ds_cir, ds_pir;

	u_int mac_limit_cnt;


	OamCtcDBPortVlan_t vlan;
	OamCtcDBPortMCast_t mcast;
}OamCtcDBPortInfo_t;

#ifdef TCSUPPORT_VOIP
typedef enum {
	VOIP_NODE_INVALID, 
	VOIP_NODE_VALID
}VoIP_Node_Flag_e;

typedef struct {
	/* VOIP_PORT */
	u_char  enable;

    VoIP_Node_Flag_e potsStatusFlag;
	OamPOTSStatus_t potsStatus;

	/* SIP_PARAM_CONFIG */
	u_char	heartbeatSwitch;
	u_short   mgPort;
	u_int	sipProxyServIp;
	u_int	backupSipProxyServIp;
	u_int	activeSipProxyServIp;
	u_int	sipRegServIp;
	u_int	backupSipRegServIp;
	u_int	outBoundServIp;
	u_int	sipRegInterval;
	u_short	sipProxyServPort;
	u_short	backupSipProxyServPort;
	u_short	sipRegServPort;
	u_short	backupSipRegServPort;
	u_short	outBoundServPort;
	u_short	heartbeatCycle;
	u_short	heartbeatCount;

	/* SIP_USR_PARAM_CONFIG */
	char	userAccount[16];
	char	userName[32];
	char	userPassword[16];

	/* H248_USR_TID_INFO */
	u_char UserTIDName[32];

	/* H248_RTP_TID_INFO */
	u_char NumOfRTPTID;
	u_char RTPTIDPrefix[16];
	u_char RTPTIDDigitBegin[8];
	u_char RTPTIDMode;
	u_char RTPTIDDigitLen;
	
}OamCtcDBVoipPortConf_t;

typedef struct {
	u_char  proto_spt;
	char     iad_swver[32];
	char     iad_swtime[32];	
}OamCtcDBVoipInfo_t;


#define SIP_DIGITMAP_LEN_MAX  512
#define OAM_VOIP_PORT_NUM_MAX  2

typedef struct {
    VoIP_Node_Flag_e glbParamFlag;
    OamGlobalParamConfig_t glbParam;
    OamGlobalParamConfig_t glbParamCfgSet;

	/* FAX_MODEM_CONFIG */
	u_char t38enable;
	u_char control;

	/* H248_PARAM_CONFIG */
	u_short MGPortNo;
	u_short MgcComPortNo;
	u_short BackupMgcComPortNo;
	u_short HeartbeatCycle;
	u_int  MGCIP;
	u_int   BackupMgcIp;
	u_char ActiveMGC;
	u_char RegMode;
	u_char MID[64];
	u_char HeartbeatMode;
	u_char HeartbeatCount;

	/* SIP_DIGIT_MAP */
	char  digitmap[SIP_DIGITMAP_LEN_MAX];

	/* PORT_CONFIG */
	OamCtcDBVoipPortConf_t port[OAM_VOIP_PORT_NUM_MAX];
}OamCtcDBVoipConf_t;
#endif

/* These infos are not changed after init them. */
typedef struct{
	u_char lan_num;
	u_char ge_num;
	u_char fe_num;
	u_char wlan_num; // SSID
	u_char adsl_num;
	u_char vdsl_num;
	u_char voip_num;
	u_char usb_num;
	u_char tdm_num;
	u_char us_queue_num;
	u_char us_queue_max_per_port;
	u_char ds_queue_num;
	u_char ds_queue_max_per_port;
	u_char battery_backup;
	
	u_char fec_ability; // default 2 (Support FEC)
	u_char max_llid_num;
	u_char service_sla_num;
	u_char protect_type; // default 0=NOT_SUPPORT
	u_char pon_if_num; // default 1
	
	u_char ipv6_spt;
	u_char power_ctrl_type; // NOT_SPT/TX/TRX
	u_char powersaving_cap;
	u_char powersaving_early_wakeup_spt;
	
	/* device info for EPON ONU */
	//u_char onu_type; // hgu/sfu
	u_char vendor_id[4];
	u_char model[4];
	u_char hw_ver[8];
	u_char sw_ver[16];
	u_char ext_model[16];
	char   fw_ver[127]; // dynmic ?
	
	/* Chipset ID info */
	u_short pon_vendor_id;
	u_short pon_chip_model;
	u_char  pon_revision;
	u_char  pon_ver_date[3];
#ifdef TCSUPPORT_VOIP
	OamCtcDBVoipInfo_t voip; /* VOIP info */
#endif
}OamCtcDBDevInfo_t;

/* These infos would be changed when running */
typedef struct{
	u_char cur_llid_en_num; // default is 1
	
	u_char powersaving_early_wakeup_en;
	long long powersaving_max_duration;
	u_int  port_aging_time;

	u_short pon_los_optical_time; // default 2ms
	u_short pon_los_mac_time; // default 55ms
	
	u_char  pon_holdover_state; 
	u_int   pon_holdover_time; // default 50ms
	
	OamCtcDBSla_t sla;
	OamCtcDBMCast_t mcast;
#ifdef TCSUPPORT_VOIP
	OamCtcDBVoipConf_t voip;
#endif
}OamCtcDBConf_t;

#define isMT7520S(x)  ((x)==1)
#define isMT7520F(x)  ((x)==2)
#define isMT7520G(x)  ((x)==3)
#define isMT7525F(x)  ((x)==4)
#define isMT7525G(x)  ((x)==5)
#define isEN7521(x)  ((x)==6)


/************************************************************
 * Now, general info is direct use api set to system, so no need use DATA_SYNC.
 * If when you add new node which use tcapi functions, you should use 
 * DATA_SYNC mechanism to save to cfg_manager
 ************************************************************/
#define DB_SYNC_FLAG_UPDATA_INFO          (0xFFFFFFFF)
#define DB_SYNC_FLAG_UPDATA_AUTH	        (1<<0)
#if 1 //def TCSUPPORT_VOIP
#define DB_SYNC_PORT_VOIP_0    (1<<4)
#define DB_SYNC_PORT_VOIP_1    (1<<5)

#define DB_SYNC_SAVE_CONFIG    (1<<0)

#define DB_SYNC_LOOP_DETECT    (1<<6)
#define DB_SYNC_RMS_CONFIG1    (1<<7)
#define DB_SYNC_VOIPBASIC_COMMIT_CONFIG    (1<<8)
#define DB_SYNC_VOIPADV_COMMIT_CONFIG    (1<<9)
#define DB_SYNC_VOIPH248_COMMON_COMMIT_CONFIG    (1<<10)
#define DB_SYNC_VOIPDIGIT_MAP_ENTRY_COMMIT_CONFIG    (1<<11)
#define DB_SYNC_RMS_ACSURL_CONFIG    (1<<12)
#define DB_SYNC_CUC_PINT_TEST_CONFIG    (1<<13)
#define DB_SYNC_VIOP_PORT_CONFIG    (1<<14)
#define DB_SYNC_SIP_PARAMETER_CONFIG    (1<<15)
#define DB_SYNC_SIP_USER_PARAMETER_CONFIG    (1<<16)
#define DB_SYNC_FAX_MODEM_CONFIG    (1<<17)
#define DB_SYNC_H248_PARAMETER_CONFIG    (1<<18)
#define DB_SYNC_H248_USER_TID_CONFIG    (1<<19)
#define DB_SYNC_H248_RTP_TID_CONFIG    (1<<20)
#define DB_SYNC_SET_TIMER_CONFIG    (1<<21)
#define DB_SYNC_SIP_DIGITMAP_CONFIG    (1<<22)
#define DB_SYNC_CTC_PORT_LOOPED_CONFIG    (1<<23)
#define DB_SYNC_DISABLE_LOOPED_CONFIG    (1<<24)


/* VOIP_OFFSET should ensure the max OFFSET less than 32 */
#define VOIP_OFFSET                       16
#define DB_SYNC_FLAG_VOIP_PORT   				(1<<VOIP_OFFSET)
#define DB_SYNC_FLAG_VOIP_GLB_PARAM_CFG         (1<<(VOIP_OFFSET+1))
#define DB_SYNC_FLAG_VOIP_SIP_PARMA_CFG         (1<<(VOIP_OFFSET+2))
#define DB_SYNC_FLAG_VOIP_SIP_USR_PARAM_CFG     (1<<(VOIP_OFFSET+3))
#define DB_SYNC_FLAG_VOIP_FAX_MODEM_CFG         (1<<(VOIP_OFFSET+4))
#define DB_SYNC_FLAG_VOIP_IAD_OPERA		        (1<<(VOIP_OFFSET+5))
#define DB_SYNC_FLAG_VOIP_SIP_DIGIT_MAP         (1<<(VOIP_OFFSET+6))
#define DB_SYNC_FLAG_VOIP_H248_PARAM_CFG        (1<<(VOIP_OFFSET+7))
#define DB_SYNC_FLAG_VOIP_H248_USR_TID          (1<<(VOIP_OFFSET+8))
#define DB_SYNC_FLAG_VOIP_H248_RTP_TID          (1<<(VOIP_OFFSET+9))
#endif // TCSUPPORT_VOIP

typedef struct {
	u_char cpu_type;
	u_char debug_level;
	u_int  oui;

	u_int config_counter;
	OamCtcDBDevInfo_t dev_info;
	OamCtcDBConf_t    cfg_info;
	OamCtcDBSoftwareUpgrad_t swup;
	OamCtcDBLlid_t llid[EPON_LLID_MAX_NUM];
	OamCtcDBPortInfo_t port[4]; // max port is 4

	u_int  db_sync_port_mask : 8;
	u_int  db_sync_llid: 4;
	u_int  db_sync_sflag; // for set
	u_int  db_sync_gflag; // for get

	sem_t db_sync_sem;
	pthread_t db_sync_thread;
	
}OamCtcDB_t, *OamCtcDB_ptr;

typedef struct 
{
	u_int id;
	u_int fe_num;
	u_int ge_num;
	u_int voip_num;
	u_int wlan_num;
	u_int usb_num;
}DeviceInformation_t;


typedef enum {
	MT7520S = 1,
	MT7520,
	MT7520G,
	MT7525,
	MT7525G,
	EN7521S,
	EN7521F,
	EN7526F,
	EN7526D,
	EN7526G,
	EN751221,
} chipId_t ;



int oam_ctc_db_init(OamCtcDB_t *pDB);
int oam_ctc_db_sync_init(void);
void *oam_ctc_db_sync_task(void);

int oam_ctc_cmd_update_conf(void);
int oam_ctc_db_update_conf(OamCtcDB_t *pDB, u_int mask);

int oam_cuc_cmd_db_save();
int oam_cuc_cmd_db_load();

int oam_ctc_db_voip_conf_set(OamCtcDB_t *pDB, u_int mask, int port);

void postDBSyncMsg(u_int msg);
#endif
