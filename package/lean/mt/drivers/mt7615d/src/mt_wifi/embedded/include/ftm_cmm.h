/****************************************************************************
* Mediatek Inc.
* 5F., No.5, Taiyuan 1st St., Zhubei City,
* Hsinchu County 302, Taiwan, R.O.C.
* (c) Copyright 2014, Mediatek, Inc.
*
* All rights reserved. Mediatek's source code is an unpublished work and the
* use of a copyright notice does not imply otherwise. This source code
* contains confidential trade secret material of Mediatek. Any attemp
* or participation in deciphering, decoding, reverse engineering or in any
* way altering the source code is stricitly prohibited, unless the prior
* written consent of Mediatek, Inc. is obtained.
****************************************************************************

	Module Name:
	ftm_cmm.h

	Abstract:
	802.11mc FTM protocol struct, enum, and macro declaration.

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	annie		 2014.11.22   Initial version.
*/

#ifndef __FTM_CMM_H__
#define __FTM_CMM_H__

#define NUMBER_PID_FTM				(PID_FTM_MAX - PID_FTM_MIN + 1)	/* 32: 0x21~0x40 */
#define MASK_PID_FTM				(NUMBER_PID_FTM - 1)	/* 0x1F: 5 bits */
#define FTMPID_NOT_WAITING			0

#define MAX_FTM_TBL_EXPONENT		4
#define MAX_FTM_TBL_SIZE			(1 << MAX_FTM_TBL_EXPONENT)	/* 16 = 2^4 */
#define FTM_TBL_HASH_INDEX(Addr)	(MAC_ADDR_HASH(Addr) & (MAX_FTM_TBL_SIZE - 1))

#define INVALID_FTM_ENTRY_IDX		0xFF
#define VALIDATE_FTM_ENTRY_IDX(idx)	(idx < MAX_FTM_TBL_SIZE)

#define INCREASE_IDX(var, min, max)	{ \
		if ((var) == (max - 1)) \
			(var) = (min); \
		else \
			(var)++; \
	}

#define DECREASE_IDX(var, min, max)	{ \
		if ((var) == (min)) \
			(var) = (max - 1); \
		else \
			(var)--; \
	}

#define GET_FTM_PEER_IDX(pAd, ptr)	((ptr) - (pAd)->pFtmCtrl->FtmPeer)

#define CNT_DOWN_DECREASE(var)	{ \
		if (var) \
			(var)--; \
		else \
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,  \
					 ("%s()#%d: CAUSION!! Before CntDown the value is zero!!\n", __func__, __LINE__)); \
	}


#define FTM_DEFAULT_ASAP				FALSE
#define FTM_DEFAULT_MIN_DELTA_FTM		80
#define FTM_DEFAULT_FTMS_PER_BURST		5
#define FTM_DEFAULT_NUM_BURST_EXP		0	/* follow up testplan: single burst */
#define FTM_DEFAULT_BURST_DURATION		FTMBST_128MS
#define FTM_DEFAULT_BURST_PERIOD		2
#define FTM_DEFAULT_PTSF_DELTA			15	/* unit: ms */
#define FTM_MAX_NUM_OF_RETRY			15
#define FTM_EACH_TMR_WAITING_TIME		1	/* unit: 1 ms, follows RTMPSetTimer */
#define FTM_TRGGER_TIMEOUT			5000	/* unit: ms */


/* 802.11mc D3.0: 8.4.2.20.10 (p.743):
	Figure 8-162, Measurement Request field format for LCI request
*/
typedef struct _MSMT_REQ_LCI {
	UINT8 LocationSubj;
} MSMT_REQ_LCI, *PMSMT_REQ_LCI;

/* 802.11mc D3.0: 8.4.2.20.10 (p.744):
	Figure 8-164, Azimuth Request field
*/
typedef union GNU_PACKED _AZIMUTH_REQUEST {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {
		UINT8 Reserved:3;
		UINT8 AzType:1;		/* Azimuth Type */
		UINT8 AzResReq:4;	/* Azimuth Resolution Requested */
	} field;
#else
	struct GNU_PACKED {
		UINT8 AzResReq:4;	/* Azimuth Resolution Requested */
		UINT8 AzType:1;		/* Azimuth Type */
		UINT8 Reserved:3;
	} field;
#endif /* RT_BIG_ENDIAN */
	UINT8 word;
} AZIMUTH_REQUEST, *PAZIMUTH_REQUEST;

/* 802.11mc D3.0: 8.4.2.20.14 (p.751):
	Figure 8-178, Location Civic Request field format
*/
typedef struct GNU_PACKED _MSMT_REQ_CIVIC {
	UINT8 LocationSubj;
	UINT8 CivicLocType;
	UINT8 ServiceIntvUnits;
	UINT16 ServiceInterval;
} MSMT_REQ_CIVIC, *PMSMT_REQ_CIVIC;

/* 802.11mc D3.0: 8.4.2.21.10 (p.780):
	Figure 8-210, LCI subelement format
*/
typedef struct GNU_PACKED _MSMT_RPT_SUBELEMENT {
	UINT8 SubElement;
	UINT8 Length;
} MSMT_RPT_SUBELEMENT, *PMSMT_RPT_SUBELEMENT;

/* 802.11mc D3.0:  (p.781):
	Figure 8-211, LCI field format
*/
typedef union GNU_PACKED _LCI_FIELD {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {
		UINT8 STAVersion:2;
		UINT8 Dependent:1;
		UINT8 RegLocDSE:1;
		UINT8 RegLocAgreement:1;
		UINT8 Datum:3;

		/* Altitude: 5 bytes */
		UINT8 Altitude_b22_b29;

		UINT32 Altitude_b0_b21:22;
		UINT32 AltitudeUncertainty:6;
		UINT32 AltitudeType:4;

		/* Longitude: 5 bytes */
		UINT32 Longitude_b2_b33;
		UINT8 Longitude_b0_b1:2;
		UINT8 LongitudeUncertainty:6;

		/* Latitude: 5 bytes */
		UINT32 Latitude_b2_b33;
		UINT8 Latitude_b0_b1:2;
		UINT8 LatitudeUncertainty:6;
	} field;
#else
	struct GNU_PACKED {
		/* Latitude: 5 bytes = 1+4 */
		UINT8 LatitudeUncertainty:6;
		UINT8 Latitude_b0_b1:2;
		UINT32 Latitude_b2_b33;

		/* Longitude: 5 bytes = 1+4 */
		UINT8 LongitudeUncertainty:6;
		UINT8 Longitude_b0_b1:2;
		UINT32 Longitude_b2_b33;

		/* Altitude: 5 bytes = 4+1 */
		UINT32 AltitudeType:4;
		UINT32 AltitudeUncertainty:6;
		UINT32 Altitude_b0_b21:22;
		UINT8 Altitude_b22_b29;

		UINT8 Datum:3;
		UINT8 RegLocAgreement:1;
		UINT8 RegLocDSE:1;
		UINT8 Dependent:1;
		UINT8 STAVersion:2;
	} field;
#endif /* RT_BIG_ENDIAN */

	UINT8 byte[16];
} LCI_FIELD, *PLCI_FIELD;

/* 802.11mc D3.0: 8.4.2.21.10 (p.784):
	Figure 8-215, STA Floor Info field format
*/
typedef union GNU_PACKED _STA_FLOOR_INFO {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {
		UINT16 LocationPolicy:1;
		UINT16 FloorNum:14;
		UINT16 ExpectedToMove:1;
	} field;
#else
	struct GNU_PACKED {
		UINT16 ExpectedToMove:1;
		UINT16 FloorNum:14;
		UINT16 LocationPolicy:1;
	} field;
#endif /* RT_BIG_ENDIAN */

	UINT16 word;
} STA_FLOOR_INFO, *PSTA_FLOOR_INFO;

/* 802.11mc D3.0:  (p.783):
	Figure 8-214, Z subelement format
*/
typedef struct GNU_PACKED _Z_ELEMENT {
	UINT8 SubElement;
	UINT8 Length;
	STA_FLOOR_INFO Floor;
	UINT16 HeightAboveFloor;
	UINT8 HeightUncertainty;
} Z_ELEMENT, *PZ_ELEMENT;

/* 802.11mc D4.0: (p.794)
	Usage Rules/Policy Parameters field format
*/
typedef union GNU_PACKED _LCI_USAGE_RULES_POLICY {
#ifdef RT_BIG_ENDIAN
	struct GNU_PACKED {
		UINT8 Reserved:5;
		UINT8 LocationPolicy:1;
		UINT8 RetExpiresPresent:1;
		UINT8 RetransAllowed:1;
	} field;
#else
	struct GNU_PACKED {
		UINT8 RetransAllowed:1;
		UINT8 RetExpiresPresent:1;
		UINT8 LocationPolicy:1;
		UINT8 Reserved:5;
	} field;
#endif /* RT_BIG_ENDIAN */

	UINT8 word;
} LCI_USAGE_RULES_POLICY, *PLCI_USAGE_RULES_POLICY;

/*
	D4.0: (P793)
	Usage Rules/Policy subelement
*/
typedef struct GNU_PACKED _USAGE_SUBELEMENT {
	UINT8 SubElement;
	UINT8 Length;
	LCI_USAGE_RULES_POLICY RulesAndPolicy;
	UINT16 RetExpires;
} USAGE_SUBELEMENT, *PUSAGE_SUBELEMENT;

#define MAX_CIVIC_CA_VALUE_LENGTH		0xF5
typedef struct GNU_PACKED _LOCATION_CIVIC {
	char CountryCode[2];
	UINT8 CA_Type;
	UINT8 CA_Length;
	UINT8 CA_Value[0];
} LOCATION_CIVIC, *PLOCATION_CIVIC;

typedef struct GNU_PACKED _FTM_REQUEST_FRAME {
	UINT8 Category;
	UINT8 Action;
	UINT8 Trigger;
	UINT8 Variable[0];
} FTM_REQUEST_FRAME, *PFTM_REQUEST_FRAME;

typedef struct GNU_PACKED _FTM_ERROR_FIELD {
	UINT16 MaxError:15;
	UINT16 NotConti:1;
} FTM_ERROR_FIELD, *PFTM_ERROR_FIELD;

typedef struct GNU_PACKED _FTM_FRAME {
	UINT8 Category;
	UINT8 Action;
	UINT8 DialogToken;
	UINT8 FollowUpDialogToken;
	UINT8 TOD[6];
	UINT8 TOA[6];
	FTM_ERROR_FIELD	TODError;
	FTM_ERROR_FIELD	TOAError;
	UINT8 Variable[0];
} FTM_FRAME, *PFTM_FRAME;

typedef struct _TMR_NODE {
	TMR_FRM_STRUC HwReport;
	UINT64 toa;
	UINT64 tod;
} TMR_NODE, *PTMR_NODE;

#ifdef FTM_INITIATOR
typedef struct _FTM_MAPPING {
	DL_LIST list;	/* queue head: RxTmrQ in FTM_PEER_ENTRY */
	TMR_NODE tmr;
	UINT16 sn;

	/* Rx information */
	UINT8 DialogToken;
	UINT8 FollowUpToken;
	UINT64 PeerTOA;
	UINT64 PeerTOD;
} FTM_MAPPING, *PFTM_MAPPING;
#endif /* FTM_INITIATOR */

typedef struct _FTM_PEER_ENTRY {
	/* FTM Timer related */
	RALINK_TIMER_STRUCT FtmTimer;
	RALINK_TIMER_STRUCT FtmTxTimer;
	PVOID pAd;	/* for timer callback */

	/* FTM initiator info maintain */
	UINT8 State;
	UINT8 Addr[MAC_ADDR_LEN];
	BOOLEAN bLciMsmtReq;
	BOOLEAN bCivicMsmtReq;
	BOOLEAN bLciMsmtReport;
	BOOLEAN bCivicMsmtReport;
	FTM_PARAMETER PeerReqParm;
	FTM_PARAMETER VerdictParm;
	UINT8 DialogToken;
	UINT8 LciToken;
	UINT8 CivicToken;
	UINT8 BurstCntDown;
	UINT8 FtmCntDown;

	/* For Software Retry */
	UINT8 FollowUpToken;
	TMR_NODE FollowUpTmr;

	/* Hardware report */
	TMR_NODE Tmr;
	BOOLEAN bNeedTmr;
	DL_LIST PidList;	/* queue head: PidPendingQ in FTM_CTRL */
	UINT8 PendingPid;	/* valid value: 0x21~0x40, valid period: after Tx, before TMR inetrrupt */
	UINT8 TransAndRetrans; /* Num of FTM (of the same follow up token) transmission and retransmission times  */
	BOOLEAN bGotTmr;
	BOOLEAN bTxOK;
	BOOLEAN bTxSCallbackCheck;	/* 1: FtmTxTimer is used for TxS check, 0: FtmTxTimer is used for min_delta */

#ifdef FTM_INITIATOR
	/* FTM initiator control */
	DL_LIST RxTmrQ;
	NDIS_SPIN_LOCK RxTmrQLock;
#endif /* FTM_INITIATOR */
} FTM_PEER_ENTRY, *PFTM_PEER_ENTRY;

typedef struct _FTM_NEIGHBORS {
	/* FTM Neighbor info maintain */
	BOOLEAN NeighborValid;
	UINT8 NeighborBSSID[MAC_ADDR_LEN];
	UINT8 NeighborPhyType;
	UINT8 NeighborFTMrespCap;
	UINT8 NeighborOpClass;
	UINT8 NeighborChannel;
	BOOLEAN NeighborFTMCap;

	/* Neighbor LCI/CIVIC info*/
	MSMT_RPT_SUBELEMENT LciHdr;
	LCI_FIELD LciField;
	Z_ELEMENT LciZ;
	USAGE_SUBELEMENT LciUsage;
	MSMT_RPT_SUBELEMENT CivicHdr;
	LOCATION_CIVIC Civic;
	UINT8 CA_Value[MAX_CIVIC_CA_VALUE_LENGTH];
	BOOLEAN	bSetNeighbotZRpt;

} FTM_NEIGHBORS, *PFTM_NEIGHBORS;

typedef struct _FTM_CTRL {
	UINT8 DialogToken;
	UINT8 LatestJoinPeer;
	DL_LIST PidPendingQ;
	NDIS_SPIN_LOCK PidPendingQLock;

	/* FTM Parameters */
	BOOLEAN asap;
	UINT8 min_delta_ftm;
	UINT8 ftms_per_burst;
	UINT8 num_burst_exponent;
	UINT8 burst_duration;
	UINT16 burst_period;
	MSMT_RPT_SUBELEMENT LciHdr;
	LCI_FIELD LciField;
	Z_ELEMENT LciZ;
	USAGE_SUBELEMENT LciUsage;
	MSMT_RPT_SUBELEMENT CivicHdr;
	LOCATION_CIVIC Civic;
	UINT8 CA_Value[MAX_CIVIC_CA_VALUE_LENGTH];

	/* FTM initiator peers */
	FTM_PEER_ENTRY FtmPeer[MAX_FTM_TBL_SIZE];

	UINT32 BurstCntDown;
	UINT8 Responder[MAC_ADDR_LEN];
	BOOLEAN WaitForNego;
	RALINK_TIMER_STRUCT FtmReqTimer;
	BOOLEAN bSetLciReq;
	BOOLEAN bSetCivicReq;
	BOOLEAN bSetLciRpt;
	BOOLEAN bSetZRpt;
	BOOLEAN bSetCivicRpt;
	UINT8 TokenLCI;
	UINT8 TokenCivic;
	/* FTM Neighbors */
	UINT8 FtmNeighborIdx;
	FTM_NEIGHBORS FtmNeighbor[MAX_FTM_TBL_SIZE];

	/*PF2 4.2.6 manual test*/
	UINT16 RandomizationInterval;
	UINT8 MinimumApCount;

} FTM_CTRL, *PFTM_CTRL;


enum FTM_PEER_STATE {
	FTMPEER_UNUSED,
	FTMPEER_IDLE,
	FTMPEER_NEGO,
	FTMPEER_MEASURING_IN_BURST,
	FTMPEER_MEASURING_WAIT_TRIGGER
};

/* 802.11mc D3.0: 8.4.2.20 (p.726):
	Table 8-90, Measurement Type definitions for measurement requests
*/
enum MEASUREMENT_TYPE_REQUEST {
	MSMT_LCI_REQ			= 8,
	MSMT_LOCATION_CIVIC_REQ	= 11,
	MSMT_FTM_RANGE_REQ		= 16,
};

/* 802.11mc D3.0: 8.4.2.20.10 (p.743):
	Table 8-102, Location subject definition
*/
enum LOCATION_SUBJECT {
	LSUBJ_LOCAL				= 0,
	LSUBJ_REMOTE			= 1,
	LSUBJ_THIRD_PARTY		= 2
};

/* 802.11mc D3.0: 8.4.2.20.10 (p.743):
	Table 8-103, Optional subelement IDs for LCI request
*/
enum LCI_REQUEST_ID {
	LCI_REQID_AZIMUTH			= 1,
	LCI_REQID_ORI_REQ_MAC_ADDR	= 2,
	LCI_REQID_TARGET_MAC_ADDR	= 3,
	LCI_REQID_MAX_AGE			= 4
};

/* 802.11mc D3.0: 8.4.2.20.10 (p.744) */
enum AZIMUTH_TYPE {
	AZTYPE_RADIO_RECEPTION	= 0,
	AZTYPE_FRONT_SURFACE	= 1
};

/* 802.11mc D3.0: 8.4.2.20.14 (p.751):
	Table 8-108, Civic Location Type
*/
enum CIVIC_LOCATION_TYPE {
	CIVIC_TYPE_IETF_RFC4776_2006 = 0,
	/* IETF RFC 4776-2006 starting at the country code field (i.e., excluding the
	GEOCONF_CIVIC/ OPTION_GEOCONF_CIVIC, N/option-len and what
	fields); includes all subsequent RFCs that define additional civic address Types */

	CIVIC_TYPE_VENDOR_SPECIFIC = 1
};

/* 802.11mc D3.0: 8.4.2.20.14 (p.751):
	Table 8-109, Location Service Interval Units
*/
enum CIVIC_LOCATION_SERVICE_INTERVAL_UNITS {
	CIVIC_UNIT_SECOND	= 0,
	CIVIC_UNIT_MINUTE	= 1,
	CIVIC_UNIT_HOURS	= 2
};

/* 802.11mc D3.0: 8.4.2.21 (p.759):
	Table 8-115, Measurement Type definitions for measurement reports
*/
enum MEASUREMENT_TYPE_REPORT {
	MSMT_LCI_RPT			= 8,
	MSMT_LOCATION_CIVIC_RPT	= 11
};

/* 802.11mc D3.0: 8.4.2.21.10 (p.780):
	Table 8-124, Subelement IDs for Location Configuration Information Report
*/
enum LCI_REPORT_ID {
	LCI_RPTID_LCI					= 0,
	LCI_RPTID_AZIMUTH				= 1,
	LCI_RPTID_ORI_REQ_MAC_ADDR		= 2,
	LCI_RPTID_TARGET_MAC_ADDR		= 3,
	LCI_RPTID_Z						= 4,
	LCI_RPTID_LOCATION_ERROR		= 5,
	LCI_RPTID_USAGE_RULES			= 6
};

/* 802.11mc D3.0: 8.4.2.21.13 (p.793):
	Table 8-129, Subelement IDs for Location Civic report
*/
enum CIVIC_REPORT_ID {
	CIVIC_RPTID_CIVIC				= 0,
	CIVIC_RPTID_ORI_REQ_MAC_ADDR	= 1,
	CIVIC_RPTID_TARGET_MAC_ADDR		= 2,
	CIVIC_RPTID_LOCATION_REF		= 3,
	CIVIC_RPTID_LOCATION_SHAPE		= 4,
	CIVIC_RPTID_MAP_IMAGE			= 5,
	CIVIC_RPTID_VENDOR_SPECIFIC		= 221
};


#define FTM_NUM_NO_PREFERENCE		0
#define FTM_DELTA_NO_PREFERENCE		0


/* 802.11mc D4.0: 8.4.2.166 (p.1051):
	Table 8-245, Statue indication field values
*/
enum FTM_STATUS_CODE {
	FTMSTATUS_RSVD,
	FTMSTATUS_SUCCESS,
	FTMSTATUS_REQ_INCAPABLE,
	FTMSTATUS_REQ_FAILED,
};


/* 802.11mc D4.3: 8.4.2.166 (p.1077):
	Table 8-250, Burst Duration field encoding
*/
enum FTM_BURST_DURATION_ENCODING {
	FTMBST_250US		= 2,
	FTMBST_500US		= 3,
	FTMBST_1MS			= 4,
	FTMBST_2MS			= 5,
	FTMBST_4MS			= 6,
	FTMBST_8MS			= 7,
	FTMBST_16MS			= 8,
	FTMBST_32MS			= 9,
	FTMBST_64MS			= 10,
	FTMBST_128MS		= 11,
	FTMBST_NO_PREFERENCE = 15
};


/* 802.11mc D4.0: 8.4.2.166 (p.1053):
	Table 8-247, FTM Format and Bandwidth field
*/
enum FTM_CHANNEL_SPACING {
	FTM_BW_NO_PREFERENCE	= 0,
	FTM_BW_NONHT_BW5		= 4,
	FTM_BW_NONHT_BW10		= 6,
	FTM_BW_NONHT_BW20		= 8,
	FTM_BW_HT_BW20			= 9,
	FTM_BW_VHT_BW20			= 10,
	FTM_BW_HT_BW40			= 11,
	FTM_BW_VHT_BW40		= 12,
	FTM_BW_VHT_BW80		= 13,
	FTM_BW_VHT_BW80_80		= 14,
	FTM_BW_VHT_BW160_2RFLO	= 15,
	FTM_BW_VHT_BW160_1RFLO	= 16,
	FTM_BW_DMG_BW2160		= 31,
};


/* 802.11mc D3.0: 8.6.8.32 (p.1121):
	FTM Request Trigger Code
*/
enum FTM_REQ_TRIGGER_CODE {
	FTM_TRIGGER_STOP,
	FTM_TRIGGER_START_OR_CONTI
};

enum FTM_TX_REASON {
	FTMTX_START,
	FTMTX_ONGOING,
	FTMTX_STOP
};

#endif /* __FTM_CMM_H__ */
