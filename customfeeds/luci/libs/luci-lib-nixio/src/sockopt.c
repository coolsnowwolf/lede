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

#ifndef __WINNT__
#include <net/if.h>
#endif

#include <sys/types.h>
#include <sys/time.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

#ifndef IPV6_ADD_MEMBERSHIP
#define IPV6_ADD_MEMBERSHIP		IPV6_JOIN_GROUP
#endif

#ifndef IPV6_DROP_MEMBERSHIP
#define IPV6_DROP_MEMBERSHIP	IPV6_LEAVE_GROUP
#endif

static int nixio_sock_fileno(lua_State *L) {
	lua_pushinteger(L, nixio__checkfd(L, 1));
	return 1;
}

/**
 * setblocking()
 */
static int nixio_sock_setblocking(lua_State *L) {
	int fd = nixio__checkfd(L, 1);
	luaL_checkany(L, 2);
	int set = lua_toboolean(L, 2);

#ifndef __WINNT__

	int flags = fcntl(fd, F_GETFL);

	if (flags == -1) {
		return nixio__perror(L);
	}

	if (!set) {
		flags |= O_NONBLOCK;
	} else {
		flags &= ~O_NONBLOCK;
	}

	return nixio__pstatus(L, !fcntl(fd, F_SETFL, flags));

#else /* __WINNT__ */

	lua_getmetatable(L, 1);
	luaL_getmetatable(L, NIXIO_META);
	if (lua_equal(L, -1, -2)) {	/* Socket */
		unsigned long val = !set;
		return nixio__pstatus_s(L, !ioctlsocket(fd, FIONBIO, &val));
	} else {					/* File */
		WSASetLastError(WSAENOTSOCK);
		return nixio__perror_s(L);
	}

#endif /* __WINNT__ */
}

static int nixio__gso_int(lua_State *L, int fd, int level, int opt, int set) {
	int value;
	socklen_t optlen = sizeof(value);
	if (!set) {
		if (!getsockopt(fd, level, opt, (char *)&value, &optlen)) {
			lua_pushinteger(L, value);
			return 1;
		}
	} else {
		value = luaL_checkinteger(L, set);
		if (!setsockopt(fd, level, opt, (char *)&value, optlen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

static int nixio__gso_ling(lua_State *L, int fd, int level, int opt, int set) {
	struct linger value;
	socklen_t optlen = sizeof(value);
	if (!set) {
		if (!getsockopt(fd, level, opt, (char *)&value, &optlen)) {
			lua_pushinteger(L, value.l_onoff ? value.l_linger : 0);
			return 1;
		}
	} else {
		value.l_linger = luaL_checkinteger(L, set);
		value.l_onoff = value.l_linger ? 1 : 0;
		if (!setsockopt(fd, level, opt, (char *)&value, optlen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

static int nixio__gso_timev(lua_State *L, int fd, int level, int opt, int set) {
	struct timeval value;
	socklen_t optlen = sizeof(value);
	if (!set) {
		if (!getsockopt(fd, level, opt, (char *)&value, &optlen)) {
			lua_pushinteger(L, value.tv_sec);
			lua_pushinteger(L, value.tv_usec);
			return 2;
		}
	} else {
		value.tv_sec  = luaL_checkinteger(L, set);
		value.tv_usec = luaL_optinteger(L, set + 1, 0);
		if (!setsockopt(fd, level, opt, (char *)&value, optlen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

#ifdef SO_BINDTODEVICE

static int nixio__gso_b(lua_State *L, int fd, int level, int opt, int set) {
	if (!set) {
		socklen_t optlen = IFNAMSIZ;
		char ifname[IFNAMSIZ];
		if (!getsockopt(fd, level, opt, (char *)ifname, &optlen)) {
			lua_pushlstring(L, ifname, optlen);
			return 1;
		}
	} else {
		size_t valuelen;
		const char *value = luaL_checklstring(L, set, &valuelen);
		luaL_argcheck(L, valuelen <= IFNAMSIZ, set, "invalid interface name");
		if (!setsockopt(fd, level, opt, (char *)value, valuelen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

#endif /* SO_BINDTODEVICE */

static int nixio__gso_mreq4(lua_State *L, int fd, int level, int opt, int set) {
	struct ip_mreq value;
	socklen_t optlen = sizeof(value);
	if (!set) {
		char buf[INET_ADDRSTRLEN];
		if (!getsockopt(fd, level, opt, (char *)&value, &optlen)) {
			if (!inet_ntop(AF_INET, &value.imr_multiaddr, buf, sizeof(buf))) {
				return nixio__perror_s(L);
			}
			lua_pushstring(L, buf);
			if (!inet_ntop(AF_INET, &value.imr_interface, buf, sizeof(buf))) {
				return nixio__perror_s(L);
			}
			lua_pushstring(L, buf);
			return 2;
		}
	} else {
		const char *maddr = luaL_checkstring(L, set);
		const char *iface = luaL_optstring(L, set + 1, "0.0.0.0");
		if (inet_pton(AF_INET, maddr, &value.imr_multiaddr) < 1) {
			return nixio__perror_s(L);
		}
		if (inet_pton(AF_INET, iface, &value.imr_interface) < 1) {
			return nixio__perror_s(L);
		}
		if (!setsockopt(fd, level, opt, (char *)&value, optlen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

static int nixio__gso_mreq6(lua_State *L, int fd, int level, int opt, int set) {
	struct ipv6_mreq val;
	socklen_t optlen = sizeof(val);
	if (!set) {
		char buf[INET_ADDRSTRLEN];
		if (!getsockopt(fd, level, opt, (char *)&val, &optlen)) {
			if (!inet_ntop(AF_INET6, &val.ipv6mr_multiaddr, buf, sizeof(buf))) {
				return nixio__perror_s(L);
			}
			lua_pushstring(L, buf);
			lua_pushinteger(L, val.ipv6mr_interface);
			return 2;
		}
	} else {
		const char *maddr = luaL_checkstring(L, set);
		if (inet_pton(AF_INET6, maddr, &val.ipv6mr_multiaddr) < 1) {
			return nixio__perror_s(L);
		}
		val.ipv6mr_interface = luaL_optlong(L, set + 1, 0);
		if (!setsockopt(fd, level, opt, (char *)&val, optlen)) {
			lua_pushboolean(L, 1);
			return 1;
		}
	}
	return nixio__perror_s(L);
}

/**
 * get/setsockopt() helper
 */
static int nixio__getsetsockopt(lua_State *L, int set) {
	nixio_sock *sock = nixio__checksock(L);
	const char *level = luaL_optlstring(L, 2, "", NULL);
	const char *option = luaL_optlstring(L, 3, "", NULL);
	set = (set) ? 4 : 0;

	if (!strcmp(level, "socket")) {
		if (!strcmp(option, "keepalive")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_KEEPALIVE, set);
		} else if (!strcmp(option, "reuseaddr")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_REUSEADDR, set);
		} else if (!strcmp(option, "rcvbuf")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_RCVBUF, set);
		} else if (!strcmp(option, "sndbuf")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_SNDBUF, set);
		} else if (!strcmp(option, "priority")) {
#ifdef SO_PRIORITY
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_PRIORITY, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else if (!strcmp(option, "broadcast")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_BROADCAST, set);
		} else if (!strcmp(option, "dontroute")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_DONTROUTE, set);
		} else if (!strcmp(option, "error")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_ERROR, set);
		} else if (!strcmp(option, "oobinline")) {
			return nixio__gso_int(L, sock->fd, SOL_SOCKET, SO_OOBINLINE, set);
		} else if (!strcmp(option, "linger")) {
			return nixio__gso_ling(L, sock->fd, SOL_SOCKET, SO_LINGER, set);
		} else if (!strcmp(option, "sndtimeo")) {
			return nixio__gso_timev(L, sock->fd, SOL_SOCKET, SO_SNDTIMEO, set);
		} else if (!strcmp(option, "rcvtimeo")) {
			return nixio__gso_timev(L, sock->fd, SOL_SOCKET, SO_RCVTIMEO, set);
		} else if (!strcmp(option, "bindtodevice")) {
#ifdef SO_BINDTODEVICE
			return nixio__gso_b(L, sock->fd, SOL_SOCKET, SO_BINDTODEVICE, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else {
			return luaL_argerror(L, 3, "supported values: keepalive, reuseaddr,"
			 " sndbuf, rcvbuf, priority, broadcast, linger, sndtimeo, rcvtimeo,"
			 " dontroute, bindtodevice, error, oobinline"
			);
		}
	} else if (!strcmp(level, "tcp")) {
		if (!strcmp(option, "cork")) {
#ifdef TCP_CORK
			return nixio__gso_int(L, sock->fd, IPPROTO_TCP, TCP_CORK, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else if (!strcmp(option, "nodelay")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_TCP, TCP_NODELAY, set);
		} else {
			return luaL_argerror(L, 3, "supported values: cork, nodelay");
		}
	} else if (!strcmp(level, "ip")) {
		if (!strcmp(option, "mtu")) {
#ifdef IP_MTU
			return nixio__gso_int(L, sock->fd, IPPROTO_IP, IP_MTU, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else if (!strcmp(option, "hdrincl")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_IP, IP_HDRINCL,
					set);
		} else if (!strcmp(option, "multicast_loop")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_IP, IP_MULTICAST_LOOP,
					set);
		} else if (!strcmp(option, "multicast_ttl")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_IP, IP_MULTICAST_TTL,
					set);
		} else if (!strcmp(option, "multicast_if")) {
			return nixio__gso_mreq4(L, sock->fd, IPPROTO_IP, IP_MULTICAST_IF,
					set);
		} else if (!strcmp(option, "add_membership")) {
			return nixio__gso_mreq4(L, sock->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
					set);
		} else if (!strcmp(option, "drop_membership")) {
			return nixio__gso_mreq4(L, sock->fd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
					set);
		} else {
			return luaL_argerror(L, 3,
				"supported values: hdrincl, mtu, multicast_loop, "
				"multicast_ttl, multicast_if, add_membership, drop_membership");
		}
	} else if (!strcmp(level, "ipv6")) {
		if (!strcmp(option, "mtu")) {
#ifdef IPV6_MTU
			return nixio__gso_int(L, sock->fd, IPPROTO_IPV6, IPV6_MTU, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else if (!strcmp(option, "v6only")) {
#ifdef IPV6_V6ONLY
			return nixio__gso_int(L, sock->fd, IPPROTO_IPV6, IPV6_V6ONLY, set);
#else
			return nixio__pstatus(L, !(errno = ENOPROTOOPT));
#endif
		} else if (!strcmp(option, "multicast_loop")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_IPV6,
					IPV6_MULTICAST_LOOP, set);
		} else if (!strcmp(option, "multicast_hops")) {
			return nixio__gso_int(L, sock->fd, IPPROTO_IPV6,
					IPV6_MULTICAST_HOPS, set);
		} else if (!strcmp(option, "multicast_if")) {
			return nixio__gso_mreq6(L, sock->fd, IPPROTO_IPV6,
					IPV6_MULTICAST_IF, set);
		} else if (!strcmp(option, "add_membership")) {
			return nixio__gso_mreq6(L, sock->fd, IPPROTO_IPV6,
					IPV6_ADD_MEMBERSHIP, set);
		} else if (!strcmp(option, "drop_membership")) {
			return nixio__gso_mreq6(L, sock->fd, IPPROTO_IPV6,
					IPV6_DROP_MEMBERSHIP, set);
		} else {
			return luaL_argerror(L, 3,
				"supported values: v6only, mtu, multicast_loop, multicast_hops,"
				" multicast_if, add_membership, drop_membership");
		}
	} else {
		return luaL_argerror(L, 2, "supported values: socket, tcp, ip, ipv6");
	}
}

/**
 * getsockopt()
 */
static int nixio_sock_getsockopt(lua_State *L) {
	return nixio__getsetsockopt(L, 0);
}

/**
 * setsockopt()
 */
static int nixio_sock_setsockopt(lua_State *L) {
	return nixio__getsetsockopt(L, 1);
}

/* module table */
static const luaL_reg M[] = {
	{"setblocking", nixio_sock_setblocking},
	{"getsockopt",	nixio_sock_getsockopt},
	{"setsockopt",	nixio_sock_setsockopt},
	{"getopt",		nixio_sock_getsockopt},
	{"setopt",		nixio_sock_setsockopt},
	{"fileno",		nixio_sock_fileno},
	{NULL,			NULL}
};

void nixio_open_sockopt(lua_State *L) {
	lua_pushvalue(L, -2);
	luaL_register(L, NULL, M);
	lua_pop(L, 1);

	luaL_getmetatable(L, NIXIO_FILE_META);
	lua_pushcfunction(L, nixio_sock_setblocking);
	lua_setfield(L, -2, "setblocking");
	lua_pushcfunction(L, nixio_sock_fileno);
	lua_setfield(L, -2, "fileno");
	lua_pop(L, 1);
}
