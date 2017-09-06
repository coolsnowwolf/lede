/*
 * (C) Copyright 2004
 * Texas Instruments
 * Richard Woodruff <r-woodruff2@ti.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 * Alex Zuepke <azu@sysgo.de>
 *
 * (C) Copyright 2002
 * Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
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

#include <common.h>
#include <asm/io.h>

#define TIMER_CLOCK	(CONFIG_SYS_CLK_FREQ / (1 << (CONFIG_TIMER_PRESCALE * 4)))
#define TIMER_LOAD_VAL 0xFFFFFF

/* macro to read the 32 bit timer */
#define READ_TIMER	(TIMER_LOAD_VAL - readl(CONFIG_SYS_TIMERBASE + TIMER_CURR)) \
			/ (TIMER_CLOCK / CONFIG_SYS_HZ)

#define READ_TIMER_HW	(TIMER_LOAD_VAL - readl(CONFIG_SYS_TIMERBASE + TIMER_CURR))

DECLARE_GLOBAL_DATA_PTR;

int timer_init (void)
{
	int32_t val;

	/* Start the counter ticking up */
	writel(TIMER_LOAD_VAL, CONFIG_SYS_TIMERBASE + TIMER_LOAD);	/* reload value on overflow*/

	val = (CONFIG_TIMER_PRESCALE << TIMER_PRESCALE_SHIFT) |
			(TIMER_MODE_PERIODIC << TIMER_MODE_SHIFT) |
			(TIMER_ENABLE << TIMER_ENABLE_SHIFT);		/* mask to enable timer*/
	writel(val, CONFIG_SYS_TIMERBASE + TIMER_CTRL);	/* start timer */

	/* reset time */
	gd->arch.lastinc = READ_TIMER;	/* capture current incrementer value */
	gd->arch.tbl = 0;		/* start "advancing" time stamp */

	return(0);
}
/*
 * timer without interrupts
 */
ulong get_timer (ulong base)
{
	return get_timer_masked () - base;
}

/* delay x useconds AND preserve advance timestamp value */
void __udelay (unsigned long usec)
{
	ulong tmo, tmp;

	if (usec > 100000) {		/* if "big" number, spread normalization to seconds */
		tmo = usec / 1000;	/* start to normalize for usec to ticks per sec */
		tmo *= CONFIG_SYS_HZ;	/* find number of "ticks" to wait to achieve target */
		tmo /= 1000;		/* finish normalize. */

		tmp = get_timer (0);		/* get current timestamp */
		while (get_timer (tmp) < tmo)/* loop till event */
			/*NOP*/;
	} else {			/* else small number, convert to hw ticks */
		tmo = usec * (TIMER_CLOCK / 1000) / 1000;
		/* timeout is no more than 0.1s, and the hw timer will roll over at most once */
		tmp = READ_TIMER_HW;
		while (((READ_TIMER_HW -tmp) & TIMER_LOAD_VAL) < tmo)/* loop till event */
			/*NOP*/;
	}
}

ulong get_timer_masked (void)
{
	ulong now = READ_TIMER;		/* current tick value */

	if (now >= gd->arch.lastinc) {		/* normal mode (non roll) */
		/* move stamp fordward with absoulte diff ticks */
		gd->arch.tbl += (now - gd->arch.lastinc);
	} else {
		/* we have rollover of incrementer */
		gd->arch.tbl += ((TIMER_LOAD_VAL / (TIMER_CLOCK / CONFIG_SYS_HZ))
				 - gd->arch.lastinc) + now;
	}
	gd->arch.lastinc = now;
	return gd->arch.tbl;
}


/*
 * This function is derived from PowerPC code (read timebase as long long).
 * On ARM it just returns the timer value.
 */
unsigned long long get_ticks(void)
{
	return get_timer(0);
}
/*
 * This function is derived from PowerPC code (timebase clock frequency).
 * On ARM it returns the number of timer ticks per second.
 */
ulong get_tbclk (void)
{
	ulong tbclk;
	tbclk = CONFIG_SYS_HZ;
	return tbclk;
}
