/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/


#ifndef _UNION_IC_INIT_H_
#define _UNION_IC_INIT_H_
#include "common/xpon_global.h"
#include "common/drv_types.h"
#include "epon/epon.h"
#include "epon/epon_act.h"
#include "epon/epon_dev.h"
#include "common/union_ic_def.h"

#include <ecnt_hook/ecnt_hook_qdma.h>

typedef int (*union_action_func)(const void *in, void *out);
typedef struct ic_func_tbl_s{
	int func_id;
	union_action_func func;
}ic_func_tbl_t, *ic_func_tbl_ptr;

extern ic_func_tbl_t en7580_action[];
extern ic_func_tbl_t an7581_action[];

extern reg_check_t en7580_epon_reg[];
extern reg_check_t an7581_epon_reg[];
extern reg_ind_check_t an7581_indirect_reg[];

/*********************************Union IC Define*******************************************/
#define US_KEY_CHANGE_INT		(1<<29)
#define DS_KEY_MISS_INT			(1<<28)
#define DS_KEY_CHANGE_INT		(1<<27)

#define EPON_Q_THRESHLD_RWCMD						(1<<31)
#define EPON_Q_THRESHLD_VALUE_SHIFT					(8)
#define EPON_Q_THRESHLD_VALUE_MASK					(0xFFFF)
#define EPON_Q_THRESHLD_TYPE_SHIFT					(6)
#define EPON_Q_THRESHLD_TYPE_MASK					(0x3)
#define EPON_Q_THRESHLD_QUEUE_SHIFT					(0)
#define EPON_Q_THRESHLD_QUEUE_MASK					(0x7)

#define EPON_10G_MAX_KEY_NUM 	3
#define EPON_THURNING_KEY_LEN 	3
#define EPON_READ_KEY 			0
#define EPON_WRITE_KEY 			1
#define DPOE_MAX_KEY_NUM 		4

#define SCU_RESET_BIT				(0x01<<31)
#define EPON_LOGIC_RESET_BIT		(0x01<<10)
#define ONUTYPE_MASK 				(0x3)
#define COMBOPON_MASK 				(0x4)
#define BBF247_MASK 				(0x8)
#define ONUMODE_MASK 				(0xF0)

void dump_pon_type_mode_info(PON_OnuType_t onu_type, XMCSIF_WanDetectionMode_t onu_mode);


/**********************************EN7580 Function Declare*********************************************/
int en7580_epon_check_mac_tx_mbi_stop(const void *in, void *out);
int en7580_epon_check_mac_tx_mpi_stop(const void *in, void *out);
int en7580_epon_set_queue_threshold_cfg(const void *in, void *out);
int en7580_epon_get_queue_threshold_cfg(const void *in, void *out);
int en7580_epon_get_wan_cfg(const void *in, void *out);
int en7580_epon_qos_tx_rate_meter_cfg(const void *in, void *out);
int en7580_epon_set_mac_report_qsizeadjs(const void *in, void *out);
int en7580_epon_set_mac_report_qsizeadjs_for_fec(const void *in, void *out);
int en7580_epon_set_default_report_method(const void *in, void *out);
int en7580_epon_set_ipg(const void *in, void *out);
int en7580_epon_set_mpcp_tmout_intvl(const void *in, void *out);
int en7580_epon_rtt_adjust(const void *in, void *out);
int en7580_epon_set_llid_report_bitmap(const void *in, void *out);
int en7580_epon_get_llid_report_bitmap(const void *in, void *out);
int en7580_epon_init_discovery_status(const void *in, void *out);
int en7580_epon_get_mac_reg_address(const void *in, void *out);
int en7580_epon_set_llid_key(const void *in, void *out);
int en7580_epon_get_llid_key(const void *in, void *out);
int en7580_epon_set_llid_tx_fec(const void *in, void *out);
int en7580_epon_get_llid_tx_fec(const void *in, void *out);
int en7580_epon_set_llid_thrshld_num(const void *in, void *out);
int en7580_epon_get_llid_thrshld_num(const void *in, void *out);
int en7580_epon_set_tx_buff_usage(const void *in, void *out);
int en7580_epon_set_10G_llid_key(const void *in, void *out);
int en7580_epon_get_10G_llid_key(const void *in, void *out);
int en7580_pon_mac_scu_reset(const void *in, void *out);
int en7580_epon_mac_logic_reset(const void *in, void *out);
int en7580_epon_get_debug_statistic_count(const void *in, void *out);
int en7580_epon_get_register_state(const void *in, void *out);
int en7580_epon_set_sniffer(const void *in, void *out);
int en7580_epon_set_static_report(const void *in, void *out);
int en7580_pon_get_onu_mode_type(const void *in, void *out);
int en7580_epon_set_dygasp_hw_en(const void *in, void *out);
int en7580_epon_get_dygasp_hw_en(const void *in, void *out);
int en7580_epon_set_dygasp_num(const void *in, void *out);
int en7580_epon_get_dygasp_num(const void *in, void *out);


/**********************************AN7581 Function Declare*********************************************/
int an7581_epon_check_mac_tx_mbi_stop(const void *in, void *out);
int an7581_epon_check_mac_tx_mpi_stop(const void *in, void *out);
int an7581_epon_get_olt_mac_address(const void *in, void *out);
int an7581_epon_ext_int_enable(const void *in, void *out);
int an7581_epon_set_queue_threshold_cfg(const void *in, void *out);
int an7581_epon_get_queue_threshold_cfg(const void *in, void *out);
int an7581_epon_get_wan_cfg(const void *in, void *out);
int an7581_epon_qos_tx_rate_meter_cfg(const void *in, void *out);
int an7581_epon_set_mac_report_qsizeadjs(const void *in, void *out);
int an7581_epon_set_mac_report_qsizeadjs_for_fec(const void *in, void *out);
int an7581_epon_set_default_report_method(const void *in, void *out);
int an7581_epon_set_ipg(const void *in, void *out);
int an7581_epon_set_mpcp_tmout_intvl(const void *in, void *out);
int an7581_epon_rtt_adjust(const void *in, void *out);
int an7581_epon_set_llid_report_bitmap(const void *in, void *out);
int an7581_epon_get_llid_report_bitmap(const void *in, void *out);
int an7581_epon_init_discovery_status(const void *in, void *out);
int an7581_epon_downstream_key_change_handler(const void *in, void *out);
int an7581_epon_downstream_key_miss_handler(const void *in, void *out);
int an7581_epon_upstream_key_change_handler(const void *in, void *out);
int an7581_epon_set_llid_key(const void *in, void *out);
int an7581_epon_get_llid_key(const void *in, void *out);
int an7581_epon_set_llid_tx_fec(const void *in, void *out);
int an7581_epon_get_llid_tx_fec(const void *in, void *out);
int an7581_epon_set_llid_thrshld_num(const void *in, void *out);
int an7581_epon_get_llid_thrshld_num(const void *in, void *out);
int an7581_epon_set_tx_buff_usage(const void *in, void *out);
int an7581_epon_set_10G_llid_key(const void *in, void *out);
int an7581_epon_get_10G_llid_key(const void *in, void *out);
int an7581_epon_set_dpoe_decrypt_llid_key(const void *in, void *out);
int an7581_pon_mac_scu_reset(const void *in, void *out);
int an7581_epon_mac_logic_reset(const void *in, void *out);
int an7581_epon_get_debug_statistic_count(const void *in, void *out);
int an7581_epon_get_register_state(const void *in, void *out);
int an7581_epon_set_sniffer(const void *in, void *out);
int an7581_epon_set_static_report(const void *in, void *out);
int an7581_epon_set_band_utilization(const void *in, void *out);
int an7581_epon_tx_rate_analyze(const void *in, void *out);
int an7581_pon_get_onu_mode_type(const void *in, void *out);
int an7581_epon_set_dygasp_hw_en(const void *in, void *out);
int an7581_epon_get_dygasp_hw_en(const void *in, void *out);
int an7581_epon_set_dygasp_num(const void *in, void *out);
int an7581_epon_get_dygasp_num(const void *in, void *out);


#endif

