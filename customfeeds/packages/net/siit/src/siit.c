/*
 * siit.c: the Stateless IP/ICMP Translator (SIIT) module for Linux.
 *
 *
 */

#include <linux/version.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>       /* printk() */
#include <linux/slab.h>

#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/interrupt.h>    /* mark_bh */
#include <linux/random.h>
#include <linux/in.h>
#include <linux/netdevice.h>    /* struct device, and other headers */
#include <linux/etherdevice.h>  /* eth_type_trans */
#include <net/ip.h>             /* struct iphdr */
#include <net/icmp.h>           /* struct icmphdr */
#include <net/ipv6.h>
#include <net/udp.h>
#include <linux/skbuff.h>
#include <linux/in6.h>
#include <linux/init.h>
#include <asm/uaccess.h>
#include <asm/checksum.h>
#include <net/ip6_checksum.h>
#include <linux/in6.h>
#include "siit.h"

MODULE_AUTHOR("Dmitriy Moscalev, Grigory Klyuchnikov, Felix Fietkau");

/*
 * If tos_ignore_flag != 0, we don't copy TOS and Traffic Class
 * from origin paket and set it to 0
 */
int tos_ignore_flag = 0;

#define siit_stats(_dev) (&(_dev)->stats)

/*
 * The Utility  stuff
 */

#ifdef SIIT_DEBUG
/* print dump bytes (data point data area sizeof len and message
 * before dump.
 */
static int siit_print_dump(char *data, int len, char *message)
{
	int i;
	int j = 0, k = 1;

	len = len > BUFF_SIZE ? BUFF_SIZE : len;
	printk("%s:\n", message);
	for (i=0; i < len; i++, k++) {
		if( i == len-1 || k == 16) {
			printk("%02x\n", (~(~0 << 8) & *(data+i)));
			j = 0;
			k = 0;
		}
		else if (j) {
			printk("%02x ", (~(~0 << 8) & *(data+i)));
			j--;
		}
		else {
			printk("%02x", (~(~0 << 8) & *(data+i)));
			j++;
		}
	}
	return 0;
}
#endif

/*
 * Open and close
 */
static int siit_open(struct net_device *dev)
{
	netif_start_queue(dev);
	return 0;
}


static int siit_release(struct net_device *dev)
{
	netif_stop_queue(dev); /* can't transmit any more */
	return 0;
}

/*
 * Translation IPv4 to IPv6 stuff
 *
 * ip4_ip6 (src, len, dst, include_flag)
 *
 * where
 * src - buffer with original IPv4 packet,
 * len - size of original packet,
 * dst - new buffer for IPv6 packet,
 * include_flag - if = 1, dst point to IPv4 packet that is ICMP error
 *                included IP packet, else = 0
 */

static int ip4_ip6(char *src, int len, char *dst, int include_flag)
{
	struct iphdr *ih4 = (struct iphdr *) src; /* point to current IPv4 header struct */
	struct icmphdr *icmp_hdr;   /* point to current ICMPv4 header struct */
	struct udphdr *udp_hdr;     /* point to current IPv4 UDP header struct */

	struct ipv6hdr *ih6 = (struct ipv6hdr *) dst; /* point to current IPv6 header struct */
	struct frag_hdr *ih6_frag = (struct frag_hdr *)(dst+sizeof(struct ipv6hdr));
										      /* point to current IPv6 fragment header struct */
	struct icmp6hdr *icmp6_hdr; /* point to current ICMPv6 header */

	int hdr_len = (int)(ih4->ihl * 4); /* IPv4 header length */
	int icmp_len;               /* ICMPv4 packet length */
	int plen;                   /* payload length */

	unsigned int csum;          /* need to calculate ICMPv6 and UDP checksum */
	int fl_csum = 0;            /* flag to calculate UDP checksum */
	int icmperr = 1;            /* flag to indicate ICMP error message and to need
								   translate ICMP included IP packet */
	int fr_flag = 0;            /* fragment flag, if = 0 - don't add
								   fragment header */
	__u16 new_tot_len;          /* need to calculate IPv6 total length */
	__u8 new_nexthdr;           /* next header code */
	__u16 icmp_ptr = 0;         /* Pointer field in ICMP_PARAMETERPROB */

#ifdef SIIT_DEBUG              /* print IPv4 header dump */
	siit_print_dump(src, hdr_len, "siit: ip4_ip6() (in) ip4 header dump");
#endif

	/* If DF == 1 && MF == 0 && Fragment Offset == 0
	 * or this packet is ICMP included IP packet
	 * we don't need fragment header */
	if (ntohs(ih4->frag_off) == IP_DF || include_flag ) {
		/* not fragment and we need not to add Fragment
		 * Header to IPv6 packet. */
		/* total length = total length from IPv4 packet */
		new_tot_len = ntohs(ih4->tot_len);

		if (ih4->protocol == IPPROTO_ICMP)
			new_nexthdr = NEXTHDR_ICMP;
		else
			new_nexthdr = ih4->protocol;
	}
	else {
		/* need to add Fragment Header */
		fr_flag = 1;
		/* total length = total length from IPv4 packet +
		   length of Fragment Header */
		new_tot_len = ntohs(ih4->tot_len) + sizeof(struct frag_hdr);
		/* IPv6 Header NextHeader = NEXTHDR_FRAGMENT */
		new_nexthdr = NEXTHDR_FRAGMENT;
		/* Fragment Header NextHeader copy from IPv4 packet */
		if (ih4->protocol == IPPROTO_ICMP)
			ih6_frag->nexthdr = NEXTHDR_ICMP;
		else
			ih6_frag->nexthdr = ih4->protocol;

		/* copy frag offset from IPv4 packet */
		ih6_frag->frag_off = htons((ntohs(ih4->frag_off) & IP_OFFSET) << 3);
		/* copy MF flag from IPv4 packet */
		ih6_frag->frag_off = htons((ntohs(ih6_frag->frag_off) |
									((ntohs(ih4->frag_off) & IP_MF) >> 13)));
		/* copy Identification field from IPv4 packet */
		ih6_frag->identification = htonl(ntohs(ih4->id));
		/* reserved field initialized to zero */
		ih6_frag->reserved = 0;
	}

	/* Form rest IPv6 fields */

	/*
	 * At this point we need to add checking of unxpired source
	 * route optin and if it is, send ICMPv4 "destination
	 * unreacheble/source route failes" Type 3/Code 5 and
	 * drop packet. (NOT RELEASED YET)
	 */

	/* IP version = 6 */
	ih6->version = 6;

	if (tos_ignore_flag) {
		ih6->priority = 0;
		ih6->flow_lbl[0] = 0;
	} else {
		ih6->priority = (ih4->tos & 0xf0) >> 4;
		ih6->flow_lbl[0] = (ih4->tos & 0x0f) << 4;
	}
	ih6->flow_lbl[1] = 0;
	ih6->flow_lbl[2] = 0;

	/* Hop Limit = IPv4 TTL */
	ih6->hop_limit = ih4->ttl;

	/* Translate source destination addresses,
	   for IPv6 host it's IPv4-translated IPv6 address,
	   for IPv4 host it's IPv4-mapped IPv6 address

	   !!WARNING!! Instead IPv4-mapped IPv6 addresses we use addreesses
	   with unused prefix ::ffff:ffff:0:0/96, because KAME implementation
	   doesn't support IPv4-mapped addresses in IPv6 packets and discard them.

	*/

	if (include_flag) {
		/*
		   It's ICMP included IP packet and there is a diffirence
		   in src/dst addresses then src/dst in normal direction
		 */

		/*
		   Source address
		   is IPv4-translated IPv6 address because packet traveled
		   from IPv6 to IPv4 area
		*/
		ih6->saddr.in6_u.u6_addr32[0] = 0;
		ih6->saddr.in6_u.u6_addr32[1] = 0;
		ih6->saddr.in6_u.u6_addr32[2] = htonl(TRANSLATED_PREFIX); /* to network order bytes */
		ih6->saddr.in6_u.u6_addr32[3] = ih4->saddr;

		/*
		   Destination address
		   is IPv4-mapped address (but it's not IPv4- mapped, we use
		   prefix ::ffff:ffff:0:0/96
		 */
		ih6->daddr.in6_u.u6_addr32[0] = 0;
		ih6->daddr.in6_u.u6_addr32[1] = 0;
		ih6->daddr.in6_u.u6_addr32[2] = htonl(MAPPED_PREFIX); /* to network order bytes */
		ih6->daddr.in6_u.u6_addr32[3] = ih4->daddr;
	}
	else {

		/*
		   This is normal case (packet isn't included IP packet)

		   Source address
		   is IPv4-mapped address (but it's not IPv4- mapped, we use
		   prefix ::ffff:ffff:0:0/96)
		*/
		ih6->saddr.in6_u.u6_addr32[0] = 0;
		ih6->saddr.in6_u.u6_addr32[1] = 0;
		ih6->saddr.in6_u.u6_addr32[2] = htonl(MAPPED_PREFIX); /* to network order bytes */
		ih6->saddr.in6_u.u6_addr32[3] = ih4->saddr;

		/* Destination address
		   is is IPv4-translated IPv6 address
		 */
		ih6->daddr.in6_u.u6_addr32[0] = 0;
		ih6->daddr.in6_u.u6_addr32[1] = 0;
		ih6->daddr.in6_u.u6_addr32[2] = htonl(TRANSLATED_PREFIX); /* to network order bytes */
		ih6->daddr.in6_u.u6_addr32[3] = ih4->daddr;
	}

	/* Payload Length */
	plen = new_tot_len - hdr_len; /* Payload length = IPv4 total len - IPv4 header len */
	ih6->payload_len = htons(plen);

	/* Next Header */
	ih6->nexthdr = new_nexthdr; /* Next Header */

	/* Process ICMP protocols data */

	switch (ih4->protocol) {
	case IPPROTO_ICMP:
		if ( (ntohs(ih4->frag_off) & IP_OFFSET) != 0 || (ntohs(ih4->frag_off) & IP_MF) != 0 ) {
			PDEBUG("ip4_ip6(): don't translate ICMPv4 fragments - packet dropped.\n");
			return -1;
		}

		icmp_hdr = (struct icmphdr *) (src+hdr_len); /* point to ICMPv4 header */
		csum = 0;
		icmp_len =  ntohs(ih4->tot_len) - hdr_len; /* ICMPv4 packet length */
		icmp6_hdr = (struct icmp6hdr *)(dst+sizeof(struct ipv6hdr)
									    +fr_flag*sizeof(struct frag_hdr)); /* point to ICMPv6 header */

		if (include_flag) {
			/* ICMPv4 packet cannot be included in ICMPv4 Error message */
			/* !!! May be it's WRONG !!! ICMPv4 QUERY packet can be included
			   in ICMPv4 Error message */
			PDEBUG("ip4_ip6(): It's included ICMPv4 in ICMPv4 Error message - packet dropped.\n");
			return -1;
		}

		/* Check ICMPv4 Type field */
		switch (icmp_hdr->type) {
		/* ICMP Error messages */
		/* Destination Unreachable (Type 3) */
		case ICMP_DEST_UNREACH:
			icmp6_hdr->icmp6_type = ICMPV6_DEST_UNREACH; /* to Type 1 */
			icmp6_hdr->icmp6_unused = 0;
			switch (icmp_hdr->code)
			{
			case ICMP_NET_UNREACH: /* Code 0 */
			case ICMP_HOST_UNREACH: /* Code 1 */
			case ICMP_SR_FAILED: /* Code 5 */
			case ICMP_NET_UNKNOWN: /* Code 6 */
			case ICMP_HOST_UNKNOWN: /* Code 7 */
			case ICMP_HOST_ISOLATED: /* Code 8 */
			case ICMP_NET_UNR_TOS: /* Code 11 */
			case ICMP_HOST_UNR_TOS: /* Code 12 */
				icmp6_hdr->icmp6_code = ICMPV6_NOROUTE; /* to Code 0 */
				break;
			case ICMP_PROT_UNREACH: /* Code 2 */
				icmp6_hdr->icmp6_type = ICMPV6_PARAMPROB; /* to Type 4 */
				icmp6_hdr->icmp6_code = ICMPV6_UNK_NEXTHDR; /* to Code 1 */
				/* Set pointer filed to 6, it's octet offset IPv6 Next Header field */
				icmp6_hdr->icmp6_pointer = htonl(6);
				break;
			case ICMP_PORT_UNREACH: /* Code 3 */
				icmp6_hdr->icmp6_code = ICMPV6_PORT_UNREACH; /* to Code 4 */
				break;
			case ICMP_FRAG_NEEDED: /* Code 4 */
				icmp6_hdr->icmp6_type = ICMPV6_PKT_TOOBIG; /* to Type 2 */
				icmp6_hdr->icmp6_code = 0;
				/* Correct MTU  */
				if (icmp_hdr->un.frag.mtu == 0)
					/* we use minimum MTU for IPv4 PMTUv4 RFC1191, section 5;
					   IPv6 implementation wouldn't accept Path MTU < 1280,
					   but it records info correctly to always include
					   a fragment header */
					icmp6_hdr->icmp6_mtu = htonl(576);
				else
					/* needs to adjusted for difference between IPv4/IPv6 headers
					 * SIIT RFC2765, section 3.3,
					 * we assume that difference is 20 bytes */
					icmp6_hdr->icmp6_mtu = htonl(ntohs(icmp_hdr->un.frag.mtu)+IP4_IP6_HDR_DIFF);

				break;
			case ICMP_NET_ANO: /* Code 9 */
			case ICMP_HOST_ANO: /* Code 10 */
				icmp6_hdr->icmp6_code = ICMPV6_ADM_PROHIBITED; /* to Code 1 */
				break;
			default: /* discard any other Code */
				PDEBUG("ip4_ip6(): Unknown ICMPv4 Type %d Code %d - packet dropped.\n",
					   ICMP_DEST_UNREACH, icmp_hdr->code);
				return -1;
			}
			break;
			/* Time Exceeded (Type 11) */
		case ICMP_TIME_EXCEEDED:
			icmp6_hdr->icmp6_type = ICMPV6_TIME_EXCEED;
			icmp6_hdr->icmp6_code = icmp_hdr->code;
			break;
			/* Parameter Problem (Type 12) */
		case ICMP_PARAMETERPROB:
			icmp6_hdr->icmp6_type = ICMPV6_PARAMPROB;
			icmp6_hdr->icmp6_code = icmp_hdr->code;

			icmp_ptr = ntohs(icmp_hdr->un.echo.id) >> 8;
			switch (icmp_ptr) {
			case 0:
				icmp6_hdr->icmp6_pointer = 0; /* IPv4 Version -> IPv6 Version */
				break;
			case 2:
				icmp6_hdr->icmp6_pointer = __constant_htonl(4); /* IPv4 length -> IPv6 Payload Length */
				break;
			case 8:
				icmp6_hdr->icmp6_pointer = __constant_htonl(7); /* IPv4 TTL -> IPv6 Hop Limit */
				break;
			case 9:
				icmp6_hdr->icmp6_pointer = __constant_htonl(6); /* IPv4 Protocol -> IPv6 Next Header */
				break;
			case 12:
				icmp6_hdr->icmp6_pointer = __constant_htonl(8); /* IPv4 Src Addr -> IPv6 Src Addr */
				break;
			case 16:
				icmp6_hdr->icmp6_pointer = __constant_htonl(24); /* IPv4 Dst Addr -> IPv6 Dst Addr */
				break;
			default:
				icmp6_hdr->icmp6_pointer = 0xffffffff; /* set to all ones in any other cases */
				break;
			}
			break;
		case ICMP_ECHO:
			icmperr = 0;
			icmp6_hdr->icmp6_type = ICMPV6_ECHO_REQUEST;
			icmp6_hdr->icmp6_code = 0;
			/* Copy rest ICMP data to new IPv6 packet without changing */
			memcpy(((char *)icmp6_hdr)+4, ((char *)icmp_hdr)+4, len - hdr_len - 4);
			break;

		case ICMP_ECHOREPLY:
			icmperr = 0;
			icmp6_hdr->icmp6_type = ICMPV6_ECHO_REPLY;
			icmp6_hdr->icmp6_code = 0;
			/* Copy rest ICMP data to new IPv6 packet without changing */
			memcpy(((char *)icmp6_hdr)+4, ((char *)icmp_hdr)+4, len - hdr_len - 4);
			break;

			/* Discard any other ICMP messages */
		default:
			PDEBUG("ip4_ip6(): Unknown ICMPv4 packet Type %x - packet dropped.\n", icmp_hdr->type);
			return -1;
		}

		/* Now if it's ICMPv4 Error message we must translate included IP packet */

		if (icmperr) {
			/* Call our ip4_ip6() to translate included IP packet */
			if (ip4_ip6(src+hdr_len+sizeof(struct icmphdr), len - hdr_len - sizeof(struct icmphdr),
						dst+sizeof(struct ipv6hdr)+fr_flag*sizeof(struct frag_hdr)
						+sizeof(struct icmp6hdr), 1) == -1) {
				PDEBUG("ip4_ip6(): Uncorrect translation of ICMPv4 Error message - packet dropped.\n");
				return -1;
			}
			/* correct ICMPv6 packet length for diffirence between IPv4 and IPv6 headers
			   in included IP packet
			   */
			icmp_len += 20;
			/* and correct Payload length for diffirence between IPv4 and IPv6 headers */
			plen += 20;
			ih6->payload_len = htons(plen);
		}

		/* Calculate ICMPv6 checksum */

		icmp6_hdr->icmp6_cksum = 0;
		csum = 0;

		csum = csum_partial((u_char *)icmp6_hdr, icmp_len, csum);
		icmp6_hdr->icmp6_cksum = csum_ipv6_magic(&ih6->saddr, &ih6->daddr, icmp_len,
									         IPPROTO_ICMPV6, csum);
		break;

	/* Process TCP protocols data */
	case IPPROTO_TCP:
		/* Copy TCP data to new IPv6 packet without changing */
		memcpy(dst+sizeof(struct ipv6hdr)+fr_flag*sizeof(struct frag_hdr),
				   src+hdr_len, len - hdr_len);
		break;

	/* Process UDP protocols data */
	case IPPROTO_UDP:
		udp_hdr = (struct udphdr *)(src+hdr_len);
		if ((ntohs(ih4->frag_off) & IP_OFFSET) == 0) {
			if ((ntohs(ih4->frag_off) & IP_MF) != 0) {
				/* It's a first fragment */
				if (udp_hdr->check == 0) {
					/* System management event */
					printk("siit: First fragment of UDP with zero checksum - packet droped\n");
					printk("siit: addr: %x src port: %d dst port: %d\n",
						   htonl(ih4->saddr), htons(udp_hdr->source), htons(udp_hdr->dest));
					return -1;
				}
			}
			else if (udp_hdr->check == 0)
				fl_csum = 1;
		}

		/* Copy UDP data to new IPv6 packet */
		udp_hdr = (struct udphdr *)(dst+sizeof(struct ipv6hdr)
									+ fr_flag*sizeof(struct frag_hdr));
		memcpy((char *)udp_hdr, src+hdr_len, len - hdr_len);

		/* Calculate UDP checksum if UDP checksum in IPv4 packet was ZERO
		   and if it isn't included IP packet
		 */
		if (fl_csum && (!include_flag)) {
			udp_hdr->check = 0;
			csum = 0;
			csum = csum_partial((unsigned char *)udp_hdr, plen - fr_flag*sizeof(struct frag_hdr), csum);
			udp_hdr->check = csum_ipv6_magic(&ih6->saddr, &ih6->daddr, plen -
										     fr_flag*sizeof(struct frag_hdr), IPPROTO_UDP, csum);
		}
		break;

	/* Discard packets with any other protocol */
	default:
		PDEBUG("ip4_ip6(): Unknown upper protocol - packet dropped.\n");
		return -1;
	}

#ifdef SIIT_DEBUG
	siit_print_dump(dst, sizeof(struct ipv6hdr), "siit: ip4_ip6(): (out) ipv6 header dump");
#endif

	return 0;
}

/*
 * Translation IPv6 to IPv4 stuff
 *
 * ip6_ip4(src, len, dst, include_flag)
 *
 * where
 * src - buffer with original IPv6 packet,
 * len - size of original packet,
 * dst - new buffer for IPv4 packet,
 * include_flag - if = 1, dst point to IPv6 packet that is ICMP error
 *                included IP packet, else = 0
 *
 */

static int ip6_ip4(char *src, int len, char *dst, int include_flag)
{
	struct ipv6hdr *ip6_hdr;    /* point to current IPv6 header struct */
	struct iphdr *ip_hdr;       /* point to current IPv4 header struct */
	int opts_len = 0;           /* to sum Option Headers length */
	int icmperr = 1;            /* if = 1, indicate that packet is ICMP Error message, else = 0 */
	int ntot_len = 0;           /* to calculate IPv6 Total Length field */
	int real_len;
	int len_delta;
	int ip6_payload_len;
	int inc_opts_len = 0;       /* to sum Option Headers length in ICMP included IP packet */
	__u8 next_hdr;              /* Next Header */

#ifdef SIIT_DEBUG
	siit_print_dump(src, sizeof(struct ipv6hdr), "siit: ip6_ip4(): (in) ipv6 header dump");
#endif

	if ( (len_delta = len - sizeof(struct ipv6hdr)) >= 0)
	{
		ip6_hdr = (struct ipv6hdr *)src;
		ip_hdr = (struct iphdr *)dst;

		real_len = sizeof(struct iphdr);

		/* Check validation of Saddr & Daddr? is a packet to fall under our translation? */
		if (include_flag) { /* It's ICMP included IP packet,
							   about process include_flag see comment in ip4_ip6() */
			if (ip6_hdr->saddr.s6_addr32[2] != htonl(MAPPED_PREFIX)) {
				PDEBUG("ip6_ip4(): Included IP packet Src addr isn't mapped addr: %x%x%x%x, packet dropped.\n",
					   ip6_hdr->saddr.s6_addr32[0], ip6_hdr->saddr.s6_addr32[1],
					   ip6_hdr->saddr.s6_addr32[2], ip6_hdr->saddr.s6_addr32[3]);
				return -1;
			}
			if ( ip6_hdr->daddr.s6_addr32[2] != htonl(TRANSLATED_PREFIX)) {
				PDEBUG("ip6_ip4(): Included IP packet Dst addr isn't translated addr: %x%x%x%x, packet dropped.\n",
					   ip6_hdr->daddr.s6_addr32[0], ip6_hdr->daddr.s6_addr32[1],
					   ip6_hdr->daddr.s6_addr32[2], ip6_hdr->daddr.s6_addr32[3]);
				return -1;
			}
		}
		else { /* It's normal IP packet (not included in ICMP) */
			if (ip6_hdr->saddr.s6_addr32[2] != htonl(TRANSLATED_PREFIX)) {
				PDEBUG("ip6_ip4(): Src addr isn't translated addr: %x%x%x%x, packet dropped.\n",
					   ip6_hdr->saddr.s6_addr32[0], ip6_hdr->saddr.s6_addr32[1],
					   ip6_hdr->saddr.s6_addr32[2], ip6_hdr->saddr.s6_addr32[3]);
				return -1;
			}
			if ( ip6_hdr->daddr.s6_addr32[2] != htonl(MAPPED_PREFIX)) {
				PDEBUG("ip6_ip4(): Dst addr isn't mapped addr: %x%x%x%x, packet dropped.\n",
					   ip6_hdr->daddr.s6_addr32[0], ip6_hdr->daddr.s6_addr32[1],
					   ip6_hdr->daddr.s6_addr32[2], ip6_hdr->daddr.s6_addr32[3]);
				return -1;
			}
		}

		/* Set IPv4 Fragment Offset and ID to 0
		   before process any Option Headers */
		ip_hdr->frag_off = 0;
		ip_hdr->id = 0;

		/*
		 * We process only Fragment Header. Any other options headers
		 * are ignored, i.e. there is no attempt to translate them.
		 * However, the Total Length field and the Protocol field would
		 * have to be adjusted to "skip" these extension headers.
		 */

		next_hdr = ip6_hdr->nexthdr;

		/* Hop_by_Hop options header (ip6_hdr->nexthdr = 0). It must
		 * appear only in IPv6 header's Next Header field.
		 */
		if (next_hdr == NEXTHDR_HOP) {
			if ( (len_delta - sizeof(struct ipv6_opt_hdr)) >= 0)
			{
				struct ipv6_opt_hdr *ip6h =
					(struct ipv6_opt_hdr *)(src+sizeof(struct ipv6hdr) + opts_len);
				if ( (len_delta -= ip6h->hdrlen*8 + 8) >= 0)
				{
					opts_len += ip6h->hdrlen*8 + 8;  /* See  RFC 2460 page 11:
							Hdr Ext Len  8-bit unsigned integer.  Length of the Hop-by-
										 Hop Options header in 8-octet units, not
										 including the first 8 octets.
							*/
					next_hdr = ip6h->nexthdr;
				}
				else
				{
					PDEBUG("ip6_ip4(): hop_by_hop header error, packet droped");
					/* Generate ICMP Parameter Problem */
					return -1;
				}
			}
		}

		if (len_delta > 0)
		{
			while(next_hdr != NEXTHDR_ICMP && next_hdr != NEXTHDR_TCP
				  && next_hdr != NEXTHDR_UDP)
			{
				/* Destination options header */
				if (next_hdr == NEXTHDR_DEST)
				{
					if ( (len_delta - sizeof(struct ipv6_opt_hdr)) >= 0)
					{
						struct ipv6_opt_hdr *ip6d =
							(struct ipv6_opt_hdr *)(src + sizeof(struct ipv6hdr) + opts_len);
						if ( (len_delta -= ip6d->hdrlen*8 + 8) >= 0)
						{
							opts_len += ip6d->hdrlen*8 + 8;
							next_hdr = ip6d->nexthdr;
						}
					}
					else
					{
						PDEBUG("ip6_ip4(): destination header error, packet droped");
						/* Generate ICMP Parameter Problem */
						return -1;
					}
				}
				/* Routing options header */
				else if (next_hdr == NEXTHDR_ROUTING)
				{
					if ( (len_delta - sizeof(struct ipv6_rt_hdr)) >= 0)
					{
						struct ipv6_rt_hdr *ip6rt =
							(struct ipv6_rt_hdr *)(src+sizeof(struct ipv6hdr) + opts_len);
						/* RFC 2765 SIIT, 4.1:
						   If a routing header with a non-zero Segments Left field is present
						   then the packet MUST NOT be translated, and an ICMPv6 "parameter
						   problem/ erroneous header field encountered" (Type 4/Code 0) error
						   message, with the Pointer field indicating the first byte of the
						   Segments Left field, SHOULD be returned to the sender.
						   */
						if (ip6rt->segments_left != 0) {
							/* Build ICMPv6 "Parameter Problem/Erroneous Header
							   Field Encountered" & drop the packet */
							/* !!! We don't send ICMPv6 "Parameter Problem" !!! */
							PDEBUG("ip6_ip4(): routing header type != 0\n");
							return -1;
						}
						if ( (len_delta -= ip6rt->hdrlen*8 + 8) >= 0)
						{
							opts_len += ip6rt->hdrlen*8 + 8;
							next_hdr = ip6rt->nexthdr;
						}
						else
						{
							PDEBUG("ip6_ip4(): routing header error, packet droped");
							/* Generate ICMP Parameter Problem */
							return -1;
						}
					}
				}
				/* Fragment options header */
				else if (next_hdr == NEXTHDR_FRAGMENT)
				{
					if ( (len_delta -= sizeof(struct frag_hdr)) >= 0)
					{
						struct frag_hdr *ip6f =
							(struct frag_hdr *)(src+sizeof(struct ipv6hdr)+opts_len);

						opts_len += sizeof(struct frag_hdr);      /* Frag Header Length = 8 */
						ip_hdr->id = htons(ntohl(ip6f->identification)); /* ID field */
						ip_hdr->frag_off = htons((ntohs(ip6f->frag_off) & IP6F_OFF_MASK) >> 3);
										                                   /* fragment offset */
						ip_hdr->frag_off = htons(ntohs(ip_hdr->frag_off) |
										         ((ntohs(ip6f->frag_off) & IP6F_MORE_FRAG) << 13));
										                                  /* more fragments flag */
						next_hdr = ip6f->nexthdr;
					}
					else
					{
						PDEBUG("ip6_ip4(): fragment header error, packet droped");
						/* Generate ICMP Parameter Problem */
						return -1;
					}
				}
				/* No Next Header */
				else if (next_hdr == NEXTHDR_NONE)
				{
					/* RFC 2460 IPv6 Specification, 4.7
					   4.7 No Next Header

					   The value 59 in the Next Header field of an IPv6 header or any
					   extension header indicates that there is nothing following that
					   header.  If the Payload Length field of the IPv6 header indicates the
					   presence of octets past the end of a header whose Next Header field
					   contains 59, those octets must be ignored, and passed on unchanged if
					   the packet is forwarded.
					   */
					break;
				}
				else if (next_hdr == NEXTHDR_ESP || next_hdr == NEXTHDR_AUTH)
				{
					PDEBUG("ip6_ip4(): cannot translate AUTH or ESP extension header, packet dropped\n");
					return -1;
				}
				else if (next_hdr == NEXTHDR_IPV6)
				{
					PDEBUG("ip6_ip4(): cannot translate IPv6-IPv6 packet, packet dropped\n");
					return -1;
				}
				else if (next_hdr == 0)
				{
					/* As say RFC 2460 (IPv6 Spec) we should discard the packet and send an
					   ICMP Parameter Problem message to the source of the packet, with an
					   ICMP Code value of 1 ("unrecognized Next Header type encountered")
					   and the ICMP Pointer field containing the offset of the unrecognized
					   value within the original packet
					   */
					/* NOT IMPLEMENTED */
					PDEBUG("ip6_ip4(): NEXTHDR in extension header = 0, packet dropped\n");
					return -1;
				}
				else
				{
					PDEBUG("ip6_ip4(): cannot translate extension header = %d, packet dropped\n", next_hdr);
					return -1;
				}
			}
		}
	}
	else
	{
	   PDEBUG("ip6_ip4(): error packet len, packet dropped.\n");
	   return -1;
	}

	/* Building ipv4 packet */

	ip_hdr->version = IPVERSION;
	ip_hdr->ihl = 5;

	/* TOS see comment about TOS in ip4_ip6() */
	if (tos_ignore_flag)
		ip_hdr->tos = 0;
	else {
		ip_hdr->tos = ip6_hdr->priority << 4;
		ip_hdr->tos = ip_hdr->tos | (ip6_hdr->flow_lbl[0] >> 4);
	}

	/* IPv4 Total Len = IPv6 Payload Len +
	   IPv4 Header Len (without options) - Options Headers Len */
	ip6_payload_len = ntohs(ip6_hdr->payload_len);

	if (ip6_payload_len == 0)
		ntot_len = 0;
	else
		ntot_len = ip6_payload_len + IP4_IP6_HDR_DIFF - opts_len;

	ip_hdr->tot_len = htons(ntot_len);

	/* IPv4 TTL = IPv6 Hop Limit */
	ip_hdr->ttl = ip6_hdr->hop_limit;

	/* IPv4 Protocol = Next Header that will point to upper layer protocol */
	ip_hdr->protocol = next_hdr;

	/* IPv4 Src addr = last 4 bytes from IPv6 Src addr */
	ip_hdr->saddr = ip6_hdr->saddr.s6_addr32[3];
	/* IPv4 Dst addr = last 4 bytes from IPv6 Dst addr */
	ip_hdr->daddr = ip6_hdr->daddr.s6_addr32[3];

	/* Calculate IPv4 header checksum */
	ip_hdr->check = 0;
	ip_hdr->check = ip_fast_csum((unsigned char *)ip_hdr, ip_hdr->ihl);

	if (len_delta > 0)
	{
		/* PROCESS ICMP */

		if (next_hdr == NEXTHDR_ICMP)
		{
			struct icmp6hdr *icmp6_hdr;
			struct icmphdr *icmp_hdr;

			if ((len_delta -= sizeof(struct icmp6hdr)) >= 0)
			{
				icmp6_hdr = (struct icmp6hdr *)(src + sizeof(struct ipv6hdr) + opts_len);
				icmp_hdr = (struct icmphdr *)(dst + sizeof(struct iphdr));

				real_len += len_delta + sizeof(struct icmphdr);

				/* There is diffirent between ICMPv4/ICMPv6 protocol codes
				   IPPROTO_ICMP = 1
				   IPPROTO_ICMPV6 = 58        */
				ip_hdr->protocol = IPPROTO_ICMP;

				if (include_flag) {
					/* !!! Warnig !!! We discard ICMP packets with any ICMP as included
					   in ICMP Error. But ICMP Error messages can include ICMP Query message
					   */
					if (icmp6_hdr->icmp6_type != ICMPV6_ECHO_REQUEST)
					{
						PDEBUG("ip6_ip4(): included ICMPv6 in ICMPv6 Error message, packet dropped\n");
						return -1;
					}
				}

			/* Translate ICMPv6 to ICMPv4 */
				switch (icmp6_hdr->icmp6_type)
				{
/* ICMP Error messages */
			/* Destination Unreachable (Type 1) */
				case ICMPV6_DEST_UNREACH: /* Type 1 */
					icmp_hdr->type = ICMP_DEST_UNREACH; /* to Type 3 */
					icmp_hdr->un.echo.id = 0;
					icmp_hdr->un.echo.sequence = 0;
					switch (icmp6_hdr->icmp6_code)
					{
					case ICMPV6_NOROUTE: /* Code 0 */
					case ICMPV6_NOT_NEIGHBOUR: /* Code 2 */
					case ICMPV6_ADDR_UNREACH: /* Code 3  */
						icmp_hdr->code = ICMP_HOST_UNREACH; /* To Code 1 */
						break;
					case ICMPV6_ADM_PROHIBITED: /* Code 1 */
						icmp_hdr->code = ICMP_HOST_ANO; /* To Code 10 */
						break;
					case ICMPV6_PORT_UNREACH: /* Code 4 */
						icmp_hdr->code = ICMP_PORT_UNREACH; /* To Code 3 */

						break;
					default:            /* discard any other codes */
						PDEBUG("ip6_ip4(): Unknown ICMPv6 Type %d Code %d - packet dropped.\n",
							   ICMPV6_DEST_UNREACH, icmp6_hdr->icmp6_code);
						return -1;
					}
					break;
			/* Packet Too Big (Type 2) */
				case ICMPV6_PKT_TOOBIG: /* Type 2 */
					icmp_hdr->type = ICMP_DEST_UNREACH; /* to Type 3  */
					icmp_hdr->code = ICMP_FRAG_NEEDED; /*  to Code 4 */
					/* Change MTU, RFC 2765 (SIIT), 4.2:
					   The MTU field needs to be adjusted for the difference between
					   the IPv4 and IPv6 header sizes taking into account whether or
					   not the packet in error includes a Fragment header.
					   */
					/* !!! Don't implement !!! */
					icmp_hdr->un.frag.mtu = (__u16) icmp6_hdr->icmp6_mtu;
					break;
			/* Time Exceeded (Type 3) */
				case ICMPV6_TIME_EXCEED:
					icmp_hdr->type = ICMP_TIME_EXCEEDED; /* to Type 11 */
					icmp_hdr->code = icmp6_hdr->icmp6_code; /* Code unchanged */
					break;
			/* Parameter Problem (Type 4) */
				case ICMPV6_PARAMPROB:
					switch (icmp6_hdr->icmp6_code) {
					case ICMPV6_UNK_NEXTHDR: /* Code 1 */
						icmp_hdr->type = ICMP_DEST_UNREACH; /* to Type 3 */
						icmp_hdr->code = ICMP_PROT_UNREACH; /* to Code 2 */
						break;
					default: /* if Code != 1 */
						icmp_hdr->type = ICMP_PARAMETERPROB; /* to Type 12 */
						icmp_hdr->code = 0; /* to Code 0 */
						/* Update Pointer field
						   RFC 2765 (SIIT), 4.2:
						   The Pointer needs to be updated to point to the corresponding
						   field in the translated include IP header.
						   */
						switch (ntohl(icmp6_hdr->icmp6_pointer))
						{
						case 0: /* IPv6 Version -> IPv4 Version */
							icmp_hdr->un.echo.id = 0;
							break;
						case 4: /* IPv6 PayloadLength -> IPv4 Total Length */
							icmp_hdr->un.echo.id = 0x0002; /* 2 */
							break;
						case 6: /* IPv6 Next Header-> IPv4 Protocol */
							icmp_hdr->un.echo.id = 0x0009; /* 9 */
							break;
						case 7: /* IPv6 Hop Limit -> IPv4 TTL */
							icmp_hdr->un.echo.id = 0x0008; /* 8 */
							break;
						case 8: /* IPv6 Src addr -> IPv4 Src addr */
							icmp_hdr->un.echo.id = 0x000c; /* 12 */
							break;
						case 24: /* IPv6 Dst addr -> IPv4 Dst addr*/
							icmp_hdr->un.echo.id = 0x0010; /* 16 */
							break;
						default: /* set all ones in other cases */
							icmp_hdr->un.echo.id = 0xff;
							break;
						}
						break;
					}
					break;

/* End of ICMP Error messages */

			/* Echo Request and Echo Reply (Type 128 and 129)  */
				case ICMPV6_ECHO_REQUEST:
					icmperr = 0;        /* not error ICMP message */
					icmp_hdr->type = ICMP_ECHO; /* to Type 8 */
					icmp_hdr->code = 0; /* to Code 0 */
					icmp_hdr->un.echo.id = icmp6_hdr->icmp6_identifier;
					icmp_hdr->un.echo.sequence = icmp6_hdr->icmp6_sequence;
					/* copy rest of ICMP data to result packet */
					if (len_delta > 0)
						memcpy(((char *)icmp_hdr) + sizeof(struct icmphdr),
							   ((char *)icmp6_hdr) + sizeof(struct icmp6hdr), len_delta);
					break;
				case ICMPV6_ECHO_REPLY:
					icmperr = 0;        /* not error ICMP message */
					icmp_hdr->type = ICMP_ECHOREPLY; /* to Type 0 */
					icmp_hdr->code = 0; /* to Code 0 */
					icmp_hdr->un.echo.id = icmp6_hdr->icmp6_identifier;
					icmp_hdr->un.echo.sequence = icmp6_hdr->icmp6_sequence;
					/* copy rest of ICMP data */
					if (len_delta > 0)
						memcpy(((char *)icmp_hdr) + sizeof(struct icmphdr),
							   ((char *)icmp6_hdr) + sizeof(struct icmp6hdr), len_delta);
					break;
				default:
					/* Unknown error messages. Silently drop. */
					PDEBUG("ip6_ip4(): unknown ICMPv6 Type %d, packet dropped.\n", icmp6_hdr->icmp6_type);
					return -1;
				}

				if (icmperr)
				{
					/* If ICMP Error message, we translate IP included packet*/
					if (len_delta >= sizeof(struct ipv6hdr))
					{
						if((inc_opts_len = ip6_ip4((char *)icmp6_hdr + sizeof(struct icmp6hdr), len_delta,
										           (char *)icmp_hdr + sizeof(struct icmphdr), 1)) == -1) {
							PDEBUG("ip6_ip4(): incorrect translation of ICMPv6 Error message, packet dropped\n");
							return -1;
						}
						/* correct IPv4 Total Len that = old Total Len
						   - Options Headers Len in included IP packet
						   - diffirence between IPv6 Header Len and IPv4 Header Len
						   */
						if (ntot_len != 0)
							ip_hdr->tot_len = htons(ntot_len - inc_opts_len - IP4_IP6_HDR_DIFF);
						real_len = real_len - inc_opts_len - IP4_IP6_HDR_DIFF;
					}
					else if (len_delta > 0)
					{
						/* May be it need set 0x0 to rest area in result IPv4 packet,
						 * but we copy rest data unchanged
						 */
						memcpy(((char *)icmp_hdr) + sizeof(struct icmphdr),
							   ((char *)icmp6_hdr) + sizeof(struct icmp6hdr), len_delta);
					}
				}

				/* Calculate IPv4 Header checksum */
				ip_hdr->check = 0;
				ip_hdr->check = ip_fast_csum((unsigned char *)ip_hdr, ip_hdr->ihl);

				/* Calculate ICMPv4 checksum */
				if (ntot_len != 0)
				{
					icmp_hdr->checksum = 0;
					icmp_hdr->checksum = ip_compute_csum((unsigned char *)icmp_hdr, ntohs(ip_hdr->tot_len)
										             - sizeof(struct iphdr));
				}
			}
			else
			{
				PDEBUG("ip6_ip4(): error length ICMP packet, packet dropped.\n");
				return -1;
			}

		}
		/* PROCESS TCP and UDP (and rest data) */

		else {
			real_len += len_delta;
			/* we copy rest data to IPv4 packet without changing */
			memcpy(dst+sizeof(struct iphdr), src + sizeof(struct ipv6hdr) + opts_len, len_delta);
		}
	}

	if (include_flag)           /* if it's included IP packet */
		return opts_len;        /* return options headers length */
	else
		return real_len; /* result packet len */
}

/*
 * ip4_fragment(skb, len, hdr_len, dev, eth_h)
 * to fragment original IPv4 packet if result IPv6 packet will be > 1280
 */

static int ip4_fragment(struct sk_buff *skb, int len, int hdr_len, struct net_device *dev, struct ethhdr *eth_h)
{
	struct sk_buff *skb2 = NULL;       /* pointer to new struct sk_buff for transleded packet */
	char buff[FRAG_BUFF_SIZE+hdr_len]; /* buffer to form new fragment packet */
	char *cur_ptr = skb->data+hdr_len; /* pointter to current packet data with len = frag_len */
	struct iphdr *ih4 = (struct iphdr *) skb->data;
	struct iphdr *new_ih4 = (struct iphdr *) buff; /* point to new IPv4 hdr */
	struct ethhdr *new_eth_h;   /* point to ether hdr, need to set hard header data in fragment */
	int data_len = len - hdr_len; /* origin packet data len */
	int rest_len = data_len;    /* rest data to fragment */
	int frag_len = 0;           /* current fragment len */
	int last_frag = 0;          /* last fragment flag, if = 1, it's last fragment */
	int flag_last_mf = 0;
	__u16 new_id = 0;           /* to generate identification field */
	__u16 frag_offset = 0;      /* fragment offset */
	unsigned int csum;
	unsigned short udp_len;

#ifdef SIIT_DEBUG
	printk("siit: it's DF == 0 and result IPv6 packet will be > 1280\n");
	siit_print_dump(skb->data, hdr_len, "siit: (orig) ipv4_hdr dump");
#endif

	if ((ntohs(ih4->frag_off) & IP_MF) == 0 )
		/* it's a case we'll clear MF flag in our last packet */
		flag_last_mf = 1;

	if (ih4->protocol == IPPROTO_UDP) {
		if ( (ntohs(ih4->frag_off) & IP_OFFSET) == 0) {
			struct udphdr *udp_hdr = (struct udphdr *)((char *)ih4 + hdr_len);
			if (!flag_last_mf) {
				if (udp_hdr->check == 0) {
					/* it's a first fragment with ZERO checksum and we drop packet */
					printk("siit: First fragment of UDP with zero checksum - packet droped\n");
					printk("siit: addr: %x src port: %d dst port: %d\n",
						   htonl(ih4->saddr), htons(udp_hdr->source), htons(udp_hdr->dest));
					return -1;
				}
			}
			else if (udp_hdr->check == 0) {
				/* Calculate UDP checksum only if it's not fragment */
				udp_len = ntohs(udp_hdr->len);
				csum = 0;
				csum = csum_partial((unsigned char *)udp_hdr, udp_len, csum);
				udp_hdr->check = csum_tcpudp_magic(ih4->saddr, ih4->daddr, udp_len, IPPROTO_UDP, csum);
			}
		}
	}

	frag_offset = ntohs(ih4->frag_off) & IP_OFFSET;

	new_id = ih4->id;

	while(1) {
		if (rest_len <= FRAG_BUFF_SIZE) {
			/* it's last fragmen */
			frag_len = rest_len; /* rest data */
			last_frag = 1;
		}
		else
			frag_len = FRAG_BUFF_SIZE;

		/* copy IP header to buffer */
		memcpy(buff, skb->data, hdr_len);
		/* copy data to buffer with len = frag_len */
		memcpy(buff + hdr_len, cur_ptr, frag_len);

		/* set id field in new IPv4 header*/
		new_ih4->id = new_id;

		/* is it last fragmet */
		if(last_frag && flag_last_mf)
			/* clear MF flag */
			new_ih4->frag_off = htons(frag_offset & (~IP_MF));
		else
			/* set MF flag */
			new_ih4->frag_off = htons(frag_offset | IP_MF);

		/* change packet total length */
		new_ih4->tot_len = htons(frag_len+hdr_len);

		/* rebuild the header checksum (IP needs it) */
		new_ih4->check = 0;
		new_ih4->check = ip_fast_csum((unsigned char *)new_ih4,new_ih4->ihl);

		/* Allocate new sk_buff to compose translated packet */
		skb2 = dev_alloc_skb(frag_len+hdr_len+dev->hard_header_len+IP4_IP6_HDR_DIFF+IP6_FRAGMENT_SIZE);
		if (!skb2) {
			printk(KERN_DEBUG "%s: alloc_skb failure - packet dropped.\n", dev->name);
			dev_kfree_skb(skb2);
			return -1;
		}
		/* allocate skb->data portion for IP header len, fragment data len and ether header len
		 * and copy to head ether header from origin skb
		 */
		memcpy(skb_put(skb2, frag_len+hdr_len+dev->hard_header_len+IP4_IP6_HDR_DIFF+IP6_FRAGMENT_SIZE), (char *) eth_h,
			   dev->hard_header_len);
		/* correct ether header data, ether protocol field to ETH_P_IPV6 */
		new_eth_h = (struct ethhdr *)skb2->data;
		new_eth_h->h_proto = htons(ETH_P_IPV6);

		/* reset the mac header */
		skb_reset_mac_header(skb2);

		/* pull ether header from new skb->data */
		skb_pull(skb2, dev->hard_header_len);
		/* set skb protocol to IPV6 */
		skb2->protocol = htons(ETH_P_IPV6);

		/* call translation function */
		if ( ip4_ip6(buff, frag_len+hdr_len, skb2->data, 0) == -1) {
			dev_kfree_skb(skb2);
			return -1;
		}

		/*
		 * Set needed fields in new sk_buff
		 */
		skb2->dev = dev;
		skb2->ip_summed = CHECKSUM_UNNECESSARY;
		skb2->pkt_type = PACKET_HOST;

		/* Add transmit statistic */
		siit_stats(dev)->tx_packets++;
		siit_stats(dev)->tx_bytes += skb2->len;

		/* send packet to upper layer */
		netif_rx(skb2);

		/* exit if it was last fragment */
		if (last_frag)
			break;

		/* correct current data pointer */
		cur_ptr += frag_len;
		/* rest data len */
		rest_len -= frag_len;
		/* current fragment offset */
		frag_offset = (frag_offset*8 + frag_len)/8;
	}

	return 0;
}
/*
 * Transmit a packet (called by the kernel)
 *
 * siit_xmit(skb, dev)
 *
 * where
 * skb - pointer to struct sk_buff with incomed packet
 * dev - pointer to struct device on which packet revieved
 *
 * Statistic:
 * for all incoming packes:
 *            stats.rx_bytes+=skb->len
 *            stats.rx_packets++
 * for packets we can't transle:
 *            stats.tx_errors++
 * device busy:
 *            stats.tx_errors++
 * for packets we can't allocate sk_buff:
 *            stats.tx_dropped++
 * for outgoing packes:
 *            stats.tx_packets++
 *            stats.tx_bytes+=skb2->len !!! But we don't set skb2->len !!!
 */

static int siit_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct sk_buff *skb2 = NULL;/* pointer to new struct sk_buff for transleded packet */
	struct ethhdr *eth_h;       /* pointer to incoming Ether header */
	int len;                    /* original packets length */
	int new_packet_len;
	int skb_delta = 0;          /* delta size for allocate new skb */
	char new_packet_buff[2048];

	/* Check pointer to sk_buff and device structs */
	if (skb == NULL || dev == NULL)
		return -EINVAL;

	/* Add receive statistic */
	siit_stats(dev)->rx_bytes += skb->len;
	siit_stats(dev)->rx_packets++;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,7,0)
	netif_trans_update(dev);
#else
	dev->trans_start = jiffies;
#endif

	/* Upper layer (IP) protocol forms sk_buff for outgoing packet
	 * and sets IP header + Ether header too. IP layer sets outgoing
	 * device in sk_buff->dev.
	 * In function (from linux/net/core/dev.c) ther is a call to
	 * device transmit function (dev->hard_start_xmit):
	 *
	 *    dev_queue_xmit(struct sk_buff *skb)
	 *    {
	 *    ...
	 *          device *dev = skb->dev;
	 *    ...
	 *          dev->hard_start_xmit(skb, dev);
	 *    ...
	 *    }
	 * We save pointer to ether header in eth_h and skb_pull ether header
	 * from data field of skb_buff
	 */

	eth_h = (struct ethhdr *)skb->data; /* point to incoming packet Ether Header */

#ifdef SIIT_DEBUG
	siit_print_dump(skb->data, ETH_HLEN, "siit: eth_hdr dump");
#endif

	/* Remove hardware header from origin sk_buff */
	skb_pull(skb,dev->hard_header_len);

	/*
	 * Process IPv4 paket
	 */
	if (ntohs(skb->protocol) == ETH_P_IP) {
		int hdr_len;            /* IPv4 header length */
		int data_len;           /* IPv4 data length */
		struct iphdr *ih4;      /* pointer to IPv4 header */
		struct icmphdr *icmp_hdr;   /* point to current ICMPv4 header struct */

		ih4 = (struct iphdr *)skb->data; /* point to incoming packet's IPv4 header */

		/* Check IPv4 Total Length */
		if (skb->len != ntohs(ih4->tot_len)) {
			PDEBUG("siit_xmit(): Different skb_len %x and ip4 tot_len %x - packet dropped.\n",
				   skb->len, ih4->tot_len);
			siit_stats(dev)->tx_errors++;
			dev_kfree_skb(skb);
			return 0;
		}

		len = skb->len;     /* packet's total len */
		hdr_len = (int)(ih4->ihl * 4); /* packet's header len */
		data_len = len - hdr_len; /* packet's data len */

		/* If DF == 0 */
		if ( (ntohs(ih4->frag_off) & IP_DF) == 0 ) {
			/* If result IPv6 packet will be > 1280
			   we need to fragment original IPv4 packet
			*/
			if ( data_len > FRAG_BUFF_SIZE ) {
				/* call function that fragment packet and translate to IPv6 each fragment
				 * and send to upper layer
				 */
				if ( ip4_fragment(skb, len, hdr_len, dev, eth_h) == -1) {
					siit_stats(dev)->tx_errors++;
				}
				/* Free incoming skb */
				dev_kfree_skb(skb);
				/* Device can accept a new packet */

				return 0;

			}
		}
		/* If DF == 1 && MF == 0 && Fragment Offset == 0
		 * we don't include fragment header
		 */
		if ( ntohs(ih4->frag_off) == IP_DF )
			skb_delta = IP4_IP6_HDR_DIFF; /* delta is +20 */
		else
			skb_delta = IP4_IP6_HDR_DIFF + IP6_FRAGMENT_SIZE; /* delta is +20 and +8 */

		/* If it's ICMP, check is it included IP packet in it */
		if ( ih4->protocol == IPPROTO_ICMP) {
			icmp_hdr = (struct icmphdr *) (skb->data+hdr_len); /* point to ICMPv4 header */
			if ( icmp_hdr->type != ICMP_ECHO && icmp_hdr->type != ICMP_ECHOREPLY) {
				/*
				 * It's ICMP Error that has included IP packet
				 * we'll add only +20 because we don't include Fragment Header
				 * into translated included IP packet
				 */
				skb_delta += IP4_IP6_HDR_DIFF;
			}
		}

		/* Allocate new sk_buff to compose translated packet */
		skb2 = dev_alloc_skb(len+dev->hard_header_len+skb_delta);
		if (!skb2) {
			printk(KERN_DEBUG "%s: alloc_skb failure - packet dropped.\n", dev->name);
			dev_kfree_skb(skb);
			siit_stats(dev)->rx_dropped++;

			return 0;
		}
		/* allocate skb->data portion = IPv4 packet len + ether header len
		 * + skb_delta (max = two times (diffirence between IPv4 header and
		 * IPv6 header + Frag Header), second for included packet,
		 * and copy to head of skb->data ether header from origin skb
		 */
		memcpy(skb_put(skb2, len+dev->hard_header_len+skb_delta), (char *)eth_h, dev->hard_header_len);
		/* correct ether header data, ether protocol field to ETH_P_IPV6 */
		eth_h = (struct ethhdr *)skb2->data;
		eth_h->h_proto = htons(ETH_P_IPV6);
		skb_reset_mac_header(skb2);
		/* remove ether header from new skb->data,
		 * NOTE! data will rest, pointer to data and data len will change
		 */
		skb_pull(skb2,dev->hard_header_len);
		/* set skb protocol to IPV6 */
		skb2->protocol = htons(ETH_P_IPV6);

		/* call translation function */
		if (ip4_ip6(skb->data, len, skb2->data, 0) == -1 ) {
			dev_kfree_skb(skb);
			dev_kfree_skb(skb2);
			siit_stats(dev)->rx_errors++;

			return 0;
		}
	}
	/*
	 * IPv6 paket
	 */
	else if (ntohs(skb->protocol) == ETH_P_IPV6) {

#ifdef SIIT_DEBUG
		siit_print_dump(skb->data, sizeof(struct ipv6hdr), "siit: (in) ip6_hdr dump");
#endif
		/* packet len = skb->data len*/
		len = skb->len;

		/* call translation function */
		if ((new_packet_len = ip6_ip4(skb->data, len, new_packet_buff, 0)) == -1 )
		{
			PDEBUG("siit_xmit(): error translation ipv6->ipv4, packet dropped.\n");
			siit_stats(dev)->rx_dropped++;
			goto end;
		}

		/* Allocate new sk_buff to compose translated packet */
		skb2 = dev_alloc_skb(new_packet_len + dev->hard_header_len);
		if (!skb2) {
			printk(KERN_DEBUG "%s: alloc_skb failure, packet dropped.\n", dev->name);
			siit_stats(dev)->rx_dropped++;
			goto end;
		}
		memcpy(skb_put(skb2, new_packet_len + dev->hard_header_len), (char *)eth_h, dev->hard_header_len);
		eth_h = (struct ethhdr *)skb2->data;
		eth_h->h_proto = htons(ETH_P_IP);
		skb_reset_mac_header(skb2);
		skb_pull(skb2, dev->hard_header_len);
		memcpy(skb2->data, new_packet_buff, new_packet_len);
		skb2->protocol = htons(ETH_P_IP);
	}
	else {
		PDEBUG("siit_xmit(): unsupported protocol family %x, packet dropped.\n", skb->protocol);
		goto end;
	}

	/*
	 * Set needed fields in new sk_buff
	 */
	skb2->pkt_type = PACKET_HOST;
	skb2->dev = dev;
	skb2->ip_summed = CHECKSUM_UNNECESSARY;

	/* Add transmit statistic */
	siit_stats(dev)->tx_packets++;
	siit_stats(dev)->tx_bytes += skb2->len;

	/* Send packet to upper layer protocol */
	netif_rx(skb2);

end:
	dev_kfree_skb(skb);

	return 0;
}

static bool header_ops_init = false;
static struct header_ops siit_header_ops ____cacheline_aligned;

static const struct net_device_ops siit_netdev_ops = {
	.ndo_open		= siit_open,
	.ndo_stop		= siit_release,
	.ndo_start_xmit		= siit_xmit,
};

/*
 * The init function initialize of the SIIT device..
 * It is invoked by register_netdev()
 */

static void
siit_init(struct net_device *dev)
{
	ether_setup(dev);    /* assign some of the fields */
	random_ether_addr(dev->dev_addr);

	/*
	 * Assign device function.
	 */
	dev->netdev_ops = &siit_netdev_ops;
	dev->flags           |= IFF_NOARP;     /* ARP not used */
	dev->tx_queue_len = 10;

	if (!header_ops_init) {
		memcpy(&siit_header_ops, dev->header_ops, sizeof(struct header_ops));
		siit_header_ops.cache = NULL;
	}
	dev->header_ops = &siit_header_ops;
}

/*
 * Finally, the module stuff
 */
static struct net_device *siit_dev = NULL;

int init_module(void)
{
	int res = -ENOMEM;
	int priv_size;

	priv_size = sizeof(struct header_ops);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
	siit_dev = alloc_netdev(priv_size, "siit%d", NET_NAME_UNKNOWN, siit_init);
#else
	siit_dev = alloc_netdev(priv_size, "siit%d", siit_init);
#endif
	if (!siit_dev)
		goto err_alloc;

	res = register_netdev(siit_dev);
	if (res)
		goto err_register;

	return 0;

err_register:
	free_netdev(siit_dev);
err_alloc:
	printk(KERN_ERR "Error creating siit device: %d\n", res);
	return res;
}

void cleanup_module(void)
{
	unregister_netdev(siit_dev);
	free_netdev(siit_dev);
}

MODULE_LICENSE("GPL");
