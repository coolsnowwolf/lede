/*
 * User space header to send message to the fast classifier
 *
 * Copyright (c) 2013,2016 The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/if_ether.h>

#define FAST_CLASSIFIER_GENL_VERSION	(1)
#define FAST_CLASSIFIER_GENL_NAME	"FC"
#define FAST_CLASSIFIER_GENL_MCGRP	"FC_MCGRP"
#define FAST_CLASSIFIER_GENL_HDRSIZE	(0)

enum {
	FAST_CLASSIFIER_A_UNSPEC,
	FAST_CLASSIFIER_A_TUPLE,
	__FAST_CLASSIFIER_A_MAX,
};

#define FAST_CLASSIFIER_A_MAX (__FAST_CLASSIFIER_A_MAX - 1)

enum {
	FAST_CLASSIFIER_C_UNSPEC,
	FAST_CLASSIFIER_C_OFFLOAD,
	FAST_CLASSIFIER_C_OFFLOADED,
	FAST_CLASSIFIER_C_DONE,
	__FAST_CLASSIFIER_C_MAX,
};

#define FAST_CLASSIFIER_C_MAX (__FAST_CLASSIFIER_C_MAX - 1)

struct fast_classifier_tuple {
	unsigned short ethertype;
	unsigned char proto;
	union {
		struct in_addr in;
		struct in6_addr in6;
	} src_saddr;
	union {
		struct in_addr in;
		struct in6_addr in6;
	} dst_saddr;
	unsigned short sport;
	unsigned short dport;
	unsigned char smac[ETH_ALEN];
	unsigned char dmac[ETH_ALEN];
};
