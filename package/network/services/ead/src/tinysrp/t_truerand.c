/*
 *      Physically random numbers (very nearly uniform)
 *      D. P. Mitchell
 *      Modified by Matt Blaze 7/95
 */
/*
 * The authors of this software are Don Mitchell and Matt Blaze.
 *              Copyright (c) 1995 by AT&T.
 * Permission to use, copy, and modify this software without fee
 * is hereby granted, provided that this entire notice is included in
 * all copies of any software which is or includes a copy or
 * modification of this software and in all copies of the supporting
 * documentation for such software.
 *
 * This software may be subject to United States export controls.
 *
 * THIS SOFTWARE IS BEING PROVIDED "AS IS", WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTY.  IN PARTICULAR, NEITHER THE AUTHORS NOR AT&T MAKE ANY
 * REPRESENTATION OR WARRANTY OF ANY KIND CONCERNING THE MERCHANTABILITY
 * OF THIS SOFTWARE OR ITS FITNESS FOR ANY PARTICULAR PURPOSE.
 */

/*
 * WARNING: depending on the particular platform, raw_truerand()
 * output may be biased or correlated.  In general, you can expect
 * about 16 bits of "pseudo-entropy" out of each 32 bit word returned
 * by truerand(), but it may not be uniformly diffused.  You should
 * raw_therefore run the output through some post-whitening function
 * (like MD5 or DES or whatever) before using it to generate key
 * material.  (RSAREF's random package does this for you when you feed
 * raw_truerand() bits to the seed input function.)
 *
 * The application interface, for 8, 16, and 32 bit properly "whitened"
 * random numbers, can be found in trand8(), trand16(), and trand32().
 * Use those instead of calling raw_truerand() directly.
 *
 * The basic idea here is that between clock "skew" and various
 * hard-to-predict OS event arrivals, counting a tight loop will yield
 * a little (maybe a third of a bit or so) of "good" randomness per
 * interval clock tick.  This seems to work well even on unloaded
 * machines.  If there is a human operator at the machine, you should
 * augment truerand with other measure, like keyboard event timing.
 * On server machines (e.g., when you need to generate a
 * Diffie-Hellman secret) truerand alone may be good enough.
 *
 * Test these assumptions on your own platform before fielding a
 * system based on this software or these techniques.
 *
 * This software seems to work well (at 10 or so bits per
 * raw_truerand() call) on a Sun Sparc-20 under SunOS 4.1.3 and on a
 * P100 under BSDI 2.0.  You're on your own elsewhere.
 *
 */

#include "t_defines.h"

#include <signal.h>
#include <setjmp.h>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>

#ifdef OLD_TRUERAND
static jmp_buf env;
#endif
static unsigned volatile count
#ifndef OLD_TRUERAND
  , done = 0
#endif
;

static unsigned ocount;
static unsigned buffer;

static void
tick()
{
	struct itimerval it, oit;

	it.it_interval.tv_sec = 0;
	it.it_interval.tv_usec = 0;
	it.it_value.tv_sec = 0;
	it.it_value.tv_usec = 16665;
	if (setitimer(ITIMER_REAL, &it, &oit) < 0)
		perror("tick");
}

static void
interrupt()
{
	if (count) {
#ifdef OLD_TRUERAND
		longjmp(env, 1);
#else
		++done;
		return;
#endif
	}

	(void) signal(SIGALRM, interrupt);
	tick();
}

static unsigned long
roulette()
{
#ifdef OLD_TRUERAND
	if (setjmp(env)) {
		count ^= (count>>3) ^ (count>>6) ^ ocount;
		count &= 0x7;
		ocount=count;
		buffer = (buffer<<3) ^ count;
		return buffer;
	}
#else
	done = 0;
#endif
	(void) signal(SIGALRM, interrupt);
	count = 0;
	tick();
#ifdef OLD_TRUERAND
	for (;;)
#else
	while(done == 0)
#endif
		count++;        /* about 1 MHz on VAX 11/780 */
#ifndef OLD_TRUERAND
	count ^= (count>>3) ^ (count>>6) ^ ocount;
	count &= 0x7;
	ocount=count;
	buffer = (buffer<<3) ^ count;
	return buffer;
#endif
}

unsigned long
raw_truerand()
{
	count=0;
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	(void) roulette();
	return roulette();
}
