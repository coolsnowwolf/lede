/*
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#include <linux/init.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/io.h>

#include <asm/bootinfo.h>
#include <asm/addrspace.h>

#include <asm/mach-adm5120/adm5120_info.h>
#include <asm/mach-adm5120/adm5120_defs.h>
#include <asm/mach-adm5120/adm5120_switch.h>
#include <asm/mach-adm5120/adm5120_mpmc.h>

#ifdef DEBUG
#  define mem_dbg(f, a...)	printk(KERN_INFO "mem_detect: " f, ## a)
#else
#  define mem_dbg(f, a...)
#endif

unsigned long adm5120_memsize;

#define MEM_READL(a)		__raw_readl((void __iomem *)(a))
#define MEM_WRITEL(a, v)	__raw_writel((v), (void __iomem *)(a))

static int __init mem_check_pattern(u8 *addr, unsigned long offs)
{
	u32 *p1 = (u32 *)addr;
	u32 *p2 = (u32 *)(addr+offs);
	u32 t, u, v;

	/* save original value */
	t = MEM_READL(p1);

	u = MEM_READL(p2);
	if (t != u)
		return 0;

	v = 0x55555555;
	if (u == v)
		v = 0xAAAAAAAA;

	mem_dbg("write 0x%08X to 0x%08lX\n", v, (unsigned long)p1);

	MEM_WRITEL(p1, v);
	adm5120_ndelay(1000);
	u = MEM_READL(p2);

	mem_dbg("pattern at 0x%08lX is 0x%08X\n", (unsigned long)p2, u);

	/* restore original value */
	MEM_WRITEL(p1, t);

	return (v == u);
}

static void __init adm5120_detect_memsize(void)
{
	u32	memctrl;
	u32	size, maxsize;
	u8	*p;

	memctrl = SW_READ_REG(SWITCH_REG_MEMCTRL);
	switch (memctrl & MEMCTRL_SDRS_MASK) {
	case MEMCTRL_SDRS_4M:
		maxsize = 4 << 20;
		break;
	case MEMCTRL_SDRS_8M:
		maxsize = 8 << 20;
		break;
	case MEMCTRL_SDRS_16M:
		maxsize = 16 << 20;
		break;
	default:
		maxsize = 64 << 20;
		break;
	}

	mem_dbg("checking for %uMB chip in 1st bank\n", maxsize >> 20);

	/* detect size of the 1st SDRAM bank */
	p = (u8 *)KSEG1ADDR(0);
	for (size = 2<<20; size <= (maxsize >> 1); size <<= 1) {
		if (mem_check_pattern(p, size)) {
			/* mirrored address */
			mem_dbg("mirrored data found at offset 0x%08X\n", size);
			break;
		}
	}

	mem_dbg("chip size in 1st bank is %uMB\n", size >> 20);
	adm5120_memsize = size;

	if (size != maxsize)
		/* 2nd bank is not supported */
		goto out;

	if ((memctrl & MEMCTRL_SDR1_ENABLE) == 0)
		/* 2nd bank is disabled */
		goto out;

	/*
	 * some bootloaders enable 2nd bank, even if the 2nd SDRAM chip
	 * are missing.
	 */
	mem_dbg("check presence of 2nd bank\n");

	p = (u8 *)KSEG1ADDR(maxsize+size-4);
	if (mem_check_pattern(p, 0))
		adm5120_memsize += size;

	if (maxsize != size) {
		/* adjusting MECTRL register */
		memctrl &= ~(MEMCTRL_SDRS_MASK);
		switch (size>>20) {
		case 4:
			memctrl |= MEMCTRL_SDRS_4M;
			break;
		case 8:
			memctrl |= MEMCTRL_SDRS_8M;
			break;
		case 16:
			memctrl |= MEMCTRL_SDRS_16M;
			break;
		default:
			memctrl |= MEMCTRL_SDRS_64M;
			break;
		}
		SW_WRITE_REG(SWITCH_REG_MEMCTRL, memctrl);
	}

out:
	mem_dbg("%dx%uMB memory found\n", (adm5120_memsize == size) ? 1 : 2 ,
		size>>20);
}

void __init adm5120_mem_init(void)
{
	adm5120_detect_memsize();
	add_memory_region(0, adm5120_memsize, BOOT_MEM_RAM);
}
