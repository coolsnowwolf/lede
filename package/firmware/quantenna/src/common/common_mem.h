/*
 * (C) Copyright 2014 Quantenna Communications Inc.
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
 * Header file which describes the Ruby and Topaz platforms.
 * Used by both run-time and boot loader images.
 *
 * Do not put run-time specific definitions in this file.
 */

#ifndef __COMMON_MEM_H
#define __COMMON_MEM_H

#include "ruby_config.h"

/* Platform memory */
/* SRAM */
#define RUBY_SRAM_UNIFIED_BEGIN			0x98000000
#define RUBY_SRAM_UNIFIED_NOCACHE_BEGIN		0xf8000000
#define RUBY_SRAM_FLIP_BEGIN			0x88000000
#define RUBY_SRAM_FLIP_NOCACHE_BEGIN		0x60000000
#define RUBY_SRAM_NOFLIP_BEGIN			0x80000000
#define RUBY_SRAM_NOFLIP_NOCACHE_BEGIN		0x60000000
#define RUBY_SRAM_BANK_SIZE			(64 * 1024)

#ifdef TOPAZ_PLATFORM
	#define RUBY_SRAM_SIZE			(8 * RUBY_SRAM_BANK_SIZE)
	#define RUBY_SRAM_BANK_SAFE_SIZE	RUBY_SRAM_BANK_SIZE
#else
	#define RUBY_SRAM_END_BANK_GUARD_SIZE	32
	#define RUBY_SRAM_SIZE			(4 * RUBY_SRAM_BANK_SIZE)
	#define RUBY_SRAM_BANK_SAFE_SIZE	(RUBY_SRAM_BANK_SIZE - RUBY_SRAM_END_BANK_GUARD_SIZE)
#endif

/* DDR */
#define RUBY_DRAM_UNIFIED_BEGIN			0x80000000
#define RUBY_DRAM_UNIFIED_NOCACHE_BEGIN		0xd0000000
#define RUBY_DRAM_FLIP_BEGIN			0x80000000
#define RUBY_DRAM_FLIP_NOCACHE_BEGIN		0x40000000
#define RUBY_DRAM_NOFLIP_BEGIN			0x0
#define RUBY_DRAM_NOFLIP_NOCACHE_BEGIN		0x40000000
#define RUBY_MAX_DRAM_SIZE			DDR_128MB
#define RUBY_MIN_DRAM_SIZE			DDR_64MB

#if TOPAZ_MMAP_UNIFIED
	#define RUBY_SRAM_BEGIN			RUBY_SRAM_UNIFIED_BEGIN
	#define RUBY_SRAM_BUS_BEGIN		RUBY_SRAM_UNIFIED_BEGIN
	#define RUBY_SRAM_NOCACHE_BEGIN		RUBY_SRAM_UNIFIED_NOCACHE_BEGIN
	#define RUBY_DRAM_BEGIN			RUBY_DRAM_UNIFIED_BEGIN
	#define RUBY_DRAM_BUS_BEGIN		RUBY_DRAM_UNIFIED_BEGIN
	#define RUBY_DRAM_NOCACHE_BEGIN		RUBY_DRAM_UNIFIED_NOCACHE_BEGIN
#elif RUBY_MMAP_FLIP
	#define RUBY_SRAM_BEGIN			RUBY_SRAM_FLIP_BEGIN
	#define RUBY_SRAM_BUS_BEGIN		RUBY_SRAM_NOFLIP_BEGIN
	#define RUBY_SRAM_NOCACHE_BEGIN		RUBY_SRAM_FLIP_NOCACHE_BEGIN
	#define RUBY_DRAM_BEGIN			RUBY_DRAM_FLIP_BEGIN
	#define RUBY_DRAM_BUS_BEGIN		RUBY_DRAM_NOFLIP_BEGIN
	#define RUBY_DRAM_NOCACHE_BEGIN		RUBY_DRAM_FLIP_NOCACHE_BEGIN
#else
	#define RUBY_SRAM_BEGIN			RUBY_SRAM_NOFLIP_BEGIN
	#define RUBY_SRAM_BUS_BEGIN		RUBY_SRAM_NOFLIP_BEGIN
	#define RUBY_SRAM_NOCACHE_BEGIN		RUBY_SRAM_NOFLIP_NOCACHE_BEGIN
	#define RUBY_DRAM_BEGIN			RUBY_DRAM_NOFLIP_BEGIN
	#define RUBY_DRAM_BUS_BEGIN		RUBY_DRAM_NOFLIP_BEGIN
	#define RUBY_DRAM_NOCACHE_BEGIN		RUBY_DRAM_NOFLIP_NOCACHE_BEGIN
#endif

/*****************************************************************************/
/* SPI memory mapped                                                         */
/*****************************************************************************/
#define RUBY_SPI_FLASH_ADDR     0x90000000

 /* Hardware */
#define RUBY_HARDWARE_BEGIN	0xC0000000

#define	ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))

/* Config space */
#define CONFIG_ARC_CONF_SIZE		(8 * 1024)
/* Config area for Universal H/W ID */
#define CONFIG_ARC_CONF_BASE		(0x80000000 + CONFIG_ARC_CONF_SIZE)

#define CONFIG_ARC_KERNEL_PAGE_SIZE	(8 * 1024)

#define RUBY_KERNEL_LOAD_DRAM_BEGIN	(RUBY_DRAM_BEGIN + 0x3000000)

/* DDR layout */
#define CONFIG_ARC_NULL_BASE		0x00000000
#define CONFIG_ARC_NULL_SIZE		(64 * 1024)
#define CONFIG_ARC_NULL_END		(CONFIG_ARC_NULL_BASE + CONFIG_ARC_NULL_SIZE)

/* PCIe BDA area */
#define CONFIG_ARC_PCIE_BASE		(RUBY_DRAM_BEGIN + CONFIG_ARC_NULL_END)
#define CONFIG_ARC_PCIE_SIZE		(64 * 1024) /* minimal PCI BAR size */
#if ((CONFIG_ARC_PCIE_BASE & (64 * 1024 - 1)) != 0)
	#error "The reserved region for PCIe BAR should 64k aligned!"
#endif

/*
 * CONFIG_ARC_MUC_STACK_OFFSET_UBOOT must be equal to CONFIG_ARC_MUC_STACK_OFFSET
 * and RUBY_CRUMBS_OFFSET_UBOOT must be equal to RUBY_CRUMBS_OFFSET.
 * Their values can be obtained with host/utilities/ruby_mem_helper.
 */
#if TOPAZ_RX_ACCELERATE
	/* Must be equal to CONFIG_ARC_MUC_STACK_OFFSET */
	#define CONFIG_ARC_MUC_STACK_OFFSET_UBOOT	(0x0003F7C0)
	/* MuC stack, included in CONFIG_ARC_MUC_SRAM_SIZE */
	#define CONFIG_ARC_MUC_STACK_SIZE		(4 * 1024)
#else
	/* Must be equal to CONFIG_ARC_MUC_STACK_OFFSET */
	#define CONFIG_ARC_MUC_STACK_OFFSET_UBOOT	(0x0003FFA0)
	/* MuC stack, included in CONFIG_ARC_MUC_SRAM_SIZE */
	#define CONFIG_ARC_MUC_STACK_SIZE		(6 * 1024)
#endif

#define CONFIG_ARC_MUC_STACK_INIT_UBOOT		(RUBY_SRAM_BEGIN + CONFIG_ARC_MUC_STACK_OFFSET_UBOOT)

#ifdef TOPAZ_PLATFORM
	/* Must be equal to RUBY_CRUMBS_OFFSET */
	#define RUBY_CRUMBS_OFFSET_UBOOT	(0x0003FFC0)
#else
	#define RUBY_CRUMBS_OFFSET_UBOOT	(0x0003FFA0)
#endif

#define RUBY_CRUMBS_ADDR_UBOOT			(RUBY_SRAM_BEGIN + RUBY_CRUMBS_OFFSET_UBOOT)

/*
 * Crumb structure, sits at the end of SRAM. Each core can use it to
 * store the last run function to detect bus hangs.
 */
#ifndef __ASSEMBLY__
	struct ruby_crumbs_percore {
		unsigned long	blink;
		unsigned long	status32;
		unsigned long	sp;
	};

	struct ruby_crumbs_mem_section {
		unsigned long	start;
		unsigned long	end;
	};

	struct ruby_crumbs {
		struct ruby_crumbs_percore	lhost;
		struct ruby_crumbs_percore	muc;
		struct ruby_crumbs_percore	dsp;
		/*
		 * allow (somewhat) intelligent parsing of muc stacks by
		 * specifying the text section
		 */
		struct ruby_crumbs_mem_section	muc_dram;
		struct ruby_crumbs_mem_section	muc_sram;

		/*
		 * magic token; if set incorrectly we probably have
		 * random values after power-on
		 */
		unsigned long			magic;
	};

	#define	RUBY_CRUMBS_MAGIC	0x7c97be8f

#endif /* __ASSEMBLY__ */

/* Utility functions */
#ifndef __ASSEMBLY__
	#if defined(AUC_BUILD) || defined(RUBY_MINI)
		#define NO_RUBY_WEAK	1
	#else
		#define NO_RUBY_WEAK	0
	#endif

	#define RUBY_BAD_BUS_ADDR	((unsigned  long)0)
	#define RUBY_BAD_VIRT_ADDR	((void*)RUBY_BAD_BUS_ADDR)
	#define RUBY_ERROR_ADDR		((unsigned long)0xefefefef)

	#if defined(__CHECKER__)
		#define RUBY_INLINE			static inline __attribute__((always_inline))
		#define RUBY_WEAK(name)			RUBY_INLINE
	#elif defined(__GNUC__)
		/*GCC*/
		#define RUBY_INLINE			static inline __attribute__((always_inline))
		#if NO_RUBY_WEAK
			#define RUBY_WEAK(name)		RUBY_INLINE
		#else
			#define RUBY_WEAK(name)		__attribute__((weak))
		#endif
	#else
		/*MCC*/
		#define RUBY_INLINE			static _Inline
		#if NO_RUBY_WEAK
			#define RUBY_WEAK(name)		RUBY_INLINE
		#else
			#define RUBY_WEAK(name)		pragma Weak(name);
		#endif
		#pragma Offwarn(428)
	#endif

	#define ____in_mem_range(addr, start, size)	\
		(((addr) >= (start)) && ((addr) < (start) + (size)))

	#if defined(STATIC_CHECK) || defined(__CHECKER__)
		RUBY_INLINE int __in_mem_range(unsigned long addr, unsigned long start, unsigned long size)
		{
			return (((addr) >= (start)) && ((addr) < (start) + (size)));
		}
	#else
		#define __in_mem_range ____in_mem_range
	#endif

	#if RUBY_MMAP_FLIP
		RUBY_INLINE unsigned long virt_to_bus(const void *addr)
		{
			unsigned long ret = (unsigned long)addr;
			if (__in_mem_range(ret, RUBY_SRAM_FLIP_BEGIN, RUBY_SRAM_SIZE)) {
				ret = ret - RUBY_SRAM_FLIP_BEGIN + RUBY_SRAM_NOFLIP_BEGIN;
			} else if (__in_mem_range(ret, RUBY_DRAM_FLIP_BEGIN, RUBY_MAX_DRAM_SIZE)) {
				ret = ret - RUBY_DRAM_FLIP_BEGIN + RUBY_DRAM_NOFLIP_BEGIN;
			} else if (ret < RUBY_HARDWARE_BEGIN) {
				ret = RUBY_BAD_BUS_ADDR;
			}
			return ret;
		}
		RUBY_WEAK(bus_to_virt) void* bus_to_virt(unsigned long addr)
		{
			unsigned long ret = addr;
			if (__in_mem_range(ret, RUBY_SRAM_NOFLIP_BEGIN, RUBY_SRAM_SIZE)) {
				ret = ret - RUBY_SRAM_NOFLIP_BEGIN + RUBY_SRAM_FLIP_BEGIN;
			} else if (__in_mem_range(ret, RUBY_DRAM_NOFLIP_BEGIN, RUBY_MAX_DRAM_SIZE)) {
				ret = ret - RUBY_DRAM_NOFLIP_BEGIN + RUBY_DRAM_FLIP_BEGIN;
			} else if (ret < RUBY_HARDWARE_BEGIN) {
				ret = (unsigned long)RUBY_BAD_VIRT_ADDR;
			}
			return (void*)ret;
		}
	#else
		/* Map 1:1, (x) address must be upper then 0x8000_0000. */
		#define virt_to_bus(x) ((unsigned long)(x))
		#define bus_to_virt(x) ((void *)(x))
	#endif /* #if RUBY_MMAP_FLIP */

	#ifndef __GNUC__
		/* MCC */
		#pragma Popwarn()
	#endif

#endif /* #ifndef __ASSEMBLY__ */

#endif /* __COMMON_MEM_H */
