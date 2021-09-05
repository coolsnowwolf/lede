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
#include <unistd.h>
#include <string.h>
#include <errno.h>


/**
 * create new socket
 */
static int nixio_socket(lua_State *L) {
	const char *domain = luaL_optlstring(L, 1, "", NULL);
	const char *type   = luaL_optlstring(L, 2, "", NULL);
	const char *proto  = lua_tolstring(L, 3, NULL);

	nixio_sock *sock = lua_newuserdata(L, sizeof(nixio_sock));
	if (!sock) {
		return luaL_error(L, "out of memory");
	}

	if (!strcmp(domain, "inet")) {
		sock->domain = AF_INET;
	} else if (!strcmp(domain, "inet6")) {
		sock->domain = AF_INET6;
	} else if (!strcmp(domain, "unix")) {
		sock->domain = AF_UNIX;
	/*} else if (!strcmp(domain, "packet")) {
		sock->domain = AF_PACKET;*/
	} else {
		return luaL_argerror(L, 1,
		 "supported values: inet, inet6, unix, packet"
		);
	}

	if (!strcmp(type, "stream")) {
		sock->type = SOCK_STREAM;
	} else if (!strcmp(type, "dgram")) {
		sock->type = SOCK_DGRAM;
	} else if (!strcmp(type, "raw")) {
		sock->type = SOCK_RAW;
	} else {
		return luaL_argerror(L, 2, "supported values: stream, dgram, raw");
	}

	if (!proto) {
		sock->protocol = 0;
	} else if (!strcmp(proto, "icmp")) {
		sock->protocol = IPPROTO_ICMP;
	} else if (!strcmp(proto, "icmpv6")) {
		sock->protocol = IPPROTO_ICMPV6;
	} else {
		return luaL_argerror(L, 3, "supported values: [empty], icmp, icmpv6");
	}

	/* create userdata */
	luaL_getmetatable(L, NIXIO_META);
	lua_setmetatable(L, -2);

	sock->fd = socket(sock->domain, sock->type, sock->protocol);

	if (sock->fd < 0) {
		return nixio__perror_s(L);
	}

	return 1;
}

/**
 * close a socket
 */
static int nixio_sock_close(lua_State *L) {
	nixio_sock *sock = nixio__checksock(L);
	int sockfd = sock->fd;
	int res;
	sock->fd = -1;

	do {
#ifndef __WINNT__
		res = close(sockfd);
#else
		res = closesocket(sockfd);
#endif
	} while (res == -1 && errno == EINTR);

	return nixio__pstatus_s(L, !res);
}

/**
 * garbage collector
 */
static int nixio_sock__gc(lua_State *L) {
	nixio_sock *sock = (nixio_sock*)luaL_checkudata(L, 1, NIXIO_META);
	int res;
	if (sock && sock->fd != -1) {
		do {
#ifndef __WINNT__
			res = close(sock->fd);
#else
			res = closesocket(sock->fd);
#endif
		} while (res == -1 && errno == EINTR);
	}
	return 0;
}

/**
 * string representation
 */
static int nixio_sock__tostring(lua_State *L) {
	lua_pushfstring(L, "nixio socket %d", nixio__checksockfd(L));
	return 1;
}

/**
 * shutdown a socket
 */
static int nixio_sock_shutdown(lua_State *L) {
	int sockfd = nixio__checksockfd(L);
	const char *what = luaL_optlstring(L, 2, "rdwr", NULL);
	int how;

	if (!strcmp(what, "rdwr") || !strcmp(what, "both")) {
		how = SHUT_RDWR;
	} else if (!strcmp(what, "rd") || !strcmp(what, "read")) {
		how = SHUT_RD;
	} else if (!strcmp(what, "wr") || !strcmp(what, "write")) {
		how = SHUT_WR;
	} else {
		return luaL_argerror(L, 2, "supported values: both, read, write");
	}

	return nixio__pstatus_s(L, !shutdown(sockfd, how));
}

/* module table */
static const luaL_reg R[] = {
	{"socket",		nixio_socket},
	{NULL,			NULL}
};

/* object table */
static const luaL_reg M[] = {
	{"close",		nixio_sock_close},
	{"shutdown",	nixio_sock_shutdown},
	{"__gc",		nixio_sock__gc},
	{"__tostring",	nixio_sock__tostring},
	{NULL,			NULL}
};

void nixio_open_socket(lua_State *L) {
	luaL_register(L, NULL, R);

	lua_pushvalue(L, -2);
	luaL_register(L, NULL, M);
	lua_pop(L, 1);
}
