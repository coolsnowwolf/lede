//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : EN7580 PON PHY
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

#include "en7580.h"
#include "en7580_reg.h"
#include "en7580_pma.h"

#if EN7580_EFUSE
	#if defined(TCSUPPORT_CPU_EN7580)
		#include <boot/packageInfo.h>
	#endif
#endif

// --  variables ----------------------------------------------------------------


#define _definition_SPLIT_LINE_

#if ASIC_SERDES	// for EN7580_ASIC
#define _PMA_FUNCTION_SPLIT_LINE_BASIC 
// -- EN7580 PMA function split  from Morris------------------------------------------------

int xpon_pma_mode_init(void)
{
	UINT32 read_data ;


#if !EN7580_EYE_SCAN
	//step 1: select scu mode
	scu_mode_sel(gpPhyPriv->wan_sel);

	//step 2: mode init setting
	xpon_init(gpPhyPriv->wan_sel); 

	//step 3: reset
	if(pon_phy_get_los_status()==1)
	{
		printk("<<<<<<<<<<< No Laser! >>>>>>>>>>>>>\n");
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
		printk("======= 350ms delay =======\n");
		mdelay(350);	//add by david 20190508  for Hangyan
		//*************************************************************************//
	}

	
#endif
#if EN7580_EYE_SCAN
    printk("___________PHY init for EN7580_EYE_SCAN_____________\r\n");
#endif

	return 0;
}


void xpon_init (int bit_sel)
{

	switch(bit_sel)
		{
		case Async_GPON: //DS_2.48832G  /  US_1.24416G		 
			 setting_mode_XPON_DIG(scu_Async_GPON); 
			 setting_mode_XPON_TXPLL(9); 
			 setting_mode_XPON_TX(TX_Async_GPON); 
			 setting_mode_XPON_RX(2); 
			 setting_XPON_DIG(); 
			 setting_XPON_ANA(1); 
			 seq_XPON_DIG_reset(); 
			 seq_JCPLL_on(); 
			 seq_XPON_DIG_fm_on(fm_Async_GPON); 			 
			 
			 break;


		case Async_XEPON: //DS_10.3125G  /  US_1.25G			 
			 setting_mode_XPON_DIG(scu_Async_XEPON); 
			 setting_mode_XPON_TXPLL(10); 
			 setting_mode_XPON_TX(TX_Async_XEPON); 
			 setting_mode_XPON_RX(10); 
			 setting_XPON_DIG(); 
			 setting_XPON_ANA(1); 
			 seq_XPON_DIG_reset(); 
			 seq_JCPLL_on(); 
			 seq_XPON_DIG_fm_on(fm_Async_XEPON); 		 
			 
			 break;

			 
		case Sync_XEPON: //DS_10.3125G  /  US_10.3125G
			 setting_mode_XPON_DIG(scu_Sync_XEPON); 
             setting_mode_XPON_TXPLL(10); 
             setting_mode_XPON_TX(TX_Sync_XEPON); 
             setting_mode_XPON_RX(10); 
             setting_XPON_DIG(); 
             setting_XPON_ANA(10); 
             seq_XPON_DIG_reset(); 
             seq_JCPLL_on(); 
             seq_XPON_DIG_fm_on(fm_Sync_XEPON); 
			 
			 break; 

	
		case Sync_XGSPON_1: //DS_9.95328G  /  US_9.95328G		
		case Sync_XGSPON_2: //DS_9.95328G  /  US_9.95328G	
			 //setting_mode_XPON_DIG(scu_Sync_XGSPON); 
             setting_mode_XPON_TXPLL(9); 
             setting_mode_XPON_TX(TX_Sync_XGSPON); 
             setting_mode_XPON_RX(9); 
             setting_XPON_DIG(); 
             setting_XPON_ANA(9); 
             seq_XPON_DIG_reset(); 
             seq_JCPLL_on(); 
             seq_XPON_DIG_fm_on(fm_Sync_XGSPON); 			 
			 
			 break; 


		case Async_XGPON_1: //DS_9.95328G  /  US_2.48832G	
		case Async_XGPON_2: //DS_9.95328G  /  US_2.48832G
			 setting_mode_XPON_DIG(scu_Async_XGPON); 
             setting_mode_XPON_TXPLL(9); 
             setting_mode_XPON_TX(TX_Async_XGPON); 
             setting_mode_XPON_RX(9); 
             setting_XPON_DIG(); 
             setting_XPON_ANA(9); 
             seq_XPON_DIG_reset(); 
             seq_JCPLL_on(); 
             seq_XPON_DIG_fm_on(fm_Async_XGPON); 			 
			 
			 break; 

	
		case Sync_GPON: //DS_2.48832G  /  US_2.48832G			 	
			 setting_mode_XPON_DIG(scu_Sync_GPON); 
			 setting_mode_XPON_TXPLL(2); 
			 setting_mode_XPON_TX(TX_Sync_GPON); 
			 setting_mode_XPON_RX(2); 
			 setting_XPON_DIG(); 
			 setting_XPON_ANA(2); 
			 seq_XPON_DIG_reset(); 
			 seq_JCPLL_on(); 
			 seq_XPON_DIG_fm_on(fm_Sync_GPON); 			 
			 
			 break; 
	
	
		case Sync_EPON_1: //DS_1.25G      /  US_1.25G	
		case Sync_EPON_2: //DS_1.25G      /  US_1.25G	
			 setting_mode_XPON_DIG(scu_Sync_EPON); 
			 setting_mode_XPON_TXPLL(1); 
			 setting_mode_XPON_TX(TX_Sync_EPON); 
			 setting_mode_XPON_RX(1); 
			 setting_XPON_DIG(); 
			 setting_XPON_ANA(1); 
			 seq_XPON_DIG_reset(); 
			 seq_JCPLL_on(); 
			 seq_XPON_DIG_fm_on(fm_Sync_EPON); 		 
			 
			 break; 
		
			default:
				break;	
		 }

          seq_XPON_TXPLL_on(); 
		  seq_XPON_TX_on(); 		//move from fiber_plug_reset () to enable TX CLK making all PCS RG addressable before first_plug_in fibre.  by david 20190515

		  
#if LAB_PON_Test
		  seq_XPON_TX_on(); 
	         seq_XPON_RX_preset(); 
 	         aux_XPON_TDC_off(); 
 	         seq_XPON_RX_on(); 
 	         seq_XPON_RX_L2R(); 
 	         seq_XPON_RX_pical(); 
 	         seq_XPON_RX_pdos(); 
 	         seq_XPON_RX_feos(); 
 	         seq_XPON_RX_sdcal(); 
 	         aux_XPON_phy_status();
	      //seq_XPON_RX_L2D();
	      //EO_Scan(0,tmp_word,0,7,0);
          //seq_XPON_RX_blwc_on(); 
	      //aux_XPON_TDC_on();
         	  seq_XPON_RX_rxrdy(); 	     
		  aux_XPON_phy_status();
		  aux_XPON_BIST_setting();
		  aux_XPON_R2T_sel(bist_data); 
		  //EO_Scan(0,10,0,7);
#endif


}



void fiber_plug_reset (int plug_sel , int bit_sel)
{

switch(plug_sel)
	{
	
 case FIRST_PLUG_IN: 
 	
//		seq_XPON_TX_on(); 		//move to xpon_init() to enable TX CLK making all PCS RG addressable before first_plug_in fibre.  by david 20190515
		seq_XPON_RX_preset(); 
		aux_XPON_TDC_off(); 
		seq_XPON_RX_on(); 
		seq_XPON_RX_L2R(); 
		seq_XPON_RX_pical(); 
		seq_XPON_RX_pdos(); 
		seq_XPON_RX_feos(); 
		seq_XPON_RX_sdcal(); 
		aux_XPON_phy_status();
		seq_XPON_RX_L2D();
	   	EO_Scan(0,bit_sel,0,7,0); //20181218
		seq_XPON_RX_blwc_on();
		aux_XPON_TDC_on();
		seq_XPON_RX_rxrdy();		
		aux_XPON_phy_status();		
		aux_XPON_R2T_sel(nor_pma_data);
		
		//aux_XPON_BIST_setting();
		//aux_XPON_R2T_sel(bist_data); 

	break;	

 case PLUG_IN: 

	  // sw_tx_rst(); //reset tx, add by ang_20191031
	   seq_XPON_DIG_reset(); // add digital reset when plug out/in, ang_20200915
	  
	   sw_pma_rst_hold();
       seq_XPON_RX_L2D();
	   aux_XPON_TDC_on();
	   sw_pma_rst_release();
	   aux_XPON_phy_status();
	   aux_XPON_R2T_sel(nor_pma_data);	
	   
	   //aux_XPON_R2T_sel(bist_data); 

	break;	


 case PLUG_OUT: 

	   aux_XPON_TDC_off();
	   seq_XPON_RX_L2R();	
	   seq_XPON_FLL_Reset();

	break;	


  default:
	break;
	
	
    }


}


void Ext_R2T (int bit_sel)
{
//digital mode
//10p3125G
//CK for MAC layer

IO_SPHYA_REG_BITS(0xBFB00830, 8, 8, 0x1);   //SCU_Reset
IO_SPHYA_REG_BITS(0xBFB00830, 8, 8, 0x0);   //SCU_Reset


switch(bit_sel)
	{
	case Sync_XEPON: // 10G_ mode                
		 setting_mode_XPON_DIG(scu_Sync_XEPON); 
		 setting_mode_XPON_TXPLL(10); 
		 setting_mode_XPON_TX(TX_Sync_XEPON); 
		 setting_mode_XPON_RX(10); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(10); 
		 seq_XPON_DIG_reset(); 
		 seq_JCPLL_on(); 
		 seq_XPON_DIG_fm_on(fm_Sync_XEPON); 
		 
         break;	

    case Sync_XGSPON_1: // 9G_ mode      
    case Sync_XGSPON_2: // 9G_ mode
		// setting_mode_XPON_DIG(scu_Sync_XGSPON); 
		 setting_mode_XPON_TXPLL(9); 
		 setting_mode_XPON_TX(TX_Sync_XGSPON); 
		 setting_mode_XPON_RX(9); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(9); 
		 seq_XPON_DIG_reset(); 
		 seq_JCPLL_on(); 
		 seq_XPON_DIG_fm_on(fm_Sync_XGSPON); 
		 
         break;	

	case Sync_GPON: // 2G_ mode			 
		 setting_mode_XPON_DIG(scu_Sync_GPON); 
		 setting_mode_XPON_TXPLL(2); 
		 setting_mode_XPON_TX(TX_Sync_GPON); 
		 setting_mode_XPON_RX(2); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(2); 
		 seq_XPON_DIG_reset(); 
		 seq_JCPLL_on(); 
		 seq_XPON_DIG_fm_on(fm_Sync_GPON); 
		 
		 break;	

	case Sync_EPON_1: // 1G_ mode	
	case Sync_EPON_2: // 1G_ mode
		 setting_mode_XPON_DIG(scu_Sync_EPON); 
		 setting_mode_XPON_TXPLL(1); 
		 setting_mode_XPON_TX(TX_Sync_EPON); 
		 setting_mode_XPON_RX(1); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(1); 
		 seq_XPON_DIG_reset(); 
		 seq_JCPLL_on(); 
		 seq_XPON_DIG_fm_on(fm_Sync_EPON); 
		 
		 break;	
	
		default:
			break;	
	 }

        seq_XPON_TXPLL_on(); 
        seq_XPON_TX_on(); 
        seq_XPON_RX_preset(); 
        aux_XPON_TDC_off(); 
        seq_XPON_RX_on(); 
        seq_XPON_RX_L2R(); 
        seq_XPON_RX_pical(); 
        seq_XPON_RX_pdos(); 
        seq_XPON_RX_feos(); 
        seq_XPON_RX_sdcal(); 
        aux_XPON_phy_status(); 
        //seq_XPON_RX_osrdy(); 
        seq_XPON_RX_L2D(); 
		seq_XPON_RX_blwc_on();
        aux_XPON_TDC_on(); 
        seq_XPON_RX_rxrdy(); 
        aux_XPON_BIST_setting();         
        aux_XPON_R2T_sel(r2t_data);

  if(bit_sel == Sync_GPON )
	 {
        IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 2, 0, 0x1);   //rg_xpon_mode: 0,1,2,3,4 => 16b(/1), 8b(/4), 16b(/4), 10b(/8), digital_T2R
	 }

}


void Ext_T2R (int bit_sel)
{
//digital mode
//10p3125G
//CK for MAC layer

IO_SPHYA_REG_BITS(0xBFB00830, 8, 8, 0x1);   //SCU_Reset
IO_SPHYA_REG_BITS(0xBFB00830, 8, 8, 0x0);   //SCU_Reset


switch(bit_sel)
	{
  case Sync_XEPON: // 10G_ mode                      
		 setting_mode_XPON_DIG(scu_Sync_XEPON); 
		 setting_mode_XPON_TXPLL(10); 
		 setting_mode_XPON_TX(TX_Sync_XEPON); 
		 setting_mode_XPON_RX(10); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(10); 
		 
         break;	

  case Sync_XGSPON_1: // 9G_ mode  
  case Sync_XGSPON_2: // 9G_ mode   
		// setting_mode_XPON_DIG(scu_Sync_XGSPON); 
		 setting_mode_XPON_TXPLL(9); 
		 setting_mode_XPON_TX(TX_Sync_XGSPON); 
		 setting_mode_XPON_RX(9); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(9);
		 
         break;	

  case Sync_GPON: // 2G_ mode				 
		 setting_mode_XPON_DIG(scu_Sync_GPON); 
	     setting_mode_XPON_TXPLL(2); 
	     setting_mode_XPON_TX(TX_Sync_GPON); 
	     setting_mode_XPON_RX(2); 
	     setting_XPON_DIG(); 
	     setting_XPON_ANA(2); 
		 
		 break;	

  case Sync_EPON_1: // 1G_ mode	
  case Sync_EPON_2: // 1G_ mode
		 setting_mode_XPON_DIG(scu_Sync_EPON); 
		 setting_mode_XPON_TXPLL(1); 
		 setting_mode_XPON_TX(TX_Sync_EPON); 
		 setting_mode_XPON_RX(1); 
		 setting_XPON_DIG(); 
		 setting_XPON_ANA(1); 
		 
		 break;	
	
		default:
			break;	
	 }

         seq_XPON_DIG_reset(); 
         seq_JCPLL_on(); 	
         seq_XPON_TXPLL_on(); 
         seq_XPON_TX_on(); 
         seq_XPON_RX_preset(); 
         aux_XPON_TDC_off(); 
         seq_XPON_RX_on(); 
         seq_XPON_RX_L2R();	
         aux_XPON_phy_status();	
         //aux_XPON_BIST_setting();	
         seq_XPON_RX_rxrdy(); 
         aux_XPON_BIST_setting();          
         aux_XPON_R2T_sel(bist_data);
		 aux_XPON_BIST_setting();
}


void Pon_Version (void)
{
 printk("=== XPON_Physical_Init._Ver1.1 ===\n" );
 printk("=== Add Application Function ===\n" );
 printk(" a.XPON_Ext.R2T (Support All bit Rate)\n" );
 printk(" b.XPON_Ext.T2R (Support All bit Rate)\n" );
 printk(" c.XPON_EO Scan (Support All bit Rate)\n" );
 printk("=== Modify TX_FIR Setting for E2 ===\n" );
 printk("=== 10G/9G [0,0,C,4] ===\n" );
 printk("=== 1G/2G [0,0,0,0] ===\n" );
}




void setting_mode_XPON_DIG(uint scu_bit_sel)
{
//*************************************************
// 0x00 = Async_GPON	     DS_2.48832G  /  US_1.24416G  
// 0x01 = Sync_EPON	     DS_1.25G       /  US_1.25G
// 0x06 = Async_XEPON    DS_10.3125G  /  US_1.25G
// 0x07 = Sync_XEPON      DS_10.3125G  /  US_10.3125G
// 0x09 = Async_XGPON    DS_9.95328G  /  US_2.48832G
// 0x0b = Sync_XGSPON   DS_9.95328G  /  US_9.95328G
// 0x0d = Sync_GPON       DS_2.48832G  /  US_2.48832G
//*************************************************/	

//digital mode
//10p3125G
//CK for MAC layer

switch(scu_bit_sel)
	{
	case scu_Sync_XEPON: // Sync_XEPON(10G)      
         IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x7); //XEPON 10G 10G mode
         
 #if LAB_PRINT_PON
         printk("=== XPON_DIG_Sync_XEPON(10G)_done ===\n");
 #endif		 	
         break;	


 	case scu_Async_XEPON: // Async_XEPON(10G)      
         IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x6); //XEPON 10G 1G mode
         
 #if LAB_PRINT_PON
         printk("=== XPON_DIG_Async_XEPON(10G)_done ===\n");
 #endif		 	
         break;	


    case scu_Sync_XGSPON: // Sync_XGSPON(9G)       
         //IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x9); //XGPON 9G 9G mode
         IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0xa); //XGPON 9G 9G mode_Jay

 #if LAB_PRINT_PON
         printk("=== XPON_DIG_Sync_XGSPON(9G)_done ===\n");
 #endif		 
         break;	


     case scu_Async_XGPON: // Sync_XGSPON(9G)    
         IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x9); //XGPON 9G 2G mode_Jay

 #if LAB_PRINT_PON
         printk("=== XPON_DIG_Async_XGPON(9G)_done ===\n");
 #endif		 
         break;	
 
 

	case scu_Sync_GPON: // Sync_GPON(2G)		
		 //IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x0); //GPON 2.5G 2.5G mode
		 IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0xd); //GPON 2.5G 2.5G mode_Jay

 #if LAB_PRINT_PON
		 printk("=== XPON_DIG_Sync_GPON(2G)_done ===\n");
 #endif		  
		 break;	
 

	   case scu_Async_GPON: // Async_GPON(2G)		
			IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x0); //GPON 2.5G 1G mode
			
#if LAB_PRINT_PON
			printk("=== XPON_DIG_Async_GPON(2G)_done ===\n");
#endif			   
			break; 

 

	case scu_Sync_EPON: // Sync_EPON(1G)
		IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0x8); //EPON 1.25G 1.25G mode

 #if LAB_PRINT_PON
		 printk("=== XPON_DIG_Sync_EPON(1G)_done ===\n");
 #endif		 
		 break;	


 	case Sync_XGSPON_2: // NGPON2_10G_10G
		IO_SPHYA_REG_BITS(0xBFB00070, 7, 0, 0xb); //EPON 1.25G 1.25G mode

 #if LAB_PRINT_PON
		 printk("=== XPON_DIG_Sync_NGPON2_done ===\n");
 #endif		 
		 break;	

	
		default:
			break;	
	 }

}




void setting_mode_XPON_TXPLL(uint rate_sel)
{

switch(rate_sel)
	{
	
	case 10: // 10G_ mode    
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2, 31,  0, 0x08400000);   //DA_XPON_PLL_SDM_PCW[30:0] 		 
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1, 31,  0, 0x21000000);   //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2, 31,  0, 0x21000000);   //rg_lcpll_pon_hrdds_pcw_ncpo_epon

         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8,  8, 0x1);     	 //rg_lcpll_ncpo_load

 #if LAB_PRINT_PON
         printk("=== XPON_TXPLL_10G_done ===\n");
 #endif
		  
		 break;  
		 
    case 9: // 9G_ mode
	     IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2, 31, 0, 0x07F67620);   //DA_XPON_PLL_SDM_PCW[30:0]    
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1, 31, 0, 0x1FD9D880);   //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2, 31, 0, 0x1FD9D880);   //rg_lcpll_pon_hrdds_pcw_ncpo_epon
		
		 IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8, 8, 0x1);     	 //rg_lcpll_ncpo_load
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_9G_done ===\n");
 #endif
		   
		 break;	

    case 2: // 2G_ mode
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2, 31, 0, 0x07F67620);   //DA_XPON_PLL_SDM_PCW[30:0]  
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1, 31, 0, 0x1FD9D880);   //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2, 31, 0, 0x1FD9D880);   //rg_lcpll_pon_hrdds_pcw_ncpo_epon
		
		 IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8, 8, 0x1);     	 //rg_lcpll_ncpo_load
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_2G_done ===\n");
 #endif
		 
		 break;	

	case 1: // 1G_ mode
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2, 31, 0, 0x08000000);   //DA_XPON_PLL_SDM_PCW[30:0]  
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1, 31, 0, 0x20000000);   //rg_lcpll_pon_hrdds_pcw_ncpo_gpon
         IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2, 31, 0, 0x20000000);   //rg_lcpll_pon_hrdds_pcw_ncpo_epon
		
		 IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8, 8, 0x1);     	 //rg_lcpll_ncpo_load
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_1G_done ===\n");
 #endif
		 
		 break;	
		
		default:
		    break;	
     }		 

}



void setting_mode_XPON_TX(uint tx_bit_sel)
{

switch(tx_bit_sel)
	{

	case TX_Async_GPON: //DS_2.48832G  /  US_1.24416G
	   
			//TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x2);  //RG_XPON_TX_CKSEL[2:0]
	
			//TX bus width setting
			//IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x2);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
			IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x0);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  5,  4, 0x0);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b			
			
			
			//TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b1
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_GPON_done(Async_GPON) ===\n");
#endif
			
			break; 


    case TX_Async_XEPON: //DS_10.3125G  /  US_1.25G		
			
			//TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x1);   //RG_XPON_TX_CKSEL[2:0]
	
			//TX bus width setting
			IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL, 1,  0, 0x1);	 //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 5,  4, 0x1);	 //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
			
			//TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);	//rg_r2t_mode , 1b1
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_XEPON_done(Async_XEPON) ===\n");
#endif
			
			break; 



	
	case TX_Sync_XEPON: //DS_10.3125G  /  US_10.3125G    
	
         //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x5);  //RG_XPON_TX_CKSEL[2:0]

         //TX bus width setting
         IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x2);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  5,  4, 0x2);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
         
         //TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b1

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_XEPON_done(Sync_XEPON) ===\n");
 #endif
		 
         break; 


    case TX_Async_XGPON: //DS_9.95328G  /  US_2.48832G			
			
			//TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x3);   //RG_XPON_TX_CKSEL[2:0]
	
			//TX bus width setting
			IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x0);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  5,  4, 0x0);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
			
			//TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);	//rg_r2t_mode , 1b1
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_Async_XGPON_done(Async_XGPON) ===\n");
#endif
			 
			break; 

 

	case TX_Sync_XGSPON: //DS_9.95328G  /  US_9.95328G 
        
         //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x5); //RG_XPON_TX_CKSEL[2:0]

         //TX bus width setting
         IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x2);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  5,  4, 0x2);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
         
         //TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b1

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_XGSPON_done(Sync_XGSPON) ===\n");
 #endif
		  
         break; 
		 
	case TX_Sync_GPON: //DS_2.48832G  /  US_2.48832G  	     
		 
         //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x3);   //RG_XPON_TX_CKSEL[2:0]

         //TX bus width setting
         IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL,  1,  0, 0x0);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  5,  4, 0x0);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
         
         //TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b1

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_GPON_done(Sync_GPON) ===\n");
 #endif
		  
         break; 

	case TX_Sync_EPON: //DS_1.25G      /  US_1.25G 	     
		 
         //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 12, 10, 0x2);   //RG_XPON_TX_CKSEL[2:0]

         //TX bus width setting
         IO_SPHYA_REG_BITS(XPON_PMA_XPON_TX_RATE_CTRL, 1,  0, 0x1);   //rg_pon_tx_rate_ctrl: 0,1,2 => 8b/10b/16b
         IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0, 5,  4, 0x1);   //RG_XPON_TX_RATE_CTRL[1:0]: 0,1,2 => 8b/10b/16b
         
         //TX input mux sel, {r2t_mode, tx_bist_gen_en}: 0,1,2,3: normal, from_RX, from_BIST
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0); //rg_tx_bist_gen_en
         IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b1

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_EPON_done(Sync_EPON) ===\n");
 #endif
		 
         break; 	 
		 
	     default:
		    break;	
	}	


}


void setting_mode_XPON_RX(uint rx_bit_sel)
{

  switch(rx_bit_sel)
	{
	case 10: // 10G_ mode 
	
//RX OSR setting
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x0); //RG_XPON_CDR_LPF_RATIO: 0,1,2,3 => /1, /2, /4, /8

//10p3125G
//RX bus width setting
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,  1, 0, 0x2);   //rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b

//RX output mux sel	
	 IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 2, 0, 0x0);		//rg_xpon_mode: 0,1,2,3,4 => 16b(/1), 8b(/4), 16b(/4), 10b(/8), digital_T2R

#if LAB_PRINT_PON
	 printk("=== XPON_RX_10G_done ===\n");
#endif
	 
     break; 


   case 9: // 9G_ mode
	 
//RX OSR setting
	 IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x0); //RG_XPON_CDR_LPF_RATIO: 0,1,2,3 => /1, /2, /4, /8
	 
//9p95328G
//RX bus width setting
	 IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x2);	 //rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b
	 
//RX output mux sel	 
	 IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 2, 0, 0x0);	 //rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b

#if LAB_PRINT_PON
	 printk("=== XPON_RX_9G_done ===\n");
#endif
	 
	 break; 


	case 2: // 2G_ mode 	 
		  
//RX OSR setting
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x2); //RG_XPON_CDR_LPF_RATIO: 0,1,2,3 => /1, /2, /4, /8
		  
//2p48832G
//RX bus width setting
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x2); 	   //rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b
		  
//Ext. T2R
//RX output mux sel
   IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 2, 0, 0x2);   //rg_xpon_mode: 0,1,2,3,4 => 16b(/1), 8b(/4), 16b(/4), 10b(/8), digital_T2R

#if LAB_PRINT_PON
     printk("=== XPON_RX_2G_done ===\n");
#endif
     break; 


   case 1: // 1G_ mode	
			   
//RX OSR setting
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x3); //RG_XPON_CDR_LPF_RATIO: 0,1,2,3 => /1, /2, /4, /8
			   
//1p25G
//RX bus width setting    
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1, 1, 0, 0x1); 	//rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b
			   
//RX output mux sel    
     IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 2, 0, 0x3);		//rg_xpon_rx_rate_ctrl: 0,1,2 => 8b/10b/16b

#if LAB_PRINT_PON
     printk("=== XPON_RX_1G_done ===\n");
#endif

	 break; 	

	 default:
		break;	
	}

}


void setting_XPON_DIG(void)
{
//digital setting 
//================================
//from ==> JCPLL Setting
//================================
  IO_SPHYA_REG_BITS(XFI_PMA_JCPLL_DA_RG_CTRL_2,  0,  0, 0x1);   //jcpll_digtcl_dac_en_hw_mode
  IO_SPHYA_REG_BITS(XFI_PMA_JCPLL_DA_RG_CTRL_2,  1,  1, 0x1);   //jcpll_chpup_en_hw_mode
  IO_SPHYA_REG_BITS(XFI_PMA_JCPLL_DA_RG_CTRL_2,  3,  3, 0x1);   //jcpll_ldochp_en_hw_mode
  IO_SPHYA_REG_BITS(XFI_PMA_JCPLL_DA_RG_CTRL_2,  4,  4, 0x1);   //jcpll_auto_k_load_hw_mode
  IO_SPHYA_REG_BITS(XFI_PMA_JCPLL_DA_RG_CTRL_2,  5,  5, 0x1);   //xfi_xtalck_cgm_en_hw_mode

  IO_SPHYA_REG_BITS(XFI_PMA_SS_LCPLL_PWCTL_SETTING_1, 15,  8, 0xD);   //rg_xfi_lcpll_en_timer[7:0]                                              
  IO_SPHYA_REG_BITS(XFI_PMA_SS_LCPLL_PWCTL_SETTING_1, 23, 16, 0x14); //rg_xfi_lcpll_ck_stb_timer[7:0]	
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_SS_JCPLL_PWCTL_SETTING_1, 15, 00, 0x13BA); //rg_jcpll_en_timer
  IO_SPHYA_REG_BITS(XFI_PMA_SS_JCPLL_PWCTL_SETTING_1, 23, 16, 0x14);   //rg_jcpll_pcw_man_load_timer                                                              
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_PLL_STB_CNT, 15, 00, 0xFA0); //rg_xfi_pll_stb_nt_0
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_PLL_STB_CNT, 31, 16, 0x0);   //rg_xfi_pll_stb_nt_1                                                             
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_PLL_STOP_CNT, 15, 00, 0x109A);//rg_xfi_pll_stop_cnt_0
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_PLL_STOP_CNT, 31, 16, 0x0);   //rg_xfi_pll_stop_cnt_1                                                              
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_1, 15, 00, 0xFA);   //rg_tc_ck_en_wait
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_1, 31, 16, 0x113);  //rg_tx_hsdata_en_wait                                                              
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_2, 15, 00, 0x210);  //rg_tx_power_on_wait
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_2, 31, 16, 0x9B);   //rg_tx_serdes_rdy_wait                                                              
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_0, 15, 00, 0x26);  //rg_txcakib_50us
  IO_SPHYA_REG_BITS(XFI_PMA_XFI_TX_COUNTER_0, 31, 16, 0x4);   //rg_txcalib_5us

// TDC
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  1,  0, 0x2);   //rg_lcpll_ncpo_shift 	// by david 20190412
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1, 11,  8, lcpll_a_tdc);  //rg_lcpll_a_tdc	// by david 20190412
  //322C, rg_lcpll_a_tdc  real TDC bandwidth is 2 times of the value in table below
							  // TDC Bandwidth Table: 
							  // BW: 160k 	0d'08
							  // BW: 140k 	0d'09
							  // BW: 110k 	0d'01
							  // BW: 84k  	0d'10
							  // BW: 69k  	0d'11
							  // BW: 53k  	0d'02
							  // BW: 39k  	0d'12
							  // BW: 33k  	0d'13
							  // BW: 25k  	0d'03
							  // BW: 19k  	0d'14
							  // BW: 12k  	0d'04
							  // BW: 9.5k 	0d'15
							  // BW: 6.1k 	0d'05
							  // BW: 3k   	0d'06
							  // BW: 1.5k 	0d'07		  
  
// **pma_dig_tx_setting ** //  
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_1, 31, 16, 0x113);//rg_tx_hsdata_en_wait
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_1, 15, 00, 0xFA); //rg_tx_ck_en_wait
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_2, 31, 16, 0x9B); //rg_tx_serdes_rdy_wait
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_2, 15, 00, 0x210);//rg_tx_power_on_wait
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_0, 31, 16, 0x4);  //rg_txcalib_5us
  IO_SPHYA_REG_BITS(XPON_PMA_PON_TX_COUNTER_0, 15, 00, 0x26); //rg_txcalib_50us
                                                   
  IO_SPHYA_REG_BITS(XPON_PMA_TX_DLY_CTRL, 23, 16, 0x2);  //rg_tx_ben_exten_ftune
                                                   
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_0, 10,  8, 0x3);   //rg_lcpll_ki                                                   
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5, 24, 24, 0x0);    //rg_lcpll_tdc_sync_in_mode

//PLL EN HW Mode 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 31, 24, 0x1);  //rg_lcpll_ck_stb_timer
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 23, 16, 0x1);  //rg_lcpll_pcw_man_load_timer
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 15,  8, 0xA);   //rg_lcpll_en_timer
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_1,  0,  0, 0x1);   //rg_lcpll_man_pwdb 
   
// **pma_dig_rx_setting ** //
//RX HW mode counter
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 15,  0, 0x2);     //rg_rx_pical_start
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 31, 16, 0x3E8);  //rg_rx_pical_end  
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 15,  0, 0x2);     //rg_rx_sdcal_start
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 31, 16, 0x3E8);  //rg_rx_sdcal_end
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 15,  0, 0x2);     //rg_rx_pdos_start
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 31, 16, 0x3E8);  //rg_rx_pdos_end
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 15,  0, 0x2);    //rg_rx_feos_start
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 31, 16, 0x3E8); //rg_rx_feos_end 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0, 23,  8, 0x1);  //rg_rx_os_start
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,  2,  0, 0x1);   //rg_osc_speed_opt
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6, 15,  0, 0x2);  //rg_rx_os_end    
//END RX Hw mode counter
//------------------------------------------

//RX setting
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,  1,  0, 0x3);   //rg_a_sel  
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2, 12,  8, 0x5);   //rg_fom_num_order[4:0]
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240);   //rg_x_max
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10,  0, 0x1C0);   //rg_x_min   
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,  0,  0, 0x1);   //rg_eyecnt_fast
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,  8,  8, 0x0);   //rg_data_shift

//EYE
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);   //rg_eye_nextpts 	                                                          	
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15,  8, 0x4);   //rg_eyecnt_vth
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,  7,  0, 0x4);   //rg_eyecnt_hth                                                               	
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10,  0, 0x4);   //rg_eo_hth                                                 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);   //rg_eye_mask 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,  9,  0, 0xD0);   //rg_cntlen
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,  7,  0, 0x80);   //rg_eq_en_delay
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0, 18,  8, 0x0);   //rg_heo_mask
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0, 31, 24, 0x0);   //rg_veo_mask
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,  0,  0, 0x1);   //rg_a_lgain
	                                                             	  
//(Default)	
  IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,  8,  8, 0x0);   //rg_eye_blwc_add
  IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,  0,  0, 0x1);   //rg_data_blwc_add                                                              	
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1, 25, 24, 0x1);   //rg_cal_cyc
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1, 15,  8, 0x2E);   //rg_cal_1us_set
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,  0,  0, 0x1);   //rg_sim_fast_sim
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1, 17, 16, 0x1);   //rg_cal_stb 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,  0,  0, 0x0);   //rg_cal_os_pulse
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2, 11,  8, 0x0);   //rg_cal_out_os
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2, 17, 16, 0x1);   //rg_cal_cyc_time

  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 15, 00, 0x4);   //rg_rx_blwc_rdy_en
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 31, 16, 0xC8);   //rg_rx_rdy
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,  7,  0, 0x0);   //rg_lfsel
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,  8,  8, 0x0);   //rg_eq_force_blwc_freeze 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 11,  8, 0xA);   //rg_eq_blwc_gain
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,  0,  0, 0x1);   //rg_eq_blwc_pol
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 22, 16, 0x10);   //rg_eq_blwc_cnt_top_lim
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 29, 23, 0x70);   //rg_eq_blwc_cnt_bot_lim 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,  6,  0, 0x40);   //rg_y_min
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 14,  8, 0x3F);   //rg_y_max
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x1);   //rg_index_mode	
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19,  0, 0x18);   //rg_eyedur    
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EXTRAL_CTRL, 15,  8, 0x2);   //rg_l2d_trig_eq_en_time
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EXTRAL_CTRL,  1,  1, 0x1);   //rg_os_rdy_latch
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EXTRAL_CTRL,  0,  0, 0x1);   //rg_disb_leq
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 10,  8, 0x1);   //rg_kpgain
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 17, 16, 0x0);   //rg_eq_pi_cal_rdy_dly
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_0, 18,  8, 0xA5);   //rg_fpkdiv
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_0, 2,   0, 0x3);   //rg_kband_prediv
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_0, 25, 24, 0x0);   //rg_kband_kfc
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_1, 26, 24, 0x4);   //rg_symbol_wd
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_1, 18, 16, 0x1);   //rg_settle_time_sel                                                           
                                            
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_5, 10,  0, 0x1FF);   //rg_fll_idac_max                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_5, 26, 16, 0x400);   //rg_fll_idac_min	
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_2, 10,  8, 0x4);   //rg_amp
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_2,  2,  0, 0x3);   //rg_prbs_sel
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4, 24, 24, 0x1);   //rg_disable_blwc_offset   
  IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,  0,  0, 0x1);   //rg_data_blwc_add, 1490/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,  8,  8, 0x0);   //rg_eye_blwc_add, 1490/0          
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,  0,  0, 0x0);   //rg_eq_blwc_pol               
                                        
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 11,  8, 0xA);   //rg_eq_blwc_gain 
                               
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 22, 16, 0x10);   //rg_eq_blwc_cnt_top_lim, +16
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC, 29, 23, 0x70);   //rg_eq_blwc_cnt_bot_lim, -16
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,  8,  8, 0x0);   //rg_eq_force_blwc_freeze

#if LAB_PRINT_PON
   printk("=== XPON_Dig_done ===\n");
#endif

}


void setting_XPON_ANA(uint rate_sel)
{
//analog setting 
//================================
//JCPLL Setting
//================================
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0, 27, 24, 0x0); //RG_JCPLL_IOFF
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0, 17, 15, 0x5); //RG_JCPLL_BR[2:0]
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0, 14, 13, 0x3); //RG_JCPLL_RSTDLY[1:0]
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0, 11, 11, 0x0); //RG_JCPLL_SHLPF_EN
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0, 10, 10, 0x1); //RG_JCPLL_LDOCHP_EN
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,  9,  9, 0x1);   //RG_JCPLL_CHPUP_EN
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0, 18, 17, 0x3); //RG_JCPLL_AUTOK_KF
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0, 16, 15, 0x3); //RG_JCPLL_AUTOK_KS
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0, 14, 13, 0x0); //RG_JCPLL_AUTOK_PREDIV[1:0]
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,  4,  4, 0x0); //RG_JCPLL_SDM_IFM                                                               
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0, 12,  5, 0x0);   //RG_JCPLL_TDC_FBKDIV
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0, 16, 16, 0x1);   //RG_JCPLL_DIGTCL_DAC_EN
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,  3,  3, 0x1);   //RG_JCPLL_MONVCO_SEL
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,  2,  2, 0x1);   //RG_JCPLL_MONVCO_EN
                                                 
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0, 31, 24, 0x0);   //RG_JCPLL_RESERVE0[7:0]
  IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0, 23, 16, 0xFF ); //RG_JCPLL_RESERVE1[7:0]

//================================
// XPON ANA setting
//================================
//XPON, CMN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0, 24, 22, 0x4);   //RG_XPON_CMN_VREFSEL[2:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0, 31, 31, 0x1);   //RG_XPON_CMN_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0, 30, 25, 0x1);   //RG_XPON_CMN_MPXSELTOP_DC[5:0]                                                                
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 22, 22, 0x1);   //RG_XPON_CLKPATH_LDO_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 30, 23, 0x21);   //RG_XPON_CLKPATH_DFM_DA_tmp[7:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 21, 21, 0x0);   //RG_XPON_CLKPATH_LDO_VOD_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 31, 31, 0x0);   //RG_XPON_CLKPATH_ACBUF_EN 
                                                 
//XPON, TDC  
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 20, 20, 0x1);   //RG_XPON_TDC_AUTOEN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 19, 19, 0x0);   //RG_XPON_TDC_FT_CK_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 18, 18, 0x1);   //RG_XPON_TDC_MON_RXCK_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 17, 17, 0x1);   //RG_XPON_TDC_MON_TXCK_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0, 16, 16, 0x0);   //RG_XPON_TDC_SYNC_CK_SEL

//XPON, TXPLL(LC type)
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 31, 29, 0x4);   //RG_XPON_PLL_TCLVAR_BIAS[2:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 28, 26, 0x4);   //RG_XPON_PLL_VCO_SCA_BIAS[2:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 25, 22, 0x8);   //RG_XPON_PLL_TCL_TEMPCODE[3:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 21, 21, 0x1);   //RG_XPON_PLL_FORCE_BYPSYNC_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 18, 17, 0x3);   //RG_XPON_PLL_AUTOK_KF[1:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 16, 15, 0x3);   //RG_XPON_PLL_AUTOK_KS[1:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,  4,  4, 0x1);   //RG_XPON_PLL_SDM_IFM
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0, 12, 5, 0xFF);   //RG_XPON_PLL_PLL_BAND_tmp[7:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0, 31, 30, 0x3);       //RG_XPON_PLL_SDN_ORD
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0, 16, 16, 0x1);       //RG_XPON_PLL_DIGTCL_DAC_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0, 12, 05, 0x80);      //RG_XPON_PLL_TDC_FBKDIV[7:0]   
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0, 30, 23, 0x0);      //RG_XPON_PLL_RESERVE0[7:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0, 22, 15, 0xFF);     //RG_XPON_PLL_RESERVE1[7:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 31, 28, 0x6);            //RG_XPON_PLL_IC[3:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 27, 24, 0x3);            //RG_XPON_PLL_IOFF[3:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 20, 18, 0x0);            //RG_XPON_PLL_BP[2:0]		
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 23, 21, 0x1);            //RG_XPON_PLL_BC
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 17, 15, 0x5);            //RG_XPON_PLL_BR
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0, 10, 10, 0x1);            //RG_XPON_PLL_LDOCHP_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,  9,  9, 0x1);            //RG_XPON_PLL_CHPUP_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,  8,  8, 0x1);            //RG_XPON_PLL_VCO_HALFLSB_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,  5,  5, 0x1);            //RG_XPON_PLL_TDCBUF_PDB
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,  4,  4, 0x1);            //RG_XPON_PLL_DIGTEST_CK_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,  3,  1, 0x4);            //RG_XPON_PLL_VCOVAR_BIAS
  IO_SPHYA_REG_BITS(XPON_PMA_XPON_PLL_STB_CNT, 15,  0, 0xC80);          //rg_xpon_pll_stb_cnt
  IO_SPHYA_REG_BITS(XPON_PMA_XPON_PLL_STOP_CNT, 15,  0, 0xD48);         //rg_xpon_pll_stop_cnt    

//XPON, BENTX
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 11, 11, 0x1);   //RG_XPON_BENTX_PRED_IMP
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 15, 12, 0x7);   //RG_XPON_BENTX_PRED_IBIAS[3:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 29, 24, 0x1F ); //RG_XPON_BENTX_DRV_IBIAS[5:0]
  #if !Power_Saving_PMA  //julia_20191120
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 16, 16, 0x1);   //RG_XPON_BENTX_PRED_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 30, 30, 0x1);   //RG_XPON_BENTX_DRV_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0, 31, 31, 0x1);   //RG_XPON_BENTX_BIAS_EN
  #endif 

//XPON, TX
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 31, 31, 0x1);   //RG_XPON_TX_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 30, 29, 0x2);   //RG_XPON_TX_SER_LOADSEL[1:0]
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,  5,  5, 0x1);   //RG_XPON_TX_LPEIDLE_EN   
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 31, 31, 0x0);  //RG_XPON_TX_LSDATA_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 30, 30, 0x0);  //RG_XPON_TX_LSDATA
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 17, 17, 0x0);  //RG_XPON_TX_TXLBRX_EN
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  9,  8, 0x2);   //RG_XPON_TX_CALI_VREF_H[1:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,  7,  6, 0x2);   //RG_XPON_TX_CALI_VREF_L[1:0]                                                 
                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 23, 18, 0x0);  //RG_XPON_TX_MPX_SEL[5:0]
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 25, 25, 0x0);  //RG_XPON_TX_MPX_OP_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 24, 24, 0x0);  //RG_XPON_TX_MPX_OP_BYPASS 

  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 10, 10, 0x1);         //RG_XPON_TX_FORCE_CX
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 29, 29, 0x0);  //RG_XPON_TX_FIR_CN1_INV
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 28, 28, 0x0);  //RG_XPON_TX_FIR_C0_INV
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 27, 27, 0x1);  //RG_XPON_TX_FIR_C1_INV
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0, 26, 26, 0x1);  //RG_XPON_TX_FIR_C2_INV 

if ((rate_sel == 10) || (rate_sel == 9))
  {

    //Lucas 6Inch TB_Best_for SE1
    //IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 27, 24, 0x1);   //RG_XPON_TX_FIR_CN1[3:0]
    //IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 23, 19, 0x0);   //RG_XPON_TX_FIR_INV_C0[4:0]
    //IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 18, 14, 0xD);   //RG_XPON_TX_FIR_C1[4:0]
    //IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 13, 11, 0x3);   //RG_XPON_TX_FIR_C2[2:0]

    //Lucas 6Inch TB_Best_for SE2
    IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 27, 24, 0x0);   //RG_XPON_TX_FIR_CN1[3:0]
    IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 23, 19, 0x0);   //RG_XPON_TX_FIR_INV_C0[4:0]
    IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 18, 14, 0xC);   //RG_XPON_TX_FIR_C1[4:0]
    IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 13, 11, 0x4);   //RG_XPON_TX_FIR_C2[2:0]

}
else
{
	//Lucas 6Inch TB_Best_for SE2
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 27, 24, 0x0);   //RG_XPON_TX_FIR_CN1[3:0]
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 23, 19, 0x0);   //RG_XPON_TX_FIR_INV_C0[4:0]
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 18, 14, 0x0);   //RG_XPON_TX_FIR_C1[4:0]
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0, 13, 11, 0x0);   //RG_XPON_TX_FIR_C2[2:0]
}

//XPON, RX
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 21, 21, 0x1);   //RG_XPON_RX_FE_VB_EQ3_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 22, 22, 0x1);   //RG_XPON_RX_FE_VB_EQ2_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 23, 23, 0x1);   //RG_XPON_RX_FE_VB_EQ1_EN
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 19, 17, 0x4);   //RG_XPON_RX_FE_VCM_SEL
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 20, 20, 0x1);   //RG_XPON_RX_FE_VCM_GEN_PWDB

//top, bot: (10,70) > (08,78) > (04, 7C) > (02, 7E) > (01,7F)
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 23, 05, 0x08000);   //RG_XPON_CDR_LPF_TOP_LIM, hugo
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0, 29, 11, 0x78000);   //RG_XPON_CDR_LPF_BOT_LIM, hugo
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0, 27, 27, 0x0);       //rg_xpon_cdr_pr_rstb_bypass
  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0, 16, 16, 0x0);       //rx_rev0

#if LAB_PRINT_PON
  printk("=== XPON_ANA_done ===\n");
#endif

}


void seq_XPON_DIG_reset(void)
{
	
//digital reset

	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 4, 0,	0x0);  //rg_xxx_rstn   

	//add by Jay_20181023
	IO_SPHYA_REG_BITS(XPON_PMA_TX_FIFO_MODE_SEL, 1, 1,	0x0); 
#if LAB_PRINT_PON	
	printk("====================seq_XPON_DIG_reset.=================\n");
#endif

	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 1, 1, 0x1);   //rg_sw_rx_rst_n
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 4, 4, 0x1);   //rg_sw_allpcs_rst_n
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 3, 3, 0x1);   //rg_sw_pma_rst_n
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 2, 2,	0x1);  //rg_sw_tx_rst_n
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 0, 0,	0x1);  //rg_sw_rx_fifo_rst_n

	//PATCH!
	IO_SPHYA_REG_BITS(XPON_PMA_TX_FIFO_MODE_SEL, 1, 1, 0x1);   //rg_tx_afifo_mode (ECO: rx_domain rst)

#if LAB_PRINT_PON
  printk("=== XPON_DIG_reset_done ===\n");
#endif

}


void seq_JCPLL_on(void)
{
// ** JCPLL_force_on ** //
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_1, 0,  0, 0x1);   //rg_xfi_lcpll_man_pwdb
udelay(5000);

#if LAB_PRINT_PON
   printk("=== XPON_JCPLL_On_done ===\n");
#endif

}


void seq_XPON_DIG_fm_on(int fm_bit_sel)
{

switch(fm_bit_sel)
	{

case fm_Sync_XEPON: //fm_10p3125G_10p3125G  

         //enable all freq meter
         //watch TX/RX/JCPLL
         //phya_tx_ck /div8 frequency meter setting
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x100);//rg_pll_unlock_cyclecnt
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x100);//rg_pll_lcok_cyclecnt  
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x9F); //rg_pll_lock_target_end
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x9E); //rg_pll_lock_target_beg                                                               
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x9F); //rg_pll_unlock_target_end
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x9E); //rg_pll_unlock_target_beg

         //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200);   //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200);   //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x27C);   //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x27B);   //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x27C);   //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x27B);   //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  );   //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  );   //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  );   //rg_freqlock_det_en
        
     break; 

	 

 case fm_Async_XEPON: //fm_10p3125G_1p25G  

         //enable all freq meter
         //watch TX/RX/JCPLL
         //phya_tx_ck /div8 frequency meter setting
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x30); //rg_pll_unlock_cyclecnt
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x30); //rg_pll_lcok_cyclecnt  
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x9A); //rg_pll_lock_target_end
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x99); //rg_pll_lock_target_beg                                                               
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x9A); //rg_pll_unlock_target_end
         IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x99); //rg_pll_unlock_target_beg

         //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200);   //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200);   //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x27C);   //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x27B);   //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x27C);   //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x27B);   //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  );   //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  );   //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  );   //rg_freqlock_det_en
        
     break; 
	 

case fm_Sync_XGSPON: //fm_9p95328G_9p95328G 
     
        //phya_tx_ck /div8 frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x100); //rg_pll_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x100); //rg_pll_lcok_cyclecnt  
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0xa5); //rg_pll_lock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0xa4); //rg_pll_lock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0xa5); //rg_pll_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0xa4); //rg_pll_unlock_target_beg

        //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200); //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200); //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x293); //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x292); //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x293); //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x292); //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  ); //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  ); //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  ); //rg_freqlock_det_en
 	 
     break; 
	 

case fm_Async_XGPON: //fm_9p95328G_2p48832G 
     
        //phya_tx_ck /div8 frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x70); //rg_pll_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x70); //rg_pll_lcok_cyclecnt  
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x91); //rg_pll_lock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x90); //rg_pll_lock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x91); //rg_pll_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x90); //rg_pll_unlock_target_beg

        //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200); //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200); //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x293); //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x292); //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x293); //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x27B); //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  ); //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  ); //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  ); //rg_freqlock_det_en
 	 
     break; 


case fm_Sync_GPON: //fm_2p48832G_2p48832G 
     
        //phya_tx_ck /div8 frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x70); //rg_pll_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x70); //rg_pll_lcok_cyclecnt  
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x91); //rg_pll_lock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x90); //rg_pll_lock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x91); //rg_pll_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x90); //rg_pll_unlock_target_beg

        //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200); //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200); //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x293); //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x292); //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x293); //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x292); //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  ); //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  ); //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  ); //rg_freqlock_det_en
 	 
     break; 


 case fm_Async_GPON: //fm_2p48832G_1p24416G 
     
        //phya_tx_ck /div8 frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x38); //rg_pll_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x38); //rg_pll_lcok_cyclecnt  
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x91); //rg_pll_lock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x90); //rg_pll_lock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x91); //rg_pll_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x90); //rg_pll_unlock_target_beg

        //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200); //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200); //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x293); //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x292); //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x293); //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x292); //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  ); //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  ); //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  ); //rg_freqlock_det_en
 	 
     break; 


case fm_Sync_EPON: //fm_1p25G_1p25G 	

        //phya_tx_ck /div8 frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 31, 16, 0x30); //rg_pll_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0, 15, 00, 0x30); //rg_pll_lcok_cyclecnt  
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 31, 16, 0x9A); //rg_pll_lock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1, 15, 00, 0x99); //rg_pll_lock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 15, 12, 0x8);  //rg_pll_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3, 11,  8, 0x3);  //rg_pll_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,  2,  0, 0x7);  //rg_pll_freqlock_det_en                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 31, 16, 0x9A); //rg_pll_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2, 15, 00, 0x99); //rg_pll_unlock_target_beg

        //END phya_tx_ck /div8 frequency meter setting

        //-------------------------------------------
        //CDR_FB_CK frequency meter setting
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x200);   //rg_unlock_cyclecnt
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1, 15, 00, 0x200);   //rg_lock_cyclecnt   
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x290);   //rg_lock_target_beg
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2, 15, 00, 0x28F);   //rg_lock_target_end                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x290);   //rg_unlock_target_end
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3, 15, 00, 0x28F);   //rg_unlock_target_beg                                                               
                                                 
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 15, 12, 0x8  );   //rg_unlockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 11, 8,  0x1  );   //rg_lock_lockth
        IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4, 2,  0,  0x7  );   //rg_freqlock_det_en
 	 
     break; 


default:
	   break;  

	}  
	 


#if LAB_PRINT_PON
   printk("=== XPON_FM_On_done ===\n");
#endif

}


void seq_XPON_TXPLL_on(void)
{
// ** LCPLL_force_on ** //  
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0, 24, 24, 0x1);   //rg_sw_pll_en
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,  0,  0, 0x1);   //rg_lcpll_force_on
udelay(5000);

#if LAB_PRINT_PON
   printk("=== XPON_TXPLL_On_done ===\n");
#endif

}


void seq_XPON_TX_on(void)
{
  UINT32 read_data;
// ** TX_on ** //
//controlled by DA_TX_CK_EN or DA_TX_HSDATA_EN
  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B, 0,  0, 0x1);   //tx_top_rst_b
//wait_ms(1);

//3750 , CLK PATH EN
//3750 , CLK PATH EN                               
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,  8,  8, 0x1);   //rg_clkpath_rstb_ck
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,  0,  0, 0x1);   //rg_clkpath_rst_en
                                              
//3550 , TX CAL RST
  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,  8,  8, 0x1);   //txclib_rst_b
  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,  0,  0, 0x1);   //tx_top_rst_b
                                                   
//3554, TX CAL                                     

 //********************************************OPTIMIZE TX IMPEDANCE by EFUSE*****************************************************//
#if defined(TCSUPPORT_CPU_EN7580)
	  PON_PHY_PRINT(PHY_MSG_ERR,"EN7580_EFUSE: %x\n", EN7580_EFUSE);
	#if EN7580_EFUSE
		  //-------TX R50 Calibration load from EFUSE-------//
		  // if EFUSE is zero, default set TX R50 impedance to level 2 (0x2)
			  read_data = get_efuse_data(DA_XPON_TX_TERMP_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 25, 24, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"DA_XPON_TX_TERMP_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 25, 24, 0x2);
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set TXP Impedance Level 2 as default! \n");
			  }
			  read_data = get_efuse_data(DA_XPON_TX_TERMN_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1, 25, 24, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"DA_XPON_TX_TERMN_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1, 25, 24, 0x2);
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set TXN Impedance Level 2 as default! \n");
			  }
	#else
		  //-------TX R50 Calibration use default value-------//
			  PON_PHY_PRINT(PHY_MSG_ERR,"TX R50 Calibration use default value. \n");
			  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 25, 24, 0x2);
			  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1, 25, 24, 0x2);	  
	#endif
			  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 16, 16, 0x1);	//force P enable, by ang_20190820
			  IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1, 16, 16, 0x1);	//force N enable, by ang_20190820	  
#endif

//*****************************************************************************************************************************//


  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 25, 24, 0x2);   //rg_txcalib_force_termp_sel
  
  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0, 16, 16, 0x1);   //force 50om, by Morris_20190820
                                                   
  //udelay(50);
  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,  0,  0, 0x1);   //rg_txcalib_p_en ,1b1
  //udelay(50);
  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,  0,  0, 0x0);   //rg_txcalib_p_en ,1b0
  
  //udelay(50);                                                 
  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,  0,  0, 0x1);   //rg_txcalib_n_en ,1b1
  //udelay(50);
  //IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,  0,  0, 0x0);   //rg_txcalib_n_en ,1b0 
  //udelay(50);

#if LAB_PRINT_PON
   printk("=== XPON_TX_On_done ===\n");
#endif

}


void seq_XPON_RX_preset(void)
{
// ** RX_precondition ** //
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,  8,  8, 0x0);  //rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,  8,  8, 0x0);   //rg_disb_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24, 24, 0x0);  //rg_force_rx_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24, 0x0);   //rg_disb_rx_rdy
                                                 
//L2R
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 16, 16, 0x0);  //rg_force_da_xpon_cdr_lpf_lck2data
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x0);  //rg_force_da_xpon_cdr_lpf_rstb
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 16, 16, 0x0);   //rg_disb_da_xpon_cdr_lpf_lck2data
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 24, 24, 0x0);   //rg_disb_da_xpon_cdr_lpf_rstb
                                                 
//LEQ setting
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,  1,  0, 0x1);  //rg_force_da_xpon_rx_fe_gain_ctrl
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 10,  8, 0x3);  //rg_force_da_xpon_rx_fe_peaking_ctrl  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,  0,  0, 0x0);   //rg_disb_da_xpon_rx_fe_gain_ctrl
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,  8,  8, 0x0);   //rg_disb_da_xpon_rx_fe_peaking_ctrl
                                                 
//keep EYE reset                          
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9,  8,  8, 0x1);  //rg_force_eye_reset_plu_o, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_8,  8,  8, 0x0);   //rg_disb_eye_reset_plu_o, 1'b0                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x0);  //rg_force_eye_top_en, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x0);   //rg_disb_eye_top_en, 1'b0

//keep BLWC reset
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,  0,  0, 0x0);       // rg_eq_blwc_rst_b, 1494/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,  0,  0, 0x0);   // rg_disb_blwc_rx_rst_b, 1674/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,  0,  0, 0x0);  // rg_force_blwc_rx_rst_b, 1668/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16, 16, 0x0);    //rg_disb_rx_blwc_en
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 16, 16, 0x0);   //rg_force_rx_blwc_en

#if LAB_PRINT_PON
   printk("=== XPON_RX_Preset_done ===\n");
#endif

}


void aux_XPON_TDC_off(void)
{
// ** TDC ** //
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8,  8, 0x1);  //PCW from rg
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,  0,  0, 0x0);   //rg_lcpll_tdc_dig_pwdb     
udelay(1000);

#if LAB_PRINT_PON
   printk("=== XPON_TDC_Off_done ===\n");
#endif

}


void seq_XPON_RX_on(void)
{
// ** RX_on ** // 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  8,  8, 0x1);   //rg_da_xpon_cdr_pw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  0,  0, 0x1);   //rg_da_xpon_rx_fw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 24, 24, 0x1);   //rg_da_xpon_cdr_pd_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 16, 16, 0x1);   //rg_da_xpon_cdr_pr_pieye_pwdb
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,  0,  0, 0x1);   //rg_da_xpon_rx_sigdet_pwdb
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  8,  8, 0x0);   //rg_da_xpon_cdr_pw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  0,  0, 0x0);   //rg_da_xpon_rx_fw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 24, 24, 0x0);   //rg_da_xpon_cdr_pd_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 16, 16, 0x0);   //rg_da_xpon_cdr_pr_pieye_pwdb
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,  0,  0, 0x0);   //rg_da_xpon_rx_sigdet_pwdb
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  8,  8, 0x1);   //rg_da_xpon_cdr_pw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,  0,  0, 0x1);   //rg_da_xpon_rx_fw_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 24, 24, 0x1);   //rg_da_xpon_cdr_pd_pwdb 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0, 16, 16, 0x1);   //rg_da_xpon_cdr_pr_pieye_pwdb
  IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,  0,  0, 0x1);   //rg_da_xpon_rx_sigdet_pwdb
udelay(500);

#if LAB_PRINT_PON
   printk("=== XPON_RX_On_done ===\n");
#endif

}


void seq_XPON_RX_L2R(void)
{
//L2R 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 16, 16, 0x0);  //rg_force_da_xpon_cdr_lpf_lck2data
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x0);  //rg_force_da_xpon_cdr_lpf_rstb
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 16, 16, 0x0);   //rg_disb_da_xpon_cdr_lpf_lck2data
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 24, 24, 0x0);   //rg_disb_da_xpon_cdr_lpf_rstb
udelay(100);

#if LAB_PRINT_PON
   printk("=== XPON_RX_L2R_done ===\n");
#endif

}


void seq_XPON_RX_pical(void)
{
//PICAL
//*** pre-condition
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2, 24, 24, 0x1);  //rg_disb_da_xpon_cdr_pr_pieye
                                                 
//reset block                                    
  IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x8);     //rg_eq_en_delay
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x4);     //rg_kpgain
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0, 16, 16, 0x0);      //rg_eq_pi_cal_rst_b, 1'b0                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x0);   //rg_force_rx_and_pical_rstb, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6, 8, 8, 0x0);    //rg_disb_rx_and_pical_rstb, 1'b0                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x0);   //rg_force_ref_and_pical_rstb, 1'b0  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6, 0, 0, 0x0);    //rg_disb_ref_and_pical_rstb, 1'b0
                                                  
//enable                                          
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,  8,  8, 0x0);  //rg_force_rx_or_pical_en, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5, 24, 24, 0x0);   //rg_disb_rx_or_pical_en, 1'b0                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  8,  8, 0x0);  //rg_force_rx_pical_en, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,  8,  8, 0x0);   //rg_disb_rx_pical_en, 1'b0                                                   
                                                  
//release reset & enable                          
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0, 16, 16, 0x1);      //rg_eq_pi_cal_rst_b, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 8, 8, 0x1);   //rg_force_rx_and_pical_rstb, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 0, 0, 0x1);   //rg_force_ref_and_pical_rstb, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x1);   //rg_force_rx_or_pical_en, 1'b1
udelay(200);

  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x0);   //rg_force_rx_or_pical_en, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x0);    //rg_disb_eq_pi_cal_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x1);   //rg_force_eq_pi_cal_rdy

#if LAB_PRINT_PON
   printk("=== XPON_RX_Pical_done ===\n");
#endif

}


void seq_XPON_RX_pdos(void)
{
//PDOS
//*** pre-condition
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,  8,  8, 0x0);   //rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,  8,  8, 0x0);   //rg_disb_rx_os_rdy                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,  8,  8, 0x1);   //rg_disb_da_xpon_rx_dac_d0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1, 16, 16, 0x1);   //rg_disb_da_xpon_rx_dac_d1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1, 24, 24, 0x1);   //rg_disb_da_xpon_rx_dac_e0          
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,  0,  0, 0x1);   //rg_disb_da_xpon_rx_dac_e1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,  8,  8, 0x1);   //rg_disb_da_xpon_rx_dac_eye                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,  0,  0, 0x0);  //rg_force_blwc_rx_rst_b, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,  0,  0, 0x0);   //rg_disb_blwc_rx_rst_b, 1'b0                                                 
  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x0);   //rg_force_eyedur_init_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1,  8,  8, 0x0);   //rg_disb_eyedur_init_b, 1'b0                                                  
  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8, 24, 24, 0x0);   //rg_force_eyecnt_rx_rst_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x0);   //rg_disb_eyecnt_rx_rst_b, 1'b0                                                  
  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x0);   //rg_force_eyedur_en, 1'b0  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1,  0,  0, 0x0);   //rg_disb_eyedur_en, 1'b0 

//*** setting
  IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0, 18, 16, 0x2);   //rg_sap_sel 

//*** seq
//reset
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 16, 16, 0x0);   //rg_force_pdos_rx_rst_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6, 16, 16, 0x0);    //rg_disb_pdos_rx_rst_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,  0,  0, 0x0);        //rg_pdos_rst_b, 1'b0 
                                                 
//disable                                        
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x0);   //rg_force_rx_pdos_en, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16, 16, 0x0);   //rg_disb_rx_pdos_en, 1'b0 
                                                 
//release reset & enable                         
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  0,  0, 0x1);   //rg_force_rx_os_en, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,  0,  0, 0x0);    //rg_disb_rx_os_en, 1'b0 
                                              
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7, 16, 16, 0x1);   //rg_force_pdos_rx_rst_b, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,  0,  0, 0x1);        //rg_pdos_rst_b, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x1);   //rg_force_rx_pdos_en, 1'b1
udelay(200);

                                                 
//disable                                        
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16, 16, 0x0);   //rg_force_rx_pdos_en, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  0,  0, 0x0);   //rg_force_rx_os_en, 1'b0

//release eye related
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x0);	 //rg_force_eyedur_init_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1,  8,  8, 0x1);	 //rg_disb_eyedur_init_b, 1'b0	  

  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8, 24, 24, 0x0);	//rg_force_eyecnt_rx_rst_b, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x1);   //rg_disb_eyecnt_rx_rst_b, 1'b0	 

  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x0);	 //rg_force_eyedur_en, 1'b0  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1,  0,  0, 0x1);	 //rg_disb_eyedur_en, 1'b0 
  udelay(200);

#if LAB_PRINT_PON
   printk("=== XPON_RX_PDOS_done ===\n");
#endif

}


void seq_XPON_RX_feos(void)
{
//FEOS
//*** pre-condition
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,  8,  8, 0x0);   //rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,  8,  8, 0x0);    //rg_disb_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2, 16, 16, 0x1);  //rg_disb_da_xpon_rx_fe_vos                                                 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,  0,  0, 0x0); //rg_force_blwc_rx_rst_b, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,  0,  0, 0x0);  //rg_disb_blwc_rx_rst_b, 1'b0  

//*** setting
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,  7,  0, 0x30);   //rg_lfsel


//*** seq
//reset
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8, 16, 16, 0x0); //rg_force_feos_rx_rst_b, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7, 16, 16, 0x0);  //rg_disb_feos_rx_rst_b, 1'b0                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,  8,  8, 0x0);      //rg_feos_rst_b, 1'b0
                                                  
//disable         
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x0); //rg_force_rx_feos_en, 1'b0 
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24, 24, 0x0);  //rg_disb_rx_feos_en, 1'b0

//release reset & enable
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  0,  0, 0x1); //rg_force_rx_os_en, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,  0,  0, 0x0);  //rg_disb_rx_os_en, 1'b0                                                  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8, 16, 16, 0x1);               //rg_force_feos_rx_rst_b, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,  8,  8, 0x1);                    //rg_feos_rst_b, 1'b1
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x1); //rg_force_rx_feos_en, 1'b1
udelay(200);


//disable                                        
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24, 24, 0x0); //rg_force_rx_feos_en, 1'b0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  0,  0, 0x0); //rg_force_rx_os_en, 1'b0

#if LAB_PRINT_PON
   printk("=== XPON_RX_FEOS_done ===\n");
#endif

}


void seq_XPON_RX_sdcal(void)
{
//SIGDET_Calibration
//by pass now
  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,  0,  0, 0x0);   //rg_disb_da_xpon_rx_sigdet_os
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,  4,  0, 0x0);  //rg_force_da_xpon_rx_sigdet_os

#if LAB_PRINT_PON
   printk("=== XPON_RX_SD_Cal_done ===\n");
#endif

}


void aux_XPON_phy_status(void)
{
// RX cal ready
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,  8,  8, 0x1);  //rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,  8,  8, 0x0);   //rg_disb_rx_os_rdy
udelay(1);

#if LAB_PRINT_PON
   printk("=== XPON_Rhy_Status_done ===\n");
#endif

}


void seq_XPON_RX_osrdy(void)
{	

volatile unsigned int tmp_read;
volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos, i, blwc_offset, rx_blwc_offset, pical_data_out;
volatile unsigned int RXCK, TXCK , Kband,FLL;
volatile unsigned int EO_Buf[10]= {0};


IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6, 31, 0, 0x1010101);   //FLL_probe_enable
IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6, 31, 0, 0x0);         //FLL_probe_enable

IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x0);   //toggle to generate latch signal
IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x1);   //toggle to generate latch signal

tmp_read = IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_a, 10, 0); //Result=Kband+FLL 
Kband = IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_9, 26, 16);   //Kband //RO(int)
FLL = IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_9, 10,  0);     //FLL //RO(sign) 

IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x0);   //toggle to generate latch signal_0
IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x1);   //toggle to generate latch signal_1

dac_d0 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 6, 0);     //dac_d0 //RO(sign)
dac_d1 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 14,  8);   //dac_d1 //RO(sign)
dac_e0 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 22, 16);   //dac_e0 //RO(sign)
dac_e1 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 30, 24);   //dac_e1 //RO(sign)
dac_eye = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1, 6,   0);  //dac_eye //RO(sign)
feos = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9, 13,  8);     //feos //RO(sign)
pical_data_out = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2, 22, 16);  //pical_data_out //RO(int)
blwc_offset = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1, 14,  8);     //ro_blwc_offset //RO(sign)
rx_blwc_offset = IO_GPHYA_REG_BITS(XPON_PMA_ADD_RO_RX2ANA_2, 22, 16);   //ro_rx_blwc_offset //RO(sign)

TXCK = IO_GPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1, 16, 16);  //TXCK_stable //RO(HEX)
RXCK = IO_GPHYA_REG_BITS(XPON_PMA_RO_RX_FREQDET,  0,  0);   //RXCK_stable  //RO(HEX)


EO_Buf[0] = blwc_offset;
EO_Buf[1] = rx_blwc_offset;
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

  printk(" Result=Kband+FLL(HEX) => %x\n", tmp_read);
  printk(" Kband(HEX) => %x\n", Kband);
  printk(" FLL(HEX) => %x\n", FLL);
  printk(" dac_d0(DEC) => %d\n", EO_Buf[2]);
  printk(" dac_d1(DEC) => %d\n", EO_Buf[3]);  
  printk(" dac_e0(DEC) => %d\n", EO_Buf[4]);
  printk(" dac_e1(DEC) => %d\n", EO_Buf[5]);
  printk(" dac_eye(DEC) => %d\n", EO_Buf[6]);
  printk(" feos(DEC) => %d\n", EO_Buf[7]);
  printk(" pical_data_out(DEC) => %d\n", pical_data_out);
  printk(" ro_blwc_offset(DEC) => %d\n", EO_Buf[0]);
  printk(" ro_rx_blwc_offset(DEC) => %d\n", EO_Buf[1]);
  printk(" TXCK_stable(HEX) => %x\n", TXCK);
  printk(" RXCK_stable(HEX) => %x\n", RXCK); 
 

#if LAB_PRINT_PON
   printk("=== XPON_RX_Osrdy_done ===\n");
#endif

}


void seq_XPON_RX_L2D(void)
{

//L2D  
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x1);   //rg_force_da_xpon_cdr_lpf_rstb
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 16, 16, 0x1);   //rg_force_da_xpon_cdr_lpf_lck2data  
udelay(200);

#if LAB_PRINT_PON
   printk("=== XPON_RX_L2D_done ===\n");
#endif

}

void seq_XPON_FLL_Reset(void)
{


  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_3, 0, 0, 0);   //rg_fll_dig_rst_force_en
  udelay(500);
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_3, 0, 0, 0x01);   //rg_fll_dig_rst_force_en

  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_3, 8, 8, 0);   //rg_fll_dig_rstb_force
  udelay(500);
  IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_3, 8, 8, 0x01);   //rg_fll_dig_rstb_force


#if LAB_PRINT_PON
   printk("=== seq_XPON_FLL_Reset ===\n");
#endif

}


void aux_XPON_TDC_on(void)
{
// ** TDC ** //
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,  8,  8, 0x0);   //PCW from rg
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,  0,  0, 0x1);   //rg_lcpll_tdc_dig_pwdb
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,  0,  0, 0x1);   //rg_lcpll_tdc_cal_bw
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,  8,  8, 0x1);   //rg_lcpll_tdc_cal_offset , 1b1  
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,  8,  8, 0x0);   //rg_lcpll_tdc_cal_offset , 1b0 
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,  0,  0, 0x0);   //rg_lcpll_tdc_cal_bw
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4, 16, 16, 0x1);   //rg_lcpll_tdc_en
udelay(100);
                                         
  IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5, 16, 16, 0x1);   //rg_lcpll_autok_tdc
//udelay(1000);
mdelay(5);	// add delay for reset flow stability    by david 20190412 

#if LAB_PRINT_PON
   printk("=== XPON_TDC_On_done ===\n");
#endif
  
}


void seq_XPON_RX_blwc_on(void)
{
// ** BLWC ** //                          
  IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,  0,  0, 0x1);        //rg_eq_blwc_rst_b, 1494/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,  0,  0, 0x1);   //rg_force_blwc_rx_rst_b, 1668/0
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 16, 16, 0x1);   //rg_force_rx_blwc_en
udelay(50);

#if LAB_PRINT_PON
   printk("=== XPON_RX_Blwc_On ===\n");
#endif

}
 
 
void seq_XPON_RX_rxrdy(void)
{ 
  UINT32 read_data;
//RX data path ready
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24, 24, 0x1); //rg_force_rx_rdy
  IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24, 0x0);  //rg_disb_rx_rdy
udelay(10);

//reset RX FIFO
  IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 0,  0, 0x0);   //rg_sw_rx_fifo_rst_n,1b0
  IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 0,  0, 0x1);   //rg_sw_rx_fifo_rst_n,1b1

//Force RX Impedance //			

  //********************************************OPTIMIZE RX IMPEDANCE by EFUSE*****************************************************//
#if defined(TCSUPPORT_CPU_EN7580)
	  PON_PHY_PRINT(PHY_MSG_ERR,"EN7580_EFUSE: %x\n", EN7580_EFUSE);
	#if EN7580_EFUSE
		  //-------RX R50 Calibration load from EFUSE-------//
			  read_data = get_efuse_data(RG_XPON_RX_FE_50OHMS_SEL);
			  if ((read_data > 0) && (read_data <= 3)){
				  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 26, 25, read_data);
				  PON_PHY_PRINT(PHY_MSG_ERR,"RG_XPON_RX_FE_500HMS_SEL[1:0]=%x\n", read_data);
			  }
			  else {
				  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 26, 25, 0x2);   // if EFUSE readout = 0x0, set RX R50 impedance to level 2(0x2) by default
				  PON_PHY_PRINT(PHY_MSG_ERR,"Set RX Impedance Level 2 as default! \n");
			  }
	#else
		  //-------RX R50 Calibration use default value-------//
			  PON_PHY_PRINT(PHY_MSG_ERR,"RX R50 Calibration use default value. \n");
			  IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 26, 25, 0x2);   // if EFUSE readout = 0x0, set RX R50 impedance to level 2(0x2) by default  
	#endif
 
#endif

  //IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0, 26, 25, 0x2);   //RG_XPON_RX_FE_50OHMS_SEL

#if LAB_PRINT_PON
   printk("=== XPON_RX_Rxrdy ===\n");
#endif

}


void aux_XPON_BIST_setting(void)
{ 
// ** BIST_tx_rx ** //
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT, 31, 0, 0x8ff1fd53);   //bistctl_align_pattern
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED, 31, 0, 0x0ff1fd53);   //bistctl_prbs_init_seed
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 15, 0, 0x1);   //fail_thresold
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 4, 0, 0x5);   //pat_sel , PRBS31
                                                                //Test pattern selection
                                                                //0x01: PRBS7 pattern  6081_fd53
                                                                //0x02: PRBS9 pattern  d97b_0d53
                                                                //0x03: PRBS15 pattern e1fd_7d53
                                                                //0x04: PRBS23 pattern 5cf1_fd53
                                                                //0x05: PRBS31 pattern 8ff1_fd53
                                                                //0x13: PRBS11 pattern a190_3d53
                                                                //0x0C: Square wave pattern (n = 8) LFTP (8180)
							 
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_POLLUTION, 16, 16, 0x1);   //ro_bistctl_prbs_errcnt_read_latch
  IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 0, 0, 0x0);   //anlt_px_lnx_lt_los
  IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 24, 24, 0x0);   //rg_lnx_bistctl_bit_error_rst_sel

#if LAB_PRINT_PON
   printk("=== XPON_Bist_Setting ===\n");
#endif

}


void aux_XPON_BIST_on(void)
{ 
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 8, 8, 0x1);   //BIST_TX_EN 
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x1); //rg_tx_bist_gen_en 
  IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 8, 8, 0x1);         //all_lane_prbs_tx_en
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x1); //rg_bistctl_pat_rx_check_en

udelay(100);
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x0); //rg_bistctl_pat_rx_check_en
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x1); //rg_bistctl_pat_rx_check_en
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x0); //rg_bistctl_pat_rx_check_en
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x1); //rg_bistctl_pat_rx_check_en
  
#if LAB_PRINT_PON
   printk("=== XPON_Bist_On ===\n");
#endif

}


void aux_XPON_R2T_sel(uint r2t_sel)
{

 switch(r2t_sel)
  {
  
 case bist_data:	
   	
		  aux_XPON_BIST_on();
		 
        break;

		
 case r2t_data  :
 	
		  aux_XPON_BIST_on();
	      aux_XPON_R2T_on();
		   
	   break;
	   
 case nor_pma_data  :
 	
          IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0);   //rg_bistctl_pat_tx_en
          IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b0
          
		                                                             //{rg_r2t_mode , rg_tx_bist_gen_en}
		                                                             //2'b01 : da_tx_data = bist_data
		                                                             //2'b10 : da_tx_data = r2t_data
		                                                             //otherwise : normal_pma_tx_data
	   break;
	

      default:
	 break;	
	}

}

void aux_XPON_R2T_on(void)
{ 
  // ** R2T ** //
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x0);   //rg_tx_bist_gen_en
                                           
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,  8,  8, 0x0);   //rg_bistctl_pat_tx_en ,1b0
  IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 16, 16, 0x0);   //rg_bistctl_pat_tx_en
                                           
  IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,  8,  8, 0x0);         //all_lane_prbs_tx_en
                                           
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x0);   //rg_r2t_mode , 1b0
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,  8,  8, 0x1);   //rg_r2t_mode , 1b1                                               
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 24, 24, 0x0);   //rg_r2t_fifo_en , 1b0
  IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 24, 24, 0x1);   //rg_r2t_fifo_en , 1b1

#if LAB_PRINT_PON
   printk("=== XPON_R2T_On ===\n");
#endif

}


void EO_Scan(uint mode_sel, uint bit_sel, uint EO_start, uint EO_end, uint dbg_sel)
{
   volatile int j = 0;
   volatile unsigned int leq_gain = 1;            
   volatile unsigned int leq_peaking = 0;
   volatile unsigned int sweep_start = EO_start;
   volatile unsigned int sweep_end = EO_end ;            
   volatile unsigned int fom_num = 0;
   volatile unsigned int best_fom = 0 , fom_last = 0;   
   volatile unsigned int best_gain = 1;
   volatile unsigned int best_peaking = 0;


if( mode_sel == 0 && dbg_sel == 0)
 {     
       XPON_eye_setting(bit_sel);

      //EYE setting, EYE OPEN related              
       IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);        //rg_eq_en_delay, 8'h80
       IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x4);         //rg_kpgain, 3'h4               

       IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 0, 0, 0x0);        //rg_disb_da_xpon_rx_fe_gain_ctrl
       IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 1, 0, leq_gain);  //rg_force_da_xpon_rx_fe_gain_ctrl                   

   for (j = sweep_start; j < sweep_end + 1; j++)
      {
         IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 8, 8, 0x0);           //rg_disb_da_xpon_rx_fe_peaking_ctrl
         IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 10, 8, leq_peaking); //rg_force_da_xpon_rx_fe_gain_ctrl  
              
     #if Pon_Eye_Scan_Result
         printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
     #endif      
	 
         XPON_eye_Cal();

         fom_num = XPON_eye_EO(bit_sel);                    
                
         if (fom_num > best_fom)
           {
             best_fom = fom_num;                        
             best_peaking = leq_peaking;
             leq_peaking++;
           }
         else
           {
             leq_peaking++;
           }                   
       }
                
       //set back the best leq gain&peaking setting
       IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 0, 0, 0x0);              //rg_disb_da_xpon_rx_fe_gain_ctrl  
       IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 1, 0, best_gain);       //rg_force_da_xfi_rx_fe_gain_ctrl 
       
       IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 8, 8, 0x0);             //rg_disb_da_xpon_rx_fe_peaking_ctrl 
       IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 10, 8, best_peaking);  //rg_force_best_da_xpon_rx_fe_gain_ctrl    
       
    #if Pon_Eye_Scan_Result
       printk("XPON_best_leq_gain => %x , XPON_best_leq_peaking => %x\n" ,best_gain , best_peaking);
 	#endif	 
  }
else if ( mode_sel == 0 && dbg_sel == 1)
   {
     XPON_eye_setting(bit_sel);

     //EYE setting, EYE OPEN related              
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);        //rg_eq_en_delay, 8'h80
     IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 10, 8, 0x4);         //rg_kpgain, 3'h4               

     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 0, 0, 0x0);        //rg_disb_da_xpon_rx_fe_gain_ctrl
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 1, 0, leq_gain);  //rg_force_da_xpon_rx_fe_gain_ctrl                   

   for (j = sweep_start; j < sweep_end + 1; j++)
      {
         IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 8, 8, 0x0);           //rg_disb_da_xpon_rx_fe_peaking_ctrl
         IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 10, 8, leq_peaking); //rg_force_da_xpon_rx_fe_gain_ctrl  
         
     #if Pon_Eye_Scan_Result
         printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
     #endif      
	 
         XPON_eye_Cal();

         fom_num = XPON_eye_EO(bit_sel);                    
                
         if (fom_num > fom_last)
           {             
			 
 			  if (leq_peaking >=7)
			  	{
                  leq_peaking = 7;
			    }	
else 
  {   
				  best_fom = fom_num;						 
				  best_peaking = leq_peaking;
			      leq_peaking++;
			    }
           }
         else
           {  
			  if (leq_peaking <= 0)
			  	{
                  leq_peaking = 0;
			    }
			  else
			  	{
				  leq_peaking--;
			    }
           }   

         fom_last = fom_num;
		 
       }
                
       //set back the best leq gain&peaking setting
       IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 0, 0, 0x0);              //rg_disb_da_xpon_rx_fe_gain_ctrl  
       IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 1, 0, best_gain);       //rg_force_da_xfi_rx_fe_gain_ctrl 
       
       IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 8, 8, 0x0);             //rg_disb_da_xpon_rx_fe_peaking_ctrl 
       IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 10, 8, best_peaking);  //rg_force_best_da_xpon_rx_fe_gain_ctrl    
       
    #if Pon_Eye_Scan_Result
       printk("XPON_best_leq_gain => %x , XPON_best_leq_peaking => %x\n" ,best_gain , best_peaking);
 	#endif	

   }    
         
else if( mode_sel == 1 && dbg_sel == 0)
  {   
         XFI_eye_setting(bit_sel);

         //EYE setting, EYE OPEN related
         IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80); //rg_eq_en_delay, 8'h80 
         IO_SPHYA_REG_BITS(XFI_PMA_SS_RX_PI_CAL, 10, 8, 0x4);  //rg_kpgain, 3'h4 
               
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 0, 0, 0x0);        //rg_disb_da_xpon_rx_fe_gain_ctrl
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 1, 0, leq_gain);  //rg_force_da_xpon_rx_fe_gain_ctrl

    for (j = sweep_start; j < sweep_end + 1; j++)
       {
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 8, 8, 0x0);            //rg_disb_da_xpon_rx_fe_peaking_ctrl 
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 10, 8, leq_peaking);  //rg_force_da_xpon_rx_fe_peaking_ctrl  
         
    #if Xfi_Eye_Scan_Result
	     printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
    #endif
	
         XFI_eye_Cal();
         
         fom_num = XFI_eye_EO(bit_sel);                       

         if (fom_num > best_fom)
           {
              best_fom = fom_num;
              best_peaking = leq_peaking;
              leq_peaking++;
           }
         else
           {
              leq_peaking++;
           }   
                
       }                

         //set back the best leq gain&peaking setting
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 0, 0, 0x0);             //rg_disb_da_xpon_rx_fe_gain_ctrl   
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 8, 8, 0x0);             //rg_disb_da_xfi_rx_fe_peaking_ctrl
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 10, 8, best_peaking);  //rg_force_da_xfi_rx_fe_peaking_ctrl
         
     #if Xfi_Eye_Scan_Result           
         printk("XFI_best_leq_gain => %x , XFI_best_leq_peaking => %x\n" ,best_gain , best_peaking);
	 #endif
	 
  }
  
else if( mode_sel == 1 && dbg_sel == 1)
  {   
         XFI_eye_setting(bit_sel);

         //EYE setting, EYE OPEN related
         IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80); //rg_eq_en_delay, 8'h80 
         IO_SPHYA_REG_BITS(XFI_PMA_SS_RX_PI_CAL, 10, 8, 0x4);  //rg_kpgain, 3'h4 
               
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 0, 0, 0x0);        //rg_disb_da_xpon_rx_fe_gain_ctrl
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 1, 0, leq_gain);  //rg_force_da_xpon_rx_fe_gain_ctrl

    for (j = sweep_start; j < sweep_end + 1; j++)
       {
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 8, 8, 0x0);            //rg_disb_da_xpon_rx_fe_peaking_ctrl 
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 10, 8, leq_peaking);  //rg_force_da_xpon_rx_fe_peaking_ctrl  
    
    #if Xfi_Eye_Scan_Result
	     printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
    #endif
	
         XFI_eye_Cal();
         
         fom_num = XFI_eye_EO(bit_sel);                       

         if (fom_num > fom_last)
           {             
			 
 			  if (leq_peaking >=7)
			  	{
                  leq_peaking = 7;
			    }	
			  else
			  	{
				  best_fom = fom_num;						 
				  best_peaking = leq_peaking;
			      leq_peaking++;
			    }
           }
         else
           {  
			  if (leq_peaking <= 0)
			  	{
                  leq_peaking = 0;
			    }
			  else
			  	{
				  leq_peaking--;
			    }
           }   

         fom_last = fom_num;
                
       }      

         //set back the best leq gain&peaking setting
		 IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 0, 0, 0x0);        //rg_disb_da_xpon_rx_fe_gain_ctrl
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 1, 0, best_gain);  //rg_force_da_xpon_rx_fe_gain_ctrl
         
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 0, 0, 0x0);             //rg_disb_da_xpon_rx_fe_gain_ctrl   
         IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 8, 8, 0x0);             //rg_disb_da_xfi_rx_fe_peaking_ctrl
         IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 10, 8, best_peaking);  //rg_force_da_xfi_rx_fe_peaking_ctrl
         
     #if Xfi_Eye_Scan_Result           
         printk("XFI_best_leq_gain => %x , XFI_best_leq_peaking => %x\n" ,best_gain , best_peaking);
	 #endif
	 
  }

}


void XPON_eye_setting(uint bit_set)
{
    if (bit_set == EO_Async_XGPON_1 || bit_set == EO_Sync_XGSPON_2 || bit_set == EO_Sync_XGSPON_1 || bit_set == EO_Sync_XGSPON_2)
    {                
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x0);       //rg_xpon_cdr_lpf_ratio,0x2
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      //rg_eye_mask, 8'hFF
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);    //rg_x_min, 11'h1C0 , 448 => 512+/-1*64 =448/576
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240);   //rg_x_max, 11'h240 , 576
    }
    else if (bit_set == EO_Sync_GPON || bit_set == EO_Async_GPON)
    {                
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x2);       //rg_xpon_cdr_lpf_ratio,0x2
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0x55);      //rg_eye_mask, 8'h55
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x100);    //rg_x_min, 11'h100 , 256 => 512+/-64*4 = 256/768
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x300);   //rg_x_max, 11'h300 , 768
    }
    else if (bit_set == EO_Sync_EPON_1 || bit_set == EO_Sync_EPON_2 )
    {                
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x3);       //rg_xpon_cdr_lpf_ratio,0x3
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0x22);      //rg_eye_mask, 8'h22
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x0);      //rg_x_min, 11'h0   , 0 => 512+/-8*64 = 0/1024
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x400);   //rg_x_max, 11'h400 , 1024
    }
    // default choose 10G setting
    else
    {                
     IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0, 25, 24, 0x0);       //rg_xpon_cdr_lpf_ratio,0x0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      //rg_eye_mask, 8'hFF
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);    //rg_x_min, 11'h1C0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x234);   //rg_x_max, 11'h234
    }

     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xF8);        //rg_cntlen, 10'h0F8
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 16, 16, 0x0);       //rg_cntforever, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x0);         //rg_data_shift, 1'b0

     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x0);     //rg_index_mode, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0xFFF8);   //rg_eyedur, 20'hFFFF8

     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 0, 0, 0x0);       //rg_eye_nextpts_sel, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 8, 8, 0x0);       //rg_eye_nextpts_toggle, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);     //rg_eye_nextpts, 1'b1

     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x4);     //rg_eyecnt_hth, 8'd2
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x4);    //rg_eyecnt_vth, 8'd2
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x4);    //rg_eo_hth, 11'd10
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth, 7'd10

     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1, 16, 16, 0x0);       //rg_heo_emphasis=1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1, 0, 0, 0x0);         //rg_a_lgain=1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1, 8, 8, 0x0);         //rg_a_mgain=1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2, 1, 0, 0x1);         //rg_a_sel=2'b01
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1, 24, 24, 0x0);       //rg_b_zero_sel=1'b0               
}



void XFI_eye_setting(uint bit_set)
{

    if(bit_set == EO_Sync_XFI_5G_baseR)
    {                
     IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0, 14, 13, 0x1);       //rg_xpon_cdr_lpf_ratio,0x2
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      //rg_eye_mask, 8'h55
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x180);    //rg_x_min, 11'h1C0 , 384 => 512+/-2*64 =384/640
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x280);   //rg_x_max, 11'h280 , 640
    }
    else if(bit_set == EO_Sync_XFI_HSGMII)
    {                
     IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0, 14, 13, 0x1);       //rg_xpon_cdr_lpf_ratio,0x3
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      //rg_eye_mask, 8'hFF
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x140);    //rg_x_min, 11'h140 , 320 => 512+/-3*64 = 320/704
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x2C0);   //rg_x_max, 11'h2C0 , 704
    }
    else if(bit_set == EO_Sync_XFI_SGMII)
    {                
     IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0, 14, 13, 0x3);       //rg_xpon_cdr_lpf_ratio,0x3
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0x22);      //rg_eye_mask, 8'h22
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x0);      //rg_x_min, 11'h0   , 0 => 512+/-8*64 = 0/1024
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x400);   //rg_x_max, 11'h400 , 1024
    }
    // default choose 10G setting
    else
    {                
     IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0, 14, 13, 0x0);       //rg_xpon_cdr_lpf_ratio,0x0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      //rg_eye_mask, 8'hFF
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);    //rg_x_min, 11'h1C0 , 448 => 512 +/ -1*64 = 448 / 576
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240);   //rg_x_max, 11'h240 , 576
    }

     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xF8);        //rg_cntlen, 10'h0F8
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 16, 16, 0x0);       //rg_cntforever, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x0);         //rg_data_shift, 1'b0

     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x0);     //rg_index_mode, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0xFFF8);   //rg_eyedur, 20'hFFFF8

     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 0, 0, 0x0);       //rg_eye_nextpts_sel, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 8, 8, 0x0);       //rg_eye_nextpts_toggle, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);     //rg_eye_nextpts, 1'b1

     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x4);     //rg_eyecnt_hth, 8'd2
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x4);    //rg_eyecnt_vth, 8'd2
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x4);    //rg_eo_hth, 11'd10
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth, 7'd10

     IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_1, 16, 16, 0x0);       //rg_heo_emphasis=1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_1, 0, 0, 0x0);         //rg_a_lgain=1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_1, 8, 8, 0x0);         //rg_a_mgain=1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_2, 1, 0, 0x1);         //rg_a_sel=2'b01     
     IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_1, 24, 24, 0x0);       //rg_b_zero_sel=1'b0              
 
}



void XPON_eye_Cal(void)
{
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2, 24, 24, 0x1);  //rg_disb_da_xpon_cdr_pr_pieye
//pical redo
//reset block
     IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,  7,  0, 0x80);  //rg_eq_en_delay, 8'h80
	 IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL, 10,  8, 0x1);	   //rg_kpgain, 3'h4
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0, 16, 16, 0x0); 	   //rg_eq_pi_cal_rst_b, 1'b0   
		
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,  8,  8, 0x0);  //rg_disb_rx_and_pical_rstb, 1'b0
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,  8,  8, 0x0); //rg_force_rx_and_pical_rstb, 1'b0		 
		
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,  0,  0, 0x0);  //rg_disb_ref_and_pical_rstb, 1'b0
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,  0,  0, 0x0); //rg_force_ref_and_pical_rstb, 1'b0	   
		
//enable		  
  	 IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5, 24, 24, 0x0);  //rg_disb_rx_or_pical_en, 1'b0
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,  8,  8, 0x0); //rg_force_rx_or_pical_en, 1'b0
		
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,  8,  8, 0x0);	   //rg_disb_rx_pical_en
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  8,  8, 0x0);   //rg_force_rx_pical_en
		
//release reset
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0, 16, 16, 0x1); 	   //rg_eq_pi_cal_rst_b, 1'b1
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,  8,  8, 0x1); //rg_force_rx_and_pical_rstb, 1'b1
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,  0,  0, 0x1); //rg_force_ref_and_pical_rstb, 1'b1
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,  8,  8, 0x1); //rg_force_rx_or_pical_en, 1'b1			
		
     //udelay(50000); //delay for 50ms 
     udelay(1000); //delay for 1ms 
		
  	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6, 8, 8, 0x0);   //rg_force_rx_or_pical_en, 1'b1		 
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x0);	   //rg_disb_eq_pi_cal_rdy, 1'b0
	 IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x1);   //rg_force_eq_pi_cal_rdy, 1'b1
}



 void XFI_eye_Cal(void)
 {
 
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_2, 24, 24, 0x1);	//rg_disb_da_xpon_cdr_pr_pieye
 //pical redo
 //reset block
	  IO_SPHYA_REG_BITS(XFI_PMA_PHY_EQ_CTRL_0,  7,  0, 0x80);	//rg_eq_en_delay, 8'h80
	  IO_SPHYA_REG_BITS(XFI_PMA_SS_RX_PI_CAL, 10,  8, 0x4);		//rg_kpgain, 3'h4
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_RESET_0, 16, 16, 0x0);		//rg_eq_pi_cal_rst_b, 1'b0	 
		 
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_6,  8,  8, 0x0);	//rg_disb_rx_and_pical_rstb, 1'b0
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_7,  8,  8, 0x0);	//rg_force_rx_and_pical_rstb, 1'b0		  
		 
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_6,  0,  0, 0x0);	//rg_disb_ref_and_pical_rstb, 1'b0
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_7,  0,  0, 0x0);	//rg_force_ref_and_pical_rstb, 1'b0 		  
		 
 //enable		   
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_5, 24, 24, 0x0);	//rg_disb_rx_or_pical_en, 1'b0
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_6,  8,  8, 0x0);	//rg_force_rx_or_pical_en, 1'b0
		 
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,  8,  8, 0x0);	 //rg_disb_rx_pical_en
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,  8,  8, 0x0); //rg_force_rx_pical_en
		 
 //release reset
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_RESET_0, 16, 16, 0x1);		//rg_eq_pi_cal_rst_b, 1'b1
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_7,  8,  8, 0x1);	//rg_force_rx_and_pical_rstb, 1'b1
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_7,  0,  0, 0x1);	//rg_force_ref_and_pical_rstb, 1'b1
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_6,  8,  8, 0x1);	//rg_force_rx_or_pical_en, 1'b1 		
	  
	  udelay(1000); //delay for 1ms 
		 
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_6, 8, 8, 0x0); 	//rg_force_rx_or_pical_en, 1'b1 	  
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_3, 0, 0, 0x0); 	//rg_disb_eq_pi_cal_rdy, 1'b0
	  IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_3, 0, 0, 0x1); 	//rg_force_eq_pi_cal_rdy, 1'b1
  
 }


 

 int XPON_eye_EO(uint bit_rate)
{
   volatile unsigned int fom_num;

     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xA);        //rg_cntlen, 10'h0F8
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C);   //rg_eyedur, 20'hFFFF8

//reset eye_top               
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_8, 8, 8, 0x0);     //rg_disb_eye_reset_plu_o, 1'b0  
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x1);    //rg_force_eye_reset_plu_o, 1'b0                      
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x0);    //rg_force_eye_reset_plu_o, 1'b1------->0 

     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x0);   //rg_disb_eye_top_en, 1'b0 
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x0);  //rg_force_eye_top_en, 1'b0 
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x1);  //rg_force_eye_top_en, 1'b1    
     
     if (bit_rate == EO_Async_XEPON || bit_rate == EO_Sync_XEPON || bit_rate == EO_Async_XGPON_1 || bit_rate == EO_Async_XGPON_2 || bit_rate == EO_Sync_XGSPON_1 || bit_rate == EO_Sync_XGSPON_2)
      {
        udelay(5500); //delay for 5.5ms  
        //udelay(10000); //delay for 10ms 
      }
	 else
	  {
		 udelay(55000); //delay for 55ms  
	  }

     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2, 8, 8, 0x1);     //rg_disb_da_xpon_rx_dac_eye  
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2, 24, 24, 0x1);   //rg_disb_da_xpon_cdr_pr_pieye 
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x1);  //rg_disb_eyedur_init_b, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7, 24, 24, 0x1);          //rg_disb_eyecnt_rx_rst_b, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x1);  //rg_disb_eyedur_en, 1'b0  

//read out EO results
     fom_num = XPON_readout_EO(0); 

//clear EXE_X_SW offset value
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 8, 8, 0x1);    //rg_force_eye_reset_plu_o, 1'b0  
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_8, 16, 16, 0x0);   //rg_disb_eye_top_en, 1'b0
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_9, 16, 16, 0x0);  //rg_force_eye_top_en, 1'b0                        

//reset pi_cal_rdy
     IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_3, 0, 0, 0x0);     //rg_disb_eq_pi_cal_rdy, 1'b0  
     IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_3, 0, 0, 0x0);    //rg_force_eq_pi_cal_rdy, 1'b1

   return fom_num;
}


  int XFI_eye_EO(uint bit_rate)
{

   volatile unsigned int fom_num;


	 if (bit_rate == EO_Sync_XFI_HSGMII )
	   {
		 IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xA);	  //rg_cntlen, 10'h0F8
         IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x898); //rg_eyedur, 20'hFFFF8         
	   }
	 else
	   {
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xA);	  //rg_cntlen, 10'h0F8
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C); //rg_eyedur, 20'hFFFF8
	   }     
    
//reset eye_top               
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_8, 8, 8, 0x0);     //rg_disb_eye_reset_plu_o, 1'b0 	 
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x1);     //rg_force_eye_reset_plu_o, 1'b0    
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x0);     //rg_force_eye_reset_plu_o, 1'b1------->0 
     
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_8, 16, 16, 0x0);   //rg_disb_eye_top_en, 1'b0 
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x0);   //rg_force_eye_top_en, 1'b0 
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x1);   //rg_force_eye_top_en, 1'b1   
     
     if (bit_rate == EO_Sync_XFI_10G || bit_rate == EO_Sync_XFI_5G_baseR)
      {
        udelay(10000); //delay for 5.5ms  
      }
	 else if (bit_rate == EO_Sync_XFI_SGMII )
	  {
		 udelay(55000); //delay for 55ms  
      }
	 else
	  {
        udelay(110000); //delay for 55ms  	  
	  }

     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_2, 8, 8, 0x1);     //rg_disb_da_xpon_rx_dac_eye  
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_2, 24, 24, 0x1);   //rg_disb_da_xpon_cdr_pr_pieye 
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x1); //rg_disb_eyedur_init_b, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_7, 24, 24, 0x1);         //rg_disb_eyecnt_rx_rst_b, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x1); //rg_disb_eyedur_en, 1'b0  

//read out EO results
     fom_num = XFI_readout_EO(0); 
     
//clear EXE_X_SW offset value
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x1);    //rg_force_eye_reset_plu_o, 1'b0  
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_8, 16, 16, 0x0);   //rg_disb_eye_top_en, 1'b0
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x0);  //rg_force_eye_top_en, 1'b0                        

//reset pi_cal_rdy
     IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_3, 0, 0, 0x0);     //rg_disb_eq_pi_cal_rdy, 1'b0  
     IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_3, 0, 0, 0x0);    //rg_force_eq_pi_cal_rdy, 1'b1

   return fom_num;
 
   
}


int XPON_readout_EO(int sweep_direction)
{
   volatile unsigned int eye_er, eye_el, eye_eu, eye_eb, i = 0;
   volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos;
   volatile	unsigned int eye_x_done, eye_y_done, heo_rdy, veo_rdy;
   volatile unsigned int veo;   
   volatile int fom_num,heo,heo_tmp;
   volatile unsigned int pical_data_out;   
   volatile unsigned int EO_Buf[10]= {0};
   

	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x0); //toggle to generate latch signal
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0, 24, 24, 0x1); //toggle to generate latch signal		

	pical_data_out = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2, 22, 16);
	eye_x_done = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9, 16, 16); //eye_x_done 
	eye_y_done = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9, 24, 24); //eye_y_done 
	heo_rdy = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_5, 16, 16);    //heo_rdy
	veo_rdy = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_5, 24, 24);    //veo_rdy 				
   
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
     
	 eye_el = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_10, 26, 16); //eye_el
	 eye_er = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_10, 10, 0);  //eye_er
	 eye_eu = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_11, 6, 0);   //eye_eu
	 eye_eb = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_11, 14, 8);  //eye_eb
	 veo = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_4, 23, 16);		   
	 //heo= eye_er - eye_el; 

	 //ABS_Funt._0724 Morris
	 int eye_er_tmp = (int) eye_er;  //uint to int 
	 int eye_el_tmp = (int) eye_el;
		   
     heo_tmp= eye_er_tmp - eye_el_tmp;
   if (heo_tmp>0)
	 {
       heo = heo_tmp;
	 }
   else
     {
        heo = heo_tmp * -1;
     }	

	 dac_d0 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 6, 0);   //dac_d0
	 dac_d1 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 14, 8);  //dac_d1
	 dac_e0 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 22, 16); //dac_e0
	 dac_e1 = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0, 30, 24); //dac_e1
	 dac_eye = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1, 6, 0);  //dac_eye
	 feos = IO_GPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9, 13, 8);    //feos
	   
	 fom_num = heo; //hugo, 170613		  
    
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
      //EOvalid = 0;
	  IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0, 24, 24, 0x0); //rg_disb_da_xpon_cdr_lpf_rstb  
	  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x0); //rg_force_da_xpon_cdr_lpf_rstb	  
	  udelay(500); //delay for 500us 

	  IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0, 24, 24, 0x1); //rg_force_da_xpon_cdr_lpf_rstb	 
	  udelay(500); //delay for 500us 
	 }
   return fom_num;
   
}	   



int XFI_readout_EO(int sweep_direction)
{

   volatile unsigned int eye_er, eye_el, eye_eu, eye_eb, i = 0;
   volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos;
   volatile	unsigned int eye_x_done, eye_y_done, heo_rdy, veo_rdy;
   volatile unsigned int veo;   
   volatile int fom_num,heo,heo_tmp;
   volatile unsigned int pical_data_out;   
   volatile unsigned int EO_Buf[10]= {0};   

   IO_SPHYA_REG_BITS(XFI_PMA_RX_DEBUG_0, 24, 24, 0x0); //toggle to generate latch signal
   IO_SPHYA_REG_BITS(XFI_PMA_RX_DEBUG_0, 24, 24, 0x1); //toggle to generate latch signal		

   pical_data_out = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_2, 22, 16);
   eye_x_done = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_9, 16, 16); //eye_x_done 
   eye_y_done = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_9, 24, 24); //eye_y_done 
   heo_rdy = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_5, 16, 16);    //heo_rdy
   veo_rdy = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_5, 24, 24);    //veo_rdy 				

   
 if ((eye_x_done == 1) && (eye_y_done == 1))
   {	
#if Xfi_Eye_Scan_Result
		printk("\n" );	    
		printk("X&Y index scan done\n");
#endif
	if ((heo_rdy == 1) && (veo_rdy == 1))
	  {		       
#if Xfi_Eye_Scan_Result
	     printk("\n" );			 
	     printk("heo&veo result ready\n");
#endif
	  }
	}
	else
	   {		   
#if Xfi_Eye_Scan_Result
		 printk("X&Y index scan NOT done\n");
#endif
	   }
     
    eye_el = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_10, 26, 16); //eye_el
	eye_er = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_10, 10, 0);  //eye_er
	eye_eu = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_11, 6, 0);   //eye_eu
	eye_eb = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_11, 14, 8);  //eye_eb
	veo = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_4, 23, 16);		   
	//heo= eye_er - eye_el; 
	

	//ABS_Funt._0724 Morris
	int eye_er_tmp = (int) eye_er;  //uint to int 
	int eye_el_tmp = (int) eye_el;
		   
    heo_tmp= eye_er_tmp - eye_el_tmp;
		   
	if (heo_tmp>0)
	  {
        heo = heo_tmp;
	  }
	else
	  {
        heo = heo_tmp * -1;
	  }	

	 dac_d0 = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_0, 6, 0);   //dac_d0
	 dac_d1 = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_0, 14, 8);  //dac_d1
	 dac_e0 = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_0, 22, 16); //dac_e0
	 dac_e1 = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_0, 30, 24); //dac_e1
	 dac_eye = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_1, 6, 0);  //dac_eye
	 feos = IO_GPHYA_REG_BITS(XFI_PMA_RX_TORGS_DEBUG_9, 13, 8);    //feos
		   
	 fom_num = heo; //hugo, 170613		  
    
	 //print out these results to textbox
	 if (sweep_direction == 0)
	    {	
 #if Xfi_Eye_Scan_Result
		   printk("Y index sweep direction: bottom-up\n");
 #endif
		}
	 else
	    {
 #if Xfi_Eye_Scan_Result
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

 #if Xfi_Eye_Scan_Result		   
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
		   
 #if Xfi_Eye_Scan_Result	
	    printk("EO result isn't valid.\n");
 #endif	

	   //EOvalid = 0;
	   IO_SPHYA_REG_BITS(XFI_PMA_RX_DISB_MODE_0, 24, 24, 0x0); //rg_disb_da_xpon_cdr_lpf_rstb  
	   IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 24, 24, 0x0); //rg_force_da_xpon_cdr_lpf_rstb
	   udelay(500); //delay for 500us 

	   IO_SPHYA_REG_BITS(XFI_PMA_RX_FORCE_MODE_0, 24, 24, 0x1); //rg_force_da_xpon_cdr_lpf_rstb	
	   udelay(500); //delay for 500us 
    }
		   
    return fom_num;
   
}	   



void debug_Mode (void)
{
	   
   volatile int n=0 , k=0 , j=0 , i =1 ;  
   volatile int FLL[5] = {0x7F0 , 0x7F8 , 0 , 0x8 , 0x10} , F0 , G0; 
   volatile int F0_FLL[1024]= {0} ,G0_FLL[1024]= {0};
	   	   
   n = 0x77D; 
   k = 0; 


for ( j = 0; j < 20 ; j++ ) // Full scan
	 { 
	    IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6		   
	    udelay(5000);
	    IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6

	    F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_9 , 10 , 0 ); // ro_fll_idac
		printk("=== ro_fll_idac: %x \n", F0);	 
     }

        printk("\n");

   
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 0 , 0 , 0x1 ); // rg_fll_repli
		

   
		 
//for ( j = 2016; j < 2048 ; j++ )
 for ( j = 1917; j < 2048 ; j++ ) // Full scan
 { 
				 
	#ifdef LAB_PRINT_PON	  
		   //printk("=== rg_fll_idac_repli: %x \n", n);   
	#endif	
		   
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 18 , 8 , n ); // rg_fll_idac_repli		   
			   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
			   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   

		

		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
			   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
		delay_ponphy(300);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   
			   	   
				   
		//delay_ponphy(300);
		//F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_7 , 19 , 0 ); // ro_adc_freq		   
				   
		//delay_ponphy(300);
		G0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_8 , 23 , 0 ); // ro_cor_gain 
				   
				   
		   if (G0 <= 8388608)
			 {				 
				G0 = G0;				
			 }
		   else
			 {			  
			   G0 -= 16777216;			   
			 }	 		   
	   F0_FLL[i]= F0;
       i += 1;
	   n += 1; 
		
					 
    #ifdef LAB_PRINT_PON          		  
		   printk("=== ro_adc_freq: %d \n", F0);
		   //printk("=== ro_cor_gain: %d \n", G0);    
	#endif

 }					 
							 
	   
			//for ( j = 0; j < 32 ; j++ )
for ( j = 0; j < 124 ; j++ ) // full scan
{				   
    #ifdef LAB_PRINT_PON
	//printk("=== ro_adc_freq: %x \n", k);	   
	#endif
			   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 18 , 8 , k ); // rg_xpon_cdr_pr_monmuxvc
	   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
				   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101); // SS_RX_FLL_6
	   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   
				   	   
				   

    delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
			   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
	delay_ponphy(300);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug 
				   
	
	//delay_ponphy(300);
	//F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_7 , 19 , 0 ); // ro_adc_freq  
	
	delay_ponphy(300);
	G0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_8 , 23 , 0 ); // ro_cor_gain 
				   
				   
	if (G0 <= 8388608)
	    {				 
		  G0 = G0;				
	    }
    else
		{			  
		  G0 -= 16777216;			   
		}	
	
	F0_FLL[i]= F0;
    i += 1;	   
	k += 1;		   
	   
	#ifdef LAB_PRINT_PON 			
	   printk("=== ro_adc_freq: %d \n", F0);
	   //printk("=== ro_cor_gain: %d \n", G0); 
	   //printk("\n");
	#endif
}

	   
	   
 //n = 0x7E0 ; 
   n = 0x77D ; 
   k = 0; 
   i = 1;
	   
	#ifdef LAB_PRINT_PON 
	   printk("\n");
	#endif
	   
 //for ( j = 2016; j < 2048 ; j++ )
for ( j = 1917; j < 2048 ; j++ ) // Full scan
{	   
  #ifdef LAB_PRINT_PON
     //#define("=== rg_fll_idac_repli: %x \n", n);
  #endif
			   
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 18 , 8 , n ); // rg_fll_idac_repli
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   


   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   
				      
   
				   
				   
   //delay_ponphy(300);
   //F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_7 , 19 , 0 ); // ro_adc_freq		   
				   
   //delay_ponphy(300);
   G0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_8 , 23 , 0 ); // ro_cor_gain 
	   
	n++;
				   
  if (G0 <= 8388608)
      {				 
		G0 = G0;				
	  }
  else
      {			  
		G0 -= 16777216; 			   
	  }

  G0_FLL[i]= G0;
  i += 1;
  
  
  #ifdef LAB_PRINT_PON       			  
	//printk("=== ro_adc_freq: %d \n", F0);
   printk("=== ro_cor_gain: %d \n", G0); 
 #endif
}	  
			   
	   
 //for ( j = 0; j < 32 ; j++ )
  for ( j = 0; j < 124 ; j++ ) // full scan
{ 
   //n = 0x0 ; 
	   
  #ifdef LAB_PRINT_PON
   //printk("=== rg_fll_idac_repli: %x \n", k);
  #endif
			   
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 18 , 8 , k ); // rg_fll_idac_repli
	   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101); // SS_RX_FLL_6
	   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   


   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101); // SS_RX_FLL_6
	   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug
				   
   delay_ponphy(300);
   IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug   
				   	   
				   
				   
   //delay_ponphy(300);
   //F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_7 , 19 , 0 ); // ro_adc_freq	  
   
   //delay_ponphy(300);
   G0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_8 , 23 , 0 ); // ro_cor_gain		   
				   
 if (G0 <= 8388608)
     {				 
       G0 = G0;				
 	 }
 else
	 {			  
	   G0 -= 16777216; 			   
	 }
 
  G0_FLL[i]= G0;
  i += 1;
				   
   k += 1;				   
	   
	#ifdef LAB_PRINT_PON
	   //printk("=== ro_adc_freq: %d \n", F0);
	   printk("=== ro_cor_gain: %d \n", G0); 
	   //printk("\n");
	#endif
 }

//if((F0_FLL[132] > F0_FLL[124]) && (F0_FLL[124] > F0_FLL[116]) && (F0_FLL[132] > F0_FLL[140]) && (F0_FLL[140] > F0_FLL[148]))
//   { 		   
//	  printk("\n");
//      printk("=== ro_adc_freq : PASS ===\n");
//   }
			
//else
//   {
//     printk("\n");
//	 printk("=== ro_adc_freq : FAIL ===\n");
//   }


if((G0_FLL[116] > G0_FLL[124]) && (G0_FLL[124] > 0) && (0 > G0_FLL[140]) && (G0_FLL[140] > G0_FLL[148]))
   { 		   
	  printk("\n");
      printk("=== ro_cor_gain : PASS ===\n");
   }
			
else
   {
     printk("\n");
	 printk("=== ro_cor_gain : FAIL ===\n");
   }    
/*
	 printk("\n");
        printk("=== ro_adc_freq === %d \n" , F0_FLL[116]);
	 printk("=== ro_adc_freq === %d \n" , F0_FLL[124]);
	 printk("=== ro_adc_freq === %d \n" , F0_FLL[132]);
        printk("=== ro_adc_freq === %d \n" , F0_FLL[140]);	 
	 printk("=== ro_adc_freq === %d \n" , F0_FLL[148]);

	 printk("\n");
	 printk("=== ro_cor_gain === %d \n" , G0_FLL[116]);
	 printk("=== ro_cor_gain === %d \n" , G0_FLL[124]);
	 printk("=== ro_cor_gain === %d \n" , G0_FLL[132]);
        printk("=== ro_cor_gain === %d \n" , G0_FLL[140]);	 
	 printk("=== ro_cor_gain === %d \n" , G0_FLL[148]);
*/

	 IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 0 , 0 , 0x0 ); // rg_fll_repli


	 for ( j = 0; j < 20 ; j++ ) // Full scan
		  { 
			 IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x00000000 ); // SS_RX_FLL_6 		
			 udelay(5000);
			 IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
	 
			 F0 =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_9 , 10 , 0 ); // ro_fll_idac
			 printk("=== ro_fll_idac: %x \n", F0);	  
		  }


  
}


void RXFLL (void)
{
	volatile int F0=0 ,G0=0 , F0_temp=0 , G0_temp=0 ;
	volatile unsigned int i , j , read_temp ;  

	//volatile int FLL[4] = {0x7F0 , 0x7F8 , 0x8 , 0x16}; 
	volatile int FLL[2] = {0x7F6 , 0x10 };
	volatile int F0_FLL[5]= {0} ,G0_FLL[5]= {0} , avg_num = 1;
	//unsigned long timeout = jiffies;
    //unsigned long time1 = jiffies;
	//unsigned long time2 = jiffies ;
	
	//time_t timep;
	//time (&timep);	
	

    //prom_printf("=== Read_adc_Freq & cor_gain : ===\n");	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 0 , 0 , 0x1 ); // rg_fll_repli

	 //n = 0x77D; 

for ( i = 0; i < 2 ; i++ )
   { 	   
	   for ( j = 0; j < avg_num ; j++ )
		{ 		    
			//time1 = jiffies_to_msecs(jiffies);
			//printk("=== ro_cor_gain_time1 : %lu ===\n", time1); 
			
			
			//printk("=== rg_fll_idac_repli: %x ===\n", FLL[i]);
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_4 , 18 , 8 , FLL[i] ); // rg_fll_idac_repli
			
            
            delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x0 ); // SS_RX_FLL_6	
			
			delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6
			
            delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug	
			
			delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug 


            delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x0 ); // SS_RX_FLL_6				
			
			delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_6 , 31 , 0 , 0x01010101 ); // SS_RX_FLL_6			
            
            delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0xFFFF ); // RX_debug				
			
			delay_ponphy(300);	
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0 , 31 , 0 , 0x0100FFFF ); // RX_debug						    
			
			
			//delay_ponphy(300);	
			G0_temp =  IO_GPHYA_REG_BITS(XPON_PMA_SS_RX_FLL_8 , 23 , 0 ); // ro_cor_gain 

			
			//printk("=== ro_cor_gain G%d : %d ===\n", i , G0_temp); 
			
			//time2 = jiffies_to_msecs(jiffies);
			//printk("=== ro_cor_gain_time2 : %lu ===\n", time1); 
			
			
			if (G0_temp <= 8388608)
			  { 			  
				 G0_temp = G0_temp;				 
			  }
			else
			  {			   
			    G0_temp -= 16777216;				
			  }		                     
			   
			   G0 += G0_temp;			
			  
		 }			     

			
			 //G0_FLL[i] = (G0 / avg_num) ;
			 G0_FLL[i] = G0;
			 
			 printk("=== ro_cor_gain G%d (AVG) : %d ===\n", i , G0_FLL[i]); 
			 G0 = 0 ;

			 
	} 

  
   //if((G0_FLL[0] > G0_FLL[1]) && (G0_FLL[1] > 0 ) && ( 0 > G0_FLL[2]) && (G0_FLL[2] > G0_FLL[3]))
   if((G0_FLL[0] > G0_FLL[1]))
   { 		   
		  printk("=== PASS ===\n");		 
   }
			
	else
   {
	      printk("=== FAIL ===\n");		  
   }    

}

void delay_ponphy(int cycle_num)
{
  int i;
  for(i=0;i<cycle_num*1000;i++)
    ;
}



#define _PMA_FUNCTION_SPLIT_LINE_EXTRA


void sw_pma_rst_hold(void)
{

// holding & Releasing sw_pma_rst_n can avoid big EqD variation with multiple plug-out/plug-in tests 
  IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 3, 3, 0x0); 	  //hold sw_pma_rst_n	   // by david 20190304 

}

void sw_pma_rst_release(void)
{

// holding & Releasing sw_pma_rst_n can avoid big EqD variation with multiple plug-out/plug-in tests 
  IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 3, 3, 0x1); 	  //release sw_pma_rst_n	   // by david 20190304 

}

void sw_tx_rst(void) //add by ang_20191031
{
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 2, 2,	 0x0);	//rg_sw_tx_rst_n
	IO_SPHYA_REG_BITS(XPON_PMA_SW_RST_SET, 2, 2,	 0x1);	//rg_sw_tx_rst_n
}

#endif

// -- EN7580 PMA function split END------------------------------------------------


