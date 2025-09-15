/*
 * mapcalc - MAP parameter calculation
 *
 * Author: Steven Barth <cyrus@openwrt.org>
 * Copyright (c) 2014-2015 cisco Systems, Inc.
 * Copyright (c) 2015 Steven Barth <cyrus@openwrt.org>
 * Copyright (c) 2018 Hans Dedecker <dedeckeh@gmail.com>
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

#include <stdlib.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <libubus.h>
#include <libubox/utils.h>


struct blob_attr *dump = NULL;

enum {
	DUMP_ATTR_INTERFACE,
	DUMP_ATTR_MAX
};

static const struct blobmsg_policy dump_attrs[DUMP_ATTR_MAX] = {
	[DUMP_ATTR_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_ARRAY },
};


enum {
	IFACE_ATTR_INTERFACE,
	IFACE_ATTR_PREFIX,
	IFACE_ATTR_ADDRESS,
	IFACE_ATTR_MAX,
};

static const struct blobmsg_policy iface_attrs[IFACE_ATTR_MAX] = {
	[IFACE_ATTR_INTERFACE] = { .name = "interface", .type = BLOBMSG_TYPE_STRING },
	[IFACE_ATTR_PREFIX] = { .name = "ipv6-prefix", .type = BLOBMSG_TYPE_ARRAY },
	[IFACE_ATTR_ADDRESS] = { .name = "ipv6-address", .type = BLOBMSG_TYPE_ARRAY },
};


enum {
	PREFIX_ATTR_ADDRESS,
	PREFIX_ATTR_MASK,
	PREFIX_ATTR_MAX,
};

static const struct blobmsg_policy prefix_attrs[PREFIX_ATTR_MAX] = {
	[PREFIX_ATTR_ADDRESS] = { .name = "address", .type = BLOBMSG_TYPE_STRING },
	[PREFIX_ATTR_MASK] = { .name = "mask", .type = BLOBMSG_TYPE_INT32 },
};

static int bmemcmp(const void *av, const void *bv, size_t bits)
{
	const uint8_t *a = av, *b = bv;
	size_t bytes = bits / 8;
	bits %= 8;

	int res = memcmp(a, b, bytes);
	if (res == 0 && bits > 0)
		res = (a[bytes] >> (8 - bits)) - (b[bytes] >> (8 - bits));

	return res;
}

static void bmemcpy(void *av, const void *bv, size_t bits)
{
	uint8_t *a = av;
	const uint8_t *b = bv;

	size_t bytes = bits / 8;
	bits %= 8;
	memcpy(a, b, bytes);

	if (bits > 0) {
		uint8_t mask = (1 << (8 - bits)) - 1;
		a[bytes] = (a[bytes] & mask) | ((~mask) & b[bytes]);
	}
}

static void bmemcpys64(void *av, const void *bv, size_t frombits, size_t nbits)
{
	uint64_t buf = 0;
	const uint8_t *b = bv;
	size_t frombyte = frombits / 8, tobyte = (frombits + nbits) / 8;

	memcpy(&buf, &b[frombyte], tobyte - frombyte + 1);
	buf = cpu_to_be64(be64_to_cpu(buf) << (frombits % 8));

	bmemcpy(av, &buf, nbits);
}

static void handle_dump(struct ubus_request *req __attribute__((unused)),
		int type __attribute__((unused)), struct blob_attr *msg)
{
	struct blob_attr *tb[DUMP_ATTR_INTERFACE];
	blobmsg_parse(dump_attrs, DUMP_ATTR_MAX, tb, blob_data(msg), blob_len(msg));

	if (!tb[DUMP_ATTR_INTERFACE])
		return;

	dump = blob_memdup(tb[DUMP_ATTR_INTERFACE]);
}

static void match_prefix(int *pdlen, struct in6_addr *pd, struct blob_attr *cur,
		const struct in6_addr *ipv6prefix, int prefix6len, bool lw4o6)
{
	struct blob_attr *d;
	unsigned drem;

	if (!cur || blobmsg_type(cur) != BLOBMSG_TYPE_ARRAY || !blobmsg_check_attr(cur, false))
		return;

	blobmsg_for_each_attr(d, cur, drem) {
		struct blob_attr *ptb[PREFIX_ATTR_MAX];
		blobmsg_parse(prefix_attrs, PREFIX_ATTR_MAX, ptb,
				blobmsg_data(d), blobmsg_data_len(d));

		if (!ptb[PREFIX_ATTR_ADDRESS] || !ptb[PREFIX_ATTR_MASK])
			continue;

		struct in6_addr prefix = IN6ADDR_ANY_INIT;
		int mask = blobmsg_get_u32(ptb[PREFIX_ATTR_MASK]);
		inet_pton(AF_INET6, blobmsg_get_string(ptb[PREFIX_ATTR_ADDRESS]), &prefix);

		// lw4over6 /128-address-as-PD matching madness workaround
		if (lw4o6 && mask == 128)
			mask = 64;

		if (*pdlen < mask && mask >= prefix6len &&
				!bmemcmp(&prefix, ipv6prefix, prefix6len)) {
			bmemcpy(pd, &prefix, mask);
			*pdlen = mask;
		} else if (lw4o6 && *pdlen < prefix6len && mask < prefix6len &&
				!bmemcmp(&prefix, ipv6prefix, mask)) {
			bmemcpy(pd, ipv6prefix, prefix6len);
			*pdlen = prefix6len;
		}
	}
}

enum {
	OPT_TYPE,
	OPT_FMR,
	OPT_EALEN,
	OPT_PREFIX4LEN,
	OPT_PREFIX6LEN,
	OPT_IPV6PREFIX,
	OPT_IPV4PREFIX,
	OPT_OFFSET,
	OPT_PSIDLEN,
	OPT_PSID,
	OPT_BR,
	OPT_DMR,
	OPT_PD,
	OPT_PDLEN,
	OPT_MAX
};

static char *const token[] = {
	[OPT_TYPE] = "type",
	[OPT_FMR] = "fmr",
	[OPT_EALEN] = "ealen",
	[OPT_PREFIX4LEN] = "prefix4len",
	[OPT_PREFIX6LEN] = "prefix6len",
	[OPT_IPV6PREFIX] = "ipv6prefix",
	[OPT_IPV4PREFIX] = "ipv4prefix",
	[OPT_OFFSET] = "offset",
	[OPT_PSIDLEN] = "psidlen",
	[OPT_PSID] = "psid",
	[OPT_BR] = "br",
	[OPT_DMR] = "dmr",
	[OPT_PD] = "pd",
	[OPT_PDLEN] = "pdlen",
	[OPT_MAX] = NULL
};


int main(int argc, char *argv[])
{
	int status = 0;
	const char *iface = argv[1];

	const char *legacy_env = getenv("LEGACY");
	bool legacy = legacy_env && atoi(legacy_env);


	if (argc < 3) {
		fprintf(stderr, "Usage: %s <interface|*> <rule1> [rule2] [...]\n", argv[0]);
		return 1;
	}

	uint32_t network_interface;
	struct ubus_context *ubus = ubus_connect(NULL);
	if (ubus) {
		ubus_lookup_id(ubus, "network.interface", &network_interface);
		ubus_invoke(ubus, network_interface, "dump", NULL, handle_dump, NULL, 5000);
	}

	int rulecnt = 0;
	for (int i = 2; i < argc; ++i) {
		bool lw4o6 = false;
		bool fmr = false;
		int ealen = -1;
		int addr4len = 32;
		int prefix4len = 32;
		int prefix6len = -1;
		int pdlen = -1;
		struct in_addr ipv4prefix = {INADDR_ANY};
		struct in_addr ipv4addr = {INADDR_ANY};
		struct in6_addr ipv6addr = IN6ADDR_ANY_INIT;
		struct in6_addr ipv6prefix = IN6ADDR_ANY_INIT;
		struct in6_addr pd = IN6ADDR_ANY_INIT;
		int offset = -1;
		int psidlen = -1;
		int psid = -1;
		uint16_t psid16 = 0;
		const char *dmr = NULL;
		const char *br = NULL;

		for (char *rule = strdup(argv[i]); *rule; ) {
			char *value;
			int intval;
			int idx = getsubopt(&rule, token, &value);
			errno = 0;

			if (idx == OPT_TYPE) {
				lw4o6 = (value && !strcmp(value, "lw4o6"));
			} else if (idx == OPT_FMR) {
				fmr = true;
			} else if (idx == OPT_EALEN && (intval = strtoul(value, NULL, 0)) <= 48 && !errno) {
				ealen = intval;
			} else if (idx == OPT_PREFIX4LEN && (intval = strtoul(value, NULL, 0)) <= 32 && !errno) {
				prefix4len = intval;
			} else if (idx == OPT_PREFIX6LEN && (intval = strtoul(value, NULL, 0)) <= 128 && !errno) {
				prefix6len = intval;
			} else if (idx == OPT_IPV4PREFIX && inet_pton(AF_INET, value, &ipv4prefix) == 1) {
				// dummy
			} else if (idx == OPT_IPV6PREFIX && inet_pton(AF_INET6, value, &ipv6prefix) == 1) {
				// dummy
			} else if (idx == OPT_PD && inet_pton(AF_INET6, value, &pd) == 1) {
				// dummy
			} else if (idx == OPT_OFFSET && (intval = strtoul(value, NULL, 0)) <= 16 && !errno) {
				offset = intval;
			} else if (idx == OPT_PSIDLEN && (intval = strtoul(value, NULL, 0)) <= 16 && !errno) {
				psidlen = intval;
			} else if (idx == OPT_PDLEN && (intval = strtoul(value, NULL, 0)) <= 128 && !errno) {
				pdlen = intval;
			} else if (idx == OPT_PSID && (intval = strtoul(value, NULL, 0)) <= 65535 && !errno) {
				psid = intval;
			} else if (idx == OPT_DMR) {
				dmr = value;
			} else if (idx == OPT_BR) {
				br = value;
			} else {
				if (idx == -1 || idx >= OPT_MAX)
					fprintf(stderr, "Skipped invalid option: %s\n", value);
				else
					fprintf(stderr, "Skipped invalid value %s for option %s\n",
							value, token[idx]);
			}
		}

		if (offset < 0)
			offset = (lw4o6) ? 0 : (legacy) ? 4 : 6;

		// LW4over6 doesn't have an EALEN and has no psid-autodetect
		if (lw4o6) {
			if (psidlen < 0)
				psidlen = 0;

			ealen = psidlen;
		}

		// Find PD
		if (pdlen < 0) {
			struct blob_attr *c;
			unsigned rem;
			blobmsg_for_each_attr(c, dump, rem) {
				struct blob_attr *tb[IFACE_ATTR_MAX];
				blobmsg_parse(iface_attrs, IFACE_ATTR_MAX, tb, blobmsg_data(c), blobmsg_data_len(c));

				if (!tb[IFACE_ATTR_INTERFACE] || (strcmp(argv[1], "*") && strcmp(argv[1],
						blobmsg_get_string(tb[IFACE_ATTR_INTERFACE]))))
					continue;

				match_prefix(&pdlen, &pd, tb[IFACE_ATTR_PREFIX], &ipv6prefix, prefix6len, lw4o6);

				if (lw4o6)
					match_prefix(&pdlen, &pd, tb[IFACE_ATTR_ADDRESS], &ipv6prefix, prefix6len, lw4o6);

				if (pdlen >= 0) {
					iface = blobmsg_get_string(tb[IFACE_ATTR_INTERFACE]);
					break;
				}
			}
		}

		if (ealen < 0 && pdlen >= 0)
			ealen = pdlen - prefix6len;

		if (psidlen <= 0) {
			psidlen = ealen - (32 - prefix4len);
			if (psidlen < 0)
				psidlen = 0;

			psid = -1;
		}

		if (prefix4len < 0 || prefix6len < 0 || ealen < 0 || psidlen > 16 || ealen < psidlen) {
			fprintf(stderr, "Skipping invalid or incomplete rule: %s\n", argv[i]);
			status = 1;
			continue;
		}

		if (psid < 0 && psidlen >= 0 && pdlen >= 0) {
			bmemcpys64(&psid16, &pd, prefix6len + ealen - psidlen, psidlen);
			psid = be16_to_cpu(psid16);
		}

		if (psidlen > 0) {
			psid = psid >> (16 - psidlen);
			psid16 = cpu_to_be16(psid);
			psid = psid << (16 - psidlen);
		}

		if (pdlen >= 0 || ealen == psidlen) {
			bmemcpys64(&ipv4addr, &pd, prefix6len, ealen - psidlen);
			ipv4addr.s_addr = htonl(ntohl(ipv4addr.s_addr) >> prefix4len);
			bmemcpy(&ipv4addr, &ipv4prefix, prefix4len);

			if (prefix4len + ealen < 32)
				addr4len = prefix4len + ealen;
		}

		if (pdlen < 0 && !fmr) {
			fprintf(stderr, "Skipping non-FMR without matching PD: %s\n", argv[i]);
			status = 1;
			continue;
		} else if (pdlen >= 0) {
			size_t v4offset = (legacy) ? 9 : 10;
			memcpy(&ipv6addr.s6_addr[v4offset], &ipv4addr, 4);
			memcpy(&ipv6addr.s6_addr[v4offset + 4], &psid16, 2);
			bmemcpy(&ipv6addr, &pd, pdlen);
		}

		++rulecnt;
		char ipv4addrbuf[INET_ADDRSTRLEN];
		char ipv4prefixbuf[INET_ADDRSTRLEN];
		char ipv6prefixbuf[INET6_ADDRSTRLEN];
		char ipv6addrbuf[INET6_ADDRSTRLEN];
		char pdbuf[INET6_ADDRSTRLEN];

		inet_ntop(AF_INET, &ipv4addr, ipv4addrbuf, sizeof(ipv4addrbuf));
		inet_ntop(AF_INET, &ipv4prefix, ipv4prefixbuf, sizeof(ipv4prefixbuf));
		inet_ntop(AF_INET6, &ipv6prefix, ipv6prefixbuf, sizeof(ipv6prefixbuf));
		inet_ntop(AF_INET6, &ipv6addr, ipv6addrbuf, sizeof(ipv6addrbuf));
		inet_ntop(AF_INET6, &pd, pdbuf, sizeof(pdbuf));

		printf("RULE_%d_FMR=%d\n", rulecnt, fmr);
		printf("RULE_%d_EALEN=%d\n", rulecnt, ealen);
		printf("RULE_%d_PSIDLEN=%d\n", rulecnt, psidlen);
		printf("RULE_%d_OFFSET=%d\n", rulecnt, offset);
		printf("RULE_%d_PREFIX4LEN=%d\n", rulecnt, prefix4len);
		printf("RULE_%d_PREFIX6LEN=%d\n", rulecnt, prefix6len);
		printf("RULE_%d_IPV4PREFIX=%s\n", rulecnt, ipv4prefixbuf);
		printf("RULE_%d_IPV6PREFIX=%s\n", rulecnt, ipv6prefixbuf);

		if (pdlen >= 0) {
			printf("RULE_%d_IPV6PD=%s\n", rulecnt, pdbuf);
			printf("RULE_%d_PD6LEN=%d\n", rulecnt, pdlen);
			printf("RULE_%d_PD6IFACE=%s\n", rulecnt, iface);
			printf("RULE_%d_IPV6ADDR=%s\n", rulecnt, ipv6addrbuf);
			printf("RULE_BMR=%d\n", rulecnt);
		}

		if (ipv4addr.s_addr) {
			printf("RULE_%d_IPV4ADDR=%s\n", rulecnt, ipv4addrbuf);
			printf("RULE_%d_ADDR4LEN=%d\n", rulecnt, addr4len);
		}


		if (psidlen > 0 && psid >= 0) {
			printf("RULE_%d_PORTSETS='", rulecnt);
			for (int k = (offset) ? 1 : 0; k < (1 << offset); ++k) {
				int start = (k << (16 - offset)) | (psid >> offset);
				int end = start + (1 << (16 - offset - psidlen)) - 1;

				if (start == 0)
					start = 1;

				if (start <= end)
					printf("%d-%d ", start, end);
			}
			printf("'\n");
		}

		if (dmr)
			printf("RULE_%d_DMR=%s\n", rulecnt, dmr);

		if (br)
			printf("RULE_%d_BR=%s\n", rulecnt, br);
	}

	printf("RULE_COUNT=%d\n", rulecnt);
	return status;
}
