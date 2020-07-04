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
    dot11n_ht.h
 
    Abstract:
	Defined IE/frame structures of 802.11n
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
*/

#ifdef DOT11_N_SUPPORT


#ifndef _DOT11N_HT_H_
#define _DOT11N_HT_H_

#include "rtmp_type.h"
#include "dot11_base.h"

/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_CAP_INFO{
#ifdef RT_BIG_ENDIAN
	UINT16	LSIGTxopProSup:1;
	UINT16	Forty_Mhz_Intolerant:1;
	UINT16	PSMP:1;
	UINT16	CCKmodein40:1;
	UINT16	AMsduSize:1;
	UINT16	DelayedBA:1;
	UINT16	RxSTBC:2;
	UINT16	TxSTBC:1;
	UINT16	ShortGIfor40:1;
	UINT16	ShortGIfor20:1;
	UINT16	GF:1;
	UINT16	MimoPs:2;
	UINT16	ChannelWidth:1;
	UINT16	ht_rx_ldpc:1;
#else
	UINT16	ht_rx_ldpc:1;
	UINT16	ChannelWidth:1;
	UINT16	MimoPs:2;		/* mimo power safe */
	UINT16	GF:1;			/* green field */
	UINT16	ShortGIfor20:1;
	UINT16	ShortGIfor40:1;	/* for40MHz */
	UINT16	TxSTBC:1;		/* 0:not supported,  1:if supported */
	UINT16	RxSTBC:2;
	UINT16	DelayedBA:1;
	UINT16	AMsduSize:1;	/* only support as zero */
	UINT16	CCKmodein40:1;
	UINT16	PSMP:1;
	UINT16	Forty_Mhz_Intolerant:1;
	UINT16	LSIGTxopProSup:1;
#endif /* RT_BIG_ENDIAN */
} HT_CAP_INFO;


/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_CAP_PARM{
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:3;/*momi power safe */
	UINT8	MpduDensity:3;
	UINT8	MaxRAmpduFactor:2;
#else
	UINT8	MaxRAmpduFactor:2;
	UINT8	MpduDensity:3;
	UINT8	rsv:3;/*momi power safe */
#endif /* RT_BIG_ENDIAN */
} HT_CAP_PARM, *PHT_CAP_PARM;


typedef struct GNU_PACKED _HT_MCS_SET_TX_SUBFIELD{
#ifdef RT_BIG_ENDIAN
	UINT8	TxMCSSetDefined:1; 
	UINT8	TxRxNotEqual:1;
	UINT8	TxMaxStream:2;
	UINT8	TxUnqualModulation:1;
	UINT8	rsv:3;
#else
	UINT8	rsv:3;
	UINT8	TxUnqualModulation:1;
	UINT8	TxMaxStream:2;
	UINT8	TxRxNotEqual:1;
	UINT8	TxMCSSetDefined:1;
#endif /* RT_BIG_ENDIAN */
}HT_MCS_SET_TX_SUBFIELD, *PHT_MCS_SET_TX_SUBFIELD;


/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _HT_MCS_SET{
	UINT8	MCSSet[10];
	UINT8	SupRate[2];  /* unit : 1Mbps */
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:3;
	UINT8	MpduDensity:1;
	UINT8	TxStream:2;
	UINT8	TxRxNotEqual:1;
	UINT8	TxMCSSetDefined:1; 
#else
	UINT8	TxMCSSetDefined:1; 
	UINT8	TxRxNotEqual:1;
	UINT8	TxStream:2;
	UINT8	MpduDensity:1;
	UINT8	rsv:3;
#endif /* RT_BIG_ENDIAN */
	UINT8	rsv3[3];  
} HT_MCS_SET, *PHT_MCS_SET;

/*  HT Capability INFO field in HT Cap IE . */
typedef struct GNU_PACKED _EXT_HT_CAP_INFO{
#ifdef RT_BIG_ENDIAN 
	UINT16	rsv2:4;
	UINT16	RDGSupport:1;	/*reverse Direction Grant  support */
	UINT16	PlusHTC:1;	/*+HTC control field support */
	UINT16	MCSFeedback:2;	/*0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	UINT16	rsv:5;/*momi power safe */
	UINT16	TranTime:2;
	UINT16	Pco:1;
#else
	UINT16	Pco:1;
	UINT16	TranTime:2;
	UINT16	rsv:5;/*momi power safe */
	UINT16	MCSFeedback:2;	/*0:no MCS feedback, 2:unsolicited MCS feedback, 3:Full MCS feedback,  1:rsv. */
	UINT16	PlusHTC:1;	/*+HTC control field support */
	UINT16	RDGSupport:1;	/*reverse Direction Grant  support */
	UINT16	rsv2:4;
#endif /* RT_BIG_ENDIAN */
} EXT_HT_CAP_INFO, *PEXT_HT_CAP_INFO;


/* HT Explicit Beamforming Feedback Capable */
#define HT_ExBF_FB_CAP_NONE			0
#define HT_ExBF_FB_CAP_DELAYED		1
#define HT_ExBF_FB_CAP_IMMEDIATE		2
#define HT_ExBF_FB_CAP_BOTH			3

/* HT Beamforming field in HT Cap IE */
typedef struct GNU_PACKED _HT_BF_CAP{
#ifdef RT_BIG_ENDIAN
	UINT32	rsv:3;
	UINT32	ChanEstimation:2;
	UINT32	CSIRowBFSup:2;
	UINT32	ComSteerBFAntSup:2;
	UINT32	NoComSteerBFAntSup:2;
	UINT32	CSIBFAntSup:2;
	UINT32	MinGrouping:2;
	UINT32	ExpComBF:2;
	UINT32	ExpNoComBF:2;
	UINT32	ExpCSIFbk:2;
	UINT32	ExpComSteerCapable:1;
	UINT32	ExpNoComSteerCapable:1;
	UINT32	ExpCSICapable:1;
	UINT32	Calibration:2;
	UINT32	ImpTxBFCapable:1;
	UINT32	TxNDPCapable:1;	
	UINT32	RxNDPCapable:1;	
	UINT32	TxSoundCapable:1;
	UINT32	RxSoundCapable:1;
	UINT32	TxBFRecCapable:1;
#else
	UINT32	TxBFRecCapable:1;
	UINT32	RxSoundCapable:1;
	UINT32	TxSoundCapable:1;
	UINT32	RxNDPCapable:1;	
	UINT32	TxNDPCapable:1;	
	UINT32	ImpTxBFCapable:1;	
	UINT32	Calibration:2;
	UINT32	ExpCSICapable:1;
	UINT32	ExpNoComSteerCapable:1;
	UINT32	ExpComSteerCapable:1;
	UINT32	ExpCSIFbk:2;	
	UINT32	ExpNoComBF:2;	
	UINT32	ExpComBF:2;	
	UINT32	MinGrouping:2;
	UINT32	CSIBFAntSup:2;
	UINT32	NoComSteerBFAntSup:2;
	UINT32	ComSteerBFAntSup:2;
	UINT32	CSIRowBFSup:2;
	UINT32	ChanEstimation:2;
	UINT32	rsv:3;
#endif /* RT_BIG_ENDIAN */
} HT_BF_CAP, *PHT_BF_CAP;

/*  HT antenna selection field in HT Cap IE . */
typedef struct GNU_PACKED _HT_AS_CAP{
#ifdef RT_BIG_ENDIAN
	UINT8	rsv:1;
	UINT8	TxSoundPPDU:1;
	UINT8	RxASel:1;
	UINT8	AntIndFbk:1;
	UINT8	ExpCSIFbk:1;
	UINT8	AntIndFbkTxASEL:1;
	UINT8	ExpCSIFbkTxASEL:1;
	UINT8	AntSelect:1; 
#else
	UINT8	AntSelect:1; 
	UINT8	ExpCSIFbkTxASEL:1;
	UINT8	AntIndFbkTxASEL:1;
	UINT8	ExpCSIFbk:1;
	UINT8	AntIndFbk:1;
	UINT8	RxASel:1;
	UINT8	TxSoundPPDU:1;
	UINT8	rsv:1;
#endif /* RT_BIG_ENDIAN */
} HT_AS_CAP, *PHT_AS_CAP;


/* Draft 1.0 set IE length 26, but is extensible.. */
#define SIZE_HT_CAP_IE		26
/* The structure for HT Capability IE. */
typedef struct GNU_PACKED _HT_CAPABILITY_IE{
	HT_CAP_INFO		HtCapInfo;
	HT_CAP_PARM		HtCapParm;
/*	HT_MCS_SET		HtMCSSet; */
	UCHAR			MCSSet[16];
	EXT_HT_CAP_INFO	ExtHtCapInfo;
	HT_BF_CAP		TxBFCap;	/* beamforming cap. rt2860c not support beamforming. */
	HT_AS_CAP		ASCap;	/*antenna selection. */
} HT_CAPABILITY_IE, *PHT_CAPABILITY_IE;


/*   field in Addtional HT Information IE . */
typedef struct GNU_PACKED _ADD_HTINFO{
#ifdef RT_BIG_ENDIAN
	UCHAR	SerInterGranu:3;
	UCHAR	S_PSMPSup:1;
	UCHAR	RifsMode:1; 
	UCHAR	RecomWidth:1;	
	UCHAR	ExtChanOffset:2;
#else
	UCHAR	ExtChanOffset:2;
	UCHAR	RecomWidth:1;
	UCHAR	RifsMode:1; 
	UCHAR	S_PSMPSup:1;	 /*Indicate support for scheduled PSMP */
	UCHAR	SerInterGranu:3;	 /*service interval granularity */
#endif
} ADD_HTINFO, *PADD_HTINFO;


typedef struct GNU_PACKED _ADD_HTINFO2{
#ifdef RT_BIG_ENDIAN
	USHORT	rsv2:11; 
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv:1;
	USHORT	NonGfPresent:1;
	USHORT	OperaionMode:2;	
#else
	USHORT	OperaionMode:2;
	USHORT	NonGfPresent:1;
	USHORT	rsv:1; 
	USHORT	OBSS_NonHTExist:1;
	USHORT	rsv2:11; 
#endif
} ADD_HTINFO2, *PADD_HTINFO2;


/* TODO: Need sync with spec about the definition of StbcMcs. In Draft 3.03, it's reserved. */
typedef struct GNU_PACKED _ADD_HTINFO3{
#ifdef RT_BIG_ENDIAN
	USHORT	rsv:4;
	USHORT	PcoPhase:1;
	USHORT	PcoActive:1;
	USHORT	LsigTxopProt:1;
	USHORT	STBCBeacon:1;
	USHORT	DualCTSProtect:1;
	USHORT	DualBeacon:1;
	USHORT	StbcMcs:6;
#else
	USHORT	StbcMcs:6;
	USHORT	DualBeacon:1;
	USHORT	DualCTSProtect:1; 
	USHORT	STBCBeacon:1;
	USHORT	LsigTxopProt:1;	/* L-SIG TXOP protection full support */
	USHORT	PcoActive:1; 
	USHORT	PcoPhase:1; 
	USHORT	rsv:4; 
#endif /* RT_BIG_ENDIAN */
} ADD_HTINFO3, *PADD_HTINFO3;

#define SIZE_ADD_HT_INFO_IE		22
typedef struct  GNU_PACKED _ADD_HT_INFO_IE{
	UCHAR				ControlChan;
	ADD_HTINFO			AddHtInfo;
	ADD_HTINFO2			AddHtInfo2;	 
	ADD_HTINFO3			AddHtInfo3;	 
	UCHAR				MCSSet[16];		/* Basic MCS set */
} ADD_HT_INFO_IE, *PADD_HT_INFO_IE;


/* 802.11n draft3 related structure definitions. */
/* 7.3.2.60 */
#define dot11OBSSScanPassiveDwell							20	/* in TU. min amount of time that the STA continously scans each channel when performing an active OBSS scan. */
#define dot11OBSSScanActiveDwell							10	/* in TU.min amount of time that the STA continously scans each channel when performing an passive OBSS scan. */
#define dot11BSSWidthTriggerScanInterval					300  /* in sec. max interval between scan operations to be performed to detect BSS channel width trigger events. */
#define dot11OBSSScanPassiveTotalPerChannel					200	/* in TU. min total amount of time that the STA scans each channel when performing a passive OBSS scan. */
#define dot11OBSSScanActiveTotalPerChannel					20	/*in TU. min total amount of time that the STA scans each channel when performing a active OBSS scan */
#define dot11BSSWidthChannelTransactionDelayFactor			5	/* min ratio between the delay time in performing a switch from 20MHz BSS to 20/40 BSS operation and the maximum */
																/*	interval between overlapping BSS scan operations. */
#define dot11BSSScanActivityThreshold						25	/* in %%, max total time that a STA may be active on the medium during a period of */
																/*	(dot11BSSWidthChannelTransactionDelayFactor * dot11BSSWidthTriggerScanInterval) seconds without */
																/*	being obligated to perform OBSS Scan operations. default is 25(== 0.25%) */

typedef struct GNU_PACKED _OVERLAP_BSS_SCAN_IE{
	USHORT		ScanPassiveDwell;
	USHORT		ScanActiveDwell;
	USHORT		TriggerScanInt;				/* Trigger scan interval */
	USHORT		PassiveTalPerChannel;		/* passive total per channel */
	USHORT		ActiveTalPerChannel;		/* active total per channel */
	USHORT		DelayFactor;				/* BSS width channel transition delay factor */
	USHORT		ScanActThre;				/* Scan Activity threshold */
}OVERLAP_BSS_SCAN_IE, *POVERLAP_BSS_SCAN_IE;


/*  7.3.2.56. 20/40 Coexistence element used in  Element ID = 72 = IE_2040_BSS_COEXIST */
typedef union GNU_PACKED _BSS_2040_COEXIST_IE{
 struct GNU_PACKED {
 #ifdef RT_BIG_ENDIAN
	UCHAR	rsv:3;
 	UCHAR	ObssScanExempGrant:1;
	UCHAR	ObssScanExempReq:1;
	UCHAR	BSS20WidthReq:1;
	UCHAR	Intolerant40:1;	
	UCHAR	InfoReq:1; 
 #else
	UCHAR	InfoReq:1;
	UCHAR	Intolerant40:1;			/* Inter-BSS. set 1 when prohibits a receiving BSS from operating as a 20/40 Mhz BSS. */
	UCHAR	BSS20WidthReq:1;		/* Intra-BSS set 1 when prohibits a receiving AP from operating its BSS as a 20/40MHz BSS. */
	UCHAR	ObssScanExempReq:1;
	UCHAR	ObssScanExempGrant:1;
	UCHAR	rsv:3;
#endif /* RT_BIG_ENDIAN */
    } field;
 UCHAR   word;
} BSS_2040_COEXIST_IE, *PBSS_2040_COEXIST_IE;


#endif /* _DOT11N_HT_H_ */
#endif /* DOT11_N_SUPPORT */

