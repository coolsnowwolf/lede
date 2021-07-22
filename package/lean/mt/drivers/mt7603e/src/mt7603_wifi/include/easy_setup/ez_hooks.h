#ifndef __EZ_HOOK_H__
#define __EZ_HOOK_H__

#ifdef WH_EZ_SETUP
#include "ez_common_structs.h"
#define EZ_STATUS_CODE_SUCCESS         0x0000
#define EZ_STATUS_CODE_MIC_ERROR       0x0001
#define EZ_STATUS_CODE_INVALID_DATA    0x0002
#define EZ_STATUS_CODE_NO_RESOURCE     0x0003
#define EZ_STATUS_CODE_PEER_CONNECTED  0x0004
#define EZ_STATUS_CODE_LOOP			   0x0005
#define EZ_STATUS_CODE_UNKNOWN         0xFFFF

#define MTK_VENDOR_CAPABILITY_SIZE    4
#define MTK_VENDOR_EASY_SETUP         0x40
#define MTK_OUI_LEN                   3
#define RALINK_OUI_LEN                3

#define MLME_SYNC_LOCK					0x1
#define BEACON_UPDATE_LOCK				0x2
#define EZ_MINIPORT_LOCK				0x3
#define SCAN_PAUSE_TIMER_LOCK			0x4

#define EZ_CLEAR_ACTION 0
#define EZ_SET_ACTION 1

#define EZ_CAP_HAS_APCLI_INF  		(1 << 10)
#define EZ_SET_CAP_HAS_APCLI_INF(__cap) (__cap |= EZ_CAP_HAS_APCLI_INF)

#define EZ_INDEX_NOT_FOUND              0xFF

#define EZ_TAG_NON_EZ_BEACON       	  	0x16
#define EZ_TAG_CAPABILITY_INFO        	0x06

#define EZ_PMK_LEN                    32


#define EZ_TLV_TAG_SIZE               1
#define EZ_TLV_LEN_SIZE               1

#define EZ_TAG_OFFSET                 (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE)
#define EZ_TAG_LEN_OFFSET             (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE+EZ_TLV_TAG_SIZE)
#define EZ_TAG_DATA_OFFSET            (MTK_OUI_LEN+MTK_VENDOR_CAPABILITY_SIZE+EZ_TLV_TAG_SIZE+EZ_TLV_LEN_SIZE)

#define EZ_CAPABILITY_LEN             4
#define EZ_GROUP_ID_LEN               4
#define EZ_TAG_BEACON_INFO         	  0x14
#define EZ_TAG_OPEN_GROUP_ID	0x22



#ifdef EZ_DUAL_BAND_SUPPORT
#define IS_SINGLE_CHIP_DBDC(_pAd)	((_pAd)->SingleChip && *(_pAd)->SingleChip == TRUE)
#define IS_DUAL_CHIP_DBDC(_pAd)		!(IS_SINGLE_CHIP_DBDC(_pAd))
#else
#define IS_SINGLE_CHIP_DBDC(_pAd)	1
#define IS_DUAL_CHIP_DBDC(_pAd)		0
#endif


#define OPEN_GROUP_MAX_LEN		20
#define AUTH_MODE_EZ                  0xFF01

#define EZ_MAX_STA_NUM 8
#define IS_CONF_EZ_SETUP_ENABLED(_wdev)  ((_wdev)?(((_wdev)->ez_driver_params.enable_easy_setup) == TRUE):FALSE)
#define IS_EZ_SETUP_ENABLED(_wdev)  ((_wdev)?(((_wdev)->ez_driver_params.ez_started) == TRUE):FALSE)
#define IS_ADPTR_EZ_SETUP_ENABLED(pAd) (pAd->ApCfg.MBSSID[0].wdev.ez_driver_params.ez_started == TRUE)

#ifdef DUAL_CHIP 
#define EZ_ACQUIRE_DUAL_CHIP_DBDC_MLME_LOCK(pAd) \
	do { \
		if (IS_CONF_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev)) \
			if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband()) \
				ez_acquire_lock(pAd, NULL, MLME_SYNC_LOCK); \
	} while(0)

#define EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd) \
	do { \
		if (IS_CONF_EZ_SETUP_ENABLED(&pAd->ApCfg.MBSSID[0].wdev)) \
			if (IS_DUAL_CHIP_DBDC(pAd) && !ez_is_triband()) \
				ez_release_lock(pAd, NULL, MLME_SYNC_LOCK); \
	} while(0)
#else
#define EZ_ACQUIRE_DUAL_CHIP_DBDC_MLME_LOCK(pAd) \
	do { \
	} while(0)

#define EZ_RELEASE_DUAL_CHIP_DBDC_MLME_LOCK(pAd) \
	do { \
	} while(0)
#endif

#ifdef EZ_REGROUP_SUPPORT
#define IS_REGRP_SUPP(_wdev)  (((_wdev)->ez_driver_params.en_regrp_supp) == TRUE)
#define REGRP_MODE_TIME_OUT				300000 /* 300 second 		recheck with app for sync in all places	   (app time + 5sec) */
#define REGRP_UNBLOCK_MODE_TIME_OUT		15000 /* 15 second  		recheck with app for sync in all places	   (app time + 5sec)*/

#define HAS_DIRECT_INTERNET_BIT	 2
#define EZ_CAP_DIRECT_INTERNET        (1 << 14)

#define EZ_GET_CAP_DIRECT_INTERNET(__cap) (__cap & EZ_CAP_DIRECT_INTERNET)
#define EZ_SET_CAP_DIRECT_INTERNET(__cap) (__cap |= EZ_CAP_DIRECT_INTERNET)
#define EZ_CLEAR_CAP_DIRECT_INTERNET(__cap) (__cap &= 0xFFFFBFFF)

#endif
#define EZ_DROP_GROUP_DATA_BAND24G			0
#define EZ_DROP_GROUP_DATA_BAND5G			1


#define EZ_TIMER_INIT(__ad, __data, __time_var, __time_flg, __time_fn) \
	do{ \
	        RTMPInitTimer((__ad), (__time_var), GET_TIMER_FUNCTION(__time_fn), (__data), FALSE); \
	    	(__time_flg) = FALSE; \
	}while(0)

#define EZ_CANCEL_TIMER(__time_var, __time_flg) \
	do{ \
			unsigned char __cancelled; \
	        RTMPCancelTimer(__time_var, &__cancelled); \
	    	(__time_flg) = FALSE; \
	}while(0)

#define EZ_RELEASE_TIMER(__time_var, __time_flg) \
	do{ \
			unsigned char __cancelled; \
	        RTMPReleaseTimer(__time_var, &__cancelled); \
	    	(__time_flg) = FALSE; \
	}while(0)




typedef struct web_conf_info_s{
	unsigned char data_len;
	char data[250];	
} web_conf_info_t;


typedef struct GNU_PACKED ez_custom_data_cmd_s {
	UINT8 data_len;
	UINT8 data_body[0];
}ez_custom_data_cmd_t, *p_ez_custom_data_cmd_t;

typedef enum enum_group_merge_action
{
	EXIT_SWITCH_NOT_GROUP_MERGE,
	TERMINATE_LOOP_MULTIPLE_AP_FOUND,
	TERMINATE_LOOP_TARGET_AP_FOUND,
	CONTINUE_LOOP_TARGET_AP_FOUND,
	CONTINUE_LOOP
}enum_group_merge_action_t;


enum EZ_CONN_ACTION
{
	EZ_ALLOW_ALL,
	EZ_DISALLOW_ALL,
	EZ_ADD_DISALLOW,
	EZ_ADD_ALLOW,
	EZ_DISALLOW_ALL_ALLOW_ME,
	EZ_ALLOW_ALL_TIMEOUT,
	EZ_ENQUEUE_PERMISSION,
	EZ_DEQUEUE_PERMISSION,
};

extern int EzDebugLevel;
#define EZ_DEBUG(__debug_cat, __debug_sub_cat, __debug_level, __fmt) \
do{ \
	if (__debug_level <= EzDebugLevel) { \
			printk __fmt;\
	}	\
}while(0)
typedef struct ez_timer_s{
	RALINK_TIMER_STRUCT ez_timer;
	BOOLEAN ez_timer_running;
}ez_timer_t;


typedef struct ez_driver_params_s{
	void *ez_ad; 
	void *ezdev;
	unsigned int group_id_len;
	unsigned int ez_group_id_len;	//for localy maintain EzGroupID
	unsigned int gen_group_id_len;  //for localy maintain EzGenGroupID
	unsigned char *group_id;
	unsigned char *ez_group_id;		//for localy maintain EzGroupID
	unsigned char *gen_group_id;	//for localy maintain EzGenGroupID	
	char ez_api_mode;

	//!timers
	ez_timer_t ez_scan_timer;
//	ez_timer_t ez_stop_scan_timer;
	ez_timer_t ez_scan_pause_timer;
	ez_timer_t ez_group_merge_timer;
	ez_timer_t ez_loop_chk_timer;
	ez_timer_t ez_connect_wait_timer;
	BOOLEAN do_not_restart_interfaces;
	BOOLEAN ez_wps_reconnect;
	BOOLEAN need_tx_satus;
	BOOLEAN ez_scan;
	BOOLEAN scan_one_channel;
	UCHAR ez_wps_bssid[MAC_ADDR_LEN];
	ULONG	ez_wps_reconnect_timestamp;
	
	unsigned int open_group_id_len;
	unsigned char open_group_id[OPEN_GROUP_MAX_LEN];
	
	unsigned char default_ssid[MAX_LEN_OF_SSID];
	unsigned char default_ssid_len;
	unsigned char default_pmk[EZ_PMK_LEN];
	unsigned char default_pmk_valid;
	int rssi_threshold;
#ifdef EZ_REGROUP_SUPPORT
	BOOLEAN en_regrp_supp;
	UINT8 regrp_mode; // rename to avoid confusion with regrp supp
	NDIS_SPIN_LOCK	regrp_mode_lock;
	RALINK_TIMER_STRUCT regrp_mode_tmr; // regrp_mode_exit_tmr
	RALINK_TIMER_STRUCT regrp_unblock_mode_tmr; // regrp_unblock_mode_exit_tmr
	UINT32 regrp_mode_time; // time to remain in this mode
	unsigned char regrp_mode_tmr_running;
	unsigned char regrp_unblock_mode_tmr_running; // time to remain in this mode
	UINT32 regrp_unblock_mode_time;
	UCHAR ap_entry_count;
	struct _drvr_cand_list ap_list[MAX_AP_CANDIDATES];
	BOOLEAN regrp_triggrd;
#endif
	UCHAR enable_easy_setup;
	ULONG partial_scan_time_stamp;
	BOOLEAN bPartialScanRunning;
	unsigned char default_group_data_band;
	BOOLEAN ez_started;
#ifdef IF_UP_DOWN
	UINT8 ez_intf_count_config_ap;
	UINT8 ez_intf_count_config_cli;
	UINT8 ez_apcli_peer_ap_config_cnt;
#endif
} ez_driver_params_t;

#ifdef EZ_REGROUP_SUPPORT

typedef struct GNU_PACKED _ntw_info {
	BOOLEAN Non_MAN;
	UINT8 ssid[32];
	CHAR rssi; // unsigned?
	UINT8 bssid[MAC_ADDR_LEN];
	unsigned char internet_status;
	UINT8 nw_wt[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	//BOOLEAN processed;
	// channel ??
}ntw_info,*pntw_info;

typedef struct vr_ap_info {
	//UINT8 mode;  // singleband, dualBand, triband etc.
	UINT8 ssid_len;
	UINT8 ssid[33];
	UINT8 intf_prefix[8];
	UINT8 mac_addr[MAC_ADDR_LEN];
	UINT8 wdev_id;//! a non zero value means a virtual repeater is created on wdev_index 
	//UINT32 ip_addr;
	//INT8	rssi;
	/*Add More*/
} vr_ap_info_struct, *p_vr_ap_info_struct;

typedef struct _node_num_wt
{
	EZ_NODE_NUMBER node_number;
	UINT8 network_wt[NETWORK_WEIGHT_LEN];
}node_num_wt, *p_node_num_wt;


typedef struct GNU_PACKED _regrp_ap_info_struct {
	UINT8 valid;
	BOOLEAN Non_MAN;
	//UINT8 ssid[32];
	//UINT8 ssid_len;
	INT32 avg_rssi; // unsigned?
	INT32 rssi_sum;
	CHAR last_rssi;
	UINT8 bssid[MAC_ADDR_LEN];
	unsigned char internet_status;
	UINT8 nw_wt[NETWORK_WEIGHT_LEN];
	EZ_NODE_NUMBER node_number;
	ULONG last_rx_time;
	INT32 rx_cnt;
	//BOOLEAN processed;
	// channel ??
}regrp_ap_info_struct,*p_regrp_ap_info_struct;

typedef struct GNU_PACKED _apcli_info{
	BOOLEAN is_enabled;
	BOOLEAN is_connected;
	unsigned char internet_status;
	//BOOLEAN is_duplicate_link;
	struct _ntw_info ntw_info;
} apcli_info, *papcli_info;

#endif


//#include "ez_hooks_proto.h"


//#####################################################
void ez_send_unicast_deauth(void *ad_obj, UCHAR *peer_addr);

void ez_wait_for_connection_allow_timeout(
		IN PVOID SystemSpecific1,
		IN PVOID FunctionContext,
		IN PVOID SystemSpecific2,
		IN PVOID SystemSpecific3);
VOID ez_scan_pause_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ez_group_merge_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);
VOID ez_loop_chk_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

VOID ez_scan_timeout(
	IN PVOID SystemSpecific1,
	IN PVOID FunctionContext,
	IN PVOID SystemSpecific2,
	IN PVOID SystemSpecific3);

#ifdef EZ_REGROUP_SUPPORT
regrp_ap_info_struct * ez_add_regrp_ap(regrp_ap_info_struct *p_ap_info_list);
regrp_ap_info_struct * ez_find_regrp_ap_by_bssid(regrp_ap_info_struct *p_ap_info_list, UINT8 * bssid);
#endif


#endif /* __EZ_CMM_H__ */
#endif
