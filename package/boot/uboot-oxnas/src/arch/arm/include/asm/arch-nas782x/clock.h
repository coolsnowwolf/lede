#ifndef _NAS782X_CLOCK_H
#define _NAS782X_CLOCK_H

#include <asm/arch/sysctl.h>
#include <asm/arch/cpu.h>

/* bit numbers of clock control register */
#define SYS_CTRL_CLK_COPRO  0
#define SYS_CTRL_CLK_DMA    1
#define SYS_CTRL_CLK_CIPHER 2
#define SYS_CTRL_CLK_SD     3
#define SYS_CTRL_CLK_SATA   4
#define SYS_CTRL_CLK_I2S    5
#define SYS_CTRL_CLK_USBHS  6
#define SYS_CTRL_CLK_MACA   7
#define SYS_CTRL_CLK_MAC   SYS_CTRL_CLK_MACA
#define SYS_CTRL_CLK_PCIEA  8
#define SYS_CTRL_CLK_STATIC 9
#define SYS_CTRL_CLK_MACB   10
#define SYS_CTRL_CLK_PCIEB  11
#define SYS_CTRL_CLK_REF600 12
#define SYS_CTRL_CLK_USBDEV 13
#define SYS_CTRL_CLK_DDR    14
#define SYS_CTRL_CLK_DDRPHY 15
#define SYS_CTRL_CLK_DDRCK  16

/* bit numbers of reset control register */
#define SYS_CTRL_RST_SCU          0
#define SYS_CTRL_RST_COPRO        1
#define SYS_CTRL_RST_ARM0         2
#define SYS_CTRL_RST_ARM1         3
#define SYS_CTRL_RST_USBHS        4
#define SYS_CTRL_RST_USBHSPHYA    5
#define SYS_CTRL_RST_MACA         6
#define SYS_CTRL_RST_MAC	SYS_CTRL_RST_MACA
#define SYS_CTRL_RST_PCIEA        7
#define SYS_CTRL_RST_SGDMA        8
#define SYS_CTRL_RST_CIPHER       9
#define SYS_CTRL_RST_DDR          10
#define SYS_CTRL_RST_SATA         11
#define SYS_CTRL_RST_SATA_LINK    12
#define SYS_CTRL_RST_SATA_PHY     13
#define SYS_CTRL_RST_PCIEPHY      14
#define SYS_CTRL_RST_STATIC       15
#define SYS_CTRL_RST_GPIO         16
#define SYS_CTRL_RST_UART1        17
#define SYS_CTRL_RST_UART2        18
#define SYS_CTRL_RST_MISC         19
#define SYS_CTRL_RST_I2S          20
#define SYS_CTRL_RST_SD           21
#define SYS_CTRL_RST_MACB         22
#define SYS_CTRL_RST_PCIEB        23
#define SYS_CTRL_RST_VIDEO        24
#define SYS_CTRL_RST_DDR_PHY      25
#define SYS_CTRL_RST_USBHSPHYB    26
#define SYS_CTRL_RST_USBDEV       27
#define SYS_CTRL_RST_ARMDBG       29
#define SYS_CTRL_RST_PLLA         30
#define SYS_CTRL_RST_PLLB         31

static inline void reset_block(int block, int reset)
{
	u32 reg;
	if (reset)
		reg = SYS_CTRL_RST_SET_CTRL;
	else
		reg = SYS_CTRL_RST_CLR_CTRL;

	writel(BIT(block), reg);
}

static inline void enable_clock(int block)
{
	writel(BIT(block), SYS_CTRL_CLK_SET_CTRL);
}

static inline void disable_clock(int block)
{
	writel(BIT(block), SYS_CTRL_CLK_CLR_CTRL);
}

int plla_set_config(int idx);

#endif /* _NAS782X_CLOCK_H */
