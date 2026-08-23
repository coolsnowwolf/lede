/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef DBA_MAIL_TYPE_H_
#define DBA_MAIL_TYPE_H_

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
//panjun npu modify

typedef enum {
	DBA_SET_WAIT = 1,
	DBA_SET_NO_WAIT,
	DBA_GET_WAIT,
	DBA_GET_NO_WAIT,
	DBA_OP_MAX_NUM 
} DBA_MAIL_FuncType_t;
	
typedef enum {
	DBA_MAIL_FUNCTION_SET_PRINT_DBA_EN,
	DBA_MAIL_FUNCTION_SET_PRINT_NO_IDLE_EN,
	DBA_MAIL_FUNCTION_SET_CLEAN_NO_IDLE,
	DBA_MAIL_FUNCTION_SET_DBA_TIMER_EN,
	DBA_MAIL_FUNCTION_SET_ONU_STATE,
	DBA_MAIL_FUNCTION_SET_ONU_US_FEC,
	DBA_MAIL_FUNCTION_SET_TCONT_DBA,
	DBA_MAIL_FUNCTION_SET_TCONT_STATE,
	DBA_MAIL_FUNCTION_GET_TCONT_INFO,
	DBA_MAIL_FUNCTION_SET_PLOAMU_FLAG,
	DBA_MAIL_FUNCTION_SW_BWMAP_TEST,
	DBA_MAIL_FUNCTION_SET_OLT_PHY_DATA,
	DBA_MAIL_FUNCTION_GET_OLT_PHY_DATA,
	DBA_MAIL_FUNCTION_SET_OLT_MAC_DATA,
	DBA_MAIL_FUNCTION_GET_OLT_MAC_DATA,
	DBA_MAIL_FUNCTION_SET_WAIT_MAX_NUM
} DBA_MAIL_Set_Wait_Func_t;

typedef struct DBA_MAIL_Data {
	uint16_t alloc_id;
	uint16_t fix_band;
	uint16_t max_band;
	uint16_t assure_band;
	uint8_t funcType;
	uint8_t funcId;
	uint8_t print_dba_info_en;
	uint8_t clean_dba_no_idle_en;
	uint8_t print_dba_no_idle_en;
	uint8_t get_dba_timer_en;
	uint8_t do_dba_timer_en;
	uint8_t onu_id;
	uint8_t state;
	uint8_t sw_bwmap_test_flag;
	uint8_t dba_type;
	uint8_t valid;
	uint8_t fec;
	uint8_t tcont_id;
	uint8_t ploamu_flag;
	uint32_t reg;
	uint32_t reg_val;
}DBA_MAIL_Data_t;


#endif 


