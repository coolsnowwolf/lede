/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _RTL838X_SPI_H
#define _RTL838X_SPI_H


/*
 * Register access macros
 */

#define spi_r32(reg)		readl(rtl838x_nor->base + reg)
#define spi_w32(val, reg)	writel(val, rtl838x_nor->base + reg)
#define spi_w32_mask(clear, set, reg)	\
	spi_w32((spi_r32(reg) & ~(clear)) | (set), reg)

#define SPI_WAIT_READY		do { \
				} while (!(spi_r32(SFCSR) & SFCSR_SPI_RDY))

#define spi_w32w(val, reg)	do { \
					writel(val, rtl838x_nor->base + reg); \
					SPI_WAIT_READY; \
				} while (0)

#define SFCR   (0x00)			/*SPI Flash Configuration Register*/
	#define SFCR_CLK_DIV(val)	((val)<<29)
	#define SFCR_EnableRBO		(1<<28)
	#define SFCR_EnableWBO		(1<<27)
	#define SFCR_SPI_TCS(val)	((val)<<23) /*4 bit, 1111 */

#define SFCR2  (0x04)	/*For memory mapped I/O */
	#define SFCR2_SFCMD(val)	((val)<<24) /*8 bit, 1111_1111 */
	#define SFCR2_SIZE(val)		((val)<<21) /*3 bit, 111 */
	#define SFCR2_RDOPT		(1<<20)
	#define SFCR2_CMDIO(val)	((val)<<18) /*2 bit, 11 */
	#define SFCR2_ADDRIO(val)	((val)<<16) /*2 bit, 11 */
	#define SFCR2_DUMMYCYCLE(val)	((val)<<13) /*3 bit, 111 */
	#define SFCR2_DATAIO(val)	((val)<<11) /*2 bit, 11 */
	#define SFCR2_HOLD_TILL_SFDR2	(1<<10)
	#define SFCR2_GETSIZE(x)	(((x)&0x00E00000)>>21)

#define SFCSR  (0x08)	/*SPI Flash Control&Status Register*/
	#define SFCSR_SPI_CSB0		(1<<31)
	#define SFCSR_SPI_CSB1		(1<<30)
	#define SFCSR_LEN(val)		((val)<<28)  /*2 bits*/
	#define SFCSR_SPI_RDY		(1<<27)
	#define SFCSR_IO_WIDTH(val)	((val)<<25)  /*2 bits*/
	#define SFCSR_CHIP_SEL		(1<<24)
	#define SFCSR_CMD_BYTE(val)	((val)<<16)  /*8 bit, 1111_1111 */

#define SFDR   (0x0C)	/*SPI Flash Data Register*/
#define SFDR2  (0x10)	/*SPI Flash Data Register - for post SPI bootup setting*/
	#define SPI_CS_INIT		(SFCSR_SPI_CSB0 | SFCSR_SPI_CSB1 | SPI_LEN1)
	#define SPI_CS0			SFCSR_SPI_CSB0
	#define SPI_CS1			SFCSR_SPI_CSB1
	#define SPI_eCS0		((SFCSR_SPI_CSB1)) /*and SFCSR to active CS0*/
	#define SPI_eCS1		((SFCSR_SPI_CSB0)) /*and SFCSR to active CS1*/

	#define SPI_WIP (1)		/* Write In Progress */
	#define SPI_WEL (1<<1)		/* Write Enable Latch*/
	#define SPI_SST_QIO_WIP (1<<7)	/* SST QIO Flash Write In Progress */
	#define SPI_LEN_INIT 0xCFFFFFFF /* and SFCSR to init   */
	#define SPI_LEN4    0x30000000	/* or SFCSR to set */
	#define SPI_LEN3    0x20000000	/* or SFCSR to set */
	#define SPI_LEN2    0x10000000	/* or SFCSR to set */
	#define SPI_LEN1    0x00000000	/* or SFCSR to set */
	#define SPI_SETLEN(val) do {		\
			SPI_REG(SFCSR) &= 0xCFFFFFFF;   \
			SPI_REG(SFCSR) |= (val-1)<<28;	\
		} while (0)
/*
 * SPI interface control
 */
#define RTL8390_SOC_SPI_MMIO_CONF (0x04)

#define IOSTATUS_CIO_MASK (0x00000038)

/* Chip select: bits 4-7*/
#define CS0 (1<<4)
#define R_MODE 0x04

/* io_status */
#define IO1 (1<<0)
#define IO2 (1<<1)
#define CIO1 (1<<3)
#define CIO2 (1<<4)
#define CMD_IO1 (1<<6)
#define W_ADDR_IO1 ((1)<<12)
#define R_ADDR_IO2 ((2)<<9)
#define R_DATA_IO2 ((2)<<15)
#define W_DATA_IO1 ((1)<<18)

/* Commands */
#define SPI_C_RSTQIO 0xFF

#define SPI_MAX_TRANSFER_SIZE 256

#endif		/* _RTL838X_SPI_H */
