/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mlme.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	John Chang	2003-08-28		Created
	John Chang  2004-09-06      modified for RT2600

*/
#ifndef __MLME_H__
#define __MLME_H__

#include "rtmp_dot11.h"

#ifdef DOT11R_FT_SUPPORT
#include "common/link_list.h"
#include "ft_cmm.h"
#endif /* DOT11R_FT_SUPPORT */

#ifdef DOT11K_RRM_SUPPORT
#include "dot11r_ft.h"
#endif /* DOT11K_RRM_SUPPORT */

#include "security/owe_cmm.h"

/* Extern Variables */
extern UCHAR WPS_OUI[];
extern UCHAR RALINK_OUI[];

/* maximum supported capability information - */
/* ESS, IBSS, Privacy, Short Preamble, Spectrum mgmt, Short Slot */
#define SUPPORTED_CAPABILITY_INFO   0x0533

#define END_OF_ARGS                 -1
#define LFSR_MASK                   0x80000057
#define MLME_TASK_EXEC_INTV         100 /*200*/
#define LEAD_TIME                   5

#define MLME_TASK_EXEC_MULTIPLE       10  /*5*/       /* MLME_TASK_EXEC_MULTIPLE * MLME_TASK_EXEC_INTV = 1 sec */
#define REORDER_EXEC_INTV				100       /* 0.1 sec */
#define STA_KEEP_ALIVE_NOTIFY_L2	60


/*#define TBTT_PRELOAD_TIME         384        // usec. LomgPreamble + 24-byte at 1Mbps */

/* The definition of Radar detection duration region */
#define CE		0
#define FCC		1
#define JAP		2
#define JAP_W53	3
#define JAP_W56	4
#define CHN		5
#define MAX_RD_REGION 6
#if defined(CONFIG_MULTI_CHANNEL) || defined(DOT11Z_TDLS_SUPPORT)
#define BEACON_LOST_TIME           (12 * OS_HZ)    /* 2048 msec = 2 sec */
#else
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA) || \
defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA) || defined(MT7663_FPGA)
#define BEACON_LOST_TIME            (40 * OS_HZ)    /* 2048 msec = 2 sec */
#else
#define BEACON_LOST_TIME            (4 * OS_HZ)    /* 2048 msec = 2 sec */
#endif /* MT7603_FPGA */
#endif /* defined(CONFIG_MULTI_CHANNEL) || defined(DOT11Z_TDLS_SUPPORT) */

#define DLS_TIMEOUT                 1200      /* unit: msec */
#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA) || \
defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA) || defined(MT7663_FPGA)
#define AUTH_TIMEOUT                5000       /* unit: msec */
#define ASSOC_TIMEOUT               5000       /* unit: msec */
#else
#define AUTH_TIMEOUT                300       /* unit: msec */
#define ASSOC_TIMEOUT               300       /* unit: msec */
#endif /* MT7603_FPGA */

#if defined(MT7603_FPGA) || defined(MT7628_FPGA) || defined(MT7636_FPGA) || defined(MT7637_FPGA) || \
defined(MT7615_FPGA) || defined(MT7622_FPGA) || defined(P18_FPGA) || defined(MT7663_FPGA)
#define JOIN_TIMEOUT                2000        /* unit: msec */
#else
#define JOIN_TIMEOUT                2000        /* unit: msec */
#endif /* MT7603_FPGA */
#define SHORT_CHANNEL_TIME          90        /* unit: msec */
#define MIN_CHANNEL_TIME            110        /* unit: msec, for dual band scan */
#define MAX_CHANNEL_TIME            140       /* unit: msec, for single band scan */
#define FAST_ACTIVE_SCAN_TIME	    30		  /* Active scan waiting for probe response time */

#define AUTO_CHANNEL_SEL_TIMEOUT		400		/* uint: msec */
#define LINK_DOWN_TIMEOUT           20000      /* unit: msec */
#define AUTO_WAKEUP_TIMEOUT			70			/*unit: msec */

/* Note: RSSI_TO_DBM_OFFSET has been changed to variable for new RF (2004-0720). */
/* SHould not refer to this constant anymore */
/*#define RSSI_TO_DBM_OFFSET          120 // for RT2530 RSSI-115 = dBm */
#define RSSI_FOR_MID_TX_POWER       -55  /* -55 db is considered mid-distance */
#define RSSI_FOR_LOW_TX_POWER       -45  /* -45 db is considered very short distance and */
/* eligible to use a lower TX power */
#define RSSI_FOR_LOWEST_TX_POWER    -30
/*#define MID_TX_POWER_DELTA          0   // 0 db from full TX power upon mid-distance to AP */
#define LOW_TX_POWER_DELTA          6    /* -3 db from full TX power upon very short distance. 1 grade is 0.5 db */
#define LOWEST_TX_POWER_DELTA       16   /* -8 db from full TX power upon shortest distance. 1 grade is 0.5 db */

#define RSSI_TRIGGERED_UPON_BELOW_THRESHOLD     0
#define RSSI_TRIGGERED_UPON_EXCCEED_THRESHOLD   1
#define RSSI_THRESHOLD_FOR_ROAMING              25
#define RSSI_DELTA                              5

/* Channel Quality Indication */
#define CQI_IS_GOOD(cqi)            ((cqi) >= 50)
/*#define CQI_IS_FAIR(cqi)          (((cqi) >= 20) && ((cqi) < 50)) */
#define CQI_IS_POOR(cqi)            (cqi < 50)  /*(((cqi) >= 5) && ((cqi) < 20)) */
#define CQI_IS_BAD(cqi)             (cqi < 5)
#define CQI_IS_DEAD(cqi)            (cqi == 0)

/* weighting factor to calculate Channel quality, total should be 100% */
#define RSSI_WEIGHTING                   50
#define TX_WEIGHTING                     30
#define RX_WEIGHTING                     20


#define BSS_NOT_FOUND                    0xFFFFFFFF

#define MAX_LEN_OF_MLME_QUEUE            256


enum SCAN_MODE {
	/* Active scan, send probe request, and wait beacon and probe response */
	SCAN_ACTIVE = 0x00,			/* all channels */
	SCAN_CISCO_ACTIVE = 0x1,	/* single channel only */
	FAST_SCAN_ACTIVE = 0x2,
#ifdef WSC_INCLUDED
	SCAN_WSC_ACTIVE = 0x3,
#endif /* WSC_INCLUDED */
#ifdef DOT11N_DRAFT3
	SCAN_2040_BSS_COEXIST = 0x4,
#endif /* DOT11N_DRAFT3 */
	SCAN_ACTIVE_MAX,

	/* Passive scan, no probe request, only wait beacon and probe response */
	SCAN_PASSIVE = 0x80,		/* all channels */
	SCAN_CISCO_PASSIVE = 0x81,	/* single channel only */
	SCAN_CISCO_NOISE = 0x82,	/* single channel only, for noise histogram collection */
	SCAN_CISCO_CHANNEL_LOAD = 0x83,	/* single channel only, for channel load collection */
	SCAN_PASSIVE_MAX,

	SCAN_TYPE_MAX
};

#define SCAN_MASK	0x80
#define SCAN_MODE_ACT(_x)	(((_x) & SCAN_MASK) == 0)
#define SCAN_MODE_PSV(_x)	(((_x) & SCAN_MASK) == SCAN_MASK)
#define SCAN_MODE_VALID(_x)	((SCAN_MODE_ACT(_x) && ((_x) < SCAN_ACTIVE_MAX)) || \
							 (SCAN_MODE_PSV(_x) && ((_x) < SCAN_PASSIVE_MAX)))

/*#define BSS_TABLE_EMPTY(x)             ((x).BssNr == 0) */
#define MAC_ADDR_IS_GROUP(Addr)       (((Addr[0]) & 0x01))
#define MAC_ADDR_HASH(Addr)            (Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define MAC_ADDR_HASH_INDEX(Addr)      (MAC_ADDR_HASH(Addr) & (HASH_TABLE_SIZE - 1))
#define TID_MAC_HASH(Addr, TID)            (TID^Addr[0] ^ Addr[1] ^ Addr[2] ^ Addr[3] ^ Addr[4] ^ Addr[5])
#define TID_MAC_HASH_INDEX(Addr, TID)      (TID_MAC_HASH(Addr, TID) & (HASH_TABLE_SIZE - 1))


/* bit definition of the 2-byte pBEACON->Capability field */
#define CAP_IS_ESS_ON(x)                 (((x) & 0x0001) != 0)
#define CAP_IS_IBSS_ON(x)                (((x) & 0x0002) != 0)
#define CAP_IS_CF_POLLABLE_ON(x)         (((x) & 0x0004) != 0)
#define CAP_IS_CF_POLL_REQ_ON(x)         (((x) & 0x0008) != 0)
#define CAP_IS_PRIVACY_ON(x)             (((x) & 0x0010) != 0)
#define CAP_IS_SHORT_PREAMBLE_ON(x)      (((x) & 0x0020) != 0)
#define CAP_IS_PBCC_ON(x)                (((x) & 0x0040) != 0)
#define CAP_IS_AGILITY_ON(x)             (((x) & 0x0080) != 0)
#define CAP_IS_SPECTRUM_MGMT(x)          (((x) & 0x0100) != 0)  /* 802.11e d9 */
#define CAP_IS_QOS(x)                    (((x) & 0x0200) != 0)  /* 802.11e d9 */
#define CAP_IS_SHORT_SLOT(x)             (((x) & 0x0400) != 0)
#define CAP_IS_APSD(x)                   (((x) & 0x0800) != 0)  /* 802.11e d9 */
#define CAP_IS_IMMED_BA(x)               (((x) & 0x1000) != 0)  /* 802.11e d9 */
#define CAP_IS_DSSS_OFDM(x)              (((x) & 0x2000) != 0)
#define CAP_IS_DELAY_BA(x)               (((x) & 0x4000) != 0)  /* 802.11e d9 */

#define CAP_GENERATE(ess, ibss, priv, s_pre, s_slot, spectrum)  (((ess) ? 0x0001 : 0x0000) | ((ibss) ? 0x0002 : 0x0000) | ((priv) ? 0x0010 : 0x0000) | ((s_pre) ? 0x0020 : 0x0000) | ((s_slot) ? 0x0400 : 0x0000) | ((spectrum) ? 0x0100 : 0x0000))

/*#define STA_QOS_CAPABILITY               0 // 1-byte. see 802.11e d9.0 for bit definition */

#define ERP_IS_NON_ERP_PRESENT(x)        (((x) & 0x01) != 0)    /* 802.11g */
#define ERP_IS_USE_PROTECTION(x)         (((x) & 0x02) != 0)    /* 802.11g */
#define ERP_IS_USE_BARKER_PREAMBLE(x)    (((x) & 0x04) != 0)    /* 802.11g */

#define BA_NOTUSE	2
/*BA Policy subfiled value in ADDBA frame */
#define IMMED_BA	1
#define DELAY_BA	0

/* BA Initiator subfield in DELBA frame */
#define ORIGINATOR	1
#define RECIPIENT	0

/* ADDBA Status Code */
#define ADDBA_RESULTCODE_SUCCESS					0
#define ADDBA_RESULTCODE_REFUSED					37
#define ADDBA_RESULTCODE_INVALID_PARAMETERS			38

/* DELBA Reason Code */
#define DELBA_REASONCODE_QSTA_LEAVING				36
#define DELBA_REASONCODE_END_BA						37
#define DELBA_REASONCODE_UNKNOWN_BA					38
#define DELBA_REASONCODE_TIMEOUT					39

/* reset all OneSecTx counters */
#define RESET_ONE_SEC_TX_CNT(__pEntry) \
	do {\
		if (((__pEntry)) != NULL) {	\
			(__pEntry)->OneSecTxRetryOkCount = 0; \
			(__pEntry)->OneSecTxFailCount = 0; \
			(__pEntry)->OneSecTxNoRetryOkCount = 0; \
			(__pEntry)->OneSecRxLGICount = 0; \
			(__pEntry)->OneSecRxSGICount = 0; \
		}	\
	} while (0)


/*
	802.11 frame formats
*/


typedef struct  _TRIGGER_EVENTA {
	BOOLEAN			bValid;
	UCHAR	BSSID[6];
	UCHAR	RegClass;	/* Regulatory Class */
	USHORT	Channel;
} TRIGGER_EVENTA, *PTRIGGER_EVENTA;


/* 20/40 trigger event table */
/* If one Event A delete or created, or if Event B is detected or not detected, STA should send 2040BSSCoexistence to AP. */
#define MAX_TRIGGER_EVENT		64
typedef struct  _TRIGGER_EVENT_TAB {
	UCHAR	EventANo;
	TRIGGER_EVENTA	EventA[MAX_TRIGGER_EVENT];
	ULONG			EventBCountDown;	/* Count down counter for Event B. */
} TRIGGER_EVENT_TAB, *PTRIGGER_EVENT_TAB;


/*
	Extended capabilities information IE( ID = 127 = IE_EXT_CAPABILITY)

*/
typedef struct GNU_PACKED _EXT_CAP_INFO_ELEMENT {
#ifdef RT_BIG_ENDIAN
	UINT32	interworking:1;
	UINT32	TDLSChSwitchSupport:1; /* bit30: TDLS Channel Switching */
	UINT32	TDLSPeerPSMSupport:1; /* bit29: TDLS Peer PSM Support */
	UINT32	UAPSDBufSTASupport:1; /* bit28: Peer U-APSD Buffer STA Support */
	UINT32	utc_tsf_offset:1;
	UINT32	DMSSupport:1;
	UINT32	ssid_list:1;
	UINT32	channel_usage:1;
	UINT32	timing_measurement:1;
	UINT32	mbssid:1;
	UINT32	ac_sta_cnt:1;
	UINT32	qos_traffic_cap:1;
	UINT32	BssTransitionManmt:1;
	UINT32	tim_bcast:1;
	UINT32	WNMSleepSupport:1;/*bit 17*/
	UINT32	TFSSupport:1;/*bit 16*/
	UINT32	geospatial_location:1;
	UINT32	civic_location:1;
	UINT32	collocated_interference_report:1;
	UINT32	proxy_arp:1;
	UINT32	FMSSupport:1;/*bit 11*/
	UINT32	location_tracking:1;
	UINT32	mcast_diagnostics:1;
	UINT32	diagnostics:1;
	UINT32	event:1;
	UINT32	s_psmp_support:1;
	UINT32	rsv5:1;
	UINT32	psmp_cap:1;
	UINT32	rsv3:1;
	UINT32	ExtendChannelSwitch:1;
	UINT32	rsv1:1;
	UINT32	BssCoexistMgmtSupport:1;
#else
	UINT32	BssCoexistMgmtSupport:1;
	UINT32	rsv1:1;
	UINT32	ExtendChannelSwitch:1;
	UINT32	rsv3:1;
	UINT32	psmp_cap:1;
	UINT32	rsv5:1;
	UINT32	s_psmp_support:1;
	UINT32	event:1;
	UINT32	diagnostics:1;
	UINT32	mcast_diagnostics:1;
	UINT32	location_tracking:1;
	UINT32	FMSSupport:1;/*bit 11*/
	UINT32	proxy_arp:1;
	UINT32	collocated_interference_report:1;
	UINT32	civic_location:1;
	UINT32	geospatial_location:1;
	UINT32	TFSSupport:1;/*bit 16*/
	UINT32	WNMSleepSupport:1;/*bit 17*/
	UINT32	tim_bcast:1;
	UINT32	BssTransitionManmt:1;
	UINT32	qos_traffic_cap:1;
	UINT32	ac_sta_cnt:1;
	UINT32	mbssid:1;
	UINT32	timing_measurement:1;
	UINT32	channel_usage:1;
	UINT32	ssid_list:1;
	UINT32	DMSSupport:1;
	UINT32	utc_tsf_offset:1;
	UINT32	UAPSDBufSTASupport:1; /* bit28: Peer U-APSD Buffer STA Support */
	UINT32	TDLSPeerPSMSupport:1; /* bit29: TDLS Peer PSM Support */
	UINT32	TDLSChSwitchSupport:1; /* bit30: TDLS Channel Switching */
	UINT32	interworking:1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	UINT32 rsv63:1;
	UINT32 operating_mode_notification:1;
	UINT32 tdls_wider_bw:1;
	UINT32 rsv49:12;
	UINT32 utf8_ssid:1;
	UINT32 rsv47:1;
	UINT32 wnm_notification:1;
	UINT32 uapsd_coex:1;
	UINT32 id_location:1;
	UINT32 service_interval_granularity:3;
	UINT32 reject_unadmitted_frame:1;
	UINT32 TDLSChSwitchProhibited:1; /* bit39: TDLS Channel Switching Prohibited */
	UINT32 TDLSProhibited:1; /* bit38: TDLS Prohibited */
	UINT32 TDLSSupport:1; /* bit37: TDLS Support */
	UINT32 msgcf_cap:1;
	UINT32 rsv35:1;
	UINT32 sspn_inf:1;
	UINT32 ebr:1;
	UINT32 qosmap:1;
#else
	UINT32 qosmap:1;
	UINT32 ebr:1;
	UINT32 sspn_inf:1;
	UINT32 rsv35:1;
	UINT32 msgcf_cap:1;
	UINT32 TDLSSupport:1; /* bit37: TDLS Support */
	UINT32 TDLSProhibited:1; /* bit38: TDLS Prohibited */
	UINT32 TDLSChSwitchProhibited:1; /* bit39: TDLS Channel Switching Prohibited */
	UINT32 reject_unadmitted_frame:1;
	UINT32 service_interval_granularity:3;
	UINT32 id_location:1;
	UINT32 uapsd_coex:1;
	UINT32 wnm_notification:1;
	UINT32 rsv47:1;
	UINT32 utf8_ssid:1;
	UINT32 rsv49:12;
	UINT32 tdls_wider_bw:1;
	UINT32 operating_mode_notification:1;
	UINT32 rsv63:1;
#endif /* RT_BIG_ENDIAN */

#ifdef RT_BIG_ENDIAN
	UINT8 ftm_init:1;	/* bit71: FTM Initiator in 802.11mc D4.0*/
	UINT8 ftm_resp:1;	/* bit70: FTM responder */
	UINT8 rsv69:1;
	UINT8 rsv68:1;
	UINT8 rsv67:1;
	UINT8 rsv66:1;
	UINT8 rsv65:1;
	UINT8 rsv64:1;
#else
	UINT8 rsv64:1;
	UINT8 rsv65:1;
	UINT8 rsv66:1;
	UINT8 rsv67:1;
	UINT8 rsv68:1;
	UINT8 rsv69:1;
	UINT8 ftm_resp:1;	/* bit70: FTM responder */
	UINT8 ftm_init:1;	/* bit71: FTM Initiator in 802.11mc D4.0*/
#endif /* RT_BIG_ENDIAN */
} EXT_CAP_INFO_ELEMENT, *PEXT_CAP_INFO_ELEMENT;

#define EXT_CAP_MIN_SAFE_LENGTH		8


/* 802.11n 7.3.2.61 */
typedef struct GNU_PACKED _BSS_2040_COEXIST_ELEMENT {
	UCHAR					ElementID;		/* ID = IE_2040_BSS_COEXIST = 72 */
	UCHAR					Len;
	BSS_2040_COEXIST_IE		BssCoexistIe;
} BSS_2040_COEXIST_ELEMENT, *PBSS_2040_COEXIST_ELEMENT;


/*802.11n 7.3.2.59 */
typedef struct GNU_PACKED _BSS_2040_INTOLERANT_CH_REPORT {
	UCHAR				ElementID;		/* ID = IE_2040_BSS_INTOLERANT_REPORT = 73 */
	UCHAR				Len;
	UCHAR				RegulatoryClass;
	UCHAR				ChList[0];
} BSS_2040_INTOLERANT_CH_REPORT, *PBSS_2040_INTOLERANT_CH_REPORT;


/* The structure for channel switch annoucement IE. This is in 802.11n D3.03 */
typedef struct GNU_PACKED _CHA_SWITCH_ANNOUNCE_IE {
	UCHAR			SwitchMode; /*channel switch mode */
	UCHAR			NewChannel;
	UCHAR			SwitchCount;
} CHA_SWITCH_ANNOUNCE_IE, *PCHA_SWITCH_ANNOUNCE_IE;


/* The structure for channel switch annoucement IE. This is in 802.11n D3.03 */
typedef struct GNU_PACKED _SEC_CHA_OFFSET_IE {
	UCHAR			SecondaryChannelOffset;	 /* 1: Secondary above, 3: Secondary below, 0: no Secondary */
} SEC_CHA_OFFSET_IE, *PSEC_CHA_OFFSET_IE;

/* 802.11mc D4.3 8.4.2.20 */
typedef struct GNU_PACKED _MEASUREMENT_REQ {
	UCHAR ID;
	UCHAR Length;
	UCHAR Token;
	UCHAR RequestMode;
	UCHAR Type;
} MEASUREMENT_REQ;

/* 802.11mc D4.3 8.4.2.21 */
typedef struct GNU_PACKED _MEASUREMENT_REPORT {
	UCHAR ID;
	UCHAR Length;
	UCHAR Token;
	UCHAR ReportMode;
	UCHAR Type;
} MEASUREMENT_REPORT;

/* 802.11mc D4.3 8.4.2.166 */
/* Length: 9 bytes */
typedef struct GNU_PACKED _FTM_PARAMETER {
	/* byte 0 */
	UCHAR status:2;
	UCHAR value:5;
	UCHAR rsv_1:1;
	/* byte 1 */
	UCHAR num_burst_exponent:4;
	UCHAR burst_duration:4;
	/* byte 2 */
	UCHAR min_delta_ftm;
	/* byte 3~4 */
	USHORT partial_tsf_timer;
	/* byte 5 */
	UCHAR tsf_no_preference:1;
	UCHAR asap_capable:1;
	UCHAR asap:1;
	UCHAR ftms_per_burst:5;
	/* byte 6 */
	UCHAR rsv_2:2;
	UCHAR ftm_format_and_bw:6;
	/* byte 7~8 */
	USHORT burst_period;	/* unit: 100 ms */
} FTM_PARAMETER, *PFTM_PARAMETER;

#ifdef FTM_SUPPORT
/* 802.11mc D4.3 8.4.2.166 */
typedef struct GNU_PACKED _FTM_PARM_IE {
	UCHAR ID;
	UCHAR Length;
	FTM_PARAMETER p;
} FTM_PARM_IE, *PFTM_PARM_IE;
#endif /* FTM_SUPPORT */

/* This structure is extracted from struct RT_HT_CAPABILITY and RT_VHT_CAP */
typedef struct _RT_PHY_INFO {
	BOOLEAN		bHtEnable;	 /* If we should use ht rate. */
	BOOLEAN		bPreNHt;	 /* If we should use ht rate. */
	/*Substract from HT Capability IE */
	UCHAR		MCSSet[16];
#ifdef DOT11_VHT_AC
	BOOLEAN	bVhtEnable;
	UCHAR		vht_bw;
	VHT_MCS_SET vht_mcs_set;
#endif /* DOT11_VHT_AC */
} RT_PHY_INFO;


#ifdef DOT11_VHT_AC
typedef struct _RT_VHT_CAP {
	UINT32 vht_bw:2;
	UINT32 vht_txstbc:1;
	UINT32 vht_rxstbc:3;
	UINT32 sgi_80m:1;
	UINT32 vht_htc:1;

	UINT32 vht_mcs_ss1:2;
	UINT32 vht_mcs_ss2:2;
	UINT32 vht_rx_rate:2;
	UINT32 vht_tx_rate:2;

	UINT32 rsv:16;
} RT_VHT_CAP;
#endif /* DOT11_VHT_AC */


/*
	This structure substracts ralink supports from all 802.11n-related features.
	Features not listed here but contained in 802.11n spec are not supported in rt2860
*/
typedef struct {
#ifdef RT_BIG_ENDIAN
	USHORT	rsv:6;
	USHORT	AmsduSize:1;	/* Max receiving A-MSDU size */
	USHORT	AmsduEnable:1;	/* Enable to transmit A-MSDU. Suggest disable. We should use A-MPDU to gain best benifit of 802.11n */
	USHORT	RxSTBC:2;
	USHORT	TxSTBC:1;
	USHORT	ShortGIfor40:1;	/*for40MHz */
	USHORT	ShortGIfor20:1;
	USHORT	GF:1;	/*green field */
	USHORT	MimoPs:2; /*mimo power safe MMPS_ */
#else
	USHORT	MimoPs:2; /*mimo power safe MMPS_ */
	USHORT	GF:1;	/*green field */
	USHORT	ShortGIfor20:1;
	USHORT	ShortGIfor40:1;	/*for 40MHz */
	USHORT	TxSTBC:1;	/* 0:not supported,  1:if supported */
	USHORT	RxSTBC:2;	/* 2 bits */
	USHORT	AmsduEnable:1;	/* Enable to transmit A-MSDU. Suggest disable. We should use A-MPDU to gain best benifit of 802.11n */
	USHORT	AmsduSize:1;	/* Max receiving A-MSDU size */
	USHORT	rsv:6;
#endif

	/*Substract from Addiont HT INFO IE */
#ifdef RT_BIG_ENDIAN
	UCHAR	ExtChanOffset:2;	/* Please not the difference with following	UCHAR	NewExtChannelOffset; from 802.11n */
	UCHAR	MpduDensity:3;
	UCHAR	MaxRAmpduFactor:2;
#else
	UCHAR	MaxRAmpduFactor:2;
	UCHAR	MpduDensity:3;
	UCHAR	ExtChanOffset:2;	/* Please not the difference with following	UCHAR	NewExtChannelOffset; from 802.11n */
#endif

#ifdef RT_BIG_ENDIAN
	USHORT	rsv2:11;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv3:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv3:1;
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11;
#endif

	/* New Extension Channel Offset IE */
	UCHAR	NewExtChannelOffset;
	/* Extension Capability IE = 127 */
	UCHAR	BSSCoexist2040;
} RT_HT_CAPABILITY, *PRT_HT_CAPABILITY;


typedef struct  GNU_PACKED _NEW_EXT_CHAN_IE {
	UCHAR				NewExtChanOffset;
} NEW_EXT_CHAN_IE, *PNEW_EXT_CHAN_IE;

typedef struct GNU_PACKED _FRAME_802_11 {
	HEADER_802_11   Hdr;
	UCHAR            Octet[1];
} FRAME_802_11, *PFRAME_802_11;

/* QoSNull embedding of management action. When HT Control MA field set to 1. */
typedef struct GNU_PACKED _MA_BODY {
	UCHAR            Category;
	UCHAR            Action;
	UCHAR            Octet[1];
} MA_BODY, *PMA_BODY;

typedef	struct GNU_PACKED _HEADER_802_3	{
	UCHAR           DAAddr1[MAC_ADDR_LEN];
	UCHAR           SAAddr2[MAC_ADDR_LEN];
	UCHAR           Octet[2];
} HEADER_802_3, *PHEADER_802_3;


/* Block ACK related format */
/* 2-byte BA Parameter  field  in	DELBA frames to terminate an already set up bA */
typedef struct GNU_PACKED _DELBA_PARM {
#ifdef RT_BIG_ENDIAN
	USHORT      TID:4;	/* value of TC os TS */
	USHORT      Initiator:1;	/* 1: originator    0:recipient */
	USHORT      Rsv:11;	/* always set to 0 */
#else
	USHORT      Rsv:11;	/* always set to 0 */
	USHORT      Initiator:1;	/* 1: originator    0:recipient */
	USHORT      TID:4;	/* value of TC os TS */
#endif /* RT_BIG_ENDIAN */
} DELBA_PARM, *PDELBA_PARM;

/* 2-byte BA Parameter Set field  in ADDBA frames to signal parm for setting up a BA */
typedef struct GNU_PACKED _BA_PARM {
#ifdef RT_BIG_ENDIAN
	USHORT      BufSize:10;	/* number of buffe of size 2304 octetsr */
	USHORT      TID:4;	/* value of TC os TS */
	USHORT      BAPolicy:1;	/* 1: immediately BA    0:delayed BA */
	USHORT      AMSDUSupported:1;	/* 0: not permitted		1: permitted */
#else
	USHORT      AMSDUSupported:1;	/* 0: not permitted		1: permitted */
	USHORT      BAPolicy:1;	/* 1: immediately BA    0:delayed BA */
	USHORT      TID:4;	/* value of TC os TS */
	USHORT      BufSize:10;	/* number of buffe of size 2304 octetsr */
#endif /* RT_BIG_ENDIAN */
} BA_PARM, *PBA_PARM;

/* 2-byte BA Starting Seq CONTROL field */
typedef union GNU_PACKED _BASEQ_CONTROL {
	struct GNU_PACKED {
#ifdef RT_BIG_ENDIAN
		USHORT      StartSeq:12; /* sequence number of the 1st MSDU for which this BAR is sent */
		USHORT      FragNum:4;	/* always set to 0 */
#else
		USHORT      FragNum:4;	/* always set to 0 */
		USHORT      StartSeq:12; /* sequence number of the 1st MSDU for which this BAR is sent */
#endif /* RT_BIG_ENDIAN */
	}   field;
	USHORT           word;
} BASEQ_CONTROL, *PBASEQ_CONTROL;

/*BAControl and BARControl are the same */
/* 2-byte BA CONTROL field in BA frame */
typedef struct GNU_PACKED _BA_CONTROL {
#ifdef RT_BIG_ENDIAN
	USHORT      TID:4;
	USHORT      Rsv:9;
	USHORT      Compressed:1;
	USHORT      MTID:1;		/*EWC V1.24 */
	USHORT      ACKPolicy:1; /* only related to N-Delayed BA. But not support in RT2860b. 0:NormalACK  1:No ACK */
#else
	USHORT      ACKPolicy:1; /* only related to N-Delayed BA. But not support in RT2860b. 0:NormalACK  1:No ACK */
	USHORT      MTID:1;		/*EWC V1.24 */
	USHORT      Compressed:1;
	USHORT      Rsv:9;
	USHORT      TID:4;
#endif /* RT_BIG_ENDIAN */
} BA_CONTROL, *PBA_CONTROL;

/* 2-byte BAR CONTROL field in BAR frame */
typedef struct GNU_PACKED _BAR_CONTROL {
#ifdef RT_BIG_ENDIAN
	USHORT      TID:4;
	USHORT      Rsv1:9;
	USHORT      Compressed:1;
	USHORT      MTID:1;		/*if this bit1, use  FRAME_MTBA_REQ,  if 0, use FRAME_BA_REQ */
	USHORT      ACKPolicy:1;
#else
	USHORT      ACKPolicy:1; /* 0:normal ack,  1:no ack. */
	USHORT      MTID:1;		/*if this bit1, use  FRAME_MTBA_REQ,  if 0, use FRAME_BA_REQ */
	USHORT      Compressed:1;
	USHORT      Rsv1:9;
	USHORT      TID:4;
#endif /* !RT_BIG_ENDIAN */
} BAR_CONTROL, *PBAR_CONTROL;

/* BARControl in MTBAR frame */
typedef struct GNU_PACKED _MTBAR_CONTROL {
#ifdef RT_BIG_ENDIAN
	USHORT      NumTID:4;
	USHORT      Rsv1:9;
	USHORT      Compressed:1;
	USHORT      MTID:1;
	USHORT      ACKPolicy:1;
#else
	USHORT      ACKPolicy:1;
	USHORT      MTID:1;
	USHORT      Compressed:1;
	USHORT      Rsv1:9;
	USHORT      NumTID:4;
#endif /* RT_BIG_ENDIAN */
} MTBAR_CONTROL, *PMTBAR_CONTROL;

typedef struct GNU_PACKED _PER_TID_INFO {
#ifdef RT_BIG_ENDIAN
	USHORT      TID:4;
	USHORT      Rsv1:12;
#else
	USHORT      Rsv1:12;
	USHORT      TID:4;
#endif /* RT_BIG_ENDIAN */
} PER_TID_INFO, *PPER_TID_INFO;

typedef struct {
	PER_TID_INFO      PerTID;
	BASEQ_CONTROL	 BAStartingSeq;
} EACH_TID, *PEACH_TID;


/* BAREQ AND MTBAREQ have the same subtype BAR, 802.11n BAR use compressed bitmap. */
typedef struct GNU_PACKED _FRAME_BA_REQ {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL  BARControl;
	BASEQ_CONTROL	 BAStartingSeq;
}   FRAME_BA_REQ, *PFRAME_BA_REQ;

typedef struct GNU_PACKED _FRAME_MTBA_REQ {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	MTBAR_CONTROL  MTBARControl;
	PER_TID_INFO	PerTIDInfo;
	BASEQ_CONTROL	 BAStartingSeq;
}   FRAME_MTBA_REQ, *PFRAME_MTBA_REQ;

/* Compressed format is mandantory in HT STA */
typedef struct GNU_PACKED _FRAME_MTBA {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BA_CONTROL  BAControl;
	BASEQ_CONTROL	 BAStartingSeq;
	UCHAR		BitMap[8];
}   FRAME_MTBA, *PFRAME_MTBA;

typedef struct GNU_PACKED _FRAME_SMPS_ACTION {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	smps;	/* 7.3.1.22 */
}   FRAME_SMPS_ACTION;

typedef struct GNU_PACKED _FRAME_ACTION_HDR {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
}   FRAME_ACTION_HDR, *PFRAME_ACTION_HDR;

/*Action Frame */
/*Action Frame  Category:Spectrum,  Action:Channel Switch. 7.3.2.20 */
typedef struct GNU_PACKED _CHAN_SWITCH_ANNOUNCE {
	UCHAR					ElementID;	/* ID = IE_CHANNEL_SWITCH_ANNOUNCEMENT = 37 */
	UCHAR					Len;
	CHA_SWITCH_ANNOUNCE_IE	CSAnnounceIe;
}   CHAN_SWITCH_ANNOUNCE, *PCHAN_SWITCH_ANNOUNCE;


/*802.11n : 7.3.2.20a */
typedef struct GNU_PACKED _SECOND_CHAN_OFFSET {
	UCHAR				ElementID;		/* ID = IE_SECONDARY_CH_OFFSET = 62 */
	UCHAR				Len;
	SEC_CHA_OFFSET_IE	SecChOffsetIe;
}   SECOND_CHAN_OFFSET, *PSECOND_CHAN_OFFSET;


typedef struct GNU_PACKED _FRAME_SPETRUM_CS {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	CHAN_SWITCH_ANNOUNCE	CSAnnounce;
	SECOND_CHAN_OFFSET		SecondChannel;
}   FRAME_SPETRUM_CS, *PFRAME_SPETRUM_CS;


typedef struct GNU_PACKED _FRAME_ADDBA_REQ {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;	/* 1 */
	BA_PARM		BaParm;	      /*  2 - 10 */
	USHORT		TimeOutValue;	/* 0 - 0 */
	BASEQ_CONTROL	BaStartSeq; /* 0-0 */
}   FRAME_ADDBA_REQ, *PFRAME_ADDBA_REQ;

typedef struct GNU_PACKED _FRAME_ADDBA_RSP {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;
	USHORT	StatusCode;
	BA_PARM		BaParm; /*0 - 2 */
	USHORT		TimeOutValue;
}   FRAME_ADDBA_RSP, *PFRAME_ADDBA_RSP;

typedef struct GNU_PACKED _FRAME_DELBA_REQ {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	DELBA_PARM		DelbaParm;
	USHORT	ReasonCode;
}   FRAME_DELBA_REQ, *PFRAME_DELBA_REQ;


/*7.2.1.7 */
typedef struct GNU_PACKED _FRAME_BAR {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL		BarControl;
	BASEQ_CONTROL	StartingSeq;
}   FRAME_BAR, *PFRAME_BAR;

/*7.2.1.7 */
typedef struct GNU_PACKED _FRAME_BA {
	FRAME_CONTROL   FC;
	USHORT          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	BAR_CONTROL		BarControl;
	BASEQ_CONTROL	StartingSeq;
	UCHAR		bitmask[8];
}   FRAME_BA, *PFRAME_BA;


/* Radio Measuement Request Frame Format */
typedef struct GNU_PACKED _FRAME_RM_REQ_ACTION {
	HEADER_802_11   Hdr;
	UCHAR	Category;
	UCHAR	Action;
	UCHAR	Token;
	USHORT	Repetition;
	UCHAR   data[0];
}   FRAME_RM_REQ_ACTION, *PFRAME_RM_REQ_ACTION;

typedef struct GNU_PACKED _HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE {
	UCHAR		ID;
	UCHAR		Length;
	UCHAR		ChannelSwitchMode;
	UCHAR		NewRegClass;
	UCHAR		NewChannelNum;
	UCHAR		ChannelSwitchCount;
} HT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE, *PHT_EXT_CHANNEL_SWITCH_ANNOUNCEMENT_IE;


/* */
/* _Limit must be the 2**n - 1 */
/* _SEQ1 , _SEQ2 must be within 0 ~ _Limit */
/* */
#define SEQ_STEPONE(_SEQ1, _SEQ2, _Limit)	((_SEQ1 == ((_SEQ2+1) & _Limit)))
#define SEQ_SMALLER(_SEQ1, _SEQ2, _Limit)	(((_SEQ1-_SEQ2) & ((_Limit+1)>>1)))
#define SEQ_LARGER(_SEQ1, _SEQ2, _Limit)	((_SEQ1 != _SEQ2) && !(((_SEQ1-_SEQ2) & ((_Limit+1)>>1))))
#define SEQ_WITHIN_WIN(_SEQ1, _SEQ2, _WIN, _Limit) (SEQ_LARGER(_SEQ1, _SEQ2, _Limit) &&  \
		SEQ_SMALLER(_SEQ1, ((_SEQ2+_WIN+1)&_Limit), _Limit))

/*
	Contention-free parameter (without ID and Length)
*/
typedef struct GNU_PACKED _CF_PARM {
	BOOLEAN     bValid;         /* 1: variable contains valid value */
	UCHAR       CfpCount;
	UCHAR       CfpPeriod;
	USHORT      CfpMaxDuration;
	USHORT      CfpDurRemaining;
} CF_PARM, *PCF_PARM;


typedef	struct _CIPHER_SUITE {
	NDIS_802_11_ENCRYPTION_STATUS	PairCipher;		/* Unicast cipher 1, this one has more secured cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	PairCipherAux;	/* Unicast cipher 2 if AP announce two unicast cipher suite */
	NDIS_802_11_ENCRYPTION_STATUS	GroupCipher;	/* Group cipher */
	USHORT							RsnCapability;	/* RSN capability from beacon */
	BOOLEAN							bMixMode;		/* Indicate Pair & Group cipher might be different */
}	CIPHER_SUITE, *PCIPHER_SUITE;


struct _vendor_ie_cap {
	ULONG ra_cap;
	ULONG mtk_cap;
	ULONG brcm_cap;
	BOOLEAN ldpc;
	BOOLEAN sgi;
	BOOLEAN is_rlt;
	BOOLEAN is_mtk;
	BOOLEAN is_brcm_etxbf_2G;
};

/* EDCA configuration from AP's BEACON/ProbeRsp */
#define WMM_AC_BK	0
#define WMM_AC_BE	1
#define WMM_AC_VI	2
#define WMM_AC_VO	3
typedef struct _EDCA_PARM {
	BOOLEAN     bValid;         /* 1: variable contains valid value */
	BOOLEAN     bAdd;         /* 1: variable contains valid value */
	BOOLEAN     bQAck;
	BOOLEAN     bQueueRequest;
	BOOLEAN     bTxopRequest;
	BOOLEAN     bAPSDCapable;
	/*  BOOLEAN     bMoreDataAck; */
	UCHAR       EdcaUpdateCount;
	UCHAR       Aifsn[WMM_NUM_OF_AC];       /* 0:AC_BK, 1:AC_BE, 2:AC_VI, 3:AC_VO */
	UCHAR       Cwmin[WMM_NUM_OF_AC];
	UCHAR       Cwmax[WMM_NUM_OF_AC];
	USHORT      Txop[WMM_NUM_OF_AC];      /* in unit of 32-us */
	BOOLEAN     bACM[WMM_NUM_OF_AC];      /* 1: Admission Control of AC_BK is mandattory */
	UCHAR       BandIdx;
	UCHAR	   WmmSet;
	UCHAR        RefCnt;
} EDCA_PARM, *PEDCA_PARM;

typedef struct _WMM_CFG {
	struct wifi_dev *wdev;
	struct _EDCA_PARM EdcaParm;
} WMM_CFG;

struct pbc_ctrl {
	struct wifi_dev *wdev;
	struct _MAC_TABLE_ENTRY *entry;
	UCHAR type;
};

struct rts_thld {
	struct wifi_dev *wdev;
	UCHAR pkt_thld;
	UINT32 len_thld;
};

enum _ac_type {
	AC_BK = (1 << WMM_AC_BK),
	AC_BE = (1 << WMM_AC_BE),
	AC_VI = (1 << WMM_AC_VI),
	AC_VO = (1 << WMM_AC_VO),
	AC_MSK = (AC_BK | AC_BE | AC_VI | AC_VO)
};

/* QBSS LOAD information from QAP's BEACON/ProbeRsp */
typedef struct {
	BOOLEAN     bValid;                     /* 1: variable contains valid value */
	USHORT      StaNum;
	UCHAR       ChannelUtilization;
	USHORT      RemainingAdmissionControl;  /* in unit of 32-us */
} QBSS_LOAD_PARM, *PQBSS_LOAD_PARM;


/* QBSS Info field in QSTA's assoc req */
typedef struct GNU_PACKED _QBSS_STA_INFO_PARM {
#ifdef RT_BIG_ENDIAN
	UCHAR		Rsv2:1;
	UCHAR		MaxSPLength:2;
	UCHAR		Rsv1:1;
	UCHAR		UAPSD_AC_BE:1;
	UCHAR		UAPSD_AC_BK:1;
	UCHAR		UAPSD_AC_VI:1;
	UCHAR		UAPSD_AC_VO:1;
#else
	UCHAR		UAPSD_AC_VO:1;
	UCHAR		UAPSD_AC_VI:1;
	UCHAR		UAPSD_AC_BK:1;
	UCHAR		UAPSD_AC_BE:1;
	UCHAR		Rsv1:1;
	UCHAR		MaxSPLength:2;
	UCHAR		Rsv2:1;
#endif /* RT_BIG_ENDIAN */
} QBSS_STA_INFO_PARM, *PQBSS_STA_INFO_PARM;

typedef struct {
	QBSS_STA_INFO_PARM	QosInfo;
	UCHAR	Rsv;
	UCHAR	Q_AC_BE[4];
	UCHAR	Q_AC_BK[4];
	UCHAR	Q_AC_VI[4];
	UCHAR	Q_AC_VO[4];
} QBSS_STA_EDCA_PARM, *PQBSS_STA_EDCA_PARM;

/* QBSS Info field in QAP's Beacon/ProbeRsp */
typedef struct GNU_PACKED _QBSS_AP_INFO_PARM {
#ifdef RT_BIG_ENDIAN
	UCHAR		UAPSD:1;
	UCHAR		Rsv:3;
	UCHAR		ParamSetCount:4;
#else
	UCHAR		ParamSetCount:4;
	UCHAR		Rsv:3;
	UCHAR		UAPSD:1;
#endif /* RT_BIG_ENDIAN */
} QBSS_AP_INFO_PARM, *PQBSS_AP_INFO_PARM;

/* QOS Capability reported in QAP's BEACON/ProbeRsp */
/* QOS Capability sent out in QSTA's AssociateReq/ReAssociateReq */
typedef struct {
	BOOLEAN     bValid;                     /* 1: variable contains valid value */
	BOOLEAN     bQAck;
	BOOLEAN     bQueueRequest;
	BOOLEAN     bTxopRequest;
	/*  BOOLEAN     bMoreDataAck; */
	UCHAR       EdcaUpdateCount;
} QOS_CAPABILITY_PARM, *PQOS_CAPABILITY_PARM;



typedef struct _BSS_ENTRY {
	UCHAR MacAddr[MAC_ADDR_LEN];
	UCHAR Bssid[MAC_ADDR_LEN];
	UCHAR Channel;
	UCHAR CentralChannel;	/*Store the wide-band central channel for 40MHz.  .used in 40MHz AP. Or this is the same as Channel. */
	ULONG ClientStatusFlags;
	UCHAR BssType;
	USHORT AtimWin;
	USHORT BeaconPeriod;

	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	UCHAR Erp;
	HT_CAPABILITY_IE HtCapability;
	UCHAR HtCapabilityLen;
	ADD_HT_INFO_IE AddHtInfo;	/* AP might use this additional ht info IE */
	UCHAR AddHtInfoLen;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;	/* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */
	UCHAR NewExtChanOffset;
	CHAR Rssi;

#ifdef CFG80211_SCAN_SIGNAL_AVG
	SHORT	AvgRssiX8;
	CHAR	AvgRssi;
#endif /* CFG80211_SCAN_SIGNAL_AVG */

#ifdef DOT11_VHT_AC
	UCHAR vht_cap_len;
	UCHAR vht_op_len;
	VHT_CAP_IE vht_cap_ie;
	VHT_OP_IE vht_op_ie;
#endif /* DOT11_VHT_AC */

	CHAR MinSNR;
	UCHAR Privacy;			/* Indicate security function ON/OFF. Don't mess up with auth mode. */
	UCHAR Hidden;

	BOOLEAN FromBcnReport; /* source from beacon report */
	USHORT DtimPeriod;
	USHORT CapabilityInfo;

	USHORT CfpCount;
	USHORT CfpPeriod;
	USHORT CfpMaxDuration;
	USHORT CfpDurRemaining;
	UCHAR SsidLen;
	CHAR Ssid[MAX_LEN_OF_SSID];

	UCHAR SameRxTimeCount;
	ULONG LastBeaconRxTimeA; /* OS's timestamp */
	ULONG LastBeaconRxTime; /* OS's timestamp */

	BOOLEAN bSES;

	UINT32 AKMMap;
	UINT32 PairwiseCipher;	/* Pairwise Key */
	UINT32 GroupCipher; /* Group Key */
	USHORT RsnCapability; /* RSN capability from beacon */

	/* New for microsoft WPA support */
	NDIS_802_11_FIXED_IEs FixIEs;
	NDIS_802_11_AUTHENTICATION_MODE AuthModeAux;	/* Addition mode for WPA2 / WPA capable AP */
	NDIS_802_11_AUTHENTICATION_MODE AuthMode;
	NDIS_802_11_WEP_STATUS	WepStatus;				/* Unicast Encryption Algorithm extract from VAR_IE */
	USHORT VarIELen;				/* Length of next VIE include EID & Length */
	UCHAR VarIEs[MAX_VIE_LEN];
	USHORT VarIeFromProbeRspLen;
	UCHAR *pVarIeFromProbRsp;
#ifdef DOT11W_PMF_SUPPORT
	BOOLEAN IsSupportSHA256KeyDerivation;
#endif /* DOT11W_PMF_SUPPORT */

	/* CCX Ckip information */
	UCHAR CkipFlag;

	/* CCX 2 TSF */
	UCHAR PTSF[4];		/* Parent TSF */
	UCHAR TTSF[8];		/* Target TSF */

	/* 802.11e d9, and WMM */
	EDCA_PARM EdcaParm;
	QOS_CAPABILITY_PARM QosCapability;
	QBSS_LOAD_PARM QbssLoad;

#ifdef WSC_INCLUDED
	UCHAR WpsAP;		/* 0x00: not support WPS, 0x01: support normal WPS, 0x02: support Ralink auto WPS, 0x04: support WAC AP */
	USHORT WscDPIDFromWpsAP;
#endif /* WSC_INCLUDED */



#if defined(DOT11R_FT_SUPPORT) || defined(DOT11K_RRM_SUPPORT)
	BOOLEAN	 bHasMDIE;
	FT_MDIE FT_MDIE;
#endif /* DOT11R_FT_SUPPORT || DOT11K_RRM_SUPPORT */


#ifdef DOT11K_RRM_SUPPORT
	UINT8 RegulatoryClass;
	UINT8 CondensedPhyType;
	UINT8 RSNI;
#endif /* DOT11K_RRM_SUPPORT */
#ifdef CONFIG_OWE_SUPPORT
	/* it can find a pairing OWE bss,
	 * hide this OPEN bss,
	 * skip to show it in scan result and skip to connect to it.
	 */
	BOOLEAN		hide_open_owe_bss;
#endif /*CONFIG_OWE_SUPPORT*/
} BSS_ENTRY;

typedef struct {
	UINT BssNr;
	UCHAR           BssOverlapNr;
	BSS_ENTRY       BssEntry[MAX_LEN_OF_BSS_TABLE];
} BSS_TABLE, *PBSS_TABLE;


struct raw_rssi_info {
	CHAR raw_rssi[4];
	UCHAR raw_snr;
	UCHAR Channel;
};

typedef struct _MLME_QUEUE_ELEM {
	UCHAR Msg[MAX_MGMT_PKT_LEN];	/* move here to fix alignment issue for ARM CPU */
	ULONG Machine;
	ULONG MsgType;
	ULONG MsgLen;
	LARGE_INTEGER TimeStamp;
	struct raw_rssi_info rssi_info;
	UCHAR Signal;
	UCHAR Channel;
	UCHAR Wcid;
	BOOLEAN Occupied;
	UCHAR OpMode;
	ULONG Priv;
	UCHAR RxPhyMode;
	struct wifi_dev *wdev;
} MLME_QUEUE_ELEM, *PMLME_QUEUE_ELEM;


typedef struct _MLME_QUEUE {
	ULONG Num;
	ULONG Head;
	ULONG Tail;
	NDIS_SPIN_LOCK Lock;
	MLME_QUEUE_ELEM Entry[MAX_LEN_OF_MLME_QUEUE];
} MLME_QUEUE, *PMLME_QUEUE;

typedef VOID(*STATE_MACHINE_FUNC)(VOID * pAd, MLME_QUEUE_ELEM * Elem);

typedef struct _STATE_MACHINE {
	ULONG				Base;
	ULONG				NrState;
	ULONG				NrMsg;
	ULONG				CurrState;
	STATE_MACHINE_FUNC	*TransFunc;
} STATE_MACHINE, *PSTATE_MACHINE;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
typedef VOID(*APCLI_STATE_MACHINE_FUNC)(VOID * pAd, MLME_QUEUE_ELEM * Elem, PULONG pCurrState, USHORT ifIndex);

typedef struct _STA_STATE_MACHINE {
	ULONG Base;
	ULONG NrState;
	ULONG NrMsg;
	ULONG CurrState;
	APCLI_STATE_MACHINE_FUNC *TransFunc;
} APCLI_STATE_MACHINE, *PSTA_STATE_MACHINE;
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

/* MLME AUX data structure that hold temporarliy settings during a connection attempt. */
/* Once this attemp succeeds, all settings will be copy to pAd->StaActive. */
/* A connection attempt (user set OID, roaming, CCX fast roaming,..) consists of */
/* several steps (JOIN, AUTH, ASSOC or REASSOC) and may fail at any step. We purposely */
/* separate this under-trial settings away from pAd->StaActive so that once */
/* this new attempt failed, driver can auto-recover back to the active settings. */
typedef struct _MLME_AUX {
	UCHAR               BssType;
	UCHAR               Ssid[MAX_LEN_OF_SSID];
	UCHAR               SsidLen;
	UCHAR               Bssid[MAC_ADDR_LEN];
	UCHAR			AutoReconnectSsid[MAX_LEN_OF_SSID];
	UCHAR			AutoReconnectSsidLen;
	USHORT			Alg;
	UCHAR			ScanType;
	UCHAR			Channel;
	UCHAR				OldChannel;
	UCHAR               CentralChannel;
#ifdef CONFIG_MULTI_CHANNEL
	UCHAR               InfraChannel;
#endif /* CONFIG_MULTI_CHANNEL */
	UCHAR			DtimPeriod;
	USHORT              Aid;
	USHORT              CapabilityInfo;
	USHORT              BeaconPeriod;
	USHORT              CfpMaxDuration;
	USHORT              CfpPeriod;
	USHORT              AtimWin;

	/* Copy supported rate from desired AP's beacon. We are trying to match */
	/* AP's supported and extended rate settings. */
	UCHAR		        SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		        ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR		        SupRateLen;
	UCHAR		        ExtRateLen;
	HT_CAPABILITY_IE		HtCapability;
	UCHAR				HtCapabilityLen;
	ADD_HT_INFO_IE		AddHtInfo;	/* AP might use this additional ht info IE */
	EXT_CAP_INFO_ELEMENT ExtCapInfo; /* this is the extened capibility IE appreed in MGMT frames. Doesn't need to update once set in Init. */
	UCHAR			NewExtChannelOffset;
	/*RT_HT_CAPABILITY	SupportedHtPhy; */

#ifdef DOT11_VHT_AC
	UCHAR vht_cap_len;
	UCHAR vht_op_len;
	VHT_CAP_IE vht_cap;
	VHT_OP_IE vht_op;
	UCHAR vht_cent_ch;
	BOOLEAN force_op_mode;
	OPERATING_MODE op_mode;
#endif /* DOT11_VHT_AC */

	/* new for QOS */
	QOS_CAPABILITY_PARM APQosCapability;    /* QOS capability of the current associated AP */
	EDCA_PARM           APEdcaParm;         /* EDCA parameters of the current associated AP */
	QBSS_LOAD_PARM      APQbssLoad;         /* QBSS load of the current associated AP */

	/* new to keep Ralink specific feature */
	struct _vendor_ie_cap vendor_ie;

	BSS_TABLE           SsidBssTab;     /* AP list for the same SSID */
	BSS_TABLE           RoamTab;        /* AP list eligible for roaming */
	ULONG               BssIdx;
	ULONG               RoamIdx;
	BOOLEAN				CurrReqIsFromNdis;

	RALINK_TIMER_STRUCT BeaconTimer, ScanTimer;
	RALINK_TIMER_STRUCT AuthTimer;
	RALINK_TIMER_STRUCT AssocTimer, ReassocTimer, DisassocTimer;

	TIMER_FUNC_CONTEXT  BeaconTimerFuncContext;
	TIMER_FUNC_CONTEXT  ScanTimerFuncContext;
	TIMER_FUNC_CONTEXT  AuthTimerFuncContext;
	TIMER_FUNC_CONTEXT  AssocTimerFuncContext;
	TIMER_FUNC_CONTEXT  ReassocTimerFuncContext;
	TIMER_FUNC_CONTEXT  DisassocTimerFuncContext;

#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
	USHORT              VarIELen;           /* Length of next VIE include EID & Length */
	UCHAR               VarIEs[MAX_VIE_LEN];
	LONG				Rssi; /* Record the rssi value when receive Probe Rsp. */
	RALINK_TIMER_STRUCT ProbeTimer, ApCliAssocTimer, ApCliAuthTimer;
	RALINK_TIMER_STRUCT WpaDisassocAndBlockAssocTimer;
#ifdef DOT11W_PMF_SUPPORT
	RSN_CAPABILITIES	RsnCap;
	BOOLEAN			IsSupportSHA256KeyDerivation;
#endif /* DOT11W_PMF_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


	UINT32 AKMMap;
	UINT32 PairwiseCipher;
	UINT32 GroupCipher;
	UINT32 IntegrityGroupCipher;
} MLME_AUX, *PMLME_AUX;


#ifdef DOT11_N_SUPPORT
/* StaActive.SupportedHtPhy.MCSSet is copied from AP beacon.  Don't need to update here. */
#define COPY_HTSETTINGS_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd, _pEntry, _pStaCfg)                                 \
	{                                                                                       \
		_pStaCfg->StaActive.SupportedHtPhy.MimoPs = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.MimoPs;      \
		_pStaCfg->StaActive.SupportedHtPhy.GF = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.GF;      \
		_pStaCfg->StaActive.SupportedHtPhy.ShortGIfor20 = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor20;      \
		_pStaCfg->StaActive.SupportedHtPhy.ShortGIfor40 = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.ShortGIfor40;      \
		_pStaCfg->StaActive.SupportedHtPhy.TxSTBC = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.TxSTBC;      \
		_pStaCfg->StaActive.SupportedHtPhy.RxSTBC = _pStaCfg->MlmeAux.HtCapability.HtCapInfo.RxSTBC;      \
		_pStaCfg->StaActive.SupportedHtPhy.ExtChanOffset = _pStaCfg->MlmeAux.AddHtInfo.AddHtInfo.ExtChanOffset;      \
		_pStaCfg->StaActive.SupportedHtPhy.OperaionMode = _pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.OperaionMode;      \
		_pStaCfg->StaActive.SupportedHtPhy.NonGfPresent = _pStaCfg->MlmeAux.AddHtInfo.AddHtInfo2.NonGfPresent;      \
		NdisMoveMemory((_pEntry)->HTCapability.MCSSet, (_pStaCfg)->StaActive.SupportedPhyInfo.MCSSet, sizeof(UCHAR) * 16);\
	}

#define COPY_AP_HTSETTINGS_FROM_BEACON(_pAd, _pHtCapability, _pEntry)                                 \
	{                                                                                       \
		_pEntry->AMsduSize = (UCHAR)(_pHtCapability->HtCapInfo.AMsduSize);	\
		_pEntry->MmpsMode = (UCHAR)(_pHtCapability->HtCapInfo.MimoPs);	\
		_pEntry->MaxRAmpduFactor = (UCHAR)(_pHtCapability->HtCapParm.MaxRAmpduFactor);	\
	}
#endif /* DOT11_N_SUPPORT */

#ifdef DOT11_VHT_AC
#define COPY_VHT_FROM_MLME_AUX_TO_ACTIVE_CFG(_pAd)                                 \
	{                                                                                       \
	}
#endif /* DOT11_VHT_AC */


typedef struct _MLME_ADDBA_REQ_STRUCT {
	UCHAR   Wcid;
	UCHAR   pAddr[MAC_ADDR_LEN];
	UCHAR   BaBufSize;
	USHORT	TimeOutValue;
	UCHAR   TID;
	UCHAR   Token;
	USHORT	BaStartSeq;
} MLME_ADDBA_REQ_STRUCT, *PMLME_ADDBA_REQ_STRUCT;


typedef struct _MLME_DELBA_REQ_STRUCT {
	UCHAR   Wcid;	/* */
	UCHAR     Addr[MAC_ADDR_LEN];
	UCHAR   TID;
	UCHAR	Initiator;
} MLME_DELBA_REQ_STRUCT, *PMLME_DELBA_REQ_STRUCT;

/* assoc struct is equal to reassoc */
typedef struct _MLME_ASSOC_REQ_STRUCT {
	UCHAR     Addr[MAC_ADDR_LEN];
	USHORT    CapabilityInfo;
	USHORT    ListenIntv;
	ULONG     Timeout;
} MLME_ASSOC_REQ_STRUCT, *PMLME_ASSOC_REQ_STRUCT, MLME_REASSOC_REQ_STRUCT, *PMLME_REASSOC_REQ_STRUCT;

typedef struct _MLME_DISASSOC_REQ_STRUCT {
	UCHAR     Addr[MAC_ADDR_LEN];
	USHORT    Reason;
} MLME_DISASSOC_REQ_STRUCT, *PMLME_DISASSOC_REQ_STRUCT;

typedef struct _MLME_AUTH_REQ_STRUCT {
	UCHAR        Addr[MAC_ADDR_LEN];
	USHORT       Alg;
	ULONG        Timeout;
#ifdef MAC_REPEATER_SUPPORT
	UCHAR	BssIdx;
	UCHAR	CliIdx;
#endif /* MAC_REPEATER_SUPPORT */
} MLME_AUTH_REQ_STRUCT, *PMLME_AUTH_REQ_STRUCT;

typedef struct _MLME_DEAUTH_REQ_STRUCT {
	UCHAR        Addr[MAC_ADDR_LEN];
	USHORT       Reason;
} MLME_DEAUTH_REQ_STRUCT, *PMLME_DEAUTH_REQ_STRUCT;
typedef struct _MLME_BROADCAST_DEAUTH_REQ_STRUCT {
    UCHAR        		Addr[MAC_ADDR_LEN];
    USHORT       		Reason;
	struct wifi_dev  	*wdev;
} MLME_BROADCAST_DEAUTH_REQ_STRUCT, *PMLME_BROADCAST_DEAUTH_REQ_STRUCT;
typedef struct {
	ULONG      BssIdx;
} MLME_JOIN_REQ_STRUCT;

typedef struct _MLME_SCAN_REQ_STRUCT {
	UCHAR      Bssid[MAC_ADDR_LEN];
	UCHAR      BssType;
	UCHAR      ScanType;
	UCHAR      SsidLen;
	CHAR       Ssid[MAX_LEN_OF_SSID];
} MLME_SCAN_REQ_STRUCT, *PMLME_SCAN_REQ_STRUCT;

typedef struct _MLME_START_REQ_STRUCT {
	CHAR        Ssid[MAX_LEN_OF_SSID];
	UCHAR       SsidLen;
} MLME_START_REQ_STRUCT, *PMLME_START_REQ_STRUCT;

/* Timing Measurement Frame REQ Format */
typedef struct GNU_PACKED _FRAME_TMR_REQ_ACTION {
	HEADER_802_11   Hdr;
	UCHAR   Category;
	UCHAR   Action;
	UCHAR   Trigger;
}   FRAME_TMR_REQ_ACTION, *PFRAME_TMR_REQ_ACTION;

/* Timing Measurement Frame Format */
typedef struct GNU_PACKED _FRAME_FTM_ACTION {
	HEADER_802_11   Hdr;
	UCHAR   Category;
	UCHAR   Action;
	UCHAR   DialogToken;
	UCHAR   FollowUpDialogToken;
	UCHAR   TOD[6];/*Unit 0.1ns */
	UCHAR   TOA[6];/*Unit 0.1ns */
	UCHAR   TodErr[2];
	UCHAR   ToaErr[2];
	/*TODO: three optional present IE. (LCI, LCivic, FTM IE)*/
}   FRAME_FTM_ACTION, *PFRAME_FTM_ACTION;

typedef struct GNU_PACKED _EID_STRUCT {
	UCHAR   Eid;
	UCHAR   Len;
	UCHAR   Octet[1];
} EID_STRUCT, *PEID_STRUCT, BEACON_EID_STRUCT, *PBEACON_EID_STRUCT;


/* ========================== AP mlme.h =============================== */
#define TBTT_PRELOAD_TIME       384        /* usec. LomgPreamble + 24-byte at 1Mbps */
#define DEFAULT_DTIM_PERIOD     1

/* weighting factor to calculate Channel quality, total should be 100% */
/*#define RSSI_WEIGHTING                   0 */
/*#define TX_WEIGHTING                     40 */
/*#define RX_WEIGHTING                     60 */

#define MAC_TABLE_AGEOUT_TIME			480			/* unit: sec */
#define MAC_TABLE_MIN_AGEOUT_TIME		60			/* unit: sec */
#define MAC_TABLE_ASSOC_TIMEOUT			5			/* unit: sec */
/* #define MAC_TABLE_FULL(Tab)				((Tab).size == MAX_LEN_OF_MAC_TABLE) */

/* AP shall drop the sta if contine Tx fail count reach it. */
#define MAC_ENTRY_LIFE_CHECK_CNT		1024			/* packet cnt. */

/* Value domain of pMacEntry->Sst */
typedef enum _Sst {
	SST_NOT_AUTH,   /* 0: equivalent to IEEE 802.11/1999 state 1 */
	SST_AUTH,       /* 1: equivalent to IEEE 802.11/1999 state 2 */
	SST_ASSOC       /* 2: equivalent to IEEE 802.11/1999 state 3 */
} SST;

/* value domain of pMacEntry->AuthState */
typedef enum _AuthState {
	AS_NOT_AUTH,
	AS_AUTH_OPEN,       /* STA has been authenticated using OPEN SYSTEM */
	AS_AUTH_KEY,        /* STA has been authenticated using SHARED KEY */
	AS_AUTHENTICATING,  /* STA is waiting for AUTH seq#3 using SHARED KEY */
	AS_AUTH_SAE,        /* STA has been authenticated using SAE */
} AUTH_STATE;


struct _build_ie_info {
	UCHAR *frame_buf;
	UCHAR frame_subtype;
	BOOLEAN g_band_256_qam;
	UCHAR channel;
	UCHAR phy_mode;
	BOOLEAN is_draft_n_type;
	struct wifi_dev *wdev;
};


struct _op_info {
	UINT8 bw;
	UINT8 cent_ch;
};


typedef struct _IE_lists {
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR ApAddr[MAC_ADDR_LEN];
	USHORT CapabilityInfo;
	USHORT ListenInterval;
	UCHAR SsidLen;
	UCHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SupportedRatesLen;
	UCHAR SupportedRates[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupportedChlLen;
	UCHAR SupportedChl[MAX_LEN_OF_SUPPORTED_CHL];
	UCHAR RSN_IE[MAX_LEN_OF_RSNIE];
	UCHAR RSNIE_Len;
	BOOLEAN bWmmCapable;
#if defined(WSC_AP_SUPPORT) || defined(RT_CFG80211_SUPPORT)
	BOOLEAN bWscCapable;
#endif /* defined(WSC_AP_SUPPORT) || defined(RT_CFG80211_SUPPORT) */
	struct _vendor_ie_cap vendor_ie;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
#ifdef DOT11R_FT_SUPPORT
	FT_INFO FtInfo;
#endif /* DOT11R_FT_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
	RRM_EN_CAP_IE RrmEnCap;
#endif /* DOT11K_RRM_SUPPORT */
	UCHAR ht_cap_len;
	HT_CAPABILITY_IE HTCapability;
#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap;
	VHT_OP_IE vht_op;
	UCHAR vht_cap_len;
	UCHAR vht_op_len;
	UCHAR operating_mode_len;
	OPERATING_MODE operating_mode;
#endif /* DOT11_VHT_AC */
#ifdef CONFIG_OWE_SUPPORT
	EXT_ECDH_PARAMETER_IE ecdh_ie;
#endif /*CONFIG_OWE_SUPPORT*/
} IE_LISTS;


typedef struct _bcn_ie_list {
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Bssid[MAC_ADDR_LEN];
	CHAR Ssid[MAX_LEN_OF_SSID];
	UCHAR SsidLen;
	UCHAR BssType;
	USHORT BeaconPeriod;
	UCHAR Channel;
	UCHAR NewChannel;
	USHORT AtimWin;
	USHORT CapabilityInfo;
	UCHAR Erp;
	UCHAR DtimCount;
	UCHAR DtimPeriod;
	UCHAR BcastFlag;
	UCHAR MessageToMe;
	UCHAR SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR SupRateLen;
	UCHAR ExtRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR ExtRateLen;
	UCHAR CkipFlag;
	UCHAR AironetCellPowerLimit;
	LARGE_INTEGER TimeStamp;
	CF_PARM CfParm;
	EDCA_PARM EdcaParm;
	QBSS_LOAD_PARM QbssLoad;
	QOS_CAPABILITY_PARM QosCapability;
	struct _vendor_ie_cap vendor_ie;
	EXT_CAP_INFO_ELEMENT ExtCapInfo;
	UCHAR HtCapabilityLen;
	UCHAR PreNHtCapabilityLen;
	HT_CAPABILITY_IE HtCapability;
	UCHAR AddHtInfoLen;
	ADD_HT_INFO_IE AddHtInfo;
	UCHAR NewExtChannelOffset;
#ifdef DOT11_VHT_AC
	VHT_CAP_IE vht_cap_ie;
	VHT_OP_IE vht_op_ie;
	UCHAR vht_cap_len;
	UCHAR vht_op_len;
	WIDE_BW_CH_SWITCH_ELEMENT wb_info;
#endif /* DOT11_VHT_AC */
	BOOLEAN  FromBcnReport;
	BOOLEAN is_marvell_ap;
	BOOLEAN is_atheros_ap;

} BCN_IE_LIST;

VOID MlmeHandler(struct _RTMP_ADAPTER *pAd);

#define RTMP_MLME_HANDLER(pAd)	RtmpMLMEUp(&((pAd)->mlmeTask))

#define ACTION_QOSMAP_CONFIG	4

typedef struct _QOSMAP_SET {
	UCHAR	DSCP_Field_Len;
	USHORT	DSCP_Field[29];		/* Use to time out while slience, unit: second , set by UI */
} QOSMAP_SET, *PQOSMAP_SET;

typedef struct _MLME_QOS_ACTION_STRUCT {
	UCHAR		ActionField;
	UCHAR		Addr[MAC_ADDR_LEN];
	UCHAR		apidx;
	QOSMAP_SET	QOSMap;
} MLME_QOS_ACTION_STRUCT, *PMLME_QOS_ACTION_STRUCT;

#endif	/* MLME_H__ */
