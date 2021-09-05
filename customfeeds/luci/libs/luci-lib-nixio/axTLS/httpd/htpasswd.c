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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ssl.h"

int tfd;

void base64_encode(const uint8_t *in, size_t inlen, char *out, size_t outlen)
{
    static const char b64str[64] =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    while (inlen && outlen)
    {
        *out++ = b64str[(in[0] >> 2) & 0x3f];
        if (!--outlen)
            break;

        *out++ = b64str[((in[0] << 4)
                + (--inlen ? in[1] >> 4 : 0)) & 0x3f];
        if (!--outlen)
            break;
        *out++ = (inlen
             ? b64str[((in[1] << 2)
                 + (--inlen ? in[2] >> 6 : 0))
             & 0x3f]
             : '=');
        if (!--outlen)
            break;
        *out++ = inlen ? b64str[in[2] & 0x3f] : '=';
        if (!--outlen)
            break;
        if (inlen)
            inlen--;
        if (inlen)
            in += 3;
    }

    if (outlen)
        *out = '\0';
}

static void usage(void) 
{
    fprintf(stderr,"Usage: htpasswd username\n");
    exit(1);
}

#ifdef WIN32
static char * getpass(const char *prompt)
{
    static char buf[127];
    FILE *fp = stdin;

    printf(prompt); TTY_FLUSH();
#if 0
    fp = fopen("/dev/tty", "w");
    if (fp == NULL) 
    {
        printf("null\n"); TTY_FLUSH();
        fp = stdin;
    }
#endif

    fgets(buf, sizeof(buf), fp);
    while (buf[strlen(buf)-1] < ' ') 
        buf[strlen(buf)-1] = '\0';

    //if (fp != stdin) 
    //    fclose(fp);
    return buf;
}
#endif

int main(int argc, char *argv[]) 
{
    char* pw;
    uint8_t md5_salt[MD5_SIZE], md5_pass[MD5_SIZE];
    char b64_salt[MD5_SIZE+10], b64_pass[MD5_SIZE+10];
    MD5_CTX ctx;

    if (argc != 2)
        usage();

    pw = strdup(getpass("New password:"));
    if (strcmp(pw, getpass("Re-type new password:")) != 0)
    {
        fprintf(stderr, "They don't match, sorry.\n" );
        exit(1);
    }

    RNG_initialize((uint8_t *)pw, sizeof(pw));
    get_random(MD5_SIZE, md5_salt);
    RNG_terminate();
    base64_encode(md5_salt, MD5_SIZE, b64_salt, sizeof(b64_salt));

    MD5_Init(&ctx);
    MD5_Update(&ctx, md5_salt, MD5_SIZE);
    MD5_Update(&ctx, (uint8_t *)pw, strlen(pw));
    MD5_Final(md5_pass, &ctx);
    base64_encode(md5_pass, MD5_SIZE, b64_pass, sizeof(b64_pass));

    printf("Add the following to your '.htpasswd' file\n");
    printf("%s:%s$%s\n", argv[1], b64_salt, b64_pass);
    return 0;
}
