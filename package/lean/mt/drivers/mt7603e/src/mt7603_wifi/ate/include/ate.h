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
	ate.h
*/

#ifndef __ATE_H__
#define __ATE_H__

#define ATE_ON(_p) ((((_p)->ATECtrl.Mode) & ATE_START) == ATE_START)
INT32 ATEInit(struct _RTMP_ADAPTER *pAd);
INT32 ATEExit(struct _RTMP_ADAPTER *pAd);

/*
	Use bitmap to allow coexist of ATE_TXFRAME
	and ATE_RXFRAME(i.e.,to support LoopBack mode).
*/
#define fATE_IDLE					(1 << 0)
#define fATE_TX_ENABLE				(1 << 1)
#define fATE_RX_ENABLE				(1 << 2)
#define fATE_TXCONT_ENABLE			(1 << 3)
#define fATE_TXCARR_ENABLE			(1 << 4)
#define fATE_TXCARRSUPP_ENABLE		(1 << 5)
#define fATE_RESERVED_1				(1 << 6)
#define fATE_RFTEST_ENABLE			(1 << 7)
#define fATE_EXIT					(1 << 8)

/* Stop Transmission */
#define ATE_TXSTOP                  ((~(fATE_TX_ENABLE))&(~(fATE_TXCONT_ENABLE))&(~(fATE_TXCARR_ENABLE))&(~(fATE_TXCARRSUPP_ENABLE)))
/* Stop Receiving Frames */
#define ATE_RXSTOP					(~(fATE_RX_ENABLE))


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


#define ATE_RFTEST					(fATE_RFTEST_ENABLE)

#define BULK_OUT_LOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_LOCK((pLock), IrqFlags);
		
#define BULK_OUT_UNLOCK(pLock, IrqFlags)	\
		if(1 /*!(in_interrupt() & 0xffff0000)*/)	\
			RTMP_IRQ_UNLOCK((pLock), IrqFlags);

/* ContiTxTone */
#define WF0_TX_ONE_TONE_5M		0x0
#define WF0_TX_TWO_TONE_5M		0x1
#define WF1_TX_ONE_TONE_5M		0x2
#define WF1_TX_TWO_TONE_5M		0x3
#define WF0_TX_ONE_TONE_10M		0x4
#define WF1_TX_ONE_TONE_10M		0x5
#define WF0_TX_ONE_TONE_DC		0x6
#define WF1_TX_ONE_TONE_DC		0x7

struct _RTMP_ADAPTER;
struct _RX_BLK;

typedef struct _ATE_OPERATION {
	INT32 (*ATEStart)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ATEStop)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartTx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StopTx)(struct _RTMP_ADAPTER *pAd, UINT32 Mode);
	INT32 (*StopRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetTxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetRxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetTxPower0)(struct _RTMP_ADAPTER *pAd, CHAR Value);
	INT32 (*SetTxPower1)(struct _RTMP_ADAPTER *pAd, CHAR Value);
	INT32 (*SetTxAntenna)(struct _RTMP_ADAPTER *pAd, CHAR Ant);
	INT32 (*SetRxAntenna)(struct _RTMP_ADAPTER *pAd, CHAR Ant);
	INT32 (*SetTxFreqOffset)(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset);
	INT32 (*SetChannel)(struct _RTMP_ADAPTER *pAd, INT16 Value);
	INT32 (*SetBW)(struct _RTMP_ADAPTER *pAd, INT16 Value);
	INT32 (*SampleRssi)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
    INT32 (*SetAIFS)(struct _RTMP_ADAPTER *pAd, CHAR Value);
    INT32 (*SetTSSI)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
    INT32 (*SetDPD)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
    INT32 (*StartTxTone)(struct _RTMP_ADAPTER *pAd, UINT32 Mode);
	INT32 (*SetTxTonePower)(struct _RTMP_ADAPTER *pAd, INT32 pwr1, INT pwr2);
    INT32 (*StopTxTone)(struct _RTMP_ADAPTER *pAd);
    INT32 (*StartContinousTx)(struct _RTMP_ADAPTER *pAd, CHAR WFSel);
	INT32 (*RfRegWrite)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value);
	INT32 (*RfRegRead)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value);
	INT32 (*GetFWInfo)(struct _RTMP_ADAPTER *pAd, UCHAR *FWInfo);
} ATE_OPERATION;

typedef struct _ATE_CTRL {
	ATE_OPERATION *ATEOp;
#ifdef RTMP_MAC_PCI
	PVOID AteAllocVa[TX_RING_SIZE];
	PNDIS_PACKET pAtePacket[TX_RING_SIZE];
#endif /* RTMP_MAC_PCI */
	UINT32 Mode;
	BOOLEAN PassiveMode;
	CHAR TxPower0;
	CHAR TxPower1;
	CHAR TxPower2;
	CHAR MinTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR MaxTxPowerBandA; /* Power range of early chipsets is -7~15 in A band */
	CHAR TxAntennaSel;
	CHAR RxAntennaSel;
	USHORT TxInfoLen;
	USHORT TxWILen;
	USHORT QID;
	UCHAR *TemplateFrame;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR Channel;
	UCHAR ControlChl;
	UCHAR BW;
	UCHAR PhyMode;
	UCHAR Stbc;
	UCHAR Sgi;
	UCHAR Mcs;
	UCHAR Preamble;
	UCHAR Payload;		/* Payload pattern */
	BOOLEAN bFixedPayload;
	UCHAR TxMethod; /* Early chipsets must be applied old TXCONT/TXCARR/TXCARS mechanism. */
	UINT32 TxLength;
	UINT32 TxCount;
	UINT32 TxDoneCount;	/* Tx DMA Done */
	UINT32 RFFreqOffset;
	UINT32 IPG;
	BOOLEAN bRxFER;		/* Show Rx Frame Error Rate */
	/* Used when using QA Tool */
	BOOLEAN	bQAEnabled;	/* QA is used. */
	BOOLEAN bQATxStart;	/* Have compiled QA in and use it to ATE tx. */
	BOOLEAN bQARxStart;	/* Have compiled QA in and use it to ATE rx. */
	BOOLEAN bAutoTxAlc;	/* Set Auto Tx Alc */
	BOOLEAN bAutoVcoCal; /* Set Auto VCO periodic calibration. */
	BOOLEAN bLowTemperature; /* Trigger Temperature Sensor */
	BOOLEAN bTestTimeReduction; /* Set ATE test time reduction. */
#ifdef RTMP_MAC_PCI
	BOOLEAN bFWLoading;	/* Reload firmware when ATE is done. */
#endif				/* RTMP_MAC_PCI */
	UINT32 RxTotalCnt;
	UINT32 RxCntPerSec;

	CHAR LastSNR0;		/* last received SNR */
	CHAR LastSNR1;		/* last received SNR for 2nd  antenna */
	CHAR LastSNR2;
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
	UINT32 SNR2;
	INT32 BF_SNR[3];	/* Last RXWI BF SNR. Units=0.25 dB */
	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UCHAR TxStatus;
    UINT32 RCPI0;
    UINT32 RCPI1;
    UINT32 RCPI2;
    UINT32 FreqOffsetFromRx;
    UINT32 RxMacFCSErrCount;
    UINT32 RxMacMdrdyCount;
	UINT32 thermal_val;
#endif
	RTMP_OS_COMPLETION cmd_done;
	ULONG cmd_expire;
	RALINK_TIMER_STRUCT PeriodicTimer;
	ULONG OneSecPeriodicRound;
	ULONG PeriodicRound;
	OS_NDIS_SPIN_LOCK TssiSemLock;
	UINT32 tssi0_wf0_cr;
	UINT32 tssi0_wf1_cr;
	UINT32 tssi1_wf0_cr;
	UINT32 tssi1_wf1_cr;
	UINT32 phy_mux_27; 
	UINT32 rmac_pcr1;
	UINT32 rmac_pcr;
} ATE_CTRL;
VOID RtmpDmaEnable(RTMP_ADAPTER *pAd, INT Enable);


VOID ATE_RTUSBBulkOutDataPacket(
        IN      PRTMP_ADAPTER   pAd,
        IN      UCHAR                   BulkOutPipeId);

VOID ATE_RTUSBCancelPendingBulkInIRP(
        IN      PRTMP_ADAPTER   pAd);


#endif /*  __ATE_H__ */
