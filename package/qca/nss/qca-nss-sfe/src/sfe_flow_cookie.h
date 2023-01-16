/*
 * sfe_flow_cookie.h
 *	Flow cookie related callbacks.
 *
 * Copyright (c) 2013-2016, 2019-2020, The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_NF_FLOW_COOKIE
#define SFE_FLOW_COOKIE_SIZE 2048
#define SFE_FLOW_COOKIE_MASK 0x7ff

struct sfe_ipv4_connection_match;
struct sfe_ipv6_connection_match;

struct sfe_flow_cookie_entry {
	struct sfe_ipv4_connection_match *match;
	unsigned long last_clean_time;
};

struct sfe_ipv6_flow_cookie_entry {
	struct sfe_ipv6_connection_match *match;
	unsigned long last_clean_time;
};

typedef int (*flow_cookie_set_func_t)(u32 protocol, __be32 src_ip, __be16 src_port,
				      __be32 dst_ip, __be16 dst_port, u16 flow_cookie);
/*
 * sfe_register_flow_cookie_cb
 *	register a function in SFE to let SFE use this function to configure flow cookie for a flow
 *
 * Hardware driver which support flow cookie should register a callback function in SFE. Then SFE
 * can use this function to configure flow cookie for a flow.
 * return: 0, success; !=0, fail
 */
int sfe_register_flow_cookie_cb(flow_cookie_set_func_t cb);

/*
 * sfe_unregister_flow_cookie_cb
 *	unregister function which is used to configure flow cookie for a flow
 *
 * return: 0, success; !=0, fail
 */
int sfe_unregister_flow_cookie_cb(flow_cookie_set_func_t cb);

typedef int (*sfe_ipv6_flow_cookie_set_func_t)(u32 protocol, __be32 src_ip[4], __be16 src_port,
						__be32 dst_ip[4], __be16 dst_port, u16 flow_cookie);

/*
 * sfe_ipv6_register_flow_cookie_cb
 *	register a function in SFE to let SFE use this function to configure flow cookie for a flow
 *
 * Hardware driver which support flow cookie should register a callback function in SFE. Then SFE
 * can use this function to configure flow cookie for a flow.
 * return: 0, success; !=0, fail
 */
int sfe_ipv6_register_flow_cookie_cb(sfe_ipv6_flow_cookie_set_func_t cb);

/*
 * sfe_ipv6_unregister_flow_cookie_cb
 *	unregister function which is used to configure flow cookie for a flow
 *
 * return: 0, success; !=0, fail
 */
int sfe_ipv6_unregister_flow_cookie_cb(sfe_ipv6_flow_cookie_set_func_t cb);

#endif /*CONFIG_NF_FLOW_COOKIE*/
