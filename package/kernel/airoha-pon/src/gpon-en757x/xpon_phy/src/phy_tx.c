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
#include "en7571_api.h"   //dyma_20170117

extern PHY_GlbPriv_T *gpPhyPriv;

extern int	mt7570_select;
extern int	en7571_select;   //dyma_20170117
#ifdef TCSUPPORT_AUTOBENCH
//UINT16 SIF_X_Read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum, UINT32 u4WordAddr,
//                  UINT8 *pu1Buf, UINT16 u2ByteCnt)
//{
//	return 0;
//}
#endif

/*****************************************************************************
 //function :
 // 	 phy_gpon_preamble
 //description : 
 // 	 this function is used to set GPON preamble 
 //input :	 
 // 	 PPHY_GponPreb_T structure ; defined in phy_def.h
 //output :
 // 	 setting success or failure
******************************************************************************/
int phy_gpon_preamble(PPHY_GponPreb_T pon_preb)
{
	uint write_data = 0;
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_GPON_PREAMBLE);
	//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Gpon_Preamble read_data:0x%.8x\n", read_data);
	if((pon_preb->mask & PHY_GUARD_BIT_NUM_EN) == PHY_GUARD_BIT_NUM_EN )
	{
		read_data = (read_data & PHY_GUARD_BIT_NUM_MASK) | pon_preb->guard_bit_num;
	}
	if((pon_preb->mask & PHY_PRE_T1_NUM_EN) == PHY_PRE_T1_NUM_EN)
	{
		read_data = (read_data & PHY_PRE_T1_NUM_MAKS) | (pon_preb->preamble_t1_num << PHY_PRE_T1_NUM_OFFSET);
	}
	if((pon_preb->mask & PHY_PRE_T2_NUM_EN) == PHY_PRE_T2_NUM_EN)
	{
		read_data = (read_data & PHY_PRE_T2_NUM_MASK) | (pon_preb->preamble_t2_num << PHY_PRE_T2_NUM_OFFSET);
	}
	if((pon_preb->mask & PHY_PRE_T3_PAT_EN) == PHY_PRE_T3_PAT_EN)
	{
		read_data = (read_data & PHY_PRE_T3_PAT_MASK) | (pon_preb->preamble_t3_pat << PHY_PRE_T3_PAT_OFFSET);
	}
	write_data = read_data;
	//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Gpon_Preamble write_data:0x%.8x\n", write_data);
    gpPhyPriv->phyPreamble = write_data;
	IO_SPHYREG(PHY_CSR_GPON_PREAMBLE, write_data);
	return PHY_SUCCESS;

}



/*****************************************************************************
//function :
//		phy_tx_fec_status
//description : 
//		this is used to get the Tx FEC status
//input :	
//		N/A
//output :
//		PHY_TRUE : means Tx FEC now is enabled
//		PHY_FALSE : means TX FEC now is disabled
******************************************************************************/
int phy_tx_fec_status(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_TX_STATUS);
	if((read_data & PHY_TX_FEC) == PHY_TX_FEC)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "TX FEC on. \n");
		return PHY_TRUE;
	}
	else
	{
		PON_PHY_MSG(PHY_MSG_DBG, "TX FEC off. \n");
		return PHY_FALSE;
	}
	
}


/*****************************************************************************
 //function :
 // 	 phy_gpon_delimiter_guard
 //description : 
 // 	 this function is used to set delimiter pattern and guard time pattern
 //input :	 
 // 	 delimiter_pat : [24 bits] used to set delimiter pattern
 //	 guard_time_pat : [8 bits] used to set guard time pattern
 //output :
 // 	 setting success or failure
******************************************************************************/
int phy_gpon_delimiter_guard(uint delimiter_pat, unchar guard_time_pat)
{
	uint write_data = 0;
	if((delimiter_pat & PHY_GPON_DELIM_PAT_LEN_MASK) != 0)
	{
		PON_PHY_MSG(PHY_MSG_DBG, "error input : delimiter pattern. \n");
		return PHY_FAILURE;
	}
	else
	{
		write_data = (guard_time_pat << PHY_GPON_GUARD_PAT_OFFSET) | delimiter_pat;
		//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Gpon_Delimiter_Guard write_data:0x%.8x\n", write_data);
		gpPhyPriv->phyGuardBitDelm = write_data;
		IO_SPHYREG(PHY_CSR_GPON_DELIMITER_GUARD, write_data);
		return PHY_SUCCESS;
	}
}

/*****************************************************************************
 //function :
 // 	 phy_gpon_extend_preamble
 //description : 
 // 	 this function is used to set GPON extend preamble
 //input :	 
 // 	 PPHY_GponPreb_T structure ; defined in phy_def.h
 //output :
 // 	 setting success or failure
******************************************************************************/
int phy_gpon_extend_preamble(PPHY_GponPreb_T pon_preb)
{
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_GPON_EXTEND_PREAMBLE);
	//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble read_data:0x%.8x\n", read_data);

	if((pon_preb->mask & PHY_T3_O4_PRE_EN) == PHY_T3_O4_PRE_EN)
		read_data = (read_data & PHY_T3_O4_PRE_MASK) | pon_preb->t3_O4_preamble;
	if((pon_preb->mask & PHY_T3_O5_PRE_EN) == PHY_T3_O5_PRE_EN)
		read_data = (read_data & PHY_T3_O5_PRE_MASK) | (pon_preb->t3_O5_preamble << PHY_T3_O5_PRE_OFFSET);
	if((pon_preb->mask & PHY_EXT_BUR_MODE_EN) == PHY_EXT_BUR_MODE_EN)
	{
		if((pon_preb->extend_burst_mode != PHY_ENABLE) && (pon_preb->extend_burst_mode!= PHY_DISABLE))
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : extend_burst_mode. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_EXT_BUR_MODE_MASK) | (pon_preb->extend_burst_mode << PHY_EXT_BUR_MODE_OFFSET);
	}
	if((pon_preb->mask & PHY_OPER_RANG_EN) == PHY_OPER_RANG_EN)
	{
		if( (pon_preb->oper_ranged_st & PHY_OPER_RANG_LEG_MASK) != 0x00)
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_OPER_RANG_MASK) | (pon_preb->oper_ranged_st<< PHY_OPER_RANG_OFFSET);
	}
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	if((pon_preb->mask & PHY_EXTB_LENG_SEL_EN) == PHY_EXTB_LENG_SEL_EN)
	{
		if( (pon_preb->extb_length_sel & PHY_EXTB_LENG_SEL_LEN_MASK) != 0x00)
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : oper_ranged_st. \n");
			return PHY_FAILURE;
		}
		read_data = (read_data & PHY_EXTB_LENG_SEL_MASK) | (pon_preb->extb_length_sel<< PHY_EXTB_LENG_SEL_OFFSET);
	}
#endif /* CONFIG_USE_MT7520_ASIC */
	if((pon_preb->mask & PHY_DIS_SCRAM_EN) == PHY_DIS_SCRAM_EN)
	{
		if((pon_preb->dis_scramble != PHY_ENABLE) && (pon_preb->dis_scramble!= PHY_DISABLE))
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : dis_scramble. \n");
			return PHY_FAILURE;
		}	
		read_data = (read_data & PHY_DIS_SCRAM_MASK) | (pon_preb->dis_scramble<< PHY_DIS_SCRAM_OFFSET);
	}
	write_data = read_data;
				

	//PON_PHY_MSG(PHY_MSG_DBG, "Phy_Gpon_Extend_Preamble write_data:0x%.8x\n", write_data);
	gpPhyPriv->phyExtPreamble = write_data;
	IO_SPHYREG(PHY_CSR_GPON_EXTEND_PREAMBLE, write_data);
	return PHY_SUCCESS;
	
}


/*****************************************************************************
 //function :
 // 	 phy_tx_rogue_prbs_config
 //description : 
 // 	 this function is used in rogue mode, echo Phy_Rogue_PRBS 1/0 > /proc/pon_phy/debug
 //input :	 
 // 	 rogue_mode : ONU will send prbs which make other ONU offline.
 //output :
 // 	 setting success or failure
******************************************************************************/
int phy_rogue_prbs_config(unchar rogue_mode)
{
#if defined(CONFIG_USE_A60928) || defined(CONFIG_USE_A60901)
	uint phyRegData = 0 ;
#endif


	uint write_data = 0 ;
	uint read_data = 0 ;


	if((rogue_mode != PHY_TX_ROGUE_MODE) && (rogue_mode != PHY_TX_NORMAL_MODE))
	{
		PON_PHY_MSG(PHY_MSG_ERR, "error input : rogue mode. \n");
		return PHY_FAILURE;
	}

	if(PHY_GPON_CONFIG != gpPhyPriv->phyCfg.flags.mode && \
       PHY_EPON_CONFIG != gpPhyPriv->phyCfg.flags.mode    )
	{
		PON_PHY_MSG(PHY_MSG_ERR, "PON mode error. \n");
		return PHY_FAILURE;
	}

    if(rogue_mode == PHY_TX_ROGUE_MODE) 
	{
        REPORT_EVENT_TO_MAC(PHY_EVENT_START_ROGUE_MODE);
        gpPhyPriv->phyCfg.flags.rogue = 1;
        if(PHY_GPON_CONFIG == gpPhyPriv->phyCfg.flags.mode )
			{
				/* GPON mode set PRBS pattern */
#if defined(CONFIG_USE_MT7520_ASIC)
            /* GPON set PRBS pattern, it will cause Illegal INT */
#ifdef TCSUPPORT_CPU_EN7521 //EN7521 GPON Rogue ONU mode
			read_data = IO_GPHYREG(PHY_CSR_MISC) ;
			write_data = read_data | (0x01<<28);
			IO_SPHYREG(PHY_CSR_MISC,write_data);
#endif				
			//IO_SBITS(0xBFAF04a0,0x5);
			//IO_SBITS(0xBFAF04a4,0x1);
			read_data = IO_GPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL) ;
			write_data = read_data | 0x05;
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,write_data);

			read_data = IO_GPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN) ;
			write_data = read_data | 0x01;				
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,write_data);

#elif defined(CONFIG_USE_A60928) || defined(CONFIG_USE_A60901)
			phyRegData = IO_GPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL);
			phyRegData |= (0x01 << 0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL, phyRegData);
#endif
		}else if (PHY_EPON_CONFIG == gpPhyPriv->phyCfg.flags.mode){
				/* EPON mode set continue mode */
				//IO_SBITS(0xBFAF0108,0x80);
				read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
				write_data = read_data | 0x80;
				IO_SPHYREG(PHY_CSR_PHYSET3,write_data);
			}
		
			PON_PHY_MSG(PHY_MSG_DBG, "Phy_Rogue_PRBS mode\n");
        } 
		else 
		{
    	if(PHY_GPON_CONFIG == gpPhyPriv->phyCfg.flags.mode)
					{
#if defined(CONFIG_USE_MT7520_ASIC)
				/* GPON clear set PRBS pattern */
#if TCSUPPORT_CPU_EN7521
			read_data = IO_GPHYREG(PHY_CSR_MISC) ;
			write_data = read_data & ~(0x01<<28);
			IO_SPHYREG(PHY_CSR_MISC,write_data);			
#endif

			//IO_CBITS(0xBFAF04a0,0x5);
			//IO_CBITS(0xBFAF04a4,0x1);
			read_data = IO_GPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL) ;
			write_data = read_data & ~0x05;
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,write_data);

			read_data = IO_GPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN) ;
			write_data = read_data & ~0x01;				
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,write_data);
				
        	IO_SPHYREG(PHY_CSR_XPON_INT_STA_CLR, PHY_ILLG_INT) ;

#elif defined(CONFIG_USE_A60928) || defined(CONFIG_USE_A60901)
			/* set register 4a0[0] to 0 for closing rogue */
			phyRegData = IO_GPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL);
			phyRegData &= ~(0x01 << 0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL, phyRegData);
			/* disable tx invert */
			phy_trans_burst_invert(PHY_DISABLE);
#endif
        }else if (PHY_EPON_CONFIG == gpPhyPriv->phyCfg.flags.mode){
			//IO_CBITS(0xBFAF0108,0x80);
			read_data = IO_GPHYREG(PHY_CSR_PHYSET3) ;
			write_data = read_data & ~0x80;
			IO_SPHYREG(PHY_CSR_PHYSET3,write_data);	
			}

        REPORT_EVENT_TO_MAC(PHY_EVENT_STOP_ROGUE_MODE);
        gpPhyPriv->phyCfg.flags.rogue = 0;
        
            PON_PHY_MSG(PHY_MSG_DBG, "Phy_NO_Rogue_PRBS mode\n");
        }
    
		return PHY_SUCCESS;
}

/*****************************************************************************
 //function :
 // 	 phy_tx_burst_config
 //description : 
 // 	 this function is used to set Tx burst/continuous mode
 //input :	 
 // 	 burst_mode : PHY_TX_BURST_MODE to make Tx work in burst mode ;  PHY_TX_CONT_MODE to make Tx work in continuous mode
 //output :
 // 	 setting success or failure
******************************************************************************/
int phy_tx_burst_config(unchar burst_mode)
{
	if((burst_mode != PHY_TX_BURST_MODE) && (burst_mode != PHY_TX_CONT_MODE))
	{
		PON_PHY_MSG(PHY_MSG_ERR, "error input : burst_mode. \n");
		return PHY_FAILURE;
	}
	else
	{
		uint write_data = 0;
		uint read_data = 0;
		read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
		write_data = (read_data & PHY_TX_CONT_MODE_MASK) | (burst_mode * PHY_TX_CONT_MODE_OFFSET);
		PON_PHY_MSG(PHY_MSG_DBG, "Phy_Tx_Burst_Config read_data:0x%.8x\n", read_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Phy_Tx_Burst_Config write_data:0x%.8x\n", write_data);
		IO_SPHYREG(PHY_CSR_PHYSET3, write_data);
		//sw workaround
		if(phy_los_status() == PHY_LOS_HAPPEN)
		{
			/*reset phy*/
			read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
			write_data = read_data | (PHY_PLL_RST) |(PHY_COUNT_RST);
			IO_SPHYREG(PHY_CSR_PHYSET3, write_data) ;
			mdelay(1);
			IO_SPHYREG(PHY_CSR_PHYSET3, read_data) ;
		}

		gpPhyPriv->phyCfg.flags.txLongFlag = ((burst_mode==PHY_TX_CONT_MODE)?PHY_TRUE:PHY_FALSE);

		if (burst_mode == PHY_TX_BURST_MODE){
			phy_tx_power_config(PHY_ENABLE);
			phy_trans_power_switch(PHY_ENABLE);
		}
		
		return PHY_SUCCESS;
	}
}

/*****************************************************************************
 //function :
 // 	 phy_tx_burst_getting
 //description : 
 // 	 this function is used to get Tx burst/continuous status
 //input :	 
 // 	 N/A
 //output :
 // 	 PHY_TX_BURST_MODE means that Tx work in burst mode ;  PHY_TX_CONT_MODE means that Tx work in continuous mode
******************************************************************************/
int phy_tx_burst_getting(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_PHYSET3);
	if( (read_data & PHY_TX_CONT_MODE_OFFSET)== 0 )
		return PHY_TX_BURST_MODE;
	else
		return PHY_TX_CONT_MODE;
}

/*****************************************************************************
******************************************************************************/
int phy_gpon_tx_power(ushort tx_power)
{
	return 0;
}

/*****************************************************************************
//function :
//		phy_tx_amp_setting(
//description : 
//		this function is used to modify Tx output swing
//input :	
//		amp_level : [4 bits] used to set Tx output swing
//output :
//		setting success or failure
******************************************************************************/
int phy_tx_amp_setting(unchar amp_level)
{
	if((amp_level & PHY_TX_AMP_LEVEL_LEN_MASK) != 0x0)
	{
		PON_PHY_MSG(PHY_MSG_ERR, "error input : amp_level. \n");
		return PHY_FAILURE;
	}
	else
	{
		uint read_data = 0;
		uint write_data = 0;
		read_data = IO_GPHYREG(PHY_CSR_ANATXREG1);
		write_data = (read_data & PHY_TX_AMP_LEVEL_MASK) | (amp_level << PHY_TX_AMP_LEVEL_OFFSET);
		IO_SPHYREG(PHY_CSR_ANATXREG1 , write_data);
		return PHY_SUCCESS;
	}
}

/*****************************************************************************
//function :
//		phy_trans_tx_setting
//description : 
//		this function is used for transceiver to define pin function 
//input :	
//		PPHY_TransConfig_T structure : PHY_ENABLE means low active ; PHY_DISABLE means high active
//output :
//		setting success or failure
******************************************************************************/
int phy_trans_tx_setting(PPHY_TransConfig_T trans_status)
{
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);
	if((trans_status->trans_tx_sd_inv_status == PHY_ENABLE) || (trans_status->trans_tx_sd_inv_status == PHY_DISABLE) )//tx_sd
		read_data = (read_data & PHY_TX_SD_INV_MASK) | (trans_status->trans_tx_sd_inv_status << PHY_TX_SD_INV_OFFSET);
	else 
		PON_PHY_MSG(PHY_MSG_ERR, "error input : tx_sd_inv.\n");

	if((trans_status->trans_burst_en_inv_status == PHY_ENABLE) || (trans_status->trans_burst_en_inv_status == PHY_DISABLE) )//burst_en
		read_data = (read_data & PHY_BURST_EN_INV_MASK) | (trans_status->trans_burst_en_inv_status<< PHY_BURST_EN_INV_OFFSET);
	else 
		PON_PHY_MSG(PHY_MSG_ERR, "error input : burst_en_inv.\n");

	if((trans_status->trans_tx_fault_inv_status == PHY_ENABLE) || (trans_status->trans_tx_fault_inv_status == PHY_DISABLE) )//tx_fault
		read_data = (read_data & PHY_TX_FAULT_INV_MASK) | (trans_status->trans_tx_fault_inv_status << PHY_TX_FAULT_INV_OFFSET);
	else 
		PON_PHY_MSG(PHY_MSG_ERR, "error input : tx_fault_inv.\n");

	write_data = read_data;
	IO_SPHYREG(PHY_CSR_XPON_SETTING,write_data);
	return PHY_SUCCESS;
}


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
	uint read_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);
	if((read_data & ~(PHY_TX_SD_INV_MASK))!= 0)
		trans_status->trans_tx_sd_inv_status = PHY_ENABLE;
	else
		trans_status->trans_tx_sd_inv_status = PHY_DISABLE;
	if((read_data & ~(PHY_BURST_EN_INV_MASK))!= 0)
		trans_status->trans_burst_en_inv_status = PHY_ENABLE;
	else
		trans_status->trans_burst_en_inv_status = PHY_DISABLE;
	if((read_data & ~(PHY_TX_FAULT_INV_MASK))!= 0)
		trans_status->trans_tx_fault_inv_status = PHY_ENABLE;
	else
		trans_status->trans_tx_fault_inv_status = PHY_DISABLE;
	
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
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		//unchar *ptr = NULL;
		unchar ptr[1];
		ushort u2ByteCnt;
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 1,PHY_TRANS_ALARM1, ptr, 1);
		read_data = ptr[0];
		//if(ptr)
			//kfree(ptr);
	#else
		io_trans_reg(PHY_TRANS_REG_TABLE2,PHY_TRANS_ALARM1);
		read_data = IO_GPHYREG(PHY_CSR_I2C_DATA_PORT);
	#endif  /* CONFIG_USE_MT7520_ASIC */
	if( (read_data & (PHY_TX_POWER_LOW_ALARM)) == PHY_TX_POWER_LOW_ALARM )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Tx power is below low alarm. \n");
		return_value = return_value | PHY_TRANS_TX_LOW_POWER_ALARM;
	}

	if( (read_data & (PHY_TX_POWER_HIGH_ALARM)) == PHY_TX_POWER_HIGH_ALARM )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Tx power exceeds high alarm. \n");
		return_value = return_value | PHY_TRANS_TX_HIGH_POWER_ALARM;
	}

	if( (read_data & (PHY_TX_CUR_LOW_ALARM)) == PHY_TX_CUR_LOW_ALARM )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Bias Current is below low alarm. \n");
		return_value = return_value | PHY_TRANS_TX_LOW_CUR_ALARM;
	}

	if( (read_data & (PHY_TX_CUR_HIGH_ALARM)) == PHY_TX_CUR_HIGH_ALARM )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Bias Current exceeds high alarm. \n");
		return_value = return_value | PHY_TRANS_TX_HIGH_CUR_ALARM;
	}
	return return_value;
}



/*****************************************************************************
//function :
//		phy_tx_frame_counter
//description : 
//		this function is used to read Tx frame counter
//input :	
//		N/A
//output :
//		Tx frame counter
******************************************************************************/
int phy_tx_frame_counter(void)
{
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_TX_TEST_TRIG);
	write_data = read_data | PHY_TX_CNT_LCH;//latch
	IO_SPHYREG(PHY_CSR_TX_TEST_TRIG,write_data);
	read_data = IO_GPHYREG(PHY_CSR_TX_FRAME_COUNTER);
	PON_PHY_MSG(PHY_MSG_DBG, "Phy Tx Frame Counter : %.8x\n", read_data);
	return read_data;
#else
	return 0;
#endif /* CONFIG_USE_MT7520_ASIC */
}

/*****************************************************************************
//function :
//		phy_tx_burst_counter
//description : 
//		this function is used to read Tx burst counter
//input :	
//		N/A
//output :
//		Tx burst counter
******************************************************************************/
int phy_tx_burst_counter(void)
{
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	uint read_data = 0;
	uint write_data = 0;
	read_data = IO_GPHYREG(PHY_CSR_TX_TEST_TRIG);
	write_data = read_data | PHY_TX_CNT_LCH;//latch
	IO_SPHYREG(PHY_CSR_TX_TEST_TRIG,write_data);
	read_data = IO_GPHYREG(PHY_CSR_TX_BURST_COUNTER);
	PON_PHY_MSG(PHY_MSG_DBG, "Phy Tx Burst Counter : %.8x\n", read_data);
#else
	return 0;
#endif /* CONFIG_USE_MT7520_ASIC */
}

/*****************************************************************************
//function :
//		phy_tx_test_pattern
//description : 
//		this function is used to transmit test pattern
//input :	
//		pattern
//output :
//		N/A
******************************************************************************/
void phy_tx_test_pattern(uint pattern)
{
	uint read_data = 0;
	uint write_data = 0;
	switch(pattern)
	{
		case PHY_BIST_IDLE :
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,pattern);
			break;	
		case PHY_BIST_CLOCK :
		case PHY_BIST_CLOCK_DIV5 :
		case PHY_BIST_CLOCK_MIX :
		case PHY_BIST_REVRS_LP :
	#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
		case PHY_BIST_ALL0 :
		case PHY_BIST_ALL1 :
	#endif /* CONFIG_USE_MT7520_ASIC */
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,pattern);
			break;	
		case PHY_BIST_PRBS7 :
		case PHY_BIST_PRBS23 :
		case PHY_BIST_PRBS31 :
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,pattern);
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,1);
			break;
		case PHY_EPON_TEST_PAT:
			IO_SPHYREG(PHY_CSR_TF_PAT_SEL,2);
			IO_SPHYREG(PHY_CSR_TEST_FRAME_EN,1);
			break;
		case PHY_EPON_TEST_PAT_0:
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,0);
			IO_SPHYREG(PHY_CSR_TF_PAT_SEL,0);
			IO_SPHYREG(PHY_CSR_TEST_FRAME_EN,0);
			read_data = IO_GPHYREG(PHY_CSR_TX_FECENC_SW_RST)&PHY_CSR_TX_SW_RST_MASK;
			write_data = (read_data|~PHY_CSR_TX_SW_RST_MASK); 
			IO_SPHYREG(PHY_CSR_TX_FECENC_SW_RST,write_data);
			IO_SPHYREG(PHY_CSR_TX_FECENC_SW_RST,read_data);			
		default : 
			IO_SPHYREG(PHY_CSR_BISTCTL_PRBS_TX_EN,0);
			IO_SPHYREG(PHY_CSR_BISTCTL_LOOPBACK_SEL,0);
			break;			
	}
}

#if defined(CONFIG_USE_A60928) || defined(CONFIG_USE_A60901)
/*****************************************************************************
//function :
//		phy_trans_burst_invert
//description : 
//		Transceiver single end burst enable invert
//input :	
//		enable: PHY_ENABLE  means enable invert ; PHY_DISABLE  means disable invert 
//output :
//		N/A
******************************************************************************/
void phy_trans_burst_invert(unchar enable)
{
	uint read_data = 0;

	if(enable == PHY_ENABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);
		read_data |= (0x01 << 7);
		IO_SPHYREG(PHY_CSR_XPON_SETTING, read_data);
	}
	else if(enable == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_XPON_SETTING);
		read_data &= ~(0x01 << 7);
		IO_SPHYREG(PHY_CSR_XPON_SETTING, read_data);
	}

    return ;
}
#endif

#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
/*****************************************************************************
//function :
//		phy_byte_delay_manual
//description : 
//		this function is used to set byte delay by manual
//input :	
//		byte_dly_en : PHY_ENABLE  means enable manual mode ; PHY_DISABLE  means disable manual mode
//		byte_dly : [2 bits] used to set byte delay value
//output :
//		setting success or failure
******************************************************************************/
int phy_byte_delay_manual(unchar byte_dly_en, unchar byte_dly)
{
	uint read_data = 0;
	uint write_data = 0;
	if(byte_dly_en == PHY_ENABLE)
	{
		if((byte_dly & PHY_BYTE_DELAY_LEN_MASK) != 0x00)//check length
		{
			PON_PHY_MSG(PHY_MSG_ERR, "error input : byte dly. \n");
			return PHY_FAILURE;
		}
		read_data = IO_GPHYREG(PHY_CSR_TX_MISC);
		write_data = (read_data & PHY_BYTE_DELAY_MASK) | PHY_BYTE_DELAY_MANUAL | (byte_dly<<PHY_BYTE_DELAY_OFFSET);//clear and set
		IO_SPHYREG(PHY_CSR_TX_MISC,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Byte delay manual enable setting success. \n");
		return PHY_SUCCESS;
	}
	else if(byte_dly_en == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_TX_MISC);
		write_data = (read_data & PHY_BYTE_DELAY_MASK);//clear setting
		IO_SPHYREG(PHY_CSR_TX_MISC,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Byte delay manual disable setting success. \n");
		return PHY_SUCCESS;
	}
	else
		return PHY_FAILURE;
}

/*****************************************************************************
//function :
//		phy_tx_fec_manual
//description : 
//		this function is used to set Tx FEC by manual
//input :	
//		tx_fec_en : PHY_ENABLE  means enable manual mode ; PHY_DISABLE  means disable manual mode
//output :
//		setting success or failure
******************************************************************************/
int phy_tx_fec_manual(unchar tx_fec_en)
{
	uint read_data = 0;
	uint write_data = 0;
	if(tx_fec_en == PHY_ENABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_TX_MISC);
		write_data = (read_data & PHY_TX_FEC_MANUAL_MASK) | PHY_TX_FEC_MANUAL | PHY_TX_FEC_EN;//clear and set
		IO_SPHYREG(PHY_CSR_TX_MISC,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Tx FEC manual enable setting success. \n");
		return PHY_SUCCESS;
	}
	else if(tx_fec_en == PHY_DISABLE)
	{
		read_data = IO_GPHYREG(PHY_CSR_TX_MISC);
		write_data = (read_data & PHY_TX_FEC_MANUAL_MASK);//clear setting
		IO_SPHYREG(PHY_CSR_TX_MISC,write_data);
		PON_PHY_MSG(PHY_MSG_DBG, "Tx FEC manual disable setting success. \n");
		return PHY_SUCCESS;
	}
	else
		return PHY_FAILURE;
}

void phy_tx_ctl(unchar val)
{
	if(PHY_ENABLE == val) /* enable tx power should turn off tx disable */
		ledTurnOff(LED_PHY_TX_POWER_DISABLE);
	else if (PHY_DISABLE == val) /* disbale tx power should turn on tx disable */
		ledTurnOn(LED_PHY_TX_POWER_DISABLE);
	else
		PON_PHY_MSG(PHY_MSG_ERR, "PHY Tx ctl type error. \n");
}


void mt7570_safe_circuit_reset_for_work_queue(struct work_struct * worker)
{
    mt7570_safe_circuit_reset();
}
void en7571_safe_circuit_reset_for_work_queue(struct work_struct * worker)   //dyma_20170117
{
    en7571_safe_circuit_reset();
}

DECLARE_WORK(work_mt7570_safe_circuit_reset, mt7570_safe_circuit_reset_for_work_queue);
DECLARE_WORK(work_mt7571_safe_circuit_reset, en7571_safe_circuit_reset_for_work_queue);  //dyma_20170117

#endif /* CONFIG_USE_MT7520_ASIC */

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
#if defined(CONFIG_USE_MT7520_ASIC)
	uint read_data = 0;
	uint write_data = 0 ;


        if(trans_switch == PHY_DISABLE)
        {
			phy_tx_ctl(PHY_DISABLE);
			PON_PHY_MSG(PHY_MSG_DBG, "turn off transceiver power success! \n");
            return PHY_SUCCESS;
        }
        else if(trans_switch == PHY_ENABLE)
        {
            if (gpPhyPriv->phyCfg.flags.txPowerEnFlag) {
				phy_tx_ctl(PHY_ENABLE);
				
				if((mt7570_select==1)&&(NULL != i2c_access_queue)){
					queue_work(i2c_access_queue, &work_mt7570_safe_circuit_reset);
				}
				if((en7571_select==1)&&(NULL != i2c_access_queue)){
					queue_work(i2c_access_queue, &work_mt7571_safe_circuit_reset);   //dyma_20170117
				}

				PON_PHY_MSG(PHY_MSG_DBG, "turn on transceiver power success! \n");
	            }
			return PHY_SUCCESS;
		}
		else
		{
			PON_PHY_MSG(PHY_MSG_DBG, "setting transceiver power fail. \n");
			return PHY_FAILURE;
		}
#else

#if defined(CONFIG_USE_A60928) || defined(CONFIG_USE_A60901)
	/* FPGA has no GPIO to disable transceiver power, we only can use phy invert 
	 * function to switch off the laser.
	 */

    phy_trans_burst_invert(PHY_DISABLE == trans_switch ? PHY_ENABLE : PHY_DISABLE);
	return PHY_SUCCESS;

#else 
	return PHY_FAILURE;
#endif

#endif
}



