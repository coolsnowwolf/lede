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
	mt_txbf_cal.h
*/


#ifndef _RT_TXBF_CAL_H_
#define _RT_TXBF_CAL_H_

#define TX_PATH_2   2
#define TX_PATH_3   3
#define TX_PATH_4   4

#define GROUP_0     0
#define CH_001      1
#define CH_008      8
#define CH_014      14
#define GROUP_1     1
#define CH_184      184
#define CH_196      196
#define GROUP_2     2
#define CH_036      36
#define CH_044      44
#define GROUP_3     3
#define CH_052      52
#define CH_060      60
#define CH_068      68
#define GROUP_4     4
#define CH_072      72
#define CH_084      84
#define CH_092      92
#define GROUP_5     5
#define CH_096      96
#define CH_104      104
#define CH_112      112
#define GROUP_6     6
#define CH_116      116
#define CH_124      124
#define CH_136      136
#define GROUP_7     7
#define CH_140      140
#define CH_149      149
#define CH_157      157
#define GROUP_8     8
#define CH_161      161
#define CH_173      173
#define CH_181      181

#define GROUP_L     0
#define GROUP_M     1
#define GROUP_H     2

#define IBF_PHASE_EEPROM_START      0x220

#define IBF_LNA_PHASE_G0_ADDR       IBF_PHASE_EEPROM_START
#define IBF_LNA_PHASE_G1_ADDR       (IBF_LNA_PHASE_G0_ADDR  + 0x15)
#define IBF_LNA_PHASE_G2_ADDR       (IBF_LNA_PHASE_G1_ADDR  + 0x1E)
#define IBF_LNA_PHASE_G3_ADDR       (IBF_LNA_PHASE_G2_ADDR  + 0x1A)
#define IBF_LNA_PHASE_G4_ADDR       (IBF_LNA_PHASE_G3_ADDR  + 0x1E)
#define IBF_LNA_PHASE_G5_ADDR       (IBF_LNA_PHASE_G4_ADDR  + 0x1E)
#define IBF_LNA_PHASE_G6_ADDR       (IBF_LNA_PHASE_G5_ADDR  + 0x1E)
#define IBF_LNA_PHASE_G7_ADDR       (IBF_LNA_PHASE_G6_ADDR  + 0x1E)
#define IBF_LNA_PHASE_G8_ADDR       (IBF_PHASE_EEPROM_START + 0x120)

#define IBF_R0_H_G0                 0
#define IBF_R0_M_G0                 1
#define IBF_R0_L_G0                 2
#define IBF_R1_H_G0                 3
#define IBF_R1_M_G0                 4
#define IBF_R1_L_G0                 5
#define IBF_R2_H_G0                 6
#define IBF_R2_M_G0                 7
#define IBF_R2_L_G0                 8
#define IBF_R3_H_G0                 9
#define IBF_R3_M_G0                 10
#define IBF_R3_L_G0                 11
#define IBF_T0_M_G0                 12
#define IBF_T1_M_G0                 13
#define IBF_T2_M_G0                 14
#define IBF_T0_L_G0                 15
#define IBF_T1_L_G0                 16
#define IBF_T2_L_G0                 17
#define IBF_T0_H_G0                 18
#define IBF_T1_H_G0                 19
#define IBF_T2_H_G0                 20

#define IBF_R0_H_Gx                 0
#define IBF_R0_M_Gx                 1
#define IBF_R0_L_Gx                 2
#define IBF_R1_H_Gx                 3
#define IBF_R1_M_Gx                 4
#define IBF_R1_L_Gx                 5
#define IBF_R2_H_Gx                 6
#define IBF_R2_M_Gx                 7
#define IBF_R2_L_Gx                 8
#define IBF_R3_H_Gx                 9
#define IBF_R3_M_Gx                 10
#define IBF_R3_L_Gx                 11
#define IBF_R2_SX2_H_Gx             12
#define IBF_R2_SX2_M_Gx             13
#define IBF_R2_SX2_L_Gx             14
#define IBF_R3_SX2_H_Gx             15
#define IBF_R3_SX2_M_Gx             16
#define IBF_R3_SX2_L_Gx             17
#define IBF_T0_M_Gx                 18
#define IBF_T1_M_Gx                 19
#define IBF_T2_M_Gx                 20
#define IBF_T2_SX2_M_Gx             21
#define IBF_T0_L_Gx                 22
#define IBF_T1_L_Gx                 23
#define IBF_T2_L_Gx                 24
#define IBF_T2_SX2_L_Gx             25
#define IBF_T0_H_Gx                 26
#define IBF_T1_H_Gx                 27
#define IBF_T2_H_Gx                 28
#define IBF_T2_SX2_H_Gx             29

typedef enum _IBF_PHASE_E2P_UPDATE_TYPE {
	IBF_PHASE_ONE_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_UPDATE,
	IBF_PHASE_ALL_GROUP_ERASE,
	IBF_PHASE_ALL_GROUP_READ_FROM_E2P
} IBF_PHASE_E2P_UPDATE_TYPE;

typedef enum _IBF_PHASE_CAL_TYPE {
	IBF_PHASE_CAL_NOTHING,
	IBF_PHASE_CAL_NORMAL,
	IBF_PHASE_CAL_VERIFY,
	IBF_PHASE_CAL_NORMAL_INSTRUMENT,

	IBF_PHASE_CAL_VERIFY_INSTRUMENT

} IBF_PHASE_CAL_TYPE;

typedef enum _IBF_PHASE_STATUS_INSTRUMENT {
	STATUS_EBF_INVALID,
	STATUS_IBF_INVALID,
	STATUS_OTHER_ISSUE,
	STATUS_DONE
} IBF_PHASE_STATUS_INSTRUMENT;

#if defined(MT7615)
typedef struct _MT7615_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC3_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC3_M;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC3_H;
} MT7615_IBF_PHASE_OUT, *P_MT7615_IBF_PHASE_OUT;

typedef struct _MT7615_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_M_T0_H;
	UINT8 ucG0_M_T1_H;
	UINT8 ucG0_M_T2_H;
	UINT8 ucG0_L_T0_H;
	UINT8 ucG0_L_T1_H;
	UINT8 ucG0_L_T2_H;
	UINT8 ucG0_H_T0_H;
	UINT8 ucG0_H_T1_H;
	UINT8 ucG0_H_T2_H;
} MT7615_IBF_PHASE_G0_T, *P_MT7615_IBF_PHASE_G0_T;

typedef struct _MT7615_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_M_T0_H;
	UINT8 ucGx_M_T1_H;
	UINT8 ucGx_M_T2_H;
	UINT8 ucGx_M_T2_H_SX2;
	UINT8 ucGx_L_T0_H;
	UINT8 ucGx_L_T1_H;
	UINT8 ucGx_L_T2_H;
	UINT8 ucGx_L_T2_H_SX2;
	UINT8 ucGx_H_T0_H;
	UINT8 ucGx_H_T1_H;
	UINT8 ucGx_H_T2_H;
	UINT8 ucGx_H_T2_H_SX2;
} MT7615_IBF_PHASE_Gx_T, *P_MT7615_IBF_PHASE_Gx_T;
#endif

#if defined(MT7622)
typedef struct _MT7622_IBF_PHASE_OUT {
	UINT8 ucC0_L;
	UINT8 ucC1_L;
	UINT8 ucC2_L;
	UINT8 ucC0_M;
	UINT8 ucC1_M;
	UINT8 ucC2_M;
	UINT8 ucC0_H;
	UINT8 ucC1_H;
	UINT8 ucC2_H;
	UINT8 ucC0_UH;
	UINT8 ucC1_UH;
	UINT8 ucC2_UH;
	UINT8 ucC_L;
	UINT8 ucC_M;
	UINT8 ucC_H;
	UINT8 ucC_UH;
} MT7622_IBF_PHASE_OUT, *P_MT7622_IBF_PHASE_OUT;

typedef struct _MT7622_IBF_PHASE_G0_T {
	UINT8 ucG0_R0_UH;
	UINT8 ucG0_R0_H;
	UINT8 ucG0_R0_M;
	UINT8 ucG0_R0_L;
	UINT8 ucG0_R1_UH;
	UINT8 ucG0_R1_H;
	UINT8 ucG0_R1_M;
	UINT8 ucG0_R1_L;
	UINT8 ucG0_R2_UH;
	UINT8 ucG0_R2_H;
	UINT8 ucG0_R2_M;
	UINT8 ucG0_R2_L;
	UINT8 ucG0_R3_UH;
	UINT8 ucG0_R3_H;
	UINT8 ucG0_R3_M;
	UINT8 ucG0_R3_L;
	UINT8 ucG0_M_T0_UH;
	UINT8 ucG0_M_T1_UH;
	UINT8 ucG0_M_T2_UH;
	UINT8 ucG0_L_T0_UH;
	UINT8 ucG0_L_T1_UH;
	UINT8 ucG0_L_T2_UH;
	UINT8 ucG0_H_T0_UH;
	UINT8 ucG0_H_T1_UH;
	UINT8 ucG0_H_T2_UH;
} MT7622_IBF_PHASE_G0_T, *P_MT7622_IBF_PHASE_G0_T;

typedef struct _MT7622_IBF_PHASE_Gx_T {
	UINT8 ucGx_R0_UH;
	UINT8 ucGx_R0_H;
	UINT8 ucGx_R0_M;
	UINT8 ucGx_R0_L;
	UINT8 ucGx_R1_UH;
	UINT8 ucGx_R1_H;
	UINT8 ucGx_R1_M;
	UINT8 ucGx_R1_L;
	UINT8 ucGx_R2_UH;
	UINT8 ucGx_R2_H;
	UINT8 ucGx_R2_M;
	UINT8 ucGx_R2_L;
	UINT8 ucGx_R3_UH;
	UINT8 ucGx_R3_H;
	UINT8 ucGx_R3_M;
	UINT8 ucGx_R3_L;
	UINT8 ucGx_R2_UH_SX2;
	UINT8 ucGx_R2_H_SX2;
	UINT8 ucGx_R2_M_SX2;
	UINT8 ucGx_R2_L_SX2;
	UINT8 ucGx_R3_UH_SX2;
	UINT8 ucGx_R3_H_SX2;
	UINT8 ucGx_R3_M_SX2;
	UINT8 ucGx_R3_L_SX2;
	UINT8 ucGx_M_T0_UH;
	UINT8 ucGx_M_T1_UH;
	UINT8 ucGx_M_T2_UH;
	UINT8 ucGx_M_T2_UH_SX2;
	UINT8 ucGx_L_T0_UH;
	UINT8 ucGx_L_T1_UH;
	UINT8 ucGx_L_T2_UH;
	UINT8 ucGx_L_T2_UH_SX2;
	UINT8 ucGx_H_T0_UH;
	UINT8 ucGx_H_T1_UH;
	UINT8 ucGx_H_T2_UH;
	UINT8 ucGx_H_T2_UH_SX2;
} MT7622_IBF_PHASE_Gx_T, *P_MT7622_IBF_PHASE_Gx_T;
#endif

#if defined(MT7615)
VOID mt7615_iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID mt7615_iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7615_iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID mt7615_iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7615_iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID mt7615_iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);
#endif /* MT7615 */

#if defined(MT7622)
VOID mt7622_iBFPhaseComp(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR ucGroup,
						IN PCHAR pCmdBuf);

VOID mt7622_iBFPhaseCalInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7622_iBFPhaseFreeMem(IN struct _RTMP_ADAPTER *pAd);

VOID mt7622_iBFPhaseCalE2PInit(IN struct _RTMP_ADAPTER *pAd);

VOID mt7622_iBFPhaseCalE2PUpdate(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucUpdateAllType);

VOID mt7622_iBFPhaseCalReport(IN struct _RTMP_ADAPTER *pAd,
						IN UCHAR   ucGroupL_M_H,
						IN UCHAR   ucGroup,
						IN BOOLEAN fgSX2,
						IN UCHAR   ucStatus,
						IN UCHAR   ucPhaseCalType,
						IN PUCHAR  pBuf);
#endif /* MT7622 */
#endif /* _RT_TXBF_CAL_H_ */
