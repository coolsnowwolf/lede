/*
 * smp.h
 *
 *  Created on: Sep 24, 2013
 *      Author: mahaijun
 */

#ifndef _NAS782X_SMP_H_
#define _NAS782X_SMP_H_

#include <mach/hardware.h>

extern void ox820_secondary_startup(void);
extern void ox820_cpu_die(unsigned int cpu);

static inline void write_pen_release(int val)
{
	writel(val, HOLDINGPEN_CPU);
}

static inline int read_pen_release(void)
{
	return readl(HOLDINGPEN_CPU);
}

extern struct smp_operations ox820_smp_ops;

extern unsigned char ox820_fiq_start, ox820_fiq_end;
extern void v6_dma_map_area(const void *, size_t, int);
extern void v6_dma_unmap_area(const void *, size_t, int);
extern void v6_dma_flush_range(const void *, const void *);
extern void v6_flush_kern_dcache_area(void *, size_t);

#endif /* _NAS782X_SMP_H_ */
