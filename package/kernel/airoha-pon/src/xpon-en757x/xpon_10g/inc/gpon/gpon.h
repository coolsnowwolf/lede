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
#ifndef _GPON_H
#define _GPON_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/crypto.h>
#include "xmcs/xmcs_const.h"
#include "xmcs/xmcs_gpon.h"
#include "gpon/gpon_act.h"
#include "gpon/gpon_ploam.h"
#include "gpon/gpon_const.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_reg.h"
#include "gpon/gpon_recovery.h"
#include "xpon_driver_global.h"
#include "gpon/gpon_ploam_raw.h"
#include <ecnt_hook/ecnt_hook_qdma.h>
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*               M A C R O S
*************************************************************************
*/

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

typedef void (*gponTimerCallback)(TIMER_FUN_PAAM);
int gpon_create_timer(struct timer_list *timer, gponTimerCallback callback, unsigned long param);
#define GPON_CREATE_TIMER(timer,func,para) gpon_create_timer(timer,func,para)

#define GPON_STOP_TIMER(timer)	\
				{ \
					if(in_interrupt()) { \
						del_timer(&timer) ; \
					} else {\
						del_timer_sync(&timer) ; \
					} \
				}
#define GPON_START_TIMER(timer,para)  { timer.expires = para; mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }
#define NGPON2_MODE  is_ngpon2_mode() 

typedef enum {
    KEY_STATE_KN0 = 0, 
    KEY_STATE_KN1, 
    KEY_STATE_KN2, 
    KEY_STATE_KN3, 
    KEY_STATE_KN4, 
} ENUM_Encrypt_Key_State_t;

typedef enum {
	TRAFFIC_DOWN = 0,
	TRAFFIC_UP
} Traffic_Status_t ;

typedef struct {
	REG_FIFO_ERR_STS   gponFifoErrSts;
	REG_TX_ERR_STS     gponTxErrSts;
    REG_RX_ERR_STS     gponRxErrSts;
	REG_DBG_BWM_CKH_STS gponbwpChkStatus ;
} GPON_DEV_ERR_STATUS_T;

typedef struct {
    unchar					msk[GPON_MSK_LENS];
    unchar					sk[GPON_SK_LENS];
	unchar                  ploamIk[GPON_KEY_GROUP_NUM][GPON_PLOAM_IK_LENS] ;
	unchar                  ploamIkIdx ;
	unchar                  omciIk[GPON_KEY_GROUP_NUM][GPON_OMCI_IK_LENS] ;
	unchar                  omciIkIdx ;
	unchar                  kek[GPON_KEY_GROUP_NUM][GPON_KEK_LENS] ;
	unchar                  kekIdx ;
	unchar                  aesUcKey[GPON_KEY_GROUP_NUM][GPON_DATA_ENCRYPT_KEY_LENS] ;
	unchar                  aesUcKeyIdx ;
	unchar                  aesBcKey[GPON_KEY_GROUP_NUM][GPON_DATA_ENCRYPT_KEY_LENS] ;
	unchar                  smaValid; /*secure mutual association*/
    unchar                  registerIDState; /*record if registerID report to olt*/
    unchar                  txKeyValid ; /*This flag should always set to 1 only set to 0 for debug */
	unchar                  defaultMsk[GPON_MSK_LENS] ;
	unchar                  defaultPloamIk[GPON_MSK_LENS] ;
    ENUM_Encrypt_Key_State_t state;
    struct timer_list		TK4_timer ;
	struct timer_list		TK5_timer ;
	struct crypto_cipher    *tfm ;	/*aes cmac cipher transformations*/
	struct crypto_cipher    *aesEcbTfm ;	/*aes ecb cipher transformations*/
}GPON_Security_t;

typedef struct {
    unchar					index;
    unchar					profVer;
	unchar                  ponId[4] ;
	unchar                  dwlcdId ;
	unchar                  dnFreqOffset ;
	unchar                  dnRate ;
	unchar                  dnFEC ;
	unchar                  channelPartition ;
	__u32                   defaultRespChan ;
	unchar                  SnGrantType ;
	unchar                  usPonId[4] ;
	unchar                  uwlchId ;
	__u32                   usFreq ;
	unchar                  opticalLinkType ;
	unchar                  usRate ;
	unchar                  defaultOnuAttenu ;
	unchar                  respThreshold ;
	unchar                  clonedConfig ;	
}Channel_Prof_T;

typedef struct {
    unchar					ng2sysId[3];
    unchar					sysProfVer;
	//unchar                  usOpWaveBands ;
	unchar                  twdmChannelCnt ;
	unchar                  workDsChannel;
	unchar                  workUsChannel;
	unchar                  standbyDsChannel;
	unchar                  standbyUsChannel;
	unchar                  protectFlag;
	unchar                  rollbackFlag;
	unchar                  tuningOnGoing;
	unchar                  workPonID[4];
	__u32                   preSeqNo ;
	//unchar                  channelSpacing ;
	//unchar                  usMSE ;
	//ushort                  fsr ;
	//unchar                  twdm_amcc_ctrl ;
	//unchar                  looseCalibBound ;
	Channel_Prof_T          channenProf[NG2_CHANNEL_MAX_NUM];
	PLOAM_RAW_Profile_T		burstProf[4];
}NGPON2_system_T;

typedef struct {
	uint 					 onu_id ;	/* ONU ID, the default is GPON_UNASSIGN_ONU_ID */
	unchar					 sn[GPON_SN_LENS] ;	/* onu serial number*/
	unchar 					 reg_id[GPON_REG_ID_LENS] ; /*onu register id number*/
	unchar                   RegidLength ;
	unchar					 hexFlag;
	unchar 					passwd[GPON_PASSWD_LENS] ;  
	unchar					passwdLength ;
	ushort					 omcc ;		/* omci channel gem port id */
	uint					 to1Timer ;	/* o1 timer time for onu activation */
	uint					 to2Timer ;	/* o2 timer time for onu activation */
	uint					 to3Timer ;	/* o3 timer time for ng2 onu activation */
	uint					 to4Timer ;	/* o4 timer time for ng2 onu activation */
	uint					 to5Timer ;	/* o5 timer time for ng2 onu activation */
//	uint					 to6Timer ;	/* o6 timer time for ng2 onu activation */
	uint					 toZTimer ;	/* oZ timer time for ng2 onu activation */
	uint					 hardware_timer; /* hardware timer for dev update hardware cnts */
	uint					 silenceInterval ; /* Silence interval the uint is ms */
	uint					 feChnSetDoneTimer; /* fe channel set done timer the uint is ms */
	uint					 eqd ; 		/* keep the latest eqd value */
	unchar 					 ponTag[GPON_TAG_LENS] ; /*pon tag*/
	unchar					 profVersion ; /*profile version field*/	
	unchar                   ploamCtrl ; /*ploam ctrl in o2-3 and o4 by sw or hw*/
    unchar                   usOmciMicCtrl ; /*upstream omci mic calculated by sw or hw*/
    unchar                   dsOmciMicCtrl ; /*downstream omci mic calculated by sw or hw*/
	XPON_Mode_t              ploamMicErrDrop ; /*drop ctrl if ploam mic err*/	
    XPON_Mode_t              omciMicErrDrop ; /*drop ctrl if omci mic err*/	
	XPON_Mode_t				 txLateResyncEn; /*HW auto resync when tx start late err happened*/
	XPON_Mode_t				 MbiTxStopEn; /*control mbi tx stop in typeb,just for test,best with it*/
	unchar 					 eqdO4Offset ;   /* O4 eqd Offset */
	unchar 					 eqdO5Offset ;   /* O5 eqd Offset */
	uint                   	 eqd_olt_init ;  /*the first eqd value */		
	uint                     eqd_olt_absolute;   /*eqd value from olt */ 	
	GPON_10G_DYING_GASP_MODE_T  dyingGasp;
	GPON_10G_DEV_SNIFFER_MODE_T sniffer_mode;	
	AllocId_Config_t         allocIdConfig;
    Key_Report_Config_t      keyReport;
    unchar                   rangingAckSeqNo;  /*record ranging ploam seq no in O4*/
	ushort					 onuResponseTime;
	GPON_10G_HEC3ERR_T       hec3errCtrl ;
    GPON_10G_COUNTER_TYPE_t		counterType;
	GPON_10G_DEV_PLOAMD_FILTER_MODE_T ploamFilter;
	ushort					 idleGemThreshold;
	GPON_DEV_ERR_STATUS_T    intErrStatus;
	struct {
		unchar				isTodUpdate 		: 1 ;    /*not used yet*/
		unchar 				eqdOffsetFlag		: 1 ;	 /* eqd offset flag 1--neg*/
	} flags ;
	__u32                    bwmLenLimit;
	__u32                    bwmChkCtrl;
	XPON_Mode_t				 Wan2WanTrafficTest;
	NGPON2_system_T         ng2;
	GPON_10G_DEV_DS_FEC_MODE_T	dsFecMode;
	struct
	{
		char				cage[8];
		char				moduleVendor[GPON_MODULE_VENDOR_LEN];
		char				moduleName[GPON_MODULE_NAME_LEN];
		char				moduleVersion[GPON_MODULE_VERSION_LEN];
		char				moduleFwVersion[GPON_MODULE_FW_VERSION_LEN];
		char				connector[24];
		char				status[GPON_MODULE_STATUS_LEN];
	}phyMedModule;
} GPON_Config_T ;

typedef struct {
	ENUM_Gpon_10G_State_t		state ;
	struct timer_list		to1_timer ;
	struct timer_list		to2_timer ;
	struct timer_list		to3_timer ;
	struct timer_list		to4_timer ;
	struct timer_list		to5_timer ;
	//struct timer_list		to6_timer ;
	struct timer_list		toZ_timer ;
	struct timer_list		hardware_timer ;
	struct timer_list		silence_timer ;
	struct timer_list		fe_chn_done_timer ;
	Traffic_Status_t	    gpon_traffic_status;
	ploam_recv_handler_t	ploamRecvHandler[PLOAM_DOWN_MAX_TYPE] ;
	__u32 			dsPloamCounter[PLOAM_DOWN_MAX_TYPE];
	__u32 			usPloamCounter[PLOAM_UP_MAX_TYPE];
	REG_CUR_KIDX	gponCurKeyIdx;
	PLOAM_RAW_General_T 	prePloamMsg ;
	GPON_Config_T			gponCfg ;
	GPON_Security_t         gponSecurity ;
	unchar					emergencyState ;
	unchar                  typeBOnGoing   ;
    UPAES_Mode_t            gemUpAESMode;
	spinlock_t				act_lock ;
	spinlock_t				mac_reset_lock ;
    spinlock_t				cmac_test ;
	struct tasklet_struct   securityKey_task;
	struct tasklet_struct   allocId_task;
	struct tasklet_struct   keyReport_task;
    struct tasklet_struct   rangingAck_task;    
	struct tasklet_struct   swreplyploam_task;
	GPON_Alarm_T			gponAlarm;
	int           			gponAlmKeepTime;
	unsigned long 			jiffiesClk[GPON_ALARM_MAX_NUM];
	unsigned int			activationCnt;
	GPON_PLOAM_CNT_T		ploamMsgcounter;
} GPON_GlbPriv_T ;

typedef enum {
	RDI_INDEX	,
	PEE_INDEX	,
	LOS_INDEX	,
	LOF_INDEX	,
	DIS_INDEX	,
	DACT_INDEX	,
	MIS_INDEX	,
	MEM_INDEX	,
	SUF_INDEX	,
	SF_INDEX	,
	SD_INDEX	,
	LCDG_INDEX	,
	TF_INDEX	,
	ROGUE_INDEX	,
	LODS_INDEX	,
	MAX_INDEX	,
}ALARM_INDEX_T;

typedef enum{
	GPON_MAC_PLAIN_RESET,
	GPON_MAC_WITH_PHY_RESET,
}GPON_RESET_TYPE_t;
/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/
void gpon_enable(void);
void gpon_disable(GPON_RESET_TYPE_t type);
void gpon_isr(void);
void gpon_phy_ready_handler(PHY_Event_Source_t src);
void gpon_phy_loss_handler(PHY_Event_Source_t src);
void gpon_stop_timer(void);
void gpon_silence_interval_expires(TIMER_FUN_PAAM arg) ;
void ng2_o9_recv_grant_handler(void);
int is_ngpon2_mode(void);
void ng2_system_config_reset(void);
void swap_work_standby_channel(void);
void forgotten_onu_monitor(void);
__u32 unchar_convert_to_u32(unchar *tmp, int len);
void gpon_set_alarmBit(ALARM_INDEX_T index);
void gpon_clear_alarmBit(ALARM_INDEX_T index);
void gpon_check_alarm_jiffiesClk(void);
void gpon_reset_alarm_jiffiesClk(void);
void gpon_clear_all_alarm(void);

#endif /*_GPON_H*/
