/*
 * arch/arm/mach-cns3xxx/include/mach/gpio.h
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 *
 */
#ifndef __ASM_ARCH_CNS3XXX_GPIO_H
#define __ASM_ARCH_CNS3XXX_GPIO_H

#include <linux/kernel.h>

extern void __init cns3xxx_gpio_init(int gpio_base, int ngpio,
	u32 base, int irq, int secondary_irq_base);

#endif
