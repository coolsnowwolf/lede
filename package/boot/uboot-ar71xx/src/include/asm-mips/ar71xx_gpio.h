/*
 * (C) Copyright 2010
 * Michael Kurz <michi.kurz@googlemail.com>.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _AR71XX_GPIO_H
#define _AR71XX_GPIO_H

#include <common.h>
#include <asm/ar71xx.h>

static inline void ar71xx_setpin(uint8_t pin, uint8_t state)
{
	uint32_t reg = readl(KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OUT));

	if (state != 0) {
       reg |= (1 << pin);
   } else {
       reg &= ~(1 << pin);
   }

	writel(reg, KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OUT));
	readl(KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OUT));
}

static inline uint32_t ar71xx_getpin(uint8_t pin)
{
    uint32_t reg = readl(KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_IN));
    return (((reg & (1 << pin)) != 0) ? 1 : 0);
}

static inline void ar71xx_setpindir(uint8_t pin, uint8_t direction)
{
	uint32_t reg = readl(KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OE));

	if (direction != 0) {
        reg |= (1 << pin);
    } else {
        reg &= ~(1 << pin);
    }

	writel(reg, KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OE));
	readl(KSEG1ADDR(AR71XX_GPIO_BASE + GPIO_REG_OE));
}


#endif /* AR71XX_GPIO_H */
