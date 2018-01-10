/*
 *  Atheros AR71xx SoC specific definitions
 *
 *  Copyright (C) 2008-2009 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Parts of this file are based on Atheros' 2.6.15 BSP
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#ifndef __ASM_MACH_AR71XX_H
#define __ASM_MACH_AR71XX_H

#include <linux/types.h>
#include <asm/io.h>
#include <linux/bitops.h>

#ifndef __ASSEMBLER__

#define BIT(x)	(1<<(x))

#define AR71XX_PCI_MEM_BASE	0x10000000
#define AR71XX_PCI_MEM_SIZE	0x08000000
#define AR71XX_APB_BASE		0x18000000
#define AR71XX_GE0_BASE		0x19000000
#define AR71XX_GE0_SIZE		0x01000000
#define AR71XX_GE1_BASE		0x1a000000
#define AR71XX_GE1_SIZE		0x01000000
#define AR71XX_EHCI_BASE	0x1b000000
#define AR71XX_EHCI_SIZE	0x01000000
#define AR71XX_OHCI_BASE	0x1c000000
#define AR71XX_OHCI_SIZE	0x01000000
#define AR7240_OHCI_BASE	0x1b000000
#define AR7240_OHCI_SIZE	0x01000000
#define AR71XX_SPI_BASE		0x1f000000
#define AR71XX_SPI_SIZE		0x01000000

#define AR71XX_DDR_CTRL_BASE	(AR71XX_APB_BASE + 0x00000000)
#define AR71XX_DDR_CTRL_SIZE	0x10000
#define AR71XX_CPU_BASE		(AR71XX_APB_BASE + 0x00010000)
#define AR71XX_UART_BASE	(AR71XX_APB_BASE + 0x00020000)
#define AR71XX_UART_SIZE	0x10000
#define AR71XX_USB_CTRL_BASE	(AR71XX_APB_BASE + 0x00030000)
#define AR71XX_USB_CTRL_SIZE	0x10000
#define AR71XX_GPIO_BASE	(AR71XX_APB_BASE + 0x00040000)
#define AR71XX_GPIO_SIZE	0x10000
#define AR71XX_PLL_BASE		(AR71XX_APB_BASE + 0x00050000)
#define AR71XX_PLL_SIZE		0x10000
#define AR71XX_RESET_BASE	(AR71XX_APB_BASE + 0x00060000)
#define AR71XX_RESET_SIZE	0x10000
#define AR71XX_MII_BASE		(AR71XX_APB_BASE + 0x00070000)
#define AR71XX_MII_SIZE		0x10000
#define AR71XX_SLIC_BASE	(AR71XX_APB_BASE + 0x00090000)
#define AR71XX_SLIC_SIZE	0x10000
#define AR71XX_DMA_BASE		(AR71XX_APB_BASE + 0x000A0000)
#define AR71XX_DMA_SIZE		0x10000
#define AR71XX_STEREO_BASE	(AR71XX_APB_BASE + 0x000B0000)
#define AR71XX_STEREO_SIZE	0x10000

#define AR724X_PCI_CRP_BASE	(AR71XX_APB_BASE + 0x000C0000)
#define AR724X_PCI_CRP_SIZE	0x100

#define AR724X_PCI_CTRL_BASE	(AR71XX_APB_BASE + 0x000F0000)
#define AR724X_PCI_CTRL_SIZE	0x100

#define AR91XX_WMAC_BASE	(AR71XX_APB_BASE + 0x000C0000)
#define AR91XX_WMAC_SIZE	0x30000

#define AR71XX_MEM_SIZE_MIN	0x0200000
#define AR71XX_MEM_SIZE_MAX	0x10000000

#define AR71XX_CPU_IRQ_BASE	0
#define AR71XX_MISC_IRQ_BASE	8
#define AR71XX_MISC_IRQ_COUNT	8
#define AR71XX_GPIO_IRQ_BASE	16
#define AR71XX_GPIO_IRQ_COUNT	32
#define AR71XX_PCI_IRQ_BASE     48
#define AR71XX_PCI_IRQ_COUNT	8

#define AR71XX_CPU_IRQ_IP2	(AR71XX_CPU_IRQ_BASE + 2)
#define AR71XX_CPU_IRQ_USB	(AR71XX_CPU_IRQ_BASE + 3)
#define AR71XX_CPU_IRQ_GE0	(AR71XX_CPU_IRQ_BASE + 4)
#define AR71XX_CPU_IRQ_GE1	(AR71XX_CPU_IRQ_BASE + 5)
#define AR71XX_CPU_IRQ_MISC	(AR71XX_CPU_IRQ_BASE + 6)
#define AR71XX_CPU_IRQ_TIMER	(AR71XX_CPU_IRQ_BASE + 7)

#define AR71XX_MISC_IRQ_TIMER	(AR71XX_MISC_IRQ_BASE + 0)
#define AR71XX_MISC_IRQ_ERROR	(AR71XX_MISC_IRQ_BASE + 1)
#define AR71XX_MISC_IRQ_GPIO	(AR71XX_MISC_IRQ_BASE + 2)
#define AR71XX_MISC_IRQ_UART	(AR71XX_MISC_IRQ_BASE + 3)
#define AR71XX_MISC_IRQ_WDOG	(AR71XX_MISC_IRQ_BASE + 4)
#define AR71XX_MISC_IRQ_PERFC	(AR71XX_MISC_IRQ_BASE + 5)
#define AR71XX_MISC_IRQ_OHCI	(AR71XX_MISC_IRQ_BASE + 6)
#define AR71XX_MISC_IRQ_DMA	(AR71XX_MISC_IRQ_BASE + 7)

#define AR71XX_GPIO_IRQ(_x)	(AR71XX_GPIO_IRQ_BASE + (_x))

#define AR71XX_PCI_IRQ_DEV0	(AR71XX_PCI_IRQ_BASE + 0)
#define AR71XX_PCI_IRQ_DEV1	(AR71XX_PCI_IRQ_BASE + 1)
#define AR71XX_PCI_IRQ_DEV2	(AR71XX_PCI_IRQ_BASE + 2)
#define AR71XX_PCI_IRQ_CORE	(AR71XX_PCI_IRQ_BASE + 4)

extern u32 ar71xx_ahb_freq;
extern u32 ar71xx_cpu_freq;
extern u32 ar71xx_ddr_freq;

enum ar71xx_soc_type {
	AR71XX_SOC_UNKNOWN,
	AR71XX_SOC_AR7130,
	AR71XX_SOC_AR7141,
	AR71XX_SOC_AR7161,
	AR71XX_SOC_AR7240,
	AR71XX_SOC_AR7241,
	AR71XX_SOC_AR7242,
	AR71XX_SOC_AR9130,
	AR71XX_SOC_AR9132
};

extern enum ar71xx_soc_type ar71xx_soc;

/*
 * PLL block
 */
#define AR71XX_PLL_REG_CPU_CONFIG	0x00
#define AR71XX_PLL_REG_SEC_CONFIG	0x04
#define AR71XX_PLL_REG_ETH0_INT_CLOCK	0x10
#define AR71XX_PLL_REG_ETH1_INT_CLOCK	0x14

#define AR71XX_PLL_DIV_SHIFT		3
#define AR71XX_PLL_DIV_MASK		0x1f
#define AR71XX_CPU_DIV_SHIFT		16
#define AR71XX_CPU_DIV_MASK		0x3
#define AR71XX_DDR_DIV_SHIFT		18
#define AR71XX_DDR_DIV_MASK		0x3
#define AR71XX_AHB_DIV_SHIFT		20
#define AR71XX_AHB_DIV_MASK		0x7

#define AR71XX_ETH0_PLL_SHIFT		17
#define AR71XX_ETH1_PLL_SHIFT		19

#define AR724X_PLL_REG_CPU_CONFIG	0x00
#define AR724X_PLL_REG_PCIE_CONFIG	0x18

#define AR724X_PLL_DIV_SHIFT		0
#define AR724X_PLL_DIV_MASK		0x3ff
#define AR724X_PLL_REF_DIV_SHIFT	10
#define AR724X_PLL_REF_DIV_MASK		0xf
#define AR724X_AHB_DIV_SHIFT		19
#define AR724X_AHB_DIV_MASK		0x1
#define AR724X_DDR_DIV_SHIFT		22
#define AR724X_DDR_DIV_MASK		0x3

#define AR91XX_PLL_REG_CPU_CONFIG	0x00
#define AR91XX_PLL_REG_ETH_CONFIG	0x04
#define AR91XX_PLL_REG_ETH0_INT_CLOCK	0x14
#define AR91XX_PLL_REG_ETH1_INT_CLOCK	0x18

#define AR91XX_PLL_DIV_SHIFT		0
#define AR91XX_PLL_DIV_MASK		0x3ff
#define AR91XX_DDR_DIV_SHIFT		22
#define AR91XX_DDR_DIV_MASK		0x3
#define AR91XX_AHB_DIV_SHIFT		19
#define AR91XX_AHB_DIV_MASK		0x1

#define AR91XX_ETH0_PLL_SHIFT		20
#define AR91XX_ETH1_PLL_SHIFT		22

// extern void __iomem *ar71xx_pll_base;

// static inline void ar71xx_pll_wr(unsigned reg, u32 val)
// {
	// __raw_writel(val, ar71xx_pll_base + reg);
// }

// static inline u32 ar71xx_pll_rr(unsigned reg)
// {
	// return __raw_readl(ar71xx_pll_base + reg);
// }

/*
 * USB_CONFIG block
 */
#define USB_CTRL_REG_FLADJ	0x00
#define USB_CTRL_REG_CONFIG	0x04

// extern void __iomem *ar71xx_usb_ctrl_base;

// static inline void ar71xx_usb_ctrl_wr(unsigned reg, u32 val)
// {
	// __raw_writel(val, ar71xx_usb_ctrl_base + reg);
// }

// static inline u32 ar71xx_usb_ctrl_rr(unsigned reg)
// {
	// return __raw_readl(ar71xx_usb_ctrl_base + reg);
// }

/*
 * GPIO block
 */
#define GPIO_REG_OE		0x00
#define GPIO_REG_IN		0x04
#define GPIO_REG_OUT		0x08
#define GPIO_REG_SET		0x0c
#define GPIO_REG_CLEAR		0x10
#define GPIO_REG_INT_MODE	0x14
#define GPIO_REG_INT_TYPE	0x18
#define GPIO_REG_INT_POLARITY	0x1c
#define GPIO_REG_INT_PENDING	0x20
#define GPIO_REG_INT_ENABLE	0x24
#define GPIO_REG_FUNC		0x28

#define AR71XX_GPIO_FUNC_STEREO_EN	BIT(17)
#define AR71XX_GPIO_FUNC_SLIC_EN	BIT(16)
#define AR71XX_GPIO_FUNC_SPI_CS2_EN	BIT(13)
#define AR71XX_GPIO_FUNC_SPI_CS1_EN	BIT(12)
#define AR71XX_GPIO_FUNC_UART_EN	BIT(8)
#define AR71XX_GPIO_FUNC_USB_OC_EN	BIT(4)
#define AR71XX_GPIO_FUNC_USB_CLK_EN	BIT(0)

#define AR71XX_GPIO_COUNT	16

#define AR724X_GPIO_FUNC_GE0_MII_CLK_EN		BIT(19)
#define AR724X_GPIO_FUNC_SPI_EN			BIT(18)
#define AR724X_GPIO_FUNC_SPI_CS_EN2		BIT(14)
#define AR724X_GPIO_FUNC_SPI_CS_EN1		BIT(13)
#define AR724X_GPIO_FUNC_CLK_OBS5_EN		BIT(12)
#define AR724X_GPIO_FUNC_CLK_OBS4_EN		BIT(11)
#define AR724X_GPIO_FUNC_CLK_OBS3_EN		BIT(10)
#define AR724X_GPIO_FUNC_CLK_OBS2_EN		BIT(9)
#define AR724X_GPIO_FUNC_CLK_OBS1_EN		BIT(8)
#define AR724X_GPIO_FUNC_ETH_SWITCH_LED4_EN	BIT(7)
#define AR724X_GPIO_FUNC_ETH_SWITCH_LED3_EN	BIT(6)
#define AR724X_GPIO_FUNC_ETH_SWITCH_LED2_EN	BIT(5)
#define AR724X_GPIO_FUNC_ETH_SWITCH_LED1_EN	BIT(4)
#define AR724X_GPIO_FUNC_ETH_SWITCH_LED0_EN	BIT(3)
#define AR724X_GPIO_FUNC_UART_RTS_CTS_EN	BIT(2)
#define AR724X_GPIO_FUNC_UART_EN		BIT(1)
#define AR724X_GPIO_FUNC_JTAG_DISABLE		BIT(0)

#define AR724X_GPIO_COUNT	18

#define AR91XX_GPIO_FUNC_WMAC_LED_EN	BIT(22)
#define AR91XX_GPIO_FUNC_EXP_PORT_CS_EN	BIT(21)
#define AR91XX_GPIO_FUNC_I2S_REFCLKEN	BIT(20)
#define AR91XX_GPIO_FUNC_I2S_MCKEN	BIT(19)
#define AR91XX_GPIO_FUNC_I2S1_EN	BIT(18)
#define AR91XX_GPIO_FUNC_I2S0_EN	BIT(17)
#define AR91XX_GPIO_FUNC_SLIC_EN	BIT(16)
#define AR91XX_GPIO_FUNC_UART_RTSCTS_EN	BIT(9)
#define AR91XX_GPIO_FUNC_UART_EN	BIT(8)
#define AR91XX_GPIO_FUNC_USB_CLK_EN	BIT(4)

#define AR91XX_GPIO_COUNT	22

// extern void __iomem *ar71xx_gpio_base;

// static inline void ar71xx_gpio_wr(unsigned reg, u32 value)
// {
	// __raw_writel(value, ar71xx_gpio_base + reg);
// }

// static inline u32 ar71xx_gpio_rr(unsigned reg)
// {
	// return __raw_readl(ar71xx_gpio_base + reg);
// }

// void ar71xx_gpio_init(void) __init;
// void ar71xx_gpio_function_enable(u32 mask);
// void ar71xx_gpio_function_disable(u32 mask);
// void ar71xx_gpio_function_setup(u32 set, u32 clear);

/*
 * DDR_CTRL block
 */
#define AR71XX_DDR_REG_PCI_WIN0		0x7c
#define AR71XX_DDR_REG_PCI_WIN1		0x80
#define AR71XX_DDR_REG_PCI_WIN2		0x84
#define AR71XX_DDR_REG_PCI_WIN3		0x88
#define AR71XX_DDR_REG_PCI_WIN4		0x8c
#define AR71XX_DDR_REG_PCI_WIN5		0x90
#define AR71XX_DDR_REG_PCI_WIN6		0x94
#define AR71XX_DDR_REG_PCI_WIN7		0x98
#define AR71XX_DDR_REG_FLUSH_GE0	0x9c
#define AR71XX_DDR_REG_FLUSH_GE1	0xa0
#define AR71XX_DDR_REG_FLUSH_USB	0xa4
#define AR71XX_DDR_REG_FLUSH_PCI	0xa8

#define AR724X_DDR_REG_FLUSH_GE0	0x7c
#define AR724X_DDR_REG_FLUSH_GE1	0x80
#define AR724X_DDR_REG_FLUSH_USB	0x84
#define AR724X_DDR_REG_FLUSH_PCIE	0x88

#define AR91XX_DDR_REG_FLUSH_GE0	0x7c
#define AR91XX_DDR_REG_FLUSH_GE1	0x80
#define AR91XX_DDR_REG_FLUSH_USB	0x84
#define AR91XX_DDR_REG_FLUSH_WMAC	0x88

#define PCI_WIN0_OFFS	0x10000000
#define PCI_WIN1_OFFS	0x11000000
#define PCI_WIN2_OFFS	0x12000000
#define PCI_WIN3_OFFS	0x13000000
#define PCI_WIN4_OFFS	0x14000000
#define PCI_WIN5_OFFS	0x15000000
#define PCI_WIN6_OFFS	0x16000000
#define PCI_WIN7_OFFS	0x07000000

// extern void __iomem *ar71xx_ddr_base;

// static inline void ar71xx_ddr_wr(unsigned reg, u32 val)
// {
	// __raw_writel(val, ar71xx_ddr_base + reg);
// }

// static inline u32 ar71xx_ddr_rr(unsigned reg)
// {
	// return __raw_readl(ar71xx_ddr_base + reg);
// }

// void ar71xx_ddr_flush(u32 reg);

/*
 * PCI block
 */
#define AR71XX_PCI_CFG_BASE	(AR71XX_PCI_MEM_BASE + PCI_WIN7_OFFS + 0x10000)
#define AR71XX_PCI_CFG_SIZE	0x100

#define PCI_REG_CRP_AD_CBE	0x00
#define PCI_REG_CRP_WRDATA	0x04
#define PCI_REG_CRP_RDDATA	0x08
#define PCI_REG_CFG_AD		0x0c
#define PCI_REG_CFG_CBE		0x10
#define PCI_REG_CFG_WRDATA	0x14
#define PCI_REG_CFG_RDDATA	0x18
#define PCI_REG_PCI_ERR		0x1c
#define PCI_REG_PCI_ERR_ADDR	0x20
#define PCI_REG_AHB_ERR		0x24
#define PCI_REG_AHB_ERR_ADDR	0x28

#define PCI_CRP_CMD_WRITE	0x00010000
#define PCI_CRP_CMD_READ	0x00000000
#define PCI_CFG_CMD_READ	0x0000000a
#define PCI_CFG_CMD_WRITE	0x0000000b

#define PCI_IDSEL_ADL_START	17

#define AR724X_PCI_CFG_BASE	(AR71XX_PCI_MEM_BASE + 0x4000000)
#define AR724X_PCI_CFG_SIZE	0x1000

#define AR724X_PCI_REG_APP		0x00
#define AR724X_PCI_REG_RESET		0x18
#define AR724X_PCI_REG_INT_STATUS	0x4c
#define AR724X_PCI_REG_INT_MASK		0x50

#define AR724X_PCI_APP_LTSSM_ENABLE	BIT(0)
#define AR724X_PCI_RESET_LINK_UP	BIT(0)

#define AR724X_PCI_INT_DEV0		BIT(14)

/*
 * RESET block
 */
#define AR71XX_RESET_REG_TIMER			0x00
#define AR71XX_RESET_REG_TIMER_RELOAD		0x04
#define AR71XX_RESET_REG_WDOG_CTRL		0x08
#define AR71XX_RESET_REG_WDOG			0x0c
#define AR71XX_RESET_REG_MISC_INT_STATUS	0x10
#define AR71XX_RESET_REG_MISC_INT_ENABLE	0x14
#define AR71XX_RESET_REG_PCI_INT_STATUS		0x18
#define AR71XX_RESET_REG_PCI_INT_ENABLE		0x1c
#define AR71XX_RESET_REG_GLOBAL_INT_STATUS	0x20
#define AR71XX_RESET_REG_RESET_MODULE		0x24
#define AR71XX_RESET_REG_PERFC_CTRL		0x2c
#define AR71XX_RESET_REG_PERFC0			0x30
#define AR71XX_RESET_REG_PERFC1			0x34
#define AR71XX_RESET_REG_REV_ID			0x90

#define AR91XX_RESET_REG_GLOBAL_INT_STATUS	0x18
#define AR91XX_RESET_REG_RESET_MODULE		0x1c
#define AR91XX_RESET_REG_PERF_CTRL		0x20
#define AR91XX_RESET_REG_PERFC0			0x24
#define AR91XX_RESET_REG_PERFC1			0x28

#define AR724X_RESET_REG_RESET_MODULE		0x1c

#define WDOG_CTRL_LAST_RESET		BIT(31)
#define WDOG_CTRL_ACTION_MASK		3
#define WDOG_CTRL_ACTION_NONE		0	/* no action */
#define WDOG_CTRL_ACTION_GPI		1	/* general purpose interrupt */
#define WDOG_CTRL_ACTION_NMI		2	/* NMI */
#define WDOG_CTRL_ACTION_FCR		3	/* full chip reset */

#define MISC_INT_DMA			BIT(7)
#define MISC_INT_OHCI			BIT(6)
#define MISC_INT_PERFC			BIT(5)
#define MISC_INT_WDOG			BIT(4)
#define MISC_INT_UART			BIT(3)
#define MISC_INT_GPIO			BIT(2)
#define MISC_INT_ERROR			BIT(1)
#define MISC_INT_TIMER			BIT(0)

#define PCI_INT_CORE			BIT(4)
#define PCI_INT_DEV2			BIT(2)
#define PCI_INT_DEV1			BIT(1)
#define PCI_INT_DEV0			BIT(0)

#define RESET_MODULE_EXTERNAL		BIT(28)
#define RESET_MODULE_FULL_CHIP		BIT(24)
#define RESET_MODULE_AMBA2WMAC		BIT(22)
#define RESET_MODULE_CPU_NMI		BIT(21)
#define RESET_MODULE_CPU_COLD		BIT(20)
#define RESET_MODULE_DMA		BIT(19)
#define RESET_MODULE_SLIC		BIT(18)
#define RESET_MODULE_STEREO		BIT(17)
#define RESET_MODULE_DDR		BIT(16)
#define RESET_MODULE_GE1_MAC		BIT(13)
#define RESET_MODULE_GE1_PHY		BIT(12)
#define RESET_MODULE_USBSUS_OVERRIDE	BIT(10)
#define RESET_MODULE_GE0_MAC		BIT(9)
#define RESET_MODULE_GE0_PHY		BIT(8)
#define RESET_MODULE_USB_OHCI_DLL	BIT(6)
#define RESET_MODULE_USB_HOST		BIT(5)
#define RESET_MODULE_USB_PHY		BIT(4)
#define RESET_MODULE_USB_OHCI_DLL_7240	BIT(3)
#define RESET_MODULE_PCI_BUS		BIT(1)
#define RESET_MODULE_PCI_CORE		BIT(0)

#define AR724X_RESET_GE1_MDIO		BIT(23)
#define AR724X_RESET_GE0_MDIO		BIT(22)
#define AR724X_RESET_PCIE_PHY_SERIAL	BIT(10)
#define AR724X_RESET_PCIE_PHY		BIT(7)
#define AR724X_RESET_PCIE		BIT(6)

#define REV_ID_MAJOR_MASK	0xfff0
#define REV_ID_MAJOR_AR71XX	0x00a0
#define REV_ID_MAJOR_AR913X	0x00b0
#define REV_ID_MAJOR_AR7240	0x00c0
#define REV_ID_MAJOR_AR7241	0x0100
#define REV_ID_MAJOR_AR7242	0x1100

#define AR71XX_REV_ID_MINOR_MASK	0x3
#define AR71XX_REV_ID_MINOR_AR7130	0x0
#define AR71XX_REV_ID_MINOR_AR7141	0x1
#define AR71XX_REV_ID_MINOR_AR7161	0x2
#define AR71XX_REV_ID_REVISION_MASK	0x3
#define AR71XX_REV_ID_REVISION_SHIFT	2

#define AR91XX_REV_ID_MINOR_MASK	0x3
#define AR91XX_REV_ID_MINOR_AR9130	0x0
#define AR91XX_REV_ID_MINOR_AR9132	0x1
#define AR91XX_REV_ID_REVISION_MASK	0x3
#define AR91XX_REV_ID_REVISION_SHIFT	2

#define AR724X_REV_ID_REVISION_MASK	0x3

// extern void __iomem *ar71xx_reset_base;

static inline void ar71xx_reset_wr(unsigned reg, u32 val)
{
	__raw_writel(val, KSEG1ADDR(AR71XX_RESET_BASE) + reg);
}

static inline u32 ar71xx_reset_rr(unsigned reg)
{
	return __raw_readl(KSEG1ADDR(AR71XX_RESET_BASE) + reg);
}

// void ar71xx_device_stop(u32 mask);
// void ar71xx_device_start(u32 mask);
// int ar71xx_device_stopped(u32 mask);

/*
 * SPI block
 */
#define SPI_REG_FS		0x00	/* Function Select */
#define SPI_REG_CTRL		0x04	/* SPI Control */
#define SPI_REG_IOC		0x08	/* SPI I/O Control */
#define SPI_REG_RDS		0x0c	/* Read Data Shift */

#define SPI_FS_GPIO		BIT(0)	/* Enable GPIO mode */

#define SPI_CTRL_RD		BIT(6)	/* Remap Disable */
#define SPI_CTRL_DIV_MASK	0x3f

#define SPI_IOC_DO		BIT(0)	/* Data Out pin */
#define SPI_IOC_CLK		BIT(8)	/* CLK pin */
#define SPI_IOC_CS(n)		BIT(16 + (n))
#define SPI_IOC_CS0		SPI_IOC_CS(0)
#define SPI_IOC_CS1		SPI_IOC_CS(1)
#define SPI_IOC_CS2		SPI_IOC_CS(2)
#define SPI_IOC_CS_ALL		(SPI_IOC_CS0 | SPI_IOC_CS1 | SPI_IOC_CS2)

// void ar71xx_flash_acquire(void);
// void ar71xx_flash_release(void);

/*
 * MII_CTRL block
 */
#define MII_REG_MII0_CTRL	0x00
#define MII_REG_MII1_CTRL	0x04

#define MII0_CTRL_IF_GMII	0
#define MII0_CTRL_IF_MII	1
#define MII0_CTRL_IF_RGMII	2
#define MII0_CTRL_IF_RMII	3

#define MII1_CTRL_IF_RGMII	0
#define MII1_CTRL_IF_RMII	1

#endif /* __ASSEMBLER__ */

#endif /* __ASM_MACH_AR71XX_H */
