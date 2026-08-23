/*
* File Name: phy_init.c
* Description: Initialization for xPON PHY
*
******************************************************************/
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>

#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy_init.h"
#include "phy_tx.h"
#include "phy_reg.h"
#include "a60972.h"
#include "a60972_reg.h"


#ifdef TCSUPPORT_CPU_EN7581
#include "en7581.h"
#include "en7581_pma.h"
#include "en7581_reg.h"

#include "en7583.h"
//#include "en7583_pma.h"
#include "en7583_reg.h"

#else
#include "en7580.h"
#include "en7580_pma.h"
#include "en7580_reg.h"
#endif

PHY_GlbPriv_T *gpPhyPriv = NULL;

struct workqueue_struct * i2c_access_queue = NULL;


#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON)
	int los_status = 0;
#endif/*TCSUPPORT_COMPILE*/

#if ASIC_SERDES || A60972_SERDES
#ifndef TCSUPPORT_CPU_AN7583
	uint pma_low_rate_opt_val = XPON_PMA_OPT_DEFAULT_VALUE_LOW_RATE;
	uint pma_hi_rate_opt_val = XPON_PMA_OPT_DEFAULT_VALUE_HIGH_RATE;
#endif
#endif

/*****************************************************************************
//function :
//		gpio_BOSA_Tx_power_on
//
//description : 
//		this function is used to turn on BOSA Tx 3V3.
//		BOSA_Tx power switch is connected to a GPIO output pin in MT7520, the setting in this function need to 
//		fit customer's allocation of 3V3_BOSA_Tx.
//		ex: On reference 7570 BOB board, 3V3_BOSA_Tx matches GPIO2, so this function pulls GPIO2 low.
//
//input :	
//		N/A
//
//output :
//		N/A
//
//date :
//		by YMC 20150731
//
******************************************************************************/
void gpio_BOSA_Tx_power_on(void)
{
#ifdef TCSUPPORT_KERNEL_API
	ledTurnOffRT(LED_PHY_VCC_DISABLE);
#else
	ledTurnOff(LED_PHY_VCC_DISABLE);
#endif
}


/*****************************************************************************
//function :
//		xPON_Phy_Reset
//description : 
//		this function is used to reset xPON PHY
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
//static spinlock_t  pon_phy_lock;
int xPON_Phy_Reset(void){

	return ponPhyFunc[PHY_RESET_FUNC](gpPhyPriv);

}
	
/*****************************************************************************
//function :
//		phy_int_recover_expires
//description : 
//		this function is used to recover interrupt
//input :	
//		interrupt type          
//		TRANS_LOS_INT				0x01
//		PHY_LOF_INT					0x02
//		TRANS_TF_INT				0x04
//		TRANS_INT					0x08
//		TRANS_SD_FAIL_INT			0x10
//		PHYRDY_INT					0x20
//output :
//		N/A
******************************************************************************/
static void phy_int_recover_expires(ulong data)
{
	phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
}
	


/*****************************************************************************
//function :
//		phy_dev_init
//description : 
//		this function is used to set 7525 to XPON mode
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
//#define TCSUPPORT_PON_ROGUE_ONU		//YMC_20150206
int phy_dev_init(void)
{
	return ponPhyFunc[PHY_DEVICE_INIT_FUNC](NULL);
}

/*****************************************************************************
//function :
//		phy_los_handler
//description : 
//		this function is used to process phy los interrupt
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
void phy_los_handler(void)
{

}

void phy_ready_handler(void)
{

}

inline void handle_hw_irq_event(PHY_Event_Type_t id)
{
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
    phy_event.id = id;
    phy_event_handler(&phy_event);

}

/******************************************************************************
//function :
//		phy_register_cb_handler
//description : 
//		this function is used to register MAC function
//input :	
//		type		: used to select function
//		*pCbFun 	: MAC function
//output :
//		0 	: success
//		-1	: no such type
******************************************************************************/
int phy_register_cb_handler(PHY_CbType_t type, void *pCbFun)
{
	if(!pCbFun) {
		return -1 ;	
	}
	
	switch(type) {
#ifdef TCSUPPORT_XPON_HAL_API_EXT		
		case PHY_CALLBACK_XPON_STATE_NOTIFY:
			gpPhyPriv->phyXponStateNotifyHandler = pCbFun ;
			break ;
#endif /* TCSUPPORT_XPON_HAL_API_EXT */	
		default:
			return -1 ;	
	}
	return 0 ;
}

/******************************************************************************
******************************************************************************/
int phy_unregister_cb_handler(PHY_CbType_t type)
{
	switch(type) {
#ifdef TCSUPPORT_XPON_HAL_API_EXT		
		case PHY_CALLBACK_XPON_STATE_NOTIFY:
			gpPhyPriv->phyXponStateNotifyHandler = NULL ;
			break ;
#endif /* TCSUPPORT_XPON_HAL_API_EXT */
		default:
			return -EFAULT ;	
	}
	return 0 ;
}


struct ecnt_hook_ops phy_api_dispatch_hook_ops = {
    .name = "pon_phy_api_dispatch",
    .hookfn = pon_phy_api_dispatch,
    .is_execute = 1,
    .maintype = ECNT_XPON_PHY,
    .subtype = ECNT_XPON_PHY_API,
    .priority   = 1,
};

int pon_phy_get_wan_sel(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{
	uint ret = 0;

#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
		ret=an7583_get_wan_sel();
	#else
	ret=en7581_get_wan_sel();
	#endif
#else
	ret=en7580_get_wan_sel();
#endif

	return ret;
}


int pon_phy_func_init(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{
	uint ret = 0;

#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
		ret=an7583_func_init();
	#else
	ret=en7581_func_init();
	#endif
#else
	ret=en7580_func_init();
#endif

	return ret;

}

int pon_phy_clear_int(void)
{
	uint ret = 0;
		
#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
		ret=an7583_phy_clear_int();
	#else
	ret=en7581_phy_clear_int();
	#endif
#else
	ret=en7580_phy_clear_int();
#endif

	return ret;
}
int phy_pma_reset(void)
{
	
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	
	ulong flags;
#if (A60972_SERDES)||(ASIC_SERDES)

	phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); 	//ang_20191101, turn off tx before reset.   //tx_dis_reset_protection
	if(spin_is_locked(&gpPhyPriv->pma_reset_lock)==0){
	spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
	ponPhyFunc[PHY_PMA_RESET_FUNC](NULL);
	spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);
	}
	else{
		ponPhyFunc[PHY_PMA_RESET_FUNC](NULL);
	}		
	phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW);		//david 20180418  //tx_dis_reset_protection

#endif

#if XILINX_SERDES

		if(spin_is_locked(&gpPhyPriv->pma_reset_lock)==0){
			spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
			//reset xilinx serdes, ang_20211117 //julia_20220511
			read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
			write_data = read_data | XILINX_SERDES_RST_ON;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
			mdelay(10);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
			mdelay(10);
			spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);
			}
			else{
				//reset xilinx serdes, ang_20211117 //julia_20220511
				read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
				write_data = read_data | XILINX_SERDES_RST_ON;
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
				mdelay(10);
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
				mdelay(10);
			}	

#endif

	return PHY_SUCCESS;
}

int phy_pma_reset_with_lock(void)
{
	
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	
	ulong flags;
#if (A60972_SERDES)||(ASIC_SERDES)
	phy_trans_power_switch(PHY_DISABLE); 	//ang_20191101, turn off tx before reset. 
	spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
	ponPhyFunc[PHY_PMA_RESET_FUNC](NULL);
	spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);
	phy_trans_power_switch(PHY_ENABLE);		//david 20180418

#endif

#if XILINX_SERDES
	spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
	//reset xilinx serdes, ang_20211117 //julia_20220511
	read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
	write_data = read_data | XILINX_SERDES_RST_ON;
	IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
	mdelay(10);
	IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
	mdelay(10);
	spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);
#endif

	return PHY_SUCCESS;
}

int phy_pcs_fpgaif_reset(void)	//reset pcs and FPGA interface
{
	PHY_INTERNAL_SET(PHY_SET_PCS_FPGAIF_RESET);
	return PHY_SUCCESS;
}

int phy_pcs_reset(void)	//only reset pcs
{
	PHY_INTERNAL_SET(PHY_SET_PCS_RESET);
	return PHY_SUCCESS;
}

#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm
/*****************************************************************************
//function :
//		phy_isr
//description : 
//		this function is used to process phy interrupt
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
static irqreturn_t phy_isr_request(int irq, void *dev_id) //julia_arm
{
	phy_isr();
	return IRQ_HANDLED;

}
#endif

/*****************************************************************************
//function :
//		pon_phy_init
//description : 
//		this function is the initial setting for XPON phy when insmod xpon_phy.ko
//input :	
//		N/A
//output :
//		0 	: initial success
//		-1	: initial fail
******************************************************************************/
int pon_phy_init(void)
{
	int ret=0;
	uint read_data = 0;
	uint write_data = 0;
	unchar ptr[4];
	uint phyIntStatus;	 //david 20170124

	//__________________ At very beginning we know onthing about : chip ID , PON mode __________________
	
printk("\r\n____________________________________________\r\n");
printk("* PHY_VERSION:  X.%-3d       Date: %8d *\n",PHY_VERSION,PHY_DATE_VERSION);
printk("Note : %s\n",PHY_VERSION_NOTE);
printk("FW Compile Date: %s, Time: %s\n",__DATE__,__TIME__);
//printk("FE_GENTIME : FPGA Date : %x : Time : %x\n",(IO_GPHYREG(FE_GENTIME)>>8),(IO_GPHYREG(FE_GENTIME)&0x000000FF));
printk("LINUX_VERSION_CODE : %x\n",LINUX_VERSION_CODE);

#if A60972_SERDES
	printk("A60972_SERDES\r\n");
#endif

#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
		printk("AN7583_SERDES\r\n");
	#else
		printk("EN7581_SERDES\r\n");
	#endif
#else
		printk("EN7580_SERDES\r\n");
	#endif


#ifdef __BIG_ENDIAN
	printk("__BIG_ENDIAN\r\n");
#else
	printk("__LITTLE_ENDIAN\r\n");
#endif

	if (NULL == gpPhyPriv)
	{
		gpPhyPriv = (PHY_GlbPriv_T *)kmalloc(sizeof(PHY_GlbPriv_T), GFP_KERNEL) ;
		if( NULL == gpPhyPriv) {
			printk("Alloc data struct memory failed\n") ;
			return ret ;
		}
	}
	memset(gpPhyPriv, 0, sizeof(PHY_GlbPriv_T)) ;

#ifdef TCSUPPORT_CPU_ARMV8
	gpPhyPriv->scu_hir_np_sys_hw_id=(GET_HIR())&0x0000FFFF;
	//printk("SCU_HIR = 0x%x\n",gpPhyPriv->scu_hir_np_sys_hw_id);
	switch(gpPhyPriv->scu_hir_np_sys_hw_id)
	{
		case 0x5:
		case 0x7:
			PON_PHY_PRINT(PHY_MSG_ERR,"ERROR! MT752X HW ID \n");
			break;

		case 0x8:
		case 0xa:
			PON_PHY_PRINT(PHY_MSG_ERR,"EN7580 HW ID \n");
			break;		
			
		case 0xe:
			PON_PHY_PRINT(PHY_MSG_ERR,"EN7581 HW ID \n");
			break;
			
		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EPON UNKNOWN scu_hir_np_sys_hw_id = 0x%x\n",__FUNCTION__,__LINE__,gpPhyPriv->scu_hir_np_sys_hw_id);
	}

	read_data=GET_WAN_CONF();
	gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
	printk("SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);

	//IO_SPHYREG(RG_GPIO_XPON_MODE,1);   //GPIO setting meant to be set by Top Layer
	//SET_IOMUX_CTRL_3(1);	
	read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE_7523	
	write_data =(read_data | 0x01); //bit [0]: 1	
	SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE_7523
#else
	gpPhyPriv->scu_hir_np_sys_hw_id=(IO_GPHYREG(SCU_HIR)>>16)&0x0000FFFF;
	printk("SCU_HIR = 0x%x\n",gpPhyPriv->scu_hir_np_sys_hw_id);

	read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
	gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
	printk("SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);

	IO_SPHYREG(RG_GPIO_XPON_MODE,1);   //GPIO setting meant to be set by Top Layer
#endif

	
	//	_________ from here, we know chip ID = gpPhyPriv->scu_hw_id_product_id _________________
	
	/* these two line must be put before registering isr handler */
	gpPhyPriv->rx_fec_setting = PHY_DEFAULT;
	gpPhyPriv->is_irq_requested = FALSE;
	gpPhyPriv->is_phy_start = FALSE;
	gpPhyPriv->phy_init_done = FALSE; 
#ifdef TCSUPPORT_MT7570
	gpPhyPriv->en7571_init_done = FALSE; 
#endif
	gpPhyPriv->phyCfg.flags.rogue =0;
	gpPhyPriv->phyCfg.flags.mode = PHY_UNKNOWN_CONFIG ;
	gpPhyPriv->trans_index = PHY_TRANS_NOT_FOUND_IN_IOT_LIST;
	gpPhyPriv->mac_set_phy_init_step = 0;
	gpPhyPriv->i2c_u2_clk_div = I2C_U2_CLK_DIV ;
	gpPhyPriv->i2c_addr_num = 1;
	gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
	gpPhyPriv->trans_tx_enable = PHY_DISABLE;
	gpPhyPriv->trans_tx_status = PHY_DISABLE;  //tx_dis_reset_protection
	gpPhyPriv->phy_unexpected_isr_flag=FALSE;
	gpPhyPriv->first_plugin_flag = TRUE; //ang_20180926
	gpPhyPriv->pma_init_done = FALSE;	//ang_20190108
	gpPhyPriv->xe_syncloss_cnt = 0;
	gpPhyPriv->rogue_onu_det_en = 0;
	gpPhyPriv->rogue_onu_ben_det_en = 0; //julia_20240510
	gpPhyPriv->is_rogue_onu =0;
	gpPhyPriv->rogueonu_happen = 0; //julia_20230614
	gpPhyPriv->rogue_state =0;
	gpPhyPriv->temprature =0; //julia_20230614
	gpPhyPriv->trans_msg_print_cnt = 95;
    gpPhyPriv->phy_deinit_flag = FALSE;
gpPhyPriv->pon_stop_flag =FALSE;
#if defined(TCSUPPORT_COMBO_PON)	
	gpPhyPriv->phy_mode_task_wait= NULL;
	gpPhyPriv->is_phy_mode_detected = FALSE;
	gpPhyPriv->combo_pon_enable = (get_onutype() & 0x04) >> 2 ;
	printk("combo pon flag = %d\n",gpPhyPriv->combo_pon_enable);
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
	gpPhyPriv->i2c1_used = FALSE;
#endif

#endif

#if EN7580_TEST_CODE || EN7581_TEST_CODE	
	gpPhyPriv->debugLevel = (PHY_MSG_ERR|PHY_MSG_TRANS); //by ang_20180621  //|PHY_MSG_INT
#else
    gpPhyPriv->debugLevel = (PHY_MSG_ERR);
#endif
	printk("debugLevel=0x%x\n",gpPhyPriv->debugLevel);

#if 0
	read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
	printk("Original HW SCU_WAN_CONF_REG_WAN_SEL=%d \n",read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS);
	IO_SPHYREG(SCU_WAN_CONF_REG,((read_data&SCU_WAN_CONF_REG_WAN_SEL_MASK)|PON_PHY_FW_WAN_SEL));
	printk("#PHY FW# Set SCU_WAN_CONF_REG_WAN_SEL=%d\n",PON_PHY_FW_WAN_SEL);
#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
	gpPhyPriv->lddla_auto_mode = FALSE;
#endif
  	gpPhyPriv->phy_trans_task_wait = NULL;
	gpPhyPriv->phy_ngpon2_task_wait = NULL;

	spin_lock_init(&gpPhyPriv->event_handle_lock);
	spin_lock_init(&gpPhyPriv->pma_reset_lock);

	//init_timer(&gpPhyPriv->event_poll_timer);
	//gpPhyPriv->event_poll_timer.data = 1500;
	//gpPhyPriv->event_poll_timer.function = phy_event_poll; 
	
    gpPhyPriv->event_poll_timer_value = 1500; // julia_20201017 for kernel 5.4
	PHY_CREATE_TIMER(&gpPhyPriv->event_poll_timer,phy_event_poll,gpPhyPriv->event_poll_timer_value);//julia_20201017 for kernel 5.4


	if((ret = phy_debug_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "phy debug init failed.\n") ;
		return ret ;
	}

#if A60972_SERDES
	gpPhyPriv->trans_tx_ben_level=LOW_ACTIVE;
	if((ret = xpon_pma_debug_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "A60972 debug init failed.\n") ;
		return ret ;
	}
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON)
		printk("CT do los_status_init\n");
		if ((ret = los_status_init()) != 0) {
			PON_PHY_PRINT(PHY_MSG_ERR, "phy los status initialization failed.\n") ;
			return ret;
		}
#endif/*TCSUPPORT_COMPILE*/

	if (ECNT_REGISTER_SUCCESS != ecnt_register_hook(&phy_api_dispatch_hook_ops) ){
		panic("Register hook function failed! [%s:%d]", __FUNCTION__,__LINE__);
	}
#if defined(TCSUPPORT_COMBO_PON)
	if(gpPhyPriv->combo_pon_enable ==TRUE)
	{
		#ifdef TCSUPPORT_CPU_AN7583
		SET_IOMUX_CTRL_3(0x08000000); //bit [27]=1 GPIO_PON2_MODE	//7583 COMBO_PON
		#endif
				if(phy_mode_detection()!=0)
				{
					gpPhyPriv->phy_mode_task_wait= kthread_run(phy_mode_task_wait, NULL, "phy_mode_task_wait");
					PON_PHY_PRINT(PHY_MSG_ERR,"\e[1;33mphy_mode_detection failed\e[0m\n");
				}
	}
	else
	{
		combo_pon_phy_init();
	}
#else
	if((ret = pon_phy_get_wan_sel()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"pon_phy_get_wan_sel fail.\n") ;
		return ret ;
	}

	if((ret = pon_phy_func_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"pon_phy_func_init fail.\n") ;
		return ret ;
	}	

	rcu_assign_pointer (set_pon_phy_mode_config,phy_mode_config);
	rcu_assign_pointer (set_pon_phy_start,xpon_phy_start);
	rcu_assign_pointer (set_pon_phy_stop,xpon_phy_stop);
	rcu_assign_pointer (get_pon_phy_trans_status,pon_phy_get_trans_status);

	//set_pma_opt(XPON_PMA_OPT_DEFAULT_VALUE_HIGH_RATE);	// default use pma_opt value for ECNT REF board 	by david 20180920

#if EN7580_FLASH || EN7581_FLASH
// Get FLASH FIR&BOB
	if((ret = get_flash_matrix()) != 0){			
		set_flash_register_default();	//Reset flash_matrix to default
		PON_PHY_PRINT(PHY_MSG_ERR,"Get FLASH fail: FLASH matrix reset to default!\n");	
		return ret ;
	}
#endif

#if 0  //do scu_reset and backup reg default value here for recovery later
		xPON_Phy_Reset();
		pma_reg_dump();
		phy_dump();
#endif

#endif
	printk("____________________________________________\r\n\r\n");

	return ret ;
}

#if defined(TCSUPPORT_COMBO_PON)
int current_phy_mode = PHY_UNKNOWN_CONFIG;
int phy_mode_detection(void)  //1=low priority, 2=high priority
{
	int switch_to = PHY_UNKNOWN_CONFIG;
	uint write_data = 0;
	uint read_data = 0;
	int pon_phy_los_1 = 1;  //GPON
	int pon_phy_los_2 = 1;  //XGPON
	
	read_data = GET_WAN_CONF(); // 1fb00070
	pon_phy_los_2 = gpioctlGetVal("xgspon_Los");
	pon_phy_los_1 = gpioctlGetVal("gpon_Los");
	PON_PHY_PRINT(PHY_MSG_ERR,"los_1 = %d, los_2 = %d \r\n",pon_phy_los_1,pon_phy_los_2);	
	if(current_phy_mode ==PHY_UNKNOWN_CONFIG)
	{
    if((pon_phy_los_2==0)&&(pon_phy_los_1==0))
    {
		if(gpPhyPriv->wan_sel ==SCU_WAN_CONF_REG_WAN_SEL_GPON)
		{
			switch_to=PHY_GPON_CONFIG;
			gpPhyPriv->is_phy_mode_detected=TRUE;
			gpioctlSetOutput("pon_S_Sel",0);
			write_data = read_data & 0xffffff00;
			SET_WAN_CONF(write_data);
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
			//gpPhyPriv->i2c1_used = TRUE;
#endif
			combo_pon_phy_init();
			REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_COMBO_GPON);
		}
		else
		{
			gpPhyPriv->wan_sel =SCU_WAN_CONF_REG_WAN_SEL_XGSPON;
			switch_to=PHY_XGSPON_CONFIG;
			gpPhyPriv->is_phy_mode_detected=TRUE;
			gpioctlSetOutput("pon_S_Sel",1);
			write_data =(read_data & 0xffffff00|0x0a);
			SET_WAN_CONF(write_data);
			combo_pon_phy_init();
			REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_COMBO_XGSPON);
		}
		
    }
	else if(pon_phy_los_2==0)
	{
		gpPhyPriv->wan_sel =SCU_WAN_CONF_REG_WAN_SEL_XGSPON;
		switch_to=PHY_XGSPON_CONFIG;
		gpPhyPriv->is_phy_mode_detected=TRUE;
		gpioctlSetOutput("pon_S_Sel",1);
		write_data =(read_data & 0xffffff00|0x0a);
		SET_WAN_CONF(write_data);
		combo_pon_phy_init();
		REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_ONLY_XGSPON);

	}
	else if(pon_phy_los_1==0)
	{
	    gpPhyPriv->wan_sel =SCU_WAN_CONF_REG_WAN_SEL_GPON;
		switch_to=PHY_GPON_CONFIG;
		gpPhyPriv->is_phy_mode_detected=TRUE;
		gpioctlSetOutput("pon_S_Sel",0);
		write_data = read_data & 0xffffff00;
		SET_WAN_CONF(write_data);
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
		//gpPhyPriv->i2c1_used = TRUE;
#endif
		combo_pon_phy_init();
		REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_ONLY_GPON);
	}
	else
	{
		switch_to=PHY_UNKNOWN_CONFIG;
		gpPhyPriv->is_phy_mode_detected=FALSE;
			//return 1;
	}

	if(switch_to!=current_phy_mode)
	{
		current_phy_mode=switch_to;	
	}

	}
	else
	{
	    if((pon_phy_los_2==0)&&(pon_phy_los_1==0))
	    {
			if(gpPhyPriv->wan_sel ==SCU_WAN_CONF_REG_WAN_SEL_GPON)
			{
				if(current_phy_mode != PHY_GPON_CONFIG)
				{
					REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_COMBO_GPON);
					return 0;
				}
			}
			else
			{
				if(current_phy_mode != PHY_XGSPON_CONFIG)
				{
					REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_COMBO_XGSPON);
					return 0;
				}
			}
			
	    }
		else if(pon_phy_los_2==0)
		{
			if(current_phy_mode != PHY_XGSPON_CONFIG)
			{
				REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_ONLY_XGSPON);
                        	return 0;
                        }
		}
		else if(pon_phy_los_1==0)
		{
			if(current_phy_mode != PHY_GPON_CONFIG)
			{
				REPORT_EVENT_TO_COMBO(PHY_EVENT_DETECT_ONLY_GPON);
	                        return 0;
                        }
		}
	}

	return 1;
}

int phy_mode_task_wait(void)
{
	int ret=0;

	PON_PHY_PRINT(PHY_MSG_TRANS, "\r\n#TASK:[%s] run\r\n",__FUNCTION__);
	
	while(!kthread_should_stop())
	{
		msleep(1000);
		if((ret = phy_mode_detection())==0)
		{
			PON_PHY_PRINT(PHY_MSG_ERR,"\e[1;33mphy_mode_detection mismatch\e[0m\n");
			return ret;
		}
	}
	
	return ret;
}
int combo_pon_phy_init(void)
{
	int ret=0;

		
	if((ret = pon_phy_get_wan_sel()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"combo:pon_phy_get_wan_sel fail.\n") ;
		return ret ;
	}

	if((ret = pon_phy_func_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"combo:pon_phy_func_init fail.\n") ;
		return ret ;
	}	
	
	rcu_assign_pointer (set_pon_phy_mode_config,phy_mode_config);
	rcu_assign_pointer (set_pon_phy_start,xpon_phy_start);
	rcu_assign_pointer (set_pon_phy_stop,xpon_phy_stop);
	rcu_assign_pointer (get_pon_phy_trans_status,pon_phy_get_trans_status);	

	//set_pma_opt(XPON_PMA_OPT_DEFAULT_VALUE_HIGH_RATE);	// default use pma_opt value for ECNT REF board 	by david 20180920

#if EN7580_FLASH || EN7581_FLASH
// Get FLASH FIR&BOB
	if((ret = get_flash_matrix()) != 0){			
		set_flash_register_default();	//Reset flash_matrix to default
		PON_PHY_PRINT(PHY_MSG_ERR,"combo:Get FLASH fail: FLASH matrix reset to default!\n");	
		return ret ;
	}
#endif


	//printk("combo:____________________________________________\r\n\r\n");

	return ret ;
}

#endif

int phy_mode_config(Xpon_Phy_Mode_t xpon_mode, int tx_enable)
{
	int ret = PHY_SUCCESS;
	uint write_data = 0 ;
	uint read_data = 0 ;

    if(PHY_ENABLE != tx_enable && PHY_DISABLE != tx_enable) {
        printk("can not recognize tx_enable %d\n", tx_enable);
		ret=PHY_FAILURE;
        return ret;
    }

	gpPhyPriv->phyCfg.flags.mode=xpon_mode;
	gpPhyPriv->trans_tx_enable=tx_enable;

	printk("\r\n[SW] set xpon_mode=%d \n",xpon_mode);

#if 0 //XILINX_SERDES
	read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
	printk("Original HW SCU_WAN_CONF_REG_WAN_SEL=%d \n",read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS);
	IO_SPHYREG(SCU_WAN_CONF_REG,((read_data&SCU_WAN_CONF_REG_WAN_SEL_MASK)|SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G));
	printk("#PHY FW# Set SCU_WAN_CONF_REG_WAN_SEL=%d\n",SCU_WAN_CONF_REG_WAN_SEL_EPON);
#endif	

	if((ret = pon_phy_mod_init())!=0){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] pon_phy_mod_init fail !\r\n\r\n",__FUNCTION__,__LINE__);
		gpPhyPriv->phy_trans_task_wait= kthread_run(phy_trans_task_wait, NULL, "phy_trans_task_wait");
		return ret;  //return success to MAC trigger MAC set xpon_phy_start PHY_SUCCESS;  // 
	}
	
	if(TRUE==gpPhyPriv->pma_init_done){
		if((ret = phy_trans_power_switch(gpPhyPriv->trans_tx_enable))!=0){
			PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] phy_trans_power_switch fail !\r\n\r\n",__FUNCTION__,__LINE__);
			return ret;
		}
	}
	
	printk("[FW] Final phy_mode_config mode=%d\n",gpPhyPriv->phyCfg.flags.mode);

	printk("[FW] Final phy_tx_enable mode=%d\n",gpPhyPriv->trans_tx_enable);

	return ret ;
}


/*****************************************************************************
//function :
//		pon_phy_mod_init
//description : 
//		this function is the initial setting for XPON phy after PHY mode set by MAC
//input :	
//		N/A
//output :
//		0 	: initial success
//		-1	: initial fail
******************************************************************************/
int pon_phy_mod_init(void)
{
	int ret=0;
	uint read_data = 0;
	uint write_data = 0;
	unchar ptr[4];
	uint phyIntStatus;	 //david 20170124

#if defined(TCSUPPORT_COMBO_PON)
	if(FALSE==gpPhyPriv->is_phy_mode_detected)
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"pon_phy_mod_init is_phy_mode_detected = %d !!\r\n",gpPhyPriv->is_phy_mode_detected);
	}
#endif

	PON_PHY_PRINT(PHY_MSG_ERR,"\r\n_______ PHY init step 1 _______ (start)\r\n");

	if(gpPhyPriv->mac_set_phy_init_step<1)
	{
		gpPhyPriv->mac_set_phy_init_step = 1;
	}
	PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d]mac_set_phy_init_step=%d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->mac_set_phy_init_step);

	// =====================  move code from   pon_phy_init()	to here  ====================
	if(TRUE==gpPhyPriv->phy_init_done){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\nAvoid do [%s:%d] twice !!\r\n",__FUNCTION__,__LINE__);
		return ret;
	}

	//PON_PHY_PRINT(PHY_MSG_TRACE,"2x=0x%8x  ge=0x%8x  xg=0x%8x  xe=0x%8x	FuncTab\n",&mt752x_func,&en7580_gepon_func,&en7580_xgpon_func,&en7580_xepon_func);
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x	0x%8x  *=0x%8x	ponPhyFunc\n",&ponPhyFunc,ponPhyFunc,*ponPhyFunc);

	if((ret = pon_phy_get_wan_sel()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"pon_phy_get_wan_sel fail !\n") ;
		return ret ;
	}

	if((ret = pon_phy_func_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR,"pon_phy_func_init fail !\n") ;
		return ret ;
	}

    if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G) {//nick20240821_automode	
	    if((ret = xPON_Phy_Reset()) != 0) {
		    PON_PHY_PRINT(PHY_MSG_ERR,"xPON_Phy_Reset fail !\n") ;
		    return ret ;
	    }
	}
	
	if((ret = phy_trans_model_setting())!=0)
	{
		PON_PHY_PRINT(PHY_MSG_ERR, "[%s:%d]phy_trans_model_setting init failed.\n",__FUNCTION__,__LINE__) ;
		return ret;
	}

#ifdef LDDLA_SUPPORT_SET_TX_MODE
	
	if(TRUE==gpPhyPriv->lddla_auto_mode)
	{
		if((82==gpPhyPriv->trans_index)||(93==gpPhyPriv->trans_index))
		{
			if((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XGPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XGSPON))
			{
				LDDLA_SET_TX_MODE(0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Auto Mode[XG/XGS] LDDLA_SET_TX_MODE(0)\n");
			}
			else
			if((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G))
			{
				LDDLA_SET_TX_MODE(1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Auto Mode[XE/XES] LDDLA_SET_TX_MODE(1)\n");
			}
		}
	}
#endif

	// *******************	Roger_A60972  *******************  
#if A60972_SERDES
	if((ret = xpon_pma_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "A60972 init failed.\n") ;
		return ret ;
	}

	if((ret = phy_pma_reset()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "A60972 reset failed.\n") ;
		return ret ;
	}

	gpPhyPriv->pma_init_done=TRUE;
#endif

#if ASIC_SERDES

	if((ret = xpon_pma_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "EN7580 PMA init failed.\n") ;
		return ret ;
	}
	
#endif


	if((ret = phy_dev_init()) != 0) {
		PON_PHY_PRINT(PHY_MSG_ERR, "phy device initialization failed.\n") ;
		return ret ;
	}

	//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);	

	/* Enable BIP error counter */
	phy_cnt_enable(PHY_ENABLE, PHY_ENABLE, PHY_ENABLE);

	phy_reset_counter();

#if 0
	init_timer(&gphy_rx_test_timer);
	gphy_rx_test_timer.data = 10000;
	gphy_rx_test_timer.function = phy_rx_sens_timer;

	init_timer(&gpPhyPriv->int_timer) ;
	gpPhyPriv->int_timer.data = 50000 ; /* 50,000 ms */
	gpPhyPriv->int_timer.function = phy_int_recover_expires ;	
#endif

	/* callback function initial */
#ifdef TCSUPPORT_XPON_HAL_API_EXT
	gpPhyPriv->phyXponStateNotifyHandler = NULL ;
#endif /* TCSUPPORT_XPON_HAL_API_EXT */

	//memset(&gpPhyPriv->phyCfg, 0, sizeof(PHY_Config_T)) ;
	gpPhyPriv->phyCfg.t1 = 200 ;
	gpPhyPriv->phyCfg.t2 = 30 ;
	gpPhyPriv->phyCfg.intCounter = 0 ;
	gpPhyPriv->phyCfg.intTime = 0 ;
	
	gpPhyPriv->phyCfg.flags.txPowerEnFlag = TRUE;
	gpPhyPriv->phyCfg.flags.txLongFlag = FALSE;

	
	gpPhyPriv->phy_init_done = TRUE;

	PON_PHY_PRINT(PHY_MSG_ERR,"_______ PHY init step 1 _______ (done)\r\n\r\n");

	return ret ;
}


int xpon_phy_start(void)
{
	int ret = 0;	//ang_20180208
	uint read_data = 0;
	uint write_data = 0;
	uint wan_set = 0;

#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm
	struct device *dev = NULL ;
#endif
    gpPhyPriv->pon_stop_flag =FALSE;
#if defined(TCSUPPORT_COMBO_PON)
	if(FALSE==gpPhyPriv->is_phy_mode_detected)
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"xpon_phy_start is_phy_mode_detected = %d!!\r\n",gpPhyPriv->is_phy_mode_detected);
	}
#endif

	PON_PHY_PRINT(PHY_MSG_ERR,"\r\n_______ PHY init step 2 _______ (start)\r\n");
	gpPhyPriv->mac_set_phy_init_step = 2;
	PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d]mac_set_phy_init_step=%d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->mac_set_phy_init_step);
	
	if(TRUE==gpPhyPriv->is_phy_start){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\nAvoid do [%s:%d] twice !!  gpPhyPriv->is_phy_start = %d  ret = %d\r\n",__FUNCTION__,__LINE__, gpPhyPriv->is_phy_start, ret);
		return ret ;
	}

	if(FALSE==gpPhyPriv->phy_init_done){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] WARNING ! xpon_phy init NOT complete, init again !\r\n\r\n",__FUNCTION__,__LINE__);
		if((ret = pon_phy_mod_init())!=0){
			PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] pon_phy_mod_init fail !\r\n\r\n",__FUNCTION__,__LINE__);
			return ret;
		}
		
		if((ret = phy_trans_power_switch(gpPhyPriv->trans_tx_enable))!=0){
			PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] phy_trans_power_switch fail !\r\n\r\n",__FUNCTION__,__LINE__);
			return ret;
		}
	}

	gpPhyPriv->is_phy_start = TRUE ;

	gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;	//by david 20180323

	ret = pon_phy_clear_int();	//by ang_20180502

	if(FALSE==gpPhyPriv->is_irq_requested)
	{
		gpPhyPriv->is_irq_requested=TRUE;
		phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);	
#ifdef TCSUPPORT_CPU_ARMV8
		if((dev=get_pon_phy_dev()) == NULL){
			PON_PHY_PRINT(PHY_MSG_ERR, "Get device failed.\n") ;
			return ret;
		}
		else{
		ret = request_irq(get_pon_phy_irq(),phy_isr_request,0,"phy_isr",dev);
			PON_PHY_PRINT(PHY_MSG_ERR, "\n--- ARM phy_isr_request ---\n") ;
		}
#else
		ret = request_irq(XPON_PHY_INTR,phy_isr,0,"phy_isr",NULL);
#endif
		if(ret != 0) {
			PON_PHY_PRINT(PHY_MSG_ERR, "Register PON ISR handler function failed !\n") ;
			return ret ;
		}
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d]request_irq already, enable ISR instead, is_irq_requested=%d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->is_irq_requested);
		phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
	}

	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	PON_PHY_PRINT(PHY_MSG_ERR,"_______ PHY init step 2 _______ (done)\r\n\r\n");

	return 0 ;
}

int xpon_phy_stop(void)
{
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm
		struct device *dev = NULL ;
#endif
	gpPhyPriv->pon_stop_flag =TRUE;


	if(TRUE==gpPhyPriv->is_phy_start){
	    gpPhyPriv->is_phy_start = FALSE ;

		if(TRUE==gpPhyPriv->is_irq_requested)
		{
			if(in_interrupt())
			{
				PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d]Can't free_irq in ISR, disable ISR instead, is_irq_requested=%d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->is_irq_requested);
				phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			}
			else
			{
				gpPhyPriv->is_irq_requested = FALSE;
				phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
				PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] is_phy_start = %d,is_irq_requested = %d\r\n",__FUNCTION__,__LINE__, gpPhyPriv->is_phy_start,gpPhyPriv->is_irq_requested);
#ifdef TCSUPPORT_CPU_ARMV8
	if((dev=get_pon_phy_dev()) == NULL){
		PON_PHY_PRINT(PHY_MSG_ERR, "Get device failed.\n") ;
		return -1;
	}
	else{
		free_irq(get_pon_phy_irq(),dev);  //julia_arm
		PON_PHY_PRINT(PHY_MSG_ERR, "\n--- ARM free_irq ---\n") ;
		return 0;
	}
#else
		free_irq(XPON_PHY_INTR, NULL);
#endif
			}
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] Bypass free_irq due to is_irq_requested=%d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->is_irq_requested);
		}

   	 	phy_fw_ready(PHY_DISABLE);
   	 	PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
    	return 0;
	}
	else
	{
		if((gpPhyPriv->phy_deinit_flag ==TRUE)||(gpPhyPriv->pon_stop_flag ==TRUE))
			PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
		
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n xpon_phy NOT start yet!! [%s:%d] gpPhyPriv->is_phy_start = %d\r\n",__FUNCTION__,__LINE__, gpPhyPriv->is_phy_start);
		return 0;
	}
}


/*****************************************************************************
******************************************************************************/
void pon_phy_deinit(void)
{
	/* Stop the phy and int timer */
	//PHY_STOP_TIMER(gpPhyPriv->phy_timer) ; /* mark it ,cause phy-timer didnt init */
	printk("\r\n______pon_phy_deinit_______\r\n\r\n");

	gpPhyPriv->phy_deinit_flag =TRUE;
		
	xpon_phy_stop();
    mdelay(2000);
	
	if(!IS_ERR(gpPhyPriv->phy_trans_task_wait)){
		if(gpPhyPriv->phy_trans_task_wait != NULL)
		kthread_stop(gpPhyPriv->phy_trans_task_wait);
	}

#if defined(TCSUPPORT_COMBO_PON)	
	if(gpPhyPriv->combo_pon_enable ==TRUE)
	{
		if(!IS_ERR(gpPhyPriv->phy_mode_task_wait)){
			if(gpPhyPriv->phy_mode_task_wait != NULL)
			kthread_stop(gpPhyPriv->phy_mode_task_wait);
		}
	}
#endif

	if(!IS_ERR(gpPhyPriv->phy_ngpon2_task_wait)){ //julia_20221021 ngpon2
	if(gpPhyPriv->phy_ngpon2_task_wait != NULL)
		kthread_stop(gpPhyPriv->phy_ngpon2_task_wait);
	}


	//PHY_STOP_TIMER(gpPhyPriv->int_timer) ;
	/* transceiver power off*/
	phy_trans_power_switch(PHY_DISABLE);

	phy_debug_deinit();

	rcu_assign_pointer (set_pon_phy_mode_config,NULL);
	rcu_assign_pointer (set_pon_phy_start,NULL);
	rcu_assign_pointer (set_pon_phy_stop,NULL);
	rcu_assign_pointer (get_pon_phy_trans_status,NULL);

	#if A60972_SERDES
	xpon_pma_debug_deinit();
	#endif

	if(gpPhyPriv != NULL) {
		kfree(gpPhyPriv) ;
		gpPhyPriv = NULL ;
	}

    ecnt_unregister_hook(&phy_api_dispatch_hook_ops);
}



module_init(pon_phy_init)
module_exit(pon_phy_deinit)
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(phy_register_cb_handler);
EXPORT_SYMBOL(phy_unregister_cb_handler);

