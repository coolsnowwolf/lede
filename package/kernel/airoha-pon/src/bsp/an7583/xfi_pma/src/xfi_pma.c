//******************************************************************************
//
// Copyright (C) 2017 ECONET
//
// Abstract : AN7583 XFI PHY
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

#include "../inc/an7583_xfi_pma.h"
#include "../../phy_efuse_def.h"



uint InitSpd_TX = 0 , InitSpd_TXPLL = 0 , InitSpd_RX = 0 , InitSpd_ANA = 0 , InitSpd_Preset = 0 , InitSpd_RxOn = 0;
u32 XFI_rx_term_sel = 0 , XFI_tx_term_sel = 0 , XFI_CMN_TRIM = 0;
static u8 xfi_cn1_set = 0x0, xfi_c0b_set = 0xE, xfi_c1_set = 0x4, xfi_c2_set = 0x0;

// --  variables ----------------------------------------------------------------

void __iomem *xfi_ana = NULL; 
void __iomem *xfi_pma  = NULL;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern void __iomem* Get_Base(u32 base);
extern u32 get_olt_phy_data(u32 reg);
extern void set_olt_phy_data(u32 reg, u32 val);
extern u32 get_phy_efuse(u32 start_bit, u32 len);


/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
static void get_xfi_ana_base(void)
{
	xfi_ana = Get_Base(XFI_ANA_RG_BASE); 
	if (IS_ERR(xfi_ana))
	{
		printk("[ERROR]xfi_ana base= %lx\n", (unsigned long)xfi_ana);
	}
}

static void get_xfi_pma_base(void)
{
	xfi_pma = Get_Base(XFI_PMA_RG_BASE); 
	if (IS_ERR(xfi_pma))
	{
		printk("[ERROR]xfi_pma base= %lx\n", (unsigned long)xfi_pma);
	}
}

static void set_xfi_ana_rg(u32 reg, u32 val)
{
	if(xfi_ana == NULL)
	{
		get_xfi_ana_base();
	}
	writel(val, xfi_ana + reg); 
}	

static void set_xfi_pma_rg(u32 reg, u32 val)
{
	if(xfi_pma == NULL)
	{
		get_xfi_pma_base();
	}
	writel(val, xfi_pma + reg); 
}	

static u32 get_xfi_ana_rg(u32 reg)
{
	if(xfi_ana == NULL)
	{
		get_xfi_ana_base();
	}
	return readl(xfi_ana + reg);
}

static u32 get_xfi_pma_rg(u32 reg)
{
	if(xfi_pma == NULL)
	{
		get_xfi_pma_base();
	}
	return readl(xfi_pma + reg);
}

static u32 get_xfi_phya_data(u32 reg)
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

	if( (XFI_ANA_RG_BASE <= reg_phy) && (reg_phy < XFI_ANA_RG_BASE+0xfff) )
    {    	
    	reg_val= get_xfi_ana_rg(reg_phy - XFI_ANA_RG_BASE);
    } 
	else if( (XFI_PMA_RG_BASE <= reg_phy) && (reg_phy < XFI_PMA_RG_BASE+0xfff) )
    {    	
    	reg_val= get_xfi_pma_rg(reg_phy - XFI_PMA_RG_BASE);
    }
	else
	{
		printk("\n Datapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
	}

	return reg_val;
}

static u32 get_xfi_bcdr_data(u32 reg)
{
	u32 reg_phy = 0;
	u32 reg_val = 0;

	// translate addr to physical addr 
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Get reg_val error, reg=0x%08X\n", __func__, reg);		
		return 0;
	}
	
	if( (XFI_BCDR_RG_BASE <= reg_phy) && (reg_phy < XFI_BCDR_RG_BASE+0xC4) )
	{
		reg_val= get_olt_phy_data(reg_phy);	//implement in ecnt_olt_phy.c
	}
	else if( (XFI_BIST_RG_BASE <= reg_phy) && (reg_phy < XFI_BIST_RG_BASE+0x3C) )
	{
		reg_val= get_olt_phy_data(reg_phy); //implement in ecnt_olt_phy.c
	}
	else
	{
		printk("\n Datapath(%s) get reg error, reg=0x%08X\n", __func__, reg);
	}

	return reg_val;
}

static void set_xfi_phya_data(u32 reg, u32 val)
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
	
	if( (XFI_ANA_RG_BASE <= reg_phy) && (reg_phy < XFI_ANA_RG_BASE+0xfff) )
    {    	
    	set_xfi_ana_rg(reg_phy - XFI_ANA_RG_BASE, val);
    } 
	else if( (XFI_PMA_RG_BASE <= reg_phy) && (reg_phy < XFI_PMA_RG_BASE+0xfff) )
    {    	
    	set_xfi_pma_rg(reg_phy - XFI_PMA_RG_BASE, val);
    }
	else
	{
		printk("\nDatapath(%s) set reg error, reg=0x%08X\n", __func__, reg);
	}

}

static void set_xfi_bcdr_data(u32 reg, u32 val)
{
	u32 reg_phy = 0;
	
	// translate addr to physical addr 
	if( reg > 0xa0000000)
		reg_phy = (reg & 0x1fffffff);
	else
		reg_phy = reg;

	//reg_offset = reg_phy % 4;
	if(( reg_phy % 4) != 0){
		printk("\n(%s)Set reg_val error, reg=0x%08X\n", __func__, reg);		
		return ;
	}
	
	if( (XFI_BCDR_RG_BASE <= reg_phy) && (reg_phy < XFI_BCDR_RG_BASE+0xC4) )
	{
		set_olt_phy_data(reg_phy, val);	//implement in ecnt_olt_phy.c
	}
	else if( (XFI_BIST_RG_BASE <= reg_phy) && (reg_phy < XFI_BIST_RG_BASE+0x3C) )
	{
		set_olt_phy_data(reg_phy, val); //implement in ecnt_olt_phy.c
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
			if( (reg_name & 0xfffff000) == 0x1fbe5000)
				return (IO_GBCDR_REG(reg_name)) ; 
			else
				return (IO_GPHYREG(reg_name)) ; 
		}
		else
		{
			if( (reg_name & 0xfffff000) == 0x1fbe5000)
				return ((IO_GBCDR_REG(reg_name)>>start_index) & ((1<<(end_index-start_index+1))-1)) ;
			else
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

	if( (reg_name & 0xfffff000) == 0x1fbe5000)
		data=IO_GBCDR_REG(reg_name);
	else
		data=IO_GPHYREG(reg_name);
		
	if((end_index>=start_index)&&(end_index<32))
	{	
		if((end_index==31)&&(start_index==0))
		{
			if( (reg_name & 0xfffff000) == 0x1fbe5000)
				IO_SBCDR_REG(reg_name,value);
			else
				IO_SPHYREG(reg_name,value);
		}
		else
		{
			if( (reg_name & 0xfffff000) == 0x1fbe5000)
				IO_SBCDR_REG(reg_name,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index))) ;
			else
				IO_SPHYREG(reg_name,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index))) ;
		}
	}
	else
	{
		printk("%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
	}
}





void XFI_Init (int spd_sel)
{
	XFI_DIG_reset_hold(spd_sel);	
	XFI_CfgPhyType(spd_sel);
	XFI_LinkControl();  
}


void XFI_LinkControl (void)
{   
   XFI_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	   
   XFI_TX_on();
}


void OLT_plug_reset (int spd_sel) // RX_init
{

   XFI_RX_preset(InitSpd_Preset);            
   XFI_RX_on(InitSpd_RxOn); 
   XFI_RX_L2R(spd_sel); 			
   XFI_DIG_ref_release();

   XFI_RX_OSCal(); 
   //XFI_RX_pical();  
   XFI_RX_pdos(); 
   XFI_RX_feos();
   XFI_RX_sdcal();   
   
   XFI_phy_status();			
   //XFI_OLT_Eth_DIG_reset();
   XFI_DIG_reset_release(spd_sel);
            
   //EO_Scan(pon_Spd,0,7); //20221011           
   XFI_OLT_RX_rxrdy();  
   //XFI_OLT_BIST_setting();
   XFI_Eth_OLT_R2T_sel(nor_pma_data);              

   //-----------------------------------------------------------------
   
}




void Eth_Ser_plug_reset (int plug_sel , int spd_sel)
{
    static int  plug_out_flag=1,plug_in_flag=0;
    switch(plug_sel)
	{
        case XFI_FIRST_PLUG_IN: 
            XFI_RX_preset(InitSpd_Preset);            
            XFI_RX_on(InitSpd_RxOn); 
            XFI_RX_L2R(spd_sel); 			
			XFI_DIG_ref_release();

            XFI_RX_OSCal(); 
            XFI_RX_pical();  
            XFI_RX_pdos(); 
            XFI_RX_feos();
            XFI_RX_sdcal();   
   
            XFI_phy_status();			
			XFI_DIG_reset_release(spd_sel);

			XFI_RX_L2D();
            //EO_Scan(pon_Spd,0,7); //20221011           
			XFI_RX_rxrdy(); 
			XFI_Eth_OLT_R2T_sel(nor_pma_data);       
			
            plug_out_flag =1;
			plug_in_flag =0;
            
        break;	
        //-----------------------------------------------------------------

	  case XFI_PLUG_IN: 
			if(plug_in_flag ==1)
			{  	
			    XFI_DIG_reset_hold(spd_sel);				
			    XFI_RX_L2D();			   
			    //XPON_TDC_on(); 
			   
			    XFI_DIG_reset_release(spd_sel);		
			    XFI_RX_rxrdy(); 			   
			    XFI_phy_status();
			    XFI_Eth_OLT_R2T_sel(nor_pma_data);       
		       
				plug_out_flag =1;
				plug_in_flag =0;
				if(XFI_PCS_PRINT)printk("XFI_PLUG_IN, spd_sel = %x\n",spd_sel);				
			}
        break;	
        //-----------------------------------------------------------------

        case XFI_PLUG_OUT:
			if(plug_out_flag ==1)
			{
				//XPON_TDC_off();	
			    XFI_RX_L2R(spd_sel); 

				plug_out_flag =0;
				plug_in_flag =1;
				if(XFI_PCS_PRINT)printk("XFI_PLUG_OUT, spd_sel = %x\n",spd_sel);
			}
        break;
        default:
	    break;
    }

}






void XFI_CfgPhyType(int spd_sel)
{

  XFI_WanSelInit(spd_sel);

  //TXPLL_Dis
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 24,24 , 0x1); // rg_force_sel_da_pxp_txpll_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 16,16 , 0x0); // rg_force_da_pxp_txpll_en

  //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_FLT_3, 1,0 , 0x1);                // rg_lcpll_ncpo_shift
  //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_FLT_1, 11,8 , 0x5);               // rg_lcpll_a_tdc
  //IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TX_TERMCAL_SELPN, 17,16 , 0x0);     // RG_XPON_TX_TDC_CK_SEL
  //IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 24,24 , 0x1);         // RG_XPON_RX_TDC_CK_SEL 

                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_1, 31,24 , 0x1);        //rg_lcpll_ck_stb_timer      
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_1, 23,16 , 0x10);       //rg_lcpll_pcw_man_load_timer;
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_1, 15,8 , 0xA);         //rg_lcpll_en_timer          
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_1, 0,0 , 0x1);          //rg lcpll_man_pwdb  
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_1, 31,16 , 0x113);           // rg_tx_hsdata_en_wait
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_1, 15,0 , 0xFA);             // rg_tx_ck_en_wait  
                                                                            
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_2, 31,16 , 0x9B);            // rg_tx_serdes_rdy_wait
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_2, 15,0 , 0x210);            // rg_tx_power_on_wait
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_0, 31,16 , 0x4);             // rg_txcalib_5us
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PON_TX_COUNTER_0, 15,0 , 0x26);             // rg_txcalib_50us
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_DLY_CTRL, 23,16 , 0x2);                  // rg_tx_ben_exten_ftune
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_FLT_0, 10,8 , 0x3);               // rg_lcpll_ki
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PW_5, 24,24 , 0x0);               // rg_lcpll_tdc_sync_in_mode

  XFI_TXPLL(InitSpd_TXPLL);
  XFI_TX(InitSpd_TX);   

  //pma_dig_rx_setting 
  //RX HW mode counter 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0, 23,8 , 0x1);       // rg_rx_os_start
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_6, 15,0 , 0x2);       // rg_rx_os_end
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0, 2,0 , 0x1);        // rg_osc_speed_opt
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1, 15,0 , 0x2);       // rg_rx_pical_start
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1, 31,16 , 0x3E8);    // rg_rx_pical_end
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4, 15,0 , 0x2);       // rg_rx_sdcal_start
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4, 31,16 , 0x3E8);    // rg_rx_sdcal_end
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2, 15,0 , 0x2);       // rg_rx_pdos_start
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2, 31,16 , 0x3E8);    // rg_rx_pdos_end
                                                                              
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3, 15,0 , 0x2);       // rg_rx_feos_start
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3, 31,16 , 0x3E8);    // rg_rx_feos_end

  //RX setting
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_2, 12,8 , 0x1);                 // rg_fom_num_order
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_2, 1,0 , 0x3);                  // rg_a_sel

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26,16 , 0x240); // rg_x_max
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10,0 , 0x1C0);  // rg_x_min

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8,8 , 0x0);       // rg_data_shift
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 0,0 , 0x1);       // rg_eyecnt_fast

  XFI_RX(InitSpd_RX); 	
  XFI_ANA(InitSpd_ANA); 

  
  //EYE
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16,16 , 0x1);   // rg_eye_nextpts

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15,8 , 0x4);  //rg_eyecnt_vth
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7,0 , 0x4);   //rg_eyecnt_hth

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23,16 , 0x4); //rg_eo_vth
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10,0 , 0x4);  //rg_eo_hth

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31,24 , 0xFF);    // rg_eye_mask
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9,0 , 0xD0);      // rg_cntlen

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 7,0 , 0x1);                  // rg_eq_en_delay
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 31,24 , 0x0);                // rg_veo_mask
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 18,8 , 0x0);                 // rg_heo_mask

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_1, 0,0 , 0x1);                  // rg_a_lgain

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_1, 25,24 , 0x1);                    // rg_cal_cyc
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_1, 15,8 , 0x2E);                    // rg_cal_1us_set
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_1, 0,0 , 0x1);                      // rg_sim_fast_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_1, 17,16 , 0x1);                    // rg_cal_stb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_2, 17,16 , 0x0);                    // rg_cal_cyc_time
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_2, 11,8 , 0x0);                     // rg_cal_out_os
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CAL_2, 0,0 , 0x0);                      // rg_cal_os_pulse

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5, 15,0 , 0x5);      // rg_rx_blwc_rdy_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5, 31,16 , 0xA);     // rg_rx_rdy

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FEOS, 7,0 , 0x0);                       // rg_lfsel           
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FEOS, 8,8 , 0x0);                       // rg_eq_force_blwc_freeze

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 6,0 , 0x40);   // rg_y_min
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 14,8 , 0x3F);  // rg_y_max
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16,16 , 0x1);  // rg_index_mode
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19,0 , 0x18);  // rg_eyedur

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EXTRAL_CTRL, 15,8 , 0x2);               // rg_l2d_trig_eq_en_time
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EXTRAL_CTRL, 1,1 , 0x1);                // rg_os_rdy_latch
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EXTRAL_CTRL, 0,0 , 0x0);                // rg_disb_leq

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_0, 25,24 , 0x0);                    // rg_kband_kfc            
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_0, 18,8 , 0xA5);                    // rg_fpkdiv
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_0, 2,0 , 0x2);                      // rg_kband_prediv

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_1, 26,24 , 0x4);                    // rg_symbol_wd
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_1, 18,16 , 0x1);                    // rg_settle_time_sel

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_5, 10,0 , 0x1FF);                   // rg_fll_idac_max
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_5, 26,16 , 0x400);                  // rg_fll_idac_min

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_2, 10,8 , 0x4);                     // rg_amp     
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_2, 2,0 , 0x3);                      // rg_prbs_sel

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_4, 24,24 , 0x0);              // rg_disb_blwc_offset 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PDOS_CTRL_0, 0,0 , 0x1);                // rg_eye_blwc_add
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PDOS_CTRL_0, 8,8 , 0x0);                // rg_data_blwc_add

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_BLWC, 0,0 , 0x1);                       // rg_eq_blwc_pol
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_BLWC, 11,8 , 0xA);                      // rg_eq_blwc_gain
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_BLWC, 22,16 , 0x70);                    // rg_eq_blwc_cnt_top_lim
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_BLWC, 29,23 , 0x10);                    // rg_eq_blwc_cnt_bot_lim
  

#if 0
   //********************************************OPTIMIZE CMN_TRIM by EFUSE*****************************************************// 

    XFI_CMN_TRIM = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XFI_CMN_TRIM_LSB, 5) : 0x1;	

	if ((XFI_CMN_TRIM > 0xD) && (XFI_CMN_TRIM <= 0x16)){
	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_EN, 28, 24, XFI_CMN_TRIM);

	 #if LAB_PRINT_XFI
	   printk("DA_XFI_CMN_TRIM[28:24]=%x\n", XFI_CMN_TRIM);
	 #endif 
	}
	else {
	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_EN, 28, 24, 0x10);

	 #if LAB_PRINT_XFI
	   printk("Set CMN_TRIM default! \n");
	 #endif 
	}

  //*****************************************************************************************************************************//
 #endif
  
}



void XFI_WanSelInit (uint xfi_Spd)
{

   switch(xfi_Spd)
   {     
	   case Eth_Ser_USXGMII: 	   //DS(RX)_10.31252G  /  US(TX)_10.3125G 
		 
		InitSpd_TXPLL = 3; 
		InitSpd_TX = 1; 
		InitSpd_RX = 1;
		InitSpd_ANA = 3;
		InitSpd_Preset = 1;
		InitSpd_RxOn= 1;

		break;	


	   case Eth_Ser_5GBaseR:       //DS(RX)_5.15625G   /  US(TX)_5.15625G

		InitSpd_TXPLL = 3; 
		InitSpd_TX = 2; 
		InitSpd_RX = 2;
		InitSpd_ANA = 3;
		InitSpd_Preset = 2;
		InitSpd_RxOn= 2;	

		break;	
		
	   
	   case Eth_Ser_HSGMII:           //DS(RX)_3.125G      /  US(TX)_3.125G

		InitSpd_TXPLL = 1; 
		InitSpd_TX = 3; 
		InitSpd_RX = 3;
		InitSpd_ANA = 1;
		InitSpd_Preset = 3;
		InitSpd_RxOn= 3;

		
		break;

	  
	   case Eth_Ser_SGMII:            //DS(RX)_1.25G       /  US(TX)_1.25G

		InitSpd_TXPLL = 4; 
		InitSpd_TX = 4; 
		InitSpd_RX = 4;
		InitSpd_ANA = 4;
		InitSpd_Preset = 4;
		InitSpd_RxOn= 4;			

		break;
		

	   case OLT_GPON_ASYM_MODE:      //DS(RX)_1.24416G  /  US(TX)_2.48832G

		InitSpd_TXPLL = 2; 
		InitSpd_TX = 5; 
		InitSpd_RX = 5;
		InitSpd_ANA = 2;
		InitSpd_Preset = 5;
		InitSpd_RxOn= 5;		

		break;


	   case OLT_GPON_SYM_MODE:       //DS(RX)_2.48832G  /  US(TX)_2.48832G
	   case OLT_GPON_SYM_MODE_7552: 
	   case OLT_GPON_MIX_MODE:
	   case OLT_MODE_DEFAULT:

		InitSpd_TXPLL = 2; 
		InitSpd_TX = 5; 
		InitSpd_RX = 6;
		InitSpd_ANA = 2;
		InitSpd_Preset = 6;
		InitSpd_RxOn= 6;				

		break; 
		
   }

}




void XFI_TXPLL(uint xfi_txpll)
{
  //setting_mode_XPON_TXPLL
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_idac, 24,24 , 0x1);          // rg_force_sel_da_pxp_txpll_sdm_pcw
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_FLT_3, 8,8 , 0x1 );					   // rg_lcpll_ncpo_load

 
  if(xfi_txpll == XFI_10p3125G_BYP_JCPLL) //DS(RX)_10.31252G  /  US(TX)_10.3125G & //DS(RX)_5.15625G  /  US(TX)_5.15625G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x33900000);  // rg_force_da_pxp_txpll_sdm_pcw
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x67200000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x67200000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_epon
  }  
  else if(xfi_txpll == XFI_12p4G_BYP_JCPLL) //DS(RX)_1.24416G  /  US(TX)_2.48832G & DS(RX)_2.48832G  /  US(TX)_2.48832G
  {
  
      IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x3E353F7C);  // rg_force_da_pxp_txpll_sdm_pcw
      IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x7C6A7EF8);         // rg_lcpll_pon_hrdds_pcw_ncpo_gpon
      IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x7C6A7EF8);         // rg_lcpll_pon_hrdds_pcw_ncpo_epon
  }
  else if(xfi_txpll == XFI_12p5G_BYP_JCPLL) //DS(RX)_3.125G  /  US(TX)_3.125G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x3E800000);  // rg_force_da_pxp_txpll_sdm_pcw
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x7D000000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x7D000000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_epon
  }
  else //DS(RX)_1.25G  /  US(TX)_1.25G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_sdm_pcw, 30,0 , 0x32000000);  // rg_force_da_pxp_txpll_sdm_pcw
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_1, 30,0 , 0x64000000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_TDC_PCW_2, 30,0 , 0x64000000); 		// rg_lcpll_pon_hrdds_pcw_ncpo_epon
  }


#if LAB_PRINT_XFI
   printk("=== XFI_TXPLL_done ===\n");
#endif
}

void XFI_ETH_TXFIR_Bringup_Setting(u8 cn1, u8 c0b, u8 c1, u8 c2,u8 prt)
{
    xfi_cn1_set = cn1;
    xfi_c0b_set = c0b;
    xfi_c1_set = c1;
    xfi_c2_set = c2;
    if(prt) printk("ETH TXFIR Default Set as [%x, %x, %x, %x]\n",xfi_cn1_set,xfi_c0b_set,xfi_c1_set,xfi_c2_set);
}
EXPORT_SYMBOL(XFI_ETH_TXFIR_Bringup_Setting);


void XFI_TX(uint xfi_tx)
{
  //setting_mode_XFI_TX
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TX_CKLDO_EN, 24,24 , 0x1);           // RG_XPON_TX_DMEDGEGEN_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TX_CKLDO_EN, 0,0 , 0x1);             // RG_XPON_TX_CKLDO_EN
                                                                               
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_EN, 0,0 , 0x1);                  // RG_XPON_CMN_EN   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_EN, 18,16 , 0x4);                // RG_XPON_CMN_VREFSEL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_EN, 13,8 , 0x1);                 // RG_XPON_CMN_MPXSELTOP_DC
  //IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CMN_MODE_SEL, 8,8 , 0x0);            // RG_XPON_CMN_BYPASS_LPF

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_acjtag_en, 24,24 , 0x1);  // rg_force_sel_da_pxp_tx_ckin_sel
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_acjtag_en, 16,16 , 0x1);  // rg_force_da_pxp_tx_ckin_sel


  //TX_FIR
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 24,24 , 0x1);    // rg_force_sel_da_pxp_tx_fir_cn1
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 8,8 , 0x1);      // rg_force_sel_da_pxp_tx_fir_c0b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 8,8 , 0x1);       // rg_force_sel_da_pxp_tx_fir_c1
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 24,24 , 0x1);     // rg_force_sel_da_pxp_tx_fir_c2

  if(xfi_tx == TX_Spd_USXGMII)
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, xfi_cn1_set); //rg_force_da_pxp_tx_fir_cn1
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, xfi_c0b_set);   //rg_force_da_pxp_tx_fir_c0b
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, xfi_c1_set);    //rg_force_da_pxp_tx_fir_c1
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, xfi_c2_set);  //rg_force_da_pxp_tx_fir_c2
  }
  else
  {

     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 20,16 , 0x0);    // rg_force_da_pxp_tx_fir_cn1
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 5,0 , 0x0);      // rg_force_da_pxp_tx_fir_c0b
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 4,0 , 0x0);       // rg_force_da_pxp_tx_fir_c1
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 18,16 , 0x0);     // rg_force_da_pxp_tx_fir_c2
  }


  if(xfi_tx == TX_Spd_USXGMII) //DS(RX)_10.31252G  /  US(TX)_10.3125G 
  {
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1);   // rg_force_sel_da_pxp_tx_ckin_divisor
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x5);   // rg_force_da_pxp_tx_ckin_divisor //DIV2.5

     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1);    // rg_force_sel_da_pxp_tx_rate_ctrl
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x2);    // rg_force_da_pxp_tx_rate_ctrl
     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_xpon_tx_rate_ctrl, 1,0 , 0x2);               // rg_pon_tx_rate_ctrl (16bit)
  }
  
  else if (xfi_tx == TX_Spd_OLT)//DS(RX)_1.24416G  /  US(TX)_2.48832G & DS(RX)_2.48832G  /  US(TX)_2.48832G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1);	// rg_force_sel_da_pxp_tx_ckin_divisor
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x0);	// rg_force_da_pxp_tx_ckin_divisor //DIV2.5
	  
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1);	// rg_force_sel_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x2);	// rg_force_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_xpon_tx_rate_ctrl, 1,0 , 0x2);				// rg_pon_tx_rate_ctrl (16bit)  
  }
  else if (xfi_tx == TX_Spd_5GBaseR) //DS(RX)_5.15625G   /  US(TX)_5.15625G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1);	// rg_force_sel_da_pxp_tx_ckin_divisor
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x4);	// rg_force_da_pxp_tx_ckin_divisor //DIV2.5
	  
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1);	// rg_force_sel_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x2);	// rg_force_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_xpon_tx_rate_ctrl, 1,0 , 0x2);				// rg_pon_tx_rate_ctrl (16bit)  
  }
    else if (xfi_tx == TX_Spd_HSGMII) //DS(RX)_3.125G      /  US(TX)_3.125G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1);	// rg_force_sel_da_pxp_tx_ckin_divisor
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x4);	// rg_force_da_pxp_tx_ckin_divisor //DIV2.5
	  
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1);	// rg_force_sel_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x1);	// rg_force_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_xpon_tx_rate_ctrl, 1,0 , 0x1);				// rg_pon_tx_rate_ctrl (10bit)  
  }
  else //DS(RX)_1.25G      /  US(TX)_1.25G
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 24,24 , 0x1);	// rg_force_sel_da_pxp_tx_ckin_divisor
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_term_sel, 19,16 , 0x2);	// rg_force_da_pxp_tx_ckin_divisor //DIV2.5
	  
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 8,8 , 0x1);	// rg_force_sel_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 1,0 , 0x1);	// rg_force_da_pxp_tx_rate_ctrl
	  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_xpon_tx_rate_ctrl, 1,0 , 0x1);				// rg_pon_tx_rate_ctrl (10bit)  
  }


#if LAB_PRINT_XFI
   printk("=== XFI_TX_done ===\n");
#endif  
}


void XFI_RX(uint xfi_rx)
{
  //setting_mode_XFI_RX
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 19,18 , 0x0);              // RG_XPON_RX_REV_1 
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 22,20 , 0x4);              // RG_XPON_RX_REV_1
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 26,24 , 0x4);              // RG_XPON_RX_REV_1  

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_OSCAL_FORCE, 17,8 , 0x3FF);       // RG_PXP_RX_OSCAL_FORCE

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PD_PICAL_CKD8_INV, 8,0 , 0x0);   // RG_XPON_CDR_PD_EDGE_DIS
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PD_PICAL_CKD8_INV, 0,0 , 0x0);   // RG_XPON_CDR_PD_PICAL_CKD8_INV

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_rstb, 24,24 , 0x1);      // rg_force_sel_da_pxp_cdr_injck_sel
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_rstb, 16,16 , 0x1);      // rg_force_da_pxp_cdr_injck_sel

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);        //rg_force_sel_da_pxp_rx_fe_gain_ctrl

  
  if(xfi_rx == RX_Spd_USXGMII) //DS(RX)_10.31252G  /  US(TX)_10.3125G 
  {

    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x1030);            // RG_XPON_RX_REV_0
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x1); // rg_force_da_pxp_rx_fe_gain_ctrl	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x100);            //rg_dig_reserve_0  
    
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x1);                 // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );				  // rg_disb_da_XPON_RX_FE_GAIN_CTRL
    
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x8);         // RG_XPON_CDR_PR_BETA_DAC
    
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x2);            // RG_XPON_RX_PHYCK_SEL
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);         // RG_XPON_CDR_PR_XFICK_EN

    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);         // RG_XPON_RX_PHY_CK_SEL_FORCE
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);         // RG_XPON_RX_PHY_CK_SEL

    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);          // RG_XPON_RX_PHYCK_RSTB
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x42);           // RG_XPON_RX_PHYCK_DIV

	//RX_OSR_setting
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x0);           // RG_XPON_CDR_LPF_RATIO

    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0);           // RG_XPON_RX_BUSBIT_SEL_FORCE
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0);           // RG_XPON_RX_BUSBIT_SEL

    
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);     // rg_force_sel_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x0);     // rg_force_da_pxp_rx_osr_sel
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2);           // rg_xpon_rx_rate_ctrl (16bit)
  }
  else if(xfi_rx == RX_Spd_Asym_OLT) //DS(RX)_1.24416G  /  US(TX)_2.48832G
  {	  

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0); 		   // RG_XPON_RX_REV_0
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x3); // rg_force_da_pxp_rx_fe_gain_ctrl	
	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x3);				  // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]	 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );				  // rg_disb_da_XPON_RX_FE_GAIN_CTRL
    
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x6);		  // RG_XPON_CDR_PR_BETA_DAC
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 20,16 , 0x6); //RG_XPON_CDR_PR_DAC_BAND
		
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1); 		      // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);		   // RG_XPON_CDR_PR_XFICK_EN
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);		   // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);		   // RG_XPON_RX_PHY_CK_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);		   // RG_XPON_RX_PHYCK_RSTB
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x15);		   // RG_XPON_RX_PHYCK_DIV
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x1);		   // RG_XPON_CDR_LPF_RATIO
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);	  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x1);	  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2); 		  // rg_xpon_rx_rate_ctrl (16bit)
  }
  else if(xfi_rx == RX_Spd_Sym_OLT) //DS(RX)_2.48832G  /  US(TX)_2.48832G
  {	  

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0); 		   // RG_XPON_RX_REV_0	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x3); // rg_force_da_pxp_rx_fe_gain_ctrl	
	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x3);			   // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]	 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );			   // rg_disb_da_XPON_RX_FE_GAIN_CTRL
			
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x6);		   // RG_XPON_CDR_PR_BETA_DAC
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 20,16 , 0x6);      //RG_XPON_CDR_PR_DAC_BAND

		
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1); 		   // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);		   // RG_XPON_CDR_PR_XFICK_EN
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);		   // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);		   // RG_XPON_RX_PHY_CK_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);		   // RG_XPON_RX_PHYCK_RSTB
	
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x14);		   // RG_XPON_RX_PHYCK_DIV
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x0);		   // RG_XPON_CDR_LPF_RATIO
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);	  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x0);	  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2); 		  // rg_xpon_rx_rate_ctrl (16bit)
  }
  else if(xfi_rx == RX_Spd_5GBaseR) //DS(RX)_5.15625G   /  US(TX)_5.15625G
  {	  

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0); 		   // RG_XPON_RX_REV_0
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x3); // rg_force_da_pxp_rx_fe_gain_ctrl	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x400);         //rg_dig_reserve_0  
	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x3);			   // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );			   // rg_disb_da_XPON_RX_FE_GAIN_CTRL
    
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x8);	   // RG_XPON_CDR_PR_BETA_DAC		
	
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x2); 		   // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);	   // RG_XPON_CDR_PR_XFICK_EN
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);	   // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);	   // RG_XPON_RX_PHY_CK_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);	   // RG_XPON_RX_PHYCK_RSTB	
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x42);		   // RG_XPON_RX_PHYCK_DIV
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x1);		   // RG_XPON_CDR_LPF_RATIO

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x1);		   // RG_XPON_RX_BUSBIT_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x1);  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x2); 	   // rg_xpon_rx_rate_ctrl (16bit)
  }
  else if(xfi_rx == RX_Spd_HSGMII) //DS(RX)_3.125G      /  US(TX)_3.125G
  {	  

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0); 		   // RG_XPON_RX_REV_0	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x3); // rg_force_da_pxp_rx_fe_gain_ctrl	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x300);         //rg_dig_reserve_0  
	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x3);			   // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );			   // rg_disb_da_XPON_RX_FE_GAIN_CTRL
	
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x6);	   // RG_XPON_CDR_PR_BETA_DAC
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 20,16 , 0x6); //RG_XPON_CDR_PR_DAC_BAND
		
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1); 		   // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);	   // RG_XPON_CDR_PR_XFICK_EN
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);	   // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);	   // RG_XPON_RX_PHY_CK_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);	   // RG_XPON_RX_PHYCK_RSTB
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0xB);		   // RG_XPON_RX_PHYCK_DIV
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x1);		   // RG_XPON_CDR_LPF_RATIO
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0);		   // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0);		   // RG_XPON_RX_BUSBIT_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x1);  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0); 	   // rg_xpon_rx_rate_ctrl (8bit)
  }
  else  //DS(RX)_1.25G      /  US(TX)_1.25G
  {	  

	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_REV_0, 15,0 , 0x18B0); 		   // RG_XPON_RX_REV_0
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, 0x3); // rg_force_da_pxp_rx_fe_gain_ctrl	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x300);         //rg_dig_reserve_0  
	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 1,0 , 0x3);			   // rg_force_da_XPON_RX_FE_GAIN_CTRL[1:0]	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 0,0 , 0x0 );			   // rg_disb_da_XPON_RX_FE_GAIN_CTRL
    
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_BETA_DAC, 6,0 , 0x8);	   // RG_XPON_CDR_PR_BETA_DAC
		
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 9,8 , 0x1); 		   // RG_XPON_RX_PHYCK_SEL
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 2,2 , 0x1);	   // RG_XPON_CDR_PR_XFICK_EN
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 24,24 , 0x1);      // RG_XPON_RX_PHY_CK_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 16,16 , 0x0);	   // RG_XPON_RX_PHY_CK_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 16,16 , 0x1);	   // RG_XPON_RX_PHYCK_RSTB
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_PHYCK_DIV, 7,0 , 0x29);		   // RG_XPON_RX_PHYCK_DIV
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1,0 , 0x3);		   // RG_XPON_CDR_LPF_RATIO
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 8,8 , 0x0);		   // RG_XPON_RX_BUSBIT_SEL_FORCE
	IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_BUSBIT_SEL, 0,0 , 0x0);		   // RG_XPON_RX_BUSBIT_SEL
	  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 24,24 , 0x1);  // rg_force_sel_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_aeq_speed, 17,16 , 0x3);  // rg_force_da_pxp_rx_osr_sel
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RG_XPON_RX_RESERVED_1, 1,0 , 0x0); 	   // rg_xpon_rx_rate_ctrl (8bit)
  }

  
 #if LAB_PRINT_XFI
   printk("=== XFI_RX_done ===\n");
#endif   
}


void XFI_ANA(uint xfi_rxana)
{
  //setting_XFI_ANA
  //XFI, TDC 
  //IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_PLL_MONCLK_SEL, 24,24 , 0x1);        // RG_XPON_TDC_AUTOEN 
  
  
  //TXPLL_bring_up
  //TXPLL_VCO_LDO_Out                                 
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_PERIOD, 25,24 , 0x1);      // RG_XPON_TXPLL_LDO_VCO_OUT
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_PERIOD, 17,16 , 0x1);      // RG_XPON_TXPLL_LDO_OUT
                                                                            
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VTP_EN, 10,8 , 0x0);           // RG_XPON_TXPLL_VTP
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VTP_EN, 0,0 , 0x1);            // RG_XPON_TXPLL_VTP_EN
                                                                            
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TDC_SYNC_CK_SEL, 17,16 , 0x1);       // RG_XPON_PLL_LDO_CKDRV_VSEL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TDC_SYNC_CK_SEL, 8,8 , 0x1);         // RG_XPON_PLL_LDO_CKDRV_EN


  //TXPLL_RSTB							   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 24,24 , 0x1);  // RG_XPON_TXPLL_PLL_RSTB
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 18,16 , 0x4);  // RG_XPON_TXPLL_RST_DLY
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 0,0 , 0x0);    // RG_XPON_TXPLL_REFIN_INTERNAL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_REFIN_INTERNAL, 9,8 , 0x0);    // RG_XPON_TXPLL_REFIN_DIV


  //TXPLL_SDM
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 25,24 , 0x0);       // RG_XPON_TXPLL_SDM_MODE

  if((xfi_rxana == XFI_10p3125G_BYP_JCPLL)||(xfi_rxana == XFI_6p25G_BYP_JCPLL)) //DS(RX)_10.31252G  /  US(TX)_10.3125G & //DS(RX)_5.15625G   /  US(TX)_5.15625G
  {                                   
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 16,16 , 0x1);       // RG_XPON_TXPLL_SDM_IFM
    IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 9,8 , 0x0);		  // RG_XPON_TXPLL_SDM_DI_LS

      if(xfi_rxana == XFI_6p25G_BYP_JCPLL) 
      {          
		  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x0);         // RG_XPON_TXPLL_SDM_DI_EN
	  }
	  else
	  {		  
		  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x1);         // RG_XPON_TXPLL_SDM_DI_EN		  
	  }	  
  }
  else if((xfi_rxana == XFI_10G_BYP_JCPLL)||(xfi_rxana == XFI_12p5G_BYP_JCPLL)) 
  {						   
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 16,16 , 0x0);	   // RG_XPON_TXPLL_SDM_IFM	 
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x0); 		   // RG_XPON_TXPLL_SDM_DI_EN	 
     IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 9,8 , 0x0);		   // RG_XPON_TXPLL_SDM_DI_LS
  }
  else
  {
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 16,16 , 0x1);	   // RG_XPON_TXPLL_SDM_IFM	  					   
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 0,0 , 0x1); 		   // RG_XPON_TXPLL_SDM_DI_EN
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_DI_EN, 9,8 , 0x0);		   // RG_XPON_TXPLL_SDM_DI_LS
  }  

                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD, 8,8 , 0x0);           // RG_XPON_TXPLL_SDM_OUT
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD, 1,0 , 0x3);           // RG_XPON_TXPLL_SDM_ORD


  //TXPLL_SSC
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_DELTA1, 31,16 , 0x0);      // RG_XPON_TXPLL_SSC_DELTA
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_DELTA1, 15,0 , 0x0);       // RG_XPON_TXPLL_SSC_DELTA1

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_EN, 16,16 , 0x0);          // RG_XPON_TXPLL_SSC_TRI_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_EN, 8,8 , 0x0);            // RG_XPON_TXPLL_SSC_PHASE_INI
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_EN, 0,0 , 0x0);            // RG_XPON_TXPLL_SSC_EN
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SSC_PERIOD, 15,0 , 0x0);       // RG_XPON_TXPLL_SSC_PERIOD
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_DOUBLE_EN, 0,0 , 0x0);     // TXPLL_SPARE_L(ICHP_DOUBLE)


  //TXPLL_LPF
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 28,24 , 0x1F);      // RG_XPON_TXPLL_LPF_BC

  if((xfi_rxana == XFI_10p3125G_BYP_JCPLL)||(xfi_rxana == XFI_12p4G_BYP_JCPLL))
  {
     IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 20,16 , 0xA);       // RG_XPON_TXPLL_LPF_BR
  }
  else
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 20,16 , 0x5);		// RG_XPON_TXPLL_LPF_BR
  }
  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 13,8 , 0x0);        // RG_XPON_TXPLL_CHP_IOFST

  if(xfi_rxana == XFI_10G_BYP_JCPLL) 
  { 
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 5,0 , 0x31);        // RG_XPON_TXPLL_CHP_IBIAS
  }
  else if(xfi_rxana == XFI_12p5G_BYP_JCPLL) 
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 5,0 , 0x1E);		// RG_XPON_TXPLL_CHP_IBIAS
  }
  else
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_CHP_IBIAS, 5,0 , 0x18);		 // RG_XPON_TXPLL_CHP_IBIAS
  }
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP, 20,16 , 0x18);         // RG_XPON_TXPLL_LPF_BWC

  if((xfi_rxana == XFI_12p5G_BYP_JCPLL)|| (xfi_rxana == XFI_10G_BYP_JCPLL))
  { 
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP, 12,8 , 0xB);			// RG_XPON_TXPLL_LPF_BWR
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD, 16,16 , 0x0); 		// RG_XPON_TXPLL_SDM_HREN	
  }
  else
  {
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP, 12,8 , 0x16);           // RG_XPON_TXPLL_LPF_BWR
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD, 16,16 , 0x1); 		// RG_XPON_TXPLL_SDM_HREN	
  }
  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP, 4,0 , 0x2);            // RG_XPON_TXPLL_LPF_BP


  //TXPLL_VCO
  if(xfi_rxana == XFI_10p3125G_BYP_JCPLL) //DS(RX)_10.31252G  /  US(TX)_10.3125G 
  { 
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x2);      // RG_XPON_TXPLL_VCO_CFIX
  }
  else if(xfi_rxana == XFI_10G_BYP_JCPLL)
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x3); 	 // RG_XPON_TXPLL_VCO_CFIX
  }
  else
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 25,24 , 0x0);    // RG_XPON_TXPLL_VCO_CFIX
  }
  	
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 0,0 , 0x1);    // RG_XPON_TXPLL_VCO_HALFLSB_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 10,8 , 0x7);   // RG_XPON_TXPLL_VCO_SCAPWR
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 26,24 , 0x4);  // RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 18,16 , 0x4);  // RG_XPON_TXPLL_VCO_TCLVAR
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_VCO_HALFLSB_EN, 29,27 , 0x0);  // RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L

  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_CODE, 25,24 , 0x3);      // RG_XPON_TXPLL_KBAND_KF
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_CODE, 17,16 , 0x0);      // RG_XPON_TXPLL_KBAND_KFC
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_CODE, 10,8 , 0x2);       // RG_XPON_TXPLL_KBAND_DIV
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_KS, 1,0 , 0x1);          // RG_XPON_TXPLL_KBAND_KS
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_CODE, 7,0 , 0xE4);       // RG_XPON_TXPLL_KBAND_CODE
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_LPF_BP, 24,24 , 0x0);          // RG_XPON_TXPLL_KBAND_OPTION
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 24,24 , 0x0);  // RG_XPON_TXPLL_VCO_KBAND_MEAS_EN                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_kband_load_en, 8,8 , 0x1);        // rg_force_sel_da_pxp_txpll_kband_load_en 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_kband_load_en, 0,0 , 0x0);        // rg_force_da_pxp_txpll_kband_load_en
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_KS, 17,16 , 0x0);        // RG_XPON_TXPLL_MMD_PREDIV_MODE
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_KBAND_KS, 8,8 , 0x1);          // RG_XPON_TXPLL_POSTDIV_EN


  //TXPLL_TCL
  if(xfi_rxana == XFI_12p5G_BYP_JCPLL)
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 12,8 , 0xE); 	// RG_XPON_TXPLL_TCL_AMP_VREF  
  }
  else
  {
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 12,8 , 0xB); 	// RG_XPON_TXPLL_TCL_AMP_VREF	
  }
  
  if((xfi_rxana == XFI_12p4G_BYP_JCPLL)||(xfi_rxana == XFI_12p5G_BYP_JCPLL))
  { 	  
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 17,16 , 0x1);  // RG_PXP_TXPLL_POSTDIV_D256_EN  
	  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 17,16 , 0x1);      // RG_XPON_TXPLL_VCODIV                     
  }
  else 
  {
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 17,16 , 0x0);      // RG_XPON_TXPLL_VCODIV      
  }


  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 4,0 , 0xF);    // RG_XPON_TXPLL_TCL_KBAND_VREF      
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_AMP_GAIN, 2,0 , 0x3);      // RG_XPON_TXPLL_TCL_AMP_GAIN
  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 10,8 , 0x0);       // RG_XPON_TXPLL_TCL_LPF_BW
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_LPF_EN, 0,0 , 0x1);        // RG_XPON_TXPLL_TCL_LPF_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_SDM_ORD, 24,24 , 0x1);         // RG_XPON_TXPLL_TCL_AMP_EN


  //TX_TERMCAL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TX_TXLBRX_EN, 18,16 , 0x2);          // RG_XPON_TX_TERMCAL_VREF_H
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TX_TXLBRX_EN, 26,24 , 0x2);          // RG_XPON_TX_TERMCAL_VREF_L


  //XPON_RX
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_FE_EQ_HZEN, 24,24 , 0x1);         // RG_PXP_RX_FE_VB_EQ3_EN  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_FE_EQ_HZEN, 16,16 , 0x1);         // RG_PXP_RX_FE_VB_EQ2_EN  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_FE_EQ_HZEN, 8,8 , 0x1);           // RG_PXP_RX_FE_VB_EQ1_EN  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_FE_EQ_HZEN, 0,0 , 0x0);           // RG_PXP_RX_FE_EQ_HZEN 
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_FE_VCM_GEN_PWDB, 0,0 , 0x1);      // RG_XPON_RX_FE_VCM_GEN_PWDB
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 26,8 , 0x8000);       // RG_XPON_CDR_LPF_TOP_LIM
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_BOT_LIM, 18,0 , 0x78000);    // RG_XPON_CDR_LPF_BOT_LIM
                                   
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_CKREF_DIV, 16,16 , 0x0);      // RG_XPON_CDR_PR_RSTB_BYPASS
  //IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_DAC_RANGE, 9,8 , 0x0);            // RG_XPON_RX_DAC_RANGE_EYE

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_DAC_RANGE, 9, 8, 0x02); //RG_PXP_RX_DAC_RANGE_EYE , full_eye


 #if LAB_PRINT_XFI
   printk("=== XFI_ANA_done ===\n");
#endif   

}


void XFI_TXPLL_on(void)
{
  //seq_XFI_TXPLL_on
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 24,24 , 0x1); // rg_force_sel_da_pxp_txpll_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 16,16 , 0x1); // rg_force_da_pxp_txpll_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_LCPLL_PWCTL_SETTING_0, 24,24 , 0x1);          // rg_sw_lcpll_en
  
  udelay(6);

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 8,8 , 0x1);   // rg_force_sel_da_pxp_txpll_ckout_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_txpll_ckout_en, 0,0 , 0x1);   // rg_force_da_pxp_txpll_ckout_en

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 16,16 , 0x1);   // RG_XPON_TXPLL_FREQ_MEAS_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 8,8 , 0x0);     // RG_XPON_TXPLL_VREF_SEL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_PHY_CK1_EN, 8,8 , 0x1);         // RG_XPON_TXPLL_PHY_CK2_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_PHY_CK1_EN, 0,0 , 0x1);         // RG_XPON_TXPLL_PHY_CK1_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_TCL_KBAND_VREF, 16,16 , 0x0);   // RG_XPON_TXPLL_FREQ_MEAS_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_TXPLL_IB_EXT_EN, 24,24 , 0x0);        // RG_XPON_TXPLL_IB_EXT_EN

  udelay(500);

 #if LAB_PRINT_XFI
   printk("=== XFI_TXPLL_ON_done ===\n");
#endif 

}


void XFI_TX_on(void)
{
  //seq_XFI_TX_on
  //controlled by DA_TX_CK_EN or DA_TX_HSDATA_EN
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_RST_B, 0,0 , 0x1);                         // tx_top_rst_b


  //CLK PATH EN 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_CLKPATH_RST_0, 8,8 , 0x1);                // rg_clkpath_rstb_ck
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_CLKPATH_RST_0, 0,0 , 0x1);                // rg_clkpath_rst_en


  //TX CAL RST
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_RST_B, 8,8 , 0x1);                         // txcalib_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_RST_B, 0,0 , 0x1);                         // tx_top_rst_b

  udelay(100);

  //********************************************OPTIMIZE TX IMPEDANCE by EFUSE*****************************************************//
    XFI_tx_term_sel = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XFI_RX_FE_50OHMS_SEL_LSB, 2) : 0xF;
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_CALIB_0, 16, 16, 0x1);	//force P enable,

	if ((XFI_tx_term_sel > 0) && (XFI_tx_term_sel <= 3)){
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_CALIB_0, 25, 24, XFI_tx_term_sel);

	 #if LAB_PRINT_XFI
	   printk("DA_XFI_TX_TERMP_SEL[1:0]=%x\n", XFI_tx_term_sel);
	 #endif 
	}
	else {
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_TX_CALIB_0, 25, 24, 0x1);

	 #if LAB_PRINT_XFI
	   printk("Set TXP Impedance Level 1 as default! \n");
	 #endif 
	}

  //*****************************************************************************************************************************//

#if LAB_PRINT_XFI
   printk("=== XFI_TX_ON_done ===\n");
#endif 
}


void XFI_RX_preset(uint xfi_preset)
{
  //seq_XFI_RX_preset
  // ** RX_precondition
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 9,8 , 0x2);         // RG_XPON_RX_SIGDET_PEAK
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 20,16 , 0x2);       // RG_XPON_RX_SIGDET_VTH_SEL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_DAC_RANGE, 25,24 , 0x3);          // RG_XPON_RX_SIGDET_LPF_CTRL

  if((xfi_preset == RX_Spd_USXGMII)||(xfi_preset == RX_Spd_5GBaseR)||(xfi_preset == RX_Spd_SGMII)) //DS(RX)_10.31252G  /  US(TX)_10.3125G 
  {								   
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 19,19 , 0x0);       // RG_XPON_CDR_PR_CAP_EN
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 18,16 , 0x7);       // RG_XPON_CDR_PR_BUF_IN_SR
  }
  else
  {
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 19,19 , 0x1);       // RG_XPON_CDR_PR_CAP_EN
      IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_MONPR_EN, 18,16 , 0x6);       // RG_XPON_CDR_PR_BUF_IN_SR
  }

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0);   // rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0);    // rg_disb_rx_os_rdy


  //L2R
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1); // rg_force_sel_da_pxp_cdr_lpf_lck2data
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x0); // rg_force_da_pxp_cdr_lpf_lck2data


  //LEQ setting
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 24,24 , 0x1);       // rg_force_sel_da_pxp_rx_fe_peaking_ctrl
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 19,16 , 0x0);       // rg_force_da_pxp_rx_fe_peaking_ctrl


  //keep EYE reset
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 8,8 , 0x1);                  // rg_force_eye_reset_plu_o
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 8,8 , 0x0);                   // rg_disb_eye_reset_plu_o
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 16,16 , 0x0);                // rg_force_eye_top_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 16,16 , 0x0);                 // rg_disb_eye_top_en


  //keep BLWC reset
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 0,0 , 0x0);                   // rg_disb_blwc_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 0,0 , 0x0);                  // rg_force_blwc_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16,16 , 0x1);   // rg_disb_rx_blwc_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 16,16 , 0x1);  // rg_force_rx_blwc_en

  #if LAB_PRINT_XFI
   printk("=== XFI_RX_preset_done ===\n");
#endif 
}


void XFI_RX_on(uint xfi_rxon)
{
  //seq_XFI_RX_on
  //RX_on //
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 24,24 , 0x1);  // rg_force_sel_da_pxp_cdr_pr_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16,16 , 0x1);  // rg_force_da_pxp_cdr_pr_pwdb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8,8 , 0x1);    // rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0,0 , 0x1);    // rg_force_da_pxp_cdr_pr_pieye_pwdb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb, 24,24 , 0x0);        // rg_force_sel_da_pxp_cdr_pr_kband_rstb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb, 16,16 , 0x0);        // rg_force_da_pxp_cdr_pr_kband_rstb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb, 8,8 , 0x1);          // rg_force_sel_da_pxp_cdr_pd_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb, 0,0 , 0x1);          // rg_force_da_pxp_cdr_pd_pwdb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 8,8 , 0x1);           // rg_force_sel_da_pxp_rx_fe_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 0,0 , 0x1);           // rg_force_da_pxp_rx_fe_pwdb


  //RX_SigDet_Pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b, 24,24 , 0x1);      // rg_force_sel_da_pxp_rx_sigdet_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b, 16,16 , 0x1);      // rg_force_da_pxp_rx_sigdet_pwdb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b, 8,8 , 0x0);        // rg_force_sel_da_pxp_rx_scan_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b, 0,0 , 0x0);        // rg_force_da_pxp_rx_scan_rst_b

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1);   // rg_force_sel_da_pxp_cdr_lpf_rstb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1);   // rg_force_da_pxp_cdr_lpf_rstb

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 24,24 , 0x1);                     // rg_da_xpon_cdr_pd_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 16,16 , 0x1);                     // rg_da_xpon_cdr_pr_pieye_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 8,8 , 0x1);                       // rg_da_xpon_cdr_pr_pwdb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 0,0 , 0x1);                       // rg_da_xpon_rx_fe_pwdbb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_1, 0,0 , 0x1);                       //rg_da_xpon_rx_sigdet_pwdb 
 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RX_SYS_EN_SEL_0, 1,0 , 0x1);                  // rg_da_rx_sys_en_sel

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 25,24 , 0x0);      //RG_XPON_CDR_PR_FBKSEL  
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 10,8 , 0x5);       //RG_XPON_CDR_PR_VREG_CKBUF_VAL
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_VREG_IBAND_VAL, 2,0 , 0x5);        //RG_XPON_CDR_PR_VREG_IBAND_VAL

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8,8 , 0x0);         // rg_disb_rx_pical_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16,16 , 0x0);       // rg_disb_rx_pdos_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24,24 , 0x0);       // rg_disb_rx_feos_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0,0 , 0x0);         // rg_disb_rx_sdcal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0);         // rg_disb_rx_os_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 16,16 , 0x0);       // rg_disb_rx_blwc_en

  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_CKREF_DIV, 1,0 , 0x0);             // RG_XPON_CDR_PR_CKREF_DIV
  IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_PR_TDC_REF_SEL, 25,24 , 0x0);         // RG_XPON_CDR_PR_CKREF_DIV1


  //RX_RSTB
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x1);                           // rg_sw_rx_rst_n
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x1);                           // rg_sw_ref_rst_n

  if((xfi_rxon != RX_Spd_Asym_OLT)||(xfi_rxon != RX_Spd_Sym_OLT))
  {
       //CDR_LPF_RSTB 
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0);   // rg_force_da_pxp_cdr_lpf_rstb
	   udelay(100);	   
       IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1);   // rg_force_da_pxp_cdr_lpf_rstb
  }


  #if LAB_PRINT_XFI
   printk("=== XFI_RX_on_done ===\n");
#endif 

}


void XFI_RX_L2R(uint spd)
{
    //seq_XFI_RX_L2R
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x0);     //rg_force_da_pxp_cdr_lpf_lck2data
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1);     //rg_force_sel_da_pxp_cdr_lpf_lck2data

    udelay(100);
	
	if((spd != OLT_GPON_ASYM_MODE)||(spd != OLT_GPON_SYM_MODE)||(spd != OLT_GPON_SYM_MODE_7552)||(spd != OLT_GPON_MIX_MODE)||(spd != OLT_MODE_DEFAULT))
    {

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1);   //rg_force_sel_da_pxp_cdr_lpf_rstb 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0);   //rg_force_da_pxp_cdr_lpf_rstb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1);   //rg_force_da_pxp_cdr_lpf_rstb   
	}

    #if LAB_PRINT_XFI
   printk("=== XFI_RX_L2R_done ===\n");
#endif 
}

void XFI_RX_L2D(void)
{
	//L2D 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0,0 , 0x1 );    //rg_force_da_pxp_cdr_lpf_lck2data
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8,8 , 0x1 );    //rg_force_sel_da_pxp_cdr_lpf_lck2data  	
	udelay(200);

	//seq_XFI_RX_L2D
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24,24 , 0x1 );  //rg_force_sel_da_pxp_cdr_lpf_rstb  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x0 );  //rg_force_da_pxp_cdr_lpf_rstb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16,16 , 0x1 );  //rg_force_da_pxp_cdr_lpf_rstb
    
}


void XFI_EO_Scan(uint bit_sel, uint EO_start, uint EO_end , uint XFI_Eye_Scan)
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

	
    if(XFI_Eye_Scan) printk("bit_sel => %x \n" ,bit_sel);
     
if(bit_sel != EO_OLT_GPON_ASYM_MODE || bit_sel != EO_OLT_GPON_SYM_MODE || bit_sel != EO_OLT_GPON_SYM_MODE_7552 || bit_sel != EO_OLT_GPON_MIX_MODE || bit_sel != EO_OLT_MODE_DEFAULT)	 
{

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01); //rg_force_da_pxp_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
   

    XFI_eye_setting(bit_sel);

    //EYE setting, EYE OPEN related  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);  //rg_eq_en_delay
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PI_CAL, 10, 8, 0x04);     //rg_kpgain
	
	
    if(bit_sel == EO_Eth_Ser_USXGMII)	 
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

		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);		//rg_force_sel_da_pxp_rx_fe_gain_ctrl
		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, leq_gain);	//rg_force_da_pxp_rx_fe_gain_ctrl

		leq_peaking = 0;   
	
        for (j = sweep_start; j < sweep_end + 1; j++)
        {   
	        IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);      //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	        IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, leq_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl
	       udelay(500);
	  
              
        
            if(XFI_Eye_Scan)printk("leq_gain => %x , leq_peaking => %x\n" ,leq_gain , leq_peaking);
             
	 
            XFI_eye_Cal();             
            XFI_eye_EO(bit_sel , &fom_num , &veo , XFI_Eye_Scan);	

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
		        if (veo > best_veo)
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
   	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 8, 8, 0x01);       //rg_force_sel_da_pxp_rx_fe_gain_ctrl
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0, best_gain);  //rg_force_da_pxp_rx_fe_gain_ctrl

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 24, 24, 0x01);        //rg_force_sel_da_pxp_rx_fe_peaking_ctrl
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17, best_peaking);  //rg_force_da_pxp_rx_fe_peaking_ctrl

       
    
     if(XFI_Eye_Scan)printk("XFI_best_leq_gain => %x , XFI_best_leq_peaking => %x\n" ,best_gain , best_peaking);
 	
  }
}


EXPORT_SYMBOL(XFI_EO_Scan);


void XFI_eye_setting(uint bit_set)
{
    if (bit_set == EO_Eth_Ser_5GBaseR)
    {         
	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x01);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x180);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x280);   // rg_x_max
	   
    }
    else if (bit_set == EO_Eth_Ser_HSGMII)
    {  

	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x01);           // RG_PXP_CDR_LPF_RATIO	   
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x140);    // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x2C0);   // rg_x_max
    }
    else if (bit_set == EO_Eth_Ser_SGMII)
    {

	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x03);           // RG_PXP_CDR_LPF_RATIO	   	  
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);      // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x0);      // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x400);   // rg_x_max	
		
    }   
    else
    {  
	   IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_CDR_LPF_RATIO, 1, 0, 0x00);           // RG_PXP_CDR_LPF_RATIO
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 31, 24, 0xFF);	  // rg_eye_mask
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 10, 0, 0x1C0);   // rg_x_min
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0, 26, 16, 0x234);  // rg_x_max	
    }

	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0xF8);   //rg_cntlen
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 16, 16, 0x0);  //rg_cntforever
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2, 8, 8, 0x0);    //rg_data_shift 	 

	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1, 16, 16, 0x00);   //rg_index_mode
	   //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0xFFF8);  //rg_eyedur	 
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C);  //rg_eyedur	 

	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 0, 0, 0x0);    //rg_eye_nextpts_sel
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 8, 8, 0x0);    //rg_eye_nextpts_toggle
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3, 16, 16, 0x1);  //rg_eye_nextpts	 

	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 7, 0, 0x4);     //rg_eyecnt_hth
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0, 15, 8, 0x4);    //rg_eyecnt_vth
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 10, 0, 0x4);    //rg_eo_hth
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1, 23, 16, 0x4);   //rg_eo_vth

	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_1, 16, 16, 0x0);   //rg_heo_emphasis
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_1, 0, 0, 0x0);     //rg_a_lgain
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_1, 8, 8, 0x0);     //rg_a_mgain	   
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_2, 1, 0, 0x1); 	   //rg_a_sel
	   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_1, 24, 24, 0x0);   //rg_b_zero_sel
	 
	 
}


void XFI_eye_Cal(void)
{

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, 0x0); 	 //rg_force_da_pxp_cdr_pr_pieye
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x0);     //rg_force_sel_da_pxp_cdr_pr_pieye
    
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 22, 16, 0x0);   //rg_force_da_pxp_rx_dac_eye	 	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 24, 24, 0x0);   //rg_force_sel_da_pxp_rx_dac_eye
	
	
	 
    //pical redo
    //reset block
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 7, 0, 0x80);   //rg_eq_en_delay
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PI_CAL, 10, 8, 0x01);      //rg_kpgain
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 16, 16, 0x00);    //rg_eq_pi_cal_rst_b

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_6, 8, 8, 0x00);  //rg_disb_rx_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 8, 8, 0x00); //rg_force_rx_and_pical_rstb

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_6, 0, 0, 0x00);  //rg_disb_ref_and_pical_rstb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 0, 0, 0x00); //rg_force_ref_and_pical_rstb

		
    //enable	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_5, 24, 24, 0x00);               //rg_disb_rx_or_pical_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);                //rg_force_rx_or_pical_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8, 8, 0x00);   //rg_disb_rx_pical_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8, 8, 0x00);  //rg_force_rx_pical_en
   
		
    //release reset
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 16, 16, 0x01);     //rg_eq_pi_cal_rst_b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 8, 8, 0x01);  //rg_force_rx_and_pical_rstb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 0, 0, 0x01);  //rg_force_ref_and_pical_rstb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8, 8, 0x01);  //rg_force_rx_or_pical_en     
    udelay(1000); //delay for 1ms 

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8, 8, 0x00);   //rg_force_rx_or_pical_en      
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_3, 0, 0, 0x00);	 //rg_disb_eq_pi_cal_rdy
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_3, 0, 0, 0x01);	 //rg_force_eq_pi_cal_rdy   

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 0, 0, 0x1);   //rg_force_eyecnt_rdy 
	udelay(1000); //delay for 1ms 

}



 void XFI_eye_EO(uint bit_rate, volatile int *fom_num , volatile int *veo , uint XFI_Eye_Scan)
{
   //volatile unsigned int fom_num;
   volatile int R_fom_num , R_veo;


    //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0, 9, 0, 0x0A);     //rg_cntlen  
    //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2, 19, 0, 0x44C); //rg_eyedur     

    //reset eye_top   
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 8, 8, 0x00);     //rg_disb_eye_reset_plu_o 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);    //rg_force_eye_reset_plu_o  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x00);    //rg_force_eye_reset_plu_o  

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 16, 16, 0x00);   //rg_disb_eye_top_en 
	 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);  //rg_force_eye_top_en 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x01);  //rg_force_eye_top_en    

     
   if (bit_rate == EO_Eth_Ser_USXGMII)
   {
       udelay(5500); //delay for 5.5ms        
   }
   else
   {	   
	   mdelay(55); //delay for 55ms  
   }

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 8, 8, 0x01);     //rg_disb_da_xpon_rx_dac_eye  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 24, 24, 0x01);   //rg_disb_da_xpon_cdr_pr_pieye  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01);   //rg_disb_eyedur_init_b  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 24, 24, 0x01);           //rg_disb_eyecnt_rx_rst_b  
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01);   //rg_disb_eyedur_en
   

   //read out EO results
   XFI_readout_EO(0 , &R_fom_num , &R_veo , XFI_Eye_Scan);

   //clear EXE_X_SW offset value
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 8, 8, 0x01);     //rg_force_eye_reset_plu_o  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 16, 16, 0x00);    //rg_disb_eye_top_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 16, 16, 0x00);   //rg_disb_eye_top_en  
   

   //reset pi_cal_rdy
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_3, 0, 0, 0x00);     //rg_disb_eq_pi_cal_rdy  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_3, 0, 0, 0x00);    //rg_force_eq_pi_cal_rdy  

   //reset eyecnt_rdy
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_5, 16, 16, 0x0);  //rg_disb_eyecnt_rdy , Morris_Test
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 0, 0, 0x0);   //rg_force_eyecnt_rdy , Morris_Test
      

   *fom_num = R_fom_num;
   *veo = R_veo;
}



void XFI_readout_EO(int sweep_direction , volatile int *R_fom_num , volatile int *R_veo , uint XFI_Eye_Scan)
{
   volatile unsigned int eye_er, eye_el, eye_eu, eye_eb, i = 0;
   volatile unsigned int dac_d0, dac_d1, dac_e0, dac_e1, dac_eye, feos;
   volatile	unsigned int eye_x_done, eye_y_done, heo_rdy, veo_rdy;
   //volatile unsigned int veo;   
   volatile int fom_num,heo, veo,heo_tmp ,veo_tmp , eye_er_tmp , eye_el_tmp , eye_eu_tmp , eye_eb_tmp;
   volatile unsigned int pical_data_out;   
   volatile unsigned int EO_Buf[10]= {0};
   


   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 0, 0, 0x01);	  //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 8, 8, 0x01);	  //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 16, 16, 0x01);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 24, 24, 0x01);  //ro_lnx_sw_fll_ro_4_latch_en  
   
   udelay(50);

   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 0, 0, 0x00);    //ro_lnx_sw_fll_ro_1_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 8, 8, 0x00);    //ro_lnx_sw_fll_ro_2_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 16, 16, 0x00);  //ro_lnx_sw_fll_ro_3_latch_en  
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FLL_6, 24, 24, 0x00);  //ro_lnx_sw_fll_ro_4_latch_en  

   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
   udelay(100);
   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle  
	

	pical_data_out = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_2, 22, 16); //ro_pi_cal_data_out
	eye_x_done = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_9, 16, 16);     //eo_x_done
	eye_y_done = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_9, 24, 24);     //eo_y_done
	heo_rdy = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_5, 16, 16);        //heo_rdy
	veo_rdy = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_5, 24, 24);        //veo_rdy
	
   
  if ((eye_x_done == 1) && (eye_y_done == 1))
  {	   
	  
	  if(XFI_Eye_Scan){
		  printk("\n" );	    
		  printk("X&Y index scan done\n");
	  	}

	if ((heo_rdy == 1) && (veo_rdy == 1))
	  {		
	       
         if(XFI_Eye_Scan){
	         printk("\n" );			 
		     printk("heo&veo result ready\n");
		}

	   }
	 }
	else
	   {   
		if(XFI_Eye_Scan) printk("X&Y index scan NOT done\n");
	   }    

	 eye_el = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_10, 26, 16); //eye_el	 
	 eye_er = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_10, 10, 0);  //eye_er
	 eye_eu = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_11, 6, 0);   //eye_eu	 
	 eye_eb = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_11, 14, 8);  //eye_eb
	 

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

	 dac_d0 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_1, 14, 8);  //ro_rx_dac_d0
	 dac_d1 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_1, 22, 16); //ro_rx_dac_d1
	 dac_e0 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_1, 30, 24); //ro_rx_dac_e0
	 dac_e1 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_2, 6, 0);   //ro_rx_dac_e1
	 dac_eye = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_1, 6, 0);  //ro_rx_dac_eye	 
	 feos    = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_2, 13, 8);  //ro_rx_feos_out
	   
	 fom_num = heo;   
    
	 //print out these results to textbox
   if (sweep_direction == 0)
      {	
		if(XFI_Eye_Scan) printk("Y index sweep direction: bottom-up\n");
	  }
   else
	  {
	    if(XFI_Eye_Scan) printk("Y index sweep direction: upside-down\n");
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
	

if(XFI_Eye_Scan){
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
      if(XFI_Eye_Scan) printk("EO result isn't valid.\n");
   
	 IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_0, 24, 24, 0x00);    //rg_disb_da_xpon_cdr_lpf_rstb  
	 IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 24, 24, 0x00);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	 udelay(500); //delay for 500us 	  

	 IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_0, 24, 24, 0x01);    //rg_force_da_xpon_cdr_lpf_rstb  	  
	  udelay(500); //delay for 500us 
	 }
   //return fom_num;
   *R_fom_num = fom_num;
   *R_veo = veo;
   
}	   



void XFI_phy_eyescan_test(uint bit_sel, int start_p, int sweep_r)

{
	uint pical_data_out=0;
    uint ro_dac_eye , eye_offset=0;
    int EYE_X_FW, EYE_Y_FW=0;
    uint EYE_X_HW, EYE_Y_HW=0;            
    uint eyecnt = 0;
    int i,j,k=0;           

    int Start_Point = start_p;
    int Sweep_Range = sweep_r;
	//int Ovr = Ovr_sel; 	//need to modify for different RX Rate
	int Ovr = 1;

	printk("fe_gain => %x\n",IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_gain_ctrl, 1, 0)); 
	printk("fe_peaking => %x\n",IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_da_pxp_jcpll_sdm_scan, 19, 17)); 

	
    printk("Start_Point = %d	Sweep_Range = %d	Ovr = %d \n", Start_Point, Sweep_Range, Ovr);

	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0, 0, 0x01); //rg_force_da_pxp_cdr_pr_pieye_pwdb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 8, 8, 0x01); //rg_force_sel_da_pxp_cdr_pr_pieye_pwdb
    
    XFI_eye_setting(bit_sel);
    
    //reset_eye_control	
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x00); //rg_disb_eyedur_init_b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x00); //rg_force_eyedur_init_b
    
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 24, 24, 0x00);  //rg_disb_eyecnt_rx_rst_b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 24, 24, 0x00); //rg_force_eyecnt_rx_rst_b
    
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x01); //rg_disb_eyedur_init_b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x01); //rg_force_eyedur_init_b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x01); //rg_disb_eyedur_en		
    	
    XFI_eye_Cal();
    				   
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
       udelay(100);
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle        
    
    ro_dac_eye = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_RO_RX2ANA_1, 6, 0);  //ro_rx_dac_eye
    pical_data_out = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_2, 22, 16); //ro_pi_cal_data_out
		
	

	ro_dac_eye = eye_offset + ro_dac_eye;
	
	
	EYE_X_HW = pical_data_out;    
	EYE_X_FW = EYE_X_HW;

	EYE_Y_HW = 64;	
	EYE_Y_FW = -64; 
	
	printk("pical_data_out = %d \n", pical_data_out);		 
				   	
	for (i = 0; i < Start_Point*Ovr; i++){
			
		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_HW); //rg_force_da_pxp_cdr_pr_pieye
		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);      //rg_force_sel_da_pxp_cdr_pr_pieye					  
		
		
		EYE_X_HW--;
		EYE_X_FW--;			
	}	

	printk("eyecnt		EYE_X_FW		EYE_Y_FW \n");
	//snack sequence sweep full eye scan																							  
	for (k = 0; k < (Sweep_Range/2) ; k++)																							  
	{																																  
		for (i = 0; i < 65; i++)																									  
		{																															  		
			eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);
				 	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++;

		}																															  
		EYE_Y_HW = 1;																												  
		EYE_Y_FW = 1;

		ssleep(1);
																																		  
		for (j = 0; j < 63; j++)																									  
		{																															  
			eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  
			EYE_Y_HW++; 																											  
			EYE_Y_FW++; 

		}			

		ssleep(1); 
		
			EYE_X_HW = XFI_eyescan_moveX(EYE_X_FW, EYE_Y_FW, bit_sel);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW--; 																												  
			EYE_Y_FW--; 																												  
																																		  
			for (j = 0; j < 63; j++)																									  
			{																															  
				eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 

			}																															  
			EYE_Y_HW = 128; 																											  
			EYE_Y_FW = 0;

			ssleep(1);
																																		  
			for (i = 0; i < 65; i++)																									  
			{																															  
				eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
				printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	  	  
				EYE_Y_HW--; 																											  
				EYE_Y_FW--; 	

			}																															  
			EYE_X_HW = XFI_eyescan_moveX(EYE_X_FW, EYE_Y_FW,bit_sel);																					  
			EYE_X_FW = EYE_X_FW + Ovr;																									  
																																		  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 																												  
		}																																  
																																		  
		//last time bottom-up sweep Y index 																							  
		for (i = 0; i < 65; i++)																										  
		{																																  
			eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 

		}			
		EYE_Y_HW = 1;																													  
		EYE_Y_FW = 1;																													  
		for (j = 0; j < 63; j++)																										  
		{																																  
			eyecnt =XFI_eyescan_countPoint(EYE_X_HW, EYE_Y_HW, bit_sel);																				  
			printk("%d	%d %d \n", eyecnt, EYE_X_FW, EYE_Y_FW);	 				  
			EYE_Y_HW++; 																												  
			EYE_Y_FW++; 	

		}

}

EXPORT_SYMBOL(XFI_phy_eyescan_test);



int XFI_eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, uint bit_sel){
	
	int eyecnt = 0;																			
	int eyecnt_rdy = 0;																	
																								

    //Set the eyecnt count point (EYE_X_FW, EYE_Y_FW)
    // X index         
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_FW); //rg_force_da_pxp_cdr_pr_pieye
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);	   //rg_force_sel_da_pxp_cdr_pr_pieye		 
    
    // Y index
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 22, 16, EYE_Y_FW); //rg_force_da_pxp_rx_dac_eye
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_fll_cor, 24, 24, 0x1);	   //rg_force_sel_da_pxp_rx_dac_eye
	

    // EYE cnt enable 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8, 8, 0x0);	   //rg_disb_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x0);	//rg_force_eyedur_init_b		

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 24, 24, 0x0);	//rg_disb_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 24, 24, 0x0); //rg_force_eyecnt_rx_rst_b 

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0, 0, 0x0);	   //rg_disb_eyedur_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x0);	   //rg_force_eyedur_en
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 24, 24, 0x1); //rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24, 24, 0x1);	 //rg_force_eyedur_init_b

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16, 16, 0x1);	 //rg_force_eyedur_en
    mdelay(1);
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x00);    //rg_ro_toggle  	
	udelay(100);	    
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DEBUG_0, 24, 24, 0x01);    //rg_ro_toggle

    
    eyecnt_rdy = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_4, 24, 24);  //eyecnt_rdy
	
    if (eyecnt_rdy == 1)   // if eyecnt_rdy
    {             
        eyecnt = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_7, 19, 0);  //eyecnt
    }
    else
    {
        printk("eyecnt_rdy = %d \n", eyecnt_rdy);
    }  
     
    
     return eyecnt;																	
}



uint XFI_eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW,uint bit_sel){
	int i ;
	uint Ovr_sel = 1;
	
       for (i = 0; i < Ovr_sel; i++)
       {
           // X index
           IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 22, 16, EYE_X_HW); //rg_force_da_pxp_cdr_pr_pieye
		   IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_rate_ctrl, 24, 24, 0x1);      //rg_force_sel_da_pxp_cdr_pr_pieye               
           EYE_X_HW++;
       }   
   
    return EYE_X_HW;
}






void XFI_RX_OSCal(void)
{
    //FBCK_Lock
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_8, 0,0 , 0x0);                       // rg_disb_fbck_lock (0:force_mode / 1:normal_mode)
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_9, 0,0 , 0x1);                      // rg_force_fbck_lock

    //RX_OSCal_En
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24,24 , 0x1); // rg_force_sel_da_pxp_rx_oscal_ckon
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16,16 , 0x1); // rg_force_da_pxp_rx_oscal_ckon

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 24,24 , 0x1);        // rg_force_sel_da_pxp_rx_oscal_rstb
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 16,16 , 0x1);        // rg_force_da_pxp_rx_oscal_rstb

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1);          // rg_force_sel_da_pxp_rx_oscal_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x1);          // rg_force_da_pxp_rx_oscal_en

    udelay(200);
  
    //set_normal_or_force_mode
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0);         // rg_disb_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0);         // rg_disb_rx_os_rdy

    //disable_force_mode_signal
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0);        // rg_force_rx_os_en
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0);        // rg_force_rx_os_rdy

    //release_reset_Enable
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x1);        // rg_force_rx_os_en


#if LAB_PRINT_XFI
   printk("=== XFI_RX_OSCal_done ===\n");
#endif 
}


void XFI_RX_pical(void)
{
    //PICAL
    //pre-condition 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 24,24 , 0x1 );                    //rg_disb_da_xpon_cdr_pr_pieye  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PI_CAL, 10,8 , 0x4 );                          //rg_kpgain	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_PHY_EQ_CTRL_0, 7,0 , 0x8 );                       //rg_eq_en_delay

    //reset block 	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 16,16 , 0x0 );                        //rg_eq_pi_cal_rst_b
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 8,8 , 0x0 );                     //rg_force_rx_and_pical_rstb	 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_6, 8,8 , 0x0 );                      //rg_disb_rx_and_pical_rstb
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 0,0 , 0x0 );                     //rg_force_ref_and_pical_rstb	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_6, 0,0 , 0x0 );                      //rg_disb_ref_and_pical_rstb
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_3, 0,0 , 0x0 );                      //rg_disb_eq_pi_cal_rdy


	//enable
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8,8 , 0x0 );                     //rg_force_rx_or_pical_en	 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_5, 24,24 , 0x0 );                    //rg_disb_rx_or_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x0 );       //rg_force_rx_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 8,8 , 0x0 );        //rg_disb_rx_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_3, 0,0 , 0x0 );                     //rg_force_eq_pi_cal_rdy 


	//release reset & enable
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 16,16 , 0x1 );                        //rg_eq_pi_cal_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 8,8 , 0x1 );                     //rg_force_rx_and_pical_rstb	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 0,0 , 0x1 );                     //rg_force_ref_and_pical_rstb	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8,8 , 0x1 );                     //rg_force_rx_or_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x1 );       //rg_force_rx_pical_en 
	udelay(200);
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 8,8 , 0x0 );       //rg_force_rx_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_6, 8,8 , 0x0 );                     //rg_force_rx_or_pical_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_3, 0,0 , 0x1 );                     //rg_force_eq_pi_cal_rdy


	#if LAB_PRINT_XFI
   printk("=== XFI_RX_pical_done ===\n");
#endif 
}




void XFI_RX_pdos(void)
{
  
  //aux_XFI_RX_pdos
	//PDOS
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 24,24 , 0x1);		   // rg_force_sel_da_pxp_rx_pdoscal_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 16,16 , 0x1);		   // rg_force_da_pxp_rx_pdoscal_en
  
  
	//*** pre-condition
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0); 	   // rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0);		   // rg_disb_rx_os_rdy
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_1, 8,8 , 0x1);					   // rg_disb_da_xpon_rx_dac_d0
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_1, 16,16 , 0x1);					   // rg_disb_da_xpon_rx_dac_d1
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_1, 24,24 , 0x1);					   // rg_disb_da_xpon_rx_dac_e0
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 0,0 , 0x1);					   // rg_disb_da_xpon_rx_dac_e1
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 8,8 , 0x1);					   // rg_disb_da_xpon_rx_dac_eye
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 0,0 , 0x0);					   // rg_force_blwc_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 0,0 , 0x0);					   // rg_disb_blwc_rx_rst_b
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24,24 , 0x0);		   // rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8,8 , 0x0);			   // rg_disb_eyedur_init_b
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 24,24 , 0x0); 				   // rg_force_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 24,24 , 0x0);					   // rg_disb_eyecnt_rx_rst_b
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16,16 , 0x0);		   // rg_force_eyedur_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0,0 , 0x0);			   // rg_disb_eyedur_en
  
  
	// setting 
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_PDOS_CTRL_0, 18,16 , 0x2);					   // rg_sap_sel
  
	//seq
	//reset
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 16,16 , 0x0); 				   // rg_force_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_6, 16,16 , 0x0);					   // rg_disb_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 0,0 , 0x0);						   // rg_pdos_rst_b
  
	//disable  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x0);	   // rg_force_rx_pdos_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 16,16 , 0x0);	   // rg_disb_rx_pdos_en
  
	//release reset & enable
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0); 	   // rg_force_rx_os_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0);		   // rg_disb_rx_os_en
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_7, 16,16 , 0x1); 				   // rg_force_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 0,0 , 0x1);						   // rg_pdos_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x1);	   // rg_force_rx_pdos_en
	udelay(200);
  
	//disable
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 16,16 , 0x0);	   // rg_force_rx_pdos_en  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0); 	   // rg_force_rx_os_en
	
	//release eye related
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 24,24 , 0x0);		   // rg_force_eyedur_init_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 8,8 , 0x1);			   // rg_disb_eyedur_init_b
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 24,24 , 0x0); 				   // rg_force_eyecnt_rx_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 24,24 , 0x1);					   // rg_disb_eyecnt_rx_rst_b
  
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 16,16 , 0x0);		   // rg_force_eyedur_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1, 0,0 , 0x1);			   // rg_disb_eyedur_en
  
	//Disable_PDOS
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 24,24 , 0x1);		   // rg_force_sel_da_pxp_rx_pdoscal_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 16,16 , 0x0);		   // rg_force_da_pxp_rx_pdoscal_en


 #if LAB_PRINT_XFI
   printk("=== XFI_RX_pdos_done ===\n");
#endif 
}


void XFI_RX_feos(void)
{

  //FEOS
  //pre-condition
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x0);        // rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0);         // rg_disb_rx_os_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_2, 16,16 , 0x1);                     // rg_disb_da_xpon_rx_fe_vos

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 0,0 , 0x0);                      // rg_force_blwc_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 0,0 , 0x0);                       // rg_disb_blwc_rx_rst_b

  //setting
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FEOS, 7,0 , 0x30);                             // rg_lfsel

  //seq
  //reset
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 16,16 , 0x0);                    // rg_force_feos_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 16,16 , 0x0);                     // rg_disb_feos_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 8,8 , 0x0);                           // rg_feos_rst_b
  
  //disable
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x0);      // rg_force_rx_feos_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 24,24 , 0x0);       // rg_disb_rx_feos_en

  //release reset & enable
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x1);        // rg_force_rx_os_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0, 0,0 , 0x0);         // rg_disb_rx_os_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 16,16 , 0x1);                    // rg_force_feos_rx_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 8,8 , 0x1);                           // rg_feos_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x1);      // rg_force_rx_feos_en
  udelay(1000);
  //phy_delay1ms(1);

  //disable
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 24,24 , 0x0);      // rg_force_rx_feos_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0, 0,0 , 0x0);        // rg_force_rx_os_en


 #if LAB_PRINT_XFI
   printk("=== XFI_RX_feos_done ===\n");
#endif 

}


void XFI_RX_sdcal(void)
{

  //SIGDET_Calibration
  //pre-condition
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8,8 , 0x1);    // rg_force_sel_da_pxp_rx_sigdet_cal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0,0 , 0x1);    // rg_force_da_pxp_rx_sigdet_cal_en

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1);         // rg_force_sel_da_pxp_rx_oscal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x1);         // rg_force_da_pxp_rx_oscal_en

  //reset
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 24,24 , 0x0);                        // rg_cal_rst_b

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 8,8 , 0x0);                     // rg_force_sdcal_ref_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 0,0 , 0x0);        // rg_disb_rx_sdcal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_DISB_MODE_7, 8,8 , 0x0);                      // rg_disb_sdcal_ref_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x0);       // rg_disb_sdcal_ref_rst_b

  //release reset & enable
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_0, 24,24 , 0x1);                        // rg_cal_rst_b
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_FORCE_MODE_8, 8,8 , 0x1);                     // rg_force_sdcal_ref_rst_b

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x1);       // rg_force_rx_sdcal_en
  udelay(200); 

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 0,0 , 0x0);       // rg_force_rx_sdcal_en
  // RX_SIGDET_CAL_Dis
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 8,8 , 0x1);    // rg_force_sel_da_pxp_rx_sigdet_cal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_sigdet_cal_en, 0,0 , 0x0);    // rg_force_da_pxp_rx_sigdet_cal_en

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 24,24 , 0x1);  // rg_force_sel_da_pxp_rx_oscal_ckon
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_jcpll_sdm_scan_rstb, 16,16 , 0x0);  // rg_force_da_pxp_rx_oscal_ckon

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 24,24 , 0x1);       // rg_force_sel_da_pxp_rx_oscal_rstb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 16,16 , 0x0);       // rg_force_da_pxp_rx_oscal_rstb
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 8,8 , 0x1);         // rg_force_sel_da_pxp_rx_oscal_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_oscal_en, 0,0 , 0x0);         // rg_force_da_pxp_rx_oscal_en


 #if LAB_PRINT_XFI
   printk("=== XFI_RX_sdcal_done ===\n");
#endif 

}


void XFI_phy_status(void)
{
  //aux_XFI_phy_status
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 8,8 , 0x1);       // rg_force_rx_os_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 8,8 , 0x0);        // rg_disb_rx_os_rdy
  udelay(1);


 #if LAB_PRINT_XFI
   printk("=== XFI_phy_status_done ===\n");
#endif 

}


void XFI_DIG_reset_hold(uint spd)
{

	if((spd == OLT_GPON_ASYM_MODE)||(spd == OLT_GPON_SYM_MODE)||(spd == OLT_GPON_SYM_MODE_7552)||(spd == OLT_GPON_MIX_MODE)||(spd == OLT_MODE_DEFAULT))
    {
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 15,15 , 0x0);					   // rg_sw_ponolt_rxmac_rst_n		
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 14,14 , 0x0);					   // rg_sw_ponolt_rxpcs_rst_n
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 13,13 , 0x0);					   // rg_sw_ponolt_txmaclrst_n
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 12,12 , 0x0);					   // rg_sw_ponolt_txpcs_rst_n
    }

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x0);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 6,6 , 0x0);       // rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x0);       // rg_sw_ref_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 4,4 , 0x0);       // rg_sw_allpcs_rst_n 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 2,2 , 0x0);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x0);       // rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 3,3 , 0x0);       // rg_sw_pma_rst_n
	
    udelay(50);


#if LAB_PRINT_XFI
	  printk("=== XPON_DIG_reset_hold_done ===\n");
#endif
}


void XFI_DIG_ref_release(void)
{
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x1);	   // rg_sw_ref_rst_n	
	udelay(100);

	
#if LAB_PRINT_XFI
	  printk("=== XPON_DIG_ref_release_done ===\n");
#endif
}



void XFI_DIG_reset_release(uint spd)
{
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x1);	   // rg_sw_ref_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 2,2 , 0x1);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x1);       // rg_sw_rx_rst_n

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 3,3 , 0x1);	   // rg_sw_pma_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x1);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 6,6 , 0x1);       // rg_sw_tx_fifo_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 4,4 , 0x1);       // rg_sw_allpcs_rst_n  

    if((spd == OLT_GPON_ASYM_MODE)||(spd == OLT_GPON_SYM_MODE)||(spd == OLT_GPON_SYM_MODE_7552)||(spd == OLT_GPON_MIX_MODE)||(spd == OLT_MODE_DEFAULT))
    {
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 15,15 , 0x1);					   // rg_sw_ponolt_rxmac_rst_n
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 14,14 , 0x1);					   // rg_sw_ponolt_rxpcs_rst_n
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 13,13 , 0x1);					   // rg_sw_ponolt_txmaclrst_n
	     IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 12,12 , 0x1);					   // rg_sw_ponolt_txpcs_rst_n
    }
    udelay(100);


	#if LAB_PRINT_XFI
	  printk("=== XFI_DIG_reset_release_done ===\n");
#endif
}



void XFI_OLT_Eth_DIG_reset(void)
{
    //seq_XFI_OLT_DIG_reset
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x0);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 6,6 , 0x0);       // rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x0);       // rg_sw_ref_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 4,4 , 0x0);       // rg_sw_allpcs_rst_n 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 2,2 , 0x0);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x0);       // rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 3,3 , 0x0);       // rg_sw_pma_rst_n
    udelay(500);
   
    //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x1);       // rg_sw_ref_rst_n   
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 4,4 , 0x1);       // rg_sw_allpcs_rst_n    
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 3,3 , 0x1);       // rg_sw_pma_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x1);       // rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 6,6 , 0x1);       // rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 2,2 , 0x1);       // rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x1);       // rg_sw_rx_rst_n
    udelay(100);

 #if LAB_PRINT_XFI
   printk("=== XFI_OLT_DIG_reset_done ===\n");
#endif 

}




void XFI_OLT_RX_rxrdy(void)
{
  //seq_XFI_OLT_RX_rxrdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24,24 , 0x1);  //rg_force_rx_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24,24 , 0x0);   // rg_disb_rx_rdy 

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x0);                       // rg_sw_rx_fifo_rst_n         
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x1);                       // rg_sw_rx_fifo_rst_n

  //rx_gearbox_FIFO
  //IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_11, 2,0 , 0x5);               // rx_gearbox_FIFO

  udelay(500);

  //force on_rx_gearbox
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_24, 12,12 , 0x1);             // rg_force_rx_gearbox  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_24, 8,8 , 0x1);               // rg_force_sel_rx_gearbox

  //********************************************OPTIMIZE RX IMPEDANCE by EFUSE*****************************************************//
  
  XFI_rx_term_sel = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XFI_RX_FE_50OHMS_SEL_LSB, 2) : 0xF;
  
  if ((XFI_rx_term_sel > 0) && (XFI_rx_term_sel <= 3)){
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, XFI_rx_term_sel);

    #if LAB_PRINT_XFI
	 printk("DA_XFI_RX_TERMP_SEL[1:0]=%x\n", XFI_rx_term_sel);
	#endif 
  }
  else {
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, 0x1);

	#if LAB_PRINT_XFI
	 printk("Set XFI Impedance Level 2 as default! \n");
    #endif
  }
  
  //***************************************************************************************************************************//



 #if LAB_PRINT_XFI
   printk("=== XFI_OLT_RX_rxrdy_done ===\n");
#endif 

}




void XFI_RX_rxrdy(void)
{
  //seq_XFI_OLT_RX_rxrdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24,24 , 0x1);  //rg_force_rx_rdy
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24,24 , 0x0);   // rg_disb_rx_rdy 

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x0);                       // rg_sw_rx_fifo_rst_n         
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x1);                       // rg_sw_rx_fifo_rst_n


  //********************************************OPTIMIZE RX IMPEDANCE by EFUSE*****************************************************//  
  XFI_rx_term_sel = (get_phy_efuse(PON_XFI_efuse_valid_LSB, 1) == 1) ? get_phy_efuse(RG_PXP_XFI_RX_FE_50OHMS_SEL_LSB, 2) : 0xF;
  
  if ((XFI_rx_term_sel > 0) && (XFI_rx_term_sel <= 3)){
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, XFI_rx_term_sel);

   #if LAB_PRINT_XFI
	 printk("DA_XFI_RX_TERMP_SEL[1:0]=%x\n", XFI_rx_term_sel);
   #endif 
  }
  else {
	 IO_SPHYA_REG_BITS(AN7583_XFI_ANA_RG_XPON_RX_SIGDET_NOVTH, 25, 24, 0x2);

   #if LAB_PRINT_XFI
	 printk("Set XFI Impedance Level 2 as default! \n");
   #endif 
  }
  
  //***************************************************************************************************************************//

  

 #if LAB_PRINT_XFI
   printk("=== XFI_RX_rxrdy_done ===\n");
#endif 

}



void XFI_R2T_on(void)
{ 

    //R2T/
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 16, 16, 0x00);                     //rg_tx_bist_gen_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 8, 8, 0x00);                       //rg_bistctl_pat_tx_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16, 16, 0x01);                     // rg_bistctl_pat_rx_check_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BIST_1, 8, 8, 0x00);                                //all_lane_prbs_tx_en

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 8, 8, 0x00);                       //rg_r2t_mode
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 8, 8, 0x01);                       //rg_r2t_mode

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 24, 24, 0x00);                     //rg_r2t_fifo_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 24, 24, 0x01);                     //rg_r2t_fifo_en 
  

#if LAB_PRINT_XFI
   printk("=== XFI_R2T_On ===\n");
#endif

}






void XFI_OLT_BIST_setting(void)
{
  //aux_XFI_OLT_BIST_setting
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 2,0 , 0x2);                   // bcdr core control
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 6,4 , 0x2);                   // rg_smp_offset
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 10,8 , 0x3);                  // rg_vote_thrsh
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 13,12 , 0x0);                 // rg_shaper_method
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 17,16 , 0x2);                 // rg_reco_method
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 22,20 , 0x3);                 // rg_pre_tg_pnt
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_CORE_CTRL, 26,24 , 0x3);                 // rg_pre_sm_pnt

  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_csr_bistctl_sel, 3,0 , 0x5);             // sel: PRBS 7
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_MISC_CTRL2, 0,0 , 0x1);                  // PRBS test mode enable 
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_20, 4,4 , 0x1);               // rg_force_pma_tx_fifo_wr
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_20, 0,0 , 0x1);               // rg_force_sel_pma_tx_fifo_wr


 #if LAB_PRINT_XFI
   printk("=== XFI_OLT_BIST_setting_done ===\n");
#endif 

}



void XFI_BIST_setting(void)
{
  //aux_XFI_BIST_setting
   //BIST_tx_rx  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_ALIGN_PAT, 31,0 , 0x6081FD53 );          // bistctl_align_pattern
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_PRBS_INITIAL_SEED, 31,0 , 0xFF1FD53 );   // bistctl_prbs_init_seed
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 15,0 , 0x1 );       // rg_bistctl_prbs_fail_threshold
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 4,0 , 0x1 );                    // rg_bistctl_pat_sel
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_POLLUTION, 16,16 , 0x1 );                // rg_bist_tx_data_pollution_latch
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BIST_1, 0,0 , 0x0 );                             // anlt_px_lnx_lt_los
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BIST_1, 24,24 , 0x0 );                           // rg_lnx_bistctl_bit_error_rst_sel



 #if LAB_PRINT_XFI
   printk("=== XFI_BIST_setting_done ===\n");
#endif 

}



void XFI_OLT_BIST_on(void)
{
  //aux_XFI_OLT_BIST_on
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_csr_clear_en, 3,3 , 0x1);                 // csr_tx_en  
  udelay(100);
  
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_csr_clear_en, 0,0 , 0x1);                 // csr_clear_en , clear  counter ,1'b1 
  udelay(50);
  IO_SPHYA_REG_BITS(AN7583_XFI_OLT_BCDR_csr_clear_en, 0,0 , 0x0);                 // csr_clear_en , clear  counter ,1'b1 


 #if LAB_PRINT_XFI
   printk("=== XFI_OLT_BIST_on_done ===\n");
#endif 

}


void XFI_BIST_on(void)
{
  //aux_XFI_BIST_on
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 8,8 , 0x1 );                 // rg_bistctl_pat_tx_en
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 8,8 , 0x0 );                 // rg_r2t_mode_0: T2R_1: R2T
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 24,24 , 0x1 );               //rg_r2t_fifo_en  
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 16,16 , 0x1 );               // rg_tx_bist_gen_en 
  
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BIST_1, 8,8 , 0x0 );                          // all_lane_prbs_tx_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );				 // rg_bistctl_pat_rx_check_en
  udelay(100);
  
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16,16 , 0x0 );               // rg_bistctl_pat_rx_check_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );               // rg_bistctl_pat_rx_check_en

  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16,16 , 0x0 );               // rg_bistctl_pat_rx_check_en
  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_BISTCTL_CONTROL, 16,16 , 0x1 );               // rg_bistctl_pat_rx_check_en



 #if LAB_PRINT_XFI
   printk("=== XFI_BIST_on_done ===\n");
#endif 

}






void XFI_Eth_OLT_R2T_sel(uint r2t_sel)
{

 switch(r2t_sel)
  {
  
 case Eth_Ser_bist_data:	
   	
		  XFI_BIST_on();
		 
        break;

		
 case Eth_Ser_r2t_data  :
 	
	      XFI_BIST_on();
	      XFI_R2T_on();
		   
	   break;
	   
 case nor_pma_data  :

		  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 16, 16, 0x00); //rg_tx_bist_gen_en
		  IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 8, 8, 0x00);   //rg_r2t_mode
          
		                                        //{rg_r2t_mode , rg_tx_bist_gen_en}
		                                        //2'b01 : da_tx_data = bist_data
		                                        //2'b10 : da_tx_data = r2t_data
		                                        //otherwise : normal_pma_tx_data
            
	   break;

 case OLT_bist_data:	
   	
		  XFI_OLT_BIST_on();
		 
        break;
	

      default:
	 break;	
	}

}



void OLT_Ext_T2R (int spd_sel)
{  

   XFI_DIG_reset_hold(spd_sel);    
   XFI_CfgPhyType(spd_sel);   
     
   XFI_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	     
   XFI_TX_on();
   
   XFI_RX_preset(InitSpd_Preset); 
   //XFI_TDC_off(); 
   XFI_RX_on(InitSpd_RxOn); 
   XFI_RX_L2R(spd_sel); 

   XFI_RX_OSCal(); 
   //XFI_RX_pical();
   XFI_RX_pdos();
   XFI_RX_feos();
   XFI_RX_sdcal();  
   
   XFI_phy_status();
   XFI_DIG_reset_release(spd_sel);     
   //XFI_RX_L2D();   
   //EO_Scan(pon_Spd,0,7); //20221011
   //XPON_TDC_on();     
   XFI_OLT_RX_rxrdy();
   XFI_OLT_BIST_setting();
   XFI_Eth_OLT_R2T_sel(OLT_bist_data);     
   
}

EXPORT_SYMBOL(OLT_Ext_T2R);


void OLT_BCDR_PG (void)
{ 

   XFI_OLT_BIST_setting();
   XFI_Eth_OLT_R2T_sel(OLT_bist_data); 
}

EXPORT_SYMBOL(OLT_BCDR_PG);




void Eth_Ser_Ext_T2R (int spd_sel)
{  

   XFI_DIG_reset_hold(spd_sel);    
   XFI_CfgPhyType(spd_sel);   
   
   XFI_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	     
   XFI_TX_on();
   
   XFI_RX_preset(InitSpd_Preset); 
   //XFI_TDC_off(); 
   XFI_RX_on(InitSpd_RxOn); 
   XFI_RX_L2R(spd_sel); 

   XFI_RX_OSCal(); 
   XFI_RX_pical();
   XFI_RX_pdos();
   XFI_RX_feos();
   XFI_RX_sdcal();  
   
   XFI_phy_status();
   XFI_DIG_reset_release(spd_sel);     
   XFI_RX_L2D();   
   XFI_EO_Scan(spd_sel,0,7,0); //20221011
   //XFI_TDC_on();     
   XFI_RX_rxrdy();
   XFI_BIST_setting();
   XFI_Eth_OLT_R2T_sel(Eth_Ser_bist_data);     
   
}

EXPORT_SYMBOL(Eth_Ser_Ext_T2R);


void Eth_Ser_Ext_R2T (int spd_sel)
{  

   XFI_DIG_reset_hold(spd_sel);    
   XFI_CfgPhyType(spd_sel);   
   
   XFI_TXPLL_on();
   //XPON_DIG_fm_on(pon_Spd); 	     
   XFI_TX_on();
   
   XFI_RX_preset(InitSpd_Preset); 
   //XFI_TDC_off(); 
   XFI_RX_on(InitSpd_RxOn); 
   XFI_RX_L2R(spd_sel); 

   XFI_RX_OSCal(); 
   XFI_RX_pical();
   XFI_RX_pdos();
   XFI_RX_feos();
   XFI_RX_sdcal();  
   
   XFI_phy_status();
   XFI_DIG_reset_release(spd_sel);     
   XFI_RX_L2D();   
   XFI_EO_Scan(spd_sel,0,7,0); //20221011
   //XFI_TDC_on();     
   XFI_RX_rxrdy();
   XFI_BIST_setting();
   XFI_Eth_OLT_R2T_sel(Eth_Ser_r2t_data);     
   
}

EXPORT_SYMBOL(Eth_Ser_Ext_R2T);




//================= ASIC_XFI_ETH_SERDES_API =================


u8 XFI_ETH_RX_SigDet_Flag(void)
{	
	u8 i,cnt = 0;		
	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RESERVE_0, 31, 0, 0x30000); //rg_dig_reserve_0

	
	for (i=0;i<=5;i++){				
		cnt = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_ADD_DIG_RO_RESERVE_2, 8, 8);  //rg_dig_reserve_8		
		cnt += cnt;
	}
	if(XFI_PCS_PRINT)printk("RX_SigDet_Flag, cnt %x\n",cnt);	
	
	return cnt >= 4? 1:0;  
}
EXPORT_SYMBOL(XFI_ETH_RX_SigDet_Flag);

u8 XFI_ETH_RX_SigDet_OUT_Read(void)
{		
	return IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_TORGS_DEBUG_11, 24, 24);  
}
EXPORT_SYMBOL(XFI_ETH_RX_SigDet_OUT_Read);

u8 XFI_ETH_RX_SigDet_Flag_D(void)
{
	uint XPON_INT_STA_3 = 0 ;

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_EN_3, 16, 16, 0x0); //rg_rx_sigdet_int_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_SIGDET_1, 0, 0, 0x0); //rg_sigdet_en
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 8, 8, 0x0); //rg_sigdet_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 8, 8, 0x1); //rg_sigdet_rst_b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 16, 16, 0x1); //rg_sigdet_int	
	udelay(50);	

	XPON_INT_STA_3 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 16, 16);  //rg_sigdet_int
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_SIGDET_1, 0, 0, 0x0); //rg_sigdet_en	
	
	if(XFI_PCS_PRINT)printk("RX_SigDet_Flag_D %x\n",XPON_INT_STA_3);
	return XPON_INT_STA_3;
}
EXPORT_SYMBOL(XFI_ETH_RX_SigDet_Flag_D);


void XFI_ETH_SigDet_Int_Init(u8 en)
{
	if(XFI_PCS_PRINT)printk("SigDet_Int_Init, en %x\n",en);

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 16, 16, 0x1); //rg_sigdet_int	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 31, 0, 0x0); //rg_sigdet_int	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_EN_3, 16, 16, en); //rg_rx_sigdet_int_en	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_SIGDET_1, 0, 0, en); //rg_sigdet_en	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 8, 8, 0x0); //rg_sigdet_rst_b	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_RESET_1, 8, 8, 0x1); //rg_sigdet_rst_b	
}
EXPORT_SYMBOL(XFI_ETH_SigDet_Int_Init);


u8 XFI_ETH_SigDet_IntEn_sta(void)
{
	u8 read_data=0;

	read_data = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_EN_3, 16, 16);  //rg_rx_sigdet_int_en	
	
	return (u8)read_data;
}
EXPORT_SYMBOL(XFI_ETH_SigDet_IntEn_sta);


u32 XFI_ETH_SigDet_Int_sta3_read(void)
{	
	uint read_data=0;
	
	read_data = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 31, 0);  	

	return read_data;
}
EXPORT_SYMBOL(XFI_ETH_SigDet_Int_sta3_read);


void XFI_ETH_SigDet_Int_sta3_write(u32 data)
{	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_XPON_INT_STA_3, 31, 0, data); //rg_sigdet_rst_b	
}
EXPORT_SYMBOL(XFI_ETH_SigDet_Int_sta3_write);


u8 XFI_ETH_RX_CDR_LFP_L2D_sta(void)
 {
	u8 sta; 

	sta = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8);  //rg_force_sel_da_pxp_cdr_lpf_lck2data
	
	if(XFI_PCS_PRINT) printk("RX_CDR_LFP_L2D_sta %x\n",sta);
	return sta;//Read 1fa7b818 bit8 
}
EXPORT_SYMBOL(XFI_ETH_RX_CDR_LFP_L2D_sta);


u8 XFI_ETH_RX_RDY_Sta(void)
{
	uint read_data=0;

	read_data = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24);  //rg_disb_rx_dly

	if(XFI_PCS_PRINT) printk("RX_RDY_Sta %x \n",read_data);	
	return read_data ;	
}
EXPORT_SYMBOL(XFI_ETH_RX_RDY_Sta);



void XFI_ETH_RX_RDY(u8 mod,u8 sel)
{	
	if(XFI_PCS_PRINT) printk("RX_RDY %x, sel %x\n",mod,sel);

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1, 24, 24, mod); //rg_disb_rx_rdy	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1, 24, 24, sel); //rg_force_rx_rdy	
	
}
EXPORT_SYMBOL(XFI_ETH_RX_RDY);


void XFI_ETH_RX_CDR_LFP_L2D(u8 mod,u8 sel)
{
	if(XFI_PCS_PRINT) printk("RX_CDR_LFP_L2D mode %x, sel %x\n",mod,sel);
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 8, 8, mod); //rg_force_sel_da_pxp_cdr_lpf_lck2data	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 0, 0, sel); //rg_force_da_pxp_cdr_lpf_lck2data	
}
EXPORT_SYMBOL(XFI_ETH_RX_CDR_LFP_L2D);


void XFI_ETH_RX_CDR_LPF_RSTB(u8 mod,u8 sel)
{	
	if(XFI_PCS_PRINT) printk("RX_CDR_LPF_RSTB mode %x, sel%x\n",mod,sel);

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 24, 24, mod); //rg_force_sel_da_pxp_cdr_lpf_rstb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_lpf_lck2data, 16, 16, sel); //rg_force_da_pxp_cdr_lpf_rstb
}
EXPORT_SYMBOL(XFI_ETH_RX_CDR_LPF_RSTB);

void XFI_ETH_Power_Down(void)
{

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 0,0 , 0x0 );							// rg_sw_rx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 6,6 , 0x0 );							// rg_sw_tx_fifo_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 5,5 , 0x0 );							// rg_sw_ref_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 4,4 , 0x0 );							// rg_sw_allpcs_rst_n 
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 2,2 , 0x0 );							// rg_sw_tx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 1,1 , 0x0 );							// rg_sw_rx_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 3,3 , 0x0 );							// rg_sw_pma_rst_n
    udelay(10);


	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 16,16 , 0x0 );	// rg_force_da_pxp_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pr_pieye_pwdb, 0,0 , 0x0 );	// rg_force_da_pxp_cdr_pr_pieye_pwdb	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_cdr_pd_pwdb, 0,0 , 0x0 );			// rg_force_da_pxp_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_fe_pwdb, 0,0 , 0x0 );			// rg_force_da_pxp_rx_fe_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_rx_scan_rst_b, 16,16 , 0x0 );		// rg_force_da_pxp_rx_sigdet_pwdb

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 24,24 , 0x0 ); 					// rg_da_xpon_cdr_pd_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 16,16 , 0x0 ); 					// rg_da_xpon_cdr_pr_pieye_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 8,8 , 0x0 );						// rg_da_xpon_cdr_pr_pwdb
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_0, 0,0 , 0x0 );						// rg_da_xpon_rx_fe_pwdbb   
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_DA_XPON_PWDB_1, 0,0 , 0x0 );						//rg_da_xpon_rx_sigdet_pwdb 

}
EXPORT_SYMBOL(XFI_ETH_Power_Down);



void XFI_ETH_PCS_Reset_Hold(void)
{	
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 7,7 , 0x0);       // rg_sw_xfi_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 8,8 , 0x0);       // rg_sw_xfi_rxpcs_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 9,9 , 0x0);       // rg_sw_xfi_rxpcs_bist_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 10,10 , 0x0);     // rg_sw_hsg_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 11,11 , 0x0);     // rg_sw_hsg_rxpcs_rst_n

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 16,16 , 0x0);     // rg_sw_xfi_txmac_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 17,17 , 0x0);     // rg_sw_xfi_rxmac_rst_n
}
EXPORT_SYMBOL(XFI_ETH_PCS_Reset_Hold);



void XFI_ETH_PCS_Reset_Release(int spd_sel)
{	

	//printk("ETH spd_sel = 0x%x\n",spd_sel);
	if(spd_sel == Eth_Ser_5GBaseR)
	{
		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 5,3 , 0x1); 
		IO_SPHYA_REG_BITS(AN7583_XFI_PMA_ADD_XPON_MODE_1, 11,9 , 0x1);
		//printk("5G BaseR PHY Setting!\n");
	}
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 7,7 , 0x1);       // rg_sw_xfi_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 8,8 , 0x1);       // rg_sw_xfi_rxpcs_rst_n
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 9,9 , 0x1);       // rg_sw_xfi_rxpcs_bist_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 10,10 , 0x1);     // rg_sw_hsg_txpcs_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 11,11 , 0x1);     // rg_sw_hsg_rxpcs_rst_n

	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 16,16 , 0x1);     // rg_sw_xfi_txmac_rst_n
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_SW_RST_SET, 17,17 , 0x1);     // rg_sw_xfi_rxmac_rst_n

}
EXPORT_SYMBOL(XFI_ETH_PCS_Reset_Release);


void XFI_ETH_TXFIR_Set(u8 cn1, u8 c0b, u8 c1, u8 c2)
{
	//TX_FIR
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 24,24 , 0x1 ); 		  // rg_force_sel_da_pxp_tx_fir_cn1
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c0b
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 8,8 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c1
	IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 24,24 , 0x1 );			  // rg_force_sel_da_pxp_tx_fir_c2
	

    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16, cn1);       //rg_force_da_pxp_tx_fir_cn1
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0, c0b);         //rg_force_da_pxp_tx_fir_c0b
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0, c1);          //rg_force_da_pxp_tx_fir_c1
    IO_SPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16, c2);        //rg_force_da_pxp_tx_fir_c2
}
EXPORT_SYMBOL(XFI_ETH_TXFIR_Set);

void XFI_ETH_TXFIR_Get(void)
{
    u8 cn1 = 0,c0b = 0,c1 = 0,c2 = 0;
    //TX_FIR
    cn1 = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 20, 16);       //rg_force_da_pxp_tx_fir_cn1
    c0b = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c0b, 5, 0);         //rg_force_da_pxp_tx_fir_c0b
    c1  = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 4, 0);          //rg_force_da_pxp_tx_fir_c1
    c2  = IO_GPHYA_REG_BITS(AN7583_XFI_PMA_rg_force_da_pxp_tx_fir_c1, 18, 16);        //rg_force_da_pxp_tx_fir_c2

    printk("XFI TXFIR[cn1,c0b,c1,c2] = [%x, %x, %x, %x]\n",cn1,c0b,c1,c2);
}
EXPORT_SYMBOL(XFI_ETH_TXFIR_Get);



//================= ASIC_XFI_ETH_SERDES_API =================



void XFI_RG_RW_test(void)
{
	printk("\nXFI_RW_test\n");
	//printk("RG:0x%x = 0x%x\n", EN7583_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en, IO_GPHYREG(EN7581_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en));
	//JCPLL_Dis
	//IO_SPHYA_REG_BITS(EN7581_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en, 24, 24, 0x01); //rg_force_sel_da_pxp_jcpll_en
	//printk("RG:0x%x = 0x%x\n", EN7581_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en, IO_GPHYREG(EN7581_XFI_PMA_rg_force_da_pxp_jcpll_ckout_en));
}
EXPORT_SYMBOL(XFI_RG_RW_test);

EXPORT_SYMBOL(XFI_Init);

EXPORT_SYMBOL(OLT_plug_reset);

EXPORT_SYMBOL(Eth_Ser_plug_reset);




void OLT_RG_RW_test(void)
{
	printk("\nOLT_RG_RW_test\n");
	printk("RG:0x%x = 0x%x\n", 0x1fbe5800, IO_GPHYA_REG_BITS(0x1fbe5800, 31, 0));
	printk("\nOLT_RG_Write_test_0x5555AAAA\n");
	IO_SPHYA_REG_BITS(0x1fbe5800, 31, 0, 0x5555AAAA);
	printk("RG:0x%x = 0x%x\n", 0x1fbe5800, IO_GPHYA_REG_BITS(0x1fbe5800, 31, 0));

	//printk("RG:0x%x = 0x%x\n", 0x1fbe5900, IO_GPHYA_REG_BITS(0x1fbe5900, 31, 0));
	//IO_SPHYA_REG_BITS(0x1fbe5900, 31, 0, 0x5555AAAA);
	//printk("RG:0x%x = 0x%x\n", 0x1fbe5900, IO_GPHYA_REG_BITS(0x1fbe5900, 31, 0));

	
}
EXPORT_SYMBOL(OLT_RG_RW_test);



