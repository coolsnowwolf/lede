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


#ifndef __SCS_H__
#define __SCS_H__

#define TriggerTrafficeTh							250000/*2M*/
#define MAX_LEN_OF_SCS_BSS_TABLE					6
#define MinRssiTolerance                         	10
#define ThTolerance                                 10
#define PdBlkEnabeOffset							19
#define RTSDropCntEnabeOffset						0
#define RTSDropRdClrEnabeOffset						8
#define RtsDropCountOffset							16
#define RTSDropCountMask							0x0000ffff
#define PdBlkEnabeOffsetB1							25
#define PdBlkOfmdThMask								0x1ff
#define PdBlkOfmdThOffset							20
#define PdBlkOfmdThOffsetB1							16 /* Band1 */
#define PdBlkCckThMask								0xff
#define PdBlkCckThOffset							1
#define PdBlkCck1RThOffset							24
#define PdBlkOfmdThDefault							0x13c
#define PdBlkCckThDefault							0x92 /*-110dBm*/
#define BandNum										1/*MT7615 HW not support DBDC/MT7622 only Band0*/
#define BssNum										4
#define BssOffset									0x10
#define RtsRtyCountOffset							16
#define RtsCountMask								0x0000ffff
#define TxTrafficTh									9
/* #define BandOffset		0x200 */
#define OneStep										2/* dB */
#define FastInitTh									0xa6/* -90dBm */
#define FastInitThOfdm								0x14c/* -90dBm */
#define FalseCcaUpBondDefault						500
#define FalseCcaLowBondDefault						50
#define CckFixedRssiBondDefault						184/* -72dBm. -72+256 */
#define OfdmFixedRssiBondDefault					368/* -72dBm. (-72*2)+512 */

enum {
	SCS_DISABLE,
	SCS_ENABLE,
	SCS_MANUAL,
};

enum {
	PD_BLOCKING_OFF,
	PD_BLOCKING_ON,
};

enum {
	SCS_Gen1,/* old chip */
	SCS_Gen2,/* MT7615   */
	SCS_Gen3,/* MT7622   */
};

typedef struct _SCS_BSS_ENTRY {
	UCHAR Bssid[MAC_ADDR_LEN];
	/* UCHAR Channel; */
} SCS_BSS_ENTRY;

typedef struct {
	UCHAR           BssNr;
	UCHAR           BssOverlapNr;
	SCS_BSS_ENTRY       BssEntry[MAX_LEN_OF_SCS_BSS_TABLE]; /* Number is 6 */
} SCS_BSS_TABLE, *PSCS_BSS_TABLE;

/*
IPI Level							Power(dBm)
    0							IPI<=-92
    1							-92<IPI<=-89
    2							-89<IPI<=-86
    :							:
    9							-60<IPI<=-55
    10							-55<IPI
*/

typedef struct {
	UINT32           IPI_Histogram[11];
} IPI_TABLE, *PIPI_TABLE; /* idel power indicator */

typedef struct _SMART_CARRIER_SENSE_CTRL {
	UINT8	SCSGeneration;/* 2:MT7615 3:MT7622*/
	BOOL	SCSEnable[DBDC_BAND_NUM];	/* 0:Disable, 1:Enable */
	/* SCS_BSS_TABLE	SCSBssTab2G; */		/* store AP information for SCS */
	/* SCS_BSS_TABLE	SCSBssTab5G; */		/* store AP information for SCS */
	UINT8	SCSStatus[DBDC_BAND_NUM];/* 0: Normal, 1:Low_gain */
	CHAR	SCSMinRssi[DBDC_BAND_NUM];
	UINT32	SCSTrafficThreshold[DBDC_BAND_NUM]; /* Traffic Threshold */
	/* UINT8                    NoiseEnv[DBDC_BAND_NUM]; */
	/* IPI_TABLE               Ipi2G; */
	/* IPI_TABLE               Ipi5G; */
	UINT32	OneSecTxByteCount[DBDC_BAND_NUM];
	UINT32	OneSecRxByteCount[DBDC_BAND_NUM];
	INT32	CckPdBlkTh[DBDC_BAND_NUM];
	INT32	OfdmPdBlkTh[DBDC_BAND_NUM];
	INT32	SCSMinRssiTolerance[DBDC_BAND_NUM];
	INT32	SCSThTolerance[DBDC_BAND_NUM];
	UCHAR	OfdmPdSupport[DBDC_BAND_NUM];
	BOOL	ForceScsOff[DBDC_BAND_NUM];
	UINT32	RtsCount[DBDC_BAND_NUM];
	UINT32	RtsRtyCount[DBDC_BAND_NUM];
	/* MT7622  support Band0*/
	UINT32	RTS_MPDU_DROP_CNT;
	UINT32	Retry_MPDU_DROP_CNT;
	UINT32	LTO_MPDU_DROP_CNT;
	/*------------------------*/
	UINT32	CckFalseCcaCount[DBDC_BAND_NUM];
	UINT32	OfdmFalseCcaCount[DBDC_BAND_NUM];
	UINT16	CckFalseCcaUpBond[DBDC_BAND_NUM];
	UINT16	CckFalseCcaLowBond[DBDC_BAND_NUM];
	UINT16	OfdmFalseCcaUpBond[DBDC_BAND_NUM];
	UINT16	OfdmFalseCcaLowBond[DBDC_BAND_NUM];
	INT32	CckFixedRssiBond[DBDC_BAND_NUM];
	INT32	OfdmFixedRssiBond[DBDC_BAND_NUM];
} SMART_CARRIER_SENSE_CTRL, *PSMART_CARRIER_SENSE_CTRL;

VOID SmartCarrierSense_Gen2(RTMP_ADAPTER *pAd);
VOID SmartCarrierSense_Gen3(RTMP_ADAPTER *pAd);
VOID SetSCS(RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value);
VOID SCS_init(RTMP_ADAPTER *pAd);

#endif /* __SCS_H__ */

