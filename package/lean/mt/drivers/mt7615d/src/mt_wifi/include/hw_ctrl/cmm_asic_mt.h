/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic_mt.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_ASIC_MT_H__
#define __CMM_ASIC_MT_H__

struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _EXT_CMD_CHAN_SWITCH_T;
struct _BCTRL_INFO_T;
struct _RX_BLK;
struct _BSS_INFO_ARGUMENT_T;

/* SIFS time, 20us, and assume bcn len is 512 byte, tx by 1Mbps.*/
#define BCN_TRANSMIT_ESTIMATE_TIME  (4096 + 20)

typedef enum {
	MT_BSS_MODE_AP = 0,
	MT_BSS_MODE_ADHOC,
	MT_BSS_MODE_STA,
} MT_BSS_OP_MODE_T;

typedef struct {
	UINT8 BssSet;						/*HW Bssid Set: 0~3*/
	UINT8 MbssidIdx;					/*Virtual Multi BSSID index: 0~15*/
	UINT8	PreTbttInterval;
	USHORT BeaconPeriod;
	USHORT DtimPeriod;
	MT_BSS_OP_MODE_T BssOpMode;
	UCHAR BandIdx;
} MT_BSS_SYNC_CTRL_T;

typedef enum {
	NO_PROTECT = 0,
	PROTECT_MIX_MODE = 1,
	PROTECT_HT40_ONLY = 2,
	PROTECT_HT40_HT20 = 3,
	PROTECT_MODE_END = 8
} PROTECT_OP_MODE_T;

#define ERP_DISABLE 0
#define ERP_OMAC0   (1 << 0)
#define ERP_OMAC1   (1 << 1)
#define ERP_OMAC2   (1 << 2)
#define ERP_OMAC3   (1 << 3)
#define ERP_OMAC4   (1 << 4)
#define ERP_OMAC_ALL    ((ERP_OMAC0)|(ERP_OMAC1)|(ERP_OMAC2)|(ERP_OMAC3)|(ERP_OMAC4))
typedef struct {
	/* for hdev */
	UINT8  band_idx;
	UINT8  long_nav;
	UINT8  mix_mode;
	UINT8  gf;
	UINT8  rifs;
	UINT8  bw40;
	UINT8  bw80;
	UINT8  bw160;
	UINT16 erp_mask;
} MT_PROTECT_CTRL_T;

#define PKT_NUM_THLD(x) (((x) >> 25) - 1)
#define PKT_LEN_THLD(x) ((x) & 0xfffff)
typedef struct {
	UINT8  band_idx;
	UINT32 pkt_len_thld;
	UINT16 pkt_num_thld;
	UINT16 retry_limit;
} MT_RTS_THRESHOLD_T;

#define RDG_DISABLE     0x0
#define RDG_INITIATOR   (1 << 0)
#define RDG_RESPONDER   (1 << 1)
#define RDG_BOTH        (RDG_INITIATOR|RDG_RESPONDER)
typedef struct _MT_RDG_CTRL_T {
	UINT8   WlanIdx;
	UINT8   BandIdx;
	UINT8   Init;
	UINT8   Resp;
	UINT8   LongNav;
	UINT32  Txop;
} MT_RDG_CTRL_T, *P_MT_RDG_CTRL_T;

typedef struct _MT_SWITCH_CHANNEL_CFG {
	UCHAR ControlChannel;
	/*Only used for 80+80 case*/
	UCHAR ControlChannel2;
	UCHAR CentralChannel;
	UCHAR Bw;
	UCHAR TxStream;
	UCHAR RxStream;
	BOOLEAN bScan;
#ifdef COMPOS_TESTMODE_WIN
	BOOLEAN isMCC;
#endif
#ifdef MT_DFS_SUPPORT
	BOOLEAN bDfsCheck;
#endif
	UCHAR BandIdx;
	UCHAR Channel_Band;
    UINT32 OutBandFreq;
} MT_SWITCH_CHANNEL_CFG;

typedef enum {
	RX_STBC_BCN_BC_MC = 1<<0,
	RX_FCS_ERROR = 1 << 1,
	RX_PROTOCOL_VERSION = 1 << 2, /*drop 802.11 protocol version not is 0*/
	RX_PROB_REQ = 1 << 3,
	RX_MC_ALL =  1 << 4,     /*drop all mcast frame*/
	RX_BC_ALL = 1 << 5,
	RX_MC_TABLE = 1 << 6,     /*drop mcast frame that is not in the mcast table*/
	RX_BC_MC_OWN_MAC_A3 = 1 << 7, /*drop bc/mc packet matches the following condition: ToDS=0,FromDS=1,A3=OwnMAC0 or OwnMAC1*/
	RX_BC_MC_DIFF_BSSID_A3 = 1 << 8, /*drop bc/mc packet matches the following condition: ToDS=0,FromDS=0,A3!=BSSID0 or BSSID1*/
	RX_BC_MC_DIFF_BSSID_A2 =  1 << 9,/*drop bc/mc packet matches the following condition: ToDS=0,FromDS=1,A2!=BSSID0 or BSSID1*/
	RX_BCN_DIFF_BSSID =  1 << 10,  /*drop bcn packet and A3!=BSSID0 or BSSID1*/
	RX_CTRL_RSV = 1 << 11,    /*drop control packets with reserve type*/
	RX_CTS = 1 << 12,
	RX_RTS = 1 << 13,
	RX_DUPLICATE = 1 << 14,   /*drop duplicate frame, BA session not includign in this filter*/
	RX_NOT_OWN_BSSID = 1 << 15,  /*drop not my BSSID0/1/2/3 if enabled*/
	RX_NOT_OWN_UCAST = 1 << 16,  /*drop uncast packet not to OWN MAC 0/1/2/3/4*/
	RX_NOT_OWN_BTIM = 1<<17,  /*drop diff bassid TIM broadcast*/
	RX_NDPA = 1<<18,     /*drop NDPA control frame*/
} MT_RX_FILTER_T;

typedef struct {
	UINT32  filterMask;
	BOOLEAN bPromiscuous;
	BOOLEAN bFrameReport;
	UCHAR u1BandIdx;
} MT_RX_FILTER_CTRL_T;

typedef enum _BA_SESSION_TYPE {
	BA_SESSION_INV = 0,
	BA_SESSION_ORI = 1,
	BA_SESSION_RECP = 2,
} BA_SESSION_TYPE;

typedef struct _MT_BA_CTRL_T {
	UCHAR Wcid;
	UCHAR Tid;
	UCHAR BaWinSize;
	BOOLEAN isAdd;
	UINT8 band_idx;
	BA_SESSION_TYPE BaSessionType;
	UCHAR PeerAddr[MAC_ADDR_LEN];
	UINT16 Sn;							/*Sequence number for a specific TID*/
} MT_BA_CTRL_T, *P_MT_BA_CTRL_T;

typedef enum {
	MT_WCID_TYPE_AP,
	MT_WCID_TYPE_CLI,
	MT_WCID_TYPE_APCLI,
	MT_WCID_TYPE_BMCAST,
	MT_WCID_TYPE_APCLI_MCAST,
	MT_WCID_TYPE_REPEATER,
	MT_WCID_TYPE_WDS,
} MT_WCID_TYPE_T;

typedef struct {
	USHORT Wcid;
	USHORT Aid;
	UINT8 BssidIdx;
	UINT8 MacAddrIdx;
	UINT8 SmpsMode;
	UINT8 MaxRAmpduFactor;
	UINT8 MpduDensity;
	UINT8 aad_om;
	UINT8 dyn_bw;
	MT_WCID_TYPE_T WcidType;
	UCHAR Addr[MAC_ADDR_LEN];
	UINT32 CipherSuit;
	UCHAR  PfmuId;
	UCHAR  spe_idx;
	UCHAR  gid;
	UCHAR  rca2;
	UCHAR  rv;
	BOOLEAN fgTiBf;
	BOOLEAN fgTeBf;
	BOOLEAN fgTibfVht;
	BOOLEAN fgTebfVht;
	BOOLEAN SupportHT;
	BOOLEAN SupportVHT;
	BOOLEAN SupportRDG;
	BOOLEAN SupportQoS;
	BOOLEAN DisRHTR;
	BOOLEAN IsReset;
#ifdef A4_CONN
	BOOLEAN a4_enable;
#endif
	BOOLEAN SkipClearPrevSecKey;
#ifdef MBSS_AS_WDS_AP_SUPPORT
    BOOLEAN fg4AddrEnable;
#endif
} MT_WCID_TABLE_INFO_T;

typedef enum _ENUM_CIPHER_SUIT_T {
	CIPHER_SUIT_NONE = 0,
	CIPHER_SUIT_WEP_40,
	CIPHER_SUIT_TKIP_W_MIC,
	CIPHER_SUIT_TKIP_WO_MIC,
	CIPHER_SUIT_CCMP_W_MIC, /* for DFP or 802.11w MFP */
	CIPHER_SUIT_WEP_104,
	CIPHER_SUIT_BIP,
	CIPHER_SUIT_WEP_128 = 7,
	CIPHER_SUIT_WPI,
	CIPHER_SUIT_CCMP_W_MIC_FOR_CCX = 9,  /* for DFP or CCX MFP */
	CIPHER_SUIT_CCMP_256,
	CIPHER_SUIT_GCMP_128,
	CIPHER_SUIT_GCMP_256
} ENUM_CIPHER_SUIT_T256, *P_ENUM_CIPHER_SUIT_T;

typedef struct _SW_CIPHER_ID_MAP_HW_ID_T {
	NDIS_802_11_WEP_STATUS  sw_id;
	ENUM_CIPHER_SUIT_T256   hw_id;
} SW_CIPHER_ID_MAP_HW_ID_T, *PSW_CIPHER_ID_MAP_HW_ID_T;

/* The security mode definition in MAC register */
typedef enum {
	CIPHER_NONE            = 0,
	CIPHER_WEP64           = 1,
	CIPHER_WEP128         = 2,
	CIPHER_TKIP               = 3,
	CIPHER_AES                 = 4,
	CIPHER_CKIP64           = 5,
	CIPHER_CKIP128         = 6,
	CIPHER_CKIP152         = 7,
	CIPHER_SMS4		   = 8,
	CIPHER_WEP152           = 9,
	CIPHER_BIP	                   = 10,
	CIPHER_WAPI             = 11,
	CIPHER_TKIP_NO_MIC = 12,
} MT_SEC_CIPHER_SUITS_T;

typedef struct {
	BOOLEAN isAdd;
	UCHAR BssIdx;
	UCHAR Wcid;
	MT_SEC_CIPHER_SUITS_T KeyType;
	UCHAR KeyIdx;
	MT_BSS_OP_MODE_T OpMode;
	struct _CIPHER_KEY *pCipherKey;
	UCHAR PeerAddr[MAC_ADDR_LEN];
} MT_SECURITY_CTRL;

typedef struct _MT_TX_COUNTER {
	UINT32 TxCount;
	UINT32 TxFailCount;
	UINT16 Rate1TxCnt;
	UINT16 Rate1FailCnt;
	UINT16 Rate2OkCnt;
	UINT16 Rate3OkCnt;
	UCHAR Rate2TxCnt;
	UCHAR Rate3TxCnt;
	UCHAR Rate4TxCnt;
	UCHAR Rate5TxCnt;
	UCHAR RateIndex;
} MT_TX_COUNTER;

typedef enum _MCU_STAT {
	METH1_RAM_CODE,
	METH1_ROM_CODE,
	METH3_FW_RELOAD,
	METH3_INIT_STAT,
	METH3_FW_PREPARE_LOAD,
	METH3_FW_RUN,
	MCU_STAT_END
} MCU_STAT;

typedef struct {
	INT mode;						/*0: LMAC,1: BYPASS, 2: HyBrid",*/
	BOOLEAN bBeaconSpecificGroup;	/*is want to allocate a specific group for beacon frame*/
} MT_DMASCH_CTRL_T;

/* mapping table entry for tttt cr and HW_bssid_idx*/
typedef struct _TTTT_CR_BSSID_IDX_MAP_T {
	UINT32             u4HwBssidIdx;
	UINT32             u4TTTTEnableCr;
	UINT32             u4TTTTIntervalCr;
} TTTT_CR_BSSID_IDX_MAP_T, *PTTTT_CR_BSSID_IDX_MAP_T;

typedef enum _HWBSSID_TABLE {
	HW_BSSID_0 = 0,
	HW_BSSID_1,
	HW_BSSID_2,
	HW_BSSID_3,
	HW_BSSID_MAX
} HWBSSID_TABLE;

#define BA_SN_MSK 0xfff
enum _TID_SN {
	TID0_SN = 0,
	AC0_SN = TID0_SN,
	TID1_SN,
	AC1_SN = TID1_SN,
	TID2_SN,
	AC2_SN = TID2_SN,
	TID3_SN,
	AC3_SN = TID3_SN,
	TID4_SN,
	TID5_SN,
	TID6_SN,
	TID7_SN
};

char *get_bw_str(int bandwidth);
VOID MTMlmeLpEnter(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID MTMlmeLpExit(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID MTPollTxRxEmpty(struct _RTMP_ADAPTER *pAd);
VOID MTHifPolling(struct _RTMP_ADAPTER *pAd, UINT8 ucDbdcIdx);
VOID MTRadioOn(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
VOID MTRadioOff(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev);
#ifdef RTMP_MAC_PCI
VOID MTPciPollTxRxEmpty(struct _RTMP_ADAPTER *pAd);
#endif /* RTMP_MAC_PCI */


UINT32 MtAsicGetCrcErrCnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetPhyErrCnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetCCACnt(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetChBusyCnt(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
INT MtAsicSetAutoFallBack(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT32 MtAsicAutoFallbackInit(struct _RTMP_ADAPTER *pAd);

#ifdef COMPOS_WIN
VOID MtAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, struct _EXT_CMD_CHAN_SWITCH_T CmdChSwitch);
#else
VOID MtAsicSwitchChannel(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);
#endif /*COMPOS_WIN*/

VOID MtAsicUpdateProtect(struct _RTMP_ADAPTER *pAd, MT_PROTECT_CTRL_T *ProtectCtrl);
VOID MtAsicUpdateRtsThld(struct _RTMP_ADAPTER *pAd, MT_RTS_THRESHOLD_T *RtsThld);

#ifdef SINGLE_SKU_V2
VOID MtAsicUpdateSkuTable(RTMP_ADAPTER *pAd, UINT8 *data);
#endif

VOID MtAsicResetBBPAgent(struct _RTMP_ADAPTER *pAd);
VOID MtAsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx);
INT32 MtAsicSetDevMac(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature);

#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssMode(struct _RTMP_ADAPTER *pAd, UCHAR NumOfBcns);
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT MtAsicSetReptFuncEnableByDriver(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable);
VOID MtAsicInsertRepeaterEntry(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx, UCHAR *pAddr);
VOID MtAsicRemoveRepeaterEntry(struct _RTMP_ADAPTER *pAd, UCHAR CliIdx);
VOID MtAsicInsertRepeaterRootEntry(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx);
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

#ifndef COMPOS_TESTMODE_WIN
VOID MtSetTmrCal(struct _RTMP_ADAPTER *pAd, UCHAR TmrType, UCHAR Channel, UCHAR Bw);
VOID MtSetTmrCR(struct _RTMP_ADAPTER *pAd, UCHAR TmrType);
#endif
INT MtAsicSetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T RxFilte);

#ifdef CONFIG_WTBL_TLV_MODE
INT MtAsicSetRDGByTLV(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UINT8 Wcid);
#else
#ifdef DOT11_N_SUPPORT
INT MtAsicSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR BandIdx);
INT MtAsicWtblSetRDG(struct _RTMP_ADAPTER *pAd, BOOLEAN bEnable, UINT8 Wcid);
#endif /* DOT11_N_SUPPORT */
#endif

VOID MtAsicSetPiggyBack(struct _RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack);
INT MtAsicSetPreTbtt(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR HwBssidIdx);
INT MtAsicSetGPTimer(struct _RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout);
INT MtAsicSetChBusyStat(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
INT MtAsicGetTsfTimeByDriver(
	struct _RTMP_ADAPTER *pAd,
	UINT32 *high_part,
	UINT32 *low_part,
	UCHAR HwBssidIdx);
VOID MtAsicRssiGet(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR *RssiSet);
VOID MtRssiGet(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR *RssiSet);

#ifdef CONFIG_AP_SUPPORT
VOID APCheckBcnQHandler(struct _RTMP_ADAPTER *pAd, INT apidx, BOOLEAN *is_pretbtt_int);
#endif /* CONFIG_AP_SUPPORT */

VOID MtAsicDisableSyncByDriver(struct _RTMP_ADAPTER *pAd, UCHAR HWBssidIdx);
VOID MtAsicEnableBssSyncByDriver(
	struct _RTMP_ADAPTER *pAd,
	USHORT BeaconPeriod,
	UCHAR HWBssidIdx,
	UCHAR OPMode);

INT MtAsicSetWmmParam(struct _RTMP_ADAPTER *pAd, UCHAR idx, UINT ac, UINT type, UINT val);
VOID MtAsicSetEdcaParm(struct _RTMP_ADAPTER *pAd, struct _EDCA_PARM *pEdcaParm);
INT MtAsicSetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit);
UINT32 MtAsicGetRetryLimit(struct _RTMP_ADAPTER *pAd, UINT32 type);
VOID MtAsicSetSlotTime(struct _RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx);

INT MtAsicSetMacMaxLen(struct _RTMP_ADAPTER *pAd);

/* Function by Driver */
INT32 MtAsicSetDevMacByDriver(
	struct _RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature);

INT32 MtAsicSetBssidByDriver(
	struct _RTMP_ADAPTER *pAd,
	struct _BSS_INFO_ARGUMENT_T bss_info_argument);

INT32 MtAsicSetStaRecByDriver(
	struct _RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg);

VOID MtAsicDelWcidTabByDriver(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR wcid_idx);
VOID MtAsicUpdateRxWCIDTableByDriver(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo);
INT32 MtAsicUpdateBASessionByDriver(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_BA_CTRL_T BaCtrl);

/* Generic fucntion */
VOID MtAsicUpdateRxWCIDTable(
	IN struct _RTMP_ADAPTER *pAd,
	IN MT_WCID_TABLE_INFO_T WtblInfo);

VOID MtAsicGetTxTscByDriver(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc);
VOID MtAsicAddSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIdx, UCHAR KeyIdx, struct _CIPHER_KEY *pKey);
VOID MtAsicRemoveSharedKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR BssIndex, UCHAR KeyIdx);
VOID MtAsicTxCntUpdate(struct _RTMP_ADAPTER *pAd, UCHAR wcid, struct _MT_TX_COUNTER *pTxInfo);
VOID MtAsicRssiUpdate(struct _RTMP_ADAPTER *pAd);
VOID MtAsicRcpiReset(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid);
VOID MtAsicSetSMPSByDriver(struct _RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Smps);
VOID MtAsicSetSMPS(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR smps);
UINT16 MtAsicGetTidSnByDriver(struct _RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid);
INT32 MtAsicUpdateBASession(struct _RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl);
VOID MtAsicTxCapAndRateTableUpdate(struct _RTMP_ADAPTER *pAd, UCHAR ucWcid, struct _RA_PHY_CFG_T *prTxPhyCfg, UINT32 *Rate, BOOLEAN fgSpeEn);


VOID MtAsicAddPairwiseKeyEntry(struct _RTMP_ADAPTER *pAd, UCHAR WCID, struct _CIPHER_KEY *pKey);

INT MtAsicSendCommandToMcu(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);
BOOLEAN MtAsicSendCmdToMcuAndWait(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic);
BOOLEAN MtAsicSendCommandToMcuBBP(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked);

VOID MtAsicTurnOffRFClk(struct _RTMP_ADAPTER *pAd, UCHAR Channel);


#ifdef STREAM_MODE_SUPPORT
UINT32 MtStreamModeRegVal(struct _RTMP_ADAPTER *pAd);
VOID MtAsicSetStreamMode(struct _RTMP_ADAPTER *pAd, UCHAR *mac, INT chainIdx, BOOLEAN enable);
VOID MtAsicStreamModeInit(struct _RTMP_ADAPTER *pAd);
#endif /* STREAM_MODE_SUPPORT */


#ifdef DOT11_N_SUPPORT
INT MtAsicReadAggCnt(struct _RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len);
INT MtAsicSetRalinkBurstMode(struct _RTMP_ADAPTER *pAd, BOOLEAN enable);
#endif /* DOT11_N_SUPPORT */

INT MtAsicWaitMacTxRxIdle(struct _RTMP_ADAPTER *pAd);
INT32 MtAsicSetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT MtAsicSetWPDMA(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable, UINT8 WPDMABurstSize);
BOOLEAN MtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us);
BOOLEAN MtAsicResetWPDMA(struct _RTMP_ADAPTER *pAd);
INT MtAsicSetMacWD(struct _RTMP_ADAPTER *pAd);
INT MtAsicSetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNum, UCHAR BandIdx);
INT MtAsicSetRxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT MtAsicSetBW(struct _RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx);
INT MtAsicSetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
INT MtAsicStopContinousTx(struct _RTMP_ADAPTER *pAd);

BOOLEAN MtAsicSetBmcQCR(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Operation,
	IN UCHAR CrReadWrite,
	IN UCHAR HwBssidIdx,
	IN UINT32 apidx,
	IN OUT UINT32    *pcr_val);

#define CR_READ         1
#define CR_WRITE        2

#define BMC_FLUSH       1
#define BMC_ENABLE      2
#define BMC_CNT_UPDATE  3

BOOLEAN MtAsicSetBcnQCR(
	IN struct _RTMP_ADAPTER *pAd,
	IN UCHAR Operation,
	IN UCHAR HwBssidIdx,
	IN UINT32 apidx);

#define BCN_FLUSH       1
#define BCN_ENABLE      2

VOID MtAsicSetDmaFQCR(
	IN struct _RTMP_ADAPTER *pAd,
	IN BOOLEAN FQ_MODE,
	IN UCHAR DEST_QID,
	IN UCHAR DEST_PID,
	IN UCHAR TARG_QID,
	IN UCHAR apidx,
	IN UCHAR widx);

#define HQA_RX_STAT_MACFCSERRCNT		0x1
#define HQA_RX_STAT_MAC_MDRDYCNT		0x2
#define HQA_RX_STAT_PHY_MDRDYCNT		0x3
#define HQA_RX_STAT_PHY_FCSERRCNT		0x4
#define HQA_RX_STAT_PD				0x5
#define HQA_RX_STAT_CCK_SIG_SFD			0x6
#define HQA_RX_STAT_OFDM_SIG_TAG		0x7
#define HQA_RX_STAT_RSSI			0x8
#define HQA_RX_RESET_PHY_COUNT			0x9
#define HQA_RX_RESET_MAC_COUNT			0xa
#define HQA_RX_STAT_RSSI_RX23			0xB
#define HQA_RX_STAT_ACI_HITL			0xC
#define HQA_RX_STAT_ACI_HITH			0xD
#define HQA_RX_STAT_MACFCSERRCNT_BAND1		0xE
#define HQA_RX_STAT_MAC_MDRDYCNT_BAND1		0xF
#define HQA_RX_STAT_MAC_RXLENMISMATCH		0x10
#define HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1	0x11
#define HQA_RX_FIFO_FULL_COUNT			0x12
#define HQA_RX_FIFO_FULL_COUNT_BAND1		0x13
#define HQA_RX_STAT_PHY_MDRDYCNT_BAND1		0x14
#define HQA_RX_STAT_PHY_FCSERRCNT_BAND1		0x15
#define HQA_RX_STAT_PD_BAND1			0x16
#define HQA_RX_STAT_CCK_SIG_SFD_BAND1		0x17
#define HQA_RX_STAT_OFDM_SIG_TAG_BAND1		0x18
#define HQA_RX_ACI_HIT				0x19
#define HQA_RX_STAT_MAC_FCS_OK_COUNT		0x1A

UINT32 MtAsicGetRxStat(struct _RTMP_ADAPTER *pAd, UINT type);
#ifdef CONFIG_ATE
INT MtAsicSetTxTonePower(struct _RTMP_ADAPTER *pAd, INT dec0, INT dec1);
INT MtAsicSetRfFreqOffset(struct _RTMP_ADAPTER *pAd, UINT32 FreqOffset);
INT MtAsicSetTSSI(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect);
INT MtAsicSetDPD(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect);

INT MtAsicSetTxToneTest(struct _RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR Type);
INT MtAsicStartContinousTx(struct _RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Mcs, UINT32 WFSel);
#ifdef COMPOS_TESTMODE_WIN
INT MTAsicTxSetFrequencyOffset(struct _RTMP_ADAPTER *pAd, UINT32 iOffset, BOOLEAN HasBeenSet);
INT MTAsicTxConfigPowerRange(struct _RTMP_ADAPTER *pAd, UCHAR ucMaxPowerDbm, UCHAR ucMinPowerDbm);
INT MTAsicSetTMR(struct _RTMP_ADAPTER *pAd, UCHAR enable);
#endif
#endif /* CONFIG_ATE */

VOID MtAsicSetRxGroup(struct _RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable);

#ifdef DMA_SCH_SUPPORT
INT32 MtAsicDMASchedulerInit(struct _RTMP_ADAPTER *pAd, MT_DMASCH_CTRL_T DmaSchCtrl);
#endif /* DMA_SCH_SUPPORT */

INT MtAsicSetBAWinSizeRange(struct _RTMP_ADAPTER *pAd);
INT MtAsicSetBARTxRate(struct _RTMP_ADAPTER *pAd);
VOID MtAsicSetBARTxCntLimit(struct _RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count);
VOID MtAsicSetTxSClassifyFilter(struct _RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ, UINT32 AggNums, UINT32 Filter, UCHAR BandIdx);
VOID MtAsicInitMac(struct _RTMP_ADAPTER *pAd);
UINT32 MtAsicGetWmmParam(struct _RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType);
VOID MtAsicAddRemoveKey(struct _RTMP_ADAPTER *pAd, MT_SECURITY_CTRL SecurityCtrl);
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
VOID MtAsicAddRemoveKeyByDriver(struct _RTMP_ADAPTER *pAd, struct _ASIC_SEC_INFO *pInfo);
#endif
INT32 MtAsicGetMacInfo(struct _RTMP_ADAPTER *pAd, UINT32 *ChipId, UINT32 *HwVer, UINT32 *FwVer);
INT32 MtAsicGetAntMode(struct _RTMP_ADAPTER *pAd, UCHAR *AntMode);
INT32 MtAsicSetDmaByPassMode(struct _RTMP_ADAPTER *pAd, BOOLEAN isByPass);
BOOLEAN MtAsicGetMcuStatus(struct _RTMP_ADAPTER *pAd, MCU_STAT State);
INT32 MtAsicGetFwSyncValue(struct _RTMP_ADAPTER *pAd);
INT MtAsicTOPInit(struct _RTMP_ADAPTER *pAd);

BOOLEAN MtDmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
BOOLEAN MtDmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
#ifdef CONFIG_AP_SUPPORT
VOID MtDmacSetMbssHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtDmacSetExtTTTTHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtDmacSetExtMbssEnableCR(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
#endif

#if  defined(MT7615)
VOID EnhancedPDMAInit(struct _RTMP_ADAPTER *pAd);
#endif

INT32 MtAsicSetTxQ(struct _RTMP_ADAPTER *pAd, INT WmmSet, INT BandIdx, BOOLEAN Enable);


VOID MtAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MTRestartFW(struct _RTMP_ADAPTER *pAd);
INT32 MtAsicRxHeaderTransCtl(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 MtAsicRxHeaderTaranBLCtl(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);

#ifdef DBDC_MODE
INT32 MtAsicGetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pbInfo);
INT32 MtAsicSetDbdcCtrl(struct _RTMP_ADAPTER *pAd, struct _BCTRL_INFO_T *pbInfo);
#endif /*DBDC_MODE*/


INT32 MtAsicRxHeaderTransCtl(struct _RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP);
INT32 MtAsicRxHeaderTaranBLCtl(struct _RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType);
/* SMAC Start */
BOOLEAN MtSmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
BOOLEAN MtSmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void);
VOID MtSmacSetMbssHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtSmacSetExtTTTTHwCRSetting(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
VOID MtSmacSetExtMbssEnableCR(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable);
/* SMAC End */

#ifdef DOT11_VHT_AC
INT MtAsicSetRtsSignalTA(struct _RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable);
#endif /*DOT11_VHT_AC*/

INT MtAsicAMPDUEfficiencyAdjust(struct _RTMP_ADAPTER *ad, UCHAR	wmm_idx, UCHAR aifs_adjust);
#endif /* __CMM_ASIC_MT_H__ */
INT mt_asic_rts_on_off(struct _RTMP_ADAPTER *ad, UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en);
