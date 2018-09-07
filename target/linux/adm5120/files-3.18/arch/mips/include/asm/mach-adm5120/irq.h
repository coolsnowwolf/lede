/*
 *  ADM5120 specific IRQ numbers
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */
#ifndef _ASM_MIPS_MACH_ADM5120_IRQ_H
#define _ASM_MIPS_MACH_ADM5120_IRQ_H

#define MIPS_CPU_IRQ_BASE	0
#define NR_IRQS			24

#include_next <irq.h>

#include <asm/mach-adm5120/adm5120_intc.h>

#define NO_IRQ			(-1)

#define MIPS_CPU_IRQ_COUNT	8
#define MIPS_CPU_IRQ(x)		(MIPS_CPU_IRQ_BASE + (x))

#define ADM5120_INTC_IRQ_BASE	(MIPS_CPU_IRQ_BASE + MIPS_CPU_IRQ_COUNT)
#define ADM5120_INTC_IRQ(x)	(ADM5120_INTC_IRQ_BASE + (x))

#define ADM5120_IRQ_INTC	MIPS_CPU_IRQ(2)
#define ADM5120_IRQ_COUNTER	MIPS_CPU_IRQ(7)

#define ADM5120_IRQ_TIMER	ADM5120_INTC_IRQ(INTC_IRQ_TIMER)
#define ADM5120_IRQ_UART0	ADM5120_INTC_IRQ(INTC_IRQ_UART0)
#define ADM5120_IRQ_UART1	ADM5120_INTC_IRQ(INTC_IRQ_UART1)
#define ADM5120_IRQ_USBC	ADM5120_INTC_IRQ(INTC_IRQ_USBC)
#define ADM5120_IRQ_GPIO2	ADM5120_INTC_IRQ(INTC_IRQ_GPIO2)
#define ADM5120_IRQ_GPIO4	ADM5120_INTC_IRQ(INTC_IRQ_GPIO4)
#define ADM5120_IRQ_PCI0	ADM5120_INTC_IRQ(INTC_IRQ_PCI0)
#define ADM5120_IRQ_PCI1	ADM5120_INTC_IRQ(INTC_IRQ_PCI1)
#define ADM5120_IRQ_PCI2	ADM5120_INTC_IRQ(INTC_IRQ_PCI2)
#define ADM5120_IRQ_SWITCH	ADM5120_INTC_IRQ(INTC_IRQ_SWITCH)

#endif /* _ASM_MIPS_MACH_ADM5120_IRQ_H */
