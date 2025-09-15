/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
 	Module Name:
	rt_ate.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __RT_ATE_H__
#define __RT_ATE_H__

#ifdef CONFIG_ATE
#ifndef STATS_COUNT_SUPPORT
#error "For ATE support, please set HAS_ATE=y and HAS_STATS_COUNT=y."
#endif /* !STATS_COUNT_SUPPORT */
#endif /* CONFIG_ATE */

#if defined(RTMP_MAC) || defined(RLT_MAC)
#include "mac/mac_ral/rtmp_mac.h"
#elif defined(MT_MAC)
#include "mac/mac_mt/mt_mac.h"
#endif
#include "phy/phy.h"

// TODO: shiang-6590, remove it after ATE code is re-organized!!
#define BBP_R1			1
#define BBP_R3			3
#define BBP_R4			4
#define BBP_R21			21
#define BBP_R22			22
#define BBP_R24			24
#define BBP_R27			27
#define BBP_R50			50
#define BBP_R51			51
#define BBP_R52			52
#define BBP_R60			60
#define BBP_R66			66
#define BBP_R69			69
#define BBP_R70			70
#define BBP_R174			174
#define BBP_R182			182
// TODO: ---End

typedef struct _ATE_CHIP_STRUCT {
	/* functions */
	VOID	(*ChannelSwitch)(PRTMP_ADAPTER pAd);
	INT		(*TxPwrHandler)(PRTMP_ADAPTER pAd, char index);
	INT		(*TxPwrEvaluation)(PRTMP_ADAPTER pAd);
	INT		(*TssiCalibration)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT		(*ExtendedTssiCalibration)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	VOID	(*RxVGAInit)(PRTMP_ADAPTER pAd);
	VOID	(*AsicSetTxRxPath)(PRTMP_ADAPTER pAd);
	VOID	(*AdjustTxPower)(PRTMP_ADAPTER pAd);
	VOID	(*AsicExtraPowerOverMAC)(PRTMP_ADAPTER pAd);
	VOID	(*AsicCalibration)(PRTMP_ADAPTER pAd, UCHAR ate_mode);
	VOID 	(*TemperCompensation)(RTMP_ADAPTER *pAd);
	
	/* command handlers */
	INT		(*Set_BW_Proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT		(*Set_FREQ_OFFSET_Proc)(RTMP_ADAPTER *pAd, RTMP_STRING *arg);	

	/* variables */
	INT maxTxPwrCnt;
	BOOLEAN bBBPStoreTXCARR;
	BOOLEAN bBBPStoreTXCARRSUPP;
	BOOLEAN bBBPStoreTXCONT;
	BOOLEAN bBBPLoadATESTOP;
		
	/* TSSI related */
	INT32 tssi_slope[2];
	INT32 tssi_offset[3][2];
	INT32 pwr_diff_pre[2];
	INT32 ant_pwr_offset[2];
	INT32 curr_temperature;
}ATE_CHIP_STRUCT, *PATE_CHIP_STRUCT;

typedef union _CAPTURE_MODE_SHARE_MEMORY {
	struct 
	{
		UINT32       LOW_BYTE0:8;
		UINT32       LOW_BYTE1:8;
		UINT32       HIGH_BYTE0:8;
		UINT32       HIGH_BYTE1:8;
	} field;
	UINT32                   Value;
}CAPTURE_MODE_SHARE_MEMORY, *PCAPTURE_MODE_SHARE_MEMORY;

typedef struct _ATE_INFO {
	PATE_CHIP_STRUCT pChipStruct; 
#ifdef RTMP_MAC_PCI
	PVOID AteAllocVa[TX_RING_SIZE];
	PNDIS_PACKET pAtePacket[TX_RING_SIZE];
#endif /* RTMP_MAC_PCI */
	UCHAR Mode;
	BOOLEAN PassiveMode;
#ifdef RT3350
	UCHAR   PABias;
#endif /* RT3350 */
	CHAR TxPower0;
	CHAR TxPower1;
#ifdef DOT11N_SS3_SUPPORT
	CHAR TxPower2;
#endif /* DOT11N_SS3_SUPPORT */
	CHAR MinTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR MaxTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR TxAntennaSel;
	CHAR RxAntennaSel;
	USHORT TxInfoLen;
	USHORT TxWILen;
	TXINFO_STRUC TxInfo;	/* TxInfo */
	TXWI_STRUC TxWI;	/* TXWI */
	USHORT QID;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR Channel;
	UCHAR Payload;		/* Payload pattern */
	BOOLEAN bFixedPayload;
	UCHAR TxMethod; /* Early chipsets must be applied old TXCONT/TXCARR/TXCARS mechanism. */
	UINT32 TxLength;
	UINT32 TxCount;
	UINT32 TxDoneCount;	/* Tx DMA Done */
	UINT32 RFFreqOffset;
	UINT32 IPG;
	BOOLEAN bRxFER;		/* Show Rx Frame Error Rate */
	BOOLEAN	bQAEnabled;	/* QA is used. */
	BOOLEAN bQATxStart;	/* Have compiled QA in and use it to ATE tx. */
	BOOLEAN bQARxStart;	/* Have compiled QA in and use it to ATE rx. */
	BOOLEAN bAutoTxAlc;	/* Set Auto Tx Alc */
	BOOLEAN bAutoVcoCal; /* Set Auto VCO periodic calibration. */
	BOOLEAN bLowTemperature; /* Trigger Temperature Sensor */
	BOOLEAN bTestTimeReduction; /* Set ATE test time reduction. */
#ifdef RTMP_INTERNAL_TX_ALC
#if defined(RT3350) || defined(RT3352)
	BOOLEAN bTSSICalbrEnableG; /* Enable TSSI calibration */
	CHAR	TssiRefPerChannel[CFG80211_NUM_OF_CHAN_2GHZ];
	CHAR	TssiDeltaPerChannel[CFG80211_NUM_OF_CHAN_2GHZ];
#endif /* defined(RT3350) || defined(RT3352) */
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_MAC_PCI
	BOOLEAN bFWLoading;	/* Reload firmware when ATE is done. */
#endif				/* RTMP_MAC_PCI */
	UINT32 RxTotalCnt;
	UINT32 RxCntPerSec;
	UCHAR	forceBBPReg;	/* force to not update the specific BBP register, now used for ATE TxBF */

	CHAR LastSNR0;		/* last received SNR */
	CHAR LastSNR1;		/* last received SNR for 2nd  antenna */
#ifdef DOT11N_SS3_SUPPORT
	CHAR LastSNR2;
#endif				/* DOT11N_SS3_SUPPORT */
	CHAR LastRssi0;		/* last received RSSI */
	CHAR LastRssi1;		/* last received RSSI for 2nd  antenna */
	CHAR LastRssi2;		/* last received RSSI for 3rd  antenna */
	CHAR AvgRssi0;		/* last 8 frames' average RSSI */
	CHAR AvgRssi1;		/* last 8 frames' average RSSI */
	CHAR AvgRssi2;		/* last 8 frames' average RSSI */
	SHORT AvgRssi0X8;	/* sum of last 8 frames' RSSI */
	SHORT AvgRssi1X8;	/* sum of last 8 frames' RSSI */
	SHORT AvgRssi2X8;	/* sum of last 8 frames' RSSI */
	UINT32 NumOfAvgRssiSample;
	UINT32 Default_TX_PIN_CFG;
	USHORT HLen;		/* Header Length */

#ifdef CONFIG_QA
	/* Tx frame */
	USHORT PLen;		/* Pattern Length */
	UCHAR Header[32];	/* Header buffer */
	UCHAR Pattern[32];	/* Pattern buffer */
	USHORT DLen;		/* Data Length */
	USHORT seq;
	UINT32 CID;
	RTMP_OS_PID AtePid;
	/* counters */
	UINT32 U2M;
	UINT32 OtherData;
	UINT32 Beacon;
	UINT32 OtherCount;
	UINT32 TxAc0;
	UINT32 TxAc1;
	UINT32 TxAc2;
	UINT32 TxAc3;
	UINT32 TxHCCA;
	UINT32 TxMgmt;
	UINT32 RSSI0;
	UINT32 RSSI1;
	UINT32 RSSI2;
	UINT32 SNR0;
	UINT32 SNR1;
#ifdef DOT11N_SS3_SUPPORT
	UINT32 SNR2;
#endif /* DOT11N_SS3_SUPPORT */
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UCHAR TxStatus;
#ifdef DOT11_VHT_AC
	UCHAR vht_nss;
#endif /* DOT11_VHT_AC */
#endif /* CONFIG_QA */
	RALINK_TIMER_STRUCT PeriodicTimer;
	ULONG OneSecPeriodicRound;
	ULONG PeriodicRound;
	OS_NDIS_SPIN_LOCK TssiSemLock;
} ATE_INFO, *PATE_INFO;

/* 
	Use bitmap to allow coexist of ATE_TXFRAME 
	and ATE_RXFRAME(i.e.,to support LoopBack mode).
*/
#define fATE_IDLE					0x00
#define fATE_TX_ENABLE				0x01
#define fATE_RX_ENABLE				0x02
#define fATE_TXCONT_ENABLE			0x04
#define fATE_TXCARR_ENABLE			0x08
#define fATE_TXCARRSUPP_ENABLE		0x10
#define fATE_RESERVED_1				0x20
#define fATE_RESERVED_2				0x40
#define fATE_EXIT					0x80

/* Enter/Reset ATE */
#define	ATE_START                   (fATE_IDLE)
/* Stop/Exit ATE */
#define	ATE_STOP                    (fATE_EXIT)
/* Continuous Transmit Frames (without time gap) */
#define	ATE_TXCONT                  ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE))
/* Transmit Carrier */
#define	ATE_TXCARR                  ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE))
/* Transmit Carrier Suppression (information without carrier) */
#define	ATE_TXCARRSUPP              ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE))
/* Transmit Frames */
#define	ATE_TXFRAME                 (fATE_TX_ENABLE)
/* Receive Frames */
#define	ATE_RXFRAME                 (fATE_RX_ENABLE)


#ifdef RTMP_INTERNAL_TX_ALC
#define EEPROM_TSSI_ENABLE 0x36
#define EEPROM_TSSI_MODE_EXTEND 0x76

#define ATE_MDSM_NORMAL_TX_POWER						0x00
#define ATE_MDSM_DROP_TX_POWER_BY_6dBm					0x01
#define ATE_MDSM_DROP_TX_POWER_BY_12dBm					0x02
#define ATE_MDSM_ADD_TX_POWER_BY_6dBm					0x03
#define ATE_MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK	0x03
#endif /* RTMP_INTERNAL_TX_ALC */

#define	LEN_OF_ARG 16
#define ATE_ON(_p)              (((_p)->ate.Mode) != ATE_STOP)
#define TX_METHOD_0 0 /* Early chipsets must be applied this original TXCONT/TXCARR/TXCARS mechanism. */
#define TX_METHOD_1 1 /* Default TXCONT/TXCARR/TXCARS mechanism is TX_METHOD_1 */
#define ANT_ALL 0
#define ANT_0 1
#define ANT_1 2
#ifdef DOT11N_SS3_SUPPORT
#define ANT_2 3
#endif /* DOT11N_SS3_SUPPORT */

#define ATE_ASIC_CALIBRATION(__pAd, __ate_mode)	\
do {	\
		if (__pAd->ate.pChipStruct->AsicCalibration != NULL)	\
			__pAd->ate.pChipStruct->AsicCalibration(__pAd, __ate_mode);	\
} while (0)


#define ATE_MAC_TX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_TX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
#define ATE_MAC_TX_CTS_ENABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) |= (1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 0. */
#define ATE_MAC_TX_CTS_DISABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) &= ~(1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear BBP R22 to reset Tx Mode (bit7~bit0) = 0. */
#ifdef RTMP_BBP
#define ATE_BBP_RESET_TX_MODE(_A, _I, _pV)			\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= (0x00);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}
#endif /* RTMP_BBP */

/* Set BBP R22 to start Continuous Tx Mode (bit7) = 1. */
#define ATE_BBP_START_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R22 to stop Continuous Tx Mode (bit7) = 0. */
#define ATE_BBP_STOP_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Set BBP R24 to send out Continuous Tx sin wave (bit0) = 1. */
#define ATE_BBP_CTS_TX_SIN_WAVE_ENABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R24 to stop Continuous Tx sin wave (bit0) = 0. */
#define ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/*
==========================================================================
	Description:
		This routine sets initial value of VGA in the RX chain.
		AGC is the abbreviation of "Automatic Gain Controller",
		while VGA (Variable Gain Amplifier) is a part of AGC loop.
		(i.e., VGA + level detector + feedback loop = AGC)

    Return:
        VOID
==========================================================================
*/
#define ATE_CHIP_RX_VGA_GAIN_INIT(__pAd)								\
	if (__pAd->ate.pChipStruct->RxVGAInit != NULL)	\
		__pAd->ate.pChipStruct->RxVGAInit(__pAd)

#define ATE_CHIP_SET_TX_RX_PATH(__pAd)								\
	if (__pAd->ate.pChipStruct->AsicSetTxRxPath != NULL)	\
		__pAd->ate.pChipStruct->AsicSetTxRxPath(__pAd)


#ifdef RTMP_MAC_PCI
#define ATEPCIReadBBPRegister(_A, _I, _pV)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             j, k;                               \
    if (!IS_RT65XX(_A)) {\
	    for (j=0; j<MAX_BUSY_COUNT; j++)                    \
	    {                                                   \
	        RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
	        if (BbpCsr.field.Busy == BUSY)                  \
	        {                                               \
	            continue;                                   \
	        }                                               \
	        BbpCsr.word = 0;                                \
	        BbpCsr.field.fRead = 1;                         \
	        BbpCsr.field.BBP_RW_MODE = 1;                         \
	        BbpCsr.field.Busy = 1;                          \
	        BbpCsr.field.RegNum = _I;                       \
	        RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
	        for (k=0; k<MAX_BUSY_COUNT; k++)                \
	        {                                               \
	            RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word); \
	            if (BbpCsr.field.Busy == IDLE)              \
	                break;                                  \
	        }                                               \
	        if ((BbpCsr.field.Busy == IDLE) &&              \
	            (BbpCsr.field.RegNum == _I))                \
	        {                                               \
	            *(_pV) = (UCHAR)BbpCsr.field.Value;         \
	            break;                                      \
	        }                                               \
	    }                                                   \
	    if (BbpCsr.field.Busy == BUSY)                      \
	    {                                                   \
	        DBGPRINT_ERR(("BBP read R%d fail\n", _I));      \
	        *(_pV) = (_A)->BbpWriteLatch[_I];               \
	    }                                                   \
    	}\
}

#define ATEPCIWriteBBPRegister(_A, _I, _V)        \
{                                                       \
	BBP_CSR_CFG_STRUC  BbpCsr;                             \
	int             BusyCnt;                            \
	if (!IS_RT65XX(_A)) {\
		for (BusyCnt=0; BusyCnt<MAX_BUSY_COUNT; BusyCnt++)  \
		{                                                   \
		    RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
		    if (BbpCsr.field.Busy == BUSY)                  \
		        continue;                                   \
		    BbpCsr.word = 0;                                \
		    BbpCsr.field.fRead = 0;                         \
		    BbpCsr.field.BBP_RW_MODE = 1;                         \
		    BbpCsr.field.Busy = 1;                          \
		    BbpCsr.field.Value = _V;                        \
		    BbpCsr.field.RegNum = _I;                       \
		    RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
		    (_A)->BbpWriteLatch[_I] = _V;                   \
		    break;                                          \
		}                                                   \
		if (BusyCnt == MAX_BUSY_COUNT)                      \
		{                                                   \
		    DBGPRINT_ERR(("BBP write R%d fail\n", _I));     \
		}                                                   \
	}\
}


#define ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)		ATEPCIReadBBPRegister(_A, _I, _pV)
#define ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)		ATEPCIWriteBBPRegister(_A, _I, _V)
#endif /* RTMP_MAC_PCI */


INT DefaultATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);





#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd);

INT RT28xxATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* defined(RT28xx) || defined(RT2880) */



#ifdef CONFIG_QA
VOID ATE_QA_Statistics(
	IN RTMP_ADAPTER *pAd,
	IN RXWI_STRUC *pRxWI,
	IN RXINFO_STRUC *pRxInfo,
	IN PHEADER_802_11 pHeader);
	
INT RtmpDoAte(
	IN RTMP_ADAPTER *pAd, 
	IN RTMP_IOCTL_INPUT_STRUCT *wrq,
	IN RTMP_STRING *wrq_name);

INT Set_TxStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RxStop_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DBG
INT Set_EERead_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_EEWrite_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BBPRead_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_BBPWrite_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RFWrite_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DBG */ 
#endif /* CONFIG_QA */

#ifdef RTMP_RF_RW_SUPPORT
#ifdef RLT_RF
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _B, _I, _pV)     rlt_rf_read(_A, _B, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _B, _I, _V)     rlt_rf_write(_A, _B, _I, _V)
#endif /* RLT_RF */
#ifdef RTMP_RF
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _I, _pV)     RT30xxReadRFRegister(_A, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _I, _V)     RT30xxWriteRFRegister(_A, _I, _V)
#endif /* RTMP_RF */
#ifdef MT_RF
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _I, _V)
#endif /* MT_RF */
#endif /* RTMP_RF_RW_SUPPORT */

#ifdef CONFIG_QA
#define SYNC_CHANNEL_WITH_QA(_A, _pV)\
	if ((_A->bQATxStart == TRUE) || (_A->bQARxStart == TRUE))\
	{\
		return;\
	}\
	else\
		*_pV = _A->Channel
#else
#define SYNC_CHANNEL_WITH_QA(_A, _pV)\
	*_pV = _A->Channel
#endif /* CONFIG_QA */

VOID rt_ee_read_all(
	IN  PRTMP_ADAPTER   pAd,
	OUT USHORT *Data);

VOID rt_ee_write_all(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT *Data);

VOID rt_ee_write_bulk(
	IN  PRTMP_ADAPTER	pAd,
	IN  USHORT *Data,
	IN  USHORT offset,
	IN  USHORT length);

INT Set_ATE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_DA_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_SA_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_BSSID_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


INT Set_ATE_CHANNEL_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_INIT_CHAN_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ADCDump_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
INT Set_ATE_TSSI_CALIBRATION_EX_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

#ifdef RTMP_INTERNAL_TX_ALC
INT Set_ATE_TSSI_CALIBRATION_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TSSI_CALIBRATION_EX_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


#if defined(RT3350) || defined(RT3352)
INT RT335x_Set_ATE_TSSI_CALIBRATION_ENABLE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

CHAR InsertTssi(
	IN UCHAR InChannel,
	IN UCHAR Channel0,
	IN UCHAR Channel1,
	IN CHAR Tssi0,
	IN CHAR Tssi1);

INT RT335xATETssiCalibrationExtend(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* defined(RT3350) || defined(RT3352) */

CHAR ATEGetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd);

#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef RTMP_TEMPERATURE_CALIBRATION
INT Set_ATE_TEMP_CAL_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_SHOW_TSSI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#endif /* RTMP_TEMPERATURE_CALIBRATION */

#ifdef RTMP_TEMPERATURE_COMPENSATION

INT Set_ATE_READ_EXTERNAL_TSSI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

INT Set_ATE_TX_POWER0_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_POWER1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef DOT11N_SS3_SUPPORT
INT Set_ATE_TX_POWER2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* DOT11N_SS3_SUPPORT */

INT Set_ATE_TX_POWER_EVALUATION_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_RX_Antenna_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID DefaultATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd);

VOID ATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER	pAd);
#ifdef RT3350
INT Set_ATE_PA_Bias_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RT3350 */

INT Default_Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);





#if defined(RT28xx) || defined(RT2880)
INT RT28xx_Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* defined(RT28xx) || defined(RT2880) */


INT Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Default_Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);





#if defined(RT28xx) || defined(RT2880)
INT RT28xx_Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* defined(RT28xx) || defined(RT2880) */


INT Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_LENGTH_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_COUNT_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_MCS_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_STBC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_MODE_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TX_GI_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);


INT Set_ATE_RX_FER_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Read_RF_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#if (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF))
INT Set_ATE_Write_RF1_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Write_RF2_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Write_RF3_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Write_RF4_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* (!defined(RTMP_RF_RW_SUPPORT)) && (!defined(RLT_RF)) */

INT Set_ATE_Load_E2P_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef RTMP_EFUSE_SUPPORT
INT Set_ATE_Load_E2P_From_Buf_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* RTMP_EFUSE_SUPPORT */

INT Set_ATE_Read_E2P_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */

INT Set_ATE_AUTO_ALC_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_TEMP_SENSOR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_IPG_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Payload_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Fixed_Payload_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);



INT Set_ATE_TTR_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Show_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ATE_Help_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID DefaultATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);




VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);


VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN RXWI_STRUC *pRxWI);	

#ifdef RTMP_MAC_PCI
NDIS_STATUS ATEPayloadAlloc(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 idx);

PNDIS_PACKET ATEPayloadInit(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 TxIdx);

NDIS_STATUS TxRingGetReady(
	IN PRTMP_ADAPTER pAd);
#endif /* RTMP_MAC_PCI */


INT TxDmaBusy(RTMP_ADAPTER *pAd);
INT RxDmaBusy(RTMP_ADAPTER *pAd);

VOID RtmpDmaEnable(RTMP_ADAPTER *pAd, INT Enable);

INT ATESetUpFrame(RTMP_ADAPTER *pAd, UINT32 TxIdx);



NDIS_STATUS ChipStructAssign(
 IN	PRTMP_ADAPTER pAd);

NDIS_STATUS ATEInit(
 IN PRTMP_ADAPTER pAd);

#ifdef CONFIG_QA
VOID ReadQATxTypeFromBBP(
 IN	PRTMP_ADAPTER	pAd);
#endif /* CONFIG_QA */

#ifdef RTMP_BBP
NDIS_STATUS ATEBBPWriteWithRxChain(
 IN RTMP_ADAPTER *pAd,
 IN UCHAR bbpId,
 IN CHAR bbpVal,
 IN RX_CHAIN_IDX rx_ch_idx);
#endif /* RTMP_BBP */






#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATERxVGAInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* defined(RT28xx) || defined(RT2880) */



VOID  ATEPeriodicExec(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3);

VOID ATEAsicSetTxRxPath(RTMP_ADAPTER *pAd);

VOID RtmpRfIoWrite(RTMP_ADAPTER *pAd);

VOID ATEAsicSwitchChannel(RTMP_ADAPTER *pAd);

VOID BbpSoftReset(RTMP_ADAPTER *pAd);


#endif /* __RT_ATE_H__ */

