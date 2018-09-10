/*
 *  ADM5120 MPMC (Multiport Memory Controller) register definitions
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_MPMC_H
#define _MACH_ADM5120_MPMC_H

#define MPMC_READ_REG(r)	__raw_readl( \
	(void __iomem *)KSEG1ADDR(ADM5120_MPMC_BASE) + MPMC_REG_ ## r)
#define MPMC_WRITE_REG(r, v)	__raw_writel((v), \
	(void __iomem *)KSEG1ADDR(ADM5120_MPMC_BASE) + MPMC_REG_ ## r)

#define MPMC_REG_CTRL	0x0000
#define MPMC_REG_STATUS	0x0004
#define MPMC_REG_CONF	0x0008
#define MPMC_REG_DC	0x0020
#define MPMC_REG_DR	0x0024
#define MPMC_REG_DRP	0x0030

#define MPMC_REG_DC0	0x0100
#define MPMC_REG_DRC0	0x0104
#define MPMC_REG_DC1	0x0120
#define MPMC_REG_DRC1	0x0124
#define MPMC_REG_DC2	0x0140
#define MPMC_REG_DRC2	0x0144
#define MPMC_REG_DC3	0x0160
#define MPMC_REG_DRC3	0x0164
#define MPMC_REG_SC0	0x0200  /* for F_CS1_N */
#define MPMC_REG_SC1	0x0220  /* for F_CS0_N */
#define MPMC_REG_SC2    0x0240
#define MPMC_REG_WEN2	0x0244
#define MPMC_REG_OEN2	0x0248
#define MPMC_REG_RD2	0x024C
#define MPMC_REG_PG2	0x0250
#define MPMC_REG_WR2	0x0254
#define MPMC_REG_TN2	0x0258
#define MPMC_REG_SC3    0x0260

/* Control register bits */
#define MPMC_CTRL_AM		(1 << 1)	/* Address Mirror */
#define MPMC_CTRL_LPM		(1 << 2)	/* Low Power Mode */
#define MPMC_CTRL_DWB		(1 << 3)	/* Drain Write Buffers */

/* Status register bits */
#define MPMC_STATUS_BUSY	(1 << 0)	/* Busy */
#define MPMC_STATUS_WBS		(1 << 1)	/* Write Buffer Status */
#define MPMC_STATUS_SRA		(1 << 2)	/* Self-Refresh Acknowledge*/

/* Dynamic Control register bits */
#define MPMC_DC_CE		(1 << 0)
#define MPMC_DC_DMC		(1 << 1)
#define MPMC_DC_SRR		(1 << 2)
#define MPMC_DC_SI_SHIFT	7
#define MPMC_DC_SI_MASK		(3 << 7)
#define MPMC_DC_SI_NORMAL	(0 << 7)
#define MPMC_DC_SI_MODE		(1 << 7)
#define MPMC_DC_SI_PALL		(2 << 7)
#define MPMC_DC_SI_NOP		(3 << 7)

#define SRAM_REG_CONF	0x00
#define SRAM_REG_WWE	0x04
#define SRAM_REG_WOE	0x08
#define SRAM_REG_WRD    0x0C
#define SRAM_REG_WPG    0x10
#define SRAM_REG_WWR    0x14
#define SRAM_REG_WTR    0x18

/* Dynamic Configuration register bits */
#define DC_BE		(1 << 19) /* buffer enable */
#define DC_RW_SHIFT	28	/* shift for number of rows */
#define DC_RW_MASK	0x03
#define DC_NB_SHIFT	26	/* shift for number of banks */
#define DC_NB_MASK	0x01
#define DC_CW_SHIFT	22	/* shift for number of columns */
#define DC_CW_MASK	0x07
#define DC_DW_SHIFT	7	/* shift for device width */
#define DC_DW_MASK	0x03

/* Static Configuration register bits */
#define SC_MW_MASK	0x03	/* memory width mask */
#define SC_MW_8		0x00	/* 8 bit memory width */
#define SC_MW_16	0x01	/* 16 bit memory width */
#define SC_MW_32	0x02	/* 32 bit memory width */

#endif /* _MACH_ADM5120_MPMC_H */
