/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_phy.h

	Abstract:
	Ralink Wireless Chip PHY(BBP/RF) related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_PHY_H__
#define __RTMP_PHY_H__


#include "mac_ral/rf_ctrl.h"
#ifdef RLT_BBP
#include "phy/rlt_bbp.h"
#endif /* RLT_BBP */

#ifdef RTMP_BBP
#include "phy/rtmp_bbp.h"
#endif /* RTMP_BBP */


/*
	RF sections
*/
#define RF_R00			0
#define RF_R01			1
#define RF_R02			2
#define RF_R03			3
#define RF_R04			4
#define RF_R05			5
#define RF_R06			6
#define RF_R07			7
#define RF_R08			8
#define RF_R09			9
#define RF_R10			10
#define RF_R11			11
#define RF_R12			12
#define RF_R13			13
#define RF_R14			14
#define RF_R15			15
#define RF_R16			16
#define RF_R17			17
#define RF_R18			18
#define RF_R19			19
#define RF_R20			20
#define RF_R21			21
#define RF_R22			22
#define RF_R23			23
#define RF_R24			24
#define RF_R25			25
#define RF_R26			26
#define RF_R27			27
#define RF_R28			28
#define RF_R29			29
#define RF_R30			30
#define RF_R31			31
#define RF_R32			32
#define RF_R33			33
#define RF_R34			34
#define RF_R35			35
#define RF_R36			36
#define RF_R37			37
#define RF_R38			38
#define RF_R39			39
#define RF_R40			40
#define RF_R41			41
#define RF_R42			42
#define RF_R43			43
#define RF_R44			44
#define RF_R45			45
#define RF_R46			46
#define RF_R47			47
#define RF_R48			48
#define RF_R49			49
#define RF_R50			50
#define RF_R51			51
#define RF_R52			52
#define RF_R53			53
#define RF_R54			54
#define RF_R55			55
#define RF_R56			56
#define RF_R57			57
#define RF_R58			58
#define RF_R59			59
#define RF_R60			60
#define RF_R61			61
#define RF_R62			62
#define RF_R63			63

#define RF_R64			64
#define RF_R65			65
#define RF_R66			66
#define RF_R67			67
#define RF_R68			68
#define RF_R69			69
#define RF_R70			70
#define RF_R71			71
#define RF_R72			72
#define RF_R73			73
#define RF_R74			74
#define RF_R75			75
#define RF_R76			76
#define RF_R77			77
#define RF_R78			78
#define RF_R79			79
#define RF_R126			126
#define RF_R127			127

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
#define RFIC_6352			15	 	/* 2.4G 2T2R */
#define RFIC_7650					15		/* 2.4G/5G 1x1 VHT with BT*/
#define RFIC_7610E					16		/* 5G 1x1 VHT */
#define RFIC_7610U					17
#define RFIC_7630					18		/* 2.4G 1x1 HT with BT */
#define RFIC_7662					19		/* 2.4G/5G 2T2R VHT with BT */
#define RFIC_7612					20		/* 2.4G/5G 2T2R VHT */
#define RFIC_7602					21		/* 2.4G 2T2R VHT */
#define RFIC_UNKNOWN				0xff

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
	(__pAd->RfIcType == RFIC_UNKNOWN))


#ifdef RTMP_MAC_PCI
// TODO: shiang, what's the difference of BoardType v.s. band and interface??
#define BOARD_2G_ONLY           1
#define BOARD_5G_ONLY           2

#define BOARD_IS_2G_ONLY(__pAd)         \
        (__pAd->Antenna.field.BoardType == BOARD_2G_ONLY)

#define BOARD_IS_5G_ONLY(__pAd)         \
        (__pAd->Antenna.field.BoardType == BOARD_5G_ONLY)
#else
#define BOARD_IS_2G_ONLY(__pAd)         0
#define BOARD_IS_5G_ONLY(__pAd)         0
#endif /* !RTMP_MAC_PCI */

typedef enum{
	RX_CHAIN_0 = 1<<0,
	RX_CHAIN_1 = 1<<1,
	RX_CHAIN_2 = 1<<2,
	RX_CHAIN_ALL = 0xf
}RX_CHAIN_IDX;


/* */
/* BBP R49 TSSI (Transmit Signal Strength Indicator) */
/* */
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R49_STRUC {
	struct
	{
		UCHAR	adc5_in_sel:1; /* 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390) */
		UCHAR	bypassTSSIAverage:1; /* 0: the average TSSI (the average of the 16 samples), 1: the current TSSI */
		UCHAR	Reserved:1; /* Reserved field */
		UCHAR	TSSI:5; /* TSSI value */
	} field;

	UCHAR		byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#else
typedef union _BBP_R49_STRUC {
	struct
	{
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
#define MAX_BUSY_COUNT  100         /* Number of retry before failing access BBP & RF indirect register */
#define MAX_BUSY_COUNT_US 2000      /* Number of retry before failing access BBP & RF indirect register */

/*#define PHY_TR_SWITCH_TIME          5  // usec */

#define RSSI_FOR_VERY_LOW_SENSIBILITY   -35
#define RSSI_FOR_LOW_SENSIBILITY		-58
#define RSSI_FOR_MID_LOW_SENSIBILITY	-65 /*-80*/
#define RSSI_FOR_MID_SENSIBILITY		-90

/*****************************************************************************
	RF register Read/Write marco definition
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
#define RTMP_RF_IO_WRITE32(_A, _V)                  \
{                                                   					\
	if ((_A)->bPCIclkOff == FALSE) 	                \
	{												\
		RF_CSR_CFG0_STRUC  _value;                          \
		ULONG           _busyCnt = 0;                    \
											\
		do {                                            \
			RTMP_IO_READ32((_A), RF_CSR_CFG0, &_value.word);  \
			if (_value.field.Busy == IDLE)               \
				break;                                  \
			_busyCnt++;                                  \
		}while (_busyCnt < MAX_BUSY_COUNT);			\
		if(_busyCnt < MAX_BUSY_COUNT)                   \
		{                                               \
			RTMP_IO_WRITE32((_A), RF_CSR_CFG0, (_V));          \
    		}                                               \
    	}								\
    if ((_A)->ShowRf)					\
    {									\
    	printk("RF:%x\n", _V);			\
	}									\
}
#endif /* RTMP_MAC_PCI */



/*****************************************************************************
	BBP register Read/Write marco definitions.
	we read/write the bbp value by register's ID. 
	Generate PER to test BA
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
/* 
	Hardware Periodic Timer interrupt setting.
	Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
*/
#define RTMP_HW_TIMER_INT_SET(_pAd, _V)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(_pAd, INT_TIMER_CFG, &temp);		\
		temp &= 0x0000ffff;								\
		temp |= _V << 20; 								\
		RTMP_IO_WRITE32(_pAd, INT_TIMER_CFG, temp);		\
	}

/* Enable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_ENABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp |=0x2;										\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}

/* Disable Hardware Periodic Timer interrupt */
#define RTMP_HW_TIMER_INT_DISABLE(_pAd)					\
	{													\
		UINT32 temp;									\
		RTMP_IO_READ32(pAd, INT_TIMER_EN, &temp);		\
		temp &=~(0x2);									\
		RTMP_IO_WRITE32(pAd, INT_TIMER_EN, temp);		\
	}
#endif /* RTMP_MAC_PCI */


struct _RMTP_ADAPTER;


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

INT bbp_set_bw(struct _RTMP_ADAPTER *pAd, UINT8 bw);
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

NDIS_STATUS NICInitBBP(struct _RTMP_ADAPTER *pAd);
VOID InitRFRegisters(struct _RTMP_ADAPTER *pAd);

typedef struct phy_ops{
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
}PHY_OPS;


#endif /* __RTMP_PHY_H__ */

