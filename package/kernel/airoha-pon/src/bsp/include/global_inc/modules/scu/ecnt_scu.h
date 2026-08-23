 /***************************************************************************************
 *      Copyright(c) 2014 ECONET Incorporation All rights reserved.
 *
 *      This is unpublished proprietary source code of ECONET Incorporation
 *
 *      The copyright notice above does not evidence any actual or intended
 *      publication of such source code.
 ***************************************************************************************
 */

/*======================================================================================
 * MODULE NAME: scu
 * FILE NAME: ecnt_scu.h
 * DATE: 2019/08/15
 * VERSION: 1.00
 * PURPOSE: 
 * NOTES:
 *
 * AUTHOR :          REVIEWED by
 *
 * FUNCTIONS
 *
 *
 * DEPENDENCIES
 *
 * * $History: $
 * MODIFICTION HISTORY:
 * Version 1.00
 * ** This is the first versoin for creating to support the functions of
 *    current module.
 *
 *======================================================================================
 */

#ifndef __ECNT_SCU_H__
#define __ECNT_SCU_H__

#include <linux/types.h>
/* INCLUDE FILE DECLARATIONS --------------------------------------------------------- */

/* NAMING CONSTANT DECLARATIONS ------------------------------------------------------ */

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
/* CHIP SCU */
#ifdef TCSUPPORT_CPU_ARMV8
#define CR_NP_SCU_BASE                  (0x00000000)
#define CR_CHIP_SCU_BASE				(0x00000000)
#else
#define CR_NP_SCU_BASE				    (0xBFB00000)
#define CR_CHIP_SCU_BASE				(0xBFA20000)
#endif

#define BOOT_TRAP_CONF_DEC_EMMC_OFFSET	(6)
#define BOOT_TRAP_CONF_DEC_EMMC_MASK	(0x1 << BOOT_TRAP_CONF_DEC_EMMC_OFFSET)

typedef union {
	struct {
		unsigned int	packageID			: 4 ;
		unsigned int	isDDR4				: 1 ;
		unsigned int	isSecureHwTrapEn	: 1 ; /* secure HW trap */
		unsigned int	isSecureModeEn		: 1 ; /* RSA key has been written or not */
		unsigned int	packageID_ext		: 1 ;
		unsigned int	isCtrlEcc			: 1 ;
		unsigned int	isFpga				: 1 ;
		unsigned int	sys_clk				: 10; /* bus clock can support up to 512MHz */
		unsigned int	dram_size			: 10; /* Unit = 16MB, DRAM size can support up to 2048MB */
		unsigned int	resv2				: 2 ;
	} raw ;
	unsigned int word;
} SYS_GLOBAL_PARM_T ;

/* CHIP SCU registers */
#if defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define CR_CHIP_SCU_SPI_CLK			            (CR_CHIP_SCU_BASE + 0x1C4)
#define CR_CHIP_SCU_NP_BUS_DOM_CLK_GAT			(CR_CHIP_SCU_BASE + 0x1E4)

#if defined(TCSUPPORT_CPU_EN7581) || defined(TCSUPPORT_CPU_AN7552)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1		(CR_CHIP_SCU_BASE + 0x1EC)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2		(CR_CHIP_SCU_BASE + 0x200)
#define CR_CHIP_SCU_IOMUX_CTRL_1				(CR_CHIP_SCU_BASE + 0x214)
#define CR_CHIP_SCU_IOMUX_CTRL_2				(CR_CHIP_SCU_BASE + 0x218)
#define CR_CHIP_SCU_IOMUX_CTRL_3				(CR_CHIP_SCU_BASE + 0x21C)
#define CR_CHIP_SCU_PON_I2C_MODE				(CR_CHIP_SCU_BASE + 0x214)
#define RG_FORCE_GPIO2_EN						(CR_CHIP_SCU_BASE + 0x228)
#define CR_CHIP_SCU_FORCE_GPIO32_EN				(CR_CHIP_SCU_BASE + 0x22C)
#else /* TCSUPPORT_CPU_EN7523 */
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1		(CR_CHIP_SCU_BASE + 0x1E8)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_2		(CR_CHIP_SCU_BASE + 0x1EC)
#define CR_CHIP_SCU_IOMUX_CTRL_1				(CR_CHIP_SCU_BASE + 0x210)
#define CR_CHIP_SCU_IOMUX_CTRL_2				(CR_CHIP_SCU_BASE + 0x214)
#define CR_CHIP_SCU_IOMUX_CTRL_3				(CR_CHIP_SCU_BASE + 0x218)
#define CR_CHIP_SCU_PON_I2C_MODE				(CR_CHIP_SCU_BASE + 0x210)
#define RG_FORCE_GPIO2_EN	       			 	(CR_CHIP_SCU_BASE + 0x224)
#define CR_CHIP_SCU_FORCE_GPIO32_EN				(CR_CHIP_SCU_BASE + 0x228)
#endif

#define RG_GPIO_2ND_I2C_MODE                    (0x00010000)
#define RG_SW_TOD_1PPS_MODE                     (0x00010001)
#define RG_PON_TOD_1PPS_MODE                    (0x00010002)
#define RG_GPIO_LAN0_LED0_MODE                  (0x00010003)
#define RG_GPIO_LAN0_LED1_MODE                  (0x00010004)
#define RG_GPIO_LAN1_LED0_MODE                  (0x00010005)
#define RG_GPIO_LAN1_LED1_MODE                  (0x00010006)
#define RG_GPIO_LAN2_LED0_MODE                  (0x00010007)
#define RG_GPIO_LAN2_LED1_MODE                  (0x00010008)
#define RG_GPIO_LAN3_LED0_MODE                  (0x00010009)
#define RG_GPIO_LAN3_LED1_MODE                  (0x0001000a)
#define RG_GPIO_I2C_SLAVE_MODE                  (0x0001000b)
#define RG_GPIO_I2S_MODE                        (0x0001000c)

#define RG_GPIO_SPI_CS1_MODE                    (0x00020000)
#define RG_GPIO_SPI_CS2_MODE                    (0x00020001)
#define RG_GPIO_SPI_CS3_MODE                    (0x00020002)
#define RG_GPIO_SPI_CS4_MODE                    (0x00020003)
#define RG_GPIO_SPI_QUAD_MODE                   (0x00020004)
#define RG_GPIO_PCM_RESET_MODE                  (0x00020008)
#define RG_GPIO_PCM_INT_MODE                    (0x00020009)
#define RG_GPIO_PCM1_MODE                       (0x0002000c)
#define RG_GPIO_PCM2_MODE                       (0x0002000d)
#define RG_GPIO_PCM_SPI_MODE                    (0x00020010)
#define RG_GPIO_PCM_SPI_CS1_MODE                (0x00020011)
#define RG_GPIO_PCM_SPI_CS2_MODE_128            (0x00020012)
#define RG_GPIO_PCM_SPI_CS2_MODE_156            (0x00020013)
#define RG_GPIO_PCM_SPI_CS3_MODE                (0x00020014)
#define RG_GPIO_PCM_SPI_CS4_MODE                (0x00020015)

#define RG_GPIO_PON_MODE                        (0x00030000)
#define RG_SIPO_MODE                            (0x00030002)
#define RG_GPIO_UART2_MODE                      (0x00030003)
#define RG_GPIO_UART2_CTS_RTS_MODE              (0x00030004)
#define RG_GPIO_UART3_MODE                      (0x00030005)
#define RG_GPIO_UART3_CTS_RTS_MODE              (0x00030006)
#define RG_GPIO_UART4_MODE                      (0x00030007)
#define RG_GPIO_UART5_MODE                      (0x00030008)
#define RG_PCIE_RESET0_GPIO_MODE                (0x00030009)
#define RG_PCIE_RESET1_GPIO_MODE                (0x0003000a)
#define RG_SIPO_RCLK_MODE                       (0x0003000b)
#define RG_GPIO_SGMII_MDIO_MODE                 (0x0003000d)

#define RG_GHIP_PROBE0_MODE                     (0x00040000)
#define RG_CHIP_PROBE1_MODE                     (0x00040001)

#define RG_NPU_UART_EN                          (0x00050002)
#define RG_DFD_EJTAG_EN                         (0x00050003)
#define RG_UDI_EJTAG_EN                         (0x00050004)

#define RG_FORCE_GPIO6_EN                       (0x00060006)
#define RG_FORCE_GPIO7_EN                       (0x00060007)
#define RG_FORCE_GPIO8_EN                       (0x00060008)
#define RG_FORCE_GPIO9_EN                       (0x00060009)
#define RG_FORCE_GPIO10_EN                      (0x0006000a)
#define RG_FORCE_GPIO11_EN                      (0x0006000b)
#define RG_FORCE_GPIO12_EN                      (0x0006000c)
#define RG_FORCE_GPIO13_EN                      (0x0006000d)
#define RG_FORCE_GPIO14_EN                      (0x0006000e)
#define RG_FORCE_GPIO15_EN                      (0x0006000f)
#define RG_FORCE_GPIO16_EN                      (0x00060010)
#define RG_FORCE_GPIO17_EN                      (0x00060011)
#define RG_FORCE_GPIO18_EN                      (0x00060012)
#define RG_FORCE_GPIO19_EN                      (0x00060013)
#define RG_FORCE_GPIO20_EN                      (0x00060014)
#define RG_FORCE_GPIO21_EN                      (0x00060015)
#define RG_FORCE_GPIO22_EN                      (0x00060016)
#define RG_FORCE_GPIO23_EN                      (0x00060017)
#define RG_FORCE_GPIO24_EN                      (0x00060018)
#define RG_FORCE_GPIO25_EN                      (0x00060019)
#define RG_FORCE_GPIO26_EN                      (0x0006001a)
#define RG_FORCE_GPIO27_EN                      (0x0006001b)
#define RG_FORCE_GPIO28_EN                      (0x0006001c)
#define RG_FORCE_GPIO29_EN                      (0x0006001d)
#define RG_FORCE_GPIO30_EN                      (0x0006001e)
#define RG_FORCE_GPIO31_EN                      (0x0006001f)

#define RG_FORCE_GPIO32_EN                      (0x00070000)
#define RG_FORCE_GPIO33_EN                      (0x00070001)
#define RG_FORCE_GPIO34_EN                      (0x00070002)
#define RG_FORCE_GPIO35_EN                      (0x00070003)
#define RG_FORCE_GPIO36_EN                      (0x00070004)
#define RG_FORCE_GPIO37_EN                      (0x00070005)
#define RG_FORCE_GPIO38_EN                      (0x00070006)
#define RG_FORCE_GPIO39_EN                      (0x00070007)
#define RG_FORCE_GPIO40_EN                      (0x00070008)
#define RG_FORCE_GPIO41_EN                      (0x00070009)
#define RG_FORCE_GPIO42_EN                      (0x0007000a)
#define RG_FORCE_GPIO43_EN                      (0x0007000b)

#elif defined(TCSUPPORT_CPU_EN7580)
#define CR_CHIP_SCU_SPI_CLK			            (CR_CHIP_SCU_BASE + 0x1B8)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1		(CR_CHIP_SCU_BASE + 0x1DC)
#define CR_CHIP_SCU_TOD_DIVIDER_ENABLE		    (CR_CHIP_SCU_BASE + 0x1E4)
#define CR_CHIP_SCU_IOMUX_CTRL_1	(CR_CHIP_SCU_BASE + 0x210)
#define CR_CHIP_SCU_IOMUX_CTRL_2	(CR_CHIP_SCU_BASE + 0x214)
#define CR_CHIP_SCU_IOMUX_CTRL_3	(CR_CHIP_SCU_BASE + 0x218)
#define CR_CHIP_SCU_PON_I2C_MODE	(CR_CHIP_SCU_BASE + 0x210)
#define RG_FORCE_GPIO2_EN	        (CR_CHIP_SCU_BASE + 0x224)
#elif defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527)
#define CR_CHIP_SCU_SPI_CLK			            (CR_CHIP_SCU_BASE + 0x11c)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1		(CR_CHIP_SCU_BASE + 0x13c)
#define CR_CHIP_SCU_TOD_DIVIDER_ENABLE		    (CR_CHIP_SCU_BASE + 0x144)
#define CR_CHIP_SCU_IOMUX_CTRL_1	            (CR_CHIP_SCU_BASE + 0x15C)
#define CR_CHIP_SCU_IOMUX_CTRL_2	            (CR_CHIP_SCU_BASE + 0x160)
#define CR_CHIP_SCU_PON_I2C_MODE		        CR_CHIP_SCU_IOMUX_CTRL_1
#define RG_FORCE_GPIO2_EN	                    CR_CHIP_SCU_IOMUX_CTRL_2
#else
#define CR_CHIP_SCU_SPI_CLK			            (CR_CHIP_SCU_BASE + 0x0CC)
#define CR_CHIP_SCU_NP_PER_DOM_CLK_GAT_1		(CR_CHIP_SCU_BASE + 0x0E4)
#define CR_CHIP_SCU_TOD_DIVIDER_ENABLE		    (CR_CHIP_SCU_BASE + 0x0EC)
#define CR_CHIP_SCU_IOMUX_CTRL_1	(CR_CHIP_SCU_BASE + 0x104)
#define CR_CHIP_SCU_IOMUX_CTRL_2	(CR_CHIP_SCU_BASE + 0x108)
#define CR_CHIP_SCU_IOMUX_CTRL_3	(CR_CHIP_SCU_BASE + 0x10C)
#define CR_CHIP_SCU_PON_I2C_MODE	CR_CHIP_SCU_IOMUX_CTRL_1
#define RG_FORCE_GPIO2_EN	        CR_CHIP_SCU_IOMUX_CTRL_2
#endif


extern uint32_t GET_SMB0ALS(void);
extern void SET_SMB0ALS(uint32_t data);

extern uint32_t GET_SCU_RSTCTRL1(void);
extern void SET_SCU_RSTCTRL1(uint32_t val);

extern uint32_t GET_SCU_RST_RG(void);
extern void SET_SCU_RST_RG(uint32_t val);
extern uint32_t GET_SSR3(void);


#endif /* ifndef __ECNT_SCU_H__ */

