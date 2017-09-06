/*
 *  arch/arm/mach-ox820/platsmp.c
 *
 *  Copyright (C) 2002 ARM Ltd.
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/init.h>
#include <linux/device.h>
#include <linux/jiffies.h>
#include <linux/smp.h>
#include <linux/io.h>
#include <linux/dma-mapping.h>
#include <linux/cache.h>
#include <asm/cacheflush.h>
#include <asm/smp_scu.h>
#include <asm/tlbflush.h>
#include <asm/cputype.h>
#include <linux/delay.h>
#include <asm/fiq.h>

#include <linux/irqchip/arm-gic.h>
#include <mach/iomap.h>
#include <mach/smp.h>
#include <mach/hardware.h>
#include <mach/irqs.h>

#ifdef CONFIG_DMA_CACHE_FIQ_BROADCAST

#define FIQ_GENERATE		0x00000002
#define OXNAS_MAP_AREA		0x01000000
#define OXNAS_UNMAP_AREA	0x02000000
#define OXNAS_FLUSH_RANGE	0x03000000

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

static struct fiq_handler fh = {
	.name = "oxnas-fiq"
};

DEFINE_PER_CPU(struct fiq_req, fiq_data);

static inline void ox820_set_fiq_regs(unsigned int cpu)
{
	struct pt_regs FIQ_regs;
	struct fiq_req *fiq_req = &per_cpu(fiq_data, !cpu);

	FIQ_regs.ARM_r8 = 0;
	FIQ_regs.ARM_ip = (unsigned int)fiq_req;
	FIQ_regs.ARM_sp = (int)(cpu ? RPSC_IRQ_SOFT : RPSA_IRQ_SOFT);
	fiq_req->reg = cpu ? RPSC_IRQ_SOFT : RPSA_IRQ_SOFT;

	set_fiq_regs(&FIQ_regs);
}

static void __init ox820_init_fiq(void)
{
	void *fiqhandler_start;
	unsigned int fiqhandler_length;
	int ret;

	fiqhandler_start = &ox820_fiq_start;
	fiqhandler_length = &ox820_fiq_end - &ox820_fiq_start;

	ret = claim_fiq(&fh);

	if (ret)
		return;

	set_fiq_handler(fiqhandler_start, fiqhandler_length);

	writel(IRQ_SOFT, RPSA_FIQ_IRQ_TO_FIQ);
	writel(1, RPSA_FIQ_ENABLE);
	writel(IRQ_SOFT, RPSC_FIQ_IRQ_TO_FIQ);
	writel(1, RPSC_FIQ_ENABLE);
}

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
	req->flags = dir | OXNAS_MAP_AREA;
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
	req->flags = dir | OXNAS_UNMAP_AREA;
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
	req->flags = OXNAS_FLUSH_RANGE;
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
#else

#define ox820_set_fiq_regs(cpu)	do {} while (0) /* nothing */
#define ox820_init_fiq()	do {} while (0) /* nothing */

#endif /* DMA_CACHE_FIQ_BROADCAST */

static DEFINE_SPINLOCK(boot_lock);

void ox820_secondary_init(unsigned int cpu)
{
	/*
	 * Setup Secondary Core FIQ regs
	 */
	ox820_set_fiq_regs(1);

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

int ox820_boot_secondary(unsigned int cpu, struct task_struct *idle)
{
	unsigned long timeout;

	/*
	 * Set synchronisation state between this boot processor
	 * and the secondary one
	 */
	spin_lock(&boot_lock);

	/*
	 * This is really belt and braces; we hold unintended secondary
	 * CPUs in the holding pen until we're ready for them.  However,
	 * since we haven't sent them a soft interrupt, they shouldn't
	 * be there.
	 */
	write_pen_release(cpu);

	writel(1, IOMEM(OXNAS_GICN_BASE_VA(cpu) + GIC_CPU_CTRL));

	/*
	 * Send the secondary CPU a soft interrupt, thereby causing
	 * the boot monitor to read the system wide flags register,
	 * and branch to the address found there.
	 */

	arch_send_wakeup_ipi_mask(cpumask_of(cpu));
	timeout = jiffies + (1 * HZ);
	while (time_before(jiffies, timeout)) {
		smp_rmb();
		if (read_pen_release() == -1)
			break;

		udelay(10);
	}

	/*
	 * now the secondary core is starting up let it run its
	 * calibrations, then wait for it to finish
	 */
	spin_unlock(&boot_lock);

	return read_pen_release() != -1 ? -ENOSYS : 0;
}

void *scu_base_addr(void)
{
	return IOMEM(OXNAS_SCU_BASE_VA);
}

/*
 * Initialise the CPU possible map early - this describes the CPUs
 * which may be present or become present in the system.
 */
static void __init ox820_smp_init_cpus(void)
{
	void __iomem *scu_base = scu_base_addr();
	unsigned int i, ncores;

	ncores = scu_base ? scu_get_core_count(scu_base) : 1;

	/* sanity check */
	if (ncores > nr_cpu_ids) {
		pr_warn("SMP: %u cores greater than maximum (%u), clipping\n",
			ncores, nr_cpu_ids);
		ncores = nr_cpu_ids;
	}

	for (i = 0; i < ncores; i++)
		set_cpu_possible(i, true);
}

static void __init ox820_smp_prepare_cpus(unsigned int max_cpus)
{

	scu_enable(scu_base_addr());

	/*
	 * Write the address of secondary startup into the
	 * system-wide flags register. The BootMonitor waits
	 * until it receives a soft interrupt, and then the
	 * secondary CPU branches to this address.
	 */
	writel(virt_to_phys(ox820_secondary_startup),
					HOLDINGPEN_LOCATION);
	ox820_init_fiq();

	ox820_set_fiq_regs(0);
}

struct smp_operations ox820_smp_ops __initdata = {
	.smp_init_cpus		= ox820_smp_init_cpus,
	.smp_prepare_cpus	= ox820_smp_prepare_cpus,
	.smp_secondary_init	= ox820_secondary_init,
	.smp_boot_secondary	= ox820_boot_secondary,
#ifdef CONFIG_HOTPLUG_CPU
	.cpu_die		= ox820_cpu_die,
#endif
};
