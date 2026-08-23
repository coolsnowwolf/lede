/*
* File Name: phy_tx.c
* Description: .
*
******************************************************************/

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/time.h>
#include "phy_global.h"
#include "i2c.h"
#include "phy.h"
#include "phy_def.h"
#include "phy_debug.h"
#include "phy_reg.h"

/*****************************************************************************
//function :
//		phy_rx_power_alarm
//description : 
//		this function is used to get the alarm information of transceiver 
//input :	
//		N/A
//output :
//		return_value : defined in phy_def.h (ENUM_PhyTransAlarm)
******************************************************************************/
int phy_rx_power_alarm(void)
{
	uint alarm_data = 0;
	uint return_value = PHY_TRANS_NO_ALARM;
	//unchar *ptr = NULL;
	unchar ptr[1];
	ushort u2ByteCnt;
	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		if(gpPhyPriv->trans_mod_id == PHY_TRANS_IDENTIFIER_XFP) //julia_20220511
		{		
			//PON_PHY_PRINT(PHY_MSG_ERR,"NO XFP rx alarm\r\n ");
		}
		else			
		{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num,PHY_TRANS_ALARM_FLAGS_2, ptr, 1);
		alarm_data= ptr[0];
		//if(ptr)
			//kfree(ptr);

		if( (alarm_data & (PHY_TRANS_ALARM_FLAGS_2_RX_POWER_LOW_ALARM)) == PHY_TRANS_ALARM_FLAGS_2_RX_POWER_LOW_ALARM )
		{
			//change_alarm_led_status(ALARM_LED_FLICKER);
			PON_PHY_PRINT(PHY_MSG_DBG, "Rx power is below low alarm. \n");
			return_value = return_value | PHY_TRANS_RX_LOW_POWER_ALARM;
		}
		
		if( (alarm_data & (PHY_TRANS_ALARM_FLAGS_2_RX_POWER_HIGH_ALARM)) == PHY_TRANS_ALARM_FLAGS_2_RX_POWER_HIGH_ALARM )
		{
			PON_PHY_PRINT(PHY_MSG_DBG, "Rx power exceeds high alarm. \n");
			return_value = return_value |  PHY_TRANS_RX_HIGH_POWER_ALARM;
		}
	}
	}
	return return_value;
}

