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

#include <AT91RM9200.h>

#define	GREEN_LED	AT91C_PIO_PB0
#define	YELLOW_LED	AT91C_PIO_PB1
#define	RED_LED	AT91C_PIO_PB2

void	LED_set(unsigned int led)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
	PIOB->PIO_SODR		= (led ^ 0x7) & 0x7;		// All 0's => Set PIO high => OFF
	PIOB->PIO_CODR		=  led & 0x7;			// All 1's => Set PIO low   => ON
}

void	green_LED_on(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_CODR		= GREEN_LED;
	PIOB->PIO_CODR		= (1 << 0);
}

void	 yellow_LED_on(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_CODR		= YELLOW_LED;
	PIOB->PIO_CODR		= (1 << 1);
}

void	 red_LED_on(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_CODR		= RED_LED;
	PIOB->PIO_CODR		= (1 << 2);
}

void	green_LED_off(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_SODR		= GREEN_LED;
	PIOB->PIO_SODR		= (1 << 0);
}

void	yellow_LED_off(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_SODR		= YELLOW_LED;
	PIOB->PIO_SODR		= (1 << 1);
}

void	red_LED_off(void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
//	PIOB->PIO_SODR		= RED_LED;
	PIOB->PIO_SODR		= (1 << 2);
}

void	LED_blink(unsigned int led)
{
	volatile int i,j;
	for(i = 0; i < 5; i++) {
		LED_set((1 << led)&0x7);
		for(j= 0; j < 200000; j++);
		LED_set(0);
		for(j= 0; j < 200000; j++);
	}		
}


void LED_init (void)
{
	AT91PS_PIO	PIOB	= AT91C_BASE_PIOB;
	AT91PS_PMC	PMC	= AT91C_BASE_PMC;
	PMC->PMC_PCER		= (1 << AT91C_ID_PIOB);	// Enable PIOB clock
	// Disable peripherals on LEDs
	PIOB->PIO_PER		= AT91C_PIO_PB2 | AT91C_PIO_PB1 | AT91C_PIO_PB0;
	// Enable pins as outputs
	PIOB->PIO_OER		= AT91C_PIO_PB2 | AT91C_PIO_PB1 | AT91C_PIO_PB0;
	// Turn all LEDs OFF
	PIOB->PIO_SODR		= AT91C_PIO_PB2 | AT91C_PIO_PB1 | AT91C_PIO_PB0;
}
