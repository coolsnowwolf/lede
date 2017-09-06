/*
 *  linux/arch/arm/mach-cns3xxx/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  Copyright 2012 Gateworks Corporation
 *		   Chris Lang <clang@gateworks.com>
 *         Tim Harvey <tharvey@gateworks.com>
 *
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>

#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <asm/unified.h>
#include <asm/fiq.h>
#include <mach/smp.h>
#include "cns3xxx.h"

static struct fiq_handler fh = {
	.name = "cns3xxx-fiq"
};

struct fiq_req {
	union {
		struct {
			const void *addr;
			size_t size;
		} map;
		struct {
			const void *addr;
			size_t size;
		} unmap;
		struct {
			const void *start;
			const void *end;
		} flush;
	};
	volatile uint flags;
	void __iomem *reg;
} ____cacheline_aligned;

extern unsigned int fiq_number[2];

DEFINE_PER_CPU(struct fiq_req, fiq_data);

#define FIQ_ENABLED         0x80000000
#define FIQ_GENERATE        0x00010000
#define CNS3XXX_MAP_AREA    0x01000000
#define CNS3XXX_UNMAP_AREA  0x02000000
#define CNS3XXX_FLUSH_RANGE 0x03000000

extern void cns3xxx_secondary_startup(void);
extern unsigned char cns3xxx_fiq_start, cns3xxx_fiq_end;

#define SCU_CPU_STATUS 0x08
static void __iomem *scu_base;

static inline void cns3xxx_set_fiq_regs(unsigned int cpu)
{
	struct pt_regs FIQ_regs;
	struct fiq_req *fiq_req = &per_cpu(fiq_data, !cpu);

	FIQ_regs.ARM_r8 = 0;
	FIQ_regs.ARM_ip = (unsigned int)fiq_req;
	FIQ_regs.ARM_sp = (int) MISC_FIQ_CPU(!cpu);
	fiq_req->reg = MISC_FIQ_CPU(!cpu);

	set_fiq_regs(&FIQ_regs);
}

static void __init cns3xxx_init_fiq(void)
{
	void *fiqhandler_start;
	unsigned int fiqhandler_length;
	int ret;

	fiqhandler_start = &cns3xxx_fiq_start;
	fiqhandler_length = &cns3xxx_fiq_end - &cns3xxx_fiq_start;

	ret = claim_fiq(&fh);
	if (ret)
		return;

	set_fiq_handler(fiqhandler_start, fiqhandler_length);
}


/*
 * Write pen_release in a way that is guaranteed to be visible to all
 * observers, irrespective of whether they're taking part in coherency
 * or not.  This is necessary for the hotplug code to work reliably.
 */
static void write_pen_release(int val)
{
	pen_release = val;
	smp_wmb();
	__cpuc_flush_dcache_area((void *)&pen_release, sizeof(pen_release));
	outer_clean_range(__pa(&pen_release), __pa(&pen_release + 1));
}

static DEFINE_SPINLOCK(boot_lock);

static void cns3xxx_secondary_init(unsigned int cpu)
{
	/*
	 * Setup Secondary Core FIQ regs
	 */
	cns3xxx_set_fiq_regs(1);

	/*
	 * let the primary processor know we're out of the
	 * pen, then head off into the C entry point
	 */
	write_pen_release(-1);

	/*
	 * Synchronise with the boot thread.
	 */
	spin_lock(&boot_lock);
	spin_unlock(&boot_lock);
}

static int cns3xxx_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * The secondary processor is waiting to be released from
	 * the holding pen - release it, then wait for it to flag
	 * that it has been released by resetting pen_release.
	 *
	 * Note that "pen_release" is the hardware CPU ID, whereas
	 * "cpu" is Linux's internal ID.
	 */
	write_pen_release(cpu);

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */
	arch_send_wakeup_ipi_mask(cpumask_of(cpu));;

	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (pen_release == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return pen_release != -1 ? -ENOSYS : 0;
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init cns3xxx_smp_init_cpus(void)
{
	unsigned int i, ncores;
	unsigned int status;

	scu_base = (void __iomem *) CNS3XXX_TC11MP_SCU_BASE_VIRT;

	/* for CNS3xxx SCU_CPU_STATUS must be examined instead of SCU_CONFIGURATION
	 * used in scu_get_core_count
	 */
	status = __raw_readl(scu_base + SCU_CPU_STATUS);
	for (i = 0; i < NR_CPUS+1; i++) {
		if (((status >> (i*2)) & 0x3) == 0)
			set_cpu_possible(i, true);
		else
			break;
	}
	ncores = i;
}

static void __init cns3xxx_smp_prepare_cpus(unsigned int max_cpus)
{
	/*
	 * enable SCU
	 */
	scu_enable(scu_base);

	/*
	 * Write the address of secondary startup into the
	 * system-wide flags register. The boot monitor waits
	 * until it receives a soft interrupt, and then the
	 * secondary CPU branches to this address.
	 */
	__raw_writel(virt_to_phys(cns3xxx_secondary_startup),
			(void __iomem *)(CNS3XXX_MISC_BASE_VIRT + 0x0600));

	/*
	 * Setup FIQ's for main cpu
	 */
	cns3xxx_init_fiq();
	cns3xxx_set_fiq_regs(0);
}

extern void v6_dma_map_area(const void *, size_t, int);
extern void v6_dma_unmap_area(const void *, size_t, int);
extern void v6_dma_flush_range(const void *, const void *);
extern void v6_flush_kern_dcache_area(void *, size_t);

void fiq_dma_map_area(const void *addr, size_t size, int dir)
{
	unsigned long flags;
	struct fiq_req *req;

	raw_local_irq_save(flags);
	/* currently, not possible to take cpu0 down, so only check cpu1 */
	if (!cpu_online(1)) {
		raw_local_irq_restore(flags);
		v6_dma_map_area(addr, size, dir);
		return;
	}

	req = this_cpu_ptr(&fiq_data);
	req->map.addr = addr;
	req->map.size = size;
	req->flags = dir | CNS3XXX_MAP_AREA;
	smp_mb();

	writel_relaxed(FIQ_GENERATE, req->reg);

	v6_dma_map_area(addr, size, dir);
	while (req->flags)
		barrier();

	raw_local_irq_restore(flags);
}

void fiq_dma_unmap_area(const void *addr, size_t size, int dir)
{
	unsigned long flags;
	struct fiq_req *req;

	raw_local_irq_save(flags);
	/* currently, not possible to take cpu0 down, so only check cpu1 */
	if (!cpu_online(1)) {
		raw_local_irq_restore(flags);
		v6_dma_unmap_area(addr, size, dir);
		return;
	}

	req = this_cpu_ptr(&fiq_data);
	req->unmap.addr = addr;
	req->unmap.size = size;
	req->flags = dir | CNS3XXX_UNMAP_AREA;
	smp_mb();

	writel_relaxed(FIQ_GENERATE, req->reg);

	v6_dma_unmap_area(addr, size, dir);
	while (req->flags)
		barrier();

	raw_local_irq_restore(flags);
}

void fiq_dma_flush_range(const void *start, const void *end)
{
	unsigned long flags;
	struct fiq_req *req;

	raw_local_irq_save(flags);
	/* currently, not possible to take cpu0 down, so only check cpu1 */
	if (!cpu_online(1)) {
		raw_local_irq_restore(flags);
		v6_dma_flush_range(start, end);
		return;
	}

	req = this_cpu_ptr(&fiq_data);

	req->flush.start = start;
	req->flush.end = end;
	req->flags = CNS3XXX_FLUSH_RANGE;
	smp_mb();

	writel_relaxed(FIQ_GENERATE, req->reg);

	v6_dma_flush_range(start, end);

	while (req->flags)
		barrier();

	raw_local_irq_restore(flags);
}

void fiq_flush_kern_dcache_area(void *addr, size_t size)
{
	fiq_dma_flush_range(addr, addr + size);
}

struct smp_operations cns3xxx_smp_ops __initdata = {
	.smp_init_cpus      = cns3xxx_smp_init_cpus,
	.smp_prepare_cpus   = cns3xxx_smp_prepare_cpus,
	.smp_secondary_init = cns3xxx_secondary_init,
	.smp_boot_secondary = cns3xxx_boot_secondary,
};
