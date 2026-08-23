/*
* File Name: gpon_init.c
* Description: Initialization for GPON
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/version.h>

#include <ecnt_hook/ecnt_hook_net.h>
#include "common/xpon_global.h"
#include "epon/epon_act.h"
#include "epon/epon_mpcp.h"
#include "common/xpon_api.h"




#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
extern long eponMacIoctl(struct file *file, unsigned int cmd,
	                unsigned long arg);
#else
extern int eponMacIoctl (struct inode *inode, struct file *filp,
                  unsigned int cmd, unsigned long arg);
#endif
extern void eponPhyTxPwrUp(TIMER_FUN_PAAM lparam);
extern int pwan_net_start_xmit(struct sk_buff *skb, struct net_device *dev);
extern int epon_proc_init(void);
extern int epon_get_onu_mac_addr(__u8 *macAddr);
extern int eponCmdInit(void);
extern int epon_set_qdma_qos(void);
extern void epon_debug_print_exit(void);
extern void epon_proc_exit(void);
extern void eponCmdExit(void);


#define PADD_HEADER_LEN     			(2)
#define DYING_GAPS_PKT_LEN  			(62)

PEPON_MAC_REGS 		g_EPON_MAC_BASE = NULL;
EPON_GLOBAL_DATA_T	g_epon_global_data  = {0};
pEPON_GLOBAL_DATA_T gp_epon_global_data = &g_epon_global_data;

extern __u32 llids_cfg_map[8];

static char dying_gasp_oam[]=
{
	0x00,0x00,0x01,0x80,0xC2,0x00,0x00,0x02,0x00,0x25,
	0x12,0xBB,0x25,0x0E,0x88,0x09,0x03,0x00,0x52,0x00,
	0x01,0x10,0x01,0x00,0x00,0x00,0x0F,0x05,0xEE,0x00,
	0x13,0x25,0x00,0x22,0x01,0x00,0x02,0x10,0x01,0x00,
	0x00,0x00,0x0F,0x05,0xEE,0x00,0x13,0x25,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

struct timer_list eponTxPwrDwnTmr;
struct timer_list eponTxRate;

uint8 *GetMacAddr(void);
/*______________________________________________________________________________
**	function name
**		xmit_dying_gasp_oam
**	description:
**		trans dying gasp packet to QDMA
**	parameters:
**		None
**	global:
**		dying_gasp_oam
**	return:
**		None
**	call:
**		dev_get_by_name
**		pwan_net_start_xmit
**	revision:
**		v1.0
**____________________________________________________________________________*/

void xmit_dying_gasp_oam(void)
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
	CALL_USER_HOOK_PHY_EVENT_NOTIFY(XMCS_EVENT_TYPE_EPON, XMCS_EVENT_EPON_DYING_GASP);
	
end:
	return;
}
/*______________________________________________________________________________
**	function name
**		epon_get_mac_reg_address
**	description:
**		get mac address
**	parameters:
**		None
**	global:
**		g_EPON_MAC_BASE
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_get_mac_reg_address(void)
{
	g_EPON_MAC_BASE = (PEPON_MAC_REGS)(ioremap_nocache(CONFIG_EPON_BASE_ADDR, CONFIG_EPON_REG_RANGE)) ; 
	if(!g_EPON_MAC_BASE) {
		printk("ERR: ioremap the EPON base address failed:addr=0x%x,size=0x%x\n",CONFIG_EPON_BASE_ADDR,(uint32_t)CONFIG_EPON_REG_RANGE) ;
		return -ENOMEM ;
	}
    return EPON_SUCCESS;
}

/*______________________________________________________________________________
**	function name
**		epon_init_global_data
**	description:
**		init epon global data
**	parameters:
**		epon_data_p
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/
int epon_init_global_data(void)
{
    /*epon parameter init*/
    gp_epon_global_data->debug_level            = MSG_LEVEL_ERROR;
    gp_epon_global_data->hold_over_time         = EPON_HOLD_OVER_TME_DEFAULT;
    gp_epon_global_data->hold_over_enable       = FALSE;
    gp_epon_global_data->typeb_holdOn_enable    = FALSE;
    gp_epon_global_data->laser_off              = EPON_DEFAULT_LASER_OFF;
    gp_epon_global_data->laser_on               = EPON_DEFAULT_LASER_ON;
    gp_epon_global_data->txPower_flag           = TRUE;
    gp_epon_global_data->llid_bit_mask          = EPON_LLID_BIT_MASK_DEFAULT;
    gp_epon_global_data->mpcp_send_check_type   = EPON_MPCP_SEND_CHECK_INTERRUPT;
    gp_epon_global_data->static_report_enable   = FALSE;
    gp_epon_global_data->silent_time_config     = EPON_SLIENT_DEFAULT_TIME;


	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_INIT_DSCVRY_STS,NULL,NULL);

    /* llid info clear */
    memset(&gp_epon_global_data->llid_entry,0,(sizeof(EPON_LLID_INFO_T)*EPON_LLID_MAX_NUM));
    
    /* epon timer creat*/
    EPON_CREATE_TIMER(&gp_epon_global_data->silent_timer,epon_silent_timer_expires,EPON_SILENT_INTERVAL);
    EPON_CREATE_TIMER(&gp_epon_global_data->typeb_timer,epon_typeb_timer_expires,EPON_HOLD_OVER_TME_DEFAULT);
    EPON_CREATE_TIMER(&gp_epon_global_data->oamCnt_timer,epon_oamCnt_check_timer_expires,EPON_CHECK_OAM_CNT_TME_DEFAULT);
    EPON_INIT_TASK(&gp_epon_global_data->epon_start_task,epon_start,TASK_INPUT_PARA_DEFAULT);
    
    return  EPON_SUCCESS;
}

struct file_operations eponMacFops = {
	.owner =		THIS_MODULE,
	.write =		NULL,
	.read =			NULL,
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
	.unlocked_ioctl =	eponMacIoctl,	
#ifdef TCSUPPORT_CPU_ARMV8_64
	.compat_ioctl	= eponMacIoctl,
#endif
#else	
	.ioctl =		eponMacIoctl,
#endif	
	.open =			NULL,
	.release =		NULL,
};

//extern spinlock_t epon_reg_lock;

#ifdef TCSUPPORT_EPON_POWERSAVING
/*______________________________________________________________________________
**	function name
**		epon_early_wakeup_expires
**	description:
**		early wake up timeout
**	parameters:
**		arg
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_early_wakeup_expires(TIMER_FUN_PAAM arg)
{
    e_pwr_sv_cfg_SET_onu_wakeup(e_pwr_sv_cfg, 1);
	DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ONU manual wakeup!\n");
}
#endif

/*______________________________________________________________________________
**	function name
**		epon_init_private_data
**	description:
**		init private data
**	parameters:
**		epon_priv_p
**	global:
**		None
**	return:
**		0:success
**          -1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

static int epon_init_private_data(EPON_GlbPriv_T *epon_priv_p)
{
	int ret = 0 ;
#ifdef TCSUPPORT_EPON_POWERSAVING
	epon_priv_p->eponCfg.earlyWakeupTimer = 50;
	epon_priv_p->eponCfg.earlyWakeupFlag = 0;
	epon_priv_p->eponCfg.earlyWakeupCount = 0;
#endif

	return ret;
}

/*______________________________________________________________________________
**	function name
**		epon_init
**	description:
**		init epon driver
**	parameters:
**		None
**	global:
**		gp_epon_global_data
**		eponMacFops
**		gpEponPriv
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_debug_print_init
**		epon_get_mac_reg_address
**		epon_init_global_data
**		init_msg_route_table
**		epon_proc_init
**		epon_isr_handler_init
**		epon_mac_table_init
**		epon_get_onu_mac_addr
**		eponCmdInit
**		epon_init_private_data
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_init(void)
{
	int ret = 0;
    if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)
                &&(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
    {
            printk(" No XEPON init, pon mode:%d \n",gpPonSysData->sysPonMode) ;            
            return ret;
    }

	epon_debug_print_init();

    ret = UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_MAC_BASE_REG_ADDRESS,NULL,NULL);
	if(EPON_SUCCESS != ret)
    {
    	printk("ERROR epon init get_mac_reg_address fail\n");
        return ret;
    }

    ret = epon_init_global_data();
	if(EPON_SUCCESS != ret)
    {
    	printk("ERROR epon init init_global_data fail\n");
        return ret;
    }
    
	ret = epon_proc_init();
	if(EPON_SUCCESS != ret)
    {
    	printk("ERROR epon init proc fail\n");
        return ret;
    }
    
	ret = register_chrdev(EPON_MAC_MAJOR, "epon_mac", &eponMacFops);
	if (ret < 0)
    {    
    	printk("epon create ioctrl fail\n");
		return ret;
    }

    
    epon_isr_handler_init();
    	
	epon_get_onu_mac_addr(gp_epon_global_data->onu_mac_addr);
	/* cmd init */
	eponCmdInit();
	
	ret = epon_init_private_data(gpEponPriv);
	if (ret < 0) {
		printk("EPON private data initialization failed\n");
		return ret;
	}
#ifdef TCSUPPORT_EPON_POWERSAVING
	EPON_CREATE_TIMER(&gpEponPriv->early_wakeup_timer,epon_early_wakeup_expires,gpEponPriv->eponCfg.earlyWakeupTimer);
#endif

    /*set peak mode and margin for EPON*/
    epon_set_qdma_qos();

    memset(&eponTxPwrDwnTmr, 0, sizeof(struct timer_list));
	/*TX Power Control Timer Init*/
	EPON_CREATE_TIMER(&eponTxPwrDwnTmr,eponPhyTxPwrUp,0);
    	
    memset(&eponTxRate, 0, sizeof(struct timer_list));	
    EPON_CREATE_TIMER(&eponTxRate,epon_Tx_Rate_expires,4000);
    return ret;
}

void epon_int_disable(void)
{
	__u32 raw = 0;
	WRITE_REG_WORD(e_int_en , raw);
}
/*______________________________________________________________________________
**	function name
**		epon_deinit
**	description:
**		deinit epon driver
**	parameters:
**		None
**	global:
**		gp_epon_global_data
**		g_EPON_MAC_BASE
**		gpEponPriv
**	return:
**		0:success
**          -1:fail
**	call:
**		epon_debug_print_exit
**		epon_proc_exit
**		epon_Mac_Table_Exit
**		eponCmdExit
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_deinit(void)
{
    int ret = 0;
    if((gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_1G_EPON)&&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_10G_10G_EPON)&&\
            (gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_1G_1G_EPON)&&\
			(gpPonSysData->sysPonMode != XMCS_IF_WAN_DETECT_MODE_TURBO_EPON))
    {
        printk(" No XEPON deinit, pon mode:%d \n",gpPonSysData->sysPonMode) ; 
        return ret;
    }
	epon_int_disable();
	epon_debug_print_exit();
    
	if(g_EPON_MAC_BASE) {
	    iounmap(g_EPON_MAC_BASE) ;
	    g_EPON_MAC_BASE = NULL ;
    }

    tasklet_kill(&gp_epon_global_data->epon_start_task);
    EPON_STOP_TIMER(gp_epon_global_data->silent_timer) ;
    EPON_STOP_TIMER(gp_epon_global_data->typeb_timer) ;

    epon_proc_exit();
	
	/* cmd exit */
	eponCmdExit();
	unregister_chrdev(EPON_MAC_MAJOR, "epon_mac");
    
#ifdef TCSUPPORT_EPON_POWERSAVING
	EPON_STOP_TIMER(gpEponPriv->early_wakeup_timer) ;
#endif
	return 0;
}


