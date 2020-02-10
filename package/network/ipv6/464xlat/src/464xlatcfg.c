/* 464xlatcfg.c
 *
 * Copyright (c) 2015 Steven Barth <cyrus@openwrt.org>
 * Copyright (c) 2017 Hans Dedecker <dedeckeh@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <netinet/icmp6.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <netdb.h>

static void sighandler(__attribute__((unused)) int signal)
{
}

int main(int argc, const char *argv[])
{
	char buf[INET6_ADDRSTRLEN], prefix[INET6_ADDRSTRLEN + 4];
	int pid;

	if (argc <= 1) {
		fprintf(stderr, "Usage: %s <name> [ifname] [ipv6prefix] [ipv4addr] [ipv6addr]\n", argv[0]);
		return 1;
	}

	snprintf(buf, sizeof(buf), "/var/run/%s.pid", argv[1]);
	FILE *fp = fopen(buf, "r");
	if (fp) {
		if (fscanf(fp, "%d", &pid) == 1)
			kill(pid, SIGTERM);

		unlink(buf);
		fclose(fp);
	}

	if (!argv[2])
		return 0;

	if (!argv[3] || !argv[4] || !(fp = fopen(buf, "wx")))
		return 1;

	signal(SIGTERM, SIG_DFL);
	setvbuf(fp, NULL, _IOLBF, 0);
	fprintf(fp, "%d\n", getpid());

	prefix[sizeof(prefix) - 1] = 0;
	strncpy(prefix, argv[3], sizeof(prefix) - 1);

	if (!prefix[0]) {
		struct addrinfo hints = { .ai_family = AF_INET6 }, *res;
		if (getaddrinfo("ipv4only.arpa", NULL, &hints, &res) || !res) {
			sleep(3);
			if (getaddrinfo("ipv4only.arpa", NULL, &hints, &res) || !res)
				return 2;
		}

		struct sockaddr_in6 *sin6 = (struct sockaddr_in6*)res->ai_addr;
		inet_ntop(AF_INET6, &sin6->sin6_addr, prefix, sizeof(prefix) - 4);
		strcat(prefix, "/96");
		freeaddrinfo(res);
	}

	int i = 0;
	int sock;
	struct sockaddr_in6 saddr;

	do {
		socklen_t saddrlen = sizeof(saddr);
		struct icmp6_filter filt;

		sock = socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);
		ICMP6_FILTER_SETBLOCKALL(&filt);
		setsockopt(sock, IPPROTO_ICMPV6, ICMP6_FILTER, &filt, sizeof(filt));
		setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, argv[2], strlen(argv[2]));
		memset(&saddr, 0, sizeof(saddr));
		saddr.sin6_family = AF_INET6;
		saddr.sin6_addr.s6_addr32[0] = htonl(0x2001);
		saddr.sin6_addr.s6_addr32[1] = htonl(0xdb8);
		if (connect(sock, (struct sockaddr*)&saddr, sizeof(saddr)) ||
				getsockname(sock, (struct sockaddr*)&saddr, &saddrlen))
			return 3;

		if (!IN6_IS_ADDR_LINKLOCAL(&saddr.sin6_addr) || argv[5])
			break;

		close(sock);
		sleep(3);
		i++;
	} while (i < 3);

	struct ipv6_mreq mreq = {saddr.sin6_addr, if_nametoindex(argv[2])};
	if (!argv[5]) {
		if (IN6_IS_ADDR_LINKLOCAL(&mreq.ipv6mr_multiaddr))
			return 5;

		srandom(mreq.ipv6mr_multiaddr.s6_addr32[0] ^ mreq.ipv6mr_multiaddr.s6_addr32[1] ^
				mreq.ipv6mr_multiaddr.s6_addr32[2] ^ mreq.ipv6mr_multiaddr.s6_addr32[3]);
		mreq.ipv6mr_multiaddr.s6_addr32[2] = random();
		mreq.ipv6mr_multiaddr.s6_addr32[3] = random();
	} else if (inet_pton(AF_INET6, argv[5], &mreq.ipv6mr_multiaddr) != 1) {
		return 1;
	}

	if (setsockopt(sock, SOL_IPV6, IPV6_JOIN_ANYCAST, &mreq, sizeof(mreq)))
		return 3;

	inet_ntop(AF_INET6, &mreq.ipv6mr_multiaddr, buf, sizeof(buf));
	fputs(buf, stdout);
	fputc('\n', stdout);
	fflush(stdout);

	FILE *nat46 = fopen("/proc/net/nat46/control", "w");
	if (!nat46 || fprintf(nat46, "add %s\nconfig %s local.style NONE local.v4 %s/32 local.v6 %s/128 "
			"remote.style RFC6052 remote.v6 %s\n", argv[1], argv[1], argv[4], buf, prefix) < 0 ||
			fclose(nat46))
		return 4;

	if (!(pid = fork())) {
		fclose(fp);
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);
		chdir("/");
		setsid();
		signal(SIGTERM, sighandler);
		pause();

		nat46 = fopen("/proc/net/nat46/control", "w");
		if (nat46) {
			fprintf(nat46, "del %s\n", argv[1]);
			fclose(nat46);
		}
	} else {
		rewind(fp);
		fprintf(fp, "%d\n", pid);
	}

	return 0;
}
