/*   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Copyright (C) 2014-2016 Sean Wang <sean.wang@mediatek.com>
 *   Copyright (C) 2016-2017 John Crispin <blogic@openwrt.org>
 */

#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/if.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/reset.h>
#include <linux/netfilter.h>
#include <linux/netdevice.h>
#include <net/netfilter/nf_flow_table.h>
#include <linux/debugfs.h>
#include <linux/etherdevice.h>
#include <linux/bitfield.h>

#include "mtk_eth_soc.h"

#ifdef CONFIG_RALINK
/* ramips compat */
#define mtk_eth					fe_priv
#define MTK_GDMA_FWD_CFG(x)			(0x500 + (x * 0x1000))
#define mtk_m32					fe_m32

static inline u32
mtk_r32(struct mtk_eth *eth, u32 reg)
{
	return fe_r32(reg);
}

static inline void
mtk_w32(struct mtk_eth *eth, u32 val, u32 reg)
{
	fe_w32(val, reg);
}
#endif

#define MTK_REG_PPE_GLO_CFG			0xe00
#define   MTK_PPE_GLO_CFG_BUSY			BIT(31)
#define   MTK_PPE_GLO_CFG_TTL0_DROP		BIT(4)
#define   MTK_PPE_GLO_CFG_EN			BIT(0)

#define MTK_REG_PPE_FLOW_CFG			0xe04
#define   MTK_PPE_FLOW_CFG_IPV4_GREK_EN		BIT(19)
#define   MTK_PPE_FLOW_CFG_IPV4_NAT_FRAG_EN	BIT(17)
#define   MTK_PPE_FLOW_CFG_IPV4_NAPT_EN		BIT(13)
#define   MTK_PPE_FLOW_CFG_IPV4_NAT_EN		BIT(12)
#define   MTK_PPE_FLOW_CFG_FUC_FOE		BIT(2)
#define   MTK_PPE_FLOW_CFG_FMC_FOE		BIT(1)

#define MTK_REG_PPE_IP_PROT_CHK			0xe08

#define MTK_REG_PPE_TB_BASE			0xe20

#define MTK_REG_PPE_BNDR			0xe28
#define   MTK_PPE_BNDR_RATE_MASK		0xffff

#define MTK_REG_PPE_BIND_LMT_0			0xe2C

#define MTK_REG_PPE_BIND_LMT_1			0xe30
#define   MTK_PPE_NTU_KA			BIT(16)

#define MTK_REG_PPE_KA				0xe34
#define   MTK_PPE_KA_T				BIT(0)
#define   MTK_PPE_KA_TCP			BIT(16)
#define   MTK_PPE_KA_UDP			BIT(24)

#define MTK_REG_PPE_UNB_AGE			0xe38
#define   MTK_PPE_UNB_AGE_MNP_MASK		(0xffff << 16)
#define   MTK_PPE_UNB_AGE_MNP			(1000 << 16)
#define   MTK_PPE_UNB_AGE_DLTA_MASK		0xff
#define   MTK_PPE_UNB_AGE_DLTA			3

#define MTK_REG_PPE_BND_AGE0			0xe3c
#define   MTK_PPE_BND_AGE0_NTU_DLTA_MASK	(0xffff << 16)
#define   MTK_PPE_BND_AGE0_NTU_DLTA		(5 << 16)
#define   MTK_PPE_BND_AGE0_UDP_DLTA_MASK	0xffff
#define   MTK_PPE_BND_AGE0_UDP_DLTA		5

#define MTK_REG_PPE_BND_AGE1			0xe40
#define   MTK_PPE_BND_AGE1_FIN_DLTA_MASK	(0xffff << 16)
#define   MTK_PPE_BND_AGE1_FIN_DLTA		(5 << 16)
#define   MTK_PPE_BND_AGE1_TCP_DLTA_MASK	0xffff
#define   MTK_PPE_BND_AGE1_TCP_DLTA		5

#define MTK_REG_PPE_DFT_CPORT			0xe48

#define MTK_REG_PPE_TB_CFG			0xe1c
#define   MTK_PPE_TB_CFG_X_MODE_MASK		(3 << 18)
#define   MTK_PPE_TB_CFG_HASH_MODE1		BIT(14)
#define   MTK_PPE_TB_CFG_HASH_MODE_MASK		(0x3 << 14)
#define   MTK_PPE_TB_CFG_KA			(3 << 12)
#define   MTK_PPE_TB_CFG_KA_MASK		(0x3 << 12)
#define   MTK_PPE_TB_CFG_FIN_AGE		BIT(11)
#define   MTK_PPE_TB_CFG_UDP_AGE		BIT(10)
#define   MTK_PPE_TB_CFG_TCP_AGE		BIT(9)
#define   MTK_PPE_TB_CFG_UNBD_AGE		BIT(8)
#define   MTK_PPE_TB_CFG_NTU_AGE		BIT(7)
#define   MTK_PPE_TB_CFG_SMA_FWD_CPU		(0x3 << 4)
#define   MTK_PPE_TB_CFG_SMA_MASK		(0x3 << 4)
#define   MTK_PPE_TB_CFG_ENTRY_SZ_64B		0
#define   MTK_PPE_TB_CFG_ENTRY_SZ_MASK		BIT(3)
#define   MTK_PPE_TB_CFG_TBL_SZ_4K		2
#define   MTK_PPE_TB_CFG_TBL_SZ_MASK		0x7

#define MTK_REG_PPE_HASH_SEED			0xe44
#define   MTK_PPE_HASH_SEED			0x12345678


#define MTK_REG_PPE_CAH_CTRL			0xf20
#define   MTK_PPE_CAH_CTRL_X_MODE		BIT(9)
#define   MTK_PPE_CAH_CTRL_EN			BIT(0)

struct mtk_foe_unbind_info_blk {
	u32 time_stamp:8;
	u32 pcnt:16;		/* packet count */
	u32 preb:1;
	u32 pkt_type:3;
	u32 state:2;
	u32 udp:1;
	u32 sta:1;		/* static entry */
} __attribute__ ((packed));

struct mtk_foe_bind_info_blk {
	u32 time_stamp:15;
	u32 ka:1;		/* keep alive */
	u32 vlan_layer:3;
	u32 psn:1;		/* egress packet has PPPoE session */
#ifdef CONFIG_RALINK
	u32 vpm:2;		/* 0:ethertype remark, 1:0x8100(CR default) */
#else
	u32 vpm:1;		/* 0:ethertype remark, 1:0x8100(CR default) */
	u32 ps:1;		/* packet sampling */
#endif
	u32 cah:1;		/* cacheable flag */
	u32 rmt:1;		/* remove tunnel ip header (6rd/dslite only) */
	u32 ttl:1;
	u32 pkt_type:3;
	u32 state:2;
	u32 udp:1;
	u32 sta:1;		/* static entry */
} __attribute__ ((packed));

struct mtk_foe_info_blk2 {
	u32 qid:4;		/* QID in Qos Port */
	u32 fqos:1;		/* force to PSE QoS port */
	u32 dp:3;		/* force to PSE port x 
				 0:PSE,1:GSW, 2:GMAC,4:PPE,5:QDMA,7=DROP */
	u32 mcast:1;		/* multicast this packet to CPU */
	u32 pcpl:1;		/* OSBN */
	u32 mlen:1;		/* 0:post 1:pre packet length in meter */
	u32 alen:1;		/* 0:post 1:pre packet length in accounting */
	u32 port_mg:6;		/* port meter group */
	u32 port_ag:6;		/* port account group */
	u32 dscp:8;		/* DSCP value */
} __attribute__ ((packed));

struct mtk_foe_ipv4_hnapt {
	union {
		struct mtk_foe_bind_info_blk bfib1;
		struct mtk_foe_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 sip;
	u32 dip;
	u16 dport;
	u16 sport;
	union {
		struct mtk_foe_info_blk2 iblk2;
		u32 info_blk2;
	};
	u32 new_sip;
	u32 new_dip;
	u16 new_dport;
	u16 new_sport;
	u32 resv1;
	u32 resv2;
	u32 resv3:26;
	u32 act_dp:6;		/* UDF */
	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	u16 vlan2;
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
} __attribute__ ((packed));

struct mtk_foe_entry {
	union {
		struct mtk_foe_unbind_info_blk udib1;
		struct mtk_foe_bind_info_blk bfib1;
		struct mtk_foe_ipv4_hnapt ipv4_hnapt;
	};
};

enum mtk_foe_entry_state {
	FOE_STATE_INVALID = 0,
	FOE_STATE_UNBIND = 1,
	FOE_STATE_BIND = 2,
	FOE_STATE_FIN = 3
};


#define MTK_RXD4_FOE_ENTRY		GENMASK(13, 0)
#define MTK_RXD4_CPU_REASON		GENMASK(18, 14)
#define MTK_RXD4_SRC_PORT		GENMASK(21, 19)
#define MTK_RXD4_ALG			GENMASK(31, 22)

enum mtk_foe_cpu_reason {
	MTK_CPU_REASON_TTL_EXCEEDED		= 0x02,
	MTK_CPU_REASON_OPTION_HEADER		= 0x03,
	MTK_CPU_REASON_NO_FLOW			= 0x07,
	MTK_CPU_REASON_IPV4_FRAG		= 0x08,
	MTK_CPU_REASON_IPV4_DSLITE_FRAG		= 0x09,
	MTK_CPU_REASON_IPV4_DSLITE_NO_TCP_UDP	= 0x0a,
	MTK_CPU_REASON_IPV6_6RD_NO_TCP_UDP	= 0x0b,
	MTK_CPU_REASON_TCP_FIN_SYN_RST		= 0x0c,
	MTK_CPU_REASON_UN_HIT			= 0x0d,
	MTK_CPU_REASON_HIT_UNBIND		= 0x0e,
	MTK_CPU_REASON_HIT_UNBIND_RATE_REACHED	= 0x0f,
	MTK_CPU_REASON_HIT_BIND_TCP_FIN		= 0x10,
	MTK_CPU_REASON_HIT_TTL_1		= 0x11,
	MTK_CPU_REASON_HIT_BIND_VLAN_VIOLATION	= 0x12,
	MTK_CPU_REASON_KEEPALIVE_UC_OLD_HDR	= 0x13,
	MTK_CPU_REASON_KEEPALIVE_MC_NEW_HDR	= 0x14,
	MTK_CPU_REASON_KEEPALIVE_DUP_OLD_HDR	= 0x15,
	MTK_CPU_REASON_HIT_BIND_FORCE_CPU	= 0x16,
	MTK_CPU_REASON_TUNNEL_OPTION_HEADER	= 0x17,
	MTK_CPU_REASON_MULTICAST_TO_CPU		= 0x18,
	MTK_CPU_REASON_MULTICAST_TO_GMAC1_CPU	= 0x19,
	MTK_CPU_REASON_HIT_PRE_BIND		= 0x1a,
	MTK_CPU_REASON_PACKET_SAMPLING		= 0x1b,
	MTK_CPU_REASON_EXCEED_MTU		= 0x1c,
	MTK_CPU_REASON_PPE_BYPASS		= 0x1e,
	MTK_CPU_REASON_INVALID			= 0x1f,
};


/* our table size is 4K */
#define MTK_PPE_ENTRY_CNT		0x1000
#define MTK_PPE_TBL_SZ			\
			(MTK_PPE_ENTRY_CNT * sizeof(struct mtk_foe_entry))

int ra_ppe_debugfs_init(struct mtk_eth *eth);


