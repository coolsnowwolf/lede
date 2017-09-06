/*
 *  FIPS-180-1 compliant SHA-1 implementation
 *
 *  Copyright (C) 2003-2006  Christophe Devine
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License, version 2.1 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *  MA  02110-1301  USA
 */
/*
 *  The SHA-1 standard was published by NIST in 1993.
 *
 *  http://www.itl.nist.gov/fipspubs/fip180-1.htm
 */

#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE 1
#endif

#include <string.h>
#include <stdio.h>

#include "sha1.h"

/* 
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_UINT32_BE
#define GET_UINT32_BE(n,b,i)                    \
{                                               \
    (n) = ( (ulong) (b)[(i)    ] << 24 )        \
        | ( (ulong) (b)[(i) + 1] << 16 )        \
        | ( (ulong) (b)[(i) + 2] <<  8 )        \
        | ( (ulong) (b)[(i) + 3]       );       \
}
#endif
#ifndef PUT_UINT32_BE
#define PUT_UINT32_BE(n,b,i)                    \
{                                               \
    (b)[(i)    ] = (uchar) ( (n) >> 24 );       \
    (b)[(i) + 1] = (uchar) ( (n) >> 16 );       \
    (b)[(i) + 2] = (uchar) ( (n) >>  8 );       \
    (b)[(i) + 3] = (uchar) ( (n)       );       \
}
#endif

/*
 * Core SHA-1 functions
 */
void sha1_starts( sha1_context *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xEFCDAB89;
    ctx->state[2] = 0x98BADCFE;
    ctx->state[3] = 0x10325476;
    ctx->state[4] = 0xC3D2E1F0;
}

void sha1_process( sha1_context *ctx, uchar data[64] )
{
    ulong temp, W[16], A, B, C, D, E;

    GET_UINT32_BE( W[0],  data,  0 );
    GET_UINT32_BE( W[1],  data,  4 );
    GET_UINT32_BE( W[2],  data,  8 );
    GET_UINT32_BE( W[3],  data, 12 );
    GET_UINT32_BE( W[4],  data, 16 );
    GET_UINT32_BE( W[5],  data, 20 );
    GET_UINT32_BE( W[6],  data, 24 );
    GET_UINT32_BE( W[7],  data, 28 );
    GET_UINT32_BE( W[8],  data, 32 );
    GET_UINT32_BE( W[9],  data, 36 );
    GET_UINT32_BE( W[10], data, 40 );
    GET_UINT32_BE( W[11], data, 44 );
    GET_UINT32_BE( W[12], data, 48 );
    GET_UINT32_BE( W[13], data, 52 );
    GET_UINT32_BE( W[14], data, 56 );
    GET_UINT32_BE( W[15], data, 60 );

#define S(x,n) ((x << n) | ((x & 0xFFFFFFFF) >> (32 - n)))

#define R(t)                                            \
(                                                       \
    temp = W[(t -  3) & 0x0F] ^ W[(t - 8) & 0x0F] ^     \
           W[(t - 14) & 0x0F] ^ W[ t      & 0x0F],      \
    ( W[t & 0x0F] = S(temp,1) )                         \
)

#define P(a,b,c,d,e,x)                                  \
{                                                       \
    e += S(a,5) + F(b,c,d) + K + x; b = S(b,30);        \
}

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];

#define F(x,y,z) (z ^ (x & (y ^ z)))
#define K 0x5A827999

    P( A, B, C, D, E, W[0]  );
    P( E, A, B, C, D, W[1]  );
    P( D, E, A, B, C, W[2]  );
    P( C, D, E, A, B, W[3]  );
    P( B, C, D, E, A, W[4]  );
    P( A, B, C, D, E, W[5]  );
    P( E, A, B, C, D, W[6]  );
    P( D, E, A, B, C, W[7]  );
    P( C, D, E, A, B, W[8]  );
    P( B, C, D, E, A, W[9]  );
    P( A, B, C, D, E, W[10] );
    P( E, A, B, C, D, W[11] );
    P( D, E, A, B, C, W[12] );
    P( C, D, E, A, B, W[13] );
    P( B, C, D, E, A, W[14] );
    P( A, B, C, D, E, W[15] );
    P( E, A, B, C, D, R(16) );
    P( D, E, A, B, C, R(17) );
    P( C, D, E, A, B, R(18) );
    P( B, C, D, E, A, R(19) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0x6ED9EBA1

    P( A, B, C, D, E, R(20) );
    P( E, A, B, C, D, R(21) );
    P( D, E, A, B, C, R(22) );
    P( C, D, E, A, B, R(23) );
    P( B, C, D, E, A, R(24) );
    P( A, B, C, D, E, R(25) );
    P( E, A, B, C, D, R(26) );
    P( D, E, A, B, C, R(27) );
    P( C, D, E, A, B, R(28) );
    P( B, C, D, E, A, R(29) );
    P( A, B, C, D, E, R(30) );
    P( E, A, B, C, D, R(31) );
    P( D, E, A, B, C, R(32) );
    P( C, D, E, A, B, R(33) );
    P( B, C, D, E, A, R(34) );
    P( A, B, C, D, E, R(35) );
    P( E, A, B, C, D, R(36) );
    P( D, E, A, B, C, R(37) );
    P( C, D, E, A, B, R(38) );
    P( B, C, D, E, A, R(39) );

#undef K
#undef F

#define F(x,y,z) ((x & y) | (z & (x | y)))
#define K 0x8F1BBCDC

    P( A, B, C, D, E, R(40) );
    P( E, A, B, C, D, R(41) );
    P( D, E, A, B, C, R(42) );
    P( C, D, E, A, B, R(43) );
    P( B, C, D, E, A, R(44) );
    P( A, B, C, D, E, R(45) );
    P( E, A, B, C, D, R(46) );
    P( D, E, A, B, C, R(47) );
    P( C, D, E, A, B, R(48) );
    P( B, C, D, E, A, R(49) );
    P( A, B, C, D, E, R(50) );
    P( E, A, B, C, D, R(51) );
    P( D, E, A, B, C, R(52) );
    P( C, D, E, A, B, R(53) );
    P( B, C, D, E, A, R(54) );
    P( A, B, C, D, E, R(55) );
    P( E, A, B, C, D, R(56) );
    P( D, E, A, B, C, R(57) );
    P( C, D, E, A, B, R(58) );
    P( B, C, D, E, A, R(59) );

#undef K
#undef F

#define F(x,y,z) (x ^ y ^ z)
#define K 0xCA62C1D6

    P( A, B, C, D, E, R(60) );
    P( E, A, B, C, D, R(61) );
    P( D, E, A, B, C, R(62) );
    P( C, D, E, A, B, R(63) );
    P( B, C, D, E, A, R(64) );
    P( A, B, C, D, E, R(65) );
    P( E, A, B, C, D, R(66) );
    P( D, E, A, B, C, R(67) );
    P( C, D, E, A, B, R(68) );
    P( B, C, D, E, A, R(69) );
    P( A, B, C, D, E, R(70) );
    P( E, A, B, C, D, R(71) );
    P( D, E, A, B, C, R(72) );
    P( C, D, E, A, B, R(73) );
    P( B, C, D, E, A, R(74) );
    P( A, B, C, D, E, R(75) );
    P( E, A, B, C, D, R(76) );
    P( D, E, A, B, C, R(77) );
    P( C, D, E, A, B, R(78) );
    P( B, C, D, E, A, R(79) );

#undef K
#undef F

    ctx->state[0] += A;
    ctx->state[1] += B;
    ctx->state[2] += C;
    ctx->state[3] += D;
    ctx->state[4] += E;
}

void sha1_update( sha1_context *ctx, uchar *input, uint length )
{
    ulong left, fill;

    if( ! length ) return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += length;
    ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < length )
        ctx->total[1]++;

    if( left && length >= fill )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, fill );
        sha1_process( ctx, ctx->buffer );
        length -= fill;
        input  += fill;
        left = 0;
    }

    while( length >= 64 )
    {
        sha1_process( ctx, input );
        length -= 64;
        input  += 64;
    }

    if( length )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, length );
    }
}

static uchar sha1_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

void sha1_finish( sha1_context *ctx, uchar digest[20] )
{
    ulong last, padn;
    ulong high, low;
    uchar msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_UINT32_BE( high, msglen, 0 );
    PUT_UINT32_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    sha1_update( ctx, sha1_padding, padn );
    sha1_update( ctx, msglen, 8 );

    PUT_UINT32_BE( ctx->state[0], digest,  0 );
    PUT_UINT32_BE( ctx->state[1], digest,  4 );
    PUT_UINT32_BE( ctx->state[2], digest,  8 );
    PUT_UINT32_BE( ctx->state[3], digest, 12 );
    PUT_UINT32_BE( ctx->state[4], digest, 16 );
}

/*
 * Output SHA-1(file contents), returns 0 if successful.
 */
int sha1_file( char *filename, uchar digest[20] )
{
    FILE *f;
    size_t n;
    sha1_context ctx;
    uchar buf[1024];

    if( ( f = fopen( filename, "rb" ) ) == NULL )
        return( 1 );

    sha1_starts( &ctx );

    while( ( n = fread( buf, 1, sizeof( buf ), f ) ) > 0 )
        sha1_update( &ctx, buf, (uint) n );

    sha1_finish( &ctx, digest );

    fclose( f );
    return( 0 );
}

/*
 * Output SHA-1(buf)
 */
void sha1_csum( uchar *buf, uint buflen, uchar digest[20] )
{
    sha1_context ctx;

    sha1_starts( &ctx );
    sha1_update( &ctx, buf, buflen );
    sha1_finish( &ctx, digest );
}

/*
 * Output HMAC-SHA-1(key,buf)
 */
void sha1_hmac( uchar *key, uint keylen, uchar *buf, uint buflen,
                uchar digest[20] )
{
    uint i;
    sha1_context ctx;
    uchar k_ipad[64];
    uchar k_opad[64];
    uchar tmpbuf[20];

    memset( k_ipad, 0x36, 64 );
    memset( k_opad, 0x5C, 64 );

    for( i = 0; i < keylen; i++ )
    {
        if( i >= 64 ) break;

        k_ipad[i] ^= key[i];
        k_opad[i] ^= key[i];
    }

    sha1_starts( &ctx );
    sha1_update( &ctx, k_ipad, 64 );
    sha1_update( &ctx, buf, buflen );
    sha1_finish( &ctx, tmpbuf );

    sha1_starts( &ctx );
    sha1_update( &ctx, k_opad, 64 );
    sha1_update( &ctx, tmpbuf, 20 );
    sha1_finish( &ctx, digest );

    memset( k_ipad, 0, 64 );
    memset( k_opad, 0, 64 );
    memset( tmpbuf, 0, 20 );
    memset( &ctx, 0, sizeof( sha1_context ) );
}

#ifdef SELF_TEST
/* 
 * FIPS-180-1 test vectors
 */
static char *sha1_test_str[3] = 
{
    "abc",
    "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
    NULL
};

static uchar sha1_test_sum[3][20] =
{
    { 0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
      0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D },
    { 0x84, 0x98, 0x3E, 0x44, 0x1C, 0x3B, 0xD2, 0x6E, 0xBA, 0xAE,
      0x4A, 0xA1, 0xF9, 0x51, 0x29, 0xE5, 0xE5, 0x46, 0x70, 0xF1 },
    { 0x34, 0xAA, 0x97, 0x3C, 0xD4, 0xC4, 0xDA, 0xA4, 0xF6, 0x1E,
      0xEB, 0x2B, 0xDB, 0xAD, 0x27, 0x31, 0x65, 0x34, 0x01, 0x6F }
};

/*
 * Checkup routine
 */
int sha1_self_test( void )
{
    int i, j;
    uchar buf[1000];
    uchar sha1sum[20];
    sha1_context ctx;

    for( i = 0; i < 3; i++ )
    {
        printf( "  SHA-1 test #%d: ", i + 1 );

        sha1_starts( &ctx );

        if( i < 2 )
            sha1_update( &ctx, (uchar *) sha1_test_str[i],
                         strlen( sha1_test_str[i] ) );
        else
        {
            memset( buf, 'a', 1000 );
            for( j = 0; j < 1000; j++ )
                sha1_update( &ctx, (uchar *) buf, 1000 );
        }

        sha1_finish( &ctx, sha1sum );

        if( memcmp( sha1sum, sha1_test_sum[i], 20 ) != 0 )
        {
            printf( "failed\n" );
            return( 1 );
        }

        printf( "passed\n" );
    }

    printf( "\n" );
    return( 0 );
}
#else
int sha1_self_test( void )
{
    printf( "SHA-1 self-test not available\n\n" );
    return( 1 );
}
#endif
