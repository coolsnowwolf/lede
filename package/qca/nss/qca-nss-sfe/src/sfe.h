/*
 * sfe.h
 *	Shortcut forwarding engine.
 *
 * Copyright (c) 2013-2016, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#ifndef __SFE_H
#define __SFE_H

/*
 * Maximum number of accelerated IPv4 or IPv6 connections
 */
#if defined(SFE_MEM_PROFILE_LOW)
#define SFE_MAX_CONNECTION_NUM 512
#elif defined(SFE_MEM_PROFILE_MEDIUM)
#define SFE_MAX_CONNECTION_NUM 2048
#else
#define SFE_MAX_CONNECTION_NUM 4096
#endif

#define SFE_L2_PARSE_FLAGS_PPPOE_INGRESS 0x01	/* Indicates presence of a valid PPPoE header */

/*
 * IPv6 address structure
 */
struct sfe_ipv6_addr {
	__be32 addr[4];
};

typedef union {
	__be32			ip;
	struct sfe_ipv6_addr	ip6[1];
} sfe_ip_addr_t;

typedef enum sfe_sync_reason {
	SFE_SYNC_REASON_STATS,	/* Sync is to synchronize stats */
	SFE_SYNC_REASON_FLUSH,	/* Sync is to flush a entry */
	SFE_SYNC_REASON_DESTROY	/* Sync is to destroy a entry(requested by connection manager) */
} sfe_sync_reason_t;

/*
 * Structure used to store L2 information
 */
struct sfe_l2_info {
	u16 parse_flags;	/* Flags indicating L2.5 headers presence */
	u16 l2_hdr_offset;	/* Offset of L2 header */
	u16 l2_hdr_size;	/* L2 header size */
	u16 pppoe_hdr_offset;	/* PPPOE header offset */
	u16 protocol;		/* L3 Protocol */
};

/*
 * Structure used to sync connection stats/state back within the system.
 *
 * NOTE: The addresses here are NON-NAT addresses, i.e. the true endpoint addressing.
 * 'src' is the creator of the connection.
 */
struct sfe_connection_sync {
	struct net_device *src_dev;
	struct net_device *dest_dev;
	int is_v6;			/* Is it for ipv6? */
	int protocol;			/* IP protocol number (IPPROTO_...) */
	sfe_ip_addr_t src_ip;		/* Non-NAT source address, i.e. the creator of the connection */
	sfe_ip_addr_t src_ip_xlate;	/* NATed source address */
	__be16 src_port;		/* Non-NAT source port */
	__be16 src_port_xlate;		/* NATed source port */
	sfe_ip_addr_t dest_ip;		/* Non-NAT destination address, i.e. to whom the connection was created */
	sfe_ip_addr_t dest_ip_xlate;	/* NATed destination address */
	__be16 dest_port;		/* Non-NAT destination port */
	__be16 dest_port_xlate;		/* NATed destination port */
	u32 src_td_max_window;
	u32 src_td_end;
	u32 src_td_max_end;
	u64 src_packet_count;
	u64 src_byte_count;
	u32 src_new_packet_count;
	u32 src_new_byte_count;
	u32 dest_td_max_window;
	u32 dest_td_end;
	u32 dest_td_max_end;
	u64 dest_packet_count;
	u64 dest_byte_count;
	u32 dest_new_packet_count;
	u32 dest_new_byte_count;
	u32 reason;		/* reason for stats sync message, i.e. destroy, flush, period sync */
	u64 delta_jiffies;		/* Time to be added to the current timeout to keep the connection alive */
};

/*
 * connection mark structure
 */
struct sfe_connection_mark {
	int protocol;
	sfe_ip_addr_t src_ip;
	sfe_ip_addr_t dest_ip;
	__be16 src_port;
	__be16 dest_port;
	u32 mark;
};

/*
 * Expose the hook for the receive processing.
 */
extern int (*athrs_fast_nat_recv)(struct sk_buff *skb);

/*
 * Expose what should be a static flag in the TCP connection tracker.
 */
extern int nf_ct_tcp_no_window_check;

/*
 * This callback will be called in a timer
 * at 100 times per second to sync stats back to
 * Linux connection track.
 *
 * A RCU lock is taken to prevent this callback
 * from unregistering.
 */
typedef void (*sfe_sync_rule_callback_t)(struct sfe_connection_sync *);

/*
 * IPv4 APIs used by connection manager
 */
int sfe_ipv4_recv(struct net_device *dev, struct sk_buff *skb, struct sfe_l2_info *l2_info, bool tun_outer);
int sfe_ipv4_create_rule(struct sfe_ipv4_rule_create_msg *msg);
void sfe_ipv4_destroy_rule(struct sfe_ipv4_rule_destroy_msg *msg);
void sfe_ipv4_destroy_all_rules_for_dev(struct net_device *dev);
void sfe_ipv4_register_sync_rule_callback(sfe_sync_rule_callback_t callback);
void sfe_ipv4_update_rule(struct sfe_ipv4_rule_create_msg *msg);

#ifdef SFE_SUPPORT_IPV6
/*
 * IPv6 APIs used by connection manager
 */
int sfe_ipv6_recv(struct net_device *dev, struct sk_buff *skb, struct sfe_l2_info *l2_info, bool tun_outer);
int sfe_ipv6_create_rule(struct sfe_ipv6_rule_create_msg *msg);
void sfe_ipv6_destroy_rule(struct sfe_ipv6_rule_destroy_msg *msg);
void sfe_ipv6_destroy_all_rules_for_dev(struct net_device *dev);
void sfe_ipv6_register_sync_rule_callback(sfe_sync_rule_callback_t callback);
void sfe_ipv6_update_rule(struct sfe_ipv6_rule_create_msg *msg);
#else
static inline int sfe_ipv6_recv(struct net_device *dev, struct sk_buff *skb, struct sfe_l2_info *l2_info, bool tun_outer)
{
	return 0;
}

static inline int sfe_ipv6_create_rule(struct sfe_ipv6_rule_create_msg *msg)
{
	return 0;
}

static inline void sfe_ipv6_destroy_rule(struct sfe_ipv6_rule_destroy_msg *msg)
{
	return;
}

static inline void sfe_ipv6_destroy_all_rules_for_dev(struct net_device *dev)
{
	return;
}

static inline void sfe_ipv6_register_sync_rule_callback(sfe_sync_rule_callback_t callback)
{
	return;
}

static inline void sfe_ipv6_update_rule(struct sfe_ipv6_rule_create_msg *msg)
{
	return;
}

#endif

/*
 * sfe_ipv6_addr_equal()
 *	compare ipv6 address
 *
 * return: 1, equal; 0, no equal
 */
static inline int sfe_ipv6_addr_equal(struct sfe_ipv6_addr *a,
				      struct sfe_ipv6_addr *b)
{
	return a->addr[0] == b->addr[0] &&
	       a->addr[1] == b->addr[1] &&
	       a->addr[2] == b->addr[2] &&
	       a->addr[3] == b->addr[3];
}

/*
 * sfe_ipv4_addr_equal()
 *	compare ipv4 address
 *
 * return: 1, equal; 0, no equal
 */
#define sfe_ipv4_addr_equal(a, b) ((u32)(a) == (u32)(b))

/*
 * sfe_addr_equal()
 *	compare ipv4 or ipv6 address
 *
 * return: 1, equal; 0, no equal
 */
static inline int sfe_addr_equal(sfe_ip_addr_t *a,
				 sfe_ip_addr_t *b, int is_v4)
{
	return is_v4 ? sfe_ipv4_addr_equal(a->ip, b->ip) : sfe_ipv6_addr_equal(a->ip6, b->ip6);
}

/*
 * sfe_l2_parse_flag_set()
 *	Set L2 parse flag
 */
static inline void sfe_l2_parse_flag_set(struct sfe_l2_info *l2_info, u16 flag)
{
	l2_info->parse_flags |= flag;
}

/*
 * sfe_l2_parse_flag_get()
 *	Get L2 parse flag
 */
static inline u16 sfe_l2_parse_flag_get(struct sfe_l2_info *l2_info)
{
	return l2_info->parse_flags;
}

/*
 * sfe_l2_parse_flag_check()
 *	Check L2 parse flag
 */
static inline bool sfe_l2_parse_flag_check(struct sfe_l2_info *l2_info, u16 flag)
{
	return !!(l2_info->parse_flags & flag);
}

/*
 * sfe_l2_hdr_offset_get()
 *	Get L2 header offset
 */
static inline u16 sfe_l2_hdr_offset_get(struct sfe_l2_info *l2_info)
{
	return l2_info->l2_hdr_offset;
}

/*
 * sfe_l2_hdr_offset_set()
 *	Set L2 header offset
 */
static inline void sfe_l2_hdr_offset_set(struct sfe_l2_info *l2_info, u16 offset)
{
	l2_info->l2_hdr_offset = offset;
}

/*
 * sfe_l2_pppoe_hdr_offset_get()
 *	Get L2 PPPoE header offset
 */
static inline u16 sfe_l2_pppoe_hdr_offset_get(struct sfe_l2_info *l2_info)
{
	return l2_info->pppoe_hdr_offset;
}

/*
 * sfe_l2_pppoe_hdr_offset_set()
 *	Set L2 PPPoE header offset
 */
static inline void sfe_l2_pppoe_hdr_offset_set(struct sfe_l2_info *l2_info, u16 offset)
{
	l2_info->pppoe_hdr_offset = offset;
}

/*
 * sfe_l2_hdr_size_get()
 *	Get L2 header size
 */
static inline u16 sfe_l2_hdr_size_get(struct sfe_l2_info *l2_info)
{
	return l2_info->l2_hdr_size;
}

/*
 * sfe_l2_hdr_size_set()
 *	Set L2 header size
 */
static inline void sfe_l2_hdr_size_set(struct sfe_l2_info *l2_info, u16 size)
{
	l2_info->l2_hdr_size = size;
}

/*
 * sfe_l2_protocol_get()
 *	Get L2 protocol
 */
static inline u16 sfe_l2_protocol_get(struct sfe_l2_info *l2_info)
{
	return l2_info->protocol;
}

/*
 * sfe_l2_protocol_set()
 *	Set L2 protocol
 */
static inline void sfe_l2_protocol_set(struct sfe_l2_info *l2_info, u16 proto)
{
	l2_info->protocol = proto;
}

int sfe_init_if(void);
void sfe_exit_if(void);

#endif /* __SFE_H */
