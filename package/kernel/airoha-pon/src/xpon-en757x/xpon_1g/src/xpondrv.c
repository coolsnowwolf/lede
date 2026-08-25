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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/netdevice.h>
#include <ecnt_hook/ecnt_hook_pon_mac.h>

#include "common/drv_global.h"
#include "common/phy_if_wrapper.h"

#include "epon/epon.h"
#include "epon/epon_mpcp.h"
#include "epon/epon_reg.h"
#include "gpon/gpon.h"
#include "gpon/gpon_init.h"
#include "gpon/gpon_dvt.h"
#include "gpon/gpon_reg.h"
#include "pwan/xpon_netif.h"
#include "xmcs/xmcs_mci.h"
#include "xmcs/xmcs_sdi.h"
#include "xmcs/xmcs_if.h"
#include "pwan/gpon_wan.h"
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
#include "pwan/hybird_wan.h"
#endif/*TCSUPPORT_COMPILE*/

#include "gpon/gpon_power_management.h"
#include <ecnt_hook/ecnt_hook_fe.h>

#include "common/xpon_daemon.h"
#include "common/xpon_led.h"

#include <ecnt_hook/ecnt_hook_qdma_7510_20.h>
#include <ecnt_hook/ecnt_hook_dgasp.h>
#include "gpon/gpon_recovery.h"
#include "gpon/gpon_dev.h"
#include "xmcs/xmcs_gpon.h"

/* 0: slow mode,  1: fast mode */
int gpon_fast_mode_flag = 0;

extern int g_rm_tcont_delay_us;
extern struct list_head fix_reg_list;
extern int channel_switch_enable;
extern int eponGetTrafficStatus(void);
extern void setMpcpFwd(int flag);
extern int xmcs_get_onu_info(struct XMCS_GponOnuInfo_S *pOnuInfo);
extern int xmcs_set_sn_passwd(struct XMCS_GponSnPasswd_S *pSnPasswd);
extern int xmcs_get_olt_distance(uint32_t * oltDistance);
extern void eponIsr(void);
#ifdef TCSUPPORT_CPU_ARMV8
extern char get_onutype(void);
extern u32 GET_DMTC(void);
extern void SET_DMTC(u32 val);
extern struct device* get_xpon_dev(void);
extern int get_xpon_irq(int index);
#endif
#ifdef CONFIG_USE_FOR_TEST
struct workqueue_struct * gem_valid_work_queue = NULL;
atomic_t work_queue_valid = ATOMIC_INIT(0);
int omci_drop_flag = 0;
#endif /* CONFIG_USE_FOR_TEST */
int stop_rouge_tx_enable = 0;
#define LOG_MONITOR_CPU  1
extern char channel_retire_en;
uint dump_all_gemport = 1;
extern uint distanceAdjust;
#if defined(TCSUPPORT_CPU_EN7581)
int gpon_tod_adjust = 0;
#endif
extern uint hw_cnt_enable;


/*****************************************************************************
******************************************************************************
   Define the Global variable 
******************************************************************************
******************************************************************************/
#define PON_MODE_GPON					(1)
#define PON_MODE_EPON					(1<<1)
#define PHY_RD							(1<<2)

#define PADD_HEADER_LEN     			(2)
#define DYING_GAPS_PKT_LEN  			(62)
#define MAX_LOG_LINE_CNT                (1000)
#define MAX_LOG_FILE_CNT                (5)

int log_task_control = LOG_TASK_EXIT;

static char dying_gasp_oam[]=
{
	0x00,0x00,0x01,0x80,0xC2,0x00,0x00,0x02,0x00,0x25,
	0x12,0xBB,0x25,0x0E,0x88,0x09,0x03,0x00,0x52,0x00,
	0x01,0x10,0x01,0x00,0x00,0x00,0x0F,0x05,0xEE,0x00,
	0x13,0x25,0x00,0x22,0x01,0x00,0x02,0x10,0x01,0x00,
	0x00,0x00,0x0F,0x05,0xEE,0x00,0x13,0x25,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

uint8 pon_mode;
int DISABLE_CALL_TRACE_DEBUG_FLAG = 0;
int phyNoLosNoReadyHandleCycle = 5;
static uint epon_proc_cnt = 0;
static uint gpon_dbru_modify_size = 0x2fff;
xpon_regs *xpon_regs_p = NULL;
static epon_stat *epon_stat_prev = NULL;
static struct proc_dir_entry *xpon_proc_dir = NULL;
static struct proc_dir_entry *gpon_proc_dir = NULL;
static struct proc_dir_entry *epon_proc_dir = NULL;

RDK_GTC_Dbg_T rdk_gtc_dbg = {};
PON_SysData_T   * gpPonSysData = NULL ;
PWAN_GlbPriv_T  * gpWanPriv    = NULL ;
MCS_GlbPriv_T   * gpMcsPriv    = NULL ;
GPON_GlbPriv_T  * gpGponPriv   = NULL ;
EPON_GlbPriv_T  * gpEponPriv   = NULL ;
unsigned int gTypeOfAction = 0;
int typeB_delay_g = 0;
int onu_rogue_test = 0;
#ifdef TCSUPPORT_AUTOBENCH
extern int epon_slt_test;
extern int gpon_slt_test;
extern int pon_loopback_result_wan;
#endif

extern int sniffer_flag;
extern unsigned int sniffer_config;
extern unsigned int special_tag;
#ifdef EPON_MAC_HW_TEST
extern int loopback_flag;
extern int print_packet;
extern int isWan2WanTrafficTest;
extern int epon_random_delay_test;
extern int random_result[2000];
extern int random_delay_test_cnt;
extern int random_dly_cnt;
extern int no_epon_mac_reset;
#endif
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
extern int randomAdjust_flag;
extern unsigned int random_config;
extern int minus_disable;
#endif

extern int xmcs_set_connection_start(XPON_Mode_t mode);
extern int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection);
extern void gpon_isr_send_rdi(void);
extern void auto_mode(void);
extern void eponSetMpcpDiscStateToDefault(void);
extern int xmcs_clear_rogue_status(void);
extern int xmcs_get_rogue_status(PON_RogueStatus_t *status);

__u32 modifyBit(__u32 rgst, __u32 p, __u32 b){
	__u32 mask = 1 << p;
	return ((rgst & ~mask) | (b << p));
}

static void gponDbruSlightModify(unsigned char modify_flag){
	if(modify_flag)
	{
		//open slight modify and set 0x2fff to bfb64354
		PON_MSG(MSG_DBG,"%s %d modify flag %d\n", __FUNCTION__,__LINE__,modify_flag);
		IO_SREG(DBG_DBA_BACK_DOOR, 0x08000000);
		IO_SREG(DBG_SLIGHT_MODIFY_SIZE_2,gpon_dbru_modify_size);
	}
	else
	{
		PON_MSG(MSG_DBG,"%s %d modify flag %d\n", __FUNCTION__,__LINE__,modify_flag);
		IO_SREG(DBG_DBA_BACK_DOOR, 0);
		IO_SREG(DBG_SLIGHT_MODIFY_SIZE_2,0);
	}
}

void trans_status_refresh(TIMER_FUN_PAAM arg)
{
    XPON_DAEMON_Job_data_t job = {0};

    if(!gpPhyData->trans_status_refresh_pending ){
        job.id       = XPON_DAEMON_JOB_GET_PHY_PARAM ;
        job.priority = XPON_DAEMON_JOB_PRIORITY_LOW  ;
        gpPhyData->trans_status_refresh_pending = 1;
        xpon_daemon_job_enqueue(&job) ;
        wake_up_xpon_daemon() ;
    }

    if (FALSE == gpPonSysData->ponMacPhyReset) 
    {
        XPON_START_TIMER(gpPhyData->trans_status_refresh_timer,1000) ;/* 1,000 ms */
    }
}

void traffic_status_refresh(TIMER_FUN_PAAM arg)
{
    static ENUM_PhyTrafficStatus last_status = TRAFFIC_STATUS_DOWN;
    ENUM_PhyTrafficStatus status = TRAFFIC_STATUS_DOWN;
    static uint8 is_first = 1;
    

    if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
        status = ( GPON_CURR_STATE == GPON_STATE_O5 )? TRAFFIC_STATUS_UP: TRAFFIC_STATUS_DOWN;
    }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus ){
        status = ( 1 == eponGetTrafficStatus() )? TRAFFIC_STATUS_UP: TRAFFIC_STATUS_DOWN;
    }else {
        status = TRAFFIC_STATUS_DOWN;
    }

    if(last_status != status || is_first){
        last_status = status;
        XPON_PHY_SET_API(PON_SET_PHY_TRAFFIC_STATUS, &status);
		LDDLA_SET_API(LDDLA_SET_PHY_TRAFFIC_STATUS, &status);
    }

    if(is_first)
        is_first = 0;

    if (PON_MAC_START == gpPonSysData->sysMACStartup){
        XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer,1000);/* 1,000 ms */
	}
    return;
}

static void xpondrv_init_phy_data(void)
{
    gpPhyData->calibrating   = FALSE;
	if(isEN7580 || isEN7581 || isAN7583){
		gpPhyData->is_fix_mode	 = TRUE;
	}else{
		gpPhyData->is_fix_mode   = FALSE;
		gpPhyData->working_mode  = PHY_UNKNOWN_CONFIG	 ;
	}
    gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_NOT_START ;
    gpPhyData->phy_link_status     = PHY_LINK_STATUS_UNKNOWN ;

    spin_lock_init(&gpPhyData->trans_params_lock);
    memset(&gpPhyData->trans_params, 0 , sizeof(gpPhyData->trans_params));

    gpPhyData->trans_status_refresh_pending = 0 ;
	gpPhyData->check_sync_timer_value =1000;/* 1s */
	GPON_CREATE_TIMER(&gpPhyData->trans_status_refresh_timer,trans_status_refresh,1000);/* 1,000 ms */
	GPON_CREATE_TIMER(&gpPhyData->check_sync_timer,check_sync_result,gpPhyData->check_sync_timer_value);
	GPON_CREATE_TIMER(&gpPhyData->traffic_status_refresh_timer,traffic_status_refresh,1000);/* 1,000 ms */

}


void pon_log_counter_print(struct file *fp, off_t * offset)
{
    struct tm t;
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	struct timeval tv;
#else
	struct timespec64 tv;
#endif
    char str[100] = {0};

    if(NULL == fp)
    {
        printk("==open log file error !===\n");
        return;
    }
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
    do_gettimeofday(&tv);
    time_to_tm(tv.tv_sec, 0, &t);
#else
	ktime_get_real_ts64(&tv);
	time64_to_tm(tv.tv_sec, 0, &t);
#endif   
   
    sprintf(str,"\n[%02dH%02dM%02dS] Save log \n", t.tm_hour, t.tm_min, t.tm_sec);
    fp->f_op->write(fp, str, strlen(str), &fp->f_pos);

    sprintf(str,"print whatever you need \r\n");
    fp->f_op->write(fp, str, strlen(str), &fp->f_pos);
   
    *offset = fp->f_pos;
    
    return;
}

/*****************************************************************************
******************************************************************************/
static void xpondrv_init_global_data(void)
{
#if defined (TCSUPPORT_CPU_ARMV8) 
    gpPonSysData->sysOnuType = get_onutype();
#else
	gpPonSysData->sysOnuType = READ_FLASH_BYTE(flash_base + 0xff9c);
#endif
	gpPonSysData->sysLinkStatus = PON_LINK_STATUS_OFF ;
	gpPonSysData->sysPrevLink   = PON_LINK_STATUS_OFF ;
    gpPonSysData->sysStartup    = PON_WAN_STOP ;

	gpEponPriv = &gpPonSysData->eponPrivData ;
	gpGponPriv = &gpPonSysData->gponPrivData ;

	gpWanPriv = &gpPonSysData->wanPrivData ;
	gpMcsPriv = &gpPonSysData->mcsPrivData ;
    gpPhyData =  &gpPonSysData->phyCfg ;

	//gpPonSysData->debugLevel = (MSG_TRACE|MSG_ERR|MSG_INT|MSG_ACT|MSG_OAM|MSG_OMCI) ;//| (MSG_TRACE|MSG_CONTENT) ;
	//gpPonSysData->debugLevel = (MSG_INT | MSG_OAM | MSG_ACT) ;
	gpPonSysData->debugLevel = 0 ;
	if(isEN7580 || isEN7581 || isAN7583){
		if(XMCS_IF_WAN_DETECT_MODE_GPON == ((gpPonSysData->sysOnuType & ONUMODE_MASK )>>4)){
			gpPhyData->working_mode = PHY_GPON_CONFIG;
		}else if(XMCS_IF_WAN_DETECT_MODE_GPON_SYM == ((gpPonSysData->sysOnuType & ONUMODE_MASK )>>4)){
			gpPhyData->working_mode = PHY_GPON_SYM_CONFIG;
		}else{
			gpPhyData->working_mode = PHY_EPON_CONFIG;
        }
#ifdef TCSUPPORT_CPU_EN7581
		gpPonSysData->sysComboPon = (gpPonSysData->sysOnuType & COMBOPON_MASK) >> 2;
		if(gpPonSysData->sysComboPon == 1){
			if(gpPhyData->working_mode != PHY_GPON_CONFIG){
				printk("*** Flash onu mode is %d, change to GPON ! ***\n", gpPhyData->working_mode);
				gpPhyData->working_mode = PHY_GPON_CONFIG;
			}		
		}
#endif
	}

	gpPonSysData->sysBBF247 = (gpPonSysData->sysOnuType & BBF247_MASK) >> 3;

	if((gpPonSysData->sysOnuType & ONUTYPE_MASK) != PON_ONU_TYPE_HGU)//set default onu type to SFU
		gpPonSysData->sysOnuType  = PON_ONU_TYPE_SFU;
	else
		gpPonSysData->sysOnuType = PON_ONU_TYPE_HGU;

	gpPonSysData->dyingGaspData.isEponHwFlag = 1 ;

	if (isMT7520E2E3) // for E2/E3 chip 
		gpPonSysData->dyingGaspData.isGponHwFlag = GPON_SW ;
	else // for E4 and after chip
		gpPonSysData->dyingGaspData.isGponHwFlag = GPON_HW ;
	
	gpPonSysData->sysOltType = 0;

    init_waitqueue_head(&gpPonSysData->xpon_daemon.wq) ;

    memset(&gpPonSysData->xpon_daemon.job_queue, 0, sizeof(gpPonSysData->xpon_daemon.job_queue));
    spin_lock_init(&gpPonSysData->xpon_daemon.job_queue.lock);

    
	gpPonSysData->xpon_daemon.task= kthread_run(xpon_daemon, NULL, "xpon_daemon");
	if(IS_ERR(gpPonSysData->xpon_daemon.task))
	{		
		panic("@%s>>%d--xpon_daemon init failed\n", __FUNCTION__, __LINE__);
	}
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_FALSE ;
	gpPonSysData->fe_reset_flag = FALSE;
    gpPonSysData->fe_reset_happened = FALSE;
    xpondrv_init_phy_data();

    gpPonSysData->isUpDataTraffic    = XPON_DISABLE ;
    gpPonSysData->isUpOmciOamTraffic = XPON_DISABLE ;
    gpPonSysData->ponMacPhyReset     = FALSE ;

    spin_lock_init(&gpPonSysData->event_lock) ;
}


/******************************************************************************
******************************************************************************/
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t gpon_debug_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int index=0 ;
	
	return index ;
}

/******************************************************************************
******************************************************************************/
static ssize_t gpon_bbf247_flag_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len;
    loff_t pos = *ppos;
	len = sprintf(buf, "%s\n", gpPonSysData->sysBBF247 ? "1" : "0") ;
    len -= pos;

    if (len > count){
        len = count;
    }
    if (len < 0){
        len = 0;
    }
	
	printk("xpon_1g drv [%s] bbf 247\n",gpPonSysData->sysBBF247 ? "Enable" : "Disable");
	return len;
}
#else
static int gpon_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	
	return index ;
}
/******************************************************************************
******************************************************************************/
static int gpon_fast_slow_read_proc(char * buf,char * * start,off_t off,int count,int * eof,void * data)
{
	if (gpon_fast_mode_flag) {
		printk("gpon_fast_mode_flag value is %d, gpon up stream fast mode enable\n", gpon_fast_mode_flag);
	} else {
		printk("gpon_fast_mode_flag value is %d, gpon up stream fast mode disable\n", gpon_fast_mode_flag);
	}
	return 0;	
}

/******************************************************************************
******************************************************************************/
static int gpon_bbf247_flag_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
    
	len = sprintf(buf, "%s\n", gpPonSysData->sysBBF247 ? "1" : "0") ;
    len -= off;
    *start = buf + off;

    if (len > count){
        len = count;
    }else{
        *eof = 1;
    }
    if (len < 0){
        len = 0;
    }
	
	printk("xpon_1g drv [%s] bbf 247\n",gpPonSysData->sysBBF247 ? "Enable" : "Disable");
	return len;
}
#endif

/******************************************************************************
******************************************************************************/

#ifdef CONFIG_TP_IMAGE
	
	static int gpon_linkstate_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
	{
		return sprintf(buf, "%d", gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON ? 1 : 0);
	}
	
	static int gpon_state_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
	{
		return sprintf(buf, "%d", gpGponPriv->state);
	}
	
	static int gpon_onuid_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
	{
		return sprintf(buf, "%d", gpGponPriv->gponCfg.onu_id);
	}
	
#endif /* CONFIG_TP_IMAGE */
/*****************************************************************************
******************************************************************************/

#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t epon_debug_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int index=0 ;
	int cpu_type = 0;    
	int i;
	loff_t off = *ppos;
	chipInformation devices_table[] = 
	{
		{MT7520S, isMT7520S},
		{MT7520, isMT7520},
		{MT7520G, isMT7520G},
		{MT7525, isMT7525},
		{MT7525G, isMT7525G},
		{EN7521S, isEN7521S},
		{EN7521F, isEN7521F},
		{EN7526F, isEN7526F},
		{EN7526D, isEN7526D},
		{EN7526G, isEN7526G},
        {EN751221, isEN751221},
		{0, 0},
	};
	for ( i = 0; i <ARRAY_SIZE(devices_table); i++ )
	{
		if(devices_table[i].chipId)
		{
			cpu_type = devices_table[i].chidIdx;
			break;
		}
	}

	index = sprintf(buf, "%d\n", cpu_type);

	index -= off;

	if (index > count)
		index = count;

	if (index < 0)
		index = 0;

	return index;


}
#else
static int epon_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	int cpu_type = 0;    
	int i;
	chipInformation devices_table[] = 
	{
		{MT7520S, isMT7520S},
		{MT7520, isMT7520},
		{MT7520G, isMT7520G},
		{MT7525, isMT7525},
		{MT7525G, isMT7525G},
		{EN7521S, isEN7521S},
		{EN7521F, isEN7521F},
		{EN7526F, isEN7526F},
		{EN7526D, isEN7526D},
		{EN7526G, isEN7526G},
        {EN751221, isEN751221},
		{0, 0},
	};
	for ( i = 0; i <ARRAY_SIZE(devices_table); i++ )
	{
		if(devices_table[i].chipId)
		{
			cpu_type = devices_table[i].chidIdx;
			break;
		}
	}

	index = sprintf(buf, "%d\n", cpu_type);

	index -= off;
	*start = buf + off;

	if (index > count)
		index = count;
	else
		*eof = 1;

	if (index < 0)
		index = 0;

	return index;


}
#endif


///******************************************************************************
//******************************************************************************/
//int enableQdmaScheduling(void)
//{
//	QDMA_TrtcmParam_T trtcmParam ;
//	int i ;
//	
//	qdma_set_trtcm_param_unit(QDMA_TRTCM_UNIT_16BYTE) ;
//	qdma_set_trtcm_mode(QDMA_ENABLE) ;
//
//	trtcmParam.channel = 1 ;
//	trtcmParam.cir = 4096 ;
//	trtcmParam.cbs = 256 ;
//	trtcmParam.pir = 10240 ;
//	trtcmParam.pbs = 512 ;
//	if(qdma_set_trtcm_params(&trtcmParam) < 0) {
//		printk("===========>0000\n") ;
//	}
//	
//	memset(&trtcmParam, 0, sizeof(QDMA_TrtcmParam_T)) ;
//	
//	for(i=0 ; i<15 ; i++) {
//		trtcmParam.channel = i ;
//		if(qdma_get_trtcm_params(&trtcmParam) < 0) {
//			printk("===========>0001\n") ;
//		} else {
//			printk("========>%d: CIR:%d, CBS:%d, PIR:%d, PBS:%d\n", trtcmParam.channel,
//													 				trtcmParam.cir,
//													 				trtcmParam.cbs,
//													 				trtcmParam.pir,
//													 				trtcmParam.pbs) ;
//		}
//	}
//	
//	
//	/* Configure the QDMA scheduling mode */
//	if(qdma_set_scheduling_mode(QDMA_WAN_TYPE_GPON) < 0) {
//		printk("=======>1111\n") ;
//	}
//	return 0 ;
//}

/******************************************************************************
******************************************************************************/
unchar reiSeq = 0 ;
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
uint8 attackEnhance = 0;
unsigned int dropThred = 500;
uint8 rxDropCheckTimer = 5;
#endif

#ifndef isdigit
#define isdigit(x)	((x)>='0'&&(x)<='9')
#endif

int atoi_temp(char *s)
{
	int i = 0;

	if(s == NULL)
	{
		printk("\r\ns is NULL ====> atoi in xpon drv");
		return -1;
	}
	
        while (isdigit(*s)) {
                i = i*10 + *(s++) - '0';
        }
        return i;
}

uint8 *GetMacAddr(void);

void
xmit_dying_gasp_oam(void)
{
	struct sk_buff * skb =  NULL;
	struct net_device * dev = NULL;

	dev = dev_get_by_name(&init_net, "oam");

	if (!dev) goto end;

   
	skb = skbmgr_alloc_skb2k();

	if(unlikely(!skb))
		goto end;

	memcpy((dying_gasp_oam + (PADD_HEADER_LEN + ETH_ALEN) ), GetMacAddr(),ETH_ALEN);
	memcpy(skb->data, dying_gasp_oam, sizeof(dying_gasp_oam));
	skb_put(skb,DYING_GAPS_PKT_LEN);
	pwan_net_start_xmit(skb, dev);
#ifndef TCSUPPORT_CPU_ARMV8  
	dev_put(dev);
#endif	
end:
	return;
}
extern uint g_EponLaserTime;
extern __u8 g_silence_time;
extern epon_t eponDrv;

#ifdef XPON_MAC_CONFIG_DEBUG
int drop_print_flag= 0;
int xpon_mac_print_open=0;
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
extern FHNet_Mapping_Result_Out_t gMappingRet;
extern uint fhNetMappingDebug;
extern unsigned char  gFhDrop;
#endif/*TCSUPPORT_COMPILE*/

#ifdef TCSUPPORT_VNPTT
extern __u8 ignore_deactive_flag;
#endif
irqreturn_t xpon_dying_gasp_interrupt(int irq, void *dev_id);

#ifdef CONFIG_USE_FOR_TEST
static void gem_valid_work_queue_func(struct work_struct * worker)
{
    int i = 0;
	struct XMCS_GemPortInfo_S *gemInfo ;
	uint gemPortId, gemIdx ;
    int cnt = 0;
    
	printk("do gem_valid_work_queue_func start !!!\n");

	gemInfo = kzalloc(sizeof(struct XMCS_GemPortInfo_S), GFP_ATOMIC);
	if(NULL == gemInfo){
		printk("func:%s line:%d kzalloc fail\n",__func__,__LINE__);
		return;

	}

	if(xmcs_get_gem_port_info(gemInfo) < 0)
		printk("exec failed") ;
            
	while(atomic_read(&work_queue_valid)){
        cnt += 2;
		for(i=0 ; i<gemInfo->entryNum ; i++) {
			gemPortId = gemInfo->info[i].gemPortId;
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK); 	
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel){
				gpWanPriv->gpon.gemPort[gemIdx].info.valid = 0 ;
				gponDevSetGemInfo(gemPortId, XPON_DISABLE, XPON_DISABLE) ;
			}
		}
		//msleep(1);
		if (cnt == 200){
            msleep(1);
            cnt = 1 ;
        }

		for(i=0 ; i<gemInfo->entryNum ; i++) {
			gemPortId = gemInfo->info[i].gemPortId;
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK); 	
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel){
				gpWanPriv->gpon.gemPort[gemIdx].info.valid = 1;
				gponDevSetGemInfo(gemPortId, XPON_ENABLE, XPON_DISABLE) ;
			}
		}
		if (cnt >= 201){
            msleep(1);
            cnt = 0 ;
        }
    }
	kfree(gemInfo);
	
    printk("do gem_valid_work_queue_func end !!!\n");
	return;
}

extern struct work_struct gem_valid_work_queue_struct;
DECLARE_WORK(gem_valid_work_queue_struct, (work_func_t)gem_valid_work_queue_func);

extern uint eqdDbgFlag;
extern uint EqdCounter;
extern uint EqdArray[EQD_ARRAY_MAX_NUM];
#endif /* CONFIG_USE_FOR_TEST */


int	gpon_alarm_proc_write(char *cmd, char *subcmd)
{
	ALARM_INDEX_T index;
	if(!strcmp(subcmd, "RDI"))
		index = RDI_INDEX;
	else if(!strcmp(subcmd, "PEE"))
		index = PEE_INDEX;
	else if(!strcmp(subcmd, "LOS"))
		index = LOS_INDEX;
	else if(!strcmp(subcmd, "LOF"))
		index = LOF_INDEX;
	else if(!strcmp(subcmd, "DIS"))
		index = DIS_INDEX;
	else if(!strcmp(subcmd, "DACT"))
		index = DACT_INDEX;
	else if(!strcmp(subcmd, "MIS"))
		index = MIS_INDEX;
	else if(!strcmp(subcmd, "MEM"))
		index = MEM_INDEX;
	else if(!strcmp(subcmd, "SUF"))
		index = SUF_INDEX;
	else if(!strcmp(subcmd, "SF"))
		index = SF_INDEX;
	else if(!strcmp(subcmd, "SD"))
		index = SD_INDEX;
	else if(!strcmp(subcmd, "LCDG"))
		index = LCDG_INDEX;
	else if(!strcmp(subcmd, "TF"))
		index = TF_INDEX;
	else if(!strcmp(subcmd, "ROGUE"))
		index = ROGUE_INDEX;
	else if(!strcmp(subcmd, "LODS"))
		index = LODS_INDEX;
	else if(!strcmp(subcmd, "MAX"))
		index = MAX_INDEX;
	else{
		printk("invalid command!\n") ;
		return 0;
	}

	if(!strcmp(cmd, "set_alarm"))
		gpon_set_alarmBit(index);
	else
		gpon_clear_alarmBit(index);

	return 0;
}
/******************************************************************************
******************************************************************************/
static int rdkb_write_gtc_count(char * subcmd,uint action)
{
    rdk_gtc_dbg.proc_flag = 1;
    if(!strcmp(subcmd, "corrBytes")){
        rdk_gtc_dbg.fecCounter.corrBytes = action;
    }else if(!strcmp(subcmd, "corrCodeWords")){
        rdk_gtc_dbg.fecCounter.corrCodeWords = action;
    }else if(!strcmp(subcmd, "unCorrCodeWords")){
        rdk_gtc_dbg.fecCounter.unCorrCodeWords = action;
    }else if(!strcmp(subcmd, "totalRxCodeWords")){
        rdk_gtc_dbg.fecCounter.totalRxCodeWords = action;

    }else if(!strcmp(subcmd, "XGEMHECErrCount")){
        rdk_gtc_dbg.hecCounter.XGEMHECErrCount = action;
    }else{
		printk("invalid command!\n") ;
		return 0;
	}
    return 0;

}
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t gpon_debug_write_proc(struct file *file, const char *buffer, size_t count, loff_t *data)
#else
static int gpon_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
#endif
{
	extern __u8 eponStaticRptEnable;
	extern __u32 eponDebugLevel;
	extern __u32 epon_rx_diff;
	
#ifdef TCSUPPORT_PON_TEST
	extern __u32 eponDrvDbgMask;
	extern __u8 eponDrvTmpMacAddr[];
#endif
	char val_string[64], cmd[32], subcmd[32] ;
	uint action = 0;
	int i = 0;
	int ret = 0;
	char* endpo =NULL; 
	struct XMCS_GemPortInfo_S *gemInfo = NULL;	
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%s %s %x", cmd, subcmd, &action) ;
    
    if(!strcmp(cmd, "show_flag")) {
        printk("sysLinkStatus:\t %d\n",gpPonSysData->sysLinkStatus);
        printk("sysStartup:\t %d\n",gpPonSysData->sysStartup);
        printk("drop_print_flag:\t %d\n",drop_print_flag);
		printk("xpon_mac_print_open:\t %d\n",xpon_mac_print_open);
	}
	
	else if(!strcmp(cmd, "epon_rx_diff")) {
		if(!strcmp(subcmd, "value")) {
			epon_rx_diff = action;
			printk("set rx_diff_value %d success\n", action);
		}
	}
	
	else if(!strcmp(cmd, "set")) {
		if(!strcmp(subcmd, "rx_ratelimit")) {
			gpon_enable_cpu_protection(action);
			printk("set rx ratelimit %d FPS success\n", action);
		}
		else if(!strcmp(subcmd, "disable_debug")) {
			DISABLE_CALL_TRACE_DEBUG_FLAG = action;
			printk("set disable_debug %d success\n",DISABLE_CALL_TRACE_DEBUG_FLAG);
		}
	}
	
	else if(!strcmp(cmd, "gpon_recovery_dbg"))
	{
		if(!strcmp(subcmd, "1")){
			gponRecovery.dbgPrint = 1;
		}else{
			gponRecovery.dbgPrint = 0;
		}
	}
	else if(!strcmp(cmd, "channel_switch"))
	{
		if(!strcmp(subcmd, "enable")){
			channel_switch_enable = 1;
			printk("channel switch enable\n");
		}else if(!strcmp(subcmd, "disable")){
			channel_switch_enable = 0;
			printk("channel switch disable\n");
		}else{
			printk(" subcmd error, enable or disable.\n");
		}
	}
	else if(!strcmp(cmd, "pon_phy_sync_timer")){
		uint value1 = 0;
	    sscanf(val_string, "%s %x", cmd, &value1) ;
		gpPhyData->check_sync_timer_value =value1;
		XPON_START_TIMER(gpPhyData->check_sync_timer,gpPhyData->check_sync_timer_value);
	}
    else if(!strcmp(cmd, "olttype")) {
		if(!strcmp(subcmd, "set")) {
			gpPonSysData->sysOltType = action;
			printk("set olt type: %d\n",gpPonSysData->sysOltType);
		} else{
			printk("get olt type: %d\n",gpPonSysData->sysOltType);
		}
	} else if(!strcmp(cmd, "hwnat")) {
		if (!strcmp(subcmd, "noclean")){
			is_hwnat_dont_clean = ((action==0)?0:1);
			gpGponPriv->gponCfg.flags.hotplug = ((action==0)?0:1);
			printk("hw_nat table is %s!\n", (action==0)?"Clean":"Not clean");
		}else
            printk("wrong hwnat subcmd\n");
	}else if(!strcmp(cmd,"consid_tx_rx_ph")){
		if(!strcmp(subcmd,"stop"))
		{
			gpGponPriv->gponCfg.consid_tx_rx_ph = 1;
			printk("close the MAC TX RX consid\n");
		}
		else
		{
		    gpGponPriv->gponCfg.consid_tx_rx_ph = 0;
			printk("open the MAC TX RX consid\n");
		}
	}else if(!strcmp(cmd,"swreset")){
		if(!strcmp(subcmd,"test"))
		{
			gpGponPriv->gponCfg.sw_reset_flag = ((action==0)?0:1);
			printk("sw_reset_flag = %d\n", gpGponPriv->gponCfg.sw_reset_flag);
		}
	}else	if (!strcmp(cmd, "pwan")){
		if (!strcmp(subcmd, "qos")){
			gpWanPriv->devCfg.flags.isQosUp = action;
			ECNT_QDMA_GREEN_DROP_CTRL_HOOK(ECNT_QDMA_SET_QOS_FLAG,action);
		}else if (!strcmp(subcmd, "mapqos")){
			QDMA_TxQosScheduler_T txQosSch;

			if (action ==1 ){
				gpWanPriv->devCfg.flags.isQosUp = 1;
			}else if (action == 0){
				gpWanPriv->devCfg.flags.isQosUp = 0;
			}
			
			// set qos scheduler to sp mode
			memset(&txQosSch, 0, sizeof(QDMA_TxQosScheduler_T));
			for (i = 0; i < CONFIG_QDMA_QUEUE; ++i){
				txQosSch.queue[i].weight= 255; // skip set queue weight
			}

			for (i = 0; i < CONFIG_QDMA_CHANNEL; ++i){
				txQosSch.channel = i;
				txQosSch.qosType = QDMA_TXQOS_TYPE_SP;
				QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQosSch);
			}
		}
	}else if (!strcmp(cmd, "epon")){
		if (!strcmp(subcmd, "staticrpt")){
			eponStaticRptEnable = ((action==0)?0:1);			
			printk("%s epon static report!\n", (action==0)?"Disable":"Enable");
			
			if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON){
				if (action == 0){ // disable static report for epon
					e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 0);
					e_tx_cal_cnst_SET_default_ovrhd(e_tx_cal_cnst, 0xc);
				}else{ // enable static report & set def rpt value
					e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 1); // clear rpt buffer
					e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
					e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
					e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
					e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
					e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 0); // return normal
					e_rpt_len_SET_df_rpt_dt_len(e_rpt_len, 1);// set def rpt data length
					e_rpt_data_SET_df_rpt_data(e_rpt_data, 0x01018000); // static report value
					e_rpt_data_SET_df_rpt_data(e_rpt_data, 0);					
					e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 1);
					e_tx_cal_cnst_SET_default_ovrhd(e_tx_cal_cnst, 0x8);
				}
			}
		} 
		else if(!strcmp(subcmd, "debuglevel")){
			eponDebugLevel = action;			
		}
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
		else if(!strcmp(subcmd, "minusDisable")){
			minus_disable = action;		
			printk("%s, action=%x\n", minus_disable ?"close":"open", minus_disable);
		}
		else if(!strcmp(subcmd, "randomAdjust")){
			if(action == 0)
		        randomAdjust_flag = 0;
		    else
		    {
			    randomAdjust_flag = 1;
			    random_config = action;
			}
			printk("%s, action=%x\n", randomAdjust_flag?"open":"close", random_config);

		}
#endif
		else if(!strcmp(subcmd, "mpcpIntMode")){
			gpEponPriv->mpcpInterruptMode = action;
		}
#ifdef EPON_MAC_HW_TEST
        else if(!strcmp(subcmd, "wan2wan_traffic"))
        {
            isWan2WanTrafficTest = action;
            printk("wan2wan traffic test %s\n", action?"start":"stop");
        }
        else if(!strcmp(subcmd, "no_epon_mac_reset"))
        {
            no_epon_mac_reset = action;
            printk("%s the switch that epon mac do not reset\n", action?"open":"close");
        }
        else if(!strcmp(subcmd, "random_dly_test"))
        {
            epon_random_delay_test = 1;
            random_dly_cnt = 0;
            random_delay_test_cnt = action;
            memset(random_result, 0, sizeof(random_result));
            printk("random delay test start, test count is %d\n", action);
        }
        else if(!strcmp(subcmd, "random_dly_show"))
        {
            epon_random_delay_test = 0;
            printk("random delay test stop, test count is %d\n", random_dly_cnt);
            for(i = 0; i < random_dly_cnt; i++)
                printk("random_delay[%d] = %d\n", i , random_result[i]);
        }
		else if(!strcmp(subcmd, "dying_gasp_oam")){
			gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON;
			gpPonSysData->isUpOmciOamTraffic = XPON_ENABLE;
			if (action > 0)
			{
				while (action)
				{
					xmit_dying_gasp_oam();
					action--;
				}
			}
		}
		else if(!strcmp(subcmd, "loopback_flag")){
			loopback_flag = action;
		}
		else if(!strcmp(subcmd, "print_packet")){
			print_packet = action;
		}
#endif
		else if(!strcmp(subcmd, "sniffer_mode")){
		    if(action == 0)
		        sniffer_flag = 0;
		    else
		    {
			    sniffer_flag = 1;
			    sniffer_config = action;
			}
			printk("%s up to los sniffer mpcp,data,oam, action=%x\n", sniffer_flag?"open":"close", sniffer_config);
		}
		else if(!strcmp(subcmd, "special_tag")){
			special_tag = action;
		}
		else if (!strcmp(subcmd, "dying")){
			if(action == 0) {
				gpPonSysData->dyingGaspData.isEponHwFlag = 0 ;	
				eponHwDygaspCtrl(XPON_DISABLE);
				printk("The dying gasp message will be send by software\n") ;		
			} else if (action == 1) {
				gpPonSysData->dyingGaspData.isEponHwFlag = 1 ;	
				eponHwDygaspCtrl(XPON_ENABLE);	
				printk("The dying gasp message will be send by hardware\n") ;	
			}
		}
		else if (!strcmp(subcmd, "txoamfavor")){
			if(action == 0) {
				gpEponPriv->eponCfg.txOamFavorMode = 0;	
				printk("Tx OAM Favor mode is disabled\n") ;		
			} else if (action == 1) {
				gpEponPriv->eponCfg.txOamFavorMode = 1;	
				printk("Tx OAM Favor mode is enabled\n") ;	
			}
		}
#ifdef TCSUPPORT_PON_TEST
		else if (!strcmp(subcmd, "tmpmac")){
			int tmp[6]= {0}, i =0;

			if (action ==1){
				if (9==sscanf(val_string, "%s %s %x %02x:%02x:%02x:%02x:%02x:%02x", cmd, subcmd, &action, \
					&tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5])){
					for (i=0; i<6; i++){
						eponDrvTmpMacAddr[i] = tmp[i];
					}
					eponDrvDbgMask |= EPON_DBGMASK_TEMPMAC;
					printk("Enable epon tmp mac %02x:%02x:%02x:%02x:%02x:%02x!\n DBGMASK = 0x%08x\n", \
						eponDrvTmpMacAddr[0], eponDrvTmpMacAddr[1], eponDrvTmpMacAddr[2], \
						eponDrvTmpMacAddr[3], eponDrvTmpMacAddr[4], eponDrvTmpMacAddr[5], eponDrvDbgMask);
				}else{
					printk("Error: param is wrong!\n");
				}
			}else{
				eponDrvDbgMask &= ~EPON_DBGMASK_TEMPMAC;
				printk("Disable epon tmp mac!\n");
			}
		}
#endif		
	}
#if defined(TCSUPPORT_EPON_ATTACK_ENHANCE)
	else if (!strcmp(cmd, "drop")){
		if (!strcmp(subcmd, "1")){
			printk("Enable the attack enhance function!\n");	
			attackEnhance = 1;
		}
		else
		{	
			printk("Disable the attack enhance function!\n");	
			attackEnhance = 0;
		}

	}
	else if (!strcmp(cmd, "dropThreld")){
		int threld = atoi_temp(subcmd);
		if(threld>=1 && threld<=0x1000)
			dropThred = threld;
		printk("The threld for drop packets is 0x%08x", dropThred);
	}
	else if (!strcmp(cmd, "dropTimer")){
		int dropTimer = atoi_temp(subcmd);
		if(dropTimer>=1 && dropTimer<=60)
			rxDropCheckTimer = (uint8)dropTimer;
		printk("The rx drop check timer is 0x%x", rxDropCheckTimer);
	}		
#endif

#ifdef XPON_MAC_CONFIG_DEBUG
	if(!strcmp(cmd, "msg")) {
		int level = gpPonSysData->debugLevel ;
		
		if(!strcmp(subcmd, "act")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_ACT) : (level&~MSG_ACT) ;
		} else if(!strcmp(subcmd, "omci")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_OMCI) : (level&~MSG_OMCI) ;
		} else if(!strcmp(subcmd, "oam")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_OAM) : (level&~MSG_OAM) ;
		} else if(!strcmp(subcmd, "int")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_INT) : (level&~MSG_INT) ;
		} else if(!strcmp(subcmd, "trace")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_TRACE) : (level&~MSG_TRACE) ;
		} else if(!strcmp(subcmd, "warning")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_WARN) : (level&~MSG_WARN) ;
		} else if(!strcmp(subcmd, "content")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_CONTENT) : (level&~MSG_CONTENT) ;
		} else if(!strcmp(subcmd, "debug")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_DBG) : (level&~MSG_DBG) ;
		} else if(!strcmp(subcmd, "err")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_ERR) : (level&~MSG_ERR) ;
		} else if(!strcmp(subcmd, "eqd")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_EQD) : (level&~MSG_EQD) ;
		} else if(!strcmp(subcmd, "xmcs")) {
			gpPonSysData->debugLevel = (action) ? (level|MSG_XMCS) : (level&~MSG_XMCS) ;
		} else if(!strcmp(subcmd, "typeb")){
		    gpPonSysData->debugLevel = (action) ? (level|MSG_TYPEB) : (level&~MSG_TYPEB) ;
		}
		printk("Debug Level: %x\n", gpPonSysData->debugLevel) ; 
	}  

#ifdef TCSUPPORT_WAN_GPON
	else if(!strcmp(cmd, "csr")) {
		if(!strcmp(subcmd, "gem")) {
			gpon_dvt_gemport(action) ;
		}
		else if(!strcmp(subcmd, "gpidx")){
            gpon_index_table(action);
		}
		else if(!strcmp(subcmd, "tcont")){
			gpon_dvt_channel_vld_info();
		}
	} else if(!strcmp(cmd, "test")) {
		if(!strcmp(subcmd, "loopback")) {
			for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
				if(gpWanPriv->gpon.gemPort[i].info.valid) {
					gwan_config_gemport(gpWanPriv->gpon.gemPort[i].info.portId, ENUM_CFG_LOOPBACK, action) ;
				}
			}
			gpWanPriv->devCfg.flags.isRandomLb = (action==2) ? 1 : 0 ;
			printk("Test Loopback Mode: %d\n", action) ;
		} else if(!strcmp(subcmd, "gpon")) {
			gpPonSysData->sysLinkStatus = PON_LINK_STATUS_GPON ;
		} else if(!strcmp(subcmd, "epon")) {
			gpPonSysData->sysLinkStatus = PON_LINK_STATUS_EPON ;
		} else if(!strcmp(subcmd, "eqd")) {
			gpon_dvt_eqd_adjustment(action) ;
			printk("Modify the EqD to %d in O5 state\n", action) ;
        } else if(!strcmp(subcmd, "eqd_dis")) {
            if(action==1) {
                gpPonSysData->gponPrivData.gponCfg.dis_ranging_in_o5=1;
                printk("\nDisable Ranging in O5!\n");
            } else {
                gpPonSysData->gponPrivData.gponCfg.dis_ranging_in_o5=0;
                printk("\nEnable Ranging in O5!\n");
            }    
		} else if(!strcmp(subcmd, "report")) {
			//enableQdmaScheduling() ;	
		} else if(!strcmp(subcmd, "eqdOffsetFlag")) {
			gpGponPriv->gponCfg.flags.eqdOffsetFlag = (action) ? GPON_EQD_OFFSET_FLAG_SUBTRACT : GPON_EQD_OFFSET_FLAG_ADD ;
			printk("%s eqdOffset %s the EqD\n", (action)?"Subtract":"Add",
												(action)?"from":"to") ;		
		} else if(!strcmp(subcmd, "eqdO4Offset")) {
			gpGponPriv->gponCfg.eqdO4Offset = action ;
			printk("Modify the EqD offset = %x at ranging_time PLOAM in O4\n", action) ;
		} else if(!strcmp(subcmd, "eqdO5Offset")) {
			gpGponPriv->gponCfg.eqdO5Offset = action ;
			printk("Modify the EqD offset = %x in O5\n", gpGponPriv->gponCfg.eqdO5Offset) ;
			gpon_dvt_eqd_adjustment(gpGponPriv->gponCfg.eqd) ;
		} 
		if(!strcmp(subcmd, "connection")) {
			gpon_dvt_onu_connection(action) ;
			printk("Finish to test the gpon ONU connection.\n") ;
		}
		if(!strcmp(subcmd, "hotplug")) {
			gpon_dvt_transceiver_on_off_connection(action) ;
			printk("Finish to test the gpon ONU hotplug connection.\n") ;
		}
		if(!strcmp(subcmd,"mpi")){
			printk("[%s][%d] call gpon disable.\n", __FUNCTION__, __LINE__) ;
		    gpon_disable();
		}
		if(!strcmp(subcmd,"asic_tod")){
		    gpon_dvt_asic_tod_GPIO22_cfg();
		}
		if(!strcmp(subcmd,"max_rdm_dly")){
			gpGponPriv->gponCfg.flags.maxRdmDlyFlag = action ;
			printk("test max_rdm_dly,after setting 1,the max_rdm_dly reg will not be reinitialized during the online process\n") ;
		}
	} else if(!strcmp(cmd, "ploam")) {
		if(!strcmp(subcmd, "dying")) {
			if(action == 0) {
				gpPonSysData->dyingGaspData.isGponHwFlag = GPON_SW ;	
				gponDevHardwareDyingGasp(GPON_SW) ;
				printk("The dying gasp message will be send by software\n") ;		
			} else if (action == 1) {
				gpPonSysData->dyingGaspData.isGponHwFlag = GPON_HW ;	
				gponDevHardwareDyingGasp(GPON_HW) ;
				printk("The dying gasp message will be send by hardware\n") ;		
			} else if (action == 0x10) {
			    if(gp_dying_info->opt_type == GPON_DG_SEND_ONEBYONE) 
                {
                    gpon_set_ploamu_int(TRUE); 
                    gp_dying_info->current_send_cnt = 0;
                }
				ploam_send_dying_gasp() ;
			} else if (action == 0x11 ) {
				REG_DBG_US_DYING_GASP_CTRL gponDGCtrl ;
				IO_SREG(G_PLOAMu_WDATA, 0) ;
				IO_SREG(G_PLOAMu_WDATA, 0) ;
				IO_SREG(G_PLOAMu_WDATA, 0) ;				
				gponDGCtrl.Raw = IO_GREG(DBG_US_DYING_GASP_CTRL) ;
				gponDGCtrl.Bits.dying_gasp_test = 1 ;	
				IO_SREG(DBG_US_DYING_GASP_CTRL, gponDGCtrl.Raw) ;
			}
		}
#if defined(TCSUPPORT_CPU_EN7526) || defined(TCSUPPORT_CPU_EN7527)
        if(!strcmp(subcmd,"rdi_enable"))
        {
            action = (action == 1)? 1 : 0;
            if(action != RDI_SEND_ENABLE){
                RDI_SEND_ENABLE = action;
                if(1 == RDI_SEND_ENABLE){
                    GPON_STOP_TIMER(rdi_timer);
                    RDI_TIMER_EN = 1;
                    GPON_START_TIMER(rdi_timer,RDI_SEND_DELAY);
                }else{
                    RDI_TIMER_EN = 0;
                    GPON_STOP_TIMER(rdi_timer);
                }
            }
            printk("Software RDI is %s \n",(RDI_SEND_ENABLE == 1) ? "enabled" : "disabled");
        }
        if(!strcmp(subcmd,"rdi_times"))
        {
            RDI_SEND_TIMES = action;
            printk("Software RDI is %s .Should send RDI for 0x%x times delay is %d\n",
                (RDI_SEND_ENABLE == 1) ? "enabled" : "disabled",RDI_SEND_TIMES,RDI_SEND_DELAY);
        }
        if(!strcmp(subcmd,"rdi_delay"))
        {
            RDI_SEND_DELAY = action;
            printk("Software RDI is %s .Should send RDI for 0x%x times delay is %d\n",
                (RDI_SEND_ENABLE == 1) ? "enabled" : "disabled",RDI_SEND_TIMES,RDI_SEND_DELAY);
        } if(!strcmp(subcmd,"rdi_send"))
        {
            gpon_isr_send_rdi();
        }
#endif
		if(!strcmp(subcmd,"hd_send_rdi_times"))
        {
            gpGponPriv->gponCfg.rdiSendNum = action;
            printk("hardware send RDI for 0x%x times\n",gpGponPriv->gponCfg.rdiSendNum);
        }
	} 
    else if(!strcmp(cmd, "g_dying")) 
    {    
        sscanf(val_string, "%s %s %u", cmd, subcmd, &action) ;
		if(!strcmp(subcmd, "delay")) 
        {
        	gp_dying_info->opt_type = GPON_DG_DELAY_US;
            gp_dying_info->delay_time = action;
            
            printk("The dying gasp send delay %u us\n",gp_dying_info->delay_time) ;
        }
        else if(!strcmp(subcmd, "test")) 
        {
        	char dev_id[] = "GPON";
        	xpon_dying_gasp_interrupt(0,dev_id);
        	printk("SW call xpon_dying_gasp_interrupt \n");
        }
        else if(!strcmp(subcmd, "onebyone")) 
        {
        	__u32 action2 = 0;
        	sscanf(val_string, "%s %s %u %u", cmd, subcmd, &action,&action2) ;
        	gp_dying_info->opt_type = GPON_DG_SEND_ONEBYONE;
            gp_dying_info->total_send_cnt = action;
            gp_dying_info->current_send_cnt = 0;
            gp_dying_info->delay_time = action2;
            
            printk("set ploamu_send_int enable\n");
            printk("The dying gasp send after last send success,total cnt %u delay interval %u \n",
                gp_dying_info->total_send_cnt,gp_dying_info->delay_time);
        }
        else if(!strcmp(subcmd, "reset")) 
        {
        	memset(&gp_dying_info,0,sizeof(gp_dying_info));
            gp_dying_info->opt_type = GPON_DG_NORMAL;
            gp_dying_info->dying_flag	= FALSE;
        	gpon_set_ploamu_int(FALSE); 
            printk("reset dying gasp normal, set ploamu_send_int disable\n");
        }
        else if(!strcmp(subcmd, "show"))
        {
        	printk("GPON dying gasp send by %s\n",(gpPonSysData->dyingGaspData.isGponHwFlag?"HW":"SW"));
            if(gpPonSysData->dyingGaspData.isGponHwFlag == GPON_HW)  return count; 
            switch(gp_dying_info->opt_type)
            {
            	case GPON_DG_NORMAL:
                    printk("GPON dying opt type Normal\n");
                    break;
                case GPON_DG_DELAY_US:
                    printk("GPON dying opt type DELAY interval is %d\n",gp_dying_info->delay_time);
                    break;
                case GPON_DG_SEND_ONEBYONE:
                    printk("GPON dying opt type ONEBYONE total send %d times, interval %u\n",
                        gp_dying_info->total_send_cnt,gp_dying_info->delay_time);
                    break;
                default:
                    break;
            }
        }
    }
    else if(!strcmp(cmd, "sn")) {
		struct XMCS_GponOnuInfo_S onuInfo ;
		struct XMCS_GponSnPasswd_S gponSnPasswd ;
		
		memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
		xmcs_get_onu_info(&onuInfo);
		memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
		
		memcpy(gponSnPasswd.sn, subcmd, 4) ;
		gponSnPasswd.sn[4] = (unchar)(action>>24) ;
		gponSnPasswd.sn[5] = (unchar)(action>>16) ;
		gponSnPasswd.sn[6] = (unchar)(action>>8) ;
		gponSnPasswd.sn[7] = (unchar)(action) ;
		xmcs_set_sn_passwd(&gponSnPasswd);
	} else if(!strcmp(cmd, "passwd")) {
		if(strlen(subcmd) <= GPON_PASSWD_LENS) {
			memset(gpGponPriv->gponCfg.passwd, 0, GPON_PASSWD_LENS) ;
			strcpy(gpGponPriv->gponCfg.passwd, subcmd) ;
			printk("Password: %s\n", gpGponPriv->gponCfg.passwd) ;
		}
	} else if(!strcmp(cmd, "show")) {
		if(!strcmp(subcmd, "xpon_print")) {
        	xpon_mac_print_open = action;
		}else if(!strcmp(subcmd, "pwan_drop")) {
        	drop_print_flag = action;
		}
		else if(!strcmp(subcmd, "gem")) {
			uint gemPortId, gemIdx ;
			gemInfo = kzalloc(sizeof(struct XMCS_GemPortInfo_S), GFP_ATOMIC);
			if(NULL == gemInfo){
				printk("func:%s line:%d kzalloc fail\n",__func__,__LINE__);
				return -1;
			}
			if(xmcs_get_gem_port_info(gemInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<gemInfo->entryNum ; i++) {
					gemPortId = gemInfo->info[i].gemPortId;
					gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);		
					printk("GEM ID:%d, Alloc ID:%d, If Idx:%d, Loopback:%s, Encryption:%s, valid is %d, chn:%d\n", 
								gemInfo->info[i].gemPortId, 
								gemInfo->info[i].allocId, 
								gemInfo->info[i].aniIdx, 
								(gemInfo->info[i].lbMode)?"ENABLE":"DISABLE", 
								(gemInfo->info[i].enMode)?"ENABLE":"DISABLE",
								gpWanPriv->gpon.gemPort[gemIdx].info.valid,
								gpWanPriv->gpon.gemPort[gemIdx].info.channel) ;
				}
			}
			kfree(gemInfo);
		}
		else if(!strcmp(subcmd, "static_hw"))
		{
		    int sipIndex = 13, dipIndex = 2, sport = 1025, dport = 1024, smacIdex = 1, s_vlan = 501;
		    char dstMac[20]="00:AA:BB:01:23:45";
			uint gemPortId, gemIdx ;
			gemInfo = kzalloc(sizeof(struct XMCS_GemPortInfo_S), GFP_ATOMIC);
			if(NULL == gemInfo){
				printk("func:%s line:%d kzalloc fail\n",__func__,__LINE__);
				return -1;
			}
			if(action == 0)
			    printk("generate static NAT rule\n");
			else
			    printk("generate static NATP rule(UDP)\n");
			if(xmcs_get_gem_port_info(gemInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<gemInfo->entryNum ; i++) {
					gemPortId = gemInfo->info[i].gemPortId;	
					gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);
					if(gpWanPriv->gpon.gemPort[gemIdx].info.channel == 0 || gpWanPriv->gpon.gemPort[gemIdx].info.channel == 32)
					    continue;
					if(action)
					{
					    printk("hw_nat -a -h %s -i 10:00:01:00:%02x:%02x -j 192.168.%d.%d -k 192.168.%d.%d -l %d -m %d -n 192.168.%d.%d -o 192.168.%d.%d -p %d -q %d ",
					                dstMac, (smacIdex>>8), (smacIdex&0xff),(10+(sipIndex>>8)), (sipIndex&0xff),(100+(dipIndex>>8)), (dipIndex&0xff),
					                sport, dport, (100+(dipIndex>>8)), (dipIndex&0xff), (10+(sipIndex>>8)), (sipIndex&0xff), dport, sport);
					    printk("-x 0 -y 0 -v Udp -Y 0 -3 %x -z ffffffff -w QDMA_HW -C 0x0 -D 0x3f -W 1 -s %X -F 1 -t %x\n", 					        
					        gpWanPriv->gpon.gemPort[gemIdx].info.channel, s_vlan, gemInfo->info[i].gemPortId) ;
					    sport += 2;
			            dport += 2;
					}
					else
					{
					    printk("hw_nat -a -h %s -i 10:00:01:00:%02x:%02x -j 192.168.%d.%d -k 192.168.%d.%d -n 192.168.%d.%d -o 192.168.%d.%d ",
					                dstMac, (smacIdex>>8), (smacIdex&0xff),(10+(sipIndex>>8)), (sipIndex&0xff),(100+(dipIndex>>8)), (dipIndex&0xff),
					                (100+(dipIndex>>8)), (dipIndex&0xff), (10+(sipIndex>>8)), (sipIndex&0xff));
					    printk("-G FD -x 0 -y 0 -Y 1 -W 1 -s %X -t %X -z 00fff051 -3 %x -L 2 -F 1\n", 					        
					         s_vlan, gemInfo->info[i].gemPortId,gpWanPriv->gpon.gemPort[gemIdx].info.channel) ;
					}
                    smacIdex++;
			        sipIndex += 2;
			        dipIndex += 2;
			        s_vlan++;
				}
			}
			kfree(gemInfo);
		}
		else if(!strcmp(subcmd, "llid"))
		{
			struct XMCS_EponLlidInfo_S Info;
			memset(&Info, 0, sizeof(Info));
			if(xmcs_get_llid_info(&Info) < 0)
			{
		    	printk("exec failed") ;
			}
			else
			{
				for(i=0 ; i<Info.entryNum ; i++) 
				{
					printk("idx = %d, llid = %d, Channel = %d, rxMode = %d, txMode = %d.\n", Info.info[i].idx, Info.info[i].llid, Info.info[i].channel, Info.info[i].rxMode, Info.info[i].txMode) ;
				}
			}
		}
		else if(!strcmp(subcmd, "tcont")) {
			struct XMCS_TcontInfo_S tcontInfo;
			memset(&tcontInfo, 0, sizeof(struct XMCS_TcontInfo_S));
			if(xmcs_get_tcont_info(&tcontInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<tcontInfo.entryNum ; i++) {
					printk("TCONT ID:%d, Channel:%d\n", tcontInfo.info[i].allocId, tcontInfo.info[i].channel) ;
				}
			}
		} else if(!strcmp(subcmd, "superframe")) {
			uint counter;
			gponDevGetSuperframe(&counter);
			printk("Super frame Counter = %x\n", counter);
		} else if(!strcmp(subcmd, "status")) {
			struct XMCS_GponOnuInfo_S onuInfo ;
			xmcs_get_onu_info(&onuInfo);
			printk("ONU Info:\n") ;
			printk("ONU ID: 	%d\n", onuInfo.onuId) ;
			printk("ONU State: 	%d\n", onuInfo.state) ;
			printk("SN: 		") ;
			for(i=0 ; i<4 ; i++) {
				printk("%c", onuInfo.sn[i]) ;
			}
			for(i=4 ; i<8 ; i++) {
				printk("%.2x", onuInfo.sn[i]) ;
			}
			printk("\nPASSWD: 	%s\n", onuInfo.passwd) ;
			printk("Key Idx: 	%d\n", onuInfo.keyIdx) ;
			printk("Key: 		") ;
			for(i=0 ; i<16 ; i++) {
				printk("%.2x", onuInfo.key[i]) ;
			}
			printk("\nO1 Timer: 	%d\n", onuInfo.actTo1Timer) ;
			printk("O2 Timer: 	%d\n", onuInfo.actTo2Timer) ;
			printk("OMCC ID: 	%d\n", onuInfo.omcc) ;
		} else if(!strcmp(subcmd, "counter")) {
			uint64_t rdata, tdata ;
			
			for(i=0 ; i<GPON_GEMPORT_MAX_NUM ; i++) {
				if(gpWanPriv->gpon.gemPort[i].info.valid) {
					if(action == gpWanPriv->gpon.gemPort[i].info.portId || action==0x10000) {
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_RX_FRAME_CNT, &rdata, 0) ;
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_TX_FRAME_CNT, &tdata, 0) ;
						printk("Gem Port:%d, SW Tx:%u, SW Rx:%u, HW Tx:%llu, HW Rx:%llu\n", 
										gpWanPriv->gpon.gemPort[i].info.portId,
										(uint)gpWanPriv->gpon.gemPort[i].stats.tx_packets, 
										(uint)gpWanPriv->gpon.gemPort[i].stats.rx_packets, 
										tdata, rdata) ;
					}
					else if(action == 0x20000)
					{
					    gponDevClearGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId);
					    memset(&(gpWanPriv->gpon.gemPort[i].stats), 0, sizeof(gpWanPriv->gpon.gemPort[i].stats));
					}
				}
			}
		} else if(!strcmp(subcmd, "gemcounter")) {
			struct XMCS_CounterCfg_S gponGemCounter ;
			uint tmpAction = 0;
			memset(&gponGemCounter, 0, sizeof(struct XMCS_CounterCfg_S)) ;
			sscanf(val_string, "%s %s %u %u", cmd, subcmd, &action, &tmpAction) ;
			gponGemCounter.gemPortId = (uint16_t)tmpAction;

			if(0 == action){
				get_counter_from_reg(&gponGemCounter, 0) ;
				printk("Read gemport counter no clear\n");
			}else{
				get_counter_from_reg(&gponGemCounter, 1) ;
				printk("Read gemport counter with clear\n");
			}
			printk("Gem Port:%d\n", gponGemCounter.gemPortId);
			printk("GemFrame Rx: %llu\n", gponGemCounter.sta.rxGemFrame) ;
			printk("GemPayload Rx: %llu\n", gponGemCounter.sta.rxGemPayload) ;
			printk("GemFrame Tx: %llu\n", gponGemCounter.sta.txGemFrame) ;
			printk("GemPayload Tx: %llu\n", gponGemCounter.sta.txGemPayload) ;
		}else if(!strcmp(subcmd, "gpon_recovery_backup")){
			printk("------ bakAllocId ------\n");
			for(i = 0; i < gpGponPriv->pGponRecovery->allocIdNum; i++)
			{
				printk("gponRecovery.allocId[%d] = %d\n", i, gpGponPriv->pGponRecovery->allocId[i]);
			}
			printk("\n");
			printk("------ bakGemport ------\n");
			for(i = 0; i < gpGponPriv->pGponRecovery->gemPortNum; i++)
			{
				printk("gponRecovery.gemPort[%d].allocId = %d\n", i, gpGponPriv->pGponRecovery->gemPort[i].allocId);
				printk("gponRecovery.gemPort[%d].gemPortId = %d\n", i, gpGponPriv->pGponRecovery->gemPort[i].gemPortId);
				printk("gponRecovery.gemPort[%d].channel = %d\n", i, gpGponPriv->pGponRecovery->gemPort[i].channel);
				printk("gponRecovery.gemPort[%d].encryption = %d\n", i, gpGponPriv->pGponRecovery->gemPort[i].encryption);
                printk("gponRecovery.gemPort[%d].ani = %d\n", i, gpGponPriv->pGponRecovery->gemPort[i].ani);
				printk("\n");
			}
		}
	} 
	else if(!strcmp(cmd, "phy")){
		if(!strcmp(subcmd, "nolosnoreadytimes")){
			phyNoLosNoReadyHandleCycle = action;
			printk("phyNoLosNoReadyHandleCycle is %d\n",phyNoLosNoReadyHandleCycle);
		}
	}
	else if(!strcmp(cmd, "rogue")) {
		if(!strcmp(subcmd, "get")) {
			PON_RogueStatus_t status;
			xmcs_get_rogue_status(&status);
			if(PON_STATUS_ROGUE ==	status)
				printk("Rogue ONU was detected!\n");
			else
				printk("No rogue ONU was detected!\n");
		}else if(!strcmp(subcmd, "clear")) {
			xmcs_clear_rogue_status();
			printk("Clear rogue ONU status!\n");
		}
	}
	else if(!strcmp(cmd, "gpon")) {
		if(!strcmp(subcmd, "startup")) {
			if((action == 0) || (action == 2)) { /*Disable or PowerDown*/
				xmcs_set_connection_start(action);
				//gpPonSysData->sysStartup = PON_WAN_STOP ;
				//gpon_disable() ;
				printk("Disable GPON ONU\n") ;
			} else {
				xmcs_set_connection_start(action);
				//gpPonSysData->sysStartup = PON_WAN_START ;
				//gpon_enable() ;
				printk("Enable GPON ONU\n") ;
			}
#if defined(TCSUPPORT_CPU_EN7580) || defined(TCSUPPORT_CPU_EN7528)
		}else if(!strcmp(subcmd, "reset")){
			if(action == 1){
				uint read_data = 0;
				int ret1 = 0;
				int ret2 = 0;
				ret1 = gponDevMpiStop(XPON_ENABLE);
				/*Reset GPON MAC*/
				IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET);
				delay1us(1);
				/*Reset GPON MAC*/
				IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET);
				ret2 = gponDevMpiStop(XPON_ENABLE);
				read_data = IO_GREG(0xbfaf0144);
				if(1 == ret1){
					printk("mpi stop 1 waiting timeout\n");
				}
				if(1 == ret2){
					printk("mpi stop 2 waiting timeout\n");
				}
				printk("Only reset GPON_MAC, REG 0xbfaf0144 = 0x%x\n",read_data);
			}
#endif
		}else if(!strcmp(subcmd, "only_mac_reset")){
			if(action == 1){
				uint read_data = 0;
				
				/*Reset GPON MAC*/
				IO_SBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET);
				delay1us(1);
				/*Reset GPON MAC*/
				IO_CBITS(SCU_RESET_REG, SCU_GPON_MAC_RESET);
				
				read_data = IO_GREG(0xbfaf0144);
				printk("Only reset GPON_MAC without MPI stop, REG 0xbfaf0144 = 0x%x\n",read_data);
			}	 
		} else if(!strcmp(subcmd, "clear")) {
			gponDevClearSwCounter();
			printk("Successful to clear the GEM counter\n") ;
		} else if(!strcmp(subcmd, "cntreset")) {
			gponDevClearHwCounter();
		} else if(!strcmp(subcmd, "silencetimer")) {
			gpGponPriv->gponCfg.silenceInterval = (action*1000 ) ;
			gpon_update_silence();
			printk("Silence Timer interval = %u seconds \n",gpGponPriv->gponCfg.silenceInterval) ;
		} else if(!strcmp(subcmd, "rei")) {
			int errCount = action ;
			ploam_send_rei_msg(errCount, &reiSeq) ;
			printk("Send REI PLOAM message successful\n") ;
		} else if(!strcmp(subcmd, "responseTime")) {
			 gpGponPriv->gponCfg.onuResponseTime = action ;
			printk("Set ONU Response Time to %x\n", gpGponPriv->gponCfg.onuResponseTime) ;
		} else if(!strcmp(subcmd, "preambleoffset")) {
			gpGponPriv->gponCfg.t3PreambleOffset = action ;
			printk("Set the O3/O4 type 3 preamble offset %d successful.\n", gpGponPriv->gponCfg.t3PreambleOffset) ;
		} else if(!strcmp(subcmd, "preambleFlag")) {
			gpGponPriv->gponCfg.flags.preambleFlag = action ;
			printk("%s the preambe length over 128 byte\n", (action)?"Enable":"Disable") ;		
		} else if(!strcmp(subcmd, "dbruSlightModify")){
			gpon_dbru_modify_size = action;
			printk("dbru modify size to %x\n",gpon_dbru_modify_size);
		} else if(!strcmp(subcmd, "dbruSlightModifyEn")) {
			gponDbruSlightModify(action);
		} else if(!strcmp(subcmd, "typeBdelay")) {
			typeB_delay_g  = action;
			printk("typeB_delay_g = %d\n",typeB_delay_g) ;
		} else if(!strcmp(subcmd, "xmit_dying_gasp")) {
			if (action > 0)
			{
				while (action)
				{
					xmit_dying_gasp_oam();
					action--;
				}
			}
		}
        else if (!strcmp(subcmd,"guard_time")){
            if(action > 255){
                printk("guard time is invalid \r\n");
            }else{
                gpGponPriv->gponCfg.phy_guard_bit_num = action;
                printk("set phy_guard_bit_num guard time to %d \r\n",gpGponPriv->gponCfg.phy_guard_bit_num);
            }
		}else if(!strcmp(subcmd, "intDly")) {
			gpGponPriv->gponCfg.internalDelayFineTune = action ;
			printk("Set ONU internal Delay Fine-Tune to %x\n", gpGponPriv->gponCfg.internalDelayFineTune) ;
        }
	} else if(!strcmp(cmd, "retire")) {
		ret = FE_API_SET_CHANNEL_RETIRE(FE_GDM_SEL_GDMA2, action, FE_LINKDOWN) ;

		if(ret == 0) {
			printk("Successful to set the channel %d retire\n", action) ;
		} else {
			printk("Failed to set the channel %d retire\n", action) ;
		}
	} else if(!strcmp(cmd, "pcp")) {
		if(!strcmp(subcmd, "auto")) {
			gpon_dvt_pcp_auto_testing() ;
			printk("Finished to auto test PCP function\n") ;
		} else {
			if(gpGponPriv->gponCfg.dvtPcpCounter) {
				printk("Last PCP check counter is %d, %d\n", gpGponPriv->gponCfg.dvtPcpCounter, gpGponPriv->gponCfg.dvtPcpCheckErr) ;
				gpGponPriv->gponCfg.dvtPcpCounter = 0 ;
			}
			
			if(!strcmp(subcmd, "disable")) {	
				gpGponPriv->gponCfg.dvtPcpCheck = 0 ;
				WRITE_FE_REG(0xbfb51514, 0) ;
			} else if(action==0x1 || action==0x2 || action==0x4 || action==0x8) {
				if(!strcmp(subcmd, "gdmrx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x40 | action) ;
					WRITE_FE_REG(0xbfb51514, (action<<8)) ;
				} else if(!strcmp(subcmd, "cdmrx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x20 | action) ;
					WRITE_FE_REG(0xbfb51514, (action<<4)) ;
				} else if(!strcmp(subcmd, "cdmtx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x10 | action) ;
					WRITE_FE_REG(0xbfb51514, (action)) ;
				}
			}
			printk("Successful to set the PCP %.8x\n", IO_GREG(0xbfb51514)) ;
		} 
	}

	else if(!strcmp(cmd, "epon"))
	{
        if(!strcmp(subcmd, "clnOAMRx"))
        {
            struct net_device *dev ;
            PWAN_NetPriv_T *pNetPriv = NULL ;
            dev = gpWanPriv->pPonNetDev[PWAN_IF_OAM] ;
	        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
            pNetPriv->stats.rx_packets = 0;
            pNetPriv->stats.rx_bytes = 0;
            printk("OAM TX packets: %lu\n", pNetPriv->stats.rx_packets);
        }

        if(!strcmp(subcmd, "showOAMRx"))
        {
            struct net_device *dev ;
            PWAN_NetPriv_T *pNetPriv = NULL ;
            dev = gpWanPriv->pPonNetDev[PWAN_IF_OAM] ;
	        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
            printk("0x%lx\n", pNetPriv->stats.rx_packets);
        }
        
        if(!strcmp(subcmd, "clnOAMTx"))
        {
            struct net_device *dev ;
            PWAN_NetPriv_T *pNetPriv = NULL ;
            dev = gpWanPriv->pPonNetDev[PWAN_IF_OAM] ;
	        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
            pNetPriv->stats.tx_packets = 0;
            pNetPriv->stats.tx_bytes = 0;
            printk("OAM TX packets: %lu\n", pNetPriv->stats.tx_packets);
        }

        if(!strcmp(subcmd, "showOAMTx"))
        {
            struct net_device *dev ;
            PWAN_NetPriv_T *pNetPriv = NULL ;
            dev = gpWanPriv->pPonNetDev[PWAN_IF_OAM] ;
	        pNetPriv = (PWAN_NetPriv_T *)netdev_priv(dev) ;
            printk("0x%lx\n", pNetPriv->stats.tx_packets);
        }
        
        if(!strcmp(subcmd, "bitmap"))
        {
            gpEponPriv->gBitmapSet = action;
            printk("gBitmapSet %s\n",(action)?"Enable":"Disable");
        }

        if(!strcmp(subcmd, "maxTryCnt"))
        {
            gpEponPriv->maxTryCnt = action;
            printk("maxTryCnt = %d\n", gpEponPriv->maxTryCnt);
        }
        
		if(!strcmp(subcmd, "laser_time"))
		{
			g_EponLaserTime = action;
			printk("epon laser time del %d\n",g_EponLaserTime);
		}
		else if(!strcmp(subcmd, "grtLen"))
		{
		    gpEponPriv->eponRdmMfy.grtLen = action;
		    printk("grant length: 0x%x\n", action);
		}
		else if(!strcmp(subcmd, "grtMode"))  /*0:fix, 1:dynamic*/
		{
		    gpEponPriv->eponRdmMfy.grtMode = action;
		    printk("grant mode: 0x%x\n", action);
		}
		else if(!strcmp(subcmd, "grtFlag"))
		{
		    gpEponPriv->eponRdmMfy.grtFlag = action;
		    printk("grant length modify %s\n",(action)?"Enable":"Disable");
		}
		else if(!strcmp(subcmd, "mpcp_fwd"))
		{
		    gpEponPriv->fwdFlag = action;
			setMpcpFwd(action);
			printk("set epon mpcp fwd %s\n",(action)?"Enable":"Disable");
		}
		else if(!strcmp(subcmd, "set_llid"))
		{
			printk("epon set llid %d traffic up\n",action);
			xmcs_set_epon_llid_config(action);
		}
		if(!strcmp(subcmd, "silent_time"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &action) ;
			g_silence_time = action;
			printk("epon set silent time %d\n",g_silence_time);
		}
		if(!strcmp(subcmd, "default_threshold"))
		{
		    sscanf(val_string, "%s %s %d", cmd, subcmd, &action);
		    eponDrv.DefaultThreshold = action;
		    eponSetLlid0DeafultThreshold();
		    printk("epon set default threshold %d\n",eponDrv.DefaultThreshold);
		}
		if(!strcmp(subcmd, "fec_adjust"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &action) ;
			eponDrv.eponFecAdjust = action;
			printk("epon set fec adjust %d\n",eponDrv.eponFecAdjust);
		}        
	}
	else if(!strcmp(cmd, "assign"))
	{
		ushort gemportId = 0;
		if(!strcmp(subcmd, "gemport"))
		{
			gemportId = action;
			assignGemportId(gemportId);
		}
	}	
	else if(!strcmp(cmd, "bip_cnt")) 
	{
			if(!strcmp(subcmd, "show")) 
			{
				unsigned int bip_cnt_tmp = 0;
				bip_cnt_tmp = XPON_PHY_GET(PON_GET_PHY_BIP_COUNTER);
				gpGponPriv->bip_cnt_val += bip_cnt_tmp;
				printk("current_bip_cnt_counter = %u, total_bip_cnt_counter = %u\n",bip_cnt_tmp,gpGponPriv->bip_cnt_val);
			}
			else if(!strcmp(subcmd, "clear")) 
			{
				gpGponPriv->bip_cnt_val = 0;
			}
	}
	else if(!strcmp(cmd, "emergence_state"))
	{
		if(!strcmp(subcmd, "show"))
		{
			printk("show GPON emergence_state begin\n");
			gpon_show_emergence_info();
		}
	}
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_FWC_ENV)
	else if(!strcmp(cmd, "fhnet_mapping") ) 
	{
		uint value1, value2=0, value3 ;
		sscanf(val_string, "%s %x %d %x %x", cmd, &value1, &value2, &value3, &gFhDrop) ;
		fhNetMappingDebug = value1;
		gMappingRet.gemportId = value2;
		gMappingRet.queueId = value3;
		fhnet_set_vlan_mapping_hook();
		printk("fhnet_mapping debug %s gemport %d queueId %d\n",value1?"enable":"disbale",value2,value3);
	}
#endif/*TCSUPPORT_COMPILE*/
#ifdef TCSUPPORT_VNPTT
	else if(!strcmp(cmd, "ignore_deactive"))
	{
		if(!strcmp(subcmd, "disable"))
		{
			printk("no ignore deactive \n");
			ignore_deactive_flag = FALSE;
		}
		else if(!strcmp(subcmd, "enable"))
		{
			printk("ignore deactive \n");
			ignore_deactive_flag = TRUE;
		}
	}
#endif
#endif /* TCSUPPORT_WAN_GPON */
#endif /* XPON_MAC_CONFIG_DEBUG */
	else if(!strcmp(cmd, "epon_test") )
	{
		if(!strcmp(subcmd, "holdoverflag")){
			eponDrv.hldoverEnable = action;
			printk("hldOverFlag = %d\n",eponDrv.hldoverEnable);
		}
		else if(!strcmp(subcmd, "typeBOnGoing")){
			eponDrv.typeBOnGoing = action;
			printk("typeBOnGoing = %d\n",eponDrv.typeBOnGoing);
		}
		else if(!strcmp(subcmd, "time")){
			eponDrv.hldOverTime= action;
			printk("hldOverTime = %d\n",eponDrv.hldOverTime);
		}
	}
#ifdef CONFIG_USE_FOR_TEST
	else if(!strcmp(cmd, "eqdAdjust")) {
		if(!strcmp(subcmd, "debug")) {
			eqdDbgFlag = action;
		}else if(!strcmp(subcmd, "show")) {
			printk("###EqdCounter == %d###\n", EqdCounter);
			for(i=0; i<EqdCounter; i++)
				printk("EqdArray[%d] == %d\n", i,EqdArray[i]);
		}else if (!strcmp(subcmd, "empty")){
			EqdCounter = 0;
			memset(EqdArray,0,1001*sizeof(uint));
		}
	}else if(!strcmp(cmd, "gem_valid")) {
		if(!strcmp(subcmd, "modify"))
		{
		    int tmp = (action == 0) ? 0 : 1;
   		/*test init work-queue*/
			if(!gem_valid_work_queue)
    			gem_valid_work_queue = create_workqueue("gem_valid_work_queue");
		    atomic_set(&work_queue_valid,tmp);
            if(tmp && gem_valid_work_queue )
                queue_work(gem_valid_work_queue, &gem_valid_work_queue_struct);
            printk("%s gem port vlaid--invalid workqueue \r\n",tmp?"Start":"End");
        }
	}else if(!strcmp(cmd, "omci_drop")){
		if(!strcmp(subcmd, "1")){
			omci_drop_flag = 1;
			printk("Start drop omci packets\n");
		}else{
			omci_drop_flag = 0;
			printk("Stop drop omci packets\n");
		}
	}else if(!strcmp(cmd, "hgu")){
		if(!strcmp(subcmd, "txq")){
			gpWanPriv->gpon.hgu_mode_txq = action;
		}
		printk("Hgu mode txq is: %d %s\n", gpWanPriv->gpon.hgu_mode_txq,
			gpWanPriv->gpon.hgu_mode_txq ? "OMCI Config" : "Local Config"); 
	}else if(!strcmp(cmd, "set_omci")){
		if(!strcmp(subcmd, "gem_port")){
			REG_G_OMCI_ID gponOmciId ;
			ushort port_id ;
			port_id = action;

			printk("Set omci gem port : %d \n",port_id);
			gpGponPriv->gponCfg.omcc = port_id ;
			
			/* Setting the MAC registger */
			/* activate the port id of OMCI channel */
			gponOmciId.Raw = IO_GREG(G_OMCI_ID) ;
			gponOmciId.Bits.omci_port_id_vld = 1 ;
			gponOmciId.Bits.omci_gpid = port_id ;
			IO_SREG(G_OMCI_ID, gponOmciId.Raw) ;
		}
	}
#endif /* CONFIG_USE_FOR_TEST */
	else if(!strcmp(cmd, "set"))
		{
			if(!strcmp(subcmd, "TypeOfAction"))
			{
				gTypeOfAction = action;
				printk("the versionNum is %d\n",gTypeOfAction);
			}
			
		}
	else if(!strcmp(cmd, "rm_tcont_delay") )
	{
	    g_rm_tcont_delay_us = simple_strtoul(subcmd, &endpo, 10);
		
		printk("g_rm_tcont_delay_us = %d\n",g_rm_tcont_delay_us);
	}else if(!strcmp(cmd, "tbl_init")){
		if(!strcmp(subcmd, "start")){
			REG_G_MIB_TBL_INIT gponMibTblInit ;
			REG_G_GPIDX_TBL_INIT gponGPIdxTblInit ;
			REG_G_GEM_TBL_INIT gponGemTblInit ;
			REG_G_TX_FCS_TBL_INIT gponTxFCSTblInit ;

			int times = 1;
			gponMibTblInit.Raw = 0 ;
			gponMibTblInit.Bits.mib_tbl_init_start = 1 ;
			IO_SREG(G_MIB_TBL_INIT, gponMibTblInit.Raw) ;
			while(times++){
				gponMibTblInit.Raw = IO_GREG(G_MIB_TBL_INIT) ;
				if(gponMibTblInit.Bits.mib_tbl_init_done) {
					printk("MIB table initialization done times: %d\n",times);
					break ;
				} 				
			}
			times = 1;
			gponGPIdxTblInit.Raw = 0 ;
			gponGPIdxTblInit.Bits.gpidx_tbl_init_start = 1 ;
			IO_SREG(G_GPIDX_TBL_INIT, gponGPIdxTblInit.Raw) ;
			while(times++){
				gponGPIdxTblInit.Raw = IO_GREG(G_GPIDX_TBL_INIT) ;
				if(gponGPIdxTblInit.Bits.gpidx_tbl_init_done) {
					printk("Gem port index table initialization done times: %d\n",times);
					break ;
				}				
			}
			times = 1;
			gponGemTblInit.Raw = 0 ;
			gponGemTblInit.Bits.gem_tbl_init_start = 1 ;
			IO_SREG(G_GEM_TBL_INIT, gponGemTblInit.Raw) ;
			while(times++){
				gponGemTblInit.Raw = IO_GREG(G_GEM_TBL_INIT) ;
				if(gponGemTblInit.Bits.gem_tbl_init_done) {
					printk("Gem port table initialization done times: %d\n",times);
					break ;
				}				
			}
			times = 1;
			gponTxFCSTblInit.Raw = 0 ;
			gponTxFCSTblInit.Bits.tx_fcs_tbl_init_start = 1 ;
			IO_SREG(G_TX_FCS_TBL_INIT, gponTxFCSTblInit.Raw) ;
			while(times++){
				gponTxFCSTblInit.Raw = IO_GREG(G_TX_FCS_TBL_INIT) ;
				if(gponTxFCSTblInit.Bits.tx_fcs_tbl_init_done) {
					printk("TX FCS table initialization done times: %d\n",times);
					break ;
				}				
			}
		}
    }else if(!strcmp(cmd, "grant_size_sum")) {
		if(!strcmp(subcmd, "start"))
		{
			uint gtc_num = 0;
			uint spfCnt = 0;
			int times = 1;
		
#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)		 
			uint tcont_sel = 0; 		
			REG_CAL_GNT_SIZE_TCONT_EN calGntSizeTcontEn;
			REG_CAL_GNT_SIZE_CTRL_STS calGntSizeCtrlSts ;
			REG_CAL_GNT_SIZE_SUM_TCONT_SEL calGntSizeSumTcontSel;
			REG_CAL_GNT_SIZE_SUM_L32 calGntSizeSumL32;
			REG_CAL_GNT_SIZE_SUM_H32 calGntSizeSumH32;
			
			sscanf(val_string, "%s %s %x %x %u", cmd, subcmd, &action,&gtc_num,&tcont_sel);
			
			calGntSizeTcontEn.Raw = IO_GREG(CAL_GNT_SIZE_TCONT_EN);
			calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS);			
			calGntSizeTcontEn.Bits.cal_gnt_size_tcont_en = action;
			calGntSizeCtrlSts.Bits.cal_gnt_size_gtc_num = gtc_num;
			calGntSizeCtrlSts.Bits.cal_gnt_size_start = 1;

			IO_SREG(CAL_GNT_SIZE_TCONT_EN, calGntSizeTcontEn.Raw);
			IO_SREG(CAL_GNT_SIZE_CTRL_STS, calGntSizeCtrlSts.Raw);
			
			gponDevGetSuperframe(&spfCnt);
			printk("7581 gpon Calculate grant size start super frame counter = %08x\n", spfCnt);
			
			while(times++) 
			{
				calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS) ;
				if(1 == calGntSizeCtrlSts.Bits.cal_gnt_size_done) 
				{
					calGntSizeSumTcontSel.Raw = IO_GREG(CAL_GNT_SIZE_SUM_TCONT_SEL);
					calGntSizeSumTcontSel.Bits.cal_gnt_size_sum_tcont_sel = tcont_sel;
					IO_SREG(CAL_GNT_SIZE_SUM_TCONT_SEL, calGntSizeSumTcontSel.Raw);

					calGntSizeSumL32.Raw = IO_GREG(CAL_GNT_SIZE_SUM_L32);
					calGntSizeSumH32.Raw = IO_GREG(CAL_GNT_SIZE_SUM_H32);	
					
					printk("Calculate grant size sum_h32 = %.8x, sum_l32 = %.8x\n", calGntSizeSumH32.Raw, calGntSizeSumL32.Raw);
					printk("Calculate grant size done times: %d\n",times);
					break ;
				} 
			}
#else 
			REG_CAL_GNT_SIZE_CTRL_STS calGntSizeCtrlSts ;
			sscanf(val_string, "%s %s %u %u", cmd, subcmd, &action,&gtc_num);
			calGntSizeCtrlSts.Raw = 0 ;
			calGntSizeCtrlSts.Bits.cal_gnt_size_alloc_id = action;
			calGntSizeCtrlSts.Bits.cal_gnt_size_gtc_num = gtc_num;
			calGntSizeCtrlSts.Bits.cal_gnt_size_start = 1 ;
			IO_SREG(CAL_GNT_SIZE_CTRL_STS, calGntSizeCtrlSts.Raw) ;
			gponDevGetSuperframe(&spfCnt);
			printk("Calculate grant size start super frame counter = %08x\n", (spfCnt+1));
			while(times++) {
				calGntSizeCtrlSts.Raw = IO_GREG(CAL_GNT_SIZE_CTRL_STS) ;
				if(calGntSizeCtrlSts.Bits.cal_gnt_size_done) {
					printk("Calculate grant size done times: %d\n",times);
					break ;
				} 
			}
#endif			
		}
	}else if(!strcmp(cmd, "hotplug"))
	{
		if(!strcmp(subcmd, "set"))
		{
			gpGponPriv->gTypeOfHotplug = action;
			printk("%s hotplug func \r\n",gpGponPriv->gTypeOfHotplug?"Start":"Stop");
		}
	}else if(!strcmp(cmd, "lof"))
	{
		if(!strcmp(subcmd, "enable")){
			gpGponPriv->lofEnableFlag = 1;
		}else if(!strcmp(subcmd, "disable")){
			gpGponPriv->lofEnableFlag = 0;
		}
		printk("current lof control status in O5 is %d\n",gpGponPriv->lofEnableFlag);
	}
	else if(!strcmp(cmd, "stop_rouge"))
	{
		if(!strcmp(subcmd, "enable"))
		{
			stop_rouge_tx_enable = action;
		}
		printk("stop_rouge_tx_enable = %d, %s\n",stop_rouge_tx_enable,
				stop_rouge_tx_enable ? "XPON_PHY_TX_ENABLE" : "XPON_PHY_TX_DISABLE");
	}else if( !strcmp(subcmd, "channel_retire_en"))
    {
        if(!strcmp(subcmd, "enable"))
		{
			channel_retire_en = XPON_ENABLE;
		    printk("INVALID channel do one channel retire ENABLE\n");
		}else{
            channel_retire_en = XPON_DISABLE;
		    printk("INVALID channel do one channel retire DISABLE\n");
        }
    }
	else if(!strcmp(cmd, "dump_gem"))
	{
		if(!strcmp(subcmd, "enable"))
		{
			dump_all_gemport = action;
		}
		
		printk("dump_all_gemport = %d, %s\n\n",dump_all_gemport,
				dump_all_gemport ? "dump all gemport:" : "dump valid gemport:");
		
		if(0 != gponDevDumpGemInfo())
		{
			printk("gponDevDumpGemInfo return error!\n");
		}
	}
#if defined(TCSUPPORT_CPU_EN7581)
	else if(!strcmp(cmd, "tod"))
	{
		if(!strcmp(subcmd, "adjust"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &gpon_tod_adjust) ;
			printk("xpon_tod_adjust = %dns\n",gpon_tod_adjust);
		}
		else
		{
			printk("subcmd input error,do nothing");
		}
	}
#endif
		else if(!strcmp(cmd, "alarm"))
	{
		if(!strcmp(subcmd, "keep_time"))
		{
			sscanf(val_string, "%s %s %d", cmd, subcmd, &action) ; //Decimal easy to use
			gpGponPriv->gponAlmKeepTime = action;
			if(gpGponPriv->gponAlmKeepTime == 0){
				gpon_reset_alarm_jiffiesClk();
			}
		}
		else if(!strcmp(subcmd, "clear_all"))
		{
			gpon_clear_all_alarm();
		}
		else{
			printk("invalue command!\noptional command: keep_time [TIME]; clear_all\n");
			printk("Note: if want to see the alarm history, i.e. no any clear action, set the [TIME] as 0\n");
		}

		printk("cur alarm keep_time = %d\n",gpGponPriv->gponAlmKeepTime);
	}
	else if(!strcmp(cmd, "set_alarm") || !strcmp(cmd, "clear_alarm"))
	{
		gpon_alarm_proc_write(cmd, subcmd);
	}
    else if(!strcmp(cmd, "RDK_GTC"))
    {
        rdkb_write_gtc_count(subcmd,action);
    }
	else if(!strcmp(cmd, "distance_adjust"))
	{
		if(!strcmp(subcmd, "add"))
		{
			distanceAdjust = action;
			printk("add distanceAdjust = %d\r\n",distanceAdjust);
		}else if(!strcmp(subcmd, "del")){
			distanceAdjust = -action;
			printk("delete distanceAdjust = %d\r\n",distanceAdjust);
		}else if(!strcmp(subcmd, "show")){
			uint32_t olt_distance = 0;
			xmcs_get_olt_distance(&olt_distance);
			printk("onu cal olt distance = %d\n", olt_distance);
		}else{
			printk("distanceAdjust = %d\r\n",distanceAdjust);
		}
	}
		else if(!strcmp(cmd, "wait_queue")){
		if(!strcmp(subcmd, "stop")){
			gpMcsPriv->xmcsEventStatus.items = 1;
		    wake_up_interruptible(&gpMcsPriv->xmcsWaitQueue) ;
		}
	}
	else if(!strcmp(cmd, "hw_cnt"))
	{
		if(!strcmp(subcmd, "enable"))
		{
			hw_cnt_enable = action;
			printk("hw_cnt_enable = %d\r\n",hw_cnt_enable);
		}
	}

	return count ;
}

static int gpon_fast_slow_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[96] = {0}, cmd[32]={0}, subcmd[32]={0};
	// int value = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	// sscanf(val_string, "%d", &value);
	sscanf(val_string, "%31s %31s", cmd, subcmd) ;
	val_string[count] = '\0';
	if(!strcmp(cmd, "fast")){
		if(!strcmp(subcmd, "enable") || !strcmp(subcmd, "1")) {
			gpon_fast_mode_flag= 1;
		} else {
			gpon_fast_mode_flag= 0;
		}
	} else{
	
	}
	
	// printk("gpon_fast_mode value is written %d\n", fast_mode);
	return count;
}

/******************************************************************************
******************************************************************************/
void gpon_dev_get_error_status(void)
{
	GPON_DEV_ERR_STATUS_T status;

	memset(&status, 0x0, sizeof(GPON_DEV_ERR_STATUS_T));

	gponDevGetErrStatus(&status);

	printk("Rx ethernet FCS error:                              %d\n", status.dbg_grp_0.Bits.ether_fcs_err) ;
	printk("Received Alen field is not equal to zero:           %d\n", status.dbg_grp_1.Bits.alen_neq_zero) ;
	printk("Rx PLOAMd CRC error:                                %d\n", status.dbg_grp_1.Bits.ploamd_crc_err) ;	
	printk("Rx Loss of GEM Delineation:                         %d\n", status.dbg_grp_1.Bits.logd) ;
	printk("Received more than two interGEM frag interrupt:     %d\n", status.intStatus.Bits.rx_gem_intlv_err_int) ;
	printk("phy_rx_eof signal error interrupt:                  %d\n", status.intStatus.Bits.rx_eof_err_int) ;
	printk("Loss of GEM delineation interrupt:                  %d\n\n", status.intStatus.Bits.los_gem_del_int) ;

	printk("Tx alignment FIFO underrun:                         %d\n", status.dbg_grp_1.Bits.tx_align_fifo_udrn) ;
	printk("Tx start time is later than Tx counter:             %d\n", status.dbg_grp_1.Bits.tx_late_start) ;
	printk("Tx local time late interrupt:                       %d\n\n", status.intStatus.Bits.tx_late_start_int) ;

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	printk("Last grant not enough pend FEC 16byte parity:       %d\n", status.dbg_grp_0.Bits.my_bst_last_gnt_len_err) ;
#endif
	printk("Payload FIFO in MBI module overrun:                 %d\n", status.dbg_grp_0.Bits.mbi_pl_fifo_ovrn) ;
	printk("AES engine random cipher FIF0 overrun occur:        %d\n", status.dbg_grp_0.Bits.aes_rdm_ciph_fifo_ovrn) ;
	printk("BWM burst FIFO next full:                           %d\n", status.dbg_grp_1.Bits.bst_next_full) ;
	printk("BWM single FIFO next full:                          %d\n", status.dbg_grp_1.Bits.sgl_next_full) ;
	printk("AES engine crypto counter FIFO overrun:             %d\n", status.dbg_grp_1.Bits.aes_cryp_cnt_fifo_ovrn) ;
	printk("AES engine cipher text FIFO overrun:                %d\n", status.dbg_grp_1.Bits.aes_ciph_txt_fifo_ovrn) ;
	printk("GEM header FIFO overrun:                            %d\n", status.dbg_grp_1.Bits.mbi_hdr_fifo_ovrn) ;
	printk("BWM un-correctable error:                           %d\n", status.dbg_grp_1.Bits.bwm_uc_err) ;
	printk("Blen filed error:                                   %d\n", status.dbg_grp_1.Bits.blen_err) ;
	printk("US FEC status error interrupt:                      %d\n", status.intStatus.Bits.bwm_us_fec_err_int) ;
	printk("OLT assigned stop time error when US FEC on:        %d\n", status.intStatus.Bits.bwm_stop_time_err_int) ;
	printk("When sfifo_next_full asserted, then sgl_wr happen:  %d\n", status.intStatus.Bits.sfifo_full_int) ;
	printk("When sfifo_next_full asserted, then bst_wr happen:  %d\n", status.intStatus.Bits.bfifo_full_int) ;
	printk("Burst and single grant buffer mismatch interrupt:   %d\n\n\n", status.intStatus.Bits.bst_sgl_diff_int) ;

	return;
}
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t gpon_err_status_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	gpon_dev_get_error_status();
	return 0;
}
/*****************************************************************************
******************************************************************************/
static ssize_t gpon_err_status_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[40]={0};
	int cmd = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buf, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';

	if(cmd == 1){
		gponDevSetErrStatusClear();
		printk("gpon mac set error status clear!\n");
	}
	
	return count;
}
#else
static int gpon_err_status_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	gpon_dev_get_error_status();
	return 0;
}
/*****************************************************************************
******************************************************************************/
static int gpon_err_status_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)	
{
	char val_string[40]={0};
	int cmd = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL;
	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;
	
	sscanf(val_string,"%d",&cmd);
	val_string[count] = '\0';

	if(cmd == 1){
		gponDevSetErrStatusClear();
		printk("gpon mac set error status clear!\n");
	}
	
	return count;
}
#endif

/*****************************************************************************
******************************************************************************/
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	static int qdma_reg_init(void)
	{
		int ret = 0;
		QDMA_txCngstCfg_t pTxCngstCfg;
		QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
		QDMA_TxBufCtrl_T txBufUsage;
		
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
		pTxCngstCfg.txCngstMode = QDMA_TXCNGST_DYNAMIC_NORMAL;
		QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(ECNT_QDMA_WAN, &pTxCngstCfg);
		QDMA_API_SET_TXQ_DEI_DROP_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);

		memset(&txqDynCngstThrh, 0, sizeof(QDMA_TxQDynCngstThrh_T));
		QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
		/* default for single Tcont */
		txqDynCngstThrh.dynCngstChnlMinThrh = 0xA0;
		QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
	
		/* limite PSE buffer usage for each channel, then
		 * low priority packes won't block high priority
		 * packets (in PSE buffer) for too long, in order
		 * to prevent highest priority packets from dropping */
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		txBufUsage.mode = QDMA_ENABLE;
		txBufUsage.chnThreshold = 0x20; //PSE blocks
		txBufUsage.totalThreshold = 0x20;
		if(!(isEN7580 || isEN7523 || isEN7581 || isAN7583 || isAN7552)){
			ret = QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufUsage);
            if (ret) {
    			printk("\nFAILED(%s): qdma setting for txBufUsage\n" , __FUNCTION__);
    			return -1;
    		}
		}

#endif
	
		return 0;
	}

#endif
static int xpondrv_qdma_init(void)
{
	QDMA_InitCfg_t qdmaInitCfg ;
	uint dbg=0 ;
	
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	/* HW QDMA QoS init */
	qdma_reg_init();
#endif
		
	memset(&qdmaInitCfg, 0, sizeof(QDMA_InitCfg_t)) ;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HYBIRD)
        qdmaInitCfg.cbRecvPkts = hybird_pwan_cb_rx_packet ;
#else/*TCSUPPORT_COMPILE*/
        qdmaInitCfg.cbRecvPkts = pwan_cb_rx_packet ;
#endif/*TCSUPPORT_COMPILE*/
	qdmaInitCfg.cbEventHandler = pwan_cb_event_handler ;

	QDMA_API_INIT(ECNT_QDMA_WAN, &qdmaInitCfg);
	/*if rmmod the xpon_10g,the QDMA WAN RX INT will be disabled,so we should enable it in case*/
	QDMA_API_ENABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
	QDMA_API_TX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_RX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_LOOPBACK_MODE(ECNT_QDMA_WAN, QDMA_LOOPBACK_DISABLE) ;

    PON_MSG(MSG_TRACE, "Prepare %d receive packet buffers in QDMA init stage.\n", dbg) ;

	return 0 ;
}

static void xpondrv_qdma_deinit(void)
{
	QDMA_InitCfg_t qdmaInitCfg;

	/*when rmmod the xpon_10g,we should disable QDMA WAN RX INT*/
	QDMA_API_DISABLE_RXPKT_INT(ECNT_QDMA_WAN) ;
	
	memset(&qdmaInitCfg, 0x0, sizeof(QDMA_InitCfg_t));
	QDMA_API_INIT(ECNT_QDMA_WAN, &qdmaInitCfg);
}


#ifndef TCSUPPORT_CPU_ARMV8
void delay1us(int us)
{
	volatile __u32 timer_now, timer_last;
	volatile __u32 tick_acc;
	__u32 one_tick_unit = 1 * SYS_HCLK / 2;
	volatile __u32 tick_wait = us * one_tick_unit;
	volatile __u32 timer1_ldv = regRead32(CR_TIMER1_LDV);

	tick_acc = 0;
 	timer_last = regRead32(CR_TIMER1_VLR);
	do {
   		timer_now = regRead32(CR_TIMER1_VLR);
       	if (timer_last >= timer_now)
       		tick_acc += timer_last - timer_now;
      	else
       		tick_acc += timer1_ldv - timer_now + timer_last;
     	timer_last = timer_now;
	} while (tick_acc < tick_wait);
}/*end delay1us*/
#endif
/*****************************************************************************
******************************************************************************/
irqreturn_t xpon_dying_gasp_interrupt(int irq, void *dev_id)
{
#ifdef TCSUPPORT_CPU_ARMV8
    u32 raw = 0;	
#endif
    DGASP_OPTIMIZE_PON();
#ifdef TCSUPPORT_DYING_GASP_OPTIMIZATION
         if(isEN751221)
         {      
                /* power cost from large to small */
                VPint(CR_INTC_IMR) = 0x0; /* stop all interrupt.*/		
	       VPint(0xbfb40050) = 0x0;  /* stop all interrupt.*/
               
                /*shutdown zarlink slic*/
                /*
                uint8 temp_val =0x04;
                VpMpiCmd(0,0x1,0x04,1, &temp_val);
                */
               
                /*PCIe gen  POWER DOWN */
                VPint(0xbfaf2000) = 0x7f7f0010; /* PCIe gen1 */
                VPint(0xbfac030c) = 0x80000000; /*  PCIe gen2 */

                VPint(0xbfb000c4) = 0x1000603;/* usb port0 + p1 phy clock bit[25][26][27]*/
                VPint(0xbfb000ec) = 0xe0; /* usb port0 + p1 phy reset [6][7][8]*/

                /*SCU reset*/
                VPint(0xbfb00834) = ~((1<<8) | (1<<13) | (1<<31)); /*reset all block exclude timer, GPIO , pon*/   

                /*Disable PPE*/
                VPint(0xbfb50e00) &= ~0x1; 
                
                
                timerSet(5, 2000, ENABLE, TIMER_TOGGLEMODE, TIMER_HALTDISABLE);
                timer_WatchDogConfigure(ENABLE, ENABLE);      
         }
#endif
/* Clear pre dying gasp interrupt status */
#ifdef TCSUPPORT_CPU_ARMV8
	raw = GET_DMTC();
	raw = raw | SCU_DYING_GASP_STATUS;
	SET_DMTC(raw);
#else
	IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ; 
#endif

#ifdef TCSUPPORT_WAN_GPON
	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		if (gpPonSysData->dyingGaspData.isGponHwFlag == GPON_SW) {
			gpon_detect_dying_gasp();
		}
	}
#endif 	/* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	else	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {	
		if (!gpPonSysData->dyingGaspData.isEponHwFlag) {
			xmit_dying_gasp_oam();
		}
	}
#endif /* TCSUPPORT_WAN_EPON */

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_DYINGGASP) ;
#endif/*TCSUPPORT_COMPILE*/

#ifdef TCSUPPORT_DYING_GASP_OPTIMIZATION
        dying_gasp_setup_mem_cpu();
#endif

#if defined(TCSUPPORT_DYING_GASP) && defined(TCSUPPORT_CPU_ARMV8)
    DGASP_OPTIMIZE_PON();
#endif

	return IRQ_HANDLED;
}

/*****************************************************************************
******************************************************************************/
static int xpon_dying_gasp_init(void)
{
	 /* Register Dying Gasp interrupt */
#ifdef TCSUPPORT_CPU_ARMV8
	 int irq = -1;
	 u32 raw = 0;
	 struct device* dev = NULL;
     dev = get_xpon_dev();
	 irq = get_xpon_irq(1);
	 
	 /* Clear pre dying gasp interrupt status */
	 raw = GET_DMTC();
	 raw = raw | SCU_DYING_GASP_STATUS;
	 SET_DMTC(raw);
	 if(request_irq(irq, xpon_dying_gasp_interrupt, 0, "dying gasp", dev) != 0) {
		 printk("Request the interrupt service routine fail, irq:%d.\n", irq) ;
		 return -1 ;
	 }
#else
	/* Clear pre dying gasp interrupt status */
	IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ;
	if(request_irq(DYINGGASP_INT, xpon_dying_gasp_interrupt, 0, "dying gasp", NULL) != 0) {
		printk("Request the interrupt service routine fail, irq:%d.\n", DYINGGASP_INT) ;
		return -ENODEV ;
	}
#endif
	
	return 0;
}

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_BOSA_CARLIBRATION)
#define GPON_PHY_EEPROM_SIZE  256
#define GPON_PHY_I2C_ADDR1 0x50
#define GPON_PHY_I2C_ADDR2 0x51
#define MAX_ARGS 160
#define MAX_ARG_SIZE 8
/* file system */
enum fs_enum {PROC_FS, SYS_FS};

//bob data calibration
#define BOB_TABLE_SIZE 128
#define BOB_TABLE_NUM 10

static int writeToBobFileViaI2C(void)
{
	UINT8 tableIndex = 0;
	UINT8 *tmpBuffer=NULL;
	__u32 bufOffset=0;
	__u32 offset=0;
	int i=0;
	UINT8 readBaseLen=32; // read 32 bytes one time
	struct file				*srcf = NULL;
	__u32 tmp_fpos = 0;
	char *src = NULL;
	mm_segment_t			orgfs;	

	const int bufSize = BOB_TABLE_SIZE*BOB_TABLE_NUM;
	int  reval = 0;
	tmpBuffer = kmalloc(bufSize, GFP_ATOMIC);
	if(tmpBuffer == NULL)
		return -1;	

	for(tableIndex =0; tableIndex < BOB_TABLE_NUM; tableIndex++)
	{
		bufOffset = tableIndex * BOB_TABLE_SIZE;
		if(tableIndex < 2)
		{
			offset = 0;
			for(i=0; i<4; i++)
			{
				phy_I2C_read(0, I2C_U2_CLK_DIV, GPON_PHY_I2C_ADDR1,1,offset+bufOffset,tmpBuffer+bufOffset+offset,readBaseLen);
				offset = offset+readBaseLen;
			}
			
		}
		else if(tableIndex == 2)
		{
			offset = 0;
			for(i=0; i<4; i++)
			{
				phy_I2C_read(0,0xc7,GPON_PHY_I2C_ADDR2,1,offset,tmpBuffer+bufOffset+offset,readBaseLen);
				offset = offset+readBaseLen;
			}
		}
		else
		{
			UINT8 writeBuf=0;
			offset = 0;
			writeBuf=tableIndex - 3;
			phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1,127,&writeBuf,1);
			
			for(i=0; i<4; i++)
			{
				phy_I2C_read(0,0xc7,GPON_PHY_I2C_ADDR2,1,offset+BOB_TABLE_SIZE,tmpBuffer+bufOffset+offset,readBaseLen);
				offset = offset+readBaseLen;
			}
		}
	}
	
	src = "/tmp/gpon_bob.conf";
	orgfs = get_fs();
	set_fs(KERNEL_DS);
	if (src && *src)
	{
		srcf = filp_open(src, O_RDWR|O_CREAT, 0);
		if (IS_ERR(srcf))
		{
			printk("--> Error opening \n");
			goto error;
		}
		else
		{
			srcf->f_pos = 0;
			if(ecnt_kernel_fs_write(srcf, tmpBuffer, bufSize, &srcf->f_pos) >0)
			{
			
			}
			else
			{
				printk("--> Error write \n");
				filp_close(srcf,NULL);						
				goto error;
			}
				
			filp_close(srcf,NULL);
		}
	}
	set_fs(orgfs);
	if(tmpBuffer != NULL)
		kfree(tmpBuffer);
	
	return 0;

error:
	set_fs(orgfs);
	if(tmpBuffer != NULL)
		kfree(tmpBuffer);
	return -1;		
}

/*****************************************************************************
******************************************************************************/
static int readFromBobFile(UINT16 bufSize, UINT8 *mbuf, UINT16 *retlen)
{
	struct file 			*srcf = NULL;
	char *src = NULL;
	mm_segment_t			orgfs;

	src = "/tmp/gpon_bob.conf";
	orgfs = get_fs();
	set_fs(KERNEL_DS);

		if (src && *src)
		{
			srcf = filp_open(src, O_RDONLY, 0);
			if (IS_ERR(srcf))
			{
				printk("--> Error opening \n");
				goto error;
			}
			else
			{
				ecnt_kernel_fs_read(srcf, mbuf, bufSize, &srcf->f_pos);
				filp_close(srcf,NULL);
			}
		}
		set_fs(orgfs);
		return 0;

error:
		set_fs(orgfs);
		return -1;	
}


static int gpon_init_i2c_data(void) 
{
	int ret = 0, i = 0;
	unchar temp1[4] = {0x00, 0x00, 0x00, 0x00} ;
	unchar temp2[4] = {0xFF, 0xFF, 0xFF, 0xFF} ;
	UINT8 *mbuf= NULL;
	__u32 relen = 0;
	__u32 base_length = 16;//write 16 bytes one time
	UINT8 table=0;
	UINT16 bufSize = BOB_TABLE_SIZE * BOB_TABLE_NUM;
	UINT8 *table3Buf=NULL;
	
	mbuf = kmalloc(bufSize, GFP_KERNEL);
	if(mbuf == NULL)
	{
		printk("kmalloc fail!\n");
		ret = -1;
		goto error;
	}
	memset(mbuf,0, bufSize);

	//copy 0x51 table 3 for write latest.
	table3Buf = kmalloc(BOB_TABLE_SIZE, GFP_KERNEL);
	if(table3Buf == NULL)
	{
		printk("kmalloc fail!\n");
		ret = -1;
		goto error;
	}
	memset(table3Buf,0, BOB_TABLE_SIZE);	

	readFromBobFile(bufSize, mbuf, &relen);
	memcpy(table3Buf, mbuf+0x300, BOB_TABLE_SIZE);
	if((memcmp(mbuf+0x300,temp1,4)!=0) && (memcmp(mbuf+0x300,temp2,4)!=0))
	{
		/*read from flash, and write into i2c*/
//		printk("read bosa value from flash, and write into i2c!\n");			
		for(i=0; i<bufSize; )
		{
			if(i < 256)
			{
				//0x50 table 0 and table 1
				phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR1,1, i, mbuf+i,base_length);
				i += base_length;
			}
			else if(i < 384)
			{
				//0x51 table 0
				if(i+base_length >=384)
				{
					phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, i-256, mbuf+i,base_length-1);
				}
				else
				{
					phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, i-256, mbuf+i,base_length);			
				}				
				i += base_length;			
			}
			else
			{
				//0x51 table 1~7
				if(i%BOB_TABLE_SIZE == 0)
				{
					table = i/BOB_TABLE_SIZE - 3;
					if(table == 3)
					{
						i+=BOB_TABLE_SIZE;
						continue;
					}
					phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, 127, &table,1);
				}
				phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, (i%BOB_TABLE_SIZE)+BOB_TABLE_SIZE, (mbuf+i), base_length);				
				i+=base_length;
			}	
		}

		table = 3;	
		phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, 127, &table,1);
		
		for(i=0; i<128;)
		{
			phy_I2C_write(0,0xc7,GPON_PHY_I2C_ADDR2,1, i+BOB_TABLE_SIZE, (table3Buf+i), base_length);
			i+=base_length;
		}	
		
	}
	else
	{
		/*read from I2C, and write into flash*/
		printk("Bob data have not been stored in flash!\n");			
	}
	
error:
	if(mbuf != NULL)
		kfree(mbuf);
	if(table3Buf != NULL)
		kfree(table3Buf);	
	
	return ret;
}

/* Calls the appropriate function based on user command */
static int exec_command(const char *buf, unsigned long count, int fs_type)
{
    int i=0, j=0, argc = 0, val = 0;
	UINT8 *ptr=NULL;
	UINT8 *ptr_read=NULL;
    char cmd;
    UINT8 i2c_addr, set = 0;
	UINT16 offset=0, length=0;
    char arg[MAX_ARGS][MAX_ARG_SIZE];
	#define LOG_WR_KBUF_SIZE 1024 
    char kbuf[LOG_WR_KBUF_SIZE];
	int retvalue=0;

    if(fs_type == PROC_FS)
    {
        if ((count > LOG_WR_KBUF_SIZE-1) || 
            (copy_from_user(kbuf, buf, count) != 0))
            goto errorHandle;
        kbuf[count]=0;
        argc = sscanf(kbuf, "%c %s %s %s %s", &cmd, arg[0], arg[1], 
                      arg[2], arg[3]);
    }

	if(argc == 1 && ((cmd == 'b') || (cmd == 'c')))
	{
		;
	}
	else
	{
	    if (argc <= 3) {
	        printk("Need at-least 4 arguments! \n");
	        goto errorHandle;
	    }

	    for (i=0; i<MAX_ARGS; ++i) {
	        arg[i][MAX_ARG_SIZE-1] = '\0';
	    }
	}

    switch (cmd) {
 
       case 'a':
        if (argc >= 4) {
            i2c_addr = (UINT8) simple_strtoul(arg[0], NULL, 0);// ic address
            offset = (UINT16) simple_strtoul(arg[1], NULL, 0);// reg offset
            length = (UINT16) simple_strtoul(arg[2], NULL, 0);// length

            if (i2c_addr > 127 ) {
                printk("Invalid I2C addr!\n");
                goto errorHandle;
            }
			
			/* Verify that max_offset is below the max_eeprom_size (256 Bytes)*/
            if (offset + length > GPON_PHY_EEPROM_SIZE ) {
                printk("offset + count must be less than Max EEPROM Size of 256!\n");
                goto errorHandle;
            }			


            if (argc > 4) {
				
				ptr = (UINT8 *) kmalloc(length, GFP_KERNEL);
				if (ptr == NULL){
					printk("doSirmWriteOp Memory Allocation Fail!\n");
					goto errorHandle;
				}
				j=0;				
				i=0;
			
				char *p = kbuf;
				while(((p=strstr(p, " ")) != NULL) && j < LOG_WR_KBUF_SIZE){
					p++;
					i++;
					j++;
					if(i > 4)
							sscanf(p, "%s", arg[i-1]);	

				}				

				for(i=0; i<length; i++)
				{
                	ptr[i] = (UINT8) simple_strtoul(arg[3+i], NULL, 0); // value
				}
                set = 1;
					
            }
			
			if(set == 1)
			{
				// write
				/*limit max 16 bytes to write in one time */
				if (length > 16 ) {
					printk("should not write more than 16 bytes in one time!\n");
					goto errorHandle;
				}	

			//	printk("write: i2c_addr = 0x%x, offset = %d, len = %d\n", i2c_addr, offset, length);
				retvalue = phy_I2C_write(0,0xc7,i2c_addr,1,offset,ptr,length);
				if(retvalue < 0)
				{
					printk(KERN_ALERT "write operation error!errorcode is %d.\n",retvalue);
					goto errorHandle;
				}

			}
			else
			{
				//read
				/*limit max 40 bytes to read in one time */
				if (length > 40 ) {
					printk("should not read more than 40 bytes in one time!\n");
					goto errorHandle;
				}					
			//	printk("read: i2c_addr = 0x%x, offset = %d, len = %d \n", i2c_addr, offset, length);
				
				ptr_read = (UINT8 *) kmalloc(length, GFP_KERNEL);
				if (ptr_read == NULL){
					printk("doSirmReadOp Memory Allocation Fail!\n");
					goto errorHandle;
				}	
				retvalue = phy_I2C_read(0,0xc7,i2c_addr,1,offset,ptr_read,length);
				
				if(retvalue < 0)
				{
					printk("read verify error!errorcode is %d.\n",retvalue);
					goto errorHandle;
				}
	
				printk("Read Vaule:\n");
				printk("%08x: ",offset);
				for(i = 0; i< length; i++)
				{	
					if((i != 0) && ((i % 8) == 0))
					{
						offset = offset + 8;
						printk("\n%08x: ",offset);
					}				
					printk("0x%02x ",ptr_read[i]);
				}	
				printk("\n");		

			}
        } else {
            printk("Need at-least 4 arguments!\n");
            goto errorHandle;
        }
        break;

	case 'b':
		printk("writeToBobFileViaI2C.\n"); 
		writeToBobFileViaI2C();
		break;

	case 'c':
		printk("read from bob file, and write into i2c.\n"); 
		gpon_init_i2c_data();
		break;

    default:
        printk("Invalid command.\n"); 
		printk("Valid commands:\n"); 
        printk("  Generic I2C access: a <i2c_addr(7-bit)>");  
        printk(" <offset> <length(1-256)> [value] \n"); 
        break;
    }

	if(ptr)
		kfree(ptr);
	if(ptr_read)
		kfree(ptr_read);
	
	return count;
	
errorHandle:
	printk("correct cmd:a <i2c_addr(7-bit)> <offset(0-255)> <length(1-256)> [value] \n");
	if(ptr)
		kfree(ptr);
	if(ptr_read)
		kfree(ptr_read);
	return -1;	
}


#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t gpon_phy_test_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	printk(" Usage: echo command >/proc/i2c-gpon/gponPhyTest \n");
	printk(" supported commands: \n"); 
	printk("  Generic I2C access: a <i2c_addr(7-bit)>");
	printk(" <offset> <length(1-4)> [value] \n");
	printk("\n");
	return 0;
}

static ssize_t gpon_phy_test_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	return exec_command(buf, count, PROC_FS);
}
#else
static int gpon_phy_test_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	printk(" Usage: echo command >/proc/i2c-gpon/gponPhyTest \n");
	printk(" supported commands: \n"); 
	printk("  Generic I2C access: a <i2c_addr(7-bit)>");
	printk(" <offset> <length(1-4)> [value] \n");
	printk("\n");
	return 0;
}

static int gpon_phy_test_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	return exec_command(buffer, count, PROC_FS);
}
#endif
#endif/*TCSUPPORT_COMPILE*/

void schedule_fe_reset(void)
{
    return;
}

extern atomic_t to1_timeout_cnt;
extern atomic_t to1_rst_cnt;
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t gpon_stage_change_chk_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int index=0 ;
	loff_t pos=0, begin=0 ;
	loff_t off = *ppos;
	index += sprintf(buf+ index, "to1_timeout_cnt:%d rst:%d\n", atomic_read(&to1_timeout_cnt),  atomic_read(&to1_rst_cnt)) ;
	CHK_BUF() ;

done:
	index -= (off - begin) ;
	if(index<0)		index = 0 ;
	if(index>count)		index = count ;
	return index ;
}

static ssize_t gpon_stage_change_chk_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[64]={0};
	int timerout_cnt = 0, rst_cnt = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buf, count))
		return -EFAULT ;
	
	sscanf(val_string, "%d %d", &timerout_cnt, &rst_cnt);
	atomic_set(&to1_timeout_cnt, timerout_cnt);
	atomic_set(&to1_rst_cnt, rst_cnt);
	
	return count ;
}
#else
static int gpon_stage_change_chk_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "to1_timeout_cnt:%d rst:%d\n", atomic_read(&to1_timeout_cnt),  atomic_read(&to1_rst_cnt)) ;
	CHK_BUF() ;
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0)		index = 0 ;
	if(index>count)		index = count ;
	return index ;
}

static int gpon_stage_change_chk_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64]={0};
	int timerout_cnt = 0, rst_cnt = 0;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;
	
	sscanf(val_string, "%d %d", &timerout_cnt, &rst_cnt);
	atomic_set(&to1_timeout_cnt, timerout_cnt);
	atomic_set(&to1_rst_cnt, rst_cnt);
	
	return count ;
}
#endif
void xpon_dump_raw_data(char *buf, int *len)
{
	*len += sprintf((*len+buf), "\n");
	*len += sprintf((*len+buf), "**************************************\n");
	*len += sprintf((*len+buf), "**             Raw data             **\n");
	*len += sprintf((*len+buf), "**************************************\n");
	*len += sprintf((*len+buf), "\n");

	*len += sprintf((*len+buf), " SCU_WAN_CONF_REG         (0xBFB00070): %#03x\n", xpon_regs_p->wan_mode_val);
	if(pon_mode & PON_MODE_GPON) {
		*len += sprintf((*len+buf), " G_ACTIVATION     (0xBFB640BC): %#03x\n", xpon_regs_p->gpon_ploam_stat_val);
		*len += sprintf((*len+buf), " G_EQD            (0xBFB640A8): %#03x\n", xpon_regs_p->gpon_eqd_val);
		*len += sprintf((*len+buf), " G_RSP_TIME       (0xBFB640BC): %#03x\n", xpon_regs_p->gpon_resp_time_val);
		*len += sprintf((*len+buf), " DBG_ERR_CTRL                (0xBFB64260): %#03x\n", IO_GREG(DBG_ERR_CTRL           ));
		*len += sprintf((*len+buf), " DBG_SW_RDI_CTRL             (0xBFB64264): %#03x\n", IO_GREG(DBG_SW_RDI_CTRL        ));
		*len += sprintf((*len+buf), " DBG_RX_GEM_CNT              (0xBFB64300): %#03x\n", IO_GREG(DBG_RX_GEM_CNT         ));
		*len += sprintf((*len+buf), " DBG_RX_CRC_ERR_CNT          (0xBFB64304): %#03x\n", IO_GREG(DBG_RX_CRC_ERR_CNT     ));
		*len += sprintf((*len+buf), " DBG_RX_GTC_CNT              (0xBFB64308): %#03x\n", IO_GREG(DBG_RX_GTC_CNT         ));
		*len += sprintf((*len+buf), " DBG_TX_GEM_CNT              (0xBFB6430C): %#03x\n", IO_GREG(DBG_TX_GEM_CNT         ));
		*len += sprintf((*len+buf), " DBG_TX_BST_CNT              (0xBFB64310): %#03x\n", IO_GREG(DBG_TX_BST_CNT         ));
		*len += sprintf((*len+buf), " DBG_GEM_HEC_ONE_ERR_CNT     (0xBFB64330): %#03x\n", IO_GREG(DBG_GEM_HEC_ONE_ERR_CNT));
		*len += sprintf((*len+buf), " DBG_GEM_HEC_TWO_ERR_CNT     (0xBFB64334): %#03x\n", IO_GREG(DBG_GEM_HEC_TWO_ERR_CNT));
		*len += sprintf((*len+buf), " DBG_GEM_HEC_UC_ERR_CNT      (0xBFB64338): %#03x\n", IO_GREG(DBG_GEM_HEC_UC_ERR_CNT ));
		*len += sprintf((*len+buf), " DBG_DS_SPF_CNT              (0xBFB64358): %#03x\n", IO_GREG(DBG_DS_SPF_CNT         ));
		*len += sprintf((*len+buf), " DBG_DS_GTC_EXTR_ETH_CNT     (0xBFB64300): %#03x\n", IO_GREG(DBG_DS_GTC_EXTR_ETH_CNT));
		*len += sprintf((*len+buf), " DBG_US_GTC_EXTR_ETH_CNT     (0xBFB64264): %#03x\n", IO_GREG(DBG_US_GTC_EXTR_ETH_CNT));
		*len += sprintf((*len+buf), " DBG_GPON_MAC_SET            (0xBFB64300): %#03x\n", IO_GREG(DBG_GPON_MAC_SET       ));
	
		*len += sprintf((*len+buf), " DBG_GRP_0                   (0xBFB64228): %#03x\n", IO_GREG(DBG_GRP_0              ));
		*len += sprintf((*len+buf), " DBG_GRP_1                   (0xBFB6422C): %#03x\n", IO_GREG(DBG_GRP_1              ));
	}
	if(pon_mode & PON_MODE_EPON) {
		*len += sprintf((*len+buf), " E_MPCP_STAT      (0xBFB66120): %#03x\n", xpon_regs_p->epon_rx_mpcp_cnt);
		*len += sprintf((*len+buf), " E_MPCP_RGST_STAT (0xBFB66124): %#03x\n", xpon_regs_p->epon_tx_mpcp_rgst_cnt);
		*len += sprintf((*len+buf), " E_SYNC_TIME      (0xBFB660D4): %#03x\n", xpon_regs_p->epon_sync_time_val);
	}

	return;
}

void get_epon_stat(char *buf, int *len, epon_stat *epon_stat_cur)
{
	epon_stat_cur->jiffies_val = jiffies;
		
	xpon_regs_p->epon_rx_mpcp_cnt = IO_GREG(e_mpcp_stat);
	epon_stat_cur->dscvr_gate_cnt = (xpon_regs_p->epon_rx_mpcp_cnt)&0xffff;
	epon_stat_cur->rx_rgst_cnt = (((xpon_regs_p->epon_rx_mpcp_cnt)&0x00ff0000)>>16);
				
	xpon_regs_p->epon_tx_mpcp_rgst_cnt = IO_GREG(e_mpcp_rgst_stat);
	epon_stat_cur->tx_rgst_cnt = (((xpon_regs_p->epon_tx_mpcp_rgst_cnt)&0x0000ff00)>>8);
	epon_stat_cur->tx_rgst_ack_cnt = ((xpon_regs_p->epon_tx_mpcp_rgst_cnt)&0xff);
				
	xpon_regs_p->epon_sync_time_val = IO_GREG(e_sync_time);

	if(pon_mode & PHY_RD) {
		if(epon_proc_cnt && !(epon_stat_cur->rx_rgst_cnt-epon_stat_prev->rx_rgst_cnt) && !(epon_stat_cur->tx_rgst_cnt-epon_stat_prev->tx_rgst_cnt) && !(epon_stat_cur->tx_rgst_ack_cnt-epon_stat_prev->tx_rgst_ack_cnt))
			*len += sprintf((*len+buf), " MPCP Status: Registered");
		else if(epon_proc_cnt)
			*len += sprintf((*len+buf), " MPCP Status: Registering");
		else
			*len += sprintf((*len+buf), " MPCP Status: Unknown");
	
		*len += sprintf((*len+buf), "	Jiffies: %#lx(+%#lx +%ld)\n", epon_stat_cur->jiffies_val, (epon_proc_cnt ? (epon_stat_cur->jiffies_val-epon_stat_prev->jiffies_val) : epon_proc_cnt), (epon_proc_cnt ? (epon_stat_cur->jiffies_val-epon_stat_prev->jiffies_val) : epon_proc_cnt));
		*len += sprintf((*len+buf), " Discover Gate Count:	 %#x(+%#x +%d)\n", epon_stat_cur->dscvr_gate_cnt, (epon_proc_cnt ? (epon_stat_cur->dscvr_gate_cnt-epon_stat_prev->dscvr_gate_cnt) : epon_proc_cnt), (epon_proc_cnt ? (epon_stat_cur->dscvr_gate_cnt-epon_stat_prev->dscvr_gate_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " RX Register Count: 	 %#x(+%d)\n", epon_stat_cur->rx_rgst_cnt, (epon_proc_cnt ? (epon_stat_cur->rx_rgst_cnt-epon_stat_prev->rx_rgst_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " TX Register Count: 	 %#x(+%d)\n", epon_stat_cur->tx_rgst_cnt, (epon_proc_cnt ? (epon_stat_cur->tx_rgst_cnt-epon_stat_prev->tx_rgst_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " TX Register ACK Count:  %#x(+%d)\n", epon_stat_cur->tx_rgst_ack_cnt, (epon_proc_cnt ? (epon_stat_cur->tx_rgst_ack_cnt-epon_stat_prev->tx_rgst_ack_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " Sync Time: %#x\n", (xpon_regs_p->epon_sync_time_val)&0xffff);

		epon_proc_cnt++;
	}
	
	epon_stat_prev->jiffies_val = epon_stat_cur->jiffies_val;
	epon_stat_prev->dscvr_gate_cnt = epon_stat_cur->dscvr_gate_cnt;
	epon_stat_prev->rx_rgst_cnt = epon_stat_cur->rx_rgst_cnt;
	epon_stat_prev->tx_rgst_cnt = epon_stat_cur->tx_rgst_cnt;
	epon_stat_prev->tx_rgst_ack_cnt = epon_stat_cur->tx_rgst_ack_cnt;
	
	return;
}

extern epon_t eponDrv;
char mpcpStateStr[MAX_MPCP_STATE_NUM][MAX_MPCP_STATE_STR_LEN] = 
{
	"WAIT",
	"REGISTERING",
	"REGISTER_REQUEST",
	"REGISTER_PENDING",
	"RETRY",
	"DENIED",
	"REGISTER_ACK",
	"NACK",
	"REGISTERED",
	"REMOTE_DEREGISTER",
	"LOCAL_DEREGISTER"
};

static inline int should_ignore_phy_los(void)
{
    return ((PHY_LINK_STATUS_READY != gpPhyData->phy_link_status ) || (PHY_UNKNOWN_CONFIG == gpPhyData->working_mode) );
}

static void xpon_phy_los_illegal_handler(PHY_Event_Source_t src, PHY_Event_Type_t id)
{

    XPON_DPRINT_MSG("id:%d\n", id);
    
    if(should_ignore_phy_los() ) {
        XPON_DPRINT_MSG("[%s] ignore phy los!\n", __FUNCTION__);
        return;
    }


    switch(gpPhyData->working_mode)
    {
        case PHY_GPON_CONFIG:
		case PHY_GPON_SYM_CONFIG:
            gpon_detect_los_lof(id);
            break;

        case PHY_EPON_CONFIG:
            eponDetectPhyLosLof();
            break;

        default:
            dump_stack();
            printk("Illegal value of gpPhyData->working_mode: %d! %s:%d\n", 
                gpPhyData->working_mode, __FUNCTION__, __LINE__);
            break;
    }
}

static inline int should_ignore_phy_ready(void)
{
    return (PHY_LINK_STATUS_READY == gpPhyData->phy_link_status);
}

/*prepare the cmdline args, make it similar to 'main' function's arg*/
/*ret val: 1 success, 0 fail*/
int xpon_prepare_params(char * cmdline, size_t * argc, char ** argv)
{
    size_t cnt = 0, pos = 0;
    enum{SEARCH_ARG, HIT_ARG} state = SEARCH_ARG;
    
    for(pos = 0; cmdline[pos] != 0; ++pos)
    {
        switch(state)
        {
            case SEARCH_ARG:
                if('\x20' != cmdline[pos] && '\n' != cmdline[pos])
                {
                    state = HIT_ARG;
                    argv[cnt] = & cmdline[pos];
                }
                break;

            case HIT_ARG:
                if('\x20' == cmdline[pos] || '\n' == cmdline[pos])
                {
                    cmdline[pos] = 0;
                    state = SEARCH_ARG;
                    ++ cnt;
                    if(cnt > MAX_CMD_LINE_ARGC)
                    {
                        return 1;
                    }
                }
                break;
        }
    }

    *argc = cnt;
    return 1;
}

static void xpon_phy_ready_handler(PHY_Event_Source_t src)
{
    if(should_ignore_phy_ready() ){
        XPON_DPRINT_MSG("[%s] ignore phy ready!\n", __FUNCTION__);
        return;
    }
      
    if(PHY_UNKNOWN_CONFIG == gpPhyData->working_mode ) {
        gpPhyData->working_mode  = XPON_PHY_GET(PON_GET_PHY_MODE) ;	
		if((gpPhyData->working_mode ==PHY_GPON_CONFIG)||(gpPhyData->working_mode ==PHY_EPON_CONFIG)||(gpPhyData->working_mode ==PHY_GPON_SYM_CONFIG))				
			gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_SYNCED;			
    }

    #if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
		XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_SD) ;
    #endif/*TCSUPPORT_COMPILE*/
    
    switch(gpPhyData->working_mode)
    {
        case PHY_GPON_CONFIG:
		case PHY_GPON_SYM_CONFIG:
            gpon_detect_phy_ready();
            break;

        case PHY_EPON_CONFIG:
            gpPhyData->phy_link_status = PHY_LINK_STATUS_READY ;
			enable_cpu_us_traffic();
            eponDetectPhyReady();
            break;
            
        default:
            printk("Illegal value of gpPhyData->working_mode: %d! %s:%d\n", 
                gpPhyData->working_mode, __FUNCTION__, __LINE__);
            break;
    }
}

void xpon_phy_start_rouge_handle(void)
{
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_TRUE ;
}

void xpon_phy_stop_rouge_handle(void)
{
    if( PHY_GPON_CONFIG == gpPhyData->working_mode   && \
        GPON_STATE_O7   == GPON_CURR_STATE && stop_rouge_tx_enable) {
        	gpGponPriv->emergencystate = 0;
			XPON_DPRINT_MSG("call gpon disable.\n");
            gpon_disable();
			XPON_PHY_TX_ENABLE();
	        xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_ENABLE, 0) ;
            change_alarm_led_status(ALARM_LED_OFF) ;
        } 
        
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_FALSE ;
}

static void xpon_tx_power_handle(unchar opt)
{
	__u8 llidIndex = 0;
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	uint tx_power_last = eponDrv.eponTxPowerFlag;
#endif/*TCSUPPORT_COMPILE*/
    if((PHY_EPON_CONFIG == gpPhyData->working_mode)
     &&(opt == XPON_PHY_TX_POWER_OFF))
    {
		for(llidIndex = 0; llidIndex < EPON_1G_MAX_LLID_NUM; llidIndex++){
			eponMpcpLocalDergstr(llidIndex);
		}	    
    }	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if(PHY_EPON_CONFIG == gpPhyData->working_mode)
	{
		eponDrv.eponTxPowerFlag = opt;
		
		if(opt == XPON_PHY_TX_POWER_OFF)
		{
			xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_TX_POWER_OFF, 0);
		}
		else if(opt == XPON_PHY_TX_POWER_ON)
		{
			if (XPON_PHY_GET(PON_GET_PHY_LOS_STATUS) == PHY_LOS_HAPPEN)
			{
			#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
				XPON_PHY_NOTIFY_EVENT(PHY_XPON_STATE_NOTIFY_LOS) ;
        	#endif/*TCSUPPORT_COMPILE*/
			
				xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_LOS, 0) ;
			}
			
			if(opt != tx_power_last){
				eponMacReinit(FE_CHANNEL_RETIRE_ENABLE);
			}
		}
		else{
			printk("Error operation %u\n",opt);
		}
	}
	else if((PHY_GPON_CONFIG == gpPhyData->working_mode) || (gpPhyData->working_mode ==PHY_GPON_SYM_CONFIG)){
		PON_MSG(MSG_DBG,"phy set tx power %u ,GPON do nothing\n",opt);
	}
#endif/*TCSUPPORT_COMPILE*/
}


static int gponMacRxDis(unsigned char dis)
{
    int ret = ECNT_RETURN;
    REG_G_MBI_MPI_STOP gponMbiStop ;
    
    if((dis != ECNT_XPON_MAC_RX_DISABLE) && (dis != ECNT_XPON_MAC_RX_ENABLE)) {
       return ECNT_HOOK_ERROR;
    }
       
    /* STOP/START MBI Interface */
    gponMbiStop.Raw = IO_GREG(G_MBI_MPI_STOP) ;
    /* 0: not stop 
    *  1: stop
    */
    gponMbiStop.Bits.mbi_rx_stop = (dis == ECNT_XPON_MAC_RX_ENABLE) ? 0 : 1 ;
    IO_SREG(G_MBI_MPI_STOP, gponMbiStop.Raw) ;
    PON_MSG(MSG_DBG,"%s %d MAC rx dis = %d \n", __FUNCTION__,__LINE__,dis);
    
    return ret;
}


static int eponMacRxDis(unsigned char dis){
    uint Raw = READ_REG_WORD(e_glb_cfg);

    if (ECNT_XPON_MAC_RX_DISABLE == dis)
        Raw |= (RXMBI_STOP);
    else if (ECNT_XPON_MAC_RX_ENABLE== dis)
        Raw &= ~(RXMBI_STOP);
    else 
        return ECNT_HOOK_ERROR;
    
    WRITE_REG_WORD(e_glb_cfg,Raw);

    PON_MSG(MSG_DBG,"%s %d MAC rx dis = %d \n", __FUNCTION__,__LINE__,dis);
    return ECNT_RETURN;
}

static int compute_cnt(int flag)
{
	static uint cnt = 0; /*used for auto mode*/ 
	if(PHY_EVENT_PHYRDY_INT == flag)
	{
		cnt = 0;
	}
	else
	{
		cnt += 1;
		if((cnt % phyNoLosNoReadyHandleCycle) == 0)
		{
			cnt = 0;
			return 1;
		}
	}
	return 0;
}

static void xpon_no_los_no_ready_handle(void)
{
	if((gpPhyData->is_fix_mode == FALSE) && (1 == compute_cnt(PHY_EVENT_NO_LOS_NO_READY)))
	{ 
		XPON_DPRINT_MSG("Phy event:no los no ready handle!\n");
		if(gpPonSysData->sysPrevLink == PON_LINK_STATUS_GPON)
		{
			xmcs_report_event(XMCS_EVENT_TYPE_GPON, XMCS_EVENT_GPON_MODE_CHANGE, 0);
		}
		else if(gpPonSysData->sysPrevLink == PON_LINK_STATUS_EPON)
		{
			eponSetMpcpDiscStateToDefault();
			xmcs_report_event(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_MODE_CHANGE, 0);
		}
		gpPhyData->working_mode = PHY_UNKNOWN_CONFIG;
		auto_mode();
	}
}

static void xpon_phy_rongue_onu_handle(void)
{

	XPON_PHY_TX_DISABLE();
	gpPonSysData->ponRogueStatus = PON_STATUS_ROGUE;
	printk("Rogue ONU was detected!\n");

	return;
}

static int xpon_phy_event_dispatch(PON_PHY_Event_data_t * pEvent)
{
    unsigned long flag;
    
    spin_lock_irqsave(&gpPonSysData->event_lock, flag) ;
    if(PON_WAN_START != gpPonSysData->sysStartup){
        if(TRUE == gpPhyData->calibrating  && PHY_EVENT_CALIBRATION_STOP == pEvent->id){
            //gpPhyData->calibrating = FALSE;
            xmcs_set_connection_start(XPON_ENABLE);
        }else{
            XPON_DPRINT_MSG("PON WAN STOP! Ignore phy event:%d!\n", pEvent->id);
        }
        goto end;
    }else{
        XPON_DPRINT_MSG("Phy event:%d!\n", pEvent->id);
    }
    
    switch (pEvent->id)
    {
        case PHY_EVENT_TRANS_LOS_INT      :
        case PHY_EVENT_PHY_ILLG_INT       :
        case PHY_EVENT_TRANS_LOS_ILLG_INT :
			gpon_set_alarmBit(LOS_INDEX);
        		xpon_phy_los_illegal_handler(pEvent->src, pEvent->id);
			break;
		case PHY_EVENT_PHY_LOF_INT:	
            xpon_phy_los_illegal_handler(pEvent->src, pEvent->id);
			gpon_set_alarmBit(LOF_INDEX);
            break;

        case PHY_EVENT_PHYRDY_INT:
			gpon_clear_alarmBit(LOS_INDEX);
			gpon_clear_alarmBit(LOF_INDEX);
            if(gpPhyData->is_fix_mode == FALSE)
            {
                compute_cnt(PHY_EVENT_PHYRDY_INT);
            }
            xpon_phy_ready_handler(pEvent->src);
            break;
            
        case PHY_EVENT_TF_INT:
			if(onu_rogue_test)
			{
				printk("[%s][%d] pon driver start to do rongue onu handle!\n",__FUNCTION__,__LINE__);
				xpon_phy_rongue_onu_handle();
			}
            break;
            
        case PHY_EVENT_TRANS_INT:
            break;
            
        case PHY_EVENT_TRANS_SD_FAIL_INT:
            break;
            
        case PHY_EVENT_I2CM_INT:
            break;

        case PHY_EVENT_START_ROGUE_MODE:
            xpon_phy_start_rouge_handle();
			gpon_set_alarmBit(ROGUE_INDEX);
            break;

        case PHY_EVENT_STOP_ROGUE_MODE:
            xpon_phy_stop_rouge_handle();
            break;

        case PHY_EVENT_CALIBRATION_START:
            gpPhyData->calibrating = TRUE;
            xmcs_set_connection_start(XPON_DISABLE);
            break;
            
        case PHY_EVENT_TX_POWER_ON:
        	xpon_tx_power_handle(XPON_PHY_TX_POWER_ON);
        	break;
        case PHY_EVENT_TX_POWER_OFF:
        	xpon_tx_power_handle(XPON_PHY_TX_POWER_OFF);
        	break;
        case PHY_EVENT_NO_LOS_NO_READY:
            xpon_no_los_no_ready_handle();
            break;
        default:
            printk("Event %d handler not implemented! %s:%d\n", pEvent->id, __FUNCTION__, __LINE__);
            break;
    }
    
end:
    spin_unlock_irqrestore(&gpPonSysData->event_lock, flag) ;
    return 0;
}

static int get_tcont_from_gemportid(uint gemport_id)
{
	uint gemIdx ;
	int channelId = -1;

    gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemport_id] & GPON_GEM_IDX_MASK);
    
	if(gemIdx>=GPON_GEMPORT_MAX_NUM || !gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
		return -1 ;
	}

	channelId = gpWanPriv->gpon.gemPort[gemIdx].info.channel;

	if(channelId == GPON_MULTICAST_CHANNEL)
	    return -1;

	return channelId;
	
}
#if defined (TCSUPPORT_AUTOBENCH) && (defined (TCSUPPORT_CPU_EN7528)||defined (TCSUPPORT_CPU_EN7527)||defined (TCSUPPORT_CPU_EN7581)||defined (TCSUPPORT_CPU_AN7552))
static int file_write(char *filename,char *buffer,int bufSize){
	struct file *srcf = NULL;
	char    *src = NULL;
	mm_segment_t    orgfs = {0};

	if(buffer == NULL)
	       return 1;
   	src = filename;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	if (src && *src)
	{
		srcf = filp_open(src, O_RDWR, 0);//|O_CREAT
		if (IS_ERR(srcf))
		{
			printk("--> gpon test: error opening %s\n",filename);
			return 1;
		}
		else
		{
			ecnt_kernel_fs_write(srcf, buffer, bufSize, &srcf->f_pos);
			filp_close(srcf,NULL);
		}
	}
	set_fs(orgfs);
	return 0;
}

int do_xpon_slt_action(int mode, int epon_online_enable)
{
	int delay_cnt = 0;
	
	if(mode == XMCS_IF_WAN_DETECT_MODE_GPON)
	{
		printk("GPON SLT online testing\n");
		xmcs_set_link_detection(XMCS_IF_WAN_DETECT_MODE_GPON);
		xmcs_set_connection_start(XPON_ENABLE);
		for(delay_cnt = 0 ; delay_cnt < 600; delay_cnt++){
			if( gpon_slt_test != 0)
				break;
			msleep(50);
			printk(".");
		}
		printk("\n");
		printk("delay = %d ms\n",delay_cnt*50);
		xmcs_set_connection_start(XPON_DISABLE);
		if(gpon_slt_test == 0){
			printk("GPON SLT TEST FAIL!!\n");
			return ECNT_RETURN;
		}
		else{
			printk("GPON SLT TEST SUCCESS!!\n");
			return ECNT_CONTINUE;
		}
	}
	else  /*default is EPON SLT test*/
	{
		
		xmcs_set_link_detection(XMCS_IF_WAN_DETECT_MODE_EPON);
		xmcs_set_connection_start(XPON_ENABLE);
		if(epon_online_enable)
		{
			printk("EPON SLT online testing\n");
			for(delay_cnt = 0 ; delay_cnt < 600; delay_cnt++){
				if( epon_slt_test != 0)
					break;
				msleep(50);
				printk(".");
			}
			printk("\n");
			printk("delay = %d ms\n",delay_cnt*50);
			if(epon_slt_test == 0){
				printk("EPON SLT ONLINE TEST FAIL!!\n");
				return ECNT_RETURN;
			}
			else{
				printk("EPON SLT ONLINE TEST SUCCESS!!\n");
				//return ECNT_CONTINUE;
			}
		}
		else
		{
			mdelay(100);
		}
		printk("EPON SLT loopback testing\n");
        xmcs_set_connection_start(XPON_DISABLE);
        /*Disable flow control*/
//        WRITE_FE_REG(FLOW_CONTROL, 0);
        file_write("/proc/qdma_wan/self_test","init eponmac",strlen("init eponmac"));
        file_write("/proc/qdma_wan/self_test","tc2_epon 30 100",strlen("tc2_epon 30 100"));
        
        if(pon_loopback_result_wan){
			printk("EPON SLT LOOPBACK TEST SUCCESS!!\n");
            return ECNT_CONTINUE;
        }
        else{
			printk("EPON SLT LOOPBACK TEST FAIL!!\n");
            return ECNT_RETURN;
        }
	}
}

#endif

static int xpon_mac_pub_module_hook_dispatch(xpon_mac_hook_data_t  * data)
{
    struct XMCS_GponOnuInfo_S onuInfo ;
    struct XMCS_GponSnPasswd_S gponSnPasswd ;
    unsigned char rx_dis;
	unsigned char modify_flag;
    
    switch (data->pub_info.type)
    {
        case XPON_SN_SET:
        {
            memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
			xmcs_get_onu_info(&onuInfo);
            memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
            memcpy(gponSnPasswd.sn, data->pub_info.sn, 8) ;
			xmcs_set_sn_passwd(&gponSnPasswd) ;
            break;
        }
		case XPON_DBRU_SLIGHT_MODIFY:
		{
			if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
				modify_flag = data->pub_info.dbru_modify_flag;
				gponDbruSlightModify(modify_flag);
			}
			else{
				//do nothing
			}
			break;
		}
        case XPON_MAC_MODE_GET:
        {
            if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus || PHY_GPON_CONFIG == gpPhyData->working_mode || PHY_GPON_SYM_CONFIG == gpPhyData->working_mode){
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_GPON;
            }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus || PHY_EPON_CONFIG == gpPhyData->working_mode){
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_EPON;
            }else {
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_OFF;
            }
            break;
        }
        case XPON_MAC_RX_DIS_SET:
        {
            rx_dis = data->pub_info.rx_dis;
            if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
                return gponMacRxDis(rx_dis);
            }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus ){
                return eponMacRxDis(rx_dis);
            }else {
                printk("XPON MAC is Link off\r\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_ONU_TYPE_GET:
        {
            if (0 != xmcs_get_onu_type((XMCSIF_OnuType_t *)& data->pub_info.onu_type) )
            {
                printk("Get onu type failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_TCONT_GET:
        {
            *(data->pub_info.gemtotcont.tcont) = get_tcont_from_gemportid(data->pub_info.gemtotcont.gemportid);
            break;
        }
        case XPON_GEMPORT_REMOVE:
        {
            if (0 != xmcs_remove_gem_port(data->pub_info.Remove_gemportID))
            {
                PON_MSG(MSG_ERR, "XPON_GEMPORT_REMOVE failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_QOS_SET:
        {
            if (0 != xpon_set_qos(data->pub_info.ponsetqos.enable,data->pub_info.ponsetqos.queueNum))
            {
                PON_MSG(MSG_ERR, "XPON_QOS_SET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_TCONT_INFO_GET:
        {
            if (0 != xmcs_get_tcont_info((XMCS_TcontInfo_t *)data->pub_info.pTcontInfo))
            {
                PON_MSG(MSG_ERR, "XPON_TCONT_INFO_GET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_GEMPORT_CREATE:
        {
            if (0 != xmcs_create_gem_port((XMCS_GemPortCreate_t *)data->pub_info.pGemCreate))
            {
                PON_MSG(MSG_ERR, "XPON_GEMPORT_CREATE failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
	    case XPON_CHANNEL_QOS_SET:
        {
            if (0 != xmcs_set_channel_scheduler((XMCS_ChannelQoS_t *)data->pub_info.pScheduler))
            {
                PON_MSG(MSG_ERR, "XPON_CHANNEL_QOS_SET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
	    case XPON_WANLINK_CONFIG_GET:
        {
            if (0 != xmcs_get_wan_link_status((WAN_LINKCFG_t *)data->pub_info.pSysLinkStatus))
            {
                PON_MSG(MSG_ERR, "XPON_WANLINK_CONFIG_GET failed!\n");
                return ECNT_HOOK_ERROR;
            }
            break;
        }
        case XPON_MULITCAST_ANI_GET:
        {
            data->pub_info.mulitcast_ani = mulitcast_ani;
            break;
        }
        case XPON_WAN_NET_START_XMIT:
        {
            data->pub_info.wan_xmit.ret = pwan_net_start_xmit(data->pub_info.wan_xmit.skb, data->pub_info.wan_xmit.dev);
            break;
        }
#if defined (TCSUPPORT_AUTOBENCH) && (defined (TCSUPPORT_CPU_EN7528)||defined (TCSUPPORT_CPU_EN7527)||defined (TCSUPPORT_CPU_EN7581) ||defined (TCSUPPORT_CPU_AN7552))
        case XPON_SLT:
        {
			int ret = ECNT_CONTINUE;
            ret = do_xpon_slt_action(XMCS_IF_WAN_DETECT_MODE_GPON,0); //GPON ONLINE TEST
#if !(defined (TCSUPPORT_CPU_EN7581) || defined (TCSUPPORT_CPU_AN7552))
			if(ret == ECNT_RETURN)
				return ret;
			ret = do_xpon_slt_action(XMCS_IF_WAN_DETECT_MODE_EPON,0); //EPON ONLY LOOPBACK TEST
#endif
			return ret;
            //break;
        }
#endif
        default:
            printk("XPON MAC Driver not ready to receive event from module with id:%d pub_type:%d", data->src_module,data->pub_info.type);
            return ECNT_HOOK_ERROR;
    }

    return ECNT_RETURN;
}

static int xpon_mac_hook_dispatch(struct ecnt_data  * in_data)
{
    xpon_mac_hook_data_t * data = (xpon_mac_hook_data_t *)in_data;
    
    switch (data->src_module)
    {
        case XPON_PHY_MODULE:
            xpon_phy_event_dispatch(data->pEvent);
            break;
        case XPON_PUB_MODULE:
            return xpon_mac_pub_module_hook_dispatch(data);
            break;
        case XPON_INT_MODULE:
            if(PHY_GPON_CONFIG == gpPhyData->working_mode || PHY_GPON_SYM_CONFIG == gpPhyData->working_mode){
            	gpon_isr();
            }
            if(PHY_EPON_CONFIG == gpPhyData->working_mode){
            	eponIsr();
            }
            break;
        default:
            panic("XPON MAC Driver not ready to receive event from module with id:%d,", data->src_module);
            return ECNT_HOOK_ERROR;
    }

    return ECNT_RETURN;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t xpon_rogue_mode_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len;
	loff_t off = *ppos;
	len = sprintf(buf, "%d\n", gpPonSysData->rogue_state);

	len -= off;

	if (len > count)
		len = count;

	if (len < 0)
		len = 0;

	return len;
}
#else
static int xpon_rogue_mode_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	len = sprintf(page, "%d\n", gpPonSysData->rogue_state);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static ssize_t xpon_fe_reset_happened_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	loff_t off = *ppos;
	len = sprintf(buf, "%d\n", gpPonSysData->fe_reset_happened);

	len -= off;

	if (len > count)
		len = count;

	if (len < 0)
		len = 0;

	return len;
}


static ssize_t xpon_fe_reset_happened_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[8] = {0};

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buf, count))
		return -EFAULT;

	val_string[count] = '\0';

	gpPonSysData->fe_reset_happened = simple_strtoul(val_string, NULL, 10);
	
	return count;
}
#else
static int xpon_fe_reset_happened_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "%d\n", gpPonSysData->fe_reset_happened);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}


static int xpon_fe_reset_happened_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[8] = {0};

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	gpPonSysData->fe_reset_happened = simple_strtoul(val_string, NULL, 10);
	
	return count;
}
#endif
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
static const struct proc_ops proc_gpon_debug_ops = {
	.proc_read	= gpon_debug_read_proc,
	.proc_write	= gpon_debug_write_proc,
};

static const struct proc_ops proc_bbf247_flag_ops = {
	.proc_read	= gpon_bbf247_flag_read_proc,
	.proc_write	= NULL,
};

static const struct proc_ops proc_stage_change_chk_ops = {
	.proc_read	= gpon_stage_change_chk_read_proc,
	.proc_write	= gpon_stage_change_chk_write_proc,
};

static const struct proc_ops proc_power_management_ops = {
	.proc_read	= gpon_power_management_read_proc,
	.proc_write	= gpon_power_management_write_proc,
};

static const struct proc_ops proc_omci_oam_monitor_ops = {
	.proc_read	= xpon_omci_oam_monitor_read_proc,
	.proc_write	= xpon_omci_oam_monitor_write_proc,
};

static const struct proc_ops proc_los_led_ops = {
	.proc_read	= xpon_los_led_read_proc,
	.proc_write	= xpon_los_led_write_proc,
};

static const struct proc_ops proc_rogue_mode_ops = {
	.proc_read	= xpon_rogue_mode_read_proc,
	.proc_write	= NULL,
};

static const struct proc_ops proc_fe_reset_happened_ops = {
	.proc_read	= xpon_fe_reset_happened_read_proc,
	.proc_write	= xpon_fe_reset_happened_write_proc,
};

static const struct proc_ops proc_max_dscv_gate_ops = {
	.proc_read	= max_dscv_gate_read_proc,
	.proc_write	= max_dscv_gate_write_proc,
};

static const struct proc_ops proc_err_status_ops = {
	.proc_read	= gpon_err_status_read_proc,
	.proc_write	= gpon_err_status_write_proc,
};

static const struct proc_ops proc_epon_dbg_ops = {
	.proc_read	= epon_debug_read_proc,
};
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_BOSA_CARLIBRATION)
static const struct proc_ops proc_gpon_phy_test_ops = {
	.proc_read	= gpon_phy_test_read_proc,
	.proc_write	= gpon_phy_test_write_proc,
};
#endif/*TCSUPPORT_COMPILE*/
#endif

static int xpon_proc_init(void)
{
	struct proc_dir_entry *temp_proc=NULL;
    
    if(gpon_proc_dir || xpon_proc_dir){
        return 0;
    }
    
    gpon_proc_dir=proc_mkdir("gpon", NULL);
    if(NULL == gpon_proc_dir){
        return -1;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	temp_proc = proc_create("gpon/debug", 0, NULL, &proc_gpon_debug_ops) ;
    if(NULL == temp_proc) {
        return -1;
    }
	
	temp_proc = proc_create("gpon/bbf247Flag", 0, NULL, &proc_bbf247_flag_ops);
    if(NULL == temp_proc) {
        return -1;
    }
	
	temp_proc = proc_create("gpon/stage_chk_cnt", 0, NULL, &proc_stage_change_chk_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
	temp_proc = proc_create("gpon/power_management", 0, NULL, &proc_power_management_ops) ;
    if(NULL == temp_proc) {
        return -1;
    }
	
#else
	temp_proc = create_proc_entry("debug", 0, gpon_proc_dir) ;
    if(NULL == temp_proc) {
        return -1;
    }
	temp_proc->read_proc = gpon_debug_read_proc ;
	temp_proc->write_proc = gpon_debug_write_proc ;
#ifdef CONFIG_TP_IMAGE
	temp_proc = create_proc_entry("linkstate", 0, gpon_proc_dir) ;
	if(NULL == temp_proc) {
		return -1;
	}
	temp_proc->read_proc = gpon_linkstate_read_proc;
	temp_proc->write_proc = NULL;

	temp_proc = create_proc_entry("state", 0, gpon_proc_dir) ;
	if(NULL == temp_proc) {
		return -1;
	}
	temp_proc->read_proc = gpon_state_read_proc;
	temp_proc->write_proc = NULL;

	temp_proc = create_proc_entry("onuid", 0, gpon_proc_dir) ;
	if(NULL == temp_proc) {
		return -1;
	}
	temp_proc->read_proc = gpon_onuid_read_proc;
	temp_proc->write_proc = NULL;
#endif

	temp_proc = create_proc_entry("fastmode", 0, gpon_proc_dir) ;
	if(NULL == temp_proc) {
		return -1;
	}
	temp_proc->read_proc = gpon_fast_slow_read_proc ;
	temp_proc->write_proc = gpon_fast_slow_write_proc ;
	
	temp_proc = create_proc_entry("bbf247Flag", 0, gpon_proc_dir);
    if(NULL == temp_proc) {
        return -1;
    }
    temp_proc->read_proc = gpon_bbf247_flag_read_proc;
    temp_proc->write_proc = NULL;

	temp_proc = create_proc_entry("stage_chk_cnt", 0, gpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc = gpon_stage_change_chk_read_proc;
	temp_proc->write_proc = gpon_stage_change_chk_write_proc;

	temp_proc = create_proc_entry("power_management", 0, gpon_proc_dir) ;
    if(NULL == temp_proc) {
        return -1;
    }
	temp_proc->read_proc  = gpon_power_management_read_proc ;
	temp_proc->write_proc = gpon_power_management_write_proc ;
#endif

	xpon_proc_dir = proc_mkdir("xpon", NULL);
    if(NULL == xpon_proc_dir){
        return -1;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	temp_proc = proc_create("xpon/omci_oam_monitor", 0, NULL, &proc_omci_oam_monitor_ops);
    if(NULL == temp_proc) {
        return -1;
    }
	
	temp_proc = proc_create("xpon/los_led",  0, NULL, &proc_los_led_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
	temp_proc = proc_create("xpon/rogue_mode",  0, NULL, &proc_rogue_mode_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
	temp_proc = proc_create("xpon/fe_reset_happened",  0, NULL, &proc_fe_reset_happened_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
#else
	temp_proc = create_proc_entry("omci_oam_monitor", 0, xpon_proc_dir);
    if(NULL == temp_proc) {
        return -1;
    }
    temp_proc->read_proc  = xpon_omci_oam_monitor_read_proc;
	temp_proc->write_proc = xpon_omci_oam_monitor_write_proc;

	temp_proc = create_proc_entry("los_led",  0, xpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc  = xpon_los_led_read_proc;
	temp_proc->write_proc = xpon_los_led_write_proc;

	temp_proc = create_proc_entry("rogue_mode",  0, xpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc  = xpon_rogue_mode_read_proc;
	temp_proc->write_proc = NULL;
    

	temp_proc = create_proc_entry("fe_reset_happened",  0, xpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc  = xpon_fe_reset_happened_read_proc;
	temp_proc->write_proc = xpon_fe_reset_happened_write_proc;
#endif
    epon_proc_dir = proc_mkdir("epon", NULL);
    if(NULL == epon_proc_dir){
        return -1;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	temp_proc = proc_create("epon/max_dscv_gate",  0, NULL, &proc_max_dscv_gate_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
	temp_proc = proc_create("epon/debug",  0,  NULL, &proc_epon_dbg_ops);
	if(NULL == temp_proc){
		return -1;
	}
	
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_BOSA_CARLIBRATION)
		//gpon phy proc
	temp_proc = proc_create("tc3162/gpon_phy_test", 0, NULL, &proc_gpon_phy_test_ops);
    if(NULL == temp_proc){
        return -1;
    }
#endif/*TCSUPPORT_COMPILE*/

	temp_proc = proc_create("epon/errsts",  0, NULL, &proc_err_status_ops);
	if(NULL == temp_proc){
		return -1;
	}
#else
	temp_proc = create_proc_entry("max_dscv_gate",  0, epon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}

	temp_proc->read_proc = max_dscv_gate_read_proc;
	temp_proc->write_proc = max_dscv_gate_write_proc;
    
    temp_proc = create_proc_entry("debug",  0, epon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
    temp_proc->read_proc = epon_debug_read_proc;

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_BOSA_CARLIBRATION)
		//gpon phy proc
	temp_proc = create_proc_entry("tc3162/gpon_phy_test", 0, NULL);
    if(NULL == temp_proc){
        return -1;
    }
	temp_proc->read_proc = gpon_phy_test_read_proc;
	temp_proc->write_proc = gpon_phy_test_write_proc;
#endif/*TCSUPPORT_COMPILE*/

	temp_proc = create_proc_entry("errsts",  0, gpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc  = gpon_err_status_read_proc;
	temp_proc->write_proc = gpon_err_status_write_proc;
#endif

    return 0;
}


struct ecnt_hook_ops xpondrv_hook_dispatch_ops = {
    .name = "xpon_mac_hook_dispatch",
    .hookfn = xpon_mac_hook_dispatch,
    .is_execute = 1,
    .maintype = ECNT_XPON_MAC,
    .subtype = ECNT_XPON_MAC_HOOK,
};

/*****************************************************************************
******************************************************************************/
void xpondrv_cleanup(void)
{
#ifdef TCSUPPORT_CPU_ARMV8
	int irq = -1;
	struct device *dev = NULL ;
#endif

    XPON_STOP_TIMER(gpPhyData->check_sync_timer)           ;
    if(gpPonSysData != NULL)
    	gpPonSysData->sysStartup = PON_WAN_STOP                ;
    XPON_STOP_TIMER(gpPhyData->trans_status_refresh_timer) ;
	if(gpPonSysData != NULL)
    	gpPonSysData->sysMACStartup = PON_MAC_STOP             ;
    XPON_STOP_TIMER(gpPhyData->traffic_status_refresh_timer) ;


	xpon_daemon_quit();
	

#ifdef TCSUPPORT_WAN_GPON
    gpon_deinit() ;
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
    eponExit(); 
#endif /* TCSUPPORT_WAN_EPON */

    pon_mci_destroy() ;
    
    pwan_destroy() ;
	xpondrv_qdma_deinit();
#ifdef TCSUPPORT_CPU_ARMV8
	irq = get_xpon_irq(1);
	dev = get_xpon_dev();
	free_irq(irq, dev);
#else
    free_irq(DYINGGASP_INT, NULL);
#endif

    stop_omci_oam_monitor();
	
    if(gpPonSysData != NULL) {
        kfree(gpPonSysData) ;
        gpPonSysData = NULL ;
    }
#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
	if(gpon_proc_dir){
		remove_proc_entry("gpon/debug", NULL);
		remove_proc_entry("gpon/power_management", NULL);
		remove_proc_entry("gpon/stage_chk_cnt", NULL);
	}
	remove_proc_entry("gpon", NULL);
	
	if(xpon_proc_dir){
		remove_proc_entry("xpon/ponInfo", NULL);
		remove_proc_entry("xpon/fe_reset_happened", NULL);
		remove_proc_entry("xpon/los_led", NULL);
		remove_proc_entry("xpon/rogue_mode", NULL);
		remove_proc_entry("xpon/omci_oam_monitor", NULL);
	}
	remove_proc_entry("xpon", NULL);
	
	if(epon_proc_dir){
		remove_proc_entry("epon/max_dscv_gate", NULL);
		remove_proc_entry("epon/debug", NULL);
	}
	remove_proc_entry("epon", NULL);
#else
    if(gpon_proc_dir){
        remove_proc_entry("power_management", gpon_proc_dir);
        remove_proc_entry("debug", gpon_proc_dir);
	remove_proc_entry("stage_chk_cnt", gpon_proc_dir);
        remove_proc_entry("errsts", gpon_proc_dir);
        remove_proc_entry("bbf247Flag", gpon_proc_dir);
	remove_proc_entry("fastmode", gpon_proc_dir);
    }
    remove_proc_entry("gpon",NULL);
	
    if (xpon_proc_dir) {
        remove_proc_entry("ponInfo",xpon_proc_dir);
	remove_proc_entry("fe_reset_happened",xpon_proc_dir);
        remove_proc_entry("los_led",xpon_proc_dir);
        remove_proc_entry("rogue_mode",xpon_proc_dir);
        remove_proc_entry("omci_oam_monitor",xpon_proc_dir);
    }
    remove_proc_entry("xpon",NULL);

    if (epon_proc_dir){
        remove_proc_entry("max_dscv_gate",epon_proc_dir);
        remove_proc_entry("debug",epon_proc_dir);
    }
    remove_proc_entry("epon",NULL);
#endif

    if(epon_stat_prev)
        kfree(epon_stat_prev);

    ecnt_unregister_hook(&xpondrv_hook_dispatch_ops);
    log_task_control = LOG_TASK_EXIT;
}

/*****************************************************************************
******************************************************************************/

int xpon_check_emergence_state(void)
{
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)
	if((PHY_GPON_CONFIG == gpPhyData->working_mode) || (PHY_GPON_SYM_CONFIG == gpPhyData->working_mode))
	{
		if(gpGponPriv->gponCfg.emergencystate == XPON_ENABLE)
		{
			PON_MSG(MSG_INT, "%s Now Gpon in emergence_state return TRUE\n",__FUNCTION__) ;
			return TRUE;
		}
		else
			return FALSE;
	}
	else if(PHY_EPON_CONFIG == gpPhyData->working_mode)
	{
		//if(gpPhyPriv->phyCfg.flags.txPowerEnFlag == PHY_DISABLE)
		if(eponDrv.eponTxPowerFlag == XPON_PHY_TX_POWER_OFF)
		{
			PON_MSG(MSG_INT, "%s Now Epon in tx power off return TRUE\n",__FUNCTION__) ;
			return TRUE;
		}
		else
			return FALSE;
	}
#endif/*TCSUPPORT_COMPILE*/
	return FALSE;
}



/*****************************************************************************
******************************************************************************/
int xpondrv_init(void)
{
	printk("xPON driver initialization\n") ;
    
    INIT_LIST_HEAD(&fix_reg_list);

    /* initial the global data memory */
	gpPonSysData = (PON_SysData_T *)kmalloc(sizeof(PON_SysData_T), GFP_KERNEL) ;
	if(gpPonSysData == NULL) {
		printk("Alloc data struct memory failed\n") ;
		goto ret ;
	} else {
		memset(gpPonSysData, 0, sizeof(PON_SysData_T));
		printk("Alloc data struct memory successful, %d\n",(uint)sizeof(PON_SysData_T)) ;
		xpondrv_init_global_data() ;
	}

    omci_oam_monitor_init(&gpPonSysData->Omci_Oam_Monitor) ;

	/* WAN/OAM/OMCI interface driver initizliation */
	if(pwan_init() != 0) {
		printk("PON WAN/OAM/OMCI interface driver initialization failed\n") ;
		goto ret ;
	}
 
	/* XPON character device driver initizliation */
	if(pon_mci_init() != 0) {
		printk("PON monitor and control interface driver initialization failed\n") ;
		goto ret ;
	}
	if (ECNT_REGISTER_SUCCESS != ecnt_register_hook(&xpondrv_hook_dispatch_ops) ){
        panic("Register hook function failed! %s:%d", __FUNCTION__, __LINE__);
    }
	/* EPON initialization */
	if(eponInit() != 0) {
		printk("GPON initialization failed\n") ;
		goto ret ;
	}
	/* GPON initialization */
	if(gpon_init() != 0) {
		printk("GPON initialization failed\n") ;
		goto ret ;
	}
	
	if(xpondrv_qdma_init() != 0) {
		printk("QDMA buffer manager initialization failed\n") ;
		goto ret ;
	}
	
	if(xpon_dying_gasp_init() != 0) {
		printk("xpon dying gasp initialization failed\n") ;
		goto ret ;
	}

	if(xpon_proc_init() != 0) {
		printk("xpon vlan filter initialization failed\n") ;
		goto ret ;
	}
  
	/*LED flicker*/	
	change_alarm_led_status(ALARM_LED_FLICKER);

    gpPonSysData->sysMACStartup = PON_MAC_START;
    XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer,1000) ;/* 1,000 ms */
	return 0 ;

ret:
	xpondrv_cleanup() ;
	return -EFAULT ;
}

void xpon_stop_timer(void)
{
    /*Don't access phy register, so timer don't stop*/
    /*XPON_STOP_TIMER(gpPhyData->trans_status_refresh_timer);*/

    if(PHY_GPON_CONFIG == gpPhyData->working_mode){
        gpon_stop_timer();
    }else if(PHY_EPON_CONFIG == gpPhyData->working_mode){
        /* todo: addd epon_stop_timer */
    }
}

void xpon_exec_gem_recover(void)
{
	DBG_INFO(gponRecovery.dbgPrint, "recover\n");
	if(!gpGponPriv->pGponRecovery->mibFlag)
	{
		gpon_recover_create_gemport();
	}
	gpGponPriv->pGponRecovery->mibFlag = 0;
}

module_init(xpondrv_init)
module_exit(xpondrv_cleanup)
MODULE_LICENSE("GPL");

