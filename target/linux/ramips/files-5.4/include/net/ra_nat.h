/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (c) 2019 MediaTek Inc.
 * Author: Harry Huang <harry.huang@mediatek.com>
 */

#ifndef _RA_NAT_WANTED
#define _RA_NAT_WANTED

#include <linux/ip.h>
#include <linux/ipv6.h>

#ifndef NEXTHDR_IPIP
#define NEXTHDR_IPIP 4
#endif

#define hwnat_vlan_tx_tag_present(__skb)     ((__skb)->vlan_tci & VLAN_TAG_PRESENT)
#define hwnat_vlan_tag_get(__skb)         ((__skb)->vlan_tci & ~VLAN_TAG_PRESENT)

#if defined(CONFIG_HW_NAT)
extern void hwnat_magic_tag_set_zero(struct sk_buff *skb);
extern void hwnat_check_magic_tag(struct sk_buff *skb);
extern void hwnat_set_headroom_zero(struct sk_buff *skb);
extern void hwnat_set_tailroom_zero(struct sk_buff *skb);
extern void hwnat_copy_headroom(u8 *data, struct sk_buff *skb);
extern void hwnat_copy_tailroom(u8 *data, int size, struct sk_buff *skb);
extern void hwnat_setup_dma_ops(struct device *dev, bool coherent);
#else

static inline void hwnat_magic_tag_set_zero(struct sk_buff *skb)
{
}

static inline void hwnat_check_magic_tag(struct sk_buff *skb)
{
}

static inline void hwnat_set_headroom_zero(struct sk_buff *skb)
{
}

static inline void hwnat_set_tailroom_zero(struct sk_buff *skb)
{
}

static inline void hwnat_copy_headroom(u8 *data, struct sk_buff *skb)
{
}

static inline void hwnat_copy_tailroom(u8 *data, int size, struct sk_buff *skb)
{
}

#endif

enum foe_cpu_reason {
	TTL_0 = 0x02,		/* IPv4(IPv6) TTL(hop limit) = 0 */
	/* IPv4(IPv6) has option(extension) header */
	HAS_OPTION_HEADER = 0x03,
	NO_FLOW_IS_ASSIGNED = 0x07,	/* No flow is assigned */
	/* IPv4 HNAT doesn't support IPv4 /w fragment */
	IPV4_WITH_FRAGMENT = 0x08,
	/* IPv4 HNAPT/DS-Lite doesn't support IPv4 /w fragment */
	IPV4_HNAPT_DSLITE_WITH_FRAGMENT = 0x09,
	/* IPv4 HNAPT/DS-Lite can't find TCP/UDP sport/dport */
	IPV4_HNAPT_DSLITE_WITHOUT_TCP_UDP = 0x0A,
	/* IPv6 5T-route/6RD can't find TCP/UDP sport/dport */
	IPV6_5T_6RD_WITHOUT_TCP_UDP = 0x0B,
	/* Ingress packet is TCP fin/syn/rst */
	/*(for IPv4 NAPT/DS-Lite or IPv6 5T-route/6RD) */
	TCP_FIN_SYN_RST = 0x0C,
	UN_HIT = 0x0D,		/* FOE Un-hit */
	HIT_UNBIND = 0x0E,	/* FOE Hit unbind */
	/* FOE Hit unbind & rate reach */
	HIT_UNBIND_RATE_REACH = 0x0F,
	HIT_BIND_TCP_FIN = 0x10,	/* Hit bind PPE TCP FIN entry */
	/* Hit bind PPE entry and TTL(hop limit) = 1 */
	/* and TTL(hot limit) - 1 */
	HIT_BIND_TTL_1 = 0x11,
	/* Hit bind and VLAN replacement violation */
	/*(Ingress 1(0) VLAN layers and egress 4(3 or 4) VLAN layers) */
	HIT_BIND_WITH_VLAN_VIOLATION = 0x12,
	/* Hit bind and keep alive with unicast old-header packet */
	HIT_BIND_KEEPALIVE_UC_OLD_HDR = 0x13,
	/* Hit bind and keep alive with multicast new-header packet */
	HIT_BIND_KEEPALIVE_MC_NEW_HDR = 0x14,
	/* Hit bind and keep alive with duplicate old-header packet */
	HIT_BIND_KEEPALIVE_DUP_OLD_HDR = 0x15,
	/* FOE Hit bind & force to CPU */
	HIT_BIND_FORCE_TO_CPU = 0x16,
	/* Hit bind and remove tunnel IP header, */
	/* but inner IP has option/next header */
	HIT_BIND_WITH_OPTION_HEADER = 0x17,
	/* Hit bind and exceed MTU */
	HIT_BIND_EXCEED_MTU = 0x1C,
	HIT_BIND_PACKET_SAMPLING = 0x1B,	/*  PS packet */
	/*  Switch clone multicast packet to CPU */
	HIT_BIND_MULTICAST_TO_CPU = 0x18,
	/*  Switch clone multicast packet to GMAC1 & CPU */
	HIT_BIND_MULTICAST_TO_GMAC_CPU = 0x19,
	HIT_PRE_BIND = 0x1A	/*  Pre-bind */
};

#define MAX_IF_NUM 64

struct dmad_rx_descinfo4 {
	uint32_t foe_entry_num:15;
	uint32_t rsv0:3;
	uint32_t CRSN:5;
	uint32_t rsv1:3;
	uint32_t SPORT:4;
	uint32_t ppe:1;
	uint32_t ALG:1;
	uint32_t IF:8;
	uint32_t WDMAID:2;
	uint32_t RXID:2;
	uint32_t WCID:10;
	uint32_t BSSID:6;
	uint32_t rsv3:4;
	uint16_t minfo:1;
	uint16_t ntype:3;
	uint16_t chid:8;
	uint16_t rsv4:4;
	u16 MAGIC_TAG_PROTECT;
} __packed;

struct pdma_rx_desc_info4 {
	u16 MAGIC_TAG_PROTECT;
	uint32_t foe_entry_num:14;
	uint32_t CRSN:5;
	uint32_t SPORT:4;
	uint32_t rsv:6;
	uint32_t foe_entry_num_1:1;
	uint32_t ppe:1;
	uint32_t ALG:1;
	uint32_t IF:8;
	uint32_t WDMAID:2;
	uint32_t RXID:2;
	uint32_t WCID:10;
	uint32_t BSSID:6;
	uint32_t rsv2:4;
	uint16_t minfo:1;
	uint16_t ntype:3;
	uint16_t chid:8;
	uint16_t rsv3:4;
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	u16 SOURCE;
	u16 DEST;
#endif
} __packed;

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
struct head_rx_descinfo4 {
	uint32_t foe_entry_num:14;
	uint32_t CRSN:5;
	uint32_t SPORT:4;
	uint32_t rsv:6;
	uint32_t foe_entry_num_1:1;
	uint32_t ppe:1;
	uint32_t ALG:1;
	uint32_t IF:8;
	uint32_t WDMAID:2;
	uint32_t RXID:2;
	uint32_t WCID:10;
	uint32_t BSSID:6;
	uint32_t rsv2:4;
	uint16_t minfo:1;
	uint16_t ntype:3;
	uint16_t chid:8;
	uint16_t rsv3:4;
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	u16 SOURCE;
	u16 DEST;
#endif
	u16 MAGIC_TAG_PROTECT;
} __packed;
#else
struct head_rx_descinfo4 {
	uint32_t foe_entry_num:14;
	uint32_t CRSN:5;
	uint32_t SPORT:3;
	uint32_t rsv:1;
	uint32_t ALG:1;
	uint32_t IF:4;
	uint32_t rsv2:4;
	uint32_t MAGIC_TAG_PROTECT: 16;
	uint32_t WDMAID:2;
	uint32_t RXID:2;
	uint32_t WCID:10;
	uint32_t BSSID:6;
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	u16 SOURCE;
	u16 DEST;
#endif
} __packed;
#endif

struct cb_rx_desc_info4 {
	u16 MAGIC_TAG_PROTECT0;
	uint32_t foe_entry_num:15;
	uint32_t CRSN:5;
	uint32_t SPORT:4;
	uint32_t ALG:1;
	uint32_t rsv:7;
	uint16_t IF:8;
	uint16_t WDMAID:2;
	uint16_t RXID:2;
	uint16_t WCID:10;
	uint16_t BSSID:6;
	uint16_t rsv1:4;
	uint16_t minfo:1;
	uint16_t ntype:3;
	uint16_t chid:8;
	uint16_t rsv2:4;
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	u16 SOURCE;
	u16 DEST;
#endif
	u16 MAGIC_TAG_PROTECT1;
} __packed;



#define FOE_INFO_LEN		    12
#define WIFI_INFO_LEN		    6


#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_INFO_LEN		    (6 + 4 + WIFI_INFO_LEN)
#define FOE_MAGIC_FASTPATH	    0x77
#define FOE_MAGIC_L2TPPATH	    0x78
#endif

#define FOE_MAGIC_PCI		    0x73
#define FOE_MAGIC_WLAN		    0x74
#define FOE_MAGIC_GE		    0x75
#define FOE_MAGIC_PPE		    0x76
#define FOE_MAGIC_WED0		    0x78
#define FOE_MAGIC_WED1		    0x79
#define FOE_MAGIC_MED		    0x80
#define FOE_MAGIC_EDMA0		    0x81
#define FOE_MAGIC_EDMA1		    0x82
#define TAG_PROTECT                 0x6789
#define USE_HEAD_ROOM               0
#define USE_TAIL_ROOM               1
#define USE_CB                      2
#define ALL_INFO_ERROR              3

/**************************DMAD FORMAT********************************/
#define FOE_TAG_PROTECT(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->MAGIC_TAG_PROTECT)

#define FOE_ENTRY_NUM(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->foe_entry_num)
#define FOE_ALG(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->ALG)
#define FOE_AI(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->CRSN)
#define FOE_SP(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->SPORT)
#define FOE_MAGIC_TAG(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->IF)
#define FOE_WDMA_ID(skb)  \
	(((struct dmad_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID(skb)	(((struct dmad_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID(skb)	(((struct dmad_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID(skb)	(((struct dmad_rx_descinfo4 *)((skb)->head))->BSSID)
#define FOE_PPE(skb)	(((struct dmad_rx_descinfo4 *)((skb)->head))->ppe)

/***********************HEAD FORMAT*************************************/

#define FOE_TAG_PROTECT_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->MAGIC_TAG_PROTECT)
#define FOE_ENTRY_NUM_LSB_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->foe_entry_num)
#define FOE_ENTRY_NUM_MSB_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->foe_entry_num_1)

#define FOE_ENTRY_NUM_HEAD(skb)  \
	(((FOE_ENTRY_NUM_MSB_HEAD(skb) & 0x1) << 14) | FOE_ENTRY_NUM_LSB_HEAD(skb))


#define FOE_ALG_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->ALG)
#define FOE_AI_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->CRSN)
#define FOE_SP_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->SPORT)
#define FOE_MAGIC_TAG_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->IF)


#define FOE_WDMA_ID_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->RXID)
#define FOE_WC_ID_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->BSSID)
#define FOE_PPE_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->PPE)

/****************************TAIL FORMAT***************************************/
#define FOE_TAG_PROTECT_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->MAGIC_TAG_PROTECT)
#define FOE_ENTRY_NUM_LSB_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->foe_entry_num)

#define FOE_ENTRY_NUM_MSB_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->foe_entry_num_1)
#define FOE_ENTRY_NUM_TAIL(skb)  \
	(((FOE_ENTRY_NUM_MSB_TAIL(skb) & 0x1) << 14) | FOE_ENTRY_NUM_LSB_TAIL(skb))
#define FOE_ALG_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->ALG)
#define FOE_AI_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->CRSN)
#define FOE_SP_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->SPORT)
#define FOE_MAGIC_TAG_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->IF)

#define FOE_WDMA_ID_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->WDMAID)
#define FOE_RX_ID_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->RXID)
#define FOE_WC_ID_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->WCID)
#define FOE_BSS_ID_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->BSSID)

#define FOE_PPE_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->ppe)
/*********************************************************************/
#define FOE_WDMA_ID_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->head))->WDMAID)
#define FOE_RX_ID_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->head))->RXID)
#define FOE_WC_ID_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->head))->WCID)
#define FOE_BSS_ID_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->head))->BSSID)

#define FOE_MINFO(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->minfo)
#define FOE_MINFO_NTYPE(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->ntype)
#define FOE_MINFO_CHID(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->chid)
#define FOE_MINFO_HEAD(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->minfo)
#define FOE_MINFO_NTYPE_HEAD(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->ntype)
#define FOE_MINFO_CHID_HEAD(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->chid)

#define FOE_MINFO_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->minfo)
#define FOE_MINFO_NTYPE_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->ntype)
#define FOE_MINFO_CHID_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->chid)

#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST(skb)	(((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#endif

#define IS_SPACE_AVAILABLE_HEAD(skb)  \
	((((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)))
#define IS_SPACE_AVAILABLE_HEAD(skb)  \
	((((skb_headroom(skb) >= FOE_INFO_LEN) ? 1 : 0)))
#define FOE_INFO_START_ADDR_HEAD(skb)	(skb->head)

#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#endif

#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->SOURCE)
#define FOE_DEST_HEAD(skb)  \
	(((struct head_rx_descinfo4 *)((skb)->head))->DEST)
#endif
#define IS_SPACE_AVAILABLE_TAIL(skb)  \
	(((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0))
#define IS_SPACE_AVAILABLE_TAIL(skb)  \
	(((skb_tailroom(skb) >= FOE_INFO_LEN) ? 1 : 0))
#define FOE_INFO_START_ADDR_TAIL(skb)  \
	((unsigned char *)(long)(skb_end_pointer(skb) - FOE_INFO_LEN))

#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->SOURCE)
#define FOE_DEST_TAIL(skb)  \
	(((struct pdma_rx_desc_info4 *)((long)((skb_end_pointer(skb)) - FOE_INFO_LEN)))->DEST)
#endif

/* change the position of skb_CB if necessary */
#define CB_OFFSET		    40
#define IS_SPACE_AVAILABLE_CB(skb)    1
#define FOE_INFO_START_ADDR_CB(skb)    (skb->cb +  CB_OFFSET)
#define FOE_TAG_PROTECT_CB0(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG_PROTECT0)
#define FOE_TAG_PROTECT_CB1(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->MAGIC_TAG_PROTECT1)
#define FOE_ENTRY_NUM_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->foe_entry_num)
#define FOE_ALG_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->ALG)
#define FOE_AI_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->CRSN)
#define FOE_SP_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->SPORT)
#define FOE_MAGIC_TAG_CB(skb)  \
	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->IF)

#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
#define FOE_SOURCE_CB(skb)	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->SOURCE)
#define FOE_DEST_CB(skb)	(((struct cb_rx_desc_info4 *)((skb)->cb + CB_OFFSET))->DEST)
#endif

#define IS_MAGIC_TAG_PROTECT_VALID_HEAD(skb)  \
	(FOE_TAG_PROTECT_HEAD(skb) == TAG_PROTECT)
#define IS_MAGIC_TAG_PROTECT_VALID_TAIL(skb)  \
	(FOE_TAG_PROTECT_TAIL(skb) == TAG_PROTECT)
#define IS_MAGIC_TAG_PROTECT_VALID_CB(skb)  \
	((FOE_TAG_PROTECT_CB0(skb) == TAG_PROTECT) && \
	(FOE_TAG_PROTECT_CB0(skb) == FOE_TAG_PROTECT_CB1(skb)))

#define IS_IF_PCIE_WLAN_HEAD(skb)  \
	((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) || \
	(FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) || \
	(FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE))

#define IS_IF_PCIE_WLAN_TAIL(skb)  \
	((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) || \
	(FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN))

#define IS_IF_PCIE_WLAN_CB(skb)  \
	((FOE_MAGIC_TAG_CB(skb) == FOE_MAGIC_PCI) || \
	(FOE_MAGIC_TAG_CB(skb) == FOE_MAGIC_WLAN))

/* macros */
#define magic_tag_set_zero(skb) \
{ \
	if ((FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_PCI) || \
	    (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_WLAN) || \
	    (FOE_MAGIC_TAG_HEAD(skb) == FOE_MAGIC_GE)) { \
		if (IS_SPACE_AVAILABLE_HEAD(skb)) \
			FOE_MAGIC_TAG_HEAD(skb) = 0; \
	} \
	if ((FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_PCI) || \
	    (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_WLAN) || \
	    (FOE_MAGIC_TAG_TAIL(skb) == FOE_MAGIC_GE)) { \
		if (IS_SPACE_AVAILABLE_TAIL(skb)) \
			FOE_MAGIC_TAG_TAIL(skb) = 0; \
	} \
}

static inline void hwnat_set_l2tp_unhit(struct iphdr *iph, struct sk_buff *skb)
{
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	/* only clear headeroom for TCP OR not L2TP packets */
	if ((iph->protocol == 0x6) || (ntohs(udp_hdr(skb)->dest) != 1701)) {
		if (IS_SPACE_AVAILABLE_HEAD(skb)) {
			FOE_MAGIC_TAG(skb) = 0;
			FOE_AI(skb) = UN_HIT;
		}
	}
#endif
}

static inline void hwnat_set_l2tp_fast_path(u32 l2tp_fast_path, u32 pptp_fast_path)
{
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	l2tp_fast_path = 1;
	pptp_fast_path = 0;
#endif
}

static inline void hwnat_clear_l2tp_fast_path(u32 l2tp_fast_path)
{
#if defined(CONFIG_RA_HW_NAT_PPTP_L2TP)
	l2tp_fast_path = 0;
#endif
}

/* #define CONFIG_HW_NAT_IPI */
#if defined(CONFIG_HW_NAT_IPI)
extern int debug_level;
int get_rps_cpu(struct net_device *dev, struct sk_buff *skb,
		struct rps_dev_flow **rflowp);
uint32_t ppe_extif_rx_handler(struct sk_buff *skb);
int hitbind_force_to_cpu_handler(struct sk_buff *skb, struct foe_entry *entry);
extern unsigned int ipidbg[num_possible_cpus()][10];
extern unsigned int ipidbg2[num_possible_cpus()][10];
/* #define HNAT_IPI_RXQUEUE	1 */
#define HNAT_IPI_DQ		1
#define HNAT_IPI_HASH_NORMAL	0
#define HNAT_IPI_HASH_VTAG		1
#define HNAT_IPI_HASH_FROM_EXTIF	2
#define HNAT_IPI_HASH_FROM_GMAC		4

struct hnat_ipi_s {
#if defined(HNAT_IPI_DQ)
	struct sk_buff_head     skb_input_queue;
	struct sk_buff_head     skb_process_queue;
#elif defined(HNAT_IPI_RXQUEUE)
	atomic_t rx_queue_num;
	unsigned int rx_queue_ridx;
	unsigned int rx_queue_widx;
	struct sk_buff **rx_queue;
#else
	/* unsigned int dummy0[0]; */
	struct sk_buff_head     skb_ipi_queue;
	/* unsigned int dummy1[8]; */
#endif
	unsigned long time_rec, recv_time;
	unsigned int ipi_accum;
	/*hwnat ipi use*/
	spinlock_t      ipilock;
	struct tasklet_struct smp_func_call_tsk;
} ____cacheline_aligned_in_smp;

struct hnat_ipi_stat {
	unsigned long drop_pkt_num_from_extif;
	unsigned long drop_pkt_num_from_ppehit;
	unsigned int smp_call_cnt_from_extif;
	unsigned int smp_call_cnt_from_ppehit;
	atomic_t cpu_status;
	/* atomic_t cpu_status_from_extif; */
	/* atomic_t cpu_status_from_ppehit; */

	/* atomic_t hook_status_from_extif; */
	/* atomic_t hook_status_from_ppehit; */
} ____cacheline_aligned_in_smp;

#define cpu_status_from_extif	cpu_status
#define cpu_status_from_ppehit	cpu_status

struct hnat_ipi_cfg {
	unsigned int enable_from_extif;
	unsigned int enable_from_ppehit;
	unsigned int queue_thresh_from_extif;
	unsigned int queue_thresh_from_ppehit;
	unsigned int drop_pkt_from_extif;
	unsigned int drop_pkt_from_ppehit;
	unsigned int ipi_cnt_mod_from_extif;
	unsigned int ipi_cnt_mod_from_ppehit;
} ____cacheline_aligned_in_smp;

int hnat_ipi_init(void);
int hnat_ipi_de_init(void);
#endif

#define QDMA_RX		5
#define PDMA_RX		0


#endif
