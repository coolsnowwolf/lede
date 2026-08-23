#ifndef _MT7570_DEF_H_
#define _MT7570_DEF_H_

#define flash_base_addr						0xbcff0600
#define temperature_offset					495.8	// ADC code for 0 degree
#define temperature_ADC_slop				327.5	// 1 digita code imply to 0.9422 degreel
#define temperature_mask					0x7fff
#define temperature_two_complement			65536	//by HC 20150327
#define temperature_minus					1
#define temperature_40						0x2800
#define default_BOSA_temperature_offset		5
#define voltage_8472_unit					0.0001	// 1 digital code imply to 100uV
#define voltahe_mask						0xef
#define voltahe_enable						0x10
#define voltage_1V45						0x212
#define default_voltage_slope				0.005474//by HC 20150327
#define bias_current_slope					0.02442	// 1 digital code imply to 0.02442mA
#define bias_current_8472_slope				2		// 1 digital code imply to 2uA
#define mod_current_slope					0.02198	// 1 digital code imply to 0.02198mA
#define ADC_select_SD_mask					0xef
#define ADC_select_SD_enable				0x10
#define ADC_select_inmux_mask				0xe1	//by HC
#define tia_mux_mask						0xf1	//by HC
#define tia_mux_select_SD					0x02
#define default_tia_mux_select				0x08	//by HC
#define tia_sd_upper_mask					0xfe	//by HC
#define APD_voltage_0						40		//40 voltage at 0 degree
#define default_APD_zero_code_voltage		30.0	//30V when APD = 0x00
#define default_APD_voltage_step			0.09375	//voltage/code = 0.09375
#define APD_offset							8
#define APD_mask							0xff
#define	APD_softstart_enable				0x20
#define	APD_control_enable					0x01
#define	ADC_select_temperature				0x08
#define ADC_select_BG_1V76					0x06
#define ADC_select_BG_0V875					0x4C
#define ADC_RSSI_enable						0x02
#define ADC_TxPW_enable						0x04
#define ADC_BG_0V875_mask					0xb3
#define	ADC_select_mask						0xe1
#define	ADC_latch							0x10
#define	ADC_latch_mask						0xef
#define ADC_0V75							0x100
#define ADC_0V5								0xB7
#define ADC_1V3								0x1DB
#define ADC_1V4								0x200
#define ADC_RSSI_defend_noise_threshold			0x32
#define LOS_calibration_mask				0xfe
#define LOS_calibration_trig				0x01
#define LOS_ADCREV2_mask					0xfb
#define LOS_ADCREV2_enable					0x04
#define LOS_ADCREV1_mask					0xbf
#define LOS_ADCREV1_enable					0x40
#define LOS_cnt_mask						0x80
#define LOS_cnt_setting						0x05					
#define LOS_confidence_mask					0xe0
#define LOS_confidence_setting				0x1f
#define LOS_comp_thld_mask					0x80
#define LOS_comp_thld_H_setting				0x30
#define LOS_comp_thld_L_setting				0x20
#define LOS_ain_stable_cnt_mask				0xe0
#define LOS_ain_stable_cnt_setting			0x1f
#define ERC_enable_mask						0xf7
#define ERC_enable							0x08
#define ERC_start_mask						0xfe
#define ERC_start							0x01
#define ERC_open_loop_mode_mask				0xfc
#define ERC_open_loop_mode					0x02
#define	T1_T0_delay_mask					0x00
#define	T1_T0_delay_setting1				0x66
#define	T1_T0_delay_setting2				0xaa
#define	T1_T0_delay_setting_GPON			0xaa
#define	T1_T0_delay_setting_EPON			0x77
#define	T1_timer_reset_mask					0x00
#define T1_timer_reset_value				0x7f
#define	T0_timer_reset_mask					0x00
#define T0_timer_reset_value				0x7f
#define TGEN_reset_value_mask				0xdf
#define TGEN_reset_value_T1T0_timer			0x20
#define TGEN_method2_mask					0xfe
#define TGEN_method2_enable					0x01
#define LA_Rx_HighZ_mask					0xef
#define LA_Rx_HighZ_enable					0x10
#define RSSI_cal_en							0x10
#define RSSI_cal_mask						0xef
#define RSSI_gain_mask						0xf8
#define RogueONU_mask						0xfe	//by YMC
#define flash_Ibias_mask					0xfff
#define flash_Imod_mask						0xfff
#define flash_P0_mask						0x3ff
#define	flash_P1_mask						0x3ff
#define	flash_Tx_MPD_current_mask			0x0000ffff	//by HC 20150331
#define	flash_Tx_power_mask					0xffff0000
#define	flash_temperature_slope_mask		0xffff0000	//by YM 20150714
#define	flash_temperature_offset_mask		0x0000ffff	//by YM 20150714
#define	flash_Tx_power_offset				16
#define	flash_temperature_slope_offset		16			//by YM 20150714
#define	flash_Rx_RSSI_mask					0x3ff
#define flash_Rx_gain_mask					0x7000
#define flash_Rx_gain_offset				12
#define	flash_Rx_power_mask					0xffff0000
#define	flash_Rx_power_offset				16
#define	flash_Rx_RSSI_current_mask			0x0000ffff	//by HC 20150127
#define	ADC_temperature						0x01
#define ADC_voltage							0x02
#define Ibias								0x03
#define Imod								0x04
#define P0									0x05		//by HC 20150309
#define P1									0x06		//by HC 20150309
#define safe_circuit_mask					0xbf
#define safe_circuit_reset					0x40
#define mt7570_tx_power_low_alarm_thod		0x2710		//0dBm
#define mt7570_tx_power_high_alarm_thod		0x8a99		//5.5dBm
#define	mt7570_tx_cur_low_alarm_thod		0x1f4		//1mA
#define	mt7570_tx_cur_high_alarm_thod		0xc350		//100mA
#define mt7570_rx_power_low_alarm_thod		0xa			//-30dBm
#define mt7570_rx_power_high_alarm_thod		0x9cf		//-6dBm
#define mt7570_supply_volt_low_alarm_thod	0x7148		//2.9V
#define mt7570_supply_volt_high_alarm_thod	0x9088		//3.7V
#define mt7570_temperature_low_alarm_thod	0xfb00		//-5'C
#define mt7570_temperature_high_alarm_thod	0x5500		//85'C
#define TX_8472_POWER_THRESHOLD_4dBm		25118		//by HC 20150226
#define TX_8472_POWER_THRESHOLD_3dBm		19952		//by HC 20150303
#define TX_8472_POWER_THRESHOLD_2dBm		15848		//by HC 20150303
#define TX_8472_POWER_THRESHOLD_3dBm5		22387		//by HC 20150303
#define TX_8472_POWER_THRESHOLD_0dBm5		11220		//by HC 20150303
#define TX_8472_POWER_THRESHOLD_1dBm		12589		//by HC 20150603

#endif /* _MT7570_DEF_H_ */

