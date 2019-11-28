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
	phy.h
*/

#ifndef __PHY_H__
#define __PHY_H__


#ifdef MT_BBP
#include "phy/mt_bbp.h"
#endif /* MT_BBP */

#ifdef MT_RF
#include "phy/mt_rf.h"
#endif

/* value domain of pAd->RfIcType */
#define RFIC_2820                   1       /* 2.4G 2T3R */
#define RFIC_2850                   2       /* 2.4G/5G 2T3R */
#define RFIC_2720                   3       /* 2.4G 1T2R */
#define RFIC_2750                   4       /* 2.4G/5G 1T2R */
#define RFIC_3020                   5       /* 2.4G 1T1R */
#define RFIC_2020                   6       /* 2.4G B/G */
#define RFIC_3021                   7       /* 2.4G 1T2R */
#define RFIC_3022                   8       /* 2.4G 2T2R */
#define RFIC_3052                   9       /* 2.4G/5G 2T2R */
#define RFIC_2853					10		/* 2.4G.5G 3T3R */
#define RFIC_3320                   11      /* 2.4G 1T1R with PA (RT3350/RT3370/RT3390) */
#define RFIC_3322                   12      /* 2.4G 2T2R with PA (RT3352/RT3371/RT3372/RT3391/RT3392) */
#define RFIC_3053                   13      /* 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662) */
#define RFIC_3853                   13      /* 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662) */
#define RFIC_5592			14	 /* 2.4G/5G */
#define RFIC_6352		15	/* 2.4G 2T2R */
#define RFIC_7650					15		/* 2.4G/5G 1x1 VHT with BT*/
#define RFIC_7610E					16		/* 5G 1x1 VHT */
#define RFIC_7610U					17
#define RFIC_7630					18		/* 2.4G 1x1 HT with BT */
#define RFIC_7662					19		/* 2.4G/5G 2T2R VHT with BT */
#define RFIC_7612					20		/* 2.4G/5G 2T2R VHT */
#define RFIC_7602					21		/* 2.4G 2T2R VHT */
#define RFIC_7603					22		/* 2.4G 2T2R HT */
#define RFIC_7628					23		/* 2.4G 2T2R HT */
#define RFIC_7636					24		/* ?? */
#define RFIC_7615					25
#define RFIC_7637					26
#define RFIC_7622					27              /* ?? */ /*wilsonl */
#define RFIC_P18					28
#define RFIC_7663					29
#define RFIC_7611					30
#define RFIC_7616					31
#define RFIC_7615A					32
#define RFIC_UNKNOWN				0xff

/* TODO: shiang-MT7615 */
#define RFIC_IS_5G_BAND(__pAd)			\
	((__pAd->RfIcType == RFIC_2850) ||	\
	(__pAd->RfIcType == RFIC_2750) ||	\
	(__pAd->RfIcType == RFIC_3052) ||	\
	(__pAd->RfIcType == RFIC_2853) ||	\
	(__pAd->RfIcType == RFIC_3053) ||	\
	(__pAd->RfIcType == RFIC_3853) ||	\
	(__pAd->RfIcType == RFIC_5592) ||	\
	(__pAd->RfIcType == RFIC_7650) ||	\
	(__pAd->RfIcType == RFIC_7610E) ||	\
	(__pAd->RfIcType == RFIC_7610U) ||	\
	(__pAd->RfIcType == RFIC_7662) ||	\
	(__pAd->RfIcType == RFIC_7612) ||	\
	(__pAd->RfIcType == RFIC_7636) ||	\
	(__pAd->RfIcType == RFIC_7615) ||	\
	(__pAd->RfIcType == RFIC_7636) ||	\
	(__pAd->RfIcType == RFIC_7637) ||	\
	(__pAd->RfIcType == RFIC_P18) ||	\
	(__pAd->RfIcType == RFIC_7663) ||	\
	(__pAd->RfIcType == RFIC_UNKNOWN))


#define BOARD_IS_2G_ONLY(__pAd)         0
#define BOARD_IS_5G_ONLY(__pAd)         ((__pAd->RfIcType == RFIC_7611) || (__pAd->RfIcType == RFIC_7615A))
#define BOARD_IS_NO_256QAM(__pAd)	(__pAd->RfIcType == RFIC_7616)

typedef enum{
	RX_CHAIN_0 = 1<<0,
	RX_CHAIN_1 = 1<<1,
	RX_CHAIN_2 = 1<<2,
	RX_CHAIN_ALL = 0xf
} RX_CHAIN_IDX;


/* */
/* BBP R49 TSSI (Transmit Signal Strength Indicator) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R49_STRUC {
	struct {
		UCHAR	adc5_in_sel:1; /* 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390) */
		UCHAR	bypassTSSIAverage:1; /* 0: the average TSSI (the average of the 16 samples), 1: the current TSSI */
		UCHAR	Reserved:1; /* Reserved field */
		UCHAR	TSSI:5; /* TSSI value */
	} field;

	UCHAR		byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#else
typedef union _BBP_R49_STRUC {
	struct {
		UCHAR	TSSI:5; /* TSSI value */
		UCHAR	Reserved:1; /* Reserved field */
		UCHAR	bypassTSSIAverage:1; /* 0: the average TSSI (the average of the 16 samples), 1: the current TSSI */
		UCHAR	adc5_in_sel:1; /* 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390) */
	} field;

	UCHAR		byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#endif

#define MAX_BBP_MSG_SIZE	4096


/* */
/* BBP & RF are using indirect access. Before write any value into it. */
/* We have to make sure there is no outstanding command pending via checking busy bit. */
/* */
#ifdef COMPOS_WIN
#define MAX_BUSY_COUNT  20         /* Number of retry before failing access BBP & RF indirect register */
#else
#define MAX_BUSY_COUNT  100         /* Number of retry before failing access BBP & RF indirect register */
#define MAX_BUSY_COUNT_US 2000      /* Number of retry before failing access BBP & RF indirect register */
#endif

/*#define PHY_TR_SWITCH_TIME          5  // usec */

#define RSSI_FOR_VERY_LOW_SENSIBILITY   -35
#define RSSI_FOR_LOW_SENSIBILITY		-58
#define RSSI_FOR_MID_LOW_SENSIBILITY	-65 /*-80*/
#define RSSI_FOR_MID_SENSIBILITY		-90

struct _RTMP_ADAPTER;



struct rx_signal_info {
	CHAR raw_rssi[4];
	UCHAR raw_snr[4];
	CHAR freq_offset;
};


typedef struct _RSSI_SAMPLE {
	CHAR LastRssi[4]; /* last received RSSI for ant 0~2 */
	CHAR AvgRssi[4];
	SHORT AvgRssiX8[4];
	CHAR LastSnr[4];
	CHAR AvgSnr[4];
	SHORT AvgSnrX8[4];
	/*CHAR LastNoiseLevel[3]; */
} RSSI_SAMPLE;

typedef enum ChannelSelAlg {
	ChannelAlgRandom, /*use by Dfs */
	ChannelAlgApCnt,
	ChannelAlgCCA,
	ChannelAlgBusyTime
} ChannelSel_Alg;

/*****************************************************************************
	RF register Read/Write marco definition
 *****************************************************************************/


/*****************************************************************************
	BBP register Read/Write marco definitions.
	we read/write the bbp value by register's ID.
	Generate PER to test BA
 *****************************************************************************/


#define _G_BAND 0x0
#define _A_BAND 0x1
#define _BAND 0x3

#define _BW_20 (0x0 << 2)
#define _BW_40 (0x1 << 2)
#define _BW_80 (0x2 << 2)
#define _BW_160 (0x3 << 2)
#define _BW (0x3 << 2)

#define RF_Path0 0
#define RF_Path1 1

#define _1T1R (0x0 << 4)
#define _2T1R (0x1 << 4)
#define _1T2R (0x2 << 4)
#define _2T2R (0x3 << 4)
#define _TX_RX_SETTING (0x3 << 4)

#define _LT (0x0 << 6)
#define _HT (0x1 << 6)
#define _TEM_SETTING (0x1 << 6)

INT bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw, UCHAR BandIdx);
INT bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, UINT8 ext_ch);
INT bbp_set_rxpath(struct _RTMP_ADAPTER *pAd, INT rxpath);
INT bbp_get_temp(struct _RTMP_ADAPTER *pAd, CHAR *temp_val);
INT bbp_tx_comp_init(struct _RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode);
INT bbp_set_txdac(struct _RTMP_ADAPTER *pAd, INT tx_dac);
INT bbp_set_mmps(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower);
INT bbp_is_ready(struct _RTMP_ADAPTER *pAd);
INT bbp_set_agc(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX idx);
INT bbp_get_agc(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX idx);
INT filter_coefficient_ctrl(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
UCHAR get_random_seed_by_phy(struct _RTMP_ADAPTER *pAd);
#ifdef DYNAMIC_VGA_SUPPORT
INT dynamic_vga_enable(struct _RTMP_ADAPTER *pAd);
INT dynamic_vga_disable(struct _RTMP_ADAPTER *pAd);
INT dynamic_vga_adjust(struct _RTMP_ADAPTER *pAd);
#endif /* DYNAMIC_VGA_SUPPORT */
#ifdef SMART_CARRIER_SENSE_SUPPORT
INT Smart_Carrier_Sense(struct _RTMP_ADAPTER *pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
NDIS_STATUS NICInitBBP(struct _RTMP_ADAPTER *pAd);
VOID InitRFRegisters(struct _RTMP_ADAPTER *pAd);
INT32 ShowAllBBP(struct _RTMP_ADAPTER *pAd);
INT32 ShowPartialBBP(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
INT32 ShowAllRF(struct _RTMP_ADAPTER *pAd);
INT32 ShowPartialRF(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
#ifdef CONFIG_AP_SUPPORT
UCHAR AutoCh(struct _RTMP_ADAPTER *pAd, ChannelSel_Alg Alg, BOOLEAN IsABand);
#endif/*CONFIG_AP_SUPPORT*/

typedef struct phy_ops {
	INT (*bbp_is_ready)(struct _RTMP_ADAPTER *pAd);
	UCHAR (*get_random_seed_by_phy)(struct _RTMP_ADAPTER *pAd);
	INT (*filter_coefficient_ctrl)(struct _RTMP_ADAPTER *pAd, UCHAR Channel);
	INT (*bbp_set_agc)(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX chain);
	INT (*bbp_get_agc)(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX chain);
	INT (*bbp_set_mmps)(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower);
	INT (*bbp_set_bw)(struct _RTMP_ADAPTER *pAd, UINT8 bw);
	INT (*bbp_set_ctrlch)(struct _RTMP_ADAPTER *pAd, UINT8 ext_ch);
	INT (*bbp_set_rxpath)(struct _RTMP_ADAPTER *pAd, INT rxpath);
	INT (*bbp_set_txdac)(struct _RTMP_ADAPTER *pAd, INT tx_dac);
	INT (*bbp_tx_comp_init)(struct _RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode);
	INT (*bbp_get_temp)(struct _RTMP_ADAPTER *pAd, CHAR *temp_val);
	INT (*bbp_init)(struct _RTMP_ADAPTER *pAd);
#ifdef DYNAMIC_VGA_SUPPORT
	INT (*dynamic_vga_enable)(struct _RTMP_ADAPTER *pAd);
	INT (*dynamic_vga_disable)(struct _RTMP_ADAPTER *pAd);
	INT (*dynamic_vga_adjust)(struct _RTMP_ADAPTER *pAd);
#endif /* DYNAMIC_VGA_SUPPORT */
	INT32 (*ShowPartialBBP)(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
	INT32 (*ShowAllBBP)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ShowPartialRF)(struct _RTMP_ADAPTER *pAd, UINT32 Start, UINT32 End);
	INT32 (*ShowAllRF)(struct _RTMP_ADAPTER *pAd);
#ifdef CONFIG_AP_SUPPORT
	UCHAR (*AutoCh)(struct _RTMP_ADAPTER *pAd, struct wifi_dev *pwdev, ChannelSel_Alg Alg, BOOLEAN IsABand);
#endif/*CONFIG_AP_SUPPORT*/
#ifdef SMART_CARRIER_SENSE_SUPPORT
	INT (*Smart_Carrier_Sense)(struct _RTMP_ADAPTER *pAd);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
} PHY_OPS;


#endif /* __PHY_H__ */

