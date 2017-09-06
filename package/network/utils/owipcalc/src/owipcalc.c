/*
 * owipcalc - OpenWrt IP Calculator
 *
 *   Copyright (C) 2012 Jo-Philipp Wich <jo@mein.io>
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

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>


struct cidr {
	uint8_t family;
	uint32_t prefix;
	union {
		struct in_addr v4;
		struct in6_addr v6;
	} addr;
	union {
		char v4[sizeof("255.255.255.255/255.255.255.255 ")];
		char v6[sizeof("FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:255.255.255.255/128 ")];
	} buf;
	struct cidr *next;
};

struct op {
	const char *name;
	const char *desc;
	struct {
		bool (*a1)(struct cidr *a);
		bool (*a2)(struct cidr *a, struct cidr *b);
	} f4;
	struct {
		bool (*a1)(struct cidr *a);
		bool (*a2)(struct cidr *a, struct cidr *b);
	} f6;
};


static bool quiet = false;
static bool printed = false;

static struct cidr *stack = NULL;

#define qprintf(...) \
	do { \
		if (!quiet) printf(__VA_ARGS__); \
		printed = true; \
	} while(0)

static void cidr_push(struct cidr *a)
{
	if (a)
	{
		a->next = stack;
		stack = a;
	}
}

static bool cidr_pop(struct cidr *a)
{
	struct cidr *old = stack;

	if (old)
	{
		stack = stack->next;
		free(old);

		return true;
	}

	return false;
}

static struct cidr * cidr_clone(struct cidr *a)
{
	struct cidr *b = malloc(sizeof(*b));

	if (!b)
	{
		fprintf(stderr, "out of memory\n");
		exit(255);
	}

	memcpy(b, a, sizeof(*b));
	cidr_push(b);

	return b;
}


static struct cidr * cidr_parse4(const char *s)
{
	char *p = NULL, *r;
	struct in_addr mask;
	struct cidr *addr = malloc(sizeof(struct cidr));

	if (!addr || (strlen(s) >= sizeof(addr->buf.v4)))
		goto err;

	snprintf(addr->buf.v4, sizeof(addr->buf.v4), "%s", s);

	addr->family = AF_INET;

	if ((p = strchr(addr->buf.v4, '/')) != NULL)
	{
		*p++ = 0;

		if (strchr(p, '.') != NULL)
		{
			if (inet_pton(AF_INET, p, &mask) != 1)
				goto err;

			for (addr->prefix = 0; mask.s_addr; mask.s_addr >>= 1)
				addr->prefix += (mask.s_addr & 1);
		}
		else
		{
			addr->prefix = strtoul(p, &r, 10);

			if ((p == r) || (*r != 0) || (addr->prefix > 32))
				goto err;
		}
	}
	else
	{
		addr->prefix = 32;
	}

	if (p == addr->buf.v4+1)
		memset(&addr->addr.v4, 0, sizeof(addr->addr.v4));
	else if (inet_pton(AF_INET, addr->buf.v4, &addr->addr.v4) != 1)
		goto err;

	return addr;

err:
	if (addr)
		free(addr);

	return NULL;
}

static bool cidr_add4(struct cidr *a, struct cidr *b)
{
	uint32_t x = ntohl(a->addr.v4.s_addr);
	uint32_t y = ntohl(b->addr.v4.s_addr);

	struct cidr *n = cidr_clone(a);

	if ((n->family != AF_INET) || (b->family != AF_INET))
		return false;

	if ((uint32_t)(x + y) < x)
	{
		fprintf(stderr, "overflow during 'add'\n");
		return false;
	}

	n->addr.v4.s_addr = htonl(x + y);
	return true;
}

static bool cidr_sub4(struct cidr *a, struct cidr *b)
{
	uint32_t x = ntohl(a->addr.v4.s_addr);
	uint32_t y = ntohl(b->addr.v4.s_addr);

	struct cidr *n = cidr_clone(a);

	if ((n->family != AF_INET) || (b->family != AF_INET))
		return false;

	if ((uint32_t)(x - y) > x)
	{
		fprintf(stderr, "underflow during 'sub'\n");
		return false;
	}

	n->addr.v4.s_addr = htonl(x - y);
	return true;
}

static bool cidr_network4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr &= htonl(~((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

static bool cidr_broadcast4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr |= htonl(((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

static bool cidr_contains4(struct cidr *a, struct cidr *b)
{
	uint32_t net1 = a->addr.v4.s_addr & htonl(~((1 << (32 - a->prefix)) - 1));
	uint32_t net2 = b->addr.v4.s_addr & htonl(~((1 << (32 - a->prefix)) - 1));

	if (printed)
		qprintf(" ");

	if ((b->prefix >= a->prefix) && (net1 == net2))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_netmask4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr = htonl(~((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

static bool cidr_private4(struct cidr *a)
{
	uint32_t x = ntohl(a->addr.v4.s_addr);

	if (printed)
		qprintf(" ");

	if (((x >= 0x0A000000) && (x <= 0x0AFFFFFF)) ||
	    ((x >= 0xAC100000) && (x <= 0xAC1FFFFF)) ||
	    ((x >= 0xC0A80000) && (x <= 0xC0A8FFFF)))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_linklocal4(struct cidr *a)
{
	uint32_t x = ntohl(a->addr.v4.s_addr);

	if (printed)
		qprintf(" ");

	if ((x >= 0xA9FE0000) && (x <= 0xA9FEFFFF))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_prev4(struct cidr *a, struct cidr *b)
{
	struct cidr *n = cidr_clone(a);

	n->prefix = b->prefix;
	n->addr.v4.s_addr -= htonl(1 << (32 - b->prefix));

	return true;
}

static bool cidr_next4(struct cidr *a, struct cidr *b)
{
	struct cidr *n = cidr_clone(a);

	n->prefix = b->prefix;
	n->addr.v4.s_addr += htonl(1 << (32 - b->prefix));

	return true;
}

static bool cidr_6to4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);
	uint32_t x = a->addr.v4.s_addr;

	memset(&n->addr.v6.s6_addr, 0, sizeof(n->addr.v6.s6_addr));

	n->family = AF_INET6;
	n->prefix = 48;

	n->addr.v6.s6_addr[0] = 0x20;
	n->addr.v6.s6_addr[1] = 0x02;
	n->addr.v6.s6_addr[2] = (x >> 24);
	n->addr.v6.s6_addr[3] = (x >> 16) & 0xFF;
	n->addr.v6.s6_addr[4] = (x >>  8) & 0xFF;
	n->addr.v6.s6_addr[5] = x & 0xFF;

	return true;
}

static bool cidr_print4(struct cidr *a)
{
	char *p;

	if (!a || (a->family != AF_INET))
		return false;

	if (!(p = (char *)inet_ntop(AF_INET, &a->addr.v4, a->buf.v4, sizeof(a->buf.v4))))
		return false;

	if (printed)
		qprintf(" ");

	qprintf("%s", p);

	if (a->prefix < 32)
		qprintf("/%u", a->prefix);

	cidr_pop(a);

	return true;
}


static struct cidr * cidr_parse6(const char *s)
{
	char *p = NULL, *r;
	struct cidr *addr = malloc(sizeof(struct cidr));

	if (!addr || (strlen(s) >= sizeof(addr->buf.v6)))
		goto err;

	snprintf(addr->buf.v4, sizeof(addr->buf.v6), "%s", s);

	addr->family = AF_INET6;

	if ((p = strchr(addr->buf.v4, '/')) != NULL)
	{
		*p++ = 0;

		addr->prefix = strtoul(p, &r, 10);

		if ((p == r) || (*r != 0) || (addr->prefix > 128))
			goto err;
	}
	else
	{
		addr->prefix = 128;
	}

	if (p == addr->buf.v4+1)
		memset(&addr->addr.v6, 0, sizeof(addr->addr.v6));
	else if (inet_pton(AF_INET6, addr->buf.v4, &addr->addr.v6) != 1)
		goto err;

	return addr;

err:
	if (addr)
		free(addr);

	return NULL;
}

static bool cidr_add6(struct cidr *a, struct cidr *b)
{
	uint8_t idx = 15, carry = 0, overflow = 0;

	struct cidr *n = cidr_clone(a);
	struct in6_addr *x = &n->addr.v6;
	struct in6_addr *y = &b->addr.v6;

	if ((a->family != AF_INET6) || (b->family != AF_INET6))
		return false;

	do {
		overflow = !!((x->s6_addr[idx] + y->s6_addr[idx] + carry) >= 256);
		x->s6_addr[idx] += y->s6_addr[idx] + carry;
		carry = overflow;
	}
	while (idx-- > 0);

	if (carry)
	{
		fprintf(stderr, "overflow during 'add'\n");
		return false;
	}

	return true;
}

static bool cidr_sub6(struct cidr *a, struct cidr *b)
{
	uint8_t idx = 15, carry = 0, underflow = 0;

	struct cidr *n = cidr_clone(a);
	struct in6_addr *x = &n->addr.v6;
	struct in6_addr *y = &b->addr.v6;

	if ((n->family != AF_INET6) || (b->family != AF_INET6))
		return false;

	do {
		underflow = !!((x->s6_addr[idx] - y->s6_addr[idx] - carry) < 0);
		x->s6_addr[idx] -= y->s6_addr[idx] + carry;
		carry = underflow;
	}
	while (idx-- > 0);

	if (carry)
	{
		fprintf(stderr, "underflow during 'sub'\n");
		return false;
	}

	return true;
}

static bool cidr_prev6(struct cidr *a, struct cidr *b)
{
	uint8_t idx, carry = 1, underflow = 0;
	struct cidr *n = cidr_clone(a);
	struct in6_addr *x = &n->addr.v6;

	if (b->prefix == 0)
	{
		fprintf(stderr, "underflow during 'prev'\n");
		return false;
	}

	idx = (b->prefix - 1) / 8;

	do {
		underflow = !!((x->s6_addr[idx] - carry) < 0);
		x->s6_addr[idx] -= carry;
		carry = underflow;
	}
	while (idx-- > 0);

	if (carry)
	{
		fprintf(stderr, "underflow during 'prev'\n");
		return false;
	}

	n->prefix = b->prefix;

	return true;
}

static bool cidr_next6(struct cidr *a, struct cidr *b)
{
	uint8_t idx, carry = 1, overflow = 0;
	struct cidr *n = cidr_clone(a);
	struct in6_addr *x = &n->addr.v6;

	if (b->prefix == 0)
	{
		fprintf(stderr, "overflow during 'next'\n");
		return false;
	}

	idx = (b->prefix - 1) / 8;

	do {
		overflow = !!((x->s6_addr[idx] + carry) >= 256);
		x->s6_addr[idx] += carry;
		carry = overflow;
	}
	while (idx-- > 0);

	if (carry)
	{
		fprintf(stderr, "overflow during 'next'\n");
		return false;
	}

	n->prefix = b->prefix;

	return true;
}

static bool cidr_network6(struct cidr *a)
{
	uint8_t i;
	struct cidr *n = cidr_clone(a);

	for (i = 0; i < (128 - n->prefix) / 8; i++)
		n->addr.v6.s6_addr[15-i] = 0;

	if ((128 - n->prefix) % 8)
		n->addr.v6.s6_addr[15-i] &= ~((1 << ((128 - n->prefix) % 8)) - 1);

	return true;
}

static bool cidr_contains6(struct cidr *a, struct cidr *b)
{
	struct cidr *n = cidr_clone(a);
	struct in6_addr *x = &n->addr.v6;
	struct in6_addr *y = &b->addr.v6;
	uint8_t i = (128 - n->prefix) / 8;
	uint8_t m = ~((1 << ((128 - n->prefix) % 8)) - 1);
	uint8_t net1 = x->s6_addr[15-i] & m;
	uint8_t net2 = y->s6_addr[15-i] & m;

	if (printed)
		qprintf(" ");

	if ((b->prefix >= n->prefix) && (net1 == net2) &&
	    ((i == 15) || !memcmp(&x->s6_addr, &y->s6_addr, 15-i)))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_linklocal6(struct cidr *a)
{
	if (printed)
		qprintf(" ");

	if ((a->addr.v6.s6_addr[0] == 0xFE) &&
	    (a->addr.v6.s6_addr[1] >= 0x80) &&
	    (a->addr.v6.s6_addr[1] <= 0xBF))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_ula6(struct cidr *a)
{
	if (printed)
		qprintf(" ");

	if ((a->addr.v6.s6_addr[0] >= 0xFC) &&
	    (a->addr.v6.s6_addr[0] <= 0xFD))
	{
		qprintf("1");
		return true;
	}
	else
	{
		qprintf("0");
		return false;
	}
}

static bool cidr_print6(struct cidr *a)
{
	char *p;

	if (!a || (a->family != AF_INET6))
		return NULL;

	if (!(p = (char *)inet_ntop(AF_INET6, &a->addr.v6, a->buf.v6, sizeof(a->buf.v6))))
		return false;

	if (printed)
		qprintf(" ");

	qprintf("%s", p);

	if (a->prefix < 128)
		qprintf("/%u", a->prefix);

	cidr_pop(a);

	return true;
}


static struct cidr * cidr_parse(const char *op, const char *s, int af_hint)
{
	char *r;
	struct cidr *a;

	uint8_t i;
	uint32_t sum = strtoul(s, &r, 0);

	if ((r > s) && (*r == 0))
	{
		a = malloc(sizeof(struct cidr));

		if (!a)
			return NULL;

		if (af_hint == AF_INET)
		{
			a->family = AF_INET;
			a->prefix = sum;
			a->addr.v4.s_addr = htonl(sum);
		}
		else
		{
			a->family = AF_INET6;
			a->prefix = sum;

			for (i = 0; i <= 15; i++)
			{
				a->addr.v6.s6_addr[15-i] = sum % 256;
				sum >>= 8;
			}
		}

		return a;
	}

	if (strchr(s, ':'))
		a = cidr_parse6(s);
	else
		a = cidr_parse4(s);

	if (!a)
		return NULL;

	if (a->family != af_hint)
	{
		fprintf(stderr, "attempt to '%s' %s with %s address\n",
				op,
				(af_hint == AF_INET) ? "ipv4" : "ipv6",
				(af_hint != AF_INET) ? "ipv4" : "ipv6");
		exit(4);
	}

	return a;
}

static bool cidr_howmany(struct cidr *a, struct cidr *b)
{
	if (printed)
		qprintf(" ");

	if (b->prefix < a->prefix)
		qprintf("0");
	else
		qprintf("%u", 1 << (b->prefix - a->prefix));

	return true;
}

static bool cidr_prefix(struct cidr *a, struct cidr *b)
{
	a->prefix = b->prefix;
	return true;
}

static bool cidr_quiet(struct cidr *a)
{
	quiet = true;
	return true;
}


struct op ops[] = {
	{ .name = "add",
	  .desc = "Add argument to base address",
	  .f4.a2 = cidr_add4,
	  .f6.a2 = cidr_add6 },

	{ .name = "sub",
	  .desc = "Substract argument from base address",
	  .f4.a2 = cidr_sub4,
	  .f6.a2 = cidr_sub6 },

	{ .name = "next",
	  .desc = "Advance base address to next prefix of given size",
	  .f4.a2 = cidr_next4,
	  .f6.a2 = cidr_next6 },

	{ .name = "prev",
	  .desc = "Lower base address to previous prefix of give size",
	  .f4.a2 = cidr_prev4,
	  .f6.a2 = cidr_prev6 },

	{ .name = "network",
	  .desc = "Turn base address into network address",
	  .f4.a1 = cidr_network4,
	  .f6.a1 = cidr_network6 },

	{ .name = "broadcast",
	  .desc = "Turn base address into broadcast address",
	  .f4.a1 = cidr_broadcast4 },

	{ .name = "prefix",
	  .desc = "Set the prefix of base address to argument",
	  .f4.a2 = cidr_prefix,
	  .f6.a2 = cidr_prefix },

	{ .name = "netmask",
	  .desc = "Calculate netmask of base address",
	  .f4.a1 = cidr_netmask4 },

	{ .name = "6to4",
	  .desc = "Calculate 6to4 prefix of given ipv4-address",
	  .f4.a1 = cidr_6to4 },

	{ .name = "howmany",
	  .desc = "Print amount of righ-hand prefixes that fit into base address",
	  .f4.a2 = cidr_howmany,
	  .f6.a2 = cidr_howmany },

	{ .name = "contains",
	  .desc = "Print '1' if argument fits into base address or '0' if not",
	  .f4.a2 = cidr_contains4,
	  .f6.a2 = cidr_contains6 },

	{ .name = "private",
	  .desc = "Print '1' if base address is in RFC1918 private space or '0' "
	          "if not",
	  .f4.a1 = cidr_private4 },

	{ .name = "linklocal",
	  .desc = "Print '1' if base address is in 169.254.0.0/16 or FE80::/10 "
	          "link local space or '0' if not",
	  .f4.a1 = cidr_linklocal4,
	  .f6.a1 = cidr_linklocal6 },

	{ .name = "ula",
	  .desc = "Print '1' if base address is in FC00::/7 unique local address "
	          "(ULA) space or '0' if not",
	  .f6.a1 = cidr_ula6 },

	{ .name = "quiet",
	  .desc = "Suppress output, useful for test operation where the result can "
	          "be inferred from the exit code",
	  .f4.a1 = cidr_quiet,
	  .f6.a1 = cidr_quiet },

	{ .name = "pop",
	  .desc = "Pop intermediate result from stack",
	  .f4.a1 = cidr_pop,
	  .f6.a1 = cidr_pop },

	{ .name = "print",
	  .desc = "Print intermediate result and pop it from stack, invoked "
	          "implicitely at the end of calculation if no intermediate prints "
	          "happened",
	  .f4.a1 = cidr_print4,
	  .f6.a1 = cidr_print6 },
};

static void usage(const char *prog)
{
	int i;

	fprintf(stderr,
	        "\n"
	        "Usage:\n\n"
	        "  %s {base address} operation [argument] "
	        "[operation [argument] ...]\n\n"
	        "Operations:\n\n",
	        prog);

	for (i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
	{
		if (ops[i].f4.a2 || ops[i].f6.a2)
		{
			fprintf(stderr, "  %s %s\n",
			        ops[i].name,
			        (ops[i].f4.a2 && ops[i].f6.a2) ? "{ipv4/ipv6/amount}" :
			         (ops[i].f6.a2 ? "{ipv6/amount}" : "{ipv4/amount}"));
		}
		else
		{
			fprintf(stderr, "  %s\n", ops[i].name);
		}

		fprintf(stderr, "    %s.\n", ops[i].desc);

		if ((ops[i].f4.a1 && ops[i].f6.a1) || (ops[i].f4.a2 && ops[i].f6.a2))
			fprintf(stderr, "    Applicable to ipv4- and ipv6-addresses.\n\n");
		else if (ops[i].f6.a2 || ops[i].f6.a1)
			fprintf(stderr, "    Only applicable to ipv6-addresses.\n\n");
		else
			fprintf(stderr, "    Only applicable to ipv4-addresses.\n\n");
	}

	fprintf(stderr,
	        "Examples:\n\n"
	        " Calculate a DHCP range:\n\n"
	        "  $ %s 192.168.1.1/255.255.255.0 network add 100 print add 150 print\n"
			"  192.168.1.100\n"
			"  192.168.1.250\n\n"
			" Count number of prefixes:\n\n"
			"  $ %s 2001:0DB8:FDEF::/48 howmany ::/64\n"
			"  65536\n\n",
	        prog, prog);

	exit(1);
}

static bool runop(char ***arg, int *status)
{
	int i;
	char *arg1 = **arg;
	char *arg2 = *(*arg+1);
	struct cidr *a = stack;
	struct cidr *b = NULL;

	if (!arg1)
		return false;

	for (i = 0; i < sizeof(ops) / sizeof(ops[0]); i++)
	{
		if (!strcmp(ops[i].name, arg1))
		{
			if (ops[i].f4.a2 || ops[i].f6.a2)
			{
				if (!arg2)
				{
					fprintf(stderr, "'%s' requires an argument\n",
							ops[i].name);

					*status = 2;
					return false;
				}

				b = cidr_parse(ops[i].name, arg2, a->family);

				if (!b)
				{
					fprintf(stderr, "invalid address argument for '%s'\n",
							ops[i].name);

					*status = 3;
					return false;
				}

				*arg += 2;

				if (((a->family == AF_INET)  && !ops[i].f4.a2) ||
					((a->family == AF_INET6) && !ops[i].f6.a2))
				{
					fprintf(stderr, "'%s' not supported for %s addresses\n",
					        ops[i].name,
							(a->family == AF_INET) ? "ipv4" : "ipv6");

					*status = 5;
					return false;
				}

				*status = !((a->family == AF_INET) ? ops[i].f4.a2(a, b)
				                                   : ops[i].f6.a2(a, b));

				return true;
			}
			else
			{
				*arg += 1;

				if (((a->family == AF_INET)  && !ops[i].f4.a1) ||
					((a->family == AF_INET6) && !ops[i].f6.a1))
				{
					fprintf(stderr, "'%s' not supported for %s addresses\n",
					        ops[i].name,
							(a->family == AF_INET) ? "ipv4" : "ipv6");

					*status = 5;
					return false;
				}

				*status = !((a->family == AF_INET) ? ops[i].f4.a1(a)
				                                   : ops[i].f6.a1(a));

				return true;
			}
		}
	}

	return false;
}

int main(int argc, char **argv)
{
	int status = 0;
	char **arg = argv+2;
	struct cidr *a;

	if (argc < 3)
		usage(argv[0]);

	a = strchr(argv[1], ':') ? cidr_parse6(argv[1]) : cidr_parse4(argv[1]);

	if (!a)
		usage(argv[0]);

	cidr_push(a);

	while (runop(&arg, &status));

	if (*arg)
	{
		fprintf(stderr, "unknown operation '%s'\n", *arg);
		exit(6);
	}

	if (!printed && (status < 2))
	{
		if (stack->family == AF_INET)
			cidr_print4(stack);
		else
			cidr_print6(stack);
	}

	qprintf("\n");

	exit(status);
}
