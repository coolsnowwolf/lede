/*
 * Copyright (c) 1997-1999  The Stanford SRP Authentication Project
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS-IS" AND WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS, IMPLIED OR OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY
 * WARRANTY OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
 *
 * IN NO EVENT SHALL STANFORD BE LIABLE FOR ANY SPECIAL, INCIDENTAL,
 * INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER OR NOT ADVISED OF
 * THE POSSIBILITY OF DAMAGE, AND ON ANY THEORY OF LIABILITY, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * In addition, the following conditions apply:
 *
 * 1. Any software that incorporates the SRP authentication technology
 *    must display the following acknowlegment:
 *    "This product uses the 'Secure Remote Password' cryptographic
 *     authentication system developed by Tom Wu (tjw@CS.Stanford.EDU)."
 *
 * 2. Any software that incorporates all or part of the SRP distribution
 *    itself must also display the following acknowledgment:
 *    "This product includes software developed by Tom Wu and Eugene
 *     Jhong for the SRP Distribution (http://srp.stanford.edu/srp/)."
 *
 * 3. Redistributions in source or binary form must retain an intact copy
 *    of this copyright notice and list of conditions.
 */

#ifndef T_DEFINES_H
#define T_DEFINES_H

#ifndef P
#if defined(__STDC__) || defined(__cplusplus)
#define P(x) x
#else
#define P(x) ()
#endif
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#ifndef _DLLDECL
#define _DLLDECL

#ifdef MSVC15   /* MSVC1.5 support for 16 bit apps */
#define _MSVC15EXPORT _export
#define _MSVC20EXPORT
#define _DLLAPI _export _pascal
#define _TYPE(a) a _MSVC15EXPORT
#define DLLEXPORT 1

#elif MSVC20
#define _MSVC15EXPORT
#define _MSVC20EXPORT _declspec(dllexport)
#define _DLLAPI
#define _TYPE(a) _MSVC20EXPORT a
#define DLLEXPORT 1

#else                   /* Default, non-dll.  Use this for Unix or DOS */
#define _MSVC15DEXPORT
#define _MSVC20EXPORT
#define _DLLAPI
#define _TYPE(a) a
#endif
#endif

#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else /* not STDC_HEADERS */
#ifndef HAVE_STRCHR
#define strchr index
#define strrchr rindex
#endif
char *strchr(), *strrchr(), *strtok();
#ifndef HAVE_MEMCPY
#define memcpy(d, s, n) bcopy((s), (d), (n))
#endif
#endif /* not STDC_HEADERS */

#include <sys/types.h>

#if TIME_WITH_SYS_TIME
#include <sys/time.h>
#include <time.h>
#else  /* not TIME_WITH_SYS_TIME */
#if HAVE_SYS_TIME_H
#include <sys/time.h>
#else
#include <time.h>
#endif
#endif /* not TIME_WITH_SYS_TIME */

#if HAVE_TERMIOS_H
#include <termios.h>
#define STTY(fd, termio) tcsetattr(fd, TCSANOW, termio)
#define GTTY(fd, termio) tcgetattr(fd, termio)
#define TERMIO struct termios
#define USE_TERMIOS
#elif HAVE_TERMIO_H
#include <sys/ioctl.h>
#include <termio.h>
#define STTY(fd, termio) ioctl(fd, TCSETA, termio)
#define GTTY(fd, termio) ioctl(fd, TCGETA, termio)
#define TEMRIO struct termio
#define USE_TERMIO
#elif HAVE_SGTTY_H
#include <sgtty.h>
#define STTY(fd, termio) stty(fd, termio)
#define GTTY(fd, termio) gtty(fd, termio)
#define TERMIO struct sgttyb
#define USE_SGTTY
#endif

#ifdef USE_FTIME
#include <sys/timeb.h>
#endif

#ifndef MATH_PRIV
typedef void * BigInteger;
#endif

_TYPE( BigInteger ) BigIntegerFromInt P((unsigned int number));
_TYPE( BigInteger ) BigIntegerFromBytes P((unsigned char * bytes, int length));
_TYPE( int ) BigIntegerToBytes P((BigInteger src, unsigned char * dest));
_TYPE( int ) BigIntegerBitLen P((BigInteger b));
_TYPE( int ) BigIntegerCmp P((BigInteger c1, BigInteger c2));
_TYPE( int ) BigIntegerCmpInt P((BigInteger c1, unsigned int c2));
_TYPE( void ) BigIntegerLShift P((BigInteger result, BigInteger x,
				unsigned int bits));
_TYPE( void ) BigIntegerAdd P((BigInteger result, BigInteger a1, BigInteger a2));
_TYPE( void ) BigIntegerAddInt P((BigInteger result,
				BigInteger a1, unsigned int a2));
_TYPE( void ) BigIntegerSub P((BigInteger result, BigInteger s1, BigInteger s2));
_TYPE( void ) BigIntegerSubInt P((BigInteger result,
				BigInteger s1, unsigned int s2));
/* For BigIntegerMul{,Int}: result != m1, m2 */
_TYPE( void ) BigIntegerMul P((BigInteger result, BigInteger m1, BigInteger m2));
_TYPE( void ) BigIntegerMulInt P((BigInteger result,
				BigInteger m1, unsigned int m2));
_TYPE( void ) BigIntegerDivInt P((BigInteger result,
				BigInteger d, unsigned int m));
_TYPE( void ) BigIntegerMod P((BigInteger result, BigInteger d, BigInteger m));
_TYPE( unsigned int ) BigIntegerModInt P((BigInteger d, unsigned int m));
_TYPE( void ) BigIntegerModMul P((BigInteger result,
				BigInteger m1, BigInteger m2, BigInteger m));
_TYPE( void ) BigIntegerModExp P((BigInteger result, BigInteger base,
				BigInteger expt, BigInteger modulus));
_TYPE( void ) BigIntegerModExpInt P((BigInteger result, BigInteger base,
				   unsigned int expt, BigInteger modulus));
_TYPE( int ) BigIntegerCheckPrime P((BigInteger n));
_TYPE( void ) BigIntegerFree P((BigInteger b));

#endif
