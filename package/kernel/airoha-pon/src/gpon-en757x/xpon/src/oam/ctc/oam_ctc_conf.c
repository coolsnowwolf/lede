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
	oam_cuc_conf.c

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
	Lightman		2012/7/5		Create
*/
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "epon_oam.h"
#include "ctc/oam_ctc_dspch.h"
#include "ctc/oam_ctc_var.h"
#include "ctc/oam_ctc_node.h"
#include "ctc/oam_ctc_event.h"
#include "ctc/oam_ctc_auth.h"
#include "ctc/oam_ctc_swup.h"
#include "ctc/oam_ctc_alarm.h"
#include "ctc/oam_ext_mgr.h"
#include "ctc/oam_sys_api.h"
#include "epon_oam.h"
#include "api/mgr_api.h"

#ifdef TCSUPPORT_EPON_OAM_CUC
    #include "ctc/oam_cuc_init.h"
#endif // TCSUPPORT_EPON_OAM_CUC

#include "libtcapi.h"
#include "ctc/oam_ctc_node.h"
#include "libepon.h"

#ifdef TCSUPPORT_EPON_MAPPING
    #include "libeponmap.h"
#endif

#ifdef TCSUPPORT_VOIP
#include "ctc/oam_ctc_voip_conf.h"
#endif

extern int eponOamCtcDbgLvl;
extern eponOamCfg_t eponOamCfg;
extern u_char gOuiCtc[3];
extern OamCtcDB_t gCtcDB;

#define dprintf  tcdbg_printf
/*
 * Init sys param DB for CTC Used only once at boot.
 *
 */

static inline int tcapi_get_sw_Ver(char * buf)
{
#ifdef TCSUPPORT_CT
    return tcapi_get("DeviceInfo_devParaStatic", "CustomerSWVersion", buf);
#else
#if !defined(TCSUPPORT_CT_PON_ONUCONFIG) && !defined(TCSUPPORT_CT_C5_HEN_SFU) 
    return tcapi_get("Info_devDefInf", "CustomerSoftwareVersion", buf);
#endif
#endif
}

static inline int tcapi_get_hw_Ver(char * buf)
{
#ifdef TCSUPPORT_CT
    return tcapi_get("DeviceInfo_devParaStatic", "CustomerHWVersion", buf);
#else
    return tcapi_get(EPON_ONU, "HWVer", buf);
#endif
}

static inline int tcapi_get_modle_name(char * buf)
{
#if defined(TCSUPPORT_CT) && !defined(TCSUPPORT_CT_PON_C7) && !defined(TCSUPPORT_CT_PON_CN_JS)
    return tcapi_get("DeviceInfo_devParaStatic", "ModelName", buf);
#else
    return tcapi_get(EPON_ONU, "Model", buf);
#endif
}
 
static inline int tcapi_get_fw_ver(char * buf)
{
#ifdef TCSUPPORT_CT
    return tcapi_get(EPON_ONU, "FWVer", buf);
#else
    return tcapi_get("DeviceInfo", "FwVer", buf);
#endif
}

void oam_ctc_device_init(OamCtcDB_t *pDB,int cpu_type)
{
	int i = 0;
	DeviceInformation_t device_table[] = 
	{
#if !defined(TCSUPPORT_CT_C5_HEN_SFU) 
		{MT7520S, 0, 1, 0, 0, 0},
#endif
		{MT7520, 3, 1, 2, 0, 0},
		{MT7520G, 0, 4, 2, 0, 0},
		{MT7525, 3, 1, 2, 2, 2},
		{MT7525G, 0, 4, 2, 2, 2},
		{EN7521S, 0, 1, 0, 0, 0},
		{EN7521F, 3, 1, 1, 0, 0},
		{EN7526F, 3, 1, 1, 1, 1},
		{EN7526D, 3, 1, 1, 2, 2},
		{EN7526G, 0, 4, 1, 2, 2},
        {EN751221, 3, 1, 2, 1, 2},
		{0, 0, 0, 0, 0, 0},
	};

	for (i = 0; i < ARRAY_SIZE(device_table); i++)
	{
		if (device_table[i].id == cpu_type)
		{
			pDB->dev_info.fe_num = device_table[i].fe_num;
			pDB->dev_info.ge_num = device_table[i].ge_num;
			pDB->dev_info.voip_num = device_table[i].voip_num;
			pDB->dev_info.wlan_num = device_table[i].wlan_num;
			pDB->dev_info.usb_num = device_table[i].usb_num;
			break;
		}
	}
}


int oam_ctc_db_init(OamCtcDB_t *pDB){
	char buf[120] = {0};
	char attr[15] = {0};
	char node[64] = {0};
	u_int val = 0;
	int i = 0, lan_port_num;
	int cpu_type = 0;
	FILE *fp = NULL;
	int len = 0;

	memset(pDB, 0, sizeof(OamCtcDB_t));

	/* Get CPU Type First */
	if ((fp = fopen("/proc/gpon/debug", "r")) == NULL){
		tcdbg_printf("Critical Bugs, Can't get cpu_type!\n");
		return -1;
	}
	
	fscanf(fp, "%d", &cpu_type);
	fclose(fp);	
	pDB->cpu_type = cpu_type;
	
	#ifdef CONFIG_DEBUG
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get("Oam_CTC", "DebugLevel", buf)){
		eponOamCtcDbgLvl = atoi(buf);
		pDB->debug_level = eponOamCtcDbgLvl;
	}
	#endif
	
	pDB->oui = 0x111111;
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get("Oam_CTC", "OUI", buf)){
		sscanf(buf, "%x", &val);
		gOuiCtc[0] = (u_char)(val>>16);
		gOuiCtc[1] = (u_char)(val>>8);
		gOuiCtc[2] = (u_char)val;
		pDB->oui = val;
	}
	
	/* Init Device Info */
	pDB->dev_info.adsl_num = 0;
	pDB->dev_info.vdsl_num = 0;
	pDB->dev_info.tdm_num = 0;
	pDB->dev_info.voip_num = 0;
	pDB->dev_info.usb_num = 0;
	pDB->dev_info.wlan_num = 0;
	pDB->dev_info.ge_num = 0;
	pDB->dev_info.fe_num = 0;


	oam_ctc_device_init(pDB,cpu_type);
	

//#ifdef TCSUPPORT_CT
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "GEPortNum", buf)){
		pDB->dev_info.ge_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "FEPortNum", buf)){
		pDB->dev_info.fe_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "WLANPortNum", buf)){
		pDB->dev_info.wlan_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "USBPortNum", buf)){
		pDB->dev_info.usb_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "POTSPortNum", buf)){
		pDB->dev_info.voip_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "E1PortNum", buf)){
		pDB->dev_info.tdm_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "ADSL2PortNum", buf)){
		pDB->dev_info.adsl_num = atoi(buf);
	}
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "VDSL2PortNum", buf)){
		pDB->dev_info.vdsl_num = atoi(buf);
	}	
//#endif

	pDB->dev_info.lan_num = pDB->dev_info.ge_num + pDB->dev_info.fe_num;
	
	pDB->dev_info.us_queue_num = 8; // our device default
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "USQueuesNum", buf)){
		pDB->dev_info.us_queue_num = atoi(buf);
	}

	pDB->dev_info.us_queue_max_per_port = 8; // our device default
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "USQueueMaxPerPort", buf)){
	    pDB->dev_info.us_queue_max_per_port = atoi(buf);
	}

	pDB->dev_info.ds_queue_num = 8; // our device default
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "DSQueuesNum", buf)){
	    pDB->dev_info.ds_queue_num = atoi(buf);
	}

	pDB->dev_info.ds_queue_max_per_port = 8; // our device default
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "DSQueueMaxPerPort", buf)){
	    pDB->dev_info.ds_queue_max_per_port = atoi(buf);
	}

	memset(buf, 0, sizeof(buf));
	pDB->dev_info.battery_backup = BATTERY_BACKUP_NO;
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "BatteryBackup", buf)){
	    if (0 == strcmp(buf, "Yes"))
	    	pDB->dev_info.battery_backup = BATTERY_BACKUP_YES;
	}

	pDB->dev_info.fec_ability = 2; // support FEC
	pDB->dev_info.max_llid_num = EPON_LLID_MAX_NUM;
	pDB->dev_info.service_sla_num = 1;
	pDB->dev_info.protect_type = PROTECTION_TYPE_NOSUPPORTED;
	pDB->dev_info.pon_if_num = 1;
	#ifdef TCSUPPORT_IPV6
	pDB->dev_info.ipv6_spt = IPV6_SUPPORTED;
	#else
	pDB->dev_info.ipv6_spt = IPV6_NOSUPPORTED;
	#endif

	pDB->dev_info.power_ctrl_type = POWER_CTL_NOSUPPORTED;
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "PowerSupply", buf)){
		u_char psType = atoi(buf);
		if (psType < 3){
			pDB->dev_info.power_ctrl_type = psType;
		}
	}

	pDB->dev_info.powersaving_cap = SLEEP_MODE_NONE;
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_POWERSAVING, "Capability", buf)){
		u_char sMode = atoi(buf);
		if (sMode < 4){
			pDB->dev_info.powersaving_cap = sMode;
		}
	}

	pDB->dev_info.powersaving_early_wakeup_spt = EARLY_WAKE_NOT_SUPPORTED;
	memset(buf, 0, sizeof(buf));
	if (TCAPI_SUCCESS == tcapi_get(EPON_POWERSAVING, "EarlyWakeup", buf)){
		if (0 == strcmp(buf, "Yes")){
			pDB->dev_info.powersaving_early_wakeup_spt = EARLY_WAKE_SUPPORTED;
		}
	}

	/* Get ONU Info  */
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "VendorID", buf)){
	#ifdef TCSUPPORT_CT
	    lowAlignStrcpy(pDB->dev_info.vendor_id, buf, 4);
	#else
		memcpy(pDB->dev_info.vendor_id, buf, 4);
    #endif
	}
	if (TCAPI_SUCCESS == tcapi_get_modle_name(buf)){
	#ifdef TCSUPPORT_CT
		len = strlen(buf);
		if(len < 4)
			lowAlignStrcpy(pDB->dev_info.model, buf, 4);
		else
			memcpy(pDB->dev_info.model, buf+(len-4), 4);
    #else
		memcpy(pDB->dev_info.model, buf, 4);
	#endif
	}
	if (TCAPI_SUCCESS == tcapi_get_hw_Ver(buf)){
		lowAlignStrcpy(pDB->dev_info.hw_ver, buf, 8);
	}
	if (TCAPI_SUCCESS == tcapi_get_sw_Ver(buf)){
		lowAlignStrcpy(pDB->dev_info.sw_ver, buf, 16);
	}
	if (TCAPI_SUCCESS == tcapi_get(EPON_ONU, "ExtModel", buf)){
		lowAlignStrcpy(pDB->dev_info.ext_model, buf, 16);
	}
	if (TCAPI_SUCCESS == tcapi_get_fw_ver(buf)){
		strncpy(pDB->dev_info.fw_ver, buf, 126);
		pDB->dev_info.fw_ver[127] = '\0';
	}

	{
		u_short vendorID, model, revision;
		u_int version[3];
		
		memset(buf, 0, sizeof(buf));
		if ( (TCAPI_SUCCESS != tcapi_get(EPON_PON, "VendorID", buf)) || 
			 (1 != sscanf(buf, "%hX", &vendorID)) ){
			return FAIL;
		}
		pDB->dev_info.pon_vendor_id  = vendorID;

		memset(buf, 0, sizeof(buf));
		if ( (TCAPI_SUCCESS != tcapi_get(EPON_PON, "Model", buf)) ||
			 (1 != sscanf(buf, "%hX", &model)) ){
			return FAIL;
		}
		pDB->dev_info.pon_chip_model = model;


		memset(buf, 0, sizeof(buf));
		if ( (TCAPI_SUCCESS != tcapi_get(EPON_PON, "Revision", buf)) ||
			 (1 != sscanf(buf, "%hu", &revision))  ){
			return FAIL;
		}
		pDB->dev_info.pon_revision  = revision;

		memset(buf, 0, sizeof(buf));
		if ( (0 != tcapi_get(EPON_PON, "Version", buf)) ||
			 (3 != sscanf(buf, "%u/%u/%u", &(version[0]), \
				&(version[1]),&(version[2]))) ){
			return FAIL;
		}
		pDB->dev_info.pon_ver_date[0] = version[0];
		pDB->dev_info.pon_ver_date[1] = version[1];
		pDB->dev_info.pon_ver_date[2] = version[2];
	}
	
	/* Init Config info */
	pDB->cfg_info.cur_llid_en_num = 1;
	pDB->cfg_info.powersaving_early_wakeup_en = FALSE;
	pDB->cfg_info.powersaving_max_duration = 0;
	pDB->cfg_info.port_aging_time = 300;
	pDB->cfg_info.pon_los_optical_time = 2;
	pDB->cfg_info.pon_los_mac_time = 55;
	pDB->cfg_info.pon_holdover_state = HOLDOVER_DISACTIVATED;
	pDB->cfg_info.pon_holdover_time = 50;
	
	memset(&(pDB->cfg_info.sla), 0, sizeof(OamCtcDBSla_t));
	pDB->cfg_info.sla.enable = FALSE;
	
	/* Init Port Info */
	lan_port_num = pDB->dev_info.lan_num;
	lan_port_num = MIN(lan_port_num, 4);
	for (i =0; i < lan_port_num; ++i){
		memset(pDB->port+i, 0, sizeof(OamCtcDBPortInfo_t));
		pDB->port[i].vlan.mode = VLAN_MODE_TRANSPARENT;
		pDB->port[i].phy_ctrl_en = TRUE;
		pDB->port[i].auto_neg_en = TRUE;
	}
	
	/* Init Software Upgrade Info */
	pDB->swup.main = 0;
	pDB->swup.valid0 = 1;
	pDB->swup.valid1 = 0;
	pDB->swup.active = 0;	
	if (TCAPI_SUCCESS == tcapi_get(EPON_SOFTIMAGE, "Main", buf)){
		pDB->swup.main = atoi(buf);
	}
	if (TCAPI_SUCCESS == tcapi_get(EPON_SOFTIMAGE, "Valid0", buf)){
		pDB->swup.valid0 = atoi(buf);
	}
	if (TCAPI_SUCCESS == tcapi_get(EPON_SOFTIMAGE, "Valid1", buf)){
		pDB->swup.valid1 = atoi(buf);
	}
	if (TCAPI_SUCCESS == tcapi_get(EPON_SOFTIMAGE, "Active", buf)){
		pDB->swup.active = atoi(buf);
	}
	
	/* Init Per LLID's Info */
	for (i=0; i < pDB->cfg_info.cur_llid_en_num; ++i){
		oam_ctc_db_init_llid(i, (pDB->llid+i));
	}

#ifdef TCSUPPORT_EPON_OAM_CUC
	oam_cuc_cmd_db_load();
	eponOamInitParam2();
#endif

#ifdef TCSUPPORT_VOIP
    oam_ctc_db_voip_conf_update(pDB, 0);
#endif
	
	return SUCCESS;
}

int oam_ctc_db_init_llid(int llid, OamCtcDBLlid_t *pLlid){
	int i = 0;
	char node[64]={0}, attr[64]={0}, buf[128] = {0};
	
	pLlid->fec_en = FALSE;
	
	memset(&(pLlid->auth), 0, sizeof(OamCtcDBAuth_t));
	pLlid->auth.desired_type = AUTH_TYPE_LOID_PWD;
	pLlid->auth.state = 0;
	
	if (eponOamCfg.tmpTestMask & EPON_OAM_TESTMASK_TMPLOID)
		strcpy(node, "Info_Oam");
	else
		strcpy(node, EPON_LOIDAUTH);
	
	memset(buf, 0, sizeof(buf));
	sprintf(attr, "LOID%d", i);
	if (TCAPI_SUCCESS == tcapi_get(node, attr, buf)){
		lowAlignStrcpy((char*)pLlid->auth.loid, buf, 24);
	}
	
	memset(buf, 0, sizeof(buf));
	sprintf(attr, "Password%d", i);
	if (TCAPI_SUCCESS == tcapi_get(node, attr, buf)){
		lowAlignStrcpy((char*)pLlid->auth.passwd, buf,12);
	}
	
	memset(&(pLlid->llid_queue_cfg), 0, sizeof(OamCtcDBLQCfg_t));
	return SUCCESS;
}

inline int oam_ctc_db_show(){
	return oam_ctc_db_print(&gCtcDB);
}

int oam_ctc_db_print(OamCtcDB_t *pDB){
	int i = 0;
	OamCtcDBPortInfo_t *pPort = NULL;
	OamCtcDBPortVlan_t *pVlan = NULL;
	OamCtcDBPortMCast_t *pMcast = NULL;
	OamCtcDBDevInfo_t *pDev = NULL;
	
	if (!pDB)
		return -1;
	
	dprintf("======General Info======\n");
	#ifdef TCSUPPORT_EPON_OAM_CUC
	dprintf("[ConfigCounter: %u]\n", pDB->config_counter);
	#endif
	dprintf("[CPU: %s, OUI: %06X]\n", (isMT7520S(pDB->cpu_type)?"MT7520S":\
								  (isMT7520F(pDB->cpu_type)?"MT7520F":\
								   (isMT7520G(pDB->cpu_type)?"MT7520G":\
								    (isMT7525F(pDB->cpu_type)?"MT7525F":\
								    (isMT7525G(pDB->cpu_type)?"MT7525G":\
								  "Unkown"))))), pDB->oui);

	pDev = &pDB->dev_info;
	dprintf("======Device Info======\n");
	dprintf("[GE         : %u, FE       : %u, WLAN   : %u]\n", pDev->ge_num, pDev->fe_num, pDev->wlan_num);
	dprintf("[VOIP       : %u, USB      : %u,  Battery :%u]\n", pDev->voip_num, pDev->usb_num, pDev->battery_backup);
	dprintf("[US QNum    : %u, PerPort  : %u]\n", pDev->us_queue_num, pDev->us_queue_max_per_port);
	dprintf("[DS QNum    : %u, PerPort  : %u]\n", pDev->ds_queue_num, pDev->ds_queue_max_per_port);
	dprintf("[FEC        : %u, MaxLLID  : %u, SLA    : %u]\n", pDev->fec_ability, pDev->max_llid_num, pDev->service_sla_num);
	dprintf("[Protect    : %u, PON IF   : %u]\n", pDev->protect_type, pDev->pon_if_num);
	dprintf("[IPV6       : %u, Power Supply: %u]\n", pDev->ipv6_spt, pDev->power_ctrl_type);
	dprintf("[PowerSaving: %u, EarlyWakeup : %u]\n", pDev->powersaving_cap, pDev->powersaving_early_wakeup_spt);
	dprintf("[VendorID   : %c%c%c%c, Model: %c%c%c%c]\n", pDev->vendor_id[0], 
		pDev->vendor_id[1], pDev->vendor_id[2], pDev->vendor_id[3], pDev->model[0],
		pDev->model[1],pDev->model[2],pDev->model[3]);
	dprintf("[HWVer          : %s,    SWVer    : %s]\n", pDev->hw_ver, pDev->sw_ver);
	dprintf("[ExtModel       : %s,     FWVer   : %s]\n", pDev->ext_model, pDev->fw_ver);
	dprintf("[Pon VendorID   : %04x, ChipModel : %04x]\n", pDev->pon_vendor_id, pDev->pon_chip_model);
	dprintf("[Revision       : %u,   VerDate   : %u/%u/%u]\n", pDev->pon_revision, pDev->pon_ver_date[0], 
		pDev->pon_ver_date[1], pDev->pon_ver_date[2]);
	
	dprintf("======Config Info======\n");
	dprintf("[EnLLIDNum       : %u, PortAgingTime: %u]\n", pDB->cfg_info.cur_llid_en_num, pDB->cfg_info.port_aging_time);
	dprintf("[PowerSaving: MaxDuration: %u, EarlyWakeUpEn: %u]\n", pDB->cfg_info.powersaving_max_duration, pDB->cfg_info.powersaving_early_wakeup_en);
	dprintf("[Pon: LosOptical : %u, LosMac : %u]\n", pDB->cfg_info.pon_los_optical_time, pDB->cfg_info.pon_los_mac_time);
	dprintf("[Holdover State  : %u, Time   : %u]\n", pDB->cfg_info.pon_holdover_state, pDB->cfg_info.pon_holdover_time);

	
	dprintf("======Port Info======\n");
	for (i=0; i<pDev->lan_num; ++i){
		pPort = pDB->port + i;
		dprintf("  >Port%d:\n", i);
		dprintf("    [PhyCtrlEn    : %d, PhyLinkUp    : %d]\n", pPort->phy_ctrl_en, pPort->phy_linkup);
		dprintf("    [AutoNegEn    : %d, FlowCtrl     : %d]\n", pPort->auto_neg_en, pPort->flow_ctrl_en);
		dprintf("    [LoopDetect   : %d, DisableLoop  : %d]\n", pPort->loopdetect, pPort->disable_looped);
		dprintf("    [UsPolicingEn : %d, DsRateLmtEn  : %d]\n", pPort->us_policing_en, pPort->ds_ratelimit_en);
		dprintf("    [UsCIR: %d, UsCBS: %d, UsEBS: %d]\n", pPort->us_cir, pPort->us_cbs, pPort->us_ebs);
		dprintf("    [DsCIR: %d, DsPBS: %d]\n", pPort->ds_cir, pPort->ds_pir);

		pVlan = &pPort->vlan;
		dprintf("      VLAN INFO:\n");
		dprintf("        [Mode: %d, Num: %d, DefTag: %x]\n", pVlan->mode, pVlan->num, pVlan->def_tag);

		pMcast = &pPort->mcast;
		dprintf("      MCast INFO:\n");
		dprintf("        [MVlanNum : %d, TagOper: %d]\n", pMcast->trunk_num, pMcast->tag_strip);
		dprintf("        [GrpMaxNum: %d, ]\n", pMcast->max_grp_num);
	}
	
}


int oam_ctc_db_sync_init(void){
	int ret = 0;

	if (0 != sem_init(&gCtcDB.db_sync_sem, 0, 0)){
		return FAIL;
	}

	ret = pthread_create(&gCtcDB.db_sync_thread, NULL, (void*)oam_ctc_db_sync_task, NULL);
	if (0 != ret)
		return FAIL;

	return SUCCESS;
}
#ifdef TCSUPPORT_EPON_OAM_CUC
extern int setTR069WanInfo();	
extern int doOamSetPingTest();
extern int doOamSetCucPortLoopDetect();
#endif
extern int doOamSetVoIPPort(); 	
extern int doOamSetSIPParameterConfig();
extern int doOamSetSIPUserParameterConfig();
extern int doOamSetFaxModemConfig();
extern int doOamSetH248ParameterConfig();
extern int doOamSetH248UserTIDInfo();
extern int doOamSetH248RTPTIDConfig();
extern int doOamSetTimerCfg();
extern int doOamSetPortDisableLooped();
#ifdef TCSUPPORT_EPON_OAM_CUC
extern int doOamCucSetPortDisableLooped();
extern char gTR069URL[256];
#endif
extern char gStrSIPDigitMap[1024];

void *oam_ctc_db_sync_task(void){
	u_int  sync_port_mask;
	u_int  sync_llid;
	u_int  sync_sflag;
	u_int  sync_gflag;
	int port = 0;

	
	while(1){
		sem_wait(&gCtcDB.db_sync_sem);

		/* use temp to check sync flag protect other thread change */
		sync_port_mask = gCtcDB.db_sync_port_mask;
		sync_llid = gCtcDB.db_sync_llid;
		sync_sflag = gCtcDB.db_sync_sflag;
		sync_gflag = gCtcDB.db_sync_gflag;
		/* clear FLAGs */
		gCtcDB.db_sync_port_mask = 0;
		gCtcDB.db_sync_llid = 0;
		gCtcDB.db_sync_sflag = 0;
		gCtcDB.db_sync_gflag = 0;

		eponOamExtDbg(DBG_OAM_L4, "<T> Task do sync port_mask=%x, sflag=%x!\n", sync_port_mask, sync_sflag);
		if (sync_sflag){
			/* VOIP CONFIG SET */
		#ifdef TCSUPPORT_VOIP
			if (sync_port_mask & DB_SYNC_PORT_VOIP_0){
				oam_ctc_db_voip_conf_set(&gCtcDB, sync_sflag, 0);
			}
			if (sync_port_mask & DB_SYNC_PORT_VOIP_1){
				oam_ctc_db_voip_conf_set(&gCtcDB, sync_sflag, 1);
			}

			if (sync_sflag & DB_SYNC_VOIPBASIC_COMMIT_CONFIG){
				tcapi_commit(VOIPBASIC);	
			}	

			if (sync_sflag & DB_SYNC_VOIPADV_COMMIT_CONFIG){
				tcapi_commit(VOIPADVANCED);	
			}

			if (sync_sflag & DB_SYNC_VOIPH248_COMMON_COMMIT_CONFIG){
				tcapi_commit(VOIPH248_COMMON);	
			}

			if (sync_sflag & DB_SYNC_VOIPDIGIT_MAP_ENTRY_COMMIT_CONFIG){
				tcapi_commit(VOIP_DIGIT_MAP_ENTRY);	
			}
			#ifdef TCSUPPORT_EPON_OAM_CUC
			if (sync_sflag & DB_SYNC_CUC_PINT_TEST_CONFIG){
				doOamSetPingTest(); 
			}
			#endif
			if (sync_sflag & DB_SYNC_VIOP_PORT_CONFIG){
				doOamSetVoIPPort(); 				
			}

			if (sync_sflag & DB_SYNC_SIP_PARAMETER_CONFIG){
				doOamSetSIPParameterConfig();
			}

			if (sync_sflag & DB_SYNC_SIP_USER_PARAMETER_CONFIG){
				doOamSetSIPUserParameterConfig(); 				
			}

			if (sync_sflag & DB_SYNC_FAX_MODEM_CONFIG){
				doOamSetFaxModemConfig();				
			}

			if (sync_sflag & DB_SYNC_H248_PARAMETER_CONFIG){
				doOamSetH248ParameterConfig();
			}

			if (sync_sflag & DB_SYNC_H248_USER_TID_CONFIG){
				doOamSetH248UserTIDInfo();
			}
			
			if (sync_sflag & DB_SYNC_H248_RTP_TID_CONFIG){
				doOamSetH248RTPTIDConfig();
			}

			if (sync_sflag & DB_SYNC_SET_TIMER_CONFIG){
				doOamSetTimerCfg();
			}

			if (sync_sflag & DB_SYNC_SIP_DIGITMAP_CONFIG){
				tcapi_set("Cwmp_Entry", "acsUrl", gStrSIPDigitMap);
			}
			
		#endif
			#ifdef TCSUPPORT_EPON_OAM_CUC
			if (sync_sflag & DB_SYNC_RMS_ACSURL_CONFIG){
				tcapi_set("Cwmp_Entry", "acsUrl", gTR069URL);
			}
		
			if (sync_sflag & DB_SYNC_LOOP_DETECT){
				doOamSetCucPortLoopDetect();
			}

			if (sync_sflag & DB_SYNC_DISABLE_LOOPED_CONFIG){
				doOamCucSetPortDisableLooped();
			}		
			#endif
			if (sync_sflag & DB_SYNC_CTC_PORT_LOOPED_CONFIG){
				doOamSetPortDisableLooped();
			}
			#ifdef TCSUPPORT_EPON_OAM_CUC
			if (sync_sflag & DB_SYNC_RMS_CONFIG1){
				setTR069WanInfo();
			}
			/* CUC Save Config */
			if (sync_sflag & DB_SYNC_SAVE_CONFIG){
				eponOamExtDbg(DBG_OAM_L4, "<T> Task do sync : save config to romfile!\n");
				oam_cuc_cmd_db_save();
			}
			#endif

            		tcapi_set("Info_Oam","SaveFlag","1");
			tcapi_save();
            		tcapi_set("Info_Oam","SaveFlag","0");
		}

		if (sync_gflag){
			oam_ctc_db_update_conf(&gCtcDB, sync_gflag);
		}

	}
}

int oam_ctc_cmd_update_conf(void){
	OamCtcDB_t db;
	int ret = 0;
	
	eponOamExtDbg(DBG_OAM_L1, "<C> execute cmd update config by tcapi save!\n");
	oam_ctc_db_update_conf(&gCtcDB, DB_SYNC_FLAG_UPDATA_INFO);

#ifdef TCSUPPORT_EPON_OAM_CUC
	if (gCtcDB.config_counter != 0){
		memset(&db, 0, sizeof(OamCtcDB_t));
		memcpy(&db.dev_info, &gCtcDB.dev_info, sizeof(OamCtcDBDevInfo_t));
		ret = oam_cuc_db_load(&db);
		if (ret == SUCCESS){
			ret = oam_cuc_db_cmp(&gCtcDB, &db);
			if (ret == FALSE){
				gCtcDB.config_counter = 0;
				tcapi_set(OAM_CUC, "CfgCnt", "0");
				tcapi_save();
			}
		}
	}
#endif
}

/*
 *  This function is used to update info from system when tcapi_save be called!:
 *       like voip info & auth loid&password
 */
int oam_ctc_db_update_conf(OamCtcDB_t *pDB, u_int mask){
	int i = 0;
	char node[64] = {0};
	char buf[64] = {0};
	char attr[64] = {0};

	/* Update auth info : loid & password */
	if (mask & DB_SYNC_FLAG_UPDATA_AUTH){
		for (i=0; i<pDB->cfg_info.cur_llid_en_num; ++i){		
			if (eponOamCfg.tmpTestMask & EPON_OAM_TESTMASK_TMPLOID)
				strcpy(node, "Info_Oam");
			else
				strcpy(node, EPON_LOIDAUTH);

			memset(buf, 0, sizeof(buf));
			sprintf(attr, "LOID%d", i);
			if (TCAPI_SUCCESS == tcapi_get(node, attr, buf)){
				lowAlignStrcpy((char*)pDB->llid[i].auth.loid, buf, 24);
			}

			memset(buf, 0, sizeof(buf));
			sprintf(attr, "Password%d", i);
			if (TCAPI_SUCCESS == tcapi_get(node, attr, buf)){
				lowAlignStrcpy((char*)pDB->llid[i].auth.passwd, buf,12);
			}
		}
	}

	/* Update VoIP info by mask */
#ifdef TCSUPPORT_VOIP
	oam_ctc_db_voip_conf_update(pDB, mask);
#endif

	return SUCCESS;
}

#ifdef TCSUPPORT_VOIP

/* This function will be called to update voip config & state when tcapi_save
 * OR update voip state only when OLT get voip nodes used oam.
 */
int
oam_ctc_db_voip_conf_update(OamCtcDB_t *pDB, u_int mask){
    eponOamExtDbg(DBG_OAM_L4, "Begin voip db update!\n");

    db_voip_conf_update_glb_param_cfg(pDB);
    db_voip_conf_update_pots_status(pDB);
	return SUCCESS;
}

int
oam_ctc_db_voip_conf_set(OamCtcDB_t *pDB, u_int mask, int port){
	char node[64] = {0};
	char buf[128] = {0};

	if (port >= OAM_VOIP_PORT_NUM_MAX)
		return FAIL;

	if (mask & DB_SYNC_FLAG_VOIP_PORT){
        eponOamExtDbg(DBG_OAM_L2, "<T> Task do sync port %d: state=%d\n",
                      port, pDB->cfg_info.voip.port[port].enable);
		sprintf(node, "VoIPBasic_Entry%d", port);
		if (pDB->cfg_info.voip.port[port].enable){
			strcpy(buf, "Yes");
		}else{
			strcpy(buf, "No");
		}
		tcapi_set(node, "SIPEnable", buf);
	}
	
	if (mask & DB_SYNC_FLAG_VOIP_GLB_PARAM_CFG){
	}
	if (mask & DB_SYNC_FLAG_VOIP_SIP_PARMA_CFG){
	}
	if (mask & DB_SYNC_FLAG_VOIP_SIP_USR_PARAM_CFG){
	}
	if (mask & DB_SYNC_FLAG_VOIP_FAX_MODEM_CFG){
	}
	if (mask & DB_SYNC_FLAG_VOIP_IAD_OPERA){
	}
	if (mask & DB_SYNC_FLAG_VOIP_SIP_DIGIT_MAP){
	}
	if (mask & DB_SYNC_FLAG_VOIP_H248_PARAM_CFG){
	}
	if (mask & DB_SYNC_FLAG_VOIP_H248_USR_TID){
	}
	if (mask & DB_SYNC_FLAG_VOIP_H248_RTP_TID){
	}

}
#endif

#ifdef TCSUPPORT_EPON_OAM_CUC
#define TCAPI_SET_UINT(N, A, V)  {\
		sprintf(buf, "%u", (V));\
		tcapi_set((N), (A), buf);\
	}

#define TCAPI_SET_HEX(N, A, V)  {\
		sprintf(buf, "%x", (V));\
		tcapi_set((N), (A), buf);\
	}

/*
 * This function used to save OAM config to romfile:
 * 	Main line is :llid & port & voip
 */
int oam_cuc_cmd_db_save(){
	int i = 0;
	u_int mask = 0xFFFFFFFF;
	char buf[128];

	if (gCtcDB.config_counter == 0){
		TCAPI_SET_UINT(OAM_CUC, "CfgCnt", gCtcDB.config_counter);
		eponOamExtDbg(DBG_OAM_L1, "OAM CUC config counter==0! Not need to save config!\n");
		return FAIL;
	}

	oam_cuc_db_conf_set(&gCtcDB, mask);

	for (i=0; i< gCtcDB.cfg_info.cur_llid_en_num; ++i){
		oam_cuc_db_llid_conf_set(&gCtcDB, mask, i);
	}
	
	for (i=0; i<gCtcDB.dev_info.lan_num; ++i){
		oam_cuc_db_port_conf_set(&gCtcDB, mask, i);
	}

	
#ifdef TCSUPPORT_VOIP
	for (i=0; i<gCtcDB.dev_info.voip_num; ++i){
		oam_ctc_db_voip_conf_set(&gCtcDB, mask, i);
	}
#endif

	tcapi_save();
	return SUCCESS;
}


int oam_cuc_db_conf_set(OamCtcDB_t *pDB, u_int mask){
	int i = 0;
	char node[64] = {0};
	char buf[256] = {0};
	char attr[128] = {0};
	OamCtcDBConf_t *conf_p = NULL;
	OamCtcDBMCast_t *mcast_p = NULL;
	OamCtcDBSla_t *sla_p = NULL;

	if (!pDB || mask==0)
		return SUCCESS;
		
	/* Save power saving config */
	conf_p = &pDB->cfg_info;
	strcpy(node, OAM_CUC);
	TCAPI_SET_UINT(node, "PsEarlyWakeup", conf_p->powersaving_early_wakeup_en);
	sprintf(buf, "%lld", conf_p->powersaving_max_duration);
	tcapi_set(node, "PsMaxDuration", buf);
	
	TCAPI_SET_UINT(node, "MacAgingTime", conf_p->port_aging_time);
	TCAPI_SET_UINT(node, "PonLosOptTime", conf_p->pon_los_optical_time);
	TCAPI_SET_UINT(node, "PonLosMacTime", conf_p->pon_los_mac_time);
	TCAPI_SET_UINT(node, "HoldoverState", conf_p->pon_holdover_state);
	TCAPI_SET_UINT(node, "HoldoverTime", conf_p->pon_holdover_time);

	/* save multicast config */
	mcast_p = &conf_p->mcast;
	TCAPI_SET_UINT(node, "MCastMode", mcast_p->type);
	TCAPI_SET_UINT(node, "MCastFastLeave", mcast_p->fast_leave);

	/* save SLA config */
	sla_p = &conf_p->sla;
	strcpy(node, OAM_CUC_SLA);
	TCAPI_SET_UINT(node, "Enable", sla_p->enable);
	if (sla_p->enable){
		TCAPI_SET_UINT(node, "SchScheme", sla_p->sch_scheme);
		TCAPI_SET_UINT(node, "HighPriBound", sla_p->high_pri_bound);
		TCAPI_SET_UINT(node, "CycleLen", sla_p->cycle_len);
		TCAPI_SET_UINT(node, "Num", sla_p->srvs_num);
		if (sla_p->srvs_num > 0){
			for (i=0; i<sla_p->srvs_num; ++i){
				sprintf(attr, "Queue%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].queue_id);
				sprintf(attr, "FixedPktSize%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].fixed_pkt_size);
				sprintf(attr, "FixedBw%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].fixed_bw);
				sprintf(attr, "AssuredBw%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].assured_bw);
				sprintf(attr, "BestBw%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].best_bw);
				sprintf(attr, "WrrWeight%d", i);
				TCAPI_SET_UINT(node, attr, sla_p->srvs[i].wrr_wight);			
			}
		}
	}

	return SUCCESS;
}

int oam_cuc_db_llid_conf_set(OamCtcDB_t *pDB, u_int mask, int llid){
	int i = 0;
	char node[64] = {0};
	char buf[256] = {0};
	char attr[128] = {0};
	OamCtcDBLQCfg_t *pCfg = NULL;

	if (llid >= pDB->cfg_info.cur_llid_en_num){
		return FAIL;
	}
	sprintf(node, OAM_CUC_LLID, llid);

	/* Save LLID's Config Info only for LLID */
	TCAPI_SET_UINT(node, "FecMode", pDB->llid[llid].fec_en);

	/* Save llid's QueueConfig */
	pCfg = &pDB->llid[llid].llid_queue_cfg;
	TCAPI_SET_UINT(node, "QeueuNum", pCfg->num);

	if (pCfg->num > 0){
		memset(buf, 0, sizeof(buf));
		for (i=0; i<pCfg->num; ++i){
			sprintf(attr, "QueueWeight%d", i);
			sprintf(buf, "%d:%d", pCfg->qcfg[i].qidx, pCfg->qcfg[i].weight);
			tcapi_set(node, attr, buf);
		}
	}
	return SUCCESS;
}

int oam_cuc_db_port_conf_set(OamCtcDB_t *pDB, u_int mask, int port){
	int i = 0;
	char node[64] = {0};
	char attr[128] = {0};
	char buf[256] = {0};
	OamCtcDBPortInfo_t *pPort = NULL;

	if (port >= pDB->dev_info.lan_num)
		return FAIL;	
	pPort = pDB->port + port;
	
	sprintf(node, OAM_CUC_PORT, port);
	TCAPI_SET_UINT(node, "FlowControl", pPort->flow_ctrl_en);

	TCAPI_SET_UINT(node, "USPolicingEnable", pPort->us_policing_en);	
	if (pPort->us_policing_en == TRUE){
		TCAPI_SET_UINT(node, "USPolicingCIR", pPort->us_cir);
		TCAPI_SET_UINT(node, "USPolicingCBS", pPort->us_cbs);
		TCAPI_SET_UINT(node, "USPolicingEBS", pPort->us_ebs);
	}

	TCAPI_SET_UINT(node, "DSRateLimitEnable", pPort->ds_ratelimit_en); 
	if (pPort->us_policing_en == TRUE){
		TCAPI_SET_UINT(node, "DSRateLimitCIR", pPort->ds_cir);
		TCAPI_SET_UINT(node, "DSRateLimitPIR", pPort->ds_pir);
	}

	TCAPI_SET_UINT(node, "LoopDetect", pPort->loopdetect);
	TCAPI_SET_UINT(node, "DisableLooped", pPort->disable_looped);
	TCAPI_SET_UINT(node, "PhyAdminControl", pPort->phy_ctrl_en);
	TCAPI_SET_UINT(node, "AutoNegAdminControl", pPort->auto_neg_en);
	TCAPI_SET_UINT(node, "MacLimitCnt", pPort->mac_limit_cnt);
	TCAPI_SET_UINT(node, "PortMode", pPort->port_mode);

	if (!eponOamCfg.isHgu){
		/* Save VLAN CONFIG */
		TCAPI_SET_UINT(node, "VlanMode", pPort->vlan.mode);
		
		if (pPort->vlan.mode != VLAN_MODE_TRANSPARENT){
			TCAPI_SET_HEX(node, "DefTag", pPort->vlan.def_tag);
		}
		if (pPort->vlan.mode == VLAN_MODE_TRANSLATION){
			memset(buf, 0, sizeof(buf));
			if (pPort->vlan.num > 0){
				sprintf(buf, "%x:%x", pPort->vlan.old_tags[0], pPort->vlan.new_tags[0]);
				for (i=1; i<pPort->vlan.num; ++i){
					sprintf(buf, "%s;%x:%x", buf, pPort->vlan.old_tags[i], pPort->vlan.new_tags[i]);
				}
				tcapi_set(node, "VlanTranslate", buf);
			}
		}else if (pPort->vlan.mode == VLAN_MODE_TRUNK){
			memset(buf, 0, sizeof(buf));
			if (pPort->vlan.num>0){
				sprintf(buf, "%x", pPort->vlan.old_tags[0]);
				for (i=1; i<pPort->vlan.num; ++i){
					sprintf(buf, "%s;%x", buf, pPort->vlan.old_tags[i]);
				}
				tcapi_set(node, "VlanTrunk", buf);
			}
		}

		/* Save Classification&Remarking Config */
		oam_cuc_db_port_clsfy_remark_conf_set(pDB, port);
	}

	/* Save Add/Del MCastVlan config */
	TCAPI_SET_UINT(node, "MCastTrunkNum", pPort->mcast.trunk_num);
	if (pPort->mcast.trunk_num > 0){
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%x", pPort->mcast.trunk_vid[0]);
		for (i=1; i<pPort->mcast.trunk_num;++i){
			sprintf(buf, "%s;%x", buf, pPort->mcast.trunk_vid[i]);
		}
		tcapi_set(node, "MCastVlan", buf);
	}
	
	/* Save MCast Tag Opera */
	TCAPI_SET_UINT(node, "MCastTagMode", pPort->mcast.tag_strip);
	if (pPort->mcast.tag_strip == TAG_STRIPED_SWITCH){
		memset(buf, 0, sizeof(buf));
		sprintf(buf, "%x:%x", pPort->mcast.mcast_vid[0], pPort->mcast.user_vid[0]);
		for (i=1; i<pPort->mcast.trans_num;++i){
			sprintf(buf, "%s;%x:%x", buf, pPort->mcast.mcast_vid[i], pPort->mcast.user_vid[i]);
		}
		tcapi_set(node, "MCastTransVlan", buf);
	}

	TCAPI_SET_UINT(node, "MCastGrpMaxNum", pPort->mcast.max_grp_num);
	
	return SUCCESS;
}

int oam_cuc_db_port_clsfy_remark_conf_set(OamCtcDB_t *pDB, int port)
{
	int i = 0;
	char node[64] = {0};
	char buf[256] = {0};
	char attr[128] = {0};
	u_char *ptmp;
	QosMatchRule_Ptr pMatch = NULL;
	QosResult_t result;
	QosMatchRule_t matchs[MAX_FIELD_NUM];
	u_char rulesNum, matchsNum, field, j;

	rulesNum = eponmapGetClsfyRulesNum(port);
	if (rulesNum > 0){
		sprintf(attr, "%u", rulesNum);
		if (TCAPI_SUCCESS == tcapi_set(node, "RulesNum", attr)){
			for(i=0; i<rulesNum; i++){
				memset(matchs, 0, sizeof(matchs));
				memset(&result, 0, sizeof(result));
		
				matchsNum = MAX_FIELD_NUM;
				if (0 != eponmapGetClsfyRule(port, i, &result, &matchsNum, matchs))
					continue;
		
				sprintf(attr, "Rule%d_Precedence", i);
				sprintf(buf, "%u", result.precedence);
				tcapi_set(node, attr, buf);
			
				sprintf(attr, "Rule%d_EtherPriorityMark", i);
				sprintf(buf, "%u", result.priority);
				tcapi_set(node, attr, buf);
			
				sprintf(attr, "Rule%d_QueueMapped", i);
				sprintf(buf, "%u", result.queueMapped);
				tcapi_set(node, attr, buf);
					
				sprintf(attr, "Rule%d_MatchNum", i);
				sprintf(buf, "%u", matchsNum);
				tcapi_set(node, attr, buf);
		
				for (j = 0; j < matchsNum; ++j){							
					pMatch = &(matchs[j]);
					
					sprintf(attr, "Rule%d_Match%d_Field", i, j);
					sprintf(buf, "%.2x", pMatch->field);
					tcapi_set(node, attr, buf);
		
					memset(buf, 0, sizeof(buf));
					if ( pMatch->field < FIELD_IPVER){
						switch( pMatch->field){
							case FIELD_SMAC:
							case FIELD_DMAC:
								sprintf(buf, "%.2x%.2x%.2x%.2x%.2x%.2x",  pMatch->mac[0],\
									 pMatch->mac[1], pMatch->mac[2], pMatch->mac[3], pMatch->mac[4], pMatch->mac[5]);
								break;
							case FIELD_PBIT:
							case FIELD_IPPROTO4:
							case FIELD_IPDSCP4:
							case FIELD_IPDSCP6:
								sprintf(buf, "%.2x",  pMatch->v8);
								break;		
							case FIELD_VLANID:
							case FIELD_ETHTYPE:
							case FIELD_SPORT:
							case FIELD_DPORT:
								sprintf(buf, "%.4x",  pMatch->v16);
								break;						
							case FIELD_SIP4:
							case FIELD_DIP4:
								sprintf(buf, "%.8x",  pMatch->ip4);
								break;
							default:
								break;
						}
		
					}else{				
						switch( pMatch->field){
							case FIELD_SIP6:
							case FIELD_DIP6:
							case FIELD_SIP6PREX:
							case FIELD_DIP6PREX:
								ptmp = (u_char *)(&(pMatch->ip6));
								sprintf(buf, "%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x",\
									ptmp[0],ptmp[1],ptmp[2],ptmp[3],ptmp[4],ptmp[5],\
									ptmp[6],ptmp[7],ptmp[8],ptmp[9],ptmp[10],ptmp[11],\
									ptmp[12],ptmp[13],ptmp[14],ptmp[15]);					
								break;
							case FIELD_IPVER:
							case FIELD_IPPROTO6:						
								sprintf(buf, "%.2x", pMatch->v8);
								break;
							case FIELD_FLOWLABEL6:
								sprintf(buf, "%.8x", pMatch->ip4);
								break;
							default:
								break;
								
						}
					}
					
					sprintf(attr, "Rule%d_Match%d_Content", i, j);							
					tcapi_set(node, attr, buf);
					
					sprintf(attr, "Rule%d_Match%d_Operator", i, j);
					sprintf(buf, "%.2x",  pMatch->op);	
					tcapi_set(node, attr, buf);
				}
			}
		}
	}
}


#define TCAPI_GET_INT(N, A, V) {\
		if (TCAPI_SUCCESS == tcapi_get((N), (A), buf)){\
			(V) = atoi(buf);\
		}\
	}

#define TCAPI_GET_HEX(N, A, V) {\
		if (TCAPI_SUCCESS == tcapi_get((N), (A), buf)){\
			sscanf(buf, "%x", &(V));\
		}\
	}

int oam_cuc_cmd_db_load(void){
	eponOamExtDbg(DBG_OAM_L1, "<O> load CUC oam config from romfile!\n");
	return oam_cuc_db_load(&gCtcDB);
}
/* 
 * This function used to load config from system romfile:
 *	To be known: only init oam to config running system
 */
int oam_cuc_db_load(OamCtcDB_t *pDB){
	int i = 0;
	char node[64] = {0};
	char attr[64] = {0};
	char buf[256] = {0};
	OamCtcDBConf_t *conf_p = NULL;
	OamCtcDBMCast_t *mcast_p = NULL;
	OamCtcDBSla_t *sla_p = NULL;
	
	if (!pDB)
		return FAIL;

	TCAPI_GET_INT(OAM_CUC, "CfgCnt", pDB->config_counter);
	if (pDB->config_counter == 0){
		eponOamExtDbg(DBG_OAM_L1, "OAM CUC config counter==0! Not need to load config!\n");
		return FAIL;
	}
		
	/* Save power saving config */
	conf_p = &pDB->cfg_info;
	strcpy(node, OAM_CUC);
	TCAPI_GET_INT(node, "PsEarlyWakeup", conf_p->powersaving_early_wakeup_en);
	if (TCAPI_SUCCESS == tcapi_get(node, "PsMaxDuration", buf)){
		sscanf(buf, "%lld", &(conf_p->powersaving_max_duration));
	}
	
	TCAPI_GET_INT(node, "MacAgingTime", conf_p->port_aging_time);
	TCAPI_GET_INT(node, "PonLosOptTime", conf_p->pon_los_optical_time);
	TCAPI_GET_INT(node, "PonLosMacTime", conf_p->pon_los_mac_time);
	TCAPI_GET_INT(node, "HoldoverState", conf_p->pon_holdover_state);
	TCAPI_GET_INT(node, "HoldoverTime", conf_p->pon_holdover_time);

	/* save multicast config */
	mcast_p = &conf_p->mcast;
	TCAPI_GET_INT(node, "MCastMode", mcast_p->type);
	TCAPI_GET_INT(node, "MCastFastLeave", mcast_p->fast_leave);

	/* save SLA config */
	sla_p = &conf_p->sla;
	strcpy(node, OAM_CUC_SLA);
	TCAPI_GET_INT(node, "Enable", sla_p->enable);
	if (sla_p->enable){
		TCAPI_GET_INT(node, "SchScheme", sla_p->sch_scheme);
		TCAPI_GET_INT(node, "HighPriBound", sla_p->high_pri_bound);
		TCAPI_GET_INT(node, "CycleLen", sla_p->cycle_len);
		TCAPI_GET_INT(node, "Num", sla_p->srvs_num);
		if (sla_p->srvs_num > 0){
			for (i=0; i<sla_p->srvs_num; ++i){
				sprintf(attr, "Queue%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].queue_id);
				sprintf(attr, "FixedPktSize%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].fixed_pkt_size);
				sprintf(attr, "FixedBw%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].fixed_bw);
				sprintf(attr, "AssuredBw%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].assured_bw);
				sprintf(attr, "BestBw%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].best_bw);
				sprintf(attr, "WrrWeight%d", i);
				TCAPI_GET_INT(node, attr, sla_p->srvs[i].wrr_wight);			
			}
		}
	}
	
	for (i=0; i < pDB->dev_info.lan_num; ++i){
		oam_cuc_db_port_conf_get(pDB, i);
	}

	for (i=0; i < pDB->cfg_info.cur_llid_en_num; i++){
		oam_cuc_db_llid_conf_get(pDB, i);
	}

	return SUCCESS;
}

int oam_cuc_db_llid_conf_get(OamCtcDB_t *pDB, int llid){
	int i = 0;	
	char node[64] = {0};	
	char buf[256] = {0};	
	char attr[128] = {0};	
	OamCtcDBLQCfg_t *pCfg = NULL;
	
	if (llid >= EPON_LLID_MAX_NUM){
		return FAIL;	
	}
	sprintf(node, OAM_CUC_LLID, llid);

	/* Save LLID's Config Info only for LLID */
	TCAPI_GET_INT(node, "FecMode", pDB->llid[llid].fec_en);

	/* Save llid's QueueConfig */
	pCfg = &pDB->llid[llid].llid_queue_cfg;
	TCAPI_GET_INT(node, "QeueuNum", pCfg->num);

	if (pCfg->num > 0){
		int qidx, weight;
		memset(buf, 0, sizeof(buf));
		for (i=0; i<pCfg->num; ++i){
			sprintf(attr, "QueueWeight%d", i);
			if (TCAPI_SUCCESS == tcapi_get(node, attr, buf)){
				if (2 == sscanf(buf, "%d:%d", &qidx, &weight)){
					pCfg->qcfg[i].qidx = qidx;
					pCfg->qcfg[i].weight = weight;
				}
			}
		}
	}

	return SUCCESS;
}

int oam_cuc_db_port_conf_get(OamCtcDB_t *pDB, int port){
	int i = 0;	
	char node[64] = {0};	
	char buf[256] = {0};	
	char attr[128] = {0};
	char *ch_p = NULL;
	u_int oldTag, newTag;
	OamCtcDBPortInfo_t *pPort = NULL;

	if (port >= pDB->dev_info.lan_num)
		return FAIL;	
	pPort = pDB->port + port;
	
	sprintf(node, OAM_CUC_PORT, port);
	TCAPI_GET_INT(node, "FlowControl", pPort->flow_ctrl_en);

	TCAPI_GET_INT(node, "USPolicingEnable", pPort->us_policing_en);	
	if (pPort->us_policing_en == TRUE){
		TCAPI_GET_INT(node, "USPolicingCIR", pPort->us_cir);
		TCAPI_GET_INT(node, "USPolicingCBS", pPort->us_cbs);
		TCAPI_GET_INT(node, "USPolicingEBS", pPort->us_ebs);
	}

	TCAPI_GET_INT(node, "DSRateLimitEnable", pPort->ds_ratelimit_en); 
	if (pPort->us_policing_en == TRUE){
		TCAPI_GET_INT(node, "DSRateLimitCIR", pPort->ds_cir);
		TCAPI_GET_INT(node, "DSRateLimitPIR", pPort->ds_pir);
	}

	TCAPI_GET_INT(node, "LoopDetect", pPort->loopdetect);
	TCAPI_GET_INT(node, "DisableLooped", pPort->disable_looped);
	TCAPI_GET_INT(node, "PhyAdminControl", pPort->phy_ctrl_en);
	TCAPI_GET_INT(node, "AutoNegAdminControl", pPort->auto_neg_en);
	TCAPI_GET_INT(node, "MacLimitCnt", pPort->mac_limit_cnt);
	TCAPI_GET_INT(node, "PortMode", pPort->port_mode);

	eponOamExtDbg(DBG_OAM_L0, "<T> Get ONU VLAN: mode=%d\n", eponOamCfg.isHgu);
	if (!eponOamCfg.isHgu){
		/* load VLAN CONFIG */
		TCAPI_GET_INT(node, "VlanMode", pPort->vlan.mode);
		
		if (pPort->vlan.mode != VLAN_MODE_TRANSPARENT){
			TCAPI_GET_HEX(node, "DefTag", pPort->vlan.def_tag);
		}
		if (pPort->vlan.mode == VLAN_MODE_TRANSLATION){
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS == tcapi_get(node, "VlanTranslate", buf)){
				ch_p = buf;
				i = 0;
				while(ch_p != NULL && *ch_p != '\0'){
					if(2!= sscanf(ch_p, "%x:%x", &oldTag, &newTag))
						break;
					pPort->vlan.old_tags[i] = oldTag;
					pPort->vlan.new_tags[i] = newTag;
					i++;
					
					ch_p = strstr(ch_p, ";");
					if (ch_p != NULL) ch_p += 1;
				}
				pPort->vlan.num = i;
			}
		}else if (pPort->vlan.mode == VLAN_MODE_TRUNK){
			memset(buf, 0, sizeof(buf));
			if (TCAPI_SUCCESS == tcapi_get(node, "VlanTrunk", buf)){
				ch_p = buf;
				i = 0;
				while(ch_p != NULL && *ch_p != '\0'){
					if(1!= sscanf(ch_p, "%x", &oldTag))
						break;
					pPort->vlan.old_tags[i] = oldTag;
					i++;
					
					ch_p = strstr(ch_p, ";");
					if (ch_p != NULL) ch_p += 1;
				}
				pPort->vlan.num = i;
			}
		}

		/* load Classification&Remarking Config */
		oam_cuc_db_port_clsfy_remark_conf_get(pDB, port);
	}

	/* Save Add/Del MCastVlan config */
	TCAPI_GET_INT(node, "MCastTrunkNum", pPort->mcast.trunk_num);
	if (pPort->mcast.trunk_num > 0){
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "MCastVlan", buf)){
			ch_p = buf;
			i = 0;
			while(ch_p != NULL && *ch_p != '\0'){
				if(1 != sscanf(ch_p, "%x", &oldTag))
					break;
				pPort->mcast.trunk_vid[i] = oldTag;
				i++;
				
				ch_p = strstr(ch_p, ";");
				if (ch_p != NULL) ch_p += 1;
			}
			pPort->mcast.trunk_num = i;
		}
	}
	
	/* Save MCast Tag Opera */
	TCAPI_GET_INT(node, "MCastGrpMaxNum", pPort->mcast.max_grp_num);
	TCAPI_GET_INT(node, "MCastTagMode", pPort->mcast.tag_strip);
	if (pPort->mcast.tag_strip == TAG_STRIPED_SWITCH){
		memset(buf, 0, sizeof(buf));
		if (TCAPI_SUCCESS == tcapi_get(node, "MCastTransVlan", buf)){
			ch_p = buf;
			i = 0;
			while(ch_p != NULL && *ch_p != '\0'){
				if(2!= sscanf(ch_p, "%x:%x", &oldTag, &newTag))
					break;
				pPort->mcast.mcast_vid[i] = oldTag;
				pPort->mcast.user_vid[i] = newTag;
				i++;
				
				ch_p = strstr(ch_p, ";");
				if (ch_p != NULL) ch_p += 1;
			}
			pPort->mcast.trans_num = i;
		}
	}
	
	return SUCCESS;
}

int oam_cuc_db_port_clsfy_remark_conf_get(OamCtcDB_t *pDB, int port){
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
}


/* 
 * This function used to config running system param
 * when oam init in CUC version only.
 */
int oam_cuc_db_execute(OamCtcDB_t *pDB){
	int i = 0;
	char node[64]={0}, attr[64]={0}, buf[128]={0};

	if (pDB->config_counter <= 0){
		eponOamExtDbg(DBG_OAM_L2, "OAM CUC config counter=0! No need to execute!\n");
		return FAIL;
	}

	/* write DB config to running system  */

	
	
	return SUCCESS;
}

/* 
 * This function used to compare the current romfile config is same with
 * OAM DB config when TCAPI_SAVE is called to check ConfigCounter
 */
int oam_cuc_db_cmp(OamCtcDB_t *pSrcDB, OamCtcDB_t *pDstDB){
	return FALSE;
}

#endif // TCSUPPORT_EPON_OAM_CUC

int oam_ctc_db_mvlan_add(int port, u_int vid){
	int i = 0, found = 0;
	OamCtcDBPortMCast_t *pMcast = NULL;

	if (port >= gCtcDB.dev_info.lan_num){
		pMcast = &gCtcDB.port[port].mcast;
		if (pMcast->trunk_num >= OAM_VLAN_RULE_MAX_NUM)
			return FAIL;

		for (i=0; i<pMcast->trunk_num; ++i){
			if (vid == pMcast->trunk_vid[i]){
				found = 1;
				break;
			}
		}

		if (!found){
			pMcast->trunk_vid[pMcast->trunk_num++] = vid;
		}	
	}
	return SUCCESS;
}

int oam_ctc_db_mvlan_del(int port, u_int vid){
	int i = 0, found = 0;
	OamCtcDBPortMCast_t *pMcast = NULL;

	if (port >= gCtcDB.dev_info.lan_num){
		pMcast = &gCtcDB.port[port].mcast;
		if (pMcast->trunk_num >= OAM_VLAN_RULE_MAX_NUM)
			return FAIL;

		for (i=0; i<pMcast->trunk_num; ++i){
			if (vid == pMcast->trunk_vid[i]){
				found = 1;
				break;
			}
		}

		if (found){
			for (; i < pMcast->trunk_num-1; ++i){
				pMcast->trunk_vid[i] = pMcast->trunk_vid[i+1];
			}
			pMcast->trunk_num --;
		}	
	}
	return SUCCESS;
}

