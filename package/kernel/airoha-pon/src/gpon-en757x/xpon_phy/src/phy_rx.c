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


/*****************************************************************************
//function :
//		phy_rx_fec_counter
//description : 
//		this is used to get the FEC counter, and must set function "phy_cnt_enable" first
//input :	
//		PPHY_FecCount_T structure ; defined in phy_def.h
//output :
//		N/A
******************************************************************************/
void phy_rx_fec_counter(PPHY_FecCount_T fec_counter)
{
	IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, (PHY_ERR_LATCH));//latch
	fec_counter->correct_bytes = IO_GPHYREG(PHY_CSR_ERR_BYTE_CNT);
	fec_counter->correct_codewords = IO_GPHYREG(PHY_CSR_ERR_CODE_CNT);
	fec_counter->uncorrect_codewords = IO_GPHYREG(PHY_CSR_NOSOL_CODE_CNT);
	fec_counter->total_rx_codewords = IO_GPHYREG(PHY_CSR_RX_CODE_CNT);
	fec_counter->fec_seconds = IO_GPHYREG(PHY_CSR_FEC_SECONDS);

	PON_PHY_MSG(PHY_MSG_DBG, "correct_bytes: %.8x\n", fec_counter->correct_bytes);
	PON_PHY_MSG(PHY_MSG_DBG, "correct_codewords: %.8x\n", fec_counter->correct_codewords);
	PON_PHY_MSG(PHY_MSG_DBG, "uncorrect_codewords: %.8x\n", fec_counter->uncorrect_codewords);
	PON_PHY_MSG(PHY_MSG_DBG, "total_rx_codewords: %.8x\n", fec_counter->total_rx_codewords);
	PON_PHY_MSG(PHY_MSG_DBG, "fec_seconds: %.8x\n", fec_counter->fec_seconds);

}



/*****************************************************************************
//function :
//		phy_bip_counter
//description : 
//		this is used to get the BIP counter, and must set function "phy_cnt_enable" first
//input :	
//		N/A
//output :
//		BIP counter
******************************************************************************/
int phy_bip_counter(void)
{
	  int bip_err_cnt = 0;
	  IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, (PHY_BIP_LATCH)); /* latch */
	  bip_err_cnt = IO_GPHYREG(PHY_CSR_BIP_CNT);
	  PON_PHY_MSG(PHY_MSG_DBG, "Phy BIP Counter : %.8x\n", bip_err_cnt);

	  /* clear bip error counter */
	  IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, PHY_BIP_CLR);
	
	  return bip_err_cnt;
}




/*****************************************************************************
//function :
//		phy_rx_frame_counter
//description : 
//		this is used to get the Rx frame counter, and must set function "phy_cnt_enable" first
//input :	
//		PPHY_FrameCount_T structure ; defined in phy_def.h
//output :
//		N/A
******************************************************************************/
void phy_rx_frame_counter(PPHY_FrameCount_T frame_counter)
{
	IO_SPHYREG(PHY_CSR_XP_ERRCNT_CTL, (PHY_RXFRAME_LATCH));//latch 
	frame_counter->frame_count_high = IO_GPHYREG(PHY_CSR_FRAME_CNT_H);
	frame_counter->frame_count_low = IO_GPHYREG(PHY_CSR_FRAME_CNT_L);
	if(phy_gpon_mode())//LOF is only used in GPON mode
		frame_counter->lof_counter = IO_GPHYREG(PHY_CSR_LOF_CNT);

	PON_PHY_MSG(PHY_MSG_DBG, "frame_count_high: %.8x\n", frame_counter->frame_count_high);
	PON_PHY_MSG(PHY_MSG_DBG, "frame_count_low: %.8x\n", frame_counter->frame_count_low);
	if(phy_gpon_mode())
		PON_PHY_MSG(PHY_MSG_DBG, "lof_counter: %.8x\n", frame_counter->lof_counter);

}


/*****************************************************************************
//function :
//		phy_rx_fec_setting
//description : 
//		this is used to enable/disable Rx FEC function
//input :	
//		fec_control ; the valid value is "PHY_ENABLE" and "PHY_DISABLE"
//output :
//		setting success or failure
******************************************************************************/
int phy_rx_fec_setting(unchar fec_control)
{
	if(fec_control == PHY_ENABLE)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Rx FEC Enable\n");
		IO_SPHYREG(PHY_CSR_FECDEC_CTL,0x1);
		return PHY_SUCCESS;
	}
	else if(fec_control == PHY_DISABLE)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Rx FEC Disable\n");
		IO_SPHYREG(PHY_CSR_FECDEC_CTL,0x0);
		return PHY_SUCCESS;
	}
	else
		return PHY_FAILURE;
}



/*****************************************************************************
//function :
//		phy_rx_fec_getting
//description : 
//		this is used to get the function status of Rx FEC 
//input :	
//		N/A
//output :
//		0 : means RX FEC function is now disable
//		1 : means RX FEC function is now enable
******************************************************************************/
int phy_rx_fec_getting(void)
{
	return IO_GPHYREG(PHY_CSR_FECDEC_CTL);
}



/*****************************************************************************
//function :
//		phy_rx_fec_status
//description : 
//		this is used to get the Rx FEC status
//input :	
//		N/A
//output :
//		PHY_TRUE : means Rx FEC now is enabled
//		PHY_FALSE : means RX FEC now is disabled
******************************************************************************/
int phy_rx_fec_status(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_RX_STATUS);
	if((read_data & PHY_RX_FEC) == PHY_RX_FEC)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "RX FEC on.\n");
		return PHY_TRUE;
	}
	else
	{
		PON_PHY_MSG(PHY_MSG_DBG, "RX FEC off.\n");
		return PHY_FALSE;
	}
	
}

/*****************************************************************************
//function :
//		phy_trans_rx_setting
//description : 
//		this function is used for transceiver to define pin function 
//input :	
//		rx_sd_inv : PHY_RX_SD_INV means that transceiver use RX_LOS pin definition ; PHY_RX_SD : mean that transceiver use RX_SD pin definition 		
//output :
//		setting success or failure
******************************************************************************/
int phy_trans_rx_setting(unchar rx_sd_inv)
{
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);//rx_sd
	
	if((rx_sd_inv == PHY_ENABLE))//rx_sd
		write_data = (read_data & ~(PHY_RX_SD_INV)) | (PHY_RX_SD_INV);
	else if((rx_sd_inv == PHY_DISABLE))
		write_data = (read_data & ~(PHY_RX_SD_INV)) | (PHY_RX_SD);
	else 
	{
		PON_PHY_MSG(PHY_MSG_DBG, "error input : rx_sd_inv.\n");
		return PHY_FAILURE;
	}
	PON_PHY_MSG(PHY_MSG_DBG, "rx_sd_inv success \n");
	IO_SPHYREG(PHY_CSR_XPON_SETTING,write_data);
	return PHY_SUCCESS;
}



/*****************************************************************************
//function :
//		phy_trans_rx_getting
//description : 
//		this function is to get the transceiver pin define of our system 
//input :	
//		N/A		
//output :
//		PHY_ENABLE means that transceiver use RX_LOS pin definition 
//		PHY_DISABLE means that transceiver use RX_SD pin definition
******************************************************************************/
int phy_trans_rx_getting(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);
	if((read_data & PHY_RX_SD_INV) == 0)
		return PHY_DISABLE;
	else
		return PHY_ENABLE;
}

/*****************************************************************************
//function :
//		phy_ber_alarm
//description : 
//		this function is used to get BER order 
//input :	
//		N/A		
//output :
//		ber_order
******************************************************************************/
int phy_ber_alarm(void)
{
	unsigned long long int frame_count = 0;
	unsigned long long int bip = 0;
	int ber_order = 0;
	unsigned long long int ber = 0;
	static int ber_counter = 0;
	PHY_FrameCount_T frame_cnt;
	PPHY_FrameCount_T p_frame_cnt = & frame_cnt;
	if((ber_counter % 10000) == 0 )
	{
		phy_counter_clear(PHY_BIP_CNT_CLR | PHY_RXFRAME_CNT_CLR);
		ber_counter = 0;
	}
	ber_counter = ber_counter + 1;
	phy_rx_frame_counter(p_frame_cnt);
	frame_count = p_frame_cnt->frame_count_low;
	bip = phy_bip_counter();
	//bip = 1000;
	if(bip < 100)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "unknown BER \n");
		return PHY_BER_UNKNOWN;
	}
	else
	{
	/*BER counting*/
		for(ber_order = 1 ; ber_order<11 ; ber_order++)
		{
			bip = bip*10;//count BER order
			ber = bip/(frame_count*311040);
			if((10 > ber) && (ber >= 1))
				break;
		}

	}
	PON_PHY_MSG(PHY_MSG_DBG, "ber_counter = %d \n", ber_counter);
	PON_PHY_MSG(PHY_MSG_DBG, "BER_order = %d \n", ber_order);
	return ber_order;
}

/*
	Self test mode
*/
/*****************************************************************************
//function :
//		phy_gpon_psync_setting
//description : 
//		this function is used to GPON sync criterion
//input :	
//		PPHY_GponRxCtl_T ; defined in phy_def.h
//output :
//		setting success or failure
******************************************************************************/
int phy_gpon_psync_setting(PPHY_GponRxCtl_T rx_control)
{
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_GPON_PSYNC_CTL);
	
	if((rx_control->mask & PHY_GPON_M1_LMT_EN) == PHY_GPON_M1_LMT_EN )
	{
		if((rx_control->gpon_m1_lmt & PHY_GPON_M1_LEN_MASK) != 0x00 )
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : gpon_m1_lmt. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_GPON_M1_MASK) | rx_control->gpon_m1_lmt;
	}
	if((rx_control->mask & PHY_GPON_M2_LMT_EN) == PHY_GPON_M2_LMT_EN)
	{
		if((rx_control->gpon_m2_lmt & PHY_GPON_M2_LEN_MASK) != 0x00 )
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : gpon_m2_lmt. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_GPON_M2_MASK) | (rx_control->gpon_m2_lmt << PHY_GPON_M2_OFFSET);
	}
	if((rx_control->mask & PHY_PRESYNC_MIS_LMT_EN) == PHY_PRESYNC_MIS_LMT_EN)
	{
		if((rx_control->presync_mismatch_lmt & PHY_PRESYNC_MIS_LEN_MASK) != 0x00 )
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : presync_mismatch_lmt. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_PRESYNC_MIS_MASK) | (rx_control->presync_mismatch_lmt<< PHY_PRESYNC_MIS_OFFSET);
	}
	if((rx_control->mask & PHY_INSYNC_MIS_LMT_EN) == PHY_INSYNC_MIS_LMT_EN)
	{
		if((rx_control->insync_mismatch_lmt & PHY_INSYNC_MIS_LEN_MASK) != 0x00 )
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : insync_mismatch_lmt. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_INSYNC_MIS_MASK) | (rx_control->insync_mismatch_lmt<< PHY_INSYNC_MIS_OFFSET);
	}
	if((rx_control->mask & PHY_GPON_GSYNC_PROT_EN) == PHY_GPON_GSYNC_PROT_EN)
	{
		if((rx_control->gpon_gsync_prot & PHY_GPON_GSYNC_LEN_MASK) != 0x00 )
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : gpon_gsync_prot. \n");
			return PHY_FAILURE;
		}
		read_data = read_data & (PHY_GPON_GSYNC_MASK) & (rx_control->gpon_gsync_prot << PHY_GPON_GSYNC_OFFSET);
	}

	write_data = read_data;
	IO_SPHYREG(PHY_CSR_GPON_PSYNC_CTL, write_data);
	return PHY_SUCCESS;
	
}




/*****************************************************************************
//function :
//		phy_epon_frame_err_cnt
//description : 
//		this function is used to get EPON 8b/10b error counter
//input :	
//		N/A
//output :
//		error counter
******************************************************************************/
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
#ifdef TCSUPPORT_WAN_EPON
int phy_epon_frame_err_cnt(void)
{
#ifdef CONFIG_USE_MT7520_ASIC
	IO_SPHYREG(PHY_CSR_RX_EPON_CNT_CTL,PHY_EPON_ERR_FRAME_LCH);//latch
	PON_PHY_MSG(PHY_MSG_DBG, "Phy Epon Frame Error Counter : %.8x\n", IO_GPHYREG(PHY_CSR_RX_EPON_FRAME_ERR));
	return IO_GPHYREG(PHY_CSR_RX_EPON_FRAME_ERR);
#else
	return 0;
#endif /* CONFIG_USE_MT7520_ASIC */
}
#endif /* TCSUPPORT_WAN_EPON */
#endif /* CONFIG_USE_MT7520_ASIC */



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
	#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		//unchar *ptr = NULL;
		unchar ptr[1];
		ushort u2ByteCnt;
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1,PHY_TRANS_ALARM2, ptr, 1);
		alarm_data= ptr[0];
		//if(ptr)
			//kfree(ptr);
	#else
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_ALARM2);
		alarm_data = IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
	#endif /* CONFIG_USE_MT7520_ASIC */
	if( (alarm_data & (PHY_RX_POWER_LOW_ALARM)) == PHY_RX_POWER_LOW_ALARM )
	{
		//change_alarm_led_status(ALARM_LED_FLICKER);
		PON_PHY_MSG(PHY_MSG_DBG, "Rx power is below low alarm. \n");
		return_value = return_value | PHY_TRANS_RX_LOW_POWER_ALARM;
	}
	
	if( (alarm_data & (PHY_RX_POWER_HIGH_ALARM)) == PHY_RX_POWER_HIGH_ALARM )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Rx power exceeds high alarm. \n");
		return_value = return_value |  PHY_TRANS_RX_HIGH_POWER_ALARM;
	}
	
	return return_value;
}


