/*SH0
*******************************************************************************
**                                                                           **
**         Copyright (c) 2012-2013 Quantenna Communications, Inc.            **
**                            All Rights Reserved                            **
**                                                                           **
**  File        : qvsp_data.h                                                **
**  Description : Video Stream Protection                                    **
**                                                                           **
*******************************************************************************
EH0*/

#ifndef __QTN_QVSP_DATA_H__
#define __QTN_QVSP_DATA_H__

#include <net80211/_ieee80211.h>
#include <qtn/qvsp_common.h>
#if defined(__KERNEL__)
#include "compat.h"
#include <linux/in6.h>
#elif defined(MUC_BUILD)
#else
#include <netinet/in.h>
#include <sys/param.h>
#define MSEC_PER_SEC			1000L
#endif

/*
 * Minimum rate at which to calculate node cost.
 * If throughput is not sufficiently high, only small aggregates get transmitted which gives an
 * artificially high and noisy node cost estimate.
 * The threshold should be lower than QVSP_CFG_STRM_DISABLED_MAX, so that node cost continues to
 * be updated when streams are disabled.
 */
#define QVSP_MIN_NODE_KBPS_UPDATE_DFLT	480
#define QVSP_STRM_DISABLED_MAX_DFLT	(QVSP_MIN_NODE_KBPS_UPDATE_DFLT + 20)

/* Log levels */
#define LL_0				0x00
#define LL_1				0x01
#define LL_2				0x02
#define LL_3				0x03
#define LL_4				0x04
#define LL_5				0x05
#define LL_6				0x06
#define LL_7				0x07
#define LL_8				0x08
#define LL_9				0x09

enum qvsp_ioctl_state {
	QVSP_STATE_FAT,
	QVSP_STATE_FAT_AVAIL,
	QVSP_STATE_FAT_INTF,
	QVSP_STATE_STRM_TOT,
	QVSP_STATE_STRM_QTN,
	QVSP_STATE_STRM_ENA,
	QVSP_STATE_STRM_DIS,
	QVSP_STATE_STRM_DMT,
	QVSP_STATE_READ_MAX,
	QVSP_STATE_RESET,
	QVSP_STATE_TEST_FAT,
	QVSP_STATE_ALL_MAX
};

#ifndef DOXYGEN_EXCLUDE

/*
 * Restrictions:
 *   this structure must be kept in sync with qvsp_cfg_params and qvspdoc_enumstr_cfg
 *   _AC[0-3] fields must follow the 'global' equivalent (for macro assumptions)
 */
enum qvsp_cfg_param_e {
	QVSP_CFG_ENABLED,
	QVSP_CFG_ENABLED_ALWAYS,
	QVSP_CFG_FAT_MIN,
	QVSP_CFG_FAT_MIN_SOFT,
	QVSP_CFG_FAT_MIN_SOFT_CONSEC,
	QVSP_CFG_FAT_MIN_SAFE,
	QVSP_CFG_FAT_MIN_CHECK_INTV,
	QVSP_CFG_FAT_MAX_SOFT,
	QVSP_CFG_FAT_MAX_SOFT_CONSEC,
	QVSP_CFG_FAT_MAX_SAFE,
	QVSP_CFG_FAT_MAX_CHECK_INTV,
	QVSP_CFG_NODE_DATA_MIN,
	QVSP_CFG_DISABLE_DEMOTE,
	QVSP_CFG_DISABLE_DEMOTE_FIX_FAT,
	QVSP_CFG_DISABLE_WAIT,
	QVSP_CFG_DISABLE_PER_EVENT_MAX,
	QVSP_CFG_ENABLE_WAIT,
	QVSP_CFG_ENABLE_PER_EVENT_MAX,
	QVSP_CFG_STRM_RMT_DIS_TCP,
	QVSP_CFG_STRM_RMT_DIS_UDP,
	QVSP_CFG_STRM_TPUT_MIN,
	QVSP_CFG_STRM_DISABLED_MAX,
	QVSP_CFG_STRM_ADPT_THROT,
	QVSP_CFG_STRM_ADPT_THROT_STEP,
	QVSP_CFG_STRM_ADPT_THROT_MARGIN,
	QVSP_CFG_STRM_TPUT_SMPL_MIN,
	QVSP_CFG_STRM_COST_RC_ADJUST,
	QVSP_CFG_STRM_MAX,
	QVSP_CFG_STRM_MAX_AC0,
	QVSP_CFG_STRM_MAX_AC1,
	QVSP_CFG_STRM_MAX_AC2,
	QVSP_CFG_STRM_MAX_AC3,
	QVSP_CFG_STRM_MIN,
	QVSP_CFG_STRM_MIN_AC0,
	QVSP_CFG_STRM_MIN_AC1,
	QVSP_CFG_STRM_MIN_AC2,
	QVSP_CFG_STRM_MIN_AC3,
	QVSP_CFG_STRM_TPUT_MAX_TCP,
	QVSP_CFG_STRM_TPUT_MAX_FIRST = QVSP_CFG_STRM_TPUT_MAX_TCP,
	QVSP_CFG_STRM_TPUT_MAX_TCP_AC0,
	QVSP_CFG_STRM_TPUT_MAX_TCP_AC1,
	QVSP_CFG_STRM_TPUT_MAX_TCP_AC2,
	QVSP_CFG_STRM_TPUT_MAX_TCP_AC3,
	QVSP_CFG_STRM_TPUT_MAX_UDP,
	QVSP_CFG_STRM_TPUT_MAX_UDP_AC0,
	QVSP_CFG_STRM_TPUT_MAX_UDP_AC1,
	QVSP_CFG_STRM_TPUT_MAX_UDP_AC2,
	QVSP_CFG_STRM_TPUT_MAX_UDP_AC3,
	QVSP_CFG_STRM_TPUT_MAX_LAST = QVSP_CFG_STRM_TPUT_MAX_UDP_AC3,
	QVSP_CFG_STRM_ENABLE_WAIT,
	QVSP_CFG_STRM_AGE_MAX,
	QVSP_CFG_AGE_CHK_INTV,
	QVSP_CFG_3RDPT_CTL,
	QVSP_CFG_3RDPT_LOCAL_THROT,
	QVSP_CFG_3RDPT_QTN,		/* treat qtn client as 3rd party client, debug use only */
	QVSP_CFG_BA_THROT_INTV,
	QVSP_CFG_BA_THROT_DUR_MIN,
	QVSP_CFG_BA_THROT_DUR_STEP,
	QVSP_CFG_BA_THROT_WINSIZE_MIN,
	QVSP_CFG_BA_THROT_WINSIZE_MAX,
	QVSP_CFG_WME_THROT_AC,
	QVSP_CFG_WME_THROT_AIFSN,
	QVSP_CFG_WME_THROT_ECWMIN,
	QVSP_CFG_WME_THROT_ECWMAX,
	QVSP_CFG_WME_THROT_TXOPLIMIT,
	QVSP_CFG_WME_THROT_THRSH_DISABLED,
	QVSP_CFG_WME_THROT_THRSH_VICTIM,
	QVSP_CFG_EVENT_LOG_LVL,
	QVSP_CFG_DEBUG_LOG_LVL,
	QVSP_CFG_MAX,
};

struct qvsp_cfg_param {
	const char	*name;
	const char	*desc;
	const char	*units;
	uint32_t	default_val;
	uint32_t	min_val;
	uint32_t	max_val;
};

#define QVSP_CFG_PARAMS {											\
	{ "enabled",		"QTM enabled",				"number",	0,	0,	1},	\
	{ "enabled_always",	"QTM enabled when no QTM peers",	"number",	0,	0,	1},	\
	{ "fat_min",		"Min free airtime",			"msps",		100,	1,	1000 },	\
	{ "fat_min_soft",	"Soft min free airtime",		"msps",		170,	1,	1000 },	\
	{ "fat_min_soft_consec","Consecutive soft min free airtime",	"number",	3,	1,	255 },	\
	{ "fat_min_safe",	"Safe min free airtime",		"msps",		200,	1,	1000 },	\
	{ "fat_min_check_intv",	"Oversubscription check interval",	"ms",		2000,	100,	60000 },\
	{ "fat_max_soft",	"Soft max free airtime",		"msps",		350,	1,	1000 },	\
	{ "fat_max_soft_consec","Consecutive soft max free airtime",	"number",	5,	1,	255 },	\
	{ "fat_max_safe",	"Safe max free airtime",		"msps",		250,	1,	1000 },	\
	{ "fat_max_check_intv",	"Undersubscription check interval",	"ms",		2000,	100,	86400000 },\
	{ "node_data_min",	"Min data for node cost update",	"Kbps",					\
								QVSP_MIN_NODE_KBPS_UPDATE_DFLT,	1,	10000 },\
	{ "disable_demote",	"Demote stream to disable",		"number",	1,	0,	1 },	\
	{ "disable_demote_fat_fix",	"Adjust FAT when demoting streams",\
									"number",	0,	0,	1 },	\
	{ "disable_wait",	"Min re-disable wait time",		"secs",		3,	1,	86400 },\
	{ "disable_event_max",	"Max streams disabled per event",	"number",	1,	1,	256 },	\
	{ "enable_wait",	"Min re-enable wait time",		"secs",		15,	1,	86400 },\
	{ "enable_event_max",	"Max streams enabled per event",	"number",	1,	1,	256 },	\
	{ "rmt_disable_tcp",	"Disable Rx TCP streams at STA",	"number",	1,	0,	1 },	\
	{ "rmt_disable_udp",	"Disable Rx UDP streams at STA",	"number",	1,	0,	1 },	\
	{ "strm_tput_min",	"Min throughput for a real stream",	"Kbps",		1000,	1,	10000 },\
	{ "strm_disabled_max",	"Max throughput when disabled",		"Kbps",					\
								QVSP_STRM_DISABLED_MAX_DFLT,	20,	10000 },\
	{ "strm_adpt_throt",	"Adaptive throttling enabled",		"number",	1,	0,	1 },	\
	{ "strm_adpt_throt_step",	"Adaptive throttling cost step",	\
									"percent",	40,	1,	100 },\
	{ "strm_adpt_throt_margin",	"Adaptive throttling margin",	\
									"Kbps",		10000,	0,	100000 },\
	{ "strm_tput_smpl_min",	"Min throughput sampling ms",		"ms",		20,	1,      1000 },\
	{ "strm_cost_rc_adjust","Adjust stream cost for rate change",	"number",	1,	0,      1 },\
	{ "strm_max",		"Max streams",				"cnt",		256,	1,	256 },	\
	{ "strm_max_ac0",	"Max streams for AC 0",			"cnt",		0,	0,	256 },	\
	{ "strm_max_ac1",	"Max streams for AC 1",			"cnt",		0,	0,	256 },	\
	{ "strm_max_ac2",	"Max streams for AC 2",			"cnt",		0,	0,	256 },	\
	{ "strm_max_ac3",	"Max streams for AC 3",			"cnt",		0,	0,	256 },	\
	{ "strm_min",		"Min streams",				"cnt",		1,	1,	1000 },	\
	{ "strm_min_ac0",	"Min streams for AC 0",			"cnt",		0,	1,	1000 },	\
	{ "strm_min_ac1",	"Min streams for AC 1",			"cnt",		0,	1,	1000 },	\
	{ "strm_min_ac2",	"Min streams for AC 2",			"cnt",		0,	1,	1000 },	\
	{ "strm_min_ac3",	"Min streams for AC 3",			"cnt",		0,	1,	1000 },	\
	{ "strm_tput_max_tcp",	"Max stream throughput for TCP",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_tcp_ac0","Max stream throughput for TCP AC 0",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_tcp_ac1","Max stream throughput for TCP AC 1",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_tcp_ac2","Max stream throughput for TCP AC 2",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_tcp_ac3","Max stream throughput for TCP AC 3",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_udp",	"Max stream throughput for UDP",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_udp_ac0","Max stream throughput for UDP AC 0",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_udp_ac1","Max stream throughput for UDP AC 1",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_udp_ac2","Max stream throughput for UDP AC 2",	"Mbps",		0,	0,	10000 },\
	{ "strm_tput_max_udp_ac3","Max stream throughput for UDP AC 3",	"Mbps",		0,	0,	10000 },\
	{ "strm_enable_wait",	"Min stream re-enable wait time",	"secs",		30,	1,	86400 },\
	{ "strm_age_max",	"Max stream age",			"secs",		5,	1,	86400 },\
	{ "age_check_intv",	"Age check interval",			"secs",		10,	1,	86400 },\
	{ "3rd_party_ctl",	"Enable 3rd party client control",	"number",	0,	0,	1 },	\
	{ "3rd_party_local_throt",	"Throttling 3rd party client packet also in local",\
									"number",	0,	0,	1 },	\
	{ "3rd_party_qtn",	"Treat qtn client as 3rd party client",	"number",	0,	0,	1 },	\
	{ "ba_throt_intv",	"BA throttling interval",		"ms",		1000,	0,	10000 },\
	{ "ba_throt_dur_min",	"BA throttling min duration",		"ms",		50,	0,	10000 },\
	{ "ba_throt_dur_step",	"BA throttling duration step",		"ms",		100,	50,	10000 },\
	{ "ba_throt_winsize_min",	"BA throttling min winsize",	"number",	1,	0,	256 },\
	{ "ba_throt_winsize_max",	"BA throttling max winsize",	"number",	16,	1,	256 },\
	{ "wme_throt_ac",	"WME throttling AC bitmap",		"number",	3,	0,	15 },\
	{ "wme_throt_aifsn",	"WME throttling AIFSN",			"number",	15,	0,	15 },\
	{ "wme_throt_ecwmin",	"WME throttling encoded cwmin",		"number",	14,	1,	14 },\
	{ "wme_throt_ecwmax",	"WME throttling encoded cwmax",		"number",	15,	1,	15 },\
	{ "wme_throt_txoplimit","WME throttling TXOP limit",		"number",	0,	0,	65535 },\
	{ "wme_throt_thrsh_disabled",	"WME throttling disabled stream cost threshold",\
									"number",	150,	0,	1000 },\
	{ "wme_throt_thrsh_victim",	"WME throttling victim stream cost threshold",\
									"number",	150,	0,	1000 },\
	{ "event_level",	"Event log level",			"number",	LL_0,	LL_0,	LL_9  },\
	{ "debug_level",	"Debug log level",			"number",	LL_3,	LL_0,	LL_9  },\
}

/* Must be in sync with call_qcsapi_vsp_if_desc */
enum qvsp_if_e {
	QVSP_IF_ETH_RX,
	QVSP_IF_QDRV_TX,
	QVSP_IF_QDRV_RX,
	QVSP_IF_PCIE_RX,
	QVSP_IF_MAX
};

#define QVSP_IF_DESCS	{		\
	"eth_rx",			\
	"qdrv_tx",			\
	"qdrv_rx",			\
	"pcie_rx",			\
	"invalid"			\
}

/*
 * These must be kept in sync with QVSP_STRM_THROT_DESCS and QVSP_STRM_THROT_DESCS_ABBR.
 */
enum qvsp_strm_throt_policy {
	QVSP_STRM_THROT_NONE = 0,
	QVSP_STRM_THROT_BINARY = 1,
	QVSP_STRM_THROT_ADPT = 2,
	QVSP_STRM_THROT_MAX,
};

#define QVSP_STRM_THROT_DESCS {		\
	"None",				\
	"Binary",			\
	"Adaptive",			\
}

#define QVSP_STRM_THROT_DESCS_ABBR {	\
	"N/A",				\
	"BIN",				\
	"ADP",				\
}

enum qvsp_rule_dir_e {
	QVSP_RULE_DIR_ANY,
	QVSP_RULE_DIR_TX,
	QVSP_RULE_DIR_RX,
};

#define QVSP_RULE_DIR_DESCS	{	\
	"Any",				\
	"Tx",				\
	"Rx",				\
}

enum qvsp_rule_param_e {
	QVSP_RULE_PARAM_DIR,
	QVSP_RULE_PARAM_VAPPRI,
	QVSP_RULE_PARAM_AC,
	QVSP_RULE_PARAM_PROTOCOL,
	QVSP_RULE_PARAM_TPUT_MIN,
	QVSP_RULE_PARAM_TPUT_MAX,
	QVSP_RULE_PARAM_COST_MIN,
	QVSP_RULE_PARAM_COST_MAX,
	QVSP_RULE_PARAM_ORDER,
	QVSP_RULE_PARAM_THROT_POLICY,
	QVSP_RULE_PARAM_DEMOTE,
	QVSP_RULE_PARAM_MAX,
};

struct qvsp_rule_param {
	const char	*name;
	const char	*desc;
	const char	*units;
	uint32_t	min_val;
	uint32_t	max_val;
};

#define QVSP_RULE_PARAMS	{								\
	{ "dir",	"Direction",		"val",		0,	2 },			\
	{ "vappri",	"VAP Priority",		"bitmap",	0x1,	0xf },			\
	{ "ac",		"Access Classes",	"bitmap",	0x1,	0xf },			\
	{ "protocol",	"IP protocol - TCP(6) or UDP(17)", "val", 6,	17 },			\
	{ "tp_min",	"Min throughput",	"Mbps",		1,	10000 },		\
	{ "tp_max",	"Max throughput",	"Mbps",		1,	10000 },		\
	{ "cost_min",	"Cost min",		"msps",		1,	1000 },			\
	{ "cost_max",	"Cost max",		"msps",		1,	1000 },			\
	{ "order",	"Match order",		"val",		0,	QVSP_RULE_ORDER_MAX - 1 },\
	{ "throt_policy",	"Throttling policy - binary(1) or adaptive(2)",                 \
						"val",		1,	QVSP_STRM_THROT_MAX - 1 },\
	{ "demote",	"Demote stream",        "val",		0,	1},                     \
}

/*
 * These must be kept in sync with QVSP_RULE_ORDER_DESCS and QVSP_RULE_ORDER_DESCS_ABBR.
 */
enum qvsp_rule_order_e {
	QVSP_RULE_ORDER_GREATEST_COST_NODE,
	QVSP_RULE_ORDER_LEAST_COST_NODE,
	QVSP_RULE_ORDER_GREATEST_NODE_INV_PHY_RATE,
	QVSP_RULE_ORDER_LEAST_NODE_INV_PHY_RATE,
	QVSP_RULE_ORDER_GREATEST_COST_STREAM,
	QVSP_RULE_ORDER_LEAST_COST_STREAM,
	QVSP_RULE_ORDER_NEWEST,
	QVSP_RULE_ORDER_OLDEST,
	QVSP_RULE_ORDER_LOWEST_TPUT,
	QVSP_RULE_ORDER_HIGHEST_TPUT,
	QVSP_RULE_ORDER_MAX
};

#define QVSP_RULE_ORDER_DESCS	{	\
	"greatest cost node first",	\
	"least cost node first",	\
	"greatest inverse PHY rate node first",	\
	"least inverse PHY rate node first",	\
	"greatest cost stream first",	\
	"least cost stream first",	\
	"newest first",			\
	"oldest first",			\
	"lowest throughput first",	\
	"highest throughput first",	\
}

#define QVSP_RULE_ORDER_DESCS_ABBR {    \
	"GCN",                          \
	"LCN",                          \
	"GIPR",                         \
	"LIPR",                         \
	"GCS",                          \
	"LCS",                          \
	"NS",                           \
	"OS",                           \
	"LT",                           \
	"HT",                           \
}

enum qvsp_strm_state_e {
	QVSP_STRM_STATE_NONE,
	QVSP_STRM_STATE_DISABLED,
	QVSP_STRM_STATE_LOW_TPUT,
	QVSP_STRM_STATE_PRE_ENABLED,
	QVSP_STRM_STATE_ENABLED,
	QVSP_STRM_STATE_DELETED,
	QVSP_STRM_STATE_MAX
};

enum qvsp_hairpin_e {
	QVSP_HAIRPIN_NONE,
	QVSP_HAIRPIN_UCAST,
	QVSP_HAIRPIN_MCAST,
};

#define QVSP_RULE_DIR_DESCS	{	\
	"Any",				\
	"Tx",				\
	"Rx",				\
}

/* This definition must be kept in sync with the qvsp_ext_s struct */
#define QVSP_INACTIVE_REASON	{	\
		"Config",		\
		"WDS",			\
		"CoC"			\
}

#define QVSP_3RDPT_STR		"3"

#ifndef MUC_BUILD

/** \addtogroup vsp_group
 *  @{
 */

/**
 * Defines a stream based on source and destination
 */
struct qvsp_hash_flds_ipv4 {
	/** IP source address */
	__be32			saddr;

	/** IP destination address */
	__be32			daddr;

	/** UDP/TCP source port */
	__be16			sport;

	/** UDP/TCP destination port */
	__be16			dport;
};

struct qvsp_hash_flds_ipv6 {
	/** IP source address */
	struct in6_addr		saddr;

	/** IP destination address */
	struct in6_addr		daddr;

	/** UDP/TCP source port */
	__be16			sport;

	/** UDP/TCP destination port */
	__be16			dport;
};

union qvsp_hash_flds {
	struct qvsp_hash_flds_ipv4	ipv4;
	struct qvsp_hash_flds_ipv6	ipv6;
};

/**
 * Whitelist definition. Passing streams are compared with
 * the stream defined in 'hflds', ANDed with netmasks
 */
struct qvsp_wl_flds {
	union qvsp_hash_flds	hflds;

	/** IP source CIDR bitcount */
	uint8_t			s_cidr_bits;

	/** IP destination CIDR bitcount */
	uint8_t			d_cidr_bits;

	/** IP version */
	uint8_t			ip_version;
};

/**
 * IPv4 whitelist tricks for netmask; store netmasks in the hashfield union
 */
static inline __be32 * qvsp_wl_ipv4_netmask_src(struct qvsp_wl_flds *wl)
{
	struct qvsp_hash_flds_ipv4 *ipv4 = &wl->hflds.ipv4;
	return (__be32 *)&ipv4[1];
}

static inline __be32 * qvsp_wl_ipv4_netmask_dst(struct qvsp_wl_flds *wl)
{
	return &(qvsp_wl_ipv4_netmask_src(wl))[1];
}

struct qvsp_rule_flds {
	uint32_t		param[QVSP_RULE_PARAM_MAX];
};

struct qvsp_strm_stats {
	unsigned long		first_ref;
	uint32_t		pkts;
	uint32_t		bytes;
	uint32_t		bytes_sent;
	uint32_t		pkts_sent;
};

struct qvsp_stats_if {
	uint32_t		strm_add;
	uint32_t		strm_none;
	uint32_t		pkt_chk;
	uint32_t		pkt_tcp;
	uint32_t		pkt_udp;
	uint32_t		pkt_other;
	uint32_t		pkt_ignore;
	uint32_t		pkt_sent;
	uint32_t		pkt_drop_throttle;
	uint32_t		pkt_drop_disabled;
	uint32_t		pkt_demoted;
	uint32_t		pkt_frag_found;
	uint32_t		pkt_frag_not_found;
};

struct qvsp_stats {
	uint32_t		is_qtm;		/* 0: VSP or 1: QTM */
	uint32_t		strm_enable;
	uint32_t		strm_disable;
	uint32_t		strm_disable_remote;
	uint32_t		strm_reenable;
	uint32_t		fat_over;
	uint32_t		fat_under;
	uint32_t		fat_chk_disable;
	uint32_t		fat_chk_reenable;
	uint32_t		fat_chk_squeeze;
	uint32_t		fat_chk_loosen;
	struct qvsp_stats_if	stats_if[QVSP_IF_MAX];
};

struct qvsp_strm_info {
	union qvsp_hash_flds	hash_flds;
	uint16_t		node_idx;
	uint8_t			node_mac[6];
	uint8_t			vap_pri;
	uint8_t			tid;
	uint16_t		hairpin_id;
	uint16_t		hairpin_type;
	uint8_t			ip_version;
	uint8_t			ip_proto;
	uint8_t			ac_in;
	uint8_t			ac_out;
	uint8_t			strm_state;
	uint8_t			disable_remote;
	uint8_t			is_3rdpt_udp_us;
	uint16_t		last_ref_secs;
	uint32_t		ni_inv_phy_rate;
	uint32_t		phy_rate_disabled;
	uint32_t		bytes_max;
	uint32_t		ni_cost;
	uint16_t		cost_current;
	uint16_t		cost_max;
	uint8_t			hash;
	uint8_t			dir;
	uint32_t                throt_policy;
	uint32_t                throt_rate;
	uint32_t                demote_rule;
	/* current state, might be different from demote_rule when recovering */
	uint32_t                demote_state;
	struct qvsp_strm_stats	prev_stats;
};

/** @}*/

#endif	/* MUC_BUILD */

/*
 * Convert kilobits (Kb) to bytes
 */
static __inline__ uint32_t
qvsp_kbit2b(uint32_t kbps)
{
	return kbps * 1000 / NBBY;
}

/*
 * Convert bytes to kilobits (Kb)
 */
static __inline__ uint32_t
qvsp_b2kbit(uint32_t bytes)
{
	return bytes * NBBY / 1000;
}

/*
 * Convert bytes over an interval into to kilobits per second
 */
static __inline__ uint32_t
qvsp_b2kbitps(uint32_t bytes, unsigned long interval)
{
	/* bytes * NBBY / 1000 / 1000 * interval */
	return bytes * NBBY / interval;
}

/*
 * Convert bytes to megabits (Mb)
 */
static __inline__ uint32_t
qvsp_b2mbit(uint32_t bytes)
{
	return bytes * NBBY / 1000000;
}

/*
 * Convert inverse PHY rate to PHY rate
 */
static __inline__ uint32_t
qvsp_inv2phy(uint32_t inv_phy)
{
	return 65536 / inv_phy;
}

/*
 * Convert faked IP addr to Node/Tid.
 * @ip is network/big endian.
 */
static __inline__ void
qvsp_fake_ip2nodetid(const uint32_t *ip, uint8_t *node, uint8_t *tid)
{
	*node = ((uint8_t*)ip)[2];
	*tid = ((uint8_t*)ip)[3];
}

#define QVSP_TID_FAKE_IP_VERSION	4
#define QVSP_TID_FAKE_IP_PROTO		IPPROTO_UDP

/*
 * Convert Node/Tid to faked IP addr
 * Returned IP addr is network/big endian.
 */
static __inline__ void
qvsp_fake_nodetid2ip(uint32_t *ip, const uint8_t node, const uint8_t tid)
{
	((uint8_t*)ip)[0] = 192;
	((uint8_t*)ip)[1] = 168;
	((uint8_t*)ip)[2] = node;
	((uint8_t*)ip)[3] = tid;
}

#ifndef NIPQUAD_FMT
#define NIPQUAD_FMT "%d.%d.%d.%d"
#endif

#ifndef NIPQUAD_LEN
#define NIPQUAD_LEN 15
#endif

#ifndef NIPQUAD
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#endif

#define QVSP_CFG_SHOW_ANYSTR	"Any"

#endif	/* DOXYGEN_EXCLUDE */

#endif	/* __QTN_QVSP_DATA_H__ */

