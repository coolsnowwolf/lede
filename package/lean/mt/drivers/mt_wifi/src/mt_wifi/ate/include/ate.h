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
#include "LoopBack.h"

#ifdef MT7615
#define ARBITRARY_CCK_OFDM_TX 1
#endif
#ifndef COMPOS_TESTMODE_WIN
/* #define LOGDUMP_TO_FILE 1 */
#define ATE_TXTHREAD 1
#endif
#define IOCTLBUFF 2048
#define ATE_ON(_p) ((((_p)->ATECtrl.Mode) & ATE_START) == ATE_START)
INT32 ATEInit(struct _RTMP_ADAPTER *pAd);
INT32 ATEExit(struct _RTMP_ADAPTER *pAd);

#define FREQ_OFFSET_MANUAL_ENABLE	0x81021238
#define FREQ_OFFSET_MANUAL_VALUE	0x81021234

/* For CA53 GPIO CR remap usage */
#define CA53_GPIO_REMAP_SIZE 0x10

#ifdef ARBITRARY_CCK_OFDM_TX
/* MCU PTA CR */
#define ANT_SWITCH_CON2 0x810600CC
#define ANT_SWITCH_CON3 0x810600D0
#define ANT_SWITCH_CON4 0x810600D4
#define ANT_SWITCH_CON6 0x810600DC
#define ANT_SWITCH_CON7 0x810600E0
#define ANT_SWITCH_CON8 0x810600E4
#endif

/*
 *	Use bitmap to allow coexist of ATE_TXFRAME
 *	and ATE_RXFRAME(i.e.,to support LoopBack mode).
 */
#define fATE_IDLE				(1 << 0)
#define fATE_TX_ENABLE				(1 << 1)
#define fATE_RX_ENABLE				(1 << 2)
#define fATE_TXCONT_ENABLE			(1 << 3)
#define fATE_TXCARR_ENABLE			(1 << 4)
#define fATE_TXCARRSUPP_ENABLE			(1 << 5)
#define fATE_MPS				(1 << 6)
#define fATE_FFT_ENABLE				(1 << 7)
#define fATE_EXIT				(1 << 8)
#define fATE_IN_RFTEST				(1 << 9)
#define fATE_IN_BF				(1 << 10)
#define fATE_IN_ICAPOVERLAP			(1 << 11)
/* Stop Transmission */
#define ATE_TXSTOP				((~(fATE_TX_ENABLE))&(~(fATE_TXCONT_ENABLE))&(~(fATE_TXCARR_ENABLE))&(~(fATE_TXCARRSUPP_ENABLE))&(~(fATE_MPS)))
/* Stop Receiving Frames */
#define ATE_RXSTOP				(~(fATE_RX_ENABLE))

/* Enter/Reset ATE */
#define	ATE_START				(fATE_IDLE)
/* Stop/Exit ATE */
#define	ATE_STOP				(fATE_EXIT)
/* Continuous Transmit Frames (without time gap) */
#define	ATE_TXCONT				((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE))
/* Transmit Carrier */
#define	ATE_TXCARR				((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE))
/* Transmit Carrier Suppression (information without carrier) */
#define	ATE_TXCARRSUPP				((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE))
/* Transmit Frames */
#define	ATE_TXFRAME				(fATE_TX_ENABLE)
/* Receive Frames */
#define	ATE_RXFRAME				(fATE_RX_ENABLE)
/* MPS */
#define	ATE_MPS					((fATE_TX_ENABLE)|(fATE_MPS))

#define ATE_FFT					((fATE_FFT_ENABLE)|(fATE_IN_RFTEST))

#define BULK_OUT_LOCK(pLock, IrqFlags)		\
		RTMP_IRQ_LOCK((pLock), IrqFlags)

#define BULK_OUT_UNLOCK(pLock, IrqFlags)	\
		RTMP_IRQ_UNLOCK((pLock), IrqFlags)

/* WiFi PHY mode capability */
#define PHYMODE_CAP_24G		(WMODE_B | WMODE_G | WMODE_GN)
#define PHYMODE_CAP_5G		(WMODE_A | WMODE_AN | WMODE_AC)
#define PHYMODE_CAP_DUAL_BAND	(PHYMODE_CAP_24G | PHYMODE_CAP_5G)

/* ContiTxTone */
#define WF0_TX_ONE_TONE_5M		0x0
#define WF0_TX_TWO_TONE_5M		0x1
#define WF1_TX_ONE_TONE_5M		0x2
#define WF1_TX_TWO_TONE_5M		0x3
#define WF0_TX_ONE_TONE_10M		0x4
#define WF1_TX_ONE_TONE_10M		0x5
#define WF0_TX_ONE_TONE_DC		0x6
#define WF1_TX_ONE_TONE_DC		0x7

#define MAX_TEST_PKT_LEN        1496
#define MIN_TEST_PKT_LEN        25
#define MAX_TEST_BKCR_NUM       30

/* For packet tx time, in unit of byte */
#define MAX_HT_AMPDU_LEN        65000
#define MAX_VHT_MPDU_LEN        6700    /* 11454 */
#define DEFAULT_MPDU_LEN        4096
#define MAX_MSDU_LEN            2304
#define MIN_MSDU_LEN            22
#define DEFAULT_MAC_HDR_LEN     24
#define QOS_MAC_HDR_LEN         26

/* For ipg and duty cycle, in unit of us */
#define SIG_EXTENSION           6
#define DEFAULT_SLOT_TIME       9
#define DEFAULT_SIFS_TIME       10
#define MAX_SIFS_TIME           127     /* ICR has 7-bit only */ /* For the ATCR/TRCR limitation 8-bit/9-bit only*/
#define MAX_AIFSN               0xF
#define MIN_AIFSN               0x1
#define MAX_CW                  0x10
#define MIN_CW                  0x0
#define NORMAL_CLOCK_TIME       50      /* in uint of ns */
#define BBP_PROCESSING_TIME     1500    /* in uint of ns */

/* The expected enqueue packet number in one time RX event trigger */
#define ATE_ENQUEUE_PACKET_NUM	100

#if defined(MT7615) || defined(MT7622)
#define ATE_TESTPKT_LEN	13311	/* Setting max packet length to 13311 on MT7615 */
#else
#define ATE_TESTPKT_LEN	4095	/* AMPDU delimiter 12 bit, maximum 4095 */
#endif
#define ATE_MAX_PATTERN_SIZE 128
#define TESTMODE_BAND0 0
#define TESTMODE_BAND1 1
#define ATE_BF_WCID 1
#define ATE_BFMU_NUM 4
struct _RTMP_ADAPTER;
struct _RX_BLK;

#ifdef DBDC_MODE
#define IS_ATE_DBDC(_pAd) _pAd->CommonCfg.dbdc_mode
#define TESTMODE_BAND_NUM 2
#else
#define IS_ATE_DBDC(_pAd) FALSE
#define TESTMODE_BAND_NUM 1
#endif

/* Antenna mode */
#define ANT_MODE_DEFAULT 0
#define ANT_MODE_SPE_IDX 1

#if !defined(COMPOS_TESTMODE_WIN)
/* Allow Sleep */
#define TESTMODE_SEM struct semaphore
#define	TESTMODE_SEM_INIT(_psem, _val) sema_init(_psem, _val)
#define TESTMODE_SEM_DOWN(_psem) down(_psem)
#define TESTMODE_SEM_DOWN_INTERRUPTIBLE(_psem) down_interruptible(_psem)
#define TESTMODE_SEM_DOWN_TRYLOCK(_psem) down_trylock(_psem)
#define TESTMODE_SEM_UP(_psem) up(_psem)
#endif

enum _TESTMODE_MODE {
	HQA_VERIFY,
	ATE_LOOPBACK,
	MODE_NUM
};

enum _MPS_PARAM_TYPE {
	MPS_SEQDATA,
	MPS_PHYMODE,
	MPS_PATH,
	MPS_RATE,
	MPS_PAYLOAD_LEN,
	MPS_TX_COUNT,
	MPS_PWR_GAIN,
	MPS_PARAM_NUM,
	MPS_NSS,
	MPS_PKT_BW,
};

struct _ATE_OPERATION {
	INT32 (*ATEStart)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ATEStop)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartTx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StopTx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StopRx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetTxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetRxPath)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetTxPower0)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower1)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower2)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxPower3)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxForceTxPower)(struct _RTMP_ADAPTER *pAd, INT8 cTxPower, UINT8 ucPhyMode, UINT8 ucTxRate, UINT8 ucBW);
	INT32 (*SetTxPowerX)(struct _RTMP_ADAPTER *pAd, struct _ATE_TXPOWER TxPower);
	INT32 (*SetTxAntenna)(struct _RTMP_ADAPTER *pAd, UINT32 Ant);
	INT32 (*SetRxAntenna)(struct _RTMP_ADAPTER *pAd, UINT32 Ant);
	INT32 (*SetTxFreqOffset)(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset);
	INT32 (*GetTxFreqOffset)(struct _RTMP_ADAPTER *pAd, UINT32 *FreqOffset);
	INT32 (*SetChannel)(struct _RTMP_ADAPTER *pAd, INT16 Value, UINT32 pri_sel, UINT32 reason, UINT32 Ch_Band);
	INT32 (*SetBW)(struct _RTMP_ADAPTER *pAd, UINT16 system_bw, UINT16 per_pkt_bw);
	INT32 (*SetDutyCycle)(struct _RTMP_ADAPTER *pAd, UINT32 value);
	INT32 (*SetPktTxTime)(struct _RTMP_ADAPTER *pAd, UINT32 value);
	INT32 (*SampleRssi)(struct _RTMP_ADAPTER *pAd, struct _RX_BLK *pRxBlk);
	INT32 (*SetIPG)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetSlotTime)(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime);
	INT32 (*SetAIFS)(struct _RTMP_ADAPTER *pAd, CHAR Value);
	INT32 (*SetPowerDropLevel)(struct _RTMP_ADAPTER *pAd, UINT32 PowerDropLevel);
	INT32 (*SetTSSI)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
	INT32 (*LowPower)(struct _RTMP_ADAPTER *pAd, UINT32 Control);
	INT32 (*SetDPD)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, CHAR Setting);
	INT32 (*StartTxTone)(struct _RTMP_ADAPTER *pAd, UINT32 Mode);
	INT32 (*SetTxTonePower)(struct _RTMP_ADAPTER *pAd, INT32 pwr1, INT pwr2);
	INT32 (*SetDBDCTxTonePower)(struct _RTMP_ADAPTER *pAd, INT32 pwr1, INT pwr2, UINT32 AntIdx);
	INT32 (*StopTxTone)(struct _RTMP_ADAPTER *pAd);
	INT32 (*StartContinousTx)(struct _RTMP_ADAPTER *pAd, CHAR WFSel, UINT32 TxfdMode);
	INT32 (*StopContinousTx)(struct _RTMP_ADAPTER *pAd, UINT32 TxfdMode);
	INT32 (*EfuseGetFreeBlock)(struct _RTMP_ADAPTER *pAd, UINT32 GetFreeBlock, UINT32 *Value);
	INT32 (*EfuseAccessCheck)(struct _RTMP_ADAPTER *pAd, UINT32 offset, PUCHAR pData);
	INT32 (*RfRegWrite)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 Value);
	INT32 (*RfRegRead)(struct _RTMP_ADAPTER *pAd, UINT32 WFSel, UINT32 Offset, UINT32 *Value);
	INT32 (*GetFWInfo)(struct _RTMP_ADAPTER *pAd, UCHAR *FWInfo);
#ifdef PRE_CAL_MT7622_SUPPORT
	INT32 (*TxDPDTest7622)(struct _RTMP_ADAPTER *pAd, RTMP_STRING * arg);
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	INT32 (*RxSelfTest)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	INT32 (*TxDPDTest)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	INT32 (*PreCalTest)(struct _RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId);    
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	INT32 (*SetATETxSoundingProc)(struct _RTMP_ADAPTER *pAd, UCHAR SoundingMode);
	INT32 (*StartTxSKB)(struct _RTMP_ADAPTER *pAd);
#endif /* TXBF_SUPPORT && MT_MAC */
	INT32 (*MPSSetParm)(struct _RTMP_ADAPTER *pAd, enum _MPS_PARAM_TYPE data_type, INT32 items, UINT32 *data);
	INT32 (*MPSTxStart)(struct _RTMP_ADAPTER *pAd);
	INT32 (*MPSTxStop)(struct _RTMP_ADAPTER *pAd);
	INT32 (*SetAutoResp)(struct _RTMP_ADAPTER *pAd, UCHAR *mac, UCHAR mode);
	INT32 (*SetFFTMode)(struct _RTMP_ADAPTER *pAd, UINT32 mode);
	INT32 (*onOffRDD)(struct _RTMP_ADAPTER *pAd, UINT32 rdd_num, UINT32 rdd_in_sel, UINT32 is_start);
	INT32 (*SetCfgOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 Type, UINT32 Enable);
	INT32 (*GetCfgOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 Type, UINT32 *Result);
	INT32 (*SetRXFilterPktLen)(struct _RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 RxPktLen);
#ifdef TXPWRMANUAL
	INT32 (*SetTxPwrManual)(struct _RTMP_ADAPTER *pAd, BOOLEAN fgPwrManCtrl, UINT8 u1TxPwrModeManual, UINT8 u1TxPwrBwManual, UINT8 u1TxPwrRateManual, INT8 i1TxPwrValueManual, UCHAR Band);
#endif
	INT32 (*DBDCTxTone)(struct _RTMP_ADAPTER *pAd, UINT32 Control, UINT32 AntIndex, UINT32 ToneType, UINT32 ToneFreq, INT32 DcOffset_I, INT32 DcOffset_Q, UINT32 Band);
	INT32 (*TxCWTone)(struct _RTMP_ADAPTER *pAd, UINT32 Control);
	INT32 (*GetTxPower)(struct _RTMP_ADAPTER *pAd, UINT32 Enable, UINT32 Ch_Band, UINT32 *EfuseAddr, UINT32 *Power);
	INT32 (*BssInfoUpdate)(struct _RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UINT32 BssIdx, UCHAR *Bssid);
	INT32 (*DevInfoUpdate)(struct _RTMP_ADAPTER *pAd, UINT32 OwnMacIdx, UCHAR *Bssid);
	INT32 (*LogOnOff)(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 on_off, UINT32 num_log);
	INT32 (*SetICapStart)(struct _RTMP_ADAPTER *pAd, BOOLEAN Trigger, BOOLEAN RingCapEn, UINT32 Event, UINT32 Node, UINT32 Len, UINT32 StopCycle, UINT32 BW, UINT32 MACTriggerEvent, UINT32 SourceAddrLSB, UINT32 SourceAddrMSB, UINT32 Band);
	INT32 (*GetICapStatus)(struct _RTMP_ADAPTER *pAd);
	INT32 (*GetICapIQData)(struct _RTMP_ADAPTER *pAd, PINT32 pData, PINT32 pDataLen, UINT32 IQ_Type, UINT32 WF_Num);
	INT32 (*SetAntennaPort)(struct _RTMP_ADAPTER *pAd, UINT32 RfModeMask, UINT32 RfPortMask, UINT32 AntPortMask);
	INT32 (*ClockSwitchDisable)(struct _RTMP_ADAPTER *pAd, UINT8 isDisable);
};

struct _ATE_IF_OPERATION {
	INT32 (*init)(struct _RTMP_ADAPTER *pAd);
	INT32 (*clean_trx_q)(struct _RTMP_ADAPTER *pAd);
	INT32 (*setup_frame)(struct _RTMP_ADAPTER *pAd, UINT32 q_idx);
	INT32 (*test_frame_tx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*test_frame_rx)(struct _RTMP_ADAPTER *pAd);
	INT32 (*ate_leave)(struct _RTMP_ADAPTER *pAd);
};

struct _HQA_MPS_SETTING {
	UINT32 phy;
	UINT32 pkt_len;
	UINT32 pkt_cnt;
	UINT32 pwr;
	UINT32 nss;
	UINT32 pkt_bw;
};

enum _TEST_BK_CR_TYPE {
	TEST_EMPTY_BKCR = 0,
	TEST_MAC_BKCR,
	TEST_HIF_BKCR,
	TEST_PHY_BKCR,
	TEST_HW_BKCR,
	TEST_MCU_BKCR,
	TEST_BKCR_TYPE_NUM,
};

struct _TESTMODE_BK_CR {
	ULONG offset;
	UINT32 val;
	enum _TEST_BK_CR_TYPE type;
};

struct _ATE_RXV_LOG {
	RX_VECTOR1_1ST_CYCLE rxv1_1st;
	RX_VECTOR1_2ND_CYCLE rxv1_2nd;
	RX_VECTOR1_3TH_CYCLE rxv1_3rd;
	RX_VECTOR1_4TH_CYCLE rxv1_4th;
	RX_VECTOR1_5TH_CYCLE rxv1_5th;
	RX_VECTOR1_6TH_CYCLE rxv1_6th;
	RX_VECTOR2_1ST_CYCLE rxv2_1st;
	RX_VECTOR2_2ND_CYCLE rxv2_2nd;
	RX_VECTOR2_3TH_CYCLE rxv2_3rd;
};

#define ATE_RDD_LOG_SIZE 8 /* Pulse size * num of pulse = 8 * 32 for one event*/
struct _ATE_RDD_LOG {
	UINT32 u4Prefix;
	UINT32 u4Count;
	UINT8 byPass;
	UINT8 aucBuffer[ATE_RDD_LOG_SIZE];
};

#define ATE_RECAL_LOG_SIZE (CAL_ALL_LEN >> 3)
struct _ATE_LOG_RECAL {
	UINT32 cal_idx;
	UINT32 cal_type;
	UINT32 cr_addr;
	UINT32 cr_val;
};

enum {
	ATE_LOG_RXV = 1,
	ATE_LOG_RDD,
	ATE_LOG_RE_CAL,
	ATE_LOG_TYPE_NUM,
	ATE_LOG_RXINFO,
	ATE_LOG_TXDUMP,
	ATE_LOG_TEST,
};

enum {
	ATE_LOG_OFF,
	ATE_LOG_ON,
	ATE_LOG_DUMP,
	ATE_LOG_CTRL_NUM,
};

#define fATE_LOG_RXV				(1 << ATE_LOG_RXV)
#define fATE_LOG_RDD				(1 << ATE_LOG_RDD)
#define fATE_LOG_RE_CAL				(1 << ATE_LOG_RE_CAL)
#define fATE_LOG_RXINFO				(1 << ATE_LOG_RXINFO)
#define fATE_LOG_TXDUMP				(1 << ATE_LOG_TXDUMP)
#define fATE_LOG_TEST				(1 << ATE_LOG_TEST)

struct _ATE_LOG_DUMP_ENTRY {
	UINT32 log_type;
	UINT8 un_dumped;
	union {
		struct _ATE_RXV_LOG rxv;
		struct _ATE_RDD_LOG rdd;
		struct _ATE_LOG_RECAL re_cal;
	} log;
};

struct _ATE_LOG_DUMP_CB {
	NDIS_SPIN_LOCK lock;
	UINT8 overwritable;
	UINT8 is_dumping;
	UINT8 is_overwritten;
	INT32 idx;
	INT32 len;
	UINT32 recal_curr_type;
#ifdef LOGDUMP_TO_FILE
	INT32 file_idx;
	RTMP_OS_FD_EXT fd;
#endif
	struct _ATE_LOG_DUMP_ENTRY *entry;
};

#define ATE_MPS_ITEM_RUNNING	(1<<0)
struct _HQA_MPS_CB {
	NDIS_SPIN_LOCK lock;
	UINT32 mps_cnt;
	UINT32 band_idx;
	UINT32 stat;
	BOOLEAN setting_inuse;
	UINT32 ref_idx;
	struct _HQA_MPS_SETTING *mps_setting;
};

struct _ATE_PFMU_INFO {
	UCHAR wcid;
	UCHAR bss_idx;
	UCHAR up;
	UCHAR addr[MAC_ADDR_LEN];
};

#define ATE_RXV_SIZE    9
#define ATE_ANT_NUM     4

struct _ATE_RX_STATISTIC {
	INT32 FreqOffsetFromRx;
	UINT32 RxTotalCnt[TESTMODE_BAND_NUM];
	UINT32 NumOfAvgRssiSample;
	UINT32 RxMacFCSErrCount;
	UINT32 RxMacMdrdyCount;
	UINT32 RxMacFCSErrCount_band1;
	UINT32 RxMacMdrdyCount_band1;
	CHAR LastSNR[ATE_ANT_NUM];		/* last received SNR */
	CHAR LastRssi[ATE_ANT_NUM];		/* last received RSSI */
	CHAR AvgRssi[ATE_ANT_NUM];		/* last 8 frames' average RSSI */
	CHAR MaxRssi[ATE_ANT_NUM];
	CHAR MinRssi[ATE_ANT_NUM];
	SHORT AvgRssiX8[ATE_ANT_NUM];		/* sum of last 8 frames' RSSI */
	UINT32 RSSI[ATE_ANT_NUM];
	UINT32 SNR[ATE_ANT_NUM];
	UINT32 RCPI[ATE_ANT_NUM];
	UINT32 FAGC_RSSI_IB[ATE_ANT_NUM];
	UINT32 FAGC_RSSI_WB[ATE_ANT_NUM];
#ifdef CFG_SUPPORT_MU_MIMO
	UINT32 RxMacMuPktCount;
#endif
	UINT32 SIG_MCS;
	UINT32 SINR;
	UINT32 RXVRSSI;
};

struct _ATE_TX_TIME_PARAM {
	BOOLEAN pkt_tx_time_en;	/* The packet transmission time feature enable or disable */
	UINT32 pkt_tx_time;	/* The target packet transmission time */
	UINT32 pkt_tx_len;
	UINT32 pkt_msdu_len;
	UINT32 pkt_hdr_len;
	UINT32 pkt_ampdu_cnt;
	UINT8 pkt_need_qos;
	UINT8 pkt_need_amsdu;
	UINT8 pkt_need_ampdu;
};

struct _ATE_IPG_PARAM {
	UINT32 ipg;         /* The target idle time */
	UINT8 sig_ext;      /* Only OFDM/HT/VHT need to consider sig_ext */
	UINT16 slot_time;
	UINT16 sifs_time;
	UINT8 ac_num;       /* 0: AC_BK, 1: AC_BE, 2: AC_VI, 3: AC_VO */
	UINT8 aifsn;
	UINT16 cw;
	UINT16 txop;
};

#ifdef DBDC_MODE
struct _BAND_INFO {
	UCHAR *pate_pkt;	/* Buffer for TestPkt */
	PNDIS_PACKET pkt_skb;
	UINT32 is_alloc_skb;
	RTMP_OS_COMPLETION tx_wait;
	UCHAR TxStatus;	/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UINT32 Mode;
	UINT32 TxAntennaSel;
	UINT32 RxAntennaSel;
	UCHAR backup_channel;
	UCHAR backup_phymode;
	UCHAR wdev_idx;
	UCHAR wmm_idx;
	USHORT QID;
	UCHAR Channel;
	UCHAR Ch_Band;
	UCHAR ControlChl;
	UCHAR PriSel;
	UINT32 OutBandFreq;
	UCHAR Nss;
	UCHAR BW;
	UCHAR PerPktBW;
	UCHAR PrimaryBWSel;
	UCHAR PhyMode;
	UCHAR Stbc;
	UCHAR Ldpc;	/* 0:BCC 1:LDPC */
	UCHAR Sgi;
	UCHAR Mcs;
	UCHAR Preamble;
	UINT32 FixedPayload;	/* Normal:0,Repeat:1,Random:2 */
	UINT32 TxLength;
	UINT32 TxCount;
	UINT32 TxDoneCount;	/* Tx DMA Done */
	UINT32 TxedCount;
	UINT32 RFFreqOffset;
	UINT32 thermal_val;
	UINT32 duty_cycle;
	struct _ATE_TX_TIME_PARAM tx_time_param;
	struct _ATE_IPG_PARAM ipg_param;
#ifdef TXBF_SUPPORT
	UCHAR eTxBf;
	UCHAR iTxBf;
#endif
#ifdef DOT11_VHT_AC
	UCHAR Channel_2nd;
#endif
	UCHAR FAGC_Path;
	/* Tx frame */
	UCHAR TemplateFrame[32];
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR payload[ATE_MAX_PATTERN_SIZE];
	UINT32 pl_len;
	USHORT HLen;		/* Header Length */
	USHORT seq;
	struct _HQA_MPS_CB mps_cb;
	BOOLEAN  fgTxPowerSKUEn;        /* SKU On/Off status */
	BOOLEAN  fgTxPowerPercentageEn; /* Power Percentage On/Off status */
	BOOLEAN  fgTxPowerBFBackoffEn;  /* BF Backoff On/Off status */
	UINT32   PercentageLevel;       /* TxPower Percentage Level */
	INT32    RF_Power;
	INT32    Digital_Power;
	INT32    DcOffset_I;
	INT32    DcOffset_Q;
};
#endif

#ifdef ATE_TXTHREAD
#define ATE_THREAD_NUM 1
struct _ATE_TXTHREAD_CB {
	BOOLEAN is_init;
	RTMP_OS_TASK task;
	NDIS_SPIN_LOCK lock;
	UINT32 tx_cnt;
	UINT32 txed_cnt;
	UCHAR service_stat;
};
#endif

struct _ATE_CTRL {
	struct _ATE_OPERATION *ATEOp;
	struct _ATE_IF_OPERATION *ATEIfOps;
	enum _TESTMODE_MODE verify_mode;
	struct _HQA_MPS_CB mps_cb;
	UINT32 en_log;
#ifdef ATE_TXTHREAD
	struct _ATE_TXTHREAD_CB tx_thread[1];
	UINT32 current_init_thread;
	INT32 deq_cnt;
#endif
#ifdef DBDC_MODE
	struct _BAND_INFO band_ext[1];
#endif
	UCHAR *pate_pkt;	/* Buffer for TestPkt */
	PNDIS_PACKET pkt_skb;
	UINT32 is_alloc_skb;
	UINT32 Mode;
	CHAR TxPower0;
	CHAR TxPower1;
	CHAR TxPower2;
	CHAR TxPower3;
	UINT32 TxAntennaSel;	/* band0 => TX0/TX1 , band1 => TX2/TX3 */
	UINT32 RxAntennaSel;
	UCHAR backup_channel;	/* Backup normal driver's channel for recovering */
	UCHAR backup_phymode;	/* Backup normal driver's phymode for recovering */
	UCHAR wdev_idx;
	UCHAR wmm_idx;
	USHORT QID;
	UCHAR Channel;
	UCHAR Ch_Band;
	UCHAR ControlChl;
	UCHAR PriSel;
	UINT32 OutBandFreq;
	UCHAR Nss;
	UCHAR BW;
	UCHAR PerPktBW;
	UCHAR PrimaryBWSel;
	UCHAR PhyMode;
	UCHAR Stbc;
	UCHAR Ldpc;		/* 0:BCC 1:LDPC */
	UCHAR Sgi;
	UCHAR Mcs;
	UCHAR Preamble;
	UCHAR Payload;		/* Payload pattern */
	UINT32 FixedPayload;
	UINT32 TxLength;
	UINT32 TxCount;
	UINT32 TxDoneCount;	/* Tx DMA Done */
	UINT32 TxedCount;
	UINT32 RFFreqOffset;
	UINT32 thermal_val;
	UINT32 duty_cycle;
	struct _ATE_TX_TIME_PARAM tx_time_param;
	struct _ATE_IPG_PARAM ipg_param;
#ifdef TXBF_SUPPORT
	BOOLEAN fgEBfEverEnabled;
	BOOLEAN fgBw160;
	UCHAR eTxBf;
	UCHAR iTxBf;
	UCHAR *txbf_info;
	UINT32 txbf_info_len;
	UCHAR iBFCalStatus;
#endif
#ifdef INTERNAL_CAPTURE_SUPPORT
	EXT_EVENT_RBIST_ADDR_T icap_info;
#endif /* INTERNAL_CAPTURE_SUPPORT */
#ifdef DOT11_VHT_AC
	UCHAR Channel_2nd;
#endif
	/* Common part */
	UCHAR control_band_idx; /* The band_idx which user wants to control currently */
	/* Tx frame */
	UCHAR *TemplateFrame;
	UCHAR Addr1[MAC_ADDR_LEN];
	UCHAR Addr2[MAC_ADDR_LEN];
	UCHAR Addr3[MAC_ADDR_LEN];
	UCHAR payload[ATE_MAX_PATTERN_SIZE];
	UINT32 pl_len;
	USHORT HLen;		/* Header Length */
	USHORT seq;
	/* MU Related */
	BOOLEAN mu_enable;
	UINT32 mu_usrs;
	UINT8 wcid_ref;
	struct _ATE_PFMU_INFO pfmu_info[ATE_BFMU_NUM];
	/* counters */
	UINT32 num_rxv;
	UINT32 num_rxdata;
	UINT32 num_rxv_fcs;
	UINT32 num_rxdata_fcs;
	struct _ATE_RX_STATISTIC rx_stat;
	struct _ATE_LOG_DUMP_CB log_dump[ATE_LOG_TYPE_NUM];
	UCHAR FAGC_Path;
	/* Flag */
	BOOLEAN txs_enable;
	BOOLEAN	bQAEnabled;	/* QA is used. */
	BOOLEAN bQATxStart;	/* Have compiled QA in and use it to ATE tx. */
	BOOLEAN bQARxStart;	/* Have compiled QA in and use it to ATE rx. */
	BOOLEAN need_set_pwr;	/* For MPS switch power in right context */
	UCHAR TxStatus;		/* TxStatus : 0 --> task is idle, 1 --> task is running */
	UCHAR did_tx;
	UCHAR did_rx;
	UCHAR en_man_set_freq;
	/* Restore CR */
	struct _TESTMODE_BK_CR bk_cr[MAX_TEST_BKCR_NUM];
	/* OS related */
	RTMP_OS_COMPLETION tx_wait;
	RTMP_OS_COMPLETION cmd_done;
	ULONG cmd_expire;
#if !defined(COMPOS_TESTMODE_WIN)
	RALINK_TIMER_STRUCT PeriodicTimer;
	ULONG OneSecPeriodicRound;
	ULONG PeriodicRound;
	OS_NDIS_SPIN_LOCK TssiSemLock;
#endif
	BOOLEAN  fgTxPowerSKUEn;        /* SKU On/Off status */
	BOOLEAN  fgTxPowerPercentageEn; /* Power Percentage On/Off status */
	BOOLEAN  fgTxPowerBFBackoffEn;  /* BF Backoff On/Off status */
	UINT32   PercentageLevel;       /* TxPower Percentage Level */
	INT32    RF_Power;
	INT32    Digital_Power;
	INT32    DcOffset_I;
	INT32    DcOffset_Q;
};

#if !defined(COMPOS_TESTMODE_WIN)
VOID RtmpDmaEnable(RTMP_ADAPTER *pAd, INT Enable);

VOID ATE_RTUSBBulkOutDataPacket(
	IN      PRTMP_ADAPTER	pAd,
	IN      UCHAR		BulkOutPipeId);

VOID ATE_RTUSBCancelPendingBulkInIRP(
	IN      PRTMP_ADAPTER   pAd);
#endif
INT MtATESetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT SetATEApplyStaToMacTblEntry(RTMP_ADAPTER *pAd);
INT SetATEApplyStaToAsic(RTMP_ADAPTER *pAd);
#endif

#endif /*  __ATE_H__ */
