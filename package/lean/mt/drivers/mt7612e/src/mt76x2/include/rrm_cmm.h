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
typedef struct _RRM_QUIET_CB
{
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

typedef struct _RRM_CONFIG
{
	BOOLEAN bDot11kRRMEnableSet;
	BOOLEAN bDot11kRRMEnable;
	BOOLEAN bDot11kRRMNeighborRepTSFEnable;
	RRM_QUIET_CB QuietCB;
} RRM_CONFIG, *PRRM_CONFIG;

typedef union _RRM_BCN_REQ_CAP
{
	struct
	{
		UINT8 ReportCondition:1;
		UINT8 ChannelRep:1;
		UINT8 :6;
	} field;
	UINT8 word;
} RRM_BCN_REQ_CAP, *PRRM_BCN_REQ_CAP;

typedef struct _RRM_MLME_BCN_REQ_INFO
{
	UINT16 MeasureDuration;
	UINT8 Bssid[MAC_ADDR_LEN];
	PUINT8 pSsid;
	UINT8 SsidLen;
	UINT8 RegulatoryClass;
	UINT8 MeasureCh;
	UINT8 MeasureMode;
	RRM_BCN_REQ_CAP BcnReqCapFlag;
	UINT8 ChRepRegulatoryClass[MAX_NUM_OF_REGULATORY_CLASS];
} RRM_MLME_BCN_REQ_INFO, *PRRM_MLME_BCN_REQ_INFO;

typedef struct _RRM_MLME_TRANSMIT_REQ_INFO
{
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

#endif /* DOT11K_RRM_SUPPORT */

#endif /* __RRM_CMM_H */

