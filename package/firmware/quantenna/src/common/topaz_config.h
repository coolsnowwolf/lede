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
 * Header file which describes Topaz platform.
 * Has to be used by both kernel and bootloader.
 */

#ifndef __TOPAZ_CONFIG_H
#define __TOPAZ_CONFIG_H

#include "current_platform.h"

#ifdef TOPAZ_PLATFORM
#if !TOPAZ_FPGA_PLATFORM
#undef TOPAZ_ICACHE_WORKAROUND
#endif
#endif

/*
 * Control registers move depending on unified + alias bit
 */

#ifdef TOPAZ_PLATFORM
	#define TOPAZ_MMAP_UNIFIED	0
	#define TOPAZ_MMAP_ALIAS	0
	#define TOPAZ_RX_ACCELERATE	1
#else
	#define TOPAZ_MMAP_UNIFIED	0
	#define TOPAZ_MMAP_ALIAS	0
	#define TOPAZ_RX_ACCELERATE	0
#endif

/* If MU-MIMO done in HDP or SDP */
#ifdef TOPAZ_PLATFORM
	#define QTN_HDP_MU		1
#else
	#define QTN_HDP_MU		0
#endif

#if QTN_HDP_MU
#define QTN_HDP_MU_FCS_WORKROUND	1
#else
#define QTN_HDP_MU_FCS_WORKROUND	0
#endif

#if TOPAZ_MMAP_ALIAS && !TOPAZ_MMAP_UNIFIED
	#error Alias map requires unified map
#endif

#if TOPAZ_MMAP_ALIAS
	#define TOPAZ_ALIAS_MAP_SWITCH(a, b)	(b)
#else
	#define TOPAZ_ALIAS_MAP_SWITCH(a, b)	(a)
#endif

/* Topaz fixed phy addresses */
#define TOPAZ_FPGAA_PHY0_ADDR		2
#define TOPAZ_FPGAA_PHY1_ADDR		3
#define TOPAZ_FPGAB_PHY0_ADDR		4
#define TOPAZ_FPGAB_PHY1_ADDR		1
#define TOPAZ_PHY0_ADDR				1
#define TOPAZ_PHY1_ADDR				3

#ifndef TOPAZ_FPGA_PLATFORM
	#define TOPAZ_FPGA_PLATFORM	0
#endif

#ifndef TOPAZ_VNET_WR_STAGING
	#define TOPAZ_VNET_WR_STAGING	0
#endif
#define TOPAZ_VNET_WR_DMA_CHANNELS			2
#define TOPAZ_VNET_WR_STAGING_BUF_COUNT_PER_CHAIN	10
#define TOPAZ_VNET_WR_STAGING_BUF_SIZE			0x600
#if TOPAZ_VNET_WR_STAGING
	#define TOPAZ_VNET_WR_STAGING_ALIGN			0x80
	#define TOPAZ_VNET_WR_STAGING_GAP			TOPAZ_VNET_WR_STAGING_ALIGN
	#define TOPAZ_VNET_WR_STAGING_RESERVE			((TOPAZ_VNET_WR_STAGING_BUF_COUNT_PER_CHAIN * \
								  TOPAZ_VNET_WR_STAGING_BUF_SIZE) + \
								 TOPAZ_VNET_WR_STAGING_GAP)
#else
	#define TOPAZ_VNET_WR_STAGING_RESERVE			0
#endif

#ifdef TOPAZ_PLATFORM
	/* Definition indicates that Topaz platform is FPGA */
	#if TOPAZ_FPGA_PLATFORM
		/* CLK speeds are in MHz and 1/10th the speed of actual ASIC */
		#define TOPAZ_SERIAL_BAUD	38400
		#define TOPAZ_APB_CLK		12500000
		#define TOPAZ_AHB_CLK		25000000
		#define TOPAZ_CPU_CLK		50000000
		#define RUBY_FPGA_DDR
	#else
		#define TOPAZ_SERIAL_BAUD	115200
		#define TOPAZ_APB_CLK		125000000
		#define TOPAZ_AHB_CLK		250000000
		#define TOPAZ_CPU_CLK		500000000
		#define RUBY_ASIC_DDR
	#endif /* #if TOPAZ_FPGA_PLATFORM */

	/*
	 * Setting UPF_SPD_FLAG gives a developer the option to set the
	 * flag to match a UPF_ define from <linux>/include/linux/serial_core.h
	 * or set the value to 0 to use the default baud rate setting DEFAULT_BAUD
	 */
	#define UPF_SPD_FLAG	0
	#define DEFAULT_BAUD	TOPAZ_SERIAL_BAUD

	/*
	 * Re-use Ruby defines to simplify the number of changes required
	 * to compile new binaries for Topaz
	 */
	#define RUBY_SERIAL_BAUD	TOPAZ_SERIAL_BAUD
	#define RUBY_FIXED_DEV_CLK	TOPAZ_APB_CLK
	#define RUBY_FIXED_CPU_CLK	TOPAZ_CPU_CLK

	#ifdef PLATFORM_DEFAULT_BOARD_ID
	        #define DEFAULT_BOARD_ID	PLATFORM_DEFAULT_BOARD_ID
	#else
		/* Default board id used to match Topaz setting if there is no SPI Flash */
		#define DEFAULT_BOARD_ID	QTN_TOPAZ_BB_BOARD
	#endif /* TOPAZ_DEFAULT_BOARD_ID */

	#ifndef PLATFORM_ARC7_MMU_VER
		#define PLATFORM_ARC7_MMU_VER	2
	#endif

	#define CONFIG_RUBY_BROKEN_IPC_IRQS	0

	#define RUBY_IPC_HI_IRQ(bit_num)	((bit_num) + 8)
	#define RUBY_M2L_IPC_HI_IRQ(bit_num)	(bit_num)

	#define PLATFORM_REG_SWITCH(reg1, reg2)	(reg2)

	#define writel_topaz(a, b)		writel(a, b)
	#define writel_ruby(a, b)

	#define QTN_VLAN_LLC_ENCAP		1

	#define TOPAZ_128_NODE_MODE		1

	#define TOPAZ_ETH_REFLECT_SW_FWD	0

	#define DSP_ENABLE_STATS		1
#else
	#ifndef PLATFORM_ARC7_MMU_VER
		#define PLATFORM_ARC7_MMU_VER	2
	#endif

	/*
	 * For BBIC3.
	 * Workaround for IPC interrupt hw flaw. When receiver dynamically masks/unmasks
	 * interrupt, the transmitter cannot distinguish whether interrupt was acked or just masked.
	 */
	#define CONFIG_RUBY_BROKEN_IPC_IRQS	1

	#define RUBY_IPC_HI_IRQ(bit_num)	((bit_num) + 16)
	#define RUBY_M2L_IPC_HI_IRQ(bit_num)	((bit_num) + 16)

	#define PLATFORM_REG_SWITCH(reg1, reg2)	(reg1)

	#define writel_topaz(a, b)
	#define writel_ruby(a, b)		writel(a, b)

	#define QTN_VLAN_LLC_ENCAP		0
#endif /* #ifdef TOPAZ_PLATFORM */

#endif /* #ifndef __TOPAZ_CONFIG_H */

