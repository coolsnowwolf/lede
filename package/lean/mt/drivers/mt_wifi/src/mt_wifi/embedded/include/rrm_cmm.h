/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2006, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/

/****************************************************************************
	Abstract:

***************************************************************************/


#ifndef __RRM_CMM_H
#define __RRM_CMM_H


#ifdef DOT11K_RRM_SUPPORT
#include "rtmp_type.h"
#include "dot11k_rrm.h"


#define RRM_DEFAULT_QUIET_PERIOD	200
#define RRM_DEFAULT_QUIET_DURATION	30
#define RRM_DEFAULT_QUIET_OFFSET	20

#define RRM_QUIET_IDLE		0
#define RRM_QUIET_SILENT	1

#define MAX_NUM_OF_REQ_IE  13

typedef struct _RRM_QUIET_CB {
	BOOLEAN QuietState;
	UINT16 CurAid;
	UINT8 MeasureCh;
	RALINK_TIMER_STRUCT QuietOffsetTimer;
	RALINK_TIMER_STRUCT QuietTimer;


	UINT8 QuietCnt;
	UINT8 QuietPeriod;
	UINT8 QuietDuration;
	UINT8 QuietOffset;
} RRM_QUIET_CB, *PRRM_QUIET_CB;

typedef struct _RRM_CONFIG {
	BOOLEAN bDot11kRRMEnableSet;
	BOOLEAN bDot11kRRMEnable;
	BOOLEAN bDot11kRRMNeighborRepTSFEnable;
	RRM_QUIET_CB QuietCB;

	/* FOR AP Measurement Report */
	UINT8	PeerMeasurementToken;
	/* UINT8	PeerMeasurementType; */
	BOOLEAN bPeerReqLCI;
	BOOLEAN bPeerReqCIVIC;
	RRM_EN_CAP_IE rrm_capabilities;
	RRM_EN_CAP_IE max_rrm_capabilities;
} RRM_CONFIG, *PRRM_CONFIG;

typedef union _RRM_BCN_REQ_CAP {
	struct {
#ifdef RT_BIG_ENDIAN
		UINT8 Reserved:6;
		UINT8 ChannelRep:1;
		UINT8 ReportCondition:1;
#else
		UINT8 ReportCondition:1;
		UINT8 ChannelRep:1;
		UINT8 Reserved:6;
#endif
	} field;
	UINT8 word;
} RRM_BCN_REQ_CAP, *PRRM_BCN_REQ_CAP;

typedef struct _RRM_MLME_BCN_REQ_INFO {
	UINT16 MeasureDuration;
	UINT8 Bssid[MAC_ADDR_LEN];
	UINT8 Addr[MAC_ADDR_LEN];
	PUINT8 pSsid;
	UINT8 SsidLen;
	UINT8 RegulatoryClass;
	UINT8 MeasureCh;
	UINT8 MeasureMode;
	RRM_BCN_REQ_CAP BcnReqCapFlag;
	UINT8 ChRepRegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];
	UINT8 ChRepList[MAX_NUM_OF_CHS];
	UINT16 RandInt;
	UINT8 report_detail;
	UINT8 request_ie_num;
	UINT8 request_ie[MAX_NUM_OF_REQ_IE];
} RRM_MLME_BCN_REQ_INFO, *PRRM_MLME_BCN_REQ_INFO;

typedef struct _RRM_MLME_TRANSMIT_REQ_INFO {
	UINT16 MeasureDuration;
	UINT8 Tid;
	UINT8 BinRange;
	BOOLEAN bTriggerReport;
	BOOLEAN ArvCondition;
	BOOLEAN ConsecutiveCondition;
	BOOLEAN DelayCondition;
	UINT8 AvrErrorThreshold;
	UINT8 ConsecutiveErrorThreshold;
	UINT8 DelayThreshold;
	UINT8 MeasureCnt;
	UINT8 TriggerTimeout;
	UINT8 bDurationMandatory;
} RRM_MLME_TRANSMIT_REQ_INFO, *PRRM_MLME_TRANSMIT_REQ_INFO;


#ifdef CONFIG_11KV_API_SUPPORT
#define BCN_MACHINE_BASE 0

/* BCN states */
enum BCN_STATE {
	BCN_IDLE,
	WAIT_BCN_REQ,
	WAIT_BCN_REP,
	MAX_BCN_STATE,
};


/* BCN events */
enum BCN_EVENT {
	BCN_REQ,
	BCN_REQ_RAW,
	BCN_REP_TIMEOUT,
	MAX_BCN_MSG,
};

#define BCN_FUNC_SIZE (MAX_BCN_STATE * MAX_BCN_MSG)
#define BCN_REP_TIMEOUT_VALUE (60*1000)


#define NR_MACHINE_BASE 0

/* NR states */
enum NR_STATE {
	WAIT_NR_RSP,
	NR_UNKNOWN,
	MAX_NR_STATE,
};


/* NR events */
enum NR_EVENT {
	NR_RSP,
	NR_RSP_PARAM,
	NR_RSP_TIMEOUT,
	MAX_NR_MSG,
};

#define NR_FUNC_SIZE (MAX_NR_STATE * MAX_NR_MSG)
#define NR_RSP_TIMEOUT_VALUE (2*60*1000)

typedef struct GNU_PACKED _BCN_EVENT_DATA {
	UINT8 ControlIndex;
	UINT8 MeasureReqToken;
	UINT8 stamac[MAC_ADDR_LEN];
	UINT16 DataLen;
	UCHAR Data[0];
} BCN_EVENT_DATA, *PBCN_EVENT_DATA;

/* this struct is used for mlme */
typedef struct GNU_PACKED _NR_EVENT_DATA {
	UINT8 ControlIndex;
	UINT8 MeasureReqToken;
	UINT8 stamac[MAC_ADDR_LEN];
	UINT16 DataLen;
	UCHAR Data[0];
} NR_EVENT_DATA, *PNR_EVENT_DATA;

typedef struct GNU_PACKED _RRM_FRAME {
	HEADER_802_11 Hdr;
	UCHAR Category;
	union {
		struct {
			UCHAR Action;
			UCHAR DialogToken;
			UCHAR Variable[0];
		} GNU_PACKED NR_RSP;
	} u;
} RRM_FRAME, *PRRM_FRAME;

DECLARE_TIMER_FUNCTION(WaitPeerBCNRepTimeout);
DECLARE_TIMER_FUNCTION(WaitNRRspTimeout);

#endif /* CONFIG_11KV_API_SUPPORT */
#endif /* DOT11K_RRM_SUPPORT */

#endif /* __RRM_CMM_H */

