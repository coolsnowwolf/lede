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
#ifndef __EPON_MPCP
#define __EPON_MPCP

#include "common/drv_types.h"
/***********************
MPCP discover
***************************/
/*  refer to 802.3ah 64.3.3.2 */
/* mpcpDiscState */
#define MPCP_STATE_WAIT				0
#define MPCP_STATE_REGISTERING			1
#define MPCP_STATE_REGISTER_REQUEST	2
#define MPCP_STATE_REGISTER_PENDING	3
#define MPCP_STATE_RETRY				4
#define MPCP_STATE_DENIED				5
#define MPCP_STATE_REGISTER_ACK		6
#define MPCP_STATE_NACK				7
#define MPCP_STATE_REGISTERED			8
#define MPCP_STATE_REMOTE_DEREGISTER			9
#define MPCP_STATE_LOCAL_DEREGISTER			10

/* epon mpcp state max number */
#define MAX_MPCP_STATE_NUM           11
#define MAX_MPCP_STATE_STR_LEN       32
#define MAX_LLID_INDEX				(0x7)


#define EPON_MPCP_SEND_CHECK_TASK_POLLING 	(0)
#define EPON_MPCP_SEND_CHECK_INTERRUPT	 	(1)

#define EPON_REG_MPCP_CMD_DONE_SUCCESS		(1)

#define REGISTER_REQ_TX_IN_DISCVRY_GATE	(1)
#define REGISTER_REQ_TX_IN_NORMAL_GATE	(2)
#define REGISTER_ACK_TX_IN_NORMAL_GATE	(3)

#define REGISTER_REQ_FLAG_REGISTER		(0)
#define REGISTER_REQ_FLAG_DEREGISTER	(1)

#define REGISTER_ACK_FLAG_NACK			(0)
#define REGISTER_ACK_FLAG_ACK			(1)

#define TEST_M_LLID_IGNORE_NUM 			20

#define SHUT_DOWN_DISC_SYNC_UPDATE      (1<<24)

typedef enum
{
	MPCP_MSG_NONE,
	MPCP_MSG_DISCOVERY_GATE,
    MPCP_MSG_REGISTER_REQUEST,
    MPCP_MSG_REGISTER,
    MPCP_MSG_REGISTER_ACK
}EPON_MPCP_MSG_T;

typedef struct
{
	unsigned char		send_flag;
    EPON_MPCP_MSG_T		msg_type;
}MPCP_SEND_CHECK_INFO_T;


#ifdef TCSUPPORT_CUC
#define MPCP_RGST_AGAIN_TIMEOUT (50)
#else
#define MPCP_RGST_AGAIN_TIMEOUT (60)
#endif
#if 0
typedef struct eponMpcpDiscFsm_s{
	__u8 mpcpDiscState;
	__u8 mpcpDiscMacr_RgstrAck;//ACK; NACK
	__u8 mpcpReqRetryFlag;
	
	//variables
	bool begin;
	__u8 *dataRx;
	__u8 *dataTx;
	__u32 grantEndTime;
	bool insideDiscoveryWindow;
	__u32 localTime;
	__u16 opcodeRx;
	__u16 opcodeTx;
	__u16 pendingGrants;
	bool registered;
	__u16 syncTime;
	bool timestampDrift;	
#ifdef TCSUPPORT_EPON_RGST_SILENT
	__u16 rgstAgainTimeout;
#endif
	//struct timer_list eponMpcpTimer;
	
}eponMpcpDiscFsm_t, *eponMpcpDiscFsm_p;
#endif

typedef struct eponMpcp_s{
	unsigned char mpcpDiscState;
	unsigned char mpcpDiscMacr_RgstrAck;//ACK; NACK
	unsigned char mpcpReqRetryFlag;
	
	//variables
	bool begin;
    bool registered;
    __u16 rgstAgainTimeout;
}eponMpcp_t, *eponMpcp_p;

typedef union
{
    struct
    {
#ifdef __BIG_ENDIAN
        FIELD llid_dscvry_sts          : 2;
        FIELD rsv_26                    : 4;
        FIELD llid_rgstr_flg_sts       : 2;
        FIELD rsv_17                    : 7;
        FIELD llid_valid               : 1;
        FIELD llid_value               : 16;
#else
        FIELD llid_value               : 16;
        FIELD llid_valid               : 1;
        FIELD rsv_17                    : 7;
        FIELD llid_rgstr_flg_sts       : 2;
        FIELD rsv_26                    : 4;
        FIELD llid_dscvry_sts          : 2;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} REG_e_llid_dscvry_sts, *PREG_e_llid_dscvry_sts;


typedef enum
{
    MPCP_REGISTER_FLAG_RE_REGISTER = 0,
    MPCP_REGISTER_FLAG_DE_REGISTER,
	MPCP_REGISTER_FLAG_ACK,
	MPCP_REGISTER_FLAG_NACK    
}EPON_REGISTER_MSG_FLAG_T;

extern char mpcpStateStr[MAX_MPCP_STATE_NUM][MAX_MPCP_STATE_STR_LEN];


int epon_mpcp_discovery_gate_handler(void *data);
void epon_send_mpcp_check_handler(unsigned long data);
int epon_mpcp_set_register_ack_flag(__u8 llidIndex , __u8 rgstAckFlag);
int epon_mpcp_local_deregister(__u8 llid_index);
void eponMacTableInit(void);

int eponMpcpDscvFsmWaitHandler(__u8 llidIndex);
void eponTimeDrftIntHandler(__u32 data);
void eponMpcpTmOutIntHandler(__u32 data);
void eponTimeDrftIntHandler(__u32 data);
void eponMpcpDiscvGateIntHandler(unsigned long data);
int eponMpcpLocalDergstr(__u8 llidIndex);  
int eponMpcpRgstIntHandler(__u8 llidIndex);
int eponMpcpGntOvrRunIntHandler(void);
int eponMpcpRptOvrIntvalIntHandler(void);
int epon_mpcp_rcv_register_msg_handler(void *param);
extern void stop_omci_oam_monitor_not_online(void);
void epon_time_record(__u8 *Time);





#endif//end of __EPON_MPCP
