#ifndef _EN7581_PMA_H_
#define _EN7581_PMA_H_

//#include "..\..\common\general.h"

#ifdef __cplusplus
extern "C" {
#endif

extern u32 GET_PDIDR(void);


// en7581 pma split function from Morris
//===============================================================

#define Pon_Eye_Scan_Result  0
#define LAB_PRINT_PON 0
#define LAB_PON_Test  0
#define Pon_PR_WK 0


#define fm_Async_GPON     0   //fm_DS_2.48832G  /  US_1.24416G
#define fm_Sync_EPON_1    1   //fm_DS_1.25G     /  US_1.25G
#define fm_Sync_EPON_2    8   //fm_DS_1.25G     /  US_1.25G
#define fm_Async_XEPON    6   //fm_DS_10.3125G  /  US_1.25G
#define fm_Sync_XEPON     7   //fm_DS_10.3125G  /  US_10.3125G
#define fm_Async_XGPON    9   //fm_DS_9.95328G  /  US_2.48832G
#define fm_Sync_XGSPON_1  10   //fm_DS_9.95328G  /  US_9.95328G
#define fm_Sync_XGSPON_2  11   //fm_DS_9.95328G  /  US_9.95328G

#define fm_Sync_GPON      99   //fm_DS_2.48832G  /  US_2.48832G


#define scu_Async_GPON    1   //DS_2.48832G  /  US_1.24416G
#define scu_Sync_EPON     2   //DS_1.25G      /  US_1.25G
#define scu_Async_XEPON   3   //DS_10.3125G  /  US_1.25G
#define scu_Sync_XEPON    4   //DS_10.3125G  /  US_10.3125G
#define scu_Async_XGPON   5   //DS_9.95328G  /  US_2.48832G
#define scu_Sync_XGSPON   6   //DS_9.95328G  /  US_9.95328G
#define scu_Sync_GPON     7   //DS_2.48832G  /  US_2.48832G

//define_Bit Rate
#define Async_GPON        0   //DS(RX)_2.48832G  /  US(TX)_1.24416G , SCU_WAN_CONF_REG_WAN_SEL_GPON
	    
#define Sync_EPON_1       1   //DS_1.25G      /  US_1.25G                  , SCU_WAN_CONF_REG_WAN_SEL_EPON
#define Sync_EPON_2       8   //DS_1.25G      /  US_1.25G                  , SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G
	    
#define Async_XEPON       6   //DS_10.3125G  /  US_1.25G                  , SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G
	    
#define Sync_XEPON        7   //DS_10.3125G  /  US_10.3125G              , SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G
	    
#define Async_XGPON_1     9  //DS_9.95328G  /  US_2.48832G              , SCU_WAN_CONF_REG_WAN_SEL_XGPON
#define Async_XGPON_2     12  //DS_9.95328G  /  US_2.48832G              , SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G
	    
#define Sync_XGSPON_1     10   //DS_9.95328G  /  US_9.95328G              , SCU_WAN_CONF_REG_WAN_SEL_XGSPON
#define Sync_XGSPON_2     11  //DS_9.95328G  /  US_9.95328G              , SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G
	    
#define Sync_GPON         99   //DS_2.48832G  /  US_2.48832G



#define  bist_data         1
#define  r2t_data          2
#define  nor_pma_data      3

		 
#define  FIRST_PLUG_IN     1
#define  PLUG_IN           2
#define  PLUG_OUT          3
		 
#define  TX_Async_GPON     1   //DS_2.48832G  /  US_1.24416G   
#define  TX_Sync_EPON      2   //DS_1.25G      /  US_1.25G     
#define  TX_Async_XEPON    3   //DS_10.3125G  /  US_1.25G       
#define  TX_Sync_XEPON     4   //DS_10.3125G  /  US_10.3125G   
#define  TX_Async_XGPON    5   //DS_9.95328G  /  US_2.48832G
#define  TX_Sync_XGSPON    6   //DS_9.95328G  /  US_9.95328G   
#define  TX_Sync_GPON      7   //DS_2.48832G  /  US_2.48832G


#define  EO_Async_GPON     0    //DS(RX)_2.48832G  /  US(TX)_1.24416G
		 
#define  EO_Sync_EPON_1    1    //DS_1.25G       /  US_1.25G
#define  EO_Sync_EPON_2    8    //DS_1.25G       /  US_1.25G
		 
#define  EO_Async_XEPON    6    //DS_10.3125G   /  US_1.25G
		 
#define  EO_Sync_XEPON     7    //DS_10.3125G   /  US_10.3125G
		 
#define  EO_Async_XGPON_1  9   //DS_9.95328G   /  US_2.48832G
#define  EO_Async_XGPON_2  12   //DS_9.95328G   /  US_2.48832G
		 
#define  EO_Sync_XGSPON_1   10    //DS_9.95328G   /  US_9.95328G
#define  EO_Sync_XGSPON_2   11   //DS_9.95328G   /  US_9.95328G
		 
#define  EO_Sync_GPON      99   //DS_2.48832G   /  US_2.48832G



typedef enum{
	Align_PRBS7  = 0x6081fd53,
	Align_PRBS9  = 0xd97b0d53,
	Align_PRBS11 = 0xa1903d53,
	Align_PRBS15 = 0xe1fd7d53,
	Align_PRBS23 = 0x5cf1fd53,
	Align_PRBS31 = 0x8FF1FD53
	
}PON_RX_Align;


typedef enum{
	PRBS7   = 0x1, 
	PRBS9   = 0x2,	
	PRBS11  = 0x13,	
	PRBS15  = 0x3,
	PRBS23  = 0x4,
    PRBS31  = 0x5,
    HFTP    = 0x6,
    MFTP    = 0x7,
    SWP_4   = 0x8,  //Square_wave_pattern (n = 4)
    SWP_5   = 0x9,  //Square_wave_pattern (n = 5) LFTP
    SWP_6   = 0xA,  //Square_wave_pattern (n = 6) 
    SWP_7   = 0xB,  //Square_wave_pattern (n = 7) 
    SWP_8   = 0xC,  //Square_wave_pattern (n = 8) LFTP
    SWP_9   = 0xD,  //Square_wave_pattern (n = 9) 
    SWP_10  = 0xE,  //Square_wave_pattern (n = 10)
    SWP_11  = 0xF,  //Square_wave_pattern (n = 11)
    UDP     = 0x10, //User define Pattern
    All_1   = 0x11, //All_1
    All_0   = 0x12  //All_0
    
}PON_TX_Pat;


typedef enum{
	bist_pat_data  = 1,
	seq_r2t_data   = 2,
	seq_nor_pma_data = 3
}PON_R2T_Mode;


typedef struct {
    PON_RX_Align		Align_Pat;
	PON_TX_Pat			TX_Pat   ;	

} AN7581_XPON_PMA_Param_T ;




#define Power_Saving_PMA        1  //julia_20191120

#define _PMA_FUNCTION_SPLIT_LINE_BASIC 


int  xpon_pma_mode_init(void);
void xpon_init (int bit_sel);
void fiber_plug_reset(int plug_sel , int bit_sel);
void pon_Ext_R2T (int pon_Spd);
void pon_Ext_T2R (int bit_sel);
void Pon_Version (void);
void XPON_DIG(uint scu_bit_sel);
void XPON_TXPLL(uint rate_sel);
void JCPLL_on (void);
void TXPLL_on (void);
void XPON_TX(uint tx_bit_sel);
void XPON_RX(uint rx_bit_sel);
//void XPON_DIG(void);
void XPON_ANA(uint rate_sel);
void XPON_DIG_reset(int plug_sel);
void JCPLL_on(void);
void XPON_DIG_fm_on(int fm_bit_sel);
void XPON_TXPLL_on(void);
void XPON_TX_on(void);
void XPON_RX_preset(void);
void XPON_TDC_off(void);
void XPON_RX_on(int bit_set);
void XPON_RX_L2R(void);
void XPON_RX_OSCal(void);
void XPON_RX_pical(void);
void XPON_RX_pdos(void);
void XPON_RX_feos(void);
void XPON_RX_sdcal(void);
void XPON_phy_status(void);
void XPON_RX_osrdy(void);
void XPON_RX_L2D(void);
void XPON_DIG_reset_hold(void);
void XPON_DIG_reset_release(void);
void XPON_TDC_on(void);
void XPON_RX_blwc_on(void);
void XPON_RX_rxrdy(void);
void XPON_BIST_setting(void);
void XPON_BIST_on(void);
void XPON_R2T_sel(uint r2t_sel);
void XPON_R2T_on(void);
void EO_Scan(uint EO_start, uint EO_end, uint dbg_sel);
void XPON_eye_setting(uint bit_set);
//void XFI_eye_setting(uint bit_set);
void XPON_eye_Cal(void);
//void XFI_eye_Cal(void);
void debug_Mode (void);
void RXFLL (void);
void delay_ponphy(int cycle_num);
int XPON_eye_EO(uint bit_rate);
//int XFI_eye_EO(uint bit_rate);
int XPON_readout_EO(int sweep_direction);
int XPON_PrCal_WK(int bit_sel);
void XPON_FLL_Reset(void);

//int XFI_readout_EO(int sweep_direction);

void pon_WanSelInit(uint bit_set);
void pon_LinkControl(int pon_Spd);
void pon_CfgPhyType(int pon_Spd);
void Pon_Opt_Cal (int mode_Sel , int pat_sel);

#define _PMA_FUNCTION_SPLIT_LINE_EXTRA
void sw_pma_rst_hold(void);
void sw_pma_rst_release(void);
void sw_tx_rst(void); //add by ang_20191031

#endif /* _EN7580_PMA_H_ */
