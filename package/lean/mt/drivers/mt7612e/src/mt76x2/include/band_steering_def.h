/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	band_steering_def.h
*/

#ifndef _BAND_STEERING_DEF_H_
#define __BAND_STEERING_DEF_H__

#ifdef BAND_STEERING
#ifndef DOT11_N_SUPPORT
#error: "DOT11_N_SUPPORT must be enabled when using band steering"
#endif /* DOT11_N_SUPPORT */

/* use daemon */
#define BNDSTRG_DAEMON
#define BND_STRG_MAX_TABLE_SIZE	64
#define BND_STRG_TIMER_PERIOD	1000
#define BND_STRG_AGE_TIME		150000
#define BND_STRG_HOLD_TIME		90000
#define BND_STRG_CHECK_TIME_5G	30000
#define BND_STRG_RSSI_DIFF		30
#define BND_STRG_RSSI_LOW		-70
#define BND_STRG_AUTO_ONOFF_THRD 4000
#define P_BND_STRG_TABLE	(&pAd->ApCfg.BndStrgTable)

#define BND_STRG_DBG
#define BND_STRG_QA

struct _BNDSTRG_OPS;

typedef struct _BND_STRG_ENTRY_STATISTICS{
	CHAR Rssi;
	UINT8 AuthReqCount;
} BND_STRG_ENTRY_STAT, *PBND_STRG_ENTRY_STAT;

typedef struct _BND_STRG_CLI_ENTRY{
	BOOLEAN bValid;
	UINT32 Control_Flags;
	ULONG   jiffies;		/* timestamp when insert-entry */
	UINT32  elapsed_time; /* ms */
	UCHAR Addr[MAC_ADDR_LEN];
	struct _BND_STRG_CLI_ENTRY *pNext;
} BND_STRG_CLI_ENTRY, *PBND_STRG_CLI_ENTRY;

/* for setting different band steering algorithms */
typedef struct _BND_STRG_ALG_CONTROL {
	UINT32 		FrameCheck;
	UINT32		ConditionCheck;
} BND_STRG_ALG_CONTROL, *PBND_STRG_ALG_CONTROL;

typedef struct _BND_STRG_CLI_TABLE{
	BOOLEAN bInitialized;
	BOOLEAN bEnabled;
	UINT32 Size;
	BND_STRG_ALG_CONTROL AlgCtrl;
	BND_STRG_CLI_ENTRY Entry[BND_STRG_MAX_TABLE_SIZE];
	PBND_STRG_CLI_ENTRY Hash[HASH_TABLE_SIZE];
	NDIS_SPIN_LOCK Lock;
	struct _BNDSTRG_OPS *Ops;
	VOID *priv;
	BOOLEAN b2GInfReady;
	BOOLEAN b5GInfReady;
	CHAR	RssiDiff;	/* if Rssi2.4G > Rssi5G by RssiDiff, then allow client to connect 2.4G */
	CHAR	RssiLow;	/* if Rssi5G < RssiLow, then this client cannot connect to 5G */
	UINT32	AgeTime;			/* Entry Age Time (ms) */
	UINT32	HoldTime;		/* Time for holding 2.4G connection rsp (ms) */
	UINT32	CheckTime_5G;	/* Time for deciding if a client is 2.4G only (ms) */
	RALINK_TIMER_STRUCT Timer;
#ifdef BND_STRG_DBG
	UCHAR MonitorAddr[MAC_ADDR_LEN];
#endif /* BND_STRG_DBG */
	UINT8		Band;
	UINT32 AutoOnOffThrd;   /* Threshold to auto turn bndstrg on/off by 2.4G false CCA */
	BOOLEAN bDaemonReady;
} BND_STRG_CLI_TABLE, *PBND_STRG_CLI_TABLE;

enum BND_STRG_RETURN_CODE {
	BND_STRG_SUCCESS = 0,
	BND_STRG_INVALID_ARG,
	BND_STRG_RESOURCE_ALLOC_FAIL,
	BND_STRG_TABLE_FULL,
	BND_STRG_TABLE_IS_NULL,
	BND_STRG_NOT_INITIALIZED,
	BND_STRG_2G_INF_NOT_READY,
	BND_STRG_5G_INF_NOT_READY,
	BND_STRG_STA_IS_CONNECTED,
	BND_STRG_UNEXP,
};

enum BND_STRG_CONTROL_FLAGS {
	fBND_STRG_CLIENT_SUPPORT_2G			= (1 << 0),
	fBND_STRG_CLIENT_SUPPORT_5G			= (1 << 1),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_2G	= (1 << 2),
	fBND_STRG_CLIENT_ALLOW_TO_CONNET_5G	= (1 << 3),
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_2G		= (1 << 4),	
	fBND_STRG_CLIENT_NOT_SUPPORT_HT_5G		= (1 << 5),
	fBND_STRG_CLIENT_LOW_RSSI_2G		= (1 << 6),
	fBND_STRG_CLIENT_LOW_RSSI_5G		= (1 << 7),
	fBND_STRG_CLIENT_IS_2G_ONLY			= (1 << 8),
	fBND_STRG_CLIENT_IS_5G_ONLY			= (1 << 9),
};

enum BND_STRG_FRAME_CHECK_FLAGS {
	fBND_STRG_FRM_CHK_PRB_REQ			= (1 << 0),
	fBND_STRG_FRM_CHK_ATH_REQ			= (1 << 1),
	fBND_STRG_FRM_CHK_ASS_REQ			= (1 << 2),
};

enum BND_STRG_CONDITION_CHECK_FLAGS {
	fBND_STRG_CND_RSSI_DIFF			= (1 << 0),
	fBND_STRG_CND_2G_PERSIST		= (1 << 1),
	fBND_STRG_CND_HT_SUPPORT		= (1 << 2),
	fBND_STRG_CND_5G_RSSI			= (1 << 3),
};

#define OID_BNDSTRG_MSG				0x0950
/* Use for I/O between driver and daemon */
/* Must make sure the structure is the same as the one in daemon */
typedef struct _BNDSTRG_MSG{
	UINT8	 Action;
	UINT8	 ReturnCode;
	UINT8	 TalbeIndex;
	BOOLEAN OnOff;
	UINT8	Band;
	BOOLEAN b2GInfReady;
	BOOLEAN b5GInfReady;
	CHAR 	Rssi[3];
	CHAR 	RssiDiff;
	CHAR 	RssiLow;
	UINT8	FrameType;
	UINT32	Time;
	UINT32	ConditionCheck;
	UCHAR 	Addr[MAC_ADDR_LEN];
} BNDSTRG_MSG, *PBNDSTRG_MSG;

typedef struct _BNDSTRG_CLI_EVENT{
	UCHAR		MacAddr[MAC_ADDR_LEN];
	UINT8		Action; /* add or delete table entry */
} BNDSTRG_CLI_EVENT, *PBNDSTRG_CLI_EVENT;

typedef struct _BNDSTRG_PROBE_EVENT{
	UCHAR		MacAddr[MAC_ADDR_LEN];
	UINT8		Band;
	UINT8		FrameType;
	CHAR		Rssi[3];
	BOOLEAN		bAuthCheck;
} BNDSTRG_PROBE_EVENT, *PBNDSTRG_PROBE_EVENT;

enum ACTION_CODE{
	CONNECTION_REQ = 1,
	CLI_ADD,
	CLI_DEL,
	CLI_AGING_REQ,
	CLI_AGING_RSP,
	INF_STATUS_QUERY,
	INF_STATUS_RSP_2G,
	INF_STATUS_RSP_5G,
	TABLE_INFO,
	ENTRY_LIST,
	BNDSTRG_ONOFF,
	SET_RSSI_DIFF,
	SET_RSSI_LOW,
	SET_AGE_TIME,
	SET_HOLD_TIME,
	SET_CHECK_TIME,
	SET_MNT_ADDR,
	SET_CHEK_CONDITIONS,
};



typedef struct _BNDSTRG_OPS {
	VOID (*ShowTableInfo)(
			PBND_STRG_CLI_TABLE table);
	
	VOID (*ShowTableEntries)(
			PBND_STRG_CLI_TABLE table);
	
	INT (*TableEntryAdd)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
			PBND_STRG_CLI_ENTRY *entry_out);
	
	INT (*TableEntryDel)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr,
			UINT32 Index);
	
	PBND_STRG_CLI_ENTRY (*TableLookup)(
			BND_STRG_CLI_TABLE *table,
			PUCHAR pAddr);
	
	BOOLEAN (*CheckConnectionReq)(
			struct _RTMP_ADAPTER *pAd,
			PUCHAR pSrcAddr,
			UINT8 FrameType,
			PCHAR Rssi);

	INT (*SetEnable)(
			PBND_STRG_CLI_TABLE table,
			BOOLEAN enable);

	INT (*SetRssiDiff)(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiDiff);

	INT (*SetRssiLow)(
			PBND_STRG_CLI_TABLE table,
			CHAR RssiLow);

	INT (*SetAgeTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);	

	INT (*SetHoldTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetCheckTime)(
			PBND_STRG_CLI_TABLE table,
			UINT32 Time);

	INT (*SetFrmChkFlag)(
			PBND_STRG_CLI_TABLE table,
			UINT32 FrmChkFlag);

	INT (*SetCndChkFlag)(
			PBND_STRG_CLI_TABLE table,
			UINT32 CndChkFlag);

#ifdef BND_STRG_DBG
	INT (*SetMntAddr)(
			PBND_STRG_CLI_TABLE table,
			PUCHAR Addr);
#endif /* BND_STRG_DBG */

	VOID (*MsgHandle)(
			struct _RTMP_ADAPTER *pAd,
			BNDSTRG_MSG *msg);
} BNDSTRG_OPS;

#endif /* BAND_STEERING */
#endif /* _BAND_STEERING_DEF_H_ */

