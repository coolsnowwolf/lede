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

#include "nixio.h"
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>


/**
 * send() / sendto() helper
 */
static int nixio_sock__sendto(lua_State *L, int to) {
	nixio_sock *sock = nixio__checksock(L);
	struct sockaddr_storage addr_in;
#ifndef __WINNT__
	struct sockaddr_un addr_un;
#endif
	struct sockaddr *addr = NULL;
	socklen_t alen = 0;
	int argoff = 2;

	if (to) {
		argoff += 2;
		if (sock->domain == AF_INET || sock->domain == AF_INET6) {
			const char *address = luaL_checkstring(L, 3);
			addr = (struct sockaddr*)&addr_in;
			alen = sizeof(addr_in);

			nixio_addr naddr;
			memset(&naddr, 0, sizeof(naddr));
			strncpy(naddr.host, address, sizeof(naddr.host) - 1);
			naddr.port = (uint16_t)luaL_checkinteger(L, 4);
			naddr.family = sock->domain;

			if (nixio__addr_write(&naddr, addr)) {
				return nixio__perror_s(L);
			}
		}

#ifndef __WINNT__
		else if (sock->domain == AF_UNIX) {
			size_t pathlen;
			const char *path = luaL_checklstring(L, 3, &pathlen);

			addr_un.sun_family = AF_UNIX;
			luaL_argcheck(L, pathlen <= sizeof(addr_un.sun_path), 3, "out of range");
			memcpy(addr_un.sun_path, path, pathlen);

			addr = (struct sockaddr*)&addr_un;
			alen = sizeof(sa_family_t) + pathlen;
		}
#endif
	}

	size_t len;
	ssize_t sent;
	const char *data = luaL_checklstring(L, 2, &len);

	if (lua_gettop(L) > argoff) {
		int offset = luaL_optint(L, argoff + 1, 0);
		if (offset) {
			if (offset < len) {
				data += offset;
				len -= offset;
			} else {
				len = 0;
			}
		}

		unsigned int wlen = luaL_optint(L, argoff + 2, len);
		if (wlen < len) {
			len = wlen;
		}
	}

	do {
		sent = sendto(sock->fd, data, len, 0, addr, alen);
	} while(sent == -1 && errno == EINTR);
	if (sent >= 0) {
		lua_pushinteger(L, sent);
		return 1;
	} else {
		return nixio__perror_s(L);
	}
}

/**
 * send(data)
 */
static int nixio_sock_send(lua_State *L) {
	return nixio_sock__sendto(L, 0);
}

/**
 * sendto(data, address, port)
 */
static int nixio_sock_sendto(lua_State *L) {
	return nixio_sock__sendto(L, 1);
}


/**
 * recv() / recvfrom() helper
 */
static int nixio_sock__recvfrom(lua_State *L, int from) {
	nixio_sock *sock = nixio__checksock(L);
	char buffer[NIXIO_BUFFERSIZE];
	struct sockaddr_storage addr_in;
#ifndef __WINNT__
	struct sockaddr_un addr_un;
#endif
	struct sockaddr *addr = NULL;
	socklen_t alen = 0;
	uint req = luaL_checkinteger(L, 2);
	int readc;

	if (sock->domain == AF_INET || sock->domain == AF_INET6) {
		addr = (from) ? (struct sockaddr*)&addr_in : NULL;
		alen = (from) ? sizeof(addr_in) : 0;
	}
#ifndef __WINNT__
	else if (sock->domain == AF_UNIX) {
		addr = (from) ? (struct sockaddr*)&addr_un : NULL;
		alen = (from) ? sizeof(addr_un) : 0;
	}
#endif

	/* We limit the readsize to NIXIO_BUFFERSIZE */
	req = (req > NIXIO_BUFFERSIZE) ? NIXIO_BUFFERSIZE : req;

	do {
		readc = recvfrom(sock->fd, buffer, req, 0, addr, &alen);
	} while (readc == -1 && errno == EINTR);

#ifdef __WINNT__
	if (readc < 0) {
		int e = WSAGetLastError();
		if (e == WSAECONNRESET || e == WSAECONNABORTED || e == WSAESHUTDOWN) {
			readc = 0;
		}
	}
#endif

	if (readc < 0) {
		return nixio__perror_s(L);
	} else {
		lua_pushlstring(L, buffer, readc);

		if (!from) {
			return 1;
		}
		/* push address. */
		if (sock->domain == AF_INET || sock->domain == AF_INET6) {
			nixio_addr naddr;
			if (!nixio__addr_parse(&naddr, (struct sockaddr *)&addr_in)) {
				lua_pushstring(L, naddr.host);
				lua_pushinteger(L, naddr.port);
				return 3;
			} else {
				return 1;
			}
		}
#ifndef __WINNT__
		else if (sock->domain == AF_UNIX && alen > sizeof(sa_family_t)) {
			/* if first char is non-null then the path is not in the
			   abstract namespace and alen includes the trailing null */
			if (addr_un.sun_path[0])
				--alen;
			lua_pushlstring(L, addr_un.sun_path, alen - sizeof(sa_family_t));
			return 2;
		}
#endif
	}
	return 1;
}

/**
 * recv(count)
 */
static int nixio_sock_recv(lua_State *L) {
	return nixio_sock__recvfrom(L, 0);
}

/**
 * recvfrom(count)
 */
static int nixio_sock_recvfrom(lua_State *L) {
	return nixio_sock__recvfrom(L, 1);
}


/* module table */
static const luaL_reg M[] = {
	{"send",	nixio_sock_send},
	{"sendto",	nixio_sock_sendto},
	{"recv",	nixio_sock_recv},
	{"recvfrom",nixio_sock_recvfrom},
	{"write",	nixio_sock_send},
	{"read",	nixio_sock_recv},
	{NULL,			NULL}
};

void nixio_open_io(lua_State *L) {
	lua_pushvalue(L, -2);
	luaL_register(L, NULL, M);
	lua_pop(L, 1);
}
