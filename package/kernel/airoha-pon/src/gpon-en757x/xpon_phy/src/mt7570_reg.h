#ifndef _MT7570_REG_H_
#define _MT7570_REG_H_

//7570 RG
#define	mt7570_TIAMUX							(0x0000)
#define	mt7570_MPDH								(0x0004)
#define	mt7570_T1DELAY							(0x0008)
//#define	mt7570_T1ASD							(0x000C)
#define	mt7570_TIASD							(0x000C)
#define	mt7570_T0C								(0x0010)
#define	mt7570_LA_PWD							(0x0014)
#define	mt7570_LA_RSV2							(0x0018)
#define	mt7570_BGCKEN							(0x001C)
#define	mt7570_PI_TGEN							(0x0020)
#define	mt7570_SVADC_PD							(0x0024)
#define	mt7570_SVADC_REV3						(0x0028)
#define	mt7570_I2C_SCL_SMT						(0x002C)
#define	mt7570_APD_DAC_CODE						(0x0030)
#define	mt7570_SAFE_PROTECT						(0x0100)
#define	mt7570_IN_IMP_CAL_SEL					(0x0104)
#define	mt7570_IN_IMP_CAL_RPT					(0x0108)
#define	mt7570_IN_IMP_CAL_CTRL					(0x010C)
#define	mt7570_OUT_IMP_CAL_SEL					(0x0110)
#define	mt7570_OUT_IMP_CAL_RPT					(0x0114)
#define	mt7570_OUT_IMP_CAL_CTRL					(0x0118)
#define	mt7570_LOS_CTRL1						(0x011C)
#define	mt7570_LOS_CTRL2						(0x0120)
#define	mt7570_LOS_CAL_TIMER					(0x0124)
#define	mt7570_LOS_CAL_TIMEOUT_CNT				(0x0128)
#define	mt7570_LOS_CAL_TIMEOUT					(0x012C)
#define	mt7570_LOS_DBG_RG						(0x0130)
#define	mt7570_P0_PWR_CTRL_CS1					(0x0134)
#define	mt7570_P0_PWR_CTRL_CS2					(0x0138)
#define	mt7570_P0_PWR_CTRL_CS3					(0x013C)
#define	mt7570_P0_PWR_CTRL_LCH					(0x0140)
#define	mt7570_P1_PWR_CTRL_CS1					(0x0144)
#define	mt7570_P1_PWR_CTRL_CS2					(0x0148)
#define	mt7570_P1_PWR_CTRL_CS3					(0x014C)
#define	mt7570_P1_PWR_CTRL_LCH					(0x0150)
#define	mt7570_ADC_PROBE_STATUS					(0x0154)
#define	mt7570_PROBE_CONTROL					(0x0158)
#define	mt7570_DUMMY							(0x015C)
#define	mt7570_OVFL_DBG_CLR						(0x0160)
#define	mt7570_APD_OVP_LATCH					(0x0164)
#define	mt7570_ROGUE_ONU_DET_CTRL				(0x0168)
#define	mt7570_ERC_FILTER_CTRL					(0x016C)
#define	mt7570_FT_ADC_CLK_CLR					(0x0170)
#define	mt7570_SW_RESET							(0x0300)

//flash define
#define flash_Ibias_init							(0x000)
#define flash_Imod_init							(0x004)
#define	flash_P0_target							(0x008)
#define flash_P1_target							(0x00c)
#define	flash_APD_slope1						(0x010)
#define	flash_APD_slope2						(0x014)
#define	flash_APD_change_point					(0x018)
#define	flash_T_APD								(0x01c)	//by HC 20150302
#define	flash_LOS_high_thld						(0x020)
#define	flash_LOS_low_thld						(0x024)
#define	flash_TIAGAIN							(0x028)
#define	flash_internal_DDMI						(0x02c)	//by HC 20150302

// For brand new APD control
#define	flash_APD_voltage_1						(0x030)	// by HC 20151020
#define	flash_APD_voltage_2						(0x034)	// by HC 20151020
#define	flash_APD_voltage_step					(0x030)
#define	flash_APD_zero_code_voltage				(0x034)

#define	flash_voltage_slope						(0x038)
#define	flash_voltage_offset					(0x03c)
#define	flash_Tx_power_K_point1					(0x040)
#define	flash_Tx_power_K_point2					(0x044)
#define	flash_Tx_power_K_point3					(0x048)
#define	flash_Tx_power_K_point4					(0x04c)
#define	flash_Rx_power_K_point1					(0x050)
#define	flash_Rx_power_K_point2					(0x054)
#define	flash_Rx_power_K_point3					(0x058)
#define	flash_Rx_power_K_point4					(0x05c)
#define	flash_reserved_for_ETC1     				(0x060)
#define	flash_ETC_Hi_Lo_target_delta			(0x064)
#define	flash_reserved3							(0x068)
#define	flash_reserved4							(0x06c)
#define	flash_reserved1 						(0x070) 	// for single open loop
#define	flash_Ibias_SLope						(0x074) 	// for single open loop
#define	flash_P1_SLope							(0x078) 	// for single open loop
#define	flash_ETC								(0x07c)
#define	flash_temperature_K_slope_offset		(0x080) 	//by YM 20150714
#define	flash_temerature_offset					(0x084)	//by HC 20150326
#define	flash_MPD_point_up						(0x088)	//by HC 20150303
#define	flash_TEC								(0x08c)	//by HC 20150513
#define flash_reserved5							(0x090)	//by HC 20151016
#define flash_magic_number						(0x094)
#define flash_T0T1delay							(0x098)	//by HC 20151016
#define flash_T0CT1C								(0x09c)	//by HC 20151016
#endif /* _MT7570_REG_H_ */

