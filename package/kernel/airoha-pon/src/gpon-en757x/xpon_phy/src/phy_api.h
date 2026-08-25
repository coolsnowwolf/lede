#ifndef _PHY_API_H_
#define _PHY_API_H_

#include "phy_def.h"
#include "phy.h"
#include "phy_types.h"

/*BER alarm*/
int phy_ber_alarm(void);
/* transceiver power switch*/
int phy_trans_power_switch(unchar trans_switch);
/*GPON Tx overhead setting*/
int phy_gpon_extend_preamble(PPHY_GponPreb_T pon_preb);
int phy_gpon_delimiter_guard(uint delimiter_pat, unchar guard_time_pat);
/*Phy counter read and clear*/
void phy_counter_clear(unchar counter_type);
void phy_rx_fec_counter(PPHY_FecCount_T fec_counter);
void phy_rx_frame_counter(PPHY_FrameCount_T frame_counter);
void phy_rx_sensitivity_test(void);
int phy_bip_counter(void);
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_60928)
int phy_tx_frame_counter(void);
int phy_tx_burst_counter(void);
#ifdef TCSUPPORT_WAN_EPON	
int phy_epon_frame_err_cnt(void);
#endif
#endif /* CONFIG_USE_MT7520_ASIC */
/*transceiver model getting */
void phy_trans_model_setting(void);	//Fixed by HC 20150207
/*transceiver alarm */
int phy_tx_alarm(void);
int phy_rx_power_alarm(void);
void phy_trans_alarm_getting(PPHY_TransAlarm_T transceiver_alarm);
/*transceiver pin define*/
int phy_trans_rx_setting(unchar rx_sd_inv);
int phy_trans_rx_getting(void);
int phy_trans_tx_setting(PPHY_TransConfig_T trans_status);
void phy_trans_tx_getting(PPHY_TransConfig_T trans_status);
/*get transceiver parameter*/
/*Rogue mode send PRBS*/
int phy_rogue_prbs_config(unchar rogue_mode);
/*Tx bursr/continuous mode switching*/
int phy_tx_burst_config(unchar burst_mode);
int phy_tx_burst_getting(void);
/*Tx swing amplifier*/
int phy_tx_amp_setting(unchar amp_level);
/*Rx FEC setting and get TRx FEC status*/
int phy_rx_fec_setting(unchar fec_control);
int phy_rx_fec_getting(void);
int phy_rx_fec_status(void);
int eponMode_phy_reset(void);		//Fixed by HC 20150207
int phy_int_getting(void);
int phy_lof_status(void);
/*RX counter enable and reset*/
int phy_cnt_enable(unchar errcnt_en, unchar bipcnt_en, unchar fmcnt_en);
int phy_reset_counter(void);
/*Tx bit delay setting*/
int phy_bit_delay(unchar delay_value);
/**/
int phy_gpon_tx_power(ushort tx_power);
/*Tx test pattern*/									//by HC 20150304
void phy_tx_test_pattern(uint pattern);
	int phy_fw_ready(unchar fwrdy_en);
#if defined(CONFIG_USE_MT7520_ASIC) || defined(CONFIG_USE_A60928)
	/*for testing*/
	int phy_freq_meter(unchar freq_en, unchar freq_sele);
	int phy_byte_delay_manual(unchar byte_dly_en, unchar byte_dly);
	int phy_tx_fec_manual(unchar tx_fec_en);
#endif /* CONFIG_USE_MT7520_ASIC */
#ifdef TCSUPPORT_CPU_EN7521
int phy_set_epon_ts_continue_mode(uint mode);
#endif
int mt7570_init(void);
void mt7570_TGEN(int xPON);					// by HC 20150512
void mt7570_TxSD_level_set(void);
int mt7570_TxSD_level_calibration(void);
void mt7570_LOS_level_set(void);				//by HC 20150304
void mt7570_ADC_calibration(void);
void mt7570_APD_initialization(void);		//by HC 20150327
void mt7570_APD_control(void);
void mt7570_load_init_current(void);			//by HC 20150129
void mt7570_load_MPDL_MPDH(void);		//by HC 20150129
UINT16 mt7570_temperature_get_8472(void);
UINT16 mt7570_supply_voltage_get_8472(void);
UINT16 mt7570_bias_current_get_8472(void);
short mt7570_TxVoltage_get(void);
short mt7570_RxRSSI_get(void);
UINT16 mt7570_TxPower_get_8472(void);		//by HC 20150123
UINT16 mt7570_RxPower_get_8472(void);		//by HC 20150127
short mt7570_RxPower_get(void);
void mt7570_RougeONU_clear(void);			//by HC 20150121
void mt7570_safe_circuit_reset(void);			//by HC 20150127
void mt7570_trans_model_setting(void);
void mt7570_tx_power_alarm_get(void);		// by HC 20150528
void mt7570_rx_power_alarm_get(void);		// by HC 20150528
int save_flash_matrix(void);
void flash_dump(void);							//by HC 20150128
void set_flash_register(uint reg, uint offset);
void set_flash_register_default(void);
int get_flash_matrix(void);
int get_flash_register(int address_offset);
int mt7570_information_output(uint select);
int mt7570_ADC_temperature_get(void);	// by HC 20150527
int mt7570_ADC_voltage_get(void);			// by HC 20150527
void mt7570_register_dump(int length);
int mt7570_MPD_current(void);
void mt7570_MPD_current_calibration(void);//by HC 20150117
int mt7570_RSSI_current(void);
int mt7570_dark_current(void);
void mt7570_RSSI_calibration(void);			// by HC 20150609
void GPON_Tx_calibration(void);				//by HC 20150209
void EPON_Tx_calibration(void);				//by HC 20150226
void mt7570_restart_ERC(void);				//by HC 20150210
int mt7570_EN7570_detection(void);			//by HC 20150301
void mt7570_ERC_filter(void);					//by HC 20150306
void mt7570_TIAGAIN_set(void);				//by HC 20150320
void set_flash_register_BiasModCurrent(void);							//by HC 20150309
void set_flash_register_P0P1(void);										//by HC 20150309
void set_flash_register_APD(uint slope_up, uint slope_dn, uint VAPD25);	//by HC 20150309
void set_flash_register_LOS(void);											//by HC 20150309
void set_flash_register_DDMI_TxPower(uint TxPower, uint offset);	//by HC 20150309
void set_flash_register_DDMI_RxPower(uint RxPower, uint offset);	//by HC 20150309
void set_flash_register_temperature_offset(uint input, uint slope);	//by Biker 20150714
void mt7570_APD_I2C_write(unchar ptr[1]);								//by HC 20150310
void DDMI_check_8472(void);												//by HC 20150311
void set_flash_register_TIAGAIN(void);									//by HC 20150320
int GPON_BER(int pattern);												//by HC 20150324
int EPON_BER(int pattern);												//by HC 20150324
void show_BoB_information(void);											//by HC 20150325
void mt7570_temperature_get(void);										//by HC 20150326
void mt7570_TxEyeCorrection(void);										//by HC 20150504
void mt7570_calibration_pattern(int input);							//by HC 20150518
void mt7570_disable_pattern(void);										//by HC 20150521
void mt7570_CDR(int CDR_switch);										//by HC 20150522
void mt7570_OpenLoopMode(void);										//by HC 20150602
void mt7570_DualClosedLoopMode(void);									//by HC 20150602
void mt7570_SingleClosedLoopMode(void);								//by HC 20160325
void mt7570_Vtemp_ADC_get(void);										//by YM 20150714
void mt7570_internal_clock(void);
void SOL_mt7570_ERTemperatureCompensation(void);
void mt7570_BiasTracking(void);											// by HC 20151014
void mt7570_FiberPlug_Protect(void);									// by HC 20151106
void mt7570_ERTemperatureCompensation(void);
void mt7570_restart_ERC_P0(void);
void mt7570_LUT_recover(void);											// by HC 20151014
void mt7570_sw_reset(void);												// by HC 20151014
void mt7570_RSSI_gain_init(void);										// by HC 20151014
void set_flash_register_T0T1delay(void);									// by HC 20151015
void set_flash_register_Tx_data(void);									// by HC 20151015
void set_flash_register_T0CT1C(void);									// by HC 20160204
void mt7570_OpenLoopControl(void);										// by HC 20160410
void mt7570_LOS_init(void);												// by HC 20160427
int i2c_access_protect(void);

#endif /* _PHY_API_H_ */

