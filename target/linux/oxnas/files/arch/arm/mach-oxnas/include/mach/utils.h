#ifndef _NAS782X_UTILS_H
#define _NAS782X_UTILS_H

#include <linux/io.h>
#include <mach/hardware.h>

static inline void oxnas_register_clear_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val &= ~mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_set_mask(void __iomem *p, unsigned mask)
{
	u32 val = readl_relaxed(p);

	val |= mask;
	writel_relaxed(val, p);
}

static inline void oxnas_register_value_mask(void __iomem *p,
					     unsigned mask, unsigned new_value)
{
	/* TODO sanity check mask & new_value = new_value */
	u32 val = readl_relaxed(p);

	val &= ~mask;
	val |= new_value;
	writel_relaxed(val, p);
}

#endif /* _NAS782X_UTILS_H */
