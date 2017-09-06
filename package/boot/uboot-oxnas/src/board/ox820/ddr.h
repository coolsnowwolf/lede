/*******************************************************************
*
* File:			ddr_oxsemi.h
*
* Description:		Declarations for DDR routines and data objects
*
* Author:		Julien Margetts
*
* Copyright:		Oxford Semiconductor Ltd, 2009
*/

void ddr_oxsemi_setup(int mhz);

/* define to refresh in bursts of 8 */
#define BURST_REFRESH_ENABLE

#define DDR_BASE			0x44700000

#define C_DDR_CFG_REG			(DDR_BASE + 0x00)
#define C_CFG_DDR			0x80000000
#define C_CFG_SDR			0x00000000
#define C_CFG_WIDTH8			0x00200000
#define C_CFG_WIDTH16			0x00100000
#define C_CFG_WIDTH32			0x00000000
#define C_CFG_SIZE_FACTOR		0x00020000
#define C_CFG_REFRESH_ENABLE		0x00010000
#define C_CFG_BURST_REFRESH_ENABLE	0x01000000
#define C_CFG_SIZE(x)			(x << 17)
#define CFG_SIZE_2MB			1
#define CFG_SIZE_4MB			2
#define CFG_SIZE_8MB			3
#define CFG_SIZE_16MB			4
#define CFG_SIZE_32MB			5
#define CFG_SIZE_64MB			6
#define CFG_SIZE_128MB			7

#define C_DDR_BLKEN_REG			(DDR_BASE + 0x04)
#define C_BLKEN_DDR_ON			0x80000000

#define C_DDR_STAT_REG			(DDR_BASE + 0x08)

#define C_DDR_CMD_REG			(DDR_BASE + 0x0C)
#define C_CMD_SEND_COMMAND		(1UL << 31) | (1 << 21) // RAS/CAS/WE/CS all low(active), CKE High, indicates
#define C_CMD_WAKE_UP			0x80FC0000 // Asserts CKE
#define C_CMD_MODE_SDR			0x80200022 // Sets CL=2 BL=4
#define C_CMD_MODE_DDR			0x80200063 // Sets CL=2.5 BL=8
#define C_CMD_RESET_DLL			0x00000100 // A8=1 Use in conjunction with C_CMD_MODE_DDR
#define C_CMD_PRECHARGE_ALL		0x80280400
#define C_CMD_AUTO_REFRESH		0x80240000
#define C_CMD_SELF_REFRESH		0x80040000 // As AUTO-REFRESH but with CKE low
#define C_CMD_NOP			0x803C0000 // NOP just to insert guaranteed delay
#define C_CMD_DDR2_EMR1			0x80210000 // Load extended mode register 1 with zeros (for init), CKE still set
//#define C_CMD_DDR2_EMR1		0x80210400 // Load extended mode register 1 with zeros (for init), CKE still set
#define C_CMD_ENABLE_DLL		0x00000000 // Values used in conjuction with C_CMD_DDR2_EMR1
#define C_CMD_DISABLE_DLL		0x00000001
#define C_CMD_REDUCED_DRIVE		0x00000002
#define C_CMD_ODT_DISABLED		0x00000000
#define C_CMD_ODT_50			0x00000044
#define C_CMD_ODT_75			0x00000004
#define C_CMD_ODT_150			0x00000040
#define C_CMD_MODE_DDR2_OCD_DFLT	0x00000380
#define C_CMD_MODE_DDR2_OCD_EXIT	0x00000000

#define C_CMD_DDR2_EMR2			0x80220000 // Load extended mode register 2 with zeros (for init), CKE still set
#define C_CMD_DDR2_EMR3			0x80230000 // Load extended mode register 3 with zeros (for init), CKE still set

#define C_DDR_AHB_REG			(DDR_BASE + 0x10)
#define C_AHB_NO_RCACHES		0xFFFF0000
#define C_AHB_FLUSH_ALL_RCACHES		0x0000FFFF
#define C_AHB_FLUSH_AHB0_RCACHE		0x00000001
#define C_AHB_FLUSH_AHB1_RCACHE		0x00000002

#define C_DDR_DLL_REG			(DDR_BASE + 0x14)
#define C_DLL_DISABLED			0x00000000
#define C_DLL_MANUAL			0x80000000
#define C_DLL_AUTO_OFFSET		0xA0000000
#define C_DLL_AUTO_IN_REFRESH		0xC0000000
#define C_DLL_AUTOMATIC			0xE0000000

#define C_DDR_MON_REG			(DDR_BASE + 0x18)
#define C_MON_ALL			0x00000010
#define C_MON_CLIENT			0x00000000

#define C_DDR_DIAG_REG			(DDR_BASE + 0x1C)
#define C_DDR_DIAG2_REG			(DDR_BASE + 0x20)

#define C_DDR_IOC_REG			(DDR_BASE + 0x24)
#define C_DDR_IOC_PWR_DWN		(1 << 10)
#define C_DDR_IOC_SEL_SSTL		(1 << 9)
#define C_DDR_IOC_CK_DRIVE(x)		((x) << 6)
#define C_DDR_IOC_DQ_DRIVE(x)		((x) << 3)
#define C_DDR_IOC_XX_DRIVE(x)		((x) << 0)

#define C_DDR_ARB_REG			(DDR_BASE + 0x28)
#define C_DDR_ARB_MIDBUF		(1 << 4)
#define C_DDR_ARB_LRUBANK		(1 << 3)
#define C_DDR_ARB_REQAGE		(1 << 2)
#define C_DDR_ARB_DATDIR		(1 << 1)
#define C_DDR_ARB_DATDIR_NC		(1 << 0)

#define C_TOP_ADDRESS_BIT_TEST		22
#define C_MEM_BASE			C_SDRAM_BASE

#define C_MEM_TEST_BASE			0
#define C_MEM_TEST_LEN			1920
#define C_MAX_RAND_ACCESS_LEN		16

#define C_DDR_REG_IGNORE		(DDR_BASE + 0x2C)
#define C_DDR_AHB4_REG			(DDR_BASE + 0x44)

#define C_DDR_REG_TIMING0		(DDR_BASE + 0x34)
#define C_DDR_REG_TIMING1		(DDR_BASE + 0x38)
#define C_DDR_REG_TIMING2		(DDR_BASE + 0x3C)

#define C_DDR_REG_PHY0			(DDR_BASE + 0x48)
#define C_DDR_REG_PHY1			(DDR_BASE + 0x4C)
#define C_DDR_REG_PHY2			(DDR_BASE + 0x50)
#define C_DDR_REG_PHY3			(DDR_BASE + 0x54)

#define C_DDR_REG_GENERIC		(DDR_BASE + 0x60)

#define C_OXSEMI_DDRC_SIGNATURE		0x054415AA

#define DDR_PHY_BASE			(DDR_BASE + 0x80000)
#define DDR_PHY_TIMING			(DDR_PHY_BASE + 0x48)
#define DDR_PHY_TIMING_CK		(1 << 12)
#define DDR_PHY_TIMING_INC		(1 << 13)
#define DDR_PHY_TIMING_W_CE		(1 << 14)
#define DDR_PHY_TIMING_W_RST		(1 << 15)
#define DDR_PHY_TIMING_I_CE		(1 << 16)
#define DDR_PHY_TIMING_I_RST		(1 << 17)

#define C_DDR_REG_PHY_TIMING		(DDR_PHY_BASE + 0x50)
#define C_DDR_REG_PHY_WR_RATIO		(DDR_PHY_BASE + 0x74)
#define C_DDR_REG_PHY_RD_RATIO		(DDR_PHY_BASE + 0x78)

#define C_DDR_TRANSACTION_ROUTING	(DDR_PHY_BASE + 0xC8)
#define DDR_ROUTE_CPU0_INSTR_SHIFT	0
#define DDR_ROUTE_CPU0_RDDATA_SHIFT	4
#define DDR_ROUTE_CPU0_WRDATA_SHIFT	6
#define DDR_ROUTE_CPU1_INSTR_SHIFT	8
#define DDR_ROUTE_CPU1_RDDATA_SHIFT	12
#define DDR_ROUTE_CPU1_WRDATA_SHIFT	14

unsigned int ddrc_signature(void);
void set_ddr_timing(unsigned int w, unsigned int i);
int pause(unsigned int us);
void set_ddr_sel(int val);
