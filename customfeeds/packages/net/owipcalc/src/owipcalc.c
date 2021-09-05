#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include "owipcalc.h"

bool quiet = true;
bool printed = false;

struct cidr *stack = NULL;

void cidr_push(struct cidr *a)
{
	if (a)
	{
		a->next = stack;
		stack = a;
	}
}

bool cidr_pop(struct cidr *a)
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


struct cidr * cidr_parse4(const char *s)
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

bool cidr_add4(struct cidr *a, struct cidr *b)
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

bool cidr_sub4(struct cidr *a, struct cidr *b)
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

bool cidr_network4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr &= htonl(~((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

bool cidr_broadcast4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr |= htonl(((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

bool cidr_contains4(struct cidr *a, struct cidr *b)
{
	uint32_t net1 = a->addr.v4.s_addr & htonl(~((1 << (32 - a->prefix)) - 1));
	uint32_t net2 = b->addr.v4.s_addr & htonl(~((1 << (32 - a->prefix)) - 1));

	if (printed)
		qprintf(" ");

	if ((a->prefix == 0) || ((b->prefix >= a->prefix) && (net1 == net2)))
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

bool cidr_netmask4(struct cidr *a)
{
	struct cidr *n = cidr_clone(a);

	n->addr.v4.s_addr = htonl(~((1 << (32 - n->prefix)) - 1));
	n->prefix = 32;

	return true;
}

bool cidr_private4(struct cidr *a)
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

bool cidr_linklocal4(struct cidr *a)
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

bool cidr_prev4(struct cidr *a, struct cidr *b)
{
	struct cidr *n = cidr_clone(a);

	n->prefix = b->prefix;
	n->addr.v4.s_addr -= htonl(1 << (32 - b->prefix));

	return true;
}

bool cidr_next4(struct cidr *a, struct cidr *b)
{
	struct cidr *n = cidr_clone(a);

	n->prefix = b->prefix;
	n->addr.v4.s_addr += htonl(1 << (32 - b->prefix));

	return true;
}

bool cidr_6to4(struct cidr *a)
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

bool cidr_print4(struct cidr *a)
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


struct cidr * cidr_parse6(const char *s)
{
	char *p = NULL, *r;
	struct cidr *addr = malloc(sizeof(struct cidr));

	if (!addr || (strlen(s) >= sizeof(addr->buf.v6)))
		goto err;

	snprintf(addr->buf.v6, sizeof(addr->buf.v6), "%s", s);

	addr->family = AF_INET6;

	if ((p = strchr(addr->buf.v6, '/')) != NULL)
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

	if (p == addr->buf.v6+1)
		memset(&addr->addr.v6, 0, sizeof(addr->addr.v6));
	else if (inet_pton(AF_INET6, addr->buf.v6, &addr->addr.v6) != 1)
		goto err;

	return addr;

err:
	if (addr)
		free(addr);

	return NULL;
}

bool cidr_add6(struct cidr *a, struct cidr *b)
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

bool cidr_sub6(struct cidr *a, struct cidr *b)
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

bool cidr_prev6(struct cidr *a, struct cidr *b)
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

bool cidr_next6(struct cidr *a, struct cidr *b)
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

bool cidr_network6(struct cidr *a)
{
	uint8_t i;
	struct cidr *n = cidr_clone(a);

	for (i = 0; i < (128 - n->prefix) / 8; i++)
		n->addr.v6.s6_addr[15-i] = 0;

	if ((128 - n->prefix) % 8)
		n->addr.v6.s6_addr[15-i] &= ~((1 << ((128 - n->prefix) % 8)) - 1);

	return true;
}

bool cidr_contains6(struct cidr *a, struct cidr *b)
{
	struct in6_addr *x = &a->addr.v6;
	struct in6_addr *y = &b->addr.v6;
	uint8_t i = ((128 - a->prefix) / 8) % 16;
	uint8_t m = ~((1 << ((128 - a->prefix) % 8)) - 1);
	uint8_t net1 = x->s6_addr[15-i] & m;
	uint8_t net2 = y->s6_addr[15-i] & m;

	if (printed)
		qprintf(" ");

	if ((a->prefix == 0) ||
	    ((b->prefix >= a->prefix) && (net1 == net2) &&
	    ((i == 15) || !memcmp(&x->s6_addr, &y->s6_addr, 15-i))))
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

bool cidr_linklocal6(struct cidr *a)
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

bool cidr_ula6(struct cidr *a)
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

bool cidr_print6(struct cidr *a)
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


struct cidr * cidr_parse(const char *op, const char *s, int af_hint)
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

bool cidr_howmany(struct cidr *a, struct cidr *b)
{
	if (printed)
		qprintf(" ");

	if (b->prefix < a->prefix)
		qprintf("0");
	else
		qprintf("%u", 1 << (b->prefix - a->prefix));

	return true;
}

bool cidr_prefix(struct cidr *a, struct cidr *b)
{
	a->prefix = b->prefix;
	return true;
}

bool cidr_quiet(struct cidr *a)
{
	quiet = true;
	return true;
}
