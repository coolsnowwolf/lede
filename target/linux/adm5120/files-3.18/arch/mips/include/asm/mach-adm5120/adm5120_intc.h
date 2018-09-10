/*
 *  ADM5120 interrupt controller definitions
 *
 *  This header file defines the hardware registers of the ADM5120 SoC
 *  built-in interrupt controller.
 *
 *  Copyright (C) 2007-2008 Gabor Juhos <juhosg@openwrt.org>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 *
 */

#ifndef _MACH_ADM5120_INTC_H
#define _MACH_ADM5120_INTC_H

/*
 * INTC register offsets
 */
#define INTC_REG_IRQ_STATUS		0x00 /* Interrupt status after masking */
#define INTC_REG_IRQ_RAW_STATUS		0x04 /* Interrupt status before masking */
#define INTC_REG_IRQ_ENABLE		0x08 /* Used to enable the interrupt sources */
#define INTC_REG_IRQ_ENABLE_CLEAR	0x0C /* Used to disable the interrupt sources */
#define INTC_REG_IRQ_DISABLE		INTC_REG_IRQ_ENABLE_CLEAR
#define INTC_REG_INT_MODE		0x14 /* The interrupt mode of the sources */
#define INTC_REG_FIQ_STATUS		0x18 /* FIQ status */
#define INTC_REG_IRQ_TEST_SOURCE	0x1C
#define INTC_REG_IRQ_SOURCE_SELECT	0x20
#define INTC_REG_INT_LEVEL		0x24

/*
 * INTC IRQ numbers
 */
#define INTC_IRQ_TIMER	0	/* built in timer */
#define INTC_IRQ_UART0	1	/* built-in UART0 */
#define INTC_IRQ_UART1	2	/* built-in UART1 */
#define INTC_IRQ_USBC	3	/* USB Host Controller */
#define INTC_IRQ_GPIO2	4	/* GPIO line 2 */
#define INTC_IRQ_GPIO4	5	/* GPIO line 4 */
#define INTC_IRQ_PCI0	6	/* PCI slot 2 */
#define INTC_IRQ_PCI1	7	/* PCI slot 3 */
#define INTC_IRQ_PCI2	8	/* PCI slot 4 */
#define INTC_IRQ_SWITCH	9	/* built-in ethernet switch */
#define INTC_IRQ_LAST	INTC_IRQ_SWITCH
#define INTC_IRQ_COUNT	10

/*
 * INTC register bits
 */
#define INTC_INT_TIMER	(1 << INTC_IRQ_TIMER)
#define INTC_INT_UART0	(1 << INTC_IRQ_UART0)
#define INTC_INT_UART1	(1 << INTC_IRQ_UART1)
#define INTC_INT_USBC	(1 << INTC_IRQ_USBC)
#define INTC_INT_INTX0	(1 << INTC_IRQ_INTX0)
#define INTC_INT_INTX1	(1 << INTC_IRQ_INTX1)
#define INTC_INT_PCI0	(1 << INTC_IRQ_PCI0)
#define INTC_INT_PCI1	(1 << INTC_IRQ_PCI1)
#define INTC_INT_PCI2	(1 << INTC_IRQ_PCI2)
#define INTC_INT_SWITCH	(1 << INTC_IRQ_SWITCH)
#define INTC_INT_ALL	((1 << INTC_IRQ_COUNT) - 1)

#endif /* _MACH_ADM5120_INTC_H */
