/*
 * MatrixSSL helper functions
 *
 * Copyright (C) 2008 Bill Lewis
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
 * Updated December 2008 by Bill Lewis
 * 	- fixed bug in SSL_peek
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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "matrixssl_helper.h"

#define SSL_SOCKET_EOF  0x0001
#define SSL_SOCKET_CLOSE_NOTIFY  0x0002
#define		SOCKET_ERROR -1

#define socketAssert(C) if (C) ; else {printf("%s:%d sslAssert(%s)\n",\
						__FILE__, __LINE__, #C);  }
#ifndef min
#define min(a, b)  ( (a) < (b) ) ? (a) : (b)
#endif /* min */

static int _ssl_read(SSL *ssl, char *buf, int len);
static int _ssl_write(SSL *ssl, char *buf, int len);
static void _ssl_setSocketBlock(int fd);
static void _ssl_setSocketNonblock(int fd);
static void _ssl_closeSocket(int fd);
static int _ssl_doHandshake(SSL*);
static int _psSocketWrite(int sock, sslBuf_t *out);
int _always_true_validator(sslCertInfo_t *t, void *arg);

int _always_true_validator(sslCertInfo_t *t, void *arg)
{
	return 1;
}

SSL * SSL_new(sslKeys_t *keys, int flags)
{
	SSL * ssl;
	ssl = (SSL *)malloc(sizeof(SSL));
	
	if (!ssl) return 0;
	

	ssl->keys = keys;
	if ( matrixSslNewSession(&(ssl->ssl), ssl->keys, NULL, flags) < 0 ) {}
	
	ssl->insock.size = 1024;
	ssl->insock.buf = ssl->insock.start = ssl->insock.end =
		(unsigned char *)malloc(ssl->insock.size);
	
	ssl->outsock.size = 1024;
	ssl->outsock.buf = ssl->outsock.start = ssl->outsock.end = 
		(unsigned char *)malloc(ssl->outsock.size);
	
	ssl->inbuf.size = 0;
	ssl->inbuf.buf = ssl->inbuf.start = ssl->inbuf.end = NULL;

	ssl->fd = -1;
	ssl->outBufferCount = 0;
	ssl->status = 0;
	ssl->partial = 0;

	
	return ssl;
}


int SSL_accept(SSL *ssl) {

	char buf[1024];
	int  rc;
	
readMore:
	rc = _ssl_read(ssl, buf, sizeof(buf));
	if (rc == 0) {
		if (ssl->status == SSL_SOCKET_EOF || ssl->status == SSL_SOCKET_CLOSE_NOTIFY) {
			SSL_free(ssl);
			return -1;
		}
		if (matrixSslHandshakeIsComplete(ssl->ssl) == 0) {
			goto readMore;
		}
	} else if (rc > 0) {
		return 0;
	} else {
		SSL_free(ssl);
		return -1;
	}
	
	return 1;
}


void SSL_set_fd(SSL *ssl, int fd) {
	ssl->fd = fd;
}


int SSL_read(SSL *ssl, char *buf, int len) {
	int rc;
readMore:
	rc = _ssl_read(ssl, buf, len);
	if (rc <= 0) {
		if (rc < 0 || ssl->status == SSL_SOCKET_EOF || ssl->status == SSL_SOCKET_CLOSE_NOTIFY) {
			_ssl_closeSocket(ssl->fd);
			return rc;
		}
		goto readMore;
	}
	return rc;
}


int SSL_write(SSL *ssl, char *buf, int len)
{
	int rc = 0;
	int rcSum = 0;
	int offset = 0;
	int maxWrite = SSL_MAX_BUF_SIZE;
	maxWrite = maxWrite/2;
	

	if(len > maxWrite)
	{
		while(offset < len && rc >= 0)
		{
			rc = 0;
			while(rc == 0)
			{
				if(len - offset < maxWrite)
				{
					//printf("calling low-level write with len-offset = %d\n",len-offset);
					rc = _ssl_write(ssl, (char*)(buf+offset), len-offset);
				}
				else
				{
					//printf("calling low-level write with maxWrite = %d\n", maxWrite);
					rc = _ssl_write(ssl, (char*)(buf+offset), maxWrite);
					
				}
			}
			//printf("multiple write : rc = %d\n", rc);
			rcSum = rcSum + rc;
			offset = offset + maxWrite;
		}
		if(rc < 0)
		{
			rcSum = rc;
		}
	}
	else
	{
		while (rc == 0)
		{
			rc = _ssl_write(ssl, buf, len);
			//printf("single write : rc = %d\n", rc);
		}
		rcSum = rc;
	}
	return rcSum;
}

int SSL_peek(SSL *ssl, char *buf, int bufsize)
{
	int read_bytes;
	unsigned long available = (unsigned long)(ssl->inbuf.end) - (unsigned long)(ssl->inbuf.start);
	if(available > 0)
	{
		read_bytes = available >= bufsize ? bufsize : (int)available;
		memcpy(buf, ssl->inbuf.start, read_bytes);
	}
	else
	{	
		char* newBuf = (char *)malloc(bufsize);
		read_bytes = SSL_read(ssl, newBuf, bufsize);
		if(read_bytes > 0)
		{
			memcpy(buf, newBuf, read_bytes);
			ssl->inbuf.start -= read_bytes;
		}
		free(newBuf);
	}
	return read_bytes;	
}




void SSL_free(SSL * ssl)
{
	matrixSslDeleteSession(ssl->ssl);
	if (ssl->insock.buf) {
		free(ssl->insock.buf);
	}
	if (ssl->outsock.buf) {
		free(ssl->outsock.buf);
	}
	if (ssl->inbuf.buf) {
		free(ssl->inbuf.buf);
	}
	free(ssl);
}



/******************************************************************************/
/*
	Client side.  Make a socket connection and go through the SSL handshake
	phase in blocking mode.  The second parameter is an optional function
	callback for user-level certificate validation which should be NULL if
	not needed. Third parameter is an optional second argument for the validation
	function.

*/
int SSL_connect(SSL *ssl, int (*certValidator)(sslCertInfo_t *t, void *arg), void *certValidatorArgs)
{
	if (!(ssl->ssl))
	{
		SSL_free(ssl);
		return -1;
	}
	
	if(certValidator != NULL)
	{
		matrixSslSetCertValidator(ssl->ssl, certValidator, certValidatorArgs);
	}
	else
	{
		matrixSslSetCertValidator(ssl->ssl, _always_true_validator, certValidatorArgs);
	}
	int ret = _ssl_doHandshake(ssl);
	
	return ret;
}

/******************************************************************************/
/*
	Construct the initial HELLO message to send to the server and initiate
	the SSL handshake.  Can be used in the re-handshake scenario as well.
*/
int _ssl_doHandshake(SSL *ssl)
{
	char	buf[1024];
	int		err, rc;

/*
	MatrixSSL doesn't provide buffers for data internally.  Define them
	here to support buffered reading and writing for non-blocking sockets.
	Although it causes quite a bit more work, we support dynamically growing
	the buffers as needed.  Alternately, we could define 16K buffers here
	and not worry about growing them.
*/


	short cipherSuite = 0;
	err = matrixSslEncodeClientHello(ssl->ssl, &(ssl->outsock), cipherSuite);
	
	if (err < 0) {
		socketAssert(err < 0);
		return -1;
	}
	
/*
	Send the hello with a blocking write
*/
	err = _psSocketWrite(ssl->fd, &(ssl->outsock));
	if (err < 0) {
		fprintf(stdout, "Error in socketWrite\n");
		return -1;
	}
	ssl->outsock.start = ssl->outsock.end = ssl->outsock.buf;


/*
	Call _ssl_read to work through the handshake.  Not actually expecting
	data back, so the finished case is simply when the handshake is
	complete.
*/
readMore:
	rc = _ssl_read(ssl, buf, 1024);
	
	/*
	Reading handshake records should always return 0 bytes, we aren't
	expecting any data yet.
	*/
	if(rc > 0 || (rc == 0 && matrixSslHandshakeIsComplete(ssl->ssl) == 0))
	{
		goto readMore;
	}
	if(rc < 0)
	{
		return -1;
	}

	return 0;


}


/******************************************************************************/
/*
	Perform a blocking write of data to a socket
*/
int _psSocketWrite(int sock, sslBuf_t *out)
{
	unsigned char	*s;
	int				bytes;

	s = out->start;
	while (out->start < out->end) {
		bytes = send(sock, out->start, (int)(out->end - out->start), MSG_NOSIGNAL);
		if (bytes == SOCKET_ERROR) {
			return -1;
		}
		out->start += bytes;
	}
	return (int)(out->start - s);
}






static void _ssl_setSocketBlock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) & ~O_NONBLOCK);
	fcntl(fd, F_SETFD, FD_CLOEXEC);
}


static void _ssl_setSocketNonblock(int fd)
{
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}


static void _ssl_closeSocket(int fd)
{
	char buf[32];

	if (fd != -1) {
		_ssl_setSocketNonblock(fd);
		if (shutdown(fd, 1) >= 0) {
			while (recv(fd, buf, sizeof(buf), 0) > 0);
		}
		close(fd);
	}
}


static int _ssl_read(SSL *ssl, char *buf, int len)
{
	int bytes, rc, remaining;
	unsigned char error, alertLevel, alertDescription, performRead;

	ssl->status = 0;

	if (ssl->ssl == NULL || len <= 0) {
		return -1;
	}
/*
	If inbuf is valid, then we have previously decoded data that must be
	returned, return as much as possible.  Once all buffered data is
	returned, free the inbuf.
*/
	if (ssl->inbuf.buf) {
		if (ssl->inbuf.start < ssl->inbuf.end) {
			remaining = (int)(ssl->inbuf.end - ssl->inbuf.start);
			bytes = (int)min(len, remaining);
			memcpy(buf, ssl->inbuf.start, bytes);
			ssl->inbuf.start += bytes;
			return bytes;
		}
		free(ssl->inbuf.buf);
		ssl->inbuf.buf = NULL;
	}
/*
	Pack the buffered socket data (if any) so that start is at zero.
*/
	if (ssl->insock.buf < ssl->insock.start) {
		if (ssl->insock.start == ssl->insock.end) {
			ssl->insock.start = ssl->insock.end = ssl->insock.buf;
		} else {
			memmove(ssl->insock.buf, ssl->insock.start, ssl->insock.end - ssl->insock.start);
			ssl->insock.end -= (ssl->insock.start - ssl->insock.buf);
			ssl->insock.start = ssl->insock.buf;
		}
	}
/*
	Read up to as many bytes as there are remaining in the buffer.  We could
	Have encrypted data already cached in conn->insock, but might as well read more
	if we can.
*/
	performRead = 0;
readMore:
	if (ssl->insock.end == ssl->insock.start || performRead) {
		performRead = 1;
		bytes = recv(ssl->fd, (char *)ssl->insock.end, 
			(int)((ssl->insock.buf + ssl->insock.size) - ssl->insock.end), MSG_NOSIGNAL);
		if (bytes == -1) {
			ssl->status = errno;
			return -1;
		}
		if (bytes == 0) {
			ssl->status = SSL_SOCKET_EOF;
			return 0;
		}
		ssl->insock.end += bytes;
	}
/*
	Define a temporary sslBuf
*/
	ssl->inbuf.start = ssl->inbuf.end = ssl->inbuf.buf = (unsigned char *)malloc(len);
	ssl->inbuf.size = len;
/*
	Decode the data we just read from the socket
*/
decodeMore:
	error = 0;
	alertLevel = 0;
	alertDescription = 0;

	rc = matrixSslDecode(ssl->ssl, &ssl->insock, &ssl->inbuf, &error, &alertLevel, 
		&alertDescription);
	switch (rc) {
/*
	Successfully decoded a record that did not return data or require a response.
*/
	case SSL_SUCCESS:
		return 0;
/*
	Successfully decoded an application data record, and placed in tmp buf
*/
	case SSL_PROCESS_DATA:
/*
		Copy as much as we can from the temp buffer into the caller's buffer
		and leave the remainder in conn->inbuf until the next call to read
		It is possible that len > data in buffer if the encoded record
		was longer than len, but the decoded record isn't!
*/
		rc = (int)(ssl->inbuf.end - ssl->inbuf.start);
		rc = min(rc, len);
		memcpy(buf, ssl->inbuf.start, rc);
		ssl->inbuf.start += rc;
		return rc;
/*
	We've decoded a record that requires a response into tmp
	If there is no data to be flushed in the out buffer, we can write out
	the contents of the tmp buffer.  Otherwise, we need to append the data 
	to the outgoing data buffer and flush it out.
*/
	case SSL_SEND_RESPONSE:
		bytes = send(ssl->fd, (char *)ssl->inbuf.start, 
			(int)(ssl->inbuf.end - ssl->inbuf.start), MSG_NOSIGNAL);
		if (bytes == -1) {
			ssl->status = errno;
			if (ssl->status != EAGAIN) {
				goto readError;
			}
			ssl->status = 0;
		}
		ssl->inbuf.start += bytes;
		if (ssl->inbuf.start < ssl->inbuf.end) {
/*
			This must be a non-blocking socket since it didn't all get sent
			out and there was no error.  We want to finish the send here
			simply because we are likely in the SSL handshake.
*/
			_ssl_setSocketBlock(ssl->fd);
			bytes = send(ssl->fd, (char *)ssl->inbuf.start, 
				(int)(ssl->inbuf.end - ssl->inbuf.start), MSG_NOSIGNAL);
			if (bytes == -1) {
				ssl->status = errno;
				goto readError;
			}
			ssl->inbuf.start += bytes;
/*
			Can safely set back to non-blocking because we wouldn't
			have got here if this socket wasn't non-blocking to begin with.
*/
			_ssl_setSocketNonblock(ssl->fd);
		}
		ssl->inbuf.start = ssl->inbuf.end = ssl->inbuf.buf;
		return 0;
/*
	There was an error decoding the data, or encoding the out buffer.
	There may be a response data in the out buffer, so try to send.
	We try a single hail-mary send of the data, and then close the socket.
	Since we're closing on error, we don't worry too much about a clean flush.
*/
	case SSL_ERROR:
		if (ssl->inbuf.start < ssl->inbuf.end) {
			_ssl_setSocketNonblock(ssl->fd);
			bytes = send(ssl->fd, (char *)ssl->inbuf.start, 
				(int)(ssl->inbuf.end - ssl->inbuf.start), MSG_NOSIGNAL);
		}
		goto readError;
/*
	We've decoded an alert.  The level and description passed into
	matrixSslDecode are filled in with the specifics.
*/
	case SSL_ALERT:
		if (alertDescription == SSL_ALERT_CLOSE_NOTIFY) {
			ssl->status = SSL_SOCKET_CLOSE_NOTIFY;
			goto readZero;
		}
		goto readError;
/*
	We have a partial record, we need to read more data off the socket.
	If we have a completely full conn->insock buffer, we'll need to grow it
	here so that we CAN read more data when called the next time.
*/
	case SSL_PARTIAL:
		if (ssl->insock.start == ssl->insock.buf && ssl->insock.end == 
				(ssl->insock.buf + ssl->insock.size)) {
			if (ssl->insock.size > SSL_MAX_BUF_SIZE) {
				goto readError;
			}
			ssl->insock.size *= 2;
			ssl->insock.start = ssl->insock.buf = 
				(unsigned char *)realloc(ssl->insock.buf, ssl->insock.size);
			ssl->insock.end = ssl->insock.buf + (ssl->insock.size / 2);
		}
		if (!performRead) {
			performRead = 1;
			free(ssl->inbuf.buf);
			ssl->inbuf.buf = NULL;
			goto readMore;
		} else {
			goto readZero;
		}
/*
	The out buffer is too small to fit the decoded or response
	data.  Increase the size of the buffer and call decode again
*/
	case SSL_FULL:
		ssl->inbuf.size *= 2;
		if (ssl->inbuf.buf != (unsigned char*)buf) {
			free(ssl->inbuf.buf);
			ssl->inbuf.buf = NULL;
		}
		ssl->inbuf.start = ssl->inbuf.end = ssl->inbuf.buf = 
			(unsigned char *)malloc(ssl->inbuf.size);
		goto decodeMore;
	}
/*
	We consolidated some of the returns here because we must ensure
	that conn->inbuf is cleared if pointing at caller's buffer, otherwise
	it will be freed later on.
*/
readZero:
	if (ssl->inbuf.buf == (unsigned char*)buf) {
		ssl->inbuf.buf = NULL;
	}
	return 0;
readError:
	if (ssl->inbuf.buf == (unsigned char*)buf) {
		ssl->inbuf.buf = NULL;
	}
	return -1;
}


int _ssl_write(SSL *ssl, char *buf, int len)
{
	int		rc;

	ssl->status = 0;
/*
	Pack the buffered socket data (if any) so that start is at zero.
*/
	if (ssl->outsock.buf < ssl->outsock.start) {
		if (ssl->outsock.start == ssl->outsock.end) {
			ssl->outsock.start = ssl->outsock.end = ssl->outsock.buf;
		} else {
			memmove(ssl->outsock.buf, ssl->outsock.start, ssl->outsock.end - ssl->outsock.start);
			ssl->outsock.end -= (ssl->outsock.start - ssl->outsock.buf);
			ssl->outsock.start = ssl->outsock.buf;
		}
	}
/*
	If there is buffered output data, the caller must be trying to
	send the same amount of data as last time.  We don't support 
	sending additional data until the original buffered request has
	been completely sent.
*/
	if (ssl->outBufferCount > 0 && len != ssl->outBufferCount) {
		return -1;
	}
/*
	If we don't have buffered data, encode the caller's data
*/
	if (ssl->outBufferCount == 0) {
retryEncode:
		rc = matrixSslEncode(ssl->ssl, (unsigned char *)buf, len, &ssl->outsock);
		switch (rc) {
		case SSL_ERROR:
			return -1;
		case SSL_FULL:
			
			if (ssl->outsock.size > SSL_MAX_BUF_SIZE) {
				return -1;
			}
			ssl->outsock.size *= 2;
			ssl->outsock.buf = 
				(unsigned char *)realloc(ssl->outsock.buf, ssl->outsock.size);
			ssl->outsock.end = ssl->outsock.buf + (ssl->outsock.end - ssl->outsock.start);
			ssl->outsock.start = ssl->outsock.buf;
			goto retryEncode;
		}
	}
/*
	We've got data to send.
*/
	rc = send(ssl->fd, (char *)ssl->outsock.start, 
		(int)(ssl->outsock.end - ssl->outsock.start), MSG_NOSIGNAL);
	if (rc == -1) {
		ssl->status = errno;
		return -1;
	}
	ssl->outsock.start += rc;
/*
	If we wrote it all return the length, otherwise remember the number of
	bytes passed in, and return 0 to be called again later.
*/
	if (ssl->outsock.start == ssl->outsock.end) {
		ssl->outBufferCount = 0;
		return len;
	}
	ssl->outBufferCount = len;
	return 0;
}

