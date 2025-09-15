#ifndef T_SHA_H
#define T_SHA_H

#if     !defined(P)
#ifdef  __STDC__
#define P(x)    x
#else
#define P(x)    ()
#endif
#endif

#define SHA_DIGESTSIZE 20

typedef unsigned int uint32;

typedef struct {
    uint32 state[5];
    uint32 count[2];
    unsigned char buffer[64];
} SHA1_CTX;

void SHA1Init P((SHA1_CTX* context));
void SHA1Update P((SHA1_CTX* context, const unsigned char* data, unsigned int len));
void SHA1Final P((unsigned char digest[20], SHA1_CTX* context));

#endif /* T_SHA_H */
