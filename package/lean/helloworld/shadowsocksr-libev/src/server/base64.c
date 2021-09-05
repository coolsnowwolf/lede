#include "base64.h"

/* BASE 64 encode table */
static const char base64en[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BASE64_PAD  '='

#define BASE64DE_FIRST  '+'
#define BASE64DE_LAST   'z'

/* ASCII order for BASE 64 decode, -1 in unused character */
static const signed char base64de[] = {
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
        -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    /*                '+', ',', '-', '.', '/', */
        -1,  -1,  -1,  62,  -1,  -1,  -1,  63,
    /* '0', '1', '2', '3', '4', '5', '6', '7', */
        52,  53,  54,  55,  56,  57,  58,  59,
    /* '8', '9', ':', ';', '<', '=', '>', '?', */
        60,  61,  -1,  -1,  -1,  -1,  -1,  -1,
    /* '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', */
        -1,   0,   1,   2,   3,   4,   5,   6,
    /* 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', */
         7,   8,   9,  10,  11,  12,  13,  14,
    /* 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', */
        15,  16,  17,  18,  19,  20,  21,  22,
    /* 'X', 'Y', 'Z', '[', '\', ']', '^', '_', */
        23,  24,  25,  -1,  -1,  -1,  -1,  -1,
    /* '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', */
        -1,  26,  27,  28,  29,  30,  31,  32,
    /* 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', */
        33,  34,  35,  36,  37,  38,  39,  40,
    /* 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', */
        41,  42,  43,  44,  45,  46,  47,  48,
    /* 'x', 'y', 'z', */
        49,  50,  51,
};

int
base64_encode(const unsigned char *in, unsigned int inlen, char *out)
{
    unsigned int i, j;

    for (i = j = 0; i < inlen; i++) {
        int s = i % 3;          /* from 6/gcd(6, 8) */

        switch (s) {
        case 0:
            out[j++] = base64en[(in[i] >> 2) & 0x3F];
            continue;
        case 1:
            out[j++] = base64en[((in[i-1] & 0x3) << 4) + ((in[i] >> 4) & 0xF)];
            continue;
        case 2:
            out[j++] = base64en[((in[i-1] & 0xF) << 2) + ((in[i] >> 6) & 0x3)];
            out[j++] = base64en[in[i] & 0x3F];
        }
    }

    /* move back */
    i -= 1;

    /* check the last and add padding */
    if ((i % 3) == 0) {
        out[j++] = base64en[(in[i] & 0x3) << 4];
        out[j++] = BASE64_PAD;
        out[j++] = BASE64_PAD;
    } else if ((i % 3) == 1) {
        out[j++] = base64en[(in[i] & 0xF) << 2];
        out[j++] = BASE64_PAD;
    }

    return BASE64_OK;
}

int
base64_decode(const char *in, unsigned int inlen, unsigned char *out)
{
    unsigned int i, j;

    for (i = j = 0; i < inlen; i++) {
        int c;
        int s = i % 4;          /* from 8/gcd(6, 8) */

        if (in[i] == '=')
            return BASE64_OK;

        if (in[i] < BASE64DE_FIRST || in[i] > BASE64DE_LAST ||
            (c = base64de[(int)in[i]]) == -1)
            return BASE64_INVALID;

        switch (s) {
        case 0:
            out[j] = ((unsigned int)c << 2) & 0xFF;
            continue;
        case 1:
            out[j++] += ((unsigned int)c >> 4) & 0x3;

            /* if not last char with padding */
            if (i < (inlen - 3) || in[inlen - 2] != '=')
                out[j] = ((unsigned int)c & 0xF) << 4; 
            continue;
        case 2:
            out[j++] += ((unsigned int)c >> 2) & 0xF;

            /* if not last char with padding */
            if (i < (inlen - 2) || in[inlen - 1] != '=')
                out[j] =  ((unsigned int)c & 0x3) << 6;
            continue;
        case 3:
            out[j++] += (unsigned char)c;
        }
    }

    return BASE64_OK;
}
