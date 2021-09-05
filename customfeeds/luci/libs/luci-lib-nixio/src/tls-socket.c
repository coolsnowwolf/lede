 /*
 * nixio - Linux I/O library for lua
 *
 *   Copyright (C) 2009 Steven Barth <steven@midlink.org>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#include "nixio-tls.h"
#include <string.h>
#include <stdlib.h>

static int nixio__tls_sock_perror(lua_State *L, SSL *sock, int code) {
	lua_pushnil(L);
	lua_pushinteger(L, SSL_get_error(sock, code));
	return 2;
}

static int nixio__tls_sock_pstatus(lua_State *L, SSL *sock, int code) {
	if (code > 0) {
		lua_pushboolean(L, 1);
		return 1;
	} else {
		return nixio__tls_sock_perror(L, sock, code);
	}
}

static SSL* nixio__checktlssock(lua_State *L) {
	if (lua_istable(L, 1)) {
		lua_getfield(L, 1, "connection");
		lua_replace(L, 1);
	}
	nixio_tls_sock *sock = luaL_checkudata(L, 1, NIXIO_TLS_SOCK_META);
	luaL_argcheck(L, sock->socket, 1, "invalid context");
	return sock->socket;
}

#ifndef WITH_AXTLS
#define nixio_tls__check_connected(L) ;

#define nixio_tls__set_connected(L, val) ;
#else
#define nixio_tls__check_connected(L) \
	nixio_tls_sock *ctsock = luaL_checkudata(L, 1, NIXIO_TLS_SOCK_META);	\
	if (!ctsock->connected) {	\
		lua_pushnil(L);			\
		lua_pushinteger(L, 1);	\
		return 2;				\
	}

#define nixio_tls__set_connected(L, val) \
((nixio_tls_sock*)luaL_checkudata(L, 1, NIXIO_TLS_SOCK_META))->connected = val;
#endif /* WITH_AXTLS */

static int nixio_tls_sock_recv(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	nixio_tls__check_connected(L);
	uint req = luaL_checkinteger(L, 2);

	luaL_argcheck(L, req >= 0, 2, "out of range");

	/* We limit the readsize to NIXIO_BUFFERSIZE */
	req = (req > NIXIO_BUFFERSIZE) ? NIXIO_BUFFERSIZE : req;

#ifndef WITH_AXTLS

	char buffer[NIXIO_BUFFERSIZE];
	int readc = SSL_read(sock, buffer, req);

	if (readc < 0) {
		return nixio__tls_sock_pstatus(L, sock, readc);
	} else {
		lua_pushlstring(L, buffer, readc);
		return 1;
	}

#else

	if (!req) {
		lua_pushliteral(L, "");
		return 1;
	}

	nixio_tls_sock *t = lua_touserdata(L, 1);

	/* AXTLS doesn't handle buffering for us, so we have to hack around*/
	if (req < t->pbufsiz) {
		lua_pushlstring(L, t->pbufpos, req);
		t->pbufpos += req;
		t->pbufsiz -= req;
		return 1;
	} else {
		uint8_t *axbuf;
		int axread;

		/* while handshake pending */
		while ((axread = ssl_read(sock, &axbuf)) == SSL_OK);

		if (t->pbufsiz) {
			lua_pushlstring(L, t->pbufpos, t->pbufsiz);
		}

		if (axread < 0) {
			/* There is an error */
			free(t->pbuffer);
			t->pbuffer = t->pbufpos = NULL;

			if (axread != SSL_ERROR_CONN_LOST) {
				t->pbufsiz = 0;
				return nixio__tls_sock_perror(L, sock, axread);
			} else {
				if (!t->pbufsiz) {
					lua_pushliteral(L, "");
				} else {
					t->pbufsiz = 0;
				}
			}
		} else {
			int stillwant = req - t->pbufsiz;
			if (stillwant < axread) {
				/* we got more data than we need */
				lua_pushlstring(L, (char *)axbuf, stillwant);
				if(t->pbufsiz) {
					lua_concat(L, 2);
				}

				/* remaining data goes into the buffer */
				t->pbufpos = t->pbuffer;
				t->pbufsiz = axread - stillwant;
				t->pbuffer = realloc(t->pbuffer, t->pbufsiz);
				if (!t->pbuffer) {
					free(t->pbufpos);
					t->pbufpos = NULL;
					t->pbufsiz = 0;
					return luaL_error(L, "out of memory");
				}

				t->pbufpos = t->pbuffer;
				memcpy(t->pbufpos, axbuf + stillwant, t->pbufsiz);
			} else {
				lua_pushlstring(L, (char *)axbuf, axread);
				if(t->pbufsiz) {
					lua_concat(L, 2);
				}

				/* free buffer */
				free(t->pbuffer);
				t->pbuffer = t->pbufpos = NULL;
				t->pbufsiz = 0;
			}
		}
		return 1;
	}

#endif /* WITH_AXTLS */

}

static int nixio_tls_sock_send(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	nixio_tls__check_connected(L);
	size_t len;
	ssize_t sent;
	const char *data = luaL_checklstring(L, 2, &len);

	if (lua_gettop(L) > 2) {
		int offset = luaL_optint(L, 3, 0);
		if (offset) {
			if (offset < len) {
				data += offset;
				len -= offset;
			} else {
				len = 0;
			}
		}

		unsigned int wlen = luaL_optint(L, 4, len);
		if (wlen < len) {
			len = wlen;
		}
	}

	sent = SSL_write(sock, data, len);
	if (sent > 0) {
		lua_pushinteger(L, sent);
		return 1;
	} else {
		return nixio__tls_sock_pstatus(L, sock, sent);
	}
}

static int nixio_tls_sock_accept(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	const int stat = SSL_accept(sock);
	nixio_tls__set_connected(L, stat == 1);
	return nixio__tls_sock_pstatus(L, sock, stat);
}

static int nixio_tls_sock_connect(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	const int stat = SSL_connect(sock);
	nixio_tls__set_connected(L, stat == 1);
	return nixio__tls_sock_pstatus(L, sock, stat);
}

static int nixio_tls_sock_shutdown(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	nixio_tls__set_connected(L, 0);
	return nixio__tls_sock_pstatus(L, sock, SSL_shutdown(sock));
}

static int nixio_tls_sock__gc(lua_State *L) {
	nixio_tls_sock *sock = luaL_checkudata(L, 1, NIXIO_TLS_SOCK_META);
	if (sock->socket) {
		SSL_free(sock->socket);
		sock->socket = NULL;
#ifdef WITH_AXTLS
		free(sock->pbuffer);
#endif
	}
	return 0;
}

static int nixio_tls_sock__tostring(lua_State *L) {
	SSL *sock = nixio__checktlssock(L);
	lua_pushfstring(L, "nixio TLS connection: %p", sock);
	return 1;
}


/* ctx function table */
static const luaL_reg M[] = {
	{"recv", 		nixio_tls_sock_recv},
	{"send", 		nixio_tls_sock_send},
	{"read", 		nixio_tls_sock_recv},
	{"write", 		nixio_tls_sock_send},
	{"accept",	 	nixio_tls_sock_accept},
	{"connect", 	nixio_tls_sock_connect},
	{"shutdown", 	nixio_tls_sock_shutdown},
	{"__gc",		nixio_tls_sock__gc},
	{"__tostring",	nixio_tls_sock__tostring},
	{NULL,			NULL}
};


void nixio_open_tls_socket(lua_State *L) {
	/* create socket metatable */
	luaL_newmetatable(L, NIXIO_TLS_SOCK_META);
	luaL_register(L, NULL, M);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, -2, "meta_tls_socket");
}
