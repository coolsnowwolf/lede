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
    tdls.h
 
    Abstract:
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Arvin Tai  17-04-2009    created for 802.11z
 */

#ifdef DOT11Z_TDLS_SUPPORT

#ifndef __TDLS_CMM_H
#define __TDLS_CMM_H

#ifdef WFD_SUPPORT
#include "wfd_cmm.h"
#endif /* WFD_SUPPORT */

#define TDLS_MAX_BLACL_LIST_SZIE	64
#define	MAX_NUM_OF_TDLS_ENTRY        4
#define TDLS_ENTRY_AGEOUT_TIME		30		/* unit: sec */
#define TDLS_MAX_SEARCH_TABLE_SZIE	64

#define TDLS_SEARCH_ENTRY_AGEOUT 5000  /* seconds */
#define TDLS_SEARCH_ENTRY_AGEOUT_LIMIT	600000  /* seconds */
#define TDLS_SEARCH_POOL_SIZE 64
#define TDLS_SEARCH_HASH_TAB_SIZE 32  /* the legth of hash table must be power of 2. */

#define TDLS_ENTRY_POOL_SIZE 8
#define TDLS_ENTRY_HASH_TAB_SIZE 4  /* the legth of hash table must be power of 2. */

#ifdef WFD_SUPPORT
#define WFD_TUNNELED_PROBE_REQ		4
#define WFD_TUNNELED_PROBE_RSP		5
#endif /* WFD_SUPPORT */

#define TDLS_DISCOVERY_IDLE				0
#define TDLS_DISCOVERY_FIRST_TIME		1
#define TDLS_DISCOVERY_TO_SETUP			2
#define TDLS_DISCOVERY_TO_SETUP_DONE	3
#define TDLS_DISCOVERY_TO_SETUP_FAIL	4
#define TDLS_CHANNEL_SWITCH_TIME		11000 /* micro seconds */
#define TDLS_CHANNEL_SWITCH_TIMEOUT		19000 /* micro seconds */

#define TDLS_POWER_SAVE_ACTIVE_COUNT_DOWN_NUM		(5*1000/MLME_TASK_EXEC_INTV)

#define TDLS_FIFO_MGMT		0
#define TDLS_FIFO_HCCA		1
#define TDLS_FIFO_EDCA		2
#define TDLS_FIFO_ALL		3

// states of tdls ctrl state machine
typedef enum _TDLS_CTRL_STATE
{
	TDLS_CTRL_IDLE,
	TDLS_CTRL_CHANNEL_SWITCH,
	TDLS_CTRL_MAX_STATES,
} TDLS_CTRL_STATE;

// events of tdls ctrl state machine
typedef enum _TDLS_CTRL_EVENT
{
	// Events from SME.
	TDLS_MLME_CHANNEL_SWITCH_REQ,
	TDLS_MLME_CHANNEL_SWITCH_RSP,
	TDLS_PEER_CHANNEL_SWITCH_REQ,
	TDLS_PEER_CHANNEL_SWITCH_RSP,
	TDLS_CTRL_MAX_EVENTS,
} TDLS_CTRL_EVENT;

#define	TDLS_CTRL_FUNC_SIZE	(TDLS_CTRL_MAX_STATES * TDLS_CTRL_MAX_EVENTS)

// states of tdls link management state machine
typedef enum _TDLS_LINK_MNG_STATE
{
	TDLS_LINK_IDLE,
	TDLS_LINK_SETUP_REQ,
	TDLS_LINK_SETUP_RSP,
	TDLS_LINK_ESTAB,
	TDLS_LINK_TEAR_DOWN,
	TDLS_LINK_MAX_STATES,
} TDLS_LINK_MNG_STATE;

// events of tdls link management state machine
typedef enum _TDLS_LINK_MNG_EVENT
{
	// Events from SME.
	TDLS_LINK_MLME_SETUP_REQ,
	TDLS_LINK_PEER_SETUP_REQ,
	TDLS_LINK_PEER_SETUP_RSP,
	TDLS_LINK_PEER_SETUP_CONF,
	TDLS_LINK_MLME_TEARDOWN,
	TDLS_LINK_PEER_TEARDOWN,
	TDLS_LINK_MAX_EVENTS,
} TDLS_LINK_MNG_EVENT;

#define	TDLS_BLACK_NONE				0
#define	TDLS_BLACK_WAIT_NEXT_TRY	1
#define TDLS_BLACK_AUTO_DISCOVERY	2
#define	TDLS_BLACK_TDLS_BY_TEARDOWN	3

typedef struct _TDLS_SEARCH_ENTRY {
	struct _TDLS_SEARCH_ENTRY * pNext;
	ULONG	LastRefTime;
	UCHAR	RetryCount;
	UCHAR	InitRefTime;
	UCHAR	Addr[MAC_ADDR_LEN];
} TDLS_SEARCH_ENTRY, *PTDLS_SEARCH_ENTRY;

/* TDLS Settings for each link entry */
typedef struct _RT_802_11_TDLS {
	USHORT			TimeOut;		/* unit: second , set by UI */
	USHORT			CountDownTimer;	/* unit: second , used by driver only */
	UCHAR			MacAddr[MAC_ADDR_LEN];		/* set by UI */
	UCHAR			Status;			/* 0: none , 1: wait result, 2: wait add , 3: connected */
	BOOLEAN			Valid;			/* 1: valid , 0: invalid , set by UI, use to setup or tear down DLS link */
	/* The above parameters are the same as RT_802_11_DLS_UI */

#ifdef UAPSD_SUPPORT
	BOOLEAN			FlgIsWaitingUapsdTraRsp; /* 1: waiting for traffic rsp frame */
	UCHAR			UapsdTraIndDelayCnt; /* used to send a detect traffic ind */
	BOOLEAN			FlgIsUapsdTraRspRcv; /* if we have ever received rsp frame */
#endif /* UAPSD_SUPPORT */

	UCHAR			Token;			/* Dialog token */
	RALINK_TIMER_STRUCT	Timer;			/* Use to time out while handshake */
	BOOLEAN			bInitiator;		/* TRUE: I am TDLS Initiator STA, FALSE: I am TDLS Responder STA */
	UCHAR			MacTabMatchWCID;
	PVOID			pAd;
	USHORT			CapabilityInfo;

	/* Copy supported rate from desired Initiator. We are trying to match */
	/* Initiator's supported and extended rate settings. */
	UCHAR			SupRateLen;
	UCHAR			ExtRateLen;
	UCHAR			SupRate[MAX_LEN_OF_SUPPORTED_RATES];
	UCHAR			ExtRate[MAX_LEN_OF_SUPPORTED_RATES];

	/* For TPK handshake */
	UCHAR			ANonce[32];	/* Generated in Message 1, random variable */
	UCHAR			SNonce[32];	/* Generated in Message 2, random variable */
	ULONG			KeyLifetime;	/*  Use type= 'Key Lifetime Interval' unit: Seconds, min lifetime = 300 seconds */
	UCHAR			TPK[LEN_PMK];	/* TPK-KCK(16 bytes) for MIC + TPK-TP (16 bytes) for data */
	UCHAR			TPKName[LEN_PMK_NAME];
	
	/* For QOS */
	BOOLEAN			bWmmCapable;	/* WMM capable of the peer TDLS */
	UCHAR			QosCapability; /* QOS capability of the current connecting Initiator */
	EDCA_PARM		EdcaParm;		/* EDCA parameters of the Initiator */

	/* Features */
	UCHAR					HtCapabilityLen;
	HT_CAPABILITY_IE		HtCapability;
	UCHAR					TdlsExtCapLen;
	EXT_CAP_INFO_ELEMENT	TdlsExtCap;

#ifdef DOT11_VHT_AC
	UCHAR vht_cap_len;
	VHT_CAP_IE *vht_cap;
#endif /* DOT11_VHT_AC */

	/* Channel Switch */
	BOOLEAN 				bDoingPeriodChannelSwitch;
	UCHAR					ChannelSwitchCurrentState;
	USHORT					ChSwitchTime;	
	RALINK_TIMER_STRUCT 	ChannelSwitchTimer; 		// Use to channel switch
	USHORT					ChSwitchTimeout;
	RALINK_TIMER_STRUCT 	ChannelSwitchTimeoutTimer;			// Use to channel switch

#ifdef WFD_SUPPORT
	WFD_ENTRY_INFO			WfdEntryInfo;
#endif /* WFD_SUPPORT */

} RT_802_11_TDLS, *PRT_802_11_TDLS;

/* change PS mode to ACTIVE mode before building a TDLS link */
#define TDLS_CHANGE_TO_ACTIVE(__pAd)										\
{																			\
	BOOLEAN __FlgOld = (__pAd)->StaCfg.TdlsInfo.TdlsFlgIsKeepingActiveCountDown;		\
	(__pAd)->StaCfg.TdlsInfo.TdlsFlgIsKeepingActiveCountDown = TRUE;					\
	(__pAd)->StaCfg.TdlsInfo.TdlsPowerSaveActiveCountDown =							\
								TDLS_POWER_SAVE_ACTIVE_COUNT_DOWN_NUM;		\
	if (__FlgOld == FALSE)													\
	{																		\
		/* send a null frame to trigger asic wake up */						\
		RTMPSendNullFrame(__pAd, __pAd->CommonCfg.TxRate, TRUE, FALSE);				\
	}																		\
}

/* recover PS mode when TdlsPowerSaveActiveCountDown = 0 */
#define TDLS_RECOVER_POWER_SAVE(__pAd)										\
{																			\
	(__pAd)->StaCfg.TdlsInfo.TdlsFlgIsKeepingActiveCountDown = FALSE;				\
	(__pAd)->StaCfg.TdlsInfo.TdlsPowerSaveActiveCountDown = 0;						\
	/* send a null frame to trigger asic sleep */							\
	RTMPSendNullFrame(__pAd, __pAd->CommonCfg.TxRate, TRUE, __pAd->StaCfg.Psm);					\
}
#endif /* __TDLS_CMM_H */
#endif /* DOT11Z_TDLS_SUPPORT */

