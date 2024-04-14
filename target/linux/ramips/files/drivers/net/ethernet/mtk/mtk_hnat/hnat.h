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

#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <net/netevent.h>
#include <linux/mod_devicetable.h>
#include <linux/version.h>
#include "hnat_mcast.h"

#if LINUX_VERSION_CODE > KERNEL_VERSION(5, 5, 0)
#include <net/netfilter/nf_hnat.h>
#endif

/*--------------------------------------------------------------------------*/
/* Register Offset*/
/*--------------------------------------------------------------------------*/
#define PPE_GLO_CFG 0x00
#define PPE_FLOW_CFG 0x04
#define PPE_IP_PROT_CHK 0x08
#define PPE_IP_PROT_0 0x0C
#define PPE_IP_PROT_1 0x10
#define PPE_IP_PROT_2 0x14
#define PPE_IP_PROT_3 0x18
#define PPE_TB_CFG 0x1C
#define PPE_TB_BASE 0x20
#define PPE_TB_USED 0x24
#define PPE_BNDR 0x28
#define PPE_BIND_LMT_0 0x2C
#define PPE_BIND_LMT_1 0x30
#define PPE_KA 0x34
#define PPE_UNB_AGE 0x38
#define PPE_BND_AGE_0 0x3C
#define PPE_BND_AGE_1 0x40
#define PPE_HASH_SEED 0x44
#define PPE_DFT_CPORT 0x48
#define PPE_DFT_CPORT1 0x4C
#define PPE_MCAST_PPSE 0x84
#define PPE_MCAST_L_0 0x88
#define PPE_MCAST_H_0 0x8C
#define PPE_MCAST_L_1 0x90
#define PPE_MCAST_H_1 0x94
#define PPE_MCAST_L_2 0x98
#define PPE_MCAST_H_2 0x9C
#define PPE_MCAST_L_3 0xA0
#define PPE_MCAST_H_3 0xA4
#define PPE_MCAST_L_4 0xA8
#define PPE_MCAST_H_4 0xAC
#define PPE_MCAST_L_5 0xB0
#define PPE_MCAST_H_5 0xB4
#define PPE_MCAST_L_6 0xBC
#define PPE_MCAST_H_6 0xC0
#define PPE_MCAST_L_7 0xC4
#define PPE_MCAST_H_7 0xC8
#define PPE_MCAST_L_8 0xCC
#define PPE_MCAST_H_8 0xD0
#define PPE_MCAST_L_9 0xD4
#define PPE_MCAST_H_9 0xD8
#define PPE_MCAST_L_A 0xDC
#define PPE_MCAST_H_A 0xE0
#define PPE_MCAST_L_B 0xE4
#define PPE_MCAST_H_B 0xE8
#define PPE_MCAST_L_C 0xEC
#define PPE_MCAST_H_C 0xF0
#define PPE_MCAST_L_D 0xF4
#define PPE_MCAST_H_D 0xF8
#define PPE_MCAST_L_E 0xFC
#define PPE_MCAST_H_E 0xE0
#define PPE_MCAST_L_F 0x100
#define PPE_MCAST_H_F 0x104
#define PPE_MCAST_L_10 0xC00
#define PPE_MCAST_H_10 0xC04
#define PPE_MTU_DRP 0x108
#define PPE_MTU_VLYR_0 0x10C
#define PPE_MTU_VLYR_1 0x110
#define PPE_MTU_VLYR_2 0x114
#define PPE_VPM_TPID 0x118
#define PPE_CAH_CTRL 0x120
#define PPE_CAH_TAG_SRH 0x124
#define PPE_CAH_LINE_RW 0x128
#define PPE_CAH_WDATA 0x12C
#define PPE_CAH_RDATA 0x130

#define PPE_MIB_CFG 0X134
#define PPE_MIB_TB_BASE 0X138
#define PPE_MIB_SER_CR 0X13C
#define PPE_MIB_SER_R0 0X140
#define PPE_MIB_SER_R1 0X144
#define PPE_MIB_SER_R2 0X148
#define PPE_MIB_CAH_CTRL 0X150
#define PPE_MIB_CAH_TAG_SRH 0X154
#define PPE_MIB_CAH_LINE_RW 0X158
#define PPE_MIB_CAH_WDATA 0X15C
#define PPE_MIB_CAH_RDATA 0X160
#define PPE_SBW_CTRL 0x174

#define GDMA1_FWD_CFG 0x500
#define GDMA2_FWD_CFG 0x1500

/* QDMA Tx queue configuration */
#define QTX_CFG(x)			(QDMA_BASE + ((x) * 0x10))
#define QTX_CFG_HW_RESV_CNT_OFFSET	(8)
#define QTX_CFG_SW_RESV_CNT_OFFSET	(0)

#define QTX_SCH(x)			(QDMA_BASE + 0x4 + ((x) * 0x10))
#define QTX_SCH_MIN_RATE_EN		BIT(27)
#define QTX_SCH_MAX_RATE_EN		BIT(11)
#define QTX_SCH_MIN_RATE_MAN_OFFSET	(20)
#define QTX_SCH_MIN_RATE_EXP_OFFSET	(16)
#define QTX_SCH_MAX_RATE_WGHT_OFFSET	(12)
#define QTX_SCH_MAX_RATE_MAN_OFFSET	(4)
#define QTX_SCH_MAX_RATE_EXP_OFFSET	(0)

/* QDMA Tx scheduler configuration */
#define QDMA_PAGE			(QDMA_BASE + 0x1f0)
#define QDMA_TX_2SCH_BASE		(QDMA_BASE + 0x214)
#define QTX_MIB_IF			(QDMA_BASE + 0x2bc)
#define QDMA_TX_4SCH_BASE(x)		(QDMA_BASE + 0x398 + (((x) >> 1) * 0x4))
#define QDMA_TX_SCH_WFQ_EN		BIT(15)

/*--------------------------------------------------------------------------*/
/* Register Mask*/
/*--------------------------------------------------------------------------*/
/* PPE_TB_CFG mask */
#define TB_ETRY_NUM (0x7 << 0) /* RW */
#define TB_ENTRY_SIZE (0x1 << 3) /* RW */
#define SMA (0x3 << 4) /* RW */
#define NTU_AGE (0x1 << 7) /* RW */
#define UNBD_AGE (0x1 << 8) /* RW */
#define TCP_AGE (0x1 << 9) /* RW */
#define UDP_AGE (0x1 << 10) /* RW */
#define FIN_AGE (0x1 << 11) /* RW */
#define KA_CFG (0x3 << 12)
#define HASH_MODE (0x3 << 14) /* RW */
#define SCAN_MODE (0x3 << 16) /* RW */
#define XMODE (0x3 << 18) /* RW */
#define TICK_SEL (0x1 << 24) /* RW */


/*PPE_CAH_CTRL mask*/
#define CAH_EN (0x1 << 0) /* RW */
#define CAH_X_MODE (0x1 << 9) /* RW */

/*PPE_UNB_AGE mask*/
#define UNB_DLTA (0xff << 0) /* RW */
#define UNB_MNP (0xffff << 16) /* RW */

/*PPE_BND_AGE_0 mask*/
#define UDP_DLTA (0xffff << 0) /* RW */
#define NTU_DLTA (0xffff << 16) /* RW */

/*PPE_BND_AGE_1 mask*/
#define TCP_DLTA (0xffff << 0) /* RW */
#define FIN_DLTA (0xffff << 16) /* RW */

/*PPE_KA mask*/
#define KA_T (0xffff << 0) /* RW */
#define TCP_KA (0xff << 16) /* RW */
#define UDP_KA (0xff << 24) /* RW */

/*PPE_BIND_LMT_0 mask*/
#define QURT_LMT (0x3ff << 0) /* RW */
#define HALF_LMT (0x3ff << 16) /* RW */

/*PPE_BIND_LMT_1 mask*/
#define FULL_LMT (0x3fff << 0) /* RW */
#define NTU_KA (0xff << 16) /* RW */

/*PPE_BNDR mask*/
#define BIND_RATE (0xffff << 0) /* RW */
#define PBND_RD_PRD (0xffff << 16) /* RW */

/*PPE_GLO_CFG mask*/
#define PPE_EN (0x1 << 0) /* RW */
#define TTL0_DRP (0x1 << 4) /* RW */
#define MCAST_TB_EN (0x1 << 7) /* RW */
#define MCAST_HASH (0x3 << 12) /* RW */

#define MC_P3_PPSE (0xf << 12) /* RW */
#define MC_P2_PPSE (0xf << 8) /* RW */
#define MC_P1_PPSE (0xf << 4) /* RW */
#define MC_P0_PPSE (0xf << 0) /* RW */

#define MIB_EN (0x1 << 0) /* RW */
#define MIB_READ_CLEAR (0X1 << 1) /* RW */
#define MIB_CAH_EN (0X1 << 0) /* RW */

/*GDMA_FWD_CFG mask */
#define GDM_UFRC_MASK (0x7 << 12) /* RW */
#define GDM_BFRC_MASK (0x7 << 8) /*RW*/
#define GDM_MFRC_MASK (0x7 << 4) /*RW*/
#define GDM_OFRC_MASK (0x7 << 0) /*RW*/
#define GDM_ALL_FRC_MASK                                                      \
	(GDM_UFRC_MASK | GDM_BFRC_MASK | GDM_MFRC_MASK | GDM_OFRC_MASK)

/*QDMA_PAGE mask*/
#define QTX_CFG_PAGE (0xf << 0) /* RW */

/*QTX_MIB_IF mask*/
#define MIB_ON_QTX_CFG (0x1 << 31) /* RW */
#define VQTX_MIB_EN (0x1 << 28) /* RW */

/*--------------------------------------------------------------------------*/
/* Descriptor Structure */
/*--------------------------------------------------------------------------*/
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
struct hnat_unbind_info_blk {
	u32 time_stamp : 8;
	u32 sp : 4;
	u32 pcnt : 8;
	u32 ilgf : 1;
	u32 mc : 1;
	u32 preb : 1;
	u32 pkt_type : 5;
	u32 state : 2;
	u32 udp : 1;
	u32 sta : 1;		/* static entry */
} __packed;

struct hnat_bind_info_blk {
	u32 time_stamp : 8;
	u32 sp : 4;
	u32 mc : 1;
	u32 ka : 1;		/* keep alive */
	u32 vlan_layer : 3;
	u32 psn : 1;		/* egress packet has PPPoE session */
	u32 vpm : 1;		/* 0:ethertype remark, 1:0x8100(CR default) */
	u32 ps : 1;		/* packet sampling */
	u32 cah : 1;		/* cacheable flag */
	u32 rmt : 1;		/* remove tunnel ip header (6rd/dslite only) */
	u32 ttl : 1;
	u32 pkt_type : 5;
	u32 state : 2;
	u32 udp : 1;
	u32 sta : 1;		/* static entry */
} __packed;

struct hnat_info_blk2 {
	u32 qid : 7;		/* QID in Qos Port */
	u32 port_mg : 1;
	u32 fqos : 1;		/* force to PSE QoS port */
	u32 dp : 4;		/* force to PSE port x */
	u32 mcast : 1;		/* multicast this packet to CPU */
	u32 pcpl : 1;		/* OSBN */
	u32 mibf : 1;
	u32 alen : 1;
	u32 rxid : 2;
	u32 winfoi : 1;
	u32 port_ag : 4;
	u32 dscp : 8;		/* DSCP value */
} __packed;

struct hnat_winfo {
	u32 bssid : 6;		/* WiFi Bssidx */
	u32 wcid : 10;		/* WiFi wtable Idx */
} __packed;

#else
struct hnat_unbind_info_blk {
	u32 time_stamp : 8;
	u32 pcnt : 16; /* packet count */
	u32 preb : 1;
	u32 pkt_type : 3;
	u32 state : 2;
	u32 udp : 1;
	u32 sta : 1; /* static entry */
} __packed;

struct hnat_bind_info_blk {
	u32 time_stamp : 15;
	u32 ka : 1; /* keep alive */
	u32 vlan_layer : 3;
	u32 psn : 1; /* egress packet has PPPoE session */
	u32 vpm : 1; /* 0:ethertype remark, 1:0x8100(CR default) */
	u32 ps : 1; /* packet sampling */
	u32 cah : 1; /* cacheable flag */
	u32 rmt : 1; /* remove tunnel ip header (6rd/dslite only) */
	u32 ttl : 1;
	u32 pkt_type : 3;
	u32 state : 2;
	u32 udp : 1;
	u32 sta : 1; /* static entry */
} __packed;

struct hnat_info_blk2 {
	u32 qid : 4; /* QID in Qos Port */
	u32 fqos : 1; /* force to PSE QoS port */
	u32 dp : 3; /* force to PSE port x
		     * 0:PSE,1:GSW, 2:GMAC,4:PPE,5:QDMA,7=DROP
		     */
	u32 mcast : 1; /* multicast this packet to CPU */
	u32 pcpl : 1; /* OSBN */
	u32 mibf : 1; /* 0:off 1:on PPE MIB counter */
	u32 alen : 1; /* 0:post 1:pre packet length in accounting */
	u32 port_mg : 6; /* port meter group */
	u32 port_ag : 6; /* port account group */
	u32 dscp : 8; /* DSCP value */
} __packed;

struct hnat_winfo {
	u32 bssid : 6;		/* WiFi Bssidx */
	u32 wcid : 8;		/* WiFi wtable Idx */
	u32 rxid : 2;		/* WiFi Ring idx */
} __packed;
#endif

/* info blk2 for WHNAT */
struct hnat_info_blk2_whnat {
	u32 qid : 4; /* QID[3:0] in Qos Port */
	u32 fqos : 1; /* force to PSE QoS port */
	u32 dp : 3; /* force to PSE port x
		     * 0:PSE,1:GSW, 2:GMAC,4:PPE,5:QDMA,7=DROP
		     */
	u32 mcast : 1; /* multicast this packet to CPU */
	u32 pcpl : 1; /* OSBN */
	u32 mibf : 1; /* 0:off 1:on PPE MIB counter */
	u32 alen : 1; /* 0:post 1:pre packet length in accounting */
	u32 qid2 : 2; /* QID[5:4] in Qos Port */
	u32 resv : 2;
	u32 wdmaid : 1; /* 0:to pcie0 dev 1:to pcie1 dev */
	u32 winfoi : 1; /* 0:off 1:on Wi-Fi hwnat support */
	u32 port_ag : 6; /* port account group */
	u32 dscp : 8; /* DSCP value */
} __packed;

struct hnat_ipv4_hnapt {
	union {
		struct hnat_bind_info_blk bfib1;
		struct hnat_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 sip;
	u32 dip;
	u16 dport;
	u16 sport;
	union {
		struct hnat_info_blk2 iblk2;
		struct hnat_info_blk2_whnat iblk2w;
		u32 info_blk2;
	};
	u32 new_sip;
	u32 new_dip;
	u16 new_dport;
	u16 new_sport;
	u16 m_timestamp; /* For mcast*/
	u16 resv1;
	u32 resv2;
	u32 resv3 : 26;
	u32 act_dp : 6; /* UDF */
	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	union {
#if !defined(CONFIG_MEDIATEK_NETSYS_V2)
		struct hnat_winfo winfo;
#endif
		u16 vlan2;
	};
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	u16 minfo;
	struct hnat_winfo winfo;
#endif
} __packed;

struct hnat_ipv4_dslite {
	union {
		struct hnat_bind_info_blk bfib1;
		struct hnat_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 sip;
	u32 dip;
	u16 dport;
	u16 sport;

	u32 tunnel_sipv6_0;
	u32 tunnel_sipv6_1;
	u32 tunnel_sipv6_2;
	u32 tunnel_sipv6_3;

	u32 tunnel_dipv6_0;
	u32 tunnel_dipv6_1;
	u32 tunnel_dipv6_2;
	u32 tunnel_dipv6_3;

	u8 flow_lbl[3]; /* in order to consist with Linux kernel (should be 20bits) */
	u8 priority;    /* in order to consist with Linux kernel (should be 8bits) */
	u32 hop_limit : 8;
	u32 resv2 : 18;
	u32 act_dp : 6; /* UDF */

	union {
		struct hnat_info_blk2 iblk2;
		struct hnat_info_blk2_whnat iblk2w;
		u32 info_blk2;
	};

	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	union {
#if !defined(CONFIG_MEDIATEK_NETSYS_V2)
		struct hnat_winfo winfo;
#endif
		u16 vlan2;
	};
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	u16 minfo;
	struct hnat_winfo winfo;
	u32 new_sip;
        u32 new_dip;
        u16 new_dport;
        u16 new_sport;
#endif
} __packed;

struct hnat_ipv6_3t_route {
	union {
		struct hnat_bind_info_blk bfib1;
		struct hnat_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 ipv6_sip0;
	u32 ipv6_sip1;
	u32 ipv6_sip2;
	u32 ipv6_sip3;
	u32 ipv6_dip0;
	u32 ipv6_dip1;
	u32 ipv6_dip2;
	u32 ipv6_dip3;
	u32 prot : 8;
	u32 hph : 24; /* hash placeholder */

	u32 resv1;
	u32 resv2;
	u32 resv3;
	u32 resv4 : 26;
	u32 act_dp : 6; /* UDF */

	union {
		struct hnat_info_blk2 iblk2;
		struct hnat_info_blk2_whnat iblk2w;
		u32 info_blk2;
	};
	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	union {
#if !defined(CONFIG_MEDIATEK_NETSYS_V2)
		struct hnat_winfo winfo;
#endif
		u16 vlan2;
	};
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	u16 minfo;
	struct hnat_winfo winfo;
#endif
} __packed;

struct hnat_ipv6_5t_route {
	union {
		struct hnat_bind_info_blk bfib1;
		struct hnat_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 ipv6_sip0;
	u32 ipv6_sip1;
	u32 ipv6_sip2;
	u32 ipv6_sip3;
	u32 ipv6_dip0;
	u32 ipv6_dip1;
	u32 ipv6_dip2;
	u32 ipv6_dip3;
	u16 dport;
	u16 sport;

	u32 resv1;
	u32 resv2;
	u32 resv3;
	u32 resv4 : 26;
	u32 act_dp : 6; /* UDF */

	union {
		struct hnat_info_blk2 iblk2;
		struct hnat_info_blk2_whnat iblk2w;
		u32 info_blk2;
	};

	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	union {
#if !defined(CONFIG_MEDIATEK_NETSYS_V2)
		struct hnat_winfo winfo;
#endif
		u16 vlan2;
	};
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	u16 minfo;
	struct hnat_winfo winfo;
#endif
} __packed;

struct hnat_ipv6_6rd {
	union {
		struct hnat_bind_info_blk bfib1;
		struct hnat_unbind_info_blk udib1;
		u32 info_blk1;
	};
	u32 ipv6_sip0;
	u32 ipv6_sip1;
	u32 ipv6_sip2;
	u32 ipv6_sip3;
	u32 ipv6_dip0;
	u32 ipv6_dip1;
	u32 ipv6_dip2;
	u32 ipv6_dip3;
	u16 dport;
	u16 sport;

	u32 tunnel_sipv4;
	u32 tunnel_dipv4;
	u32 hdr_chksum : 16;
	u32 dscp : 8;
	u32 ttl : 8;
	u32 flag : 3;
	u32 resv1 : 13;
	u32 per_flow_6rd_id : 1;
	u32 resv2 : 9;
	u32 act_dp : 6; /* UDF */

	union {
		struct hnat_info_blk2 iblk2;
		struct hnat_info_blk2_whnat iblk2w;
		u32 info_blk2;
	};

	u16 vlan1;
	u16 etype;
	u32 dmac_hi;
	union {
#if !defined(CONFIG_MEDIATEK_NETSYS_V2)
		struct hnat_winfo winfo;
#endif
		u16 vlan2;
	};
	u16 dmac_lo;
	u32 smac_hi;
	u16 pppoe_id;
	u16 smac_lo;
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	u16 minfo;
	struct hnat_winfo winfo;
	u32 resv3;
        u32 resv4;
        u16 new_dport;
        u16 new_sport;
#endif
} __packed;

struct foe_entry {
	union {
		struct hnat_unbind_info_blk udib1;
		struct hnat_bind_info_blk bfib1;
		struct hnat_ipv4_hnapt ipv4_hnapt;
		struct hnat_ipv4_dslite ipv4_dslite;
		struct hnat_ipv6_3t_route ipv6_3t_route;
		struct hnat_ipv6_5t_route ipv6_5t_route;
		struct hnat_ipv6_6rd ipv6_6rd;
	};
};

/* If user wants to change default FOE entry number, both DEF_ETRY_NUM and
 * DEF_ETRY_NUM_CFG need to be modified.
 */
#define DEF_ETRY_NUM		16384
/* feasible values : 32768, 16384, 8192, 4096, 2048, 1024 */
#define DEF_ETRY_NUM_CFG	TABLE_16K
/* corresponding values : TABLE_32K, TABLE_16K, TABLE_8K, TABLE_4K, TABLE_2K,
 * TABLE_1K
 */
#define MAX_EXT_DEVS		(0x3fU)
#define MAX_IF_NUM		64

#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define MAX_PPE_NUM		2
#else
#define MAX_PPE_NUM		1
#endif
#define CFG_PPE_NUM		(hnat_priv->ppe_num)

struct mib_entry {
	u32 byt_cnt_l;
	u16 byt_cnt_h;
	u32 pkt_cnt_l;
	u8 pkt_cnt_h;
	u8 resv0;
	u32 resv1;
} __packed;

struct hnat_accounting {
	u64 bytes;
	u64 packets;
};

enum mtk_hnat_version {
	MTK_HNAT_V1 = 1, /* version 1: mt7621, mt7623 */
	MTK_HNAT_V2, /* version 2: mt7622 */
	MTK_HNAT_V3, /* version 3: mt7629 */
	MTK_HNAT_V4, /* version 4: mt7986 */
};

struct mtk_hnat_data {
	u8 num_of_sch;
	bool whnat;
	bool per_flow_accounting;
	bool mcast;
	enum mtk_hnat_version version;
};

struct mtk_hnat {
	struct device *dev;
	void __iomem *fe_base;
	void __iomem *ppe_base[MAX_PPE_NUM];
	struct foe_entry *foe_table_cpu[MAX_PPE_NUM];
	dma_addr_t foe_table_dev[MAX_PPE_NUM];
	u8 enable;
	u8 enable1;
	struct dentry *root;
	struct debugfs_regset32 *regset[MAX_PPE_NUM];

	struct mib_entry *foe_mib_cpu[MAX_PPE_NUM];
	dma_addr_t foe_mib_dev[MAX_PPE_NUM];
	struct hnat_accounting *acct[MAX_PPE_NUM];
	const struct mtk_hnat_data *data;

	/*devices we plays for*/
	char wan[IFNAMSIZ];
	char lan[IFNAMSIZ];
	char ppd[IFNAMSIZ];
	u16 lvid;
	u16 wvid;

	struct reset_control *rstc;

	u8 ppe_num;
	u8 gmac_num;
	u8 wan_dsa_port;
	struct ppe_mcast_table *pmcast;

	u32 foe_etry_num;
	u32 etry_num_cfg;
	struct net_device *g_ppdev;
	struct net_device *g_wandev;
	struct net_device *wifi_hook_if[MAX_IF_NUM];
	struct extdev_entry *ext_if[MAX_EXT_DEVS];
	struct timer_list hnat_sma_build_entry_timer;
	struct timer_list hnat_reset_timestamp_timer;
	struct timer_list hnat_mcast_check_timer;
	bool nf_stat_en;
};

struct extdev_entry {
	char name[IFNAMSIZ];
	struct net_device *dev;
};

struct tcpudphdr {
	__be16 src;
	__be16 dst;
};

enum FoeEntryState { INVALID = 0, UNBIND = 1, BIND = 2, FIN = 3 };

enum FoeIpAct {
	IPV4_HNAPT = 0,
	IPV4_HNAT = 1,
	IPV4_DSLITE = 3,
	IPV6_3T_ROUTE = 4,
	IPV6_5T_ROUTE = 5,
	IPV6_6RD = 7,
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
	IPV4_MAP_T = 8,
	IPV4_MAP_E = 9,
#else
	IPV4_MAP_T = 6,
	IPV4_MAP_E = 6,
#endif
};

/*--------------------------------------------------------------------------*/
/* Common Definition*/
/*--------------------------------------------------------------------------*/

#define HNAT_SW_VER   "1.1.0"
#define HASH_SEED_KEY 0x12345678

/*PPE_TB_CFG value*/
#define ENTRY_80B 1
#define ENTRY_64B 0
#define TABLE_1K 0
#define TABLE_2K 1
#define TABLE_4K 2
#define TABLE_8K 3
#define TABLE_16K 4
#define TABLE_32K 5
#define SMA_DROP 0 /* Drop the packet */
#define SMA_DROP2 1 /* Drop the packet */
#define SMA_ONLY_FWD_CPU 2 /* Only Forward to CPU */
#define SMA_FWD_CPU_BUILD_ENTRY 3 /* Forward to CPU and build new FOE entry */
#define HASH_MODE_0 0
#define HASH_MODE_1 1
#define HASH_MODE_2 2
#define HASH_MODE_3 3

/*PPE_FLOW_CFG*/
#define BIT_FUC_FOE BIT(2)
#define BIT_FMC_FOE BIT(1)
#define BIT_FBC_FOE BIT(0)
#define BIT_TCP_IP4F_NAT_EN BIT(6) 
#define BIT_UDP_IP4F_NAT_EN BIT(7) /*Enable IPv4 fragment + UDP packet NAT*/
#define BIT_IPV6_3T_ROUTE_EN BIT(8)
#define BIT_IPV6_5T_ROUTE_EN BIT(9)
#define BIT_IPV6_6RD_EN BIT(10)
#define BIT_IPV4_NAT_EN BIT(12)
#define BIT_IPV4_NAPT_EN BIT(13)
#define BIT_IPV4_DSL_EN BIT(14)
#define BIT_MIB_BUSY BIT(16)
#define BIT_IPV4_NAT_FRAG_EN BIT(17)
#define BIT_IPV4_HASH_GREK BIT(19)
#define BIT_IPV6_HASH_GREK BIT(20)
#define BIT_IPV4_MAPE_EN BIT(21)
#define BIT_IPV4_MAPT_EN BIT(22)

/*GDMA_FWD_CFG value*/
#define BITS_GDM_UFRC_P_PPE (NR_PPE0_PORT << 12)
#define BITS_GDM_BFRC_P_PPE (NR_PPE0_PORT << 8)
#define BITS_GDM_MFRC_P_PPE (NR_PPE0_PORT << 4)
#define BITS_GDM_OFRC_P_PPE (NR_PPE0_PORT << 0)
#define BITS_GDM_ALL_FRC_P_PPE                                              \
	(BITS_GDM_UFRC_P_PPE | BITS_GDM_BFRC_P_PPE | BITS_GDM_MFRC_P_PPE |  \
	 BITS_GDM_OFRC_P_PPE)

#define BITS_GDM_UFRC_P_CPU_PDMA (NR_PDMA_PORT << 12)
#define BITS_GDM_BFRC_P_CPU_PDMA (NR_PDMA_PORT << 8)
#define BITS_GDM_MFRC_P_CPU_PDMA (NR_PDMA_PORT << 4)
#define BITS_GDM_OFRC_P_CPU_PDMA (NR_PDMA_PORT << 0)
#define BITS_GDM_ALL_FRC_P_CPU_PDMA                                           \
	(BITS_GDM_UFRC_P_CPU_PDMA | BITS_GDM_BFRC_P_CPU_PDMA |               \
	 BITS_GDM_MFRC_P_CPU_PDMA | BITS_GDM_OFRC_P_CPU_PDMA)

#define BITS_GDM_UFRC_P_CPU_QDMA (NR_QDMA_PORT << 12)
#define BITS_GDM_BFRC_P_CPU_QDMA (NR_QDMA_PORT << 8)
#define BITS_GDM_MFRC_P_CPU_QDMA (NR_QDMA_PORT << 4)
#define BITS_GDM_OFRC_P_CPU_QDMA (NR_QDMA_PORT << 0)
#define BITS_GDM_ALL_FRC_P_CPU_QDMA                                           \
	(BITS_GDM_UFRC_P_CPU_QDMA | BITS_GDM_BFRC_P_CPU_QDMA |               \
	 BITS_GDM_MFRC_P_CPU_QDMA | BITS_GDM_OFRC_P_CPU_QDMA)

#define BITS_GDM_UFRC_P_DISCARD (NR_DISCARD << 12)
#define BITS_GDM_BFRC_P_DISCARD (NR_DISCARD << 8)
#define BITS_GDM_MFRC_P_DISCARD (NR_DISCARD << 4)
#define BITS_GDM_OFRC_P_DISCARD (NR_DISCARD << 0)
#define BITS_GDM_ALL_FRC_P_DISCARD                                            \
	(BITS_GDM_UFRC_P_DISCARD | BITS_GDM_BFRC_P_DISCARD |                 \
	 BITS_GDM_MFRC_P_DISCARD | BITS_GDM_OFRC_P_DISCARD)

#define hnat_is_enabled(hnat_priv) (hnat_priv->enable)
#define hnat_enabled(hnat_priv) (hnat_priv->enable = 1)
#define hnat_disabled(hnat_priv) (hnat_priv->enable = 0)
#define hnat_is_enabled1(hnat_priv) (hnat_priv->enable1)
#define hnat_enabled1(hnat_priv) (hnat_priv->enable1 = 1)
#define hnat_disabled1(hnat_priv) (hnat_priv->enable1 = 0)

#define entry_hnat_is_bound(e) (e->bfib1.state == BIND)
#define entry_hnat_state(e) (e->bfib1.state)

#define skb_hnat_is_hashed(skb)                                                \
	(skb_hnat_entry(skb) != 0x3fff && skb_hnat_entry(skb) < hnat_priv->foe_etry_num)
#define FROM_GE_LAN(skb) (skb_hnat_iface(skb) == FOE_MAGIC_GE_LAN)
#define FROM_GE_WAN(skb) (skb_hnat_iface(skb) == FOE_MAGIC_GE_WAN)
#define FROM_GE_PPD(skb) (skb_hnat_iface(skb) == FOE_MAGIC_GE_PPD)
#define FROM_GE_VIRTUAL(skb) (skb_hnat_iface(skb) == FOE_MAGIC_GE_VIRTUAL)
#define FROM_EXT(skb) (skb_hnat_iface(skb) == FOE_MAGIC_EXT)
#define FROM_WED(skb) ((skb_hnat_iface(skb) == FOE_MAGIC_WED0) ||		\
		       (skb_hnat_iface(skb) == FOE_MAGIC_WED1))
#define FOE_MAGIC_GE_LAN 0x1
#define FOE_MAGIC_GE_WAN 0x2
#define FOE_MAGIC_EXT 0x3
#define FOE_MAGIC_GE_VIRTUAL 0x4
#define FOE_MAGIC_GE_PPD 0x5
#define FOE_MAGIC_WED0 0x78
#define FOE_MAGIC_WED1 0x79
#define FOE_INVALID 0xf
#define index6b(i) (0x3fU - i)

#define IPV4_HNAPT 0
#define IPV4_HNAT 1
#define IP_FORMAT(addr)                                                        \
	(((unsigned char *)&addr)[3], ((unsigned char *)&addr)[2],              \
	((unsigned char *)&addr)[1], ((unsigned char *)&addr)[0])

/*PSE Ports*/
#define NR_PDMA_PORT 0
#define NR_GMAC1_PORT 1
#define NR_GMAC2_PORT 2
#if defined(CONFIG_MEDIATEK_NETSYS_V2)
#define NR_WHNAT_WDMA_PORT EINVAL
#define NR_PPE0_PORT 3
#define NR_PPE1_PORT 4
#else
#define NR_WHNAT_WDMA_PORT 3
#define NR_PPE0_PORT 4
#endif
#define NR_QDMA_PORT 5
#define NR_DISCARD 7
#define NR_WDMA0_PORT 8
#define NR_WDMA1_PORT 9
#define LAN_DEV_NAME hnat_priv->lan
#define IS_WAN(dev)                                                            \
	(!strncmp((dev)->name, hnat_priv->wan, strlen(hnat_priv->wan)))
#define IS_LAN(dev) (!strncmp(dev->name, LAN_DEV_NAME, strlen(LAN_DEV_NAME)))
#define IS_BR(dev) (!strncmp(dev->name, "br", 2))
#define IS_WHNAT(dev)								\
	((hnat_priv->data->whnat &&						\
	 (get_wifi_hook_if_index_from_dev(dev) != 0)) ? 1 : 0)
#define IS_EXT(dev) ((get_index_from_dev(dev) != 0) ? 1 : 0)
#define IS_PPD(dev) (!strcmp(dev->name, hnat_priv->ppd))
#define IS_IPV4_HNAPT(x) (((x)->bfib1.pkt_type == IPV4_HNAPT) ? 1 : 0)
#define IS_IPV4_HNAT(x) (((x)->bfib1.pkt_type == IPV4_HNAT) ? 1 : 0)
#define IS_IPV4_GRP(x) (IS_IPV4_HNAPT(x) | IS_IPV4_HNAT(x))
#define IS_IPV4_DSLITE(x) (((x)->bfib1.pkt_type == IPV4_DSLITE) ? 1 : 0)
#define IS_IPV4_MAPE(x) (((x)->bfib1.pkt_type == IPV4_MAP_E) ? 1 : 0)
#define IS_IPV4_MAPT(x) (((x)->bfib1.pkt_type == IPV4_MAP_T) ? 1 : 0)
#define IS_IPV6_3T_ROUTE(x) (((x)->bfib1.pkt_type == IPV6_3T_ROUTE) ? 1 : 0)
#define IS_IPV6_5T_ROUTE(x) (((x)->bfib1.pkt_type == IPV6_5T_ROUTE) ? 1 : 0)
#define IS_IPV6_6RD(x) (((x)->bfib1.pkt_type == IPV6_6RD) ? 1 : 0)
#define IS_IPV6_GRP(x)                                                         \
	(IS_IPV6_3T_ROUTE(x) | IS_IPV6_5T_ROUTE(x) | IS_IPV6_6RD(x) |          \
	 IS_IPV4_DSLITE(x) | IS_IPV4_MAPE(x) | IS_IPV4_MAPT(x))
#define IS_BOND_MODE (!strncmp(LAN_DEV_NAME, "bond", 4))
#define IS_GMAC1_MODE ((hnat_priv->gmac_num == 1) ? 1 : 0)
#define IS_HQOS_MODE (qos_toggle == 1)
#define IS_PPPQ_MODE (qos_toggle == 2)		/* Per Port Per Queue */
#define MAX_PPPQ_PORT_NUM	6

#define es(entry) (entry_state[entry->bfib1.state])
#define ei(entry, end) (hnat_priv->foe_etry_num - (int)(end - entry))
#define pt(entry) (packet_type[entry->ipv4_hnapt.bfib1.pkt_type])
#define ipv4_smac(mac, e)                                                      \
	({                                                                     \
		mac[0] = e->ipv4_hnapt.smac_hi[3];                             \
		mac[1] = e->ipv4_hnapt.smac_hi[2];                             \
		mac[2] = e->ipv4_hnapt.smac_hi[1];                             \
		mac[3] = e->ipv4_hnapt.smac_hi[0];                             \
		mac[4] = e->ipv4_hnapt.smac_lo[1];                             \
		mac[5] = e->ipv4_hnapt.smac_lo[0];                             \
	})
#define ipv4_dmac(mac, e)                                                      \
	({                                                                     \
		mac[0] = e->ipv4_hnapt.dmac_hi[3];                             \
		mac[1] = e->ipv4_hnapt.dmac_hi[2];                             \
		mac[2] = e->ipv4_hnapt.dmac_hi[1];                             \
		mac[3] = e->ipv4_hnapt.dmac_hi[0];                             \
		mac[4] = e->ipv4_hnapt.dmac_lo[1];                             \
		mac[5] = e->ipv4_hnapt.dmac_lo[0];                             \
	})

#define IS_DSA_LAN(dev) (!strncmp(dev->name, "lan", 3))
#define IS_DSA_WAN(dev) (!strncmp(dev->name, "wan", 3))
#define NONE_DSA_PORT 0xff
#define MAX_CRSN_NUM 32
#define IPV6_HDR_LEN 40

/*QDMA_PAGE value*/
#define NUM_OF_Q_PER_PAGE 16

/*IPv6 Header*/
#ifndef NEXTHDR_IPIP
#define NEXTHDR_IPIP 4
#endif

extern const struct of_device_id of_hnat_match[];
extern struct mtk_hnat *hnat_priv;

#if defined(CONFIG_NET_DSA_MT7530)
u32 hnat_dsa_fill_stag(const struct net_device *netdev,
		       struct foe_entry *entry,
		       struct flow_offload_hw_path *hw_path,
		       u16 eth_proto, int mape);

static inline bool hnat_dsa_is_enable(struct mtk_hnat *priv)
{
	return (priv->wan_dsa_port != NONE_DSA_PORT);
}
#else
static inline u32 hnat_dsa_fill_stag(const struct net_device *netdev,
				     struct foe_entry *entry,
				     struct flow_offload_hw_path *hw_path,
				     u16 eth_proto, int mape)
{
	return 0;
}

static inline bool hnat_dsa_is_enable(struct mtk_hnat *priv)
{
	return false;
}
#endif

void hnat_deinit_debugfs(struct mtk_hnat *h);
int hnat_init_debugfs(struct mtk_hnat *h);
int hnat_register_nf_hooks(void);
void hnat_unregister_nf_hooks(void);
int whnat_adjust_nf_hooks(void);
int mtk_hqos_ptype_cb(struct sk_buff *skb, struct net_device *dev,
		      struct packet_type *pt, struct net_device *unused);
extern int dbg_cpu_reason;
extern int debug_level;
extern int hook_toggle;
extern int mape_toggle;
extern int qos_toggle;

int ext_if_add(struct extdev_entry *ext_entry);
int ext_if_del(struct extdev_entry *ext_entry);
void cr_set_field(void __iomem *reg, u32 field, u32 val);
int mtk_sw_nat_hook_tx(struct sk_buff *skb, int gmac_no);
int mtk_sw_nat_hook_rx(struct sk_buff *skb);
void mtk_ppe_dev_register_hook(struct net_device *dev);
void mtk_ppe_dev_unregister_hook(struct net_device *dev);
int nf_hnat_netdevice_event(struct notifier_block *unused, unsigned long event,
			    void *ptr);
int nf_hnat_netevent_handler(struct notifier_block *unused, unsigned long event,
			     void *ptr);
uint32_t foe_dump_pkt(struct sk_buff *skb);
uint32_t hnat_cpu_reason_cnt(struct sk_buff *skb);
int hnat_enable_hook(void);
int hnat_disable_hook(void);
void hnat_cache_ebl(int enable);
void hnat_qos_shaper_ebl(u32 id, u32 enable);
void set_gmac_ppe_fwd(int gmac_no, int enable);
int entry_detail(u32 ppe_id, int index);
int entry_delete_by_mac(u8 *mac);
int entry_delete(u32 ppe_id, int index);
int hnat_warm_init(void);

struct hnat_accounting *hnat_get_count(struct mtk_hnat *h, u32 ppe_id,
				       u32 index, struct hnat_accounting *diff);

static inline u16 foe_timestamp(struct mtk_hnat *h)
{
	return (readl(hnat_priv->fe_base + 0x0010)) & 0xffff;
}
