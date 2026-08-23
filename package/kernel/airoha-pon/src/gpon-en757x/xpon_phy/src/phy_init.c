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
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>

#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_api.h"
#include "mt7570_reg.h"
#include "mt7570_def.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy_init.h"
#include "phy_tx.h"


extern atomic_t eponMacRestart_flag;
extern int mt7570_select;						//by HC 20150302
extern int en7571_select;						//by HC 20160608
extern int mt7570_version;						//by HC 20150320
extern int en7571_version;						//by HC 20160608
extern int en7571_ver;							//by HC 20170913
extern int phy_version;							//The version of phy.c in main trunk 
int phy_date_version = 170824;						//The version of phy_init.c in main trunk

extern int internal_DDMI;                       //by HC 20150119
extern uint mt7570_alarm;						//by YMC 20150414
extern uint en7571_alarm;						//by HC 20160608
extern int TEC_switch;							// by HC 20150515
extern int BiasTracking_switch;				// by HC 20150921


#ifndef TCSUPPORT_CPU_EN7521
void pon_phy_reset_init(void);
extern void (*fe_reset_pon_phy_reset_init)(void);
#endif
extern void phy_tx_fault_reset(void);

PHY_GlbPriv_T *gpPhyPriv = NULL;

struct workqueue_struct * i2c_access_queue = NULL;



int a_tdc_setting=0x2b;

void phy_tx_ctl(unchar val);
/*****************************************************************************
//function :
//		gpio_tx_dis_reset
//description : 
//		this function is used to pull down tx_dis of EN7570.
//		Since tx_dis is connected to a GPIO output pin in MT7520, the setting in this function need to 
//		fit customer's allocation of tx_dis.
//		ex: On reference BOB board, tx_dis matches GPIO25, so this function pulls GPIO25(tx_dis) low.
//		On ASB's BOB board, tx_dis matches GPIO40, so this function should pulls GPIO40(tx_dis) low.
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
void gpio_tx_dis_reset(void){
#ifdef CONFIG_USE_MT7520_ASIC
    phy_tx_ctl(PHY_DISABLE);
	phy_tx_fault_reset();
#endif
}


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
#ifdef CONFIG_USE_MT7520_ASIC
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
void xPON_Phy_Reset(void){
#ifdef CONFIG_USE_MT7520_ASIC
	uint val = 0;
#ifdef TCSUPPORT_CPU_EN7521
	//xpon phy top reset ,EN7521 high active
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
#else
	//xpon phy top reset
	val = IO_GPHYREG(TOP_TEST_MISC0_CTRL);
	val &=~(1<<25);
	IO_SPHYREG(TOP_TEST_MISC0_CTRL, val);
	udelay(1);
	//release xpon phy top reset 
	val |=(1<<25);
	IO_SPHYREG(TOP_TEST_MISC0_CTRL, val);
#endif
#endif
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
#if defined(CONFIG_USE_MT7520_ASIC)
	phy_int_config((PHY_ILLG_INT|PHYRDY_INT|PHY_LOF_INT|TRANS_LOS_INT)) ;
#else
	phy_int_config((PHYRDY_INT|PHY_LOF_INT|TRANS_LOS_INT)) ;
#endif
}
	

static void phy_ready_recover_expires(ulong data) 
{
	uint write_data = 0 ;
	uint read_data = 0 ;

	read_data=IO_GPHYREG(PHY_CSR_PHYSTA1);

	if(TRUE == gpPhyPriv->is_phy_start){
		if((((read_data >> PHYRDY_OFFSET) & PHYRDY_MASK) == 0x02) && (phy_los_status()==PHY_NO_LOS_HAPPEN)){
			/*reset phy*/
			read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
			write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
			IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
			mdelay(1);
			IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;
			PON_PHY_MSG(PHY_MSG_DBG, "Reset RX PLL when LOS=0 && PHY_READY=0.\n") ;
		}
	}

	PHY_START_TIMER(gpPhyPriv->phy_ready_timer) ;
	
}	
int LDDLA_task_wait(void)
{
	PON_PHY_MSG(PHY_MSG_TRACE, "LDDLA_task_wait\n");
	
	while(!kthread_should_stop())
	{
		/*wait_event_interruptible(pon_phy_wq_7570, (atomic_read(&pon_phy_7570_flag) != 0)); *//* replace !=0 with == 1 */
		
		if( en7571_select == 1 )
			en7571_internal_clock();
		else if( mt7570_select == 1 )
			mt7570_internal_clock();

			msleep(1000);
	}
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
    uint read_data = 0;
    uint write_data = 0;
	uint data = 0;

#if defined(CONFIG_USE_A60928)
	//PLL & software reset
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	read_data = read_data |(PHY_PLL_RST);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data);
	mdelay(1);
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	read_data = read_data | (PHY_Software_Reset);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data);
	mdelay(1);

	// Release PLL & software reset
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	read_data = read_data & ~(PHY_PLL_RST);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data);
	mdelay(1);
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	read_data = read_data & ~(PHY_Software_Reset);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data);
	mdelay(1);
#endif

#ifdef CONFIG_USE_MT7520_ASIC
#ifndef TCSUPPORT_CPU_EN7521
	/* switch GPIO to XPON mode*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data | (GPIO_PHY_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif /* CONFIG_USE_MT7520_ASIC */	
#ifdef TCSUPPORT_CPU_EN7521
		/* switch GPIO to XPON mode*/
		read_data = IO_GREG(IOMUX_Control_1_register);
		read_data = read_data | (RG_GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
		IO_SREG(IOMUX_Control_1_register, read_data);
#endif 		

	/*sigdet*/
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	read_data = read_data & ~(PHY_SQUELCH_DEGLITCH);
	IO_SPHYREG(PHY_CSR_PHYSET3, read_data);
	/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif /* CONFIG_USE_MT7520_ASIC */



#ifdef CONFIG_USE_A60901
	IO_SPHYREG(PHY_CSR_PHYSET3, 0xCD810110) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_PHYSET3, 0x45810110) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_ANAPLLREG3, 0x00086100) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_PHYSET3, 0x4581E110) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_TDCSET1, 0x400000C8) ;
	mdelay(1);
	IO_SPHYREG(PHY_CSR_ANAPLLREG4, 0x00000013) ;
	//mdelay(1);
	//IO_SPHYREG(PHY_CSR_ANASET10, 0x211A0202) ;
#endif /* CONFIG_USE_A60901 */

#if !defined CONFIG_USE_MT7520_ASIC && !defined CONFIG_USE_A60901 && !defined CONFIG_USE_A60928
			/* Reset the SerDes Tx FIFO */
			IO_SPHYREG(PHY_CSR_GIO2_SETTING, 0x1) ;
			IO_SPHYREG(PHY_CSR_GIO2_SETTING, 0x0) ;
#endif /* !defined CONFIG_USE_MT7520_ASIC && !defined CONFIG_USE_A60901 */


#if defined(CONFIG_USE_MT7520_ASIC)
	phy_int_config((PHY_ILLG_INT|PHYRDY_INT|PHY_LOF_INT|TRANS_LOS_INT)) ;
#else
	phy_int_config((PHYRDY_INT|PHY_LOF_INT|TRANS_LOS_INT)) ;
#endif

	/* Enable BIP error counter */
	phy_cnt_enable(PHY_ENABLE, PHY_ENABLE, PHY_ENABLE) ;
	
	/* change the guard time pattern */
    gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983) ;

	/* clear phy register*/
	phy_reset_counter( );

	/*change TDC default setting*/
	#ifdef CONFIG_USE_MT7520_ASIC
		#ifndef TCSUPPORT_CPU_EN7521		
		IO_SPHYREG(PHY_CSR_TDCSET2, a_tdc_setting) ;
		#else
		a_tdc_setting=0x2d;
		IO_SPHYREG(PHY_CSR_TDCSET2, a_tdc_setting) ;		
		#endif
	#endif

	PON_PHY_MSG(PHY_MSG_TRACE, "xPON PHY initialization done\n") ;
	
	return 0 ;
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
	uint phyIntStatus ;

	/* Get interrtup status and clear */
	phyIntStatus = IO_GPHYREG(PHY_CSR_XPON_INT_STA) ;
	//printk("enter pon phy isr, status=0x%x\n", phyIntStatus);		//YMC_20150206
	IO_SPHYREG(PHY_CSR_XPON_INT_STA_CLR, phyIntStatus) ;


		if( mt7570_select == 1 )							// by HC 20150512
		{			
			TEC_switch = 0;		
			BiasTracking_switch = 0;					// by HC 20150921
		}	
	
#if !defined CONFIG_USE_MT7520_ASIC && !defined CONFIG_USE_A60901
		/* Reset the SerDes Tx FIFO */
		IO_SPHYREG(PHY_CSR_GIO2_SETTING, 0x1) ;
		IO_SPHYREG(PHY_CSR_GIO2_SETTING, 0x0) ;
#endif /* !defined CONFIG_USE_MT7520_ASIC && !defined CONFIG_USE_A60901 */

		/* Clear PHY bit delay */
		phy_bit_delay(0) ;
		
		/* reset phy */
		phy_reset_counter() ;

	PON_PHY_MSG(PHY_MSG_INT, "phy_los_handler doing phy_trans_los \n");
}

void phy_ready_handler(void)
{
    if( mt7570_select == 1 )			// by HC 20150512					
    {						
    	TEC_switch = 1;
		BiasTracking_switch = 1;	// by HC 20150921
		} 
}

/* Sadlly, from what we've learned from our experience, this func cannot alaways 
   get the right result. We really should try to find out why.
*/
static inline int is_ready_int_authentic(uint int_status)
{
#ifdef TCSUPPORT_CT_PON
    return ((!(int_status & TRANS_LOS_INT)) || (!(int_status & PHY_ILLG_INT))) ;
#else
    return (!(int_status & (TRANS_LOS_INT | PHY_ILLG_INT))) ;
#endif
}

static inline void handle_hw_irq_event(PHY_Event_Type_t id)
{
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
    phy_event.id = id;
    phy_event_handler(&phy_event);

}
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
void phy_isr(void)
{
	uint phyIntStatus ;
	PHY_TransConfig_T trans_status;
	PPHY_TransConfig_T p_trans_status = & trans_status;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	
	spin_lock(&gpPhyPriv->event_handle_lock);

	/* Get interrtup status and clear */
	phyIntStatus = IO_GPHYREG(PHY_CSR_XPON_INT_STA) ;
	//printk("enter pon phy isr, status=0x%x\n", phyIntStatus);		//YMC_20150206
	IO_SPHYREG(PHY_CSR_XPON_INT_STA_CLR, phyIntStatus) ;


	if((phyIntStatus & TRANS_LOS_INT) || (phyIntStatus & PHY_ILLG_INT)) {
        PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: TRSNS_LOS(%d)|PHY_ILLG(%d) interrupt.\n", 
			((phyIntStatus & TRANS_LOS_INT)? 1 : 0), ((phyIntStatus & PHY_ILLG_INT)? 1 : 0 ));
        gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS    ;
        
        if ((phyIntStatus & TRANS_LOS_INT ) && (phyIntStatus & PHY_ILLG_INT ) ) {
            handle_hw_irq_event(PHY_EVENT_TRANS_LOS_ILLG_INT) ;
        }else if (phyIntStatus & TRANS_LOS_INT) {
        handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
        }else if (phyIntStatus & PHY_ILLG_INT){
        	if(PHY_GPON_CONFIG != gpPhyPriv->phyCfg.flags.mode || !gpPhyPriv->phyCfg.flags.rogue)
                handle_hw_irq_event(PHY_EVENT_PHY_ILLG_INT) ;
	}
		PHY_START_TIMER(gpPhyPriv->phy_ready_timer) ;
		} 
	
	if(phyIntStatus & PHY_LOF_INT) {
		PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: transceiver LOF interrupt.\n") ;
        handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT);
	}
    
	if(phyIntStatus & TRANS_TF_INT) {
		phy_trans_tx_getting(p_trans_status);
		if((p_trans_status->trans_tx_fault_inv_status == PHY_DISABLE) ) {
			PON_PHY_MSG(PHY_MSG_INT, "turn off power.\n") ;
            handle_hw_irq_event(PHY_EVENT_TF_INT);
		}
		PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: transceiver TF interrupt.\n") ;
	}
    
	if(phyIntStatus & TRANS_INT) {
		PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: transceiver interrupt.\n") ;
        handle_hw_irq_event(PHY_EVENT_TRANS_INT);
	}
    
	if(phyIntStatus & TRANS_SD_FAIL_INT) {
		PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: transceiver TX_SD fail interrupt.\n") ;
        handle_hw_irq_event(PHY_EVENT_TRANS_SD_FAIL_INT);
	}
    
	if(phyIntStatus & PHYRDY_INT) {
		/* SW workaround for phy ready interrupt when LOS happened */
        if(is_ready_int_authentic(phyIntStatus))
                {
            gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
            handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				PHY_STOP_TIMER(gpPhyPriv->phy_ready_timer) ;
            
            PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: phy ready interrupt.\n") ;
        }else{
            PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: FALSE phy ready interrupt.\n");
		}
	}
	
	if(phyIntStatus & PHY_I2CM_INT) {
		PON_PHY_MSG(PHY_MSG_INT, "PHY IRQ: I2C master interrupt.\n") ;
        handle_hw_irq_event(PHY_EVENT_I2CM_INT);       
	}

	/* the following function is used to avoid frequently interrupt */
	if(gpPhyPriv->phyCfg.intCounter == 0) {
		gpPhyPriv->phyCfg.intTime = jiffies ;
	}
	gpPhyPriv->phyCfg.intCounter++ ;
	if(gpPhyPriv->phyCfg.intCounter == 10) {
		gpPhyPriv->phyCfg.intCounter = 0 ;

		if((jiffies - gpPhyPriv->phyCfg.intTime) < 50) { //phy interrupt occur every 5 system interrupt
			#if 0
			phy_int_config(0) ;
			phy_reset_counter() ;
			
			PHY_START_TIMER(gpPhyPriv->int_timer) ;
			#endif
			PON_PHY_MSG(PHY_MSG_ERR, "PHY detect interrupt error.\n") ;
		}
	}

    spin_unlock(&gpPhyPriv->event_handle_lock);

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

/*****************************************************************************
******************************************************************************/
int xpon_phy_start(void)
{
    int ret;
    gpPhyPriv->is_phy_start = TRUE ;

	IO_SPHYREG(PHY_CSR_XPON_INT_STA_CLR, 0xFF) ;
    
	IO_SPHYREG(PHY_CSR_TDCSET2, a_tdc_setting) ;	//Roger_20170122
    
#ifdef TCSUPPORT_CPU_EN7521
	ret = QDMA_API_REGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_XPON_PHY_HANDLER, phy_isr);
#else
	ret = qdma_register_callback_function(QDMA_CALLBACK_XPON_PHY_HANDLER, phy_isr) ; //register phy_isr from QDMA
#endif
	if(ret != 0) {
		PON_PHY_MSG(PHY_MSG_ERR, "Register GPON ISR handler function failed.\n") ;
		return ret ;
	}
	
    PHY_START_TIMER(gpPhyPriv->event_poll_timer);
	return 0 ;
}

int xpon_phy_stop(void)
{
    gpPhyPriv->is_phy_start = FALSE ;
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_XPON_PHY_HANDLER);
#else
	qdma_unregister_callback_function(QDMA_CALLBACK_XPON_PHY_HANDLER) ;	
#endif

    phy_fw_ready(PHY_DISABLE);
    PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
    return 0;
}

/*****************************************************************************
******************************************************************************/

struct timer_list gphy_rx_test_timer;
extern unsigned int gphy_test_rx_ber;

void phy_rx_sens_timer(ulong fdata){
	#define G248832 26718132554.957
	#define G10_12  1000000000000
	uint data = 0;
	double ber = 0;
	
	data = IO_GPHYREG(0xBFAF02C0);
	IO_SPHYREG(0xBFAF04A0, 0x0);
	IO_SPHYREG(0xBFAF04A4, 0x0);
	printk("Rx_Sensitivity_Test PRBS error counter: 0x%x\n", data);

	ber = (double)data / (G248832);
	data = ber*G10_12;
	gphy_test_rx_ber = data;

	printk("Rx_Sensitivity_Test BER: %u\n", data);

}

void phy_rx_sensitivity_test(void){
	uint data = 0;
	struct timer_list rx_timer;

	gphy_test_rx_ber = 0;

	gphy_rx_test_timer.expires = jiffies + 10*HZ;
	
	/* clear PRBS counter */
	IO_SPHYREG(0xBFAF04A0, 0x0);
	IO_SPHYREG(0xBFAF04A4, 0x0);

	/* Set to PRBS7 test */
	IO_SPHYREG(0xBFAF0124, 0xFF200000);
	IO_SPHYREG(0xBFAF02A0, 0x81FD53);
	IO_SPHYREG(0xBFAF02A4, 0xFFFFFF);
	IO_SPHYREG(0xBFAF04A0, 0x5);

	add_timer(&gphy_rx_test_timer);
	IO_SPHYREG(0xBFAF04A4, 0x1);

	data = IO_GPHYREG(0xBFAF02C4);
	printk("Rx_Sensitivity_Test start: 0x%x\n", data);
	
}

static inline int get_tx_log_flag(void)
{
    return gpPhyPriv->phyCfg.flags.txLongFlag;
}

static void pon_phy_get_api_dispatch(xpon_phy_api_data_t * api_data)
{
    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_GET_PHY_LOS_STATUS:
            api_data->ret = phy_los_status();
            break;
			
        case PON_GET_PHY_READY_STATUS:
            api_data->ret = phy_ready_status();
            break; 
			
        case PON_GET_PHY_IS_SYNC:
            api_data->ret = is_phy_sync(); // for this API, in_data is NULL
            break;

        case PON_GET_PHY_MODE:
            api_data->ret = get_phy_mode();
            break;
            
        case PON_GET_PHY_TX_LONG_FLAG:
            api_data->ret = get_tx_log_flag();
            break;

        case PON_GET_PHY_TRANS_STATUS:
    		if(1 == en7571_select){
    			en7571_param_status_real(&api_data->trasn_status->params);
                api_data->trasn_status->alarms = en7571_alarm;
    		}else if(1 == mt7570_select){
    			mt7570_param_status_real(&api_data->trasn_status->params);
                api_data->trasn_status->alarms = mt7570_alarm;
    		}else{
    			phy_trans_param_status_real(&api_data->trasn_status->params);
                api_data->trasn_status->alarms = phy_tx_alarm() | phy_rx_power_alarm(); // get transceiver alarm message, only for transceiver module.
    		}
            break;

        case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
            api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;
            break;

        case PON_GET_PHY_BIP_COUNTER:
            api_data->ret = phy_bip_counter();
            break;

        case PON_GET_PHY_RX_FEC_COUNTER:
            phy_rx_fec_counter(api_data->rx_fec_cnt);
            break;

        case PON_GET_PHY_RX_FRAME_COUNTER:
            phy_rx_frame_counter(api_data->rx_frame_cnt);
            break;

        case PON_GET_PHY_RX_FEC_GETTING:
            api_data->ret = phy_rx_fec_getting();
            break;

        case PON_GET_PHY_RX_FEC_STATUS:
            api_data->ret = phy_rx_fec_status();
            break;

        case PON_GET_PHY_TX_FEC_STATUS:
            api_data->ret = phy_tx_fec_status();
            break;

        case PON_GET_PHY_TX_BURST_GETTING:
            api_data->ret = phy_tx_burst_getting();
            break;

        case PON_GET_PHY_TRANS_TX:
            phy_trans_tx_getting(api_data->tx_trans_cfg);
            break;

        case PON_GET_PHY_TRANS_RX_GETTING:
            api_data->ret = phy_trans_rx_getting();
            break;
#ifdef TCSUPPORT_CPU_EN7521
        case PON_GET_PHY_ROUND_TRIP_DELAY:
			api_data->ret=phy_round_trip_delay_sof();
			break;
#endif		
		case PON_GET_PHY_EN7571_VER:		//by HC 20170913
			api_data->ret = en7571_ver;
			break;
			
        default:
            dump_stack();
            printk("unknown command id!\n");
            api_data->ret = PHY_NO_API;
            break;
    }
}


static inline void resotre_preamble_and_guard_bit(void)
{   
     IO_SPHYREG(PHY_CSR_GPON_PREAMBLE, gpPhyPriv->phyPreamble);
     IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, gpPhyPriv->phyGuardBitDelm);
     IO_SPHYREG(PHY_CSR_GPON_EXTEND_PREAMBLE, gpPhyPriv->phyExtPreamble);
}


static void pon_phy_set_api_dispatch(xpon_phy_api_data_t * api_data)
{
    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);
            break;

        case PON_SET_PHY_RESET_COUNTER:
            api_data->ret = phy_reset_counter();
            break;

        case PON_SET_PHY_START:
            xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            api_data->ret = phy_fw_ready(*api_data->data);
            break;

        case PON_SET_PHY_COUNTER_CLEAR:
            phy_counter_clear(*api_data->data);
            break;

        case PON_SET_EPONMODE_PHY_RESET:
            api_data->ret = eponMode_phy_reset();
            break;

        case PON_SET_PHY_BIT_DELAY:
            api_data->ret = phy_bit_delay(*api_data->data);
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:
            api_data->ret = phy_rogue_prbs_config(*api_data->data);
            break;

        case PON_SET_PHY_DEV_INIT:
            api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            resotre_preamble_and_guard_bit();
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
            phy_trans_model_setting();
            break;

        case PON_SET_PHY_RX_FEC_SETTING:
            api_data->ret = phy_rx_fec_setting(*api_data->data);
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
            api_data->ret = phy_tx_burst_config(*api_data->data);
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:
            api_data->ret = phy_gpon_delimiter_guard(api_data->delimiter_guard->delimiter, 
                                                     api_data->delimiter_guard->guard_time);
            break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
            api_data->ret = phy_trans_tx_setting(api_data->tx_trans_cfg);
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
            api_data->ret = phy_trans_rx_setting(*api_data->data);
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            api_data->ret = phy_gpon_preamble(api_data->gpon_preamble);
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
            api_data->ret = phy_gpon_extend_preamble(api_data->gpon_ex_preamble);
            break;
#ifdef TCSUPPORT_CPU_EN7521
	case PON_SET_PHY_EPON_TS_CONTINUE_MODE:
            api_data->ret = phy_set_epon_ts_continue_mode(*api_data->data);
            break;
#endif       
#if (defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON))&& defined(TCSUPPORT_XPON_HAL_API_EXT)
        case PON_SET_PHY_NOTIFY_EVENT:
            if(gpPhyPriv->phyXponStateNotifyHandler){
                gpPhyPriv->phyXponStateNotifyHandler(*api_data->data);
            }
            break;
#endif
    case PON_SET_PHY_TRAFFIC_STATUS:
        api_data->ret = phy_set_traffic_status(*api_data->data);
            break;
            
        default:
            printk("unknown command id!\n");
            api_data->ret = PHY_NO_API;
            break;
    }
}


int pon_phy_api_dispatch(struct ecnt_data *in_data)
{
    xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *)in_data;
    switch(api_data->api_type) 
    {
        case XPON_PHY_API_TYPE_GET:
            pon_phy_get_api_dispatch(api_data);
            break;

        case XPON_PHY_API_TYPE_SET:
            pon_phy_set_api_dispatch(api_data);
            break;

        default:
            dump_stack();
            printk("unknown api_data->api_type: %d\n", api_data->api_type);
            api_data->ret = PHY_NO_API;
            break;
    }
    
    return ECNT_RETURN;
}


struct ecnt_hook_ops phy_api_dispatch_hook_ops = {
    .name = "pon_phy_api_dispatch",
    .hookfn = pon_phy_api_dispatch,
    .is_execute = 1,
    .maintype = ECNT_XPON_PHY,
    .subtype = ECNT_XPON_PHY_API,
    .priority   = 1,
};

/*****************************************************************************
//function :
//		pon_phy_init
//description : 
//		this function is the initial setting for XPON phy
//input :	
//		N/A
//output :
//		0 	: initial success
//		-1	: initial fail
******************************************************************************/
int pon_phy_init(void)
{
	int ret = 0 ;
	int LDD_LA_version = -1;
	uint read_data = 0;
	uint write_data = 0;
	unchar ptr[4];
	//read_data = IO_GPHYREG(PHY_CSR_PHYSET4);
	//write_data = read_data & (0xfffffdfc) ;
	//IO_SPHYREG(PHY_CSR_PHYSET4,write_data);

	if (NULL == gpPhyPriv)
	{
		gpPhyPriv = (PHY_GlbPriv_T *)kmalloc(sizeof(PHY_GlbPriv_T), GFP_KERNEL) ;
		if( NULL == gpPhyPriv) {
			printk("Alloc data struct memory failed\n") ;
			return ret ;
		}
	}
	
	/*Reset the xpon_phy*/
	xPON_Phy_Reset();
	#ifndef TCSUPPORT_CPU_EN7521
	fe_reset_pon_phy_reset_init = pon_phy_reset_init;
	#endif

	gpPhyPriv->i2c_u2_clk_div = I2C_U2_CLK_DIV ;
	#if !defined(CONFIG_USE_MT7520_ASIC) && !defined(CONFIG_USE_A60928) && !defined(CONFIG_USE_A60901)
	if((ret = i2c_init()) != 0) {
		PON_PHY_MSG(PHY_MSG_ERR, "PHY I2C sccb controller initialization failed.\n") ;
		return ret ;
	}
	#endif /* !defined CONFIG_USE_MT7520_ASIC */
	
    /* these two line must be put before registering isr handler */
    gpPhyPriv->is_phy_start = FALSE   ;     
    gpPhyPriv->phyCfg.flags.rogue = 0 ;
    gpPhyPriv->phyCfg.flags.mode  = PHY_UNKNOWN_CONFIG ;

    spin_lock_init(&gpPhyPriv->event_handle_lock);
   
    gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;

	init_timer(&gpPhyPriv->event_poll_timer);
	gpPhyPriv->event_poll_timer.data = 1000;
	gpPhyPriv->event_poll_timer.function = phy_event_poll; 

	if((ret = phy_debug_init()) != 0) {
		PON_PHY_MSG(PHY_MSG_ERR, "phy debug initialization failed.\n") ;
		return ret ;
	}


	if((ret = phy_dev_init()) != 0) {
            PON_PHY_MSG(PHY_MSG_ERR, "phy device initialization failed.\n") ;
            return ret ;
        }
												
	if( en7571_EN7571_detection() == 1 )		// by HC 20160608
	{
		printk("EN7571 found!\n");		
		if( (ret = en7571_init()) != 0 )		//Initialization failed 
		{
			PON_PHY_MSG(PHY_MSG_ERR, "7571 initialization failed.\n") ;
			return ret ;
		}
		gpio_tx_dis_reset();
		gpio_BOSA_Tx_power_on();				// by YMC 20150731
					
		gpPhyPriv->LDDLA_task_wait = kthread_run(LDDLA_task_wait, NULL, "LDDLA_task_wait");
		LDD_LA_version = en7571_version;
	}
	else if( mt7570_EN7570_detection() == 1 )		//by HC 20150302
	{		
		printk("EN7570 found!\n");		
		if( (ret = mt7570_init()) != 0 )		//Initialization failed 
		{
			PON_PHY_MSG(PHY_MSG_ERR, "7570 initialization failed.\n") ;
			return ret ;
		}
		gpio_tx_dis_reset();
		gpio_BOSA_Tx_power_on();				// by YMC 20150731
					
		gpPhyPriv->LDDLA_task_wait = kthread_run(LDDLA_task_wait, NULL, "LDDLA_task_wait");
		LDD_LA_version = mt7570_version;
	}
	else
		printk("EN757x not found!\n");


	phy_trans_model_setting(); 				// set transceiver config 

#ifdef CONFIG_USE_MT7520_ASIC
	/*enable FW ready*/
	//phy_fw_ready(PHY_ENABLE) ;
#endif /* CONFIG_USE_MT7520_ASIC */
	/*************************************************************************************/

	printk("PON PHY driver version is %d.%d.%d\n",phy_date_version,phy_version,LDD_LA_version);
	
	/*************************************************************************************/
	init_timer(&gphy_rx_test_timer);
	gphy_rx_test_timer.data = 10000;
	gphy_rx_test_timer.function = phy_rx_sens_timer;

	init_timer(&gpPhyPriv->int_timer) ;
	gpPhyPriv->int_timer.data = 50000 ; /* 50,000 ms */
	gpPhyPriv->int_timer.function = phy_int_recover_expires ;	

	init_timer(&gpPhyPriv->phy_ready_timer) ; 
	gpPhyPriv->phy_ready_timer.data = 2000 ; /* 2,000 ms */
	gpPhyPriv->phy_ready_timer.function = phy_ready_recover_expires ;

	/* callback function initial */
#ifdef TCSUPPORT_XPON_HAL_API_EXT
	gpPhyPriv->phyXponStateNotifyHandler = NULL ;
#endif /* TCSUPPORT_XPON_HAL_API_EXT */


	memset(&gpPhyPriv->phyCfg, 0, sizeof(PHY_Config_T)) ;
	gpPhyPriv->phyCfg.t1 = 200 ;
	gpPhyPriv->phyCfg.t2 = 30 ;
	gpPhyPriv->phyCfg.intCounter = 0 ;
	gpPhyPriv->phyCfg.intTime = 0 ;
	
	gpPhyPriv->phyCfg.flags.txPowerEnFlag = TRUE;
	gpPhyPriv->phyCfg.flags.txLongFlag = FALSE;
	gpPhyPriv->debugLevel = 0x0001;

	PHY_START_TIMER(gpPhyPriv->phy_ready_timer) ;

    if (ECNT_REGISTER_SUCCESS != ecnt_register_hook(&phy_api_dispatch_hook_ops) ){
        panic("Register hook function failed! %s:%d", __FUNCTION__, __LINE__);
    }

	return ret ;
}

/*****************************************************************************
******************************************************************************/
void pon_phy_deinit(void)
{
	/* Stop the phy and int timer */
	//PHY_STOP_TIMER(gpPhyPriv->phy_timer) ; /* mark it ,cause phy-timer didnt init */
	PHY_STOP_TIMER(gpPhyPriv->int_timer) ;
	PHY_STOP_TIMER(gpPhyPriv->phy_ready_timer) ;
	#ifndef TCSUPPORT_CPU_EN7521
	fe_reset_pon_phy_reset_init = NULL;
	#endif
	
	if(!IS_ERR(gpPhyPriv->LDDLA_task_wait)){
		kthread_stop(gpPhyPriv->LDDLA_task_wait);
	}
	
#ifdef TCSUPPORT_CPU_EN7521
	QDMA_API_UNREGISTER_HOOKFUNC(ECNT_QDMA_WAN, QDMA_CALLBACK_XPON_PHY_HANDLER);
#else
	qdma_unregister_callback_function(QDMA_CALLBACK_XPON_PHY_HANDLER) ;	
#endif
	/* transceiver power off*/
	phy_trans_power_switch(PHY_DISABLE);

	#if !defined(CONFIG_USE_MT7520_ASIC) && !defined(CONFIG_USE_A60928) && !defined(CONFIG_USE_A60901)
	i2c_exit() ;
	#endif /* !defined CONFIG_USE_MT7520_ASIC */

	phy_debug_deinit();
	
	if((mt7570_select == 1)||(en7571_select == 1)) //dyma_20170117
	{
    flush_workqueue(i2c_access_queue);
    destroy_workqueue(i2c_access_queue);
	}
	
	if(gpPhyPriv != NULL) {
		kfree(gpPhyPriv) ;
		gpPhyPriv = NULL ;
	}

    ecnt_unregister_hook(&phy_api_dispatch_hook_ops);
}

#ifndef TCSUPPORT_CPU_EN7521
void pon_phy_reset_init(void)
{
#define TEST_MISC0  		(0xBFB00380)
#define XP_PHY_SW_RSTN 	(1<<25)

	uint csrXponSetting=0, rdata=0, wdata=0;

	//csrXponSetting = IO_GPHYREG(PHY_CSR_XPON_SETTING);

#ifdef CONFIG_USE_MT7520_ASIC
	phy_fw_ready(PHY_DISABLE); /*20150413*/
#endif /* CONFIG_USE_MT7520_ASIC */

	/* reset phy use SCU register */
	rdata = IO_GREG(TEST_MISC0);
	wdata = rdata & ~(XP_PHY_SW_RSTN);
	IO_SREG(TEST_MISC0, wdata);
	udelay(1);
	IO_SREG(TEST_MISC0, rdata);
	udelay(1);

	/* reinit phy settings after PHY RESET */
	phy_dev_init();

    phy_trans_model_setting();
	
	//IO_SPHYREG(PHY_CSR_XPON_SETTING, csrXponSetting);

#ifdef CONFIG_USE_MT7520_ASIC
	/*enable FW ready*/
	//phy_fw_ready(PHY_ENABLE) ;
#endif /* CONFIG_USE_MT7520_ASIC */

    if(	PHY_UNKNOWN_CONFIG != gpPhyPriv->phyCfg.flags.mode ) {
        phy_mode_config(gpPhyPriv->phyCfg.flags.mode, PHY_ENABLE);
    }else{
        printk("PON PHY mode unknown while reset! %s:%d\n", __FUNCTION__, __LINE__);
	}
}
#endif

module_init(pon_phy_init)
module_exit(pon_phy_deinit)
MODULE_LICENSE("GPL");

EXPORT_SYMBOL(phy_register_cb_handler);
EXPORT_SYMBOL(phy_unregister_cb_handler);

