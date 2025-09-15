/* SPDX-License-Identifier: GPL-2.0-or-later
 * Copyright (C) 2022 Eneas Ulir de Queiroz
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_WOLFSSL
# include <wolfssl/options.h>
# include <wolfssl/openssl/evp.h>
#else
# include <openssl/evp.h>
#endif

int do_crypt(FILE *infile, FILE *outfile, const EVP_CIPHER *cipher, const char *key, const char *iv,
	     int enc, int padding)
{
    EVP_CIPHER_CTX *ctx;
    unsigned char inbuf[1024], outbuf[1024 + EVP_MAX_BLOCK_LENGTH];
    int inlen, outlen;

    ctx = EVP_CIPHER_CTX_new();
    EVP_CipherInit_ex(ctx, cipher, NULL, key, iv, enc);
    EVP_CIPHER_CTX_set_padding(ctx, padding);

    for (;;) {
	inlen = fread(inbuf, 1, 1024, infile);
	if (inlen <= 0)
	    break;
	if (!EVP_CipherUpdate(ctx, outbuf, &outlen, inbuf, inlen)) {
	    EVP_CIPHER_CTX_free(ctx);
	    return -1;
	}
	fwrite(outbuf, 1, outlen, outfile);
    }
    if (!EVP_CipherFinal_ex(ctx, outbuf, &outlen)) {
	EVP_CIPHER_CTX_free(ctx);
	return -1;
    }
    fwrite(outbuf, 1, outlen, outfile);

    EVP_CIPHER_CTX_free(ctx);
    return 0;
}

static void check_enc_dec(const int enc)
{
    if (enc == -1)
	return;
    fprintf(stderr, "Error: both -d and -e were specified.\n");
    exit(EXIT_FAILURE);
}

#ifndef USE_WOLFSSL
static void print_ciphers(const OBJ_NAME *name,void *arg) {
    fprintf(arg, "\t%s\n", name->name);
}
#endif

static void check_cipher(const EVP_CIPHER *cipher)
{
    if (cipher == NULL) {
        fprintf(stderr, "Error: invalid cipher: %s.\n", optarg);
#ifndef USE_WOLFSSL
        fprintf(stderr, "Supported ciphers: \n", optarg);
        OBJ_NAME_do_all_sorted(OBJ_NAME_TYPE_CIPHER_METH, print_ciphers, stderr);
#endif
        exit(EXIT_FAILURE);
    }
}

static void show_usage(const char* name)
{
    fprintf(stderr, "Usage: %s: [-d | -e] [-n] -k key [-i iv] [-c cipher]\n"
		    "-d = decrypt; -e = encrypt; -n = no padding\n", name);
}

int main(int argc, char *argv[])
{
    int enc = -1;
    unsigned char *iv = NULL;
    unsigned char *key = NULL;
    long len;
    int opt;
    int padding = 1;
    int need_iv = 1;
    const EVP_CIPHER *cipher = EVP_aes_128_cbc();
    int ret;

    while ((opt = getopt(argc, argv, "c:dei:k:n")) != -1) {
	switch (opt) {
	case 'c':
	    cipher = EVP_get_cipherbyname(optarg);
	    check_cipher(cipher);
	    int arglen = strlen(optarg);
	    if (arglen > 3 && strncmp(optarg+arglen-3, "ecb", 3) == 0) //if ends with "ecb"
	        need_iv = 0;
	    break;
	case 'd':
	    check_enc_dec(enc);
	    enc = 0;
	    break;
	case 'e':
	    check_enc_dec(enc);
	    enc = 1;
	    break;
	case 'i':
	    iv = OPENSSL_hexstr2buf((const char *)optarg, &len);
	    if (iv == NULL) {
		fprintf(stderr, "Error setting IV to %s. The IV should be encoded in hex.\n",
			optarg);
		exit(EINVAL);
	    }
	    break;
	case 'k':
	    key = OPENSSL_hexstr2buf((const char *)optarg, &len);
	    if (key == NULL) {
		fprintf(stderr, "Error setting key to %s. The key should be encoded in hex.\n",
			optarg);
		exit(EINVAL);
	    }
	    break;
	case 'n':
	    padding = 0;
	    break;
	default:
	    show_usage(argv[0]);
	    exit(EINVAL);
	}
    }
    if (need_iv && iv == NULL) {
        fprintf(stderr, "Error: iv not set.\n");
        show_usage(argv[0]);
        exit(EXIT_FAILURE);
    }
    if (key == NULL) {
	fprintf(stderr, "Error: key not set.\n");
	show_usage(argv[0]);
	exit(EXIT_FAILURE);
    }
    ret = do_crypt(stdin, stdout, cipher, key, iv, !!enc, padding);
    if (ret)
	fprintf(stderr, "Error during crypt operation.\n");
    OPENSSL_free(iv);
    OPENSSL_free(key);
    return ret;
}
