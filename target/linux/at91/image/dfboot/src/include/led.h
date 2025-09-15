/*
 * (C) Copyright 2006
 * Atmel Nordic AB <www.atmel.com>
 * Ulf Samuelsson <ulf@atmel.com>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */
 
 #ifndef __LED_H
#define __LED_H

#ifndef	__ASSEMBLY__
extern void	LED_init (void);
extern void	LED_set(unsigned int led);
extern void	red_LED_on(void);
extern void	red_LED_off(void);
extern void	green_LED_on(void);
extern void	green_LED_off(void);
extern void	yellow_LED_on(void);
extern void	yellow_LED_off(void);
extern void	LED_blink(unsigned int led);
#else
	.extern LED_init
	.extern LED_set
	.extern LED_blink
	.extern red_LED_on
	.extern red_LED_off
	.extern yellow_LED_on
	.extern yellow_LED_off
	.extern green_LED_on
	.extern green_LED_off
#endif
#endif
