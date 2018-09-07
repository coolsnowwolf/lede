/*
 * Copyright (C) 2003 Artec Design Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef __ASM_ARCH_MCS814X_H
#define __ASM_ARCH_MCS814X_H

#define MCS814X_IO_BASE		0xF0000000
#define MCS814X_IO_START	0x40000000
#define MCS814X_IO_SIZE		0x00100000

/* IRQ controller register offset */
#define MCS814X_IRQ_ICR		0x00
#define MCS814X_IRQ_ISR		0x04
#define MCS814X_IRQ_MASK	0x20
#define MCS814X_IRQ_STS0	0x40

#define MCS814X_PHYS_BASE	0x40000000
#define MCS814X_VIRT_BASE	MCS814X_IO_BASE

#define MCS814X_UART    	0x000DC000
#define MCS814X_DBGLED  	0x000EC000
#define MCS814X_SYSDBG  	0x000F8000
#define MCS814X_SYSDBG_SIZE	0x50

/* System configuration and bootstrap registers */
#define SYSDBG_BS1		0x00
#define  CPU_FREQ_SHIFT		27
#define  CPU_FREQ_MASK		0x0F
#define  SDRAM_FREQ_BIT		(1 << 22)

#define SYSDBG_BS2		0x04
#define  LED_CFG_MASK		0x03
#define  CPU_MODE_SHIFT		23
#define  CPU_MODE_MASK		0x03

#define SYSDBG_SYSCTL_MAC	0x1d
#define  BUF_SHIFT_BIT		(1 << 0)

#define SYSDBG_SYSCTL		0x08
#define  SYSCTL_EMAC		(1 << 0)
#define  SYSCTL_EPHY		(1 << 0) /* active low */
#define  SYSCTL_CIPHER		(1 << 16)

#define SYSDBG_PLL_CTL		0x3C

#endif /* __ASM_ARCH_MCS814X_H */

