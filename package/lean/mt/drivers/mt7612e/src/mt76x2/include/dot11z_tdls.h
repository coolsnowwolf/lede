/****************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ****************************************************************************

    Module Name:
    dot11z_tdls.h
 
    Abstract:
	Defined status code, IE and frame structures that TDLS (802.11zD4.0) needed.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 11z
 */

#ifdef DOT11Z_TDLS_SUPPORT
#include "rtmp_type.h"

#ifndef __DOT11Z_TDLS_H
#define __DOT11Z_TDLS_H


/* TDLS definitions in 802.11z D13.0 specification */
#define	PROTO_NAME_TDLS				2
#define	TDLS_AKM_SUITE_1X			5	// <ANA>
#define	TDLS_AKM_SUITE_PSK			7

/* TDLS Action frame definition in 802.11z D13.0 specification */
#define TDLS_ACTION_CODE_SETUP_REQUEST				0
#define TDLS_ACTION_CODE_SETUP_RESPONSE				1
#define TDLS_ACTION_CODE_SETUP_CONFIRM				2
#define TDLS_ACTION_CODE_TEARDOWN					3
#define TDLS_ACTION_CODE_PEER_TRAFFIC_INDICATION	4 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_REQUEST		5
#define TDLS_ACTION_CODE_CHANNEL_SWITCH_RESPONSE	6
#define TDLS_ACTION_CODE_PEER_PSM_REQUEST			7
#define TDLS_ACTION_CODE_PEER_PSM_RESPONSE			8
#define TDLS_ACTION_CODE_PEER_TRAFFIC_RESPONSE		9 /* for TDLS UAPSD */
#define TDLS_ACTION_CODE_DISCOVERY_REQUEST			10

/* Status codes defined in 802.11zD13.0 specification. */
#define TDLS_STATUS_CODE_WAKEUP_SCHEDULE_REJECT_BUT_ALTERNATIVE_SCHEDULE_PROVIDED	2
#define TDLS_STATUS_CODE_WAKEUP_SCHEDULE_REJECT										3
#define TDLS_STATUS_CODE_SECURITY_DISABLED											5
#define TDLS_STATUS_CODE_UNACCEPTABLE_LIFETIME										6
#define TDLS_STATUS_CODE_NOT_IN_SAME_BSS											7
#define TDLS_STATUS_CODE_INVALID_CONTENTS_OF_RSNIE									72

/* Reason codes defined in 802.11zD13.0 specification. */
#define TDLS_REASON_CODE_TEARDOWN_DUE_TO_PEER_STA_UNREACHABLE						25
#define TDLS_REASON_CODE_TEARDOWN_FOR_UNSPECIFIED_REASON							26

/* Information element ID defined in 802.11z D13.0 specification. */
#define IE_TDLS_LINK_IDENTIFIER			101
#define IE_TDLS_WAKEUP_SCHEDULE			102
#define IE_TDLS_CHANNEL_SWITCH_TIMING	104
#define IE_TDLS_PTI_CONTROL				105
#define IE_TDLS_PU_BUFFER_STATUS		106


#define TDLS_ELM_LEN_LINK_IDENTIFIER		18
#define TDLS_ELM_LEN_WAKEUP_SCHEDULE		18
#define TDLS_ELM_LEN_CHANNEL_SWITCH_TIMING	6
#define TDLS_ELM_LEN_PTI_CONTROL			5
#define TDLS_ELM_LEN_BUFFER_STATUS			3

#define TDLS_KEY_TIMEOUT			3600      /* unit: sec */

#define TDLS_LEY_LIFETIME			3600

#ifndef DOT11R_FT_SUPPORT
#define FT_MIC_LEN					16
#define FT_NONCE_LEN				32

/* Information element ID defined in 802.11rD9.0 specification. */
#define IE_FT_MDIE				54
#define IE_FT_FTIE				55
#define IE_FT_TIMEOUT_INTERVAL	56
#define IE_FT_RIC_DATA			57
#define IE_FT_RIC_DESCRIPTOR	75


/* RIC Type */
#define FT_RIC_TYPE_BA			1

/* AKM SUITE */
#define FT_AKM_SUITE_1X		3
#define FT_AKM_SUITE_PSK	4

typedef union GNU_PACKED _FT_MIC_CTR_FIELD
{
	/*
		IECnt: contains the number of IEs
		that are included int eht MIC calculation.
	*/
	struct GNU_PACKED
	{
#ifdef RT_BIG_ENDIAN
	UINT16 IECnt:8;
	UINT16 :8;
#else
	UINT16 :8;
	UINT16 IECnt:8;
#endif
	} field;
	UINT16 word;
} FT_MIC_CTR_FIELD, *PFT_MIC_CTR_FIELD;

/*
** FTIE: Fast Transition IE.
*/
typedef struct GNU_PACKED _FT_FTIE
{
	FT_MIC_CTR_FIELD MICCtr;		/* 2 Octects. */
	UINT8 MIC[FT_MIC_LEN];			/* 16 Octects. */
	UINT8 ANonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 SNonce[FT_NONCE_LEN];		/* 32 Octects. */
	UINT8 Option[0];				/* 1:R1KHID, 2:GTK, 3:ROKHId, else:Res */ 
} FT_FTIE, *PFT_FTIE;

/*
** Timeout Interval IE.
*/
typedef enum _FT_TIMEOUT_INTERVAL_TYPE
{
	REASSOC_DEADLINE_INTERVAL = 1,	/* TUs */
	KEY_LIFETIME_INTERVAL,				/* seconds. */
#ifdef DOT11W_PMF_SUPPORT
	ASSOC_COMEBACK_TIME,				/* TUs */	
#endif /* DOT11W_PMF_SUPPORT */
	RESERVED_INTERVAL
} FT_TIMEOUT_INTERVAL_TYPE, *PFT_TIMEOUT_INTERVAL_TYPE;

typedef struct GNU_PACKED _FT_TIMEOUT_INTERVAL_IE
{
	UINT8 TimeoutIntervalType;
	UINT32 TimeoutIntervalValue;
} FT_TIMEOUT_INTERVAL_IE, *PFT_TIMEOUT_INTERVAL_IE;
#endif /* DOT11R_FT_SUPPORT */

typedef struct GNU_PACKED _TDLS_LINK_IDENT_ELEMENT{
	UCHAR	BSSID[MAC_ADDR_LEN];
	UCHAR	InitiatorAddr[MAC_ADDR_LEN];
	UCHAR	ResponderAddr[MAC_ADDR_LEN];
}TDLS_LINK_IDENT_ELEMENT, *PTDLS_LINK_IDENT_ELEMENT;

typedef struct GNU_PACKED _TDLS_CH_SWITCH_TIMING_ELEMENT{
	USHORT	ChSwitchTime;
	USHORT	ChSwitchTimeOut;
}TDLS_CH_SWITCH_TIMING_ELEMENT, *PTDLS_CH_SWITCH_TIMING_ELEMENT;

typedef struct GNU_PACKED _TDLS_WAKEUP_SCHEDULE_IE
{
	UINT32	Interval;
	UINT32	AwakeDuration;
	UINT16	IdleCount;
} TDLS_WAKEUP_SCHEDULE_IE, *PTDLS_WAKEUP_SCHEDULE_IE;

typedef struct GNU_PACKED _TDLS_AP_PHY_DATA_RATE_IE
{
	UINT32	AP_PHY_Data_Rate;
} TDLS_AP_PHY_DATA_RATE_IE, *PTDLS_AP_PHY_DATA_RATE_IE;

typedef struct GNU_PACKED _TDLS_CHANNEL_SWITCH_TIMING_IE
{
	UINT16	SwitchTime;
	UINT16	SwitchTimeout;
} TDLS_CHANNEL_SWITCH_TIMING_IE, *PTDLS_CHANNEL_SWITCH_TIMING_IE;

#endif /* __DOT11Z_TDLS_H */

#endif /* DOT11Z_TDLS_SUPPORT */

