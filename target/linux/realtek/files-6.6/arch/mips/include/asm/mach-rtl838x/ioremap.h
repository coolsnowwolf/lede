/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef RTL838X_IOREMAP_H_
#define RTL838X_IOREMAP_H_

static inline int is_rtl838x_internal_registers(phys_addr_t offset)
{
	/* IO-Block */
	if (offset >= 0xb8000000 && offset < 0xb9000000)
		return 1;
	/* Switch block */
	if (offset >= 0xbb000000 && offset < 0xbc000000)
		return 1;
	return 0;
}

static inline void __iomem *plat_ioremap(phys_addr_t offset, unsigned long size,
					 unsigned long flags)
{
	if (is_rtl838x_internal_registers(offset))
		return (void __iomem *)offset;
	return NULL;
}

static inline int plat_iounmap(const volatile void __iomem *addr)
{
	return is_rtl838x_internal_registers((unsigned long)addr);
}

#endif
