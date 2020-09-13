/*
 * sfe_ipv4.c
 *	Shortcut forwarding engine - IPv4 edition.
 *
 * Copyright (c) 2013-2016, 2019, The Linux Foundation. All rights reserved.
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

#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/skbuff.h>
#include <linux/icmp.h>
#include <net/tcp.h>
#include <linux/etherdevice.h>
#include <linux/version.h>

#include "sfe.h"
#include "sfe_cm.h"

/*
 * By default Linux IP header and transport layer header structures are
 * unpacked, assuming that such headers should be 32-bit aligned.
 * Unfortunately some wireless adaptors can't cope with this requirement and
 * some CPUs can't handle misaligned accesses.  For those platforms we
 * define SFE_IPV4_UNALIGNED_IP_HEADER and mark the structures as packed.
 * When we do this the compiler will generate slightly worse code than for the
 * aligned case (on most platforms) but will be much quicker than fixing
 * things up in an unaligned trap handler.
 */
#define SFE_IPV4_UNALIGNED_IP_HEADER 1
#if SFE_IPV4_UNALIGNED_IP_HEADER
#define SFE_IPV4_UNALIGNED_STRUCT __attribute__((packed))
#else
#define SFE_IPV4_UNALIGNED_STRUCT
#endif

/*
 * An Ethernet header, but with an optional "packed" attribute to
 * help with performance on some platforms (see the definition of
 * SFE_IPV4_UNALIGNED_STRUCT)
 */
struct sfe_ipv4_eth_hdr {
	__be16 h_dest[ETH_ALEN / 2];
	__be16 h_source[ETH_ALEN / 2];
	__be16 h_proto;
} SFE_IPV4_UNALIGNED_STRUCT;

#define SFE_IPV4_DSCP_MASK 0x3
#define SFE_IPV4_DSCP_SHIFT 2

/*
 * An IPv4 header, but with an optional "packed" attribute to
 * help with performance on some platforms (see the definition of
 * SFE_IPV4_UNALIGNED_STRUCT)
 */
struct sfe_ipv4_ip_hdr {
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u8 ihl:4,
	     version:4;
#elif defined (__BIG_ENDIAN_BITFIELD)
	__u8 version:4,
	     ihl:4;
#else
#error	"Please fix <asm/byteorder.h>"
#endif
	__u8 tos;
	__be16 tot_len;
	__be16 id;
	__be16 frag_off;
	__u8 ttl;
	__u8 protocol;
	__sum16 check;
	__be32 saddr;
	__be32 daddr;

	/*
	 * The options start here.
	 */
} SFE_IPV4_UNALIGNED_STRUCT;

/*
 * A UDP header, but with an optional "packed" attribute to
 * help with performance on some platforms (see the definition of
 * SFE_IPV4_UNALIGNED_STRUCT)
 */
struct sfe_ipv4_udp_hdr {
	__be16 source;
	__be16 dest;
	__be16 len;
	__sum16 check;
} SFE_IPV4_UNALIGNED_STRUCT;

/*
 * A TCP header, but with an optional "packed" attribute to
 * help with performance on some platforms (see the definition of
 * SFE_IPV4_UNALIGNED_STRUCT)
 */
struct sfe_ipv4_tcp_hdr {
	__be16 source;
	__be16 dest;
	__be32 seq;
	__be32 ack_seq;
#if defined(__LITTLE_ENDIAN_BITFIELD)
	__u16 res1:4,
	      doff:4,
	      fin:1,
	      syn:1,
	      rst:1,
	      psh:1,
	      ack:1,
	      urg:1,
	      ece:1,
	      cwr:1;
#elif defined(__BIG_ENDIAN_BITFIELD)
	__u16 doff:4,
	      res1:4,
	      cwr:1,
	      ece:1,
	      urg:1,
	      ack:1,
	      psh:1,
	      rst:1,
	      syn:1,
	      fin:1;
#else
#error	"Adjust your <asm/byteorder.h> defines"
#endif
	__be16 window;
	__sum16	check;
	__be16 urg_ptr;
} SFE_IPV4_UNALIGNED_STRUCT;

/*
 * Specifies the lower bound on ACK numbers carried in the TCP header
 */
#define SFE_IPV4_TCP_MAX_ACK_WINDOW 65520

/*
 * IPv4 TCP connection match additional data.
 */
struct sfe_ipv4_tcp_connection_match {
	u8 win_scale;		/* Window scale */
	u32 max_win;		/* Maximum window size seen */
	u32 end;			/* Sequence number of the next byte to send (seq + segment length) */
	u32 max_end;		/* Sequence number of the last byte to ack */
};

/*
 * Bit flags for IPv4 connection matching entry.
 */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC (1<<0)
					/* Perform source translation */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST (1<<1)
					/* Perform destination translation */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK (1<<2)
					/* Ignore TCP sequence numbers */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR (1<<3)
					/* Fast Ethernet header write */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR (1<<4)
					/* Fast Ethernet header write */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK (1<<5)
					/* remark priority of SKB */
#define SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK (1<<6)
					/* remark DSCP of packet */

/*
 * IPv4 connection matching structure.
 */
struct sfe_ipv4_connection_match {
	/*
	 * References to other objects.
	 */
	struct sfe_ipv4_connection_match *next;
	struct sfe_ipv4_connection_match *prev;
	struct sfe_ipv4_connection *connection;
	struct sfe_ipv4_connection_match *counter_match;
					/* Matches the flow in the opposite direction as the one in *connection */
	struct sfe_ipv4_connection_match *active_next;
	struct sfe_ipv4_connection_match *active_prev;
	bool active;			/* Flag to indicate if we're on the active list */

	/*
	 * Characteristics that identify flows that match this rule.
	 */
	struct net_device *match_dev;	/* Network device */
	u8 match_protocol;		/* Protocol */
	__be32 match_src_ip;		/* Source IP address */
	__be32 match_dest_ip;		/* Destination IP address */
	__be16 match_src_port;		/* Source port/connection ident */
	__be16 match_dest_port;		/* Destination port/connection ident */

	/*
	 * Control the operations of the match.
	 */
	u32 flags;			/* Bit flags */
#ifdef CONFIG_NF_FLOW_COOKIE
	u32 flow_cookie;		/* used flow cookie, for debug */
#endif
#ifdef CONFIG_XFRM
	u32 flow_accel;             /* The flow accelerated or not */
#endif

	/*
	 * Connection state that we track once we match.
	 */
	union {				/* Protocol-specific state */
		struct sfe_ipv4_tcp_connection_match tcp;
	} protocol_state;
	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * rx_packet_count64/rx_byte_count64 after a sync period.
	 */
	u32 rx_packet_count;
	u32 rx_byte_count;

	/*
	 * Packet translation information.
	 */
	__be32 xlate_src_ip;		/* Address after source translation */
	__be16 xlate_src_port;	/* Port/connection ident after source translation */
	u16 xlate_src_csum_adjustment;
					/* Transport layer checksum adjustment after source translation */
	u16 xlate_src_partial_csum_adjustment;
					/* Transport layer pseudo header checksum adjustment after source translation */

	__be32 xlate_dest_ip;		/* Address after destination translation */
	__be16 xlate_dest_port;	/* Port/connection ident after destination translation */
	u16 xlate_dest_csum_adjustment;
					/* Transport layer checksum adjustment after destination translation */
	u16 xlate_dest_partial_csum_adjustment;
					/* Transport layer pseudo header checksum adjustment after destination translation */

	/*
	 * QoS information
	 */
	u32 priority;
	u32 dscp;

	/*
	 * Packet transmit information.
	 */
	struct net_device *xmit_dev;	/* Network device on which to transmit */
	unsigned short int xmit_dev_mtu;
					/* Interface MTU */
	u16 xmit_dest_mac[ETH_ALEN / 2];
					/* Destination MAC address to use when forwarding */
	u16 xmit_src_mac[ETH_ALEN / 2];
					/* Source MAC address to use when forwarding */

	/*
	 * Summary stats.
	 */
	u64 rx_packet_count64;
	u64 rx_byte_count64;
};

/*
 * Per-connection data structure.
 */
struct sfe_ipv4_connection {
	struct sfe_ipv4_connection *next;
					/* Pointer to the next entry in a hash chain */
	struct sfe_ipv4_connection *prev;
					/* Pointer to the previous entry in a hash chain */
	int protocol;			/* IP protocol number */
	__be32 src_ip;			/* Src IP addr pre-translation */
	__be32 src_ip_xlate;		/* Src IP addr post-translation */
	__be32 dest_ip;			/* Dest IP addr pre-translation */
	__be32 dest_ip_xlate;		/* Dest IP addr post-translation */
	__be16 src_port;		/* Src port pre-translation */
	__be16 src_port_xlate;		/* Src port post-translation */
	__be16 dest_port;		/* Dest port pre-translation */
	__be16 dest_port_xlate;		/* Dest port post-translation */
	struct sfe_ipv4_connection_match *original_match;
					/* Original direction matching structure */
	struct net_device *original_dev;
					/* Original direction source device */
	struct sfe_ipv4_connection_match *reply_match;
					/* Reply direction matching structure */
	struct net_device *reply_dev;	/* Reply direction source device */
	u64 last_sync_jiffies;		/* Jiffies count for the last sync */
	struct sfe_ipv4_connection *all_connections_next;
					/* Pointer to the next entry in the list of all connections */
	struct sfe_ipv4_connection *all_connections_prev;
					/* Pointer to the previous entry in the list of all connections */
	u32 mark;			/* mark for outgoing packet */
	u32 debug_read_seq;		/* sequence number for debug dump */
};

/*
 * IPv4 connections and hash table size information.
 */
#define SFE_IPV4_CONNECTION_HASH_SHIFT 12
#define SFE_IPV4_CONNECTION_HASH_SIZE (1 << SFE_IPV4_CONNECTION_HASH_SHIFT)
#define SFE_IPV4_CONNECTION_HASH_MASK (SFE_IPV4_CONNECTION_HASH_SIZE - 1)

#ifdef CONFIG_NF_FLOW_COOKIE
#define SFE_FLOW_COOKIE_SIZE 2048
#define SFE_FLOW_COOKIE_MASK 0x7ff

struct sfe_flow_cookie_entry {
	struct sfe_ipv4_connection_match *match;
	unsigned long last_clean_time;
};
#endif

enum sfe_ipv4_exception_events {
	SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL,
	SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION,
	SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK,
	SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS,
	SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION,
	SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH,
	SFE_IPV4_EXCEPTION_EVENT_NON_V4,
	SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT,
	SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE,
	SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL,
	SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR,
	SFE_IPV4_EXCEPTION_EVENT_LAST
};

static char *sfe_ipv4_exception_events_string[SFE_IPV4_EXCEPTION_EVENT_LAST] = {
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
	"ICMP_IPV4_HEADER_INCOMPLETE",
	"ICMP_IPV4_NON_V4",
	"ICMP_IPV4_IP_OPTIONS_INCOMPLETE",
	"ICMP_IPV4_UDP_HEADER_INCOMPLETE",
	"ICMP_IPV4_TCP_HEADER_INCOMPLETE",
	"ICMP_IPV4_UNHANDLED_PROTOCOL",
	"ICMP_NO_CONNECTION",
	"ICMP_FLUSHED_CONNECTION",
	"HEADER_INCOMPLETE",
	"BAD_TOTAL_LENGTH",
	"NON_V4",
	"NON_INITIAL_FRAGMENT",
	"DATAGRAM_INCOMPLETE",
	"IP_OPTIONS_INCOMPLETE",
	"UNHANDLED_PROTOCOL",
	"CLONED_SKB_UNSHARE_ERROR"
};

/*
 * Per-module structure.
 */
struct sfe_ipv4 {
	spinlock_t lock;		/* Lock for SMP correctness */
	struct sfe_ipv4_connection_match *active_head;
					/* Head of the list of recently active connections */
	struct sfe_ipv4_connection_match *active_tail;
					/* Tail of the list of recently active connections */
	struct sfe_ipv4_connection *all_connections_head;
					/* Head of the list of all connections */
	struct sfe_ipv4_connection *all_connections_tail;
					/* Tail of the list of all connections */
	unsigned int num_connections;	/* Number of connections */
	struct timer_list timer;	/* Timer used for periodic sync ops */
	sfe_sync_rule_callback_t __rcu sync_rule_callback;
					/* Callback function registered by a connection manager for stats syncing */
	struct sfe_ipv4_connection *conn_hash[SFE_IPV4_CONNECTION_HASH_SIZE];
					/* Connection hash table */
	struct sfe_ipv4_connection_match *conn_match_hash[SFE_IPV4_CONNECTION_HASH_SIZE];
					/* Connection match hash table */
#ifdef CONFIG_NF_FLOW_COOKIE
	struct sfe_flow_cookie_entry sfe_flow_cookie_table[SFE_FLOW_COOKIE_SIZE];
					/* flow cookie table*/
	flow_cookie_set_func_t flow_cookie_set_func;
					/* function used to configure flow cookie in hardware*/
	int flow_cookie_enable;
					/* Enable/disable flow cookie at runtime */
#endif

	/*
	 * Stats recorded in a sync period. These stats will be added to
	 * connection_xxx64 after a sync period.
	 */
	u32 connection_create_requests;
					/* Number of IPv4 connection create requests */
	u32 connection_create_collisions;
					/* Number of IPv4 connection create requests that collided with existing hash table entries */
	u32 connection_destroy_requests;
					/* Number of IPv4 connection destroy requests */
	u32 connection_destroy_misses;
					/* Number of IPv4 connection destroy requests that missed our hash table */
	u32 connection_match_hash_hits;
					/* Number of IPv4 connection match hash hits */
	u32 connection_match_hash_reorders;
					/* Number of IPv4 connection match hash reorders */
	u32 connection_flushes;		/* Number of IPv4 connection flushes */
	u32 packets_forwarded;		/* Number of IPv4 packets forwarded */
	u32 packets_not_forwarded;	/* Number of IPv4 packets not forwarded */
	u32 exception_events[SFE_IPV4_EXCEPTION_EVENT_LAST];

	/*
	 * Summary statistics.
	 */
	u64 connection_create_requests64;
					/* Number of IPv4 connection create requests */
	u64 connection_create_collisions64;
					/* Number of IPv4 connection create requests that collided with existing hash table entries */
	u64 connection_destroy_requests64;
					/* Number of IPv4 connection destroy requests */
	u64 connection_destroy_misses64;
					/* Number of IPv4 connection destroy requests that missed our hash table */
	u64 connection_match_hash_hits64;
					/* Number of IPv4 connection match hash hits */
	u64 connection_match_hash_reorders64;
					/* Number of IPv4 connection match hash reorders */
	u64 connection_flushes64;	/* Number of IPv4 connection flushes */
	u64 packets_forwarded64;	/* Number of IPv4 packets forwarded */
	u64 packets_not_forwarded64;
					/* Number of IPv4 packets not forwarded */
	u64 exception_events64[SFE_IPV4_EXCEPTION_EVENT_LAST];

	/*
	 * Control state.
	 */
	struct kobject *sys_sfe_ipv4;	/* sysfs linkage */
	int debug_dev;			/* Major number of the debug char device */
	u32 debug_read_seq;	/* sequence number for debug dump */
};

/*
 * Enumeration of the XML output.
 */
enum sfe_ipv4_debug_xml_states {
	SFE_IPV4_DEBUG_XML_STATE_START,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_START,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_CONNECTION,
	SFE_IPV4_DEBUG_XML_STATE_CONNECTIONS_END,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_START,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_EXCEPTION,
	SFE_IPV4_DEBUG_XML_STATE_EXCEPTIONS_END,
	SFE_IPV4_DEBUG_XML_STATE_STATS,
	SFE_IPV4_DEBUG_XML_STATE_END,
	SFE_IPV4_DEBUG_XML_STATE_DONE
};

/*
 * XML write state.
 */
struct sfe_ipv4_debug_xml_write_state {
	enum sfe_ipv4_debug_xml_states state;
					/* XML output file state machine state */
	int iter_exception;		/* Next exception iterator */
};

typedef bool (*sfe_ipv4_debug_xml_write_method_t)(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
						  int *total_read, struct sfe_ipv4_debug_xml_write_state *ws);

static struct sfe_ipv4 __si;

/*
 * sfe_ipv4_gen_ip_csum()
 *	Generate the IP checksum for an IPv4 header.
 *
 * Note that this function assumes that we have only 20 bytes of IP header.
 */
static inline u16 sfe_ipv4_gen_ip_csum(struct sfe_ipv4_ip_hdr *iph)
{
	u32 sum;
	u16 *i = (u16 *)iph;

	iph->check = 0;

	/*
	 * Generate the sum.
	 */
	sum = i[0] + i[1] + i[2] + i[3] + i[4] + i[5] + i[6] + i[7] + i[8] + i[9];

	/*
	 * Fold it to ones-complement form.
	 */
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);

	return (u16)sum ^ 0xffff;
}

/*
 * sfe_ipv4_get_connection_match_hash()
 *	Generate the hash used in connection match lookups.
 */
static inline unsigned int sfe_ipv4_get_connection_match_hash(struct net_device *dev, u8 protocol,
							      __be32 src_ip, __be16 src_port,
							      __be32 dest_ip, __be16 dest_port)
{
	size_t dev_addr = (size_t)dev;
	u32 hash = ((u32)dev_addr) ^ ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
}

/*
 * sfe_ipv4_find_sfe_ipv4_connection_match()
 *	Get the IPv4 flow match info that corresponds to a particular 5-tuple.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static struct sfe_ipv4_connection_match *
sfe_ipv4_find_sfe_ipv4_connection_match(struct sfe_ipv4 *si, struct net_device *dev, u8 protocol,
					__be32 src_ip, __be16 src_port,
					__be32 dest_ip, __be16 dest_port)
{
	struct sfe_ipv4_connection_match *cm;
	struct sfe_ipv4_connection_match *head;
	unsigned int conn_match_idx;

	conn_match_idx = sfe_ipv4_get_connection_match_hash(dev, protocol, src_ip, src_port, dest_ip, dest_port);
	cm = si->conn_match_hash[conn_match_idx];

	/*
	 * If we don't have anything in this chain then bail.
	 */
	if (unlikely(!cm)) {
		return NULL;
	}

	/*
	 * Hopefully the first entry is the one we want.
	 */
	if ((cm->match_src_port == src_port)
	    && (cm->match_dest_port == dest_port)
	    && (cm->match_src_ip == src_ip)
	    && (cm->match_dest_ip == dest_ip)
	    && (cm->match_protocol == protocol)
	    && (cm->match_dev == dev)) {
		si->connection_match_hash_hits++;
		return cm;
	}

	/*
	 * Unfortunately we didn't find it at head, so we search it in chain and
	 * move matching entry to the top of the hash chain. We presume that this
	 * will be reused again very quickly.
	 */
	head = cm;
	do {
		cm = cm->next;
	} while (cm && (cm->match_src_port != src_port
		 || cm->match_dest_port != dest_port
		 || cm->match_src_ip != src_ip
		 || cm->match_dest_ip != dest_ip
		 || cm->match_protocol != protocol
		 || cm->match_dev != dev));

	/*
	 * Not found then we're done.
	 */
	if (unlikely(!cm)) {
		return NULL;
	}

	/*
	 * We found a match so move it.
	 */
	if (cm->next) {
		cm->next->prev = cm->prev;
	}
	cm->prev->next = cm->next;
	cm->prev = NULL;
	cm->next = head;
	head->prev = cm;
	si->conn_match_hash[conn_match_idx] = cm;
	si->connection_match_hash_reorders++;

	return cm;
}

/*
 * sfe_ipv4_connection_match_update_summary_stats()
 *	Update the summary stats for a connection match entry.
 */
static inline void sfe_ipv4_connection_match_update_summary_stats(struct sfe_ipv4_connection_match *cm)
{
	cm->rx_packet_count64 += cm->rx_packet_count;
	cm->rx_packet_count = 0;
	cm->rx_byte_count64 += cm->rx_byte_count;
	cm->rx_byte_count = 0;
}

/*
 * sfe_ipv4_connection_match_compute_translations()
 *	Compute port and address translations for a connection match entry.
 */
static void sfe_ipv4_connection_match_compute_translations(struct sfe_ipv4_connection_match *cm)
{
	/*
	 * Before we insert the entry look to see if this is tagged as doing address
	 * translations.  If it is then work out the adjustment that we need to apply
	 * to the transport checksum.
	 */
	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
		/*
		 * Precompute an incremental checksum adjustment so we can
		 * edit packets in this stream very quickly.  The algorithm is from RFC1624.
		 */
		u16 src_ip_hi = cm->match_src_ip >> 16;
		u16 src_ip_lo = cm->match_src_ip & 0xffff;
		u32 xlate_src_ip = ~cm->xlate_src_ip;
		u16 xlate_src_ip_hi = xlate_src_ip >> 16;
		u16 xlate_src_ip_lo = xlate_src_ip & 0xffff;
		u16 xlate_src_port = ~cm->xlate_src_port;
		u32 adj;

		/*
		 * When we compute this fold it down to a 16-bit offset
		 * as that way we can avoid having to do a double
		 * folding of the twos-complement result because the
		 * addition of 2 16-bit values cannot cause a double
		 * wrap-around!
		 */
		adj = src_ip_hi + src_ip_lo + cm->match_src_port
		      + xlate_src_ip_hi + xlate_src_ip_lo + xlate_src_port;
		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_src_csum_adjustment = (u16)adj;

	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
		/*
		 * Precompute an incremental checksum adjustment so we can
		 * edit packets in this stream very quickly.  The algorithm is from RFC1624.
		 */
		u16 dest_ip_hi = cm->match_dest_ip >> 16;
		u16 dest_ip_lo = cm->match_dest_ip & 0xffff;
		u32 xlate_dest_ip = ~cm->xlate_dest_ip;
		u16 xlate_dest_ip_hi = xlate_dest_ip >> 16;
		u16 xlate_dest_ip_lo = xlate_dest_ip & 0xffff;
		u16 xlate_dest_port = ~cm->xlate_dest_port;
		u32 adj;

		/*
		 * When we compute this fold it down to a 16-bit offset
		 * as that way we can avoid having to do a double
		 * folding of the twos-complement result because the
		 * addition of 2 16-bit values cannot cause a double
		 * wrap-around!
		 */
		adj = dest_ip_hi + dest_ip_lo + cm->match_dest_port
		      + xlate_dest_ip_hi + xlate_dest_ip_lo + xlate_dest_port;
		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_dest_csum_adjustment = (u16)adj;
	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC) {
		u32 adj = ~cm->match_src_ip + cm->xlate_src_ip;
		if (adj < cm->xlate_src_ip) {
			adj++;
		}

		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_src_partial_csum_adjustment = (u16)adj;
	}

	if (cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST) {
		u32 adj = ~cm->match_dest_ip + cm->xlate_dest_ip;
		if (adj < cm->xlate_dest_ip) {
			adj++;
		}

		adj = (adj & 0xffff) + (adj >> 16);
		adj = (adj & 0xffff) + (adj >> 16);
		cm->xlate_dest_partial_csum_adjustment = (u16)adj;
	}

}

/*
 * sfe_ipv4_update_summary_stats()
 *	Update the summary stats.
 */
static void sfe_ipv4_update_summary_stats(struct sfe_ipv4 *si)
{
	int i;

	si->connection_create_requests64 += si->connection_create_requests;
	si->connection_create_requests = 0;
	si->connection_create_collisions64 += si->connection_create_collisions;
	si->connection_create_collisions = 0;
	si->connection_destroy_requests64 += si->connection_destroy_requests;
	si->connection_destroy_requests = 0;
	si->connection_destroy_misses64 += si->connection_destroy_misses;
	si->connection_destroy_misses = 0;
	si->connection_match_hash_hits64 += si->connection_match_hash_hits;
	si->connection_match_hash_hits = 0;
	si->connection_match_hash_reorders64 += si->connection_match_hash_reorders;
	si->connection_match_hash_reorders = 0;
	si->connection_flushes64 += si->connection_flushes;
	si->connection_flushes = 0;
	si->packets_forwarded64 += si->packets_forwarded;
	si->packets_forwarded = 0;
	si->packets_not_forwarded64 += si->packets_not_forwarded;
	si->packets_not_forwarded = 0;

	for (i = 0; i < SFE_IPV4_EXCEPTION_EVENT_LAST; i++) {
		si->exception_events64[i] += si->exception_events[i];
		si->exception_events[i] = 0;
	}
}

/*
 * sfe_ipv4_insert_sfe_ipv4_connection_match()
 *	Insert a connection match into the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline void sfe_ipv4_insert_sfe_ipv4_connection_match(struct sfe_ipv4 *si,
							     struct sfe_ipv4_connection_match *cm)
{
	struct sfe_ipv4_connection_match **hash_head;
	struct sfe_ipv4_connection_match *prev_head;
	unsigned int conn_match_idx
		= sfe_ipv4_get_connection_match_hash(cm->match_dev, cm->match_protocol,
						     cm->match_src_ip, cm->match_src_port,
						     cm->match_dest_ip, cm->match_dest_port);

	hash_head = &si->conn_match_hash[conn_match_idx];
	prev_head = *hash_head;
	cm->prev = NULL;
	if (prev_head) {
		prev_head->prev = cm;
	}

	cm->next = prev_head;
	*hash_head = cm;

#ifdef CONFIG_NF_FLOW_COOKIE
	if (!si->flow_cookie_enable)
		return;

	/*
	 * Configure hardware to put a flow cookie in packet of this flow,
	 * then we can accelerate the lookup process when we received this packet.
	 */
	for (conn_match_idx = 1; conn_match_idx < SFE_FLOW_COOKIE_SIZE; conn_match_idx++) {
		struct sfe_flow_cookie_entry *entry = &si->sfe_flow_cookie_table[conn_match_idx];

		if ((NULL == entry->match) && time_is_before_jiffies(entry->last_clean_time + HZ)) {
			flow_cookie_set_func_t func;

			rcu_read_lock();
			func = rcu_dereference(si->flow_cookie_set_func);
			if (func) {
				if (!func(cm->match_protocol, cm->match_src_ip, cm->match_src_port,
					 cm->match_dest_ip, cm->match_dest_port, conn_match_idx)) {
					entry->match = cm;
					cm->flow_cookie = conn_match_idx;
				}
			}
			rcu_read_unlock();

			break;
		}
	}
#endif
}

/*
 * sfe_ipv4_remove_sfe_ipv4_connection_match()
 *	Remove a connection match object from the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline void sfe_ipv4_remove_sfe_ipv4_connection_match(struct sfe_ipv4 *si, struct sfe_ipv4_connection_match *cm)
{
#ifdef CONFIG_NF_FLOW_COOKIE
	if (si->flow_cookie_enable) {
		/*
		 * Tell hardware that we no longer need a flow cookie in packet of this flow
		 */
		unsigned int conn_match_idx;

		for (conn_match_idx = 1; conn_match_idx < SFE_FLOW_COOKIE_SIZE; conn_match_idx++) {
			struct sfe_flow_cookie_entry *entry = &si->sfe_flow_cookie_table[conn_match_idx];

			if (cm == entry->match) {
				flow_cookie_set_func_t func;

				rcu_read_lock();
				func = rcu_dereference(si->flow_cookie_set_func);
				if (func) {
					func(cm->match_protocol, cm->match_src_ip, cm->match_src_port,
					     cm->match_dest_ip, cm->match_dest_port, 0);
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

	/*
	 * Unlink the connection match entry from the hash.
	 */
	if (cm->prev) {
		cm->prev->next = cm->next;
	} else {
		unsigned int conn_match_idx
			= sfe_ipv4_get_connection_match_hash(cm->match_dev, cm->match_protocol,
							     cm->match_src_ip, cm->match_src_port,
							     cm->match_dest_ip, cm->match_dest_port);
		si->conn_match_hash[conn_match_idx] = cm->next;
	}

	if (cm->next) {
		cm->next->prev = cm->prev;
	}

	/*
	 * If the connection match entry is in the active list remove it.
	 */
	if (cm->active) {
		if (likely(cm->active_prev)) {
			cm->active_prev->active_next = cm->active_next;
		} else {
			si->active_head = cm->active_next;
		}

		if (likely(cm->active_next)) {
			cm->active_next->active_prev = cm->active_prev;
		} else {
			si->active_tail = cm->active_prev;
		}
	}
}

/*
 * sfe_ipv4_get_connection_hash()
 *	Generate the hash used in connection lookups.
 */
static inline unsigned int sfe_ipv4_get_connection_hash(u8 protocol, __be32 src_ip, __be16 src_port,
							__be32 dest_ip, __be16 dest_port)
{
	u32 hash = ntohl(src_ip ^ dest_ip) ^ protocol ^ ntohs(src_port ^ dest_port);
	return ((hash >> SFE_IPV4_CONNECTION_HASH_SHIFT) ^ hash) & SFE_IPV4_CONNECTION_HASH_MASK;
}

/*
 * sfe_ipv4_find_sfe_ipv4_connection()
 *	Get the IPv4 connection info that corresponds to a particular 5-tuple.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static inline struct sfe_ipv4_connection *sfe_ipv4_find_sfe_ipv4_connection(struct sfe_ipv4 *si, u32 protocol,
									    __be32 src_ip, __be16 src_port,
									    __be32 dest_ip, __be16 dest_port)
{
	struct sfe_ipv4_connection *c;
	unsigned int conn_idx = sfe_ipv4_get_connection_hash(protocol, src_ip, src_port, dest_ip, dest_port);
	c = si->conn_hash[conn_idx];

	/*
	 * If we don't have anything in this chain then bale.
	 */
	if (unlikely(!c)) {
		return NULL;
	}

	/*
	 * Hopefully the first entry is the one we want.
	 */
	if ((c->src_port == src_port)
	    && (c->dest_port == dest_port)
	    && (c->src_ip == src_ip)
	    && (c->dest_ip == dest_ip)
	    && (c->protocol == protocol)) {
		return c;
	}

	/*
	 * Unfortunately we didn't find it at head, so we search it in chain.
	 */
	do {
		c = c->next;
	} while (c && (c->src_port != src_port
		 || c->dest_port != dest_port
		 || c->src_ip != src_ip
		 || c->dest_ip != dest_ip
		 || c->protocol != protocol));

	/*
	 * Will need connection entry for next create/destroy metadata,
	 * So no need to re-order entry for these requests
	 */
	return c;
}

/*
 * sfe_ipv4_mark_rule()
 *	Updates the mark for a current offloaded connection
 *
 * Will take hash lock upon entry
 */
void sfe_ipv4_mark_rule(struct sfe_connection_mark *mark)
{
	struct sfe_ipv4 *si = &__si;
	struct sfe_ipv4_connection *c;

	spin_lock_bh(&si->lock);
	c = sfe_ipv4_find_sfe_ipv4_connection(si, mark->protocol,
					      mark->src_ip.ip, mark->src_port,
					      mark->dest_ip.ip, mark->dest_port);
	if (c) {
		WARN_ON((0 != c->mark) && (0 == mark->mark));
		c->mark = mark->mark;
	}
	spin_unlock_bh(&si->lock);

	if (c) {
		DEBUG_TRACE("Matching connection found for mark, "
			    "setting from %08x to %08x\n",
			    c->mark, mark->mark);
	}
}

/*
 * sfe_ipv4_insert_sfe_ipv4_connection()
 *	Insert a connection into the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static void sfe_ipv4_insert_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c)
{
	struct sfe_ipv4_connection **hash_head;
	struct sfe_ipv4_connection *prev_head;
	unsigned int conn_idx;

	/*
	 * Insert entry into the connection hash.
	 */
	conn_idx = sfe_ipv4_get_connection_hash(c->protocol, c->src_ip, c->src_port,
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
	sfe_ipv4_insert_sfe_ipv4_connection_match(si, c->original_match);
	sfe_ipv4_insert_sfe_ipv4_connection_match(si, c->reply_match);
}

/*
 * sfe_ipv4_remove_sfe_ipv4_connection()
 *	Remove a sfe_ipv4_connection object from the hash.
 *
 * On entry we must be holding the lock that protects the hash table.
 */
static void sfe_ipv4_remove_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c)
{
	/*
	 * Remove the connection match objects.
	 */
	sfe_ipv4_remove_sfe_ipv4_connection_match(si, c->reply_match);
	sfe_ipv4_remove_sfe_ipv4_connection_match(si, c->original_match);

	/*
	 * Unlink the connection.
	 */
	if (c->prev) {
		c->prev->next = c->next;
	} else {
		unsigned int conn_idx = sfe_ipv4_get_connection_hash(c->protocol, c->src_ip, c->src_port,
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

	si->num_connections--;
}

/*
 * sfe_ipv4_sync_sfe_ipv4_connection()
 *	Sync a connection.
 *
 * On entry to this function we expect that the lock for the connection is either
 * already held or isn't required.
 */
static void sfe_ipv4_gen_sync_sfe_ipv4_connection(struct sfe_ipv4 *si, struct sfe_ipv4_connection *c,
						  struct sfe_connection_sync *sis, sfe_sync_reason_t reason,
						  u64 now_jiffies)
{
	struct sfe_ipv4_connection_match *original_cm;
	struct sfe_ipv4_connection_match *reply_cm;

	/*
	 * Fill in the update message.
	 */
	sis->is_v6 = 0;
	sis->protocol = c->protocol;
	sis->src_ip.ip = c->src_ip;
	sis->src_ip_xlate.ip = c->src_ip_xlate;
	sis->dest_ip.ip = c->dest_ip;
	sis->dest_ip_xlate.ip = c->dest_ip_xlate;
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

	sis->src_new_packet_count = original_cm->rx_packet_count;
	sis->src_new_byte_count = original_cm->rx_byte_count;
	sis->dest_new_packet_count = reply_cm->rx_packet_count;
	sis->dest_new_byte_count = reply_cm->rx_byte_count;

	sfe_ipv4_connection_match_update_summary_stats(original_cm);
	sfe_ipv4_connection_match_update_summary_stats(reply_cm);

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
 * sfe_ipv4_flush_sfe_ipv4_connection()
 *	Flush a connection and free all associated resources.
 *
 * We need to be called with bottom halves disabled locally as we need to acquire
 * the connection hash lock and release it again.  In general we're actually called
 * from within a BH and so we're fine, but we're also called when connections are
 * torn down.
 */
static void sfe_ipv4_flush_sfe_ipv4_connection(struct sfe_ipv4 *si,
					       struct sfe_ipv4_connection *c,
					       sfe_sync_reason_t reason)
{
	struct sfe_connection_sync sis;
	u64 now_jiffies;
	sfe_sync_rule_callback_t sync_rule_callback;

	rcu_read_lock();
	spin_lock_bh(&si->lock);
	si->connection_flushes++;
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
	spin_unlock_bh(&si->lock);

	if (sync_rule_callback) {
		/*
		 * Generate a sync message and then sync.
		 */
		now_jiffies = get_jiffies_64();
		sfe_ipv4_gen_sync_sfe_ipv4_connection(si, c, &sis, reason, now_jiffies);
		sync_rule_callback(&sis);
	}

	rcu_read_unlock();

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
 * sfe_ipv4_recv_udp()
 *	Handle UDP packet receives and forwarding.
 */
static int sfe_ipv4_recv_udp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct sfe_ipv4_ip_hdr *iph, unsigned int ihl, bool flush_on_find)
{
	struct sfe_ipv4_udp_hdr *udph;
	__be32 src_ip;
	__be32 dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv4_connection_match *cm;
	u8 ttl;
	struct net_device *xmit_dev;

	/*
	 * Is our packet too short to contain a valid UDP header?
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_udp_hdr) + ihl)))) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_HEADER_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("packet too short for UDP header\n");
		return 0;
	}

	/*
	 * Read the IP address and port information.  Read the IP header data first
	 * because we've almost certainly got that in the cache.  We may not yet have
	 * the UDP header cached though so allow more time for any prefetching.
	 */
	src_ip = iph->saddr;
	dest_ip = iph->daddr;

	udph = (struct sfe_ipv4_udp_hdr *)(skb->data + ihl);
	src_port = udph->source;
	dest_port = udph->dest;

	spin_lock_bh(&si->lock);

	/*
	 * Look for a connection match.
	 */
#ifdef CONFIG_NF_FLOW_COOKIE
	cm = si->sfe_flow_cookie_table[skb->flow_cookie & SFE_FLOW_COOKIE_MASK].match;
	if (unlikely(!cm)) {
		cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_UDP, src_ip, src_port, dest_ip, dest_port);
	}
#else
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_UDP, src_ip, src_port, dest_ip, dest_port);
#endif
	if (unlikely(!cm)) {
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NO_CONNECTION]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("no connection found\n");
		return 0;
	}

	/*
	 * If our packet has beern marked as "flush on find" we can't actually
	 * forward it in the fast path, but now that we've found an associated
	 * connection we can flush that out before we process the packet.
	 */
	if (unlikely(flush_on_find)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("flush on find\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

#ifdef CONFIG_XFRM
	/*
	 * We can't accelerate the flow on this direction, just let it go
	 * through the slow path.
	 */
	if (unlikely(!cm->flow_accel)) {
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);
		return 0;
	}
#endif

	/*
	 * Does our TTL allow forwarding?
	 */
	ttl = iph->ttl;
	if (unlikely(ttl < 2)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_SMALL_TTL]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("ttl too low\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

	/*
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely(len > cm->xmit_dev_mtu)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UDP_NEEDS_FRAGMENTATION]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("larger than mtu\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

	/*
	 * From this point on we're good to modify the packet.
	 */

	/*
	 * Check if skb was cloned. If it was, unshare it. Because
	 * the data area is going to be written in this path and we don't want to
	 * change the cloned skb's data section.
	 */
	if (unlikely(skb_cloned(skb))) {
		DEBUG_TRACE("%p: skb is a cloned skb\n", skb);
		skb = skb_unshare(skb, GFP_ATOMIC);
                if (!skb) {
			DEBUG_WARN("Failed to unshare the cloned skb\n");
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			return 0;
		}

		/*
		 * Update the iph and udph pointers with the unshared skb's data area.
		 */
		iph = (struct sfe_ipv4_ip_hdr *)skb->data;
		udph = (struct sfe_ipv4_udp_hdr *)(skb->data + ihl);
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
	}

	/*
	 * Decrement our TTL.
	 */
	iph->ttl = ttl - 1;

	/*
	 * Do we have to perform translations of the source address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
		u16 udp_csum;

		iph->saddr = cm->xlate_src_ip;
		udph->source = cm->xlate_src_port;

		/*
		 * Do we have a non-zero UDP checksum?  If we do then we need
		 * to update it.
		 */
		udp_csum = udph->check;
		if (likely(udp_csum)) {
			u32 sum;

			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
				sum = udp_csum + cm->xlate_src_partial_csum_adjustment;
			} else {
				sum = udp_csum + cm->xlate_src_csum_adjustment;
			}

			sum = (sum & 0xffff) + (sum >> 16);
			udph->check = (u16)sum;
		}
	}

	/*
	 * Do we have to perform translations of the destination address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
		u16 udp_csum;

		iph->daddr = cm->xlate_dest_ip;
		udph->dest = cm->xlate_dest_port;

		/*
		 * Do we have a non-zero UDP checksum?  If we do then we need
		 * to update it.
		 */
		udp_csum = udph->check;
		if (likely(udp_csum)) {
			u32 sum;

			if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
				sum = udp_csum + cm->xlate_dest_partial_csum_adjustment;
			} else {
				sum = udp_csum + cm->xlate_dest_csum_adjustment;
			}

			sum = (sum & 0xffff) + (sum >> 16);
			udph->check = (u16)sum;
		}
	}

	/*
	 * Replace the IP checksum.
	 */
	iph->check = sfe_ipv4_gen_ip_csum(iph);

	/*
	 * Update traffic stats.
	 */
	cm->rx_packet_count++;
	cm->rx_byte_count += len;

	/*
	 * If we're not already on the active list then insert ourselves at the tail
	 * of the current list.
	 */
	if (unlikely(!cm->active)) {
		cm->active = true;
		cm->active_prev = si->active_tail;
		if (likely(si->active_tail)) {
			si->active_tail->active_next = cm;
		} else {
			si->active_head = cm;
		}
		si->active_tail = cm;
	}

	xmit_dev = cm->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * Check to see if we need to write a header.
	 */
	if (likely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			dev_hard_header(skb, xmit_dev, ETH_P_IP,
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
		} else {
			/*
			 * For the simple case we write this really fast.
			 */
			struct sfe_ipv4_eth_hdr *eth = (struct sfe_ipv4_eth_hdr *)__skb_push(skb, ETH_HLEN);
			eth->h_proto = htons(ETH_P_IP);
			eth->h_dest[0] = cm->xmit_dest_mac[0];
			eth->h_dest[1] = cm->xmit_dest_mac[1];
			eth->h_dest[2] = cm->xmit_dest_mac[2];
			eth->h_source[0] = cm->xmit_src_mac[0];
			eth->h_source[1] = cm->xmit_src_mac[1];
			eth->h_source[2] = cm->xmit_src_mac[2];
		}
	}

	/*
	 * Update priority of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
	}

	/*
	 * Mark outgoing packet.
	 */
	skb->mark = cm->connection->mark;
	if (skb->mark) {
		DEBUG_TRACE("SKB MARK is NON ZERO %x\n", skb->mark);
	}

	si->packets_forwarded++;
	spin_unlock_bh(&si->lock);

	/*
	 * We're going to check for GSO flags when we transmit the packet so
	 * start fetching the necessary cache line now.
	 */
	prefetch(skb_shinfo(skb));

	/*
	 * Mark that this packet has been fast forwarded.
	 */
	skb->fast_forwarded = 1;

	/*
	 * Send the packet on its way.
	 */
	dev_queue_xmit(skb);

	return 1;
}

/*
 * sfe_ipv4_process_tcp_option_sack()
 *	Parse TCP SACK option and update ack according
 */
static bool sfe_ipv4_process_tcp_option_sack(const struct sfe_ipv4_tcp_hdr *th, const u32 data_offs,
					     u32 *ack)
{
	u32 length = sizeof(struct sfe_ipv4_tcp_hdr);
	u8 *ptr = (u8 *)th + length;

	/*
	 * Ignore processing if TCP packet has only TIMESTAMP option.
	 */
	if (likely(data_offs == length + TCPOLEN_TIMESTAMP + 1 + 1)
	    && likely(ptr[0] == TCPOPT_NOP)
	    && likely(ptr[1] == TCPOPT_NOP)
	    && likely(ptr[2] == TCPOPT_TIMESTAMP)
	    && likely(ptr[3] == TCPOLEN_TIMESTAMP)) {
		return true;
	}

	/*
	 * TCP options. Parse SACK option.
	 */
	while (length < data_offs) {
		u8 size;
		u8 kind;

		ptr = (u8 *)th + length;
		kind = *ptr;

		/*
		 * NOP, for padding
		 * Not in the switch because to fast escape and to not calculate size
		 */
		if (kind == TCPOPT_NOP) {
			length++;
			continue;
		}

		if (kind == TCPOPT_SACK) {
			u32 sack = 0;
			u8 re = 1 + 1;

			size = *(ptr + 1);
			if ((size < (1 + 1 + TCPOLEN_SACK_PERBLOCK))
			    || ((size - (1 + 1)) % (TCPOLEN_SACK_PERBLOCK))
			    || (size > (data_offs - length))) {
				return false;
			}

			re += 4;
			while (re < size) {
				u32 sack_re;
				u8 *sptr = ptr + re;
				sack_re = (sptr[0] << 24) | (sptr[1] << 16) | (sptr[2] << 8) | sptr[3];
				if (sack_re > sack) {
					sack = sack_re;
				}
				re += TCPOLEN_SACK_PERBLOCK;
			}
			if (sack > *ack) {
				*ack = sack;
			}
			length += size;
			continue;
		}
		if (kind == TCPOPT_EOL) {
			return true;
		}
		size = *(ptr + 1);
		if (size < 2) {
			return false;
		}
		length += size;
	}

	return true;
}

/*
 * sfe_ipv4_recv_tcp()
 *	Handle TCP packet receives and forwarding.
 */
static int sfe_ipv4_recv_tcp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
			     unsigned int len, struct sfe_ipv4_ip_hdr *iph, unsigned int ihl, bool flush_on_find)
{
	struct sfe_ipv4_tcp_hdr *tcph;
	__be32 src_ip;
	__be32 dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv4_connection_match *cm;
	struct sfe_ipv4_connection_match *counter_cm;
	u8 ttl;
	u32 flags;
	struct net_device *xmit_dev;

	/*
	 * Is our packet too short to contain a valid UDP header?
	 */
	if (unlikely(!pskb_may_pull(skb, (sizeof(struct sfe_ipv4_tcp_hdr) + ihl)))) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_HEADER_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("packet too short for TCP header\n");
		return 0;
	}

	/*
	 * Read the IP address and port information.  Read the IP header data first
	 * because we've almost certainly got that in the cache.  We may not yet have
	 * the TCP header cached though so allow more time for any prefetching.
	 */
	src_ip = iph->saddr;
	dest_ip = iph->daddr;

	tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
	src_port = tcph->source;
	dest_port = tcph->dest;
	flags = tcp_flag_word(tcph);

	spin_lock_bh(&si->lock);

	/*
	 * Look for a connection match.
	 */
#ifdef CONFIG_NF_FLOW_COOKIE
	cm = si->sfe_flow_cookie_table[skb->flow_cookie & SFE_FLOW_COOKIE_MASK].match;
	if (unlikely(!cm)) {
		cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
	}
#else
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, IPPROTO_TCP, src_ip, src_port, dest_ip, dest_port);
#endif
	if (unlikely(!cm)) {
		/*
		 * We didn't get a connection but as TCP is connection-oriented that
		 * may be because this is a non-fast connection (not running established).
		 * For diagnostic purposes we differentiate this here.
		 */
		if (likely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) == TCP_FLAG_ACK)) {
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_FAST_FLAGS]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("no connection found - fast flags\n");
			return 0;
		}
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NO_CONNECTION_SLOW_FLAGS]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("no connection found - slow flags: 0x%x\n",
			    flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK));
		return 0;
	}

	/*
	 * If our packet has beern marked as "flush on find" we can't actually
	 * forward it in the fast path, but now that we've found an associated
	 * connection we can flush that out before we process the packet.
	 */
	if (unlikely(flush_on_find)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_IP_OPTIONS_OR_INITIAL_FRAGMENT]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("flush on find\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

#ifdef CONFIG_XFRM
	/*
	 * We can't accelerate the flow on this direction, just let it go
	 * through the slow path.
	 */
	if (unlikely(!cm->flow_accel)) {
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);
		return 0;
	}
#endif
	/*
	 * Does our TTL allow forwarding?
	 */
	ttl = iph->ttl;
	if (unlikely(ttl < 2)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_TTL]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("ttl too low\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

	/*
	 * If our packet is larger than the MTU of the transmit interface then
	 * we can't forward it easily.
	 */
	if (unlikely((len > cm->xmit_dev_mtu) && !skb_is_gso(skb))) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_NEEDS_FRAGMENTATION]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("larger than mtu\n");
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

	/*
	 * Look at our TCP flags.  Anything missing an ACK or that has RST, SYN or FIN
	 * set is not a fast path packet.
	 */
	if (unlikely((flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK)) != TCP_FLAG_ACK)) {
		struct sfe_ipv4_connection *c = cm->connection;
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_FLAGS]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("TCP flags: 0x%x are not fast\n",
			    flags & (TCP_FLAG_SYN | TCP_FLAG_RST | TCP_FLAG_FIN | TCP_FLAG_ACK));
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
		return 0;
	}

	counter_cm = cm->counter_match;

	/*
	 * Are we doing sequence number checking?
	 */
	if (likely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK))) {
		u32 seq;
		u32 ack;
		u32 sack;
		u32 data_offs;
		u32 end;
		u32 left_edge;
		u32 scaled_win;
		u32 max_end;

		/*
		 * Is our sequence fully past the right hand edge of the window?
		 */
		seq = ntohl(tcph->seq);
		if (unlikely((s32)(seq - (cm->protocol_state.tcp.max_end + 1)) > 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_EXCEEDS_RIGHT_EDGE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("seq: %u exceeds right edge: %u\n",
				    seq, cm->protocol_state.tcp.max_end + 1);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Check that our TCP data offset isn't too short.
		 */
		data_offs = tcph->doff << 2;
		if (unlikely(data_offs < sizeof(struct sfe_ipv4_tcp_hdr))) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SMALL_DATA_OFFS]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP data offset: %u, too small\n", data_offs);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Update ACK according to any SACK option.
		 */
		ack = ntohl(tcph->ack_seq);
		sack = ack;
		if (unlikely(!sfe_ipv4_process_tcp_option_sack(tcph, data_offs, &sack))) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BAD_SACK]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP option SACK size is wrong\n");
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Check that our TCP data offset isn't past the end of the packet.
		 */
		data_offs += sizeof(struct sfe_ipv4_ip_hdr);
		if (unlikely(len < data_offs)) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_BIG_DATA_OFFS]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("TCP data offset: %u, past end of packet: %u\n",
				    data_offs, len);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		end = seq + len - data_offs;

		/*
		 * Is our sequence fully before the left hand edge of the window?
		 */
		if (unlikely((s32)(end - (cm->protocol_state.tcp.end
						- counter_cm->protocol_state.tcp.max_win - 1)) < 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_SEQ_BEFORE_LEFT_EDGE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("seq: %u before left edge: %u\n",
				    end, cm->protocol_state.tcp.end - counter_cm->protocol_state.tcp.max_win - 1);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Are we acking data that is to the right of what has been sent?
		 */
		if (unlikely((s32)(sack - (counter_cm->protocol_state.tcp.end + 1)) > 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_EXCEEDS_RIGHT_EDGE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("ack: %u exceeds right edge: %u\n",
				    sack, counter_cm->protocol_state.tcp.end + 1);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Is our ack too far before the left hand edge of the window?
		 */
		left_edge = counter_cm->protocol_state.tcp.end
			    - cm->protocol_state.tcp.max_win
			    - SFE_IPV4_TCP_MAX_ACK_WINDOW
			    - 1;
		if (unlikely((s32)(sack - left_edge) < 0)) {
			struct sfe_ipv4_connection *c = cm->connection;
			sfe_ipv4_remove_sfe_ipv4_connection(si, c);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_TCP_ACK_BEFORE_LEFT_EDGE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("ack: %u before left edge: %u\n", sack, left_edge);
			sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
			return 0;
		}

		/*
		 * Have we just seen the largest window size yet for this connection?  If yes
		 * then we need to record the new value.
		 */
		scaled_win = ntohs(tcph->window) << cm->protocol_state.tcp.win_scale;
		scaled_win += (sack - ack);
		if (unlikely(cm->protocol_state.tcp.max_win < scaled_win)) {
			cm->protocol_state.tcp.max_win = scaled_win;
		}

		/*
		 * If our sequence and/or ack numbers have advanced then record the new state.
		 */
		if (likely((s32)(end - cm->protocol_state.tcp.end) >= 0)) {
			cm->protocol_state.tcp.end = end;
		}

		max_end = sack + scaled_win;
		if (likely((s32)(max_end - counter_cm->protocol_state.tcp.max_end) >= 0)) {
			counter_cm->protocol_state.tcp.max_end = max_end;
		}
	}

	/*
	 * From this point on we're good to modify the packet.
	 */

	/*
	 * Check if skb was cloned. If it was, unshare it. Because
	 * the data area is going to be written in this path and we don't want to
	 * change the cloned skb's data section.
	 */
	if (unlikely(skb_cloned(skb))) {
		DEBUG_TRACE("%p: skb is a cloned skb\n", skb);
		skb = skb_unshare(skb, GFP_ATOMIC);
                if (!skb) {
			DEBUG_WARN("Failed to unshare the cloned skb\n");
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_CLONED_SKB_UNSHARE_ERROR]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			return 0;
		}

		/*
		 * Update the iph and tcph pointers with the unshared skb's data area.
		 */
		iph = (struct sfe_ipv4_ip_hdr *)skb->data;
		tcph = (struct sfe_ipv4_tcp_hdr *)(skb->data + ihl);
	}

	/*
	 * Update DSCP
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK)) {
		iph->tos = (iph->tos & SFE_IPV4_DSCP_MASK) | cm->dscp;
	}

	/*
	 * Decrement our TTL.
	 */
	iph->ttl = ttl - 1;

	/*
	 * Do we have to perform translations of the source address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC)) {
		u16 tcp_csum;
		u32 sum;

		iph->saddr = cm->xlate_src_ip;
		tcph->source = cm->xlate_src_port;

		/*
		 * Do we have a non-zero UDP checksum?  If we do then we need
		 * to update it.
		 */
		tcp_csum = tcph->check;
		if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
			sum = tcp_csum + cm->xlate_src_partial_csum_adjustment;
		} else {
			sum = tcp_csum + cm->xlate_src_csum_adjustment;
		}

		sum = (sum & 0xffff) + (sum >> 16);
		tcph->check = (u16)sum;
	}

	/*
	 * Do we have to perform translations of the destination address/port?
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST)) {
		u16 tcp_csum;
		u32 sum;

		iph->daddr = cm->xlate_dest_ip;
		tcph->dest = cm->xlate_dest_port;

		/*
		 * Do we have a non-zero UDP checksum?  If we do then we need
		 * to update it.
		 */
		tcp_csum = tcph->check;
		if (unlikely(skb->ip_summed == CHECKSUM_PARTIAL)) {
			sum = tcp_csum + cm->xlate_dest_partial_csum_adjustment;
		} else {
			sum = tcp_csum + cm->xlate_dest_csum_adjustment;
		}

		sum = (sum & 0xffff) + (sum >> 16);
		tcph->check = (u16)sum;
	}

	/*
	 * Replace the IP checksum.
	 */
	iph->check = sfe_ipv4_gen_ip_csum(iph);

	/*
	 * Update traffic stats.
	 */
	cm->rx_packet_count++;
	cm->rx_byte_count += len;

	/*
	 * If we're not already on the active list then insert ourselves at the tail
	 * of the current list.
	 */
	if (unlikely(!cm->active)) {
		cm->active = true;
		cm->active_prev = si->active_tail;
		if (likely(si->active_tail)) {
			si->active_tail->active_next = cm;
		} else {
			si->active_head = cm;
		}
		si->active_tail = cm;
	}

	xmit_dev = cm->xmit_dev;
	skb->dev = xmit_dev;

	/*
	 * Check to see if we need to write a header.
	 */
	if (likely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR)) {
		if (unlikely(!(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR))) {
			dev_hard_header(skb, xmit_dev, ETH_P_IP,
					cm->xmit_dest_mac, cm->xmit_src_mac, len);
		} else {
			/*
			 * For the simple case we write this really fast.
			 */
			struct sfe_ipv4_eth_hdr *eth = (struct sfe_ipv4_eth_hdr *)__skb_push(skb, ETH_HLEN);
			eth->h_proto = htons(ETH_P_IP);
			eth->h_dest[0] = cm->xmit_dest_mac[0];
			eth->h_dest[1] = cm->xmit_dest_mac[1];
			eth->h_dest[2] = cm->xmit_dest_mac[2];
			eth->h_source[0] = cm->xmit_src_mac[0];
			eth->h_source[1] = cm->xmit_src_mac[1];
			eth->h_source[2] = cm->xmit_src_mac[2];
		}
	}

	/*
	 * Update priority of skb.
	 */
	if (unlikely(cm->flags & SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK)) {
		skb->priority = cm->priority;
	}

	/*
	 * Mark outgoing packet
	 */
	skb->mark = cm->connection->mark;
	if (skb->mark) {
		DEBUG_TRACE("SKB MARK is NON ZERO %x\n", skb->mark);
	}

	si->packets_forwarded++;
	spin_unlock_bh(&si->lock);

	/*
	 * We're going to check for GSO flags when we transmit the packet so
	 * start fetching the necessary cache line now.
	 */
	prefetch(skb_shinfo(skb));

	/*
	 * Mark that this packet has been fast forwarded.
	 */
	skb->fast_forwarded = 1;

	/*
	 * Send the packet on its way.
	 */
	dev_queue_xmit(skb);

	return 1;
}

/*
 * sfe_ipv4_recv_icmp()
 *	Handle ICMP packet receives.
 *
 * ICMP packets aren't handled as a "fast path" and always have us process them
 * through the default Linux stack.  What we do need to do is look for any errors
 * about connections we are handling in the fast path.  If we find any such
 * connections then we want to flush their state so that the ICMP error path
 * within Linux has all of the correct state should it need it.
 */
static int sfe_ipv4_recv_icmp(struct sfe_ipv4 *si, struct sk_buff *skb, struct net_device *dev,
			      unsigned int len, struct sfe_ipv4_ip_hdr *iph, unsigned int ihl)
{
	struct icmphdr *icmph;
	struct sfe_ipv4_ip_hdr *icmp_iph;
	unsigned int icmp_ihl_words;
	unsigned int icmp_ihl;
	u32 *icmp_trans_h;
	struct sfe_ipv4_udp_hdr *icmp_udph;
	struct sfe_ipv4_tcp_hdr *icmp_tcph;
	__be32 src_ip;
	__be32 dest_ip;
	__be16 src_port;
	__be16 dest_port;
	struct sfe_ipv4_connection_match *cm;
	struct sfe_ipv4_connection *c;
	u32 pull_len = sizeof(struct icmphdr) + ihl;

	/*
	 * Is our packet too short to contain a valid ICMP header?
	 */
	len -= ihl;
	if (!pskb_may_pull(skb, pull_len)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_HEADER_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("packet too short for ICMP header\n");
		return 0;
	}

	/*
	 * We only handle "destination unreachable" and "time exceeded" messages.
	 */
	icmph = (struct icmphdr *)(skb->data + ihl);
	if ((icmph->type != ICMP_DEST_UNREACH)
	    && (icmph->type != ICMP_TIME_EXCEEDED)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_UNHANDLED_TYPE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("unhandled ICMP type: 0x%x\n", icmph->type);
		return 0;
	}

	/*
	 * Do we have the full embedded IP header?
	 */
	len -= sizeof(struct icmphdr);
	pull_len += sizeof(struct sfe_ipv4_ip_hdr);
	if (!pskb_may_pull(skb, pull_len)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_HEADER_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("Embedded IP header not complete\n");
		return 0;
	}

	/*
	 * Is our embedded IP version wrong?
	 */
	icmp_iph = (struct sfe_ipv4_ip_hdr *)(icmph + 1);
	if (unlikely(icmp_iph->version != 4)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_NON_V4]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("IP version: %u\n", icmp_iph->version);
		return 0;
	}

	/*
	 * Do we have the full embedded IP header, including any options?
	 */
	icmp_ihl_words = icmp_iph->ihl;
	icmp_ihl = icmp_ihl_words << 2;
	pull_len += icmp_ihl - sizeof(struct sfe_ipv4_ip_hdr);
	if (!pskb_may_pull(skb, pull_len)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_IP_OPTIONS_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("Embedded header not large enough for IP options\n");
		return 0;
	}

	len -= icmp_ihl;
	icmp_trans_h = ((u32 *)icmp_iph) + icmp_ihl_words;

	/*
	 * Handle the embedded transport layer header.
	 */
	switch (icmp_iph->protocol) {
	case IPPROTO_UDP:
		/*
		 * We should have 8 bytes of UDP header - that's enough to identify
		 * the connection.
		 */
		pull_len += 8;
		if (!pskb_may_pull(skb, pull_len)) {
			spin_lock_bh(&si->lock);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UDP_HEADER_INCOMPLETE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("Incomplete embedded UDP header\n");
			return 0;
		}

		icmp_udph = (struct sfe_ipv4_udp_hdr *)icmp_trans_h;
		src_port = icmp_udph->source;
		dest_port = icmp_udph->dest;
		break;

	case IPPROTO_TCP:
		/*
		 * We should have 8 bytes of TCP header - that's enough to identify
		 * the connection.
		 */
		pull_len += 8;
		if (!pskb_may_pull(skb, pull_len)) {
			spin_lock_bh(&si->lock);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_TCP_HEADER_INCOMPLETE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("Incomplete embedded TCP header\n");
			return 0;
		}

		icmp_tcph = (struct sfe_ipv4_tcp_hdr *)icmp_trans_h;
		src_port = icmp_tcph->source;
		dest_port = icmp_tcph->dest;
		break;

	default:
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_IPV4_UNHANDLED_PROTOCOL]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("Unhandled embedded IP protocol: %u\n", icmp_iph->protocol);
		return 0;
	}

	src_ip = icmp_iph->saddr;
	dest_ip = icmp_iph->daddr;

	spin_lock_bh(&si->lock);

	/*
	 * Look for a connection match.  Note that we reverse the source and destination
	 * here because our embedded message contains a packet that was sent in the
	 * opposite direction to the one in which we just received it.  It will have
	 * been sent on the interface from which we received it though so that's still
	 * ok to use.
	 */
	cm = sfe_ipv4_find_sfe_ipv4_connection_match(si, dev, icmp_iph->protocol, dest_ip, dest_port, src_ip, src_port);
	if (unlikely(!cm)) {
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_NO_CONNECTION]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("no connection found\n");
		return 0;
	}

	/*
	 * We found a connection so now remove it from the connection list and flush
	 * its state.
	 */
	c = cm->connection;
	sfe_ipv4_remove_sfe_ipv4_connection(si, c);
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_ICMP_FLUSHED_CONNECTION]++;
	si->packets_not_forwarded++;
	spin_unlock_bh(&si->lock);

	sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_FLUSH);
	return 0;
}

/*
 * sfe_ipv4_recv()
 *	Handle packet receives and forwaring.
 *
 * Returns 1 if the packet is forwarded or 0 if it isn't.
 */
int sfe_ipv4_recv(struct net_device *dev, struct sk_buff *skb)
{
	struct sfe_ipv4 *si = &__si;
	unsigned int len;
	unsigned int tot_len;
	unsigned int frag_off;
	unsigned int ihl;
	bool flush_on_find;
	bool ip_options;
	struct sfe_ipv4_ip_hdr *iph;
	u32 protocol;

	/*
	 * Check that we have space for an IP header here.
	 */
	len = skb->len;
	if (unlikely(!pskb_may_pull(skb, sizeof(struct sfe_ipv4_ip_hdr)))) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_HEADER_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("len: %u is too short\n", len);
		return 0;
	}

	/*
	 * Check that our "total length" is large enough for an IP header.
	 */
	iph = (struct sfe_ipv4_ip_hdr *)skb->data;
	tot_len = ntohs(iph->tot_len);
	if (unlikely(tot_len < sizeof(struct sfe_ipv4_ip_hdr))) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_BAD_TOTAL_LENGTH]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("tot_len: %u is too short\n", tot_len);
		return 0;
	}

	/*
	 * Is our IP version wrong?
	 */
	if (unlikely(iph->version != 4)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_V4]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("IP version: %u\n", iph->version);
		return 0;
	}

	/*
	 * Does our datagram fit inside the skb?
	 */
	if (unlikely(tot_len > len)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_DATAGRAM_INCOMPLETE]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("tot_len: %u, exceeds len: %u\n", tot_len, len);
		return 0;
	}

	/*
	 * Do we have a non-initial fragment?
	 */
	frag_off = ntohs(iph->frag_off);
	if (unlikely(frag_off & IP_OFFSET)) {
		spin_lock_bh(&si->lock);
		si->exception_events[SFE_IPV4_EXCEPTION_EVENT_NON_INITIAL_FRAGMENT]++;
		si->packets_not_forwarded++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("non-initial fragment\n");
		return 0;
	}

	/*
	 * If we have a (first) fragment then mark it to cause any connection to flush.
	 */
	flush_on_find = unlikely(frag_off & IP_MF) ? true : false;

	/*
	 * Do we have any IP options?  That's definite a slow path!  If we do have IP
	 * options we need to recheck our header size.
	 */
	ihl = iph->ihl << 2;
	ip_options = unlikely(ihl != sizeof(struct sfe_ipv4_ip_hdr)) ? true : false;
	if (unlikely(ip_options)) {
		if (unlikely(len < ihl)) {
			spin_lock_bh(&si->lock);
			si->exception_events[SFE_IPV4_EXCEPTION_EVENT_IP_OPTIONS_INCOMPLETE]++;
			si->packets_not_forwarded++;
			spin_unlock_bh(&si->lock);

			DEBUG_TRACE("len: %u is too short for header of size: %u\n", len, ihl);
			return 0;
		}

		flush_on_find = true;
	}

	protocol = iph->protocol;
	if (IPPROTO_UDP == protocol) {
		return sfe_ipv4_recv_udp(si, skb, dev, len, iph, ihl, flush_on_find);
	}

	if (IPPROTO_TCP == protocol) {
		return sfe_ipv4_recv_tcp(si, skb, dev, len, iph, ihl, flush_on_find);
	}

	if (IPPROTO_ICMP == protocol) {
		return sfe_ipv4_recv_icmp(si, skb, dev, len, iph, ihl);
	}

	spin_lock_bh(&si->lock);
	si->exception_events[SFE_IPV4_EXCEPTION_EVENT_UNHANDLED_PROTOCOL]++;
	si->packets_not_forwarded++;
	spin_unlock_bh(&si->lock);

	DEBUG_TRACE("not UDP, TCP or ICMP: %u\n", protocol);
	return 0;
}

static void
sfe_ipv4_update_tcp_state(struct sfe_ipv4_connection *c,
			  struct sfe_connection_create *sic)
{
	struct sfe_ipv4_connection_match *orig_cm;
	struct sfe_ipv4_connection_match *repl_cm;
	struct sfe_ipv4_tcp_connection_match *orig_tcp;
	struct sfe_ipv4_tcp_connection_match *repl_tcp;

	orig_cm = c->original_match;
	repl_cm = c->reply_match;
	orig_tcp = &orig_cm->protocol_state.tcp;
	repl_tcp = &repl_cm->protocol_state.tcp;

	/* update orig */
	if (orig_tcp->max_win < sic->src_td_max_window) {
		orig_tcp->max_win = sic->src_td_max_window;
	}
	if ((s32)(orig_tcp->end - sic->src_td_end) < 0) {
		orig_tcp->end = sic->src_td_end;
	}
	if ((s32)(orig_tcp->max_end - sic->src_td_max_end) < 0) {
		orig_tcp->max_end = sic->src_td_max_end;
	}

	/* update reply */
	if (repl_tcp->max_win < sic->dest_td_max_window) {
		repl_tcp->max_win = sic->dest_td_max_window;
	}
	if ((s32)(repl_tcp->end - sic->dest_td_end) < 0) {
		repl_tcp->end = sic->dest_td_end;
	}
	if ((s32)(repl_tcp->max_end - sic->dest_td_max_end) < 0) {
		repl_tcp->max_end = sic->dest_td_max_end;
	}

	/* update match flags */
	orig_cm->flags &= ~SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	repl_cm->flags &= ~SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
		orig_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
		repl_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
	}
}

static void
sfe_ipv4_update_protocol_state(struct sfe_ipv4_connection *c,
			       struct sfe_connection_create *sic)
{
	switch (sic->protocol) {
	case IPPROTO_TCP:
		sfe_ipv4_update_tcp_state(c, sic);
		break;
	}
}

void sfe_ipv4_update_rule(struct sfe_connection_create *sic)
{
	struct sfe_ipv4_connection *c;
	struct sfe_ipv4 *si = &__si;

	spin_lock_bh(&si->lock);

	c = sfe_ipv4_find_sfe_ipv4_connection(si,
					      sic->protocol,
					      sic->src_ip.ip,
					      sic->src_port,
					      sic->dest_ip.ip,
					      sic->dest_port);
	if (c != NULL) {
		sfe_ipv4_update_protocol_state(c, sic);
	}

	spin_unlock_bh(&si->lock);
}

/*
 * sfe_ipv4_create_rule()
 *	Create a forwarding rule.
 */
int sfe_ipv4_create_rule(struct sfe_connection_create *sic)
{
	struct sfe_ipv4 *si = &__si;
	struct sfe_ipv4_connection *c;
	struct sfe_ipv4_connection_match *original_cm;
	struct sfe_ipv4_connection_match *reply_cm;
	struct net_device *dest_dev;
	struct net_device *src_dev;

	dest_dev = sic->dest_dev;
	src_dev = sic->src_dev;

	if (unlikely((dest_dev->reg_state != NETREG_REGISTERED) ||
		     (src_dev->reg_state != NETREG_REGISTERED))) {
		return -EINVAL;
	}

	spin_lock_bh(&si->lock);
	si->connection_create_requests++;

	/*
	 * Check to see if there is already a flow that matches the rule we're
	 * trying to create.  If there is then we can't create a new one.
	 */
	c = sfe_ipv4_find_sfe_ipv4_connection(si,
					      sic->protocol,
					      sic->src_ip.ip,
					      sic->src_port,
					      sic->dest_ip.ip,
					      sic->dest_port);
	if (c != NULL) {
		si->connection_create_collisions++;

		/*
		 * If we already have the flow then it's likely that this
		 * request to create the connection rule contains more
		 * up-to-date information. Check and update accordingly.
		 */
		sfe_ipv4_update_protocol_state(c, sic);
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("connection already exists - mark: %08x, p: %d\n"
			    "  s: %s:%pM:%pI4:%u, d: %s:%pM:%pI4:%u\n",
			    sic->mark, sic->protocol,
			    sic->src_dev->name, sic->src_mac, &sic->src_ip.ip, ntohs(sic->src_port),
			    sic->dest_dev->name, sic->dest_mac, &sic->dest_ip.ip, ntohs(sic->dest_port));
		return -EADDRINUSE;
	}

	/*
	 * Allocate the various connection tracking objects.
	 */
	c = (struct sfe_ipv4_connection *)kmalloc(sizeof(struct sfe_ipv4_connection), GFP_ATOMIC);
	if (unlikely(!c)) {
		spin_unlock_bh(&si->lock);
		return -ENOMEM;
	}

	original_cm = (struct sfe_ipv4_connection_match *)kmalloc(sizeof(struct sfe_ipv4_connection_match), GFP_ATOMIC);
	if (unlikely(!original_cm)) {
		spin_unlock_bh(&si->lock);
		kfree(c);
		return -ENOMEM;
	}

	reply_cm = (struct sfe_ipv4_connection_match *)kmalloc(sizeof(struct sfe_ipv4_connection_match), GFP_ATOMIC);
	if (unlikely(!reply_cm)) {
		spin_unlock_bh(&si->lock);
		kfree(original_cm);
		kfree(c);
		return -ENOMEM;
	}

	/*
	 * Fill in the "original" direction connection matching object.
	 * Note that the transmit MAC address is "dest_mac_xlate" because
	 * we always know both ends of a connection by their translated
	 * addresses and not their public addresses.
	 */
	original_cm->match_dev = src_dev;
	original_cm->match_protocol = sic->protocol;
	original_cm->match_src_ip = sic->src_ip.ip;
	original_cm->match_src_port = sic->src_port;
	original_cm->match_dest_ip = sic->dest_ip.ip;
	original_cm->match_dest_port = sic->dest_port;
	original_cm->xlate_src_ip = sic->src_ip_xlate.ip;
	original_cm->xlate_src_port = sic->src_port_xlate;
	original_cm->xlate_dest_ip = sic->dest_ip_xlate.ip;
	original_cm->xlate_dest_port = sic->dest_port_xlate;
	original_cm->rx_packet_count = 0;
	original_cm->rx_packet_count64 = 0;
	original_cm->rx_byte_count = 0;
	original_cm->rx_byte_count64 = 0;
	original_cm->xmit_dev = dest_dev;
	original_cm->xmit_dev_mtu = sic->dest_mtu;
	memcpy(original_cm->xmit_src_mac, dest_dev->dev_addr, ETH_ALEN);
	memcpy(original_cm->xmit_dest_mac, sic->dest_mac_xlate, ETH_ALEN);
	original_cm->connection = c;
	original_cm->counter_match = reply_cm;
	original_cm->flags = 0;
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
		original_cm->priority = sic->src_priority;
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
	}
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
		original_cm->dscp = sic->src_dscp << SFE_IPV4_DSCP_SHIFT;
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK;
	}
#ifdef CONFIG_NF_FLOW_COOKIE
	original_cm->flow_cookie = 0;
#endif
#ifdef CONFIG_XFRM
	original_cm->flow_accel = sic->original_accel;
#endif
	original_cm->active_next = NULL;
	original_cm->active_prev = NULL;
	original_cm->active = false;

	/*
	 * For PPP links we don't write an L2 header.  For everything else we do.
	 */
	if (!(dest_dev->flags & IFF_POINTOPOINT)) {
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;

		/*
		 * If our dev writes Ethernet headers then we can write a really fast
		 * version.
		 */
		if (dest_dev->header_ops) {
			if (dest_dev->header_ops->create == eth_header) {
				original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
			}
		}
	}

	/*
	 * Fill in the "reply" direction connection matching object.
	 */
	reply_cm->match_dev = dest_dev;
	reply_cm->match_protocol = sic->protocol;
	reply_cm->match_src_ip = sic->dest_ip_xlate.ip;
	reply_cm->match_src_port = sic->dest_port_xlate;
	reply_cm->match_dest_ip = sic->src_ip_xlate.ip;
	reply_cm->match_dest_port = sic->src_port_xlate;
	reply_cm->xlate_src_ip = sic->dest_ip.ip;
	reply_cm->xlate_src_port = sic->dest_port;
	reply_cm->xlate_dest_ip = sic->src_ip.ip;
	reply_cm->xlate_dest_port = sic->src_port;
	reply_cm->rx_packet_count = 0;
	reply_cm->rx_packet_count64 = 0;
	reply_cm->rx_byte_count = 0;
	reply_cm->rx_byte_count64 = 0;
	reply_cm->xmit_dev = src_dev;
	reply_cm->xmit_dev_mtu = sic->src_mtu;
	memcpy(reply_cm->xmit_src_mac, src_dev->dev_addr, ETH_ALEN);
	memcpy(reply_cm->xmit_dest_mac, sic->src_mac, ETH_ALEN);
	reply_cm->connection = c;
	reply_cm->counter_match = original_cm;
	reply_cm->flags = 0;
	if (sic->flags & SFE_CREATE_FLAG_REMARK_PRIORITY) {
		reply_cm->priority = sic->dest_priority;
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_PRIORITY_REMARK;
	}
	if (sic->flags & SFE_CREATE_FLAG_REMARK_DSCP) {
		reply_cm->dscp = sic->dest_dscp << SFE_IPV4_DSCP_SHIFT;
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_DSCP_REMARK;
	}
#ifdef CONFIG_NF_FLOW_COOKIE
	reply_cm->flow_cookie = 0;
#endif
#ifdef CONFIG_XFRM
	reply_cm->flow_accel = sic->reply_accel;
#endif
	reply_cm->active_next = NULL;
	reply_cm->active_prev = NULL;
	reply_cm->active = false;

	/*
	 * For PPP links we don't write an L2 header.  For everything else we do.
	 */
	if (!(src_dev->flags & IFF_POINTOPOINT)) {
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_L2_HDR;

		/*
		 * If our dev writes Ethernet headers then we can write a really fast
		 * version.
		 */
		if (src_dev->header_ops) {
			if (src_dev->header_ops->create == eth_header) {
				reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_WRITE_FAST_ETH_HDR;
			}
		}
	}


	if (sic->dest_ip.ip != sic->dest_ip_xlate.ip || sic->dest_port != sic->dest_port_xlate) {
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST;
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC;
	}

	if (sic->src_ip.ip != sic->src_ip_xlate.ip || sic->src_port != sic->src_port_xlate) {
		original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_SRC;
		reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_XLATE_DEST;
	}

	c->protocol = sic->protocol;
	c->src_ip = sic->src_ip.ip;
	c->src_ip_xlate = sic->src_ip_xlate.ip;
	c->src_port = sic->src_port;
	c->src_port_xlate = sic->src_port_xlate;
	c->original_dev = src_dev;
	c->original_match = original_cm;
	c->dest_ip = sic->dest_ip.ip;
	c->dest_ip_xlate = sic->dest_ip_xlate.ip;
	c->dest_port = sic->dest_port;
	c->dest_port_xlate = sic->dest_port_xlate;
	c->reply_dev = dest_dev;
	c->reply_match = reply_cm;
	c->mark = sic->mark;
	c->debug_read_seq = 0;
	c->last_sync_jiffies = get_jiffies_64();

	/*
	 * Take hold of our source and dest devices for the duration of the connection.
	 */
	dev_hold(c->original_dev);
	dev_hold(c->reply_dev);

	/*
	 * Initialize the protocol-specific information that we track.
	 */
	switch (sic->protocol) {
	case IPPROTO_TCP:
		original_cm->protocol_state.tcp.win_scale = sic->src_td_window_scale;
		original_cm->protocol_state.tcp.max_win = sic->src_td_max_window ? sic->src_td_max_window : 1;
		original_cm->protocol_state.tcp.end = sic->src_td_end;
		original_cm->protocol_state.tcp.max_end = sic->src_td_max_end;
		reply_cm->protocol_state.tcp.win_scale = sic->dest_td_window_scale;
		reply_cm->protocol_state.tcp.max_win = sic->dest_td_max_window ? sic->dest_td_max_window : 1;
		reply_cm->protocol_state.tcp.end = sic->dest_td_end;
		reply_cm->protocol_state.tcp.max_end = sic->dest_td_max_end;
		if (sic->flags & SFE_CREATE_FLAG_NO_SEQ_CHECK) {
			original_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
			reply_cm->flags |= SFE_IPV4_CONNECTION_MATCH_FLAG_NO_SEQ_CHECK;
		}
		break;
	}

	sfe_ipv4_connection_match_compute_translations(original_cm);
	sfe_ipv4_connection_match_compute_translations(reply_cm);
	sfe_ipv4_insert_sfe_ipv4_connection(si, c);

	spin_unlock_bh(&si->lock);

	/*
	 * We have everything we need!
	 */
	DEBUG_INFO("new connection - mark: %08x, p: %d\n"
		   "  s: %s:%pM(%pM):%pI4(%pI4):%u(%u)\n"
		   "  d: %s:%pM(%pM):%pI4(%pI4):%u(%u)\n",
		   sic->mark, sic->protocol,
		   sic->src_dev->name, sic->src_mac, sic->src_mac_xlate,
		   &sic->src_ip.ip, &sic->src_ip_xlate.ip, ntohs(sic->src_port), ntohs(sic->src_port_xlate),
		   dest_dev->name, sic->dest_mac, sic->dest_mac_xlate,
		   &sic->dest_ip.ip, &sic->dest_ip_xlate.ip, ntohs(sic->dest_port), ntohs(sic->dest_port_xlate));

	return 0;
}

/*
 * sfe_ipv4_destroy_rule()
 *	Destroy a forwarding rule.
 */
void sfe_ipv4_destroy_rule(struct sfe_connection_destroy *sid)
{
	struct sfe_ipv4 *si = &__si;
	struct sfe_ipv4_connection *c;

	spin_lock_bh(&si->lock);
	si->connection_destroy_requests++;

	/*
	 * Check to see if we have a flow that matches the rule we're trying
	 * to destroy.  If there isn't then we can't destroy it.
	 */
	c = sfe_ipv4_find_sfe_ipv4_connection(si, sid->protocol, sid->src_ip.ip, sid->src_port,
					      sid->dest_ip.ip, sid->dest_port);
	if (!c) {
		si->connection_destroy_misses++;
		spin_unlock_bh(&si->lock);

		DEBUG_TRACE("connection does not exist - p: %d, s: %pI4:%u, d: %pI4:%u\n",
			    sid->protocol, &sid->src_ip, ntohs(sid->src_port),
			    &sid->dest_ip, ntohs(sid->dest_port));
		return;
	}

	/*
	 * Remove our connection details from the hash tables.
	 */
	sfe_ipv4_remove_sfe_ipv4_connection(si, c);
	spin_unlock_bh(&si->lock);

	sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);

	DEBUG_INFO("connection destroyed - p: %d, s: %pI4:%u, d: %pI4:%u\n",
		   sid->protocol, &sid->src_ip.ip, ntohs(sid->src_port),
		   &sid->dest_ip.ip, ntohs(sid->dest_port));
}

/*
 * sfe_ipv4_register_sync_rule_callback()
 *	Register a callback for rule synchronization.
 */
void sfe_ipv4_register_sync_rule_callback(sfe_sync_rule_callback_t sync_rule_callback)
{
	struct sfe_ipv4 *si = &__si;

	spin_lock_bh(&si->lock);
	rcu_assign_pointer(si->sync_rule_callback, sync_rule_callback);
	spin_unlock_bh(&si->lock);
}

/*
 * sfe_ipv4_get_debug_dev()
 */
static ssize_t sfe_ipv4_get_debug_dev(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct sfe_ipv4 *si = &__si;
	ssize_t count;
	int num;

	spin_lock_bh(&si->lock);
	num = si->debug_dev;
	spin_unlock_bh(&si->lock);

	count = snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", num);
	return count;
}

/*
 * sysfs attributes.
 */
static const struct device_attribute sfe_ipv4_debug_dev_attr =
	__ATTR(debug_dev, S_IWUSR | S_IRUGO, sfe_ipv4_get_debug_dev, NULL);

/*
 * sfe_ipv4_destroy_all_rules_for_dev()
 *	Destroy all connections that match a particular device.
 *
 * If we pass dev as NULL then this destroys all connections.
 */
void sfe_ipv4_destroy_all_rules_for_dev(struct net_device *dev)
{
	struct sfe_ipv4 *si = &__si;
	struct sfe_ipv4_connection *c;

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
		sfe_ipv4_remove_sfe_ipv4_connection(si, c);
	}

	spin_unlock_bh(&si->lock);

	if (c) {
		sfe_ipv4_flush_sfe_ipv4_connection(si, c, SFE_SYNC_REASON_DESTROY);
		goto another_round;
	}
}

/*
 * sfe_ipv4_periodic_sync()
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
static void sfe_ipv4_periodic_sync(struct timer_list *arg)
#else
static void sfe_ipv4_periodic_sync(unsigned long arg)
#endif /*KERNEL_VERSION(4, 15, 0)*/
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	struct sfe_ipv4 *si = (struct sfe_ipv4 *)arg->cust_data;
#else
	struct sfe_ipv4 *si = (struct sfe_ipv4 *)arg;
#endif /*KERNEL_VERSION(4, 15, 0)*/
	u64 now_jiffies;
	int quota;
	sfe_sync_rule_callback_t sync_rule_callback;

	now_jiffies = get_jiffies_64();

	rcu_read_lock();
	sync_rule_callback = rcu_dereference(si->sync_rule_callback);
	if (!sync_rule_callback) {
		rcu_read_unlock();
		goto done;
	}

	spin_lock_bh(&si->lock);
	sfe_ipv4_update_summary_stats(si);

	/*
	 * Get an estimate of the number of connections to parse in this sync.
	 */
	quota = (si->num_connections + 63) / 64;

	/*
	 * Walk the "active" list and sync the connection state.
	 */
	while (quota--) {
		struct sfe_ipv4_connection_match *cm;
		struct sfe_ipv4_connection_match *counter_cm;
		struct sfe_ipv4_connection *c;
		struct sfe_connection_sync sis;

		cm = si->active_head;
		if (!cm) {
			break;
		}

		/*
		 * There's a possibility that our counter match is in the active list too.
		 * If it is then remove it.
		 */
		counter_cm = cm->counter_match;
		if (counter_cm->active) {
			counter_cm->active = false;

			/*
			 * We must have a connection preceding this counter match
			 * because that's the one that got us to this point, so we don't have
			 * to worry about removing the head of the list.
			 */
			counter_cm->active_prev->active_next = counter_cm->active_next;

			if (likely(counter_cm->active_next)) {
				counter_cm->active_next->active_prev = counter_cm->active_prev;
			} else {
				si->active_tail = counter_cm->active_prev;
			}

			counter_cm->active_next = NULL;
			counter_cm->active_prev = NULL;
		}

		/*
		 * Now remove the head of the active scan list.
		 */
		cm->active = false;
		si->active_head = cm->active_next;
		if (likely(cm->active_next)) {
			cm->active_next->active_prev = NULL;
		} else {
			si->active_tail = NULL;
		}
		cm->active_next = NULL;

		/*
		 * Sync the connection state.
		 */
		c = cm->connection;
		sfe_ipv4_gen_sync_sfe_ipv4_connection(si, c, &sis, SFE_SYNC_REASON_STATS, now_jiffies);

		/*
		 * We don't want to be holding the lock when we sync!
		 */
		spin_unlock_bh(&si->lock);
		sync_rule_callback(&sis);
		spin_lock_bh(&si->lock);
	}

	spin_unlock_bh(&si->lock);
	rcu_read_unlock();

done:
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));
}

#define CHAR_DEV_MSG_SIZE 768

/*
 * sfe_ipv4_debug_dev_read_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_start(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
					  int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
{
	int bytes_read;

	si->debug_read_seq++;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "<sfe_ipv4>\n");
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv4_debug_dev_read_connections_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_connections_start(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
						      int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
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
 * sfe_ipv4_debug_dev_read_connections_connection()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_connections_connection(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
							   int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
{
	struct sfe_ipv4_connection *c;
	struct sfe_ipv4_connection_match *original_cm;
	struct sfe_ipv4_connection_match *reply_cm;
	int bytes_read;
	int protocol;
	struct net_device *src_dev;
	__be32 src_ip;
	__be32 src_ip_xlate;
	__be16 src_port;
	__be16 src_port_xlate;
	u64 src_rx_packets;
	u64 src_rx_bytes;
	struct net_device *dest_dev;
	__be32 dest_ip;
	__be32 dest_ip_xlate;
	__be16 dest_port;
	__be16 dest_port_xlate;
	u64 dest_rx_packets;
	u64 dest_rx_bytes;
	u64 last_sync_jiffies;
	u32 mark, src_priority, dest_priority, src_dscp, dest_dscp;
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
	src_ip = c->src_ip;
	src_ip_xlate = c->src_ip_xlate;
	src_port = c->src_port;
	src_port_xlate = c->src_port_xlate;
	src_priority = original_cm->priority;
	src_dscp = original_cm->dscp >> SFE_IPV4_DSCP_SHIFT;

	sfe_ipv4_connection_match_update_summary_stats(original_cm);
	sfe_ipv4_connection_match_update_summary_stats(reply_cm);

	src_rx_packets = original_cm->rx_packet_count64;
	src_rx_bytes = original_cm->rx_byte_count64;
	dest_dev = c->reply_dev;
	dest_ip = c->dest_ip;
	dest_ip_xlate = c->dest_ip_xlate;
	dest_port = c->dest_port;
	dest_port_xlate = c->dest_port_xlate;
	dest_priority = reply_cm->priority;
	dest_dscp = reply_cm->dscp >> SFE_IPV4_DSCP_SHIFT;
	dest_rx_packets = reply_cm->rx_packet_count64;
	dest_rx_bytes = reply_cm->rx_byte_count64;
	last_sync_jiffies = get_jiffies_64() - c->last_sync_jiffies;
	mark = c->mark;
#ifdef CONFIG_NF_FLOW_COOKIE
	src_flow_cookie = original_cm->flow_cookie;
	dst_flow_cookie = reply_cm->flow_cookie;
#endif
	spin_unlock_bh(&si->lock);

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t\t<connection "
				"protocol=\"%u\" "
				"src_dev=\"%s\" "
				"src_ip=\"%pI4\" src_ip_xlate=\"%pI4\" "
				"src_port=\"%u\" src_port_xlate=\"%u\" "
				"src_priority=\"%u\" src_dscp=\"%u\" "
				"src_rx_pkts=\"%llu\" src_rx_bytes=\"%llu\" "
				"dest_dev=\"%s\" "
				"dest_ip=\"%pI4\" dest_ip_xlate=\"%pI4\" "
				"dest_port=\"%u\" dest_port_xlate=\"%u\" "
				"dest_priority=\"%u\" dest_dscp=\"%u\" "
				"dest_rx_pkts=\"%llu\" dest_rx_bytes=\"%llu\" "
#ifdef CONFIG_NF_FLOW_COOKIE
				"src_flow_cookie=\"%d\" dst_flow_cookie=\"%d\" "
#endif
				"last_sync=\"%llu\" "
				"mark=\"%08x\" />\n",
				protocol,
				src_dev->name,
				&src_ip, &src_ip_xlate,
				ntohs(src_port), ntohs(src_port_xlate),
				src_priority, src_dscp,
				src_rx_packets, src_rx_bytes,
				dest_dev->name,
				&dest_ip, &dest_ip_xlate,
				ntohs(dest_port), ntohs(dest_port_xlate),
				dest_priority, dest_dscp,
				dest_rx_packets, dest_rx_bytes,
#ifdef CONFIG_NF_FLOW_COOKIE
				src_flow_cookie, dst_flow_cookie,
#endif
				last_sync_jiffies, mark);

	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	return true;
}

/*
 * sfe_ipv4_debug_dev_read_connections_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_connections_end(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
						    int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
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
 * sfe_ipv4_debug_dev_read_exceptions_start()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_exceptions_start(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
						     int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
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
 * sfe_ipv4_debug_dev_read_exceptions_exception()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_exceptions_exception(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
							 int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
{
	u64 ct;

	spin_lock_bh(&si->lock);
	ct = si->exception_events64[ws->iter_exception];
	spin_unlock_bh(&si->lock);

	if (ct) {
		int bytes_read;

		bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE,
				      "\t\t<exception name=\"%s\" count=\"%llu\" />\n",
				      sfe_ipv4_exception_events_string[ws->iter_exception],
				      ct);
		if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
			return false;
		}

		*length -= bytes_read;
		*total_read += bytes_read;
	}

	ws->iter_exception++;
	if (ws->iter_exception >= SFE_IPV4_EXCEPTION_EVENT_LAST) {
		ws->iter_exception = 0;
		ws->state++;
	}

	return true;
}

/*
 * sfe_ipv4_debug_dev_read_exceptions_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_exceptions_end(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
						   int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
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
 * sfe_ipv4_debug_dev_read_stats()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_stats(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
					  int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
{
	int bytes_read;
	unsigned int num_connections;
	u64 packets_forwarded;
	u64 packets_not_forwarded;
	u64 connection_create_requests;
	u64 connection_create_collisions;
	u64 connection_destroy_requests;
	u64 connection_destroy_misses;
	u64 connection_flushes;
	u64 connection_match_hash_hits;
	u64 connection_match_hash_reorders;

	spin_lock_bh(&si->lock);
	sfe_ipv4_update_summary_stats(si);

	num_connections = si->num_connections;
	packets_forwarded = si->packets_forwarded64;
	packets_not_forwarded = si->packets_not_forwarded64;
	connection_create_requests = si->connection_create_requests64;
	connection_create_collisions = si->connection_create_collisions64;
	connection_destroy_requests = si->connection_destroy_requests64;
	connection_destroy_misses = si->connection_destroy_misses64;
	connection_flushes = si->connection_flushes64;
	connection_match_hash_hits = si->connection_match_hash_hits64;
	connection_match_hash_reorders = si->connection_match_hash_reorders64;
	spin_unlock_bh(&si->lock);

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "\t<stats "
			      "num_connections=\"%u\" "
			      "pkts_forwarded=\"%llu\" pkts_not_forwarded=\"%llu\" "
			      "create_requests=\"%llu\" create_collisions=\"%llu\" "
			      "destroy_requests=\"%llu\" destroy_misses=\"%llu\" "
			      "flushes=\"%llu\" "
			      "hash_hits=\"%llu\" hash_reorders=\"%llu\" />\n",
			      num_connections,
			      packets_forwarded,
			      packets_not_forwarded,
			      connection_create_requests,
			      connection_create_collisions,
			      connection_destroy_requests,
			      connection_destroy_misses,
			      connection_flushes,
			      connection_match_hash_hits,
			      connection_match_hash_reorders);
	if (copy_to_user(buffer + *total_read, msg, CHAR_DEV_MSG_SIZE)) {
		return false;
	}

	*length -= bytes_read;
	*total_read += bytes_read;

	ws->state++;
	return true;
}

/*
 * sfe_ipv4_debug_dev_read_end()
 *	Generate part of the XML output.
 */
static bool sfe_ipv4_debug_dev_read_end(struct sfe_ipv4 *si, char *buffer, char *msg, size_t *length,
					int *total_read, struct sfe_ipv4_debug_xml_write_state *ws)
{
	int bytes_read;

	bytes_read = snprintf(msg, CHAR_DEV_MSG_SIZE, "</sfe_ipv4>\n");
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
static sfe_ipv4_debug_xml_write_method_t sfe_ipv4_debug_xml_write_methods[SFE_IPV4_DEBUG_XML_STATE_DONE] = {
	sfe_ipv4_debug_dev_read_start,
	sfe_ipv4_debug_dev_read_connections_start,
	sfe_ipv4_debug_dev_read_connections_connection,
	sfe_ipv4_debug_dev_read_connections_end,
	sfe_ipv4_debug_dev_read_exceptions_start,
	sfe_ipv4_debug_dev_read_exceptions_exception,
	sfe_ipv4_debug_dev_read_exceptions_end,
	sfe_ipv4_debug_dev_read_stats,
	sfe_ipv4_debug_dev_read_end,
};

/*
 * sfe_ipv4_debug_dev_read()
 *	Send info to userspace upon read request from user
 */
static ssize_t sfe_ipv4_debug_dev_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
{
	char msg[CHAR_DEV_MSG_SIZE];
	int total_read = 0;
	struct sfe_ipv4_debug_xml_write_state *ws;
	struct sfe_ipv4 *si = &__si;

	ws = (struct sfe_ipv4_debug_xml_write_state *)filp->private_data;
	while ((ws->state != SFE_IPV4_DEBUG_XML_STATE_DONE) && (length > CHAR_DEV_MSG_SIZE)) {
		if ((sfe_ipv4_debug_xml_write_methods[ws->state])(si, buffer, msg, &length, &total_read, ws)) {
			continue;
		}
	}

	return total_read;
}

/*
 * sfe_ipv4_debug_dev_write()
 *	Write to char device resets some stats
 */
static ssize_t sfe_ipv4_debug_dev_write(struct file *filp, const char *buffer, size_t length, loff_t *offset)
{
	struct sfe_ipv4 *si = &__si;

	spin_lock_bh(&si->lock);
	sfe_ipv4_update_summary_stats(si);

	si->packets_forwarded64 = 0;
	si->packets_not_forwarded64 = 0;
	si->connection_create_requests64 = 0;
	si->connection_create_collisions64 = 0;
	si->connection_destroy_requests64 = 0;
	si->connection_destroy_misses64 = 0;
	si->connection_flushes64 = 0;
	si->connection_match_hash_hits64 = 0;
	si->connection_match_hash_reorders64 = 0;
	spin_unlock_bh(&si->lock);

	return length;
}

/*
 * sfe_ipv4_debug_dev_open()
 */
static int sfe_ipv4_debug_dev_open(struct inode *inode, struct file *file)
{
	struct sfe_ipv4_debug_xml_write_state *ws;

	ws = (struct sfe_ipv4_debug_xml_write_state *)file->private_data;
	if (!ws) {
		ws = kzalloc(sizeof(struct sfe_ipv4_debug_xml_write_state), GFP_KERNEL);
		if (!ws) {
			return -ENOMEM;
		}

		ws->state = SFE_IPV4_DEBUG_XML_STATE_START;
		file->private_data = ws;
	}

	return 0;
}

/*
 * sfe_ipv4_debug_dev_release()
 */
static int sfe_ipv4_debug_dev_release(struct inode *inode, struct file *file)
{
	struct sfe_ipv4_debug_xml_write_state *ws;

	ws = (struct sfe_ipv4_debug_xml_write_state *)file->private_data;
	if (ws) {
		/*
		 * We've finished with our output so free the write state.
		 */
		kfree(ws);
	}

	return 0;
}

/*
 * File operations used in the debug char device
 */
static struct file_operations sfe_ipv4_debug_dev_fops = {
	.read = sfe_ipv4_debug_dev_read,
	.write = sfe_ipv4_debug_dev_write,
	.open = sfe_ipv4_debug_dev_open,
	.release = sfe_ipv4_debug_dev_release
};

#ifdef CONFIG_NF_FLOW_COOKIE
/*
 * sfe_register_flow_cookie_cb
 *	register a function in SFE to let SFE use this function to configure flow cookie for a flow
 *
 * Hardware driver which support flow cookie should register a callback function in SFE. Then SFE
 * can use this function to configure flow cookie for a flow.
 * return: 0, success; !=0, fail
 */
int sfe_register_flow_cookie_cb(flow_cookie_set_func_t cb)
{
	struct sfe_ipv4 *si = &__si;

	BUG_ON(!cb);

	if (si->flow_cookie_set_func) {
		return -1;
	}

	rcu_assign_pointer(si->flow_cookie_set_func, cb);
	return 0;
}

/*
 * sfe_unregister_flow_cookie_cb
 *	unregister function which is used to configure flow cookie for a flow
 *
 * return: 0, success; !=0, fail
 */
int sfe_unregister_flow_cookie_cb(flow_cookie_set_func_t cb)
{
	struct sfe_ipv4 *si = &__si;

	RCU_INIT_POINTER(si->flow_cookie_set_func, NULL);
	return 0;
}

/*
 * sfe_ipv4_get_flow_cookie()
 */
static ssize_t sfe_ipv4_get_flow_cookie(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct sfe_ipv4 *si = &__si;
	return snprintf(buf, (ssize_t)PAGE_SIZE, "%d\n", si->flow_cookie_enable);
}

/*
 * sfe_ipv4_set_flow_cookie()
 */
static ssize_t sfe_ipv4_set_flow_cookie(struct device *dev,
					struct device_attribute *attr,
					const char *buf, size_t size)
{
	struct sfe_ipv4 *si = &__si;
	strict_strtol(buf, 0, (long int *)&si->flow_cookie_enable);

	return size;
}

/*
 * sysfs attributes.
 */
static const struct device_attribute sfe_ipv4_flow_cookie_attr =
	__ATTR(flow_cookie_enable, S_IWUSR | S_IRUGO, sfe_ipv4_get_flow_cookie, sfe_ipv4_set_flow_cookie);
#endif /*CONFIG_NF_FLOW_COOKIE*/

/*
 * sfe_ipv4_init()
 */
static int __init sfe_ipv4_init(void)
{
	struct sfe_ipv4 *si = &__si;
	int result = -1;

	DEBUG_INFO("SFE IPv4 init\n");

	/*
	 * Create sys/sfe_ipv4
	 */
	si->sys_sfe_ipv4 = kobject_create_and_add("sfe_ipv4", NULL);
	if (!si->sys_sfe_ipv4) {
		DEBUG_ERROR("failed to register sfe_ipv4\n");
		goto exit1;
	}

	/*
	 * Create files, one for each parameter supported by this module.
	 */
	result = sysfs_create_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);
	if (result) {
		DEBUG_ERROR("failed to register debug dev file: %d\n", result);
		goto exit2;
	}

#ifdef CONFIG_NF_FLOW_COOKIE
	result = sysfs_create_file(si->sys_sfe_ipv4, &sfe_ipv4_flow_cookie_attr.attr);
	if (result) {
		DEBUG_ERROR("failed to register flow cookie enable file: %d\n", result);
		goto exit3;
	}
#endif /* CONFIG_NF_FLOW_COOKIE */

	/*
	 * Register our debug char device.
	 */
	result = register_chrdev(0, "sfe_ipv4", &sfe_ipv4_debug_dev_fops);
	if (result < 0) {
		DEBUG_ERROR("Failed to register chrdev: %d\n", result);
		goto exit4;
	}

	si->debug_dev = result;

	/*
	 * Create a timer to handle periodic statistics.
	 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 15, 0))
	timer_setup(&si->timer, sfe_ipv4_periodic_sync, 0);
	si->timer.cust_data = (unsigned long)si;
#else
	setup_timer(&si->timer, sfe_ipv4_periodic_sync, (unsigned long)si);
#endif /*KERNEL_VERSION(4, 15, 0)*/
	mod_timer(&si->timer, jiffies + ((HZ + 99) / 100));

	spin_lock_init(&si->lock);

	return 0;

exit4:
#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_flow_cookie_attr.attr);

exit3:
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);

exit2:
	kobject_put(si->sys_sfe_ipv4);

exit1:
	return result;
}

/*
 * sfe_ipv4_exit()
 */
static void __exit sfe_ipv4_exit(void)
{
	struct sfe_ipv4 *si = &__si;

	DEBUG_INFO("SFE IPv4 exit\n");

	/*
	 * Destroy all connections.
	 */
	sfe_ipv4_destroy_all_rules_for_dev(NULL);

	del_timer_sync(&si->timer);

	unregister_chrdev(si->debug_dev, "sfe_ipv4");

#ifdef CONFIG_NF_FLOW_COOKIE
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_flow_cookie_attr.attr);
#endif /* CONFIG_NF_FLOW_COOKIE */
	sysfs_remove_file(si->sys_sfe_ipv4, &sfe_ipv4_debug_dev_attr.attr);

	kobject_put(si->sys_sfe_ipv4);

}

module_init(sfe_ipv4_init)
module_exit(sfe_ipv4_exit)

EXPORT_SYMBOL(sfe_ipv4_recv);
EXPORT_SYMBOL(sfe_ipv4_create_rule);
EXPORT_SYMBOL(sfe_ipv4_destroy_rule);
EXPORT_SYMBOL(sfe_ipv4_destroy_all_rules_for_dev);
EXPORT_SYMBOL(sfe_ipv4_register_sync_rule_callback);
EXPORT_SYMBOL(sfe_ipv4_mark_rule);
EXPORT_SYMBOL(sfe_ipv4_update_rule);
#ifdef CONFIG_NF_FLOW_COOKIE
EXPORT_SYMBOL(sfe_register_flow_cookie_cb);
EXPORT_SYMBOL(sfe_unregister_flow_cookie_cb);
#endif

MODULE_DESCRIPTION("Shortcut Forwarding Engine - IPv4 edition");
MODULE_LICENSE("Dual BSD/GPL");

