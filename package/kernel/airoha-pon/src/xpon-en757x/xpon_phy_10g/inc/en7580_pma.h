#ifndef _EN7580_PMA_H_
#define _EN7580_PMA_H_


// en7580 pma split function from Morris
//===============================================================

#define Pon_Eye_Scan_Result  0
#define Xfi_Eye_Scan_Result  0
#define LAB_PRINT_PON 0
#define LAB_PON_Test  0


#define fm_Async_GPON     1   //fm_DS_2.48832G  /  US_1.24416G
#define fm_Sync_EPON      2   //fm_DS_1.25G      /  US_1.25G
#define fm_Async_XEPON    3   //fm_DS_10.3125G  /  US_1.25G
#define fm_Sync_XEPON     4   //fm_DS_10.3125G  /  US_10.3125G
#define fm_Async_XGPON    5   //fm_DS_9.95328G  /  US_2.48832G
#define fm_Sync_XGSPON    6   //fm_DS_9.95328G  /  US_9.95328G
#define fm_Sync_GPON      7   //fm_DS_2.48832G  /  US_2.48832G

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



#define bist_data         1
#define r2t_data          2
#define nor_pma_data      3

#define FIRST_PLUG_IN     1
#define PLUG_IN           2
#define PLUG_OUT          3

#define TX_Async_GPON     1   //DS_2.48832G  /  US_1.24416G   
#define TX_Sync_EPON      2   //DS_1.25G      /  US_1.25G     
#define TX_Async_XEPON    3   //DS_10.3125G  /  US_1.25G       
#define TX_Sync_XEPON     4   //DS_10.3125G  /  US_10.3125G   
#define TX_Async_XGPON    5   //DS_9.95328G  /  US_2.48832G
#define TX_Sync_XGSPON    6   //DS_9.95328G  /  US_9.95328G   
#define TX_Sync_GPON      7   //DS_2.48832G  /  US_2.48832G


#define EO_Async_GPON     0    //DS(RX)_2.48832G  /  US(TX)_1.24416G

#define EO_Sync_EPON_1    1    //DS_1.25G       /  US_1.25G
#define EO_Sync_EPON_2    8    //DS_1.25G       /  US_1.25G

#define EO_Async_XEPON    6    //DS_10.3125G   /  US_1.25G

#define EO_Sync_XEPON     7    //DS_10.3125G   /  US_10.3125G
 
#define EO_Async_XGPON_1  9   //DS_9.95328G   /  US_2.48832G
#define EO_Async_XGPON_2  12   //DS_9.95328G   /  US_2.48832G

#define EO_Sync_XGSPON_1   10    //DS_9.95328G   /  US_9.95328G
#define EO_Sync_XGSPON_2   11   //DS_9.95328G   /  US_9.95328G

#define EO_Sync_GPON      99   //DS_2.48832G   /  US_2.48832G




#define EO_Sync_XFI_10G        0   //DS_10.3125G  /  US_10.3125G
#define EO_Sync_XFI_5G_baseR   1   //DS_5.15625G  /  US_5.15625G
#define EO_Sync_XFI_HSGMII     4   //DS_3.125G     /  US_3.125G
#define EO_Sync_XFI_SGMII      5   //DS_1.25G      /  US_1.25G

#define Power_Saving_PMA        1  //julia_20191120

#define _PMA_FUNCTION_SPLIT_LINE_BASIC 
int xpon_pma_mode_init(void);
void xpon_init (int bit_sel);
void fiber_plug_reset(int plug_sel , int bit_sel);
void Ext_R2T (int bit_sel);
void Ext_T2R (int bit_sel);
void Pon_Version (void);
void setting_mode_XPON_DIG(uint scu_bit_sel);
void setting_mode_XPON_TXPLL(uint rate_sel);
void setting_mode_XPON_TX(uint tx_bit_sel);
void setting_mode_XPON_RX(uint rx_bit_sel);
void setting_XPON_DIG(void);
void setting_XPON_ANA(uint rate_sel);
void seq_XPON_DIG_reset(void);
void seq_JCPLL_on(void);
void seq_XPON_DIG_fm_on(int fm_bit_sel);
void seq_XPON_TXPLL_on(void);
void seq_XPON_TX_on(void);
void seq_XPON_RX_preset(void);
void aux_XPON_TDC_off(void);
void seq_XPON_RX_on(void);
void seq_XPON_RX_L2R(void);
void seq_XPON_RX_pical(void);
void seq_XPON_RX_pdos(void);
void seq_XPON_RX_feos(void);
void seq_XPON_RX_sdcal(void);
void aux_XPON_phy_status(void);
void seq_XPON_RX_osrdy(void);
void seq_XPON_RX_L2D(void);
void seq_XPON_FLL_Reset(void);
void aux_XPON_TDC_on(void);
void seq_XPON_RX_blwc_on(void);
void seq_XPON_RX_rxrdy(void);
void aux_XPON_BIST_setting(void);
void aux_XPON_BIST_on(void);
void aux_XPON_R2T_sel(uint r2t_sel);
void aux_XPON_R2T_on(void);
void EO_Scan(uint mode_sel, uint bit_sel, uint EO_start, uint EO_end, uint dbg_sel);
void XPON_eye_setting(uint bit_set);
void XFI_eye_setting(uint bit_set);
void XPON_eye_Cal(void);
void XFI_eye_Cal(void);
void debug_Mode (void);
void RXFLL (void);
void delay_ponphy(int cycle_num);
int XPON_eye_EO(uint bit_rate);
int XFI_eye_EO(uint bit_rate);
int XPON_readout_EO(int sweep_direction);
int XFI_readout_EO(int sweep_direction);


#define _PMA_FUNCTION_SPLIT_LINE_EXTRA
void sw_pma_rst_hold(void);
void sw_pma_rst_release(void);
void sw_tx_rst(void); //add by ang_20191031


#endif /* _EN7580_PMA_H_ */
