/*
 * sfe_init.c
 *	Shortcut forwarding engine initialization.
 *
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <linux/etherdevice.h>
#include <linux/version.h>

#include "sfe_api.h"
#include "sfe.h"
#include "sfe_ipv4.h"
#include "sfe_ipv6.h"

int max_ipv4_conn = SFE_MAX_CONNECTION_NUM;
module_param(max_ipv4_conn, int, S_IRUGO);
MODULE_PARM_DESC(max_ipv4_conn, "Max number of IPv4 connections");

int max_ipv6_conn = SFE_MAX_CONNECTION_NUM;
module_param(max_ipv6_conn, int, S_IRUGO);
MODULE_PARM_DESC(max_ipv6_conn, "Max number of IPv6 connections");

/*
 * sfe_init()
 *	Initialize SFE engine.
 */
static int __init sfe_init(void)
{
	/*
	 * Initialize SFE IPv4 engine.
	 */
	if (sfe_ipv4_init()) {
		goto fail0;
	}

#ifdef SFE_SUPPORT_IPV6

	/*
	 * Initialize SFE IPv6 engine.
	 */
	if (sfe_ipv6_init()) {
		goto fail1;
	}
#endif

	/*
	 * Initialize SFE infrastructure and register SFE hook with Linux stack
	 */
	if (sfe_init_if()) {
		goto fail2;
	}

	return 0;

fail2:
#ifdef SFE_SUPPORT_IPV6
	sfe_ipv6_exit();
fail1:
#endif

	sfe_ipv4_exit();

fail0:

	return -1;
}

/*
 * sfe_exit()
 */
static void __exit sfe_exit(void)
{

	sfe_exit_if();

#ifdef SFE_SUPPORT_IPV6
	sfe_ipv6_exit();
#endif
	sfe_ipv4_exit();
}

module_init(sfe_init)
module_exit(sfe_exit)

MODULE_AUTHOR("Qualcomm Technologies");
MODULE_DESCRIPTION("Shortcut Forwarding Engine");
MODULE_LICENSE("Dual BSD/GPL");
