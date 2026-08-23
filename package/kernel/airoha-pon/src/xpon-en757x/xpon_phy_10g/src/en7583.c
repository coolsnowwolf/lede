//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : AN7583 PON PHY
//
//******************************************************************************

// -- includes -----------------------------------------------------------------
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#include <linux/timer.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36) 
#include <linux/time64.h>
#endif

#include "phy_global.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy.h"
#include "phy_init.h"
#include "phy_tx.h"
#include "phy_reg.h"
#include "phy_reg_util.h"
#include "i2c.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>

#include "en7581.h"
#include "en7581_pma.h"
#include "en7581_reg.h"
#include "en7583.h"
//#include "en7583_pma.h"
#include "en7583_reg.h"


#if ASIC_SERDES
#include <asm/tc3162/ledcetrl.h>
#endif

#if EN7581_EFUSE
	#ifdef TCSUPPORT_CPU_EN7581
		#include <boot/packageInfo.h>
	#endif
#endif
// -- defines ------------------------------------------------------------------

#define PHY_REG_READ_WRITE_TEST  0
#define PHY_REG_ALL_ZEROS        0x0
#define PHY_REG_ALL_ONES         0xFFFFFFFF

//extern uint pma_hi_rate_opt_val;


#define reversebit(x,y) 		x^=(1<<y)

// -- typedefs -----------------------------------------------------------------


// -- external reference functions--------------------------------------------------



#if 0 //ASIC_SERDES //julia_7583_pma
//julia_20220921
void xpon_rx_bist_recheck_result(uint check_onoff,  uint duration);	//add by david 20180525
int  xpon_bist_check_pattern_sel(XPON_PMA_RX_Bist_Check_t align_pattern);	//add by david 20180525
void t2r_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern);		//add by david 20180525
void normal_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern);		//add by david 20180525
int pma_dbg_reg_dump(void);

#endif
//-----------------------------------------------------------------------


//===============================================================

extern void set_pma_fir(void);
extern void get_pma_fir(void);


	
//========Subroutines declarations=====================================
#define _I_AM_PMA_INIT_SPLIT_LINE_   


// an7583_gepon_func
int an7583_gepon_pon_phy_reset(char* buf);
int an7583_gepon_phy_dump(char* buf);
int an7583_gepon_phy_dev_init(char* buf);
int an7583_gepon_phy_int_config(char* buf);
int an7583_gepon_phy_get_api_dispatch(char* buf);
int an7583_gepon_phy_set_api_dispatch(char* buf);
int an7583_gepon_phy_isr(char* buf);
int an7583_gepon_phy_event_poll(char* buf);
int an7583_gepon_phy_dbg(char* buf);
int an7583_gepon_phy_pma_reset(char* buf);



// an7583_xgpon_func
int an7583_xgpon_pon_phy_reset(char* buf);
int an7583_xgpon_phy_dump(char* buf);
int an7583_xgpon_phy_dev_init(char* buf);
int an7583_xgpon_phy_int_config(char* buf);
int an7583_xgpon_phy_get_api_dispatch(char* buf);
int an7583_xgpon_phy_set_api_dispatch(char* buf);
int an7583_xgpon_phy_isr(char* buf);
int an7583_xgpon_phy_event_poll(char* buf);
int an7583_xgpon_phy_dbg(char* buf);
int an7583_xgpon_phy_pma_reset(char* buf);




// an7583_xepon_func
int an7583_xepon_pon_phy_reset(char* buf);
int an7583_xepon_phy_dump(char* buf);
int an7583_xepon_phy_dev_init(char* buf);
int an7583_xepon_phy_int_config(char* buf);
int an7583_xepon_phy_get_api_dispatch(char* buf);
int an7583_xepon_phy_set_api_dispatch(char* buf);
int an7583_xepon_phy_isr(char* buf);
int an7583_xepon_phy_event_poll(char* buf);
int an7583_xepon_phy_dbg(char* buf);
int an7583_xepon_phy_pma_reset(char* buf);





// --  variables ----------------------------------------------------------------
#if ASIC_SERDES

	XPON_PMA_Param_T xpon_pma_param;
#endif

ponPhyFuncTbl an7583_gepon_func[]={
	an7583_gepon_pon_phy_reset,
	an7583_gepon_phy_dump,
	an7583_gepon_phy_dev_init,
	an7583_gepon_phy_int_config,
	an7583_gepon_phy_get_api_dispatch,
	an7583_gepon_phy_set_api_dispatch,
	an7583_gepon_phy_isr,
	an7583_gepon_phy_event_poll,
	an7583_gepon_phy_dbg,
	an7583_gepon_phy_pma_reset
};

ponPhyFuncTbl an7583_xgpon_func[]={
	an7583_xgpon_pon_phy_reset,
	an7583_xgpon_phy_dump,
	an7583_xgpon_phy_dev_init,
	an7583_xgpon_phy_int_config,
	an7583_xgpon_phy_get_api_dispatch,
	an7583_xgpon_phy_set_api_dispatch,
	an7583_xgpon_phy_isr,
	an7583_xgpon_phy_event_poll,
	an7583_xgpon_phy_dbg,
	an7583_xgpon_phy_pma_reset
};

ponPhyFuncTbl an7583_xepon_func[]={
	an7583_xepon_pon_phy_reset,
	an7583_xepon_phy_dump,
	an7583_xepon_phy_dev_init,
	an7583_xepon_phy_int_config,
	an7583_xepon_phy_get_api_dispatch,
	an7583_xepon_phy_set_api_dispatch,
	an7583_xepon_phy_isr,
	an7583_xepon_phy_event_poll,
	an7583_xepon_phy_dbg,
	an7583_xepon_phy_pma_reset
};


#define _definition_SPLIT_LINE_ 
                                                                               
        
// -- functions  ----------------------------------------------------------------
#if 0
void phy_delay1ms(int ms)
{
#ifdef TCSUPPORT_CPU_ARMV8

	delay1ms(ms);

#else
	volatile uint32 timer_now, timer_last;
	volatile uint32 tick_acc;

	uint32 one_tick_unit = SYS_HCLK * 500;// 1 * SYS_HCLK * 1000 / 2

	volatile uint32 tick_wait = ms * one_tick_unit; 
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

#endif

}

#else
void phy_delay1ms(int ms)
{
	volatile uint32 timer_now=0, timer_last=0;
	volatile uint32 tick_acc=0;

	#ifdef TCSUPPORT_CPU_ARMV8
	uint32 one_tick_unit = getOneMsTick();
	#else
	uint32 one_tick_unit = SYS_HCLK * 500;// 1 * SYS_HCLK * 1000 / 2
	#endif

	volatile uint32 tick_wait = ms * one_tick_unit; 
	volatile uint32 timer1_ldv = 0;

	#ifdef TCSUPPORT_CPU_ARMV8
	timer1_ldv = get_ldv(1);
	timer_last = get_vlr(1);
	#else
	timer1_ldv = regRead32(CR_TIMER1_LDV);
	timer_last = regRead32(CR_TIMER1_VLR);
	#endif
	
	do {
		#ifdef TCSUPPORT_CPU_ARMV8
		timer_now = get_vlr(1);
		#else
		timer_now = regRead32(CR_TIMER1_VLR);
		#endif
	  	if (timer_last >= timer_now) 
	  		tick_acc += (timer_last - timer_now);
		else
			tick_acc += (timer1_ldv - timer_now + timer_last);
		timer_last = timer_now;
	} while (tick_acc < tick_wait);
	
}

void phy_delay1us(int us)
{
	volatile uint32 timer_now=0, timer_last=0;
	volatile uint32 tick_acc=0;

	#ifdef TCSUPPORT_CPU_ARMV8
	uint32 one_tick_unit = getOneUsTick();
	#else
	uint32 one_tick_unit = SYS_HCLK >>1;// 1 * SYS_HCLK * 1000 / 2
	#endif
	
	volatile uint32 tick_wait = us * one_tick_unit; 
	volatile uint32 timer1_ldv = 0;

	#ifdef TCSUPPORT_CPU_ARMV8
	timer1_ldv = get_ldv(1);
	timer_last = get_vlr(1);
	#else
	timer1_ldv = regRead32(CR_TIMER1_LDV);
	timer_last = regRead32(CR_TIMER1_VLR);
	#endif

	printk("time before :%ld \r\n ",get_vlr(1));
	do {
		#ifdef TCSUPPORT_CPU_ARMV8
		timer_now = get_vlr(1);
		#else
		timer_now = regRead32(CR_TIMER1_VLR);
		#endif
	  	if (timer_last >= timer_now) 
	  		tick_acc += (timer_last - timer_now);
		else
			tick_acc += (timer1_ldv - timer_now + timer_last);
		timer_last = timer_now;
	} while (tick_acc < tick_wait);

	
	printk("time end :%ld tick_wait : %ld\r\n ",tick_acc,tick_wait);

}

#endif


#if 0 //ASIC_SERDES
//dump pma reg to cur
int pma_dbg_reg_dump(void){
	
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	uint val = 0;

	phy_reg_total_num=500;
	phy_reg_all=an7583_xpon_pma_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ PMA REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		PON_PHY_PRINT(PHY_MSG_ERR,"%-60s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
	}

	return PHY_SUCCESS;
}

#endif

void an7583_pon_phy_clear_rogueonu_BEN(void) //julia_20240510
{
	uint read_data=0,write_data=0;
	
	#if XILINX_SERDES
	//clear rogue onu alarm
	IO_SPHYA_REG_BITS(EN7583_TX_BEN_CTRL_0 , 5 , 5 , 1 ); //ad BEN alarm clr
	IO_SPHYA_REG_BITS(EN7583_TX_BEN_CTRL_0 , 5 , 5 , 0 );
	
	IO_SPHYA_REG_BITS(EN7583_PCS_SB_STS_0 , 0 , 0 , 1 ); //ad BEN alarm clr
	IO_SPHYA_REG_BITS(EN7583_PCS_SB_STS_0 , 0 , 0 , 0 );
	#endif

	#if ASIC_SERDES
	
	AN7583_Clear_PMA_Rogue();

	
	//2.set 1fa8e960[5] =1,set 1fa8e960[5] =0
	read_data = AN7583_Get_Trans_Brust_Forve();
	write_data = read_data|0x20;
	AN7583_Set_Trans_Brust_Forve(write_data);

	read_data = AN7583_Get_Trans_Brust_Forve();
	write_data = read_data&~0x20;
	AN7583_Set_Trans_Brust_Forve(write_data);
		
	#endif
	
}

void an7583_pon_phy_rogueonu_int_en_BEN(unchar rogue_int_en) //julia_20240510
{	
	uint read_data=0, write_data=0;


#if ASIC_SERDES
	if(PHY_ENABLE == rogue_int_en) 
	{

		//1.read 1fa8e96
		//2.set 1fa8e960[4] =1
		read_data = AN7583_Get_Trans_Brust_Forve();
		write_data = read_data|EN7583_ben_alarm_en;
		AN7583_Set_Trans_Brust_Forve(write_data);
		read_data = AN7583_Get_Trans_Brust_Forve();
		PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e960    :0x%.8x\n", read_data);	
		
		//set 1fa8e280[28] =1
		read_data = AN7583_Get_PON_PMA_XPON_INT_EN_0();
		write_data = read_data|EN7583_TX_BEN_ALARM_INT_ASIC;
		AN7583_Set_PON_PMA_XPON_INT_EN_0(write_data);
		read_data = AN7583_Get_PON_PMA_XPON_INT_EN_0();
		PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e280    :0x%.8x\n", read_data);
	}
	else
	{		
		//2.set 1fa8e960[4] =1
		read_data = AN7583_Get_Trans_Brust_Forve();
		write_data = read_data&~EN7583_ben_alarm_en;
		AN7583_Set_Trans_Brust_Forve(write_data);
		read_data = AN7583_Get_Trans_Brust_Forve();
		PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e960    :0x%.8x\n", read_data);	
		
		//set 1fa8e280[28] =1
		read_data = AN7583_Get_PON_PMA_XPON_INT_EN_0();
		write_data = read_data&~EN7583_TX_BEN_ALARM_INT_ASIC;
		AN7583_Set_PON_PMA_XPON_INT_EN_0(write_data);
		read_data = AN7583_Get_PON_PMA_XPON_INT_EN_0();
		PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e280    :0x%.8x\n", read_data);
	}
#endif

#if XILINX_SERDES
	read_data = IO_GPHYREG(EN7583_PCS_SB_CTRL_0);
	PON_PHY_PRINT(PHY_MSG_TRACE,"EN7583_PCS_SB_CTRL_0     :0x%.8x\n", read_data);

	if(PHY_ENABLE == rogue_int_en) 
	{	
		read_data = AN7583_Get_Trans_Brust_Forve(); //julia_7583_pma
		write_data =  (read_data&0x100)>>8;
		write_data = (~write_data) &0x01;
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 5 , 5 , write_data ); //for tx BEN inv
		IO_SPHYA_REG_BITS(EN7583_TX_BEN_CTRL_0 , 4 , 4 , 1 ); //enable tx BEN alarm interrupt
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 0, 0 , 1 ); //ponphy sb int en 
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 8 , 8 , 1 ); //BEN alarm interrupt
		read_data = IO_GPHYREG(EN7583_PCS_SB_CTRL_0);
		PON_PHY_PRINT(PHY_MSG_INT,"XGPON EN7583_PCS_SB_CTRL_0 write    :0x%.8x\n", read_data);	
	}
	else
	{		
		write_data =  (IO_GPHYREG(EN7583_PCS_SB_CTRL_0)&0x20)>>5;
		write_data = (~write_data) &0x01;
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 5 , 5 , 0 ); //for tx BEN inv
		IO_SPHYA_REG_BITS(EN7583_TX_BEN_CTRL_0 , 4 , 4 , 0 ); //enable tx BEN alarm interrupt
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 0, 0 , 0 ); //ponphy sb int en 
		IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 8 , 8 , 0 ); //BEN alarm interrupt
		read_data = IO_GPHYREG(EN7583_PCS_SB_CTRL_0);
		PON_PHY_PRINT(PHY_MSG_INT,"XGPON EN7583_PCS_SB_CTRL_0 write    :0x%.8x\n", read_data);	
	}
#endif


}

void an7583_pon_phy_rogueonu_detect() //julia_20240510
{
	uint read_data=0, write_data=0;
#if ASIC_SERDES
	if(gpPhyPriv->rogue_onu_ben_det_en)
		{
			//read 1fa8e288
			read_data = AN7583_Get_PON_PMA_XPON_INT_STA_0();

			if(read_data & EN7583_TX_BEN_ALARM_INT_ASIC)
			{
				AN7583_Clear_PMA_Rogue();
				
				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = (read_data&0xffffffde)|EN7583_ben_alarm_clr|EN7583_ben_alarm_en;
				AN7583_Set_Trans_Brust_Forve(write_data);
				read_data = AN7583_Get_Trans_Brust_Forve();
				//PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e960	 :0x%.8x\n", read_data);	

				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = (read_data&0xffffffce)|EN7583_ben_alarm_en;
				AN7583_Set_Trans_Brust_Forve(write_data);
				read_data = AN7583_Get_Trans_Brust_Forve();
				//PON_PHY_PRINT(PHY_MSG_INT,"SET 1fa8e960	 :0x%.8x\n", read_data);	
				gpPhyPriv->is_rogue_onu =1;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_BEN_ROGUE_ONU detected. 0x%08x\r\n",read_data);
				
			}
		}
#endif

#if XILINX_SERDES
		if(gpPhyPriv->rogue_onu_det_en)
			{
				read_data = IO_GPHYREG(EN7583_TX_BEN_STS_1);

				if(read_data & EN7583_TX_BEN_ALARM_INT)
				{
					IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 0, 0 , 0 ); //ponphy sb int en 
					IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 8 , 8 , 0 ); //BEN alarm interrupt
					PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_BEN_ROGUE_ONU detected. 0x%08x\r\n",read_data);
					
				}
			}
#endif
}

int an7583_get_wan_sel(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{

	uint read_data = 0;
	uint write_data = 0;
	
#if defined(TCSUPPORT_COMBO_PON)
	if(gpPhyPriv->combo_pon_enable ==TRUE)
	{

		if(gpPhyPriv->is_phy_mode_detected ==FALSE)
		{
			read_data=GET_WAN_CONF();
			gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
			PON_PHY_PRINT(PHY_MSG_ERR,"SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR,"COMBO:SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);

		}
	}
	else
		{
	
#ifdef TCSUPPORT_CPU_ARMV8
	read_data=GET_WAN_CONF();
	gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
	PON_PHY_PRINT(PHY_MSG_ERR,"SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);
#endif
		}

#else

#ifdef TCSUPPORT_CPU_ARMV8
	read_data=GET_WAN_CONF();
	gpPhyPriv->wan_sel=read_data&SCU_WAN_CONF_REG_WAN_SEL_BITS;
	PON_PHY_PRINT(PHY_MSG_ERR,"SCU_WAN_CONF_REG_WAN_SEL=%d\n",gpPhyPriv->wan_sel);
#endif
#endif


	switch(gpPhyPriv->wan_sel)
	{			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:
			PON_PHY_PRINT(PHY_MSG_ERR,"GPON SYM mode\n");
					break;
					
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"TURBO EPON mode\n");
					break;
					
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"EPON mode\n");
					break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"GPON mode\n");
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"XGPON mode\n");

			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"XGSPON mode\n");
		
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"XEPON_10G_1G mode\n");
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"XEPON_10G_10G mode\n");
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"EN7583_XEPON_1G_1G mode\n");
			
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"NGPON2_10G_10G mode\n");
			
			break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"UNKNOWN mode\n");

			return PHY_FAILURE;
	}


	return PHY_SUCCESS;
}

int an7583_func_init(void)  // (ponPhyFuncTbl** pPonPhyFunc)   * pPonPhyFunc=mt752x_func
{

	uint read_data = 0;

	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x  pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	PON_PHY_PRINT(PHY_MSG_ERR,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);

	//wan conf // should be marked later since it should be configured by sw

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:	
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 GPON SYM mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_gepon_func;
			break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:	
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 TURBO EPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_gepon_func;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:	
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 EPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_gepon_func;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:			
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 GPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_gepon_func;
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 XGPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_xgpon_func;

			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 XGSPON mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_xgpon_func;
		
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 NGPON2_10G_10G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_xgpon_func;
		
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 XEPON_10G_1G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_xepon_func;
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 XEPON_10G_10G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_xepon_func;
			
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] EN7583 EN7583_XEPON_1G_1G mode\n",__FUNCTION__,__LINE__);
			ponPhyFunc=an7583_gepon_func;
			
			break;

		default:
			printk("[%s:%d] UNKNOWN mode = %d\n",__FUNCTION__,__LINE__,gpPhyPriv->wan_sel);

			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}

#if XILINX_SERDES
int en7581_set_trans_setting(void)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int ret;
	
    ret = PHY_SUCCESS;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:	
			if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				#if XILINX_SERDES //for GEPON, BEN is controled by EN7581_FPGA_XILINX_SERDES_CTL
				read_data=IO_GPHYREG(EN7581_GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",EN7581_GEPON_CSR_XPON_SETTING,read_data);
				IO_SPHYREG(EN7581_GEPON_CSR_XPON_SETTING, phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting);
				read_data=IO_GPHYREG(EN7581_GEPON_CSR_XPON_SETTING);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_GEPON_CSR_XPON_SETTING(0x%x)=0x%x \n",EN7581_GEPON_CSR_XPON_SETTING,read_data);

				read_data=IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_CTL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_FPGA_XILINX_SERDES_CTL(0x%x)=0x%x \n",EN7581_FPGA_XILINX_SERDES_CTL,read_data);
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_CTL,phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
				read_data=IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_CTL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_FPGA_XILINX_SERDES_CTL(0x%x)=0x%x \n",EN7581_FPGA_XILINX_SERDES_CTL,read_data);
				
				if( gpPhyPriv->wan_sel== SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON)
				{
					if(phy_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0 == 0)
						IO_SPHYA_REG_BITS(EN7581_FPGA_XILINX_SERDES_RST , 18 , 18 , 1 ); //ad BEN high active
				}
				
				if (!(phy_trans_iot_list[gpPhyPriv->trans_index].gepon_csr_xpon_setting & EN7581_GEPON_CSR_XPON_SETTING_BURST_EN_INV))
				{
					gpPhyPriv->trans_tx_ben_level=HIGH_ACTIVE;
				}
				#endif
				
			}
			break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{				
				#if (XILINX_SERDES)			
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",EN7581_XEPON_PCS_SFP_STATUS,read_data);
				IO_SPHYREG(EN7581_XEPON_PCS_SFP_STATUS,phy_trans_iot_list[gpPhyPriv->trans_index].xepon_pcs_sfp_status) ;
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_XEPON_PCS_SFP_STATUS(0x%x)=0x%x \n",EN7581_XEPON_PCS_SFP_STATUS,read_data);
				#endif
            }
			#if 0 //def LDDLA_SUPPORT_SET_TX_MODE //julia_20240814
			if((82==gpPhyPriv->trans_index)&&(TRUE==gpPhyPriv->lddla_auto_mode))
			{
				LDDLA_SET_TX_MODE(1);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Auto Mode[XE/XES] LDDLA_SET_TX_MODE(1)\n");
			}
			#endif
            break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				// SFP RX valid level
				#if (XILINX_SERDES)
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
				IO_SPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL,phy_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
				#endif

            }
			#if 0 //def LDDLA_SUPPORT_SET_TX_MODE //julia_20240814
			if((82==gpPhyPriv->trans_index)&&(TRUE==gpPhyPriv->lddla_auto_mode))
			{
				LDDLA_SET_TX_MODE(0);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Auto Mode[XG/XGS] LDDLA_SET_TX_MODE(0)\n");
			}
			#endif
            break;

			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G: //julia_20221021 ngpon2
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G: //julia_20221021 ngpon2
			if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				
			}
			break;
					
		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			ret = PHY_FAILURE;
			break;
	}
	
	return ret;
}
#endif

int an7583_set_trans_setting(void)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int ret;
	
    ret = PHY_FAILURE;

	 if((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G)
	 	||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G) 	
	    ||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G))
		{	
			if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				#if ASIC_SERDES
				
					read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);  //julia_7583_pma
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
					IO_SPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL,xfp_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
					read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
					
					read_data=AN7583_Get_PMA_XPON_Setting0();
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read AN7583_PON_PMA_XPON_SETTING_0(0x1fa8E270)=0x%x \n",read_data);
					AN7583_Set_PMA_XPON_Setting0(xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
					read_data=AN7583_Get_PMA_XPON_Setting0();
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set AN7583_PON_PMA_XPON_SETTING_0(0x1fa8E270)=0x%x \n",read_data);
					
					read_data=AN7583_Get_PMA_XPON_Setting1();
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read AN7583_PON_PMA_XPON_SETTING_1(0x1fa8E274)=0x%x \n",read_data);
					AN7583_Set_PMA_XPON_Setting1(xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
					read_data=AN7583_Get_PMA_XPON_Setting1();
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set AN7583_PON_PMA_XPON_SETTING_1(0x1fa8E274)=0x%x \n",read_data);
			#endif
			
			#if 0	//julia_7583_pma		
					read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
					IO_SPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL,xfp_trans_iot_list[gpPhyPriv->trans_index].xgpon_phy_sfp_vld_level) ;
					read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_VLD_LEVEL);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_XGPON_PHY_SFP_VLD_LEVEL(0x%x)=0x%x \n",EN7581_XGPON_PHY_SFP_VLD_LEVEL,read_data);
					
					read_data=IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",EN7581_XPON_PMA_XPON_SETTING_0,read_data);
					IO_SPHYREG(EN7581_XPON_PMA_XPON_SETTING_0,xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_0) ;
					read_data=IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_XPON_PMA_XPON_SETTING_0(0x%x)=0x%x \n",EN7581_XPON_PMA_XPON_SETTING_0,read_data);
					
					read_data=IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7581_XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",EN7581_XPON_PMA_XPON_SETTING_1,read_data);
					IO_SPHYREG(EN7581_XPON_PMA_XPON_SETTING_1,xfp_trans_iot_list[gpPhyPriv->trans_index].pma_xpon_setting_1) ;
					read_data=IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
					PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7581_XPON_PMA_XPON_SETTING_1(0x%x)=0x%x \n",EN7581_XPON_PMA_XPON_SETTING_1,read_data);
			#endif
                        ret = PHY_SUCCESS;
			}			
		}
	 else
	 	{
	 		//tx brust en
			#if XILINX_SERDES
				write_data = (~(phy_trans_iot_list_7583[gpPhyPriv->trans_index].tx_setting))&0x01;
			
			#else
				write_data = phy_trans_iot_list_7583[gpPhyPriv->trans_index].tx_setting;
			#endif
			
			read_data = AN7583_Get_PMA_XPON_Setting0(); //julia_7583_pma
			PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_TRANS_BRUST_INV(0x1FA8E270)=0x%x Table TX:0x%x\n",read_data,phy_trans_iot_list_7583[gpPhyPriv->trans_index].tx_setting);
			write_data = ((read_data &~ 0x100)|(write_data <<8)); //bit 8
			AN7583_Set_PMA_XPON_Setting0(write_data); 
			read_data = AN7583_Get_PMA_XPON_Setting0(); 
			PON_PHY_PRINT(PHY_MSG_TRANS, "Set AN7583_TRANS_BRUST_INV(0x1FA8E270)=0x%x \n",read_data);
			
			#if 0 //julia_7583_pma
			read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
			PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_TRANS_BRUST_INV(0x%x)=0x%x Table TX:0x%x\n",EN7583_TRANS_BRUST_INV,read_data,phy_trans_iot_list_7583[gpPhyPriv->trans_index].tx_setting);
			IO_SPHYA_REG_BITS(EN7583_TRANS_BRUST_INV, 8, 8, write_data); 
			read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
			PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_TRANS_BRUST_INV(0x%x)=0x%x \n",EN7583_TRANS_BRUST_INV,read_data);
	 		#endif
			
	 		//Rx los
			#if XILINX_SERDES
			
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM)
				{
					write_data = phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting;
				}
			else
				{
					write_data = (~(phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting))&0x01;
				}
			#else
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM)
				{
					write_data = (~(phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting))&0x01;
				}
			else
				{
					write_data = phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting;
				}
			#endif
			
			read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
			PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x Table RX:0x%x\n",EN7583_PON_PCS_RX_SD_CTRL,read_data,phy_trans_iot_list_7583[gpPhyPriv->trans_index].rx_setting);
			IO_SPHYA_REG_BITS(EN7583_PON_PCS_RX_SD_CTRL, 0, 0, write_data); 
			read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
			PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x \n",EN7583_PON_PCS_RX_SD_CTRL,read_data);
			
	 		ret = PHY_SUCCESS;
	 	}
	
	return ret;
}


int an7583_phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:	
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:	
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_ERRCNT_MASK;
			if((errcnt_en == PHY_ENABLE) || (errcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (errcnt_en << PHY_ERR_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_BIPCNT_MASK;
			if((bipcnt_en == PHY_ENABLE) || (bipcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (bipcnt_en << PHY_BIP_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_FRAMECNT_MASK;
			if((fmcnt_en == PHY_ENABLE) || (fmcnt_en == PHY_DISABLE))
			{
				write_data = read_data | (fmcnt_en << PHY_FRAME_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, write_data);
			}
			break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			break;
	}
	
	return PHY_SUCCESS;
}

int an7583_phy_reset_counter(void) 
	{
		UINT32 read_data = 0;
	
		switch(gpPhyPriv->wan_sel)
		{
			case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM: 
			case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:	
			case SCU_WAN_CONF_REG_WAN_SEL_EPON:
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			case SCU_WAN_CONF_REG_WAN_SEL_GPON:
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear

				break;
			
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
				IO_SPHYREG(EN7581_XEPON_PCS_CLR_ALL_NUM, EN7581_XEPON_PCS_CLR_ALL_NUM_CLR);
				break;
	
			case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
			case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
			case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
			case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data|EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data&~EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
				break;
	
			default:
				printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);
	
				break;
		}
		
		return PHY_SUCCESS;
	}

int an7583_phy_clear_int(void)
{
	uint phyIntStatus = 0;
	PON_PHY_PRINT(PHY_MSG_TRACE,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);

#if ASIC_SERDES
	AN7583_Clear_PMA_Rogue(); //julia_7583_pma
#if 0 //julia_7583_pma
	// clear PMA rogue int before trans_power_ON
	phyIntStatus=IO_GPHYREG(EN7581_XPON_PMA_XPON_INT_STA_0);
	PON_PHY_PRINT(PHY_MSG_ERR,"EN7581_XPON_PMA_XPON_INT_STA_0= %x\n", phyIntStatus);
	IO_SPHYREG(EN7581_XPON_PMA_XPON_INT_STA_0, phyIntStatus);
	phyIntStatus=IO_GPHYREG(EN7581_XPON_PMA_XPON_INT_STA_0);
	PON_PHY_PRINT(PHY_MSG_ERR,"EN7581_XPON_PMA_XPON_INT_STA_0= %x\n", phyIntStatus);
#endif
#endif
	
	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:	
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			//phyIntStatus=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_STA);
			//IO_SPHYREG(EN7581_GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:			
			phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);	//by ang_20180502
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);	//by ang_20180502
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G: //julia_20220511
			phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);	//by yucheng_20220210
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;	
	
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G: //julia_20220511
			phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);	//by yucheng_20220210
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G: //julia_20220511
			phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);	//by yucheng_20220210
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			/*get interrupt status and clear*/
			phyIntStatus=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
			IO_SPHYREG(EN7581_XEPON_PCS_INT_STATUS, phyIntStatus);
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			/*get interrupt status and clear*/
			phyIntStatus=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
			IO_SPHYREG(EN7581_XEPON_PCS_INT_STATUS, phyIntStatus);
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			//phyIntStatus=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_STA);
			//IO_SPHYREG(EN7581_GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
			break;

		default:			
			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}

/*****************************************************************************
//function :
//		en7581_phy_gpon_mode
//description : 
//		this function is used to read which mode the system work at 
//input :	
//		N/A
//output :
//		1 : means GPON mode ; 0 : means EPON mode
******************************************************************************/
unchar an7583_phy_gpon_mode(void)
{
	uint read_data ;
	
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET10) ;
	return ((read_data&EN7581_GEPON_PHY_GPON_MODE) ? 1 : 0) ;
}

/*****************************************************************************
//function :
//		en7581_phy_set_epon_ts_continue_mode
//description : 
//		this function is used to set burst mode or continue mode
//input :	
//		mode : PHY_ENABLE meas continue mode(phy_burst_en | mac_burst_en); PHY_DISABLE means burst_mode(phy_burst_en)
//output :
//		PHY_SUCCESS
******************************************************************************/
int an7583_phy_set_epon_ts_continue_mode(uint mode) //julia_debug no use
{
	uint read_data = 0;

#if ASIC_SERDES//EN7580_ASIC

	if (mode == PHY_ENABLE)
	{
		read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET10);
		read_data |= (0x01 << 26);
		IO_SPHYREG(EN7581_GEPON_CSR_PHYSET10, read_data);
	}
	else if (mode == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET10);
		read_data &= ~(0x01 << 26);
		IO_SPHYREG(EN7581_GEPON_CSR_PHYSET10, read_data);
	}
#endif

	return PHY_SUCCESS;
}


/*****************************************************************************
//function :
//		en7581_phy_round_trip_delay_sof
//description : 
//		
//input :	
//		void
//output :
//		ushort round_trip_delay_sof
******************************************************************************/

ushort an7583_phy_round_trip_delay_sof(void) //julia_debug no use
{
	  uint read_data = 0;
	  ushort round_trip_delay_sof=0;

	  read_data = IO_GPHYREG(EN7581_GEPON_CSR_ROUND_TRIP_DELAY_VALUE);	  
	  round_trip_delay_sof=read_data&0x0000ffff;	
	  return round_trip_delay_sof;
}


int an7583_fw_ready(int fwrdy_en)
{
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int ret;
	
    ret = PHY_SUCCESS;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM:	
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET2);
			if(fwrdy_en == PHY_ENABLE)
				write_data = read_data & (EN7581_GEPON_PHY_FW_RDY_MASK) | (EN7581_GEPON_PHY_FW_RDY_EN);
			else if(fwrdy_en == PHY_DISABLE)
				write_data = read_data & (EN7581_GEPON_PHY_FW_RDY_MASK);
			else
				ret = PHY_FAILURE;
			IO_SPHYREG(EN7581_GEPON_CSR_PHYSET2,write_data);
			//PON_PHY_PRINT(PHY_MSG_ERR,"en7581_fw_ready %d\n",fwrdy_en);
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] XPON PHY do noting\n",__FUNCTION__,__LINE__);
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			ret = PHY_FAILURE;
			break;
	}
	
	return ret;
}


int an7583_disable_pcs_tdc(void)
{

	uint read_data, write_data;

	PON_PHY_PRINT(PHY_MSG_DBG,"%s mode %d, product id = 0x%x\n",__FUNCTION__,gpPhyPriv->phyCfg.flags.mode,gpPhyPriv->scu_hir_np_sys_hw_id);


	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM: 
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:			
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			break;	
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G: //julia_20220511
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			break;	

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			#if ASIC_SERDES
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
			//disable rx
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
		
			break;

		default:
			
			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	return PHY_SUCCESS;
}

int an7583_set_tx_pattern(int rogue_onoff , int timeslot , int rogue_pattern) //julia_20240607
{	
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	
	PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern=%d, tx_d_in_timeslot=%d\n",rogue_onoff,rogue_pattern,timeslot);
	
	if(rogue_onoff == PHY_TX_ROGUE_MODE)
	{
		gpPhyPriv->phyCfg.flags.rogue = 1;
		if (timeslot == 1)
		{
			if(rogue_pattern<=4)
				{
					read_data = AN7583_Get_PMA_BistCTL_Forve(); //1fa8e210
					write_data = (read_data & EN7581_XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK)|(rogue_pattern+1);
					write_data = (write_data & EN7581_XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK)|0x100;
					AN7583_Set_PMA_BistCTL_Forve(write_data);
				
					read_data = AN7583_Get_PMA_XPON_Mode_1(); //1fa8e414
					write_data = (read_data & EN7581_XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
					write_data = (write_data & EN7581_XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK)|0x10000;				
					AN7583_Set_PMA_XPON_Mode_1(write_data); 	
					
					read_data = AN7583_Get_Trans_Brust_Forve(); //1fa8e960
					write_data = read_data |(0x01); //force BEN on
					AN7583_Set_Trans_Brust_Forve(write_data);
					
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON(PRBS only)\n");
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS9,2=PRBS15,3=PRBS23,4=PRBS31)\n",rogue_pattern);
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=4) while Rogue mode 1 and in_timeslot 1\n",rogue_pattern);
				}
		}
		else if (timeslot == 0)
		{
			if(rogue_pattern<=2)
			{
				
				read_data = AN7583_Get_PMA_TX_Dly_Ctrl();//julia_7583_pma
				write_data = (read_data & EN7581_XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|((rogue_pattern+1)<<28);
				AN7583_Set_PMA_TX_Dly_Ctrl(write_data); 
													
				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = read_data |(0x01); //force BEN on
				AN7583_Set_Trans_Brust_Forve(write_data);
				
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON (PRBS + DATA)\n");
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=free run cnt,1=PRBS9,2=PRBS31)\n",rogue_pattern);
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: pattern %d(must <=2) while Rogue mode 1 and in_timeslot 0\n",rogue_pattern);
				
			}
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR, "error input: in_timeslot %d(must <=1)\n",timeslot);
		}
					
	} 
	else if(rogue_onoff == PHY_TX_NORMAL_MODE)
	{
		gpPhyPriv->phyCfg.flags.rogue = 0;
		
		PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");

		read_data = AN7583_Get_PMA_BistCTL_Forve() ; //1fa8e210
		write_data = (read_data & EN7581_XPON_PMA_BISTCTL_CONTROL_PAT_SEL_MASK);
		write_data = (write_data & EN7581_XPON_PMA_BISTCTL_CONTROL_PAT_TX_EN_MASK);
		AN7583_Set_PMA_BistCTL_Forve(write_data);

		read_data = AN7583_Get_PMA_XPON_Mode_1(); //1fa8e414
		write_data = (read_data & EN7581_XPON_PMA_ADD_XPON_MODE_1_R2T_MODE_MASK);
		write_data = (write_data & EN7581_XPON_PMA_ADD_XPON_MODE_1_TX_BIST_MASK);				
		AN7583_Set_PMA_XPON_Mode_1(write_data);

	#if 0
		read_data = AN7583_Get_PMA_MD32_Clk_Ctrl() ;
		write_data = (read_data & EN7581_XPON_PMA_MD32_MEM_CLK_CTRL_CLK_SEL_MASK)|0x3;
		AN7583_Set_PMA_MD32_Clk_Ctrl(write_data);
	#endif
		
		read_data = AN7583_Get_PMA_TX_Dly_Ctrl(); //1fa8e468
		write_data = (read_data & EN7581_XPON_PMA_TX_DLY_CTRL_OUTBEN_DATA_MASK)|(1<<28);
		AN7583_Set_PMA_TX_Dly_Ctrl(write_data); 
						
		read_data = AN7583_Get_Trans_Brust_Forve();
		write_data = read_data &(0xFFFFFFFE); //not force BEN on 
		AN7583_Set_Trans_Brust_Forve(write_data);
		
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1)\n",rogue_onoff);		
	}
	
	return PHY_SUCCESS;
}

#define _I_AM_GEPON_SPLIT_LINE_                                                
                                                                               
/////////////////////////////////////////////////////////////////////////////////////
// en7581_gepon_func                                                           
/////////////////////////////////////////////////////////////////////////////////////

int an7583_gepon_pon_phy_reset(char* buf)
{
	uint val = 0;
	uint wan_cfg=0;

	PON_PHY_PRINT(PHY_MSG_INT,"\r\nAN7583 << GEPON >> pon_phy_reset\r\n");

	//gating PHY-D ck out
	PON_PMA_PON_CK_SET(0);
	udelay(1);

	//switch wan mode
	wan_cfg=GET_WAN_CONF();
	SET_WAN_CONF(((wan_cfg&0xffffff00)|0x11));

	//gating PHY-A ck out
	val=PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_GET(); 
	PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_SET(val&0xfffffeff); 

	//disable PHY-A ck
	val=PON_PMA_rg_force_da_pxp_txpll_ckout_en_GET(); 
	PON_PMA_rg_force_da_pxp_txpll_ckout_en_SET((val&0xfffeffff)|0x01000000);
	
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
	val |=0x01;
	SET_SCU_RST_RG(val);
	udelay(1);
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
	//release xpon phy top reset 
	val &=~(0x01);
	SET_SCU_RST_RG(val);		
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);		
#else
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
#endif

	SET_WAN_CONF(wan_cfg);

	return PHY_SUCCESS;
}

int an7583_gepon_phy_dump(char* buf)
{
	
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	UINT32 temp=0;
	phy_reg_total_num=121;
	phy_reg_all=en7581_gepon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}

#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		mdelay(10);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
        temp = IO_GPHYREG((phy_reg_all+i)->addr);
        mdelay(10);
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
        mdelay(10);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
        IO_SPHYREG((phy_reg_all+i)->addr,temp);
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif

	return PHY_SUCCESS;
}

int an7583_gepon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);


	/* switch GPIO to XPON mode*/
#ifdef TCSUPPORT_CPU_ARMV8 
	/* switch GPIO to XPON mode*/		
	read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE_7523	
	write_data =(read_data | 0x01); //bit [0]: 1	
	SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE_7523
#else 

	/* switch GPIO to XPON mode*/
	read_data = IO_GREG(IOMUX_Control_1_register);
	read_data = read_data | (GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
	IO_SREG(IOMUX_Control_1_register, read_data);
#endif 	

	

	/*sigdet*/
	//read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	//read_data = read_data & ~(PHY_SQUELCH_DEGLITCH);
	//IO_SPHYREG(PHY_CSR_PHYSET3, read_data);

#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
	/*after sw reset, register can be modify by PBUS*/
	read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
	/* transceiver power initial setting*/
	read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
	read_data = read_data & ~(TOP_LED1_MODE);
	SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
	/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif

	
	
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET10);

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON: 
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			write_data = (read_data & ~EN7581_GEPON_PHY_GPON_MODE);

			//for EN7581, epon fec issue has been solved, remove workaround. ang_20211101
			/*
			if(gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)
			{
				IO_SPHYREG(EN7581_GEPON_CSR_FECDEC_CTL,0x0);
			}
			*/
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM: 
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			write_data = (read_data | EN7581_GEPON_PHY_GPON_MODE);
			break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}

	IO_SPHYREG(EN7581_GEPON_CSR_PHYSET10, write_data);

#if XILINX_SERDES
	if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM)	 //julia_20231009 //julia_debug check
	{
		read_data = IO_GPHYREG(EN7583_GEPON_CSR_PON_CTRL_1_WREN);
		write_data = (read_data | EN7581_GEPON_PHY_GPON_SYM_MODE);
		
		IO_SPHYREG(EN7583_GEPON_CSR_PON_CTRL_1_WREN, write_data);
	}
#endif
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x	0x%8x  *=0x%8x	**=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

	
	/* change the guard time pattern */
	gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);

	//read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3);
	//write_data = read_data |EN7581_GEPON_CSR_PHYSET3_PLL_RST;  // |EN7581_GEPON_CSR_PHYSET3_PHY_RST|EN7581_GEPON_CSR_PHYSET3_SOFTWARE_RST;  //EN7581_GEPON_CSR_PHYSET3_LOS_RST
	//IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3, write_data) ;
	phy_fw_ready(PHY_DISABLE);

	return PHY_SUCCESS ;

}

int an7583_gepon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus;
	
	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		write_data=(EN7581_GEPON_CSR_XPON_PHYRDY_INT_EN|EN7581_GEPON_CSR_XPON_LOF_INT_EN|EN7581_GEPON_CSR_XPON_TRANS_LOS_INT_EN|EN7581_GEPON_CSR_XPON_NO_LOS_INT_EN);
		//write_data=EN7581_GEPON_CSR_XPON_PHYRDY_INT_EN;
		PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}

	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system

	read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config read     :0x%.8x\n", read_data);
	
	IO_SPHYREG(EN7581_GEPON_CSR_XPON_INT_EN, write_data);	
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"GEPON Phy_Int_Config write    :0x%.8x\n", read_data);


	return PHY_SUCCESS;
}

int an7583_gepon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int phyReadyStatus,phyLosStatus;

    api_data->ret = PHY_SUCCESS;
	//PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);

    switch(api_data->cmd_id)
    {
        case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_STA);
			if( (read_data & EN7581_GEPON_CSR_XPON_STA_LOS) == EN7581_GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}
            break;
			
        case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSTA1);//read bit[18:20]
			if(((read_data >> EN7581_GEPON_PHYRDY_OFFSET) & EN7581_GEPON_PHYRDY_MASK) == EN7581_GEPON_PHYRDY_STATUS)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}
            break; 
			
        case PON_GET_PHY_IS_SYNC:
            //api_data->ret = is_phy_sync(); // for this API, in_data is NULL
            
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_STA);
			if( (read_data & EN7581_GEPON_CSR_XPON_STA_LOS) == EN7581_GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
			{
				phyLosStatus= PHY_LOS_HAPPEN;
			}
			else
			{
				phyLosStatus = PHY_NO_LOS_HAPPEN;
			}
			
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSTA1);//read bit[18:20]
			if(((read_data >> EN7581_GEPON_PHYRDY_OFFSET) & EN7581_GEPON_PHYRDY_MASK) == EN7581_GEPON_PHYRDY_STATUS)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				phyReadyStatus= PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				phyReadyStatus = PHY_FALSE;
			}
			
			api_data->ret = ((phyReadyStatus == PHY_TRUE) && (phyLosStatus == PHY_NO_LOS_HAPPEN) );
            break;

        case PON_GET_PHY_MODE:
            api_data->ret = gpPhyPriv->phyCfg.flags.mode;
            break;
            
        case PON_GET_PHY_TX_LONG_FLAG:
            api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;
            break;

        case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
            break;

        case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
            api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;
            break;

        case PON_GET_PHY_BIP_COUNTER:
			  IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, (PHY_BIP_LATCH)); /* latch */
			  api_data->ret = IO_GPHYREG(EN7581_GEPON_CSR_BIP_CNT);
			  /* clear bip error counter */
			  IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);
            break;

        case PON_GET_PHY_RX_FEC_COUNTER:
			IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, (PHY_ERR_LATCH));//latch
			api_data->rx_fec_cnt->correct_bytes = IO_GPHYREG(EN7581_GEPON_CSR_ERR_BYTE_CNT);
			api_data->rx_fec_cnt->correct_codewords = IO_GPHYREG(EN7581_GEPON_CSR_ERR_CODE_CNT);
			api_data->rx_fec_cnt->uncorrect_codewords = IO_GPHYREG(EN7581_GEPON_CSR_NOSOL_CODE_CNT);
			api_data->rx_fec_cnt->total_rx_codewords = IO_GPHYREG(EN7581_GEPON_CSR_RX_CODE_CNT);
			api_data->rx_fec_cnt->fec_seconds = IO_GPHYREG(EN7581_GEPON_CSR_FEC_SECONDS);
            break;

        case PON_GET_PHY_RX_FRAME_COUNTER:
            IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, (PHY_RXFRAME_LATCH));//latch 
			api_data->rx_frame_cnt->frame_count_high = IO_GPHYREG(EN7581_GEPON_CSR_FRAME_CNT_H);
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(EN7581_GEPON_CSR_FRAME_CNT_L);
			if(an7583_phy_gpon_mode())//LOF is only used in GPON mode
				api_data->rx_frame_cnt->lof_counter = IO_GPHYREG(EN7581_GEPON_CSR_LOF_CNT);
	
            break;

        case PON_GET_PHY_RX_FEC_GETTING:
            api_data->ret = IO_GPHYREG(EN7581_GEPON_CSR_FECDEC_CTL);
            break;

        case PON_GET_PHY_RX_FEC_STATUS:
           	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_STATUS);
			if((read_data & EN7581_GEPON_PHY_RX_FEC) == EN7581_GEPON_PHY_RX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC on.\n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC off.\n");
				api_data->ret = PHY_FALSE;
			}
            break;

        case PON_GET_PHY_TX_FEC_STATUS:
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYTX_STATUS);
			if((read_data & EN7581_GEPON_PHY_TX_FEC) == EN7581_GEPON_PHY_TX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
				api_data->ret = PHY_FALSE;
			}
            break;

        case PON_GET_PHY_TX_BURST_GETTING:
#if ASIC_SERDES
			
           	//read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1);
			read_data = AN7583_Get_PMA_XPON_TX_Force_1(); //julia_7583_pma
			if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
	          // 	read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2);
		         read_data = AN7583_Get_PMA_XPON_TX_Force_2(); //julia_7583_pma
					
				if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
					api_data->ret = PHY_TX_CONT_MODE;
				}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
            break;

        case PON_GET_PHY_TRANS_TX:
						
#if ASIC_SERDES //EN7580_ASIC //julia_debug 7583 need recodeing
			read_data = AN7583_Get_PMA_XPON_Setting1(); 
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;
			
			read_data = AN7583_Get_PMA_XPON_Setting0();
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
#endif

			#if 0 //EN7580_ASIC  //julia_7583_pma
			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;
			
			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
            break;

        case PON_GET_PHY_TRANS_RX_GETTING:
			
#if ASIC_SERDES

            read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_SETTING);
			if((read_data & EN7581_GEPON_PHY_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
			#endif
			
            break;

        case PON_GET_PHY_ROUND_TRIP_DELAY:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_ROUND_TRIP_DELAY_VALUE);	  
	  		api_data->ret=read_data&0x0000ffff;

			break;
			
        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(EN7581_LOF);	  
	  		api_data->ret=0;

			break;
			
		case PON_GET_PHY_TX_FRAME_COUNTER:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYTX_TEST_TRIG);
			write_data = read_data | PHY_TX_CNT_LCH;//latch
			IO_SPHYREG(EN7581_GEPON_CSR_PHYTX_TEST_TRIG,write_data);
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_TX_FRAME_COUNTER);
			api_data->ret = read_data;
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Tx Frame Counter : %.8x\n", read_data);
			break;
			
		case PON_GET_PHY_INIT_STATUS:
			api_data->ret = gpPhyPriv->phy_init_done;
			break;
			
        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }
	return PHY_SUCCESS;	//ang_20180208
}

int an7583_gepon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;

    api_data->ret = PHY_SUCCESS;

	
	//PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
	//api_data->ret = PHY_NO_API;

   switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);

		#if ASIC_SERDES
			if(FALSE==gpPhyPriv->pma_init_done){		// incase of pma_tx_ben keeping high
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
			//	printk("==========API: PON_SET_PHY_MODE_CONFIG========= \n");
			}	
		#endif
		
            break;

		case PHY_SET_ERR_CNT_EN:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_ERRCNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_ERR_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;
			
		case PHY_SET_BIP_CNT_EN:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_BIPCNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_BIP_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;

		case PHY_SET_FM_CNT_EN:
			read_data = IO_GPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN);
			read_data = read_data & PHY_FRAMECNT_MASK;
			if(((*api_data->data) == PHY_ENABLE) || ((*api_data->data) == PHY_DISABLE))
			{
				read_data = read_data | ((*api_data->data) << PHY_FRAME_CNT_OFFSET);
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_EN, read_data);
			}
			break;

        case PON_SET_PHY_RESET_COUNTER:
            IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
			IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
			IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
				
            break;

        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET2);
			if((*api_data->data) == PHY_ENABLE)
				write_data = read_data & (EN7581_GEPON_PHY_FW_RDY_MASK) | (EN7581_GEPON_PHY_FW_RDY_EN);
			else if((*api_data->data) == PHY_DISABLE)
				write_data = read_data & (EN7581_GEPON_PHY_FW_RDY_MASK);
			else
				api_data->ret = PHY_FAILURE;
			IO_SPHYREG(EN7581_GEPON_CSR_PHYSET2,write_data);
			//printk("==========API: PON_SET_PHY_FW_READY========= %d\n",(*api_data->data));
            break;
				
		case PON_SET_PHY_COUNTER_CLEAR:
			if(((*api_data->data) & PHY_ERR_CNT_CLR)== PHY_ERR_CNT_CLR)
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_ERR_CLR);//clear
			if(((*api_data->data) & PHY_BIP_CNT_CLR)== PHY_BIP_CNT_CLR)
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);//clear
			if(((*api_data->data) & PHY_RXFRAME_CNT_CLR)== PHY_RXFRAME_CNT_CLR)
				IO_SPHYREG(EN7581_GEPON_CSR_XP_ERRCNT_CTL, PHY_RXFRAME_CLR);//clear
		
			break;

        case PON_SET_EPONMODE_PHY_RESET:

            break;

        case PON_SET_PHY_BIT_DELAY:
			//#if ASIC_SERDES //EN7580_ASIC
            if(((*api_data->data) & EN7581_GEPON_PHY_BIT_DELAY_LEN_MASK) == 0x00)
			{
				read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET5);
				write_data = (read_data & EN7581_GEPON_PHY_BIT_DELAY_MASK) | ((*api_data->data) << EN7581_GEPON_PHY_DIG_BIT_DELAY_OFFSET) | EN7581_GEPON_PHY_TX_BIT_DEL_SEL;
				IO_SPHYREG(EN7581_GEPON_CSR_PHYSET5,write_data);
				return PHY_SUCCESS;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : delay_value. \n");
				return PHY_FAILURE;
			}
			//#endif
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:
			
            #if  XILINX_SERDES
			if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)
			{
        		gpPhyPriv->phyCfg.flags.rogue = 1;
				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_Rogue_PRBS mode %d\n",gpPhyPriv->wan_sel);
				if((SCU_WAN_CONF_REG_WAN_SEL_GPON==gpPhyPriv->wan_sel)||((SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM==gpPhyPriv->wan_sel)))
				{
				
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_MISC) ;
					write_data = read_data | (0x01<<28);
					IO_SPHYREG(EN7581_GEPON_CSR_MISC,write_data);
					PON_PHY_PRINT(PHY_MSG_ERR, "Phy_Rogue_PRBS mode %x\n",write_data);
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_BISTCTL_LOOPBACK_SEL) ;
					write_data = read_data | 0x05;
					IO_SPHYREG(EN7581_GEPON_CSR_BISTCTL_LOOPBACK_SEL,write_data);
            
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_BISTCTL_PRBS_TX_EN) ;
					write_data = read_data | 0x01;				
					IO_SPHYREG(EN7581_GEPON_CSR_BISTCTL_PRBS_TX_EN,write_data);
				}
				else if (SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G==gpPhyPriv->wan_sel)
				{
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3) ;
					write_data = read_data | 0x80;
					IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3,write_data);
				}
			} 
			else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
			{
				gpPhyPriv->phyCfg.flags.rogue = 0;
				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
				if((SCU_WAN_CONF_REG_WAN_SEL_GPON==gpPhyPriv->wan_sel)||((SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM==gpPhyPriv->wan_sel)))
				{
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_MISC) ;
					write_data = read_data & ~(0x01<<28);
					IO_SPHYREG(EN7581_GEPON_CSR_MISC,write_data);			

					read_data = IO_GPHYREG(EN7581_GEPON_CSR_BISTCTL_LOOPBACK_SEL) ;
					write_data = read_data & ~0x05;
					IO_SPHYREG(EN7581_GEPON_CSR_BISTCTL_LOOPBACK_SEL,write_data);

					read_data = IO_GPHYREG(EN7581_GEPON_CSR_BISTCTL_PRBS_TX_EN) ;
					write_data = read_data & ~0x01;				
					IO_SPHYREG(EN7581_GEPON_CSR_BISTCTL_PRBS_TX_EN,write_data);
				}
				else if (SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G==gpPhyPriv->wan_sel)
				{
					read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3) ;
					write_data = read_data & ~0x80;
					IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3,write_data);
				}			
				
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d\n",api_data->phy_rogue_cfg->rogue_onoff);
			}            
			#endif
			
			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
        
			an7583_set_tx_pattern(api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->tx_d_in_timeslot,api_data->phy_rogue_cfg->rogue_pattern);
				
			#endif
            break;

        case PON_SET_PHY_DEV_INIT:
            api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
			
			//julia_7583_pma suggest by dyma	

			PON_PHY_PRINT(PHY_MSG_ERR, "PON_SET_PHY_TRANS_MODEL_SETTING -- 7583 mark\n"); //julia_7583_pma
            break;
			
		case PON_SET_PHY_LOGIC_RESET:

			break;

		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
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
			if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
			{
				IO_SPHYREG(EN7581_GEPON_CSR_FECDEC_CTL,0x1);
			}
			else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
			{
				IO_SPHYREG(EN7581_GEPON_CSR_FECDEC_CTL,0x0);
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW);  //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_INT,"PHY_SCU_RESET\n");
			#endif
			break;
			
        case PON_SET_PHY_RX_FEC_SETTING:
			if(*api_data->data == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
				PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Enable\n");
					IO_SPHYREG(EN7581_GEPON_CSR_FECDEC_CTL,0x1);
				}
			}
			else if(*api_data->data == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				if((gpPhyPriv->scu_hir_np_sys_hw_id == 0xa)&&
				((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_EPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)))
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "EN7580 auto FEC\n");
				}
				else
				{
				PON_PHY_PRINT(PHY_MSG_DBG, "Rx FEC Disable\n");
					IO_SPHYREG(EN7581_GEPON_CSR_FECDEC_CTL,0x0);
				}
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
			}
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
			
#if ASIC_SERDES

			PON_PHY_PRINT(PHY_MSG_ERR, "TX_BURST_CONFIG=%d\n",(*api_data->data));
			if (*api_data->data == PHY_TX_CONT_MODE)
			{

				#if 0 ////julia_7583_BEN_HEC
				#if TCSUPPORT_CPU_EN7581 //julia_20220923 //julia_7583_pma //julia_debug ???
				//SET_FORCE_GPIO32_EN((1<<9),0x200);
				SET_FORCE_GPIO32_EN(EN7581_XPON_SCU_GPIO_MASK,(1|EN7581_XPON_SCU_GPIO_MASK));
				
				read_data=IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
				if((read_data & (1<<8)) == 0) //for BEN high active or low active
				ledTurnOn(LED_PON_TX_BEN);
				else
					ledTurnOff(LED_PON_TX_BEN);
				#else
				read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1);
				write_data = read_data &( ~EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
				read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2);
				write_data = read_data |(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
				#endif
				#endif
				
				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = read_data |(0x01); //force BEN on
				AN7583_Set_Trans_Brust_Forve(write_data);
				
				gpPhyPriv->phyCfg.flags.txLongFlag = ((*api_data->data==PHY_TX_CONT_MODE)?PHY_TRUE:PHY_FALSE);
			
				if (*api_data->data == PHY_TX_BURST_MODE){
					phy_tx_power_config(PHY_ENABLE);
					phy_trans_power_switch(PHY_ENABLE);
				}
				
				api_data->ret = PHY_SUCCESS;
			}
			else if (*api_data->data == PHY_TX_BURST_MODE)
			{
				#if 0 ////julia_7583_BEN_HEC
				#if TCSUPPORT_CPU_EN7581 //julia_20220923 //julia_debug ???
				//ledTurnOff(LED_PON_TX_BEN);
				SET_FORCE_GPIO32_EN(EN7581_XPON_SCU_GPIO_MASK,0);
				#else
				read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1);
				write_data = read_data |(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1, write_data);
				
				read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2);
				write_data = read_data &(~EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
				IO_SPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2, write_data);
				#endif
				//sw workaround
				//read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_STA);
				//if( (read_data & EN7581_GEPON_CSR_XPON_STA_LOS) == EN7581_GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
				//{
					/*reset phy*/
				//	fiber_plug_reset();
				//}
				#endif
				
				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = read_data &(0xFFFFFFFE); //not force BEN on
				AN7583_Set_Trans_Brust_Forve(write_data);
				gpPhyPriv->phyCfg.flags.txLongFlag = ((*api_data->data==PHY_TX_CONT_MODE)?PHY_TRUE:PHY_FALSE);
			
				if (*api_data->data == PHY_TX_BURST_MODE){
					phy_tx_power_config(PHY_ENABLE);
					phy_trans_power_switch(PHY_ENABLE);
				}
				
				api_data->ret = PHY_SUCCESS;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
#endif
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:
			if((api_data->delimiter_guard->delimiter & EN7581_GEPON_PHY_GPON_DELIM_PAT_LEN_MASK) != 0)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "error input : delimiter pattern. \n");
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				write_data = (api_data->delimiter_guard->guard_time << EN7581_GEPON_PHY_GPON_GUARD_PAT_OFFSET) | api_data->delimiter_guard->delimiter;
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Delimiter_Guard write_data:0x%.8x\n", write_data);
				gpPhyPriv->phyGuardBitDelm = write_data;
				IO_SPHYREG(EN7581_GEPON_CSR_GPON_DELIMITER_GUARD, write_data);
				api_data->ret = PHY_SUCCESS;
			}

            break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
			
			#if 0 //ASIC_SERDES //EN7580_ASIC //julia_debug 7583 mark 
            read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
			if((api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_sd_inv_status == PHY_DISABLE) )//tx_sd
				write_data = (read_data & EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_sd_inv_status << EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_sd_inv.\n");
			IO_SPHYREG(EN7581_XPON_PMA_XPON_SETTING_1,write_data);

            read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_burst_en_inv_status == PHY_DISABLE) )//burst_en
				read_data = (read_data & EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK) | (api_data->tx_trans_cfg->trans_burst_en_inv_status<< EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_en_inv.\n");

			if((api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_ENABLE) || (api_data->tx_trans_cfg->trans_tx_fault_inv_status == PHY_DISABLE) )//tx_fault
				read_data = (read_data & EN7581_XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK) | (api_data->tx_trans_cfg->trans_tx_fault_inv_status << EN7581_XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_OFFSET);
			else 
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : tx_fault_inv.\n");

			write_data = read_data;
			IO_SPHYREG(EN7581_XPON_PMA_XPON_SETTING_0,write_data);
			#endif
			
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_SET_PHY_TRANS_TX_SETTINGS -- 7583 mark\n"); //julia_7583_pma
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
			#if 1 //EN7580_ASIC
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_SETTING);//rx_sd
	
			if(((*api_data->data) == PHY_ENABLE))//rx_sd
				write_data = (read_data & ~(EN7581_GEPON_PHY_RX_SD_INV)) | (EN7581_GEPON_PHY_RX_SD_INV);
			else if(((*api_data->data) == PHY_DISABLE))
				write_data = (read_data & ~(EN7581_GEPON_PHY_RX_SD_INV)) | (EN7581_GEPON_PHY_RX_SD);
			else 
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "error input : rx_sd_inv.\n");
				return PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_DBG, "rx_sd_inv success \n");
			IO_SPHYREG(EN7581_GEPON_CSR_XPON_SETTING,write_data);
			#endif
			
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_GPON_PREAMBLE);
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Preamble read_data:0x%.8x\n", read_data);
			if((api_data->gpon_preamble->mask & PHY_GUARD_BIT_NUM_EN) == PHY_GUARD_BIT_NUM_EN )
			{
				read_data = (read_data & EN7581_GEPON_PHY_GUARD_BIT_NUM_MASK) | api_data->gpon_preamble->guard_bit_num;
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T1_NUM_EN) == PHY_PRE_T1_NUM_EN)
			{
				read_data = (read_data & EN7581_GEPON_PHY_PRE_T1_NUM_MAKS) | (api_data->gpon_preamble->preamble_t1_num << EN7581_GEPON_PHY_PRE_T1_NUM_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T2_NUM_EN) == PHY_PRE_T2_NUM_EN)
			{
				read_data = (read_data & EN7581_GEPON_PHY_PRE_T2_NUM_MASK) | (api_data->gpon_preamble->preamble_t2_num << EN7581_GEPON_PHY_PRE_T2_NUM_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_PRE_T3_PAT_EN) == PHY_PRE_T3_PAT_EN)
			{
				read_data = (read_data & EN7581_GEPON_PHY_PRE_T3_PAT_MASK) | (api_data->gpon_preamble->preamble_t3_pat << EN7581_GEPON_PHY_PRE_T3_PAT_OFFSET);
			}
			write_data = read_data;
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Preamble write_data:0x%.8x\n", write_data);
		    gpPhyPriv->phyPreamble = write_data;
			IO_SPHYREG(EN7581_GEPON_CSR_GPON_PREAMBLE, write_data);
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
            read_data = IO_GPHYREG(EN7581_GEPON_CSR_GPON_EXTENDED_PREAMBLE);
			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble read_data:0x%.8x\n", read_data);

			if((api_data->gpon_preamble->mask & PHY_T3_O4_PRE_EN) == PHY_T3_O4_PRE_EN)
				read_data = (read_data & EN7581_GEPON_PHY_T3_O4_PRE_MASK) | api_data->gpon_preamble->t3_O4_preamble;
			if((api_data->gpon_preamble->mask & PHY_T3_O5_PRE_EN) == PHY_T3_O5_PRE_EN)
				read_data = (read_data & EN7581_GEPON_PHY_T3_O5_PRE_MASK) | (api_data->gpon_preamble->t3_O5_preamble << EN7581_GEPON_PHY_T3_O5_PRE_OFFSET);
			if((api_data->gpon_preamble->mask & PHY_EXT_BUR_MODE_EN) == PHY_EXT_BUR_MODE_EN)
			{
				if((api_data->gpon_preamble->extend_burst_mode != PHY_ENABLE) && (api_data->gpon_preamble->extend_burst_mode!= PHY_DISABLE))
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : extend_burst_mode. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & EN7581_GEPON_PHY_EXT_BUR_MODE_MASK) | (api_data->gpon_preamble->extend_burst_mode << EN7581_GEPON_PHY_EXT_BUR_MODE_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_OPER_RANG_EN) == PHY_OPER_RANG_EN)
			{
				if( (api_data->gpon_preamble->oper_ranged_st & EN7581_GEPON_PHY_OPER_RANG_LEG_MASK) != 0x00)
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & EN7581_GEPON_PHY_OPER_RANG_MASK) | (api_data->gpon_preamble->oper_ranged_st<< EN7581_GEPON_PHY_OPER_RANG_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_EXTB_LENG_SEL_EN) == PHY_EXTB_LENG_SEL_EN)
			{
				if( (api_data->gpon_preamble->extb_length_sel & EN7581_GEPON_PHY_EXTB_LENG_SEL_LEN_MASK) != 0x00)
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
					api_data->ret = PHY_FAILURE;
				}
				read_data = (read_data & EN7581_GEPON_PHY_EXTB_LENG_SEL_MASK) | (api_data->gpon_preamble->extb_length_sel<< EN7581_GEPON_PHY_EXTB_LENG_SEL_OFFSET);
			}
			if((api_data->gpon_preamble->mask & PHY_DIS_SCRAM_EN) == PHY_DIS_SCRAM_EN)
			{
				if((api_data->gpon_preamble->dis_scramble != PHY_ENABLE) && (api_data->gpon_preamble->dis_scramble!= PHY_DISABLE))
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input : dis_scramble. \n");
					api_data->ret = PHY_FAILURE;
				}	
				read_data = (read_data & EN7581_GEPON_PHY_DIS_SCRAM_MASK) | (api_data->gpon_preamble->dis_scramble<< EN7581_GEPON_PHY_DIS_SCRAM_OFFSET);
			}
			write_data = read_data;

			//PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble write_data:0x%.8x\n", write_data);
			gpPhyPriv->phyExtPreamble = write_data;
			IO_SPHYREG(EN7581_GEPON_CSR_GPON_EXTENDED_PREAMBLE, write_data);
            break;
			
		case PHY_SET_TX_FEC_EN:
			if((*api_data->data) == PHY_ENABLE)
			{
				read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYTX_MISC);
				write_data = (read_data & EN7581_GEPON_PHY_TX_FEC_MANUAL_MASK) | EN7581_GEPON_PHY_TX_FEC_MANUAL | EN7581_GEPON_PHY_TX_FEC_EN;//clear and set
				IO_SPHYREG(EN7581_GEPON_CSR_PHYTX_MISC,write_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Tx FEC manual enable setting success. \n");
				api_data->ret = PHY_SUCCESS;
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYTX_MISC);
				write_data = (read_data & EN7581_GEPON_PHY_TX_FEC_MANUAL_MASK);//clear setting
				IO_SPHYREG(EN7581_GEPON_CSR_PHYTX_MISC,write_data);
				PON_PHY_PRINT(PHY_MSG_DBG, "Tx FEC manual disable setting success. \n");
				api_data->ret = PHY_SUCCESS;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;


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
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:
			if((*api_data->data) == PHY_TRUE)
			{
				gpPhyPriv->en7571_init_done = TRUE;
			}
			else if((*api_data->data) == PHY_FALSE)
			{
				gpPhyPriv->en7571_init_done = FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "EN7571 Init %s\n",(gpPhyPriv->en7571_init_done==TRUE)?"Done":"Fail");

			break;

		case PON_SET_PHY_RX_CDR:
			
#if ASIC_SERDES

			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				//xpon_pma_param_opt(pma_hi_rate_opt_val);//julia_7583_pma
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }
	return PHY_SUCCESS;	//ang_20180208
}



int an7583_gepon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn ;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,phy_rx_rdy_cnt=0,phy_lof_cnt=0,phy_other_cnt=0,phy_fake_sync_cnt=0;

	static uint trans_laser_detected = FALSE;
	UINT32 read_data=0,read_data1=0;
	UINT32 write_data = 0;

	isr_cnt++;
	
	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_STA);
	IO_SPHYREG(EN7581_GEPON_CSR_XPON_INT_STA_CLR, phyIntStatus);
	
	phy_print_time();

	#if ASIC_SERDES
	if(trans_laser_detected==TRUE)
	{
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_PHYRDY_INT)) 
		{
		    if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)
		    {
		        read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSTA1);	
		        IO_SPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_CNT_CTL, 0xa);
	            read_data1 = IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_SYNC_STATUS);
	            IO_SPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_CNT_CTL, 0x5);
                if((((read_data >> EN7581_GEPON_PHYRDY_OFFSET) & EN7581_GEPON_PHYRDY_MASK) == EN7581_GEPON_PHYRDY_STATUS)&&(read_data1 == 0x0))
                {
			        phy_rdy_cnt++;
		            PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY EPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			        gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		            #ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			            handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
		            #endif
			        trans_laser_detected=FALSE;
                }
			    else
			    { 
			        phy_fake_sync_cnt++;
				    gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
				    PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY LINK is not stable! GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);	
			    }
		    }
			else
			{
			    phy_rdy_cnt++;
		        PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			    gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		        #ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			        handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
		        #endif
			    trans_laser_detected=FALSE;
			}
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when laser on ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			trans_laser_detected=FALSE;
			phy_fw_ready(PHY_DISABLE);
			#if ASIC_SERDES	// for ASIC
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
		} 
		else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when ready ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
			trans_laser_detected=FALSE;

			
			//read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3);
			//write_data = read_data |EN7581_GEPON_CSR_PHYSET3_PLL_RST;	// |EN7581_GEPON_CSR_PHYSET3_PHY_RST|EN7581_GEPON_CSR_PHYSET3_SOFTWARE_RST;  //EN7581_GEPON_CSR_PHYSET3_LOS_RST
			//IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3, write_data) ;
			phy_fw_ready(PHY_DISABLE);
			#if ASIC_SERDES	// for ASIC
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			#endif
		} 
		else
		{   
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when laser on fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
                }
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT))
	{
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif
		
		//read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3);
		//write_data = read_data |EN7581_GEPON_CSR_PHYSET3_PLL_RST;	// |EN7581_GEPON_CSR_PHYSET3_PHY_RST|EN7581_GEPON_CSR_PHYSET3_SOFTWARE_RST;  //EN7581_GEPON_CSR_PHYSET3_LOS_RST
		//IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3, write_data) ;
		
		phy_fw_ready(PHY_DISABLE);
		#if ASIC_SERDES	// for ASIC
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
		#endif
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_NO_LOS_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		#if ASIC_SERDES
		mdelay(10);

		phy_pma_reset();
		#else		
		IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_SERDES);
		IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE);
		#endif
		trans_laser_detected = TRUE;
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes Done\r\n");
	}
	else if(phyIntStatus & EN7581_GEPON_CSR_XPON_LOF_INT)  // LOF may happen after LOS
	{
		phy_lof_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x,phyrx_status=0x%x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus,IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_STATUS));
	}
	else
    {
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! laser=0x%x, fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",trans_laser_detected,gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
	}
	
#endif


#if XILINX_SERDES
	if((SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G==gpPhyPriv->wan_sel)||(SCU_WAN_CONF_REG_WAN_SEL_EPON==gpPhyPriv->wan_sel)) //for epon lof case
	{
		 if(phyIntStatus & EN7581_GEPON_CSR_XPON_LOF_INT) 
		 {
			 phy_lof_cnt++;
			 //ang_20211217
			 gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
			 gpPhyPriv->event_poll_timer_value = 20; //in case , LOF without LOS.
			 PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x,phyrx_status=0x%x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus,IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_STATUS));
			 if(phy_lof_cnt==5)
			 {		 	
				#if 1
				//reset xilinx serdes, ang_20211117 //julia_20220511
				read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
				write_data = read_data | XILINX_SERDES_RST_ON;
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
				mdelay(10);
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
				mdelay(10);
				#endif
				
			 PON_PHY_PRINT(PHY_MSG_ERR,"RX_LOF Serdes RESET!\r\n");
			 }
			 
		 }
		 else
		 {
		 	phy_lof_cnt =0;
		 }
	}
	
	if(trans_laser_detected==TRUE)
	{
		//if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_PHYRDY_INT)) 
		if(phyIntStatus & EN7581_GEPON_CSR_XPON_PHYRDY_INT)  //ang_20211220
		{
			phy_rdy_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
		#endif
			trans_laser_detected=FALSE;
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when laser on ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			trans_laser_detected=FALSE;
		} 
		else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT)) 
		{
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS when ready ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
			trans_laser_detected=FALSE;
		} 
		else
		{
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when laser on fw=0x%x, EN=0x%08x GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
        }
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_TRANS_LOS_INT))
	{
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_GEPON_CSR_XPON_NO_LOS_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);

		#if 1
		mdelay(200);
		//reset xilinx serdes, ang_20211117 //julia_20220511
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
		write_data = read_data | XILINX_SERDES_RST_ON;
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
		mdelay(10);
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
		mdelay(10);
		#endif
		
		phy_fw_ready(PHY_ENABLE);
		//phy_pma_reset();

		trans_laser_detected = TRUE;
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_NO_LOS Reset__Serdes Done\r\n");
	}
	/* else if(phyIntStatus & EN7581_GEPON_CSR_XPON_LOF_INT)  // LOF may happen after LOS
	{
		phy_lof_cnt++;
		//ang_20211217
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		gpPhyPriv->event_poll_timer_value = 20;	//in case , LOF without LOS.
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF GEPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x,phyrx_status=0x%x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus,IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_STATUS));
	} */
	else
    {
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(EN7581_GEPON_CSR_XPON_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! 0x1faf097c=0x%x,0x1faf0940=0x%x,0x1faf0944=0x%x, rogue_onu_det_en=0x%x,phyIntEn=0x%x, REG IntStatus=0x%08x\r\n",IO_GPHYREG(0x1faf097c),IO_GPHYREG(0x1faf0940),IO_GPHYREG(0x1faf0944),gpPhyPriv->rogue_onu_det_en,phyIntEn,phyIntStatus);
		if(phy_other_cnt ==10) //for rogue debug
		{			
			IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 0, 0 , 0 ); //ponphy sb int en 
			IO_SPHYA_REG_BITS(EN7583_PCS_SB_CTRL_0 , 8 , 8 , 0 ); //BEN alarm interrupt
			read_data = IO_GPHYREG(EN7583_PCS_SB_CTRL_0);
			PON_PHY_PRINT(PHY_MSG_INT,"XGPON EN7583_PCS_SB_CTRL_0 write    :0x%.8x\n", read_data);	
		}
	}
	
#endif

#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#endif	

	spin_unlock(&gpPhyPriv->event_handle_lock); 

	return PHY_SUCCESS;

}

int an7583_gepon_phy_event_poll(char* buf)
{
	ulong flags;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus=0,phyLosStatus=0;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;
	UINT32 write_data=0,read_data = 0,read_data1 = 0;
	int i=0,ready_cnt=0;

	if(FALSE == gpPhyPriv->is_phy_start){
		return PHY_FAILURE;	//ang_20180208
	}

	gpPhyPriv->event_poll_timer_value = 1500; //ang_20211217

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);

	//PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __START__ \r\n");

	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();

	
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSTA1);//read bit[18:20]
	if(((read_data >> EN7581_GEPON_PHYRDY_OFFSET) & EN7581_GEPON_PHYRDY_MASK) == EN7581_GEPON_PHYRDY_STATUS)
	{
	    if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)
	    {
	        for(i=0;i<2;i++)
            {
                mdelay(1);
                read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSTA1);
      	        IO_SPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_CNT_CTL, 0xa);
	            read_data1 = IO_GPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_SYNC_STATUS);
	            IO_SPHYREG(EN7581_GEPON_CSR_PHYRX_EPON_CNT_CTL, 0x5);
		        if((((read_data >> EN7581_GEPON_PHYRDY_OFFSET) & EN7581_GEPON_PHYRDY_MASK) == EN7581_GEPON_PHYRDY_STATUS) &&(read_data1==0))
				{
		            ready_cnt++;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_INT," POLLING => EPON dec10b8b_err_cnt = %d, loss_sycn_cnt = %d\r\n",(read_data1&&0xffffff00)>>8,read_data1&&0xff);
				}
             }
			 phyReadyStatus = (ready_cnt == 2) ? PHY_TRUE : PHY_FALSE;
	    }
		else
		{
		      phyReadyStatus = PHY_TRUE;
	     }
	}
	else
	{
		phyReadyStatus = PHY_FALSE;
	}

	
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_XPON_STA);
	if( (read_data & EN7581_GEPON_CSR_XPON_STA_LOS) == EN7581_GEPON_CSR_XPON_STA_LOS) //it is transceiver sfp rx LOSS no PHY digital
	{
		phyLosStatus = PHY_LOS_HAPPEN;
	}
	else
	{
		phyLosStatus = PHY_NO_LOS_HAPPEN;
	}

	//if(phy_no_ready_no_los())
	if((!phyReadyStatus)&&(!phyLosStatus))
	{
	    poll_no_ready_no_los_cnt++;

	    #if ASIC_SERDES
		phy_print_time();

		if((poll_no_ready_no_los_cnt)%10 ==0)
		{
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			pma_no_los_no_ready_reset();
		}

		gpPhyPriv->event_poll_timer_value = 3500;
		PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4

		#endif
			
		#if XILINX_SERDES
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT," POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);

		#if 1
		//reset xilinx serdes, ang_20211117 //julia_20220511
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
		write_data = read_data | XILINX_SERDES_RST_ON;
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
		mdelay(10);
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
		mdelay(10);
		#endif
			
		phy_fw_ready(PHY_ENABLE);
		//phy_pma_reset();

		gpPhyPriv->event_poll_timer_value = 3500;
		PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4

		#endif

		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		#ifdef LDDLA_SUPPORT_SET_TX_MODE
		phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
		phy_event_handler(&phy_event);
		#endif
		#endif
	}	
    else if(!(phyReadyStatus&&(!phyLosStatus))) 
	{
		//if(!is_phy_sync())  /* LOS */
			poll_set_los_cnt++;

			PON_PHY_PRINT(PHY_MSG_INT," POLLING => LOS(%d) ReadyStatus=%d, LosStatus=%d\r\n",poll_set_los_cnt,phyReadyStatus,phyLosStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		    phy_event_handler(&phy_event);
			#endif				
	}
	else if(phyReadyStatus&&(!phyLosStatus))
	{		  															
		//if(is_phy_sync()) /* PHY SYNC */
			poll_set_ready_cnt++;
			poll_no_ready_no_los_cnt=0;
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT," POLLING => READY(%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);

			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event);
			#endif
	}

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;

	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	if(PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		poll_los_cnt++;
	}
	else if(PHY_LINK_STATUS_READY==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		poll_ready_cnt++;
	}
#ifdef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_event_handler(&phy_event);
	}
	pre_phy_status=gpPhyPriv->phy_status;
#endif
	PON_PHY_PRINT(PHY_MSG_DBG," - POLLING - __END__ (%d)(%d)(%d) Get phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

	return PHY_SUCCESS;

}

int an7583_gepon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=121;
	phy_reg_all=en7581_gepon_phy_reg_all;

	
	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);
	switch(dbg_id)
	{
		case 1:
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			for(i=36;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			

			break;

		case 2:
			
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			for(i=43;i<=51;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;

		case 3:
			
			printk("\r\n%-44s	 address=value\r\n","_____ GEPON REG DBG _____");
			i=115;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=11;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=4;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			i=3;
			(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
			printk("%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);

			break;			

		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}

	return PHY_SUCCESS;

}


int an7583_gepon_phy_pma_reset(char* buf)
{
	
#if ASIC_SERDES	// for ASIC

	UINT32 read_data = 0;
	UINT32 write_data = 0;

	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3);
	write_data = read_data |EN7581_GEPON_CSR_PHYSET3_PLL_RST;  // |EN7581_GEPON_CSR_PHYSET3_PHY_RST|EN7581_GEPON_CSR_PHYSET3_SOFTWARE_RST;  //EN7581_GEPON_CSR_PHYSET3_LOS_RST
	IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3, write_data) ;
	phy_fw_ready(PHY_DISABLE);

	//clear bit delay
	read_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET5);
	write_data = (read_data & EN7581_GEPON_PHY_BIT_DELAY_MASK)| EN7581_GEPON_PHY_TX_BIT_DEL_SEL;
	IO_SPHYREG(EN7581_GEPON_CSR_PHYSET5,write_data);
	
	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	
	write_data = IO_GPHYREG(EN7581_GEPON_CSR_PHYSET3);
	write_data = write_data&~(EN7581_GEPON_CSR_PHYSET3_PLL_RST);	
	IO_SPHYREG(EN7581_GEPON_CSR_PHYSET3, write_data) ;
	
	phy_fw_ready(PHY_ENABLE);
#endif


	return PHY_SUCCESS;
}




#define _I_AM_XGPON_SPLIT_LINE_

/////////////////////////////////////////////////////////////////////////////////////
// en7581_xgpon_func
/////////////////////////////////////////////////////////////////////////////////////

int an7583_xgpon_pon_phy_reset(char* buf)
{
	uint val = 0;
	uint wan_cfg=0;

	PON_PHY_PRINT(PHY_MSG_INT,"\r\nEN7583 << XGPON >> pon_phy_reset\r\n");

	//gating PHY-D ck out
	PON_PMA_PON_CK_SET(0);
	udelay(1);

	//switch wan mode
	wan_cfg=GET_WAN_CONF();
	SET_WAN_CONF(((wan_cfg&0xffffff00)|0x11));
	udelay(1);

	//gating PHY-A ck out
	val=PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_GET(); 
	PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_SET(val&0xfffffeff); 

	//disable PHY-A ck
	val=PON_PMA_rg_force_da_pxp_txpll_ckout_en_GET(); 
	PON_PMA_rg_force_da_pxp_txpll_ckout_en_SET((val&0xfffeffff)|0x01000000);
	
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
	val |=0x01;
	SET_SCU_RST_RG(val);
	udelay(1);
	//val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);

	//release xpon phy top reset 
	val &=~(0x01);
	SET_SCU_RST_RG(val);		
	//val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
#else
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
#endif

	SET_WAN_CONF(wan_cfg);
	udelay(1);
	
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_ON);
	udelay(1);	
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N,EN7581_XGPON_PHY_XG_PHY_RST_N_OFF);

	return PHY_SUCCESS;
}


int an7583_xgpon_phy_dump(char* buf)
{
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=90;
	phy_reg_all=en7581_xgpon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ XGPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	
#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif
	

	return PHY_SUCCESS;
}


int an7583_xgpon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;
	
	static char phy_ngpon2_task_wait_start=FALSE;
		
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);

	
#ifdef TCSUPPORT_CPU_ARMV8 
		/* switch GPIO to XPON mode*/		
		read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE	
		write_data =(read_data | 0x01); //bit [0]: 1	
		SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE
#else 
	
		/* switch GPIO to XPON mode*/
		read_data = IO_GREG(IOMUX_Control_1_register);
		read_data = read_data | (GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
		IO_SREG(IOMUX_Control_1_register, read_data);
#endif 	

#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
		/*after sw reset, register can be modify by PBUS*/
		read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
		read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
		SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
		/* transceiver power initial setting*/
		read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
		read_data = read_data & ~(TOP_LED1_MODE);
		SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
		/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif

	//enable PCS RX control 	david 20170124
	read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	PON_PHY_PRINT(PHY_MSG_ERR,"Read XGPON_RX_SYNC_CTRL=0x%x \n",read_data);
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL,read_data|EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE) ;
	read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	PON_PHY_PRINT(PHY_MSG_ERR,"Set XGPON_RX_SYNC_CTRL=0x%x \n",read_data);


	//phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_ALL);

	/* Enable BIP error counter */
	
	/* change the guard time pattern */
	gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);

	/* en7581 rx fec control:
	rx_fec force off:0a84[5]=0, 0a84[4]=0,0a84[1]=0.  
	rx_fec force on: 0a84[6]=1. rx fec forced on for xg/xgs/ngpon.
	rx_fec refer to oc body: 0a84[5]=1,oa84[4]=1,0a84[1]=1. only for xgs/ngpon2. OLT may not support ocbody in xgpon mode.
	*/

	//for xgs/ngpon2, rx_fec refer to oc body. ang_20211206
	if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel))
		//||(SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)) //turn on XGS FEC by default
	{
		read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
		write_data = read_data | EN7581_XGPON_PHY_DBG_XG_OC_EN | EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF | EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN;
		IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
	}


	if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel))
	{
		if(FALSE==phy_ngpon2_task_wait_start)
		{
			phy_ngpon2_task_wait_start=TRUE;
			gpPhyPriv->phy_ngpon2_task_wait= kthread_run(xfp_trans_set_ngpon2_chan, NULL, "phy_ngpon2_task_wait"); //julia_20221021 ngpon2
			PON_PHY_PRINT(PHY_MSG_ERR,"run phy_ngpon2_task_wait\n");
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR,"bypass 2nd phy_ngpon2_task_wait\n");
		}
	}

	
#if XILINX_SERDES //julia_20220511
		if(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel)
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, 0x200002); //for ngpon2 BEN dly
	
#endif

	return PHY_SUCCESS ;

}

int an7583_xgpon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus = 0;

	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		//write_data=(EN7581_XGPON_PHY_RX_RDY_INT_EN|EN7581_XGPON_PHY_RX_LOF_INT_EN|EN7581_XGPON_PHY_RX_SYNC_OK_INT_EN|EN7581_XGPON_PHY_RX_LOS_INT_EN);
		write_data= EN7581_XGPON_PHY_TX_FAULT_INT_EN\
					|EN7581_XGPON_PHY_TX_BURST_SPACE_ERR_INT_EN\
					|EN7581_XGPON_PHY_TX_MPI_ERR_INT_EN\
					|EN7581_XGPON_PHY_TX_PSBU_INFO_ERR_INT_EN\
					|EN7581_XGPON_PHY_TX_INFO_FIFO_INT_EN\
					|EN7581_XGPON_PHY_RX_RDY_INT_EN\
					|EN7581_XGPON_PHY_RX_INFO_FIFO_INT_EN\
					|EN7581_XGPON_PHY_RX_BER_HIGH_INT_EN\
					|EN7581_XGPON_PHY_RX_LOF_INT_EN\
					|EN7581_XGPON_PHY_RX_SYNC_OK_INT_EN\
					|EN7581_XGPON_PHY_RX_LOS_INT_EN;
		
					//|EN7581_XGPON_PHY_RX_NGPON2_OC_ERR_INT_EN\ //julia_20231009
					//EN7581_XGPON_PHY_XG_PHYA_RDY_INT_EN\		
					//|EN7581_XGPON_PHY_TX_SFP_CONTINUE_INT_EN\
					//|EN7581_XGPON_PHY_TX_SFP_ABNORMAL_INT_EN\
					//|EN7581_XGPON_PHY_RX_CW_CNT_ERR_INT_EN\
					//|EN7581_XGPON_PHY_RX_FEC_ERR_INT_EN\

		PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}
	
	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system
	
	read_data = IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config read     :0x%.8x\n", read_data);

	IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_EN, write_data);
	read_data = IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON Phy_Int_Config write    :0x%.8x\n", read_data);


	return PHY_SUCCESS;
}


int an7583_xgpon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int phyReadyStatus,phyLosStatus;

	UINT32 i=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	phy_reg_all=en7581_xgpon_phy_reg_all;


	//////by ang_20180129
	UINT32 xgpon_preamble_Up=0 ;
	UINT32 xgpon_preamble_Low=0;

	UINT32 xgpon_delimiter_Up=0;
	UINT32 xgpon_delimiter_Low=0;

	EN7581_REG_PHY_FEC_INDICATION phyFec_indication;
	EN7581_REG_PHY_PSBU_INFO psbuInfo;
	//////
	

	api_data->ret = PHY_SUCCESS;

	switch(api_data->cmd_id)
	{
		case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(EN7581_XGPON_PHY_SFP_STA);
			if(EN7581_XGPON_PHY_SFP_RX_LOS_ST==(read_data & EN7581_XGPON_PHY_SFP_RX_LOS_ST)) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}

			break;
			
		case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
			if(EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data& EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}

			break; 
			
		case PON_GET_PHY_IS_SYNC:
			//api_data->ret = is_phy_sync(); // for this API, in_data is NULL
			
				read_data = IO_GPHYREG(EN7581_XGPON_PHY_SFP_STA);
				if(EN7581_XGPON_PHY_SFP_RX_LOS_ST==(read_data & EN7581_XGPON_PHY_SFP_RX_LOS_ST)) //it is transceiver sfp rx LOSS no PHY digital
				{
					phyLosStatus = PHY_LOS_HAPPEN;
				}
				else
				{
					phyLosStatus = PHY_NO_LOS_HAPPEN;
				}
			
				read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
				if(EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data& EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
				{
					//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
					phyReadyStatus = PHY_TRUE;
				}
				else
				{
					//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
					phyReadyStatus = PHY_FALSE;
				}

				api_data->ret = ((phyReadyStatus == PHY_TRUE) && (phyLosStatus == PHY_NO_LOS_HAPPEN) );
			break;

		case PON_GET_PHY_MODE:
			api_data->ret = gpPhyPriv->phyCfg.flags.mode;

			break;
			
		case PON_GET_PHY_TX_LONG_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;

			break;

		case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
			break;

		case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;

			break;

		case PON_GET_PHY_BIP_COUNTER:
			
			break;

		case PON_GET_PHY_RX_FEC_COUNTER:
			
			api_data->rx_fec_cnt->correct_bytes=IO_GPHYREG(EN7581_XGPON_PHY_FEC_CORRECTED_BYTE_CNT);
			api_data->rx_fec_cnt->correct_codewords=IO_GPHYREG(EN7581_XGPON_PHY_FEC_CORRECTED_CW_CNT);
			api_data->rx_fec_cnt->uncorrect_codewords=IO_GPHYREG(EN7581_XGPON_PHY_FEC_UNCORRECTED_CW_CNT);
			api_data->rx_fec_cnt->total_rx_codewords=IO_GPHYREG(EN7581_XGPON_PHY_FEC_TOTAL_CW_CNT);
			//EN7581_XGPON_PHY_FEC_ERR_SECONDS
			api_data->rx_fec_cnt->fec_seconds=IO_GPHYREG(EN7581_XGPON_PHY_FEC_ERR_SECONDS); // by ang_20170808

			break;

		case PON_GET_PHY_RX_FRAME_COUNTER:
			api_data->rx_frame_cnt->frame_count_high = 0x0;
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(EN7581_XGPON_PHY_DBG_RX_FRAME2PHYD_CNT);
			api_data->rx_frame_cnt->lof_counter = IO_GPHYREG(EN7581_XGPON_PHY_DBG_LOF_CNT);
			break;

		case PON_GET_PHY_RX_FEC_GETTING:
			//ang_20211206
			read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			if(read_data&EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF)
			{
				api_data->ret = PHY_TRUE; 
			}
			else
			{
				api_data->ret = PHY_FALSE; //rx fec force_off
			}

			break;

		case PON_GET_PHY_RX_FEC_STATUS:

			//ang_20211206
			write_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_XG_PHYD_PROBE_SEL);

			IO_SPHYREG(EN7581_XGPON_PHY_DBG_XG_PHYD_PROBE_SEL,((write_data & 0xFFFFFFF0)|EN7581_XGPON_PHY_PROBE_RX_FEC_CTRL)) ; //select rx fec probe

			read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_XG_PHYD_LOWER_PROBE);
			if((read_data & EN7581_XGPON_PHY_RX_FEC_ENABLE_STATUS) == EN7581_XGPON_PHY_RX_FEC_ENABLE_STATUS)
				{
					api_data->ret = PHY_TRUE;
				}
				else
				{
					api_data->ret = PHY_FALSE;
				}
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_XG_PHYD_PROBE_SEL,write_data) ; //restore reg

			break;

		case PON_GET_PHY_TX_FEC_STATUS:		//by ang_20180115
			read_data = IO_GPHYREG(EN7581_XGPON_PHY_DBG_TX_FEC_STA);
			if((read_data & EN7581_XGPON_PHY_TX_FEC) == EN7581_XGPON_PHY_TX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
				api_data->ret = PHY_FALSE;
			}

			break;

		case PON_GET_PHY_TX_BURST_GETTING:
			
#if ASIC_SERDES
			
           //	read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1);
           read_data =AN7583_Get_PMA_XPON_TX_Force_1();
			if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
				//	read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2);
				read_data =AN7583_Get_PMA_XPON_TX_Force_2();

				if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
					api_data->ret = PHY_TX_CONT_MODE;
				}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
			break;

		case PON_GET_PHY_TRANS_TX:
			
			#if 0 //ASIC_SERDES //EN7580_ASIC //julia_7583_mark
			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;

			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
			
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_GET_PHY_TRANS_TX -- 7583 mark\n"); //julia_7583_pma
			break;

		case PON_GET_PHY_TRANS_RX_GETTING:

//#if ASIC_SERDES			
			#if 0 //julia_debug
            read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((read_data & EN7581_XPON_PMA_XPON_SETTING_0_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
			#endif
			
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_GET_PHY_TRANS_RX_GETTING -- 7583 mark\n"); //julia_7583_pma
			break;

		case PON_GET_PHY_ROUND_TRIP_DELAY:

			break;

        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(EN7581_LOF);	  
	  		api_data->ret=0;

			break;
		case PON_GET_PHY_XGPON_PROFILE:

			phyFec_indication.Raw = IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);	

			if(api_data->xgpon_profile->profile_index==0)	// profile index is 0
			{
				//get preamble
				xgpon_preamble_Up = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE1_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE1_LOWER);	
				//get delimiter
				xgpon_delimiter_Up = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER1_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER1_LOWER);
	
				//get fec status
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en1;
					
				//get preamble repeat count & preableb length & delimiter length
				psbuInfo.Raw = IO_GPHYREG(EN7581_XGPON_PHY_PSBU_INFO1);
			}
			else if(api_data->xgpon_profile->profile_index==1)	// profile index is 1
			{	
				xgpon_preamble_Up = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE2_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE2_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER2_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER2_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en2;
					
				psbuInfo.Raw = IO_GPHYREG(EN7581_XGPON_PHY_PSBU_INFO2);
			}
			else if(api_data->xgpon_profile->profile_index==2)	// profile index is 2
			{	
				xgpon_preamble_Up = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE3_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE3_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER3_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER3_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en3;
					
				psbuInfo.Raw = IO_GPHYREG(EN7581_XGPON_PHY_PSBU_INFO3);
			}
			else if(api_data->xgpon_profile->profile_index==3)	// profile index is 3
			{	
				xgpon_preamble_Up = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE4_UPPER);
				xgpon_preamble_Low = IO_GPHYREG(EN7581_XGPON_PHY_PREAMBLE4_LOWER);	

				xgpon_delimiter_Up = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER4_UPPER);
				xgpon_delimiter_Low = IO_GPHYREG(EN7581_XGPON_PHY_DELIMITER4_LOWER);
	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en4;
					
				psbuInfo.Raw = IO_GPHYREG(EN7581_XGPON_PHY_PSBU_INFO4);
			}
			else
			{	
				xgpon_preamble_Up = 0;
				xgpon_preamble_Low = 0;	
				xgpon_delimiter_Up = 0;
				xgpon_delimiter_Low = 0;	
				api_data->xgpon_profile->fec_indication = phyFec_indication.Bits.xg_tx_fec_en4;				
				psbuInfo.Raw = 0;
				PON_PHY_PRINT(PHY_MSG_ERR, "PON_PHY ERROR! profile_index is not between 0 and 3 ! \n");	
			}

			api_data->xgpon_profile->preamble[0] = (xgpon_preamble_Up >> 24)  & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[1] = (xgpon_preamble_Up >> 16)  & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[2] = (xgpon_preamble_Up >>  8)  & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[3] = (xgpon_preamble_Up >>  0)  & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[4] = (xgpon_preamble_Low >> 24) & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[5] = (xgpon_preamble_Low >> 16) & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[6] = (xgpon_preamble_Low >>  8) & EN7581_XGPON_PHY_PREAMBLE_MASK;
			api_data->xgpon_profile->preamble[7] = (xgpon_preamble_Low >>  0) & EN7581_XGPON_PHY_PREAMBLE_MASK;

			api_data->xgpon_profile->delimiter[0] = (xgpon_delimiter_Up >> 24)  & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[1] = (xgpon_delimiter_Up >> 16)  & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[2] = (xgpon_delimiter_Up >>  8)  & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[3] = (xgpon_delimiter_Up >>  0)  & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[4] = (xgpon_delimiter_Low >> 24) & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[5] = (xgpon_delimiter_Low >> 16) & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[6] = (xgpon_delimiter_Low >>  8) & EN7581_XGPON_PHY_DELIMITER_MASK;
			api_data->xgpon_profile->delimiter[7] = (xgpon_delimiter_Low >>  0) & EN7581_XGPON_PHY_DELIMITER_MASK;

			api_data->xgpon_profile->preamble_repeat_count = psbuInfo.Bits.prmb_rpt_num;	
			api_data->xgpon_profile->preamble_length = psbuInfo.Bits.ptmb_len;
			api_data->xgpon_profile->delimiter_length = psbuInfo.Bits.dlmt_len;	

			PON_PHY_PRINT(PHY_MSG_TRACE,"profile index  		= %d\r\n", api_data->xgpon_profile->profile_index);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_preamble_Up 		= 0x%x\r\n", xgpon_preamble_Up);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_preamble_Low 	= 0x%x\r\n", xgpon_preamble_Low);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_delimiter_Up 	= 0x%x\r\n", xgpon_delimiter_Up);
			PON_PHY_PRINT(PHY_MSG_TRACE,"xgpon_delimiter_Low 	= 0x%x\r\n", xgpon_delimiter_Low);
			PON_PHY_PRINT(PHY_MSG_TRACE,"preamble repeat number = %d\r\n", psbuInfo.Bits.prmb_rpt_num);
			PON_PHY_PRINT(PHY_MSG_TRACE,"preamble length 		= %d\r\n", psbuInfo.Bits.ptmb_len);
			PON_PHY_PRINT(PHY_MSG_TRACE,"delimiter length  		= %d\r\n", psbuInfo.Bits.dlmt_len);

			break;
			
		case PON_GET_PHY_INIT_STATUS: //ang_20190821
			api_data->ret = gpPhyPriv->phy_init_done;
			break;
		
		//get ngpon2 tx/rx chan, ang_20190605
		//case PON_GET_PHY_NGPON2_CHAN: //julia_20221021 ngpon2
       //     api_data->ret = xfp_trans_get_ngpon2_chan(api_data->ngpon2_chan_sel);
       //     break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
			api_data->ret = PHY_NO_API;
			break;
	}

	return PHY_SUCCESS;	//ang_20180208
}

int an7583_xgpon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	
	UINT32 i=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	phy_reg_all=en7581_xgpon_phy_reg_all;

	//////by ang_20180129
	UINT32 xgpon_preamble_Up=0 ;
	UINT32 xgpon_preamble_Low=0;

	UINT32 xgpon_delimiter_Up=0;
	UINT32 xgpon_delimiter_Low=0;

	EN7581_REG_PHY_FEC_INDICATION phyFec_indication;
	EN7581_REG_PHY_PSBU_INFO psbuInfo;
	//////
	
    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);
            break;

		case PHY_SET_ERR_CNT_EN:
		case PHY_SET_BIP_CNT_EN:
		case PHY_SET_FM_CNT_EN:
			//PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, cmd_id=0x%x, data=%d\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id,(*api_data->data));
			break;

		case PHY_SET_PCS_FPGAIF_RESET:
			break;
		case PON_SET_PHY_LOGIC_RESET:

			#if ASIC_SERDES
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
			//disable rx
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			//gpPhyPriv->first_plugin_flag = TRUE; 
			//xpon_init(gpPhyPriv->wan_sel); 
			//xpon_pma_param_opt(pma_hi_rate_opt_val); //julia_7583_pma mark by morris suggest
			phy_pma_reset();
			PON_PHY_PRINT(PHY_MSG_INT,"PHY_LOGIC_RESET\n");
			#endif
			
			#if XILINX_SERDES
			if(gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)
			{
					
			//enable rx 
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE; //enable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			//reset xilinx serdes, ang_20211117 //julia_20220511
			read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
			write_data = read_data | XILINX_SERDES_RST_ON;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
			mdelay(10);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
			mdelay(10);
			
			
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_SERDES);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE);
			//gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			//handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);  
			
			PON_PHY_PRINT(PHY_MSG_ERR,"PHY_LOGIC_RESET.\n");
			}
			#endif
			
			break;

		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
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
			if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);

				write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
				write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
				
				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			}
			else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_ON);
				write_data &= (~EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF);

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			}
			else if(gpPhyPriv->rx_fec_setting == DS_FEC_SETTING_AS_SPEC) //julia_20230209
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				
				//if((SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)) //refer to oc
				if((SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel)) //refer to oc				
				{
					write_data = read_data | EN7581_XGPON_PHY_DBG_XG_OC_EN;
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN;
				}
				else if(SCU_WAN_CONF_REG_WAN_SEL_XGPON==gpPhyPriv->wan_sel) //force on
				{
					write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
					write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
				}
				else
				{
					gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
					PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC setting now (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
				}

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC AS SPEC (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			else if(gpPhyPriv->rx_fec_setting == DS_FEC_SETTING_FORCE_OC) //julia_20230209
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				write_data = read_data | EN7581_XGPON_PHY_DBG_XG_OC_EN;
				write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
				write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN;

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC refer to OC (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW); //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_INT,"PHY_SCU_RESET\n");
			#endif
			break;

		case PHY_SET_PCS_RESET:
			break;
			
        case PON_SET_PHY_RESET_COUNTER:
            read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data|EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data&~EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
            break;

        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            break;

        case PON_SET_PHY_COUNTER_CLEAR:
            read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data|EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data&~EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
            break;

        case PON_SET_EPONMODE_PHY_RESET:
            break;

        case PON_SET_PHY_BIT_DELAY:
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_PHYD_DLY_STA);
			//IO_SPHYREG(EN7581_XGPON_PHY_DBG_PHYD_DLY_STA,(*api_data->data));
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;
			
		case PON_SET_PHY_TX_D_PADDING_CONFIG:
			api_data->ret = PHY_SUCCESS;
            if(api_data->data == PHY_ENABLE)
			{
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_IDLE_CTRL, EN7581_XGPON_PHY_XG_TX_IDLE_CTRL_PATTERN_SEL(0));
				write_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_IDLE_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"TX_D_PADDING ON (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_XG_TX_IDLE_CTRL,write_data);

			}
			else if(api_data->data == PHY_DISABLE)
			{
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_IDLE_CTRL, EN7581_XGPON_PHY_XG_TX_IDLE_CTRL_OFF);
				write_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_IDLE_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"TX_D_PADDING OFF (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_XG_TX_IDLE_CTRL,write_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}		
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:
			
			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
						
			an7583_set_tx_pattern(api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->tx_d_in_timeslot,api_data->phy_rogue_cfg->rogue_pattern);
			
			#endif
			
#if XILINX_SERDES

			if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1))
			{
				gpPhyPriv->phyCfg.flags.rogue = 1;
				
				IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN\  
													EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(api_data->phy_rogue_cfg->rogue_pattern)\
													|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(api_data->phy_rogue_cfg->tx_d_in_timeslot));
				
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);

				PON_PHY_PRINT(PHY_MSG_ERR,"Set EN7581_XGPON_PHY_XG_CONTINUE_CTRL(0x%x)=0x%x \n",EN7581_XGPON_PHY_XG_CONTINUE_CTRL,read_data);		
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue ON\n");
				PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS23,2=PRBS31,3=USER_CONFIG)\n",api_data->phy_rogue_cfg->rogue_pattern);
				PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot %s\n",(EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL_ALL_PRBS&read_data)?"Rogue":"Normal");
				
				PON_PHY_PRINT(PHY_MSG_ERR,"Set AN7583_Set_Trans_Brust_Forve(0x1fafE960)=0x%x \n",write_data);		
				#if 0
				if(3==api_data->phy_rogue_cfg->rogue_pattern)
				{
					PON_PHY_PRINT(PHY_MSG_ERR,"Note : Rogue pattern is USER_CONFIG\n\tPlease set data by follow cmds:\n\tsys memwl bfaf0a7c <low_32bit_data>\n\tsys memwl bfaf0a80 <up_32bit_data>\n");
				}			
				#endif
			} 
			else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE)
			{

				IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_DISABLE\
													|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(0)\
													|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(0));
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);
				PON_PHY_PRINT(PHY_MSG_ERR, "Set EN7581_XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);	

				
				read_data = AN7583_Get_Trans_Brust_Forve();
				write_data = read_data &(0xFFFFFFFE); //not force BEN on
				AN7583_Set_Trans_Brust_Forve(write_data);
				
				gpPhyPriv->phyCfg.flags.rogue = 0;

				PON_PHY_PRINT(PHY_MSG_ERR, "Phy_NO_Rogue_PRBS mode\n");
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				api_data->ret = PHY_FAILURE;
			}
#endif
            break;

        case PON_SET_PHY_DEV_INIT:
            //api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
			#if 0
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				//tx brust en
#if XILINX_SERDES
				write_data = ~phy_trans_iot_list[gpPhyPriv->trans_index].tx_setting;
#else
				write_data = phy_trans_iot_list[gpPhyPriv->trans_index].tx_setting;
#endif
				read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_TRANS_BRUST_INV(0x%x)=0x%x \n",EN7583_TRANS_BRUST_INV,read_data);
				IO_SPHYA_REG_BITS(EN7583_TRANS_BRUST_INV, 8, 8, write_data); 
				read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_TRANS_BRUST_INV(0x%x)=0x%x \n",EN7583_TRANS_BRUST_INV,read_data);
							
				//Rx los
#if XILINX_SERDES
				write_data = ~phy_trans_iot_list[gpPhyPriv->trans_index].rx_setting;
#else
				write_data = phy_trans_iot_list[gpPhyPriv->trans_index].rx_setting;
#endif
				read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x \n",EN7583_PON_PCS_RX_SD_CTRL,read_data);
				IO_SPHYA_REG_BITS(EN7583_PON_PCS_RX_SD_CTRL, 0, 0, write_data); 
				read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x \n",EN7583_PON_PCS_RX_SD_CTRL,read_data);

            }
			#endif
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_SET_PHY_TRANS_MODEL_SETTING -- 7583 mark\n"); //julia_7583_pma
            break;

        case PON_SET_PHY_RX_FEC_SETTING:

            if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);

				write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
				write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
				
				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Force ON (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
				write_data &= (~EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_ON);
				write_data &= (~EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF);

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Force OFF (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			else if((*api_data->data) == DS_FEC_SETTING_AS_SPEC) //julia_20230209 //for xgpon,ds fec always on,for xgspon/ngpon2 refer to ocbody
			{
				gpPhyPriv->rx_fec_setting=DS_FEC_SETTING_AS_SPEC;
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				
				//if((SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)) //refer to oc
				if((SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel)) //refer to oc				
				{
					write_data = read_data | EN7581_XGPON_PHY_DBG_XG_OC_EN;
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN;
				}
				else if(SCU_WAN_CONF_REG_WAN_SEL_XGPON==gpPhyPriv->wan_sel) //force on
				{
					write_data = read_data&(~EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN);
					write_data &= (~EN7581_XGPON_PHY_DBG_XG_OC_EN);
					write_data |= EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF;
				}
				else
				{
					gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
					PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC setting now (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
				}

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC AS SPEC (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			else if((*api_data->data) == DS_FEC_SETTING_FORCE_OC) //julia_20230209
			{
				gpPhyPriv->rx_fec_setting=DS_FEC_SETTING_FORCE_OC;
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				write_data = read_data | EN7581_XGPON_PHY_DBG_XG_OC_EN | EN7581_XGPON_PHY_DBG_RX_FEC_FORCE_OFF | EN7581_XGPON_PHY_DBG_RX_FEC_OC_REF_EN;

				IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,write_data);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC refer to ocbody (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC setting now (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_DBG_CTRL,read_data);
			}
			
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
			PON_PHY_PRINT(PHY_MSG_ERR, "TX_BURST_CONFIG=%d\n",(*api_data->data));

            if(((*api_data->data) != PHY_TX_BURST_MODE) && ((*api_data->data) != PHY_TX_CONT_MODE))
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				//set idle pattern. 0x00:PRBS7; 0x01:PRBS31; 0x11:user pattern
				IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,(*api_data->data)\
													|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(0x0)\
													|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_MODE_SEL(0));
				//IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0);
				//IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);
				PON_PHY_PRINT(PHY_MSG_ERR,"XG_CONTINUE_CTRL (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_XG_CONTINUE_CTRL,read_data);

				if((*api_data->data) == PHY_TX_BURST_MODE) //julia_7583_BEN_HEC
				{					
					read_data = AN7583_Get_Trans_Brust_Forve();
					write_data = read_data &(0xFFFFFFFE); //not force BEN on
					AN7583_Set_Trans_Brust_Forve(write_data);
				}
				else if((*api_data->data) == PHY_TX_CONT_MODE)
				{
					read_data = AN7583_Get_Trans_Brust_Forve();
					write_data = read_data |(0x01); //not force BEN on
					AN7583_Set_Trans_Brust_Forve(write_data);
				}
					
					
				gpPhyPriv->phyCfg.flags.txLongFlag = (((*api_data->data)==PHY_TX_CONT_MODE) ? PHY_TRUE : PHY_FALSE);
				api_data->ret = PHY_SUCCESS;
			}            
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:

            break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
             break;
			
        case PHY_SET_XGPON_PROFILE_DELIMITER:
			for(i=13;i<=20;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;     

		case PHY_SET_XGPON_PROFILE_PREAMBLE:
			for(i=5;i<=12;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			break;
			
		case PHY_SET_XGPON_PROFILE_LEN:
			for(i=22;i<=25;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			break;		
			
		case PHY_SET_XGPON_LASER_ON_LEN:
			for(i=26;i<=26;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}	
			break;

		case PHY_SET_XGPON_LASER_ON_PATTERN:
			for(i=27;i<=27;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,(*api_data->data));
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s 0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}	
			break;
			
		case PHY_SET_TX_FEC_EN:
            if((*api_data->data) == PHY_ENABLE)
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL,EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL_ENABLE);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Enable (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL, EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL_DISABLE);
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Disable (0x%x)=0x%.8x\n", EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL,read_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;
			
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

		case PON_SET_PHY_XGPON_PROFILE:
			//get preamble and deilmiter
			xgpon_preamble_Up=((api_data->xgpon_profile->preamble[0])<<24)|((api_data->xgpon_profile->preamble[1])<<16)\
					   			|(api_data->xgpon_profile->preamble[2]<<8)|(api_data->xgpon_profile->preamble[3]);
			
			xgpon_preamble_Low=(api_data->xgpon_profile->preamble[4]<<24)|(api_data->xgpon_profile->preamble[5]<<16)\
					   			|(api_data->xgpon_profile->preamble[6]<<8)|(api_data->xgpon_profile->preamble[7]);
			
			xgpon_delimiter_Up=((api_data->xgpon_profile->delimiter[0])<<24)|((api_data->xgpon_profile->delimiter[1])<<16)\
					   			|((api_data->xgpon_profile->delimiter[2])<<8)|(api_data->xgpon_profile->delimiter[3]);
			
			xgpon_delimiter_Low=((api_data->xgpon_profile->delimiter[4])<<24)|((api_data->xgpon_profile->delimiter[5])<<16)\
					   			|((api_data->xgpon_profile->delimiter[6])<<8)|(api_data->xgpon_profile->delimiter[7]);

			phyFec_indication.Raw =IO_GPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL);		//get current fec status
			
			//get preamble repeat count & preableb length & delimiter length
			psbuInfo.Bits.prmb_rpt_num = api_data->xgpon_profile->preamble_repeat_count;	
			psbuInfo.Bits.ptmb_len = api_data->xgpon_profile->preamble_length;
			psbuInfo.Bits.dlmt_len = api_data->xgpon_profile->delimiter_length;	

			if(api_data->xgpon_profile->profile_index==0)	// profile index is 0
			{	
				//set preamble
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE1_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE1_LOWER, xgpon_preamble_Low);	
				//set delimiter
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER1_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER1_LOWER, xgpon_delimiter_Low);	
				//set fec enabler or disable
				phyFec_indication.Bits.xg_tx_fec_en1=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);		
				//set preamble repeat count & preableb length & delimiter length
				IO_SPHYREG(EN7581_XGPON_PHY_PSBU_INFO1, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==1)	// profile index is 1
			{
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE2_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE2_LOWER, xgpon_preamble_Low);

				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER2_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER2_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en2=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(EN7581_XGPON_PHY_PSBU_INFO2, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==2)	// profile index is 2
			{
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE3_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE3_LOWER, xgpon_preamble_Low); 
				
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER3_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER3_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en3=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(EN7581_XGPON_PHY_PSBU_INFO3, psbuInfo.Raw);
			}
			else if(api_data->xgpon_profile->profile_index==3)	// profile index is 3
			{
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE4_UPPER, xgpon_preamble_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_PREAMBLE4_LOWER, xgpon_preamble_Low); 

				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER4_UPPER, xgpon_delimiter_Up);
				IO_SPHYREG(EN7581_XGPON_PHY_DELIMITER4_LOWER, xgpon_delimiter_Low);

				phyFec_indication.Bits.xg_tx_fec_en4=api_data->xgpon_profile->fec_indication;
				IO_SPHYREG(EN7581_XGPON_PHY_XG_TX_FEC_EN_CTRL, phyFec_indication.Raw);
				
				IO_SPHYREG(EN7581_XGPON_PHY_PSBU_INFO4, psbuInfo.Raw);
			}
		
				break;
			
		case PON_SET_PHY_XGPON_RX_ENABLE:
			
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE;	//enable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			break;

		case PON_SET_PHY_XGPON_RX_DISABLE:
			
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE);	//disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			break;

		//set ngpon2 tx/rx chan, ang_20190605
		case PON_SET_PHY_NGPON2_TX_CHAN_CONFIG:
			gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan=api_data->ngpon2_chan_sel->ngpon2_tx_chan; //julia_20221021 ngpon2
			break;
		case PON_SET_PHY_NGPON2_RX_CHAN_CONFIG:
			gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan=api_data->ngpon2_chan_sel->ngpon2_rx_chan; //julia_20221021 ngpon2
			break;
			
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:

			gpPhyPriv->en7571_init_done = FALSE;
			PON_PHY_PRINT(PHY_MSG_ERR, "ERROR: Should not init 7571 in XGPON mode !\r\n");

			break;
				
		case PON_SET_PHY_RX_CDR:
#if ASIC_SERDES

			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				//xpon_pma_param_opt(pma_hi_rate_opt_val); //julia_7583_pma mark by morris suggest
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif
		
#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif		

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }

	return PHY_SUCCESS;	//ang_20180208
}

int an7583_xgpon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn,read_data,write_data,read_data_1,read_data_2, phypmaIntEn, phypmaIntStatus ;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,phy_rx_rdy_cnt=0,phy_lof_cnt=0,phy_other_cnt=0,phy_fake_sync_cnt=0;
	static uint trans_laser_detected = FALSE;
	static uint rogue_detected_flag = FALSE;
	isr_cnt++;
	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA);
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_INT_STA, phyIntStatus);
	phy_print_time();

	gpPhyPriv->event_poll_timer_value =1500;//julia_20220511

#if ASIC_SERDES

	if(gpPhyPriv->rogue_onu_det_en)
	{
		rogue_detected_flag = AN7583_Rogue_Onu_Det(); //julia_debug need return rogue or no

		if(rogue_detected_flag ==TRUE)
		{
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_PMA_ROGUE_ONU detected.\r\n");
			handle_hw_irq_event(PHY_EVENT_TF_INT);
			phy_trans_power_switch(PHY_DISABLE);  //for debug
		}

		
		#if 0
		read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_INT_EN_0);

		if(read_data & EN7581_XPON_PMA_TRANS_ROGUE_ONU_INT_EN)
		{
			phypmaIntStatus=IO_GPHYREG(EN7581_XPON_PMA_XPON_INT_STA_0);
			IO_SPHYREG(EN7581_XPON_PMA_XPON_INT_STA_0, phypmaIntStatus);

			//clear rogue onu cnt
			IO_SPHYA_REG_BITS(EN7581_XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 1 );
			IO_SPHYA_REG_BITS(EN7581_XPON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 0 );

			if(phypmaIntStatus & EN7581_XPON_PMA_TRANS_ROGUE_ONU_INT)
			{
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_PMA_ROGUE_ONU detected. XGPON. REG IntStatus=0x%08x\r\n",phypmaIntStatus);
				handle_hw_irq_event(PHY_EVENT_TF_INT);
				phy_trans_power_switch(PHY_DISABLE);  //for debug
			}
		}
		#endif
	}
#endif

	
	if((gpPhyPriv->phy_status != PHY_LINK_STATUS_LOS)&&(phyIntStatus & EN7581_XGPON_PHY_RX_LOS_INT))
	{
		gpPhyPriv->phy_unexpected_isr_flag=FALSE;
		
		#if ASIC_SERDES
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
		
		//disable rx
		read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
		IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

		//read_data=IO_GPHYREG(EN7581_XPON_PMA_RO_RX_FREQDET);	//for debug
		//PON_PHY_PRINT(PHY_MSG_DBG,"after tdc off and disabel rx,RX_CK 0xbfaf3820 = 0x%x\n", read_data);

		#endif

		
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
		trans_laser_detected=FALSE;
		#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
		#endif

	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XGPON_PHY_RX_RDY_INT))
	{
		phy_rx_rdy_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_RDY Reset__Serdes XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		phy_delay1ms(2);
		#if ASIC_SERDES
		phy_pma_reset();		
		#else
		
		//enable rx 
		read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
		write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE; //enable rx
		IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

		//reset xilinx serdes, ang_20211117 //julia_20220511
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
		write_data = read_data | XILINX_SERDES_RST_ON;
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
		mdelay(10);
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
		mdelay(10);

		IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_SERDES);
		IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE);
		#endif
		trans_laser_detected = TRUE;
		phy_delay1ms(8); //after pma reset, have to wait the whole system stable. ang_20180621
		
		gpPhyPriv->event_poll_timer_value =50; //julia_20220511
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XGPON_PHY_RX_SYNC_OK_INT)) 
	{
		read_data_1 = IO_GPHYREG(EN7581_XGPON_PHY_SFP_STA);
		read_data_2 = IO_GPHYREG(EN7581_XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
		
		if(EN7581_XGPON_PHY_SFP_RX_LOS_ST!=(read_data_1 & EN7581_XGPON_PHY_SFP_RX_LOS_ST))
		{
			if(EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data_2& EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				phy_rdy_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				#endif
			}
			else
			{
				phy_fake_sync_cnt++;
				gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;//ang_20191009
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY LINK is not stable! XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);	
			}
		}
		else //NO laser
		{
			#if ASIC_SERDES
		
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
			
			//disable rx
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			//read_data=IO_GPHYREG(EN7581_XPON_PMA_RO_RX_FREQDET);	//for debug
			//PON_PHY_PRINT(PHY_MSG_DBG,"after tdc off and disabel rx,RX_CK 0xbfaf3820 = 0x%x\n", read_data);

			#endif

			
			phy_los_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS XGPON, REG IntStatus=0x%08x\r\n",phyIntStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			trans_laser_detected=FALSE;
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
			#endif
		}
			
	}
	else if(phyIntStatus & EN7581_XGPON_PHY_RX_LOF_INT)  // LOF may happen after LOS
	{
		//#if ASIC_SERDES
		//read_data=IO_GPHYREG(EN7581_XPON_PMA_RO_RX_FREQDET); //for debug
		//PON_PHY_PRINT(PHY_MSG_DBG,"When LOF int,RX_CK 0xbfaf3820 = 0x%x\n", read_data);
		//#endif
		
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		phy_lof_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
		handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT);  //report lof event to mac


		if(phyIntStatus & EN7581_XGPON_PHY_RX_SYNC_OK_INT) // in case LOF and SYNC_OK assert at same time. ang_20200820
		{
			read_data_2 = IO_GPHYREG(EN7581_XGPON_PHY_DBG_RX_SYNC_ST);//read bit[18:20]
			
			if(EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC==(read_data_2& EN7581_XGPON_PHY_DBG_RX_SYNC_ST_SYNC))
			{
				phy_rdy_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				#endif
			}
			else
			{
				phy_fake_sync_cnt++;
				gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;//ang_20191009
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY LINK is not stable! XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);	
			}	
			
		}

	}
	else
	{
		phy_other_cnt++;
		gpPhyPriv->phy_unexpected_isr_flag=TRUE;
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;
		phyIntEn=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw=0x%x, EN=0x%08x XGPON ISR=%d=%d+%d+%d+%d+%d,REG IntStatus=0x%08x\r\n",gpPhyPriv->phy_status,phyIntEn,isr_cnt,phy_rdy_cnt,phy_los_cnt,phy_rx_rdy_cnt,phy_lof_cnt,phy_other_cnt,phyIntStatus);
	}

	#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	#endif		
	spin_unlock(&gpPhyPriv->event_handle_lock); 
	return PHY_SUCCESS;

}

int an7583_xgpon_phy_event_poll(char* buf)
{
	ulong flags;
	uint read_data,write_data ;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus,phyLosStatus;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;

	if(FALSE == gpPhyPriv->is_phy_start){
		return	PHY_FAILURE;	//ang_20180208
	}

	gpPhyPriv->event_poll_timer_value = 1500;

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);
	
	//phy_print_time();

	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();

	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __START__ [%s:%d] phy_status=%d (ready=%d,los=%d)\r\n",__FUNCTION__,__LINE__,gpPhyPriv->phy_status,phyReadyStatus,phyLosStatus);
	PON_PHY_PRINT(PHY_MSG_INT," | POLLING | ncpo = 0x%08x\n",PON_PMA_LCLCPLL_TDC_RO_4_GET());

	if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status) 
	{
		poll_no_ready_no_los_cnt=0;
		//if(!is_phy_sync())  /* LOS */
		if(!(phyReadyStatus&&(!phyLosStatus)))
		{
			poll_set_los_cnt++;
			#if XILINX_SERDES

			//enable rx 
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE; //enable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);
			
			//reset xilinx serdes, ang_20211117 //julia_20220511
			read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
			write_data = read_data | XILINX_SERDES_RST_ON;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
			mdelay(10);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
			mdelay(10);
			
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_SERDES);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => LOS(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_set_los_cnt,phyReadyStatus,phyLosStatus);
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			#else
			//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
			#endif

			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			#endif
		}
	}
	else
	{	 
		//if(phy_no_ready_no_los())
		if((!phyReadyStatus)&&(!phyLosStatus))
		{
			poll_no_ready_no_los_cnt++;

			#if XILINX_SERDES

			//enable rx 
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
			write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE; //enable rx
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			//reset xilinx serdes, ang_20211117 //julia_20220511
			read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
			write_data = read_data | XILINX_SERDES_RST_ON;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
			mdelay(10);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
			mdelay(10);
			
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_SERDES);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N, EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#endif

			#if ASIC_SERDES
//			read_data=IO_GPHYREG(EN7583_XPON_PMA_RO_RX_FREQDET); //for debug
//			PON_PHY_PRINT(PHY_MSG_INT,"When polling no_ready_no_loss, reg 0x%8x = 0x%08x\n", EN7583_XPON_PMA_RO_RX_FREQDET,read_data);
			
			if(gpPhyPriv->event_poll_timer_value>100)  //avoid reset soon after ISR
			{
				if((poll_no_ready_no_los_cnt)%10 ==0)
				{
					PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
					PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
					pma_no_los_no_ready_reset();	
				}
			}
			else  // polling too quick will get no ready no los, need not reset pma
			{
				PON_PHY_PRINT(PHY_MSG_INT," | POLLING(%d ms) => no_ready_no_los(%d)Bypass Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",gpPhyPriv->event_poll_timer_value,poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			}
			
			gpPhyPriv->event_poll_timer_value = 3500;
			PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4


			#endif


			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			#ifdef LDDLA_SUPPORT_SET_TX_MODE
			phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
			phy_event_handler(&phy_event);
			#endif
			#endif
		}																

		//if(is_phy_sync())	/* PHY SYNC */
		else if(phyReadyStatus&&(!phyLosStatus))
		{
			
			if(gpPhyPriv->phy_unexpected_isr_flag==TRUE)
			{

				if(gpPhyPriv->wan_sel==SCU_WAN_CONF_REG_WAN_SEL_XGSPON || gpPhyPriv->wan_sel==SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G)	//julia_20220511	//ALU XGSPON OLT is not stable when laser on.
				{
					gpPhyPriv->phy_unexpected_isr_flag=FALSE;
					PON_PHY_PRINT(PHY_MSG_INT,"Unexpected ISR happened, make it LOS\r\n");
					gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
					#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
					handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT);
					#endif
				}
			}
			
			poll_set_ready_cnt++;
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => READY(%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			
			#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event);
			#endif
		}

		else if(!(phyReadyStatus&&(!phyLosStatus)))
		{
			poll_set_los_cnt++;
			
			if (gpPhyPriv->phy_status == PHY_LINK_STATUS_UNKNOWN)	//,only execute when power on without laser. ang_20180502
			{
				#ifndef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
				phy_event.id = PHY_EVENT_TRANS_LOS_INT;
				phy_event_handler(&phy_event);
				#endif

				#if ASIC_SERDES
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);// ang_20180921
				#endif
				
				//disable rx
				read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
				write_data= read_data & (~EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE); //disable rx
				IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

			}

			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			gpPhyPriv->phy_unexpected_isr_flag=FALSE;
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => NO laser! (%d) ReadyStatus=%d,LosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
		}
	}
	
	phy_print_time();

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;

	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	if(PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_TRANS_LOS_INT;
		poll_los_cnt++;
	}
	else if(PHY_LINK_STATUS_READY==gpPhyPriv->phy_status)
	{
		phy_event.id = PHY_EVENT_PHYRDY_INT;
		poll_ready_cnt++;
	}
	#ifdef PON_PHY_STATUS_REPORT_BY_POLLING_ONLY
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		phy_event_handler(&phy_event);
	}
	pre_phy_status=gpPhyPriv->phy_status;
	#endif

	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - __END__ (%d)(%d)(%d) Get phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

	return PHY_SUCCESS;

}



int an7583_xgpon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;
	
	//temp=buf;
	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);

#ifdef	EN7581_HEC_TEST
	phy_reg_total_num=90;
	phy_reg_all=en7581_xgpon_phy_reg_all;

	switch(dbg_id)
	{
		case 0:
			printk("\r\n\r\n___________Read XGPON debug counter only(no reset)___________\n");
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			phy_uptime();
			break;
			
		case 1:
			
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			printk("DBG_CNT_CLEAR on 0x%08x\n",EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data|EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}

			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			printk("DBG_CNT_CLEAR off 0x%08x\n",EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);
			CLRPOSITION(read_data,EN7581_XGPON_PHY_DBG_CTRL_DBG_CNT_CLEAR);	//ang_20180208
			IO_SPHYREG(EN7581_XGPON_PHY_DBG_CTRL,read_data);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_DBG_CTRL);
			printk("Read  EN7581_XGPON_PHY_DBG_CTRL(0x%x)=0x%08x\n",EN7581_XGPON_PHY_DBG_CTRL,read_data);

			break;

		case 2:
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("FEC_DECODE_CNT_CLR on 0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			IO_SPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data|EN7581_XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);

			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("FEC_DECODE_CNT_CLR off 0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);
			CLRPOSITION(read_data,EN7581_XGPON_PHY_PHY_PM_CNT_CLR_FEC_DECODE_CNT_CLR);	//ang_20180208
			IO_SPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("Read  EN7581_XGPON_PHY_PHY_PM_CNT_CLR(0x%x)=0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data);

			break;
			
		case 3:
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("PSBD_HEC_ERR_CNT_CLR on 0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			IO_SPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data|EN7581_XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
		
		
			printk("%-44s	 address=value\r\n","_____ XGPON CNT_____");
			for(i=33;i<=41;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}			
			for(i=43;i<=45;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=47;i<=47;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			for(i=74;i<=77;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			printk("%-44s	 address=value\r\n","_____ XGPON STATUS_____");
			for(i=42;i<=42;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=46;i<=46;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=50;i<=50;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=58;i<=58;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=78;i<=78;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}			
			printk("%-44s	 address=value\r\n","_____ XGPON FEC HEC PON-ID_____");
			for(i=48;i<=49;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=80;i<=82;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=84;i<=85;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("PSBD_HEC_ERR_CNT_CLR off 0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);
			CLRPOSITION(read_data,EN7581_XGPON_PHY_PHY_PM_CNT_CLR_PSBD_HEC_ERR_CNT_CLR);	//ang_20180208
			IO_SPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data);
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_PHY_PM_CNT_CLR);
			printk("Read  EN7581_XGPON_PHY_PHY_PM_CNT_CLR(0x%x)=0x%08x\n",EN7581_XGPON_PHY_PHY_PM_CNT_CLR,read_data);
		
			break;	
			
		case 10:
		
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_SFP_STA);
			printk("Read EN7581_XGPON_PHY_SFP_STA=0x%8x\n",read_data);
			printk("	 _MOD_ABS_ST      =0x%8x\n",read_data&EN7581_XGPON_PHY_SFP_MOD_ABS_ST);
			printk("	 _TX_FAULT_ST     =0x%8x\n",read_data&EN7581_XGPON_PHY_SFP_TX_FAULT_ST);
			printk("	 _TX_SD_ST  	  =0x%8x\n",read_data&EN7581_XGPON_PHY_SFP_TX_SD_ST);
			printk("	 _RX_LOS_ST	 	  =0x%8x\n",read_data&EN7581_XGPON_PHY_SFP_RX_LOS_ST);
		
			break;	

		case 20:
		case 21:
		case 22:
			REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 1;
		
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(dbg_id-20)|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);	

			break;			

		case 23:
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0x0ff00ff0) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER=0x%x \n",read_data);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0xa00ff00a) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER=0x%x \n",read_data);
		
			REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 1;
		
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_PATTERN_SEL(dbg_id-20)|EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_EN) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);		
		
			break;
			
		case 24:
		case 25:
		case 26:
		
			break;
		
		case 27:
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER,0x0ff00ff0) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_LOWER=0x%x \n",read_data);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER,0xa00ff00a) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CFG_PATTERN_UPPER=0x%x \n",read_data);

			break;
		
		case 28:
			IO_SPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL,EN7581_XGPON_PHY_XG_CONTINUE_CTRL_CONTINUE_DISABLE) ;
			read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_CONTINUE_CTRL);
			printk("Set EN7581_XGPON_PHY_XG_CONTINUE_CTRL=0x%x \n",read_data);		
			
			REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
			gpPhyPriv->phyCfg.flags.rogue = 0;
		
			break;
		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
#endif
	return PHY_SUCCESS;

}



int an7583_xgpon_phy_pma_reset(char* buf)
{

	uint read_data = 0;
	uint write_data = 0;

	//PON_PHY_PRINT(PHY_MSG_INT,"[%s:%d]\r\n",__FUNCTION__,__LINE__);	//ang_20181010

	
#if ASIC_SERDES	// for ASIC

	//reset PCS logic, hold PCS reset
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N,EN7581_XGPON_PHY_XG_PHY_RST_N_ALL) ;
	read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N);
	PON_PHY_PRINT(PHY_MSG_DBG,"Hold EN7581_XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
	//mdelay(1);
	phy_delay1ms(1);

	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	
	PON_PHY_PRINT(PHY_MSG_DBG,"XGPON PMA Reset! \n");	//ang_20181010
	//mdelay(1);
	phy_delay1ms(1);
	
//reset PCS logic, release PCS reset
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N,EN7581_XGPON_PHY_XG_PHY_RST_N_DISABLE) ;
	read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PHY_RST_N);
	PON_PHY_PRINT(PHY_MSG_DBG,"Release EN7581_XGPON_PHY_XG_PHY_RST_N=0x%x \n",read_data);
	//mdelay(1);
	phy_delay1ms(1);

//enable rx	
	read_data=IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL);
	write_data= read_data | EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL_RX_ENABLE;	//enable rx
	IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_RX_SYNC_CTRL, write_data);

#endif

	return PHY_SUCCESS;
}

#define _I_AM_XEPON_SPLIT_LINE_

/////////////////////////////////////////////////////////////////////////////////////
// en7581_xepon_func
/////////////////////////////////////////////////////////////////////////////////////

int an7583_xepon_pon_phy_reset(char* buf)
{
	uint val = 0;

	PON_PHY_PRINT(PHY_MSG_INT,"[%s:%d]\r\n",__FUNCTION__,__LINE__);
	
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
	val |=0x01;
	SET_SCU_RST_RG(val);
	udelay(1);
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);
	//release xpon phy top reset 
	val &=~(0x01);
	SET_SCU_RST_RG(val);		
	val = GET_SCU_RST_RG(); 
	//printk("\r\nGET_SCU_RST_RG=0x%x\r\n",val);		
#else
	val = IO_GPHYREG(TOP_RST_CTRL_SW2);
	val |=0x01;
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
	udelay(1);
	//release xpon phy top reset 
	val &=~(0x01);
	IO_SPHYREG(TOP_RST_CTRL_SW2, val);
#endif


	return PHY_SUCCESS;
}


int an7583_xepon_phy_dump(char* buf)
{
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=172;
	phy_reg_all=en7581_xepon_phy_reg_all;

	printk("\r\n%-44s	 address=value\r\n","_____ XEPON REG DUMP _____");
	for(i=0;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYREG((phy_reg_all+i)->addr);
		PON_PHY_PRINT(PHY_MSG_TRACE,"%-44s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	
#if	PHY_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=phy_reg_total_num;i++)
	{
		IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
		printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif
	

	return PHY_SUCCESS;
}


int an7583_xepon_phy_dev_init(char* buf)
{
	uint read_data = 0;
	uint write_data = 0;

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d]\r\n",__FUNCTION__,__LINE__);

	/* switch GPIO to XPON mode*/
#ifdef TCSUPPORT_CPU_ARMV8 
	/* switch GPIO to XPON mode*/		
	read_data = GET_IOMUX_CTRL_3(); //GPIO_PON_MODE_7523	
	write_data =(read_data | 0x01); //bit [0]: 1	
	SET_IOMUX_CTRL_3(write_data); //0-->1 //GPIO_PON_MODE_7523
#else 
		
	/* switch GPIO to XPON mode*/
	read_data = IO_GREG(IOMUX_Control_1_register);                                               
	read_data = read_data | (GPIO_PON_MODE) | (RG_PON_I2C_MODE) ;
	IO_SREG(IOMUX_Control_1_register, read_data);                          
#endif 	

	

	/*sigdet*/
	//read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	//read_data = read_data & ~(PHY_SQUELCH_DEGLITCH);
	//IO_SPHYREG(PHY_CSR_PHYSET3, read_data);

#ifdef TCSUPPORT_CPU_ARMV8 //julia_asic
	/*after sw reset, register can be modify by PBUS*/
	read_data = GET_SCU_RST_ACC_CHK_BMA(); //TOP_CSR_PBUS_CTRL
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	SET_SCU_RST_ACC_CHK_BMA(read_data); //TOP_CSR_PBUS_CTRL
	/* transceiver power initial setting*/
	read_data = GET_GPIO_SSR(); //TOP_CSR_GPIO_SHARE
	read_data = read_data & ~(TOP_LED1_MODE);
	SET_GPIO_SSR(read_data); //TOP_CSR_GPIO_SHARE
#else	
	/*after sw reset, register can be modify by PBUS*/
	read_data = IO_GREG(TOP_CSR_PBUS_CTRL);
	read_data = read_data & ~(PHY_PBUS_CTRL_DISABLE);
	IO_SREG(TOP_CSR_PBUS_CTRL, read_data);
	/* transceiver power initial setting*/
	read_data = IO_GREG(TOP_CSR_GPIO_SHARE);
	read_data = read_data & ~(TOP_LED1_MODE);
	IO_SREG(TOP_CSR_GPIO_SHARE, read_data);
#endif
	//enable PCS RX control 	david 20170124
	read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
	PON_PHY_PRINT(PHY_MSG_ERR,"Read XEPON PCS RX CTRL=0x%x \n",read_data);

	#if ASIC_SERDES
	IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,read_data|EN7581_XEPON_PCS_RX_CTRL_CFG_SYNC_TIMES(0)) ;
//	IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,read_data|EN7581_XEPON_PCS_RX_CTRL_CFG_PCS_RX_EN|EN7581_XEPON_PCS_RX_CTRL_CFG_SYNC_TIMES(0)) ;
	#endif 
	#if (XILINX_SERDES)
	IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,read_data|EN7581_XEPON_PCS_RX_CTRL_CFG_PCS_RX_EN) ;
	#endif

	read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
	PON_PHY_PRINT(PHY_MSG_ERR,"Set XEPON PCS RX CTRL=0x%x \n",read_data);

	//configure ISR_EN
	//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);   //EN7581_XEPON_PCS_INT_SYNC_OK_EN); //
	//phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE); 

	/* Enable BIP error counter */
	
	/* change the guard time pattern */
	//gpPhyPriv->phyGuardBitDelm = 0xAAAB5983;
	//IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, 0xAAAB5983);

	/* clear phy register*/
	
	return PHY_SUCCESS ;

}

int an7583_xepon_phy_int_config(char* buf)
{
	UINT32 *mode = (UINT32 *)buf;
	UINT32 *int_type = (UINT32 *)buf+1;

	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 phyIntStatus;

	if(PHY_INT_EN_MODE_DEFAULT==(*mode))
	{
		write_data=(EN7581_XEPON_PCS_INT_SYNC_OK_EN\
					|EN7581_XEPON_PCS_INT_SYNC_LOSS_EN\
					|EN7581_XEPON_PCS_INT_LASER_RX_LOSS_EN\
					|EN7581_XEPON_PCS_INT_NOT_LASER_RX_LOSS_EN);									

		/*
			
					|EN7581_XEPON_PCS_INT_TX_DET_OVERRUN\
					|EN7581_XEPON_PCS_INT_TX_DET_UNDERRUN\
					|EN7581_XEPON_PCS_INT_TX_GB_OVERRUN_EN\
					|EN7581_XEPON_PCS_INT_TX_GB_UNDERRUN_EN\
					|EN7581_XEPON_PCS_INT_RX_GB_OVERRUN\
					|EN7581_XEPON_PCS_INT_BER_MONITOR_DONE\
					|EN7581_XEPON_PCS_INT_BER_HIGH_FLAG

					|EN7581_XEPON_PCS_INT_CDR_STABLE\
					|EN7581_XEPON_PCS_INT_CDR_STABLE_LOSS
		
					|EN7581_XEPON_PCS_INT_RX_GB_EMPTY_APPEAR\
					
					|EN7581_XEPON_PCS_INT_PHYA_TX_RDY\
					|EN7581_XEPON_PCS_INT_PHYA_RX_RDY\	

					|EN7581_XEPON_PCS_INT_RX_SOF_NO_EOF\
					|EN7581_XEPON_PCS_INT_RX_EOF_NO_SOF\

					|EN7581_XEPON_PCS_INT_TX_SD_NORMAL\
					|EN7581_XEPON_PCS_INT_TXSD_NEQUAL_TXBST\

					|EN7581_XEPON_PCS_INT_PHYA_TX_RDY_LOSS\
					|EN7581_XEPON_PCS_INT_PHYA_RX_RDY_LOSS);

		*/
		
		PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy use default int_type:0x%.8x\n", write_data);
	}
	else
	{
		write_data=(*int_type);
		PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config type     :0x%.8x\n", write_data);
	}

	//NOT Clear PHY ISR Status, Let PHY_ISR() function deal with the ISR Status state machine
	//Otherwise, we may miss the ISR status after PHY init and before register ISR to system

	read_data = IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);	
	PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config read     :0x%.8x\n", read_data);

	IO_SPHYREG(EN7581_XEPON_PCS_INT_EN, write_data);
	read_data = IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
	PON_PHY_PRINT(PHY_MSG_TRACE,"XEPON Phy_Int_Config write    :0x%.8x\n", read_data);
	

	return PHY_SUCCESS;
}

int an7583_xepon_phy_get_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;


	api_data->ret = PHY_SUCCESS;

	switch(api_data->cmd_id)
	{
		case PON_GET_PHY_LOS_STATUS:
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
			if( (read_data & EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) == EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) //it is transceiver sfp rx LOSS no PHY digital
			{
				api_data->ret = PHY_LOS_HAPPEN;
			}
			else
			{
				api_data->ret = PHY_NO_LOS_HAPPEN;
			}
			break;
			
		case PON_GET_PHY_READY_STATUS:
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);//read bit[18:20]
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy Ready. \n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				//PON_PHY_PRINT(PHY_MSG_DBG, "Phy not Ready. \n");
				api_data->ret = PHY_FALSE;
			}
			break; 
			
		case PON_GET_PHY_IS_SYNC:
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
			if( (read_data & EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) != EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) //it is transceiver sfp rx LOSS no PHY digital
			{
				read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);
				if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
				{
					api_data->ret = PHY_TRUE; // PHY Sync OK 
					break;
				}
				else{
					api_data->ret = PHY_FALSE; // PHY Not Sync 
			break;
				}
			}
			else{
				api_data->ret = PHY_FALSE; // PHY Not Sync 
				break;
			}

		case PON_GET_PHY_MODE:
			api_data->ret = gpPhyPriv->phyCfg.flags.mode;
			break;
			
		case PON_GET_PHY_TX_LONG_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txLongFlag;
			break;

		case PON_GET_PHY_TRANS_STATUS:
			phy_trans_param_status_real(&api_data->trans_status->params);
			api_data->trans_status->alarms = phy_tx_alarm() | phy_rx_power_alarm();
			break;

		case PON_GET_PHY_GET_TX_POWER_EN_FLAG:
			api_data->ret = gpPhyPriv->phyCfg.flags.txPowerEnFlag;
			break;

		case PON_GET_PHY_BIP_COUNTER:
			
			break;

		case PON_GET_PHY_RX_FEC_COUNTER:
			api_data->rx_fec_cnt->correct_bytes=IO_GPHYREG(EN7581_XEPON_PCS_ALL_ERR_CORR_CDWD);	
			api_data->rx_fec_cnt->correct_codewords=IO_GPHYREG(EN7581_XEPON_PCS_ALL_ERR_CORR_CDWD);
			api_data->rx_fec_cnt->uncorrect_codewords=IO_GPHYREG(EN7581_XEPON_PCS_ALL_ERR_NO_CORR_CDWD);	// no correctable bytes counter for XEPON
			api_data->rx_fec_cnt->total_rx_codewords=IO_GPHYREG(EN7581_XEPON_PCS_ALL_CDWD);
			api_data->rx_fec_cnt->fec_seconds=IO_GPHYREG(EN7581_XEPON_PCS_FEC_ERR_SECOND); 
			break;

		case PON_GET_PHY_RX_FRAME_COUNTER:
			api_data->rx_frame_cnt->frame_count_high = 0x0;
			api_data->rx_frame_cnt->frame_count_low = IO_GPHYREG(EN7581_XEPON_PCS_RX_SOF_NUM);;
			break;

		case PON_GET_PHY_RX_FEC_GETTING:
			//api_data->ret = (IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG)>>31);//cheng_20220328
			if(((IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG)>>31) & 0x01) == 1)//cheng_20220328 call trace
			{
				api_data->ret = PHY_TRUE;
			}
			else
			{
				api_data->ret = PHY_FALSE;
			}
			break;

		case PON_GET_PHY_RX_FEC_STATUS:
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			if((read_data & EN7581_XEPON_PHY_RX_FEC) == EN7581_XEPON_PHY_RX_FEC)
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC on.\n");
				api_data->ret = PHY_TRUE;
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_DBG, "RX FEC off.\n");
				api_data->ret = PHY_FALSE;
			}
			break;

		case PON_GET_PHY_TX_FEC_STATUS:
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G)
			{
				read_data = IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
				if((read_data & EN7581_XEPON_SYMM_PHY_TX_FEC) == EN7581_XEPON_SYMM_PHY_TX_FEC)
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
					api_data->ret = PHY_TRUE;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
					api_data->ret = PHY_FALSE;
				}
			}
			else if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G)
			{
				  read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_STATUS);
				if((read_data & EN7581_XEPON_ASYM_PHY_TX_FEC) == EN7581_XEPON_ASYM_PHY_TX_FEC)
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC on. \n");
					api_data->ret = PHY_TRUE;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_DBG, "TX FEC off. \n");
					api_data->ret = PHY_FALSE;
				}
			}
			else 
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "Mode Error:wan_sel =%d \n",gpPhyPriv->wan_sel);
				api_data->ret = PHY_NO_API;
			}
				
			break;

		case PON_GET_PHY_TX_BURST_GETTING:

#if ASIC_SERDES

           	//read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_1); //julia_7583_pma
           	read_data = AN7583_Get_PMA_XPON_TX_Force_1();
			if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_1_NORMAL_MODE_MASK )
			{
				api_data->ret = PHY_TX_BURST_MODE;
			}
			else
			{
	           //read_data = IO_GPHYREG(EN7581_XPON_PMA_DA_XPON_TX_FORCE_2);
				read_data = AN7583_Get_PMA_XPON_TX_Force_2();
				if( (read_data & EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)== EN7581_XPON_PMA_DA_XPON_TX_FORCE_2_FORCE_MODE_MASK)
				{
					api_data->ret = PHY_TX_CONT_MODE;
				}
				else
				{
					api_data->ret = PHY_TX_BURST_MODE;
				}
			}
#endif
			break;

		case PON_GET_PHY_TRANS_TX:

			#if 0 //ASIC_SERDES //EN7580_ASIC //julia_debug 7583 need recodeing
			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_1);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_1_TX_SD_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_sd_inv_status = PHY_DISABLE;

			read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_BURST_EN_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_burst_en_inv_status = PHY_DISABLE;
			if((read_data & ~(EN7581_XPON_PMA_XPON_SETTING_0_TX_FAULT_INV_MASK))!= 0)
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_ENABLE;
			else
				api_data->tx_trans_cfg->trans_tx_fault_inv_status = PHY_DISABLE;
			#endif
			
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_GET_PHY_TRANS_TX -- 7583 mark\n"); //julia_7583_pma
			break;

		case PON_GET_PHY_TRANS_RX_GETTING:

#if 0 //ASIC_SERDES

            read_data = IO_GPHYREG(EN7581_XPON_PMA_XPON_SETTING_0);
			if((read_data & EN7581_XPON_PMA_XPON_SETTING_0_RX_SD_INV) == 0)
				api_data->ret = PHY_DISABLE;
			else
				api_data->ret = PHY_ENABLE;
#endif
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_GET_PHY_TRANS_RX_GETTING -- 7583 mark\n"); //julia_7583_pma

			break;

		case PON_GET_PHY_ROUND_TRIP_DELAY:

			break;

        case PON_GET_PHY_LOF_STATUS:
			//read_data = IO_GPHYREG(EN7581_LOF);	  
	  		api_data->ret=0;

			break;
				
		case PON_GET_PHY_INIT_STATUS: //ang_20190821
			api_data->ret = gpPhyPriv->phy_init_done;
			break;
				
		default:
			PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
			api_data->ret = PHY_NO_API;
			break;
	}

	return PHY_SUCCESS; //ang_20180208
}

int an7583_xepon_phy_set_api_dispatch(char* buf)
{
	xpon_phy_api_data_t * api_data = (xpon_phy_api_data_t *) buf;
	UINT32 read_data = 0;
	UINT32 write_data = 0;
	int rogue_pattern = 0;

    api_data->ret = PHY_SUCCESS;
    
    switch(api_data->cmd_id)
    {
        case PON_SET_PHY_MODE_CONFIG:
            api_data->ret = phy_mode_config(api_data->phy_mode_cfg->mode, api_data->phy_mode_cfg->txEnable);
            break;
			
		case PHY_SET_ERR_CNT_EN:
		case PHY_SET_BIP_CNT_EN:
		case PHY_SET_FM_CNT_EN:
			break; 

        case PON_SET_PHY_RESET_COUNTER:
            IO_SPHYREG(EN7581_XEPON_PCS_CLR_ALL_NUM, EN7581_XEPON_PCS_CLR_ALL_NUM_CLR);
            break;
			
        case PON_SET_PHY_START:
            api_data->ret = xpon_phy_start();
            break;

        case PON_SET_PHY_STOP:
            api_data->ret = xpon_phy_stop();
            break;

        case PON_SET_PHY_FW_READY:
            break;

        case PON_SET_PHY_COUNTER_CLEAR:
			IO_SPHYREG(EN7581_XEPON_PCS_CLR_ALL_NUM, EN7581_XEPON_PCS_CLR_ALL_NUM_CLR);
            break;

        case PON_SET_EPONMODE_PHY_RESET:
            break;

        case PON_SET_PHY_BIT_DELAY:
            break;

        case PON_SET_PHY_TX_POWER_CONFIG:
            phy_tx_power_config(*api_data->data);
            break;

        case PON_SET_PHY_ROGUE_PRBS_CONFIG:

			#if ASIC_SERDES
			if(gpPhyPriv->first_plugin_flag == TRUE)
			{
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
				gpPhyPriv->first_plugin_flag = FALSE;
				gpPhyPriv->pma_init_done= TRUE;
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
				mdelay(350);	//add by david 20190508  
				phy_trans_power_switch(PHY_ENABLE);	
			}	// by david 20190522	incase onu startup without OLT DS Signal, RX & TX could L2R lock.
			
			an7583_set_tx_pattern(api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->tx_d_in_timeslot,api_data->phy_rogue_cfg->rogue_pattern);
			
			#endif
			
#if XILINX_SERDES			
			if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G)
			{
				PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern_sel=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1))
				{
					gpPhyPriv->phyCfg.flags.rogue = 1;
					if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 1)
					{
						api_data->phy_rogue_cfg->tx_d_in_timeslot = 0;
					}else if (api_data->phy_rogue_cfg->tx_d_in_timeslot == 0)
					{
						api_data->phy_rogue_cfg->tx_d_in_timeslot = 1;
					}

					rogue_pattern = api_data->phy_rogue_cfg->rogue_pattern;
						
					if (api_data->phy_rogue_cfg->rogue_pattern == 0)
					{
						api_data->phy_rogue_cfg->rogue_pattern = 1;
					}else if (api_data->phy_rogue_cfg->rogue_pattern == 1)
					{
						api_data->phy_rogue_cfg->rogue_pattern = 0;
					}
				
					IO_SPHYREG(EN7581_XEPON_PCS_CONTINUE_CTRL,EN7581_XEPON_PCS_CONTINUE_CTRL_EN\
														|EN7581_XEPON_PCS_CONTINUE_CTRL_DATA_SEL(api_data->phy_rogue_cfg->rogue_pattern)\
														|EN7581_XEPON_PCS_CONTINUE_CTRL_MODE_SEL(api_data->phy_rogue_cfg->tx_d_in_timeslot)) ;
					read_data=IO_GPHYREG(EN7581_XEPON_PCS_CONTINUE_CTRL);
					PON_PHY_PRINT(PHY_MSG_ERR,"Set EN7581_XEPON_PCS_CONTINUE_CTRL(0x%x)=0x%x \n",EN7581_XEPON_PCS_CONTINUE_CTRL,read_data);		
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue %s\n",(EN7581_XEPON_PCS_CONTINUE_CTRL_EN&read_data)?"ON":"OFF");
					PON_PHY_PRINT(PHY_MSG_ERR,"Rogue pattern %d (0=PRBS7,1=PRBS23,2=PRBS31,3=USER_CONFIG)\n",rogue_pattern);
					PON_PHY_PRINT(PHY_MSG_ERR,"In_timeslot %s\n",(EN7581_XEPON_PCS_CONTINUE_CTRL_MODE_NORMAL&read_data)?"Normal":"Rogue");
					if(3==api_data->phy_rogue_cfg->rogue_pattern)
					{
						PON_PHY_PRINT(PHY_MSG_DBG,"Note : Rogue pattern is USER_CONFIG\n\tPlease set data by follow cmds:\n\tsys memwl bfaf102c <high_32bit_data>\n\tsys memwl bfaf1030 <low_32bit_data>\n");
					}
				} 
				else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE) 
				{
					IO_SPHYREG(EN7581_XEPON_PCS_CONTINUE_CTRL,EN7581_XEPON_PCS_CONTINUE_CTRL_DISABLE\
														|EN7581_XEPON_PCS_CONTINUE_CTRL_DATA_SEL(0)\
														|EN7581_XEPON_PCS_CONTINUE_CTRL_MODE_SEL(0)) ;
					read_data=IO_GPHYREG(EN7581_XEPON_PCS_CONTINUE_CTRL);
					PON_PHY_PRINT(PHY_MSG_ERR,"Set EN7581_XEPON_PCS_CONTINUE_CTRL(0x%x)=0x%x \n",EN7581_XEPON_PCS_CONTINUE_CTRL,read_data);	
					PON_PHY_PRINT(PHY_MSG_ERR, "Rogue %s\n",(EN7581_XEPON_PCS_CONTINUE_CTRL_EN&read_data)?"ON":"OFF");
					gpPhyPriv->phyCfg.flags.rogue = 0;
				}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
					api_data->ret = PHY_FAILURE;
				}
			}
			else if(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G)
			{
				PON_PHY_PRINT(PHY_MSG_ERR,"rogue_mode=%d, rogue_pattern_sel=%d, tx_d_in_timeslot=%d\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
				if((api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_ROGUE_MODE)&&(api_data->phy_rogue_cfg->rogue_pattern<=3)&&(api_data->phy_rogue_cfg->tx_d_in_timeslot<=1)) 
				{
        			gpPhyPriv->phyCfg.flags.rogue = 1;
					
					/* EPON mode set continue mode */
					//IO_SBITS(0xBFAF0108,0x80);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYSET3) ;
					write_data = read_data | 0x80;
					IO_SPHYREG(EN7581_XEPON_1G_PHYSET3,write_data);
					
					PON_PHY_PRINT(PHY_MSG_DBG, "Phy_Rogue_PRBS mode\n");
			     	} 
				else if(api_data->phy_rogue_cfg->rogue_onoff == PHY_TX_NORMAL_MODE) 
				{
					//IO_CBITS(0xBFAF0108,0x80);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYSET3) ;
					write_data = read_data & ~0x80;
					IO_SPHYREG(EN7581_XEPON_1G_PHYSET3,write_data);	
			        gpPhyPriv->phyCfg.flags.rogue = 0;
			        PON_PHY_PRINT(PHY_MSG_DBG, "Phy_NO_Rogue_PRBS mode\n");
		    	}
				else
				{
					PON_PHY_PRINT(PHY_MSG_ERR, "error input: Rogue mode %d (must <=1), pattern %d(must <=3), in_timeslot %d(must <=1)\n",api_data->phy_rogue_cfg->rogue_onoff,api_data->phy_rogue_cfg->rogue_pattern,api_data->phy_rogue_cfg->tx_d_in_timeslot);
					api_data->ret = PHY_FAILURE;
				}
			}
				#endif

            break;

        case PON_SET_PHY_DEV_INIT:
            api_data->ret = phy_dev_init();
            break;

        case PON_SET_PHY_TRANS_POWER_SWITCH:
            api_data->ret = phy_trans_power_switch(*api_data->data);
            break;

        case PON_SET_PHY_RESTORE_PREAMBLE_AND_GUARD_BIT:
            break;

        case PON_SET_PHY_TRANS_MODEL_SETTING:
			#if 0
            if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
			{
				
				//tx brust en
#if XILINX_SERDES
				write_data = ~phy_trans_iot_list[gpPhyPriv->trans_index].tx_setting;
#else
				write_data = phy_trans_iot_list[gpPhyPriv->trans_index].tx_setting;
#endif
				read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_TRANS_BRUST_INV(0x%x)=0x%x \n",EN7583_TRANS_BRUST_INV,read_data);
				IO_SPHYA_REG_BITS(EN7583_TRANS_BRUST_INV, 8, 8, write_data); 
				read_data=IO_GPHYREG(EN7583_TRANS_BRUST_INV);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_TRANS_BRUST_INV(0x%x)=0x%x \n",EN7583_TRANS_BRUST_INV,read_data);
							
				//Rx los
#if XILINX_SERDES
				write_data = ~phy_trans_iot_list[gpPhyPriv->trans_index].rx_setting;
#else
				write_data = phy_trans_iot_list[gpPhyPriv->trans_index].rx_setting;
#endif
				read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Read EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x \n",EN7583_PON_PCS_RX_SD_CTRL,read_data);
				IO_SPHYA_REG_BITS(EN7583_PON_PCS_RX_SD_CTRL, 0, 0, write_data); 
				read_data=IO_GPHYREG(EN7583_PON_PCS_RX_SD_CTRL);
				PON_PHY_PRINT(PHY_MSG_TRANS, "Set EN7583_PON_PCS_RX_SD_CTRL(0x%x)=0x%x \n",EN7583_PON_PCS_RX_SD_CTRL,read_data);
				
            }
			#endif
			PON_PHY_PRINT(PHY_MSG_ERR, "PON_SET_PHY_TRANS_MODEL_SETTING -- 7583 mark\n"); //julia_7583_pma
            break;

		case PON_SET_PHY_LOGIC_RESET:

			break;
			
		case PON_SET_PHY_SCU_RST:
			#if ASIC_SERDES
			phy_trans_power_switch(PHY_TX_DIS_ON_HW_ONLY); //tx_dis_reset_protection
			gpPhyPriv->debugLevel_backup=gpPhyPriv->debugLevel;
			gpPhyPriv->debugLevel=PHY_MSG_OFF;
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			//PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);
			xPON_Phy_Reset();
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
			if(gpPhyPriv->rx_fec_setting == PHY_ENABLE)
			{
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, write_data|EN7581_XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			}
			else if(gpPhyPriv->rx_fec_setting == PHY_DISABLE)
			{
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, write_data&~EN7581_XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (%x)=0x%.8x\n", EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			}
			xpon_phy_start();
			gpPhyPriv->debugLevel=gpPhyPriv->debugLevel_backup;
			phy_trans_power_switch(PHY_TX_DIS_RESTORE_BY_SW); //tx_dis_reset_protection
			PON_PHY_PRINT(PHY_MSG_INT,"PHY_SCU_RESET\n");
			#endif
			break;

        case PON_SET_PHY_RX_FEC_SETTING:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_ENABLE;
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, write_data|EN7581_XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Enable (%x)=0x%.8x\n", EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->rx_fec_setting=PHY_DISABLE;
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, write_data&~EN7581_XEPON_PCS_RX_CTRL_CFG_FEC_DEC_EN);
				write_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Rx FEC Disable (%x)=0x%.8x\n", EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			}
			else
			{
				gpPhyPriv->rx_fec_setting=PHY_DEFAULT;
			}
            break;

        case PON_SET_PHY_TX_BURST_CONFIG:
            if(((*api_data->data) != PHY_TX_BURST_MODE) && ((*api_data->data) != PHY_TX_CONT_MODE))
			{
				PON_PHY_PRINT(PHY_MSG_ERR, "error input : burst_mode = %d\n",(*api_data->data));
				api_data->ret = PHY_FAILURE;
			}
			else
			{
				IO_SPHYREG(EN7581_XEPON_PCS_CONTINUE_CTRL, (EN7581_XEPON_PCS_CONTINUE_CTRL_MODE_NORMAL\
													|EN7581_XEPON_PCS_CONTINUE_CTRL_DATA_SEL(0x1)\
													|(*api_data->data)));
				//	EN7581_XEPON_PCS_CONTINUE_CTRL_DATA_SEL: 
				//										0x0  PRBS23
				//										0x1  PRBS 7
				//										0x2  PRBS31
				//										0x3  User Config 
				//	IO_SPHYREG(EN7581_XEPON_PCS_CONTINUE_USER_CFG_DATA0,0);
				//	IO_SPHYREG(EN7581_XEPON_PCS_CONTINUE_USER_CFG_DATA1,0);
				gpPhyPriv->phyCfg.flags.txLongFlag = (((*api_data->data)==PHY_TX_CONT_MODE)? PHY_TRUE : PHY_FALSE);
				api_data->ret = PHY_SUCCESS;
			}
			
            break;

        case PON_SET_PHY_GPON_DELIMITER_GUARD:
             break;

        case PON_SET_PHY_TRANS_TX_SETTINGS:
            break;

        case PON_SET_PHY_TRANS_RX_SETTING:
            break;

        case PON_SET_PHY_GPON_PREAMBLE:
            break;

        case PON_SET_PHY_GPON_EXTEND_PREAMBLE:
            break;
			
		case PHY_SET_TX_FEC_EN:
            if((*api_data->data) == PHY_ENABLE)
			{
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,read_data|EN7581_XEPON_PCS_TX_CTRL_CFG_FEC_EN);
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Enable (0x%x)=0x%.8x\n", EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
				IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG, read_data&~EN7581_XEPON_PCS_TX_CTRL_CFG_FEC_EN);
				read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
				PON_PHY_PRINT(PHY_MSG_TRACE,"Tx FEC Disable (0x%x)=0x%.8x\n", EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			break;			
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
#ifdef TCSUPPORT_MT7570
		case PON_SET_PHY_EN7571_INIT_DONE:

			gpPhyPriv->en7571_init_done = FALSE;
			PON_PHY_PRINT(PHY_MSG_ERR, "ERROR: Should not init 7571 in XEPON mode !\r\n");

			break;

		case PON_SET_PHY_RX_CDR:
#if ASIC_SERDES

			if((*api_data->data) == PHY_ENABLE)
			{
				xpon_phy_stop();
				gpPhyPriv->phy_init_done=FALSE;
				gpPhyPriv->first_plugin_flag = TRUE; 
				pon_phy_mod_init();		// xpon_phy_10g module reset (the most complete reset cmd)
				xpon_phy_start();
			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				xpon_phy_stop();
				xpon_init(gpPhyPriv->wan_sel);
				//xpon_pma_param_opt(pma_hi_rate_opt_val); //julia_7583_pma mark by morris suggest
				fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); 							// reset pma and first_plug_in
				fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	// force RX L2R lock
				phy_int_config(PHY_INT_EN_MODE_MANUAL, PHY_INT_EN_DISABLE);		//disable PON_PHY isr mask
				PHY_STOP_TIMER(gpPhyPriv->event_poll_timer);	// stop event_poll_timer	
				mdelay(350);
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
#endif
			break;
#endif

#ifdef LDDLA_SUPPORT_SET_TX_MODE
		case PON_SET_PHY_LDDLA_AUTO_MODE:
			if((*api_data->data) == PHY_ENABLE)
			{
				gpPhyPriv->lddla_auto_mode= TRUE; 

			}
			else if((*api_data->data) == PHY_DISABLE)
			{
				gpPhyPriv->lddla_auto_mode= FALSE;
			}
			else
			{
				api_data->ret = PHY_FAILURE;
			}
			PON_PHY_PRINT(PHY_MSG_ERR, "MAC set lddla_auto_mode = %s\n",(gpPhyPriv->lddla_auto_mode==TRUE)?"TRUE":"FALSE");
			break;
#endif

        default:
            PON_PHY_PRINT(PHY_MSG_ERR,"[%s:%d] type %d, Unknown cmd_id=0x%x !\r\n",__FUNCTION__,__LINE__,api_data->api_type,api_data->cmd_id);
            api_data->ret = PHY_NO_API;
            break;
    }

	return PHY_SUCCESS ;	//ang_20180208
}



int an7583_xepon_phy_isr(char* buf)
{
	ulong flags;
	uint phyIntStatus, phyIntEn ;
	PHY_TransConfig_T trans_status;
	PPHY_TransConfig_T p_trans_status = & trans_status;
    PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_HW_IRQ };
	static uint isr_cnt=0,phy_rdy_cnt=0,phy_los_cnt=0,laser_no_los_cnt=0,phy_other_cnt=0,laser_los_cnt=0,phy_lof_cnt=0;
	uint read_data = 0,write_data = 0;
	static uint trans_laser_detected = FALSE;  //david 20170606
	//solve linux 3.18 compile issue

	
	phy_print_time();

	spin_lock(&gpPhyPriv->event_handle_lock);	
	isr_cnt++;	
	/*get interrupt status and clear*/
	phyIntStatus=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
	IO_SPHYREG(EN7581_XEPON_PCS_INT_STATUS, phyIntStatus);


#if XILINX_SERDES

	if (trans_laser_detected == TRUE)	
	{		
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_OK)) 
		{
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
		
			handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			phy_rdy_cnt++;
			trans_laser_detected = FALSE;			
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
		}
		else
		{
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
			trans_laser_detected = FALSE;
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected when detected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		}
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&((phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_LOSS)||(phyIntStatus & EN7581_XEPON_PCS_INT_LASER_RX_LOSS)))
	{
		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
	
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
		phy_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS " );
	}
	else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{
		
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_start trans_laser_detected=%d\r\n", trans_laser_detected);

		laser_no_los_cnt++;
		trans_laser_detected = TRUE;
		
		//julia_20220511
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST); 
		write_data = read_data | XILINX_SERDES_RST_ON;
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
		mdelay(10);
		IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
		mdelay(10);
		
		IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
		IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);
		
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_done trans_laser_detected=%d\r\n", trans_laser_detected);
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_TX_GB_OVERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_OVERRUN " );
	}

	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_TX_GB_UNDERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_UNDERRUN " );
	}
	else if(phyIntStatus & EN7581_XEPON_PCS_INT_BER_MONITOR_DONE)
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> EN7581_XEPON_PCS_INT_BER_MONITOR_DONE " );
		if(phyIntStatus & EN7581_XEPON_PCS_INT_BER_HIGH_FLAG)
		{
			PON_PHY_PRINT(PHY_MSG_INT," with EN7581_XEPON_PCS_INT_BER_HIGH_FLAG " );		
		}	
	}
	
	else
	{
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
	}
#endif

#if ASIC_SERDES

	if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{		
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_start trans_laser_detected=%d\r\n", trans_laser_detected);
		read_data = IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
		if((read_data& EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) != EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			laser_no_los_cnt++;
			trans_laser_detected = TRUE;
			phy_pma_reset_with_lock();
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);    // david 20240808
			PON_PHY_PRINT(PHY_MSG_INT,"XE_PHY_Int_EN=0x%.8x", read_data);
		}
		else
		{
			phy_other_cnt++;
			trans_laser_detected = FALSE;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Fake No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
		}		
	}
		
	else if(trans_laser_detected == TRUE)
	{		
		if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_OK)) 
		{
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				phy_rdy_cnt++;			
				trans_laser_detected = FALSE;			
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			}
			else{
				phy_other_cnt++;
				phyIntEn=IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX receive signal yet sync loss! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
			//	phy_pma_reset();
				// disable interrupt mask  david 20240808
				phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
				gpPhyPriv->event_poll_timer_value = 3500;
			}
		}
		else if((gpPhyPriv->phy_status != PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_LASER_RX_LOSS))
		{		
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			phy_los_cnt++;
			trans_laser_detected = FALSE;	
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LASER_RX_LOSS " );
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );				
		}	
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_LASER_RX_LOSS)&&(phyIntStatus & EN7581_XEPON_PCS_INT_NOT_LASER_RX_LOSS))
	{

		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
		phy_los_cnt++;
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;

		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LOS and NO_LOS int assert at same time. Reset_PMA_start \r\n");
		
		
		read_data = IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
		if((read_data& EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) != EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			laser_no_los_cnt++;
			trans_laser_detected = TRUE;
			phy_pma_reset_with_lock();
			phy_print_time();
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Reset_PMA_done \r\n");
		}
		else
		{
			phy_other_cnt++;
			trans_laser_detected = FALSE;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Fake No Laser Loss Reset_PMA_done trans_laser_detected=%d\r\n", trans_laser_detected);
		}

	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_LASER_RX_LOSS))
	{

		gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
		phy_los_cnt++;
		IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_LOS " );
		PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
		handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
		fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
	}
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_LOSS))
	{	

		gpPhyPriv->xe_syncloss_cnt++;

		phy_lof_cnt++ ;

		if(gpPhyPriv->xe_syncloss_cnt <= 3)
		{
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF, ");
			handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT) ;	
		}
		else //if sync loss int occur 3 times in a row, signal quality issue,  make it loss
		{
			//gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;	
			//phy_los_cnt++;
			// david 20240808: make continue LOF status as unknown, wait for polling to check
			gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN;	
			phy_other_cnt++;									
			// should not disable rx before Laser LOS
			//IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			//PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_SYNC_LOS " );
			handle_hw_irq_event(PHY_EVENT_TRANS_LOS_INT) ;
			
			// disable interrupt mask  david 20240808
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			gpPhyPriv->event_poll_timer_value = 3500;
			//fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);	//tdc_off & RX L2R
		}

/*
// for debug, ang_20200819

			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> RX_LOF. \r\n");
			handle_hw_irq_event(PHY_EVENT_PHY_LOF_INT) ;

			if(phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_OK) 	// LOF and SYNC_OK assert at same time.
			{
				read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);
				if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
				{
					gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
					phy_rdy_cnt++;			
					trans_laser_detected = FALSE;			
					PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY from LOF ");
					handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
				}
			}
*/
		
	}
	else if(((gpPhyPriv->phy_status == PHY_LINK_STATUS_UNKNOWN)||(gpPhyPriv->phy_status == PHY_LINK_STATUS_READY))&&(phyIntStatus & EN7581_XEPON_PCS_INT_SYNC_OK))
	{
		read_data = IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
		if((read_data& EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS) != EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS)
		{
			read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);
			if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
			{
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				phy_rdy_cnt++;			
				trans_laser_detected = FALSE;			
				PON_PHY_PRINT(PHY_MSG_INT,"<ISR> PHY_LINK_STATUS_READY ");
				handle_hw_irq_event(PHY_EVENT_PHYRDY_INT);
			}
		}
		else
		{
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			PON_PHY_PRINT(PHY_MSG_INT,"<RX_ENABLE OFF>" );
			trans_laser_detected = FALSE;
			phy_other_cnt++;
			phyIntEn=IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
			PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Please Plug In Fiber! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		}
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_TX_GB_OVERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_OVERRUN " );
	}
	
	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_READY)&&(phyIntStatus & EN7581_XEPON_PCS_INT_TX_GB_UNDERRUN))
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> TX_GB_UNDERRUN " );
	}
	else if(phyIntStatus & EN7581_XEPON_PCS_INT_BER_MONITOR_DONE)
	{
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> EN7581_XEPON_PCS_INT_BER_MONITOR_DONE " );
		if(phyIntStatus & EN7581_XEPON_PCS_INT_BER_HIGH_FLAG)
		{
			PON_PHY_PRINT(PHY_MSG_INT," with EN7581_XEPON_PCS_INT_BER_HIGH_FLAG " );		
		}	
	}

	else if((gpPhyPriv->phy_status == PHY_LINK_STATUS_LOS)&&(phyIntStatus & EN7581_XEPON_PCS_INT_LASER_RX_LOSS))
	{
		laser_los_cnt++;
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> LASER_LOS_INT <DO NOTHING>" );						
	}
	else{
		phy_other_cnt++;
		phyIntEn=IO_GPHYREG(EN7581_XEPON_PCS_INT_EN);
		PON_PHY_PRINT(PHY_MSG_INT,"<ISR> Unexpected! fw status=0x%x,INT EN=0x%08x",gpPhyPriv->phy_status,phyIntEn);
		gpPhyPriv->phy_status = PHY_LINK_STATUS_UNKNOWN; //ang_20200416
	}
#endif

	PON_PHY_PRINT(PHY_MSG_INT," XE_ISR=%d=%d(RDY)+%d(LOS)+%d(NO_LOS)+%d+%d+%d(LOF),Int_Sts=0x%08x\r\n",isr_cnt,phy_rdy_cnt,phy_los_cnt,laser_no_los_cnt,phy_other_cnt,laser_los_cnt,phy_lof_cnt,phyIntStatus);

	#ifndef PON_PHY_PERIODIC_POLLING
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	#endif

	spin_unlock(&gpPhyPriv->event_handle_lock); 

	return PHY_SUCCESS;
}


int an7583_xepon_phy_event_poll(char* buf)
{
	ulong flags;
	PON_PHY_Event_data_t phy_event = {.id = 0, .src = PON_PHY_EVENT_SOURCE_SW_POLL};
	int phyReadyStatus,phyLosStatus;
	static UINT32 poll_set_los_cnt=0,poll_set_ready_cnt=0,poll_no_ready_no_los_cnt=0,poll_los_cnt=0,poll_ready_cnt=0;
	
	uint read_data = 0,write_data = 0;
	static PHY_Los_Status_t  pre_phy_status=PHY_LINK_STATUS_UNKNOWN;
	uint polling_need_reset = 0;   // david 20240806
	
	if(FALSE == gpPhyPriv->is_phy_start){
		return	PHY_FAILURE;	//ang_20180208
	}
	//phy_print_time();
	gpPhyPriv->event_poll_timer_value = 1500;

	spin_lock_irqsave(&gpPhyPriv->event_handle_lock, flags);

	phyReadyStatus=phy_ready_status();
	phyLosStatus=phy_los_status();


	
#if XILINX_SERDES
	
		if(PHY_LINK_STATUS_READY == gpPhyPriv->phy_status) 
		{
			if(!(phyReadyStatus&&(!phyLosStatus)))
			{
				poll_set_los_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				//should not handle event here, since LOSS in not detected here, it is just double check on whether ISR result is error, nomarlly code will not enter this route
				gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
				phy_event.id = PHY_EVENT_TRANS_LOS_INT;
				phy_event_handler(&phy_event);
			}
		}
		else
		{	 
			if((!phyReadyStatus)&&(!phyLosStatus))
			{
				poll_no_ready_no_los_cnt++;
				
//julia_20220511				
				read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
				write_data = read_data | XILINX_SERDES_RST_ON;
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, write_data);
				mdelay(10);
				IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST, read_data);
				mdelay(10);

				IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
				IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => no_ready_no_los(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
				PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
				phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
				phy_event_handler(&phy_event);
			}
	
			if(phyReadyStatus&&(!phyLosStatus))
			{
				poll_set_ready_cnt++;
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => READY(%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_set_ready_cnt,phyReadyStatus,phyLosStatus);
				
				gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
				phy_event.id = PHY_EVENT_PHYRDY_INT;
				phy_event_handler(&phy_event);
			}
			if(phyLosStatus)
			{
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
				gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
				phy_event.id = PHY_EVENT_TRANS_LOS_INT;
				phy_event_handler(&phy_event);
				poll_los_cnt++;
			}
	
		}
	
#endif

#if ASIC_SERDES

	
		gpPhyPriv->xe_syncloss_cnt=0;	
	
		if((PHY_LINK_STATUS_LOS==gpPhyPriv->phy_status) && (phyLosStatus))
			{
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS (%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_los_cnt,phyReadyStatus,phyLosStatus);
				gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS;
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			poll_los_cnt++;
		}				
		else if((PHY_LINK_STATUS_READY==gpPhyPriv->phy_status) && (phyReadyStatus &&(!phyLosStatus)))
		{
			poll_ready_cnt++;
			poll_no_ready_no_los_cnt=0;
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => READY (%d) phyReadyStatus=%d,phyLosStatus=%d\r\n",poll_ready_cnt,phyReadyStatus,phyLosStatus);
	
			gpPhyPriv->phy_status = PHY_LINK_STATUS_READY;
			phy_event.id = PHY_EVENT_PHYRDY_INT;
			phy_event_handler(&phy_event); 
		 }
		else if((PHY_LINK_STATUS_UNKNOWN == gpPhyPriv->phy_status)&&(phyReadyStatus && (!phyLosStatus))) //ang_20200416
		{
			#if 0
			fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);
			// PCS_RX_Disable
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
			mdelay(10);			
			phy_pma_reset();
			mdelay(10);
			#endif
			poll_no_ready_no_los_cnt++;
			polling_need_reset = 1;		// david 20240808
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => SyncOK, recover from SyncLOS. \r\n");
		}
		else if((PHY_LINK_STATUS_UNKNOWN == gpPhyPriv->phy_status)&&((!phyReadyStatus) && phyLosStatus))//ang_20200416
		{
			PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
			gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
			phy_event.id = PHY_EVENT_TRANS_LOS_INT;
			phy_event_handler(&phy_event);
			poll_los_cnt++;
		}
		else	
		{
			if(!phyLosStatus)
			{
				poll_no_ready_no_los_cnt++;
			#if XILINX_SERDES
				IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
				IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => no_ready_no_los(%d) PHY_RST_N_SERDES,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			#else
				polling_need_reset = 1;
				gpPhyPriv->event_poll_timer_value = 3500;
				PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
	
				#ifdef LDDLA_SUPPORT_SET_TX_MODE
				phy_event.id = PHY_EVENT_NO_LOS_NO_READY;
				phy_event_handler(&phy_event);
				#endif

			#endif
			}
			else
			{
				PON_PHY_PRINT(PHY_MSG_INT," - POLLING => LOS(%d)No Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);  
				gpPhyPriv->phy_status = PHY_LINK_STATUS_LOS; 
				phy_event.id = PHY_EVENT_TRANS_LOS_INT;
				phy_event_handler(&phy_event);
				poll_los_cnt++;
			}
		}
	
#endif 

	spin_unlock_irqrestore(&gpPhyPriv->event_handle_lock, flags);

	if(1 == polling_need_reset){	//david 20240808
		if((poll_no_ready_no_los_cnt)%10 ==0)
		{
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)SCU Reset,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			PHY_INTERNAL_SET_API(PON_SET_PHY_SCU_RST,NULL);
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_INT," | POLLING => no_ready_no_los(%d)Reset pma,ReadyStatus=%d,LosStatus=%d\r\n",poll_no_ready_no_los_cnt,phyReadyStatus,phyLosStatus);
			pma_no_los_no_ready_reset();
		}
	}

	// enable sync_ok & sync_loss interrupt mask (if disabled in LOF interrupt case)  david 20240808
	phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);

#ifdef PON_PHY_PERIODIC_POLLING
	if(pre_phy_status!=gpPhyPriv->phy_status)
	{
		PON_PHY_PRINT(PHY_MSG_INT," - POLLING - (%d)(%d)(%d) phy_status = %d\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status);
	}
	pre_phy_status=gpPhyPriv->phy_status;
	//PHY_START_TIMER(gpPhyPriv->event_poll_timer) ;	
	PHY_START_TIMER(gpPhyPriv->event_poll_timer,gpPhyPriv->event_poll_timer_value); // julia_20201017 for kernel 5.4
#else
	PON_PHY_PRINT(PHY_MSG_INT," - POLLING - set(%d)(%d)(%d) query phy_status = %d (%d)(%d)\r\n",poll_set_los_cnt,poll_set_ready_cnt,poll_no_ready_no_los_cnt,gpPhyPriv->phy_status,poll_los_cnt,poll_ready_cnt);
#endif

	return PHY_SUCCESS;

}



int an7583_xepon_phy_dbg(char* buf)
{
	char dbg_id = *buf;
	char dbg_option1 = *(buf+1);
	char dbg_option2 = *(buf+2);
	UINT32 write_data = 0;
	UINT32 read_data = 0;
	UINT32 i=0,phy_reg_total_num=0;
	UINT32 frame_count_high=0,frame_count_low=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	printk("[%s:%d] id = %d\n",__FUNCTION__,__LINE__,dbg_id);
	//printk("gpPhyPriv->phyCfg.flags.mode = %d\n",gpPhyPriv->phyCfg.flags.mode);
	
#ifdef	EN7581_HEC_TEST

	phy_reg_total_num=172;
	phy_reg_all=en7581_xepon_phy_reg_all;

	switch(dbg_id)
	{
		case 0:
			printk("\r\n\r\n___________Read XEPON debug counter only(no reset)___________\n");
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=40;i<=46;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=40;i<=46;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(EN7581_XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_H",EN7581_XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_L",EN7581_XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					

					break;

				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

					return PHY_FAILURE;
			}
			//read_data = IO_GPHYREG(0xbfb66120); //  msb 8 bit  = MAC error
			//printk("#MAC: Error CNT 0x%8x=0x%8x MSB 8 bit=0x%2x\r\n",0xbfb66120,read_data,read_data>>24);
			//read_data = IO_GPHYREG(0xbfb66360); //  MAC error
			//printk("#MAC: Error CNT 0x%8x=0x%8x\r\n",0xbfb66360,read_data);

			break;
			
		case 2:
			/*	XEPON 10G mode	*/
			IO_SPHYREG(EN7581_XEPON_PCS_CLR_ALL_NUM, EN7581_XEPON_PCS_CLR_ALL_NUM_CLR);//latch 
			
		
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=143;i<=148;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
				
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					//for(i=143;i<=148;i++)
					for(i=40;i<=46;i++)//cheng_20220314 wrong register index
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
		
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(EN7581_XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_H",EN7581_XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_L",EN7581_XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					
		
					break;
		
				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);
		
					return PHY_FAILURE;
			}
			
			break;

		case 3:
			printk("XEPON Tx gear box test : Threshold = %d\r\n",dbg_option1);
			/* Disable ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			/* Set Tx gear box threshold */
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Read EN7581_XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);
			printk("     TX_GB_THRESHOLD      =0x%8x\n",dbg_option1);
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,((read_data&EN7581_XEPON_PCS_TX_CTRL_CFG_TX_GB_THR_MASK)|dbg_option1)) ;
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Set  EN7581_XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);
				
			/* Reset PCS logic to clear Tx gear box */
			IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
			IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);

			/* Clear ISR status*/
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
			IO_SPHYREG(EN7581_XEPON_PCS_INT_STATUS, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_INT_STATUS=0x%08x\n",read_data);
			
			/* Enable ISR */
			phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);

			break;
			
		case 4:
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Read EN7581_XEPON_PCS_TX_CTRL_CFG=0x%8x\n",read_data);

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
			IO_SPHYREG(EN7581_XEPON_PCS_INT_STATUS, read_data);
			printk("Before Clear EN7581_XEPON_PCS_INT_STATUS=0x%08x\n",read_data);

			if(read_data&EN7581_XEPON_PCS_INT_TX_DET_OVERRUN)
			{
				printk("21:TX_DET_OVERRUN\n");
			}
			if(read_data&EN7581_XEPON_PCS_INT_TX_DET_UNDERRUN)
			{
				printk("20: TX_DET_UNDERRUN\n");
			}
			if(read_data&EN7581_XEPON_PCS_INT_TX_GB_OVERRUN)
			{
				printk("19:  TX_GB_OVERRUN\n");
			}
			if(read_data&EN7581_XEPON_PCS_INT_TX_GB_UNDERRUN)
			{
				printk("18:   TX_GB_UNDERRUN\n");
			}
			if(read_data&EN7581_XEPON_PCS_INT_RX_GB_OVERRUN)
			{
				printk("17:    RX_GB_OVERRUN\n");
			}
			if(read_data&EN7581_XEPON_PCS_INT_RX_GB_EMPTY_APPEAR)
			{
				printk("16:     RX_GB_EMPTY_APPEAR\n");
			}

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_INT_STATUS);
			printk("After  Clear EN7581_XEPON_PCS_INT_STATUS=0x%08x\n",read_data);
			
			break;

		case 5:

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_SFP_STATUS);
			printk("Read EN7581_XEPON_PCS_SFP_STATUS=0x%8x\n",read_data);
			printk("     _TX_FAULT           =0x%8x\n",read_data&EN7581_XEPON_PCS_SFP_STATUS_TX_FAULT);
			printk("	 _RX_LOSS            =0x%8x\n",read_data&EN7581_XEPON_PCS_SFP_STATUS_RX_LOSS);
			printk("	 _TX_SD              =0x%8x\n",read_data&EN7581_XEPON_PCS_SFP_STATUS_TX_SD);
			printk("	 _MOD_ABS            =0x%8x\n",read_data&EN7581_XEPON_PCS_SFP_STATUS_MOD_ABS);
			printk("	 _P_DOWN             =0x%8x\n",read_data&EN7581_XEPON_PCS_SFP_STATUS_P_DOWN);

			break;	
			
		case 10:
			
			printk("\r\n\r\n___________XEPON BER Monitor & Counter___________\n");

			read_data = EN7581_XEPON_PCS_RX_BER_START_MONITOR;
			IO_SPHYREG(EN7581_XEPON_PCS_BER_START_MONITOR,read_data);
			mdelay(10);

			(phy_reg_all+15)->cur=IO_GPHYREG((phy_reg_all+15)->addr);
			printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+15)->name,(phy_reg_all+15)->addr,(phy_reg_all+15)->cur,(phy_reg_all+15)->cur);
			
			for(i=17;i<=19;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}
			break;
			
		case 41:
			switch(gpPhyPriv->wan_sel)
			{
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_1G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					for(i=143;i<=148;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
				
					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON_10G_10G HEC test 1 CNT_____");
					for(i=21;i<=22;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}			
					//for(i=143;i<=148;i++)
					for(i=40;i<=46;i++)//cheng_20220314  wrong register index
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}
					for(i=23;i<=26;i++)
					{
						(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
						printk("%-44s =0x%8x=0x%8x(%u)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
					}

					break;
					
				case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
					/*	XEPON 1G mode  */
					printk("\r\n%-44s	 address=value\r\n","_____ XEPON HEC test 1 CNT 1G mode_____");
					//Rx framer cnt
					IO_SPHYREG(EN7581_XEPON_1G_XP_ERRCNT_CTL, PHY_RXFRAME_LATCH);//latch 
					frame_count_high = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_H);
					frame_count_low = IO_GPHYREG(EN7581_XEPON_1G_FRAME_CNT_L);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_H",EN7581_XEPON_1G_FRAME_CNT_H,frame_count_high,frame_count_high);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_FRAME_CNT_L",EN7581_XEPON_1G_FRAME_CNT_L,frame_count_low,frame_count_low);
					
					//Tx framer cnt
					read_data = IO_GPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG);
					write_data = read_data | PHY_TX_CNT_LCH;//latch
					IO_SPHYREG(EN7581_XEPON_1G_PHYTX_TEST_TRIG,write_data);
					read_data = IO_GPHYREG(EN7581_XEPON_1G_TX_FRAME_COUNTER);
					printk("%-44s =0x%8x=0x%8x(%u)\r\n","EN7581_XEPON_1G_TX_FRAME_COUNTER",EN7581_XEPON_1G_TX_FRAME_COUNTER,read_data,read_data);
					

					break;

				default:
					printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

					return PHY_FAILURE;
			}
			//read_data = IO_GPHYREG(0xbfb66120); //  msb 8 bit  = MAC error
			//printk("#MAC: Error CNT 0x%8x=0x%8x MSB 8 bit=0x%2x\r\n",0xbfb66120,read_data,read_data>>24);
			//read_data = IO_GPHYREG(0xbfb66360); //  MAC error
			//printk("#MAC: Error CNT 0x%8x=0x%8x\r\n",0xbfb66360,read_data);

			break;
		


		case 51:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase ___\r\n");

			/*	XEPON PCS loop back write	*/
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);

			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);

			
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
			



		case 52:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
				}				
			}
			else
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

			
		case 53:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
			}

			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);
			
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
	
		case 54:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=31;i<=84;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				}
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
					if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
					{
						printk("=Fail");
					}
					else
					{
						printk("=Pass");
					}
				}				
			}
			else
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;	

		case 55:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_LOOP_BACK|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 56:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				for(i=31;i<=84;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				}
				for(i=85;i<=136;i++)
				{
					(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
					printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
					if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
					{
						printk("=Fail");
					}
					else
					{
						printk("=Pass");
					}
				}				
			}
			else
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 57:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);

			}
			
			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 58:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE\n");
			}
			else
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}

			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 59:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 60:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			
			if(EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE==(read_data&EN7581_XEPON_PCS_RX_TEST_DONE_RX_TEST_DONE))
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE\n");
			}
			else
			{
				printk("EN7581_XEPON_PCS_RX_TEST_DONE detect RX_TEST_DONE Fail !\n");
			}
			
			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}

			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;


		case 61:
			printk("\r\n___ PCS TX (NOT Internal LPBK) RX LPBK mode ___ Increase ___\r\n");
		
			/*	XEPON PCS loop back write	*/
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);
		
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
		
			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);
		
			
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
		
			break;


		case 62:
			printk("\r\n___ PCS_LOOP_BACK ___ Increase check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("%-44s =0x%8x=0x%8x(%d)\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur,(phy_reg_all+i)->cur);
			}				

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

			
		case 63:
			printk("\r\n___ PCS TX (NO Internal LPBK) RX LPBK mode ___ ALL ZEROS ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);
			
			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ZEROS);
			}

			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);
			
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	
		
		case 64:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ZEROS check ___\r\n");
			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}				

			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;

		case 65:
			printk("\r\n___ PCS TX (NOT Internal LPBK) RX LPBK mode ___ ALL ONES ___\r\n");
			/* Disable Rx and ISR */
			phy_int_config(PHY_INT_EN_MODE_MANUAL,PHY_INT_EN_DISABLE);

			write_data=0x1c48;
			IO_SPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_TX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_TX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_CTRL_CFG,read_data);

			/*	XEPON PCS loop back write	*/
			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,read_data);
			for(i=32;i<=83;i++)
			{
				IO_SPHYREG((phy_reg_all+i)->addr,PHY_REG_ALL_ONES);

			}
			
			write_data=0x10302;
			IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG,write_data);
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG);
			printk("Write EN7581_XEPON_PCS_RX_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_CTRL_CFG,read_data);

			IO_SPHYREG(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));
			printk("Write EN7581_XEPON_PCS_TX_TEST_CTRL_CFG(0x%x)=0x%08x\n",EN7581_XEPON_PCS_TX_TEST_CTRL_CFG,(EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_EN|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TEST_MODE_USER_CFG|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_OVERHEAD(2)|EN7581_XEPON_PCS_TX_TEST_CTRL_CFG_TX_TEST_TRIG));

			break;	

		
		case 66:
			printk("\r\n___ PCS_LOOP_BACK ___ ALL ONES check ___\r\n");

			/*	XEPON PCS loop back read and check	*/

			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			IO_SPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE, read_data);
			printk("Read & Clear EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);

			for(i=31;i<=84;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
			}
			for(i=85;i<=136;i++)
			{
				(phy_reg_all+i)->cur=IO_GPHYREG((phy_reg_all+i)->addr);
				printk("\r\n%-44s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
				if(PHY_REG_ALL_ONES!=(phy_reg_all+i)->cur)
				{
					printk("=Fail");
				}
				else
				{
					printk("=Pass");
				}
			}				
			
			read_data=IO_GPHYREG(EN7581_XEPON_PCS_RX_TEST_DONE);
			printk("\r\nAfter Clear  EN7581_XEPON_PCS_RX_TEST_DONE(0x%x)=0x%08x\n",EN7581_XEPON_PCS_RX_TEST_DONE,read_data);
			
			//phy_int_config(PHY_INT_EN_MODE_DEFAULT,PHY_INT_EN_MODE_DEFAULT);
			break;






		default:
			printk("[%s:%d] UNKNOWN ID\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
#endif
	return PHY_SUCCESS;

}


int an7583_xepon_phy_pma_reset(char* buf)
{


	IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_DISABLE);
	IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
	phy_delay1ms(1);

	if(gpPhyPriv->first_plugin_flag == TRUE)
	{
		fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
		gpPhyPriv->first_plugin_flag = FALSE;
		gpPhyPriv->pma_init_done= TRUE;
	}
	else
	{
		fiber_plug_reset(PLUG_IN,gpPhyPriv->wan_sel);
	}
	PON_PHY_PRINT(PHY_MSG_DBG,"XEPON PMA Reset! \n");
	phy_delay1ms(1);

	IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);
	phy_delay1ms(1);

	IO_SPHYREG(EN7581_XEPON_PCS_RX_CTRL_CFG, EN7581_XEPON_PCS_RX_ENABLE);
	PON_PHY_PRINT(PHY_MSG_DBG,"<RX_ENABLE ON>!\n");
	
	return PHY_SUCCESS;
}


#define _I_AM_PMA_SPLIT_LINE_
#if ASIC_SERDES	// for ASIC
int xpon_pma_init(void)
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"%s\r\n",__FUNCTION__);
	//param init based on wan_sel
	//xpon_pma_param_init();
	//xpon_pma_param_disp();
	xpon_pma_mode_init();
	set_pma_fir();	
	get_pma_fir();

	return PHY_SUCCESS;
}

int xpon_pma_mode_init(void)
{
	UINT32 read_data ;

    //step 1: select scu mode
    //scu_mode_sel(gpPhyPriv->wan_sel);

    //step 2: mode init setting
    xpon_init(gpPhyPriv->wan_sel); 
    //step 3: reset
    if(pon_phy_get_los_status()==1)
    {
	    PON_PHY_PRINT(PHY_MSG_ERR,"<<<<<<<<<<< No Laser! >>>>>>>>>>>>>\n");
	    //fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel); //when no laser, power saving will cause rogue ONU., ang_20190107
	    // make pma initial done, dyma_20210618
	    fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);    //dyma_20210618
	    mdelay(1);	
	    fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);         //dyma_20210618
	    mdelay(1);	
	
	    gpPhyPriv->pma_init_done= FALSE;
	    return 0;
    }
    else
    {   
	    fiber_plug_reset(FIRST_PLUG_IN,gpPhyPriv->wan_sel);
	    gpPhyPriv->first_plugin_flag = FALSE;
	    gpPhyPriv->pma_init_done= TRUE;

	    //*************************************************************************//
	    phy_print_time();
	    PON_PHY_PRINT(PHY_MSG_ERR,"======= 350ms delay =======\n");
	    mdelay(350);	//add by david 20190508  for Hangyan
	    //*************************************************************************//
    }	
  

	return 0;
}


void pma_no_los_no_ready_reset(void)
{
	//PLUG_OUT			
	fiber_plug_reset(PLUG_OUT,gpPhyPriv->wan_sel);			

	//PLUG_IN
	//phy_pma_reset();
	phy_pma_reset_with_lock();
}


#endif

#define _I_AM_PMA_SUPPLEMENT_FUNC_SPLIT_LINE_   



