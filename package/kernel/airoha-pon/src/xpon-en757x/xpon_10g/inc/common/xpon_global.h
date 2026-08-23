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
#ifndef _XPON_GOLBAL_H
#define _XPON_GOLBAL_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <linux/skbuff.h>
#include <linux/workqueue.h>
#include <linux/netdevice.h>
#include <linux/cdev.h>


#include "common/xpon_debug.h"
#include "gpon/gpon.h" 
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_mci.h"
#include "xmcs/xmcs_if.h"
#include "xmcs/xmcs_fdet.h"
#include "pwan/xpon_netif.h"  
#include "epon/epon.h"

//#include "qdma_bmgr.h"
#include "omci_oam_monitor.h"
#include <linux/hardirq.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "common/union_ic_def.h"


/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#ifdef TCSUPPORT_CPU_ARMV8_64
extern u32 GET_WAN_CONF(void);
extern void SET_WAN_CONF(u32 val);
extern u32 GET_SCU_RSTCTRL1(void);
extern void SET_SCU_RSTCTRL1(u32 val);
extern u32 GET_GPIO_CTRL(void);
extern void SET_GPIO_CTRL(u32 data);
extern u32 GET_GPIO_DATA(void);
extern void SET_GPIO_DATA(u32 data);
extern u32 GET_GPIO_OE(void);
extern void SET_GPIO_OE(u32 data);
extern u32 GET_IS_FPGA(void);
extern u32 GET_SSR3(void);
extern void SET_SSR3(u32 val);
extern u32 GET_CLK_PER_ICG_ENABLE(void);
extern void SET_CLK_PER_ICG_ENABLE(u32 val);
extern u32 GET_TOD_DIVIDER_ENABLE(void);
extern void SET_TOD_DIVIDER_ENABLE(u32 val);
#endif

/************************************************************************
*               M A C R O S
*************************************************************************
*/
#define PHY_CSR_PHYSET3             (0xBFAF0108)
#define PHY_CSR_PHYSET10            (0xBFAF0124)
#define PHY_CSR_DUMMY_REG_RX		(0xBFAF0290)
    
#define PHY_GPON_MODE               (1<<31)
#define TOP_TEST_MISC0_CTRL         (0xbfb00380)    /* used to reset PON PHY */
    
#define GPIO_CSR_DATA               (0xbfbf0204)
#define PHY_TX_EPON_CONT_MODE       (0xffffffdf)
#define PHY_PLL_RST                 (1<<31)
#define PHY_COUNT_RST               (1<<27)
#define PHY_GPON_DEMASK             (1<<29)
    
#define WAN_GPON_MODE                   (0)
#define WAN_EPON_MODE                   (1)
#define SCU_DYING_GASP_STATUS           (1<<16)
#define XPON_PHY_TX_POWER_ON    0
#define XPON_PHY_TX_POWER_OFF   1

#define GPON_EMERGENCE_STATE_MAX_NUM 10

#ifdef TCSUPPORT_CPU_ARMV8_64
#define isEPONFWID   0
#endif

/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/

    
    
typedef struct {
    unchar isGponHwFlag : 1 ;
    unchar isEponHwFlag : 1 ;
    unchar resv : 6;
}PON_DyingGasp_t ;

typedef enum {
    PON_WAN_START,
    PON_WAN_STOP
} PON_StartMode_t ;

 typedef enum {
	PON_LINK_STATUS_OFF,
	PON_LINK_STATUS_GPON,
	PON_LINK_STATUS_EPON,
	/*PON_LINK_STATUS_P2P,*/
} PON_WanLinkStatus_t ;

typedef enum {
	PON_ONU_TYPE_UNKNOWN =0,
	PON_ONU_TYPE_SFU,
	PON_ONU_TYPE_HGU,
} PON_OnuType_t ;

typedef enum {
    XPON_ROUGE_STATE_FALSE,
    XPON_ROUGE_STATE_TRUE,
} PON_Rouge_Status_t ;


typedef enum{
    XPON_PHY_SYNC_STATUS_NOT_START ,
    XPON_PHY_SYNC_STATUS_SYNCING   ,
    XPON_PHY_SYNC_STATUS_SYNCED    ,
    XPON_PHY_SYNC_STATUS_STOPPED   ,
} XponPhy_Mode_Detect_Status_t ;


typedef  enum {
	GPON_DISABLE_SN_REPORT_O7 =0,
	GPON_DISABLE_SN_SET_EMERGNCE_STATE,		
	GPON_PHY_READY_REPORT_O7,
	GPON_SET_CONNECTION_REPORT_O7,
	GPON_OMCI_SET_EMERGNCE_STATE,
}GPON_Emergence_Record;

 typedef enum {
	PON_PHY_LOS=0,
	PON_PHY_RDY,
} PON_PhyStatus_t;

typedef enum {
	PON_STATUS_NORMAL=0,
	PON_STATUS_ROGUE,
} PON_RogueStatus_t;

typedef enum {
    PON_MAC_START,
    PON_MAC_STOP,
} PON_MACStartMode_t ;


typedef enum XPON_DAEMON_Job_type_s {
    XPON_DAEMON_JOB_GET_PHY_PARAM  ,
    XPON_DAEMON_JOB_QUIT,
} XPON_DAEMON_Job_type_t ;

typedef enum XPON_DAEMON_Job_Priority_s {
    XPON_DAEMON_JOB_PRIORITY_LOW   ,
    XPON_DAEMON_JOB_PRIORITY_HIGH  ,
} XPON_DAEMON_Job_Priority_t ;

typedef struct XPON_DAEMON_Job_data_s{
    XPON_DAEMON_Job_type_t           id ;
    XPON_DAEMON_Job_Priority_t priority ;
    void *                 private_data ;
}XPON_DAEMON_Job_data_t;

typedef struct XPON_DAEMON_Job_s{
    XPON_DAEMON_Job_data_t  data  ;
    volatile unchar         valid ; /* TRUE means have data, otherwise mean empty*/
}XPON_DAEMON_Job_t;

#define XPON_DAEMON_JOB_QUEUE_SIZE 16
#define JOB_QUEUE_IDX_INC(x) (((x) + 1) & 0xF )

typedef struct XPON_DAEMON_Job_Queue_s {
    XPON_DAEMON_Job_t   jobs[XPON_DAEMON_JOB_QUEUE_SIZE] ;
    spinlock_t          lock         ;
    volatile uint       in_index     ;
    volatile uint       out_index    ;
    uint                drop_counter ;
}XPON_DAEMON_Job_Queue_t ;

typedef struct XPON_Daemon_s{
    wait_queue_head_t        wq            ; /* wait queue to block monitor procedure      */
    struct task_struct	*    task          ; /* kernel thread to run monitor procedure     */
    XPON_DAEMON_Job_Queue_t  job_queue     ; /* jobs needed to be done                     */
} XPON_Daemon_t;


/* add or delate this enum, must change file apps\private\xpon\ponmgr\inc\core\cmgr\cmgr_enum.h at the same time!!!! */
typedef struct {
    unchar    should_detect_stop ;
    //spinlock_t lock ;
    struct timer_list   delay_start_detect_timer ; /* delayed start of auto-detection */
    struct timer_list   check_sync_timer   ;       /* check if sync */
    
    unchar is_fix_mode  : 1; /* TRUE or FALSE */

    /* current auto-detection status, meaningful only when 'is_fix_mode' is false */
    Xpon_Phy_Mode_t              detect_mode   : 4;
    XponPhy_Mode_Detect_Status_t detect_status : 2;
    
    /* PHY mode currently used by this system, in fix mode, it equals to 'set_phy_mode'. 
       In auto-detection mode, it depends on detecting result.
    */
    Xpon_Phy_Mode_t working_mode : 4 ;
    unchar          calibrating  : 1 ;
	uint				signal_fail;
	uint				signal_degrade;
	
    
    PHY_Los_Status_t    phy_link_status : 3 ; /* Los or Ready status */

    unchar              trans_status_refresh_pending ;
    struct timer_list   trans_status_refresh_timer ; /* refresh transceiver staus timer */
	struct timer_list   traffic_status_refresh_timer ; /* refresh traffic staus timer */
    spinlock_t          trans_params_lock;
    PHY_TransParam_T    trans_params;
    PHY_TRANSINFO_T     trans_info;
} XPON_PHY_Data_t;

typedef struct {
	PON_WanLinkStatus_t      sysLinkStatus ;
	PON_WanLinkStatus_t			sysPrevLink ;
    PON_StartMode_t             sysStartup  ;
	PON_MACStartMode_t          sysMACStartup;
	PWAN_GlbPriv_T			 wanPrivData ;
	MCS_GlbPriv_T				xmcsPrivData ;
	GPON_GlbPriv_T			 gponPrivData ;
	EPON_GlbPriv_T				eponPrivData ;
	PON_DyingGasp_t				dyingGaspData ;

	PON_OnuType_t			    sysOnuType ;
    XMCSIF_WanDetectionMode_t	sysPonMode ;
	unsigned char                sysComboPon ;
	unsigned char                sysBBF247 ;
	ushort					 debugLevel ;
    XPON_PHY_Data_t             phyCfg       ;
	XPON_Daemon_t               xpon_daemon  ;
	PON_Rouge_Status_t       rogue_state  ;


	Omci_Oam_Monitor_t		    Omci_Oam_Monitor ;
	unsigned long				onlineStartTime;
	PON_PhyStatus_t				ponPhyStaus;
	PON_RogueStatus_t			ponRogueStatus;

} PON_SysData_T ;

typedef struct {
    char  proc_flag;
    struct {
        uint32_t        corrBytes ;
        uint32_t        corrCodeWords ;
        uint32_t        unCorrCodeWords ;
        uint32_t        totalRxCodeWords ;
    } fecCounter ;
    struct {
        uint32_t        XGTCHECErrCount ;
        uint32_t        PSBdHECErrCount ;
        uint32_t        XGEMHECErrCount ;
        uint32_t        XGEMHECLostWordCount ;
    } hecCounter ;

}RDK_GTC_Dbg_T;
extern RDK_GTC_Dbg_T rdk_gtc_dbg;

typedef struct {
    char            proc_micErr_flag;
    uint32_t        PloamMicErrCount ;
    uint32_t        OmciMicErrCount ;

}RDK_MIC_Dbg_T;
extern RDK_MIC_Dbg_T rdk_mic_err_dbg;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern PON_SysData_T	* gpPonSysData ;
extern PWAN_GlbPriv_T   * gpWanPriv ;
extern GPON_GlbPriv_T	* gpGponPriv ;
extern MCS_GlbPriv_T    * gpMcsPriv ;
extern EPON_GlbPriv_T   *gpEponPriv ;
extern XPON_PHY_Data_t  *gpPhyData  ;
extern PHY_FecCount_T   gphyFecCount;

#define XPON_STOP_TIMER(timer)	do	            \
			{                                   \
				if(in_interrupt()) {            \
					del_timer(&timer) ;         \
				} else {                        \
					del_timer_sync(&timer) ;    \
				}                               \
			}while(0)
			
#define XPON_START_TIMER(timer,para)			{ timer.expires = para; mod_timer(&timer, (jiffies + ((timer.expires*HZ)/1000))) ; }

extern struct net init_net;

#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

#define COPY_TO_USER(user,kernel,size,ret) \
                        if(0 == ret){\
                            ret = copy_to_user(user,kernel,size);\
                        	if(0 != ret){\
                        		printk("func:%s line:%d copy_to_user failed\n",__func__,__LINE__);\
                        	}\
                        }\
                                        
#define COPY_FROM_USER(kernel,user,size,ret) \
                        ret = copy_from_user(kernel,user,size);\
                        if(0 != ret){\
                            printk("func:%s line:%d copy_from_user failed\n",__func__,__LINE__);\
                            break;\
                        }\


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/

#endif /*_XPON_GOLBALH*/
