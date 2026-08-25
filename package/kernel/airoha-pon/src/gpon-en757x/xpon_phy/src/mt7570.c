#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "i2c.h"
#include "phy_def.h"
#include "mt7570_reg.h"
#include "mt7570_def.h"
#include "phy_global.h"
#include "phy_api.h"
#include "phy_types.h"
#include "phy_debug.h"
#include "phy.h"
#include "phy_init.h"
#include "phy_tx.h"

int mt7570_version = 108;				//The version of mt7570.c in main trunk by HC 20150320

int internal_DDMI = 1;					//by HC 20150311
int fast_DDMI		= 0;					//by HC 20160517
int TEC				= 1;					// by HC 20150513
int TEC_switch		= 0;					// by HC 20150515
int xPON_mode 		= -1; 					// by HC 20150520
int mt7570_select = 0;						//by HC 20150301
int vEN7570   = 1;							//by HC 20150301
int BG1V76    = 0;
int BG0V875   = 0;
int RSSI_Vref = 0;							// by HC 20150609
int RSSI_V    = 0;							// by HC 20150609
int ETC		  = 0;							// by YM 20150731
int FiberPlug = 0;						// by HC 20151106
int PatternEnabled = 0;					// by HC 20151123

float RSSI_factor = 0;						// by HC 20150609
int delay_cnt = 0;
uint cnt7570 	= 0;						// by HC 20150727
int MPD_current_cnt             = 0;		//by HC 20150303
int T0C_extension 				= 0;		// by HC 20150512
float IC_temperature			= 25.0; 	//by HC 20150326
float BOSA_temperature			= 20.0; 	//by HC 20150326
float Env_temperature			= 25.0;	//by HC 20151006
float APD_voltage    			= 35.0;		//by HC 20150326
float ADC_slope					= 0;		//by HC 20150603
float ADC_offset				= 0;		//by HC 20150603

int global_temperature_code   	= 0;
int global_vcc_code           	= 0;

uint global_bias_current       	= 0;
uint global_MPDL					= 0;		// by HC 20150505
uint global_MPDH					= 0;		// by HC 20150505
uint global_modulation_current 	= 0;

int global_rssi_current       	= 0;
int global_mpd_current        	= 0;
int global_RSSI_current	  	  	= 0;
int global_MPD_current	 	  	= 0;
int global_MPD_current_var		= 0;		//by HC 20150226
int global_DDMI_rx_cal_flag		=0;		//biker_20151001

UINT16 global_supply_voltage_8472 = 0;		//by HC 20150311
UINT16 global_IC_temperature_8472 = 0;		//by HC 20150311
UINT16 global_bias_current_8472	  = 0;		//by HC 20150311
UINT16 DDMI_voltage     = 0;				//by HC 20150311
UINT16 DDMI_current     = 0;				//by HC 20150311
UINT16 DDMI_temperature = 0;				//by HC 20150311
UINT16 DDMI_tx_power    = 0;		//by HC 20150310
UINT16 DDMI_rx_power    = 0;		//by HC 20150310

int Ibias_outputlow_flag = 0;

uint MPD_current_offset = 0;		//by HC 20150117
uint T_APD = 600;					//APD voltage update time. YMC 20150122
uint flash_matrix[100];
uint mt7570_alarm = 0;				//by YMC 20150414
uint SCL =0 ;                 	//Single open loop flag, Jayyc 20150902
uint DOL =0 ;                  //Dual open loop flag, Jayyc 20150915
uint BiasTracking_switch = 0;		// by HC 20150921
uint BOSA_Ith = 3000;					// 3000uA by HC 20150921

uint LUT_Ibias_Imod[64][2] = 		//I_bias & I_mod look-up-table by HC
{
//	{bias,  modulation}
	{0x106, 0x3b2},	//-40
	{0x10a, 0x3b2},	//-37.5
	{0x10f, 0x3b6},	//-35
	{0x118, 0x3bb},	//-32.5
	{0x11d, 0x3bb},	//-30
	{0x121, 0x3c0},	//-27.5
	{0x12a, 0x3c4},	//-25
	{0x12f, 0x3c4},	//-22.5
	{0x133, 0x3c9},	//-20
	{0x13c, 0x3cd},	//-17.5
	{0x141, 0x3cd},	//-15
	{0x146, 0x3d2},	//-12.5
	{0x14e, 0x3d6},	//-10
	{0x157, 0x3d8},	//-7.5
	{0x164, 0x3e4},	//-5
	{0x171, 0x3ed},	//-2.5
	{0x17e, 0x3f6},	//0
	{0x18b, 0x3ff},	//2.5
	{0x198, 0x408},	//5
	{0x1a5, 0x411},	//7.5
	{0x1ad, 0x416},	//10
	{0x1ba, 0x41f},	//12.5
	{0x1c7, 0x428},	//15
	{0x1d4, 0x431},	//17.5
	{0x1e1, 0x43a},	//20
	{0x1ee, 0x444},	//22.5
	{0x1fb, 0x44d},	//25
	{0x214, 0x45a},	//27.5
	{0x223, 0x468},	//30
	{0x236, 0x476},	//32.5
	{0x245, 0x488},	//35
	{0x258, 0x495},	//37.5
	{0x268, 0x4a3},	//40
	{0x277, 0x4b1},	//42.5
	{0x28a, 0x4c3},	//45
	{0x299, 0x4d1},	//47.5
	{0x2ac, 0x4de},	//50
	{0x2bb, 0x4ec},	//52.5
	{0x2cf, 0x4fe},	//55
	{0x2fe, 0x510},	//57.5
	{0x332, 0x527},	//60
	{0x362, 0x53e},	//62.5
	{0x396, 0x555},	//65
	{0x3c6, 0x56b},	//67.5
	{0x3fa, 0x582},	//70
	{0x46b, 0x5b9},	//72.5
	{0x4dd, 0x5ef},	//75
	{0x54e, 0x626},	//77.5
	{0x5c4, 0x65c},	//80
	{0x635, 0x693},	//82.5
	{0x666, 0x6ce},	//85
	{0x666, 0x6ce},	//87.5
	{0x666, 0x6ce},	//90
	{0x666, 0x6ce},	//92.5
	{0x666, 0x6ce},	//95
	{0x666, 0x6ce},	//97.5
	{0x666, 0x6ce},	//100
	{0x666, 0x6ce},	//102.5
	{0x666, 0x6ce},	//105
	{0x666, 0x6ce},	//107.5
	{0x666, 0x6ce},	//110
	{0x666, 0x6ce},	//112.5
	{0x666, 0x6ce},	//115
	{0x666, 0x6ce}		//117.5
};


/*****************************************************************************
	I2C Protection
******************************************************************************/
atomic_t i2c_protect = ATOMIC_INIT(0);

int
i2c_access_protect(void){
#define MAX_I2C_LOOP	10000
	int count = MAX_I2C_LOOP;
	while(atomic_read(&i2c_protect) == 1 ){
		udelay(1);
		if(count==0) return -1;
		count --;
	}
	return 0;
}


/*****************************************************************************
//function :
//		mt7570_register_dump
//description :
//		this function is used to get mt7570 registers
//input :
//		register length
//output :
//		N/A
//
******************************************************************************/
void mt7570_register_dump(int length)
{
	int i = 0;
	unchar ptr[4];
	uint return_value = 0;
	for(i = 0 ; i < length ; i++)
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (i<<2), ptr, 4);
		return_value = ptr[0] | (ptr[1]<<8) | (ptr[2]<<16) | (ptr[3]<<24);
		printk("EN7570 reg 0x%.3x", i*4);
		printk("\t%.8x\n", return_value);
	}
}


/*****************************************************************************
//function :
//		get_flash_register
//description :
//		this function is used to get the flash_matrix registers
//input :
//		register offset
//output :
//		register value
//
******************************************************************************/
int get_flash_register(int address_offset)
{
	uint return_int = 0;
	return_int = flash_matrix[(address_offset>>2)];
	return return_int;
}

/*****************************************************************************
//function :
//		get_flash_matrix
//description :
//		this function is used to read the registers from 7570_bob.conf to flash_matrix
//input :
//		N/A
//output :
//		0 : success ; -1 : fail
//
******************************************************************************/
int get_flash_matrix(void)
{
	struct file *f;
	loff_t pos = 0;
	ssize_t ret;
	const char *path = "/tmp/7570_bob.conf";

	f = filp_open(path, O_RDONLY, 0);
	if (IS_ERR(f)) {
		pr_err("--> Error opening %s: %ld\n", path, PTR_ERR(f));
		return -1;
	}

	ret = kernel_read(f, flash_matrix, sizeof(flash_matrix), &pos);
	filp_close(f, NULL);
	if (ret < 0) {
		pr_err("--> Error reading file: %zd\n", ret);
		return -1;
	}

	return 0;
}

/*****************************************************************************
//function :
//		set_flash_register
//description :
//		this function is used to set flash_matrix registers
//input :
//		register value and register offset
//output :
//		N/A
//
******************************************************************************/
void set_flash_register(uint reg, uint offset)
{
	flash_matrix[offset>>2] = reg;
}

/*****************************************************************************
//function :
//		set_flash_register_default
//description :
//		this function is used to set default value to flash_matrix
//input :
//		N/A
//output :
//		N/A
//
******************************************************************************/
void set_flash_register_default(void)
{
	int index = 0;
	for(index = 0; index < 40; index++)
			flash_matrix[index] = 0xffffffff;

}
/*****************************************************************************
//Function :
//		set_flash_register_BiasModCurrent
//Description :
//		This function is to set flash_matrix registers for current Ibias and Imod
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_BiasModCurrent(void)
{
	uint ibias = mt7570_information_output(Ibias);
	uint imod  = mt7570_information_output(Imod );
	set_flash_register(ibias, flash_Ibias_init);
	set_flash_register(imod , flash_Imod_init );
	printk("Ibias = 0x%x\n", ibias);
	printk("Imod = 0x%x\n" , imod );
}
/*****************************************************************************
//Function :
//		set_flash_register_P0P1
//Description :
//		This function is to set flash_matrix registers for current MPDL and MPDH
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_P0P1(void)
{
	uint p0 = mt7570_information_output(P0);
	uint p1 = mt7570_information_output(P1);
	set_flash_register(p0, flash_P0_target);
	set_flash_register(p1, flash_P1_target);
	printk("P0 = 0x%x\n", p0);
	printk("P1 = 0x%x\n", p1);
}
/*****************************************************************************
//Function :
//		set_flash_register_APD
//Description :
//		This function is to set flash_matrix registers for APD
//Input :
//		slope up, slope down, and APD voltage at 25C
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_APD(uint slope_up, uint slope_dn, uint VAPD)
{
	set_flash_register(slope_up, flash_APD_slope1);
	set_flash_register(slope_dn, flash_APD_slope2);
	set_flash_register(VAPD    , flash_APD_change_point);
	printk("slope up = 0x%x\n"  , slope_up);
	printk("slope down = 0x%x\n", slope_dn);
	printk("VAPD@25C = 0x%x\n"  , VAPD    );
}
/*****************************************************************************
//Function :
//		set_flash_register_LOS
//Description :
//		This function is to set flash_matrix registers for LOS
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_LOS(void)
{
	unchar ptr[4];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	ptr[3] = ptr[3] & 0x7f;	//Low threshold
	ptr[2] = ptr[2] & 0x7f;	//High threshold
	set_flash_register(ptr[3], flash_LOS_low_thld);
	set_flash_register(ptr[2], flash_LOS_high_thld);
	printk("LOS low = 0x%x\n", ptr[3]);
	printk("LOS high = 0x%x\n", ptr[2]);
}
/*****************************************************************************
//Function :
//		set_flash_register_DDMI_TxPower
//Description :
//		This function is to set flash_matrix registers for DDMI Tx Power
//Input :
//		Tx power (dBm) x100, FLASH address
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_DDMI_TxPower(uint TxPower, uint offset)
{
	uint input = 0;
	input = ( (TxPower<<16)&flash_Tx_power_mask ) | (mt7570_MPD_current()&flash_Tx_MPD_current_mask);		//by HC 20150331
	set_flash_register(input, offset);

	if( vEN7570==0 && offset==flash_Tx_power_K_point1 )
		set_flash_register(input&flash_Tx_MPD_current_mask, flash_MPD_point_up);

	printk("0x%08x\n", input);
}
/*****************************************************************************
//Function :
//		set_flash_register_DDMI_RxPower
//Description :
//		This function is to set flash_matrix registers for DDMI Tx Power
//Input :
//		Rx power (uW) x100, FLASH address
//Output :
//		N/A
//Date :
//		2015/03/09 by HC
******************************************************************************/
void set_flash_register_DDMI_RxPower(uint RxPower, uint offset)
{
	uint input = 0;
	input = ( (RxPower<<16)&flash_Rx_power_mask ) | (mt7570_RSSI_current()&flash_Rx_RSSI_current_mask);		//by HC 20150331
	set_flash_register(input, offset);
	printk("0x%08x\n", input);
	global_DDMI_rx_cal_flag = 0;		//biker_20151001

}
/*****************************************************************************
//Function :
//		set_flash_register_TIAGAIN
//Description :
//		This function is to set flash_matrix registers for TIA gain
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/20 by HC
******************************************************************************/
void set_flash_register_TIAGAIN(void)
{
	unchar ptr[1];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX+0x0001 , ptr, 1);
	ptr[0] = ptr[0] >> 6;
	set_flash_register(ptr[0], flash_TIAGAIN);
	printk("TIAGAIN = 0x%x\n", ptr[0]);
}


/*****************************************************************************
//Function :
//		set_flash_register_EnvTemp
//Description :
//		This function is to calibrate enviroment temperature
//Input :
//		enviroment temperature
//Output :
//		N/A
//Date :
//		2015/10/06 by HC
******************************************************************************/
void set_flash_register_EnvTemp(uint temperature_input)
{
	uint write_data = 0;
	uint Env_temperature_offset = 0;

	// Save enviroment temperature into FLASH
	set_flash_register(temperature_input, flash_reserved_for_ETC1);
	printk("Env_temperature = %d.%.1dC\n", temperature_input/10, temperature_input%10);

	// Calculate IC temperature first
	mt7570_temperature_get();

	// Offset = IC - enviroment ( x10 to HEX )
	Env_temperature_offset = (uint)(IC_temperature*10) - temperature_input;
	if( Env_temperature_offset < 0 )		// Because IC temperature is always higher than enviroment one
		printk("Not thermal balance\n");
	else
	{
		printk("Env_temperature_offset = %d.%.1dC\n", Env_temperature_offset/10, Env_temperature_offset%10);

		write_data = (Env_temperature_offset << 16) | (get_flash_register(flash_temerature_offset) & 0x0000ffff);
		set_flash_register(write_data, flash_temerature_offset);
	}
}


/*****************************************************************************
//Function :
//		set_flash_register_Temp_offset
//Description :
//		This function is to set flash_temp registers for temp offset
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/07/14 by Biker_20150714
******************************************************************************/
void set_flash_register_temperature_offset(uint temperature_input, uint slope)
{

	uint tmp;

	if ((temperature_input > 0x500) || (temperature_input < 0))
	{
		printk ("Please input temperature value in range : 0 ~ 0x500\n");
		return;
	}
	// in case user not input "slope"
	if (slope == 0)
		slope = 0xffff;
	else
		printk("Temperature slope = 0x%x\n",slope);

	// Clear offset as 0xffff and set slope as input
	tmp = (slope << 16) | 0xffff;
	set_flash_register(tmp, flash_temperature_K_slope_offset);

	mt7570_temperature_get();

	tmp = temperature_input - IC_temperature*10 + temperature_offset*10;
	printk("Temperature offset = 0x%x\n",tmp);

	tmp = (tmp & flash_temperature_offset_mask) | (slope << 16) ;
	set_flash_register(tmp, flash_temperature_K_slope_offset);

}


/*****************************************************************************
//Function :
//		set_flash_register_T0T1delay
//Description :
//		This function is to set flash_matrix registers for T0/T1 delay
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/10/15 by HC
******************************************************************************/
void set_flash_register_T0T1delay(void)
{
	unchar ptr[1];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 1);

	set_flash_register(ptr[0], flash_T0T1delay);
	printk("T0/T1 delay = 0x%x\n", ptr[0]);
}


/*****************************************************************************
//Function :
//		set_flash_register_T0CT1C
//Description :
//		This function is to set flash_matrix registers for T0C/T1C of TGEN
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2016/02/04 by HC
******************************************************************************/
void set_flash_register_T0CT1C(void)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
	ptr[1] = (ptr[1] & T1_timer_reset_value);	// T1C
	ptr[2] = (ptr[2] & T0_timer_reset_value);	// T0C

	set_flash_register( ( (ptr[1]<<16) | ptr[2] ), flash_T0CT1C);
	printk("T0C = 0x%x\nT1C = 0x%x\n", ptr[2], ptr[1]);
}


/*****************************************************************************
//Function :
//		sset_flash_register_Tx_data
//Description :
//		This function is to set flash_matrix registers for Tx data
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/10/15 by HC
******************************************************************************/
void set_flash_register_Tx_data(void)
{
	set_flash_register_BiasModCurrent();
	set_flash_register_P0P1();
	set_flash_register_TIAGAIN();
	set_flash_register_T0T1delay();
	set_flash_register_T0CT1C();			// by HC 20160204
}


/*****************************************************************************
//function :
//		save_flash_matrix
//description :
//		this function is used to save flash_matrix to 7570_bob.conf
//input :
//		N/A
//output :
//		0 : success ; -1 : fail
//
******************************************************************************/
int save_flash_matrix(void)
{
	struct file *f;
	loff_t pos = 0;
	ssize_t ret;
	const char *path = "/tmp/7570_bob.conf";

	f = filp_open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (IS_ERR(f)) {
		pr_err("--> Error opening %s for write: %ld\n", path, PTR_ERR(f));
		return -1;
	}

	ret = kernel_write(f, flash_matrix, sizeof(flash_matrix), &pos);

	filp_close(f, NULL);

	if (ret < 0) {
		pr_err("--> Error writing file: %zd\n", ret);
		return -1;
	}

	pr_info("--> write flash_matrix success, bytes: %zd\n", ret);
	return 0;
}

/*****************************************************************************
//Function :
//		flash_dump
//Description :
//		This function is to show FLASH content
//Input :
//		N/A
//Output :
//		FLASH content
//Date:
//		2015/01/28 by HC
//
******************************************************************************/
void flash_dump(void)
{
	int i = 0;
	int j = 0;
	for(i=0; i<40; i++)
	{
		j = 4*i;
		printk("FLASH address 0x%x", j);
		printk("\t");
		printk("0x%08x\n", get_flash_register(j));
	}
}
/*****************************************************************************
//Function :
//		DOL_flash_dump
//Description :
//		This function is to show FLASH content
//Input :
//		N/A
//Output :
//		FLASH content
//Date:
//		2015/01/28 by HC
//
******************************************************************************/
void DOL_flash_dump(void)
{
	int i = 0;
	int j = 0;

	printk("FLASH");
	printk("\t");
	printk("temperature");
	printk("\t");
	printk("Value\n");

	for(i=0; i<16; i++)
	{
		j = 0xa0+4*i;
		printk("0x%x", j);
		printk("\t");
		printk("%d", -40+(10*i));
		printk("\t");
		printk("\t");
		printk("0x%08x\n", get_flash_register(j));
	}
}

/*****************************************************************************
//Function :
//		GPON_Tx_calibration
//Description :
//		This function is to initialize GPON Tx calibration
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/02/04 by HC
//
******************************************************************************/
void GPON_Tx_calibration(void)
{
	//ERC filter
	mt7570_ERC_filter();

	//Load Initial I_bias & I_mod
	mt7570_load_init_current();

	// GPON mode by YW 20160422
	phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);

	//T_GEN
	mt7570_TGEN(PHY_GPON_CONFIG);

	//Calibration pattern
	mt7570_calibration_pattern(PHY_GPON_CONFIG);

	//Clear Tx_fault
	mt7570_safe_circuit_reset();

	printk("GPON_Tx_calibration done\n");
}
/*****************************************************************************
//Function :
//		EPON_Tx_calibration
//Description :
//		This function is to initialize EPON Tx calibration
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/02/26 by HC
//
******************************************************************************/
void EPON_Tx_calibration(void)
{
	//ERC filter
	mt7570_ERC_filter();

	//Load Initial I_bias & I_mod
	mt7570_load_init_current();

    //EPON mode by YW 20160422
	phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);

	//T_GEN
	mt7570_TGEN(PHY_EPON_CONFIG);

	//Calibration pattern
	mt7570_calibration_pattern(PHY_EPON_CONFIG);

	//Clear Tx_fault
	mt7570_safe_circuit_reset();

	printk("EPON_Tx_calibration done\n");
}
/*****************************************************************************
//Function :
//		GPON_BER
//Description :
//		This function is to check GPON BER
//Input :
//		PRBS pattern, 5 for PRBS7; 6 for PRBS23; 7 for PRBS31
//Output :
//		bfaf02c4
//Date:
//		2015/03/24 by HC
//
******************************************************************************/
int GPON_BER(int pattern)
{
	int read_data = 0;
	// Enable CDR
	mt7570_CDR(ENABLE);
	//Disable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_START);

	mdelay(100);
	//GPON mode
	phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);
	mdelay(100);

	//bfaf04a0 0						neccesary by CH 20150805
	IO_SPHYREG(REG_BASE_4+0x00a0, 0x0);

	switch(pattern)
	{
		case 5:
			//bfaf02a0 81fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0x81fd53);
			//bfaf02a4 ffffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0xffffff);
			//bfaf04a0 5
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x5);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		case 6:
			//bfaf02a0 71fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0x71fd53);
			//bfaf02a4 7fffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0x7fffff);
			//bfaf04a0 6
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x6);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		case 7:
			//bfaf02a0 ff1fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0xff1fd53);
			//bfaf02a4 7ffffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0x7ffffff);
			//bfaf04a0 7
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x7);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		default:
			break;
	}
	mdelay(250);
	//Read bfaf02c4
	read_data = IO_GPHYREG(REG_BASE_2+0x00c4);

	if( read_data == 5 )
		printk("Pattern Aligned\n");
	else
		printk("Aligned Failed\n");

	return read_data;
}
/*****************************************************************************
//Function :
//		EPON_BER
//Description :
//		This function is to check EPON BER
//Input :
//		PRBS pattern, 5 for PRBS7; 6 for PRBS23; 7 for PRBS31
//Output :
//		bfaf02c4
//Date:
//		2015/03/24 by HC
//
******************************************************************************/
int EPON_BER(int pattern)
{
	int read_data = 0;
	// Enable CDR
	mt7570_CDR(ENABLE);
	//Disable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_START);

	mdelay(100);
	//EPON mode
	phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);
	mdelay(100);

	//bfaf04a0 0						neccesary by CH 20150805
	IO_SPHYREG(REG_BASE_4+0x00a0, 0x0);

	switch(pattern)
	{
		case 5:
			//bfaf02a0 81fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0x81fd53);
			//bfaf02a4 ffffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0xffffff);
			//bfaf04a0 5
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x5);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		case 6:
			//bfaf02a0 71fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0x71fd53);
			//bfaf02a4 7fffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0x7fffff);
			//bfaf04a0 6
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x6);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		case 7:
			//bfaf02a0 ff1fd53
			IO_SPHYREG(REG_BASE_2+0x00a0, 0xff1fd53);
			//bfaf02a4 7ffffff
			IO_SPHYREG(REG_BASE_2+0x00a4, 0x7ffffff);
			//bfaf04a0 7
			IO_SPHYREG(REG_BASE_4+0x00a0, 0x7);
			//bfaf04a4
			IO_SPHYREG(REG_BASE_4+0x00a4, 0x1);
			break;
		default:
			break;
	}
	mdelay(250);
	//Read bfaf02c4
	read_data = IO_GPHYREG(REG_BASE_2+0x00c4);

	if( read_data == 5 )
		printk("Pattern Aligned\n");
	else
		printk("Aligned Failed\n");

	return read_data;
}
/*****************************************************************************
//Function :
//		mt7570_restart_ERC
//Description :
//		This function is to restart ERC
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/02/10 by HC
//
******************************************************************************/
void mt7570_restart_ERC(void)
{
		unchar ptr[4];

		//Close P0 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = ptr[0] & ERC_start_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4 );
		//Close P1 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = ptr[0] & ERC_start_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4 );
		//Start P0 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = (ptr[0] & ERC_start_mask) | (ERC_start);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4 );
		//Start P1 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = (ptr[0] & ERC_start_mask) | (ERC_start);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4 );

		PON_PHY_MSG(PHY_MSG_DBG, "ERC restarted\n");
}


/*****************************************************************************
//function :
//		mt7570_calibration_pattern
//description :
//		This function is to send xPON calibration pattern
//input :
//		1 : GPON PRBS23
//		0 : EPON 8B/10B pattern
//output :
//		N/A
//Date :
//		2015/05/18 by HC
******************************************************************************/
void mt7570_calibration_pattern(int input)
{
	// Disable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_START);

	// Disable CDR
	mt7570_CDR(DISABLE);

	if( input == PHY_GPON_CONFIG )
	{
		phy_tx_test_pattern(PHY_BIST_PRBS23);
		phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);
		printk("PRBS23 enabled\n");
	}
	else if( input == PHY_EPON_CONFIG )
	{
		phy_tx_test_pattern(PHY_EPON_TEST_PAT);
		phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);
		printk("8B/10B pattern enabled\n");
	}


	PatternEnabled = 1;
}


/*****************************************************************************
//function :
//		mt7570_disable_pattern
//description :
//		This function is to disable xPON calibration pattern
//input :
//		N/A
//output :
//		N/A
//Date :
//		2015/05/18 by HC
******************************************************************************/
void mt7570_disable_pattern(void)
{
	phy_tx_test_pattern(PHY_BIST_IDLE);
	phy_tx_test_pattern(PHY_EPON_TEST_PAT_0);

	// Enable CDR
	mt7570_CDR(ENABLE);

	// Enable driver
    REPORT_EVENT_TO_MAC(PHY_EVENT_CALIBRATION_STOP);


	printk("Pattern disabled\n");
	PatternEnabled = 0;
}

/*****************************************************************************
//Function :
//		mt7570_CDR
//Description :
//		This function is to enable/disable CDR in 752X
//Input :
//		1: Enable
//		0: Disable
//Output :
//		N/A
//Date:
//		2015/05/22 by HC
******************************************************************************/
void mt7570_CDR(int CDR_switch)
{
	uint read_data  = 0;

	if( CDR_switch == DISABLE )
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET1);
		IO_SPHYREG(PHY_CSR_PHYSET1, (read_data | PHY_TX_LOCK_2_REF));
		printk("CDR disabled\n");
	}
	else if( CDR_switch == ENABLE )
	{
		read_data = IO_GPHYREG(PHY_CSR_PHYSET1);
		IO_SPHYREG(PHY_CSR_PHYSET1, (read_data & PHY_TX_LOCK_2_DATA));
		printk("CDR enabled\n");
	}
}

/*****************************************************************************
//function :
//		mt7570_init
//description :
//		this function is used to initialize Tx and Rx of EN7570
//input :
//		N/A
//output :
//		0 : the system is ready
//
******************************************************************************/
int mt7570_init(void)
{
	uint read_data  = 0;

    i2c_access_queue = create_workqueue("i2c_access_queue");
    INIT_WORK(&gpPhyPriv->EN7570_reset, mt7570_safe_circuit_reset_for_work_queue);

	// Get FLASH
	if(get_flash_matrix() == 0)			//Get FLASH successfuly
		printk("FLASH matrix got\n");
	else											//Get FLASH fail
		set_flash_register_default();	//Reset FLASH to default


	//SW reset
	mt7570_sw_reset();
#if 1												// Moved here for "mt7570_MPD_current_calibration" by YW 20151223
	//Set TIAGAIN
	mt7570_TIAGAIN_set();
#endif
	//ERC filter
	mt7570_ERC_filter();

	// Get MPD current offset													// Must placed after "mt7570_TIAGAIN_set"
	mt7570_MPD_current_calibration();									// Moved by HC 20150413

	// ADC calibration															// This shouldn't be placed too close after "mt7570_sw_reset"
	mt7570_ADC_calibration();												// by HC 20150603

	// RSSI calibration
	mt7570_RSSI_calibration();											// by HC 20150609

	// Get magic number
	read_data = get_flash_register(flash_magic_number);

	//If magic number is 0x07050700, run GPON initialization
	if(read_data == 0x07050700)
	{
		printk("Start GPON Tx Calibration\n");
		phy_mode_config(PHY_GPON_CONFIG, PHY_ENABLE);				//GPON mode by HC 20150513
		xPON_mode = PHY_GPON_CONFIG; 									// xPON mode by HC 20150520

		//RSSI gain initialization
		mt7570_RSSI_gain_init();

		//Rx LOS
		mt7570_LOS_level_set();

		// Polarity setting for EN7570
		IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x10f);

		// TGEN
		mt7570_TGEN(PHY_GPON_CONFIG);

		//Load Initial I_bias & I_mod
		mt7570_load_init_current();

		//Load MPDL/MPDH from FLASH
		mt7570_load_MPDL_MPDH();

		//Set Tx_SD
		mt7570_TxSD_level_set();


		//APD initialization
		mt7570_APD_initialization();									// Moved by HC 20150514
		if( get_flash_register(flash_T_APD) != 0xffffffff )		//by HC
		{
			T_APD = get_flash_register(flash_T_APD);
			if( T_APD < 10 )													//Check T_APD >= 10 by YMC 20150125
			{
				printk("APD update period is too short, reset to 10s\n");
				T_APD=10;
			}
		}
		mt7570_APD_control();
	}

	//If magic number is 0xe7050700, run EPON initialization
	else if(read_data == 0xe7050700)									//EPON by HC
	{
		printk("Start EPON Tx Calibration\n");
		phy_mode_config(PHY_EPON_CONFIG, PHY_ENABLE);					//EPON mode by HC
		xPON_mode = PHY_EPON_CONFIG; 										// xPON mode by HC 20150520

		//RSSI gain initialization
		mt7570_RSSI_gain_init();

		//Rx LOS
		mt7570_LOS_level_set();

		// Polarity setting for EN7570
		IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x10f);

		// TGEN
		mt7570_TGEN(PHY_EPON_CONFIG);

		//Load Initial I_bias & I_mod
		mt7570_load_init_current();

		//Load MPDL/MPDH from FLASH
		mt7570_load_MPDL_MPDH();

		//set Tx SD
		mt7570_TxSD_level_set();
	}
	//If magic number is wrong, return.
	else
	{
		internal_DDMI = 0;												// by HC 20150218
		printk("Internal DDMI Disabled\n");
		return 0;
	}

	// Check single-closed and open loop modes
	if( get_flash_register(flash_reserved1) == 0x80000000)
	{
		SCL = 1;
		mt7570_SingleClosedLoopMode();
	}
	else if( get_flash_register(flash_reserved1) == 0xc0000000)
	{
		DOL = 1;
		mt7570_LUT_recover();
		mt7570_OpenLoopMode();
	}



	//Clear Rogue ONU															// by YMC 20150125
	mt7570_RougeONU_clear();

	//Reset safe_circuit 														// by YMC 20150125
	mt7570_safe_circuit_reset();


	// Check internal DDMI						// Moved by HC 20150410
	if( get_flash_register(flash_internal_DDMI) == 0 )
	{
		internal_DDMI = 0;
		printk("Internal DDMI Disabled\n");
	}
	else
	{
		printk("Internal DDMI Enabled\n");
		if( get_flash_register(flash_internal_DDMI) == 2 )		// by HC 20160517
			fast_DDMI = 1;
	}

	// Check TEC															// by HC 20150513
	if( get_flash_register(flash_TEC) == 0 )
	{
		TEC = 0;
		printk("TEC Disabled\n");
	}
	else if( get_flash_register(flash_TEC) != 0xffffffff )
	{
		BOSA_Ith = get_flash_register(flash_TEC);			// Ith of BOSA, uA
		printk("TEC Enabled, BOSA's Ith = %duA\n", BOSA_Ith);
	}
	else
	{
		printk("TEC Enabled\n");
	}


	// Check ETC by YM 20150731
	if( get_flash_register(flash_ETC) == 1 )					// by YM
	{
		ETC = 1;
		printk("ETC = 1\n");
	}
	else if ( get_flash_register(flash_ETC) == 2 )			// by Jay
	{
		ETC = 2;
		printk("ETC = 2\n");
	}
	else if ( get_flash_register(flash_ETC) == 3 )			// by HC
	{
		ETC = 3;
		mt7570_LUT_recover();
		mt7570_temperature_get();
		mt7570_BiasTracking();
		printk("ETC = 3\n");
		printk("BiasTracking Enabled\n");
	}
	else
	{
		printk("ETC Disabled\n");
	}



	printk("EN7570 Initialization Done!\n");
	return 0 ;
}




/*****************************************************************************
//Function :
//		mt7570_sw_reset
//Description :
//		This function is reset all registers of 7570
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/10/14 by HC
******************************************************************************/
void mt7570_sw_reset(void)
{
	unchar ptr[4];

	//SW reset
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SW_RESET, ptr, 4);
	ptr[0] = (ptr[0] & 0xf8) | (0x01);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SW_RESET, ptr, 4);
}


/*****************************************************************************
//Function :
//		mt7570_RSSI_gain_init
//Description :
//		This function is to initialize RSSI gain
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/10/14 by HC
******************************************************************************/
void mt7570_RSSI_gain_init(void)
{
	unchar ptr[4];

	//RSSI gain initialization
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
	ptr[2] = (ptr[2] & RSSI_gain_mask) | (0x05);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
}



/*****************************************************************************
//Function :
//		mt7570_TGEN
//Description :
//		This function is to do EN7570 TGEN
//Input :
//		1: GPON
//		0: EPON
//Output :
//		N/A
//Date:
//		2015/05/12 by HC
******************************************************************************/
void mt7570_TGEN(int xPON)
{
	unchar ptr[4];
	uint RGS_T0C = 0;
	uint RGS_T1C = 0;
	int i = 0;

	//Disable CDR
	mt7570_CDR(DISABLE);

	//752X sends PRBS23
	phy_tx_test_pattern(PHY_BIST_PRBS23);


	mdelay(10);

	for( i=0; i<32; i++ )		// Get MAX T0C/T1C
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & ERC_enable_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[0] = (ptr[0] & T1_T0_delay_mask) | (T1_T0_delay_setting1);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[1] = (ptr[1] & T1_timer_reset_mask) | (T1_timer_reset_value);
		ptr[2] = (ptr[2] & T0_timer_reset_mask) | (T0_timer_reset_value);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_reset_value_mask) | (TGEN_reset_value_T1T0_timer);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_reset_value_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_method2_mask) | (TGEN_method2_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		udelay(2);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
		ptr[3] = (ptr[3] & TGEN_method2_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T0C, ptr, 4);
		if( ptr[0] > RGS_T0C )
			RGS_T0C = ptr[0];
		if( ptr[1] > RGS_T1C )
			RGS_T1C = ptr[1];

	}

	mdelay(10);

	// Set T0T1delay
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_T1DELAY, ptr, 4);	// by HC 20151015
	if( get_flash_register(flash_T0T1delay) != 0xffffffff )
	{
		ptr[0] = (ptr[0] & T1_T0_delay_mask) | (get_flash_register(flash_T0T1delay) & 0xff);
		printk("T0/T1 delay = 0x%02x\n", get_flash_register(flash_T0T1delay) & 0xff);
	}
	else if( xPON == PHY_GPON_CONFIG )
	{
		ptr[0] = (ptr[0] & T1_T0_delay_mask) | 0x9a;		// 0x9a by HC 20150814
		printk("T0/T1 delay = 0x9a\n");
	}
	else if( xPON == PHY_EPON_CONFIG )
	{
			ptr[0] = (ptr[0] & T1_T0_delay_mask) | 0x47;	// 0x47 by CH 20150623
			printk("T0/T1 delay = 0x47\n");
	}
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_T1DELAY, ptr, 4 );

	mdelay(10);

	#if 1													// Set RGS_T0C/T1C by HC 20150623
	if( get_flash_register(flash_T0CT1C) != 0xffffffff )	// Get fixed T0C/T1C
	{
		RGS_T0C = get_flash_register(flash_T0CT1C) & 0x0000ffff;
		RGS_T1C = (get_flash_register(flash_T0CT1C) & 0xffff0000)>>16;
	}
	printk("RGS_T0C = 0x%x\nRGS_T1C = 0x%x\n", RGS_T0C, RGS_T1C);
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
	ptr[1] = (ptr[1] & T1_timer_reset_mask) | RGS_T1C;
	ptr[2] = (ptr[2] & T0_timer_reset_mask) | RGS_T0C;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & TGEN_reset_value_mask) | (TGEN_reset_value_T1T0_timer);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & TGEN_reset_value_mask);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );
	#endif
	mdelay(10);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4);
	ptr[3] = (ptr[3] & ERC_enable_mask) | (ERC_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_T1DELAY, ptr, 4 );

	//Enable CDR
	mt7570_CDR(ENABLE);

	//752X back to normal mode
	phy_tx_test_pattern(PHY_BIST_IDLE);

	printk("TGEN done\n");
}

/*****************************************************************************
//function :
//		mt7570_LOS_level_set
//description :
//		this function is used to set LOS level
//input :
//		N/A
//output :
//		N/A
//
******************************************************************************/
void mt7570_LOS_level_set(void)
{
	unchar ptr[4];
	#if 1							// Replaced by HC 20160427
	mt7570_LOS_init();
	#else
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	ptr[0] = (ptr[0] & (LOS_calibration_mask)) | (LOS_calibration_trig);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_ADCREV2_mask)) | (LOS_ADCREV2_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[2] = (ptr[2] & (LOS_ADCREV1_mask)) | (LOS_ADCREV1_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
	ptr[1] = (ptr[1] & (LOS_confidence_mask)) | (LOS_confidence_setting);
	ptr[0] = (ptr[0] & (LOS_cnt_mask)) | (LOS_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
	#endif
	if((get_flash_register(flash_LOS_high_thld) != 0xffffffff) && (get_flash_register(flash_LOS_low_thld) != 0xffffffff))
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
		ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | (get_flash_register(flash_LOS_low_thld));	//Fix coding mistake YMC 20150128
		ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | (get_flash_register(flash_LOS_high_thld));	//Fix coding mistake YMC 20150128
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	}
	else
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
		ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | (LOS_comp_thld_L_setting);
		ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | (LOS_comp_thld_H_setting);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	}
	printk("Rx LOS is set\n");
}


/*****************************************************************************
//function :
//		mt7570_TxSD_level_set
//description :
//		this function is used to set TX SD level
//input :
//		N/A
//output :
//		N/A
******************************************************************************/
void mt7570_TxSD_level_set(void)
{
	int delta = 0;
	uint tia_sd = 0;
	uint MPDH = 0;
	uint MPDL = 0;
	unchar ptr[4];

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH, ptr, 4);			// Get MPDH and MPDL
		MPDH = ((ptr[1]<<8) | ptr[0]) & (0x3ff);
		MPDL = ((ptr[3]<<8) | ptr[2]) & (0x3ff);

		delta = mt7570_TxSD_level_calibration();					// tiaflt - TIASD_0
		if ( delta >= 0 )	// tiaflt - TIASD_0 >= 0
		{
			tia_sd = (0.1/2)*(MPDH + (MPDL>>2)) + (1.4/0.6)*delta;	// Set TIASD = 0.1*(MPDH+MPDL/4)/2 + (1.4/0.6)*(tiaflt-TIASD_0)
			ptr[0] = tia_sd & (0xff);
			ptr[1] = (tia_sd >> 8) & (0x01);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_TIASD, ptr, 2 );
			printk("Tx SD set\n");
		}
		else				// tiaflt - TIASD_0 < 0
		{
			printk("Tx SD error\n");
		}
}


/*****************************************************************************
//Function :
//		mt7570_TxSD_level_calibration
//Description :
//		This function is to calibrate Tx SD level
//		It's not neccesary to add I2C protection because of mt7570_TxSD_level_set
//Input :
//		N/A
//Output :
//		tiaflt - TIASD_0
//
******************************************************************************/
int mt7570_TxSD_level_calibration(void)
{
	unchar ptr[2];
	int tiaflt  = 0;
	int TIASD_0 = 0;

	// Set RG_TIAMUX to b'100													//by HC 20150413
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
	ptr[0] = (ptr[0] & tia_mux_mask) | default_tia_mux_select;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

	//ADC select, set RG_SVADC_REV1[3] = 1 ( 0x24[20] = 1 )
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD+0x02, ptr, 1);				//Read RG_SVADC_REV1
	ptr[0] = (ptr[0] & ADC_select_SD_mask) | (ADC_select_SD_enable);		//Set 0x24[20] = 1, prt[0] and 11101111 or 10000
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD+0x02, ptr, 1);
	mdelay(1);

	//ADC select, set RG_SVADC_INMUX = b'0010 ( 0x24[2] = 1 )						//by HC 20150108
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);					//Read RG_SVADC_INMUX
	ptr[0] = (ptr[0] & ADC_select_inmux_mask) | (ADC_TxPW_enable);			//Set 0x24[2] = 1, prt[0] and 11100001 or 100
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
	mdelay(1);

	//Latch ADC value, set reg_w_adlch = 1 ( 0x158[12] = 1 )
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);	//Read reg_w_adlch
	ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);						//Set 0x158[12] = 1, prt[0] and 11101111 or 10000
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
	mdelay(1);

	//Read tiaflt, ad_svadc_dout_lch_to_i2c ( 0x154[09:00] )
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);			//Read 0x154[15:00]
	tiaflt = (ptr[0] | (ptr[1]<<8));

	mdelay(10);

	//TIA mux select, set RG_TIAMUX = b'001 ( 0x000[1] = 1 )
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);					//Read RG_TIAMUX
	ptr[0] = (ptr[0] & tia_mux_mask) | (tia_mux_select_SD);					//Set 0x000[1] = 1, prt[0] and 11110001 or 10
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
	mdelay(1);


	//Set RG_TIASD = 0 ( 0x00C[08:00] = 0 ) 									//by HC 20150109
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIASD, ptr, 2);						//Read RG_TIASD
	ptr[0] = (ptr[0] & 0x00);									 			//Set 0x00C[07:00] = 0, prt[0] and 00000000
	ptr[1] = (ptr[1] & tia_sd_upper_mask); 									//Set 0x00C[8] = 0, prt[1] and 1111110
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIASD, ptr, 2);
	mdelay(1);

	//Latch ADC value, set reg_w_adlch = 1 ( 0x158[12] = 1 )
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);	//Read reg_w_adlch
	ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);						//Set 0x158[12] = 1, prt[0] and 11101111 or 10000
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
	mdelay(1);

	//Read tiasd_0 value
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);			//Read 0x154[15:00]
	TIASD_0 = (ptr[0] | (ptr[1]<<8));

	//Set RG_SVADC_REV1[3] to default 0
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD+0x02, ptr, 1);
	ptr[0] = (ptr[0] & ADC_select_SD_mask);									//Set 0x24[20] = 0, prt[0] and 11101111
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD+0x02, ptr, 1);


	//Set RG_SVADC_INMUX to default 0											//by HC 20150118
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);					//Read RG_SVADC_INMUX
	ptr[0] = (ptr[0] & ADC_select_inmux_mask);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

	// Set RG_TIAMUX to default b'100											//by HC 20150413
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
	ptr[0] = (ptr[0] & tia_mux_mask) | default_tia_mux_select;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

	return (tiaflt - TIASD_0);
}
/*****************************************************************************
//Function :
//		mt7570_ADC_calibration
//Description :
//		This function is to calibrate ADC
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/06/08 by HC
******************************************************************************/
void mt7570_ADC_calibration(void)
{
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
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD , ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_select_BG_1V76;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
	    for(i=0;i<cnt;i++)
		{
			// Latch ADC
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
			ptr[0] = (ptr[0]&ADC_latch_mask) | ADC_latch;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
			// Read 1.76V value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
			temp += (ptr[0] | (ptr[1]<<8));
		}
		// Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		BG1V76 = ((temp>>2)+1)>>1;
		//Set temp to 0
		temp = 0;

		// Set ADC to BG 0.875V
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_SVADC_PD + 0x01) , ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & (ADC_BG_0V875_mask))|(ADC_select_BG_0V875);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_SVADC_PD + 0x01), ptr, 1);
		for(i=0;i<cnt;i++)
		{
			// Latch ADC
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
			ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
			// Read 0.875V value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
			temp += (ptr[0] | (ptr[1]<<8));
		}
		//Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_SVADC_PD + 0x01), ptr, 1);
		atomic_set(&i2c_protect, 0);
		BG0V875 = ((temp>>2)+1)>>1;
		//Set temp to 0
		temp = 0;
		//printk("BG 1V76 = 0x%x\nBG 0V875 =0x%x\n", BG1V76, BG0V875);
		PON_PHY_MSG(PHY_MSG_DBG, "BG 1V76 = 0x%x\nBG 0V875 =0x%x\n", BG1V76, BG0V875);

		if( (BG1V76-BG0V875) <= 0 )		// Error
		{
			printk("ADC calibration fail\n");
			return;
		}

		ADC_slope  = (1.76-0.875)/(float)(BG1V76-BG0V875);
		ADC_offset = 1.76 - ADC_slope*(float)BG1V76;
	}
}
/*****************************************************************************
//Function :
//		mt7570_APD_initialization
//Description :
//		This function is to initialize APD voltage
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		20150127 by HC
//
******************************************************************************/
void mt7570_APD_initialization(void)
{
	unchar ptr[4];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 4);
	ptr[2] = ptr[2]|(APD_softstart_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 4);
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 4);
	ptr[1] = ptr[1]|(APD_control_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 4);
	printk("APD initialization done\n");
}
/*****************************************************************************
//Function :
//		mt7570_APD_control
//Description :
//		This function is used to set APD voltage
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		20150113 by HC
//
******************************************************************************/
void mt7570_APD_control(void)
{
	if( get_flash_register(0x030)>0x0000ffff && get_flash_register(0x034)>0x0000ffff )
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
		if(get_flash_register(flash_APD_slope1)!=0xffffffff)
			APD_slope_up = ( (float) get_flash_register(flash_APD_slope1) )/100;

		if(get_flash_register(flash_APD_slope2)!=0xffffffff)
			APD_slope_dn = ( (float) get_flash_register(flash_APD_slope2) )/100;

		if(get_flash_register(flash_APD_change_point)!=0xffffffff)
			APD_voltage_NT = ( (float) get_flash_register(flash_APD_change_point) )/100;

		if( ( get_flash_register(flash_APD_voltage_1)|get_flash_register(flash_APD_voltage_2) ) != 0xffffffff)		//by HC 20150303
		{
			APD_voltage_0x00 = ( (float)( (get_flash_register(flash_APD_voltage_1)&0xffff0000)>>16) )/10;
			APD_voltage_0x40 = ( (float)( get_flash_register(flash_APD_voltage_1)&0x0000ffff)         )/10;
			APD_voltage_0x80 = ( (float)( (get_flash_register(flash_APD_voltage_2)&0xffff0000)>>16) )/10;
			APD_voltage_0xC0 = ( (float)( get_flash_register(flash_APD_voltage_2)&0x0000ffff)         )/10;

			// If not ( V40>V00 and V80>V40 and VC0>V80 ), return
			// Logic : (xyz)' = x' + y' + z'
			if( (APD_voltage_0x40 <= APD_voltage_0x00) || (APD_voltage_0x80 <= APD_voltage_0x40) || (APD_voltage_0xC0 <= APD_voltage_0x80) )
				{
					printk("APD slope error\n");
					return;
				}
		}

		//Equation for Vapd calculation
		if( BOSA_temperature > 25 )
			APD_voltage = APD_voltage_NT + APD_slope_up*( BOSA_temperature - 25 );
		else
			APD_voltage = APD_voltage_NT - APD_slope_dn*( 25 - BOSA_temperature );

		//Print BOSA_temperature
		a = (int)BOSA_temperature;
		b = abs( (int)(BOSA_temperature*10)%10 );
		PON_PHY_MSG(PHY_MSG_DBG, "BOSA temperature = %d.%dC\n", a, b);
		//Print APD_voltage
		a = (int)APD_voltage;
		b = (int)(APD_voltage*10)%10;
		PON_PHY_MSG(PHY_MSG_DBG, "APD Voltage = %d.%dV\n", a, b);

		//Equation for RG_APD_DAC_CODE calculation, the default_APD_zero_code_voltage and default_APD_voltage_step depend on circuitry.
		if( APD_voltage < APD_voltage_0x80 )
		{
			if( APD_voltage < APD_voltage_0x40 )	// V00 <= VAPD < V40
			{
				APD_voltage_step = (APD_voltage_0x40 - APD_voltage_0x00)/64;
				ptr[0] = 0x00 + (int)( (APD_voltage - APD_voltage_0x00)/APD_voltage_step );
				PON_PHY_MSG(PHY_MSG_DBG, "APD slope 1\n");
			}
			else												// V40 <= VAPD < V80
			{
				APD_voltage_step = (APD_voltage_0x80 - APD_voltage_0x40)/64;
				ptr[0] = 0x40 + (int)( (APD_voltage - APD_voltage_0x40)/APD_voltage_step );
				PON_PHY_MSG(PHY_MSG_DBG, "APD slope 2\n");
			}

		}
		else
		{
			APD_voltage_step = (APD_voltage_0xC0 - APD_voltage_0x80)/64;
			PON_PHY_MSG(PHY_MSG_DBG, "APD slope 3\n");

			if( APD_voltage < APD_voltage_0xC0 )	// V80 <= VAPD < VC0
				ptr[0] = 0x80 + (int)( (APD_voltage - APD_voltage_0x80)/APD_voltage_step );
			else												// VC0 <= VAPD <= VFF
			{
				ptr[0] = 0xC0 + (int)( (APD_voltage - APD_voltage_0xC0)/APD_voltage_step );
				if( ptr[0] > 0xff )
					ptr[0] = 0xff;
			}
		}

		PON_PHY_MSG(PHY_MSG_DBG, "APD = 0x%x\n", ptr[0]);
		mt7570_APD_I2C_write( ptr );
	}
	else
	{
		unchar ptr[1];
		int a = 0;
		int b = 0;
		float APD_slope_up   = 0.10;		//Default by HC
		float APD_slope_dn   = 0.07;		//Default by HC
		float APD_voltage_NT = 35.0;		//Default by HC
		float APD_zero_code_voltage = default_APD_zero_code_voltage;	//Default by HC 20150303
		float APD_voltage_step      = default_APD_voltage_step;			//Default by HC 20150303

		//Load slope and voltage from FLASH
		if(get_flash_register(flash_APD_slope1)!=0xffffffff)
			APD_slope_up = ( (float) get_flash_register(flash_APD_slope1) )/100;

		if(get_flash_register(flash_APD_slope2)!=0xffffffff)
			APD_slope_dn = ( (float) get_flash_register(flash_APD_slope2) )/100;

		if(get_flash_register(flash_APD_change_point)!=0xffffffff)
			APD_voltage_NT = ( (float) get_flash_register(flash_APD_change_point) )/100;

		if(get_flash_register(flash_APD_zero_code_voltage)!=0xffffffff)	//by HC 20150303
			APD_zero_code_voltage = ( (float) get_flash_register(flash_APD_zero_code_voltage) )/100;

		if(get_flash_register(flash_APD_voltage_step)!=0xffffffff)		//by HC 20150303
			APD_voltage_step = ( (float) get_flash_register(flash_APD_voltage_step) )/1000;

		//Equation for Vapd calculation
		if( BOSA_temperature > 25 )
			APD_voltage = APD_voltage_NT + APD_slope_up*( BOSA_temperature - 25 );
		else
			APD_voltage = APD_voltage_NT - APD_slope_dn*( 25 - BOSA_temperature );

		//Print BOSA_temperature
		a = (int)BOSA_temperature;
		b = abs( (int)(BOSA_temperature*10)%10 );
		PON_PHY_MSG(PHY_MSG_DBG, "BOSA temperature = %d.%dC\n", a, b);
		//Print APD_voltage
		a = (int)APD_voltage;
		b = (int)(APD_voltage*10)%10;
		PON_PHY_MSG(PHY_MSG_DBG, "APD Voltage = %d.%dV\n", a, b);

		//Equation for RG_APD_DAC_CODE calculation, the default_APD_zero_code_voltage and default_APD_voltage_step depend on circuitry.
		ptr[0] = (int)( (APD_voltage - APD_zero_code_voltage)/APD_voltage_step );
		PON_PHY_MSG(PHY_MSG_DBG, "APD = 0x%x\n", ptr[0]);
		#if 1	//New
		mt7570_APD_I2C_write( ptr );
		#else	//Before
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 1);
		#endif
	}
}
/*****************************************************************************
//Function :
//		mt7570_APD_I2C_write
//Description :
//		This function is to write APD register
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		20150310 by HC
//
******************************************************************************/
void mt7570_APD_I2C_write(unchar ptr[1])
{
	PON_PHY_MSG(PHY_MSG_DBG, "APD in mt7570_APD_I2C_write = 0x%x\n", ptr[0]);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_APD_DAC_CODE, ptr, 1);
}
/*****************************************************************************
//function :
//		mt7570_information_output
//description :
//		this function is used to select 7570 information output
//input :
//		select : 1 for temperature ; 2 for supply voltage ; 3 for bias current ; 4 for modulation current ;
//		5 for P0 ; 6 for P1
//output :
//		information output
//
******************************************************************************/
int mt7570_information_output(uint select)
{
	uint output = 0;
	unchar ptr[4];

		if(select == Ibias)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 4);
			output = (ptr[2] | ((ptr[3] & 0x0f) << 8 ));
			global_bias_current = output;
		}
		else if(select == Imod)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P1_PWR_CTRL_CS3, ptr, 4);
			output = (ptr[2] | ((ptr[3] & 0x0f) << 8 ));
			global_modulation_current = output;
		}
		else if(select == P0)							//by HC 20150309
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_MPDH, ptr, 4);
			output = (ptr[2] | ((ptr[3] & 0x03) << 8 ));
			global_MPDL = output;
		}
		else if(select == P1)							//by HC 20150309
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_MPDH, ptr, 4);
			output = (ptr[0] | ((ptr[1] & 0x03) << 8 ));
			global_MPDH = output;
		}

	return output;
}
/*****************************************************************************
//function :
//		mt7570_ADC_temperature_get
//description :
//		this function is to get the ADC code of temperature
//input :
//		N/A
//output :
//		ADC code
//
******************************************************************************/
int mt7570_ADC_temperature_get(void)
{
	unchar ptr[4];
	uint cnt = 8;
	uint i = 0;
	uint temp = 0;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
		return global_temperature_code;
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD , ptr, 1);
		ptr[0] = (ptr[0] & (ADC_select_mask))|(ADC_select_temperature);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		for(i=0;i<cnt;i++)
		{
			//latch ADC value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
			ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

			// Get temperature
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
			temp += (ptr[0] | (ptr[1]<<8));
		}
		//set to default ADCAUX
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD , ptr, 1);
		ptr[0] = ptr[0] & (ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
        atomic_set(&i2c_protect, 0);
		global_temperature_code = ((temp>>2)+1)>>1;
		//Set temp to 0
		temp = 0;

		return global_temperature_code;
	}
}


/*****************************************************************************
//function :
//		mt7570_ADC_voltage_get
//description :
//		this function is to get the ADC code of voltage
//input :
//		N/A
//output :
//		ADC code
//
******************************************************************************/
int mt7570_ADC_voltage_get(void)
{
	unchar ptr[4];
	uint read_data = 0;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
		return global_vcc_code;
	}
	else
	{
		atomic_set(&i2c_protect, 1);
		#if 1
		//internal termination resistor is off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask))|(LA_Rx_HighZ_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 1);

		// Set ADC to VCC output, RG_SVADC_INMUX = b'0000 ( 0x24[4:1] = 0 )
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD , ptr, 1);
		read_data = ptr[0];
		ptr[0] = ptr[0] & (ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		// Latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

		// Get voltage
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		global_vcc_code = (ptr[0] | (ptr[1]<<8));

		//internal termination resistor is on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask));
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 1);

		// Set ADCAUX to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		#else
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask))|(LA_Rx_HighZ_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 1);

		//latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

		// Get voltage
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		global_vcc_code = (ptr[0] | (ptr[1]<<8));

		//set to default ADCAUX
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD , ptr, 1);
		ptr[0] = ptr[0] & (ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		#endif
		atomic_set(&i2c_protect, 0);
		return global_vcc_code;
	}
}


/*****************************************************************************
//Function :
//		mt7570_load_init_current
//Description :
//		This function is to load initial bias and modulation current
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/01/29 by HC
//
******************************************************************************/
void mt7570_load_init_current(void)
{
	unchar ptr[4];
	uint read_data = 0;
	int temperature_index = 0;

	//Load I_bias & I_mod from flash
	if( (get_flash_register(flash_Ibias_init)!=0xffffffff) && (get_flash_register(flash_Imod_init)!=0xffffffff) )
	{
		//Read Ibias initial value
		read_data = get_flash_register(flash_Ibias_init); // & (flash_Ibias_mask);
		read_data = read_data & (flash_Ibias_mask);
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS2, ptr, 2 );

		//Read Imod initial value
		read_data = get_flash_register(flash_Imod_init); // & (flash_Imod_mask);
		read_data = read_data & (flash_Imod_mask);
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS2, ptr, 2 );
		printk("Initial bias/mod current loaded from FLASH\n");
	}
	//Load I_bias & I_mod from LUT
	else
	{
		//Calculate temperature_index
		mt7570_temperature_get();
		temperature_index = (int)( (BOSA_temperature+40)/2.5 );															//In order to find the relative I_bias and I_mod in LUT_Ibias_Imod
		if( temperature_index < 0 )																						//temperature_index should be >= 0
			temperature_index = 0;

		//Read Ibias initial value
		read_data = LUT_Ibias_Imod[temperature_index][0];
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS2, ptr, 2 );

		//Read Imod initial value
		read_data = LUT_Ibias_Imod[temperature_index][1];
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS2, ptr, 2 );
		printk("Initial bias/mod current loaded from LUT\n");
	}
}


/*****************************************************************************
//Function :
//		mt7570_temperature_get
//Description :
//		This function is to get IC and BOSA temperature
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/03/26 by HC
//
******************************************************************************/
void mt7570_temperature_get(void)
{
	uint read_data = 0;
	float BOSA_temperature_offset = default_BOSA_temperature_offset;
	float Env_temperature_offset = 10;
	float T_V_slope = temperature_ADC_slop;
	float T_V_offset = temperature_offset;
   	 float sensor_voltage;

	if( (get_flash_register(flash_temperature_K_slope_offset)& flash_temperature_slope_mask) != 0xffff0000 )	//Load temperature slope
	{
		read_data = get_flash_register(flash_temperature_K_slope_offset);
		T_V_slope = (float)((read_data & flash_temperature_slope_mask) >> flash_temperature_slope_offset)/10;
	}

	if( (get_flash_register(flash_temperature_K_slope_offset)& flash_temperature_offset_mask) != 0xffff )	//Load temperature offset
	{
		read_data = get_flash_register(flash_temperature_K_slope_offset);
		T_V_offset = (float)(read_data & flash_temperature_offset_mask)/10;
	}


    mt7570_ADC_temperature_get();
	sensor_voltage = ADC_slope*((float)global_temperature_code)+ADC_offset;
	/*IC_temperature = temperature_offset - ( temperature_slop * sensor_voltage );//The equation of IC temperature*/
	IC_temperature = T_V_offset - ( T_V_slope * sensor_voltage );	//The equation of IC temperature

	read_data = get_flash_register(flash_temerature_offset);
	if( (read_data & 0x0000ffff) != 0xffff )
		BOSA_temperature_offset = read_data & 0x0000ffff;
	if( (read_data & 0xffff0000)>>16 != 0xffff )									// Enviroment temperature offset by HC 20151006
		Env_temperature_offset = (float)((read_data & 0xffff0000)>>16)/10;

	BOSA_temperature = IC_temperature - (float)BOSA_temperature_offset;
	Env_temperature = IC_temperature - Env_temperature_offset;

	#if 0
    a = (int)IC_temperature;
	b = abs( (int)(IC_temperature*10)%10 );
	printk("IC temperature = %d.%dC\n", a, b);
	printk("BG 1V76 = 0x%x\nBG 0V875 =0x%x\n", BG1V76, BG0V875);
	printk("temperature ADC = 0x%x\n", global_temperature_code);
	printk("V_ADC = 0x%x\n", (uint)(sensor_voltage*10000));
	#endif
}
/*****************************************************************************
//Function :
//		mt7570_load_MPDL_MPDH
//Description :
//		This function is to load MPDL and MPDH setting from FLASH, and start ERC
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/01/29 by HC
//
******************************************************************************/
void mt7570_load_MPDL_MPDH(void)
{
	unchar ptr[4];
	uint read_data = 0;
	if( (get_flash_register(flash_P1_target) != 0xffffffff) && (get_flash_register(flash_P0_target) != 0xffffffff))
	{
		// Read MPDL/MPDH
		read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask)) | ((get_flash_register(flash_P0_target) & (flash_P0_mask)) << 16);
		ptr[0] = read_data;
		ptr[1] = (read_data >> 8);
		ptr[2] = (read_data >> 16);
		ptr[3] = (read_data >> 24);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH, ptr, 4 );

		// Start P0 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = (ptr[0] & ERC_start_mask) | (ERC_start);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4 );
		// Start P1 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = (ptr[0] & ERC_start_mask) | (ERC_start);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 4 );

		printk("MPDL/MPDH loaded\n");
	}
}

/*****************************************************************************
//function :
//		mt7570_temperature_get_8472
//description :
//		this function is used to get temperature
//input :
//		N/A
//output :
//		temperature in 8472 type
//
******************************************************************************/
UINT16 mt7570_temperature_get_8472(void)
{
	int a = 0;
	int b = 0;

	mt7570_temperature_get();
	if( IC_temperature > 0)
		global_IC_temperature_8472 = (UINT16)(IC_temperature*256);
	else
		global_IC_temperature_8472 = temperature_two_complement - (UINT16)( (-1)*IC_temperature*256 );

	a = (int)IC_temperature;
	b = abs( (int)(IC_temperature*10)%10 );
	PON_PHY_MSG(PHY_MSG_DBG, "IC temperature = %d.%dC\n", a, b);

	return global_IC_temperature_8472;
}
/*****************************************************************************
//function :
//		mt7570_supply_voltage_get_8472
//description :
//		this function is used to get supply voltage
//input :
//		N/A
//output :
//		supply voltage in 8472 type
//
******************************************************************************/
UINT16 mt7570_supply_voltage_get_8472(void)
{
	#if 0
	unchar ptr[2];
	uint voltage = 0;
	float voltage_slope = default_voltage_slope;
	#endif
	#if 1
	//To 8472
	global_supply_voltage_8472 = 2*(ADC_slope*mt7570_ADC_voltage_get()+ADC_offset)/voltage_8472_unit;
	return global_supply_voltage_8472;
	#else
	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
		return global_supply_voltage_8472;
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		//internal termination resistor is off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask))|(LA_Rx_HighZ_enable);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 1);

		mdelay(1);

		//latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

		mdelay(1);

		//read voltage value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		voltage = (ptr[0] | (ptr[1]<<8));

		if(get_flash_register(flash_voltage_slope)!=0xffffffff)
			voltage_slope = get_flash_register(flash_voltage_slope) * (0.000001);

		//To 8472
		global_supply_voltage_8472 = (voltage_slope/voltage_8472_unit)*voltage;

		//internal termination resistor is on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD , ptr, 1);
		ptr[0] = (ptr[0] & (LA_Rx_HighZ_mask));
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 1);

		atomic_set(&i2c_protect, 0);

		return global_supply_voltage_8472;
	}
	#endif
}
/*****************************************************************************
//function :
//		mt7570_bias_current_get_8472
//description :
//		this function is used to get bias current
//input :
//		N/A
//output :
//		bias current in 8472 type
//
******************************************************************************/
UINT16 mt7570_bias_current_get_8472(void)
{
	UINT16 bias_current_8472 = 0;

	bias_current_8472 = mt7570_information_output(Ibias)*bias_current_slope*1000/bias_current_8472_slope;	//To 8472

	if( FiberPlug == 1 && bias_current_8472 < 500 )						// Fiber was plugged and still not recovered ( < 1000uA )
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Fiber isn't recovered\n");
	}
	else
	{
		if( global_bias_current_8472 - bias_current_8472 > 500 )	// bias variation > 1000uA
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Fiber plugged\n");
			FiberPlug = 1;															// Fiber was plugged
		}
		else
			FiberPlug = 0;
	}

	global_bias_current_8472 = bias_current_8472;
	return global_bias_current_8472;
}
/*****************************************************************************
//function :
//		mt7570_RSSI_calibration
//description :
//		this function is to calibrate Rx RSSI current
//input :
//		N/A
//output :
//		N/A
//
******************************************************************************/
void mt7570_RSSI_calibration(void)
{
		unchar ptr[4];
		uint read_data = 0;

		// RSSI calibration on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);
		ptr[1] = (ptr[1]&RSSI_cal_mask) | RSSI_cal_en;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);

		// RSSI V mode on
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);
		ptr[1] = (ptr[1]&0xbf) | 0x40;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);

		// Set ADC to RSSI output
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		// Latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
		// Read ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		RSSI_Vref = ptr[0] | (ptr[1]<<8);
		printk("RSSI_Vref = 0x%x\n", RSSI_Vref);

		// RSSI V mode off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);
		ptr[1] = ptr[1] & 0xbf;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);

		// Latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
		// Read ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		RSSI_V = ptr[0] | (ptr[1]<<8);
		printk("RSSI_V = 0x%x\n", RSSI_V);

		// RSSI calibration off
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);
		ptr[1] = ptr[1] & RSSI_cal_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 2);

		// Set ADC to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		// RSSI_V = RSSI_Vref + 0.35*RSSI_factor;
		if( RSSI_V > RSSI_Vref )
			RSSI_factor = ( ADC_slope*(RSSI_V- RSSI_Vref)+ADC_offset  )/0.35;
		else
			printk("RSSI calibration fail\n");
}
/*****************************************************************************
//function :
//		mt7570_RSSI_current
//description :
//		this function is to get RSSI current
//input :
//		N/A
//output :
//		RSSI current
//
******************************************************************************/
int mt7570_RSSI_current(void)
{
	unchar ptr[4];
	uint read_data        = 0;
	uint RSSI_voltage     = 0;
	uint RSSI_GAIN        = 0;
	uint RSSI_GAIN_factor = 0;
	uint cnt = 4;
	uint temp = 0;
	uint i = 0;
	int search_cnt        = 0;											//by HC 20150401

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
		return global_RSSI_current;
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		//set ADC to RSSI voltage output
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		for( search_cnt = 0; search_cnt < 6; search_cnt++ )				//by HC 20150401
		{
			RSSI_GAIN = 5 - search_cnt;		//RSSI_GAIN from 5 to 0			//by HC 20150401
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
			ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
			temp = 0;
			for(i=0;i<cnt;i++)
			{									//by Biker 20150615
				//latch ADC value
				phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
				ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
				//read voltage value
				phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
				temp += ptr[0] | (ptr[1]<<8);
			}
			RSSI_voltage = ((temp>>1)+1)>>1;

			if(RSSI_voltage < RSSI_Vref - ADC_RSSI_defend_noise_threshold)	//by Biker 20150615
				break;
		}
		if (ADC_0V5> RSSI_voltage)
			PON_PHY_MSG(PHY_MSG_DBG, "RSSI ADC code lower than 0.5V, not precise!\n");	//by Biker 20150615

		PON_PHY_MSG(PHY_MSG_DBG, "RSSI GAIN = %d, RSSI_voltage = %x, \n", RSSI_GAIN,RSSI_voltage);
		// Set RSSI gain back to 0x05
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | (0x05);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);

		// Set ADCAUX back to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

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

		if ( RSSI_Vref >= RSSI_voltage )
		{
			global_RSSI_current = (RSSI_Vref - RSSI_voltage)*RSSI_GAIN_factor;
			return global_RSSI_current;
		}
		else
		{
			global_RSSI_current = 0;
			return global_RSSI_current;
		}
	}
}


/*****************************************************************************
//function :
//		mt7570_dark_current
//description :
//		this function is to find BOSA's dark current
//input :
//		N/A
//output :
//		dark current
//Date :
//		2016/04/11 by HC
******************************************************************************/
int mt7570_dark_current(void)
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
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		read_data = ptr[0];
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_RSSI_enable;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		RSSI_GAIN = 0;//5 - search_cnt;		//RSSI_GAIN from 5 to 0			//by HC 20150401

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | RSSI_GAIN;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);

		for(i=0;i<cnt;i++)
		{									//by Biker 20150615
			//latch ADC value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
			ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);
			//read voltage value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
			RSSI_voltage += ptr[0] | (ptr[1]<<8);
		}

		printk("RSSI voltage = %x\n", RSSI_voltage);

		// Set RSSI gain back to 0x05
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);
		ptr[2] = (ptr[2] & RSSI_gain_mask) | (0x05);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LA_PWD, ptr, 4);

		// Set ADCAUX back to default
		ptr[0] = read_data;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		atomic_set(&i2c_protect, 0);
	}

	return 0;
}


/*****************************************************************************
//Function :
//		mt7570_RougeONU_clear
//Description :
//		This function is to clear rouge ONU status
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/01/27 by HC
******************************************************************************/
void mt7570_RougeONU_clear(void)
{
	unchar ptr[1];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_ROGUE_ONU_DET_CTRL + 0x01, ptr, 1);
	ptr[0] = (ptr[0] & RogueONU_mask) | 0x1;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_ROGUE_ONU_DET_CTRL + 0x01, ptr, 1);
	printk("Rogue ONU clear\n");
}
/*****************************************************************************
//Function :
//		mt7570_MPD_current_calibration
//Description :
//		This function gets BOSA's MPD current offset value when Tx is off.
//		Waiting for EN7570 E2
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/01/18 by HC
******************************************************************************/
void mt7570_MPD_current_calibration(void)
{
		unchar ptr[2];
		#if 0														// It's neccesary if after disabling BOSA Tx power
		mdelay(1000);
		#endif
		// Set RG_TIAMUX to b'100										//by HC 20150413
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
		ptr[0] = (ptr[0] & tia_mux_mask) | default_tia_mux_select;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

		//Set RG_SVADC_INMUX[04:01] = b'0010 ( 0x24[2] = 1 )
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_TxPW_enable;		//and 11100001 or 100
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		//Latch ADC value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
		ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

		//Read MPD current value
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
		MPD_current_offset = (ptr[0] | (ptr[1]<<8));
		global_MPD_current = MPD_current_offset;					//Default by HC 20150213
		printk("MPD Current Offset = 0x%x\n", MPD_current_offset);

		// Set RG_TIAMUX to default b'100								//by HC 20150413
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
		ptr[0] = (ptr[0] & tia_mux_mask) | default_tia_mux_select;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

		//Set RG_SVADC_INMUX to default 0
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		ptr[0] = (ptr[0] & ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
}


/*****************************************************************************
//Function :
//		mt7570_MPD_current
//Description :
//		This function is to get BOSA's MPD current
//Input :
//		N/A
//Output :
//		MPD current
//
******************************************************************************/
int mt7570_MPD_current(void)
{
	unchar ptr[2];
	uint MPD_current = 0;
	uint temp = 0;
	int i	  = 0;
	int cnt   = 0;

	if( vEN7570 == 0 && TEC_switch )			//by HC 20150331
		cnt = 10;
	else
		cnt = 1;

	if( i2c_access_protect() == -1 )
	{
		printk("\r\n atomic, [%s : %d]\r\n", __FUNCTION__ , __LINE__);
		return global_MPD_current;
	}
	else
	{
		atomic_set(&i2c_protect, 1);

		//Set RG_TIAMUX to b'000																				// by HC 20150413
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
		ptr[0] = (ptr[0] & tia_mux_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

		//Set ADC to LDD voltage output, RG_SVADC_INMUX = b'0010 ( 0x24[2] = 1 )
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		ptr[0] = (ptr[0] & ADC_select_mask) | ADC_TxPW_enable;	//and 11100001 or 100
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		for( i=0; i<cnt; i++ )																				// YMC_20150206
		{
			//Latch ADC value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)), ptr, 1);
			ptr[0] = (ptr[0] & (ADC_latch_mask))|(ADC_latch);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, (mt7570_PROBE_CONTROL+(0x01)) , ptr, 1);

			//Read MPD current value
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ADC_PROBE_STATUS, ptr, 2);
			MPD_current = (ptr[0] | (ptr[1]<<8));

			if(temp == 0)																					// YMC_20150206
				temp = MPD_current;
			else if(temp < MPD_current)																		// by HC 20150226
				temp = MPD_current;
		}

		//Set RG_SVADC_INMUX to default 0
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);
		ptr[0] = (ptr[0] & ADC_select_mask);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 1);

		// Set RG_TIAMUX to default b'100																		//by HC 20150413
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);
		ptr[0] = (ptr[0] & tia_mux_mask) | default_tia_mux_select;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX, ptr, 1);

		atomic_set(&i2c_protect, 0);

		PON_PHY_MSG(PHY_MSG_DBG, "MPD temp = 0x%x\n", temp);
		if( temp < MPD_current_offset )							//No Tx power
		{
			PON_PHY_MSG(PHY_MSG_DBG, "No Tx power, return %x\n", MPD_current_offset);
			global_MPD_current = MPD_current_offset;
			return global_MPD_current;
		}

		if( vEN7570 == 0 && TEC_switch )		//by HC 20150331
		{
			int div              = 0;
			int MPD_current_up   = get_flash_register(flash_MPD_point_up);
			int MPD_current_dn   = MPD_current_offset;
			float slope 		 = 0;
			float tx_power  	 = 0;		//unit: mW x100
			int tx_power_up 	 = 0;		//unit: mW x100
			int tx_power_dn 	 = 0;		//unit: mW x100
			UINT16 tx_power_8472 = 0;		//unit: 0.1 uW
			int TX_POWER_THRESHOLD_up = 0;
			int TX_POWER_THRESHOLD_dn = 0;

			div = MPD_current_up - MPD_current_dn;
			if( div == 0 )
			{
				printk("Invalid content of FLASH 0x88\n");
				global_MPD_current = temp;
				return global_MPD_current;
			}
			else//div != 0
			{
				if( xPON_mode == PHY_GPON_CONFIG )			// by HC 20151119
				{
					PON_PHY_MSG(PHY_MSG_DBG, "GPON mode\n");
					tx_power_up = 0xb1;
					TX_POWER_THRESHOLD_up = TX_8472_POWER_THRESHOLD_4dBm;
					TX_POWER_THRESHOLD_dn = TX_8472_POWER_THRESHOLD_1dBm;
				}
				else if( xPON_mode == PHY_EPON_CONFIG )	// by HC 20151119
				{
					PON_PHY_MSG(PHY_MSG_DBG, "EPON mode\n");
					tx_power_up = 0x9e;
					TX_POWER_THRESHOLD_up = TX_8472_POWER_THRESHOLD_3dBm5;
					TX_POWER_THRESHOLD_dn = TX_8472_POWER_THRESHOLD_0dBm5;
				}

				slope = (float)(tx_power_up-tx_power_dn)/(float)(MPD_current_up-MPD_current_dn);
				tx_power = (float)(temp-MPD_current_dn)*slope + tx_power_dn;
				tx_power_8472 = (int)(10*tx_power)/0.1;

				if( global_MPD_current_var==0 && tx_power_8472>TX_POWER_THRESHOLD_up)
				{
					MPD_current_cnt += 1;
					if( MPD_current_cnt == 2)
						global_MPD_current_var = temp - get_flash_register(flash_MPD_point_up);
				}
				PON_PHY_MSG(PHY_MSG_DBG, "MPD current var = 0x%x\n", global_MPD_current_var);

				if( tx_power_8472>TX_POWER_THRESHOLD_up && global_MPD_current_var!=0 )								//Zone 1
				{
					PON_PHY_MSG(PHY_MSG_DBG, "MPD in zone 1, Tx_power_8472 = %d\n", tx_power_8472);
					temp -= global_MPD_current_var;																	//by HC 20150303
					PON_PHY_MSG(PHY_MSG_DBG, "temp(var) = 0x%x\n", temp);
					tx_power = (float)(temp-MPD_current_dn)*slope + tx_power_dn;
					tx_power_8472 = (int)(10*tx_power)/0.1;
					if( tx_power_8472>TX_POWER_THRESHOLD_up || tx_power_8472<TX_POWER_THRESHOLD_dn)					//Zone 1a, modified by HC 20150603
					{
						PON_PHY_MSG(PHY_MSG_DBG, "MPD in zone 1a, Tx_power_8472 = %d, return %x\n", tx_power_8472, global_MPD_current);
						global_MPD_current_var = 0;
						MPD_current_cnt = 1;
						return global_MPD_current;
					}
					else																							//Zone 1b
					{
						PON_PHY_MSG(PHY_MSG_DBG, "MPD in zone 1b, Tx_power_8472 = %d, return %x\n", tx_power_8472, temp);
						global_MPD_current = temp;
						return global_MPD_current;
					}
				}
				else if( tx_power_8472>TX_POWER_THRESHOLD_up && global_MPD_current_var==0 )							//Zone 2
				{
					PON_PHY_MSG(PHY_MSG_DBG, "MPD in zone 2, Tx_power_8472 = %d, return %x\n", tx_power_8472, MPD_current_up);
					global_MPD_current = MPD_current_up;
					return global_MPD_current;
				}
				else																								//Zone 3
				{
					PON_PHY_MSG(PHY_MSG_DBG, "MPD in zone 3, Tx_power_8472 = %d, return %x\n", tx_power_8472, temp);
					global_MPD_current = temp;																		// by HC 20150413
					return global_MPD_current;
				}
			}
		}
		else
		{
			global_MPD_current = temp;
			return global_MPD_current;
		}
	}
}
/*****************************************************************************
//function :
//		mt7570_RxPower_get_8472
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
UINT16 mt7570_RxPower_get_8472(void)
{
	int read_data = 0;

	int RSSI_current    = 0;

	int rx_power  = 0;			//unit: uW x100
	UINT16 rx_power_8472 = 0;	//unit: 0.1 uW

	float div     = 0;
	float slope   = 0;
	int RSSI_current_up = 0;
	int RSSI_current_md = 0;
	int RSSI_current_dn = 0;

	int rx_power_up = 0;		//unit: uW x100
	int rx_power_md = 0;		//unit: uW x100
	int rx_power_dn = 0;		//unit: uW x100

	float RSSI_current_1, RSSI_current_2, RSSI_current_3, Rx_power_K1, Rx_power_K2, Rx_power_K3;
	static float d, e, f, g, x, y, z;

	if( (get_flash_register(flash_Rx_power_K_point4)&0x0000000f) == 1 )	//K 3-points to calculate formula: rx_power = x*RSSI^2 + y*RSSI + z;  biker_20151001
	{
		if (global_DDMI_rx_cal_flag == 0)
		{
			read_data = get_flash_register(flash_Rx_power_K_point1);
			RSSI_current_1 = read_data & flash_Rx_RSSI_current_mask;
			Rx_power_K1 = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;

			read_data = get_flash_register(flash_Rx_power_K_point2);
			RSSI_current_2 = read_data & flash_Rx_RSSI_current_mask;
			Rx_power_K2 = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;

			read_data = get_flash_register(flash_Rx_power_K_point3);
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

			global_DDMI_rx_cal_flag = 1;
		}

		RSSI_current = mt7570_RSSI_current();
		rx_power = (float) RSSI_current*RSSI_current*x + (float)RSSI_current*y + z;

		if(rx_power < 0)
			rx_power = 0;
		PON_PHY_MSG(PHY_MSG_DBG, "x *10^12 = %d, y*10^6 = %d, z*10 = %d\n",(int)(x*1000000000000),(int)(y*1000000),(int)(z*10));

	}
	else 	//3_line algorithm
	{
		RSSI_current = mt7570_RSSI_current();
		if( get_flash_register(flash_Rx_power_K_point1) != 0xffffffff )	//Load Rx power point #1
		{
			read_data = get_flash_register(flash_Rx_power_K_point1);
			RSSI_current_up = read_data & flash_Rx_RSSI_current_mask;
			rx_power_up = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}
		if( get_flash_register(flash_Rx_power_K_point2) != 0xffffffff )	//Load Rx power point #2
		{
			read_data = get_flash_register(flash_Rx_power_K_point2);
			RSSI_current_md = read_data & flash_Rx_RSSI_current_mask;
			rx_power_md = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}
		if( get_flash_register(flash_Rx_power_K_point3) != 0xffffffff )	//Load Rx power point #3
		{
			read_data = get_flash_register(flash_Rx_power_K_point3);
			RSSI_current_dn = read_data & flash_Rx_RSSI_current_mask;
			rx_power_dn = (read_data & flash_Rx_power_mask) >> flash_Rx_power_offset;
		}

		if( RSSI_current <= RSSI_current_dn )												//Zone 1
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 1\n");
			div = RSSI_current_dn;
			if(div != 0)
			{	//slope = rx_power_dn/RSSI_current_dn
				slope = (float)rx_power_dn/(float)RSSI_current_dn;
				rx_power = RSSI_current*slope;												//Bug fixed by HC 20150203
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
		else if( (RSSI_current_dn < RSSI_current) && (RSSI_current < RSSI_current_md) ) 	//Zone 2
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 2\n");
			div = RSSI_current_md-RSSI_current_dn;
			if(div != 0)
			{	//slope = (rx_power_md-rx_power_dn)/(RSSI_current_md-RSSI_current_dn)
				slope = (float)(rx_power_md-rx_power_dn)/(float)(RSSI_current_md-RSSI_current_dn);
				rx_power = (RSSI_current-RSSI_current_dn)*slope + rx_power_dn;
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
		else if( RSSI_current_md <= RSSI_current )											//Zone 3
		{
			PON_PHY_MSG(PHY_MSG_DBG, "Rx power in zone 3\n");
			div = RSSI_current_up-RSSI_current_md;
			if(div != 0)
			{	//slope = (rx_power_up-rx_power_md)/(RSSI_current_up-RSSI_current_md)
				slope = (float)(rx_power_up-rx_power_md)/(float)(RSSI_current_up-RSSI_current_md);
				rx_power = (RSSI_current-RSSI_current_md)*slope + rx_power_md;
			}
			else
				PON_PHY_MSG(PHY_MSG_DBG, "Invalid Rx power point\n");
		}
	}

	//To 8472
	rx_power_8472 = rx_power/10;
	PON_PHY_MSG(PHY_MSG_DBG, "Rx_power_8472 = %d\n", rx_power_8472);
	return rx_power_8472;
}


/*****************************************************************************
//Function :
//		mt7570_safe_circuit_reset
//Description :
//		This function is to reset safe circuit
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/01/27 by HC
//
******************************************************************************/
void mt7570_safe_circuit_reset(void)
{
	unchar ptr[4];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SAFE_PROTECT, ptr, 4);
	ptr[1] = (ptr[1] & safe_circuit_mask) | (safe_circuit_reset);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SAFE_PROTECT, ptr, 4);
}
/*****************************************************************************
//function :
//		mt7570_TxPower_get_8472
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
UINT16 mt7570_TxPower_get_8472(void)
{
	int div 			 = 0;
	int read_data 		 = 0;
	int MPD_current    	 = 0;
	int MPD_current_up 	 = 0;
	int MPD_current_dn 	 = MPD_current_offset;	//Defualt by HC 20150126
	float slope			 = 0;
	float tx_power  	 = 0;					//unit: mW x100
	int tx_power_up 	 = 0;					//unit: mW x100
	int tx_power_dn 	 = 0;					//unit: mW x100
	UINT16 tx_power_8472 = 0;					//unit: 0.1 uW

	MPD_current = mt7570_MPD_current();
	PON_PHY_MSG(PHY_MSG_DBG, "MPD_current = 0x%x\n", MPD_current);
	if( get_flash_register(flash_Tx_power_K_point1) != 0xffffffff )
	{
		read_data = get_flash_register(flash_Tx_power_K_point1);
		MPD_current_up = read_data & flash_Tx_MPD_current_mask;
		tx_power_up = (read_data & flash_Tx_power_mask)>>flash_Tx_power_offset;
	}
	if( get_flash_register(flash_Tx_power_K_point2) != 0xffffffff )
	{
		read_data = get_flash_register(flash_Tx_power_K_point2);
		MPD_current_dn = read_data & flash_Tx_MPD_current_mask;
		tx_power_dn = (read_data & (flash_Tx_power_mask))>>flash_Tx_power_offset;
	}
	div = MPD_current_up - MPD_current_dn;
	if( div != 0 )
	{
		slope = (float)(tx_power_up-tx_power_dn)/(float)(MPD_current_up-MPD_current_dn);
		tx_power = (float)(MPD_current-MPD_current_dn)*slope + tx_power_dn;
	}
	else
	{
		PON_PHY_MSG(PHY_MSG_DBG, "Invalid Tx power point\n");
	}
	//To 8472
	tx_power_8472 = (int)(10*tx_power)/0.1;
	PON_PHY_MSG(PHY_MSG_DBG, "Tx_power_8472 = %d\n", tx_power_8472);
	return tx_power_8472;
}
/*****************************************************************************
//function :
//		mt7570_internal_clock
//description :
//		this function is used to get transceiver alarm and return to Handler function
//		this function will start in phy_ready, and stop at LOS.
//input :
//		N/A
//output :
//		N/A
******************************************************************************/
void mt7570_internal_clock(void)
{

	// Internal DDMI
	if( internal_DDMI == 1 )
	{
		if(cnt7570%T_APD == T_APD-1)
		{
			// APD control
			if( xPON_mode == PHY_GPON_CONFIG )							// by HC 20150520
				mt7570_APD_control();								//tune APD voltage per T_APD(sec). YMC 20150122
		}
		if(cnt7570%10 == 0 || fast_DDMI)
			DDMI_voltage     = mt7570_supply_voltage_get_8472();

		//if(cnt7570%10 == 2)												// Removed by HC 20151014
		DDMI_current     = mt7570_bias_current_get_8472();

		if(cnt7570%10 == 4 || fast_DDMI)
			DDMI_temperature = mt7570_temperature_get_8472();
		if(cnt7570%10 == 6 || fast_DDMI)
		{
			DDMI_tx_power    = mt7570_TxPower_get_8472();		//by HC 20150129
			mt7570_tx_power_alarm_get();
		}
		if(cnt7570%10 == 8 || fast_DDMI)
		{
			DDMI_rx_power    = mt7570_RxPower_get_8472();		//by HC 20150129
			mt7570_rx_power_alarm_get();
		}
	}




	if ( SCL == 1 )												// For single-closed loop mode
	{
		if( cnt7570%29 == 0 )
		{
			if ( ETC == 3 )
			{
				#if 0
				if( PatternEnabled || BiasTracking_switch )	// For PRBS and linking
				#endif
					mt7570_BiasTracking();					// Trace target bias current by HC, DDMI is needed
			}
		}
	}
	else if ( DOL == 1 )										// For open loop mode
	{
		if( cnt7570%5 == 0 )
			mt7570_OpenLoopControl();						// by HC 20160410
	}
	else															// For dual-closed loop mode
	{
		// Avoid eye failure after plugging fiber, DDMI is needed
		if( xPON_mode != -1 && FiberPlug == 1 && PatternEnabled == 1 )	// Just for PRBS but not for calibration
			mt7570_FiberPlug_Protect();


		if( cnt7570%30 == 29 )
		{
			if ( ETC == 1 )
			{
				mt7570_ERTemperatureCompensation();			// by YM
			}
			else if ( ETC == 2 )
			{
				// For Jay
			}
			else if ( ETC == 3 )
			{
				if( PatternEnabled || BiasTracking_switch )	// For PRBS and linking
					mt7570_BiasTracking();							// Trace target bias current by HC, DDMI is needed
			}

		}


		// TEC, DDMI is needed
		if( TEC != 0 && TEC_switch == 1 )							// Just for linking
			mt7570_TxEyeCorrection();								// by HC 20150515

	}


	// ADC calibration
	if( cnt7570%120 == 119 )											// by HC 20150727
	{
		if( xPON_mode != -1 )											// by HC 20160218
			mt7570_ADC_calibration();
	}



	cnt7570 ++;															// by HC 20150727

}

/*****************************************************************************
//function :
//		mt7570_param_status_real
//description :
//		this function is used to get transceiver parameter realtime
//		Note: before call this function, should check if(i2c_protect == 0)
//input :
//		PPHY_TransParam_T structure, defined in phy_api.h
//output :
//		N/A
******************************************************************************/
void mt7570_param_status_real(PPHY_TransParam_T transceiver_param)
{

	transceiver_param->temprature = DDMI_temperature;
	transceiver_param->supply_voltage = DDMI_voltage;
	transceiver_param->tx_current = DDMI_current;
	transceiver_param->tx_power = DDMI_tx_power;
	transceiver_param->rx_power = DDMI_rx_power;
}
/*****************************************************************************
//Function :
//		DDMI_check_8472
//Description :
//		This function is to show the five DDMI items in 8472 format
//Input :
//		N/A
//Output :
//		Five DDMI items in 8472 format
//Date :
//		2015/03/11 by HC
******************************************************************************/
void DDMI_check_8472(void)
{
	DDMI_voltage	 = mt7570_supply_voltage_get_8472();
	DDMI_current	 = mt7570_bias_current_get_8472();
	DDMI_temperature = mt7570_temperature_get_8472();
	DDMI_tx_power	 = mt7570_TxPower_get_8472();
	DDMI_rx_power	 = mt7570_RxPower_get_8472();

	printk("Supply Voltage = %05d\n" , DDMI_voltage	   );
	printk("Tx Bias Current = %05d\n", DDMI_current	   );
	printk("Temperature = %05d\n"    , DDMI_temperature);
	printk("Tx power = %05d\n"       , DDMI_tx_power   );
	printk("Rx Power = %05d\n"       , DDMI_rx_power   );
}
/*****************************************************************************
//Function :
//		mt7570_trans_model_setting
//Description :
//		This function is to
//Input :
//
//Output :
//		N/A
//Date :
//
******************************************************************************/
extern struct work_struct work_mt7570_safe_circuit_reset;

void mt7570_trans_model_setting(void)
{
		#if 1
        IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x10f);
		#if 0
        mt7570_RougeONU_clear();				//cannot call this function during interrupt occur ,by Jay 20151120
		#endif
        if(NULL != i2c_access_queue){
            queue_work(i2c_access_queue, &work_mt7570_safe_circuit_reset);
        }
		#else
		IO_SPHYREG(PHY_CSR_XPON_SETTING, 0x11f);		// 0x11f for open loop mode
		#endif

}
/*****************************************************************************
//Function :
//		mt7570_trans_alarm_getting
//Description :
//		This function is to
//Input :
//
//Output :
//		N/A
//Date :
//
******************************************************************************/
void mt7570_trans_alarm_getting(PPHY_TransAlarm_T transceiver_alarm)
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
//		mt7570_tx_power_alarm
//Description :
//		This function is to get BOB Tx alarm.
//Input :
//		N/A
//Output :
//		EN7570 Tx alarm status.
//Date :20150414 by YMC
//
******************************************************************************/
void mt7570_tx_power_alarm_get(void)
{
	if(DDMI_tx_power < mt7570_tx_power_low_alarm_thod)
		mt7570_alarm |= PHY_TRANS_TX_LOW_POWER_ALARM;
	if(DDMI_tx_power > mt7570_tx_power_high_alarm_thod)
		mt7570_alarm |= PHY_TRANS_TX_HIGH_POWER_ALARM;
	if(DDMI_current < mt7570_tx_cur_low_alarm_thod)
		mt7570_alarm |= PHY_TRANS_TX_LOW_CUR_ALARM;
	if(DDMI_current > mt7570_tx_cur_high_alarm_thod)
		mt7570_alarm |= PHY_TRANS_TX_HIGH_CUR_ALARM;
}
/*****************************************************************************
//Function :
//		mt7570_rx_power_alarm
//Description :
//		This function is to get BOB Rx alarm.
//Input :
//		N/A
//Output :
//		EN7570 Rx alarm status.
//Date :20150414 by YMC
//
******************************************************************************/
void mt7570_rx_power_alarm_get(void)
{
	if(DDMI_rx_power < mt7570_rx_power_low_alarm_thod)
		mt7570_alarm |= PHY_TRANS_RX_LOW_POWER_ALARM;
	if(DDMI_rx_power > mt7570_rx_power_high_alarm_thod)
		mt7570_alarm |= PHY_TRANS_RX_HIGH_POWER_ALARM;
}
/*****************************************************************************
//Function :
//		mt7570_EN7570_detection
//Description :
//		This function is to detect EN7570
//Input :
//		N/A
//Output :
//		0 for EN7570 not detected
//		1 for EN7570 detected
//Date :
//		2015/03/02 by HC
******************************************************************************/
int mt7570_EN7570_detection(void)
{
	unchar ptr[1];
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_FT_ADC_CLK_CLR, ptr, 1);
	if( ptr[0] == 0x03 )	//EN7570 exists
	{
		mt7570_select = 1;
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_DUMMY, ptr, 1);
		vEN7570 = ptr[0];
		return 1;
	}
	else
	{
		mt7570_select = 0;
		return 0;
	}
}
/*****************************************************************************
//Function :
//		LOS_calibration
//Description :
//		This function is to calibrate LOS level
//Input :
//		High/Low threshold
//Output :
//		N/A
//
//Date :
//		2015/03/03 by HC
******************************************************************************/
void LOS_calibration(int LOS_thld_H, int LOS_thld_L)
{
	unchar ptr[4];
	#if 1							// Replaced by HC 20160427
	mt7570_LOS_init();
	#else
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	ptr[0] = (ptr[0] & (LOS_calibration_mask)) | (LOS_calibration_trig);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_ADCREV2_mask)) | (LOS_ADCREV2_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[2] = (ptr[2] & (LOS_ADCREV1_mask)) | (LOS_ADCREV1_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
	ptr[1] = (ptr[1] & (LOS_confidence_mask)) | (LOS_confidence_setting);
	ptr[0] = (ptr[0] & (LOS_cnt_mask)) | (LOS_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
	#endif
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_comp_thld_mask)) | LOS_thld_L;
	ptr[2] = (ptr[2] & (LOS_comp_thld_mask)) | LOS_thld_H;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);

	printk("%s done\n", __FUNCTION__);
}
/*****************************************************************************
//Function :
//		mt7570_ERC_filter
//Description :
//		This function is to set digital ERC filter
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/06 by HC
******************************************************************************/
void mt7570_ERC_filter(void)
{
	unchar ptr[4];
	//sifm xw 0 0xc7 0x70 2 0x16c 4 ff a7 58 00
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ERC_FILTER_CTRL, ptr, 4);
	ptr[2] = 0x58;
	ptr[1] = 0xa7;
	ptr[0] = 0xff;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ERC_FILTER_CTRL, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ERC_FILTER_CTRL, ptr, 4);
	ptr[3] = (ptr[3] & 0xfe) | (0x01);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ERC_FILTER_CTRL, ptr, 4);

	printk("ERC filter set\n");
}
/*****************************************************************************
//Function :
//		mt7570_TIAGAIN_set
//Description :
//		This function is to set TIA gain
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/20 by HC
******************************************************************************/
void mt7570_TIAGAIN_set(void)
{
	unchar ptr[1];
	uint read_data = 0;

	if( get_flash_register(flash_TIAGAIN) != 0xffffffff )
	{
		read_data = get_flash_register(flash_TIAGAIN) & 0x3;
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX+0x0001 , ptr, 1);
		ptr[0] = (ptr[0] & 0x3f) | (read_data<<6);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_TIAMUX+0x0001, ptr, 1);

		printk("TIAGAIN set 0x%x\n", read_data);
	}
}


/*****************************************************************************
//Function :
//		show_BoB_information
//Description :
//		This function is to show BoB common information
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/03/25 by HC
******************************************************************************/
void show_BoB_information(void)
{
	unchar ptr[4];
	float temp = 0;
	int a = 0;
	int b = 0;
	int i ;

	//LOS
	#if 1
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_DBG_RG, ptr, 4);
	ptr[3] = ptr[3] & 0x1;
	printk("LOS status = %d\n", ptr[3]);
	#else
	printk("LOS status = %d\n", phy_los_status());
	#endif
	//Rouge ONU
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ROGUE_ONU_DET_CTRL, ptr, 2);
	ptr[1] = (ptr[1]>>2 & 0x1);
	printk("Rouge ONU status = %d\n", ptr[1]);

	//Tx SD
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_ROGUE_ONU_DET_CTRL, ptr, 2);
	ptr[1] = (ptr[1]>>3 & 0x1);
	printk("Tx SD status = %d\n", ptr[1]);

	//Tx fault
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SAFE_PROTECT, ptr, 2);
	ptr[1] = (ptr[1]>>7 & 0x1);
	printk("Tx fault status = %d\n", ptr[1]);

	//Bias current
	temp = (float)mt7570_information_output(Ibias)*bias_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("I bias = %d.%.2dmA\n", a, b);

	//Modulation current
	temp = (float)mt7570_information_output(Imod)*mod_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("I mod = %d.%.2dmA\n", a, b);

	//IC, BOSA, and enviroment temperature
	a = (int)IC_temperature;
	b = abs( (int)(IC_temperature*10)%10 );
	printk("IC temperature = %d.%dC\n", a, b);
	a = (int)BOSA_temperature;
	printk("BOSA temperature = %d.%dC\n", a, b);
	a = (int)Env_temperature;
	b = abs( (int)(Env_temperature*10)%10 );
	printk("Enviroment temperature = %d.%dC\n", a, b);

	//APD voltage
	a = (int)APD_voltage;
	b = (int)(APD_voltage*10)%10;
	printk("APD Voltage = %d.%dV\n", a, b);

	// VCC
	a = (int)global_supply_voltage_8472/10000;
	b = (int)global_supply_voltage_8472%10000;
	printk("VCC = %d.%.4dV\n", a, b);

	// Tx power
	temp = (float)DDMI_tx_power/1000/10;	// mW
	if( temp == 0 )
		printk("Tx power = -40 dBm\n");
	else
	{
		temp = (temp-1)/temp;			// ( x-1)/x
		temp = temp + (temp*temp)/2 + (temp*temp*temp)/3 + (temp*temp*temp*temp)/4 + (temp*temp*temp*temp*temp)/5 + (temp*temp*temp*temp*temp*temp)/6;	// lnX
		temp = temp/2.302585093;	// logX = lnX / ln10
		temp *= 10; 						// 10*logX
		a = (int)temp;
		b = abs( (int)(temp*100)%100 );
		printk("Tx power = %d.%.2d dBm\n", a, b);
	}

	// Rx power by YW_20160517
	temp = (float)DDMI_rx_power;
	for ( i=0; i<7 ; i++) 		// find resolution of input 0.5 <= x < 5
	{
		if ( temp >= 5)
			temp = temp/10;
		else
			break;
	}

	if( temp == 0 )
		printk("Rx power = -40 dBm\n");
	else
	{
		temp = (temp-1)/temp;			// ( x-1)/x
		temp = temp + (temp*temp)/2 + (temp*temp*temp)/3 + (temp*temp*temp*temp)/4 + (temp*temp*temp*temp*temp)/5 + (temp*temp*temp*temp*temp*temp)/6 + (temp*temp*temp*temp*temp*temp*temp)/7 +(temp*temp*temp*temp*temp*temp*temp*temp)/8 ;	// lnX
		temp = temp/2.302585093;	// logX = lnX / ln10
		temp -= 4-i;						// to log (mW)
		temp *= 10;						// 10*logX
		a = (int)temp;
		b = abs( (int)(temp*100)%100 );
		printk("Rx power = %d.%.2d dBm\n", a, b);
	}
}
/*****************************************************************************
//Function :
//		mt7570_TxEyeCorrection
//Description :
//		This function is to fix Tx eye
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/05/13 by HC
******************************************************************************/
void mt7570_TxEyeCorrection(void)
{
	uint MPDL = 0;
	unchar ptr[4];
	static int TEC_cnt = 0;

	if( TEC_cnt > 7 )												// Limit to 8 times by HC 20151123
		return;

	if ( (global_bias_current_8472<<1) < BOSA_Ith )		// Bias current < BOSA's Ith, eye failed
	{
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 1);

		if( (ptr[0]&0x0f) == 0x5 )								// In dual-closed loop mode by HC 20151123
		{
			MPDL = mt7570_information_output(P0) + 0x4;		// P0 + 0x4
			ptr[0] = MPDL;
			ptr[1] = MPDL >> 8;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2, ptr, 2);

			mt7570_restart_ERC_P0();
			printk("TEC done\n");
			TEC_cnt++;
		}
		#if 0
		else
			//printk("TEC is not in dual-closed loop mode\n");
		#endif
	}


}


/*****************************************************************************
//Function :
//		mt7570_OpenLoopMode
//Description :
//		This function is to switch to open-loop mode
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/06/02 by HC
******************************************************************************/
void mt7570_OpenLoopMode(void)
{
	unchar ptr[4];

	// P0 open-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_open_loop_mode;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 1);

	// P1 open-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P1_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_open_loop_mode;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 1);

	printk("Switch to Open-Loop Mode\n");
}
/*****************************************************************************
//Function :
//		save calibration_tempature
//Description :
//		This function is to save tempature after calibration
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/09/02 by Jay
******************************************************************************/
void Ibias_temperature_set(void)
{
	int a = 0;
	int b = 0;
	int read_data = 0;
	unchar ptr[4];
	float temp = 0;
	mt7570_temperature_get();
	a = (int)IC_temperature;

	read_data = get_flash_register(flash_reserved_for_ETC1);
	read_data = (read_data & 0xffff0000 ) | a;
	set_flash_register(read_data ,flash_reserved_for_ETC1);
	printk("tempature %d degree save done\n",a);


	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS2, ptr, 2);
	read_data = get_flash_register(flash_reserved_for_ETC1);
	read_data = ((read_data & 0x0000ffff )|( ptr[0] << 16 ) |( ptr[1] << 24 ));
	set_flash_register(read_data ,flash_reserved_for_ETC1);
	#if 0
	printk("flash value = 0x%.8x ",read_data);
	printk("\t");
	#endif
	temp = (float)mt7570_information_output(Ibias)*bias_current_slope;	//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	printk("Ibias current = %d.%.2dmA \n", a, b);

}



/*****************************************************************************
//Function :
//		mt7570_DualClosedLoopMode
//Description :
//		This function is to switch to dual closed-loop mode
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/06/02 by HC
******************************************************************************/
void mt7570_DualClosedLoopMode(void)
{
	unchar ptr[4];

	// P0 closed-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_start;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 1);

	// P1 closed-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P1_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_start;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 1);

	printk("Switch to Dual Closed-Loop Mode\n");
}

/*****************************************************************************
//Function :
//		mt7570_Vtemp_ADC_get
//Description :
//		This function is to get 7570 temperature sensor voltage by digital code
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/07/10 by YM
******************************************************************************/
void mt7570_Vtemp_ADC_get(void)
{
	float sensor_voltage;

	mt7570_ADC_temperature_get();
	sensor_voltage = ADC_slope*((float)global_temperature_code)+ADC_offset;
	printk("V_ADC = 0x%x\n", (uint)(sensor_voltage*10000));
}

/*****************************************************************************
//Function :
//		mt7570_ERTemperatureCompensation
//Description :
//		This function is to fix Tx ER value
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/07/31 by HC
******************************************************************************/
void mt7570_ERTemperatureCompensation(void)
{
	unchar ptr[4];
	uint read_data = 0;
	int norm_temp  =40;
	int Power_comp = 0;

	uint Power_comp_P0_Hi = 0;
	uint Power_comp_P1_Hi = 0;
	uint Power_comp_P0_Lo = 0;
	uint Power_comp_P1_Lo = 0;

	if((SCL ==1) || (DOL==1))
		return;

	if( (TEC != 0) && (TEC_switch == 1) )
	{
		if (Ibias_outputlow_flag ==1)
	{
			printk("Ibias output lower\n");
			return;
		}
	}
	else
	{
		Ibias_outputlow_flag = 0;
	}

			#if 1
			printk("ETC\n");
			#endif
			if( (get_flash_register(flash_reserved_for_ETC1) != 0xffffffff))
			{
				norm_temp = (get_flash_register(flash_reserved_for_ETC1) & 0xff) ;
			}

			if( (get_flash_register(flash_ETC_Hi_Lo_target_delta)&0xff) != 0xff)
			{
				Power_comp_P0_Hi = (get_flash_register(flash_ETC_Hi_Lo_target_delta) & 0xff) ;
			}
			if( ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>8)&0xff) != 0xff)
			{
				Power_comp_P1_Hi = ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>8)&0xff) ;
			}
			if( ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>16)&0xff) != 0xff)
			{
				Power_comp_P0_Lo = ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>16)&0xff) ;
			}
			if( ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>24)&0xff) != 0xff)
			{
				Power_comp_P1_Lo = ((get_flash_register(flash_ETC_Hi_Lo_target_delta)>>24)&0xff) ;
			}
			#if 0
			printk("IC_temperature = %d\n",(int)(IC_temperature ) );
			printk("Power_comp_P0_Hi = %d\n",Power_comp_P0_Hi );
			printk("Power_comp_P1_Hi = %d\n",Power_comp_P1_Hi );
			printk("Power_comp_P0_Lo = %d\n",Power_comp_P0_Lo );
			printk("Power_comp_P1_Lo = %d\n",Power_comp_P1_Lo );
			#endif
			if ( (int)(IC_temperature ) > norm_temp && ((Power_comp_P0_Hi+Power_comp_P1_Hi)!=0))
			{
				if((int)IC_temperature > (norm_temp+25))
					Power_comp = Power_comp_P0_Hi;
				else
					Power_comp = (int)(((int)IC_temperature - norm_temp)*((float)Power_comp_P0_Hi/25.0)) ;

				read_data = (get_flash_register(flash_P0_target) & (flash_P0_mask));
				#if 0
				printk("Power_comp_P0 = %d\n",Power_comp );
				#endif
				read_data = read_data + Power_comp ;
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2, ptr, 2);
				if((int)IC_temperature > (norm_temp+25))
					Power_comp = Power_comp_P1_Hi;
				else
					Power_comp = (int)(((int)IC_temperature - norm_temp)*((float)Power_comp_P1_Hi/25.0)) ;

				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask));
				#if 0
				printk("Power_comp_P1 = %d\n",Power_comp );
				#endif
				read_data = read_data + Power_comp ;
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 2);

                mt7570_TxSD_level_set();
			}
			else if ( (int)(IC_temperature ) < (norm_temp-20) && ((Power_comp_P0_Lo+Power_comp_P1_Lo)!=0))
			{

				if((int)IC_temperature < (norm_temp-30))
					Power_comp = Power_comp_P0_Lo*(-1);
				else
					Power_comp = (int)(((int)IC_temperature - (norm_temp-20))*((float)Power_comp_P0_Lo/10.0)) ;

				read_data = (get_flash_register(flash_P0_target) & (flash_P0_mask));
				#if 0
				printk("Power_comp_P0 = %d\n",Power_comp );
				#endif
				read_data = read_data + Power_comp ;
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);

				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2, ptr, 2);
				if((int)IC_temperature < (norm_temp-30))
					Power_comp = Power_comp_P1_Lo*(-1);
				else
					Power_comp = (int)(((int)IC_temperature - (norm_temp-20))*((float)Power_comp_P1_Lo/10.0)) ;

				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask));
				#if 0
				printk("Power_comp_P1 = %d\n",Power_comp );
				#endif
				read_data = read_data + Power_comp ;
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 2);
                mt7570_TxSD_level_set();
			}
			else
			{
				// Read MPDL/MPDH
				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask)) | ((get_flash_register(flash_P0_target) & (flash_P0_mask)) << 16);
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2 , ptr, 2);

				ptr[2] = (read_data >> 16);
				ptr[3] = (read_data >> 24);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 4);

				#if 0
				printk("none \n");
				#endif
                mt7570_TxSD_level_set();
			}
}


/*****************************************************************************
//Function :
//		mt7570_Single_open_loop_ERTemperatureCompensation
//Description :
//		This function is to fix Single loop Tx ER value
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/08/31 by Jay
******************************************************************************/
void SOL_mt7570_ERTemperatureCompensation(void)
{
	unchar ptr[4];
	uint read_data = 0;

	int norm_temp  =40;
	int Power_comp = 0;

	float temp = 0;
	int LT_turnning_point = 0;
	int NT_turnning_point = 0;
	int HT_turnning_point = 0;

	int LT_turnning_point_ibias = 0;
	int NT_turnning_point_ibias = 0;
	int HT_turnning_point_ibias = 0;

	uint Ibias_init_tmp = 0;
	uint Ibias_slop1 = 0;
	uint Ibias_slop2 = 0;
	uint Ibias_slop3 = 0;
	uint Ibias_slop4 = 0;

	uint Power_comp_P1_Hi = 0;
	uint Power_comp_P1_Lo = 0;
	int a = 0;
	int b = 0;
	int Ibias_set = 0;

	if((get_flash_register(flash_reserved_for_ETC1) | get_flash_register(flash_reserved1) | get_flash_register(flash_Ibias_SLope) | get_flash_register(flash_P1_target) )== 0xffffffff )
	{
		printk("SOL wrong flash setting \n");
	}
	else
	{
			//get tempature
			if( (get_flash_register(flash_reserved_for_ETC1) != 0xffffffff))
			{
				norm_temp = (get_flash_register(flash_reserved_for_ETC1) & 0xff) ;
			}
			//get LT/NT/LT turnning point
			//LT
			if( (get_flash_register(flash_reserved1)&0xff) != 0xff)
			{
				LT_turnning_point = (get_flash_register(flash_reserved1) & 0xff) ;
			}
			//NT
			if( ((get_flash_register(flash_reserved1)>>8 )&0xff) != 0xff)
			{
				NT_turnning_point = ((get_flash_register(flash_reserved1)>>8) & 0xff) ;
			}
			//HT
			if( ((get_flash_register(flash_reserved1)>>16 )&0xff) != 0xff)
			{
				HT_turnning_point = ((get_flash_register(flash_reserved1) >>16) & 0xff) ;
			}

			//get 4 segment slope
			//Slope 1
			if( (get_flash_register(flash_Ibias_SLope)&0xff) != 0xff)
			{
				Ibias_slop1 = (get_flash_register(flash_Ibias_SLope) & 0xff) ;
			}
			//slope 2
			if( ((get_flash_register(flash_Ibias_SLope) >>8 )&0xff) != 0xff)
			{
				Ibias_slop2 = ((get_flash_register(flash_Ibias_SLope) >> 8) & 0xff) ;
			}
			//slope 3
			if( ((get_flash_register(flash_Ibias_SLope) >>16 )&0xff) != 0xff)
			{
				Ibias_slop3 = ((get_flash_register(flash_Ibias_SLope) >> 16) & 0xff) ;
			}
			//slope 4
			if( ((get_flash_register(flash_Ibias_SLope) >>24 )&0xff) != 0xff)
			{
				Ibias_slop4 = ((get_flash_register(flash_Ibias_SLope) >> 24) & 0xff) ;
			}

			//get K  init Ibias
			if( ((get_flash_register(flash_reserved_for_ETC1) >> 16)&0xfff) != 0xfff)
			{
				Ibias_init_tmp = ((get_flash_register(flash_reserved_for_ETC1) >> 16) & 0xfff) ;
			}
			else
			{
				printk("SOL Ibias equal to 0 , not correct \n");
				return;
			}
			//get P1 com
			//L T comp
			if( (get_flash_register(flash_P1_SLope)&0xff) != 0xff)
			{
				Power_comp_P1_Lo = (get_flash_register(flash_P1_SLope) & 0xff) ;
			}
			//HT comp
			if( ((get_flash_register(flash_P1_SLope) >>16 )&0xff) != 0xff)
			{
				Power_comp_P1_Hi = ((get_flash_register(flash_P1_SLope) >> 16) & 0xff) ;
			}

			#if 0
				printk("IC_temperature = %d\n",(int)(IC_temperature ) );
				printk("LT_turnning_point = %x\n",LT_turnning_point );
				printk("NT_turnning_point = %x\n",NT_turnning_point );
				printk("HT_turnning_point = %x\n",HT_turnning_point );
				printk("Ibias_slop1 = %x\n",Ibias_slop1 );
				printk("Ibias_slop2 = %x\n",Ibias_slop2 );
				printk("Ibias_slop3 = %x\n",Ibias_slop3 );
				printk("Ibias_slop4 = %x\n",Ibias_slop4 );
				printk("Ibias_init_tmp = %x\n",Ibias_init_tmp );
				printk("Power_comp_P1_Lo = %x\n",Power_comp_P1_Lo );
				printk("Power_comp_P1_Hi = %x\n",Power_comp_P1_Hi );
			#endif

			//set P1 comp
			if ( (int)(IC_temperature ) > norm_temp )
			{
				if((int)IC_temperature > 60)
				{
					Power_comp = Power_comp_P1_Hi;
				}
				else
				{
					Power_comp = (int)(((int)IC_temperature -norm_temp)*((float)Power_comp_P1_Hi/(60 - norm_temp ))) ;
				}

				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask));
				#if 0
				printk("SOL : Power_comp_P1 plus = %d\n",Power_comp );
				#endif

				read_data = read_data + Power_comp ;
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 2);

                mt7570_TxSD_level_set();

			}
			else if ( (int)(IC_temperature ) < (norm_temp) )
			{
				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask));
				if((int)IC_temperature < -10)
				{
					Power_comp = Power_comp_P1_Lo;
					read_data = read_data - Power_comp ;
					#if 0
					printk("SOL : under -10 ,Power_comp_P1 minus = %d\n",Power_comp );
					#endif
				}
				else
				{
					if((int)IC_temperature < 0)
					{
						Power_comp = (int)(((abs((int)IC_temperature))+ norm_temp)*((float)Power_comp_P1_Lo/(norm_temp + 10))) ;
						read_data = read_data - Power_comp ;
						#if 0
						printk("SOL : under 0 ,Power_comp_P1 minus = %d\n",Power_comp );
						#endif
					}
					else
					{
						Power_comp = (int)(((int)IC_temperature-norm_temp)*((float)Power_comp_P1_Lo/(norm_temp + 10))) ;
						read_data = read_data + Power_comp ;
						#if 0
						printk("SOL : Power_comp_P1 minus = %d\n",Power_comp );
						#endif
					}
				}

				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 2);

                mt7570_TxSD_level_set();
			}
			else
			{
				read_data = (get_flash_register(flash_P1_target) & (flash_P1_mask));
				ptr[0] = read_data;
				ptr[1] = (read_data >> 8);
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH , ptr, 2);

       	        mt7570_TxSD_level_set();
			}

			if((NT_turnning_point < LT_turnning_point ) || ( NT_turnning_point > HT_turnning_point ))
			{
				printk("turning point range error\n");
				return;
			}

			//set Ibias comp
			if(( norm_temp > NT_turnning_point) &&( norm_temp < HT_turnning_point))
			{
				NT_turnning_point_ibias = Ibias_init_tmp - Ibias_slop3*(norm_temp - NT_turnning_point);
				HT_turnning_point_ibias = Ibias_init_tmp + Ibias_slop3*(HT_turnning_point -norm_temp);
				LT_turnning_point_ibias = NT_turnning_point_ibias - Ibias_slop2*(NT_turnning_point - LT_turnning_point);
			}
			else if (( NT_turnning_point > norm_temp) && ( norm_temp > LT_turnning_point))
			{
				NT_turnning_point_ibias = Ibias_init_tmp + Ibias_slop2*(NT_turnning_point - norm_temp);
				LT_turnning_point_ibias = Ibias_init_tmp - Ibias_slop2*(norm_temp-LT_turnning_point);
				HT_turnning_point_ibias = NT_turnning_point_ibias + Ibias_slop3*(HT_turnning_point - NT_turnning_point);

			}
			else if ( NT_turnning_point == norm_temp)
			{
				NT_turnning_point_ibias = Ibias_init_tmp ;
				LT_turnning_point_ibias = NT_turnning_point_ibias - Ibias_slop2*(NT_turnning_point - LT_turnning_point);
				HT_turnning_point_ibias = NT_turnning_point_ibias + Ibias_slop3*(HT_turnning_point - NT_turnning_point);

			}
			else if ( HT_turnning_point == norm_temp)
			{
				HT_turnning_point_ibias = Ibias_init_tmp;
				NT_turnning_point_ibias = Ibias_init_tmp - Ibias_slop3*(HT_turnning_point - NT_turnning_point);
				LT_turnning_point_ibias = NT_turnning_point_ibias - Ibias_slop2*(NT_turnning_point-LT_turnning_point);
			}
			else if ( norm_temp > HT_turnning_point)
			{
				HT_turnning_point_ibias = Ibias_init_tmp - Ibias_slop4*(norm_temp - HT_turnning_point);
				NT_turnning_point_ibias = Ibias_init_tmp - Ibias_slop3*(HT_turnning_point - NT_turnning_point);
				LT_turnning_point_ibias = NT_turnning_point_ibias - Ibias_slop2*(NT_turnning_point-LT_turnning_point);
			}
			else if ( norm_temp <= LT_turnning_point)
			{
				printk("Calibration tempature too lower , Wrong setting \n");
				return;
			}

			if((int)IC_temperature >= HT_turnning_point )
			{
				Ibias_set = HT_turnning_point_ibias + Ibias_slop4*(int)((int)IC_temperature - HT_turnning_point);
				#if 0
				printk("Slope4\n");
				#endif
			}
			else if(((int)IC_temperature >= NT_turnning_point ) & ((int)IC_temperature < HT_turnning_point))
			{
				Ibias_set = NT_turnning_point_ibias + Ibias_slop3*(int)((int)IC_temperature - NT_turnning_point);
				#if 0
				printk("Slope3\n");
				#endif
			}
			else if(((int)IC_temperature < NT_turnning_point ) & ((int)IC_temperature >= LT_turnning_point))
			{
				Ibias_set = LT_turnning_point_ibias + Ibias_slop2*(int)((int)IC_temperature - LT_turnning_point);
				#if 0
				printk("Slope2\n");
				#endif
			}
			else if ((int)IC_temperature < LT_turnning_point )
			{
				if((int)IC_temperature <0 )
				{
					Ibias_set = LT_turnning_point_ibias - Ibias_slop1 * ((abs((int)IC_temperature)) + LT_turnning_point);
				}
				else
				{
					Ibias_set = LT_turnning_point_ibias + Ibias_slop1 * ((int)IC_temperature - LT_turnning_point);
				}
				#if 0
				printk("Slope1\n");
				#endif
			}

			#if 0
			printk("HT_turnning_point_ibias = %x\n",HT_turnning_point_ibias );
			printk("NT_turnning_point_ibias = %x\n",NT_turnning_point_ibias );
			printk("LT_turnning_point_ibias = %x\n",LT_turnning_point_ibias );
			printk("Ibias_set = %x\n",Ibias_set );
			#endif

			//P0 switch to open loop mode
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 4);
			ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_open_loop_mode;
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3 , ptr, 4);

			if (Ibias_set < 0x25)
				Ibias_set = 0x25;

			//Read Ibias initial value
			read_data = Ibias_set;
			ptr[0] = read_data;
			ptr[1] = (read_data >> 8);
			phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS2 , ptr, 2);

			#if 1
			//Bias current
			temp = (float)global_bias_current*bias_current_slope;	//mA
			a = (int)temp;
			b = (int)(temp*100)%100;
			printk("SOL mode, Ibias = %d.%.2dmA", a, b);

			//Tempature
			printk("\t");
			a = (int)IC_temperature;
			b = abs( (int)(IC_temperature*10)%10 );
			printk("IC temperature = %d.%dC\n", a, b);
			#endif
	}
}
/*****************************************************************************
//Function :
//		mt7570_FiberPlug_Protect
//Description :
//		This function is to avoid Ibias variating too much by restarting ERC.
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/11/06 by HC
//
******************************************************************************/
void mt7570_FiberPlug_Protect(void)
{
	mt7570_restart_ERC();
}


/*****************************************************************************
//Function :
//		mt7570_restart_ERC_P0
//Description :
//		This function is to restart P0 ERC
//Input :
//		N/A
//Output :
//		N/A
//Date:
//		2015/02/10 by HC
//
******************************************************************************/
void mt7570_restart_ERC_P0(void)
{
		unchar ptr[4];

		//Close P0 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = ptr[0] & ERC_start_mask;
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4 );
		//Start P0 calibration
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4);
		ptr[0] = (ptr[0] & ERC_start_mask) | (ERC_start);
		phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 4 );
		PON_PHY_MSG(PHY_MSG_DBG, "P0 ERC restarted\n");
}


/*****************************************************************************
//Function :
//		mt7570_BiasTracking
//Description :
//		This function is to fix Tx eye by track bias current
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/09/21 by HC
******************************************************************************/
void mt7570_BiasTracking(void)
{
	unchar ptr[4];
	uint read_data 				= 0;
	uint MPDL 						= 0;
	uint Ibias_now 				= 0;

	int temperature_index 	= 0;
	int a 							= 0;
	int b 							= 0;

	float temp 					= 0;


	//Calculate temperature_index
	temperature_index = (int)( (Env_temperature+40)/2.5 ); 						//In order to find the relative I_bias and I_mod in LUT_Ibias_Imod
	if( temperature_index < 0 ) 															//temperature_index should be >= 0
		temperature_index = 0;

	// Load target Ibias and write into 0x138
	read_data = LUT_Ibias_Imod[temperature_index][0];

	if( read_data == 0xfff )																	// No availible target bias current
		return;

	ptr[0] = read_data;
	ptr[1] = (read_data >> 8);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS2, ptr, 2 );

	PON_PHY_MSG(PHY_MSG_DBG, "Target bias loaded, 0x%x\n", read_data);
	temp = (float)read_data*bias_current_slope;										//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	PON_PHY_MSG(PHY_MSG_DBG, "Target Ibias = %d.%.2dmA\n", a, b);

	Ibias_now = global_bias_current;


	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 1);
	if( (ptr[0]&0x0f) == 0x5 )																	// In dual-closed loop mode by HC 20151123
	{

		if ( read_data > Ibias_now )															// Target bias current > bias current
		{
			if ( read_data - Ibias_now > 0x14 )												// Target bias current - bias current > 0.5 mA
			{
				MPDL = mt7570_information_output(P0) + 0x4;							// P0 + 0x4
				ptr[0] = MPDL;
				ptr[1] = MPDL >> 8;
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2, ptr, 2);
				PON_PHY_MSG(PHY_MSG_DBG, "Bias tracking done\n");
			}
		}
		else if ( read_data < Ibias_now )													// Target bias current < bias current
		{
			if( Ibias_now - read_data > 0x14 )											// Bias current - target bias current > 0.5 mA
			{
				MPDL = mt7570_information_output(P0) - 0x4; 							// P0 - 0x4
				ptr[0] = MPDL;
				ptr[1] = MPDL >> 8;
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_MPDH + 0x2, ptr, 2);
				PON_PHY_MSG(PHY_MSG_DBG, "Bias tracking done\n");
			}
		}

	}
	#if 0
	else
		printk("BiasTracking is not in dual-closed loop mode\n");
	#endif

}



/*****************************************************************************
//Function :
//		mt7570_LUT_recover
//Description :
//		This function is to recover the LUT of bias/modulation currents
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/10/12 by HC
******************************************************************************/
void mt7570_LUT_recover(void)
{
	uint Init_Ibias				= 0;
	uint FLASH_Ibias			= 0;
	uint FLASH_Ibias_up 		= 0;
	uint FLASH_Imod				= 0;
	uint FLASH_Imod_up 		= 0;
	uint Ibias_offset			= 0;
	uint Ibias_recover			= 0;
	uint Imod_recover			= 0;
	uint Ibias_cal				= 0;

	int FLASH_index			= 0;
	int index					= 0;
	float temperature_cal	= 25.0;
	float temperature			= 0;


	if( DOL == 1 )														// For open loop mode, by HC 20160410
	{
		for( index = 0; index < 64; index ++ )
		{
			temperature = (float)index*2.5 - 40;
			FLASH_index = (int)( (temperature+40)/10 );		// In order to find the relative bias/mod from FLASH

			if( FLASH_index < 0 ) 									// FLASH_index should be >= 0
				FLASH_index = 0;
			else if( FLASH_index >15 )								// FLASH_index should be <= 15
				FLASH_index = 15;

			//Read Ibias initial value
			FLASH_Ibias =(get_flash_register(0xa0+ FLASH_index*4    ) & 0x0fff0000) >> 16;
			FLASH_Ibias_up =(get_flash_register(0xa0+(FLASH_index+1)*4) & 0x0fff0000) >> 16;
			FLASH_Imod =(get_flash_register(0xa0+ FLASH_index*4    ) & 0x00000fff);
			FLASH_Imod_up =(get_flash_register(0xa0+(FLASH_index+1)*4) & 0x00000fff);

			// Recover Ibias
			if( FLASH_Ibias == 0xfff || FLASH_Ibias_up == 0xfff )	// If one of FLASH contents is empty
			{
				LUT_Ibias_Imod[index][0] = 0xfff;
			}
			else
			{
				if(FLASH_index < 4)
					Ibias_recover = FLASH_Ibias + ((temperature +40) - (FLASH_index*10))*(FLASH_Ibias_up - FLASH_Ibias)/10;
				else
					Ibias_recover = FLASH_Ibias + (temperature - ((10*FLASH_index) -40))*(FLASH_Ibias_up - FLASH_Ibias)/10;

				LUT_Ibias_Imod[index][0] = Ibias_recover;
			}

			// Recover Imod
			if( FLASH_Imod == 0xfff || FLASH_Imod_up == 0xfff )		// If one of FLASH contents is empty
			{
				LUT_Ibias_Imod[index][1] = 0xfff;
			}
			else
			{
				if(FLASH_index < 4)
					Imod_recover = FLASH_Imod + ((temperature +40) - (FLASH_index*10))*(FLASH_Imod_up - FLASH_Imod)/10;
				else
					Imod_recover = FLASH_Imod + (temperature - ((10*FLASH_index) -40))*(FLASH_Imod_up - FLASH_Imod)/10;

				LUT_Ibias_Imod[index][1] = Imod_recover;
			}
		}

		// -40, -30,...0...90, 100,...
		for( FLASH_index=0; FLASH_index<=15; FLASH_index++ )
		{
			index = 4*FLASH_index;

			if( ((get_flash_register( 0xa0 + FLASH_index*4 ) & 0x0fff0000) >> 16 ) != 0xfff )
			{
				LUT_Ibias_Imod[index][0] = ( (get_flash_register( 0xa0 + FLASH_index*4 ) & 0x0fff0000) >> 16 );
			}

			if( (get_flash_register( 0xa0 + FLASH_index*4 ) & 0x00000fff) != 0xfff )
			{
				LUT_Ibias_Imod[index][1] = ( get_flash_register( 0xa0 + FLASH_index*4 ) & 0x00000fff );
			}
		}
	}
	else
	{
		if( get_flash_register(flash_reserved_for_ETC1) != 0xffffffff )
		{
			temperature_cal = (float)get_flash_register(flash_reserved_for_ETC1)/10;
			if( (temperature_cal<0) || (temperature_cal>40))
			{
				printk("EnvTemp error\n");
				return;
			}
		}

		FLASH_index = (int)( (temperature_cal+40)/10 );		//In order to find the Ibias at ?C from FLASH

		if (get_flash_register(0xa0+FLASH_index*4) == 0xffffffff)	// If table is empty
		{
			printk("Lack of bias-current table\n");
			return;
		}



		//Read Ibias initial value
		FLASH_Ibias =(get_flash_register(0xa0+FLASH_index*4) & 0x0fff0000) >> 16;
		FLASH_Ibias_up =(get_flash_register(0xa0+(FLASH_index+1)*4) & 0x0fff0000) >> 16;
		Ibias_cal = FLASH_Ibias + (temperature_cal - ((10*FLASH_index) -40))*(FLASH_Ibias_up - FLASH_Ibias)/10;
		// Read calibrated bias current
		Init_Ibias = get_flash_register(flash_Ibias_init);
		// Calculate bias offset
		Ibias_offset = Init_Ibias - Ibias_cal;
		printk("Bias offset = 0x%x\n", Ibias_offset);


		for( index = 0; index < 64; index ++ )
		{
			temperature = (float)index*2.5 - 40;
			FLASH_index = (int)( (temperature+40)/10 );		// In order to find the relative I_bias from FLASH

			if( FLASH_index < 0 ) 									// FLASH_index should be >= 0
				FLASH_index = 0;
			else if( FLASH_index >15 )								// FLASH_index should be <= 15
				FLASH_index = 15;

			//Read Ibias initial value
			FLASH_Ibias =(get_flash_register(0xa0+ FLASH_index*4    ) & 0x0fff0000) >> 16;
			FLASH_Ibias_up =(get_flash_register(0xa0+(FLASH_index+1)*4) & 0x0fff0000) >> 16;

			if( FLASH_Ibias == 0xfff || FLASH_Ibias_up == 0xfff )	// If one of FLASH contents is empty
			{
				LUT_Ibias_Imod[index][0] = 0xfff;
			}
			else
			{
				if(FLASH_index < 4)
					Ibias_recover = FLASH_Ibias + ((temperature +40) - (FLASH_index*10))*(FLASH_Ibias_up - FLASH_Ibias)/10;
				else
					Ibias_recover = FLASH_Ibias + (temperature - ((10*FLASH_index) -40))*(FLASH_Ibias_up - FLASH_Ibias)/10;

				LUT_Ibias_Imod[index][0] = Ibias_recover + Ibias_offset;
			}

			if( LUT_Ibias_Imod[index][0] <= 0x0 )	// Check if bias <= 0 mA
			{
				printk("LUT recover failed\n");
				return;
			}

		}

		// -40, -30,...0...90, 100,...
		for( FLASH_index=0; FLASH_index<=15; FLASH_index++ )
		{
			index = 4*FLASH_index;
			if( ((get_flash_register( 0xa0 + FLASH_index*4 ) & 0x0fff0000) >> 16 ) != 0xfff )
			{
				LUT_Ibias_Imod[index][0] = ( (get_flash_register( 0xa0 + FLASH_index*4 ) & 0x0fff0000) >> 16 ) + Ibias_offset;
			}
		}
	}

}


/*****************************************************************************
//Function :
//		mt7570_show_LUT
//Description :
//		This function is to print the LUT of bias/modulation currents
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2015/10/14 by HC
******************************************************************************/
void mt7570_show_LUT(void)
{
	int index = 0;

	printk("Temperature");
	printk("\t");
	printk("Ibias");
	printk("\t");
	printk("Imod\n");


	for( index = 0; index < 64; index ++ )
	{
		printk("%03d.%01d\t\t0x%03x\t0x%03x\n", (index*25-400)/10, abs(index*25-400)%10, LUT_Ibias_Imod[index][0], LUT_Ibias_Imod[index][1] );
	}

}


/*****************************************************************************
//Function :
//		mt7570_SingleClosedLoopMode
//Description :
//		This function is to switch to single-closed loop mode
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2016/03/25 by HC
******************************************************************************/
void mt7570_SingleClosedLoopMode(void)
{
	unchar ptr[4];

	// P0 open-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P0_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_open_loop_mode;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS3, ptr, 1);

	// P1 closed-loop mode
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_P1_PWR_CTRL_CS3, ptr, 1);
	ptr[0] = (ptr[0] & ERC_open_loop_mode_mask) | ERC_start;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS3, ptr, 1);

	printk("Switch to Single-closed Loop Mode\n");
}


/*****************************************************************************
//Function :
//		mt7570_OpenLoopControl
//Description :
//		This function is to control open loop mode.
//Input :
//		N/A
//Output :
//		N/A
//Date :
//		2016/04/10 by HC
******************************************************************************/
void mt7570_OpenLoopControl(void)
{
	unchar ptr[4];
	uint read_data 				= 0;

	int temperature_index 	= 0;
	int a 							= 0;
	int b 							= 0;

	float temp 					= 0;


	//Calculate temperature_index
	temperature_index = (int)( (Env_temperature+40)/2.5 ); 						//In order to find the relative I_bias and I_mod in LUT_Ibias_Imod
	if( temperature_index < 0 ) 															//temperature_index should be >= 0
		temperature_index = 0;

	// Load target Ibias and write into 0x138
	read_data = LUT_Ibias_Imod[temperature_index][0];

	if( read_data == 0xfff )																	// No availible target bias current
		return;

	ptr[0] = read_data;
	ptr[1] = (read_data >> 8);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P0_PWR_CTRL_CS2, ptr, 2 );

	PON_PHY_MSG(PHY_MSG_DBG, "Target bias loaded, 0x%x\n", read_data);
	temp = (float)read_data*bias_current_slope;										//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	PON_PHY_MSG(PHY_MSG_DBG, "Target Ibias = %d.%.2dmA\n", a, b);


	// Load target Ibias and write into 0x148
	read_data = LUT_Ibias_Imod[temperature_index][1];

	if( read_data == 0xfff )																	// No availible target modulation current
		return;

	ptr[0] = read_data;
	ptr[1] = (read_data >> 8);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2,  mt7570_P1_PWR_CTRL_CS2, ptr, 2 );

	PON_PHY_MSG(PHY_MSG_DBG, "Target mod loaded, 0x%x\n", read_data);
	temp = (float)read_data*mod_current_slope;										//mA
	a = (int)temp;
	b = (int)(temp*100)%100;
	PON_PHY_MSG(PHY_MSG_DBG, "Target Imod = %d.%.2dmA\n", a, b);

}


/*****************************************************************************
//function :
//		mt7570_LOS_init
//description :
//		this function is to initialize LOS function
//input :
//		N/A
//output :
//		N/A
//Date :
//		2016/04/27 by HC
******************************************************************************/
void mt7570_LOS_init(void)
{
	unchar ptr[4];

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);
	ptr[0] = (ptr[0] & (LOS_calibration_mask)) | (LOS_calibration_trig);
	// Set rg_ain_stable_cnt
	ptr[1] = (ptr[1] & (LOS_ain_stable_cnt_mask)) | (LOS_ain_stable_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL1, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[3] = (ptr[3] & (LOS_ADCREV2_mask)) | (LOS_ADCREV2_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);
	ptr[2] = (ptr[2] & (LOS_ADCREV1_mask)) | (LOS_ADCREV1_enable);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_SVADC_PD, ptr, 4);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
	ptr[1] = (ptr[1] & (LOS_confidence_mask)) | (LOS_confidence_setting);
	// Set rg_los_sd_cnt
	ptr[0] = (ptr[0] & (LOS_cnt_mask)) | (LOS_cnt_setting);
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, 0x70, 2, mt7570_LOS_CTRL2, ptr, 4);
}
