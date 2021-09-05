/*
License:
Copyright 2013 Steven Barth <steven@midlink.org>

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <stdlib.h>
#include <stdbool.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/ether.h>

char *ether_ntoa_l (const struct ether_addr *addr, char *buf)
{
  sprintf (buf, "%02x:%02x:%02x:%02x:%02x:%02x",
           addr->ether_addr_octet[0], addr->ether_addr_octet[1],
           addr->ether_addr_octet[2], addr->ether_addr_octet[3],
           addr->ether_addr_octet[4], addr->ether_addr_octet[5]);
  return buf;
}

static int neightbl_get(lua_State *L) {
	int sock = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	struct sockaddr_nl kernel = {AF_NETLINK, 0, 0, 0};
	if (connect(sock, (struct sockaddr*)&kernel, sizeof(kernel)))
		goto error;

	const char *ifname = luaL_checkstring(L, 1);
	int ifindex = if_nametoindex(ifname);
	if (ifindex <= 0)
		goto error;

	struct {
		struct nlmsghdr hdr;
		struct ndmsg ndm;
	} req = {
		{sizeof(req), RTM_GETNEIGH, NLM_F_REQUEST | NLM_F_DUMP, 1, 0},
		{AF_INET6, 0, 0, ifindex, 0, 0, 0},
	};

	if (send(sock, &req, sizeof(req), 0) != sizeof(req))
		goto error;

	lua_newtable(L);

	char buf[8192];
	struct nlmsghdr *nh = (struct nlmsghdr*)buf;
	do {
		ssize_t len = recv(sock, buf, sizeof(buf), 0);
		if (len < 0) {
			lua_pop(L, 1);
			goto error;
		}


		for (;NLMSG_OK(nh, (size_t)len) && nh->nlmsg_type == RTM_NEWNEIGH;
				nh = NLMSG_NEXT(nh, len)) {
			struct ndmsg *ndm = NLMSG_DATA(nh);
			if (NLMSG_PAYLOAD(nh, 0) < sizeof(*ndm) || ndm->ndm_ifindex != ifindex)
				continue;

			ssize_t alen = NLMSG_PAYLOAD(nh, sizeof(*ndm));
			char buf[INET6_ADDRSTRLEN] = {0}, *mac = NULL, str_buf[ETH_ALEN];
			for (struct rtattr *rta = (struct rtattr*)&ndm[1]; RTA_OK(rta, alen);
					rta = RTA_NEXT(rta, alen)) {
				if (rta->rta_type == NDA_DST && RTA_PAYLOAD(rta) >= sizeof(struct in6_addr))
					inet_ntop(AF_INET6, RTA_DATA(rta), buf, sizeof(buf));
				else if (rta->rta_type == NDA_LLADDR && RTA_PAYLOAD(rta) >= 6)
					mac = ether_ntoa_l(RTA_DATA(rta),str_buf);
			}

			if (mac)
				lua_pushstring(L, mac);
			else
				lua_pushboolean(L, false);

			lua_setfield(L, -2, buf);
		}
	} while (nh->nlmsg_type == RTM_NEWNEIGH);

	close(sock);
        return 1;

error:
	close(sock);
	lua_pushnil(L);
	lua_pushinteger(L, errno);
	lua_pushstring(L, strerror(errno));
	return 3;
}


static const luaL_reg R[] = {
        {"get", neightbl_get},
        {NULL, NULL}
};


int luaopen_neightbl(lua_State *l)
{
	luaL_register(l, "neightbl", R);
	return 1;
}
