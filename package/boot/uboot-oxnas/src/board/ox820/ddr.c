/*******************************************************************
 *
 * File:            ddr_oxsemi.c
 *
 * Description:     Declarations for DDR routines and data objects
 *
 * Author:          Julien Margetts
 *
 * Copyright:       Oxford Semiconductor Ltd, 2009
 */
#include <common.h>
#include <asm/arch/clock.h>

#include "ddr.h"

typedef unsigned int UINT;

// DDR TIMING PARAMETERS
typedef struct {
	unsigned int holdoff_cmd_A;
	unsigned int holdoff_cmd_ARW;
	unsigned int holdoff_cmd_N;
	unsigned int holdoff_cmd_LM;
	unsigned int holdoff_cmd_R;
	unsigned int holdoff_cmd_W;
	unsigned int holdoff_cmd_PC;
	unsigned int holdoff_cmd_RF;
	unsigned int holdoff_bank_R;
	unsigned int holdoff_bank_W;
	unsigned int holdoff_dir_RW;
	unsigned int holdoff_dir_WR;
	unsigned int holdoff_FAW;
	unsigned int latency_CAS;
	unsigned int latency_WL;
	unsigned int recovery_WR;
	unsigned int width_update;
	unsigned int odt_offset;
	unsigned int odt_drive_all;
	unsigned int use_fixed_re;
	unsigned int delay_wr_to_re;
	unsigned int wr_slave_ratio;
	unsigned int rd_slave_ratio0;
	unsigned int rd_slave_ratio1;
} T_DDR_TIMING_PARAMETERS;

// DDR CONFIG PARAMETERS

typedef struct {
	unsigned int ddr_mode;
	unsigned int width;
	unsigned int blocs;
	unsigned int banks8;
	unsigned int rams;
	unsigned int asize;
	unsigned int speed;
	unsigned int cmd_mode_wr_cl_bl;
} T_DDR_CONFIG_PARAMETERS;

//cmd_mode_wr_cl_bl
//when SDR : cmd_mode_wr_cl_bl = 0x80200002 + (latency_CAS_RAM * 16) + (recovery_WR - 1) * 512; -- Sets write rec XX, CL=XX; BL=8
//else       cmd_mode_wr_cl_bl = 0x80200003 + (latency_CAS_RAM * 16) + (recovery_WR - 1) * 512; -- Sets write rec XX, CL=XX; BL=8

//                                                            cmd_                    bank_ dir_     lat_  rec_ width_ odt_   odt_ fix delay     ratio
//                                                                A                                F  C         update offset all  re  re_to_we  w  r0  r1
//                                                                R     L        P  R        R  W  A  A  W  W
//Timing Parameters                                            A  W  N  M  R  W  C  F  R  W  W  R  W  S  L  R
static const T_DDR_TIMING_PARAMETERS C_TP_DDR2_25E_CL5_1GB = { 4, 5, 0, 2, 4, 4,
	5, 51, 23, 24, 9, 11, 18, 5, 4, 6, 3, 2, 0, 1, 2, 75, 56, 56 }; //elida device.
static const T_DDR_TIMING_PARAMETERS C_TP_DDR2_25E_CL5_2GB = { 4, 5, 0, 2, 4, 4,
	5, 79, 22, 24, 9, 11, 20, 5, 4, 6, 3, 2, 0, 1, 2, 75, 56, 56 };
static const T_DDR_TIMING_PARAMETERS C_TP_DDR2_25_CL6_1GB = { 4, 5, 0, 2, 4, 4,
	4, 51, 22, 26, 10, 12, 18, 6, 5, 6, 3, 2, 0, 1, 2, 75, 56, 56 }; // 400MHz, Speedgrade 25 timings (1Gb parts)

//                                                          D     B  B  R  A   S
//                                                          D  W  L  K  A  S   P
//Config Parameters                                         R  D  C  8  M  Z   D CMD_MODE
//static const T_DDR_CONFIG_PARAMETERS C_CP_DDR2_25E_CL5  = { 2,16, 1, 0, 1, 32,25,0x80200A53}; // 64 MByte
static const T_DDR_CONFIG_PARAMETERS C_CP_DDR2_25E_CL5 = { 2, 16, 1, 1, 1, 64,
	25, 0x80200A53 }; // 128 MByte
static const T_DDR_CONFIG_PARAMETERS C_CP_DDR2_25_CL6 = { 2, 16, 1, 1, 1, 128,
	25, 0x80200A63 }; // 256 MByte

static void ddr_phy_poll_until_locked(void)
{
	volatile UINT reg_tmp = 0;
	volatile UINT locked = 0;

	//Extra read to put in delay before starting to poll...
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY2;      // read

	//POLL C_DDR_PHY2_REG register until clock and flock
	//!!! Ideally have a timeout on this.
	while (locked == 0) {
		reg_tmp = *(volatile UINT *) C_DDR_REG_PHY2;      // read

		//locked when bits 30 and 31 are set
		if (reg_tmp & 0xC0000000) {
			locked = 1;
		}
	}
}

static void ddr_poll_until_not_busy(void)
{
	volatile UINT reg_tmp = 0;
	volatile UINT busy = 1;

	//Extra read to put in delay before starting to poll...
	reg_tmp = *(volatile UINT *) C_DDR_STAT_REG;      // read

	//POLL DDR_STAT register until no longer busy
	//!!! Ideally have a timeout on this.
	while (busy == 1) {
		reg_tmp = *(volatile UINT *) C_DDR_STAT_REG;      // read

		//when bit 31 is clear - core is no longer busy
		if ((reg_tmp & 0x80000000) == 0x00000000) {
			busy = 0;
		}
	}
}

static void ddr_issue_command(int commmand)
{
	*(volatile UINT *) C_DDR_CMD_REG = commmand;
	ddr_poll_until_not_busy();
}

static void ddr_timing_initialisation(
	const T_DDR_TIMING_PARAMETERS *ddr_timing_parameters)
{
	volatile UINT reg_tmp = 0;
	/* update the DDR controller registers for timing parameters */
	reg_tmp = (ddr_timing_parameters->holdoff_cmd_A << 0);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_ARW << 4);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_N << 8);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_LM << 12);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_R << 16);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_W << 20);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_cmd_PC << 24);
	*(volatile UINT *) C_DDR_REG_TIMING0 = reg_tmp;

	reg_tmp = (ddr_timing_parameters->holdoff_cmd_RF << 0);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_bank_R << 8);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_bank_W << 16);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_dir_RW << 24);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_dir_WR << 28);
	*(volatile UINT *) C_DDR_REG_TIMING1 = reg_tmp;

	reg_tmp = (ddr_timing_parameters->latency_CAS << 0);
	reg_tmp = reg_tmp + (ddr_timing_parameters->latency_WL << 4);
	reg_tmp = reg_tmp + (ddr_timing_parameters->holdoff_FAW << 8);
	reg_tmp = reg_tmp + (ddr_timing_parameters->width_update << 16);
	reg_tmp = reg_tmp + (ddr_timing_parameters->odt_offset << 21);
	reg_tmp = reg_tmp + (ddr_timing_parameters->odt_drive_all << 24);

	*(volatile UINT *) C_DDR_REG_TIMING2 = reg_tmp;

	/* Program the timing parameters in the PHY too */
	reg_tmp = (ddr_timing_parameters->use_fixed_re << 16)
			| (ddr_timing_parameters->delay_wr_to_re << 8)
			| (ddr_timing_parameters->latency_WL << 4)
			| (ddr_timing_parameters->latency_CAS << 0);

	*(volatile UINT *) C_DDR_REG_PHY_TIMING = reg_tmp;

	reg_tmp = ddr_timing_parameters->wr_slave_ratio;

	*(volatile UINT *) C_DDR_REG_PHY_WR_RATIO = reg_tmp;

	reg_tmp = ddr_timing_parameters->rd_slave_ratio0;
	reg_tmp += ddr_timing_parameters->rd_slave_ratio1 << 8;

	*(volatile UINT *) C_DDR_REG_PHY_RD_RATIO = reg_tmp;

}

static void ddr_normal_initialisation(
	const T_DDR_CONFIG_PARAMETERS *ddr_config_parameters, int mhz)
{
	int i;
	volatile UINT tmp = 0;
	volatile UINT reg_tmp = 0;
	volatile UINT emr_cmd = 0;
	UINT refresh;

	//Total size of memory in Mbits...
	tmp = ddr_config_parameters->rams * ddr_config_parameters->asize
		* ddr_config_parameters->width;
	//Deduce value to program into DDR_CFG register...
	switch (tmp) {
	case 16:
		reg_tmp = 0x00020000 * 1;
		break;
	case 32:
		reg_tmp = 0x00020000 * 2;
		break;
	case 64:
		reg_tmp = 0x00020000 * 3;
		break;
	case 128:
		reg_tmp = 0x00020000 * 4;
		break;
	case 256:
		reg_tmp = 0x00020000 * 5;
		break;
	case 512:
		reg_tmp = 0x00020000 * 6;
		break;
	case 1024:
		reg_tmp = 0x00020000 * 7;
		break;
	case 2048:
		reg_tmp = 0x00020000 * 8;
		break;
	default:
		reg_tmp = 0; //forces sims not to work if badly configured
	}

	//Memory width
	tmp = ddr_config_parameters->rams * ddr_config_parameters->width;
	switch (tmp) {
	case 8:
		reg_tmp = reg_tmp + 0x00400000;
		break;
	case 16:
		reg_tmp = reg_tmp + 0x00200000;
		break;
	case 32:
		reg_tmp = reg_tmp + 0x00000000;
		break;
	default:
		reg_tmp = 0; //forces sims not to work if badly configured
	}

	//Setup DDR Mode
	switch (ddr_config_parameters->ddr_mode) {
	case 0:
		reg_tmp = reg_tmp + 0x00000000;
		break;   //SDR
	case 1:
		reg_tmp = reg_tmp + 0x40000000;
		break;   //DDR
	case 2:
		reg_tmp = reg_tmp + 0x80000000;
		break;   //DDR2
	default:
		reg_tmp = 0; //forces sims not to work if badly configured
	}

	//Setup Banks
	if (ddr_config_parameters->banks8 == 1) {
		reg_tmp = reg_tmp + 0x00800000;
	}

	//Program DDR_CFG register...
	*(volatile UINT *) C_DDR_CFG_REG = reg_tmp;

	//Configure PHY0 reg - se_mode is bit 1,
	//needs to be 1 for DDR (single_ended drive)
	switch (ddr_config_parameters->ddr_mode) {
	case 0:
		reg_tmp = 2 + (0 << 4);
		break;   //SDR
	case 1:
		reg_tmp = 2 + (4 << 4);
		break;   //DDR
	case 2:
		reg_tmp = 0 + (4 << 4);
		break;   //DDR2
	default:
		reg_tmp = 0;
	}

	//Program DDR_PHY0 register...
	*(volatile UINT *) C_DDR_REG_PHY0 = reg_tmp;

	//Read DDR_PHY* registers to exercise paths for vcd
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY3;
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY2;
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY1;
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY0;

	//Start up sequences - Different dependant on DDR mode
	switch (ddr_config_parameters->ddr_mode) {
	case 2:   //DDR2
		//Start-up sequence: follows procedure described in Micron datasheet.
		//start up DDR PHY DLL
		reg_tmp = 0x00022828;       // dll on, start point and inc = h28
		*(volatile UINT *) C_DDR_REG_PHY2 = reg_tmp;

		reg_tmp = 0x00032828; // start on, dll on, start point and inc = h28
		*(volatile UINT *) C_DDR_REG_PHY2 = reg_tmp;

		ddr_phy_poll_until_locked();

		udelay(200);   //200us

		//Startup SDRAM...
		//!!! Software: CK should be running for 200us before wake-up
		ddr_issue_command( C_CMD_WAKE_UP);
		ddr_issue_command( C_CMD_NOP);
		ddr_issue_command( C_CMD_PRECHARGE_ALL);
		ddr_issue_command( C_CMD_DDR2_EMR2);
		ddr_issue_command( C_CMD_DDR2_EMR3);

		emr_cmd = C_CMD_DDR2_EMR1 + C_CMD_ODT_75 + C_CMD_REDUCED_DRIVE
			+ C_CMD_ENABLE_DLL;

		ddr_issue_command(emr_cmd);
		//Sets CL=3; BL=8 but also reset DLL to trigger a DLL initialisation...
		udelay(1);   //1us
		ddr_issue_command(
			ddr_config_parameters->cmd_mode_wr_cl_bl
			+ C_CMD_RESET_DLL);
		udelay(1);   //1us

		//!!! Software: Wait 200 CK cycles before...
		//for(i=1; i<=2; i++) {
		ddr_issue_command(C_CMD_PRECHARGE_ALL);
		// !!! Software: Wait here at least 8 CK cycles
		//}
		//need a wait here to ensure PHY DLL lock before the refresh is issued
		udelay(1);   //1us
		for (i = 1; i <= 2; i++) {
			ddr_issue_command( C_CMD_AUTO_REFRESH);
			//!!! Software: Wait here at least 8 CK cycles to satify tRFC
			udelay(1);   //1us
		}
		//As before but without 'RESET_DLL' bit set...
		ddr_issue_command(ddr_config_parameters->cmd_mode_wr_cl_bl);
		udelay(1);   //1us
		// OCD commands
		ddr_issue_command(emr_cmd + C_CMD_MODE_DDR2_OCD_DFLT);
		ddr_issue_command(emr_cmd + C_CMD_MODE_DDR2_OCD_EXIT);
		break;

	default:
		break;  //Do nothing
	}

	//Enable auto-refresh

	// 8192 Refreshes required every 64ms, so maximum refresh period is 7.8125 us
	// We have a 400 MHz DDR clock (2.5ns period) so max period is 3125 cycles
	// Our core now does 8 refreshes in a go, so we multiply this period by 8

	refresh = (64000 * mhz) / 8192; // Refresh period in clocks

	reg_tmp = *(volatile UINT *) C_DDR_CFG_REG;      // read
#ifdef BURST_REFRESH_ENABLE
	reg_tmp |= C_CFG_REFRESH_ENABLE | (refresh * 8);
	reg_tmp |= C_CFG_BURST_REFRESH_ENABLE;
#else
	reg_tmp |= C_CFG_REFRESH_ENABLE | (refresh * 1);
	reg_tmp &= ~C_CFG_BURST_REFRESH_ENABLE;
#endif
	*(volatile UINT *) C_DDR_CFG_REG = reg_tmp;

	//Verify register contents
	reg_tmp = *(volatile UINT *) C_DDR_REG_PHY2;      // read
	//printf("Warning XXXXXXXXXXXXXXXXXXXXXX - get bad read data from C_DDR_PHY2_REG, though it looks OK on bus XXXXXXXXXXXXXXXXXX");
	//TBD   Check_data (read_data,  dll_reg, "Error: bad C_DDR_PHY2_REG read", tb_pass);
	reg_tmp = *(volatile UINT *) C_DDR_CFG_REG;      // read
	//TBD   Check_data (read_data,  cfg_reg, "Error: bad DDR_CFG read", tb_pass);

	//disable optimised wrapping
	if (ddr_config_parameters->ddr_mode == 2) {
		reg_tmp = 0xFFFF0000;
		*(volatile UINT *) C_DDR_REG_IGNORE = reg_tmp;
	}

	//enable midbuffer followon
	reg_tmp = *(volatile UINT *) C_DDR_ARB_REG;      // read
	reg_tmp = 0xFFFF0000 | reg_tmp;
	*(volatile UINT *) C_DDR_ARB_REG = reg_tmp;

	// Enable write behind coherency checking for all clients

	reg_tmp = 0xFFFF0000;
	*(volatile UINT *) C_DDR_AHB4_REG = reg_tmp;

	//Wait for 200 clock cycles for SDRAM DLL to lock...
	udelay(1);   //1us
}

// Function used to Setup DDR core

void ddr_setup(int mhz)
{
	static const T_DDR_TIMING_PARAMETERS *ddr_timing_parameters =
		&C_TP_DDR2_25_CL6_1GB;
	static const T_DDR_CONFIG_PARAMETERS *ddr_config_parameters =
		&C_CP_DDR2_25_CL6;

	//Bring core out of Reset
	*(volatile UINT *) C_DDR_BLKEN_REG = C_BLKEN_DDR_ON;

	//DDR TIMING INITIALISTION
	ddr_timing_initialisation(ddr_timing_parameters);

	//DDR NORMAL INITIALISATION
	ddr_normal_initialisation(ddr_config_parameters, mhz);

	// route all writes through one client
	*(volatile UINT *) C_DDR_TRANSACTION_ROUTING = (0
		<< DDR_ROUTE_CPU0_INSTR_SHIFT)
		| (1 << DDR_ROUTE_CPU0_RDDATA_SHIFT)
		| (3 << DDR_ROUTE_CPU0_WRDATA_SHIFT)
		| (2 << DDR_ROUTE_CPU1_INSTR_SHIFT)
		| (3 << DDR_ROUTE_CPU1_RDDATA_SHIFT)
		| (3 << DDR_ROUTE_CPU1_WRDATA_SHIFT);

	//Bring all clients out of reset
	*(volatile UINT *) C_DDR_BLKEN_REG = C_BLKEN_DDR_ON + 0x0000FFFF;

}

void set_ddr_timing(unsigned int w, unsigned int i)
{
	unsigned int reg;
	unsigned int wnow = 16;
	unsigned int inow = 32;

	/* reset all timing controls to known value (31) */
	writel(DDR_PHY_TIMING_W_RST | DDR_PHY_TIMING_I_RST, DDR_PHY_TIMING);
	writel(DDR_PHY_TIMING_W_RST | DDR_PHY_TIMING_I_RST | DDR_PHY_TIMING_CK,
	       DDR_PHY_TIMING);
	writel(DDR_PHY_TIMING_W_RST | DDR_PHY_TIMING_I_RST, DDR_PHY_TIMING);

	/* step up or down read delay to the requested value */
	while (wnow != w) {
		if (wnow < w) {
			reg = DDR_PHY_TIMING_INC;
			wnow++;
		} else {
			reg = 0;
			wnow--;
		}
		writel(DDR_PHY_TIMING_W_CE | reg, DDR_PHY_TIMING);
		writel(DDR_PHY_TIMING_CK | DDR_PHY_TIMING_W_CE | reg,
		       DDR_PHY_TIMING);
		writel(DDR_PHY_TIMING_W_CE | reg, DDR_PHY_TIMING);
	}

	/* now write delay */
	while (inow != i) {
		if (inow < i) {
			reg = DDR_PHY_TIMING_INC;
			inow++;
		} else {
			reg = 0;
			inow--;
		}
		writel(DDR_PHY_TIMING_I_CE | reg, DDR_PHY_TIMING);
		writel(DDR_PHY_TIMING_CK | DDR_PHY_TIMING_I_CE | reg,
		       DDR_PHY_TIMING);
		writel(DDR_PHY_TIMING_I_CE | reg, DDR_PHY_TIMING);
	}
}

//Function used to Setup SDRAM in DDR/SDR mode
void init_ddr(int mhz)
{
	/* start clocks */
	enable_clock(SYS_CTRL_CLK_DDRPHY);
	enable_clock(SYS_CTRL_CLK_DDR);
	enable_clock(SYS_CTRL_CLK_DDRCK);

	/* bring phy and core out of reset */
	reset_block(SYS_CTRL_RST_DDR_PHY, 0);
	reset_block(SYS_CTRL_RST_DDR, 0);

	/* DDR runs at half the speed of the CPU */
	ddr_setup(mhz >> 1);
	return;
}
