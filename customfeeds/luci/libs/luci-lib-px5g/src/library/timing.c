/*
 *  Portable interface to the CPU cycle counter
 *
 *  Based on XySSL: Copyright (C) 2006-2008  Christophe Devine
 *
 *  Copyright (C) 2009  Paul Bakker <polarssl_maintainer at polarssl dot org>
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the names of PolarSSL or XySSL nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "polarssl/config.h"

#if defined(POLARSSL_TIMING_C)

#include "polarssl/timing.h"

#if defined(WIN32)

#include <windows.h>
#include <winbase.h>

struct _hr_time
{
    LARGE_INTEGER start;
};

#else

#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

struct _hr_time
{
    struct timeval start;
};

#endif

#if (defined(_MSC_VER) && defined(_M_IX86)) || defined(__WATCOMC__)

unsigned long hardclock( void )
{
    unsigned long tsc;
    __asm   rdtsc
    __asm   mov  [tsc], eax
    return( tsc );
}

#else
#if defined(__GNUC__) && defined(__i386__)

unsigned long hardclock( void )
{
    unsigned long tsc;
    asm( "rdtsc" : "=a" (tsc) );
    return( tsc );
}

#else
#if defined(__GNUC__) && (defined(__amd64__) || defined(__x86_64__))

unsigned long hardclock( void )
{
    unsigned long lo, hi;
    asm( "rdtsc" : "=a" (lo), "=d" (hi) );
    return( lo | (hi << 32) );
}

#else
#if defined(__GNUC__) && (defined(__powerpc__) || defined(__ppc__))

unsigned long hardclock( void )
{
    unsigned long tbl, tbu0, tbu1;

    do
    {
        asm( "mftbu %0" : "=r" (tbu0) );
        asm( "mftb  %0" : "=r" (tbl ) );
        asm( "mftbu %0" : "=r" (tbu1) );
    }
    while( tbu0 != tbu1 );

    return( tbl );
}

#else
#if defined(__GNUC__) && defined(__sparc__)

unsigned long hardclock( void )
{
    unsigned long tick;
    asm( ".byte 0x83, 0x41, 0x00, 0x00" );
    asm( "mov   %%g1, %0" : "=r" (tick) );
    return( tick );
}

#else
#if defined(__GNUC__) && defined(__alpha__)

unsigned long hardclock( void )
{
    unsigned long cc;
    asm( "rpcc %0" : "=r" (cc) );
    return( cc & 0xFFFFFFFF );
}

#else
#if defined(__GNUC__) && defined(__ia64__)

unsigned long hardclock( void )
{
    unsigned long itc;
    asm( "mov %0 = ar.itc" : "=r" (itc) );
    return( itc );
}

#else

static int hardclock_init = 0;
static struct timeval tv_init;

unsigned long hardclock( void )
{
    struct timeval tv_cur;

    if( hardclock_init == 0 )
    {
        gettimeofday( &tv_init, NULL );
        hardclock_init = 1;
    }

    gettimeofday( &tv_cur, NULL );
    return( ( tv_cur.tv_sec  - tv_init.tv_sec  ) * 1000000
          + ( tv_cur.tv_usec - tv_init.tv_usec ) );
}

#endif /* generic */
#endif /* IA-64   */
#endif /* Alpha   */
#endif /* SPARC8  */
#endif /* PowerPC */
#endif /* AMD64   */
#endif /* i586+   */

int alarmed = 0;

#if defined(WIN32)

unsigned long get_timer( struct hr_time *val, int reset )
{
    unsigned long delta;
    LARGE_INTEGER offset, hfreq;
    struct _hr_time *t = (struct _hr_time *) val;

    QueryPerformanceCounter(  &offset );
    QueryPerformanceFrequency( &hfreq );

    delta = (unsigned long)( ( 1000 *
        ( offset.QuadPart - t->start.QuadPart ) ) /
           hfreq.QuadPart );

    if( reset )
        QueryPerformanceCounter( &t->start );

    return( delta );
}

DWORD WINAPI TimerProc( LPVOID uElapse )
{
    Sleep( (DWORD) uElapse );
    alarmed = 1;
    return( TRUE );
}

void set_alarm( int seconds )
{
    DWORD ThreadId;

    alarmed = 0;
    CloseHandle( CreateThread( NULL, 0, TimerProc,
        (LPVOID) ( seconds * 1000 ), 0, &ThreadId ) );
}

void m_sleep( int milliseconds )
{
    Sleep( milliseconds );
}

#else

unsigned long get_timer( struct hr_time *val, int reset )
{
    unsigned long delta;
    struct timeval offset;
    struct _hr_time *t = (struct _hr_time *) val;

    gettimeofday( &offset, NULL );

    delta = ( offset.tv_sec  - t->start.tv_sec  ) * 1000
          + ( offset.tv_usec - t->start.tv_usec ) / 1000;

    if( reset )
    {
        t->start.tv_sec  = offset.tv_sec;
        t->start.tv_usec = offset.tv_usec;
    }

    return( delta );
}

static void sighandler( int signum )
{
    alarmed = 1;
    signal( signum, sighandler );
}

void set_alarm( int seconds )
{
    alarmed = 0;
    signal( SIGALRM, sighandler );
    alarm( seconds );
}

void m_sleep( int milliseconds )
{
    struct timeval tv;

    tv.tv_sec  = milliseconds / 1000;
    tv.tv_usec = milliseconds * 1000;

    select( 0, NULL, NULL, NULL, &tv );
}

#endif

#endif
