/*
* File Name: phy.c
* Description: .
*
******************************************************************/
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36) 
#include <linux/time64.h>
#endif


#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include <asm/tc3162/tc3162.h>

#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_debug.h"
#include "phy_reg_util.h"

#ifdef TCSUPPORT_XPON_LED
#include <asm/tc3162/ledcetrl.h>
#endif
#include "phy_init.h"
#include "phy_tx.h"
#include "phy_reg.h"
#include "a60972_reg.h"

#ifdef TCSUPPORT_CPU_EN7581
#include "en7581.h"
#include "en7581_pma.h"
#include "en7581_reg.h"
#ifdef TCSUPPORT_CPU_AN7583
#include "en7583.h"
//#include "en7583_pma.h"
#include "en7583_reg.h"
#endif
#else
#include "en7580.h"
#include "en7580_pma.h"
#include "en7580_reg.h"	
#endif


#ifdef TCSUPPORT_CPU_AN7583
extern void pon_Ext_T2R (int pon_Spd);
extern void Eth_Ser_Ext_T2R (int xfi_Spd);
extern void Eth_Ser_Ext_R2T (int xfi_Spd);

extern void OLT_Ext_T2R (int xfi_Spd);
extern void Bert_pon_Ext_T2R (int pon_Spd);

extern void pcie_Ext_T2R (int pon_Spd);
extern void phy_eyescan_test(uint bit_sel, int start_p, int sweep_r);
extern void XFI_phy_eyescan_test(uint bit_sel, int start_p, int sweep_r);
extern void Pon_EO_Scan(uint bit_sel, uint EO_start, uint EO_end ,uint PON_EO_Scan);
extern void XFI_EO_Scan(uint bit_sel, uint EO_start, uint EO_end ,uint XFI_EO_Scan);


extern void pon_Ext_R2T (int pon_Spd);

extern void Read_AEQ_Result(void);

extern void pcie_phy_eyescan_test(int start_p, int sweep_r);
extern void pcie_EO_Scan(uint bit_sel, uint EO_start, uint EO_end,uint PXP_EO_Scan);

extern void Pon_Opt_Cal (int mode_Sel , int pat_sel);
#endif


//int isNeedResetTransciver = 0;

/*****************************************************************************
XPON PHY Function lookup table
******************************************************************************/


#ifdef TCSUPPORT_CPU_EN7581 //julia_7583
	#ifdef TCSUPPORT_CPU_AN7583 //julia_7583
	ponPhyFuncTbl* ponPhyFunc=an7583_gepon_func;
	#else
	ponPhyFuncTbl* ponPhyFunc=en7581_gepon_func;
	#endif
#else
	ponPhyFuncTbl* ponPhyFunc=en7580_gepon_func;
#endif

#if ASIC_SERDES
char rxcalib_force_sel = 0x2;
char txcalib_force_sel = 0x1;
char lcpll_a_tdc=0x5;
#endif
uint lddla_set_tx_mode_flag = 0xff; // whliu_20230602

void phy_print_time(void)
{    
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TIME,"|%ds:%03dms:%03dus| ",sec,msec,usec);

}

atomic_t i2c_access_flag = ATOMIC_INIT(1);
// only when i2c_access_flag is enable(1), API:PON_GET_PHY_TRANS_STATUS can access optical modules info. with i2c.

uint flash_matrix[FLASH_MATRIX_SIZE];		// for flash read/write FIR & LDDLA data

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON)
extern int los_status;
#endif/*TCSUPPORT_COMPILE*/
/*****************************************************************************
//function :
//		phy_counter_clear
//description : 
//		this function is used to clear phy counter
//input :	
//		counter_type, defined in phy_def.h (ENUM_PhyCounterClr_T)
//output :
//		N/A
******************************************************************************/



/*****************************************************************************
//function :
//		phy_int_config
//description : 
//		this function is used to config the system's interrupt 
//input :	
//		int_type : interrupt type, defined in phy_def.h  Interrupt
//output :
//		N/A
******************************************************************************/
int phy_int_config(UINT32 mode,UINT32 int_type)
{
	UINT32 config[2]={0};
	config[0] = mode;
	config[1] = int_type;
	
	return ponPhyFunc[PHY_INT_CONFIG_FUNC](&config);
}

/*****************************************************************************
//function :
//		phy_set_traffic_status
//description : 
//		this function is used to set traffic status from MAC layer 
//input :	
//		traffic status : TRAFFIC_STATUS_DOWN / TRAFFIC_STATUS_UP
//output :
//		setting success or failure
******************************************************************************/
int phy_set_traffic_status(ENUM_PhyTrafficStatus  traffic_status)
{
    if (TRAFFIC_STATUS_DOWN != traffic_status && TRAFFIC_STATUS_UP !=  traffic_status)
        return PHY_FAILURE;
    
	gpPhyPriv->phyCfg.flags.trafficStatus  = traffic_status;
	return PHY_SUCCESS ;
}

/*****************************************************************************
//function :
//		phy_ready_status
//description : 
//		this function is used to get the phy ready state of system  
//input :	
//		N/A
//output :
//		PHY_TRUE : the system is now phy ready
//		PHY_FALSE : the system isn't phy ready
******************************************************************************/
int phy_ready_status(void)
{
	return PHY_INTERNAL_GET(PON_GET_PHY_READY_STATUS);
}

/*****************************************************************************
//function :
//		phy_los_status
//description : 
//		this function is used to get the LOS state of system  
//input :	
//		N/A
//output :
//		PHY_LOS_HAPPEN : LOS happen
//		PHY_NO_LOS_HAPPEN: LOS does not happen
******************************************************************************/
int phy_los_status(void)
{
	return PHY_INTERNAL_GET(PON_GET_PHY_LOS_STATUS);
}


/*****************************************************************************
//function :
//		phy_lof_status
//description : 
//		this function is used to get the LOF state of system  
//input :	
//		N/A
//output :
//		PHY_TRUE : LOF happen
//		PHY_FALSE : LOF does not happen or LOF interrupt is not enabled
******************************************************************************/
int phy_lof_status(void)
{
	return PHY_INTERNAL_GET(PON_GET_PHY_LOF_STATUS);
}

/*****************************************************************************
//function :
//		phy_reset_counter
//description : 
//		this function is used to reset phy and reset counter
//input :	
//		N/A
//output :
//		setting success or failure
******************************************************************************/
#if 0
int phy_reset_counter(void) 
{
	return PHY_INTERNAL_SET(PON_SET_PHY_RESET_COUNTER);
}
#endif
int phy_reset_counter(void) 
{
	uint ret = 0;					
#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
		ret=an7583_phy_reset_counter();
	#else
	ret=en7581_phy_reset_counter();
	#endif
#else
	ret=en7580_phy_reset_counter();
#endif
	return ret;
}


/*****************************************************************************
//function :
//		phy_cnt_enable
//description : 
//		this function is used to enable counter statistic 
//input :	
//		errcnt_en : PHY_ENABLE means turn on rx error counter ; PHY_DISABLE means turn off rx error counter
//		bipcnt_en : PHY_ENABLE means turn on rx bip error counter ; PHY_DISABLE means turn off rx bip error counter
//		fmcnt_en : PHY_ENABLE means turn on rx frame counter ; PHY_DISABLE means turn off rx frame counter
//output :
//		setting success or failure
******************************************************************************/

int phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en)
{
	uint ret = 0;
#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
	ret=an7583_phy_cnt_enable(errcnt_en, bipcnt_en,fmcnt_en);
	#else
	ret=en7581_phy_cnt_enable(errcnt_en, bipcnt_en,fmcnt_en);
	#endif
#else
	ret=en7580_phy_cnt_enable(errcnt_en, bipcnt_en,fmcnt_en);
#endif
	return ret;
}



/*****************************************************************************
//function :
//		phy_bit_delay
//description : 
//		this function is used to set bit delay
//input :	
//		delay_value : [3 bits] used to set bit delay value
//output :
//		setting success or failure
******************************************************************************/
int phy_bit_delay(int delay_value)
{
	PHY_INTERNAL_SET_API(PON_SET_PHY_BIT_DELAY,&delay_value);

	return 0;	//ang_20180208
}


/*****************************************************************************
//function :
//		phy_fw_ready
//description : 
//		set fw ready to start HW function
//input :	
//		fwrdy_en : PHY_ENABLE means fw is now ready ; PHY_DISABLE means fw is not ready
//output :
//		setting success or failure
******************************************************************************/
int phy_fw_ready(int fwrdy_en)
{
	uint ret = 0;
#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
	ret=an7583_fw_ready(fwrdy_en);
	#else
	ret=en7581_fw_ready(fwrdy_en);
	#endif
#else
	ret=en7580_fw_ready(fwrdy_en);
#endif
	return ret;
}

void phy_uptime(void)
{
	ktime_t time;
	UINT32 up_time_sec=0;
	UINT8 day, hour, min, sec;
	
	time = ktime_get();
	
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 
	up_time_sec =((UINT32)time.tv.sec - gpPhyPriv->phy_ready_time)>0?((UINT32)time.tv.sec - gpPhyPriv->phy_ready_time):0;
	#else
	up_time_sec =((UINT32)ktime_to_sec(time)- gpPhyPriv->phy_ready_time)>0?((UINT32)ktime_to_sec(time)- gpPhyPriv->phy_ready_time):0;
	#endif
	day = (UINT8)(up_time_sec/86400);
	hour = (UINT8)((UINT32)(up_time_sec/3600)-(24*day));
	min = (UINT8)((UINT32)(up_time_sec/60)-(1440*day)-(60*hour));
	sec = (UINT8)((UINT32)(up_time_sec)-(86400*day)-(3600*hour)-(60*min));
	
	phy_print_time();

	if(is_phy_sync() == PHY_TRUE)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"PHY Uptime: %d day %d:%d:%d	", day, hour, min, sec);
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"PHY Down ! previous Uptime: %d day %d:%d:%d	", day, hour, min, sec);
	}

}

/*****************************************************************************
//function :
//		 pon_phy_status
//description : 
//		this function is used to monitor pon phy status
//input :	
//		N/A
//output :
//		G/EPON MODE, FW RDY, PHY RDY and MT7570 version
******************************************************************************/
void pon_phy_status (void)
{
	uint read_data = 0;
	unchar ptr[4];
	unchar vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1];
	unchar vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1];

	pon_phy_get_wan_sel();


	if(is_phy_sync() == PHY_TRUE)
	{
		printk("PHY SYNC\n");
		phy_uptime();
	}
	else
	{
		printk("PHY Los\n");
	}

	printk("PON PHY driver version is %d(%d)_%s\n",PHY_VERSION,PHY_DATE_VERSION,PHY_VERSION_NOTE);
	printk("LOS status = %d\n", phy_los_status()); // check LOS

	printk("[SW]txPowerEnFlag = %d\n",gpPhyPriv->phyCfg.flags.txPowerEnFlag);
	printk("Rogue onu :det enable(7581) = %d,BEN det en(7583) =%d,rogue onu happen:%d\n",gpPhyPriv->rogue_onu_det_en,gpPhyPriv->rogue_onu_ben_det_en,gpPhyPriv->is_rogue_onu);
}

int pon_phy_get_los_status(void)
{

    xpon_phy_api_data_t phy_api_data = {0};
    
    phy_api_data.api_type = XPON_PHY_API_TYPE_PHY_INTERNAL_GET;
    phy_api_data.cmd_id   = PON_GET_PHY_LOS_STATUS  ;
    phy_api_data.raw      = NULL ;

	ponPhyFunc[PHY_GET_API_DISPATCH_FUNC](&phy_api_data);

    return phy_api_data.ret;
}

int pon_phy_get_trans_status(PHY_Trans_Status_t * trans_status)
{
	if (trans_status!=NULL)
	{
	    phy_trans_param_status_real(&trans_status->params);
	    trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
	    return PHY_SUCCESS;
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d]NULL input\n",__FUNCTION__,__LINE__);
		return PHY_FAILURE;
	}

}

void pon_phy_get_api_dispatch(xpon_phy_api_data_t * api_data)
{
	static unsigned int pre_cmd_id = 0;
	ulong flags;

	if(gpPhyPriv->phy_deinit_flag == TRUE)
		return;

	if(api_data->cmd_id == PON_GET_PHY_TRANS_STATUS)	//the system may be hung, if I2C runs in spon_lock
	{
		phy_trans_param_status_real(&api_data->trasn_status->params);
        api_data->trasn_status->alarms = phy_tx_alarm() | phy_rx_power_alarm(); // get transceiver alarm message, only for transceiver module.

		return;
	}

	//get ngpon2 tx/rx chan, ang_20190605 //julia_20221021 ngpon2
	if(api_data->cmd_id ==  PON_GET_PHY_NGPON2_CHAN) //the system may be hung, if I2C runs in spon_lock
	{
        xfp_trans_get_ngpon2_chan(api_data->ngpon2_chan_sel);
		return;
	}

	spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
	ponPhyFunc[PHY_GET_API_DISPATCH_FUNC](api_data);
	spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);

	if(pre_cmd_id!=api_data->cmd_id)
	{
		PON_PHY_PRINT(PHY_MSG_API,"[%s:%d] type=%d , id=0x%x\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
	}
	pre_cmd_id=api_data->cmd_id;	
}

void pon_phy_set_api_dispatch(xpon_phy_api_data_t * api_data)
{
	static unsigned int pre_cmd_id = 0;
	ulong flags;

	if(gpPhyPriv->phy_deinit_flag == TRUE)
		return;

	if(api_data->cmd_id == PON_SET_PHY_START)	//the system may be hung, if request_irq runs in spon_lock
	{
		xpon_phy_start();
		return;
	}

	if(api_data->cmd_id == PON_SET_PHY_STOP)	//the system may be hung, if request_irq runs in spon_lock
	{
		xpon_phy_stop();
		return;
	}

	if(api_data->cmd_id == PON_SET_PHY_SCU_RST)	//the system may be hung, if stop_timer runs in spon_lock
	{
		PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);   //report LOS
	}
	
	spin_lock_irqsave(&gpPhyPriv->pma_reset_lock, flags);
	ponPhyFunc[PHY_SET_API_DISPATCH_FUNC](api_data);
	spin_unlock_irqrestore(&gpPhyPriv->pma_reset_lock, flags);

	if(pre_cmd_id!=api_data->cmd_id)
	{
		PON_PHY_PRINT(PHY_MSG_API,"[%s:%d] type=%d , id=0x%x\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
	}
	pre_cmd_id=api_data->cmd_id;
}

int pon_phy_api_dispatch(struct ecnt_data *in_data)
{
    xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *)in_data;

	if(FALSE==gpPhyPriv->phy_init_done){
		if((PON_SET_PHY_MODE_CONFIG!=api_data->cmd_id)\
			&&(PHY_SET_ERR_CNT_EN!=api_data->cmd_id)\
			&&(PHY_SET_BIP_CNT_EN!=api_data->cmd_id)\
			&&(PHY_SET_FM_CNT_EN!=api_data->cmd_id)\
			&&(PON_SET_PHY_RESET_COUNTER!=api_data->cmd_id)\
			&&(PON_SET_PHY_START!=api_data->cmd_id)\
			&&(PON_SET_PHY_FW_READY!=api_data->cmd_id)\
			&&(PON_GET_PHY_TRANS_STATUS!=api_data->cmd_id))
		{
#if defined(CONFIG_TP_IMAGE)
			/* remove log */
			//PON_PHY_PRINT(PHY_MSG_DBG,"\r\n[%s:%d]api_type=0x%x,cmd_id=0x%x, WARNING ! xpon_phy NOT init !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
#else
			PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d]api_type=0x%x,cmd_id=0x%x, WARNING ! xpon_phy NOT init !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
#endif
		}
	}

	if((api_data->cmd_id == PON_GET_PHY_TRANS_STATUS)&&(atomic_read(&i2c_access_flag) == 0 )){
		PON_PHY_PRINT(PHY_MSG_DBG,"i2c access is disabled! \n");
		return ECNT_RETURN;
	}

    switch(api_data->api_type) 
    {
        case XPON_PHY_API_TYPE_GET:
		case XPON_PHY_API_TYPE_PHY_INTERNAL_GET:
            pon_phy_get_api_dispatch(api_data);
            break;

        case XPON_PHY_API_TYPE_SET:
		case XPON_PHY_API_TYPE_PHY_INTERNAL_SET:
            pon_phy_set_api_dispatch(api_data);
            break;

        default:
            dump_stack();
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d]unknown api_type: %d\n",__FUNCTION__,__LINE__,api_data->api_type);
            api_data->ret = PHY_NO_API;
            break;
    }
    
    return ECNT_RETURN;
}

/*****************************************************************************
//function :
//		phy_read_proc
//description : 
//		This function is to output EN7570 ADC codes for external DDMI usage
//		Type "cat /proc/pon_phy/debug" to get these codes
//input :	
//		N/A
//output :
//		EN7570 ADC codes
******************************************************************************/
static int phy_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{

	if(FALSE==gpPhyPriv->phy_init_done){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] WARNING ! xpon_phy NOT init !\r\n",__FUNCTION__,__LINE__);
		return -EACCES ;
	}

	printk("PON PHY driver version is %d.%d\n",PHY_DATE_VERSION,PHY_VERSION);

	if(PHY_INTERNAL_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE)
	{
		printk("PHY Ready\n");
		phy_uptime();
	}
	else
	{
		printk("PHY Los\n");
	}

	printk("LOS status = %d\n", PHY_INTERNAL_GET(PON_GET_PHY_LOS_STATUS));

	return 0;
}

/*****************************************************************************
******************************************************************************/
//int xpon_phy_print_open=0;

static int phy_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64], cmd[64] ,subcmd[64];
	uint dec1=0, dec2=0, dec3=0, dec4=0;
	uint hex1=0, hex2=0, hex3=0, hex4=0;	// ang_20180208
	uint input1=0, input2=0, input3=0 ,input4=0;
	int ret;

	uint read_data = 0;
	uint write_data = 0;

	PHY_Rogue_T cmd_rogue_cfg;
	PHY_Xgpon_Profile_Msg_T cmd_xgpon_profile;

	if(FALSE==gpPhyPriv->phy_init_done){
		PON_PHY_PRINT(PHY_MSG_ERR,"\r\n[%s:%d] WARNING ! xpon_phy NOT init !\r\n",__FUNCTION__,__LINE__);
		//trans_dbg return -EACCES ;
	}

	memset(val_string,0,(sizeof(char)*64));
	memset(cmd,0,(sizeof(char)*64));
	memset(subcmd,0,(sizeof(char)*64));

	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;
	sscanf(val_string, "%s %d %d %d %d", 	cmd, &dec1, 	&dec2, 		&dec3,		&dec4);
	sscanf(val_string, "%s %s %d %d %d", 	cmd, subcmd, 	&dec2, 		&dec3,		&dec4);
	sscanf(val_string, "%s %x %x %x %x", 	cmd, &hex1, 	&hex2, 		&hex3,		&hex4);
	sscanf(val_string, "%s %s %x %x %x", 	cmd, subcmd, 	&hex2, 		&hex3,		&hex4);
	sscanf(val_string, "%s %x %d %d %x",	cmd, &input1, 	&input2,	&input3,	&input4);//Roger_a60972
#if 0
	printk("0)%s\r\n",val_string);
	printk("1)%s = %d %d %d %d\r\n",		cmd, dec1, 		dec2, 		dec3, 		dec4);
	printk("2)%s = %s %d %d %d\r\n",		cmd, subcmd, 	dec2, 		dec3, 		dec4);
	printk("3)%s = 0x%x 0x%x 0x%x 0x%x\r\n",cmd, hex1, 		hex2, 		hex3, 		hex4);
	printk("4)%s = %s 0x%x 0x%x 0x%x\r\n",	cmd, subcmd, 	hex2, 		hex3, 		hex4);
	printk("5)%s = 0x%x %d %d 0x%x\r\n",	cmd, input1, 	input2, 	input3, 	input4);
#endif
	if((!strcmp(cmd, "Phy_Int_Config"))||(!strcmp(cmd, "int"))) 
	{
		phy_int_config(hex1,hex2);
		if(PHY_ENABLE == hex1)
		{
			PHY_STOP_TIMER(gpPhyPriv->event_poll_timer); //stop polling timier
		}
		else if(PHY_DISABLE == hex1)
		{
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
		}
	}
	else if(!strcmp(cmd, "polling_stop")) 
	{
		PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
	}
	#if defined(TCSUPPORT_COMBO_PON)	
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
	else if(!strcmp(cmd, "i2c1"))
	{
			if(1 == hex1)
		{
			gpPhyPriv->i2c1_used =1;
			printk("i2c1 enable\n");
		}
		else if(0 == hex1)
		{
			gpPhyPriv->i2c1_used =0;
			printk("i2c1 disable\n");

		}
	}
	#endif
	#endif
	else if(!strcmp(cmd, "polling_start")) 
	{
		//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
		PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	}
	else if(!strcmp(cmd, "dbg")) 
	{
		phy_dbg(dec1,dec2,dec3);
	}
	else if(!strcmp(cmd, "write")) 
	{
		phy_reg_set(hex1,hex2);
	}
	else if(!strcmp(cmd, "read")) 
	{
		phy_reg_get(hex1);
	}
	else if (!strcmp(cmd, "dump")){					
		phy_dump();
	}
	else if (!strcmp(cmd, "pma_dump")){
		#if ASIC_SERDES		
		#ifndef TCSUPPORT_CPU_AN7583 //julia_7583_pma
		pma_dbg_reg_dump();
		#endif
		#endif
	}
	else if (!strcmp(cmd, "trans_dump")){					
		phy_trans_params_dump();
	}
	else if (!strcmp(cmd, "trans_info")){
		phy_trans_params_parser();
	}
	
	else if (!strcmp(cmd, "trans_set")){
		gpPhyPriv->debugLevel|=PHY_MSG_ERR;
		gpPhyPriv->debugLevel|=PHY_MSG_TIME;
		gpPhyPriv->debugLevel|=PHY_MSG_TRANS;
		if (!strcmp(subcmd, "disp")){

			if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel))
			{
				printk("trans_set disp only\n");
				printk("trans_index   = %d\n",gpPhyPriv->trans_index);
				printk("vendor id	  = %s\n",xfp_trans_iot_list[gpPhyPriv->trans_index].vendor_id);
				printk("vendor pn	  = %s\n",xfp_trans_iot_list[gpPhyPriv->trans_index].vendor_pn);
				printk("gepon_setting = 0x%x\n",xfp_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting);
				printk("xgpon_sfp_vld = 0x%x\n",xfp_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level);
				printk("xepon_sfp_sts = 0x%x\n",xfp_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status);
				printk("pma_setting_0 = 0x%x\n",xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0);
				printk("pma_setting_1 = 0x%x\n",xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1);
			}
			else
			{
			printk("trans_set disp only\n");
			printk("trans_index   = %d\n",gpPhyPriv->trans_index);
			printk("vendor id     = %s\n",phy_trans_iot_list[gpPhyPriv->trans_index].vendor_id);
			printk("vendor pn     = %s\n",phy_trans_iot_list[gpPhyPriv->trans_index].vendor_pn);
			#ifdef TCSUPPORT_CPU_AN7583
			printk("tx_setting = 0x%x\n",phy_trans_iot_list_7583[gpPhyPriv->trans_index].tx_setting);
	        printk("rx_setting = 0x%x\n",phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting);
			#else
			printk("gepon_setting = 0x%x\n",phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting);
	        printk("xgpon_sfp_vld = 0x%x\n",phy_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level);
			printk("xepon_sfp_sts = 0x%x\n",phy_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status);
			printk("pma_setting_0 = 0x%x\n",phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0);
			#endif
		}
		}
		else if (!strcmp(subcmd, "auto")){
			printk("trans auto detect\n");
			gpPhyPriv->trans_index=PHY_TRANS_NOT_FOUND_IN_IOT_LIST;
		}
#ifdef TCSUPPORT_MT7570
		else if(!strcmp(subcmd, "7571")){
			gpPhyPriv->en7571_init_done=TRUE;
		}
		#endif
		else if (!strcmp(subcmd, "apply")){
			gpPhyPriv->phy_init_done=FALSE;
			pon_phy_mod_init();
			phy_trans_power_switch(gpPhyPriv->trans_tx_enable);
		}
		else{
			printk("cmd demo:echo trans_set [1g_setting] [xg_setting] [xe_setting] [pma_setting] > /proc/pon_phy/debug\n");
		}
		
		
	}

	else if (!strcmp(cmd, "wan_sel")){					
		#ifdef TCSUPPORT_CPU_ARMV8
		read_data=GET_WAN_CONF();
		printk("Original HW SCU_WAN_CONF_REG_WAN_SEL=%d \n",read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS);
		SET_WAN_CONF(((read_data&SCU_WAN_CONF_REG_WAN_SEL_MASK)|dec1));
		read_data=GET_WAN_CONF();
		printk("PHY cmd Set SCU_WAN_CONF_REG_WAN_SEL=%d\n",read_data);
		#else
		read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
		printk("Original HW SCU_WAN_CONF_REG_WAN_SEL=%d \n",read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS);
		IO_SPHYREG(SCU_WAN_CONF_REG,((read_data&SCU_WAN_CONF_REG_WAN_SEL_MASK)|dec1));
		read_data=IO_GPHYREG(SCU_WAN_CONF_REG);
		printk("#PHY FW# Set SCU_WAN_CONF_REG_WAN_SEL=%d\n",read_data);
		#endif
	}
	else if (!strcmp(cmd, "reset")){					
		xpon_phy_stop();
		gpPhyPriv->phy_init_done=FALSE;
		pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
		xpon_phy_start();
	}
	else if (!strcmp(cmd, "xpon_phy_reset")){
		xPON_Phy_Reset();
	}
	else if(!strcmp(cmd, "report_mac"))
	{
		REPORT_EVENT_TO_MAC(dec1);  //0=los,5=ready
	}
#ifdef TCSUPPORT_CPU_EN7581
	else if (!strcmp(cmd, "sw_rst")){
		
#if 0 //julia_fpga
		read_data=IO_GPHYREG(EN7581_XPON_PMA_SW_RST_SET);
		write_data=read_data&(~(hex1));
		printk("sw_rst 0x%x = 0x%x \n",EN7581_XPON_PMA_SW_RST_SET,write_data);
		udelay(10);
		IO_SPHYREG(EN7581_XPON_PMA_SW_RST_SET,read_data);
		printk("sw_rst 0x%x = 0x%x \n",EN7581_XPON_PMA_SW_RST_SET,read_data);
		if(hex2==1)
		{
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
			printk("fiber_plug_reset(PLUG_OUT) \n");
		}
		if(hex3==1)
		{
			printk("fiber_plug_reset(PLUG_IN) \n");
			fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
		}
#endif
	}
#endif
	else if (!strcmp(cmd, "sw_init")){
		phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
		PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
		//xPON_Phy_Reset();  // only sw init, no scu reset
		gpPhyPriv->phy_init_done=FALSE;
		gpPhyPriv->is_phy_start = FALSE;
		gpPhyPriv->phy_init_done = FALSE; 
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		gpPhyPriv->trans_tx_enable = PHY_DISABLE;
		gpPhyPriv->phy_unexpected_isr_flag=FALSE;
		gpPhyPriv->first_plugin_flag = TRUE; //ang_20180926
		gpPhyPriv->pma_init_done = FALSE;	//ang_20190108
		gpPhyPriv->trans_msg_print_cnt = 95;
		pon_phy_mod_init();
		xpon_phy_start();
		PON_PHY_PRINT(PHY_MSG_ERR,"sw_init\n");
	}
	else if (!strcmp(cmd, "logic_reset")){
		PHY_INTERNAL_SET_API(PON_SET_PHY_LOGIC_RESET,NULL);
		printk("cmd logic_reset\n");
	}
	else if (!strcmp(cmd, "scu_reset")){
		PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
		printk("cmd scu_reset\n");
	}
#ifdef LDDLA_SUPPORT_SET_TX_MODE
	else if (!strcmp(cmd, "lddla_auto")){
		if(dec1 == PHY_ENABLE)
		{
			gpPhyPriv->lddla_auto_mode= TRUE; 
		}
		else if(dec1 == PHY_DISABLE)
		{
			gpPhyPriv->lddla_auto_mode= FALSE;
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR, "invalid input %d (should be 0/1)\n",dec1);
		}
		PON_PHY_PRINT(PHY_MSG_ERR, "cmd set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
	}
#endif
	else if (!strcmp(cmd, "init")){	
		printk("init\n");
		pon_phy_init();   //module init
	}
	else if (!strcmp(cmd, "mode_init")){	
		printk("mode_init\n");
		gpPhyPriv->phy_init_done=FALSE;
		pon_phy_mod_init(); //step 1
	}
	else if (!strcmp(cmd, "start")){					
		printk("start\n");
		xpon_phy_start(); //step 2
	}
	else if (!strcmp(cmd, "stop")){					
		printk("stop\n");
		xpon_phy_stop();
	}
	else if (!strcmp(cmd, "pma_reset")){					
		phy_pma_reset();	// reset PCS RX interface & part of PMA for fiber plug-in/out situations
	}
	#if 0
	else if(!strcmp(cmd, "pma_init")){
		#if ASIC_SERDES
		xpon_pma_mode_init();	// only reset All of PMA (not include PCS) for RX/TX unlock situations
		#endif
		
		#if A60972_SERDES
		if((ret=xpon_pma_init()) != 0) {
			PON_PHY_PRINT(PHY_MSG_ERR, "A60972 init failed.\n") ;
			return ret ;
		}
		#endif
	}
	#endif
	else if(!strcmp(cmd, "first_plug_in")){
		#if ASIC_SERDES
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
		#endif 
		#if A60972_SERDES
		printk("EN7580_FPGA with a60972 Not support this command! \n");
		#endif
	}
	else if(!strcmp(cmd, "plug_in")){
		#if ASIC_SERDES
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
		#endif 
		#if A60972_SERDES
		printk("EN7580_FPGA with a60972 Not support this command! \n");
		#endif
	}
	else if(!strcmp(cmd, "plug_out")){
		#if ASIC_SERDES
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
		#endif
		#if A60972_SERDES
		printk("EN7580_FPGA with a60972 Not support this command! \n");
		#endif
	}
	#if 0
	else if(!strcmp(cmd, "TDC"))
	{
		if (dec1 == 1){
			#ifndef TCSUPPORT_CPU_EN7581 //julia_20220921
			aux_XPON_TDC_on();	//TDC_on & 
			printk("TDC ON!\n");
			#endif
			#ifndef TCSUPPORT_CPU_EN7581//julia_20220921
			printk("EN7580_FPGA with a60972 Not support this command! \n");
			#endif
		}
		else if(dec1 == 0){
			#ifndef TCSUPPORT_CPU_EN7581//julia_20220921
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			printk("TDC OFF!\n");
			#endif 
			#ifndef TCSUPPORT_CPU_EN7581//julia_20220921
			printk("EN7580_FPGA with a60972 Not support this command! \n");
			#endif
		}
	}
	#endif
	else if (!strcmp(cmd, "pcs_if_reset")){
		phy_pcs_fpgaif_reset();
	}
	else if (!strcmp(cmd, "pcs_reset")){
		phy_pcs_reset();
	}
#ifdef TCSUPPORT_CPU_AN7583
	else if (!strcmp(cmd, "pon_Ext_T2R")) {       //Morris_add
		pon_Ext_T2R(hex1);
	}
	else if (!strcmp(cmd, "Eth_Ser_Ext_T2R")) {   //Morris_add
		Eth_Ser_Ext_T2R(hex1);
	}
	else if (!strcmp(cmd, "Eth_Ser_Ext_R2T")) {   //Morris_add
		Eth_Ser_Ext_R2T(hex1);
	}
	else if (!strcmp(cmd, "OLT_Ext_T2R")) {       //Morris_add
		OLT_Ext_T2R(hex1);
	}	
	else if (!strcmp(cmd, "Bert_pon_Ext_T2R")) {  //Morris_add
		Bert_pon_Ext_T2R(hex1);
	}
	else if (!strcmp(cmd, "pcie_Ext_T2R")) {      //Morris_add
		pcie_Ext_T2R(hex1);
	}
	else if (!strcmp(cmd, "phy_eyescan")) {       //Morris_add
		phy_eyescan_test(dec1,dec2, dec3);
	}
	else if (!strcmp(cmd, "XFI_phy_eyescan")) {       //Morris_add
		XFI_phy_eyescan_test(dec1,dec2, dec3);
	}
	else if (!strcmp(cmd, "Pon_EO_Scan")) {       //Morris_add
		Pon_EO_Scan(hex1,0, 7 , hex2);
	}
	else if (!strcmp(cmd, "XFI_EO_Scan")) {       //Morris_add
		XFI_EO_Scan(hex1,0, 7, hex2);
	}
	else if (!strcmp(cmd, "pon_Ext_R2T")) {       //Morris_add
		pon_Ext_R2T(hex1);
	}
	else if (!strcmp(cmd, "Read_AEQ")) {       //Morris_add
		Read_AEQ_Result();
	}
	else if (!strcmp(cmd, "pcie_phy_eyescan")) {       //Morris_add
		pcie_phy_eyescan_test(hex1,hex2);
	}
	else if (!strcmp(cmd, "pcie_EO_Scan")) {       //Morris_add
		pcie_EO_Scan(hex1,0,7, hex2);
	} 
	else if(!strcmp(cmd, "t2r")) { // Morris add
		pon_Ext_T2R(hex1 );
	}
    else if(!strcmp(cmd, "xgpon_cal")) { // Morris add
        Pon_Opt_Cal(hex1,hex2);
	}	
#endif

	#if ASIC_SERDES
#ifndef TCSUPPORT_CPU_EN7581
/*
	else if (!strcmp(cmd, "phy_eyescan")){
		phy_eyescan_test(dec1, dec2, dec3, dec4);
		//dec1:	PHY_EYESCAN_MODE_LIST mode
		//dec2:	int Ovr_sel
		//dec3:	int start_p
		//dec4:	int sweep_r
	}
	*/
	#endif
	#endif
	else if (!strcmp(cmd, "freq")){					
		#if A60972_SERDES //EN7580_ASIC
		#define CRYSTAL_CLK 50
		read_data=IO_GPHYREG(EN7580_IF_RO_INTERFACE_D);
		printk("Read EN7580_IF_RO_INTERFACE_D(0x%08x)= 0x%08x \n",EN7580_IF_RO_INTERFACE_D,read_data); 
		dec1=(read_data&EN7580_IF_RO_INTERFACE_D_RX_MAX)*CRYSTAL_CLK/4096;
		dec2=((read_data&EN7580_IF_RO_INTERFACE_D_RX_MAX)*CRYSTAL_CLK*100/4096)-dec1*100;
		dec3=((read_data&EN7580_IF_RO_INTERFACE_D_TX_MAX)>>16)*CRYSTAL_CLK/4096;
		dec4=(((read_data&EN7580_IF_RO_INTERFACE_D_TX_MAX)>>16)*CRYSTAL_CLK*100/4096)-dec3*100;
		printk("Read        | %3d.%2d | %3d.%2d\n",dec1,dec2,dec3,dec4);
		printk("STANDARD    |  RX    |  TX\n");
		printk("EPON        |  62.5  |  62.5\n");
		printk("XE_10G_1G   | 322.27 |  62.5\n");
		printk("XE_10G_10G  | 322.27 | 322.27\n");
		printk("GPON        | 155.53 |  77.76\n");
		printk("XG_10G_2.5G | 311.04 |  77.76\n");
		printk("XG_10G_10G  | 311.04 | 311.04\n");
		#endif
		
		#if ASIC_SERDES
		//freq_check();
		#endif
	}
	else if (!strcmp(cmd, "get")){	
		if (!strcmp(subcmd, "los")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_LOS_STATUS));
		}
		else if(!strcmp(subcmd, "ready")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_READY_STATUS));
		}
		else if(!strcmp(subcmd, "bip")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_BIP_COUNTER));
		}
		else if(!strcmp(subcmd, "rx_frame_cnt")){
			PHY_FrameCount_T rx_frame_cnt = {0};
			PHY_INTERNAL_GET_API(PON_GET_PHY_RX_FRAME_COUNTER,&rx_frame_cnt);
			printk("high = %d\r\nlow = %d\r\nlof = %d\r\n",rx_frame_cnt.frame_count_high,rx_frame_cnt.frame_count_low,rx_frame_cnt.lof_counter);

		}
		else if(!strcmp(subcmd, "rx_fec_cnt")){
			PHY_FecCount_T rx_fec_cnt = {0};
			PHY_INTERNAL_GET_API(PON_GET_PHY_RX_FEC_COUNTER,&rx_fec_cnt);
			printk("correct_bytes       = %u\r\n",rx_fec_cnt.correct_bytes);
			printk("correct_codewords   = %u\r\n",rx_fec_cnt.correct_codewords);
			printk("uncorrect_codewords = %u\r\n",rx_fec_cnt.uncorrect_codewords);
			printk("total_rx_codewords  = %u\r\n",rx_fec_cnt.total_rx_codewords);
			printk("fec_seconds         = %u\r\n",rx_fec_cnt.fec_seconds);

		}
		else if(!strcmp(subcmd, "tx_frame_cnt")){
			printk("%s = %.8x\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_TX_FRAME_COUNTER));
		}
		else if((!strcmp(subcmd, "rx_fec_status"))||(!strcmp(cmd, "olt_ds_fec_status"))){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_RX_FEC_STATUS));
		}
		else if(!strcmp(subcmd, "rx_fec_get")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_RX_FEC_GETTING));
		}
		else if(!strcmp(subcmd, "tx_fec_status")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_TX_FEC_STATUS));
		}
		else if(!strcmp(subcmd, "sync")){
			printk("%s = %d\r\n",subcmd,PHY_INTERNAL_GET(PON_GET_PHY_IS_SYNC));
		}
		else if(!strcmp(subcmd, "profile")){
			PHY_Xgpon_Profile_Msg_T xgpon_profile = {0};
			PHY_INTERNAL_GET_API(PON_GET_PHY_XGPON_PROFILE,&xgpon_profile);
		}
		else if (!strcmp(subcmd, "trans_ngpon2_chan")){	
			
			//xfp_trans_get_ngpon2_chan(&ngpon2_chan);
			PHY_Ngpon2_Chan_Sel_T ngpon2_chan = {0};
			//PHY_INTERNAL_GET_API(PON_GET_PHY_NGPON2_CHAN,&ngpon2_chan); //julia_20221021 ngpon2
			
			UINT8 ptr[1];
			
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_TX_CHAN_SEL, ptr, 1); //get module identifier sfp/xfp
			
			ngpon2_chan.ngpon2_tx_chan=ptr[0];
			
			mdelay(5);
			
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_RX_CHAN_SEL, ptr, 1); //get module identifier sfp/xfp
			
			ngpon2_chan.ngpon2_rx_chan=ptr[0];
			printk("ngpon2 TX channel = %d \r\n", ngpon2_chan.ngpon2_tx_chan);
			printk("tx channel 0 : 1532.68nm \r\n");
			printk("tx channel 1 : 1533.47nm \r\n");
			printk("tx channel 2 : 1534.25nm \r\n");
			printk("tx channel 3 : 1535.04nm \r\n");
			printk("******************************* \r\n");
			printk("ngpon2 RX channel = %d \r\n", ngpon2_chan.ngpon2_rx_chan);
			printk("rx channel 0 : 1596.34nm \r\n");
			printk("rx channel 1 : 1597.19nm \r\n");
			printk("rx channel 2 : 1598.04nm \r\n");
			printk("rx channel 3 : 1598.89nm \r\n");
		}	
		else
		{
			printk("0x%x = %d\r\n",hex1,PHY_INTERNAL_GET(hex1));
		}
	}
	else if (!strcmp(cmd, "set")){
		if (!strcmp(subcmd, "rxfec")){
			PHY_INTERNAL_SET_API(PON_SET_PHY_RX_FEC_SETTING,&dec2);
		}
		else if (!strcmp(subcmd, "txfec")){
			PHY_INTERNAL_SET_API(PHY_SET_TX_FEC_EN,&dec2);
		}
		else if(!strcmp(subcmd, "rogue")){
			PHY_Rogue_T cmd_rogue_cfg = {0};
			cmd_rogue_cfg.rogue_onoff      = dec2;
			cmd_rogue_cfg.rogue_pattern    = dec3;
			cmd_rogue_cfg.tx_d_in_timeslot = dec4;
			PHY_INTERNAL_SET_API(PON_SET_PHY_ROGUE_PRBS_CONFIG,&cmd_rogue_cfg);
			printk("%s = %d, %d, %d\r\n",subcmd,dec2,dec3,dec4);
		}
		else if(!strcmp(subcmd, "continue")){
			PHY_INTERNAL_SET_API(PON_SET_PHY_TX_BURST_CONFIG,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if(!strcmp(subcmd, "tx_d_padding")){
			PHY_INTERNAL_SET_API(PON_SET_PHY_TX_D_PADDING_CONFIG,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if(!strcmp(subcmd, "delimiter")){
			PHY_INTERNAL_SET_API(PHY_SET_XGPON_PROFILE_DELIMITER,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);

		}
		else if(!strcmp(subcmd, "preamble")){
			PHY_INTERNAL_SET_API(PHY_SET_XGPON_PROFILE_PREAMBLE,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if(!strcmp(subcmd, "psbu_len")){
			write_data=((dec2&0xff)<<16)|((dec3&0xf)<<8)|(dec4&0xf);
			PHY_INTERNAL_SET_API(PHY_SET_XGPON_PROFILE_LEN,&write_data);
			printk("%s:0x%x prmb_rpt=%d, prmb_len=%d, dlmt_len=%d\r\n",subcmd,write_data,dec2,dec3,dec4);
		}
		else if(!strcmp(subcmd, "laser_len")){
			dec2=dec2&0x7ff;
			PHY_INTERNAL_SET_API(PHY_SET_XGPON_LASER_ON_LEN,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if(!strcmp(subcmd, "laser_pattern")){
			dec2=dec2&0xff;
			PHY_INTERNAL_SET_API(PHY_SET_XGPON_LASER_ON_PATTERN,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if(!strcmp(subcmd, "fw_ready")){
			PHY_INTERNAL_SET_API(PON_SET_PHY_FW_READY,&dec2);
			printk("%s = %d\r\n",subcmd,dec2);
		}
		else if (!strcmp(subcmd, "trans_ngpon2_tx_chan")){	

			PHY_Ngpon2_Chan_Sel_T ngpon2_chan = {0};
			ngpon2_chan.ngpon2_tx_chan=dec2;

			PHY_INTERNAL_SET_API(PON_SET_PHY_NGPON2_TX_CHAN_CONFIG,&ngpon2_chan);
			mdelay(20);
			
			PHY_INTERNAL_GET_API(PON_GET_PHY_NGPON2_CHAN,&ngpon2_chan);
			//PHY_INTERNAL_GET_API(PON_GET_PHY_NGPON2_CHAN,&ngpon2_chan);

			//xfp_trans_set_ngpon2_tx_chan(dec2);
			//xfp_trans_get_ngpon2_chan(&ngpon2_chan);

			printk("ngpon2 TX channel = %d \r\n", ngpon2_chan.ngpon2_tx_chan);
			printk("tx channel 0 : 1532.68nm \r\n");
			printk("tx channel 1 : 1533.47nm \r\n");
			printk("tx channel 2 : 1534.25nm \r\n");
			printk("tx channel 3 : 1535.04nm \r\n");

		}
		else if (!strcmp(subcmd, "trans_ngpon2_rx_chan")){	

			PHY_Ngpon2_Chan_Sel_T ngpon2_chan = {0};
			ngpon2_chan.ngpon2_rx_chan=dec2;
			PHY_INTERNAL_SET_API(PON_SET_PHY_NGPON2_RX_CHAN_CONFIG,&ngpon2_chan);
			mdelay(20);
			PHY_INTERNAL_GET_API(PON_GET_PHY_NGPON2_CHAN,&ngpon2_chan);
			//PHY_INTERNAL_GET_API(PON_GET_PHY_NGPON2_CHAN,&ngpon2_chan);

			//xfp_trans_set_ngpon2_rx_chan(dec2);
			//xfp_trans_get_ngpon2_chan(&ngpon2_chan);
			printk("ngpon2 RX channel = %d \r\n", ngpon2_chan.ngpon2_rx_chan);
			printk("rx channel 0 : 1596.34nm \r\n");
			printk("rx channel 1 : 1597.19nm \r\n");
			printk("rx channel 2 : 1598.04nm \r\n");
			printk("rx channel 3 : 1598.89nm \r\n");
		}

		else
		{
			PHY_INTERNAL_SET_API(hex1,&dec2);
			printk("%x = %d\r\n",hex1,dec2);	//ang_20180208
		}
		
	}
	#if A60972_SERDES //EN7580_ASIC
	else if (!strcmp(cmd, "a60972reset")){					
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
		mdelay(1);
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);

	}
	#endif
	else if (!strcmp(cmd, "trans_model_setting")){
	//#if ASIC_SERDES || A60972_SERDES
		gpPhyPriv->trans_index = PHY_TRANS_NOT_FOUND_IN_IOT_LIST;
		phy_trans_model_setting();
	//#endif
	}
	else if(!strcmp(cmd, "Phy_Mode_Config")) 
	{
		phy_mode_config(hex1, PHY_ENABLE);
	}
	else if(!strcmp(cmd, "isPhyReady")) 
	{
		PHY_INTERNAL_GET(PON_GET_PHY_READY_STATUS);
	}
	else if(!strcmp(cmd, "isPhyLOS")) 
	{
		PHY_INTERNAL_GET(PON_GET_PHY_LOS_STATUS);
	}
	else if(!strcmp(cmd, "isPhyLOF")) 
	{
		phy_lof_status();
	}
	else if(!strcmp(cmd, "Phy_Reset_Counter")) 
	{
		phy_reset_counter();
	}
	else if(!strcmp(cmd, "Phy_ErrCnt_Enable")) 
	{
		phy_cnt_enable(hex1,hex2,hex3);
	}
	else if(!strcmp(cmd, "Phy_Bit_Delay")) 
	{
		phy_bit_delay(hex1);
	}
	else if(!strcmp(cmd, "Phy_Gpon_Delimiter_Guard")) 
	{
		PHY_GPON_Delimiter_Guard_t gpon_delimiter_guard = {0};

		gpon_delimiter_guard.delimiter=&hex1;
		gpon_delimiter_guard.guard_time=&hex2;
		PHY_INTERNAL_SET_API(PON_SET_PHY_GPON_DELIMITER_GUARD,&gpon_delimiter_guard);
	}
    else if((!strcmp(cmd, "Phy_Rogue_PRBS"))||(!strcmp(cmd, "rogue")))
    {
		PHY_Rogue_T cmd_rogue_cfg = {0};
		cmd_rogue_cfg.rogue_onoff      = dec1;
		cmd_rogue_cfg.rogue_pattern    = dec2;
		cmd_rogue_cfg.tx_d_in_timeslot = dec3;
		PHY_INTERNAL_SET_API(PON_SET_PHY_ROGUE_PRBS_CONFIG,&cmd_rogue_cfg);
		
		
#ifdef TCSUPPORT_CPU_EN7581//julia_20221003
#if ASIC_SERDES //julia_fpga

		if((PHY_TX_ROGUE_MODE==cmd_rogue_cfg.rogue_onoff)&&(cmd_rogue_cfg.tx_d_in_timeslot==1))
		{
			NCPO_Force(ENABLE);
		}
		else
		{
			NCPO_Force(DISABLE);
		}
		if(PHY_TX_ROGUE_MODE==cmd_rogue_cfg.rogue_onoff)
		{
			if(gpPhyPriv->rogue_state == 0)
				gpPhyPriv->rogueonu_happen++; //julia_20230614
			gpPhyPriv->rogue_state =1;
		}
		else
		{
			
			gpPhyPriv->rogue_state =0; //julia_20230614
		}
#endif
#endif		
		if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_GPON || gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G||(SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM==gpPhyPriv->wan_sel))
		{
			if(cmd_rogue_cfg.rogue_onoff == PHY_TX_ROGUE_MODE){
				REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			}else if(cmd_rogue_cfg.rogue_onoff == PHY_TX_NORMAL_MODE){
				REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
			}else{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1)\n",cmd_rogue_cfg.rogue_onoff);	
			}   
		}
		else
		{
		if((cmd_rogue_cfg.rogue_onoff == PHY_TX_ROGUE_MODE)&&(cmd_rogue_cfg.rogue_pattern<=3)&&(cmd_rogue_cfg.tx_d_in_timeslot<=1)){
			REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
		}else if(cmd_rogue_cfg.rogue_onoff == PHY_TX_NORMAL_MODE){
			REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
		}else{
			PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n", \
				cmd_rogue_cfg.rogue_onoff,cmd_rogue_cfg.rogue_pattern,cmd_rogue_cfg.tx_d_in_timeslot);	
		}   
	}
	}
	else if(!strcmp(cmd, "tx_pattern"))
	{
		PHY_Rogue_T cmd_rogue_cfg = {0};
		cmd_rogue_cfg.rogue_onoff      = dec1;
		cmd_rogue_cfg.rogue_pattern    = dec2;
		cmd_rogue_cfg.tx_d_in_timeslot = 1;

		#if ASIC_SERDES
		if(PHY_RX_CDR_DISABLE == dec3)
		{
			xpon_phy_stop();
			xpon_init(gpPhyPriv->wan_sel);
#ifndef TCSUPPORT_CPU_AN7583 //julia_7583_pma
			xpon_pma_param_opt();
#endif
			gpPhyPriv->first_plugin_flag = TRUE;
			phy_pma_reset();
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
			phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
			PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
			mdelay(350);
		}
		else if(PHY_RX_CDR_ENABLE == dec3)
		{
			xpon_phy_stop();
			gpPhyPriv->phy_init_done=FALSE;
			gpPhyPriv->first_plugin_flag = TRUE; 
			pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
			xpon_phy_start();
		}
		PON_PHY_PRINT(PHY_MSG_ERR,"RX CDR state %d (0=DEFAULT,1=CDR_DISABLE,2=CDR_ENABLE)\n", dec3);
		
		#ifdef TCSUPPORT_CPU_EN7581//julia_20220921
		if(PHY_TX_ROGUE_MODE==cmd_rogue_cfg.rogue_onoff)
		{
			NCPO_Force(ENABLE);
		}
		else
		{
			NCPO_Force(DISABLE);
		}
		#endif
		#endif

		
		PHY_INTERNAL_SET_API(PON_SET_PHY_ROGUE_PRBS_CONFIG,&cmd_rogue_cfg);
		
		if(PHY_TX_ROGUE_MODE==cmd_rogue_cfg.rogue_onoff)
		{
			phy_trans_power_switch(PHY_ENABLE);
		}
		
		
		if((cmd_rogue_cfg.rogue_onoff == PHY_TX_ROGUE_MODE)&&(cmd_rogue_cfg.rogue_pattern<=4)&&(cmd_rogue_cfg.tx_d_in_timeslot==1)){
				REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			}else if(cmd_rogue_cfg.rogue_onoff == PHY_TX_NORMAL_MODE){
				REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
			}else{
			PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=4), in_timeslot %d(must <=1)\n", \
					cmd_rogue_cfg.rogue_onoff,cmd_rogue_cfg.rogue_pattern,cmd_rogue_cfg.tx_d_in_timeslot);	
			}   
		
	}
	else if(!strcmp(cmd, "Phy_Trans_Tx_Setting")) 
	{
		PHY_TransConfig_T trans_config = {0};
		trans_config.trans_tx_sd_inv_status = hex1;
		trans_config.trans_burst_en_inv_status = hex2;
		trans_config.trans_tx_fault_inv_status = hex3;
		PHY_INTERNAL_SET_API(PON_SET_PHY_TRANS_TX_SETTINGS,&trans_config);
	}
	else if(!strcmp(cmd, "Phy_Gpon_Preamble")) 
	{
		PHY_GponPreb_T pon_preb;
		pon_preb.mask = hex1;
		pon_preb.guard_bit_num = 0x10;
		pon_preb.preamble_t1_num = 0x11;
		pon_preb.preamble_t2_num = 0x12;
		pon_preb.preamble_t3_pat = 0x13;
		
		PHY_INTERNAL_SET_API(PON_SET_PHY_GPON_PREAMBLE,&pon_preb);
		
	}
	else if(!strcmp(cmd, "Phy_Gpon_Extend_Preamble")) 
	{
		PHY_GponPreb_T pon_preb;
		pon_preb.mask = hex1;
		pon_preb.t3_O4_preamble = 0x14;
		pon_preb.t3_O5_preamble = 0x15;
		pon_preb.extend_burst_mode = 0x01;
		pon_preb.oper_ranged_st = 0x01;
		pon_preb.dis_scramble = 0x01;
		
		PHY_INTERNAL_SET_API(PON_SET_PHY_GPON_EXTEND_PREAMBLE,&pon_preb);

	}
	else if(!strcmp(cmd, "Phy_Trans_Alarm")) 
	{
		phy_rx_power_alarm();
		phy_tx_alarm();
	}
	else if((!strcmp(cmd, "power_on"))||(!strcmp(cmd, "tx")))
	{
		if(PHY_DISABLE == hex1)
		{
			REPORT_EVENT_TO_MAC(PHY_EVENT_TX_POWER_OFF);
		}
		else if(PHY_ENABLE == hex1)
		{
			REPORT_EVENT_TO_MAC(PHY_EVENT_TX_POWER_ON);
		}	
		phy_tx_power_config(hex1);
		phy_trans_power_switch(hex1);
	}
	else if ((!strcmp(cmd, "pon_phy_status"))||(!strcmp(cmd, "status"))){ 								//by Jay 20150529					
		pon_phy_status();
	}
	else if (!strcmp(cmd, "show_BoB_information")){
	//	gpPhyPriv->trans_status = PHY_INTERNAL_GET(PON_GET_PHY_TRANS_STATUS);
		phy_trans_params_show();
	}
#ifndef TCSUPPORT_CPU_AN7583 

#ifdef TCSUPPORT_CPU_EN7581 
	else if(!strcmp(cmd, "r2t")) 
	{
		pon_Ext_R2T(hex1);
		phy_trans_power_switch(0);
	}
	else if(!strcmp(cmd, "t2r")) 
	{
		pon_Ext_T2R(hex1 );
	}
    else if(!strcmp(cmd, "xgpon_cal")) // Morris add
    {
        Pon_Opt_Cal(hex1,hex2);
	}

#else  // only for 7580
    #ifdef TCSUPPORT_CPU_EN7580
	else if(!strcmp(cmd, "r2t")) 
	{
		Ext_R2T(gpPhyPriv->wan_sel);
		phy_trans_power_switch(0);
	}
	else if(!strcmp(cmd, "t2r")) 
	{
		Ext_T2R(gpPhyPriv->wan_sel);
    }
#endif
#endif
#endif

#if ASIC_SERDES
	else if(!strcmp(cmd, "rx_bist_check")) 
	{
		if(dec3 == 1)	//julia_20230130
		{
			phy_pma_reset();	// reset PCS RX interface & part of PMA for fiber plug-in/out situations				
			xpon_pma_mode_init();	// only reset All of PMA (not include PCS) for RX/TX unlock situations
		}
		
		normal_rx_bist_check(dec1,dec2);
	}
	else if(!strcmp(cmd, "bist_check")) 
	{

		PHY_Rogue_T cmd_rogue_cfg = {0};
		cmd_rogue_cfg.rogue_onoff      = dec1;
		PHY_INTERNAL_SET_API(PON_SET_PHY_ROGUE_PRBS_CONFIG,&cmd_rogue_cfg);

		printk("Turn on TX in rogue mode !\n");

		normal_rx_bist_check(dec1,dec2);
	}
	else if(!strcmp(cmd, "rx_bist_check_cnt")) 
	{
		xpon_rx_bist_recheck_result(dec1,dec2);
	}
	else if(!strcmp(cmd, "lpbk_test"))
	{
		t2r_rx_bist_check(dec1,dec2);
	}
#endif
	else if (!strcmp(cmd, "save_flash_matrix")){
		save_flash_matrix();
	}
	else if (!strcmp(cmd, "get_flash_matrix")){
		get_flash_matrix();
	}
	else if (!strcmp(cmd, "get_flash_register")){
		printk("get_flash_register offset:%x = %x \n", hex1, get_flash_register(hex1));
	}
	else if (!strcmp(cmd, "set_flash_register")){
		set_flash_register(hex1,hex2);
		printk("set_flash_register offset:%x = %x \n", hex2, hex1);
	}
	else if (!strcmp(cmd, "set_flash_register_default")){
		set_flash_register_default();
	}
	else if (!strcmp(cmd, "flash_dump")){
		flash_dump();
	}
		#ifdef TCSUPPORT_CPU_AN7583
	else if (!strcmp(cmd, "shift")){
		uint fec_err_0=0, fec_err_1=0;
		
		//if(dec1 != 0)
		//	IO_SPHYREG(EN7583_DBG_RX_SYNC_2,(dec1*10000)/64);
		fec_err_0 = IO_GPHYREG(EN7581_XGPON_PHY_FEC_UNCORRECTED_CW_CNT);
		
		read_data=IO_GPHYREG(EN7583_DBG_RX_SYNC_0);
		write_data=read_data | (0x6);
		printk("0x1faf0c18 = %x delay = %ld us\r\n",write_data,dec1);		
		printk("0x1faf0c20 = %x \r\n",IO_GPHYREG(EN7583_DBG_RX_SYNC_2));
		//printk("FEC_UNCORRECTED_CW_CNT = %ld\r\n",IO_GPHYREG(EN7581_XGPON_PHY_FEC_UNCORRECTED_CW_CNT));	
		printk("RX Align location = %ld\r\n",IO_GPHYREG(EN7583_DBG_RX_SYNC_1));
		IO_SPHYREG(EN7583_DBG_RX_SYNC_0,write_data);

#if 1
		if(dec1 != 0)
		{
			if(dec1 < 1000)
				phy_delay1us(dec1);
			else
			{
				phy_delay1ms(dec1/1000);
				phy_delay1us(dec1%1000);
			}
			
		}
		
		if(dec2 == 1)
			printk("RX Align location = %ld\r\n",IO_GPHYREG(EN7583_DBG_RX_SYNC_1));
		
		read_data=IO_GPHYREG(EN7583_DBG_RX_SYNC_0);
		write_data=read_data & (~0x6);
		//write_data=write_data & (~0x6);
		IO_SPHYREG(EN7583_DBG_RX_SYNC_0,write_data);
		printk("0x1faf0c18 = %x \r\n",IO_GPHYREG(EN7583_DBG_RX_SYNC_0));

#endif
		phy_delay1ms(200);
		fec_err_1 = IO_GPHYREG(EN7581_XGPON_PHY_FEC_UNCORRECTED_CW_CNT);
		printk("FEC_UNCORRECTED_CW_CNT = %ld (%ld ~ %ld)\r\n",fec_err_1- fec_err_0,fec_err_0,fec_err_1);
				
	}
	
	else if (!strcmp(cmd, "psync_dump")){
		int i = 0;
		int j = 0;	
		
		printk("Psync data :");
		for(i=0; i<16; i++)
		{
			j = 4*i;		
			if(j%16 == 0)
				printk("\n");
			
			printk("0x%08x ", IO_GPHYREG(EN7583_DBG_RX_SYNC_DATA_0+j));
			
		}
		printk("\r\nPsync end\r\n");
	}
	else if(!strcmp(cmd, "rogue_ben_en")){
	
		if(PHY_ENABLE == hex1)
		{
			printk("enable trans_rogue_onu_int.\n");
			gpPhyPriv->rogue_onu_ben_det_en=1;
			an7583_pon_phy_clear_rogueonu_BEN();
			an7583_pon_phy_rogueonu_int_en_BEN(PHY_ENABLE);
		}
		else if(PHY_DISABLE == hex1)
		{
			printk("disable trans_rogue_onu_int.\n");
			gpPhyPriv->rogue_onu_ben_det_en=0;
			an7583_pon_phy_rogueonu_int_en_BEN(PHY_DISABLE);
			an7583_pon_phy_clear_rogueonu_BEN();
		}
		else
		{
			printk("\r\nstatus en: %d\r\n",gpPhyPriv->rogue_onu_det_en);
			printk("\r0x1faf0940: %x\r\n",IO_GPHYREG(0x1faf0940));
			printk("\r0x1faf0970: %x\r\n",IO_GPHYREG(0x1faf0970));
			printk("\r0x1faf097c: %x\r\n",IO_GPHYREG(0x1faf097c));
		}
	}
		#endif
	
#if ASIC_SERDES
	else if(!strcmp(cmd, "rogue_det_en")){

		if(PHY_ENABLE == hex1)
		{
			printk("enable trans_rogue_onu_int.\n");
			gpPhyPriv->rogue_onu_det_en=1;
			pon_phy_clear_rogueonu();
			pon_phy_rogueonu_int_en(PHY_ENABLE);
		}
		else if(PHY_DISABLE == hex1)
		{
			printk("disable trans_rogue_onu_int.\n");
			gpPhyPriv->rogue_onu_det_en=0;
			pon_phy_rogueonu_int_en(PHY_DISABLE);
			pon_phy_clear_rogueonu();
		}	
	}	
	else if (!strcmp(cmd, "pma_fifo_check")){
		printk("------------PMA_FIFO_CHECK------------- \n");
		pma_fifo_check(dec1);
		printk("--------------------------------------- \n");
	}
#endif

	#if ASIC_SERDES //julia_7583_pma
	else if (!strcmp(cmd, "set_pma_fir")){
		printk("--------------set_pma_fir-------------- \n");
		pon_phy_get_wan_sel();
		pma_hi_rate_opt_val = hex1;
		pma_low_rate_opt_val = hex2;
		printk("[CMD] set pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
		printk("[CMD] set pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);
		set_pma_fir();
		get_pma_fir();
		printk("--------------------------------------- \n");
	}
	else if (!strcmp(cmd, "get_pma_fir")){
		printk("--------------get_pma_fir-------------- \n");
		pon_phy_get_wan_sel();
		get_pma_fir();
		printk("--------------------------------------- \n");
	}
	#endif
	
	else if (!strcmp(cmd, "msg")) {											//by Wei.Sun 20150727
		int level = gpPhyPriv->debugLevel ;

		if(!strcmp(subcmd, "act")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_ACT) : (level&~PHY_MSG_ACT) ;
		} else if(!strcmp(subcmd, "int")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_INT) : (level&~PHY_MSG_INT) ;
		} else if(!strcmp(subcmd, "trace")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_TRACE) : (level&~PHY_MSG_TRACE) ;
		} else if(!strcmp(subcmd, "debug")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_DBG) : (level&~PHY_MSG_DBG) ;
		} else if(!strcmp(subcmd, "err")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_ERR) : (level&~PHY_MSG_ERR) ;
		} else if(!strcmp(subcmd, "time")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_TIME) : (level&~PHY_MSG_TIME) ;
		}else if(!strcmp(subcmd, "api")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_API) : (level&~PHY_MSG_API) ;
		}else if(!strcmp(subcmd, "trans")) {
			gpPhyPriv->debugLevel = (hex2) ? (level|PHY_MSG_TRANS) : (level&~PHY_MSG_TRANS) ;
		} else if(!strcmp(subcmd, "disp")) {
			printk("Original ") ;
		} else {
			gpPhyPriv->debugLevel = hex1;
		}
		printk("PHY Debug Level: 0x%x\n", gpPhyPriv->debugLevel) ; 
	}  
	else if (!strcmp(cmd, "i2c_div_clock")) {
		gpPhyPriv->i2c_u2_clk_div = hex1;
	}
	else if (!strcmp(cmd, "i2c_addr_num")){
		gpPhyPriv->i2c_addr_num = hex1;
	}
#if SIF_DEBUG_LEVEL_CONTROL
	else if (!strcmp(cmd, "sif_proc_read")) {
		sifm_proc_read(&hex1);
		//read_data=itoa(hex1);
		printk("%s = 0x%x\r\n",cmd,hex1);
	}
	else if (!strcmp(cmd, "sif_proc_write")) {
		//write_data=atoi(hex1);
		sifm_proc_write(&hex1);
		printk("%s = 0x%x\r\n",cmd,hex1);
	}
#endif
	else if((!strcmp(cmd, "usage"))||(!strcmp(cmd, "?")))
	{
		
	}
	else
	{
		printk("%s => invalid /proc/pon_phy/debug cmd option\n",cmd);
	}

	return count ;
}

#if defined(TCSUPPORT_CPU_EN7581)
/*****************************************************************************
//function :
//		phy_proc_err_read_proc
//description : 
//		This function is to output EN757x BER error counter
//		Type "cat /proc/pon_phy/BER" to get this code
//input :	
//		N/A
//output :
//		LOS status
//Date   :
// 		2017/09/06 by YW
******************************************************************************/
static int phy_proc_err_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;
	uint read_data = 0 ;
	off_t pos=0, begin=0 ;

#if ASIC_SERDES

#ifdef TCSUPPORT_CPU_AN7583 //julia_7583_pma
	if(phy_los_status() == PHY_TRUE)		
		read_data = 0xfffffff;	
	else		
		read_data = AN7583_PMA_Err_Read();
	index += sprintf(buf+ index, "0x1FA8B37C = 0x%x \n",read_data);
	CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;
	
#else //julia_7583_pma
	if(phy_los_status() == PHY_TRUE)		
		read_data = 0xfffffff;	
	else		
	read_data = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RO_RESERVE_1, 31, 0);
	
	index += sprintf(buf+ index, "0x%x = 0x%x \n",EN7581_XPON_PMA_ADD_DIG_RO_RESERVE_1,read_data);
	CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;
	
	//clear bit_error
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 24, 24, 0x1);
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 24, 24, 0x0);

#endif
#endif
	return index ;
}


/*****************************************************************************
//function :
//		phy_proc_rogue_read_proc
//description : 
//		This function is to get EN7581 Rogue onu status function
//		Type "cat /proc/pon_phy/rogue" to get this code
//input :	
//		N/A
//output :
//		enable/disable rogue onu
//Date   :
// 		2017/09/06 by YW
******************************************************************************/

static int phy_proc_rogueonu_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data) //julia_20230614
{
	int index=0 ;
	uint read_data = 0 ;
	off_t pos=0, begin=0 ;

	index += sprintf(buf+ index, "rogueonu %x \n",gpPhyPriv->rogueonu_happen);
	CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;
	
	return index ;
}

/*****************************************************************************
//function :
//		phy_proc_temp_read_proc
//description : 
//		This function is to get lddla temperature 
//		Type "cat /proc/pon_phy/temperature" to get this code
//input :	
//		N/A
//output :
//		lddla temperature 
//Date   :
// 		2017/09/06 by YW
******************************************************************************/

static int phy_proc_temperature_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data) //julia_20230614
{
	
	int index=0 ;
	uint read_data = 0 ;
	off_t pos=0, begin=0 ;

	
	index += sprintf(buf+ index, "temperature 0x%x\n",gpPhyPriv->temprature>>8);
	CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;
	
	return index ;
}


#endif


static struct proc_dir_entry *phy_proc_dir=NULL;
static struct proc_dir_entry *phy_proc=NULL; 
#if defined(TCSUPPORT_CPU_EN7581)
static struct proc_dir_entry *phy_proc_err=NULL;
static struct proc_dir_entry *phy_proc_rogue=NULL;//julia_20230614
static struct proc_dir_entry *phy_proc_temperature=NULL;//julia_20230614


#endif

/*****************************************************************************
******************************************************************************/
int phy_debug_init(void) 
{

	/* create proc node */
	phy_proc_dir = proc_mkdir("pon_phy", NULL);
	if(phy_proc_dir){
		phy_proc = create_proc_entry("debug", 0, phy_proc_dir);
	    if(phy_proc) {
		    phy_proc->write_proc = phy_write_proc;
		    phy_proc->read_proc = phy_read_proc;
	    }
#if defined(TCSUPPORT_CPU_EN7581)
	    phy_proc_err = create_proc_entry("err_cnt", 0, phy_proc_dir);
		if(phy_proc_err) 
		{
			phy_proc_err->read_proc = phy_proc_err_read_proc;
		}

		
	    phy_proc_rogue = create_proc_entry("rogue", 0, phy_proc_dir); //julia_20230614
		if(phy_proc_rogue) 
		{
			phy_proc_rogue->read_proc = phy_proc_rogueonu_read_proc;
		}

		
	    phy_proc_temperature = create_proc_entry("temperature", 0, phy_proc_dir); //julia_20230614
		if(phy_proc_temperature) 
		{
			phy_proc_temperature->read_proc = phy_proc_temperature_read_proc;
		}
		
#endif
	}

	return 0 ;
}

int phy_debug_deinit(void){

	if(phy_proc){
		remove_proc_entry("debug", phy_proc_dir);
	}

#if defined(TCSUPPORT_CPU_EN7581)	
	if (phy_proc_err)
	{
		remove_proc_entry("err_cnt", phy_proc_dir);
	}

	if (phy_proc_rogue)
	{
		remove_proc_entry("rogue", phy_proc_dir); //julia_20230614
	}
	if (phy_proc_temperature)
	{
		remove_proc_entry("temperature", phy_proc_dir); //julia_20230614
	}
#endif
    remove_proc_entry("pon_phy",NULL);
	return 0 ;
}



/*****************************************************************************
 to determine if there is optical signal
******************************************************************************/


#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON)
static int los_status_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len;

	uint read_data = 0;

	
	//read_data = IO_GPHYREG(PHY_CSR_XPON_STA);
	read_data = phy_los_status();
	if( read_data == PHY_TRUE) //rx sd
	{
		los_status = 0;		/* phy los */
	}
	else
	{
		los_status = 1;		/* phy not los */
	}

	len = sprintf(page, "%d\n", los_status);  // 0 = LOS , 1 = not los , inverse regard to result of phy_los_status

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

static int los_status_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{	
	/* do nothing */
	return 0;
}

int los_status_init(void)
{
	struct proc_dir_entry *los_proc = NULL;

	los_proc = create_proc_entry("tc3162/los_status", 0, NULL);
	if(NULL == los_proc)
	{
		printk("\ncreate los_proc proc fail.");
		return -1;
	}

	los_proc->read_proc = los_status_read_proc;
	los_proc->write_proc = los_status_write_proc;

	return 0;
}

#endif/*TCSUPPORT_COMPILE*/

int phy_disable_pcs_tdc(void)
{
	uint ret = 0;			
#ifdef TCSUPPORT_CPU_EN7581
	#ifdef TCSUPPORT_CPU_AN7583
	ret=an7583_disable_pcs_tdc();
	#else
	ret=en7581_disable_pcs_tdc();
	#endif
#else
	ret=en7580_disable_pcs_tdc();
#endif
	return ret;

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
int phy_isr(void)
{
	uint  ret=0;
	static char phy_isr_msg_off=FALSE;

#if	ASIC_SERDES
	/* the following function is used to avoid frequently interrupt */
	if(gpPhyPriv->phyCfg.intCounter == 0) {
		gpPhyPriv->phyCfg.intTime = jiffies ;
	}
	gpPhyPriv->phyCfg.intCounter++ ;
	if(gpPhyPriv->phyCfg.intCounter >= 10) {
		gpPhyPriv->phyCfg.intCounter = 0 ;

		if((jiffies - gpPhyPriv->phyCfg.intTime) < 1000) { //phy interrupt occur 10 times during 1000 system clk
			PON_PHY_PRINT(PHY_MSG_INT, "[%s:%d] detect frequently ISR (0x%x~0x%x), turn off ISR dbg msg!\n",__FUNCTION__,__LINE__,gpPhyPriv->phyCfg.intTime,jiffies) ;

			ret = phy_disable_pcs_tdc();	//ang_20180502
			mdelay(50);
			phy_pma_reset();
#if 0
			if (!phy_los_status())
			{
				gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
				PON_PHY_PRINT(PHY_MSG_INT,"phy_status=0x%x , UNKNOWN \n",gpPhyPriv->phy_status);
			}
#endif
			if(gpPhyPriv->debugLevel&PHY_MSG_INT)
			{
				gpPhyPriv->debugLevel&=~PHY_MSG_INT;
				//gpPhyPriv->debugLevel&=~PHY_MSG_TIME;
				phy_isr_msg_off=TRUE;
			}
		}
		else
		{
			if(TRUE==phy_isr_msg_off)
			{
				gpPhyPriv->debugLevel|=PHY_MSG_INT;
				//gpPhyPriv->debugLevel|=PHY_MSG_TIME;
				phy_isr_msg_off=FALSE;
				PON_PHY_PRINT(PHY_MSG_INT, "[%s:%d] No frequently ISR, turn on ISR dbg msg!\n",__FUNCTION__,__LINE__) ;
			}
		}
	}
#endif

#ifdef TCSUPPORT_CPU_AN7583

	an7583_pon_phy_rogueonu_detect();
					
#endif

	return ponPhyFunc[PHY_ISR_FUNC](gpPhyPriv);
	
}

/*****************************************************************************
//function :
//		phy_event_poll
//description : 
//		this function is used to process phy polling
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/

void phy_event_poll(ulong data)
{
	ponPhyFunc[PHY_EVENT_POLL_FUNC](gpPhyPriv);
	if(gpPhyPriv->event_poll_timer_value <1500) //julia_20220511
	{
		gpPhyPriv->event_poll_timer_value =1500;		
		if((gpPhyPriv->phy_deinit_flag ==FALSE)&&(gpPhyPriv->pon_stop_flag ==FALSE))
		PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	}
}

void phy_event_handler(PON_PHY_Event_data_t * pEvent_data)
{        
	static PHY_Event_Type_t pre_id=PHY_EVENT_MAX_INT;
	ktime_t time;

    PON_PHY_PRINT(PHY_MSG_DBG,"[%s] event:%d, src:%d\n", __FUNCTION__, pEvent_data->id, pEvent_data->src);
    switch(pEvent_data->id)
    {
        case PHY_EVENT_TRANS_LOS_INT      :
        case PHY_EVENT_PHY_ILLG_INT       :
        case PHY_EVENT_TRANS_LOS_ILLG_INT :
            phy_los_handler() ;
			if(PHY_EVENT_PHYRDY_INT==pre_id)
			{
				//phy_uptime();
			}
            break;

        case PHY_EVENT_PHY_LOF_INT:
			if(PHY_EVENT_PHYRDY_INT==pre_id)
			{
				//phy_uptime();
			}
            break;

        case PHY_EVENT_TF_INT:
            /*phy_trans_power_switch(PHY_DISABLE);*/
            break;

        case PHY_EVENT_TRANS_INT:
            break;
	
        case PHY_EVENT_TRANS_SD_FAIL_INT:
            break;
	
        case PHY_EVENT_PHYRDY_INT:
            phy_ready_handler() ;
			if(PHY_EVENT_PHYRDY_INT!=pre_id)
			{
				time = ktime_get();
				#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 
				gpPhyPriv->phy_ready_time = time.tv.sec ;
				#else
				gpPhyPriv->phy_ready_time  =(UINT32) ktime_to_sec(time);
				#endif
				//phy_uptime();
			}
            break;
	
        case PHY_EVENT_I2CM_INT:
            break;

        case PHY_EVENT_NO_LOS_NO_READY:
            break;

        default:
            printk("[%s] event %d left for ISR to handle\n", __FUNCTION__, pEvent_data->id );
            break;
	}

	pre_id=pEvent_data->id;
	
    XPON_MAC_EVENT_HANDLER(pEvent_data);

}

void phy_reg_set(uint addr,uint val) //ang_20180208
{
	IO_SPHYREG(addr, val);
}

int phy_reg_get(int addr)
{
	return IO_GPHYREG(addr);
}

void phy_dbg(char id,char option1,char option2)
{
	char dbg[3]={0};

	dbg[0]=id;
	dbg[1]=option1;
	dbg[2]=option2;

	ponPhyFunc[PHY_DBG_FUNC](&dbg);
}



void phy_dump(void)
{        
	ponPhyFunc[PHY_DUMP_FUNC](gpPhyPriv);
}

/*****************************************************************************
//function :
//		get_flash_register
//description : 
//		this function is used to get the flash_matrix registers
//input :	
//		register offset
//output :
//		register value
//		
******************************************************************************/
uint get_flash_register(int address_offset)
{	
	uint return_uint = 0;
	return_uint = flash_matrix[(address_offset>>2)];
	return return_uint;
}

/*****************************************************************************
//function :
//		set_flash_register
//description : 
//		this function is used to set flash_matrix registers
//input :	
//		register value and register offset
//output :
//		N/A
//		
******************************************************************************/
void set_flash_register(uint reg, uint offset)
{
	flash_matrix[offset>>2] = reg;
}

/*****************************************************************************
//function :
//		set_flash_register_default
//description : 
//		this function is used to set default value to flash_matrix 
//input :	
//		N/A
//output :
//		N/A
//		
******************************************************************************/
void set_flash_register_default(void)
{
	int index = 0;
	for(index = 0; index < FLASH_MATRIX_SIZE; index++)
			flash_matrix[index] = 0xffffffff;

}

/*****************************************************************************
//function :
//		save_flash_matrix
//description : 
//		this function is used to save flash_matrix to pon_phy.conf 
//input :	
//		N/A
//output :
//		0 : success ; -1 : fail
//		
******************************************************************************/
int save_flash_matrix(void)
{
	struct file				*srcf = NULL;
	//UINT32 tmp_fpos = 0;
	char *src = NULL;
	mm_segment_t			orgfs;	
	
	src = "/tmp/7570_bob.conf";
	orgfs = get_fs();//memory file
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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,115) 
			if(ecnt_kernel_fs_write(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos) >0)
#else
			if(srcf->f_op->write(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos) >0)
#endif
			{
				printk("--> write flash_matrix success \n");
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
	return 0;

	error:
	set_fs(orgfs);
	return -1;	
	
}

/*****************************************************************************
//function :
//		get_flash_matrix
//description : 
//		this function is used to read the registers from pon_phy.conf to flash_matrix
//input :	
//		N/A
//output :
//		0 : success ; -1 : fail
//		
******************************************************************************/
int get_flash_matrix(void)
{
	//file open
	struct file 			*srcf = NULL;
	char *src = NULL;
	mm_segment_t			orgfs;

#if/*TCSUPPORT_COMPILE*/ !defined(TCSUPPORT_FH_ENV)		
	src = "/tmp/7570_bob.conf";
#else/*TCSUPPORT_COMPILE*/
	src = "/data/bob/7570_bob.conf";
#endif/*TCSUPPORT_COMPILE*/

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
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,115) 				
				ecnt_kernel_fs_read(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos);
#else
				srcf->f_op->read(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos);
#endif

				filp_close(srcf,NULL);
				printk("--> get flash_matrix success \n");
			}
		}
		set_fs(orgfs);
		return 0;

error:
		set_fs(orgfs);
		return -1;	
	
}

/*****************************************************************************
//Function :
//		flash_dump
//Description : 
//		This function is to show flash_matrix content
//Input :	
//		N/A
//Output :
//		FLASH content
//Date:		
//		2015/01/28 by HC
//
******************************************************************************/
void flash_dump(void)
{
	int i = 0;
	int j = 0;	
	for(i=0; i<FLASH_MATRIX_SIZE; i++)
	{
		j = 4*i;		
		printk("FLASH address 0x%x", j);
		printk("\t");
		printk("0x%08x\n", get_flash_register(j));
	}
}

int phy_create_timer(struct timer_list *timer, phy_timer_callback callback, unsigned long param) // julia_20201017 for kernel 5.4
{
	if((timer == NULL) ||( callback == NULL) ){
		return -1;
	}
	
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
	init_timer(timer);
	timer->function = callback;
#else
	timer_setup(timer, callback, 0);
#endif
	timer->expires= param;

	return 0;
}


//#if ASIC_SERDES
#if 1 //Lucas


uint IO_GPHYA_REG_BITS(UINT32 reg_name,UINT32 end_index,UINT32 start_index)		// for EN7580 ASIC
{
	if((end_index>=start_index)&&(end_index<32))
	{
		if((end_index==31)&&(start_index==0))
		{
			return (IO_GPHYREG(reg_name)) ; 
		}
		else
		{
			return ((IO_GPHYREG(reg_name)>>start_index) & ((1<<(end_index-start_index+1))-1)) ;
		}
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
		return 0;
	}
}

void IO_SPHYA_REG_BITS(UINT32 reg_name,UINT32 end_index,UINT32 start_index,UINT32 value) 	// for EN7580 ASIC
{
	uint data;
	data=IO_GPHYREG(reg_name);
	
	if(0xF1FA==reg_name)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Get REG 0x%x[%d:%d] = 0x%x\r\n",__FUNCTION__,__LINE__,reg_name,end_index,start_index,data);
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Set REG 0x%x[%d:%d] to value 0x%x\r\n",__FUNCTION__,__LINE__,reg_name,end_index,start_index,value);
	}

	if((end_index>=start_index)&&(end_index<32))
	{	
		if((end_index==31)&&(start_index==0))
		{
			IO_SPHYREG(reg_name,value);
		}
		else
		{
			IO_SPHYREG(reg_name,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index))) ;
			if(0xF1FA==reg_name)
			{
				PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Set REG 0x%x[%d:%d] to value 0x%x\r\n",__FUNCTION__,__LINE__,reg_name,end_index,start_index,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index)));
				PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Get REG 0x%x[%d:%d] = 0x%x\r\n",__FUNCTION__,__LINE__,reg_name,end_index,start_index,IO_GPHYREG(reg_name));
			}
		}
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
	}
}
#endif


