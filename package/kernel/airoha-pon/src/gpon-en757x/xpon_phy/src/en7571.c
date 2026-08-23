#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#include "i2c.h"
#include "phy_def.h"
#include "en7571_reg.h"
#include "en7571_def.h"
#include "phy_global.h"
#include "phy_api.h"
#include "en7571_api.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy.h"
#include "phy_init.h"
#include "phy_tx.h"

extern atomic_t i2c_protect;							// by HC 20160608
extern unsigned int en7571_delay1;				// by HC 20170531
extern unsigned int en7571_delay2;				// by HC 20170531

int en7571_version 			= 3;					//The version of en7571.c in main trunk by HC 20160608
int en7571_internal_DDMI 	= 1;					//by HC 20150311
int en7571_fast_DDMI		= 0;					//by HC 20160517
int en7571_xPON_mode 		= -1; 					// by HC 20150520
int en7571_select 			= 0;					//by HC 20150301
int en7571_RSSI_Vref 		= 0;					// by HC 20150609
int en7571_RSSI_V    			= 0;					// by HC 20150609
int en7571_PatternEnabled = 0;					// by HC 20151123
int en7571_DDMI_rx_cal_flag	= 0;				//biker_20151001
int en7571_ERC 				= 0;					// by HC 20161201
int en7571_KT 					= 0;					// by HC 20170321
int en7571_ver					= -1;					// by HC 20170524
int en7571_cross				= 1;					// by HC 20170531
int en7571_cross_cnt		= 0;					// by HC 20170531

float en7571_IC_temperature 		= 25.0; 	//by HC 20150326
float en7571_BOSA_temperature	= 20.0; 	//by HC 20150326
float en7571_APD_voltage    		= 35.0;	//by HC 20150326
float en7571_ADC_slope				= 0;		//by HC 20150603
float en7571_ADC_offset			= 0;		//by HC 20150603
float en7571_eFuse_temperature_offset = 0 ;	// by YW 20161214

UINT16 en7571_DDMI_voltage			= 0;		//by HC 20150311
UINT16 en7571_DDMI_current    		= 0;		//by HC 20150311
UINT16 en7571_DDMI_temperature		= 0;		//by HC 20150311
UINT16 en7571_DDMI_tx_power    		= 0;		//by HC 20150310
UINT16 en7571_DDMI_rx_power    		= 0;		//by HC 20150310

UINT32 en7571_temperature_ADC	= 0;
UINT32 en7571_voltage_ADC		= 0;
UINT32 en7571_RSSI_current	  	= 0;
UINT32 en7571_PWRADC	 	  		= 0;
UINT32 en7571_PWRADC_offset		= 0;	//by HC 20150117
UINT32 en7571_Tapd 					= 60;	//APD voltage update time. YMC 20150122
UINT32 en7571_alarm 				= 0;	//by YMC 20150414
UINT32 en7571_flash_matrix[100];

static unsigned long en7571_irq_flag = 0;
extern u32 GET_WAN_CONF(void);

void mbi_ctrl_set(u32 ctrl_val) {
	u32 mbi_ctrl_reg = 0;
	u32 mbi_ctrl = 0;

	u32 wan_cfg = GET_WAN_CONF();

	mbi_ctrl_reg = (1 == (wan_cfg&0x03)) ? 0x6000 : 0x4160;
	mbi_ctrl = get_xpon_data(mbi_ctrl_reg);
	if(0 == ctrl_val){
		mbi_ctrl |= (1 << 8);
		do { do { ({ unsigned long __dummy; typeof(en7571_irq_flag) __dummy2; (void)(&__dummy == &__dummy2); 1; }); en7571_irq_flag = arch_local_irq_save(); } while (0); } while (0);
		set_xpon_data(mbi_ctrl_reg, mbi_ctrl);
	}else{
		mbi_ctrl &= ~(1 << 8);
		set_xpon_data(mbi_ctrl_reg, mbi_ctrl);
		do { do { ({ unsigned long __dummy; typeof(en7571_irq_flag) __dummy2; (void)(&__dummy == &__dummy2); 1; }); arch_local_irq_restore(en7571_irq_flag); } while (0); } while (0);
	}
	set_xpon_data(mbi_ctrl_reg, mbi_ctrl);
}

/*****************************************************************************
//Function :
//		en7571_save_flash_APD
//Description :
//		This function is to set en7571_flash_matrix registers for APD
//Input :
//		slope up, slope down, and APD voltage at 25C
******************************************************************************/
void en7571_save_flash_APD(uint slope_up, uint slope_dn, uint VAPD)
{
	set_flash_register(slope_up, en7571_flash_APD_slope_up);
	set_flash_register(slope_dn, en7571_flash_APD_slope_dn);
	set_flash_register(VAPD    , en7571_flash_APD_change_point);
	printk("slope up = 0x%x\n"  , slope_up);
	printk("slope down = 0x%x\n", slope_dn);
	printk("VAPD@25C = 0x%x\n"  , VAPD    );
}
/*****************************************************************************
//Function :
//		en7571_save_flash_LOS
//Description :
//		This function is to set en7571_flash_matrix registers for LOS
******************************************************************************/
void en7571_save_flash_LOS(void)
{
	unchar ptr[4];
	uint temp = 0;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	ptr[3] = ptr[3] & 0x7f;	// Rx LOS threshold
	ptr[2] = ptr[2] & 0x7f;	// Rx SD threshold
	printk("LOS threshold = 0x%x\n", ptr[3]);
	printk("SD threshold = 0x%x\n", ptr[2]);
	temp = (ptr[2]<<16) | ptr[3];
	set_flash_register(temp, en7571_flash_LOS_thld);
}
/*****************************************************************************
//Function :
//		en7571_save_flash_DDMI_TxPower
//Description :
//		This function is to set en7571_flash_matrix registers for DDMI Tx Power
//Input :
//		Tx power (dBm) x100, FLASH address
******************************************************************************/
void en7571_save_flash_DDMI_TxPower(uint TxPower, uint offset)
{
	uint input = 0;
	input = ( (TxPower<<flash_Tx_power_offset)&flash_Tx_power_mask ) | (en7571_PWRADC_get()&flash_MPD_ADC_mask);		//by HC 20150331
	set_flash_register(input, offset);

	printk("0x%08x\n", input);
}
/*****************************************************************************
//Function :
//		en7571_save_flash_DDMI_RxPower
//Description :
//		This function is to set en7571_flash_matrix registers for DDMI Tx Power
//Input :
//		Rx power (uW) x100, FLASH address
******************************************************************************/
void en7571_save_flash_DDMI_RxPower(uint RxPower, uint offset)
{
	uint input = 0;
	input = ( (RxPower<<16)&flash_Rx_power_mask ) | (en7571_RSSI_get()&flash_Rx_RSSI_current_mask);		//by HC 20150331
	set_flash_register(input, offset);
	printk("0x%08x\n", input);
	en7571_DDMI_rx_cal_flag = 0;		//biker_20151001

}


/*****************************************************************************
//Function :
//		sen7571_save_flash_Tx_data
//Description :
//		This function is to set en7571_flash_matrix registers for Tx data
******************************************************************************/
void en7571_save_flash_Tx_data(void)
{
	unchar ptr[4];
	UINT32 imod  	= 0;
	UINT32 iav 	= 0;
	UINT32 write_data 	= 0;
	UINT32 pav 	= 0;
	UINT32 p1 		= 0;

	// Iav and Imod
	imod = en7571_info(SELECT_IMOD_NOW );
	iav = en7571_info(SELECT_IAV_NOW);
	printk("Ibias = 0x%x\n", en7571_info(SELECT_IBIAS_NOW));
	printk("Imod = 0x%x\n" , imod );
	printk("Iav = 0x%x\n" , iav );
	write_data = (iav<<16) | imod;
	set_flash_register( write_data, en7571_flash_Iav_Imod);

	write_data = 0;

	// Save Pav and P1
	p1 = en7571_info(SELECT_P1_CAL);
	pav = en7571_info(SELECT_PAV_CAL);
	printk("P1 = 0x%x\n" , p1 );
	printk("Pav = 0x%x\n" , pav );
	write_data = (pav<<16) | p1;
	set_flash_register(write_data, en7571_flash_Pav_P1);

	write_data = 0;

	// T0C and T1C
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	ptr[1] = (ptr[1] & T1_timer_reset_value);	// T1C
	ptr[2] = (ptr[2] & T0_timer_reset_value);	// T0C
	printk("T0C = 0x%x\nT1C = 0x%x\n", ptr[2], ptr[1]);
	write_data = ( (ptr[1]<<8) | ptr[2] ) << 16;
	// T0 and T1 delay
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	printk("T0/T1 delay = 0x%x\n", ptr[0]);
	write_data = write_data | (ptr[0]&flash_T0T1_delay_mask);
	set_flash_register( write_data, en7571_flash_T0CT1C);

}


/*****************************************************************************
//Function :
//		en7571_xPON_Tx_calibration
//Description :
//		This function is to initialize GPON Tx calibration
//Input :
//		GPON/EPON
******************************************************************************/
void en7571_xPON_Tx_calibration(int input)
{
	en7571_ERC 	= 0; // Disable ERC
	en7571_KT 	= 0; // Disable SWKT

	// GPON mode by YW 20160422
	phy_mode_config(input, PHY_ENABLE);

	// Clear T0/T1 delay and T0C/T1C
	set_flash_register(flash_empty, en7571_flash_T0CT1C);

	//T_GEN
	en7571_TGEN(input);

	// Set EN7571's registers
	en7571_reg_init();

	//Calibration pattern
	en7571_calibration_pattern(input);

	// Switch to force mode calibration
	en7571_cal_force_mode();
}


/*****************************************************************************
//function :
//		en7571_calibration_pattern
//description :
//		This function is to send xPON calibration pattern
//input :
//		1 : GPON PRBS23
//		0 : EPON PRBS7
******************************************************************************/
void en7571_calibration_pattern(int input)
{
	// Disable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_START);
	mdelay (500);

	// Disable CDR
	en7571_CDR(DISABLE);

	en7571_DCL_stop();
	en7571_link_reg(DISABLE);
	en7571_DCL_start();

	if( input == PHY_GPON_CONFIG )
	{
		phy_tx_test_pattern(PHY_BIST_PRBS23);
		phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);
		printk("PRBS23 enabled\n");
	}
	else if( input == PHY_EPON_CONFIG )
	{
		phy_tx_test_pattern(PHY_BIST_PRBS7);
		phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);
		printk("PRBS7 enabled\n");
	}

	//Clear Tx_fault
	en7571_safe_circuit_reset();

	en7571_PatternEnabled = 1;
}


/*****************************************************************************
//function :
//		en7571_disable_pattern
//description :
//		This function is to disable xPON calibration pattern
******************************************************************************/
void en7571_disable_pattern(void)
{
	phy_tx_test_pattern(PHY_BIST_IDLE);

	// Enable CDR
	en7571_CDR(ENABLE);

	en7571_DCL_stop();
	en7571_link_reg(ENABLE);
	en7571_DCL_start();

	// Enable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_STOP);

	//Clear Tx_fault
	en7571_safe_circuit_reset();

	printk("Pattern disabled\n");
	en7571_PatternEnabled = 0;
}

/*****************************************************************************
//Function :
//		en7571_CDR
//Description :
//		This function is to enable/disable CDR in 752X
//Input :
//		1: Enable
//		0: Disable
******************************************************************************/
void en7571_CDR(int CDR_switch)
{
	uint read_data  = 0;

	if( CDR_switch == DISABLE )
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET1);
		IO_SPHYREG(PHY_CSR_PHYSET1, (read_data | PHY_TX_LOCK_2_REF));
	}
	else if( CDR_switch == ENABLE )
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET1);
		IO_SPHYREG(PHY_CSR_PHYSET1, (read_data & PHY_TX_LOCK_2_DATA));
	}
}

/*****************************************************************************
//function :
//		en7571_init
//description :
//		this function is used to initialize Tx and Rx of EN7571
//input :
//		N/A
//output :
//		0 : the system is ready
//
******************************************************************************/
int en7571_init(void)
{
    i2c_access_queue = create_workqueue("i2c_access_queue");
	INIT_WORK(&gpPhyPriv->EN7570_reset, en7571_safe_circuit_reset_for_work_queue);  //dyma_20170117

	// Get FLASH
	if(get_flash_matrix() == 0)			//Get FLASH successfuly
		printk("FLASH matrix got\n");
	else											//Get FLASH fail
		set_flash_register_default();		//Reset FLASH to default


	//SW reset
	en7571_sw_reset();

	// HW reset
	en7571_hw_reset();

	// Switch to 7571 mode
	en7571_7571_enable();

	// Get PWRADC offset
	en7571_PWRADC_calibration();

	// Enable PWRADC
	en7571_PWRADC_enable();

	// ADC calibration															// This shouldn't be placed too close after "en7571_sw_reset"
	en7571_ADC_calibration();												// by HC 20150603

	// RSSI calibration
	en7571_RSSI_calibration();											// by HC 20150609

	// Get Efuse temperature offset value
	en7571_eFuse_temperature_get();

	// Polarity setting for EN7571
	IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x10f);

	//If magic number is 0x07050701, run GPON initialization
	if( get_flash_register(en7571_flash_magic_number) == FLASH_MAGIC_NUM_GPON )
	{
		printk("Start GPON Tx Calibration\n");
		phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);				//GPON mode by HC 20150513
		en7571_xPON_mode = PHY_GPON_CONFIG; 									// xPON mode by HC 20150520

		// TGEN
		en7571_TGEN(PHY_GPON_CONFIG);

		//APD initialization
		en7571_APD_initialization();									// Moved by HC 20150514
		if( get_flash_register(en7571_flash_T_APD) != flash_empty )		//by HC
		{
			en7571_Tapd = get_flash_register(en7571_flash_T_APD);
			if( en7571_Tapd < 10 )													//Check en7571_Tapd >= 10 by YMC 20150125
			{
				printk("APD update period is too short, reset to 10s\n");
				en7571_Tapd=10;
			}
		}
		en7571_APD_control();
	}
	//If magic number is 0xe7050701, run EPON initialization
	else if( get_flash_register(en7571_flash_magic_number) == FLASH_MAGIC_NUM_EPON )									//EPON by HC
	{
		printk("Start EPON Tx Calibration\n");
		phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);					//EPON mode by HC
		en7571_xPON_mode = PHY_EPON_CONFIG; 										// xPON mode by HC 20150520

		// TGEN
		en7571_TGEN(PHY_EPON_CONFIG);

	}
	//If magic number is wrong, return.
	else
	{
		en7571_LOS_init();
		en7571_internal_DDMI = 0;												// by HC 20150218
		printk("Internal DDMI Disabled\n");
		return 0;
	}

	// Set EN7571's registers
	en7571_reg_init();

	// Load Tx calibrated data
	en7571_load_Tx_cal_data();

	//Set Tx_SD
	en7571_TxSD_level_set();

	//RSSI gain initialization
	en7571_RSSI_gain_init();

	//Rx LOS
	en7571_LOS_level_set();

	//Clear Rogue ONU															// by YMC 20150125
	en7571_RougeONU_clear();

	//Reset safe_circuit 														// by YMC 20150125
	en7571_safe_circuit_reset();

	// reg init for link
	en7571_link_reg(ENABLE);

	// Enable E3 dual-closed loop
	en7571_DCL_start();

	// DDMI, KT, and ERC
	en7571_config();															// by HC 20170329

	printk("EN7571 Initialization Done!\n");
	return 0 ;
}


/*****************************************************************************
//Function :
//		en7571_HWKT
//Description :
//		This function is to enable KT function
//Input :
//		ENABLE/DISABLE
******************************************************************************/
void en7571_HWKT(int input)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_2, ptr, 3);

	if( input == ENABLE )
		ptr[0] = (ptr[0] & RG_PWR_CTRL_IMOD_ADJ_SEL_MASK) | RG_PWR_CTRL_IMOD_ADJ_SEL_KT;		// rg_pwr_ctrl_e3_Imod_Adj_sel = 1
	else if( input == DISABLE )
		ptr[0] = (ptr[0] & RG_PWR_CTRL_IMOD_ADJ_SEL_MASK);												// rg_pwr_ctrl_e3_Imod_Adj_sel = 0

	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_2, ptr, 3);
}


/*****************************************************************************
//Function :
//		en7571_hw_reset
//Description :
//		This function is to reset 7571 for operation
******************************************************************************/
void en7571_hw_reset(void)
{
	unchar ptr[4];

	// Reset
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);
	ptr[1] = ptr[1] & RG_DUMMY_HWRESET_MASK;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);
	// Set
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);
	ptr[1] = (ptr[1]&RG_DUMMY_HWRESET_MASK) | RG_DUMMY_HWRESET_SET;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);

}


/*****************************************************************************
//Function :
//		en7571_reg_init
//Description :
//		This function is to set registers of 7571 for operation
******************************************************************************/
void en7571_reg_init(void)
{
	unchar ptr[4];
	UINT32 Ibias_min		= 0x080;
	UINT32 Imod_min 		= 0x032;
	UINT32 Imax			= 0xfff;

	// rg_DA_IBIAS_min
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_LIMITER_0, ptr, 4);
	ptr[3] = (ptr[3] & 0xf0) | (Ibias_min >> 8);
	ptr[2] = (ptr[2] & 0x00) | (Ibias_min & 0xff);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_LIMITER_0, ptr, 4);

	// rg_DA_IMOD_min
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_LIMITER_2, ptr, 4);
	ptr[3] = (ptr[3] & 0xf0) | (Imod_min >> 8);;
	ptr[2] = (ptr[2] & 0x00) | (Imod_min & 0xff);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_LIMITER_2, ptr, 4);


	ptr[3] = (Imax >> 8);
	ptr[2] = (Imax & 0xff);
	ptr[1] = 0x00;
	ptr[0] = 0x00;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_6, ptr, 4);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_7, ptr, 4);


	// Initial Iav and Imod
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4);
	// init Imod
	ptr[3] = (ptr[3] & 0xf0) | (rg_pwr_ctrl_cmd_Imod_setting >> 8);
	ptr[2] = (rg_pwr_ctrl_cmd_Imod_setting & 0xff);
	// Init Iav
	ptr[1] = (ptr[1] & 0xf0) | (rg_pwr_ctrl_cmd_Iav_setting >> 8);
	ptr[0] = (rg_pwr_ctrl_cmd_Iav_setting & 0xff);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4);


	// 	rg_pwr_ctrl_e3_stepmu_sel
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);
	ptr[0] = (ptr[0] & rg_pwr_ctrl_stepmu_sel_mask) | rg_pwr_ctrl_stepmu_sel_setting;
	// 	rg_pwr_ctrl_e3_stepsize0, rg_pwr_ctrl_e3_stepsize1, rg_pwr_ctrl_e3_stepsize2
	ptr[3] = (ptr[3] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize2_setting;	// stepsize2 is step size #1 of Pav
	ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize1_setting;	// stepsize1 is step size #0 of MPDH
	ptr[1] = (ptr[1] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize0_setting;	// stepsize0 is step size #0 of Pav
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);

	// 	rg_pwr_ctrl_e3_stepsize3, rg_pwr_ctrl_e3_stepsize4, rg_pwr_ctrl_e3_stepsize5
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);
	ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize5_setting;	// stepsize5 is step size #2 of MPDH
	ptr[1] = (ptr[1] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize4_setting;	// stepsize4 is step size #2 of Pav
	ptr[0] = (ptr[0] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize3_setting;	// stepsize3 is step size #1 of MPDH
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);


	// Set variation of Iav and Imod, and set MPDH_updatePd
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
	ptr[3] = (ptr[3] & rg_pwr_ctrl_MPDH_updatePd_mask) | rg_pwr_ctrl_MPDH_updatePd_setting;	// MPDH_updatePd
	ptr[1] = rg_pwr_ctrl_delta_ImodMax_setting;
	ptr[0] = rg_pwr_ctrl_delta_IavMax_setting;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
#if 0
	// Auto-locked mode
	en7571_auto_lock_mode();
#else
	// Force mode
	en7571_force_mode();
#endif
	// Reduce phase 1/2 timing
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_5, ptr, 4);
	ptr[1] = (ptr[1] & 0xfc) | (rg_pwr_ctrl_TimerPhz1_num_setting >> 8);
	ptr[0] = (rg_pwr_ctrl_TimerPhz1_num_setting & 0xff);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_5, ptr, 4);


	// Setting of MPDH
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_1, ptr, 4);
	ptr[3] = (ptr[3] & rg_pwr_flt_P1_stepSize_mask) | rg_pwr_flt_P1_stepSize_setting;		// MPDH step size
	ptr[0] = (ptr[0] & pwr_flt_mpdx_shtbit_mask) | pwr_flt_mpdx_shtbit_setting;				// MPDH average times
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_1, ptr, 4);

	// PWRADC average times
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_E, ptr, 4);
	ptr[3] = (ptr[3] & pwr_ctrl_pavg_shtbit_mask) | PWR_CTRL_PAVG_SHTBIT_SETTING_64;		// PWRADC average times
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_E, ptr, 4);

	// Adjustment limiter
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_B, ptr, 4);
	ptr[3] = rg_pwr_ctrl_P1_intvl_setting;	// If delta_P1 > this prmt, change Imod
	ptr[2] = rg_pwr_ctrl_Pav_intvl_setting;	// If delta_Pav > this prmt, change Iav
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_B, ptr, 4);

	// Disable Dr. Chuang's TC
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_A, ptr, 4);
	ptr[2] = ptr[2] & rg_pwr_ctrl_Imod_IavScale_mask;		// rg_pwr_ctrl_e3_Imod_IavScale = 0
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_A, ptr, 4);

}


/*****************************************************************************
//Function :
//		en7571_mpdh_stepsize
//Description :
//		This function is to initialize reg for xPON
******************************************************************************/
void en7571_mpdh_stepsize(int input)
{
	unchar ptr[4];

	if( input == DISABLE )
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);
		ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask);// | rg_pwr_ctrl_stepsize1_setting; // stepsize1 is step size #0 of MPDH
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);
		ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask);// | rg_pwr_ctrl_stepsize5_setting; // stepsize5 is step size #2 of MPDH
		ptr[0] = (ptr[0] & rg_pwr_ctrl_stepsizeX_mask);// | rg_pwr_ctrl_stepsize3_setting; // stepsize3 is step size #1 of MPDH
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);
	}
	else if( input == ENABLE )
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);
		ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize1_setting; // stepsize1 is step size #0 of MPDH
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_3, ptr, 4);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);
		ptr[2] = (ptr[2] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize5_setting; // stepsize5 is step size #2 of MPDH
		ptr[0] = (ptr[0] & rg_pwr_ctrl_stepsizeX_mask) | rg_pwr_ctrl_stepsize3_setting; // stepsize3 is step size #1 of MPDH
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_4, ptr, 4);
	}
}


/*****************************************************************************
//Function :
//		en7571_7571_enable
//Description :
//		This function is to switch to 7571 mode
******************************************************************************/
void en7571_7571_enable(void)
{
	unchar ptr[4];

	// Switch to 7571 mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
	ptr[0] = (ptr[0] & pwr_ctrl_en_mask) | pwr_ctrl_en_enable;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
}


/*****************************************************************************
//Function :
//		en7571_DCL_start
//Description :
//		This function is to start 7571 dual-closed loop
******************************************************************************/
void en7571_DCL_start(void)
{
	unchar ptr[4];

	// Enable E3 dual-closed loop
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
	ptr[1] = (ptr[1] & rg_pwr_ctrl_rst_b_mask) | rg_pwr_ctrl_rst_b_enable;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
}


/*****************************************************************************
//Function :
//		en7571_DCL_stop
//Description :
//		This function is to stop 7571 dual-closed loop
******************************************************************************/
void en7571_DCL_stop(void)
{
	unchar ptr[4];

	// Disable E3 dual-closed loop
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
	ptr[1] = ptr[1] & rg_pwr_ctrl_rst_b_mask;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_0, ptr, 4);
}


/*****************************************************************************

******************************************************************************/
void en7571_cal_Iav_Imod(UINT32 input, int I)
{
	unchar ptr[4];
	UINT32 keep_pav = en7571_info(SELECT_PAV_CAL);		// To keep optical Tx power

	en7571_DCL_stop();

	en7571_auto_lock_mode();

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4 );
	if( I == 1 )			// Tune Iav
	{
		ptr[0] = input;
		ptr[1] = (input >> 8);
		ptr[2] = rg_pwr_ctrl_cmd_Imod_setting&0xff;
		ptr[3] = (rg_pwr_ctrl_cmd_Imod_setting >> 8);
	}
	else if( I == 0 )	// Tune Imod
	{
		ptr[2] = input;
		ptr[3] = (input >> 8);
	}
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4 );

	en7571_hw_reset();	// Necessary, or 0x2a4 will not be stable, this will make 0x2a4 ready soon.
	mdelay(200);				// Necessary, or 0x2a4 will not be stable, this will make 0x2a4 ready soon.

	en7571_DCL_start();

	if( I == 0 )			// For ER calibration, keep Tx power fixed.
	{
		// Change Pav
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_0, ptr, 4 );
		ptr[0] = keep_pav;
		ptr[1] = (keep_pav >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_PWR_CTRL_D, ptr, 4 );

		en7571_force_mode();
	}

}


/*****************************************************************************

******************************************************************************/
void en7571_cal_Pav_P1(UINT32 input, int P)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4 );
	if( P == 1 )			// Tune Pav
	{
		ptr[0] = input;
		ptr[1] = (input >> 8);
	}
	else if( P == 0 )	// Tune P1
	{
		ptr[2] = input;
		ptr[3] = (input >> 8);
	}
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4 );
}


/*****************************************************************************

******************************************************************************/
void en7571_cal_force_mode(void)
{
	unchar ptr[4];

	en7571_DCL_stop();

	// Stop closed loop
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
	ptr[1] = 0x00;
	ptr[0] = 0x00;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
#if 0
	en7571_force_mode();
#endif
	en7571_hw_reset();	// Necessary, or 0x2a4 will not be stable, this will make 0x2a4 ready soon.

	en7571_DCL_start();

	mdelay(200);				// Necessary, or 0x2a4 will not be stable, this will make 0x2a4 ready soon.

	// Copy 0x2a4 to 0x25c
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_0, ptr, 4 );
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_PWR_CTRL_D, ptr, 4 );

	// Closed loop
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
	ptr[1] = rg_pwr_ctrl_delta_ImodMax_setting;
	ptr[0] = rg_pwr_ctrl_delta_IavMax_setting;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);

}


/*****************************************************************************
//Function :
//		en7571_sw_reset
//Description :
//		This function is reset all registers of 7571
******************************************************************************/
void en7571_sw_reset(void)
{
	unchar ptr[4];

	//SW reset
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SW_RESET, ptr, 4);
	ptr[0] = (ptr[0] & 0xf8) | (0x01);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SW_RESET, ptr, 4);
}


/*****************************************************************************
//Function :
//		en7571_RSSI_gain_init
//Description :
//		This function is to initialize RSSI gain
******************************************************************************/
void en7571_RSSI_gain_init(void)
{
	unchar ptr[4];

	//RSSI gain initialization
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
	ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN_DEFAULT_SETTING;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
}



/*****************************************************************************
//Function :
//		en7571_TGEN
//Description :
//		This function is to do EN7571 TGEN
//Input :
//		1: GPON
//		0: EPON
******************************************************************************/
void en7571_TGEN(int xPON)
{
	unchar ptr[4];
	UINT8 RGS_T0C = 0;
	UINT8 RGS_T1C = 0;
	int i = 0;

	//Disable CDR
	en7571_CDR(DISABLE);

	//752X sends PRBS23
	phy_tx_test_pattern(PHY_BIST_PRBS23);


	mdelay(10);

	for( i=0; i<32; i++ )		// Get MAX T0C/T1C
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & ERC_enable_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[0] = (ptr[0] & T1_T0_delay_mask) | (T1_T0_delay_setting1);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[1] = (ptr[1] & T1_timer_reset_mask) | (T1_timer_reset_value);
		ptr[2] = (ptr[2] & T0_timer_reset_mask) | (T0_timer_reset_value);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_reset_value_mask) | (TGEN_reset_value_T1T0_timer);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_reset_value_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_method2_mask) | (TGEN_method2_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_method2_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T0C, ptr, 4);
		if( ptr[0] > RGS_T0C )
			RGS_T0C = ptr[0];
		if( ptr[1] > RGS_T1C )
			RGS_T1C = ptr[1];

	}

	mdelay(10);

	// Set T0T1delay
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_T1DELAY, ptr, 4);	// by HC 20151015
	if( xPON == PHY_GPON_CONFIG )
	{
		ptr[0] = (ptr[0] & T1_T0_delay_mask) | T1_T0_delay_setting_GPON;
	}
	else if( xPON == PHY_EPON_CONFIG )
	{
			ptr[0] = (ptr[0] & T1_T0_delay_mask) | T1_T0_delay_setting_EPON;
	}
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_T1DELAY, ptr, 4 );

	// Get fixed T0C/T1C
	if( get_flash_register(en7571_flash_T0CT1C) != flash_empty)
	{
		RGS_T0C = (get_flash_register(en7571_flash_T0CT1C) & flash_T0C_mask)>>16;
		RGS_T1C = (get_flash_register(en7571_flash_T0CT1C) & flash_T1C_mask)>>24;
	}
	printk("RGS_T0C = 0x%x\nRGS_T1C = 0x%x\n", RGS_T0C, RGS_T1C);
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	ptr[1] = (ptr[1] & T1_timer_reset_mask) | RGS_T1C;
	ptr[2] = (ptr[2] & T0_timer_reset_mask) | RGS_T0C;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & TGEN_reset_value_mask) | (TGEN_reset_value_T1T0_timer);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & TGEN_reset_value_mask);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & ERC_enable_mask) | (ERC_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_T1DELAY, ptr, 4 );

	//Enable CDR
	en7571_CDR(ENABLE);

	//752X back to normal mode
	phy_tx_test_pattern(PHY_BIST_IDLE);

}

/*****************************************************************************
//function :
//		en7571_LOS_level_set
//description :
//		this function is used to set LOS level
******************************************************************************/
void en7571_LOS_level_set(void)
{
	unchar ptr[4];

	en7571_LOS_init();

	if( get_flash_register(en7571_flash_LOS_thld) != flash_empty )
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
		ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | (get_flash_register(en7571_flash_LOS_thld)&FLASH_RX_LOS_MASK);	//Fix coding mistake YMC 20150128
		ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | ( (get_flash_register(en7571_flash_LOS_thld)&FLASH_RX_SD_MASK) >> 16 );	//Fix coding mistake YMC 20150128
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	}
	else
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
		ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | (LOS_comp_thld_L_setting);
		ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | (LOS_comp_thld_H_setting);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	}
	printk("Rx LOS is set\n");
}


/*****************************************************************************
//function :
//		en7571_TxSD_level_set
//description :
//		this function is used to set Tx SD level
******************************************************************************/
void en7571_TxSD_level_set(void)
{
	int tiaflt_A = 0;
	UINT32 txsd_offset = 0;
	int tiasd_B = 0;
	int read_Pav_D = 0;
	UINT32 tia_sd = 0;
	UINT8 tia_mux_default = 0;
	unchar ptr[4];

	txsd_offset = (en7571_PWRADC_offset >> 6);

	// TIAMUX to b'100
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);
	tia_mux_default = ptr[0];		// Keep default
	ptr[0] = (ptr[0] & tia_mux_mask) | TIA_MUX_TIAFLT;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);

	//Read PWRADC offset, as A
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_RG_PWRADC_DATA2+(0x03)), ptr, 1);
	ptr[0] = (ptr[0] & 0xff ) | 0x80;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_RG_PWRADC_DATA2+(0x03)), ptr, 1);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA, ptr, 2);
	tiaflt_A = ((ptr[1]<<8) | ptr[0]) & (0x3ff);
	tiaflt_A = tiaflt_A - txsd_offset ;

	// Set TIAMUX to b'001
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);
	ptr[0] = (ptr[0] & tia_mux_mask ) | TIA_MUX_TIASD;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);

	// Set TIASD to b'000
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIASD, ptr, 2);
	ptr[0] = (ptr[0] & 0x00);
	ptr[1] = (ptr[1] & tia_sd_upper_mask);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIASD, ptr, 2);

	//Read PWRADC offset, as B
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_RG_PWRADC_DATA2+(0x03)), ptr, 1);
	ptr[0] = (ptr[0] & 0xff ) | 0x80;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_RG_PWRADC_DATA2+(0x03)), ptr, 1);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA, ptr, 2);
	tiasd_B = ((ptr[1]<<8) | ptr[0]) & (0x3ff);
	tiasd_B= tiasd_B - txsd_offset;

	//get Pav from flash, as D
	read_Pav_D = ( (get_flash_register(en7571_flash_Pav_P1)&FLASH_PAV_MASK)>>18 ) - txsd_offset;

	//set new offet to RG_TIASD
	tia_sd = 0.1*2.8/1.8*read_Pav_D + (2.8/1.8)*(tiaflt_A - tiasd_B) + 6 ;		// Set TIASD = 0.1*2.8/1.8*D + (2.8/1.8)*(A-B)+6
	printk("TIASD = 0x%x\n", tia_sd);
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIASD, ptr, 2);
	ptr[0] = tia_sd & 0xff;
	ptr[1] = (tia_sd >> 8) & 0x01;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_TIASD, ptr, 2 );

	// Set RG_TIAMUX to default
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);
	ptr[0] = tia_mux_default;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);
}


/*****************************************************************************
//Function :
//		en7571_SVADC_get
//Description :
//		This function is to latch and get SVADC
//Input :
//		N/A
//Output :
//		Value of SVADC
//Date:
//		2017/02/03 by HC
******************************************************************************/
UINT32 en7571_SVADC_get(void)
{
	unchar ptr[2];
	UINT32 svadc_value = 0;

	// Latch SVADC
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_PROBE_CONTROL + 0x001), ptr, 1);
	ptr[0] = (ptr[0]&ADC_latch_mask) | ADC_latch;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_PROBE_CONTROL + 0x001), ptr, 1);

	// Read SVADC
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_ADC_PROBE_STATUS, ptr, 2);
	svadc_value = (ptr[0] | (ptr[1]<<8));

	return svadc_value;
}

/*****************************************************************************
//Function :
//		en7571_ADC_calibration
//Description :
//		This function is to calibrate ADC
******************************************************************************/
void en7571_ADC_calibration(void)
{
	int BG1V76    			= 0;
	int BG0V875   		= 0;

	unchar ptr[2];
	uint read_data = 0;
	uint cnt = 8;
	uint i = 0;
	uint temp = 0;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		// Set ADC to BG 1.76V
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD , ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_select_BG_1V76;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

	    for(i=0;i<cnt;i++)
		{
			temp += en7571_SVADC_get();
		}

		// Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		BG1V76 = ((temp>>2)+1)>>1;
		temp = 0;		// Set temp to 0

		// Set ADC to BG 0.875V
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_SVADC_PD + 0x01) , ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & (ADC_BG_0V875_mask))|(ADC_select_BG_0V875);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_SVADC_PD + 0x01), ptr, 1);

		for(i=0;i<cnt;i++)
		{
			temp += en7571_SVADC_get();
		}

		//Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (en7571_SVADC_PD + 0x01), ptr, 1);

		atomic_set(&i2c_protect, 0);

		BG0V875 = ((temp>>2)+1)>>1;
		temp = 0;		// Set temp to 0

		printk("BG 1.76V = 0x%x\nBG 0.875V = 0x%x\n", BG1V76, BG0V875);

		if( (BG1V76-BG0V875) <= 0 )		// Error
		{
			printk("ADC calibration fail\n");
			return;
		}

		en7571_ADC_slope  = (1.76-0.875)/(float)(BG1V76-BG0V875);
		en7571_ADC_offset = 1.76 - en7571_ADC_slope*(float)BG1V76;
	}
}
/*****************************************************************************
//Function :
//		en7571_APD_initialization
//Description :
//		This function is to initialize APD voltage
******************************************************************************/
void en7571_APD_initialization(void)
{
	unchar ptr[4];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 4);
	ptr[2] = ptr[2]|(APD_softstart_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 4);
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 4);
	ptr[1] = ptr[1]|(APD_control_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 4);
	printk("APD initialization done\n");
}
/*****************************************************************************
//Function :
//		en7571_APD_control
//Description :
//		This function is used to set APD voltage
******************************************************************************/
void en7571_APD_control(void)
{

		// A brand new structure by HC 20151020
		unchar ptr[1];
		int a = 0;
		int b = 0;

		float APD_slope_up   = 0.10;		//Default by HC
		float APD_slope_dn   = 0.07;		//Default by HC
		float APD_voltage_NT = 35.0;		//Default by HC
		float APD_voltage_step = default_APD_voltage_step;			//Default by HC 20150303
		float APD_voltage_0x00 = default_APD_zero_code_voltage;	//Default by HC 20150303
		float APD_voltage_0x40 = 36.0;			//Default by HC 20150303
		float APD_voltage_0x80 = 42.0;			//Default by HC 20150303
		float APD_voltage_0xC0 = 45.6;			//Default by HC 20150303

		//Load slope and voltage from FLASH
		if(get_flash_register(en7571_flash_APD_slope_up)!=flash_empty)
			APD_slope_up = ( (float) get_flash_register(en7571_flash_APD_slope_up) )/100;

		if(get_flash_register(en7571_flash_APD_slope_dn)!=flash_empty)
			APD_slope_dn = ( (float) get_flash_register(en7571_flash_APD_slope_dn) )/100;

		if(get_flash_register(en7571_flash_APD_change_point)!=flash_empty)
			APD_voltage_NT = ( (float) get_flash_register(en7571_flash_APD_change_point) )/100;

		if( ( get_flash_register(en7571_flash_APD_voltage_1)|get_flash_register(en7571_flash_APD_voltage_2) ) != flash_empty)		//by HC 20150303
		{
			APD_voltage_0x00 = ( (float)( (get_flash_register(en7571_flash_APD_voltage_1)&flash_upper_16_bits)>>16) )/10;
			APD_voltage_0x40 = ( (float)( get_flash_register(en7571_flash_APD_voltage_1)&flash_lower_16_bits)         )/10;
			APD_voltage_0x80 = ( (float)( (get_flash_register(en7571_flash_APD_voltage_2)&flash_upper_16_bits)>>16) )/10;
			APD_voltage_0xC0 = ( (float)( get_flash_register(en7571_flash_APD_voltage_2)&flash_lower_16_bits)         )/10;

			// If not ( V40>V00 and V80>V40 and VC0>V80 ), return
			// Logic : (xyz)' = x' + y' + z'
			if( (APD_voltage_0x40 <= APD_voltage_0x00) || (APD_voltage_0x80 <= APD_voltage_0x40) || (APD_voltage_0xC0 <= APD_voltage_0x80) )
				{
					printk("APD slope error\n");
					return;
				}
		}

		//Equation for Vapd calculation
		if( en7571_BOSA_temperature > 25 )
			en7571_APD_voltage = APD_voltage_NT + APD_slope_up*( en7571_BOSA_temperature - 25 );
		else
			en7571_APD_voltage = APD_voltage_NT - APD_slope_dn*( 25 - en7571_BOSA_temperature );

		//Print en7571_BOSA_temperature
		a = (int)en7571_BOSA_temperature;
		b = abs( (int)(en7571_BOSA_temperature*10)%10 );
		PON_PHY_MSG(PHY_MSG_DBG, "BOSA temperature = %d.%dC\n", a, b);
		//Print en7571_APD_voltage
		a = (int)en7571_APD_voltage;
		b = (int)(en7571_APD_voltage*10)%10;
		PON_PHY_MSG(PHY_MSG_DBG, "APD Voltage = %d.%dV\n", a, b);

		//Equation for RG_APD_DAC_CODE calculation, the default_APD_zero_code_voltage and default_APD_voltage_step depend on circuitry.
		if( en7571_APD_voltage < APD_voltage_0x80 )
		{
			if( en7571_APD_voltage < APD_voltage_0x40 )	// V00 <= VAPD < V40
			{
				APD_voltage_step = (APD_voltage_0x40 - APD_voltage_0x00)/64;
				ptr[0] = 0x00 + (int)( (en7571_APD_voltage - APD_voltage_0x00)/APD_voltage_step );
				PON_PHY_MSG(PHY_MSG_DBG, "APD slope 1\n");
			}
			else												// V40 <= VAPD < V80
			{
				APD_voltage_step = (APD_voltage_0x80 - APD_voltage_0x40)/64;
				ptr[0] = 0x40 + (int)( (en7571_APD_voltage - APD_voltage_0x40)/APD_voltage_step );
				PON_PHY_MSG(PHY_MSG_DBG, "APD slope 2\n");
			}

		}
		else
		{
			APD_voltage_step = (APD_voltage_0xC0 - APD_voltage_0x80)/64;
			PON_PHY_MSG(PHY_MSG_DBG, "APD slope 3\n");

			if( en7571_APD_voltage < APD_voltage_0xC0 )	// V80 <= VAPD < VC0
				ptr[0] = 0x80 + (int)( (en7571_APD_voltage - APD_voltage_0x80)/APD_voltage_step );
			else												// VC0 <= VAPD <= VFF
			{
				ptr[0] = 0xC0 + (int)( (en7571_APD_voltage - APD_voltage_0xC0)/APD_voltage_step );
				if( ptr[0] > 0xff )
					ptr[0] = 0xff;
			}
		}

		PON_PHY_MSG(PHY_MSG_DBG, "APD = 0x%x\n", ptr[0]);
		en7571_APD_I2C_write( ptr );

}


/*****************************************************************************
//Function :
//		en7571_APD_DAC
//Description :
//		This function is used to set DAC of APD voltage manually
//Input :
//		DAC
******************************************************************************/
void en7571_APD_DAC(UINT8 DAC)
{
	unchar ptr[1];

	en7571_APD_initialization();

	ptr[0] = DAC;
	printk("DAC = 0x%x\n", ptr[0]);
	en7571_APD_I2C_write( ptr );
}


/*****************************************************************************
//Function :
//		en7571_APD_I2C_write
//Description :
//		This function is to write APD register
//Input :
//		DAC
******************************************************************************/
void en7571_APD_I2C_write(unchar ptr[1])
{
	PON_PHY_MSG(PHY_MSG_DBG, "APD in en7571_APD_I2C_write = 0x%x\n", ptr[0]);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 1);
}
/*****************************************************************************
//function :
//		en7571_info
//description :
//		this function is used to select 7571 information output
//input :
//		select : 1 for Pav; 2 for average current; 3 for bias current ; 4 for modulation current ;
//		6 for P1
//output :
//		DAC
******************************************************************************/
UINT32 en7571_info(uint select)
{
	UINT32 output = 0;
	unchar ptr[4];
	unchar ptr_temp[4];

	if(select == SELECT_IBIAS_NOW)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P0_PWR_CTRL_CS3, ptr_temp, 4);
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P0_PWR_CTRL_CS3, ptr, 4);
		if( ptr_temp[3] == ptr[3] )
			output = ( (ptr[3]<<8) | ptr[2]  ) & 0xfff;
		else
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P0_PWR_CTRL_CS3, ptr, 4);
			output = ( (ptr[3]<<8) | ptr[2]  ) & 0xfff;
		}
	}
	else if(select == SELECT_IMOD_NOW)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P1_PWR_CTRL_CS3, ptr_temp, 4);
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P1_PWR_CTRL_CS3, ptr, 4);
		if( ptr_temp[3] == ptr[3] )
			output = ( (ptr[3]<<8) | ptr[2]  ) & 0xfff;
		else
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_P1_PWR_CTRL_CS3, ptr, 4);
			output = ( (ptr[3]<<8) | ptr[2]  ) & 0xfff;
		}
	}
	else if(select == SELECT_IAV_NOW)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_3, ptr_temp, 4);
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_3, ptr, 4);
		if( ptr_temp[1] == ptr[1] )
			output = ( (ptr[1]<<8) | ptr[0]  ) & 0xfff;
		else
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_3, ptr, 4);
			output = ( (ptr[1]<<8) | ptr[0]  ) & 0xfff;
		}
	}
	else if(select == SELECT_PAV_CAL)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4);
		output = ( (ptr[1]<<8) | ptr[0]  ) & 0xfff;
	}
	else if(select == SELECT_P1_CAL)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4);
		output = ( (ptr[3]<<8) | ptr[2]  ) & 0x3ff;
	}
	else if(select == SELECT_PAV_NOW)
	{
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_0, ptr, 4 );
		output = ( (ptr[1]<<8) | ptr[0]  ) & 0xfff;
	}
	else if(select == SELECT_P1_NOW)
	{
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RO_PWR_CTRL_0, ptr, 4 );
		output = ( (ptr[3]<<8) | ptr[2]  ) & 0x3ff;
	}

	return output;
}
/*****************************************************************************
//function :
//		en7571_ADC_temperature_get
//description :
//		this function is to get the ADC code of temperature
//input :
//		N/A
//output :
//		ADC code
//
******************************************************************************/
int en7571_ADC_temperature_get(void)
{
	unchar ptr[4];
	UINT8 read_data = 0;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD , ptr, 1);
		read_data = ptr[0];		// Keep default
		ptr[0] = (ptr[0] & (ADC_select_mask))|(ADC_select_temperature);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		en7571_temperature_ADC = en7571_SVADC_get();
		PON_PHY_MSG(PHY_MSG_DBG, "temperature ADC = 0x%x\n", en7571_temperature_ADC);

		//set to default ADCAUX
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD , ptr, 1);
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

        atomic_set(&i2c_protect, 0);
	}
	return en7571_temperature_ADC;
}

/*****************************************************************************
//function :
//		en7571_ADC_voltage_get
//description :
//		this function is to get the ADC code of voltage
//input :
//		N/A
//output :
//		ADC code
//
******************************************************************************/
int en7571_ADC_voltage_get(void)
{
	unchar ptr[4];
	uint read_data = 0;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		//internal termination resistor is off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask))|(LA_Rx_HighZ_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 1);

		// Set ADC to VCC output, RG_SVADC_INMUX = b'0000 ( 0x24[4:1] = 0 )
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD , ptr, 1);
		read_data = ptr[0];
		ptr[0] = ptr[0] & (ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		en7571_voltage_ADC = en7571_SVADC_get();
		PON_PHY_MSG(PHY_MSG_DBG, "VCC ADC = 0x%x\n", en7571_voltage_ADC);

		//internal termination resistor is on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask));
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 1);

		// Set ADCAUX to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		atomic_set(&i2c_protect, 0);
	}

	return en7571_voltage_ADC;
}


/*****************************************************************************
//Function :
//		en7571_load_Tx_cal_data
//Description :
//		This function is to load initial bias and modulation current
******************************************************************************/
void en7571_load_Tx_cal_data(void)
{
	unchar ptr[4];
	unchar ptr_tmp[2];
	UINT32 read_data = 0;

	//Load currents from flash
	if( get_flash_register(en7571_flash_Iav_Imod) != flash_empty)
	{
		//Read Iav initial value
		read_data = get_flash_register(en7571_flash_Iav_Imod)&FLASH_IAV_MASK;
		read_data = read_data >> 16;
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		//Read Imod initial value
		read_data = get_flash_register(en7571_flash_Iav_Imod)&FLASH_IMOD_MASK;
		ptr[2] = read_data;
		ptr[3] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4 );
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_FLASH_1, ptr, 4 );
		printk("Initial current loaded\n");
	}

	//Load Pav and P1 from flash
	if( get_flash_register(en7571_flash_Pav_P1) != flash_empty)
	{
		//Read Pav
		read_data = get_flash_register(en7571_flash_Pav_P1)&FLASH_PAV_MASK;
		read_data = read_data >> 16;
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		//Read P1
		read_data = get_flash_register(en7571_flash_Pav_P1)&FLASH_P1_MASK;
		ptr[2] = read_data;
		ptr[3] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4 );
		ptr_tmp[0] = ptr[0];
		ptr_tmp[1] = ptr[1];
		ptr[0] = ptr[2];
		ptr[1] = ptr[3];
		ptr[2] = ptr_tmp[0];
		ptr[3] = ptr_tmp[1];
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_FLASH_2, ptr, 4 );
		printk("PWR/ER loaded\n");
	}
#if 0
	// For auto-locked mode, Set rg_pwr_ctrl_e3_flash_Temp != 0 and rg_pwr_ctrl_e3_b_flash_in = 1
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_FLASH_3, ptr, 4);
	ptr[1] = (ptr[1]&0xfe) | 0x01;
	ptr[0] = 0xff;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_FLASH_3, ptr, 4);
#endif
}


/*****************************************************************************
//Function :
//		en7571_temperature_get
//Description :
//		This function is to get IC and BOSA temperature
******************************************************************************/
void en7571_temperature_get(void)
{
	UINT32 read_data = 0;
	float BOSA_temperature_offset = default_BOSA_temperature_offset;
	float Efuse_offset = temperature_offset_efuse;
   	float sensor_voltage;

    en7571_ADC_temperature_get();
	sensor_voltage = en7571_ADC_slope*((float)en7571_temperature_ADC)+en7571_ADC_offset;

	en7571_IC_temperature = temperature_offset - ( temperature_ADC_slop * sensor_voltage );	//The equation of IC temperature
	en7571_IC_temperature = en7571_IC_temperature - en7571_eFuse_temperature_offset - Efuse_offset ;  // The equation of Efuse comp.

	read_data = get_flash_register(en7571_flash_temp_offset);
	if( read_data != flash_empty )
		BOSA_temperature_offset = read_data;

	en7571_BOSA_temperature = en7571_IC_temperature - (float)BOSA_temperature_offset;

}


/*****************************************************************************
//function :
//		en7571_temperature_get_8472
//description :
//		this function is used to get temperature
//input :
//		N/A
//output :
//		temperature in 8472 type
//
******************************************************************************/
UINT16 en7571_temperature_get_8472(void)
{
	int a = 0;
	int b = 0;
	UINT16 IC_temperature_8472 = 0;
	en7571_temperature_get();
	if( en7571_IC_temperature > 0)
		IC_temperature_8472 = (UINT16)(en7571_IC_temperature*TEMPERATURE_CELSIUS_TO_8472);
	else
		IC_temperature_8472 = temperature_two_complement - (UINT16)( (-1)*en7571_IC_temperature*TEMPERATURE_CELSIUS_TO_8472 );

	a = (int)en7571_IC_temperature;
	b = abs( (int)(en7571_IC_temperature*10)%10 );
	PON_PHY_MSG(PHY_MSG_DBG, "IC temperature = %d.%dC\n", a, b);

	return IC_temperature_8472;
}


/*****************************************************************************
//function :
//		en7571_supply_voltage_get_8472
//description :
//		this function is used to get supply voltage
//input :
//		N/A
//output :
//		supply voltage in 8472 type
//
******************************************************************************/
UINT16 en7571_supply_voltage_get_8472(void)
{
	return ( 2*(en7571_ADC_slope*en7571_ADC_voltage_get()+en7571_ADC_offset)/voltage_8472_unit ); 	//To 8472
}


/*****************************************************************************
//function :
//		en7571_bias_current_get_8472
//description :
//		this function is used to get bias current
//input :
//		N/A
//output :
//		bias current in 8472 type
//
******************************************************************************/
UINT16 en7571_bias_current_get_8472(void)
{
	return ( en7571_info(SELECT_IBIAS_NOW)*bias_current_slope*1000/bias_current_8472_slope );	//To 8472
}


/*****************************************************************************
//function :
//		en7571_RSSI_calibration
//description :
//		this function is to calibrate Rx RSSI current
******************************************************************************/
void en7571_RSSI_calibration(void)
{
		unchar ptr[4];
		uint read_data = 0;
		float RSSI_factor = 0;

		// RSSI calibration on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);
		ptr[1] = (ptr[1]&RSSI_cal_mask) | RSSI_cal_en;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);

		// RSSI V mode on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);
		ptr[1] = (ptr[1]&0xbf) | 0x40;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);

		// Set ADC to RSSI output
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		en7571_RSSI_Vref = en7571_SVADC_get();
		printk("RSSI_Vref = 0x%x\n", en7571_RSSI_Vref);

		// RSSI V mode off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);
		ptr[1] = ptr[1] & 0xbf;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);

		en7571_RSSI_V = en7571_SVADC_get();
		printk("RSSI_V = 0x%x\n", en7571_RSSI_V);

		// RSSI calibration off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);
		ptr[1] = ptr[1] & RSSI_cal_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 2);

		// Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		// RSSI_V = RSSI_Vref + 0.35*RSSI_factor, where 0.35 is the ideal valur of IR
		if( en7571_RSSI_V > en7571_RSSI_Vref )
			RSSI_factor = ( en7571_ADC_slope*(en7571_RSSI_V- en7571_RSSI_Vref)+en7571_ADC_offset  )/RSSI_IDEAL_IR;
		else
			printk("RSSI calibration fail\n");
}
/*****************************************************************************
//function :
//		en7571_RSSI_get
//description :
//		this function is to get RSSI current
//input :
//		N/A
//output :
//		RSSI current
//
******************************************************************************/
UINT32 en7571_RSSI_get(void)
{
	unchar ptr[4];
	uint read_data        = 0;
	uint RSSI_voltage     = 0;
	uint RSSI_GAIN        = 0;
	uint RSSI_GAIN_factor = 0;
	int search_cnt        = 0;											//by HC 20150401
	UINT32 acc_num = 10;	// 1024
	int vld = 0;
	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		//set ADC to RSSI voltage output
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		// Set accumulate number
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);
		ptr[3] = (ptr[3]&rg_adlch_count_mask) | rg_adlch_count_1024;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);

		for( search_cnt = 0; search_cnt < 6; search_cnt++ )				//by HC 20150401
		{
			RSSI_GAIN = RSSI_GAIN_DEFAULT_SETTING - search_cnt;		//RSSI_GAIN from 5 to 0			//by HC 20150401
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
			ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);

			// Latch ADC value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);
			ptr[3] = (ptr[3]&rg_adlch_trig_mask) | rg_adlch_trig_trigger;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);
			mdelay(20);
			// Chack if valid
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);
			vld = (ptr[2]&0x80) >> 7;
			if(vld == 1)
			{
				// Read voltage value
				phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 4);
				RSSI_voltage = ( ( (ptr[2]<<16) | (ptr[1]<<8) | ptr[0] ) & adlch_sum_mask )>>acc_num;
				PON_PHY_MSG(PHY_MSG_DBG, "RSSI ADC = 0x%x\t", RSSI_voltage);
			}

			if(RSSI_voltage < en7571_RSSI_Vref - ADC_RSSI_defend_noise_threshold)	//by Biker 20150615
				break;
		}
		if (ADC_0V5> RSSI_voltage)
			PON_PHY_MSG(PHY_MSG_DBG, "\nRSSI ADC code lower than 0.5V, not precise!\n");	//by Biker 20150615

		PON_PHY_MSG(PHY_MSG_DBG, "\nRSSI GAIN = %d, RSSI_voltage = %x, \n", RSSI_GAIN,RSSI_voltage);

		// Set accumulate number to default
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 1);
		ptr[3] = ptr[3]&rg_adlch_count_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ADLCH_CTRL, ptr, 1);

		// Set RSSI gain back to 0x05
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN_DEFAULT_SETTING;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);

		// Set ADCAUX back to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		atomic_set(&i2c_protect, 0);

		switch(RSSI_GAIN)
		{
			case 0:
				RSSI_GAIN_factor = 1;
				break;
			case 1 :
				RSSI_GAIN_factor = 4;
				break;
			case 2 :
				RSSI_GAIN_factor = 16;
				break;
			case 3 :
				RSSI_GAIN_factor = 64;
				break;
			case 4 :
				RSSI_GAIN_factor = 128;
				break;
			case 5 :
				RSSI_GAIN_factor = 256;
				break;
		}

		if ( en7571_RSSI_Vref >= RSSI_voltage )
		{
			en7571_RSSI_current = (en7571_RSSI_Vref - RSSI_voltage)*RSSI_GAIN_factor;
		}
		else
		{
			en7571_RSSI_current = 0;
		}
	}

	PON_PHY_MSG(PHY_MSG_DBG, "RSSI = 0x%x\n", en7571_RSSI_current);
	return en7571_RSSI_current;
}


/*****************************************************************************
//function :
//		en7571_dark_current
//description :
//		this function is to find BOSA's dark current
//input :
//		N/A
//output :
//		dark current
//Date :
//		2017/01/25 by YW
******************************************************************************/
UINT32 en7571_dark_current(void)
{
	unchar ptr[4];
	uint read_data        = 0;
	uint RSSI_voltage     = 0;
	uint RSSI_GAIN        = 0;
	uint cnt = 64;
	uint i = 0;

	if( i2c_access_protect() == -1 )
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
	else
	{
		atomic_set(&i2c_protect, 1);

		//set ADC to RSSI voltage output
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		RSSI_GAIN = 0;//5 - search_cnt;		//RSSI_GAIN from 5 to 0			//by HC 20150401

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);

		for(i=0;i<cnt;i++)
		{									//by Biker 20150615
			RSSI_voltage += en7571_SVADC_get();
		}

		// Set RSSI gain back to 0x05
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | (0x05);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LA_PWD, ptr, 4);

		// Set ADCAUX back to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 1);

		atomic_set(&i2c_protect, 0);
	}

	return RSSI_voltage;
}


/*****************************************************************************
//Function :
//		en7571_RougeONU_clear
//Description :
//		This function is to clear rouge ONU status
******************************************************************************/
void en7571_RougeONU_clear(void)
{
	unchar ptr[1];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_ROGUE_ONU_DET_CTRL + 0x01, ptr, 1);
	ptr[0] = (ptr[0] & RogueONU_mask) | 0x1;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_ROGUE_ONU_DET_CTRL + 0x01, ptr, 1);
	printk("Rogue ONU clear\n");
}


/*****************************************************************************
//Function :
//		en7571_PWRADC_enable
//Description :
//		This function is to set path for PWRADC
******************************************************************************/
void en7571_PWRADC_enable(void)
{
	unchar ptr[4];

	// Set RG_TIAMUX = 3b'000
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);
	ptr[0] = ptr[0] & tia_mux_mask;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_TIAMUX, ptr, 1);

	// Set RG_PWRK1 = 0
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);
	ptr[3] = ptr[3] & RG_PWRK1_mask;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);

	// Set rg_pwr_ctrl_e3_ben_block = 2'b01
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWR_CTRL_BEN_0, ptr, 1);
	ptr[0] = (ptr[0] & rg_pwr_ctrl_ben_block_mask) | rg_pwr_ctrl_ben_block_setting;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWR_CTRL_BEN_0, ptr, 1);

	printk("PWRADC ready\n");
}


/*****************************************************************************
//Function :
//		en7571_PWRADC_calibration
//Description :
//		This function gets BOSA's MPD current offset value when Tx is off.
******************************************************************************/
void en7571_PWRADC_calibration(void)
{
	unchar ptr[4];
	int vld = 0;

	// Set RG_PWRK1 to 1
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);
	ptr[3] = (ptr[3] & RG_PWRK1_mask) | RG_PWRK1_1;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);

	// Set accumulate number
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = (ptr[3]&rg_pwradc_count_mask	) | rg_pwradc_count_1024;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);

	// Latch ADC value
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = (ptr[3]&rg_pwradc_trig_mask) | rg_pwradc_trig_trigger;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	mdelay(20);
	// Chack if valid
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	vld = (ptr[2]&0x10) >> 4;
	if(vld == 1)
	{
		// Read PWRADC
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
		en7571_PWRADC_offset = ( (ptr[2]<<16) | (ptr[1]<<8) | ptr[0] ) & pwradc_adlch_sum_mask;
		en7571_PWRADC_offset = en7571_PWRADC_offset >> 4;
		printk("PWRADC offset = 0x%x\n", en7571_PWRADC_offset);
	}
	// Set accumulate number to default
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = ptr[3]&rg_pwradc_count_mask;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);

	// Set RG_PWRK1 to 0
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);
	ptr[3] = ptr[3] & 0xef;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_ANA_CTRL1, ptr, 4);

}


/*****************************************************************************
//Function :
//		en7571_PWRADC_get
//Description :
//		This function is to get BOSA's MPD current
//Input :
//		N/A
//Output :
//		power ADC
//
******************************************************************************/
UINT32 en7571_PWRADC_get(void)
{
	unchar ptr[4];
	UINT32 ADC = 0;
	int vld = 0;

	if( (gpPhyPriv->phyCfg.flags.trafficStatus == TRAFFIC_STATUS_DOWN) && (en7571_PatternEnabled == 0) )
		return en7571_PWRADC_offset;			// Tx off

	// Set accumulate number
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = (ptr[3]&rg_pwradc_count_mask	) | rg_pwradc_count_1024;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);

	// Latch ADC value
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = (ptr[3]&rg_pwradc_trig_mask) | rg_pwradc_trig_trigger;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	mdelay(20);
	// Chack if valid
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	vld = (ptr[2]&0x10) >> 4;
	if(vld == 1)
	{
		// Read PWRADC
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
		ADC = ( (ptr[2]<<16) | (ptr[1]<<8) | ptr[0] ) & pwradc_adlch_sum_mask;
		ADC = ADC >> 4;
		PON_PHY_MSG(PHY_MSG_DBG, "PWRADC = 0x%x\n", ADC);
	}
	// Set accumulate number to default
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);
	ptr[3] = ptr[3]&rg_pwradc_count_mask;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_PWRADC_DATA2, ptr, 4);

	if( ADC < en7571_PWRADC_offset )							//No Tx power
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Tx off, return %x\n", en7571_PWRADC_offset);
		en7571_PWRADC = en7571_PWRADC_offset;
	}
	else
	{
		en7571_PWRADC = ADC;
	}

	return en7571_PWRADC;
}
/*****************************************************************************
//function :
//		en7571_RxPower_get_8472
//description :
//		This function is to get Rx power in 8472 type
//input :
//		N/A
//output :
//		Rx power in 8472 type
//Date:
//		2015/01/27 by HC
//
******************************************************************************/
UINT16 en7571_RxPower_get_8472(void)
{
	UINT32 read_data = 0;

	UINT32 RSSI_current_now    = 0;

	float rx_power_now  = 0;			//unit: uW x100
	UINT16 rx_power_8472 = 0;	//unit: 0.1 uW

	float div     = 0;
	float slope   = 0;
	UINT32 RSSI_current_up = 0;
	UINT32 RSSI_current_md = 0;
	UINT32 RSSI_current_dn = 0;

	UINT32 rx_power_up = 0;		//unit: uW x100
	UINT32 rx_power_md = 0;		//unit: uW x100
	UINT32 rx_power_dn = 0;		//unit: uW x100

	float RSSI_current_1, RSSI_current_2, RSSI_current_3, Rx_power_K1, Rx_power_K2, Rx_power_K3;
	static float d, e, f, g, x, y, z;

	if( (get_flash_register(en7571_flash_DDMI_Rx_p4)&0x0000000f) == 1 )	//K 3-points to calculate formula: rx_power_now = x*RSSI^2 + y*RSSI + z;  biker_20151001
	{
		if (en7571_DDMI_rx_cal_flag == 0)
		{
			read_data = get_flash_register(en7571_flash_DDMI_Rx_p1);
			RSSI_current_1 = read_data & flash_Rx_RSSI_current_mask;
			Rx_power_K1 = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;

			read_data = get_flash_register(en7571_flash_DDMI_Rx_p2);
			RSSI_current_2 = read_data & flash_Rx_RSSI_current_mask;
			Rx_power_K2 = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;

			read_data = get_flash_register(en7571_flash_DDMI_Rx_p3);
			RSSI_current_3 = read_data & flash_Rx_RSSI_current_mask;
			Rx_power_K3 = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;


			d= RSSI_current_1 * RSSI_current_1 * RSSI_current_2 + RSSI_current_1 * RSSI_current_3 * RSSI_current_3 + RSSI_current_2 * RSSI_current_2 * RSSI_current_3 - RSSI_current_2 * RSSI_current_3 * RSSI_current_3 - RSSI_current_1 * RSSI_current_2 * RSSI_current_2 - RSSI_current_1 * RSSI_current_1 * RSSI_current_3 ;
			e= Rx_power_K1 * RSSI_current_2 + RSSI_current_1 * Rx_power_K3 + Rx_power_K2 * RSSI_current_3 - RSSI_current_2 * Rx_power_K3 - RSSI_current_1 * Rx_power_K2 - Rx_power_K1 * RSSI_current_3 ;
			f= RSSI_current_1 * RSSI_current_1 * Rx_power_K2 + Rx_power_K1 * RSSI_current_3 * RSSI_current_3 + RSSI_current_2 * RSSI_current_2 * Rx_power_K3 - Rx_power_K2 * RSSI_current_3 * RSSI_current_3 - Rx_power_K1 * RSSI_current_2 * RSSI_current_2 - RSSI_current_1 * RSSI_current_1 * Rx_power_K3 ;
			g= RSSI_current_1 * RSSI_current_1 * RSSI_current_2 * Rx_power_K3 + RSSI_current_1 * Rx_power_K2 * RSSI_current_3 * RSSI_current_3 + Rx_power_K1 * RSSI_current_2 * RSSI_current_2 * RSSI_current_3 - Rx_power_K1 * RSSI_current_2 * RSSI_current_3 * RSSI_current_3 - RSSI_current_1 * RSSI_current_2 * RSSI_current_2 * Rx_power_K3 - RSSI_current_1 * RSSI_current_1 * Rx_power_K2 * RSSI_current_3 ;

		 	if(d==0)
		 	{
		    		PON_PHY_MSG(PHY_MSG_DBG, "Invalid DDMI Rx power-RSSI mapping curve \n");
		 	}
			else
			{
				x=e/d;
				y=f/d;
				z=g/d;
			}

			en7571_DDMI_rx_cal_flag = 1;
		}

		RSSI_current_now = en7571_RSSI_get();
		rx_power_now = (float) RSSI_current_now*RSSI_current_now*x + (float)RSSI_current_now*y + z;

		if(rx_power_now < 0)
			rx_power_now = 0;

		//PON_PHY_MSG(PHY_MSG_DBG, "x *10^12 = %d, y*10^6 = %d, z*10 = %d\n",(int)(x*1000000000000),(int)(y*1000000),(int)(z*10));

	}
	else 	//3_line algorithm
	{
		RSSI_current_now = en7571_RSSI_get();
		if( get_flash_register(en7571_flash_DDMI_Rx_p1) != flash_empty )	//Load Rx power point #1
		{
			read_data = get_flash_register(en7571_flash_DDMI_Rx_p1);
			RSSI_current_up = read_data & flash_Rx_RSSI_current_mask;
			rx_power_up = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}
		if( get_flash_register(en7571_flash_DDMI_Rx_p2) != flash_empty )	//Load Rx power point #2
		{
			read_data = get_flash_register(en7571_flash_DDMI_Rx_p2);
			RSSI_current_md = read_data & flash_Rx_RSSI_current_mask;
			rx_power_md = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}
		if( get_flash_register(en7571_flash_DDMI_Rx_p3) != flash_empty )	//Load Rx power point #3
		{
			read_data = get_flash_register(en7571_flash_DDMI_Rx_p3);
			RSSI_current_dn = read_data & flash_Rx_RSSI_current_mask;
			rx_power_dn = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}

		if( RSSI_current_now <= RSSI_current_dn )												//Zone 1
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 1\n");
			div = RSSI_current_dn;
			if(div != 0)
			{	//slope = rx_power_dn/RSSI_current_dn
				slope = (float)rx_power_dn/(float)RSSI_current_dn;
				rx_power_now = RSSI_current_now*slope;												//Bug fixed by HC 20150203
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
		else if( (RSSI_current_dn < RSSI_current_now) && (RSSI_current_now < RSSI_current_md) ) 	//Zone 2
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 2\n");
			div = RSSI_current_md-RSSI_current_dn;
			if(div != 0)
			{	//slope = (rx_power_md-rx_power_dn)/(RSSI_current_md-RSSI_current_dn)
				slope = (float)(rx_power_md-rx_power_dn)/(float)(RSSI_current_md-RSSI_current_dn);
				rx_power_now = (RSSI_current_now-RSSI_current_dn)*slope + rx_power_dn;
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
		else if( RSSI_current_md <= RSSI_current_now )											//Zone 3
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 3\n");
			div = RSSI_current_up-RSSI_current_md;
			if(div != 0)
			{	//slope = (rx_power_up-rx_power_md)/(RSSI_current_up-RSSI_current_md)
				slope = (float)(rx_power_up-rx_power_md)/(float)(RSSI_current_up-RSSI_current_md);
				rx_power_now = (RSSI_current_now-RSSI_current_md)*slope + rx_power_md;
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
	}

	//To 8472
	rx_power_8472 = rx_power_now/10;
	PON_PHY_MSG(PHY_MSG_DBG, "Rx_power_8472 = %d\n", rx_power_8472);
	return rx_power_8472;
}


/*****************************************************************************
//Function :
//		en7571_safe_circuit_reset
//Description :
//		This function is to reset safe circuit
******************************************************************************/
void en7571_safe_circuit_reset(void)
{
	unchar ptr[4];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SAFE_PROTECT, ptr, 4);
	ptr[1] = (ptr[1] & safe_circuit_mask) | (safe_circuit_reset);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SAFE_PROTECT, ptr, 4);
}
/*****************************************************************************
//function :
//		en7571_TxPower_get_8472
//description :
//		This function is to get Tx power in 8472 type
//input :
//		N/A
//output :
//		Tx power in 8472 type
//Date:
//		20150123 by HC
//
******************************************************************************/
UINT16 en7571_TxPower_get_8472(void)
{
	int div 			 	 	= 0;
	UINT32 read_data 	 	= 0;
	UINT32 PWRADC_now		= 0;
	UINT32 PWRADC_up 	 	= 0;
	UINT32 PWRADC_dn 	 	= en7571_PWRADC_offset;	//Defualt by HC 20150126
	float slope			 	= 0;
	float tx_power_now	= 0;					//unit: mW x100
	UINT32 tx_power_up 	 	= 0;					//unit: mW x100
	UINT32 tx_power_dn 	 	= 0;					//unit: mW x100
	UINT16 tx_power_8472 	= 0;					//unit: 0.1 uW

	PWRADC_now = en7571_PWRADC_get();
	PON_PHY_MSG(PHY_MSG_DBG, "PWRADC_now = 0x%x\n", PWRADC_now);
	if( get_flash_register(en7571_flash_DDMI_Tx_p1) != flash_empty)
	{
		read_data = get_flash_register(en7571_flash_DDMI_Tx_p1);
		PWRADC_up = read_data & flash_MPD_ADC_mask;
		tx_power_up = (read_data & flash_Tx_power_mask)>>flash_Tx_power_offset;
	}

	div = PWRADC_up - PWRADC_dn;
	if( div != 0 )
	{
		slope = (float)(tx_power_up-tx_power_dn)/(float)(PWRADC_up-PWRADC_dn);
		tx_power_now = (float)(PWRADC_now-PWRADC_dn)*slope + tx_power_dn;
	}
	else
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Invalid Tx power point\n");
	}

	//To 8472
	tx_power_8472 = (int)(10*tx_power_now)/0.1;
	PON_PHY_MSG(PHY_MSG_DBG, "Tx_power_8472 = %d\n", tx_power_8472);
	return tx_power_8472;
}
/*****************************************************************************
//function :
//		en7571_internal_clock
//description :
//		this function is used to execute functions periodically
******************************************************************************/
void en7571_internal_clock(void)
{
	static UINT32 cnt;

	// Internal DDMI
	if( en7571_internal_DDMI == 1 )
	{
		if(cnt%en7571_Tapd == en7571_Tapd-1)
		{
			// APD control
			if( en7571_xPON_mode == PHY_GPON_CONFIG )							// by HC 20150520
				en7571_APD_control();								//tune APD voltage per en7571_Tapd(sec). YMC 20150122
		}
		if(cnt%10 == 0 || en7571_fast_DDMI)
			en7571_DDMI_voltage     = en7571_supply_voltage_get_8472();

		if(cnt%10 == 2 || en7571_fast_DDMI)
			en7571_DDMI_current     = en7571_bias_current_get_8472();

		if(cnt%10 == 4 || en7571_fast_DDMI)
			en7571_DDMI_temperature = en7571_temperature_get_8472();

		if(cnt%10 == 6 || en7571_fast_DDMI)
		{
			en7571_DDMI_tx_power    = en7571_TxPower_get_8472();		//by HC 20150129
			en7571_tx_power_alarm_get();
		}
		if(cnt%10 == 8 || en7571_fast_DDMI)
		{
			en7571_DDMI_rx_power    = en7571_RxPower_get_8472();		//by HC 20150129
			en7571_rx_power_alarm_get();
		}

	}


	if( ( (cnt%SWKT_PERIOD)==(SWKT_PERIOD-3) ) || (en7571_PatternEnabled==1) )
	{
		if( en7571_KT != 0 )
			en7571_SWKT();
	}


	cnt ++;															// by HC 20150727
}

/*****************************************************************************
//function :
//		en7571_param_status_real
//description :
//		this function is used to get transceiver parameter realtime
//		Note: before call this function, should check if(i2c_protect == 0)
//input :
//		PPHY_TransParam_T structure, defined in phy_api.h
//output :
//		N/A
******************************************************************************/
void en7571_param_status_real(PPHY_TransParam_T transceiver_param)
{

	transceiver_param->temprature 		= en7571_DDMI_temperature;
	transceiver_param->supply_voltage 	= en7571_DDMI_voltage;
	transceiver_param->tx_current 		= en7571_DDMI_current;
	transceiver_param->tx_power 			= en7571_DDMI_tx_power;
	transceiver_param->rx_power 			= en7571_DDMI_rx_power;
}
/*****************************************************************************
//Function :
//		en7571_DDMI_check_8472
//Description :
//		This function is to show the five DDMI items in 8472 format
//Input :
//		N/A
//Output :
//		Five DDMI items in 8472 format
//Date :
//		2015/03/11 by HC
******************************************************************************/
void en7571_DDMI_check_8472(void)
{
	en7571_DDMI_voltage	 		= en7571_supply_voltage_get_8472();
	en7571_DDMI_current	 		= en7571_bias_current_get_8472();
	en7571_DDMI_temperature 	= en7571_temperature_get_8472();
	en7571_DDMI_tx_power	 	= en7571_TxPower_get_8472();
	en7571_DDMI_rx_power	 	= en7571_RxPower_get_8472();

	printk("Supply Voltage = %05d\n" , en7571_DDMI_voltage	   );
	printk("Tx Bias Current = %05d\n", en7571_DDMI_current	   );
	printk("Temperature = %05d\n"    , en7571_DDMI_temperature);
	printk("Tx power = %05d\n"       , en7571_DDMI_tx_power   );
	printk("Rx Power = %05d\n"       , en7571_DDMI_rx_power   );
}
/*****************************************************************************
//Function :
//		en7571_trans_model_setting
//Description :
//		This function is to set polarity of EN7571
******************************************************************************/
extern struct work_struct work_mt7571_safe_circuit_reset;   //dyma_20170117

void en7571_trans_model_setting(void)
{
	#if 1
        IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x10f);
		#if 0                                        //dyma_20170117
			mt7570_RougeONU_clear();				//cannot call this function during interrupt occur ,by Jay 20151120
		#endif
		if(NULL != i2c_access_queue){
			queue_work(i2c_access_queue, &work_mt7571_safe_circuit_reset);
		}
	#else
		IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x11f);		// 0x11f for open loop mode
	#endif

}
/*****************************************************************************
//Function :
//		en7571_trans_alarm_getting
//Description :
//		This function is to get alarm
******************************************************************************/
void en7571_trans_alarm_getting(PPHY_TransAlarm_T transceiver_alarm)
{
		transceiver_alarm->rx_power_high_alarm_thod = 0;
		transceiver_alarm->rx_power_low_alarm_thod = 0;
		transceiver_alarm->tx_power_high_alarm_thod = 0;
		transceiver_alarm->tx_power_low_alarm_thod = 0;

		PON_PHY_MSG(PHY_MSG_DBG, "rx power high alarm threshold: %.8x\n", transceiver_alarm->rx_power_high_alarm_thod);
		PON_PHY_MSG(PHY_MSG_DBG, "rx power low alarm threshold: %.8x\n", transceiver_alarm->rx_power_low_alarm_thod);
		PON_PHY_MSG(PHY_MSG_DBG, "tx power high alarm threshold: %.8x\n", transceiver_alarm->tx_power_high_alarm_thod);
		PON_PHY_MSG(PHY_MSG_DBG, "rx power low alarm threshold: %.8x\n", transceiver_alarm->tx_power_low_alarm_thod);
}
/*****************************************************************************
//Function :
//		en7571_tx_power_alarm
//Description :
//		This function is to get BOB Tx alarm.
//Input :
//		N/A
//Output :
//		EN7571 Tx alarm status.
//Date :20150414 by YMC
//
******************************************************************************/
void en7571_tx_power_alarm_get(void)
{
	if(en7571_DDMI_tx_power < en7571_tx_power_low_alarm_thod)
		en7571_alarm |= PHY_TRANS_TX_LOW_POWER_ALARM;
	if(en7571_DDMI_tx_power > en7571_tx_power_high_alarm_thod)
		en7571_alarm |= PHY_TRANS_TX_HIGH_POWER_ALARM;
	if(en7571_DDMI_current < en7571_tx_cur_low_alarm_thod)
		en7571_alarm |= PHY_TRANS_TX_LOW_CUR_ALARM;
	if(en7571_DDMI_current > en7571_tx_cur_high_alarm_thod)
		en7571_alarm |= PHY_TRANS_TX_HIGH_CUR_ALARM;
}
/*****************************************************************************
//Function :
//		en7571_rx_power_alarm
//Description :
//		This function is to get BOB Rx alarm.
//Input :
//		N/A
//Output :
//		EN7571 Rx alarm status.
//Date :20150414 by YMC
//
******************************************************************************/
void en7571_rx_power_alarm_get(void)
{
	if(en7571_DDMI_rx_power < en7571_rx_power_low_alarm_thod)
		en7571_alarm |= PHY_TRANS_RX_LOW_POWER_ALARM;
	if(en7571_DDMI_rx_power > en7571_rx_power_high_alarm_thod)
		en7571_alarm |= PHY_TRANS_RX_HIGH_POWER_ALARM;
}
/*****************************************************************************
//Function :
//		en7571_EN7571_detection
//Description :
//		This function is to detect EN7571
//Input :
//		N/A
//Output :
//		0 for EN7571 not detected
//		1 for EN7571 detected
//Date :
//		2015/03/02 by HC
******************************************************************************/
int en7571_EN7571_detection(void)
{
	unchar ptr[1];
	UINT8 read_data_1 = 0;
	UINT8 read_data_2 = 0;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_FT_ADC_CLK_CLR, ptr, 1);
	read_data_1 = ptr[0];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 1);
	read_data_2 = ptr[0];

	if( (read_data_1 == 0x03) && (read_data_2 >= 0x03) )
	{
		en7571_select = 1;

		if(read_data_2 == 0x03)
			en7571_ver = 2;

		return 1;
	}
	else
		return 0;
}
/*****************************************************************************
//Function :
//		en7571_LOS_calibration
//Description :
//		This function is to calibrate LOS level
//Input :
//		High/Low threshold
******************************************************************************/
int en7571_LOS_calibration(int LOS_thld_H, int LOS_thld_L)
{
	unchar ptr[4];
	int LOS_status = 0;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | LOS_thld_L;
	ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | LOS_thld_H;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	mdelay(20);

	// LOS status
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_DBG_RG, ptr, 4);
	LOS_status = ptr[3] & 0x01;
	return LOS_status;
}


/*****************************************************************************

******************************************************************************/
float en7571_Taylor_series(float input)
{
	int i = 0;
	float temp 		= 0;
	float temp_2 	= 0;
	float temp_3 	= 0;

	for ( i=0; i<7 ; i++) 		// find resolution of input 0.5 <= x < 5
	{
		if ( input >= 5 )
			input = input/10;
		else
			break;
	}

	temp = (input-1)/input;		// ( x-1)/x
	temp_2 = temp*temp;
	temp_3 = temp*temp_2;

	temp = temp + temp_2/2 + temp_3/3 + (temp_2*temp_2)/4 + (temp_2*temp_3)/5 + (temp_3*temp_3)/6;	// lnX
	temp = temp/LN10;				// logX = lnX / ln10
	temp -= 4-i;						// to log (mW)
	temp *= 10; 						// 10*logX

	return temp;
}

/*****************************************************************************
//Function :
//		en7571_BoB_info
//Description :
//		This function is to show BoB common information
******************************************************************************/
void en7571_BoB_info(void)
{
	unchar ptr[4];
	float temp = 0;
	int a = 0;
	int b = 0;

	// Driver version
	printk("Driver version = %d\n", en7571_version);

	//LOS
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_DBG_RG, ptr, 4);
	ptr[3] = ptr[3] & 0x1;
	printk("LOS = %d\n", ptr[3]);

	//Tx SD
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_ROGUE_ONU_DET_CTRL, ptr, 2);
	ptr[1] = (ptr[1]>>3 & 0x1);
	printk("Tx SD = %d\n", ptr[1]);

	//Tx fault
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SAFE_PROTECT, ptr, 2);
	ptr[1] = (ptr[1]>>7 & 0x1);
	printk("Tx fault = %d\n", ptr[1]);

	//Bias current
	temp = (float)en7571_info(SELECT_IBIAS_NOW)*bias_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("I bias = %d.%.2dmA\n", a, b);

	//Modulation current
	temp = (float)en7571_info(SELECT_IMOD_NOW)*mod_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("I mod = %d.%.2dmA\n", a, b);

	//Average current
	temp = (float)en7571_info(SELECT_IAV_NOW)*av_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("I av = %d.%.2dmA\n", a, b);

	//IC, BOSA, and enviroment temperature
	a = (int)en7571_IC_temperature;
	b = abs( (int)(en7571_IC_temperature*10)%10 );
	printk("IC temperature = %d.%d'C\n", a, b);
	a = (int)en7571_BOSA_temperature;
	printk("BOSA temperature = %d.%d'C\n", a, b);

	//APD voltage
	a = (int)en7571_APD_voltage;
	b = (int)(en7571_APD_voltage*10)%10;
	printk("APD Voltage = %d.%dV\n", a, b);

	// VCC
	a = (int)en7571_DDMI_voltage/10000;
	b = (int)en7571_DDMI_voltage%10000;
	printk("VCC = %d.%.2dV\n", a, b);

	// Tx power
	if( en7571_DDMI_tx_power == 0 )
		printk("Tx power = -40 dBm\n");
	else
	{
		temp = en7571_Taylor_series( (float)en7571_DDMI_tx_power );
		a = (int)temp;
		b = abs( (int)(temp*100)%100 );
		printk("Tx power = %d.%.2d dBm\n", a, b);
	}

	// Rx power by YW_20160517
	if( en7571_DDMI_rx_power == 0 )
		printk("Rx power = -40 dBm\n");
	else
	{
		temp = en7571_Taylor_series( (float)en7571_DDMI_rx_power );
		a = (int)temp;
		b = abs( (int)(temp*100)%100 );
		printk("Rx power = %d.%.2d dBm\n", a, b);
	}
}


/*****************************************************************************
//function :
//		en7571_LOS_init
//description :
//		this function is to initialize LOS function
******************************************************************************/
void en7571_LOS_init(void)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);
	ptr[0] = (ptr[0] & (LOS_calibration_mask)) | (LOS_calibration_trig);
	// Set rg_ain_stable_cnt
	ptr[1] = (ptr[1] & (LOS_ain_stable_cnt_mask)) | (LOS_ain_stable_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_ADCREV2_mask)) | (LOS_ADCREV2_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 4);
	ptr[2] = (ptr[2] & (LOS_ADCREV1_mask)) | (LOS_ADCREV1_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL2, ptr, 4);
	ptr[1] = (ptr[1] & (LOS_confidence_mask)) | (LOS_confidence_setting);
	// Set rg_los_sd_cnt
	ptr[0] = (ptr[0] & (LOS_cnt_mask)) | (LOS_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL2, ptr, 4);
}


/*****************************************************************************
//Function :
//		en7571_fine_tune_PWR_ER
//Description :
//		This function is to fine-tune Tx power and ER
//Input :
//		1 or 0
******************************************************************************/
void en7571_fine_tune_PWR_ER(int pwr_er,int up_dn)
{
	unchar ptr[4];
	int pav = 0;
	int p1 = 0;

	en7571_ERC 	= 0; // Disable ERC
	en7571_KT 	= 0; // Disable SWKT

	pav = en7571_info(SELECT_PAV_CAL);
	p1 = en7571_info(SELECT_P1_CAL);

	if( pwr_er )		// Power
	{
		if( up_dn )		// Tune up
			pav += 0x004;
		else				// Tune down
			pav -= 0x004;

		// Overflow/Underflow Protection
		if( pav > 0xfff )
			pav = 0xfff;
		else if( pav < 0 )
			pav = 0;
	}
	else					// ER
	{
		if( up_dn )		// Tune up
			p1 += 0x001;
		else				// Tune down
			p1 -= 0x001;

		// Overflow/Underflow Protection
		if( p1 > 0x3ff )
			p1 = 0x3ff;
		else if( p1 < 0 )
			p1 = 0;
	}
	// Change Pav
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_PWR_CTRL_D, ptr, 4 );
	ptr[0] = pav;
	ptr[1] = (pav >> 8);
	ptr[2] = p1;
	ptr[3] = (p1 >> 8);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  en7571_PWR_CTRL_D, ptr, 4 );
#if 0
	en7571_force_mode();
#endif
}


/*****************************************************************************
//Function :
//		en7571_SWKT
//Description :
//		This function is SWKT
******************************************************************************/
void en7571_SWKT(void)
{
	int need_ERC		= 0;
	int Imod_delta		= 0;
	int ratio_now		= 0;

	UINT32 Iav_cal		= 0;
	UINT32 Imod_cal		= 0;
	UINT32 Imod_now		= 0;
	UINT32 MPDH_cal		= 0;
	UINT32 Imod_target	= 0;
	UINT32 factor_HT 	= 0;
	UINT32 factor_LT 	= 0;

	float Iav_delta	= 0;

	static int activated;

	Iav_cal 		= (get_flash_register(en7571_flash_Iav_Imod)&FLASH_IAV_MASK) >> 16;
	ratio_now 	= ( 100*en7571_info(SELECT_IAV_NOW) )/Iav_cal - 100;
	Imod_cal 	= get_flash_register(en7571_flash_Iav_Imod)&FLASH_IMOD_MASK;
	Imod_now		= en7571_info(SELECT_IMOD_NOW);
	factor_HT 	= get_flash_register(en7571_flash_KT)&flash_lower_16_bits;
	factor_LT 	= (get_flash_register(en7571_flash_KT)&flash_upper_16_bits) >> 16;


	Iav_delta = (float)en7571_info(SELECT_IAV_NOW) - (float)Iav_cal;

	if( ratio_now >= 0 )						// HT
	{
		if( en7571_KT == 3 )
		{
			Imod_target = (UINT32)( (float)Imod_cal*( 0x400 + (float)ratio_now*(float)factor_HT) );
			Imod_target = Imod_target>>12;	// Because of 0x400
		}
		else
		{
			Imod_target = (UINT32)( (float)Imod_cal*( 0x20000 + Iav_delta*(float)factor_HT) );
			Imod_target = Imod_target>>17;	// Because of 0x20000
		}

		Imod_delta = (int)Imod_target - (int)Imod_now;
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTFACTORHT = 0x%x\n", (int)factor_HT);
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTTARGET = 0x%03x\n", Imod_target);
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTDELTA = %d\n", Imod_delta);

		if( Imod_delta > SWKT_IMOD_EPISODE )
		{
			MPDH_cal = en7571_info(SELECT_P1_CAL) + 1;
			need_ERC = 1;
		}
		else if( (-1)*Imod_delta > SWKT_IMOD_EPISODE )
		{
			MPDH_cal = en7571_info(SELECT_P1_CAL) - 1;
			need_ERC = 1;
		}
	}
	else					// LT
	{
		if( en7571_KT == 1 )
		{
			Imod_target = (UINT32)( (float)Imod_cal*( 0x20000 + Iav_delta*(float)factor_LT) );
			Imod_target = Imod_target>>17;	// Because of 0x20000
		}
		else if( en7571_KT == 2 )
		{
			Imod_target = (UINT32)( (float)Imod_cal*( 0x20000 - Iav_delta*(float)factor_LT) );		// Inverse
			Imod_target = Imod_target>>17;	// Because of 0x20000
		}
		else if( en7571_KT == 3 )
		{
			Imod_target = (UINT32)( (float)Imod_cal*( 0x400 + (float)ratio_now*(float)factor_LT) );
			Imod_target = Imod_target>>12;	// Because of 0x400
		}

		Imod_delta = (int)Imod_target - (int)Imod_now;
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTFACTORLT = 0x%x\n", (int)factor_LT);
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTTARGET = 0x%03x\n", Imod_target);
		PON_PHY_MSG(PHY_MSG_DBG, "SWKTDELTA = %d\n", Imod_delta);

		if( Imod_delta > SWKT_IMOD_EPISODE )
		{
			MPDH_cal = en7571_info(SELECT_P1_CAL) + 1;
			need_ERC = 1;
		}
		else if( (-1)*Imod_delta > SWKT_IMOD_EPISODE )
		{
			MPDH_cal = en7571_info(SELECT_P1_CAL) - 1;
			need_ERC = 1;
		}
	}

	if( (gpPhyPriv->phyCfg.flags.trafficStatus!=TRAFFIC_STATUS_DOWN) && (en7571_ver==2) )
	{
		if(Imod_now>=0x800)	// zone 3
		{
			if(Imod_target>0x810)
			{
				en7571_change_Imod(Imod_target);
			}
			else if(Imod_target<0x7f0)
			{
				en7571_cross_Imod(Imod_target);
			}
		}
		else if( (Imod_now>=0x400) && (Imod_now<0x800) )		// zone 2
		{
			if( (Imod_target>0x410) && (Imod_target<0x7f0) )
			{
				en7571_change_Imod(Imod_target);
			}
			else if(Imod_target>0x810)
			{
				en7571_cross_Imod(Imod_target);
			}
			else if(Imod_target<0x3f0)
			{
				en7571_cross_Imod(Imod_target);
			}
		}
		else if(Imod_now<0x400)	// zone 1
		{
			if(Imod_target<0x3f0)
			{
				en7571_change_Imod(Imod_target);
			}
			else if(Imod_target>0x410)
			{
				en7571_cross_Imod(Imod_target);
			}
		}


		if(en7571_info(SELECT_IBIAS_NOW)==0x080)
		{
			en7571_cross_Imod(Imod_target);
		}


	}
	else if( (en7571_PatternEnabled==1) || (gpPhyPriv->phyCfg.flags.trafficStatus!=TRAFFIC_STATUS_DOWN) )
	{
		if( (ratio_now<-2) || (ratio_now>15) )
		{
			if( need_ERC == 1 )
			{
				en7571_change_mpdh(MPDH_cal);
				activated = 1;
			}
		}
		else
		{
			if( activated == 1 )		// If MPDH has been changed
			{
				MPDH_cal = get_flash_register(en7571_flash_Pav_P1)&FLASH_P1_MASK;
				en7571_change_mpdh(MPDH_cal);
				activated = 0;			// Reset "activated" flag, and change MPDH back to MPDH_cal
			}
		}
	}
}


/*****************************************************************************

******************************************************************************/
void en7571_change_Imod(UINT32 input)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4);
	ptr[3] = (ptr[3] & 0xf0) | (input >> 8);
	ptr[2] = (input & 0xff);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9, ptr, 4);
}


/*****************************************************************************

******************************************************************************/
void en7571_change_mpdh(UINT32 input)
{
	unchar ptr[4];
#if 0
	// Force mode
	en7571_force_mode();
#endif
	// Set variation of Imod
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
	ptr[1] = 0x00;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);

	// Change MPDH
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4 );
	ptr[2] = input;
	ptr[3] = input>>8;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_D, ptr, 4 );

	// Set variation of Imod
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
	ptr[1] = rg_pwr_ctrl_delta_ImodMax_setting;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_8, ptr, 4);
}


/*****************************************************************************

******************************************************************************/
void en7571_cross_Imod(UINT32 input)
{
	unchar ptr[2];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9+0x002, ptr, 2);
	ptr[1] = (ptr[1] & 0xf0) | (input >> 8);
	ptr[0] = (input & 0xff);
	if( en7571_cross == 1 )
	{
		// SIF_X_Imod_Write(0, 0x60, 0x70, 2, en7571_PWR_CTRL_9+0x002, ptr, 2);
		// en7571_cross_cnt++;
		mbi_ctrl_set(0);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, EN7571_IMOD_DAC, ptr,2);
		mbi_ctrl_set(1);
	}
	else
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_9+0x002, ptr, 2);
}


/*****************************************************************************
//Function :
//		en7571_ER_Ctrl
//Description :
//		This function is ER-Controller
******************************************************************************/
void en7571_ER_Ctrl(void)
{
	int need_ERC		= 0;
	int MPDH_diff		= 0;
	int ratio_now		= 0;

	UINT32 Iav_cal		= 0;
	UINT32 MPDH_cal		= 0;

	static int activated;

	UINT16 slope_dn	= (get_flash_register(en7571_flash_ERC)>>24)&FLASH_ERC_MASK;
	UINT16 slope_up	= (get_flash_register(en7571_flash_ERC)>>16)&FLASH_ERC_MASK;
	UINT16 ratio_dn	= (get_flash_register(en7571_flash_ERC)>>8)&FLASH_ERC_MASK;
	UINT16 ratio_up	= get_flash_register(en7571_flash_ERC)&FLASH_ERC_MASK;

	Iav_cal = (get_flash_register(en7571_flash_Iav_Imod)&FLASH_IAV_MASK) >> 16;
	ratio_now = ( 100*en7571_info(SELECT_IAV_NOW) )/Iav_cal - 100;

	MPDH_cal = get_flash_register(en7571_flash_Pav_P1)&FLASH_P1_MASK;


	if( ratio_up==0 || ratio_dn==0 )
		return;

	if( ratio_now >= ratio_up )
	{
		MPDH_diff = (int)((float)ratio_now*(float)slope_up*0.01);
		MPDH_cal += MPDH_diff;
		activated = 1;
		need_ERC = 1;
	}
	else if( ratio_now <= (-1*ratio_dn) )
	{
		MPDH_diff = (int)((float)ratio_now*(float)slope_dn*0.01);
		MPDH_cal += MPDH_diff;
		activated = 1;
		need_ERC = 1;
	}
	else
	{
		if( activated == 1 )		// If MPDH has been changed
		{
			MPDH_cal = get_flash_register(en7571_flash_Pav_P1)&FLASH_P1_MASK;
			activated = 0;			// Reset "activated" flag, and change MPDH back to MPDH_cal
			need_ERC = 1;
		}
	}


	if( need_ERC == 1 )
		en7571_change_mpdh(MPDH_cal);

}


/*****************************************************************************
//Function :
//		en7571_show_ERC_ratio
//Description :
//		This function is for ERC debugging
******************************************************************************/
void en7571_show_ERC_ratio(void)
{
	int ratio_now		= 0;
	int Iav_cal			= 0;
	int MPDH_diff		= 0;
	UINT32 MPDH_cal		= 0;

	Iav_cal = ( get_flash_register(en7571_flash_Iav_Imod)&FLASH_IAV_MASK) >> 16;
	ratio_now = 100*( (int)en7571_info(SELECT_IAV_NOW)-Iav_cal )/Iav_cal;
	printk("ratio = %d \n",ratio_now);

	MPDH_cal = get_flash_register(en7571_flash_Pav_P1)&FLASH_P1_MASK;
	MPDH_diff = en7571_info(SELECT_P1_CAL) - MPDH_cal;
	printk("offset = %d\n", MPDH_diff);
}


/*****************************************************************************
//function :
//		en7571_VBR_find
//description :
//		this function is to find APD voltage DAC for Vbr
//input :
//	       High : decision High bound , Low : decision Low bound

******************************************************************************/
	void en7571_VBR_find(int High , int low)
{
	int i			= 0 ;
	int APD_VBR	= 0 ;
	unchar ptr[4];
	uint RSSI_voltage	= 0x9000;
	uint cnt 				= 85;
	unchar APD_DAC[1];
	int read = 0 ;
	int APD_low =0 ;
	int APD_high = 255 ;
	int counter = 1 ;

		en7571_APD_initialization();

		APD_DAC[0] = 0x7f ;
		while(RSSI_voltage < 0x1000 | RSSI_voltage> 0x6000)
		{
			en7571_APD_I2C_write(APD_DAC);
			mdelay(50);
			RSSI_voltage = en7571_dark_current();


			//printk("-------counter = %d-------\n", counter);
			//printk("APD_DAC = 0x%x\n", APD_DAC[0]);
			//printk("RSSI_voltage = 0x%x\n", RSSI_voltage);
			//printk("APD_high = 0x%x , APD_low = 0x%x\n", APD_high,APD_low);


			read = APD_DAC[0] ;
			if (RSSI_voltage< low)
				{
					APD_DAC[0] = (APD_low + read)/2 ;
					APD_high = read;
				}

			if(RSSI_voltage > High)
				{
					APD_DAC[0] = (APD_high + read)/2 ;
					APD_low = read ;
				}


			counter ++;

			if (counter == 15)
				{
					//printk("VBR is not found,Error \n ");
					break;
				}
		}

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 1);
		printk("APD_VBR = 0x%x\n", ptr[0]);



	#if 0
	for( i=0; i<cnt; i++ )  // find Vbr point
	{
		APD_DAC[0] +=  0x3 ;
		en7571_APD_I2C_write(APD_DAC);
		mdelay(50);

		RSSI_voltage = en7571_dark_current();

		if(RSSI_voltage < 0x6000)
			break ;
		else
			RSSI_voltage = 0 ;
	}


	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_APD_DAC_CODE, ptr, 1);

	if (i == cnt)
		printk("VBR is not found, error \n ");
	else
			printk("APD_VBR = 0x%x\n", ptr[0]);
	#endif

}


/*****************************************************************************
//function :
//		en7571_LOS_find
//description :
//		this function is to find LOS  thershold
******************************************************************************/
void en7571_LOS_find(int LOS_find)
{
	unchar ptr[4];
	int i 				= 0 ;
	int cnt 			= 127 ;
	int LOS_status	= 0 ;
	int LOS_offset	= 1 ;
	int read			= 0 ;
	UINT8 LOS_thld_L	= 0x00;
	UINT8 LOS_thld_H	= 0x7f;
	int DAC = 0 ;
	int counter = 0 ;

	if (LOS_find == 0)
	{
		// Read Initial LOS_high data
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1+(0x02), ptr, 1);
		read = ptr[0] ;

		// Set LOS = 0
		en7571_LOS_calibration(0x00, 0x00);
		mdelay(100);

		// Set find LOS_low initial setting
		LOS_status = en7571_LOS_calibration(0x7f, 0x00);

		if (LOS_status ==1 ) // if LOS is high, can not find real LOS_low
		{
			printk("LOS is high, error\n");
		}

		DAC = 0x3f ; //LOS  initial

		// Dichotomy
		while (1)
		{
			LOS_status = en7571_LOS_calibration(0x7F, DAC);
			mdelay(20);
			//printk("========counter = %d========\n", counter);
			//printk("LOS_DAC = 0x%x\n", DAC);
			//printk("LOS_high = 0x%x , LOS_low = 0x%x\n", LOS_thld_H,LOS_thld_L);
			//printk("LOS_status = %d\n", LOS_status);

			if (LOS_status ==1)
				{
					LOS_thld_H = DAC ;
					DAC =  (LOS_thld_L + DAC)/2 ;
				}
			if (LOS_status ==0)
				{
					LOS_thld_L = DAC ;
					DAC = (LOS_thld_H + DAC)/2 ;
				}
			counter ++;
			en7571_LOS_calibration(0x00, 0x00);	 // Set LOS = 0
			if (counter == 8)
				break;
		}


		printk("Rx LOS = 0x%x \n", DAC);
		en7571_LOS_calibration(0x00, DAC);

		// Step forward
		#if 0
		// Find LOS_low  loop
		for(i=0;i<cnt;i++)
		{
			LOS_status = en7571_LOS_calibration(0x7f, LOS_thld_L);

			if (LOS_status ==1)
			{
				printk("Rx LOS = 0x%x \n", LOS_thld_L);
				break ;
			}
			LOS_thld_L += LOS_offset;
		}


		if (LOS_status == 0)
		{
			printk("Rx LOS is not found, error\n");
		}
		#endif

		// Recover LOS_high setting
		ptr[0] = (ptr[0] & (LOS_comp_thld_mask)) | read;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1+(0x02) , ptr, 1);

	}
	else
	{
		// Read Initial LOS_low data
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1+(0x03), ptr, 1);
		read = ptr[0] ;

		// Set LOS = 1
		en7571_LOS_calibration(0x7f, 0x7f);
		mdelay(100);

		// Set find LOS_high initial setting
		LOS_status = en7571_LOS_calibration(0x7f, 0x00);

		if (LOS_status == 0 ) // if LOS is low, can not find real LOS_high
		{
			printk("LOS is low, error\n");
		}


		DAC = 0x3F ; //LOS  initial

		// Dichotomy
		while (1)
		{
			LOS_status = en7571_LOS_calibration(DAC, 0x00);
			mdelay(20);
			//printk("========counter = %d========\n", counter);
			//printk("LOS_DAC = 0x%x\n", DAC);
			//printk("LOS_high = 0x%x , LOS_low = 0x%x\n", LOS_thld_H,LOS_thld_L);
			//printk("LOS_status = %d\n", LOS_status);

			if (LOS_status ==1)
				{
					LOS_thld_H = DAC ;
					DAC =  (LOS_thld_L + DAC)/2 ;
				}
			if (LOS_status ==0)
				{
					LOS_thld_L = DAC ;
					DAC = (LOS_thld_H + DAC)/2 ;
				}
			counter ++;
			en7571_LOS_calibration(0x7F, 0x7F);	 // Set LOS = 1

			if (counter == 12)
				break;
		}

		printk("Rx SD = 0x%x \n", DAC);
		en7571_LOS_calibration(DAC, 0x00);

		//Step forward
		#if 0
		// Find LOS_high loop
		for(i=0;i<cnt;i++)
		{
			LOS_status = en7571_LOS_calibration(LOS_thld_H, 0x00);

			if (LOS_status == 0)
			{
				printk("Rx SD = 0x%x \n", LOS_thld_H);
				break ;
			}
			LOS_thld_H -= LOS_offset;
		}

		if (LOS_status == 1)
		{
			printk("Rx SD is not found, error\n");
		}
		#endif
		// Recover LOS_low setting
		ptr[0] = (ptr[0] & (LOS_comp_thld_mask)) | read;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_LOS_CTRL1+(0x03) , ptr, 1);

	}
}


/*****************************************************************************
//Function :
//		en7571_eFuse_temperature_get
//Description :
//		This function is to get eFuse temperature offset
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2016/12/14 by YW
//
******************************************************************************/
void en7571_eFuse_temperature_get(void)
{
	int a 		= 0 ;
	int b 		= 0 ;
	int read = 0 ;
	unchar ptr[4];

	// Read plus for eFuse
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_eFuse_Temp, ptr, 4);
	ptr[2] = (ptr[2]&0xfe) | 0x01;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_eFuse_Temp, ptr, 4);
	mdelay(50);
	// read eFuse value
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_RG_eFuse_Temp, ptr, 4);
	read = ptr[0] ;

	if (read <127)
		en7571_eFuse_temperature_offset = (float)read *0.3;
	else
		en7571_eFuse_temperature_offset = (-0.3)*(float)(temperature_two_complement_efuse-read);

	printk("eFuse value = 0x%x\n", ptr[0]);

	a = (int)en7571_eFuse_temperature_offset;
	b = abs( (int)(en7571_eFuse_temperature_offset*10)%10 );
	printk("eFuse offset = %d.%d'C\n", a, b);

}


/*****************************************************************************

******************************************************************************/
void en7571_force_mode(void)
{
	unchar ptr[4];

	// Force mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_C, ptr, 4);
	ptr[3] = (ptr[3] & rg_pwr_ctrl_Pav_P1_cal_force_mask) | rg_pwr_ctrl_Pav_P1_cal_force_enable;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_C, ptr, 4);
}


/*****************************************************************************

******************************************************************************/
void en7571_auto_lock_mode(void)
{
	unchar ptr[4];

	// Auto-locked mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_C, ptr, 4);
	ptr[3] = (ptr[3] & rg_pwr_ctrl_Pav_P1_cal_force_mask);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_C, ptr, 4);
}


/*****************************************************************************

******************************************************************************/
void en7571_tune_KT(UINT16 input)
{
	unchar ptr[4];

	en7571_ERC 	= 0; // Disable ERC
	en7571_KT 	= 0; // Disable SWKT

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_2, ptr, 3);
	ptr[2] = (ptr[1] & 0xfc) | (input>>8);
	ptr[1] = (ptr[0] & 0x00) | (input&0xff);
	ptr[0] = (ptr[0] & RG_PWR_CTRL_IMOD_ADJ_SEL_MASK) | RG_PWR_CTRL_IMOD_ADJ_SEL_KT;		// rg_pwr_ctrl_e3_Imod_Adj_sel = 1
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_PWR_CTRL_2, ptr, 3);
}


/*****************************************************************************

******************************************************************************/
int en7571_isDCLworking(void)
{
	static UINT32 info_array[3];
	UINT32 info_array_last[3];
	int i = 0;

	for(i=0;i<3;i++){
		info_array_last[i] = info_array[i];}

	info_array[0] = en7571_info(SELECT_IBIAS_NOW);
	info_array[1] = en7571_info(SELECT_IMOD_NOW);
	info_array[2] = en7571_info(SELECT_IAV_NOW);

	if( (info_array_last[0]==info_array[0]) && (info_array_last[1]==info_array[1]) && (info_array_last[2]==info_array[2]) )
		return 0;
	else
		return 1;
}


/*****************************************************************************

******************************************************************************/
void en7571_config(void)
{
	// Check internal DDMI
	if( get_flash_register(en7571_flash_internal_DDMI) == 0 )
	{
		en7571_internal_DDMI = 0;
		printk("Internal DDMI Disabled\n");
	}
	else
	{
		printk("Internal DDMI Enabled\n");
		if( get_flash_register(en7571_flash_internal_DDMI) == 2 )		// by HC 20160517
			en7571_fast_DDMI = 1;
	}




	if( get_flash_register(en7571_flash_ERC) != flash_empty )			// Check ERC
	{
		en7571_ERC = 1;
		printk("ERC Enabled\n");
	}
	else																					// Check KT
	{
		if( get_flash_register(en7571_flash_KT) == flash_empty )
		{
			set_flash_register(0x00060020, en7571_flash_KT);			// default = 0x00060020
			en7571_KT = 1;
			printk("KT Enabled\n");
		}																					// user input
		else
		{
			en7571_KT = 1;
			printk("KT Enabled\n");
		}

		if( get_flash_register(en7571_flash_KT_switch) == 2 )			// Check KT inverse
		{
			en7571_KT = 2;
			printk("KT Inversed\n");
		}
		else if( get_flash_register(en7571_flash_KT_switch) == 3 )		// Check enhanced KT
		{
			en7571_KT = 3;
			printk("KT Enhanced\n");
		}
	}

	en7571_delay1 = 0;
	en7571_delay2 = 600;

	if( get_flash_register(en7571_flash_crs) == 0 )
		en7571_cross = 0;

}


/*****************************************************************************

******************************************************************************/
void en7571_link_reg(int input)
{
	if(en7571_ver==2)
	{
		if(input==ENABLE)
			en7571_mpdh_stepsize(DISABLE);
		else if(input==DISABLE)
			en7571_mpdh_stepsize(ENABLE);
	}
	else
		en7571_burst_ctrl(input);
}


/*****************************************************************************

******************************************************************************/
void en7571_burst_ctrl(int input)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);
	if( input == ENABLE )
		ptr[1] = ( ptr[1] & RG_DUMMY_BURST_CTRL_MASK );
	else if( input == DISABLE )
		ptr[1] = ( ptr[1] & RG_DUMMY_BURST_CTRL_MASK ) | RG_DUMMY_BURST_CTRL_OFF;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, en7571_DUMMY, ptr, 4);
}




