#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/time.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/proc_fs.h>

#include "phy.h"
#include "phy_def.h"

#include "phy_init.h"
#include "phy_debug.h"
#include "phy.h"
#include "phy_reg.h"

#include "a60972.h"
#include "a60972_reg.h"

#ifdef TCSUPPORT_CPU_EN7581
#include "en7581.h"
#include "en7581_pma.h"
#include "en7581_reg.h"
#else
#include "en7580.h"
#include "en7580_pma.h"
#include "en7580_reg.h"	
#endif


#if A60972_SERDES  //mark A60972 FPGA code

// -- defines ------------------------------------------------------------------

#define PHYA_REG_READ_WRITE_TEST  0
#define PHYA_REG_ALL_ZEROS        0x0
#define PHYA_REG_ALL_ONES         0xFFFFFFFF

// -- typedefs -----------------------------------------------------------------


// -- external reference functions--------------------------------------------------


// -- local forward declarations ---------------------------------------------------
A60972_XPON_PMA_Param_T xpon_pma_param;

int a60972_excute_flag=0;

// --  variables ----------------------------------------------------------------

static PHY_REG_ENTRY a60972_reg_all[]={
	
	// id	,	name											   ,  addr										   
	{ 0   ,"PMA_TOP_PHYD_TX_CLK0"						 ,PMA_TOP_PHYD_TX_CLK0						  , 0 , 0},
	{ 1   ,"PMA_TOP_PHYD_TX_CLK1"						 ,PMA_TOP_PHYD_TX_CLK1						  , 0 , 0},
	{ 2   ,"PMA_TOP_PHYD_TX_CLK2"						 ,PMA_TOP_PHYD_TX_CLK2						  , 0 , 0},
	{ 3   ,"PMA_TOP_PHYD_TX_CLK3"						 ,PMA_TOP_PHYD_TX_CLK3						  , 0 , 0},
	{ 4   ,"PMA_TOP_PHYD_TX_D0" 						 ,PMA_TOP_PHYD_TX_D0						  , 0 , 0},
	{ 5   ,"PMA_TOP_PHYD_TX_D1" 						 ,PMA_TOP_PHYD_TX_D1						  , 0 , 0},
	{ 6   ,"PMA_TOP_PHYD_TX_D2" 						 ,PMA_TOP_PHYD_TX_D2						  , 0 , 0},
	{ 7   ,"PMA_TOP_PHYD_TX_D3" 						 ,PMA_TOP_PHYD_TX_D3						  , 0 , 0},
	{ 8   ,"PMA_TOP_PHYD_TX_D4" 						 ,PMA_TOP_PHYD_TX_D4						  , 0 , 0},
	{ 9   ,"PMA_TOP_PHYD_TX_D5" 						 ,PMA_TOP_PHYD_TX_D5						  , 0 , 0},
	{ 10  ,"PMA_TOP_PHYD_TX_D6" 						 ,PMA_TOP_PHYD_TX_D6						  , 0 , 0},
	{ 11  ,"PMA_TOP_PHYD_TX_D7" 						 ,PMA_TOP_PHYD_TX_D7						  , 0 , 0},
	{ 12  ,"PMA_TOP_PHYD_TX_D8" 						 ,PMA_TOP_PHYD_TX_D8						  , 0 , 0},
	{ 13  ,"PMA_TOP_PHYD_TX_D9" 						 ,PMA_TOP_PHYD_TX_D9						  , 0 , 0},
	{ 14  ,"PMA_TOP_PHYD_TX_D10"						 ,PMA_TOP_PHYD_TX_D10						  , 0 , 0},
	{ 15  ,"PMA_TOP_PHYD_TX_D11"						 ,PMA_TOP_PHYD_TX_D11						  , 0 , 0},
	{ 16  ,"PMA_TOP_PHYD_TX_D12"						 ,PMA_TOP_PHYD_TX_D12						  , 0 , 0},
	{ 17  ,"PMA_TOP_PHYD_TX_D13"						 ,PMA_TOP_PHYD_TX_D13						  , 0 , 0},
	{ 18  ,"PMA_TOP_PHYD_TX_D14"						 ,PMA_TOP_PHYD_TX_D14						  , 0 , 0},
	{ 19  ,"PMA_TOP_PHYD_TX_D15"						 ,PMA_TOP_PHYD_TX_D15						  , 0 , 0},
	{ 20  ,"PMA_TOP_PHYD_TX_D16"						 ,PMA_TOP_PHYD_TX_D16						  , 0 , 0},
	{ 21  ,"PMA_TOP_PHYD_TX_D17"						 ,PMA_TOP_PHYD_TX_D17						  , 0 , 0},
	{ 22  ,"PMA_TOP_PHYD_TX_D18"						 ,PMA_TOP_PHYD_TX_D18						  , 0 , 0},
	{ 23  ,"PMA_TOP_PHYD_TX_D19"						 ,PMA_TOP_PHYD_TX_D19						  , 0 , 0},
	{ 24  ,"PMA_TOP_PHYD_TX_D20"						 ,PMA_TOP_PHYD_TX_D20						  , 0 , 0},
	{ 25  ,"PMA_TOP_PHYD_TX_D21"						 ,PMA_TOP_PHYD_TX_D21						  , 0 , 0},
	{ 26  ,"PMA_TOP_PHYD_TX_D22"						 ,PMA_TOP_PHYD_TX_D22						  , 0 , 0},
	{ 27  ,"PMA_TOP_PHYD_TX_D23"						 ,PMA_TOP_PHYD_TX_D23						  , 0 , 0},
	{ 28  ,"PMA_TOP_PHYD_TX_D24"						 ,PMA_TOP_PHYD_TX_D24						  , 0 , 0},
	{ 29  ,"PMA_TOP_PHYD_TX_D25"						 ,PMA_TOP_PHYD_TX_D25						  , 0 , 0},
	{ 30  ,"PMA_TOP_PHYD_TX_EN0"						 ,PMA_TOP_PHYD_TX_EN0						  , 0 , 0},
	{ 31  ,"PMA_TOP_PHYD_TX_EN1"						 ,PMA_TOP_PHYD_TX_EN1						  , 0 , 0},
	{ 32  ,"PMA_TOP_PHYD_TX_EN2"						 ,PMA_TOP_PHYD_TX_EN2						  , 0 , 0},
	{ 33  ,"PMA_TOP_PHYD_TX_EN3"						 ,PMA_TOP_PHYD_TX_EN3						  , 0 , 0},
	{ 34  ,"PMA_TOP_PHYD_TX_BV0"						 ,PMA_TOP_PHYD_TX_BV0						  , 0 , 0},
	{ 35  ,"PMA_TOP_PHYD_TX_BV1"						 ,PMA_TOP_PHYD_TX_BV1						  , 0 , 0},
	{ 36  ,"PMA_TOP_PHYD_TX_BV2"						 ,PMA_TOP_PHYD_TX_BV2						  , 0 , 0},
	{ 37  ,"PMA_TOP_PHYD_TX_BV3"						 ,PMA_TOP_PHYD_TX_BV3						  , 0 , 0},
	{ 38  ,"PMA_TOP_PHYD_TX_BIT_DLY0"					 ,PMA_TOP_PHYD_TX_BIT_DLY0					  , 0 , 0},
	{ 39  ,"PMA_TOP_PHYD_TX_BIT_DLY1"					 ,PMA_TOP_PHYD_TX_BIT_DLY1					  , 0 , 0},
	{ 40  ,"PMA_TOP_PHYD_TX_BIT_DLY2"					 ,PMA_TOP_PHYD_TX_BIT_DLY2					  , 0 , 0},
	{ 41  ,"PMA_TOP_PHYD_TX_BIT_DLY3"					 ,PMA_TOP_PHYD_TX_BIT_DLY3					  , 0 , 0},
	{ 42  ,"PMA_TOP_PHYD_TX_PARITY0"					 ,PMA_TOP_PHYD_TX_PARITY0					  , 0 , 0},
	{ 43  ,"PMA_TOP_PHYD_TX_PARITY1"					 ,PMA_TOP_PHYD_TX_PARITY1					  , 0 , 0},
	{ 44  ,"PMA_TOP_PHYD_TX_PARITY2"					 ,PMA_TOP_PHYD_TX_PARITY2					  , 0 , 0},
	{ 45  ,"PMA_TOP_PHYD_TX_PARITY3"					 ,PMA_TOP_PHYD_TX_PARITY3					  , 0 , 0},
	{ 46  ,"PMA_TOP_PHYD_RX_CLK0"						 ,PMA_TOP_PHYD_RX_CLK0						  , 0 , 0},
	{ 47  ,"PMA_TOP_PHYD_RX_CLK1"						 ,PMA_TOP_PHYD_RX_CLK1						  , 0 , 0},
	{ 48  ,"PMA_TOP_PHYD_RX_CLK2"						 ,PMA_TOP_PHYD_RX_CLK2						  , 0 , 0},
	{ 49  ,"PMA_TOP_PHYD_RX_CLK3"						 ,PMA_TOP_PHYD_RX_CLK3						  , 0 , 0},
	{ 50  ,"PMA_TOP_PHYD_RX_D0" 						 ,PMA_TOP_PHYD_RX_D0						  , 0 , 0},
	{ 51  ,"PMA_TOP_PHYD_RX_D1" 						 ,PMA_TOP_PHYD_RX_D1						  , 0 , 0},
	{ 52  ,"PMA_TOP_PHYD_RX_D2" 						 ,PMA_TOP_PHYD_RX_D2						  , 0 , 0},
	{ 53  ,"PMA_TOP_PHYD_RX_D3" 						 ,PMA_TOP_PHYD_RX_D3						  , 0 , 0},
	{ 54  ,"PMA_TOP_PHYD_RX_D4" 						 ,PMA_TOP_PHYD_RX_D4						  , 0 , 0},
	{ 55  ,"PMA_TOP_PHYD_RX_D5" 						 ,PMA_TOP_PHYD_RX_D5						  , 0 , 0},
	{ 56  ,"PMA_TOP_PHYD_RX_D6" 						 ,PMA_TOP_PHYD_RX_D6						  , 0 , 0},
	{ 57  ,"PMA_TOP_PHYD_RX_D7" 						 ,PMA_TOP_PHYD_RX_D7						  , 0 , 0},
	{ 58  ,"PMA_TOP_PHYD_RX_D8" 						 ,PMA_TOP_PHYD_RX_D8						  , 0 , 0},
	{ 59  ,"PMA_TOP_PHYD_RX_D9" 						 ,PMA_TOP_PHYD_RX_D9						  , 0 , 0},
	{ 60  ,"PMA_TOP_PHYD_RX_D10"						 ,PMA_TOP_PHYD_RX_D10						  , 0 , 0},
	{ 61  ,"PMA_TOP_PHYD_RX_D11"						 ,PMA_TOP_PHYD_RX_D11						  , 0 , 0},
	{ 62  ,"PMA_TOP_PHYD_RX_D12"						 ,PMA_TOP_PHYD_RX_D12						  , 0 , 0},
	{ 63  ,"PMA_TOP_PHYD_RX_D13"						 ,PMA_TOP_PHYD_RX_D13						  , 0 , 0},
	{ 64  ,"PMA_TOP_PHYD_RX_D14"						 ,PMA_TOP_PHYD_RX_D14						  , 0 , 0},
	{ 65  ,"PMA_TOP_PHYD_RX_D15"						 ,PMA_TOP_PHYD_RX_D15						  , 0 , 0},
	{ 66  ,"PMA_TOP_PHYD_RX_D16"						 ,PMA_TOP_PHYD_RX_D16						  , 0 , 0},
	{ 67  ,"PMA_TOP_PHYD_RX_D17"						 ,PMA_TOP_PHYD_RX_D17						  , 0 , 0},
	{ 68  ,"PMA_TOP_PHYD_RX_D18"						 ,PMA_TOP_PHYD_RX_D18						  , 0 , 0},
	{ 69  ,"PMA_TOP_PHYD_RX_D19"						 ,PMA_TOP_PHYD_RX_D19						  , 0 , 0},
	{ 70  ,"PMA_TOP_PHYD_RX_D20"						 ,PMA_TOP_PHYD_RX_D20						  , 0 , 0},
	{ 71  ,"PMA_TOP_PHYD_RX_D21"						 ,PMA_TOP_PHYD_RX_D21						  , 0 , 0},
	{ 72  ,"PMA_TOP_PHYD_RX_D22"						 ,PMA_TOP_PHYD_RX_D22						  , 0 , 0},
	{ 73  ,"PMA_TOP_PHYD_RX_D23"						 ,PMA_TOP_PHYD_RX_D23						  , 0 , 0},
	{ 74  ,"PMA_TOP_PHYD_RX_D24"						 ,PMA_TOP_PHYD_RX_D24						  , 0 , 0},
	{ 75  ,"PMA_TOP_PHYD_RX_D25"						 ,PMA_TOP_PHYD_RX_D25						  , 0 , 0},
	{ 76  ,"PMA_TOP_PHYD_RX_RDY0"						 ,PMA_TOP_PHYD_RX_RDY0						  , 0 , 0},
	{ 77  ,"PMA_TOP_PHYD_RX_RDY1"						 ,PMA_TOP_PHYD_RX_RDY1						  , 0 , 0},
	{ 78  ,"PMA_TOP_PHYD_RX_RDY2"						 ,PMA_TOP_PHYD_RX_RDY2						  , 0 , 0},
	{ 79  ,"PMA_TOP_PHYD_RX_RDY3"						 ,PMA_TOP_PHYD_RX_RDY3						  , 0 , 0},
	{ 80  ,"PMA_TOP_PHYD_RX_PARITY0"					 ,PMA_TOP_PHYD_RX_PARITY0					  , 0 , 0},
	{ 81  ,"PMA_TOP_PHYD_RX_PARITY1"					 ,PMA_TOP_PHYD_RX_PARITY1					  , 0 , 0},
	{ 82  ,"PMA_TOP_PHYD_RX_PARITY2"					 ,PMA_TOP_PHYD_RX_PARITY2					  , 0 , 0},
	{ 83  ,"PMA_TOP_PHYD_RX_PARITY3"					 ,PMA_TOP_PHYD_RX_PARITY3					  , 0 , 0},
	{ 84  ,"PMA_TOP_PHYD_TX_EOF0"						 ,PMA_TOP_PHYD_TX_EOF0						  , 0 , 0},
	{ 85  ,"PMA_TOP_PHYD_TX_EOF1"						 ,PMA_TOP_PHYD_TX_EOF1						  , 0 , 0},
	{ 86  ,"PMA_TOP_PHYD_TX_EOF2"						 ,PMA_TOP_PHYD_TX_EOF2						  , 0 , 0},
	{ 87  ,"PMA_TOP_PHYD_TX_EOF3"						 ,PMA_TOP_PHYD_TX_EOF3						  , 0 , 0},
	{ 88  ,"PMA_TOP_PHYD_RXEN_SYNC0"					 ,PMA_TOP_PHYD_RXEN_SYNC0					  , 0 , 0},
	{ 89  ,"PMA_TOP_PHYD_RXEN_SYNC1"					 ,PMA_TOP_PHYD_RXEN_SYNC1					  , 0 , 0},
	{ 90  ,"PMA_TOP_PHYD_RXEN_SYNC2"					 ,PMA_TOP_PHYD_RXEN_SYNC2					  , 0 , 0},
	{ 91  ,"PMA_TOP_PHYD_RXEN_SYNC3"					 ,PMA_TOP_PHYD_RXEN_SYNC3					  , 0 , 0},
	{ 92  ,"PMA_TOP_PHYD_RX_ALIGN0" 					 ,PMA_TOP_PHYD_RX_ALIGN0					  , 0 , 0},
	{ 93  ,"PMA_TOP_PHYD_RX_ALIGN1" 					 ,PMA_TOP_PHYD_RX_ALIGN1					  , 0 , 0},
	{ 94  ,"PMA_TOP_PHYD_RX_ALIGN2" 					 ,PMA_TOP_PHYD_RX_ALIGN2					  , 0 , 0},
	{ 95  ,"PMA_TOP_PHYD_RX_ALIGN3" 					 ,PMA_TOP_PHYD_RX_ALIGN3					  , 0 , 0},
	{ 96  ,"PMA_TOP_PHYD_RX_LOF0"						 ,PMA_TOP_PHYD_RX_LOF0						  , 0 , 0},
	{ 97  ,"PMA_TOP_PHYD_RX_LOF1"						 ,PMA_TOP_PHYD_RX_LOF1						  , 0 , 0},
	{ 98  ,"PMA_TOP_PHYD_RX_LOF2"						 ,PMA_TOP_PHYD_RX_LOF2						  , 0 , 0},
	{ 99  ,"PMA_TOP_PHYD_RX_LOF3"						 ,PMA_TOP_PHYD_RX_LOF3						  , 0 , 0},
	{ 100 ,"PMA_TOP_PHYD_CAL_LOS0"						 ,PMA_TOP_PHYD_CAL_LOS0 					  , 0 , 0},
	{ 101 ,"PMA_TOP_PHYD_CAL_LOS1"						 ,PMA_TOP_PHYD_CAL_LOS1 					  , 0 , 0},
	{ 102 ,"PMA_TOP_PHYD_CAL_LOS2"						 ,PMA_TOP_PHYD_CAL_LOS2 					  , 0 , 0},
	{ 103 ,"PMA_TOP_PHYD_CAL_LOS3"						 ,PMA_TOP_PHYD_CAL_LOS3 					  , 0 , 0},
	{ 104 ,"PMA_TOP_PHYD_SEC_EN0"						 ,PMA_TOP_PHYD_SEC_EN0						  , 0 , 0},
	{ 105 ,"PMA_TOP_PHYD_SEC_EN1"						 ,PMA_TOP_PHYD_SEC_EN1						  , 0 , 0},
	{ 106 ,"PMA_TOP_PHYD_SEC_EN2"						 ,PMA_TOP_PHYD_SEC_EN2						  , 0 , 0},
	{ 107 ,"PMA_TOP_PHYD_SEC_EN3"						 ,PMA_TOP_PHYD_SEC_EN3						  , 0 , 0},
	{ 108 ,"PMA_TOP_TRAN_BURST_EN0" 					 ,PMA_TOP_TRAN_BURST_EN0					  , 0 , 0},
	{ 109 ,"PMA_TOP_TRAN_BURST_EN1" 					 ,PMA_TOP_TRAN_BURST_EN1					  , 0 , 0},
	{ 110 ,"PMA_TOP_TRAN_BURST_EN2" 					 ,PMA_TOP_TRAN_BURST_EN2					  , 0 , 0},
	{ 111 ,"PMA_TOP_TRAN_BURST_EN3" 					 ,PMA_TOP_TRAN_BURST_EN3					  , 0 , 0},
	{ 112 ,"PMA_TOP_TRAN_TX_FAULT0" 					 ,PMA_TOP_TRAN_TX_FAULT0					  , 0 , 0},
	{ 113 ,"PMA_TOP_TRAN_TX_FAULT1" 					 ,PMA_TOP_TRAN_TX_FAULT1					  , 0 , 0},
	{ 114 ,"PMA_TOP_TRAN_TX_FAULT2" 					 ,PMA_TOP_TRAN_TX_FAULT2					  , 0 , 0},
	{ 115 ,"PMA_TOP_TRAN_TX_FAULT3" 					 ,PMA_TOP_TRAN_TX_FAULT3					  , 0 , 0},
	{ 116 ,"PMA_TOP_TRAN_TX_SD0"						 ,PMA_TOP_TRAN_TX_SD0						  , 0 , 0},
	{ 117 ,"PMA_TOP_TRAN_TX_SD1"						 ,PMA_TOP_TRAN_TX_SD1						  , 0 , 0},
	{ 118 ,"PMA_TOP_TRAN_TX_SD2"						 ,PMA_TOP_TRAN_TX_SD2						  , 0 , 0},
	{ 119 ,"PMA_TOP_TRAN_TX_SD3"						 ,PMA_TOP_TRAN_TX_SD3						  , 0 , 0},
	{ 120 ,"PMA_TOP_TRAN_RX_SD0"						 ,PMA_TOP_TRAN_RX_SD0						  , 0 , 0},
	{ 121 ,"PMA_TOP_TRAN_RX_SD1"						 ,PMA_TOP_TRAN_RX_SD1						  , 0 , 0},
	{ 122 ,"PMA_TOP_TRAN_RX_SD2"						 ,PMA_TOP_TRAN_RX_SD2						  , 0 , 0},
	{ 123 ,"PMA_TOP_TRAN_RX_SD3"						 ,PMA_TOP_TRAN_RX_SD3						  , 0 , 0},
	{ 124 ,"PMA_TOP_I2C_SDA0"							 ,PMA_TOP_I2C_SDA0							  , 0 , 0},
	{ 125 ,"PMA_TOP_I2C_SDA1"							 ,PMA_TOP_I2C_SDA1							  , 0 , 0},
	{ 126 ,"PMA_TOP_I2C_SDA2"							 ,PMA_TOP_I2C_SDA2							  , 0 , 0},
	{ 127 ,"PMA_TOP_I2C_SDA3"							 ,PMA_TOP_I2C_SDA3							  , 0 , 0},
	{ 128 ,"PMA_TOP_I2C_SCL0"							 ,PMA_TOP_I2C_SCL0							  , 0 , 0},
	{ 129 ,"PMA_TOP_I2C_SCL1"							 ,PMA_TOP_I2C_SCL1							  , 0 , 0},
	{ 130 ,"PMA_TOP_I2C_SCL2"							 ,PMA_TOP_I2C_SCL2							  , 0 , 0},
	{ 131 ,"PMA_TOP_I2C_SCL3"							 ,PMA_TOP_I2C_SCL3							  , 0 , 0},
	{ 132 ,"PMA_TOP_GPIO0"								 ,PMA_TOP_GPIO0 							  , 0 , 0},
	{ 133 ,"PMA_TOP_GPIO1"								 ,PMA_TOP_GPIO1 							  , 0 , 0},
	{ 134 ,"PMA_TOP_GPIO2"								 ,PMA_TOP_GPIO2 							  , 0 , 0},
	{ 135 ,"PMA_TOP_GPIO3"								 ,PMA_TOP_GPIO3 							  , 0 , 0},
	{ 136 ,"PMA_TOP_GPIO4"								 ,PMA_TOP_GPIO4 							  , 0 , 0},
	{ 137 ,"PMA_TOP_GPIO5"								 ,PMA_TOP_GPIO5 							  , 0 , 0},
	{ 138 ,"PMA_TOP_GPIO6"								 ,PMA_TOP_GPIO6 							  , 0 , 0},
	{ 139 ,"PMA_TOP_PROBE0" 							 ,PMA_TOP_PROBE0							  , 0 , 0},
	{ 140 ,"PMA_TOP_PROBE1" 							 ,PMA_TOP_PROBE1							  , 0 , 0},
	{ 141 ,"PMA_TOP_PROBE2" 							 ,PMA_TOP_PROBE2							  , 0 , 0},
	{ 142 ,"PMA_TOP_PROBE3" 							 ,PMA_TOP_PROBE3							  , 0 , 0},
	{ 143 ,"PMA_TOP_PROBE4" 							 ,PMA_TOP_PROBE4							  , 0 , 0},
	{ 144 ,"PMA_TOP_PROBE5" 							 ,PMA_TOP_PROBE5							  , 0 , 0},
	{ 145 ,"PMA_TOP_PROBE6" 							 ,PMA_TOP_PROBE6							  , 0 , 0},
	{ 146 ,"PMA_TOP_PROBE7" 							 ,PMA_TOP_PROBE7							  , 0 , 0},
	{ 147 ,"PMA_TOP_PROBE8" 							 ,PMA_TOP_PROBE8							  , 0 , 0},
	{ 148 ,"PMA_TOP_PROBE9" 							 ,PMA_TOP_PROBE9							  , 0 , 0},
	{ 149 ,"PMA_TOP_PROBE10"							 ,PMA_TOP_PROBE10							  , 0 , 0},
	{ 150 ,"PMA_TOP_PROBE11"							 ,PMA_TOP_PROBE11							  , 0 , 0},
	{ 151 ,"PMA_TOP_PROBE12"							 ,PMA_TOP_PROBE12							  , 0 , 0},
	{ 152 ,"PMA_TOP_RESETB0"							 ,PMA_TOP_RESETB0							  , 0 , 0},
	{ 153 ,"PMA_TOP_RESETB1"							 ,PMA_TOP_RESETB1							  , 0 , 0},
	{ 154 ,"PMA_TOP_RESETB2"							 ,PMA_TOP_RESETB2							  , 0 , 0},
	{ 155 ,"PMA_TOP_RESETB3"							 ,PMA_TOP_RESETB3							  , 0 , 0},
	{ 156 ,"PMA_TOP_XTAL_SEL0"							 ,PMA_TOP_XTAL_SEL0 						  , 0 , 0},
	{ 157 ,"PMA_TOP_XTAL_SEL1"							 ,PMA_TOP_XTAL_SEL1 						  , 0 , 0},
	{ 158 ,"PMA_TOP_XTAL_SEL2"							 ,PMA_TOP_XTAL_SEL2 						  , 0 , 0},
	{ 159 ,"PMA_TOP_XTAL_SEL3"							 ,PMA_TOP_XTAL_SEL3 						  , 0 , 0},
	{ 160 ,"PMA_TOP_PON_DGINP0" 						 ,PMA_TOP_PON_DGINP0						  , 0 , 0},
	{ 161 ,"PMA_TOP_PON_DGINP1" 						 ,PMA_TOP_PON_DGINP1						  , 0 , 0},
	{ 162 ,"PMA_TOP_PON_DGINP2" 						 ,PMA_TOP_PON_DGINP2						  , 0 , 0},
	{ 163 ,"PMA_TOP_PON_DGINP3" 						 ,PMA_TOP_PON_DGINP3						  , 0 , 0},
	{ 164 ,"PMA_TOP_JTAG_TCK0"							 ,PMA_TOP_JTAG_TCK0 						  , 0 , 0},
	{ 165 ,"PMA_TOP_JTAG_TCK1"							 ,PMA_TOP_JTAG_TCK1 						  , 0 , 0},
	{ 166 ,"PMA_TOP_JTAG_TCK2"							 ,PMA_TOP_JTAG_TCK2 						  , 0 , 0},
	{ 167 ,"PMA_TOP_JTAG_TCK3"							 ,PMA_TOP_JTAG_TCK3 						  , 0 , 0},
	{ 168 ,"PMA_TOP_JTAG_TDI0"							 ,PMA_TOP_JTAG_TDI0 						  , 0 , 0},
	{ 169 ,"PMA_TOP_JTAG_TDI1"							 ,PMA_TOP_JTAG_TDI1 						  , 0 , 0},
	{ 170 ,"PMA_TOP_JTAG_TDI2"							 ,PMA_TOP_JTAG_TDI2 						  , 0 , 0},
	{ 171 ,"PMA_TOP_JTAG_TDI3"							 ,PMA_TOP_JTAG_TDI3 						  , 0 , 0},
	{ 172 ,"PMA_TOP_JTAG_TMS0"							 ,PMA_TOP_JTAG_TMS0 						  , 0 , 0},
	{ 173 ,"PMA_TOP_JTAG_TMS1"							 ,PMA_TOP_JTAG_TMS1 						  , 0 , 0},
	{ 174 ,"PMA_TOP_JTAG_TMS2"							 ,PMA_TOP_JTAG_TMS2 						  , 0 , 0},
	{ 175 ,"PMA_TOP_JTAG_TMS3"							 ,PMA_TOP_JTAG_TMS3 						  , 0 , 0},
	{ 176 ,"PMA_TOP_JTAG_TRST0" 						 ,PMA_TOP_JTAG_TRST0						  , 0 , 0},
	{ 177 ,"PMA_TOP_JTAG_TRST1" 						 ,PMA_TOP_JTAG_TRST1						  , 0 , 0},
	{ 178 ,"PMA_TOP_JTAG_TRST2" 						 ,PMA_TOP_JTAG_TRST2						  , 0 , 0},
	{ 179 ,"PMA_TOP_JTAG_TRST3" 						 ,PMA_TOP_JTAG_TRST3						  , 0 , 0},
	{ 180 ,"PMA_TOP_JTAG_TDO0"							 ,PMA_TOP_JTAG_TDO0 						  , 0 , 0},
	{ 181 ,"PMA_TOP_JTAG_TDO1"							 ,PMA_TOP_JTAG_TDO1 						  , 0 , 0},
	{ 182 ,"PMA_TOP_JTAG_TDO2"							 ,PMA_TOP_JTAG_TDO2 						  , 0 , 0},
	{ 183 ,"PMA_TOP_JTAG_TDO3"							 ,PMA_TOP_JTAG_TDO3 						  , 0 , 0},
	{ 184 ,"PMA_TOP_BGPOR_TOP_0"						 ,PMA_TOP_BGPOR_TOP_0						  , 0 , 0},
	{ 185 ,"PMA_TOP_BGPOR_TOP_1"						 ,PMA_TOP_BGPOR_TOP_1						  , 0 , 0},
	{ 186 ,"PMA_TOP_BGPOR_TOP_2"						 ,PMA_TOP_BGPOR_TOP_2						  , 0 , 0},
	{ 187 ,"PMA_TOP_BGPOR_TOP_3"						 ,PMA_TOP_BGPOR_TOP_3						  , 0 , 0},
	{ 188 ,"PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_0"			 ,PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_0			  , 0 , 0},
	{ 189 ,"PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1"			 ,PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1			  , 0 , 0},
	{ 190 ,"PMA_TOP_TRAN_BURST_EN_RD_1" 				 ,PMA_TOP_TRAN_BURST_EN_RD_1				  , 0 , 0},
	{ 191 ,"PMA_TOP_TRAN_TX_FAULT_RD_1" 				 ,PMA_TOP_TRAN_TX_FAULT_RD_1				  , 0 , 0},
	{ 192 ,"PMA_TOP_TRAN_TX_SD_RD_1"					 ,PMA_TOP_TRAN_TX_SD_RD_1					  , 0 , 0},
	{ 193 ,"PMA_TOP_TRAN_RX_SD_RD_1"					 ,PMA_TOP_TRAN_RX_SD_RD_1					  , 0 , 0},
	{ 194 ,"PMA_TOP_I2C_SDA_RD_1"						 ,PMA_TOP_I2C_SDA_RD_1						  , 0 , 0},
	{ 195 ,"PMA_TOP_I2C_SCL_RD_1"						 ,PMA_TOP_I2C_SCL_RD_1						  , 0 , 0},
	{ 196 ,"PMA_TOP_PON_DGINP_RD_1" 					 ,PMA_TOP_PON_DGINP_RD_1					  , 0 , 0},
	{ 197 ,"PMA_TOP_TX_DATA_NEG_VLD_0"					 ,PMA_TOP_TX_DATA_NEG_VLD_0 				  , 0 , 0},
	{ 198 ,"PMA_TOP_TX_DATA_NEG_VLD_1"					 ,PMA_TOP_TX_DATA_NEG_VLD_1 				  , 0 , 0},
	{ 199 ,"PMA_TOP_TX_DATA_NEG_VLD_2"					 ,PMA_TOP_TX_DATA_NEG_VLD_2 				  , 0 , 0},
	{ 200 ,"PMA_TOP_TX_DATA_NEG_VLD_3"					 ,PMA_TOP_TX_DATA_NEG_VLD_3 				  , 0 , 0},
	{ 201 ,"PMA_TOP_ADD_MD32_PM_CFG"					 ,PMA_TOP_ADD_MD32_PM_CFG					  , 0 , 0},
	{ 202 ,"PMA_TOP_ADD_MD32_PM_ADDR"					 ,PMA_TOP_ADD_MD32_PM_ADDR					  , 0 , 0},
	{ 203 ,"PMA_TOP_ADD_MD32_PM_DATA"					 ,PMA_TOP_ADD_MD32_PM_DATA					  , 0 , 0},
	{ 204 ,"PMA_TOP_ADD_MD32_DM_CFG"					 ,PMA_TOP_ADD_MD32_DM_CFG					  , 0 , 0},
	{ 205 ,"PMA_TOP_ADD_MD32_DM_ADDR"					 ,PMA_TOP_ADD_MD32_DM_ADDR					  , 0 , 0},
	{ 206 ,"PMA_TOP_ADD_MD32_DM_DATA"					 ,PMA_TOP_ADD_MD32_DM_DATA					  , 0 , 0},
	{ 207 ,"PMA_TOP_ADD_MD32_EN"						 ,PMA_TOP_ADD_MD32_EN						  , 0 , 0},
	{ 208 ,"PMA_TOP_ADD_DMEM_ADDR"						 ,PMA_TOP_ADD_DMEM_ADDR 					  , 0 , 0},
	{ 209 ,"PMA_TOP_ADD_R2T_MODE_0" 					 ,PMA_TOP_ADD_R2T_MODE_0					  , 0 , 0},
	{ 210 ,"PMA_TOP_ADD_R2T_MODE_1" 					 ,PMA_TOP_ADD_R2T_MODE_1					  , 0 , 0},
	{ 211 ,"PMA_TOP_ADD_R2T_MODE_2" 					 ,PMA_TOP_ADD_R2T_MODE_2					  , 0 , 0},
	{ 212 ,"PMA_TOP_XPON_TX_RAWDATA_ADDR"				 ,PMA_TOP_XPON_TX_RAWDATA_ADDR				  , 0 , 0},
	{ 213 ,"PMA_TOP_XPON_TX_RAWDATA_DATA"				 ,PMA_TOP_XPON_TX_RAWDATA_DATA				  , 0 , 0},
	{ 214 ,"PMA_TOP_XPON_RX_RAWDATA_ADDR"				 ,PMA_TOP_XPON_RX_RAWDATA_ADDR				  , 0 , 0},
	{ 215 ,"PMA_TOP_XPON_RX_RAWDATA_DATA"				 ,PMA_TOP_XPON_RX_RAWDATA_DATA				  , 0 , 0},
	{ 216 ,"PMA_TOP_XPON_R2T_MODE_EN"					 ,PMA_TOP_XPON_R2T_MODE_EN					  , 0 , 0},
	{ 217 ,"PMA_TOP_TX_PAR_PROBE_PLL_CTRL"				 ,PMA_TOP_TX_PAR_PROBE_PLL_CTRL 			  , 0 , 0},
	{ 218 ,"PMA_TOP_DUMP_RX_PCSRST_CTRL"				 ,PMA_TOP_DUMP_RX_PCSRST_CTRL				  , 0 , 0},
	{ 219 ,"PMA_TOP_RESERVE_2"							 ,PMA_TOP_RESERVE_2 						  , 0 , 0},
	{ 220 ,"PMA_TOP_TX_ASIC_FF_CTRL"					 ,PMA_TOP_TX_ASIC_FF_CTRL					  , 0 , 0},
	{ 221 ,"PMA_TOP_add_top_reserve_4"					 ,PMA_TOP_add_top_reserve_4 				  , 0 , 0},
	{ 222 ,"PMA_TOP_add_BIAS_TUNE_0"					 ,PMA_TOP_add_BIAS_TUNE_0					  , 0 , 0},
	{ 223 ,"PMA_TOP_add_XPON_TX_AFIFO_0"				 ,PMA_TOP_add_XPON_TX_AFIFO_0				  , 0 , 0},
	{ 224 ,"PMA_TOP_TX_AFIFO_1" 						 ,PMA_TOP_TX_AFIFO_1						  , 0 , 0},
	{ 225 ,"PMA_TOP_SW_RST_0"							 ,PMA_TOP_SW_RST_0							  , 0 , 0},
	{ 226 ,"PMA_TOP_PARITY_CHK" 						 ,PMA_TOP_PARITY_CHK						  , 0 , 0},
	{ 227 ,"PMA_TOP_XPON_TX_FIFO_STS"					 ,PMA_TOP_XPON_TX_FIFO_STS					  , 0 , 0},
	{ 228 ,"PMA_TOP_XFI_TX_FIFO_STS"					 ,PMA_TOP_XFI_TX_FIFO_STS					  , 0 , 0},
	{ 229 ,"PMA_TOP_TRAN_TX_DIS_0"						 ,PMA_TOP_TRAN_TX_DIS_0 					  , 0 , 0},
	{ 230 ,"PMA_TOP_TRAN_TX_DIS_1"						 ,PMA_TOP_TRAN_TX_DIS_1 					  , 0 , 0},
	{ 231 ,"PMA_TOP_TRAN_TX_DIS_2"						 ,PMA_TOP_TRAN_TX_DIS_2 					  , 0 , 0},
	{ 232 ,"PMA_TOP_TRAN_TX_DIS_3"						 ,PMA_TOP_TRAN_TX_DIS_3 					  , 0 , 0},
	{ 233 ,"PMA_TOP_TRAN_RX_DIS_0"						 ,PMA_TOP_TRAN_RX_DIS_0 					  , 0 , 0},
	{ 234 ,"PMA_TOP_TRAN_RX_DIS_1"						 ,PMA_TOP_TRAN_RX_DIS_1 					  , 0 , 0},
	{ 235 ,"PMA_TOP_TRAN_RX_DIS_2"						 ,PMA_TOP_TRAN_RX_DIS_2 					  , 0 , 0},
	{ 236 ,"PMA_TOP_TRAN_RX_DIS_3"						 ,PMA_TOP_TRAN_RX_DIS_3 					  , 0 , 0},
	{ 237 ,"PMA_TOP_TRAN_TX_DIS_RD_1"					 ,PMA_TOP_TRAN_TX_DIS_RD_1					  , 0 , 0},
	{ 238 ,"PMA_TOP_TRAN_RX_DIS_RD_1"					 ,PMA_TOP_TRAN_RX_DIS_RD_1					  , 0 , 0},
	{ 239 ,"PMA_TOP_MD32_IREQ_SELECT"					 ,PMA_TOP_MD32_IREQ_SELECT					  , 0 , 0},
	{ 240 ,"PMA_TOP_PMA_INTERFACE_SELECT"				 ,PMA_TOP_PMA_INTERFACE_SELECT				  , 0 , 0},
	{ 241 ,"PMA_TOP_XFI_TX_AFIFO_0" 					 ,PMA_TOP_XFI_TX_AFIFO_0					  , 0 , 0},
	{ 242 ,"PMA_TOP_XFI_TX_RAWDATA_ADDR"				 ,PMA_TOP_XFI_TX_RAWDATA_ADDR				  , 0 , 0},
	{ 243 ,"PMA_TOP_XFI_TX_RAWDATA_DATA"				 ,PMA_TOP_XFI_TX_RAWDATA_DATA				  , 0 , 0},
	{ 244 ,"PMA_TOP_XFI_RX_RAWDATA_ADDR"				 ,PMA_TOP_XFI_RX_RAWDATA_ADDR				  , 0 , 0},
	{ 245 ,"PMA_TOP_XFI_RX_RAWDATA_DATA"				 ,PMA_TOP_XFI_RX_RAWDATA_DATA				  , 0 , 0},
	{ 246 ,"PMA_TOP_XFI_R2T_MODE_EN"					 ,PMA_TOP_XFI_R2T_MODE_EN					  , 0 , 0},
	{ 247 ,"PMA_TOP_TRAN_TRX_CNTRL" 					 ,PMA_TOP_TRAN_TRX_CNTRL					  , 0 , 0},
	{ 248 ,"PMA_TOP_rg_xfi_tx_rx_mode"					 ,PMA_TOP_rg_xfi_tx_rx_mode 				  , 0 , 0},
	{ 249 ,"PMA_TOP_XPON_PLL_STB_CNT"					 ,PMA_TOP_XPON_PLL_STB_CNT					  , 0 , 0},
	{ 250 ,"PMA_TOP_XPON_PLL_STOP_CNT"					 ,PMA_TOP_XPON_PLL_STOP_CNT 				  , 0 , 0},
	{ 251 ,"PMA_TOP_XFI_PLL_STB_CNT"					 ,PMA_TOP_XFI_PLL_STB_CNT					  , 0 , 0},
	{ 252 ,"PMA_TOP_XFI_PLL_STOP_CNT"					 ,PMA_TOP_XFI_PLL_STOP_CNT					  , 0 , 0},
	{ 253 ,"PMA_TOP_GPIO_MUX_CTRL"						 ,PMA_TOP_GPIO_MUX_CTRL 					  , 0 , 0},
	{ 254 ,"PMA_TOP_RAWDATA_DUMP_SEL"					 ,PMA_TOP_RAWDATA_DUMP_SEL					  , 0 , 0},
	{ 255 ,"XPON_PMA_RG_XPON_CMN_EN_0"					 ,XPON_PMA_RG_XPON_CMN_EN_0 				  , 0 , 0},
	{ 256 ,"XPON_PMA_RG_XPON_BENTX_BIAS_EN_0"			 ,XPON_PMA_RG_XPON_BENTX_BIAS_EN_0			  , 0 , 0},
	{ 257 ,"XPON_PMA_RG_XPON_RX_MPXSEL_0"				 ,XPON_PMA_RG_XPON_RX_MPXSEL_0				  , 0 , 0},
	{ 258 ,"XPON_PMA_RG_XPON_RX_REV_0_0"				 ,XPON_PMA_RG_XPON_RX_REV_0_0				  , 0 , 0},
	{ 259 ,"XPON_PMA_RG_XPON_RX_PHYCK_DIV_0"			 ,XPON_PMA_RG_XPON_RX_PHYCK_DIV_0			  , 0 , 0},
	{ 260 ,"XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0"	 ,XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0	  , 0 , 0},
	{ 261 ,"XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0"			 ,XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0 		  , 0 , 0},
	{ 262 ,"XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0"		 ,XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0		  , 0 , 0},
	{ 263 ,"XPON_PMA_RG_XPON_CDR_PR_INJ_FORCE_OFF_0"	 ,XPON_PMA_RG_XPON_CDR_PR_INJ_FORCE_OFF_0	  , 0 , 0},
	{ 264 ,"XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0"		 ,XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0		  , 0 , 0},
	{ 265 ,"XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0" 		 ,XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0		  , 0 , 0},
	{ 266 ,"XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0"		 ,XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0		  , 0 , 0},
	{ 267 ,"XPON_PMA_RG_XPON_PLL_IC_0"					 ,XPON_PMA_RG_XPON_PLL_IC_0 				  , 0 , 0},
	{ 268 ,"XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0" 		 ,XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0		  , 0 , 0},
	{ 269 ,"XPON_PMA_RG_XPON_PLL_SDM_ORD_0" 			 ,XPON_PMA_RG_XPON_PLL_SDM_ORD_0			  , 0 , 0},
	{ 270 ,"XPON_PMA_RG_XPON_PLL_RESERVE0_0"			 ,XPON_PMA_RG_XPON_PLL_RESERVE0_0			  , 0 , 0},
	{ 271 ,"XPON_PMA_RGS_XPON_PLL_AUTOK_BAND_0" 		 ,XPON_PMA_RGS_XPON_PLL_AUTOK_BAND_0		  , 0 , 0},
	{ 272 ,"XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0"		 ,XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0		  , 0 , 0},
	{ 273 ,"XPON_PMA_RG_XPON_TX_EN_0"					 ,XPON_PMA_RG_XPON_TX_EN_0					  , 0 , 0},
	{ 274 ,"XPON_PMA_RG_XPON_TX_LSDATA_EN_0"			 ,XPON_PMA_RG_XPON_TX_LSDATA_EN_0			  , 0 , 0},
	{ 275 ,"XPON_PMA_RG_XPON_TX_RESERVED_0" 			 ,XPON_PMA_RG_XPON_TX_RESERVED_0			  , 0 , 0},
	{ 276 ,"XPON_PMA_SS_TX_DA_1"						 ,XPON_PMA_SS_TX_DA_1						  , 0 , 0},
	{ 277 ,"XPON_PMA_SS_LCPLL_PWCTL_SETTING_0"			 ,XPON_PMA_SS_LCPLL_PWCTL_SETTING_0 		  , 0 , 0},
	{ 278 ,"XPON_PMA_SS_LCPLL_PWCTL_SETTING_1"			 ,XPON_PMA_SS_LCPLL_PWCTL_SETTING_1 		  , 0 , 0},
	{ 279 ,"XPON_PMA_SS_LCPLL_PWCTL_SETTING_2"			 ,XPON_PMA_SS_LCPLL_PWCTL_SETTING_2 		  , 0 , 0},
	{ 280 ,"XPON_PMA_SS_LCPLL_PWCTL_DBG_SETTING"		 ,XPON_PMA_SS_LCPLL_PWCTL_DBG_SETTING		  , 0 , 0},
	{ 281 ,"XPON_PMA_SS_LCPLL_TDC_PW_0" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_0				  , 0 , 0},
	{ 282 ,"XPON_PMA_SS_LCPLL_TDC_PW_1" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_1				  , 0 , 0},
	{ 283 ,"XPON_PMA_SS_LCPLL_TDC_PW_2" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_2				  , 0 , 0},
	{ 284 ,"XPON_PMA_SS_LCPLL_TDC_PW_3" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_3				  , 0 , 0},
	{ 285 ,"XPON_PMA_SS_LCPLL_TDC_PW_4" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_4				  , 0 , 0},
	{ 286 ,"XPON_PMA_SS_LCPLL_TDC_PW_5" 				 ,XPON_PMA_SS_LCPLL_TDC_PW_5				  , 0 , 0},
	{ 287 ,"XPON_PMA_SS_LCPLL_TDC_FLT_0"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_0				  , 0 , 0},
	{ 288 ,"XPON_PMA_SS_LCPLL_TDC_FLT_1"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_1				  , 0 , 0},
	{ 289 ,"XPON_PMA_SS_LCPLL_TDC_FLT_2"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_2				  , 0 , 0},
	{ 290 ,"XPON_PMA_SS_LCPLL_TDC_FLT_3"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_3				  , 0 , 0},
	{ 291 ,"XPON_PMA_SS_LCPLL_TDC_FLT_4"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_4				  , 0 , 0},
	{ 292 ,"XPON_PMA_SS_LCPLL_TDC_FLT_5"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_5				  , 0 , 0},
	{ 293 ,"XPON_PMA_SS_LCPLL_TDC_FLT_6"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_6				  , 0 , 0},
	{ 294 ,"XPON_PMA_SS_LCPLL_TDC_FLT_7"				 ,XPON_PMA_SS_LCPLL_TDC_FLT_7				  , 0 , 0},
	{ 295 ,"XPON_PMA_SS_LCPLL_TDC_PCW_1"				 ,XPON_PMA_SS_LCPLL_TDC_PCW_1				  , 0 , 0},
	{ 296 ,"XPON_PMA_SS_LCPLL_TDC_PCW_2"				 ,XPON_PMA_SS_LCPLL_TDC_PCW_2				  , 0 , 0},
	{ 297 ,"XPON_PMA_SS_LCPLL_TDC_RO_1" 				 ,XPON_PMA_SS_LCPLL_TDC_RO_1				  , 0 , 0},
	{ 298 ,"XPON_PMA_SS_LCPLL_TDC_RO_2" 				 ,XPON_PMA_SS_LCPLL_TDC_RO_2				  , 0 , 0},
	{ 299 ,"XPON_PMA_SS_LCPLL_TDC_RO_3" 				 ,XPON_PMA_SS_LCPLL_TDC_RO_3				  , 0 , 0},
	{ 300 ,"XPON_PMA_SS_LCPLL_TDC_RO_4" 				 ,XPON_PMA_SS_LCPLL_TDC_RO_4				  , 0 , 0},
	{ 301 ,"XPON_PMA_SS_LCPLL_TDC_RO_5" 				 ,XPON_PMA_SS_LCPLL_TDC_RO_5				  , 0 , 0},
	{ 302 ,"XPON_PMA_SS_LCPLL_TDC_CTRL_0"				 ,XPON_PMA_SS_LCPLL_TDC_CTRL_0				  , 0 , 0},
	{ 303 ,"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0"		 ,XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0		  , 0 , 0},
	{ 304 ,"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1"		 ,XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1		  , 0 , 0},
	{ 305 ,"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2"		 ,XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2		  , 0 , 0},
	{ 306 ,"XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3"		 ,XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3		  , 0 , 0},
	{ 307 ,"XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0"		 ,XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0 	  , 0 , 0},
	{ 308 ,"XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1"		 ,XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1 	  , 0 , 0},
	{ 309 ,"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0"			 ,XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0 		  , 0 , 0},
	{ 310 ,"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1"			 ,XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_1 		  , 0 , 0},
	{ 311 ,"XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2"			 ,XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2 		  , 0 , 0},
	{ 312 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0			  , 0 , 0},
	{ 313 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1			  , 0 , 0},
	{ 314 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2			  , 0 , 0},
	{ 315 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3			  , 0 , 0},
	{ 316 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4			  , 0 , 0},
	{ 317 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5			  , 0 , 0},
	{ 318 ,"XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6"			 ,XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6			  , 0 , 0},
	{ 319 ,"XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0"		 ,XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0 	  , 0 , 0},
	{ 320 ,"XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1"		 ,XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1 	  , 0 , 0},
	{ 321 ,"XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0" 	 ,XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0	  , 0 , 0},
	{ 322 ,"XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1" 	 ,XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1	  , 0 , 0},
	{ 323 ,"XPON_PMA_PHY_EQ_CTRL_0" 					 ,XPON_PMA_PHY_EQ_CTRL_0					  , 0 , 0},
	{ 324 ,"XPON_PMA_PHY_EQ_CTRL_1" 					 ,XPON_PMA_PHY_EQ_CTRL_1					  , 0 , 0},
	{ 325 ,"XPON_PMA_PHY_EQ_CTRL_2" 					 ,XPON_PMA_PHY_EQ_CTRL_2					  , 0 , 0},
	{ 326 ,"XPON_PMA_PHY_EQ_CTRL_3" 					 ,XPON_PMA_PHY_EQ_CTRL_3					  , 0 , 0},
	{ 327 ,"XPON_PMA_PHY_EQ_CTRL_4" 					 ,XPON_PMA_PHY_EQ_CTRL_4					  , 0 , 0},
	{ 328 ,"XPON_PMA_PHY_EQ_CTRL_5" 					 ,XPON_PMA_PHY_EQ_CTRL_5					  , 0 , 0},
	{ 329 ,"XPON_PMA_PHY_EQ_CTRL_6" 					 ,XPON_PMA_PHY_EQ_CTRL_6					  , 0 , 0},
	{ 330 ,"XPON_PMA_PHY_EQ_CTRL_7" 					 ,XPON_PMA_PHY_EQ_CTRL_7					  , 0 , 0},
	{ 331 ,"XPON_PMA_PHY_EQ_CTRL_8" 					 ,XPON_PMA_PHY_EQ_CTRL_8					  , 0 , 0},
	{ 332 ,"XPON_PMA_PHY_EQ_CTRL_9" 					 ,XPON_PMA_PHY_EQ_CTRL_9					  , 0 , 0},
	{ 333 ,"XPON_PMA_PHY_EQ_CTRL_10"					 ,XPON_PMA_PHY_EQ_CTRL_10					  , 0 , 0},
	{ 334 ,"XPON_PMA_SS_RX_FEOS"						 ,XPON_PMA_SS_RX_FEOS						  , 0 , 0},
	{ 335 ,"XPON_PMA_SS_RX_BLWC"						 ,XPON_PMA_SS_RX_BLWC						  , 0 , 0},
	{ 336 ,"XPON_PMA_SS_RX_FREQ_DET_1"					 ,XPON_PMA_SS_RX_FREQ_DET_1 				  , 0 , 0},
	{ 337 ,"XPON_PMA_SS_RX_FREQ_DET_2"					 ,XPON_PMA_SS_RX_FREQ_DET_2 				  , 0 , 0},
	{ 338 ,"XPON_PMA_SS_RX_FREQ_DET_3"					 ,XPON_PMA_SS_RX_FREQ_DET_3 				  , 0 , 0},
	{ 339 ,"XPON_PMA_SS_RX_FREQ_DET_4"					 ,XPON_PMA_SS_RX_FREQ_DET_4 				  , 0 , 0},
	{ 340 ,"XPON_PMA_SS_RX_PI_CAL"						 ,XPON_PMA_SS_RX_PI_CAL 					  , 0 , 0},
	{ 341 ,"XPON_PMA_SS_RX_CAL_1"						 ,XPON_PMA_SS_RX_CAL_1						  , 0 , 0},
	{ 342 ,"XPON_PMA_SS_RX_CAL_2"						 ,XPON_PMA_SS_RX_CAL_2						  , 0 , 0},
	{ 343 ,"XPON_PMA_SS_RX_SIGDET_0"					 ,XPON_PMA_SS_RX_SIGDET_0					  , 0 , 0},
	{ 344 ,"XPON_PMA_SS_RX_SIGDET_1"					 ,XPON_PMA_SS_RX_SIGDET_1					  , 0 , 0},
	{ 345 ,"XPON_PMA_SS_RX_FLL_0"						 ,XPON_PMA_SS_RX_FLL_0						  , 0 , 0},
	{ 346 ,"XPON_PMA_SS_RX_FLL_1"						 ,XPON_PMA_SS_RX_FLL_1						  , 0 , 0},
	{ 347 ,"XPON_PMA_SS_RX_FLL_2"						 ,XPON_PMA_SS_RX_FLL_2						  , 0 , 0},
	{ 348 ,"XPON_PMA_SS_RX_FLL_3"						 ,XPON_PMA_SS_RX_FLL_3						  , 0 , 0},
	{ 349 ,"XPON_PMA_SS_RX_FLL_4"						 ,XPON_PMA_SS_RX_FLL_4						  , 0 , 0},
	{ 350 ,"XPON_PMA_SS_RX_FLL_5"						 ,XPON_PMA_SS_RX_FLL_5						  , 0 , 0},
	{ 351 ,"XPON_PMA_SS_RX_FLL_6"						 ,XPON_PMA_SS_RX_FLL_6						  , 0 , 0},
	{ 352 ,"XPON_PMA_SS_RX_FLL_7"						 ,XPON_PMA_SS_RX_FLL_7						  , 0 , 0},
	{ 353 ,"XPON_PMA_SS_RX_FLL_8"						 ,XPON_PMA_SS_RX_FLL_8						  , 0 , 0},
	{ 354 ,"XPON_PMA_SS_RX_FLL_9"						 ,XPON_PMA_SS_RX_FLL_9						  , 0 , 0},
	{ 355 ,"XPON_PMA_SS_RX_FLL_a"						 ,XPON_PMA_SS_RX_FLL_a						  , 0 , 0},
	{ 356 ,"XPON_PMA_SS_RX_FLL_b"						 ,XPON_PMA_SS_RX_FLL_b						  , 0 , 0},
	{ 357 ,"XPON_PMA_RX_PDOS_CTRL_0"					 ,XPON_PMA_RX_PDOS_CTRL_0					  , 0 , 0},
	{ 358 ,"XPON_PMA_RX_RESET_0"						 ,XPON_PMA_RX_RESET_0						  , 0 , 0},
	{ 359 ,"XPON_PMA_RX_RESET_1"						 ,XPON_PMA_RX_RESET_1						  , 0 , 0},
	{ 360 ,"XPON_PMA_RX_DEBUG_0"						 ,XPON_PMA_RX_DEBUG_0						  , 0 , 0},
	{ 361 ,"XPON_PMA_BISTCTL_CONTROL"					 ,XPON_PMA_BISTCTL_CONTROL					  , 0 , 0},
	{ 362 ,"XPON_PMA_BISTCTL_ALIGN_PAT" 				 ,XPON_PMA_BISTCTL_ALIGN_PAT				  , 0 , 0},
	{ 363 ,"XPON_PMA_BISTCTL_PROGRAM_PAT_0" 			 ,XPON_PMA_BISTCTL_PROGRAM_PAT_0			  , 0 , 0},
	{ 364 ,"XPON_PMA_BISTCTL_PROGRAM_PAT_1" 			 ,XPON_PMA_BISTCTL_PROGRAM_PAT_1			  , 0 , 0},
	{ 365 ,"XPON_PMA_BISTCTL_PROGRAM_PAT_2" 			 ,XPON_PMA_BISTCTL_PROGRAM_PAT_2			  , 0 , 0},
	{ 366 ,"XPON_PMA_BISTCTL_PRBS_INITIAL_SEED" 		 ,XPON_PMA_BISTCTL_PRBS_INITIAL_SEED		  , 0 , 0},
	{ 367 ,"XPON_PMA_BISTCTL_PRBS_EVENT"				 ,XPON_PMA_BISTCTL_PRBS_EVENT				  , 0 , 0},
	{ 368 ,"XPON_PMA_BISTCTL_PRBS_ERRCNT"				 ,XPON_PMA_BISTCTL_PRBS_ERRCNT				  , 0 , 0},
	{ 369 ,"XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD"		 ,XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD		  , 0 , 0},
	{ 370 ,"XPON_PMA_RX_TORGS_DEBUG_0"					 ,XPON_PMA_RX_TORGS_DEBUG_0 				  , 0 , 0},
	{ 371 ,"XPON_PMA_RX_TORGS_DEBUG_1"					 ,XPON_PMA_RX_TORGS_DEBUG_1 				  , 0 , 0},
	{ 372 ,"XPON_PMA_RX_TORGS_DEBUG_2"					 ,XPON_PMA_RX_TORGS_DEBUG_2 				  , 0 , 0},
	{ 373 ,"XPON_PMA_RX_TORGS_DEBUG_3"					 ,XPON_PMA_RX_TORGS_DEBUG_3 				  , 0 , 0},
	{ 374 ,"XPON_PMA_RX_TORGS_DEBUG_4"					 ,XPON_PMA_RX_TORGS_DEBUG_4 				  , 0 , 0},
	{ 375 ,"XPON_PMA_RX_TORGS_DEBUG_5"					 ,XPON_PMA_RX_TORGS_DEBUG_5 				  , 0 , 0},
	{ 376 ,"XPON_PMA_RX_TORGS_DEBUG_6"					 ,XPON_PMA_RX_TORGS_DEBUG_6 				  , 0 , 0},
	{ 377 ,"XPON_PMA_RX_TORGS_DEBUG_7"					 ,XPON_PMA_RX_TORGS_DEBUG_7 				  , 0 , 0},
	{ 378 ,"XPON_PMA_RX_TORGS_DEBUG_8"					 ,XPON_PMA_RX_TORGS_DEBUG_8 				  , 0 , 0},
	{ 379 ,"XPON_PMA_RX_TORGS_DEBUG_9"					 ,XPON_PMA_RX_TORGS_DEBUG_9 				  , 0 , 0},
	{ 380 ,"XPON_PMA_RX_TORGS_DEBUG_10" 				 ,XPON_PMA_RX_TORGS_DEBUG_10				  , 0 , 0},
	{ 381 ,"XPON_PMA_SS_TX_RST_B"						 ,XPON_PMA_SS_TX_RST_B						  , 0 , 0},
	{ 382 ,"XPON_PMA_SS_TX_CALIB_0" 					 ,XPON_PMA_SS_TX_CALIB_0					  , 0 , 0},
	{ 383 ,"XPON_PMA_SS_TX_CALIB_1" 					 ,XPON_PMA_SS_TX_CALIB_1					  , 0 , 0},
	{ 384 ,"XPON_PMA_SS_TX_CALIB_2" 					 ,XPON_PMA_SS_TX_CALIB_2					  , 0 , 0},
	{ 385 ,"XPON_PMA_XPON_SETTING_0"					 ,XPON_PMA_XPON_SETTING_0					  , 0 , 0},
	{ 386 ,"XPON_PMA_XPON_SETTING_1"					 ,XPON_PMA_XPON_SETTING_1					  , 0 , 0},
	{ 387 ,"XPON_PMA_XPON_SETTING_2"					 ,XPON_PMA_XPON_SETTING_2					  , 0 , 0},
	{ 388 ,"XPON_PMA_XPON_STA"							 ,XPON_PMA_XPON_STA 						  , 0 , 0},
	{ 389 ,"XPON_PMA_XPON_INT_EN_0" 					 ,XPON_PMA_XPON_INT_EN_0					  , 0 , 0},
	{ 390 ,"XPON_PMA_XPON_INT_EN_1" 					 ,XPON_PMA_XPON_INT_EN_1					  , 0 , 0},
	{ 391 ,"XPON_PMA_XPON_INT_STA_CLR_0"				 ,XPON_PMA_XPON_INT_STA_CLR_0				  , 0 , 0},
	{ 392 ,"XPON_PMA_XPON_INT_STA_CLR_1"				 ,XPON_PMA_XPON_INT_STA_CLR_1				  , 0 , 0},
	{ 393 ,"XPON_PMA_XPON_INT_STA_0"					 ,XPON_PMA_XPON_INT_STA_0					  , 0 , 0},
	{ 394 ,"XPON_PMA_XPON_INT_STA_1"					 ,XPON_PMA_XPON_INT_STA_1					  , 0 , 0},
	{ 395 ,"XPON_PMA_RX_TORGS_DEBUG_11" 				 ,XPON_PMA_RX_TORGS_DEBUG_11				  , 0 , 0},
	{ 396 ,"XPON_PMA_RX_FORCE_MODE_0"					 ,XPON_PMA_RX_FORCE_MODE_0					  , 0 , 0},
	{ 397 ,"XPON_PMA_RX_FORCE_MODE_1"					 ,XPON_PMA_RX_FORCE_MODE_1					  , 0 , 0},
	{ 398 ,"XPON_PMA_RX_FORCE_MODE_2"					 ,XPON_PMA_RX_FORCE_MODE_2					  , 0 , 0},
	{ 399 ,"XPON_PMA_RX_DISB_MODE_0"					 ,XPON_PMA_RX_DISB_MODE_0					  , 0 , 0},
	{ 400 ,"XPON_PMA_RX_DISB_MODE_1"					 ,XPON_PMA_RX_DISB_MODE_1					  , 0 , 0},
	{ 401 ,"XPON_PMA_RX_DISB_MODE_2"					 ,XPON_PMA_RX_DISB_MODE_2					  , 0 , 0},
	{ 402 ,"XPON_PMA_RX_FORCE_MODE_3"					 ,XPON_PMA_RX_FORCE_MODE_3					  , 0 , 0},
	{ 403 ,"XPON_PMA_RX_FORCE_MODE_4"					 ,XPON_PMA_RX_FORCE_MODE_4					  , 0 , 0},
	{ 404 ,"XPON_PMA_RX_FORCE_MODE_5"					 ,XPON_PMA_RX_FORCE_MODE_5					  , 0 , 0},
	{ 405 ,"XPON_PMA_RX_FORCE_MODE_6"					 ,XPON_PMA_RX_FORCE_MODE_6					  , 0 , 0},
	{ 406 ,"XPON_PMA_RX_DISB_MODE_3"					 ,XPON_PMA_RX_DISB_MODE_3					  , 0 , 0},
	{ 407 ,"XPON_PMA_RX_DISB_MODE_4"					 ,XPON_PMA_RX_DISB_MODE_4					  , 0 , 0},
	{ 408 ,"XPON_PMA_RX_DISB_MODE_5"					 ,XPON_PMA_RX_DISB_MODE_5					  , 0 , 0},
	{ 409 ,"XPON_PMA_RX_FORCE_MODE_7"					 ,XPON_PMA_RX_FORCE_MODE_7					  , 0 , 0},
	{ 410 ,"XPON_PMA_RX_FORCE_MODE_8"					 ,XPON_PMA_RX_FORCE_MODE_8					  , 0 , 0},
	{ 411 ,"XPON_PMA_RX_FORCE_MODE_9"					 ,XPON_PMA_RX_FORCE_MODE_9					  , 0 , 0},
	{ 412 ,"XPON_PMA_RX_DISB_MODE_6"					 ,XPON_PMA_RX_DISB_MODE_6					  , 0 , 0},
	{ 413 ,"XPON_PMA_RX_DISB_MODE_7"					 ,XPON_PMA_RX_DISB_MODE_7					  , 0 , 0},
	{ 414 ,"XPON_PMA_RX_DISB_MODE_8"					 ,XPON_PMA_RX_DISB_MODE_8					  , 0 , 0},
	{ 415 ,"XPON_PMA_SS_BIST_0" 						 ,XPON_PMA_SS_BIST_0						  , 0 , 0},
	{ 416 ,"XPON_PMA_SS_BIST_1" 						 ,XPON_PMA_SS_BIST_1						  , 0 , 0},
	{ 417 ,"XPON_PMA_SS_BIST_2" 						 ,XPON_PMA_SS_BIST_2						  , 0 , 0},
	{ 418 ,"XPON_PMA_SS_DA_XPON_PWDB_0" 				 ,XPON_PMA_SS_DA_XPON_PWDB_0				  , 0 , 0},
	{ 419 ,"XPON_PMA_SS_DA_XPON_PWDB_1" 				 ,XPON_PMA_SS_DA_XPON_PWDB_1				  , 0 , 0},
	{ 420 ,"XPON_PMA_SS_LCPLL_0"						 ,XPON_PMA_SS_LCPLL_0						  , 0 , 0},
	{ 421 ,"XPON_PMA_SS_LCPLL_1"						 ,XPON_PMA_SS_LCPLL_1						  , 0 , 0},
	{ 422 ,"XPON_PMA_SS_LCPLL_2"						 ,XPON_PMA_SS_LCPLL_2						  , 0 , 0},
	{ 423 ,"XPON_PMA_ADD_DIG_RESERVE_0" 				 ,XPON_PMA_ADD_DIG_RESERVE_0				  , 0 , 0},
	{ 424 ,"XPON_PMA_ADD_DIG_RESERVE_1" 				 ,XPON_PMA_ADD_DIG_RESERVE_1				  , 0 , 0},
	{ 425 ,"XPON_PMA_ADD_DIG_RESERVE_2" 				 ,XPON_PMA_ADD_DIG_RESERVE_2				  , 0 , 0},
	{ 426 ,"XPON_PMA_ADD_DIG_RESERVE_3" 				 ,XPON_PMA_ADD_DIG_RESERVE_3				  , 0 , 0},
	{ 427 ,"XPON_PMA_ADD_DIG_RESERVE_4" 				 ,XPON_PMA_ADD_DIG_RESERVE_4				  , 0 , 0},
	{ 428 ,"XPON_PMA_RG_XPON_RX_RESERVED_1" 			 ,XPON_PMA_RG_XPON_RX_RESERVED_1			  , 0 , 0},
	{ 429 ,"XPON_PMA_ADD_DIG_RO_RESERVE_0"				 ,XPON_PMA_ADD_DIG_RO_RESERVE_0 			  , 0 , 0},
	{ 430 ,"XPON_PMA_ADD_DIG_RO_RESERVE_1"				 ,XPON_PMA_ADD_DIG_RO_RESERVE_1 			  , 0 , 0},
	{ 431 ,"XPON_PMA_ADD_DIG_RO_RESERVE_2"				 ,XPON_PMA_ADD_DIG_RO_RESERVE_2 			  , 0 , 0},
	{ 432 ,"XPON_PMA_ADD_DIG_RO_RESERVE_3"				 ,XPON_PMA_ADD_DIG_RO_RESERVE_3 			  , 0 , 0},
	{ 433 ,"XPON_PMA_ADD_DIG_RO_RESERVE_4"				 ,XPON_PMA_ADD_DIG_RO_RESERVE_4 			  , 0 , 0},
	{ 434 ,"XPON_PMA_ADD_RX_SYS_EN_SEL_0"				 ,XPON_PMA_ADD_RX_SYS_EN_SEL_0				  , 0 , 0},
	{ 435 ,"XPON_PMA_PLL_TDC_FREQDET_0" 				 ,XPON_PMA_PLL_TDC_FREQDET_0				  , 0 , 0},
	{ 436 ,"XPON_PMA_PLL_TDC_FREQDET_1" 				 ,XPON_PMA_PLL_TDC_FREQDET_1				  , 0 , 0},
	{ 437 ,"XPON_PMA_PLL_TDC_FREQDET_2" 				 ,XPON_PMA_PLL_TDC_FREQDET_2				  , 0 , 0},
	{ 438 ,"XPON_PMA_PLL_TDC_FREQDET_3" 				 ,XPON_PMA_PLL_TDC_FREQDET_3				  , 0 , 0},
	{ 439 ,"XPON_PMA_DA_XPON_TX_FORCE_0"				 ,XPON_PMA_DA_XPON_TX_FORCE_0				  , 0 , 0},
	{ 440 ,"XPON_PMA_DA_XPON_TX_FORCE_1"				 ,XPON_PMA_DA_XPON_TX_FORCE_1				  , 0 , 0},
	{ 441 ,"XPON_PMA_DA_XPON_TX_FORCE_2"				 ,XPON_PMA_DA_XPON_TX_FORCE_2				  , 0 , 0},
	{ 442 ,"XPON_PMA_RX_FORCE_MODE_10"					 ,XPON_PMA_RX_FORCE_MODE_10 				  , 0 , 0},
	{ 443 ,"XPON_PMA_ADD_CLKPATH_RST_0" 				 ,XPON_PMA_ADD_CLKPATH_RST_0				  , 0 , 0},
	{ 444 ,"XPON_PMA_ADD_XPON_MODE_1"					 ,XPON_PMA_ADD_XPON_MODE_1					  , 0 , 0},
	{ 445 ,"XPON_PMA_ADD_R2T_MODE_1"					 ,XPON_PMA_ADD_R2T_MODE_1					  , 0 , 0},
	{ 446 ,"XPON_PMA_ADD_T2R_MODE_1"					 ,XPON_PMA_ADD_T2R_MODE_1					  , 0 , 0},
	{ 447 ,"XPON_PMA_ADD_LCPLL_RO_1"					 ,XPON_PMA_ADD_LCPLL_RO_1					  , 0 , 0},
	{ 448 ,"XPON_PMA_ADD_RO_RX2ANA_1"					 ,XPON_PMA_ADD_RO_RX2ANA_1					  , 0 , 0},
	{ 449 ,"XPON_PMA_ADD_RO_RX2ANA_2"					 ,XPON_PMA_ADD_RO_RX2ANA_2					  , 0 , 0},
	{ 450 ,"XPON_PMA_ADD_RO_RX2ANA_3"					 ,XPON_PMA_ADD_RO_RX2ANA_3					  , 0 , 0},
	{ 451 ,"XPON_PMA_ADD_RO_R2TMODE_1"					 ,XPON_PMA_ADD_RO_R2TMODE_1 				  , 0 , 0},
	{ 452 ,"XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_0"		 ,XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_0 	  , 0 , 0},
	{ 453 ,"XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1"		 ,XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1 	  , 0 , 0},
	{ 454 ,"XPON_PMA_RG_TX_HSDATA_EN_EXT_CNT_0" 		 ,XPON_PMA_RG_TX_HSDATA_EN_EXT_CNT_0		  , 0 , 0},
	{ 455 ,"XPON_PMA_PON_TX_COUNTER_0"					 ,XPON_PMA_PON_TX_COUNTER_0 				  , 0 , 0},
	{ 456 ,"XPON_PMA_PON_TX_COUNTER_1"					 ,XPON_PMA_PON_TX_COUNTER_1 				  , 0 , 0},
	{ 457 ,"XPON_PMA_PON_TX_COUNTER_2"					 ,XPON_PMA_PON_TX_COUNTER_2 				  , 0 , 0},
	{ 458 ,"XPON_PMA_PON_TX_COUNTER_3"					 ,XPON_PMA_PON_TX_COUNTER_3 				  , 0 , 0},
	{ 459 ,"XFI_PMA_RG_XFI_XTALCK_CGM_EN_0" 			 ,XFI_PMA_RG_XFI_XTALCK_CGM_EN_0			  , 0 , 0},
	{ 460 ,"XFI_PMA_RG_JCPLL_IC_0"						 ,XFI_PMA_RG_JCPLL_IC_0 					  , 0 , 0},
	{ 461 ,"XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0" 			 ,XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0			  , 0 , 0},
	{ 462 ,"XFI_PMA_RG_JCPLL_SDM_ORD_0" 				 ,XFI_PMA_RG_JCPLL_SDM_ORD_0				  , 0 , 0},
	{ 463 ,"XFI_PMA_RG_JCPLL_RESERVE0_0"				 ,XFI_PMA_RG_JCPLL_RESERVE0_0				  , 0 , 0},
	{ 464 ,"XFI_PMA_RGS_JCPLL_AUTOK_BAND_0" 			 ,XFI_PMA_RGS_JCPLL_AUTOK_BAND_0			  , 0 , 0},
	{ 465 ,"XFI_PMA_RG_XFI_CMN_CUREN_0" 				 ,XFI_PMA_RG_XFI_CMN_CUREN_0				  , 0 , 0},
	{ 466 ,"XFI_PMA_RG_XFI_CLKPATH_LDO_EN_0"			 ,XFI_PMA_RG_XFI_CLKPATH_LDO_EN_0			  , 0 , 0},
	{ 467 ,"XFI_PMA_RG_XFI_TOP_REV_0_0" 				 ,XFI_PMA_RG_XFI_TOP_REV_0_0				  , 0 , 0},
	{ 468 ,"XFI_PMA_RG_XFI_RX_MPXSEL_0" 				 ,XFI_PMA_RG_XFI_RX_MPXSEL_0				  , 0 , 0},
	{ 469 ,"XFI_PMA_RG_XFI_RX_REV_0_0"					 ,XFI_PMA_RG_XFI_RX_REV_0_0 				  , 0 , 0},
	{ 470 ,"XFI_PMA_RG_XFI_RX_PHYCK_DIV_0"				 ,XFI_PMA_RG_XFI_RX_PHYCK_DIV_0 			  , 0 , 0},
	{ 471 ,"XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0"			 ,XFI_PMA_RG_XFI_CDR_LPF_KI_GAIN_0			  , 0 , 0},
	{ 472 ,"XFI_PMA_RG_XFI_CDR_LPF_TOP_LIM_0"			 ,XFI_PMA_RG_XFI_CDR_LPF_TOP_LIM_0			  , 0 , 0},
	{ 473 ,"XFI_PMA_RG_XFI_CDR_LPF_SETVALUE_0"			 ,XFI_PMA_RG_XFI_CDR_LPF_SETVALUE_0 		  , 0 , 0},
	{ 474 ,"XFI_PMA_RG_XFI_CDR_PR_INJ_FORCE_OFF_0"		 ,XFI_PMA_RG_XFI_CDR_PR_INJ_FORCE_OFF_0 	  , 0 , 0},
	{ 475 ,"XFI_PMA_RG_XFI_CDR_PR_CKREF_DIV_0"			 ,XFI_PMA_RG_XFI_CDR_PR_CKREF_DIV_0 		  , 0 , 0},
	{ 476 ,"XFI_PMA_RG_XFI_CDR_PR_RESERVE0_0"			 ,XFI_PMA_RG_XFI_CDR_PR_RESERVE0_0			  , 0 , 0},
	{ 477 ,"XFI_PMA_RG_XFI_RX_DAC_RANGE_EYE_0"			 ,XFI_PMA_RG_XFI_RX_DAC_RANGE_EYE_0 		  , 0 , 0},
	{ 478 ,"XFI_PMA_RG_XFI_TXPLL1_IPATH_LOAD_KBAND_EN_0" ,XFI_PMA_RG_XFI_TXPLL1_IPATH_LOAD_KBAND_EN_0 , 0 , 0},
	{ 479 ,"XFI_PMA_RG_XFI_TXPLL1_BR_0" 				 ,XFI_PMA_RG_XFI_TXPLL1_BR_0				  , 0 , 0},
	{ 480 ,"XFI_PMA_RG_XFI_TXPLL1_DAC_BAND_0"			 ,XFI_PMA_RG_XFI_TXPLL1_DAC_BAND_0			  , 0 , 0},
	{ 481 ,"XFI_PMA_RG_XFI_TXPLL1_DDS_MONEN_0"			 ,XFI_PMA_RG_XFI_TXPLL1_DDS_MONEN_0 		  , 0 , 0},
	{ 482 ,"XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_DELTA_0"		 ,XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_DELTA_0 	  , 0 , 0},
	{ 483 ,"XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_EN_0" 		 ,XFI_PMA_RG_XFI_TXPLL1_DDS_SSC_EN_0		  , 0 , 0},
	{ 484 ,"XFI_PMA_RGS_XFI_TXPLL1_SDM_TD_0"			 ,XFI_PMA_RGS_XFI_TXPLL1_SDM_TD_0			  , 0 , 0},
	{ 485 ,"XFI_PMA_RG_XFI_TX_EN_0" 					 ,XFI_PMA_RG_XFI_TX_EN_0					  , 0 , 0},
	{ 486 ,"XFI_PMA_RG_XFI_TX_LSDATA_EN_0"				 ,XFI_PMA_RG_XFI_TX_LSDATA_EN_0 			  , 0 , 0},
	{ 487 ,"XFI_PMA_RG_XFI_TX_RESERVED_0"				 ,XFI_PMA_RG_XFI_TX_RESERVED_0				  , 0 , 0},
	{ 488 ,"XFI_PMA_SS_TX_DA_1" 						 ,XFI_PMA_SS_TX_DA_1						  , 0 , 0},
	{ 489 ,"XFI_PMA_SS_LCPLL_PWCTL_SETTING_0"			 ,XFI_PMA_SS_LCPLL_PWCTL_SETTING_0			  , 0 , 0},
	{ 490 ,"XFI_PMA_SS_LCPLL_PWCTL_SETTING_1"			 ,XFI_PMA_SS_LCPLL_PWCTL_SETTING_1			  , 0 , 0},
	{ 491 ,"XFI_PMA_SS_LCPLL_PWCTL_SETTING_2"			 ,XFI_PMA_SS_LCPLL_PWCTL_SETTING_2			  , 0 , 0},
	{ 492 ,"XFI_PMA_SS_LCPLL_PWCTL_DBG_SETTING" 		 ,XFI_PMA_SS_LCPLL_PWCTL_DBG_SETTING		  , 0 , 0},
	{ 493 ,"XFI_PMA_SS_LCPLL_TDC_PW_0"					 ,XFI_PMA_SS_LCPLL_TDC_PW_0 				  , 0 , 0},
	{ 494 ,"XFI_PMA_SS_LCPLL_TDC_PW_1"					 ,XFI_PMA_SS_LCPLL_TDC_PW_1 				  , 0 , 0},
	{ 495 ,"XFI_PMA_SS_LCPLL_TDC_PW_2"					 ,XFI_PMA_SS_LCPLL_TDC_PW_2 				  , 0 , 0},
	{ 496 ,"XFI_PMA_SS_LCPLL_TDC_PW_3"					 ,XFI_PMA_SS_LCPLL_TDC_PW_3 				  , 0 , 0},
	{ 497 ,"XFI_PMA_SS_LCPLL_TDC_PW_4"					 ,XFI_PMA_SS_LCPLL_TDC_PW_4 				  , 0 , 0},
	{ 498 ,"XFI_PMA_SS_LCPLL_TDC_PW_5"					 ,XFI_PMA_SS_LCPLL_TDC_PW_5 				  , 0 , 0},
	{ 499 ,"XFI_PMA_SS_LCPLL_TDC_FLT_0" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_0				  , 0 , 0},
	{ 500 ,"XFI_PMA_SS_LCPLL_TDC_FLT_1" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_1				  , 0 , 0},
	{ 501 ,"XFI_PMA_SS_LCPLL_TDC_FLT_2" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_2				  , 0 , 0},
	{ 502 ,"XFI_PMA_SS_LCPLL_TDC_FLT_3" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_3				  , 0 , 0},
	{ 503 ,"XFI_PMA_SS_LCPLL_TDC_FLT_4" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_4				  , 0 , 0},
	{ 504 ,"XFI_PMA_SS_LCPLL_TDC_FLT_5" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_5				  , 0 , 0},
	{ 505 ,"XFI_PMA_SS_LCPLL_TDC_FLT_6" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_6				  , 0 , 0},
	{ 506 ,"XFI_PMA_SS_LCPLL_TDC_FLT_7" 				 ,XFI_PMA_SS_LCPLL_TDC_FLT_7				  , 0 , 0},
	{ 507 ,"XFI_PMA_SS_LCPLL_TDC_PCW_1" 				 ,XFI_PMA_SS_LCPLL_TDC_PCW_1				  , 0 , 0},
	{ 508 ,"XFI_PMA_SS_LCPLL_TDC_PCW_2" 				 ,XFI_PMA_SS_LCPLL_TDC_PCW_2				  , 0 , 0},
	{ 509 ,"XFI_PMA_SS_LCPLL_TDC_RO_1"					 ,XFI_PMA_SS_LCPLL_TDC_RO_1 				  , 0 , 0},
	{ 510 ,"XFI_PMA_SS_LCPLL_TDC_RO_2"					 ,XFI_PMA_SS_LCPLL_TDC_RO_2 				  , 0 , 0},
	{ 511 ,"XFI_PMA_SS_LCPLL_TDC_RO_3"					 ,XFI_PMA_SS_LCPLL_TDC_RO_3 				  , 0 , 0},
	{ 512 ,"XFI_PMA_SS_LCPLL_TDC_RO_4"					 ,XFI_PMA_SS_LCPLL_TDC_RO_4 				  , 0 , 0},
	{ 513 ,"XFI_PMA_SS_LCPLL_TDC_RO_5"					 ,XFI_PMA_SS_LCPLL_TDC_RO_5 				  , 0 , 0},
	{ 514 ,"XFI_PMA_SS_LCPLL_TDC_CTRL_0"				 ,XFI_PMA_SS_LCPLL_TDC_CTRL_0				  , 0 , 0},
	{ 515 ,"XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0" 		 ,XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0		  , 0 , 0},
	{ 516 ,"XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1" 		 ,XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1		  , 0 , 0},
	{ 517 ,"XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2" 		 ,XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2		  , 0 , 0},
	{ 518 ,"XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3" 		 ,XFI_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3		  , 0 , 0},
	{ 519 ,"XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0"		 ,XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0		  , 0 , 0},
	{ 520 ,"XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1"		 ,XFI_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1		  , 0 , 0},
	{ 521 ,"XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0"			 ,XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_0			  , 0 , 0},
	{ 522 ,"XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1"			 ,XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_1			  , 0 , 0},
	{ 523 ,"XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2"			 ,XFI_PMA_RX_EYE_TOP_EYECNT_CTRL_2			  , 0 , 0},
	{ 524 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_0			  , 0 , 0},
	{ 525 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_1			  , 0 , 0},
	{ 526 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_2			  , 0 , 0},
	{ 527 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_3			  , 0 , 0},
	{ 528 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_4			  , 0 , 0},
	{ 529 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_5			  , 0 , 0},
	{ 530 ,"XFI_PMA_RX_CTRL_SEQUENCE_CTRL_6"			 ,XFI_PMA_RX_CTRL_SEQUENCE_CTRL_6			  , 0 , 0},
	{ 531 ,"XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0"		 ,XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0		  , 0 , 0},
	{ 532 ,"XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1"		 ,XFI_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1		  , 0 , 0},
	{ 533 ,"XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0"		 ,XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0 	  , 0 , 0},
	{ 534 ,"XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1"		 ,XFI_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1 	  , 0 , 0},
	{ 535 ,"XFI_PMA_PHY_EQ_CTRL_0"						 ,XFI_PMA_PHY_EQ_CTRL_0 					  , 0 , 0},
	{ 536 ,"XFI_PMA_PHY_EQ_CTRL_1"						 ,XFI_PMA_PHY_EQ_CTRL_1 					  , 0 , 0},
	{ 537 ,"XFI_PMA_PHY_EQ_CTRL_2"						 ,XFI_PMA_PHY_EQ_CTRL_2 					  , 0 , 0},
	{ 538 ,"XFI_PMA_PHY_EQ_CTRL_3"						 ,XFI_PMA_PHY_EQ_CTRL_3 					  , 0 , 0},
	{ 539 ,"XFI_PMA_PHY_EQ_CTRL_4"						 ,XFI_PMA_PHY_EQ_CTRL_4 					  , 0 , 0},
	{ 540 ,"XFI_PMA_PHY_EQ_CTRL_5"						 ,XFI_PMA_PHY_EQ_CTRL_5 					  , 0 , 0},
	{ 541 ,"XFI_PMA_PHY_EQ_CTRL_6"						 ,XFI_PMA_PHY_EQ_CTRL_6 					  , 0 , 0},
	{ 542 ,"XFI_PMA_PHY_EQ_CTRL_7"						 ,XFI_PMA_PHY_EQ_CTRL_7 					  , 0 , 0},
	{ 543 ,"XFI_PMA_PHY_EQ_CTRL_8"						 ,XFI_PMA_PHY_EQ_CTRL_8 					  , 0 , 0},
	{ 544 ,"XFI_PMA_PHY_EQ_CTRL_9"						 ,XFI_PMA_PHY_EQ_CTRL_9 					  , 0 , 0},
	{ 545 ,"XFI_PMA_PHY_EQ_CTRL_10" 					 ,XFI_PMA_PHY_EQ_CTRL_10					  , 0 , 0},
	{ 546 ,"XFI_PMA_SS_RX_FEOS" 						 ,XFI_PMA_SS_RX_FEOS						  , 0 , 0},
	{ 547 ,"XFI_PMA_SS_RX_BLWC" 						 ,XFI_PMA_SS_RX_BLWC						  , 0 , 0},
	{ 548 ,"XFI_PMA_SS_RX_FREQ_DET_1"					 ,XFI_PMA_SS_RX_FREQ_DET_1					  , 0 , 0},
	{ 549 ,"XFI_PMA_SS_RX_FREQ_DET_2"					 ,XFI_PMA_SS_RX_FREQ_DET_2					  , 0 , 0},
	{ 550 ,"XFI_PMA_SS_RX_FREQ_DET_3"					 ,XFI_PMA_SS_RX_FREQ_DET_3					  , 0 , 0},
	{ 551 ,"XFI_PMA_SS_RX_FREQ_DET_4"					 ,XFI_PMA_SS_RX_FREQ_DET_4					  , 0 , 0},
	{ 552 ,"XFI_PMA_SS_RX_PI_CAL"						 ,XFI_PMA_SS_RX_PI_CAL						  , 0 , 0},
	{ 553 ,"XFI_PMA_SS_RX_CAL_1"						 ,XFI_PMA_SS_RX_CAL_1						  , 0 , 0},
	{ 554 ,"XFI_PMA_SS_RX_CAL_2"						 ,XFI_PMA_SS_RX_CAL_2						  , 0 , 0},
	{ 555 ,"XFI_PMA_SS_RX_SIGDET_0" 					 ,XFI_PMA_SS_RX_SIGDET_0					  , 0 , 0},
	{ 556 ,"XFI_PMA_SS_RX_SIGDET_1" 					 ,XFI_PMA_SS_RX_SIGDET_1					  , 0 , 0},
	{ 557 ,"XFI_PMA_SS_RX_FLL_0"						 ,XFI_PMA_SS_RX_FLL_0						  , 0 , 0},
	{ 558 ,"XFI_PMA_SS_RX_FLL_1"						 ,XFI_PMA_SS_RX_FLL_1						  , 0 , 0},
	{ 559 ,"XFI_PMA_SS_RX_FLL_2"						 ,XFI_PMA_SS_RX_FLL_2						  , 0 , 0},
	{ 560 ,"XFI_PMA_SS_RX_FLL_3"						 ,XFI_PMA_SS_RX_FLL_3						  , 0 , 0},
	{ 561 ,"XFI_PMA_SS_RX_FLL_4"						 ,XFI_PMA_SS_RX_FLL_4						  , 0 , 0},
	{ 562 ,"XFI_PMA_SS_RX_FLL_5"						 ,XFI_PMA_SS_RX_FLL_5						  , 0 , 0},
	{ 563 ,"XFI_PMA_SS_RX_FLL_6"						 ,XFI_PMA_SS_RX_FLL_6						  , 0 , 0},
	{ 564 ,"XFI_PMA_SS_RX_FLL_7"						 ,XFI_PMA_SS_RX_FLL_7						  , 0 , 0},
	{ 565 ,"XFI_PMA_SS_RX_FLL_8"						 ,XFI_PMA_SS_RX_FLL_8						  , 0 , 0},
	{ 566 ,"XFI_PMA_SS_RX_FLL_9"						 ,XFI_PMA_SS_RX_FLL_9						  , 0 , 0},
	{ 567 ,"XFI_PMA_SS_RX_FLL_a"						 ,XFI_PMA_SS_RX_FLL_a						  , 0 , 0},
	{ 568 ,"XFI_PMA_SS_RX_FLL_b"						 ,XFI_PMA_SS_RX_FLL_b						  , 0 , 0},
	{ 569 ,"XFI_PMA_RX_PDOS_CTRL_0" 					 ,XFI_PMA_RX_PDOS_CTRL_0					  , 0 , 0},
	{ 570 ,"XFI_PMA_RX_RESET_0" 						 ,XFI_PMA_RX_RESET_0						  , 0 , 0},
	{ 571 ,"XFI_PMA_RX_RESET_1" 						 ,XFI_PMA_RX_RESET_1						  , 0 , 0},
	{ 572 ,"XFI_PMA_RX_DEBUG_0" 						 ,XFI_PMA_RX_DEBUG_0						  , 0 , 0},
	{ 573 ,"XFI_PMA_BISTCTL_CONTROL"					 ,XFI_PMA_BISTCTL_CONTROL					  , 0 , 0},
	{ 574 ,"XFI_PMA_BISTCTL_ALIGN_PAT"					 ,XFI_PMA_BISTCTL_ALIGN_PAT 				  , 0 , 0},
	{ 575 ,"XFI_PMA_BISTCTL_PROGRAM_PAT_0"				 ,XFI_PMA_BISTCTL_PROGRAM_PAT_0 			  , 0 , 0},
	{ 576 ,"XFI_PMA_BISTCTL_PROGRAM_PAT_1"				 ,XFI_PMA_BISTCTL_PROGRAM_PAT_1 			  , 0 , 0},
	{ 577 ,"XFI_PMA_BISTCTL_PROGRAM_PAT_2"				 ,XFI_PMA_BISTCTL_PROGRAM_PAT_2 			  , 0 , 0},
	{ 578 ,"XFI_PMA_BISTCTL_PRBS_INITIAL_SEED"			 ,XFI_PMA_BISTCTL_PRBS_INITIAL_SEED 		  , 0 , 0},
	{ 579 ,"XFI_PMA_BISTCTL_PRBS_EVENT" 				 ,XFI_PMA_BISTCTL_PRBS_EVENT				  , 0 , 0},
	{ 580 ,"XFI_PMA_BISTCTL_PRBS_ERRCNT"				 ,XFI_PMA_BISTCTL_PRBS_ERRCNT				  , 0 , 0},
	{ 581 ,"XFI_PMA_BISTCTL_PRBS_FAIL_THRESHOLD"		 ,XFI_PMA_BISTCTL_PRBS_FAIL_THRESHOLD		  , 0 , 0},
	{ 582 ,"XFI_PMA_RX_TORGS_DEBUG_0"					 ,XFI_PMA_RX_TORGS_DEBUG_0					  , 0 , 0},
	{ 583 ,"XFI_PMA_RX_TORGS_DEBUG_1"					 ,XFI_PMA_RX_TORGS_DEBUG_1					  , 0 , 0},
	{ 584 ,"XFI_PMA_RX_TORGS_DEBUG_2"					 ,XFI_PMA_RX_TORGS_DEBUG_2					  , 0 , 0},
	{ 585 ,"XFI_PMA_RX_TORGS_DEBUG_3"					 ,XFI_PMA_RX_TORGS_DEBUG_3					  , 0 , 0},
	{ 586 ,"XFI_PMA_RX_TORGS_DEBUG_4"					 ,XFI_PMA_RX_TORGS_DEBUG_4					  , 0 , 0},
	{ 587 ,"XFI_PMA_RX_TORGS_DEBUG_5"					 ,XFI_PMA_RX_TORGS_DEBUG_5					  , 0 , 0},
	{ 588 ,"XFI_PMA_RX_TORGS_DEBUG_6"					 ,XFI_PMA_RX_TORGS_DEBUG_6					  , 0 , 0},
	{ 589 ,"XFI_PMA_RX_TORGS_DEBUG_7"					 ,XFI_PMA_RX_TORGS_DEBUG_7					  , 0 , 0},
	{ 590 ,"XFI_PMA_RX_TORGS_DEBUG_8"					 ,XFI_PMA_RX_TORGS_DEBUG_8					  , 0 , 0},
	{ 591 ,"XFI_PMA_RX_TORGS_DEBUG_9"					 ,XFI_PMA_RX_TORGS_DEBUG_9					  , 0 , 0},
	{ 592 ,"XFI_PMA_RX_TORGS_DEBUG_10"					 ,XFI_PMA_RX_TORGS_DEBUG_10 				  , 0 , 0},
	{ 593 ,"XFI_PMA_SS_TX_RST_B"						 ,XFI_PMA_SS_TX_RST_B						  , 0 , 0},
	{ 594 ,"XFI_PMA_SS_TX_CALIB_0"						 ,XFI_PMA_SS_TX_CALIB_0 					  , 0 , 0},
	{ 595 ,"XFI_PMA_SS_TX_CALIB_1"						 ,XFI_PMA_SS_TX_CALIB_1 					  , 0 , 0},
	{ 596 ,"XFI_PMA_SS_TX_CALIB_2"						 ,XFI_PMA_SS_TX_CALIB_2 					  , 0 , 0},
	{ 597 ,"XFI_PMA_XFI_SETTING_0"						 ,XFI_PMA_XFI_SETTING_0 					  , 0 , 0},
	{ 598 ,"XFI_PMA_XFI_SETTING_1"						 ,XFI_PMA_XFI_SETTING_1 					  , 0 , 0},
	{ 599 ,"XFI_PMA_XFI_SETTING_2"						 ,XFI_PMA_XFI_SETTING_2 					  , 0 , 0},
	{ 600 ,"XFI_PMA_XFI_STA"							 ,XFI_PMA_XFI_STA							  , 0 , 0},
	{ 601 ,"XFI_PMA_XFI_INT_EN_0"						 ,XFI_PMA_XFI_INT_EN_0						  , 0 , 0},
	{ 602 ,"XFI_PMA_XFI_INT_EN_1"						 ,XFI_PMA_XFI_INT_EN_1						  , 0 , 0},
	{ 603 ,"XFI_PMA_XFI_INT_STA_CLR_0"					 ,XFI_PMA_XFI_INT_STA_CLR_0 				  , 0 , 0},
	{ 604 ,"XFI_PMA_XFI_INT_STA_CLR_1"					 ,XFI_PMA_XFI_INT_STA_CLR_1 				  , 0 , 0},
	{ 605 ,"XFI_PMA_XFI_INT_STA_0"						 ,XFI_PMA_XFI_INT_STA_0 					  , 0 , 0},
	{ 606 ,"XFI_PMA_XFI_INT_STA_1"						 ,XFI_PMA_XFI_INT_STA_1 					  , 0 , 0},
	{ 607 ,"XFI_PMA_RX_TORGS_DEBUG_11"					 ,XFI_PMA_RX_TORGS_DEBUG_11 				  , 0 , 0},
	{ 608 ,"XFI_PMA_RX_FORCE_MODE_0"					 ,XFI_PMA_RX_FORCE_MODE_0					  , 0 , 0},
	{ 609 ,"XFI_PMA_RX_FORCE_MODE_1"					 ,XFI_PMA_RX_FORCE_MODE_1					  , 0 , 0},
	{ 610 ,"XFI_PMA_RX_FORCE_MODE_2"					 ,XFI_PMA_RX_FORCE_MODE_2					  , 0 , 0},
	{ 611 ,"XFI_PMA_RX_DISB_MODE_0" 					 ,XFI_PMA_RX_DISB_MODE_0					  , 0 , 0},
	{ 612 ,"XFI_PMA_RX_DISB_MODE_1" 					 ,XFI_PMA_RX_DISB_MODE_1					  , 0 , 0},
	{ 613 ,"XFI_PMA_RX_DISB_MODE_2" 					 ,XFI_PMA_RX_DISB_MODE_2					  , 0 , 0},
	{ 614 ,"XFI_PMA_RX_FORCE_MODE_3"					 ,XFI_PMA_RX_FORCE_MODE_3					  , 0 , 0},
	{ 615 ,"XFI_PMA_RX_FORCE_MODE_4"					 ,XFI_PMA_RX_FORCE_MODE_4					  , 0 , 0},
	{ 616 ,"XFI_PMA_RX_FORCE_MODE_5"					 ,XFI_PMA_RX_FORCE_MODE_5					  , 0 , 0},
	{ 617 ,"XFI_PMA_RX_FORCE_MODE_6"					 ,XFI_PMA_RX_FORCE_MODE_6					  , 0 , 0},
	{ 618 ,"XFI_PMA_RX_DISB_MODE_3" 					 ,XFI_PMA_RX_DISB_MODE_3					  , 0 , 0},
	{ 619 ,"XFI_PMA_RX_DISB_MODE_4" 					 ,XFI_PMA_RX_DISB_MODE_4					  , 0 , 0},
	{ 620 ,"XFI_PMA_RX_DISB_MODE_5" 					 ,XFI_PMA_RX_DISB_MODE_5					  , 0 , 0},
	{ 621 ,"XFI_PMA_RX_FORCE_MODE_7"					 ,XFI_PMA_RX_FORCE_MODE_7					  , 0 , 0},
	{ 622 ,"XFI_PMA_RX_FORCE_MODE_8"					 ,XFI_PMA_RX_FORCE_MODE_8					  , 0 , 0},
	{ 623 ,"XFI_PMA_RX_FORCE_MODE_9"					 ,XFI_PMA_RX_FORCE_MODE_9					  , 0 , 0},
	{ 624 ,"XFI_PMA_RX_DISB_MODE_6" 					 ,XFI_PMA_RX_DISB_MODE_6					  , 0 , 0},
	{ 625 ,"XFI_PMA_RX_DISB_MODE_7" 					 ,XFI_PMA_RX_DISB_MODE_7					  , 0 , 0},
	{ 626 ,"XFI_PMA_RX_DISB_MODE_8" 					 ,XFI_PMA_RX_DISB_MODE_8					  , 0 , 0},
	{ 627 ,"XFI_PMA_SS_BIST_0"							 ,XFI_PMA_SS_BIST_0 						  , 0 , 0},
	{ 628 ,"XFI_PMA_SS_BIST_1"							 ,XFI_PMA_SS_BIST_1 						  , 0 , 0},
	{ 629 ,"XFI_PMA_SS_BIST_2"							 ,XFI_PMA_SS_BIST_2 						  , 0 , 0},
	{ 630 ,"XFI_PMA_SS_DA_XFI_PWDB_0"					 ,XFI_PMA_SS_DA_XFI_PWDB_0					  , 0 , 0},
	{ 631 ,"XFI_PMA_SS_DA_XFI_PWDB_1"					 ,XFI_PMA_SS_DA_XFI_PWDB_1					  , 0 , 0},
	{ 632 ,"XFI_PMA_SS_LCPLL_0" 						 ,XFI_PMA_SS_LCPLL_0						  , 0 , 0},
	{ 633 ,"XFI_PMA_SS_LCPLL_1" 						 ,XFI_PMA_SS_LCPLL_1						  , 0 , 0},
	{ 634 ,"XFI_PMA_SS_LCPLL_2" 						 ,XFI_PMA_SS_LCPLL_2						  , 0 , 0},
	{ 635 ,"XFI_PMA_ADD_DIG_RESERVE_0"					 ,XFI_PMA_ADD_DIG_RESERVE_0 				  , 0 , 0},
	{ 636 ,"XFI_PMA_ADD_DIG_RESERVE_1"					 ,XFI_PMA_ADD_DIG_RESERVE_1 				  , 0 , 0},
	{ 637 ,"XFI_PMA_ADD_DIG_RESERVE_2"					 ,XFI_PMA_ADD_DIG_RESERVE_2 				  , 0 , 0},
	{ 638 ,"XFI_PMA_ADD_DIG_RESERVE_3"					 ,XFI_PMA_ADD_DIG_RESERVE_3 				  , 0 , 0},
	{ 639 ,"XFI_PMA_ADD_DIG_RESERVE_4"					 ,XFI_PMA_ADD_DIG_RESERVE_4 				  , 0 , 0},
	{ 640 ,"XFI_PMA_RG_XFI_RX_RESERVED_1"				 ,XFI_PMA_RG_XFI_RX_RESERVED_1				  , 0 , 0},
	{ 641 ,"XFI_PMA_ADD_DIG_RO_RESERVE_0"				 ,XFI_PMA_ADD_DIG_RO_RESERVE_0				  , 0 , 0},
	{ 642 ,"XFI_PMA_ADD_DIG_RO_RESERVE_1"				 ,XFI_PMA_ADD_DIG_RO_RESERVE_1				  , 0 , 0},
	{ 643 ,"XFI_PMA_ADD_DIG_RO_RESERVE_2"				 ,XFI_PMA_ADD_DIG_RO_RESERVE_2				  , 0 , 0},
	{ 644 ,"XFI_PMA_ADD_DIG_RO_RESERVE_3"				 ,XFI_PMA_ADD_DIG_RO_RESERVE_3				  , 0 , 0},
	{ 645 ,"XFI_PMA_ADD_DIG_RO_RESERVE_4"				 ,XFI_PMA_ADD_DIG_RO_RESERVE_4				  , 0 , 0},
	{ 646 ,"XFI_PMA_ADD_RX_SYS_EN_SEL_0"				 ,XFI_PMA_ADD_RX_SYS_EN_SEL_0				  , 0 , 0},
	{ 647 ,"XFI_PMA_PLL_TDC_FREQDET_0"					 ,XFI_PMA_PLL_TDC_FREQDET_0 				  , 0 , 0},
	{ 648 ,"XFI_PMA_PLL_TDC_FREQDET_1"					 ,XFI_PMA_PLL_TDC_FREQDET_1 				  , 0 , 0},
	{ 649 ,"XFI_PMA_PLL_TDC_FREQDET_2"					 ,XFI_PMA_PLL_TDC_FREQDET_2 				  , 0 , 0},
	{ 650 ,"XFI_PMA_PLL_TDC_FREQDET_3"					 ,XFI_PMA_PLL_TDC_FREQDET_3 				  , 0 , 0},
	{ 651 ,"XFI_PMA_DA_XFI_TX_FORCE_0"					 ,XFI_PMA_DA_XFI_TX_FORCE_0 				  , 0 , 0},
	{ 652 ,"XFI_PMA_DA_XFI_TX_FORCE_1"					 ,XFI_PMA_DA_XFI_TX_FORCE_1 				  , 0 , 0},
	{ 653 ,"XFI_PMA_DA_XFI_TX_FORCE_2"					 ,XFI_PMA_DA_XFI_TX_FORCE_2 				  , 0 , 0},
	{ 654 ,"XFI_PMA_RX_FORCE_MODE_10"					 ,XFI_PMA_RX_FORCE_MODE_10					  , 0 , 0},
	{ 655 ,"XFI_PMA_ADD_CLKPATH_RST_0"					 ,XFI_PMA_ADD_CLKPATH_RST_0 				  , 0 , 0},
	{ 656 ,"XFI_PMA_ADD_XFI_MODE_1" 					 ,XFI_PMA_ADD_XFI_MODE_1					  , 0 , 0},
	{ 657 ,"XFI_PMA_ADD_R2T_MODE_1" 					 ,XFI_PMA_ADD_R2T_MODE_1					  , 0 , 0},
	{ 658 ,"XFI_PMA_ADD_T2R_MODE_1" 					 ,XFI_PMA_ADD_T2R_MODE_1					  , 0 , 0},
	{ 659 ,"XFI_PMA_ADD_LCPLL_RO_1" 					 ,XFI_PMA_ADD_LCPLL_RO_1					  , 0 , 0},
	{ 660 ,"XFI_PMA_ADD_RO_RX2ANA_1"					 ,XFI_PMA_ADD_RO_RX2ANA_1					  , 0 , 0},
	{ 661 ,"XFI_PMA_ADD_RO_RX2ANA_2"					 ,XFI_PMA_ADD_RO_RX2ANA_2					  , 0 , 0},
	{ 662 ,"XFI_PMA_ADD_RO_RX2ANA_3"					 ,XFI_PMA_ADD_RO_RX2ANA_3					  , 0 , 0},
	{ 663 ,"XFI_PMA_ADD_RO_R2TMODE_1"					 ,XFI_PMA_ADD_RO_R2TMODE_1					  , 0 , 0},
	{ 664 ,"XFI_PMA_SS_JCPLL_PWCTL_SETTING_0"			 ,XFI_PMA_SS_JCPLL_PWCTL_SETTING_0			  , 0 , 0},
	{ 665 ,"XFI_PMA_SS_JCPLL_PWCTL_SETTING_1"			 ,XFI_PMA_SS_JCPLL_PWCTL_SETTING_1			  , 0 , 0},
	{ 666 ,"XFI_PMA_SS_JCPLL_TDC_FLT_2" 				 ,XFI_PMA_SS_JCPLL_TDC_FLT_2				  , 0 , 0},
	{ 667 ,"XFI_PMA_JCPLL_CHG"							 ,XFI_PMA_JCPLL_CHG 						  , 0 , 0},
	{ 668 ,"XFI_PMA_RG_JCPLL_SDM_PCW_CTRL"				 ,XFI_PMA_RG_JCPLL_SDM_PCW_CTRL 			  , 0 , 0},
	{ 669 ,"XFI_PMA_RG_JCPLL_SDM_PCW_CHG_CTRL"			 ,XFI_PMA_RG_JCPLL_SDM_PCW_CHG_CTRL 		  , 0 , 0},
	{ 670 ,"XFI_PMA_RG_JCPLL_XFI_CMN_STB_CTRL"			 ,XFI_PMA_RG_JCPLL_XFI_CMN_STB_CTRL 		  , 0 , 0},
	{ 671 ,"XFI_PMA_PLL_TDC_TX_FREQDET_0"				 ,XFI_PMA_PLL_TDC_TX_FREQDET_0				  , 0 , 0},
	{ 672 ,"XFI_PMA_PLL_TDC_TX_FREQDET_1"				 ,XFI_PMA_PLL_TDC_TX_FREQDET_1				  , 0 , 0},
	{ 673 ,"XFI_PMA_PLL_TDC_TX_FREQDET_2"				 ,XFI_PMA_PLL_TDC_TX_FREQDET_2				  , 0 , 0},
	{ 674 ,"XFI_PMA_PLL_TDC_TX_FREQDET_3"				 ,XFI_PMA_PLL_TDC_TX_FREQDET_3				  , 0 , 0},
	{ 675 ,"XFI_PMA_XFI_IPATH_CTRL_0"					 ,XFI_PMA_XFI_IPATH_CTRL_0					  , 0 , 0},
	{ 676 ,"XFI_PMA_XFI_IPATH_CTRL_1"					 ,XFI_PMA_XFI_IPATH_CTRL_1					  , 0 , 0},
	{ 677 ,"XFI_PMA_XFI_IPATH_CTRL_2"					 ,XFI_PMA_XFI_IPATH_CTRL_2					  , 0 , 0},
	{ 678 ,"XFI_PMA_XFI_IPATH_CTRL_3"					 ,XFI_PMA_XFI_IPATH_CTRL_3					  , 0 , 0},
	{ 679 ,"XFI_PMA_XFI_IPATH_CTRL_4"					 ,XFI_PMA_XFI_IPATH_CTRL_4					  , 0 , 0},
	{ 680 ,"XFI_PMA_XFI_IPATH_CTRL_5"					 ,XFI_PMA_XFI_IPATH_CTRL_5					  , 0 , 0},
	{ 681 ,"XFI_PMA_XFI_IPATH_CTRL_6"					 ,XFI_PMA_XFI_IPATH_CTRL_6					  , 0 , 0},
	{ 682 ,"XFI_PMA_PLL_JCPLL_FT_FREQDET_0" 			 ,XFI_PMA_PLL_JCPLL_FT_FREQDET_0			  , 0 , 0},
	{ 683 ,"XFI_PMA_PLL_JCPLL_FT_FREQDET_1" 			 ,XFI_PMA_PLL_JCPLL_FT_FREQDET_1			  , 0 , 0},
	{ 684 ,"XFI_PMA_PLL_JCPLL_FT_FREQDET_2" 			 ,XFI_PMA_PLL_JCPLL_FT_FREQDET_2			  , 0 , 0},
	{ 685 ,"XFI_PMA_PLL_JCPLL_FT_FREQDET_3" 			 ,XFI_PMA_PLL_JCPLL_FT_FREQDET_3			  , 0 , 0},
	{ 686 ,"XFI_PMA_JCPLL_FT_CK_MUX"					 ,XFI_PMA_JCPLL_FT_CK_MUX					  , 0 , 0},
	{ 687 ,"XFI_PMA_JCPLL_DA_RG_CTRL_0" 				 ,XFI_PMA_JCPLL_DA_RG_CTRL_0				  , 0 , 0},
	{ 688 ,"XFI_PMA_JCPLL_DA_RG_CTRL_1" 				 ,XFI_PMA_JCPLL_DA_RG_CTRL_1				  , 0 , 0},
	{ 689 ,"XFI_PMA_JCPLL_DA_RG_CTRL_2" 				 ,XFI_PMA_JCPLL_DA_RG_CTRL_2				  , 0 , 0},
	{ 690 ,"XFI_PMA_XFI_PLL_DA_RG_CTRL_0"				 ,XFI_PMA_XFI_PLL_DA_RG_CTRL_0				  , 0 , 0},
	{ 691 ,"XFI_PMA_XFI_PLL_DA_RG_CTRL_1"				 ,XFI_PMA_XFI_PLL_DA_RG_CTRL_1				  , 0 , 0},
	{ 692 ,"XFI_PMA_XFI_PLL_DA_RG_CTRL_2"				 ,XFI_PMA_XFI_PLL_DA_RG_CTRL_2				  , 0 , 0},
	{ 693 ,"XFI_PMA_AD_DA_PROBE_STS_0"					 ,XFI_PMA_AD_DA_PROBE_STS_0 				  , 0 , 0},
	{ 694 ,"XFI_PMA_AD_DA_PROBE_STS_1"					 ,XFI_PMA_AD_DA_PROBE_STS_1 				  , 0 , 0},
	{ 695 ,"XFI_PMA_JCPLL_SCAN_CTRL"					 ,XFI_PMA_JCPLL_SCAN_CTRL					  , 0 , 0},
	{ 696 ,"XFI_PMA_XFI_TX_COUNTER_0"					 ,XFI_PMA_XFI_TX_COUNTER_0					  , 0 , 0},
	{ 697 ,"XFI_PMA_XFI_TX_COUNTER_1"					 ,XFI_PMA_XFI_TX_COUNTER_1					  , 0 , 0},
	{ 698 ,"XFI_PMA_XFI_TX_COUNTER_2"					 ,XFI_PMA_XFI_TX_COUNTER_2					  , 0 , 0},
	{ 699 ,"XFI_PMA_XFI_TX_COUNTER_3"					 ,XFI_PMA_XFI_TX_COUNTER_3					  , 0 , 0},
	{ 700 ,"XFI_PMA_XFI_GPIO_SEL_0" 					 ,XFI_PMA_XFI_GPIO_SEL_0					  , 0 , 0},
	{ 701 ,"XFI_PMA_TXPLL_SCAN" 						 ,XFI_PMA_TXPLL_SCAN						  , 0 , 0} 
};




uint IO_GPHYA_REG(UINT32 reg_name,UINT32 reg)//Roger_a60972
{
	return (phy_I2C_read_translet(reg));
}
 
uint IO_SPHYA_REG(UINT32 reg_name,UINT32 reg, UINT32 value)//Roger_a60972
{
	phy_I2C_write_translet(reg, value);
	return 0;
}


uint IO_GPHYA_REG_BITS(UINT32 reg_name,UINT32 reg,UINT32 end_index,UINT32 start_index)//Roger_a60972
{
	if((end_index>=start_index)&&(end_index<32))
	{
		if((end_index==31)&&(start_index==0))
		{
			return (IO_GPHYA_REG(reg_name,reg)) ;
		}
		else
		{
			return ((IO_GPHYA_REG(reg_name,reg)>>start_index) & ((1<<(end_index-start_index+1))-1)) ;
		}
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
		return 0;
	}
}

void IO_SPHYA_REG_BITS(UINT32 reg_name,UINT32 reg,UINT32 end_index,UINT32 start_index,UINT32 value) //Roger_a60972
{
	uint data;
	data=IO_GPHYA_REG(reg_name,reg);
	
	if(0xF1FA==reg)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Get REG 0x%x[%d:%d] = 0x%x\r\n",__FUNCTION__,__LINE__,reg,end_index,start_index,data);
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Set REG 0x%x[%d:%d] to value 0x%x\r\n",__FUNCTION__,__LINE__,reg,end_index,start_index,value);
	}

	if((end_index>=start_index)&&(end_index<32))
	{	
		if((end_index==31)&&(start_index==0))
		{
			IO_SPHYA_REG(reg_name,reg,value);
		}
		else
		{
			IO_SPHYA_REG(reg_name,reg,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index))) ;
			if(0xF1FA==reg)
			{
				PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Set REG 0x%x[%d:%d] to value 0x%x\r\n",__FUNCTION__,__LINE__,reg,end_index,start_index,((data & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index)));
				PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Get REG 0x%x[%d:%d] = 0x%x\r\n",__FUNCTION__,__LINE__,reg,end_index,start_index,IO_GPHYA_REG(reg_name,reg));
			}
		}
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
	}
	//mdelay(50);
}

void IO_SPHYA_REG_BITS_TO_SW(UINT32 reg_name,UINT32 reg,UINT32 end_index,UINT32 start_index,UINT32 value,UINT32 *data) //Roger_a60972
{
	if((end_index>=start_index)&&(end_index<32))
	{	
		 (* data)=(((* data) & ~(((1<<(end_index-start_index+1))-1)<<start_index)) | ((value&((1<<(end_index-start_index+1))-1))<<start_index));
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_ERR,"%s end_index=%d	start_index=%d Error!\r\n",__FUNCTION__,end_index,start_index);
	}
}

int a60972_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);


	// 1. XFI CMN //
	xpon_pma_xfi_cmn_init();
	mdelay(10);
	
	// 2. XPON CMN //
	xpon_pma_xpon_cmn_init();
	mdelay(10);

	// 3. XFI JCPLL 7p5g //
	xpon_pma_jcpll_init();
	mdelay(10);
	
	// 4.XPON PLL 10p3125g //
	xpon_pma_lcpll_cmn_init();
	mdelay(10);

	//mode SEL
	xpon_pma_lcpll_mode_setting();

	mdelay(10);
	// 5. PLL EN //
	xpon_pma_lcpll_enable();
	mdelay(10);

	// 6. XPON CLKPATH //
	xpon_pma_clkpath_setting();
	mdelay(10);
	
	// 7. SW_RESET //
	xpon_pma_sw_reset_init();
	mdelay(10);
	
	// 8. XPON TX LDO //
	xpon_pma_tx_ldo_setting();
	mdelay(10);


	// 9. XPON_10G_EPON_10G_10G//
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);


	//AEVB(3_inch) + ISI(2_inch), FIR Setting
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0xa);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x3);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);

	mdelay(10);

	//Mode Select
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);

	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);

	mdelay(10);

	// 1. XPON BIST Pattern//
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);

	mdelay(10);

	// 2. RX set//

	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);

	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);

	
	mdelay(10);

	//CDR LPF

	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);

	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);

	mdelay(10);

//*****************************
//manuall calibration
//*****************************
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);


	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);

	mdelay(10);

	//*****************************
	//CDR L2D
	//*****************************


	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x8ff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);

	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);

	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);

	//new add
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_2,0x0628,24,24,0x1);
	

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}

int a60972_hugo_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);



	/* rate related */
#if 0
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x08400000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x08400000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x08400000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);


	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);

	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
#else
	xpon_pma_lcpll_mode_setting();
	xpon_pma_rx_rate();
	xpon_pma_tx_rate();
#endif
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);

	/* CMN */
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_RX_SYS_EN_SEL_0,0x172c,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_RX_SYS_EN_SEL_0,0x172c,8,8,0x0);
	mdelay(1);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,25,25,0x1);
    IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,24,24,0x1);
	mdelay(1);

	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	mdelay(1);

	IO_SPHYA_REG_BITS(XPON_PMA_ADD_RX_SYS_EN_SEL_0,0x172c,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_RX_SYS_EN_SEL_0,0x172c,8,8,0x1);
	mdelay(1);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	mdelay(1);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);

	//new add
	xpon_pma_trans_ben_active_level();

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}

int a60972_init_for_External_T2R_lpbk(void)
{
	//solve linux 3.18 compile issue
			
			ktime_t time;
			UINT32 usec = 0;	 
			UINT32 msec = 0;  
			UINT32 sec = 0;
		
		
			time = ktime_get();
			
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
			usec = (time.tv.nsec / 1000) % 1000;	 
			msec = (time.tv.nsec / 1000000) % 1000;   
			sec = time.tv.sec ;
	#else
			usec =(UINT32) ktime_to_us(time) % 1000;
			msec =(UINT32) ktime_to_ms(time) % 1000;
			sec  =(UINT32) ktime_to_sec(time);
	#endif


	// initialize a60972 with script for test mode

	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x00000004);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x00000006);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x00000005);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x00000004);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x00000004);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x00000004);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x00000008);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0x000000FF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x00000003);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x00000000);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0x000000FF);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x00000006);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x00000005);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x00000080);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x00000001);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x08400000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,25,25,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,24,24,0x00000001);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x00000021);
	 
	 
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x00000001);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0x000000FF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0x000000FF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x00000001);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00000001);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x00000001);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x00000005);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x00000002);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x0000001F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x00000007);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0x0000000c);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0x0000000c);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x00000006);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x00000006);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x00000200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x00000300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x000003bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x00000300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x000003bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x00001000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x00000008);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0x000000c8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x00000007);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x0000002e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x00000007);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x000001f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x00000234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0x000000ff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x00000067);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x00000019);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x00000018);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0x000000d0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x00000003);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x00000005);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x00000009);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x0000001f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x00000021);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x00000004);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x00000002);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x00000000);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00000001);
	 
	mdelay(10);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x00000002);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0xd97b0d53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0x0ff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x00000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x00000001);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0x000000FF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x00000000);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x00000001);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x00000000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x00000002);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x00000001);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,0x175c,1,1,0x00000000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,0x175c,0,0,0x00000000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x00000000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x00000001);


		time = ktime_get();
		
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
	#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
	#endif
	
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);
	
		return 0;

}


int a60972_original_script_init(void)
{
		//solve linux 3.18 compile issue
		
		ktime_t time;
		UINT32 usec = 0;	 
		UINT32 msec = 0;  
		UINT32 sec = 0;
	
	
		time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
#endif
		
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
	   
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
	   
	   
	   
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
	   
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x08400000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x08400000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x08400000);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	   
	   
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x01);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1c0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x214);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x3F);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x40);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x4);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,7,5,0x6);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x10000);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,29,11,0x60000);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x2);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0xd97b0d53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,0x175C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	   
	   
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	   
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	  
	time = ktime_get();
		
	#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
			usec = (time.tv.nsec / 1000) % 1000;	 
			msec = (time.tv.nsec / 1000000) % 1000;   
			sec = time.tv.sec ;
	#else
			usec =(UINT32) ktime_to_us(time) % 1000;
			msec =(UINT32) ktime_to_ms(time) % 1000;
			sec  =(UINT32) ktime_to_sec(time);
	#endif
	
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);

	
	return 0;
			
}

int a60972_epon_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);

	
	IO_SPHYA_REG_BITS(PMA_TOP_PHYD_TX_PARITY1,0x010c,31,0,0x100);//add
	
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
	 
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x08000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x08000000);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x08000000);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,25,25,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,24,24,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	 
	 
	 
	//IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x00);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x2);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x3);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x100);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x340);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0x22);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x1);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x3F);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x40);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,31,0,0xff040000);//add
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x4);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,7,5,0x6);
	 
	 
	 
	//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x20000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,31,0,0x23200000);//add
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x8000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,29,11,0x78000);
	 
	 
	 
	 
	 
	 
	 
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	 
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,31,0,0x0);//add
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,31,0,0x90000000);//add
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);	
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x00);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	 
	//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,31,0,0x0);//add
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x3);
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x3);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);
	 

	xpon_pma_trans_ben_active_level();
	IO_SPHYA_REG_BITS(XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1,0x1788,16,16,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}

int a60972_gpon_script_init(void)
{
		//solve linux 3.18 compile issue
		
		ktime_t time;
		UINT32 usec = 0;	 
		UINT32 msec = 0;  
		UINT32 sec = 0;
	
		time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
#endif
	
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);
	
	
	
	
		IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
		 
		 
		 
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
		IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
		 
		 
		 
		 
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x07F67620);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x07F67620);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x07F67620);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
		IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,25,25,0x1);
		IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,24,24,0x1);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
		 
		 
		 
		//IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
		IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x2);
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
		 
		 
		 
		 
		 
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x2);
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
		IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
		IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
		 
		 
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x1);
		 
		 
		 
		 
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x5);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x5);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x2);
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x1);
		 
		 
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
		 
		 
		 
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		 
		 
		IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x2);
		
		xpon_pma_trans_ben_active_level();
	 
		IO_SPHYA_REG_BITS(XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1,0x1788,16,16,0x1);
		
		IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
		time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
#endif
	
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);
	
	
		return 0;
	
}
int a60972_xgpon_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);



	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x07F67620);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x07F67620);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x07F67620);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	 
	 
	 
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x3);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,3,3,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0,0x111c,31,13,0x00000);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);


	xpon_pma_trans_ben_active_level();

	IO_SPHYA_REG_BITS(XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1,0x1788,16,16,0x1);

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}

int a60972_xgspon_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);

	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x07F67620);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x07F67620);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x07F67620);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	 
	 
	 
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x3);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,3,3,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0,0x111c,31,13,0x00000);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);


	xpon_pma_trans_ben_active_level();


	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}


int a60972_xgpon_2_xepon_script_init(void)
{
	//solve linux 3.18 compile issue
	
	ktime_t time;
	UINT32 usec = 0;	 
	UINT32 msec = 0;  
	UINT32 sec = 0;

	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);

	// hold A60972 PHYD CLK
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x00);            //david 20170810

	xpon_pma_lcpll_mode_setting();
	xpon_pma_rx_rate();
	xpon_pma_tx_rate();


	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,31,28,0x6);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,27,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,23,21,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,20,18,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,17,15,0x5);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,14,13,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,12,12,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,11,11,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,10,10,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,9,9,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,8,8,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,7,7,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,6,6,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,5,5,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_IC_0,0x2004,3,1,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,31,29,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,28,26,0x4);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,25,22,0x8);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,21,21,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,20,20,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,19,19,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,18,17,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,16,15,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,12,5,0xFF);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,3,3,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,2,1,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,31,30,0x3);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,29,29,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,28,28,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,27,27,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,26,26,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,25,25,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,24,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,23,23,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,22,19,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,18,17,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,16,16,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,15,15,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,14,13,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,12,5,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,4,4,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,3,3,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,2,2,0x1);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,31,24,0x0);
	IO_SPHYA_REG_BITS(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,23,16,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	 
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230,30,0,0x07F67620);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248,30,0,0x07F67620);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c,30,0,0x07F67620);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	 
	 
	 
	// IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);            //david 20170810
	IO_SPHYA_REG_BITS(XFI_PMA_ADD_LCPLL_RO_1,0x2760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_LCPLL_RO_1,0x1760,16,16,0xFF);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);
	 
	 
	 
	 
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x3);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
	 
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x3);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x0);
	//xgpon_2_xepon IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x1);
	 
	 
	 
	 
	 
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,3,3,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0,0x111c,31,13,0x00000);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x5);		//change a_tdc from    0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
	 
	 
	 
	 
	 
	 
	//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);  //mark rx_rate related
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	 
	 
	 
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);


	xpon_pma_trans_ben_active_level();

	IO_SPHYA_REG_BITS(XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1,0x1788,16,16,0x1); 

	//release A60972 PHYD CLK
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);            //david 20170810
	time = ktime_get();
	
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
	usec = (time.tv.nsec / 1000) % 1000;	 
	msec = (time.tv.nsec / 1000000) % 1000;   
	sec = time.tv.sec ;
#else
	usec =(UINT32) ktime_to_us(time) % 1000;
	msec =(UINT32) ktime_to_ms(time) % 1000;
	sec  =(UINT32) ktime_to_sec(time);
#endif

	PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);


	return 0;

}

int a60972_xepon_script_init(void)
{
		//solve linux 3.18 compile issue
		
		ktime_t time;
		UINT32 usec = 0;	 
		UINT32 msec = 0;  
		UINT32 sec = 0;
	
	
		time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
#endif
	
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s start\r\n",sec,msec,usec,__FUNCTION__);
	
		// 0. Trans BEN Active level //
		xpon_pma_trans_ben_active_level();
	
		// 1. XFI CMN //
		xpon_pma_xfi_cmn_init();
	
		// 2. XPON CMN //
		xpon_pma_xpon_cmn_init();
	
		// 3. XFI JCPLL 7p5g //
		xpon_pma_jcpll_init();
	
		// 4.XPON PLL 10p3125g //
		xpon_pma_lcpll_cmn_init();
	
		//mode SEL
		xpon_pma_lcpll_mode_setting();
	
		// 5. PLL EN //
		xpon_pma_lcpll_enable();
		
		// 6. XPON CLKPATH //
		xpon_pma_clkpath_setting();
			
		// 7. SW_RESET //
		xpon_pma_sw_reset_init();
		
		// 8. XPON RX //
		xpon_pma_rx_rate();
		
		// 9. XPON TX //
		xpon_pma_tx_ldo_setting();
		xpon_pma_tx_rate();
		
		// cmn setting	//
		xpon_pma_tx_cmn_setting();
		xpon_pma_rx_cmn_setting();


		time = ktime_get();
		
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36) 	
		usec = (time.tv.nsec / 1000) % 1000;	 
		msec = (time.tv.nsec / 1000000) % 1000;   
		sec = time.tv.sec ;
#else
		usec =(UINT32) ktime_to_us(time) % 1000;
		msec =(UINT32) ktime_to_ms(time) % 1000;
		sec  =(UINT32) ktime_to_sec(time);
#endif
	
		PON_PHY_PRINT(PHY_MSG_TRACE,"[%ds:%03dms:%03dus] %s end\r\n",sec,msec,usec,__FUNCTION__);
	
		return 0;

}


//****************************************************************************************************
//XFI common setting
//****************************************************************************************************

int xpon_pma_xfi_cmn_init(void)
{

	#define XFI_PMA_RG_XFI_CMN_CUREN_0_LANE_FUNC_ON (1<<31)
	#define XFI_PMA_RG_XFI_CMN_CUREN_0_VREFSEL_0p5V (0x4<<22)

	IO_SPHYA_REG(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,XFI_PMA_RG_XFI_CMN_CUREN_0_LANE_FUNC_ON\
													|XFI_PMA_RG_XFI_CMN_CUREN_0_VREFSEL_0p5V);

#if 0
	write_data=IO_GPHYA_REG(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018);
	IO_SPHYA_REG_BITS_TO_SW(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,31,31,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,24,22,0x4,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,30,25,0x0,&write_data);
	IO_SPHYA_REG(XFI_PMA_RG_XFI_CMN_CUREN_0,0x2018,write_data);

#endif
	return 0;
}

//****************************************************************************************************
//XPON common setting
//****************************************************************************************************

int xpon_pma_xpon_cmn_init(void)
{
	#define XPON_PMA_RG_XPON_CMN_EN_0_LANE_FUNC_ON (1<<31)
	#define XPON_PMA_RG_XPON_CMN_EN_0_VREFSEL_0p5V (0x4<<22)

	IO_SPHYA_REG(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,XPON_PMA_RG_XPON_CMN_EN_0_LANE_FUNC_ON\
													|XPON_PMA_RG_XPON_CMN_EN_0_VREFSEL_0p5V);

#if 0
	write_data=IO_GPHYA_REG(XPON_PMA_RG_XPON_CMN_EN_0,0x1100);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,31,31,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,24,22,0x4,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,30,25,0x0,&write_data);
	IO_SPHYA_REG(XPON_PMA_RG_XPON_CMN_EN_0,0x1100,write_data);

#endif

	return 0;
}

//****************************************************************************************************
//JCPLL common setting  7.5G
//****************************************************************************************************
int xpon_pma_jcpll_init(void)
{

	#define XFI_PMA_RG_JCPLL_IC_0_IC_75uA   		(0x6<<28)
	#define XFI_PMA_RG_JCPLL_IC_0_IOFF   			(0x0<<24)  // ?
	#define XFI_PMA_RG_JCPLL_IC_0_BC   				(0x3<<21)  // ?
	#define XFI_PMA_RG_JCPLL_IC_0_BP   				(0x3<<18)  // ?
	#define XFI_PMA_RG_JCPLL_IC_0_BR_6R  			(0x5<<15)
	#define XFI_PMA_RG_JCPLL_IC_0_RSTDLY_2048_T_REF	(0x3<<13)
	#define XFI_PMA_RG_JCPLL_IC_0_VOD_EN   			(1<<12)
	#define XFI_PMA_RG_JCPLL_IC_0_SHLPF_EN   		(1<<11)
	#define XFI_PMA_RG_JCPLL_IC_0_LDOCHP_EN   		(1<<10)
	#define XFI_PMA_RG_JCPLL_IC_0_CHPUP_EN			(1<<9)
	#define XFI_PMA_RG_JCPLL_IC_0_VCO_HALFLSB_EN	(1<<8)
	#define XFI_PMA_RG_JCPLL_IC_0_VCOPN_TEST_EN 	(1<<7)
	#define XFI_PMA_RG_JCPLL_IC_0_VTCLTIEL_EN		(1<<6)
	#define XFI_PMA_RG_JCPLL_IC_0_TDCBUF_PDB_EN		(1<<5)
	#define XFI_PMA_RG_JCPLL_IC_0_DIGTEST_CK_EN 	(1<<4)
	#define XFI_PMA_RG_JCPLL_IC_0_VCOVAR_BIAS_350mV	(0x4<<1)
	
	IO_SPHYA_REG(XFI_PMA_RG_JCPLL_IC_0,0x2004,XFI_PMA_RG_JCPLL_IC_0_IC_75uA\
											|XFI_PMA_RG_JCPLL_IC_0_BC\
											|XFI_PMA_RG_JCPLL_IC_0_BP\
											|XFI_PMA_RG_JCPLL_IC_0_BR_6R\
											|XFI_PMA_RG_JCPLL_IC_0_RSTDLY_2048_T_REF\
											|XFI_PMA_RG_JCPLL_IC_0_LDOCHP_EN\
											|XFI_PMA_RG_JCPLL_IC_0_CHPUP_EN\
											|XFI_PMA_RG_JCPLL_IC_0_VCO_HALFLSB_EN\
											|XFI_PMA_RG_JCPLL_IC_0_TDCBUF_PDB_EN\
											|XFI_PMA_RG_JCPLL_IC_0_VCOVAR_BIAS_350mV);


	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_650mV			(0x4<<29)
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_900mV			(0x4<<26) 
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_890mV			(0x8<<22) 
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_FORCE_BYPSYNC_EN	(1<<21)  
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_EN			(1<<20)
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_LOAD_EN 	(1<<19)
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_KF			(0x3<<17)
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_KS			(0x3<<15)
	#define XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_PLL_BAND			(0xff<<5)


	IO_SPHYA_REG(XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0,0x2008,XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_650mV\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_900mV\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_890mV\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_FORCE_BYPSYNC_EN\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_EN\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_LOAD_EN\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_KF\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_AUTOK_KS\
													|XFI_PMA_RG_JCPLL_TCLVAR_BIAS_0_PLL_BAND);

	#define XFI_PMA_RG_JCPLL_SDM_ORD_0_3rd_SDM			(0x3<<30)
	#define XFI_PMA_RG_JCPLL_SDM_ORD_0_DIGTCL_DAC_EN	(1<<16) 
	#define XFI_PMA_RG_JCPLL_SDM_ORD_0_MONVCO_SEL_DIV_4	(1<<3) 
	#define XFI_PMA_RG_JCPLL_SDM_ORD_0_MONVCO_EN		(1<<2)  

	IO_SPHYA_REG(XFI_PMA_RG_JCPLL_SDM_ORD_0,0x200C,XFI_PMA_RG_JCPLL_SDM_ORD_0_3rd_SDM\
												|XFI_PMA_RG_JCPLL_SDM_ORD_0_DIGTCL_DAC_EN\
												|XFI_PMA_RG_JCPLL_SDM_ORD_0_MONVCO_SEL_DIV_4\
												|XFI_PMA_RG_JCPLL_SDM_ORD_0_MONVCO_EN);

	#define XFI_PMA_RG_JCPLL_RESERVE0_0_RESERVE1	(0xFF<<16)
	IO_SPHYA_REG(XFI_PMA_RG_JCPLL_RESERVE0_0,0x2010,XFI_PMA_RG_JCPLL_RESERVE0_0_RESERVE1);

	return 0;

}


//****************************************************************************************************
//XPON RX LCPLL Common setting  10.3125G
//****************************************************************************************************

int xpon_pma_lcpll_cmn_init(void)
{

#define XPON_PMA_RG_XPON_PLL_IC_0_IC_75uA		(0x6<<28)
#define XPON_PMA_RG_XPON_PLL_IC_0_IOFF_18p75uA	(0x3<<24) 
#define XPON_PMA_RG_XPON_PLL_IC_0_BC_10pF		(0x1<<21) 
#define XPON_PMA_RG_XPON_PLL_IC_0_BR_10R		(0x5<<15)  
#define XPON_PMA_RG_XPON_PLL_IC_0_RSTDLY_4096_T_REF (0x3<<13)
#define XPON_PMA_RG_XPON_PLL_IC_0_LDOCHP_EN		(1<<10)
#define XPON_PMA_RG_XPON_PLL_IC_0_CHPUP_EN 		(1<<9)
#define XPON_PMA_RG_XPON_PLL_IC_0_VCO_HALFLSB_EN	(1<<8)
#define XPON_PMA_RG_XPON_PLL_IC_0_TDCBUF_PDB 		(1<<5)
#define XPON_PMA_RG_XPON_PLL_IC_0_VCOVAR_BIAS_350mV	(0x4<<1)

IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,XPON_PMA_RG_XPON_PLL_IC_0_IC_75uA\
											|XPON_PMA_RG_XPON_PLL_IC_0_IOFF_18p75uA\
											|XPON_PMA_RG_XPON_PLL_IC_0_BC_10pF\
											|XPON_PMA_RG_XPON_PLL_IC_0_BR_10R\
											|XPON_PMA_RG_XPON_PLL_IC_0_RSTDLY_4096_T_REF\
											|XPON_PMA_RG_XPON_PLL_IC_0_LDOCHP_EN\
											|XPON_PMA_RG_XPON_PLL_IC_0_CHPUP_EN\
											|XPON_PMA_RG_XPON_PLL_IC_0_VCO_HALFLSB_EN\
											|XPON_PMA_RG_XPON_PLL_IC_0_TDCBUF_PDB\
											|XPON_PMA_RG_XPON_PLL_IC_0_VCOVAR_BIAS_350mV);


#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_650mV				(0x4<<29)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_VCO_SCA_BIAS_900mV	(0x4<<26)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_TCL_TEMPCODE_890mV	(0x8<<22)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_FORCE_BYPSYNC_EN		(0x1<<21)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_EN				(0x1<<20)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_LOAD			(0x1<<19)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_KF				(0x3<<17)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_KS				(0x3<<15)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_PLL_BAND				(0xFF<<5)
#define	XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_SDM_IFM_FRACTIONAL_N	(0x1<<4)

IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_650mV\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_VCO_SCA_BIAS_900mV\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_TCL_TEMPCODE_890mV\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_FORCE_BYPSYNC_EN\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_EN\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_LOAD\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_KF\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_AUTOK_KS\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_PLL_BAND\
												|XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0_SDM_IFM_FRACTIONAL_N);


#define	XPON_PMA_RG_XPON_PLL_SDM_ORD_0_3rd_SDM			(0x3<<30)
#define	XPON_PMA_RG_XPON_PLL_SDM_ORD_0_DIGTCL_DAC_EN	(1<<16)
#define	XPON_PMA_RG_XPON_PLL_SDM_ORD_0_TDC_FBKDIV_128	(0x80<<5)
#define	XPON_PMA_RG_XPON_PLL_SDM_ORD_0_TDC_FBKDIV_256	(0xff<<5)


IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,XPON_PMA_RG_XPON_PLL_SDM_ORD_0_3rd_SDM\
												|XPON_PMA_RG_XPON_PLL_SDM_ORD_0_DIGTCL_DAC_EN\
												|XPON_PMA_RG_XPON_PLL_SDM_ORD_0_TDC_FBKDIV_256);


#define	XPON_PMA_RG_XPON_PLL_RESERVE0_0_RESERVE1  	(0xFF<<15)

IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,XPON_PMA_RG_XPON_PLL_RESERVE0_0_RESERVE1);


#define	_XPON_PMA_SS_LCPLL_TDC_FLT_3_NCPO_LOAD		(1<<8)

IO_SPHYA_REG(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,_XPON_PMA_SS_LCPLL_TDC_FLT_3_NCPO_LOAD);

#if 0
	uint write_data = 0;

	write_data=IO_GPHYA_REG(XPON_PMA_RG_XPON_PLL_IC_0,0x1130);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,31,28,0x6,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,27,24,0x3,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,23,21,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,20,18,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,17,15,0x5,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,14,13,0x3,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,12,12,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,11,11,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,10,10,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,9,9,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,8,8,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,7,7,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,6,6,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,5,5,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,4,4,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,3,1,0x4,&write_data);
	IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_IC_0,0x1130,write_data);

	write_data=IO_GPHYA_REG(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,31,29,0x4,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,28,26,0x4,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,25,22,0x8,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,21,21,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,20,20,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,19,19,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,18,17,0x3,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,16,15,0x3,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,14,13,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,12,5,0xFF,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,4,4,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,3,3,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,2,1,0x0,&write_data);
	IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_TCLVAR_BIAS_0,0x1134,write_data);

	write_data=IO_GPHYA_REG(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,31,30,0x3,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,29,29,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,28,28,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,27,27,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,26,26,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,25,25,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,24,24,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,23,23,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,22,19,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,18,17,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,16,16,0x1,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,15,15,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,14,13,0x0,&write_data);
	IO_SPHYA_REG_BITS_TO_SW(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,12,5,0x80,&write_data);
	IO_SPHYA_REG(XPON_PMA_RG_XPON_PLL_SDM_ORD_0,0x1138,write_data);

	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,30,23,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,22,15,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x1);

#endif	
	return 0;

}
	
//****************************************************************************************************
//XPON RX LCPLL Setting
//DA_XPON_PLL_SDM_PCW[30:0]

 //EPON: T-10.3125G / R-10.3125G: 31'h08400000
// EPON: T-1.25G	/ R-10.3125G: 31'h08400000
// EPON: T-1.25G	/ R-1.25G	: 31'h08000000
// GPON: T-9.95328G / R-9.95328G: 31'h07F67620
// GPON: T-2.48832G / R-9.95328G: 31'h07F67620
// GPON: T-1.24416G / R-9.95328G: 31'h07F67620

//RG_XPON_CDR_LPF_RATIO[1:0]
//00: 10G mode
//10: 2.5G mode
//11: 1.25G mode
//****************************************************************************************************
int xpon_pma_lcpll_mode_setting(void)
{
	switch(xpon_pma_param.LCPLL_Mode)
	{
		case RX_pll_10p3125G:
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230, 30,0, 0x08400000);
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248, 30,0, 0x08400000);
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c, 30,0, 0x08400000);
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118, 25, 24, 0x0);	//RG_XPON_CDR_LPF_RATIO
			break;
		case RX_pll_9p95328G:
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230, 30,0,	0x07F67620);	// DA_XPON_PLL_SDM_PCW[30:0]
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248, 30,0,	0x07F67620);	// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c, 30,0,	0x07F67620);	// rg_lcpll_pon_hrdds_pcw_ncpo_epon
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118, 25, 24, 0x0);	//RG_XPON_CDR_LPF_RATIO
			break;
		case Rx_pll_2p48832G:
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230, 30,0,	0x07F67620);	// DA_XPON_PLL_SDM_PCW[30:0]
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248, 30,0,	0x07F67620);	// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c, 30,0,	0x07F67620);	// rg_lcpll_pon_hrdds_pcw_ncpo_epon
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118, 25, 24, 0x2);	//RG_XPON_CDR_LPF_RATIO
			break;
		case Rx_pll_1p25G:
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_2,0x1230, 30,0,	0x08000000);	// DA_XPON_PLL_SDM_PCW[30:0]
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_1,0x1248, 30,0,	0x08000000);	// rg_lcpll_pon_hrdds_pcw_ncpo_gpon
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PCW_2,0x124c, 30,0,	0x08000000);	// rg_lcpll_pon_hrdds_pcw_ncpo_epon
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118, 25, 24, 0x3);	//RG_XPON_CDR_LPF_RATIO			
			break;
		default:
			break;
	}
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,		0x1118, 3,	3,	0x1);	//RG_XPON_CDR_LPF_SET
	return 0;
}


//****************************************************************************************************
//XPON RX LCPLL Enable
//****************************************************************************************************

int xpon_pma_lcpll_enable(void)
{
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_PWCTL_SETTING_0,0x1200,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_PLL_RESERVE0_0,0x113C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_CLKPATH_RST_0,0x1750,8,8,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,25,25,0x1);
	IO_SPHYA_REG_BITS(PMA_TOP_TX_PAR_PROBE_PLL_CTRL,0x0644,24,24,0x1);

	return 0;
}

//****************************************************************************************************
//XPON CLK path setting
//****************************************************************************************************

int xpon_pma_clkpath_setting(void)
{
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,31,31,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,30,23,0x21);
	return 0;
}
//****************************************************************************************************
//XPON SW reset init
//****************************************************************************************************

int xpon_pma_sw_reset_init(void)
{	
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,24,24,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,1,1,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,0,0,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);
	IO_SPHYA_REG_BITS(PMA_TOP_DUMP_RX_PCSRST_CTRL,0x0648,16,16,0x00);

	return 0;
}

//****************************************************************************************************
//XPON TX LDO setting
//****************************************************************************************************

int xpon_pma_tx_ldo_setting(void)
{	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,21,21,0x0);

	return 0;
}

//****************************************************************************************************
//RX Mode Select
//rg_xpon_mode[2:0]
//000: RX 16 bit (10G/2G,10G/10G,10G/1G,10G/10G)
//001: R2T mode
//010: RX 16bit mode (2G/1,2G/2G)
//011: RX 10 bit mode (1G/1G EPON)
//100: T2R mode

 //EPON: T-10.3125G / R-10.3125G: 31'h08400000
// EPON: T-1.25G    / R-10.3125G: 31'h08400000
// EPON: T-1.25G    / R-1.25G   : 31'h08000000
// GPON: T-9.95328G / R-9.95328G: 31'h07F67620
// GPON: T-2.48832G / R-9.95328G: 31'h07F67620
// GPON: T-1.24416G / R-2.48832G: 31'h07F67620

//rg_xpon_rx_rate_ctrl[1:0]
//00: 8 bit      (DA_XPON_RX_DATA[7:0])
//01: 10 bit     (DA_XPON_RX_DATA[9:0])
//10: 16 bit     (DA_XPON_RX_DATA[15:0])
// EPON: T-10.3125G / R-10.3125G: 2'b10
// EPON: T-1.25G    / R-10.3125G: 2'b10
// EPON: T-1.25G    / R-1.25G   : 2'b01
// GPON: T-9.95328G / R-9.95328G: 2'b10
// GPON: T-2.48832G / R-9.95328G: 2'b10
// GPON: T-1.24416G / R-2.48832G: 2'b00

//****************************************************************************************************
int xpon_pma_rx_rate(void)
{
	switch(xpon_pma_param.RX_Mode)
	{
		case RX_Rate_10p3125G:
		case RX_Rate_9p95328G:
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,			0x1754,	2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,	0x1714,	1,	0,	0x02);//rg_xpon_rx_rate_ctrl
			break;
			
		case Rx_Rate_2p48832G:
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,			0x1754,	2,	0,	0x02);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,	0x1714,	1,	0,	0x00);//rg_xpon_rx_rate_ctrl
			break;
			
		case Rx_Rate_1p25G:
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,			0x1754,	2,	0,	0x01);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,	0x1714,	1,	0,	0x01);//rg_xpon_rx_rate_ctrl
			break;

		default:
			break;
	}
	

	return 0;
}


//****************************************************************************************************
//XPON RX common setting
//****************************************************************************************************
int xpon_pma_rx_cmn_setting(void)
{
	uint read_data = 0;

	IO_SPHYA_REG(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_50OHMS_SEL(1)\
															|XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ1_EN\
															|XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ2_EN\
															|XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0_FE_VB_EQ3_EN);

	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);



	IO_SPHYA_REG(XPON_PMA_RX_DISB_MODE_0,0x163C,XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_RSTB_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_LCK2DATA_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_GAIN_CTRL_FORCE_MODE);
		

	IO_SPHYA_REG(XPON_PMA_RX_FORCE_MODE_0,0x1630,XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_RSTB\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_LCK2REF\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL(0x2)\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_GAIN_CTRL(0x1));


	
	IO_SPHYA_REG(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PD_PWDB\
													|XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PR_PIEYE_PWDB\
													|XPON_PMA_SS_DA_XPON_PWDB_0_CDR_PR_PWDB\
													|XPON_PMA_SS_DA_XPON_PWDB_0_RX_FE_PWDB);
		

	IO_SPHYA_REG(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,XPON_PMA_SS_DA_XPON_PWDB_1_RX_SIGDET_PWDB);


	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x0); 


	IO_SPHYA_REG(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_RDY_FORCE_MODE\
															|XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_BLWC_EN_NORMAL_MODE\
															|XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_OS_RDY_NORMAL_MODE\
															|XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1_RX_SDCAL_EN_NORMAL_MODE);
	IO_SPHYA_REG(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_RDY_OFF);




	IO_SPHYA_REG(XPON_PMA_RX_DISB_MODE_0,0x163C,XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_RSTB_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_CDR_LPF_LCK2DATA_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL_FORCE_MODE\
												|XPON_PMA_RX_DISB_MODE_0_DA_XPON_RX_FE_GAIN_CTRL_FORCE_MODE);
		

	IO_SPHYA_REG(XPON_PMA_RX_FORCE_MODE_0,0x1630,XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_RSTB\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_CDR_LPF_LCK2DATA\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_PEAKING_CTRL(0x2)\
												|XPON_PMA_RX_FORCE_MODE_0_DA_XPON_RX_FE_GAIN_CTRL(0x1));

	
	
	IO_SPHYA_REG(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1_RX_RDY);

	IO_SPHYA_REG(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,XPON_PMA_SS_LCPLL_TDC_FLT_1_A_TDC(0x7));
	IO_SPHYA_REG(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,0x0);

	// ? detail ? set in many stages
	IO_SPHYA_REG(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_DFM_DA(0x21)\
															|XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_LDO_EN\
															|XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_AUTOEN\
															|XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_MON_RXCK_EN\
															|XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0_TDC_MON_TXCK_EN);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);  //TDC_DIG_PWDB
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);  //cal_bw
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);  //cal_offset
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);  //cal_offset off
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);  //cal_bw off

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);  //EN_MUX

	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1); //tdc_en
	
	// set PMA XPON CDR LPF PARAM   //david 20170509
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x20000); //RG_XPON_CDR_LPF_TOP_LIM
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PD_PICAL_CKD8_INV_0,0x1114,29,11,0x60000); //RG_XPON_CDR_LPF_BOT_LIM
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1); //rg_xpon_cdr_pr_rstb_bypass, manual enable

	return 0;

}


//****************************************************************************************************
//RG_XPON_TX_CKSEL 0x1150[12:10]                       //RG_XPON_TX_RATE_CTRL 0x1150[1:0]             
// TYPE: Tx rate/Rx rate                                         // TYPE: Tx rate/Rx rate                       
// 0x05 =>EPON: T-10.3125G / R-10.3125G: 3'b101  // 0x02 =>EPON: T-10.3125G / R-10.3125G: 2'b10 
// 0x01 =>EPON: T-1.25G      / R-10.3125G: 3'b001  // 0x01 =>EPON: T-1.25G      / R-10.3125G: 2'b01 
// 0x02 =>EPON: T-1.25G      / R-1.25G     : 3'b010  // 0x01 =>EPON: T-1.25G      / R-1.25G     : 2'b01 
// 0x05 =>GPON: T-9.95328G / R-9.95328G: 3'b101  // 0x02 =>GPON: T-9.95328G / R-9.95328G: 2'b10 
// 0x03 =>GPON: T-2.48832G / R-9.95328G: 3'b011  // 0x00 =>GPON: T-2.48832G / R-9.95328G: 2'b00 
// 0x02 =>GPON: T-1.24416G / R-9.95328G: 3'b010  // 0x00 =>GPON: T-1.24416G / R-9.95328G: 2'b00 
//****************************************************************************************************
int xpon_pma_tx_rate(void)
{
	switch(xpon_pma_param.TX_Mode)
	{
		case TX_Rate_10p3125G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x5);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x2);	//RG_XPON_TX_RATE_CTRL[1:0]
			break;
		case TX_Rate_9p95328G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x5);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x2);	//RG_XPON_TX_RATE_CTRL[1:0]
			break;
		case Tx_Rate_2p48832G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x3);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x0);	//RG_XPON_TX_RATE_CTRL[1:0]
			break;
		case Tx_Rate_1p25G_RX_10p3125G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x1);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x1);	//RG_XPON_TX_RATE_CTRL[1:0]	
			break;
		case Tx_Rate_1p25G_RX_1p25G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x2);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x1);	//RG_XPON_TX_RATE_CTRL[1:0]	
			break;
		case Tx_Rate_1p24416G:
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	12,	10,	0x2);	//RG_XPON_TX_CKSEL[2:0]
			IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,	0x1150,	5,	4,	0x0);	//RG_XPON_TX_RATE_CTRL[1:0]			
			break;
		default:
			break;
	}

	return 0;
}



//****************************************************************************************************
//TX Common setting
//****************************************************************************************************
int xpon_pma_tx_cmn_setting(void)
{
	IO_SPHYA_REG(XPON_PMA_RG_XPON_TX_EN_0,0x1148,XPON_PMA_RG_XPON_TX_EN_0_TX_EN\
												|XPON_PMA_RG_XPON_TX_EN_0_SER_LOADSEL_T7\
												|XPON_PMA_RG_XPON_TX_EN_0_FIR_CN1(0x3)\
												|XPON_PMA_RG_XPON_TX_EN_0_FIR_INV_C0(0x0)\
												|XPON_PMA_RG_XPON_TX_EN_0_FIR_C1(0xa)\
												|XPON_PMA_RG_XPON_TX_EN_0_FIR_C2(0x3)\
												|XPON_PMA_RG_XPON_TX_EN_0_FORCE_CX\
												|XPON_PMA_RG_XPON_TX_EN_0_SR_CTRL(0x0)\
												|XPON_PMA_RG_XPON_TX_EN_0_LPEIDLE_EN\
												|XPON_PMA_RG_XPON_TX_EN_0_CKMON_SEL(0x0));


	IO_SPHYA_REG(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_BIAS_EN\
														|XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_DRV_EN\
														|XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_DRV_IBIAS(0x1f)\
														|XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_EN\
														|XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_IBIAS(0x7)\
														|XPON_PMA_RG_XPON_BENTX_BIAS_EN_0_PRED_IMP_SMALL);

	IO_SPHYA_REG(XPON_PMA_SS_TX_RST_B,0x1550,XPON_PMA_SS_TX_RST_B_CALIB_RST_B\
											|XPON_PMA_SS_TX_RST_B_TOP_RST_B);

	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,XPON_PMA_SS_TX_CALIB_0_CALIB_P_EN);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,!XPON_PMA_SS_TX_CALIB_0_CALIB_P_EN);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,XPON_PMA_SS_TX_CALIB_1_CALIB_N_EN);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,!XPON_PMA_SS_TX_CALIB_1_CALIB_N_EN);

	IO_SPHYA_REG_BITS(XPON_PMA_RG_LCPLL_xpon_CMN_STB_CTRL_1,0x1788,16,16,0x1);

	return 0;

}

//****************************************************************************************************
//TX Bist Control
//****************************************************************************************************
int xpon_pma_tx_bist_pattern_setting(void)
{
	// select XPON test pattern selection: XPON_PMA_BISTCTL_CONTROL 0x1500[4:0]
	//   5'd1:	PRBS7_PATTERN
	//	5'd2:	PRBS9_PATTERN
	//	5'd3:	PRBS15_PATTERN
	//	5'd4:	PRBS23_PATTERN
	//	5'd5:	PRBS31_PATTERN
	//	5'd6:	HFTP_PATTERN (10101010101010101010)
	//	5'd9:	Square_wave_PATTERN (n=5) LFTP
	//	5'd14	Square_wave_PATTERN (n=10) LFTP
	//	5'd17	All 1
	//	5'd18	All 0
	//	5'd19	PRBS11_PATTERN

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 4,	0,	xpon_pma_param.TX_Bist_Pattern);//rg_bistctl_pat_sel			 david 20170428
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,	0x1514, 31, 0,	0x0ff1fd53);//bistctl_prbs_init_seed	 david 20170428

	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,					0x1684, 8,	8,	0x01);//all_lane_prbs_tx_en
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,					0x1684, 0,	0,	0x00);//anlt_px_lnx_lt_los
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 24, 24, 0x0);//rg_bistctl_io_data_inv
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,		0x1714,	1,	0,	0x2);//rg_xpon_rate_ctrl          //david 20170505

	return 0;
}

//****************************************************************************************************
//RX Bist Align Control
//PRBS7:  6081 df53
//PRBS9:  d97b 0d53
//PRBS15: e1fd 7d53
//PRBS23: 5cf1 fd53
//PRBS31: 8ff1 fd53
//PRBS11: a190_3d53
//****************************************************************************************************
int xpon_pma_rx_bist_check_setting(XPON_PMA_RX_Bist_Check_t align_pattern)
{

	printk("[%s:%d] pattern %d\n",__FUNCTION__,__LINE__,align_pattern);

	switch(align_pattern)
	{
		case PRBS7_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,			0x1504,	31,	0,	0x6081DF53);//rg_px_lnx_bist_ctl_align_pattern	
			break;

		case PRBS9_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,			0x1504,	31,	0,	0xD97B0D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS15_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,			0x1504,	31,	0,	0xE1FD7D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS23_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,			0x1504,	31,	0,	0x5CF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS31_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,			0x1504,	31,	0,	0x8FF1FD53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		case PRBS11_ALIGN_PATTERN:
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	4,	0,	align_pattern);//rg_bistctl_pat_sel             david 20170428
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT, 			0x1504, 31, 0,	0xA1903D53);//rg_px_lnx_bist_ctl_align_pattern
			break;

		default:
			break;
				
	}

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED, 		0x1514, 31, 0,	0x0FF1FD53);//bistctl_prbs_init_seed	
//	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 	0x1520, 15, 0,	0x01);//rg_bistctl_prbs_fail_threshold
//	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,						0x1684, 0,	0,	0x0);//anlt_px_lnx_lt_los
//	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 				0x1500, 24, 24, 0x0);//rg_bistctl_io_data_inv
//	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 				0x1500, 16, 16, 0x1);//rg_bistctl_pat_rx_check_en
	

	return 0;
}
//****************************************************************************************************
//RX Bist Align check
//****************************************************************************************************

int xpon_pma_rx_bist_check_result(void)
{
	uint prbs_result = 0;
	uint prbs_err_cnt = 0;

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 				0x1500, 16, 16, 0x0);//rg_bistctl_pat_rx_check_en
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 				0x1500, 16, 16, 0x1);//rg_bistctl_pat_rx_check_en
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,			0x1510, 16, 16, 0x1);//ro_bistctl_prbs_errcnt_read_latch
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD, 	0x1520, 15, 0,	0x01);//rg_bistctl_prbs_fail_threshold
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,						0x1684, 24, 24, 0x00);//rg_inx_bistctl_bit_error_rst_sel
		
	prbs_result = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,				0x1518, 16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
	prbs_err_cnt = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,				0x151c, 15, 0);//ro_bistctl_prbs_err_cnt 
	printk("0x%8x = %8x\r\n",XPON_PMA_BISTCTL_PRBS_EVENT,IO_GPHYA_REG(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518));
	if( prbs_result == 0x10001 )	   //david 20170428
	{
		printk("Pattern Aligned\n");		
	}
	else
	{
		printk("Aligned Failed\n");
	}
#if 0	
	printk("XPON_PMA_BISTCTL_PRBS_EVENT(0x%x) = 0x%x\n",XPON_PMA_BISTCTL_PRBS_EVENT,prbs_result);
	printk("PRBS Align %s\n",(prbs_result&XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_DONE)?"Done":"Not Done");		
	printk("PRBS Check %s\n",(prbs_result&XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_FAIL)?"Fail":"Pass");
	printk("PRBS Compare %s\n",(prbs_result&XPON_PMA_BISTCTL_PRBS_EVENT_BISTCTL_PRBS_COMPARE)?"Ongoing":"Not Match");
	printk("XPON_PMA_BISTCHECK_ERRCNT = %8x\r\n",IO_GPHYA_REG(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151C));
#endif	
	return prbs_result;
}

//****************************************************************************************************
//RX Bist Error counters read
//****************************************************************************************************

int xpon_pma_rx_bist_err_cnt_read(void)
{
	int err_cnt = 0;

	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,			0x1510,	16,	16,	0x1);//ro_bistctl_prbs_errcnt_read_latch

	err_cnt=IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,			0x151c,	15,	0);//ro_bistctl_prbs_err_cnt
	printk("PRBS error counter=%d\n",err_cnt);

	return err_cnt;
}

// select PRBS pattern at the pcs side of interface      // david 20170505
void xpon_pma_interface_bist_sel_setting(void)
{
//	select PRBS pattern and enable rg_bist_pat_tx_en & rg_bist_pat_rx_check_en
		switch(xpon_pma_param.TX_Bist_Pattern)
	{
		case PATTERN_DISABLE:
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,				0x1754,	16,	16,	0x00);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,				0x1500,	8,	8,	0x00);//rg_bistctl_pat_tx_en             //david 20170428
			break;

		case PRBS7_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x103);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0x6081fd53);
			break;
			
		case PRBS9_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x203);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0xd97b0d53);
			break;
			
		case PRBS15_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x303);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0xe1fd7d53);
			break;
			
		case PRBS23_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x403);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0x5cf1fd53);
			break;
			
		case PRBS31_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x503);
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0x8ff1fd53);
			break;

		case PROGRAMMED_IF_TX_TEST_PATTERN:
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0x1003);
		default:
			break;
				
		}

	uint read_data = 0;
	read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_0);
	printk("EN7580_IF_RG_INTERFACE_0(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_0,read_data);

	read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_1);
	printk("EN7580_IF_RG_INTERFACE_0(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_1,read_data);

}

// set bist check at the pcs side of interface    // david 20170505
void xpon_pma_interface_bist_check_setting(void)
{
	uint read_data = 0;
	read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_9);
	printk("EN7580_IF_RG_INTERFACE_9(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_9,read_data);
	printk("PRBS Align %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_DONE)?"Done":"Not Done");		
	printk("PRBS Check %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_FAIL)?"Fail":"Pass");
	printk("PRBS Compare %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_COMPARING)?"Ongoing":"Not yet");
	printk("if fail, try : echo if_bist_check > /proc/a60972/debug\r\n");	
}

// read BIST check error counter               //david 20160620
void xpon_pma_err_cnt_check(void)
{
	
	uint prbs_result = 0;
	uint prbs_err_cnt = 0;

//	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,						0x1684, 24, 24, 0x00);//rg_inx_bistctl_bit_error_rst_sel
		
	prbs_result = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,				0x1518, 16, 0);//ro_bistctl_prbs_compare&fail&done	  david 20170428
	prbs_err_cnt = IO_GPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,				0x151c, 15, 0);//ro_bistctl_prbs_err_cnt 
	printk("0x%8x = %8x\r\n",XPON_PMA_BISTCTL_PRBS_EVENT,IO_GPHYA_REG(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518));
	printk("0x%8x = %8x\r\n",XPON_PMA_BISTCTL_PRBS_ERRCNT,IO_GPHYA_REG(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151C));
	if( prbs_result == 0x10001 )	   //david 20170428
	{
		printk("Pattern Aligned\n");
		printk("Error Counter = %8x\r\n", prbs_err_cnt);
	}
	else
	{
		printk("Aligned Failed\n");
		printk("Error Counter = %8x\r\n", prbs_err_cnt);
	}
	
}

// TX interface quality calibration function     //david 20170522
int xpon_pcs_tx_quality_check(void)
{
	// set cycle to be 4
	IO_SPHYREG(EN7580_IF_RG_PROGRAME_N1,0x3);
	// set program pattern: 0x3FF=>0x3FF=>0x0=>0x0=>0x3FF=>0x3FF
	IO_SPHYREG(EN7580_IF_RG_PROGRAME_0,0xFFFFFFFF);
	IO_SPHYREG(EN7580_IF_RG_PROGRAME_1,0xFFFFFFFF);
	IO_SPHYREG(EN7580_IF_RG_PROGRAME_2,0x0);
	IO_SPHYREG(EN7580_IF_RG_PROGRAME_3,0x0);	

	return 0;	//ang_20180208
}

void xpon_pcs_pma_tx_quality_check_result(void)
{
	uint read_data = 0;
	IO_SPHYA_REG_BITS(0,	0x0640	,0,	0,	0x1);	//dump en
	mdelay(50);
	IO_SPHYA_REG_BITS(0,	0x0640	,16,	16,	0x1);	//wait dump full
	IO_SPHYA_REG_BITS(0,	0x0640	,0,	0,	0x0);	//dump en

	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x0);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x0, 0x634 = 0x%x\n",read_data);   //
	
	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x1);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x1, 0x634 = 0x%x\n",read_data);   //

	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x2);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x2, 0x634 = 0x%x\n",read_data);   //

	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x3);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x3, 0x634 = 0x%x\n",read_data);   //

	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x4);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x4, 0x634 = 0x%x\n",read_data);   //

	IO_SPHYA_REG_BITS(0,	0x0630	,12,	0,	0x5);	//
	read_data = IO_GPHYA_REG_BITS(0,	0x0634	,31,	0);	//
	printk("0x630 = 0x5, 0x634 = 0x%x\n",read_data);   //
	
}




void xpon_pma_eye_open_calibration(void)
{

	IO_SPHYA_REG_BITS(0,	0x174c	,8,	8,	0x00);	//rg_disb_status_rx_rst_b	
	IO_SPHYA_REG_BITS(0,	0x174c	,0,	0,	0x00);	//rg_force_status_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x1360	,8,	8,	0x00);	//rg_disb_rx_os_rdy	
	IO_SPHYA_REG_BITS(0,	0x1368	,8,	8,	0x00);	//rg_force_rx_os_rdy
	IO_SPHYA_REG_BITS(0,	0x1360	,24,	24,0x00);	//rg_disb_rx_rdy
	IO_SPHYA_REG_BITS(0,	0x1368	,24,	24,0x00);	//rg_force_rx_rdy
	IO_SPHYA_REG_BITS(0,	0x135C	,0,	0,0x00);	//rg_disb_rx_os_en
	IO_SPHYA_REG_BITS(0,	0x1364	,0,	0,0x01);	//rg_force_rx_os_en
	mdelay(100);	
	
	

	IO_SPHYA_REG_BITS(0,	0x1430	,10	,	8	,	0x1 );	//rg_kpgain, 3'h1	
	IO_SPHYA_REG_BITS(0,	0x1494	,16	,	16	,	0x0 );	//rg_eq_pi_cal_rst_b, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1670	,8	,	8	,	0x0 );	//rg_disb_rx_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1664	,8	,	8	,	0x0 );	//rg_force_rx_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1670	,0	,	0	,	0x0 );	//rg_disb_ref_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1664	,0	,	0	,	0x0 );	//rg_force_ref_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1670	,24	,	24	,	0x0 );	//rg_disb_rx_or_pical_en, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1654	,8	,	8	,	0x0 );	//rg_force_rx_or_pical_en, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1494	,16	,	16	,	0x1 );	//rg_eq_pi_cal_rst_b, 1'b1
	IO_SPHYA_REG_BITS(0,	0x1670	,8	,	8	,	0x0 );	//rg_disb_rx_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1664	,8	,	8	,	0x1 );	//rg_force_rx_and_pical_rstb, 1'b1
	IO_SPHYA_REG_BITS(0,	0x1670	,0	,	0	,	0x0 );	//rg_disb_ref_and_pical_rstb, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1664	,0	,	0	,	0x1 );	//rg_force_ref_and_pical_rstb, 1'b1
	IO_SPHYA_REG_BITS(0,	0x1660	,24	,	24	,	0x0 );	//rg_disb_rx_or_pical_en, 1'b0
	IO_SPHYA_REG_BITS(0,	0x1654	,8	,	8	,	0x1 );	//rg_force_rx_or_pical_en, 1'b1
	IO_SPHYA_REG_BITS(0,	0x1654	,8	,	8	,	0x1 );	//rg_force_rx_or_pical_en, 1'b1
	IO_SPHYA_REG_BITS(0,	0x1654	,8	,	8	,	0x1 );	//rg_force_rx_or_pical_en, 1'b1
	mdelay(100);
	
	IO_SPHYA_REG_BITS(0,	0x1654	,8	,	8	,	0x0 );	//rg_force_rx_or_pical_en, 1'b0
	IO_SPHYA_REG_BITS(0,	0x135C	,8	,	8	,	0x00);	//rg_disb_rx_pical_en
	IO_SPHYA_REG_BITS(0,	0x1364	,8	,	8	,	0x01);	//rg_force_rx_pical_en
	mdelay(100);

	IO_SPHYA_REG_BITS(0,	0x1490	,18	,	16	,	0x02);	//rg_sap_sel
	IO_SPHYA_REG_BITS(0,	0x1670	,16	,	16	,	0x00);	//rg_disb_pdos_rx_rst_b	
	IO_SPHYA_REG_BITS(0,	0x1664	,16	,	16	,	0x00);	//rg_force_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x1498	,0	,	0	,	0x00);	//rg_pdos_rst_b
	mdelay(100);
	IO_SPHYA_REG_BITS(0,	0x1498	,0	,	0	,	0x01);	//rg_pdos_rst_b
	IO_SPHYA_REG_BITS(0,	0x1670	,16	,	16	,	0x00);	//rg_disb_pdos_rx_rst_b

	IO_SPHYA_REG_BITS(0,	0x1664	,16	,	16	,	0x01);	//rg_force_pdos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x135C	,16	,	16	,	0x00);	//rg_disb_rx_pdos_en
	IO_SPHYA_REG_BITS(0,	0x1364	,16	,	16	,	0x01);	//rg_force_rx_pdos_en
	
	mdelay(100);
	IO_SPHYA_REG_BITS(0,	0x1364	,16	,	16	,	0x00);	//rg_force_rx_pdos_en
	mdelay(100);

	
	
	IO_SPHYA_REG_BITS(0,	0x1400	,7	,	0	,	0x30);	//rg_lfsel
//	IO_SPHYA_REG_BITS(0,	0x1494	,8	,	8	,	0x00);	//rg_feos_rst_b
	IO_SPHYA_REG_BITS(0,	0x1674	,16	,	16	,	0x00);	//rg_disb_feos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x1668	,16	,	16	,	0x00);	//rg_force_feos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x1494	,8	,	8	,	0x00);	//rg_feos_rst_b
	mdelay(100);
	IO_SPHYA_REG_BITS(0,	0x1494	,8	,	8	,	0x01);	//rg_feos_rst_b
	IO_SPHYA_REG_BITS(0,	0x1674	,16	,	16	,	0x00);	//rg_disb_feos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x1668	,16	,	16	,	0x01);	//rg_force_feos_rx_rst_b
	IO_SPHYA_REG_BITS(0,	0x135C	,24	,	24	,	0x00);	//rg_disb_rx_feos_en
	IO_SPHYA_REG_BITS(0,	0x1364	,24	,	24	,	0x00);	//rg_force_rx_feos_en
	IO_SPHYA_REG_BITS(0,	0x1364	,24	,	24	,	0x00);	//rg_force_rx_feos_en
	IO_SPHYA_REG_BITS(0,	0x1364	,24	,	24	,	0x00);	//rg_force_rx_feos_en
	IO_SPHYA_REG_BITS(0,	0x1364	,24	,	24	,	0x00);	//rg_force_rx_feos_en
	mdelay(100);

	
	

	IO_SPHYA_REG_BITS(0,	0x135C	,0	,	0	,	0x00	)	;	//rg_disb_rx_os_en
	IO_SPHYA_REG_BITS(0,	0x1364	,0	,	0	,	0x00	)	;	//rg_force_rx_os_en
	IO_SPHYA_REG_BITS(0,	0x1360	,0	,	0	,	0x00	)	;	//rg_disb_rx_sdcal_en
	IO_SPHYA_REG_BITS(0,	0x1368	,0	,	0	,	0x00	)	;	//rg_force_rx_sdcal_en
	IO_SPHYA_REG_BITS(0,	0x1360	,16	,	16	,	0x00	)	;	//rg_disb_rx_blwc_en
	IO_SPHYA_REG_BITS(0,	0x1368	,16	,	16	,	0x00	)	;	//rg_force_rx_blwc_en
	mdelay(100);
		

	

	//read	out	pdos	
	IO_SPHYA_REG_BITS(0,	0x149c	,24	,	24	,	0x0 )	;	//toggle to generate latch signal
	mdelay(50);
	IO_SPHYA_REG_BITS(0,	0x149c	,24	,	24	,	0x1 )	;	//toggle to generate latch signal
	//IO_SPHYA_REG_BITS(0,	0x1524	,6	,	0	,	0x0 )	;	//ro_dac_d0
	//IO_SPHYA_REG_BITS(0,	0x1524	,14	,	8	,	0x0 )	;	//ro_dac_d1
	//IO_SPHYA_REG_BITS(0,	0x1524	,22	,	16	,	0x0 )	;	//ro_dac_e0
	//IO_SPHYA_REG_BITS(0,	0x1524	,30	,	24	,	0x0 )	;	//ro_dac_e1
	//IO_SPHYA_REG_BITS(0,	0x1528	,6	,	0	,	0x0 )	;	//ro_dac_eye
	
	//read	out	feos	
	//IO_SPHYA_REG_BITS(0,	0x1548	,13	,	8	,	0x0 )	;	//ro_feos_out	
	//read	out	pical		
	//IO_SPHYA_REG_BITS(0,	0x152c	,22	,	16	,	0x0 )	;	//ro_pi_cal_data_out
	mdelay(100);

	IO_SPHYA_REG_BITS(0,	0x135C	,0	,	0	,	0x00	)	;	//rg_disb_rx_os_en
	IO_SPHYA_REG_BITS(0,	0x1364	,0	,	0	,	0x00	)	;	//rg_force_rx_os_en
	IO_SPHYA_REG_BITS(0,	0x1360	,8	,	8	,	0x00	)	;	//rg_disb_rx_os_rdy
	mdelay(100);
	IO_SPHYA_REG_BITS(0,	0x1368	,8	,	8	,	0x01	)	;	//rg_force_rx_os_rdy
	mdelay(100);

	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,					0x163C,	16,	16,	0x00);//rg_disb_da_xpon_cdr_lpf_lck2data
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,					0x1630,	16,	16,	0x01);//rg_force_da_xpon_cdr_lpf_lck2data
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,	0x1368,	24,	24,	0x01);//rg_force_rx_rdy
	
}


//EYE Setting
void xpon_pma_eye_open_init(void)
{
	IO_SPHYA_REG_BITS(0, 0x1330, 9, 0, 0x0F8);	//rg_cntlen, 10'h0F8
	IO_SPHYA_REG_BITS(0, 0x1330, 16, 16, 0x0);	//rg_cntforever, 1'b0
	IO_SPHYA_REG_BITS(0, 0x1330, 31, 24, 0xFF); //rg_eye_mask, 8'hFF
	IO_SPHYA_REG_BITS(0, 0x1338, 8, 8, 0x0);	//rg_data_shift, 1'b0

	IO_SPHYA_REG_BITS(0, 0x1300, 10, 0, 0x1d8); 	//rg_x_min, 11'h1d8
	IO_SPHYA_REG_BITS(0, 0x1300, 26, 16, 0x234);	//rg_x_max, 11'h234

	IO_SPHYA_REG_BITS(0, 0x1304, 16, 16, 0x0);		//rg_index_mode, 1'b0
	IO_SPHYA_REG_BITS(0, 0x1308, 19, 0, 0x0FFF8);	//rg_eyedur, 20'hFFFF8

	IO_SPHYA_REG_BITS(0, 0x130c, 0, 0, 0x0);		//rg_eye_nextpts_sel, 1'b0
	IO_SPHYA_REG_BITS(0, 0x130c, 8, 8, 0x0);		//rg_eye_nextpts_toggle, 1'b0
	IO_SPHYA_REG_BITS(0, 0x130c, 16, 16, 0x1);		//rg_eye_nextpts, 1'b1

	IO_SPHYA_REG_BITS(0, 0x1320, 7, 0, 0x4);		//rg_eyecnt_hth, 8'd2
	IO_SPHYA_REG_BITS(0, 0x1320, 15, 8, 0x4);		//rg_eyecnt_vth, 8'd2
	IO_SPHYA_REG_BITS(0, 0x1324, 10, 0, 0x4);		//rg_eo_hth, 11'd10
	IO_SPHYA_REG_BITS(0, 0x1324, 22, 16, 0x4);		//rg_eo_vth, 7'd10

	IO_SPHYA_REG_BITS(0, 0x1374, 16, 16, 0x0);		//rg_heo_emphasis=1'b0
	IO_SPHYA_REG_BITS(0, 0x1374, 0, 0, 0x0);		//rg_a_lgain=1'b0
	IO_SPHYA_REG_BITS(0, 0x1374, 8, 8, 0x0);		//rg_a_mgain=1'b0
	IO_SPHYA_REG_BITS(0, 0x1378, 1, 0, 0x1);		//rg_a_sel=2'b01
	IO_SPHYA_REG_BITS(0, 0x1374, 24, 24, 0x0);		//rg_b_zero_sel=1'b0
}

 uint xpon_pma_eye_open_read(void)
{
    uint eye_er, eye_el;
    uint eye_eu, eye_eb;
    uint eye_x_done, eye_y_done, heo_rdy, veo_rdy;
    uint heo, veo=0;	//ang_20180208
    uint heo_center;
    uint fom_num;
	uint eo_rdy_cnt=0;

    IO_SPHYA_REG_BITS(0, 0x149c, 24, 24, 0x0);      //toggle to generate latch signal
    mdelay(1000);
    IO_SPHYA_REG_BITS(0, 0x149c, 24, 24, 0x1);      //toggle to generate latch signal

    eye_x_done = IO_GPHYA_REG_BITS(0, 0x1548, 16, 16);
    eye_y_done = IO_GPHYA_REG_BITS(0, 0x1548, 24, 24);

	while(1)
	{
	    heo_rdy = IO_GPHYA_REG_BITS(0, 0x1538, 16, 16);
	    veo_rdy = IO_GPHYA_REG_BITS(0, 0x1538, 24, 24);

		mdelay(100);
		if((heo_rdy==1)||(eo_rdy_cnt>50))
		{
		  	printk("eo_rdy_cnt=%d\n",eo_rdy_cnt);
			break;
		}

		eo_rdy_cnt++;
	}
	
    if ((eye_x_done == 0x1) && (eye_y_done == 0x1))
    {
        printk("\n");
        printk("X&Y index scan done\n");

		
        if ((heo_rdy == 1) && (veo_rdy == 1))
        {
            printk("\n");
            printk("heo&veo result ready\n");
        }
    }

    eye_el = (UINT32)IO_GPHYA_REG_BITS(0, 0x154c, 26, 16);
    eye_er = (UINT32)IO_GPHYA_REG_BITS(0, 0x154c, 10, 0);
    eye_eu = (UINT32)IO_GPHYA_REG_BITS(0, 0x1628, 6, 0);
    eye_eb = (UINT32)IO_GPHYA_REG_BITS(0, 0x1628, 14, 8);
//            veo = (UInt32)IO_GPHYA_REG_BITS(0, 0x1534, 22, 16);

    #if 0
    if (eye_eu >= 0x40)
    {
        eye_eu = eye_eu - 0x80;
    }

    if (eye_eb >= 0x40)
    {
        eye_eb = eye_eb - 0x80;
    }
	#endif

	if(eye_er>=eye_el)
    	heo = eye_er - eye_el;
	else
		heo = eye_el - eye_er;

	#if 0
    veo = ((eye_eb >= 64) | (eye_eu >= 64)) ? (UInt32)Math.Abs(eye_eu - eye_eb - 128) : (UInt32)Math.Abs(eye_eu - eye_eb);
    fom_num = (veo_rdy == 1) ? heo + veo : heo;
	#endif
    //print out these results to textbox
    #if 0
    richTextBox3.AppendText("Y index sweep direction: " + comboBoxY.SelectedItem.ToString() + "\n");
    richTextBox3.AppendText("fom_num = " + fom_num + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x1538[16:16]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + heo_rdy.ToString() + "\t" + "//heo_rdy" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x1538[24:24]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + veo_rdy.ToString() + "\t" + "//veo_rdy" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x154C[10:00]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + eye_er.ToString() + "\t" + "//eye_er" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x154C[26:16]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + eye_el.ToString() + "\t" + "//eye_el" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x1628[6:0]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + eye_eu.ToString() + "\t" + "//eye_eu" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x1628[14:8]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + eye_eb.ToString() + "\t" + "//eye_eb" + "\n");
    richTextBox3.AppendText("I2C_EXT" + "\t" + "74" + "\t" + "0x1534[22:16]" + "\t" + "0x00000000" + "\t" + "RO" + "\t" + veo.ToString() + "\t" + "//veo" + "\n");
    richTextBox3.AppendText("\n");
	#endif
	printk("heo_rdy =%4x,veo_rdy=%4x,er=%4x,el=%4x,eu=%4x,eb=%4x,heo=%4x\n", heo_rdy,heo_rdy,eye_er,eye_el,eye_eu,eye_eb,heo);

	
    return veo;
}


void xpon_pma_eye_open(void)
{
	
		   int i;
		   uint leq_gain = 0x1;
		   int j;
		   uint leq_peaking = 0x2;
		   uint fom_num, veo;
		   int k = 0;

		
		  // for (i = 1; i < 4; i = i + 2)
		   {
			   IO_SPHYA_REG_BITS(0, 0x163c, 0, 0, 0x0);    //rg_disb_da_xpon_rx_fe_gain_ctrl
			   IO_SPHYA_REG_BITS(0, 0x1630, 1, 0, leq_gain);	//rg_force_da_xpon_rx_fe_gain_ctrl
			 //  for (j = 0; j < 8; j++)
			   {
				   IO_SPHYA_REG_BITS(0, 0x163c, 8, 8, 0x0);    //rg_disb_da_xpon_rx_fe_peaking_ctrl
				   IO_SPHYA_REG_BITS(0, 0x1630, 10, 8, leq_peaking);   //rg_force_da_xpon_rx_fe_peaking_ctrl
					printk("leq_gain =%x,leq_peaking=%x\n", leq_gain,leq_peaking);
				   //EYE Setting
				   #if 0
				   if (comboBoxY.SelectedItem.ToString() == "bottom-up")
				   {
					   //bottom-up sweep Y index
					   IO_SPHYA_REG_BITS(0, 0x1304, 6, 0, 0x40);  //rg_y_min, 7'h40
					   IO_SPHYA_REG_BITS(0, 0x1304, 14, 8, 0x3F); //rg_y_max, 7'h3F
				   }
				   if (comboBoxY.SelectedItem.ToString() == "upside-down")
				   {
					   //upside-down sweep Y index
					   IO_SPHYA_REG_BITS(0, 0x1304, 6, 0, 0x3F);  //rg_y_min, 7'h3F
					   IO_SPHYA_REG_BITS(0, 0x1304, 14, 8, 0x40); //rg_y_max, 7'h40
				   }
				   else 
				   	#endif
				   {
					   //bi-directional sweep Y index default use bottom-up sweep first
					   IO_SPHYA_REG_BITS(0, 0x1304, 6, 0, 0x40);  //rg_y_min, 7'h40
					   IO_SPHYA_REG_BITS(0, 0x1304, 14, 8, 0x3F); //rg_y_max, 7'h3F
				   }

				   xpon_pma_eye_open_init();

				   //EYE setting, EYE OPEN related
				   IO_SPHYA_REG_BITS(0, 0x1370, 7, 0, 0x80);	   //rg_eq_en_delay, 8'h80
				   IO_SPHYA_REG_BITS(0, 0x1430, 10, 8, 0x4);	   //rg_kpgain, 3'h4
				   IO_SPHYA_REG_BITS(0, 0x1494, 16, 16, 0x0);	   //rg_eq_pi_cal_rst_b, 1'b0

				   IO_SPHYA_REG_BITS(0, 0x1670, 8, 8, 0x1); 	   //rg_disb_rx_and_pical_rstb, 1'b1
				   IO_SPHYA_REG_BITS(0, 0x1664, 8, 8, 0x0); 	   //rg_force_rx_and_pical_rstb, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x1670, 0, 0, 0x1); 	   //rg_disb_ref_and_pical_rstb, 1'b1
				   IO_SPHYA_REG_BITS(0, 0x1664, 0, 0, 0x0); 	   //rg_force_ref_and_pical_rstb, 1'b0
				   
				   //IO_SPHYA_REG_BITS(0, 0x1670, 0, 0, 0x1);		 //rg_disb_rx_or_pical_en, 1'b0   //rg_force_rx_or_pical_en, 
				   //IO_SPHYA_REG_BITS(0, 0x1660, 24, 24, 0x0); 	 //rg_force_rx_or_pical_en, 1'b0  //rg_disb_rx_or_pical_en, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x1660, 24, 24, 0x0);		 //rg_disb_rx_or_pical_en, 1'b0   
				   IO_SPHYA_REG_BITS(0, 0x1654, 8, 8, 0x0); 	 //rg_force_rx_or_pical_en, 1'b0  

				   //reset eye_top
				   IO_SPHYA_REG_BITS(0, 0x1678, 8, 8, 0x0); 	   //rg_disb_eye_reset_plu_o, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 8, 8, 0x1); 	   //rg_force_eye_reset_plu_o, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 8, 8, 0x0); 	   //rg_force_eye_reset_plu_o, 1'b1------->0

				   IO_SPHYA_REG_BITS(0, 0x1678, 16, 16, 0x0);	   //rg_disb_eye_top_en, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x1);	   //rg_force_eye_top_en, 1'b1

				   IO_SPHYA_REG_BITS(0, 0x1658, 0, 0, 0x0); 	   //rg_disb_eq_pi_cal_rdy, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x1648, 0, 0, 0x1); 	   //rg_force_eq_pi_cal_rdy, 1'b1


				   //delay for 2 seconds
				   mdelay(2000);

				   //read out EO results
				   veo = xpon_pma_eye_open_read();

				   //exit eye_top force mode
				   IO_SPHYA_REG_BITS(0, 0x1678, 16, 16, 0x0);	   //rg_disb_eye_top_en, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b1
				   //reset pi_cal_rdy
				   IO_SPHYA_REG_BITS(0, 0x1658, 0, 0, 0x0); 	   //rg_disb_eq_pi_cal_rdy, 1'b0
				   IO_SPHYA_REG_BITS(0, 0x1648, 0, 0, 0x0); 	   //rg_force_eq_pi_cal_rdy, 1'b1


					#if 0
					#if 0
				   if (comboBoxY.SelectedItem.ToString() == "bi-directional")
				   	#endif
				   {
					   //bidirectional sweep Y index would sweep upside-down too
					   IO_SPHYA_REG_BITS(0, 0x1304, 6, 0, 0x3F);  //rg_y_min, 7'h3F
					   IO_SPHYA_REG_BITS(0, 0x1304, 14, 8, 0x40); //rg_y_max, 7'h40

					   xpon_pma_eye_open_init();

					   //EYE setting, EYE OPEN related
					   IO_SPHYA_REG_BITS(0, 0x1370, 7, 0, 0x80);	   //rg_eq_en_delay, 8'h80
					   IO_SPHYA_REG_BITS(0, 0x1430, 10, 8, 0x4);	   //rg_kpgain, 3'h4
					   IO_SPHYA_REG_BITS(0, 0x1494, 16, 16, 0x0);	   //rg_eq_pi_cal_rst_b, 1'b0

					   IO_SPHYA_REG_BITS(0, 0x1670, 8, 8, 0x1); 	   //rg_disb_rx_and_pical_rstb, 1'b1
					   IO_SPHYA_REG_BITS(0, 0x1664, 8, 8, 0x0); 	   //rg_force_rx_and_pical_rstb, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x1670, 0, 0, 0x1); 	   //rg_disb_ref_and_pical_rstb, 1'b1
					   IO_SPHYA_REG_BITS(0, 0x1664, 0, 0, 0x0); 	   //rg_force_ref_and_pical_rstb, 1'b0

					   IO_SPHYA_REG_BITS(0, 0x1670, 0, 0, 0x1); 	   //rg_disb_rx_or_pical_en, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x1660, 24, 24, 0x0);	   //rg_force_rx_or_pical_en, 1'b0

					   //reset eye_top
					   IO_SPHYA_REG_BITS(0, 0x1678, 8, 8, 0x0); 	   //rg_disb_eye_reset_plu_o, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 8, 8, 0x1); 	   //rg_force_eye_reset_plu_o, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 8, 8, 0x0); 	   //rg_force_eye_reset_plu_o, 1'b1------->0

					   IO_SPHYA_REG_BITS(0, 0x1678, 16, 16, 0x0);	   //rg_disb_eye_top_en, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x1);	   //rg_force_eye_top_en, 1'b1

					   IO_SPHYA_REG_BITS(0, 0x1658, 0, 0, 0x0); 	   //rg_disb_eq_pi_cal_rdy, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x1648, 0, 0, 0x1); 	   //rg_force_eq_pi_cal_rdy, 1'b1


					   //delay for 2 seconds
					   msleep(1000);

					   //read out EO results
					   veo = xpon_pma_eye_open_read();

					   //exit eye_top force mode
					   IO_SPHYA_REG_BITS(0, 0x1678, 16, 16, 0x0);	   //rg_disb_eye_top_en, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x166c, 16, 16, 0x0);	   //rg_force_eye_top_en, 1'b1
					   //reset pi_cal_rdy
					   IO_SPHYA_REG_BITS(0, 0x1658, 0, 0, 0x0); 	   //rg_disb_eq_pi_cal_rdy, 1'b0
					   IO_SPHYA_REG_BITS(0, 0x1648, 0, 0, 0x0); 	   //rg_force_eq_pi_cal_rdy, 1'b1
				   }
					#endif
				   leq_peaking++;
				   veo = 0x0;
			   }							 
			   leq_gain = 0x3;
			   leq_peaking = 0x0;
		   }
}


int xpon_pma_eye_scan(void)
{
	return 0; //ang_20180208
}


int xpon_pma_frequency_meter(void)
{
	return 0; //ang_20180208
}

int xpon_pma_trans_ben_active_level(void)
{
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_2,0x0628,24,24,xpon_pma_param.Trans_Ben_Active_Level);
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] Trans_Ben_Active_Level=%d\r\n",__FUNCTION__,__LINE__,xpon_pma_param.Trans_Ben_Active_Level);
	return 0;
}

int xpon_pma_xpon_mode_config(void)
{

	xpon_pma_lcpll_mode_setting();
	xpon_pma_rx_rate();
	xpon_pma_tx_rate();			

	return 0;
}



int phyd_rx_clk_dly_go_through(void)
{
	int value1=0,value2=0,value3=0,value4=0;

	uint read_data=0;

	for(value1=0;value1<=0x7f;value1=value1+16)
	{
		IO_SPHYA_REG_BITS(PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1,				0x0514,	6,	0,	value1);
		mdelay(100);

		for(value2=0;value2<=0x7f;value2=value2+16)
		{
			IO_SPHYA_REG_BITS(PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1,			0x0514,	14,	8,	value1);
			mdelay(100);
			
			for(value3=0;value3<=0x7f;value3=value3+16)
			{
				IO_SPHYA_REG_BITS(PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1,			0x0514,	22,	16,	value1);
				mdelay(100);
				for(value4=0;value4<=0x7f;value4=value4+16)
				{
					IO_SPHYA_REG_BITS(PMA_TOP_DLY_CHAIN_PHYD_TX_CLK_1,			0x0514,	30,	24,	value1);
					mdelay(2000);

					#ifdef TCSUPPORT_CPU_EN7581
						read_data = IO_GPHYREG(EN7581_XEPON_PCS_RX_SYNC_STATUS);
					#else
						read_data = IO_GPHYREG(XEPON_PCS_RX_SYNC_STATUS);
					#endif

					if((read_data& XEPON_PCS_RX_SYNC_STATUS_OK) == XEPON_PCS_RX_SYNC_STATUS_OK)
					{
						printk("Phy Ready. value1=%x,value2=%x,value3=%x,value4=%x\n",value1,value2,value3,value4);

						mdelay(1000);
						printk("FEC1=%x,0xbfb66120=%x\n",IO_GPHYREG(0xBFAF12a0),IO_GPHYREG(0xBFB66120));
						mdelay(1000);
						printk("FEC2=%x,0xbfb66120=%x\n",IO_GPHYREG(0xBFAF12a0),IO_GPHYREG(0xBFB66120));
						mdelay(1000);
						printk("FEC3=%x,0xbfb66120=%x\n",IO_GPHYREG(0xBFAF12a0),IO_GPHYREG(0xBFB66120));
						
					}
				}
			}
		
		}

	}

	return 0;
	
}


int xpon_pma_internal_lpbk_setting(void)
{

	switch(xpon_pma_param.LoopBack_Path)
	{
		case normal_mode:
			xpon_pma_xpon_mode_config();
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,					0x175C, 1,	1,	0x00);//tr2_bist_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,					0x175C, 0,	0,	0x00);//tr2 FIFO en
			break;

//		disable tx_bist_gen of A60972 to accept pcs loopback data	  david 20170505
		case t2r_mode: 
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x00);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 8,	8,	0x00);//rg_bistctl_pat_tx_en	
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 2,	0,	0x04);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175C, 1,	1,	0x00);//tr2_bist_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175C, 0,	0,	0x01);//tr2 FIFO en
			break;
	
//		add one case for A60972 self T2R check		david 20170505
		case pma_t2r_mode:
		//	 enable TX Bist Gen
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x01);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 8,	8,	0x01);//rg_bistctl_pat_tx_en	
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 2,	0,	0x04);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175c, 1,	1,	0x01);//t2r_bist_mode					   
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175c, 0,	0,	0x01);//t2r_FIFO_en 					
			break;

		case r2t_mode:
			 break;
		default:
			break;
	}

	return 0;	//ang_20180208
	
}

int xpon_pma_external_lpbk_setting(void)
{

	switch(xpon_pma_param.LoopBack_Path)
	{
		case normal_mode:
			xpon_pma_xpon_mode_config();
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x00);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,					0x175C, 1,	1,	0x00);//tr2_bist_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,					0x175C, 0,	0,	0x00);//tr2 FIFO en
			break;

//		disable tx_bist_gen of A60972 to accept pcs loopback data	  david 20170505
		case t2r_mode: 
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x00);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 8,	8,	0x00);//rg_bistctl_pat_tx_en	
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175C, 1,	1,	0x00);//tr2_bist_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175C, 0,	0,	0x00);//tr2 FIFO en
			break;
	
//		add one case for A60972 self T2R check		david 20170505
		case pma_t2r_mode:
		//	 enable TX Bist Gen
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x01);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 8,	8,	0x00);//rg_bistctl_pat_tx_en	
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175c, 1,	1,	0x01);//t2r_bist_mode					   
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_T2R_MODE_1,				0x175c, 0,	0,	0x00);//t2r_FIFO_en 					
			break;

		case r2t_mode:
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 2,	0,	0x00);//rg_xpon_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1, 			0x1754, 16, 16, 0x00);//rg_tx_bist_gen_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 8,	8,	0x00);//rg_bistctl_pat_tx_en	
			IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1, 			0x1684, 8,	8,	0x00);//all_lane_prbs_tx_en
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 16,	16,	0x00);//rg_bistctl_pat_rx_check_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 8,	8,	0x00);//rg_r2t_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 8,	8,	0x01);//rg_r2t_mode
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 24,	24,	0x00);//rg_r2t_fifo_en
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,					0x1754, 24,	24,	0x01);//rg_r2t_fifo_en
			//R2T turn on BIST
			IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL, 			0x1500, 16,	16,	0x01);//rg_bistctl_pat_rx_check_en
			 break;
			 


		default:
			break;
}

	return 0;	//ang_20180208
}

int xpon_pma_param_init(void)
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] wan_sel = %d\r\n",__FUNCTION__,__LINE__,gpPhyPriv->wan_sel);

	xpon_pma_param.TX_Bist_Pattern=PATTERN_DISABLE;
	xpon_pma_param.LoopBack_Path=normal_mode;
	xpon_pma_param.Trans_Ben_Active_Level=gpPhyPriv->trans_tx_ben_level;

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			xpon_pma_param.LCPLL_Mode=Rx_pll_1p25G;
			xpon_pma_param.RX_Mode=Rx_Rate_1p25G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p25G_RX_1p25G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			xpon_pma_param.LCPLL_Mode=Rx_pll_2p48832G;
			xpon_pma_param.RX_Mode=Rx_Rate_2p48832G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p24416G;
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
			xpon_pma_param.LCPLL_Mode=RX_pll_9p95328G;
			xpon_pma_param.RX_Mode=RX_Rate_9p95328G;
			xpon_pma_param.TX_Mode=Tx_Rate_2p48832G;
			break;			

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			xpon_pma_param.LCPLL_Mode=RX_pll_10p3125G;
			xpon_pma_param.RX_Mode=RX_Rate_10p3125G;
			xpon_pma_param.TX_Mode=Tx_Rate_1p25G_RX_10p3125G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			xpon_pma_param.LCPLL_Mode=RX_pll_10p3125G;
			xpon_pma_param.RX_Mode=RX_Rate_10p3125G;
			xpon_pma_param.TX_Mode=TX_Rate_10p3125G;
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:
			xpon_pma_param.LCPLL_Mode=RX_pll_9p95328G;
			xpon_pma_param.RX_Mode=RX_Rate_9p95328G;
			xpon_pma_param.TX_Mode=TX_Rate_9p95328G; 
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G:
			xpon_pma_param.LCPLL_Mode=Rx_pll_2p48832G;
			xpon_pma_param.RX_Mode=Rx_Rate_2p48832G;
			xpon_pma_param.TX_Mode=Tx_Rate_2p48832G; 
			break;

		default:
			printk("[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x  0x%8x  *=0x%8x  **=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);


	return 0;

}
int xpon_pma_param_disp(void)
{
	printk("%s (wan_sel = %d)\r\n",__FUNCTION__,gpPhyPriv->wan_sel);
	printk("%% LCPLL_Mode = %d\r\n",xpon_pma_param.LCPLL_Mode);
	printk("%% RX_Mod = %d\r\n",xpon_pma_param.RX_Mode);
	printk("%% TX_Mode = %d\r\n",xpon_pma_param.TX_Mode);
	printk("%% TX_Bist_Pattern = %d\r\n",xpon_pma_param.TX_Bist_Pattern);
	printk("%% LoopBack_Path = %d\r\n",xpon_pma_param.LoopBack_Path);
	printk("%% Trans_Ben_Active_Level = %d\r\n",xpon_pma_param.Trans_Ben_Active_Level);
	return 0;
}


void xpon_pma_manual_calibration(void)
{

	uint read_data = 0;
	//*****************************										
	//manuall	calibration									
	//*****************************	

	if(a60972_excute_flag==0)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//reset A60972
		printk("\r\na60972_init\r\n");
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
		mdelay(1);
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);

		#ifdef TCSUPPORT_CPU_EN7581
		IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_ON);
		#else
		IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_ON);
		#endif
		
		printk("\r\nEN7580 << XEPON >> logic reset hold\r\n");

		
		a60972_init();

		
	}
	if(a60972_excute_flag==1)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 9. XPON_10G_EPON_10G_10G//
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,31,31,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,5,5,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,30,29,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,28,28,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,3,3,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,2,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,4,4,0x0);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,31,31,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,30,30,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,17,17,0x0);

		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,13,13,0x0);

		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,23,18,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,25,25,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,24,24,0x0);


		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,15,15,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,14,14,0x0);

		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,29,29,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,28,28,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,27,27,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_LSDATA_EN_0,0x114C,26,26,0x1);


	}	
	if(a60972_excute_flag==2)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//AEVB(3_inch) + ISI(2_inch), FIR Setting
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,10,10,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,27,24,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,23,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,18,14,0xa);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,13,11,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_EN_0,0x1148,9,6,0x0);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,9,8,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,7,6,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,31,16,0x1);

	}
	if(a60972_excute_flag==3)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//Mode Select
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,31,31,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,30,30,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,29,24,0x1F);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,15,12,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_BENTX_BIAS_EN_0,0x1104,11,11,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_0,0x1554,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_1,0x1558,0,0,0x0);
		//IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,0,0,0x0);
		//IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_CALIB_2,0x155c,8,8,0x0);

	}
	if(a60972_excute_flag==4)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 1. XPON BIST Pattern//
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x6081fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xff1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);

	}
	if(a60972_excute_flag==5)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
		
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
		//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
		IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);

	}
	if(a60972_excute_flag==51)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);

	}
	if(a60972_excute_flag==52)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//

		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
		//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);

	}
	if(a60972_excute_flag==521)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//

		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);


	}
	if(a60972_excute_flag==522)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);



	}
	if(a60972_excute_flag==523)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);

	}
	if(a60972_excute_flag==524)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
		//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);


	}

	
	if(a60972_excute_flag==53)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
		//IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);


	}
	if(a60972_excute_flag==54)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
		IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);


	}
	if(a60972_excute_flag==55)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
			
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);

		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);

		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);

	}

	if(a60972_excute_flag==551)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//
			
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	}

	if(a60972_excute_flag==552)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);


	}
	if(a60972_excute_flag==553)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);

	}
	if(a60972_excute_flag==554)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);

	}
	if(a60972_excute_flag==555)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	}

	if(a60972_excute_flag==556)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);

		IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);

	}

	
	if(a60972_excute_flag==56)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//

		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);

		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);

		
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);

		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);

	}
	if(a60972_excute_flag==561)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		// 2. RX set//

		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
		//IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	}

	if(a60972_excute_flag==562)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);

	}

	if(a60972_excute_flag==563)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	}	

	if(a60972_excute_flag==564)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);

	}
	if(a60972_excute_flag==565)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);

	}


	if(a60972_excute_flag==6)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//CDR LPF
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);

		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
		

		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
		IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);

	}
	if(a60972_excute_flag==7)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//*****************************
		//manuall calibration
		//*****************************
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
		
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
		
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);

	}

	if(a60972_excute_flag==71)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//*****************************
		//manuall calibration
		//*****************************
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);

			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
			
	}
	if(a60972_excute_flag==711)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//*****************************
		//manuall calibration
		//*****************************
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);

			
	}
	if(a60972_excute_flag==712)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
			
	}


	if(a60972_excute_flag==72)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

			
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);

				IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);

				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);

					IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
					IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
					IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
					IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
					IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);

				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
				
				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
				
				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);

	}

	if(a60972_excute_flag==73)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);

				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);

					IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
					IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
				
				IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
				
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	}

	if(a60972_excute_flag==74)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

			IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);

			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);

	}

	if(a60972_excute_flag==75)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);

	}

	if(a60972_excute_flag==76)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	}

	if(a60972_excute_flag==77)
	{
			PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
			IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
		
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);

	}
	
		if(a60972_excute_flag==771)
		{
				PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
				IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);

	
		}

		if(a60972_excute_flag==772)
		{
				PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
			
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	
		}

	
	if(a60972_excute_flag==8)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		//*****************************
		//CDR L2D
		//*****************************
		
		
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
		
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
		
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x8ff1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
		
		//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
		
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);
		
		//new add
		//IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_2,0x0628,24,24,0x1);

	}

	if(a60972_excute_flag==81)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
		IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);

	}

	if(a60972_excute_flag==82)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
		
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x8ff1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);

	}
	if(a60972_excute_flag==83)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
			IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);

	}
	
	if(a60972_excute_flag==84)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);

	}
	

	if(a60972_excute_flag==85)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
		

	}

	if(a60972_excute_flag==86)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);

		IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);

		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
		
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
			IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
		
		IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);


	}

	if(a60972_excute_flag==87)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		
				//IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
			
		IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);

	}	



	
	if(a60972_excute_flag==9)
	{
		PON_PHY_PRINT(PHY_MSG_TRACE,"%s:%d a60972_excute_flag=%d\r\n",__FUNCTION__,__LINE__,a60972_excute_flag);
		read_data = EN7580_IF_RG_A60972_CTRL_IF_SW_RESET_ONLY;
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
		mdelay(1);
		read_data = EN7580_IF_RG_A60972_CTRL_NORMAL_OPERATION;
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL, read_data);
		printk("\r\nEN7580_IF_RG_A60972_CTRL_IF_SW_RESET Done\r\n");
		
		printk("\r\nEN7580 << XEPON >> logic reset release\r\n");
		#ifdef TCSUPPORT_CPU_EN7581
		IO_SPHYREG(EN7581_XEPON_PCS_LOGIC_RST, EN7581_XEPON_PCS_LOGIC_RST_OFF);
		#else
		IO_SPHYREG(XEPON_PCS_LOGIC_RST, XEPON_PCS_LOGIC_RST_OFF);
		#endif
		

	}

		
}
void xpon_pma_manual_init(void)
{


	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_0,0x1730,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_1,0x1734,31,16,0xc);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,7,0,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,23,16,0x6);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_2,0x1738,27,24,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PLL_TDC_FREQDET_3,0x173C,15,0,0x200);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_1,0x1420,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,15,0,0x300);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_2,0x1424,31,16,0x3bc);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,7,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,11,8,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_3,0x1428,23,16,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,3,0,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FREQ_DET_4,0x142C,23,8,0x1000);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,2,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_0,0x1340,23,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_1,0x1344,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_2,0x1348,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_3,0x134C,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_4,0x1350,31,16,0x8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,15,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_5,0x1354,31,16,0xc8);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_CTRL_6,0x1358,15,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,15,8,0x2e);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_1,0x1440,25,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,11,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_CAL_2,0x1444,17,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,25,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x7);
	IO_SPHYA_REG_BITS(PMA_TOP_ADD_R2T_MODE_1,0x0624,17,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,10,0,0x1f0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_0,0x1300,26,16,0x234);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,31,24,0xff);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_2,0x1338,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,6,0,0x67);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,14,8,0x19);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_1,0x1304,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_2,0x1308,19,0,0x18);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEINDEX_CTRL_3,0x130C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_0,0x1320,15,8,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,10,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYEOPENING_CTRL_1,0x1324,22,16,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,9,0,0xd0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_EYE_TOP_EYECNT_CTRL_0,0x1330,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,7,0,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,18,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_0,0x1370,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_1,0x1374,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,1,0,0x3);
	IO_SPHYA_REG_BITS(XPON_PMA_PHY_EQ_CTRL_2,0x1378,12,8,0x5);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,8,8,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,11,8,0x9);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,22,16,0x1f);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_BLWC,0x1410,29,23,0x21);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_TX_RST_B,0x1550,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,23,23,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,22,22,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,21,21,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_RESERVE1_0,0x1128,7,6,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_0,0x168C,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_DA_XPON_PWDB_1,0x1690,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,12,10,0x5);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_TX_RESERVED_0,0x1150,5,4,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_RESERVED_1,0x1714,1,0,0x2);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_4,0x165c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,19,17,0x4);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,4,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,1,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,10,8,0x02);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x01000);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,23,5,0x7F000);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_1,0x1640,24,24,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_2,0x1644,24,24,0x01);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,22,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_1,0x1634,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,6,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,14,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,21,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_2,0x1638,30,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_PR_CKREF_DIV_0,0x1124,27,27,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_REV_0_0,0x110c,16,16,0x0);
	IO_SPHYA_REG_BITS(PMA_TOP_GPIO_MUX_CTRL,0x0738,17,16,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_10,0x174c,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x01);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_PI_CAL,0x1430,10,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_5,0x1660,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_6,0x1654,8,8,0x0);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,8,8,0x01);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_PDOS_CTRL_0,0x1490,18,16,0x02);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_1,0x1498,0,0,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_6,0x1670,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_7,0x1664,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,16,16,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_RX_FEOS,0x1400,7,0,0x30);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_RESET_0,0x1494,8,8,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_7,0x1674,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_8,0x1668,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,24,24,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,0,0,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,16,16,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DEBUG_0,0x149c,24,24,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,6,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,14,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,22,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_0,0x1524,30,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_1,0x1528,6,0,0x0);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_9,0x1548,13,8,0x0);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_TORGS_DEBUG_2,0x152c,22,16,0x0);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_0,0x135C,0,0,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_0,0x1364,0,0,0x00);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_DISB_CTRL_1,0x1360,8,8,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,8,8,0x01);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RX_DISB_MODE_0,0x163C,16,16,0x00);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_FORCE_MODE_0,0x1630,16,16,0x01);
	IO_SPHYA_REG_BITS(XPON_PMA_RX_CTRL_SEQUENCE_FORCE_CTRL_1,0x1368,24,24,0x01);
	
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,4,0,0x5);
	
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_ALIGN_PAT,0x1504,31,0,0x8ff1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_INITIAL_SEED,0x1514,30,0,0xFF1fd53);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_FAIL_THRESHOLD,0x1520,15,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PROGRAM_PAT_2,0x1510,16,16,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,0,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,8,8,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_EVENT,0x1518,16,16,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_PRBS_ERRCNT,0x151c,15,0,0xFF);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,24,24,0x0);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_MJV_EN_0,0x1118,3,3,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CDR_LPF_SETVALUE_0,0x111c,31,13,0x00000);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_3,0x1234,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,20,20,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,19,19,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,18,18,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,17,17,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_CLKPATH_ACBUF_EN_0,0x1144,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_0,0x1210,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,0,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_5,0x1224,0,0,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_PW_4,0x1220,16,16,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_SS_LCPLL_TDC_FLT_1,0x122c,11,8,0x7);
	
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,2,0,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_SS_BIST_1,0x1684,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,8,8,0x1);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x0);
	IO_SPHYA_REG_BITS(XPON_PMA_ADD_XPON_MODE_1,0x1754,24,24,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_BISTCTL_CONTROL,0x1500,16,16,0x1);
	
	IO_SPHYA_REG_BITS(XPON_PMA_RG_XPON_RX_SIGDET_VTH_SEL_0,0x112C,26,25,0x1);


}



int xpon_pma_init(void)
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"%s\r\n",__FUNCTION__);

	//param init based on wan_sel
	xpon_pma_param_init();
	xpon_pma_param_disp();
	
	return 0;
}

void a60972_trans_tx_control(unchar val) 
{
	PON_PHY_PRINT(PHY_MSG_TRACE,"[%s:%d] trans_tx_control = %d\r\n",__FUNCTION__,__LINE__,val);

	if(PHY_ENABLE == val)
	{
		IO_SPHYA_REG(PMA_TOP_TRAN_TRX_CNTRL,0x0720,PMA_TOP_TRAN_TRX_CNTRL_TX_EN);
	}
	else if (PHY_DISABLE == val) 
	{
		IO_SPHYA_REG(PMA_TOP_TRAN_TRX_CNTRL,0x0720,PMA_TOP_TRAN_TRX_CNTRL_TX_DIS);
	}
	else
		PON_PHY_PRINT(PHY_MSG_ERR, "[%s:%d] Tx ctl type error.\n",__FUNCTION__,__LINE__);

}

int a60972_init(void) 
{

//	printk("[%s:%d] gpPhyPriv->wan_sel = %d\n",__FUNCTION__,__LINE__,gpPhyPriv->wan_sel);

#if 1

	switch(gpPhyPriv->wan_sel)
	{
		case SCU_WAN_CONF_REG_WAN_SEL_EPON:
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G:			
			a60972_epon_script_init();
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_GPON:
			a60972_gpon_script_init();
			break;

		case SCU_WAN_CONF_REG_WAN_SEL_XGPON:
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G:
			a60972_xgpon_2_xepon_script_init();
			break;			

		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G:
			a60972_xgpon_2_xepon_script_init();
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G:
			a60972_xgpon_2_xepon_script_init();
			break;
			
		case SCU_WAN_CONF_REG_WAN_SEL_XGSPON:			
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G:		
		case SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G:
			a60972_xgpon_2_xepon_script_init();
			break;

		default:
			PON_PHY_PRINT(PHY_MSG_ERR, "[%s:%d] UNKNOWN mode\n",__FUNCTION__,__LINE__);

			return PHY_FAILURE;
	}
	//PON_PHY_PRINT(PHY_MSG_TRACE,"&=0x%8x	0x%8x  *=0x%8x	**=0x%8x	pon_phy_func_init\n",&pPonPhyFunc,pPonPhyFunc,*pPonPhyFunc,**pPonPhyFunc);

#else
	a60972_xgpon_2_xepon_script_init();
#endif

	return PHY_SUCCESS;

}



void a60972_dump(void) //manual mode: default is 10G EPON 
{
	UINT32 i=0,phy_reg_total_num=0;
	PPHY_REG_ENTRY phy_reg_all=NULL;

	phy_reg_total_num=701;
	phy_reg_all=a60972_reg_all;
	
	printk("\r\n%-48s	 address=value\r\n","_____ A60972 REG DUMP _____");

	//printk("\r\n%-44s	 address=value\r\n","_____ A60972 PMA TOP REG DUMP _____");
	for(i=0;i<=254;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		mdelay(1);
		printk("%-48s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	//printk("\r\n%-44s	 address=value\r\n","_____ A60972 XPON PMA REG DUMP _____");
	for(i=255;i<=458;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		mdelay(1);
		printk("%-48s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}
	//printk("\r\n%-44s	 address=value\r\n","_____ A60972 XFI PMA REG DUMP _____");
	for(i=459;i<=phy_reg_total_num;i++)
	{
		(phy_reg_all+i)->def=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		mdelay(1);
		printk("%-48s =0x%08x=0x%08x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
	}

#if	PHYA_REG_READ_WRITE_TEST
	//Register Read/Write test
	printk("\r\n___ A60972_REG_READ_WRITE_TEST ___ ALL ZEROS ___\r\n");

	for(i=0;i<=123;i++)
	{
		IO_SPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr,PHYA_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHYA_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	for(i=124;i<=131;i++)
	{
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		printk("=N/A");
	}
	for(i=132;i<=phy_reg_total_num;i++)
	{
		IO_SPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr,PHYA_REG_ALL_ZEROS);
		(phy_reg_all+i)->cur=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHYA_REG_ALL_ZEROS!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_0");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ A60972_REG_READ_WRITE_TEST ___ ALL ONES ___\r\n");

	for(i=0;i<=123;i++)
	{
		IO_SPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr,PHYA_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHYA_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	for(i=124;i<=131;i++)
	{
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		printk("=N/A");
	}
	for(i=132;i<=phy_reg_total_num;i++)
	{
		IO_SPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr,PHYA_REG_ALL_ONES);
		(phy_reg_all+i)->cur=IO_GPHYA_REG(A60972_DUMMY_NAME,(phy_reg_all+i)->addr);
		printk("\r\n%-48s 0x%08x=0x%08x",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->cur);
		if(PHYA_REG_ALL_ONES!=(phy_reg_all+i)->cur)
		{
			printk("=Not_all_1");
		}
		else
		{
			printk("=Pass");
		}
	}
	printk("\r\n___ PHY_REG_READ_WRITE_TEST ___ END ___\r\n");
#endif
		


}


static int a60972_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
	char val_string[64], cmd[64] ,subcmd[64];
	uint dec1=0, dec2=0, dec3=0, dec4=0;
	uint hex1=0, hex2=0, hex3=0, hex4=0;
	uint input1=0, input2=0, input3=0 ,input4=0;

	uint read_data = 0;
	uint write_data = 0;
	memset(val_string,0,(sizeof(char)*64));
	memset(cmd,0,(sizeof(char)*64));
	memset(subcmd,0,(sizeof(char)*64));
	
	if (count > sizeof(val_string) - 1)
		return -EINVAL ;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT ;

	sscanf(val_string, "%s %d %d %d %d", 	cmd, &dec1, 	&dec2, 		&dec3,		&dec4);
	sscanf(val_string, "%s %s %d %d %d", 	cmd, subcmd, 	&dec2, 		&dec3,		&dec4);
	sscanf(val_string, "%s %x %x %x %x", 	cmd, &hex1, 	&hex2, 		&hex3,		&hex4);
	sscanf(val_string, "%s %s %x %x %x", 	cmd, subcmd, 	&hex2, 		&hex3,		&hex4);
	sscanf(val_string, "%s %x %d %d %x",	cmd, &input1, 	&input2,	&input3,	&input4);//Roger_a60972


	if ((!strcmp(cmd, "phya_reg_get"))||(!strcmp(cmd, "read"))) {
		printk("0x%8x = %8x\r\n",hex1,IO_GPHYA_REG(A60972_DUMMY_NAME,hex1));	
	}
	else if ((!strcmp(cmd, "phya_reg_set"))||(!strcmp(cmd, "write"))){
		IO_SPHYA_REG(A60972_DUMMY_NAME,hex1, hex2);
		printk("0x%8x = %8x\r\n",hex1,hex2);	
	}
	else if(!strcmp(cmd, "bits_get")){
		printk("0x%8x[%d:%d] = %8x\r\n",input1,input2,input3,IO_GPHYA_REG_BITS(A60972_DUMMY_NAME,input1,input2,input3));
	}
	else if(!strcmp(cmd, "bits_set")){
		IO_SPHYA_REG_BITS(A60972_DUMMY_NAME,input1,input2,input3,input4);
		printk("0x%8x[%d:%d] = %8x\r\n",input1,input2,input3,input4);
	}
	else if(!strcmp(cmd, "calibration")){
		printk("A60972 EYE_OPEN_Calibration\n");
		xpon_pma_eye_open_calibration();
	}
	else if(!strcmp(cmd, "eye_open")){
		printk("A60972 EYE_OPEN\n");
		xpon_pma_eye_open();
	}
	else if (!strcmp(cmd, "script"))
	{
		a60972_script_init();
	}
	else if (!strcmp(cmd, "disp")){
		xpon_pma_param_disp();
	}
	else if (!strcmp(cmd, "dump"))
	{
		a60972_dump();
	}
	else if (!strcmp(cmd, "rxfifo"))
	{
		read_data = IO_GPHYA_REG(A60972_DUMMY_NAME,XPON_PMA_ADD_RO_RX2ANA_3);
		printk("XPON_PMA_ADD_RO_RX2ANA_3 = 0x%x\n",read_data);
		printk("XPON_PMA_ADD_RO_RX2ANA_3_FIFO_FULL_CNT_ = 0x%x\n",(read_data>>XPON_PMA_ADD_RO_RX2ANA_3_FIFO_FULL_CNT_OFFSET)&XPON_PMA_ADD_RO_RX2ANA_3_FIFO_FULL_CNT_MASK);
		printk("XPON_PMA_ADD_RO_RX2ANA_3_FIFO_EMPTY_CNT = 0x%x\n",(read_data>>XPON_PMA_ADD_RO_RX2ANA_3_FIFO_EMPTY_CNT_OFFSET)&XPON_PMA_ADD_RO_RX2ANA_3_FIFO_EMPTY_CNT_MASK);
		printk("XPON_PMA_ADD_RO_RX2ANA_3_PI_CAL_DATA_OUT = 0x%x\n",(read_data>>XPON_PMA_ADD_RO_RX2ANA_3_PI_CAL_DATA_OUT_OFFSET)&XPON_PMA_ADD_RO_RX2ANA_3_PI_CAL_DATA_OUT_MASK);
	}
	else if (!strcmp(cmd, "reset"))
	{
		phy_pma_reset();
	}
	else if (!strcmp(cmd, "topreset"))
	{

		//hold xpon phy top reset 
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
		read_data = GET_SCU_RST_RG(); 
		read_data |=0x01;
		SET_SCU_RST_RG(read_data);
#else
		read_data = IO_GPHYREG(TOP_RST_CTRL_SW2);
		read_data |=0x01;
		IO_SPHYREG(TOP_RST_CTRL_SW2, read_data);

#endif


		//reset A60972
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_ON);
		mdelay(1);
		IO_SPHYREG(EN7580_IF_RG_A60972_CTRL,EN7580_IF_RG_A60972_CTRL_A60972_SW_RESET_OFF);

		
		//init A60972
		a60972_script_init();
				
		//release xpon phy top reset 
#ifdef TCSUPPORT_CPU_ARMV8 //julia_arm //7523
		//release xpon phy top reset 
		read_data &=~(0x01);
		SET_SCU_RST_RG(read_data);		
#else
		//release xpon phy top reset 
		read_data = IO_GPHYREG(TOP_RST_CTRL_SW2);
		read_data &=~(0x01);
		IO_SPHYREG(TOP_RST_CTRL_SW2, read_data);
#endif

	}
	else if (!strcmp(cmd, "rxreset"))
	{
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x0);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x0);

		
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,8,8,0x01);
		IO_SPHYA_REG_BITS(PMA_TOP_SW_RST_0,0x0668,16,16,0x01);		
	}
	else if (!strcmp(cmd, "manual"))
	{
		if(dec1<=999)
		{
			a60972_excute_flag=dec1;
			xpon_pma_manual_calibration();
		}
		else if(dec1>999)
		{
			a60972_excute_flag=0;
			xpon_pma_manual_calibration();
			a60972_excute_flag=1;
			xpon_pma_manual_calibration();
			a60972_excute_flag=2;
			xpon_pma_manual_calibration();
			a60972_excute_flag=3;
			xpon_pma_manual_calibration();
			a60972_excute_flag=4;
			xpon_pma_manual_calibration();
			a60972_excute_flag=5;
			xpon_pma_manual_calibration();
			a60972_excute_flag=6;
			xpon_pma_manual_calibration();
			a60972_excute_flag=7;
			xpon_pma_manual_calibration();
			a60972_excute_flag=8;
			xpon_pma_manual_calibration();
			a60972_excute_flag=9;
			xpon_pma_manual_calibration();

		}
		
	}
	else if(!strcmp(cmd, "phyd_rx_clk"))
	{
		phyd_rx_clk_dly_go_through();
	}
	else if(!strcmp(cmd, "rx_clk"))
	{
		phyd_rx_clk_dly_go_through();
	}
	else if(!strcmp(cmd, "lpbk_path"))
	{
		xpon_pma_param.LoopBack_Path=dec1;
		printk("LoopBack_Path=%d\r\n",dec1);
		xpon_pma_internal_lpbk_setting();
	}
	else if((!strcmp(cmd, "tx_bist_pattern"))||(!strcmp(cmd, "rogue")))
	{
		xpon_pma_param.TX_Bist_Pattern=dec1;
		printk("tx_bist_pattern=%d\r\n",dec1);
		xpon_pma_tx_bist_pattern_setting();
	}
	else if(!strcmp(cmd, "rx_bist_pattern"))
	{
		xpon_pma_rx_bist_check_setting(dec1);
	}
	else if(!strcmp(cmd, "rx_bist_check"))
	{
		xpon_pma_rx_bist_check_result();
		
	}
	else if(!strcmp(cmd, "rx_bist_error_cnt"))
	{
		xpon_pma_rx_bist_err_cnt_read();

	}

	else if(!strcmp(cmd, "lpbk_pcs_pma"))
	{
		printk("VU440+A60972 T2R Loopback test\r\n");	
		//select pcs&pma loopback path
		xpon_pma_param.LoopBack_Path=1;
		printk("LoopBack_Path=%d\r\n",xpon_pma_param.LoopBack_Path);
		xpon_pma_internal_lpbk_setting();	
		//set A60972 BIST gen & check 
		xpon_pma_param.TX_Bist_Pattern=2;
		printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
		xpon_pma_tx_bist_pattern_setting();
		xpon_pma_rx_bist_check_setting(2);
		//start PRBS gen at the pcs side
		xpon_pma_interface_bist_sel_setting();	
		mdelay(1000);	
		//bist check on both pma side and pcs sides of interface		
		xpon_pma_rx_bist_check_result();
		xpon_pma_interface_bist_check_setting();
		printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");		
	}
	// david 20170505
	else if(!strcmp(cmd, "lpbk_pma_only"))
	{
		printk("A60972 T2R Loopback test\r\n");	
		//select pcs&pma loopback path
		xpon_pma_param.LoopBack_Path=2;
		printk("LoopBack_Path=%d\r\n",xpon_pma_param.LoopBack_Path);
		xpon_pma_internal_lpbk_setting();	
		//set A60972 BIST gen & check 
		xpon_pma_param.TX_Bist_Pattern=2;
		printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
		xpon_pma_tx_bist_pattern_setting(); 
		xpon_pma_rx_bist_check_setting(2);
		mdelay(1000);	
		//bist check on both pma side and pcs side of interface 	
		xpon_pma_rx_bist_check_result();
		xpon_pma_interface_bist_check_setting();
		printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");		
	}
	
	else if(!strcmp(cmd, "lpbk_test"))
	{
		//select pcs&pma loopback path
		if(dec1==0)
		{	
			printk("LoopBack_Path is internal! \r\n");

			if(dec2==1)  //loopback t2r whole chip
			{
				xpon_pma_param.LoopBack_Path=1;
				xpon_pma_internal_lpbk_setting();
				//set A60972 BIST gen & check 
				xpon_pma_param.TX_Bist_Pattern=2;
				printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
				xpon_pma_tx_bist_pattern_setting();
				xpon_pma_rx_bist_check_setting(2);
				//start PRBS gen at the pcs side
				xpon_pma_interface_bist_sel_setting();	
				mdelay(1000);	
				//bist check on both pma side and pcs sides of interface		
				xpon_pma_rx_bist_check_result();
				xpon_pma_interface_bist_check_setting();
				printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");
			}
			else if(dec2==2)  //loopback t2r pma only
			{
				xpon_pma_param.LoopBack_Path=2;
				xpon_pma_internal_lpbk_setting();
				//set A60972 BIST gen & check 
				xpon_pma_param.TX_Bist_Pattern=2;
				printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
				xpon_pma_tx_bist_pattern_setting(); 
				xpon_pma_rx_bist_check_setting(2);
				mdelay(1000);	
				//bist check on both pma side and pcs side of interface 	
				xpon_pma_rx_bist_check_result();
				xpon_pma_interface_bist_check_setting();
				printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");
			}

		}
		else if(dec1==1)
		{
			printk("LoopBack_Path is external! \r\n");

			if(dec2==1)   //loopback t2r whole chip
			{
				xpon_pma_param.LoopBack_Path=1;
				xpon_pma_external_lpbk_setting();
				//set A60972 BIST gen & check 
				xpon_pma_param.TX_Bist_Pattern=2;
				printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
				xpon_pma_tx_bist_pattern_setting();
				xpon_pma_rx_bist_check_setting(2);
				//start PRBS gen at the pcs side
				xpon_pma_interface_bist_sel_setting();	
				mdelay(1000);	
				//bist check on both pma side and pcs sides of interface		
				xpon_pma_rx_bist_check_result();
				xpon_pma_interface_bist_check_setting();
				printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");
			}
			else if(dec2==2)  //loopback t2r pma only
			{
				xpon_pma_param.LoopBack_Path=2;
				xpon_pma_external_lpbk_setting();
				//set A60972 BIST gen & check 
				xpon_pma_param.TX_Bist_Pattern=2;
				printk("tx_bist_pattern=%d\r\n",xpon_pma_param.TX_Bist_Pattern);
				xpon_pma_tx_bist_pattern_setting(); 
				xpon_pma_rx_bist_check_setting(2);
				mdelay(1000);	
				//bist check on both pma side and pcs side of interface 	
				xpon_pma_rx_bist_check_result();
				xpon_pma_interface_bist_check_setting();
				printk("if fail, try : echo rx_bist_check > /proc/a60972/debug\r\n");
			}
			else if(dec2==3)  //loopback r2t 
			{
				xpon_pma_param.LoopBack_Path=3;
				xpon_pma_external_lpbk_setting();
				printk("This is R2T mode! \r\n");
				// check tx output by JBERT
			}
		}
	}

	else if(!strcmp(cmd, "reload_t2r"))
	{
		a60972_init_for_External_T2R_lpbk();
	}
		
	else if(!strcmp(cmd, "tx_check"))
	{
		printk("VU440 to A60972 TX quality test\r\n");
		//select pcs&pma loopback path
		xpon_pma_param.LoopBack_Path=1;
		printk("LoopBack_Path=%d\r\n",xpon_pma_param.LoopBack_Path);
		xpon_pma_internal_lpbk_setting();
		//set tx quality test pattern
		xpon_pcs_tx_quality_check();
		xpon_pma_param.TX_Bist_Pattern=16; 
		//start PRBS gen at the pcs side
		xpon_pma_interface_bist_sel_setting();
		//print check result
		xpon_pcs_pma_tx_quality_check_result();
	}
	else if(!strcmp(cmd, "if_rogue"))
	{
		if(dec1==1)
		{
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, (EN7580_IF_RG_INTERFACE_0_BIST_PAT_SEL_PRBS7|EN7580_IF_RG_INTERFACE_0_BIST_PAT_TX_EN));
		}
		else if(dec1==0)
		{
			IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, 0);
		}
		printk("0x%8x = %8x\r\n",EN7580_IF_RG_INTERFACE_0,IO_GPHYREG(EN7580_IF_RG_INTERFACE_0));
	}
	else if(!strcmp(cmd, "if_bist_check"))
	{
		read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_9);
		printk("EN7580_IF_RG_INTERFACE_9(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_9,read_data);
		printk("PRBS Align %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_DONE)?"Done":"Not Done");		
		printk("PRBS Check %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_FAIL)?"Fail":"Pass");
		printk("PRBS Compare %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_COMPARING)?"Ongoing":"Not yet");
	}
	else if(!strcmp(cmd, "if_lpbk"))
	{
		xpon_pma_param.LoopBack_Path=1;
		printk("LoopBack_Path=%d\r\n",xpon_pma_param.LoopBack_Path);
		xpon_pma_internal_lpbk_setting();

		IO_SPHYREG(EN7580_IF_RG_INTERFACE_0, (EN7580_IF_RG_INTERFACE_0_BIST_PAT_SEL_PRBS7|EN7580_IF_RG_INTERFACE_0_BIST_PAT_TX_EN));
		read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_0);
		printk("EN7580_IF_RG_INTERFACE_0(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_0,read_data);

		IO_SPHYREG(EN7580_IF_RG_INTERFACE_1, 0x6081fd53);
		read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_1);
		printk("EN7580_IF_RG_INTERFACE_0(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_1,read_data);
	

		mdelay(1000);

		read_data = IO_GPHYREG(EN7580_IF_RG_INTERFACE_9);
		printk("EN7580_IF_RG_INTERFACE_9(0x%x) = 0x%x\n",EN7580_IF_RG_INTERFACE_9,read_data);
		printk("PRBS Align %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_DONE)?"Done":"Not Done");		
		printk("PRBS Check %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_FAIL)?"Fail":"Pass");
		printk("PRBS Compare %s\n",(read_data&EN7580_IF_RG_INTERFACE_9_BIST_PRBS_COMPARING)?"Ongoing":"Not yet");
		printk("if fail, try : echo if_bist_check > /proc/a60972/debug\r\n");
	}
	else if(!strcmp(cmd, "tx_raw_dump"))
	{
		printk("idx ,0x%8x,  en  ,  data\n",PMA_TOP_XPON_TX_RAWDATA_DATA);
		for(write_data=1;write_data<dec1;write_data++)
		{
			IO_SPHYA_REG(A60972_DUMMY_NAME,PMA_TOP_XPON_TX_RAWDATA_ADDR,write_data);
			read_data=IO_GPHYA_REG(A60972_DUMMY_NAME,PMA_TOP_XPON_TX_RAWDATA_DATA);
			printk("%4d,0x%8x,  %4x,  %4x\n",write_data,read_data,(read_data&0xffff0000)>>16,read_data&0x0000ffff);
		}
	}
	else if(!strcmp(cmd, "rx_raw_dump"))
	{
		printk("idx ,0x%8x,  en  ,  data\n",PMA_TOP_XPON_RX_RAWDATA_DATA);
		for(write_data=1;write_data<dec1;write_data++)
		{
			IO_SPHYA_REG(A60972_DUMMY_NAME,PMA_TOP_XPON_RX_RAWDATA_ADDR,write_data);
			read_data=IO_GPHYA_REG(A60972_DUMMY_NAME,PMA_TOP_XPON_RX_RAWDATA_DATA);
			printk("0x%8x,0x%8x,  %4x,  %4x\n",write_data,read_data,(read_data&0xffff0000)>>16,read_data&0x0000ffff);	//ang_20180208, add print write_data
		}
	}

	return count ;
}

static int a60972_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	int index=0 ;

	char buf_if[10] = {0};


	off_t pos=0, begin=0 ;


	CHK_BUF() ;
	
	*eof = 1 ;

done:
	*start = buf + (off - begin) ;
	index -= (off - begin) ;
	if(index<0) 	index = 0 ;
	if(index>count) 	index = count ;

	return index ;
}

//*****************************************************************************
//*****************************************************************************
static struct proc_dir_entry *a60972_proc_dir=NULL; 
static struct proc_dir_entry *a60972_proc=NULL;
//*****************************************************************************
int xpon_pma_debug_init(void) 
{

	/* create proc node */
	a60972_proc_dir = proc_mkdir("a60972", NULL);
	if(a60972_proc_dir){
    
		a60972_proc = create_proc_entry("debug", 0, a60972_proc_dir);
	    if(a60972_proc) {
		    a60972_proc->write_proc = a60972_write_proc;
		    a60972_proc->read_proc = a60972_read_proc;
	    }   

	}

	return 0 ;
}
//*****************************************************************************
int xpon_pma_debug_deinit(void){

	if(a60972_proc){
		remove_proc_entry("debug", a60972_proc_dir);
	}

    remove_proc_entry("a60972",NULL);
	return 0 ;
}
//*****************************************************************************
//*****************************************************************************

#endif
