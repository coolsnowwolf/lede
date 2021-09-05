/*
 * Copyright (c) 2007, Cameron Rich
 * 
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, 
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice, 
 *   this list of conditions and the following disclaimer in the documentation 
 *   and/or other materials provided with the distribution.
 * * Neither the name of the axTLS project nor the names of its contributors 
 *   may be used to endorse or promote products derived from this software 
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Some misc. routines to help things out
 */

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "crypto_misc.h"
#ifdef CONFIG_WIN32_USE_CRYPTO_LIB
#include "wincrypt.h"
#endif

#ifndef WIN32
static int rng_fd = -1;
#elif defined(CONFIG_WIN32_USE_CRYPTO_LIB)
static HCRYPTPROV gCryptProv;
#endif

#if (!defined(CONFIG_USE_DEV_URANDOM) && !defined(CONFIG_WIN32_USE_CRYPTO_LIB))
static uint64_t rng_num;
#endif

static int rng_ref_count;
const char * const unsupported_str = "Error: Feature not supported\n";

#ifndef CONFIG_SSL_SKELETON_MODE
/** 
 * Retrieve a file and put it into memory
 * @return The size of the file, or -1 on failure.
 */
int get_file(const char *filename, uint8_t **buf)
{
    int total_bytes = 0;
    int bytes_read = 0; 
    int filesize;
    FILE *stream = fopen(filename, "rb");

    if (stream == NULL)
    {
#ifdef CONFIG_SSL_FULL_MODE         
        printf("file '%s' does not exist\n", filename); TTY_FLUSH();
#endif
        return -1;
    }

    /* Win CE doesn't support stat() */
    fseek(stream, 0, SEEK_END);
    filesize = ftell(stream);
    *buf = (uint8_t *)malloc(filesize);
    fseek(stream, 0, SEEK_SET);

    do
    {
        bytes_read = fread(*buf+total_bytes, 1, filesize-total_bytes, stream);
        total_bytes += bytes_read;
    } while (total_bytes < filesize && bytes_read > 0);
    
    fclose(stream);
    return filesize;
}
#endif

/**
 * Initialise the Random Number Generator engine.
 * - On Win32 use the platform SDK's crypto engine.
 * - On Linux use /dev/urandom
 * - If none of these work then use a custom RNG.
 */
EXP_FUNC void STDCALL RNG_initialize(const uint8_t *seed_buf, int size)
{
    if (rng_ref_count == 0)
    {
#if !defined(WIN32) && defined(CONFIG_USE_DEV_URANDOM)
        rng_fd = ax_open("/dev/urandom", O_RDONLY);
#elif defined(WIN32) && defined(CONFIG_WIN32_USE_CRYPTO_LIB)
        if (!CryptAcquireContext(&gCryptProv, 
                          NULL, NULL, PROV_RSA_FULL, 0))
        {
            if (GetLastError() == NTE_BAD_KEYSET &&
                    !CryptAcquireContext(&gCryptProv, 
                           NULL, 
                           NULL, 
                           PROV_RSA_FULL, 
                           CRYPT_NEWKEYSET))
            {
                printf("CryptoLib: %x\n", unsupported_str, GetLastError());
                exit(1);
            }
        }
#else   
        /* help seed with the user's private key - this is a number that 
           should be hard to find, due to the fact that it relies on knowing 
           the private key */
        int i;  

        for (i = 0; i < size/(int)sizeof(uint64_t); i++)
            rng_num ^= *((uint64_t *)&seed_buf[i*sizeof(uint64_t)]);

        srand((long)&seed_buf);  /* use the stack ptr as another rnd seed */
#endif
    }

    rng_ref_count++;
}

/**
 * Terminate the RNG engine.
 */
EXP_FUNC void STDCALL RNG_terminate(void)
{
    if (--rng_ref_count == 0)
    {
#ifndef WIN32
        close(rng_fd);
#elif defined(CONFIG_WIN32_USE_CRYPTO_LIB)
        CryptReleaseContext(gCryptProv, 0);
#endif
    }
}

/**
 * Set a series of bytes with a random number. Individual bytes can be 0
 */
EXP_FUNC void STDCALL get_random(int num_rand_bytes, uint8_t *rand_data)
{   
#if !defined(WIN32) && defined(CONFIG_USE_DEV_URANDOM)
    /* use the Linux default */
    read(rng_fd, rand_data, num_rand_bytes);    /* read from /dev/urandom */
#elif defined(WIN32) && defined(CONFIG_WIN32_USE_CRYPTO_LIB)
    /* use Microsoft Crypto Libraries */
    CryptGenRandom(gCryptProv, num_rand_bytes, rand_data);
#else   /* nothing else to use, so use a custom RNG */
    /* The method we use when we've got nothing better. Use RC4, time 
       and a couple of random seeds to generate a random sequence */
    RC4_CTX rng_ctx;
    struct timeval tv;
    uint64_t big_num1, big_num2;

    gettimeofday(&tv, NULL);    /* yes I know we shouldn't do this */

    /* all numbers by themselves are pretty simple, but combined should 
     * be a challenge */
    big_num1 = (uint64_t)tv.tv_sec*(tv.tv_usec+1); 
    big_num2 = (uint64_t)rand()*big_num1;
    big_num1 ^= rng_num;

    memcpy(rand_data, &big_num1, sizeof(uint64_t));
    if (num_rand_bytes > sizeof(uint64_t))
        memcpy(&rand_data[8], &big_num2, sizeof(uint64_t));

    if (num_rand_bytes > 16)
    {
        /* clear rest of data */
        memset(&rand_data[16], 0, num_rand_bytes-16); 
    }

    RC4_setup(&rng_ctx, rand_data, 16); /* use as a key */
    RC4_crypt(&rng_ctx, rand_data, rand_data, num_rand_bytes);
    
    /* use last 8 bytes for next time */
    memcpy(&rng_num, &rand_data[num_rand_bytes-8], sizeof(uint64_t));    
#endif
}

/**
 * Set a series of bytes with a random number. Individual bytes are not zero.
 */
void get_random_NZ(int num_rand_bytes, uint8_t *rand_data)
{
    int i;
    get_random(num_rand_bytes, rand_data);

    for (i = 0; i < num_rand_bytes; i++)
    {
        while (rand_data[i] == 0)  /* can't be 0 */
            rand_data[i] = (uint8_t)(rand());
    }
}

/**
 * Some useful diagnostic routines
 */
#if defined(CONFIG_SSL_FULL_MODE) || defined(CONFIG_DEBUG)
int hex_finish;
int hex_index;

static void print_hex_init(int finish)
{
    hex_finish = finish;
    hex_index = 0;
}

static void print_hex(uint8_t hex)
{
    static int column;

    if (hex_index == 0)
    {
        column = 0;
    }

    printf("%02x ", hex);
    if (++column == 8)
    {
        printf(": ");
    }
    else if (column >= 16)
    {
        printf("\n");
        column = 0;
    }

    if (++hex_index >= hex_finish && column > 0)
    {
        printf("\n");
    }
}

/**
 * Spit out a blob of data for diagnostics. The data is is a nice column format
 * for easy reading.
 *
 * @param format   [in]    The string (with possible embedded format characters)
 * @param size     [in]    The number of numbers to print
 * @param data     [in]    The start of data to use
 * @param ...      [in]    Any additional arguments
 */
EXP_FUNC void STDCALL print_blob(const char *format, 
        const uint8_t *data, int size, ...)
{
    int i;
    char tmp[80];
    va_list(ap);

    va_start(ap, size);
    sprintf(tmp, "%s\n", format);
    vprintf(tmp, ap);
    print_hex_init(size);
    for (i = 0; i < size; i++)
    {
        print_hex(data[i]);
    }

    va_end(ap);
    TTY_FLUSH();
}
#elif defined(WIN32)
/* VC6.0 doesn't handle variadic macros */
EXP_FUNC void STDCALL print_blob(const char *format, const unsigned char *data,
        int size, ...) {}
#endif

#if defined(CONFIG_SSL_HAS_PEM) || defined(CONFIG_HTTP_HAS_AUTHORIZATION)
/* base64 to binary lookup table */
static const uint8_t map[128] =
{
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
    52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
    255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
    7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
    19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
    255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
    37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
    49,  50,  51, 255, 255, 255, 255, 255
};

EXP_FUNC int STDCALL base64_decode(const char *in, int len,
                    uint8_t *out, int *outlen)
{
    int g, t, x, y, z;
    uint8_t c;
    int ret = -1;

    g = 3;
    for (x = y = z = t = 0; x < len; x++)
    {
        if ((c = map[in[x]&0x7F]) == 0xff)
            continue;

        if (c == 254)   /* this is the end... */
        {
            c = 0;

            if (--g < 0)
                goto error;
        }
        else if (g != 3) /* only allow = at end */
            goto error;

        t = (t<<6) | c;

        if (++y == 4)
        {
            out[z++] = (uint8_t)((t>>16)&255);

            if (g > 1)
                out[z++] = (uint8_t)((t>>8)&255);

            if (g > 2)
                out[z++] = (uint8_t)(t&255);

            y = t = 0;
        }
    }

    if (y != 0)
        goto error;

    if (outlen)
        *outlen = z;
    ret = 0;

error:
#ifdef CONFIG_SSL_FULL_MODE
    if (ret < 0)
        printf("Error: Invalid base64\n"); TTY_FLUSH();
#endif
    TTY_FLUSH();
    return ret;

}
#endif

