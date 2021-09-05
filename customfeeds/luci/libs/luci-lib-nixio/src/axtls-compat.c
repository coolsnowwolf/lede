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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "ssl.h"

static char *key_password = NULL;

void *SSLv23_server_method(void) { return NULL; }
void *SSLv3_server_method(void) { return NULL; }
void *TLSv1_server_method(void) { return NULL; }
void *SSLv23_client_method(void) { return NULL; }
void *SSLv3_client_method(void) { return NULL; }
void *TLSv1_client_method(void) { return NULL; }
void *SSLv23_method(void) { return NULL; }
void *TLSv1_method(void) { return NULL; }

SSL_CTX * SSL_CTX_new(void *meth)
{
    SSL_CTX *ssl_ctx = ssl_ctx_new(SSL_SERVER_VERIFY_LATER, 5);
    return ssl_ctx;
}

void SSL_CTX_free(SSL_CTX * ssl_ctx)
{
    ssl_ctx_free(ssl_ctx);
}

SSL * SSL_new(SSL_CTX *ssl_ctx)
{
    SSL *ssl;

    ssl = ssl_new(ssl_ctx, -1);        /* fd is set later */

    return ssl;
}

int SSL_set_fd(SSL *s, int fd)
{
    s->client_fd = fd;
    return 1;   /* always succeeds */
}

int SSL_accept(SSL *ssl)
{
	ssl->next_state = HS_CLIENT_HELLO;
    while (ssl_read(ssl, NULL) == SSL_OK)
    {
        if (ssl->next_state == HS_CLIENT_HELLO)
            return 1;   /* we're done */
    }

    return -1;
}

int SSL_connect(SSL *ssl)
{
	SET_SSL_FLAG(SSL_IS_CLIENT);
	int stat = do_client_connect(ssl);
	ssl_display_error(stat);
    return  (stat == SSL_OK) ? 1 : -1;
}

void SSL_free(SSL *ssl)
{
    ssl_free(ssl);
}

int SSL_read(SSL *ssl, void *buf, int num)
{
    uint8_t *read_buf;
    int ret;

    while ((ret = ssl_read(ssl, &read_buf)) == SSL_OK);

    if (ret > SSL_OK)
    {
        memcpy(buf, read_buf, ret > num ? num : ret);
    }

    return ret;
}

int SSL_write(SSL *ssl, const void *buf, int num)
{
    return ssl_write(ssl, buf, num);
}

int SSL_CTX_use_certificate_file(SSL_CTX *ssl_ctx, const char *file, int type)
{
    return (ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CERT, file, NULL) == SSL_OK);
}

int SSL_CTX_use_PrivateKey_file(SSL_CTX *ssl_ctx, const char *file, int type)
{
    return (ssl_obj_load(ssl_ctx, SSL_OBJ_RSA_KEY, file, key_password) == SSL_OK);
}

int SSL_CTX_use_certificate_ASN1(SSL_CTX *ssl_ctx, int len, const uint8_t *d)
{
    return (ssl_obj_memory_load(ssl_ctx,
                        SSL_OBJ_X509_CERT, d, len, NULL) == SSL_OK);
}

int SSL_CTX_set_session_id_context(SSL_CTX *ctx, const unsigned char *sid_ctx,
                                            unsigned int sid_ctx_len)
{
    return 1;
}

int SSL_CTX_set_default_verify_paths(SSL_CTX *ctx)
{
    return 1;
}

int SSL_CTX_use_certificate_chain_file(SSL_CTX *ssl_ctx, const char *file)
{
    return (ssl_obj_load(ssl_ctx,
                        SSL_OBJ_X509_CERT, file, NULL) == SSL_OK);
}

int SSL_shutdown(SSL *ssl)
{
    return 1;
}

/*** get/set session ***/
SSL_SESSION *SSL_get1_session(SSL *ssl)
{
    return (SSL_SESSION *)ssl_get_session_id(ssl); /* note: wrong cast */
}

int SSL_set_session(SSL *ssl, SSL_SESSION *session)
{
    memcpy(ssl->session_id, (uint8_t *)session, SSL_SESSION_ID_SIZE);
    return 1;
}

void SSL_SESSION_free(SSL_SESSION *session) { }
/*** end get/set session ***/

long SSL_CTX_ctrl(SSL_CTX *ctx, int cmd, long larg, void *parg)
{
    return 0;
}

void SSL_CTX_set_verify(SSL_CTX *ctx, int mode,
                                 int (*verify_callback)(int, void *)) {
	if (mode & SSL_VERIFY_PEER) {
		ctx->options &= ~SSL_SERVER_VERIFY_LATER;
		ctx->options |= SSL_CLIENT_AUTHENTICATION;
	} else {
		ctx->options |= SSL_SERVER_VERIFY_LATER;
		ctx->options &= ~SSL_CLIENT_AUTHENTICATION;
	}
}

void SSL_CTX_set_verify_depth(SSL_CTX *ctx,int depth) { }

int SSL_CTX_load_verify_locations(SSL_CTX *ctx, const char *CAfile,
                                           const char *CApath)
{
    return 1;
}

void *SSL_load_client_CA_file(const char *file)
{
    return (void *)file;
}

void SSL_CTX_set_client_CA_list(SSL_CTX *ssl_ctx, void *file)
{

    ssl_obj_load(ssl_ctx, SSL_OBJ_X509_CERT, (const char *)file, NULL);
}

void SSL_CTX_set_default_passwd_cb(SSL_CTX *ctx, void *cb) { }

void SSL_CTX_set_default_passwd_cb_userdata(SSL_CTX *ctx, void *u)
{
    key_password = (char *)u;
}

int SSL_peek(SSL *ssl, void *buf, int num)
{
    memcpy(buf, ssl->bm_data, num);
    return num;
}

void SSL_set_bio(SSL *ssl, void *rbio, void *wbio) { }

long SSL_get_verify_result(const SSL *ssl)
{
    return ssl_handshake_status(ssl);
}

int SSL_state(SSL *ssl)
{
    return 0x03; // ok state
}

/** end of could do better list */

void *SSL_get_peer_certificate(const SSL *ssl)
{
    return &ssl->ssl_ctx->certs[0];
}

int SSL_clear(SSL *ssl)
{
    return 1;
}


int SSL_CTX_check_private_key(const SSL_CTX *ctx)
{
    return 1;
}

int SSL_CTX_set_cipher_list(SSL_CTX *s, const char *str)
{
    return 1;
}

int SSL_get_error(const SSL *ssl, int ret)
{
    ssl_display_error(ret);
    return ret;   /* TODO: return proper return code */
}

void SSL_CTX_set_options(SSL_CTX *ssl_ctx, int option) {}
int SSL_library_init(void ) { return 1; }
void SSL_load_error_strings(void ) {}
void ERR_print_errors_fp(FILE *fp) {}

long SSL_CTX_get_timeout(const SSL_CTX *ssl_ctx) {
                            return CONFIG_SSL_EXPIRY_TIME*3600; }
long SSL_CTX_set_timeout(SSL_CTX *ssl_ctx, long t) {
                            return SSL_CTX_get_timeout(ssl_ctx); }
void BIO_printf(FILE *f, const char *format, ...)
{
    va_list(ap);
    va_start(ap, format);
    vfprintf(f, format, ap);
    va_end(ap);
}

void* BIO_s_null(void) { return NULL; }
FILE *BIO_new(bio_func_type_t func)
{
    if (func == BIO_s_null)
        return fopen("/dev/null", "r");
    else
        return NULL;
}

FILE *BIO_new_fp(FILE *stream, int close_flag) { return stream; }
int BIO_free(FILE *a) { if (a != stdout && a != stderr) fclose(a); return 1; }
