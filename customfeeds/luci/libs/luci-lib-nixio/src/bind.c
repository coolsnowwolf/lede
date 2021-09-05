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
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>


/**
 * connect()/bind() shortcut
 */
static int nixio__bind_connect(lua_State *L, int do_bind) {
	const char *host = NULL;
	if (!lua_isnoneornil(L, 1)) {
		host = luaL_checklstring(L, 1, NULL);
	}
	const char *port = luaL_checklstring(L, 2, NULL);
	const char *family = luaL_optlstring(L, 3, "any", NULL);
	const char *socktype = luaL_optlstring(L, 4, "stream", NULL);

	struct addrinfo hints, *result, *rp;
	memset(&hints, 0, sizeof(hints));

	if (!strcmp(family, "any")) {
		hints.ai_family = AF_UNSPEC;
	} else if (!strcmp(family, "inet")) {
		hints.ai_family = AF_INET;
	} else if (!strcmp(family, "inet6")) {
		hints.ai_family = AF_INET6;
	} else {
		return luaL_argerror(L, 3, "supported values: any, inet, inet6");
	}

	if (!strcmp(socktype, "any")) {
		hints.ai_socktype = 0;
	} else if (!strcmp(socktype, "stream")) {
		hints.ai_socktype = SOCK_STREAM;
	} else if (!strcmp(socktype, "dgram")) {
		hints.ai_socktype = SOCK_DGRAM;
	} else {
		return luaL_argerror(L, 4, "supported values: any, stream, dgram");
	}

	if (do_bind) {
		hints.ai_flags |= AI_PASSIVE;
	}

	hints.ai_protocol = 0;

	int aistat = getaddrinfo(host, port, &hints, &result);
	if (aistat) {
		lua_pushnil(L);
		lua_pushinteger(L, aistat);
		lua_pushstring(L, gai_strerror(aistat));
		return 3;
	}

	/* create socket object */
	nixio_sock *sock = lua_newuserdata(L, sizeof(nixio_sock));
	int status = -1, clstat;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sock->fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sock->fd == -1) {
			continue;
		}

		if (do_bind) {
			int one = 1;
			setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR,
			 (char*)&one, sizeof(one));
			status = bind(sock->fd, rp->ai_addr, rp->ai_addrlen);
		} else {
			do {
				status = connect(sock->fd, rp->ai_addr, rp->ai_addrlen);
			} while (status == -1 && errno == EINTR);
		}

		/* on success */
		if (!status) {
			sock->domain = rp->ai_family;
			sock->type = rp->ai_socktype;
			sock->protocol = rp->ai_protocol;
			break;
		}

		do {
#ifndef __WINNT__
			clstat = close(sock->fd);
#else
			clstat = closesocket(sock->fd);
#endif
		} while (clstat == -1 && errno == EINTR);
	}

	freeaddrinfo(result);

	/* on failure */
	if (status) {
		return nixio__perror_s(L);
	}

	luaL_getmetatable(L, NIXIO_META);
	lua_setmetatable(L, -2);

	return 1;
}

/**
 * bind(host, port, [family=any], [type=any]) shortcut
 */
static int nixio_bind(lua_State *L) {
	return nixio__bind_connect(L, 1);
}

/**
 * connect(host, port, [family=any], [type=any]) shortcut
 */
static int nixio_connect(lua_State *L) {
	return nixio__bind_connect(L, 0);
}

/**
 * bind()/connect() helper
 */
static int nixio_sock__bind_connect(lua_State *L, int do_bind) {
	nixio_sock *sock = nixio__checksock(L);
	int status = -1;

	if (sock->domain == AF_INET || sock->domain == AF_INET6) {
		const char *host = NULL;
		if (!lua_isnoneornil(L, 2)) {
			host = luaL_checklstring(L, 2, NULL);
		}
		const char *port = luaL_checklstring(L, 3, NULL);

		struct addrinfo hints, *result, *rp;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = sock->domain;
		hints.ai_socktype = sock->type;
		hints.ai_protocol = sock->protocol;

		if (do_bind) {
			hints.ai_flags |= AI_PASSIVE;
		}

		int aistat = getaddrinfo(host, port, &hints, &result);
		if (aistat) {
			lua_pushnil(L);
			lua_pushinteger(L, aistat);
			lua_pushstring(L, gai_strerror(aistat));
			return 3;
		}

		for (rp = result; rp != NULL; rp = rp->ai_next) {
			if (do_bind) {
				status = bind(sock->fd, rp->ai_addr, rp->ai_addrlen);
			} else {
				do {
					status = connect(sock->fd, rp->ai_addr, rp->ai_addrlen);
				} while (status == -1 && errno == EINTR);
			}

			/* on success */
			if (!status || errno == EINPROGRESS) {
				break;
			}
		}

		freeaddrinfo(result);
#ifndef __WINNT__
	} else if (sock->domain == AF_UNIX) {
		size_t pathlen;
		const char *path = luaL_checklstring(L, 2, &pathlen);

		struct sockaddr_un addr;
		addr.sun_family = AF_UNIX;
		luaL_argcheck(L, pathlen <= sizeof(addr.sun_path), 2, "out of range");
		memcpy(addr.sun_path, path, pathlen);
		socklen_t alen = sizeof(sa_family_t) + pathlen;

		if (do_bind) {
			status = bind(sock->fd, (struct sockaddr*)&addr, alen);
		} else {
			do {
				status = connect(sock->fd, (struct sockaddr*)&addr, alen);
			} while (status == -1 && errno == EINTR);
		}
#endif
	} else {
		return luaL_error(L, "not supported");
	}
	return nixio__pstatus_s(L, !status);
}

/**
 * bind()
 */
static int nixio_sock_bind(lua_State *L) {
	return nixio_sock__bind_connect(L, 1);
}

/**
 * connect()
 */
static int nixio_sock_connect(lua_State *L) {
	return nixio_sock__bind_connect(L, 0);
}

/**
 * listen()
 */
static int nixio_sock_listen(lua_State *L) {
	int sockfd = nixio__checksockfd(L);
	int backlog = luaL_checkinteger(L, 2);
	return nixio__pstatus_s(L, !listen(sockfd, backlog));
}

/**
 * accept()
 */
static int nixio_sock_accept(lua_State *L) {
	nixio_sock *sock = nixio__checksock(L);
	struct sockaddr_storage saddr;
	nixio_addr addr;
	socklen_t saddrlen = sizeof(saddr);
	int newfd;

	do {
		newfd = accept(sock->fd, (struct sockaddr *)&saddr, &saddrlen);
	} while (newfd == -1 && errno == EINTR);
	if (newfd < 0) {
		return nixio__perror_s(L);
	}

	/* create userdata */
	nixio_sock *clsock = lua_newuserdata(L, sizeof(nixio_sock));
	luaL_getmetatable(L, NIXIO_META);
	lua_setmetatable(L, -2);

	memcpy(clsock, sock, sizeof(clsock));
	clsock->fd = newfd;

	if (!nixio__addr_parse(&addr, (struct sockaddr *)&saddr)) {
		lua_pushstring(L, addr.host);
		lua_pushinteger(L, addr.port);
		return 3;
	} else {
		return 1;
	}
}

/* module table */
static const luaL_reg R[] = {
	{"bind",		nixio_bind},
	{"connect",		nixio_connect},
	{NULL,			NULL}
};

/* object table */
static const luaL_reg M[] = {
	{"bind",		nixio_sock_bind},
	{"connect",		nixio_sock_connect},
	{"listen",		nixio_sock_listen},
	{"accept",		nixio_sock_accept},
	{NULL,			NULL}
};

void nixio_open_bind(lua_State *L) {
	luaL_register(L, NULL, R);

	lua_pushvalue(L, -2);
	luaL_register(L, NULL, M);
	lua_pop(L, 1);
}
