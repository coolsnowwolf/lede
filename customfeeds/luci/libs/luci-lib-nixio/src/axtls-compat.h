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

/*
 * Enable a subset of openssl compatible functions. We don't aim to be 100%
 * compatible - just to be able to do basic ports etc.
 *
 * Only really tested on mini_httpd, so I'm not too sure how extensive this
 * port is.
 */

#include "nixio.h"
#include "config.h"

#define WITH_AXTLS						1
#define WITHOUT_OPENSSL					1
#define SSL_OP_NO_SSLv3					0x02000000L
#define SSL_OP_NO_SSLv2					0x01000000L
#define SSL_FILETYPE_PEM				1
#define SSL_FILETYPE_ASN1				2
#define SSL_VERIFY_NONE					0x00
#define SSL_VERIFY_PEER					0x01
#define SSL_VERIFY_FAIL_IF_NO_PEER_CERT 0x02
#define SSL_VERIFY_CLIENT_ONCE			0x03
#define MD5_DIGEST_LENGTH				16
#define SHA_DIGEST_LENGTH				20

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ssl.h"

typedef SHA1_CTX SHA_CTX;

void *SSLv23_server_method(void);
void *SSLv3_server_method(void);
void *TLSv1_server_method(void);
void *SSLv23_client_method(void);
void *SSLv3_client_method(void);
void *TLSv1_client_method(void);
void *SSLv23_method(void);
void *TLSv1_method(void);


typedef void * (*ssl_func_type_t)(void);
typedef void * (*bio_func_type_t)(void);

SSL_CTX * SSL_CTX_new(void *meth);
void SSL_CTX_free(SSL_CTX * ssl_ctx);
SSL * SSL_new(SSL_CTX *ssl_ctx);
int SSL_set_fd(SSL *s, int fd);
int SSL_accept(SSL *ssl);
int SSL_connect(SSL *ssl);
void SSL_free(SSL *ssl);
int SSL_read(SSL *ssl, void *buf, int num);
int SSL_write(SSL *ssl, const void *buf, int num);
int SSL_CTX_use_certificate_file(SSL_CTX *ssl_ctx, const char *file, int type);
int SSL_CTX_use_PrivateKey_file(SSL_CTX *ssl_ctx, const char *file, int type);
int SSL_CTX_use_certificate_ASN1(SSL_CTX *ssl_ctx, int len, const uint8_t *d);
int SSL_CTX_set_session_id_context(SSL_CTX *ctx, const unsigned char *sid_ctx,
                                            unsigned int sid_ctx_len);
int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx);
int SSL_CTX_use_certificate_chain_file(SSL_CTX *ssl_ctx, const char *file);
int SSL_shutdown(SSL *ssl);

/*** get/set session ***/
SSL_SESSION *SSL_get1_session(SSL *ssl);
int SSL_set_session(SSL *ssl, SSL_SESSION *session);
void SSL_SESSION_free(SSL_SESSION *session);
/*** end get/set session ***/

long SSL_CTX_ctrl(SSL_CTX *ctx, int cmd, long larg, void *parg);
void SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                                 int (*verify_callback)(int, void *));

void SSL_CTX_set_verify_depth(SSL_CTX *ctx,int depth);

int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,
                                           const char *CApath);

void *SSL_load_client_CA_file(const char *file);

void SSL_CTX_set_client_CA_list(SSL_CTX *ssl_ctx, void *file);

void SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx, void *cb);

void SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *ctx, void *u);

int SSL_peek(SSL *ssl, void *buf, int num);

void SSL_set_bio(SSL *ssl, void *rbio, void *wbio);

long SSL_get_verify_result(const SSL *ssl);

int SSL_state(SSL *ssl);

/** end of could do better list */

void *SSL_get_peer_certificate(const SSL *ssl);

int SSL_clear(SSL *ssl);


int SSL_CTX_check_private_key(const SSL_CTX *ctx);

int SSL_CTX_set_cipher_list(SSL_CTX *s, const char *str);

int SSL_get_error(const SSL *ssl, int ret);

void SSL_CTX_set_options(SSL_CTX *ssl_ctx, int option);
int SSL_library_init(void );
void SSL_load_error_strings(void );
void ERR_print_errors_fp(FILE *fp);

long SSL_CTX_get_timeout(const SSL_CTX *ssl_ctx);
long SSL_CTX_set_timeout(SSL_CTX *ssl_ctx, long t);
void BIO_printf(FILE *f, const char *format, ...);

void* BIO_s_null(void);
FILE *BIO_new(bio_func_type_t func);

FILE *BIO_new_fp(FILE *stream, int close_flag);
int BIO_free(FILE *a);
