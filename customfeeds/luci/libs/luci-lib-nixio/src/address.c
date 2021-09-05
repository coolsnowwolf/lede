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
#include <sys/param.h>
#include <errno.h>
#include <string.h>

#ifdef __linux__

#include <signal.h>
#include <setjmp.h>
#include <unistd.h>

/* setjmp() / longjmp() stuff */
static jmp_buf nixio__jump_alarm;
static void nixio__handle_alarm(int sig) { longjmp(nixio__jump_alarm, 1); }

#include <linux/netdevice.h>

/* struct net_device_stats is buggy on amd64, redefine it */
struct nixio__nds {
     uint32_t rx_packets;
     uint32_t tx_packets;
     uint32_t rx_bytes;
     uint32_t tx_bytes;
     uint32_t rx_errors;
     uint32_t tx_errors;
     uint32_t rx_dropped;
     uint32_t tx_dropped;
     uint32_t multicast;
     uint32_t collisions;

     uint32_t rx_length_errors;
     uint32_t rx_over_errors;
     uint32_t rx_crc_errors;
     uint32_t rx_frame_errors;
     uint32_t rx_fifo_errors;
     uint32_t rx_missed_errors;

     uint32_t tx_aborted_errors;
     uint32_t tx_carrier_errors;
     uint32_t tx_fifo_errors;
     uint32_t tx_heartbeat_errors;
     uint32_t tx_window_errors;

     uint32_t rx_compressed;
     uint32_t tx_compressed;
};
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

/**
 * address pushing helper
 */
int nixio__addr_parse(nixio_addr *addr, struct sockaddr *saddr) {
	void *baddr;

	addr->family = saddr->sa_family;
	if (saddr->sa_family == AF_INET) {
		struct sockaddr_in *inetaddr = (struct sockaddr_in*)saddr;
		addr->port = ntohs(inetaddr->sin_port);
		baddr = &inetaddr->sin_addr;
	} else if (saddr->sa_family == AF_INET6) {
		struct sockaddr_in6 *inet6addr = (struct sockaddr_in6*)saddr;
		addr->port = ntohs(inet6addr->sin6_port);
		baddr = &inet6addr->sin6_addr;
#ifdef AF_PACKET
	} else if (saddr->sa_family == AF_PACKET) {
		struct sockaddr_ll *etheradddr = (struct sockaddr_ll*)saddr;
		addr->prefix = etheradddr->sll_hatype;
		addr->port = etheradddr->sll_ifindex;
		char *c = addr->host;
		for (size_t i = 0; i < etheradddr->sll_halen; i++) {
			*c++ = nixio__bin2hex[(etheradddr->sll_addr[i] & 0xf0) >> 4];
			*c++ = nixio__bin2hex[(etheradddr->sll_addr[i] & 0x0f)];
			*c++ = ':';
		}
		*(c-1) = 0;
		return 0;
#endif
	} else {
		errno = EAFNOSUPPORT;
		return -1;
	}

	if (!inet_ntop(saddr->sa_family, baddr, addr->host, sizeof(addr->host))) {
		return -1;
	}

	return 0;
}

/**
 * address pulling helper
 */
int nixio__addr_write(nixio_addr *addr, struct sockaddr *saddr) {
	if (addr->family == AF_UNSPEC) {
		if (strchr(addr->host, ':')) {
			addr->family = AF_INET6;
		} else {
			addr->family = AF_INET;
		}
	}
	if (addr->family == AF_INET) {
		struct sockaddr_in *inetaddr = (struct sockaddr_in *)saddr;
		memset(inetaddr, 0, sizeof(struct sockaddr_in));

		if (inet_pton(AF_INET, addr->host, &inetaddr->sin_addr) < 1) {
			return -1;
		}

		inetaddr->sin_family = AF_INET;
		inetaddr->sin_port = htons((uint16_t)addr->port);
		return 0;
	} else if (addr->family == AF_INET6) {
		struct sockaddr_in6 *inet6addr = (struct sockaddr_in6 *)saddr;
		memset(inet6addr, 0, sizeof(struct sockaddr_in6));

		if (inet_pton(AF_INET6, addr->host, &inet6addr->sin6_addr) < 1) {
			return -1;
		}

		inet6addr->sin6_family = AF_INET6;
		inet6addr->sin6_port = htons((uint16_t)addr->port);
		return 0;
	} else {
		errno = EAFNOSUPPORT;
		return -1;
	}
}

/**
 * netmask to prefix helper
 */
static int nixio__addr_prefix(struct sockaddr *saddr) {
	int prefix = 0;
	size_t len;
	uint8_t *addr;

	if (saddr->sa_family == AF_INET) {
		addr = (uint8_t*)(&((struct sockaddr_in*)saddr)->sin_addr);
		len = 4;
	} else if (saddr->sa_family == AF_INET6) {
		addr = (uint8_t*)(&((struct sockaddr_in6*)saddr)->sin6_addr);
		len = 16;
	} else {
		errno = EAFNOSUPPORT;
		return -1;
	}

	for (size_t i = 0; i < len; i++) {
		if (addr[i] == 0xff) {
			prefix += 8;
		} else if (addr[i] == 0x00) {
			break;
		} else {
			for (uint8_t c = addr[i]; c; c <<= 1) {
				prefix++;
			}
		}
	}

	return prefix;
}

/**
 * getaddrinfo(host, family, port)
 */
static int nixio_getaddrinfo(lua_State *L) {
	const char *host = NULL;
	if (!lua_isnoneornil(L, 1)) {
		host = luaL_checklstring(L, 1, NULL);
	}
	const char *family = luaL_optlstring(L, 2, "any", NULL);
	const char *port = lua_tolstring(L, 3, NULL);

	struct addrinfo hints, *result, *rp;
	memset(&hints, 0, sizeof(hints));

	if (!strcmp(family, "any")) {
		hints.ai_family = AF_UNSPEC;
	} else if (!strcmp(family, "inet")) {
		hints.ai_family = AF_INET;
	} else if (!strcmp(family, "inet6")) {
		hints.ai_family = AF_INET6;
	} else {
		return luaL_argerror(L, 2, "supported values: any, inet, inet6");
	}

	hints.ai_socktype = 0;
	hints.ai_protocol = 0;

	int aistat = getaddrinfo(host, port, &hints, &result);
	if (aistat) {
		lua_pushnil(L);
		lua_pushinteger(L, aistat);
		lua_pushstring(L, gai_strerror(aistat));
		return 3;
	}

	/* create socket object */
	lua_newtable(L);
	int i = 1;

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		/* avoid duplicate results */
#ifndef __WINNT__
		if (!port && rp->ai_socktype != SOCK_STREAM) {
			continue;
		}
#endif

		if (rp->ai_family == AF_INET || rp->ai_family == AF_INET6) {
			lua_createtable(L, 0, port ? 4 : 2);
			if (rp->ai_family == AF_INET) {
				lua_pushliteral(L, "inet");
			} else if (rp->ai_family == AF_INET6) {
				lua_pushliteral(L, "inet6");
			}
			lua_setfield(L, -2, "family");

			if (port) {
				switch (rp->ai_socktype) {
					case SOCK_STREAM:
						lua_pushliteral(L, "stream");
						break;
					case SOCK_DGRAM:
						lua_pushliteral(L, "dgram");
						break;
					case SOCK_RAW:
						lua_pushliteral(L, "raw");
						break;
					default:
						lua_pushnil(L);
						break;
				}
				lua_setfield(L, -2, "socktype");
			}

			nixio_addr addr;
			if (nixio__addr_parse(&addr, rp->ai_addr)) {
				freeaddrinfo(result);
				return nixio__perror_s(L);
			}

			if (port) {
				lua_pushinteger(L, addr.port);
				lua_setfield(L, -2, "port");
			}

			lua_pushstring(L, addr.host);
			lua_setfield(L, -2, "address");
			lua_rawseti(L, -2, i++);
		}
	}

	freeaddrinfo(result);

	return 1;
}

/**
 * getnameinfo(address, family[, timeout])
 */
static int nixio_getnameinfo(lua_State *L) {
	const char *ip = luaL_checkstring(L, 1);
	const char *family = luaL_optstring(L, 2, NULL);

#ifdef __linux__
	struct sigaction sa_new, sa_old;
	int timeout = luaL_optnumber(L, 3, 0);
	if (timeout > 0 && timeout < 1000)
	{
		sa_new.sa_handler = nixio__handle_alarm;
		sa_new.sa_flags   = 0;
		sigemptyset(&sa_new.sa_mask);
		sigaction(SIGALRM, &sa_new, &sa_old);

		/* user timeout exceeded */
		if (setjmp(nixio__jump_alarm))
		{
			sigaction(SIGALRM, &sa_old, NULL);

			lua_pushnil(L);
			lua_pushinteger(L, EAI_AGAIN);
			lua_pushstring(L, gai_strerror(EAI_AGAIN));

			return 3;
		}

		ualarm(timeout * 1000, 0);
	}
#endif

	char host[NI_MAXHOST];

	struct sockaddr_storage saddr;
	nixio_addr addr;
	memset(&addr, 0, sizeof(addr));
	strncpy(addr.host, ip, sizeof(addr.host) - 1);

	if (!family) {
		addr.family = AF_UNSPEC;
	} else if (!strcmp(family, "inet")) {
		addr.family = AF_INET;
	} else if (!strcmp(family, "inet6")) {
		addr.family = AF_INET6;
	} else {
		return luaL_argerror(L, 2, "supported values: inet, inet6");
	}

	nixio__addr_write(&addr, (struct sockaddr *)&saddr);

	int res = getnameinfo((struct sockaddr *)&saddr,
	 (saddr.ss_family == AF_INET)
	  ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6),
	 host, sizeof(host), NULL, 0, NI_NAMEREQD);

#ifdef __linux__
	if (timeout > 0 && timeout < 1000)
	{
		ualarm(0, 0);
		sigaction(SIGALRM, &sa_old, NULL);
	}
#endif

	if (res) {
		lua_pushnil(L);
		lua_pushinteger(L, res);
		lua_pushstring(L, gai_strerror(res));
		return 3;
	} else {
		lua_pushstring(L, host);
		return 1;
	}
}

/**
 * getsockname()
 */
static int nixio_sock_getsockname(lua_State *L) {
	int sockfd = nixio__checksockfd(L);
	struct sockaddr_storage saddr;
	socklen_t addrlen = sizeof(saddr);
	nixio_addr addr;

	if (getsockname(sockfd, (struct sockaddr*)&saddr, &addrlen) ||
	 nixio__addr_parse(&addr, (struct sockaddr*)&saddr)) {
		return nixio__perror_s(L);
	}

	lua_pushstring(L, addr.host);
	lua_pushinteger(L, addr.port);
	return 2;
}

/**
 * getpeername()
 */
static int nixio_sock_getpeername(lua_State *L) {
	int sockfd = nixio__checksockfd(L);
	struct sockaddr_storage saddr;
	socklen_t addrlen = sizeof(saddr);
	nixio_addr addr;

	if (getpeername(sockfd, (struct sockaddr*)&saddr, &addrlen) ||
	 nixio__addr_parse(&addr, (struct sockaddr*)&saddr)) {
		return nixio__perror_s(L);
	}

	lua_pushstring(L, addr.host);
	lua_pushinteger(L, addr.port);
	return 2;
}

#if defined(__linux__) || defined(BSD)
#ifdef BSD
#include <net/if.h>
#endif
#include <ifaddrs.h>

static int nixio_getifaddrs(lua_State *L) {
	nixio_addr addr;
	struct ifaddrs *ifaddr, *c;
	if (getifaddrs(&ifaddr) == -1) {
		return nixio__perror(L);
	}

	lua_newtable(L);
	unsigned int i = 1;

	for (c = ifaddr; c; c = c->ifa_next) {
		lua_newtable(L);

		lua_pushstring(L, c->ifa_name);
		lua_setfield(L, -2, "name");

		lua_createtable(L, 0, 7);
			lua_pushboolean(L, c->ifa_flags & IFF_UP);
			lua_setfield(L, -2, "up");

			lua_pushboolean(L, c->ifa_flags & IFF_BROADCAST);
			lua_setfield(L, -2, "broadcast");

			lua_pushboolean(L, c->ifa_flags & IFF_LOOPBACK);
			lua_setfield(L, -2, "loopback");

			lua_pushboolean(L, c->ifa_flags & IFF_POINTOPOINT);
			lua_setfield(L, -2, "pointtopoint");

			lua_pushboolean(L, c->ifa_flags & IFF_NOARP);
			lua_setfield(L, -2, "noarp");

			lua_pushboolean(L, c->ifa_flags & IFF_PROMISC);
			lua_setfield(L, -2, "promisc");

			lua_pushboolean(L, c->ifa_flags & IFF_MULTICAST);
			lua_setfield(L, -2, "multicast");
		lua_setfield(L, -2, "flags");

		if (c->ifa_addr) {
			if (!nixio__addr_parse(&addr, c->ifa_addr)) {
				lua_pushstring(L, addr.host);
				lua_setfield(L, -2, "addr");
			}

			if (c->ifa_addr->sa_family == AF_INET) {
				lua_pushliteral(L, "inet");
			} else if (c->ifa_addr->sa_family == AF_INET6) {
				lua_pushliteral(L, "inet6");
#ifdef AF_PACKET
			} else if (c->ifa_addr->sa_family == AF_PACKET) {
				lua_pushliteral(L, "packet");
#endif
			} else {
				lua_pushliteral(L, "unknown");
			}
			lua_setfield(L, -2, "family");

#ifdef __linux__
			if (c->ifa_addr->sa_family == AF_PACKET) {
				lua_pushinteger(L, addr.port);
				lua_setfield(L, -2, "ifindex");

				lua_pushinteger(L, addr.prefix);
				lua_setfield(L, -2, "hatype");
			}
#endif
		}

#ifdef __linux__
		if (c->ifa_data && (!c->ifa_addr
							|| c->ifa_addr->sa_family == AF_PACKET)) {
			if (!c->ifa_addr) {
				lua_pushliteral(L, "packet");
				lua_setfield(L, -2, "family");
			}

			lua_createtable(L, 0, 10);
			struct nixio__nds *stats = c->ifa_data;

			lua_pushnumber(L, stats->rx_packets);
			lua_setfield(L, -2, "rx_packets");

			lua_pushnumber(L, stats->tx_packets);
			lua_setfield(L, -2, "tx_packets");

			lua_pushnumber(L, stats->rx_bytes);
			lua_setfield(L, -2, "rx_bytes");

			lua_pushnumber(L, stats->tx_bytes);
			lua_setfield(L, -2, "tx_bytes");

			lua_pushnumber(L, stats->rx_errors);
			lua_setfield(L, -2, "rx_errors");

			lua_pushnumber(L, stats->tx_errors);
			lua_setfield(L, -2, "tx_errors");

			lua_pushnumber(L, stats->rx_dropped);
			lua_setfield(L, -2, "rx_dropped");

			lua_pushnumber(L, stats->tx_dropped);
			lua_setfield(L, -2, "tx_dropped");

			lua_pushnumber(L, stats->multicast);
			lua_setfield(L, -2, "multicast");

			lua_pushnumber(L, stats->collisions);
			lua_setfield(L, -2, "collisions");
		} else {
			lua_newtable(L);
		}
		lua_setfield(L, -2, "data");
#endif

		if (c->ifa_netmask && !nixio__addr_parse(&addr, c->ifa_netmask)) {
			lua_pushstring(L, addr.host);
			lua_setfield(L, -2, "netmask");

			lua_pushinteger(L, nixio__addr_prefix(c->ifa_netmask));
			lua_setfield(L, -2, "prefix");
		}

		if (c->ifa_broadaddr && !nixio__addr_parse(&addr, c->ifa_broadaddr)) {
			lua_pushstring(L, addr.host);
			lua_setfield(L, -2, "broadaddr");
		}

		if (c->ifa_dstaddr && !nixio__addr_parse(&addr, c->ifa_dstaddr)) {
			lua_pushstring(L, addr.host);
			lua_setfield(L, -2, "dstaddr");
		}

		lua_rawseti(L, -2, i++);
	}

	freeifaddrs(ifaddr);
	return 1;
}
#endif


/* module table */
static const luaL_reg R[] = {
#if defined(__linux__) || defined(BSD)
	{"getifaddrs",	nixio_getifaddrs},
#endif
	{"getaddrinfo",	nixio_getaddrinfo},
	{"getnameinfo",	nixio_getnameinfo},
	{NULL,			NULL}
};

/* object table */
static const luaL_reg M[] = {
	{"getsockname",	nixio_sock_getsockname},
	{"getpeername",	nixio_sock_getpeername},
	{NULL,			NULL}
};

void nixio_open_address(lua_State *L) {
	luaL_register(L, NULL, R);

	lua_pushvalue(L, -2);
	luaL_register(L, NULL, M);
	lua_pop(L, 1);
}
