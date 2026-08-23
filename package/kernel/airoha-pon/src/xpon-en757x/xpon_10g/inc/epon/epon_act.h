/***************************************************************
Copyright Statement:

This software/firmware and related documentation (“EcoNet Software”) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (“EcoNet”) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (“ECONET SOFTWARE”) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN “AS IS” 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER’S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER’S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _EPON_ACT_H_
#define _EPON_ACT_H_

#include "epon/epon.h"

#define TYPE_B_ENABLE							gp_epon_global_data->typeb_holdOn_enable
#define HOLD_OVER_CFG_ENABLE					gp_epon_global_data->hold_over_enable
#define TX_POWER_ENABLE							gp_epon_global_data->txPower_flag
#define EPON_ENABLE_FLAG						gp_epon_global_data->epon_enable_flag
#define EPON_SYNC_TIME							gp_epon_global_data->sync_time
#define STATIC_REPORT_ENABLE					gp_epon_global_data->static_report_enable


#define POIING_TX_STOP_STAS_NUM                 10000
/* hold over time and typeB set clear */
#define EPON_DRIVER_SET(flag,val)				((flag) = (val))
#define EPON_DRIVER_CLEAR(flag)					((flag) = FALSE)
#define EPON_DRIVER_CHECK(flag,val)				((flag == val)?TRUE:FALSE)

#define EPON_SET_LINK_STATUS(val)				(gpPonSysData->sysLinkStatus = val)

#define EPON_SET_LLID_MASK(mask)				(gp_epon_global_data->llid_bit_mask  = mask)

#define EPON_CHECK_LLID_INDEX_VALID(llid_index)		((gp_epon_global_data->llid_bit_mask & (1 << llid_index)) \
    									  			&& (llid_index < EPON_LLID_MAX_NUM))      


#define EPON_GET_LLID_BY_INDEX(llid_info,index)		(llid_info = &gp_epon_global_data->llid_entry[index])	

#define EPON_CHECK_SILENT_TIME_VALID(time)			(time > 0 && time <= gp_epon_global_data->silent_time_config)


void epon_silent_timer_expires(TIMER_FUN_PAAM data);
void epon_typeb_timer_expires(TIMER_FUN_PAAM data);
void epon_oamCnt_check_timer_expires(TIMER_FUN_PAAM data);
void start_oamCnt_ckeck_timer(void);

void epon_start(unsigned long data);
int epon_typeb_hold_on_start(void *data);
int epon_detect_los_lof_handler(void *data);
int epon_detect_phy_ready_handler(void *data);
void epon_change_state(__u8 llid_index, __u8 new_state);
int epon_hw_dying_gasp_control(__u8 flag);
int epon_get_dying_gasp_mode(__u8 *mode);
int epon_set_dying_gasp_number(__u32 num);
int epon_get_dying_gasp_number(__u32 *num);
void epon_isr_handler_init(void);
int epon_mpcp_state_init(__u8 llidIndex);
bool epon_check_current_state(pEPON_LLID_INFO_T llid_info, __u8 state);
int epon_los_state(void *data);


int epon_rcv_rgst_ack_change_state(void *param);
int epon_act_send_register_ack(void *param);
//int epon_start_mpcp_send_check(EPON_LLID_INFO_T *llid_info);
int epon_rcv_re_register_change_state(void *param);
int epon_de_register_change_state(void *param);
int epon_llid_reinit(void *param);
int epon_rcv_nack_flag_msg_process(void *param);
void xmit_dying_gasp_oam(void);


__u16 epon_get16(__u8 *cp);
__u32 epon_get32(__u8 *cp);
int epon_isr(void);


int epon_stop(void *param);
int epon_reset(void *param);

int epon_phy_ready_sw_init(void *param);
int epon_phy_ready_hw_init(void *param);

#define EPON_CHECK_CURRENT_STATE(llid_info,state) 	if(FALSE == epon_check_current_state(llid_info,state))\
    												{\
    													xpon_debug_print("%s check current false ",__FUNCTION__);\
														return EPON_NORMAL_ERROR;\
													}

#endif

