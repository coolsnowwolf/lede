/*
 * sfe_ipv6.c
 *	Shortcut forwarding engine - IPv6 support.
 *
 * Copyright (c) 2015-2016, 2019-2020, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <linux/etherdevice.h>
#include <linux/version.h>
#include <net/udp.h>
#include <net/vxlan.h>
#include <linux/refcount.h>
#include <linux/netfilter.h>
#include <linux/inetdevice.h>
#include <linux/netfilter_ipv6.h>
#include "sfe_debug.h"
#include "sfe_api.h"
#include "sfe.h"
#include "sfe_flow_cookie.h"
#include "sfe_ipv6.h"
#include "sfe_ipv6_udp.h"
#include "sfe_ipv6_tcp.h"
#include "sfe_ipv6_icmp.h"

#define sfe_ipv6_addr_copy(src, dest) memcpy((void *)(dest), (void *)(src), 16)

static char *sfe_ipv6_exception_events_string[SFE_IPV6_EXCEPTION_EVENT_LAST] = {
	"UDP_HEADER_INCOMPLETE",
	"UDP_NO_CONNECTION",
	"UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT",
	"UDP_SMALL_TTL",
	"UDP_NEEDS_FRAGMENTATION",
	"TCP_HEADER_INCOMPLETE",
	"TCP_NO_CONNECTION_SLOW_FLAGS",
	"TCP_NO_CONNECTION_FAST_FLAGS",
	"TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT",
	"TCP_SMALL_TTL",
	"TCP_NEEDS_FRAGMENTATION",
	"TCP_FLAGS",
	"TCP_SEQ_EXCEEDS_RIGHT_EDGE",
	"TCP_SMALL_DATA_OFFS",
	"TCP_BAD_SACK",
	"TCP_BIG_DATA_OFFS",
	"TCP_SEQ_BEFORE_LEFT_EDGE",
	"TCP_ACK_EXCEEDS_RIGHT_EDGE",
	"TCP_ACK_BEFORE_LEFT_EDGE",
	"ICMP_HEADER_INCOMPLETE",
	"ICMP_UNHANDLED_TYPE",
	"ICMP_IPV6_HEADER_INCOMPLETE",
	"ICMP_IPV6_NON_V6",
	"ICMP_IPV6_IP_OPTIONS_INCOMPLETE",
	"ICMP_IPV6_UDP_HEADER_INCOMPLETE",
	"ICMP_IPV6_TCP_HEADER_INCOMPLETE",
	"ICMP_IPV6_UNHANDLED_PROTOCOL",
	"ICMP_NO_CONNECTION",
	"ICMP_FLUSHED_CONNECTION",
	"HEADER_INCOMPLETE",
	"BAD_TOTAL_LENGTH",
	"NON_V6",
	"NON_INITIAL_FRAGMENT",
	"DATAGRAM_INCOMPLETE",
	"IP_OPTIONS_INCOMPLETE",
	"UNHANDLED_PROTOCOL",
	"FLOW_COOKIE_ADD_FAIL"
};

static struct sfe_ipv6 __si6;

/*
 * sfe_ipv6_get_debug_dev()
 */
static ssize_t sfe_ipv6_get_debug_dev(struct device *dev, struct device_attribute *attr, char *buf);

/*
 * sysfs attributes.
 */
static const struct device_attribute sfe_ipv6_debug_dev_attr =
	__ATTR(debug_dev, S_IWUSR | S_IRUGO, sfe_ipv6_get_debug_dev, NULL);

/*
 * sfe_ipv6_get_connection_match_hash()
 *	Generate the hash used in connection match lookups.
 */
static inline unsigned int sfe_ipv6_get_connection_match_hash(struct net_device *dev, u8 protocol,
							      struct sfe_ipv6_addr *src_ip, __be16 src_port,
							      struct sfe_ipv6_addr *dest_ip, __be16 dest_port)
{
	u32 idx, hash = 0;
	size_t dev_addr = (size_t)dev;

	for (idx = 0; idx < 4; idx++) {
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
	}
	hash = ((u32)dev_addr) ^ hash ^ protocol ^ ntohs(src_port ^ dest_port);
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
}

/*
 * sfe_ipv6_find_connection_match_rcu()
 *	Get the IPv6 flow match info that corresponds to a particular 5-tuple.
 */
struct sfe_ipv6_connection_match *
sfe_ipv6_find_connection_match_rcu(struct sfe_ipv6 *si, struct net_device *dev, u8 protocol,
					struct sfe_ipv6_addr *src_ip, __be16 src_port,
					struct sfe_ipv6_addr *dest_ip, __be16 dest_port)
{
	struct sfe_ipv6_connection_match *cm = NULL;
	unsigned int conn_match_idx;
	struct hlist_head *lhead;
	WARN_ON_ONCE(!rcu_read_lock_held());

	conn_match_idx = sfe_ipv6_get_connection_match_hash(dev, protocol, src_ip, src_port, dest_ip, dest_port);

	lhead = &si->hlist_conn_match_hash_head[conn_match_idx];

	/*
	 * Hopefully the first entry is the one we want.
	 */
	hlist_for_each_entry_rcu(cm, lhead, hnode) {
		if ((cm->match_dest_port != dest_port) ||
		    (!sfe_ipv6_addr_equal(cm->match_src_ip, src_ip)) ||
		    (!sfe_ipv6_addr_equal(cm->match_dest_ip, dest_ip)) ||
		    (cm->match_protocol != protocol) ||
		    (cm->match_dev != dev)) {
			continue;
		}

		this_cpu_inc(si->stats_pcpu->connection_match_hash_hits64);

		break;

	}

	return cm;
}

/*
 * sfe_ipv6_connection_match_update_summary_stats()
 *	Update the summary stats for a connection match entry.
 */
static inline void sfe_ipv6_connection_match_update_summary_stats(struct sfe_ipv6_connection_match *cm,
                                               u32 *packets, u32 *bytes)

{
	u32 packet_count, byte_count;

	packet_count = atomic_read(&cm->rx_packet_count);
	cm->rx_packet_count64 += packet_count;
	atomic_sub(packet_count, &cm->rx_packet_count);

	byte_count = atomic_read(&cm->rx_byte_count);
	cm->rx_byte_count64 += byte_count;
	atomic_sub(byte_count, &cm->rx_byte_count);

	*packets = packet_count;
	*bytes = byte_count;
}

/*
 * sfe_ipv6_connection_match_compute_translations()
 *	Compute port and address translations for a connection match entry.
 */
static void sfe_ipv6_connection_match_compute_translations(struct sfe_ipv6_connection_match *cm)
{
	u32 diff[9];
	u32 *idx_32;
	u16 *idx_16;

	/*
	 * Before we insert the entry look to see if this is tagged as doing address
	 * translations.  If it is then work out the adjustment that we need to apply
	 * to the transport checksum.
	 */
	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC) {
		u32 adj = 0;
		u32 carry = 0;

		/*
		 * Precompute an incremental checksum adjustment so we can
		 * edit packets in this stream very quickly.  The algorithm is from RFC1624.
		 */
		idx_32 = diff;
		*(idx_32++) = cm->match_src_ip[0].addr[0];
		*(idx_32++) = cm->match_src_ip[0].addr[1];
		*(idx_32++) = cm->match_src_ip[0].addr[2];
		*(idx_32++) = cm->match_src_ip[0].addr[3];

		idx_16 = (u16 *)idx_32;
		*(idx_16++) = cm->match_src_port;
		*(idx_16++) = ~cm->xlate_src_port;
		idx_32 = (u32 *)idx_16;

		*(idx_32++) = ~cm->xlate_src_ip[0].addr[0];
		*(idx_32++) = ~cm->xlate_src_ip[0].addr[1];
		*(idx_32++) = ~cm->xlate_src_ip[0].addr[2];
		*(idx_32++) = ~cm->xlate_src_ip[0].addr[3];

		/*
		 * When we compute this fold it down to a 16-bit offset
		 * as that way we can avoid having to do a double
		 * folding of the twos-complement result because the
		 * addition of 2 16-bit values cannot cause a double
		 * wrap-around!
		 */
		for (idx_32 = diff; idx_32 < diff + 9; idx_32++) {
			u32 w = *idx_32;
			adj += carry;
			adj += w;
			carry = (w > adj);
		}
		adj += carry;
		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_src_csum_adjustment = (u16)adj;
	}

	if (cm->flags & SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST) {
		u32 adj = 0;
		u32 carry = 0;

		/*
		 * Precompute an incremental checksum adjustment so we can
		 * edit packets in this stream very quickly.  The algorithm is from RFC1624.
		 */
		idx_32 = diff;
		*(idx_32++) = cm->match_dest_ip[0].addr[0];
		*(idx_32++) = cm->match_dest_ip[0].addr[1];
		*(idx_32++) = cm->match_dest_ip[0].addr[2];
		*(idx_32++) = cm->match_dest_ip[0].addr[3];

		idx_16 = (u16 *)idx_32;
		*(idx_16++) = cm->match_dest_port;
		*(idx_16++) = ~cm->xlate_dest_port;
		idx_32 = (u32 *)idx_16;

		*(idx_32++) = ~cm->xlate_dest_ip[0].addr[0];
		*(idx_32++) = ~cm->xlate_dest_ip[0].addr[1];
		*(idx_32++) = ~cm->xlate_dest_ip[0].addr[2];
		*(idx_32++) = ~cm->xlate_dest_ip[0].addr[3];

		/*
		 * When we compute this fold it down to a 16-bit offset
		 * as that way we can avoid having to do a double
		 * folding of the twos-complement result because the
		 * addition of 2 16-bit values cannot cause a double
		 * wrap-around!
		 */
		for (idx_32 = diff; idx_32 < diff + 9; idx_32++) {
			u32 w = *idx_32;
			adj += carry;
			adj += w;
			carry = (w > adj);
		}
		adj += carry;
		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_dest_csum_adjustment = (u16)adj;
	}
}

/*
 * sfe_ipv6_update_summary_stats()
 *	Update the summary stats.
 */
static void sfe_ipv6_update_summary_stats(struct sfe_ipv6 *si, struct sfe_ipv6_stats *stats)
{
	int i = 0;

	memset(stats, 0, sizeof(*stats));

	for_each_possible_cpu(i) {
		const struct sfe_ipv6_stats *s = per_cpu_ptr(si->stats_pcpu, i);

		stats->connection_create_requests64 += s->connection_create_requests64;
		stats->connection_create_collisions64 += s->connection_create_collisions64;
		stats->connection_create_failures64 += s->connection_create_failures64;
		stats->connection_destroy_requests64 += s->connection_destroy_requests64;
		stats->connection_destroy_misses64 += s->connection_destroy_misses64;
		stats->connection_match_hash_hits64 += s->connection_match_hash_hits64;
		stats->connection_match_hash_reorders64 += s->connection_match_hash_reorders64;
		stats->connection_flushes64 += s->connection_flushes64;
		stats->packets_dropped64 += s->packets_dropped64;
		stats->packets_forwarded64 += s->packets_forwarded64;
		stats->packets_not_forwarded64 += s->packets_not_forwarded64;
		stats->pppoe_encap_packets_forwarded64 += s->pppoe_encap_packets_forwarded64;
		stats->pppoe_decap_packets_forwarded64 += s->pppoe_decap_packets_forwarded64;
	}
}

/*
 * sfe_ipv6_insert_connection_match()
 *	Insert a connection match into the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline void sfe_ipv6_insert_connection_match(struct sfe_ipv6 *si,
						    struct sfe_ipv6_connection_match *cm)
{
	unsigned int conn_match_idx
		= sfe_ipv6_get_connection_match_hash(cm->match_dev, cm->match_protocol,
						     cm->match_src_ip, cm->match_src_port,
						     cm->match_dest_ip, cm->match_dest_port);

	lockdep_assert_held(&si->lock);

	hlist_add_head_rcu(&cm->hnode, &si->hlist_conn_match_hash_head[conn_match_idx]);
#ifdef CONFIG_NF_FLOW_COOKIE
	if (!si->flow_cookie_enable || !(cm->flags & (SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_SRC | SFE_IPV6_CONNECTION_MATCH_FLAG_XLATE_DEST)))
		return;

	/*
	 * Configure hardware to put a flow cookie in packet of this flow,
	 * then we can accelerate the lookup process when we received this packet.
	 */
	for (conn_match_idx = 1; conn_match_idx < SFE_FLOW_COOKIE_SIZE; conn_match_idx++) {
		struct sfe_ipv6_flow_cookie_entry *entry = &si->sfe_flow_cookie_table[conn_match_idx];

		if ((NULL == entry->match) && time_is_before_jiffies(entry->last_clean_time + HZ)) {
			sfe_ipv6_flow_cookie_set_func_t func;

			rcu_read_lock();
			func = rcu_dereference(si->flow_cookie_set_func);
			if (func) {
				if (!func(cm->match_protocol, cm->match_src_ip->addr, cm->match_src_port,
					 cm->match_dest_ip->addr, cm->match_dest_port, conn_match_idx)) {
					entry->match = cm;
					cm->flow_cookie = conn_match_idx;
				} else {
					si->exception_events[SFE_IPV6_EXCEPTION_EVENT_FLOW_COOKIE_ADD_FAIL]++;
				}
			}
			rcu_read_unlock();

			break;
		}
	}
#endif
}

/*
 * sfe_ipv6_remove_connection_match()
 *	Remove a connection match object from the hash.
 */
static inline void sfe_ipv6_remove_connection_match(struct sfe_ipv6 *si, struct sfe_ipv6_connection_match *cm)
{

	lockdep_assert_held(&si->lock);
#ifdef CONFIG_NF_FLOW_COOKIE
	if (si->flow_cookie_enable) {
		/*
		 * Tell hardware that we no longer need a flow cookie in packet of this flow
		 */
		unsigned int conn_match_idx;

		for (conn_match_idx = 1; conn_match_idx < SFE_FLOW_COOKIE_SIZE; conn_match_idx++) {
			struct sfe_ipv6_flow_cookie_entry *entry = &si->sfe_flow_cookie_table[conn_match_idx];

			if (cm == entry->match) {
				sfe_ipv6_flow_cookie_set_func_t func;

				rcu_read_lock();
				func = rcu_dereference(si->flow_cookie_set_func);
				if (func) {
					func(cm->match_protocol, cm->match_src_ip->addr, cm->match_src_port,
					     cm->match_dest_ip->addr, cm->match_dest_port, 0);
				}
				rcu_read_unlock();

				cm->flow_cookie = 0;
				entry->match = NULL;
				entry->last_clean_time = jiffies;
				break;
			}
		}
	}
#endif
	hlist_del_init_rcu(&cm->hnode);

}

/*
 * sfe_ipv6_get_connection_hash()
 *	Generate the hash used in connection lookups.
 */
static inline unsigned int sfe_ipv6_get_connection_hash(u8 protocol, struct sfe_ipv6_addr *src_ip, __be16 src_port,
							struct sfe_ipv6_addr *dest_ip, __be16 dest_port)
{
	u32 idx, hash = 0;

	for (idx = 0; idx < 4; idx++) {
		hash ^= src_ip->addr[idx] ^ dest_ip->addr[idx];
	}
	hash = hash ^ protocol ^ ntohs(src_port ^ dest_port);
	return ((hash >> SFE_IPV6_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV6_CONNECTION_HASH_MASK;
}

/*
 * sfe_ipv6_find_connection()
 *	Get the IPv6 connection info that corresponds to a particular 5-tuple.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline struct sfe_ipv6_connection *sfe_ipv6_find_connection(struct sfe_ipv6 *si, u32 protocol,
								   struct sfe_ipv6_addr *src_ip, __be16 src_port,
								   struct sfe_ipv6_addr *dest_ip, __be16 dest_port)
{
	struct sfe_ipv6_connection *c;

	unsigned int conn_idx = sfe_ipv6_get_connection_hash(protocol, src_ip, src_port, dest_ip, dest_port);

	lockdep_assert_held(&si->lock);
	c = si->conn_hash[conn_idx];

	while (c) {
		if ((c->src_port == src_port)
		    && (c->dest_port == dest_port)
		    && (sfe_ipv6_addr_equal(c->src_ip, src_ip))
		    && (sfe_ipv6_addr_equal(c->dest_ip, dest_ip))
		    && (c->protocol == protocol)) {
			return c;
		}
		c = c->next;
	}

	return NULL;
}

/*
 * sfe_ipv6_insert_connection()
 *	Insert a connection into the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static void sfe_ipv6_insert_connection(struct sfe_ipv6 *si, struct sfe_ipv6_connection *c)
{
	struct sfe_ipv6_connection **hash_head;
	struct sfe_ipv6_connection *prev_head;
	unsigned int conn_idx;

	lockdep_assert_held(&si->lock);

	/*
	 * Insert entry into the connection hash.
	 */
	conn_idx = sfe_ipv6_get_connection_hash(c->protocol, c->src_ip, c->src_port,
						c->dest_ip, c->dest_port);
	hash_head = &si->conn_hash[conn_idx];
	prev_head = *hash_head;
	c->prev = NULL;
	if (prev_head) {
		prev_head->prev = c;
	}

	c->next = prev_head;
	*hash_head = c;

	/*
	 * Insert entry into the "all connections" list.
	 */
	if (si->all_connections_tail) {
		c->all_connections_prev = si->all_connections_tail;
		si->all_connections_tail->all_connections_next = c;
	} else {
		c->all_connections_prev = NULL;
		si->all_connections_head = c;
	}

	si->all_connections_tail = c;
	c->all_connections_next = NULL;
	si->num_connections++;

	/*
	 * Insert the connection match objects too.
	 */
	sfe_ipv6_insert_connection_match(si, c->original_match);
	sfe_ipv6_insert_connection_match(si, c->reply_match);
}

/*
 * sfe_ipv6_remove_connection()
 *	Remove a sfe_ipv6_connection object from the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
bool sfe_ipv6_remove_connection(struct sfe_ipv6 *si, struct sfe_ipv6_connection *c)
{

	lockdep_assert_held(&si->lock);
	if (c->removed) {
		DEBUG_ERROR("%px: Connection has been removed already\n", c);
		return false;
	}

	/*
	 * Remove the connection match objects.
	 */
	sfe_ipv6_remove_connection_match(si, c->reply_match);
	sfe_ipv6_remove_connection_match(si, c->original_match);

	/*
	 * Unlink the connection.
	 */
	if (c->prev) {
		c->prev->next = c->next;
	} else {
		unsigned int conn_idx = sfe_ipv6_get_connection_hash(c->protocol, c->src_ip, c->src_port,
								     c->dest_ip, c->dest_port);
		si->conn_hash[conn_idx] = c->next;
	}

	if (c->next) {
		c->next->prev = c->prev;
	}

	/*
	 * Unlink connection from all_connections list
	 */
	if (c->all_connections_prev) {
		c->all_connections_prev->all_connections_next = c->all_connections_next;
	} else {
		si->all_connections_head = c->all_connections_next;
	}

	if (c->all_connections_next) {
		c->all_connections_next->all_connections_prev = c->all_connections_prev;
	} else {
		si->all_connections_tail = c->all_connections_prev;
	}

	/*
	 * If I am the next sync connection, move the sync to my next or head.
	 */
	if (unlikely(si->wc_next == c)) {
		si->wc_next = c->all_connections_next;
	}

	c->removed = true;
	si->num_connections--;
	return true;
}

/*
 * sfe_ipv6_gen_sync_connection()
 *	Sync a connection.
 *
 * On entry to this function we expect that the lock for the connection is either
 * already held (while called from sfe_ipv6_periodic_sync() or isn't required
 * (while called from sfe_ipv6_flush_sfe_ipv6_connection())
 */
static void sfe_ipv6_gen_sync_connection(struct sfe_ipv6 *si, struct sfe_ipv6_connection *c,
					struct sfe_connection_sync *sis, sfe_sync_reason_t reason,
					u64 now_jiffies)
{
	struct sfe_ipv6_connection_match *original_cm;
	struct sfe_ipv6_connection_match *reply_cm;
	u32 packet_count, byte_count;

	/*
	 * Fill in the update message.
	 */
	sis->is_v6 = 1;
	sis->protocol = c->protocol;
	sis->src_ip.ip6[0] = c->src_ip[0];
	sis->src_ip_xlate.ip6[0] = c->src_ip_xlate[0];
	sis->dest_ip.ip6[0] = c->dest_ip[0];
	sis->dest_ip_xlate.ip6[0] = c->dest_ip_xlate[0];
	sis->src_port = c->src_port;
	sis->src_port_xlate = c->src_port_xlate;
	sis->dest_port = c->dest_port;
	sis->dest_port_xlate = c->dest_port_xlate;

	original_cm = c->original_match;
	reply_cm = c->reply_match;
	sis->src_td_max_window = original_cm->protocol_state.tcp.max_win;
	sis->src_td_end = original_cm->protocol_state.tcp.end;
	sis->src_td_max_end = original_cm->protocol_state.tcp.max_end;
	sis->dest_td_max_window = reply_cm->protocol_state.tcp.max_win;
	sis->dest_td_end = reply_cm->protocol_state.tcp.end;
	sis->dest_td_max_end = reply_cm->protocol_state.tcp.max_end;

	sfe_ipv6_connection_match_update_summary_stats(original_cm, &packet_count, &byte_count);
	sis->src_new_packet_count = packet_count;
	sis->src_new_byte_count = byte_count;

	sfe_ipv6_connection_match_update_summary_stats(reply_cm, &packet_count, &byte_count);
	sis->dest_new_packet_count = packet_count;
	sis->dest_new_byte_count = byte_count;

	sis->src_dev = original_cm->match_dev;
	sis->src_packet_count = original_cm->rx_packet_count64;
	sis->src_byte_count = original_cm->rx_byte_count64;

	sis->dest_dev = reply_cm->match_dev;
	sis->dest_packet_count = reply_cm->rx_packet_count64;
	sis->dest_byte_count = reply_cm->rx_byte_count64;

	sis->reason = reason;

	/*
	 * Get the time increment since our last sync.
	 */
	sis->delta_jiffies = now_jiffies - c->last_sync_jiffies;
	c->last_sync_jiffies = now_jiffies;
}

/*
 * sfe_ipv6_free_sfe_ipv6_connection_rcu()
 *	Called at RCU qs state to free the connection object.
 */
static void sfe_ipv6_free_sfe_ipv6_connection_rcu(struct rcu_head *head)
{
	struct sfe_ipv6_connection *c;
	struct udp_sock *up;
	struct sock *sk;

	/*
	 * We dont need spin lock as the connection is already removed from link list
	 */
	c = container_of(head, struct sfe_ipv6_connection, rcu);
	BUG_ON(!c->removed);

	DEBUG_TRACE("%px: connecton has been deleted\n", c);

	/*
	 * Decrease the refcount taken in function sfe_ipv6_create_rule()
	 * during call of __udp6_lib_lookup()
	 */
	up = c->reply_match->up;
	if (up) {
		sk = (struct sock *)up;
		sock_put(sk);
	}

	/*
	 * Release our hold of the source and dest devices and free the memory
	 * for our connection objects.
	 */
	dev_put(c->original_dev);
	dev_put(c->reply_dev);
	kfree(c->original_match);
	kfree(c->reply_match);
	kfree(c);
}

/*
 * sfe_ipv6_sync_status()
 *	update a connection status to its connection manager.
 *
 * si: the ipv6 context
 * c: which connection to be notified
 * reason: what kind of reason: flush, or destroy
 */
void sfe_ipv6_sync_status(struct sfe_ipv6 *si,
				      struct sfe_ipv6_connection *c,
				      sfe_sync_reason_t reason)
{
	struct sfe_connection_sync sis;
	u64 now_jiffies;
	sfe_sync_rule_callback_t sync_rule_callback;

	rcu_read_lock();
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);

	if (unlikely(!sync_rule_callback)) {
		rcu_read_unlock();
		return;
	}

	/*
	 * Generate a sync message and then sync.
	 */
	now_jiffies = get_jiffies_64();
	sfe_ipv6_gen_sync_connection(si, c, &sis, reason, now_jiffies);
	sync_rule_callback(&sis);

	rcu_read_unlock();
}

/*
 * sfe_ipv6_flush_connection()
 *	Flush a connection and free all associated resources.
 *
 * We need to be called with bottom halves disabled locally as we need to acquire
 * the connection hash lock and release it again.  In general we're actually called
 * from within a BH and so we're fine, but we're also called when connections are
 * torn down.
 */
void sfe_ipv6_flush_connection(struct sfe_ipv6 *si,
				      struct sfe_ipv6_connection *c,
				      sfe_sync_reason_t reason)
{
	BUG_ON(!c->removed);

	this_cpu_inc(si->stats_pcpu->connection_flushes64);
	sfe_ipv6_sync_status(si, c, reason);

	/*
	 * Release our hold of the source and dest devices and free the memory
	 * for our connection objects.
	 */
	call_rcu(&c->rcu, sfe_ipv6_free_sfe_ipv6_connection_rcu);
}

 /*
 * sfe_ipv6_exception_stats_inc()
 *	Increment exception stats.
 */
void sfe_ipv6_exception_stats_inc(struct sfe_ipv6 *si, enum sfe_ipv6_exception_events reason)
{
	struct sfe_ipv6_stats *stats = this_cpu_ptr(si->stats_pcpu);

	stats->exception_events64[reason]++;
	stats->packets_not_forwarded64++;
}

/*
 * sfe_ipv6_recv()
 *	Handle packet receives and forwaring.
 *
 * Returns 1 if the packet is forwarded or 0 if it isn't.
 */
int sfe_ipv6_recv(struct net_device *dev, struct sk_buff *skb, struct sfe_l2_info *l2_info, bool tun_outer)
{
	struct sfe_ipv6 *si = &__si6;
	unsigned int len;
	unsigned int payload_len;
	unsigned int ihl = sizeof(struct ipv6hdr);
	bool sync_on_find = false;
	struct ipv6hdr *iph;
	u8 next_hdr;

	/*
	 * Check that we have space for an IP header and an uplayer header here.
	 */
	len = skb->len;
	if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE);
		DEBUG_TRACE("len: %u is too short\n", len);
		return 0;
	}

	/*
	 * Is our IP version wrong?
	 */
	iph = (struct ipv6hdr *)skb->data;
	if (unlikely(iph->version != 6)) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_NON_V6);
		DEBUG_TRACE("IP version: %u\n", iph->version);
		return 0;
	}

	/*
	 * Does our datagram fit inside the skb?
	 */
	payload_len = ntohs(iph->payload_len);
	if (unlikely(payload_len > (len - ihl))) {

		sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE);
		DEBUG_TRACE("payload_len: %u, exceeds len: %u\n", payload_len, (len - (unsigned int)sizeof(struct ipv6hdr)));
		return 0;
	}

	next_hdr = iph->nexthdr;
	while (unlikely(sfe_ipv6_is_ext_hdr(next_hdr))) {
		struct sfe_ipv6_ext_hdr *ext_hdr;
		unsigned int ext_hdr_len;

		ext_hdr = (struct sfe_ipv6_ext_hdr *)(skb->data + ihl);

		ext_hdr_len = ext_hdr->hdr_len;
		ext_hdr_len <<= 3;
		ext_hdr_len += sizeof(struct sfe_ipv6_ext_hdr);
		ihl += ext_hdr_len;
		if (!pskb_may_pull(skb, ihl + sizeof(struct sfe_ipv6_ext_hdr))) {
			sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_HEADER_INCOMPLETE);

			DEBUG_TRACE("extension header %d not completed\n", next_hdr);
			return 0;
		}
		/*
		 * Any packets have extend hdr, won't be handled in the fast
		 * path,sync its status and exception to the kernel.
		 */
		sync_on_find = true;
		next_hdr = ext_hdr->next_hdr;
	}

	if (IPPROTO_UDP == next_hdr) {
		return sfe_ipv6_recv_udp(si, skb, dev, len, iph, ihl, sync_on_find, l2_info, tun_outer);
	}

	if (IPPROTO_TCP == next_hdr) {
		return sfe_ipv6_recv_tcp(si, skb, dev, len, iph, ihl, sync_on_find, l2_info);
	}

	if (IPPROTO_ICMPV6 == next_hdr) {
		return sfe_ipv6_recv_icmp(si, skb, dev, len, iph, ihl);
	}

	sfe_ipv6_exception_stats_inc(si, SFE_IPV6_EXCEPTION_EVENT_UNHANDLED_PROTOCOL);
	DEBUG_TRACE("not UDP, TCP or ICMP: %u\n", next_hdr);
	return 0;
}

/*
 * sfe_ipv6_update_tcp_state()
 *	update TCP window variables.
 */
static void
sfe_ipv6_update_tcp_state(struct sfe_ipv6_connection *c,
			  struct sfe_ipv6_rule_create_msg *msg)
{
	struct sfe_ipv6_connection_match *orig_cm;
	struct sfe_ipv6_connection_match *repl_cm;
	struct sfe_ipv6_tcp_connection_match *orig_tcp;
	struct sfe_ipv6_tcp_connection_match *repl_tcp;

	orig_cm = c->original_match;
	repl_cm = c->reply_match;
	orig_tcp = &orig_cm->protocol_state.tcp;
	repl_tcp = &repl_cm->protocol_state.tcp;

	/* update orig */
	if (orig_tcp->max_win < msg->tcp_rule.flow_max_window) {
		orig_tcp->max_win = msg->tcp_rule.flow_max_window;
	}
	if ((s32)(orig_tcp->end - msg->tcp_rule.flow_end) < 0) {
		orig_tcp->end = msg->tcp_rule.flow_end;
	}
	if ((s32)(orig_tcp->max_end - msg->tcp_rule.flow_max_end) < 0) {
		orig_tcp->max_end = msg->tcp_rule.flow_max_end;
	}

	/* update reply */
	if (repl_tcp->max_win < msg->tcp_rule.return_max_window) {
		repl_tcp->max_win = msg->tcp_rule.return_max_window;
	}
	if ((s32)(repl_tcp->end - msg->tcp_rule.return_end) < 0) {
		repl_tcp->end = msg->tcp_rule.return_end;
	}
	if ((s32)(repl_tcp->max_end - msg->tcp_rule.return_max_end) < 0) {
		repl_tcp->max_end = msg->tcp_rule.return_max_end;
	}

	/* update match flags */
	orig_cm->flags &= ~SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	repl_cm->flags &= ~SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	if (msg->rule_flags & SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK) {
		orig_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
		repl_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	}
}

/*
 * sfe_ipv6_update_protocol_state()
 *	update protocol specified state machine.
 */
static void
sfe_ipv6_update_protocol_state(struct sfe_ipv6_connection *c,
			       struct sfe_ipv6_rule_create_msg *msg)
{
	switch (msg->tuple.protocol) {
	case IPPROTO_TCP:
		sfe_ipv6_update_tcp_state(c, msg);
		break;
	}
}

/*
 * sfe_ipv6_update_rule()
 *	update forwarding rule after rule is created.
 */
void sfe_ipv6_update_rule(struct sfe_ipv6_rule_create_msg *msg)

{
	struct sfe_ipv6_connection *c;
	struct sfe_ipv6 *si = &__si6;

	spin_lock_bh(&si->lock);

	c = sfe_ipv6_find_connection(si,
				     msg->tuple.protocol,
				     (struct sfe_ipv6_addr *)msg->tuple.flow_ip,
				     msg->tuple.flow_ident,
				     (struct sfe_ipv6_addr *)msg->tuple.return_ip,
				     msg->tuple.return_ident);
	if (c != NULL) {
		sfe_ipv6_update_protocol_state(c, msg);
	}

	spin_unlock_bh(&si->lock);
}

/*
 * sfe_ipv6_xmit_eth_type_check
 *	Checking if MAC header has to be written.
 */
static inline bool sfe_ipv6_xmit_eth_type_check(struct net_device *dev, u32 cm_flags)
{
	if (!(dev->flags & IFF_NOARP)) {
		return true;
	}

	/*
	 * For PPPoE, since we are now supporting PPPoE encapsulation, we are writing L2 header.
	 */
	if (cm_flags & SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP) {
		return true;
	}

	return false;
}

/*
 * sfe_ipv6_create_rule()
 *	Create a forwarding rule.
 */
int sfe_ipv6_create_rule(struct sfe_ipv6_rule_create_msg *msg)
{
	struct sfe_ipv6 *si = &__si6;
	struct sfe_ipv6_connection *c, *old_c;
	struct sfe_ipv6_connection_match *original_cm;
	struct sfe_ipv6_connection_match *reply_cm;
	struct net_device *dest_dev;
	struct net_device *src_dev;
	struct sfe_ipv6_5tuple *tuple = &msg->tuple;
	struct sock *sk;
	struct net *net;
	unsigned int src_if_idx;

	s32 flow_interface_num = msg->conn_rule.flow_top_interface_num;
	s32 return_interface_num = msg->conn_rule.return_top_interface_num;

	if (msg->rule_flags & SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE) {
		flow_interface_num = msg->conn_rule.flow_interface_num;
	}

	if (msg->rule_flags & SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE) {
		return_interface_num = msg->conn_rule.return_interface_num;
	}

	src_dev = dev_get_by_index(&init_net, flow_interface_num);
	if (!src_dev) {
		DEBUG_WARN("%px: Unable to find src_dev corresponding to %d\n", msg,
						flow_interface_num);
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		return -EINVAL;
	}

	dest_dev = dev_get_by_index(&init_net, return_interface_num);
	if (!dest_dev) {
		DEBUG_WARN("%px: Unable to find dest_dev corresponding to %d\n", msg,
						return_interface_num);
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		dev_put(src_dev);
		return -EINVAL;
	}

	if (unlikely((dest_dev->reg_state != NETREG_REGISTERED) ||
		     (src_dev->reg_state != NETREG_REGISTERED))) {
		DEBUG_WARN("%px: src_dev=%s and dest_dev=%s are unregistered\n", msg,
						src_dev->name, dest_dev->name);
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		dev_put(src_dev);
		dev_put(dest_dev);
		return -EINVAL;
	}

	/*
	 * Allocate the various connection tracking objects.
	 */
	c = (struct sfe_ipv6_connection *)kmalloc(sizeof(struct sfe_ipv6_connection), GFP_ATOMIC);
	if (unlikely(!c)) {
		DEBUG_WARN("%px: memory allocation of connection entry failed\n", msg);
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		dev_put(src_dev);
		dev_put(dest_dev);
		return -ENOMEM;
	}

	original_cm = (struct sfe_ipv6_connection_match *)kmalloc(sizeof(struct sfe_ipv6_connection_match), GFP_ATOMIC);
	if (unlikely(!original_cm)) {
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		DEBUG_WARN("%px: memory allocation of connection match entry failed\n", msg);
		kfree(c);
		dev_put(src_dev);
		dev_put(dest_dev);
		return -ENOMEM;
	}

	reply_cm = (struct sfe_ipv6_connection_match *)kmalloc(sizeof(struct sfe_ipv6_connection_match), GFP_ATOMIC);
	if (unlikely(!reply_cm)) {
		this_cpu_inc(si->stats_pcpu->connection_create_failures64);
		DEBUG_WARN("%px: memory allocation of connection match entry failed\n", msg);
		kfree(original_cm);
		kfree(c);
		dev_put(src_dev);
		dev_put(dest_dev);
		return -ENOMEM;
	}

	this_cpu_inc(si->stats_pcpu->connection_create_requests64);

	spin_lock_bh(&si->lock);

	/*
	 * Check to see if there is already a flow that matches the rule we're
	 * trying to create.  If there is then we can't create a new one.
	 */
	old_c = sfe_ipv6_find_connection(si, tuple->protocol, (struct sfe_ipv6_addr *)tuple->flow_ip, tuple->flow_ident,
					 (struct sfe_ipv6_addr *)tuple->return_ip, tuple->return_ident);

	if (old_c != NULL) {
		this_cpu_inc(si->stats_pcpu->connection_create_collisions64);

		/*
		 * If we already have the flow then it's likely that this
		 * request to create the connection rule contains more
		 * up-to-date information. Check and update accordingly.
		 */
		sfe_ipv6_update_protocol_state(old_c, msg);
		spin_unlock_bh(&si->lock);

		kfree(reply_cm);
		kfree(original_cm);
		kfree(c);
		dev_put(src_dev);
		dev_put(dest_dev);

		DEBUG_TRACE("connection already exists -  p: %d\n"
			    "  s: %s:%pxM:%pI6:%u, d: %s:%pxM:%pI6:%u\n",
			    tuple->protocol,
			    src_dev->name, msg->conn_rule.flow_mac, tuple->flow_ip, ntohs(tuple->flow_ident),
			   dest_dev->name, msg->conn_rule.return_mac, tuple->return_ip, ntohs(tuple->return_ident));
		return -EADDRINUSE;
	}

	/*
	 * Fill in the "original" direction connection matching object.
	 * Note that the transmit MAC address is "dest_mac_xlate" because
	 * we always know both ends of a connection by their translated
	 * addresses and not their public addresses.
	 */
	original_cm->match_dev = src_dev;
	original_cm->match_protocol = tuple->protocol;
	original_cm->match_src_ip[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	original_cm->match_src_port = netif_is_vxlan(src_dev) ? 0 : tuple->flow_ident;
	original_cm->match_dest_ip[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	original_cm->match_dest_port = tuple->return_ident;

	original_cm->xlate_src_ip[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	original_cm->xlate_src_port = tuple->flow_ident;
	original_cm->xlate_dest_ip[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	original_cm->xlate_dest_port =  tuple->return_ident;

	atomic_set(&original_cm->rx_packet_count, 0);
	original_cm->rx_packet_count64 = 0;
	atomic_set(&original_cm->rx_byte_count, 0);
	original_cm->rx_byte_count64 = 0;
	original_cm->xmit_dev = dest_dev;

	original_cm->xmit_dev_mtu = msg->conn_rule.return_mtu;

	original_cm->connection = c;
	original_cm->counter_match = reply_cm;

	/*
	 * Valid in decap direction only
	 */
	RCU_INIT_POINTER(original_cm->up, NULL);

	original_cm->flags = 0;
	if (msg->valid_flags & SFE_RULE_CREATE_MARK_VALID) {
		original_cm->mark =  msg->mark_rule.flow_mark;
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_MARK;
	}
	if (msg->valid_flags & SFE_RULE_CREATE_QOS_VALID) {
		original_cm->priority = msg->qos_rule.flow_qos_tag;
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
	}
	if (msg->valid_flags & SFE_RULE_CREATE_DSCP_MARKING_VALID) {
		original_cm->dscp = msg->dscp_rule.flow_dscp << SFE_IPV6_DSCP_SHIFT;
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK;
	}
	if (msg->rule_flags & SFE_RULE_CREATE_FLAG_BRIDGE_FLOW) {
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW;
	}

#ifdef CONFIG_NF_FLOW_COOKIE
	original_cm->flow_cookie = 0;
#endif
#ifdef CONFIG_XFRM
	if (msg->valid_flags & SFE_RULE_CREATE_DIRECTION_VALID) {
		original_cm->flow_accel = msg->direction_rule.flow_accel;
	} else {
		original_cm->flow_accel = 1;
	}
#endif
	/*
	 * If l2_features are disabled and flow uses l2 features such as macvlan/bridge/pppoe/vlan,
	 * bottom interfaces are expected to be disabled in the flow rule and always top interfaces
	 * are used. In such cases, do not use HW csum offload. csum offload is used only when we
	 * are sending directly to the destination interface that supports it.
	 */
	if (likely(dest_dev->features & NETIF_F_HW_CSUM) && !netif_is_vxlan(dest_dev)) {
		if ((msg->conn_rule.return_top_interface_num == msg->conn_rule.return_interface_num) ||
			(msg->rule_flags & SFE_RULE_CREATE_FLAG_USE_RETURN_BOTTOM_INTERFACE)) {
			 original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD;
		}
	}

	reply_cm->flags = 0;

	/*
	 * Adding PPPoE parameters to original and reply entries based on the direction where
	 * PPPoE header is valid in ECM rule.
	 *
	 * If PPPoE is valid in flow direction (from interface is PPPoE), then
	 *	original cm will have PPPoE at ingress (strip PPPoE header)
	 *	reply cm will have PPPoE at egress (add PPPoE header)
	 *
	 * If PPPoE is valid in return direction (to interface is PPPoE), then
	 *	original cm will have PPPoE at egress (add PPPoE header)
	 *	reply cm will have PPPoE at ingress (strip PPPoE header)
	 */
	if (msg->valid_flags & SFE_RULE_CREATE_PPPOE_DECAP_VALID) {
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_DECAP;
		original_cm->pppoe_session_id = msg->pppoe_rule.flow_pppoe_session_id;
		ether_addr_copy(original_cm->pppoe_remote_mac, msg->pppoe_rule.flow_pppoe_remote_mac);

		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP;
		reply_cm->pppoe_session_id = msg->pppoe_rule.flow_pppoe_session_id;
		ether_addr_copy(reply_cm->pppoe_remote_mac, msg->pppoe_rule.flow_pppoe_remote_mac);
	}

	if (msg->valid_flags & SFE_RULE_CREATE_PPPOE_ENCAP_VALID) {
		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP;
		original_cm->pppoe_session_id = msg->pppoe_rule.return_pppoe_session_id;
		ether_addr_copy(original_cm->pppoe_remote_mac, msg->pppoe_rule.return_pppoe_remote_mac);

		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_DECAP;
		reply_cm->pppoe_session_id = msg->pppoe_rule.return_pppoe_session_id;
		ether_addr_copy(reply_cm->pppoe_remote_mac, msg->pppoe_rule.return_pppoe_remote_mac);
	}

	/*
	 * For the non-arp interface, we don't write L2 HDR.
	 * Excluding PPPoE from this, since we are now supporting PPPoE encap/decap.
	 */
	if (sfe_ipv6_xmit_eth_type_check(dest_dev, original_cm->flags)) {

		/*
		 * Check whether the rule has configured a specific source MAC address to use.
		 * This is needed when virtual L3 interfaces such as br-lan, macvlan, vlan are used during egress
		 */
		if (msg->rule_flags & SFE_RULE_CREATE_FLAG_BRIDGE_FLOW) {
			ether_addr_copy((u8 *)original_cm->xmit_src_mac, (u8 *)msg->conn_rule.flow_mac);
		} else {
			if ((msg->valid_flags & SFE_RULE_CREATE_SRC_MAC_VALID) &&
			    (msg->src_mac_rule.mac_valid_flags & SFE_SRC_MAC_RETURN_VALID)) {
				ether_addr_copy((u8 *)original_cm->xmit_src_mac, (u8 *)msg->src_mac_rule.return_src_mac);
			} else {
				ether_addr_copy((u8 *)original_cm->xmit_src_mac, (u8 *)dest_dev->dev_addr);
			}
		}
		ether_addr_copy((u8 *)original_cm->xmit_dest_mac, (u8 *)msg->conn_rule.return_mac);

		original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;

		/*
		 * If our dev writes Ethernet headers then we can write a really fast
		 * version
		 */
		if (dest_dev->header_ops) {
			if (dest_dev->header_ops->create == eth_header) {
				original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
			}
		}
	}

	/*
	 * Fill in the "reply" direction connection matching object.
	 */
	reply_cm->match_dev = dest_dev;
	reply_cm->match_protocol = tuple->protocol;
	reply_cm->match_src_ip[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	reply_cm->match_dest_ip[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	reply_cm->match_dest_port = tuple->flow_ident;
	reply_cm->xlate_src_ip[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	reply_cm->xlate_src_port = tuple->return_ident;
	reply_cm->xlate_dest_ip[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	reply_cm->xlate_dest_port = tuple->flow_ident;

	/*
	 * Keep source port as 0 for VxLAN tunnels.
	 */
	if (netif_is_vxlan(src_dev) || netif_is_vxlan(dest_dev)) {
		reply_cm->match_src_port = 0;
	} else {
		reply_cm->match_src_port = tuple->return_ident;
	}

	atomic_set(&original_cm->rx_byte_count, 0);
	reply_cm->rx_packet_count64 = 0;
	atomic_set(&reply_cm->rx_byte_count, 0);
	reply_cm->rx_byte_count64 = 0;
	reply_cm->xmit_dev = src_dev;
	reply_cm->xmit_dev_mtu = msg->conn_rule.flow_mtu;

	reply_cm->connection = c;
	reply_cm->counter_match = original_cm;

	reply_cm->flags = 0;
	if (msg->valid_flags & SFE_RULE_CREATE_MARK_VALID) {
		reply_cm->mark =  msg->mark_rule.return_mark;
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_MARK;
	}
	if (msg->valid_flags & SFE_RULE_CREATE_QOS_VALID) {
		reply_cm->priority = msg->qos_rule.return_qos_tag;
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
	}
	if (msg->valid_flags & SFE_RULE_CREATE_DSCP_MARKING_VALID) {
		reply_cm->dscp = msg->dscp_rule.return_dscp << SFE_IPV6_DSCP_SHIFT;
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_DSCP_REMARK;
	}

	if (msg->rule_flags & SFE_RULE_CREATE_FLAG_BRIDGE_FLOW) {
		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_BRIDGE_FLOW;
	}

	/*
	 * Setup UDP Socket if found to be valid for decap.
	 */
	RCU_INIT_POINTER(reply_cm->up, NULL);
	net = dev_net(reply_cm->match_dev);
	src_if_idx = src_dev->ifindex;

	rcu_read_lock();

	/*
	 * Look for the associated sock object.
	 * __udp6_lib_lookup() holds a reference for this sock object,
	 * which will be released in sfe_ipv6_flush_connection()
	 */
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	sk = __udp6_lib_lookup(net, (const struct in6_addr *)reply_cm->match_dest_ip,
			reply_cm->match_dest_port, (const struct in6_addr *)reply_cm->xlate_src_ip,
			reply_cm->xlate_src_port, src_if_idx, &udp_table);
#else
	sk = __udp6_lib_lookup(net, (const struct in6_addr *)reply_cm->match_dest_ip,
			reply_cm->match_dest_port, (const struct in6_addr *)reply_cm->xlate_src_ip,
			reply_cm->xlate_src_port, src_if_idx, 0, &udp_table, NULL);
#endif
	rcu_read_unlock();

	/*
	 * We set the UDP sock pointer as valid only for decap direction.
	 */
	if (sk && udp_sk(sk)->encap_type) {
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
		if (!atomic_add_unless(&sk->sk_refcnt, 1, 0)) {
#else
		if (!refcount_inc_not_zero(&sk->sk_refcnt)) {
#endif
			kfree(reply_cm);
			kfree(original_cm);
			kfree(c);

			DEBUG_INFO("sfe: unable to take reference for socket  p:%d\n", tuple->protocol);
			DEBUG_INFO("SK: connection - \n"
		     			"  s: %s:%pI6(%pI6):%u(%u)\n"
				   "  d: %s:%pI6(%pI6):%u(%u)\n",
					reply_cm->match_dev->name, &reply_cm->match_src_ip, &reply_cm->xlate_src_ip,
					ntohs(reply_cm->match_src_port), ntohs(reply_cm->xlate_src_port),
					reply_cm->xmit_dev->name, &reply_cm->match_dest_ip, &reply_cm->xlate_dest_ip,
					ntohs(reply_cm->match_dest_port), ntohs(reply_cm->xlate_dest_port));

			dev_put(src_dev);
			dev_put(dest_dev);

			return -ESHUTDOWN;
		}

		rcu_assign_pointer(reply_cm->up, udp_sk(sk));
		DEBUG_INFO("Sock lookup success with reply_cm direction(%p)\n", sk);
		DEBUG_INFO("SK: connection - \n"
			   "  s: %s:%pI6(%pI6):%u(%u)\n"
			   "  d: %s:%pI6(%pI6):%u(%u)\n",
			reply_cm->match_dev->name, &reply_cm->match_src_ip, &reply_cm->xlate_src_ip,
			ntohs(reply_cm->match_src_port), ntohs(reply_cm->xlate_src_port),
			reply_cm->xmit_dev->name, &reply_cm->match_dest_ip, &reply_cm->xlate_dest_ip,
			ntohs(reply_cm->match_dest_port), ntohs(reply_cm->xlate_dest_port));
	}

#ifdef CONFIG_NF_FLOW_COOKIE
	reply_cm->flow_cookie = 0;
#endif
#ifdef CONFIG_XFRM
	if (msg->valid_flags & SFE_RULE_CREATE_DIRECTION_VALID) {
		reply_cm->flow_accel = msg->direction_rule.return_accel;
	} else {
		reply_cm->flow_accel = 1;
	}
#endif
	/*
	 * If l2_features are disabled and flow uses l2 features such as macvlan/bridge/pppoe/vlan,
	 * bottom interfaces are expected to be disabled in the flow rule and always top interfaces
	 * are used. In such cases, do not use HW csum offload. csum offload is used only when we
	 * are sending directly to the destination interface that supports it.
	 */
	if (likely(src_dev->features & NETIF_F_HW_CSUM) && !(netif_is_vxlan(src_dev) || netif_is_vxlan(dest_dev))) {
		if ((msg->conn_rule.flow_top_interface_num == msg->conn_rule.flow_interface_num) ||
			(msg->rule_flags & SFE_RULE_CREATE_FLAG_USE_FLOW_BOTTOM_INTERFACE)) {
			 reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_CSUM_OFFLOAD;
		}
	}

	/*
	 * For the non-arp interface, we don't write L2 HDR.
	 * Excluding PPPoE from this, since we are now supporting PPPoE encap/decap.
	 */
	if (sfe_ipv6_xmit_eth_type_check(src_dev, reply_cm->flags)) {

		/*
		 * Check whether the rule has configured a specific source MAC address to use.
		 * This is needed when virtual L3 interfaces such as br-lan, macvlan, vlan are used during egress
		 */
		if (msg->rule_flags & SFE_RULE_CREATE_FLAG_BRIDGE_FLOW) {
			ether_addr_copy((u8 *)reply_cm->xmit_src_mac, (u8 *)msg->conn_rule.return_mac);
		} else {
			if ((msg->valid_flags & SFE_RULE_CREATE_SRC_MAC_VALID) &&
			    (msg->src_mac_rule.mac_valid_flags & SFE_SRC_MAC_FLOW_VALID)) {
				ether_addr_copy((u8 *)reply_cm->xmit_src_mac, (u8 *)msg->src_mac_rule.flow_src_mac);
			} else {
				ether_addr_copy((u8 *)reply_cm->xmit_src_mac, (u8 *)src_dev->dev_addr);
			}
		}

		ether_addr_copy((u8 *)reply_cm->xmit_dest_mac, (u8 *)msg->conn_rule.flow_mac);

		reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;

		/*
		 * If our dev writes Ethernet headers then we can write a really fast
		 * version.
		 */
		if (src_dev->header_ops) {
			if (src_dev->header_ops->create == eth_header) {
				reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
			}
		}
	}

	/*
	 * No support for NAT in ipv6
	 */

	c->protocol = tuple->protocol;
	c->src_ip[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	c->src_ip_xlate[0] = *(struct sfe_ipv6_addr *)tuple->flow_ip;
	c->src_port = tuple->flow_ident;
	c->src_port_xlate = tuple->flow_ident;
	c->original_dev = src_dev;
	c->original_match = original_cm;

	c->dest_ip[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	c->dest_ip_xlate[0] = *(struct sfe_ipv6_addr *)tuple->return_ip;
	c->dest_port = tuple->return_ident;
	c->dest_port_xlate = tuple->return_ident;

	c->reply_dev = dest_dev;
	c->reply_match = reply_cm;
	c->debug_read_seq = 0;
	c->last_sync_jiffies = get_jiffies_64();
	c->removed = false;

	/*
	 * Initialize the protocol-specific information that we track.
	 */
	switch (tuple->protocol) {
	case IPPROTO_TCP:
		original_cm->protocol_state.tcp.win_scale = msg->tcp_rule.flow_window_scale;
		original_cm->protocol_state.tcp.max_win = msg->tcp_rule.flow_max_window ? msg->tcp_rule.flow_max_window : 1;
		original_cm->protocol_state.tcp.end = msg->tcp_rule.flow_end;
		original_cm->protocol_state.tcp.max_end = msg->tcp_rule.flow_max_end;
		reply_cm->protocol_state.tcp.win_scale = msg->tcp_rule.return_window_scale;
		reply_cm->protocol_state.tcp.max_win = msg->tcp_rule.return_max_window ? msg->tcp_rule.return_max_window : 1;
		reply_cm->protocol_state.tcp.end = msg->tcp_rule.return_end;
		reply_cm->protocol_state.tcp.max_end = msg->tcp_rule.return_max_end;
		if (msg->rule_flags & SFE_RULE_CREATE_FLAG_NO_SEQ_CHECK) {
			original_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
			reply_cm->flags |= SFE_IPV6_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
		}
		break;
	}

	sfe_ipv6_connection_match_compute_translations(original_cm);
	sfe_ipv6_connection_match_compute_translations(reply_cm);
	sfe_ipv6_insert_connection(si, c);

	spin_unlock_bh(&si->lock);

	/*
	 * We have everything we need!
	 */
	DEBUG_INFO("new connection - p: %d\n"
		   "  s: %s:%pxM(%pxM):%pI6(%pI6):%u(%u)\n"
		   "  d: %s:%pxM(%pxM):%pI6(%pI6):%u(%u)\n",
		   tuple->protocol,
		   src_dev->name, msg->conn_rule.flow_mac, NULL,
		   (void *)tuple->flow_ip, (void *)tuple->flow_ip, ntohs(tuple->flow_ident), ntohs(tuple->flow_ident),
		   dest_dev->name, NULL, msg->conn_rule.return_mac,
		   (void *)tuple->return_ip, (void *)tuple->return_ip, ntohs(tuple->return_ident), ntohs(tuple->return_ident));

	return 0;
}

/*
 * sfe_ipv6_destroy_rule()
 *	Destroy a forwarding rule.
 */
void sfe_ipv6_destroy_rule(struct sfe_ipv6_rule_destroy_msg *msg)
{
	struct sfe_ipv6 *si = &__si6;
	struct sfe_ipv6_connection *c;
	bool ret;
	struct sfe_ipv6_5tuple *tuple = &msg->tuple;

	this_cpu_inc(si->stats_pcpu->connection_destroy_requests64);

	spin_lock_bh(&si->lock);

	/*
	 * Check to see if we have a flow that matches the rule we're trying
	 * to destroy.  If there isn't then we can't destroy it.
	 */
	c = sfe_ipv6_find_connection(si, tuple->protocol, (struct sfe_ipv6_addr *)tuple->flow_ip, tuple->flow_ident,
				     (struct sfe_ipv6_addr *)tuple->return_ip, tuple->return_ident);
	if (!c) {
		spin_unlock_bh(&si->lock);

		this_cpu_inc(si->stats_pcpu->connection_destroy_misses64);

		DEBUG_TRACE("connection does not exist - p: %d, s: %pI6:%u, d: %pI6:%u\n",
			    tuple->protocol, tuple->flow_ip, ntohs(tuple->flow_ident),
			    tuple->return_ip, ntohs(tuple->return_ident));
		return;
	}

	/*
	 * Remove our connection details from the hash tables.
	 */
	ret = sfe_ipv6_remove_connection(si, c);
	spin_unlock_bh(&si->lock);

	if (ret) {
		sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
	}

	DEBUG_INFO("connection destroyed - p: %d, s: %pI6:%u, d: %pI6:%u\n",
		   tuple->protocol, tuple->flow_ip, ntohs(tuple->flow_ident),
		   tuple->return_ip, ntohs(tuple->return_ident));
}

/*
 * sfe_ipv6_register_sync_rule_callback()
 *	Register a callback for rule synchronization.
 */
void sfe_ipv6_register_sync_rule_callback(sfe_sync_rule_callback_t sync_rule_callback)
{
	struct sfe_ipv6 *si = &__si6;

	spin_lock_bh(&si->lock);
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
	spin_unlock_bh(&si->lock);
}

/*
 * sfe_ipv6_get_debug_dev()
 */
static ssize_t sfe_ipv6_get_debug_dev(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct sfe_ipv6 *si = &__si6;
	ssize_t count;
	int num;

	spin_lock_bh(&si->lock);
	num = si->debug_dev;
	spin_unlock_bh(&si->lock);

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
	return count;
}

/*
 * sfe_ipv6_destroy_all_rules_for_dev()
 *	Destroy all connections that match a particular device.
 *
 * If we pass dev as NULL then this destroys all connections.
 */
void sfe_ipv6_destroy_all_rules_for_dev(struct net_device *dev)
{
	struct sfe_ipv6 *si = &__si6;
	struct sfe_ipv6_connection *c;
	bool ret;

another_round:
	spin_lock_bh(&si->lock);

	for (c = si->all_connections_head; c; c = c->all_connections_next) {
		/*
		 * Does this connection relate to the device we are destroying?
		 */
		if (!dev
		    || (dev == c->original_dev)
		    || (dev == c->reply_dev)) {
			break;
		}
	}

	if (c) {
		ret = sfe_ipv6_remove_connection(si, c);
	}

	spin_unlock_bh(&si->lock);

	if (c) {
		if (ret) {
			sfe_ipv6_flush_connection(si, c, SFE_SYNC_REASON_DESTROY);
		}
		goto another_round;
	}
}

/*
 * sfe_ipv6_periodic_sync()
 */
static void sfe_ipv6_periodic_sync(struct work_struct *work)
{
	struct sfe_ipv6 *si = container_of((struct delayed_work *)work, struct sfe_ipv6, sync_dwork);
	u64 now_jiffies;
	int quota;
	sfe_sync_rule_callback_t sync_rule_callback;
	struct sfe_ipv6_connection *c;

	now_jiffies = get_jiffies_64();

	rcu_read_lock();
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
	if (!sync_rule_callback) {
		rcu_read_unlock();
		goto done;
	}

	spin_lock_bh(&si->lock);

	/*
	 * If we have reached the end of the connection list, walk from
	 * the connection head.
	 */
	c = si->wc_next;
	if (unlikely(!c)) {
		c = si->all_connections_head;
	}
	/*
	 * Get an estimate of the number of connections to parse in this sync.
	 */
	quota = (si->num_connections + 63) / 64;

	/*
	 * Walk the "all connection" list and sync the connection state.
	 */
	while (likely(c && quota)) {
		struct sfe_ipv6_connection_match *cm;
		struct sfe_ipv6_connection_match *counter_cm;
		struct sfe_connection_sync sis;

		cm = c->original_match;
		counter_cm = c->reply_match;

		/*
		 * Didn't receive packets in the origial direction or reply
		 * direction, move to the next connection.
		 */
		if (!atomic_read(&cm->rx_packet_count) && !atomic_read(&counter_cm->rx_packet_count)) {
			c = c->all_connections_next;
			continue;
		}

		quota--;

		/*
		 * Sync the connection state.
		 */
		sfe_ipv6_gen_sync_connection(si, c, &sis, SFE_SYNC_REASON_STATS, now_jiffies);

		si->wc_next = c->all_connections_next;

		spin_unlock_bh(&si->lock);
		sync_rule_callback(&sis);
		spin_lock_bh(&si->lock);

		/*
		 * c must be set and used in the same lock/unlock window;
		 * because c could be removed when we don't hold the lock,
		 * so delay grabbing until after the callback and relock.
		 */
		c = si->wc_next;
	}

	/*
	 * At the end of loop, put wc_next to the connection we left
	 */
	si->wc_next = c;

	spin_unlock_bh(&si->lock);
	rcu_read_unlock();

done:
	schedule_delayed_work_on(si->work_cpu, (struct delayed_work *)work, ((HZ + 99) / 100));
}

/*
 * sfe_ipv6_debug_dev_read_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_start(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
					  int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	si->debug_read_seq++;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv6>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_connections_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_connections_start(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
						      int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<connections>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_connections_connection()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_connections_connection(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
							   int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	struct sfe_ipv6_connection *c;
	struct sfe_ipv6_connection_match *original_cm;
	struct sfe_ipv6_connection_match *reply_cm;
	int bytes_read;
	int protocol;
	struct net_device *src_dev;
	struct sfe_ipv6_addr src_ip;
	struct sfe_ipv6_addr src_ip_xlate;
	__be16 src_port;
	__be16 src_port_xlate;
	u64 src_rx_packets;
	u64 src_rx_bytes;
	struct net_device *dest_dev;
	struct sfe_ipv6_addr dest_ip;
	struct sfe_ipv6_addr dest_ip_xlate;
	__be16 dest_port;
	__be16 dest_port_xlate;
	u64 dest_rx_packets;
	u64 dest_rx_bytes;
	u64 last_sync_jiffies;
	u32 src_mark, dest_mark,  src_priority, dest_priority, src_dscp, dest_dscp;
	u32 packet, byte, original_cm_flags;
	u16 pppoe_session_id;
	u8 pppoe_remote_mac[ETH_ALEN];
#ifdef CONFIG_NF_FLOW_COOKIE
	int src_flow_cookie, dst_flow_cookie;
#endif

	spin_lock_bh(&si->lock);

	for (c = si->all_connections_head; c; c = c->all_connections_next) {
		if (c->debug_read_seq < si->debug_read_seq) {
			c->debug_read_seq = si->debug_read_seq;
			break;
		}
	}

	/*
	 * If there were no connections then move to the next state.
	 */
	if (!c) {
		spin_unlock_bh(&si->lock);
		ws->state++;
		return true;
	}

	original_cm = c->original_match;
	reply_cm = c->reply_match;

	protocol = c->protocol;
	src_dev = c->original_dev;
	src_ip = c->src_ip[0];
	src_ip_xlate = c->src_ip_xlate[0];
	src_port = c->src_port;
	src_port_xlate = c->src_port_xlate;
	src_priority = original_cm->priority;
	src_dscp = original_cm->dscp >> SFE_IPV6_DSCP_SHIFT;

	sfe_ipv6_connection_match_update_summary_stats(original_cm, &packet, &byte);
	sfe_ipv6_connection_match_update_summary_stats(reply_cm, &packet, &byte);

	src_rx_packets = original_cm->rx_packet_count64;
	src_rx_bytes = original_cm->rx_byte_count64;
	src_mark = original_cm->mark;
	dest_dev = c->reply_dev;
	dest_ip = c->dest_ip[0];
	dest_ip_xlate = c->dest_ip_xlate[0];
	dest_port = c->dest_port;
	dest_port_xlate = c->dest_port_xlate;
	dest_priority = reply_cm->priority;
	dest_dscp = reply_cm->dscp >> SFE_IPV6_DSCP_SHIFT;
	dest_rx_packets = reply_cm->rx_packet_count64;
	dest_rx_bytes = reply_cm->rx_byte_count64;
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
	original_cm_flags = original_cm->flags;
	pppoe_session_id = original_cm->pppoe_session_id;
	ether_addr_copy(pppoe_remote_mac, original_cm->pppoe_remote_mac);
	dest_mark = reply_cm->mark;
#ifdef CONFIG_NF_FLOW_COOKIE
	src_flow_cookie = original_cm->flow_cookie;
	dst_flow_cookie = reply_cm->flow_cookie;
#endif
	spin_unlock_bh(&si->lock);

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
				"protocol=\"%u\" "
				"src_dev=\"%s\" "
				"src_ip=\"%pI6\" src_ip_xlate=\"%pI6\" "
				"src_port=\"%u\" src_port_xlate=\"%u\" "
				"src_priority=\"%u\" src_dscp=\"%u\" "
				"src_rx_pkts=\"%llu\" src_rx_bytes=\"%llu\" "
				"src_mark=\"%08x\" "
				"dest_dev=\"%s\" "
				"dest_ip=\"%pI6\" dest_ip_xlate=\"%pI6\" "
				"dest_port=\"%u\" dest_port_xlate=\"%u\" "
				"dest_priority=\"%u\" dest_dscp=\"%u\" "
				"dest_rx_pkts=\"%llu\" dest_rx_bytes=\"%llu\" "
				"dest_mark=\"%08x\" "
#ifdef CONFIG_NF_FLOW_COOKIE
				"src_flow_cookie=\"%d\" dst_flow_cookie=\"%d\" "
#endif
				"last_sync=\"%llu\" ",
				protocol,
				src_dev->name,
				&src_ip, &src_ip_xlate,
				ntohs(src_port), ntohs(src_port_xlate),
				src_priority, src_dscp,
				src_rx_packets, src_rx_bytes,
				src_mark,
				dest_dev->name,
				&dest_ip, &dest_ip_xlate,
				ntohs(dest_port), ntohs(dest_port_xlate),
				dest_priority, dest_dscp,
				dest_rx_packets, dest_rx_bytes,
				dest_mark,
#ifdef CONFIG_NF_FLOW_COOKIE
				src_flow_cookie, dst_flow_cookie,
#endif
				last_sync_jiffies);

	if (original_cm_flags &= (SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_DECAP | SFE_IPV6_CONNECTION_MATCH_FLAG_PPPOE_ENCAP)) {
		bytes_read += snprintf(msg + bytes_read, CHAR_DEV_MSG_SIZE, "pppoe_session_id=\"%u\" pppoe_server_MAC=\"%pM\" ",
			pppoe_session_id, pppoe_remote_mac);
	}

	bytes_read += snprintf(msg + bytes_read, CHAR_DEV_MSG_SIZE, ")/>\n");

	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	return true;
}

/*
 * sfe_ipv6_debug_dev_read_connections_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_connections_end(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
						    int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</connections>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_exceptions_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_exceptions_start(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
						     int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<exceptions>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_exceptions_exception()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_exceptions_exception(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
							 int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int i;
	u64 val = 0;

	for_each_possible_cpu(i) {
		const struct sfe_ipv6_stats *s = per_cpu_ptr(si->stats_pcpu, i);
			val += s->exception_events64[ws->iter_exception];
	}

	if (val) {
		int bytes_read;

		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
				      "\t\t<exception name=\"%s\" count=\"%llu\" />\n",
				      sfe_ipv6_exception_events_string[ws->iter_exception],
				      val);

		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
			return false;
		}

		*length -= bytes_read;
		*total_read += bytes_read;
	}

	ws->iter_exception++;
	if (ws->iter_exception >= SFE_IPV6_EXCEPTION_EVENT_LAST) {
		ws->iter_exception = 0;
		ws->state++;
	}

	return true;
}

/*
 * sfe_ipv6_debug_dev_read_exceptions_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_exceptions_end(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
						   int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t</exceptions>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_stats()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_stats(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
					  int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;
	struct sfe_ipv6_stats stats;
	unsigned int num_conn;

	sfe_ipv6_update_summary_stats(si, &stats);

	spin_lock_bh(&si->lock);
	num_conn = si->num_connections;
	spin_unlock_bh(&si->lock);

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
			      "num_connections=\"%u\" "
			      "pkts_dropped=\"%llu\" "
			      "pkts_forwarded=\"%llu\" pkts_not_forwarded=\"%llu\" "
			      "create_requests=\"%llu\" create_collisions=\"%llu\" "
			      "create_failures=\"%llu\" "
			      "destroy_requests=\"%llu\" destroy_misses=\"%llu\" "
			      "flushes=\"%llu\" "
			      "hash_hits=\"%llu\" hash_reorders=\"%llu\" "
			      "pppoe_encap_pkts_fwded=\"%llu\" "
			      "pppoe_decap_pkts_fwded=\"%llu\" />\n",

				num_conn,
				stats.packets_dropped64,
				stats.packets_forwarded64,
				stats.packets_not_forwarded64,
				stats.connection_create_requests64,
				stats.connection_create_collisions64,
				stats.connection_create_failures64,
				stats.connection_destroy_requests64,
				stats.connection_destroy_misses64,
				stats.connection_flushes64,
				stats.connection_match_hash_hits64,
				stats.connection_match_hash_reorders64,
				stats.pppoe_encap_packets_forwarded64,
				stats.pppoe_decap_packets_forwarded64);
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv6_debug_dev_read_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv6_debug_dev_read_end(struct sfe_ipv6 *si, char *buffer, char *msg, size_t *length,
					int *total_read, struct sfe_ipv6_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv6>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * Array of write functions that write various XML elements that correspond to
 * our XML output state machine.
 */
static sfe_ipv6_debug_xml_write_method_t sfe_ipv6_debug_xml_write_methods[SFE_IPV6_DEBUG_XML_STATE_DONE] = {
	sfe_ipv6_debug_dev_read_start,
	sfe_ipv6_debug_dev_read_connections_start,
	sfe_ipv6_debug_dev_read_connections_connection,
	sfe_ipv6_debug_dev_read_connections_end,
	sfe_ipv6_debug_dev_read_exceptions_start,
	sfe_ipv6_debug_dev_read_exceptions_exception,
	sfe_ipv6_debug_dev_read_exceptions_end,
	sfe_ipv6_debug_dev_read_stats,
	sfe_ipv6_debug_dev_read_end,
};

/*
 * sfe_ipv6_debug_dev_read()
 *	Send info to userspace upon read request from user
 */
static ssize_t sfe_ipv6_debug_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	char msg[CHAR_DEV_MSG_SIZE];
	int total_read = 0;
	struct sfe_ipv6_debug_xml_write_state *ws;
	struct sfe_ipv6 *si = &__si6;

	ws = (struct sfe_ipv6_debug_xml_write_state *)filp->private_data;
	while ((ws->state != SFE_IPV6_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
		if ((sfe_ipv6_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
			continue;
		}
	}
	return total_read;
}

/*
 * sfe_ipv6_debug_dev_open()
 */
static int sfe_ipv6_debug_dev_open(struct inode *inode, struct file *file)
{
	struct sfe_ipv6_debug_xml_write_state *ws;

	ws = (struct sfe_ipv6_debug_xml_write_state *)file->private_data;
	if (ws) {
		return 0;
	}

	ws = kzalloc(sizeof(struct sfe_ipv6_debug_xml_write_state), GFP_KERNEL);
	if (!ws) {
		return -ENOMEM;
	}

	ws->state = SFE_IPV6_DEBUG_XML_STATE_START;
	file->private_data = ws;

	return 0;
}

/*
 * sfe_ipv6_debug_dev_release()
 */
static int sfe_ipv6_debug_dev_release(struct inode *inode, struct file *file)
{
	struct sfe_ipv6_debug_xml_write_state *ws;

	ws = (struct sfe_ipv6_debug_xml_write_state *)file->private_data;
	if (ws) {
		/*
		 * We've finished with our output so free the write state.
		 */
		kfree(ws);
		file->private_data = NULL;
	}

	return 0;
}

/*
 * File operations used in the debug char device
 */
static struct file_operations sfe_ipv6_debug_dev_fops = {
	.read = sfe_ipv6_debug_dev_read,
	.open = sfe_ipv6_debug_dev_open,
	.release = sfe_ipv6_debug_dev_release
};

#ifdef CONFIG_NF_FLOW_COOKIE
/*
 * sfe_ipv6_register_flow_cookie_cb
 *	register a function in SFE to let SFE use this function to configure flow cookie for a flow
 *
 * Hardware driver which support flow cookie should register a callback function in SFE. Then SFE
 * can use this function to configure flow cookie for a flow.
 * return: 0, success; !=0, fail
 */
int sfe_ipv6_register_flow_cookie_cb(sfe_ipv6_flow_cookie_set_func_t cb)
{
	struct sfe_ipv6 *si = &__si6;

	BUG_ON(!cb);

	if (si->flow_cookie_set_func) {
		return -1;
	}

	rcu_assign_pointer(si->flow_cookie_set_func, cb);
	return 0;
}

/*
 * sfe_ipv6_unregister_flow_cookie_cb
 *	unregister function which is used to configure flow cookie for a flow
 *
 * return: 0, success; !=0, fail
 */
int sfe_ipv6_unregister_flow_cookie_cb(sfe_ipv6_flow_cookie_set_func_t cb)
{
	struct sfe_ipv6 *si = &__si6;

	RCU_INIT_POINTER(si->flow_cookie_set_func, NULL);
	return 0;
}

/*
 * sfe_ipv6_get_flow_cookie()
 */
static ssize_t sfe_ipv6_get_flow_cookie(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct sfe_ipv6 *si = &__si6;
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", si->flow_cookie_enable);
}

/*
 * sfe_ipv6_set_flow_cookie()
 */
static ssize_t sfe_ipv6_set_flow_cookie(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct sfe_ipv6 *si = &__si6;
	si->flow_cookie_enable = strict_strtol(buf, NULL, 0);

	return size;
}

/*
 * sysfs attributes.
 */
static const struct device_attribute sfe_ipv6_flow_cookie_attr =
	__ATTR(flow_cookie_enable, S_IWUSR | S_IRUGO, sfe_ipv6_get_flow_cookie, sfe_ipv6_set_flow_cookie);
#endif /*CONFIG_NF_FLOW_COOKIE*/

/*
 * sfe_ipv6_get_cpu()
 */
static ssize_t sfe_ipv6_get_cpu(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct sfe_ipv6 *si = &__si6;
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", si->work_cpu);
}

/*
 * sfe_ipv4_set_cpu()
 */
static ssize_t sfe_ipv6_set_cpu(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct sfe_ipv6 *si = &__si6;
	int work_cpu;

	work_cpu = simple_strtol(buf, NULL, 0);
	if ((work_cpu >= 0) && (work_cpu <= NR_CPUS)) {
		si->work_cpu = work_cpu;
	} else {
		dev_err(dev, "%s is not in valid range[0,%d]", buf, NR_CPUS);
	}

	return size;
}
/*
 * sysfs attributes.
 */
static const struct device_attribute sfe_ipv6_cpu_attr =
	__ATTR(stat_work_cpu, S_IWUSR | S_IRUGO, sfe_ipv6_get_cpu, sfe_ipv6_set_cpu);

 /*
 * sfe_ipv6_hash_init()
 *	Initialize conn match hash lists
 */
static void sfe_ipv6_conn_match_hash_init(struct sfe_ipv6 *si, int len)
{
	struct hlist_head *hash_list = si->hlist_conn_match_hash_head;
	int i;

	for (i = 0; i < len; i++) {
		INIT_HLIST_HEAD(&hash_list[i]);
	}
}

#ifdef SFE_PROCESS_LOCAL_OUT
/*
 * sfe_ipv6_local_out()
 *	Called for packets from ip_local_out() - post encapsulation & other packets
 */
static unsigned int sfe_ipv6_local_out(void *priv,
				struct sk_buff *skb,
				const struct nf_hook_state *nhs)
{
	DEBUG_TRACE("sfe: sfe_ipv6_local_out hook called.\n");

	if (likely(skb->skb_iif)) {
		return sfe_ipv6_recv(skb->dev, skb, NULL, true) ? NF_STOLEN : NF_ACCEPT;
	}

	return NF_ACCEPT;
}

/*
 * struct nf_hook_ops sfe_ipv6_ops_local_out[]
 *	Hooks into netfilter local out packet monitoring points.
 */
static struct nf_hook_ops sfe_ipv6_ops_local_out[] __read_mostly = {

	/*
	 * Local out routing hook is used to monitor packets.
	 */
	{
		.hook           = sfe_ipv6_local_out,
		.pf             = PF_INET6,
		.hooknum        = NF_INET_LOCAL_OUT,
		.priority       = NF_IP6_PRI_FIRST,
	},
};
#endif

/*
 * sfe_ipv6_init()
 */
int sfe_ipv6_init(void)
{
	struct sfe_ipv6 *si = &__si6;
	int result = -1;

	DEBUG_INFO("SFE IPv6 init\n");

	sfe_ipv6_conn_match_hash_init(si, ARRAY_SIZE(si->hlist_conn_match_hash_head));

	si->stats_pcpu = alloc_percpu_gfp(struct sfe_ipv6_stats, GFP_KERNEL | __GFP_ZERO);
	if (!si->stats_pcpu) {
		DEBUG_ERROR("failed to allocate stats memory for sfe_ipv6\n");
		goto exit0;
	}

	/*
	 * Create sys/sfe_ipv6
	 */
	si->sys_ipv6 = kobject_create_and_add("sfe_ipv6", NULL);
	if (!si->sys_ipv6) {
		DEBUG_ERROR("failed to register sfe_ipv6\n");
		goto exit1;
	}

	/*
	 * Create files, one for each parameter supported by this module.
	 */
	result = sysfs_create_file(si->sys_ipv6, &sfe_ipv6_debug_dev_attr.attr);
	if (result) {
		DEBUG_ERROR("failed to register debug dev file: %d\n", result);
		goto exit2;
	}

	result = sysfs_create_file(si->sys_ipv6, &sfe_ipv6_cpu_attr.attr);
	if (result) {
		DEBUG_ERROR("failed to register debug dev file: %d\n", result);
		goto exit3;
	}

#ifdef CONFIG_NF_FLOW_COOKIE
	result = sysfs_create_file(si->sys_ipv6, &sfe_ipv6_flow_cookie_attr.attr);
	if (result) {
		DEBUG_ERROR("failed to register flow cookie enable file: %d\n", result);
		goto exit4;
	}
#endif /* CONFIG_NF_FLOW_COOKIE */

#ifdef SFE_PROCESS_LOCAL_OUT
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	result = nf_register_hooks(sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#else
	result = nf_register_net_hooks(&init_net, sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#endif
#endif
	if (result < 0) {
		DEBUG_ERROR("can't register nf local out hook: %d\n", result);
		goto exit5;
	} else {
		DEBUG_ERROR("Register nf local out hook success: %d\n", result);
	}

	/*
	 * Register our debug char device.
	 */
	result = register_chrdev(0, "sfe_ipv6", &sfe_ipv6_debug_dev_fops);
	if (result < 0) {
		DEBUG_ERROR("Failed to register chrdev: %d\n", result);
		goto exit6;
	}

	si->debug_dev = result;
	si->work_cpu = WORK_CPU_UNBOUND;

	/*
	 * Create work to handle periodic statistics.
	 */
	INIT_DELAYED_WORK(&(si->sync_dwork), sfe_ipv6_periodic_sync);
	schedule_delayed_work_on(si->work_cpu, &(si->sync_dwork), ((HZ + 99) / 100));
	spin_lock_init(&si->lock);

	return 0;

exit6:
#ifdef SFE_PROCESS_LOCAL_OUT
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	DEBUG_TRACE("sfe: Unregister local out hook\n");
	nf_unregister_hooks(sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#else
	DEBUG_TRACE("sfe: Unregister local out hook\n");
	nf_unregister_net_hooks(&init_net, sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#endif
#endif

exit5:
#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_flow_cookie_attr.attr);

exit4:
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_cpu_attr.attr);

exit3:
	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_debug_dev_attr.attr);

exit2:
	kobject_put(si->sys_ipv6);

exit1:
	free_percpu(si->stats_pcpu);

exit0:
	return result;
}

/*
 * sfe_ipv6_exit()
 */
void sfe_ipv6_exit(void)
{
	struct sfe_ipv6 *si = &__si6;

	DEBUG_INFO("SFE IPv6 exit\n");

	/*
	 * Destroy all connections.
	 */
	sfe_ipv6_destroy_all_rules_for_dev(NULL);

	cancel_delayed_work(&si->sync_dwork);

	unregister_chrdev(si->debug_dev, "sfe_ipv6");

	free_percpu(si->stats_pcpu);

#ifdef SFE_PROCESS_LOCAL_OUT
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 13, 0))
	DEBUG_TRACE("sfe: Unregister local out hook\n");
	nf_unregister_hooks(sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#else
	DEBUG_TRACE("sfe: Unregister local out hook\n");
	nf_unregister_net_hooks(&init_net, sfe_ipv6_ops_local_out, ARRAY_SIZE(sfe_ipv6_ops_local_out));
#endif
#endif

#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_flow_cookie_attr.attr);
#endif /* CONFIG_NF_FLOW_COOKIE */

	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_cpu_attr.attr);

	sysfs_remove_file(si->sys_ipv6, &sfe_ipv6_debug_dev_attr.attr);

	kobject_put(si->sys_ipv6);
}

#ifdef CONFIG_NF_FLOW_COOKIE
EXPORT_SYMBOL(sfe_ipv6_register_flow_cookie_cb);
EXPORT_SYMBOL(sfe_ipv6_unregister_flow_cookie_cb);
#endif
