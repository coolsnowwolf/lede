#ifndef _EN7571_DEF_H_
#define _EN7571_DEF_H_

#define temperature_offset					495.8		// ADC code for 0 degree
#define temperature_offset_efuse				3.44 		// eFuse offset
#define temperature_ADC_slop					327.5		// 1 digita code imply to 0.9422 degreel
#define TEMPERATURE_CELSIUS_TO_8472			256		// Transfer 'C to 8472 format
#define temperature_two_complement			65536		//by HC 20150327
#define temperature_two_complement_efuse		256		//by YW 20161121
#define default_BOSA_temperature_offset		5
#define voltage_8472_unit					0.0001	// 1 digital code imply to 100uV
#define default_voltage_slope				0.005474	//by HC 20150327
#define bias_current_slope					0.02442	// 1 digital code imply to 0.02442mA
#define bias_current_8472_slope				2			// 1 digital code imply to 2uA
#define mod_current_slope					0.02198	// 1 digital code imply to 0.02198mA
#define av_current_slope						0.02198	// 1 digital code imply to 0.02198mA
#define tia_mux_mask							0xf1		//by HC
#define TIA_MUX_TIASD						0x02
#define TIA_MUX_TIAFLT						0x08		//by HC
#define tia_sd_upper_mask					0xfe		//by HC
#define default_APD_zero_code_voltage		30.0		//30V when APD = 0x00
#define default_APD_voltage_step				0.09375	//voltage/code = 0.09375
#define APD_softstart_enable					0x20
#define APD_control_enable					0x01
#define ADC_select_temperature				0x08
#define ADC_select_BG_1V76					0x06
#define ADC_select_BG_0V875					0x4C
#define ADC_RSSI_enable						0x02
#define ADC_BG_0V875_mask					0xb3
#define ADC_select_mask						0xe1
#define ADC_latch							0x10
#define ADC_latch_mask						0xef
#define ADC_0V5								0xB7
#define ADC_RSSI_defend_noise_threshold		0x32
#define LOS_calibration_mask					0xfe
#define LOS_calibration_trig					0x01
#define LOS_ADCREV2_mask						0xfb
#define LOS_ADCREV2_enable					0x04
#define LOS_ADCREV1_mask						0xbf
#define LOS_ADCREV1_enable					0x40
#define LOS_cnt_mask							0x80
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
#define T1_T0_delay_mask						0x00
#define T1_T0_delay_setting1					0x66
#define T1_T0_delay_setting_GPON				0xaa
#define T1_T0_delay_setting_EPON				0x77
#define T1_timer_reset_mask					0x00
#define T1_timer_reset_value					0x7f
#define T0_timer_reset_mask					0x00
#define T0_timer_reset_value					0x7f
#define TGEN_reset_value_mask				0xdf
#define TGEN_reset_value_T1T0_timer			0x20
#define TGEN_method2_mask					0xfe
#define TGEN_method2_enable					0x01
#define LA_Rx_HighZ_mask						0xef
#define LA_Rx_HighZ_enable					0x10
#define RSSI_cal_en							0x10
#define RSSI_cal_mask						0xef
#define RSSI_gain_mask						0xf8
#define RSSI_GAIN_DEFAULT_SETTING			0x05
#define RSSI_IDEAL_IR						0.35
#define RogueONU_mask						0xfe			//by YMC

#define flash_empty							0xffffffff
#define flash_upper_16_bits					0xffff0000
#define flash_lower_16_bits					0x0000ffff
#define flash_MPD_ADC_mask					0x0000ffff	//by HC 20150331
#define flash_Tx_power_mask					0xffff0000
#define flash_Tx_power_offset				16
#define flash_Rx_power_mask					0xffff0000
#define flash_Rx_power_offset				16
#define flash_Rx_RSSI_current_mask			0x0000ffff	//by HC 20150127
#define flash_T0T1_delay_mask				0x000000ff
#define flash_T0C_mask						0x00ff0000
#define flash_T1C_mask						0xff000000
#define FLASH_PAV_MASK						0x0fff0000	// by HC 20170206
#define FLASH_P1_MASK						0x000003ff	// by HC 20170206
#define FLASH_IAV_MASK						0x0fff0000	// by HC 20170206
#define FLASH_IMOD_MASK						0x00000fff	// by HC 20170206
#define FLASH_ERC_MASK						0xff			// by HC 20170206
#define FLASH_RX_LOS_MASK					0x0000007f	// by HC 20170206
#define FLASH_RX_SD_MASK						0x007f0000	// by HC 20170206
#define FLASH_MAGIC_NUM_GPON					0x07050701	// by HC 20170207
#define FLASH_MAGIC_NUM_EPON					0xe7050701	// by HC 20170207

#define SELECT_PAV_CAL						0x01
#define SELECT_IAV_NOW						0x02
#define SELECT_IBIAS_NOW						0x03
#define SELECT_IMOD_NOW						0x04
#define SELECT_P1_CAL						0x06			//by HC 20150309
#define SELECT_PAV_NOW						0x07
#define SELECT_P1_NOW						0x08
#define LN10									2.302585093	// by HC 20170204
#define SWKT_IMOD_EPISODE					0x02e
#define SWKT_PERIOD							30

#define safe_circuit_mask					0xbf
#define safe_circuit_reset					0x40
#define pwr_ctrl_en_mask						0xfe		// by HC 20160613
#define pwr_ctrl_en_enable					0x01		// by HC 20160613
#define rg_pwr_ctrl_rst_b_enable				0x01
#define rg_pwr_ctrl_rst_b_mask				0xfe
#define pwr_flt_mpdx_shtbit_mask				0xf8		// by HC 20160613
#define pwr_flt_mpdx_shtbit_setting			0x07		// by HC 20160613
#define pwr_ctrl_pavg_shtbit_mask			0xf8		// by HC 20160613
#define PWR_CTRL_PAVG_SHTBIT_SETTING_64		0x03		// by HC 20160613
#define PWR_CTRL_PAVG_SHTBIT_SETTING_1024	0x07		// by HC 20160613
#define pwradc_adlch_sum_mask				0xfffff	// by HC 20160616
#define rg_pwradc_trig_mask					0x7f		// by HC 20160616
#define rg_pwradc_trig_trigger				0x80		// by HC 20160616
#define rg_pwradc_count_mask					0xf8		// by HC 20160616
#define rg_pwradc_count_64					0x03		// by HC 20160616
#define rg_pwradc_count_1024					0x07		// by HC 20160616
#define adlch_sum_mask						0xfffff	// by HC 20160616
#define rg_adlch_trig_mask					0x7f		// by HC 20160616
#define rg_adlch_trig_trigger				0x80		// by HC 20160616
#define rg_adlch_count_mask					0x8f		// by HC 20160616
#define rg_adlch_count_1024					0x70		// by HC 20160616
#define rg_pwr_ctrl_TimerPhz1_num_setting	0x007		// by HC 20170531
#define rg_pwr_ctrl_cmd_Iav_setting			0x300		// by HC 20160707
#define rg_pwr_ctrl_cmd_Imod_setting			0x200		// by HC 20160707
#define rg_pwr_ctrl_P1_intvl_setting			0x00		// by HC 20160707
#define rg_pwr_ctrl_Pav_intvl_setting		0x00		// by HC 20160707
#define rg_pwr_ctrl_stepmu_sel_mask 			0xfc		// by HC 20160707
#define rg_pwr_ctrl_stepmu_sel_setting 		0x02		// by HC 20160707
#define rg_pwr_ctrl_stepsizeX_mask			0xc0		// by HC 20160726
#define rg_pwr_ctrl_stepsize0_setting		0x3f		// by HC 20160707
#define rg_pwr_ctrl_stepsize1_setting		0x25		// by HC 20160707
#define rg_pwr_ctrl_stepsize2_setting		0x3f		// by HC 20160707
#define rg_pwr_ctrl_stepsize3_setting		0x25		// by HC 20160707
#define rg_pwr_ctrl_stepsize4_setting		0x3f		// by HC 20160707
#define rg_pwr_ctrl_stepsize5_setting		0x25		// by HC 20160707
#define rg_pwr_ctrl_MPDH_updatePd_mask		0xf8		// by HC 20161116
#define rg_pwr_ctrl_MPDH_updatePd_setting	0x01		// by HC 20161116
#define rg_pwr_ctrl_MPDH_updateTH_mask		0xf8		// by HC 20160709
#define rg_pwr_ctrl_MPDH_updateTH_setting	0x04		// by HC 20160707
#define rg_pwr_ctrl_delta_IavMax_setting		0x04		// by HC 20160707
#define rg_pwr_ctrl_delta_ImodMax_setting	0x02		// by HC 20160707
#define rg_pwr_ctrl_Imod_IavScale_mask		0x80		// by HC 20160726
#define rg_pwr_flt_P1_stepSize_mask			0xc0		// by HC 20160726
#define rg_pwr_flt_P1_stepSize_setting		0x02		// by HC 20160726
#define rg_pwr_ctrl_Pav_P1_cal_force_mask	0xfc		// by HC 20160726
#define rg_pwr_ctrl_Pav_P1_cal_force_enable	0x03		// by HC 20160726
#define rg_pwr_ctrl_ben_block_mask			0xfc		// by HC 20160726
#define rg_pwr_ctrl_ben_block_setting		0x01		// by HC 20160726
#define RG_PWRK1_mask						0xef		// by HC 20160726
#define RG_PWRK1_1							0x10		// by HC 20170126
#define RG_PWR_CTRL_IMOD_ADJ_SEL_MASK		0xfe		// by HC 20170207
#define RG_PWR_CTRL_IMOD_ADJ_SEL_KT			0x01		// by HC 20170207
#define RG_DUMMY_HWRESET_MASK				0xfe		// by HC 20170207
#define RG_DUMMY_HWRESET_SET					0x01		// by HC 20170207
#define RG_DUMMY_T1_EVENT_MASK				0xfc		// by HC 20170322
#define RG_DUMMY_T1_EVENT_REDUCE_SLIGHTLY	0x01		// by HC 20170322
#define RG_DUMMY_T1_EVENT_REDUCE_MORE		0x02		// by HC 20170322
#define RG_DUMMY_BURST_CTRL_MASK				0xfb		// by HC 20170608
#define RG_DUMMY_BURST_CTRL_OFF				0x04		// by HC 20170608

#define en7571_tx_power_low_alarm_thod		0x2710		//0dBm
#define en7571_tx_power_high_alarm_thod		0x8a99		//5.5dBm
#define en7571_tx_cur_low_alarm_thod			0x1f4		//1mA
#define en7571_tx_cur_high_alarm_thod		0xc350		//100mA
#define en7571_rx_power_low_alarm_thod		0xa			//-30dBm
#define en7571_rx_power_high_alarm_thod		0x9cf		//-6dBm
#define en7571_supply_volt_low_alarm_thod	0x7148		//2.9V
#define en7571_supply_volt_high_alarm_thod	0x9088		//3.7V
#define en7571_temperature_low_alarm_thod	0xfb00		//-5'C
#define en7571_temperature_high_alarm_thod	0x5500		//85'C

#define EN7571_IMOD_DAC            (0x0148) // P1_PWR_CTRL_CS2

#endif /* _EN7571_DEF_H_ */

