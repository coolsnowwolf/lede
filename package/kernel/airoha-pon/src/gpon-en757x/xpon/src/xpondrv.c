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

#include "../inc/common/drv_global.h"
#include "../inc/common/phy_if_wrapper.h"

#include "../inc/epon/epon.h"
#include "../inc/epon/epon_mpcp.h"
#include "../inc/epon/epon_reg.h"
#include "../inc/gpon/gpon.h"
#include "../inc/gpon/gpon_init.h"
#include "../inc/gpon/gpon_dvt.h"
#include "../inc/gpon/gpon_reg.h"
#include "../inc/pwan/xpon_netif.h"
#include "../inc/xmcs/xmcs_mci.h"
#include "../inc/xmcs/xmcs_sdi.h"
#include "../inc/xmcs/xmcs_if.h"
#include "../inc/pwan/gpon_wan.h"

#ifdef TCSUPPORT_CPU_EN7521
#include "gpon/gpon_power_management.h"
#include <ecnt_hook/ecnt_hook_fe.h>
#endif
#include "../inc/common/xpon_daemon.h"
#include "../inc/common/xpon_led.h"

#include <ecnt_hook/ecnt_hook_qdma_7510_20.h>

#include "gpon/gpon_recovery.h"

extern int xpon_los_status ;
extern unsigned char RDI_SEND_ENABLE;
extern unsigned int RDI_SEND_TIMES;
extern unsigned int RDI_SEND_DELAY;

#ifdef CONFIG_USE_FOR_TEST
struct workqueue_struct * gem_valid_work_queue = NULL;
atomic_t work_queue_valid = ATOMIC_INIT(0);
#endif /* CONFIG_USE_FOR_TEST */

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
static uint epon_proc_cnt = 0;
xpon_regs *xpon_regs_p = NULL;
static epon_stat *epon_stat_prev = NULL;
static struct proc_dir_entry *xpon_proc_dir = NULL;
static struct proc_dir_entry *gpon_proc_dir = NULL;
static struct proc_dir_entry *epon_proc_dir = NULL;
static struct proc_dir_entry *gpon_debug_proc_dir=NULL;
static struct proc_dir_entry *gpon_power_management_proc_dir=NULL;

PON_SysData_T   * gpPonSysData = NULL ;
PWAN_GlbPriv_T  * gpWanPriv    = NULL ;
MCS_GlbPriv_T   * gpMcsPriv    = NULL ;
GPON_GlbPriv_T  * gpGponPriv   = NULL ;
EPON_GlbPriv_T  * gpEponPriv   = NULL ;

#ifdef TCSUPPORT_AUTOBENCH
extern int epon_slt_test;
extern int gpon_slt_test;
extern int pon_loopback_result;
#endif

extern int xmcs_set_connection_start(XPON_Mode_t mode);
extern int xmcs_set_link_detection(XMCSIF_WanDetectionMode_t detection);

void trans_status_refresh(unsigned long arg)
{
    XPON_DAEMON_Job_data_t job;

    if(!gpPhyData->trans_status_refresh_pending ){
        job.id       = XPON_DAEMON_JOB_GET_PHY_PARAM ;
        job.priority = XPON_DAEMON_JOB_PRIORITY_LOW  ;
        gpPhyData->trans_status_refresh_pending = 1;
        xpon_daemon_job_enqueue(&job) ;
        wake_up_xpon_daemon() ;
    }

    if (
    #ifndef TCSUPPORT_CPU_EN7521
        0    == atomic_read(&fe_reset_flag)       &&
    #endif
        FALSE == gpPonSysData->ponMacPhyReset) 
    {
        XPON_START_TIMER(gpPhyData->trans_status_refresh_timer) ;
    }
}

void traffic_status_refresh(unsigned long arg)
{
    static ENUM_PhyTrafficStatus last_status = TRAFFIC_STATUS_DOWN;
    ENUM_PhyTrafficStatus status = TRAFFIC_STATUS_DOWN;
    static uint8 is_first = 1;
    

    if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
        status = ( GPON_TRAFFIC_UP == gwan_get_traffic_status() )? TRAFFIC_STATUS_UP: TRAFFIC_STATUS_DOWN;
    }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus ){
        status = ( 1 == eponGetTrafficStatus() )? TRAFFIC_STATUS_UP: TRAFFIC_STATUS_DOWN;
    }else {
        status = TRAFFIC_STATUS_DOWN;
    }

    if(last_status != status || is_first){
        last_status = status;
        XPON_PHY_SET_API(PON_SET_PHY_TRAFFIC_STATUS, &status);
    }

    if(is_first)
        is_first = 0;

    if (PON_MAC_START == gpPonSysData->sysMACStartup)
        XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer);
    return;
}

static void xpondrv_init_phy_data(void)
{
    gpPhyData->is_fix_mode   = FALSE;
    gpPhyData->calibrating   = FALSE;
    gpPhyData->working_mode  = PHY_UNKNOWN_CONFIG    ;
    gpPhyData->detect_mode   = PHY_UNKNOWN_CONFIG    ;
    gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_NOT_START ;
    gpPhyData->phy_link_status     = PHY_LINK_STATUS_UNKNOWN ;

    spin_lock_init(&gpPhyData->trans_params_lock);
    memset(&gpPhyData->trans_params, 0 , sizeof(gpPhyData->trans_params));

	init_timer(&gpPhyData->trans_status_refresh_timer) ;/*used for transceiver alarm*/
	gpPhyData->trans_status_refresh_timer.data = 1000 ; /* 1,000 ms */
	gpPhyData->trans_status_refresh_timer.function = trans_status_refresh ;
    gpPhyData->trans_status_refresh_pending = 0 ;

	init_timer(&gpPhyData->delay_start_detect_timer) ;
	gpPhyData->delay_start_detect_timer.data = 3000  ;  /* 1s */
	gpPhyData->delay_start_detect_timer.function =  try_set_phy_mode ;

	init_timer(&gpPhyData->check_sync_timer) ;
	gpPhyData->check_sync_timer.data = 1000  ;  /* 1s */
	gpPhyData->check_sync_timer.function =  check_sync_result ;
    
    init_timer(&gpPhyData->traffic_status_refresh_timer) ;/*used for transceiver alarm*/
	gpPhyData->traffic_status_refresh_timer.data = 1000 ; /* 1,000 ms */
	gpPhyData->traffic_status_refresh_timer.function = traffic_status_refresh ;
}

/*****************************************************************************
******************************************************************************/
static void xpondrv_init_global_data(void)
{
	gpPonSysData->sysOnuType = READ_FLASH_BYTE(flash_base + 0xff9c);
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

	if(gpPonSysData->sysOnuType != PON_ONU_TYPE_HGU)//set default onu type to SFU
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
    gpPonSysData->fe_reset_happened = FALSE;
    xpondrv_init_phy_data();

#ifndef TCSUPPORT_CPU_EN7521
	init_timer(&gpPonSysData->gsw_p6_rate_timer) ;
	gpPonSysData->gsw_p6_rate_timer.data = 5000 ; /* 5 s */
	gpPonSysData->gsw_p6_rate_timer.function = xpon_disable_cpu_protection;
    gpPonSysData->gswRateLimitFlag = FALSE ;
#endif
    gpPonSysData->isUpDataTraffic    = XPON_DISABLE ;
    gpPonSysData->isUpOmciOamTraffic = XPON_DISABLE ;
    gpPonSysData->ponMacPhyReset     = FALSE ;

    spin_lock_init(&gpPonSysData->event_lock) ;
}


/******************************************************************************
******************************************************************************/
static int gpon_debug_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	int cpu_type = 0;
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
	#if 0
	if (isMT7520S)
		cpu_type = 1;
	else if (isMT7520)
		cpu_type = 2;
	else if (isMT7520G)
		cpu_type = 3;
	else if (isMT7525)
		cpu_type = 4;
	else if (isMT7525G)
		cpu_type = 5;
	else if (isEN751221)
		cpu_type = 6;
	#endif
	int i;
	for ( i = 0; i <ARRAY_SIZE(devices_table); i++ )
	{
		if(devices_table[i].chipid)
		{
			cpu_type = devices_table[i].onu_type_id;
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

#if 0
	off_t pos=0, begin=0 ;

	/* need to add the debug information */
	//index += sprintf(buf+ index, "CSR: G_PLOAMu_FIFO_STS   	:%.8x\n", G_PLOAMu_FIFO_STS->Raw) ; 
	//CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0)		index = 0 ;
	if(index>count)		index = count ;
#endif
	return index ;
}

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
unsigned long int dropThred = 500;
uint8 rxDropCheckTimer = 5;
#endif
#define isdigit(x)	((x)>='0'&&(x)<='9')
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



#ifndef TCSUPPORT_CPU_EN7521
#define FE_GDMA2_FWD_CFG 0xbfb51500
#endif

#ifndef TCSUPPORT_CPU_EN7521
static void print_debug_param(void)
{
    printk(" \t macPhyReset: %x\n \t rstDbgDly: %x\n"
           " \t reg4208: %x\n \t responseTime: %x\n", 
            gpGponPriv->gponCfg.macPhyReset, gpGponPriv->gponCfg.rstDbgDly,
            gpGponPriv->gponCfg.reg4208, gpGponPriv->gponCfg.onuResponseTime);
}
#endif /* TCSUPPORT_CPU_EN7521 */

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
   
	dev_put(dev);
	
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


extern __u8 green_drop_flag;
#ifdef TCSUPPORT_VNPTT
extern __u8 ignore_deactive_flag;
#endif
irqreturn_t xpon_dying_gasp_interrupt(int irq, void *dev_id);

#ifdef CONFIG_USE_FOR_TEST
static int gem_valid_work_queue_func(struct work_struct * worker)
{
	int open = 0;
    int i = 0;
	struct XMCS_GemPortInfo_S gemInfo ;
	uint gemPortId, gemIdx ;
    int cnt = 0;
    
	printk("do gem_valid_work_queue_func start !!!\n");

	if(xmcs_get_gem_port_info(&gemInfo) < 0)
		printk("exec failed") ;
            
	while(atomic_read(&work_queue_valid)){
        cnt += 2;
		open=0;
		for(i=0 ; i<gemInfo.entryNum ; i++) {
			gemPortId = gemInfo.info[i].gemPortId;
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK); 	
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel){
				gpWanPriv->gpon.gemPort[gemIdx].info.valid = (open)?1:0 ;
				gponDevSetGemInfo(gemPortId, (open)?XPON_ENABLE:XPON_DISABLE, XPON_DISABLE) ;
			}
		}
		//msleep(1);
		if (cnt == 200){
            msleep(1);
            cnt = 1 ;
        }
		open=1;

		for(i=0 ; i<gemInfo.entryNum ; i++) {
			gemPortId = gemInfo.info[i].gemPortId;
			gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK); 	
			if(gpWanPriv->gpon.gemPort[gemIdx].info.channel){
				gpWanPriv->gpon.gemPort[gemIdx].info.valid = (open)?1:0 ;
				gponDevSetGemInfo(gemPortId, (open)?XPON_ENABLE:XPON_DISABLE, XPON_DISABLE) ;
			}
		}
		if (cnt >= 201){
            msleep(1);
            cnt = 0 ;
        }
    }
    printk("do gem_valid_work_queue_func end !!!\n");
}

extern struct work_struct gem_valid_work_queue_struct;
DECLARE_WORK(gem_valid_work_queue_struct, gem_valid_work_queue_func);

extern uint eqdDbgFlag;
extern uint EqdCounter;
extern uint EqdArray[1001];
#endif /* CONFIG_USE_FOR_TEST */

static int gpon_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	extern __u8 eponStaticRptEnable;
#ifdef TCSUPPORT_PON_TEST
	extern __u32 eponDrvDbgMask;
	extern __u8 eponDrvTmpMacAddr[];
#endif
	char val_string[64], cmd[32], subcmd[32] ;
	uint action ;
	int i, j ;
	int ret;
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%s %s %x", cmd, subcmd, &action) ;

    if(!strcmp(cmd, "show_flag")) {
        printk("sysLinkStatus:\t %d\n",gpPonSysData->sysLinkStatus);
        printk("sysStartup:\t %d\n",gpPonSysData->sysStartup);
    #ifndef TCSUPPORT_CPU_EN7521
        printk("txPacketCount:\t %d\n",gpWanPriv->devCfg.txPacketCount);
    #endif
        printk("drop_print_flag:\t %d\n",drop_print_flag);
		printk("xpon_mac_print_open:\t %d\n",xpon_mac_print_open);
#ifndef TCSUPPORT_CPU_EN7521
        extern atomic_t fe_reset_flag;
        printk("fe_reset_flag:\t %d\n", atomic_read(&fe_reset_flag));
#endif /* TCSUPPORT_CPU_EN7521 */
	}
#ifdef TCSUPPORT_CPU_EN7521
	else if(!strcmp(cmd, "set")) {
		if(!strcmp(subcmd, "rx_ratelimit")) {
			gpon_enable_cpu_protection(action);
			printk("set rx ratelimit %d FPS success\n", action);
		}
	}
#endif
	
	else if(!strcmp(cmd, "gpon_recovery_dbg"))
	{
		if(!strcmp(subcmd, "1")){
			gponRecovery.dbgPrint = 1;
		}else{
			gponRecovery.dbgPrint = 0;
		}
	}
	
    else if(!strcmp(cmd, "olttype")) {
		if(!strcmp(subcmd, "set")) {
			gpPonSysData->sysOltType = action;
			printk("set olt type: %d\n",gpPonSysData->sysOltType);
		} else{
			printk("get olt type: %d\n",gpPonSysData->sysOltType);
		}
	} else	if (!strcmp(cmd, "pwan")){
		if (!strcmp(subcmd, "qos")){
			xpon_set_qdma_qos(action);
			ECNT_QDMA_GREEN_DROP_CTRL_HOOK(ECNT_QDMA_SET_QOS_FLAG,action);
		}else if (!strcmp(subcmd, "mapqos")){
			QDMA_TxQosScheduler_T txQosSch;

			if (action ==1 ){
				if (gpWanPriv->devCfg.flags.isQosUp){
					printk("xPon: Qos is aleady up.\n");
					return count;
				}
				xpon_set_qdma_qos(1);
			}else if (action == 0){
				xpon_set_qdma_qos(0);
			}
			
			// set qos scheduler to sp mode
			memset(&txQosSch, 0, sizeof(QDMA_TxQosScheduler_T));
			for (i = 0; i < CONFIG_QDMA_QUEUE; ++i){
				txQosSch.queue[i].weight= 255; // skip set queue weight
			}

			for (i = 0; i < CONFIG_QDMA_CHANNEL; ++i){
				txQosSch.channel = i;
				txQosSch.qosType = QDMA_TXQOS_TYPE_SP;
#ifdef TCSUPPORT_CPU_EN7521
				QDMA_API_SET_TX_QOS(ECNT_QDMA_WAN, &txQosSch);
#else /* TCSUPPORT_CPU_EN7521 */
				qdma_set_tx_qos(&txQosSch);
#endif /* TCSUPPORT_CPU_EN7521 */
			}
		}else if (!strcmp(subcmd, "drop")){
			if (action > 3000){
				printk("Set TxDropTimer = %u\n", action);
				gpWanPriv->txDropTimer.data = action;
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
#ifdef TCSUPPORT_CPU_EN7521
		else if (!strcmp(subcmd, "txoamfavor") && isEN7526c){
			if(action == 0) {
				gpEponPriv->eponCfg.txOamFavorMode = 0;	
				printk("Tx OAM Favor mode is disabled\n") ;		
			} else if (action == 1) {
				gpEponPriv->eponCfg.txOamFavorMode = 1;	
				printk("Tx OAM Favor mode is enabled\n") ;	
			}
		}
#endif
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
		printk("The threld for drop packets is 0x%08lx", dropThred);
	}
	else if (!strcmp(cmd, "dropTimer")){
		int dropTimer = atoi_temp(subcmd);
		if(dropTimer>=1 && dropTimer<=60)
			rxDropCheckTimer = dropTimer;
		printk("The rx drop check timer is 0x%08lx", rxDropCheckTimer);
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
		} 
		printk("Debug Level: %x\n", gpPonSysData->debugLevel) ; 
	}  

#ifdef TCSUPPORT_WAN_GPON
	else if(!strcmp(cmd, "csr")) {
		if(!strcmp(subcmd, "gem")) {
			gpon_dvt_gemport(action) ;
		}
		else if(!strcmp(subcmd, "tcont")) {
			if(G_TCONT_ID_0_1->Bits.t_cont0_vld)
				printk("Channel 0 valid, ID:%d\n", G_TCONT_ID_0_1->Bits.t_cont0_id) ; 
			if(G_TCONT_ID_0_1->Bits.t_cont1_vld)
				printk("Channel 1 valid, ID:%d\n", G_TCONT_ID_0_1->Bits.t_cont1_id) ; 
			if(G_TCONT_ID_2_3->Bits.t_cont2_vld)
				printk("Channel 2 valid, ID:%d\n", G_TCONT_ID_2_3->Bits.t_cont2_id) ; 
			if(G_TCONT_ID_2_3->Bits.t_cont3_vld)
				printk("Channel 3 valid, ID:%d\n", G_TCONT_ID_2_3->Bits.t_cont3_id) ; 
			if(G_TCONT_ID_4_5->Bits.t_cont4_vld)
				printk("Channel 4 valid, ID:%d\n", G_TCONT_ID_4_5->Bits.t_cont4_id) ; 
			if(G_TCONT_ID_4_5->Bits.t_cont5_vld)
				printk("Channel 5 valid, ID:%d\n", G_TCONT_ID_4_5->Bits.t_cont5_id) ; 
			if(G_TCONT_ID_6_7->Bits.t_cont6_vld)
				printk("Channel 6 valid, ID:%d\n", G_TCONT_ID_6_7->Bits.t_cont6_id) ; 
			if(G_TCONT_ID_6_7->Bits.t_cont7_vld)
				printk("Channel 7 valid, ID:%d\n", G_TCONT_ID_6_7->Bits.t_cont7_id) ; 
			if(G_TCONT_ID_8_9->Bits.t_cont8_vld)
				printk("Channel 8 valid, ID:%d\n", G_TCONT_ID_8_9->Bits.t_cont8_id) ; 
			if(G_TCONT_ID_8_9->Bits.t_cont9_vld)
				printk("Channel 9 valid, ID:%d\n", G_TCONT_ID_8_9->Bits.t_cont9_id) ; 
			if(G_TCONT_ID_10_11->Bits.t_cont10_vld)
				printk("Channel 10 valid, ID:%d\n", G_TCONT_ID_10_11->Bits.t_cont10_id) ; 
			if(G_TCONT_ID_10_11->Bits.t_cont11_vld)
				printk("Channel 11 valid, ID:%d\n", G_TCONT_ID_10_11->Bits.t_cont11_id) ; 
			if(G_TCONT_ID_12_13->Bits.t_cont12_vld)
				printk("Channel 12 valid, ID:%d\n", G_TCONT_ID_12_13->Bits.t_cont12_id) ; 
			if(G_TCONT_ID_12_13->Bits.t_cont13_vld)
				printk("Channel 13 valid, ID:%d\n", G_TCONT_ID_12_13->Bits.t_cont13_id) ; 
			if(G_TCONT_ID_14_15->Bits.t_cont14_vld)
				printk("Channel 14 valid, ID:%d\n", G_TCONT_ID_14_15->Bits.t_cont14_id) ; 
			if(G_TCONT_ID_14_15->Bits.t_cont15_vld)
				printk("Channel 15 valid, ID:%d\n", G_TCONT_ID_14_15->Bits.t_cont15_id) ; 
		}
	} else if(!strcmp(cmd, "test")) {
		if(!strcmp(subcmd, "loopback")) {
			for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
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
				IO_SREG(G_PLOAMu_WDATA, 0) ;
				IO_SREG(G_PLOAMu_WDATA, 0) ;
				IO_SREG(G_PLOAMu_WDATA, 0) ;				
				DBG_US_DYING_GASP_CTRL->Bits.dying_gasp_test = 1 ;
			}
		}
        if(!strcmp(subcmd,"rdi_enable"))
        {
            RDI_SEND_ENABLE = (action == 1)? 1 : 0;
            printk("Software RDI is %s \n",(RDI_SEND_ENABLE == 1) ? "enabled" : "disabled");
        }
        if(!strcmp(subcmd,"rdi_times"))
        {
            RDI_SEND_TIMES = action;
            printk("Software RDI is %s .Should send RDI for %d times delay %d us \n",
                (RDI_SEND_ENABLE == 1) ? "enabled" : "disabled",RDI_SEND_TIMES,RDI_SEND_DELAY);
        }
        if(!strcmp(subcmd,"rdi_delay"))
        {
            RDI_SEND_DELAY = action;
            printk("Software RDI is %s .Should send RDI for %d times delay %d us \n",
                (RDI_SEND_ENABLE == 1) ? "enabled" : "disabled",RDI_SEND_TIMES,RDI_SEND_DELAY);
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
        	uint32 action2 = 0;
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
		gpon_cmd_proc(GPON_IOG_ONU_INFO, (ulong)&onuInfo) ;
		memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
		
		memcpy(gponSnPasswd.sn, subcmd, 4) ;
		gponSnPasswd.sn[4] = (unchar)(action>>24) ;
		gponSnPasswd.sn[5] = (unchar)(action>>16) ;
		gponSnPasswd.sn[6] = (unchar)(action>>8) ;
		gponSnPasswd.sn[7] = (unchar)(action) ;
		gpon_cmd_proc(GPON_IOS_SN_PASSWD, (ulong)&gponSnPasswd) ;
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
			struct XMCS_GemPortInfo_S gemInfo ;
			uint gemPortId, gemIdx ;
			if(xmcs_get_gem_port_info(&gemInfo) < 0) {
				printk("exec failed") ;
			} else {
				for(i=0 ; i<gemInfo.entryNum ; i++) {
					gemPortId = gemInfo.info[i].gemPortId;
					gemIdx = (gpWanPriv->gpon.gemIdToIndex[gemPortId] & GPON_GEM_IDX_MASK);		
					printk("GEM ID:%d, Alloc ID:%d, If Idx:%d, Loopback:%s, Encryption:%s, valid is %d, chn:%d\n", 
								gemInfo.info[i].gemPortId, 
								gemInfo.info[i].allocId, 
								gemInfo.info[i].aniIdx, 
								(gemInfo.info[i].lbMode)?"ENABLE":"DISABLE", 
								(gemInfo.info[i].enMode)?"ENABLE":"DISABLE",
								gpWanPriv->gpon.gemPort[gemIdx].info.valid,
								gpWanPriv->gpon.gemPort[gemIdx].info.channel) ;
				}
			}
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
			struct XMCS_TcontInfo_S tcontInfo ;
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
			gpon_cmd_proc(GPON_IOG_ONU_INFO, (ulong)&onuInfo) ;
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
			uint tmp, rdata, tdata ;
			
			for(i=0 ; i<CONFIG_GPON_MAX_GEMPORT ; i++) {
				if(gpWanPriv->gpon.gemPort[i].info.valid) {
					if(action == gpWanPriv->gpon.gemPort[i].info.portId || action==0x10000) {
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_RX_FRAME_CNT, &tmp, &rdata) ;
						gponDevGetGemPortCounter(gpWanPriv->gpon.gemPort[i].info.portId, GEMPORT_TX_FRAME_CNT, &tmp, &tdata) ;
						printk("Gem Port:%d, SW Tx:%u, SW Rx:%u, HW Tx:%u, HW Rx:%u\n", 
										gpWanPriv->gpon.gemPort[i].info.portId,
										(uint)gpWanPriv->gpon.gemPort[i].stats.tx_packets, 
										(uint)gpWanPriv->gpon.gemPort[i].stats.rx_packets, 
										tdata, rdata) ;
					}
				}
			}
		} else if(!strcmp(subcmd, "gemcounter")) {
			struct XMCS_GponGemCounter_S gponGemCounter ;
			memset(&gponGemCounter, 0, sizeof(struct XMCS_GponGemCounter_S)) ;
			gponGemCounter.gemPortId = action ;

			get_counter_from_reg(&gponGemCounter) ;
			printk("Gem Port:%d\n", gponGemCounter.gemPortId);
			printk("GemFrame Rx High:%u, GemFrame Rx Low:%u\n", gponGemCounter.rxGemFrameH, gponGemCounter.rxGemFrameL) ;
			printk("GemPayload Rx High:%u, GemPayload Rx Low:%u\n", gponGemCounter.rxGemPayloadH, gponGemCounter.rxGemPayloadL) ;
			printk("GemFrame Tx High:%u, GemFrame Tx Low:%u\n", gponGemCounter.txGemFrameH, gponGemCounter.txGemFrameL) ;
			printk("GemPayload Tx High:%u, GemPayload Tx Low:%u\n", gponGemCounter.txGemPayloadH, gponGemCounter.txGemPayloadL) ;
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
				printk("\n");
			}
		}
	} else if(!strcmp(cmd, "gpon")) {
		if(!strcmp(subcmd, "startup")) {
			if((action == 0) || (action == 2)) { /*Disable or PowerDown*/
				if_cmd_proc(IO_IOS_WAN_LINK_START, action) ;
				//gpPonSysData->sysStartup = PON_WAN_STOP ;
				//gpon_disable() ;
				printk("Disable GPON ONU\n") ;
			} else {
				if_cmd_proc(IO_IOS_WAN_LINK_START, action) ;
				//gpPonSysData->sysStartup = PON_WAN_START ;
				//gpon_enable() ;
				printk("Enable GPON ONU\n") ;
			}
		} else if(!strcmp(subcmd, "clear")) {
			gponDevClearSwCounter();
			printk("Successful to clear the GEM counter\n") ;
		} else if(!strcmp(subcmd, "cntreset")) {
			gponDevClearHwCounter();
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
		} else if(!strcmp(subcmd, "sniffer")) {
			int i = 0;
			uint gemIdx = 0;
			uint value = 0;
			gpGponPriv->gponCfg.flags.sniffer = action ;
			if (gpGponPriv->gponCfg.flags.sniffer == XPON_ENABLE){
				#ifdef TCSUPPORT_CPU_EN7521
				FE_API_SET_DROP_CRC_ERR_ENABLE(FE_GDM_SEL_GDMA2, FE_DISABLE);
				FE_API_SET_DROP_RUNT_ENABLE(FE_GDM_SEL_GDMA2, FE_DISABLE);
				#else
				value = IO_GREG(FE_GDMA2_FWD_CFG);
				value &= ~(0x1800000);
				IO_SREG(FE_GDMA2_FWD_CFG, value);
				#endif
				for(i=0 ; i<GPON_MAX_GEM_ID ; i++) {
					gemIdx = gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK;
					if((gemIdx >= CONFIG_GPON_MAX_GEMPORT) ||(!gpWanPriv->gpon.gemPort[gemIdx].info.valid)) {	
						gponDevSetGemInfo(i, XPON_ENABLE, XPON_DISABLE) ;
					}
				}
			} else {
				#ifdef TCSUPPORT_CPU_EN7521
				FE_API_SET_DROP_CRC_ERR_ENABLE(FE_GDM_SEL_GDMA2, FE_ENABLE);
				FE_API_SET_DROP_RUNT_ENABLE(FE_GDM_SEL_GDMA2, FE_ENABLE);
				#else
				value = IO_GREG(FE_GDMA2_FWD_CFG);
				value |= (0x1800000);
				IO_SREG(FE_GDMA2_FWD_CFG, value);
				#endif
				for(i=0 ; i<GPON_MAX_GEM_ID ; i++) {
					gemIdx = gpWanPriv->gpon.gemIdToIndex[i] & GPON_GEM_IDX_MASK;
					if((gemIdx >= CONFIG_GPON_MAX_GEMPORT) ||(!gpWanPriv->gpon.gemPort[gemIdx].info.valid)) {	
						gponDevSetGemInfo(i, XPON_DISABLE, XPON_DISABLE) ;
					}
				}
			}
			printk("%s sniffer all gemport downstream data\n", (action)?"Enable":"Disable") ;		
#ifndef TCSUPPORT_CPU_EN7521
		} else if(!strcmp(subcmd, "dly_reset")) {
            gpGponPriv->gponCfg.rstDbgDly = action;
		} else if(!strcmp(subcmd, "dev_reset")) {
            gpGponPriv->gponCfg.macPhyReset = action;
		} else if(!strcmp(subcmd, "mac4208")) {
            gpGponPriv->gponCfg.reg4208 = action;
		} else if(!strcmp(subcmd, "showDbgParam")) {
            print_debug_param();
#endif /* TCSUPPORT_CPU_EN7521 */
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
	} else if(!strcmp(cmd, "retire")) {
#ifdef TCSUPPORT_CPU_EN7521
		ret = FE_API_SET_CHANNEL_RETIRE(FE_GDM_SEL_GDMA2, action, FE_LINKDOWN) ;
#else
		ret = qdma_set_channel_retire(action);
#endif
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
				IO_SREG(0xbfb51514, 0) ;
			} else if(action==0x1 || action==0x2 || action==0x4 || action==0x8) {
				if(!strcmp(subcmd, "gdmrx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x40 | action) ;
					IO_SREG(0xbfb51514, (action<<8)) ;
				} else if(!strcmp(subcmd, "cdmrx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x20 | action) ;
					IO_SREG(0xbfb51514, (action<<4)) ;
				} else if(!strcmp(subcmd, "cdmtx")) {
					gpGponPriv->gponCfg.dvtPcpCheck = (0x10 | action) ;
					IO_SREG(0xbfb51514, (action)) ;
				}
			}
			printk("Successful to set the PCP %.8x\n", IO_GREG(0xbfb51514)) ;
		} 
	}
	else if(!strcmp(cmd, "vlan_trans"))
	{
		uint val1,val2;
        ushort srcVlan,dstVlan;
	}
	else if(!strcmp(cmd, "epon"))
	{
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
#ifndef TCSUPPORT_CPU_EN7521
		if(!strcmp(subcmd, "mpcp_fwd"))
		{
			setMpcpFwd(action);
			printk("set epon mpcp fwd %s\n",(action)?"Enable":"Disable");
		}
#endif
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
	else if(!strcmp(cmd, "green_drop"))
	{
		if(!strcmp(subcmd, "disable"))
		{
			printk("disable green_drop config \n");
			green_drop_flag = FALSE;
			xpon_set_qdma_qos(XPON_DISABLE);
		}
		else if(!strcmp(subcmd, "enable"))
		{
			printk("enable green_drop config \n");
			green_drop_flag = TRUE;
			xpon_set_qdma_qos(XPON_ENABLE);
		}
	}
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
			for(i=0; i<=EqdCounter; i++)
				printk("EqdArray[%d] == %d\n", i,EqdArray[i]);
		}else if (!strcmp(subcmd, "empty")){
			EqdCounter = 0;
			memset(EqdArray,0,1001);
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
	}
#endif /* CONFIG_USE_FOR_TEST */

	return count ;
}

/*****************************************************************************
******************************************************************************/
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	static int qdma_reg_init(void)
	{
		int i;
		int ret;
#ifdef TCSUPPORT_CPU_EN7521
		QDMA_txCngstCfg_t pTxCngstCfg;
		QDMA_TxQDynCngstThrh_T txqDynCngstThrh;
#else
		QDMA_TxQueueCongestThreshold_T txqThreshold;
		QDMA_TxQueueCongestScale_T txqScale;
		QDMA_Mode_t qdmaMode = QDMA_ENABLE;
#endif
		QDMA_TxBufCtrl_T txBufUsage;
		
#if defined (QOS_REMARKING) || defined (TCSUPPORT_HW_QOS)
#ifdef TCSUPPORT_CPU_EN7521
		pTxCngstCfg.txCngstMode = QDMA_TXCNGST_DYNAMIC_NORMAL;
		QDMA_API_SET_TXQ_CNGST_AUTO_CONFIG(ECNT_QDMA_WAN, &pTxCngstCfg);
		QDMA_API_SET_TXQ_DEI_DROP_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);

		memset(&txqDynCngstThrh, 0, sizeof(QDMA_TxQDynCngstThrh_T));
		QDMA_API_GET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
		/* default for single Tcont */
		txqDynCngstThrh.dynCngstChnlMinThrh = 0xA0;
		QDMA_API_SET_TXQ_CNGST_DYNAMIC_THRESHOLD(ECNT_QDMA_WAN, &txqDynCngstThrh);
#else
		/* do QDMA congestion configuration, otherwise low
		 * priority packets may use all tx descriptors. 
		 * We suppose each queue (for all channels) can 
		 * use 16*5==80 dscps at most! */
		memset(&txqScale, 0, sizeof(QDMA_TxQueueCongestScale_T));
		txqScale.maxScale = QDMA_TXQUEUE_SCALE_16_DSCP;
		qdma_set_congestion_scale(&txqScale);
		
		memset(&txqThreshold, 0, sizeof(QDMA_TxQueueCongestThreshold_T));
		for (i = 0; i < TX_QUEUE_NUM; i++)
		{
			txqThreshold.queueIdx = i;
			txqThreshold.grnMaxThreshold = 5;
			qdma_set_congest_threshold(&txqThreshold);
		}

#endif
	
		/* limite PSE buffer usage for each channel, then
		 * low priority packes won't block high priority
		 * packets (in PSE buffer) for too long, in order
		 * to prevent highest priority packets from dropping */
		memset(&txBufUsage, 0, sizeof(QDMA_TxBufCtrl_T));
		txBufUsage.mode = QDMA_ENABLE;
		txBufUsage.chnThreshold = 0x20; //PSE blocks
		txBufUsage.totalThreshold = 0x20;
#ifdef TCSUPPORT_CPU_EN7521
		ret = QDMA_API_SET_TXBUF_THRESHOLD(ECNT_QDMA_WAN, &txBufUsage);
#else /* TCSUPPORT_CPU_EN7521 */
		ret = qdma_set_txbuf_threshold (&txBufUsage);
#endif /* TCSUPPORT_CPU_EN7521 */
		if (ret) {
			printk("\nFAILED(%s): qdma setting for txBufUsage\n" , __FUNCTION__);
			return -1;
		}
#endif
	
		return 0;
	}

#endif
static int xpondrv_qdma_init(void)
{
	QDMA_InitCfg_t qdmaInitCfg ;
	QDMA_TxBufCtrl_T txBufCtrl ;
	uint dbg=0 ;
	int ret ;
	
#if defined(TCSUPPORT_CT_PON) || defined(TCSUPPORT_XPON_HAL_API_QOS) || defined(TCSUPPORT_XPON_HAL_API_EXT)
	/* HW QDMA QoS init */
	qdma_reg_init();
#endif
		
	memset(&qdmaInitCfg, 0, sizeof(QDMA_InitCfg_t)) ;
#ifndef TCSUPPORT_CPU_EN7521
	qdmaInitCfg.txRecycleMode = QDMA_TX_INTERRUPT ;
#ifdef CONFIG_USE_RX_NAPI
	qdmaInitCfg.rxRecvMode = QDMA_RX_NAPI ;
#else
	qdmaInitCfg.rxRecvMode = QDMA_RX_INTERRUPT ;
#endif /* CONFIG_USE_RX_NAPI */
	qdmaInitCfg.txIrqThreshold = 32 ;
	qdmaInitCfg.txIrqPTime = 128 ;
	qdmaInitCfg.rxDelayInt = 0 ;
	qdmaInitCfg.rxDelayPTime = 0 ;
#ifdef TCSUPPORT_MERGED_DSCP_FORMAT
	qdmaInitCfg.cbXmitMsg = pwan_cb_prepare_tx_message ;
#endif /* TCSUPPORT_MERGED_DSCP_FORMAT */
	qdmaInitCfg.cbXmitFinish = pwan_cb_tx_finished ;
#endif
#if !defined(TCSUPPORT_XPON_HYBIRD) 
        qdmaInitCfg.cbRecvPkts = pwan_cb_rx_packet ;
#endif
	qdmaInitCfg.cbEventHandler = pwan_cb_event_handler ;
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_API_INIT(ECNT_QDMA_WAN, &qdmaInitCfg);
	QDMA_API_TX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_RX_DMA_MODE(ECNT_QDMA_WAN, QDMA_ENABLE);
	QDMA_API_LOOPBACK_MODE(ECNT_QDMA_WAN, QDMA_LOOPBACK_DISABLE) ;
#else /* TCSUPPORT_CPU_EN7521 */
	qdma_init(&qdmaInitCfg) ;

	do {
 		if(pwan_prepare_rx_buffer(NULL, NULL) != 0) {
			break ;
		}
		dbg++ ;
	} while(qdma_has_free_rxdscp()) ;

	qdma_dma_mode(QDMA_ENABLE, QDMA_ENABLE, QDMA_ENABLE) ;
	qdma_loopback_mode(QDMA_LOOPBACK_DISABLE) ;
#endif /* TCSUPPORT_CPU_EN7521 */

	//txBufCtrl.mode = QDMA_ENABLE ;
	//txBufCtrl.chnThreshold = 0xC0 ;
	//txBufCtrl.totalThreshold = 0xC0 ;
	//ret = qdma_set_txbuf_threshold(&txBufCtrl) ;
	//if(ret < 0) {
	//	return ret ;
	//}
	xpon_set_qdma_qos(0);

    PON_MSG(MSG_TRACE, "Prepare %d receive packet buffers in QDMA init stage.\n", dbg) ;

	return 0 ;
}
void delay1us(int us)
{
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;
	uint32 one_tick_unit = 1 * SYS_HCLK / 2;
	volatile uint32 tick_wait = us * one_tick_unit;
	volatile uint32 timer1_ldv = regRead32(CR_TIMER1_LDV);

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
/*****************************************************************************
******************************************************************************/
irqreturn_t xpon_dying_gasp_interrupt(int irq, void *dev_id)
{

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

#ifdef TCSUPPORT_WAN_GPON
	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_GPON) {
		if (gpPonSysData->dyingGaspData.isGponHwFlag == GPON_SW) {
			/* Clear pre dying gasp interrupt status */
			IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ; 
			gpon_detect_dying_gasp();
		}
	}
#endif 	/* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
	else	if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON) {	
		/* Clear pre dying gasp interrupt status */
		IO_SBITS(0xBFB00084, SCU_DYING_GASP_STATUS) ; 
		if (!gpPonSysData->dyingGaspData.isEponHwFlag) {
			xmit_dying_gasp_oam();
		}
	}
#endif /* TCSUPPORT_WAN_EPON */


#ifdef TCSUPPORT_DYING_GASP_OPTIMIZATION
        dying_gasp_setup_mem_cpu();
#endif

	return IRQ_HANDLED;
}

/*****************************************************************************
******************************************************************************/
static int xpon_dying_gasp_init(void)
{
	 /* Register Dying Gasp interrupt */
	if(request_irq(DYINGGASP_INT, xpon_dying_gasp_interrupt, 0, "dying gasp", NULL) != 0) {
		printk("Request the interrupt service routine fail, irq:%d.\n", DYINGGASP_INT) ;
		return -ENODEV ;
	}
	
	return 0;
}


void schedule_fe_reset(void)
{
#ifndef TCSUPPORT_CPU_EN7521
    XPON_DPRINT_MSG("#######About to do FE Reset!#######\n");

    XPON_DAEMON_Job_data_t job     ;
    job.id       = XPON_DAEMON_JOB_FE_RESET ;
    job.priority = XPON_DAEMON_JOB_PRIORITY_HIGH  ;
    gpPonSysData->sysStartup = PON_WAN_STOP;

    xpon_daemon_job_enqueue(&job)  ;
    
    wake_up_xpon_daemon() ;
#endif
}

extern atomic_t to1_timeout_cnt;
extern atomic_t to1_rst_cnt;
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
	
		*len += sprintf((*len+buf), "	Jiffies: %#lx(+%#lx +%#ld)\n", epon_stat_cur->jiffies_val, (epon_proc_cnt ? (epon_stat_cur->jiffies_val-epon_stat_prev->jiffies_val) : epon_proc_cnt), (epon_proc_cnt ? (epon_stat_cur->jiffies_val-epon_stat_prev->jiffies_val) : epon_proc_cnt));
		*len += sprintf((*len+buf), " Discover Gate Count:	 %#x(+%#x +%#d)\n", epon_stat_cur->dscvr_gate_cnt, (epon_proc_cnt ? (epon_stat_cur->dscvr_gate_cnt-epon_stat_prev->dscvr_gate_cnt) : epon_proc_cnt), (epon_proc_cnt ? (epon_stat_cur->dscvr_gate_cnt-epon_stat_prev->dscvr_gate_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " RX Register Count: 	 %#x(+%#d)\n", epon_stat_cur->rx_rgst_cnt, (epon_proc_cnt ? (epon_stat_cur->rx_rgst_cnt-epon_stat_prev->rx_rgst_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " TX Register Count: 	 %#x(+%#d)\n", epon_stat_cur->tx_rgst_cnt, (epon_proc_cnt ? (epon_stat_cur->tx_rgst_cnt-epon_stat_prev->tx_rgst_cnt) : epon_proc_cnt));
		*len += sprintf((*len+buf), " TX Register ACK Count:  %#x(+%#d)\n", epon_stat_cur->tx_rgst_ack_cnt, (epon_proc_cnt ? (epon_stat_cur->tx_rgst_ack_cnt-epon_stat_prev->tx_rgst_ack_cnt) : epon_proc_cnt));
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

static void epon_dump_llid_info(char *buf, int *len)
{
	__u8 llidIndex=0;
	eponLlid_t *llidEntry_p = NULL;
	char state[20] = {0};
	__u8 discStateIndex = 0;

	for(llidIndex=0;llidIndex < EPON_LLID_MAX_NUM;llidIndex++)
	{
		if((eponDrv.llidMask)&(1<<llidIndex))//this LLID enable
		{
			llidEntry_p = &(eponDrv.eponLlidEntry[llidIndex]);
			discStateIndex = llidEntry_p->eponMpcp.eponDiscFsm.mpcpDiscState;
			if(discStateIndex >= MAX_MPCP_STATE_NUM)
			{
				printk("MPCP disc State Index %d exceed max Num\n",discStateIndex);
				return;
			}
			strcpy(state,mpcpStateStr[discStateIndex]);
			*len += sprintf((*len+buf)," EPON LLID %d MPCP Disc State: %s \n",llidIndex,state);
			*len += sprintf((*len+buf)," EPON LLID %d is in  %s State \n",llidIndex, ((MPCP_STATE_DENIED == discStateIndex) ? "Silent":"Normal"));
			if(MPCP_STATE_DENIED == discStateIndex)
				*len += sprintf((*len+buf)," EPON LLID %d Silent Time left %d s(Total %d s) \n",llidIndex,llidEntry_p->eponMpcp.eponDiscFsm.rgstAgainTimeout, g_silence_time);
		}	
	}
}	


static int xpon_info_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	pon_mode = 0;

	if(!epon_proc_cnt) {
		epon_stat_prev = (epon_stat *)kmalloc(sizeof(epon_stat), GFP_KERNEL);
		if(epon_stat_prev == NULL){
			printk("Error: can't allocate buffer for epon_stat_prev.\n");
			return -1;
		}
	}
	epon_stat *epon_stat_cur = (epon_stat *)kmalloc(sizeof(epon_stat), GFP_KERNEL);
	if(epon_stat_cur == NULL){
		printk("Error: can't allocate buffer for epon_stat_cur.\n");
		return -1;
	}
	xpon_regs_p = (xpon_regs *)kmalloc(sizeof(xpon_regs), GFP_KERNEL);
	if(xpon_regs_p == NULL){
		printk("Error: can't allocate buffer for xpon_regs_p.\n");
		return -1;
	}

	len = sprintf(buf, "\n");
	len += sprintf(buf+len, "%s\n", "**************************************");
	len += sprintf(buf+len, "%s\n", "**        XPON MAC Status           **");
	len += sprintf(buf+len, "%s\n", "**************************************");
	len += sprintf(buf+len, "\n");

	// Get PON Mode
	xpon_regs_p->wan_mode_val = IO_GREG(SCU_WAN_CONF_REG) & 0x1;
	
	if(WAN_GPON_MODE == xpon_regs_p->wan_mode_val  && PHY_GPON_CONFIG == gpPhyData->working_mode) {
		pon_mode |= PON_MODE_GPON;
		len += sprintf(buf+len, " Mode: GPON\n");
	}else if(WAN_EPON_MODE == xpon_regs_p->wan_mode_val && PHY_EPON_CONFIG == gpPhyData->working_mode) {
		pon_mode |= PON_MODE_EPON;
		len += sprintf(buf+len, " Mode: EPON\n");
	}else
		len += sprintf(buf+len, " Mode: Error\n");
	
	// Get GPON Status
	if(pon_mode & PON_MODE_GPON) {
		xpon_regs_p->gpon_ploam_stat_val = IO_GREG(G_ACTIVATION_ST);
		xpon_regs_p->gpon_eqd_val        = IO_GREG(G_EQD);
		xpon_regs_p->gpon_resp_time_val  = IO_GREG(G_RSP_TIME);
		if(pon_mode & PHY_RD) {
			len += sprintf(buf+len, " Ploam state: O%d\n", (xpon_regs_p->gpon_ploam_stat_val)&0x7);
			len += sprintf(buf+len, " Equalization Delay: %#x\n", xpon_regs_p->gpon_eqd_val);			
			len += sprintf(buf+len, " Response Time: %#x\n", xpon_regs_p->gpon_resp_time_val);
		}
	}

	// Get EPON Status
	if(pon_mode & PON_MODE_EPON)
	{	
		get_epon_stat(buf, &len, epon_stat_cur);
		epon_dump_llid_info(buf, &len);
	}


	// Dump raw data
	xpon_dump_raw_data(buf, &len);

	kfree(epon_stat_cur);
	kfree(xpon_regs_p);

	len -= off;
	*start = buf + off;

	if(len > count)
		len = count;
	else
		*eof = 1;

	if(len < 0)
		len = 0;

	return len;
}

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

    change_alarm_led_status(ALARM_LED_FLICKER);
	xpon_los_status = 0;

    switch(gpPhyData->working_mode)
    {
        case PHY_GPON_CONFIG:
            gpon_detect_los_lof();
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

            default:
                return 0;
        }
    }

    *argc = cnt;
    return 1;
}

static void xpon_phy_ready_handler(PHY_Event_Source_t src)
{
    uint   job_done = 0;

#ifdef TCSUPPORT_CPU_EN7521
	if(PHY_GPON_CONFIG == gpPhyData->working_mode ) {
		/* restore sniffer mode */
		gponDevSetSniffMode(&gpGponPriv->gponCfg.sniffer_mode);
	}
#endif /* TCSUPPORT_CPU_EN7521 */

    if(should_ignore_phy_ready() ){
        XPON_DPRINT_MSG("[%s] ignore phy ready!\n", __FUNCTION__);
        return;
    }
      
    xpon_los_status = 1;
    do{
        
        if(PHY_UNKNOWN_CONFIG == gpPhyData->working_mode ) {
            gpPhyData->detect_status = XPON_PHY_SYNC_STATUS_SYNCED    ;
            gpPhyData->working_mode  = XPON_PHY_GET(PON_GET_PHY_MODE) ;
            continue;
        }

        gpPhyData->phy_link_status = PHY_LINK_STATUS_READY ;
        
        switch(gpPhyData->working_mode)
        {
            case PHY_GPON_CONFIG:
                gpon_detect_phy_ready();
                job_done = 1;
                break;

            case PHY_EPON_CONFIG:
				enable_cpu_us_traffic();
                eponDetectPhyReady();
                job_done = 1;
                break;
                
            default:
                dump_stack();
                printk("Illegal value of gpPhyData->working_mode: %d! %s:%d\n", 
                    gpPhyData->working_mode, __FUNCTION__, __LINE__);
                break;
        }
    } while(!job_done) ;
}

static void xpon_phy_start_rouge_handle(void)
{
    gpPonSysData->rogue_state = XPON_ROUGE_STATE_TRUE ;
}

static void xpon_phy_stop_rouge_handle(void)
{
    if( PHY_GPON_CONFIG == gpPhyData->working_mode   && \
        GPON_STATE_O7   == GPON_CURR_STATE ) {
        	gpGponPriv->emergencystate = 0;
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
    
    if((PHY_EPON_CONFIG == gpPhyData->working_mode)
     &&(opt == XPON_PHY_TX_POWER_OFF))
    {
		for(llidIndex = 0; llidIndex < EPON_LLID_MAX_NUM; llidIndex++){
			eponMpcpLocalDergstr(llidIndex);
		}	    
    }	
}


static int gponMacRxDis(unsigned char dis)
{
    int ret = ECNT_RETURN;
    REG_G_MBI_STOP gponMbiStop ;
    
    if((dis != ECNT_XPON_MAC_RX_DISABLE) && (dis != ECNT_XPON_MAC_RX_ENABLE)) {
       return ECNT_HOOK_ERROR;
    }
       
    /* STOP/START MBI Interface */
    gponMbiStop.Raw = IO_GREG(G_MBI_STOP) ;
    /* 0: not stop 
    *  1: stop
    */
    gponMbiStop.Bits.mbi_rx_stop = (dis == ECNT_XPON_MAC_RX_ENABLE) ? 0 : 1 ;
    IO_SREG(G_MBI_STOP, gponMbiStop.Raw) ;
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
    
static int xpon_phy_event_dispatch(PON_PHY_Event_data_t * pEvent)
{
    unsigned long flag;
    
    spin_lock_irqsave(&gpPonSysData->event_lock, flag) ;
    if(PON_WAN_START != gpPonSysData->sysStartup){
        if(TRUE == gpPhyData->calibrating  && PHY_EVENT_CALIBRATION_STOP == pEvent->id){
            gpPhyData->calibrating = FALSE;
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
            xpon_phy_los_illegal_handler(pEvent->src, pEvent->id);
            break;

        case PHY_EVENT_PHYRDY_INT:
            xpon_phy_ready_handler(pEvent->src);
            break;

        case PHY_EVENT_PHY_LOF_INT:
            break;
            
        case PHY_EVENT_TF_INT:
            break;
            
        case PHY_EVENT_TRANS_INT:
            break;
            
        case PHY_EVENT_TRANS_SD_FAIL_INT:
            break;
            
        case PHY_EVENT_I2CM_INT:
            break;

        case PHY_EVENT_START_ROGUE_MODE:
            xpon_phy_start_rouge_handle();
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
    
	if(gemIdx>=CONFIG_GPON_MAX_GEMPORT || !gpWanPriv->gpon.gemPort[gemIdx].info.valid) {
		return -1 ;
	}

	channelId = gpWanPriv->gpon.gemPort[gemIdx].info.channel;

	if(channelId == GPON_MULTICAST_CHANNEL)
	    return -1;

	return channelId;
	
}

static int xpon_mac_pub_module_hook_dispatch(struct ecnt_data  * in_data)
{
    xpon_mac_hook_data_t * data = (xpon_mac_hook_data_t *)in_data;
    struct XMCS_GponOnuInfo_S onuInfo ;
    struct XMCS_GponSnPasswd_S gponSnPasswd ;
    unsigned char rx_dis;
    int delay_cnt;
    
    switch (data->pub_info.type)
    {
        case XPON_SN_SET:
        {
            memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S)) ;
            gpon_cmd_proc(GPON_IOG_ONU_INFO, (ulong)&onuInfo) ;
            memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
            memcpy(gponSnPasswd.sn, data->pub_info.sn, 8) ;

            gpon_cmd_proc(GPON_IOS_SN_PASSWD, (ulong)&gponSnPasswd) ;
            break;
        }
        case XPON_MAC_MODE_GET:
        {
            if (PON_LINK_STATUS_GPON == gpPonSysData->sysLinkStatus ){
                *(data->pub_info.mode) = ECNT_XPON_MAC_MODE_GPON;
            }else if (PON_LINK_STATUS_EPON == gpPonSysData->sysLinkStatus ){
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
#if defined (TCSUPPORT_AUTOBENCH) && defined (TCSUPPORT_CPU_EN7527)
        case XPON_SLT:
        {
#if defined(TCSUPPORT_WAN_GPON)
            unsigned char sn[8]={0x4D, 0x54, 0x4B, 0x47, 0x00, 0x00, 0x00, 0x01}; /*MTKG00000001*/
            memset(&gponSnPasswd, 0, sizeof(struct XMCS_GponSnPasswd_S));
            gpon_cmd_proc(GPON_IOG_ONU_INFO, (ulong)&onuInfo);
            memcpy(gponSnPasswd.passwd, onuInfo.passwd, GPON_PASSWD_LENS);
            memcpy(gponSnPasswd.sn,sn, 8);
            gpon_cmd_proc(GPON_IOS_SN_PASSWD, (ulong)&gponSnPasswd);

	        printk("\nSLT Gpon testing\n");
	        /*PHY XPON SETTING*/
	        regWrite32(0xbfaf0138,0x10f);
	        xmcs_set_connection_start(XPON_DISABLE);
	        xmcs_set_link_detection(XMCS_IF_WAN_DETECT_MODE_GPON);
	        msleep(1);
	        xmcs_set_connection_start(XPON_ENABLE);

	        for(delay_cnt = 0; delay_cnt < 500; delay_cnt++)
	        {
	            if(gpon_slt_test != 0)
	                break;
	            msleep(50);
	            printk(".");
	            if((10 == delay_cnt)&&(1 == regRead32(G_ACTIVATION_ST)))
	                break;
	        }
	        
	        if(1 == gpon_slt_test){
	            printk("\nGpon testing success.\n");
	            return ECNT_RETURN;
	        }else{
                printk("\nGpon test fail or Epon mode.\n");
            }
#endif
#if defined(TCSUPPORT_WAN_EPON)
            printk("\nSLT Epon testing\n");
            /*PHY XPON SETTING*/
            regWrite32(0xbfaf0138,0x10f);
            xmcs_set_connection_start(XPON_DISABLE);
            xmcs_set_link_detection(XMCS_IF_WAN_DETECT_MODE_EPON);
            msleep(1);
            xmcs_set_connection_start(XPON_ENABLE);
            for(delay_cnt = 0 ; delay_cnt < 500; delay_cnt++)
            {
                if(epon_slt_test != 0)
                    break;
                msleep(50);
                printk(".");
            }
            
            if(1 == epon_slt_test){
                printk("\nEpon test success.\n");
                return ECNT_RETURN;
            }
            else{
                printk("\nEpon test fail.\n");
                return ECNT_CONTINUE;
            }
#endif
#ifdef TCSUPPORT_AUTOBENCH_EPON_LOOPBACK
            /*Don't support, need qdma module*/
            printk("epon loopback testing\n");
            xmcs_set_connection_start(XPON_DISABLE);
            file_write("/proc/qdma/self_test","init eponmac",strlen("init eponmac"));
            file_write("/proc/qdma/self_test","tc2_epon 30 100",strlen("tc2_epon 30 100"));
            
            if(pon_loopback_result != 0){
                printk("epon loopback test pass.\n");
            }
            else{
                printk("epon loopback test fail.\n");
            }
#endif
            break;
        }
#endif
        default:
            panic("XPON MAC Driver not ready to receive event from module with id:%d pub_type:%d", data->src_module,data->pub_info.type);
            return ECNT_HOOK_ERROR;
    }

    return ECNT_RETURN;
}

static int xpon_mac_hook_dispatch(struct ecnt_data  * in_data)
{
    xpon_mac_hook_data_t * data = (xpon_mac_hook_data_t *)in_data;
    int ret = ECNT_RETURN;
    
    switch (data->src_module)
    {
        case XPON_PHY_MODULE:
            xpon_phy_event_dispatch(data->pEvent);
            break;
        case XPON_PUB_MODULE:
            ret = xpon_mac_pub_module_hook_dispatch(data);
            break;
        default:
            panic("XPON MAC Driver not ready to receive event from module with id:%d,", data->src_module);
            return ECNT_HOOK_ERROR;
    }

    return ret;
}


#ifndef TCSUPPORT_XPON_LED
static void xpon_alarm_led_init(void)
{
	init_timer(&gpPonSysData->led_timer) ;/*used for lec flicker*/
	gpPonSysData->led_timer.data     = 500 ; /* 500 ms */
	gpPonSysData->led_timer.function = xpon_los_led_flicker ;
    gpPonSysData->flicker_stop       = 0 ;
}
#endif


static int max_dscv_gate_proc_init(void)
{
    if(NULL == xpon_proc_dir){
        return -1;
    }

	struct proc_dir_entry * proc_entry = create_proc_entry("max_dscv_gate",  0, xpon_proc_dir);
	if(NULL == proc_entry)
	{
		printk("\ncreate max_dscv_gate proc fail.");
		return -1;
	}

	proc_entry->read_proc = max_dscv_gate_read_proc;
	proc_entry->write_proc = max_dscv_gate_write_proc;

	return 0;
}

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
	unsigned long val = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	gpPonSysData->fe_reset_happened = simple_strtoul(val_string, NULL, 10);
	
	return count;
}


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

	temp_proc = create_proc_entry("debug", 0, gpon_proc_dir) ;
    if(NULL == temp_proc) {
        return -1;
    }
	temp_proc->read_proc = gpon_debug_read_proc ;
	temp_proc->write_proc = gpon_debug_write_proc ;

	temp_proc = create_proc_entry("stage_chk_cnt", 0, gpon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc = gpon_stage_change_chk_read_proc;
	temp_proc->write_proc = gpon_stage_change_chk_write_proc;

#ifdef TCSUPPORT_CPU_EN7521
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

	temp_proc = create_proc_entry("ponInfo", 0, xpon_proc_dir);
    if(NULL == temp_proc) {
        return -1;
    }
    temp_proc->read_proc = xpon_info_read_proc;

	temp_proc = create_proc_entry("omci_oam_monitor", 0, xpon_proc_dir);
    if(NULL == temp_proc) {
        return -1;
    }
    temp_proc->read_proc  = xpon_omai_oam_monitor_read_proc;
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

    epon_proc_dir = proc_mkdir("epon", NULL);
    if(NULL == epon_proc_dir){
        return -1;
    }
    
	temp_proc = create_proc_entry("max_dscv_gate",  0, epon_proc_dir);
	if(NULL == temp_proc){
		return -1;
	}
	temp_proc->read_proc = max_dscv_gate_read_proc;
	temp_proc->write_proc = max_dscv_gate_write_proc;

    
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

    XPON_STOP_TIMER(gpPhyData->delay_start_detect_timer)   ;
    XPON_STOP_TIMER(gpPhyData->check_sync_timer)           ;
    
    gpPonSysData->sysStartup = PON_WAN_STOP                ;
    XPON_STOP_TIMER(gpPhyData->trans_status_refresh_timer) ;
    gpPonSysData->sysMACStartup = PON_MAC_STOP             ;
    XPON_STOP_TIMER(gpPhyData->traffic_status_refresh_timer) ;
#ifndef TCSUPPORT_CPU_EN7521
    XPON_STOP_TIMER(gpPonSysData->gsw_p6_rate_timer);
#endif

	xpon_daemon_quit();
	

#ifdef TCSUPPORT_WAN_GPON
    gpon_deinit() ;
#endif /* TCSUPPORT_WAN_GPON */

#ifdef TCSUPPORT_WAN_EPON
    eponExit(); 
#endif /* TCSUPPORT_WAN_EPON */

    pon_mci_destroy() ;
    
    pwan_destroy() ;
    free_irq(DYINGGASP_INT, NULL);

    stop_omci_oam_monitor();
	
    if(gpPonSysData != NULL) {
        kfree(gpPonSysData) ;
        gpPonSysData = NULL ;
    }
    
    if(gpon_proc_dir){
#ifdef TCSUPPORT_CPU_EN7521
        remove_proc_entry("power_management", gpon_proc_dir);
#endif
        remove_proc_entry("debug", gpon_proc_dir);
	remove_proc_entry("stage_chk_cnt", gpon_proc_dir);
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
    }
    remove_proc_entry("epon",NULL);


    if(epon_stat_prev)
        kfree(epon_stat_prev);

    ecnt_unregister_hook(&xpondrv_hook_dispatch_ops);

}

/*****************************************************************************
******************************************************************************/

int xpon_check_emergence_state(void)
{
	return FALSE;
}



/*****************************************************************************
******************************************************************************/
int xpondrv_init(void)
{
	printk("xPON driver initialization\n") ;

	/* initial the global data memory */
	gpPonSysData = (PON_SysData_T *)kmalloc(sizeof(PON_SysData_T), GFP_KERNEL) ;
	if(gpPonSysData == NULL) {
		printk("Alloc data struct memory failed\n") ;
		goto ret ;
	} else {
		memset(gpPonSysData, 0, sizeof(PON_SysData_T));
		printk("Alloc data struct memory successful, %d\n", sizeof(PON_SysData_T)) ;
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

    if (ECNT_REGISTER_SUCCESS != ecnt_register_hook(&xpondrv_hook_dispatch_ops) ){
        panic("Register hook function failed! %s:%d", __FUNCTION__, __LINE__);
    }
    
#ifndef TCSUPPORT_XPON_LED
    xpon_alarm_led_init();
#endif

	/*LED flicker*/	
	change_alarm_led_status(ALARM_LED_FLICKER);

    XPON_START_TIMER(gpPhyData->trans_status_refresh_timer) ;
    gpPonSysData->sysMACStartup = PON_MAC_START;
    XPON_START_TIMER(gpPhyData->traffic_status_refresh_timer) ;
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

#ifndef TCSUPPORT_CPU_EN7521
void enable_cpu_us_omci_oam_traffic(void)
{
    gpPonSysData->isUpOmciOamTraffic = XPON_ENABLE;
}

void enable_cpu_us_data_traffic(void)
{
    gpPonSysData->isUpDataTraffic = XPON_ENABLE;
}

void disable_cpu_us_data_traffic(void)
{
    gpPonSysData->isUpDataTraffic = XPON_DISABLE;
}

void xpon_enable_cpu_protection(void)
{
    attack_protect_set(1, 1);
    gpPonSysData->gswRateLimitFlag = TRUE;
    /* set p6 egress traffic limit (cir=10mbps,cbs=4kbyte) */
    IO_SREG(0xbfb59640, 0x1408488) ;
}

void xpon_disable_cpu_protection(void)
{
    attack_protect_set(0, 1) ;
    gpPonSysData->gswRateLimitFlag = FALSE;
    /*close p6 egress traffic limit */
	regWrite32(0xbfb59640,0x0) ;
}
#endif /* TCSUPPORT_CPU_EN7521 */

module_init(xpondrv_init)
module_exit(xpondrv_cleanup)
MODULE_LICENSE("GPL");

