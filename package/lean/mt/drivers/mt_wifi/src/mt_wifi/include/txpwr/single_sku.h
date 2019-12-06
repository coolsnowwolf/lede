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
	single_sku.h
*/

#ifndef __CMM_SINGLE_SKU_H__
#define __CMM_SINGLE_SKU_H__


/* TODO: shiang-usw, integrate these data structures to a single one! */

/* Debug log color */
#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define G_BAND        0
#define A_BAND        1

#define	SINGLE_SKU_TABLE_LENGTH		(SINGLE_SKU_TABLE_CCK_LENGTH+SINGLE_SKU_TABLE_OFDM_LENGTH+(SINGLE_SKU_TABLE_HT_LENGTH*2)+SINGLE_SKU_TABLE_VHT_LENGTH)

#define SINGLE_SKU_TABLE_EFFUSE_ADDRESS 0x12C

#if defined(MT7615) || defined(MT7622)
#define SINGLE_SKU_TABLE_CCK_LENGTH         2
#define SINGLE_SKU_TABLE_OFDM_LENGTH        5
#define SINGLE_SKU_TABLE_HT_LENGTH          16
#define SINGLE_SKU_TABLE_VHT_LENGTH         7     /* VHT80 MCS 0 ~ 9 */
#else
#define SINGLE_SKU_TABLE_CCK_LENGTH         4
#define SINGLE_SKU_TABLE_OFDM_LENGTH        8
#define SINGLE_SKU_TABLE_HT20_LENGTH        8
#define SINGLE_SKU_TABLE_HT40_LENGTH        8
#define SINGLE_SKU_TABLE_VHT20_LENGTH       10
#define SINGLE_SKU_TABLE_VHT40_LENGTH       10
#define SINGLE_SKU_TABLE_VHT80_LENGTH       10
#define SINGLE_SKU_TABLE_VHT160_LENGTH      10

#define SINGLE_SKU_TYPE_PARSE_NUM           6     /* CCK(1) + OFDM(1) + VHT20/40/80/160(4) */
#define SINGLE_SKU_PARAM_PARSE_NUM          52    /* CCK(4) + OFDM(8) + VHT20/40/80/160(10+10+10+10) */
#define SINGLE_SKU_TYPE_NUM                 8     /* CCK(1) + OFDM(1) + HT20/40(2) + VHT20/40/80/160(4) */
#define SINGLE_SKU_PARAM_NUM                68    /* CCK(4) + OFDM(8) + HT20/40(8+8) + VHT20/40/80/160(10+10+10+10) */

#define BACKOFF_TABLE_BF_OFF_CCK_LENGTH     4
#define BACKOFF_TABLE_BF_OFF_OFDM_LENGTH    4
#define BACKOFF_TABLE_BF_ON_OFDM_LENGTH     3
#define BACKOFF_TABLE_BF_OFF_HT20_LENGTH    10
#define BACKOFF_TABLE_BF_ON_HT20_LENGTH     10
#define BACKOFF_TABLE_BF_OFF_HT40_LENGTH    10
#define BACKOFF_TABLE_BF_ON_HT40_LENGTH     10
#define BACKOFF_TABLE_BF_OFF_VHT20_LENGTH   10
#define BACKOFF_TABLE_BF_ON_VHT20_LENGTH    10
#define BACKOFF_TABLE_BF_OFF_VHT40_LENGTH   10
#define BACKOFF_TABLE_BF_ON_VHT40_LENGTH    10
#define BACKOFF_TABLE_BF_OFF_VHT80_LENGTH   10
#define BACKOFF_TABLE_BF_ON_VHT80_LENGTH    10
#define BACKOFF_TABLE_BF_OFF_VHT160_LENGTH  10
#define BACKOFF_TABLE_BF_ON_VHT160_LENGTH   10

#define BACKOFF_TYPE_PARSE_NUM              11   /* CCK(0+1) + OFDM(1+1) + VHT20/40/80/160(1+1, 1+1, 1+1, 1+1) */
#define BACKOFF_PARAM_PARSE_NUM             91   /* CCK(0+4) + OFDM(3+4) VHT20/40/80/160(10+10, 10+10, 10+10, 10+10) */
#define BACKOFF_TYPE_NUM                    15   /* CCK(0+1) + OFDM(1+1) + HT20/40(1+1, 1+1) + VHT20/40/80/160(1+1, 1+1, 1+1, 1+1) */
#define BACKOFF_PARAM_NUM                   131  /* CCK(0+4) + OFDM(3+4) + HT20/40(10+10, 10+10) + VHT20/40/80/160(10+10, 10+10, 10+10, 10+10) */

#define TABLE_PARSE_TYPE_NUM                2

/* 0: None, 1: G_BAND, 2: A_BAND, 3: G_BAND and A_BAND */
#define TABLE_NO_PARSE                      0
#define TABLE_PARSE_G_BAND                  BIT(0)
#define TABLE_PARSE_A_BAND                  BIT(1)
#define TABLE_PARSE_G_A_BAND                BITS(0, 1)
#endif /* defined(MT7615) || defined(MT7622) */

#define SINGLE_SKU_TABLE_TX_OFFSET_NUM  3
#define SINGLE_SKU_TABLE_NSS_OFFSET_NUM 4

#define SKUTABLE_1                      1
#define SKUTABLE_2                      2
#define SKUTABLE_3                      3
#define SKUTABLE_4                      4
#define SKUTABLE_5                      5
#define SKUTABLE_6                      6
#define SKUTABLE_7                      7
#define SKUTABLE_8                      8
#define SKUTABLE_9                      9
#define SKUTABLE_10                    10
#define SKUTABLE_11                    11
#define SKUTABLE_12                    12
#define SKUTABLE_13                    13
#define SKUTABLE_14                    14
#define SKUTABLE_15                    15
#define SKUTABLE_16                    16
#define SKUTABLE_17                    17
#define SKUTABLE_18                    18
#define SKUTABLE_19                    19
#define SKUTABLE_20                    20
#define TABLE_SIZE                     20

#define VHT20_OFFSET                    0
#define VHT40_OFFSET                    7
#define VHT80_OFFSET                   14
#define VHT160C_OFFSET                 21

/* PHY Mode */
#define SKU_CCK_OFFSET                  0
#define SKU_OFDM_OFFSET                 2
#define SKU_HT_OFFSET                   7
#define SKU_VHT_OFFSET                 21

/* MCS Rate */
#define SKU_CCK_RATE_M01                0
#define SKU_CCK_RATE_M23                1

#define SKU_OFDM_RATE_M01               0
#define SKU_OFDM_RATE_M23               1
#define SKU_OFDM_RATE_M45               2
#define SKU_OFDM_RATE_M6                3
#define SKU_OFDM_RATE_M7                4

#define SKU_HT_RATE_M0                  0
#define SKU_HT_RATE_M32                 1
#define SKU_HT_RATE_M12                 2
#define SKU_HT_RATE_M34                 3
#define SKU_HT_RATE_M5                  4
#define SKU_HT_RATE_M6                  5
#define SKU_HT_RATE_M7                  6

#define SKU_VHT_RATE_M0                 0
#define SKU_VHT_RATE_M12                1
#define SKU_VHT_RATE_M34                2
#define SKU_VHT_RATE_M56                3
#define SKU_VHT_RATE_M7                 4
#define SKU_VHT_RATE_M8                 5
#define SKU_VHT_RATE_M9                 6

#if defined(MT7615) || defined(MT7622)
#else
typedef enum _POWER_LIMIT_TABLE {
    POWER_LIMIT_TABLE_TYPE_SKU = 0,
    POWER_LIMIT_TABLE_TYPE_BACKOFF,
    POWER_LIMIT_TABLE_TYPE_NUM
} POWER_LIMIT_TABLE, *P_POWER_LIMIT_TABLE;

typedef enum _POWER_LIMIT_TABLE_TYPE_SKU_CATEGORY {
    SINGLE_SKU_TABLE_CCK = 0,
    SINGLE_SKU_TABLE_OFDM,
    SINGLE_SKU_TABLE_HT20,
    SINGLE_SKU_TABLE_HT40,
    SINGLE_SKU_TABLE_VHT20,
    SINGLE_SKU_TABLE_VHT40,
    SINGLE_SKU_TABLE_VHT80,
    SINGLE_SKU_TABLE_VHT160,
    SINGLE_SKU_TABLE_NUM
} POWER_LIMIT_TABLE_TYPE_SKU_CATEGORY, *P_POWER_LIMIT_TABLE_TYPE_SKU_CATEGORY;

typedef enum _POWER_LIMIT_TABLE_TYPE_BACKOFF_CATEGORY {
    BACKOFF_TABLE_BF_OFF_CCK = 0,
    BACKOFF_TABLE_BF_OFF_OFDM,
    BACKOFF_TABLE_BF_ON_OFDM,
    BACKOFF_TABLE_BF_OFF_HT20,
    BACKOFF_TABLE_BF_ON_HT20,
    BACKOFF_TABLE_BF_OFF_HT40,
    BACKOFF_TABLE_BF_ON_HT40,
    BACKOFF_TABLE_BF_OFF_VHT20,
    BACKOFF_TABLE_BF_ON_VHT20,
    BACKOFF_TABLE_BF_OFF_VHT40,
    BACKOFF_TABLE_BF_ON_VHT40,
    BACKOFF_TABLE_BF_OFF_VHT80,
    BACKOFF_TABLE_BF_ON_VHT80,
    BACKOFF_TABLE_BF_OFF_VHT160,
    BACKOFF_TABLE_BF_ON_VHT160,
    BACKOFF_TABLE_NUM
} POWER_LIMIT_TABLE_TYPE_BACKOFF_CATEGORY, *P_POWER_LIMIT_TABLE_TYPE_BACKOFF_CATEGORY;
#endif /* defined(MT7615) || defined(MT7622) */

/* TODO: shiang-usw, need to re-organize these for MT7610/MT7601/MT7620!! */
#if defined(MT7615) || defined(MT7622)
typedef struct _CH_POWER_ {
    DL_LIST      List;
    UCHAR        StartChannel;
    UCHAR        num;
    UCHAR        *Channel;
    UCHAR        band;
    UCHAR        u1PwrLimitCCK[SINGLE_SKU_TABLE_CCK_LENGTH];
    UCHAR        u1PwrLimitOFDM[SINGLE_SKU_TABLE_OFDM_LENGTH];
    UCHAR        u1PwrLimitHT20[SINGLE_SKU_TABLE_HT_LENGTH];
    UCHAR        u1PwrLimitHT40[SINGLE_SKU_TABLE_HT_LENGTH];
    UCHAR        u1PwrLimitVHT20[SINGLE_SKU_TABLE_VHT_LENGTH];
    UCHAR        u1PwrLimitVHT40[SINGLE_SKU_TABLE_VHT_LENGTH];
    UCHAR        u1PwrLimitVHT80[SINGLE_SKU_TABLE_VHT_LENGTH];
    UCHAR        u1PwrLimitVHT160[SINGLE_SKU_TABLE_VHT_LENGTH];
    UCHAR        u1PwrLimitTxStreamDelta[SINGLE_SKU_TABLE_TX_OFFSET_NUM];
    UCHAR        u1PwrLimitTxNSSDelta[SINGLE_SKU_TABLE_TX_OFFSET_NUM];
} CH_POWER, *P_CH_POWER;

typedef struct _BACKOFF_POWER_ {
    DL_LIST      List;
    UCHAR        StartChannel;
    UCHAR        num;
    UCHAR        *Channel;
    UCHAR        band;
    UCHAR        PwrMax[3];
} BACKOFF_POWER, *P_BACKOFF_POWER;
#else
typedef struct _CH_POWER_ {
    DL_LIST      List;
    UINT8        u1StartChannel;
    UINT8        u1ChNum;
    PUINT8       pu1ChList;
    UINT8        u1ChBand;
    PUINT8       pu1PwrLimit;
} CH_POWER, *P_CH_POWER;
#endif /* defined(MT7615) || defined(MT7622) */

#ifdef SINGLE_SKU_V2
#if defined(MT7615) || defined(MT7622)
INT MtSingleSkuLoadParam(
    RTMP_ADAPTER *pAd
    );

VOID
MtSingleSkuUnloadParam(
    RTMP_ADAPTER *pAd
    );

INT
MtBfBackOffLoadParam(
    RTMP_ADAPTER *pAd
    );

VOID
MtBfBackOffUnloadParam(
    RTMP_ADAPTER *pAd
    );

VOID
MtFillSkuParam(
    RTMP_ADAPTER *pAd, UINT8 channel, UCHAR Band, UCHAR TxStream, UINT8 *txPowerSku
    );

VOID
MtFillBackoffParam(
    RTMP_ADAPTER *pAd, UINT8 channel, UCHAR Band, UINT8 *BFPowerBackOff
    );

VOID
MtShowSkuTable(
    RTMP_ADAPTER * pAd,
    UINT8 u1DebugLevel
    );

#else
INT
MtPwrLimitLoadParamHandle(
    RTMP_ADAPTER *pAd, UINT8 u1Type
    );

VOID
MtPwrLimitUnloadParamHandle(
    RTMP_ADAPTER *pAd, UINT8 u1Type
    );

VOID
MtParsePwrLimitTable(
    RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type
    );

BOOLEAN
MtReadPwrLimitTable(
    RTMP_ADAPTER *pAd, PCHAR pi1Buffer, UINT8 u1Type
    );

VOID
MtPwrFillLimitParam(
    RTMP_ADAPTER *pAd, UINT8 u1Channel, UINT8 ChBand, PINT8 pi1PwrLimitParam, UINT8 u1Type
    );

VOID
MtPwrLimitParse(
    PUINT pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type
    );

BOOLEAN
MtPwrLimitSimilarCheck(
    PUINT pi1PwrLimitStartCh, PUINT pi1PwrLimitNewCh, UINT8 u1ChBand, UINT8 u1Type
    );

VOID
MtShowPwrLimitTable(
    RTMP_ADAPTER *pAd, UINT8 u1Type, UINT8 u1DebugLevel
    );
#endif /* defined(MT7615) || defined(MT7622) */

CHAR
SKUTxPwrOffsetGet(
    RTMP_ADAPTER *pAd, UINT8 ucBandIdx, UINT8 ucBW, UINT8 ucPhymode, UINT8 ucMCS, UINT8 ucNss, BOOLEAN fgSE
    );

#endif /* SINGLE_SKU_V2 */
#endif /*__CMM_SINGLE_SKU_H__*/
