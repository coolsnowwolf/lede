/*
 * (C) Copyright 2010 Quantenna Communications Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Header file which describes Ruby platform.
 * Has to be used by both kernel and bootloader.
 */

#ifndef __RUBY_CONFIG_H
#define __RUBY_CONFIG_H

#include "topaz_config.h"

/*******************************************************************/

#if TOPAZ_MMAP_UNIFIED
	#define RUBY_MMAP_FLIP		0
#else
	#if !(defined(MUC_BUILD) || defined(DSP_BUILD) || defined(AUC_BUILD))
		#define RUBY_MMAP_FLIP		1
	#else
		#define RUBY_MMAP_FLIP		0
	#endif
#endif

/* Set to 1 if MuC need to enable TLB, otherwise set to 0 */
#define RUBY_MUC_TLB_ENABLE		1

/*******************************************************************/

#ifdef RUBY_PLATFORM

	#if RUBY_FPGA_PLATFORM
		#define RUBY_SERIAL_BAUD	38400
		#define RUBY_FIXED_DEV_CLK	12500000
		#define RUBY_FIXED_CPU_CLK	40000000
		#define RUBY_FPGA_DDR
	#else
		#define RUBY_SERIAL_BAUD	115200
		#define RUBY_FIXED_DEV_CLK	125000000
		#define RUBY_FIXED_CPU_CLK	400000000
		#define RUBY_ASIC_DDR
	#endif /* #if RUBY_FPGA_PLATFORM */

	#define UPF_SPD_FLAG	0
	#define DEFAULT_BAUD	RUBY_SERIAL_BAUD

#endif /* #ifdef RUBY_PLATFORM */

/*******************************************************************/
/* Define some constants for Linux ARC kernel */
#define CONFIG_ARC700_SERIAL_BAUD	RUBY_SERIAL_BAUD
#define CONFIG_ARC700_CLK		RUBY_FIXED_CPU_CLK
#define CONFIG_ARC700_DEV_CLK		RUBY_FIXED_DEV_CLK

/*******************************************************************/

/* RGMII related defines */
#define CONFIG_ARCH_RUBY_ENET_RGMII

#define CONFIG_ARCH_RGMII_DEFAULT	0x8F8F8F8F
#define CONFIG_ARCH_RGMII_DLL_TIMING	0x8F8D8F8F
#define CONFIG_ARCH_RGMII_S1P8NS_H1P9NS	0x8F891F1F
#define CONFIG_ARCH_RGMII_NODELAY	0x1F1F1F1F
#define CONFIG_ARCH_RGMII_710F		CONFIG_ARCH_RGMII_NODELAY
#define CONFIG_ARCH_RGMII_P1RX00TX0E    0x0E8E1F1F

/* EMAC related defines */

/* EMAC flags */
#define EMAC_NOT_IN_USE			(0)
#define EMAC_IN_USE			(BIT(0))
#define EMAC_PHY_NOT_IN_USE		(BIT(1))  // do not initialize/access phy mdio
#define EMAC_PHY_FORCE_10MB		(BIT(2))
#define EMAC_PHY_FORCE_100MB		(BIT(3))
#define EMAC_PHY_FORCE_1000MB		(BIT(4))
#define EMAC_PHY_FORCE_HDX		(BIT(5))
#define EMAC_PHY_RESET			(BIT(6)) // force PHY reset
#define EMAC_PHY_MII			(BIT(7)) // default is rgmii
#define EMAC_PHY_AUTO_MASK		(EMAC_PHY_FORCE_10MB | EMAC_PHY_FORCE_100MB | EMAC_PHY_FORCE_1000MB)
#define EMAC_PHY_AR8236			(BIT(8))
#define EMAC_PHY_AR8327			(BIT(9))
#define EMAC_PHY_GPIO1_RESET		(BIT(10))
#define EMAC_PHY_GPIO13_RESET		(BIT(11))
#define EMAC_PHY_NO_COC			(BIT(12)) // do not adjust link speed for power savings
#define EMAC_PHY_MV88E6071		(BIT(13))
#define EMAC_PHY_FPGAA_ONLY		(BIT(15))
#define EMAC_PHY_FPGAB_ONLY		(BIT(16))
#define EMAC_PHY_RTL8363SB_P0		(BIT(18))
#define EMAC_PHY_RTL8363SB_P1		(BIT(19))
#define EMAC_BONDED			(BIT(20))
#define EMAC_PHY_RTL8365MB		(BIT(21))
#define EMAC_PHY_RTL8211DS		(BIT(22))
#define EMAC_PHY_CUSTOM			(BIT(31))

#define EMAC_MV88E6071			(EMAC_IN_USE | EMAC_PHY_MII | EMAC_PHY_NOT_IN_USE |	\
						EMAC_PHY_NO_COC | EMAC_PHY_FORCE_100MB | EMAC_PHY_MV88E6071)
#define EMAC_SLOW_PHY			(EMAC_PHY_FORCE_10MB|EMAC_PHY_FORCE_100MB|EMAC_PHY_MII)

/* force phy addr scan */
#define EMAC_PHY_ADDR_SCAN		(32)	// scan bus for addr

/* Flash memory sizes */
#define FLASH_32MB			(32*1024*1024)
#define FLASH_16MB			(16*1024*1024)
#define FLASH_8MB			(8*1024*1024)
#define FLASH_4MB			(4*1024*1024)
#define FLASH_2MB			(2*1024*1024)
#define FLASH_64KB			(64*1024)
#define DEFAULT_FLASH_SIZE		(FLASH_8MB)
#define FLASH_SIZE_JEDEC		(0)

/* DDR memory sizes */
#define DDR_256MB			(256*1024*1024)
#define DDR_128MB			(128*1024*1024)
#define DDR_64MB			(64*1024*1024)
#define DDR_32MB			(32*1024*1024)
#define DDR_AUTO			(0)
#define DEFAULT_DDR_SIZE		(DDR_64MB)

/* Other DDR defines */
#define DDR3_800MHz		800
#define DDR3_640MHz		640
#define DDR3_500MHz		500
#define DDR3_400MHz		400
#define DDR3_320MHz		320
#define DDR_400			400
#define DDR_320			320
#define DDR_250			250
#define DDR_200			200
#define DDR_160			160
#define DDR_125			125
#define DEFAULT_DDR_SPEED	(DDR_160)

#define	DDR_32_MICRON		0
#define DDR_16_MICRON		1
#define DDR_16_ETRON		2
#define DDR_16_SAMSUNG		3
#define DDR_32_ETRON		4
#define DDR_32_SAMSUNG		5
#define DDR_16_HYNIX		6
#define DDR3_16_WINBOND		7
#define DDR3_32_WINBOND		8
#define DEFAULT_DDR_CFG		(DDR_16_MICRON)

/* UART1 defines */
#define	UART1_NOT_IN_USE	0
#define	UART1_IN_USE		1

#define PCIE_NOT_IN_USE		0
#define PCIE_IN_USE		(BIT(0))
#define PCIE_USE_PHY_LOOPBK	(BIT(1))
#define PCIE_RC_MODE		(BIT(2))
#define PCIE_ENDPOINT		(PCIE_IN_USE | PCIE_USE_PHY_LOOPBK)
#define PCIE_ROOTCOMPLEX	(PCIE_IN_USE | PCIE_RC_MODE | PCIE_USE_PHY_LOOPBK)

/*******************************************************************/

#define CONFIG_USE_SPI1_FOR_IPC	PLATFORM_REG_SWITCH(1, 0)

#endif // #ifndef __RUBY_CONFIG_H


