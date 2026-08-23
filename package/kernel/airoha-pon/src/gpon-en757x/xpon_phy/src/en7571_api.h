#ifndef _EN7571_API_H_
#define _EN7571_API_H_
#include "phy_types.h"     //dyma_20170117

int en7571_init(void);
int en7571_ADC_temperature_get(void);	// by HC 20150527
int en7571_ADC_voltage_get(void);			// by HC 20150527
int en7571_EN7571_detection(void);			//by HC 20150301
int en7571_LOS_calibration(int LOS_thld_H, int LOS_thld_L);		// by HC 20160608
int en7571_isDCLworking(void);

UINT16 en7571_temperature_get_8472(void);
UINT16 en7571_supply_voltage_get_8472(void);
UINT16 en7571_bias_current_get_8472(void);
UINT16 en7571_TxPower_get_8472(void);		//by HC 20150123
UINT16 en7571_RxPower_get_8472(void);		//by HC 20150127

UINT32 en7571_info(uint select);
UINT32 en7571_SVADC_get(void);				//by HC 20170203
UINT32 en7571_PWRADC_get(void);
UINT32 en7571_RSSI_get(void);
UINT32 en7571_dark_current(void);

void en7571_TGEN(int xPON);					// by HC 20150512
void en7571_TxSD_level_set(void);
void en7571_LOS_level_set(void);				//by HC 20150304
void en7571_ADC_calibration(void);
void en7571_APD_initialization(void);		//by HC 20150327
void en7571_APD_control(void);
void en7571_APD_DAC(UINT8 DAC);
void en7571_load_Tx_cal_data(void);			//by HC 20150129
void en7571_RougeONU_clear(void);			//by HC 20150121
void en7571_safe_circuit_reset(void);			//by HC 20150127
void en7571_trans_model_setting(void);
void en7571_tx_power_alarm_get(void);		// by HC 20150528
void en7571_rx_power_alarm_get(void);		// by HC 20150528
void en7571_PWRADC_calibration(void);		//by HC 20150117
void en7571_PWRADC_enable(void);			//by HC 20160718
void en7571_RSSI_calibration(void);			// by HC 20150609
void en7571_xPON_Tx_calibration(int input);				//by HC 20150209
void en7571_save_flash_APD(uint slope_up, uint slope_dn, uint VAPD25);	//by HC 20150309
void en7571_save_flash_LOS(void);											//by HC 20150309
void en7571_save_flash_DDMI_TxPower(uint TxPower, uint offset);	//by HC 20150309
void en7571_save_flash_DDMI_RxPower(uint RxPower, uint offset);	//by HC 20150309
void en7571_save_flash_Tx_data(void);										// by HC 20160608
void en7571_APD_I2C_write(unchar ptr[1]);								//by HC 20150310
void en7571_DDMI_check_8472(void);									//by HC 20150311
void en7571_BoB_info(void);												//by HC 20150325
void en7571_temperature_get(void);										//by HC 20150326
void en7571_calibration_pattern(int input);							//by HC 20150518
void en7571_disable_pattern(void);										//by HC 20150521
void en7571_CDR(int CDR_switch);										//by HC 20150522
void en7571_internal_clock(void);
void en7571_HWKT(int input);												// by HC 20161115
void en7571_hw_reset(void);												// by HC 20161115
void en7571_reg_init(void);													// by HC 20160613
void en7571_mpdh_stepsize(int input);
void en7571_7571_enable(void);											// by HC 20160914
void en7571_DCL_start(void);												// by HC 20160705
void en7571_DCL_stop(void);
void en7571_cal_Iav_Imod(UINT32 Iav_mod_k, int I);
void en7571_cal_Pav_P1(UINT32 Pav_P1_k, int P);
void en7571_cal_force_mode(void);
void en7571_sw_reset(void);												// by HC 20151014
void en7571_RSSI_gain_init(void);										// by HC 20151014
void en7571_LOS_init(void);												// by HC 20160427
void en7571_fine_tune_PWR_ER(int pwr_er,int up_dn);				// by HC 20160823
void en7571_SWKT(void);
void en7571_change_Imod(UINT32 input);
void en7571_change_mpdh(UINT32 input);
void en7571_cross_Imod(UINT32 input);
void en7571_ER_Ctrl(void);													// by HC 20161205
void en7571_show_ERC_ratio(void);										// by HC 20170106
void en7571_VBR_find(int High , int Low);												// by YW 20161214
void en7571_LOS_find(int LOS_find);									//by YW 20170103
void en7571_eFuse_temperature_get(void);								//by YW 20161214
void en7571_force_mode(void);
void en7571_auto_lock_mode(void);
void en7571_tune_KT(UINT16 input);
void en7571_config(void);
void en7571_link_reg(int input);
void en7571_burst_ctrl(int input);

#endif /* _EN7571_API_H_ */

