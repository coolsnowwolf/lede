//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : AN7583 PON PHY
//
//******************************************************************************

// -- includes -----------------------------------------------------------------
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>

#include <asm/io.h>
#include <asm/tc3162/tc3162.h>

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>

#include <linux/timer.h>
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36) 
#include <linux/time64.h>
#endif

#include "../inc/an7583_pon_pma.h"
#include "../../phy_efuse_def.h"
#include "uapi/ecnt_event_global/ecnt_event_serdes.h"



uint InitSpd_Pon_TX = 0 , InitSpd_Pon_TXPLL = 0 , InitSpd_Pon_RX = 0 , InitSpd_Pon_ANA = 0 , InitSpd_Pon_Preset = 0, RX_CDR_FM = 0 , wan_sel = 0;
char lcpll_a_tdc=0x5;
static uint en7581_bist_check_err_cnt;  
u32 Pon_rx_term_sel = 0 , Pon_tx_term_sel = 0 , PON_CMN_TRIM = 0;
static u8 pon_cn1_set = 0x0, pon_c0b_set = 0xE, pon_c1_set = 0x4, pon_c2_set = 0x0;

uint pma_hi_rate_opt_val = XPON_PMA_OPT_DEFAULT_VALUE_HIGH_RATE;
uint pma_low_rate_opt_val = XPON_PMA_OPT_DEFAULT_VALUE_LOW_RATE;

EXPORT_SYMBOL(pma_hi_rate_opt_val);
EXPORT_SYMBOL(pma_low_rate_opt_val);







AN7583_XPON_PMA_Param_T pon_pma_param;





// --  variables ----------------------------------------------------------------

void __iomem *pon_ana = NULL; 
void __iomem *pon_pma  = NULL;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern void __iomem* Get_Base(u32 base);
extern u32 get_phy_efuse(u32 start_bit, u32 len);


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void get_pon_ana_base(void)
{
	pon_ana = Get_Base(PON_ANA_RG_BASE); 
	if (IS_ERR(pon_ana))
	{
		printk("[ERROR]pon_ana base= %lx\n", (unsigned long)pon_ana);
	}
}

static void get_pon_pma_base(void)
{
	pon_pma = Get_Base(PON_PMA_RG_BASE); 
	if (IS_ERR(pon_pma))
	{
		printk("[ERROR]pon_pma base= %lx\n", (unsigned long)pon_pma);
	}
}

static void set_pon_ana_rg(u32 reg, u32 val)
{
	if(pon_ana == NULL)
	{
		get_pon_ana_base();
	}
	writel(val, pon_ana + reg); 
}	

static void set_pon_pma_rg(u32 reg, u32 val)
{
	if(pon_pma == NULL)
	{
		get_pon_pma_base();
	}
	writel(val, pon_pma + reg); 
}	

static u32 get_pon_ana_rg(u32 reg)
{
	if(pon_ana == NULL)
	{
		get_pon_ana_base();
	}
	return readl(pon_ana + reg);
}

static u32 get_pon_pma_rg(u32 reg)
{
	if(pon_pma == NULL)
	{
		get_pon_pma_base();
	}
	return readl(pon_pma + reg);
}

static u32 get_pon_phya_data(u32 reg)
{
	u32 reg_phy = 0;
	u32 reg_val = 0;

	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Get reg_val error, reg=0x%08X\n", __func__, reg);		
		return 0;
	}

	if( (PON_ANA_RG_BASE <= reg_phy) && (reg_phy < PON_ANA_RG_BASE+0xfff) )
    {    	
    	reg_val= get_pon_ana_rg(reg_phy - PON_ANA_RG_BASE);
    } 
	else if( (PON_PMA_RG_BASE <= reg_phy) && (reg_phy < PON_PMA_RG_BASE+0xfff) )
    {    	
    	reg_val= get_pon_pma_rg(reg_phy - PON_PMA_RG_BASE);
    }
	else
	{
		printk("\n Datapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
	}

	return reg_val;
}

static void set_pon_phya_data(u32 reg, u32 val)
{
	u32 reg_phy = 0;
	
	/* translate addr to physical addr */
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Set reg_val error, reg=0x%08X\n", __func__, reg);		
		return ;
	}
	
	if( (PON_ANA_RG_BASE <= reg_phy) && (reg_phy < PON_ANA_RG_BASE+0xfff) )
    {    	
    	set_pon_ana_rg(reg_phy - PON_ANA_RG_BASE, val);
    } 
	else if( (PON_PMA_RG_BASE <= reg_phy) && (reg_phy < PON_PMA_RG_BASE+0xfff) )
    {    	
    	set_pon_pma_rg(reg_phy - PON_PMA_RG_BASE, val);
    }
	else
	{
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
	}

}


static u32 IO_GPHYA_REG_BITS(u32 reg_name,u32 end_index,u32 start_index)
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
		printk("%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
		return 0;
	}
}



static void IO_SPHYA_REG_BITS(u32 reg_name,u32 end_index,u32 start_index,u32 value)
{
	u32 data;
	data=IO_GPHYREG(reg_name);
	
	if((end_index>=start_index)&&(end_index<32))
	{	
		if((end_index==31)&&(start_index==0))
		{
			IO_SPHYREG(reg_name,value);
		}
		else
		{
			IO_SPHYREG(reg_name,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index))) ;
		}
	}
	else
	{
		printk("%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
	}
}



void xpon_init (int bit_sel)
{
	pon_DIG_reset_hold();
	pon_CfgPhyType(bit_sel);
	pon_LinkControl();  
}


void pon_LinkControl (void)
{
   
   XPON_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	   
   XPON_TX_on();
}


//======================================================================================

void fiber_plug_reset (int plug_sel , int pon_Spd)
{
	static int	plug_out_flag=1,plug_in_flag=0;

    switch(plug_sel)
	{
        case FIRST_PLUG_IN: 
            XPON_RX_preset(InitSpd_Pon_Preset); 
            XPON_TDC_off(); 
            XPON_RX_on(); 
            XPON_RX_L2R(); 
			pon_DIG_ref_release();

            XPON_RX_OSCal(); 
            XPON_RX_pical();
            XPON_RX_pdos(); 
            XPON_RX_feos();
            XPON_RX_sdcal();   
   
            XPON_phy_status();
			//XPON_DIG_reset();	
			pon_DIG_reset_release();
            XPON_RX_L2D();   
            Pon_EO_Scan(pon_Spd,0,7 ,0); //20221011
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
			  pon_DIG_reset_hold();
			
			  XPON_RX_L2D();
			  XPON_TDC_on(); 

			  pon_DIG_reset_release();	
			
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
			  XPON_FLL_Reset();
			  XPON_RX_L2R(); 

			  plug_out_flag =0;
			  plug_in_flag =1;
			}

        break;	

		case XFI_PON_FIRST_PLUG_IN:

				XPON_RX_preset(InitSpd_Pon_Preset); 
				XPON_TDC_off(); 
				XPON_XFI_RX_Setting(pon_Spd);
				XPON_RX_on(); 
				XPON_RX_L2R(); 
				pon_DIG_ref_release();
				
				XPON_RX_OSCal(); 
				XPON_RX_pical();
				XPON_RX_pdos(); 
				XPON_RX_feos();
				XPON_RX_sdcal();   
				
				XPON_phy_status();
				XPON_DIG_reset();			
				XPON_RX_L2D();	 
				Pon_EO_Scan(pon_Spd,0,7,0); //20221011
				//XPON_TDC_on(); 
				XPON_RX_rxrdy();  
				XPON_BIST_setting();
				XPON_R2T_sel(nor_pma_data);  

        break;


		
        //-----------------------------------------------------------------
        default:
	    break;
    }
}



void pon_CfgPhyType (int pon_Spd)
{
	pon_WanSelInit(pon_Spd);
	
	//TXPLL_Dis	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 24,24 , 0x1 );  // rg_force_sel_da_pxp_txpll_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 16,16 , 0x0 );  // rg_force_da_pxp_txpll_en
	  
	
	// TDC	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_3, 1,0 , 0x1 );               // rg_lcpll_ncpo_shift
    //IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 11,8 , lcpll_a_tdc );              // rg_lcpll_a_tdc
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 11,8 , 0x5 );              // rg_lcpll_a_tdc
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TX_TERMCAL_SELPN, 17,16 , 0x0 );    // RG_XPON_TX_TDC_CK_SEL                      
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 24,24 , 0x1 );        // RG_XPON_RX_TDC_CK_SEL
    
	
	//PLL EN HW Mode 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_PWCTL_SETTING_1, 31,24 , 0x1 );       //rg_lcpll_ck_stb_timer      
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_PWCTL_SETTING_1, 23,16 , 0x10 );      //rg_lcpll_pcw_man_load_timer;
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_PWCTL_SETTING_1, 15,8 , 0xA );        //rg_lcpll_en_timer          
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_PWCTL_SETTING_1, 0,0 , 0x1 );         //rg lcpll_man_pwdb 
	
	
	//pma_dig_tx_setting
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_1, 31,16 , 0x113 );          // rg_tx_hsdata_en_wait
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_1, 15,0 , 0xFA );            // rg_tx_ck_en_wait  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_2, 31,16 , 0x9B );           // rg_tx_serdes_rdy_wait
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_2, 15,0 , 0x210 );           // rg_tx_power_on_wait	

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_0, 31,16 , 0x4 );            // rg_txcalib_5us
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PON_TX_COUNTER_0, 15,0 , 0x26 );            // rg_txcalib_50us	

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23,16 , 0x2 );                 // rg_tx_ben_exten_ftune

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_0, 10,8 , 0x3 );              // rg_lcpll_ki
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PW_5, 24,24 , 0x0 );              // rg_lcpll_tdc_sync_in_mode

	XPON_TXPLL(InitSpd_Pon_TXPLL);	
	XPON_TX(InitSpd_Pon_TX); 

	
	//pma_dig_rx_setting 
	//RX HW mode counter 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_0, 23,8 , 0x1 ); 	 // rg_rx_os_start
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_6, 15,0 , 0x2 ); 	 // rg_rx_os_end
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_0, 2,0 , 0x1 );		 // rg_osc_speed_opt

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 15,0 , 0x2 );      // rg_rx_pical_start
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_1, 31,16 , 0x3E8 );   // rg_rx_pical_end

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 15,0 , 0x2 );      // rg_rx_sdcal_start
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_4, 31,16 , 0x3E8 );   // rg_rx_sdcal_end

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 15,0 , 0x2 );      // rg_rx_pdos_start
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_2, 31,16 , 0x3E8 );   // rg_rx_pdos_end

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 15,0 , 0x2 );      // rg_rx_feos_start
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_3, 31,16 , 0x3E8 );   // rg_rx_feos_end
	
	
	//RX setting 	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_2, 12,8 , 0x1 );                // rg_fom_num_order
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_2, 1,0 , 0x3 );                 // rg_a_sel

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26,16 , 0x240 );  // rg_x_max
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10,0 , 0x1C0 );   // rg_x_min

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8,8 , 0x0 );        // rg_data_shift
    

	XPON_RX(InitSpd_Pon_RX); 	
	XPON_ANA(InitSpd_Pon_ANA); 	 
	
	//EYE
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 0,0 , 0x1 ); 	   // rg_eyecnt_fast
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16,16 , 0x1 );    // rg_eye_nextpts

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15,8 , 0x4 );   //rg_eyecnt_vth
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7,0 , 0x4 );    //rg_eyecnt_hth

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23,16 , 0x4 );  //rg_eo_vth
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10,0 , 0x4 );   //rg_eo_hth

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31,24 , 0xFF );     // rg_eye_mask
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9,0 , 0xD0 );       // rg_cntlen

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 7,0 , 0x1 );                   // rg_eq_en_delay
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 31,24 , 0x0 );                 // rg_veo_mask
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 18,8 , 0x0 );                  // rg_heo_mask

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_1, 0,0 , 0x1 );                   // rg_a_lgain
	
	
	//(Default)
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_1, 25,24 , 0x1 );                      // rg_cal_cyc
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_1, 15,8 , 0x2E );                      // rg_cal_1us_set
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_1, 0,0 , 0x1 );                        // rg_sim_fast_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_1, 17,16 , 0x1 );                      // rg_cal_stb

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_2, 17,16 , 0x0 );                      // rg_cal_cyc_time
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_2, 11,8 , 0x0 );                       // rg_cal_out_os
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CAL_2, 0,0 , 0x0 );                        // rg_cal_os_pulse

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 15,0 , 0x5 );        // rg_rx_blwc_rdy_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_CTRL_5, 31,16 , 0xA );       // rg_rx_rdy
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FEOS, 7,0 , 0x0 );                         // rg_lfsel 

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FEOS, 8,8 , 0x0 );                         // rg_eq_force_blwc_freeze

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 6,0 , 0x40 );     // rg_y_min
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 14,8 , 0x3F );    // rg_y_max
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16,16 , 0x1 );    // rg_index_mode
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19,0 , 0x18 );    // rg_eyedur

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EXTRAL_CTRL, 15,8 , 0x2 );                 // rg_l2d_trig_eq_en_time
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EXTRAL_CTRL, 1,1 , 0x1 );                  // rg_os_rdy_latch
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EXTRAL_CTRL, 0,0 , 0x0 );                  // rg_disb_leq
	

	//IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FLL_0, 2, 0, 0x03);   //rg_kband_prediv
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_0, 18,8 , 0xA5 );                      // rg_fpkdiv
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_0, 2,0 , 0x2 );                        // rg_kband_prediv??
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_0, 25,24 , 0x0 );                      // rg_kband_kfc 

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_1, 26,24 , 0x4 );                      // rg_symbol_wd
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_1, 18,16 , 0x1 );                      // rg_settle_time_sel
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_5, 10,0 , 0x1FF );                     // rg_fll_idac_max
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_5, 26,16 , 0x400 );                    // rg_fll_idac_min

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_2, 10,8 , 0x4 );                       // rg_amp     
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_2, 2,0 , 0x3 );                        // rg_prbs_sel

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_4, 24,24 , 0x0 );                // rg_disb_blwc_offset 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PDOS_CTRL_0, 0,0 , 0x1 );                  // rg_eye_blwc_add
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PDOS_CTRL_0, 8,8 , 0x0 );                  // rg_data_blwc_add

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_BLWC, 0,0 , 0x1 );                         // rg_eq_blwc_pol
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_BLWC, 11,8 , 0xA );                        // rg_eq_blwc_gain
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_BLWC, 22,16 , 0x70 );                      // rg_eq_blwc_cnt_top_lim
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_BLWC, 29,23 , 0x10 );                      // rg_eq_blwc_cnt_bot_lim

#if 0
	//********************************************OPTIMIZE CMN_TRIM by EFUSE*****************************************************// 	
	  PON_CMN_TRIM = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XPON_CMN_TRIM_LSB, 5) : 0x1; 
	
	  if ((PON_CMN_TRIM > 0xD) && (PON_CMN_TRIM <= 0x16)){
		 IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_EN, 28, 24, PON_CMN_TRIM);
		 
	   #if LAB_PRINT_PON
		 printk("DA_XPON_CMN_TRIM[28:24]=%x\n", PON_CMN_TRIM);
	   #endif
	  }
	  else {
		 IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_EN, 28, 24, 0x10);
		 
	   #if LAB_PRINT_PON
		 printk("Set XPON CMN_TRIM default! \n");
	   #endif
	  }	
	//*****************************************************************************************************************************//	
	
#endif	
}


void pon_WanSelInit (uint pon_Spd)
{

   switch(pon_Spd)
   {
     
	   case Async_GPON:   //DS_2p48832 / US_1p24416		 
	   
		InitSpd_Pon_TXPLL = 2; 
		InitSpd_Pon_TX = 5;
		InitSpd_Pon_RX = 3;
		InitSpd_Pon_ANA = 2;
		InitSpd_Pon_Preset = 3;

		break;	


	   case Sync_EPON_1:   //DS_1.25G      /  US_1.25G
	   case Sync_EPON_2: 

		InitSpd_Pon_TXPLL = 3; 
		InitSpd_Pon_TX = 6;
		InitSpd_Pon_RX = 5;
		InitSpd_Pon_ANA = 3;
		InitSpd_Pon_Preset = 5;

		break;	
		
	   
	   case Async_XEPON:  //DS_10.3125G  /  US_1.25G  

		InitSpd_Pon_TXPLL = 1; 
		InitSpd_Pon_TX = 8;
		InitSpd_Pon_RX = 1;
		InitSpd_Pon_ANA = 1;	
		InitSpd_Pon_Preset = 1;

		break;

	  
	   case Sync_XEPON:    //DS_10.3125G  /  US_10.3125G

		InitSpd_Pon_TXPLL = 1; 
		InitSpd_Pon_TX = 1;
		InitSpd_Pon_RX = 1;
		InitSpd_Pon_ANA = 1;	
		InitSpd_Pon_Preset = 1;

		break;
		

	   case Async_XGPON_1:   //DS_9.95328G  /  US_2.48832G
	   case Async_XGPON_2:

		InitSpd_Pon_TXPLL = 2; 
		InitSpd_Pon_TX = 2;
		InitSpd_Pon_RX = 2;
		InitSpd_Pon_ANA = 2;	
		InitSpd_Pon_Preset = 2;


		break;


	   case Sync_XGSPON_1:    //DS_9.95328G  /  US_9.95328G  
	   case Sync_XGSPON_2: 

		InitSpd_Pon_TXPLL = 2; 
		InitSpd_Pon_TX = 3;
		InitSpd_Pon_RX = 2;
		InitSpd_Pon_ANA = 2;
		InitSpd_Pon_Preset = 2;

		break;
		
	  
	   case Sync_GPON:       //DS_2.48832G  /  US_2.48832G

		InitSpd_Pon_TXPLL = 2; 
		InitSpd_Pon_TX = 4;
		
		//InitSpd_Pon_RX = 4;  //NGPON2_Sym
		InitSpd_Pon_RX = 3;   //GPON_Sym
		
		InitSpd_Pon_ANA = 2;	
		InitSpd_Pon_Preset = 4;
	

		break;

		case Turbo_EPON:    //DS_2.5G  /  US_1.25G             

		InitSpd_Pon_TXPLL = 3; 
		InitSpd_Pon_TX = 7;
		InitSpd_Pon_RX = 6;
		InitSpd_Pon_ANA = 3;			
		InitSpd_Pon_Preset = 6;
		

		break;

		case Turbo_EPON_Sym:    //DS_2.5G  /  US_2.5G             

		InitSpd_Pon_TXPLL = 3; 
		InitSpd_Pon_TX = 0xB;
		InitSpd_Pon_RX = 6;
		InitSpd_Pon_ANA = 3;			
		InitSpd_Pon_Preset = 6;
		

		break;
		
		
	    case Eth_Ser_5GBaseR:       //DS(RX)_5.15625G   /  US(TX)_5.15625G

		InitSpd_Pon_TXPLL = 1; 
		InitSpd_Pon_TX = 9; 
		InitSpd_Pon_RX = 7;
		InitSpd_Pon_ANA = 1;
		InitSpd_Pon_Preset = 7;

		break;	


	    case Eth_Ser_HSGMII:       //DS(RX)_3.125G   /  US(TX)_3.125G

		InitSpd_Pon_TXPLL = 4; 
		InitSpd_Pon_TX = 0xA; 
		InitSpd_Pon_RX = 8;
		InitSpd_Pon_ANA = 4;
		InitSpd_Pon_Preset = 8;

		break;
		
		
		

   }

}


void XPON_TXPLL(uint rate_sel)
{

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_txpll_sdm_pcw
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_3, 8,8 , 0x1 );						  // rg_lcpll_ncpo_load

switch(rate_sel)
	{
	 //TXPLL_PCW
	case PON_10p3125G_BYP_JCPLL:   
	
		 IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x33900000 );  // rg_force_da_pxp_txpll_sdm_pcw
         IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x67200000 );                // rg_lcpll_pon_hrdds_pcw_ncpo_gpon
         IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x67200000 );                // rg_lcpll_pon_hrdds_pcw_ncpo_epon
         		 

 #if LAB_PRINT_PON
         printk("=== XPON_TXPLL_10G_done ===\n");
 #endif
		  
		 break;  
		 
    case PON_9p95328G_BYP_JCPLL:   
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x31C432CA );  // rg_force_da_pxp_txpll_sdm_pcw
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x63886594 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_gpon
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x63886594 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_epon
         	 
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_9G_done ===\n");
         printk("=== XPON_TXPLL_2G_done ===\n");
 #endif
		   
		 break;	

	case PON_10G_BYP_JCPLL: 
	
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x32000000 );  // rg_force_da_pxp_txpll_sdm_pcw
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x64000000 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_gpon
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x64000000 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_epon

         		 
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_1G_done ===\n");
 #endif
		 
		 break;	

 	case PON_12p5GG_BYP_JCPLL: 
	
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x3E800000 );  // rg_force_da_pxp_txpll_sdm_pcw
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x7D000000 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_gpon
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x7D000000 );				  // rg_lcpll_pon_hrdds_pcw_ncpo_epon

		
         		 
		 
 #if LAB_PRINT_PON
		 printk("=== XPON_TXPLL_12p5G_done ===\n");
 #endif
		 
		 break;	
		
		default:
		    break;	
     }		 

}

void XFI_PON_TXFIR_Bringup_Setting(u8 cn1, u8 c0b, u8 c1, u8 c2,u8 prt)
{
    pon_cn1_set = cn1;
    pon_c0b_set = c0b;
    pon_c1_set = c1;
    pon_c2_set = c2;
    if(prt) printk("PON TXFIR Default Set as [%x, %x, %x, %x]\n",pon_cn1_set,pon_c0b_set,pon_c1_set,pon_c2_set);
}
EXPORT_SYMBOL(XFI_PON_TXFIR_Bringup_Setting);

void XPON_TX(uint tx_bit_sel)
{

    //TX_Config
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TX_CKLDO_EN, 24,24 , 0x1 );                  // RG_XPON_TX_DMEDGEGEN_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TX_CKLDO_EN, 0,0 , 0x1 );                    // RG_XPON_TX_CKLDO_EN

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_EN, 0,0 , 0x1 );                         // RG_XPON_CMN_EN   
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_EN, 18,16 , 0x4 );                       // RG_XPON_CMN_VREFSEL
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_EN, 13,8 , 0x1 );                        // RG_XPON_CMN_MPXSELTOP_DC
    //IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CMN_MODE_SEL, 8,8 , 0x1 );                             // RG_XPON_CMN_BYPASS_LPF  

    //TX_CKIN_SEL
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_acjtag_en, 24,24 , 0x1 );         // rg_force_sel_da_pxp_tx_ckin_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_acjtag_en, 16,16 , 0x1 );         // rg_force_da_pxp_tx_ckin_sel

	//TX_FIR
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 24,24 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_fir_cn1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c0b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 24,24 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c2
	
    if((tx_bit_sel == TX_Spd_10GEPON_Sym)||(tx_bit_sel == TX_Spd_NGPON2_1))
    {
        IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, pon_cn1_set);       //rg_force_da_pxp_tx_fir_cn1
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, pon_c0b_set);         //rg_force_da_pxp_tx_fir_c0b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, pon_c1_set);          //rg_force_da_pxp_tx_fir_c1
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, pon_c2_set);        //rg_force_da_pxp_tx_fir_c2
	}
	else
	{
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 20,16 , 0x0 ); 	  // rg_force_da_pxp_tx_fir_cn1
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 5,0 , 0x8 );	      // rg_force_da_pxp_tx_fir_c0b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 4,0 , 0x0 );	      // rg_force_da_pxp_tx_fir_c1
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 18,16 , 0x0 );		  // rg_force_da_pxp_tx_fir_c2
	}


switch(tx_bit_sel)
	{

	case TX_Spd_10GEPON_Sym: //DS(RX)_10.31252G  /  US(TX)_10.3125G
    case TX_Spd_NGPON2_1:    //DS(RX)_9.95328G   /  US(TX)_9.95328G

	   //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );          // rg_force_sel_da_pxp_tx_ckin_divisor
       IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x5 );          // rg_force_da_pxp_tx_ckin_divisor 


	   //TX bus width setting
	   //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 );           // rg_force_sel_da_pxp_tx_rate_ctrl
       IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x2 );           // rg_force_da_pxp_tx_rate_ctrl
       IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x2 );                      // rg_pon_tx_rate_ctrl

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23, 16, 0x16); //rg_tx_ben_extend_ftune
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 14, 8, 0xD);   //rg_tx_dly_data_ftune
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 6, 0, 0x30);   //rg_tx_dly_data_ftune
	   

	
#if LAB_PRINT_PON
			printk("=== XPON_TX_10GEPON_NGPON2_1_Sym_done ===\n");
#endif
			
			break; 


	case TX_Spd_NGPON2:   //DS_9.95328G  /	US_2.48832G 
	case TX_Spd_GPON_Sym: //DS(RX)_2.48832G   /  US(TX)_2.48832G    

	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x3 );		  // rg_force_da_pxp_tx_ckin_divisor 



	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x0 ); 		  // rg_force_da_pxp_tx_rate_ctrl
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x0 );					  // rg_pon_tx_rate_ctrl		

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23, 16, 0x8); //rg_tx_ben_extend_ftune
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 14, 8, 0x0);  //rg_tx_dly_data_ftune
        IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 6, 0, 0x8);   //rg_tx_dly_data_ftune
	   
	
#if LAB_PRINT_PON
			printk("=== XPON_TX_NGPON2_GPON_Sym_done ===\n");
#endif
			
			break; 

	
	case TX_Spd_10GEPON_Asym:    //DS(RX)_10.31252G /  US(TX)_1.25G 
	
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x1 );		  // rg_force_da_pxp_tx_ckin_divisor 
	
	
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x1 ); 		  // rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x1 );					  // rg_pon_tx_rate_ctrl

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23, 16, 0x7); //rg_tx_ben_extend_ftune
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 14, 8, 0x2);	//rg_tx_dly_data_ftune
        IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 6, 0, 0x6);   //rg_tx_dly_data_ftune
        

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_XEPON_done(Sync_XEPON) ===\n");
 #endif
		 
         break; 

 	

	case TX_Spd_GPON: //DS(RX)_2.48832G   /  US(TX)_1.24416G  

	  
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x2 );		  // rg_force_da_pxp_tx_ckin_divisor 
	    
	  
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x0 ); 		  // rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x0 );					  // rg_pon_tx_rate_ctrl

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23, 16, 0x2); //rg_tx_ben_extend_ftune
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 14, 8, 0x8);	//rg_tx_dly_data_ftune
        IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 6, 0, 0x4);   //rg_tx_dly_data_ftune

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_EPON_done(Sync_EPON) ===\n");
 #endif
		 
         break; 	


 	case TX_Spd_EPON:   //DS(RX)_1.25G       /  US(TX)_1.25G 
 	case TX_Spd_2GEPON: //DS(RX)_2.5G        /  US(TX)_1.25G 
 	case TX_Spd_2GEPON_Sym: //DS(RX)_2.5G        /  US(TX)_2.5G 

	  
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor

		if(tx_bit_sel == TX_Spd_2GEPON_Sym)
		{
	       IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x3 );		  // rg_force_da_pxp_tx_ckin_divisor 
	    }
		else
		{
		   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x2 );		  // rg_force_da_pxp_tx_ckin_divisor 
		}
	    
	  
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x1 ); 		  // rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x1 );					  // rg_pon_tx_rate_ctrl

		if(tx_bit_sel == TX_Spd_2GEPON )
		{
			IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN, 12,12 , 0x1 );		  // RG_XPON_TXPLL_PHY_CK2_SEL
		}
	    
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 23, 16, 0x7); //rg_tx_ben_extend_ftune	    
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 14, 8, 0x2);  //rg_tx_dly_ben_ftune		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 6, 0, 0x6);   //rg_tx_dly_data_ftune
		

 #if LAB_PRINT_PON
         printk("=== XPON_TX_Sync_EPON_done(Sync_EPON) ===\n");
 #endif
		 
         break; 


  	case TX_Spd_5GBaseR:   //DS(RX)_5.15625G       /  US(TX)_5.15625G  	
	  
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x4 );		  // rg_force_da_pxp_tx_ckin_divisor 
	    
	  
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x2 ); 		  // rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x2 );					  // rg_pon_tx_rate_ctrl 


 #if LAB_PRINT_PON
         printk("=== XPON_TX_Spd_5GBaseR_done ===\n");
 #endif
		 
         break; 


   	case TX_Spd_HSGMII:   //DS(RX)_5.15625G       /  US(TX)_5.15625G  	
	  
	    //TX CKsel: 1,2,3,4,5, etc=> /8.25, /8, /4, /2, /1, off
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1 );		  // rg_force_sel_da_pxp_tx_ckin_divisor
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x4 );		  // rg_force_da_pxp_tx_ckin_divisor 
	    
	  
	    //TX bus width setting
	    //TX_rate_ctrl: 0,1,2 => 8b/10b/16b
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x1 ); 		  // rg_force_da_pxp_tx_rate_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_xpon_tx_rate_ctrl, 1,0 , 0x1 );					  // rg_pon_tx_rate_ctrl 


 #if LAB_PRINT_PON
         printk("=== XPON_TX_Spd_5GBaseR_done ===\n");
 #endif
		 
         break;  
 
		 
	     default:
		    break;	
	}	


    // TX DATA toggle outside timeslot 
    //free run counter setting:
      IO_SPHYA_REG_BITS(AN7583_PON_PMA_MD32_MEM_CLK_CTRL, 1, 0, 0x03);	// BFAF38A0 free run counter clk sel
      IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_DLY_CTRL, 30, 28, 0x01);	    // BFAF37B8 sel free run counter



}



void XPON_RX(uint rx_bit_sel)
{

   uint Rfb_id ;

   
    //RX_Path_Init.	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 19,18 , 0x0 );                     // RG_XPON_RX_REV_1 
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 22,20 , 0x4 );                     // RG_XPON_RX_REV_1
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 26,24 , 0x4 );                     // RG_XPON_RX_REV_1 

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_OSCAL_FORCE, 17,8 , 0x3FF );              // RG_PXP_RX_OSCAL_FORCE

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PD_PICAL_CKD8_INV, 8,0 , 0x0 );		  // RG_XPON_CDR_PD_EDGE_DIS
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PD_PICAL_CKD8_INV, 0,0 , 0x0 );		  // RG_XPON_CDR_PD_PICAL_CKD8_INV

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_rstb, 24,24 , 0x1 );             // rg_force_sel_da_pxp_cdr_injck_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_rstb, 16,16 , 0x1 );             // rg_force_da_pxp_cdr_injck_sel


	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1 );                   // RG_XPON_RX_PHYCK_SEL
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x8 );                // RG_XPON_CDR_PR_BETA_DAC

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);        //rg_force_sel_da_pxp_rx_fe_gain_ctrl

	//FEOS_Modify    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_12, 0, 0, 0x01); //rg_dig_reserve_12_FEEOS_0
    
	get_rfb_id(&RFB_ID);	
	Rfb_id = RFB_ID.id1;

	if( Rfb_id == 2)
	{
	     if(get_serdes_interface_sel (ECNT_EVENT_SERDES_PORT_0) == ECNT_EVENT_SERDES_PON_PON)
	     {
		     IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 25,24 , 0x1 );				  // rg_bistctl_io_data_inv
		     IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 0,0 , 0x1 );				  // rg_RX_data_inv
	         //printk(" Ref_Ver = %x\n" , RFB_ID.id1);
	     }
	}
  

  switch(rx_bit_sel)
	{
	case RX_Spd_10G: // 10G_ mode 
	case RX_Spd_9G: // 9G_ mode

    //10p3125G    
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x01);        // rg_force_da_pxp_rx_fe_gain_ctrl	
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x1030 );                   // RG_XPON_RX_REV_0
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x0 );                // RG_XPON_CDR_PR_XFICK_EN    
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x1);                    //rg_dig_reserve_0  
    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x1 );						 // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );						 // rg_disb_da_XPON_RX_FE_GAIN_CTRL

    //RX PHYCK SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x0 );                // RG_XPON_RX_PHY_CK_SEL_FORCE
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 );                // RG_XPON_RX_PHY_CK_SEL

    //RX PHYCK  	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x0 ); 				  // RG_XPON_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x0 );				  // RG_XPON_RX_PHYCK_RSTB
  

    //RX OSR setting
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x0 );                  // RG_XPON_CDR_LPF_RATIO

    //RX bus width setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0 );                  // RG_XPON_RX_BUSBIT_SEL_FORCE
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0 );                  // RG_XPON_RX_BUSBIT_SEL


    //OSR_Sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 );            // rg_force_sel_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x0 );            // rg_force_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2 );                  // rg_xpon_rx_rate_ctrl

    	
#if LAB_PRINT_PON
	 printk("=== XPON_RX_10G_done ===\n");
#endif
	 
     break; 



  case RX_Spd_2G: // 2G_ mode 	
  case RX_Spd_2G_Sym: // 2G_ mode 	
  //case Sync_GPON: //DS_2.48832G  /  US_2.48832G  	

    //2p48832G    
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03);        // rg_force_da_pxp_rx_fe_gain_ctrl	
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0 );                   // RG_XPON_RX_REV_0
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x0 );                // RG_XPON_CDR_PR_XFICK_EN       
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x4);                    // rg_dig_reserve_0
    
    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x3 );						  // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );						  // rg_disb_da_XPON_RX_FE_GAIN_CTRL

    //RX PHYCK SEL
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x0 );                // RG_XPON_RX_PHY_CK_SEL_FORCE
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 );                // RG_XPON_RX_PHY_CK_SEL

    //RX PHYCK
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x0 ); 				  // RG_XPON_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x0 );				  // RG_XPON_RX_PHYCK_RSTB

    //RX OSR setting
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x2 );                  // RG_XPON_CDR_LPF_RATIO


	//RX bus width setting
    if(rx_bit_sel == RX_Spd_2G)
    {       
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x1 ); 			  // RG_XPON_RX_BUSBIT_SEL_FORCE
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x1 ); 			  // RG_XPON_RX_BUSBIT_SEL 
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2 );               // rg_xpon_rx_rate_ctrl
    }
	else
	{
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0 );			  // RG_XPON_RX_BUSBIT_SEL_FORCE
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0 );			  // RG_XPON_RX_BUSBIT_SEL
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0 );			  // rg_xpon_rx_rate_ctrl
	}

    //OSR_Sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 );            // rg_force_sel_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x2 );            // rg_force_da_pxp_rx_osr_sel
	


#if LAB_PRINT_PON
     printk("=== XPON_RX_2G__2G_Sym_done ===\n");
#endif
     break;  


   case RX_Spd_turbo_EPON: //DS(RX)_2.5G         /  US(TX)_1.25G   
   
	   //2p5G	
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03);        // rg_force_da_pxp_rx_fe_gain_ctrl	
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0 );                   // RG_XPON_RX_REV_0
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1 );				 // RG_XPON_CDR_PR_XFICK_EN	   
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x3);                    // rg_dig_reserve_0 	   

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x3 );						 // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );						 // rg_disb_da_XPON_RX_FE_GAIN_CTRL

	   
	   //RX PHYCK SEL
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1 );				 // RG_XPON_RX_PHY_CK_SEL_FORCE
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 );				 // RG_XPON_RX_PHY_CK_SEL
   
	   //RX PHYCK	   
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x28 );					 // RG_XPON_RX_PHYCK_DIV
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1 );				 // RG_XPON_RX_PHYCK_RSTB
	 
   
	   //RX OSR setting
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x2 ); 				 // RG_XPON_CDR_LPF_RATIO
   
	   //RX bus width setting
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0 ); 				 // RG_XPON_RX_BUSBIT_SEL_FORCE
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0 ); 				 // RG_XPON_RX_BUSBIT_SEL
   
   
	   //OSR_Sel
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 );			 // rg_force_sel_da_pxp_rx_osr_sel
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x2 );			 // rg_force_da_pxp_rx_osr_sel
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0 ); 				 // rg_xpon_rx_rate_ctrl
   
		   
#if LAB_PRINT_PON
		printk("=== XPON_RX_turbo_EPON_done ===\n");
#endif
		
		break; 




   case RX_Spd_1G: // 1G_ mode	
   
    //1p25G
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03);        //rg_force_da_pxp_rx_fe_gain_ctrl	
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0 );                   // RG_XPON_RX_REV_0
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1 );				  // RG_XPON_CDR_PR_XFICK_EN
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x3);                    //rg_dig_reserve_0  


	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x3 );                        // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );                         // rg_disb_da_XPON_RX_FE_GAIN_CTRL

	
	//RX PHYCK SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1 );				  // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 );				  // RG_XPON_RX_PHY_CK_SEL
	
	//RX PHYCK		
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x29 );				  // RG_XPON_RX_PHYCK_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1 );				  // RG_XPON_RX_PHYCK_RSTB
	
	
	//RX OSR setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x3 );				  // RG_XPON_CDR_LPF_RATIO
	
	//RX bus width setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0 );				  // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0 );				  // RG_XPON_RX_BUSBIT_SEL
	
	
	//OSR_Sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 );			  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x3 );			  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0 );				  // rg_xpon_rx_rate_ctrl



#if LAB_PRINT_PON
     printk("=== XPON_RX_1G_done ===\n");
#endif

	 break; 	


  case RX_Spd_5GBaseR: // 5G_ mode  
		
	//5p15625G
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03);        //rg_force_da_pxp_rx_fe_gain_ctrl	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0 );				      // RG_XPON_RX_REV_0		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x4);					  //rg_dig_reserve_0  	
	 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x3 ); 					      // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );						  // rg_disb_da_XPON_RX_FE_GAIN_CTRL

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x2 );                   // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1 ); 			      // RG_XPON_CDR_PR_XFICK_EN	 
		 
	//RX PHYCK SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1 ); 			      // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 ); 			      // RG_XPON_RX_PHY_CK_SEL		 
		 
	//RX PHYCK 	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1 );				  // RG_XPON_RX_PHYCK_RSTB
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x42 );				  // RG_XPON_RX_PHYCK_DIV		 
		 
	//RX OSR setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x1 );				  // RG_XPON_CDR_LPF_RATIO
		 
	//RX bus width setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x1 );				  // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x1 );				  // RG_XPON_RX_BUSBIT_SEL		 
		 
	//OSR_Sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 ); 		      // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x1 ); 		      // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2 );				  // rg_xpon_rx_rate_ctrl
	 
	 
	 
#if LAB_PRINT_PON
    printk("=== XPON_RX_Spd_5GBaseR_done ===\n");
#endif
	 
		  break;  


 case RX_Spd_HSGMII: // 3.125G_ mode  
			 
	//3p125G
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x03);        //rg_force_da_pxp_rx_fe_gain_ctrl	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0 );				      // RG_XPON_RX_REV_0 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 10, 8, 0x3);                    //rg_dig_reserve_0  
		  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 1,0 , 0x3 );						  // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 0,0 , 0x0 ); 						  // rg_disb_da_XPON_RX_FE_GAIN_CTRL

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x6 );                // RG_XPON_CDR_PR_BETA_DAC
	 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1 );				      // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1 );				  // RG_XPON_CDR_PR_XFICK_EN	  
			  
	//RX PHYCK SEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1 );				  // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0 );				  // RG_XPON_RX_PHY_CK_SEL	  
			  
	//RX PHYCK	 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1 ); 				  // RG_XPON_RX_PHYCK_RSTB
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0xB );				      // RG_XPON_RX_PHYCK_DIV 	  
			  
	//RX OSR setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x1 );				  // RG_XPON_CDR_LPF_RATIO
			  
	//RX bus width setting
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0 );				  // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0 );				  // RG_XPON_RX_BUSBIT_SEL
			  
			  
	//OSR_Sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1 );			  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x1 );			  // rg_force_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0 );				  // rg_xpon_rx_rate_ctrl
		  
		  
		  
#if LAB_PRINT_PON
    printk("=== XPON_RX_Spd_HSGMII_done ===\n");
#endif
		  
			   break; 


	 default:
		break;	
	}

}


void XPON_ANA(uint rate_sel)
{

    //XPON, TDC
 	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_PLL_MONCLK_SEL, 24,24 , 0x1 );               // RG_XPON_TDC_AUTOEN  	

    //TXPLL_bring_up
    //TXPLL_VCO_LDO_Out
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_PERIOD, 25,24 , 0x1 );			  // RG_XPON_TXPLL_LDO_VCO_OUT
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_PERIOD, 17,16 , 0x1 );			  // RG_XPON_TXPLL_LDO_OUT 

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VTP_EN, 25,24 , 0x0 );                 // RG_PXP_TXPLL_LDO_VCO_VTP
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VTP_EN, 16,16 , 0x0 );                 // RG_PXP_TXPLL_LDO_VCO_VTP_EN

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VTP_EN, 10,8 , 0x0 );                  // RG_XPON_TXPLL_VTP
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VTP_EN, 0,0 , 0x1 );                   // RG_XPON_TXPLL_VTP_EN

	//TXPLL_LDO
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TDC_SYNC_CK_SEL, 17,16 , 0x1 );			  // RG_XPON_PLL_LDO_CKDRV_VSEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TDC_SYNC_CK_SEL, 8,8 , 0x1 );				  // RG_XPON_PLL_LDO_CKDRV_EN


    //TXPLL_RSTB
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 24,24 , 0x1 );         // RG_XPON_TXPLL_PLL_RSTB
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 18,16 , 0x4 );         // RG_XPON_TXPLL_RST_DLY
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 0,0 , 0x0 );           // RG_XPON_TXPLL_REFIN_INTERNAL
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 9,8 , 0x0 );           // RG_XPON_TXPLL_REFIN_DIV
	

    //TXPLL_SDM
	if(rate_sel == PON_12p5GG_BYP_JCPLL)
	{		
	    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 16,16 , 0x0);           // RG_XPON_TXPLL_SDM_IFM
        IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_ORD, 16,16 , 0x0);             // RG_XPON_TXPLL_SDM_HREN  
        IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x0 );		      // RG_XPON_TXPLL_SDM_DI_EN   
	}
	else
	{        
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 16,16 , 0x1 );	      // RG_XPON_TXPLL_SDM_IFM
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_ORD, 16,16 , 0x1 );		      // RG_XPON_TXPLL_SDM_HREN
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x1 );	          // RG_XPON_TXPLL_SDM_DI_EN		
	}
	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 9,8 , 0x0 );			      // RG_XPON_TXPLL_SDM_DI_LS	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_DI_EN, 25,24 , 0x0 );              // RG_XPON_TXPLL_SDM_MODE  
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_ORD, 8,8 , 0x0 );                  // RG_XPON_TXPLL_SDM_OUT
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_ORD, 1,0 , 0x3 );                  // RG_XPON_TXPLL_SDM_ORD
	

    //TXPLL_SSC
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_DELTA1, 31,16 , 0x0 );			  // RG_XPON_TXPLL_SSC_DELTA
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_DELTA1, 15,0 , 0x0 ); 			  // RG_XPON_TXPLL_SSC_DELTA1
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_EN, 16,16 , 0x0 );				  // RG_XPON_TXPLL_SSC_TRI_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_EN, 8,8 , 0x0 );					  // RG_XPON_TXPLL_SSC_PHASE_INI
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_EN, 0,0 , 0x0 );					  // RG_XPON_TXPLL_SSC_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SSC_PERIOD, 15,0 , 0x0 ); 			  // RG_XPON_TXPLL_SSC_PERIOD

   

   if(rate_sel == PON_12p5GG_BYP_JCPLL)
   {
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_DOUBLE_EN, 0,0 , 0x1 );		  // TXPLL_SPARE_L(ICHP_DOUBLE)	   
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 5,0 , 0x1E );			  // RG_XPON_TXPLL_CHP_IBIAS
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 20,16 , 0x5 ); 		  // RG_XPON_TXPLL_LPF_BR
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_LPF_BP, 12,8 , 0xB ); 			  // RG_XPON_TXPLL_LPF_BWR
	   
   }
   else
   {
      IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_DOUBLE_EN, 0,0 , 0x0 );          // TXPLL_SPARE_L(ICHP_DOUBLE)      
      IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 5,0 , 0x18 );			  // RG_XPON_TXPLL_CHP_IBIAS
      IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 20,16 , 0xA ); 			  // RG_XPON_TXPLL_LPF_BR
      IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_LPF_BP, 12,8 , 0x16 ); 			  // RG_XPON_TXPLL_LPF_BWR
   }
   
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 28,24 , 0x1F );			  // RG_XPON_TXPLL_LPF_BC   
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_CHP_IBIAS, 13,8 , 0x0 );				  // RG_XPON_TXPLL_CHP_IOFST   
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_LPF_BP, 20,16 , 0x18 );				  // RG_XPON_TXPLL_LPF_BWC   
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_LPF_BP, 4,0 , 0x2 );					  // RG_XPON_TXPLL_LPF_BP 
   
 

 //TXPLL_VCO
 if (rate_sel == PON_10p3125G_BYP_JCPLL)
 {
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x2 );             // RG_XPON_TXPLL_VCO_CFIX
 }
 else if ((rate_sel == PON_9p95328G_BYP_JCPLL)||(rate_sel == PON_10G_BYP_JCPLL))
 {	
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x3 );             // RG_XPON_TXPLL_VCO_CFIX
 }
 else
 {
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x0 );			   // RG_XPON_TXPLL_VCO_CFIX
 }


	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 0,0 , 0x1 );           // RG_XPON_TXPLL_VCO_HALFLSB_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 10,8 , 0x7 );          // RG_XPON_TXPLL_VCO_SCAPWR
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 26,24 , 0x4 );         // RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 29,27 , 0x0 );         // RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 18,16 , 0x4 );         // RG_XPON_TXPLL_VCO_TCLVAR

    //TXPLL_Kband
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_CODE, 25,24 , 0x3 );			  // RG_XPON_TXPLL_KBAND_KF
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_CODE, 17,16 , 0x0 );			  // RG_XPON_TXPLL_KBAND_KFC
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_CODE, 10,8 , 0x2 );			  // RG_XPON_TXPLL_KBAND_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_KS, 1,0 , 0x1 );				  // RG_XPON_TXPLL_KBAND_KS

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_CODE, 7,0 , 0xE4 );              // RG_XPON_TXPLL_KBAND_CODE
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_LPF_BP, 24,24 , 0x0 );                 // RG_XPON_TXPLL_KBAND_OPTION
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 24,24 , 0x0 );         // RG_XPON_TXPLL_VCO_KBAND_MEAS_EN
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_kband_load_en, 8,8 , 0x1 );    // rg_force_sel_da_pxp_txpll_kband_load_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_kband_load_en, 0,0 , 0x0 );    // rg_force_da_pxp_txpll_kband_load_en	


    //TXPLL_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_KS, 17,16 , 0x0 ); 			  // RG_XPON_TXPLL_MMD_PREDIV_MODE
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_KBAND_KS, 8,8 , 0x1 );				  // RG_XPON_TXPLL_POSTDIV_EN


    //TXPLL_TCL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 4,0 , 0xF ); 		  // RG_XPON_TXPLL_TCL_KBAND_VREF
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 2,0 , 0x3 );			  // RG_XPON_TXPLL_TCL_AMP_GAIN
	
	if(rate_sel == PON_12p5GG_BYP_JCPLL)
    {
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 12,8 , 0xE );	      // RG_XPON_TXPLL_TCL_AMP_VREF
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 17,16 , 0x1 );		  // RG_XPON_TXPLL_VCODIV
	}
	else
	{
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 12,8 , 0xB );	      // RG_XPON_TXPLL_TCL_AMP_VREF
		IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 17,16 , 0x0 );			  // RG_XPON_TXPLL_VCODIV
	}
	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 10,8 , 0x0 );			  // RG_XPON_TXPLL_TCL_LPF_BW
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 0,0 , 0x1 ); 			  // RG_XPON_TXPLL_TCL_LPF_EN
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_SDM_ORD, 24,24 , 0x1 );				  // RG_XPON_TXPLL_TCL_AMP_EN


    //TX_TERMCAL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TX_TXLBRX_EN, 18,16 , 0x2 );                 // RG_XPON_TX_TERMCAL_VREF_H
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TX_TXLBRX_EN, 26,24 , 0x2 );                 // RG_XPON_TX_TERMCAL_VREF_L


    //XPON_RX
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_FE_EQ_HZEN, 24,24 , 0x1 );				  // RG_PXP_RX_FE_VB_EQ3_EN 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_FE_EQ_HZEN, 16,16 , 0x1 );				  // RG_PXP_RX_FE_VB_EQ2_EN 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_FE_EQ_HZEN, 8,8 , 0x1 );				  // RG_PXP_RX_FE_VB_EQ1_EN 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_FE_EQ_HZEN, 0,0 , 0x0 );				  // RG_PXP_RX_FE_EQ_HZEN 
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_FE_VCM_GEN_PWDB, 0,0 , 0x1 );			  // RG_XPON_RX_FE_VCM_GEN_PWDB

	
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 26,8 , 0x8000 );              // RG_XPON_CDR_LPF_TOP_LIM
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_BOT_LIM, 18,0 , 0x78000 );           // RG_XPON_CDR_LPF_BOT_LIM
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_CKREF_DIV, 16,16 , 0x0 );             // RG_XPON_CDR_PR_RSTB_BYPASS
    //IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_DAC_RANGE, 9,8 , 0x0 );                   // RG_XPON_RX_DAC_RANGE_EYE

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_DAC_RANGE, 9, 8, 0x02); //RG_PXP_RX_DAC_RANGE_EYE , full_eye
 

#if LAB_PRINT_PON
  printk("=== XPON_ANA_done ===\n");
#endif

}



void XPON_TXPLL_on(void)
{

	// LCPLL_force_on 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 24,24 , 0x1 );       // rg_force_sel_da_pxp_txpll_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 16,16 , 0x1 );       // rg_force_da_pxp_txpll_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_PWCTL_SETTING_0, 24,24 , 0x1 );                // rg_sw_lcpll_en
	
    udelay(6);    

    //Re-Setting TXPLL for Kband
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 8,8 , 0x1 );         // rg_force_sel_da_pxp_txpll_ckout_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 0,0 , 0x1 );         // rg_force_da_pxp_txpll_ckout_en
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 16,16 , 0x1 );         // RG_XPON_TXPLL_FREQ_MEAS_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 8,8 , 0x0 );           // RG_XPON_TXPLL_VREF_SEL

    //TXPLL_Out
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN, 8,8 , 0x1 );               // RG_XPON_TXPLL_PHY_CK2_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN, 0,0 , 0x1 );               // RG_XPON_TXPLL_PHY_CK1_EN


	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 16,16 , 0x0 );         // RG_XPON_TXPLL_FREQ_MEAS_EN
    IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_TXPLL_IB_EXT_EN, 24,24 , 0x0 );              // RG_XPON_TXPLL_IB_EXT_EN

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
/*
     //-------------------------------------------
     //CDR_FB_CK frequency meter setting    

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0x9F43);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0x9E7A);   //rg_lock_cyclecnt

	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0x9F43);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0x9E7A);   //rg_unlock_target_beg
*/	 
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
/*
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0xA4FF);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0xA436);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0xA4FF);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0xA436);   //rg_unlock_target_beg
*/	 
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
/*
     //-------------------------------------------
     //CDR_FB_CK frequency meter setting
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 31, 16, 0x7FFF);  //rg_unlock_cyclecnt
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_1, 15, 0, 0x7FFF);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 31, 16, 0xA43A);  //rg_lock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_2, 15, 0, 0xA371);   //rg_lock_cyclecnt
	 
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 31, 16, 0xA43A);  //rg_unlock_target_end
	 IO_SPHYA_REG_BITS(EN7581_XPON_PMA_SS_RX_FREQ_DET_3, 15, 0, 0xA371);   //rg_unlock_target_beg 
*/		
     break; 


default:
	   break;  

	} 	  


#if LAB_PRINT_PON
   printk("=== XPON_FM_On_done ===\n");
#endif

}






void XPON_TX_on(void)
{
  //UINT32 read_data;

   // TX_on
   //controlled by DA_TX_CK_EN or DA_TX_HSDATA_EN
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_RST_B, 0,0 , 0x1 );                               // tx_top_rst_b


   //3750 , CLK PATH EN 
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_CLKPATH_RST_0, 8,8 , 0x1 );                      // rg_clkpath_rstb_ck
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_CLKPATH_RST_0, 0,0 , 0x1 );                      // rg_clkpath_rst_en

   //3550 , TX CAL RST  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_RST_B, 8,8 , 0x1 );								 // txcalib_rst_b
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_RST_B, 0,0 , 0x1 );								 // tx_top_rst_b
   
   //IO_SPHYA_REG_BITS(EN7581_XPON_PMA_ADD_XPON_MODE_1, 16, 16, 0x01); //rg_tx_bist_gen_en                                                   
                                   
   udelay(100);
 //********************************************OPTIMIZE TX IMPEDANCE by EFUSE*****************************************************//
    Pon_tx_term_sel = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XPON_TX_TERM_SEL_LSB, 2) : 0xF;
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_CALIB_0, 16, 16, 0x1);	//force P enable,

	if ((Pon_tx_term_sel > 0) && (Pon_tx_term_sel <= 3)){
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_CALIB_0, 25, 24, Pon_tx_term_sel);
	   
	 #if LAB_PRINT_PON
	   printk("DA_XPON_TX_TERMP_SEL[1:0]=%x\n", Pon_tx_term_sel);
	 #endif
	}
	else {
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_TX_CALIB_0, 25, 24, 0x1);
	   
	 #if LAB_PRINT_PON
	   printk("Set TXP Impedance Level 1 as default! \n");
	 #endif
	}

//*****************************************************************************************************************************//


#if LAB_PRINT_PON
   printk("=== XPON_TX_On_done ===\n");
#endif

}


void XPON_RX_preset(uint xfi_preset)
{

   // ** RX_precondition
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_SIGDET_NOVTH, 9,8 , 0x2 );				// RG_XPON_RX_SIGDET_PEAK
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_SIGDET_NOVTH, 20,16 , 0x2 ); 			// RG_XPON_RX_SIGDET_VTH_SEL
   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_DAC_RANGE, 25,24 , 0x3 );				// RG_XPON_RX_SIGDET_LPF_CTRL

   if(xfi_preset == RX_Spd_HSGMII)
   {
       IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 19,19 , 0x1 ); 			// RG_XPON_CDR_PR_CAP_EN
       IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 18,16 , 0x6 ); 			// RG_XPON_CDR_PR_BUF_IN_SR   
   }
   else
   {
       IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 19,19 , 0x0 ); 			// RG_XPON_CDR_PR_CAP_EN
       IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 18,16 , 0x7 ); 			// RG_XPON_CDR_PR_BUF_IN_SR   
   }


   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0 );         // rg_force_rx_os_rdy
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0 );          // rg_disb_rx_os_rdy
												   
   //L2R
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x0 );		// rg_force_sel_da_pxp_cdr_lpf_lck2data
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1 );		// rg_force_da_pxp_cdr_lpf_lck2data
   
                                                   
   //LEQ setting
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 24,24 , 0x1 );			// rg_force_sel_da_pxp_rx_fe_peaking_ctrl
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 19,16 , 0x0 );			// rg_force_da_pxp_rx_fe_peaking_ctrl   

   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);       //rg_force_sel_da_pxp_rx_fe_gain_ctrl
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x01);       //rg_force_da_pxp_rx_fe_gain_ctrl   
                                                 
   //keep EYE reset
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 8,8 , 0x1 );						// rg_force_eye_reset_plu_o
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 8,8 , 0x0 );						// rg_disb_eye_reset_plu_o
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 16,16 , 0x0 ); 					// rg_force_eye_top_en
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 16,16 , 0x0 );						// rg_disb_eye_top_en
   

   //keep BLWC reset
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 0,0 , 0x0 );                        // rg_disb_blwc_rx_rst_b
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 0,0 , 0x0 );                       // rg_force_blwc_rx_rst_b 
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16,16 , 0x1 );        // rg_disb_rx_blwc_en
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 16,16 , 0x1 );       // rg_force_rx_blwc_en


   
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_Preset_done ===\n");
#endif

}


void XPON_TDC_off(void)
{
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x01); //rg_force_sel_da_pxp_txpll_sdm_pcw

    //TDC   
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_3, 8,8 , 0x1 );						// rg_lcpll_ncpo_load	  

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x01);  //rg_force_sel_da_pxp_txpll_sdm_pcw_chg
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x00);  //rg_force_da_pxp_txpll_sdm_pcw_chg
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x01);  //rg_force_da_pxp_txpll_sdm_pcw_chg

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 0, 0, 0x0);                       //rg_lcpll_gpon_sel
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PW_0, 0,0 , 0x0 );						// rg_lcpll_tdc_dig_pwdb
    udelay(1000);

#if LAB_PRINT_PON
   printk("=== XPON_TDC_Off_done ===\n");
#endif

}



void XPON_XFI_RX_Setting(uint pon_Spd)
{
  if(pon_Spd == Sync_XEPON) 
  {
	  IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x42);			// RG_XPON_RX_PHYCK_DIV
	  IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x2);            // RG_XPON_RX_PHYCK_SEL
	  IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);          // RG_XPON_RX_PHYCK_RSTB
	  IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);         // RG_XPON_CDR_PR_XFICK_EN

	  IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);         // RG_XPON_RX_PHY_CK_SEL_FORCE
      IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);         // RG_XPON_RX_PHY_CK_SEL
  }
}



void XPON_RX_on(void)
{

	//int freq_lock = 0;


    // RX_on/ 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 24,24 , 0x1 );	// rg_force_sel_da_pxp_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16,16 , 0x1 );	// rg_force_da_pxp_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8,8 , 0x1 );	// rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0,0 , 0x1 );	// rg_force_da_pxp_cdr_pr_pieye_pwdb
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 24,24 , 0x0 );		// rg_force_sel_da_pxp_cdr_pr_kband_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 16,16 , 0x0 );		// rg_force_da_pxp_cdr_pr_kband_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 8,8 , 0x1 );			// rg_force_sel_da_pxp_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 0,0 , 0x1 );			// rg_force_da_pxp_cdr_pd_pwdb
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 8,8 , 0x1 );			// rg_force_sel_da_pxp_rx_fe_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 0,0 , 0x1 );			// rg_force_da_pxp_rx_fe_pwdb
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_scan_rst_b, 24,24 , 0x1 );		// rg_force_sel_da_pxp_rx_sigdet_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_scan_rst_b, 16,16 , 0x1 );		// rg_force_da_pxp_rx_sigdet_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_scan_rst_b, 8,8 , 0x0 );		// rg_force_sel_da_pxp_rx_scan_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_scan_rst_b, 0,0 , 0x0 );		// rg_force_da_pxp_rx_scan_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1 );	// rg_force_sel_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );	// rg_force_da_pxp_cdr_lpf_rstb

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 24,24 , 0x1 ); 					// rg_da_xpon_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 16,16 , 0x1 ); 					// rg_da_xpon_cdr_pr_pieye_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 8,8 , 0x1 );						// rg_da_xpon_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 0,0 , 0x1 );						// rg_da_xpon_rx_fe_pwdbb



    //RX_SigDet_Pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_1, 0,0 , 0x1 );						//rg_da_xpon_rx_sigdet_pwdb 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_RX_SYS_EN_SEL_0, 1,0 , 0x1 );                  // rg_da_rx_sys_en_sel	

	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 25,24 , 0x0 );		//RG_XPON_CDR_PR_FBKSEL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 20,16 , 0x8 );		//RG_XPON_CDR_PR_DAC_BAND
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 10,8 , 0x5 );		//RG_XPON_CDR_PR_VREG_CKBUF_VAL
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 2,0 , 0x5 );		//RG_XPON_CDR_PR_VREG_IBAND_VAL

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8,8 , 0x0 ); 		// rg_disb_rx_pical_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16,16 , 0x0 );		// rg_disb_rx_pdos_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24,24 , 0x0 );		// rg_disb_rx_feos_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0,0 , 0x0 ); 		// rg_disb_rx_sdcal_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0 ); 		// rg_disb_rx_os_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16,16 , 0x0 );		// rg_disb_rx_blwc_en


    //Disable_AEQ
 	//IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_en, 8, 8, 0x01);      //rg_force_sel_da_pxp_aeq_en
	//IO_SPHYA_REG_BITS(EN7581_XPON_PMA_rg_force_da_pxp_aeq_en, 0, 0, 0x00);      //rg_force_da_pxp_aeq_en
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_CKREF_DIV, 1,0 , 0x0 ); 			// RG_XPON_CDR_PR_CKREF_DIV
	IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_PR_TDC_REF_SEL, 25,24 , 0x0 ); 		// RG_XPON_CDR_PR_CKREF_DIV1
	
 

    //RX_RSTB
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x1 );                           // rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x1 );                           // rg_sw_ref_rst_n

    //CDR_LPF_RSTB  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0 );   // rg_force_da_pxp_cdr_lpf_rstb
    udelay(100);
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );	// rg_force_da_pxp_cdr_lpf_rstb





#if LAB_PRINT_PON
   printk("=== XPON_RX_On_done ===\n");
#endif

}


void XPON_RX_L2R(void)
{

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x0 );     //rg_force_da_pxp_cdr_lpf_lck2data
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1 );     //rg_force_sel_da_pxp_cdr_lpf_lck2data

	udelay(100);

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1 );   //rg_force_sel_da_pxp_cdr_lpf_rstb 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0 );   //rg_force_da_pxp_cdr_lpf_rstb  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );   //rg_force_da_pxp_cdr_lpf_rstb   

    

#if LAB_PRINT_PON
   printk("=== XPON_RX_L2R_done ===\n");
#endif

}


void XPON_RX_OSCal(void)
{

    //FBCK_Lock
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 0,0 , 0x0 );                        // rg_disb_fbck_lock (0:force_mode / 1:normal_mode)
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 0,0 , 0x1 );                       // rg_force_fbck_lock


    //RX_OSCal_En
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24,24 , 0x1 ); // rg_force_sel_da_pxp_rx_oscal_ckon
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16,16 , 0x1 ); // rg_force_da_pxp_rx_oscal_ckon


	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 24,24 , 0x1 );		 // rg_force_sel_da_pxp_rx_oscal_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 16,16 , 0x1 );		 // rg_force_da_pxp_rx_oscal_rstb
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1 );			 // rg_force_sel_da_pxp_rx_oscal_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x1 );			 // rg_force_da_pxp_rx_oscal_en	
    udelay(200);

    //set_normal_or_force_mode
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0 );          // rg_disb_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0 );          // rg_disb_rx_os_rdy

    //disable_force_mode_signal
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0 );         // rg_force_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0 );         // rg_force_rx_os_rdy

    //release_reset_Enable
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x1 );         // rg_force_rx_os_en

}



void XPON_RX_pical(void)
{

    //PICAL
    //pre-condition  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 24,24 , 0x1 );                     // rg_disb_da_xpon_cdr_pr_pieye
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PI_CAL, 10,8 , 0x4 );                           // rg_kpgain
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 7,0 , 0x8 );                        // rg_eq_en_delay   


    //reset block  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 16,16 , 0x0 ); 						// rg_eq_pi_cal_rst_b	

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 8,8 , 0x0 );						// rg_force_rx_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_6, 8,8 , 0x0 );						// rg_disb_rx_and_pical_rstb

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 0,0 , 0x0 );						// rg_force_ref_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_6, 0,0 , 0x0 );						// rg_disb_ref_and_pical_rstb

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_3, 0,0 , 0x0 );                       // rg_disb_eq_pi_cal_rdy

    //enable  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8,8 , 0x0 );						// rg_force_rx_or_pical_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_5, 24,24 , 0x0 ); 					// rg_disb_rx_or_pical_en	

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x0 );		// rg_force_rx_pical_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8,8 , 0x0 ); 		// rg_disb_rx_pical_en

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_3, 0,0 , 0x0 );                      // rg_force_eq_pi_cal_rdy 

    //release reset & enable
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 16,16 , 0x1 ); 						// rg_eq_pi_cal_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 8,8 , 0x1 );						// rg_force_rx_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 0,0 , 0x1 );						// rg_force_ref_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8,8 , 0x1 );						// rg_force_rx_or_pical_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x1 );		// rg_force_rx_pical_en	
    udelay(200);

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x0 );        // rg_force_rx_pical_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8,8 , 0x0 );                      // rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_3, 0,0 , 0x1 );                      // rg_force_eq_pi_cal_rdy

#if LAB_PRINT_PON
   printk("=== XPON_RX_Pical_done ===\n");
#endif

}


void XPON_RX_pdos(void)
{

    //PDOS
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 24,24 , 0x1 ); 		// rg_force_sel_da_pxp_rx_pdoscal_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 16,16 , 0x1 ); 		// rg_force_da_pxp_rx_pdoscal_en  

    //*** pre-condition
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0 );		// rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0 ); 		// rg_disb_rx_os_rdy
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_1, 8,8 , 0x1 );						// rg_disb_da_xpon_rx_dac_d0
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_1, 16,16 , 0x1 ); 					// rg_disb_da_xpon_rx_dac_d1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_1, 24,24 , 0x1 ); 					// rg_disb_da_xpon_rx_dac_e0
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 0,0 , 0x1 );						// rg_disb_da_xpon_rx_dac_e1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 8,8 , 0x1 );						// rg_disb_da_xpon_rx_dac_eye

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 0,0 , 0x0 );                      // rg_force_blwc_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 0,0 , 0x0 );                       // rg_disb_blwc_rx_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24,24 , 0x0 );			// rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8,8 , 0x0 ); 			// rg_disb_eyedur_init_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 24,24 , 0x0 );                    // rg_force_eyecnt_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 24,24 , 0x0 );                     // rg_disb_eyecnt_rx_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16,16 , 0x0 );           // rg_force_eyedur_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0,0 , 0x0 );             // rg_disb_eyedur_en
  

    //*** setting 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PDOS_CTRL_0, 18,16 , 0x2 );                     // rg_sap_sel


    //*** seq
    //reset
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 16,16 , 0x0 );                    // rg_force_pdos_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_6, 16,16 , 0x0 );                     // rg_disb_pdos_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 0,0 , 0x0 );                           // rg_pdos_rst_b


    //disable  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x0 );		// rg_force_rx_pdos_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16,16 , 0x0 );		// rg_disb_rx_pdos_en


    //release reset & enable  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0 );		// rg_force_rx_os_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0 ); 		// rg_disb_rx_os_en	

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 16,16 , 0x1 );					// rg_force_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 0,0 , 0x1 );							// rg_pdos_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x1 );		// rg_force_rx_pdos_en	
    udelay(200);
                                                 
    //disable  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x0 );      // rg_force_rx_pdos_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0 );        // rg_force_rx_os_en

    //release eye related
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24,24 , 0x0 );			// rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8,8 , 0x1 ); 			// rg_disb_eyedur_init_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 24,24 , 0x0 );					// rg_force_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 24,24 , 0x1 ); 					// rg_disb_eyecnt_rx_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16,16 , 0x0 );           // rg_force_eyedur_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0,0 , 0x1 );             // rg_disb_eyedur_en

    //Disable_PDOS
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 24,24 , 0x1 );         // rg_force_sel_da_pxp_rx_pdoscal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 16,16 , 0x0 );         // rg_force_da_pxp_rx_pdoscal_en


#if LAB_PRINT_PON
   printk("=== XPON_RX_PDOS_done ===\n");
#endif

}


void XPON_RX_feos(void)
{

    //FEOS
    //*** pre-condition
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0 );		// rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0 ); 		// rg_disb_rx_os_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 16,16 , 0x1 ); 					// rg_disb_da_xpon_rx_fe_vos

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 0,0 , 0x0 );                      // rg_force_blwc_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 0,0 , 0x0 );                       // rg_disb_blwc_rx_rst_b

    //*** setting 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FEOS, 7,0 , 0x30 );                             // rg_lfsel

    //*** seq
    //reset
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 16,16 , 0x0 );					// rg_force_feos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 16,16 , 0x0 ); 					// rg_disb_feos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 8,8 , 0x0 );							// rg_feos_rst_b	
                                                  
    //disable
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x0 );		// rg_force_rx_feos_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24,24 , 0x0 );		// rg_disb_rx_feos_en
	

    //release reset & enable
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x1 );        // rg_force_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0 );         // rg_disb_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 16,16 , 0x1 );                    // rg_force_feos_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 8,8 , 0x1 );                           // rg_feos_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x1 );      // rg_force_rx_feos_en
    udelay(200);

    //disable  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x0 );      // rg_force_rx_feos_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0 );        // rg_force_rx_os_en


#if LAB_PRINT_PON
   printk("=== XPON_RX_FEOS_done ===\n");
#endif

}


void XPON_RX_sdcal(void)
{

    //SIGDET_Calibration
    //*** pre-condition
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8,8 , 0x1 );     // rg_force_sel_da_pxp_rx_sigdet_cal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0,0 , 0x1 );     // rg_force_da_pxp_rx_sigdet_cal_en

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1 );          // rg_force_sel_da_pxp_rx_oscal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x1 );          // rg_force_da_pxp_rx_oscal_en
  
  
    //reset
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 24,24 , 0x0 );                         // rg_cal_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 8,8 , 0x0 );                      // rg_force_sdcal_ref_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0,0 , 0x0 );         // rg_disb_rx_sdcal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 8,8 , 0x0 );                       // rg_disb_sdcal_ref_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x0 );        // rg_disb_sdcal_ref_rst_b
  

    //release reset & enable
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 24,24 , 0x1 ); 						// rg_cal_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 8,8 , 0x1 );						// rg_force_sdcal_ref_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x1 );		// rg_force_rx_sdcal_en
    udelay(200);

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x0 );        // rg_force_rx_sdcal_en

    //Disable_SDCal
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8,8 , 0x1 );     // rg_force_sel_da_pxp_rx_sigdet_cal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0,0 , 0x0 );     // rg_force_da_pxp_rx_sigdet_cal_en

  
    //RX_OSCal_Dis
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24,24 , 0x1 );  // rg_force_sel_da_pxp_rx_oscal_ckon
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16,16 , 0x0 );  // rg_force_da_pxp_rx_oscal_ckon

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 24,24 , 0x1 );        // rg_force_sel_da_pxp_rx_oscal_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 16,16 , 0x0 );        // rg_force_da_pxp_rx_oscal_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1 );          // rg_force_sel_da_pxp_rx_oscal_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x0 );          // rg_force_da_pxp_rx_oscal_en
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_SD_Cal_done ===\n");
#endif

}




void XPON_phy_status(void)
{

    //RX_cal_ready
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x1 );		// rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0 ); 		// rg_disb_rx_os_rdy	
    udelay(1);   


#if LAB_PRINT_PON
   printk("=== XPON_Rhy_Status_done ===\n");
#endif

}



void pon_DIG_reset_hold(void)
{

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x0);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 6,6 , 0x0);       // rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x0);       // rg_sw_ref_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 4,4 , 0x0);       // rg_sw_allpcs_rst_n 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 2,2 , 0x0);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x0);       // rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 3,3 , 0x0);       // rg_sw_pma_rst_n
	
    udelay(50);


#if LAB_PRINT_PON
	  printk("=== XPON_DIG_reset_hold_done ===\n");
#endif
}


void pon_DIG_ref_release(void)
{
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x1);	   // rg_sw_ref_rst_n	
	udelay(100);

	
#if LAB_PRINT_PON
	  printk("=== XPON_DIG_ref_release_done ===\n");
#endif
}



void pon_DIG_reset_release(void)
{
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x1);	   // rg_sw_ref_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 2,2 , 0x1);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x1);       // rg_sw_rx_rst_n

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 3,3 , 0x1);	   // rg_sw_pma_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x1);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 6,6 , 0x1);       // rg_sw_tx_fifo_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 4,4 , 0x1);       // rg_sw_allpcs_rst_n  
    udelay(100);


	#if LAB_PRINT_PON
	  printk("=== XPON_DIG_reset_release_done ===\n");
#endif
}



void XPON_RX_rxrdy(void)
{ 
  //UINT32 read_data;
  
    //RX data path ready
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24,24 , 0x1 );		// rg_force_rx_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24,24 , 0x0 );		// rg_disb_rx_rdy 
    udelay(10);

    //reset RX FIFO
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x0 );                           // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x1 );                           // rg_sw_rx_fifo_rst_n
	udelay(100);

//Force RX Impedance //			

  //********************************************OPTIMIZE RX IMPEDANCE by EFUSE*****************************************************// 
  Pon_rx_term_sel = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XPON_RX_FE_50OHMS_SEL_LSB, 2) : 0xF;
  
  if ((Pon_rx_term_sel > 0) && (Pon_rx_term_sel <= 3)){
	 IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, Pon_rx_term_sel);

	#if LAB_PRINT_PON
	 printk("DA_XPON_RX_TERMP_SEL[1:0]=%x\n", Pon_rx_term_sel);
	#endif
  }
  else {
	 IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, 0x1);

	#if LAB_PRINT_PON
	 printk("Set RXP Impedance Level 1 as default! \n");
	#endif
  }

  //***************************************************************************************************************************//
  

#if LAB_PRINT_PON
   printk("=== XPON_RX_Rxrdy ===\n");
#endif

}


void XPON_DIG_reset(void)
{
	
   //digital reset
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x0 );						   // rg_sw_rx_fifo_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 6,6 , 0x0 );						   // rg_sw_tx_fifo_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x0 );						   // rg_sw_ref_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 4,4 , 0x0 );						   // rg_sw_allpcs_rst_n 
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 2,2 , 0x0 );						   // rg_sw_tx_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x0 );						   // rg_sw_rx_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 3,3 , 0x0 );						   // rg_sw_pma_rst_n
   udelay(10);

   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x1 );						   // rg_sw_ref_rst_n	
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 4,4 , 0x1 );						   // rg_sw_allpcs_rst_n  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 3,3 , 0x1 );						   // rg_sw_pma_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x1 );						   // rg_sw_rx_fifo_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 6,6 , 0x1 );						   // rg_sw_tx_fifo_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 2,2 , 0x1 );						   // rg_sw_tx_rst_n
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x1 );						   // rg_sw_rx_rst_n



#if LAB_PRINT_PON
  printk("=== XPON_DIG_reset_done ===\n");
#endif

}




void XPON_BIST_setting(void)
{ 

    //BIST_tx_rx    	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT, 31,0 , 0x8FF1FD53 );            // bistctl_align_pattern

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_INITIAL_SEED, 31,0 , 0xFF1FD53 );     // bistctl_prbs_init_seed
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 15,0 , 0x1 );         // rg_bistctl_prbs_fail_threshold	

	//IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 4,0 , an7583_pon_pma_param.TX_Pat = PRBS31 );               // rg_bistctl_pat_sel
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 4,0 , 0x5 );                      // rg_bistctl_pat_sel

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_POLLUTION, 16,16 , 0x1 );                  // rg_bist_tx_data_pollution_latch
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 0,0 , 0x0 );                               // anlt_px_lnx_lt_los
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24,24 , 0x0 );                             // rg_lnx_bistctl_bit_error_rst_sel
	

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

		  IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 16,16 , 0x0 );					 // rg_tx_bist_gen_en 
		  IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 8,8 , 0x0 ); 					 // rg_r2t_mode_0: T2R_1: R2T
          
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

	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 8,8 , 0x1 );                      // rg_bistctl_pat_tx_en
	 
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 8,8 , 0x0 ); 					 // rg_r2t_mode_0: T2R_1: R2T
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 16,16 , 0x1 );					 // rg_tx_bist_gen_en 
 
     IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 8,8 , 0x0 );                               // all_lane_prbs_tx_en
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );                    // rg_bistctl_pat_rx_check_en  
     udelay(100);


	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16,16 , 0x0 );                    // rg_bistctl_pat_rx_check_en
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );                    // rg_bistctl_pat_rx_check_en

	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16,16 , 0x0 );                    // rg_bistctl_pat_rx_check_en
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );                    // rg_bistctl_pat_rx_check_en
    
  
#if LAB_PRINT_PON
   printk("=== XPON_Bist_On ===\n");
#endif

}


void XPON_RX_L2D(void)
{

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1 );	// rg_force_sel_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0 );	// rg_force_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );	// rg_force_da_pxp_cdr_lpf_rstb


    //L2D  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x1 );     // rg_force_da_pxp_cdr_lpf_lck2data
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1 );     // rg_force_sel_da_pxp_cdr_lpf_lck2data   
    udelay(200);


#if LAB_PRINT_PON
   printk("=== XPON_RX_L2D_done ===\n");
#endif

}


void XPON_FLL_Reset(void)
{

  IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_3, 0, 0, 0x00); //rg_fll_dig_rst_force_en
  udelay(500);
  IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_3, 0, 0, 0x01); //rg_fll_dig_rst_force_en  

  
  IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_3, 8, 8, 0x00); //rg_fll_dig_rst_force
  udelay(500);
  IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_3, 8, 8, 0x01); //rg_fll_dig_rst_force


  #if LAB_PRINT_PON
   printk("=== XPON_FLL_Reset_done ===\n");
#endif
   
}





void Pon_EO_Scan(uint bit_sel, uint EO_start, uint EO_end , uint Pon_Eye_Scan)
{

    volatile int j = 0 , k = 0;
    volatile unsigned int leq_gain = 1;            
    volatile unsigned int leq_peaking = 0;
    volatile unsigned int sweep_start = EO_start;
    volatile unsigned int sweep_end = EO_end ;            
    volatile unsigned int fom_num = 0, veo = 0;
    volatile unsigned int best_fom = 0, best_veo = 0;   
    volatile unsigned int best_gain = 1;
    volatile unsigned int best_peaking = 0; 




	
    if(Pon_Eye_Scan) printk("bit_sel => %x \n" ,bit_sel);
    

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01); //rg_force_da_pxp_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb   

    XPON_eye_setting(bit_sel);

    //EYE setting, EYE OPEN related  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);  //rg_eq_en_delay
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PI_CAL, 10, 8, 0x04);     //rg_kpgain
    


	if(bit_sel == Async_XEPON || bit_sel == Sync_XEPON || bit_sel == Async_XGPON_1 || bit_sel == Async_XGPON_2 || bit_sel == Sync_XGSPON_1 || bit_sel == Sync_XGSPON_2)	 
    {
        leq_gain = 1;
		best_gain = 1;
    }
	else
	{
		leq_gain = 3;
		best_gain = 3;
	}


	for (k = leq_gain; k < 4; k++)
	{
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);      //rg_force_sel_da_pxp_rx_fe_gain_ctrl
	    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, leq_gain);  //rg_force_da_pxp_rx_fe_gain_ctrl

		leq_peaking = 0;   

       for (j = sweep_start; j < sweep_end + 1; j++)
       {
   
	       IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);      //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	       IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, leq_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl
	       udelay(500);
         
           if(Pon_Eye_Scan) printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);              
	 
           XPON_eye_Cal();
           XPON_eye_EO(bit_sel , &fom_num , &veo , Pon_Eye_Scan);

	      if ((veo > 53)&&(best_veo > 53))
	      {
		      if (fom_num > best_fom)
		      {
			      best_fom = fom_num; 
			      best_veo = veo;
			      best_peaking = leq_peaking;
			      best_gain = leq_gain;		
			      leq_peaking++;		
		      }
		      else if ((fom_num == best_fom) && (veo > best_veo))
		      {
			      best_fom = fom_num; 
			      best_veo = veo;
			      best_peaking = leq_peaking;
			      best_gain = leq_gain;
			      leq_peaking++;		
		      } 
		      else
		      {
			      leq_peaking++;
		      }
	      }
	      else 	   
	      {
		      if  (veo > best_veo)
		      {
			      best_fom = fom_num; 
			      best_veo = veo;
			      best_peaking = leq_peaking;
			      best_gain = leq_gain;		
			      leq_peaking++;		
		      }
		      else 
		      {
			      leq_peaking++;
		      }	   
	      }	   
    }
	   
	leq_gain++;	
  }
				
    //set back the best leq gain&peaking setting
   	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);       //rg_force_sel_da_pxp_rx_fe_gain_ctrl
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, best_gain);  //rg_force_da_pxp_rx_fe_gain_ctrl

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);        //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, best_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl     
    
    if(Pon_Eye_Scan) printk("XPON_best_leq_gain => %x , XPON_best_leq_peaking => %x\n" ,best_gain , best_peaking); 	

}


EXPORT_SYMBOL(Pon_EO_Scan);



void XPON_eye_setting(uint bit_set)
{
    if (bit_set == EO_Async_XGPON_1 || bit_set == EO_Async_XGPON_2 || bit_set == EO_Sync_XGSPON_1 || bit_set == EO_Sync_XGSPON_2)
    {         
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x00);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x240);   // rg_x_max
	   
    }
    else if (bit_set == EO_Sync_GPON || bit_set == EO_Async_GPON || bit_set == EO_Turbo_EPON_Sym|| bit_set == EO_Turbo_EPON)
    {  

	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x02);           // RG_PXP_CDR_LPF_RATIO	   
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x100);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x300);   // rg_x_max
    }
    else if (bit_set == EO_Sync_EPON_1 || bit_set == EO_Sync_EPON_2 )
    {

	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x03);           // RG_PXP_CDR_LPF_RATIO	   
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x0);      // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x400);   // rg_x_max	
		
    }   
	else if (bit_set == EO_Eth_Ser_HSGMII)
    {         
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x00);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x140);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x2C0);   // rg_x_max
	   
    }	
    else if (bit_set == EO_Eth_Ser_5GBaseR)
    { 
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x01);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x180);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x280);   // rg_x_max
    }    
    else  //10p3125G
    {  
	   IO_SPHYA_REG_BITS(AN7583_PON_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x00);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);	  // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);   // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x234);  // rg_x_max	
    }

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xF8);   //rg_cntlen
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 16, 16, 0x0);  //rg_cntforever
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x0);    //rg_data_shift 	 

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x00);   //rg_index_mode
	   //IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0xFFF8);  //rg_eyedur	 
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C);  //rg_eyedur	 

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 0, 0, 0x0);    //rg_eye_nextpts_sel
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 8, 8, 0x0);    //rg_eye_nextpts_toggle
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);  //rg_eye_nextpts	 

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x4);     //rg_eyecnt_hth
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x4);    //rg_eyecnt_vth
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x4);    //rg_eo_hth
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth

	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_1, 16, 16, 0x0);                  //rg_heo_emphasis
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_1, 0, 0, 0x0);                    //rg_a_lgain
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_1, 8, 8, 0x0);                    //rg_a_mgain	   
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_2, 1, 0, 0x1); 	                  //rg_a_sel
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_1, 24, 24, 0x0); 	              //rg_b_zero_sel
	 
	 
}


void XPON_eye_Cal(void)
{

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, 0x0);	//rg_force_da_pxp_cdr_pr_pieye
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x0);   //rg_force_sel_da_pxp_cdr_pr_pieye
    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 22, 16, 0x0);   //rg_force_da_pxp_rx_dac_eye	 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 24, 24, 0x0);   //rg_force_sel_da_pxp_rx_dac_eye	
	
	 
    //pical redo
    //reset block
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);   //rg_eq_en_delay
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_PI_CAL, 10, 8, 0x01);      //rg_kpgain
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 16, 16, 0x00);    //rg_eq_pi_cal_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_6, 8, 8, 0x00);  //rg_disb_rx_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 8, 8, 0x00); //rg_force_rx_and_pical_rstb

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_6, 0, 0, 0x00);  //rg_disb_ref_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 0, 0, 0x00); //rg_force_ref_and_pical_rstb

		
    //enable	
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_5, 24, 24, 0x00);  //rg_disb_rx_or_pical_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);   //rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8, 8, 0x00);   //rg_disb_rx_pical_en
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x00);  //rg_force_rx_pical_en
   
		
    //release reset
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_0, 16, 16, 0x01);     //rg_eq_pi_cal_rst_b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 8, 8, 0x01);  //rg_force_rx_and_pical_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_7, 0, 0, 0x01);  //rg_force_ref_and_pical_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8, 8, 0x01);  //rg_force_rx_or_pical_en     
    udelay(1000); //delay for 1ms 

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);   //rg_force_rx_or_pical_en      
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_3, 0, 0, 0x00);	 //rg_disb_eq_pi_cal_rdy
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_3, 0, 0, 0x01);	 //rg_force_eq_pi_cal_rdy   

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 0, 0, 0x1);   //rg_force_eyecnt_rdy 
	
	udelay(1000); //delay for 1ms 
}



 void XPON_eye_EO(uint bit_rate, volatile int *fom_num , volatile int *veo , uint Pon_Eye_Scan)
{
   //volatile unsigned int fom_num;
   volatile int R_fom_num , R_veo;


    //IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0x0A);     //rg_cntlen  
    //IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C); //rg_eyedur     

    //reset eye_top   
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 8, 8, 0x00);     //rg_disb_eye_reset_plu_o 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);    //rg_force_eye_reset_plu_o  
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 8, 8, 0x00);    //rg_force_eye_reset_plu_o  

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 16, 16, 0x00);   //rg_disb_eye_top_en 
	 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);  //rg_force_eye_top_en 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 16, 16, 0x01);  //rg_force_eye_top_en    

     
   if (bit_rate == EO_Async_XEPON || bit_rate == EO_Sync_XEPON || bit_rate == EO_Async_XGPON_1 || bit_rate == EO_Async_XGPON_2 || bit_rate == EO_Sync_XGSPON_1 || bit_rate == EO_Sync_XGSPON_2)
   {
       udelay(5500); //delay for 5.5ms     
   }
   else
   {	   
	   mdelay(55); //delay for 55ms  
   }

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 8, 8, 0x01);     //rg_disb_da_xpon_rx_dac_eye  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_2, 24, 24, 0x01);   //rg_disb_da_xpon_cdr_pr_pieye  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01);   //rg_disb_eyedur_init_b  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 24, 24, 0x01);           //rg_disb_eyecnt_rx_rst_b  
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01);   //rg_disb_eyedur_en
   

   //read out EO results   
   XPON_readout_EO(0 , &R_fom_num , &R_veo ,Pon_Eye_Scan);

   //clear EXE_X_SW offset value
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);     //rg_force_eye_reset_plu_o  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_8, 16, 16, 0x00);    //rg_disb_eye_top_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);   //rg_disb_eye_top_en  
   

   //reset pi_cal_rdy
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_3, 0, 0, 0x00);     //rg_disb_eq_pi_cal_rdy  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_3, 0, 0, 0x00);    //rg_force_eq_pi_cal_rdy  
   
   //reset eyecnt_rdy
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy 
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_6, 0, 0, 0x0);   //rg_force_eyecnt_rdy      

   
   *fom_num = R_fom_num;
   *veo = R_veo;
}



void XPON_readout_EO(int sweep_direction , volatile int *R_fom_num , volatile int *R_veo , uint Pon_Eye_Scan)
{
   volatile unsigned int eye_er, eye_el, eye_eu, eye_eb, i = 0;
   volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos;
   volatile	unsigned int eye_x_done, eye_y_done, heo_rdy, veo_rdy;   
   volatile int fom_num,heo,veo,heo_tmp , veo_tmp , eye_er_tmp , eye_el_tmp, eye_eu_tmp , eye_eb_tmp;
   //volatile int heo, heo_tmp , veo_tmp , eye_er_tmp , eye_el_tmp, eye_eu_tmp , eye_eb_tmp;
   //int fom_num,veo;

   
   volatile unsigned int pical_data_out;   
   volatile unsigned int EO_Buf[10]= {0};
   


   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 0, 0, 0x01);	  //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 8, 8, 0x01);	  //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 16, 16, 0x01);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 24, 24, 0x01);  //ro_lnx_sw_fll_ro_4_latch_en  
   
   udelay(50);

   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 0, 0, 0x00);    //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 8, 8, 0x00);    //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 16, 16, 0x00);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FLL_6, 24, 24, 0x00);  //ro_lnx_sw_fll_ro_4_latch_en  

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
	udelay(100);
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle  
	

	pical_data_out = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_2, 22, 16); //ro_pi_cal_data_out
	eye_x_done = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_9, 16, 16);     //eo_x_done
	eye_y_done = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_9, 24, 24);     //eo_y_done
	heo_rdy = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_5, 16, 16);        //heo_rdy
	veo_rdy = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_5, 24, 24);        //veo_rdy
	
   
  if ((eye_x_done == 1) && (eye_y_done == 1))
  {	  

	   if (Pon_Eye_Scan) printk("\n" );	    
	   if (Pon_Eye_Scan) printk("X&Y index scan done\n");

	if ((heo_rdy == 1) && (veo_rdy == 1))
	  {	  

	    if (Pon_Eye_Scan) printk("\n" );			 
		if (Pon_Eye_Scan) printk("heo&veo result ready\n");
	   }
	 }
	else
	   {		   

		 if (Pon_Eye_Scan) printk("X&Y index scan NOT done\n");
	   }    

	 eye_el = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_10, 26, 16); //eye_el	 
	 eye_er = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_10, 10, 0);  //eye_er
	 eye_eu = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_11, 6, 0);   //eye_eu	 
	 eye_eb = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_11, 14, 8);	//eye_eb
	 

	 //ABS_Funt.
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

	 dac_d0 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_1, 14, 8);  //ro_rx_dac_d0
	 dac_d1 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_1, 22, 16); //ro_rx_dac_d1
	 dac_e0 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_1, 30, 24); //ro_rx_dac_e0
	 dac_e1 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_2, 6, 0);   //ro_rx_dac_e1
	 dac_eye = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_1, 6, 0);  //ro_rx_dac_eye	 
	 feos    = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_2, 13, 8);  //ro_rx_feos_out
	   
	 fom_num = heo;   
    
	 //print out these results to textbox
   if (sweep_direction == 0)
      {	
		 if (Pon_Eye_Scan) printk("Y index sweep direction: bottom-up\n");
	  }
   else
	  {
	     if (Pon_Eye_Scan) printk("Y index sweep direction: upside-down\n");
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


	eye_eu_tmp = (int) EO_Buf[0];	//uint to int 
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


  

 if (Pon_Eye_Scan){		   
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
 	}	   

//check whether CLK unlocking happens
  if (heo_rdy != 1)
	{		   

       if (Pon_Eye_Scan) printk("EO result isn't valid.\n");
   
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_0, 24, 24, 0x00);    //rg_disb_da_xpon_cdr_lpf_rstb  
	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 24, 24, 0x00);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	 udelay(500); //delay for 500us 	  

	 IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_0, 24, 24, 0x01);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	  udelay(500); //delay for 500us 
	 }   

   *R_fom_num = fom_num;
   *R_veo = veo;
   
}	   






void XPON_TDC_on(void)
{

    //TDC    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_6, 9,8 , 0x0 );                      // rg_lcpll_ncpo_chg_delay
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_0, 1,0 , 0x1 );                      // rg_lcpll_pon_rx_cdr_divtdc

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8,8 , 0x0 );	// rg_force_sel_da_pxp_txpll_sdm_pcw_chg
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24,24 , 0x0 );		// rg_force_sel_da_pxp_txpll_sdm_pcw	


	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PW_5, 16,16 , 0x1 ); 					// rg_lcpll_autok_tdc
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_3, 8,8 , 0x0 );						// rg_lcpll_ncpo_load	
    udelay(100);

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_5, 16,16 , 0x0 );                    // rg_lcpll_tdc_autopw_ncpo
    udelay(100);

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 0,0 , 0x1 );						// rg_lcpll_gpon_sel 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_PW_0, 0,0 , 0x1 );						// rg_lcpll_tdc_dig_pwdb

	//mdelay(5);	// add delay for reset flow stability	
    udelay(500); //delay for 500us 

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1 );   // rg_force_sel_da_pxp_cdr_lpf_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0 );   // rg_force_da_pxp_cdr_lpf_rstb
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );   // rg_force_da_pxp_cdr_lpf_rstb	



#if LAB_PRINT_PON
   printk("=== XPON_TDC_On_done ===\n");
#endif
  
}




void XPON_R2T_on(void)
{ 

    //R2T/
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 16, 16, 0x00);                     //rg_tx_bist_gen_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 8, 8, 0x00);                       //rg_bistctl_pat_tx_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16, 16, 0x01);                     // rg_bistctl_pat_rx_check_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 8, 8, 0x00);                                //all_lane_prbs_tx_en

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 8, 8, 0x00);                       //rg_r2t_mode
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 8, 8, 0x01);                       //rg_r2t_mode

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 24, 24, 0x00);                     //rg_r2t_fifo_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 24, 24, 0x01);                     //rg_r2t_fifo_en 
  

#if LAB_PRINT_PON
   printk("=== XPON_R2T_On ===\n");
#endif

}


void Pon_Version (void)
{
    printk("=== XPON_Physical_Init._Ver1.1 ===\n" );
}


void phy_eyescan_test(uint bit_sel, int start_p, int sweep_r)
{
	uint pical_data_out=0;
    uint ro_dac_eye , eye_offset=0;
    int EYE_X_FW, EYE_Y_FW=0;
    uint EYE_X_HW, EYE_Y_HW=0;            
    uint eyecnt = 0;
    int i,j,k=0;
           
    //ro_dac_eye = Convert.ToUInt32(Yoffset.Text, 16);
    //eye_offset = Convert.ToUInt32(Yoffset.Text, 16);
    int Start_Point = start_p;
    int Sweep_Range = sweep_r;
	//int Ovr = Ovr_sel; 	//need to modify for different RX Rate
	int Ovr = 1;

	printk("fe_gain => %x\n",IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0)); 
	printk("fe_peaking => %x\n",IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17));	
    printk("Start_Point = %d	Sweep_Range = %d	Ovr = %d \n", Start_Point, Sweep_Range, Ovr);
	
	
	//	 uint step = 5;
	printk("XPON_EyeScan");

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01); //rg_force_da_pxp_cdr_pr_pieye_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb

		
		
	XPON_eye_setting(bit_sel);

	//reset_eye_control	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x00);   //rg_disb_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x00); //rg_force_eyedur_init_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 24, 24, 0x00);  //rg_disb_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 24, 24, 0x00); //rg_force_eyecnt_rx_rst_b

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01); //rg_disb_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x01); //rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01); //rg_disb_eyedur_en		
			
	XPON_eye_Cal();
						   
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
	udelay(100);
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle        
	    
    ro_dac_eye = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_RO_RX2ANA_1, 6, 0);  //ro_rx_dac_eye
	pical_data_out = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_2, 22, 16); //ro_pi_cal_data_out
	
	

	ro_dac_eye = eye_offset + ro_dac_eye;

	
	//EYE_X_HW = 0;
	EYE_X_HW = pical_data_out;
    //EYE_Y_HW = eye_offset + ro_dac_eye;
	EYE_X_FW = EYE_X_HW;

	EYE_Y_HW = 64;	
	//EYE_Y_HW = 64-pical_data_out;	
	//EYE_Y_HW = 64 + ro_dac_eye;	
	EYE_Y_FW = -64; 

	//printk("Yoffset = %d \n", ro_dac_eye);
	printk("pical_data_out = %d \n", pical_data_out);		 
				   	
	for (i = 0; i < Start_Point*Ovr; i++){
	
		// X index
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_HW); //rg_force_da_pxp_cdr_pr_pieye
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);      //rg_force_sel_da_pxp_cdr_pr_pieye			
		
		EYE_X_HW--;
		EYE_X_FW--;			
	}	

	printk("eyecnt		EYE_X_FW		EYE_Y_FW \n");
	//snack sequence sweep full eye scan																							  
	for (k = 0; k < (Sweep_Range/2) ; k++)																							  
	{																																  
		for (i = 0; i < 65; i++)																									  
		{																															  		
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);
				 	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++;
		}	
		
		EYE_Y_HW = 1;																												  
		EYE_Y_FW = 1;

		ssleep(1);
																																		  
		for (j = 0; j < 63; j++)																									  
		{																															  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++; 
		}			

		ssleep(1); 
		
			EYE_X_HW = eyescan_moveX(EYE_X_FW, EYE_Y_FW, bit_sel);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW--; 																												  
			EYE_Y_FW--; 																												  
																																		  
			for (j = 0; j < 63; j++)																									  
			{																															  
				eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 

			}	
			
			EYE_Y_HW = 128; 																											  
			EYE_Y_FW = 0;

			ssleep(1);
																																		  
			for (i = 0; i < 65; i++)																									  
			{																															  
				eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 	

			}																															  
			EYE_X_HW = eyescan_moveX(EYE_X_FW, EYE_Y_FW,bit_sel);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 																												  
		}																																  
																																		  
		//last time bottom-up sweep Y index 																							  
		for (i = 0; i < 65; i++)																										  
		{																																  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 
		}
		
		EYE_Y_HW = 1;																													  
		EYE_Y_FW = 1;	
		
		for (j = 0; j < 63; j++)																										  
		{																																  
			eyecnt =eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 
		}

}

EXPORT_SYMBOL(phy_eyescan_test);



int eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, uint bit_sel ){
	
	int eyecnt = 0;																			
	int eyecnt_rdy = 0;																		
																								
	
    //Set the eyecnt count point (EYE_X_FW, EYE_Y_FW)
    // X index         
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_FW); //rg_force_da_pxp_cdr_pr_pieye
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);	   //rg_force_sel_da_pxp_cdr_pr_pieye		 
         
    // Y index
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 22, 16, EYE_Y_FW); //rg_force_da_pxp_rx_dac_eye
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 24, 24, 0x1);	   //rg_force_sel_da_pxp_rx_dac_eye
		 

    // EYE cnt enable 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x0);	   //rg_disb_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x0);	//rg_force_eyedur_init_b		

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DISB_MODE_7, 24, 24, 0x0);	//rg_disb_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 24, 24, 0x0); //rg_force_eyecnt_rx_rst_b 

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x0);	   //rg_disb_eyedur_en
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x0);	   //rg_force_eyedur_en
		 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_FORCE_MODE_8, 24, 24, 0x1); //rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x1);	 //rg_force_eyedur_init_b

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x1);	 //rg_force_eyedur_en
        
    if (bit_sel == EO_Async_XGPON_1 || bit_sel == EO_Sync_XGSPON_2 || bit_sel == EO_Sync_XGSPON_1 || bit_sel == EO_Sync_XGSPON_2|| bit_sel == EO_Async_XEPON|| bit_sel == EO_Sync_XEPON)
    {
	   mdelay(1);

	}
	else
	{
	   mdelay(30);
    }		 

		 
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
	udelay(100);	    
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle

      
    eyecnt_rdy = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_4, 24, 24);  //eyecnt_rdy

	//for_long_delay_Test_230613_Morris_Add
    //eyecnt = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_7, 19, 0);  //eyecnt

		 
    if (eyecnt_rdy == 1)   // if eyecnt_rdy
    {             
        eyecnt = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_7, 19, 0);  //eyecnt
    }
    else
    {
        printk("eyecnt_rdy = %d \n", eyecnt_rdy);
    }      
    
   
     return eyecnt;																	
}



uint eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW,uint bit_sel){
	int i ;
	uint Ovr_sel = 1;	
	            
       for (i = 0; i < Ovr_sel; i++)
       {
           // X index  
           IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_HW); //rg_force_da_pxp_cdr_pr_pieye
		   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);      //rg_force_sel_da_pxp_cdr_pr_pieye
           EYE_X_HW++;
       }   
   
    return EYE_X_HW;
}



void pon_Ext_T2R (int pon_Spd)
{  

   pon_CfgPhyType(pon_Spd);   
   
   XPON_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	     
   XPON_TX_on();
   
   XPON_RX_preset(InitSpd_Pon_Preset); 
   XPON_TDC_off(); 
   XPON_RX_on(); 
   XPON_RX_L2R(); 
   pon_DIG_ref_release();

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   XPON_RX_feos();
   XPON_RX_sdcal();  
   
   XPON_phy_status();   
   pon_DIG_reset_release();
   XPON_RX_L2D();   
   Pon_EO_Scan(pon_Spd,0,7,0); //20221011  
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(bist_data);     
   
}


EXPORT_SYMBOL(pon_Ext_T2R);




void Bert_pon_Ext_T2R (int pon_Spd)
{  

   pon_CfgPhyType(pon_Spd);   
   
   XPON_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	     
   XPON_TX_on();
   
   XPON_RX_preset(InitSpd_Pon_Preset); 
   XPON_TDC_off(); 
   XPON_RX_on(); 
   XPON_RX_L2R(); 
   pon_DIG_ref_release();

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   XPON_RX_feos();
   XPON_RX_sdcal();  
   
   XPON_phy_status();   
   pon_DIG_reset_release();
   XPON_RX_L2D();   
   Pon_EO_Scan(pon_Spd,0,7,0); //20221011
   XPON_TDC_on();     
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(bist_data);     
   
}

EXPORT_SYMBOL(Bert_pon_Ext_T2R);



void pon_Ext_R2T (int pon_Spd)
{

   pon_CfgPhyType(pon_Spd);
      
   XPON_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd);    
   XPON_TX_on();
   
   XPON_RX_preset(InitSpd_Pon_Preset); 
   XPON_TDC_off(); 
   XPON_RX_on(); 
   XPON_RX_L2R(); 
   pon_DIG_ref_release();

   XPON_RX_OSCal(); 
   XPON_RX_pical();
   XPON_RX_pdos(); 
   XPON_RX_feos();
   XPON_RX_sdcal();   
   
   XPON_phy_status();   
   pon_DIG_reset_release();
   XPON_RX_L2D();   
   Pon_EO_Scan(pon_Spd,0,7,0); //20221011
   XPON_TDC_on();     
   XPON_RX_rxrdy();
   XPON_BIST_setting();
   XPON_R2T_sel(r2t_data);  
   
}

EXPORT_SYMBOL(pon_Ext_R2T);



//================= ASIC_PON_SERDES_API =================

void normal_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern)
{
	uint prbs_result = 0 , wan_sel = 0 ;	
	int read_data=0;
	int i=0;

	wan_sel = GET_WAN_CONF();

	
	if(BIST_check_switch == PHY_ENABLE)
	{
      
		xpon_bist_check_pattern_sel(align_pattern);

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,	24, 24, 0x1);  //bistctl_io_data_invert rx
		read_data=IO_GPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,	25, 24);
		printk("0x%8x[25:24](bistctl_io_data_inv)=0x%x\n",AN7583_PON_PMA_BISTCTL_CONTROL,read_data);
		
	 	// Create_Morris_(PON_CSR_PMA_BASE + 0x3684 )//
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 31, 0, 0x00000000); 	// BFAF3684 default = 0x01000000
		// IO_SPHYA_REG_BITS(SS_BIST_1, 0, 0, 0x00));  	        		//3684 , anlt_px_lnx_lt_los
		// IO_SPHYA_REG_BITS(SS_BIST_1, 8, 8, 0x01));  	        		//3684 , all_lane_prbs_tx_en
		// IO_SPHYA_REG_BITS(SS_BIST_1, 24, 24, 0x00));  	    		//3684 , rg_lnx_bistctl_bit_error_rst_sel
		// customer need both tx and rx work, marked by ang_20180713
		//IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 	8,	8,	0x0);	//BIST TX EN		check RX BIST only, no need TX sending PRBS	

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  16, 16, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16, 16, 0x01);	//3500, rg_bistctl_pat_rx_check_en	 do rx BIST check
													
		for(i=0; i<16; i++){		
			prbs_result = IO_GPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_EVENT,	16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
			printk("0x%8x = %8x\r\n",AN7583_PON_PMA_BISTCTL_PRBS_EVENT, prbs_result);
			if(prbs_result == 0x10001)	   //david 20170428
			{
				printk("Pattern Aligned!\n");		
					break;
			}
			else if(prbs_result == 0x10101)
			{
				printk("Pattern Aligned with BER error!\n");
					break;
			}
			else
			{
				printk("PRBS check Not Align yet!\n");
			}
			if(i==8)
			{
				IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  24, 24, 0x0);   //bistctl_io_data_invert rx
				read_data=IO_GPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,	25, 24);
				printk("0x%8x[25:24](bistctl_io_data_inv)=0x%x\n",AN7583_PON_PMA_BISTCTL_CONTROL,read_data);
			}
			mdelay(500);
		}		
		if((prbs_result == 0x0)||(prbs_result == 0x100))
		{
			printk("PRBS check Failed! please check RX rates & selected pattern!\n");
		}
        
	}
	else if(BIST_check_switch == PHY_DISABLE)
	{		
		fiber_plug_reset(PLUG_OUT,wan_sel);	//tdc_off & RX L2R		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  16, 0, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 31, 0, 0x01000000); 	// BFAF3684 default = 0x01000000
		printk("RX BIST Check End!\n");	
	}
}	

EXPORT_SYMBOL(normal_rx_bist_check);



int xpon_bist_check_pattern_sel(XPON_PMA_RX_Bist_Check_t align_pattern)
{
	printk("[%s:%d] pattern %d\n",__FUNCTION__,__LINE__,align_pattern);

	switch(align_pattern)
	{
		case PRBS7_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,		 4,	 0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT,	 31,	0,	0x6081DF53);//rg_px_lnx_bist_ctl_align_pattern	
			break;

		case PRBS9_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0xD97B0D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS15_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0xE1FD7D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS23_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0x5CF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS31_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT,		31,	0,	0x8FF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS11_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,			4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_ALIGN_PAT, 	   31, 0,	0xA1903D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		default:
			break;
				
	}

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_INITIAL_SEED, 	31, 0,	0x0FF1FD53);//bistctl_prbs_init_seed	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,  15, 0,	0x01);//rg_bistctl_prbs_fail_threshold

	return 0;
}

EXPORT_SYMBOL(xpon_bist_check_pattern_sel);



void pon_phy_rogueonu_int_en(unchar rogue_int_en)
{	
	uint read_data=0, write_data=0;


	read_data = IO_GPHYREG(AN7583_PON_PMA_XPON_INT_EN_0);
	//PON_PHY_PRINT(PHY_MSG_TRACE,"AN7583_XPON_PMA_XPON_INT_EN_0 read     :0x%.8x\n", read_data);
	printk("AN7583_XPON_PMA_XPON_INT_EN_0 read     :0x%.8x\n", read_data);

	if(PHY_ENABLE == rogue_int_en)
	{
		write_data = read_data | AN7583_XPON_PMA_TRANS_ROGUE_ONU_INT_EN;
		IO_SPHYREG(AN7583_PON_PMA_XPON_INT_EN_0, write_data);
		read_data = IO_GPHYREG(AN7583_PON_PMA_XPON_INT_EN_0);
		//PON_PHY_PRINT(PHY_MSG_TRACE,"XGPON AN7583_XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);	
		printk("XGPON AN7583_XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);
	}
	else
	{
		write_data = read_data & (~(AN7583_XPON_PMA_TRANS_ROGUE_ONU_INT_EN));
		IO_SPHYREG(AN7583_PON_PMA_XPON_INT_EN_0, write_data);
		read_data = IO_GPHYREG(AN7583_PON_PMA_XPON_INT_EN_0);
		//PON_PHY_PRINT(PHY_MSG_TRACE,"AN7583_XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);	
		printk("AN7583_XPON_PMA_XPON_INT_EN_0 write    :0x%.8x\n", read_data);
	}
}

EXPORT_SYMBOL(pon_phy_rogueonu_int_en);


void pon_phy_clear_rogueonu(void)
{
	uint read_data=0;
	
	//clear rogue onu cnt
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_SD_BEHAVIOR_STA);
	printk("AN7583_XPON_PMA_XPON_SD_BEHAVIOR_STA= %x\n", read_data);
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 1 );
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 0 );
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_SD_BEHAVIOR_STA);
	printk("AN7583_PON_PMA_XPON_SD_BEHAVIOR_STA= %x\n", read_data);

	//clear rogue onu int
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);
	printk("AN7583_XPON_PMA_XPON_INT_STA_0= %x\n", read_data);
	IO_SPHYREG(AN7583_PON_PMA_XPON_INT_STA_0, read_data);
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);
	printk("AN7583_XPON_PMA_XPON_INT_STA_0= %x\n", read_data);
}

EXPORT_SYMBOL(pon_phy_clear_rogueonu);


u32 pma_fifo_check (uint ncpo_check_cnt)	//add by ang_20190130
{
	uint read_data=0;
	//uint write_data=0;
	uint i=0;
	uint *ncpo_arr;
	uint ncpo_max=0;
	uint ncpo_min=0;

	if((ncpo_check_cnt>0)&&(ncpo_check_cnt<=50000))
	{
		ncpo_arr = (uint*)vmalloc(sizeof(uint) * ncpo_check_cnt);
	}
	else
	{
		printk("Alloc data memory size %d error, recommand(1~50000)\n",ncpo_check_cnt) ;
		return 0;
	}
		
	
	if(!ncpo_arr) 
	{
		printk("Alloc data memory failed\n") ;
		return 0;
	}

	memset(ncpo_arr, 0, (sizeof(*ncpo_arr)*ncpo_check_cnt)) ;

	read_data = IO_GPHYREG(AN7583_PON_PMA_RX_DEBUG_0);

	IO_SPHYREG(AN7583_PON_PMA_RX_DEBUG_0,reversebit(read_data,24));
	IO_SPHYREG(AN7583_PON_PMA_RX_DEBUG_0,reversebit(read_data,24));

	printk("	0x%08x=0x%08x\r\n",AN7583_PON_PMA_ADD_RO_RX2ANA_3,IO_GPHYREG(AN7583_PON_PMA_ADD_RO_RX2ANA_3));
	printk("	0x%08x=0x%08x\r\n",AN7583_PON_PMA_ADD_RO_TX2ANA_1,IO_GPHYREG(AN7583_PON_PMA_ADD_RO_TX2ANA_1));
	printk("	0x%08x=0x%08x\r\n",AN7583_PON_PMA_FIFO_CK_STATUS,IO_GPHYREG(AN7583_PON_PMA_FIFO_CK_STATUS));

	for(i=0;i<ncpo_check_cnt;i++)
	{
		ncpo_arr[i]=IO_GPHYREG(AN7583_PON_PMA_LCPLL_TDC_RO_4);
	}

	ncpo_max=ncpo_arr[0];
	ncpo_min=ncpo_arr[0];
	
	for(i=1;i<ncpo_check_cnt;i++)
	{
		if(ncpo_max<ncpo_arr[i])
		{
			ncpo_max=ncpo_arr[i];
		}

		if(ncpo_min>ncpo_arr[i])
		{
			ncpo_min=ncpo_arr[i];
		}
	}
	
	printk("	ncpo_max=0x%08x\r\n",ncpo_max);
	printk("	ncpo_min=0x%08x\r\n",ncpo_min);
	printk("	ncpo_diff=0x%08x (ncpo_max-ncpo_min)\r\n",(ncpo_max-ncpo_min));

	vfree(ncpo_arr);
	return 0;

}

EXPORT_SYMBOL(pma_fifo_check);


void NCPO_Force(int enable) //julia_20221003
{
	
	if(enable == ENABLE)
	{		
		printk("NCPO FORCE : %x\r\n",IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw)); 

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x1);
		mdelay(1);
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x1);
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x1);
		mdelay(1);
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x0);
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x0);
		
		mdelay(1);
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x1);
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x1);
	}
	else if(enable == DISABLE)
	{
		
		printk("NCPO FORCE DISABLE\r\n"); 
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x0);
		mdelay(1);
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 0, 0, 0x0);
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_sdm_pcw_chg, 8, 8, 0x0);		
	}
}

EXPORT_SYMBOL(NCPO_Force);


void tdc_bw(uint bandwidth_level)
{
//	uint lcpll_a_tdc=0;
	switch(bandwidth_level)
	{
		case 3:
			lcpll_a_tdc = 0x03;
			printk("========== SET TDC BW: 80kHz ==========\n");
			break;

		case 4:
			lcpll_a_tdc = 0x04;
			printk("========== SET TDC BW: 38kHz ==========\n");
			break;

		case 5:
			lcpll_a_tdc = 0x05;
			printk("========== SET TDC BW: 22kHz ==========\n");
			break;

		case 6:
			lcpll_a_tdc = 0x06;
			printk("========== SET TDC BW: 10kHz ==========\n");
			break;	
			
		case 0xc:
			lcpll_a_tdc = 0x0c;
			printk("========== SET TDC BW: 114kHz ==========\n");
			break;
		
		case 0xd:
			lcpll_a_tdc = 0x0d;
			printk("========== SET TDC BW: 109kHz ==========\n");
			break;

		case 0xe:
			lcpll_a_tdc = 0x0e;
			printk("========== SET TDC BW: 58kHz ==========\n");
			break;

		case 0xf:
			lcpll_a_tdc = 0x0f;
			printk("========== SET TDC BW: 32kHz ==========\n");
			break;

		default:
			printk("=== extra BW level: please choose integer between 3~6 and 0xe~0xf ===\n");
			break;
	}		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 11, 8, lcpll_a_tdc);	

	printk("current tdc_bw = 0x%x\n",IO_GPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_1, 11, 8));

}

EXPORT_SYMBOL(tdc_bw);



//****************************************************************************************************
//RX Bist Align check
//****************************************************************************************************
void xpon_rx_bist_recheck_result(uint check_onoff, uint duration)
{
	//int i;
	int prbs_check_duration=0;
	
	if(duration<100)
	{
		prbs_check_duration = duration * 1000;
		printk("prbs_check_duration = %d s\n", duration);
	}
	else
	{
		prbs_check_duration = duration;
		printk("prbs_check_duration = %d ms\n", prbs_check_duration);
	}
	
	if(check_onoff == PHY_ENABLE)
	{
		en7581_bist_check_err_cnt = 0;

		//clear bit_error
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x0);

		//phy_print_time();
		printk("original PRBS Error Counter = %u \n", en7581_bist_check_err_cnt);
		msleep(prbs_check_duration);
		//en7581_bist_check_err_cnt = IO_GPHYREG(EN7581_GEPON_CSR_PRBS_ERR_CNT);//ro_bistctl_prbs_err_cnt //0x1faf02c0
		en7581_bist_check_err_cnt = IO_GPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RO_RESERVE_1,	31, 0);

		//phy_print_time();
		printk("PRBS Error Counter = %u \n",en7581_bist_check_err_cnt); //unsigned output
		
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x0);
	}
	else if(check_onoff == PHY_DISABLE)
	{
		en7581_bist_check_err_cnt = 0;
		//reset rx_bist_check error counter in case it overflow
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x1);//bit_error_rst_sel
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 24, 24, 0x0);
	}
}

EXPORT_SYMBOL(xpon_rx_bist_recheck_result);



void t2r_rx_bist_check(uint BIST_check_switch, XPON_PMA_RX_Bist_Check_t align_pattern)
{
	uint prbs_result = 0;
	//uint prbs_err_cnt = 0;
	int i;

	if(BIST_check_switch == PHY_ENABLE)
	{
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 16, 16, 0x01);//rg_tx_bist_gen_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  8,	8,	0x00);//rg_bistctl_pat_tx_en	
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1,  2,	0,	0x00);//rg_xpon_mode
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_T2R_MODE_1,	 1,	1,	0x01);//t2r_bist_mode					   
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_T2R_MODE_1,	 0,	0,	0x00);//t2r_FIFO_en 		

		xpon_bist_check_pattern_sel(align_pattern);

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1,		8,	8,	0x01);//all_lane_prbs_tx_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1,		0,	0,	0x00);//anlt_px_lnx_lt_los
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 24, 24, 0x0);//rg_bistctl_io_data_inv

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  8, 8, 0x1); //TX_BIST_EN		

		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  16, 16, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 16, 16, 0x01); 	//3500, rg_bistctl_pat_rx_check_en	 do rx BIST check

		for(i=0; i<16; i++){		
		prbs_result = IO_GPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_PRBS_EVENT,	16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
		printk("0x%8x = %8x\r\n",AN7583_PON_PMA_BISTCTL_PRBS_EVENT, prbs_result);
		if(prbs_result == 0x10001)	   //david 20170428
		{
			printk("Pattern Aligned!\n");		
				break;
		}
		else if(prbs_result == 0x10101)
		{
			printk("Pattern Aligned with BER error!\n");
				break;
		}
		else
		{
				printk("PRBS check Not Align yet!\n");
			}
			mdelay(500);
		}		
		if((prbs_result == 0x0)||(prbs_result == 0x100))
		{
			printk("PRBS check Failed! please check RX rates & selected pattern!\n");
		}
	}
	else if(BIST_check_switch == PHY_DISABLE)
	{
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL,  16, 0, 0x0);//rg_bistctl_pat_rx_check_en
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1, 31, 0, 0x01000000); 	// BFAF3684 default = 0x01000000
		printk("RX BIST Check End!\n");	
	}
}	


EXPORT_SYMBOL(t2r_rx_bist_check);



void AN7583_Clear_PMA_Rogue(void)
{
	uint read_data=0;

	//clear rogue onu int
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);
	printk("AN7583_XPON_PMA_XPON_INT_STA_0= %x\n", read_data);
	IO_SPHYREG(AN7583_PON_PMA_XPON_INT_STA_0, read_data);
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);
	printk("AN7583_XPON_PMA_XPON_INT_STA_0= %x\n", read_data);
}

EXPORT_SYMBOL(AN7583_Clear_PMA_Rogue);



u32 AN7583_Get_PMA_XPON_Setting0(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_SETTING_0);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_XPON_Setting0);

void AN7583_Set_PMA_XPON_Setting0(u32 wdata)
{	
	IO_SPHYREG(AN7583_PON_PMA_XPON_SETTING_0 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_XPON_Setting0);

u32 AN7583_Get_PMA_XPON_Setting1(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_SETTING_1);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_XPON_Setting1);

void AN7583_Set_PMA_XPON_Setting1(u32 wdata)
{	
	IO_SPHYREG(AN7583_PON_PMA_XPON_SETTING_1 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_XPON_Setting1);


u32 AN7583_Get_PMA_MD32_Clk_Ctrl(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_MD32_MEM_CLK_CTRL);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_MD32_Clk_Ctrl);


void AN7583_Set_PMA_MD32_Clk_Ctrl(u32 wdata)
{	
	IO_SPHYREG(AN7583_PON_PMA_MD32_MEM_CLK_CTRL , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_MD32_Clk_Ctrl);



u32 AN7583_Get_PMA_TX_Dly_Ctrl(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_TX_DLY_CTRL);

	
	return read_data;
}


EXPORT_SYMBOL(AN7583_Get_PMA_TX_Dly_Ctrl);


void AN7583_Set_PMA_TX_Dly_Ctrl(u32 wdata)
{	
	IO_SPHYREG(AN7583_PON_PMA_TX_DLY_CTRL , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_TX_Dly_Ctrl);




u32 AN7583_Get_Trans_Brust_Forve(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_BEN_CTRL_0);

	
	return read_data;
}


EXPORT_SYMBOL(AN7583_Get_Trans_Brust_Forve);


void AN7583_Set_Trans_Brust_Forve(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_BEN_CTRL_0 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_Trans_Brust_Forve);




u32 AN7583_Get_PMA_BistCTL_Forve(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_BISTCTL_CONTROL);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_BistCTL_Forve);



void AN7583_Set_PMA_BistCTL_Forve(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_BISTCTL_CONTROL , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_BistCTL_Forve);




u32 AN7583_Get_PMA_XPON_Mode_1(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_ADD_XPON_MODE_1);

	
	return read_data;
}


EXPORT_SYMBOL(AN7583_Get_PMA_XPON_Mode_1);


void AN7583_Set_PMA_XPON_Mode_1(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_ADD_XPON_MODE_1 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_XPON_Mode_1);



u32 AN7583_Get_PMA_XPON_TX_Force_1(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_DA_XPON_TX_FORCE_1);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_XPON_TX_Force_1);


void AN7583_Set_PMA_XPON_TX_Force_1(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_DA_XPON_TX_FORCE_1 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_XPON_TX_Force_1);




u32 AN7583_Get_PMA_XPON_TX_Force_2(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_DA_XPON_TX_FORCE_2);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PMA_XPON_TX_Force_2);


void AN7583_Set_PMA_XPON_TX_Force_2(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_DA_XPON_TX_FORCE_2 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PMA_XPON_TX_Force_2);



u32 AN7583_Get_Trans_Brust_INV(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_SETTING_0);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_Trans_Brust_INV);



void AN7583_Set_Trans_Brust_INV(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_XPON_SETTING_0 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_Trans_Brust_INV);

u32 AN7583_Get_PON_PMA_XPON_INT_EN_0(void)
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_EN_0);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PON_PMA_XPON_INT_EN_0);



void AN7583_Set_PON_PMA_XPON_INT_EN_0(u32 wdata)
{
	
	IO_SPHYREG(AN7583_PON_PMA_XPON_INT_EN_0 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PON_PMA_XPON_INT_EN_0);

u32 AN7583_Get_PON_PMA_XPON_INT_STA_0(void) //1fa8e288
{
	uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);

	
	return read_data;
}

EXPORT_SYMBOL(AN7583_Get_PON_PMA_XPON_INT_STA_0);



void AN7583_Set_PON_PMA_XPON_INT_STA_0(u32 wdata) //1fa8e288
{
	
	IO_SPHYREG(AN7583_PON_PMA_XPON_INT_STA_0 , wdata);	
	
}

EXPORT_SYMBOL(AN7583_Set_PON_PMA_XPON_INT_STA_0);




u32 AN7583_Rogue_Onu_Det(void)
{
	uint read_data = 0 , phypmaIntStatus = 0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_EN_0);


    if(read_data & AN7583_XPON_PMA_TRANS_ROGUE_ONU_INT_EN)
	{
	   phypmaIntStatus=IO_GPHYREG(AN7583_PON_PMA_XPON_INT_STA_0);
	   IO_SPHYREG(AN7583_PON_PMA_XPON_INT_STA_0, phypmaIntStatus);

	   //clear rogue onu cnt
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 1 );
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_SD_BEHAVIOR_SETTING_1 , 8 , 8 , 0 );

	   if(phypmaIntStatus & AN7583_XPON_PMA_TRANS_ROGUE_ONU_INT) { return 1;}
	   else { return 0;}
    }
	else
	{
      return 0;
	}    
	
}

EXPORT_SYMBOL(AN7583_Rogue_Onu_Det);



u32 AN7583_PMA_Err_Read(void)
{

    uint read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_ADD_DIG_RO_RESERVE_1);

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1,  24, 24, 0x1);
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_BIST_1,  24, 24, 0x0);


	return read_data;
}

EXPORT_SYMBOL(AN7583_PMA_Err_Read);


void set_pma_fir(void) 
{
	uint cn1=0x1, c0b=0x1, c1=0x1, c2=0x0;

	wan_sel = GET_WAN_CONF();
	

	switch(wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			//PON_PHY_PRINT(PHY_MSG_ERR,"<select> pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
			printk("<select> pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
			cn1=((pma_hi_rate_opt_val)>>24)&0xff;
			c0b=((pma_hi_rate_opt_val)>>16)&0xff;
			c1 =((pma_hi_rate_opt_val)>>8)&0xff;
			c2 =(pma_hi_rate_opt_val)&0xff;
		break;
		
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:
			//PON_PHY_PRINT(PHY_MSG_ERR,"<select> pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);
			printk("<select> pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);
			cn1=((pma_low_rate_opt_val)>>24)&0xff;
			c0b=((pma_low_rate_opt_val)>>16)&0xff;
			c1 =((pma_low_rate_opt_val)>>8)&0xff;
			c2 =(pma_low_rate_opt_val)&0xff;
		break;	
		
		default:
	        //PON_PHY_PRINT(PHY_MSG_ERR,"set_pma_fir_hw mode error = 0x%x\n", gpPhyPriv->wan_sel);
	        printk("set_pma_fir_hw mode error = 0x%x\n", wan_sel);
	}	

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 24, 24, 0x01); //rg_force_sel_da_pxp_tx_fir_cn1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b,  8,  8, 0x01); //rg_force_sel_da_pxp_tx_fir_c0b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1,  24, 24, 0x01); //rg_force_sel_da_pxp_tx_fir_c2
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1,   8,  8, 0x01); //rg_force_sel_da_pxp_tx_fir_c1			
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, cn1); //0x1fa8b778
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 05, 00, c0b); //0x1fa8b778
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1,  04, 00, c1);	//0x1fa8b780
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1,  18, 16, c2);	//0x1fa8b780
}
EXPORT_SYMBOL(set_pma_fir);




void get_pma_fir(void)
{
	uint fir_cn1=0, fir_c0b=0, fir_c1=0, fir_c2=0;

    //PON_PHY_PRINT(PHY_MSG_ERR,"get SW pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
    //PON_PHY_PRINT(PHY_MSG_ERR,"get SW pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);
	printk("get SW pma_hi_rate_opt_val = 0x%x\n", pma_hi_rate_opt_val);
	printk("get SW pma_low_rate_opt_val = 0x%x\n", pma_low_rate_opt_val);

	

    fir_cn1 = ((IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b)>>16)&0x1F);  //0x1fa8b778
	fir_c0b = ((IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b)&0x3F));      //0x1fa8b778
	fir_c1	= ((IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1) &0x1F));      //0x1fa8b780
	fir_c2	= ((IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1)>>16)&0x07);   //0x1fa8b780
    
    //PON_PHY_PRINT(PHY_MSG_ERR,"get HW FIR: CN1=0x%x, C0B=0x%x, C1=0x%x, C2=0x%x\n", fir_cn1,fir_c0b,fir_c1,fir_c2);
    printk("get HW FIR: CN1=0x%x, C0B=0x%x, C1=0x%x, C2=0x%x\n", fir_cn1,fir_c0b,fir_c1,fir_c2);
}
EXPORT_SYMBOL(get_pma_fir);



//================= ASIC_PON_SERDES_API =================




//================= ASIC_XFI_PON_SERDES_API =================


u8 XFI_PON_RX_SigDet_Flag(void)
{	
	u8 i,cnt = 0;		
	
	//IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x30000); //rg_dig_reserve_0
	
	for (i=0;i<=5;i++){				
		cnt = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_11, 24, 24);   //rg_dig_reserve_8		
		cnt += cnt;
	}
	//if(dbg_print)printk("RX_SigDet_Flag, cnt %x\n",cnt);
	
	
	return cnt >= 4? 1:0;  
}
EXPORT_SYMBOL(XFI_PON_RX_SigDet_Flag);

u8 XFI_PON_RX_SigDet_OUT_Read(void)
{		
	return IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_TORGS_DEBUG_11, 24, 24);  
}
EXPORT_SYMBOL(XFI_PON_RX_SigDet_OUT_Read);

u8 XFI_PON_RX_SigDet_Flag_D(void)
{
	uint XPON_INT_STA_3 = 0 ;

	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_EN_3, 16, 16, 0x0); //rg_rx_sigdet_int_en		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_SIGDET_1, 0, 0, 0x0); //rg_sigdet_en		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 8, 8, 0x0); //rg_sigdet_rst_b		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 8, 8, 0x1); //rg_sigdet_rst_b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 16, 16, 0x1); //rg_sigdet_int	
	udelay(50);	
		
	
	XPON_INT_STA_3 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 16, 16);  //rg_sigdet_int		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_SIGDET_1, 0, 0, 0x0); //rg_sigdet_en	
	
	//if(dbg_print)printk("RX_SigDet_Flag_D %x\n",XPON_INT_STA_3.hal.rx_sigdet_int);
	return XPON_INT_STA_3;
}
EXPORT_SYMBOL(XFI_PON_RX_SigDet_Flag_D);


void XFI_PON_SigDet_Int_Init(u8 en)
{

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 16, 16, 0x1); //rg_sigdet_int			
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 31, 0, 0x0); //rg_sigdet_int		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_EN_3, 16, 16, en); //rg_rx_sigdet_int_en	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_SIGDET_1, 0, 0, en); //rg_sigdet_en		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 8, 8, 0x0); //rg_sigdet_rst_b		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_RESET_1, 8, 8, 0x1); //rg_sigdet_rst_b		
}
EXPORT_SYMBOL(XFI_PON_SigDet_Int_Init);


u8 XFI_PON_SigDet_IntEn_sta(void)
{
	u8 read_data=0;	
	
	read_data = IO_GPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_EN_3, 16, 16);  //rg_rx_sigdet_int_en	
	
	return (u8)read_data;
}
EXPORT_SYMBOL(XFI_PON_SigDet_IntEn_sta);


u32 XFI_PON_SigDet_Int_sta3_read(void)
{	
	uint read_data=0;
	
	read_data = IO_GPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 31, 0);  	
	
	return read_data;

}
EXPORT_SYMBOL(XFI_PON_SigDet_Int_sta3_read);


void XFI_PON_SigDet_Int_sta3_write(u32 data)
{		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_XPON_INT_STA_3, 31, 0, data); //rg_sigdet_rst_b	
}
EXPORT_SYMBOL(XFI_PON_SigDet_Int_sta3_write);


u8 XFI_PON_RX_CDR_LFP_L2D_sta(void)
 {
	
	u8 sta; 
	
	sta = IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8);  //rg_force_sel_da_pxp_cdr_lpf_lck2data	
	//if(dbg_print) printk("RX_CDR_LFP_L2D_sta %x\n",sta);
	return sta;//Read 1fa7b818 bit8 
}
EXPORT_SYMBOL(XFI_PON_RX_CDR_LFP_L2D_sta);


u8 XFI_PON_RX_RDY_Sta(void)
{
	uint read_data=0;
	
	read_data = IO_GPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24);  //rg_disb_rx_dly
	//if(dbg_print) printk("RX_RDY_Sta %x \n",RX_CTRL_SEQUENCE_DISB_CTRL_1.hal.rg_disb_rx_rdy);	
	return read_data ;
	
}
EXPORT_SYMBOL(XFI_PON_RX_RDY_Sta);



void XFI_PON_RX_RDY(u8 mod,u8 sel)
{
	
	//if(dbg_print) printk("RX_RDY %x, sel %x\n",mod,sel);	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24, mod); //rg_disb_rx_rdy		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24, 24, sel); //rg_force_rx_rdy	
	
}
EXPORT_SYMBOL(XFI_PON_RX_RDY);


void XFI_PON_RX_CDR_LFP_L2D(u8 mod,u8 sel)
{	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8, mod); //rg_force_sel_da_pxp_cdr_lpf_lck2data	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0, 0, sel); //rg_force_da_pxp_cdr_lpf_lck2data	
}
EXPORT_SYMBOL(XFI_PON_RX_CDR_LFP_L2D);


void XFI_PON_RX_CDR_LPF_RSTB(u8 mod,u8 sel)
{		
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, mod); //rg_force_sel_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, sel); //rg_force_da_pxp_cdr_lpf_rstb
}
EXPORT_SYMBOL(XFI_PON_RX_CDR_LPF_RSTB);


void XFI_PON_Power_Down(void)
{

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 0,0 , 0x0 );							// rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 6,6 , 0x0 );							// rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 5,5 , 0x0 );							// rg_sw_ref_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 4,4 , 0x0 );							// rg_sw_allpcs_rst_n 
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 2,2 , 0x0 );							// rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 1,1 , 0x0 );							// rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 3,3 , 0x0 );							// rg_sw_pma_rst_n
    udelay(10);


	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16,16 , 0x0 );	// rg_force_da_pxp_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0,0 , 0x0 );	// rg_force_da_pxp_cdr_pr_pieye_pwdb	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pd_pwdb, 0,0 , 0x0 );			// rg_force_da_pxp_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_fe_pwdb, 0,0 , 0x0 );			// rg_force_da_pxp_rx_fe_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_rx_scan_rst_b, 16,16 , 0x0 );		// rg_force_da_pxp_rx_sigdet_pwdb

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 24,24 , 0x0 ); 					// rg_da_xpon_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 16,16 , 0x0 ); 					// rg_da_xpon_cdr_pr_pieye_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 8,8 , 0x0 );						// rg_da_xpon_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_0, 0,0 , 0x0 );						// rg_da_xpon_rx_fe_pwdbb   
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_DA_XPON_PWDB_1, 0,0 , 0x0 );						//rg_da_xpon_rx_sigdet_pwdb 

}
EXPORT_SYMBOL(XFI_PON_Power_Down);


void XFI_PON_PCS_Reset_Hold(void)
{	
	//if(dbg_print) printk("RX_CDR_LPF_RSTB mode %x, sel%x\n",mod,sel);

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 7,7 , 0x0);       // rg_sw_xfi_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 8,8 , 0x0);       // rg_sw_xfi_rxpcs_rst_n
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 9,9 , 0x0);       // rg_sw_xfi_rxpcs_bist_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 10,10 , 0x0);     // rg_sw_hsg_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 11,11 , 0x0);     // rg_sw_hsg_rxpcs_rst_n

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 16,16 , 0x0);     // rg_sw_xfi_txmac_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 17,17 , 0x0);     // rg_sw_xfi_rxmac_rst_n
}
EXPORT_SYMBOL(XFI_PON_PCS_Reset_Hold);



void XFI_PON_PCS_Reset_Release(int pon_Spd)
{	
	//if(dbg_print) printk("RX_CDR_LPF_RSTB mode %x, sel%x\n",mod,sel);
	printk("PON spd_sel = 0x%x\n",pon_Spd);
	if(pon_Spd == Eth_Ser_5GBaseR)
	{
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 5,3 , 0x1); 
		IO_SPHYA_REG_BITS(AN7583_PON_PMA_ADD_XPON_MODE_1, 11,9 , 0x1);
		printk("5G BaseR PHY Setting!\n");			
	}
	
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 7,7 , 0x1);       // rg_sw_xfi_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 8,8 , 0x1);       // rg_sw_xfi_rxpcs_rst_n
    	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 9,9 , 0x1);       // rg_sw_xfi_rxpcs_bist_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 10,10 , 0x1);     // rg_sw_hsg_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 11,11 , 0x1);     // rg_sw_hsg_rxpcs_rst_n

	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 16,16 , 0x1);     // rg_sw_xfi_txmac_rst_n
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_SW_RST_SET, 17,17 , 0x1);     // rg_sw_xfi_rxmac_rst_n

}
EXPORT_SYMBOL(XFI_PON_PCS_Reset_Release);

void XFI_PON_TXFIR_Set(u8 cn1, u8 c0b, u8 c1, u8 c2)
{
	//TX_FIR
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 24,24 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_fir_cn1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c0b
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c1
	IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 24,24 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c2
	

    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, cn1);       //rg_force_da_pxp_tx_fir_cn1
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, c0b);         //rg_force_da_pxp_tx_fir_c0b
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, c1);          //rg_force_da_pxp_tx_fir_c1
    IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, c2);        //rg_force_da_pxp_tx_fir_c2
}
EXPORT_SYMBOL(XFI_PON_TXFIR_Set);

void XFI_PON_TXFIR_Get(void)
{
    u8 cn1 = 0,c0b = 0,c1 = 0,c2 = 0;
    //TX_FIR
    cn1 = IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16);       //rg_force_da_pxp_tx_fir_cn1
    c0b = IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0);         //rg_force_da_pxp_tx_fir_c0b
    c1  = IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0);          //rg_force_da_pxp_tx_fir_c1
    c2  = IO_GPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16);        //rg_force_da_pxp_tx_fir_c2

    printk("XFI PON TXFIR[cn1,c0b,c1,c2] = [%x, %x, %x, %x]\n",cn1,c0b,c1,c2);
}
EXPORT_SYMBOL(XFI_PON_TXFIR_Get);


//================= ASIC_XFI_PON_SERDES_API =================





void PON_RG_RW_test(void)
{
	printk("\nPON_RW_test\n");
	//printk("RG:0x%x = 0x%x\n", AN7583_PON_PMA_rg_force_da_pxp_jcpll_ckout_en, IO_GPHYREG(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en));
	//JCPLL_Dis
	//IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_jcpll_en
	//printk("RG:0x%x = 0x%x\n", AN7583_PON_PMA_rg_force_da_pxp_jcpll_ckout_en, IO_GPHYREG(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en));
}
EXPORT_SYMBOL(PON_RG_RW_test);


void PON_PMA_PON_CK_SET(u32 data)
{
	//printk("\nAN7583_PON_PMA_PON_CK_SET 0x%8x = 0x%8x\n",AN7583_PON_PMA_PON_CK_SET,data);
	IO_SPHYREG(AN7583_PON_PMA_PON_CK_SET, data);
}
EXPORT_SYMBOL(PON_PMA_PON_CK_SET);

u32 PON_PMA_PON_CK_GET(void)
{
	u32 read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_PON_CK_SET);
	//printk("\nAN7583_PON_PMA_PON_CK_GET 0x%8x = 0x%8x\n",AN7583_PON_PMA_PON_CK_SET,read_data);
	return read_data;
}
EXPORT_SYMBOL(PON_PMA_PON_CK_GET);

u32 PON_PMA_LCLCPLL_TDC_RO_4_GET(void)
{
	u32 read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_LCPLL_TDC_RO_4);
	//printk("\nAN7583_PON_PMA_LCPLL_TDC_RO_4_GET 0x%8x = 0x%8x\n",AN7583_PON_PMA_LCPLL_TDC_RO_4,read_data);
	return read_data;
}
EXPORT_SYMBOL(PON_PMA_LCLCPLL_TDC_RO_4_GET);


void PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_SET(u32 data)
{
	IO_SPHYREG(AN7583_PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN, data);
}
EXPORT_SYMBOL(PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_SET);

u32 PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_GET(void)
{
	u32 read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN);
	return read_data;
}
EXPORT_SYMBOL(PON_ANA_RG_XPON_TXPLL_PHY_CK1_EN_GET);

void PON_PMA_rg_force_da_pxp_txpll_ckout_en_SET(u32 data)
{
	IO_SPHYREG(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, data);
}
EXPORT_SYMBOL(PON_PMA_rg_force_da_pxp_txpll_ckout_en_SET);

u32 PON_PMA_rg_force_da_pxp_txpll_ckout_en_GET(void)
{
	u32 read_data=0;
	read_data=IO_GPHYREG(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en);
	return read_data;
}
EXPORT_SYMBOL(PON_PMA_rg_force_da_pxp_txpll_ckout_en_GET);



void Pon_Opt_Cal (int mode_Sel , int pat_sel)
{

   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_cdr_pr_idac, 24, 24, 0x01);     //rg_force_sel_da_pxp_txpll_sdm_pcw
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_3, 8, 8, 0x01);  //rg_lcpll_ncpo_load
   udelay(100);

   if(mode_Sel == 0x0)
   {	   
	  IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_2, 31, 0, 0x33900000);  //rg_lcpll_ncpo_value
   }
   else if (mode_Sel == 0x1)
   {
	   IO_SPHYA_REG_BITS(AN7583_PON_PMA_LCPLL_TDC_FLT_2, 31, 0, 0x31C432CA);  //rg_lcpll_ncpo_value
   }
   

   if(pat_sel == 0x0)
   {	   
	  IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 4, 0, pon_pma_param.TX_Pat = PRBS7 );   //rg_bistctl_pat_sel 
   }
   else if (pat_sel == 0x1)
   {
	  IO_SPHYA_REG_BITS(AN7583_PON_PMA_BISTCTL_CONTROL, 4, 0, pon_pma_param.TX_Pat = PRBS31 );   //rg_bistctl_pat_sel 
   }
   

   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x00); //rg_force_da_pxp_txpll_en
   udelay(100);
   IO_SPHYA_REG_BITS(AN7583_PON_PMA_rg_force_da_pxp_txpll_ckout_en, 16, 16, 0x01); //rg_force_da_pxp_txpll_en

}
EXPORT_SYMBOL(Pon_Opt_Cal);








EXPORT_SYMBOL(xpon_init);
EXPORT_SYMBOL(fiber_plug_reset);




