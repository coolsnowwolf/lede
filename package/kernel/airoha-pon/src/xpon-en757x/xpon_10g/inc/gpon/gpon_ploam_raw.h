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
#ifndef _PLOAM_RAW_H
#define _PLOAM_RAW_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/types.h>
#include <linux/timer.h>
#include "gpon_const.h"

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define PLOAM_DEST_ID_HIGHER					0
#define PLOAM_DEST_ID_LOWER						1
#define PLOAM_XGPON_LINE_RATE					0
#define PLOAM_XGSPON_LINE_RATE					1
#define PLOAM_UNASSIGNED_ADDR					(0x3FF)
#define PLOAM_BROADCAST_ADDR					(0x3FF)
#define PLOAM_XGSPON_BROADCAST_ADDR				(0x3FE) /*1022 used only in XGSPON*/
#define PLOAM_XGSPON_BROADCAST_ADDR_NOKIA		(0x7FF)//nokia broadcast

#define PLOAM_KEY_FRAGMENT_LEN					32
#define PLOAM_KEY_FRAGMENT_NUM					0   /*the first fragment is number 0*/
#define PLOAM_NEW_ENCRYPTED_KEY_LEN				16
#define PLOAM_EXISTING_HASH_RESULT_LEN			17
#define PLOAM_EXISTING_HASH_KEY_LEN				16
#define PLOAM_MSG_MIC_LEN						8
#define PLOAM_DELIMITER_PATTERN_LENS			8
#define PLOAM_PREAMBLE_PATTERN_LENS				8



#define PLOAM_DOWN_MSG_PROFILE					(0x01)
#define PLOAM_DOWN_MSG_ASSIGN_ONUID				(0x03)
#define PLOAM_DOWN_MSG_RANGING_TIME				(0x04)
#define PLOAM_DOWN_MSG_DEACTIVATE_ONUID			(0x05)
#define PLOAM_DOWN_MSG_DISABLE_SERIAL_NUM		(0x06)
#define PLOAM_DOWN_MSG_REQUEST_REGISTRATION		(0x09)
#define PLOAM_DOWN_MSG_ASSIGN_ALLOCID 			(0x0A)
#define PLOAM_DOWN_MSG_KEY_CONTROL				(0x0D)
#define PLOAM_DOWN_MSG_SLEEP_ALLOW				(0x12)

/**************NG2 ONLY*************************/
#define PLOAM_DOWN_MSG_CALIBRATION_REQUEST		(0x13)
#define PLOAM_DOWN_MSG_ADJUST_TX_WAVELENGTH		(0x14)
#define PLOAM_DOWN_MSG_TUNING_CONTROL 			(0x15)
#define PLOAM_DOWN_MSG_SYSTEM_PROFILE			(0x17)
#define PLOAM_DOWN_MSG_CHANNEL_PROFILE			(0x18)
#define PLOAM_DOWN_MSG_PROTECTION_CONTROL		(0x19)
#define PLOAM_DOWN_MSG_CHANGE_POWER_LEVEL		(0x1A)
#define PLOAM_DOWN_MSG_POWER_CONSUM_INQUIRE		(0x1B)
#define PLOAM_DOWN_MSG_RATE_CONTROL				(0x1C)
/**************NG2 ONLY*************************/
#define PLOAM_DOWN_MSG_REBOOT_ONU				(0x1D)
#define PLOAM_DOWN_MAX_TYPE						(0x1E)
	
#define PLOAM_UP_MSG_SERIAL_NUMBER 				(0x01)
#define PLOAM_UP_MSG_REGISTRATION				(0x02)
#define PLOAM_UP_MSG_KEY_REPORT					(0x05)
#define PLOAM_UP_MSG_ACKNOWLEDGE				(0x09)
#define PLOAM_UP_MSG_SLEEP_REQUEST		        (0x10)


/**************NG2 ONLY*************************/
#define PLOAM_UP_TUNING_RESPONSE		        (0x1A)
#define PLOAM_UP_POWER_COMSUM_REPORT	        (0x1B)
#define PLOAM_UP_RATE_RESPONSE					(0x1C)
/**************NG2 ONLY*************************/
#define PLOAM_UP_MAX_TYPE					    (0x1D)


/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
/* XGPON PLOAM General message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar msg_content[36] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN	
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_General_T ;

/***************************************
 Downstream PLOAM Message
***************************************/
/* Profile message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
#ifdef __BIG_ENDIAN		
		unchar prof_version			:4 ;
		unchar resv1				:1 ;
		unchar line_rate			:1 ;
		unchar prof_index			:2 ;
#else
		unchar prof_index			:2 ;
		unchar line_rate			:1 ;
		unchar resv1				:1 ;
		unchar prof_version 		:4 ;
#endif /* __BIG_ENDIAN */
#ifdef __BIG_ENDIAN
		unchar resv2				:6 ;
		unchar cross				:1 ;
		unchar fec					:1 ;
#else
		unchar fec					:1 ;
		unchar cross				:1 ;
		unchar resv2				:6 ;
#endif /* __BIG_ENDIAN */
#ifdef __BIG_ENDIAN
		unchar resv3				:4 ;
		unchar delimiter_lens		:4 ;
#else
		unchar delimiter_lens		:4 ;		
		unchar resv3				:4 ;
#endif /* __BIG_ENDIAN */
		unchar delimiter[PLOAM_DELIMITER_PATTERN_LENS] ;
#ifdef __BIG_ENDIAN
		unchar resv4				:4 ;
		unchar preamble_lens		:4 ;
#else
		unchar preamble_lens		:4 ;
		unchar resv4				:4 ;
#endif /* __BIG_ENDIAN */
		unchar preamble_repeat_cnt ;
		unchar preamble[PLOAM_PREAMBLE_PATTERN_LENS] ;
		unchar pon_tag[GPON_TAG_LENS] ;
		unchar ds_pon_id[4];
		unchar padding[3] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv5[3] ;
#ifdef __BIG_ENDIAN		
		unchar resv6				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv6				:7 ;
#endif /* __BIG_ENDIAN */		
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Profile_T ;


/* Assign_ONU_ID message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;	
#ifdef __BIG_ENDIAN
		unchar resv1				:6 ;
		unchar onu_id_m 			:2 ;
#else
		unchar onu_id_m 			:2 ;
		unchar resv1				:6 ;
#endif /* __BIG_ENDIAN */
		unchar onu_id_l ;
		unchar sn[GPON_SN_LENS] ;
		unchar padding[26] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv2[3] ;
#ifdef __BIG_ENDIAN
		unchar resv3				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv3				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Assign_OnuID_T ;


/* Ranging_Time message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
#ifdef __BIG_ENDIAN
		unchar resv1				:6 ;		
		unchar eqd_adjust			:1 ;
		unchar eqd_mode				:1 ;
#else
		unchar eqd_mode 			:1 ;
		unchar eqd_adjust			:1 ;		
		unchar resv1				:6 ;
#endif /* __BIG_ENDIAN */		
		unchar eqd_value[4] ;
		unchar padding[31] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv2[3] ;
#ifdef __BIG_ENDIAN
		unchar resv3				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv3				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Ranging_Time_T;

/* Deactivate_ONU_ID message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar padding[36] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;		
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */		
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Deactivate_OnuID_T;

/* Disable_Serial_Number message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar mode ;
		unchar sn[GPON_SN_LENS] ;
		unchar padding[27] ;	
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Disable_SN_T;

/* Request_Registration message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar padding[36] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Request_Registration_T;


/* Assign_Alloc-ID message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
#ifdef __BIG_ENDIAN
		unchar alloc_id_resv 		:2;
		unchar alloc_id_m 			:6;
#else
        unchar alloc_id_m           :6;
        unchar alloc_id_resv        :2;        
#endif
		unchar alloc_id_l ;
		unchar alloc_id_type ;
		unchar padding[33] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Assign_AllocID_T ;

/* Key_Control message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar resv1 ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar control 				:1 ;
#else
		unchar control				:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
#ifdef __BIG_ENDIAN
		unchar resv3				:6 ;
		unchar key_index 			:2 ;
#else
		unchar key_index			:2 ;
		unchar resv3				:6 ;
#endif /* __BIG_ENDIAN */
		unchar key_lens ;
		unchar padding[32] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv4[3] ;
#ifdef __BIG_ENDIAN
		unchar resv5				:7 ;
		unchar mic_status			:1 ;
#else	
		unchar mic_status			:1 ;
		unchar resv5				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Key_Control_T ;

/* Sleep_Allow message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;		
#ifdef __BIG_ENDIAN
		unchar resv1				:7 ;
		unchar type 				:1 ;
#else
		unchar type 				:1 ;
		unchar resv1				:7 ;
#endif /* __BIG_ENDIAN */
		unchar padding[35] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv2[3] ;		
#ifdef __BIG_ENDIAN
		unchar resv3				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv3				:7 ;		
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Sleep_Allow_T ;

/* Reboot ONU message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar sn[GPON_SN_LENS] ;
		unchar reboot_depth ;
		unchar reboot_image ;
		unchar onu_state ;		
#ifdef __BIG_ENDIAN
		unchar resv1				:6 ;
		unchar flags 				:2;
#else
		unchar flags 				:2;
		unchar resv1				:6 ;
#endif /* __BIG_ENDIAN */
		unchar padding[24] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv2[3] ;
#ifdef __BIG_ENDIAN
		unchar resv3				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv3				:7 ;		
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
}PLOAM_RAW_Reboot_ONU_T ;

typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar operCode ;
		unchar scheduledSFC[2] ;
		unchar rollbackFlag ;
		unchar targetDnPonID[4] ;
		unchar targetUsPonID[4] ;
		unchar calibFlag ;
		unchar padding[23];
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv3[3] ;
#ifdef __BIG_ENDIAN
		unchar resv4				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv4				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Tuning_Ctrl_T;

/* System Profile message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar ng2_sys_id[3] ;
#ifdef __BIG_ENDIAN
		unchar sys_prof_ver			:4 ;
		unchar resv1 				:4 ;
#else
		unchar resv1 				:4 ;
		unchar sys_prof_ver			:4 ;
#endif /* __BIG_ENDIAN */
		unchar us_op_wave_bands ;
		unchar twdm_channel_cnt ;
		unchar channel_spacing ;
		unchar us_mse ;
		unchar fsr[2] ;
		unchar twdm_amcc_ctl ;
		unchar loose_calib_bund ;
		unchar resv2[8] ;
		unchar padding[16] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv3[3] ;
#ifdef __BIG_ENDIAN
		unchar resv4				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv4				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_System_Profile_T ;


/* Channel Profile message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
#ifdef __BIG_ENDIAN
		unchar prof_index			:4 ;
		unchar resv1				:1 ;
		unchar channel_indicator	:1 ;
		unchar dn_void_incator		:1 ;
		unchar us_void_incator 		:1 ;
#else
		unchar us_void_incator 		:1 ;
		unchar dn_void_incator		:1 ;
		unchar channel_indicator	:1 ;
		unchar resv1				:1 ;
		unchar prof_index			:4 ;
#endif /* __BIG_ENDIAN */

#ifdef __BIG_ENDIAN
		unchar prof_version 		:4 ;
		unchar resv2				:4 ;
#else
		unchar resv2				:4 ;
		unchar prof_version 		:4 ;
#endif /* __BIG_ENDIAN */

		unchar pon_id[4] ;
		unchar ds_freq_offset ;
#ifdef __BIG_ENDIAN
		unchar resv3				:2 ;
		unchar dn_line_rate 		:1 ;
		unchar dn_fec				:1 ;
		unchar dn_line_code			:4 ;
#else
		unchar dn_line_code			:4 ;
		unchar dn_fec				:1 ;
		unchar dn_line_rate 		:1 ;
		unchar resv3				:2 ;
#endif /* __BIG_ENDIAN */

		unchar channel_partition ;
		unchar default_resp_channel[4] ;
#ifdef __BIG_ENDIAN
		unchar resv4				:6 ;
		unchar sn_grant_in_band 	:1 ;
		unchar sn_grant_amcc		:1 ;
#else
		unchar sn_grant_amcc		:1 ;
		unchar sn_grant_in_band 	:1 ;
		unchar resv4				:6 ;
#endif /* __BIG_ENDIAN */
		unchar amcc_wind_spec[4] ;
#ifdef __BIG_ENDIAN
		unchar resv5				:4 ;
		unchar uwlch_id 			:4 ;
#else
		unchar uwlch_id 			:4 ;
		unchar resv5				:4 ;
#endif /* __BIG_ENDIAN */
		unchar us_freq[4] ;
		unchar optical_link_type ;
		unchar us_rate ;
		unchar default_onu_atten ;
		unchar resp_thrd ;
		unchar cloned_config ;
		unchar padding[8] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv6[3] ;
#ifdef __BIG_ENDIAN
		unchar resv7				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv7				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Channel_Profile_T ;


/* Protection_Control message */
typedef union {
	struct {
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar protect_ds_pon[4] ;
		unchar protect_us_pon[4] ;
		unchar enable_flag ;
		unchar padding[27] ;
		unchar msg_mic[PLOAM_MSG_MIC_LEN] ;
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar mic_status			:1 ;
#else
		unchar mic_status			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
	} raw ;
	uint value[13] ;
} PLOAM_RAW_Protect_Ctl_T ;
/***************************************
 Upstream PLOAM Message
***************************************/
/* Serial_Number_ONU message */
typedef union {
	struct {
		unchar resv1[3] ;		
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar vender_id[4] ;
		unchar vssn[4] ;
		unchar random_delay[4] ;
		unchar correl_tag[2] ;
		unchar cur_ds_pon_id[4] ;	
		unchar cur_us_pon_id[4] ;	
		unchar calibra_rec_sta[4] ;
		unchar tuning_gran ;
		unchar step_tuning_time ;
		unchar us_line_rate_cap ;
		unchar attenuation ;
		unchar power_level_cap ;
		unchar act_debug_info ;
		unchar padding[4] ;
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Serial_Number_T ;

/* Registration message */
typedef union {
	struct {
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar registration_id[GPON_REG_ID_LENS] ;	
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Registration_T ;

/* Key_Report message */
typedef union {
	struct {
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;		
#ifdef __BIG_ENDIAN
		unchar resv3				:7 ;
		unchar report_type			:1 ;
#else
		unchar report_type			:1 ;
		unchar resv3				:7 ;
#endif /* __BIG_ENDIAN */		
#ifdef __BIG_ENDIAN
		unchar resv4				:6 ;
		unchar report_key_index		:2 ;
#else
		unchar report_key_index 	:2 ;
		unchar resv4				:6 ;		
#endif /* __BIG_ENDIAN */		
#ifdef __BIG_ENDIAN
		unchar resv5				:5 ;
		unchar fragment_num			:3 ;
#else
		unchar fragment_num 		:3 ;
		unchar resv5				:5 ;		
#endif /* __BIG_ENDIAN */
		unchar resv6 ;
		unchar key_fragment[32] ;	
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Key_Report_T ;

/* Acknowledgment message */
typedef union {
	struct {
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar completion_code ;
		unchar padding[35] ;	
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Acknowledgment_T ;

/* Sleep_Request message */
typedef union {
	struct {
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar activity_level ;
		unchar padding[35] ;	
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Sleep_Request_T ;

/* Tuning_Response message */
typedef union {
	struct {
		unchar resv1[3] ;
#ifdef __BIG_ENDIAN
		unchar resv2				:7 ;
		unchar key_index			:1 ;
#else
		unchar key_index			:1 ;
		unchar resv2				:7 ;
#endif /* __BIG_ENDIAN */
		unchar dest_id[2] ;
		unchar msg_id ;
		unchar seq_no ;
		unchar operCode ;
		unchar respCode[2] ;
		unchar vendorID[4] ;
		unchar vssn[4] ;
		unchar corTag[2] ;
		unchar ponID[4] ;
		unchar uwlchID ;
		unchar calRecordSt[8] ;
		unchar tuningGrau ;
		unchar stepTuningTime ;
		unchar usLineRateCap ;
		unchar attenuation ;
		unchar powerLevelcap ;
		unchar padding[5] ;
	} raw ;
	uint value[11] ;
} PLOAM_RAW_Tuning_Resp_T ;
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/

#endif /*_PLOAM_RAW_H*/
