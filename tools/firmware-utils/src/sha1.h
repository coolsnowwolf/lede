#ifndef _SHA1_H
#define _SHA1_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _STD_TYPES
#define _STD_TYPES

#define uchar   unsigned char
#define uint    unsigned int
#define ulong   unsigned long int

#endif

typedef struct
{
    ulong total[2];
    ulong state[5];
    uchar buffer[64];
}
sha1_context;

/*
 * Core SHA-1 functions
 */
void sha1_starts( sha1_context *ctx );
void sha1_update( sha1_context *ctx, void *input, uint length );
void sha1_finish( sha1_context *ctx, uchar digest[20] );

/*
 * Output SHA-1(file contents), returns 0 if successful.
 */
int sha1_file( char *filename, uchar digest[20] );

/*
 * Output SHA-1(buf)
 */
void sha1_csum( uchar *buf, uint buflen, uchar digest[20] );

/*
 * Output HMAC-SHA-1(key,buf)
 */
void sha1_hmac( uchar *key, uint keylen, uchar *buf, uint buflen,
                uchar digest[20] );

/*
 * Checkup routine
 */
int sha1_self_test( void );

#ifdef __cplusplus
}
#endif

#endif /* sha1.h */
