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
#ifndef __XPON_10G_EPON_H
#define __XPON_10G_EPON_H

#include "../common/xpondrv.h"
#include "epon/epon_mpcp.h"
#include "xpon_epon_ioctl.h"
#include "epon/epon_timer.h"
#include "epon/epon_reg.h"
#include "epon/epon_msg_route.h"
#include "xmcs/xmcs_sdi.h"

#include <asm/tc3162/cmdparse.h>

/* epon return result */
#define EPON_SUCCESS       				(0)
#define EPON_NORMAL_ERROR     			(-1)
#define EPON_INPUT_PARA_ERROR       	(1)
#define EPON_INPUT_POINT_NULL       	(2)
#define	EPON_ARRAY_INDEX_OUTBOUND		(3)

/* epon global data define */
#define EPON_MAC_ADDRESS_LENTH    		(6)
#define EPON_MAC_ADDRESS_OFFSET			(0xff48)
#define EPON_OUI_MAX_LENTH       		(3)
#define EPON_VENDER_SPECIFIC_LENTH  	(4) 
//#define EPON_LLID_MAX_NUM				(32)
#define KEY_LLID_INDEX_MASK             (0x1f)
#define EPON_HOLD_OVER_TME_DEFAULT		(1000)

#define EPON_CHECK_OAM_CNT_TME_DEFAULT	(10000)

#define EPON_DEFAULT_LASER_ON			(0x20)
#define EPON_DEFAULT_LASER_OFF			(0x20)

#define EPON_SLIENT_DEFAULT_TIME        (60)

#define EPON_LLID_BIT_MASK_NONE			(0x0)
#define EPON_LLID_BIT_MASK_DEFAULT		(0x01)

#define EPON_SILENT_INTERVAL			(1000)

#define RX_TMSTP_ADJ					(0x18)
#define D2G_RX_TMSTP_ADJ				(0xfff7)
#define U10G_TX_STMADJ					(0x8)
#define D10G_RX_TSADJ					(0xff90)

#define EPON_IPG_DEFAULT                (8)
#define EPON_MPCP_TMOUT_INTVL           (0x1f4)            

#define EPON_MAX_QUEUE_PER_CHANNEL  	(8)
#define EPON_MAX_QUEUE_THRESHOLD		(3)

#define SYNC_TIME_DEFAULT				(0x20)
#define PHY_TX_DELAY					(0x0f)
#define PHY_TX_DELAY_BY_MAC				(0x10000-PHY_TX_DELAY)

#define MPCP_SYNC_TIME_MAX  			((isEPONFWID)?(0x40):(0x5F))


/* input param  */
#define MSG_INPUT_NULL					(0)	


/* epon task define */
#define TASK_INPUT_PARA_DEFAULT			(0)
#define EPON_INIT_TASK(task, callback,para)		tasklet_init(task,callback,para)
#define EPON_START_TASK(task)				tasklet_schedule(task);

#define EPON_INTERRUPT_STATUS_BIT_MAX	(96)

#define MAX_CHAR_NUM                    (32)

typedef int (*epon_isr_handler_t)(__u32 pInput);

typedef struct epon_llid_info_s{
	__u8 		llidIndex;
	__u8 		enableFlag;
	__u16 		llid;
	__u8 		macAddr[EPON_MAC_ADDRESS_LENTH];
	__u8 		oui[EPON_OUI_MAX_LENTH];
	__u8 		vendorSpecInfo[EPON_VENDER_SPECIFIC_LENTH];	
	__u8		silent_enable;    
	__u16		silent_time_left;   
	__u8		rx_fec_flag;	/*1:enable,0:disable*/
	__u8 		llid_key_index;/*1 or 0*/
	eponMpcp_t 	eponMpcp;
	MPCP_SEND_CHECK_INFO_T  mpcp_send_info;
	__u8 		oltMacAddr[EPON_MAC_ADDRESS_LENTH];
}EPON_LLID_INFO_T, *pEPON_LLID_INFO_T;

typedef enum
{
	SILENT_ENABLE,
    SILENT_DISABLE
}EPON_SILENT_STATUS;

typedef struct mpcp_send_info_s{
    struct list_head    list;    
    __u8				llid_index;
	__u8 				msg_type;
    __u8				register_req_flag;
    __u8				register_ack_flag;
}MPCP_SEND_INFO_T;

typedef struct mpcp_send_list_entry_s{
    struct list_head    list;    
    MPCP_SEND_INFO_T	msg_info;
}MPCP_SEND_LIST_ENTRY_T;


typedef struct epon_global_data_t
{
	__u32 				llid_bit_mask;
    __u8				onu_mac_addr[EPON_MAC_ADDRESS_LENTH];
	__u16 				hold_over_time;//ms    
	__u8 				hold_over_enable;//0:disable; 1:enable
	__u8				epon_enable_flag;
	__u32 				debug_level;
    struct timer_list	silent_timer;
    struct timer_list	typeb_timer;
	struct timer_list	oamCnt_timer;
	__u16				sync_time;  
    __u16				silent_time_config;
    __u8				typeb_holdOn_enable; 
    __u8				laser_on;
    __u8				laser_off;
    __u8				static_report_enable;
    
	struct tasklet_struct epon_discovery_gate_task;
	struct tasklet_struct epon_start_task;
    struct tasklet_struct epon_mpcp_send_check_task;
	
	EPON_LLID_INFO_T 	llid_entry[EPON_LLID_MAX_NUM];
    epon_isr_handler_t	e_isr_handler[EPON_INTERRUPT_STATUS_BIT_MAX];
	__u32 				txPower_flag;	

    __u32 				llid_mask_config;
    __u32 				mac_mode;
    __u8				mpcp_send_check_type;
    __u8				oam_auth_fail;

	__u8				lost_time[MAX_CHAR_NUM];
	__u8				register_time[MAX_CHAR_NUM];
	__u8				last_register_time[MAX_CHAR_NUM];
	__u8				lost_count;
	__u16				oamCnt[EPON_LLID_MAX_NUM];

	__u8				m_llid_flag;
	__u8				ignore_flag;
	__u8				tx_rate_mode;
}EPON_GLOBAL_DATA_T,*pEPON_GLOBAL_DATA_T;
extern pEPON_GLOBAL_DATA_T gp_epon_global_data;


/*************************OLD**************************/

#define MODULE_VERSION_EPON	"MTK EPON MAC driver version 0.1"

#define EPON_MAC_MAJOR		221

#define EPON_TIMEDRIFT_THRSHLD 	0x08
#define EPON_TIMEDRIFT_RESET_THRSHLD 	0x10



typedef struct {
	__u32 earlyWakeupTimer;
	__u8 earlyWakeupFlag;
	__u16 earlyWakeupCount;
} EPON_Config_T ;


typedef struct {
	struct timer_list		early_wakeup_timer;
	EPON_Config_T			eponCfg ;
} EPON_GlbPriv_T ;

typedef struct {
	__u32 index;
	char* int_name;
} epon_int_t;

#if 0
#if DBG
u32    RTDebugLevel;
#define DBGPRINT(Level, fmt, args...) 					\
{                                   \
    if (Level <= RT_DEBUG_TRACE)      \
    {                               \
        printf(NIC_DBG_STRING);   \
		printf( fmt, ## args);			\
    }                               \
}
#else
#define DBGPRINT(Level, fmt, args...) 	
#endif
#endif



#define EPON_DBGMASK_TEMPMAC 1

//define struct for each LLID
typedef struct eponLlid_s{
	__u8 llidIndex;
	__u8 enableFlag;
	__u16 llid;
	__u8 macAddr[6];
	__u8 oui[3];
	__u8 vendorSpecInfo[4];	
	eponMpcp_t eponMpcp;
	//spinlock_t mpcpDscvStsLock;
}eponLlid_t, *eponLlid_p;

	

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
static inline s64 ktime_to_sec(const ktime_t kt){	return ktime_divns(kt, NSEC_PER_SEC);}
#endif

extern int cmd_register(cmds_t *);
extern int cmd_unregister(char *name);
extern int subcmd(const cmds_t tab[], int argc, char *argv[], void *p);

int epon_set_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int epon_get_queue_threshold(eponQueueThreshold_t *pEponQThr) ;
int eponMacSetLlidEnableMask(__u32 llidMask);
int eponSetRateMode(__u8 mode);
int epon_mpcp_register_req_tx_done(EPON_LLID_INFO_T *llid_info);
int epon_mpcp_register_ack_tx_done(EPON_LLID_INFO_T *llid_info);
int eponSetMacSecInfo(unsigned char* info);



int epon_init(void);
int epon_deinit(void);

#endif  //__XPON_10G_EPON_H

