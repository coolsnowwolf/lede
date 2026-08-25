#ifndef _PHY_REG_H_
#define _PHY_REG_H_

#include "phy_reg_util.h"
//#include "mt752x_reg.h"
//#include "en7580_reg.h"
//#include "a60972_reg.h"
/*   ===========  NOT PHY REG, may be used by XG, needs confirm  ===========   */

/* top register*/
#define TOP_TEST_MISC0_CTRL								0xbfb00380 	/*Use reset PON PHY*/
#define TOP_CSR_GPIO_SHARE								0xbfb00860
#define TOP_CSR_PBUS_CTRL								0xbfb0092c

/*GPIO register*/
#define GPIO_CSR_CTRL									0xbfbf0200
#define GPIO_CSR_DATA									0xbfbf0204
#define GPIO_CSR_OE										0xbfbf0214
#define GPIO_CSR_OE1									0xbfbf0278
#define GPIO_CSR_CTRL1									0xbfbf0220

#define GPIO_CSR_CTRL3									0xbfbf0264
#define GPIO_CSR_DATA1									0xbfbf0270
#define GPIO_CSR_OE1								    0xbfbf0278

#define GPIO_CSR_CTRL2									0xbfbf0260


#define IOMUX_Control_1_register						0xbfa20104    //ASIC 0xbfa2 Gordon
#define TOP_LED1_MODE					                (1<<10)

#define FE_GENTIME										0xbfb50020   //FPGA build date

#define RG_GPIO_PON_MODE			            		(1<<15)
#define RG_PON_I2C_MODE			            			(1<<0)
#define RG_GPIO_XPON_MODE							(0xBFA20218)

/*  SCU  register  */
#define SCU_BASE				(0xBFB00000)
#define SCU_PDIDR				(SCU_BASE+0x005C)   //ECO Version
#define SCU_HIR					(SCU_BASE+0x0064)   //Hardware Version
#define SCU_FPGA_IR				(SCU_BASE+0x0068)   //FPGA Version
#define SCU_FPGA_TEMP			(SCU_BASE+0x006C)   //FPGA Version

/* EN7580 Flash */
#define FLASH_MATRIX_SIZE							56			// david 20180907

#define SCU_WAN_CONF_REG							(0xBFB00070)
#define SCU_WAN_CONF_REG_WAN_SEL_BITS				(0x000000FF)
#define SCU_WAN_CONF_REG_WAN_SEL_MASK				(0xFFFFFF00)
#define SCU_WAN_CONF_REG_WAN_SEL_GPON				0
#define SCU_WAN_CONF_REG_WAN_SEL_EPON				1
#define SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_1G		6
#define SCU_WAN_CONF_REG_WAN_SEL_XEPON_10G_10G		7
#define SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G		8
#define SCU_WAN_CONF_REG_WAN_SEL_XGPON				9
#define SCU_WAN_CONF_REG_WAN_SEL_XGSPON				10
#define SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_10G		11
#define SCU_WAN_CONF_REG_WAN_SEL_NGPON2_10G_2G		12
#define SCU_WAN_CONF_REG_WAN_SEL_NGPON2_2G_2G		13
#define SCU_WAN_CONF_REG_WAN_SEL_GPON_SYM			20 //0x14
#define SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON			21


#endif /* _PHY_REG_H_ */

