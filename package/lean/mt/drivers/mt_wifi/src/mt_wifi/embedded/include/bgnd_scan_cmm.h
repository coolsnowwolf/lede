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

    Abstract:


 */
#ifndef __BGND_SCAN_CMM_H__
#define __BGND_SCAN_CMM_H__
#define BGND_SCAN_MACHINE_BASE		0
#define BGND_SCAN_IDLE				0
#define BGND_SCAN_LISTEN			1
#define BGND_RDD_DETEC				2
#define BGND_CS_ANN					3
#define BGND_SCAN_WAIT				4
#define BGND_SCAN_MAX_STATE			5

#define BGND_SCAN_REQ				0
#define BGND_SCAN_TIMEOUT			1
#define BGND_SCAN_CNCL				2
#define BGND_SWITCH_CHANNEL			3
#define BGND_RDD_REQ				4
#define BGND_RDD_CNCL				5
#define BGND_RDD_TIMEOUT			6
#define BGND_OUTBAND_RADAR_FOUND               	7
#define BGND_OUTBAND_SWITCH               	8
#define BGND_CH_SW_ANN				9
#define BGND_PARTIAL_SCAN			10
#define BGND_SCAN_DONE				11
#define BGND_DEDICATE_RDD_REQ			12
#define BGND_SCAN_MAX_MSG			13

#define BGND_SCAN_FUNC_SIZE    (BGND_SCAN_MAX_STATE * BGND_SCAN_MAX_MSG)

#define DefaultBgndScanInterval		1800 /* 30min.  The auto trigger interval shoulde more than this value */
#define DefaultBgndScanPerChInterval	10 /* 10 seconds. Partial Scan interval */
#define DefaultNoisyThreshold		85 /* % */
#define DefaultChBusyTimeThreshold	600000
#define DefaultMyAirtimeUsageThreshold	100000
#define DefaultScanDuration		200 /*ms*/
#define B0IrpiSwCtrlOnlyOffset		29
#define B0IrpiSwCtrlResetOffset		18
#define B0IpiEnableCtrlOffset		12
#define B0IpiEnableCtrlValue		0x5
#define DefaultIdleTimeThreshold		80000 /*  *8us */

enum {
	TYPE_BGND_DISABLE_SCAN,
	TYPE_BGND_PARTIAL_SCAN,
	TYPE_BGND_CONTINUOUS_SCAN,
	TYPE_BGND_CONTINUOUS_SCAN_SWITCH_CH
};

typedef struct _BGND_SCAN_SUPP_CH_LIST {
	UCHAR GroupType; /*0:BW20, 1:BW40, 2:BW80 */
	UCHAR Channel;
	UCHAR CenChannel;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;
	BOOLEAN SkipChannel;
	UINT32 PccaTime;
	UINT32 SccaTime;
	UINT32 EDCCATime;
	UINT32 Band0TxTime;
	UINT32 Mdrdy;

	/*
		Channel property:

		CHANNEL_DISABLED: The channel is disabled.
		CHANNEL_PASSIVE_SCAN: Only passive scanning is allowed.
		CHANNEL_NO_IBSS: IBSS is not allowed.
		CHANNEL_RADAR: Radar detection is required.
		CHANNEL_NO_FAT_ABOVE: Extension channel above this channel is not allowed.
		CHANNEL_NO_FAT_BELOW: Extension channel below this channel is not allowed.
		CHANNEL_40M_CAP: 40 BW channel group
		CHANNEL_80M_CAP: 800 BW channel group
	 */
#define CHANNEL_DEFAULT_PROP	0x00
#define CHANNEL_DISABLED		0x01	/* no use */
#define CHANNEL_PASSIVE_SCAN	0x02
#define CHANNEL_NO_IBSS			0x04
#define CHANNEL_RADAR			0x08
#define CHANNEL_NO_FAT_ABOVE	0x10
#define CHANNEL_NO_FAT_BELOW	0x20
#define CHANNEL_40M_CAP			0x40
#define CHANNEL_80M_CAP			0x80

	UCHAR Flags;

} BGND_SCAN_SUPP_CH_LIST, *PBGND_SCAN_SUPP_CH_LIST;

typedef struct _BGND_SCAN_CH_GROUP_LIST {
	UCHAR BestCtrlChannel; /* Min in group */
	UCHAR CenChannel;
	UCHAR DfsReq;
	UCHAR RegulatoryDomain;
	UINT32 Max_PCCA_Time; /* Max in group */
	UINT32 Min_PCCA_Time; /* Min in group */
	UINT32 Band0_Tx_Time;
	BOOLEAN SkipGroup;
} BGND_SCAN_CH_GROUP_LIST, *PBGND_SCAN_CH_GROUP_LIST;

typedef struct _MT_BGND_SCAN_NOTIFY {
	UCHAR NotifyFunc; /* bit0 for notify RA, bit1 for MU, bit2 for BF, bit3 for Smart Ant. bit4 reserve. bit5~7 indicate TxStream */
	UCHAR BgndScanStatus;/* 0:stop, 1:start, 2:running */
} MT_BGND_SCAN_NOTIFY;

typedef struct _MT_BGND_SCAN_CFG {
	UCHAR ControlChannel;
	UCHAR CentralChannel;
	UCHAR Bw;
	UCHAR TxStream;
	UCHAR RxPath;
	UCHAR Reason;
	UCHAR BandIdx;
} MT_BGND_SCAN_CFG;
typedef struct _BACKGROUND_SCAN_CTRL {
	/*
		UCHAR ScanType;
		UCHAR BssType;
		UCHAR Channel;
		UCHAR SsidLen;
		CHAR Ssid[MAX_LEN_OF_SSID];
		UCHAR Bssid[MAC_ADDR_LEN];
	*/
	UCHAR					ScanBW;
	UCHAR					ChannelListNum;
	BOOL					IsABand;
	BOOL					BgndScanSupport;
	BGND_SCAN_SUPP_CH_LIST	BgndScanChList[MAX_NUM_OF_CHANNELS];	/* list all supported channels for background scan */
	/* BGND_SCAN_CHINFO		BgndScanChInfo; */
	UCHAR					ScanChannel; /* Current scan control channel */
	UCHAR					ScanCenChannel; /* Current scan centrol channel */
	STATE_MACHINE_FUNC		BgndScanFunc[BGND_SCAN_FUNC_SIZE];
	STATE_MACHINE			BgndScanStatMachine;

	RALINK_TIMER_STRUCT		BgndScanTimer;
	RALINK_TIMER_STRUCT		DfsZeroWaitTimer;
	UCHAR					RxPath;		/* RxPath for background scan */
	UCHAR					TxStream;	/* TxStream number for background scan */
	UCHAR					FirstChannel; /* Record first channle of channle list */
	UCHAR					ChannelIdx;
	UINT16					ScanDuration;
	BGND_SCAN_CH_GROUP_LIST	GroupChList[MAX_NUM_OF_CHANNELS];
	UCHAR					GroupChListNum;
	UCHAR					BestChannel;
	MT_SWITCH_CHANNEL_CFG	CurrentSwChCfg[1];
	UINT32				PartialScanInterval;
	UINT32				PartialScanIntervalCount;
	UINT32				BgndScanIntervalCount;
	UCHAR				Noisy;
	UCHAR				NoisyTH;
	UINT32				ChBusyTimeTH;
	UCHAR				DfsZeroWaitChannel;
	ULONG				DfsZeroWaitDuration;
	BOOL				SkipDfsChannel;
	BOOL				DfsZeroWaitSupport;
	BOOL				RadarDetected;
	UCHAR				BFSTARecord[MAX_LEN_OF_MAC_TABLE];
	BOOL				DriverTrigger;
	UINT8				ScanType; /* 0:Disable 1:partial scan 2:continuous scan */
	UINT8				SkipChannelNum;
	UCHAR				SkipChannelList[MAX_NUM_OF_CHANNELS];
	BOOL				IsSwitchChannel;
	UINT32				IPIIdleTime;
	UINT32				IPIIdleTimeTH;

} BACKGROUND_SCAN_CTRL, *PBACKGROUND_SCAN_CTRL;

#endif /* __BGND_SCAN_CMM_H__ */
