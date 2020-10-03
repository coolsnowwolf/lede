/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_txbf.c

	Abstract:
	Tx Beamforming related constants and data structures

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
	Shiang     2010/06/29
*/


#ifndef _RT_TXBF_H_
#define _RT_TXBF_H_

#ifdef TXBF_SUPPORT


/* #define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station */


/* TxSndgPkt Sounding type definitions */
#define SNDG_TYPE_DISABLE	0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP		2

/* Explicit TxBF feedback mechanism */
#define ETXBF_FB_DISABLE	0
#define ETXBF_FB_CSI		1
#define ETXBF_FB_NONCOMP	2
#define ETXBF_FB_COMP		4


/* #define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station */

/*
	eTxBfEnCond values:
	 0:no etxbf,
	 1:etxbf update periodically,
	 2:etxbf updated if mcs changes in RateSwitchingAdapt() or APQuickResponeForRateUpExecAdapt().
	 3:auto-selection: if mfb changes or timer expires, then send sounding packets <------not finished yet!!!
	 note:
		when = 1 or 3, NO_SNDG_CNT_THRD controls the frequency to update the
		matrix(ETXBF_EN_COND=1) or activate the whole bf evaluation process(not defined)
*/

/* Defines to include optional code. */
/* NOTE: Do not define these options. ETxBfEnCond==3 and */
/* MCS Feedback are not fully implemented */
/* #define ETXBF_EN_COND3_SUPPORT	// Include ETxBfEnCond==3 code */
/* #define MFB_SUPPORT				// Include MCS Feedback code */

/* MCS FB definitions */
#define MSI_TOGGLE_BF		6
#define TOGGLE_BF_PKTS		5/* the number of packets with inverted BF status */

/* TXBF State definitions */
#define READY_FOR_SNDG0		0/* jump to WAIT_SNDG_FB0 when channel change or periodically */
#define WAIT_SNDG_FB0		1/* jump to WAIT_SNDG_FB1 when bf report0 is received */
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5

#define NO_SNDG_CNT_THRD	0/* send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec. If this =0, bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt() */


/* ------------ BEAMFORMING PROFILE HANDLING ------------ */

#define IMP_MAX_BYTES		14		/* Implicit: 14 bytes per subcarrier */
#define IMP_MAX_BYTES_ONE_COL	7	/* Implicit: 7 bytes per subcarrier, when reading first column */
#define EXP_MAX_BYTES		18		/* Explicit: 18 bytes per subcarrier */
#define IMP_COEFF_SIZE		 9		/* 9 bits/coeff */
#define IMP_COEFF_MASK		0x1FF

#define PROFILE_MAX_CARRIERS_20		56		/* Number of subcarriers in 20 MHz mode */
#define PROFILE_MAX_CARRIERS_40		114		/* Number of subcarriers in 40 MHz mode */


/* Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz */
typedef struct {
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;


typedef struct {
	BOOLEAN impProfile;
	BOOLEAN fortyMHz;
	int rows, columns;
	int grouping;
	UCHAR tag[EXP_MAX_BYTES];
	UCHAR data[PROFILE_MAX_CARRIERS_40][EXP_MAX_BYTES];
} PROFILE_DATA;

extern PROFILE_DATA profData;



typedef
struct {
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
	UCHAR E0gBeg[2];
	UCHAR E0gEnd[2];
	UCHAR E0aLowBeg[2];
	UCHAR E0aLowEnd[2];
	UCHAR E0aMidBeg[2];
	UCHAR E0aMidEnd[2];
	UCHAR E0aHighBeg[2];
	UCHAR E0aHighEnd[2];
#endif

#ifdef MT76x2
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
#endif
} ITXBF_PHASE_PARAMS;			/* ITxBF BBP reg phase calibration parameters */

typedef
struct {
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
	UCHAR E0gBeg[2];
	UCHAR E0gEnd[2];
	UCHAR E0aLowBeg[2];
	UCHAR E0aLowEnd[2];
	UCHAR E0aMidBeg[2];
	UCHAR E0aMidEnd[2];
	UCHAR E0aHighBeg[2];
	UCHAR E0aHighEnd[2];
#endif

#ifdef MT76x2
	UCHAR E1gBeg[3];
	UCHAR E1gEnd[3];
	UCHAR E1aHighBeg[3];
	UCHAR E1aHighEnd[3];
	UCHAR E1aLowBeg[3];
	UCHAR E1aLowEnd[3];
	UCHAR E1aMidBeg[3];
	UCHAR E1aMidMid[3];
	UCHAR E1aMidEnd[3];
#endif
} ITXBF_LNA_PARAMS;			/* ITxBF BBP reg LNA calibration parameters */

typedef
struct {
#if defined(RT2883) || defined(RT3883) || defined(RT3593)
	UCHAR E0gBeg[2];
	UCHAR E0gEnd[2];
	UCHAR E0aLow[2];
	UCHAR E0aMid[2];
	UCHAR E0aHigh[2];
#endif

#ifdef MT76x2
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
#endif
} ITXBF_DIV_PARAMS;				/* ITxBF Divider Calibration parameters */

void ITxBFGetEEPROM(
	IN RTMP_ADAPTER * pAd,
	IN ITXBF_PHASE_PARAMS * phaseParams,
	IN ITXBF_LNA_PARAMS * lnaParams,
	IN ITXBF_DIV_PARAMS * divParams);


INT ITxBFDividerCalibration(
	IN RTMP_ADAPTER * pAd,
	IN int calFunction,
	IN int calMethod,
	OUT UCHAR *divPhase);

VOID ITxBFLoadLNAComp(
	IN RTMP_ADAPTER * pAd);

int ITxBFLNACalibration(
	IN RTMP_ADAPTER * pAd,
	IN int calFunction,
	IN int calMethod,
	IN BOOLEAN gBand);


void Read_TxBfProfile(
	IN	RTMP_ADAPTER * pAd,
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum,
	IN	BOOLEAN			implicitProfile);

void Write_TxBfProfile(
	IN	RTMP_ADAPTER * pAd,
	IN	PROFILE_DATA	*prof,
	IN	int				profileNum);

void Read_TagField(
	IN	PRTMP_ADAPTER	pAd,
	IN  UCHAR	*row,
	IN  int		profileNum);

/* Write_TagField - write a profile tagfield */
void Write_TagField(
	IN	RTMP_ADAPTER * pAd,
	IN  UCHAR	*row,
	IN  int		profileNum);

/* displayTagfield - display one tagfield */
void displayTagfield(
	IN	RTMP_ADAPTER * pAd,
	IN	int		profileNum,
	IN	BOOLEAN implicitProfile);

/* Unpack an ITxBF matrix element from a row of bytes */
int Unpack_IBFValue(
	IN UCHAR *row,
	IN int elemNum);

int iCalcCalibration(
	IN RTMP_ADAPTER * pAd,
	IN int calParams[2],
	IN int profileNum);

void ITxBFSetEEPROM(
	IN RTMP_ADAPTER * pAd,
	IN ITXBF_PHASE_PARAMS * phaseParams,
	IN ITXBF_LNA_PARAMS * lnaParams,
	IN ITXBF_DIV_PARAMS * divParams);

#endif /* TXBF_SUPPORT // */

#endif /* _RT_TXBF_H_ */
