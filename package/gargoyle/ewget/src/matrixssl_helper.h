/*
 * MatrixSSL helper functions
 *
 * Copyright (C) 2008 Eric Bishop <eric@gargoyle-router.com>
 * Copyright (C) 2005 Nicolas Thill <nthill@free.fr>
 *
 * Updated March 2008 by Eric Bishop <eric@gargoyle-router.com>
 *    - fixed bug to allow writing of more than 16k of data 
 * 	(Even Bill Gates offered 640K! How could a bug like that last this long???)
 *    - fixed bug caused by ssl->outBufferCount never being initialized
 *    - Initialized ssl->status & ssl->partial, since these weren't initialized either
 *    - Added functions SSL_peek & SSL_connect to allow use in a client
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Portions borrowed from MatrixSSL example code
 *
 */


#ifndef __MATRIXSSL_HELPER_H__
#define __MATRIXSSL_HELPER_H__

#include <matrixSsl/matrixSsl.h>

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif


typedef struct __SSL {
  int fd;
  ssl_t *ssl;
  sslKeys_t *keys;
  sslBuf_t insock, outsock, inbuf;
  int outBufferCount;
  int partial;
  int status;
} SSL;


SSL* SSL_new(sslKeys_t *keys, int flags);
void SSL_set_fd(SSL *ssl, int fd);
int SSL_accept(SSL *ssl);
int SSL_read(SSL *ssl, char *b, int len);
int SSL_write(SSL *ssl, char *buf, int len);
int SSL_peek(SSL *ssl, char *buf, int len);
void SSL_free(SSL *ssl);
int SSL_connect(SSL *ssl, int (*certValidator)(sslCertInfo_t *t, void *arg), void *certValidatorArgs);

#endif /* __MATRIXSSL_HELPER_H__ */
