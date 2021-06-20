/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2006-2012 Tony Wu (tonywu@realtek.com)
 * Copyright (C) 2020 B. Koblitz
 */
#ifndef _MACH_RTL838X_H_
#define _MACH_RTL838X_H_

#include <asm/types.h>
/*
 * Register access macros
 */

#define RTL838X_SW_BASE		((volatile void *) 0xBB000000)

#define rtl83xx_r32(reg)	readl(reg)
#define rtl83xx_w32(val, reg)	writel(val, reg)
#define rtl83xx_w32_mask(clear, set, reg) rtl83xx_w32((rtl83xx_r32(reg) & ~(clear)) | (set), reg)

#define rtl83xx_r8(reg)		readb(reg)
#define rtl83xx_w8(val, reg)	writeb(val, reg)

#define sw_r32(reg)		readl(RTL838X_SW_BASE + reg)
#define sw_w32(val, reg)	writel(val, RTL838X_SW_BASE + reg)
#define sw_w32_mask(clear, set, reg)	\
				sw_w32((sw_r32(reg) & ~(clear)) | (set), reg)
#define sw_r64(reg)		((((u64)readl(RTL838X_SW_BASE + reg)) << 32) | \
				readl(RTL838X_SW_BASE + reg + 4))

#define sw_w64(val, reg)	do { \
					writel((u32)((val) >> 32), RTL838X_SW_BASE + reg); \
					writel((u32)((val) & 0xffffffff), \
							RTL838X_SW_BASE + reg + 4); \
				} while (0)

/*
 * SPRAM
 */
#define RTL838X_ISPRAM_BASE	0x0
#define RTL838X_DSPRAM_BASE	0x0

/*
 * IRQ Controller
 */
#define RTL838X_IRQ_CPU_BASE	0
#define RTL838X_IRQ_CPU_NUM	8
#define RTL838X_IRQ_ICTL_BASE	(RTL838X_IRQ_CPU_BASE + RTL838X_IRQ_CPU_NUM)
#define RTL838X_IRQ_ICTL_NUM	32

#define RTL83XX_IRQ_UART0		31
#define RTL83XX_IRQ_UART1		30
#define RTL83XX_IRQ_TC0			29
#define RTL83XX_IRQ_TC1			28
#define RTL83XX_IRQ_OCPTO		27
#define RTL83XX_IRQ_HLXTO		26
#define RTL83XX_IRQ_SLXTO		25
#define RTL83XX_IRQ_NIC			24
#define RTL83XX_IRQ_GPIO_ABCD		23
#define RTL83XX_IRQ_GPIO_EFGH		22
#define RTL83XX_IRQ_RTC			21
#define RTL83XX_IRQ_SWCORE		20
#define RTL83XX_IRQ_WDT_IP1		19
#define RTL83XX_IRQ_WDT_IP2		18

#define RTL9300_UART1_IRQ		31
#define RTL9300_UART0_IRQ		30
#define RTL9300_USB_H2_IRQ		28
#define RTL9300_NIC_IRQ			24
#define RTL9300_SWCORE_IRQ		23
#define RTL9300_GPIO_ABC_IRQ		13
#define RTL9300_TC4_IRQ			11
#define RTL9300_TC3_IRQ			10
#define RTL9300_TC2_IRQ			 9
#define RTL9300_TC1_IRQ			 8
#define RTL9300_TC0_IRQ			 7


/*
 * MIPS32R2 counter
 */
#define RTL838X_COMPARE_IRQ	(RTL838X_IRQ_CPU_BASE + 7)

/*
 *  ICTL
 *  Base address 0xb8003000UL
 */
#define RTL838X_ICTL1_IRQ	(RTL838X_IRQ_CPU_BASE + 2)
#define RTL838X_ICTL2_IRQ	(RTL838X_IRQ_CPU_BASE + 3)
#define RTL838X_ICTL3_IRQ	(RTL838X_IRQ_CPU_BASE + 4)
#define RTL838X_ICTL4_IRQ	(RTL838X_IRQ_CPU_BASE + 5)
#define RTL838X_ICTL5_IRQ	(RTL838X_IRQ_CPU_BASE + 6)

#define GIMR			(0x00)
#define UART0_IE		(1 << 31)
#define UART1_IE		(1 << 30)
#define TC0_IE			(1 << 29)
#define TC1_IE			(1 << 28)
#define OCPTO_IE		(1 << 27)
#define HLXTO_IE		(1 << 26)
#define SLXTO_IE		(1 << 25)
#define NIC_IE			(1 << 24)
#define GPIO_ABCD_IE		(1 << 23)
#define GPIO_EFGH_IE		(1 << 22)
#define RTC_IE			(1 << 21)
#define WDT_IP1_IE		(1 << 19)
#define WDT_IP2_IE		(1 << 18)

#define GISR			(0x04)
#define UART0_IP		(1 << 31)
#define UART1_IP		(1 << 30)
#define TC0_IP			(1 << 29)
#define TC1_IP			(1 << 28)
#define OCPTO_IP		(1 << 27)
#define HLXTO_IP		(1 << 26)
#define SLXTO_IP		(1 << 25)
#define NIC_IP			(1 << 24)
#define GPIO_ABCD_IP		(1 << 23)
#define GPIO_EFGH_IP		(1 << 22)
#define RTC_IP			(1 << 21)
#define WDT_IP1_IP		(1 << 19)
#define WDT_IP2_IP		(1 << 18)


/* Interrupt Routing Selection */
#define UART0_RS		2
#define UART1_RS		1
#define TC0_RS			5
#define TC1_RS			1
#define OCPTO_RS		1
#define HLXTO_RS		1
#define SLXTO_RS		1
#define NIC_RS			4
#define GPIO_ABCD_RS		4
#define GPIO_EFGH_RS		4
#define RTC_RS			4
#define	SWCORE_RS		3
#define WDT_IP1_RS		4
#define WDT_IP2_RS		5

/* Interrupt IRQ Assignments */
#define UART0_IRQ		31
#define UART1_IRQ		30
#define TC0_IRQ			29
#define TC1_IRQ			28
#define OCPTO_IRQ		27
#define HLXTO_IRQ		26
#define SLXTO_IRQ		25
#define NIC_IRQ			24
#define GPIO_ABCD_IRQ		23
#define GPIO_EFGH_IRQ		22
#define RTC_IRQ			21
#define	SWCORE_IRQ		20
#define WDT_IP1_IRQ		19
#define WDT_IP2_IRQ		18

#define SYSTEM_FREQ		200000000
#define RTL838X_UART0_BASE	((volatile void *)(0xb8002000UL))
#define RTL838X_UART0_BAUD	38400  /* ex. 19200 or 38400 or 57600 or 115200 */
#define RTL838X_UART0_FREQ	(SYSTEM_FREQ - RTL838X_UART0_BAUD * 24)
#define RTL838X_UART0_MAPBASE	0x18002000UL
#define RTL838X_UART0_MAPSIZE	0x100
#define RTL838X_UART0_IRQ	UART0_IRQ

#define RTL838X_UART1_BASE	((volatile void *)(0xb8002100UL))
#define RTL838X_UART1_BAUD	38400  /* ex. 19200 or 38400 or 57600 or 115200 */
#define RTL838X_UART1_FREQ	(SYSTEM_FREQ - RTL838X_UART1_BAUD * 24)
#define RTL838X_UART1_MAPBASE	0x18002100UL
#define RTL838X_UART1_MAPSIZE	0x100
#define RTL838X_UART1_IRQ	UART1_IRQ

#define UART0_RBR		(RTL838X_UART0_BASE + 0x000)
#define UART0_THR		(RTL838X_UART0_BASE + 0x000)
#define UART0_DLL		(RTL838X_UART0_BASE + 0x000)
#define UART0_IER		(RTL838X_UART0_BASE + 0x004)
#define UART0_DLM		(RTL838X_UART0_BASE + 0x004)
#define UART0_IIR		(RTL838X_UART0_BASE + 0x008)
#define UART0_FCR		(RTL838X_UART0_BASE + 0x008)
#define UART0_LCR		(RTL838X_UART0_BASE + 0x00C)
#define UART0_MCR		(RTL838X_UART0_BASE + 0x010)
#define UART0_LSR		(RTL838X_UART0_BASE + 0x014)

#define UART1_RBR		(RTL838X_UART1_BASE + 0x000)
#define UART1_THR		(RTL838X_UART1_BASE + 0x000)
#define UART1_DLL		(RTL838X_UART1_BASE + 0x000)
#define UART1_IER		(RTL838X_UART1_BASE + 0x004)
#define UART1_DLM		(RTL838X_UART1_BASE + 0x004)
#define UART1_IIR		(RTL838X_UART1_BASE + 0x008)
#define UART1_FCR		(RTL838X_UART1_BASE + 0x008)
#define UART1_LCR		(RTL838X_UART1_BASE + 0x00C)
#define UART1_MCR		(RTL838X_UART1_BASE + 0x010)
#define UART1_LSR		(RTL838X_UART1_BASE + 0x014)

/*
 * Memory Controller
 */
#define MC_MCR			0xB8001000
#define MC_MCR_VAL		0x00000000

#define MC_DCR			0xB8001004
#define MC_DCR0_VAL		0x54480000

#define MC_DTCR			0xB8001008
#define MC_DTCR_VAL		0xFFFF05C0

/*
 * GPIO
 */
#define GPIO_CTRL_REG_BASE		((volatile void *) 0xb8003500)
#define RTL838X_GPIO_PABC_CNR		(GPIO_CTRL_REG_BASE + 0x0)
#define RTL838X_GPIO_PABC_TYPE		(GPIO_CTRL_REG_BASE + 0x04)
#define RTL838X_GPIO_PABC_DIR		(GPIO_CTRL_REG_BASE + 0x8)
#define RTL838X_GPIO_PABC_DATA		(GPIO_CTRL_REG_BASE + 0xc)
#define RTL838X_GPIO_PABC_ISR		(GPIO_CTRL_REG_BASE + 0x10)
#define RTL838X_GPIO_PAB_IMR		(GPIO_CTRL_REG_BASE + 0x14)
#define RTL838X_GPIO_PC_IMR		(GPIO_CTRL_REG_BASE + 0x18)

#define RTL838X_MODEL_NAME_INFO		(0x00D4)
#define RTL839X_MODEL_NAME_INFO		(0x0FF0)
#define RTL93XX_MODEL_NAME_INFO		(0x0004)

#define RTL838X_LED_GLB_CTRL		(0xA000)
#define RTL839X_LED_GLB_CTRL		(0x00E4)
#define RTL9302_LED_GLB_CTRL		(0xcc00)
#define RTL930X_LED_GLB_CTRL		(0xC400)
#define RTL931X_LED_GLB_CTRL		(0x0600)

#define RTL838X_EXT_GPIO_DIR		(0xA08C)
#define RTL839X_EXT_GPIO_DIR		(0x0214)
#define RTL838X_EXT_GPIO_DATA		(0xA094)
#define RTL839X_EXT_GPIO_DATA		(0x021c)
#define RTL838X_EXT_GPIO_INDRT_ACCESS	(0xA09C)
#define RTL839X_EXT_GPIO_INDRT_ACCESS	(0x0224)
#define RTL838X_EXTRA_GPIO_CTRL		(0xA0E0)
#define RTL838X_DMY_REG5		(0x0144)
#define RTL838X_EXTRA_GPIO_CTRL		(0xA0E0)

#define RTL838X_GMII_INTF_SEL		(0x1000)
#define RTL838X_IO_DRIVING_ABILITY_CTRL	(0x1010)

#define RTL838X_GPIO_A7		31
#define RTL838X_GPIO_A6		30
#define RTL838X_GPIO_A5		29
#define RTL838X_GPIO_A4		28
#define RTL838X_GPIO_A3		27
#define RTL838X_GPIO_A2		26
#define RTL838X_GPIO_A1		25
#define RTL838X_GPIO_A0		24
#define RTL838X_GPIO_B7		23
#define RTL838X_GPIO_B6		22
#define RTL838X_GPIO_B5		21
#define RTL838X_GPIO_B4		20
#define RTL838X_GPIO_B3		19
#define RTL838X_GPIO_B2		18
#define RTL838X_GPIO_B1		17
#define RTL838X_GPIO_B0		16
#define RTL838X_GPIO_C7		15
#define RTL838X_GPIO_C6		14
#define RTL838X_GPIO_C5		13
#define RTL838X_GPIO_C4		12
#define RTL838X_GPIO_C3		11
#define RTL838X_GPIO_C2		10
#define RTL838X_GPIO_C1		9
#define RTL838X_GPIO_C0		8

#define RTL838X_INT_RW_CTRL		(0x0058)
#define RTL838X_EXT_VERSION		(0x00D0)
#define RTL838X_PLL_CML_CTRL		(0x0FF8)
#define RTL838X_STRAP_DBG		(0x100C)

/*
 * Reset
 */
#define	RGCR				(0x1E70)
#define RTL838X_RST_GLB_CTRL_0		(0x003c)
#define RTL838X_RST_GLB_CTRL_1		(0x0040)
#define RTL839X_RST_GLB_CTRL		(0x0014)
#define RTL930X_RST_GLB_CTRL_0		(0x000c)
#define RTL931X_RST_GLB_CTRL		(0x0400)

/* LED control by switch */
#define RTL838X_LED_MODE_SEL		(0x1004)
#define RTL838X_LED_MODE_CTRL		(0xA004)
#define RTL838X_LED_P_EN_CTRL		(0xA008)

/* LED control by software */
#define RTL838X_LED_SW_CTRL		(0x0128)
#define RTL839X_LED_SW_CTRL		(0xA00C)
#define RTL838X_LED_SW_P_EN_CTRL	(0xA010)
#define RTL839X_LED_SW_P_EN_CTRL	(0x012C)
#define RTL838X_LED0_SW_P_EN_CTRL	(0xA010)
#define RTL839X_LED0_SW_P_EN_CTRL	(0x012C)
#define RTL838X_LED1_SW_P_EN_CTRL	(0xA014)
#define RTL839X_LED1_SW_P_EN_CTRL	(0x0130)
#define RTL838X_LED2_SW_P_EN_CTRL	(0xA018)
#define RTL839X_LED2_SW_P_EN_CTRL	(0x0134)
#define RTL838X_LED_SW_P_CTRL		(0xA01C)
#define RTL839X_LED_SW_P_CTRL		(0x0144)

#define RTL839X_MAC_EFUSE_CTRL		(0x02ac)

/*
 * MDIO via Realtek's SMI interface
 */
#define RTL838X_SMI_GLB_CTRL		(0xa100)
#define RTL838X_SMI_ACCESS_PHY_CTRL_0	(0xa1b8)
#define RTL838X_SMI_ACCESS_PHY_CTRL_1	(0xa1bc)
#define RTL838X_SMI_ACCESS_PHY_CTRL_2	(0xa1c0)
#define RTL838X_SMI_ACCESS_PHY_CTRL_3	(0xa1c4)
#define RTL838X_SMI_PORT0_5_ADDR_CTRL	(0xa1c8)
#define RTL838X_SMI_POLL_CTRL		(0xa17c)

#define RTL839X_SMI_GLB_CTRL		(0x03f8)
#define RTL839X_SMI_PORT_POLLING_CTRL	(0x03fc)
#define RTL839X_PHYREG_ACCESS_CTRL	(0x03DC)
#define RTL839X_PHYREG_CTRL		(0x03E0)
#define RTL839X_PHYREG_PORT_CTRL	(0x03E4)
#define RTL839X_PHYREG_DATA_CTRL	(0x03F0)
#define RTL839X_PHYREG_MMD_CTRL		(0x3F4)

#define RTL930X_SMI_GLB_CTRL		(0xCA00)
#define RTL930X_SMI_POLL_CTRL		(0xca90)
#define RTL930X_SMI_PORT0_15_POLLING_SEL (0xCA08)
#define RTL930X_SMI_PORT16_27_POLLING_SEL (0xCA0C)
#define RTL930X_SMI_PORT0_5_ADDR	(0xCB80)
#define RTL930X_SMI_ACCESS_PHY_CTRL_0	(0xCB70)
#define RTL930X_SMI_ACCESS_PHY_CTRL_1	(0xCB74)
#define RTL930X_SMI_ACCESS_PHY_CTRL_2	(0xCB78)
#define RTL930X_SMI_ACCESS_PHY_CTRL_3	(0xCB7C)

#define RTL931X_SMI_GLB_CTRL1		(0x0CBC)
#define RTL931X_SMI_GLB_CTRL0		(0x0CC0)
#define RTL931X_SMI_PORT_POLLING_CTRL	(0x0CCC)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_0	(0x0C00)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_1	(0x0C04)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_2	(0x0C08)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_3	(0x0C10)
#define RTL931X_SMI_INDRT_ACCESS_BC_PHYID_CTRL (0x0C14)
#define RTL931X_SMI_INDRT_ACCESS_MMD_CTRL (0xC18)

#define RTL930X_SMI_GLB_CTRL		(0xCA00)
#define RTL930X_SMI_POLL_CTRL		(0xca90)
#define RTL930X_SMI_PORT0_15_POLLING_SEL (0xCA08)
#define RTL930X_SMI_PORT16_27_POLLING_SEL (0xCA0C)
#define RTL930X_SMI_PORT0_5_ADDR	(0xCB80)
#define RTL930X_SMI_ACCESS_PHY_CTRL_0	(0xCB70)
#define RTL930X_SMI_ACCESS_PHY_CTRL_1	(0xCB74)
#define RTL930X_SMI_ACCESS_PHY_CTRL_2	(0xCB78)
#define RTL930X_SMI_ACCESS_PHY_CTRL_3	(0xCB7C)

#define RTL931X_SMI_GLB_CTRL1		(0x0CBC)
#define RTL931X_SMI_GLB_CTRL0		(0x0CC0)
#define RTL931X_SMI_PORT_POLLING_CTRL	(0x0CCC)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_0	(0x0C00)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_1	(0x0C04)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_2	(0x0C08)
#define RTL931X_SMI_INDRT_ACCESS_CTRL_3	(0x0C10)

/*
 * Switch interrupts
 */
#define RTL838X_IMR_GLB			(0x1100)
#define RTL838X_IMR_PORT_LINK_STS_CHG	(0x1104)
#define RTL838X_ISR_GLB_SRC		(0x1148)
#define RTL838X_ISR_PORT_LINK_STS_CHG	(0x114C)

#define RTL839X_IMR_GLB			(0x0064)
#define RTL839X_IMR_PORT_LINK_STS_CHG	(0x0068)
#define RTL839X_ISR_GLB_SRC		(0x009c)
#define RTL839X_ISR_PORT_LINK_STS_CHG	(0x00a0)

#define RTL930X_IMR_GLB			(0xC628)
#define RTL930X_IMR_PORT_LINK_STS_CHG	(0xC62C)
#define RTL930X_ISR_GLB			(0xC658)
#define RTL930X_ISR_PORT_LINK_STS_CHG	(0xC660)

// IMR_GLB does not exit on RTL931X
#define RTL931X_IMR_PORT_LINK_STS_CHG	(0x126C)
#define RTL931X_ISR_GLB_SRC		(0x12B4)
#define RTL931X_ISR_PORT_LINK_STS_CHG	(0x12B8)

/* Definition of family IDs */
#define RTL8389_FAMILY_ID   (0x8389)
#define RTL8328_FAMILY_ID   (0x8328)
#define RTL8390_FAMILY_ID   (0x8390)
#define RTL8350_FAMILY_ID   (0x8350)
#define RTL8380_FAMILY_ID   (0x8380)
#define RTL8330_FAMILY_ID   (0x8330)
#define RTL9300_FAMILY_ID   (0x9300)
#define RTL9310_FAMILY_ID   (0x9310)

/* Basic SoC Features */
#define RTL838X_CPU_PORT			28
#define RTL839X_CPU_PORT			52
#define RTL930X_CPU_PORT			28
#define RTL931X_CPU_PORT			56

struct rtl83xx_soc_info {
	unsigned char *name;
	unsigned int id;
	unsigned int family;
	unsigned char *compatible;
	volatile void *sw_base;
	volatile void *icu_base;
	int cpu_port;
};

/* rtl83xx-related functions used across subsystems */
int rtl838x_smi_wait_op(int timeout);
int rtl838x_read_phy(u32 port, u32 page, u32 reg, u32 *val);
int rtl838x_write_phy(u32 port, u32 page, u32 reg, u32 val);
int rtl839x_read_phy(u32 port, u32 page, u32 reg, u32 *val);
int rtl839x_write_phy(u32 port, u32 page, u32 reg, u32 val);
int rtl930x_read_phy(u32 port, u32 page, u32 reg, u32 *val);
int rtl930x_write_phy(u32 port, u32 page, u32 reg, u32 val);
int rtl931x_read_phy(u32 port, u32 page, u32 reg, u32 *val);
int rtl931x_write_phy(u32 port, u32 page, u32 reg, u32 val);

#endif   /* _MACH_RTL838X_H_ */
