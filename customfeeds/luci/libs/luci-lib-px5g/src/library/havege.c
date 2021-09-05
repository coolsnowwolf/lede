/*
 *  HAVEGE: HArdware Volatile Entropy Gathering and Expansion
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
/*
 *  The HAVEGE RNG was designed by Andre Seznec in 2002.
 *
 *  http://www.irisa.fr/caps/projects/hipsor/publi.php
 *
 *  Contact: seznec(at)irisa_dot_fr - orocheco(at)irisa_dot_fr
 */

#include <string.h>
#include <time.h>

#include "polarssl/config.h"

#if defined(POLARSSL_HAVEGE_C)

#include "polarssl/havege.h"
#include "polarssl/timing.h"

/* ------------------------------------------------------------------------
 * On average, one iteration accesses two 8-word blocks in the havege WALK
 * table, and generates 16 words in the RES array.
 *
 * The data read in the WALK table is updated and permuted after each use.
 * The result of the hardware clock counter read is used  for this update.
 *
 * 25 conditional tests are present.  The conditional tests are grouped in
 * two nested  groups of 12 conditional tests and 1 test that controls the
 * permutation; on average, there should be 6 tests executed and 3 of them
 * should be mispredicted.
 * ------------------------------------------------------------------------
 */

#define SWAP(X,Y) { int *T = X; X = Y; Y = T; }

#define TST1_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;
#define TST2_ENTER if( PTEST & 1 ) { PTEST ^= 3; PTEST >>= 1;

#define TST1_LEAVE U1++; }
#define TST2_LEAVE U2++; }

#define ONE_ITERATION                                   \
                                                        \
    PTEST = PT1 >> 20;                                  \
                                                        \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
    TST1_ENTER  TST1_ENTER  TST1_ENTER  TST1_ENTER      \
                                                        \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
    TST1_LEAVE  TST1_LEAVE  TST1_LEAVE  TST1_LEAVE      \
                                                        \
    PTX = (PT1 >> 18) & 7;                              \
    PT1 &= 0x1FFF;                                      \
    PT2 &= 0x1FFF;                                      \
    CLK = (int) hardclock();                            \
                                                        \
    i = 0;                                              \
    A = &WALK[PT1    ]; RES[i++] ^= *A;                 \
    B = &WALK[PT2    ]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 1]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 4]; RES[i++] ^= *D;                 \
                                                        \
    IN = (*A >> (1)) ^ (*A << (31)) ^ CLK;              \
    *A = (*B >> (2)) ^ (*B << (30)) ^ CLK;              \
    *B = IN ^ U1;                                       \
    *C = (*C >> (3)) ^ (*C << (29)) ^ CLK;              \
    *D = (*D >> (4)) ^ (*D << (28)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 2]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 2]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 3]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 6]; RES[i++] ^= *D;                 \
                                                        \
    if( PTEST & 1 ) SWAP( A, C );                       \
                                                        \
    IN = (*A >> (5)) ^ (*A << (27)) ^ CLK;              \
    *A = (*B >> (6)) ^ (*B << (26)) ^ CLK;              \
    *B = IN; CLK = (int) hardclock();                   \
    *C = (*C >> (7)) ^ (*C << (25)) ^ CLK;              \
    *D = (*D >> (8)) ^ (*D << (24)) ^ CLK;              \
                                                        \
    A = &WALK[PT1 ^ 4];                                 \
    B = &WALK[PT2 ^ 1];                                 \
                                                        \
    PTEST = PT2 >> 1;                                   \
                                                        \
    PT2 = (RES[(i - 8) ^ PTY] ^ WALK[PT2 ^ PTY ^ 7]);   \
    PT2 = ((PT2 & 0x1FFF) & (~8)) ^ ((PT1 ^ 8) & 0x8);  \
    PTY = (PT2 >> 10) & 7;                              \
                                                        \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
    TST2_ENTER  TST2_ENTER  TST2_ENTER  TST2_ENTER      \
                                                        \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
    TST2_LEAVE  TST2_LEAVE  TST2_LEAVE  TST2_LEAVE      \
                                                        \
    C = &WALK[PT1 ^ 5];                                 \
    D = &WALK[PT2 ^ 5];                                 \
                                                        \
    RES[i++] ^= *A;                                     \
    RES[i++] ^= *B;                                     \
    RES[i++] ^= *C;                                     \
    RES[i++] ^= *D;                                     \
                                                        \
    IN = (*A >> ( 9)) ^ (*A << (23)) ^ CLK;             \
    *A = (*B >> (10)) ^ (*B << (22)) ^ CLK;             \
    *B = IN ^ U2;                                       \
    *C = (*C >> (11)) ^ (*C << (21)) ^ CLK;             \
    *D = (*D >> (12)) ^ (*D << (20)) ^ CLK;             \
                                                        \
    A = &WALK[PT1 ^ 6]; RES[i++] ^= *A;                 \
    B = &WALK[PT2 ^ 3]; RES[i++] ^= *B;                 \
    C = &WALK[PT1 ^ 7]; RES[i++] ^= *C;                 \
    D = &WALK[PT2 ^ 7]; RES[i++] ^= *D;                 \
                                                        \
    IN = (*A >> (13)) ^ (*A << (19)) ^ CLK;             \
    *A = (*B >> (14)) ^ (*B << (18)) ^ CLK;             \
    *B = IN;                                            \
    *C = (*C >> (15)) ^ (*C << (17)) ^ CLK;             \
    *D = (*D >> (16)) ^ (*D << (16)) ^ CLK;             \
                                                        \
    PT1 = ( RES[(i - 8) ^ PTX] ^                        \
            WALK[PT1 ^ PTX ^ 7] ) & (~1);               \
    PT1 ^= (PT2 ^ 0x10) & 0x10;                         \
                                                        \
    for( n++, i = 0; i < 16; i++ )                      \
        hs->pool[n % COLLECT_SIZE] ^= RES[i];

/*
 * Entropy gathering function
 */
static void havege_fill( havege_state *hs )
{
    int i, n = 0;
    int  U1,  U2, *A, *B, *C, *D;
    int PT1, PT2, *WALK, RES[16];
    int PTX, PTY, CLK, PTEST, IN;

    WALK = hs->WALK;
    PT1  = hs->PT1;
    PT2  = hs->PT2;

    PTX  = U1 = 0;
    PTY  = U2 = 0;

    memset( RES, 0, sizeof( RES ) );

    while( n < COLLECT_SIZE * 4 )
    {
        ONE_ITERATION
        ONE_ITERATION
        ONE_ITERATION
        ONE_ITERATION
    }

    hs->PT1 = PT1;
    hs->PT2 = PT2;

    hs->offset[0] = 0;
    hs->offset[1] = COLLECT_SIZE / 2;
}

/*
 * HAVEGE initialization
 */
void havege_init( havege_state *hs )
{
    memset( hs, 0, sizeof( havege_state ) );

    havege_fill( hs );
}

/*
 * HAVEGE rand function
 */
int havege_rand( void *p_rng )
{
    int ret;
    havege_state *hs = (havege_state *) p_rng;

    if( hs->offset[1] >= COLLECT_SIZE )
        havege_fill( hs );

    ret  = hs->pool[hs->offset[0]++];
    ret ^= hs->pool[hs->offset[1]++];

    return( ret );
}

#if defined(POLARSSL_RAND_TEST)

#include <stdio.h>

int main( int argc, char *argv[] )
{
    FILE *f;
    time_t t;
    int i, j, k;
    havege_state hs;
    unsigned char buf[1024];

    if( argc < 2 )
    {
        fprintf( stderr, "usage: %s <output filename>\n", argv[0] );
        return( 1 );
    }

    if( ( f = fopen( argv[1], "wb+" ) ) == NULL )
    {
        printf( "failed to open '%s' for writing.\n", argv[0] );
        return( 1 );
    }

    havege_init( &hs );

    t = time( NULL );

    for( i = 0, k = 32768; i < k; i++ )
    {
        for( j = 0; j < sizeof( buf ); j++ )
            buf[j] = havege_rand( &hs );

        fwrite( buf, sizeof( buf ), 1, f );

        printf( "Generating 32Mb of data in file '%s'... %04.1f" \
                "%% done\r", argv[1], (100 * (float) (i + 1)) / k );
        fflush( stdout );
    }

    if( t == time( NULL ) )
        t--;

    fclose( f );
    return( 0 );
}

#endif

#endif
