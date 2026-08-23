//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : EN7581 PON PHY
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
#include "a60972_reg.h"
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include "en7581.h"
#include "en7581_reg.h"
#include "en7581_pma.h"
//#include "en7581_ana.h"


#if EN7581_EFUSE
	#ifdef TCSUPPORT_CPU_EN7581
		#include <boot/packageInfo.h>
	#endif
#endif

// --  variables ----------------------------------------------------------------


#define _definition_SPLIT_LINE_

#if ASIC_SERDES	// for EN7581_ASIC

#define _PMA_FUNCTION_SPLIT_LINE_BASIC 


uint InitSpd_TX = 0 , InitSpd_TXPLL = 0 , InitSpd_RX = 0 , InitSpd_ANA = 0 , wan_sel = 0 , RX_CDR_FM = 0;

AN7581_XPON_PMA_Param_T pon_pma_param;



// -- EN7581 PMA function split  from Morris------------------------------------------------

int xpon_pma_mode_init(void)
{
	UINT32 read_data ;

    #if !EN7581_EYE_SCAN
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
    #endif
    
    #if EN7581_EYE_SCAN
        printk("___________PHY init for EN7581_EYE_SCAN_____________\r\n");
    #endif

	return 0;
}
//======================================================================================

void xpon_init (int bit_sel)
{
	XPON_DIG_reset_hold();	
	pon_CfgPhyType(bit_sel);
	pon_LinkControl(bit_sel);  
}


void pon_LinkControl (int pon_Spd)
{
   JCPLL_on();   
   TXPLL_on();
   XPON_DIG_fm_on(pon_Spd); 	   
   XPON_TX_on();
}


//======================================================================================

void fiber_plug_reset (int plug_sel , int pon_Spd)
{
	static int  plug_out_flag=1,plug_in_flag=0;
	uint read_data = 0;
    switch(plug_sel)
	{
        case FIRST_PLUG_IN: 
            XPON_RX_preset(); 
            XPON_TDC_off(); 
            XPON_RX_on(pon_Spd); 
            XPON_RX_L2R(); 
			
			XPON_DIG_ref_release();

            XPON_RX_OSCal(); 
            XPON_RX_pical();
            XPON_RX_pdos(); 

			if(GET_PDIDR() == 0x1) 
			{
			   if((pon_Spd == Async_XEPON) ||(pon_Spd == Sync_XEPON) || (pon_Spd == Sync_XGSPON_1)|| (pon_Spd == Sync_XGSPON_2)|| (pon_Spd == Async_XGPON_1)|| (pon_Spd == Async_XGPON_2))
			   {				  
				  XPON_RX_feos();
			   }
			}
			else
			{
              XPON_RX_feos();
			}
			
            XPON_RX_sdcal();   
   
            XPON_phy_status();
			XPON_DIG_reset_release();	
            XPON_RX_L2D();   
            EO_Scan(pon_Spd,0,7); //20221011
            XPON_TDC_on(); 
			XPON_RX_rxrdy();  
            XPON_BIST_setting();
            XPON_R2T_sel(nor_pma_data);  
					
			plug_out_flag =1;
			plug_in_flag =0;
			
        break;	
        //-----------------------------------------------------------------
        
        case PLUG_IN: 
			if(plug_in_flag ==1)
			{  	
			    XPON_DIG_reset_hold();				
			    XPON_RX_L2D();
			   
			    XPON_TDC_on(); 			   
			    //TDC_Release();	
			   
			    XPON_DIG_reset_release();		
			    XPON_RX_rxrdy(); 			   
			    XPON_phy_status();
			    XPON_R2T_sel(nor_pma_data); 		
		       
				plug_out_flag =1;
				plug_in_flag =0;				
			}
        break;	
        //-----------------------------------------------------------------

        case PLUG_OUT:
			if(plug_out_flag ==1)
			{
				XPON_TDC_off();
				//TDC_Recall(InitSpd_TXPLL);	
				
			    XPON_RX_L2R(); 

				XPON_FLL_Reset();

				plug_out_flag =0;
				plug_in_flag =1;
			}
        break;	
        //-----------------------------------------------------------------
        default:
	    break;
    }
}


void pon_EO_Scan (void)
{  
   EO_Scan(InitSpd_RX,0,7); //20221011   
}

void pon_PR_WK (int pon_Spd)
{  
   XPON_PrCal_WK(pon_Spd);
}


void pon_CfgPhyType (int pon_Spd)
{
	pon_WanSelInit(pon_Spd);
	
	//JCPLL_Dis
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_jcpll_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 16, 16, 0x00); //rg_force_da_pxp_jcpll_en	
	
	//JCPLL_PCW
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_vos, 16, 16, 0x01); //rg_force_sel_da_pxp_jcpll_sdm_pcw
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_sdm_pcw, 31, 0, 0x25800000); //rg_force_da_pxp_jcpll_sdm_pcw
	
	//TXPLL_Dis	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_txpll_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x00); //rg_force_da_pxp_txpll_en
	  
	
	// TDC	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_3, 1, 0, 0x01);  //rg_lcpll_ncpo_shift
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_1, 11, 8, lcpll_a_tdc);  //rg_lcpll_a_tdc
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_MULTLANE_EN, 17, 16, 0x00); //RG_PXP_TX_TDC_CK_SEL
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 24, 24, 0x01);   //RG_PXP_RX_TDC_CK_SEL
    
	
	//PLL EN HW Mode 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 31, 24, 0x01);  // rg_lcpll_ck_stb_timer
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 23, 16, 0x10);  //rg_lcpll_pcw_man_load_timer
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 15, 8, 0x0A);   //rg_lcpll_en_timer
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 0, 0, 0x01);    //rg_lcpll_man_pwdb
	
	
	//pma_dig_tx_setting
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_1, 31, 16, 0x113); //rg_tx_hsdata_en_wait
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_1, 15, 0, 0xFA);   //rg_tx_ck_en_wait	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_2, 31, 16, 0x9B); //rg_tx_serdes_rdy_wait
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_2, 15, 0, 0x210); //rg_tx_power_on_wait


	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_0, 31, 16, 0x04); //rg_txcalib_5us
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PON_TX_COUNTER_0, 15, 0, 0x26);  //rg_txcalib_50us

	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 23, 16, 0x02); //rg_tx_ben_exten_ftune
	
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_0, 10, 8, 0x03); //rg_lcpll_ki
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PW_5, 24, 24, 0x00); //rg_lcpll_tdc_sync_in_mode	

	XPON_TXPLL(InitSpd_TXPLL);	
	XPON_TX(pon_Spd); 
	

	
	//pma_dig_rx_setting 
	//RX HW mode counter 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0, 23, 8, 0x01);  //rg_rx_os_start
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6, 15, 0, 0x02);  //rg_rx_os_end
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0, 2, 0, 0x01);   //rg_osc_speed_opt
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 15, 0, 0x02);   //rg_rx_pical_start
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 31, 16, 0x3E8); //rg_rx_pical_end

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 15, 0, 0x02);  //rg_rx_sdcal_start
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 31, 16, 0x3E8); //rg_rx_sdcal_end

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 15, 0, 0x02);  //rg_rx_pdos_start
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 31, 16, 0x3E8); //rg_rx_pdos_end
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 15, 0, 0x02);  //rg_rx_feos_start
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 31, 16, 0x3E8); //rg_rx_feos_end
	
	
	//RX setting 	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_2, 12, 8, 0x01); //rg_fom_num_order
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_2, 1, 0, 0x03);  //rg_a_sel

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240); //rg_x_max
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);  //rg_x_min

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x00); //rg_data_shift
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 0, 0, 0x01); //rg_eyecnt_fast

	XPON_RX(InitSpd_RX); 	
	XPON_ANA(InitSpd_ANA); 	 
	
	//EYE
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x01); //rg_eye_nextpts

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x04); //rg_eyecnt_vth
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x04);  //rg_eyecnt_hth
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x04); //rg_eo_vth
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x04);  //rg_eo_hth

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF); //rg_eye_mask
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xD0);   //rg_cntlen

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x01);   //rg_eq_en_delay
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 18, 8, 0x00);  //rg_heo_mask
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 31, 24, 0x00); //rg_veo_mask	

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_1, 0, 0, 0x01);   //rg_a_lgain
	
	
	//(Default)
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_1, 25, 24, 0x01);  //rg_cal_cyc
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_1, 15, 8, 0x2E);   //rg_cal_1us_set
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_1, 0, 0, 0x01);    //rg_sim_fast_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_1, 17, 16, 0x01);  //rg_cal_stb
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_2, 0, 0, 0x00);   //rg_cal_os_pulse
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_2, 11, 8, 0x00);  //rg_cal_out_os
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_CAL_2, 17, 16, 0x00); //rg_cal_cyc_time

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 15, 0, 0x05);  //rg_rx_blwc_rdy_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 31, 16, 0x0A); //rg_rx_rdy
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FEOS, 7, 0, 0x00); //rg_lfsel	

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FEOS, 8, 8, 0x00); //rg_eq_force_blwc_freeze
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 6, 0, 0x40);   //rg_y_min
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 14, 8, 0x3F);  //rg_y_max
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x01); //rg_index_mode
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x18);  //rg_eyedur

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EXTRAL_CTRL, 15, 8, 0x02); //rg_l2d_trig_eq_en_time
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EXTRAL_CTRL, 1, 1, 0x01);  //rg_os_rdy_latch
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EXTRAL_CTRL, 0, 0, 0x00);  //rg_disb_leq
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_0, 18, 8, 0xA5);  //rg_fpkdiv
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_0, 2, 0, 0x03);   //rg_kband_prediv
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_0, 25, 24, 0x00); //rg_kband_kfc	
	
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_1, 26, 24, 0x04); //rg_symbol_wd
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_1, 18, 16, 0x01); //rg_settle_time_sel
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_5, 10, 0, 0x1FF);  //rg_fll_idac_max
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_5, 26, 16, 0x400); //rg_fll_idac_min

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_2, 10, 8, 0x04); //rg_amp
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_2, 2, 0, 0x03);  //rg_prbs_sel

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_4, 24, 24, 0x00); //rg_disb_blwc_offset
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_PDOS_CTRL_0, 0, 0, 0x01);   //rg_eye_blwc_add
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_PDOS_CTRL_0, 8, 8, 0x00);   //rg_data_blwc_add
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_BLWC, 0, 0, 0x01);   //rg_eq_blwc_pol
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_BLWC, 11, 8, 0x0A);  //rg_eq_blwc_gain
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_BLWC, 22, 16, 0x70); //rg_eq_blwc_cnt_top_lim
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_BLWC, 29, 23, 0x10); //rg_eq_blwc_cnt_bot_lim
	
}


void pon_WanSelInit (uint pon_Spd)
{

   switch(pon_Spd)
   {
     
	   case Async_GPON: 
		 
		InitSpd_TXPLL = 9; 		
		InitSpd_RX = 21;
		InitSpd_ANA = 2;

		break;	


	   case Sync_EPON_1: 
	   case Sync_EPON_2: 

	    InitSpd_TXPLL = 1; 		
		InitSpd_RX = 1;
		InitSpd_ANA = 1;	

		break;	
		
	   
	   case Async_XEPON: 

	    InitSpd_TXPLL = 10; 		
		InitSpd_RX = 10;
		InitSpd_ANA = 1;	
		
		break;

	  
	   case Sync_XEPON: 

		InitSpd_TXPLL = 10; 		
		InitSpd_RX = 10;
		InitSpd_ANA = 10;			

		break;
		

	   case Async_XGPON_1: 
	   case Async_XGPON_2:

	    InitSpd_TXPLL = 9; 		
	    InitSpd_RX = 9;
	    InitSpd_ANA = 1;		

		break;


	   case Sync_XGSPON_1: 
	   case Sync_XGSPON_2: 

	    InitSpd_TXPLL = 9; 		
	    InitSpd_RX = 9;
	    InitSpd_ANA = 9;			

		break;
		
	  
	   case Sync_GPON:  

	    InitSpd_TXPLL = 2; 		
	    InitSpd_RX = 2;
	    InitSpd_ANA = 2;		
	    RX_CDR_FM = 99;		

		break;


   }



}


void XPON_TXPLL(uint rate_sel)
{
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x01); //rg_force_sel_da_pxp_txpll_sdm_pcw
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_3, 8, 8, 0x01);            //rg_lcpll_ncpo_load

switch(rate_sel)
	{
	 //TXPLL_PCW
	case 10: // 10G_ mode   
		 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30, 0, 0x08400000); //rg_force_da_pxp_txpll_sdm_pcw
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_1, 30, 0, 0x10800000); //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_2, 30, 0, 0x10800000); //rg_lcpll_pon_hrdds_pcw_ncpo_epon
         		 

 #if LAB_PRINT_PON
         printk("=== XPON_TXPLL_10G_done ===\n");
 #endif
		  
		 break;  
		 
    case 9: // 9G_ mode
    case 2: // 2G_ mode
		 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30, 0, 0x07F66E86); //rg_force_da_pxp_txpll_sdm_pcw
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_1, 30, 0, 0x0FECDD0C); //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_2, 30, 0, 0x0FECDD0C); //rg_lcpll_pon_hrdds_pcw_ncpo_epon
         	 
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_9G_done ===\n");
         printk("=== XPON_TXPLL_2G_done ===\n");
 #endif
		   
		 break;	

	case 1: // 1G_ mode
		 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30, 0, 0x08000000); //rg_force_da_pxp_txpll_sdm_pcw
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_1, 30, 0, 0x10000000); //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PCW_2, 30, 0, 0x10000000); //rg_lcpll_pon_hrdds_pcw_ncpo_epon
         		 
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_1G_done ===\n");
 #endif
		 
		 break;	
		
		default:
		    break;	
     }		 

}


void XPON_TX(uint tx_bit_sel)
{

    //TX_Config
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_CKLDO_EN, 24, 24, 0x01); //RG_PXP_TX_DMEDGEGEN_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_CKLDO_EN, 0, 0, 0x01);   //RG_PXP_TX_CKLDO_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CMN_EN, 0, 0, 0x01);        //RG_PXP_CMN_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CMN_EN, 18, 16, 0x04);      //RG_PXP_CMN_VREFSEL
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CMN_EN, 13, 8, 0x01);       //RG_PXP_CMN_MPXSELTOP_DC
  

    //TX_CKIN_SEL
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_acjtag_en, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_acjtag_en, 16, 16, 0x01); //rg_force_da_pxp_tx_ckin_sel


switch(tx_bit_sel)
	{

	case Async_GPON: //DS_2.48832G  /  US_1.24416G

	   //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_divisor
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 19, 16, 0x02); //rg_force_da_pxp_tx_ckin_divisor


	   //TX bus width setting
	   //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8, 8, 0x01);  //rg_force_sel_da_pxp_tx_rate_ctrl 
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1, 0, 0x00);  //rg_force_da_pxp_tx_rate_ctrl
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_xpon_tx_rate_ctrl, 1, 0, 0x00); //rg_pon_tx_rate_ctrl 
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 6, 0, 0x4);  //rg_tx_dly_data_ftune
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_GPON_done(Async_GPON) ===\n");
#endif
			
			break; 


    case Async_XEPON: //DS_10.3125G  /  US_1.25G	

	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	   	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 19, 16, 0x01); //rg_force_da_pxp_tx_ckin_divisor


	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	   	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8, 8, 0x01);  //rg_force_sel_da_pxp_tx_rate_ctrl 
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1, 0, 0x01);  //rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_xpon_tx_rate_ctrl, 1, 0, 0x01); //rg_pon_tx_rate_ctrl 
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 6, 0, 0x6);  //rg_tx_dly_data_ftune
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 14, 8, 0x2);  //rg_tx_dly_data_ftune

	   
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_XEPON_done(Async_XEPON) ===\n");
#endif
			
			break; 

	
	case Sync_XEPON:    //DS_10.3125G  /  US_10.3125G
	case Sync_XGSPON_1: //DS_9.95328G  /  US_9.95328G 
	case Sync_XGSPON_2: //DS_9.95328G  /  US_9.95328G 	
	
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 19, 16, 0x05); //rg_force_da_pxp_tx_ckin_divisor
	
	
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b	
	   	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8, 8, 0x01);  //rg_force_sel_da_pxp_tx_rate_ctrl 
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1, 0, 0x02);  //rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_xpon_tx_rate_ctrl, 1, 0, 0x02); //rg_pon_tx_rate_ctrl 	
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 6, 0, 0x30); //rg_tx_dly_data_ftune
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 14, 8, 0xD); //rg_tx_dly_data_ftune

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_XEPON_done(Sync_XEPON) ===\n");
 #endif
		 
         break; 


    case Async_XGPON_1: //DS_9.95328G  /  US_2.48832G	
    case Async_XGPON_2: //DS_9.95328G  /  US_2.48832G
    case Sync_GPON: //DS_2.48832G  /  US_2.48832G  	
    
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 19, 16, 0x03); //rg_force_da_pxp_tx_ckin_divisor
	
	
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8, 8, 0x01);  //rg_force_sel_da_pxp_tx_rate_ctrl 
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1, 0, 0x00);  //rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_xpon_tx_rate_ctrl, 1, 0, 0x00); //rg_pon_tx_rate_ctrl 
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 6, 0, 0x8); //rg_tx_dly_data_ftune
		
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_XGPON_done(Async_XGPON) ===\n");
#endif
			 
			break; 	

	case Sync_EPON_1: //DS_1.25G      /  US_1.25G 
	case Sync_EPON_2: //DS_1.25G      /  US_1.25G   
	  
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_term_sel, 19, 16, 0x02); //rg_force_da_pxp_tx_ckin_divisor  
	    
	  
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8, 8, 0x01);  //rg_force_sel_da_pxp_tx_rate_ctrl 
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1, 0, 0x01);  //rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_xpon_tx_rate_ctrl, 1, 0, 0x01); //rg_pon_tx_rate_ctrl 
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 6, 0, 0x6); //rg_tx_dly_data_ftune
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_TX_DLY_CTRL, 14, 8, 0x2); //rg_tx_dly_data_ftune

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_EPON_done(Sync_EPON) ===\n");
 #endif
		 
         break; 	 
		 
	     default:
		    break;	
	}	


}



void XPON_RX(uint rx_bit_sel)
{

    //RX_Path_Init.
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_REV_0, 15, 0, 0x1030);  //RG_PXP_RX_REV_0
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_REV_0, 31, 16, 0xCCCB); //RG_PXP_RX_REV_1  
	

    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_PEAKING_CTRL_MSB, 24, 24, 0x01); //RG_PXP_RX_DAC_D0_BYPASS_AEQ
  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_D1_BYPASS_AEQ, 0, 0, 0x01);     //RG_PXP_RX_DAC_D1_BYPASS_AEQ
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_D1_BYPASS_AEQ, 8, 8, 0x01);     //RG_PXP_RX_DAC_E0_BYPASS_AEQ
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_D1_BYPASS_AEQ, 16, 16, 0x01);   //RG_PXP_RX_DAC_E1_BYPASS_AEQ
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_D1_BYPASS_AEQ, 24, 24, 0x01);   //RG_PXP_RX_DAC_EYE_BYPASS_AEQ	

    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_AEQ_CFORCE, 19, 8, 0x200); //RG_PXP_AEQ_OFORCE
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_OSCAL_WATCH_WNDW, 17, 8, 0x3FF); //RG_PXP_RX_OSCAL_FORCE

  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PD_PICAL_CKD8_INV, 8, 0, 0x00); //RG_PXP_CDR_PD_EDGE_DIS
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PD_PICAL_CKD8_INV, 0, 0, 0x00); //RG_PXP_CDR_PD_PICAL_CKD8_INV

  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_bypass, 24, 24, 0x01); //rg_force_sel_da_pxp_aeq_ckon
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_bypass, 16, 16, 0x00); //rg_force_da_pxp_aeq_ckon

  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_rstb, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_injck_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_rstb, 16, 16, 0x01); //rg_force_da_pxp_cdr_injck_sel

  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 28, 24, 0x00); //RG_PXP_RX_DAC_MON
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 9, 8, 0x01); //RG_PXP_RX_PHYCK_SEL


	//FEOS_Modify_U22_20230807
    //For_U22_Pon
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_12, 0, 0, 0x01); //rg_dig_reserve_12_FEEOS_0
	
	//For_T22_Pon
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_47, 28, 28, 0x01); //rg_dig_reserve_47_FEEOS_28
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_47, 29, 29, 0x01); //rg_dig_reserve_47_FEEOS_29
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_47, 30, 30, 0x01); //rg_dig_reserve_47_FEEOS_30
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_47, 31, 31, 0x01); //rg_dig_reserve_47_FEEOS_31


  switch(rx_bit_sel)
	{
	case 10: // 10G_ mode 
	case 9: // 9G_ mode

	//case Async_XEPON: //DS_10.3125G  /  US_1.25G	
	//case Sync_XEPON:    //DS_10.3125G  /  US_10.3125G
	//case Sync_XGSPON_1: //DS_9.95328G  /  US_9.95328G 
	//case Sync_XGSPON_2: //DS_9.95328G  /  US_9.95328G 


    //10p3125G
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 2, 2, 0x01); //RG_PXP_CDR_PR_XFICK_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x100); //rg_dig_reserve_0
  

    //RX PHYCK SEL
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 24, 24, 0x00); //RG_PXP_RX_PHY_CK_SEL_FORCE 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 16, 16, 0x00); //RG_PXP_RX_PHY_CK_SEL

    //RX PHYCK  	
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 7, 0, 0x00);   //RG_PXP_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 16, 16, 0x00); //RG_PXP_RX_PHYCK_RSTB

  

    //RX OSR setting
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x00); //RG_PXP_CDR_LPF_RATIO

    //RX bus width setting
  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 8, 8, 0x00); //RG_PXP_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 0, 0, 0x01); //RG_PXP_RX_BUSBIT_SEL 


    //OSR_Sel
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 17, 16, 0x00); //rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x02);       //rg_xpon_rx_rate_ctrl
	
#if LAB_PRINT_PON
	 printk("=== XPON_RX_10G_done ===\n");
#endif
	 
     break; 



  case 2: // 2G_ mode 	
  //case Sync_GPON: //DS_2.48832G  /  US_2.48832G  	

    //2p48832G
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 2, 2, 0x00); //RG_PXP_CDR_PR_XFICK_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x300); //rg_dig_reserve_0

    //RX PHYCK SEL
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 24, 24, 0x00); //RG_PXP_RX_PHY_CK_SEL_FORCE 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 16, 16, 0x00); //RG_PXP_RX_PHY_CK_SEL

    //RX PHYCK
  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 7, 0, 0x00);   //RG_PXP_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 16, 16, 0x00); //RG_PXP_RX_PHYCK_RSTB

    //RX OSR setting
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x02); //RG_PXP_CDR_LPF_RATIO

    //RX bus width setting
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 8, 8, 0x00); //RG_PXP_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 0, 0, 0x01); //RG_PXP_RX_BUSBIT_SEL 

    //OSR_Sel
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 17, 16, 0x02); //rg_force_da_pxp_rx_osr_sel
	//IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x02);       //rg_xpon_rx_rate_ctrl //julia_20230303 //for GPON_cal
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x00);       //rg_xpon_rx_rate_ctrl


#if LAB_PRINT_PON
     printk("=== XPON_RX_2G_done ===\n");
#endif
     break; 


	 case 21: // 2G_ mode	   
	 //case Async_GPON: //DS_2.48832G  /  US_1.24416G
   
	   //2p48832G
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 2, 2, 0x00); //RG_PXP_CDR_PR_XFICK_EN
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x300); //rg_dig_reserve_0
   
	   //RX PHYCK SEL
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 24, 24, 0x00); //RG_PXP_RX_PHY_CK_SEL_FORCE 
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 16, 16, 0x00); //RG_PXP_RX_PHY_CK_SEL
   
	   //RX PHYCK
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 7, 0, 0x00);	 //RG_PXP_RX_PHYCK_DIV
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 16, 16, 0x00); //RG_PXP_RX_PHYCK_RSTB
   
	   //RX OSR setting
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x02); //RG_PXP_CDR_LPF_RATIO
   
	   //RX bus width setting
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 8, 8, 0x01); //RG_PXP_RX_BUSBIT_SEL_FORCE
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 0, 0, 0x01); //RG_PXP_RX_BUSBIT_SEL 
   
	   //OSR_Sel
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_osr_sel
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 17, 16, 0x02); //rg_force_da_pxp_rx_osr_sel
	   //IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x02);	   //rg_xpon_rx_rate_ctrl //julia_20230303 //for GPON_cal
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x00);       //rg_xpon_rx_rate_ctrl   
   
#if LAB_PRINT_PON
		printk("=== XPON_RX_2G_done ===\n");
#endif
		break; 



   case 1: // 1G_ mode	
   	//case Sync_EPON_1: //DS_1.25G      /  US_1.25G 
	//case Sync_EPON_2: //DS_1.25G      /  US_1.25G  

    //1p25G
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 2, 2, 0x01); //RG_PXP_CDR_PR_XFICK_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x300); //rg_dig_reserve_0

    //RX PHYCK SEL
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 24, 24, 0x01); //RG_PXP_RX_PHY_CK_SEL_FORCE 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 16, 16, 0x00); //RG_PXP_RX_PHY_CK_SEL

    //RX PHYCK
  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 7, 0, 0x29);   //RG_PXP_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 16, 16, 0x01); //RG_PXP_RX_PHYCK_RSTB

    //RX OSR setting  
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x03); //RG_PXP_CDR_LPF_RATIO

    //RX bus width setting
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 8, 8, 0x00); //RG_PXP_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_BUSBIT_SEL, 0, 0, 0x00); //RG_PXP_RX_BUSBIT_SEL 

    //OSR_Sel
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_speed, 17, 16, 0x03); //rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x00);       //rg_xpon_rx_rate_ctrl


#if LAB_PRINT_PON
     printk("=== XPON_RX_1G_done ===\n");
#endif

	 break; 	

	 default:
		break;	
	}

}


void XPON_ANA(uint rate_sel)
{

 //volatile unsigned int bit3 = 0x8;
 

    //JCPLL_LDO
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SPARE_H, 15, 8, 0x20); //RG_PXP_JCPLL_SPARE_L
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SPARE_H, 7, 0, 0xFF);  //RG_PXP_JCPLL_SPARE_H

    //JCPLL_RSTB
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 2, 0, 0x04); //RG_PXP_JCPLL_RST_DLY
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 8, 8, 0x01); //RG_PXP_JCPLL_PLL_RSTB  

    //JCPLL_SDM
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 16, 16, 0x00); //RG_PXP_JCPLL_SDM_DI_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 25, 24, 0x00); //RG_PXP_JCPLL_SDM_DI_LS
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_IFM, 0, 0, 0x00);   //RG_PXP_JCPLL_SDM_IFM
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_IFM, 9, 8, 0x00);   //RG_PXP_JCPLL_SDM_MODE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_IFM, 17, 16, 0x03); //RG_PXP_JCPLL_SDM_ORD
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_IFM, 24, 24, 0x00); //RG_PXP_JCPLL_SDM_OUT
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_HREN, 0, 0, 0x00);  //RG_PXP_JCPLL_SDM_HREN
 

    //JCPLL_SSC
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SSC_DELTA, 15, 0, 0x00);  //RG_PXP_JCPLL_SSC_DELTA
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SSC_DELTA, 31, 16, 0x00); //RG_PXP_JCPLL_SSC_PERIOD
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SSC_TRI_EN, 0, 0, 0x00);  //RG_PXP_JCPLL_SSC_TRI_EN
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SSC_TRI_EN, 23, 8, 0x00); //RG_PXP_JCPLL_SSC_DELTA1
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 17, 17, 0x00);//RG_PXP_JCPLL_SSC_PHASE_INI
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 10, 8, 0x00); //RG_PXP_JCPLL_VCO_VCOVAR_BIAS_L
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 16, 16, 0x00);//RG_PXP_JCPLL_SSC_EN
 

    //JCPLL_LPF
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_IB_EXT_EN, 21, 16, 0x18); //RG_PXP_JCPLL_CHP_IBIAS
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_IB_EXT_EN, 29, 24, 0x00); //RG_PXP_JCPLL_CHP_IOFST
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_IB_EXT_EN, 8, 8, 0x00);   //RG_PXP_JCPLL_LPF_SHCK_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BR, 28, 24, 0x00);    //RG_PXP_JCPLL_LPF_BWR
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BR, 20, 16, 0x10);    //RG_PXP_JCPLL_LPF_BP
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BR, 12, 8, 0x1F);     //RG_PXP_JCPLL_LPF_BC 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BR, 4, 0, 0x0A);      //RG_PXP_JCPLL_LPF_BR
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BWC, 4, 0, 0x00);     //RG_PXP_JCPLL_LPF_BWC
 

    //JCPLL_VCO
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCODIV, 9, 8, 0x01);      //RG_PXP_JCPLL_VCO_CFIX
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCODIV, 26, 24, 0x04);    //RG_PXP_JCPLL_VCO_SCAPWR
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCODIV, 16, 16, 0x01);    //RG_PXP_JCPLL_VCO_HALFLSB_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 5, 3, 0x03);  //RG_PXP_JCPLL_VCO_VCOVAR_BIAS_H
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 10, 8, 0x00); //RG_PXP_JCPLL_VCO_VCOVAR_BIAS_L
	
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 2, 0, 0x05);  //RG_PXP_JCPLL_VCO_TCLVAR
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCO_TCLVAR, 2, 0, 0x03);  //RG_PXP_JCPLL_VCO_TCLVAR , kevin suggest for UMC
	

    //JCPLL_DIV
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_MMD_PREDIV_MODE, 1, 0, 0x00);   //RG_PXP_JCPLL_MMD_PREDIV_MODE
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_MMD_PREDIV_MODE, 24, 24, 0x00); //RG_PXP_JCPLL_POSTDIV_D5
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_VCODIV, 1, 0, 0x00); //RG_PXP_JCPLL_VCODIV
 

    //JCPLL_KBand
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_KBAND_KFC, 1, 0, 0x00);    //RG_PXP_JCPLL_KBAND_KFC
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_KBAND_KFC, 9, 8, 0x03);    //RG_PXP_JCPLL_KBAND_KF
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_KBAND_KFC, 17, 16, 0x00);  //RG_PXP_JCPLL_KBAND_KS
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BWC, 26, 24, 0x02);    //RG_PXP_JCPLL_KBAND_DIV
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BWC, 23, 16, 0xE4);    //RG_PXP_JCPLL_KBAND_CODE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_LPF_BWC, 8, 8, 0x00);      //RG_PXP_JCPLL_KBAND_OPTION
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 8, 8, 0x00); //RG_PXP_JCPLL_VCO_KBAND_MEAS_EN

 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_scan_mode, 8, 8, 0x01); //rg_force_sel_da_pxp_jcpll_kband_load_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_scan_mode, 0, 0, 0x00); //rg_force_da_pxp_jcpll_kband_load_en


    //JCPLL_TCL
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SPARE_H, 20, 16, 0x10);    //RG_PXP_JCPLL_TCL_KBAND_VREF
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_HREN, 18, 16, 0x01);   //RG_PXP_JCPLL_TCL_AMP_GAIN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_HREN, 28, 24, 0x05);   //RG_PXP_JCPLL_TCL_AMP_VREF
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_SDM_HREN, 8, 8, 0x01);     //RG_PXP_JCPLL_TCL_AMP_EN
	
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_TCL_CMP_EN, 26, 24, 0x00); //RG_PXP_JCPLL_TCL_LPF_BW 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_TCL_CMP_EN, 26, 24, 0x01); //RG_PXP_JCPLL_TCL_LPF_BW , kevin suggest for UMC
	
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_TCL_CMP_EN, 16, 16, 0x01); //RG_PXP_JCPLL_TCL_LPF_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_TCL_CMP_EN, 10, 8, 0x00);  //RG_PXP_JCPLL_TCL_CMP_VTH
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_TCL_CMP_EN, 0, 0, 0x01);   //RG_PXP_JCPLL_TCL_CMP_EN 


    //XPON, TDC  
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_PLL_MONCLK_SEL, 24, 24, 0x01); //RG_PXP_TDC_AUTOEN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TDC_TXCK_SEL, 16, 16, 0x00);   //RG_PXP_TDC_FT_CK_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TDC_TXCK_SEL, 24, 24, 0x00);   //RG_PXP_TDC_MON_CK_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TDC_SYNC_CK_SEL, 0, 0, 0x00);  //RG_PXP_TDC_SYNC_CK_SEL 


    //TXPLL_bring_up
    //TXPLL_VCO_LDO_Out
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_PERIOD, 25, 24, 0x01); //RG_PXP_TXPLL_LDO_VCO_OUT
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_PERIOD, 17, 16, 0x01); //RG_PXP_TXPLL_LDO_OUT
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VTP_EN, 25, 24, 0x00);     //RG_PXP_TXPLL_LDO_VCO_VTP
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VTP_EN, 16, 16, 0x00);     //RG_PXP_TXPLL_LDO_VCO_VTP_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VTP_EN, 10, 8, 0x00);      //RG_PXP_TXPLL_VTP
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VTP_EN, 0, 0, 0x01);       //RG_PXP_TXPLL_VTP_EN  


    //TXPLL_RSTB
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 24, 24, 0x01); //RG_PXP_TXPLL_PLL_RSTB
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 18, 16, 0x04); //RG_PXP_TXPLL_RST_DLY 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 0, 0, 0x01);   //RG_PXP_TXPLL_REFIN_INTERNAL
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 9, 8, 0x00);   //RG_PXP_TXPLL_REFIN_DIV
	

    //TXPLL_SDM
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_DI_EN, 25, 24, 0x00); //RG_PXP_TXPLL_SDM_MODE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_DI_EN, 16, 16, 0x01); //RG_PXP_TXPLL_SDM_IFM
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_DI_EN, 9, 8, 0x00);   //RG_PXP_TXPLL_SDM_DI_LS
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_DI_EN, 0, 0, 0x00);   //RG_PXP_TXPLL_SDM_DI_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_ORD, 16, 16, 0x01);   //RG_PXP_TXPLL_SDM_HREN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_ORD, 8, 8, 0x00);     //RG_PXP_TXPLL_SDM_OUT
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_ORD, 1, 0, 0x03);     //RG_PXP_TXPLL_SDM_ORD  
	

    //TXPLL_SSC
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_DELTA1, 31, 16, 0x00); //RG_PXP_TXPLL_SSC_DELTA
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_DELTA1, 15, 0, 0x00);  //RG_PXP_TXPLL_SSC_DELTA1
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_EN, 16, 16, 0x00);     //RG_PXP_TXPLL_SSC_TRI_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_EN, 8, 8, 0x00);       //RG_PXP_TXPLL_SSC_PHASE_INI
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_EN, 0, 0, 0x00);       //RG_PXP_TXPLL_SSC_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SSC_PERIOD, 15, 0, 0x00);  //RG_PXP_TXPLL_SSC_PERIOD 
	

   //TXPLL_LPF
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 28, 24, 0x1F);    //RG_PXP_TXPLL_LPF_BC
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 20, 16, 0x05);    //RG_PXP_TXPLL_LPF_BR
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 13, 8, 0x00);     //RG_PXP_TXPLL_CHP_IOFST
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 5, 0, 0x0C);       //RG_PXP_TXPLL_CHP_IBIAS
   //IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 5, 0, 0x06);       //RG_PXP_TXPLL_CHP_IBIAS , hw_test
   //IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_CHP_IBIAS, 5, 0, 0x1F);       //RG_PXP_TXPLL_CHP_IBIAS , hw_test

   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_LPF_BP, 20, 16, 0x18);        //RG_PXP_TXPLL_LPF_BWC
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_LPF_BP, 12, 8, 0x0B);        //RG_PXP_TXPLL_LPF_BWR
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_LPF_BP, 4, 0, 0x02);         //RG_PXP_TXPLL_LPF_BP
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 25, 25, 0x00); //RG_PXP_TXPLL_LPF_SHCK_EN

 

 //TXPLL_VCO
 if (rate_sel == 10)
 {    
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_LPF_EN, 25, 24, 0x01); //RG_PXP_TXPLL_VCO_CFIX
 }
 else
 {	
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_LPF_EN, 25, 24, 0x03); //RG_PXP_TXPLL_VCO_CFIX
 }

 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN, 0, 0, 0x01);   //RG_PXP_TXPLL_VCO_HALFLSB_EN 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN, 10, 8, 0x07);  //RG_PXP_TXPLL_VCO_SCAPWR
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN, 26, 24, 0x04); //RG_PXP_TXPLL_VCO_VCOVAR_BIAS_H
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN, 29, 27, 0x00); //RG_PXP_TXPLL_VCO_VCOVAR_BIAS_L
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_VCO_HALFLSB_EN, 18, 16, 0x04); //RG_PXP_TXPLL_VCO_TCLVAR

    //TXPLL_Kband
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_CODE, 25, 24, 0x03); //RG_PXP_TXPLL_KBAND_KF
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_CODE, 17, 16, 0x00); //RG_PXP_TXPLL_KBAND_KFC
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_KS, 1, 0, 0x01); //RG_PXP_TXPLL_KBAND_KS

 if (rate_sel == 9)
 {   
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_CODE, 10, 8, 0x05); //RG_PXP_TXPLL_KBAND_DIV
 }
 else
 {  
    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_CODE, 10, 8, 0x04); //RG_PXP_TXPLL_KBAND_DIV
 }

 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_CODE, 7, 0, 0xE4);             //RG_PXP_TXPLL_KBAND_CODE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_LPF_BP, 24, 24, 0x00);               //RG_PXP_TXPLL_KBAND_OPTION
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_KBAND_VREF, 24, 24, 0x00);       //RG_PXP_TXPLL_VCO_KBAND_MEAS_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_kband_load_en, 8, 8, 0x01); //rg_force_sel_da_pxp_txpll_kband_load_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_kband_load_en, 0, 0, 0x00); //rg_force_da_pxp_txpll_kband_load_en


    //TXPLL_DIV
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_KS, 17, 16, 0x00);   //RG_PXP_TXPLL_MMD_PREDIV_MODE
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_KBAND_KS, 8, 8, 0x01);     //RG_PXP_TXPLL_POSTDIV_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_LPF_EN, 17, 16, 0x00); //RG_PXP_TXPLL_VCODIV


    //TXPLL_TCL
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_KBAND_VREF, 4, 0, 0x0F); //RG_PXP_TXPLL_TCL_KBAND_VREF
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_AMP_GAIN, 2, 0, 0x03);   //RG_PXP_TXPLL_TCL_AMP_GAIN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_AMP_GAIN, 12, 8, 0x0B);  //RG_PXP_TXPLL_TCL_AMP_VREF 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_LPF_EN, 10, 8, 0x00);    //RG_PXP_TXPLL_TCL_LPF_BW 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_LPF_EN, 0, 0, 0x01);     //RG_PXP_TXPLL_TCL_LPF_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_SDM_ORD, 24, 24, 0x01);      //RG_PXP_TXPLL_TCL_AMP_EN
 

    //XPON, TX
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_SER_LOADSEL, 1, 0, 0x02); //RG_PXP_TX_SER_LOADSEL 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_TXLBRX_EN, 0, 0, 0x00);   //RG_PXP_TX_TXLBRX_EN
 

    //TX_TERMCAL
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_TXLBRX_EN, 18, 16, 0x02); //RG_PXP_TX_TERMCAL_VREF_H
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TX_TXLBRX_EN, 26, 24, 0x02); //RG_PXP_TX_TERMCAL_VREF_L 

    //TX_FIR
 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_fir_cn1
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 8, 8, 0x01);   //rg_force_sel_da_pxp_tx_fir_c0b
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 8, 8, 0x01);    //rg_force_sel_da_pxp_tx_fir_c1
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 24, 24, 0x01);  //rg_force_sel_da_pxp_tx_fir_c2

	if ((rate_sel == 10) || (rate_sel == 9)) //IL_3inch
	{ 
		IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, 0x00); //rg_force_da_pxp_tx_fir_cn1
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, 0x0E);   //rg_force_da_pxp_tx_fir_c0b
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, 0x04);    //rg_force_da_pxp_tx_fir_c1
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, 0x00);  //rg_force_da_pxp_tx_fir_c2
	}
	else
	{
	 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, 0x00); //rg_force_da_pxp_tx_fir_cn1
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, 0x00);   //rg_force_da_pxp_tx_fir_c0b
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, 0x00);    //rg_force_da_pxp_tx_fir_c1
	    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, 0x00);  //rg_force_da_pxp_tx_fir_c2
	}



    //XPON_RX
 	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_EQ_HZEN, 24, 24, 0x01);     //RG_PXP_RX_FE_VB_EQ3_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_EQ_HZEN, 16, 16, 0x01);     //RG_PXP_RX_FE_VB_EQ2_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_EQ_HZEN, 8, 8, 0x01);       //RG_PXP_RX_FE_VB_EQ1_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_VCM_GEN_PWDB, 10, 8, 0x04); //RG_PXP_RX_FE_VCM_SEL
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_EQ_HZEN, 0, 0, 0x00);       //RG_PXP_RX_FE_EQ_HZEN 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_FE_VCM_GEN_PWDB, 0, 0, 0x01);  //RG_PXP_RX_FE_VCM_GEN_PWDB

	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 26, 8, 0x08000);    //RG_PXP_CDR_LPF_TOP_LIM , follow EN7580
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_BOT_LIM, 18, 0, 0x78000);  //RG_XPON_CDR_LPF_BOT_LIM , follow EN7580	
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_CKREF_DIV, 16, 16, 0x00); //RG_PXP_CDR_PR_RSTB_BYPASS
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_PHYCK_DIV, 24, 24, 0x01); //RG_PXP_RX_TDC_CK_SEL
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_RANGE, 9, 8, 0x00); //RG_PXP_RX_DAC_RANGE_EYE
 

#if LAB_PRINT_PON
  printk("=== XPON_ANA_done ===\n");
#endif

}



void JCPLL_on(void)
{

  // ** JCPLL_force_on ** //
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_jcpll_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 16, 16, 0x01); //rg_force_da_pxp_jcpll_en

  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 8, 8, 0x01); //rg_force_sel_da_pxp_jcpll_ckout_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_ckout_en, 0, 0, 0x01); //rg_force_da_pxp_jcpll_ckout_en

    //JCPLL_Retoggle
  	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 8, 8, 0x00); //RG_PXP_JCPLL_PLL_RSTB
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_RST_DLY, 8, 8, 0x01); //RG_PXP_JCPLL_PLL_RSTB
  

  udelay(500);

#if LAB_PRINT_PON
   printk("=== XPON_JCPLL_On_done ===\n");
#endif

}


void TXPLL_on(void)
{

	// LCPLL_force_on 
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_txpll_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x01); //rg_force_da_pxp_txpll_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_PWCTL_SETTING_0, 24, 24, 0x01); //rg_sw_lcpll_en
	
    udelay(6);

    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 0, 0, 0x01); //RG_PXP_JCPLL_FREQ_MEAS_EN 

    //Re-Setting TXPLL for Kband
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 8, 8, 0x01); //rg_force_sel_da_pxp_txpll_ckout_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 0, 0, 0x01); //rg_force_da_pxp_txpll_ckout_en 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_KBAND_VREF, 16, 16, 0x01);  //RG_PXP_TXPLL_FREQ_MEAS_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_TCL_KBAND_VREF, 8, 8, 0x00);    //RG_PXP_TXPLL_VREF_SEL

    //TXPLL_Out
  	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_PHY_CK1_EN, 8, 8, 0x01);     //RG_PXP_TXPLL_PHY_CK2_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_PHY_CK1_EN, 0, 0, 0x01);     //RG_PXP_TXPLL_PHY_CK1_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 0, 0, 0x00);   //RG_PXP_JCPLL_FREQ_MEAS_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 24, 24, 0x00); //RG_PXP_TXPLL_IB_EXT_EN
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_JCPLL_FREQ_MEAS_EN, 16, 16, 0x00); //RG_PXP_750M_SYS_CK_EN  


    //TXPLL_Retoggle
  	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 24, 24, 0x00); //RG_PXP_TXPLL_PLL_RSTB
	//IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_TXPLL_REFIN_INTERNAL, 24, 24, 0x01); //RG_PXP_TXPLL_PLL_RSTB  

udelay(500);

#if LAB_PRINT_PON
   printk("=== XPON_TXPLL_On_done ===\n");
#endif

}

void XPON_DIG_fm_on(int fm_bit_sel)
{   


switch(fm_bit_sel)
	{

case fm_Sync_XEPON: //fm_DS(RX)_10.3125G  /  US_10.3125G
case fm_Async_XEPON: //fm_10p3125G_1p25G  


    //enable all freq meter
    //watch TX/RX/JCPLL
    //phya_tx_ck /div8 frequency meter setting     
	/*

    _u16RW_pon_csr_pma__rg_pll_unlock_cyclecnt = 0x100;
	_u16RW_pon_csr_pma__rg_pll_lock_cyclecnt = 0x100;

	_u16RW_pon_csr_pma__rg_pll_lock_target_end = 0x9F;
	_u16RW_pon_csr_pma__rg_pll_lock_target_beg = 0x9E;

	_fldRW_pon_csr_pma__rg_pll_unlockth = 0x8;
	_fldRW_pon_csr_pma__rg_pll_lock_lockth = 0x3;
	_fldRW_pon_csr_pma__rg_pll_freqlock_det_en = 0x7;

	_u16RW_pon_csr_pma__rg_pll_unlock_target_end = 0x9F;
	_u16RW_pon_csr_pma__rg_pll_unlock_target_beg = 0x9E;		 

     //END phya_tx_ck /div8 frequency meter setting
*/

     //-------------------------------------------
     //CDR_FB_CK frequency meter setting    

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x9F43);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0x9E7A);   //rg_lock_cyclecnt

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x9F43);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0x9E7A);   //rg_unlock_target_beg
		 
     break;  


case fm_Sync_XGSPON_1: //fm_9p95328G_9p95328G 
case fm_Sync_XGSPON_2: //fm_9p95328G_9p95328G 
case fm_Async_XGPON: //fm_9p95328G_2p48832G 
case fm_Sync_GPON:   //fm_2p48832G_2p48832G 
case fm_Async_GPON:  //fm_2p48832G_1p24416G 


/*
     
     //phya_tx_ck /div8 frequency meter setting

	 _u16RW_pon_csr_pma__rg_pll_unlock_cyclecnt = 0x100;
	 _u16RW_pon_csr_pma__rg_pll_lock_cyclecnt = 0x100;

	 _u16RW_pon_csr_pma__rg_pll_lock_target_end = 0xa5;
	 _u16RW_pon_csr_pma__rg_pll_lock_target_beg = 0xa4;

	 _fldRW_pon_csr_pma__rg_pll_unlockth = 0x8;
	 _fldRW_pon_csr_pma__rg_pll_lock_lockth = 0x3;
	 _fldRW_pon_csr_pma__rg_pll_freqlock_det_en = 0x7;

	 _u16RW_pon_csr_pma__rg_pll_unlock_target_end = 0xa5;
	 _u16RW_pon_csr_pma__rg_pll_unlock_target_beg = 0xa4;		

     //END phya_tx_ck /div8 frequency meter setting
*/
     //-------------------------------------------
     //CDR_FB_CK frequency meter setting 

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0xA4FF);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0xA436);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0xA4FF);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0xA436);   //rg_unlock_target_beg
	 
     break;
	 

case fm_Sync_EPON_1: //fm_1p25G_1p25G 	
case fm_Sync_EPON_2:


/*
     //phya_tx_ck /div8 frequency meter setting

	 _u16RW_pon_csr_pma__rg_pll_unlock_cyclecnt = 0x30;
	 _u16RW_pon_csr_pma__rg_pll_lock_cyclecnt = 0x30;

	 _u16RW_pon_csr_pma__rg_pll_lock_target_end = 0x9A;
	 _u16RW_pon_csr_pma__rg_pll_lock_target_beg = 0x99;

	 _fldRW_pon_csr_pma__rg_pll_unlockth = 0x8;
	 _fldRW_pon_csr_pma__rg_pll_lock_lockth = 0x3;
	 _fldRW_pon_csr_pma__rg_pll_freqlock_det_en = 0x7;

	 _u16RW_pon_csr_pma__rg_pll_unlock_target_end = 0x9A;
	 _u16RW_pon_csr_pma__rg_pll_unlock_target_beg = 0x99;
	
     //END phya_tx_ck /div8 frequency meter setting
*/
     //-------------------------------------------
     //CDR_FB_CK frequency meter setting
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0xA43A);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0xA371);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0xA43A);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0xA371);   //rg_unlock_target_beg 
		
     break; 


default:
	   break;  

	} 	  


#if LAB_PRINT_PON
   printk("=== XPON_FM_On_done ===\n");
#endif

}


int XPON_PrCal_WK(int bit_sel)	
{
  uint PrCal_Serach = 0 , RO_FL_Out = 0 , FL_Out_target = 0x9EDF ;
  uint pr_idac = 0  , RO_pr_idac = 0;
  int  cdr_pr_idac_tmp = 0, RO_state_freqdet = 0, turn_pr_idac_bit_position = 0;


  switch(bit_sel)
  {
	
	case Async_XEPON:   //DS_10.3125G  /  US_1.25G
    case Sync_XEPON:    //DS_10.3125G  /  US_10.3125G

	   FL_Out_target = 0x9EDF;
	
	break;
	
	case Sync_XGSPON_1: //DS_9.95328G  /  US_9.95328G 
	case Sync_XGSPON_2: //DS_9.95328G  /  US_9.95328G 
	case Async_XGPON_1: //DS_9.95328G  /  US_2.48832G	
    case Async_XGPON_2: //DS_9.95328G  /  US_2.48832G
    case Async_GPON: //DS_2.48832G  /  US_1.24416G
	case Sync_GPON: //DS_2.48832G  /  US_2.48832G  

	   FL_Out_target = 0xA49A;

	break;  


	case Sync_EPON_1: //DS_1.25G      /  US_1.25G 
	case Sync_EPON_2: //DS_1.25G      /  US_1.25G 

	   FL_Out_target = 0xA3D5;

	break;  	
	
		     default:
		    break;	
  }	


      IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_INJ_MODE, 24, 24, 0x1);  //RG_PXP_CDR_PR_INJ_FORCE_OFF

	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 24, 24, 0x1);  //rg_force_sel_da_pxp_cdr_pr_lpf_r_en
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 16, 16, 0x1);  //rg_force_da_pxp_cdr_pr_lpf_r_en	  
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 8, 8, 0x1);	//rg_force_sel_da_pxp_cdr_pr_lpf_c_en
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 0, 0, 0x1);	//rg_force_da_pxp_cdr_pr_lpf_c_en

      IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 16, 16, 0x1);       //rg_force_sel_da_pxp_cdr_pr_idac
      IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 24, 24, 0x1); //rg_force_sel_da_pxp_cdr_pr_pwdb

	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16, 16, 0x0); //rg_force_da_pxp_cdr_pr_pwdb
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16, 16, 0x1); //rg_force_da_pxp_cdr_pr_pwdb  


  for (PrCal_Serach = 1; PrCal_Serach < 8 ; PrCal_Serach++)
  {

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 10, 0, (PrCal_Serach<<8));  //rg_force_da_pxp_cdr_pr_idac
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_4, 2, 0, 0x3);  //rg_freqlock_det_en	 

	 udelay(5000);
	 
	 RO_FL_Out = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RO_RX_FREQDET, 31, 16); //ro_fl_out 
	 RO_pr_idac = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 10, 0); //RO_RX_FREQDET

	 #if Pon_PR_WK
	   printk("pr_idac = 0x%x ,RO_FL_Out = 0x%x\n" ,RO_pr_idac , RO_FL_Out);
	 #endif	

	 if(RO_FL_Out > FL_Out_target)
	 {        
		 cdr_pr_idac_tmp = (PrCal_Serach<<8);

		 #if Pon_PR_WK
		   printk("cdr_pr_idac_tmp = 0x%x\n",cdr_pr_idac_tmp);		 
         #endif	
	 }
	 

  }

  for (turn_pr_idac_bit_position = 7; turn_pr_idac_bit_position > -1 ; turn_pr_idac_bit_position--)
  {
	  pr_idac = cdr_pr_idac_tmp |(0x1<<turn_pr_idac_bit_position); 

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 10, 0, pr_idac);  //rg_force_da_pxp_cdr_pr_idac
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_4, 2, 0, 0x3);  //rg_freqlock_det_en	 
	  
	  udelay(5000);	  
      
      RO_FL_Out = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RO_RX_FREQDET, 31, 16); //ro_fl_out

	   #if Pon_PR_WK
	    printk("pr_idac = 0x%x ,RO_FL_Out = 0x%x\n",pr_idac,RO_FL_Out);
	   #endif	

	  if(RO_FL_Out < FL_Out_target)
	  {
          pr_idac &= ~(0x1<<turn_pr_idac_bit_position);
		  cdr_pr_idac_tmp = pr_idac;
		   #if Pon_PR_WK
		    printk("cdr_pr_idac_tmp = 0x%x\n",cdr_pr_idac_tmp);
		   #endif	
	  }
	  else
	  {
		  cdr_pr_idac_tmp = pr_idac;
		   #if Pon_PR_WK
		    printk("cdr_pr_idac_tmp = 0x%x\n",cdr_pr_idac_tmp);
		   #endif	
	  }   
	  
  }

  	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 10, 0, cdr_pr_idac_tmp);  //rg_force_da_pxp_cdr_pr_idac
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_4, 2, 0, 0x3);  //rg_freqlock_det_en

   #if Pon_PR_WK
     printk("sel_cdr_pr_idac = 0x%x\n",cdr_pr_idac_tmp);
   #endif
  
  
  RO_state_freqdet = (int)IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RO_RX_FREQDET, 0, 0); //ro_fl_out

   #if Pon_PR_WK
     printk("RO_state_freqdet = 0x%x\n",RO_state_freqdet);
   #endif


   //Load_Band
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_INJ_MODE, 24, 24, 0x0);  //RG_PXP_CDR_PR_INJ_FORCE_OFF
     
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 24, 24, 0x1);  //rg_force_sel_da_pxp_cdr_pr_lpf_r_en
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 16, 16, 0x1);  //rg_force_da_pxp_cdr_pr_lpf_r_en
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 8, 8, 0x0);    //rg_force_sel_da_pxp_cdr_pr_lpf_c_en     
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_lpf_c_en, 0, 0, 0x0);    //rg_force_da_pxp_cdr_pr_lpf_c_en

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 16, 16, 0x0);    //rg_force_sel_da_pxp_cdr_pr_idac
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_b, 0, 0, 0x1);                      //rg_load_en
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_1, 10, 0, cdr_pr_idac_tmp);         //rg_ipath_idac
	 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16, 16, 0x0);    //rg_force_da_pxp_cdr_pr_pwdb
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16, 16, 0x1);    //rg_force_da_pxp_cdr_pr_pwdb  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 24, 24, 0x1);    //rg_force_da_pxp_cdr_pr_pwdb  

  return RO_state_freqdet;
  	

}





void XPON_TX_on(void)
{
  UINT32 read_data;

   // TX_on
   //controlled by DA_TX_CK_EN or DA_TX_HSDATA_EN
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_RST_B, 0, 0, 0x01); //tx_top_rst_b


   //3750 , CLK PATH EN 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_CLKPATH_RST_0, 8, 8, 0x01); //rg_clkpath_rstb_ck
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_CLKPATH_RST_0, 0, 0, 0x01); //rg_clkpath_rst_en

   //3550 , TX CAL RST  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_RST_B, 8, 8, 0x01); //txcalib_rst_b
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_RST_B, 0, 0, 0x01); //tx_top_rst_b 
   //IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x01); //rg_tx_bist_gen_en                                                   
                                   
   udelay(100);
 //********************************************OPTIMIZE TX IMPEDANCE by EFUSE*****************************************************//
 #if 0
#ifdef TCSUPPORT_CPU_EN7581
	  PON_PHY_PRINT(PHY_MSG_ERR,"EN7581_EFUSE: %x\n", EN7581_EFUSE);
	#if EN7581_EFUSE
		  //-------TX R50 Calibration load from EFUSE-------//
		  // if EFUSE is zero, default set TX R50 impedance to level 2 (0x2)
			  read_data = get_efuse_data(DA_XPON_TX_TERMP_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_0, 25, 24, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"DA_XPON_TX_TERMP_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_0, 25, 24, 0x2);
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set TXP Impedance Level 2 as default! \n");
			  }
			  read_data = get_efuse_data(DA_XPON_TX_TERMN_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_1, 25, 24, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"DA_XPON_TX_TERMN_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_1, 25, 24, 0x2);
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set TXN Impedance Level 2 as default! \n");
			  }
	#else
		  //-------TX R50 Calibration use default value-------//
			  PON_PHY_PRINT(PHY_MSG_ERR,"TX R50 Calibration use default value. \n");
			  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_0, 25, 24, 0x2);
			  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_1, 25, 24, 0x2);	  
	#endif
			  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_0, 16, 16, 0x1);	//force P enable, by ang_20190820
			  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_TX_CALIB_1, 16, 16, 0x1);	//force N enable, by ang_20190820	  
#endif
#endif

//*****************************************************************************************************************************//


#if LAB_PRINT_PON
   printk("=== XPON_TX_On_done ===\n");
#endif

}


void XPON_RX_preset(void)
{

   // ** RX_precondition
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_SIGDET_NOVTH, 9, 8, 0x02);        //RG_PXP_RX_SIGDET_PEAK
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_SIGDET_NOVTH, 20, 16, 0x02);      //RG_PXP_RX_SIGDET_VTH_SEL
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_DAC_RANGE, 25, 24, 0x03);         //RG_PXP_RX_SIGDET_LPF_CTRL
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 19, 19, 0x00);      //RG_PXP_CDR_PR_CAP_EN
   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_MONPR_EN, 18, 16, 0x07);      //RG_PXP_CDR_PR_BUF_IN_SR
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8, 8, 0x00); //rg_force_rx_os_rdy
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8, 8, 0x00);  //rg_disb_rx_os_rdy 
												   
   //L2R
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0, 0, 0x00); //rg_force_da_pxp_cdr_lpf_lck2data
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8, 0x00); //rg_force_sel_da_pxp_cdr_lpf_lck2data   
                                                   
   //LEQ setting
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 16, 0x04); //rg_force_da_pxp_rx_fe_peaking_ctrl
                                                 
   //keep EYE reset
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);   //rg_force_eye_reset_plu_o
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 8, 8, 0x00);    //rg_disb_eye_reset_plu_o
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x00); //rg_force_eye_top_en
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x00);  //rg_disb_eye_top_en

   //keep BLWC reset
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 0, 0, 0x00);  //rg_disb_blwc_rx_rst_b
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 0, 0, 0x00); //rg_force_blwc_rx_rst_b
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16, 16, 0x01);  //rg_disb_rx_blwc_en
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 16, 16, 0x01); //rg_force_rx_blwc_en
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_Preset_done ===\n");
#endif

}


void XPON_TDC_off(void)
{
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x01); //rg_force_sel_da_pxp_txpll_sdm_pcw

	//TDC   
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_3, 8, 8, 0x01);  //rg_lcpll_ncpo_load

	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x01);  //rg_force_sel_da_pxp_txpll_sdm_pcw_chg
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x00);  //rg_force_da_pxp_txpll_sdm_pcw_chg
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x01);  //rg_force_da_pxp_txpll_sdm_pcw_chg
  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_1, 0, 0, 0x00);  //rg_lcpll_gpon_sel
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PW_0, 0, 0, 0x00);   //rg_lcpll_tdc_dig_pwdb  
    //udelay(1000);

#if LAB_PRINT_PON
   printk("=== XPON_TDC_Off_done ===\n");
#endif

}


void XPON_RX_on(int bit_sel)
{

	int freq_lock = 0 , i = 0;

	uint read_data = 0;

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01); //rg_force_sel_da_pxp_rx_fe_gain_ctrl


		switch(bit_sel)	{			

		case Sync_XEPON:
		case Sync_XGSPON_1:
		case Sync_XGSPON_2:
		case Async_XGPON_1:
		case Async_XGPON_2:		
		
			IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x01); //rg_force_da_pxp_rx_fe_gain_ctrl	
			IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_REV_0, 15, 0, 0x1030);  //RG_PXP_RX_REV_0

		      break;
			  
			  
		case Async_GPON:
		case Sync_GPON:
		case Async_XEPON:
		case Sync_EPON_1:
		case Sync_EPON_2:		

		    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03); //rg_force_da_pxp_rx_fe_gain_ctrl	
		    IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_REV_0, 15, 0, 0x18B0);  //RG_PXP_RX_REV_0

		 		default:
		    break;	
     }		 


    // RX_on/ 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_pr_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16, 16, 0x01); //rg_force_da_pxp_cdr_pr_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01);   //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01);   //rg_force_da_pxp_cdr_pr_pieye_pwdb

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 24, 24, 0x00); //rg_force_sel_da_pxp_cdr_pr_kband_rstb 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 16, 16, 0x00); //rg_force_da_pxp_cdr_pr_kband_rstb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 8, 8, 0x01);   //rg_force_sel_da_pxp_cdr_pd_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 0, 0, 0x01);   //rg_force_da_pxp_cdr_pd_pwdb

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_rx_fe_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 0, 0, 0x01); //rg_force_da_pxp_rx_fe_pwdb

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_scan_rst_b, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_sigdet_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_scan_rst_b, 16, 16, 0x01); //rg_force_da_pxp_rx_sigdet_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_scan_rst_b, 8, 8, 0x00);   //rg_force_sel_da_pxp_rx_scan_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_scan_rst_b, 0, 0, 0x00);   //rg_force_da_pxp_rx_scan_rst_b

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_lpf_rstb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x01); //rg_force_da_pxp_cdr_lpf_rstb

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_DA_XPON_PWDB_0, 24, 24, 0x01); //rg_da_xpon_cdr_pd_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_DA_XPON_PWDB_0, 16, 16, 0x01); //rg_da_xpon_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_DA_XPON_PWDB_0, 8, 8, 0x01);   //rg_da_xpon_cdr_pr_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_DA_XPON_PWDB_0, 0, 0, 0x01);   //rg_da_xpon_rx_fe_pwdb


    //RX_SigDet_Pwdb
 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_DA_XPON_PWDB_1, 0, 0, 0x01);   //rg_da_xpon_rx_sigdet_pwdb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RX_SYS_EN_SEL_0, 1, 0, 0x01); //rg_da_rx_sys_en_sel
 
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_VREG_IBAND_VAL, 25, 24, 0x00); //RG_PXP_CDR_PR_FBKSEL
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_VREG_IBAND_VAL, 20, 16, 0x08); //RG_PXP_CDR_PR_DAC_BAND
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_VREG_IBAND_VAL, 10, 8, 0x05);  //RG_PXP_CDR_PR_VREG_CKBUF_VAL
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_VREG_IBAND_VAL, 2, 0, 0x05);   //RG_PXP_CDR_PR_VREG_IBAND_VAL

 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8, 8, 0x00);   //rg_disb_rx_pical_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16, 16, 0x00); //rg_disb_rx_pdos_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24, 24, 0x00); //rg_disb_rx_feos_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0, 0, 0x00);   //rg_disb_rx_os_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0, 0, 0x00);   //rg_disb_rx_sdcal_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16, 16, 0x00); //rg_disb_rx_blwc_en


    //Disable_AEQ
 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_en, 8, 8, 0x01);      //rg_force_sel_da_pxp_aeq_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_en, 0, 0, 0x00);      //rg_force_da_pxp_aeq_en
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_CKREF_DIV, 1, 0, 0x00);     //RG_PXP_CDR_PR_CKREF_DIV
	IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_PR_TDC_REF_SEL, 25, 24, 0x00); //RG_PXP_CDR_PR_CKREF_DIV1
 

    //RX_RSTB
 	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 1, 1, 0x01); //rg_sw_rx_rst_n
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x01); //rg_sw_ref_rst_n

    //CDR_LPF_RSTB 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x00); //rg_force_da_pxp_cdr_lpf_rstb    
    udelay(100);

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x01); //rg_force_da_pxp_cdr_lpf_rstb


	#if 1	

	if(GET_PDIDR() == 0x1)
	{
       freq_lock = XPON_PrCal_WK(bit_sel); 

	   if(freq_lock != 1 )
	   {
		  XPON_PrCal_WK(bit_sel); 

       #if LAB_PRINT_PON
	      printk("==PR_WK Re-try ==\n");
       #endif

	   }	
    }


#endif
#if LAB_PRINT_PON
   printk("=== XPON_RX_On_done ===\n");
#endif

}


void XPON_RX_L2R(void)
{

  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0, 0, 0x00); //rg_force_da_pxp_cdr_lpf_lck2data
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_lpf_lck2data  		
    udelay(100);

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_lpf_rstb  
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x00); //rg_force_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x01); //rg_force_da_pxp_cdr_lpf_rstb		
	
#if LAB_PRINT_PON
   printk("=== XPON_RX_L2R_done ===\n");
#endif

}


void XPON_RX_OSCal(void)
{

    //FBCK_Lock
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 0, 0, 0x00);  //rg_disb_fbck_lock
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 0, 0, 0x01); //rg_force_fbck_lock

    //RX_OSCal_En
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_oscal_ckon
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16, 16, 0x01); //rg_force_da_pxp_rx_oscal_ckon

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_oscal_rstb	 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 16, 16, 0x01); //rg_force_da_pxp_rx_oscal_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 8, 8, 0x01);   //rg_force_sel_da_pxp_rx_oscal_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 0, 0, 0x01);   //rg_force_da_pxp_rx_oscal_en   
    udelay(200);

    //set_normal_or_force_mode
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0, 0, 0x00);  //rg_disb_rx_os_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8, 8, 0x00);  //rg_disb_rx_os_rdy

    //disable_force_mode_signal
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x00); //rg_force_rx_os_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8, 8, 0x00); //rg_force_rx_os_rdy  

    //release_reset_Enable
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x01); //rg_force_rx_os_en   

}



void XPON_RX_pical(void)
{

    //PICAL
    //pre-condition  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 24, 24, 0x01);  //rg_disb_da_xpon_cdr_pr_pieye    
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x04);     //rg_kpgain	 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x08);     //rg_eq_en_delay  


    //reset block  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 16, 16, 0x00);	  //rg_eq_pi_cal_rst_b	 
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x00);	  //rg_force_rx_and_pical_rstb	 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_6, 8, 8, 0x00);	  //rg_disb_rx_and_pical_rstb	

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x00);	  //rg_force_ref_and_pical_rstb	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_6, 0, 0, 0x00);	  //rg_disb_ref_and_pical_rstb	

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x00);    //rg_disb_eq_pi_cal_rdy	  
  

    //enable 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);	  //rg_force_rx_or_pical_en	 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_5, 24, 24, 0x00);  //rg_disb_rx_or_pical_en	 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x00);	//rg_force_rx_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8, 8, 0x00);	//rg_disb_rx_pical_en

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x00);   //rg_force_eq_pi_cal_rdy  

    //release reset & enable
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 16, 16, 0x01);      //rg_eq_pi_cal_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x01);   //rg_force_rx_and_pical_rstb	
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x01);   //rg_force_ref_and_pical_rstb	
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x01);   //rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x01);   //rg_force_rx_pical_en    
    udelay(200);

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x00);  //rg_force_rx_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);  //rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x01);  //rg_force_eq_pi_cal_rdy

#if LAB_PRINT_PON
   printk("=== XPON_RX_Pical_done ===\n");
#endif

}


void XPON_RX_pdos(void)
{

    //PDOS
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 24, 24, 0x01);  //rg_force_sel_da_pxp_rx_pdoscal_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 16, 16, 0x01);  //rg_force_da_pxp_rx_pdoscal_en
  

    //*** pre-condition
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8, 8, 0x00);   //rg_force_rx_os_rdy
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8, 8, 0x00);    //rg_disb_rx_os_rdy
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_1, 8, 8, 0x01);      //rg_disb_da_xpon_rx_dac_d0
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_1, 16, 16, 0x01);    //rg_disb_da_xpon_rx_dac_d1
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_1, 24, 24, 0x01);    //rg_disb_da_xpon_rx_dac_e0
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 0, 0, 0x01);      //rg_disb_da_xpon_rx_dac_e1
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 8, 8, 0x01);      //rg_disb_da_xpon_rx_dac_eye
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 0, 0, 0x00);     //rg_force_blwc_rx_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 0, 0, 0x00);      //rg_disb_blwc_rx_rst_b
    

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x00); //rg_force_eyedur_init_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x00);   //rg_disb_eyedur_init_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 24, 24, 0x00);   //rg_force_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x00);    //rg_disb_eyecnt_rx_rst_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x00); //rg_force_eyedur_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x00);   //rg_disb_eyedur_en
  

    //*** setting 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_PDOS_CTRL_0, 18, 16, 0x02); //rg_sap_sel 


    //*** seq
    //reset
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 16, 16, 0x00); //rg_force_pdos_rx_rst_b 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_6, 16, 16, 0x00);  //rg_disb_pdos_rx_rst_b 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_1, 0, 0, 0x00);        //rg_pdos_rst_b  


    //disable  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x00); //rg_force_rx_pdos_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16, 16, 0x00);  //rg_disb_rx_pdos_en  


    //release reset & enable  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x00); //rg_force_rx_os_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0, 0, 0x00);  //rg_force_rx_os_en 

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 16, 16, 0x01); //rg_force_pdos_rx_rst_b 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_1, 0, 0, 0x01); //rg_pdos_rst_b 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x01); //rg_force_rx_pdos_en   
    udelay(200);
                                                 
    //disable  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x00); //rg_force_rx_pdos_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x00); //rg_force_rx_os_en 


    //release eye related
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x00); //rg_force_eyedur_init_b 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01);   //rg_disb_eyedur_init_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 24, 24, 0x00); //rg_force_eyecnt_rx_rst_b 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x01);  //rg_disb_eyecnt_rx_rst_b 

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x00); //rg_force_eyedur_en 	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01); //rg_disb_eyedur_en  


    //Disable_PDOS
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 24, 24, 0x01); //rg_force_sel_da_pxp_rx_pdoscal_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_pwdb, 16, 16, 0x00); //rg_force_da_pxp_rx_pdoscal_en 


#if LAB_PRINT_PON
   printk("=== XPON_RX_PDOS_done ===\n");
#endif

}


void XPON_RX_feos(void)
{

    //FEOS
    //*** pre-condition
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8, 8, 0x00);   //rg_force_rx_os_rdy
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8, 8, 0x00);    //rg_disb_rx_os_rdy
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 16, 16, 0x01);    //rg_disb_da_xpon_rx_fe_vos
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 0, 0, 0x00);     //rg_force_blwc_rx_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 0, 0, 0x00);      //rg_disb_blwc_rx_rst_b    

    //*** setting 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FEOS, 7, 0, 0x30); //rg_lfsel 

    //*** seq
    //reset
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 16, 16, 0x00); //rg_force_feos_rx_rst_b 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 16, 16, 0x00);  //rg_disb_feos_rx_rst_b 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 8, 8, 0x00);        //rg_feos_rst_b   
                                                  
    //disable
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x00); //rg_force_rx_feos_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24, 24, 0x00);  //rg_disb_rx_feos_en   
  

    //release reset & enable
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x01); //rg_force_rx_os_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0, 0, 0x00);  //rg_disb_rx_os_en 
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 16, 16, 0x01);             //rg_force_feos_rx_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 8, 8, 0x01);                    //rg_feos_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x01); //rg_force_rx_feos_en  
    //udelay(200);
    phy_delay1ms(1);

    //disable  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x00); //rg_force_rx_feos_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0, 0, 0x00);   //rg_force_rx_os_en  


#if LAB_PRINT_PON
   printk("=== XPON_RX_FEOS_done ===\n");
#endif

}


void XPON_RX_sdcal(void)
{

    //SIGDET_Calibration
    //*** pre-condition
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8, 8, 0x01); //rg_force_sel_da_pxp_rx_sigdet_cal_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0, 0, 0x00); //rg_force_da_pxp_rx_sigdet_cal_en

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 8, 8, 0x01); //rg_force_sel_da_pxp_rx_oscal_en	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 0, 0, 0x01); //rg_force_da_pxp_rx_oscal_en  
  
  
    //reset
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 24, 24, 0x00);    //rg_cal_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 8, 8, 0x00); //rg_force_sdcal_ref_rst_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0, 0, 0x00);  //rg_disb_rx_sdcal_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 8, 8, 0x00);                //rg_disb_sdcal_ref_rst_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0, 0, 0x00);  //rg_force_rx_sdcal_en
  

    //release reset & enable
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 24, 24, 0x01);     //rg_cal_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_8, 8, 8, 0x01);  //rg_force_sdcal_ref_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0, 0, 0x01);   //rg_force_rx_sdcal_en    

    udelay(200);

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0, 0, 0x00);  //rg_force_rx_sdcal_en


    //Disable_SDCal
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8, 8, 0x01);  //rg_force_sel_da_pxp_rx_sigdet_cal_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0, 0, 0x00);  //rg_force_da_pxp_rx_sigdet_cal_en  

  
    //RX_OSCal_Dis
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24, 24, 0x01);  //rg_force_sel_da_pxp_rx_oscal_ckon
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16, 16, 0x00);  //rg_force_da_pxp_rx_oscal_ckon

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 24, 24, 0x01);  //rg_force_sel_da_pxp_rx_oscal_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 16, 16, 0x00);  //rg_force_da_pxp_rx_oscal_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 8, 8, 0x01);    //rg_force_sel_da_pxp_rx_oscal_en	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_oscal_en, 0, 0, 0x00);    //rg_force_da_pxp_rx_oscal_en  
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_SD_Cal_done ===\n");
#endif

}




void XPON_phy_status()
{

    //RX_cal_ready
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8, 8, 0x01); //rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8, 8, 0x00);  //rg_disb_rx_os_rdy   
    udelay(1);   


#if LAB_PRINT_PON
   printk("=== XPON_Rhy_Status_done ===\n");
#endif

}

void XPON_RX_rxrdy(void)
{ 
  UINT32 read_data;
  
    //RX data path ready
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24, 24, 0x01); //rg_force_rx_rdy
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24, 0x00);  //rg_disb_rx_rdy	

    //udelay(10);

    //reset RX FIFO
  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x00); //rg_sw_rx_fifo_rst_n
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x01); //rg_sw_rx_fifo_rst_n  

	//udelay(100);

//Force RX Impedance //			

  //********************************************OPTIMIZE RX IMPEDANCE by EFUSE*****************************************************//
#if 0  //move to xpon_pma_param_opt()   #ifdef TCSUPPORT_CPU_EN7581

	  PON_PHY_PRINT(PHY_MSG_ERR,"EN7581_EFUSE: %x\n", EN7581_EFUSE);
	#if EN7581_EFUSE
		  //-------RX R50 Calibration load from EFUSE-------//
			  read_data = get_efuse_data(RG_XPON_RX_FE_50OHMS_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_SIGDET_NOVTH, 25, 24, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"RG_XPON_RX_FE_500HMS_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_SIGDET_NOVTH, 25, 24, 0x1);   // if EFUSE readout = 0x0, set RX R50 impedance to level 1(0x1) by default
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set RX Impedance Level 1 as default! \n");
			  }
	#else
		  //-------RX R50 Calibration use default value-------//
			  PON_PHY_PRINT(PHY_MSG_ERR,"RX R50 Calibration use default value. \n");
			  IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_RX_SIGDET_NOVTH, 25, 24, 0x1);   // if EFUSE readout = 0x0, set RX R50 impedance to level 1(0x1) by default  
	#endif
 
#endif
  //***************************************************************************************************************************//
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_Rxrdy ===\n");
#endif

}



void XPON_DIG_ref_release(void)
{
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x01);	 //rg_sw_ref_rst_n

	udelay(100);

	
#if LAB_PRINT_PON
	  printk("=== XPON_DIG_ref_release_done ===\n");
#endif

	
}



void XPON_DIG_reset(int plug_sel)
{

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 6, 6, 0x00);   //rg_sw_tx_fifo_rst_n 
   
   if(plug_sel == FIRST_PLUG_IN)
   {
     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x00);   //rg_sw_ref_rst_n
   }
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 4, 4, 0x00);   //rg_sw_allpcs_rst_n 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x00);   //rg_sw_pma_rst_n
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 2, 2, 0x00);   //rg_sw_tx_rst_n 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 1, 1, 0x00);   //rg_sw_rx_rst_n 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x00);   //rg_sw_rx_fifo_rst_n
   
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x01);	//rg_sw_ref_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 1, 1, 0x01);	//rg_sw_rx_rst_n    
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 2, 2, 0x01);   //rg_sw_tx_rst_n    
    udelay(500);//Morris_test
    
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x01);   //rg_sw_pma_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x01);   //rg_sw_rx_fifo_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 6, 6, 0x01);   //rg_sw_tx_fifo_rst_n      
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 4, 4, 0x01);   //rg_sw_allpcs_rst_n   
   
	udelay(100);


#if LAB_PRINT_PON
  printk("=== XPON_DIG_reset_done ===\n");
#endif

}



void XPON_DIG_reset_hold(void)
{  
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 6, 6, 0x00);   //rg_sw_tx_fifo_rst_n 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x00);   //rg_sw_ref_rst_n        
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 4, 4, 0x00);   //rg_sw_allpcs_rst_n 
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x00);   //rg_sw_pma_rst_n
			
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 2, 2, 0x00);   //rg_sw_tx_rst_n  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 1, 1, 0x00);   //rg_sw_rx_rst_n    
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x00);   //rg_sw_rx_fifo_rst_n


#if LAB_PRINT_PON
  printk("=== XPON_DIG_reset_done ===\n");
#endif

}



void XPON_DIG_reset_release(void)
{

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 5, 5, 0x01);	//rg_sw_ref_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 1, 1, 0x01);	//rg_sw_rx_rst_n    
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 2, 2, 0x01);   //rg_sw_tx_rst_n    
   //udelay(500); //Morris_test

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x01);   //rg_sw_pma_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 0, 0, 0x01);   //rg_sw_rx_fifo_rst_n   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 6, 6, 0x01);   //rg_sw_tx_fifo_rst_n      
   
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 4, 4, 0x01);   //rg_sw_allpcs_rst_n 


#if LAB_PRINT_PON
  printk("=== XPON_DIG_reset_done ===\n");
#endif

}




void XPON_BIST_setting(void)
{ 

    //BIST_tx_rx    	   
   	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_ALIGN_PAT, 31, 0, pon_pma_param.Align_Pat = Align_PRBS31 );  //bistctl_align_pattern 
   	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_PRBS_INITIAL_SEED, 31, 0, 0xFF1FD53); //bistctl_prbs_init_seed
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 15, 0, 0x01);    //rg_bistctl_prbs_fail_threshold	
	
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 4, 0, pon_pma_param.TX_Pat = PRBS31 );                 //rg_bistctl_pat_sel 

  	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_POLLUTION, 16, 16, 0x01); //rg_bist_tx_data_pollution_latch
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 0, 0, 0x00);           //anlt_px_lnx_lt_los 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 24, 24, 0x00);         //rg_lnx_bistctl_bit_error_rst_sel 
	

#if LAB_PRINT_PON
   printk("=== XPON_Bist_Setting ===\n");
#endif

}



void XPON_R2T_sel(uint r2t_sel)
{

 switch(r2t_sel)
  {
  
 case bist_data:	
   	
		  XPON_BIST_on();
		 
        break;

		
 case r2t_data  :
 	
		  XPON_BIST_on();
	      XPON_R2T_on();
		   
	   break;
	   
 case nor_pma_data  :

		  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x00);   //rg_tx_bist_gen_en
		  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 8, 8, 0x00);   //rg_r2t_mode
          
		                                        //{rg_r2t_mode , rg_tx_bist_gen_en}
		                                        //2'b01 : da_tx_data = bist_data
		                                        //2'b10 : da_tx_data = r2t_data
		                                        //otherwise : normal_pma_tx_data
	   break;
	

      default:
	 break;	
	}

}



void XPON_BIST_on(void)
{ 

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 8, 8, 0x01);   //rg_bisctl_pat_tx_en
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 8, 8, 0x00);   //rg_r2t_mode
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 24, 24, 0x01); //rg_r2t_fifo_en	
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x01); //rg_tx_bist_gen_en	 
     
     IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 8, 8, 0x00);         //all_lane_prbs_tx_en     
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); // rg_bistctl_pat_rx_check_en 
  
     udelay(100);

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x00); // rg_bistctl_pat_rx_check_en 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); // rg_bistctl_pat_rx_check_en 

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x00); // rg_bistctl_pat_rx_check_en 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); // rg_bistctl_pat_rx_check_en     
    
  
#if LAB_PRINT_PON
   printk("=== XPON_Bist_On ===\n");
#endif

}


void XPON_RX_L2D(void)
{

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_lpf_rstb  
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x00); //rg_force_da_pxp_cdr_lpf_rstb		
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x01); //rg_force_da_pxp_cdr_lpf_rstb	

	udelay(200);

    //L2D  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0, 0, 0x01); //rg_force_da_pxp_cdr_lpf_lck2data
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_lpf_lck2data   


#if LAB_PRINT_PON
   printk("=== XPON_RX_L2D_done ===\n");
#endif

}



void EO_Scan(uint bit_sel, uint EO_start, uint EO_end)
{

    volatile int j = 0 , k = 0;
    volatile unsigned int leq_gain = 1;            
    volatile unsigned int leq_peaking = 0;
    volatile unsigned int sweep_start = EO_start;
    volatile unsigned int sweep_end = EO_end ;            
    volatile unsigned int fom_num = 0;
    volatile unsigned int best_fom = 0;   
    volatile unsigned int best_gain = 1;
    volatile unsigned int best_peaking = 0; 

	 #if Pon_Eye_Scan_Result
         printk("bit_sel => %x \n" ,bit_sel);
     #endif 

if(bit_sel == Async_XEPON || bit_sel == Sync_XEPON || bit_sel == Async_XGPON_1 || bit_sel == Async_XGPON_2 || bit_sel == Sync_XGSPON_1 || bit_sel == Sync_XGSPON_2)	 
{

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01); //rg_force_da_pxp_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
   

    XPON_eye_setting(bit_sel);

    //EYE setting, EYE OPEN related  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);  //rg_eq_en_delay
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x04);  //rg_kpgain
   for (k = leq_gain; k < 4; k++)
   {
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);      //rg_force_sel_da_pxp_rx_fe_gain_ctrl
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, leq_gain);  //rg_force_da_pxp_rx_fe_gain_ctrl
   
	  leq_peaking = 0;

   for (j = sweep_start; j < sweep_end + 1; j++)
   {
   
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);      //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, leq_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl
	  
              
     #if Pon_Eye_Scan_Result
         printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
     #endif      
	 
      XPON_eye_Cal();

      fom_num = XPON_eye_EO(bit_sel);                    
                
      if (fom_num > best_fom)
      {
          best_fom = fom_num;                        
          best_peaking = leq_peaking;
			best_gain = leq_gain;
          leq_peaking++;
      }
      else
      {
          leq_peaking++;
      }                   
   }

	 leq_gain++;
	 
   }
				
    //set back the best leq gain&peaking setting
   	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);       //rg_force_sel_da_pxp_rx_fe_gain_ctrl
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, best_gain);  //rg_force_da_pxp_rx_fe_gain_ctrl

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);        //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, best_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl
       
    #if Pon_Eye_Scan_Result
       printk("XPON_best_leq_gain => %x , XPON_best_leq_peaking => %x\n" ,best_gain , best_peaking);
 	#endif	 
  }
}



void XPON_eye_setting(uint bit_set)
{
    if (bit_set == EO_Async_XGPON_1 || bit_set == EO_Sync_XGSPON_2 || bit_set == EO_Sync_XGSPON_1 || bit_set == EO_Sync_XGSPON_2)
    {         
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x00); //RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);     //rg_eye_mask
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);   //rg_x_min
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240);  //rg_x_max
	   
    }
    else if (bit_set == EO_Sync_GPON || bit_set == EO_Async_GPON)
    {  

	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x02); //RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0x55);     //rg_eye_mask
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x100);   //rg_x_min
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x300);  //rg_x_max
    }
    else if (bit_set == EO_Sync_EPON_1 || bit_set == EO_Sync_EPON_2 )
    {

	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x03); //RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0x22);	   //rg_eye_mask
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x0);      //rg_x_min
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x400);   //rg_x_max	
		
    }   
    else
    {  
	   IO_SPHYA_REG_BITS(EN7581_XPON_ANA_RG_PXP_CDR_LPF_RATIO, 1, 0, 0x00); //RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);	   //rg_eye_mask
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);    //rg_x_min
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x234);   //rg_x_max	
    }

	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xF8);   //rg_cntlen
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 16, 16, 0x0);  //rg_cntforever
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x0);    //rg_data_shift 	 

	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x00);   //rg_index_mode
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0xFFF8);  //rg_eyedur	 

	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 0, 0, 0x0);    //rg_eye_nextpts_sel
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 8, 8, 0x0);    //rg_eye_nextpts_toggle
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);  //rg_eye_nextpts	 

	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x4);     //rg_eyecnt_hth
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x4);    //rg_eyecnt_vth
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x4);    //rg_eo_hth
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth

	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_1, 16, 16, 0x0);   //rg_heo_emphasis
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_1, 0, 0, 0x0);     //rg_a_lgain
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_1, 8, 8, 0x0);     //rg_a_mgain	   
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_2, 1, 0, 0x1); 	//rg_a_sel
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_1, 24, 24, 0x0); 	//rg_b_zero_sel
	 
	 
}


void XPON_eye_Cal(void)
{

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, 0x0);	//rg_force_da_pxp_cdr_pr_pieye
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x0);   //rg_force_sel_da_pxp_cdr_pr_pieye
    
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 22, 16, 0x0);   //rg_force_da_pxp_rx_dac_eye	 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 24, 24, 0x0);   //rg_force_sel_da_pxp_rx_dac_eye , Morris_Test
	
	
	 
    //pical redo
    //reset block
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);   //rg_eq_en_delay
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x01);   //rg_kpgain
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 16, 16, 0x00);    //rg_eq_pi_cal_rst_b

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_6, 8, 8, 0x00);  //rg_disb_rx_and_pical_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x00); //rg_force_rx_and_pical_rstb

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_6, 0, 0, 0x00);  //rg_disb_ref_and_pical_rstb
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x00); //rg_force_ref_and_pical_rstb

		
    //enable	
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_5, 24, 24, 0x00);  //rg_disb_rx_or_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);   //rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8, 8, 0x00);   //rg_disb_rx_pical_en
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x00);  //rg_force_rx_pical_en
   
		
    //release reset
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_RESET_0, 16, 16, 0x01);     //rg_eq_pi_cal_rst_b
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x01);  //rg_force_rx_and_pical_rstb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x01);  //rg_force_ref_and_pical_rstb
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x01);  //rg_force_rx_or_pical_en   
  
    udelay(1000); //delay for 1ms 

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);   //rg_force_rx_or_pical_en      
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x00);	  //rg_disb_eq_pi_cal_rdy
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x01);	  //rg_force_eq_pi_cal_rdy   

	//Kris_test
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy , Morris_Test
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 0, 0, 0x1);   //rg_force_eyecnt_rdy , Morris_Test	
}



 int XPON_eye_EO(uint bit_rate)
{
   volatile unsigned int fom_num;


    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0x0A);     //rg_cntlen  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C); //rg_eyedur     

    //reset eye_top   
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 8, 8, 0x00);     //rg_disb_eye_reset_plu_o 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);    //rg_force_eye_reset_plu_o  
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x00);    //rg_force_eye_reset_plu_o  

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x00);   //rg_disb_eye_top_en 
	 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);  //rg_force_eye_top_en 
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x01);  //rg_force_eye_top_en    

     
   if (bit_rate == EO_Async_XEPON || bit_rate == EO_Sync_XEPON || bit_rate == EO_Async_XGPON_1 || bit_rate == EO_Async_XGPON_2 || bit_rate == EO_Sync_XGSPON_1 || bit_rate == EO_Sync_XGSPON_2)
   {
       udelay(5500); //delay for 5.5ms        
   }
   else
   {	   
	   mdelay(55); //delay for 55ms  
   }

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 8, 8, 0x01);     //rg_disb_da_xpon_rx_dac_eye  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_2, 24, 24, 0x01);   //rg_disb_da_xpon_cdr_pr_pieye  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01);   //rg_disb_eyedur_init_b  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x01);           //rg_disb_eyecnt_rx_rst_b  
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01);   //rg_disb_eyedur_en
   

   //read out EO results
   fom_num = XPON_readout_EO(0); 

   //clear EXE_X_SW offset value
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);     //rg_force_eye_reset_plu_o  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x00);    //rg_disb_eye_top_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);   //rg_disb_eye_top_en  
   

   //reset pi_cal_rdy
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x00);     //rg_disb_eq_pi_cal_rdy  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x00);    //rg_force_eq_pi_cal_rdy  
   //reset eyecnt_rdy
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy , Morris_Test
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_6, 0, 0, 0x0);   //rg_force_eyecnt_rdy , Morris_Test
      

   return fom_num;
}



int XPON_readout_EO(int sweep_direction)
{
   volatile unsigned int eye_er, eye_el, eye_eu, eye_eb, i = 0;
   volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos,y_index = 0 , DAC_RANGE_EYE = 0;
   volatile	unsigned int eye_x_done, eye_y_done, heo_rdy, veo_rdy;
   //volatile unsigned int veo;   
   volatile int fom_num,heo ,veo,heo_tmp , veo_tmp , eye_er_tmp , eye_el_tmp , eye_eu_tmp , eye_eb_tmp;
   volatile unsigned int pical_data_out;   
   volatile unsigned int EO_Buf[10]= {0};
   


   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 0, 0, 0x01);	  //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 8, 8, 0x01);	  //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 16, 16, 0x01);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 24, 24, 0x01);  //ro_lnx_sw_fll_ro_4_latch_en  
   
   udelay(50);

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 0, 0, 0x00);    //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 8, 8, 0x00);    //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 16, 16, 0x00);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_6, 24, 24, 0x00);  //ro_lnx_sw_fll_ro_4_latch_en  

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
	udelay(100);
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle  
	

	pical_data_out = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_2, 22, 16); //ro_pi_cal_data_out
	eye_x_done = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_9, 16, 16);     //eo_x_done
	eye_y_done = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_9, 24, 24);     //eo_y_done
	heo_rdy = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_5, 16, 16);        //heo_rdy
	veo_rdy = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_5, 24, 24);        //veo_rdy
	
   
  if ((eye_x_done == 1) && (eye_y_done == 1))
  {	   
	  
#if Pon_Eye_Scan_Result
		printk("\n" );	    
		printk("X&Y index scan done\n");
#endif

	if ((heo_rdy == 1) && (veo_rdy == 1))
	  {		
	       
#if Pon_Eye_Scan_Result
	    printk("\n" );			 
		printk("heo&veo result ready\n");
#endif
	   }
	 }
	else
	   {	
		   
#if Pon_Eye_Scan_Result
		printk("X&Y index scan NOT done\n");
#endif
	   }    

	 eye_el = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_10, 26, 16); //eye_el	 
	 eye_er = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_10, 10, 0);  //eye_er
	 eye_eu = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_11, 6, 0);   //eye_eu	 
	 eye_eb = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_RX_TORGS_DEBUG_11, 14, 8);	//eye_eb
	 

	 //ABS_Funt._0724 Morris
	 eye_er_tmp = (int) eye_er;  //uint to int 
	 eye_el_tmp = (int) eye_el;
		   
     heo_tmp= eye_er_tmp - eye_el_tmp;
	 
   if (heo_tmp>0)
   {
       heo = heo_tmp;
   }
   else
   {
       heo = heo_tmp * -1;
   }

	 dac_d0 = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_1, 14, 8);  //ro_rx_dac_d0
	 dac_d1 = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_1, 22, 16); //ro_rx_dac_d1
	 dac_e0 = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_1, 30, 24); //ro_rx_dac_e0
	 dac_e1 = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_2, 6, 0);   //ro_rx_dac_e1
	 dac_eye = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_1, 6, 0);  //ro_rx_dac_eye	 
	 feos = IO_GPHYA_REG_BITS(EN7581_XPON_PMA_ADD_RO_RX2ANA_2, 13, 8);  //ro_rx_feos_out
	   
	 fom_num = heo;   
    
	 //print out these results to textbox
   if (sweep_direction == 0)
      {	
 #if Pon_Eye_Scan_Result
		printk("Y index sweep direction: bottom-up\n");
 #endif
	  }
   else
	  {
 #if Pon_Eye_Scan_Result
	    printk("Y index sweep direction: upside-down\n");
 #endif
	  }

	  EO_Buf[0] = eye_eu;
	  EO_Buf[1] = eye_eb;
	  EO_Buf[2] = dac_d0;
	  EO_Buf[3] = dac_d1;
	  EO_Buf[4] = dac_e0;
	  EO_Buf[5] = dac_e1;
	  EO_Buf[6] = dac_eye;
	  EO_Buf[7] = feos;

  for (i = 0; i < 8; i++)
	 {
	    if ((EO_Buf[i] == feos) && (EO_Buf[i] >= 32))
		  {
		     EO_Buf[i] = EO_Buf[i] - 64;
		  }
	    else if (EO_Buf[i] >= 64)
		  {
		     EO_Buf[i] = EO_Buf[i] - 128;
		  }
	 }

   eye_eu_tmp = (int) EO_Buf[0];  //uint to int 
   eye_eb_tmp = (int) EO_Buf[1];

   veo_tmp= eye_eu_tmp - eye_eb_tmp;
   

   if (veo_tmp>0)
   {
       veo = veo_tmp;
   }
   else
   {       
	   veo = veo_tmp * -1;
   }
  

 #if Pon_Eye_Scan_Result		   
	  printk("fom_num => %d\n",fom_num); 
	  printk("pical_data_out => %d\n",pical_data_out);		   
	  printk("heo => %d\n",heo);		   
	  printk("veo => %d\n",veo);		   
	  printk("X_done=> %d  Y_done= %d\n", eye_x_done, eye_y_done);	   
      printk("heo_rdy=> %d\n", heo_rdy);	   
	  printk("veo_rdy=> %d\n", veo_rdy);		  
	  printk("eye_er=> %d\n", eye_er);		   
	  printk("eye_el=> %d\n", eye_el);		  
	  printk("eye_eu=> %d\n", EO_Buf[0]);		   
	  printk("eye_eb=> %d\n", EO_Buf[1]);		   
	  printk("\n");
		   
	  printk("dac_d0=> %d\n", EO_Buf[2]);	
	  printk("dac_d1=> %d\n", EO_Buf[3]);	
	  printk("dac_e0=> %d\n", EO_Buf[4]);	
	  printk("dac_e1=> %d\n", EO_Buf[5]);	
	  printk("dac_eye=> %d\n", EO_Buf[6]);
	  printk("ro_feos=> %d\n", EO_Buf[7]);	
	  printk("\n");
 #endif		   

//check whether CLK unlocking happens
  if (heo_rdy != 1)
	{	
		   
 #if Pon_Eye_Scan_Result	
      printk("EO result isn't valid.\n");
 #endif	
   
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_DISB_MODE_0, 24, 24, 0x00);    //rg_disb_da_xpon_cdr_lpf_rstb  
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x00);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	 udelay(500); //delay for 500us 	  

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x01);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	  udelay(500); //delay for 500us 
	 }
   return fom_num;
   
}	   


void XPON_FLL_Reset(void)
{
  
  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_3, 0, 0, 0x00); //rg_fll_dig_rst_force_en
  udelay(500);
  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_3, 0, 0, 0x01); //rg_fll_dig_rst_force_en  

  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_3, 8, 8, 0x00); //rg_fll_dig_rst_force
  udelay(500);
  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_3, 8, 8, 0x01); //rg_fll_dig_rst_force

  #if LAB_PRINT_PON
  printk("### = XPON_FLL_Reset\r\n");
  #endif
}



void XPON_TDC_on(void)
{

    //TDC    
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_6, 9, 8, 0x00); //rg_lcpll_ncpo_chg_delay
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_0, 1, 0, 0x01); //rg_lcpll_pon_rx_cdr_divtdc 
   
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x00); //rg_force_sel_da_pxp_txpll_sdm_pcw_chg
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x00); //rg_force_sel_da_pxp_txpll_sdm_pcw

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PW_5, 16, 16, 0x01); //rg_lcpll_autok_tdc
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_3, 8, 8, 0x00); //rg_lcpll_ncpo_load
    udelay(100);

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_5, 16, 16, 0x00); //rg_lcpll_tdc_autopw_ncpo
    udelay(100);

    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_1, 0, 0, 0x01); //rg_lcpll_gpon_sel
    IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_PW_0, 0, 0, 0x01); //rg_lcpll_tdc_dig_pwdb
    //mdelay(5);	// add delay for reset flow stability
    //mdelay(1);	// add delay for reset flow stability
    udelay(500); //delay for 500us 

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_lpf_rstb  
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x00); //rg_force_da_pxp_cdr_lpf_rstb	

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, 0x01); //rg_force_sel_da_pxp_cdr_lpf_rstb  
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, 0x01); //rg_force_da_pxp_cdr_lpf_rstb	


#if LAB_PRINT_PON
   printk("=== XPON_TDC_On_done ===\n");
#endif
  
}




void XPON_R2T_on(void)
{ 

    //R2T/
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x00); //rg_tx_bist_gen_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 8, 8, 0x00); //rg_bistctl_pat_tx_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); // rg_bistctl_pat_rx_check_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_BIST_1, 8, 8, 0x00); //all_lane_prbs_tx_en

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 8, 8, 0x00); //rg_r2t_mode
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 8, 8, 0x01); //rg_r2t_mode

	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 24, 24, 0x00); //rg_r2t_fifo_en
	IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 24, 24, 0x01); //rg_r2t_fifo_en 
  

#if LAB_PRINT_PON
   printk("=== XPON_R2T_On ===\n");
#endif

}


void Pon_Version (void)
{
    printk("=== XPON_Physical_Init._Ver1.1 ===\n" );
}


void JBERT_pon_Ext_T2R (int pon_Spd)
{  

   pon_CfgPhyType(pon_Spd);
   
   JCPLL_on();   
   TXPLL_on();
   XPON_DIG_fm_on(pon_Spd); 	     
   XPON_TX_on();
   
   XPON_RX_preset(); 
   XPON_TDC_off(); 
   XPON_RX_on(pon_Spd); 
   XPON_RX_L2R(); 

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   
   if(GET_PDIDR() == 0x1) 
   {
   	  if((pon_Spd == Async_XEPON) ||(pon_Spd == Sync_XEPON) || (pon_Spd == Sync_XGSPON_1)|| (pon_Spd == Sync_XGSPON_2)|| (pon_Spd == Async_XGPON_1)|| (pon_Spd == Async_XGPON_2))
      {        
	     XPON_RX_feos();
      }
   }
   else
   {
     XPON_RX_feos();
   }
   
   XPON_RX_sdcal();  
   
   XPON_phy_status();
   XPON_DIG_reset(FIRST_PLUG_IN);      
   XPON_RX_L2D();   
   EO_Scan(pon_Spd,0,7); //20221011
   XPON_TDC_on();     
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(bist_data);  
   
   
}

void pon_Ext_T2R (int pon_Spd)
{  

   pon_CfgPhyType(pon_Spd);
   
   JCPLL_on();   
   TXPLL_on();
   XPON_DIG_fm_on(pon_Spd); 	     
   XPON_TX_on();
   
   XPON_RX_preset(); 
   XPON_TDC_off(); 
   XPON_RX_on(pon_Spd); 
   XPON_RX_L2R(); 

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   
   if(GET_PDIDR() == 0x1) 
   {
   	  if((pon_Spd == Async_XEPON) ||(pon_Spd == Sync_XEPON) || (pon_Spd == Sync_XGSPON_1)|| (pon_Spd == Sync_XGSPON_2)|| (pon_Spd == Async_XGPON_1)|| (pon_Spd == Async_XGPON_2))
      {         
	     XPON_RX_feos();
      }
   }
   else
   {
     XPON_RX_feos();
   }

   XPON_RX_sdcal();  
   
   XPON_phy_status();
   XPON_DIG_reset(FIRST_PLUG_IN);      
   XPON_RX_L2D();   
   EO_Scan(pon_Spd,0,7); //20221011
   //XPON_TDC_on();     
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(bist_data);  
   
   
}


void pon_Ext_R2T (int pon_Spd)
{

   pon_CfgPhyType(pon_Spd);

   JCPLL_on();   
   TXPLL_on();
   XPON_DIG_fm_on(pon_Spd);    
   XPON_TX_on();
   
   XPON_RX_preset(); 
   XPON_TDC_off(); 
   XPON_RX_on(pon_Spd); 
   XPON_RX_L2R(); 

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   
   if(GET_PDIDR() == 0x1) 
   {
   	  if((pon_Spd == Async_XEPON) ||(pon_Spd == Sync_XEPON) || (pon_Spd == Sync_XGSPON_1)|| (pon_Spd == Sync_XGSPON_2)|| (pon_Spd == Async_XGPON_1)|| (pon_Spd == Async_XGPON_2))
      {         
	     XPON_RX_feos();
      }
   }
   else
   {
     XPON_RX_feos();
   }

   XPON_RX_sdcal();   
   
   XPON_phy_status();
   XPON_DIG_reset(FIRST_PLUG_IN);      
   XPON_RX_L2D();   
   EO_Scan(InitSpd_RX,0,7); //20221011
   XPON_TDC_on();     
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(r2t_data);  
   
}


void Pon_Opt_Cal (int mode_Sel , int pat_sel)
{

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x01);     //rg_force_sel_da_pxp_txpll_sdm_pcw
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_3, 8, 8, 0x01);  //rg_lcpll_ncpo_load
   udelay(100);

   if(mode_Sel == 0x0)
   {	   
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_2, 31, 0, 0x08400000);  //rg_lcpll_ncpo_value
   }
   else if (mode_Sel == 0x1)
   {
	   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_LCPLL_TDC_FLT_2, 31, 0, 0x07F66E86);  //rg_lcpll_ncpo_value
   }
   

   if(pat_sel == 0x0)
   {	   
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 4, 0, pon_pma_param.TX_Pat = PRBS7 );   //rg_bistctl_pat_sel 
   }
   else if (pat_sel == 0x1)
   {
	  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_BISTCTL_CONTROL, 4, 0, pon_pma_param.TX_Pat = PRBS31 );   //rg_bistctl_pat_sel 
   }
   

   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x00); //rg_force_da_pxp_txpll_en
   udelay(100);
   IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x01); //rg_force_da_pxp_txpll_en

}






void sw_pma_rst_hold(void)
{

// holding & Releasing sw_pma_rst_n can avoid big EqD variation with multiple plug-out/plug-in tests  
  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x00); //hold sw_pma_rst_n , follow AN7580
}


void sw_pma_rst_release(void)
{

// holding & Releasing sw_pma_rst_n can avoid big EqD variation with multiple plug-out/plug-in tests   
  IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SW_RST_SET, 3, 3, 0x01); //hold sw_pma_rst_n , follow AN7580
}


#endif
// -- EN7581 PMA function split END------------------------------------------------




