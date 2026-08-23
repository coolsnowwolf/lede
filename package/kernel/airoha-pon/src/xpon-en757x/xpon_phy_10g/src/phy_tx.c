/*
* File Name: phy_tx.c
* Description: .
*
******************************************************************/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>

#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_debug.h"
#include "phy_tx.h"
#include "phy_init.h"
#include "phy_reg.h"

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


extern PHY_GlbPriv_T *gpPhyPriv;

extern int	mt7570_select;





/*****************************************************************************
//function :
//		phy_trans_tx_getting
//description : 
//		this function is to get the transceiver pin define of our system
//input :	
//		PPHY_TransConfig_T structure : PHY_ENABLE means low active ; PHY_DISABLE means high active
//output :
//		N/A
******************************************************************************/
void phy_trans_tx_getting(PPHY_TransConfig_T trans_status)
{
	return PHY_INTERNAL_GET_API(PON_GET_PHY_TRANS_TX,trans_status);
}



/*****************************************************************************
//function :
//		phy_tx_alarm
//description : 
//		this function is used to get the alarm information of transceiver 
//input :	
//		N/A
//output :
//		return_value : defined in phy_def.h (ENUM_PhyTransAlarm)
******************************************************************************/
int phy_tx_alarm(void)
{
	uint read_data = 0;
	uint return_value = PHY_TRANS_NO_ALARM;
	//unchar *ptr = NULL;
	unchar ptr[1];
	ushort u2ByteCnt;
	
	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		if(gpPhyPriv->trans_mod_id == PHY_TRANS_IDENTIFIER_XFP) //julia_20220511
		{		
			//PON_PHY_PRINT(PHY_MSG_ERR,"NO XFP tx alarm\r\n ");
		}
		else			
		{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num,PHY_TRANS_ALARM_FLAGS, ptr, 1);
		read_data = ptr[0];
		//if(ptr)
				//kfree(ptr);
		if( (read_data & (PHY_TRANS_ALARM_FLAGS_TX_POWER_LOW_ALARM)) == PHY_TRANS_ALARM_FLAGS_TX_POWER_LOW_ALARM )
		{
			PON_PHY_PRINT(PHY_MSG_DBG, "Tx power is below low alarm. \n");
			return_value = return_value | PHY_TRANS_TX_LOW_POWER_ALARM;
		}

		if( (read_data & (PHY_TRANS_ALARM_FLAGS_TX_POWER_HIGH_ALARM)) == PHY_TRANS_ALARM_FLAGS_TX_POWER_HIGH_ALARM )
		{
			PON_PHY_PRINT(PHY_MSG_DBG, "Tx power exceeds high alarm. \n");
			return_value = return_value | PHY_TRANS_TX_HIGH_POWER_ALARM;
		}

		if( (read_data & (PHY_TRANS_ALARM_FLAGS_TX_BIAS_LOW_ALARM)) == PHY_TRANS_ALARM_FLAGS_TX_BIAS_LOW_ALARM )
		{
			PON_PHY_PRINT(PHY_MSG_DBG, "Bias Current is below low alarm. \n");
			return_value = return_value | PHY_TRANS_TX_LOW_CUR_ALARM;
		}

		if( (read_data & (PHY_TRANS_ALARM_FLAGS_TX_BIAS_HIGH_ALARM)) == PHY_TRANS_ALARM_FLAGS_TX_BIAS_HIGH_ALARM )
		{
			PON_PHY_PRINT(PHY_MSG_DBG, "Bias Current exceeds high alarm. \n");
			return_value = return_value | PHY_TRANS_TX_HIGH_CUR_ALARM;
		}
	}
	}
	return return_value;
}



void phy_tx_ctl(unchar val)
{

#if ASIC_SERDES
	if(PHY_ENABLE == val){ /* enable tx power should turn off tx disable */
#ifdef TCSUPPORT_KERNEL_API
		ledTurnOffRT(LED_PHY_TX_POWER_DISABLE);
#else
		ledTurnOff(LED_PHY_TX_POWER_DISABLE);
#endif
	}
	else if (PHY_DISABLE == val){ /* disbale tx power should turn on tx disable */
#ifdef TCSUPPORT_KERNEL_API
		ledTurnOnRT(LED_PHY_TX_POWER_DISABLE);
#else
		ledTurnOn(LED_PHY_TX_POWER_DISABLE);
#endif
	}else
		PON_PHY_PRINT(PHY_MSG_ERR, "PHY Tx ctl type error. \n");
#endif
	
#if A60972_SERDES
	a60972_trans_tx_control(val);
#endif

#if XILINX_SERDES //only for vu19p xg/xgspon xilinx_serdes, ang_20211206

	uint read_data=0,write_data=0;

	/* for old xg/xgs bitfile 
	if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_XGSPON==gpPhyPriv->wan_sel)\
		||(SCU_WAN_CONF_REG_WAN_SEL_XGPON==gpPhyPriv->wan_sel))
	{

		read_data = IO_GPHYREG(EN7581_XGPON_PHY_XG_PON_TX_CTRL);
		
			if(PHY_ENABLE == val) // enable tx power 
		{
			write_data = read_data & (~EN7581_XGPON_PHY_XILINX_SFP_TX_PDWD);
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_TX_CTRL,write_data);
		}
			else if (PHY_DISABLE == val) // disbale tx power
		{
			write_data = read_data | EN7581_XGPON_PHY_XILINX_SFP_TX_PDWD;
			IO_SPHYREG(EN7581_XGPON_PHY_XG_PON_TX_CTRL,write_data);
		}
		else
			PON_PHY_PRINT(PHY_MSG_ERR, "PHY Tx ctl type error. \n");
	}
	*/
	//julia_20220511	
	if((SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G==gpPhyPriv->wan_sel)
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G==gpPhyPriv->wan_sel)
		||(SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G==gpPhyPriv->wan_sel))
	{
		// this is for high active
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
		
		if(PHY_ENABLE == val) // enable tx power 
		{
			write_data = read_data | EN7581_PHY_XILINX_SFP_TX_PDWD;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST,write_data);
		}
		else if (PHY_DISABLE == val) // disbale tx power
		{
			write_data = read_data & (~EN7581_PHY_XILINX_SFP_TX_PDWD);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST,write_data);
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR, "PHY Tx ctl type error. \n");
		}
	
	}
	else // this is for low active
	{
		read_data = IO_GPHYREG(EN7581_FPGA_XILINX_SERDES_RST);
	
		if(PHY_ENABLE == val) // enable tx power 
		{
			write_data = read_data & (~EN7581_PHY_XILINX_SFP_TX_PDWD);
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST,write_data);
		}
		else if (PHY_DISABLE == val) // disbale tx power
		{
			write_data = read_data | EN7581_PHY_XILINX_SFP_TX_PDWD;
			IO_SPHYREG(EN7581_FPGA_XILINX_SERDES_RST,write_data);
		}
		else
		{
			PON_PHY_PRINT(PHY_MSG_ERR, "PHY Tx ctl type error. \n");
		}
	}

#endif

}




/*****************************************************************************
//function :
//		phy_trans_power_switch
//description : 
//		this function is used to switch transceiver power
//input :	
//		trans_switch : PHY_ENABLE  means enable transceiver power ; PHY_DISABLE  means disable transceiver power
//output :
//		setting success or failure
******************************************************************************/
int phy_trans_power_switch(unchar trans_switch)
{

	uint read_data = 0;
	uint write_data = 0 ;
	
	phy_print_time();

	if(trans_switch == PHY_TX_DIS_RESTORE_BY_SW)   //tx_dis_reset_protection
	{
		trans_switch = gpPhyPriv->trans_tx_status;
		PON_PHY_PRINT(PHY_MSG_INT, "%s TX_DIS restore to %d\n",__FUNCTION__,trans_switch);
	}

	if(trans_switch == PHY_DISABLE)
	{
		#if ASIC_SERDES
		// disable rogue int when disable tx
		if(gpPhyPriv->is_rogue_onu)
		{
			pon_phy_rogueonu_int_en(PHY_DISABLE);
			pon_phy_clear_rogueonu();
			phy_tx_power_config(0);
			
		}
		#endif

		#ifndef TCSUPPORT_CPU_AN7583//julia_20220923
		#if TCSUPPORT_CPU_EN7581 //julia_20220923
		//ledTurnOff(LED_PON_TX_BEN);
		SET_FORCE_GPIO32_EN(EN7581_XPON_SCU_GPIO_MASK,0);
		#endif
		#endif
		
		phy_tx_ctl(PHY_DISABLE);
		gpPhyPriv->trans_tx_status=PHY_DISABLE;     //tx_dis_reset_protection

		PON_PHY_PRINT(PHY_MSG_INT, "%s off___\n",__FUNCTION__);
		return PHY_SUCCESS;
	}
	else if(trans_switch == PHY_ENABLE)
	{
		if (gpPhyPriv->phyCfg.flags.txPowerEnFlag) 
		{
			phy_tx_ctl(PHY_ENABLE);
			gpPhyPriv->trans_tx_status=PHY_ENABLE;  //tx_dis_reset_protection 

			#if ASIC_SERDES
			// enable rogue int when enable tx
			if(gpPhyPriv->rogue_onu_det_en)
			{
				pon_phy_clear_rogueonu();
				pon_phy_rogueonu_int_en(PHY_ENABLE);
			}
			#endif

			#ifdef TCSUPPORT_MT7570
			if(gpPhyPriv->en7571_init_done == TRUE)
			{
				LDDLA_SET_TRANS_RESET();
			}
			#endif
			PON_PHY_PRINT(PHY_MSG_INT, "%s ___ON\n",__FUNCTION__);
		}

		return PHY_SUCCESS;
	}
	else if(trans_switch == PHY_TX_DIS_ON_HW_ONLY)  //tx_dis_reset_protection
	{

		#if ASIC_SERDES
		// disable rogue int when disable tx
		if(gpPhyPriv->is_rogue_onu)
		{
			pon_phy_rogueonu_int_en(PHY_DISABLE);
			pon_phy_clear_rogueonu();
		}
		#endif

		phy_tx_ctl(PHY_DISABLE);
		PON_PHY_PRINT(PHY_MSG_INT, "%s off___ hw_only\n",__FUNCTION__);
		return PHY_SUCCESS;
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_INT, "%s fail !\n",__FUNCTION__);
		return PHY_FAILURE;
	}

}

void phy_tx_power_config(unchar enable){
	gpPhyPriv->phyCfg.flags.txPowerEnFlag = enable;
	PON_PHY_PRINT(PHY_MSG_INT, "PON PHY set txPowerEnFlag to %d.\n", enable);
}


