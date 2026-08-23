#ifndef ECNT_GLOBAL_WADT_H
#define ECNT_GLOBAL_WADT_H
/***************************************************************
Copyright Statement:

This software/firmware and related documentation (Airoha Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to Airoha (HK) Limited (Airoha) and/or 
its licensors. Without the prior written permission of Airoha and/or its licensors, 
any reproduction, modification, use or disclosure of Airoha Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

Airoha (HK) Limited  Airoha. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (Airoha SOFTWARE) RECEIVED FROM Airoha 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. Airoha EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES Airoha PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE Airoha SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN Airoha SOFTWARE.

Airoha SHALL NOT BE RESPONSIBLE FOR ANY Airoha SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
Airoha'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE Airoha 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT Airoha'S SOLE OPTION, TO 
REVISE OR REPLACE THE Airoha SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO Airoha FOR SUCH 
Airoha SOFTWARE.
***************************************************************/
#include "../uapi/common/ecnt_global_wadt.h"
#if 0
/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/netlink.h>
/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define MAX_PKT_LEN 1024
#define NETLINK_WADT 27
#define NETLINK_WADT_5G 28
#define WADT_PID 100
#define MAX_REASON_LEN 25
#ifndef IFNAMSIZ
#define IFNAMSIZ 16
#endif
#ifndef MAC_ADDR_LEN
#define MAC_ADDR_LEN				6
#endif
#ifndef LEN_KEY_MIC
#define LEN_KEY_MIC				24
#endif
#ifndef LEN_PMSG
#define LEN_PMSG				128
#endif
/****    events struct    ****/
typedef struct _wadt_drv_evt_sta_assoc_success{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned int AKMMap;
	char ifname[IFNAMSIZ];
} wadt_drv_evt_sta_assoc_success;

typedef struct _wadt_drv_evt_sta_assoc_fail{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	int reason;
	char ifname[IFNAMSIZ];
	unsigned char is_mic_dif;
	unsigned char mic[LEN_KEY_MIC];
	unsigned char recv_mic[LEN_KEY_MIC];
	unsigned char pMsg[LEN_PMSG];
	int eapol_len;
} wadt_drv_evt_sta_assoc_fail;

typedef struct _wadt_drv_evt_sta_assoc_fail_for_me{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
} wadt_drv_evt_sta_assoc_fail_for_me;

typedef struct _wadt_drv_evt_block_sta_assoc{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
} wadt_drv_evt_block_sta_assoc;

typedef struct _wadt_drv_evt_sta_disassoc_deauth{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	int reason;
	char ifname[IFNAMSIZ];
} wadt_drv_evt_sta_disassoc_deauth;

typedef struct _wadt_drv_evt_sta_assoc_fail_full_entry{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
	unsigned char StaCount;
	unsigned char MaxStaNum;
} wadt_drv_evt_sta_assoc_fail_full_entry;

typedef struct _wadt_drv_evt_sta_pkt_counter{
	char ifname[IFNAMSIZ];
} wadt_drv_evt_sta_pkt_counter;

typedef struct _wadt_drv_evt_stainfo{
	char ifname[IFNAMSIZ];
} wadt_drv_evt_stainfo;

typedef struct _wadt_drv_evt_noise_floor{
	char ifname[IFNAMSIZ];
} wadt_drv_evt_noise_floor;

typedef struct _wadt_drv_evt_stat{
	char ifname[IFNAMSIZ];
} wadt_drv_evt_stat;

typedef struct _wadt_drv_evt_bcn_not_idle{
	unsigned char bssid[MAC_ADDR_LEN];
	unsigned int bcn_state;
	char ifname[IFNAMSIZ];
} wadt_drv_evt_bcn_not_idle;

typedef struct _wadt_drv_evt_sw_queue_full{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
} wadt_drv_evt_sw_queue_full;

typedef struct _wadt_drv_evt_pre_reset{
	unsigned int band_idx;
} wadt_drv_evt_pre_reset;

typedef struct _wadt_drv_evt_interface_up{
	char ifname[IFNAMSIZ];
} wadt_drv_evt_interface_up;

typedef struct _wadt_drv_evt_ch_switch_fail{
	unsigned int band_idx;
} wadt_drv_evt_ch_switch_fail;

typedef struct _wadt_drv_evt_ap_send_disassoc{
	unsigned char sta_mac[MAC_ADDR_LEN];
	unsigned char bssid[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
} wadt_drv_evt_ap_send_disassoc;

typedef struct _wadt_drv_evt_wps_m2d {
	unsigned char enrollee[MAC_ADDR_LEN];
	unsigned char register_mac[MAC_ADDR_LEN];
	char ifname[IFNAMSIZ];
} wadt_drv_evt_wps_m2d;

typedef struct _wadt_drv_evt_sys_mem_info {

} wadt_drv_evt_sys_mem_info;

typedef struct _wadt_drv_evt_pse_flush {
	char ifname[IFNAMSIZ];
} wadt_drv_evt_pse_flush;

typedef struct _wadt_drv_evt_high_false_cca {
	unsigned int false_cca;
	unsigned char high_cca_state;
	char ifname[IFNAMSIZ];
} wadt_drv_evt_high_false_cca;

typedef struct _wadt_evt_tar_log_file {

} wadt_evt_tar_log_file;

typedef struct _wadt_drv_evt_fw_cmd_timeout {
	unsigned char cmd_type;
	unsigned char ext_cmd_type;
	unsigned int timeout;
	unsigned int band_idx;
} wadt_drv_evt_fw_cmd_timeout;

typedef struct _wadt_sys_evt_basic_info {

} wadt_sys_evt_basic_info;

typedef struct _wadt_drv_evt_wlan_basic_info {

} wadt_drv_evt_wlan_basic_info;

typedef struct _wadt_drv_evt_edcca_trigger {
	unsigned int ed_cnt;
	unsigned int ed_block_tx_th;
	unsigned char is_stoped;
	unsigned int band_idx;
	unsigned char ed_percentage;
	unsigned char ed_occupy_th;
} wadt_drv_evt_edcca_triggered;

typedef struct _wadt_drv_evt_ser_dump{
	
} wadt_drv_evt_ser_dump;

typedef struct _wadt_drv_evt_dfs_trigger{
	char ifname[IFNAMSIZ];	
	unsigned char channel;
} wadt_drv_evt_dfs_trigger;

typedef struct _wadt_drv_evt_wifi_init_fail {
	unsigned int band_idx;
	int statuts;
	char reason[MAX_REASON_LEN];
} wadt_drv_evt_wifi_init_fail;

typedef struct _wadt_drv_evt_proc_bcn_check {

} wadt_drv_evt_proc_bcn_check;
/****    events struct end  ****/

/*Remember add handler function at wadt_evt_func_tbl(at wadt/event_handler.h)*/
typedef enum {
	WADT_DRV_STA_ASSOC_SUCCESS = 0,
	WADT_DRV_STA_ASSOC_FAIL,
	WADT_DRV_STA_ASSOC_FAIL_FOR_ME,
	WADT_DRV_BLOCK_STA_ASSOC,
	WADT_DRV_STA_DISASSOC_DEAUTH,
	WADT_DRV_STA_ASSOC_FAIL_FULL_ENTRY,
	WADT_DRV_STA_PKT_COUNTER,
	WADT_DRV_STAINFO,
	WADT_DRV_NOISE_FLOOR,
	WADT_DRV_STAT,
	WADT_DRV_BCN_NOT_IDLE,

	WADT_DRV_SW_QUEUE_FULL,
	WADT_DRV_PRE_RESET,
	WADT_DRV_INTERFACE_UP,
	WADT_DRV_AP_SEND_DISASSOC,
	WADT_DRV_CH_SWITH_FAIL,
	WADT_DRV_WPS_M2D,
	WADT_SYS_MEM_INFO,
	WADT_DRV_PSE_FLUSH,
	WADT_DRV_HIGH_FALSE_CCA,
	WADT_TAR_LOG_FILE,
	
	WADT_DRV_FW_CMD_TIMEOUT,
	WADT_SYS_BASIC_INFO,
	WADT_DRV_WLAN_BASIC_INFO,
	WADT_DRV_EDCCA_TRIGGERED,
	WADT_DRV_SER_DUMP,
	WADT_DRV_DFS_TRIGGER,
	WADT_DRV_WIFI_INIT_FAIL,
	WADT_DRV_PROC_BCN_CHECK,
	/*Do not put event behind last one*/
	WADT_LAST_ONE,
} WADT_GLOBAL_EVENT;

typedef union _wadt_event_data
{
	wadt_drv_evt_sta_assoc_success 			evt_sta_assoc_success;
	wadt_drv_evt_sta_assoc_fail 			evt_sta_assoc_fail;
	wadt_drv_evt_sta_assoc_fail_for_me 		evt_sta_assoc_fail_for_me;
	wadt_drv_evt_block_sta_assoc 			evt_block_sta_assoc;
	wadt_drv_evt_sta_disassoc_deauth 		evt_sta_disassoc_deauth;
	wadt_drv_evt_sta_assoc_fail_full_entry  evt_sta_assoc_fail_full_entry;
	wadt_drv_evt_sta_pkt_counter  			evt_sta_pkt_counter;
	wadt_drv_evt_stainfo  					evt_stainfo;
	wadt_drv_evt_noise_floor  				evt_noise_floor;
	wadt_drv_evt_stat  						evt_stat;
	wadt_drv_evt_bcn_not_idle  				evt_bcn_not_idle;
	wadt_drv_evt_sw_queue_full  			evt_sw_queue_full;
	wadt_drv_evt_pre_reset  				evt_pre_reset;
	wadt_drv_evt_interface_up  				evt_interface_up;
	wadt_drv_evt_ap_send_disassoc 			evt_ap_send_disassoc;
	wadt_drv_evt_ch_switch_fail 			evt_ch_switch_fail;
	wadt_drv_evt_wps_m2d 					evt_wps_m2d;
	wadt_drv_evt_sys_mem_info 				evt_sys_mem_info;
	wadt_drv_evt_pse_flush 					evt_pse_flush;
	wadt_drv_evt_high_false_cca				evt_high_false_cca;
	wadt_evt_tar_log_file					evt_tar_log_file;
	wadt_drv_evt_fw_cmd_timeout				evt_fw_cmd_timeout;
	wadt_sys_evt_basic_info					evt_basic_info;
	wadt_drv_evt_wlan_basic_info			evt_wlan_basic_info;
	wadt_drv_evt_edcca_triggered			evt_edcca_triggered;
	wadt_drv_evt_dfs_trigger				evt_dfs_trigger;
	wadt_drv_evt_wifi_init_fail				evt_wifi_init_fail;
	wadt_drv_evt_proc_bcn_check				evt_proc_bcn_check;	
} wadt_event_data;

typedef struct _wadt_event{
	WADT_GLOBAL_EVENT event_id;
	wadt_event_data data;
	unsigned int data_len;
} wadt_event;

typedef struct _wadt_ctx {
	unsigned char * rx_buf;
} wadt_ctx;

typedef enum {
	CRITIAL = 0,
	ERROR,
	WARN,
	DEBUG,
} WADT_LOG_LEVEL;
/************************************************************************
*               M A C R O S
*************************************************************************
*/


/************************************************************************
*               G L O B A L   V A R I A B L E
*************************************************************************
*/


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/


/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
#endif
#endif