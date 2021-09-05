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
 * Some performance testing of bigint.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ssl.h"

/**************************************************************************
 * BIGINT tests 
 *
 **************************************************************************/

int main(int argc, char *argv[])
{
#ifdef CONFIG_SSL_CERT_VERIFICATION
    RSA_CTX *rsa_ctx;
    BI_CTX *ctx;
    bigint *bi_data, *bi_res;
    int diff, res = 1;
    struct timeval tv_old, tv_new;
    const char *plaintext;
    uint8_t compare[MAX_KEY_BYTE_SIZE];
    int i, max_biggie = 10;    /* really crank performance */
    int len; 
    uint8_t *buf;

    /**
     * 512 bit key
     */
    plaintext = /* 64 byte number */
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^";

    len = get_file("../ssl/test/axTLS.key_512", &buf);
    asn1_get_private_key(buf, len, &rsa_ctx);
    ctx = rsa_ctx->bi_ctx;
    bi_data = bi_import(ctx, (uint8_t *)plaintext, strlen(plaintext));
    bi_res = RSA_public(rsa_ctx, bi_data);
    bi_data = bi_res;   /* reuse again */

    gettimeofday(&tv_old, NULL);
    for (i = 0; i < max_biggie; i++)
    {
        bi_res = RSA_private(rsa_ctx, bi_copy(bi_data));
        if (i < max_biggie-1)
        {
            bi_free(ctx, bi_res);
        }
    }

    gettimeofday(&tv_new, NULL);
    bi_free(ctx, bi_data);

    diff = (tv_new.tv_sec-tv_old.tv_sec)*1000 +
                (tv_new.tv_usec-tv_old.tv_usec)/1000;
    printf("512 bit decrypt time: %dms\n", diff/max_biggie);
    TTY_FLUSH();
    bi_export(ctx, bi_res, compare, 64);
    RSA_free(rsa_ctx);
    free(buf);
    if (memcmp(plaintext, compare, 64) != 0)
        goto end;

    /**
     * 1024 bit key
     */
    plaintext = /* 128 byte number */
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^";

    len = get_file("../ssl/test/axTLS.key_1024", &buf);
    asn1_get_private_key(buf, len, &rsa_ctx);
    ctx = rsa_ctx->bi_ctx;
    bi_data = bi_import(ctx, (uint8_t *)plaintext, strlen(plaintext));
    bi_res = RSA_public(rsa_ctx, bi_data);
    bi_data = bi_res;   /* reuse again */

    gettimeofday(&tv_old, NULL);
    for (i = 0; i < max_biggie; i++)
    {
        bi_res = RSA_private(rsa_ctx, bi_copy(bi_data));
        if (i < max_biggie-1)
        {
            bi_free(ctx, bi_res);
        }
    }

    gettimeofday(&tv_new, NULL);
    bi_free(ctx, bi_data);

    diff = (tv_new.tv_sec-tv_old.tv_sec)*1000 +
                (tv_new.tv_usec-tv_old.tv_usec)/1000;
    printf("1024 bit decrypt time: %dms\n", diff/max_biggie);
    TTY_FLUSH();
    bi_export(ctx, bi_res, compare, 128);
    RSA_free(rsa_ctx);
    free(buf);
    if (memcmp(plaintext, compare, 128) != 0)
        goto end;

    /**
     * 2048 bit key
     */
    plaintext = /* 256 byte number */
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^";

    len = get_file("../ssl/test/axTLS.key_2048", &buf);
    asn1_get_private_key(buf, len, &rsa_ctx);
    ctx = rsa_ctx->bi_ctx;
    bi_data = bi_import(ctx, (uint8_t *)plaintext, strlen(plaintext));
    bi_res = RSA_public(rsa_ctx, bi_data);
    bi_data = bi_res;   /* reuse again */

    gettimeofday(&tv_old, NULL);
    for (i = 0; i < max_biggie; i++)
    {
        bi_res = RSA_private(rsa_ctx, bi_copy(bi_data));
        if (i < max_biggie-1)
        {
            bi_free(ctx, bi_res);
        }
    }
    gettimeofday(&tv_new, NULL);
    bi_free(ctx, bi_data);

    diff = (tv_new.tv_sec-tv_old.tv_sec)*1000 +
                (tv_new.tv_usec-tv_old.tv_usec)/1000;
    printf("2048 bit decrypt time: %dms\n", diff/max_biggie);
    TTY_FLUSH();
    bi_export(ctx, bi_res, compare, 256);
    RSA_free(rsa_ctx);
    free(buf);
    if (memcmp(plaintext, compare, 256) != 0)
        goto end;

    /**
     * 4096 bit key
     */
    plaintext = /* 512 byte number */
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^"
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ*^";

    len = get_file("../ssl/test/axTLS.key_4096", &buf);
    asn1_get_private_key(buf, len, &rsa_ctx);
    ctx = rsa_ctx->bi_ctx;
    bi_data = bi_import(ctx, (uint8_t *)plaintext, strlen(plaintext));
    gettimeofday(&tv_old, NULL);
    bi_res = RSA_public(rsa_ctx, bi_data);
    gettimeofday(&tv_new, NULL);
    diff = (tv_new.tv_sec-tv_old.tv_sec)*1000 +
                (tv_new.tv_usec-tv_old.tv_usec)/1000;
    printf("4096 bit encrypt time: %dms\n", diff);
    TTY_FLUSH();
    bi_data = bi_res;   /* reuse again */

    gettimeofday(&tv_old, NULL);
    for (i = 0; i < max_biggie; i++)
    {
        bi_res = RSA_private(rsa_ctx, bi_copy(bi_data));
        if (i < max_biggie-1)
        {
            bi_free(ctx, bi_res);
        }
    }

    gettimeofday(&tv_new, NULL);
    bi_free(ctx, bi_data);

    diff = (tv_new.tv_sec-tv_old.tv_sec)*1000 +
                (tv_new.tv_usec-tv_old.tv_usec)/1000;
    printf("4096 bit decrypt time: %dms\n", diff/max_biggie);
    TTY_FLUSH();
    bi_export(ctx, bi_res, compare, 512);
    RSA_free(rsa_ctx);
    free(buf);
    if (memcmp(plaintext, compare, 512) != 0)
        goto end;

    /* done */
    printf("Bigint performance testing complete\n");
    res = 0;

end:
    return res;
#else
    return 0;
#endif
}
