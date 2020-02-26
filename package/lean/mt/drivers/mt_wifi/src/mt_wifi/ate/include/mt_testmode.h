
#ifndef _MT_TESTMODE_H
#define _MT_TESTMODE_H

#include "mt_testmode_dmac.h"


#define TESTMODE_WCID_BAND0 2
#define TESTMODE_WCID_BAND1 3

#define ATE_SINGLE_BAND	1
#define ATE_DUAL_BAND	2
#define ATE_GBAND_TYPE	1
#define ATE_ABAND_TYPE	2

#define ATE_BAND_WIDTH_20		0
#define ATE_BAND_WIDTH_40		1
#define ATE_BAND_WIDTH_80		2
#define ATE_BAND_WIDTH_10		3
#define ATE_BAND_WIDTH_5		4
#define ATE_BAND_WIDTH_160		5
#define ATE_BAND_WIDTH_8080		6

enum {
	ATE_USER_PAYLOAD,
	ATE_FIXED_PAYLOAD,
	ATE_RANDOM_PAYLOAD,
};

struct _ATE_DATA_RATE_MAP {
	UCHAR   mcs;            /* MCS index */
	UINT32  tx_data_rate;   /* Data rate in K Bit */
};

struct _ATE_ANT_MAP {
	UINT32 ant_sel;
	UINT32 spe_idx;
};

struct _ATE_TXPWR_GROUP_MAP {
	UINT32 start;
	UINT32 end;
	UINT32 group[4];
};

struct _ATE_CH_KHZ_MAP {
	UINT32 Channel;
	UINT32 Freq;
};

#if defined(COMPOS_TESTMODE_WIN)/* for MT_testmode.c */

#define INC_RING_INDEX(_idx, _RingSize)	\
	do {\
		(_idx)++;						\
		if ((_idx) >= (_RingSize))		\
			_idx = 0;					\
	} while (0)

#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_80		2
#define BAND_WIDTH_160		3
#define BAND_WIDTH_10		4	/* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */
#define BAND_WIDTH_BOTH		5	/* BW20 + BW40 */
#define BAND_WIDTH_5		6
#define BAND_WIDTH_8080		7

#define TX1_G_BAND_TARGET_PWR 0x5E
#define TX0_G_BAND_TARGET_PWR 0x58
enum {
	PDMA_TX,
	PDMA_RX,
	PDMA_TX_RX,
};
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) (&_pstruct->_member)
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) (_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) (_pstruct->_member = _val)
#ifndef COMPOS_TESTMODE_WIN/* NDIS only */
#define MAC_ADDR_LEN    6
/* 2-byte Frame control field */
struct _QAFRAME_CONTROL {
	UINT16		Ver : 2;
	UINT16		Type : 2;
	UINT16		SubType : 4;
	UINT16		ToDs : 1;
	UINT16		FrDs : 1;
	UINT16		MoreFrag : 1;
	UINT16		Retry : 1;
	UINT16		PwrMgmt : 1;
	UINT16		MoreData : 1;
	UINT16		Wep : 1;
	UINT16		Order : 1;
};

struct _QAHEADER_802_11 {
	struct _QAFRAME_CONTROL   FC;
	UINT16          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	UCHAR			Addr3[MAC_ADDR_LEN];
	UINT16			Frag: 4;
	UINT16			Sequence: 12;
};
#endif /* NOT COMPOS_TESTMODE_WIN */
#else
#ifdef DBDC_MODE
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) ((_band)?_pstruct->band_ext[_band-1]._member:_pstruct->_member)
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) ((_band) ?  &_pstruct->band_ext[_band-1]._member :  &_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) ({	\
		UINT32 _ret = _val;													\
		if (_band) {														\
			struct _BAND_INFO *_info = &(_pstruct->band_ext[_band-1]);			\
			_info->_member = _val;										\
		} else															\
			_pstruct->_member = _val;									\
		_ret;															\
	})
#else
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) (&_pstruct->_member)
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) (_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) ({	\
		UINT32 _ret = _val;											\
		_pstruct->_member = _val;									\
		_ret;															\
	})
#endif /* DBDC_MODE */
#endif /* defined(COMPOS_TESTMODE_WIN) */

enum _TESTMODE_STAT_TYPE {
	TESTMODE_RXV,
	TESTMODE_PER_PKT,
	TESTMODE_RESET_CNT,
	TESTMODE_COUNTER_802_11,
	TESTMODE_STAT_TYPE_NUM,
};

struct _RATE_TO_BE_FIX {
	UINT32	TXRate: 6;
	UINT32	TXMode: 3;
	UINT32	Nsts: 2;
	UINT32	STBC: 1;
	UINT32	Reserved: 20;
};

struct rssi_offset_eeprom {
	UINT32 **rssi_eeprom_band_offset;
	UINT32 *n_band_offset;
	UINT32 n_band;
};

INT32 MT_ATERFTestCB(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
INT32 MT_SetATEMPSDump(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);
INT32 MtTestModeInit(struct _RTMP_ADAPTER *pAd);
INT32 MtTestModeExit(struct _RTMP_ADAPTER *pAd);
INT MtTestModeBkCr(struct _RTMP_ADAPTER *pAd, ULONG offset, enum _TEST_BK_CR_TYPE type);
INT MtTestModeRestoreCr(struct _RTMP_ADAPTER *pAd, ULONG offset);
INT32 MT_ATETxControl(struct _RTMP_ADAPTER *pAd, UINT32 band_idx, PNDIS_PACKET pkt);
VOID MT_ATEUpdateRxStatistic(struct _RTMP_ADAPTER *pAd, enum _TESTMODE_STAT_TYPE type, VOID *data);
INT Mt_TestModeInsertPeer(struct _RTMP_ADAPTER *pAd, UINT32 band_ext, CHAR *da, CHAR *sa, CHAR *bss);
INT32 MT_ATETxPkt(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);	/* Export for Loopback */
INT MtATESetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable, UCHAR BandIdx);
INT MtATESetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT MtATESetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
INT MtATESetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T filter);
INT MtATESetCleanPerStaTxQueue(struct _RTMP_ADAPTER *pAd, BOOLEAN sta_pause_enable);
INT32 MT_ATEDumpLog(struct _RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 log_type);
VOID MtCmdATETestResp(struct cmd_msg *msg, char *data, UINT16 len);
INT32 MtATECh2Freq(UINT32 Channel, UINT32 band_idx);
INT32 MtATEGetTxPwrGroup(UINT32 Channel, UINT32 band_idx, UINT32 Ant_idx);
INT32 MT_ATEInsertLog(struct _RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len);
#if !defined(COMPOS_TESTMODE_WIN)/* for MT_testmode.c */
INT MT_ATERxDoneHandle(struct _RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#endif
INT32 MtATERSSIOffset(struct _RTMP_ADAPTER *pAd, INT32 RSSI_org, UINT32 RSSI_idx, INT32 Ch_Band);
INT32 MtATETssiTrainingProc(struct _RTMP_ADAPTER *pAd, UCHAR ucBW, UCHAR ucBandIdx);
#ifdef PRE_CAL_MT7622_SUPPORT
INT MtATE_DPD_Cal_Store_Proc_7622(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /*PRE_CAL_MT7622_SUPPORT*/
#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT MtATE_DPD_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MtATE_DCOC_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT MtATE_Pre_Cal_Proc(RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId);
#endif /* PRE_CAL_TRX_SET2_SUPPORT */

INT32 mt_ate_tx(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, struct _TX_BLK *tx_blk);
INT TxPowerManualCtrl(PRTMP_ADAPTER pAd, BOOLEAN fgPwrManCtrl, UINT8 u1TxPwrModeManual, UINT8 u1TxPwrBwManual, UINT8 u1TxPwrRateManual, INT8 i1TxPwrValueManual, UCHAR ucBandIdx);

#if defined(COMPOS_TESTMODE_WIN)
#endif
#define MT_ATEInit(_pAd) ({		\
		UINT32 _ret;					\
		_ret = MtTestModeInit(_pAd);	\
		_ret;							\
	})

#define MT_ATEExit(_pAd) ({		\
		UINT32 _ret;					\
		_ret = MtTestModeExit(_pAd);	\
		_ret;							\
	})
#endif /* _MT_TESTMODE_H */
