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
	mt_txbf.h
*/


#ifndef _RT_TXBF_H_
#define _RT_TXBF_H_

#ifdef TXBF_SUPPORT

#define TXBF_DYNAMIC_DISABLE
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _VHT_CAP_INFO;

#ifdef TXBF_DYNAMIC_DISABLE
/*  used in rStaRecBf.ucAutoSoundingCtrl
 *   if set, BF is dynamically disabled
 */
#define DYNAMIC_TXBF_DISABLE	BIT(6)
#endif /* TXBF_DYNAMIC_DISABLE */

#define Sportan_DBG 0

/* Divider phase calibration closed loop definition */
#define RX0TX0     0
#define RX1TX1     5

#define ADC0_RX0_2R   8
#define ADC1_RX1_2R   8

/* #define MRQ_FORCE_TX		//Force MRQ regardless the capability of the station */


/* TxSndgPkt Sounding type definitions */
#define SNDG_TYPE_DISABLE		0
#define SNDG_TYPE_SOUNDING	1
#define SNDG_TYPE_NDP			2

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
#define TOGGLE_BF_PKTS		5    /* the number of packets with inverted BF status */

/* TXBF State definitions */
#define READY_FOR_SNDG0	    0    /* jump to WAIT_SNDG_FB0 when channel change or periodically */
#define WAIT_SNDG_FB0		1    /* jump to WAIT_SNDG_FB1 when bf report0 is received */
#define WAIT_SNDG_FB1		2
#define WAIT_MFB			3
#define WAIT_USELESS_RSP	4
#define WAIT_BEST_SNDG		5

#define NO_SNDG_CNT_THRD	0    /* send sndg packet if there is no sounding for (NO_SNDG_CNT_THRD+1)*500msec. If this =0, bf matrix is updated at each call of APMlmeDynamicTxRateSwitchingAdapt() */


/* ------------ BEAMFORMING PROFILE HANDLING ------------ */

#define IMP_MAX_BYTES		    14  /* Implicit: 14 bytes per subcarrier */
#define IMP_MAX_BYTES_ONE_COL	7   /* Implicit: 7 bytes per subcarrier, when reading first column */
#define EXP_MAX_BYTES		    18  /* Explicit: 18 bytes per subcarrier */
#define MAX_BYTES		2   /* 2 bytes per subcarrier for implicit and explicit TxBf */
#define IMP_COEFF_SIZE		    9   /* 9 bits/coeff */
#define IMP_COEFF_MASK		    0x1FF

#define PROFILE_MAX_CARRIERS_20	56	/* Number of subcarriers in 20 MHz mode */
#define PROFILE_MAX_CARRIERS_40	114	/* Number of subcarriers in 40 MHz mode */
#define PROFILE_MAX_CARRIERS_80	242	/* Number of subcarriers in 80 MHz mode */

#define NUM_CHAIN			      3

#define TXBF_PFMU_ARRAY_SIZE      64
#define MAX_PFMU_MEM_LEN_PER_ROW  6

#define STATUS_TRUE               0

#define TX_MCS_SET_DEFINED                          BIT(0)
#define TX_MCS_SET_DEFINED_OFFSET                   0
#define TX_RX_MCS_SET_N_EQUAL                       BIT(1)
#define TX_RX_MCS_SET_N_EQUAL_OFFSET                1
#define TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED        BITS(2, 3)
#define TX_MAX_NUM_SPATIAL_STREAMS_SUPPORTED_OFFSET 2

/*
 * BF Chip Capability
 */
#define TXBF_HW_CAP                                 BIT(0)
#define TXBF_AID_HW_LIMIT                           BIT(1)
#define TXBF_HW_2BF                                 BIT(2)

#ifdef MT_MAC
typedef enum _TXBF_DBW_T {
	P_DBW20M = 0,
	P_DBW40M,
	P_DBW80M,
	P_DBW160M,
	DBW20M,
	DBW40M,
	DBW80M,
	DBW160M
} TXBF_DBW_T;

enum ENUM_SUBF_CAP_T {
	SUBF_OFF,
	SUBF_ALL,
	SUBF_BFER,
	SUBF_BFEE
};

typedef struct _VENDOR_BF_SETTING {
	BOOLEAN	fgIsBrcm2GeTxBFIe;
	UINT8	Nrow;
} VENDOR_BF_SETTING, *P_VENDOR_BF_SETTING;

typedef struct _TXBF_PFMU_STA_INFO {
	UINT_16   u2PfmuId;   /* 0xFFFF means no access right for PFMU */
	BOOLEAN   fgSU_MU;    /* 0 : SU, 1 : MU */
	BOOLEAN   fgETxBfCap; /* 0 : ITxBf, 1 : ETxBf */
	UINT_8    ucSoundingPhy; /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNdpaRate;
	UINT_8    ucNdpRate;
	UINT_8    ucReptPollRate;
	UINT_8    ucTxMode;  /* 0: legacy, 1: OFDM, 2: HT, 4: VHT */
	UINT_8    ucNc;
	UINT_8    ucNr;
	UINT_8    ucCBW;     /* 0 : 20M, 1 : 40M, 2 : 80M, 3 : 80 + 80M */
	UINT_8    ucTotMemRequire;
	UINT_8    ucMemRequire20M;
	UINT_8    ucMemRow0;
	UINT_8    ucMemCol0;
	UINT_8    ucMemRow1;
	UINT_8    ucMemCol1;
	UINT_8    ucMemRow2;
	UINT_8    ucMemCol2;
	UINT_8    ucMemRow3;
	UINT_8    ucMemCol3;
	UINT_16   u2SmartAnt;
	UINT_8    ucSEIdx;
	UINT_8    ucAutoSoundingCtrl; /* Bit7: low traffic indicator, Bit6: Stop sounding for this entry, Bit5~0: postpone sounding */
	UINT_8    uciBfTimeOut;
	UINT_8    uciBfDBW;
	UINT_8    uciBfNcol;
	UINT_8    uciBfNrow;
} TXBF_PFMU_STA_INFO, *P_TXBF_PFMU_STA_INFO;

typedef struct _CMD_STAREC_BF {
	UINT_16 u2Tag;      /* Tag = 0x02 */
	UINT_16 u2Length;
	TXBF_PFMU_STA_INFO	rTxBfPfmuInfo;
	UINT_8  ucReserved[2];
} CMD_STAREC_BF, *P_CMD_STAREC_BF;

#ifdef RT_BIG_ENDIAN
typedef union _PFMU_PROFILE_TAG1 {
	struct {
		/* DWORD0 */
		UINT_32 ucMemAddr2RowIdx    : 5; /* [31 : 27] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2ColIdx    : 3; /* [26 : 24] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr1RowIdx    : 6; /* [23 : 18] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr1ColIdx    : 3; /* [17 : 15] : column index : 0 ~ 5 */
		UINT_32 ucRMSD              : 3; /* [14:12]   : RMSD value from CE */
		UINT_32 ucInvalidProf       : 1; /* [11]      : 0:default, 1: This profile number is invalid by SW */
		UINT_32 ucSU_MU             : 1; /* [10]      : 0:SU, 1: MU */
		UINT_32 ucDBW               : 2; /* [9:8]     : 0/1/2/3: DW20/40/80/160NC */
		UINT_32 ucTxBf              : 1; /* [7]       : 0: iBF, 1: eBF */
		UINT_32 ucProfileID         : 7; /* [6:0]     : 0 ~ 63 */

		/* DWORD1 */
		UINT_32 ucReserved1         : 1; /* [63]      : Reserved */
		UINT_32 ucHtcExist          : 1; /* [62]      : HtcExist */
		UINT_32 ucCodeBook          : 2; /* [61:60]   : Code book */
		UINT_32 ucLM                : 2; /* [59 : 58] : 0/1/2 */
		UINT_32 ucNgroup            : 2; /* [57 : 56] : Ngroup */
		UINT_32 ucNcol              : 2; /* [55 : 54] : Ncol */
		UINT_32 ucNrow              : 2; /* [53 : 52] : Nrow */
		UINT_32 ucReserved          : 1; /* [51]      : Reserved */
		UINT_32 ucMemAddr4RowIdx    : 6; /* [50 : 45] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr4ColIdx    : 3; /* [44 : 42] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr3RowIdx    : 6; /* [41 : 36] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr3ColIdx    : 3; /* [35 : 33] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr2RowIdxMsb : 1; /* [32]      : MSB of row index */

		/* DWORD2 */
		UINT_32 ucSNR_STS3          : 8; /* [95:88]   : SNR_STS3 */
		UINT_32 ucSNR_STS2          : 8; /* [87:80]   : SNR_STS2 */
		UINT_32 ucSNR_STS1          : 8; /* [79:72]   : SNR_STS1 */
		UINT_32 ucSNR_STS0          : 8; /* [71:64]   : SNR_STS0 */

		/* DWORD3 */
		UINT_32 ucReserved2         : 24; /* reserved */
		UINT_32 ucIBfLnaIdx         : 8; /* [103:96]  : iBF LNA index */
	} rField;
	UINT_32 au4RawData[4];
} PFMU_PROFILE_TAG1, *P_PFMU_PROFILE_TAG1;

typedef union _PFMU_PROFILE_TAG2 {
	struct {
		/* DWORD0 */
		UINT_32 ucMCSThS1SS      : 4; /* [31:28]  : MCS TH short 1SS */
		UINT_32 ucMCSThL1SS      : 4; /* [27:24]  : MCS TH long 1SS */
		UINT_32 ucReserved1      : 1; /* [23]     : Reserved */
		UINT_32 ucRMSDThd        : 3; /* [22:20]  : RMSD Threshold */
		UINT_32 ucSEIdx          : 5; /* [19:15]  : SE index */
		UINT_32 ucReserved0      : 3; /* [14:12]  : Reserved */
		UINT_32 u2SmartAnt       : 12;/* [11:0]   : Smart Ant config */

		/* DWORD1 */
		UINT_32 ucReserved2      : 8; /* [63:56]  : Reserved */
		UINT_32 uciBfTimeOut     : 8; /* [55:48]  : iBF timeout limit */
		UINT_32 ucMCSThS3SS      : 4; /* [47:44]  : MCS TH short 3SS */
		UINT_32 ucMCSThL3SS      : 4; /* [43:40]  : MCS TH long 3SS */
		UINT_32 ucMCSThS2SS      : 4; /* [39:36]  : MCS TH short 2SS */
		UINT_32 ucMCSThL2SS      : 4; /* [35:32]  : MCS TH long 2SS */

		/* DWORD2 */
		UINT_32 u2Reserved5      : 10;/* [95:86]  : Reserved */
		UINT_32 uciBfNrow        : 2; /* [85:84]  : iBF desired Nrow = 1 ~ 4 */
		UINT_32 uciBfNcol        : 2; /* [83:82]  : iBF desired Ncol = 1 ~ 3 */
		UINT_32 uciBfDBW         : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT_32 ucReserved4      : 8; /* [79:72]  : Reserved */
		UINT_32 ucReserved3      : 8; /* [71:64]  : Reserved */
	} rField;
	UINT_32 au4RawData[3];
} PFMU_PROFILE_TAG2, *P_PFMU_PROFILE_TAG2;

typedef union _PFMU_PN {
	struct {
		/* DWORD0 */
		UINT_32 u2CMM_1STS_Tx2   : 10;
		UINT_32 u2CMM_1STS_Tx1   : 11;
		UINT_32 u2CMM_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2CMM_2STS_Tx1   : 9;
		UINT_32 u2CMM_2STS_Tx0   : 11;
		UINT_32 u2CMM_1STS_Tx3   : 11;
		UINT_32 u2CMM_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2CMM_3STS_Tx0   : 8;
		UINT_32 u2CMM_2STS_Tx3   : 11;
		UINT_32 u2CMM_2STS_Tx2   : 11;
		UINT_32 u2CMM_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2CMM_3STS_Tx3   : 7;
		UINT_32 u2CMM_3STS_Tx2   : 11;
		UINT_32 u2CMM_3STS_Tx1   : 11;
		UINT_32 u2CMM_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved         : 28;
		UINT_32 u2CMM_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN, *P_PFMU_PN;

typedef union _PFMU_PN_DBW20 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW20_1STS_Tx2   : 10;
		UINT_32 u2DBW20_1STS_Tx1   : 11;
		UINT_32 u2DBW20_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW20_2STS_Tx1   : 9;
		UINT_32 u2DBW20_2STS_Tx0   : 11;
		UINT_32 u2DBW20_1STS_Tx3   : 11;
		UINT_32 u2DBW20_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW20_3STS_Tx0   : 8;
		UINT_32 u2DBW20_2STS_Tx3   : 11;
		UINT_32 u2DBW20_2STS_Tx2   : 11;
		UINT_32 u2DBW20_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW20_3STS_Tx3   : 7;
		UINT_32 u2DBW20_3STS_Tx2   : 11;
		UINT_32 u2DBW20_3STS_Tx1   : 11;
		UINT_32 u2DBW20_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW20_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW20M, *P_PFMU_PN_DBW20M;

typedef union _PFMU_PN_DBW40 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW40_1STS_Tx2   : 10;
		UINT_32 u2DBW40_1STS_Tx1   : 11;
		UINT_32 u2DBW40_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW40_2STS_Tx1   : 9;
		UINT_32 u2DBW40_2STS_Tx0   : 11;
		UINT_32 u2DBW40_1STS_Tx3   : 11;
		UINT_32 u2DBW40_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW40_3STS_Tx0   : 8;
		UINT_32 u2DBW40_2STS_Tx3   : 11;
		UINT_32 u2DBW40_2STS_Tx2   : 11;
		UINT_32 u2DBW40_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW40_3STS_Tx3   : 7;
		UINT_32 u2DBW40_3STS_Tx2   : 11;
		UINT_32 u2DBW40_3STS_Tx1   : 11;
		UINT_32 u2DBW40_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW40_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW40M, *P_PFMU_PN_DBW40M;

typedef union _PFMU_PN_DBW80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW80_1STS_Tx2   : 10;
		UINT_32 u2DBW80_1STS_Tx1   : 11;
		UINT_32 u2DBW80_1STS_Tx0   : 11;

		/* DWORD1 */
		UINT_32 u2DBW80_2STS_Tx1   : 9;
		UINT_32 u2DBW80_2STS_Tx0   : 11;
		UINT_32 u2DBW80_1STS_Tx3   : 11;
		UINT_32 u2DBW80_1STS_Tx2Msb : 1;

		/* DWORD2 */
		UINT_32 u2DBW80_3STS_Tx0   : 8;
		UINT_32 u2DBW80_2STS_Tx3   : 11;
		UINT_32 u2DBW80_2STS_Tx2   : 11;
		UINT_32 u2DBW80_2STS_Tx1Msb : 2;

		/* DWORD3 */
		UINT_32 u2DBW80_3STS_Tx3   : 7;
		UINT_32 u2DBW80_3STS_Tx2   : 11;
		UINT_32 u2DBW80_3STS_Tx1   : 11;
		UINT_32 u2DBW80_3STS_Tx0Msb : 3;

		/* DWORD4 */
		UINT_32 reserved           : 28;
		UINT_32 u2DBW80_3STS_Tx3Msb : 4;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW80M, *P_PFMU_PN_DBW80M;

typedef union _PFMU_PN_DBW80_80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW160_2STS_Tx0  : 10;
		UINT_32 u2DBW160_1STS_Tx1  : 11;
		UINT_32 u2DBW160_1STS_Tx0  : 11;

		/* DWORD1 */
		UINT_32 reserved           : 20;
		UINT_32 u2DBW160_2STS_Tx1  : 11;
		UINT_32 u2DBW160_2STS_Tx0Msb : 1;
	} rField;
	UINT_32 au4RawData[2];
} PFMU_PN_DBW80_80M, *P_PFMU_PN_DBW80_80M;

typedef union _PFMU_DATA {
	struct {
		/* DWORD0 */
		UINT_32 ucPsi31          : 7;
		UINT_32 u2Phi21          : 9;
		UINT_32 ucPsi21          : 7;
		UINT_32 u2Phi11          : 9;

		/* DWORD1 */
		UINT_32 ucPsi32          : 7;
		UINT_32 u2Phi22          : 9;
		UINT_32 ucPsi41          : 7;
		UINT_32 u2Phi31          : 9;

		/* DWORD2 */
		UINT_32 ucPsi43          : 7;
		UINT_32 u2Phi33          : 9;
		UINT_32 ucPsi42          : 7;
		UINT_32 u2Phi32          : 9;

		/* DWORD3 */
		UINT_32 u2Reserved       : 16;
		UINT_32 u2dSNR03         : 4;
		UINT_32 u2dSNR02         : 4;
		UINT_32 u2dSNR01         : 4;
		UINT_32 u2dSNR00         : 4;

		/* DWORD4 */
		UINT_32 u4Reserved;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_DATA, *P_PFMU_DATA;

#else

typedef union _PFMU_PROFILE_TAG1 {
	struct {
		/* DWORD0 */
		UINT_32 ucProfileID         : 7; /* [6:0]     : 0 ~ 63 */
		UINT_32 ucTxBf              : 1; /* [7]       : 0: iBF, 1: eBF */
		UINT_32 ucDBW               : 2; /* [9:8]     : 0/1/2/3: DW20/40/80/160NC */
		UINT_32 ucSU_MU             : 1; /* [10]      : 0:SU, 1: MU */
		UINT_32 ucInvalidProf       : 1; /* [11]      : 0:default, 1: This profile number is invalid by SW */
		UINT_32 ucRMSD              : 3; /* [14:12]   : RMSD value from CE */
		UINT_32 ucMemAddr1ColIdx    : 3; /* [17 : 15] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr1RowIdx    : 6; /* [23 : 18] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2ColIdx    : 3; /* [26 : 24] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr2RowIdx    : 5; /* [31 : 27] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr2RowIdxMsb : 1; /* [32]      : MSB of row index */
		UINT_32 ucMemAddr3ColIdx    : 3; /* [35 : 33] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr3RowIdx    : 6; /* [41 : 36] : row index : 0 ~ 63 */
		UINT_32 ucMemAddr4ColIdx    : 3; /* [44 : 42] : column index : 0 ~ 5 */
		UINT_32 ucMemAddr4RowIdx    : 6; /* [50 : 45] : row index : 0 ~ 63 */
		UINT_32 ucReserved          : 1; /* [51]      : Reserved */
		UINT_32 ucNrow              : 2; /* [53 : 52] : Nrow */
		UINT_32 ucNcol              : 2; /* [55 : 54] : Ncol */
		UINT_32 ucNgroup            : 2; /* [57 : 56] : Ngroup */
		UINT_32 ucLM                : 2; /* [59 : 58] : 0/1/2 */
		UINT_32 ucCodeBook          : 2; /* [61:60]   : Code book */
		UINT_32 ucHtcExist          : 1; /* [62]      : HtcExist */
		UINT_32 ucReserved1         : 1; /* [63]      : Reserved */
		/* DWORD2 */
		UINT_32 ucSNR_STS0          : 8; /* [71:64]   : SNR_STS0 */
		UINT_32 ucSNR_STS1          : 8; /* [79:72]   : SNR_STS1 */
		UINT_32 ucSNR_STS2          : 8; /* [87:80]   : SNR_STS2 */
		UINT_32 ucSNR_STS3          : 8; /* [95:88]   : SNR_STS3 */
		/* DWORD3 */
		UINT_32 ucIBfLnaIdx         : 8; /* [103:96]  : iBF LNA index */
		UINT_32 ucReserved2         : 24; /*     : Reserved */
	} rField;
	UINT_32 au4RawData[4];
} PFMU_PROFILE_TAG1, *P_PFMU_PROFILE_TAG1;

typedef union _PFMU_PROFILE_TAG2 {
	struct {
		/* DWORD0 */
		UINT_32 u2SmartAnt       : 12;/* [11:0]   : Smart Ant config */
		UINT_32 ucReserved0      : 3; /* [14:12]  : Reserved */
		UINT_32 ucSEIdx          : 5; /* [19:15]  : SE index */
		UINT_32 ucRMSDThd        : 3; /* [22:20]  : RMSD Threshold */
		UINT_32 ucReserved1      : 1; /* [23]     : Reserved */
		UINT_32 ucMCSThL1SS      : 4; /* [27:24]  : MCS TH long 1SS */
		UINT_32 ucMCSThS1SS      : 4; /* [31:28]  : MCS TH short 1SS */
		/* DWORD1 */
		UINT_32 ucMCSThL2SS      : 4; /* [35:32]  : MCS TH long 2SS */
		UINT_32 ucMCSThS2SS      : 4; /* [39:36]  : MCS TH short 2SS */
		UINT_32 ucMCSThL3SS      : 4; /* [43:40]  : MCS TH long 3SS */
		UINT_32 ucMCSThS3SS      : 4; /* [47:44]  : MCS TH short 3SS */
		UINT_32 uciBfTimeOut     : 8; /* [55:48]  : iBF timeout limit */
		UINT_32 ucReserved2      : 8; /* [63:56]  : Reserved */
		/* DWORD2 */
		UINT_32 ucReserved3      : 8; /* [71:64]  : Reserved */
		UINT_32 ucReserved4      : 8; /* [79:72]  : Reserved */
		UINT_32 uciBfDBW         : 2; /* [81:80]  : iBF desired DBW 0/1/2/3 : BW20/40/80/160NC */
		UINT_32 uciBfNcol        : 2; /* [83:82]  : iBF desired Ncol = 1 ~ 3 */
		UINT_32 uciBfNrow        : 2; /* [85:84]  : iBF desired Nrow = 1 ~ 4 */
		UINT_32 u2Reserved5      : 10;/* [95:86]  : Reserved */
	} rField;
	UINT_32 au4RawData[3];
} PFMU_PROFILE_TAG2, *P_PFMU_PROFILE_TAG2;

typedef union _PFMU_PN {
	struct {
		/* DWORD0 */
		UINT_32 u2CMM_1STS_Tx0   : 11;
		UINT_32 u2CMM_1STS_Tx1   : 11;
		UINT_32 u2CMM_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2CMM_1STS_Tx2Msb : 1;
		UINT_32 u2CMM_1STS_Tx3   : 11;
		UINT_32 u2CMM_2STS_Tx0   : 11;
		UINT_32 u2CMM_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2CMM_2STS_Tx1Msb : 2;
		UINT_32 u2CMM_2STS_Tx2   : 11;
		UINT_32 u2CMM_2STS_Tx3   : 11;
		UINT_32 u2CMM_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2CMM_3STS_Tx0Msb : 3;
		UINT_32 u2CMM_3STS_Tx1   : 11;
		UINT_32 u2CMM_3STS_Tx2   : 11;
		UINT_32 u2CMM_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2CMM_3STS_Tx3Msb : 4;
		UINT_32 reserved         : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN, *P_PFMU_PN;

typedef union _PFMU_PN_DBW20 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW20_1STS_Tx0   : 11;
		UINT_32 u2DBW20_1STS_Tx1   : 11;
		UINT_32 u2DBW20_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW20_1STS_Tx2Msb : 1;
		UINT_32 u2DBW20_1STS_Tx3   : 11;
		UINT_32 u2DBW20_2STS_Tx0   : 11;
		UINT_32 u2DBW20_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW20_2STS_Tx1Msb : 2;
		UINT_32 u2DBW20_2STS_Tx2   : 11;
		UINT_32 u2DBW20_2STS_Tx3   : 11;
		UINT_32 u2DBW20_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW20_3STS_Tx0Msb : 3;
		UINT_32 u2DBW20_3STS_Tx1   : 11;
		UINT_32 u2DBW20_3STS_Tx2   : 11;
		UINT_32 u2DBW20_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW20_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW20M, *P_PFMU_PN_DBW20M;

typedef union _PFMU_PN_DBW40 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW40_1STS_Tx0   : 11;
		UINT_32 u2DBW40_1STS_Tx1   : 11;
		UINT_32 u2DBW40_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW40_1STS_Tx2Msb : 1;
		UINT_32 u2DBW40_1STS_Tx3   : 11;
		UINT_32 u2DBW40_2STS_Tx0   : 11;
		UINT_32 u2DBW40_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW40_2STS_Tx1Msb : 2;
		UINT_32 u2DBW40_2STS_Tx2   : 11;
		UINT_32 u2DBW40_2STS_Tx3   : 11;
		UINT_32 u2DBW40_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW40_3STS_Tx0Msb : 3;
		UINT_32 u2DBW40_3STS_Tx1   : 11;
		UINT_32 u2DBW40_3STS_Tx2   : 11;
		UINT_32 u2DBW40_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW40_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW40M, *P_PFMU_PN_DBW40M;

typedef union _PFMU_PN_DBW80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW80_1STS_Tx0   : 11;
		UINT_32 u2DBW80_1STS_Tx1   : 11;
		UINT_32 u2DBW80_1STS_Tx2   : 10;
		/* DWORD1 */
		UINT_32 u2DBW80_1STS_Tx2Msb : 1;
		UINT_32 u2DBW80_1STS_Tx3   : 11;
		UINT_32 u2DBW80_2STS_Tx0   : 11;
		UINT_32 u2DBW80_2STS_Tx1   : 9;
		/* DWORD2 */
		UINT_32 u2DBW80_2STS_Tx1Msb : 2;
		UINT_32 u2DBW80_2STS_Tx2   : 11;
		UINT_32 u2DBW80_2STS_Tx3   : 11;
		UINT_32 u2DBW80_3STS_Tx0   : 8;
		/* DWORD3 */
		UINT_32 u2DBW80_3STS_Tx0Msb : 3;
		UINT_32 u2DBW80_3STS_Tx1   : 11;
		UINT_32 u2DBW80_3STS_Tx2   : 11;
		UINT_32 u2DBW80_3STS_Tx3   : 7;
		/* DWORD4 */
		UINT_32 u2DBW80_3STS_Tx3Msb : 4;
		UINT_32 reserved           : 28;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_PN_DBW80M, *P_PFMU_PN_DBW80M;

typedef union _PFMU_PN_DBW80_80 {
	struct {
		/* DWORD0 */
		UINT_32 u2DBW160_1STS_Tx0  : 11;
		UINT_32 u2DBW160_1STS_Tx1  : 11;
		UINT_32 u2DBW160_2STS_Tx0  : 10;
		/* DWORD1 */
		UINT_32 u2DBW160_2STS_Tx0Msb : 1;
		UINT_32 u2DBW160_2STS_Tx1  : 11;
		UINT_32 reserved           : 20;
	} rField;
	UINT_32 au4RawData[2];
} PFMU_PN_DBW80_80M, *P_PFMU_PN_DBW80_80M;

typedef union _PFMU_DATA {
	struct {
		/* DWORD0 */
		UINT_32 u2Phi11          : 9;
		UINT_32 ucPsi21          : 7;
		UINT_32 u2Phi21          : 9;
		UINT_32 ucPsi31          : 7;
		/* DWORD1 */
		UINT_32 u2Phi31          : 9;
		UINT_32 ucPsi41          : 7;
		UINT_32 u2Phi22          : 9;
		UINT_32 ucPsi32          : 7;
		/* DWORD2 */
		UINT_32 u2Phi32          : 9;
		UINT_32 ucPsi42          : 7;
		UINT_32 u2Phi33          : 9;
		UINT_32 ucPsi43          : 7;
		/* DWORD3 */
		UINT_32 u2dSNR00         : 4;
		UINT_32 u2dSNR01         : 4;
		UINT_32 u2dSNR02         : 4;
		UINT_32 u2dSNR03         : 4;
		UINT_32 u2Reserved       : 16;
	} rField;
	UINT_32 au4RawData[5];
} PFMU_DATA, *P_PFMU_DATA;
#endif
typedef struct _PFMU_HALF_DATA {
	UINT_16 u2SubCarrIdx;
	INT_16  i2Phi11;
	INT_16  i2Phi21;
	INT_16  i2Phi31;
} PFMU_HALF_DATA, *P_PFMU_HALF_DATA;

typedef struct _BF_QD {
	UINT_32 u4qd[14];
} BF_QD, *P_BF_QD;


typedef enum _BF_SOUNDING_MODE {
	SU_SOUNDING = 0,
	MU_SOUNDING,
	SU_PERIODIC_SOUNDING,
	MU_PERIODIC_SOUNDING,
	BF_PROCESSING,
} BF_SOUNDING_MODE;
#endif /* #ifdef MT_MAC */

typedef enum _BF_ACTION_CATEGORY {
	BF_SOUNDING_OFF = 0,
	BF_SOUNDING_ON,
	BF_DATA_PACKET_APPLY,
	BF_PFMU_MEM_ALLOCATE,
	BF_PFMU_MEM_RELEASE,
	BF_PFMU_TAG_READ,
	BF_PFMU_TAG_WRITE,
	BF_PROFILE_READ,
	BF_PROFILE_WRITE,
	BF_PN_READ,
	BF_PN_WRITE,
	BF_PFMU_MEM_ALLOC_MAP_READ,
	BF_AID_SET,
	BF_STA_REC_READ,
	BF_PHASE_CALIBRATION,
	BF_IBF_PHASE_COMP,
	BF_LNA_GAIN_CONFIG,
	BF_PROFILE_WRITE_20M_ALL,
	BF_APCLIENT_CLUSTER,
	BF_AWARE_CTRL,
	BF_HW_ENABLE_STATUS_UPDATE,
	BF_REPT_CLONED_STA_TO_NORMAL_STA,
	BF_GET_QD,
	BF_BFEE_HW_CTRL,
	BF_PFMU_SW_TAG_WRITE,
	BF_MOD_EN_CTRL
} BF_ACTION_CATEGORY;

/* Indices of valid rows in Implicit and Explicit profiles for 20 and 40 MHz */
typedef struct {
	int lwb1, upb1;
	int lwb2, upb2;
} SC_TABLE_ENTRY;


typedef
struct {
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
} ITXBF_PHASE_PARAMS;			/* ITxBF BBP reg phase calibration parameters */

typedef
struct {
	UCHAR E1gBeg[3];
	UCHAR E1gEnd[3];
	UCHAR E1aHighBeg[3];
	UCHAR E1aHighEnd[3];
	UCHAR E1aLowBeg[3];
	UCHAR E1aLowEnd[3];
	UCHAR E1aMidBeg[3];
	UCHAR E1aMidMid[3];
	UCHAR E1aMidEnd[3];
} ITXBF_LNA_PARAMS;			/* ITxBF BBP reg LNA calibration parameters */

typedef
struct {
	UCHAR E1gBeg;
	UCHAR E1gEnd;
	UCHAR E1aHighBeg;
	UCHAR E1aHighEnd;
	UCHAR E1aLowBeg;
	UCHAR E1aLowEnd;
	UCHAR E1aMidBeg;
	UCHAR E1aMidMid;
	UCHAR E1aMidEnd;
} ITXBF_DIV_PARAMS;				/* ITxBF Divider Calibration parameters */


typedef struct _TXBF_STATUS_INFO {
	UCHAR               ucPhyMode;
	UCHAR               ucBW;
	USHORT              u2Channel;
	UCHAR               ucTxPathNum;
	UCHAR               ucRxPathNum;
	UCHAR               ucETxBfTxEn;
	UCHAR               ucITxBfTxEn;
	UCHAR               ucNDPARate;
	UCHAR               ucNDPRate;
	UCHAR               ucWcid;
	UINT32              u4WTBL1;
	UINT32              u4WTBL2;
	HT_BF_CAP           *pHtTxBFCap;
#ifdef VHT_TXBF_SUPPORT
	struct _VHT_CAP_INFO *pVhtTxBFCap;
#endif
	ULONG               cmmCfgITxBfTimeout;
	ULONG               cmmCfgETxBfTimeout;
	ULONG	            cmmCfgETxBfEnCond;		/* Enable sending of sounding and beamforming */
	BOOLEAN	            cmmCfgETxBfNoncompress;	/* Force non-compressed Sounding Response */
	BOOLEAN	            cmmCfgETxBfIncapable;		/* Report Incapable of BF in TX BF Capabilities */

} TXBF_STATUS_INFO;

typedef struct {
	/* ETxBF */
	UCHAR		bfState;
	UCHAR		sndgMcs;
	UCHAR		sndgBW;
	UCHAR		sndg0Mcs;
	INT			sndg0Snr0, sndg0Snr1, sndg0Snr2;

	UCHAR		noSndgCnt;
	UCHAR		eTxBfEnCond;
	UCHAR		noSndgCntThrd, ndpSndgStreams;
	UCHAR		iTxBfEn;

} TXBF_MAC_TABLE_ENTRY;

VOID mt_TxBFInit(
	IN struct _RTMP_ADAPTER   *pAd,
	IN TXBF_STATUS_INFO * pTxBfInfo,
	IN TXBF_MAC_TABLE_ENTRY * pEntryTxBf,
	IN HT_BF_CAP              *pTxBFCap,
	IN BOOLEAN			      supportsETxBF);

VOID mt_TxBFFwInit(
	IN struct _RTMP_ADAPTER  *pAd);

VOID mt_WrapTxBFInit(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct _MAC_TABLE_ENTRY	*pEntry,
	IN IE_LISTS * ie_list,
	IN BOOLEAN			     supportsETxBF);

BOOLEAN mt_clientSupportsETxBF(
	IN struct _RTMP_ADAPTER   *pAd,
	IN HT_BF_CAP              *pTxBFCap,
	IN BOOLEAN                ETxBfNoncompress);

BOOLEAN mt_WrapClientSupportsETxBF(
	IN struct _RTMP_ADAPTER  *pAd,
	IN HT_BF_CAP             *pTxBFCap);

void mt_WrapSetETxBFCap(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct wifi_dev *wdev,
	IN HT_BF_CAP       *pTxBFCap);

#ifdef VHT_TXBF_SUPPORT
BOOLEAN mt_clientSupportsVhtETxBF(
	IN struct _RTMP_ADAPTER   *pAd,
	IN VHT_CAP_INFO * pTxBFCap);

BOOLEAN mt_WrapClientSupportsVhtETxBF(
	IN struct _RTMP_ADAPTER  *pAd,
	IN VHT_CAP_INFO * pTxBFCap);

void mt_WrapSetVHTETxBFCap(
	IN struct _RTMP_ADAPTER  *pAd,
	IN struct wifi_dev *wdev,
	IN struct _VHT_CAP_INFO  *pTxBFCap);
#endif /* VHT_TXBF_SUPPORT */


VOID TxBfProfileTag_PfmuIdx(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucProfileID);

VOID TxBfProfileTag_TxBfType(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucTxBf);

VOID TxBfProfileTag_DBW(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucBw);

VOID TxBfProfileTag_SuMu(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucSuMu);

VOID TxBfProfileTag_InValid(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR InvalidFlg);

VOID TxBfProfileTag_Mem(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN PUCHAR pMemAddrColIdx,
	IN PUCHAR pMemAddrRowIdx);

VOID TxBfProfileTag_Matrix(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucNrow,
	IN UCHAR ucNcol,
	IN UCHAR ucNgroup,
	IN UCHAR ucLM,
	IN UCHAR ucCodeBook,
	IN UCHAR ucHtcExist);

VOID TxBfProfileTag_SNR(
	IN P_PFMU_PROFILE_TAG1 prPfmuTag1,
	IN UCHAR ucSNR_STS0,
	IN UCHAR ucSNR_STS1,
	IN UCHAR ucSNR_STS2,
	IN UCHAR ucSNR_STS3);

VOID TxBfProfileTag_SmtAnt(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN USHORT u2SmartAnt);

VOID TxBfProfileTag_SeIdx(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR ucSEIdx);

VOID TxBfProfileTag_RmsdThd(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR ucRMSDThd);

VOID TxBfProfileTag_McsThd(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN PUCHAR pMCSThLSS,
	IN PUCHAR pMCSThSSS);

VOID TxBfProfileTag_TimeOut(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfTimeOut);

VOID TxBfProfileTag_DesiredBW(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfDBW);

VOID TxBfProfileTag_DesiredNc(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfNcol);

VOID TxBfProfileTag_DesiredNr(
	IN P_PFMU_PROFILE_TAG2 prPfmuTag2,
	IN UCHAR uciBfNrow);

INT TxBfProfileTagRead(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		PfmuIdx,
	IN BOOLEAN              fgBFer);

INT TxBfProfileTagWrite(
	IN struct _RTMP_ADAPTER *pAd,
	IN P_PFMU_PROFILE_TAG1  prPfmuTag1,
	IN P_PFMU_PROFILE_TAG2  prPfmuTag2,
	IN UCHAR		profileIdx);

VOID TxBfProfileTagPrint(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN              fgBFer,
	IN PUCHAR               pBuf);

INT TxBfProfilePnRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 profileIdx);

INT TxBfProfilePnWrite(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR                PfmuIdx,
	IN UCHAR                ucBw,
	IN PUCHAR		pProfileData);

VOID TxBfProfilePnPrint(
	IN UCHAR                 ucBw,
	IN PUCHAR                pBuf);

INT TxBfProfileDataRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 profileIdx,
	IN BOOLEAN               fgBFer,
	IN USHORT                subCarrIdx);

INT TxBfProfileDataWrite(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR                 profileIdx,
	IN USHORT                subCarrIdx,
	IN PUCHAR                pProfileData);

INT TxBfProfileDataWrite20MAll(
	IN  struct _RTMP_ADAPTER *pAd,
	IN UCHAR                 profileIdx,
	IN PUCHAR                pProfileData);

VOID TxBfProfileDataPrint(
	IN  struct _RTMP_ADAPTER *pAd,
	IN USHORT                 subCarrIdx,
	IN PUCHAR                 pBuf);

INT TxBfQdRead(
	IN  struct _RTMP_ADAPTER *pAd,
	IN INT8                  subCarrIdx);

INT mt_Trigger_Sounding_Packet(
	IN  struct _RTMP_ADAPTER *pAd,
	IN    UCHAR              SndgEn,
	IN    UINT32             u4SNDPeriod,
	IN    UCHAR              ucSu_Mu,
	IN    UCHAR              ucMuNum,
	IN    PUCHAR             pWlanId);

VOID TxBfProfileMemAllocMap(
	IN PUCHAR                pBuf);

BOOLEAN TxBfModuleEnCtrl(
	IN  struct _RTMP_ADAPTER *pAd);

#ifdef TXBF_DYNAMIC_DISABLE
INT DynamicTxBfDisable(
	IN  struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN               fgDisable);
#endif /* TXBF_DYNAMIC_DISABLE */

VOID StaRecBfUpdate(
	IN struct _MAC_TABLE_ENTRY *pEntry,
	IN P_CMD_STAREC_BF        pCmdStaRecBf);

VOID StaRecBfRead(
	IN struct _RTMP_ADAPTER *pAd,
	IN PUCHAR pBuf);

INT32 mt_AsicBfStaRecUpdate(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR                ucPhyMode,
	IN UCHAR                ucBssIdx,
	IN UCHAR                ucWlanIdx);

INT32 mt_AsicBfStaRecRelease(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR                ucBssIdx,
	IN UCHAR                ucWlanIdx);

VOID mt_AsicClientBfCap(
	IN struct _RTMP_ADAPTER *pAd,
	IN struct _MAC_TABLE_ENTRY *pEntry);

UCHAR AsicTxBfEnCondProc(
	IN	struct _RTMP_ADAPTER *pAd,
	IN	TXBF_STATUS_INFO * pTxBfInfo);


/* displayTagfield - display one tagfield */
void displayTagfield(
	IN struct _RTMP_ADAPTER  *pAd,
	IN	int		profileNum,
	IN	BOOLEAN implicitProfile);

#endif /* TXBF_SUPPORT // */

UINT32 starec_txbf_feature_decision(struct wifi_dev *wdev, struct _MAC_TABLE_ENTRY *entry, UINT32 *feature);


#endif /* _RT_TXBF_H_ */

